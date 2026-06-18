// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
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
#include <cassert>
#include <cmath>
#include <format>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/PackagedThermalStorageCoil.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterManager.hh>
#include <EnergyPlus/WaterThermalTanks.hh>

namespace EnergyPlus::PackagedThermalStorageCoil {

// Module containing the routines dealing with the packaged thermal storage cooling

// MODULE INFORMATION:
//       AUTHOR         Brent Griffith
//       DATE WRITTEN   April 2013
//       MODIFIED       na
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// encapsulate the data and algorithms for modeling packaged thermals storage cooling coils

// Using/Aliasing
using namespace Psychrometrics;
using namespace Curve;

constexpr std::array<std::string_view, static_cast<int>(PTSCCtrlType::Num)> modeControlStrings = {"SCHEDULEDMODES", "EMSCONTROLLED"};
constexpr std::array<std::string_view, static_cast<int>(MediaType::Num)> mediaStrings = {"WATER", "USERDEFINEDFLUIDTYPE", "ICE"};
constexpr std::array<std::string_view, static_cast<int>(TESCondenserType::Num)> condenserTypesUC = {"AIRCOOLED", "EVAPORATIVELYCOOLED"};
constexpr Real64 gigaJoulesToJoules = 1.e+09;

void SimTESCoil(EnergyPlusData &state,
                std::string_view CompName, // name of the fan coil unit
                int &CompIndex,
                HVAC::FanOp const fanOp, // allows parent object to control fan mode
                PTSCOperatingMode &TESOpMode,
                ObjexxFCL::Optional<Real64 const> PartLoadRatio // part load ratio (for single speed cycling unit)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         <author>
    //       DATE WRITTEN   <date_written>
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    if (state.dataPackagedThermalStorageCoil->GetTESInputFlag) {
        GetTESCoilInput(state);
        state.dataPackagedThermalStorageCoil->GetTESInputFlag = false;
    }

    int TESCoilNum = 0;
    if (CompIndex == 0) {
        TESCoilNum = Util::FindItemInList(CompName, state.dataPackagedThermalStorageCoil->TESCoil);
        if (TESCoilNum == 0) {
            ShowFatalError(state, std::format("Thermal Energy Storage Cooling Coil not found={}", CompName));
        }
        CompIndex = TESCoilNum;
    } else {
        TESCoilNum = CompIndex;
        if (TESCoilNum > state.dataPackagedThermalStorageCoil->NumTESCoils || TESCoilNum < 1) {
            ShowFatalError(
                state,
                std::format("SimTESCoil: Invalid CompIndex passed={}, Number of Thermal Energy Storage Cooling Coil Coils={}, Coil name={}",
                            TESCoilNum,
                            state.dataPackagedThermalStorageCoil->NumTESCoils,
                            CompName));
        }
        if (state.dataPackagedThermalStorageCoil->CheckEquipName(TESCoilNum)) {
            if (!CompName.empty() && CompName != state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).Name) {
                ShowFatalError(state,
                               std::format("SimTESCoil: Invalid CompIndex passed={}, Coil name={}, stored Coil Name for that index={}",
                                           TESCoilNum,
                                           CompName,
                                           state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).Name));
            }
            state.dataPackagedThermalStorageCoil->CheckEquipName(TESCoilNum) = false;
        }
    }

    TESOpMode = PTSCOperatingMode::CoolingOnly;

    InitTESCoil(state, TESCoilNum);

    TESOpMode = state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).CurControlMode;
    switch (TESOpMode) {
    case PTSCOperatingMode::Off:
        CalcTESCoilOffMode(state, TESCoilNum);
        break;
    case PTSCOperatingMode::CoolingOnly:
        CalcTESCoilCoolingOnlyMode(state, TESCoilNum, fanOp, PartLoadRatio);
        break;
    case PTSCOperatingMode::CoolingAndCharge:
        CalcTESCoilCoolingAndChargeMode(state, TESCoilNum, fanOp, PartLoadRatio);
        break;
    case PTSCOperatingMode::CoolingAndDischarge:
        CalcTESCoilCoolingAndDischargeMode(state, TESCoilNum, fanOp, PartLoadRatio);
        break;
    case PTSCOperatingMode::ChargeOnly:
        CalcTESCoilChargeOnlyMode(state, TESCoilNum);
        break;
    case PTSCOperatingMode::DischargeOnly:
        CalcTESCoilDischargeOnlyMode(state, TESCoilNum, PartLoadRatio);
        break;
    default:
        assert(false);
    }
}

void GetTESCoilInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         <author>
    //       DATE WRITTEN   <date_written>
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // Using/Aliasing
    using DataZoneEquipment::FindControlledZoneIndexFromSystemNodeNumberForZone;
    using GlobalNames::VerifyUniqueCoilName;
    using Node::GetOnlySingleNode;
    using Node::TestCompSet;
    using WaterManager::SetupTankDemandComponent;
    using WaterManager::SetupTankSupplyComponent;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetTESCoilInput: "); // include trailing blank space
    static constexpr std::string_view routineName = "GetTESCoilInput";

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumAlphas = 0;       // Number of alphas in input
    int NumNumbers = 0;      // Number of numeric items in input
    int IOStatus = -1;       // Input status returned from GetObjectItem
    bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine

    auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
    cCurrentModuleObject = "Coil:Cooling:DX:SingleSpeed:ThermalStorage";
    state.dataPackagedThermalStorageCoil->NumTESCoils = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

    state.dataPackagedThermalStorageCoil->TESCoil.allocate(state.dataPackagedThermalStorageCoil->NumTESCoils);
    state.dataPackagedThermalStorageCoil->CheckEquipName.dimension(state.dataPackagedThermalStorageCoil->NumTESCoils, true);

    for (int item = 1; item <= state.dataPackagedThermalStorageCoil->NumTESCoils; ++item) {

        auto &thisTESCoil = state.dataPackagedThermalStorageCoil->TESCoil(item);

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 item,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNumbers,
                                                                 IOStatus,
                                                                 state.dataIPShortCut->lNumericFieldBlanks,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

        // ErrorsFound will be set to True if problem was found, left untouched otherwise
        VerifyUniqueCoilName(state, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1), ErrorsFound, cCurrentModuleObject + " Name");

        thisTESCoil.Name = state.dataIPShortCut->cAlphaArgs(1);
        if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
            thisTESCoil.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisTESCoil.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
            ErrorsFound = true;
        }

        thisTESCoil.ModeControlType = static_cast<PTSCCtrlType>(getEnumValue(modeControlStrings, state.dataIPShortCut->cAlphaArgs(3)));
        if (thisTESCoil.ModeControlType == PTSCCtrlType::Invalid) {
            ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
            ShowContinueError(state, std::format("...{}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(3), state.dataIPShortCut->cAlphaArgs(3)));
            ShowContinueError(state, "Available choices are ScheduledModes or EMSControlled");
            ErrorsFound = true;
        }

        if (thisTESCoil.ModeControlType == PTSCCtrlType::ScheduledOpModes) {
            if (state.dataIPShortCut->lAlphaFieldBlanks(4)) {
                ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(4));
                ErrorsFound = true;
            } else if ((thisTESCoil.controlModeSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(4), state.dataIPShortCut->cAlphaArgs(4));
                ErrorsFound = true;
            }
        }

        thisTESCoil.StorageMedia = static_cast<MediaType>(getEnumValue(mediaStrings, state.dataIPShortCut->cAlphaArgs(5)));
        switch (thisTESCoil.StorageMedia) {
        case MediaType::Ice:
        case MediaType::UserDefindFluid:
            // nothing else to do for now
            break;
        case MediaType::Water:
            thisTESCoil.StorageFluidName = "WATER";
            thisTESCoil.glycol = Fluid::GetWater(state);
            break;
        default:
            ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
            ShowContinueError(state, std::format("...{}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(5), state.dataIPShortCut->cAlphaArgs(5)));
            ShowContinueError(state, "Available choices are Ice, Water, or UserDefindedFluidType");
            ErrorsFound = true;
        }

        thisTESCoil.StorageFluidName = state.dataIPShortCut->cAlphaArgs(6);
        if (Util::SameString(state.dataIPShortCut->cAlphaArgs(5), "USERDEFINEDFLUIDTYPE")) {
            if (!state.dataIPShortCut->lAlphaFieldBlanks(6)) {
                ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(6));
                ErrorsFound = true;
            } else if ((thisTESCoil.glycol = Fluid::GetGlycol(state, state.dataIPShortCut->cAlphaArgs(6))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(6), state.dataIPShortCut->cAlphaArgs(6));
                ErrorsFound = true;
            }
        }

        switch (thisTESCoil.StorageMedia) {
        case MediaType::Water:
        case MediaType::UserDefindFluid:
            if (!state.dataIPShortCut->lNumericFieldBlanks(1)) {
                thisTESCoil.FluidStorageVolume = state.dataIPShortCut->rNumericArgs(1);
            } else {
                ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                ShowContinueError(state, std::format("{} cannot be blank for Water storage type", state.dataIPShortCut->cNumericFieldNames(1)));
                ShowContinueError(state, "Enter fluid storage tank volume in m3/s.");
                ErrorsFound = true;
            }
            break;
        case MediaType::Ice:
            if (!state.dataIPShortCut->lNumericFieldBlanks(2)) {
                if (state.dataIPShortCut->rNumericArgs(2) == Constant::AutoCalculate) {
                    thisTESCoil.IceStorageCapacity = state.dataIPShortCut->rNumericArgs(2);
                } else {
                    thisTESCoil.IceStorageCapacity =
                        state.dataIPShortCut->rNumericArgs(2) * gigaJoulesToJoules; // input in giga joules, used as joules internally
                }
            } else if (state.dataIPShortCut->lNumericFieldBlanks(2)) {
                ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                ShowContinueError(state, std::format("{} cannot be blank for Ice storage type", state.dataIPShortCut->cNumericFieldNames(2)));
                ShowContinueError(state, "Enter ice storage tank capacity in GJ.");
                ErrorsFound = true;
            }
            break;
        default:
            // nothing
            break;
        }

        thisTESCoil.StorageCapacitySizingFactor = state.dataIPShortCut->rNumericArgs(3);

        thisTESCoil.StorageAmbientNodeNum = GetOnlySingleNode(state,
                                                              state.dataIPShortCut->cAlphaArgs(7),
                                                              ErrorsFound,
                                                              Node::ConnectionObjectType::CoilCoolingDXSingleSpeedThermalStorage,
                                                              state.dataIPShortCut->cAlphaArgs(1),
                                                              Node::FluidType::Air,
                                                              Node::ConnectionType::Sensor,
                                                              Node::CompFluidStream::Primary,
                                                              Node::ObjectIsNotParent);

        int const ZoneIndexTrial = FindControlledZoneIndexFromSystemNodeNumberForZone(state, thisTESCoil.StorageAmbientNodeNum);
        if (ZoneIndexTrial > 0) { // tank is inside a zone so setup internal gains
            SetupZoneInternalGain(
                state, ZoneIndexTrial, thisTESCoil.Name, DataHeatBalance::IntGainType::PackagedTESCoilTank, &thisTESCoil.QdotAmbient);
        }

        thisTESCoil.StorageUA = state.dataIPShortCut->rNumericArgs(4);
        thisTESCoil.RatedFluidTankTemp = state.dataIPShortCut->rNumericArgs(5);
        thisTESCoil.RatedEvapAirVolFlowRate = state.dataIPShortCut->rNumericArgs(6);

        thisTESCoil.EvapAirInletNodeNum = GetOnlySingleNode(state,
                                                            state.dataIPShortCut->cAlphaArgs(8),
                                                            ErrorsFound,
                                                            Node::ConnectionObjectType::CoilCoolingDXSingleSpeedThermalStorage,
                                                            state.dataIPShortCut->cAlphaArgs(1),
                                                            Node::FluidType::Air,
                                                            Node::ConnectionType::Inlet,
                                                            Node::CompFluidStream::Primary,
                                                            Node::ObjectIsNotParent);
        thisTESCoil.EvapAirOutletNodeNum = GetOnlySingleNode(state,
                                                             state.dataIPShortCut->cAlphaArgs(9),
                                                             ErrorsFound,
                                                             Node::ConnectionObjectType::CoilCoolingDXSingleSpeedThermalStorage,
                                                             state.dataIPShortCut->cAlphaArgs(1),
                                                             Node::FluidType::Air,
                                                             Node::ConnectionType::Outlet,
                                                             Node::CompFluidStream::Primary,
                                                             Node::ObjectIsNotParent);
        TestCompSet(state,
                    cCurrentModuleObject,
                    state.dataIPShortCut->cAlphaArgs(1),
                    state.dataIPShortCut->cAlphaArgs(8),
                    state.dataIPShortCut->cAlphaArgs(9),
                    "Air Nodes");

        BooleanSwitch const answer = getYesNoValue(state.dataIPShortCut->cAlphaArgs(10));
        switch (answer) {
        case BooleanSwitch::Yes:
        case BooleanSwitch::No:
            thisTESCoil.CoolingOnlyModeIsAvailable = static_cast<bool>(answer);
            break;
        default:
            thisTESCoil.CoolingOnlyModeIsAvailable = false;
            ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
            ShowContinueError(state, std::format("...{}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(10), state.dataIPShortCut->cAlphaArgs(10)));
            ShowContinueError(state, "Available choices are Yes or No.");
            ErrorsFound = true;
        }

        thisTESCoil.CoolingOnlyRatedTotCap = state.dataIPShortCut->rNumericArgs(7);
        if (thisTESCoil.CoolingOnlyModeIsAvailable) { // get input data for this mode

            thisTESCoil.CoolingOnlyRatedSHR = state.dataIPShortCut->rNumericArgs(8);
            thisTESCoil.CoolingOnlyRatedCOP = state.dataIPShortCut->rNumericArgs(9);

            thisTESCoil.CoolingOnlyCapFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(11));
            if (thisTESCoil.CoolingOnlyCapFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(11)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(11)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(11), state.dataIPShortCut->cAlphaArgs(11)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingOnlyCapFTempCurve,        // Curve index
                                                                 {2},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(11)); // Field Name
            }

            thisTESCoil.CoolingOnlyCapFFlowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(12));
            if (thisTESCoil.CoolingOnlyCapFFlowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(12)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(12)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(12), state.dataIPShortCut->cAlphaArgs(12)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingOnlyCapFFlowCurve,        // Curve index
                                                                 {1},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(12)); // Field Name
            }

            thisTESCoil.CoolingOnlyEIRFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(13));
            if (thisTESCoil.CoolingOnlyEIRFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(13)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(13)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(13), state.dataIPShortCut->cAlphaArgs(13)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingOnlyEIRFTempCurve,        // Curve index
                                                                 {2},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(13)); // Field Name
            }

            thisTESCoil.CoolingOnlyEIRFFlowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(14));
            if (thisTESCoil.CoolingOnlyEIRFFlowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(14)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(14)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(14), state.dataIPShortCut->cAlphaArgs(14)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingOnlyEIRFFlowCurve,        // Curve index
                                                                 {1},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(14)); // Field Name
            }

            thisTESCoil.CoolingOnlyPLFFPLRCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(15));
            if (thisTESCoil.CoolingOnlyPLFFPLRCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(15)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(15)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(15), state.dataIPShortCut->cAlphaArgs(15)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingOnlyPLFFPLRCurve,         // Curve index
                                                                 {1},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(15)); // Field Name
            }

            thisTESCoil.CoolingOnlySHRFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(16));
            if (thisTESCoil.CoolingOnlySHRFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(16)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(16)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(16), state.dataIPShortCut->cAlphaArgs(16)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingOnlySHRFTempCurve,        // Curve index
                                                                 {2},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(16)); // Field Name
            }

            thisTESCoil.CoolingOnlySHRFFlowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(17));
            if (thisTESCoil.CoolingOnlySHRFFlowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(17)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(17)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(17), state.dataIPShortCut->cAlphaArgs(17)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingOnlySHRFFlowCurve,        // Curve index
                                                                 {1},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(17)); // Field Name
            }
        }

        BooleanSwitch const answer2 = getYesNoValue(state.dataIPShortCut->cAlphaArgs(18));
        switch (answer2) {
        case BooleanSwitch::Yes:
        case BooleanSwitch::No:
            thisTESCoil.CoolingAndChargeModeAvailable = static_cast<bool>(answer2);
            break;
        default:
            thisTESCoil.CoolingAndChargeModeAvailable = false;
            ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
            ShowContinueError(state, std::format("...{}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(18), state.dataIPShortCut->cAlphaArgs(18)));
            ShowContinueError(state, "Available choices are Yes or No.");
            ErrorsFound = true;
        }

        if (thisTESCoil.CoolingAndChargeModeAvailable) {

            thisTESCoil.CoolingAndChargeRatedTotCap = state.dataIPShortCut->rNumericArgs(10); // gross total evaporator cooling capacity [W]
            thisTESCoil.CoolingAndChargeRatedTotCapSizingFactor =
                state.dataIPShortCut->rNumericArgs(11); // sizing factor for gross total evaporator [ ]
            thisTESCoil.CoolingAndChargeRatedChargeCap =
                state.dataIPShortCut->rNumericArgs(12); // net storage charging capacity at rating conditions [W]
            thisTESCoil.CoolingAndChargeRatedChargeCapSizingFactor =
                state.dataIPShortCut->rNumericArgs(13);                                            // sizing factor for charging capacity [ ]
            thisTESCoil.CoolingAndChargeRatedSHR = state.dataIPShortCut->rNumericArgs(14);         // Sensible heat ratio (sens cap/total cap)  [W/W]
            thisTESCoil.CoolingAndChargeCoolingRatedCOP = state.dataIPShortCut->rNumericArgs(15);  // Coefficient of performance , for cooling [W/W]
            thisTESCoil.CoolingAndChargeChargingRatedCOP = state.dataIPShortCut->rNumericArgs(16); // Coefficient of performance , for charging [W/W]

            thisTESCoil.CoolingAndChargeCoolingCapFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(19));
            if (thisTESCoil.CoolingAndChargeCoolingCapFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(19)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(19)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(19), state.dataIPShortCut->cAlphaArgs(19)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndChargeCoolingCapFTempCurve, // Curve index
                                                                 {3},                                              // Valid dimensions
                                                                 RoutineName,                                      // Routine name
                                                                 cCurrentModuleObject,                             // Object Type
                                                                 thisTESCoil.Name,                                 // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(19));      // Field Name
            }

            thisTESCoil.CoolingAndChargeCoolingCapFFlowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(20));
            if (thisTESCoil.CoolingAndChargeCoolingCapFFlowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(20)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(20)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(20), state.dataIPShortCut->cAlphaArgs(20)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndChargeCoolingCapFFlowCurve, // Curve index
                                                                 {1},                                              // Valid dimensions
                                                                 RoutineName,                                      // Routine name
                                                                 cCurrentModuleObject,                             // Object Type
                                                                 thisTESCoil.Name,                                 // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(20));      // Field Name
            }
            thisTESCoil.CoolingAndChargeCoolingEIRFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(21));
            if (thisTESCoil.CoolingAndChargeCoolingEIRFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(21)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(21)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(21), state.dataIPShortCut->cAlphaArgs(21)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndChargeCoolingEIRFTempCurve, // Curve index
                                                                 {3},                                              // Valid dimensions
                                                                 RoutineName,                                      // Routine name
                                                                 cCurrentModuleObject,                             // Object Type
                                                                 thisTESCoil.Name,                                 // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(21));      // Field Name
            }

            thisTESCoil.CoolingAndChargeCoolingEIRFFlowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(22));
            if (thisTESCoil.CoolingAndChargeCoolingEIRFFlowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(22)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(22)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(22), state.dataIPShortCut->cAlphaArgs(22)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndChargeCoolingEIRFFlowCurve, // Curve index
                                                                 {1},                                              // Valid dimensions
                                                                 RoutineName,                                      // Routine name
                                                                 cCurrentModuleObject,                             // Object Type
                                                                 thisTESCoil.Name,                                 // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(22));      // Field Name
            }

            thisTESCoil.CoolingAndChargeCoolingPLFFPLRCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(23));
            if (thisTESCoil.CoolingAndChargeCoolingPLFFPLRCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(23)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(23)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(23), state.dataIPShortCut->cAlphaArgs(23)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndChargeCoolingPLFFPLRCurve, // Curve index
                                                                 {1},                                             // Valid dimensions
                                                                 RoutineName,                                     // Routine name
                                                                 cCurrentModuleObject,                            // Object Type
                                                                 thisTESCoil.Name,                                // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(23));     // Field Name
            }

            thisTESCoil.CoolingAndChargeChargingCapFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(24));
            if (thisTESCoil.CoolingAndChargeChargingCapFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(24)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(24)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(24), state.dataIPShortCut->cAlphaArgs(24)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndChargeChargingCapFTempCurve, // Curve index
                                                                 {3},                                               // Valid dimensions
                                                                 RoutineName,                                       // Routine name
                                                                 cCurrentModuleObject,                              // Object Type
                                                                 thisTESCoil.Name,                                  // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(24));       // Field Name
            }

            thisTESCoil.CoolingAndChargeChargingCapFEvapPLRCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(25));
            if (thisTESCoil.CoolingAndChargeChargingCapFEvapPLRCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(25)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(25)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(25), state.dataIPShortCut->cAlphaArgs(25)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndChargeChargingCapFEvapPLRCurve, // Curve index
                                                                 {1},                                                  // Valid dimensions
                                                                 RoutineName,                                          // Routine name
                                                                 cCurrentModuleObject,                                 // Object Type
                                                                 thisTESCoil.Name,                                     // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(25));          // Field Name
            }

            thisTESCoil.CoolingAndChargeChargingEIRFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(26));
            if (thisTESCoil.CoolingAndChargeChargingEIRFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(26)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(26)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(26), state.dataIPShortCut->cAlphaArgs(26)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndChargeChargingEIRFTempCurve, // Curve index
                                                                 {3},                                               // Valid dimensions
                                                                 RoutineName,                                       // Routine name
                                                                 cCurrentModuleObject,                              // Object Type
                                                                 thisTESCoil.Name,                                  // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(26));       // Field Name
            }

            thisTESCoil.CoolingAndChargeChargingEIRFFLowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(27));
            if (thisTESCoil.CoolingAndChargeChargingEIRFFLowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(27)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(27)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(27), state.dataIPShortCut->cAlphaArgs(27)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndChargeChargingEIRFFLowCurve, // Curve index
                                                                 {1},                                               // Valid dimensions
                                                                 RoutineName,                                       // Routine name
                                                                 cCurrentModuleObject,                              // Object Type
                                                                 thisTESCoil.Name,                                  // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(27));       // Field Name
            }

            thisTESCoil.CoolingAndChargeChargingPLFFPLRCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(28));
            if (thisTESCoil.CoolingAndChargeChargingPLFFPLRCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(28)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(28)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(28), state.dataIPShortCut->cAlphaArgs(28)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndChargeChargingPLFFPLRCurve, // Curve index
                                                                 {1},                                              // Valid dimensions
                                                                 RoutineName,                                      // Routine name
                                                                 cCurrentModuleObject,                             // Object Type
                                                                 thisTESCoil.Name,                                 // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(28));      // Field Name
            }

            thisTESCoil.CoolingAndChargeSHRFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(29));
            if (thisTESCoil.CoolingAndChargeSHRFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(29)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(29)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(29), state.dataIPShortCut->cAlphaArgs(29)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndChargeSHRFTempCurve, // Curve index
                                                                 {2, 3},                                    // Valid dimensions  // MULTIPLECURVEDIMS
                                                                 RoutineName,                               // Routine name
                                                                 cCurrentModuleObject,                      // Object Type
                                                                 thisTESCoil.Name,                          // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(29)); // Field Name
            }

            thisTESCoil.CoolingAndChargeSHRFFlowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(30));
            if (thisTESCoil.CoolingAndChargeSHRFFlowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(30)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(30)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(30), state.dataIPShortCut->cAlphaArgs(30)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndChargeSHRFFlowCurve,   // Curve index
                                                                 {1},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(30)); // Field Name
            }

        } // Cooling and Charge Mode available

        BooleanSwitch answer3 = getYesNoValue(state.dataIPShortCut->cAlphaArgs(31));
        switch (answer3) {
        case BooleanSwitch::Yes:
        case BooleanSwitch::No:
            thisTESCoil.CoolingAndDischargeModeAvailable = static_cast<bool>(answer3);
            break;
        default:
            thisTESCoil.CoolingAndDischargeModeAvailable = false;
            ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
            ShowContinueError(state, std::format("...{}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(31), state.dataIPShortCut->cAlphaArgs(31)));
            ShowContinueError(state, "Available choices are Yes or No.");
            ErrorsFound = true;
        }

        if (thisTESCoil.CoolingAndDischargeModeAvailable) {

            thisTESCoil.CoolingAndDischargeRatedTotCap = state.dataIPShortCut->rNumericArgs(17); // gross total evaporator cooling capacity  [W]
            thisTESCoil.CoolingAndDischargeRatedTotCapSizingFactor =
                state.dataIPShortCut->rNumericArgs(18); // sizing factor gross total cooling capacity []
            thisTESCoil.CoolingAndDischargeRatedDischargeCap = state.dataIPShortCut->rNumericArgs(19); // net storage discharging capacity  [W]
            thisTESCoil.CoolingAndDischargeRatedDischargeCapSizingFactor =
                state.dataIPShortCut->rNumericArgs(20);                                              // sizing factor discharging capacity []
            thisTESCoil.CoolingAndDischargeRatedSHR = state.dataIPShortCut->rNumericArgs(21);        // Sensible heat ratio (sens cap/total cap) [W/W]
            thisTESCoil.CoolingAndDischargeCoolingRatedCOP = state.dataIPShortCut->rNumericArgs(22); // Coefficient of performance , for cooling [W/W]
            thisTESCoil.CoolingAndDischargeDischargingRatedCOP =
                state.dataIPShortCut->rNumericArgs(23); // Coefficient of performance , for charging [W/W]

            thisTESCoil.CoolingAndDischargeCoolingCapFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(32));
            if (thisTESCoil.CoolingAndDischargeCoolingCapFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(32)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(32)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(32), state.dataIPShortCut->cAlphaArgs(32)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndDischargeCoolingCapFTempCurve, // Curve index
                                                                 {3},                                                 // Valid dimensions
                                                                 RoutineName,                                         // Routine name
                                                                 cCurrentModuleObject,                                // Object Type
                                                                 thisTESCoil.Name,                                    // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(32));         // Field Name
            }

            thisTESCoil.CoolingAndDischargeCoolingCapFFlowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(33));
            if (thisTESCoil.CoolingAndDischargeCoolingCapFFlowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(33)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(33)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(33), state.dataIPShortCut->cAlphaArgs(33)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndDischargeCoolingCapFFlowCurve, // Curve index
                                                                 {1},                                                 // Valid dimensions
                                                                 RoutineName,                                         // Routine name
                                                                 cCurrentModuleObject,                                // Object Type
                                                                 thisTESCoil.Name,                                    // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(33));         // Field Name
            }

            thisTESCoil.CoolingAndDischargeCoolingEIRFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(34));
            if (thisTESCoil.CoolingAndDischargeCoolingEIRFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(34)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(34)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(34), state.dataIPShortCut->cAlphaArgs(34)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndDischargeCoolingEIRFTempCurve, // Curve index
                                                                 {3},                                                 // Valid dimensions
                                                                 RoutineName,                                         // Routine name
                                                                 cCurrentModuleObject,                                // Object Type
                                                                 thisTESCoil.Name,                                    // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(34));         // Field Name
            }

            thisTESCoil.CoolingAndDischargeCoolingEIRFFlowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(35));
            if (thisTESCoil.CoolingAndDischargeCoolingEIRFFlowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(35)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(35)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(35), state.dataIPShortCut->cAlphaArgs(35)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndDischargeCoolingEIRFFlowCurve, // Curve index
                                                                 {1},                                                 // Valid dimensions
                                                                 RoutineName,                                         // Routine name
                                                                 cCurrentModuleObject,                                // Object Type
                                                                 thisTESCoil.Name,                                    // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(35));         // Field Name
            }

            thisTESCoil.CoolingAndDischargeCoolingPLFFPLRCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(36));
            if (thisTESCoil.CoolingAndDischargeCoolingPLFFPLRCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(36)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(36)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(36), state.dataIPShortCut->cAlphaArgs(36)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndDischargeCoolingPLFFPLRCurve, // Curve index
                                                                 {1},                                                // Valid dimensions
                                                                 RoutineName,                                        // Routine name
                                                                 cCurrentModuleObject,                               // Object Type
                                                                 thisTESCoil.Name,                                   // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(36));        // Field Name
            }

            thisTESCoil.CoolingAndDischargeDischargingCapFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(37));
            if (thisTESCoil.CoolingAndDischargeDischargingCapFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(37)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(37)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(37), state.dataIPShortCut->cAlphaArgs(37)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndDischargeDischargingCapFTempCurve, // Curve index
                                                                 {3},                                                     // Valid dimensions
                                                                 RoutineName,                                             // Routine name
                                                                 cCurrentModuleObject,                                    // Object Type
                                                                 thisTESCoil.Name,                                        // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(37));             // Field Name
            }

            thisTESCoil.CoolingAndDischargeDischargingCapFFlowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(38));
            if (thisTESCoil.CoolingAndDischargeDischargingCapFFlowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(38)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(38)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(38), state.dataIPShortCut->cAlphaArgs(38)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndDischargeDischargingCapFFlowCurve, // Curve index
                                                                 {1},                                                     // Valid dimensions
                                                                 RoutineName,                                             // Routine name
                                                                 cCurrentModuleObject,                                    // Object Type
                                                                 thisTESCoil.Name,                                        // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(38));             // Field Name
            }

            thisTESCoil.CoolingAndDischargeDischargingCapFEvapPLRCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(39));
            if (thisTESCoil.CoolingAndDischargeDischargingCapFEvapPLRCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(39)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(39)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(39), state.dataIPShortCut->cAlphaArgs(39)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndDischargeDischargingCapFEvapPLRCurve, // Curve index
                                                                 {1},                                                        // Valid dimensions
                                                                 RoutineName,                                                // Routine name
                                                                 cCurrentModuleObject,                                       // Object Type
                                                                 thisTESCoil.Name,                                           // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(39));                // Field Name
            }

            thisTESCoil.CoolingAndDischargeDischargingEIRFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(40));
            if (thisTESCoil.CoolingAndDischargeDischargingEIRFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(40)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(40)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(40), state.dataIPShortCut->cAlphaArgs(40)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndDischargeDischargingEIRFTempCurve, // Curve index
                                                                 {3},                                                     // Valid dimensions
                                                                 RoutineName,                                             // Routine name
                                                                 cCurrentModuleObject,                                    // Object Type
                                                                 thisTESCoil.Name,                                        // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(40));             // Field Name
            }

            thisTESCoil.CoolingAndDischargeDischargingEIRFFLowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(41));
            if (thisTESCoil.CoolingAndDischargeDischargingEIRFFLowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(41)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(41)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(41), state.dataIPShortCut->cAlphaArgs(41)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndDischargeDischargingEIRFFLowCurve, // Curve index
                                                                 {1},                                                     // Valid dimensions
                                                                 RoutineName,                                             // Routine name
                                                                 cCurrentModuleObject,                                    // Object Type
                                                                 thisTESCoil.Name,                                        // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(41));             // Field Name
            }

            thisTESCoil.CoolingAndDischargeDischargingPLFFPLRCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(42));
            if (thisTESCoil.CoolingAndDischargeDischargingPLFFPLRCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(42)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(42)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(42), state.dataIPShortCut->cAlphaArgs(42)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndDischargeDischargingPLFFPLRCurve, // Curve index
                                                                 {1},                                                    // Valid dimensions
                                                                 RoutineName,                                            // Routine name
                                                                 cCurrentModuleObject,                                   // Object Type
                                                                 thisTESCoil.Name,                                       // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(42));            // Field Name
            }

            thisTESCoil.CoolingAndDischargeSHRFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(43));
            if (thisTESCoil.CoolingAndDischargeSHRFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(43)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(43)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(43), state.dataIPShortCut->cAlphaArgs(43)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndDischargeSHRFTempCurve, // Curve index
                                                                 {2, 3},               // Valid dimensions  // MULTIPLECURVEDIMS
                                                                 RoutineName,          // Routine name
                                                                 cCurrentModuleObject, // Object Type
                                                                 thisTESCoil.Name,     // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(43)); // Field Name
            }

            thisTESCoil.CoolingAndDischargeSHRFFlowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(44));
            if (thisTESCoil.CoolingAndDischargeSHRFFlowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(44)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(44)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(44), state.dataIPShortCut->cAlphaArgs(44)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.CoolingAndDischargeSHRFFlowCurve, // Curve index
                                                                 {1},                                          // Valid dimensions
                                                                 RoutineName,                                  // Routine name
                                                                 cCurrentModuleObject,                         // Object Type
                                                                 thisTESCoil.Name,                             // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(44));  // Field Name
            }

        } // cooling and discharge mode available

        BooleanSwitch answer4 = getYesNoValue(state.dataIPShortCut->cAlphaArgs(45));
        switch (answer4) {
        case BooleanSwitch::Yes:
        case BooleanSwitch::No:
            thisTESCoil.ChargeOnlyModeAvailable = static_cast<bool>(answer4);
            break;
        default:
            thisTESCoil.ChargeOnlyModeAvailable = false;
            ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
            ShowContinueError(state, std::format("...{}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(45), state.dataIPShortCut->cAlphaArgs(45)));
            ShowContinueError(state, "Available choices are Yes or No.");
            ErrorsFound = true;
        }

        if (thisTESCoil.ChargeOnlyModeAvailable) {

            thisTESCoil.ChargeOnlyRatedCapacity = state.dataIPShortCut->rNumericArgs(24); // net storage charging capacity at rating conditions [W]
            thisTESCoil.ChargeOnlyRatedCapacitySizingFactor = state.dataIPShortCut->rNumericArgs(25); // sizing factor for charging capacity []
            thisTESCoil.ChargeOnlyRatedCOP = state.dataIPShortCut->rNumericArgs(26); // coefficient of performance at rating conditions [W/W]

            thisTESCoil.ChargeOnlyChargingCapFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(46));
            if (thisTESCoil.ChargeOnlyChargingCapFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(46)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(46)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(46), state.dataIPShortCut->cAlphaArgs(46)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.ChargeOnlyChargingCapFTempCurve, // Curve index
                                                                 {2},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(46)); // Field Name
            }

            thisTESCoil.ChargeOnlyChargingEIRFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(47));
            if (thisTESCoil.ChargeOnlyChargingEIRFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(47)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(47)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(47), state.dataIPShortCut->cAlphaArgs(47)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.ChargeOnlyChargingEIRFTempCurve, // Curve index
                                                                 {2},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(47)); // Field Name
            }

        } // Charge only mode available

        BooleanSwitch answer5 = getYesNoValue(state.dataIPShortCut->cAlphaArgs(48));
        switch (answer5) {
        case BooleanSwitch::Yes:
        case BooleanSwitch::No:
            thisTESCoil.DischargeOnlyModeAvailable = static_cast<bool>(answer5);
            break;
        default:
            thisTESCoil.DischargeOnlyModeAvailable = false;
            ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
            ShowContinueError(state, std::format("...{}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(48), state.dataIPShortCut->cAlphaArgs(48)));
            ShowContinueError(state, "Available choices are Yes or No.");
            ErrorsFound = true;
        }

        if (thisTESCoil.DischargeOnlyModeAvailable) {
            thisTESCoil.DischargeOnlyRatedDischargeCap = state.dataIPShortCut->rNumericArgs(27); // gross total evaporator cooling capacity  [W]
            thisTESCoil.DischargeOnlyRatedDischargeCapSizingFactor = state.dataIPShortCut->rNumericArgs(28); // sizing factor for cooling capacity []
            thisTESCoil.DischargeOnlyRatedSHR = state.dataIPShortCut->rNumericArgs(29); // sensible heat ratio (sens cap/total cap)
            thisTESCoil.DischargeOnlyRatedCOP = state.dataIPShortCut->rNumericArgs(30); // coefficient of performance  for discharging [W/W]

            thisTESCoil.DischargeOnlyCapFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(49));
            if (thisTESCoil.DischargeOnlyCapFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(49)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(49)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(49), state.dataIPShortCut->cAlphaArgs(49)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.DischargeOnlyCapFTempCurve,      // Curve index
                                                                 {2},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(49)); // Field Name
            }

            thisTESCoil.DischargeOnlyCapFFlowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(50));
            if (thisTESCoil.DischargeOnlyCapFFlowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(50)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(50)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(50), state.dataIPShortCut->cAlphaArgs(50)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.DischargeOnlyCapFFlowCurve,      // Curve index
                                                                 {1},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(50)); // Field Name
            }

            thisTESCoil.DischargeOnlyEIRFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(51));
            if (thisTESCoil.DischargeOnlyEIRFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(51)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(51)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(51), state.dataIPShortCut->cAlphaArgs(51)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.DischargeOnlyEIRFTempCurve,      // Curve index
                                                                 {2},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(51)); // Field Name
            }

            thisTESCoil.DischargeOnlyEIRFFlowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(52));
            if (thisTESCoil.DischargeOnlyEIRFFlowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(52)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(52)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(52), state.dataIPShortCut->cAlphaArgs(52)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.DischargeOnlyEIRFFlowCurve,      // Curve index
                                                                 {1},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(52)); // Field Name
            }

            thisTESCoil.DischargeOnlyPLFFPLRCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(53));
            if (thisTESCoil.DischargeOnlyPLFFPLRCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(53)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(53)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(53), state.dataIPShortCut->cAlphaArgs(53)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.DischargeOnlyPLFFPLRCurve,       // Curve index
                                                                 {1},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(53)); // Field Name
            }

            thisTESCoil.DischargeOnlySHRFTempCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(54));
            if (thisTESCoil.DischargeOnlySHRFTempCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(54)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(54)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(54), state.dataIPShortCut->cAlphaArgs(54)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.DischargeOnlySHRFTempCurve, // Curve index
                                                                 {2, 3},                                 // Valid dimensions  // MULTIPLECURVEDIMS
                                                                 RoutineName,                            // Routine name
                                                                 cCurrentModuleObject,                   // Object Type
                                                                 thisTESCoil.Name,                       // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(54)); // Field Name
            }

            thisTESCoil.DischargeOnlySHRFFLowCurve = GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(55));
            if (thisTESCoil.DischargeOnlySHRFFLowCurve == 0) {
                if (state.dataIPShortCut->lAlphaFieldBlanks(55)) {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(state, std::format("Required {}is blank.", state.dataIPShortCut->cAlphaFieldNames(55)));
                } else {
                    ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                    ShowContinueError(
                        state, std::format("Not found {}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(55), state.dataIPShortCut->cAlphaArgs(55)));
                }
                ErrorsFound = true;
            } else {
                // Verify Curve Object, any curve with just x as single independent variable
                ErrorsFound |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                 thisTESCoil.DischargeOnlySHRFFLowCurve,      // Curve index
                                                                 {1},                                         // Valid dimensions
                                                                 RoutineName,                                 // Routine name
                                                                 cCurrentModuleObject,                        // Object Type
                                                                 thisTESCoil.Name,                            // Object Name
                                                                 state.dataIPShortCut->cAlphaFieldNames(55)); // Field Name
            }

        } // Discharge Only mode available

        thisTESCoil.AncillaryControlsPower = state.dataIPShortCut->rNumericArgs(31);
        thisTESCoil.ColdWeatherMinimumTempLimit = state.dataIPShortCut->rNumericArgs(32);
        thisTESCoil.ColdWeatherAncillaryPower = state.dataIPShortCut->rNumericArgs(33);
        thisTESCoil.CondAirInletNodeNum = GetOnlySingleNode(state,
                                                            state.dataIPShortCut->cAlphaArgs(56),
                                                            ErrorsFound,
                                                            Node::ConnectionObjectType::CoilCoolingDXSingleSpeedThermalStorage,
                                                            thisTESCoil.Name,
                                                            Node::FluidType::Air,
                                                            Node::ConnectionType::OutsideAirReference,
                                                            Node::CompFluidStream::Primary,
                                                            Node::ObjectIsNotParent);
        thisTESCoil.CondAirOutletNodeNum = GetOnlySingleNode(state,
                                                             state.dataIPShortCut->cAlphaArgs(57),
                                                             ErrorsFound,
                                                             Node::ConnectionObjectType::CoilCoolingDXSingleSpeedThermalStorage,
                                                             thisTESCoil.Name,
                                                             Node::FluidType::Air,
                                                             Node::ConnectionType::ReliefAir,
                                                             Node::CompFluidStream::Primary,
                                                             Node::ObjectIsNotParent);

        thisTESCoil.CondenserAirVolumeFlow = state.dataIPShortCut->rNumericArgs(34);
        thisTESCoil.CondenserAirFlowSizingFactor = state.dataIPShortCut->rNumericArgs(35);

        thisTESCoil.CondenserType = static_cast<TESCondenserType>(getEnumValue(condenserTypesUC, state.dataIPShortCut->cAlphaArgs(58)));
        if (thisTESCoil.CondenserType == TESCondenserType::Invalid) {
            ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
            ShowContinueError(state, std::format("{}=\"{}\".", state.dataIPShortCut->cAlphaFieldNames(58), state.dataIPShortCut->cAlphaArgs(58)));
            ShowContinueError(state, "Available choices are AirCooled or EvaporativelyCooled.");
            ErrorsFound = true;
        }

        thisTESCoil.EvapCondEffect = state.dataIPShortCut->rNumericArgs(36);
        thisTESCoil.EvapCondPumpElecNomPower = state.dataIPShortCut->rNumericArgs(37);
        thisTESCoil.BasinHeaterPowerFTempDiff = state.dataIPShortCut->rNumericArgs(38);
        thisTESCoil.BasinHeaterSetpointTemp = state.dataIPShortCut->rNumericArgs(39);

        if (state.dataIPShortCut->lAlphaFieldBlanks(59)) {
            thisTESCoil.basinHeaterAvailSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisTESCoil.basinHeaterAvailSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(59))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(59), state.dataIPShortCut->cAlphaArgs(59));
            ErrorsFound = true;
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(60)) {
            thisTESCoil.EvapWaterSupplyMode = EvapWaterSupply::WaterSupplyFromMains;
        } else {
            thisTESCoil.EvapWaterSupplyName = state.dataIPShortCut->cAlphaArgs(60);
            thisTESCoil.EvapWaterSupplyMode = EvapWaterSupply::WaterSupplyFromTank;
            SetupTankDemandComponent(state,
                                     thisTESCoil.Name,
                                     cCurrentModuleObject,
                                     thisTESCoil.EvapWaterSupplyName,
                                     ErrorsFound,
                                     thisTESCoil.EvapWaterSupTankID,
                                     thisTESCoil.EvapWaterTankDemandARRID);
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(61)) {
            thisTESCoil.CondensateCollectMode = CondensateAction::Discard;
        } else {
            thisTESCoil.CondensateCollectName = state.dataIPShortCut->cAlphaArgs(61);
            thisTESCoil.CondensateCollectMode = CondensateAction::ToTank;
            SetupTankSupplyComponent(state,
                                     thisTESCoil.Name,
                                     cCurrentModuleObject,
                                     thisTESCoil.CondensateCollectName,
                                     ErrorsFound,
                                     thisTESCoil.CondensateTankID,
                                     thisTESCoil.CondensateTankSupplyARRID);
        }

        if (!state.dataIPShortCut->lAlphaFieldBlanks(62)) {
            thisTESCoil.TESPlantInletNodeNum = GetOnlySingleNode(state,
                                                                 state.dataIPShortCut->cAlphaArgs(62),
                                                                 ErrorsFound,
                                                                 Node::ConnectionObjectType::CoilCoolingDXSingleSpeedThermalStorage,
                                                                 state.dataIPShortCut->cAlphaArgs(1),
                                                                 Node::FluidType::Water,
                                                                 Node::ConnectionType::Inlet,
                                                                 Node::CompFluidStream::Secondary,
                                                                 Node::ObjectIsNotParent);

            thisTESCoil.TESPlantConnectionAvailable = true;
        } else {
            thisTESCoil.TESPlantConnectionAvailable = false;
        }
        if (!state.dataIPShortCut->lAlphaFieldBlanks(63)) {
            thisTESCoil.TESPlantOutletNodeNum = GetOnlySingleNode(state,
                                                                  state.dataIPShortCut->cAlphaArgs(63),
                                                                  ErrorsFound,
                                                                  Node::ConnectionObjectType::CoilCoolingDXSingleSpeedThermalStorage,
                                                                  state.dataIPShortCut->cAlphaArgs(1),
                                                                  Node::FluidType::Water,
                                                                  Node::ConnectionType::Outlet,
                                                                  Node::CompFluidStream::Secondary,
                                                                  Node::ObjectIsNotParent);
        } else {
            if (thisTESCoil.TESPlantConnectionAvailable) {
                ShowSevereError(state, std::format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisTESCoil.Name));
                ShowContinueError(state, std::format("...{} cannot be blank.", state.dataIPShortCut->cAlphaFieldNames(63)));
                ErrorsFound = true;
            }
        }
        if (thisTESCoil.TESPlantConnectionAvailable) {
            TestCompSet(state,
                        cCurrentModuleObject,
                        state.dataIPShortCut->cAlphaArgs(1),
                        state.dataIPShortCut->cAlphaArgs(62),
                        state.dataIPShortCut->cAlphaArgs(63),
                        "Water Nodes");
        }

        if (!state.dataIPShortCut->lNumericFieldBlanks(40)) {
            thisTESCoil.TESPlantDesignVolumeFlowRate = state.dataIPShortCut->rNumericArgs(40);
        }
        if (!state.dataIPShortCut->lNumericFieldBlanks(41)) {
            thisTESCoil.TESPlantEffectiveness = state.dataIPShortCut->rNumericArgs(41);
        }

        switch (thisTESCoil.StorageMedia) {
        case MediaType::UserDefindFluid:
        case MediaType::Water: {
            Real64 TminRho = -9999.0;
            Real64 TmaxRho = 9999.0;
            Real64 TminCp = -9999.0;
            Real64 TmaxCp = 9999.0;
            if (!state.dataIPShortCut->lNumericFieldBlanks(42)) {
                thisTESCoil.MinimumFluidTankTempLimit = state.dataIPShortCut->rNumericArgs(42);
            } else {
                thisTESCoil.glycol->getDensityTemperatureLimits(state, TminRho, TmaxRho);
                thisTESCoil.glycol->getSpecificHeatTemperatureLimits(state, TminCp, TmaxCp);
                thisTESCoil.MinimumFluidTankTempLimit = max(TminRho, TminCp);
            }
            if (!state.dataIPShortCut->lNumericFieldBlanks(43)) {
                thisTESCoil.MaximumFluidTankTempLimit = state.dataIPShortCut->rNumericArgs(43);
            } else {
                thisTESCoil.glycol->getDensityTemperatureLimits(state, TminRho, TmaxRho);
                thisTESCoil.glycol->getSpecificHeatTemperatureLimits(state, TminCp, TmaxCp);
                thisTESCoil.MaximumFluidTankTempLimit = min(TmaxRho, TmaxCp);
            }
        }
        default:
            // nothing
            break;
        }
    }

    if (ErrorsFound) {
        ShowFatalError(
            state, std::format("{}Errors found in getting {} input. Preceding condition(s) causes termination.", RoutineName, cCurrentModuleObject));
    }

    // setup reporting
    for (int item = 1; item <= state.dataPackagedThermalStorageCoil->NumTESCoils; ++item) {
        auto &thisTESCoil = state.dataPackagedThermalStorageCoil->TESCoil(item);

        SetupOutputVariable(state,
                            "Cooling Coil Operating Mode Index",
                            Constant::Units::None,
                            thisTESCoil.curControlModeReport,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisTESCoil.Name);

        // cCurrentModuleObject = "Coil:Cooling:DX:SingleSpeed:ThermalStorage"
        SetupOutputVariable(state,
                            "Cooling Coil Total Cooling Rate",
                            Constant::Units::W,
                            thisTESCoil.EvapTotCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisTESCoil.Name);
        SetupOutputVariable(state,
                            "Cooling Coil Total Cooling Energy",
                            Constant::Units::J,
                            thisTESCoil.EvapTotCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisTESCoil.Name,
                            Constant::eResource::EnergyTransfer,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::CoolingCoils);
        SetupOutputVariable(state,
                            "Cooling Coil Sensible Cooling Rate",
                            Constant::Units::W,
                            thisTESCoil.EvapSensCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisTESCoil.Name);
        SetupOutputVariable(state,
                            "Cooling Coil Sensible Cooling Energy",
                            Constant::Units::J,
                            thisTESCoil.EvapSensCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisTESCoil.Name);
        SetupOutputVariable(state,
                            "Cooling Coil Latent Cooling Rate",
                            Constant::Units::W,
                            thisTESCoil.EvapLatCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisTESCoil.Name);
        SetupOutputVariable(state,
                            "Cooling Coil Latent Cooling Energy",
                            Constant::Units::J,
                            thisTESCoil.EvapLatCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisTESCoil.Name);
        SetupOutputVariable(state,
                            "Cooling Coil Electricity Rate",
                            Constant::Units::W,
                            thisTESCoil.ElecCoolingPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisTESCoil.Name);
        SetupOutputVariable(state,
                            "Cooling Coil Electricity Energy",
                            Constant::Units::J,
                            thisTESCoil.ElecCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisTESCoil.Name,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Cooling);

        SetupOutputVariable(state,
                            "Cooling Coil Runtime Fraction",
                            Constant::Units::None,
                            thisTESCoil.RuntimeFraction,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisTESCoil.Name);
        SetupOutputVariable(state,
                            "Cooling Coil Cold Weather Protection Electricity Energy",
                            Constant::Units::J,
                            thisTESCoil.ElectColdWeatherEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisTESCoil.Name,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Cooling,
                            "Thermal Protection");
        SetupOutputVariable(state,
                            "Cooling Coil Cold Weather Protection Electricity Rate",
                            Constant::Units::W,
                            thisTESCoil.ElectColdWeatherPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisTESCoil.Name);

        SetupOutputVariable(state,
                            "Cooling Coil Thermal Storage Mechanical Heat Transfer Rate",
                            Constant::Units::W,
                            thisTESCoil.QdotTES,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisTESCoil.Name);

        SetupOutputVariable(state,
                            "Cooling Coil Thermal Storage Mechanical Heat Transfer Energy",
                            Constant::Units::J,
                            thisTESCoil.Q_TES,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisTESCoil.Name);

        SetupOutputVariable(state,
                            "Cooling Coil Thermal Storage Ambient Heat Transfer Rate",
                            Constant::Units::W,
                            thisTESCoil.QdotAmbient,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisTESCoil.Name);

        SetupOutputVariable(state,
                            "Cooling Coil Thermal Storage Ambient Heat Transfer Energy",
                            Constant::Units::J,
                            thisTESCoil.Q_Ambient,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisTESCoil.Name);

        if (thisTESCoil.TESPlantConnectionAvailable) {
            SetupOutputVariable(state,
                                "Cooling Coil Thermal Storage Plant Heat Transfer Rate",
                                Constant::Units::W,
                                thisTESCoil.QdotPlant,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisTESCoil.Name);
            SetupOutputVariable(state,
                                "Cooling Coil Thermal Storage Plant Heat Transfer Energy",
                                Constant::Units::J,
                                thisTESCoil.Q_Plant,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisTESCoil.Name);
        }

        if (thisTESCoil.CondenserType == TESCondenserType::Evap) {
            SetupOutputVariable(state,
                                "Cooling Coil Condenser Inlet Temperature",
                                Constant::Units::C,
                                thisTESCoil.CondInletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisTESCoil.Name);

            if (thisTESCoil.EvapWaterSupplyMode == EvapWaterSupply::WaterSupplyFromMains) {
                SetupOutputVariable(state,
                                    "Cooling Coil Evaporative Condenser Water Volume",
                                    Constant::Units::m3,
                                    thisTESCoil.EvapWaterConsump,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    thisTESCoil.Name,
                                    Constant::eResource::Water,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::Cooling);
                SetupOutputVariable(state,
                                    "Cooling Coil Evaporative Condenser Mains Supply Water Volume",
                                    Constant::Units::m3,
                                    thisTESCoil.EvapWaterConsump,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    thisTESCoil.Name,
                                    Constant::eResource::MainsWater,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::Cooling);
            } else if (thisTESCoil.EvapWaterSupplyMode == EvapWaterSupply::WaterSupplyFromTank) {
                SetupOutputVariable(state,
                                    "Cooling Coil Evaporative Condenser Storage Tank Water Volume",
                                    Constant::Units::m3,
                                    thisTESCoil.EvapWaterConsump,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    thisTESCoil.Name,
                                    Constant::eResource::Water,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::Cooling);
                SetupOutputVariable(state,
                                    "Cooling Coil Evaporative Condenser Starved Water Volume",
                                    Constant::Units::m3,
                                    thisTESCoil.EvapWaterStarvMakup,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    thisTESCoil.Name,
                                    Constant::eResource::Water,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::Cooling);
                SetupOutputVariable(state,
                                    "Cooling Coil Evaporative Condenser Starved Mains Water Volume",
                                    Constant::Units::m3,
                                    thisTESCoil.EvapWaterStarvMakup,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    thisTESCoil.Name,
                                    Constant::eResource::MainsWater,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::Cooling);
            }

            SetupOutputVariable(state,
                                "Cooling Coil Evaporative Condenser Pump Electricity Rate",
                                Constant::Units::W,
                                thisTESCoil.EvapCondPumpElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisTESCoil.Name);
            SetupOutputVariable(state,
                                "Cooling Coil Evaporative Condenser Pump Electricity Energy",
                                Constant::Units::J,
                                thisTESCoil.EvapCondPumpElecConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisTESCoil.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling);

            SetupOutputVariable(state,
                                "Cooling Coil Basin Heater Electricity Rate",
                                Constant::Units::W,
                                thisTESCoil.ElectEvapCondBasinHeaterPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisTESCoil.Name);
            SetupOutputVariable(state,
                                "Cooling Coil Basin Heater Electricity Energy",
                                Constant::Units::J,
                                thisTESCoil.ElectEvapCondBasinHeaterEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisTESCoil.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling,
                                "Thermal Protection");
        }

        switch (thisTESCoil.StorageMedia) {
        case MediaType::Water:
        case MediaType::UserDefindFluid:
            SetupOutputVariable(state,
                                "Cooling Coil Fluid Thermal Storage End Temperature",
                                Constant::Units::C,
                                thisTESCoil.FluidTankTempFinal,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisTESCoil.Name);
            break;
        case MediaType::Ice:
            SetupOutputVariable(state,
                                "Cooling Coil Ice Thermal Storage End Fraction",
                                Constant::Units::None,
                                thisTESCoil.IceFracRemain,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisTESCoil.Name);
            break;
        default:
            // nothing
            break;
        }
    }

    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
        for (int item = 1; item <= state.dataPackagedThermalStorageCoil->NumTESCoils; ++item) {
            auto &thisTESCoil = state.dataPackagedThermalStorageCoil->TESCoil(item);
            // setup EMS actuator for control mode
            SetupEMSActuator(state,
                             "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
                             thisTESCoil.Name,
                             "Operating Mode",
                             "[ ]",
                             thisTESCoil.EMSControlModeOn,
                             thisTESCoil.EMSControlModeValue);
        }
    }
}

void InitTESCoil(EnergyPlusData &state, int &TESCoilNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   <date_written>
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // Using/Aliasing

    using PlantUtilities::ScanPlantLoopsForObject;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &thisTESCoil = state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum);

    if (state.dataPackagedThermalStorageCoil->MyOneTimeFlag) {
        // initialize the environment and sizing flags
        state.dataPackagedThermalStorageCoil->MyFlag.dimension(state.dataPackagedThermalStorageCoil->NumTESCoils, true);
        state.dataPackagedThermalStorageCoil->MySizeFlag.dimension(state.dataPackagedThermalStorageCoil->NumTESCoils, true);
        state.dataPackagedThermalStorageCoil->MyEnvrnFlag.dimension(state.dataPackagedThermalStorageCoil->NumTESCoils, true);
        state.dataPackagedThermalStorageCoil->MyWarmupFlag.dimension(state.dataPackagedThermalStorageCoil->NumTESCoils, false);
        state.dataPackagedThermalStorageCoil->MyOneTimeFlag = false;
    }

    if (state.dataPackagedThermalStorageCoil->MyFlag(TESCoilNum)) {

        if (thisTESCoil.TESPlantConnectionAvailable) {
            bool errFlag = false;
            PlantLocation plantLoc{};
            ScanPlantLoopsForObject(state, thisTESCoil.Name, DataPlant::PlantEquipmentType::PackagedTESCoolingCoil, plantLoc, errFlag);

            // double check node names match
            if (errFlag) {
                ShowFatalError(state, "InitTESCoil: Program terminated due to previous condition(s).");
            }
            thisTESCoil.TESPlantLoopNum = plantLoc.loopNum;
            thisTESCoil.TESPlantLoopSideNum = plantLoc.loopSideNum;
            thisTESCoil.TESPlantBranchNum = plantLoc.branchNum;
            thisTESCoil.TESPlantCompNum = plantLoc.compNum;

            if ((DataPlant::CompData::getPlantComponent(state, plantLoc).NodeNumIn != thisTESCoil.TESPlantInletNodeNum) ||
                (DataPlant::CompData::getPlantComponent(state, plantLoc).NodeNumOut != thisTESCoil.TESPlantOutletNodeNum)) {
                ShowSevereError(
                    state,
                    std::format("InitTESCoil: Coil:Cooling:DX:SingleSpeed:ThermalStorage =\"{}\", non-matching plant nodes.", thisTESCoil.Name));
                ShowContinueError(state,
                                  std::format("...in Branch=\"{}\", Component referenced with:",
                                              state.dataPlnt->PlantLoop(thisTESCoil.TESPlantLoopNum)
                                                  .LoopSide(thisTESCoil.TESPlantLoopSideNum)
                                                  .Branch(thisTESCoil.TESPlantBranchNum)
                                                  .Name));
                ShowContinueError(state,
                                  std::format("...Inlet Node=\"{}",
                                              state.dataLoopNodes->NodeID(DataPlant::CompData::getPlantComponent(state, plantLoc).NodeNumIn)));
                ShowContinueError(state,
                                  std::format("...Outlet Node=\"{}",
                                              state.dataLoopNodes->NodeID(DataPlant::CompData::getPlantComponent(state, plantLoc).NodeNumOut)));
                ShowContinueError(state, std::format("...TES Inlet Node=\"{}", state.dataLoopNodes->NodeID(thisTESCoil.TESPlantInletNodeNum)));
                ShowContinueError(state, std::format("...TES Outlet Node=\"{}", state.dataLoopNodes->NodeID(thisTESCoil.TESPlantOutletNodeNum)));
                errFlag = true;
            }
            if (errFlag) {
                ShowFatalError(state, "InitTESCoil: Program terminated due to previous condition(s).");
            }

        } // any plant connection to TES
        state.dataPackagedThermalStorageCoil->MyFlag(TESCoilNum) = false;
    }

    if (state.dataPackagedThermalStorageCoil->MySizeFlag(TESCoilNum)) {

        SizeTESCoil(state, TESCoilNum);

        state.dataPackagedThermalStorageCoil->MySizeFlag(TESCoilNum) = false;
    }

    if (state.dataGlobal->BeginEnvrnFlag && state.dataPackagedThermalStorageCoil->MyEnvrnFlag(TESCoilNum)) {
        thisTESCoil.CurControlMode = PTSCOperatingMode::Off;
        thisTESCoil.QdotPlant = 0.0;
        thisTESCoil.Q_Plant = 0.0;
        thisTESCoil.QdotAmbient = 0.0;
        thisTESCoil.Q_Ambient = 0.0;
        thisTESCoil.QdotTES = 0.0;
        thisTESCoil.Q_TES = 0.0;
        thisTESCoil.TimeElapsed = 0.0;
        thisTESCoil.IceFracRemain = 0.0;
        thisTESCoil.IceFracRemainLastTimestep = 0.0;
        thisTESCoil.FluidTankTempFinal = thisTESCoil.RatedFluidTankTemp;
        thisTESCoil.FluidTankTempFinalLastTimestep = thisTESCoil.RatedFluidTankTemp;
        thisTESCoil.ElecCoolingPower = 0.0;     // electric power for cooling [W]
        thisTESCoil.ElecCoolingEnergy = 0.0;    // electric energy for cooling [J], metered
        thisTESCoil.EvapTotCoolingRate = 0.0;   // evaporator coil total cooling rate [W]
        thisTESCoil.EvapTotCoolingEnergy = 0.0; // evaporatory coil total cooling energy [J], metered
        thisTESCoil.EvapSensCoolingRate = 0.0;
        thisTESCoil.EvapSensCoolingEnergy = 0.0;
        thisTESCoil.EvapLatCoolingRate = 0.0;
        thisTESCoil.EvapLatCoolingEnergy = 0.0;
        thisTESCoil.RuntimeFraction = 0.0;
        thisTESCoil.ElectColdWeatherPower = 0.0;  // electric power for cold weather protection [W]
        thisTESCoil.ElectColdWeatherEnergy = 0.0; // electric energy for cold weather protection [J], metered
        thisTESCoil.ElectEvapCondBasinHeaterPower = 0.0;
        thisTESCoil.ElectEvapCondBasinHeaterEnergy = 0.0;

        state.dataPackagedThermalStorageCoil->MyEnvrnFlag(TESCoilNum) = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataPackagedThermalStorageCoil->MyEnvrnFlag(TESCoilNum) = true;
    }

    if (state.dataPackagedThermalStorageCoil->MyWarmupFlag(TESCoilNum) && (!state.dataGlobal->WarmupFlag)) {
        // reset to initial condition once warm up is over.
        thisTESCoil.IceFracRemain = 0.0;
        thisTESCoil.IceFracRemainLastTimestep = 0.0;
        thisTESCoil.FluidTankTempFinal = thisTESCoil.RatedFluidTankTemp;
        thisTESCoil.FluidTankTempFinalLastTimestep = thisTESCoil.RatedFluidTankTemp;
        state.dataPackagedThermalStorageCoil->MyWarmupFlag(TESCoilNum) = false;
    }

    if (state.dataGlobal->WarmupFlag) {
        state.dataPackagedThermalStorageCoil->MyWarmupFlag(TESCoilNum) = true;
    }

    // determine control mode
    if (thisTESCoil.availSched->getCurrentVal() != 0.0) {
        if (thisTESCoil.ModeControlType == PTSCCtrlType::ScheduledOpModes) {
            Real64 const tmpSchedValue = thisTESCoil.controlModeSched->getCurrentVal();
            // check if value is valid
            if (tmpSchedValue > static_cast<int>(PTSCOperatingMode::Invalid) && tmpSchedValue < static_cast<int>(PTSCOperatingMode::Num)) {
                thisTESCoil.CurControlMode = static_cast<PTSCOperatingMode>(tmpSchedValue);
            } else {
                thisTESCoil.CurControlMode = PTSCOperatingMode::Off;
                if (thisTESCoil.ControlModeErrorIndex == 0) {
                    ShowSevereMessage(state, "InitTESCoil: Invalid control schedule value for operating mode");
                    ShowContinueError(state, std::format("Occurs for Coil:Cooling:DX:SingleSpeed:ThermalStorage name = {}", thisTESCoil.Name));
                    ShowContinueError(state, std::format("Value returned from schedule ={:.8f}", tmpSchedValue));
                    ShowContinueError(state, "Operating mode will be set to Off, and the simulation continues");
                }
                ShowRecurringSevereErrorAtEnd(state,
                                              "InitTESCoil: Invalid control schedule value for TES operating mode, set to Off",
                                              thisTESCoil.ControlModeErrorIndex,
                                              tmpSchedValue,
                                              tmpSchedValue);
            }

        } else if (thisTESCoil.ModeControlType == PTSCCtrlType::EMSActuatedOpModes) {
            if (thisTESCoil.EMSControlModeOn) {
                int const tmpEMSValue = std::floor(thisTESCoil.EMSControlModeValue);

                // check for invalid value first
                if (tmpEMSValue <= static_cast<int>(PTSCOperatingMode::Invalid) || tmpEMSValue >= static_cast<int>(PTSCOperatingMode::Num)) {
                    thisTESCoil.CurControlMode = PTSCOperatingMode::Off;
                    if (thisTESCoil.ControlModeErrorIndex == 0) {
                        ShowSevereMessage(state, "InitTESCoil: Invalid control value for operating mode");
                        ShowContinueError(state, std::format("Occurs for Coil:Cooling:DX:SingleSpeed:ThermalStorage name = {}", thisTESCoil.Name));
                        ShowContinueError(state, std::format("Value returned from EMS ={:.8f}", thisTESCoil.EMSControlModeValue));
                        ShowContinueError(state, "Operating mode will be set to Off, and the simulation continues");
                    }
                    ShowRecurringSevereErrorAtEnd(state,
                                                  "InitTESCoil: Invalid control schedule value for TES operating mode, set to Off",
                                                  thisTESCoil.ControlModeErrorIndex,
                                                  thisTESCoil.EMSControlModeValue,
                                                  thisTESCoil.EMSControlModeValue);
                } else {
                    // at this point we have a valid value, we can cast it and assign it
                    thisTESCoil.CurControlMode = static_cast<PTSCOperatingMode>(tmpEMSValue);
                    // but then we need to do some error handling for certain cases
                    switch (thisTESCoil.CurControlMode) {
                    case PTSCOperatingMode::Off:
                        break; // nothing to check

                    case PTSCOperatingMode::CoolingOnly:
                        if (!(thisTESCoil.CoolingOnlyModeIsAvailable)) {
                            ShowSevereMessage(state, "InitTESCoil: Invalid control value for operating mode");
                            ShowContinueError(state,
                                              std::format("Occurs for Coil:Cooling:DX:SingleSpeed:ThermalStorage name = {}", thisTESCoil.Name));
                            ShowContinueError(state, "Value returned from EMS indicates Cooling Only Mode but that mode is not available.");
                            ShowContinueError(state, "Operating mode will be set to Off, and the simulation continues");
                            thisTESCoil.CurControlMode = PTSCOperatingMode::Off;
                        }
                        break;
                    case PTSCOperatingMode::CoolingAndCharge:
                        if (!(thisTESCoil.CoolingAndChargeModeAvailable)) {
                            ShowSevereMessage(state, "InitTESCoil: Invalid control value for operating mode");
                            ShowContinueError(state,
                                              std::format("Occurs for Coil:Cooling:DX:SingleSpeed:ThermalStorage name = {}", thisTESCoil.Name));
                            ShowContinueError(state, "Value returned from EMS indicates Cooling And Charge Mode but that mode is not available.");
                            ShowContinueError(state, "Operating mode will be set to Off, and the simulation continues");
                            thisTESCoil.CurControlMode = PTSCOperatingMode::Off;
                        }
                        break;
                    case PTSCOperatingMode::CoolingAndDischarge:
                        if (!(thisTESCoil.CoolingAndDischargeModeAvailable)) {
                            ShowSevereMessage(state, "InitTESCoil: Invalid control value for operating mode");
                            ShowContinueError(state,
                                              std::format("Occurs for Coil:Cooling:DX:SingleSpeed:ThermalStorage name = {}", thisTESCoil.Name));
                            ShowContinueError(state, "Value returned from EMS indicates Cooling And Discharge Mode but that mode is not available.");
                            ShowContinueError(state, "Operating mode will be set to Off, and the simulation continues");
                            thisTESCoil.CurControlMode = PTSCOperatingMode::Off;
                        }
                        break;
                    case PTSCOperatingMode::ChargeOnly:
                        if (!(thisTESCoil.ChargeOnlyModeAvailable)) {
                            ShowSevereMessage(state, "InitTESCoil: Invalid control value for operating mode");
                            ShowContinueError(state,
                                              std::format("Occurs for Coil:Cooling:DX:SingleSpeed:ThermalStorage name = {}", thisTESCoil.Name));
                            ShowContinueError(state, "Value returned from EMS indicates Charge Only Mode but that mode is not available.");
                            ShowContinueError(state, "Operating mode will be set to Off, and the simulation continues");
                            thisTESCoil.CurControlMode = PTSCOperatingMode::Off;
                        }
                        break;
                    case PTSCOperatingMode::DischargeOnly:
                        if (!(thisTESCoil.DischargeOnlyModeAvailable)) {
                            ShowSevereMessage(state, "InitTESCoil: Invalid control value for operating mode");
                            ShowContinueError(state,
                                              std::format("Occurs for Coil:Cooling:DX:SingleSpeed:ThermalStorage name = {}", thisTESCoil.Name));
                            ShowContinueError(state, "Value returned from EMS indicates Discharge Only Mode but that mode is not available.");
                            ShowContinueError(state, "Operating mode will be set to Off, and the simulation continues");
                            thisTESCoil.CurControlMode = PTSCOperatingMode::Off;
                        }
                        break;
                    default:
                        // no need to handle other cases
                        break;
                    }
                }

            } else {
                thisTESCoil.CurControlMode = PTSCOperatingMode::Off;
            }
        }
    } else {
        thisTESCoil.CurControlMode = PTSCOperatingMode::Off;
    }

    // update the integer report variable
    thisTESCoil.curControlModeReport = static_cast<int>(thisTESCoil.CurControlMode);

    thisTESCoil.QdotPlant = 0.0;   // heat exchange rate for plant connection to TES tank [W]
    thisTESCoil.Q_Plant = 0.0;     //  heat exchange energy for plant connection to TES tank [J]
    thisTESCoil.QdotAmbient = 0.0; // heat exchange rate for skin losses/gains for TES tank to surroundings [W]
    thisTESCoil.Q_Ambient = 0.0;   // heat exchange enegy for skin losses/gains for TES tank to surroundings [J]
    thisTESCoil.QdotTES = 0.0;     // heat exchange rate by mechanical systems to charge or discharge TES [W]
    thisTESCoil.Q_TES = 0.0;       // heat exchange energy by mechanical systems to charge or discharge TES [J]

    // dynamic calculated data
    thisTESCoil.ElecCoolingPower = 0.0;     // electric power for cooling [W]
    thisTESCoil.ElecCoolingEnergy = 0.0;    // electric energy for cooling [J], metered
    thisTESCoil.EvapTotCoolingRate = 0.0;   // evaporator coil total cooling rate [W]
    thisTESCoil.EvapTotCoolingEnergy = 0.0; // evaporatory coil total cooling energy [J], metered
    thisTESCoil.EvapSensCoolingRate = 0.0;
    thisTESCoil.EvapSensCoolingEnergy = 0.0;
    thisTESCoil.EvapLatCoolingRate = 0.0;
    thisTESCoil.EvapLatCoolingEnergy = 0.0;
    thisTESCoil.RuntimeFraction = 0.0;
    thisTESCoil.CondenserRuntimeFraction = 0.0;
    thisTESCoil.ElectColdWeatherPower = 0.0;  // electric power for cold weather protection [W]
    thisTESCoil.ElectColdWeatherEnergy = 0.0; // electric energy for cold weather protection [J], metered
    thisTESCoil.ElectEvapCondBasinHeaterPower = 0.0;
    thisTESCoil.ElectEvapCondBasinHeaterEnergy = 0.0;
}

void SizeTESCoil(EnergyPlusData &state, int &TESCoilNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   April 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // Using/Aliasing
    using namespace DataSizing;
    using namespace OutputReportPredefined;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("SizeTESCoil ");
    static constexpr std::string_view calcTESWaterStorageTank("CalcTESWaterStorageTank");
    Real64 constexpr FluidTankSizingDeltaT(10.0);

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 MixTemp;
    Real64 MixHumRat;
    Real64 MixEnth;
    Real64 MixWetBulb;
    Real64 SupTemp;
    Real64 SupHumRat;
    Real64 SupEnth;
    Real64 OutTemp;
    Real64 OutAirFrac;
    Real64 VolFlowRate;
    Real64 CoolCapAtPeak;
    Real64 TotCapTempModFac;
    Real64 rhoair;
    Real64 rho;
    Real64 deltaT;
    Real64 Cp;

    auto &thisTESCoil = state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum);

    if (thisTESCoil.RatedEvapAirVolFlowRate == AutoSize) {

        if (state.dataSize->CurSysNum > 0) {
            CheckSysSizing(state, "Coil:Cooling:DX:SingleSpeed:ThermalStorage", thisTESCoil.Name);
            if (state.dataSize->CurOASysNum > 0) {
                thisTESCoil.RatedEvapAirVolFlowRate = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesOutAirVolFlow;
            } else {
                thisTESCoil.RatedEvapAirVolFlowRate = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
            }
        } else if (state.dataSize->CurZoneEqNum > 0) {
            CheckZoneSizing(state, "Coil:Cooling:DX:SingleSpeed:ThermalStorage", thisTESCoil.Name);
            thisTESCoil.RatedEvapAirVolFlowRate = max(state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow,
                                                      state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow);
        }

        if (thisTESCoil.RatedEvapAirVolFlowRate < HVAC::SmallAirVolFlow) {
            thisTESCoil.RatedEvapAirVolFlowRate = 0.0;
        }
        BaseSizer::reportSizerOutput(state,
                                     "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
                                     thisTESCoil.Name,
                                     "Rated Evaporator Air Flow Rate [m3/s]",
                                     thisTESCoil.RatedEvapAirVolFlowRate);
    }

    thisTESCoil.RatedEvapAirMassFlowRate = state.dataEnvrn->StdRhoAir * thisTESCoil.RatedEvapAirVolFlowRate;

    if (thisTESCoil.CondenserAirVolumeFlow == Constant::AutoCalculate) {
        thisTESCoil.CondenserAirVolumeFlow = thisTESCoil.RatedEvapAirVolFlowRate * thisTESCoil.CondenserAirFlowSizingFactor;
        BaseSizer::reportSizerOutput(state,
                                     "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
                                     thisTESCoil.Name,
                                     "Condenser Air Flow Rate [m3/s]",
                                     thisTESCoil.CondenserAirVolumeFlow);
    }

    thisTESCoil.CondenserAirMassFlow = state.dataEnvrn->StdRhoAir * thisTESCoil.CondenserAirVolumeFlow;

    if (thisTESCoil.CoolingOnlyRatedTotCap == AutoSize) {
        if (state.dataSize->CurSysNum > 0) {
            CheckSysSizing(state, "Coil:Cooling:DX:SingleSpeed:ThermalStorage", thisTESCoil.Name);
            VolFlowRate = thisTESCoil.RatedEvapAirVolFlowRate;
            if (VolFlowRate >= HVAC::SmallAirVolFlow) {
                if (state.dataSize->CurOASysNum > 0) { // coil is in the OA stream
                    MixTemp = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).OutTempAtCoolPeak;
                    MixHumRat = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).OutHumRatAtCoolPeak;
                    SupTemp = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).PrecoolTemp;
                    SupHumRat = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).PrecoolHumRat;
                } else { // coil is on the main air loop
                    //     MixTemp = FinalSysSizing(CurSysNum)%MixTempAtCoolPeak
                    //     MixHumRat = FinalSysSizing(CurSysNum)%MixHumRatAtCoolPeak
                    SupTemp = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).CoolSupTemp;
                    SupHumRat = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).CoolSupHumRat;
                    if (state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).NumOACoolCoils ==
                        0) { // there is no precooling of the OA stream
                        MixTemp = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).MixTempAtCoolPeak;
                        MixHumRat = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).MixHumRatAtCoolPeak;
                    } else { // there is precooling of OA stream
                        if (VolFlowRate > 0.0) {
                            OutAirFrac = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesOutAirVolFlow / VolFlowRate;
                        } else {
                            OutAirFrac = 1.0;
                        }
                        OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                        MixTemp = OutAirFrac * state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).PrecoolTemp +
                                  (1.0 - OutAirFrac) * state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).RetTempAtCoolPeak;
                        MixHumRat = OutAirFrac * state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).PrecoolHumRat +
                                    (1.0 - OutAirFrac) * state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).RetHumRatAtCoolPeak;
                    }
                }
                OutTemp = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).OutTempAtCoolPeak;
                rhoair = PsyRhoAirFnPbTdbW(state, state.dataEnvrn->StdBaroPress, MixTemp, MixHumRat, RoutineName);
                MixEnth = PsyHFnTdbW(MixTemp, MixHumRat);
                MixWetBulb = PsyTwbFnTdbWPb(state, MixTemp, MixHumRat, state.dataEnvrn->StdBaroPress, RoutineName);
                SupEnth = PsyHFnTdbW(SupTemp, SupHumRat);
                TotCapTempModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingOnlyCapFTempCurve, MixWetBulb, OutTemp);
                CoolCapAtPeak = max(0.0, (rhoair * VolFlowRate * (MixEnth - SupEnth)));
                if (TotCapTempModFac > 0.0) {
                    thisTESCoil.CoolingOnlyRatedTotCap = CoolCapAtPeak / TotCapTempModFac;
                } else {
                    thisTESCoil.CoolingOnlyRatedTotCap = CoolCapAtPeak;
                }

            } else {
                thisTESCoil.CoolingOnlyRatedTotCap = 0.0;
            }
        } else if (state.dataSize->CurZoneEqNum > 0) {
            CheckZoneSizing(state, "Coil:Cooling:DX:SingleSpeed:ThermalStorage", thisTESCoil.Name);
            VolFlowRate = thisTESCoil.RatedEvapAirVolFlowRate;
            if (VolFlowRate >= HVAC::SmallAirVolFlow) {
                if (state.dataSize->ZoneEqDXCoil) {
                    if (state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum).OAVolFlow > 0.0) {
                        MixTemp = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolCoilInTemp;
                        MixHumRat = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolCoilInHumRat;
                    } else {
                        MixTemp = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).ZoneRetTempAtCoolPeak;
                        MixHumRat = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).ZoneHumRatAtCoolPeak;
                    }
                } else {
                    MixTemp = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolCoilInTemp;
                    MixHumRat = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolCoilInHumRat;
                }
                SupTemp = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).CoolDesTemp;
                SupHumRat = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).CoolDesHumRat;
                int const TimeStepNumAtMax = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).TimeStepNumAtCoolMax;
                int const DDNum = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).CoolDDNum;
                if (DDNum > 0 && TimeStepNumAtMax > 0) {
                    OutTemp = state.dataSize->DesDayWeath(DDNum).Temp(TimeStepNumAtMax);
                } else {
                    OutTemp = 0.0;
                }
                rhoair = PsyRhoAirFnPbTdbW(state, state.dataEnvrn->StdBaroPress, MixTemp, MixHumRat, RoutineName);
                MixEnth = PsyHFnTdbW(MixTemp, MixHumRat);
                MixWetBulb = PsyTwbFnTdbWPb(state, MixTemp, MixHumRat, state.dataEnvrn->StdBaroPress, RoutineName);
                SupEnth = PsyHFnTdbW(SupTemp, SupHumRat);
                TotCapTempModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingOnlyCapFTempCurve, MixWetBulb, OutTemp);
                CoolCapAtPeak = max(0.0, (rhoair * VolFlowRate * (MixEnth - SupEnth)));
                if (TotCapTempModFac > 0.0) {
                    thisTESCoil.CoolingOnlyRatedTotCap = CoolCapAtPeak / TotCapTempModFac;
                } else {
                    thisTESCoil.CoolingOnlyRatedTotCap = CoolCapAtPeak;
                }

            } else {
                thisTESCoil.CoolingOnlyRatedTotCap = 0.0;
            }
        }

        BaseSizer::reportSizerOutput(state,
                                     "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
                                     thisTESCoil.Name,
                                     "Cooling Only Mode Rated Total Evaporator Cooling Capacity [W]",
                                     thisTESCoil.CoolingOnlyRatedTotCap);
    }

    if (thisTESCoil.CoolingAndChargeModeAvailable && (thisTESCoil.CoolingAndChargeRatedTotCap == Constant::AutoCalculate)) {
        thisTESCoil.CoolingAndChargeRatedTotCap = thisTESCoil.CoolingOnlyRatedTotCap * thisTESCoil.CoolingAndChargeRatedTotCapSizingFactor;
        BaseSizer::reportSizerOutput(state,
                                     "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
                                     thisTESCoil.Name,
                                     "Cooling And Charge Mode Rated Total Evaporator Cooling Capacity [W]",
                                     thisTESCoil.CoolingAndChargeRatedTotCap);
    }

    if (thisTESCoil.CoolingAndChargeModeAvailable && (thisTESCoil.CoolingAndChargeRatedChargeCap == Constant::AutoCalculate)) {
        thisTESCoil.CoolingAndChargeRatedChargeCap = thisTESCoil.CoolingOnlyRatedTotCap * thisTESCoil.CoolingAndChargeRatedChargeCapSizingFactor;
        BaseSizer::reportSizerOutput(state,
                                     "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
                                     thisTESCoil.Name,
                                     "Cooling And Charge Mode Rated Storage Charging Capacity [W]",
                                     thisTESCoil.CoolingAndChargeRatedChargeCap);
    }

    if (thisTESCoil.CoolingAndDischargeModeAvailable && (thisTESCoil.CoolingAndDischargeRatedTotCap == Constant::AutoCalculate)) {
        thisTESCoil.CoolingAndDischargeRatedTotCap = thisTESCoil.CoolingOnlyRatedTotCap * thisTESCoil.CoolingAndDischargeRatedTotCapSizingFactor;
        BaseSizer::reportSizerOutput(state,
                                     "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
                                     thisTESCoil.Name,
                                     "Cooling And Discharge Mode Rated Total Evaporator Cooling Capacity [W]",
                                     thisTESCoil.CoolingAndDischargeRatedTotCap);
    }

    if (thisTESCoil.CoolingAndDischargeModeAvailable && (thisTESCoil.CoolingAndDischargeRatedDischargeCap == Constant::AutoCalculate)) {
        thisTESCoil.CoolingAndDischargeRatedDischargeCap =
            thisTESCoil.CoolingOnlyRatedTotCap * thisTESCoil.CoolingAndDischargeRatedDischargeCapSizingFactor;
        BaseSizer::reportSizerOutput(state,
                                     "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
                                     thisTESCoil.Name,
                                     "Cooling And Discharge Mode Rated Storage Discharging Capacity [W]",
                                     thisTESCoil.CoolingAndDischargeRatedDischargeCap);
    }

    if (thisTESCoil.ChargeOnlyModeAvailable && (thisTESCoil.ChargeOnlyRatedCapacity == Constant::AutoCalculate)) {
        thisTESCoil.ChargeOnlyRatedCapacity = thisTESCoil.CoolingOnlyRatedTotCap * thisTESCoil.ChargeOnlyRatedCapacitySizingFactor;
        BaseSizer::reportSizerOutput(state,
                                     "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
                                     thisTESCoil.Name,
                                     "Charge Only Mode Rated Storage Charging Capacity [W]",
                                     thisTESCoil.ChargeOnlyRatedCapacity);
    }

    if (thisTESCoil.DischargeOnlyModeAvailable && (thisTESCoil.DischargeOnlyRatedDischargeCap == Constant::AutoCalculate)) {
        thisTESCoil.DischargeOnlyRatedDischargeCap = thisTESCoil.CoolingOnlyRatedTotCap * thisTESCoil.DischargeOnlyRatedDischargeCapSizingFactor;
        BaseSizer::reportSizerOutput(state,
                                     "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
                                     thisTESCoil.Name,
                                     "Discharge Only Mode Rated Storage Discharging Capacity [W]",
                                     thisTESCoil.DischargeOnlyRatedDischargeCap);
    }

    switch (thisTESCoil.StorageMedia) {
    case MediaType::UserDefindFluid:
    case MediaType::Water:
        if (thisTESCoil.FluidStorageVolume == Constant::AutoCalculate) {

            // for fluid tanks, assume a 10C deltaT or diff between max and min, whichever is smaller
            deltaT = min(FluidTankSizingDeltaT, (thisTESCoil.MaximumFluidTankTempLimit - thisTESCoil.MinimumFluidTankTempLimit));
            rho = thisTESCoil.glycol->getDensity(state, Constant::CWInitConvTemp, calcTESWaterStorageTank);
            Cp = thisTESCoil.glycol->getSpecificHeat(state, Constant::CWInitConvTemp, calcTESWaterStorageTank);
            if (thisTESCoil.DischargeOnlyRatedDischargeCap > 0.0 && thisTESCoil.DischargeOnlyModeAvailable) {
                thisTESCoil.FluidStorageVolume =
                    (thisTESCoil.DischargeOnlyRatedDischargeCap * thisTESCoil.StorageCapacitySizingFactor * Constant::rSecsInHour) /
                    (rho * Cp * deltaT);
            } else {
                thisTESCoil.FluidStorageVolume =
                    (thisTESCoil.CoolingOnlyRatedTotCap * thisTESCoil.StorageCapacitySizingFactor * Constant::rSecsInHour) / (rho * Cp * deltaT);
            }
            BaseSizer::reportSizerOutput(
                state, "Coil:Cooling:DX:SingleSpeed:ThermalStorage", thisTESCoil.Name, "Fluid Storage Volume [m3]", thisTESCoil.FluidStorageVolume);
        }
        break;
    case MediaType::Ice:
        if (thisTESCoil.IceStorageCapacity == Constant::AutoCalculate) {
            if (thisTESCoil.DischargeOnlyRatedDischargeCap > 0.0 && thisTESCoil.DischargeOnlyModeAvailable) {
                thisTESCoil.IceStorageCapacity =
                    thisTESCoil.DischargeOnlyRatedDischargeCap * thisTESCoil.StorageCapacitySizingFactor * Constant::rSecsInHour;
            } else {
                thisTESCoil.IceStorageCapacity = thisTESCoil.CoolingOnlyRatedTotCap * thisTESCoil.StorageCapacitySizingFactor * Constant::rSecsInHour;
            }
            BaseSizer::reportSizerOutput(state,
                                         "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
                                         thisTESCoil.Name,
                                         "Ice Storage Capacity [GJ]",
                                         thisTESCoil.IceStorageCapacity / gigaJoulesToJoules);
        }
    default:
        break;
    }

    if ((thisTESCoil.CondenserType == TESCondenserType::Evap) && (thisTESCoil.EvapCondPumpElecNomPower == AutoSize)) {
        thisTESCoil.EvapCondPumpElecNomPower = thisTESCoil.CoolingOnlyRatedTotCap * 0.004266; // w/w (15 w/ton)
        BaseSizer::reportSizerOutput(state,
                                     "Coil:Cooling:DX:SingleSpeed:ThermalStorage",
                                     thisTESCoil.Name,
                                     "Evaporative Condenser Pump Rated Power Consumption [W]",
                                     thisTESCoil.EvapCondPumpElecNomPower);
    }

    PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoolCoilType, thisTESCoil.Name, "Coil:Cooling:DX:SingleSpeed:ThermalStorage");

    PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoolCoilTotCap, thisTESCoil.Name, thisTESCoil.CoolingOnlyRatedTotCap);
    PreDefTableEntry(state,
                     state.dataOutRptPredefined->pdchCoolCoilSensCap,
                     thisTESCoil.Name,
                     thisTESCoil.CoolingOnlyRatedTotCap * thisTESCoil.CoolingOnlyRatedSHR);
    PreDefTableEntry(state,
                     state.dataOutRptPredefined->pdchCoolCoilLatCap,
                     thisTESCoil.Name,
                     thisTESCoil.CoolingOnlyRatedTotCap - thisTESCoil.CoolingOnlyRatedTotCap * thisTESCoil.CoolingOnlyRatedSHR);
    PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoolCoilSHR, thisTESCoil.Name, thisTESCoil.CoolingOnlyRatedSHR);
    PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoolCoilNomEff, thisTESCoil.Name, thisTESCoil.CoolingOnlyRatedCOP);
}

void CalcTESCoilOffMode(EnergyPlusData &state, int const TESCoilNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   April 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    auto &thisTESCoil = state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum);

    // coil is off; just pass through conditions
    Real64 StandbyAncillaryPower = 0.0;
    if (thisTESCoil.availSched->getCurrentVal() != 0.0) {
        StandbyAncillaryPower = thisTESCoil.AncillaryControlsPower;
    }

    thisTESCoil.ElecCoolingPower = StandbyAncillaryPower;
    thisTESCoil.ElecCoolingEnergy = StandbyAncillaryPower * TimeStepSysSec;

    state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).Temp;
    state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).HumRat;
    state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRate =
        state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRate;
    state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMinAvail =
        state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMinAvail;
    state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMaxAvail =
        state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMaxAvail;
    state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Enthalpy = PsyHFnTdbW(
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp, state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat);

    state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
    state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat;
    state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate = 0.0;
    state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).MassFlowRate =
        state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate;
    state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Enthalpy = PsyHFnTdbW(
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp, state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat);
    thisTESCoil.RuntimeFraction = 0.0;
    thisTESCoil.EvapTotCoolingRate = 0.0;
    thisTESCoil.EvapTotCoolingEnergy = 0.0;
    thisTESCoil.EvapSensCoolingRate = 0.0;
    thisTESCoil.EvapSensCoolingEnergy = 0.0;
    thisTESCoil.EvapLatCoolingRate = 0.0;
    thisTESCoil.EvapLatCoolingEnergy = 0.0;

    thisTESCoil.QdotTES = 0.0;
    thisTESCoil.Q_TES = 0.0;

    UpdateTEStorage(state, TESCoilNum);

    thisTESCoil.CondInletTemp = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;

    UpdateColdWeatherProtection(state, TESCoilNum);

    if (thisTESCoil.CondenserType == TESCondenserType::Evap) {
        UpdateEvaporativeCondenserBasinHeater(state, TESCoilNum);
    }
}

void CalcTESCoilCoolingOnlyMode(EnergyPlusData &state, int const TESCoilNum, [[maybe_unused]] HVAC::FanOp const fanOp, Real64 const PartLoadRatio)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   April 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // Using/Aliasing
    Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr MaxIter(30);
    Real64 constexpr RelaxationFactor(0.4);
    Real64 constexpr Tolerance(0.1);
    static constexpr std::string_view RoutineName("CalcTESCoilCoolingOnlyMode");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 CondInletTemp; // Condenser inlet temperature (C). Outdoor dry-bulb temp for air-cooled condenser.
    // Outdoor Wetbulb +(1 - effectiveness)*(outdoor drybulb - outdoor wetbulb) for evap condenser.
    Real64 CondInletHumRat; // Condenser inlet humidity ratio (kg/kg). Zero for air-cooled condenser.
                            // For evap condenser, its the humidity ratio of the air leaving the evap cooling pads.

    auto &thisTESCoil = state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum);

    // first deal with condenser
    if (thisTESCoil.CondenserType == TESCondenserType::Air) {
        Real64 const CondAirSidePressure = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Press;
        if (CondAirSidePressure == state.dataLoopNodes->DefaultNodeValues.Press) {
            CondInletTemp = state.dataEnvrn->OutDryBulbTemp;
            CondInletHumRat = state.dataEnvrn->OutHumRat;
        } else {
            CondInletTemp = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
            CondInletHumRat = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat;
        }
    } else if (thisTESCoil.CondenserType == TESCondenserType::Evap) {
        Real64 CondAirSidePressure = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Press;
        Real64 OutdoorDryBulb = 0.0;
        Real64 OutdoorWetBulb = 0.0;
        if (CondAirSidePressure == state.dataLoopNodes->DefaultNodeValues.Press) {
            OutdoorDryBulb = state.dataEnvrn->OutDryBulbTemp;
            CondAirSidePressure = state.dataEnvrn->OutBaroPress;
            OutdoorWetBulb = state.dataEnvrn->OutWetBulbTemp;
        } else {
            OutdoorDryBulb = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
            Real64 const OutdoorHumRat = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat;
            OutdoorWetBulb = PsyTwbFnTdbWPb(state, OutdoorDryBulb, OutdoorHumRat, CondAirSidePressure, RoutineName);
        }
        // direct evap cool model
        CondInletTemp = OutdoorWetBulb + (OutdoorDryBulb - OutdoorWetBulb) * (1.0 - thisTESCoil.EvapCondEffect);
        CondInletHumRat = PsyWFnTdbTwbPb(state, CondInletTemp, OutdoorWetBulb, CondAirSidePressure, RoutineName);
    }

    Real64 const EvapAirMassFlow = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRate;

    if ((EvapAirMassFlow > HVAC::SmallMassFlow) && (PartLoadRatio > 0.0)) { // coil is running

        Real64 const EvapInletDryBulb = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).Temp;
        Real64 const EvapInletHumRat = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).HumRat;
        Real64 const EvapInletWetBulb = PsyTwbFnTdbWPb(state, EvapInletDryBulb, EvapInletHumRat, state.dataEnvrn->OutBaroPress, RoutineName);
        Real64 const EvapInletEnthalpy = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).Enthalpy;

        Real64 const AirMassFlowRatio = EvapAirMassFlow / thisTESCoil.RatedEvapAirMassFlowRate;
        Real64 TotCapTempModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingOnlyCapFTempCurve, EvapInletWetBulb, CondInletTemp);
        TotCapTempModFac = max(0.0, TotCapTempModFac); // could warn if negative, DXcoil does
        Real64 TotCapFlowModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingOnlyCapFFlowCurve, AirMassFlowRatio);
        TotCapFlowModFac = max(0.0, TotCapFlowModFac); // could warn if negative, DXcoil does
        Real64 TotCap = thisTESCoil.CoolingOnlyRatedTotCap * TotCapTempModFac * TotCapFlowModFac;

        // now see if coil might be running dry
        Real64 const PartLoadOutAirEnth = EvapInletEnthalpy - (TotCap * PartLoadRatio) / EvapAirMassFlow;
        Real64 const PartLoadDryCoilOutAirTemp = PsyTdbFnHW(PartLoadOutAirEnth, EvapInletHumRat);

        bool CoilMightBeDry = false;
        Real64 DryCoilTestEvapInletHumRat = 0.0;
        Real64 SHRadp = 0.0;
        if (PartLoadDryCoilOutAirTemp > PsyTsatFnHPb(state, PartLoadOutAirEnth, state.dataEnvrn->OutBaroPress, RoutineName)) {

            CoilMightBeDry = true;
            // find wADP, humidity ratio at apparatus dewpoint and inlet hum rat that would have dry coil
            DryCoilTestEvapInletHumRat = EvapInletHumRat;
            Real64 DryCoilTestEvapInletWetBulb = EvapInletWetBulb;
            int Counter = 0;
            bool Converged = false;
            while (!Converged) {
                TotCapTempModFac =
                    EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingOnlyCapFTempCurve, DryCoilTestEvapInletWetBulb, CondInletTemp);
                TotCapTempModFac = max(0.0, TotCapTempModFac); // could warn if negative, DXcoil does

                TotCapFlowModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingOnlyCapFFlowCurve, AirMassFlowRatio);
                TotCapFlowModFac = max(0.0, TotCapFlowModFac); // could warn if negative, DXcoil does
                TotCap = thisTESCoil.CoolingOnlyRatedTotCap * TotCapTempModFac * TotCapFlowModFac;

                // coil bypass factor = 0.0
                Real64 const hADP = EvapInletEnthalpy - (TotCap / EvapAirMassFlow);
                Real64 const tADP = PsyTsatFnHPb(state, hADP, state.dataEnvrn->OutBaroPress, RoutineName);
                Real64 const wADP = min(EvapInletHumRat, PsyWFnTdbH(state, tADP, hADP, RoutineName));
                Real64 const hTinwADP = PsyHFnTdbW(EvapInletDryBulb, wADP);
                if ((EvapInletEnthalpy - hADP) > 1.e-10) {
                    SHRadp = min((hTinwADP - hADP) / (EvapInletEnthalpy - hADP), 1.0);
                } else {
                    SHRadp = 1.0;
                }

                if ((wADP > DryCoilTestEvapInletHumRat) || (Counter >= 1 && Counter < MaxIter)) {
                    if (DryCoilTestEvapInletHumRat <= 0.0) {
                        DryCoilTestEvapInletHumRat = 0.00001;
                    }
                    Real64 const werror = (DryCoilTestEvapInletHumRat - wADP) / DryCoilTestEvapInletHumRat;

                    DryCoilTestEvapInletHumRat = RelaxationFactor * wADP + (1.0 - RelaxationFactor) * DryCoilTestEvapInletHumRat;
                    DryCoilTestEvapInletWetBulb =
                        PsyTwbFnTdbWPb(state, EvapInletDryBulb, DryCoilTestEvapInletHumRat, state.dataEnvrn->OutBaroPress, RoutineName);

                    ++Counter;
                    Converged = (std::abs(werror) <= Tolerance);
                } else {
                    Converged = true;
                }
            }
        }

        // total cooling capacity modification factors
        Real64 const SHRTempFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingOnlySHRFTempCurve, EvapInletWetBulb, EvapInletDryBulb);
        Real64 const SHRFlowFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingOnlySHRFFlowCurve, AirMassFlowRatio);

        Real64 SHR = thisTESCoil.CoolingOnlyRatedSHR * SHRTempFac * SHRFlowFac;
        SHR = min(SHR, 1.0); // warn maybe
        SHR = max(SHR, 0.0); // warn maybe
        if (CoilMightBeDry) {
            if ((EvapInletHumRat < DryCoilTestEvapInletHumRat) && (SHRadp > SHR)) { // coil is dry for sure
                SHR = 1.0;
            } else if (SHRadp > SHR) {
                SHR = SHRadp;
            }
        }

        // part load factor
        Real64 const PLF = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingOnlyPLFFPLRCurve, PartLoadRatio);
        // compressor running time divided by full time of timestep.
        Real64 RuntimeFraction = 1.0;
        if (PLF >= PartLoadRatio && PLF > 0.0) {
            RuntimeFraction = PartLoadRatio / PLF;
        } else {
            RuntimeFraction = 1.0; // warn maybe
        }
        //  Calculate full load output conditions
        Real64 const FullLoadOutAirEnth = EvapInletEnthalpy - TotCap / EvapAirMassFlow;

        // Enthalpy at inlet dry-bulb and outlet humidity ratio [J/kg]
        Real64 const hTinwout = EvapInletEnthalpy - (1.0 - SHR) * (TotCap / EvapAirMassFlow);
        // The following will often throw psych warnings for neg w, suppress warnings because error condition is handled in next IF
        Real64 FullLoadOutAirHumRat = PsyWFnTdbH(state, EvapInletDryBulb, hTinwout, RoutineName, true);
        Real64 FullLoadOutAirTemp = PsyTdbFnHW(FullLoadOutAirEnth, FullLoadOutAirHumRat);
        // Check for saturation error and modify temperature at constant enthalpy
        if (FullLoadOutAirTemp < PsyTsatFnHPb(state, FullLoadOutAirEnth, state.dataEnvrn->OutBaroPress, RoutineName)) {
            FullLoadOutAirTemp = PsyTsatFnHPb(state, FullLoadOutAirEnth, state.dataEnvrn->OutBaroPress, RoutineName);
            FullLoadOutAirHumRat = PsyWFnTdbH(state, FullLoadOutAirTemp, FullLoadOutAirEnth, RoutineName);
        }

        // Continuous fan, cycling compressor
        Real64 const EvapOutletAirEnthalpy = ((PartLoadRatio)*FullLoadOutAirEnth + (1.0 - (PartLoadRatio)) * EvapInletEnthalpy);
        Real64 EvapOutletAirHumRat = ((PartLoadRatio)*FullLoadOutAirHumRat + (1.0 - (PartLoadRatio)) * EvapInletHumRat);
        Real64 EvapOutletAirTemp = PsyTdbFnHW(EvapOutletAirEnthalpy, EvapOutletAirHumRat);
        if (EvapOutletAirTemp < PsyTsatFnHPb(state, EvapOutletAirEnthalpy, state.dataEnvrn->OutBaroPress, RoutineName)) {
            EvapOutletAirTemp = PsyTsatFnHPb(state, EvapOutletAirEnthalpy, state.dataEnvrn->OutBaroPress, RoutineName);
            EvapOutletAirHumRat = PsyWFnTdbH(state, EvapOutletAirTemp, EvapOutletAirEnthalpy, RoutineName);
        }
        // Calculate electricity consumed. First, get EIR modifying factors for off-rated conditions
        Real64 EIRTempModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingOnlyEIRFTempCurve, EvapInletWetBulb, CondInletTemp);
        EIRTempModFac = max(EIRTempModFac, 0.0);

        Real64 EIRFlowModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingOnlyEIRFFlowCurve, AirMassFlowRatio);
        EIRFlowModFac = max(EIRFlowModFac, 0.0);

        Real64 const EIR = EIRTempModFac * EIRFlowModFac / thisTESCoil.CoolingOnlyRatedCOP;

        Real64 const ElecCoolingPower = TotCap * EIR * RuntimeFraction;

        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp = EvapOutletAirTemp;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat = EvapOutletAirHumRat;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Enthalpy = EvapOutletAirEnthalpy;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRate = EvapAirMassFlow;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMinAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMinAvail;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMaxAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMaxAvail;

        // determine condenser leaving conditions
        Real64 const QdotCond = TotCap * RuntimeFraction + ElecCoolingPower;
        state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate = thisTESCoil.CondenserAirMassFlow;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).MassFlowRate = thisTESCoil.CondenserAirMassFlow;
        Real64 const CondInletEnthalpy = PsyHFnTdbW(CondInletTemp, CondInletHumRat);
        Real64 const CondOutletEnthalpy = CondInletEnthalpy + QdotCond / thisTESCoil.CondenserAirMassFlow;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp = PsyTdbFnHW(CondOutletEnthalpy, CondInletHumRat);
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat = CondInletHumRat;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Enthalpy = CondOutletEnthalpy;

        thisTESCoil.ElecCoolingPower = ElecCoolingPower + thisTESCoil.AncillaryControlsPower;
        thisTESCoil.ElecCoolingEnergy = thisTESCoil.ElecCoolingPower * TimeStepSysSec;

        thisTESCoil.RuntimeFraction = RuntimeFraction;
        thisTESCoil.CondenserRuntimeFraction = RuntimeFraction;
        thisTESCoil.EvapTotCoolingRate = TotCap * RuntimeFraction; // double check this
        thisTESCoil.EvapTotCoolingEnergy = TotCap * RuntimeFraction * TimeStepSysSec;
        Real64 const MinAirHumRat = min(state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat,
                                        state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).HumRat);
        thisTESCoil.EvapSensCoolingRate =
            EvapAirMassFlow * (PsyHFnTdbW(EvapInletDryBulb, MinAirHumRat) - PsyHFnTdbW(EvapOutletAirTemp, MinAirHumRat));
        if (thisTESCoil.EvapSensCoolingRate > thisTESCoil.EvapTotCoolingRate) {
            thisTESCoil.EvapSensCoolingRate = thisTESCoil.EvapTotCoolingRate;
        }
        thisTESCoil.EvapSensCoolingEnergy = thisTESCoil.EvapSensCoolingRate * TimeStepSysSec;
        thisTESCoil.EvapLatCoolingRate = thisTESCoil.EvapTotCoolingRate - thisTESCoil.EvapSensCoolingRate;
        thisTESCoil.EvapLatCoolingEnergy = thisTESCoil.EvapLatCoolingRate * TimeStepSysSec;

    } else { // coil is off; just pass through conditions
        thisTESCoil.ElecCoolingPower = thisTESCoil.AncillaryControlsPower;
        thisTESCoil.ElecCoolingEnergy = thisTESCoil.ElecCoolingPower * TimeStepSysSec;
        thisTESCoil.RuntimeFraction = 0.0;
        thisTESCoil.CondenserRuntimeFraction = 0.0;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).Temp;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).HumRat;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRate =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRate;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMinAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMinAvail;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMaxAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMaxAvail;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Enthalpy = PsyHFnTdbW(
            state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp, state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat);

        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat;
        state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate = 0.0;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).MassFlowRate =
            state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Enthalpy = PsyHFnTdbW(
            state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp, state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat);
        thisTESCoil.EvapTotCoolingRate = 0.0;
        thisTESCoil.EvapTotCoolingEnergy = 0.0;
        thisTESCoil.EvapSensCoolingRate = 0.0;
        thisTESCoil.EvapSensCoolingEnergy = 0.0;
        thisTESCoil.EvapLatCoolingRate = 0.0;
        thisTESCoil.EvapLatCoolingEnergy = 0.0;
    }

    thisTESCoil.QdotTES = 0.0;
    thisTESCoil.Q_TES = 0.0;

    UpdateTEStorage(state, TESCoilNum);

    thisTESCoil.CondInletTemp = CondInletTemp;

    UpdateColdWeatherProtection(state, TESCoilNum);

    if (thisTESCoil.CondenserType == TESCondenserType::Evap) {
        UpdateEvaporativeCondenserBasinHeater(state, TESCoilNum);
        UpdateEvaporativeCondenserWaterUse(state, TESCoilNum, CondInletHumRat, thisTESCoil.CondAirInletNodeNum);
    }
}

void CalcTESCoilCoolingAndChargeMode(EnergyPlusData &state,
                                     int const TESCoilNum,
                                     [[maybe_unused]] HVAC::FanOp const fanOp,
                                     Real64 const PartLoadRatio)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   April 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // Using/Aliasing
    Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr MaxIter(30);
    Real64 constexpr RelaxationFactor(0.4);
    Real64 constexpr Tolerance(0.1);
    static constexpr std::string_view RoutineName("CalcTESCoilCoolingAndChargeMode");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 CondInletTemp;   // Condenser inlet temperature (C). Outdoor dry-bulb temp for air-cooled condenser.
                            // Outdoor Wetbulb +(1 - effectiveness)*(outdoor drybulb - outdoor wetbulb) for evap condenser.
    Real64 CondInletHumRat; // Condenser inlet humidity ratio (kg/kg). Zero for air-cooled condenser.
                            // For evap condenser, its the humidity ratio of the air leaving the evap cooling pads.

    auto &thisTESCoil = state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum);

    // first deal with condenser
    if (thisTESCoil.CondenserType == TESCondenserType::Air) {
        Real64 const CondAirSidePressure = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Press;
        if (CondAirSidePressure == state.dataLoopNodes->DefaultNodeValues.Press) {
            CondInletTemp = state.dataEnvrn->OutDryBulbTemp;
            CondInletHumRat = state.dataEnvrn->OutHumRat;
        } else {
            CondInletTemp = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
            CondInletHumRat = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat;
        }
    } else if (thisTESCoil.CondenserType == TESCondenserType::Evap) {
        Real64 const CondAirSidePressure = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Press;
        Real64 OutdoorDryBulb = 0.0;
        Real64 OutdoorWetBulb = 0.0;
        if (CondAirSidePressure == state.dataLoopNodes->DefaultNodeValues.Press) {
            OutdoorDryBulb = state.dataEnvrn->OutDryBulbTemp;
            OutdoorWetBulb = state.dataEnvrn->OutWetBulbTemp;
        } else {
            OutdoorDryBulb = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
            Real64 const OutdoorHumRat = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat;
            OutdoorWetBulb = PsyTwbFnTdbWPb(state, OutdoorDryBulb, OutdoorHumRat, CondAirSidePressure, RoutineName);
        }
        // direct evap cool model
        CondInletTemp = OutdoorWetBulb + (OutdoorDryBulb - OutdoorWetBulb) * (1.0 - thisTESCoil.EvapCondEffect);
        CondInletHumRat = PsyWFnTdbTwbPb(state, CondInletTemp, OutdoorWetBulb, CondAirSidePressure, RoutineName);
    }

    Real64 QdotChargeLimit; // limit for charge cooling power to hit limit of storage.
    Real64 sTES;            // stat of Thermal energy storage [C or fraction of ice]
    bool TESCanBeCharged;

    switch (state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).StorageMedia) {
    case MediaType::Water:
    case MediaType::UserDefindFluid:
        sTES = thisTESCoil.FluidTankTempFinalLastTimestep;
        if ((sTES > thisTESCoil.MinimumFluidTankTempLimit) && (sTES < thisTESCoil.MaximumFluidTankTempLimit)) {
            TESCanBeCharged = true;
            // find charge limit to reach limits
            Real64 const rho = thisTESCoil.glycol->getDensity(state, sTES, RoutineName);
            Real64 const TankMass = rho * thisTESCoil.FluidStorageVolume;
            Real64 const CpTank = thisTESCoil.glycol->getSpecificHeat(state, sTES, RoutineName);
            // simple linear approximation of DT/Dt term in McpDT/Dt
            QdotChargeLimit = TankMass * CpTank * (sTES - thisTESCoil.MinimumFluidTankTempLimit) / TimeStepSysSec;
        } else {
            TESCanBeCharged = false;
        }
        break;
    case MediaType::Ice:
        sTES = thisTESCoil.IceFracRemainLastTimestep;
        if (sTES < 1.0) {
            TESCanBeCharged = true;
            // find charge limit to reach limit
            QdotChargeLimit = (1.0 - sTES) * thisTESCoil.IceStorageCapacity / TimeStepSysSec;
        } else {
            TESCanBeCharged = false;
        }
        break;
    default:
        break;
    }

    // local for evaporator air mass flow [kg/s]
    Real64 const EvapAirMassFlow = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRate;
    // evaporator inlet air drybulb [C]
    Real64 const EvapInletDryBulb = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).Temp;
    // evaporator inlet air humidity ratio [kg/kg]
    Real64 const EvapInletHumRat = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).HumRat;
    // evaporator inlet air wetbulb [C]
    Real64 const EvapInletWetBulb = PsyTwbFnTdbWPb(state, EvapInletDryBulb, EvapInletHumRat, state.dataEnvrn->OutBaroPress, RoutineName);

    Real64 TotChargeCap = 0.0;

    if ((EvapAirMassFlow > HVAC::SmallMassFlow) && (PartLoadRatio > 0.0)) { // coil is running

        Real64 const EvapInletEnthalpy = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).Enthalpy;

        // evaporator inlet air mass flow divided by design mass flow [ ]
        Real64 const AirMassFlowRatio = EvapAirMassFlow / thisTESCoil.RatedEvapAirMassFlowRate;

        // total cooling capacity modification factor due to temps []
        Real64 EvapTotCapTempModFac =
            EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeCoolingCapFTempCurve, EvapInletWetBulb, CondInletTemp, sTES);
        EvapTotCapTempModFac = max(0.0, EvapTotCapTempModFac); // could warn if negative, DXcoil does

        // total cooling capacity modification factor due to flow []
        Real64 EvapTotCapFlowModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeCoolingCapFFlowCurve, AirMassFlowRatio);
        EvapTotCapFlowModFac = max(0.0, EvapTotCapFlowModFac); // could warn if negative, DXcoil does

        // total cooling capacity
        Real64 EvapTotCap = thisTESCoil.CoolingAndChargeRatedTotCap * EvapTotCapTempModFac * EvapTotCapFlowModFac;

        // now see if coil is running dry
        Real64 const PartLoadOutAirEnth = EvapInletEnthalpy - (EvapTotCap * PartLoadRatio) / EvapAirMassFlow;
        Real64 const PartLoadDryCoilOutAirTemp = PsyTdbFnHW(PartLoadOutAirEnth, EvapInletHumRat);

        bool CoilMightBeDry = false;
        Real64 DryCoilTestEvapInletHumRat = 0.0;
        Real64 SHRadp = 0.0;
        if (PartLoadDryCoilOutAirTemp > PsyTsatFnHPb(state, PartLoadOutAirEnth, state.dataEnvrn->OutBaroPress, RoutineName)) {
            CoilMightBeDry = true;
            // find wADP, humidity ratio at apparatus dewpoint and inlet hum rat that would have dry coil
            DryCoilTestEvapInletHumRat = EvapInletHumRat;
            Real64 DryCoilTestEvapInletWetBulb = EvapInletWetBulb;
            int Counter = 0;
            bool Converged = false;
            while (!Converged) {
                EvapTotCapTempModFac = EnergyPlus::Curve::CurveValue(
                    state, thisTESCoil.CoolingAndChargeCoolingCapFTempCurve, DryCoilTestEvapInletWetBulb, CondInletTemp, sTES);
                EvapTotCapTempModFac = max(0.0, EvapTotCapTempModFac); // could warn if negative, DXcoil does
                EvapTotCapFlowModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeCoolingCapFFlowCurve, AirMassFlowRatio);
                EvapTotCapFlowModFac = max(0.0, EvapTotCapFlowModFac); // could warn if negative, DXcoil does
                EvapTotCap = thisTESCoil.CoolingAndChargeRatedTotCap * EvapTotCapTempModFac * EvapTotCapFlowModFac;
                // coil bypass factor = 0.0
                Real64 const hADP = EvapInletEnthalpy - (EvapTotCap / EvapAirMassFlow);
                Real64 const tADP = PsyTsatFnHPb(state, hADP, state.dataEnvrn->OutBaroPress, RoutineName);
                Real64 const wADP = min(EvapInletHumRat, PsyWFnTdbH(state, tADP, hADP, RoutineName));
                Real64 const hTinwADP = PsyHFnTdbW(EvapInletDryBulb, wADP);
                if ((EvapInletEnthalpy - hADP) > 1.e-10) {
                    SHRadp = min((hTinwADP - hADP) / (EvapInletEnthalpy - hADP), 1.0);
                } else {
                    SHRadp = 1.0;
                }

                if ((wADP > DryCoilTestEvapInletHumRat) || (Counter >= 1 && Counter < MaxIter)) {
                    if (DryCoilTestEvapInletHumRat <= 0.0) {
                        DryCoilTestEvapInletHumRat = 0.00001;
                    }
                    Real64 const werror = (DryCoilTestEvapInletHumRat - wADP) / DryCoilTestEvapInletHumRat;

                    DryCoilTestEvapInletHumRat = RelaxationFactor * wADP + (1.0 - RelaxationFactor) * DryCoilTestEvapInletHumRat;
                    DryCoilTestEvapInletWetBulb =
                        PsyTwbFnTdbWPb(state, EvapInletDryBulb, DryCoilTestEvapInletHumRat, state.dataEnvrn->OutBaroPress, RoutineName);

                    ++Counter;
                    Converged = (std::abs(werror) <= Tolerance);
                } else {
                    Converged = true;
                }
            }
        }

        // total cooling capacity modification factors
        Real64 const SHRTempFac =
            (state.dataCurveManager->curves(thisTESCoil.CoolingAndChargeSHRFTempCurve)->numDims == 2)
                ? EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeSHRFTempCurve, EvapInletWetBulb, EvapInletDryBulb)
                : EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeSHRFTempCurve, EvapInletWetBulb, EvapInletDryBulb, sTES);
        Real64 const SHRFlowFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeSHRFFlowCurve, AirMassFlowRatio);
        Real64 SHR = thisTESCoil.CoolingAndChargeRatedSHR * SHRTempFac * SHRFlowFac;
        SHR = min(SHR, 1.0); // warn maybe
        SHR = max(SHR, 0.0); // warn maybe
        if (CoilMightBeDry) {
            if ((EvapInletHumRat < DryCoilTestEvapInletHumRat) && (SHRadp > SHR)) { // coil is dry for sure
                SHR = 1.0;
            } else if (SHRadp > SHR) {
                SHR = SHRadp;
            }
        }

        // part load factor
        Real64 const PLF = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeCoolingPLFFPLRCurve, PartLoadRatio);
        // compressor running time divided by full time of timestep.
        Real64 EvapRuntimeFraction = 1.0;
        if (PLF >= PartLoadRatio && PLF > 0.0) {
            EvapRuntimeFraction = PartLoadRatio / PLF;
        } else {
            EvapRuntimeFraction = 1.0; // warn maybe
        }

        // Calculate electricity consumed. First, get EIR modifying factors for off-rated conditions
        Real64 EIRTempModFac =
            EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeCoolingEIRFTempCurve, EvapInletWetBulb, CondInletTemp, sTES);
        EIRTempModFac = max(EIRTempModFac, 0.0);

        Real64 EIRFlowModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeCoolingEIRFFlowCurve, AirMassFlowRatio);
        EIRFlowModFac = max(EIRFlowModFac, 0.0);

        Real64 const EIR = EIRTempModFac * EIRFlowModFac / thisTESCoil.CoolingAndChargeCoolingRatedCOP;

        Real64 const EvapElecCoolingPower = EvapTotCap * EIR * EvapRuntimeFraction;

        TotChargeCap = 0.0;
        Real64 ChargeRuntimeFraction = 0.0;
        Real64 ChargeElectricCoolingPower = 0.0;
        if (TESCanBeCharged) {
            Real64 ChargeCapModFac =
                EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeChargingCapFTempCurve, EvapInletWetBulb, CondInletTemp, sTES);
            ChargeCapModFac = max(0.0, ChargeCapModFac);

            Real64 ChargeCapPLRModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeChargingCapFEvapPLRCurve, PartLoadRatio);
            ChargeCapPLRModFac = max(0.0, ChargeCapPLRModFac);

            TotChargeCap = thisTESCoil.CoolingAndChargeRatedChargeCap * ChargeCapModFac * ChargeCapPLRModFac;
            if (TotChargeCap > QdotChargeLimit) {
                ChargeRuntimeFraction = QdotChargeLimit / TotChargeCap;
                TotChargeCap = min(TotChargeCap, QdotChargeLimit);
            } else {
                ChargeRuntimeFraction = 1.0;
            }
            Real64 ChargeEIRTempModFac =
                EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeChargingEIRFTempCurve, EvapInletWetBulb, CondInletTemp, sTES);
            ChargeEIRTempModFac = max(0.0, ChargeEIRTempModFac);

            Real64 ChargeEIRFlowModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeChargingEIRFFLowCurve, AirMassFlowRatio);
            ChargeEIRFlowModFac = max(0.0, ChargeEIRFlowModFac);

            Real64 const ChargeEIR = (ChargeEIRTempModFac * ChargeEIRFlowModFac) / thisTESCoil.CoolingAndChargeChargingRatedCOP;
            ChargeElectricCoolingPower = TotChargeCap * ChargeEIR;
            thisTESCoil.QdotTES = -TotChargeCap;
        } else {
            TotChargeCap = 0.0;
            ChargeElectricCoolingPower = 0.0;
            thisTESCoil.QdotTES = 0.0;
            ChargeRuntimeFraction = 0.0;
        }

        //  Calculate full load output conditions
        Real64 const FullLoadOutAirEnth = EvapInletEnthalpy - EvapTotCap / EvapAirMassFlow;

        Real64 const hTinwout = EvapInletEnthalpy - (1.0 - SHR) * (EvapTotCap / EvapAirMassFlow);
        // The following will often throw psych warnings for neg w, suppress warnings because error condition is handled in next IF
        Real64 FullLoadOutAirHumRat = PsyWFnTdbH(state, EvapInletDryBulb, hTinwout, RoutineName, true);
        Real64 FullLoadOutAirTemp = PsyTdbFnHW(FullLoadOutAirEnth, FullLoadOutAirHumRat);
        // Check for saturation error and modify temperature at constant enthalpy
        if (FullLoadOutAirTemp < PsyTsatFnHPb(state, FullLoadOutAirEnth, state.dataEnvrn->OutBaroPress, RoutineName)) {
            FullLoadOutAirTemp = PsyTsatFnHPb(state, FullLoadOutAirEnth, state.dataEnvrn->OutBaroPress, RoutineName);
            FullLoadOutAirHumRat = PsyWFnTdbH(state, FullLoadOutAirTemp, FullLoadOutAirEnth, RoutineName);
        }

        // Continuous fan, cycling compressor
        Real64 const EvapOutletAirEnthalpy = ((PartLoadRatio)*FullLoadOutAirEnth + (1.0 - (PartLoadRatio)) * EvapInletEnthalpy);
        Real64 EvapOutletAirHumRat = ((PartLoadRatio)*FullLoadOutAirHumRat + (1.0 - (PartLoadRatio)) * EvapInletHumRat);
        Real64 EvapOutletAirTemp = PsyTdbFnHW(EvapOutletAirEnthalpy, EvapOutletAirHumRat);
        if (EvapOutletAirTemp < PsyTsatFnHPb(state, EvapOutletAirEnthalpy, state.dataEnvrn->OutBaroPress, RoutineName)) {
            EvapOutletAirTemp = PsyTsatFnHPb(state, EvapOutletAirEnthalpy, state.dataEnvrn->OutBaroPress, RoutineName);
            EvapOutletAirHumRat = PsyWFnTdbH(state, EvapOutletAirTemp, EvapOutletAirEnthalpy, RoutineName);
        }

        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp = EvapOutletAirTemp;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat = EvapOutletAirHumRat;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Enthalpy = EvapOutletAirEnthalpy;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRate = EvapAirMassFlow;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMinAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMinAvail;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMaxAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMaxAvail;

        // determine condenser leaving conditions
        // condenser total heat rejection rate [W]
        Real64 const QdotCond = EvapTotCap * EvapRuntimeFraction + EvapElecCoolingPower + TotChargeCap + ChargeElectricCoolingPower;
        state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate = thisTESCoil.CondenserAirMassFlow;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).MassFlowRate = thisTESCoil.CondenserAirMassFlow;
        // condenser inlet enthalpy [J/kg]
        Real64 const CondInletEnthalpy = PsyHFnTdbW(CondInletTemp, CondInletHumRat);
        Real64 const CondOutletEnthalpy = CondInletEnthalpy + QdotCond / thisTESCoil.CondenserAirMassFlow;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp = PsyTdbFnHW(CondOutletEnthalpy, CondInletHumRat);
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat = CondInletHumRat;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Enthalpy = CondOutletEnthalpy;

        thisTESCoil.ElecCoolingPower = EvapElecCoolingPower + ChargeElectricCoolingPower + thisTESCoil.AncillaryControlsPower;
        thisTESCoil.ElecCoolingEnergy = thisTESCoil.ElecCoolingPower * TimeStepSysSec;

        thisTESCoil.RuntimeFraction = EvapRuntimeFraction;
        if (ChargeRuntimeFraction > 0.0) {
            thisTESCoil.CondenserRuntimeFraction = max(ChargeRuntimeFraction, EvapRuntimeFraction);
        } else {
            thisTESCoil.CondenserRuntimeFraction = EvapRuntimeFraction;
        }

        thisTESCoil.EvapTotCoolingRate = EvapTotCap * EvapRuntimeFraction; // double check this
        thisTESCoil.EvapTotCoolingEnergy = EvapTotCap * EvapRuntimeFraction * TimeStepSysSec;
        Real64 const MinAirHumRat = min(state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat,
                                        state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).HumRat);
        thisTESCoil.EvapSensCoolingRate =
            EvapAirMassFlow * (PsyHFnTdbW(EvapInletDryBulb, MinAirHumRat) - PsyHFnTdbW(EvapOutletAirTemp, MinAirHumRat));
        if (thisTESCoil.EvapSensCoolingRate > thisTESCoil.EvapTotCoolingRate) {
            thisTESCoil.EvapSensCoolingRate = thisTESCoil.EvapTotCoolingRate;
        }
        thisTESCoil.EvapSensCoolingEnergy = thisTESCoil.EvapSensCoolingRate * TimeStepSysSec;
        thisTESCoil.EvapLatCoolingRate = thisTESCoil.EvapTotCoolingRate - thisTESCoil.EvapSensCoolingRate;
        thisTESCoil.EvapLatCoolingEnergy = thisTESCoil.EvapLatCoolingRate * TimeStepSysSec;

    } else { // Evap off, but may still charge

        TotChargeCap = 0.0;
        Real64 ChargeElectricCoolingPower = 0.0;
        Real64 ChargeRuntimeFraction = 0.0;
        if (TESCanBeCharged) { // coil is running to charge but not to cool at evaporator
            Real64 const AirMassFlowRatio = EvapAirMassFlow / thisTESCoil.RatedEvapAirMassFlowRate;
            Real64 ChargeCapModFac =
                EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeChargingCapFTempCurve, EvapInletWetBulb, CondInletTemp, sTES);
            ChargeCapModFac = max(0.0, ChargeCapModFac);

            Real64 ChargeCapPLRModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeChargingCapFEvapPLRCurve, PartLoadRatio);
            ChargeCapPLRModFac = max(0.0, ChargeCapPLRModFac);

            TotChargeCap = thisTESCoil.CoolingAndChargeRatedChargeCap * ChargeCapModFac * ChargeCapPLRModFac;
            ChargeRuntimeFraction = 1.0;
            if (TotChargeCap > QdotChargeLimit) {
                ChargeRuntimeFraction = QdotChargeLimit / TotChargeCap;
                TotChargeCap = min(TotChargeCap, QdotChargeLimit);
            }
            Real64 ChargeEIRTempModFac =
                EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeChargingEIRFTempCurve, EvapInletWetBulb, CondInletTemp, sTES);
            ChargeEIRTempModFac = max(0.0, ChargeEIRTempModFac);

            Real64 ChargeEIRFlowModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndChargeChargingEIRFFLowCurve, AirMassFlowRatio);
            ChargeEIRFlowModFac = max(0.0, ChargeEIRFlowModFac);

            Real64 const ChargeEIR = (ChargeEIRTempModFac * ChargeEIRFlowModFac) / thisTESCoil.CoolingAndChargeChargingRatedCOP;
            ChargeElectricCoolingPower = TotChargeCap * ChargeEIR;
            thisTESCoil.QdotTES = -TotChargeCap;
        } else {
            TotChargeCap = 0.0;
            ChargeElectricCoolingPower = 0.0;
            thisTESCoil.QdotTES = 0.0;
            ChargeRuntimeFraction = 0.0;
        }

        thisTESCoil.ElecCoolingPower = ChargeElectricCoolingPower + thisTESCoil.AncillaryControlsPower;
        thisTESCoil.ElecCoolingEnergy = thisTESCoil.ElecCoolingPower * TimeStepSysSec;

        thisTESCoil.RuntimeFraction = 0.0;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).Temp;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).HumRat;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRate =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRate;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMinAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMinAvail;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMaxAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMaxAvail;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Enthalpy = PsyHFnTdbW(
            state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp, state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat);

        thisTESCoil.EvapTotCoolingRate = 0.0;
        thisTESCoil.EvapTotCoolingEnergy = 0.0;
        thisTESCoil.EvapSensCoolingRate = 0.0;
        thisTESCoil.EvapSensCoolingEnergy = 0.0;
        thisTESCoil.EvapLatCoolingRate = 0.0;
        thisTESCoil.EvapLatCoolingEnergy = 0.0;

        if (TotChargeCap == 0.0) {
            state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
            state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat;
            state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).MassFlowRate =
                state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate;
            state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Enthalpy = PsyHFnTdbW(
                state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp, state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat);
            thisTESCoil.CondenserRuntimeFraction = 0.0;
        } else {

            // determine condenser leaving conditions
            Real64 const QdotCond = TotChargeCap + ChargeElectricCoolingPower;
            state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate = thisTESCoil.CondenserAirMassFlow;
            state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).MassFlowRate = thisTESCoil.CondenserAirMassFlow;
            Real64 const CondInletEnthalpy = PsyHFnTdbW(CondInletTemp, CondInletHumRat);
            Real64 const CondOutletEnthalpy = CondInletEnthalpy + QdotCond / thisTESCoil.CondenserAirMassFlow;
            state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp = PsyTdbFnHW(CondOutletEnthalpy, CondInletHumRat);
            state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat = CondInletHumRat;
            state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Enthalpy = CondOutletEnthalpy;
            thisTESCoil.CondenserRuntimeFraction = 1.0;
        }
    }

    thisTESCoil.QdotTES = -TotChargeCap;
    thisTESCoil.Q_TES = thisTESCoil.QdotTES * TimeStepSysSec;

    UpdateTEStorage(state, TESCoilNum);

    thisTESCoil.CondInletTemp = CondInletTemp;

    UpdateColdWeatherProtection(state, TESCoilNum);

    if (thisTESCoil.CondenserType == TESCondenserType::Evap) {
        UpdateEvaporativeCondenserBasinHeater(state, TESCoilNum);
        UpdateEvaporativeCondenserWaterUse(state, TESCoilNum, CondInletHumRat, thisTESCoil.CondAirInletNodeNum);
    }
}

void CalcTESCoilCoolingAndDischargeMode(EnergyPlusData &state,
                                        int const TESCoilNum,
                                        [[maybe_unused]] HVAC::FanOp const fanOp,
                                        Real64 const PartLoadRatio)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   April 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // Using/Aliasing
    Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr MaxIter(30);
    Real64 constexpr RelaxationFactor(0.4);
    Real64 constexpr Tolerance(0.1);
    static constexpr std::string_view RoutineName("CalcTESCoilCoolingAndDischargeMode");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 CondInletTemp;   // Condenser inlet temperature (C). Outdoor dry-bulb temp for air-cooled condenser.
                            // Outdoor Wetbulb +(1 - effectiveness)*(outdoor drybulb - outdoor wetbulb) for evap condenser.
    Real64 CondInletHumRat; // Condenser inlet humidity ratio (kg/kg). Zero for air-cooled condenser.
                            // For evap condenser, its the humidity ratio of the air leaving the evap cooling pads.

    auto &thisTESCoil = state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum);

    // first deal with condenser
    if (thisTESCoil.CondenserType == TESCondenserType::Air) {
        Real64 const CondAirSidePressure = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Press;
        if (CondAirSidePressure == state.dataLoopNodes->DefaultNodeValues.Press) {
            CondInletTemp = state.dataEnvrn->OutDryBulbTemp;
            CondInletHumRat = state.dataEnvrn->OutHumRat;
        } else {
            CondInletTemp = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
            CondInletHumRat = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat;
        }
    } else if (thisTESCoil.CondenserType == TESCondenserType::Evap) {
        Real64 const CondAirSidePressure = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Press;
        Real64 OutdoorDryBulb = 0.0;
        Real64 OutdoorWetBulb = 0.0;
        if (CondAirSidePressure == state.dataLoopNodes->DefaultNodeValues.Press) {
            OutdoorDryBulb = state.dataEnvrn->OutDryBulbTemp;
            OutdoorWetBulb = state.dataEnvrn->OutWetBulbTemp;
        } else {
            OutdoorDryBulb = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
            Real64 const OutdoorHumRat = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat;
            OutdoorWetBulb = PsyTwbFnTdbWPb(state, OutdoorDryBulb, OutdoorHumRat, CondAirSidePressure, RoutineName);
        }
        // direct evap cool model
        CondInletTemp = OutdoorWetBulb + (OutdoorDryBulb - OutdoorWetBulb) * (1.0 - thisTESCoil.EvapCondEffect);
        CondInletHumRat = PsyWFnTdbTwbPb(state, CondInletTemp, OutdoorWetBulb, CondAirSidePressure, RoutineName);
    }

    Real64 QdotDischargeLimit; // limit for discharge cooling power to hit limit of storage.
    Real64 sTES;               // stat of Thermal energy storage [C or fraction of ice]
    bool TESHasSomeCharge = false;

    switch (thisTESCoil.StorageMedia) {
    case MediaType::Water:
    case MediaType::UserDefindFluid:
        sTES = thisTESCoil.FluidTankTempFinalLastTimestep;
        if ((sTES >= thisTESCoil.MinimumFluidTankTempLimit) && (sTES < thisTESCoil.MaximumFluidTankTempLimit)) {
            TESHasSomeCharge = true;
            Real64 const rho = thisTESCoil.glycol->getDensity(state, sTES, RoutineName);
            Real64 const TankMass = rho * thisTESCoil.FluidStorageVolume;
            Real64 const CpTank = thisTESCoil.glycol->getSpecificHeat(state, sTES, RoutineName);
            // simple linear approximation of DT/Dt term in McpDT/Dt
            QdotDischargeLimit = TankMass * CpTank * (thisTESCoil.MaximumFluidTankTempLimit - sTES) / TimeStepSysSec;
        } else {
            TESHasSomeCharge = false;
        }
        break;
    case MediaType::Ice:
        sTES = thisTESCoil.IceFracRemainLastTimestep;
        if (sTES > 0.0) {
            TESHasSomeCharge = true;
            // discharge limit
            QdotDischargeLimit = (sTES)*thisTESCoil.IceStorageCapacity / TimeStepSysSec;
        } else {
            TESHasSomeCharge = false;
        }
        break;
    default:
        break;
    }

    Real64 const EvapAirMassFlow = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRate;

    if ((EvapAirMassFlow > HVAC::SmallMassFlow) && (PartLoadRatio > 0.0)) { // coil is running

        Real64 const EvapInletDryBulb = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).Temp;
        Real64 const EvapInletHumRat = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).HumRat;
        Real64 const EvapInletWetBulb = PsyTwbFnTdbWPb(state, EvapInletDryBulb, EvapInletHumRat, state.dataEnvrn->OutBaroPress, RoutineName);
        Real64 const EvapInletEnthalpy = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).Enthalpy;

        // evaporator inlet air mass flow divided by design mass flow [ ]
        Real64 const AirMassFlowRatio = EvapAirMassFlow / thisTESCoil.RatedEvapAirMassFlowRate;

        // total cooling capacity modification factor due to temps []
        Real64 EvapTotCapTempModFac =
            EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndDischargeCoolingCapFTempCurve, EvapInletWetBulb, CondInletTemp, sTES);
        EvapTotCapTempModFac = max(0.0, EvapTotCapTempModFac); // could warn if negative, DXcoil does

        // total cooling capacity modification factor due to flow []
        Real64 EvapTotCapFlowModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndDischargeCoolingCapFFlowCurve, AirMassFlowRatio);
        EvapTotCapFlowModFac = max(0.0, EvapTotCapFlowModFac); // could warn if negative, DXcoil does

        // total cooling capacity
        Real64 EvapTotCap = thisTESCoil.CoolingAndDischargeRatedTotCap * EvapTotCapTempModFac * EvapTotCapFlowModFac;

        // now see if coil is running dry
        Real64 const PartLoadOutAirEnth = EvapInletEnthalpy - (EvapTotCap * PartLoadRatio) / EvapAirMassFlow;
        Real64 const PartLoadDryCoilOutAirTemp = PsyTdbFnHW(PartLoadOutAirEnth, EvapInletHumRat);

        bool CoilMightBeDry = false;
        Real64 DryCoilTestEvapInletHumRat = 0.0;
        Real64 SHRadp = 0.0;

        if (PartLoadDryCoilOutAirTemp > PsyTsatFnHPb(state, PartLoadOutAirEnth, state.dataEnvrn->OutBaroPress, RoutineName)) {
            CoilMightBeDry = true;
            // find wADP, humidity ratio at apparatus dewpoint and inlet hum rat that would have dry coil
            DryCoilTestEvapInletHumRat = EvapInletHumRat;
            Real64 DryCoilTestEvapInletWetBulb = EvapInletWetBulb;
            int Counter = 0;
            bool Converged = false;
            while (!Converged) {
                EvapTotCapTempModFac = EnergyPlus::Curve::CurveValue(
                    state, thisTESCoil.CoolingAndDischargeCoolingCapFTempCurve, DryCoilTestEvapInletWetBulb, CondInletTemp, sTES);
                EvapTotCapTempModFac = max(0.0, EvapTotCapTempModFac); // could warn if negative, DXcoil does
                EvapTotCapFlowModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndDischargeCoolingCapFFlowCurve, AirMassFlowRatio);
                EvapTotCapFlowModFac = max(0.0, EvapTotCapFlowModFac); // could warn if negative, DXcoil does
                EvapTotCap = thisTESCoil.CoolingAndDischargeRatedTotCap * EvapTotCapTempModFac * EvapTotCapFlowModFac;
                // coil bypass factor = 0.0
                Real64 const hADP = EvapInletEnthalpy - (EvapTotCap / EvapAirMassFlow);
                Real64 const tADP = PsyTsatFnHPb(state, hADP, state.dataEnvrn->OutBaroPress, RoutineName);
                Real64 const wADP = min(EvapInletHumRat, PsyWFnTdbH(state, tADP, hADP, RoutineName));
                Real64 const hTinwADP = PsyHFnTdbW(EvapInletDryBulb, wADP);
                if ((EvapInletEnthalpy - hADP) > 1.e-10) {
                    SHRadp = min((hTinwADP - hADP) / (EvapInletEnthalpy - hADP), 1.0);
                } else {
                    SHRadp = 1.0;
                }

                if ((wADP > DryCoilTestEvapInletHumRat) || (Counter >= 1 && Counter < MaxIter)) {
                    if (DryCoilTestEvapInletHumRat <= 0.0) {
                        DryCoilTestEvapInletHumRat = 0.00001;
                    }
                    Real64 const werror = (DryCoilTestEvapInletHumRat - wADP) / DryCoilTestEvapInletHumRat;

                    DryCoilTestEvapInletHumRat = RelaxationFactor * wADP + (1.0 - RelaxationFactor) * DryCoilTestEvapInletHumRat;
                    DryCoilTestEvapInletWetBulb =
                        PsyTwbFnTdbWPb(state, EvapInletDryBulb, DryCoilTestEvapInletHumRat, state.dataEnvrn->OutBaroPress, RoutineName);

                    ++Counter;
                    Converged = (std::abs(werror) <= Tolerance);
                } else {
                    Converged = true;
                }
            }
        }

        // total cooling capacity modification factors
        Real64 const SHRTempFac =
            (state.dataCurveManager->curves(thisTESCoil.CoolingAndDischargeSHRFTempCurve)->numDims == 2)
                ? EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndDischargeSHRFTempCurve, EvapInletWetBulb, EvapInletDryBulb)
                : EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndDischargeSHRFTempCurve, EvapInletWetBulb, EvapInletDryBulb, sTES);
        Real64 const SHRFlowFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndDischargeSHRFFlowCurve, AirMassFlowRatio);
        Real64 SHR = thisTESCoil.CoolingAndDischargeRatedSHR * SHRTempFac * SHRFlowFac;
        SHR = min(SHR, 1.0); // warn maybe
        SHR = max(SHR, 0.0); // warn maybe
        if (CoilMightBeDry) {
            if ((EvapInletHumRat < DryCoilTestEvapInletHumRat) && (SHRadp > SHR)) { // coil is dry for sure
                SHR = 1.0;
            } else if (SHRadp > SHR) {
                SHR = SHRadp;
            }
        }
        // part load factor
        Real64 const PLF = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndDischargeCoolingPLFFPLRCurve, PartLoadRatio);
        // compressor running time divided by full time of timestep.
        Real64 EvapRuntimeFraction = 1.0;
        if (PLF >= PartLoadRatio && PLF > 0.0) {
            EvapRuntimeFraction = PartLoadRatio / PLF;
        } else {
            EvapRuntimeFraction = 1.0; // warn maybe
        }
        // Calculate electricity consumed. First, get EIR modifying factors for off-rated conditions
        Real64 EIRTempModFac =
            EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndDischargeCoolingEIRFTempCurve, EvapInletWetBulb, CondInletTemp, sTES);
        EIRTempModFac = max(EIRTempModFac, 0.0);

        Real64 EIRFlowModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndDischargeCoolingEIRFFlowCurve, AirMassFlowRatio);
        EIRFlowModFac = max(EIRFlowModFac, 0.0);

        Real64 const EIR = EIRTempModFac * EIRFlowModFac / thisTESCoil.CoolingAndDischargeCoolingRatedCOP;

        Real64 const EvapElecCoolingPower = EvapTotCap * EIR * EvapRuntimeFraction;

        Real64 TotDischargeCap = 0.0;
        Real64 DischargeRuntimeFraction = 0.0;
        Real64 DischargeElectricCoolingPower = 0.0;
        if (TESHasSomeCharge) {
            Real64 DischargeCapTempModFac =
                EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndDischargeDischargingCapFTempCurve, EvapInletWetBulb, CondInletTemp, sTES);
            DischargeCapTempModFac = max(0.0, DischargeCapTempModFac);
            Real64 DischargeCapFlowModFac =
                EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndDischargeDischargingCapFFlowCurve, AirMassFlowRatio);
            DischargeCapFlowModFac = max(0.0, DischargeCapFlowModFac);

            Real64 const DischargePLF =
                EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndDischargeDischargingCapFEvapPLRCurve, PartLoadRatio);
            if (DischargePLF >= PartLoadRatio && DischargePLF > 0.0) {
                DischargeRuntimeFraction = PartLoadRatio / DischargePLF;
            } else {
                DischargeRuntimeFraction = 1.0; // warn maybe
            }

            TotDischargeCap =
                thisTESCoil.CoolingAndDischargeRatedDischargeCap * DischargeCapTempModFac * DischargeCapFlowModFac * DischargeRuntimeFraction;
            if (TotDischargeCap > QdotDischargeLimit) {
                TotDischargeCap = min(TotDischargeCap, QdotDischargeLimit);
            }
            Real64 DischargeEIRTempModFac =
                EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndDischargeDischargingEIRFTempCurve, EvapInletWetBulb, CondInletTemp, sTES);
            DischargeEIRTempModFac = max(0.0, DischargeEIRTempModFac);

            Real64 DischargeEIRFlowModFac =
                EnergyPlus::Curve::CurveValue(state, thisTESCoil.CoolingAndDischargeDischargingEIRFFLowCurve, AirMassFlowRatio);
            DischargeEIRFlowModFac = max(0.0, DischargeEIRFlowModFac);

            Real64 const DischargeEIR = (DischargeEIRTempModFac * DischargeEIRFlowModFac) / thisTESCoil.CoolingAndDischargeDischargingRatedCOP;
            DischargeElectricCoolingPower = TotDischargeCap * DischargeEIR * DischargeRuntimeFraction;
            thisTESCoil.QdotTES = TotDischargeCap;
        } else {
            TotDischargeCap = 0.0;
            DischargeRuntimeFraction = 0.0;
            DischargeElectricCoolingPower = 0.0;
            thisTESCoil.QdotTES = 0.0;
        }

        Real64 const TotCap = EvapTotCap + TotDischargeCap;
        //  Calculate full load output conditions

        Real64 const hTinwout = EvapInletEnthalpy - (1.0 - SHR) * (TotCap / EvapAirMassFlow);
        Real64 const FullLoadOutAirEnth = EvapInletEnthalpy - TotCap / EvapAirMassFlow;
        // The following will often throw psych warnings for neg w, suppress warnings because error condition is handled in next IF
        Real64 FullLoadOutAirHumRat = PsyWFnTdbH(state, EvapInletDryBulb, hTinwout, RoutineName, true);
        Real64 FullLoadOutAirTemp = PsyTdbFnHW(FullLoadOutAirEnth, FullLoadOutAirHumRat);
        // Check for saturation error and modify temperature at constant enthalpy
        if (FullLoadOutAirTemp < PsyTsatFnHPb(state, FullLoadOutAirEnth, state.dataEnvrn->OutBaroPress, RoutineName)) {
            FullLoadOutAirTemp = PsyTsatFnHPb(state, FullLoadOutAirEnth, state.dataEnvrn->OutBaroPress, RoutineName);
            FullLoadOutAirHumRat = PsyWFnTdbH(state, FullLoadOutAirTemp, FullLoadOutAirEnth, RoutineName);
        }
        // Continuous fan, cycling compressor
        Real64 const EvapOutletAirEnthalpy = ((PartLoadRatio)*FullLoadOutAirEnth + (1.0 - (PartLoadRatio)) * EvapInletEnthalpy);
        Real64 EvapOutletAirHumRat = ((PartLoadRatio)*FullLoadOutAirHumRat + (1.0 - (PartLoadRatio)) * EvapInletHumRat);
        Real64 EvapOutletAirTemp = PsyTdbFnHW(EvapOutletAirEnthalpy, EvapOutletAirHumRat);
        if (EvapOutletAirTemp < PsyTsatFnHPb(state, EvapOutletAirEnthalpy, state.dataEnvrn->OutBaroPress, RoutineName)) {
            EvapOutletAirTemp = PsyTsatFnHPb(state, EvapOutletAirEnthalpy, state.dataEnvrn->OutBaroPress, RoutineName);
            EvapOutletAirHumRat = PsyWFnTdbH(state, EvapOutletAirTemp, EvapOutletAirEnthalpy, RoutineName);
        }

        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp = EvapOutletAirTemp;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat = EvapOutletAirHumRat;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Enthalpy = EvapOutletAirEnthalpy;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRate = EvapAirMassFlow;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMinAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMinAvail;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMaxAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMaxAvail;

        // determine condenser leaving conditions
        Real64 const QdotCond = EvapTotCap * EvapRuntimeFraction + EvapElecCoolingPower;
        state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate = thisTESCoil.CondenserAirMassFlow;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).MassFlowRate = thisTESCoil.CondenserAirMassFlow;
        Real64 const CondInletEnthalpy = PsyHFnTdbW(CondInletTemp, CondInletHumRat);
        Real64 const CondOutletEnthalpy = CondInletEnthalpy + QdotCond / thisTESCoil.CondenserAirMassFlow;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp = PsyTdbFnHW(CondOutletEnthalpy, CondInletHumRat);
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat = CondInletHumRat;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Enthalpy = CondOutletEnthalpy;

        thisTESCoil.ElecCoolingPower = EvapElecCoolingPower + DischargeElectricCoolingPower + thisTESCoil.AncillaryControlsPower;
        thisTESCoil.ElecCoolingEnergy = thisTESCoil.ElecCoolingPower * TimeStepSysSec;
        thisTESCoil.RuntimeFraction =
            (EvapTotCap * EvapRuntimeFraction + TotDischargeCap * DischargeRuntimeFraction) / (EvapTotCap + TotDischargeCap);

        thisTESCoil.EvapTotCoolingRate = EvapTotCap * EvapRuntimeFraction + TotDischargeCap * DischargeRuntimeFraction;
        thisTESCoil.EvapTotCoolingEnergy = thisTESCoil.EvapTotCoolingRate * TimeStepSysSec;
        Real64 const MinAirHumRat = min(state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat,
                                        state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).HumRat);
        thisTESCoil.EvapSensCoolingRate =
            EvapAirMassFlow * (PsyHFnTdbW(EvapInletDryBulb, MinAirHumRat) - PsyHFnTdbW(EvapOutletAirTemp, MinAirHumRat));
        if (thisTESCoil.EvapSensCoolingRate > thisTESCoil.EvapTotCoolingRate) {
            thisTESCoil.EvapSensCoolingRate = thisTESCoil.EvapTotCoolingRate;
        }
        thisTESCoil.EvapSensCoolingEnergy = thisTESCoil.EvapSensCoolingRate * TimeStepSysSec;
        thisTESCoil.EvapLatCoolingRate = thisTESCoil.EvapTotCoolingRate - thisTESCoil.EvapSensCoolingRate;
        thisTESCoil.EvapLatCoolingEnergy = thisTESCoil.EvapLatCoolingRate * TimeStepSysSec;

    } else { // coil is off; just pass through conditions
        thisTESCoil.QdotTES = 0.0;

        thisTESCoil.ElecCoolingPower = thisTESCoil.AncillaryControlsPower;
        thisTESCoil.ElecCoolingEnergy = thisTESCoil.ElecCoolingPower * TimeStepSysSec;
        thisTESCoil.RuntimeFraction = 0.0;

        thisTESCoil.RuntimeFraction = 0.0;
        thisTESCoil.EvapTotCoolingRate = 0.0;
        thisTESCoil.EvapTotCoolingEnergy = 0.0;
        thisTESCoil.EvapSensCoolingRate = 0.0;
        thisTESCoil.EvapSensCoolingEnergy = 0.0;
        thisTESCoil.EvapLatCoolingRate = 0.0;
        thisTESCoil.EvapLatCoolingEnergy = 0.0;

        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).Temp;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).HumRat;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRate =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRate;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMinAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMinAvail;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMaxAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMaxAvail;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Enthalpy = PsyHFnTdbW(
            state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp, state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat);
        // nothing happens at condenser
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat;
        state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate = 0.0;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).MassFlowRate =
            state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Enthalpy = PsyHFnTdbW(
            state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp, state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat);
        thisTESCoil.CondInletTemp = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
    }
    thisTESCoil.Q_TES = thisTESCoil.QdotTES * TimeStepSysSec;
    UpdateTEStorage(state, TESCoilNum);

    UpdateColdWeatherProtection(state, TESCoilNum);

    if (thisTESCoil.CondenserType == TESCondenserType::Evap) {
        UpdateEvaporativeCondenserBasinHeater(state, TESCoilNum);
        UpdateEvaporativeCondenserWaterUse(state, TESCoilNum, CondInletHumRat, thisTESCoil.CondAirInletNodeNum);
    }
}

void CalcTESCoilChargeOnlyMode(EnergyPlusData &state, int const TESCoilNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   May 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // Using/Aliasing
    Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("CalcTESCoilChargeOnlyMode");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 CondInletTemp; // Condenser inlet temperature (C). Outdoor dry-bulb temp for air-cooled condenser.
    // Outdoor Wetbulb +(1 - effectiveness)*(outdoor drybulb - outdoor wetbulb) for evap condenser.
    Real64 CondInletHumRat; // Condenser inlet humidity ratio (kg/kg). Zero for air-cooled condenser.
                            // For evap condenser, its the humidity ratio of the air leaving the evap cooling pads.

    auto &thisTESCoil = state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum);

    // nothing happens at Evaporator
    state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).Temp;
    state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).HumRat;
    state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRate =
        state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRate;
    state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMinAvail =
        state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMinAvail;
    state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMaxAvail =
        state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMaxAvail;
    state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Enthalpy = PsyHFnTdbW(
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp, state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat);

    // first deal with condenser
    if (thisTESCoil.CondenserType == TESCondenserType::Air) {
        Real64 const CondAirSidePressure = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Press;
        if (CondAirSidePressure == state.dataLoopNodes->DefaultNodeValues.Press) {
            CondInletTemp = state.dataEnvrn->OutDryBulbTemp;
            CondInletHumRat = state.dataEnvrn->OutHumRat;
        } else {
            CondInletTemp = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
            CondInletHumRat = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat;
        }
    } else if (thisTESCoil.CondenserType == TESCondenserType::Evap) {
        Real64 const CondAirSidePressure = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Press;
        Real64 OutdoorDryBulb = 0.0;
        Real64 OutdoorWetBulb = 0.0;
        if (CondAirSidePressure == state.dataLoopNodes->DefaultNodeValues.Press) {
            OutdoorDryBulb = state.dataEnvrn->OutDryBulbTemp;
            OutdoorWetBulb = state.dataEnvrn->OutWetBulbTemp;
        } else {
            OutdoorDryBulb = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
            Real64 const OutdoorHumRat = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat;
            OutdoorWetBulb = PsyTwbFnTdbWPb(state, OutdoorDryBulb, OutdoorHumRat, CondAirSidePressure, RoutineName);
        }
        // direct evap cool model
        CondInletTemp = OutdoorWetBulb + (OutdoorDryBulb - OutdoorWetBulb) * (1.0 - thisTESCoil.EvapCondEffect);
        CondInletHumRat = PsyWFnTdbTwbPb(state, CondInletTemp, OutdoorWetBulb, CondAirSidePressure, RoutineName);
    }

    bool TESCanBeCharged = false; // true if room for tank to be charged.
    Real64 QdotChargeLimit;       // limit for charge cooling power to hit limit of storage.
    Real64 sTES;                  // local state of Thermal Energy Storage (C or ice fraction)

    switch (thisTESCoil.StorageMedia) {
    case MediaType::Water:
    case MediaType::UserDefindFluid:
        sTES = thisTESCoil.FluidTankTempFinalLastTimestep;
        if ((sTES > thisTESCoil.MinimumFluidTankTempLimit) && (sTES < thisTESCoil.MaximumFluidTankTempLimit)) {
            TESCanBeCharged = true;
            // find charge limit to reach limits
            // density of water in tank (kg/m3)
            Real64 const rho = thisTESCoil.glycol->getDensity(state, sTES, RoutineName);
            // Mass of water in tank (kg)
            Real64 const TankMass = rho * thisTESCoil.FluidStorageVolume;
            // Specific heat of water in tank (J/kg K)
            Real64 const CpTank = thisTESCoil.glycol->getSpecificHeat(state, sTES, RoutineName);
            // simple linear approximation of DT/Dt term in McpDT/Dt
            QdotChargeLimit = TankMass * CpTank * (sTES - thisTESCoil.MinimumFluidTankTempLimit) / TimeStepSysSec;
        } else {
            TESCanBeCharged = false;
        }
        break;
    case MediaType::Ice:
        sTES = thisTESCoil.IceFracRemainLastTimestep;
        if (sTES < 1.0) {
            TESCanBeCharged = true;
            // find charge limit to reach limit
            QdotChargeLimit = (1.0 - sTES) * thisTESCoil.IceStorageCapacity / TimeStepSysSec;
        } else {
            TESCanBeCharged = false;
        }
        break;
    default:
        break;
    }

    if (TESCanBeCharged) { // coil is running
        Real64 CapModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.ChargeOnlyChargingCapFTempCurve, CondInletTemp, sTES);
        CapModFac = max(0.0, CapModFac);
        Real64 TotCap = thisTESCoil.ChargeOnlyRatedCapacity * CapModFac;
        if (TotCap > QdotChargeLimit) {
            thisTESCoil.RuntimeFraction = QdotChargeLimit / TotCap;
            TotCap = min(TotCap, QdotChargeLimit);
        } else {
            thisTESCoil.RuntimeFraction = 1.0;
        }
        Real64 EIRModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.ChargeOnlyChargingEIRFTempCurve, CondInletTemp, sTES);
        EIRModFac = max(0.0, EIRModFac);

        Real64 const EIR = EIRModFac / thisTESCoil.ChargeOnlyRatedCOP;
        Real64 const ElecCoolingPower = TotCap * EIR;
        // condenser total heat rejection rate [W]
        Real64 const &QdotCond = TotCap + ElecCoolingPower;
        state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate = thisTESCoil.CondenserAirMassFlow;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).MassFlowRate = thisTESCoil.CondenserAirMassFlow;
        // condenser inlet enthalpy [J/kg]
        Real64 const CondInletEnthalpy = PsyHFnTdbW(CondInletTemp, CondInletHumRat);
        Real64 const CondOutletEnthalpy = CondInletEnthalpy + QdotCond / thisTESCoil.CondenserAirMassFlow;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp = PsyTdbFnHW(CondOutletEnthalpy, CondInletHumRat);
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat = CondInletHumRat;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Enthalpy = CondOutletEnthalpy;

        thisTESCoil.ElecCoolingPower = ElecCoolingPower + thisTESCoil.AncillaryControlsPower;
        thisTESCoil.ElecCoolingEnergy = thisTESCoil.ElecCoolingPower * TimeStepSysSec;

        thisTESCoil.QdotTES = -TotCap; // negative for cooling

    } else { // not running
        thisTESCoil.ElecCoolingPower = thisTESCoil.AncillaryControlsPower;
        thisTESCoil.ElecCoolingEnergy = thisTESCoil.ElecCoolingPower * TimeStepSysSec;
        thisTESCoil.RuntimeFraction = 0.0;
        thisTESCoil.QdotTES = 0.0;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat;
        state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate = 0.0;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).MassFlowRate =
            state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate;
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Enthalpy = PsyHFnTdbW(
            state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp, state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat);
    }
    thisTESCoil.Q_TES = thisTESCoil.QdotTES * TimeStepSysSec;

    thisTESCoil.EvapTotCoolingRate = 0.0;
    thisTESCoil.EvapTotCoolingEnergy = 0.0;
    thisTESCoil.EvapSensCoolingRate = 0.0;
    thisTESCoil.EvapSensCoolingEnergy = 0.0;
    thisTESCoil.EvapLatCoolingRate = 0.0;
    thisTESCoil.EvapLatCoolingEnergy = 0.0;

    UpdateTEStorage(state, TESCoilNum);

    UpdateColdWeatherProtection(state, TESCoilNum);

    if (thisTESCoil.CondenserType == TESCondenserType::Evap) {
        UpdateEvaporativeCondenserBasinHeater(state, TESCoilNum);
        UpdateEvaporativeCondenserWaterUse(state, TESCoilNum, CondInletHumRat, thisTESCoil.CondAirInletNodeNum);
    }
}

void CalcTESCoilDischargeOnlyMode(EnergyPlusData &state, int const TESCoilNum, Real64 const PartLoadRatio)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   April 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // Using/Aliasing
    Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr MaxIter(30);
    Real64 constexpr RelaxationFactor(0.4);
    Real64 constexpr Tolerance(0.1);
    static constexpr std::string_view RoutineName("CalcTESCoilDischargeOnlyMode");
    static constexpr std::string_view StorageTankName("CalcTESWaterStorageTank");

    auto &thisTESCoil = state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum);

    Real64 QdotDischargeLimit;     // limit for how much storage can be discharged without overshooting
    Real64 sTES = 0.0;             // state of charge of Thermal Energy Storage
    bool TESHasSomeCharge = false; // true when there is something available in storage

    switch (thisTESCoil.StorageMedia) {
    case MediaType::Water:
    case MediaType::UserDefindFluid:
        sTES = thisTESCoil.FluidTankTempFinalLastTimestep;
        if ((sTES >= thisTESCoil.MinimumFluidTankTempLimit) && (sTES < thisTESCoil.MaximumFluidTankTempLimit)) {
            TESHasSomeCharge = true;
            // density of water in tank (kg/m3)
            Real64 const rho = thisTESCoil.glycol->getDensity(state, sTES, StorageTankName);
            // Mass of water in tank (kg)
            Real64 const TankMass = rho * thisTESCoil.FluidStorageVolume;
            // Specific heat of water in tank (J/kg K)
            Real64 const CpTank = thisTESCoil.glycol->getSpecificHeat(state, sTES, StorageTankName);
            // simple linear approximation of DT/Dt term in McpDT/Dt
            QdotDischargeLimit = TankMass * CpTank * (thisTESCoil.MaximumFluidTankTempLimit - sTES) / TimeStepSysSec;
        } else {
            TESHasSomeCharge = false;
        }
        break;
    case MediaType::Ice:
        sTES = thisTESCoil.IceFracRemainLastTimestep;
        if (sTES > 0.0) {
            TESHasSomeCharge = true;
            // discharge limit
            QdotDischargeLimit = (sTES)*thisTESCoil.IceStorageCapacity / TimeStepSysSec;
        } else {
            TESHasSomeCharge = false;
        }
        break;
    default:
        break;
    }

    // local for evaporator air mass flow [kg/s]
    Real64 const EvapAirMassFlow = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRate;

    if ((EvapAirMassFlow > HVAC::SmallMassFlow) && (PartLoadRatio > 0.0) && TESHasSomeCharge) { // coil is running

        Real64 PLR = PartLoadRatio;

        Real64 const EvapInletDryBulb = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).Temp;
        Real64 const EvapInletHumRat = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).HumRat;
        Real64 const EvapInletWetBulb = PsyTwbFnTdbWPb(state, EvapInletDryBulb, EvapInletHumRat, state.dataEnvrn->OutBaroPress, RoutineName);
        Real64 const EvapInletEnthalpy = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).Enthalpy;

        // evaporator inlet air mass flow divided by design mass flow [ ]
        Real64 const AirMassFlowRatio = EvapAirMassFlow / thisTESCoil.RatedEvapAirMassFlowRate;

        // total cooling capacity modification factor due to temps []
        Real64 TotCapTempModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.DischargeOnlyCapFTempCurve, EvapInletWetBulb, sTES);
        TotCapTempModFac = max(0.0, TotCapTempModFac);
        // Total cooling capacity modification factor due to flow []
        Real64 TotCapFlowModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.DischargeOnlyCapFFlowCurve, AirMassFlowRatio);
        TotCapFlowModFac = max(0.0, TotCapFlowModFac);
        // total cooling capacity
        Real64 TotCap = thisTESCoil.DischargeOnlyRatedDischargeCap * TotCapTempModFac * TotCapFlowModFac;

        // part load factor
        Real64 const PLF = EnergyPlus::Curve::CurveValue(state, thisTESCoil.DischargeOnlyPLFFPLRCurve, PLR);
        // compressor running time divided by full time of timestep.
        Real64 RuntimeFraction = 1.0;
        if (PLF >= PLR && PLF > 0.0) {
            RuntimeFraction = PLR / PLF;
        } else {
            // warn maybe
        }
        // Calculate electricity consumed. First, get EIR modifying factors for off-rated conditions
        Real64 EIRTempModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.DischargeOnlyEIRFTempCurve, EvapInletWetBulb, sTES);
        EIRTempModFac = max(EIRTempModFac, 0.0);

        Real64 EIRFlowModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.DischargeOnlyEIRFFlowCurve, AirMassFlowRatio);
        EIRFlowModFac = max(EIRFlowModFac, 0.0);
        Real64 const EIR = EIRTempModFac * EIRFlowModFac / thisTESCoil.DischargeOnlyRatedCOP;

        // compressor electric power
        Real64 ElecCoolingPower = TotCap * EIR * RuntimeFraction;
        Real64 const QdotTEStest = TotCap * RuntimeFraction + ElecCoolingPower;

        if (QdotTEStest > QdotDischargeLimit) {
            Real64 const RuntimeFractionLimit = QdotDischargeLimit / (TotCap + TotCap * EIR);
            RuntimeFraction = min(RuntimeFraction, RuntimeFractionLimit);
            PLR = RuntimeFraction * PLF;
            ElecCoolingPower = TotCap * EIR * RuntimeFraction;
        }
        // now see if coil is running dry
        Real64 PartLoadOutAirEnth = EvapInletEnthalpy - (TotCap * PLR) / EvapAirMassFlow;
        Real64 const PartLoadDryCoilOutAirTemp = PsyTdbFnHW(PartLoadOutAirEnth, EvapInletHumRat);

        bool CoilMightBeDry = false;
        Real64 DryCoilTestEvapInletHumRat;
        Real64 SHRadp;
        if (PartLoadDryCoilOutAirTemp > PsyTsatFnHPb(state, PartLoadOutAirEnth, state.dataEnvrn->OutBaroPress, RoutineName)) {
            CoilMightBeDry = true;
            // find wADP, humidity ratio at apparatus dewpoint and inlet hum rat that would have dry coil
            DryCoilTestEvapInletHumRat = EvapInletHumRat;
            Real64 DryCoilTestEvapInletWetBulb = EvapInletWetBulb;
            int Counter = 0;
            bool Converged = false;
            while (!Converged) {
                TotCapTempModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.DischargeOnlyCapFTempCurve, DryCoilTestEvapInletWetBulb, sTES);
                TotCapTempModFac = max(0.0, TotCapTempModFac);
                TotCapFlowModFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.DischargeOnlyCapFFlowCurve, AirMassFlowRatio);
                TotCapFlowModFac = max(0.0, TotCapFlowModFac);
                TotCap = thisTESCoil.DischargeOnlyRatedDischargeCap * TotCapTempModFac * TotCapFlowModFac;
                // coil bypass factor = 0.0
                Real64 const hADP = EvapInletEnthalpy - (TotCap / EvapAirMassFlow);
                Real64 const tADP = PsyTsatFnHPb(state, hADP, state.dataEnvrn->OutBaroPress, RoutineName);
                Real64 const wADP = min(EvapInletHumRat, PsyWFnTdbH(state, tADP, hADP, RoutineName));
                Real64 const hTinwADP = PsyHFnTdbW(EvapInletDryBulb, wADP);
                if ((EvapInletEnthalpy - hADP) > 1.e-10) {
                    SHRadp = min((hTinwADP - hADP) / (EvapInletEnthalpy - hADP), 1.0);
                } else {
                    SHRadp = 1.0;
                }

                if ((wADP > DryCoilTestEvapInletHumRat) || (Counter >= 1 && Counter < MaxIter)) {
                    if (DryCoilTestEvapInletHumRat <= 0.0) {
                        DryCoilTestEvapInletHumRat = 0.00001;
                    }
                    Real64 const werror = (DryCoilTestEvapInletHumRat - wADP) / DryCoilTestEvapInletHumRat;

                    DryCoilTestEvapInletHumRat = RelaxationFactor * wADP + (1.0 - RelaxationFactor) * DryCoilTestEvapInletHumRat;
                    DryCoilTestEvapInletWetBulb =
                        PsyTwbFnTdbWPb(state, EvapInletDryBulb, DryCoilTestEvapInletHumRat, state.dataEnvrn->OutBaroPress, RoutineName);

                    ++Counter;
                    Converged = std::abs(werror) <= Tolerance;
                } else {
                    Converged = true;
                }
            }
        } // coil will be wet so use SHR curves

        // total cooling capacity modification factors
        Real64 SHRTempFac;
        if (state.dataCurveManager->curves(thisTESCoil.DischargeOnlySHRFTempCurve)->numDims == 2) {
            SHRTempFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.DischargeOnlySHRFTempCurve, EvapInletWetBulb, EvapInletDryBulb);
        } else {
            SHRTempFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.DischargeOnlySHRFTempCurve, EvapInletWetBulb, EvapInletDryBulb, sTES);
        }

        Real64 const SHRFlowFac = EnergyPlus::Curve::CurveValue(state, thisTESCoil.DischargeOnlySHRFFLowCurve, AirMassFlowRatio);
        Real64 SHR = thisTESCoil.DischargeOnlyRatedSHR * SHRTempFac * SHRFlowFac;
        SHR = min(SHR, 1.0); // warn maybe
        SHR = max(SHR, 0.0); // warn maybe
        if (CoilMightBeDry) {
            if ((EvapInletHumRat < DryCoilTestEvapInletHumRat) && (SHRadp > SHR)) { // coil is dry for sure
                SHR = 1.0;
            } else if (SHRadp > SHR) {
                SHR = SHRadp;
            }
        }
        //  Calculate full load output conditions
        // evaporator outlet full load enthalpy [J/kg]
        Real64 const FullLoadOutAirEnth = EvapInletEnthalpy - TotCap / EvapAirMassFlow;

        // Enthalpy at inlet dry-bulb and outlet humidity ratio [J/kg]
        Real64 const hTinwout = EvapInletEnthalpy - (1.0 - SHR) * (TotCap / EvapAirMassFlow);
        // The following will often throw psych warnings for neg w, suppress warnings because error condition is handled in next IF
        Real64 FullLoadOutAirHumRat = PsyWFnTdbH(state, EvapInletDryBulb, hTinwout, RoutineName, true);
        Real64 FullLoadOutAirTemp = PsyTdbFnHW(FullLoadOutAirEnth, FullLoadOutAirHumRat);
        // Check for saturation error and modify temperature at constant enthalpy
        if (FullLoadOutAirTemp < PsyTsatFnHPb(state, FullLoadOutAirEnth, state.dataEnvrn->OutBaroPress, RoutineName)) {
            FullLoadOutAirTemp = PsyTsatFnHPb(state, FullLoadOutAirEnth, state.dataEnvrn->OutBaroPress, RoutineName);
            FullLoadOutAirHumRat = PsyWFnTdbH(state, FullLoadOutAirTemp, FullLoadOutAirEnth, RoutineName);
        }

        // Continuous fan, cycling compressor
        Real64 const EvapOutletAirEnthalpy = ((PLR)*FullLoadOutAirEnth + (1.0 - (PLR)) * EvapInletEnthalpy);
        Real64 EvapOutletAirHumRat = ((PLR)*FullLoadOutAirHumRat + (1.0 - (PLR)) * EvapInletHumRat);
        Real64 EvapOutletAirTemp = PsyTdbFnHW(EvapOutletAirEnthalpy, EvapOutletAirHumRat);
        if (EvapOutletAirTemp < PsyTsatFnHPb(state, EvapOutletAirEnthalpy, state.dataEnvrn->OutBaroPress, RoutineName)) {
            EvapOutletAirTemp = PsyTsatFnHPb(state, EvapOutletAirEnthalpy, state.dataEnvrn->OutBaroPress, RoutineName);
            EvapOutletAirHumRat = PsyWFnTdbH(state, EvapOutletAirTemp, EvapOutletAirEnthalpy, RoutineName);
        }

        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp = EvapOutletAirTemp;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat = EvapOutletAirHumRat;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Enthalpy = EvapOutletAirEnthalpy;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRate = EvapAirMassFlow;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMinAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMinAvail;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMaxAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMaxAvail;
        thisTESCoil.ElecCoolingPower = ElecCoolingPower + thisTESCoil.AncillaryControlsPower;
        thisTESCoil.ElecCoolingEnergy = thisTESCoil.ElecCoolingPower * TimeStepSysSec;
        thisTESCoil.RuntimeFraction = RuntimeFraction;
        thisTESCoil.EvapTotCoolingRate = TotCap * RuntimeFraction; // double check this
        thisTESCoil.EvapTotCoolingEnergy = TotCap * RuntimeFraction * TimeStepSysSec;
        Real64 const MinAirHumRat = min(state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat,
                                        state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).HumRat);
        thisTESCoil.EvapSensCoolingRate =
            EvapAirMassFlow * (PsyHFnTdbW(EvapInletDryBulb, MinAirHumRat) - PsyHFnTdbW(EvapOutletAirTemp, MinAirHumRat));
        if (thisTESCoil.EvapSensCoolingRate > thisTESCoil.EvapTotCoolingRate) {
            thisTESCoil.EvapSensCoolingRate = thisTESCoil.EvapTotCoolingRate;
        }
        thisTESCoil.EvapSensCoolingEnergy = thisTESCoil.EvapSensCoolingRate * TimeStepSysSec;
        thisTESCoil.EvapLatCoolingRate = thisTESCoil.EvapTotCoolingRate - thisTESCoil.EvapSensCoolingRate;
        thisTESCoil.EvapLatCoolingEnergy = thisTESCoil.EvapLatCoolingRate * TimeStepSysSec;

        thisTESCoil.QdotTES = TotCap * RuntimeFraction + ElecCoolingPower; // all heat rejection into storage

    } else { // coil is off; just pass through conditions
        thisTESCoil.QdotTES = 0.0;

        thisTESCoil.ElecCoolingPower = thisTESCoil.AncillaryControlsPower;
        thisTESCoil.ElecCoolingEnergy = thisTESCoil.ElecCoolingPower * TimeStepSysSec;
        thisTESCoil.RuntimeFraction = 0.0;

        thisTESCoil.RuntimeFraction = 0.0;
        thisTESCoil.EvapTotCoolingRate = 0.0;
        thisTESCoil.EvapTotCoolingEnergy = 0.0;
        thisTESCoil.EvapSensCoolingRate = 0.0;
        thisTESCoil.EvapSensCoolingEnergy = 0.0;
        thisTESCoil.EvapLatCoolingRate = 0.0;
        thisTESCoil.EvapLatCoolingEnergy = 0.0;

        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).Temp;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat = state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).HumRat;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRate =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRate;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMinAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMinAvail;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).MassFlowRateMaxAvail =
            state.dataLoopNodes->Node(thisTESCoil.EvapAirInletNodeNum).MassFlowRateMaxAvail;
        state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Enthalpy = PsyHFnTdbW(
            state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).Temp, state.dataLoopNodes->Node(thisTESCoil.EvapAirOutletNodeNum).HumRat);
    }

    // nothing happens at condenser
    state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
    state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat;
    state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate = 0.0;
    state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).MassFlowRate =
        state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).MassFlowRate;
    state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Enthalpy = PsyHFnTdbW(
        state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).Temp, state.dataLoopNodes->Node(thisTESCoil.CondAirOutletNodeNum).HumRat);
    thisTESCoil.CondInletTemp = state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).Temp;
    thisTESCoil.Q_TES = thisTESCoil.QdotTES * TimeStepSysSec;
    UpdateTEStorage(state, TESCoilNum);

    UpdateColdWeatherProtection(state, TESCoilNum);

    if (thisTESCoil.CondenserType == TESCondenserType::Evap) {
        UpdateEvaporativeCondenserBasinHeater(state, TESCoilNum);
        UpdateEvaporativeCondenserWaterUse(
            state, TESCoilNum, state.dataLoopNodes->Node(thisTESCoil.CondAirInletNodeNum).HumRat, thisTESCoil.CondAirInletNodeNum);
    }
}

void UpdateTEStorage(EnergyPlusData &state, int const TESCoilNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         <author>
    //       DATE WRITTEN   <date_written>
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    switch (state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).StorageMedia) {
    case MediaType::Water:
    case MediaType::UserDefindFluid:
        CalcTESWaterStorageTank(state, TESCoilNum);
        break;
    case MediaType::Ice:
        CalcTESIceStorageTank(state, TESCoilNum);
        break;
    default:
        break;
    }
}

void CalcTESWaterStorageTank(EnergyPlusData &state, int const TESCoilNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         <author>
    //       DATE WRITTEN   <date_written>
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("CalcTESWaterStorageTank");
    static constexpr std::string_view calcTESIceStorageTank("CalcTESIceStorageTank");

    // Using/Aliasing
    Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
    using WaterThermalTanks::WaterThermalTankData;

    auto &thisTESCoil = state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum);

    // Seconds in one timestep (s)
    Real64 const SecInTimeStep = TimeStepSysSec;
    // Time remaining in the current timestep (s)
    Real64 const TimeRemaining = SecInTimeStep;

    // Fraction of the current hour that has elapsed (h)
    Real64 const TimeElapsed =
        state.dataGlobal->HourOfDay + state.dataGlobal->TimeStep * state.dataGlobal->TimeStepZone + state.dataHVACGlobal->SysTimeElapsed;

    if (thisTESCoil.TimeElapsed != TimeElapsed) {
        thisTESCoil.FluidTankTempFinalLastTimestep = thisTESCoil.FluidTankTempFinal;
        thisTESCoil.TimeElapsed = TimeElapsed;
    }

    // Instantaneous tank temperature (C)
    Real64 const TankTemp = thisTESCoil.FluidTankTempFinalLastTimestep;
    // Current ambient air temperature around tank (C)
    Real64 const AmbientTemp = state.dataLoopNodes->Node(thisTESCoil.StorageAmbientNodeNum).Temp;
    // Use side inlet temperature (C)
    Real64 const UseInletTemp = state.dataLoopNodes->Node(thisTESCoil.TESPlantInletNodeNum).Temp;
    // Source side inlet temperature (C)
    Real64 const SourceInletTemp = thisTESCoil.FluidTankTempFinalLastTimestep;
    // density of water in tank (kg/m3)
    Real64 const rho = thisTESCoil.glycol->getDensity(state, TankTemp, RoutineName);
    // Mass of water in tank (kg)
    Real64 const TankMass = rho * thisTESCoil.FluidStorageVolume;
    // Specific heat of water in tank (J/kg K)
    Real64 const CpTank = thisTESCoil.glycol->getSpecificHeat(state, TankTemp, RoutineName);

    // Use side flow rate, including effectiveness factor (kg/s)
    Real64 const UseMassFlowRate = thisTESCoil.TESPlantConnectionAvailable
                                       ? state.dataLoopNodes->Node(thisTESCoil.TESPlantInletNodeNum).MassFlowRate * thisTESCoil.TESPlantEffectiveness
                                       : 0.0;

    // Source side flow rate, including effectiveness factor (kg/s)
    Real64 constexpr SourceMassFlowRate = 0.0;
    // Loss coefficient to ambient environment (W/K)
    Real64 const LossCoeff = thisTESCoil.StorageUA;
    // heat exchange directly into tank from charging system [W]
    Real64 const QdotTES = thisTESCoil.QdotTES;

    // Predicted new tank temperature (C)
    Real64 const NewTankTemp = WaterThermalTanks::WaterThermalTankData::CalcTankTemp(TankTemp,
                                                                                     AmbientTemp,
                                                                                     UseInletTemp,
                                                                                     SourceInletTemp,
                                                                                     TankMass,
                                                                                     CpTank,
                                                                                     UseMassFlowRate,
                                                                                     SourceMassFlowRate,
                                                                                     LossCoeff,
                                                                                     QdotTES,
                                                                                     TimeRemaining);

    thisTESCoil.FluidTankTempFinal = NewTankTemp;

    if (thisTESCoil.TESPlantConnectionAvailable) {
        // Specific heat of fluid in plant connection (J/kg K)
        Real64 const CpPlantConnection =
            state.dataPlnt->PlantLoop(thisTESCoil.TESPlantLoopNum)
                .glycol->getSpecificHeat(state, state.dataLoopNodes->Node(thisTESCoil.TESPlantInletNodeNum).Temp, calcTESIceStorageTank);

        thisTESCoil.QdotPlant = state.dataLoopNodes->Node(thisTESCoil.TESPlantInletNodeNum).MassFlowRate * CpPlantConnection *
                                thisTESCoil.TESPlantEffectiveness * (UseInletTemp - NewTankTemp);
        thisTESCoil.Q_Plant = thisTESCoil.QdotPlant * TimeStepSysSec;
        // now get correct outlet temp with actual massflow (not modified by effectiveness)
        Real64 NewOutletTemp = UseInletTemp; // calculated new tankoutlet temp (C)
        if (state.dataLoopNodes->Node(thisTESCoil.TESPlantInletNodeNum).MassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
            NewOutletTemp =
                UseInletTemp - thisTESCoil.QdotPlant / (state.dataLoopNodes->Node(thisTESCoil.TESPlantInletNodeNum).MassFlowRate * CpPlantConnection);
        }
        state.dataLoopNodes->Node(thisTESCoil.TESPlantOutletNodeNum).Temp = NewOutletTemp;
    }

    // Change in integrated tank temperature, dividing by time gives the average (C s)
    Real64 const deltaTsum = WaterThermalTankData::CalcTempIntegral(TankTemp,
                                                                    NewTankTemp,
                                                                    AmbientTemp,
                                                                    UseInletTemp,
                                                                    SourceInletTemp,
                                                                    TankMass,
                                                                    CpTank,
                                                                    UseMassFlowRate,
                                                                    SourceMassFlowRate,
                                                                    LossCoeff,
                                                                    QdotTES,
                                                                    TimeRemaining);
    thisTESCoil.QdotAmbient = (LossCoeff * (AmbientTemp * TimeRemaining - deltaTsum)) / SecInTimeStep;
    thisTESCoil.Q_Ambient = thisTESCoil.QdotAmbient * TimeStepSysSec;
}

void CalcTESIceStorageTank(EnergyPlusData &state, int const TESCoilNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         <author>
    //       DATE WRITTEN   <date_written>
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // Using/Aliasing
    Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr Real64 FreezingTemp(0.0); // zero degrees C
    static constexpr std::string_view RoutineName("CalcTESIceStorageTank");

    auto &thisTESCoil = state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum);

    // Fraction of the current hour that has elapsed (h)
    Real64 const TimeElapsed =
        state.dataGlobal->HourOfDay + state.dataGlobal->TimeStep * state.dataGlobal->TimeStepZone + state.dataHVACGlobal->SysTimeElapsed;

    if (thisTESCoil.TimeElapsed != TimeElapsed) {
        thisTESCoil.IceFracRemainLastTimestep = thisTESCoil.IceFracRemain;
        thisTESCoil.TimeElapsed = TimeElapsed;
    }

    // update plant connection (if any)
    if (thisTESCoil.TESPlantConnectionAvailable) {

        auto const &inletNode = state.dataLoopNodes->Node(thisTESCoil.TESPlantInletNodeNum);
        Real64 const Cp = state.dataPlnt->PlantLoop(thisTESCoil.TESPlantLoopNum).glycol->getSpecificHeat(state, inletNode.Temp, RoutineName);

        thisTESCoil.QdotPlant = inletNode.MassFlowRate * Cp * thisTESCoil.TESPlantEffectiveness * (inletNode.Temp - FreezingTemp);
        thisTESCoil.Q_Plant = thisTESCoil.QdotPlant * TimeStepSysSec;
        // now get correct outlet temp with actual massflow (not modified by effectiveness)
        Real64 NewOutletTemp = inletNode.Temp; // calculated new tank outlet temp (C)
        if (inletNode.MassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
            NewOutletTemp += thisTESCoil.QdotPlant / (inletNode.MassFlowRate * Cp);
        }
        state.dataLoopNodes->Node(thisTESCoil.TESPlantOutletNodeNum).Temp = NewOutletTemp;
    } else {
        thisTESCoil.QdotPlant = 0.0;
        thisTESCoil.Q_Plant = 0.0;
    }

    // update ambient heat transfer

    thisTESCoil.QdotAmbient = thisTESCoil.StorageUA * (state.dataLoopNodes->Node(thisTESCoil.StorageAmbientNodeNum).Temp - FreezingTemp);
    thisTESCoil.Q_Ambient = thisTESCoil.QdotAmbient * TimeStepSysSec;

    // local rate of heat transfer to ice (negative cooling) [W]
    Real64 const QdotIce = thisTESCoil.QdotPlant + thisTESCoil.QdotAmbient + thisTESCoil.QdotTES;

    if (QdotIce < 0.0) { // charging ice level
        thisTESCoil.IceFracRemain = thisTESCoil.IceFracRemainLastTimestep + std::abs(QdotIce) / (thisTESCoil.IceStorageCapacity / TimeStepSysSec);
        if (thisTESCoil.IceFracRemain > 1.0) {
            thisTESCoil.IceFracRemain = 1.0;
        }
    } else { // not charging,but discharging
        thisTESCoil.IceFracRemain = thisTESCoil.IceFracRemainLastTimestep - QdotIce / (thisTESCoil.IceStorageCapacity / TimeStepSysSec);
        if (thisTESCoil.IceFracRemain < 0.0) {
            thisTESCoil.IceFracRemain = 0.0;
        }
    }
}

void UpdateColdWeatherProtection(EnergyPlusData &state, int const TESCoilNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   April 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // Using/Aliasing
    Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    if ((state.dataLoopNodes->Node(state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).StorageAmbientNodeNum).Temp <
         state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).ColdWeatherMinimumTempLimit) &&
        (state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).availSched->getCurrentVal() != 0.0)) {
        state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).ElectColdWeatherPower =
            state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).ColdWeatherAncillaryPower;

    } else {
        state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).ElectColdWeatherPower = 0.0;
    }
    state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).ElectColdWeatherEnergy =
        state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).ElectColdWeatherPower * TimeStepSysSec;
}

void UpdateEvaporativeCondenserBasinHeater(EnergyPlusData &state, int const TESCoilNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   April 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // determine basin heater electrical power and energy

    // Using/Aliasing
    Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    CalcBasinHeaterPower(state,
                         state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).BasinHeaterPowerFTempDiff,
                         state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).basinHeaterAvailSched,
                         state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).BasinHeaterSetpointTemp,
                         state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).ElectEvapCondBasinHeaterPower);

    state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).ElectEvapCondBasinHeaterEnergy =
        state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).ElectEvapCondBasinHeaterPower * TimeStepSysSec;
}

void UpdateEvaporativeCondenserWaterUse(EnergyPlusData &state, int const TESCoilNum, Real64 const HumRatAfterEvap, int const InletNodeNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   June 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // update and calculate water consumption for evaporatively cooled condensers

    Real64 const RhoWater = RhoH2O(state.dataLoopNodes->Node(InletNodeNum).Temp);
    state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterConsumpRate =
        (HumRatAfterEvap - state.dataLoopNodes->Node(InletNodeNum).HumRat) * state.dataLoopNodes->Node(InletNodeNum).MassFlowRate / RhoWater *
        state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).CondenserRuntimeFraction;

    // Set the demand request for supply water from water storage tank (if needed)
    if (state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterSupplyMode == EvapWaterSupply::WaterSupplyFromTank) {
        state.dataWaterData->WaterStorage(state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterSupTankID)
            .VdotRequestDemand(state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterTankDemandARRID) =
            state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterConsumpRate;
    }

    // check if should be starved by restricted flow from tank
    if (state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterSupplyMode == EvapWaterSupply::WaterSupplyFromTank) {
        Real64 const AvailWaterRate = state.dataWaterData->WaterStorage(state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterSupTankID)
                                          .VdotAvailDemand(state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterTankDemandARRID);
        if (AvailWaterRate < state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterConsumpRate) {
            state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterStarvMakupRate =
                state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterConsumpRate - AvailWaterRate;
            state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterConsumpRate = AvailWaterRate;
        } else {
            state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterStarvMakupRate = 0.0;
        }
    }

    Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
    state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterConsump =
        state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterConsumpRate * TimeStepSysSec;

    state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterStarvMakup =
        state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapWaterStarvMakupRate * TimeStepSysSec;

    state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapCondPumpElecPower =
        state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapCondPumpElecNomPower *
        state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).CondenserRuntimeFraction;
    state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapCondPumpElecConsumption =
        state.dataPackagedThermalStorageCoil->TESCoil(TESCoilNum).EvapCondPumpElecPower * TimeStepSysSec;
}

void GetTESCoilIndex(
    EnergyPlusData &state, std::string const &CoilName, int &CoilIndex, bool &ErrorsFound, std::string_view const CurrentModuleObject)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   August 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets an index for a given TES Cooling Coil -- issues error message if that
    // coil is not a legal TES Cooling Coil.

    // Obtains and allocates TESCoil related parameters from input file
    if (state.dataPackagedThermalStorageCoil->GetTESInputFlag) { // First time subroutine has been called, get input data
        GetTESCoilInput(state);
        state.dataPackagedThermalStorageCoil->GetTESInputFlag =
            false; // Set logic flag to disallow getting the input data on future calls to this subroutine
    }

    if (state.dataPackagedThermalStorageCoil->NumTESCoils > 0) {
        CoilIndex = Util::FindItem(CoilName, state.dataPackagedThermalStorageCoil->TESCoil);
    } else {
        CoilIndex = 0;
    }

    if (CoilIndex == 0) {
        if (!CurrentModuleObject.empty()) {
            ShowSevereError(state, std::format("{}, GetTESCoilIndex: TES Cooling Coil not found={}", CurrentModuleObject, CoilName));
        } else {
            ShowSevereError(state, std::format("GetTESCoilIndex: TES Cooling Coil not found={}", CoilName));
        }
        ErrorsFound = true;
    }
}

void GetTESCoilAirInletNode(
    EnergyPlusData &state, std::string const &CoilName, int &CoilAirInletNode, bool &ErrorsFound, std::string const &CurrentModuleObject)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   July 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine gets a given TES Cooling Coil's air inlet node -- issues error message if that
    // coil is not a legal TES Cooling Coil and sets air node to 0, otherwise, returns inlet air node number.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int CoilIndex;

    // Obtains and allocates TESCoil related parameters from input file
    if (state.dataPackagedThermalStorageCoil->GetTESInputFlag) { // First time subroutine has been called, get input data
        GetTESCoilInput(state);
        state.dataPackagedThermalStorageCoil->GetTESInputFlag =
            false; // Set logic flag to disallow getting the input data on future calls to this subroutine
    }

    if (state.dataPackagedThermalStorageCoil->NumTESCoils > 0) {
        CoilIndex = Util::FindItem(CoilName, state.dataPackagedThermalStorageCoil->TESCoil, state.dataPackagedThermalStorageCoil->NumTESCoils);
    } else {
        CoilIndex = 0;
    }

    if (CoilIndex == 0) {
        ShowSevereError(state, std::format("{}, GetTESCoilAirInletNode: TES Cooling Coil not found={}", CurrentModuleObject, CoilName));
        ErrorsFound = true;
        CoilAirInletNode = 0;
    } else {
        CoilAirInletNode = state.dataPackagedThermalStorageCoil->TESCoil(CoilIndex).EvapAirInletNodeNum;
    }
}

void GetTESCoilAirOutletNode(
    EnergyPlusData &state, std::string const &CoilName, int &CoilAirOutletNode, bool &ErrorsFound, std::string const &CurrentModuleObject)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   July 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine gets a given TES Cooling Coil's air outlet node -- issues error message if that
    // coil is not a legal TES Cooling Coil and sets air node to 0, otherwise, returns outlet air node number.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int CoilIndex;

    // Obtains and allocates TESCoil related parameters from input file
    if (state.dataPackagedThermalStorageCoil->GetTESInputFlag) { // First time subroutine has been called, get input data
        GetTESCoilInput(state);
        state.dataPackagedThermalStorageCoil->GetTESInputFlag =
            false; // Set logic flag to disallow getting the input data on future calls to this subroutine
    }

    if (state.dataPackagedThermalStorageCoil->NumTESCoils > 0) {
        CoilIndex = Util::FindItem(CoilName, state.dataPackagedThermalStorageCoil->TESCoil, state.dataPackagedThermalStorageCoil->NumTESCoils);
    } else {
        CoilIndex = 0;
    }

    if (CoilIndex == 0) {
        ShowSevereError(state, std::format("{}, GetTESCoilAirOutletNode: TES Cooling Coil not found={}", CurrentModuleObject, CoilName));
        ErrorsFound = true;
        CoilAirOutletNode = 0;
    } else {
        CoilAirOutletNode = state.dataPackagedThermalStorageCoil->TESCoil(CoilIndex).EvapAirOutletNodeNum;
    }
}

void GetTESCoilCoolingCapacity(
    EnergyPlusData &state, std::string const &CoilName, Real64 &CoilCoolCapacity, bool &ErrorsFound, std::string const &CurrentModuleObject)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   July 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine gets a given TES Cooling Coil's cooling only capacity -- issues error message if that
    // coil is not a legal TES Cooling Coil and sets capacity to 0, otherwise, returns cooling capacity.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int CoilIndex;

    // Obtains and allocates TESCoil related parameters from input file
    if (state.dataPackagedThermalStorageCoil->GetTESInputFlag) { // First time subroutine has been called, get input data
        GetTESCoilInput(state);
        state.dataPackagedThermalStorageCoil->GetTESInputFlag =
            false; // Set logic flag to disallow getting the input data on future calls to this subroutine
    }

    if (state.dataPackagedThermalStorageCoil->NumTESCoils > 0) {
        CoilIndex = Util::FindItem(CoilName, state.dataPackagedThermalStorageCoil->TESCoil, state.dataPackagedThermalStorageCoil->NumTESCoils);
    } else {
        CoilIndex = 0;
    }

    if (CoilIndex == 0) {
        ShowSevereError(state, std::format("{}, GetTESCoilCoolingCapacity: TES Cooling Coil not found={}", CurrentModuleObject, CoilName));
        ErrorsFound = true;
        CoilCoolCapacity = 0.0;
    } else {
        if (state.dataPackagedThermalStorageCoil->TESCoil(CoilIndex).CoolingOnlyModeIsAvailable) { // get input data for this mode
            CoilCoolCapacity = state.dataPackagedThermalStorageCoil->TESCoil(CoilIndex).CoolingOnlyRatedTotCap;
        } else if (state.dataPackagedThermalStorageCoil->TESCoil(CoilIndex).CoolingAndChargeModeAvailable) {
            CoilCoolCapacity = state.dataPackagedThermalStorageCoil->TESCoil(CoilIndex).CoolingAndChargeRatedTotCap;
        } else if (state.dataPackagedThermalStorageCoil->TESCoil(CoilIndex).CoolingAndDischargeModeAvailable) {
            CoilCoolCapacity = state.dataPackagedThermalStorageCoil->TESCoil(CoilIndex).CoolingAndDischargeRatedTotCap;
        } else {
            CoilCoolCapacity = 0.0;
        }
    }
}

void GetTESCoilCoolingAirFlowRate(
    EnergyPlusData &state, std::string const &CoilName, Real64 &CoilCoolAirFlow, bool &ErrorsFound, std::string const &CurrentModuleObject)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   September 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine gets a given TES Cooling Coil's evaporator air flow rate -- issues error message if that
    // coil is not a legal TES Cooling Coil and sets air flow to 0, otherwise, returns cooling air flow rate.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int CoilIndex;

    // Obtains and allocates TESCoil related parameters from input file
    if (state.dataPackagedThermalStorageCoil->GetTESInputFlag) { // First time subroutine has been called, get input data
        GetTESCoilInput(state);
        state.dataPackagedThermalStorageCoil->GetTESInputFlag =
            false; // Set logic flag to disallow getting the input data on future calls to this subroutine
    }

    if (state.dataPackagedThermalStorageCoil->NumTESCoils > 0) {
        CoilIndex = Util::FindItem(CoilName, state.dataPackagedThermalStorageCoil->TESCoil, state.dataPackagedThermalStorageCoil->NumTESCoils);
    } else {
        CoilIndex = 0;
    }

    if (CoilIndex == 0) {
        ShowSevereError(state, std::format("{}, GetTESCoilCoolingCapacity: TES Cooling Coil not found={}", CurrentModuleObject, CoilName));
        ErrorsFound = true;
        CoilCoolAirFlow = 0.0;
    } else {
        CoilCoolAirFlow = state.dataPackagedThermalStorageCoil->TESCoil(CoilIndex).RatedEvapAirVolFlowRate;
    }
}

} // namespace EnergyPlus::PackagedThermalStorageCoil
