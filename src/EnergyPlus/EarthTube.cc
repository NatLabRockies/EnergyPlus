// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// C++ Headers
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/EarthTube.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::EarthTube {
// Module containing the data for Earth Tube system

// MODULE INFORMATION:
//       AUTHOR         Kwang Ho Lee
//       DATE WRITTEN   November 2005

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithyms required to manage the EarthTube System Component

// REFERENCES:
// 1. M. Krarti, "Analytical Model to Predict Annual Soil Surface Temperature Variation",
// Journal of Solar Energy Engineering 117, 1995, pp 91-99
// 2. K. Labs In: J. Cook, editor, "Passive Cooling",
// Cambridge Massachusetts, MIT Press, 1989, pp 206-212

// This is an interesting one.  The actual members of the enum are never explicitly used
// The enum is used in a getEnumValue call to determine what was found in GetInput
// The value is then used as an array index to lookup thermal conductivity and such from some std::arrays
// So the IDE thinks these are unused, and I'm not sure the best way to hint that they sorta aren't
enum class SoilType
{
    Invalid = -1,
    HeavyAndSat,
    HeavyAndDamp,
    HeavyAndDry,
    LightAndDry,
    Num
};

int totEarthTube = 0;

constexpr std::array<std::string_view, static_cast<int>(Ventilation::Num)> ventilationNamesUC = {"NATURAL", "INTAKE", "EXHAUST"};
constexpr std::array<std::string_view, static_cast<int>(SoilType::Num)> soilTypeNamesUC = {
    "HEAVYANDSATURATED", "HEAVYANDDAMP", "HEAVYANDDRY", "LIGHTANDDRY"};
constexpr std::array<std::string_view, static_cast<int>(EarthTubeModelType::Num)> solutionTypeNamesUC = {"BASIC", "VERTICAL"};

void ManageEarthTube(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Kwang Ho Lee
    //       DATE WRITTEN   November 2005

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages the simulation of EarthTube unit.
    // This driver manages the calls to all of
    // the other drivers and simulation algorithms.

    // Obtains and Allocates heat balance related parameters from input file
    if (state.dataEarthTube->GetInputFlag) {
        bool ErrorsFound = false;
        GetEarthTube(state, ErrorsFound);
        state.dataEarthTube->GetInputFlag = false;
    }

    if (state.dataEarthTube->EarthTubeSys.empty()) {
        return;
    }

    initEarthTubeVertical(state);

    CalcEarthTube(state);

    ReportEarthTube(state);
}

void GetEarthTube(EnergyPlusData &state, bool &ErrorsFound) // If errors found in input
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Kwang Ho Lee
    //       DATE WRITTEN   November 2005

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine obtains input data for EarthTube units and
    // stores it in the EarthTube data structure.

    static constexpr std::string_view routineName = "GetEarthTube";

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr EarthTubeTempLimit(100.0); // degrees Celsius

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumAlpha;
    int NumNumber;
    int IOStat;
    int Loop;
    Array1D_bool RepVarSet;

    RepVarSet.dimension(state.dataGlobal->NumOfZones, true);

    // Following used for reporting
    state.dataEarthTube->ZnRptET.allocate(state.dataGlobal->NumOfZones);

    auto &s_ipsc = state.dataIPShortCut;

    s_ipsc->cCurrentModuleObject = "ZoneEarthtube:Parameters";

    int totEarthTubePars = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    state.dataEarthTube->EarthTubePars.allocate(totEarthTubePars);

    for (Loop = 1; Loop <= totEarthTubePars; ++Loop) {
        auto &thisEarthTubePars = state.dataEarthTube->EarthTubePars(Loop);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 Loop,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlpha,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNumber,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        thisEarthTubePars.nameParameters = s_ipsc->cAlphaArgs(1);
        // Check to make sure name is unique
        for (int otherParams = 1; otherParams < Loop; ++otherParams) {
            if (Util::SameString(thisEarthTubePars.nameParameters, state.dataEarthTube->EarthTubePars(otherParams).nameParameters)) {
                ShowSevereError(
                    state,
                    format("{}: {} = {} is not a unique name.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(1), s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, format("Check the other {} names for a duplicate.", s_ipsc->cCurrentModuleObject));
                ErrorsFound = true;
            }
        }

        thisEarthTubePars.numNodesAbove = s_ipsc->rNumericArgs(1);
        thisEarthTubePars.numNodesBelow = s_ipsc->rNumericArgs(2);
        thisEarthTubePars.dimBoundAbove = s_ipsc->rNumericArgs(3);
        thisEarthTubePars.dimBoundBelow = s_ipsc->rNumericArgs(4);
        thisEarthTubePars.width = s_ipsc->rNumericArgs(5);
    }

    s_ipsc->cCurrentModuleObject = "ZoneEarthtube";
    totEarthTube = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    state.dataEarthTube->EarthTubeSys.allocate(totEarthTube);

    for (Loop = 1; Loop <= totEarthTube; ++Loop) {
        auto &thisEarthTube = state.dataEarthTube->EarthTubeSys(Loop);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 Loop,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlpha,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNumber,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        // First Alpha is Zone Name
        thisEarthTube.ZonePtr = Util::FindItemInList(s_ipsc->cAlphaArgs(1), state.dataHeatBal->Zone);
        if (thisEarthTube.ZonePtr == 0) {
            ShowSevereError(state, format("{}: {} not found={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(1), s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        // Second Alpha is Schedule Name
        if (s_ipsc->lAlphaFieldBlanks(2)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
            ErrorsFound = true;
        } else if ((thisEarthTube.availSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
            ErrorsFound = true;
        }

        // Overall parameters and their limits
        thisEarthTube.DesignLevel = s_ipsc->rNumericArgs(1);

        thisEarthTube.MinTemperature = s_ipsc->rNumericArgs(2);
        if ((thisEarthTube.MinTemperature < -EarthTubeTempLimit) || (thisEarthTube.MinTemperature > EarthTubeTempLimit)) {
            ShowSevereError(state,
                            format("{}: {}={} must have a minimum temperature between -{:.0R}C and {:.0R}C",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaFieldNames(1),
                                   s_ipsc->cAlphaArgs(1),
                                   EarthTubeTempLimit,
                                   EarthTubeTempLimit));
            ShowContinueError(state, format("Entered value={:.0R}", thisEarthTube.MinTemperature));
            ErrorsFound = true;
        }

        thisEarthTube.MaxTemperature = s_ipsc->rNumericArgs(3);
        if ((thisEarthTube.MaxTemperature < -EarthTubeTempLimit) || (thisEarthTube.MaxTemperature > EarthTubeTempLimit)) {
            ShowSevereError(state,
                            format("{}: {}={} must have a maximum temperature between -{:.0R}C and {:.0R}C",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaFieldNames(1),
                                   s_ipsc->cAlphaArgs(1),
                                   EarthTubeTempLimit,
                                   EarthTubeTempLimit));
            ShowContinueError(state, format("Entered value={:.0R}", thisEarthTube.MaxTemperature));
            ErrorsFound = true;
        }

        thisEarthTube.DelTemperature = s_ipsc->rNumericArgs(4); //  3/12/03  Negative del temp now allowed COP

        // if we have a blank, then just set it to the Natural type, otherwise, search on it
        if (s_ipsc->cAlphaArgs(3).empty()) {
            thisEarthTube.FanType = Ventilation::Natural;
        } else {
            thisEarthTube.FanType = static_cast<Ventilation>(getEnumValue(ventilationNamesUC, s_ipsc->cAlphaArgs(3)));
            if (thisEarthTube.FanType == Ventilation::Invalid) {
                ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            }
        }

        thisEarthTube.FanPressure = s_ipsc->rNumericArgs(5);
        if (thisEarthTube.FanPressure < 0.0) {
            ShowSevereError(state,
                            format("{}: {}={}, {} must be positive, entered value={:.2R}",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaFieldNames(1),
                                   s_ipsc->cAlphaArgs(1),
                                   s_ipsc->cNumericFieldNames(5),
                                   thisEarthTube.FanPressure));
            ErrorsFound = true;
        }

        thisEarthTube.FanEfficiency = s_ipsc->rNumericArgs(6);
        if ((thisEarthTube.FanEfficiency <= 0.0) || (thisEarthTube.FanEfficiency > 1.0)) {
            ShowSevereError(state,
                            format("{}: {}={}, {} must be greater than zero and less than or equal to one, entered value={:.2R}",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaFieldNames(1),
                                   s_ipsc->cAlphaArgs(1),
                                   s_ipsc->cNumericFieldNames(6),
                                   thisEarthTube.FanEfficiency));
            ErrorsFound = true;
        }

        thisEarthTube.r1 = s_ipsc->rNumericArgs(7);
        if (thisEarthTube.r1 <= 0.0) {
            ShowSevereError(state,
                            format("{}: {}={}, {} must be positive, entered value={:.2R}",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaFieldNames(1),
                                   s_ipsc->cAlphaArgs(1),
                                   s_ipsc->cNumericFieldNames(7),
                                   thisEarthTube.r1));
            ErrorsFound = true;
        }

        thisEarthTube.r2 = s_ipsc->rNumericArgs(8);
        if (thisEarthTube.r2 <= 0.0) {
            ShowSevereError(state,
                            format("{}: {}={}, {} must be positive, entered value={:.2R}",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaFieldNames(1),
                                   s_ipsc->cAlphaArgs(1),
                                   s_ipsc->cNumericFieldNames(8),
                                   thisEarthTube.r2));
            ErrorsFound = true;
        }

        thisEarthTube.r3 = 2.0 * thisEarthTube.r1;

        thisEarthTube.PipeLength = s_ipsc->rNumericArgs(9);
        if (thisEarthTube.PipeLength <= 0.0) {
            ShowSevereError(state,
                            format("{}: {}={}, {} must be positive, entered value={:.2R}",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaFieldNames(1),
                                   s_ipsc->cAlphaArgs(1),
                                   s_ipsc->cNumericFieldNames(9),
                                   thisEarthTube.PipeLength));
            ErrorsFound = true;
        }

        thisEarthTube.PipeThermCond = s_ipsc->rNumericArgs(10);
        if (thisEarthTube.PipeThermCond <= 0.0) {
            ShowSevereError(state,
                            format("{}: {}={}, {} must be positive, entered value={:.2R}",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaFieldNames(1),
                                   s_ipsc->cAlphaArgs(1),
                                   s_ipsc->cNumericFieldNames(10),
                                   thisEarthTube.PipeThermCond));
            ErrorsFound = true;
        }

        thisEarthTube.z = s_ipsc->rNumericArgs(11);
        if (thisEarthTube.z <= 0.0) {
            ShowSevereError(state,
                            format("{}: {}={}, {} must be positive, entered value={:.2R}",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaFieldNames(1),
                                   s_ipsc->cAlphaArgs(1),
                                   s_ipsc->cNumericFieldNames(11),
                                   thisEarthTube.z));
            ErrorsFound = true;
        }
        if (thisEarthTube.z <= (thisEarthTube.r1 + thisEarthTube.r2 + thisEarthTube.r3)) {
            // Note that code in initEarthTubeVertical assumes that this check remains in place--if this ever gets changed,
            // code in initEarthTubeVertical must be modified
            ShowSevereError(state,
                            format("{}: {}={}, {} must be greater than 3*{} + {} entered value={:.2R} ref sum={:.2R}",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaFieldNames(1),
                                   s_ipsc->cAlphaArgs(1),
                                   s_ipsc->cNumericFieldNames(11),
                                   s_ipsc->cNumericFieldNames(7),
                                   s_ipsc->cNumericFieldNames(8),
                                   thisEarthTube.z,
                                   thisEarthTube.r1 + thisEarthTube.r2 + thisEarthTube.r3));
            ErrorsFound = true;
        }

        SoilType soilType = static_cast<SoilType>(getEnumValue(soilTypeNamesUC, s_ipsc->cAlphaArgs(4)));
        constexpr std::array<Real64, static_cast<int>(SoilType::Num)> thermalDiffusivity = {0.0781056, 0.055728, 0.0445824, 0.024192};
        constexpr std::array<Real64, static_cast<int>(SoilType::Num)> thermalConductivity = {2.42, 1.3, 0.865, 0.346};
        if (soilType == SoilType::Invalid) {
            ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
            ErrorsFound = true;
        } else {
            thisEarthTube.SoilThermDiff = thermalDiffusivity[static_cast<int>(soilType)];
            thisEarthTube.SoilThermCond = thermalConductivity[static_cast<int>(soilType)];
        }

        thisEarthTube.AverSoilSurTemp = s_ipsc->rNumericArgs(12);
        thisEarthTube.ApmlSoilSurTemp = s_ipsc->rNumericArgs(13);
        thisEarthTube.SoilSurPhaseConst = int(s_ipsc->rNumericArgs(14));

        // Override any user input for cases where natural ventilation is being used
        if (thisEarthTube.FanType == Ventilation::Natural) {
            thisEarthTube.FanPressure = 0.0;
            thisEarthTube.FanEfficiency = 1.0;
        }

        thisEarthTube.ConstantTermCoef = s_ipsc->rNumericArgs(15);
        thisEarthTube.TemperatureTermCoef = s_ipsc->rNumericArgs(16);
        thisEarthTube.VelocityTermCoef = s_ipsc->rNumericArgs(17);
        thisEarthTube.VelocitySQTermCoef = s_ipsc->rNumericArgs(18);

        // cAlphaArgs(5)--Model type: basic or vertical
        // only process cAlphaArgs(6) if cAlphaArgs(5) is "Vertical"
        if (s_ipsc->cAlphaArgs(5).empty()) {
            thisEarthTube.ModelType = EarthTubeModelType::Basic;
        } else {
            thisEarthTube.ModelType = static_cast<EarthTubeModelType>(getEnumValue(solutionTypeNamesUC, s_ipsc->cAlphaArgs(5)));
            if (thisEarthTube.ModelType == EarthTubeModelType::Invalid) {
                ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5));
                ErrorsFound = true;
            }
        }

        if (thisEarthTube.ModelType == EarthTubeModelType::Vertical) {
            thisEarthTube.r3 = 0.0; // Vertical model does not use this parameter--reset to zero (keep because r3=0 necessary so Rs=0 in calc routine)
            // Process the parameters based on the name (link via index)
            thisEarthTube.vertParametersPtr = 0;
            for (int parIndex = 1; parIndex <= totEarthTubePars; ++parIndex) {
                if (Util::SameString(s_ipsc->cAlphaArgs(6), state.dataEarthTube->EarthTubePars(parIndex).nameParameters)) {
                    thisEarthTube.vertParametersPtr = parIndex;
                    break;
                }
            }
            if (thisEarthTube.vertParametersPtr == 0) { // didn't find a match
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6));
                ErrorsFound = true;
            }
        }

        if (thisEarthTube.ZonePtr > 0) {
            if (RepVarSet(thisEarthTube.ZonePtr)) {
                RepVarSet(thisEarthTube.ZonePtr) = false;
                auto &zone = state.dataHeatBal->Zone(thisEarthTube.ZonePtr);
                auto &thisZnRptET = state.dataEarthTube->ZnRptET(thisEarthTube.ZonePtr);

                SetupOutputVariable(state,
                                    "Earth Tube Zone Sensible Cooling Energy",
                                    Constant::Units::J,
                                    thisZnRptET.EarthTubeHeatLoss,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Zone Sensible Cooling Rate",
                                    Constant::Units::W,
                                    thisZnRptET.EarthTubeHeatLossRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Zone Sensible Heating Energy",
                                    Constant::Units::J,
                                    thisZnRptET.EarthTubeHeatGain,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Zone Sensible Heating Rate",
                                    Constant::Units::W,
                                    thisZnRptET.EarthTubeHeatGainRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Air Flow Volume",
                                    Constant::Units::m3,
                                    thisZnRptET.EarthTubeVolume,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Current Density Air Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    thisZnRptET.EarthTubeVolFlowRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Standard Density Air Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    thisZnRptET.EarthTubeVolFlowRateStd,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Air Flow Mass",
                                    Constant::Units::kg,
                                    thisZnRptET.EarthTubeMass,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Air Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    thisZnRptET.EarthTubeMassFlowRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Water Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    thisZnRptET.EarthTubeWaterMassFlowRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Fan Electricity Energy",
                                    Constant::Units::J,
                                    thisZnRptET.EarthTubeFanElec,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    zone.Name,
                                    Constant::eResource::Electricity,
                                    OutputProcessor::Group::Building);
                SetupOutputVariable(state,
                                    "Earth Tube Fan Electricity Rate",
                                    Constant::Units::W,
                                    thisZnRptET.EarthTubeFanElecPower,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Zone Inlet Air Temperature",
                                    Constant::Units::C,
                                    thisZnRptET.EarthTubeAirTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Ground Interface Temperature",
                                    Constant::Units::C,
                                    thisEarthTube.GroundTempt,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Outdoor Air Heat Transfer Rate",
                                    Constant::Units::W,
                                    thisZnRptET.EarthTubeOATreatmentPower,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Zone Inlet Wet Bulb Temperature",
                                    Constant::Units::C,
                                    thisZnRptET.EarthTubeWetBulbTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Earth Tube Zone Inlet Humidity Ratio",
                                    Constant::Units::kgWater_kgDryAir,
                                    thisZnRptET.EarthTubeHumRat,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    zone.Name);
            }
        }
    }

    CheckEarthTubesInZones(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);

    if (ErrorsFound) {
        ShowFatalError(state, format("{}: Errors getting input.  Program terminates.", s_ipsc->cCurrentModuleObject));
    }
}

void CheckEarthTubesInZones(EnergyPlusData &state,
                            std::string const &ZoneName, // name of zone for error reporting
                            std::string_view FieldName,  // name of earth tube in input
                            bool &ErrorsFound            // Found a problem
)
{
    // Check to make sure there is only one earth tube statement per zone
    int numEarthTubes = (int)state.dataEarthTube->EarthTubeSys.size();
    for (int Loop = 1; Loop <= numEarthTubes - 1; ++Loop) {
        for (int Loop1 = Loop + 1; Loop1 <= numEarthTubes; ++Loop1) {
            if (state.dataEarthTube->EarthTubeSys(Loop).ZonePtr == state.dataEarthTube->EarthTubeSys(Loop1).ZonePtr) {
                ShowSevereError(state, format("{} has more than one {} associated with it.", ZoneName, FieldName));
                ShowContinueError(state, format("Only one {} is allowed per zone.  Check the definitions of {}", FieldName, FieldName));
                ShowContinueError(state, "in your input file and make sure that there is only one defined for each zone.");
                ErrorsFound = true;
            }
        }
    }
}

void initEarthTubeVertical(EnergyPlusData &state)
{
    if (state.dataEarthTube->initFirstTime) {
        state.dataEarthTube->initFirstTime = false;
        for (int etNum = 1; etNum <= totEarthTube; ++etNum) {
            auto &thisEarthTube = state.dataEarthTube->EarthTubeSys(etNum);
            if (thisEarthTube.ModelType != EarthTubeModelType::Vertical) {
                continue; // Skip earth tubes that do not use vertical solution
            }
            auto &thisEarthTubeParams = state.dataEarthTube->EarthTubePars(thisEarthTube.vertParametersPtr);
            thisEarthTube.totNodes = thisEarthTubeParams.numNodesAbove + thisEarthTubeParams.numNodesBelow + 1;
            thisEarthTube.aCoeff.resize(thisEarthTube.totNodes);
            thisEarthTube.bCoeff.resize(thisEarthTube.totNodes);
            thisEarthTube.cCoeff.resize(thisEarthTube.totNodes);
            thisEarthTube.cCoeff0.resize(thisEarthTube.totNodes);
            thisEarthTube.dCoeff.resize(thisEarthTube.totNodes);
            thisEarthTube.cPrime.resize(thisEarthTube.totNodes);
            thisEarthTube.dPrime.resize(thisEarthTube.totNodes);
            thisEarthTube.cPrime0.resize(thisEarthTube.totNodes);
            thisEarthTube.tCurrent.resize(thisEarthTube.totNodes);
            thisEarthTube.tLast.resize(thisEarthTube.totNodes);
            thisEarthTube.depthNode.resize(thisEarthTube.totNodes);
            thisEarthTube.tUndist.resize(thisEarthTube.totNodes);
            Real64 thickBase = (thisEarthTube.z - 3.0 * thisEarthTube.r1);
            Real64 thickTop = thickBase * thisEarthTubeParams.dimBoundAbove / float(thisEarthTubeParams.numNodesAbove);
            Real64 thickBottom = thickBase * thisEarthTubeParams.dimBoundBelow / float(thisEarthTubeParams.numNodesBelow);
            Real64 thickEarthTube = 4.0 * thisEarthTube.r1;
            Real64 deltat = state.dataGlobal->TimeStepZone;
            Real64 thermDiff = thisEarthTube.SoilThermDiff / Constant::rHoursInDay; // convert to "per hour" from "per day"

            // Node equations determine the _Coeff terms--see Engineering Referenve for details on these equation types
            // Note that node numbers are shifted for c++ arrays that go from 0 to numNodes-1.
            // Node Type 1 (Top Node)
            Real64 commonTerm = thermDiff * deltat / (thickTop * thickTop);
            thisEarthTube.aCoeff[0] = 0.0; // no a0 value
            thisEarthTube.bCoeff[0] = 1.0 + 3.0 * commonTerm;
            thisEarthTube.cCoeff[0] = -1.0 * commonTerm;
            thisEarthTube.dMult0 = 2.0 * commonTerm; // does not include temperatures (upper boundary or previous time step)--added later
            // Node Type 2 (Generic Top Section Node)
            for (int nodeNum = 1; nodeNum <= thisEarthTubeParams.numNodesAbove - 2; ++nodeNum) {
                thisEarthTube.aCoeff[nodeNum] = -1.0 * commonTerm;
                thisEarthTube.bCoeff[nodeNum] = 1.0 + 2.0 * commonTerm;
                thisEarthTube.cCoeff[nodeNum] = -1.0 * commonTerm;
            }
            // Node Type 3 (Last Top Section Node)
            int thisNode = thisEarthTubeParams.numNodesAbove - 1;
            Real64 commonTerm2 = 2.0 * thermDiff * deltat / (thickTop + thickEarthTube) / thickTop;
            thisEarthTube.aCoeff[thisNode] = -1.0 * commonTerm;
            thisEarthTube.bCoeff[thisNode] = 1.0 + commonTerm + commonTerm2;
            thisEarthTube.cCoeff[thisNode] = -1.0 * commonTerm2;
            // Node Type 4 (Earth Tube Node)
            thisNode = thisEarthTubeParams.numNodesAbove;
            commonTerm = 2.0 * thermDiff * deltat / (thickTop + thickEarthTube) / thickEarthTube;
            commonTerm2 = 2.0 * thermDiff * deltat / (thickBottom + thickEarthTube) / thickEarthTube;
            thisEarthTube.aCoeff[thisNode] = -1.0 * commonTerm;
            thisEarthTube.bCoeff[thisNode] = 1.0 + commonTerm + commonTerm2; // does not include earth tube air flow term--added later
            thisEarthTube.cCoeff[thisNode] = -1.0 * commonTerm2;
            // Node Type 5 (First Bottom Section Node)
            thisNode = thisEarthTubeParams.numNodesAbove + 1;
            commonTerm = thermDiff * deltat / (thickBottom * thickBottom);
            commonTerm2 = 2.0 * thermDiff * deltat / (thickBottom + thickEarthTube) / thickBottom;
            thisEarthTube.aCoeff[thisNode] = -1.0 * commonTerm2;
            thisEarthTube.bCoeff[thisNode] = 1.0 + commonTerm + commonTerm2;
            thisEarthTube.cCoeff[thisNode] = -1.0 * commonTerm;
            // Node Type 6 (Generic Bottom Section Node)
            for (int nodeNum = thisNode + 1; nodeNum <= thisEarthTube.totNodes - 2; ++nodeNum) {
                thisEarthTube.aCoeff[nodeNum] = -1.0 * commonTerm;
                thisEarthTube.bCoeff[nodeNum] = 1.0 + 2.0 * commonTerm;
                thisEarthTube.cCoeff[nodeNum] = -1.0 * commonTerm;
            }
            // Node Type 7 (Last Bottom Section Node, i.e. Last Node)
            thisNode = thisEarthTube.totNodes - 1; // shifted due to c++ arrays that go from 0 to numNodes-1
            thisEarthTube.aCoeff[thisNode] = -1.0 * commonTerm;
            thisEarthTube.bCoeff[thisNode] = 1.0 + 3.0 * commonTerm;
            thisEarthTube.cCoeff[thisNode] = 0.0;    // no cN value
            thisEarthTube.dMultN = 2.0 * commonTerm; // does not include previous temperature and earth tube air flow terms--added later

            // Initialize node temperatures using undisturbed temperature equation and node depths
            // First, nodes above the earth tube
            thisEarthTube.depthNode[thisEarthTubeParams.numNodesAbove - 1] = thisEarthTube.z - 0.5 * (thickEarthTube + thickTop);
            for (int nodeNum = thisEarthTubeParams.numNodesAbove - 2; nodeNum >= 0; --nodeNum) {
                thisEarthTube.depthNode[nodeNum] = thisEarthTube.depthNode[nodeNum + 1] - thickTop;
            }
            // Now, the earth tube node
            thisEarthTube.depthNode[thisEarthTubeParams.numNodesAbove] = thisEarthTube.z;
            // Finally the nodes below the earth tube
            thisEarthTube.depthNode[thisEarthTubeParams.numNodesAbove + 1] = thisEarthTube.z + 0.5 * (thickEarthTube + thickBottom);
            for (int nodeNumBelow = 2; nodeNumBelow <= thisEarthTubeParams.numNodesBelow; ++nodeNumBelow) {
                int nodeNum = thisEarthTubeParams.numNodesAbove + nodeNumBelow;
                thisEarthTube.depthNode[nodeNum] = thisEarthTube.depthNode[nodeNum - 1] + thickBottom;
            }
            thisEarthTube.depthUpperBound = thisEarthTube.depthNode[0] - 0.5 * thickTop;
            thisEarthTube.depthLowerBound = thisEarthTube.depthNode[thisEarthTube.totNodes - 1] + 0.5 * thickBottom;

            // Calculate constant part of air flow term at earth tube node.  Note that diffusiity/conductivity = 1/(density*specific_heat)
            thisEarthTube.airFlowCoeff = state.dataGlobal->TimeStepZone * thermDiff / thisEarthTube.SoilThermCond / thickEarthTube /
                                         thisEarthTubeParams.width / thisEarthTube.PipeLength;

            // Calculate some initial values in the Thomas algorithm.  This includes c' when effectiveness is zero (entire c').
            // For any other effectiveness, c' will be the same as c' when effectiveness for is zero for the nodes above the earth
            // tube.  So, the c' for effectiveness of zero (cPrime0) can be reused as needed.
            for (int nodeNum = 0; nodeNum <= thisEarthTube.totNodes - 1; ++nodeNum) {
                thisEarthTube.cCoeff0[nodeNum] = thisEarthTube.cCoeff[nodeNum];
            }
            thisEarthTube.initCPrime0();

            auto &zone = state.dataHeatBal->Zone(thisEarthTube.ZonePtr);
            for (int nodeNum = 1; nodeNum <= thisEarthTube.totNodes; ++nodeNum) {
                SetupOutputVariable(state,
                                    format("Earth Tube Node Temperature {}", nodeNum),
                                    Constant::Units::C,
                                    thisEarthTube.tCurrent[nodeNum - 1],
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    zone.Name);
                SetupOutputVariable(state,
                                    format("Earth Tube Undisturbed Ground Temperature {}", nodeNum),
                                    Constant::Units::C,
                                    thisEarthTube.tUndist[nodeNum - 1],
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    zone.Name);
            }
            SetupOutputVariable(state,
                                "Earth Tube Upper Boundary Ground Temperature",
                                Constant::Units::C,
                                thisEarthTube.tUpperBound,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                zone.Name);
            SetupOutputVariable(state,
                                "Earth Tube Lower Boundary Ground Temperature",
                                Constant::Units::C,
                                thisEarthTube.tLowerBound,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                zone.Name);
        }
    } // ...end of firstTimeInits block

    Real64 timeElapsedLoc =
        state.dataGlobal->HourOfDay + state.dataGlobal->TimeStep * state.dataGlobal->TimeStepZone + state.dataHVACGlobal->SysTimeElapsed;
    if (state.dataEarthTube->timeElapsed !=
        timeElapsedLoc) { // time changed, update last with "current", avoids duplicate initializations and improper updates
        if (state.dataGlobal->BeginDayFlag || state.dataGlobal->BeginEnvrnFlag) {
            // update all of the undisturbed temperatures (only need to do this once per day because the equation only changes as the day changes
            for (int etNum = 1; etNum <= totEarthTube; ++etNum) {
                auto &thisEarthTube = state.dataEarthTube->EarthTubeSys(etNum);
                if (thisEarthTube.ModelType != EarthTubeModelType::Vertical) {
                    continue; // Skip earth tubes that do not use vertical solution
                }
                thisEarthTube.tUpperBound = thisEarthTube.calcUndisturbedGroundTemperature(state, thisEarthTube.depthUpperBound);
                thisEarthTube.tLowerBound = thisEarthTube.calcUndisturbedGroundTemperature(state, thisEarthTube.depthLowerBound);
                for (int nodeNum = 0; nodeNum <= thisEarthTube.totNodes - 1; ++nodeNum) {
                    thisEarthTube.tUndist[nodeNum] = thisEarthTube.calcUndisturbedGroundTemperature(state, thisEarthTube.depthNode[nodeNum]);
                }
            }
        } // ...end of BeginDayFlag block

        if (state.dataGlobal->BeginEnvrnFlag ||
            (!state.dataGlobal->WarmupFlag && state.dataGlobal->BeginDayFlag && state.dataGlobal->DayOfSim == 1)) {
            for (int etNum = 1; etNum <= totEarthTube; ++etNum) {
                auto &thisEarthTube = state.dataEarthTube->EarthTubeSys(etNum);
                if (thisEarthTube.ModelType != EarthTubeModelType::Vertical) {
                    continue; // Skip earth tubes that do not use vertical solution
                }
                for (int nodeNum = 0; nodeNum <= thisEarthTube.totNodes - 1; ++nodeNum) {
                    thisEarthTube.tLast[nodeNum] = thisEarthTube.tUndist[nodeNum];
                    thisEarthTube.tCurrent[nodeNum] = thisEarthTube.tLast[nodeNum];
                }
            }
        }

        for (int etNum = 1; etNum <= totEarthTube; ++etNum) {
            auto &thisEarthTube = state.dataEarthTube->EarthTubeSys(etNum);
            if (thisEarthTube.ModelType != EarthTubeModelType::Vertical) {
                continue; // Skip earth tubes that do not use vertical solution
            }
            for (int nodeNum = 0; nodeNum <= thisEarthTube.totNodes - 1; ++nodeNum) {
                thisEarthTube.tLast[nodeNum] = thisEarthTube.tCurrent[nodeNum];
            }
        }
    }
    state.dataEarthTube->timeElapsed = timeElapsedLoc;
}

void EarthTubeData::initCPrime0()
{
    // Calculate c' for when effectiveness is zero.  Will use these values when there is no air flow through the earth tube
    // and also use the values in the top portion of the solution (before the earth tube node) since these will not change.
    this->cPrime0[0] = this->cCoeff0[0] / this->bCoeff[0];
    for (int i = 1; i <= this->totNodes - 2; ++i) {
        this->cPrime0[i] = this->cCoeff0[i] / (this->bCoeff[i] - this->aCoeff[i] * this->cPrime0[i - 1]);
    }
    cPrime0[this->totNodes - 1] = 0.0;
}

void CalcEarthTube(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Kwang Ho Lee
    //       DATE WRITTEN   November 2005

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine simulates the components making up the EarthTube unit.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 Process1;    // Variable Used in the Middle of the Calculation
    Real64 GroundTempt; // Ground Temperature between Depth z at time t

    Real64 AirThermCond;         // Thermal Conductivity of Air (W/mC)
    Real64 AirKinemVisco;        // Kinematic Viscosity of Air (m2/s)
    Real64 AirThermDiffus;       // Thermal Diffusivity of Air (m2/s)
    Real64 Re;                   // Reynolds Number for Flow Inside Pipe
    Real64 Pr;                   // Prandtl Number for Flow Inside Pipe
    Real64 Nu;                   // Nusselt Number for Flow Inside Pipe
    Real64 fa;                   // Friction Factor of Pipe
    Real64 PipeHeatTransCoef;    // Convective Heat Transfer Coefficient at Inner Pipe Surface
    Real64 Rc;                   // Thermal Resistance due to Convection between Air and Pipe Inner Surface
    Real64 Rp;                   // Thermal Resistance due to Conduction between Pipe Inner and Outer Surface
    Real64 Rs;                   // Thermal Resistance due to Conduction between Pipe Outer Surface and Soil
    Real64 Rt;                   // Total Thermal Resistance between Pipe Air and Soil
    Real64 OverallHeatTransCoef; // Overall Heat Transfer Coefficient of Earth Tube
    Real64 AverPipeAirVel;       // Average Pipe Air Velocity (m/s)
    Real64 AirMassFlowRate;      // Actual Mass Flow Rate of Air inside Pipe
    Real64 AirSpecHeat;          // Specific Heat of Air
    Real64 AirDensity;           // Density of Air
    Real64 EVF;

    int numEarthTubes = (int)state.dataEarthTube->EarthTubeSys.size();
    Real64 outTdb = state.dataEnvrn->OutDryBulbTemp;
    for (int Loop = 1; Loop <= numEarthTubes; ++Loop) {
        auto &thisEarthTube = state.dataEarthTube->EarthTubeSys(Loop);
        int NZ = thisEarthTube.ZonePtr;
        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(NZ);
        thisZoneHB.MCPTE = 0.0;
        thisZoneHB.MCPE = 0.0;
        thisZoneHB.EAMFL = 0.0;
        thisZoneHB.EAMFLxHumRat = 0.0;
        thisEarthTube.FanPower = 0.0;

        // Don't simulate for Basic Solution if the zone is below the minimum temperature limit, above the maximum temperature limit
        // or below the temperature difference limit
        bool tempShutDown = thisZoneHB.MAT < thisEarthTube.MinTemperature || thisZoneHB.MAT > thisEarthTube.MaxTemperature ||
                            std::abs(thisZoneHB.MAT - outTdb) < thisEarthTube.DelTemperature;
        // check for Basic model and some temperature limit preventing the earth tube from running
        if ((thisEarthTube.ModelType == EarthTubeModelType::Basic) && (tempShutDown)) {
            continue;
        }

        AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, outTdb, state.dataEnvrn->OutHumRat);
        AirSpecHeat = Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat);
        if (tempShutDown) {
            EVF = 0.0;
        } else {
            EVF = thisEarthTube.DesignLevel * thisEarthTube.availSched->getCurrentVal();
        }
        thisZoneHB.MCPE =
            EVF * AirDensity * AirSpecHeat *
            (thisEarthTube.ConstantTermCoef +
             std::abs(outTdb - state.dataZoneTempPredictorCorrector->zoneHeatBalance(NZ).MAT) * thisEarthTube.TemperatureTermCoef +
             state.dataEnvrn->WindSpeed * (thisEarthTube.VelocityTermCoef + state.dataEnvrn->WindSpeed * thisEarthTube.VelocitySQTermCoef));

        thisZoneHB.EAMFL = thisZoneHB.MCPE / AirSpecHeat;
        if (thisEarthTube.FanEfficiency > 0.0) {
            thisEarthTube.FanPower = thisZoneHB.EAMFL * thisEarthTube.FanPressure / (thisEarthTube.FanEfficiency * AirDensity);
        }

        AverPipeAirVel = EVF / Constant::Pi / pow_2(thisEarthTube.r1);
        AirMassFlowRate = EVF * AirDensity;

        if (thisEarthTube.ModelType == EarthTubeModelType::Basic) {
            // Calculation of Ground Temperature at Depth z at time t for Basic model
            GroundTempt = thisEarthTube.calcUndisturbedGroundTemperature(state, thisEarthTube.z);
            thisEarthTube.GroundTempt = GroundTempt;
        }

        // Calculation of Convective Heat Transfer Coefficient at Inner Pipe Surface
        AirThermCond = 0.02442 + 0.6992 * outTdb / 10000.0;
        AirKinemVisco = (0.1335 + 0.000925 * outTdb) / 10000.0;
        AirThermDiffus = (0.0014 * outTdb + 0.1872) / 10000.0;
        Re = 2.0 * thisEarthTube.r1 * AverPipeAirVel / AirKinemVisco;
        Pr = AirKinemVisco / AirThermDiffus;
        if (Re <= 2300.0) {
            Nu = 3.66;
        } else if (Re <= 4000.0) {
            fa = std::pow(1.58 * std::log(Re) - 3.28, -2);
            Process1 = (fa / 2.0) * (Re - 1000.0) * Pr / (1.0 + 12.7 * std::sqrt(fa / 2.0) * (std::pow(Pr, 2.0 / 3.0) - 1.0));
            Nu = (Process1 - 3.66) / (1700.0) * Re + (4000.0 * 3.66 - 2300.0 * Process1) / 1700.0;
        } else {
            fa = std::pow(1.58 * std::log(Re) - 3.28, -2);
            Nu = (fa / 2.0) * (Re - 1000.0) * Pr / (1.0 + 12.7 * std::sqrt(fa / 2.0) * (std::pow(Pr, 2.0 / 3.0) - 1.0));
        }
        PipeHeatTransCoef = Nu * AirThermCond / 2.0 / thisEarthTube.r1;

        // Calculation of Thermal Resistance and Overall Heat Transfer Coefficient
        Rc = 1.0 / 2.0 / Constant::Pi / thisEarthTube.r1 / PipeHeatTransCoef;
        Rp = std::log((thisEarthTube.r1 + thisEarthTube.r2) / thisEarthTube.r1) / 2.0 / Constant::Pi / thisEarthTube.PipeThermCond;
        if (thisEarthTube.r3 > 0.0) {
            Rs = std::log((thisEarthTube.r1 + thisEarthTube.r2 + thisEarthTube.r3) / (thisEarthTube.r1 + thisEarthTube.r2)) / 2.0 / Constant::Pi /
                 thisEarthTube.SoilThermCond;
        } else { // for the Vertical solution .r3 was reset to zero for this
            Rs = 0.0;
        }
        Rt = Rc + Rp + Rs;
        OverallHeatTransCoef = 1.0 / Rt;

        switch (thisEarthTube.ModelType) {
        case EarthTubeModelType::Vertical: {
            // First calculate term that will need to be added at the diagonal for flow and then solve the matrix for new temperatures
            Real64 eff; // effectiveness
            if (AirMassFlowRate > 0.0) {
                // Calculate the NTU parameter: NTU = UA/[(Mdot*Cp)min] where Mdot*Cp is for the air side
                // where: U = OverallHeatTransCoef
                //        A = 2*Pi*r1*TubeLength
                Real64 NTU =
                    OverallHeatTransCoef * 2.0 * Constant::Pi * thisEarthTube.r1 * thisEarthTube.PipeLength / (AirMassFlowRate * AirSpecHeat);

                // Effectiveness is 1 - e(-NTU)
                Real64 constexpr maxExpPower(50.0); // Maximum power after which EXP argument would be zero for DP variables
                if (NTU > maxExpPower) {
                    eff = 1.0;
                } else {
                    eff = 1.0 - std::exp(-NTU);
                }
            } else { // if no flow, then eff is zero
                eff = 0.0;
            }

            Real64 airFlowTerm = AirMassFlowRate * AirSpecHeat * eff * thisEarthTube.airFlowCoeff;
            thisEarthTube.calcVerticalEarthTube(state, airFlowTerm);

            int nodeET = state.dataEarthTube->EarthTubePars(thisEarthTube.vertParametersPtr).numNodesAbove;
            if (eff <= 0.0) { // no flow--air temperature leaving earth tube is the same as what went in
                thisEarthTube.InsideAirTemp = outTdb;
            } else if (eff >= 1.0) { // effectiveness is one so leaving temperature is the same as the ground node temperatre
                thisEarthTube.InsideAirTemp = thisEarthTube.tCurrent[nodeET];
            } else { // the temperature is between the inlet and ground temperatures
                thisEarthTube.InsideAirTemp = outTdb - eff * (outTdb - thisEarthTube.tCurrent[nodeET]);
            }

        } break;
        case EarthTubeModelType::Basic: { // Basic model
            if (AirMassFlowRate * AirSpecHeat == 0.0) {
                thisEarthTube.InsideAirTemp = GroundTempt;

            } else {

                // Calculation of Pipe Outlet Air Temperature
                if (outTdb > GroundTempt) {
                    Process1 =
                        (std::log(std::abs(outTdb - GroundTempt)) * AirMassFlowRate * AirSpecHeat - OverallHeatTransCoef * thisEarthTube.PipeLength) /
                        (AirMassFlowRate * AirSpecHeat);
                    thisEarthTube.InsideAirTemp = std::exp(Process1) + GroundTempt;
                } else if (outTdb == GroundTempt) {
                    thisEarthTube.InsideAirTemp = GroundTempt;
                } else {
                    Process1 =
                        (std::log(std::abs(outTdb - GroundTempt)) * AirMassFlowRate * AirSpecHeat - OverallHeatTransCoef * thisEarthTube.PipeLength) /
                        (AirMassFlowRate * AirSpecHeat);
                    thisEarthTube.InsideAirTemp = GroundTempt - std::exp(Process1);
                }
            }
        } break;
        default: { // should never get here
            assert(false);
        } break;
        }

        thisEarthTube.CalcEarthTubeHumRat(state, NZ);
    }
}

Real64 EarthTubeData::calcUndisturbedGroundTemperature(EnergyPlusData &state, Real64 depth)
{
    return this->AverSoilSurTemp -
           this->ApmlSoilSurTemp * std::exp(-depth * std::sqrt(Constant::Pi / 365.0 / this->SoilThermDiff)) *
               std::cos(2.0 * Constant::Pi / 365.0 *
                        (state.dataEnvrn->DayOfYear - this->SoilSurPhaseConst - depth / 2.0 * std::sqrt(365.0 / Constant::Pi / this->SoilThermDiff)));
}

void EarthTubeData::calcVerticalEarthTube(EnergyPlusData &state, Real64 airFlowTerm)
{
    // Perform matrix calculations to model the earth tube using the vertical solution.
    // At this point, temperatures have already been shifted so tLast is correct and
    // undisturbed ground temperature have also been calculated.  We need to assign/update
    // vectors of coefficients and then perform the Thomas algorithm.
    // Note that airFlowTerm is mdot_a*cp_a*eff*deltat/rho_soil/cp_soil/nodethickness_et/width/length

    int nodeET = state.dataEarthTube->EarthTubePars(this->vertParametersPtr).numNodesAbove;
    int nodeLast = this->totNodes - 1; // minus one because c++ arrays start at 0

    // First, calculate cPrime in the forward sweep.
    // If airFlowTerm is zero, there is no flow so we can use can use cPrime0 for cPrime.
    if (airFlowTerm <= 0.0) {
        for (int nodeNum = 0; nodeNum <= nodeLast; ++nodeNum) {
            this->cPrime[nodeNum] = this->cPrime0[nodeNum];
        }
    } else { // there is positive flow so calculate cPrime
        this->cPrime[0] = this->cCoeff[0] / this->bCoeff[0];
        for (int nodeNum = 1; nodeNum <= nodeLast; ++nodeNum) {
            Real64 addTerm = 0.0;
            if (nodeNum == nodeET) {
                addTerm = airFlowTerm;
            }
            this->cPrime[nodeNum] = this->cCoeff[nodeNum] / (this->bCoeff[nodeNum] + addTerm - this->aCoeff[nodeNum] * this->cPrime[nodeNum - 1]);
        }
    }

    // Second, set-up dCoeff
    this->dCoeff[0] = this->tLast[0] + this->dMult0 * this->tUpperBound;
    for (int nodeNum = 1; nodeNum <= nodeLast - 1; ++nodeNum) {
        if (nodeNum != nodeET) {
            this->dCoeff[nodeNum] = this->tLast[nodeNum];
        } else {
            this->dCoeff[nodeNum] = this->tLast[nodeNum] + airFlowTerm * state.dataEnvrn->OutDryBulbTemp;
        }
    }
    this->dCoeff[nodeLast] = this->tLast[nodeLast] + this->dMultN * this->tLowerBound;

    // Third, calculate dPrime in the forward sweep.
    this->dPrime[0] = this->dCoeff[0] / this->bCoeff[0];
    for (int nodeNum = 1; nodeNum <= nodeLast; ++nodeNum) {
        Real64 addTerm = 0.0;
        if (nodeNum == nodeET) {
            addTerm = airFlowTerm;
        }
        this->dPrime[nodeNum] = (this->dCoeff[nodeNum] - this->aCoeff[nodeNum] * this->dPrime[nodeNum - 1]) /
                                (this->bCoeff[nodeNum] + addTerm - this->aCoeff[nodeNum] * this->cPrime[nodeNum - 1]);
    }

    // Finally, obtain the solution (tCurrent) by back substitution.
    this->tCurrent[nodeLast] = this->dPrime[nodeLast];
    for (int nodeNum = nodeLast - 1; nodeNum >= 0; --nodeNum) {
        this->tCurrent[nodeNum] = this->dPrime[nodeNum] - this->cPrime[nodeNum] * this->tCurrent[nodeNum + 1];
    }
}

void EarthTubeData::CalcEarthTubeHumRat(EnergyPlusData &state, int const NZ)
{ // Zone number (index)

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Kwang Ho Lee
    //       DATE WRITTEN   November 2005
    //       MODIFIED       Rick Strand, June 2017 (made this a separate subroutine)

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine determines the leaving humidity ratio for the EarthTube
    // and calculates parameters associated with humidity ratio.

    Real64 InsideDewPointTemp = Psychrometrics::PsyTdpFnWPb(state, state.dataEnvrn->OutHumRat, state.dataEnvrn->OutBaroPress);
    Real64 InsideHumRat = 0.0;
    auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(NZ);

    if (this->InsideAirTemp >= InsideDewPointTemp) {
        InsideHumRat = state.dataEnvrn->OutHumRat;
        Real64 const InsideEnthalpy = Psychrometrics::PsyHFnTdbW(this->InsideAirTemp, state.dataEnvrn->OutHumRat);
        // Intake fans will add some heat to the air, raising the temperature for an intake fan...
        if (this->FanType == Ventilation::Intake) {
            Real64 OutletAirEnthalpy;
            if (thisZoneHB.EAMFL == 0.0) {
                OutletAirEnthalpy = InsideEnthalpy;
            } else {
                OutletAirEnthalpy = InsideEnthalpy + this->FanPower / thisZoneHB.EAMFL;
            }
            this->AirTemp = Psychrometrics::PsyTdbFnHW(OutletAirEnthalpy, state.dataEnvrn->OutHumRat);
        } else {
            this->AirTemp = this->InsideAirTemp;
        }
        thisZoneHB.MCPTE = thisZoneHB.MCPE * this->AirTemp;

    } else {
        InsideHumRat = Psychrometrics::PsyWFnTdpPb(state, this->InsideAirTemp, state.dataEnvrn->OutBaroPress);
        Real64 const InsideEnthalpy = Psychrometrics::PsyHFnTdbW(this->InsideAirTemp, InsideHumRat);
        // Intake fans will add some heat to the air, raising the temperature for an intake fan...
        if (this->FanType == Ventilation::Intake) {
            Real64 OutletAirEnthalpy;
            if (thisZoneHB.EAMFL == 0.0) {
                OutletAirEnthalpy = InsideEnthalpy;
            } else {
                OutletAirEnthalpy = InsideEnthalpy + this->FanPower / thisZoneHB.EAMFL;
            }
            this->AirTemp = Psychrometrics::PsyTdbFnHW(OutletAirEnthalpy, InsideHumRat);
        } else {
            this->AirTemp = this->InsideAirTemp;
        }
        thisZoneHB.MCPTE = thisZoneHB.MCPE * this->AirTemp;
    }

    this->HumRat = InsideHumRat;
    this->WetBulbTemp = Psychrometrics::PsyTwbFnTdbWPb(state, this->InsideAirTemp, InsideHumRat, state.dataEnvrn->OutBaroPress);
    thisZoneHB.EAMFLxHumRat = thisZoneHB.EAMFL * InsideHumRat;
}

void ReportEarthTube(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Kwang Ho Lee
    //       DATE WRITTEN   November 2005
    //       MODIFIED       B. Griffith April 2010 added output reports

    // PURPOSE OF THIS SUBROUTINE: This subroutine fills remaining report variables.

    Real64 const ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;

    for (int ZoneLoop = 1; ZoneLoop <= state.dataGlobal->NumOfZones; ++ZoneLoop) { // Start of zone loads report variable update loop ...
        auto &thisZone = state.dataEarthTube->ZnRptET(ZoneLoop);
        auto const &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneLoop);

        // Break the infiltration load into heat gain and loss components.
        Real64 const AirDensity =
            Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, state.dataEnvrn->OutDryBulbTemp, state.dataEnvrn->OutHumRat);
        Real64 const CpAir = Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat);
        thisZone.EarthTubeVolume = (thisZoneHB.MCPE / CpAir / AirDensity) * ReportingConstant;
        thisZone.EarthTubeMass = (thisZoneHB.MCPE / CpAir) * ReportingConstant;
        thisZone.EarthTubeVolFlowRate = thisZoneHB.MCPE / CpAir / AirDensity;
        thisZone.EarthTubeVolFlowRateStd = thisZoneHB.MCPE / CpAir / state.dataEnvrn->StdRhoAir;
        thisZone.EarthTubeMassFlowRate = thisZoneHB.MCPE / CpAir;
        thisZone.EarthTubeWaterMassFlowRate = thisZoneHB.EAMFLxHumRat;

        thisZone.EarthTubeFanElec = 0.0;
        thisZone.EarthTubeAirTemp = 0.0;
        for (auto const &thisEarthTube : state.dataEarthTube->EarthTubeSys) {
            if (thisEarthTube.ZonePtr == ZoneLoop) {
                thisZone.EarthTubeFanElec = thisEarthTube.FanPower * ReportingConstant;
                thisZone.EarthTubeFanElecPower = thisEarthTube.FanPower;

                // Break the EarthTube load into heat gain and loss components.
                if (thisZoneHB.ZT > thisEarthTube.AirTemp) {
                    thisZone.EarthTubeHeatLoss = thisZoneHB.MCPE * (thisZoneHB.ZT - thisEarthTube.AirTemp) * ReportingConstant;
                    thisZone.EarthTubeHeatLossRate = thisZoneHB.MCPE * (thisZoneHB.ZT - thisEarthTube.AirTemp);
                    thisZone.EarthTubeHeatGain = 0.0;
                    thisZone.EarthTubeHeatGainRate = 0.0;
                } else {
                    thisZone.EarthTubeHeatGain = thisZoneHB.MCPE * (thisEarthTube.AirTemp - thisZoneHB.ZT) * ReportingConstant;
                    thisZone.EarthTubeHeatGainRate = thisZoneHB.MCPE * (thisEarthTube.AirTemp - thisZoneHB.ZT);
                    thisZone.EarthTubeHeatLoss = 0.0;
                    thisZone.EarthTubeHeatLossRate = 0.0;
                }

                thisZone.EarthTubeAirTemp = thisEarthTube.AirTemp;
                thisZone.EarthTubeWetBulbTemp = thisEarthTube.WetBulbTemp;
                thisZone.EarthTubeHumRat = thisEarthTube.HumRat;
                thisZone.EarthTubeOATreatmentPower = thisZoneHB.MCPE * (thisEarthTube.AirTemp - state.dataEnvrn->OutDryBulbTemp);
                break; // DO loop
            }
        }

    } // ... end of zone loads report variable update loop.
}

} // namespace EnergyPlus::EarthTube
