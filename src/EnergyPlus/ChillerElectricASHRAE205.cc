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
#include <cassert>
#include <cmath>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include "rs0001_factory.h"
#include <EnergyPlus/Autosizing/All_Simple_Sizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/ChillerElectricASHRAE205.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/EnergyPlusLogger.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::ChillerElectricASHRAE205 {

constexpr std::array<std::string_view, static_cast<int>(AmbientTempIndicator::Num) - 1> AmbientTempNamesUC{
    "SCHEDULE",
    "ZONE",
    "OUTDOORS",
};

std::map<std::string, Btwxt::InterpolationMethod> InterpMethods = // NOLINT(cert-err58-cpp)
    {{"LINEAR", Btwxt::InterpolationMethod::linear}, {"CUBIC", Btwxt::InterpolationMethod::cubic}};

void getChillerASHRAE205Input(EnergyPlusData &state)
{
    static constexpr std::string_view RoutineName("getChillerASHRAE205Input: "); // include trailing blank space
    static constexpr std::string_view routineName = "getChillerASHRAE205Input";

    using namespace tk205;
    RSInstanceFactory::register_factory("RS0001", std::make_shared<RS0001Factory>());

    bool ErrorsFound{false};

    auto &s_ip = state.dataInputProcessing->inputProcessor;
    auto &s_ipsc = state.dataIPShortCut;

    state.dataIPShortCut->cCurrentModuleObject = ChillerElectricASHRAE205::ASHRAE205ChillerSpecs::ObjectType;
    int numElectric205Chillers = s_ip->getNumObjectsFound(state, state.dataIPShortCut->cCurrentModuleObject);

    if (numElectric205Chillers <= 0) {
        ShowSevereError(state, format("No {} equipment specified in input file", state.dataIPShortCut->cCurrentModuleObject));
        ErrorsFound = true;
    }

    state.dataChillerElectricASHRAE205->Electric205Chiller.allocate(numElectric205Chillers);

    auto const &ChillerInstances = s_ip->epJSON.find(state.dataIPShortCut->cCurrentModuleObject).value();
    int ChillerNum{0};
    auto const &objectSchemaProps = s_ip->getObjectSchemaProps(state, state.dataIPShortCut->cCurrentModuleObject);

    for (auto &instance : ChillerInstances.items()) {
        auto const &fields = instance.value();
        std::string const &thisObjectName = instance.key();

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, thisObjectName};

        GlobalNames::VerifyUniqueChillerName(
            state, state.dataIPShortCut->cCurrentModuleObject, thisObjectName, ErrorsFound, state.dataIPShortCut->cCurrentModuleObject + " Name");

        ++ChillerNum;
        auto &thisChiller = state.dataChillerElectricASHRAE205->Electric205Chiller(ChillerNum);
        thisChiller.Name = Util::makeUPPER(thisObjectName);

        s_ip->markObjectAsUsed(state.dataIPShortCut->cCurrentModuleObject, thisObjectName);

        std::string const rep_file_name = s_ip->getAlphaFieldValue(fields, objectSchemaProps, "representation_file_name");
        fs::path rep_file_path = DataSystemVariables::CheckForActualFilePath(state, fs::path(rep_file_name), std::string(RoutineName));
        if (rep_file_path.empty()) {
            ErrorsFound = true;
            // Given that several of the following expressions require the representation file to be present, we'll just throw a fatal here.
            // The ErrorsFound flag is still set to true here so that in the future, if we defer the fatal until later in this routine, it will still
            // be set The CheckForActualFilePath function emits some nice information to the ERR file, so we just need a simple fatal here
            ShowFatalError(state, "Program terminates due to the missing ASHRAE 205 RS0001 representation file.");
        }
        // Since logger context must persist across all calls to libtk205/btwxt, it must be a member
        thisChiller.LoggerContext = {&state, format("{} \"{}\"", state.dataIPShortCut->cCurrentModuleObject, thisObjectName)};
        thisChiller.Representation = std::dynamic_pointer_cast<tk205::rs0001_ns::RS0001>(
            RSInstanceFactory::create("RS0001", FileSystem::toString(rep_file_path).c_str(), std::make_shared<EnergyPlusLogger>()));
        if (nullptr == thisChiller.Representation) {
            ShowSevereError(state, format("{} is not an instance of an ASHRAE205 Chiller.", rep_file_path));
            ErrorsFound = true;
        }
        thisChiller.Representation->performance.performance_map_cooling.get_logger()->set_message_context(&thisChiller.LoggerContext);
        thisChiller.Representation->performance.performance_map_standby.get_logger()->set_message_context(&thisChiller.LoggerContext);
        thisChiller.InterpolationType =
            InterpMethods[Util::makeUPPER(s_ip->getAlphaFieldValue(fields, objectSchemaProps, "performance_interpolation_method"))];

        const auto &compressorSequence = thisChiller.Representation->performance.performance_map_cooling.grid_variables.compressor_sequence_number;
        // minmax_element is sound but perhaps overkill; as sequence numbers are required by A205 to be in ascending order
        const auto minmaxSequenceNum = std::minmax_element(compressorSequence.begin(), compressorSequence.end());
        thisChiller.MinSequenceNumber = *(minmaxSequenceNum.first);
        thisChiller.MaxSequenceNumber = *(minmaxSequenceNum.second);

        if (fields.count("rated_capacity")) {
            ShowWarningError(state, format("{}{}=\"{}\"", std::string{RoutineName}, state.dataIPShortCut->cCurrentModuleObject, thisChiller.Name));
            ShowContinueError(state, "Rated Capacity field is not yet supported for ASHRAE 205 representations.");
        }

        thisChiller.RefCap = 0.0;               // ip->getRealFieldValue(fields, objectSchemaProps, "rated_capacity");
        thisChiller.RefCapWasAutoSized = false; // for now

        //        if (thisChiller.RefCap == DataSizing::AutoSize) {
        //            thisChiller.RefCapWasAutoSized = true;
        //        }
        //        if (thisChiller.RefCap == 0.0) {
        //            ShowSevereError(
        //                state, format("{}{}=\"{}\"",std::string{RoutineName},state.dataIPShortCut->cCurrentModuleObject,thisChiller.Name);
        //            ShowContinueError(state, format("Invalid {}={:.2R}", "Rated Capacity", thisChiller.RefCap));
        //            ErrorsFound = true;
        //        }

        std::string const evap_inlet_node_name = s_ip->getAlphaFieldValue(fields, objectSchemaProps, "chilled_water_inlet_node_name");
        std::string const evap_outlet_node_name = s_ip->getAlphaFieldValue(fields, objectSchemaProps, "chilled_water_outlet_node_name");
        if (evap_inlet_node_name.empty() || evap_outlet_node_name.empty()) {
            ShowSevereError(state, format("{}{}=\"{}\"", std::string{RoutineName}, state.dataIPShortCut->cCurrentModuleObject, thisChiller.Name));
            ShowContinueError(state, "Evaporator Inlet or Outlet Node Name is blank.");
            ErrorsFound = true;
        }
        thisChiller.EvapInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                           evap_inlet_node_name,
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                           thisChiller.Name,
                                                                           DataLoopNode::NodeFluidType::Water,
                                                                           DataLoopNode::ConnectionType::Inlet,
                                                                           NodeInputManager::CompFluidStream::Primary,
                                                                           DataLoopNode::ObjectIsNotParent);
        thisChiller.EvapOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                            evap_outlet_node_name,
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                            thisChiller.Name,
                                                                            DataLoopNode::NodeFluidType::Water,
                                                                            DataLoopNode::ConnectionType::Outlet,
                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                            DataLoopNode::ObjectIsNotParent);
        BranchNodeConnections::TestCompSet(
            state, state.dataIPShortCut->cCurrentModuleObject, thisChiller.Name, evap_inlet_node_name, evap_outlet_node_name, "Chilled Water Nodes");

        thisChiller.CondenserType = DataPlant::CondenserType::WaterCooled;

        std::string const cond_inlet_node_name = s_ip->getAlphaFieldValue(fields, objectSchemaProps, "condenser_inlet_node_name");
        std::string const cond_outlet_node_name = s_ip->getAlphaFieldValue(fields, objectSchemaProps, "condenser_outlet_node_name");
        if (cond_inlet_node_name.empty() || cond_outlet_node_name.empty()) {
            ShowSevereError(state, format("{}{}=\"{}\"", std::string{RoutineName}, state.dataIPShortCut->cCurrentModuleObject, thisChiller.Name));
            ShowContinueError(state, "Condenser Inlet or Outlet Node Name is blank.");
            ErrorsFound = true;
        }
        thisChiller.CondInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                           cond_inlet_node_name,
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                           thisChiller.Name,
                                                                           DataLoopNode::NodeFluidType::Water,
                                                                           DataLoopNode::ConnectionType::Inlet,
                                                                           NodeInputManager::CompFluidStream::Secondary,
                                                                           DataLoopNode::ObjectIsNotParent);

        thisChiller.CondOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                            cond_outlet_node_name,
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                            thisChiller.Name,
                                                                            DataLoopNode::NodeFluidType::Water,
                                                                            DataLoopNode::ConnectionType::Outlet,
                                                                            NodeInputManager::CompFluidStream::Secondary,
                                                                            DataLoopNode::ObjectIsNotParent);

        BranchNodeConnections::TestCompSet(state,
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           thisChiller.Name,
                                           cond_inlet_node_name,
                                           cond_outlet_node_name,
                                           "Condenser Water Nodes");

        thisChiller.FlowMode = static_cast<DataPlant::FlowMode>(
            getEnumValue(DataPlant::FlowModeNamesUC, s_ip->getAlphaFieldValue(fields, objectSchemaProps, "chiller_flow_mode")));

        if (thisChiller.FlowMode == DataPlant::FlowMode::Invalid) {
            ShowSevereError(state, format("{}{}=\"{}\"", std::string{RoutineName}, state.dataIPShortCut->cCurrentModuleObject, thisObjectName));
            ShowContinueError(state, format("Invalid Chiller Flow Mode = {}", fields.at("chiller_flow_mode").get<std::string>()));
            ShowContinueError(state, "Available choices are ConstantFlow, NotModulated, or LeavingSetpointModulated");
            ShowContinueError(state, "Flow mode NotModulated is assumed and the simulation continues.");
            thisChiller.FlowMode = DataPlant::FlowMode::NotModulated;
        };

        thisChiller.SizFac = fields.at("sizing_factor").get<Real64>();
        if (thisChiller.SizFac <= 0.0) {
            thisChiller.SizFac = 1.0;
        }

        {
            auto &tmpFlowRate = fields.at("chilled_water_maximum_requested_flow_rate");
            if (tmpFlowRate == "Autosize") {
                thisChiller.EvapVolFlowRate = DataSizing::AutoSize;
                thisChiller.EvapVolFlowRateWasAutoSized = true;
            } else {
                thisChiller.EvapVolFlowRate = tmpFlowRate.get<Real64>();
            }
        }
        {
            auto &tmpFlowRate = fields.at("condenser_maximum_requested_flow_rate");
            if (tmpFlowRate == "Autosize") {
                thisChiller.CondVolFlowRate = DataSizing::AutoSize;
                thisChiller.CondVolFlowRateWasAutoSized = true;
            } else {
                thisChiller.CondVolFlowRate = tmpFlowRate.get<Real64>();
            }
        }

        thisChiller.AmbientTempType = static_cast<AmbientTempIndicator>(
            getEnumValue(AmbientTempNamesUC, Util::makeUPPER(s_ip->getAlphaFieldValue(fields, objectSchemaProps, "ambient_temperature_indicator"))));
        switch (thisChiller.AmbientTempType) {
        case AmbientTempIndicator::Schedule: {
            std::string const ambient_temp_schedule = s_ip->getAlphaFieldValue(fields, objectSchemaProps, "ambient_temperature_schedule");
            if ((thisChiller.ambientTempSched = Sched::GetSchedule(state, ambient_temp_schedule)) == nullptr) {
                ShowSevereItemNotFound(state, eoh, "Ambient Temperature Schedule", ambient_temp_schedule);
                ErrorsFound = true;
            }

            break;
        }
        case AmbientTempIndicator::TempZone: {
            std::string const ambient_temp_zone_name = s_ip->getAlphaFieldValue(fields, objectSchemaProps, "ambient_temperature_zone_name");
            thisChiller.AmbientTempZone = Util::FindItemInList(ambient_temp_zone_name, state.dataHeatBal->Zone);
            if (thisChiller.AmbientTempZone == 0) {
                ShowSevereError(state,
                                format("{} = {}:  Ambient Temperature Zone not found = {}",
                                       state.dataIPShortCut->cCurrentModuleObject,
                                       thisObjectName,
                                       ambient_temp_zone_name));
                ErrorsFound = true;
            } else {
                SetupZoneInternalGain(state,
                                      thisChiller.AmbientTempZone,
                                      thisChiller.Name,
                                      DataHeatBalance::IntGainType::ElectricEquipment,
                                      &thisChiller.AmbientZoneGain);
            }
            break;
        }
        case AmbientTempIndicator::OutsideAir: {
            std::string const ambient_temp_outdoor_node =
                s_ip->getAlphaFieldValue(fields, objectSchemaProps, "ambient_temperature_outdoor_air_node_name");
            thisChiller.AmbientTempOutsideAirNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                        ambient_temp_outdoor_node,
                                                                                        ErrorsFound,
                                                                                        DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                                        thisChiller.Name,
                                                                                        DataLoopNode::NodeFluidType::Air,
                                                                                        DataLoopNode::ConnectionType::OutsideAirReference,
                                                                                        NodeInputManager::CompFluidStream::Primary,
                                                                                        DataLoopNode::ObjectIsNotParent);
            if (fields.count("ambient_temperature_outdoor_air_node_name")) {
                if (!OutAirNodeManager::CheckOutAirNodeNumber(state, thisChiller.AmbientTempOutsideAirNode)) {
                    ShowSevereError(state,
                                    format("{} = {}: Outdoor Air Node not on OutdoorAir:NodeList or OutdoorAir:Node",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           thisObjectName));
                    ShowContinueError(state, format("...Referenced Node Name={}", ambient_temp_outdoor_node));
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(state, format("{} = {}", state.dataIPShortCut->cCurrentModuleObject, ambient_temp_outdoor_node));
                ShowContinueError(state, "An Ambient Outdoor Air Node name must be used when the Ambient Temperature Indicator is Outdoors.");
                ErrorsFound = true;
            }

            break;
        }
        default: {
            ShowSevereError(state,
                            format("{} = {}:  Invalid Ambient Temperature Indicator entered={}",
                                   state.dataIPShortCut->cCurrentModuleObject,
                                   thisObjectName,
                                   s_ip->getAlphaFieldValue(fields, objectSchemaProps, "ambient_temperature_indicator")));
            ShowContinueError(state, " Valid entries are SCHEDULE, ZONE, and OUTDOORS.");
            ErrorsFound = true;
            break;
        }
        }
        // end Ambient temperature
        std::string const oil_cooler_inlet_node = s_ip->getAlphaFieldValue(fields, objectSchemaProps, "oil_cooler_inlet_node_name");
        std::string const oil_cooler_outlet_node = s_ip->getAlphaFieldValue(fields, objectSchemaProps, "oil_cooler_outlet_node_name");
        if (!oil_cooler_inlet_node.empty() && !oil_cooler_outlet_node.empty()) {
            thisChiller.OilCoolerInletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                 oil_cooler_inlet_node,
                                                                                 ErrorsFound,
                                                                                 DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                                 thisChiller.Name,
                                                                                 DataLoopNode::NodeFluidType::Water,
                                                                                 DataLoopNode::ConnectionType::Inlet,
                                                                                 NodeInputManager::CompFluidStream::Tertiary,
                                                                                 DataLoopNode::ObjectIsNotParent);
            thisChiller.OilCoolerOutletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                  oil_cooler_outlet_node,
                                                                                  ErrorsFound,
                                                                                  DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                                  thisChiller.Name,
                                                                                  DataLoopNode::NodeFluidType::Water,
                                                                                  DataLoopNode::ConnectionType::Outlet,
                                                                                  NodeInputManager::CompFluidStream::Tertiary,
                                                                                  DataLoopNode::ObjectIsNotParent);
            BranchNodeConnections::TestCompSet(state,
                                               state.dataIPShortCut->cCurrentModuleObject,
                                               thisChiller.Name,
                                               oil_cooler_inlet_node,
                                               oil_cooler_outlet_node,
                                               "Oil Cooler Water Nodes");
        }
        std::string const aux_heat_inlet_node = s_ip->getAlphaFieldValue(fields, objectSchemaProps, "auxiliary_inlet_node_name");
        std::string const aux_heat_outlet_node = s_ip->getAlphaFieldValue(fields, objectSchemaProps, "auxiliary_outlet_node_name");
        if (!aux_heat_inlet_node.empty() && !aux_heat_outlet_node.empty()) {

            thisChiller.AuxiliaryHeatInletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                     aux_heat_inlet_node,
                                                                                     ErrorsFound,
                                                                                     DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                                     thisChiller.Name,
                                                                                     DataLoopNode::NodeFluidType::Water,
                                                                                     DataLoopNode::ConnectionType::Inlet,
                                                                                     NodeInputManager::CompFluidStream::Quaternary,
                                                                                     DataLoopNode::ObjectIsNotParent);
            thisChiller.AuxiliaryHeatOutletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                      aux_heat_outlet_node,
                                                                                      ErrorsFound,
                                                                                      DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                                      thisChiller.Name,
                                                                                      DataLoopNode::NodeFluidType::Water,
                                                                                      DataLoopNode::ConnectionType::Outlet,
                                                                                      NodeInputManager::CompFluidStream::Quaternary,
                                                                                      DataLoopNode::ObjectIsNotParent);
            BranchNodeConnections::TestCompSet(state,
                                               state.dataIPShortCut->cCurrentModuleObject,
                                               thisChiller.Name,
                                               aux_heat_inlet_node,
                                               aux_heat_outlet_node,
                                               "Auxiliary Water Nodes");
        }

        // TODO: When implemented, add ...WasAutoSized variables
        if (fields.count("oil_cooler_design_flow_rate")) {
            thisChiller.OilCoolerVolFlowRate = fields.at("oil_cooler_design_flow_rate").get<Real64>();
        }
        if (fields.count("auxiliary_equipment_design_flow_rate")) {
            thisChiller.AuxiliaryVolFlowRate = fields.at("auxiliary_equipment_design_flow_rate").get<Real64>();
        }

        if (fields.count("end_use_subcategory")) {
            thisChiller.EndUseSubcategory = s_ip->getAlphaFieldValue(fields, objectSchemaProps, "end_use_subcategory");
        } else {
            thisChiller.EndUseSubcategory = "General";
        }
        // Set reference conditions
        thisChiller.TempRefCondIn = 29.44;
        thisChiller.TempRefEvapOut = 6.67;
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("Errors found in processing input for {}", state.dataIPShortCut->cCurrentModuleObject));
    }
}

ASHRAE205ChillerSpecs *ASHRAE205ChillerSpecs::factory(EnergyPlusData &state, std::string const &objectName)
{
    if (state.dataChillerElectricASHRAE205->getInputFlag) {
        getChillerASHRAE205Input(state);
        state.dataChillerElectricASHRAE205->getInputFlag = false;
    }
    auto thisObj = std::find_if(state.dataChillerElectricASHRAE205->Electric205Chiller.begin(),
                                state.dataChillerElectricASHRAE205->Electric205Chiller.end(),
                                [&objectName](const ASHRAE205ChillerSpecs &myObj) { return myObj.Name == objectName; });
    if (thisObj != state.dataChillerElectricASHRAE205->Electric205Chiller.end()) {
        return thisObj;
    }
    // If we didn't find it, fatal
    ShowFatalError(state, format("ASHRAE205ChillerSpecs::factory: Error getting inputs for object named: {}", objectName)); // LCOV_EXCL_LINE
    return nullptr;                                                                                                         // LCOV_EXCL_LINE
}

void ASHRAE205ChillerSpecs::oneTimeInit_new(EnergyPlusData &state)
{
    // This function is called from GetPlantInput
    // Locate the chillers on the plant loops for later usage
    bool errFlag{false};
    PlantUtilities::ScanPlantLoopsForObject(
        state, this->Name, DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205, this->CWPlantLoc, errFlag, _, _, _, this->EvapInletNodeNum, _);
    if (this->CondenserType != DataPlant::CondenserType::AirCooled) {
        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205,
                                                this->CDPlantLoc,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->CondInletNodeNum,
                                                _);
        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->CWPlantLoc, this->CDPlantLoc, DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205, true);
    }
    if (this->OilCoolerInletNode) {
        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205,
                                                this->OCPlantLoc,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->OilCoolerInletNode,
                                                _);
    }
    if (this->AuxiliaryHeatInletNode) {
        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205,
                                                this->AHPlantLoc,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->AuxiliaryHeatInletNode,
                                                _);
    }
// If and when heat recovery is implemented, uncomment
#if 0
        if (this->HeatRecActive) {
            PlantUtilities::ScanPlantLoopsForObject(state,
                                                    this->Name,
                                                    DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205,
                                                    this->HRPlantLoc,
                                                    errFlag,
                                                    _,
                                                    _,
                                                    _,
                                                    this->HeatRecInletNodeNum,
                                                    _);
            PlantUtilities::InterConnectTwoPlantLoopSides(
                    state, this->CWPlantLoc, this->HRPlantLoc, DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205, true);
        }

        if ((this->CondenserType != DataPlant::CondenserType::AirCooled) && (this->HeatRecActive)) {
            PlantUtilities::InterConnectTwoPlantLoopSides(
                    state, this->CDPlantLoc, this->HRPlantLoc, DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205, false);
        }
#endif // #if 0

    if (errFlag) {
        ShowFatalError(state, "InitElecASHRAE205Chiller: Program terminated due to previous condition(s).");
    }

    if (this->FlowMode == DataPlant::FlowMode::Constant) {
        // reset flow priority
        this->CWPlantLoc.comp->FlowPriority = DataPlant::LoopFlowStatus::NeedyIfLoopOn;
    }

    else if (this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) {
        // reset flow priority
        this->CWPlantLoc.comp->FlowPriority = DataPlant::LoopFlowStatus::NeedyIfLoopOn;
        // check if setpoint on outlet node
        if ((state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint == DataLoopNode::SensedNodeFlagValue) &&
            (state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi == DataLoopNode::SensedNodeFlagValue)) {
            if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                if (!this->ModulatedFlowErrDone) {
                    ShowWarningError(state, format("Missing temperature setpoint for LeavingSetpointModulated mode chiller named {}", this->Name));
                    ShowContinueError(
                        state, "  A temperature setpoint is needed at the outlet node of a chiller in variable flow mode, use a SetpointManager");
                    ShowContinueError(state, "  The overall loop setpoint will be assumed for chiller. The simulation continues ... ");
                    this->ModulatedFlowErrDone = true;
                }
            } else {
                // need call to EMS to check node
                bool fatalError = false; // but not really fatal yet, but should be.
                EMSManager::CheckIfNodeSetPointManagedByEMS(state, this->EvapOutletNodeNum, HVAC::CtrlVarType::Temp, fatalError);
                state.dataLoopNodes->NodeSetpointCheck(this->EvapOutletNodeNum).needsSetpointChecking = false;
                if (fatalError) {
                    if (!this->ModulatedFlowErrDone) {
                        ShowWarningError(state,
                                         format("Missing temperature setpoint for LeavingSetpointModulated mode chiller named {}", this->Name));
                        ShowContinueError(state,
                                          "  A temperature setpoint is needed at the outlet node of a chiller evaporator in variable flow mode");
                        ShowContinueError(state, "  use a Setpoint Manager to establish a setpoint at the chiller evaporator outlet node ");
                        ShowContinueError(state, "  or use an EMS actuator to establish a setpoint at the outlet node ");
                        ShowContinueError(state, "  The overall loop setpoint will be assumed for chiller. The simulation continues ... ");
                        this->ModulatedFlowErrDone = true;
                    }
                }
            }
            this->ModulatedFlowSetToLoop = true;
            state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint =
                state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPoint;
            state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi =
                state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPointHi;
        }
    }

    this->setOutputVariables(state);
}

void ASHRAE205ChillerSpecs::initialize(EnergyPlusData &state, bool const RunFlag, Real64 const MyLoad)
{
    static constexpr std::string_view RoutineName("ASHRAE205ChillerSpecs::initialize");

    switch (this->AmbientTempType) {
    case AmbientTempIndicator::Schedule: {
        this->AmbientTemp = this->ambientTempSched->getCurrentVal();
        break;
    }
    case AmbientTempIndicator::TempZone: {
        this->AmbientTemp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(this->AmbientTempZone).MAT;
        break;
    }
    case AmbientTempIndicator::OutsideAir: {
        this->AmbientTemp = state.dataLoopNodes->Node(this->AmbientTempOutsideAirNode).Temp;
        break;
    }
    default:
        break;
    }

    this->EquipFlowCtrl = this->CWPlantLoc.comp->FlowCtrl;

    if (this->MyEnvrnFlag && state.dataGlobal->BeginEnvrnFlag && (state.dataPlnt->PlantFirstSizesOkayToFinalize)) {
        Real64 rho = this->CWPlantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);

        this->EvapMassFlowRateMax = rho * this->EvapVolFlowRate;
        PlantUtilities::InitComponentNodes(state, 0.0, this->EvapMassFlowRateMax, this->EvapInletNodeNum, this->EvapOutletNodeNum);

        if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
            rho = this->CDPlantLoc.loop->glycol->getDensity(state, this->TempRefCondIn, RoutineName);
            this->CondMassFlowRateMax = rho * this->CondVolFlowRate;
            PlantUtilities::InitComponentNodes(state, 0.0, this->CondMassFlowRateMax, this->CondInletNodeNum, this->CondOutletNodeNum);
            state.dataLoopNodes->Node(this->CondInletNodeNum).Temp = this->TempRefCondIn;
        }
        // Set mass flow rates at Oil Cooler and Aux Equipment nodes
        if (this->OilCoolerInletNode) {
            Real64 rho_oil_cooler = this->OCPlantLoc.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
            this->OilCoolerMassFlowRate = rho_oil_cooler * this->OilCoolerVolFlowRate;
            PlantUtilities::InitComponentNodes(state, 0.0, this->OilCoolerMassFlowRate, this->OilCoolerInletNode, this->OilCoolerOutletNode);
        }
        if (this->AuxiliaryHeatInletNode) {
            Real64 rho_aux = this->AHPlantLoc.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
            this->AuxiliaryMassFlowRate = rho_aux * this->AuxiliaryVolFlowRate;
            PlantUtilities::InitComponentNodes(state, 0.0, this->AuxiliaryMassFlowRate, this->AuxiliaryHeatInletNode, this->AuxiliaryHeatOutletNode);
        }
    }
    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->MyEnvrnFlag = true;
    }

    if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) && this->ModulatedFlowSetToLoop) {
        // see ReformulatedEIR or EIR Chiller for origin of the following
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint =
            state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPoint;
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi =
            state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPointHi;
    }

    Real64 mdot = ((std::abs(MyLoad) > 0.0) && RunFlag) ? this->EvapMassFlowRateMax : 0.0;
    Real64 mdotCond = ((std::abs(MyLoad) > 0.0) && RunFlag) ? this->CondMassFlowRateMax : 0.0;

    PlantUtilities::SetComponentFlowRate(state, mdot, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);

    if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
        PlantUtilities::SetComponentFlowRate(state, mdotCond, this->CondInletNodeNum, this->CondOutletNodeNum, this->CDPlantLoc);
    }

    // Set component flow rates for Oil Cooler and Aux equipment
    if (this->OilCoolerInletNode) {
        PlantUtilities::SetComponentFlowRate(
            state, this->OilCoolerMassFlowRate, this->OilCoolerInletNode, this->OilCoolerOutletNode, this->OCPlantLoc);
    }
    if (this->AuxiliaryHeatInletNode) {
        PlantUtilities::SetComponentFlowRate(
            state, this->AuxiliaryMassFlowRate, this->AuxiliaryHeatInletNode, this->AuxiliaryHeatOutletNode, this->AHPlantLoc);
    }
    // Recalculate volumetric flow rates from component mass flow rates if necessary

// Revisit when heat recovery implemented
#if 0
      // Initialize heat recovery flow rates at node
        if (this->HeatRecActive) {

            // check if inlet limit active and if exceeded.
            bool HeatRecRunFlag = RunFlag;
            if (this->heatRecInletLimitSched != nullptr) {
                Real64 HeatRecHighInletLimit = this->heatRecInletLimitSched->getCurrentVal();
                if (state.dataLoopNodes->Node(this->HeatRecInletNodeNum).Temp > HeatRecHighInletLimit) { // shut down heat recovery
                    HeatRecRunFlag = false;
                } else {
                    HeatRecRunFlag = RunFlag;
                }
            }

            mdot = HeatRecRunFlag ? this->DesignHeatRecMassFlowRate : 0.0;

            PlantUtilities::SetComponentFlowRate(state, mdot, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum, this->HRPlantLoc);
        }
#endif // if 0
}

void ASHRAE205ChillerSpecs::size([[maybe_unused]] EnergyPlusData &state)
{
    static constexpr std::string_view RoutineName("SizeElectricASHRAE205Chiller");

    bool ErrorsFound = false;
    Real64 tmpNomCap{0.0};
    Real64 tmpEvapVolFlowRate = this->EvapVolFlowRate;
    Real64 tmpCondVolFlowRate = this->CondVolFlowRate;

    // Size evaporator flow rate
    // find the appropriate Plant Sizing object
    int PltSizNum = this->CWPlantLoc.loop->PlantSizNum;

    if (PltSizNum > 0) {
        if (state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
            tmpEvapVolFlowRate = state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate * this->SizFac;
        } else {
            if (this->EvapVolFlowRateWasAutoSized) {
                tmpEvapVolFlowRate = 0.0;
            }
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->EvapVolFlowRateWasAutoSized) {
                this->EvapVolFlowRate = tmpEvapVolFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, this->ObjectType, this->Name, "Design Size Chilled Water Maximum Requested Flow Rate [m3/s]", tmpEvapVolFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 this->ObjectType,
                                                 this->Name,
                                                 "Initial Design Size Chilled Water Maximum Requested Flow Rate [m3/s]",
                                                 tmpEvapVolFlowRate);
                }
            } else { // Hard-size with sizing data
                if (this->EvapVolFlowRate > 0.0 && tmpEvapVolFlowRate > 0.0) {
                    Real64 EvapVolFlowRateUser = this->EvapVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     this->ObjectType,
                                                     this->Name,
                                                     "Design Size Chilled Water Maximum Requested Flow Rate [m3/s]",
                                                     tmpEvapVolFlowRate,
                                                     "User-Specified Chilled Water Maximum Requested Flow Rate [m3/s]",
                                                     EvapVolFlowRateUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpEvapVolFlowRate - EvapVolFlowRateUser) / EvapVolFlowRateUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state, format("{}: Potential issue with equipment sizing for {}", RoutineName, this->Name));
                                ShowContinueError(
                                    state, format("User-Specified Chilled Water Maximum Requested Flow Rate of {:.5R} [m3/s]", EvapVolFlowRateUser));
                                ShowContinueError(state,
                                                  format("differs from Design Size Chilled Water Maximum Requested Flow Rate of {:.5R} [m3/s]",
                                                         tmpEvapVolFlowRate));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                    tmpEvapVolFlowRate = EvapVolFlowRateUser;
                }
            }
        }
    } else {
        if (this->EvapVolFlowRateWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            ShowSevereError(state, "Autosizing of Electric Chiller evap flow rate requires a loop Sizing:Plant object");
            ShowContinueError(state, format("Occurs in Electric Chiller object={}", this->Name));
            ErrorsFound = true;
        }
        if (!this->EvapVolFlowRateWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport && (this->EvapVolFlowRate > 0.0)) {
            BaseSizer::reportSizerOutput(
                state, this->ObjectType, this->Name, "User-Specified Chilled Water Maximum Requested Flow Rate [m3/s]", this->EvapVolFlowRate);
        }
    }

    PlantUtilities::RegisterPlantCompDesignFlow(state, this->EvapInletNodeNum, tmpEvapVolFlowRate);

    // Size condenser flow rate
    int PltSizCondNum = this->CDPlantLoc.loop->PlantSizNum; // Change for air-cooled when it's supported
    if (PltSizCondNum > 0 && PltSizNum > 0) {
        if (state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow && tmpNomCap > 0.0) {

            Real64 rho = this->CDPlantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
            Real64 Cp = this->CDPlantLoc.loop->glycol->getSpecificHeat(state, this->TempRefCondIn, RoutineName);
            tmpCondVolFlowRate = tmpNomCap * (1.0 + (1.0 / this->RefCOP) * this->CompPowerToCondenserFrac) /
                                 (state.dataSize->PlantSizData(PltSizCondNum).DeltaT * Cp * rho);

        } else {
            if (this->CondVolFlowRateWasAutoSized) {
                tmpCondVolFlowRate = 0.0;
            }
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->CondVolFlowRateWasAutoSized) {
                this->CondVolFlowRate = tmpCondVolFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, this->ObjectType, this->Name, "Design Size Condenser Maximum Requested Flow Rate [m3/s]", tmpCondVolFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, this->ObjectType, this->Name, "Initial Design Size Condenser Maximum Requested Flow Rate [m3/s]", tmpCondVolFlowRate);
                }
            } else {
                if (this->CondVolFlowRate > 0.0 && tmpCondVolFlowRate > 0.0) {
                    Real64 CondVolFlowRateUser = this->CondVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     this->ObjectType,
                                                     this->Name,
                                                     "Design Size Condenser Maximum Requested Flow Rate [m3/s]",
                                                     tmpCondVolFlowRate,
                                                     "User-Specified Condenser Maximum Requested Flow Rate [m3/s]",
                                                     CondVolFlowRateUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpCondVolFlowRate - CondVolFlowRateUser) / CondVolFlowRateUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state, format("{}: Potential issue with equipment sizing for {}", RoutineName, this->Name));
                                ShowContinueError(
                                    state, format("User-Specified Condenser Maximum Requested Flow Rate of {:.5R} [m3/s]", CondVolFlowRateUser));
                                ShowContinueError(
                                    state,
                                    format("differs from Design Size Condenser Maximum Requested Flow Rate of {:.5R} [m3/s]", tmpCondVolFlowRate));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                    tmpCondVolFlowRate = CondVolFlowRateUser;
                }
            }
        }
    } else {
        if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {

            if (this->CondVolFlowRateWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                ShowSevereError(state, "Autosizing of Electric ASHRAE 205 Chiller condenser fluid flow rate requires a condenser");
                ShowContinueError(state, "loop Sizing:Plant object");
                ShowContinueError(state, format("Occurs in Electric ASHRAE 205 Chiller object={}", this->Name));
                ErrorsFound = true;
            }
            if (!this->CondVolFlowRateWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport && (this->CondVolFlowRate > 0.0)) {
                BaseSizer::reportSizerOutput(
                    state, this->ObjectType, this->Name, "User-Specified Condenser Maximum Requested Flow Rate [m3/s]", this->CondVolFlowRate);
            }

        } else {

            // Auto size condenser air flow to Total Capacity * 0.000114 m3/s/w (850 cfm/ton)
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                std::string_view CompType =
                    DataPlant::PlantEquipTypeNames[static_cast<int>(DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205)];
                state.dataSize->DataConstantUsedForSizing = this->RefCap;
                state.dataSize->DataFractionUsedForSizing = 0.000114;
                Real64 TempSize = this->CondVolFlowRate;
                bool bPRINT = true; // TRUE if sizing is reported to output (eio)
                AutoCalculateSizer sizerCondAirFlow;
                std::string stringOverride = "Condenser Maximum Requested Flow Rate  [m3/s]";
                if (state.dataGlobal->isEpJSON) {
                    stringOverride = "condenser_maximum_requested_flow_rate [m3/s]";
                }
                sizerCondAirFlow.overrideSizingString(stringOverride);
                sizerCondAirFlow.initializeWithinEP(state, CompType, this->Name, bPRINT, RoutineName);
                this->CondVolFlowRate = sizerCondAirFlow.size(state, TempSize, ErrorsFound);
            }
        }
    }

    // save the reference condenser water volumetric flow rate for use by the condenser water loop sizing algorithms
    PlantUtilities::RegisterPlantCompDesignFlow(state, this->CondInletNodeNum, tmpCondVolFlowRate);

    // Calculate design evaporator capacity (eventually add autosize here too)

    // TODO: Determine actual rated flow rates instead of design flow rates
    this->RefCap = this->Representation->performance.performance_map_cooling
                       .calculate_performance(this->EvapVolFlowRate,
                                              this->TempRefEvapOut + Constant::Kelvin,
                                              this->CondVolFlowRate,
                                              this->TempRefCondIn + Constant::Kelvin,
                                              this->MaxSequenceNumber,
                                              this->InterpolationType)
                       .net_evaporator_capacity;

    if (PltSizNum > 0) {
        if (state.dataSize->PlantSizData(PltSizNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
            Real64 Cp = this->CWPlantLoc.loop->glycol->getSpecificHeat(state, Constant::CWInitConvTemp, RoutineName);

            Real64 rho = this->CWPlantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
            tmpNomCap = Cp * rho * state.dataSize->PlantSizData(PltSizNum).DeltaT * tmpEvapVolFlowRate;
        } else {
            tmpNomCap = 0.0;
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->RefCapWasAutoSized) {
                this->RefCap = tmpNomCap;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state, this->ObjectType, this->Name, "Design Size Rated Capacity [W]", tmpNomCap);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state, this->ObjectType, this->Name, "Initial Design Size Rated Capacity [W]", tmpNomCap);
                }
            } else { // Hard-sized with sizing data
                if (this->RefCap > 0.0 && tmpNomCap > 0.0) {
                    Real64 RefCapUser = this->RefCap;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     this->ObjectType,
                                                     this->Name,
                                                     "Design Size Rated Capacity [W]",
                                                     tmpNomCap,
                                                     "User-Specified Rated Capacity [W]",
                                                     RefCapUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpNomCap - RefCapUser) / RefCapUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state, format("{}: Potential issue with equipment sizing for {}", RoutineName, this->Name));
                                ShowContinueError(state, format("User-Specified Rated Capacity of {:.2R} [W]", RefCapUser));
                                ShowContinueError(state, format("differs from Design Size Rated Capacity of {:.2R} [W]", tmpNomCap));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                    tmpNomCap = RefCapUser;
                }
            }
        }
    } else {
        if (this->RefCapWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            ShowSevereError(state, "Autosizing of Electric Chiller reference capacity requires a loop Sizing:Plant object");
            ShowContinueError(state, format("Occurs in Electric Chiller object={}", this->Name));
            ErrorsFound = true;
        }
        if (!this->RefCapWasAutoSized && state.dataPlnt->PlantFinalSizesOkayToReport && (this->RefCap > 0.0)) { // Hard-sized with no sizing data
            BaseSizer::reportSizerOutput(state, this->ObjectType, this->Name, "User-Specified Rated Capacity [W]", this->RefCap);
        }
    }

    if (this->OilCoolerInletNode) {
        PlantUtilities::RegisterPlantCompDesignFlow(state, this->OilCoolerInletNode, this->OilCoolerVolFlowRate);
    }

    if (this->AuxiliaryHeatInletNode) {
        PlantUtilities::RegisterPlantCompDesignFlow(state, this->AuxiliaryHeatInletNode, this->AuxiliaryVolFlowRate);
    }

    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
        // create predefined report
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechType, this->Name, this->ObjectType);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomEff, this->Name, this->RefCOP);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomCap, this->Name, this->RefCap);
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Preceding sizing errors cause program termination");
    }
}

void ASHRAE205ChillerSpecs::setOutputVariables(EnergyPlusData &state)
{
    SetupOutputVariable(state,
                        "Chiller Part Load Ratio",
                        Constant::Units::None,
                        this->ChillerPartLoadRatio,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Cycling Ratio",
                        Constant::Units::None,
                        this->ChillerCyclingRatio,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Minimum Part Load Ratio",
                        Constant::Units::None,
                        this->MinPartLoadRat,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Electricity Rate",
                        Constant::Units::W,
                        this->Power,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Electricity Energy",
                        Constant::Units::J,
                        this->Energy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Cooling,
                        this->EndUseSubcategory);

    SetupOutputVariable(state,
                        "Chiller Evaporator Cooling Rate",
                        Constant::Units::W,
                        this->QEvaporator,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Evaporator Cooling Energy",
                        Constant::Units::J,
                        this->EvapEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Chillers);

    SetupOutputVariable(state,
                        "Chiller Evaporator Inlet Temperature",
                        Constant::Units::C,
                        this->EvapInletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Evaporator Outlet Temperature",
                        Constant::Units::C,
                        this->EvapOutletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Evaporator Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->EvapMassFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Heat Transfer Rate",
                        Constant::Units::W,
                        this->QCondenser,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Heat Transfer Energy",
                        Constant::Units::J,
                        this->CondEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::HeatRejection);

    SetupOutputVariable(state,
                        "Chiller COP",
                        Constant::Units::W_W,
                        this->ActualCOP,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Inlet Temperature",
                        Constant::Units::C,
                        this->CondInletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Outlet Temperature",
                        Constant::Units::C,
                        this->CondOutletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->CondMassFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Effective Heat Rejection Temperature",
                        Constant::Units::C,
                        this->ChillerCondAvgTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Zone Heat Gain Rate",
                        Constant::Units::W,
                        this->AmbientZoneGain,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Zone Heat Gain Energy",
                        Constant::Units::J,
                        this->AmbientZoneGainEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);

    SetupOutputVariable(state,
                        "Oil Cooler Heat Transfer Rate",
                        Constant::Units::W,
                        this->QOilCooler,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Oil Cooler Heat Transfer Energy",
                        Constant::Units::J,
                        this->OilCoolerEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);

    SetupOutputVariable(state,
                        "Auxiliary Heat Transfer Rate",
                        Constant::Units::W,
                        this->QAuxiliary,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Auxiliary Heat Transfer Energy",
                        Constant::Units::J,
                        this->AuxiliaryEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);
}

void ASHRAE205ChillerSpecs::findEvaporatorMassFlowRate(EnergyPlusData &state, Real64 &load, Real64 Cp)
{
    static constexpr std::string_view RoutineName("ASHRAE205ChillerSpecs::findEvaporatorMassFlowRate");

    // If FlowLock is False (0), the chiller sets the plant loop mdot
    // If FlowLock is True (1),  the new resolved plant loop mdot is used
    if (this->CWPlantLoc.side->FlowLock == DataPlant::FlowLock::Unlocked) {
        this->PossibleSubcooling = !(this->CWPlantLoc.comp->CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased);

        Real64 evapDeltaTemp(0.0); // Evaporator temperature difference [C]

        // Either set the flow to the Constant value or calculate the flow for the variable volume case
        if ((this->FlowMode == DataPlant::FlowMode::Constant) || (this->FlowMode == DataPlant::FlowMode::NotModulated)) {
            // Set the evaporator mass flow rate to design
            // Start by assuming max (design) flow
            this->EvapMassFlowRate = this->EvapMassFlowRateMax;
            // Use PlantUtilities::SetComponentFlowRate to decide actual flow
            PlantUtilities::SetComponentFlowRate(state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
            if (this->EvapMassFlowRate != 0.0) {
                evapDeltaTemp = std::abs(load) / this->EvapMassFlowRate / Cp; // MyLoad = net evaporator capacity, QEvaporator
            } else {
                evapDeltaTemp = 0.0;
            }
            this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - evapDeltaTemp;
        } else if (this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) {
            switch (this->CWPlantLoc.loop->LoopDemandCalcScheme) {
            case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                // Calculate the Delta Temp from the inlet temp to the chiller outlet setpoint
                evapDeltaTemp =
                    state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint;
            } break;
            case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
                evapDeltaTemp =
                    state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi;
            } break;
            default: {
                assert(false);
            } break;
            }

            if (evapDeltaTemp != 0) {
                this->EvapMassFlowRate = max(0.0, (std::abs(load) / Cp / evapDeltaTemp));
                if ((this->EvapMassFlowRate - this->EvapMassFlowRateMax) > DataBranchAirLoopPlant::MassFlowTolerance) {
                    this->PossibleSubcooling = true;
                }
                // Check to see if the Maximum is exceeded, if so set to maximum
                this->EvapMassFlowRate = min(this->EvapMassFlowRateMax, this->EvapMassFlowRate);
                // Use PlantUtilities::SetComponentFlowRate to decide actual flow
                PlantUtilities::SetComponentFlowRate(
                    state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
                // Should we recalculate this with the corrected setpoint?
                switch (this->CWPlantLoc.loop->LoopDemandCalcScheme) {
                case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                    this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint;
                } break;
                case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
                    this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi;
                } break;
                default:
                    break;
                }
            } else {
                // Try to request zero flow
                this->EvapMassFlowRate = 0.0;
                // Use PlantUtilities::SetComponentFlowRate to decide actual flow
                PlantUtilities::SetComponentFlowRate(
                    state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
                // No deltaT since component is not running
                this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
                this->QEvaporator = 0.0;
                // PartLoadRat = 0.0;
                this->ChillerPartLoadRatio = 0.0;

                if (this->DeltaTErrCount < 1 && !state.dataGlobal->WarmupFlag) {
                    ++this->DeltaTErrCount;
                    ShowWarningError(state, "Evaporator DeltaTemp = 0 in mass flow calculation (Tevapin = Tevapout setpoint temp).");
                    ShowContinueErrorTimeStamp(state, "");
                } else if (!state.dataGlobal->WarmupFlag) {
                    ++this->ChillerCapFTError;
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        format("{} \"{}\": Evaporator DeltaTemp = 0 in mass flow calculation warning continues...", this->ObjectType, this->Name),
                        this->DeltaTErrCountIndex,
                        evapDeltaTemp,
                        evapDeltaTemp);
                }
            }
        }
    } else { // If FlowLock is True
        this->EvapMassFlowRate = state.dataLoopNodes->Node(this->EvapInletNodeNum).MassFlowRate;
        PlantUtilities::SetComponentFlowRate(state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
        //       Some other component set the flow to 0. No reason to continue with calculations.
        if (this->EvapMassFlowRate == 0.0) {
            load = 0.0;
            return;
        }
    } // This is the end of the FlowLock Block

    const Real64 rho = this->CWPlantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);

    this->EvapVolFlowRate = this->EvapMassFlowRate / rho;
}

void ASHRAE205ChillerSpecs::calculate(EnergyPlusData &state, Real64 &MyLoad, bool const RunFlag)
{
    static constexpr std::string_view RoutineName("CalcElecASHRAE205ChillerModel");
    this->ChillerPartLoadRatio = 0.0;
    this->ChillerCyclingRatio = 1.0;
    this->ChillerFalseLoadRate = 0.0;
    this->EvapMassFlowRate = 0.0;
    this->CondMassFlowRate = 0.0;
    this->Power = 0.0;
    this->QCondenser = 0.0;
    this->QEvaporator = 0.0;
    this->QOilCooler = 0.0;
    this->QAuxiliary = 0.0;
    // int PlantLoopNum = this->CWPlantLoc.loopNum;
    // DataPlant::LoopSideLocation LoopSideNum = this->CWPlantLoc.loopSideNum;
    // int BranchNum = this->CWPlantLoc.branchNum;
    // int CompNum = this->CWPlantLoc.compNum;

    // Set module-level chiller evaporator and condenser inlet temperature variables
    // using prior time step's temperature
    Real64 condInletTemp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
    this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp;

    // If no loop demand or chiller OFF, return
    // If chiller load is 0 or chiller is not running then leave the subroutine. Before leaving
    //  if the component control is SERIESACTIVE we set the component flow to inlet flow so that
    //  flow resolver will not shut down the branch

    // Calculate performance for standby (only used when off or cycling)
    Real64 standbyPower =
        this->Representation->performance.performance_map_standby.calculate_performance(this->AmbientTemp, this->InterpolationType).input_power;
    if (MyLoad >= 0 || !RunFlag) {
        if (this->EquipFlowCtrl == DataBranchAirLoopPlant::ControlType::SeriesActive ||
            this->CWPlantLoc.side->FlowLock == DataPlant::FlowLock::Locked) {
            this->EvapMassFlowRate = state.dataLoopNodes->Node(this->EvapInletNodeNum).MassFlowRate;
        }
        if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
            if (this->CDPlantLoc.comp->FlowCtrl == DataBranchAirLoopPlant::ControlType::SeriesActive) {
                this->CondMassFlowRate = state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRate;
            }
        }
        this->Power = standbyPower;
        this->AmbientZoneGain = standbyPower;
        return;
    }

// Revisit
#if 0
        //        // If there is a fault of chiller fouling
//        if (this->FaultyChillerFoulingFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
//            (!state.dataGlobal->KickOffSimulation)) {
//            int FaultIndex = this->FaultyChillerFoulingIndex;
//            Real64 NomCap_ff = ChillerRefCap;
//            Real64 ReferenceCOP_ff = ReferenceCOP;
//
//            // calculate the Faulty Chiller Fouling Factor using fault information
//            this->FaultyChillerFoulingFactor = state.dataFaultsMgr->FaultsChillerFouling(FaultIndex).CalFoulingFactor(state);
//
//            // update the Chiller nominal capacity and COP at faulty cases
//            ChillerRefCap = NomCap_ff * this->FaultyChillerFoulingFactor;
//            ReferenceCOP = ReferenceCOP_ff * this->FaultyChillerFoulingFactor;
//        }
#endif // 0

    // Set mass flow rates
    if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
        this->CondMassFlowRate = this->CondMassFlowRateMax;
        PlantUtilities::SetComponentFlowRate(state, this->CondMassFlowRate, this->CondInletNodeNum, this->CondOutletNodeNum, this->CDPlantLoc);
        PlantUtilities::PullCompInterconnectTrigger(
            state, this->CWPlantLoc, this->CondMassFlowIndex, this->CDPlantLoc, DataPlant::CriteriaType::MassFlowRate, this->CondMassFlowRate);

        if (this->CondMassFlowRate < DataBranchAirLoopPlant::MassFlowTolerance) {
            MyLoad = 0.0;
            this->Power = standbyPower;
            this->AmbientZoneGain = standbyPower;
            this->EvapMassFlowRate = 0.0;
            PlantUtilities::SetComponentFlowRate(state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
            return;
        }
    }
    Real64 EvapOutletTempSetPoint(0.0); // Evaporator outlet temperature setpoint [C]
    switch (this->CWPlantLoc.loop->LoopDemandCalcScheme) {
    case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
        if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) ||
            (this->CWPlantLoc.comp->CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased) ||
            (state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint != DataLoopNode::SensedNodeFlagValue)) {
            // there will be a valid setpoint on outlet
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint;
        } else { // use plant loop overall setpoint
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPoint;
        }
    } break;
    case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
        if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) ||
            (this->CWPlantLoc.comp->CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased) ||
            (state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi != DataLoopNode::SensedNodeFlagValue)) {
            // there will be a valid setpoint on outlet
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi;
        } else { // use plant loop overall setpoint
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->CWPlantLoc.loop->TempSetPointNodeNum).TempSetPointHi;
        }
    } break;
    default: {
        assert(false);
    } break;
    }
// Revisit
#if 0
        //        // If there is a fault of Chiller SWT Sensor
//        if (this->FaultyChillerSWTFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) && (!state.dataGlobal->KickOffSimulation)) {
//            int FaultIndex = this->FaultyChillerSWTIndex;
//            Real64 EvapOutletTempSetPoint_ff = EvapOutletTempSetPoint;
//
//            // calculate the sensor offset using fault information
//            this->FaultyChillerSWTOffset = state.dataFaultsMgr->FaultsChillerSWTSensor(FaultIndex).CalFaultOffsetAct(state);
//            // update the EvapOutletTempSetPoint
//            EvapOutletTempSetPoint =
//                    max(this->TempLowLimitEvapOut,
//                        min(state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp, EvapOutletTempSetPoint_ff - this->FaultyChillerSWTOffset));
//            this->FaultyChillerSWTOffset = EvapOutletTempSetPoint_ff - EvapOutletTempSetPoint;
//        }
#endif // 0
       // When implemented, TODO: correct temperature if using heat recovery

    this->EvapMassFlowRate = state.dataLoopNodes->Node(this->EvapInletNodeNum).MassFlowRate;
    // If some other component set the flow to 0, no reason to continue with calculations.
    if (this->EvapMassFlowRate == 0.0) {
        MyLoad = 0.0;
        return;
    }

    Real64 CpEvap = this->CWPlantLoc.loop->glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp, RoutineName);

    // Calculate mass flow rate based on MyLoad (TODO: then adjust it after determining if chiller can meet the load)
    this->findEvaporatorMassFlowRate(state, MyLoad, CpEvap);

    // Available chiller capacity is capacity at the highest sequence number; i.e. max chiller capacity
    const Real64 maximumChillerCap = this->Representation->performance.performance_map_cooling
                                         .calculate_performance(this->EvapVolFlowRate,
                                                                this->EvapOutletTemp + Constant::Kelvin,
                                                                this->CondVolFlowRate,
                                                                this->CondInletTemp + Constant::Kelvin,
                                                                this->MaxSequenceNumber,
                                                                this->InterpolationType)
                                         .net_evaporator_capacity;
    const Real64 minimumChillerCap = this->Representation->performance.performance_map_cooling
                                         .calculate_performance(this->EvapVolFlowRate,
                                                                this->EvapOutletTemp + Constant::Kelvin,
                                                                this->CondVolFlowRate,
                                                                this->CondInletTemp + Constant::Kelvin,
                                                                this->MinSequenceNumber,
                                                                this->InterpolationType)
                                         .net_evaporator_capacity;
    // Part load ratio based on load and available chiller capacity; cap at max P.L.R. (can be >1)
    this->ChillerPartLoadRatio = (maximumChillerCap > 0) ? max(0.0, std::abs(MyLoad) / maximumChillerCap) : 0.0;
    // Minimum capacity ratio, under which cycling occurs
    this->MinPartLoadRat = (maximumChillerCap > 0) ? minimumChillerCap / maximumChillerCap : 0.0;
    Real64 partLoadSeqNum{0.};

    // Chiller may be operating in one of three modes: cycling, modulating, or full capacity
    if (this->ChillerPartLoadRatio < this->MinPartLoadRat) // Cycling
    {
        this->ChillerCyclingRatio = this->ChillerPartLoadRatio / this->MinPartLoadRat;
        partLoadSeqNum = this->MinSequenceNumber;
    } else if (this->ChillerPartLoadRatio < 1.0) // Modulating
    {
        // Use performance map to find the fractional sequence number (which most closely matches our part load)
        Real64 constexpr accuracy{0.0001};
        int constexpr maxIter{500};
        int solFla{0};
        auto f = [MyLoad, this](Real64 partLoadSeqNum) {
            this->QEvaporator = this->Representation->performance.performance_map_cooling
                                    .calculate_performance(this->EvapVolFlowRate,
                                                           this->EvapOutletTemp + Constant::Kelvin,
                                                           this->CondVolFlowRate,
                                                           this->CondInletTemp + Constant::Kelvin,
                                                           partLoadSeqNum,
                                                           this->InterpolationType)
                                    .net_evaporator_capacity;
            return std::abs(MyLoad) - this->QEvaporator;
        };
        // Iteratively calculate this->QEvaporator by modulating partLoadSeqNum, ending at Q_Evaporator(partLoadSeqNum)
        General::SolveRoot(state, accuracy, maxIter, solFla, partLoadSeqNum, f, this->MinSequenceNumber, this->MaxSequenceNumber);
    } else // Full capacity: std::abs(MyLoad) > this->QEvaporator
    {
        this->QEvaporator = maximumChillerCap;
        partLoadSeqNum = this->MaxSequenceNumber;
        // SolveRoot stuff for eventual flow rate (can always calculate Ts if you have MFR and capacity)
        // recursion? Revisit.
        findEvaporatorMassFlowRate(state, this->QEvaporator, CpEvap);
        // if MFR changes, recalculate chiller capacity.
        // repeat until load <= capacity
    }

    // Use performance map to get the rest of results at new sequence number
    auto lookupVariablesCooling = // This is a struct returned by value, relying on RVO (THIS_AUTO_OK)
        this->Representation->performance.performance_map_cooling.calculate_performance(this->EvapVolFlowRate,
                                                                                        this->EvapOutletTemp + Constant::Kelvin,
                                                                                        this->CondVolFlowRate,
                                                                                        this->CondInletTemp + Constant::Kelvin,
                                                                                        partLoadSeqNum,
                                                                                        this->InterpolationType);
    this->QEvaporator = lookupVariablesCooling.net_evaporator_capacity * this->ChillerCyclingRatio;

    Real64 evapDeltaTemp = this->QEvaporator / this->EvapMassFlowRate / CpEvap;
    this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - evapDeltaTemp;

    // TODO: Revisit fault
#if 0
        // If there is a fault of Chiller SWT Sensor
        if (this->FaultyChillerSWTFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation) && (this->EvapMassFlowRate > 0)) {
            // calculate directly affected variables at faulty case: EvapOutletTemp, EvapMassFlowRate, QEvaporator
            int FaultIndex = this->FaultyChillerSWTIndex;
            bool VarFlowFlag = false;
            state.dataFaultsMgr->FaultsChillerSWTSensor(FaultIndex)
                    .CalFaultChillerSWT(VarFlowFlag,
                                        this->FaultyChillerSWTOffset,
                                        Cp,
                                        state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp,
                                        this->EvapOutletTemp,
                                        this->EvapMassFlowRate,
                                        this->QEvaporator);
            // update corresponding variables at faulty case
        }
#endif // 0

    Real64 cd = this->Representation->performance.cycling_degradation_coefficient;
    Real64 cyclingFactor{(1.0 - cd) + (cd * this->ChillerCyclingRatio)};
    Real64 runtimeFactor{this->ChillerCyclingRatio / cyclingFactor};
    this->Power = lookupVariablesCooling.input_power * runtimeFactor + ((1 - this->ChillerCyclingRatio) * standbyPower);
    this->QCondenser = lookupVariablesCooling.net_condenser_capacity * this->ChillerCyclingRatio;
    this->QOilCooler = lookupVariablesCooling.oil_cooler_heat;
    this->QAuxiliary = lookupVariablesCooling.auxiliary_heat;
    Real64 QExternallyCooled{0.0};
    if (this->OilCoolerInletNode) {
        QExternallyCooled += this->QOilCooler;
    }
    if (this->AuxiliaryHeatInletNode) {
        QExternallyCooled += this->QAuxiliary;
    }
    // Energy balance on the chiller system gives the amount of heat lost to the ambient zone
    this->AmbientZoneGain = this->QEvaporator + this->Power - (this->QCondenser + QExternallyCooled);

    Real64 CpCond = this->CDPlantLoc.loop->glycol->getSpecificHeat(state, condInletTemp, RoutineName);
    this->CondOutletTemp = this->QCondenser / this->CondMassFlowRate / CpCond + condInletTemp;

    // Oil cooler and Auxiliary Heat delta-T calculations
    if (this->OilCoolerInletNode) {
        Real64 oilCoolerDeltaTemp = 0.0;
        PlantUtilities::SetComponentFlowRate(
            state, this->OilCoolerMassFlowRate, this->OilCoolerInletNode, this->OilCoolerOutletNode, this->OCPlantLoc);

        Real64 CpOilCooler =
            this->OCPlantLoc.loop->glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->OilCoolerInletNode).Temp, RoutineName);

        if (this->OilCoolerMassFlowRate != 0.0) {
            oilCoolerDeltaTemp = this->QOilCooler / (this->OilCoolerMassFlowRate * CpOilCooler);
        } else {
            oilCoolerDeltaTemp = 0.0;
        }
        state.dataLoopNodes->Node(this->OilCoolerOutletNode).Temp = state.dataLoopNodes->Node(this->OilCoolerInletNode).Temp - oilCoolerDeltaTemp;
    }
    if (this->AuxiliaryHeatInletNode) {
        Real64 auxiliaryDeltaTemp = 0.0;
        PlantUtilities::SetComponentFlowRate(
            state, this->AuxiliaryMassFlowRate, this->AuxiliaryHeatInletNode, this->AuxiliaryHeatOutletNode, this->AHPlantLoc);

        Real64 CpAux =
            this->AHPlantLoc.loop->glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->AuxiliaryHeatInletNode).Temp, RoutineName);

        if (this->AuxiliaryMassFlowRate != 0.0) {
            auxiliaryDeltaTemp = this->QAuxiliary / (this->AuxiliaryMassFlowRate * CpAux);
        } else {
            auxiliaryDeltaTemp = 0.0;
        }
        state.dataLoopNodes->Node(this->AuxiliaryHeatOutletNode).Temp =
            state.dataLoopNodes->Node(this->AuxiliaryHeatInletNode).Temp - auxiliaryDeltaTemp;
    }
}

void ASHRAE205ChillerSpecs::update(EnergyPlusData &state, Real64 const MyLoad, bool const RunFlag)
{
    if (MyLoad >= 0.0 || !RunFlag) { // Chiller not running so pass inlet states to outlet states
        // Set node temperatures
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
        state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
        if (this->OilCoolerInletNode) {
            state.dataLoopNodes->Node(this->OilCoolerOutletNode).Temp = state.dataLoopNodes->Node(this->OilCoolerInletNode).Temp;
        }
        if (this->AuxiliaryHeatInletNode) {
            state.dataLoopNodes->Node(this->AuxiliaryHeatOutletNode).Temp = state.dataLoopNodes->Node(this->AuxiliaryHeatInletNode).Temp;
        }

        this->ChillerPartLoadRatio = 0.0;
        this->ChillerCyclingRatio = 0.0;
        this->ChillerFalseLoadRate = 0.0;
        this->ChillerFalseLoad = 0.0;
        this->QEvaporator = 0.0;
        this->QCondenser = 0.0;
        this->Energy = 0.0;
        this->EvapEnergy = 0.0;
        this->CondEnergy = 0.0;
        this->QOilCooler = 0.0;
        this->QAuxiliary = 0.0;
        this->OilCoolerEnergy = 0.0;
        this->AuxiliaryEnergy = 0.0;
        this->EvapInletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
        this->CondInletTemp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
        this->CondOutletTemp = state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp;
        this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp;
        this->ActualCOP = 0.0;

    } else { // Chiller is running, so pass calculated values
        // Set node temperatures
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp = this->EvapOutletTemp;
        state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp = this->CondOutletTemp;
        // Set node flow rates;  for these load based models
        // assume that sufficient evaporator flow rate is available
        this->EvapEnergy = this->QEvaporator * state.dataHVACGlobal->TimeStepSysSec;
        this->CondEnergy = this->QCondenser * state.dataHVACGlobal->TimeStepSysSec;
        this->OilCoolerEnergy = this->QOilCooler * state.dataHVACGlobal->TimeStepSysSec;
        this->AuxiliaryEnergy = this->QAuxiliary * state.dataHVACGlobal->TimeStepSysSec;
        this->EvapInletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
        this->CondInletTemp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
        this->CondOutletTemp = state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp;
        if (this->Power != 0.0) {
            this->ActualCOP = this->QEvaporator / this->Power;
        } else {
            this->ActualCOP = 0.0;
        }
    }

    // Calculate in case of standby power
    this->AmbientZoneGainEnergy = this->AmbientZoneGain * state.dataHVACGlobal->TimeStepSysSec;
    this->Energy = this->Power * state.dataHVACGlobal->TimeStepSysSec;
}

void ASHRAE205ChillerSpecs::simulate(
    EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag)
{
    if (calledFromLocation.loopNum == this->CWPlantLoc.loopNum) {
        this->initialize(state, RunFlag, CurLoad);
        this->calculate(state, CurLoad, RunFlag);
        this->update(state, CurLoad, RunFlag);
    } else if (calledFromLocation.loopNum == this->CDPlantLoc.loopNum) {
        DataPlant::LoopSideLocation LoopSide = this->CDPlantLoc.loopSideNum;
        PlantUtilities::UpdateChillerComponentCondenserSide(state,
                                                            calledFromLocation.loopNum,
                                                            LoopSide,
                                                            DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205,
                                                            this->CondInletNodeNum,
                                                            this->CondOutletNodeNum,
                                                            this->QCondenser,
                                                            this->CondInletTemp,
                                                            this->CondOutletTemp,
                                                            this->CondMassFlowRate,
                                                            FirstHVACIteration);
    }
}

void ASHRAE205ChillerSpecs::getDesignCapacities(
    [[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad)
{
    if (calledFromLocation.loopNum == this->CWPlantLoc.loopNum) {
        MinLoad = this->Representation->performance.performance_map_cooling
                      .calculate_performance(this->EvapVolFlowRate,
                                             this->TempRefEvapOut + Constant::Kelvin,
                                             this->CondVolFlowRate,
                                             this->TempRefCondIn + Constant::Kelvin,
                                             this->MinSequenceNumber,
                                             this->InterpolationType)
                      .net_evaporator_capacity;
        MaxLoad = this->RefCap;
        OptLoad = MaxLoad;
    } else {
        MinLoad = 0.0;
        MaxLoad = 0.0;
        OptLoad = 0.0;
    }
}

} // namespace EnergyPlus::ChillerElectricASHRAE205
