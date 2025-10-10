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

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/Boilers.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::Boilers {

// Module containing the routines dealing with the Boilers

// MODULE INFORMATION:
//       AUTHOR         Dan Fisher, Taecheol Kim
//       DATE WRITTEN   1998, 2000

// PURPOSE OF THIS MODULE:
// Perform boiler simulation for plant simulation

// METHODOLOGY EMPLOYED:
// The BLAST/DOE-2 empirical model based on mfg. data

BoilerSpecs *BoilerSpecs::factory(EnergyPlusData &state, std::string const &objectName)
{
    // Process the input data for boilers if it hasn't been done already
    if (state.dataBoilers->getBoilerInputFlag) {
        GetBoilerInput(state);
        state.dataBoilers->getBoilerInputFlag = false;
    }
    // Now look for this particular boiler in the list
    auto myBoiler = std::find_if(state.dataBoilers->Boiler.begin(), state.dataBoilers->Boiler.end(), [&objectName](const BoilerSpecs &boiler) {
        return boiler.Name == objectName;
    });
    if (myBoiler != state.dataBoilers->Boiler.end()) {
        return myBoiler;
    }

    // If we didn't find it, fatal
    ShowFatalError(state, format("LocalBoilerFactory: Error getting inputs for boiler named: {}", objectName)); // LCOV_EXCL_LINE
    // Shut up the compiler
    return nullptr; // LCOV_EXCL_LINE
}

void BoilerSpecs::simulate(EnergyPlusData &state,
                           [[maybe_unused]] const PlantLocation &calledFromLocation,
                           [[maybe_unused]] bool const FirstHVACIteration,
                           Real64 &CurLoad,
                           bool const RunFlag)
{
    auto &sim_component(DataPlant::CompData::getPlantComponent(state, this->plantLoc));
    this->InitBoiler(state);
    this->CalcBoilerModel(state, CurLoad, RunFlag, sim_component.FlowCtrl);
    this->UpdateBoilerRecords(state, CurLoad, RunFlag);
}

void BoilerSpecs::getDesignCapacities([[maybe_unused]] EnergyPlusData &state,
                                      [[maybe_unused]] const PlantLocation &calledFromLocation,
                                      Real64 &MaxLoad,
                                      Real64 &MinLoad,
                                      Real64 &OptLoad)
{
    MinLoad = this->NomCap * this->MinPartLoadRat;
    MaxLoad = this->NomCap * this->MaxPartLoadRat;
    OptLoad = this->NomCap * this->OptPartLoadRat;
}

void BoilerSpecs::getSizingFactor(Real64 &SizFactor)
{
    SizFactor = this->SizFac;
}

void BoilerSpecs::onInitLoopEquip(EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation)
{
    this->InitBoiler(state);
    this->SizeBoiler(state);
}

void GetBoilerInput(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    April 1998
    //       MODIFIED:        R. Raustad - FSEC, June 2008: added boiler efficiency curve object

    // PURPOSE OF THIS SUBROUTINE:
    // get all boiler data from input file

    // METHODOLOGY EMPLOYED:
    // standard EnergyPlus input retrieval using input Processor

    // Locals
    static constexpr std::string_view RoutineName("GetBoilerInput: ");
    static constexpr std::string_view routineName = "GetBoilerInput";

    auto &s_ipsc = state.dataIPShortCut;

    // LOCAL VARIABLES
    bool ErrorsFound(false); // Flag to show errors were found during GetInput

    // GET NUMBER OF ALL EQUIPMENT
    s_ipsc->cCurrentModuleObject = "Boiler:HotWater";
    int numBoilers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    if (numBoilers <= 0) {
        ShowSevereError(state, format("No {} Equipment specified in input file", s_ipsc->cCurrentModuleObject));
        ErrorsFound = true;
    }

    // See if load distribution manager has already gotten the input
    if (allocated(state.dataBoilers->Boiler)) {
        return;
    }

    state.dataBoilers->Boiler.allocate(numBoilers);

    // LOAD ARRAYS WITH CURVE FIT Boiler DATA

    for (int BoilerNum = 1; BoilerNum <= numBoilers; ++BoilerNum) {
        int NumAlphas; // Number of elements in the alpha array
        int NumNums;   // Number of elements in the numeric array
        int IOStat;    // IO Status when calling get input subroutine
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 BoilerNum,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        // ErrorsFound will be set to True if problem was found, left untouched otherwise
        GlobalNames::VerifyUniqueBoilerName(
            state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), ErrorsFound, s_ipsc->cCurrentModuleObject + " Name");
        auto &thisBoiler = state.dataBoilers->Boiler(BoilerNum);
        thisBoiler.Name = s_ipsc->cAlphaArgs(1);
        thisBoiler.Type = DataPlant::PlantEquipmentType::Boiler_Simple;

        // Validate fuel type input
        thisBoiler.FuelType = static_cast<Constant::eFuel>(getEnumValue(Constant::eFuelNamesUC, s_ipsc->cAlphaArgs(2)));

        thisBoiler.NomCap = s_ipsc->rNumericArgs(1);
        if (s_ipsc->rNumericArgs(1) == 0.0) {
            ShowSevereError(state, fmt::format("{}{}=\"{}\",", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={:.2R}", s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1)));
            ShowContinueError(state, format("...{} must be greater than 0.0", s_ipsc->cNumericFieldNames(1)));
            ErrorsFound = true;
        }
        if (thisBoiler.NomCap == DataSizing::AutoSize) {
            thisBoiler.NomCapWasAutoSized = true;
        }

        thisBoiler.NomEffic = s_ipsc->rNumericArgs(2);
        if (s_ipsc->rNumericArgs(2) == 0.0) {
            ShowSevereError(state, fmt::format("{}{}=\"{}\",", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={:.3R}", s_ipsc->cNumericFieldNames(2), s_ipsc->rNumericArgs(2)));
            ShowContinueError(state, format("...{} must be greater than 0.0", s_ipsc->cNumericFieldNames(2)));
            ErrorsFound = true;
        } else if (s_ipsc->rNumericArgs(2) > 1.0) {
            ShowWarningError(state,
                             fmt::format("{} = {}: {}={} should not typically be greater than 1.",
                                         s_ipsc->cCurrentModuleObject,
                                         s_ipsc->cAlphaArgs(1),
                                         s_ipsc->cNumericFieldNames(2),
                                         s_ipsc->rNumericArgs(2)));
        }

        if (s_ipsc->cAlphaArgs(3) == "ENTERINGBOILER") {
            thisBoiler.CurveTempMode = TempMode::ENTERINGBOILERTEMP;
        } else if (s_ipsc->cAlphaArgs(3) == "LEAVINGBOILER") {
            thisBoiler.CurveTempMode = TempMode::LEAVINGBOILERTEMP;
        } else {
            thisBoiler.CurveTempMode = TempMode::NOTSET;
        }

        if (s_ipsc->lAlphaFieldBlanks(4)) {
            // Ok if this is empty?
        } else if ((thisBoiler.EfficiencyCurve = Curve::GetCurve(state, s_ipsc->cAlphaArgs(4))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
            ErrorsFound = true;
        } else if (thisBoiler.EfficiencyCurve->numDims != 1 && thisBoiler.EfficiencyCurve->numDims != 2) {
            Curve::ShowSevereCurveDims(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4), "1 or 2", thisBoiler.EfficiencyCurve->numDims);
            ErrorsFound = true;
        } else
            // if curve uses temperature, make sure water temp mode has been set
            if (thisBoiler.EfficiencyCurve->numDims == 2) {         // curve uses water temperature
                if (thisBoiler.CurveTempMode == TempMode::NOTSET) { // throw error
                    if (!s_ipsc->lAlphaFieldBlanks(3)) {
                        ShowSevereError(state, fmt::format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                        ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3)));
                        ShowContinueError(state,
                                          format("boilers.Boiler using curve type of {} must specify {}",
                                                 Curve::objectNames[(int)thisBoiler.EfficiencyCurve->curveType],
                                                 s_ipsc->cAlphaFieldNames(3)));
                        ShowContinueError(state, "Available choices are EnteringBoiler or LeavingBoiler");
                    } else {
                        ShowSevereError(state, fmt::format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                        ShowContinueError(state, format("Field {} is blank", s_ipsc->cAlphaFieldNames(3)));
                        ShowContinueError(state,
                                          format("boilers.Boiler using curve type of {} must specify either EnteringBoiler or LeavingBoiler",
                                                 Curve::objectNames[(int)thisBoiler.EfficiencyCurve->curveType]));
                    }
                    ErrorsFound = true;
                }
            }

        thisBoiler.VolFlowRate = s_ipsc->rNumericArgs(3);
        if (thisBoiler.VolFlowRate == DataSizing::AutoSize) {
            thisBoiler.VolFlowRateWasAutoSized = true;
        }
        thisBoiler.MinPartLoadRat = s_ipsc->rNumericArgs(4);
        thisBoiler.MaxPartLoadRat = s_ipsc->rNumericArgs(5);
        thisBoiler.OptPartLoadRat = s_ipsc->rNumericArgs(6);

        thisBoiler.TempUpLimitBoilerOut = s_ipsc->rNumericArgs(7);
        // default to 99.9C if upper temperature limit is left blank.
        if (thisBoiler.TempUpLimitBoilerOut <= 0.0) {
            thisBoiler.TempUpLimitBoilerOut = 99.9;
        }

        thisBoiler.ParasiticElecLoad = s_ipsc->rNumericArgs(8);
        thisBoiler.ParasiticFuelCapacity = s_ipsc->rNumericArgs(10);
        if (thisBoiler.FuelType == Constant::eFuel::Electricity && thisBoiler.ParasiticFuelCapacity > 0) {
            ShowWarningError(state, fmt::format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("{} should be zero when the fuel type is electricity.", s_ipsc->cNumericFieldNames(10)));
            ShowContinueError(state, "It will be ignored and the simulation continues.");
            thisBoiler.ParasiticFuelCapacity = 0.0;
        }

        thisBoiler.SizFac = s_ipsc->rNumericArgs(9);
        if (thisBoiler.SizFac == 0.0) {
            thisBoiler.SizFac = 1.0;
        }

        thisBoiler.BoilerInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                            s_ipsc->cAlphaArgs(5),
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::BoilerHotWater,
                                                                            s_ipsc->cAlphaArgs(1),
                                                                            DataLoopNode::NodeFluidType::Water,
                                                                            DataLoopNode::ConnectionType::Inlet,
                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                            DataLoopNode::ObjectIsNotParent);
        thisBoiler.BoilerOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                             s_ipsc->cAlphaArgs(6),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::BoilerHotWater,
                                                                             s_ipsc->cAlphaArgs(1),
                                                                             DataLoopNode::NodeFluidType::Water,
                                                                             DataLoopNode::ConnectionType::Outlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsNotParent);
        BranchNodeConnections::TestCompSet(
            state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(5), s_ipsc->cAlphaArgs(6), "Hot Water Nodes");

        if (s_ipsc->cAlphaArgs(7) == "CONSTANTFLOW") {
            thisBoiler.FlowMode = DataPlant::FlowMode::Constant;
        } else if (s_ipsc->cAlphaArgs(7) == "LEAVINGSETPOINTMODULATED") {
            thisBoiler.FlowMode = DataPlant::FlowMode::LeavingSetpointModulated;
        } else if (s_ipsc->cAlphaArgs(7) == "NOTMODULATED") {
            thisBoiler.FlowMode = DataPlant::FlowMode::NotModulated;
        } else {
            ShowSevereError(state, fmt::format("{}{}=\"{}\"", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(7), s_ipsc->cAlphaArgs(7)));
            ShowContinueError(state, "Available choices are ConstantFlow, NotModulated, or LeavingSetpointModulated");
            ShowContinueError(state, "Flow mode NotModulated is assumed and the simulation continues.");
            // We will assume variable flow if not specified
            thisBoiler.FlowMode = DataPlant::FlowMode::NotModulated;
        }

        if (NumAlphas > 7) {
            thisBoiler.EndUseSubcategory = s_ipsc->cAlphaArgs(8);
        } else {
            thisBoiler.EndUseSubcategory = "Boiler"; // leave this as "boiler" instead of "general" like other end use subcategories since
                                                     // it appears this way in existing output files.
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("{}{}", RoutineName, "Errors found in processing " + s_ipsc->cCurrentModuleObject + " input."));
    }
}

void BoilerSpecs::SetupOutputVars(EnergyPlusData &state)
{
    std::string_view const sFuelType = Constant::eFuelNames[static_cast<int>(this->FuelType)];
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
                        format("Boiler {} Rate", sFuelType),
                        Constant::Units::W,
                        this->FuelUsed,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        format("Boiler {} Energy", sFuelType),
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
                        "Boiler Inlet Temperature",
                        Constant::Units::C,
                        this->BoilerInletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Boiler Outlet Temperature",
                        Constant::Units::C,
                        this->BoilerOutletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Boiler Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->BoilerMassFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Boiler Ancillary Electricity Rate",
                        Constant::Units::W,
                        this->ParasiticElecPower,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Boiler Ancillary Electricity Energy",
                        Constant::Units::J,
                        this->ParasiticElecConsumption,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Heating,
                        "Boiler Parasitic");
    if (this->FuelType != Constant::eFuel::Electricity) {
        SetupOutputVariable(state,
                            format("Boiler Ancillary {} Rate", sFuelType),
                            Constant::Units::W,
                            this->ParasiticFuelRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            format("Boiler Ancillary {} Energy", sFuelType),
                            Constant::Units::J,
                            this->ParasiticFuelConsumption,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name,
                            Constant::eFuel2eResource[(int)this->FuelType],
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::Heating,
                            "Boiler Parasitic");
    }
    SetupOutputVariable(state,
                        "Boiler Part Load Ratio",
                        Constant::Units::None,
                        this->BoilerPLR,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Boiler Efficiency",
                        Constant::Units::None,
                        this->BoilerEff,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
        SetupEMSInternalVariable(state, "Boiler Nominal Capacity", this->Name, "[W]", this->NomCap);
    }
}

void BoilerSpecs::oneTimeInit(EnergyPlusData &state)
{
    // Locate the boilers on the plant loops for later usage
    bool errFlag = false;
    PlantUtilities::ScanPlantLoopsForObject(
        state, this->Name, DataPlant::PlantEquipmentType::Boiler_Simple, this->plantLoc, errFlag, _, this->TempUpLimitBoilerOut, _, _, _);
    if (errFlag) {
        ShowFatalError(state, "InitBoiler: Program terminated due to previous condition(s).");
    }

    if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) || (this->FlowMode == DataPlant::FlowMode::Constant)) {
        // reset flow priority
        DataPlant::CompData::getPlantComponent(state, this->plantLoc).FlowPriority = DataPlant::LoopFlowStatus::NeedyIfLoopOn;
    }
}

void BoilerSpecs::initEachEnvironment(EnergyPlusData &state)
{
    static constexpr std::string_view RoutineName("BoilerSpecs::initEachEnvironment");
    Real64 const rho = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
    this->DesMassFlowRate = this->VolFlowRate * rho;

    PlantUtilities::InitComponentNodes(state, 0.0, this->DesMassFlowRate, this->BoilerInletNodeNum, this->BoilerOutletNodeNum);

    if (this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) { // check if setpoint on outlet node
        if ((state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPoint == DataLoopNode::SensedNodeFlagValue) &&
            (state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPointLo == DataLoopNode::SensedNodeFlagValue)) {
            if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                if (!this->ModulatedFlowErrDone) {
                    ShowWarningError(state, format("Missing temperature setpoint for LeavingSetpointModulated mode Boiler named {}", this->Name));
                    ShowContinueError(
                        state, "  A temperature setpoint is needed at the outlet node of a boiler in variable flow mode, use a SetpointManager");
                    ShowContinueError(state, "  The overall loop setpoint will be assumed for Boiler. The simulation continues ... ");
                    this->ModulatedFlowErrDone = true;
                }
            } else {
                // need call to EMS to check node
                bool FatalError = false; // but not really fatal yet, but should be.
                EMSManager::CheckIfNodeSetPointManagedByEMS(state, this->BoilerOutletNodeNum, HVAC::CtrlVarType::Temp, FatalError);
                state.dataLoopNodes->NodeSetpointCheck(this->BoilerOutletNodeNum).needsSetpointChecking = false;
                if (FatalError) {
                    if (!this->ModulatedFlowErrDone) {
                        ShowWarningError(state, format("Missing temperature setpoint for LeavingSetpointModulated mode Boiler named {}", this->Name));
                        ShowContinueError(state, "  A temperature setpoint is needed at the outlet node of a boiler in variable flow mode");
                        ShowContinueError(state, "  use a Setpoint Manager to establish a setpoint at the boiler outlet node ");
                        ShowContinueError(state, "  or use an EMS actuator to establish a setpoint at the boiler outlet node ");
                        ShowContinueError(state, "  The overall loop setpoint will be assumed for Boiler. The simulation continues ... ");
                        this->ModulatedFlowErrDone = true;
                    }
                }
            }
            this->ModulatedFlowSetToLoop = true; // this is for backward compatibility and could be removed
        }
    }
}

void BoilerSpecs::InitBoiler(EnergyPlusData &state) // number of the current boiler being simulated
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   April 2002
    //       RE-ENGINEERED  Brent Griffith, rework for plant upgrade

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for initializations of the Boiler components

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger initializations.

    // Init more variables
    if (this->MyFlag) {
        this->SetupOutputVars(state);
        this->oneTimeInit(state);
        this->MyFlag = false;
    }

    if (this->MyEnvrnFlag && state.dataGlobal->BeginEnvrnFlag && (state.dataPlnt->PlantFirstSizesOkayToFinalize)) {
        this->initEachEnvironment(state);
        this->MyEnvrnFlag = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->MyEnvrnFlag = true;
    }

    // every iteration inits.  (most in calc routine)

    if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) && this->ModulatedFlowSetToLoop) {
        // fix for clumsy old input that worked because loop setpoint was spread.
        //  could be removed with transition, testing , model change, period of being obsolete.
        if (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopDemandCalcScheme == DataPlant::LoopDemandCalcScheme::SingleSetPoint) {
            state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPoint =
                state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->plantLoc.loopNum).TempSetPointNodeNum).TempSetPoint;
        } else { // DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand
            state.dataLoopNodes->Node(this->BoilerOutletNodeNum).TempSetPointLo =
                state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->plantLoc.loopNum).TempSetPointNodeNum).TempSetPointLo;
        }
    }
}

void BoilerSpecs::SizeBoiler(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   April 2002
    //       MODIFIED       November 2013 Daeho Kang, add component sizing table entries

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for sizing Boiler Components for which capacities and flow rates
    // have not been specified in the input.

    // METHODOLOGY EMPLOYED:
    // Obtains hot water flow rate from the plant sizing array. Calculates nominal capacity from
    // the hot water flow rate and the hot water loop design delta T.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("SizeBoiler");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool ErrorsFound(false); // If errors detected in input

    // grab some initial values for capacity and flow rate
    Real64 tmpNomCap = this->NomCap;                 // local nominal capacity cooling power
    Real64 tmpBoilerVolFlowRate = this->VolFlowRate; // local boiler design volume flow rate

    int const PltSizNum = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).PlantSizNum; // Plant Sizing index corresponding to CurLoopNum

    if (PltSizNum > 0) {
        if (state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {

            Real64 const rho = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
            Real64 const Cp = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, Constant::HWInitConvTemp, RoutineName);
            tmpNomCap =
                Cp * rho * this->SizFac * state.dataSize->PlantSizData(PltSizNum).DeltaT * state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate;
        } else {
            if (this->NomCapWasAutoSized) {
                tmpNomCap = 0.0;
            }
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->NomCapWasAutoSized) {
                this->NomCap = tmpNomCap;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state, "Boiler:HotWater", this->Name, "Design Size Nominal Capacity [W]", tmpNomCap);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state, "Boiler:HotWater", this->Name, "Initial Design Size Nominal Capacity [W]", tmpNomCap);
                }
            } else { // Hard-sized with sizing data
                if (this->NomCap > 0.0 && tmpNomCap > 0.0) {
                    Real64 const NomCapUser = this->NomCap; // Hardsized nominal capacity for reporting
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "Boiler:HotWater",
                                                     this->Name,
                                                     "Design Size Nominal Capacity [W]",
                                                     tmpNomCap,
                                                     "User-Specified Nominal Capacity [W]",
                                                     NomCapUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpNomCap - NomCapUser) / NomCapUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state, format("SizeBoilerHotWater: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(state, format("User-Specified Nominal Capacity of {:.2R} [W]", NomCapUser));
                                ShowContinueError(state, format("differs from Design Size Nominal Capacity of {:.2R} [W]", tmpNomCap));
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
            ShowContinueError(state, format("Occurs in Boiler object={}", this->Name));
            ErrorsFound = true;
        }
        if (!this->NomCapWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport && (this->NomCap > 0.0)) { // Hard-sized with no sizing data
            BaseSizer::reportSizerOutput(state, "Boiler:HotWater", this->Name, "User-Specified Nominal Capacity [W]", this->NomCap);
        }
    }

    if (PltSizNum > 0) {
        if (state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
            tmpBoilerVolFlowRate = state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate * this->SizFac;
        } else {
            if (this->VolFlowRateWasAutoSized) {
                tmpBoilerVolFlowRate = 0.0;
            }
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->VolFlowRateWasAutoSized) {
                this->VolFlowRate = tmpBoilerVolFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, "Boiler:HotWater", this->Name, "Design Size Design Water Flow Rate [m3/s]", tmpBoilerVolFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, "Boiler:HotWater", this->Name, "Initial Design Size Design Water Flow Rate [m3/s]", tmpBoilerVolFlowRate);
                }
            } else {
                if (this->VolFlowRate > 0.0 && tmpBoilerVolFlowRate > 0.0) {
                    Real64 VolFlowRateUser = this->VolFlowRate; // Hardsized volume flow for reporting
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "Boiler:HotWater",
                                                     this->Name,
                                                     "Design Size Design Water Flow Rate [m3/s]",
                                                     tmpBoilerVolFlowRate,
                                                     "User-Specified Design Water Flow Rate [m3/s]",
                                                     VolFlowRateUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpBoilerVolFlowRate - VolFlowRateUser) / VolFlowRateUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state, format("SizeBoilerHotWater: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(state, format("User-Specified Design Water Flow Rate of {:.2R} [m3/s]", VolFlowRateUser));
                                ShowContinueError(state,
                                                  format("differs from Design Size Design Water Flow Rate of {:.2R} [m3/s]", tmpBoilerVolFlowRate));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                    tmpBoilerVolFlowRate = VolFlowRateUser;
                }
            }
        }
    } else {
        if (this->VolFlowRateWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            ShowSevereError(state, "Autosizing of Boiler design flow rate requires a loop Sizing:Plant object");
            ShowContinueError(state, format("Occurs in Boiler object={}", this->Name));
            ErrorsFound = true;
        }
        if (!this->VolFlowRateWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport &&
            (this->VolFlowRate > 0.0)) { // Hard-sized with no sizing data
            BaseSizer::reportSizerOutput(state, "Boiler:HotWater", this->Name, "User-Specified Design Water Flow Rate [m3/s]", this->VolFlowRate);
        }
    }

    PlantUtilities::RegisterPlantCompDesignFlow(state, this->BoilerInletNodeNum, tmpBoilerVolFlowRate);

    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
        // create predefined report
        std::string const equipName = this->Name;
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechType, equipName, "Boiler:HotWater");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomEff, equipName, this->NomEffic);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomCap, equipName, this->NomCap);

        // Std 229 Boilers new report table
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerType, equipName, "Boiler:HotWater");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerRefCap, equipName, this->NomCap);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerRefEff, equipName, this->NomEffic);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerRatedCap, equipName, this->NomCap);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerRatedEff, equipName, this->NomEffic);
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchBoilerPlantloopName,
                                                 equipName,
                                                 this->plantLoc.loopNum > 0 ? state.dataPlnt->PlantLoop(this->plantLoc.loopNum).Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(
            state,
            state.dataOutRptPredefined->pdchBoilerPlantloopBranchName,
            equipName,
            this->plantLoc.loopNum > 0
                ? state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopSide(this->plantLoc.loopSideNum).Branch(this->plantLoc.branchNum).Name
                : "N/A");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerMinPLR, equipName, this->MinPartLoadRat);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchBoilerFuelType, equipName, Constant::eFuelNames[static_cast<int>(this->FuelType)]);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchBoilerParaElecLoad, equipName, this->ParasiticElecLoad);
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Preceding sizing errors cause program termination");
    }
}

void BoilerSpecs::CalcBoilerModel(EnergyPlusData &state,
                                  Real64 const MyLoad,                                    // W - hot water demand to be met by boiler
                                  bool const RunFlag,                                     // TRUE if boiler operating
                                  DataBranchAirLoopPlant::ControlType const EquipFlowCtrl // Flow control mode for the equipment
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   April 1999
    //       MODIFIED       Taecheol Kim,May 2000
    //                      Jun. 2008, R. Raustad, FSEC. Added boiler efficiency curve object
    //                      Aug. 2011, B. Griffith, NREL. Added switch for temperature to use in curve
    //                      Nov. 2016, R. Zhang, LBNL. Applied the boiler fouling fault model

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the boiler fuel consumption and the associated
    // hot water demand met by the boiler

    // METHODOLOGY EMPLOYED:
    // The model is based on a single combustion efficiency (=1 for electric)
    // and a second order polynomial fit of performance data to obtain part
    // load performance

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("CalcBoilerModel");

    // clean up some operating conditions, may not be necessary
    this->BoilerLoad = 0.0;
    this->ParasiticElecPower = 0.0;
    this->BoilerMassFlowRate = 0.0;

    int const BoilerInletNode = this->BoilerInletNodeNum;
    int const BoilerOutletNode = this->BoilerOutletNodeNum;
    Real64 BoilerNomCap = this->NomCap;                         // W - boiler nominal capacity
    Real64 const BoilerMaxPLR = this->MaxPartLoadRat;           // boiler maximum part load ratio
    Real64 const BoilerMinPLR = this->MinPartLoadRat;           // boiler minimum part load ratio
    Real64 BoilerNomEff = this->NomEffic;                       // boiler efficiency
    Real64 const TempUpLimitBout = this->TempUpLimitBoilerOut;  // C - boiler high temperature limit
    Real64 const BoilerMassFlowRateMax = this->DesMassFlowRate; // Max Design Boiler Mass Flow Rate converted from Volume Flow Rate

    Real64 Cp = state.dataPlnt->PlantLoop(this->plantLoc.loopNum)
                    .glycol->getSpecificHeat(state, state.dataLoopNodes->Node(BoilerInletNode).Temp, RoutineName);

    // If the specified load is 0.0 or the boiler should not run then we leave this subroutine. Before leaving
    // if the component control is SERIESACTIVE we set the component flow to inlet flow so that flow resolver
    // will not shut down the branch
    if (MyLoad <= 0.0 || !RunFlag) {
        if (EquipFlowCtrl == DataBranchAirLoopPlant::ControlType::SeriesActive) {
            this->BoilerMassFlowRate = state.dataLoopNodes->Node(BoilerInletNode).MassFlowRate;
        }
        return;
    }

    // If there is a fault of boiler fouling
    if (this->FaultyBoilerFoulingFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
        (!state.dataGlobal->KickOffSimulation)) {
        int FaultIndex = this->FaultyBoilerFoulingIndex;
        Real64 NomCap_ff = BoilerNomCap;
        Real64 BoilerNomEff_ff = BoilerNomEff;

        // calculate the Faulty Boiler Fouling Factor using fault information
        this->FaultyBoilerFoulingFactor = state.dataFaultsMgr->FaultsBoilerFouling(FaultIndex).CalFoulingFactor(state);

        // update the boiler nominal capacity at faulty cases
        BoilerNomCap = NomCap_ff * this->FaultyBoilerFoulingFactor;
        BoilerNomEff = BoilerNomEff_ff * this->FaultyBoilerFoulingFactor;
    }

    // Set the current load equal to the boiler load
    this->BoilerLoad = MyLoad;

    // Initialize the delta temperature to zero
    Real64 BoilerDeltaTemp; // C - boiler inlet to outlet temperature difference, set in all necessary code paths so no initialization required

    if (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopSide(this->plantLoc.loopSideNum).FlowLock == DataPlant::FlowLock::Unlocked) {
        // Either set the flow to the Constant value or calculate the flow for the variable volume
        if ((this->FlowMode == DataPlant::FlowMode::Constant) || (this->FlowMode == DataPlant::FlowMode::NotModulated)) {
            // Then find the flow rate and outlet temp
            this->BoilerMassFlowRate = BoilerMassFlowRateMax;
            PlantUtilities::SetComponentFlowRate(state, this->BoilerMassFlowRate, BoilerInletNode, BoilerOutletNode, this->plantLoc);

            if ((this->BoilerMassFlowRate != 0.0) && (MyLoad > 0.0)) {
                BoilerDeltaTemp = this->BoilerLoad / this->BoilerMassFlowRate / Cp;
            } else {
                BoilerDeltaTemp = 0.0;
            }
            this->BoilerOutletTemp = BoilerDeltaTemp + state.dataLoopNodes->Node(BoilerInletNode).Temp;

        } else if (this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) {
            // Calculate the Delta Temp from the inlet temp to the boiler outlet setpoint
            // Then find the flow rate and outlet temp

            if (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopDemandCalcScheme == DataPlant::LoopDemandCalcScheme::SingleSetPoint) {
                BoilerDeltaTemp = state.dataLoopNodes->Node(BoilerOutletNode).TempSetPoint - state.dataLoopNodes->Node(BoilerInletNode).Temp;
            } else { // DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand
                BoilerDeltaTemp = state.dataLoopNodes->Node(BoilerOutletNode).TempSetPointLo - state.dataLoopNodes->Node(BoilerInletNode).Temp;
            }

            this->BoilerOutletTemp = BoilerDeltaTemp + state.dataLoopNodes->Node(BoilerInletNode).Temp;

            if ((BoilerDeltaTemp > 0.0) && (this->BoilerLoad > 0.0)) {
                this->BoilerMassFlowRate = this->BoilerLoad / Cp / BoilerDeltaTemp;
                this->BoilerMassFlowRate = std::min(BoilerMassFlowRateMax, this->BoilerMassFlowRate);
            } else {
                this->BoilerMassFlowRate = 0.0;
            }
            PlantUtilities::SetComponentFlowRate(state, this->BoilerMassFlowRate, BoilerInletNode, BoilerOutletNode, this->plantLoc);

        } // End of Constant/Variable Flow If Block

    } else { // If FlowLock is True
        // Set the boiler flow rate from inlet node and then check performance
        this->BoilerMassFlowRate = state.dataLoopNodes->Node(BoilerInletNode).MassFlowRate;

        if ((MyLoad > 0.0) && (this->BoilerMassFlowRate > 0.0)) { // this boiler has a heat load
            this->BoilerLoad = MyLoad;
            if (this->BoilerLoad > BoilerNomCap * BoilerMaxPLR) {
                this->BoilerLoad = BoilerNomCap * BoilerMaxPLR;
            }
            if (this->BoilerLoad < BoilerNomCap * BoilerMinPLR) {
                this->BoilerLoad = BoilerNomCap * BoilerMinPLR;
            }
            this->BoilerOutletTemp = state.dataLoopNodes->Node(BoilerInletNode).Temp + this->BoilerLoad / (this->BoilerMassFlowRate * Cp);
        } else {
            this->BoilerLoad = 0.0;
            this->BoilerOutletTemp = state.dataLoopNodes->Node(BoilerInletNode).Temp;
        }
    }

    // Limit BoilerOutletTemp.  If > max temp, trip boiler off
    if (this->BoilerOutletTemp > TempUpLimitBout) {
        this->BoilerLoad = 0.0;
        this->BoilerOutletTemp = state.dataLoopNodes->Node(BoilerInletNode).Temp;
    }
    this->BoilerPLR = this->BoilerLoad / BoilerNomCap; // operating part load ratio
    this->BoilerPLR = std::min(this->BoilerPLR, BoilerMaxPLR);
    this->BoilerPLR = std::max(this->BoilerPLR, BoilerMinPLR);

    // calculate theoretical fuel use based on nominal thermal efficiency
    Real64 const TheorFuelUse = this->BoilerLoad / BoilerNomEff; // Theoretical (stoichiometric) fuel use
    Real64 EffCurveOutput = 1.0;                                 // Output of boiler efficiency curve

    // calculate normalized efficiency based on curve object type
    if (this->EfficiencyCurve != nullptr) {
        if (this->EfficiencyCurve->numDims == 2) {
            if (this->CurveTempMode == TempMode::ENTERINGBOILERTEMP) {
                EffCurveOutput = this->EfficiencyCurve->value(state, this->BoilerPLR, state.dataLoopNodes->Node(BoilerInletNode).Temp);
            } else if (this->CurveTempMode == TempMode::LEAVINGBOILERTEMP) {
                EffCurveOutput = this->EfficiencyCurve->value(state, this->BoilerPLR, this->BoilerOutletTemp);
            }
        } else {
            EffCurveOutput = this->EfficiencyCurve->value(state, this->BoilerPLR);
        }
    }
    BoilerEff = EffCurveOutput * BoilerNomEff;

    // warn if efficiency curve produces zero or negative results
    if (!state.dataGlobal->WarmupFlag && EffCurveOutput <= 0.0) {
        if (this->BoilerLoad > 0.0) {
            if (this->EffCurveOutputError < 1) {
                ++this->EffCurveOutputError;
                ShowWarningError(state, format("Boiler:HotWater \"{}\"", this->Name));
                ShowContinueError(state, "...Normalized Boiler Efficiency Curve output is less than or equal to 0.");
                ShowContinueError(state, format("...Curve input x value (PLR)     = {:.5T}", this->BoilerPLR));
                if (this->EfficiencyCurve->numDims == 2) {
                    if (this->CurveTempMode == TempMode::ENTERINGBOILERTEMP) {
                        ShowContinueError(state, format("...Curve input y value (Tinlet) = {:.2T}", state.dataLoopNodes->Node(BoilerInletNode).Temp));
                    } else if (this->CurveTempMode == TempMode::LEAVINGBOILERTEMP) {
                        ShowContinueError(state, format("...Curve input y value (Toutlet) = {:.2T}", this->BoilerOutletTemp));
                    }
                }
                ShowContinueError(state, format("...Curve output (normalized eff) = {:.5T}", EffCurveOutput));
                ShowContinueError(state,
                                  format("...Calculated Boiler efficiency  = {:.5T} (Boiler efficiency = Nominal Thermal Efficiency * Normalized "
                                         "Boiler Efficiency Curve output)",
                                         BoilerEff));
                ShowContinueErrorTimeStamp(state, "...Curve output reset to 0.01 and simulation continues.");
            } else {
                ShowRecurringWarningErrorAtEnd(state,
                                               "Boiler:HotWater \"" + this->Name +
                                                   "\": Boiler Efficiency Curve output is less than or equal to 0 warning continues...",
                                               this->EffCurveOutputIndex,
                                               EffCurveOutput,
                                               EffCurveOutput);
            }
        }
        EffCurveOutput = 0.01;
    }

    // warn if overall efficiency greater than 1.1
    if (!state.dataGlobal->WarmupFlag && BoilerEff > 1.1) {
        if (this->BoilerLoad > 0.0 && this->EfficiencyCurve != nullptr &&
            NomEffic <= 1.0) { // NomEffic > 1 warning occurs elsewhere; avoid cascading warnings
            if (this->CalculatedEffError < 1) {
                ++this->CalculatedEffError;
                ShowWarningError(state, format("Boiler:HotWater \"{}\"", this->Name));
                ShowContinueError(state, "...Calculated Boiler Efficiency is greater than 1.1.");
                ShowContinueError(state, "...Boiler Efficiency calculations shown below.");
                ShowContinueError(state, format("...Curve input x value (PLR)     = {:.5T}", this->BoilerPLR));
                if (this->EfficiencyCurve->numDims == 2) {
                    if (this->CurveTempMode == TempMode::ENTERINGBOILERTEMP) {
                        ShowContinueError(state, format("...Curve input y value (Tinlet) = {:.2T}", state.dataLoopNodes->Node(BoilerInletNode).Temp));
                    } else if (this->CurveTempMode == TempMode::LEAVINGBOILERTEMP) {
                        ShowContinueError(state, format("...Curve input y value (Toutlet) = {:.2T}", this->BoilerOutletTemp));
                    }
                }
                ShowContinueError(state, format("...Curve output (normalized eff) = {:.5T}", EffCurveOutput));
                ShowContinueError(state,
                                  format("...Calculated Boiler efficiency  = {:.5T} (Boiler efficiency = Nominal Thermal Efficiency * Normalized "
                                         "Boiler Efficiency Curve output)",
                                         BoilerEff));
                ShowContinueErrorTimeStamp(state, "...Curve output reset to 1.1 and simulation continues.");
            } else {
                ShowRecurringWarningErrorAtEnd(state,
                                               "Boiler:HotWater \"" + this->Name +
                                                   "\": Calculated Boiler Efficiency is greater than 1.1 warning continues...",
                                               this->CalculatedEffIndex,
                                               BoilerEff,
                                               BoilerEff);
            }
        }
        EffCurveOutput = 1.1;
    }

    // calculate fuel used based on normalized boiler efficiency curve (=1 when no curve used)
    this->FuelUsed = TheorFuelUse / EffCurveOutput;
    if (this->BoilerLoad > 0.0) {
        this->ParasiticElecPower = this->ParasiticElecLoad * this->BoilerPLR;
    }
    this->ParasiticFuelRate = this->ParasiticFuelCapacity * (1.0 - this->BoilerPLR);
}

void BoilerSpecs::UpdateBoilerRecords(EnergyPlusData &state,
                                      Real64 const MyLoad, // boiler operating load
                                      bool const RunFlag   // boiler on when TRUE
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    October 1998

    // PURPOSE OF THIS SUBROUTINE:
    // boiler simulation reporting

    Real64 const ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;
    int const BoilerInletNode = this->BoilerInletNodeNum;
    int const BoilerOutletNode = this->BoilerOutletNodeNum;

    if (MyLoad <= 0 || !RunFlag) {
        PlantUtilities::SafeCopyPlantNode(state, BoilerInletNode, BoilerOutletNode);
        state.dataLoopNodes->Node(BoilerOutletNode).Temp = state.dataLoopNodes->Node(BoilerInletNode).Temp;
        this->BoilerOutletTemp = state.dataLoopNodes->Node(BoilerInletNode).Temp;
        this->BoilerLoad = 0.0;
        this->FuelUsed = 0.0;
        this->ParasiticElecPower = 0.0;
        this->BoilerPLR = 0.0;
        this->BoilerEff = 0.0;
    } else {
        PlantUtilities::SafeCopyPlantNode(state, BoilerInletNode, BoilerOutletNode);
        state.dataLoopNodes->Node(BoilerOutletNode).Temp = this->BoilerOutletTemp;
    }

    this->BoilerInletTemp = state.dataLoopNodes->Node(BoilerInletNode).Temp;
    this->BoilerMassFlowRate = state.dataLoopNodes->Node(BoilerOutletNode).MassFlowRate;
    this->BoilerEnergy = this->BoilerLoad * ReportingConstant;
    this->FuelConsumed = this->FuelUsed * ReportingConstant;
    this->ParasiticElecConsumption = this->ParasiticElecPower * ReportingConstant;
    this->ParasiticFuelConsumption = this->ParasiticFuelRate * ReportingConstant;
}

} // namespace EnergyPlus::Boilers
