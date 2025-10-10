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
#include <EnergyPlus/CTElectricGenerator.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace CTElectricGenerator {

    //__________________________________________________________________________;
    // BLAST inherited generators:
    // CTElectricGenerator (COMBUSTION Turbine)

    // MODULE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   Sept 2000

    // PURPOSE OF THIS MODULE:
    // This module simulates the performance of the COMBUSTION turbine
    // Generators.

    // METHODOLOGY EMPLOYED:
    // Once the Electric power manager determines that the CT Generator
    // is available, it calls SimCTGenerator which in turn calls the
    // appropriate COMBUSTION turbine Generator model.
    // All CT Generator models are based on a polynomial fit of Generator
    // performance data.

    CTGeneratorData *CTGeneratorData::factory(EnergyPlusData &state, std::string const &objectName)
    {
        // Process the input data for generators if it hasn't been done already
        if (state.dataCTElectricGenerator->getCTInputFlag) {
            GetCTGeneratorInput(state);
            state.dataCTElectricGenerator->getCTInputFlag = false;
        }

        // Now look for this particular generator in the list
        auto myCTGen = std::find_if(state.dataCTElectricGenerator->CTGenerator.begin(),
                                    state.dataCTElectricGenerator->CTGenerator.end(),
                                    [&objectName](const CTGeneratorData &CTElecGen) { return CTElecGen.Name == objectName; });
        if (myCTGen != state.dataCTElectricGenerator->CTGenerator.end()) {
            return myCTGen;
        }
        // If we didn't find it, fatal
        ShowFatalError(state,
                       format("LocalCombustionTurbineGeneratorFactory: Error getting inputs for combustion turbine generator named: {}",
                              objectName)); // LCOV_EXCL_LINE
        // Shut up the compiler
        return nullptr; // LCOV_EXCL_LINE
    }

    void CTGeneratorData::simulate([[maybe_unused]] EnergyPlusData &state,
                                   [[maybe_unused]] const EnergyPlus::PlantLocation &calledFromLocation,
                                   [[maybe_unused]] bool FirstHVACIteration,
                                   [[maybe_unused]] Real64 &CurLoad,
                                   [[maybe_unused]] bool RunFlag)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   Sept. 2000

        // PURPOSE OF THIS SUBROUTINE: This is the CT Generator driver.  It
        // gets the input for the models, initializes simulation variables, call
        // the appropriate model and sets up reporting variables.

        // empty function to emulate current behavior as of conversion to using the PlantComponent calling structure.
        // calls from the plant side... do nothing.
        // calls from the ElectricPowerServiceManger call the init and calculation worker functions directly.
    }

    void GetCTGeneratorInput(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Dan Fisher
        //       DATE WRITTEN:    April 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This routine will get the input
        // required by the CT Generator models.
        static constexpr std::string_view routineName = "GetCTGeneratorInput";

        int NumAlphas;                // Number of elements in the alpha array
        int NumNums;                  // Number of elements in the numeric array
        int IOStat;                   // IO Status when calling get input subroutine
        Array1D_string AlphArray(12); // character string data
        Array1D<Real64> NumArray(12); // numeric data
        bool ErrorsFound(false);      // error flag

        state.dataIPShortCut->cCurrentModuleObject = "Generator:CombustionTurbine";
        int NumCTGenerators = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataIPShortCut->cCurrentModuleObject);

        if (NumCTGenerators <= 0) {
            ShowSevereError(state, format("No {} equipment specified in input file", state.dataIPShortCut->cCurrentModuleObject));
            ErrorsFound = true;
        }

        // ALLOCATE ARRAYS
        state.dataCTElectricGenerator->CTGenerator.allocate(NumCTGenerators);

        // LOAD ARRAYS WITH CT CURVE FIT Generator DATA
        for (int genNum = 1; genNum <= NumCTGenerators; ++genNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataIPShortCut->cCurrentModuleObject,
                                                                     genNum,
                                                                     AlphArray,
                                                                     NumAlphas,
                                                                     NumArray,
                                                                     NumNums,
                                                                     IOStat,
                                                                     _,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)};

            state.dataCTElectricGenerator->CTGenerator(genNum).Name = AlphArray(1);

            state.dataCTElectricGenerator->CTGenerator(genNum).RatedPowerOutput = NumArray(1);
            if (NumArray(1) == 0.0) {
                ShowSevereError(state, format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(1), NumArray(1)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }

            // Not sure what to do with electric nodes, so do not use optional arguments
            state.dataCTElectricGenerator->CTGenerator(genNum).ElectricCircuitNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    AlphArray(2),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::GeneratorCombustionTurbine,
                                                    AlphArray(1),
                                                    DataLoopNode::NodeFluidType::Electric,
                                                    DataLoopNode::ConnectionType::Electric,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);

            state.dataCTElectricGenerator->CTGenerator(genNum).MinPartLoadRat = NumArray(2);
            state.dataCTElectricGenerator->CTGenerator(genNum).MaxPartLoadRat = NumArray(3);
            state.dataCTElectricGenerator->CTGenerator(genNum).OptPartLoadRat = NumArray(4);

            // Load Special CT Generator Input

            state.dataCTElectricGenerator->CTGenerator(genNum).PLBasedFuelInputCurve = Curve::GetCurve(state, AlphArray(3));
            if (state.dataCTElectricGenerator->CTGenerator(genNum).PLBasedFuelInputCurve == 0) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphArray(3));
                ErrorsFound = true;
            }

            state.dataCTElectricGenerator->CTGenerator(genNum).TempBasedFuelInputCurve = Curve::GetCurve(state, AlphArray(4));
            if (state.dataCTElectricGenerator->CTGenerator(genNum).TempBasedFuelInputCurve == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(4), AlphArray(4));
                ErrorsFound = true;
            }

            state.dataCTElectricGenerator->CTGenerator(genNum).ExhaustFlowCurve = Curve::GetCurve(state, AlphArray(5));
            if (state.dataCTElectricGenerator->CTGenerator(genNum).ExhaustFlowCurve == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(5), AlphArray(5));
                ErrorsFound = true;
            }

            state.dataCTElectricGenerator->CTGenerator(genNum).PLBasedExhaustTempCurve = Curve::GetCurve(state, AlphArray(6));
            if (state.dataCTElectricGenerator->CTGenerator(genNum).PLBasedExhaustTempCurve == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(6), AlphArray(6));
                ErrorsFound = true;
            }

            state.dataCTElectricGenerator->CTGenerator(genNum).TempBasedExhaustTempCurve = Curve::GetCurve(state, AlphArray(7));
            if (state.dataCTElectricGenerator->CTGenerator(genNum).TempBasedExhaustTempCurve == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(7), AlphArray(7));
                ErrorsFound = true;
            }

            state.dataCTElectricGenerator->CTGenerator(genNum).QLubeOilRecoveredCurve = Curve::GetCurve(state, AlphArray(8));
            if (state.dataCTElectricGenerator->CTGenerator(genNum).QLubeOilRecoveredCurve == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(8), AlphArray(8));
                ErrorsFound = true;
            }

            state.dataCTElectricGenerator->CTGenerator(genNum).UACoef[0] = NumArray(5);
            state.dataCTElectricGenerator->CTGenerator(genNum).UACoef[1] = NumArray(6);

            state.dataCTElectricGenerator->CTGenerator(genNum).MaxExhaustperCTPower = NumArray(7);
            state.dataCTElectricGenerator->CTGenerator(genNum).DesignMinExitGasTemp = NumArray(8);
            state.dataCTElectricGenerator->CTGenerator(genNum).DesignAirInletTemp = NumArray(9);
            state.dataCTElectricGenerator->CTGenerator(genNum).FuelHeatingValue = NumArray(10);
            state.dataCTElectricGenerator->CTGenerator(genNum).DesignHeatRecVolFlowRate = NumArray(11);

            if (state.dataCTElectricGenerator->CTGenerator(genNum).DesignHeatRecVolFlowRate > 0.0) {
                state.dataCTElectricGenerator->CTGenerator(genNum).HeatRecActive = true;
                state.dataCTElectricGenerator->CTGenerator(genNum).HeatRecInletNodeNum =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        AlphArray(9),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::GeneratorCombustionTurbine,
                                                        AlphArray(1),
                                                        DataLoopNode::NodeFluidType::Water,
                                                        DataLoopNode::ConnectionType::Inlet,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);
                if (state.dataCTElectricGenerator->CTGenerator(genNum).HeatRecInletNodeNum == 0) {
                    ShowSevereError(
                        state, format("Missing Node Name, Heat Recovery Inlet, for {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
                    ErrorsFound = true;
                }
                state.dataCTElectricGenerator->CTGenerator(genNum).HeatRecOutletNodeNum =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        AlphArray(10),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::GeneratorCombustionTurbine,
                                                        AlphArray(1),
                                                        DataLoopNode::NodeFluidType::Water,
                                                        DataLoopNode::ConnectionType::Outlet,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);
                if (state.dataCTElectricGenerator->CTGenerator(genNum).HeatRecOutletNodeNum == 0) {
                    ShowSevereError(
                        state,
                        format("Missing Node Name, Heat Recovery Outlet, for {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
                    ErrorsFound = true;
                }
                BranchNodeConnections::TestCompSet(
                    state, state.dataIPShortCut->cCurrentModuleObject, AlphArray(1), AlphArray(9), AlphArray(10), "Heat Recovery Nodes");
                PlantUtilities::RegisterPlantCompDesignFlow(state,
                                                            state.dataCTElectricGenerator->CTGenerator(genNum).HeatRecInletNodeNum,
                                                            state.dataCTElectricGenerator->CTGenerator(genNum).DesignHeatRecVolFlowRate);
            } else {
                state.dataCTElectricGenerator->CTGenerator(genNum).HeatRecActive = false;
                state.dataCTElectricGenerator->CTGenerator(genNum).HeatRecInletNodeNum = 0;
                state.dataCTElectricGenerator->CTGenerator(genNum).HeatRecOutletNodeNum = 0;
                if (!state.dataIPShortCut->lAlphaFieldBlanks(9) || !state.dataIPShortCut->lAlphaFieldBlanks(10)) {
                    ShowWarningError(state,
                                     format("Since Design Heat Flow Rate = 0.0, Heat Recovery inactive for {}={}",
                                            state.dataIPShortCut->cCurrentModuleObject,
                                            AlphArray(1)));
                    ShowContinueError(state, "However, Node names were specified for Heat Recovery inlet or outlet nodes");
                }
            }

            // Validate fuel type input
            state.dataCTElectricGenerator->CTGenerator(genNum).FuelType =
                static_cast<Constant::eFuel>(getEnumValue(Constant::eFuelNamesUC, AlphArray(11)));
            if (state.dataCTElectricGenerator->CTGenerator(genNum).FuelType == Constant::eFuel::Invalid) {
                ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(11), AlphArray(11)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }

            state.dataCTElectricGenerator->CTGenerator(genNum).HeatRecMaxTemp = NumArray(12);

            // begin CR7021
            if (state.dataIPShortCut->lAlphaFieldBlanks(12)) {
                state.dataCTElectricGenerator->CTGenerator(genNum).OAInletNode = 0;
            } else {
                state.dataCTElectricGenerator->CTGenerator(genNum).OAInletNode =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        AlphArray(12),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::GeneratorCombustionTurbine,
                                                        AlphArray(1),
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::OutsideAirReference,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);
                if (!OutAirNodeManager::CheckOutAirNodeNumber(state, state.dataCTElectricGenerator->CTGenerator(genNum).OAInletNode)) {
                    ShowSevereError(state,
                                    format("{}, \"{}\" Outdoor Air Inlet Node Name not valid Outdoor Air Node= {}",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           state.dataCTElectricGenerator->CTGenerator(genNum).Name,
                                           AlphArray(12)));
                    ShowContinueError(state, "...does not appear in an OutdoorAir:NodeList or as an OutdoorAir:Node.");
                    ErrorsFound = true;
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("Errors found in processing input for {}", state.dataIPShortCut->cCurrentModuleObject));
        }
    }

    void CTGeneratorData::setupOutputVars(EnergyPlusData &state)
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
                            this->ElecEnergyGenerated,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name,
                            Constant::eResource::ElectricityProduced,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::Cogeneration);

        SetupOutputVariable(state,
                            format("Generator {} Rate", sFuelType),
                            Constant::Units::W,
                            this->FuelEnergyUseRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            format("Generator {} Energy", sFuelType),
                            Constant::Units::J,
                            this->FuelEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name,
                            Constant::eFuel2eResource[(int)this->FuelType],
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::Cogeneration);

        //    general fuel use report (to match other generators)
        SetupOutputVariable(state,
                            "Generator Fuel HHV Basis Rate",
                            Constant::Units::W,
                            this->FuelEnergyUseRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Generator Fuel HHV Basis Energy",
                            Constant::Units::J,
                            this->FuelEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);

        SetupOutputVariable(state,
                            format("Generator {} Mass Flow Rate", sFuelType),
                            Constant::Units::kg_s,
                            this->FuelMdot,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Generator Exhaust Air Temperature",
                            Constant::Units::C,
                            this->ExhaustStackTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        if (this->HeatRecActive) {
            SetupOutputVariable(state,
                                "Generator Exhaust Heat Recovery Rate",
                                Constant::Units::W,
                                this->QExhaustRecovered,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Generator Exhaust Heat Recovery Energy",
                                Constant::Units::J,
                                this->ExhaustEnergyRec,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRecovery);

            SetupOutputVariable(state,
                                "Generator Lube Heat Recovery Rate",
                                Constant::Units::W,
                                this->QLubeOilRecovered,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Generator Lube Heat Recovery Energy",
                                Constant::Units::J,
                                this->LubeOilEnergyRec,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRecovery);

            SetupOutputVariable(state,
                                "Generator Produced Thermal Rate",
                                Constant::Units::W,
                                this->QTotalHeatRecovered,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Generator Produced Thermal Energy",
                                Constant::Units::J,
                                this->TotalHeatEnergyRec,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
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
                                "Generator Heat Recovery Mass Flow Rate",
                                Constant::Units::kg_s,
                                this->HeatRecMdot,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
        }
    }

    void CTGeneratorData::CalcCTGeneratorModel(EnergyPlusData &state,
                                               bool const RunFlag,  // TRUE when Generator operating
                                               Real64 const MyLoad, // Generator demand
                                               bool const FirstHVACIteration)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   Sept. 2000

        // PURPOSE OF THIS SUBROUTINE:
        // simulate a vapor compression Generator using the CT model

        // METHODOLOGY EMPLOYED:
        // curve fit of performance data.  This model was originally
        // developed by Dale Herron for the BLAST program

        Real64 constexpr exhaustCp(1.047); // Exhaust Gas Specific Heat (J/kg-K)
        Real64 constexpr KJtoJ(1000.0);    // convert Kjoules to joules
        static constexpr std::string_view RoutineName("CalcCTGeneratorModel");

        // min allowed operating frac full load
        Real64 minPartLoadRat = this->MinPartLoadRat;

        // max allowed operating frac full load
        Real64 maxPartLoadRat = this->MaxPartLoadRat;

        // Generator nominal capacity (W)
        Real64 ratedPowerOutput = this->RatedPowerOutput;

        // MAX EXHAUST FLOW PER W POWER OUTPUT COEFF
        Real64 maxExhaustperCTPower = this->MaxExhaustperCTPower;

        // design turbine inlet temperature (C)
        Real64 designAirInletTemp = this->DesignAirInletTemp;

        Real64 heatRecInTemp; // Heat Recovery Fluid Inlet Temperature (C)

        Real64 heatRecMdot; // Heat Recovery Fluid Mass FlowRate (kg/s)
        Real64 heatRecCp;   // Specific Heat of the Heat Recovery Fluid (J/kg-K)

        if (this->HeatRecActive) {
            int heatRecInNode = this->HeatRecInletNodeNum;
            heatRecInTemp = state.dataLoopNodes->Node(heatRecInNode).Temp;

            heatRecCp = state.dataPlnt->PlantLoop(this->HRPlantLoc.loopNum).glycol->getSpecificHeat(state, heatRecInTemp, RoutineName);
            if (FirstHVACIteration && RunFlag) {
                heatRecMdot = this->DesignHeatRecMassFlowRate;
            } else {
                heatRecMdot = state.dataLoopNodes->Node(heatRecInNode).MassFlowRate;
            }
        } else {
            heatRecInTemp = 0.0;
            heatRecCp = 0.0;
            heatRecMdot = 0.0;
        }

        // If no loop demand or Generator OFF, return
        if (!RunFlag) {
            this->ElecPowerGenerated = 0.0;
            this->ElecEnergyGenerated = 0.0;
            this->HeatRecInletTemp = heatRecInTemp;
            this->HeatRecOutletTemp = heatRecInTemp;
            this->HeatRecMdot = 0.0;
            this->QLubeOilRecovered = 0.0;
            this->QExhaustRecovered = 0.0;
            this->QTotalHeatRecovered = 0.0;
            this->LubeOilEnergyRec = 0.0;
            this->ExhaustEnergyRec = 0.0;
            this->TotalHeatEnergyRec = 0.0;
            this->FuelEnergyUseRate = 0.0;
            this->FuelEnergy = 0.0;
            this->FuelMdot = 0.0;
            this->ExhaustStackTemp = 0.0;
            return;
        }

        // CALCULATE POWER GENERATED AND PLR
        // Generator output (W)
        Real64 elecPowerGenerated = min(MyLoad, ratedPowerOutput);
        elecPowerGenerated = max(elecPowerGenerated, 0.0);

        // Generator operating part load ratio
        Real64 PLR = min(elecPowerGenerated / ratedPowerOutput, maxPartLoadRat);
        PLR = max(PLR, minPartLoadRat);
        elecPowerGenerated = PLR * ratedPowerOutput;

        // SET OFF-DESIGN AIR TEMPERATURE DIFFERENCE
        // (ATAIR) Difference between ambient actual and ambient design temperatures
        Real64 ambientDeltaT;
        if (this->OAInletNode == 0) {
            ambientDeltaT = state.dataEnvrn->OutDryBulbTemp - designAirInletTemp;
        } else {
            ambientDeltaT = state.dataLoopNodes->Node(this->OAInletNode).Temp - designAirInletTemp;
        }

        // Use Curve fit to determine Fuel Energy Input.  For electric power generated in Watts, the fuel
        // energy input is calculated in J/s.  The PLBasedFuelInputCurve selects ratio of fuel flow (J/s)/power generated (J/s).
        // The TempBasedFuelInputCurve is a correction based on deviation from design inlet air temperature conditions.
        // The first coefficient of this fit should be 1.0 to ensure that no correction is made at design conditions.
        // (EFUEL) rate of Fuel Energy Required to run COMBUSTION turbine (W)
        Real64 FuelUseRate =
            elecPowerGenerated * this->PLBasedFuelInputCurve->value(state, PLR) * this->TempBasedFuelInputCurve->value(state, ambientDeltaT);

        // Use Curve fit to determine Exhaust Flow.  This curve shows the ratio of exhaust gas flow (kg/s) to electric power
        // output (J/s).  The units on ExhaustFlowCurve are (kg/J).  When multiplied by the rated power of the unit,
        // it gives the exhaust flow rate in kg/s
        // (FEX) Exhaust Gas Flow Rate cubic meters per second???
        Real64 exhaustFlow = ratedPowerOutput * this->ExhaustFlowCurve->value(state, ambientDeltaT);

        // Use Curve fit to determine Exhaust Temperature.  This curve calculates the exhaust temperature (C) by
        // multiplying the exhaust temperature (C) for a particular part load as given by PLBasedExhaustTempCurve
        // a correction factor based on the deviation from design temperature, TempBasedExhaustTempCurve

        Real64 QExhaustRec;      // recovered exhaust heat (W)
        Real64 exhaustStackTemp; // turbine stack temp. (C)
        if ((PLR > 0.0) && ((exhaustFlow > 0.0) || (maxExhaustperCTPower > 0.0))) {

            // (TEX) Exhaust Gas Temperature in C
            Real64 exhaustTemp = this->PLBasedExhaustTempCurve->value(state, PLR) * this->TempBasedExhaustTempCurve->value(state, ambientDeltaT);

            // (UACGC) Heat Exchanger UA to Capacity
            Real64 UA_loc = this->UACoef[0] * std::pow(ratedPowerOutput, this->UACoef[1]);

            // design engine stack saturated steam temp. (C)
            Real64 designMinExitGasTemp = this->DesignMinExitGasTemp;

            exhaustStackTemp = designMinExitGasTemp + (exhaustTemp - designMinExitGasTemp) /
                                                          std::exp(UA_loc / (max(exhaustFlow, maxExhaustperCTPower * ratedPowerOutput) * exhaustCp));

            QExhaustRec = max(exhaustFlow * exhaustCp * (exhaustTemp - exhaustStackTemp), 0.0);
        } else {
            exhaustStackTemp = this->DesignMinExitGasTemp;
            QExhaustRec = 0.0;
        }

        // Use Curve fit to determine Heat Recovered Lubricant heat.  This curve calculates the lube heat recovered (J/s) by
        // multiplying the total power generated by the fraction of that power that could be recovered in the lube oil at that
        // particular part load.
        // recovered lube oil heat (W)
        Real64 QLubeOilRec = elecPowerGenerated * this->QLubeOilRecoveredCurve->value(state, PLR);

        // Check for divide by zero
        Real64 HeatRecOutTemp; // Heat Recovery Fluid Outlet Temperature (C)
        if ((heatRecMdot > 0.0) && (heatRecCp > 0.0)) {
            HeatRecOutTemp = (QExhaustRec + QLubeOilRec) / (heatRecMdot * heatRecCp) + heatRecInTemp;
        } else {
            heatRecMdot = 0.0;
            HeatRecOutTemp = heatRecInTemp;
            QExhaustRec = 0.0;
            QLubeOilRec = 0.0;
        }

        // Now verify the maximum temperature was not exceeded
        // Heat Recovery Flow Rate if minimal heat recovery is accomplished
        Real64 MinHeatRecMdot = 0.0;

        Real64 HRecRatio; // When Max Temp is reached the amount of recovered heat has to be reduced.

        if (HeatRecOutTemp > this->HeatRecMaxTemp) {
            if (this->HeatRecMaxTemp != heatRecInTemp) {
                MinHeatRecMdot = (QExhaustRec + QLubeOilRec) / (heatRecCp * (this->HeatRecMaxTemp - heatRecInTemp));
                if (MinHeatRecMdot < 0.0) {
                    MinHeatRecMdot = 0.0;
                }
            }

            // Recalculate Outlet Temperature, with adjusted flowrate
            if ((MinHeatRecMdot > 0.0) && (heatRecCp > 0.0)) {
                HeatRecOutTemp = (QExhaustRec + QLubeOilRec) / (MinHeatRecMdot * heatRecCp) + heatRecInTemp;
                HRecRatio = heatRecMdot / MinHeatRecMdot;
            } else {
                HeatRecOutTemp = heatRecInTemp;
                HRecRatio = 0.0;
            }
            QLubeOilRec *= HRecRatio;
            QExhaustRec *= HRecRatio;
        }

        // Calculate Energy
        // Generator output (J)
        Real64 ElectricEnergyGen = elecPowerGenerated * state.dataHVACGlobal->TimeStepSysSec;

        // Amount of Fuel Energy Required to run COMBUSTION turbine (J)
        Real64 FuelEnergyUsed = FuelUseRate * state.dataHVACGlobal->TimeStepSysSec;

        // recovered lube oil heat (J)
        Real64 lubeOilEnergyRec = QLubeOilRec * state.dataHVACGlobal->TimeStepSysSec;

        // recovered exhaust heat (J)
        Real64 exhaustEnergyRec = QExhaustRec * state.dataHVACGlobal->TimeStepSysSec;

        this->ElecPowerGenerated = elecPowerGenerated;
        this->ElecEnergyGenerated = ElectricEnergyGen;

        this->HeatRecInletTemp = heatRecInTemp;
        this->HeatRecOutletTemp = HeatRecOutTemp;

        this->HeatRecMdot = heatRecMdot;
        this->QExhaustRecovered = QExhaustRec;
        this->QLubeOilRecovered = QLubeOilRec;
        this->QTotalHeatRecovered = QExhaustRec + QLubeOilRec;
        this->FuelEnergyUseRate = std::abs(FuelUseRate);
        this->ExhaustEnergyRec = exhaustEnergyRec;
        this->LubeOilEnergyRec = lubeOilEnergyRec;
        this->TotalHeatEnergyRec = exhaustEnergyRec + lubeOilEnergyRec;
        this->FuelEnergy = std::abs(FuelEnergyUsed);

        // Heating Value of Fuel in (kJ/kg)
        Real64 fuelHeatingValue = this->FuelHeatingValue;

        this->FuelMdot = std::abs(FuelUseRate) / (fuelHeatingValue * KJtoJ);

        this->ExhaustStackTemp = exhaustStackTemp;

        if (this->HeatRecActive) {
            int HeatRecOutletNode = this->HeatRecOutletNodeNum;
            state.dataLoopNodes->Node(HeatRecOutletNode).Temp = this->HeatRecOutletTemp;
        }
    }

    void CTGeneratorData::InitCTGenerators(EnergyPlusData &state,
                                           bool const RunFlag, // TRUE when Generator operating
                                           bool const FirstHVACIteration)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   Oct 2000
        //       RE-ENGINEERED  Brent Griffith, Sept 2010 plant upgrades, generalize fluid props

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the CT generators.

        this->oneTimeInit(state); // Do one-time inits

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && this->MyEnvrnFlag && this->HeatRecActive) {
            int HeatRecInletNode = this->HeatRecInletNodeNum;
            int HeatRecOutletNode = this->HeatRecOutletNodeNum;
            // set the node Temperature, assuming freeze control
            state.dataLoopNodes->Node(HeatRecInletNode).Temp = 20.0;
            state.dataLoopNodes->Node(HeatRecOutletNode).Temp = 20.0;
            // set the node max and min mass flow rates
            PlantUtilities::InitComponentNodes(state, 0.0, this->DesignHeatRecMassFlowRate, HeatRecInletNode, HeatRecOutletNode);

            this->MyEnvrnFlag = false;
        } // end environmental inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->MyEnvrnFlag = true;
        }

        if (this->HeatRecActive) {
            if (FirstHVACIteration) {
                Real64 mdot;
                if (RunFlag) {
                    mdot = this->DesignHeatRecMassFlowRate;
                } else {
                    mdot = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(state, mdot, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum, this->HRPlantLoc);

            } else {
                PlantUtilities::SetComponentFlowRate(
                    state, this->HeatRecMdot, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum, this->HRPlantLoc);
            }
        }
    }

    void CTGeneratorData::oneTimeInit(EnergyPlusData &state)
    {
        std::string_view constexpr RoutineName("InitICEngineGenerators");

        if (this->MyPlantScanFlag) { // this flag to be removed
            if (allocated(state.dataPlnt->PlantLoop) && this->HeatRecActive) {
                bool errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(
                    state, this->Name, DataPlant::PlantEquipmentType::Generator_CTurbine, this->HRPlantLoc, errFlag, _, _, _, _, _);
                if (errFlag) {
                    ShowFatalError(state, "InitCTGenerators: Program terminated due to previous condition(s).");
                }
            }

            this->MyPlantScanFlag = false;
        }

        if (this->MyFlag) {
            this->setupOutputVars(state);
            this->MyFlag = false;
        }

        if (this->MySizeAndNodeInitFlag && (!this->MyPlantScanFlag) && this->HeatRecActive) {
            int HeatRecInletNode = this->HeatRecInletNodeNum;
            int HeatRecOutletNode = this->HeatRecOutletNodeNum;

            // size mass flow rate
            Real64 rho = state.dataPlnt->PlantLoop(this->HRPlantLoc.loopNum).glycol->getDensity(state, Constant::InitConvTemp, RoutineName);

            this->DesignHeatRecMassFlowRate = rho * this->DesignHeatRecVolFlowRate;

            PlantUtilities::InitComponentNodes(state, 0.0, this->DesignHeatRecMassFlowRate, HeatRecInletNode, HeatRecOutletNode);

            this->MySizeAndNodeInitFlag = false;
        }
    }

} // namespace CTElectricGenerator

} // namespace EnergyPlus
