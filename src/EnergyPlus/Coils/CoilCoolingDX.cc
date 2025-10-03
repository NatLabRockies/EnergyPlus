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

#include <memory>

#include <ObjexxFCL/Array1D.hh> // needs to be in BranchNodeConnections.hh

#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/Coils/CoilCoolingDXAshrae205Performance.hh>
#include <EnergyPlus/Coils/CoilCoolingDXCurveFitPerformance.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/StandardRatings.hh>
#include <EnergyPlus/WaterManager.hh>

using namespace EnergyPlus;

std::shared_ptr<CoilCoolingDXPerformanceBase> CoilCoolingDX::makePerformanceSubclass(EnergyPlus::EnergyPlusData &state,
                                                                                     const std::string &performance_object_name)
{
    const auto a205_object_name = CoilCoolingDX205Performance::object_name;
    const auto curve_fit_object_name = CoilCoolingDXCurveFitPerformance::object_name;

    if (findPerformanceSubclass(state, a205_object_name, performance_object_name)) {
        return std::make_shared<CoilCoolingDX205Performance>(state, performance_object_name);
    } else if (findPerformanceSubclass(state, curve_fit_object_name, performance_object_name)) {
        return std::make_shared<CoilCoolingDXCurveFitPerformance>(state, performance_object_name);
    }

    ShowFatalError(state, format("Could not find Coil:Cooling:DX:Performance object with name: {}", performance_object_name));
    return nullptr;
}

int CoilCoolingDX::factory(EnergyPlus::EnergyPlusData &state, std::string const &coilName)
{
    if (state.dataCoilCoolingDX->coilCoolingDXGetInputFlag) {
        CoilCoolingDX::getInput(state);
        state.dataCoilCoolingDX->coilCoolingDXGetInputFlag = false;
    }
    int handle = -1;
    std::string coilNameUpper = Util::makeUPPER(coilName);
    for (auto const &thisCoil : state.dataCoilCoolingDX->coilCoolingDXs) {
        handle++;
        if (coilNameUpper == Util::makeUPPER(thisCoil.name)) {
            return handle;
        }
    }
    ShowSevereError(state, "Coil:Cooling:DX Coil not found=" + coilName);
    return -1;
}

void CoilCoolingDX::getInput(EnergyPlusData &state)
{
    int numCoolingCoilDXs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataCoilCoolingDX->coilCoolingDXObjectName);
    if (numCoolingCoilDXs <= 0) {
        ShowFatalError(state, R"(No "Coil:Cooling:DX" objects in input file)");
    }
    for (int coilNum = 1; coilNum <= numCoolingCoilDXs; ++coilNum) {
        int NumAlphas;  // Number of Alphas for each GetObjectItem call
        int NumNumbers; // Number of Numbers for each GetObjectItem call
        int IOStatus;
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 state.dataCoilCoolingDX->coilCoolingDXObjectName,
                                                                 coilNum,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNumbers,
                                                                 IOStatus);
        CoilCoolingDXInputSpecification input_specs;
        input_specs.name = state.dataIPShortCut->cAlphaArgs(1);
        input_specs.evaporator_inlet_node_name = state.dataIPShortCut->cAlphaArgs(2);
        input_specs.evaporator_outlet_node_name = state.dataIPShortCut->cAlphaArgs(3);
        input_specs.availability_schedule_name = state.dataIPShortCut->cAlphaArgs(4);
        input_specs.condenser_zone_name = state.dataIPShortCut->cAlphaArgs(5);
        input_specs.condenser_inlet_node_name = state.dataIPShortCut->cAlphaArgs(6);
        input_specs.condenser_outlet_node_name = state.dataIPShortCut->cAlphaArgs(7);
        input_specs.performance_object_name = state.dataIPShortCut->cAlphaArgs(8);
        input_specs.condensate_collection_water_storage_tank_name = state.dataIPShortCut->cAlphaArgs(9);
        input_specs.evaporative_condenser_supply_water_storage_tank_name = state.dataIPShortCut->cAlphaArgs(10);
        CoilCoolingDX thisCoil;
        thisCoil.instantiateFromInputSpec(state, input_specs);
        state.dataCoilCoolingDX->coilCoolingDXs.push_back(thisCoil);
    }
}

void CoilCoolingDX::instantiateFromInputSpec(EnergyPlusData &state, const CoilCoolingDXInputSpecification &input_data)
{
    static constexpr std::string_view routineName = "CoilCoolingDX::instantiateFromInputSpec";

    ErrorObjectHeader eoh{routineName, "CoilCoolingDX", input_data.name};

    this->original_input_specs = input_data;
    bool errorsFound = false;
    this->name = input_data.name;

    // initialize reclaim heat parameters
    this->reclaimHeat.Name = this->name;
    this->reclaimHeat.SourceType = state.dataCoilCoolingDX->coilCoolingDXObjectName;

    // other construction below
    this->evapInletNodeIndex = NodeInputManager::GetOnlySingleNode(state,
                                                                   input_data.evaporator_inlet_node_name,
                                                                   errorsFound,
                                                                   DataLoopNode::ConnectionObjectType::CoilCoolingDX,
                                                                   input_data.name,
                                                                   DataLoopNode::NodeFluidType::Air,
                                                                   DataLoopNode::ConnectionType::Inlet,
                                                                   NodeInputManager::CompFluidStream::Primary,
                                                                   DataLoopNode::ObjectIsNotParent);
    this->evapOutletNodeIndex = NodeInputManager::GetOnlySingleNode(state,
                                                                    input_data.evaporator_outlet_node_name,
                                                                    errorsFound,
                                                                    DataLoopNode::ConnectionObjectType::CoilCoolingDX,
                                                                    input_data.name,
                                                                    DataLoopNode::NodeFluidType::Air,
                                                                    DataLoopNode::ConnectionType::Outlet,
                                                                    NodeInputManager::CompFluidStream::Primary,
                                                                    DataLoopNode::ObjectIsNotParent);

    this->condInletNodeIndex = NodeInputManager::GetOnlySingleNode(state,
                                                                   input_data.condenser_inlet_node_name,
                                                                   errorsFound,
                                                                   DataLoopNode::ConnectionObjectType::CoilCoolingDX,
                                                                   input_data.name,
                                                                   DataLoopNode::NodeFluidType::Air,
                                                                   DataLoopNode::ConnectionType::Inlet,
                                                                   NodeInputManager::CompFluidStream::Secondary,
                                                                   DataLoopNode::ObjectIsNotParent);

    this->condOutletNodeIndex = NodeInputManager::GetOnlySingleNode(state,
                                                                    input_data.condenser_outlet_node_name,
                                                                    errorsFound,
                                                                    DataLoopNode::ConnectionObjectType::CoilCoolingDX,
                                                                    input_data.name,
                                                                    DataLoopNode::NodeFluidType::Air,
                                                                    DataLoopNode::ConnectionType::Outlet,
                                                                    NodeInputManager::CompFluidStream::Secondary,
                                                                    DataLoopNode::ObjectIsNotParent);

    this->performance = makePerformanceSubclass(state, input_data.performance_object_name);
    this->subcoolReheatFlag = this->performance->subcoolReheatFlag();

    if (!input_data.condensate_collection_water_storage_tank_name.empty()) {
        WaterManager::SetupTankSupplyComponent(state,
                                               this->name,
                                               state.dataCoilCoolingDX->coilCoolingDXObjectName,
                                               input_data.condensate_collection_water_storage_tank_name,
                                               errorsFound,
                                               this->condensateTankIndex,
                                               this->condensateTankSupplyARRID);
    }

    if (!input_data.evaporative_condenser_supply_water_storage_tank_name.empty()) {
        WaterManager::SetupTankDemandComponent(state,
                                               this->name,
                                               state.dataCoilCoolingDX->coilCoolingDXObjectName,
                                               input_data.evaporative_condenser_supply_water_storage_tank_name,
                                               errorsFound,
                                               this->evaporativeCondSupplyTankIndex,
                                               this->evaporativeCondSupplyTankARRID);
    }

    if (input_data.availability_schedule_name.empty()) {
        this->availSched = Sched::GetScheduleAlwaysOn(state);
    } else if ((this->availSched = Sched::GetSchedule(state, input_data.availability_schedule_name)) == nullptr) {
        ShowSevereItemNotFound(state, eoh, "Availability Schedule Name", input_data.availability_schedule_name);
        errorsFound = true;
    }

    if (!input_data.condenser_zone_name.empty()) {
        this->isSecondaryDXCoilInZone = true;
        // Setup zone data here
    }

    BranchNodeConnections::TestCompSet(state,
                                       state.dataCoilCoolingDX->coilCoolingDXObjectName,
                                       this->name,
                                       input_data.evaporator_inlet_node_name,
                                       input_data.evaporator_outlet_node_name,
                                       "Air Nodes");

    if (errorsFound) {
        ShowFatalError(state,
                       std::string{routineName} + "Errors found in getting " + state.dataCoilCoolingDX->coilCoolingDXObjectName +
                           " input. Preceding condition(s) causes termination.");
    }
}

void CoilCoolingDX::oneTimeInit(EnergyPlusData &state)
{

    // setup output variables, needs to be done after object is instantiated and emplaced
    SetupOutputVariable(state,
                        "Cooling Coil Total Cooling Rate",
                        Constant::Units::W,
                        this->totalCoolingEnergyRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Total Cooling Energy",
                        Constant::Units::J,
                        this->totalCoolingEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::HVAC,
                        OutputProcessor::EndUseCat::CoolingCoils);
    SetupOutputVariable(state,
                        "Cooling Coil Sensible Cooling Rate",
                        Constant::Units::W,
                        this->sensCoolingEnergyRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Sensible Cooling Energy",
                        Constant::Units::J,
                        this->sensCoolingEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Latent Cooling Rate",
                        Constant::Units::W,
                        this->latCoolingEnergyRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Latent Cooling Energy",
                        Constant::Units::J,
                        this->latCoolingEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Electricity Rate",
                        Constant::Units::W,
                        this->performance->powerUse,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Electricity Energy",
                        Constant::Units::J,
                        this->performance->electricityConsumption,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->name,
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::HVAC,
                        OutputProcessor::EndUseCat::Cooling);

    if (this->performance->compressorFuelType != Constant::eFuel::Electricity) {
        std::string_view const sFuelType = Constant::eFuelNames[static_cast<int>(this->performance->compressorFuelType)];
        SetupOutputVariable(state,
                            format("Cooling Coil {} Rate", sFuelType),
                            Constant::Units::W,
                            this->performance->compressorFuelRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->name);
        SetupOutputVariable(state,
                            format("Cooling Coil {} Energy", sFuelType),
                            Constant::Units::J,
                            this->performance->compressorFuelConsumption,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->name,
                            Constant::eFuel2eResource[(int)this->performance->compressorFuelType],
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Cooling);
    }

    SetupOutputVariable(state,
                        "Cooling Coil Runtime Fraction",
                        Constant::Units::None,
                        this->coolingCoilRuntimeFraction,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Crankcase Heater Electricity Rate",
                        Constant::Units::W,
                        this->performance->crankcaseHeaterPower,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Crankcase Heater Electricity Energy",
                        Constant::Units::J,
                        this->performance->crankcaseHeaterElectricityConsumption,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->name,
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::HVAC,
                        OutputProcessor::EndUseCat::Cooling);
    // Ported from variable speed coil
    SetupOutputVariable(state,
                        "Cooling Coil Air Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->airMassFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Air Inlet Temperature",
                        Constant::Units::C,
                        this->inletAirDryBulbTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Air Inlet Humidity Ratio",
                        Constant::Units::kgWater_kgDryAir,
                        this->inletAirHumRat,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Air Outlet Temperature",
                        Constant::Units::C,
                        this->outletAirDryBulbTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Air Outlet Humidity Ratio",
                        Constant::Units::kgWater_kgDryAir,
                        this->outletAirHumRat,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Part Load Ratio",
                        Constant::Units::None,
                        this->partLoadRatioReport,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Upper Speed Level",
                        Constant::Units::None,
                        this->speedNumReport,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Neighboring Speed Levels Ratio",
                        Constant::Units::None,
                        this->speedRatioReport,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Condenser Inlet Temperature",
                        Constant::Units::C,
                        this->condenserInletTemperature,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Dehumidification Mode",
                        Constant::Units::None,
                        this->dehumidificationMode,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Waste Heat Power",
                        Constant::Units::W,
                        this->wasteHeatEnergyRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Cooling Coil Waste Heat Energy",
                        Constant::Units::J,
                        this->wasteHeatEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->name);

    if (this->performance->evapCondBasinHeatCap > 0) {
        SetupOutputVariable(state,
                            "Cooling Coil Basin Heater Electricity Rate",
                            Constant::Units::W,
                            this->performance->basinHeaterPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->name);
        SetupOutputVariable(state,
                            "Cooling Coil Basin Heater Electricity Energy",
                            Constant::Units::J,
                            this->performance->basinHeaterElectricityConsumption,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->name,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Cooling);
    }
    if (this->condensateTankIndex > 0) {
        SetupOutputVariable(state,
                            "Cooling Coil Condensate Volume Flow Rate",
                            Constant::Units::m3_s,
                            this->condensateVolumeFlow,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->name);
        SetupOutputVariable(state,
                            "Cooling Coil Condensate Volume",
                            Constant::Units::m3,
                            this->condensateVolumeConsumption,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->name,
                            Constant::eResource::OnSiteWater,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Condensate);
    }
    if (this->evaporativeCondSupplyTankIndex > 0) {
        SetupOutputVariable(state,
                            "Cooling Coil Evaporative Condenser Pump Electricity Rate",
                            Constant::Units::W,
                            this->evapCondPumpElecPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->name);
        SetupOutputVariable(state,
                            "Cooling Coil Evaporative Condenser Pump Electricity Energy",
                            Constant::Units::J,
                            this->evapCondPumpElecConsumption,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->name,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Condensate);
        SetupOutputVariable(state,
                            "Cooling Coil Evaporative Condenser Water Volume Flow Rate",
                            Constant::Units::m3_s,
                            this->evaporativeCondSupplyTankVolumeFlow,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->name);
        SetupOutputVariable(state,
                            "Cooling Coil Evaporative Condenser Water Volume",
                            Constant::Units::m3,
                            this->evaporativeCondSupplyTankConsump,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->name,
                            Constant::eResource::Water,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Condensate);
        SetupOutputVariable(state,
                            "Cooling Coil Evaporative Condenser Mains Supply Water Volume",
                            Constant::Units::m3,
                            this->evaporativeCondSupplyTankConsump,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->name,
                            Constant::eResource::MainsWater,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Cooling);
    }
    if (this->subcoolReheatFlag) {
        SetupOutputVariable(state,
                            "SubcoolReheat Cooling Coil Operation Mode",
                            Constant::Units::None,
                            this->performance->OperatingMode,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->name);
        SetupOutputVariable(state,
                            "SubcoolReheat Cooling Coil Operation Mode Ratio",
                            Constant::Units::None,
                            this->performance->ModeRatio,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->name);
        SetupOutputVariable(state,
                            "SubcoolReheat Cooling Coil Recovered Heat Energy Rate",
                            Constant::Units::W,
                            this->recoveredHeatEnergyRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->name);
        SetupOutputVariable(state,
                            "SubcoolReheat Cooling Coil Recovered Heat Energy",
                            Constant::Units::J,
                            this->recoveredHeatEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->name,
                            Constant::eResource::EnergyTransfer,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::HeatRecovery);
    }

    if (this->isSecondaryDXCoilInZone) {
        SetupOutputVariable(state,
                            "Secondary Coil Heat Rejection Rate",
                            Constant::Units::W,
                            this->secCoilSensHeatRejEnergyRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->name);

        SetupOutputVariable(state,
                            "Secondary Coil Heat Rejection Energy",
                            Constant::Units::J,
                            this->secCoilSensHeatRejEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->name);
    }
}

int CoilCoolingDX::getNumModes()
{
    // TODO: should this support all 3 modes?
    int numModes = 1;
    if (this->performance->maxAvailCoilMode != HVAC::CoilMode::Normal) {
        numModes++;
    }
    return numModes;
}

int CoilCoolingDX::getOpModeCapFTIndex(HVAC::CoilMode const mode)
{
    // TODO: should this support all 3 modes?
    return this->performance->indexCapFT(mode);
}

void CoilCoolingDX::setData(int fanIndex, HVAC::FanType fanType, std::string const &fanName, int _airLoopNum)
{
    this->supplyFanIndex = fanIndex;
    this->supplyFanName = fanName;
    this->supplyFanType = fanType;
    this->airLoopNum = _airLoopNum;
}

void CoilCoolingDX::getFixedData(int &_evapInletNodeIndex,
                                 int &_evapOutletNodeIndex,
                                 int &_condInletNodeIndex,
                                 int &_normalModeNumSpeeds,
                                 CoilCoolingDXPerformanceBase::CapControlMethod &_capacityControlMethod,
                                 Real64 &_minOutdoorDryBulb)
{
    _evapInletNodeIndex = this->evapInletNodeIndex;
    _evapOutletNodeIndex = this->evapOutletNodeIndex;
    _condInletNodeIndex = this->condInletNodeIndex;
    _normalModeNumSpeeds = (int)this->performance->numSpeeds();
    _capacityControlMethod = this->performance->capControlMethod;
    _minOutdoorDryBulb = this->performance->minOutdoorDrybulb;
}

void CoilCoolingDX::getDataAfterSizing(EnergyPlusData &state,
                                       Real64 &_normalModeRatedEvapAirFlowRate,
                                       Real64 &_normalModeRatedCapacity,
                                       std::vector<Real64> &_normalModeFlowRates,
                                       std::vector<Real64> &_normalModeRatedCapacities)
{
    _normalModeRatedEvapAirFlowRate = this->performance->ratedEvapAirFlowRate(state);
    _normalModeFlowRates.clear();
    _normalModeRatedCapacities.clear();
    for (auto speed = 0; speed < this->performance->numSpeeds(); speed++) {
        _normalModeFlowRates.push_back(performance->evapAirFlowRateAtSpeedIndex(state, speed));
        _normalModeRatedCapacities.push_back(performance->ratedTotalCapacityAtSpeedIndex(state, speed));
    }
    _normalModeRatedCapacity = this->performance->ratedGrossTotalCap();
}

Real64 CoilCoolingDX::condMassFlowRate(EnergyPlusData &state, HVAC::CoilMode const mode)
{
    // TODO: should this support all 3 modes?
    return this->performance->ratedCondAirMassFlowRateNomSpeed(state, mode);
}

void CoilCoolingDX::size(EnergyPlusData &state)
{
    this->performance->parentName = this->name;
    this->performance->size(state);
}

void CoilCoolingDX::simulate(EnergyPlusData &state,
                             HVAC::CoilMode coilMode,
                             int const speedNum,
                             Real64 const speedRatio,
                             HVAC::FanOp const fanOp,
                             bool const singleMode,
                             Real64 const LoadSHR)
{
    if (this->myOneTimeInitFlag) {
        this->oneTimeInit(state);
        this->myOneTimeInitFlag = false;
    }

    static constexpr std::string_view RoutineName = "CoilCoolingDX::simulate";

    // get node references
    auto &evapInletNode = state.dataLoopNodes->Node(this->evapInletNodeIndex);
    auto &evapOutletNode = state.dataLoopNodes->Node(this->evapOutletNodeIndex);
    auto &condInletNode = state.dataLoopNodes->Node(this->condInletNodeIndex);
    auto &condOutletNode = state.dataLoopNodes->Node(this->condOutletNodeIndex);

    // set some reporting variables
    this->condenserInletTemperature = condInletNode.Temp;
    this->dehumidificationMode = coilMode;

    // set condenser inlet/outlet nodes
    // once condenser inlet is connected to upstream components, will need to revisit
    condInletNode.MassFlowRate = this->condMassFlowRate(state, coilMode);
    condOutletNode.MassFlowRate = condInletNode.MassFlowRate;

    // call the simulation, which returns useful data
    // TODO: check the avail schedule and reset data/pass through data as needed
    // TODO: check the minOATcompressor and reset data/pass through data as needed
    this->performance->OperatingMode = 0;
    this->performance->ModeRatio = 0.0;
    this->performance->simulate(
        state, evapInletNode, evapOutletNode, coilMode, speedNum, speedRatio, fanOp, condInletNode, condOutletNode, singleMode, LoadSHR);
    CoilCoolingDX::passThroughNodeData(evapInletNode, evapOutletNode);

    // calculate energy conversion factor
    Real64 reportingConstant = state.dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    // update condensate collection tank
    if (this->condensateTankIndex > 0) {
        if (speedNum > 0) {
            // calculate and report condensation rates  (how much water extracted from the air stream)
            // water flow of water in m3/s for water system interactions
            Real64 averageTemp = (evapInletNode.Temp + evapOutletNode.Temp) / 2.0;
            Real64 waterDensity = Psychrometrics::RhoH2O(averageTemp);
            Real64 inHumidityRatio = evapInletNode.HumRat;
            Real64 outHumidityRatio = evapOutletNode.HumRat;
            this->condensateVolumeFlow = max(0.0, (evapInletNode.MassFlowRate * (inHumidityRatio - outHumidityRatio) / waterDensity));
            this->condensateVolumeConsumption = this->condensateVolumeFlow * reportingConstant;
            state.dataWaterData->WaterStorage(this->condensateTankIndex).VdotAvailSupply(this->condensateTankSupplyARRID) =
                this->condensateVolumeFlow;
            state.dataWaterData->WaterStorage(this->condensateTankIndex).TwaterSupply(this->condensateTankSupplyARRID) = evapOutletNode.Temp;
        } else {
            state.dataWaterData->WaterStorage(this->condensateTankIndex).VdotAvailSupply(this->condensateTankSupplyARRID) = 0.0;
            state.dataWaterData->WaterStorage(this->condensateTankIndex).TwaterSupply(this->condensateTankSupplyARRID) = evapOutletNode.Temp;
        }
    }

    // update requests for evaporative condenser tank
    if (this->evaporativeCondSupplyTankIndex > 0) {
        if (speedNum > 0) {
            Real64 condInletTemp =
                state.dataEnvrn->OutWetBulbTemp + (state.dataEnvrn->OutDryBulbTemp - state.dataEnvrn->OutWetBulbTemp) *
                                                      (1.0 - this->performance->evapCondenserEffectivenessAtSpeedIndex(state, speedNum - 1));
            Real64 condInletHumRat =
                Psychrometrics::PsyWFnTdbTwbPb(state, condInletTemp, state.dataEnvrn->OutWetBulbTemp, state.dataEnvrn->OutBaroPress, RoutineName);
            Real64 outdoorHumRat = state.dataEnvrn->OutHumRat;

            Real64 condAirMassFlow = condInletNode.MassFlowRate;
            Real64 waterDensity = Psychrometrics::RhoH2O(state.dataEnvrn->OutDryBulbTemp);
            this->evaporativeCondSupplyTankVolumeFlow = (condInletHumRat - outdoorHumRat) * condAirMassFlow / waterDensity;
            this->evaporativeCondSupplyTankConsump = this->evaporativeCondSupplyTankVolumeFlow * reportingConstant;
            if (coilMode == HVAC::CoilMode::Normal) {
                this->evapCondPumpElecPower = this->performance->currentEvapCondPumpPowerAtSpeed(state, speedNum);
            }
            state.dataWaterData->WaterStorage(this->evaporativeCondSupplyTankIndex).VdotRequestDemand(this->evaporativeCondSupplyTankARRID) =
                this->evaporativeCondSupplyTankVolumeFlow;
        } else {
            state.dataWaterData->WaterStorage(this->evaporativeCondSupplyTankIndex).VdotRequestDemand(this->evaporativeCondSupplyTankARRID) = 0.0;
        }
    }

    // update report variables
    this->airMassFlowRate = evapOutletNode.MassFlowRate;
    this->inletAirDryBulbTemp = evapInletNode.Temp;
    this->inletAirHumRat = evapInletNode.HumRat;
    this->outletAirDryBulbTemp = evapOutletNode.Temp;
    this->outletAirHumRat = evapOutletNode.HumRat;

    CalcComponentSensibleLatentOutput(evapOutletNode.MassFlowRate,
                                      evapInletNode.Temp,
                                      evapInletNode.HumRat,
                                      evapOutletNode.Temp,
                                      evapOutletNode.HumRat,
                                      this->sensCoolingEnergyRate,
                                      this->latCoolingEnergyRate,
                                      this->totalCoolingEnergyRate);
    this->totalCoolingEnergy = this->totalCoolingEnergyRate * reportingConstant;
    this->sensCoolingEnergy = this->sensCoolingEnergyRate * reportingConstant;
    this->latCoolingEnergy = this->latCoolingEnergyRate * reportingConstant;

    this->evapCondPumpElecConsumption = this->evapCondPumpElecPower * reportingConstant;

    this->coolingCoilRuntimeFraction = this->performance->RTF;
    this->elecCoolingPower = this->performance->powerUse;
    this->elecCoolingConsumption = this->performance->powerUse * reportingConstant;
    this->wasteHeatEnergyRate = this->performance->wasteHeatRate;
    this->wasteHeatEnergy = this->performance->wasteHeatRate * reportingConstant;

    this->partLoadRatioReport = speedNum > 1 ? 1.0 : speedRatio;
    this->speedNumReport = speedNum;
    this->speedRatioReport = speedNum <= 1 ? 0.0 : speedRatio;

    if (coilMode == HVAC::CoilMode::SubcoolReheat) {
        this->recoveredHeatEnergyRate = this->performance->recoveredEnergyRate;
        this->recoveredHeatEnergy = this->recoveredHeatEnergyRate * reportingConstant;
    }

    if (this->isSecondaryDXCoilInZone) {
        // call CalcSecondaryDXCoils ???
        this->secCoilSensHeatRejEnergyRate = this->totalCoolingEnergyRate + this->elecCoolingPower;
        this->secCoilSensHeatRejEnergy = this->totalCoolingEnergy + this->elecCoolingConsumption;
    }

    // Fishy global things that need to be set here, try to set the AFN stuff now
    // This appears to be the only location where airLoopNum gets used
    // DataAirLoop::LoopDXCoilRTF = max(this->coolingCoilRuntimeFraction, DXCoil(DXCoilNum).HeatingCoilRuntimeFraction);
    state.dataAirLoop->LoopDXCoilRTF = this->coolingCoilRuntimeFraction;
    state.dataHVACGlobal->DXElecCoolingPower = this->elecCoolingPower;
    if (this->airLoopNum > 0) {
        state.dataAirLoop->AirLoopAFNInfo(this->airLoopNum).AFNLoopDXCoilRTF = this->coolingCoilRuntimeFraction;
        // The original calculation is below, but no heating yet
        //        max(DXCoil(DXCoilNum).CoolingCoilRuntimeFraction, DXCoil(DXCoilNum).HeatingCoilRuntimeFraction);
    }

    // report out to the coil sizing report if needed
    if (this->reportCoilFinalSizes) {
        if (!state.dataGlobal->WarmupFlag && !state.dataGlobal->DoingHVACSizingSimulations && !state.dataGlobal->DoingSizing) {

            // report out final coil sizing info
            Real64 ratedSensCap(0.0);
            ratedSensCap = this->performance->ratedGrossTotalCap() * this->performance->grossRatedSHR(state);
            state.dataRptCoilSelection->coilSelectionReportObj->setCoilFinalSizes(state,
                                                                                  this->name,
                                                                                  state.dataCoilCoolingDX->coilCoolingDXObjectName,
                                                                                  this->performance->ratedGrossTotalCap(),
                                                                                  ratedSensCap,
                                                                                  this->performance->ratedEvapAirFlowRate(state),
                                                                                  -999.0);

            // report out fan information
            // should work for all fan types
            if (this->supplyFanIndex > 0) {
                state.dataRptCoilSelection->coilSelectionReportObj->setCoilSupplyFanInfo(state,
                                                                                         this->name,
                                                                                         state.dataCoilCoolingDX->coilCoolingDXObjectName,
                                                                                         state.dataFans->fans(this->supplyFanIndex)->Name,
                                                                                         state.dataFans->fans(this->supplyFanIndex)->type,
                                                                                         this->supplyFanIndex);
            }

            // report out coil rating conditions, just create a set of dummy nodes and run calculate on them
            DataLoopNode::NodeData dummyEvapInlet;
            DataLoopNode::NodeData dummyEvapOutlet;
            DataLoopNode::NodeData dummyCondInlet;
            DataLoopNode::NodeData dummyCondOutlet;
            int dummySpeedNum = 1;
            Real64 dummySpeedRatio = 1.0;
            HVAC::FanOp dummyFanOp = HVAC::FanOp::Cycling;
            bool dummySingleMode = false;

            Real64 constexpr RatedInletAirTemp(26.6667);   // 26.6667C or 80F
            Real64 constexpr RatedInletWetBulbTemp(19.44); // 19.44 or 67F
            Real64 constexpr RatedOutdoorAirTemp(35.0);    // 35 C or 95F
            Real64 ratedOutdoorAirWetBulb = 23.9;          // from I/O ref. more precise value?

            Real64 ratedInletEvapMassFlowRate = this->performance->ratedEvapAirMassFlowRate(state);
            dummyEvapInlet.MassFlowRate = ratedInletEvapMassFlowRate;
            dummyEvapInlet.Temp = RatedInletAirTemp;
            Real64 dummyInletAirHumRat =
                Psychrometrics::PsyWFnTdbTwbPb(state, RatedInletAirTemp, RatedInletWetBulbTemp, DataEnvironment::StdPressureSeaLevel, RoutineName);
            dummyEvapInlet.Press = DataEnvironment::StdPressureSeaLevel;
            dummyEvapInlet.HumRat = dummyInletAirHumRat;
            dummyEvapInlet.Enthalpy = Psychrometrics::PsyHFnTdbW(RatedInletAirTemp, dummyInletAirHumRat);

            // maybe we don't actually need to override weather below, we'll see
            dummyCondInlet.Temp = RatedOutdoorAirTemp;
            dummyCondInlet.HumRat =
                Psychrometrics::PsyWFnTdbTwbPb(state, RatedOutdoorAirTemp, ratedOutdoorAirWetBulb, DataEnvironment::StdPressureSeaLevel, RoutineName);
            dummyCondInlet.OutAirWetBulb = ratedOutdoorAirWetBulb;
            dummyCondInlet.Press = condInletNode.Press; // for now; TODO: Investigate

            // overriding outdoor conditions temporarily
            Real64 holdOutDryBulbTemp = state.dataEnvrn->OutDryBulbTemp;
            Real64 holdOutHumRat = state.dataEnvrn->OutHumRat;
            Real64 holdOutWetBulb = state.dataEnvrn->OutWetBulbTemp;
            Real64 holdOutBaroPress = state.dataEnvrn->OutBaroPress;
            state.dataEnvrn->OutDryBulbTemp = RatedOutdoorAirTemp;
            state.dataEnvrn->OutWetBulbTemp = ratedOutdoorAirWetBulb;
            state.dataEnvrn->OutBaroPress = DataEnvironment::StdPressureSeaLevel; // assume rating is for sea level.
            state.dataEnvrn->OutHumRat =
                Psychrometrics::PsyWFnTdbTwbPb(state, RatedOutdoorAirTemp, ratedOutdoorAirWetBulb, DataEnvironment::StdPressureSeaLevel, RoutineName);

            this->performance->simulate(state,
                                        dummyEvapInlet,
                                        dummyEvapOutlet,
                                        HVAC::CoilMode::Normal,
                                        dummySpeedNum,
                                        dummySpeedRatio,
                                        dummyFanOp,
                                        dummyCondInlet,
                                        dummyCondOutlet,
                                        dummySingleMode);

            // reset outdoor conditions back to previous state
            state.dataEnvrn->OutDryBulbTemp = holdOutDryBulbTemp;
            state.dataEnvrn->OutWetBulbTemp = holdOutWetBulb;
            state.dataEnvrn->OutBaroPress = holdOutBaroPress;
            state.dataEnvrn->OutHumRat = holdOutHumRat;

            // Real64 const coolingRate = dummyEvapInlet.MassFlowRate * (dummyEvapInlet.Enthalpy - dummyEvapOutlet.Enthalpy);
            // Real64 const thisMinAirHumRat = min(dummyEvapInlet.HumRat, dummyEvapOutlet.HumRat);
            // Real64 const sensCoolingRate = dummyEvapInlet.MassFlowRate * (Psychrometrics::PsyHFnTdbW(dummyEvapInlet.Temp, thisMinAirHumRat) -
            //                                                              Psychrometrics::PsyHFnTdbW(dummyEvapOutlet.Temp, thisMinAirHumRat));
            Real64 coolingRate = 0.0;
            Real64 sensCoolingRate = 0.0;
            Real64 latCoolingRate = 0.0;
            CalcComponentSensibleLatentOutput(dummyEvapInlet.MassFlowRate,
                                              dummyEvapInlet.Temp,
                                              dummyEvapInlet.HumRat,
                                              dummyEvapOutlet.Temp,
                                              dummyEvapOutlet.HumRat,
                                              sensCoolingRate,
                                              latCoolingRate,
                                              coolingRate);

            Real64 const ratedOutletWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                state, dummyEvapOutlet.Temp, dummyEvapOutlet.HumRat, DataEnvironment::StdPressureSeaLevel, "Coil:Cooling:DX::simulate");
            state.dataRptCoilSelection->coilSelectionReportObj->setRatedCoilConditions(state,
                                                                                       this->name,
                                                                                       state.dataCoilCoolingDX->coilCoolingDXObjectName,
                                                                                       coolingRate,
                                                                                       sensCoolingRate,
                                                                                       ratedInletEvapMassFlowRate,
                                                                                       RatedInletAirTemp,
                                                                                       dummyInletAirHumRat,
                                                                                       RatedInletWetBulbTemp,
                                                                                       dummyEvapOutlet.Temp,
                                                                                       dummyEvapOutlet.HumRat,
                                                                                       ratedOutletWetBulb,
                                                                                       RatedOutdoorAirTemp,
                                                                                       ratedOutdoorAirWetBulb,
                                                                                       this->performance->ratedCBF(state),
                                                                                       -999.0);

            this->reportCoilFinalSizes = false;
        }
    }

    // update available reclaim heat
    this->reclaimHeat.AvailCapacity = this->totalCoolingEnergyRate + this->elecCoolingPower;
}

void CoilCoolingDX::setToHundredPercentDOAS()
{
    performance->setToHundredPercentDOAS();
}

void CoilCoolingDX::passThroughNodeData(DataLoopNode::NodeData &in, DataLoopNode::NodeData &out)
{
    // pass through all the other node variables that we don't update as a part of this model calculation
    out.MassFlowRate = in.MassFlowRate;
    out.Press = in.Press;
    out.Quality = in.Quality;
    out.MassFlowRateMax = in.MassFlowRateMax;
    out.MassFlowRateMin = in.MassFlowRateMin;
    out.MassFlowRateMaxAvail = in.MassFlowRateMaxAvail;
    out.MassFlowRateMinAvail = in.MassFlowRateMinAvail;
}

void PopulateCoolingCoilStandardRatingInformation(InputOutputFile &eio,
                                                  std::string coilName,
                                                  Real64 &capacity,
                                                  Real64 &eer,
                                                  Real64 &seer_User,
                                                  Real64 &seer_Standard,
                                                  Real64 &ieer,
                                                  bool const AHRI2023StandardRatings)
{
    Real64 constexpr ConvFromSIToIP(3.412141633);
    // TODO: TOO BIG |Capacity from 135K (39565 W) to 250K Btu/hr (73268 W) - calculated as per AHRI Standard 365-2009 -
    // Ratings not yet supported in EnergyPlus
    // Define the format string based on the condition
    std::string_view Format_991;
    if (!AHRI2023StandardRatings) {
        Format_991 = " DX Cooling Coil Standard Rating Information, {}, {}, {:.1f}, {:.2f}, {:.2f}, {:.2f}, {:.2f}, {:.1f}\n";
    } else {
        Format_991 = " DX Cooling Coil AHRI 2023 Standard Rating Information, {}, {}, {:.1f}, {:.2f}, {:.2f}, {:.2f}, {:.2f}, {:.1f}\n";
    }
    print(eio,
          Format_991,
          "Coil:Cooling:DX",
          coilName,
          capacity,
          eer,
          eer * ConvFromSIToIP,
          seer_User * ConvFromSIToIP,
          seer_Standard * ConvFromSIToIP, // SEER | Capacity less than 65K Btu/h (19050 W) - calculated as per AHRI Standard 210/240-2023.
          ieer * ConvFromSIToIP); // IEER | Capacity of 65K Btu/h (19050 W) to less than 135K Btu/h (39565 W) - calculated as per AHRI Standard
                                  // 340/360-2022.
}

void CoilCoolingDX::reportAllStandardRatings(EnergyPlusData &state)
{
    if (!state.dataCoilCoolingDX->coilCoolingDXs.empty()) {
        Real64 constexpr ConvFromSIToIP(3.412141633); // Conversion from SI to IP [3.412 Btu/hr-W]
        if (state.dataHVACGlobal->StandardRatingsMyCoolOneTimeFlag) {
            static constexpr std::string_view Format_994(
                "! <DX Cooling Coil Standard Rating Information>, Component Type, Component Name, Standard Rating (Net) "
                "Cooling Capacity {W}, Standard Rating Net COP {W/W}, EER {Btu/W-h}, SEER User {Btu/W-h}, SEER Standard {Btu/W-h}, "
                "IEER "
                "{Btu/W-h}");
            print(state.files.eio, "{}\n", Format_994);
            state.dataHVACGlobal->StandardRatingsMyCoolOneTimeFlag = false;
        }
        for (auto &coil : state.dataCoilCoolingDX->coilCoolingDXs) {
            coil.performance->calcStandardRatings210240(state);
            PopulateCoolingCoilStandardRatingInformation(state.files.eio,
                                                         coil.name,
                                                         coil.performance->standardRatingCoolingCapacity,
                                                         coil.performance->standardRatingEER,
                                                         coil.performance->standardRatingSEER,
                                                         coil.performance->standardRatingSEER_Standard,
                                                         coil.performance->standardRatingIEER,
                                                         false);

            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilType, coil.name, "Coil:Cooling:DX");
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchDXCoolCoilNetCapSI, coil.name, coil.performance->standardRatingCoolingCapacity, 1);
            // W/W is the same as Btuh/Btuh so that's fine too
            if (coil.performance->standardRatingEER > 0.0) {
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchDXCoolCoilCOP, coil.name, coil.performance->standardRatingEER, 2);
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilCOP, coil.name, "N/A");
            }
            // Btu/W-h will convert to itself
            if (coil.performance->standardRatingEER > 0.0) {
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchDXCoolCoilEERIP, coil.name, coil.performance->standardRatingEER * ConvFromSIToIP, 2);
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilEERIP, coil.name, "N/A");
            }
            if (coil.performance->standardRatingSEER > 0.0) {
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchDXCoolCoilSEERUserIP, coil.name, coil.performance->standardRatingSEER * ConvFromSIToIP, 2);
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEERUserIP, coil.name, "N/A");
            }
            if (coil.performance->standardRatingSEER_Standard > 0.0) {
                OutputReportPredefined::PreDefTableEntry(state,
                                                         state.dataOutRptPredefined->pdchDXCoolCoilSEERStandardIP,
                                                         coil.name,
                                                         coil.performance->standardRatingSEER_Standard * ConvFromSIToIP,
                                                         2);
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEERStandardIP, coil.name, "N/A");
            }
            if (coil.performance->standardRatingIEER > 0.0) {
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchDXCoolCoilIEERIP, coil.name, coil.performance->standardRatingIEER * ConvFromSIToIP, 1);
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilIEERIP, coil.name, "N/A");
            }
            OutputReportPredefined::addFootNoteSubTable(state, state.dataOutRptPredefined->pdstDXCoolCoil, StandardRatings::AHRI2017FOOTNOTE);

            // AHRI 2023 Standard SEER2 Calculations
            if (state.dataHVACGlobal->StandardRatingsMyCoolOneTimeFlag2) {
                static constexpr std::string_view Format_991_(
                    "! <DX Cooling Coil AHRI 2023 Standard Rating Information>, Component Type, Component Name, Standard Rating (Net) "
                    "Cooling Capacity {W}, Standard Rating Net COP2 {W/W}, EER2 {Btu/W-h}, SEER2 User {Btu/W-h}, SEER2 Standard "
                    "{Btu/W-h}, "
                    "IEER 2022 "
                    "{Btu/W-h}");
                print(state.files.eio, "{}\n", Format_991_);
                state.dataHVACGlobal->StandardRatingsMyCoolOneTimeFlag2 = false;
            }
            PopulateCoolingCoilStandardRatingInformation(state.files.eio,
                                                         coil.name,
                                                         coil.performance->standardRatingCoolingCapacity2023,
                                                         coil.performance->standardRatingEER2,
                                                         coil.performance->standardRatingSEER2_User,
                                                         coil.performance->standardRatingSEER2_Standard,
                                                         coil.performance->standardRatingIEER2,
                                                         true);

            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilType_2023, coil.name, "Coil:Cooling:DX");
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchDXCoolCoilNetCapSI_2023, coil.name, coil.performance->standardRatingCoolingCapacity2023, 1);
            // W/W is the same as Btuh/Btuh so that's fine too
            if (coil.performance->standardRatingEER2 > 0.0) {
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchDXCoolCoilCOP_2023, coil.name, coil.performance->standardRatingEER2, 2);
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilCOP_2023, coil.name, "N/A");
            }
            // Btu/W-h will convert to itself
            if (coil.performance->standardRatingEER2 > 0.0) {
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchDXCoolCoilEERIP_2023, coil.name, coil.performance->standardRatingEER2 * ConvFromSIToIP, 2);
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilEERIP_2023, coil.name, "N/A");
            }
            if (coil.performance->standardRatingSEER2_User > 0.0) {
                OutputReportPredefined::PreDefTableEntry(state,
                                                         state.dataOutRptPredefined->pdchDXCoolCoilSEER2UserIP_2023,
                                                         coil.name,
                                                         coil.performance->standardRatingSEER2_User * ConvFromSIToIP,
                                                         2);
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEER2UserIP_2023, coil.name, "N/A");
            }
            if (coil.performance->standardRatingSEER2_Standard > 0.0) {
                OutputReportPredefined::PreDefTableEntry(state,
                                                         state.dataOutRptPredefined->pdchDXCoolCoilSEER2StandardIP_2023,
                                                         coil.name,
                                                         coil.performance->standardRatingSEER2_Standard * ConvFromSIToIP,
                                                         2);
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEER2StandardIP_2023, coil.name, "N/A");
            }
            if (coil.performance->standardRatingIEER2 > 0.0) {
                OutputReportPredefined::PreDefTableEntry(state,
                                                         state.dataOutRptPredefined->pdchDXCoolCoilIEERIP_2023,
                                                         coil.name,
                                                         coil.performance->standardRatingIEER2 * ConvFromSIToIP,
                                                         1);
            } else {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilIEERIP_2023, coil.name, "N/A");
            }
            OutputReportPredefined::addFootNoteSubTable(state, state.dataOutRptPredefined->pdstDXCoolCoil_2023, StandardRatings::AHRI2023FOOTNOTE);
        }
    }
    state.dataCoilCoolingDX->stillNeedToReportStandardRatings = false;
}

bool CoilCoolingDX::findPerformanceSubclass(EnergyPlus::EnergyPlusData &state,
                                            const std::string_view object_to_find,
                                            const std::string &idd_performance_name)
{
    const auto &ip = state.dataInputProcessing->inputProcessor;

    if (ip->getNumObjectsFound(state, object_to_find) > 0) { // e.g. "Coil::Cooling::DX::CurveFit::Performance"
        auto const &json_dict_performance = ip->epJSON.find(std::string(object_to_find)).value();
        for (auto &instance : json_dict_performance.items()) {
            std::string const &performance_name = EnergyPlus::Util::makeUPPER(instance.key());
            if (performance_name == idd_performance_name) { // e.g. "Heat Pump ACDXCoil 1 Performance"
                // ip->markObjectAsUsed(object_to_find, performance_name);
                return true;
            }
        }
    }
    return false;
}
