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
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BoilerSteam.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace BoilerSteam {

    // Module containing the routines dealing with the Boilers

    // MODULE INFORMATION:
    //    AUTHOR         Rahul Chillar
    //    DATE WRITTEN   Dec 2004

    // PURPOSE OF THIS MODULE:
    // Performs steam boiler simulation for plant simulation

    BoilerSpecs *BoilerSpecs::factory(EnergyPlusData &state, std::string const &objectName)
    {
        // Process the input data for boilers if it hasn't been done already
        if (state.dataBoilerSteam->getSteamBoilerInput) {
            GetBoilerInput(state);
            state.dataBoilerSteam->getSteamBoilerInput = false;
        }

        // Now look for this particular pipe in the list
        auto myBoiler = std::find_if(state.dataBoilerSteam->Boiler.begin(),
                                     state.dataBoilerSteam->Boiler.end(),
                                     [&objectName](const BoilerSpecs &boiler) { return boiler.Name == objectName; });
        if (myBoiler != state.dataBoilerSteam->Boiler.end()) {
            return myBoiler;
        }

        // If we didn't find it, fatal
        ShowFatalError(state, std::format("LocalBoilerSteamFactory: Error getting inputs for steam boiler named: {}", objectName)); // LCOV_EXCL_LINE
        // Shut up the compiler
        return nullptr; // LCOV_EXCL_LINE
    }

    void BoilerSpecs::simulate(
        EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag)
    {
        this->initialize(state);
        auto &sim_component(DataPlant::CompData::getPlantComponent(state, this->plantLoc));
        this->calculate(state, CurLoad, RunFlag, sim_component.FlowCtrl);
        this->update(state, CurLoad, RunFlag, FirstHVACIteration);
    }

    void
    BoilerSpecs::getDesignCapacities([[maybe_unused]] EnergyPlusData &state, const PlantLocation &, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad)
    {
        MinLoad = this->NomCap * this->MinPartLoadRat;
        MaxLoad = this->NomCap * this->MaxPartLoadRat;
        OptLoad = this->NomCap * this->OptPartLoadRat;
    }

    void BoilerSpecs::getSizingFactor(Real64 &sizFac)
    {
        sizFac = this->SizFac;
    }

    void BoilerSpecs::onInitLoopEquip(EnergyPlusData &state, const PlantLocation &)
    {
        this->initialize(state);
        this->autosize(state);
    }

    void GetBoilerInput(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rahul Chillar
        //       DATE WRITTEN   Dec 2004

        // PURPOSE OF THIS SUBROUTINE:
        // Get all boiler data from input file

        // Locals
        static constexpr std::string_view RoutineName("GetBoilerInput: ");

        // LOCAL VARIABLES
        bool ErrorsFound(false);

        state.dataIPShortCut->cCurrentModuleObject = "Boiler:Steam";
        auto *inputProcessor = state.dataInputProcessing->inputProcessor.get();
        int numBoilers = inputProcessor->getNumObjectsFound(state, state.dataIPShortCut->cCurrentModuleObject);

        if (numBoilers <= 0) {
            ShowSevereError(state, std::format("No {} equipment specified in input file", state.dataIPShortCut->cCurrentModuleObject));
            ErrorsFound = true;
        }

        // See if load distribution manager has already gotten the input
        if (allocated(state.dataBoilerSteam->Boiler)) {
            return;
        }

        // Boiler will have fuel input to it , that is it !
        state.dataBoilerSteam->Boiler.allocate(numBoilers);
        auto const &boilerSchemaProps = inputProcessor->getObjectSchemaProps(state, state.dataIPShortCut->cCurrentModuleObject);
        auto const boilerObjects = inputProcessor->epJSON.find(state.dataIPShortCut->cCurrentModuleObject);

        if (boilerObjects != inputProcessor->epJSON.end()) {
            int BoilerNum = 1;
            for (auto const &boilerInstance : boilerObjects.value().items()) {
                auto const &boilerFields = boilerInstance.value();
                auto const boilerName = Util::makeUPPER(boilerInstance.key());
                auto const fuelType = inputProcessor->getAlphaFieldValue(boilerFields, boilerSchemaProps, "fuel_type");
                auto const waterInletNodeName = inputProcessor->getAlphaFieldValue(boilerFields, boilerSchemaProps, "water_inlet_node_name");
                auto const steamOutletNodeName = inputProcessor->getAlphaFieldValue(boilerFields, boilerSchemaProps, "steam_outlet_node_name");

                inputProcessor->markObjectAsUsed(state.dataIPShortCut->cCurrentModuleObject, boilerInstance.key());

                // ErrorsFound will be set to True if problem was found, left untouched otherwise
                GlobalNames::VerifyUniqueBoilerName(
                    state, state.dataIPShortCut->cCurrentModuleObject, boilerName, ErrorsFound, state.dataIPShortCut->cCurrentModuleObject + " Name");
                auto &thisBoiler = state.dataBoilerSteam->Boiler(BoilerNum);
                thisBoiler.Name = boilerName;

                // Validate fuel type input
                thisBoiler.FuelType = static_cast<Constant::eFuel>(getEnumValue(Constant::eFuelNamesUC, fuelType));

                // INPUTS from the IDF file
                thisBoiler.BoilerMaxOperPress = inputProcessor->getRealFieldValue(boilerFields, boilerSchemaProps, "maximum_operating_pressure");
                if (thisBoiler.BoilerMaxOperPress < 1e5) {
                    ShowWarningMessage(state, std::format("{}=\"{}\"", state.dataIPShortCut->cCurrentModuleObject, boilerName));
                    ShowContinueError(state, "Field: Maximum Operation Pressure units are Pa. Verify units.");
                }
                thisBoiler.NomEffic = inputProcessor->getRealFieldValue(boilerFields, boilerSchemaProps, "theoretical_efficiency");
                thisBoiler.TempUpLimitBoilerOut =
                    inputProcessor->getRealFieldValue(boilerFields, boilerSchemaProps, "design_outlet_steam_temperature");
                thisBoiler.NomCap = inputProcessor->getRealFieldValue(boilerFields, boilerSchemaProps, "nominal_capacity");
                if (thisBoiler.NomCap == DataSizing::AutoSize) {
                    thisBoiler.NomCapWasAutoSized = true;
                }
                thisBoiler.MinPartLoadRat = inputProcessor->getRealFieldValue(boilerFields, boilerSchemaProps, "minimum_part_load_ratio");
                thisBoiler.MaxPartLoadRat = inputProcessor->getRealFieldValue(boilerFields, boilerSchemaProps, "maximum_part_load_ratio");
                thisBoiler.OptPartLoadRat = inputProcessor->getRealFieldValue(boilerFields, boilerSchemaProps, "optimum_part_load_ratio");
                thisBoiler.FullLoadCoef[0] =
                    inputProcessor->getRealFieldValue(boilerFields, boilerSchemaProps, "coefficient_1_of_fuel_use_function_of_part_load_ratio_curve");
                thisBoiler.FullLoadCoef[1] =
                    inputProcessor->getRealFieldValue(boilerFields, boilerSchemaProps, "coefficient_2_of_fuel_use_function_of_part_load_ratio_curve");
                thisBoiler.FullLoadCoef[2] =
                    inputProcessor->getRealFieldValue(boilerFields, boilerSchemaProps, "coefficient_3_of_fuel_use_function_of_part_load_ratio_curve");
                thisBoiler.SizFac = inputProcessor->getRealFieldValue(boilerFields, boilerSchemaProps, "sizing_factor");
                if (thisBoiler.SizFac <= 0.0) {
                    thisBoiler.SizFac = 1.0;
                }

                if ((thisBoiler.FullLoadCoef[0] + thisBoiler.FullLoadCoef[1] + thisBoiler.FullLoadCoef[2]) == 0.0) {
                    ShowSevereError(state, std::format("{}{}=\"{}\",", RoutineName, state.dataIPShortCut->cCurrentModuleObject, boilerName));
                    ShowContinueError(state, " Sum of fuel use curve coefficients = 0.0");
                    ErrorsFound = true;
                }

                if (thisBoiler.MinPartLoadRat < 0.0) {
                    ShowSevereError(state, std::format("{}{}=\"{}\",", RoutineName, state.dataIPShortCut->cCurrentModuleObject, boilerName));
                    ShowContinueError(state, std::format("Invalid {}={:.3f}", "Minimum Part Load Ratio", thisBoiler.MinPartLoadRat));
                    ErrorsFound = true;
                }

                if (thisBoiler.TempUpLimitBoilerOut == 0.0) {
                    ShowSevereError(state, std::format("{}{}=\"{}\",", RoutineName, state.dataIPShortCut->cCurrentModuleObject, boilerName));
                    ShowContinueError(state, std::format("Invalid {}={:.3f}", "Design Outlet Steam Temperature", thisBoiler.TempUpLimitBoilerOut));
                    ErrorsFound = true;
                }
                thisBoiler.BoilerInletNodeNum = Node::GetOnlySingleNode(state,
                                                                        waterInletNodeName,
                                                                        ErrorsFound,
                                                                        Node::ConnectionObjectType::BoilerSteam,
                                                                        boilerName,
                                                                        Node::FluidType::Steam,
                                                                        Node::ConnectionType::Inlet,
                                                                        Node::CompFluidStream::Primary,
                                                                        Node::ObjectIsNotParent);
                thisBoiler.BoilerOutletNodeNum = Node::GetOnlySingleNode(state,
                                                                         steamOutletNodeName,
                                                                         ErrorsFound,
                                                                         Node::ConnectionObjectType::BoilerSteam,
                                                                         boilerName,
                                                                         Node::FluidType::Steam,
                                                                         Node::ConnectionType::Outlet,
                                                                         Node::CompFluidStream::Primary,
                                                                         Node::ObjectIsNotParent);
                Node::TestCompSet(
                    state, state.dataIPShortCut->cCurrentModuleObject, boilerName, waterInletNodeName, steamOutletNodeName, "Hot Steam Nodes");

                thisBoiler.fluid = Fluid::GetSteam(state);
                if (thisBoiler.fluid == nullptr && BoilerNum == 1) {
                    ShowSevereError(state, "Fluid Properties for STEAM not found.");
                    ErrorsFound = true;
                }

                if (boilerFields.find("end_use_subcategory") != boilerFields.end()) {
                    thisBoiler.EndUseSubcategory = inputProcessor->getAlphaFieldValue(boilerFields, boilerSchemaProps, "end_use_subcategory");
                } else {
                    thisBoiler.EndUseSubcategory = "General";
                }
                ++BoilerNum;
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, std::format("{}Errors found in processing {} input.", RoutineName, state.dataIPShortCut->cCurrentModuleObject));
        }
    }

    void BoilerSpecs::oneTimeInit(EnergyPlusData &state)
    {
        bool errFlag = false;
        PlantUtilities::ScanPlantLoopsForObject(
            state, this->Name, DataPlant::PlantEquipmentType::Boiler_Steam, this->plantLoc, errFlag, _, _, _, _, _);
        if (errFlag) {
            ShowFatalError(state, "InitBoiler: Program terminated due to previous condition(s).");
        }
    }

    void BoilerSpecs::initEachEnvironment(EnergyPlusData &state)
    {
        static constexpr std::string_view RoutineName("BoilerSpecs::initEachEnvironment");

        int BoilerInletNode = this->BoilerInletNodeNum;

        Real64 EnthSteamOutDry = this->fluid->getSatEnthalpy(state, this->TempUpLimitBoilerOut, 1.0, RoutineName);
        Real64 EnthSteamOutWet = this->fluid->getSatEnthalpy(state, this->TempUpLimitBoilerOut, 0.0, RoutineName);
        Real64 LatentEnthSteam = EnthSteamOutDry - EnthSteamOutWet;

        Real64 CpWater = this->fluid->getSatSpecificHeat(state, this->TempUpLimitBoilerOut, 0.0, RoutineName);

        this->DesMassFlowRate =
            this->NomCap / (LatentEnthSteam + CpWater * (this->TempUpLimitBoilerOut - state.dataLoopNodes->Node(BoilerInletNode).Temp));

        PlantUtilities::InitComponentNodes(state, 0.0, this->DesMassFlowRate, this->BoilerInletNodeNum, this->BoilerOutletNodeNum);

        this->BoilerPressCheck = 0.0;
        this->FuelUsed = 0.0;
        this->BoilerLoad = 0.0;
        this->BoilerEff = 0.0;
        this->BoilerOutletTemp = 0.0;

        if ((state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPoint == Node::SensedNodeFlagValue) &&
            (state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPointLo == Node::SensedNodeFlagValue)) {
            if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                if (!this->MissingSetPointErrDone) {
                    ShowWarningError(state, std::format("Missing temperature setpoint for Boiler:Steam = {}", this->Name));
                    ShowContinueError(state, " A temperature setpoint is needed at the outlet node of the boiler, use a SetpointManager");
                    ShowContinueError(state, " The overall loop setpoint will be assumed for this boiler. The simulation continues ...");
                    this->MissingSetPointErrDone = true;
                }
            } else {
                // need call to EMS to check node
                bool FatalError = false; // but not really fatal yet, but should be.
                EMSManager::CheckIfNodeSetPointManagedByEMS(state, this->BoilerOutletNodeNum, HVAC::CtrlVarType::Temp, FatalError);
                state.dataLoopNodes->NodeSetpointCheck(this->BoilerOutletNodeNum).needsSetpointChecking = false;
                if (FatalError) {
                    if (!this->MissingSetPointErrDone) {
                        ShowWarningError(state,
                                         std::format("Missing temperature setpoint for LeavingSetpointModulated mode Boiler named {}", this->Name));
                        ShowContinueError(state, " A temperature setpoint is needed at the outlet node of the boiler.");
                        ShowContinueError(state, " Use a Setpoint Manager to establish a setpoint at the boiler outlet node ");
                        ShowContinueError(state, " or use an EMS actuator to establish a setpoint at the boiler outlet node.");
                        ShowContinueError(state, " The overall loop setpoint will be assumed for this boiler. The simulation continues...");
                        this->MissingSetPointErrDone = true;
                    }
                }
            }
            this->UseLoopSetPoint = true; // this is for backward compatibility and could be removed
        }
    }

    void BoilerSpecs::initialize(EnergyPlusData &state) // number of the current electric chiller being simulated
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rahul Chillar
        //       DATE WRITTEN   Dec 2004
        //       RE-ENGINEERED  D. Shirey, rework for plant upgrade

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the Boiler components

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // Init more variables
        if (this->myFlag) {
            this->setupOutputVars(state);
            this->oneTimeInit(state);
            this->myFlag = false;
        }

        if (state.dataGlobal->BeginEnvrnFlag && this->myEnvrnFlag && (state.dataPlnt->PlantFirstSizesOkayToFinalize)) {
            this->initEachEnvironment(state);
            this->myEnvrnFlag = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->myEnvrnFlag = true;
        }

        if (this->UseLoopSetPoint) {
            //  At some point, need to circle back and get from plant data structure instead of node
            // fix for clumsy old input that worked because loop setpoint was spread.
            //  could be removed with transition, testing , model change, period of being obsolete.
            int BoilerOutletNode = this->BoilerOutletNodeNum;
            switch (this->plantLoc.loop->LoopDemandCalcScheme) {

            case DataPlant::LoopDemandCalcScheme::SingleSetPoint:
                state.dataLoopNodes->Node(BoilerOutletNode).TempSetPoint =
                    state.dataLoopNodes->Node(this->plantLoc.loop->TempSetPointNodeNum).TempSetPoint;
                break;
            case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand:
                state.dataLoopNodes->Node(BoilerOutletNode).TempSetPointLo =
                    state.dataLoopNodes->Node(this->plantLoc.loop->TempSetPointNodeNum).TempSetPointLo;
                break;
            default:
                break;
            }
        }
    }

    void BoilerSpecs::setupOutputVars(EnergyPlusData &state)
    {
        std::string_view sFuelType = Constant::eFuelNames[static_cast<int>(this->FuelType)];
        SetupOutputVariable(state,
                            "Boiler Heating Rate",
                            Constant::Units::W,
                            this->BoilerLoad,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Boiler Heating Energy",
                            Constant::Units::J,
                            this->BoilerEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name,
                            Constant::eResource::EnergyTransfer,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::Boilers);
        SetupOutputVariable(state,
                            std::format("Boiler {} Rate", sFuelType),
                            Constant::Units::W,
                            this->FuelUsed,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            std::format("Boiler {} Energy", sFuelType),
                            Constant::Units::J,
                            this->FuelConsumed,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name,
                            Constant::eFuel2eResource[(int)this->FuelType],
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::Heating,
                            this->EndUseSubcategory);
        SetupOutputVariable(state,
                            "Boiler Steam Efficiency",
                            Constant::Units::None,
                            this->BoilerEff,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Boiler Steam Inlet Temperature",
                            Constant::Units::C,
                            this->BoilerInletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Boiler Steam Outlet Temperature",
                            Constant::Units::C,
                            this->BoilerOutletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Boiler Steam Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->BoilerMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
    }

    void BoilerSpecs::autosize(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rahul Chillar
        //       DATE WRITTEN   Dec 2004
        //       MODIFIED       November 2013 Daeho Kang, add component sizing table entries

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing Boiler Components for which capacities and flow rates
        // have not been specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains Steam flow rate from the plant sizing array. Calculates nominal capacity from
        // the hot water flow rate and the hot water loop design delta T.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("SizeBoiler");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool ErrorsFound(false); // If errors detected in input
        Real64 tmpNomCap = this->NomCap;
        int PltSizNum = this->plantLoc.loop->PlantSizNum;

        if (PltSizNum > 0) {
            if (state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
                Real64 SizingTemp = this->TempUpLimitBoilerOut;
                Real64 SteamDensity = this->fluid->getSatDensity(state, SizingTemp, 1.0, RoutineName);
                Real64 EnthSteamOutDry = this->fluid->getSatEnthalpy(state, SizingTemp, 1.0, RoutineName);
                Real64 EnthSteamOutWet = this->fluid->getSatEnthalpy(state, SizingTemp, 0.0, RoutineName);
                Real64 LatentEnthSteam = EnthSteamOutDry - EnthSteamOutWet;
                Real64 CpWater = this->fluid->getSatSpecificHeat(state, SizingTemp, 0.0, RoutineName);
                tmpNomCap = (CpWater * SteamDensity * this->SizFac * state.dataSize->PlantSizData(PltSizNum).DeltaT *
                                 state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate +
                             state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate * SteamDensity * LatentEnthSteam);
            } else {
                if (this->NomCapWasAutoSized) {
                    tmpNomCap = 0.0;
                }
            }
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (this->NomCapWasAutoSized) {
                    this->NomCap = tmpNomCap;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state, "Boiler:Steam", this->Name, "Design Size Nominal Capacity [W]", tmpNomCap);

                        // Std 229 Boilers new report table
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerType, this->Name, "Boiler:Steam");
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerRefCap, this->Name, this->NomCap);
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerRefEff, this->Name, this->NomEffic);
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerRatedCap, this->Name, this->NomCap);
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerRatedEff, this->Name, this->NomEffic);
                        OutputReportPredefined::PreDefTableEntry(state,
                                                                 state.dataOutRptPredefined->pdchBoilerPlantloopName,
                                                                 this->Name,
                                                                 (this->plantLoc.loop != nullptr) ? this->plantLoc.loop->Name : "N/A");
                        OutputReportPredefined::PreDefTableEntry(state,
                                                                 state.dataOutRptPredefined->pdchBoilerPlantloopBranchName,
                                                                 this->Name,
                                                                 (this->plantLoc.branch != nullptr) ? this->plantLoc.branch->Name : "N/A");
                        OutputReportPredefined::PreDefTableEntry(
                            state, state.dataOutRptPredefined->pdchBoilerMinPLR, this->Name, this->MinPartLoadRat);
                        OutputReportPredefined::PreDefTableEntry(state,
                                                                 state.dataOutRptPredefined->pdchBoilerFuelType,
                                                                 this->Name,
                                                                 Constant::eFuelNames[static_cast<int>(this->FuelType)]);
                        OutputReportPredefined::PreDefTableEntry(
                            state, state.dataOutRptPredefined->pdchBoilerParaElecLoad, this->Name, "Not Applicable");
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state, "Boiler:Steam", this->Name, "Initial Design Size Nominal Capacity [W]", tmpNomCap);
                    }
                } else { // Hard-sized with sizing data
                    if (this->NomCap > 0.0 && tmpNomCap > 0.0) {
                        Real64 NomCapUser = this->NomCap;
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         "Boiler:Steam",
                                                         this->Name,
                                                         "Design Size Nominal Capacity [W]",
                                                         tmpNomCap,
                                                         "User-Specified Nominal Capacity [W]",
                                                         NomCapUser);

                            // Std 229 Boilers new report table
                            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerType, this->Name, "Boiler:Steam");
                            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerRefCap, this->Name, this->NomCap);
                            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerRefEff, this->Name, this->NomEffic);
                            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerRatedCap, this->Name, this->NomCap);
                            OutputReportPredefined::PreDefTableEntry(
                                state, state.dataOutRptPredefined->pdchBoilerRatedEff, this->Name, this->NomEffic);
                            OutputReportPredefined::PreDefTableEntry(state,
                                                                     state.dataOutRptPredefined->pdchBoilerPlantloopName,
                                                                     this->Name,
                                                                     (this->plantLoc.loop != nullptr) ? this->plantLoc.loop->Name : "N/A");
                            OutputReportPredefined::PreDefTableEntry(state,
                                                                     state.dataOutRptPredefined->pdchBoilerPlantloopBranchName,
                                                                     this->Name,
                                                                     (this->plantLoc.branch != nullptr) ? this->plantLoc.branch->Name : "N/A");
                            OutputReportPredefined::PreDefTableEntry(
                                state, state.dataOutRptPredefined->pdchBoilerMinPLR, this->Name, this->MinPartLoadRat);
                            OutputReportPredefined::PreDefTableEntry(state,
                                                                     state.dataOutRptPredefined->pdchBoilerFuelType,
                                                                     this->Name,
                                                                     Constant::eFuelNames[static_cast<int>(this->FuelType)]);
                            OutputReportPredefined::PreDefTableEntry(
                                state, state.dataOutRptPredefined->pdchBoilerParaElecLoad, this->Name, "Not Applicable");

                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(tmpNomCap - NomCapUser) / NomCapUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state, std::format("SizePump: Potential issue with equipment sizing for {}", this->Name));
                                    ShowContinueError(state, std::format("User-Specified Nominal Capacity of {:.2f} [W]", NomCapUser));
                                    ShowContinueError(state, std::format("differs from Design Size Nominal Capacity of {:.2f} [W]", tmpNomCap));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                }
            }
        } else {
            if (this->NomCapWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                ShowSevereError(state, "Autosizing of Boiler nominal capacity requires a loop Sizing:Plant object");
                ShowContinueError(state, std::format("Occurs in Boiler:Steam object={}", this->Name));
                ErrorsFound = true;
            }
            if (!this->NomCapWasAutoSized && this->NomCap > 0.0 && state.dataPlnt->PlantFinalSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state, "Boiler:Steam", this->Name, "User-Specified Nominal Capacity [W]", this->NomCap);
            }
        }

        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
            // create predefined report
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechType, this->Name, "Boiler:Steam");
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomEff, this->Name, this->NomEffic);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomCap, this->Name, this->NomCap);
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Preceding sizing errors cause program termination");
        }
    }

    void BoilerSpecs::calculate(EnergyPlusData &state,
                                Real64 &MyLoad,                                         // W - hot water demand to be met by boiler
                                bool const RunFlag,                                     // TRUE if boiler operating
                                DataBranchAirLoopPlant::ControlType const EquipFlowCtrl // Flow control mode for the equipment
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rahul Chillar
        //       DATE WRITTEN   Dec 2004

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates the boiler fuel consumption and the associated
        // hot water demand met by the boiler

        // METHODOLOGY EMPLOYED:
        // The model is based on a single combustion efficiency (=1 for electric)
        // and a second order polynomial fit of performance data to obtain part
        // load performance

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("CalcBoilerModel");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 BoilerDeltaTemp(0.0); // C - boiler inlet to outlet temperature difference
        Real64 CpWater;              // Heat capacity of condensed steam

        // Loading the variables derived type in to local variables
        this->BoilerLoad = 0.0;
        this->BoilerMassFlowRate = 0.0;

        switch (this->plantLoc.loop->LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
            this->BoilerOutletTemp = state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPoint;
        } break;
        case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
            this->BoilerOutletTemp = state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPointLo;
        } break;
        default:
            break;
        }
        // If the specified load is 0.0 or the boiler should not run then we leave this subroutine.Before leaving
        // if the component control is SERIESACTIVE we set the component flow to inlet flow so that flow resolver
        // will not shut down the branch
        if (MyLoad <= 0.0 || !RunFlag) {
            if (EquipFlowCtrl == DataBranchAirLoopPlant::ControlType::SeriesActive) {
                this->BoilerMassFlowRate = state.dataLoopNodes->Node(this->BoilerInletNodeNum).MassFlowRate;
            }
            return;
        }

        // Set the current load equal to the boiler load
        this->BoilerLoad = MyLoad;

        this->BoilerPressCheck = this->fluid->getSatPressure(state, this->BoilerOutletTemp, RoutineName);

        if ((this->BoilerPressCheck) > this->BoilerMaxOperPress) {
            if (this->PressErrIndex == 0) {
                ShowSevereError(state,
                                std::format("Boiler:Steam=\"{}\", Saturation Pressure is greater than Maximum Operating Pressure,", this->Name));
                ShowContinueError(state, "Lower Input Temperature");
                ShowContinueError(state, std::format("Steam temperature=[{:.2f}] C", this->BoilerOutletTemp));
                ShowContinueError(state, std::format("Refrigerant Saturation Pressure =[{:.0f}] Pa", this->BoilerPressCheck));
            }
            ShowRecurringSevereErrorAtEnd(state,
                                          "Boiler:Steam=\"" + this->Name +
                                              "\", Saturation Pressure is greater than Maximum Operating Pressure..continues",
                                          this->PressErrIndex,
                                          this->BoilerPressCheck,
                                          this->BoilerPressCheck,
                                          _,
                                          "[Pa]",
                                          "[Pa]");
        }

        CpWater = this->fluid->getSatSpecificHeat(state, state.dataLoopNodes->Node(this->BoilerInletNodeNum).Temp, 0.0, RoutineName);

        if (this->plantLoc.side->FlowLock == DataPlant::FlowLock::Unlocked) { // TODO: Components shouldn't check FlowLock
            // Calculate the flow for the boiler

            switch (this->plantLoc.loop->LoopDemandCalcScheme) {
            case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                BoilerDeltaTemp =
                    state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPoint - state.dataLoopNodes->Node(this->BoilerInletNodeNum).Temp;
            } break;
            default: { // DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand
                BoilerDeltaTemp =
                    state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPointLo - state.dataLoopNodes->Node(this->BoilerInletNodeNum).Temp;
            } break;
            }
            this->BoilerOutletTemp = BoilerDeltaTemp + state.dataLoopNodes->Node(this->BoilerInletNodeNum).Temp;

            Real64 const EnthSteamOutDry = this->fluid->getSatEnthalpy(state, this->BoilerOutletTemp, 1.0, RoutineName);
            Real64 const EnthSteamOutWet = this->fluid->getSatEnthalpy(state, this->BoilerOutletTemp, 0.0, RoutineName);
            Real64 const LatentEnthSteam = EnthSteamOutDry - EnthSteamOutWet;
            this->BoilerMassFlowRate = this->BoilerLoad / (LatentEnthSteam + (CpWater * BoilerDeltaTemp));

            PlantUtilities::SetComponentFlowRate(
                state, this->BoilerMassFlowRate, this->BoilerInletNodeNum, this->BoilerOutletNodeNum, this->plantLoc);

        } else { // If FlowLock is True
            // Set the boiler flow rate from inlet node and then check performance
            this->BoilerMassFlowRate = state.dataLoopNodes->Node(this->BoilerInletNodeNum).MassFlowRate;
            // Assume that it can meet the setpoint
            switch (this->plantLoc.loop->LoopDemandCalcScheme) {
            case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                BoilerDeltaTemp =
                    state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPoint - state.dataLoopNodes->Node(this->BoilerInletNodeNum).Temp;
            } break;
            case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
                BoilerDeltaTemp =
                    state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPointLo - state.dataLoopNodes->Node(this->BoilerInletNodeNum).Temp;
            } break;
            default:
                break;
            }
            // If boiler outlet temp is already greater than setpoint than it does not need to operate this iteration
            if (BoilerDeltaTemp < 0.0) {
                switch (this->plantLoc.loop->LoopDemandCalcScheme) {
                case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                    this->BoilerOutletTemp = state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPoint;
                } break;
                case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
                    this->BoilerOutletTemp = state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPointLo;
                } break;
                default:
                    break;
                }
                Real64 const EnthSteamOutDry = this->fluid->getSatEnthalpy(state, this->BoilerOutletTemp, 1.0, RoutineName);
                Real64 const EnthSteamOutWet = this->fluid->getSatEnthalpy(state, this->BoilerOutletTemp, 0.0, RoutineName);
                Real64 const LatentEnthSteam = EnthSteamOutDry - EnthSteamOutWet;
                this->BoilerLoad = (this->BoilerMassFlowRate * LatentEnthSteam);

            } else {
                switch (this->plantLoc.loop->LoopDemandCalcScheme) {
                case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                    this->BoilerOutletTemp = state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPoint;
                } break;
                case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
                    this->BoilerOutletTemp = state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPointLo;
                } break;
                default:
                    break;
                }

                Real64 const EnthSteamOutDry = this->fluid->getSatEnthalpy(state, this->BoilerOutletTemp, 1.0, RoutineName);
                Real64 const EnthSteamOutWet = this->fluid->getSatEnthalpy(state, this->BoilerOutletTemp, 0.0, RoutineName);
                Real64 const LatentEnthSteam = EnthSteamOutDry - EnthSteamOutWet;
                this->BoilerLoad =
                    std::abs(this->BoilerMassFlowRate * LatentEnthSteam) + std::abs(this->BoilerMassFlowRate * CpWater * BoilerDeltaTemp);
            }

            // If load exceeds the distributed load set to the distributed load
            if (this->BoilerLoad > MyLoad) {
                this->BoilerLoad = MyLoad;

                // Reset later , here just for calculating latent heat
                switch (this->plantLoc.loop->LoopDemandCalcScheme) {
                case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                    this->BoilerOutletTemp = state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPoint;
                } break;
                case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
                    this->BoilerOutletTemp = state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPointLo;
                } break;
                default:
                    break;
                }

                Real64 const EnthSteamOutDry = this->fluid->getSatEnthalpy(state, this->BoilerOutletTemp, 1.0, RoutineName);
                Real64 const EnthSteamOutWet = this->fluid->getSatEnthalpy(state, this->BoilerOutletTemp, 0.0, RoutineName);
                Real64 const LatentEnthSteam = EnthSteamOutDry - EnthSteamOutWet;
                BoilerDeltaTemp = this->BoilerOutletTemp - state.dataLoopNodes->Node(this->BoilerInletNodeNum).Temp;
                this->BoilerMassFlowRate = this->BoilerLoad / (LatentEnthSteam + CpWater * BoilerDeltaTemp);

                PlantUtilities::SetComponentFlowRate(
                    state, this->BoilerMassFlowRate, this->BoilerInletNodeNum, this->BoilerOutletNodeNum, this->plantLoc);
            }

            // Checks Boiler Load on the basis of the machine limits.
            if (this->BoilerLoad > this->NomCap) {
                if (this->BoilerMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
                    this->BoilerLoad = this->NomCap;

                    Real64 const EnthSteamOutDry = this->fluid->getSatEnthalpy(state, this->BoilerOutletTemp, 1.0, RoutineName);
                    Real64 const EnthSteamOutWet = this->fluid->getSatEnthalpy(state, this->BoilerOutletTemp, 0.0, RoutineName);
                    Real64 const LatentEnthSteam = EnthSteamOutDry - EnthSteamOutWet;
                    BoilerDeltaTemp = this->BoilerOutletTemp - state.dataLoopNodes->Node(this->BoilerInletNodeNum).Temp;
                    this->BoilerMassFlowRate = this->BoilerLoad / (LatentEnthSteam + CpWater * BoilerDeltaTemp);

                    PlantUtilities::SetComponentFlowRate(
                        state, this->BoilerMassFlowRate, this->BoilerInletNodeNum, this->BoilerOutletNodeNum, this->plantLoc);
                } else {
                    this->BoilerLoad = 0.0;
                    this->BoilerOutletTemp = state.dataLoopNodes->Node(this->BoilerInletNodeNum).Temp;
                }
            }

        } // End of the FlowLock If block

        // Limit BoilerOutletTemp.  If > max temp, trip boiler.
        if (this->BoilerOutletTemp > this->TempUpLimitBoilerOut) {
            this->BoilerLoad = 0.0;
            this->BoilerOutletTemp = state.dataLoopNodes->Node(this->BoilerInletNodeNum).Temp;
            //  Does BoilerMassFlowRate need to be set????
        }

        Real64 OperPLR = this->BoilerLoad / this->NomCap;
        OperPLR = min(OperPLR, this->MaxPartLoadRat);
        OperPLR = max(OperPLR, this->MinPartLoadRat);
        Real64 TheorFuelUse = this->BoilerLoad / this->NomEffic;

        // Calculate fuel used
        this->FuelUsed = TheorFuelUse / (this->FullLoadCoef[0] + this->FullLoadCoef[1] * OperPLR + this->FullLoadCoef[2] * pow_2(OperPLR));
        // Calculate boiler efficiency
        this->BoilerEff = this->BoilerLoad / this->FuelUsed;
    }

    // Beginning of Record Keeping subroutines for the BOILER:SIMPLE Module

    void BoilerSpecs::update(EnergyPlusData &state,
                             Real64 const MyLoad,                           // boiler operating load
                             bool const RunFlag,                            // boiler on when TRUE
                             [[maybe_unused]] bool const FirstHVACIteration // TRUE if First iteration of simulation
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rahul Chillar
        //       DATE WRITTEN   Dec 2004

        // PURPOSE OF THIS SUBROUTINE:
        // Boiler simulation reporting

        Real64 ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;
        int BoilerInletNode = this->BoilerInletNodeNum;
        int BoilerOutletNode = this->BoilerOutletNodeNum;

        if (MyLoad <= 0.0 || !RunFlag) {
            // set node temperatures
            PlantUtilities::SafeCopyPlantNode(state, BoilerInletNode, BoilerOutletNode);
            state.dataLoopNodes->Node(BoilerOutletNode).Temp = state.dataLoopNodes->Node(BoilerInletNode).Temp;
            this->BoilerOutletTemp = state.dataLoopNodes->Node(BoilerInletNode).Temp;
            this->BoilerLoad = 0.0;
            this->FuelUsed = 0.0;
            this->BoilerEff = 0.0;
            state.dataLoopNodes->Node(BoilerInletNode).Press = this->BoilerPressCheck;
            state.dataLoopNodes->Node(BoilerOutletNode).Press = state.dataLoopNodes->Node(BoilerInletNode).Press;
            state.dataLoopNodes->Node(BoilerInletNode).Quality = 0.0;
            state.dataLoopNodes->Node(BoilerOutletNode).Quality = state.dataLoopNodes->Node(BoilerInletNode).Quality;

        } else {
            // set node temperatures
            PlantUtilities::SafeCopyPlantNode(state, BoilerInletNode, BoilerOutletNode);
            state.dataLoopNodes->Node(BoilerOutletNode).Temp = this->BoilerOutletTemp;
            state.dataLoopNodes->Node(BoilerInletNode).Press = this->BoilerPressCheck; //???
            state.dataLoopNodes->Node(BoilerOutletNode).Press = state.dataLoopNodes->Node(BoilerInletNode).Press;
            state.dataLoopNodes->Node(BoilerOutletNode).Quality = 1.0; // Model assumes saturated steam exiting the boiler
        }

        this->BoilerInletTemp = state.dataLoopNodes->Node(BoilerInletNode).Temp;
        this->BoilerMassFlowRate = state.dataLoopNodes->Node(BoilerOutletNode).MassFlowRate;
        this->BoilerEnergy = this->BoilerLoad * ReportingConstant;
        this->FuelConsumed = this->FuelUsed * ReportingConstant;
    }

} // namespace BoilerSteam

} // namespace EnergyPlus
