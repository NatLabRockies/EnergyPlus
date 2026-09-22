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
#include <format>
#include <string>
#include <string_view>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/LiquidCooledITEPlantComponents.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::LiquidCooledITEPlantComponents {

PlantComponent *CoilCoolingITEColdPlateData::factory(EnergyPlusData &state, const std::string &objectName)
{
    if (state.dataLiquidCooledITE->getInputs) {
        CoilCoolingITEColdPlateData::processInputForCoilCoolingITEColdPlate(state);
        state.dataLiquidCooledITE->getInputs = false;
    }
    for (auto &coldPlate : state.dataLiquidCooledITE->coldPlates) {
        if (coldPlate.name == Util::makeUPPER(objectName)) {
            return &coldPlate;
        }
    }
    ShowFatalError(state, std::format("Coil:Cooling:ITE:ColdPlate factory: could not find object named: {}", objectName));
    return nullptr;
}

void CoilCoolingITEColdPlateData::setupOutputVariables(EnergyPlusData &state)
{
    SetupOutputVariable(state,
                        "Coil Cooling ITE Cold Plate Heat Transfer Rate",
                        Constant::Units::W,
                        this->heatRemovedByFluid,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Coil Cooling ITE Cold Plate Heat Transfer Energy",
                        Constant::Units::J,
                        this->heatRemovedByFluidEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->name);
    SetupOutputVariable(state,
                        "Coil Cooling ITE Cold Plate Zone Heat Gain Rate",
                        Constant::Units::W,
                        this->zoneHeatGainRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Coil Cooling ITE Cold Plate Zone Heat Gain Energy",
                        Constant::Units::J,
                        this->zoneHeatGainEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->name);
    SetupOutputVariable(state,
                        "Coil Cooling ITE Cold Plate Case Temperature",
                        Constant::Units::C,
                        this->caseTemperature,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Coil Cooling ITE Cold Plate Inlet Temperature",
                        Constant::Units::C,
                        this->inletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Coil Cooling ITE Cold Plate Outlet Temperature",
                        Constant::Units::C,
                        this->outletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Coil Cooling ITE Cold Plate Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->massFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Coil Cooling ITE Cold Plate Auxiliary Electric Power",
                        Constant::Units::W,
                        this->auxElecPower,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Coil Cooling ITE Cold Plate Auxiliary Electric Energy",
                        Constant::Units::J,
                        this->auxElecEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->name,
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Cooling,
                        this->endUseSubcategory);
}

void CoilCoolingITEColdPlateData::simulate(EnergyPlusData &state,
                                           [[maybe_unused]] const PlantLocation &calledFromLocation,
                                           [[maybe_unused]] bool FirstHVACIteration,
                                           [[maybe_unused]] Real64 &CurLoad,
                                           [[maybe_unused]] bool RunFlag)
{
    if (this->oneTimeInitFlag) {
        this->oneTimeInit(state);
    }

    if (this->myEnvrnFlag && state.dataGlobal->BeginEnvrnFlag && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
        static constexpr std::string_view routineName("CoilCoolingITEColdPlateData::simulate");
        Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, state.dataLoopNodes->Node(this->inletNode).Temp, routineName);
        Real64 const maxMdot = (this->maximumFlowRate > 0.0 ? this->maximumFlowRate : this->nominalFlowRate) * rho;
        PlantUtilities::InitComponentNodes(state, 0.0, maxMdot, this->inletNode, this->outletNode);
        this->heatRemovedByFluid = 0.0;
        this->heatRemovedByFluidEnergy = 0.0;
        this->zoneHeatGainRate = 0.0;
        this->zoneHeatGainEnergy = 0.0;
        this->caseTemperature = 0.0;
        this->outletTemp = state.dataLoopNodes->Node(this->inletNode).Temp;
        this->massFlowRate = 0.0;
        this->auxElecPower = 0.0;
        this->auxElecEnergy = 0.0;
        this->myEnvrnFlag = false;
    }
    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->myEnvrnFlag = true;
    }

    this->doPhysics(state);
    this->report(state);
}

void CoilCoolingITEColdPlateData::onInitLoopEquip(EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation)
{
    if (this->myPlantScanFlag) {
        bool errFlag = false;
        PlantUtilities::ScanPlantLoopsForObject(state, this->name, DataPlant::PlantEquipmentType::CoilCoolingITEColdPlate, this->plantLoc, errFlag);
        if (errFlag) {
            ShowFatalError(state, std::format("Coil:Cooling:ITE:ColdPlate: plant loop scan failed for object \"{}\"", this->name));
        }
        this->myPlantScanFlag = false;
    }
    if (this->mySizingFlag) {
        this->sizeColdPlate(state);
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            this->mySizingFlag = false;
        }
    }
}

void CoilCoolingITEColdPlateData::oneTimeInit(EnergyPlusData &state)
{
    this->oneTimeInitFlag = false;
    this->setupOutputVariables(state);
}

Real64 CoilCoolingITEColdPlateData::getThermalResistanceModifier(EnergyPlusData &state, Real64 flowRatio) const
{
    return (this->thermalResistanceModifierCurveIndex > 0) ? Curve::CurveValue(state, this->thermalResistanceModifierCurveIndex, flowRatio) : 1.0;
}

Real64 CoilCoolingITEColdPlateData::getDesignLoad(EnergyPlusData &state, Real64 inletFluidTemperature, Real64 outletFluidTemperature)
{
    Real64 adjustedThermalResistance = this->thermalResistance * this->getThermalResistanceModifier(state, 1.0);

    if (adjustedThermalResistance <= 0.0) {
        ShowFatalError(state,
                       std::format("Thermal resistance for the Coil:Cooling:ITE:ColdPlate \"{}\" is 0 or negative for a flow ratio of 1.0. "
                                   "Check the nominal thermal resistance and the thermal resistance modifier curve.",
                                   this->name));
    }

    if (this->thermalResistanceMethod == ThermalResistanceMethod::Standard) {
        return (this->targetCaseOperatingTemperature - inletFluidTemperature) / adjustedThermalResistance;
    }

    if (this->thermalResistanceMethod == ThermalResistanceMethod::LMTD) {
        if (outletFluidTemperature <= inletFluidTemperature) {
            ShowFatalError(state,
                           std::format("Coil:Cooling:ITE:ColdPlate \"{}\": Plant loop design temperature rise must be greater than 0 when using "
                                       "the LMTD method. Check the PlantSizing object.",
                                       this->name));
        }
        if (this->targetCaseOperatingTemperature <= outletFluidTemperature) {
            ShowFatalError(state,
                           std::format("Coil:Cooling:ITE:ColdPlate \"{}\": Target Case Operating Temperature ({:.2f} C) must be greater than "
                                       "the plant loop design outlet temperature ({:.2f} C) when using the LMTD method.",
                                       this->name,
                                       this->targetCaseOperatingTemperature,
                                       outletFluidTemperature));
        }
        Real64 logMeanTemperatureDifference =
            (outletFluidTemperature - inletFluidTemperature) / std::log((this->targetCaseOperatingTemperature - inletFluidTemperature) /
                                                                        (this->targetCaseOperatingTemperature - outletFluidTemperature));
        return logMeanTemperatureDifference / adjustedThermalResistance;
    }

    ShowFatalError(state, std::format("CoilCoolingITEColdPlateData: Unknown thermal resistance method for object \"{}\"", this->name));
    return 0.0;
}

void CoilCoolingITEColdPlateData::sizeColdPlate(EnergyPlusData &state)
{
    static constexpr std::string_view routineName("sizeColdPlate");

    if (this->nominalFlowRate == DataSizing::AutoSize) {
        int const plantSizingNum = this->plantLoc.loop->PlantSizNum;

        if (plantSizingNum > 0) {
            auto &plantSizing = state.dataSize->PlantSizData(plantSizingNum);
            if (plantSizing.DeltaT <= 0.0) {
                ShowFatalError(state,
                               std::format("sizeColdPlate: Plant loop design temperature rise must be greater than 0 for object=\"{}\"", this->name));
            }
            Real64 const cp = this->plantLoc.loop->glycol->getSpecificHeat(state, plantSizing.ExitTemp, routineName);
            Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, plantSizing.ExitTemp, routineName);
            // ExitTemp is the loop supply (inlet to cold plate); ExitTemp+DeltaT is the return (outlet from cold plate)
            Real64 designLoad = getDesignLoad(state, plantSizing.ExitTemp, plantSizing.ExitTemp + plantSizing.DeltaT);

            if (designLoad > 0) {
                this->nominalFlowRate = designLoad / (plantSizing.DeltaT * cp * rho);
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    BaseSizer::reportSizerOutput(
                        state, "Coil:Cooling:ITE:ColdPlate", this->name, "Design Size Fluid Flow Rate [m3/s]", this->nominalFlowRate);
                }
            } else {
                ShowFatalError(state, std::format("sizeColdPlate: Design load must be greater than 0 for object=\"{}\"", this->name));
            }
        } else {
            ShowFatalError(state, std::format("sizeColdPlate: Missing plant sizing data for object=\"{}\"", this->name));
        }
    }

    if (this->maximumFlowRate == DataSizing::AutoSize) {
        this->maximumFlowRate = this->nominalFlowRate;
    }

    PlantUtilities::RegisterPlantCompDesignFlow(state, this->inletNode, this->nominalFlowRate);
}

void CoilCoolingITEColdPlateData::doPhysics(EnergyPlusData &state)
{
    Real64 massFlowRate = 0.0;

    // Check if the component is running
    Real64 running = (this->availabilitySchedule != nullptr) ? this->availabilitySchedule->getCurrentVal() : 1.0;

    // Auxiliary power runs whenever the cold plate is available, regardless of IT load
    this->auxElecPower = (running > 0.0) ? this->auxPower : 0.0;

    // No load or not running
    if (this->actualLoad <= 0.0 || running <= 0.0) {
        this->heatRemovedByFluid = 0.0;
        this->zoneHeatGainRate = 0.0;
        this->massFlowRate = 0.0;
        this->caseTemperature = 0.0;
        this->inletTemp = state.dataLoopNodes->Node(this->inletNode).Temp;
        this->outletTemp = this->inletTemp;
        PlantUtilities::SetComponentFlowRate(state, massFlowRate, this->inletNode, this->outletNode, this->plantLoc);
        return;
    }

    this->inletTemp = state.dataLoopNodes->Node(this->inletNode).Temp;
    Real64 const inletTemp = this->inletTemp;
    Real64 const cp = this->plantLoc.loop->glycol->getSpecificHeat(state, inletTemp, "CoilCoolingITEColdPlateData::doPhysics");
    Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, inletTemp, "CoilCoolingITEColdPlateData::doPhysics");

    Real64 const nominalMassFlowRate = this->nominalFlowRate * rho;
    Real64 const maximumMassFlowRate = (this->maximumFlowRate > 0.0) ? this->maximumFlowRate * rho : nominalMassFlowRate;

    // Handle flow based on flow lock status
    if (this->plantLoc.side->FlowLock == DataPlant::FlowLock::Unlocked) {
        bool const variableFlowIllDefined =
            (this->thermalResistanceMethod == ThermalResistanceMethod::Standard && this->thermalResistanceModifierCurveIndex == 0);
        if (this->flowMode == DataPlant::FlowMode::Constant || variableFlowIllDefined) {
            massFlowRate = nominalMassFlowRate;
        } else { // Variable flow mode
            // returns the load achievable at a given mass flow rate and target temperature
            auto loadCalculated = [this, &state, cp, inletTemp, nominalMassFlowRate](Real64 const massFlow, Real64 const targetTemp) {
                Real64 thermalResistance = this->thermalResistance * this->getThermalResistanceModifier(state, massFlow / nominalMassFlowRate);
                return (this->thermalResistanceMethod == ThermalResistanceMethod::LMTD)
                           ? massFlow * cp * (1.0 - std::exp(-1.0 / (massFlow * cp * thermalResistance))) * (targetTemp - inletTemp)
                           : (targetTemp - inletTemp) / thermalResistance;
            };

            // First: solve for flow that meets the target operating temperature
            auto targetResidual = [&loadCalculated, this](Real64 const massFlow) {
                return loadCalculated(massFlow, this->targetCaseOperatingTemperature) - this->actualLoad;
            };
            int SolFla;
            General::SolveRoot(state, 1.0e-3, 500, SolFla, massFlowRate, targetResidual, 0.0, maximumMassFlowRate);

            // If no solution, try again allowing the case to rise to the maximum temperature
            if (SolFla < 0) {
                auto maxTempResidual = [&loadCalculated, this](Real64 const massFlow) {
                    return loadCalculated(massFlow, this->maximumCaseTemperature) - this->actualLoad;
                };
                General::SolveRoot(state, 1.0e-3, 500, SolFla, massFlowRate, maxTempResidual, 0.0, maximumMassFlowRate);

                // If still no solution, use maximum flow rate, excess heat that cannot be removed will be added to the zone heat gain
                if (SolFla < 0) {
                    massFlowRate = maximumMassFlowRate;
                }
            }
        }
    } else {
        // Flow is locked, use the current flow rate
        massFlowRate = state.dataLoopNodes->Node(this->inletNode).MassFlowRate;
    }
    PlantUtilities::SetComponentFlowRate(state, massFlowRate, this->inletNode, this->outletNode, this->plantLoc);
    this->massFlowRate = massFlowRate;

    // Flow is known check cold plate maximum heat transfer rate
    Real64 thermalResistance = this->thermalResistance * this->getThermalResistanceModifier(state, massFlowRate / nominalMassFlowRate);
    Real64 maxHeatTransferRate =
        (this->thermalResistanceMethod == ThermalResistanceMethod::LMTD)
            ? massFlowRate * cp * (1.0 - std::exp(-1.0 / (massFlowRate * cp * thermalResistance))) * (this->maximumCaseTemperature - inletTemp)
            : (this->maximumCaseTemperature - inletTemp) / thermalResistance;

    // Determine how much of the load is met by the fluid and how much spills to the zone
    this->heatRemovedByFluid = std::min(this->actualLoad, maxHeatTransferRate);
    this->zoneHeatGainRate = this->actualLoad - this->heatRemovedByFluid;

    // Chip case temperature
    if (this->thermalResistanceMethod == ThermalResistanceMethod::LMTD && massFlowRate > 0.0) {
        Real64 const effectiveness = 1.0 - std::exp(-1.0 / (massFlowRate * cp * thermalResistance));
        this->caseTemperature = inletTemp + this->heatRemovedByFluid / (massFlowRate * cp * effectiveness);
    } else {
        this->caseTemperature = inletTemp + this->heatRemovedByFluid * thermalResistance;
    }

    // Store outlet temperature — node update happens in report() via SafeCopyPlantNode
    this->outletTemp = (massFlowRate > 0.0) ? inletTemp + this->heatRemovedByFluid / (massFlowRate * cp) : inletTemp;
}

void CoilCoolingITEColdPlateData::report(EnergyPlusData &state)
{
    Real64 const reportingInterval = state.dataHVACGlobal->TimeStepSysSec;

    this->heatRemovedByFluidEnergy = this->heatRemovedByFluid * reportingInterval;
    this->auxElecEnergy = this->auxElecPower * reportingInterval;
    this->zoneHeatGainEnergy = this->zoneHeatGainRate * reportingInterval;

    PlantUtilities::SafeCopyPlantNode(state, this->inletNode, this->outletNode);
    state.dataLoopNodes->Node(this->outletNode).Temp = this->outletTemp;
}

void CoilCoolingITEColdPlateData::processInputForCoilCoolingITEColdPlate(EnergyPlusData &state)
{
    bool errorsFound = false;
    const std::string cCurrentModuleObject = "Coil:Cooling:ITE:ColdPlate";
    auto *ip = state.dataInputProcessing->inputProcessor.get();

    auto const instances = ip->epJSON.find(cCurrentModuleObject);
    if (instances == ip->epJSON.end()) {
        return;
    }
    auto const &schemaProps = ip->getObjectSchemaProps(state, cCurrentModuleObject);
    state.dataLiquidCooledITE->coldPlates.reserve(instances.value().size());

    for (auto instance = instances.value().begin(); instance != instances.value().end(); ++instance) {
        auto const &fields = instance.value();
        std::string const thisObjectName = instance.key();
        ip->markObjectAsUsed(cCurrentModuleObject, thisObjectName);

        CoilCoolingITEColdPlateData thisColdPlate;
        thisColdPlate.name = Util::makeUPPER(thisObjectName);

        // Get schedule
        if (fields.contains("availability_schedule")) {
            thisColdPlate.availabilitySchedule = Sched::GetSchedule(state, fields.at("availability_schedule").get<std::string>());
        }

        // Get the numeric fields
        thisColdPlate.thermalResistance = ip->getRealFieldValue(fields, schemaProps, "nominal_thermal_resistance");
        thisColdPlate.maximumCaseTemperature = ip->getRealFieldValue(fields, schemaProps, "maximum_case_temperature");
        thisColdPlate.nominalFlowRate = ip->getRealFieldValue(fields, schemaProps, "nominal_liquid_flow_rate");
        thisColdPlate.maximumFlowRate = ip->getRealFieldValue(fields, schemaProps, "maximum_liquid_flow_rate");
        thisColdPlate.targetCaseOperatingTemperature = fields.contains("target_case_operating_temperature")
                                                           ? ip->getRealFieldValue(fields, schemaProps, "target_case_operating_temperature")
                                                           : thisColdPlate.maximumCaseTemperature;
        if (thisColdPlate.targetCaseOperatingTemperature > thisColdPlate.maximumCaseTemperature) {
            ShowSevereError(
                state,
                std::format("{}: Target Case Operating Temperature ({:.2f} C) exceeds Maximum Case Temperature ({:.2f} C) for object \"{}\"",
                            cCurrentModuleObject,
                            thisColdPlate.targetCaseOperatingTemperature,
                            thisColdPlate.maximumCaseTemperature,
                            thisColdPlate.name));
            errorsFound = true;
        }
        thisColdPlate.auxPower = ip->getRealFieldValue(fields, schemaProps, "auxiliary_electric_power");
        thisColdPlate.endUseSubcategory = ip->getAlphaFieldValue(fields, schemaProps, "end_use_subcategory");

        // Get the curve (optional — absent or blank means modifier = 1.0)
        if (fields.contains("thermal_resistance_modifier_curve_name")) {
            std::string const thermalResistanceModifierCurveName =
                Util::makeUPPER(fields.at("thermal_resistance_modifier_curve_name").get<std::string>());
            if (!thermalResistanceModifierCurveName.empty()) {
                thisColdPlate.thermalResistanceModifierCurveIndex = Curve::GetCurveIndex(state, thermalResistanceModifierCurveName);
                if (thisColdPlate.thermalResistanceModifierCurveIndex == 0) {
                    ShowSevereError(state,
                                    std::format("{}: curve \"{}\" not found for object \"{}\"",
                                                cCurrentModuleObject,
                                                thermalResistanceModifierCurveName,
                                                thisColdPlate.name));
                    errorsFound = true;
                }
            }
        }

        // Get enums
        thisColdPlate.thermalResistanceMethod = (ip->getAlphaFieldValue(fields, schemaProps, "thermal_resistance_method") == "LMTD")
                                                    ? ThermalResistanceMethod::LMTD
                                                    : ThermalResistanceMethod::Standard;
        thisColdPlate.flowMode = (ip->getAlphaFieldValue(fields, schemaProps, "flow_mode") == "VARIABLEFLOW") ? DataPlant::FlowMode::Variable
                                                                                                              : DataPlant::FlowMode::Constant;

        // Combination of flow mode and thermal resistance method that triggers constant flow
        if (thisColdPlate.flowMode == DataPlant::FlowMode::Variable && thisColdPlate.thermalResistanceMethod == ThermalResistanceMethod::Standard &&
            thisColdPlate.thermalResistanceModifierCurveIndex == 0) {
            ShowWarningError(
                state,
                std::format(
                    "{}: object \"{}\" specifies VariableFlow with the Standard thermal resistance method and no modifier curve. "
                    "In this combination the case temperature does not depend on flow rate, so the cold plate will operate at the nominal flow rate.",
                    cCurrentModuleObject,
                    thisColdPlate.name));
        }

        // Get the node names and validate them
        std::string const inletNodeName = ip->getAlphaFieldValue(fields, schemaProps, "inlet_node");
        std::string const outletNodeName = ip->getAlphaFieldValue(fields, schemaProps, "outlet_node");

        thisColdPlate.inletNode = Node::GetOnlySingleNode(state,
                                                          inletNodeName,
                                                          errorsFound,
                                                          Node::ConnectionObjectType::CoilCoolingITEColdPlate,
                                                          thisColdPlate.name,
                                                          Node::FluidType::Water,
                                                          Node::ConnectionType::Inlet,
                                                          Node::CompFluidStream::Primary,
                                                          Node::ObjectIsNotParent);
        thisColdPlate.outletNode = Node::GetOnlySingleNode(state,
                                                           outletNodeName,
                                                           errorsFound,
                                                           Node::ConnectionObjectType::CoilCoolingITEColdPlate,
                                                           thisColdPlate.name,
                                                           Node::FluidType::Water,
                                                           Node::ConnectionType::Outlet,
                                                           Node::CompFluidStream::Primary,
                                                           Node::ObjectIsNotParent);

        Node::TestCompSet(state, cCurrentModuleObject, thisColdPlate.name, inletNodeName, outletNodeName, "Liquid Coolant Nodes");

        // Get zone references
        std::string const zoneName = Util::makeUPPER(ip->getAlphaFieldValue(fields, schemaProps, "zone_name"));
        int const zoneIndex = Util::FindItemInList(zoneName, state.dataHeatBal->Zone);
        if (zoneIndex == 0) {
            ShowSevereError(state, std::format("{}: Zone \"{}\" not found for object \"{}\"", cCurrentModuleObject, zoneName, thisColdPlate.name));
            errorsFound = true;
        }

        state.dataLiquidCooledITE->coldPlates.push_back(thisColdPlate);

        auto &stored = state.dataLiquidCooledITE->coldPlates.back();

        // Set up zone heat gains
        if (zoneIndex > 0) {
            SetupZoneInternalGain(
                state, zoneIndex, stored.name, DataHeatBalance::IntGainType::CoilCoolingITEColdPlate, nullptr, &stored.zoneHeatGainRate);
        }
    }
    if (errorsFound) {
        ShowFatalError(state, std::format("Errors found in processing input for {}", cCurrentModuleObject));
    }
}

} // namespace EnergyPlus::LiquidCooledITEPlantComponents
