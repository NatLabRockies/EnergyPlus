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
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PlantComponentTemperatureSources.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace PlantComponentTemperatureSources {

    // MODULE INFORMATION:
    //       AUTHOR         Edwin Lee
    //       DATE WRITTEN   November 2012
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // This module simulates plant supply components which operate against a
    //  predefined (but variable) boundary temperature.

    // METHODOLOGY EMPLOYED:
    // Called by PlantLoopEquipment, model accepts inputs, and calculates a
    // thermal response using new plant routines such as SetComponentFlowRate

    PlantComponent *WaterSourceSpecs::factory(EnergyPlusData &state, std::string const &objectName)
    {
        if (state.dataPlantCompTempSrc->getWaterSourceInput) {
            GetWaterSourceInput(state);
            state.dataPlantCompTempSrc->getWaterSourceInput = false;
        }

        // Now look for this particular pipe in the list
        for (auto &waterSource : state.dataPlantCompTempSrc->WaterSource) {
            if (waterSource.Name == objectName) {
                return &waterSource;
            }
        }
        // If we didn't find it, fatal
        ShowFatalError(state,
                       format("LocalTemperatureSourceFactory: Error getting inputs for temperature source named: {}", objectName)); // LCOV_EXCL_LINE
        // Shut up the compiler
        return nullptr; // LCOV_EXCL_LINE
    }

    void WaterSourceSpecs::initialize(EnergyPlusData &state, Real64 &MyLoad)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   November 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the water source objects

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("InitWaterSource");

        this->oneTimeInit(state);

        // Initialize critical Demand Side Variables at the beginning of each environment
        if (this->MyEnvironFlag && state.dataGlobal->BeginEnvrnFlag && (state.dataPlnt->PlantFirstSizesOkayToFinalize)) {

            Real64 rho = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
            this->MassFlowRateMax = this->DesVolFlowRate * rho;
            PlantUtilities::InitComponentNodes(state, 0.0, this->MassFlowRateMax, this->InletNodeNum, this->OutletNodeNum);

            this->MyEnvironFlag = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->MyEnvironFlag = true;
        }

        // OK, so we can set up the inlet and boundary temperatures now
        this->InletTemp = state.dataLoopNodes->Node(this->InletNodeNum).Temp;
        if (this->tempSpecType == TempSpecType::Schedule) {
            this->BoundaryTemp = this->tempSpecSched->getCurrentVal();
        }

        // Calculate specific heat
        Real64 cp = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->BoundaryTemp, RoutineName);

        // Calculate deltaT
        Real64 delta_temp = this->BoundaryTemp - this->InletTemp;

        // If deltaT is zero then we cannot calculate a flow request, but we may still want one
        //   If myload is greater than zero, then lets request full flow at the current temperature as it may still be meeting load
        //   If myload is zero, we'll then request zero flow
        // If deltaT is non-zero then we can use the current load and deltaT to calculate a flow request:
        //   If MyLoad is > 0 then we want to heat the loop
        //   If MyLoad is < 0 then we want to cool the loop
        //   Thus, given a fixed outlet temperature (the boundary temp, Tbnd), the eq is:
        //     MyLoad = mdot * cp * (Tbnd - Tin)
        //   re-arranging:
        //     mdot = MyLoad / [cp * (Tbnd - Tin)]
        //  If there is a deltaT, but no load, the mass flow request will go to zero anyway
        if (std::abs(delta_temp) < 0.001) {
            if (std::abs(MyLoad) < 0.001) {
                this->MassFlowRate = 0.0;
            } else {
                this->MassFlowRate = this->MassFlowRateMax;
            }
        } else {
            this->MassFlowRate = MyLoad / (cp * delta_temp);
        }

        // If the mdot is negative it means we can't help the load so we will want to just go to zero.
        // If the mdot is already zero, then well, we still want to go to zero
        // If the mdot is positive, just make sure we constrain it to the design value
        if (this->MassFlowRate < 0) {
            this->MassFlowRate = 0.0;
        } else {
            if (!this->EMSOverrideOnMassFlowRateMax) {
                this->MassFlowRate = min(this->MassFlowRate, this->MassFlowRateMax);
            } else {
                this->MassFlowRate = min(this->MassFlowRate, this->EMSOverrideValueMassFlowRateMax);
            }
        }

        PlantUtilities::SetComponentFlowRate(state, this->MassFlowRate, this->InletNodeNum, this->OutletNodeNum, this->plantLoc);

        // at this point the mass flow rate, inlet temp, and boundary temp structure vars have been updated
        // the calc routine will update the outlet temp and heat transfer rate/energies
    }

    void WaterSourceSpecs::setupOutputVars(EnergyPlusData &state)
    {

        SetupOutputVariable(state,
                            "Plant Temperature Source Component Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->MassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Plant Temperature Source Component Inlet Temperature",
                            Constant::Units::C,
                            this->InletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Plant Temperature Source Component Outlet Temperature",
                            Constant::Units::C,
                            this->OutletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Plant Temperature Source Component Source Temperature",
                            Constant::Units::C,
                            this->BoundaryTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Plant Temperature Source Component Heat Transfer Rate",
                            Constant::Units::W,
                            this->HeatRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Plant Temperature Source Component Heat Transfer Energy",
                            Constant::Units::J,
                            this->HeatEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);
        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            SetupEMSActuator(state,
                             "PlantComponent:TemperatureSource",
                             this->Name,
                             "Maximum Mass Flow Rate",
                             "[kg/s]",
                             this->EMSOverrideOnMassFlowRateMax,
                             this->EMSOverrideValueMassFlowRateMax);
        }
    }

    void WaterSourceSpecs::autosize(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   November 2012
        //       MODIFIED       November 2013 Daeho Kang, add component sizing table entries
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing water source design flow rate

        // METHODOLOGY EMPLOYED:
        // Obtains flow rate from the plant sizing array.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool ErrorsFound(false);   // If errors detected in input
        Real64 DesVolFlowRateUser; // Hardsized design volume flow rate for reporting
        Real64 tmpVolFlowRate = this->DesVolFlowRate;
        int PltSizNum = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).PlantSizNum;

        if (PltSizNum > 0) {
            if (state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
                tmpVolFlowRate = state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate; //* WaterSource(SourceNum)%SizFac
                if (!this->DesVolFlowRateWasAutoSized) {
                    tmpVolFlowRate = this->DesVolFlowRate;
                }
            } else {
                if (this->DesVolFlowRateWasAutoSized) {
                    tmpVolFlowRate = 0.0;
                }
            }
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (this->DesVolFlowRateWasAutoSized) {
                    this->DesVolFlowRate = tmpVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(
                            state, "PlantComponent:TemperatureSource", this->Name, "Design Size Design Fluid Flow Rate [m3/s]", tmpVolFlowRate);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "PlantComponent:TemperatureSource",
                                                     this->Name,
                                                     "Initial Design Size Design Fluid Flow Rate [m3/s]",
                                                     tmpVolFlowRate);
                    }
                } else {
                    if (this->DesVolFlowRate > 0.0 && tmpVolFlowRate > 0.0) {
                        DesVolFlowRateUser = this->DesVolFlowRate;
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         "PlantComponent:TemperatureSource",
                                                         this->Name,
                                                         "Design Size Design Fluid Flow Rate [m3/s]",
                                                         tmpVolFlowRate,
                                                         "User-Specified Design Fluid Flow Rate [m3/s]",
                                                         DesVolFlowRateUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(tmpVolFlowRate - DesVolFlowRateUser) / DesVolFlowRateUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(
                                        state,
                                        format("SizePlantComponentTemperatureSource: Potential issue with equipment sizing for {}", this->Name));
                                    ShowContinueError(state, format("User-Specified Design Fluid Flow Rate of {:.5R} [m3/s]", DesVolFlowRateUser));
                                    ShowContinueError(state,
                                                      format("differs from Design Size Design Fluid Flow Rate of {:.5R} [m3/s]", tmpVolFlowRate));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                        tmpVolFlowRate = DesVolFlowRateUser;
                    }
                }
            }
        } else {
            if (this->DesVolFlowRateWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                ShowSevereError(state, "Autosizing of plant component temperature source flow rate requires a loop Sizing:Plant object");
                ShowContinueError(state, format("Occurs in PlantComponent:TemperatureSource object={}", this->Name));
                ErrorsFound = true;
            }
            if (!this->DesVolFlowRateWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport) {
                if (this->DesVolFlowRate > 0.0) {
                    BaseSizer::reportSizerOutput(
                        state, "PlantComponent:TemperatureSource", this->Name, "User-Specified Design Fluid Flow Rate [m3/s]", this->DesVolFlowRate);
                }
            }
        }

        PlantUtilities::RegisterPlantCompDesignFlow(state, this->InletNodeNum, tmpVolFlowRate);

        if (ErrorsFound) {
            ShowFatalError(state, "Preceding sizing errors cause program termination");
        }
    }

    void WaterSourceSpecs::calculate(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   October 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        static constexpr std::string_view RoutineName("CalcWaterSource");

        if (this->MassFlowRate > 0.0) {
            this->OutletTemp = this->BoundaryTemp;
            Real64 Cp = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->BoundaryTemp, RoutineName);
            this->HeatRate = this->MassFlowRate * Cp * (this->OutletTemp - this->InletTemp);
            this->HeatEnergy = this->HeatRate * state.dataHVACGlobal->TimeStepSysSec;
        } else {
            this->OutletTemp = this->BoundaryTemp;
            this->HeatRate = 0.0;
            this->HeatEnergy = 0.0;
        }
    }

    void WaterSourceSpecs::update(EnergyPlusData &state)
    {
        state.dataLoopNodes->Node(this->OutletNodeNum).Temp = this->OutletTemp;
    }

    void WaterSourceSpecs::simulate(EnergyPlusData &state,
                                    [[maybe_unused]] const PlantLocation &calledFromLocation,
                                    [[maybe_unused]] bool FirstHVACIteration,
                                    Real64 &CurLoad,
                                    [[maybe_unused]] bool RunFlag)
    {
        this->initialize(state, CurLoad);
        this->calculate(state);
        this->update(state);
    }

    void WaterSourceSpecs::getDesignCapacities(
        [[maybe_unused]] EnergyPlusData &state, const EnergyPlus::PlantLocation &, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad)
    {

        MaxLoad = Constant::BigNumber;
        MinLoad = 0.0;
        OptLoad = Constant::BigNumber;
    }

    void WaterSourceSpecs::getSizingFactor(Real64 &_SizFac)
    {
        _SizFac = this->SizFac;
    }

    void WaterSourceSpecs::onInitLoopEquip(EnergyPlusData &state, const PlantLocation &)
    {
        Real64 myLoad = 0.0;
        this->initialize(state, myLoad);
        this->autosize(state);
    }
    void WaterSourceSpecs::oneTimeInit(EnergyPlusData &state)
    {
        static std::string const RoutineName("InitWaterSource");

        if (this->MyFlag) {
            // setup output variables once here
            this->setupOutputVars(state);
            // Locate the component on the plant loops for later usage
            bool errFlag = false;
            PlantUtilities::ScanPlantLoopsForObject(
                state, this->Name, DataPlant::PlantEquipmentType::WaterSource, this->plantLoc, errFlag, _, _, _, this->InletNodeNum, _);
            if (errFlag) {
                ShowFatalError(state, format("{}: Program terminated due to previous condition(s).", RoutineName));
            }
            this->MyFlag = false;
        }
    }

    void GetWaterSourceInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Edwin Lee
        //       DATE WRITTEN:    October 2012

        // PURPOSE OF THIS SUBROUTINE:
        // This routine gets the inputs and processes them into local data structures

        // METHODOLOGY EMPLOYED:
        // Standard E+ input processor interaction

        // REFERENCES:
        // WaterSource,
        //  A1 , \field Name
        //  A2 , \field Inlet Node
        //  A3 , \field Outlet Node
        //  N1 , \field Design Volume Flow Rate
        //  A4 , \field Temperature Specification Type
        //  N2 , \field Boundary Temperature
        //  A5 ; \field Source Temperature Schedule Name

        static constexpr std::string_view routineName = "GetWaterSourceInput";

        // LOCAL VARIABLES:
        int NumAlphas; // Number of elements in the alpha array
        int NumNums;   // Number of elements in the numeric array
        int IOStat;    // IO Status when calling get input subroutine
        bool ErrorsFound(false);
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;

        // GET NUMBER OF ALL EQUIPMENT TYPES
        cCurrentModuleObject = "PlantComponent:TemperatureSource";
        state.dataPlantCompTempSrc->NumSources = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataPlantCompTempSrc->NumSources <= 0) {
            ShowSevereError(state, format("No {} equipment specified in input file", cCurrentModuleObject));
            ErrorsFound = true;
        }

        // See if load distribution manager has already gotten the input
        if (allocated(state.dataPlantCompTempSrc->WaterSource)) {
            return; // probably not possible, and probably should throw error
        }
        state.dataPlantCompTempSrc->WaterSource.allocate(state.dataPlantCompTempSrc->NumSources);

        // fill arrays
        for (int SourceNum = 1; SourceNum <= state.dataPlantCompTempSrc->NumSources; ++SourceNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     SourceNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNums,
                                                                     IOStat,
                                                                     _,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

            Util::IsNameEmpty(state, state.dataIPShortCut->cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

            state.dataPlantCompTempSrc->WaterSource(SourceNum).Name = state.dataIPShortCut->cAlphaArgs(1);

            state.dataPlantCompTempSrc->WaterSource(SourceNum).InletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    state.dataIPShortCut->cAlphaArgs(2),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::PlantComponentTemperatureSource,
                                                    state.dataIPShortCut->cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);
            state.dataPlantCompTempSrc->WaterSource(SourceNum).OutletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    state.dataIPShortCut->cAlphaArgs(3),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::PlantComponentTemperatureSource,
                                                    state.dataIPShortCut->cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);
            BranchNodeConnections::TestCompSet(state,
                                               cCurrentModuleObject,
                                               state.dataIPShortCut->cAlphaArgs(1),
                                               state.dataIPShortCut->cAlphaArgs(2),
                                               state.dataIPShortCut->cAlphaArgs(3),
                                               "Chilled Water Nodes");

            state.dataPlantCompTempSrc->WaterSource(SourceNum).DesVolFlowRate = state.dataIPShortCut->rNumericArgs(1);
            if (state.dataPlantCompTempSrc->WaterSource(SourceNum).DesVolFlowRate == DataSizing::AutoSize) {
                state.dataPlantCompTempSrc->WaterSource(SourceNum).DesVolFlowRateWasAutoSized = true;
            }

            if (state.dataIPShortCut->cAlphaArgs(4) == "CONSTANT") {
                state.dataPlantCompTempSrc->WaterSource(SourceNum).tempSpecType = TempSpecType::Constant;
                state.dataPlantCompTempSrc->WaterSource(SourceNum).BoundaryTemp = state.dataIPShortCut->rNumericArgs(2);
            } else if (state.dataIPShortCut->cAlphaArgs(4) == "SCHEDULED") {
                state.dataPlantCompTempSrc->WaterSource(SourceNum).tempSpecType = TempSpecType::Schedule;
                if ((state.dataPlantCompTempSrc->WaterSource(SourceNum).tempSpecSched =
                         Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(5))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(5), state.dataIPShortCut->cAlphaArgs(5));
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(state, format("Input error for {}={}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format(R"(Invalid temperature specification type.  Expected either "Constant" or "Scheduled". Encountered {})",
                                         state.dataIPShortCut->cAlphaArgs(4)));
                ErrorsFound = true;
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("Errors found in processing input for {}", cCurrentModuleObject));
        }
    }

} // namespace PlantComponentTemperatureSources

} // namespace EnergyPlus
