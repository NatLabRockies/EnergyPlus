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
    SetupOutputVariable(state,
                        "Coil Cooling ITE Cold Plate Effective Thermal Resistance",
                        Constant::Units::K_W,
                        this->effectiveThermalResistance,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
}

void CoilCoolingITEColdPlateData::simulate(EnergyPlusData &state,
                                           [[maybe_unused]] const PlantLocation &calledFromLocation,
                                           [[maybe_unused]] bool FirstHVACIteration,
                                           [[maybe_unused]] Real64 &CurLoad,
                                           [[maybe_unused]] bool RunFlag)
{
    if (this->myEnvrnFlag && state.dataGlobal->BeginEnvrnFlag) {
        this->setMassFlowRates(state);
        PlantUtilities::InitComponentNodes(state, 0.0, this->maximumMassFlowRate, this->inletNode, this->outletNode);
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

    this->sizeColdPlate(state);
}

void CoilCoolingITEColdPlateData::oneTimeInit_new(EnergyPlusData &state)
{
    this->setupOutputVariables(state);
}

Real64 CoilCoolingITEColdPlateData::getAdjustedThermalResistance(EnergyPlusData &state, Real64 flowRatio) const
{
    Real64 const modifier =
        (this->thermalResistanceModifierCurveIndex > 0) ? Curve::CurveValue(state, this->thermalResistanceModifierCurveIndex, flowRatio) : 1.0;
    return this->thermalResistance * modifier;
}

Real64 CoilCoolingITEColdPlateData::getReferenceTemperature(EnergyPlusData &state) const
{
    int const plantSizingNum = this->plantLoc.loop->PlantSizNum;
    return (plantSizingNum > 0) ? state.dataSize->PlantSizData(plantSizingNum).ExitTemp : Constant::CWInitConvTemp;
}

Real64 CoilCoolingITEColdPlateData::getDesignLoad(EnergyPlusData &state, Real64 inletFluidTemperature, Real64 outletFluidTemperature)
{
    Real64 adjustedThermalResistance = this->getAdjustedThermalResistance(state, 1.0);

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
        if (this->targetCaseOperatingTemperature <= outletFluidTemperature) {
            ShowFatalError(state,
                           std::format("Coil:Cooling:ITE:ColdPlate \"{}\": Target Case Operating Temperature ({:.2f} C) must be greater than "
                                       "the plant loop design outlet temperature ({:.2f} C) when using the LMTD method.",
                                       this->name,
                                       this->targetCaseOperatingTemperature,
                                       outletFluidTemperature));
        }
        Real64 const caseToOutlet = this->targetCaseOperatingTemperature - outletFluidTemperature;
        Real64 const caseToInlet = this->targetCaseOperatingTemperature - inletFluidTemperature;
        Real64 logMeanTemperatureDifference = (caseToOutlet - caseToInlet) / std::log(caseToOutlet / caseToInlet);
        return logMeanTemperatureDifference / adjustedThermalResistance;
    }

    ShowFatalError(state, std::format("CoilCoolingITEColdPlateData: Unknown thermal resistance method for object \"{}\"", this->name));
    return 0.0;
}

void CoilCoolingITEColdPlateData::sizeColdPlate(EnergyPlusData &state)
{
    static constexpr std::string_view routineName("sizeColdPlate");
    static constexpr std::string_view objectType("Coil:Cooling:ITE:ColdPlate");

    int const plantSizingNum = this->plantLoc.loop->PlantSizNum;

    // Nominal flow rate from the plant loop design conditions, also used for hard-sized comparison reporting
    Real64 designFlowRate = 0.0;
    Real64 designLoad = 0.0;
    if (plantSizingNum > 0) {
        auto const &plantSizing = state.dataSize->PlantSizData(plantSizingNum);
        if (plantSizing.DeltaT > 0.0) {
            Real64 const cp = this->plantLoc.loop->glycol->getSpecificHeat(state, plantSizing.ExitTemp, routineName);
            Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, this->getReferenceTemperature(state), routineName);
            // ExitTemp is the loop supply (inlet to cold plate); ExitTemp+DeltaT is the return (outlet from cold plate)
            designLoad = this->getDesignLoad(state, plantSizing.ExitTemp, plantSizing.ExitTemp + plantSizing.DeltaT);
            if (designLoad > 0.0) {
                designFlowRate = designLoad / (plantSizing.DeltaT * cp * rho);
            }
        }
    }

    if (this->nominalFlowRateWasAutoSized) {
        if (plantSizingNum == 0) {
            ShowFatalError(
                state,
                std::format("sizeColdPlate: A Sizing:Plant object could not be found for the loop that includes this Cold Plate object  =\"{}\"",
                            this->name));
        }
        if (designFlowRate <= 0.0) {
            ShowSevereError(state, std::format("{}: Coil:Cooling:ITE:ColdPlate \"{}\"", routineName, this->name));
            ShowContinueError(state,
                              std::format("Autosizing the nominal liquid flow rate requires a design load greater than 0 W, but the calculated "
                                          "design load is {:.2f} W.",
                                          designLoad));
            ShowContinueError(state,
                              std::format("Target Case Operating Temperature = {:.2f} C; plant loop design supply (cold plate inlet) temperature "
                                          "from Sizing:Plant = {:.2f} C.",
                                          this->targetCaseOperatingTemperature,
                                          state.dataSize->PlantSizData(plantSizingNum).ExitTemp));
            ShowContinueError(state,
                              "Check that the Target Case Operating Temperature is greater than the Sizing:Plant Design Loop Exit Temperature, "
                              "and that the cold plate thermal resistance (and thermal resistance modifier curve) is positive.");
            ShowContinueError(state, "Alternatively, enter a hard-sized Nominal Liquid Flow Rate instead of Autosize.");
            ShowFatalError(state, std::format("{}: Preceding condition causes termination.", routineName));
        }
        this->nominalFlowRate = designFlowRate;
        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
            BaseSizer::reportSizerOutput(state, objectType, this->name, "Design Size Nominal Liquid Flow Rate [m3/s]", this->nominalFlowRate);
        }
        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
            BaseSizer::reportSizerOutput(state, objectType, this->name, "Initial Design Size Nominal Liquid Flow Rate [m3/s]", this->nominalFlowRate);
        }
    } else if (state.dataPlnt->PlantFinalSizesOkayToReport) {
        if (designFlowRate > 0.0) {
            BaseSizer::reportSizerOutput(state,
                                         objectType,
                                         this->name,
                                         "Design Size Nominal Liquid Flow Rate [m3/s]",
                                         designFlowRate,
                                         "User-Specified Nominal Liquid Flow Rate [m3/s]",
                                         this->nominalFlowRate);
            if (state.dataGlobal->DisplayExtraWarnings &&
                (std::abs(designFlowRate - this->nominalFlowRate) / this->nominalFlowRate) > state.dataSize->AutoVsHardSizingThreshold) {
                ShowMessage(state, std::format("sizeColdPlate: Potential issue with equipment sizing for {}", this->name));
                ShowContinueError(state, std::format("User-Specified Nominal Liquid Flow Rate of {:.5f} [m3/s]", this->nominalFlowRate));
                ShowContinueError(state, std::format("differs from Design Size Nominal Liquid Flow Rate of {:.5f} [m3/s]", designFlowRate));
                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
            }
        } else {
            BaseSizer::reportSizerOutput(state, objectType, this->name, "User-Specified Nominal Liquid Flow Rate [m3/s]", this->nominalFlowRate);
        }
    }

    if (this->maximumFlowRateWasAutoSized) {
        this->maximumFlowRate = this->nominalFlowRate;
        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
            BaseSizer::reportSizerOutput(state, objectType, this->name, "Design Size Maximum Liquid Flow Rate [m3/s]", this->maximumFlowRate);
        }
        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
            BaseSizer::reportSizerOutput(state, objectType, this->name, "Initial Design Size Maximum Liquid Flow Rate [m3/s]", this->maximumFlowRate);
        }
    } else if (this->maximumFlowRate > 0.0) {
        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
            BaseSizer::reportSizerOutput(state, objectType, this->name, "User-Specified Maximum Liquid Flow Rate [m3/s]", this->maximumFlowRate);
        }
        if (this->nominalFlowRateWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize && this->maximumFlowRate < this->nominalFlowRate) {
            ShowSevereError(state,
                            std::format("{}: Maximum Liquid Flow Rate ({:.6f} m3/s) is less than the autosized Nominal Liquid Flow Rate "
                                        "({:.6f} m3/s) for object \"{}\"",
                                        objectType,
                                        this->maximumFlowRate,
                                        this->nominalFlowRate,
                                        this->name));
            ShowFatalError(state, "Preceding sizing errors cause program termination");
        }
    }

    PlantUtilities::RegisterPlantCompDesignFlow(state, this->inletNode, this->nominalFlowRate);
    this->setMassFlowRates(state);
}

void CoilCoolingITEColdPlateData::setMassFlowRates(EnergyPlusData &state)
{
    static constexpr std::string_view routineName("CoilCoolingITEColdPlateData::setMassFlowRates");
    Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, this->getReferenceTemperature(state), routineName);
    this->nominalMassFlowRate = this->nominalFlowRate * rho;
    this->maximumMassFlowRate = (this->maximumFlowRate > 0.0) ? this->maximumFlowRate * rho : this->nominalMassFlowRate;
}

void CoilCoolingITEColdPlateData::doPhysics(EnergyPlusData &state)
{
    Real64 massFlowRate = 0.0;

    Real64 const loadFromSchedule = (this->itLoadSchedule != nullptr) ? this->itLoadSchedule->getCurrentVal() : 0.0;
    if (loadFromSchedule < 0.0) {
        ShowFatalError(state,
                       std::format("Coil:Cooling:ITE:ColdPlate \"{}\": IT equipment load schedule value is negative ({:.2f} W). "
                                   "The load schedule must be non-negative.",
                                   this->name,
                                   loadFromSchedule));
    }
    if (this->loadFromITEquipment < 0.0) {
        ShowFatalError(state,
                       std::format("Coil:Cooling:ITE:ColdPlate \"{}\": IT equipment load from paired equipment is negative ({:.2f} W). "
                                   "This is an unexpected condition; check the paired ElectricEquipment:ITE:LiquidCooled object.",
                                   this->name,
                                   this->loadFromITEquipment));
    }
    Real64 const load = this->loadFromITEquipment + loadFromSchedule;

    // Check if the component is running
    Real64 const running = this->availabilitySchedule->getCurrentVal();

    // Auxiliary power runs whenever the cold plate is available, regardless of IT load
    this->auxElecPower = (running > 0.0) ? this->auxPower : 0.0;

    // No load or not running
    if (load <= 0.0 || running <= 0.0) {
        this->heatRemovedByFluid = 0.0;
        this->zoneHeatGainRate = load; // unmet load goes to zone when unavailable
        this->massFlowRate = 0.0;
        this->effectiveThermalResistance = 0.0;
        // When there is a load but no active cooling, report the maximum case temperature (same as the zero-flow guard below).
        // This is a reporting convention, not a prediction: without cooling a real chip would exceed this limit, and there is no
        // transient chip model (Tc = f(t, load)) to represent the thermal mass or the time taken to heat up.
        this->caseTemperature = (load > 0.0) ? this->maximumCaseTemperature : 0.0;
        this->inletTemp = state.dataLoopNodes->Node(this->inletNode).Temp;
        this->outletTemp = this->inletTemp;
        PlantUtilities::SetComponentFlowRate(state, massFlowRate, this->inletNode, this->outletNode, this->plantLoc);
        return;
    }

    this->inletTemp = state.dataLoopNodes->Node(this->inletNode).Temp;
    Real64 const inletTemp = this->inletTemp;
    Real64 const cp = this->plantLoc.loop->glycol->getSpecificHeat(state, inletTemp, "CoilCoolingITEColdPlateData::doPhysics");

    Real64 const nominalMassFlowRate = this->nominalMassFlowRate;
    Real64 const maximumMassFlowRate = this->maximumMassFlowRate;

    // Heat removal capacity at a given flow, thermal resistance and case temperature. The coolant cannot leave warmer than the case,
    // so the Standard method is limited by mdot * cp * (T_case - T_in); the LMTD effectiveness already enforces this limit.
    auto heatTransferCapacity = [this, cp, inletTemp](Real64 const massFlow, Real64 const resistance, Real64 const caseTemp) {
        if (massFlow <= 0.0) {
            return 0.0;
        }
        Real64 const capacity = (this->thermalResistanceMethod == ThermalResistanceMethod::LMTD)
                                    ? massFlow * cp * (1.0 - std::exp(-1.0 / (massFlow * cp * resistance))) * (caseTemp - inletTemp)
                                    : std::min(1.0 / resistance, massFlow * cp) * (caseTemp - inletTemp);
        // Coolant warmer than the case cannot remove heat; it must not add heat to the chip either
        return std::max(0.0, capacity);
    };

    // Determine the mass flow rate based on the flow mode and the load to be removed
    if (this->flowMode == DataPlant::FlowMode::Constant) {
        massFlowRate = nominalMassFlowRate;
    } else { // Variable flow mode
        // returns the load achievable at a given mass flow rate and target temperature
        auto loadCalculated = [this, &state, &heatTransferCapacity, nominalMassFlowRate](Real64 const massFlow, Real64 const targetTemp) {
            Real64 thermalResistance = this->getAdjustedThermalResistance(state, massFlow / nominalMassFlowRate);
            return heatTransferCapacity(massFlow, thermalResistance, targetTemp);
        };

        // Finds the flow that removes the load at the given case temperature; false if the load cannot be met at maximum flow.
        // The capacity is zero at zero flow, so the search always starts from zero flow.
        auto solveForFlow = [&](Real64 const targetTemp) {
            auto residual = [&loadCalculated, targetTemp, load](Real64 const massFlow) { return loadCalculated(massFlow, targetTemp) - load; };
            if (residual(maximumMassFlowRate) < 0.0) {
                return false;
            }
            int SolFla;
            General::SolveRoot(state, 1.0e-3, 500, SolFla, massFlowRate, residual, 0.0, maximumMassFlowRate);
            return SolFla >= 0;
        };

        // First meet the target operating temperature, then allow the case to rise to the maximum temperature;
        // otherwise use the maximum flow rate and the heat that cannot be removed is added to the zone heat gain
        if (!solveForFlow(this->targetCaseOperatingTemperature) && !solveForFlow(this->maximumCaseTemperature)) {
            massFlowRate = maximumMassFlowRate;
        }
    }
    PlantUtilities::SetComponentFlowRate(state, massFlowRate, this->inletNode, this->outletNode, this->plantLoc);
    this->massFlowRate = massFlowRate;

    // If the flow is zero, all of the load goes to the zone and the maximum case temperature is reported.
    // This is a reporting convention, not a prediction: without cooling a real chip would exceed this limit, and there is no
    // transient chip model (Tc = f(t, load)) to represent the thermal mass or the time taken to heat up.
    if (massFlowRate <= 0.0) {
        this->heatRemovedByFluid = 0.0;
        this->zoneHeatGainRate = load;
        this->effectiveThermalResistance = 0.0;
        this->caseTemperature = this->maximumCaseTemperature;
        this->outletTemp = inletTemp;
        return;
    }

    // Flow is known check cold plate maximum heat transfer rate
    Real64 thermalResistance = this->getAdjustedThermalResistance(state, massFlowRate / nominalMassFlowRate);
    this->effectiveThermalResistance = thermalResistance;
    Real64 const maxHeatTransferRate = heatTransferCapacity(massFlowRate, thermalResistance, this->maximumCaseTemperature);

    // Determine how much of the load is met by the fluid and how much spills to the zone
    this->heatRemovedByFluid = std::min(load, maxHeatTransferRate);
    this->zoneHeatGainRate = load - this->heatRemovedByFluid;

    // Chip case temperature
    if (this->thermalResistanceMethod == ThermalResistanceMethod::LMTD && massFlowRate > 0.0) {
        Real64 const effectiveness = 1.0 - std::exp(-1.0 / (massFlowRate * cp * thermalResistance));
        this->caseTemperature = inletTemp + this->heatRemovedByFluid / (massFlowRate * cp * effectiveness);
    } else {
        // When limited by the coolant energy balance, the coolant leaves at the case temperature
        this->caseTemperature = inletTemp + this->heatRemovedByFluid * std::max(thermalResistance, 1.0 / (massFlowRate * cp));
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
    static constexpr std::string_view routineName("processInputForCoilCoolingITEColdPlate");
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

        ErrorObjectHeader const eoh{routineName, cCurrentModuleObject, thisColdPlate.name};

        // Get schedules
        std::string const availSchedName = ip->getAlphaFieldValue(fields, schemaProps, "availability_schedule_name");
        if (availSchedName.empty()) {
            thisColdPlate.availabilitySchedule = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisColdPlate.availabilitySchedule = Sched::GetSchedule(state, availSchedName)) == nullptr) {
            ShowSevereItemNotFound(state, eoh, "Availability Schedule Name", availSchedName);
            errorsFound = true;
        }
        std::string const loadSchedName = ip->getAlphaFieldValue(fields, schemaProps, "it_equipment_load_schedule_name");
        if (!loadSchedName.empty() && (thisColdPlate.itLoadSchedule = Sched::GetSchedule(state, loadSchedName)) == nullptr) {
            ShowSevereItemNotFound(state, eoh, "IT Equipment Load Schedule Name", loadSchedName);
            errorsFound = true;
        }

        // Get the numeric fields
        thisColdPlate.thermalResistance = ip->getRealFieldValue(fields, schemaProps, "nominal_thermal_resistance");
        thisColdPlate.maximumCaseTemperature = ip->getRealFieldValue(fields, schemaProps, "maximum_case_temperature");
        thisColdPlate.nominalFlowRate = ip->getRealFieldValue(fields, schemaProps, "nominal_liquid_flow_rate");
        thisColdPlate.nominalFlowRateWasAutoSized = (thisColdPlate.nominalFlowRate == DataSizing::AutoSize);
        thisColdPlate.maximumFlowRate = ip->getRealFieldValue(fields, schemaProps, "maximum_liquid_flow_rate");
        thisColdPlate.maximumFlowRateWasAutoSized = (thisColdPlate.maximumFlowRate == DataSizing::AutoSize);
        if (!thisColdPlate.nominalFlowRateWasAutoSized && !thisColdPlate.maximumFlowRateWasAutoSized && thisColdPlate.maximumFlowRate > 0.0 &&
            thisColdPlate.maximumFlowRate < thisColdPlate.nominalFlowRate) {
            ShowSevereError(
                state,
                std::format("{}: Maximum Liquid Flow Rate ({:.6f} m3/s) is less than Nominal Liquid Flow Rate ({:.6f} m3/s) for object \"{}\"",
                            cCurrentModuleObject,
                            thisColdPlate.maximumFlowRate,
                            thisColdPlate.nominalFlowRate,
                            thisColdPlate.name));
            errorsFound = true;
        }
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
            SetupZoneInternalGain(state, zoneIndex, stored.name, DataHeatBalance::IntGainType::CoilCoolingITEColdPlate, &stored.zoneHeatGainRate);
        }
    }
    if (errorsFound) {
        ShowFatalError(state, std::format("Errors found in processing input for {}", cCurrentModuleObject));
    }
}

} // namespace EnergyPlus::LiquidCooledITEPlantComponents
