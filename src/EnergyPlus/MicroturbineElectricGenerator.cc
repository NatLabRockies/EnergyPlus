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
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MicroturbineElectricGenerator.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::MicroturbineElectricGenerator {

// MODULE INFORMATION:
//       AUTHOR         R. Raustad/D. Shirey
//       DATE WRITTEN   Mar 2008
//       MODIFIED       na
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
//  This module simulates the performance of microturbine electric
//  generators.

// METHODOLOGY EMPLOYED:
//  Once the electric power manager determines that the MT Generator
//  is available, it calls SimMTGenerator which in turn calls the
//  appropriate microturbine generator model.
//  MT Generator models are based on polynomial curve fits of generator
//  performance data.

PlantComponent *MTGeneratorSpecs::factory(EnergyPlusData &state, std::string const &objectName)
{
    // Process the input data for generator if it hasn't been done already
    if (state.dataMircoturbElectGen->GetMTInput) {
        GetMTGeneratorInput(state);
        state.dataMircoturbElectGen->GetMTInput = false;
    }

    // Now look for this particular gen in the list
    for (auto &thisMTG : state.dataMircoturbElectGen->MTGenerator) {
        if (thisMTG.Name == objectName) {
            return &thisMTG;
        }
    }
    // If we didn't find it, fatal
    ShowFatalError(
        state, format("LocalMicroTurbineGeneratorFactory: Error getting inputs for microturbine generator named: {}", objectName)); // LCOV_EXCL_LINE
    // Shut up the compiler
    return nullptr; // LCOV_EXCL_LINE
}

void GetMTGeneratorInput(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         R. Raustad/D. Shirey
    //       DATE WRITTEN   Mar 2008
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //  This routine gets the input information for the Microturbine (MT) Generator model.

    bool ErrorsFound(false);

    state.dataIPShortCut->cCurrentModuleObject = "Generator:MicroTurbine";
    state.dataMircoturbElectGen->NumMTGenerators =
        state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataIPShortCut->cCurrentModuleObject);

    if (state.dataMircoturbElectGen->NumMTGenerators <= 0) {
        ShowSevereError(state, format("No {} equipment specified in input file", state.dataIPShortCut->cCurrentModuleObject));
        ErrorsFound = true;
    }

    // ALLOCATE ARRAYS
    state.dataMircoturbElectGen->MTGenerator.allocate(state.dataMircoturbElectGen->NumMTGenerators);

    // LOAD ARRAYS WITH MICROTURBINE GENERATOR DATA
    for (int GeneratorNum = 1; GeneratorNum <= state.dataMircoturbElectGen->NumMTGenerators; ++GeneratorNum) {
        int NumAlphas;
        int NumNums;
        int IOStat;
        Array1D<Real64> NumArray(19);
        Array1D_string AlphArray(20);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 state.dataIPShortCut->cCurrentModuleObject,
                                                                 GeneratorNum,
                                                                 AlphArray,
                                                                 NumAlphas,
                                                                 NumArray,
                                                                 NumNums,
                                                                 IOStat,
                                                                 state.dataIPShortCut->lNumericFieldBlanks,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);
        Util::IsNameEmpty(state, state.dataIPShortCut->cAlphaArgs(1), state.dataIPShortCut->cCurrentModuleObject, ErrorsFound);
        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name = AlphArray(1);

        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecPowerOutput = NumArray(1);
        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecPowerOutput <= 0.0) {
            ShowSevereError(state, format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(1), NumArray(1)));
            ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
            ShowContinueError(state, format("{} must be greater than 0.", state.dataIPShortCut->cNumericFieldNames(1)));
            ErrorsFound = true;
        }

        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MinElecPowerOutput = NumArray(2);
        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MaxElecPowerOutput = NumArray(3);

        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MinElecPowerOutput < 0.0) {
            ShowSevereError(state, format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(2), NumArray(2)));
            ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
            ShowContinueError(state, format("{} must be greater than 0.", state.dataIPShortCut->cNumericFieldNames(2)));
            ErrorsFound = true;
        }

        if (state.dataIPShortCut->lNumericFieldBlanks(3)) {
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MaxElecPowerOutput =
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecPowerOutput;
        } else {
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MaxElecPowerOutput <= 0.0) {
                ShowSevereError(state, format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(3), NumArray(3)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
                ShowContinueError(state, format("{} must be greater than 0.", state.dataIPShortCut->cNumericFieldNames(3)));
                ErrorsFound = true;
            }
        }

        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MinElecPowerOutput >=
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MaxElecPowerOutput) {
            ShowSevereError(
                state, format("{}= {}", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
            ShowContinueError(state,
                              format("{} [{:.2R}] > {} [{:.2R}]",
                                     state.dataIPShortCut->cNumericFieldNames(2),
                                     NumArray(2),
                                     state.dataIPShortCut->cNumericFieldNames(3),
                                     NumArray(3)));
            ShowContinueError(state, "Minimum Full Load Electrical Power Output must be less than or equal");
            ShowContinueError(state, "to Maximum Full Load Electrical Power Output.");
            ErrorsFound = true;
        }

        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecPowerOutput >
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MaxElecPowerOutput ||
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecPowerOutput <
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MinElecPowerOutput) {
            ShowSevereError(
                state, format("{}= {}", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
            ShowContinueError(state,
                              format("{} must be >= {}", state.dataIPShortCut->cNumericFieldNames(1), state.dataIPShortCut->cNumericFieldNames(2)));
            ShowContinueError(state,
                              format("{} must be <= {}", state.dataIPShortCut->cNumericFieldNames(1), state.dataIPShortCut->cNumericFieldNames(3)));
            ShowContinueError(state, format("{} = {:.2R}", state.dataIPShortCut->cNumericFieldNames(1), NumArray(1)));
            ShowContinueError(state, format("{} = {:.2R}", state.dataIPShortCut->cNumericFieldNames(2), NumArray(2)));
            ShowContinueError(state, format("{} = {:.2R}", state.dataIPShortCut->cNumericFieldNames(3), NumArray(3)));
            ErrorsFound = true;
        }

        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecEfficiencyLHV = NumArray(4);

        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecEfficiencyLHV <= 0.0) {
            ShowSevereError(state, format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(4), NumArray(4)));
            ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
            ShowContinueError(state, format("{} must be greater than 0.", state.dataIPShortCut->cNumericFieldNames(4)));
            ErrorsFound = true;
        }

        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletTemp = NumArray(5);
        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletHumRat = NumArray(6);
        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElevation = NumArray(7);

        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletHumRat <= 0.0) {
            ShowSevereError(state, format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(6), NumArray(6)));
            ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
            ShowContinueError(state, format("{} must be greater than 0.", state.dataIPShortCut->cNumericFieldNames(6)));
            ErrorsFound = true;
        } else {
            // Reference barometric pressure, adjusted for reference elevation (Pa)
            Real64 RefBaroPressure =
                101325.0 * std::pow(1.0 - 2.25577e-05 * state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElevation, 5.2559);
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletDensity =
                Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                  RefBaroPressure,
                                                  state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletTemp,
                                                  state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletHumRat);
        }

        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ElecPowFTempElevCurveNum =
            Curve::GetCurveIndex(state, AlphArray(2)); // Convert curve name to number
        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ElecPowFTempElevCurveNum == 0) {
            ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(2), AlphArray(2)));
            ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
            ErrorsFound = true;
        } else {
            // Verify curve object, only legal type is BiQuadratic

            if (!ErrorsFound) {
                // Check electrical power output at reference combustion inlet temp and elevation
                // Output of Electrical Power Output Modifier Curve (function of temp and elev)
                Real64 ElectOutFTempElevOutput = Curve::CurveValue(state,
                                                                   state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ElecPowFTempElevCurveNum,
                                                                   state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletTemp,
                                                                   state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElevation);
                if (std::abs(ElectOutFTempElevOutput - 1.0) > 0.1) {
                    ShowWarningError(
                        state,
                        format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                    ShowContinueError(state, format("{} = {}", state.dataIPShortCut->cAlphaFieldNames(2), AlphArray(2)));
                    ShowContinueError(state, "... Curve output at reference conditions should equal 1 (+-10%).");
                    ShowContinueError(state,
                                      format("...Reference combustion air inlet temperature = {:.4T} C",
                                             state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletTemp));
                    ShowContinueError(state,
                                      format("...Reference elevation                        = {:.4T} m",
                                             state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElevation));
                    ShowContinueError(state, format("...Curve output                               = {:.4T}", ElectOutFTempElevOutput));
                }
            }
        }

        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ElecEffFTempCurveNum =
            Curve::GetCurveIndex(state, AlphArray(3)); // Convert curve name to number
        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ElecEffFTempCurveNum == 0) {
            ShowSevereError(
                state, format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
            ShowContinueError(state, format("{} not found = {}", state.dataIPShortCut->cAlphaFieldNames(3), AlphArray(3)));
            ErrorsFound = true;
        } else {
            // Verify curve object, only legal types are Quadratic and Cubic

            if (!ErrorsFound) {
                // Check electrical efficiency at reference combustion inlet temp
                // Output of Electrical Efficiency Modifier Curve (function of temp)
                Real64 ElecEfficFTempOutput = Curve::CurveValue(state,
                                                                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ElecEffFTempCurveNum,
                                                                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletTemp);
                if (std::abs(ElecEfficFTempOutput - 1.0) > 0.1) {
                    ShowWarningError(
                        state,
                        format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                    ShowContinueError(state, format("{} = {}", state.dataIPShortCut->cAlphaFieldNames(3), AlphArray(3)));
                    ShowContinueError(state, "... Curve output at reference condition should equal 1 (+-10%).");
                    ShowContinueError(state,
                                      format("... Reference combustion air inlet temperature = {:.4T} C",
                                             state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletTemp));
                    ShowContinueError(state, format("... Curve output                               = {:.4T}", ElecEfficFTempOutput));
                }
            }
        }

        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ElecEffFPLRCurveNum =
            Curve::GetCurveIndex(state, AlphArray(4)); // Convert curve name to number
        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ElecEffFPLRCurveNum == 0) {
            ShowSevereError(
                state, format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
            ShowContinueError(state, format("{} not found = {}", state.dataIPShortCut->cAlphaFieldNames(4), AlphArray(4)));
            ErrorsFound = true;
        } else {
            // Verify curve object, only legal types are Quadratic and Cubic

            if (!ErrorsFound) {
                // Check electrical efficiency at PLR = 1
                // Output of Electrical Efficiency Modifier Curve (function of PLR)
                Real64 ElecEfficFPLROutput =
                    Curve::CurveValue(state, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ElecEffFPLRCurveNum, 1.0);
                if (std::abs(ElecEfficFPLROutput - 1.0) > 0.1) {
                    ShowWarningError(
                        state,
                        format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                    ShowContinueError(state, format("{} = {}", state.dataIPShortCut->cAlphaFieldNames(4), AlphArray(4)));
                    ShowContinueError(state, "... Curve output at a part-load ratio of 1 should equal 1 (+-10%).");
                    ShowContinueError(state, format("... Curve output = {:.4T}", ElecEfficFPLROutput));
                }

                Real64 Var1Min(0.0);
                Real64 Var1Max(0.0);
                Curve::GetCurveMinMaxValues(state, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ElecEffFPLRCurveNum, Var1Min, Var1Max);
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MinPartLoadRat = Var1Min;
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MaxPartLoadRat = Var1Max;
            }
        }

        // Validate fuel type input
        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).FuelType =
            static_cast<Constant::eFuel>(getEnumValue(Constant::eFuelNamesUC, AlphArray(5)));
        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).FuelType == Constant::eFuel::Invalid) {
            ShowSevereError(
                state, format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
            ShowContinueError(state, format("Invalid {}  = {}", state.dataIPShortCut->cAlphaFieldNames(5), AlphArray(5)));
            ErrorsFound = true;
        }

        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).FuelHigherHeatingValue = NumArray(8);
        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).FuelLowerHeatingValue = NumArray(9);

        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).FuelLowerHeatingValue <= 0.0) {
            ShowSevereError(state, format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(9), NumArray(9)));
            ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
            ShowContinueError(state, format("{} must be greater than 0.", state.dataIPShortCut->cNumericFieldNames(9)));
            ErrorsFound = true;
        }

        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).FuelHigherHeatingValue <= 0.0) {
            ShowSevereError(state, format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(8), NumArray(8)));
            ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
            ShowContinueError(state, format("{} must be greater than 0.", state.dataIPShortCut->cNumericFieldNames(8)));
            ErrorsFound = true;
        }

        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).FuelLowerHeatingValue >
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).FuelHigherHeatingValue) {
            ShowSevereError(
                state, format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
            ShowContinueError(
                state,
                format("{} must be greater than the {}", state.dataIPShortCut->cNumericFieldNames(8), state.dataIPShortCut->cNumericFieldNames(9)));
            ShowContinueError(state, format("{}={:.2R}", state.dataIPShortCut->cNumericFieldNames(8), NumArray(8)));
            ShowContinueError(state, format("{}={:.2R}", state.dataIPShortCut->cNumericFieldNames(9), NumArray(9)));
            ErrorsFound = true;
        }

        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).StandbyPower = NumArray(10);
        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).StandbyPower < 0.0) {
            ShowWarningError(state, format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(10), NumArray(10)));
            ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
            ShowContinueError(state, format("{} must be greater than 0.", state.dataIPShortCut->cNumericFieldNames(10)));
            ShowContinueError(state, "Resetting to 0 and the simulation continues.");
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).StandbyPower = 0.0;
        }

        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).AncillaryPower = NumArray(11);
        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).AncillaryPower < 0.0) {
            ShowWarningError(state, format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(11), NumArray(11)));
            ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
            ShowContinueError(state, format("{} must be greater than 0.", state.dataIPShortCut->cNumericFieldNames(11)));
            ShowContinueError(state, "Resetting to 0 and the simulation continues.");
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).AncillaryPower = 0.0;
        }

        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).AncillaryPowerFuelCurveNum =
            Curve::GetCurveIndex(state, AlphArray(6)); // Convert curve name to number
        //   If blank, then the calc routine assumes modifier curve value = 1 for entire simulation
        if (!state.dataIPShortCut->lAlphaFieldBlanks(6) && state.dataMircoturbElectGen->MTGenerator(GeneratorNum).AncillaryPowerFuelCurveNum == 0) {
            ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(6), AlphArray(6)));
            ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
            ErrorsFound = true;
        } else if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).AncillaryPowerFuelCurveNum > 0) {
            // Verify curve object, only legal type is Quadratic

            if (!ErrorsFound) {
                // Fuel mass flow rate at reference conditions (kg/s)
                Real64 RefFuelUseMdot = (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecPowerOutput /
                                         state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecEfficiencyLHV) /
                                        (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).FuelLowerHeatingValue * 1000.0);
                // Output of Ancillary Power Modifier Curve (function of temps and fuel flow)
                Real64 AncillaryPowerOutput =
                    Curve::CurveValue(state, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).AncillaryPowerFuelCurveNum, RefFuelUseMdot);
                if (std::abs(AncillaryPowerOutput - 1.0) > 0.1) {
                    ShowWarningError(
                        state,
                        format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                    ShowContinueError(state, format("{} = {}", state.dataIPShortCut->cAlphaFieldNames(6), AlphArray(6)));
                    ShowContinueError(state, "... Curve output at reference conditions should equal 1 (+-10%).");
                    ShowContinueError(state,
                                      format("... Reference Electrical Power Output           = {:.2T} W",
                                             state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecPowerOutput));
                    ShowContinueError(state,
                                      format("... Reference Electrical Efficiency (LHV basis) = {:.4T}",
                                             state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecEfficiencyLHV));
                    ShowContinueError(state,
                                      format("... Fuel Lower Heating Value                    = {:.2T} kJ/kg",
                                             state.dataMircoturbElectGen->MTGenerator(GeneratorNum).FuelLowerHeatingValue));
                    ShowContinueError(state, format("... Calculated fuel flow                        = {:.4T} kg/s", RefFuelUseMdot));
                    ShowContinueError(state, format("... Curve output                                = {:.4T}", AncillaryPowerOutput));
                }
            }
        }

        if (!state.dataIPShortCut->lAlphaFieldBlanks(7)) {
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecInletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    AlphArray(7),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::GeneratorMicroTurbine,
                                                    state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name,
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);
        }

        if (!state.dataIPShortCut->lAlphaFieldBlanks(8)) {
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecOutletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    AlphArray(8),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::GeneratorMicroTurbine,
                                                    state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name,
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);
        }

        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecInletNodeNum > 0 &&
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecOutletNodeNum > 0) {
            BranchNodeConnections::TestCompSet(state,
                                               state.dataIPShortCut->cCurrentModuleObject,
                                               state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name,
                                               AlphArray(7),
                                               AlphArray(8),
                                               "Heat Recovery Nodes");
        }

        if ((state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecOutletNodeNum > 0 &&
             state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecInletNodeNum == 0) ||
            (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecOutletNodeNum == 0 &&
             state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecInletNodeNum > 0)) {
            ShowSevereError(
                state, format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
            ShowContinueError(state, "... If one Heat Recovery Water Node Name is specified, then both the Inlet and Outlet Heat Recovery");
            ShowContinueError(state, "... Water Node Names must be specified. Only one water node is being specified for this generator.");
            ErrorsFound = true;
        }

        //   Heat recovery to water input fields only valid if water nodes are defined
        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecInletNodeNum != 0 &&
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecOutletNodeNum != 0) {

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecActive = true;

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefThermalEffLHV = NumArray(12);
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefThermalEffLHV < 0.0) {
                ShowWarningError(
                    state,
                    format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                ShowContinueError(state, format("{} must be >= 0.", state.dataIPShortCut->cNumericFieldNames(12)));
                ShowContinueError(state, "Resetting to 0 and the simulation continues.");
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefThermalEffLHV = 0.0;
            }

            // Next store thermal power output ranges using nominal thermal to electrical efficiency ratio and electrical power data
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefThermalPowerOutput =
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecPowerOutput *
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefThermalEffLHV /
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecEfficiencyLHV;
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MinThermalPowerOutput =
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MinElecPowerOutput *
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefThermalEffLHV /
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecEfficiencyLHV;
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MaxThermalPowerOutput =
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).MaxElecPowerOutput *
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefThermalEffLHV /
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElecEfficiencyLHV;

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefInletWaterTemp = NumArray(13);

            if (Util::SameString(AlphArray(9), "InternalControl")) {
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).InternalFlowControl =
                    true; //  A9, \field Heat Recovery Water Flow Operating Mode
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).PlantFlowControl = false;
            }
            if ((!(Util::SameString(AlphArray(9), "InternalControl"))) && (!(Util::SameString(AlphArray(9), "PlantControl")))) {
                ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(9), AlphArray(9)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
                ShowContinueError(state, "Operating Mode must be INTERNAL CONTROL or PLANT CONTROL.");
                ErrorsFound = true;
            }

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefHeatRecVolFlowRate = NumArray(14);

            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefHeatRecVolFlowRate <= 0.0) {
                ShowSevereError(state, format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(14), NumArray(14)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
                ShowContinueError(state, format("{} must be greater than 0.", state.dataIPShortCut->cNumericFieldNames(14)));
                ErrorsFound = true;
            }

            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).InternalFlowControl) { // Get Heat Recovery Water Flow Rate Modifier Curve

                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecFlowFTempPowCurveNum = Curve::GetCurveIndex(state, AlphArray(10));
                if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecFlowFTempPowCurveNum != 0) {
                    // Verify curve object, only legal type is BiQuadratic
                }

            } // End of IF (MTGenerator(GeneratorNum)%InternalFlowControl) THEN

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ThermEffFTempElevCurveNum =
                Curve::GetCurveIndex(state, AlphArray(11)); // convert curve name to number
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ThermEffFTempElevCurveNum != 0) {
                // Verify curve object, only legal types are BiQuadratic and BiCubic

                if (!ErrorsFound) {
                    // Output of Thermal Efficiency Modifier Curve (function of temp and elevation)
                    Real64 ThermalEffTempElevOutput =
                        Curve::CurveValue(state,
                                          state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ThermEffFTempElevCurveNum,
                                          state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletTemp,
                                          state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElevation);

                    if (std::abs(ThermalEffTempElevOutput - 1.0) > 0.1) {
                        ShowWarningError(state,
                                         format("{} \"{}\"",
                                                state.dataIPShortCut->cCurrentModuleObject,
                                                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                        ShowContinueError(state, format("{} = {}", state.dataIPShortCut->cAlphaFieldNames(11), AlphArray(11)));
                        ShowContinueError(state, "... Curve output at reference conditions should equal 1 (+-10%).");
                        ShowContinueError(state,
                                          format("... Reference combustion air inlet temperature      = {:.4T} C",
                                                 state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletTemp));
                        ShowContinueError(state,
                                          format("... Reference elevation                             = {:.4T} m",
                                                 state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefElevation));
                    }
                }
            }

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecRateFPLRCurveNum =
                Curve::GetCurveIndex(state, AlphArray(12)); // convert curve name to number
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecRateFPLRCurveNum != 0) {
                // Verify curve object, only legal types are Quadratic or Cubic

                if (!ErrorsFound) {
                    // Output of Heat Recovery Rate Modifier Curve (function of PLR)
                    Real64 HeatRecRateFPLROutput =
                        Curve::CurveValue(state, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecRateFPLRCurveNum, 1.0);

                    if (std::abs(HeatRecRateFPLROutput - 1.0) > 0.1) {
                        ShowWarningError(state,
                                         format("{} \"{}\"",
                                                state.dataIPShortCut->cCurrentModuleObject,
                                                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                        ShowContinueError(state, format("{} = {}", state.dataIPShortCut->cAlphaFieldNames(12), AlphArray(12)));
                        ShowContinueError(state, "... Curve output at a part-load ratio of 1 should equal 1 (+-10%).");
                        ShowContinueError(state, format("... Curve output = {:.4T}", HeatRecRateFPLROutput));
                    }
                }
            }

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecRateFTempCurveNum =
                Curve::GetCurveIndex(state, AlphArray(13)); // convert curve name to number
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecRateFTempCurveNum != 0) {
                // Verify curve object, only legal type is Quadratic

                if (!ErrorsFound) {
                    // Output of Heat Recovery Rate Modifier Curve (function of inlet water temp)
                    Real64 HeatRecRateFTempOutput = Curve::CurveValue(state,
                                                                      state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecRateFTempCurveNum,
                                                                      state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefInletWaterTemp);

                    if (std::abs(HeatRecRateFTempOutput - 1.0) > 0.1) {
                        ShowWarningError(state,
                                         format("{} \"{}\"",
                                                state.dataIPShortCut->cCurrentModuleObject,
                                                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                        ShowContinueError(state, format("{} = {}", state.dataIPShortCut->cAlphaFieldNames(13), AlphArray(13)));
                        ShowContinueError(state, "... Curve output at reference condition should equal 1 (+-10%).");
                        ShowContinueError(state,
                                          format("... Reference inlet water temperature temperature      = {:.4T} C",
                                                 state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefInletWaterTemp));
                        ShowContinueError(state, format("... Curve output = {:.4T}", HeatRecRateFTempOutput));
                    }
                }
            }

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecRateFWaterFlowCurveNum = Curve::GetCurveIndex(state, AlphArray(14));
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecRateFWaterFlowCurveNum != 0) {
                // Verify curve object, only legal type is Quadratic

                if (!ErrorsFound) {
                    // Output of Heat Recovery Rate Modifier Curve (function of water flow rate)
                    Real64 HeatRecRateFFlowOutput =
                        Curve::CurveValue(state,
                                          state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecRateFWaterFlowCurveNum,
                                          state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefHeatRecVolFlowRate);

                    if (std::abs(HeatRecRateFFlowOutput - 1.0) > 0.1) {
                        ShowWarningError(state,
                                         format("{} \"{}\"",
                                                state.dataIPShortCut->cCurrentModuleObject,
                                                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                        ShowContinueError(state, format("{} = {}", state.dataIPShortCut->cAlphaFieldNames(14), AlphArray(14)));
                        ShowContinueError(state, "... Curve output at reference condition should equal 1 (+-10%).");
                        ShowContinueError(state,
                                          format("... Reference Heat Recovery Water Flow Rate      = {:.4T} m3/s",
                                                 state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefHeatRecVolFlowRate));
                        ShowContinueError(state, format("... Curve output = {:.4T}", HeatRecRateFFlowOutput));
                    }
                }
            }

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMinVolFlowRate = NumArray(15);
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMinVolFlowRate < 0.0) {
                ShowWarningError(
                    state,
                    format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                ShowContinueError(state, format("{} must be >= 0.", state.dataIPShortCut->cNumericFieldNames(15)));
                ShowContinueError(state, "Resetting to 0 and the simulation continues.");
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMinVolFlowRate = 0.0;
            }

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMaxVolFlowRate = NumArray(16);
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMaxVolFlowRate < 0.0) {
                ShowWarningError(
                    state,
                    format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                ShowContinueError(state, format("{} must be >= 0.", state.dataIPShortCut->cNumericFieldNames(16)));
                ShowContinueError(state, "Resetting to 0 and the simulation continues.");
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMaxVolFlowRate = 0.0;
            }

            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMaxVolFlowRate <
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMinVolFlowRate) {
                ShowWarningError(
                    state,
                    format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                ShowContinueError(
                    state, format("{} must be >= {}", state.dataIPShortCut->cNumericFieldNames(16), state.dataIPShortCut->cNumericFieldNames(15)));
                ShowContinueError(state,
                                  format("Resetting {} = {} and the simulation continues.",
                                         state.dataIPShortCut->cNumericFieldNames(16),
                                         state.dataIPShortCut->cNumericFieldNames(15)));
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMaxVolFlowRate =
                    state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMinVolFlowRate;
            }

            //     Check if reference heat recovery water flow rate is below the minimum flow rate
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefHeatRecVolFlowRate <
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMinVolFlowRate) {
                ShowWarningError(
                    state,
                    format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                ShowContinueError(
                    state, format("{} must be >= {}", state.dataIPShortCut->cNumericFieldNames(14), state.dataIPShortCut->cNumericFieldNames(15)));
                ShowContinueError(state,
                                  format("Resetting {} = {} and the simulation continues.",
                                         state.dataIPShortCut->cNumericFieldNames(14),
                                         state.dataIPShortCut->cNumericFieldNames(15)));
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefHeatRecVolFlowRate =
                    state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMinVolFlowRate;
            }

            //     Check if reference heat recovery water flow rate is above the maximum flow rate
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefHeatRecVolFlowRate >
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMaxVolFlowRate) {
                ShowWarningError(
                    state,
                    format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                ShowContinueError(
                    state, format("{} must be <= {}", state.dataIPShortCut->cNumericFieldNames(14), state.dataIPShortCut->cNumericFieldNames(16)));
                ShowContinueError(state,
                                  format("Resetting {} = {} and the simulation continues.",
                                         state.dataIPShortCut->cNumericFieldNames(14),
                                         state.dataIPShortCut->cNumericFieldNames(16)));
                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefHeatRecVolFlowRate =
                    state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMaxVolFlowRate;
            }

            PlantUtilities::RegisterPlantCompDesignFlow(state,
                                                        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecInletNodeNum,
                                                        state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMaxVolFlowRate);

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).HeatRecMaxWaterTemp = NumArray(17);

        } // End of 'IF (MTGenerator(GeneratorNum)%HeatRecInletNodeNum .NE. 0 .AND. &
        //             MTGenerator(GeneratorNum)%HeatRecOutletNodeNum .NE. 0) THEN'

        if (!state.dataIPShortCut->lAlphaFieldBlanks(15)) {
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).CombustionAirInletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    AlphArray(15),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::GeneratorMicroTurbine,
                                                    AlphArray(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Secondary,
                                                    DataLoopNode::ObjectIsNotParent);
        }

        //    Combustion air inlet node must be an outside air node
        if (!state.dataIPShortCut->lAlphaFieldBlanks(15) &&
            !OutAirNodeManager::CheckOutAirNodeNumber(state, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).CombustionAirInletNodeNum)) {
            ShowSevereError(
                state, format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
            ShowContinueError(state, format("{} is not a valid Outdoor Air Node = {}", state.dataIPShortCut->cAlphaFieldNames(15), AlphArray(15)));
            ShowContinueError(state, "it does not appear in an OutdoorAir:NodeList or as an OutdoorAir:Node.");
            ErrorsFound = true;
        }

        if (!state.dataIPShortCut->lAlphaFieldBlanks(16)) {
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).CombustionAirOutletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    AlphArray(16),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::GeneratorMicroTurbine,
                                                    AlphArray(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Secondary,
                                                    DataLoopNode::ObjectIsNotParent);
        }

        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).CombustionAirOutletNodeNum > 0 &&
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).CombustionAirInletNodeNum == 0) {
            ShowSevereError(
                state, format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
            ShowContinueError(state,
                              format("A {} must be specified when a {} is specified.",
                                     state.dataIPShortCut->cAlphaFieldNames(15),
                                     state.dataIPShortCut->cAlphaFieldNames(16)));
            ErrorsFound = true;
        }

        //   Get other exhaust air inputs only if combustion air inlet and outlet nodes are valid
        if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).CombustionAirOutletNodeNum > 0 &&
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).CombustionAirInletNodeNum > 0) {

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ExhAirCalcsActive = true;
            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefExhaustAirMassFlowRate = NumArray(18);
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefExhaustAirMassFlowRate <= 0.0 &&
                !state.dataIPShortCut->lNumericFieldBlanks(18)) {
                ShowSevereError(state, format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(18), NumArray(18)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
                ShowContinueError(state, format("{} must be greater than 0.", state.dataIPShortCut->cNumericFieldNames(18)));
                ErrorsFound = true;
            }

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ExhFlowFTempCurveNum = Curve::GetCurveIndex(state, AlphArray(17));
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ExhFlowFTempCurveNum != 0) {
                // Verify curve object, only legal types are Quadratic and Cubic

                if (!ErrorsFound) {
                    // Output of Exhaust Air Flow Modifier Curve (function of inlet air temp)
                    Real64 ExhFlowFTempOutput = Curve::CurveValue(state,
                                                                  state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ExhFlowFTempCurveNum,
                                                                  state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletTemp);

                    if (std::abs(ExhFlowFTempOutput - 1.0) > 0.1) {
                        ShowWarningError(state,
                                         format("{} \"{}\"",
                                                state.dataIPShortCut->cCurrentModuleObject,
                                                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                        ShowContinueError(state, format("{} = {}", state.dataIPShortCut->cAlphaFieldNames(17), AlphArray(17)));
                        ShowContinueError(state, "... Curve output at reference condition should equal 1 (+-10%).");
                        ShowContinueError(state,
                                          format("... Reference combustion air inlet temperature      = {:.4T} C",
                                                 state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletTemp));
                        ShowContinueError(state, format("... Curve output = {:.4T}", ExhFlowFTempOutput));
                    }
                }
            }

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ExhFlowFPLRCurveNum =
                Curve::GetCurveIndex(state, AlphArray(18)); // convert curve name to number
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ExhFlowFPLRCurveNum != 0) {
                // Verify curve object, legal types are Quadratic or Cubic

                if (!ErrorsFound) {
                    // Output of Exhaust Air Flow Modifier Curve (function of PLR)
                    Real64 ExhFlowFPLROutput =
                        Curve::CurveValue(state, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ExhFlowFPLRCurveNum, 1.0);

                    if (std::abs(ExhFlowFPLROutput - 1.0) > 0.1) {
                        ShowWarningError(state,
                                         format("{} \"{}\"",
                                                state.dataIPShortCut->cCurrentModuleObject,
                                                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                        ShowContinueError(state, format("{} = {}", state.dataIPShortCut->cAlphaFieldNames(18), AlphArray(18)));
                        ShowContinueError(state, "... Curve output at a part-load ratio of 1 should equal 1 (+-10%).");
                        ShowContinueError(state, format("... Curve output = {:.4T}", ExhFlowFPLROutput));
                    }
                }
            }

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).NomExhAirOutletTemp = NumArray(19);

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ExhAirTempFTempCurveNum = Curve::GetCurveIndex(state, AlphArray(19));
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ExhAirTempFTempCurveNum != 0) {
                // Verify curve object, only legal types are Quadratic and Cubic

                if (!ErrorsFound) {
                    // Output of Exhaust Air Temperature Modifier Curve (function of inlet air temp)
                    Real64 ExhAirTempFTempOutput = Curve::CurveValue(state,
                                                                     state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ExhAirTempFTempCurveNum,
                                                                     state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletTemp);

                    if (std::abs(ExhAirTempFTempOutput - 1.0) > 0.1) {
                        ShowWarningError(state,
                                         format("{} \"{}\"",
                                                state.dataIPShortCut->cCurrentModuleObject,
                                                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                        ShowContinueError(state, format("{} = {}", state.dataIPShortCut->cAlphaFieldNames(19), AlphArray(19)));
                        ShowContinueError(state, "... Curve output at reference condition should equal 1 (+-10%).");
                        ShowContinueError(state,
                                          format("... Reference combustion air inlet temperature      = {:.4T} C",
                                                 state.dataMircoturbElectGen->MTGenerator(GeneratorNum).RefCombustAirInletTemp));
                        ShowContinueError(state, format("... Curve output = {:.4T}", ExhAirTempFTempOutput));
                    }
                }
            }

            state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ExhAirTempFPLRCurveNum =
                Curve::GetCurveIndex(state, AlphArray(20)); // convert curve name to number
            if (state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ExhAirTempFPLRCurveNum != 0) {
                // Verify curve object, legal types are Quadratic or Cubic

                if (!ErrorsFound) {
                    // Output of Exhaust Air Temperature Modifier Curve (function of PLR)
                    Real64 ExhOutAirTempFPLROutput =
                        Curve::CurveValue(state, state.dataMircoturbElectGen->MTGenerator(GeneratorNum).ExhAirTempFPLRCurveNum, 1.0);

                    if (std::abs(ExhOutAirTempFPLROutput - 1.0) > 0.1) {
                        ShowWarningError(state,
                                         format("{} \"{}\"",
                                                state.dataIPShortCut->cCurrentModuleObject,
                                                state.dataMircoturbElectGen->MTGenerator(GeneratorNum).Name));
                        ShowContinueError(state, format("{} = {}", state.dataIPShortCut->cAlphaFieldNames(20), AlphArray(20)));
                        ShowContinueError(state, "... Curve output at a part-load ratio of 1 should equal 1 (+-10%).");
                        ShowContinueError(state, format("... Curve output = {:.4T}", ExhOutAirTempFPLROutput));
                    }
                }
            }

        } // End of '    IF (MTGenerator(GeneratorNum)%CombustionAirOutletNodeNum .GT. 0 .AND. &
          //                 MTGenerator(GeneratorNum)%CombustionAirInletNodeNum .GT. 0) THEN
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("Errors found in processing input for {}", state.dataIPShortCut->cCurrentModuleObject));
    }
}

void MTGeneratorSpecs::setupOutputVars(EnergyPlusData &state)
{
    std::string_view const sFuelType = Constant::eFuelNames[static_cast<int>(this->FuelType)];
    SetupOutputVariable(state,
                        "Generator Produced AC Electricity Rate",
                        Constant::Units::W,
                        this->ElecPowerGenerated,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Produced AC Electricity Energy",
                        Constant::Units::J,
                        this->EnergyGen,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::ElectricityProduced,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Cogeneration);

    SetupOutputVariable(state,
                        "Generator LHV Basis Electric Efficiency",
                        Constant::Units::None,
                        this->ElectricEfficiencyLHV,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    //    Fuel specific report variables
    SetupOutputVariable(state,
                        format("Generator {} HHV Basis Rate", sFuelType),
                        Constant::Units::W,
                        this->FuelEnergyUseRateHHV,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        format("Generator {} HHV Basis Energy", sFuelType),
                        Constant::Units::J,
                        this->FuelEnergyHHV,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eFuel2eResource[(int)this->FuelType],
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Cogeneration);

    SetupOutputVariable(state,
                        format("Generator {} Mass Flow Rate", sFuelType),
                        Constant::Units::kg_s,
                        this->FuelMdot,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    //    general fuel use report (to match other generators)
    SetupOutputVariable(state,
                        "Generator Fuel HHV Basis Rate",
                        Constant::Units::W,
                        this->FuelEnergyUseRateHHV,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Fuel HHV Basis Energy",
                        Constant::Units::J,
                        this->FuelEnergyHHV,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);

    //    Heat recovery (to water) report variables
    if (this->HeatRecActive) {

        SetupOutputVariable(state,
                            "Generator Produced Thermal Rate",
                            Constant::Units::W,
                            this->QHeatRecovered,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Generator Produced Thermal Energy",
                            Constant::Units::J,
                            this->ExhaustEnergyRec,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name,
                            Constant::eResource::EnergyTransfer,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::HeatRecovery);

        SetupOutputVariable(state,
                            "Generator Thermal Efficiency LHV Basis",
                            Constant::Units::None,
                            this->ThermalEfficiencyLHV,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Generator Heat Recovery Inlet Temperature",
                            Constant::Units::C,
                            this->HeatRecInletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Generator Heat Recovery Outlet Temperature",
                            Constant::Units::C,
                            this->HeatRecOutletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Generator Heat Recovery Water Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->HeatRecMdot,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
    }

    if (this->StandbyPower > 0.0) { // Report Standby Power if entered by user
        SetupOutputVariable(state,
                            "Generator Standby Electricity Rate",
                            Constant::Units::W,
                            this->StandbyPowerRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Generator Standby Electricity Energy",
                            Constant::Units::J,
                            this->StandbyEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::Cogeneration);
    }

    if (this->AncillaryPower > 0.0) { // Report Ancillary Power if entered by user
        SetupOutputVariable(state,
                            "Generator Ancillary Electricity Rate",
                            Constant::Units::W,
                            this->AncillaryPowerRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Generator Ancillary Electricity Energy",
                            Constant::Units::J,
                            this->AncillaryEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);
    }

    //   Report combustion air outlet conditions if exhaust air calculations are active
    if (this->ExhAirCalcsActive) {
        SetupOutputVariable(state,
                            "Generator Exhaust Air Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->ExhaustAirMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Generator Exhaust Air Temperature",
                            Constant::Units::C,
                            this->ExhaustAirTemperature,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
    }
}

void MTGeneratorSpecs::simulate([[maybe_unused]] EnergyPlusData &state,
                                [[maybe_unused]] const PlantLocation &calledFromLocation,
                                [[maybe_unused]] bool FirstHVACIteration,
                                [[maybe_unused]] Real64 &CurLoad,
                                [[maybe_unused]] bool RunFlag)
{
    // empty function to emulate current behavior as of conversion to using the PlantComponent calling structure.
    // calls from the plant side... do nothing.
    // calls from the ElectricPowerServiceManger call the init, calc, and update worker functions
}

void MTGeneratorSpecs::getDesignCapacities([[maybe_unused]] EnergyPlusData &state,
                                           [[maybe_unused]] const PlantLocation &calledFromLocation,
                                           Real64 &MaxLoad,
                                           Real64 &MinLoad,
                                           Real64 &OptLoad)
{
    MaxLoad = 0.0;
    MinLoad = 0.0;
    OptLoad = 0.0;
}

void MTGeneratorSpecs::InitMTGenerators(EnergyPlusData &state,
                                        bool const RunFlag,
                                        Real64 const MyLoad, // electrical load in W
                                        bool const FirstHVACIteration)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         R. Raustad/D. Shirey
    //       DATE WRITTEN   Mar 2008
    //       MODIFIED       na
    //       RE-ENGINEERED  B. Griffith, Sept 2010, plant upgrades, general fluid props

    // PURPOSE OF THIS SUBROUTINE:
    //  This subroutine is for initializations of the CT generators.

    // METHODOLOGY EMPLOYED:
    //  Uses the status flags to trigger initializations.

    this->oneTimeInit(state); // end one time inits

    if (!this->HeatRecActive) {
        return;
    }

    // Do the Begin Environment initializations
    if (state.dataGlobal->BeginEnvrnFlag && this->MyEnvrnFlag) {
        // set the node max and min mass flow rates
        PlantUtilities::InitComponentNodes(state, 0.0, this->HeatRecMaxMassFlowRate, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum);

        state.dataLoopNodes->Node(this->HeatRecInletNodeNum).Temp = 20.0; // Set the node temperature, assuming freeze control
        state.dataLoopNodes->Node(this->HeatRecOutletNodeNum).Temp = 20.0;

        this->MyEnvrnFlag = false;
    } // end environmental inits

    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->MyEnvrnFlag = true;
    }

    // set/request flow rates
    if (FirstHVACIteration) {

        Real64 DesiredMassFlowRate;
        if (!RunFlag) {
            DesiredMassFlowRate = 0.0;

        } else if (RunFlag && this->InternalFlowControl) {
            // assume dispatch power in MyLoad is what gets produced (future, reset during calc routine and iterate)
            if (this->HeatRecFlowFTempPowCurveNum != 0) {
                DesiredMassFlowRate =
                    this->DesignHeatRecMassFlowRate *
                    Curve::CurveValue(state, this->HeatRecFlowFTempPowCurveNum, state.dataLoopNodes->Node(this->HeatRecInletNodeNum).Temp, MyLoad);
            } else {
                DesiredMassFlowRate = this->DesignHeatRecMassFlowRate; // Assume modifier = 1 if curve not specified
            }

            DesiredMassFlowRate = max(DataPrecisionGlobals::constant_zero, DesiredMassFlowRate); // protect from neg. curve result

        } else if (RunFlag && (!this->InternalFlowControl)) {
            DesiredMassFlowRate = this->DesignHeatRecMassFlowRate;
        }

        PlantUtilities::SetComponentFlowRate(state, DesiredMassFlowRate, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum, this->HRPlantLoc);
    } else { // not FirstHVACIteration
        if (!RunFlag) {
            state.dataLoopNodes->Node(this->HeatRecInletNodeNum).MassFlowRate =
                min(DataPrecisionGlobals::constant_zero, state.dataLoopNodes->Node(this->HeatRecInletNodeNum).MassFlowRateMaxAvail);
            state.dataLoopNodes->Node(this->HeatRecInletNodeNum).MassFlowRate =
                max(DataPrecisionGlobals::constant_zero, state.dataLoopNodes->Node(this->HeatRecInletNodeNum).MassFlowRateMinAvail);

        } else if (RunFlag && this->InternalFlowControl) {
            // assume dispatch power in MyLoad is what gets produced (future, reset during calc routine and iterate)
            if (this->HeatRecFlowFTempPowCurveNum != 0) {
                Real64 DesiredMassFlowRate =
                    this->DesignHeatRecMassFlowRate *
                    Curve::CurveValue(state, this->HeatRecFlowFTempPowCurveNum, state.dataLoopNodes->Node(this->HeatRecInletNodeNum).Temp, MyLoad);
                PlantUtilities::SetComponentFlowRate(
                    state, DesiredMassFlowRate, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum, this->HRPlantLoc);
            } else {
                PlantUtilities::SetComponentFlowRate(
                    state, this->HeatRecMdot, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum, this->HRPlantLoc);
            }
        } else if (RunFlag && (!this->InternalFlowControl)) {
            PlantUtilities::SetComponentFlowRate(state, this->HeatRecMdot, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum, this->HRPlantLoc);
        }
    }
}

void MTGeneratorSpecs::CalcMTGeneratorModel(EnergyPlusData &state,
                                            bool const RunFlag,  // TRUE when generator is being asked to operate
                                            Real64 const MyLoad) // Generator demand (W)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         R. Raustad/D. Shirey
    //       DATE WRITTEN   Mar 2008
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //  Simulate a combustion generator.

    // METHODOLOGY EMPLOYED:
    //  Curve fits of performance data.

    Real64 constexpr KJtoJ(1000.0);          // Convert kilojoules to joules
    int constexpr MaxAncPowerIter(50);       // Maximum number of iteration (subroutine ancillary power iteration loop)
    Real64 constexpr AncPowerDiffToler(5.0); // Tolerance for Ancillary Power Difference (W)
    Real64 constexpr RelaxFactor(0.7);       // Relaxation factor for iteration loop
    static constexpr std::string_view RoutineName("CalcMTGeneratorModel");

    //   Load local variables from data structure (for code readability)
    // Min allowed operating fraction at full load
    Real64 minPartLoadRat = this->MinPartLoadRat;

    // Max allowed operating fraction at full load
    Real64 maxPartLoadRat = this->MaxPartLoadRat;

    // Generator reference capacity (W)
    Real64 ReferencePowerOutput = this->RefElecPowerOutput;

    // Reference electrical efficiency
    Real64 RefElecEfficiency = this->RefElecEfficiencyLHV;

    //   Initialize variables
    this->ElecPowerGenerated = 0.0;
    this->HeatRecInletTemp = 0.0;
    this->HeatRecOutletTemp = 0.0;
    this->HeatRecMdot = 0.0;
    this->QHeatRecovered = 0.0;
    this->ExhaustEnergyRec = 0.0;
    this->FuelEnergyUseRateHHV = 0.0;
    this->FuelMdot = 0.0;
    this->AncillaryPowerRate = 0.0;
    this->StandbyPowerRate = 0.0;
    this->FuelEnergyUseRateLHV = 0.0;
    this->ExhaustAirMassFlowRate = 0.0;
    this->ExhaustAirTemperature = 0.0;
    this->ExhaustAirHumRat = 0.0;

    Real64 HeatRecInTemp; // Heat recovery fluid inlet temperature (C)
    Real64 heatRecMdot;   // Heat recovery fluid mass flow rate (kg/s)
    Real64 HeatRecCp;     // Specific heat of the heat recovery fluid (J/kg-K)

    if (this->HeatRecActive) {
        HeatRecInTemp = state.dataLoopNodes->Node(this->HeatRecInletNodeNum).Temp;
        HeatRecCp = state.dataPlnt->PlantLoop(this->HRPlantLoc.loopNum).glycol->getSpecificHeat(state, HeatRecInTemp, RoutineName);
        heatRecMdot = state.dataLoopNodes->Node(this->HeatRecInletNodeNum).MassFlowRate;
    } else {
        HeatRecInTemp = 0.0;
        HeatRecCp = 0.0;
        heatRecMdot = 0.0;
    }

    Real64 CombustionAirInletTemp;  // Combustion air inlet temperature (C)
    Real64 CombustionAirInletPress; // Barometric pressure of combustion inlet air (Pa)
    Real64 CombustionAirInletW;     // Combustion air inlet humidity ratio (kg/kg)

    //   Set combustion inlet air temperature, humidity ratio and pressure local variables
    if (this->CombustionAirInletNodeNum == 0) { // no inlet air node specified, so use weather file values
        CombustionAirInletTemp = state.dataEnvrn->OutDryBulbTemp;
        CombustionAirInletW = state.dataEnvrn->OutHumRat;
        CombustionAirInletPress = state.dataEnvrn->OutBaroPress;
    } else { // use inlet node information
        CombustionAirInletTemp = state.dataLoopNodes->Node(this->CombustionAirInletNodeNum).Temp;
        CombustionAirInletW = state.dataLoopNodes->Node(this->CombustionAirInletNodeNum).HumRat;
        CombustionAirInletPress = state.dataLoopNodes->Node(this->CombustionAirInletNodeNum).Press;
        if (state.dataLoopNodes->Node(this->CombustionAirInletNodeNum).Height > 0.0) {
        }
        //     Initialize combustion outlet air conditions to inlet air conditions (all node properties)
        if (this->ExhAirCalcsActive) {
            state.dataLoopNodes->Node(this->CombustionAirOutletNodeNum) = state.dataLoopNodes->Node(this->CombustionAirInletNodeNum);
        }
    }

    //   If no loop demand or generator OFF, set some variables and then return
    //    IF (.NOT. RunFlag .OR. MyLoad .LE. 0.0d0) THEN
    if (MyLoad <= 0.0) {
        this->HeatRecInletTemp = HeatRecInTemp;
        this->HeatRecOutletTemp = HeatRecInTemp;
        if (RunFlag) {
            this->StandbyPowerRate = this->StandbyPower;
        }
        this->ExhaustAirTemperature = CombustionAirInletTemp;
        this->ExhaustAirHumRat = CombustionAirInletW;
        return;
    }

    //   Calculate power modifier curve value (function of inlet air temperature and elevation)
    // Power ratio as a function of inlet air temperature and elevation
    Real64 PowerFTempElev = Curve::CurveValue(state, this->ElecPowFTempElevCurveNum, CombustionAirInletTemp, state.dataEnvrn->Elevation);

    //   Warn user if power modifier curve output is less than 0
    if (PowerFTempElev < 0.0) {
        if (this->PowerFTempElevErrorIndex == 0) {
            //        MTGenerator(GeneratorNum)%PowerFTempElevErrorCount = MTGenerator(GeneratorNum)%PowerFTempElevErrorCount + 1
            ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
            ShowContinueError(state,
                              format("... Electrical Power Modifier curve (function of temperature and elevation) output is less than zero ({:.4T}).",
                                     PowerFTempElev));
            ShowContinueError(state, format("... Value occurs using a combustion inlet air temperature of {:.2T} C.", CombustionAirInletTemp));
            ShowContinueError(state, format("... and an elevation of {:.2T} m.", state.dataEnvrn->Elevation));
            ShowContinueErrorTimeStamp(state, "... Resetting curve output to zero and continuing simulation.");
        }
        ShowRecurringWarningErrorAtEnd(state,
                                       "GENERATOR:MICROTURBINE \"" + this->Name +
                                           "\": Electrical Power Modifier curve is less than zero warning continues...",
                                       this->PowerFTempElevErrorIndex,
                                       PowerFTempElev,
                                       PowerFTempElev);
        PowerFTempElev = 0.0;
    }

    //   Calculate available full-load power output. cannot exceed maximum full-load power output.
    // Generator full-load power output at actual inlet conditions and elevation (W)
    Real64 FullLoadPowerOutput = min((ReferencePowerOutput * PowerFTempElev), this->MaxElecPowerOutput);
    //   Also can't be below the minimum full-load power output.
    FullLoadPowerOutput = max(FullLoadPowerOutput, this->MinElecPowerOutput);

    // Ancillary power used by pump (if not specified in manufacturers data)
    Real64 ancillaryPowerRate = this->AncillaryPower;

    // Difference between ancillary power rate and ancillary power rate last (last iteration)
    Real64 AncillaryPowerRateDiff = AncPowerDiffToler + 1.0; // Initialize to force through DO WHILE Loop at least once

    Real64 PLR(0.0);                    // Generator operating part load ratio
    Real64 elecPowerGenerated(0.0);     // Generator electric power output (W)
    Real64 FuelUseEnergyRateLHV(0.0);   // Rate of fuel energy required to run microturbine, LHV basis (W)
    Real64 fuelHigherHeatingValue(0.0); // Higher heating value (LLV) of fuel kJ/kg)
    Real64 fuelLowerHeatingValue(0.0);  // Lower heating value (LLV) of fuel kJ/kg)
    Real64 AnciPowerFMdotFuel(0.0);     // Ancillary power as a function of fuel flow curve output
    int AncPowerCalcIterIndex = 0;      // Index for subroutine iteration loop if Ancillary Power (function of fuel flow) is used

    while (AncillaryPowerRateDiff > AncPowerDiffToler && AncPowerCalcIterIndex <= MaxAncPowerIter) {

        ++AncPowerCalcIterIndex; // Increment iteration loop counter

        //     Calculate operating power output (gross)
        elecPowerGenerated = min(max(0.0, MyLoad + ancillaryPowerRate), FullLoadPowerOutput);

        //     Calculate PLR, but must be between the minPLR and maxPLR
        if (FullLoadPowerOutput > 0.0) {
            PLR = min(elecPowerGenerated / FullLoadPowerOutput, maxPartLoadRat);
            PLR = max(PLR, minPartLoadRat);
        } else {
            PLR = 0.0;
        }

        //     Recalculate elecPowerGenerated based on "final" PLR
        elecPowerGenerated = FullLoadPowerOutput * PLR;

        //     Calculate electrical efficiency modifier curve output (function of temp)
        // Electrical efficiency as a function of temperature curve output
        Real64 ElecEfficiencyFTemp = Curve::CurveValue(state, this->ElecEffFTempCurveNum, CombustionAirInletTemp);

        //     Warn user if efficiency modifier curve output is less than 0
        if (ElecEfficiencyFTemp < 0.0) {
            if (this->EffFTempErrorIndex == 0) {
                //          MTGenerator(GeneratorNum)%EffFTempErrorCount = MTGenerator(GeneratorNum)%EffFTempErrorCount + 1
                ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                ShowContinueError(
                    state,
                    format("... Electrical Efficiency Modifier (function of temperature) output is less than zero ({:.4T}).", ElecEfficiencyFTemp));
                ShowContinueError(state, format("... Value occurs using a combustion inlet air temperature of {:.2T} C.", CombustionAirInletTemp));
                ShowContinueErrorTimeStamp(state, "... Resetting curve output to zero and continuing simulation.");
            }
            ShowRecurringWarningErrorAtEnd(
                state,
                "GENERATOR:MICROTURBINE \"" + this->Name +
                    "\": Electrical Efficiency Modifier (function of temperature) output is less than zero warning continues...",
                this->EffFTempErrorIndex,
                ElecEfficiencyFTemp,
                ElecEfficiencyFTemp);
            ElecEfficiencyFTemp = 0.0;
        }

        //     Calculate efficiency modifier curve output (function of PLR)
        // Electrical efficiency as a function of PLR curve output
        Real64 ElecEfficiencyFPLR = Curve::CurveValue(state, this->ElecEffFPLRCurveNum, PLR);

        //     Warn user if efficiency modifier curve output is less than 0
        if (ElecEfficiencyFPLR < 0.0) {
            if (this->EffFPLRErrorIndex == 0) {
                ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                ShowContinueError(state,
                                  format("... Electrical Efficiency Modifier (function of part-load ratio) output is less than zero ({:.4T}).",
                                         ElecEfficiencyFPLR));
                ShowContinueError(state, format("... Value occurs using a part-load ratio of {:.3T}.", PLR));
                ShowContinueErrorTimeStamp(state, "... Resetting curve output to zero and continuing simulation.");
            }
            ShowRecurringWarningErrorAtEnd(
                state,
                "GENERATOR:MICROTURBINE \"" + this->Name +
                    "\": Electrical Efficiency Modifier (function of part-load ratio) output is less than zero warning continues...",
                this->EffFPLRErrorIndex,
                ElecEfficiencyFPLR,
                ElecEfficiencyFPLR);
            ElecEfficiencyFPLR = 0.0;
        }

        //     Calculate operating electrical efficiency
        // Actual operating efficiency
        Real64 OperatingElecEfficiency = RefElecEfficiency * ElecEfficiencyFTemp * ElecEfficiencyFPLR;

        //     Calculate fuel use (W = J/s), LHV basis
        if (OperatingElecEfficiency > 0.0) {
            FuelUseEnergyRateLHV = elecPowerGenerated / OperatingElecEfficiency;
        } else {
            FuelUseEnergyRateLHV = 0.0; // If fuel use rate is zero, then
            elecPowerGenerated = 0.0;   //  electric power generated must be zero.
        }

        //     Set fuel heating values
        fuelHigherHeatingValue = this->FuelHigherHeatingValue;
        fuelLowerHeatingValue = this->FuelLowerHeatingValue;

        //     Calculate fuel mass flow rate
        this->FuelMdot = FuelUseEnergyRateLHV / (fuelLowerHeatingValue * KJtoJ);

        //     Calculate ancillary power requirement
        if (this->AncillaryPowerFuelCurveNum > 0) {
            AnciPowerFMdotFuel = Curve::CurveValue(state, this->AncillaryPowerFuelCurveNum, this->FuelMdot);
            //       Warn user if ancillary power modifier curve output is less than 0
            if (AnciPowerFMdotFuel < 0.0) {
                if (this->AnciPowerFMdotFuelErrorIndex == 0) {
                    ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                    ShowContinueError(
                        state,
                        format("... Ancillary Power Modifier (function of fuel input) output is less than zero ({:.4T}).", AnciPowerFMdotFuel));
                    ShowContinueError(state, format("... Value occurs using a fuel input mass flow rate of {:.4T} kg/s.", this->FuelMdot));
                    ShowContinueErrorTimeStamp(state, "... Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(
                    state,
                    "GENERATOR:MICROTURBINE \"" + this->Name +
                        "\": Ancillary Power Modifier (function of fuel input) output is less than zero warning continues...",
                    this->AnciPowerFMdotFuelErrorIndex,
                    AnciPowerFMdotFuel,
                    AnciPowerFMdotFuel);
                AnciPowerFMdotFuel = 0.0;
            }
        } else {
            AnciPowerFMdotFuel = 1.0;
        }

        // Ancillary power used by pump from last iteration (iteration loop within this subroutine)
        Real64 AncillaryPowerRateLast = ancillaryPowerRate;

        if (this->AncillaryPowerFuelCurveNum > 0) {
            ancillaryPowerRate = RelaxFactor * this->AncillaryPower * AnciPowerFMdotFuel - (1.0 - RelaxFactor) * AncillaryPowerRateLast;
        }

        AncillaryPowerRateDiff = std::abs(ancillaryPowerRate - AncillaryPowerRateLast);
    }

    if (AncPowerCalcIterIndex > MaxAncPowerIter) {

        if (this->AnciPowerIterErrorIndex == 0) {
            ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
            ShowContinueError(state, "... Iteration loop for electric power generation is not converging within tolerance.");
            ShowContinueError(state, "... Check the Ancillary Power Modifier Curve (function of fuel input).");
            ShowContinueError(state, format("... Ancillary Power = {:.1T} W.", ancillaryPowerRate));
            ShowContinueError(state, format("... Fuel input rate = {:.4T} kg/s.", AnciPowerFMdotFuel));
            ShowContinueErrorTimeStamp(state, "... Simulation will continue.");
        }
        ShowRecurringWarningErrorAtEnd(state,
                                       "GENERATOR:MICROTURBINE \"" + this->Name +
                                           "\": Iteration loop for electric power generation is not converging within tolerance continues...",
                                       this->AnciPowerIterErrorIndex);
    }

    //   Calculate electrical power generated
    this->ElecPowerGenerated = elecPowerGenerated - ancillaryPowerRate;

    //   Report fuel energy use rate on HHV basis, which is the unit of measure when the fuel is sold
    this->FuelEnergyUseRateHHV = this->FuelMdot * fuelHigherHeatingValue * KJtoJ;
    this->AncillaryPowerRate = ancillaryPowerRate;     // Move to data structure for later reporting
    this->FuelEnergyUseRateLHV = FuelUseEnergyRateLHV; // Move to data structure for reporting calculations

    //   When generator operates, standby losses are 0
    this->StandbyPowerRate = 0.0;

    Real64 QHeatRecToWater = 0.0; // Recovered waste heat to water (W)

    //   Calculate heat recovery if active
    if (this->HeatRecActive) {

        // Thermal efficiency as a function of air temperature and elevation
        Real64 ThermalEffFTempElev;
        if (this->ThermEffFTempElevCurveNum > 0) {
            ThermalEffFTempElev = Curve::CurveValue(state, this->ThermEffFTempElevCurveNum, CombustionAirInletTemp, state.dataEnvrn->Elevation);
            //       Warn user if power modifier curve output is less than 0
            if (ThermalEffFTempElev < 0.0) {
                if (this->ThermEffFTempElevErrorIndex == 0) {
                    ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                    ShowContinueError(
                        state,
                        format("... Electrical Power Modifier curve (function of temperature and elevation) output is less than zero ({:.4T}).",
                               PowerFTempElev));
                    ShowContinueError(state,
                                      format("... Value occurs using a combustion inlet air temperature of {:.2T} C.", CombustionAirInletTemp));
                    ShowContinueError(state, format("... and an elevation of {:.2T} m.", state.dataEnvrn->Elevation));
                    ShowContinueErrorTimeStamp(state, "... Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "GENERATOR:MICROTURBINE \"" + this->Name +
                                                   "\": Electrical Power Modifier curve is less than zero warning continues...",
                                               this->ThermEffFTempElevErrorIndex,
                                               ThermalEffFTempElev,
                                               ThermalEffFTempElev);
                ThermalEffFTempElev = 0.0;
            }
        } else {
            ThermalEffFTempElev = 1.0; // If no curve provided, assume multiplier factor = 1.0
        }

        QHeatRecToWater = FuelUseEnergyRateLHV * this->RefThermalEffLHV * ThermalEffFTempElev;
        Real64 HeatRecRateFPLR; // Heat recovery rate as a function of PLR curve output

        //     Calculate heat recovery rate modifier curve output (function of PLR)
        if (this->HeatRecRateFPLRCurveNum > 0) {
            HeatRecRateFPLR = Curve::CurveValue(state, this->HeatRecRateFPLRCurveNum, PLR);
            //       Warn user if heat recovery modifier curve output is less than 0
            if (HeatRecRateFPLR < 0.0) {
                if (this->HeatRecRateFPLRErrorIndex == 0) {
                    ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                    ShowContinueError(
                        state,
                        format("... Heat Recovery Rate Modifier (function of part-load ratio) output is less than zero ({:.4T}).", HeatRecRateFPLR));
                    ShowContinueError(state, format("... Value occurs using a part-load ratio of {:.3T}.", PLR));
                    ShowContinueErrorTimeStamp(state, "... Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(
                    state,
                    "GENERATOR:MICROTURBINE \"" + this->Name +
                        "\": Heat Recovery Rate Modifier (function of part-load ratio) output is less than zero warning continues...",
                    this->HeatRecRateFPLRErrorIndex,
                    HeatRecRateFPLR,
                    HeatRecRateFPLR);
                HeatRecRateFPLR = 0.0;
            }
        } else {
            HeatRecRateFPLR = 1.0; // If no curve provided, assume multiplier factor = 1.0
        }

        Real64 HeatRecRateFTemp; // Heat recovery rate as a function of inlet water temp curve output

        //     Calculate heat recovery rate modifier curve output (function of inlet water temp)
        if (this->HeatRecRateFTempCurveNum > 0) {
            HeatRecRateFTemp = Curve::CurveValue(state, this->HeatRecRateFTempCurveNum, HeatRecInTemp);
            if (HeatRecRateFTemp < 0.0) {
                if (this->HeatRecRateFTempErrorIndex == 0) {
                    ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                    ShowContinueError(state,
                                      format("... Heat Recovery Rate Modifier (function of inlet water temp) output is less than zero ({:.4T}).",
                                             HeatRecRateFTemp));
                    ShowContinueError(state, format("... Value occurs using an inlet water temperature temperature of {:.2T} C.", HeatRecInTemp));
                    ShowContinueErrorTimeStamp(state, "... Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(
                    state,
                    "GENERATOR:MICROTURBINE \"" + this->Name +
                        "\": Heat Recovery Rate Modifier (function of inlet water temp) output is less than zero warning continues...",
                    this->HeatRecRateFTempErrorIndex,
                    HeatRecRateFTemp,
                    HeatRecRateFTemp);
                HeatRecRateFTemp = 0.0;
            }
        } else {
            HeatRecRateFTemp = 1.0; // If no curve provided, assume multiplier factor = 1.0
        }

        Real64 HeatRecRateFFlow; // Heat recovery rate as a function of water flow rate curve output

        //     Calculate heat recovery rate modifier curve output (function of water [volumetric] flow rate)
        if (this->HeatRecRateFWaterFlowCurveNum > 0) {
            Real64 rho = state.dataPlnt->PlantLoop(this->HRPlantLoc.loopNum).glycol->getDensity(state, HeatRecInTemp, RoutineName);

            // Heat recovery fluid flow rate (m3/s)
            Real64 HeatRecVolFlowRate = heatRecMdot / rho;
            HeatRecRateFFlow = Curve::CurveValue(state, this->HeatRecRateFWaterFlowCurveNum, HeatRecVolFlowRate);
            if (HeatRecRateFFlow < 0.0) {
                if (this->HeatRecRateFFlowErrorIndex == 0) {
                    ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                    ShowContinueError(
                        state,
                        format("... Heat Recovery Rate Modifier (function of water flow rate) output is less than zero ({:.4T}).", HeatRecRateFFlow));
                    ShowContinueError(state, format("... Value occurs using a water flow rate of {:.4T} m3/s.", HeatRecVolFlowRate));
                    ShowContinueErrorTimeStamp(state, "... Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(
                    state,
                    "GENERATOR:MICROTURBINE \"" + this->Name +
                        "\": Heat Recovery Rate Modifier (function of water flow rate) output is less than zero warning continues...",
                    this->HeatRecRateFFlowErrorIndex,
                    HeatRecRateFFlow,
                    HeatRecRateFFlow);
                HeatRecRateFFlow = 0.0;
            }
        } else {
            HeatRecRateFFlow = 1.0; // If no curve provided, assume multiplier factor = 1.0
        }

        QHeatRecToWater *= HeatRecRateFPLR * HeatRecRateFTemp * HeatRecRateFFlow;

        Real64 HeatRecOutTemp; // Heat recovery fluid outlet temperature (C)

        //     Check for divide by zero
        if ((heatRecMdot > 0.0) && (HeatRecCp > 0.0)) {
            HeatRecOutTemp = HeatRecInTemp + QHeatRecToWater / (heatRecMdot * HeatRecCp);
        } else {
            heatRecMdot = 0.0;
            HeatRecOutTemp = HeatRecInTemp;
            QHeatRecToWater = 0.0;
        }

        //     Now verify the maximum heat recovery temperature was not exceeded
        if (HeatRecOutTemp > this->HeatRecMaxWaterTemp) {

            Real64 MinHeatRecMdot = 0.0; // Heat recovery flow rate if minimal heat recovery is accomplished (kg/s)

            if (this->HeatRecMaxWaterTemp != HeatRecInTemp) {
                MinHeatRecMdot = QHeatRecToWater / (HeatRecCp * (this->HeatRecMaxWaterTemp - HeatRecInTemp));
                if (MinHeatRecMdot < 0.0) {
                    MinHeatRecMdot = 0.0;
                }
            }

            //       Recalculate outlet water temperature with minimum flow rate (will normally match the max water outlet temp,
            //       unless the inlet water temp is greater than the max outlet temp)
            Real64 HRecRatio; // When maximum temperature is reached the amount of recovered heat has to be reduced

            if ((MinHeatRecMdot > 0.0) && (HeatRecCp > 0.0)) {
                HeatRecOutTemp = QHeatRecToWater / (MinHeatRecMdot * HeatRecCp) + HeatRecInTemp;
                HRecRatio = heatRecMdot / MinHeatRecMdot;
            } else {
                HeatRecOutTemp = HeatRecInTemp;
                HRecRatio = 0.0;
            }
            QHeatRecToWater *= HRecRatio; // Scale heat recovery rate using HRecRatio. Don't adjust flow rate.
        }

        //     Check water mass flow rate against minimum
        if (this->HeatRecMinMassFlowRate > heatRecMdot && heatRecMdot > 0.0) {
            if (this->HRMinFlowErrorIndex == 0) {
                ShowWarningError(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                ShowContinueError(state,
                                  format("...Heat reclaim water flow rate is below the generators minimum mass flow rate of ({:.4T}).",
                                         this->HeatRecMinMassFlowRate));
                ShowContinueError(state, format("...Heat reclaim water mass flow rate = {:.4T}.", heatRecMdot));
                ShowContinueErrorTimeStamp(state, "...Check inputs for heat recovery water flow rate.");
            }
            ShowRecurringWarningErrorAtEnd(
                state,
                "GENERATOR:MICROTURBINE \"" + this->Name +
                    "\": Heat recovery water flow rate is below the generators minimum mass flow rate warning continues...",
                this->HRMinFlowErrorIndex,
                heatRecMdot,
                heatRecMdot);
        }

        //     Check water mass flow rate against maximum
        if (heatRecMdot > this->HeatRecMaxMassFlowRate && heatRecMdot > 0.0) {
            if (this->HRMaxFlowErrorIndex == 0) {
                ShowWarningError(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                ShowContinueError(state,
                                  format("...Heat reclaim water flow rate is above the generators maximum mass flow rate of ({:.4T}).",
                                         this->HeatRecMaxMassFlowRate));
                ShowContinueError(state, format("...Heat reclaim water mass flow rate = {:.4T}.", heatRecMdot));
                ShowContinueErrorTimeStamp(state, "...Check inputs for heat recovery water flow rate.");
            }
            ShowRecurringWarningErrorAtEnd(
                state,
                "GENERATOR:MICROTURBINE \"" + this->Name +
                    "\": Heat recovery water flow rate is above the generators maximum mass flow rate warning continues...",
                this->HRMaxFlowErrorIndex,
                heatRecMdot,
                heatRecMdot);
        }

        //     Set report variables
        this->HeatRecInletTemp = HeatRecInTemp;
        this->HeatRecOutletTemp = HeatRecOutTemp;
        this->HeatRecMdot = heatRecMdot;
        this->QHeatRecovered = QHeatRecToWater;

    } // End of  IF (MTGenerator(GeneratorNum)%HeatRecActive) THEN

    //   Calculate combustion air outlet conditions if exhaust air calculations are active
    if (this->ExhAirCalcsActive) {

        Real64 ExhFlowFTemp; // Exhaust air flow rate as a function of temperature curve output

        if (this->ExhFlowFTempCurveNum != 0) { // Exhaust Flow Rate versus Inlet Air Temp
            ExhFlowFTemp = Curve::CurveValue(state, this->ExhFlowFTempCurveNum, CombustionAirInletTemp);
            //       Warn user if exhaust modifier curve output is less than or equal to 0
            if (ExhFlowFTemp <= 0.0) {
                if (this->ExhFlowFTempErrorIndex == 0) {
                    ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                    ShowContinueError(
                        state,
                        format("...Exhaust Air Flow Rate Modifier (function of temperature) output is less than or equal to zero ({:.4T}).",
                               ExhFlowFTemp));
                    ShowContinueError(state, format("...Value occurs using a combustion inlet air temperature of {:.2T}.", CombustionAirInletTemp));
                    ShowContinueErrorTimeStamp(state, "...Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(
                    state,
                    "GENERATOR:MICROTURBINE \"" + this->Name +
                        "\": Exhaust Air Flow Rate Modifier (function of temperature) output is less than or equal to zero warning continues...",
                    this->ExhFlowFTempErrorIndex,
                    ExhFlowFTemp,
                    ExhFlowFTemp);
                ExhFlowFTemp = 0.0;
            }
        } else {
            ExhFlowFTemp = 1.0; // No curve input means modifier = 1.0 always
        }

        Real64 ExhFlowFPLR; // Exhaust air flow rate as a function of part-load ratio curve output

        if (this->ExhFlowFPLRCurveNum != 0) { // Exhaust Flow Rate versus Part-Load Ratio
            ExhFlowFPLR = Curve::CurveValue(state, this->ExhFlowFPLRCurveNum, PLR);
            //       Warn user if exhaust modifier curve output is less than or equal to 0
            if (ExhFlowFPLR <= 0.0) {
                if (this->ExhFlowFPLRErrorIndex == 0) {
                    ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                    ShowContinueError(
                        state,
                        format("...Exhaust Air Flow Rate Modifier (function of part-load ratio) output is less than or equal to zero ({:.4T}).",
                               ExhFlowFPLR));
                    ShowContinueError(state, format("...Value occurs using a part-load ratio of {:.2T}.", PLR));
                    ShowContinueErrorTimeStamp(state, "...Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "GENERATOR:MICROTURBINE \"" + this->Name +
                                                   "\": Exhaust Air Flow Rate Modifier (function of part-load ratio) output is less than or "
                                                   "equal to zero warning continues...",
                                               this->ExhFlowFPLRErrorIndex,
                                               ExhFlowFPLR,
                                               ExhFlowFPLR);
                ExhFlowFPLR = 0.0;
            }
        } else {
            ExhFlowFPLR = 1.0; // No curve input means modifier = 1.0 always
        }

        //     Calculate exhaust air mass flow, accounting for temperature and PLR modifier factors
        // Actual exhaust air mass flow rate (accounting for temp and PLR modifier curves)
        Real64 ExhAirMassFlowRate = this->RefExhaustAirMassFlowRate * ExhFlowFTemp * ExhFlowFPLR;
        //     Adjust for difference in air density at reference conditions versus actual inlet air conditions

        // Density of air at actual combustion inlet air conditions (kg/m3)
        Real64 AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(state, CombustionAirInletPress, CombustionAirInletTemp, CombustionAirInletW);
        if (this->RefCombustAirInletDensity >= 0.0) {
            ExhAirMassFlowRate = max(0.0, ExhAirMassFlowRate * AirDensity / this->RefCombustAirInletDensity);
        } else {
            ExhAirMassFlowRate = 0.0;
        }
        this->ExhaustAirMassFlowRate = ExhAirMassFlowRate;

        Real64 ExhAirTempFTemp; // Exhaust air temperature as a function of inlet air temp curve output

        if (this->ExhAirTempFTempCurveNum != 0) { // Exhaust Air Temp versus Inlet Air Temp
            ExhAirTempFTemp = Curve::CurveValue(state, this->ExhAirTempFTempCurveNum, CombustionAirInletTemp);
            //       Warn user if exhaust modifier curve output is less than or equal to 0
            if (ExhAirTempFTemp <= 0.0) {
                if (this->ExhTempFTempErrorIndex == 0) {
                    ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                    ShowContinueError(
                        state,
                        format("...Exhaust Air Temperature Modifier (function of temperature) output is less than or equal to zero ({:.4T}).",
                               ExhAirTempFTemp));
                    ShowContinueError(state, format("...Value occurs using a combustion inlet air temperature of {:.2T}.", CombustionAirInletTemp));
                    ShowContinueErrorTimeStamp(state, "...Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "GENERATOR:MICROTURBINE \"" + this->Name +
                                                   "\": Exhaust Air Temperature Modifier (function of temperature) output is less than or equal "
                                                   "to zero warning continues...",
                                               this->ExhTempFTempErrorIndex,
                                               ExhAirTempFTemp,
                                               ExhAirTempFTemp);
                ExhAirTempFTemp = 0.0;
            }
        } else {
            ExhAirTempFTemp = 1.0; // No curve input means modifier = 1.0 always
        }

        Real64 ExhAirTempFPLR; // Exhaust air temperature as a function of part-load ratio curve output

        if (this->ExhAirTempFPLRCurveNum != 0) { // Exhaust Air Temp versus Part-Load Ratio
            ExhAirTempFPLR = Curve::CurveValue(state, this->ExhAirTempFPLRCurveNum, PLR);
            //       Warn user if exhaust modifier curve output is less than or equal to 0
            if (ExhAirTempFPLR <= 0.0) {
                if (this->ExhTempFPLRErrorIndex == 0) {
                    ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                    ShowContinueError(
                        state,
                        format("...Exhaust Air Temperature Modifier (function of part-load ratio) output is less than or equal to zero ({:.4T}).",
                               ExhAirTempFPLR));
                    ShowContinueError(state, format("...Value occurs using a part-load ratio of {:.2T}.", PLR));
                    ShowContinueErrorTimeStamp(state, "...Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "GENERATOR:MICROTURBINE \"" + this->Name +
                                                   "\": Exhaust Air Temperature Modifier (function of part-load ratio) output is less than or "
                                                   "equal to zero warning continues...",
                                               this->ExhTempFPLRErrorIndex,
                                               ExhAirTempFPLR,
                                               ExhAirTempFPLR);
                ExhAirTempFPLR = 0.0;
            }
        } else {
            ExhAirTempFPLR = 1.0; // No curve input means modifier = 1.0 always
        }

        if (ExhAirMassFlowRate <= 0.0) {
            this->ExhaustAirTemperature = CombustionAirInletTemp;
            this->ExhaustAirHumRat = CombustionAirInletW;
        } else {
            //       Calculate exhaust air temperature, accounting for inlet air temperature and PLR modifier factors
            // Actual exhaust air temperature (accounting for temp and PLR modifier curves)
            Real64 ExhaustAirTemp = this->NomExhAirOutletTemp * ExhAirTempFTemp * ExhAirTempFPLR;
            this->ExhaustAirTemperature = ExhaustAirTemp;
            //       Adjust exhaust air temperature if heat recovery to water is being done
            if (QHeatRecToWater > 0.0) {
                Real64 CpAir = Psychrometrics::PsyCpAirFnW(CombustionAirInletW);
                if (CpAir > 0.0) {
                    this->ExhaustAirTemperature = ExhaustAirTemp - QHeatRecToWater / (CpAir * ExhAirMassFlowRate);
                }
            }
            //       Calculate exhaust air humidity ratio

            // Heat of vaporization of water (J/kg)
            Real64 H2OHtOfVap = Psychrometrics::PsyHfgAirFnWTdb(1.0, 16.0); // W not used, passing 1.0 as dummy.
            // Assume fuel is at 16C (ASHRAE HOF)
            if (H2OHtOfVap > 0.0) {
                this->ExhaustAirHumRat = CombustionAirInletW + this->FuelMdot *
                                                                   ((fuelHigherHeatingValue - fuelLowerHeatingValue) * KJtoJ / H2OHtOfVap) /
                                                                   ExhAirMassFlowRate;
            } else {
                this->ExhaustAirHumRat = CombustionAirInletW;
            }
        }

        if (this->ExhaustAirTemperature < CombustionAirInletTemp) {
            if (this->ExhTempLTInletTempIndex == 0) {
                ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                ShowContinueError(state,
                                  "...The model has calculated the exhaust air temperature to be less than the combustion air inlet temperature.");
                ShowContinueError(state, format("...Value of exhaust air temperature   ={:.4T} C.", this->ExhaustAirTemperature));
                ShowContinueError(state, format("...Value of combustion air inlet temp ={:.4T} C.", CombustionAirInletTemp));
                ShowContinueErrorTimeStamp(state, "... Simulation will continue.");
            }
            ShowRecurringWarningErrorAtEnd(state,
                                           "GENERATOR:MICROTURBINE \"" + this->Name +
                                               "\": Exhaust air temperature less than combustion air inlet temperature warning continues...",
                                           this->ExhTempLTInletTempIndex,
                                           this->ExhaustAirTemperature,
                                           this->ExhaustAirTemperature);
        }

        if (this->ExhaustAirHumRat < CombustionAirInletW) {
            if (this->ExhHRLTInletHRIndex == 0) {
                ShowWarningMessage(state, format("GENERATOR:MICROTURBINE \"{}\"", this->Name));
                ShowContinueError(
                    state, "...The model has calculated the exhaust air humidity ratio to be less than the combustion air inlet humidity ratio.");
                ShowContinueError(state, format("...Value of exhaust air humidity ratio          ={:.6T} kgWater/kgDryAir.", this->ExhaustAirHumRat));
                ShowContinueError(state, format("...Value of combustion air inlet humidity ratio ={:.6T} kgWater/kgDryAir.", CombustionAirInletW));
                ShowContinueErrorTimeStamp(state, "... Simulation will continue.");
            }
            ShowRecurringWarningErrorAtEnd(state,
                                           "GENERATOR:MICROTURBINE \"" + this->Name +
                                               "\": Exhaust air humidity ratio less than combustion air inlet humidity ratio warning continues...",
                                           this->ExhHRLTInletHRIndex,
                                           this->ExhaustAirHumRat,
                                           this->ExhaustAirHumRat);
        }
    }
}

void MTGeneratorSpecs::UpdateMTGeneratorRecords(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         R. Raustad/D. Shirey
    //       DATE WRITTEN   Mar 2008
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //  Reporting and updating nodes if necessary.

    if (this->HeatRecActive) {
        state.dataLoopNodes->Node(this->HeatRecOutletNodeNum).Temp = this->HeatRecOutletTemp;
    }

    if (this->ExhAirCalcsActive) {
        state.dataLoopNodes->Node(this->CombustionAirOutletNodeNum).MassFlowRate = this->ExhaustAirMassFlowRate;
        state.dataLoopNodes->Node(this->CombustionAirInletNodeNum).MassFlowRate = this->ExhaustAirMassFlowRate;

        state.dataLoopNodes->Node(this->CombustionAirOutletNodeNum).Temp = this->ExhaustAirTemperature;
        state.dataLoopNodes->Node(this->CombustionAirOutletNodeNum).HumRat = this->ExhaustAirHumRat;
        state.dataLoopNodes->Node(this->CombustionAirOutletNodeNum).MassFlowRateMaxAvail =
            state.dataLoopNodes->Node(this->CombustionAirInletNodeNum).MassFlowRateMaxAvail;
        state.dataLoopNodes->Node(this->CombustionAirOutletNodeNum).MassFlowRateMinAvail =
            state.dataLoopNodes->Node(this->CombustionAirInletNodeNum).MassFlowRateMinAvail;
    }

    this->EnergyGen = this->ElecPowerGenerated * state.dataHVACGlobal->TimeStepSysSec;
    this->ExhaustEnergyRec = this->QHeatRecovered * state.dataHVACGlobal->TimeStepSysSec;
    this->FuelEnergyHHV = this->FuelEnergyUseRateHHV * state.dataHVACGlobal->TimeStepSysSec;
    if (this->FuelEnergyUseRateLHV > 0.0) {
        this->ElectricEfficiencyLHV = this->ElecPowerGenerated / this->FuelEnergyUseRateLHV;
        this->ThermalEfficiencyLHV = this->QHeatRecovered / this->FuelEnergyUseRateLHV;
    } else {
        this->ElectricEfficiencyLHV = 0.0;
        this->ThermalEfficiencyLHV = 0.0;
    }
    this->AncillaryEnergy = this->AncillaryPowerRate * state.dataHVACGlobal->TimeStepSysSec;
    this->StandbyEnergy = this->StandbyPowerRate * state.dataHVACGlobal->TimeStepSysSec;
}
void MTGeneratorSpecs::oneTimeInit(EnergyPlusData &state)
{

    std::string const RoutineName("InitMTGenerators");
    bool errFlag;

    if (this->myFlag) {
        this->setupOutputVars(state);
        this->myFlag = false;
    }

    if (this->MyPlantScanFlag && allocated(state.dataPlnt->PlantLoop) && this->HeatRecActive) {
        errFlag = false;
        PlantUtilities::ScanPlantLoopsForObject(
            state, this->Name, DataPlant::PlantEquipmentType::Generator_MicroTurbine, this->HRPlantLoc, errFlag, _, _, _, _, _);
        if (errFlag) {
            ShowFatalError(state, "InitMTGenerators: Program terminated due to previous condition(s).");
        }

        this->MyPlantScanFlag = false;
    }

    if (this->MySizeAndNodeInitFlag && (!this->MyPlantScanFlag) && this->HeatRecActive) {

        // size mass flow rate
        Real64 rho = state.dataPlnt->PlantLoop(this->HRPlantLoc.loopNum).glycol->getDensity(state, Constant::InitConvTemp, RoutineName);

        this->DesignHeatRecMassFlowRate = rho * this->RefHeatRecVolFlowRate;
        this->HeatRecMaxMassFlowRate = rho * this->HeatRecMaxVolFlowRate;

        PlantUtilities::InitComponentNodes(state, 0.0, this->HeatRecMaxMassFlowRate, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum);

        this->MySizeAndNodeInitFlag = false;
    }
}

} // namespace EnergyPlus::MicroturbineElectricGenerator
