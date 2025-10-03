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

#include "AirflowNetwork/Solver.hpp"

// C++ Headers
#include <algorithm>
#include <cmath>
#include <set>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array2D.hh>
// #include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Elements.hpp>
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataBranchNodeConnections.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DuctLoss.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/HVACStandAloneERV.hh>
#include <EnergyPlus/HVACVariableRefrigerantFlow.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/RoomAirModelManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SingleDuct.hh>
#include <EnergyPlus/SplitterComponent.hh>
#include <EnergyPlus/ThermalComfort.hh>
#include <EnergyPlus/UnitarySystem.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterThermalTanks.hh>
#include <EnergyPlus/WindowAC.hh>
#include <EnergyPlus/ZoneDehumidifier.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace AirflowNetwork {

    // MODULE INFORMATION:
    //       AUTHOR         Lixing Gu, Don Shirey, and Muthusamy V. Swami
    //       DATE WRITTEN   July 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // This module is used to simulate airflows and pressures. The module is modified to
    // meet requirements of EnergyPLus based on AIRNET, developed by
    // National Institute of Standards and Technology (NIST).

    // METHODOLOGY EMPLOYED:
    // An airflow network approach is used. It consists of nodes connected by airflow elements.
    // The Newton's method is applied to solve a sparse matrix. When a new solver is available, this
    // module will be replaced or updated.

    // REFERENCES:
    // Walton, G. N., 1989, "AIRNET - A Computer Program for Building Airflow Network Modeling,"
    // NISTIR 89-4072, National Institute of Standards and Technology, Gaithersburg, Maryland

    // Using/Aliasing
    using Curve::CurveValue;
    using Curve::GetCurveIndex;
    using DataEnvironment::OutDryBulbTempAt;
    using DataSurfaces::cExtBoundCondition;
    using DataSurfaces::ExternalEnvironment;
    using DataSurfaces::OtherSideCoefNoCalcExt;
    using DataSurfaces::SurfaceClass;
    using Fans::GetFanIndex;
    using Psychrometrics::PsyCpAirFnW;
    using Psychrometrics::PsyHFnTdbW;
    using Psychrometrics::PsyRhoAirFnPbTdbW;

    Solver::Solver(EnergyPlusData &state) : m_state(state), properties(state)
    {
    }

    int constexpr NumOfVentCtrTypes(6); // Number of zone level venting control types

    void Solver::manage_balance(ObjexxFCL::Optional_bool_const FirstHVACIteration, // True when solution technique on first iteration
                                ObjexxFCL::Optional_int_const Iter,                // Iteration number
                                ObjexxFCL::Optional_bool ResimulateAirZone         // True when solution technique on third iteration
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   July 28, 2005

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs simulation of air distribution system.

        // Locals
        int i;
        HVAC::FanType AFNSupplyFanType = HVAC::FanType::Invalid;

        if (AirflowNetworkGetInputFlag) {
            get_input();
            AirflowNetworkGetInputFlag = false;
            return;
        }

        if (present(ResimulateAirZone)) {
            ResimulateAirZone = false;
        }

        if (simulation_control.type == ControlType::NoMultizoneOrDistribution) {
            return;
        }

        if (m_state.dataGlobal->BeginEnvrnFlag) {
            m_state.dataHVACGlobal->TurnFansOn = false; // The FAN should be off when BeginEnvrnFlag = .True.
        }

        initialize();

        auto &NetworkNumOfNodes = ActualNumOfNodes;
        auto &NetworkNumOfLinks = ActualNumOfLinks;

        NetworkNumOfNodes = NumOfNodesMultiZone;
        NetworkNumOfLinks = NumOfLinksMultiZone;

        AirflowNetworkFanActivated = false;

        if (present(FirstHVACIteration) && distribution_simulated) {
            if (FirstHVACIteration) {
                if (allocated(m_state.dataAirLoop->AirLoopAFNInfo)) {
                    for (i = 1; i <= DisSysNumOfCVFs; i++) {
                        m_state.dataAirLoop->AirLoopAFNInfo(i).AFNLoopHeatingCoilMaxRTF = 0.0;
                        m_state.dataAirLoop->AirLoopAFNInfo(i).AFNLoopOnOffFanRTF = 0.0;
                        m_state.dataAirLoop->AirLoopAFNInfo(i).AFNLoopDXCoilRTF = 0.0;
                        m_state.dataAirLoop->AirLoopAFNInfo(i).LoopOnOffFanPartLoadRatio = 0.0;
                    }
                }
            }
            Real64 FanMassFlowRate = 0.0;
            HVAC::FanOp FanOperModeCyc = HVAC::FanOp::Invalid;
            AFNSupplyFanType = HVAC::FanType::Invalid;

            for (i = 1; i <= DisSysNumOfCVFs; i++) {
                AFNSupplyFanType = DisSysCompCVFData(i).fanType;
                FanMassFlowRate = max(FanMassFlowRate, m_state.dataLoopNodes->Node(DisSysCompCVFData(i).OutletNode).MassFlowRate);
                // VAV take high priority
                if (DisSysCompCVFData(i).fanType == HVAC::FanType::VAV) {
                    AFNSupplyFanType = DisSysCompCVFData(i).fanType;
                    break;
                }
                if (FanMassFlowRate > HVAC::VerySmallMassFlow &&
                    m_state.dataAirLoop->AirLoopAFNInfo(i).LoopFanOperationMode == HVAC::FanOp::Cycling &&
                    m_state.dataAirLoop->AirLoopAFNInfo(i).LoopSystemOnMassFlowrate > 0.0) {
                    FanOperModeCyc = HVAC::FanOp::Cycling;
                    AFNSupplyFanType = DisSysCompCVFData(i).fanType;
                    if (AFNSupplyFanType == HVAC::FanType::OnOff) {
                        break;
                    }
                }
            }
            //            Revised to meet heat exchanger requirement
            if ((FanMassFlowRate > HVAC::VerySmallMassFlow) && (!FirstHVACIteration)) {
                if (AFNSupplyFanType == HVAC::FanType::OnOff && FanOperModeCyc == HVAC::FanOp::Cycling) {
                    AirflowNetworkFanActivated = true;
                } else if (AFNSupplyFanType == HVAC::FanType::VAV) {
                    if (present(Iter) && Iter > 1) {
                        AirflowNetworkFanActivated = true;
                    }
                } else if (AirflowNetworkUnitarySystem) {
                    if (present(Iter) && Iter > 1) {
                        AirflowNetworkFanActivated = true;
                    }
                } else {
                    AirflowNetworkFanActivated = true;
                }
            }
        }
        if (allocated(m_state.dataZoneEquip->ZoneEquipConfig) && m_state.dataAvail->NumHybridVentSysAvailMgrs > 0 &&
            allocated(m_state.dataAirSystemsData->PrimaryAirSystems)) {
            hybrid_ventilation_control();
        }
        if (ventCtrlStatus == Avail::VentCtrlStatus::Open && m_state.dataAvail->NumHybridVentSysAvailMgrs > 0) {
            AirflowNetworkFanActivated = false;
        }

        if (present(Iter) && present(ResimulateAirZone) && distribution_simulated) {
            if (AirflowNetworkFanActivated && Iter < 3 && AFNSupplyFanType == HVAC::FanType::OnOff) {
                ResimulateAirZone = true;
            }
            if (AFNSupplyFanType == HVAC::FanType::VAV) {
                if (!AirflowNetworkFanActivated && Iter < 3) {
                    ResimulateAirZone = true;
                }
            }
            if (AirflowNetworkUnitarySystem) {
                if (!AirflowNetworkFanActivated && Iter < 3) {
                    ResimulateAirZone = true;
                }
            }
        }
        if (AirflowNetworkFanActivated && distribution_simulated) {
            NetworkNumOfNodes = AirflowNetworkNumOfNodes;
            NetworkNumOfLinks = AirflowNetworkNumOfLinks;
        }

        if (allocated(m_state.dataZoneEquip->ZoneEquipConfig)) {
            validate_exhaust_fan_input();
        }

        // VAV terminal set only
        if (present(FirstHVACIteration) && FirstHVACIteration) {
            VAVTerminalRatio = 0.0;
        }

        if (AirflowNetworkFanActivated && distribution_simulated) {
            if (ValidateDistributionSystemFlag) {
                validate_distribution();
                validate_fan_flowrate();
                ValidateDistributionSystemFlag = false;
                if (simulation_control.autosize_ducts) {
                    SizeDucts();
                }
            }
        }
        calculate_balance();

        if (AirflowNetworkFanActivated && distribution_simulated) {

            LoopOnOffFlag = false;
            for (i = 1; i <= DisSysNumOfCVFs; i++) {
                if (DisSysCompCVFData(i).AirLoopNum > 0) {
                    if (m_state.dataLoopNodes->Node(DisSysCompCVFData(i).InletNode).MassFlowRate > 0.0) {
                        LoopOnOffFlag(DisSysCompCVFData(i).AirLoopNum) = true;
                    }
                }
            }

            calculate_heat_balance();
            calculate_moisture_balance();
            if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                calculate_CO2_balance();
            }
            if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                calculate_GC_balance();
            }
        }

        update(FirstHVACIteration);
    }

    bool Solver::get_element_input()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason DeGraw
        //       DATE WRITTEN   Oct. 2018
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reads airflow element inputs (eventually)

        static constexpr std::string_view RoutineName{"get_element_input"};
        std::string CurrentModuleObject;
        bool success{true};

        // *** Read AirflowNetwork simulation reference crack conditions
        std::unordered_map<std::string, ReferenceConditions> referenceConditions; // Map for lookups
        ReferenceConditions defaultReferenceConditions("Default");                // Defaulted conditions
        bool conditionsAreDefaulted(true);                                        // Conditions are defaulted?
        CurrentModuleObject = "AirflowNetwork:MultiZone:ReferenceCrackConditions";
        auto instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            // globalSolverObject.referenceConditions.clear();
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());
                Real64 temperature(20.0);
                if (fields.find("reference_temperature") != fields.end()) { // required field, has default value
                    temperature = fields.at("reference_temperature").get<Real64>();
                }
                Real64 pressure(101325.0);
                if (fields.find("reference_barometric_pressure") != fields.end()) { // not required field, has default value
                    pressure = fields.at("reference_barometric_pressure").get<Real64>();
                    if (std::abs((pressure - m_state.dataEnvrn->StdBaroPress) / m_state.dataEnvrn->StdBaroPress) > 0.1) { // 10% off
                        ShowWarningError(m_state,
                                         format("{}: {}: Pressure = {:.0R} differs by more than 10% from Standard Barometric Pressure = {:.0R}.",
                                                RoutineName,
                                                CurrentModuleObject,
                                                pressure,
                                                m_state.dataEnvrn->StdBaroPress));
                        ShowContinueError(m_state, "...occurs in " + CurrentModuleObject + " = " + thisObjectName);
                    }
                    if (pressure <= 31000.0) {
                        ShowSevereError(m_state,
                                        format("{}: {}: {}. Reference Barometric Pressure must be greater than 31000 Pa.",
                                               RoutineName,
                                               CurrentModuleObject,
                                               thisObjectName));
                        success = false;
                    }
                }
                Real64 humidity(0.0);
                if (fields.find("reference_humidity_ratio") != fields.end()) { // not required field, has default value
                    humidity = fields.at("reference_humidity_ratio").get<Real64>();
                }
                // globalSolverObject.referenceConditions.emplace_back(thisObjectName, temperature, pressure, humidity);
                referenceConditions.emplace(std::piecewise_construct,
                                            std::forward_as_tuple(thisObjectName),
                                            std::forward_as_tuple(instance.key(), temperature, pressure, humidity));
            }
            // Check that there is more than one
            if (referenceConditions.size() == 1) {
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed("AirflowNetwork:MultiZone:ReferenceCrackConditions",
                                                                              referenceConditions.begin()->second.name);
                defaultReferenceConditions = referenceConditions.begin()->second;

            } else {
                conditionsAreDefaulted = false;
            }
        }
        if (!success) {
            return false;
        }

        // *** Read AirflowNetwork simulation surface crack component
        CurrentModuleObject = "AirflowNetwork:MultiZone:Surface:Crack";
        AirflowNetworkNumOfSurCracks =
            m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                        // Temporary workaround
            MultizoneSurfaceCrackData.allocate(AirflowNetworkNumOfSurCracks); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 coeff{fields.at("air_mass_flow_coefficient_at_reference_conditions")}; // Required field
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent") != fields.end()) { // not required field, has default value
                    expnt = fields.at("air_mass_flow_exponent").get<Real64>();
                }
                Real64 refT = defaultReferenceConditions.temperature;
                Real64 refP = defaultReferenceConditions.pressure;
                Real64 refW = defaultReferenceConditions.humidity_ratio;
                if (!conditionsAreDefaulted) {
                    if (fields.find("reference_crack_conditions") != fields.end()) { // not required field, *should* have default value
                        auto refCrackCondName = fields.at("reference_crack_conditions").get<std::string>();
                        auto result = referenceConditions.find(Util::makeUPPER(refCrackCondName));

                        if (result == referenceConditions.end()) {
                            ShowSevereError(m_state,
                                            format("{}: {}: {}. Cannot find reference crack conditions object \"{}\".",
                                                   RoutineName,
                                                   CurrentModuleObject,
                                                   thisObjectName,
                                                   refCrackCondName));
                            success = false;
                        } else {
                            refT = result->second.temperature;
                            refP = result->second.pressure;
                            refW = result->second.humidity_ratio;
                            m_state.dataInputProcessing->inputProcessor->markObjectAsUsed("AirflowNetwork:MultiZone:ReferenceCrackConditions",
                                                                                          result->second.name);
                        }
                    }
                }
                // globalSolverObject.cracks[thisObjectName] = SurfaceCrack(coeff, expnt, refT, refP, refW);
                MultizoneSurfaceCrackData(i).name = thisObjectName; // Name of surface crack component
                MultizoneSurfaceCrackData(i).coefficient = coeff;   // Air Mass Flow Coefficient
                MultizoneSurfaceCrackData(i).exponent = expnt;      // Air Mass Flow exponent
                MultizoneSurfaceCrackData(i).reference_density = properties.density(refP, refT, refW);
                MultizoneSurfaceCrackData(i).reference_viscosity = properties.dynamic_viscosity(refT);

                // This is the first element that is being added to the lookup table, so no check of naming overlaps
                elements[thisObjectName] = &MultizoneSurfaceCrackData(i); // Yet another workaround

                ++i;
            }
        }

        // *** Read AirflowNetwork simulation zone exhaust fan component
        CurrentModuleObject = "AirflowNetwork:MultiZone:Component:ZoneExhaustFan";
        AirflowNetworkNumOfExhFan =
            m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject);              // Temporary workaround
        NumOfExhaustFans = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "Fan:ZoneExhaust"); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                       // Temporary workaround
            MultizoneCompExhaustFanData.allocate(AirflowNetworkNumOfExhFan); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());

                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 coeff{fields.at("air_mass_flow_coefficient_when_the_zone_exhaust_fan_is_off_at_reference_conditions")}; // Required field
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent_when_the_zone_exhaust_fan_is_off") != fields.end()) { // not required field, has default value
                    expnt = fields.at("air_mass_flow_exponent_when_the_zone_exhaust_fan_is_off").get<Real64>();
                }

                // This breaks the component model, need to fix
                int fanIndex = GetFanIndex(m_state, thisObjectName);
                if (fanIndex == 0) {
                    ShowSevereError(m_state,
                                    format("{}: {} = {} is not found in Fan:ZoneExhaust objects.", RoutineName, CurrentModuleObject, thisObjectName));
                    success = false;
                }

                auto *fan = m_state.dataFans->fans(fanIndex);

                Real64 flowRate = fan->maxAirFlowRate;
                flowRate *= m_state.dataEnvrn->StdRhoAir;
                int inletNode = fan->inletNodeNum;
                int outletNode = fan->outletNodeNum;
                HVAC::FanType fanType = fan->type;
                if (fanType != HVAC::FanType::Exhaust) {
                    ShowSevereError(m_state,
                                    format("{}: {} = {}. The specified Name is not found as a valid Fan:ZoneExhaust object.",
                                           RoutineName,
                                           CurrentModuleObject,
                                           thisObjectName));
                    success = false;
                }

                Real64 refT = defaultReferenceConditions.temperature;
                Real64 refP = defaultReferenceConditions.pressure;
                Real64 refW = defaultReferenceConditions.humidity_ratio;
                if (!conditionsAreDefaulted) {
                    if (fields.find("reference_crack_conditions") != fields.end()) { // not required field, *should* have default value
                        auto refCrackCondName = fields.at("reference_crack_conditions").get<std::string>();
                        auto result = referenceConditions.find(Util::makeUPPER(refCrackCondName));
                        if (result == referenceConditions.end()) {
                            ShowSevereError(m_state,
                                            format("{}: {}: {}. Cannot find reference crack conditions object \"{}\".",
                                                   RoutineName,
                                                   CurrentModuleObject,
                                                   thisObjectName,
                                                   fields.at("reference_crack_conditions").get<std::string>()));
                            success = false;
                        } else {
                            refT = result->second.temperature;
                            refP = result->second.pressure;
                            refW = result->second.humidity_ratio;
                            m_state.dataInputProcessing->inputProcessor->markObjectAsUsed("AirflowNetwork:MultiZone:ReferenceCrackConditions",
                                                                                          result->second.name);
                        }
                    }
                }

                MultizoneCompExhaustFanData(i).name = thisObjectName; // Name of zone exhaust fan component
                MultizoneCompExhaustFanData(i).FlowCoef = coeff;      // flow coefficient
                MultizoneCompExhaustFanData(i).FlowExpo = expnt;      // Flow exponent

                MultizoneCompExhaustFanData(i).FlowRate = flowRate;
                MultizoneCompExhaustFanData(i).InletNode = inletNode;
                MultizoneCompExhaustFanData(i).OutletNode = outletNode;

                MultizoneCompExhaustFanData(i).StandardT = refT;
                MultizoneCompExhaustFanData(i).StandardP = refP;
                MultizoneCompExhaustFanData(i).StandardW = refW;

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(thisObjectName) == elements.end()) {
                    elements[thisObjectName] = &MultizoneCompExhaustFanData(i); // Yet another workaround
                } else {
                    ShowSevereError(
                        m_state,
                        format("{}: {}: Duplicated airflow element names are found = \"{}\".", RoutineName, CurrentModuleObject, thisObjectName));
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read Outdoor Airflow object
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:OutdoorAirFlow";
        NumOfOAFans = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                      // Temporary workaround
            DisSysCompOutdoorAirData.allocate(NumOfOAFans); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                std::string mixer_name = Util::makeUPPER(fields.at("outdoor_air_mixer_name").get<std::string>());
                Real64 coeff{fields.at("air_mass_flow_coefficient_when_no_outdoor_air_flow_at_reference_conditions")};
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent_when_no_outdoor_air_flow") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent_when_no_outdoor_air_flow").get<Real64>();
                }

                int OAMixerNum = MixedAir::GetOAMixerNumber(m_state, mixer_name);
                if (OAMixerNum == 0) {
                    ShowSevereError(m_state,
                                    format("{}: {}: {}. Invalid Outdoor Air Mixer Name \"{}\" given.",
                                           RoutineName,
                                           CurrentModuleObject,
                                           thisObjectName,
                                           mixer_name));
                    success = false;
                }

                Real64 refT = defaultReferenceConditions.temperature;
                Real64 refP = defaultReferenceConditions.pressure;
                Real64 refW = defaultReferenceConditions.humidity_ratio;
                if (!conditionsAreDefaulted) {
                    if (fields.find("reference_crack_conditions") != fields.end()) { // not required field, *should* have default value
                        auto refCrackCondName = fields.at("reference_crack_conditions").get<std::string>();
                        auto result = referenceConditions.find(Util::makeUPPER(refCrackCondName));
                        if (result == referenceConditions.end()) {
                            ShowSevereError(m_state,
                                            format("{}: {}: {}. Cannot find reference crack conditions object \"{}\".",
                                                   RoutineName,
                                                   CurrentModuleObject,
                                                   thisObjectName,
                                                   refCrackCondName));
                            success = false;
                        } else {
                            refT = result->second.temperature;
                            refP = result->second.pressure;
                            refW = result->second.humidity_ratio;
                            m_state.dataInputProcessing->inputProcessor->markObjectAsUsed("AirflowNetwork:MultiZone:ReferenceCrackConditions",
                                                                                          result->second.name);
                        }
                    }
                }

                DisSysCompOutdoorAirData(i).name = thisObjectName; // Name of zone exhaust fan component
                DisSysCompOutdoorAirData(i).FlowCoef = coeff;      // flow coefficient
                DisSysCompOutdoorAirData(i).FlowExpo = expnt;      // Flow exponent

                DisSysCompOutdoorAirData(i).OAMixerNum = OAMixerNum;

                DisSysCompOutdoorAirData(i).StandardT = refT;
                DisSysCompOutdoorAirData(i).StandardP = refP;
                DisSysCompOutdoorAirData(i).StandardW = refW;

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(thisObjectName) == elements.end()) {
                    elements[thisObjectName] = &DisSysCompOutdoorAirData(i); // Yet another workaround
                } else {
                    ShowSevereError(
                        m_state,
                        format("{}: {}: Duplicated airflow element names are found = \"{}\".", RoutineName, CurrentModuleObject, thisObjectName));
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read Relief Airflow object
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:ReliefAirFlow";
        NumOfReliefFans = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                      // Temporary workaround
            DisSysCompReliefAirData.allocate(m_state.afn->NumOfReliefFans); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                std::string mixer_name = Util::makeUPPER(fields.at("outdoor_air_mixer_name").get<std::string>());
                Real64 coeff{fields.at("air_mass_flow_coefficient_when_no_outdoor_air_flow_at_reference_conditions")};
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent_when_no_outdoor_air_flow") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent_when_no_outdoor_air_flow").get<Real64>();
                }

                int OAMixerNum{MixedAir::GetOAMixerNumber(m_state, mixer_name)};
                if (OAMixerNum == 0) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + ": " + CurrentModuleObject + " object " + thisObjectName + ". Invalid " +
                                        "Outdoor Air Mixer Name" + " \"" + mixer_name + "\" given.");
                    success = false;
                }

                Real64 refT = defaultReferenceConditions.temperature;
                Real64 refP = defaultReferenceConditions.pressure;
                Real64 refW = defaultReferenceConditions.humidity_ratio;
                if (!conditionsAreDefaulted) {
                    if (fields.find("reference_crack_conditions") != fields.end()) { // not required field, *should* have default value
                        auto refCrackCondName = fields.at("reference_crack_conditions").get<std::string>();
                        auto result = referenceConditions.find(Util::makeUPPER(refCrackCondName));
                        if (result == referenceConditions.end()) {
                            ShowSevereError(m_state,
                                            format("{}: {}: {}. Cannot find reference crack conditions object \"{}\".",
                                                   RoutineName,
                                                   CurrentModuleObject,
                                                   thisObjectName,
                                                   refCrackCondName));
                            success = false;
                        } else {
                            refT = result->second.temperature;
                            refP = result->second.pressure;
                            refW = result->second.humidity_ratio;
                            m_state.dataInputProcessing->inputProcessor->markObjectAsUsed("AirflowNetwork:MultiZone:ReferenceCrackConditions",
                                                                                          result->second.name);
                        }
                    }
                }

                DisSysCompReliefAirData(i).name = thisObjectName; // Name of zone exhaust fan component
                DisSysCompReliefAirData(i).FlowCoef = coeff;      // flow coefficient
                DisSysCompReliefAirData(i).FlowExpo = expnt;      // Flow exponent
                DisSysCompReliefAirData(i).OAMixerNum = OAMixerNum;
                DisSysCompReliefAirData(i).StandardT = refT;
                DisSysCompReliefAirData(i).StandardP = refP;
                DisSysCompReliefAirData(i).StandardW = refW;

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(thisObjectName) == elements.end()) {
                    elements[thisObjectName] = &DisSysCompReliefAirData(i); // Yet another workaround
                } else {
                    ShowSevereError(
                        m_state,
                        format("{}: {}: Duplicated airflow element names are found = \"{}\".", RoutineName, CurrentModuleObject, thisObjectName));
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork simulation detailed openings
        CurrentModuleObject = "AirflowNetwork:MultiZone:Component:DetailedOpening";
        AirflowNetworkNumOfDetOpenings =
            m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                            // Temporary workaround
            MultizoneCompDetOpeningData.allocate(AirflowNetworkNumOfDetOpenings); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 coeff{fields.at("air_mass_flow_coefficient_when_opening_is_closed")};
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent_when_opening_is_closed") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent_when_opening_is_closed").get<Real64>();
                }

                int LVOtype{1};
                std::string LVOstring;
                if (fields.find("type_of_rectangular_large_vertical_opening_lvo_") != fields.end()) {
                    LVOstring = fields.at("type_of_rectangular_large_vertical_opening_lvo_").get<std::string>();
                    if (Util::SameString(LVOstring, "NonPivoted") || Util::SameString(LVOstring, "1")) {
                        LVOtype = 1; // Large vertical opening type number
                    } else if (Util::SameString(LVOstring, "HorizontallyPivoted") || Util::SameString(LVOstring, "2")) {
                        LVOtype = 2; // Large vertical opening type number
                    } else {
                        // Code will never be executed, validation will catch invalid input
                        ShowSevereError(m_state,
                                        format(RoutineName) + "Invalid Type of Rectangular Large Vertical Opening (LVO) = " + LVOstring + "in " +
                                            CurrentModuleObject + " = " + thisObjectName);
                        ShowContinueError(m_state, "Valid choices are NonPivoted and HorizontallyPivoted.");
                        success = false;
                    }
                }

                Real64 extra{0.0};
                if (fields.find("extra_crack_length_or_height_of_pivoting_axis") != fields.end()) {
                    extra = fields.at("extra_crack_length_or_height_of_pivoting_axis").get<Real64>();
                }

                int N{fields.at("number_of_sets_of_opening_factor_data")};

                std::vector<Real64> factors(N);
                std::vector<Real64> cds(N);
                std::vector<Real64> width_factors(N);
                std::vector<Real64> height_factors(N);
                std::vector<Real64> start_height_factors(N);

                // Real64 factor{0.0};
                // if (fields.find("opening_factor_1") != fields.end()) {
                //    factor = fields.at("opening_factor_1");
                //}
                Real64 cd{0.001};
                if (fields.find("discharge_coefficient_for_opening_factor_1") != fields.end()) {
                    cd = fields.at("discharge_coefficient_for_opening_factor_1").get<Real64>();
                }
                Real64 width_factor{0.0};
                if (fields.find("width_factor_for_opening_factor_1") != fields.end()) {
                    width_factor = fields.at("width_factor_for_opening_factor_1").get<Real64>();
                }
                Real64 height_factor{0.0};
                if (fields.find("height_factor_for_opening_factor_1") != fields.end()) {
                    height_factor = fields.at("height_factor_for_opening_factor_1").get<Real64>();
                }
                Real64 start_height_factor{0.0};
                if (fields.find("start_height_factor_for_opening_factor_1") != fields.end()) {
                    start_height_factor = fields.at("start_height_factor_for_opening_factor_1").get<Real64>();
                }

                factors[0] = 0.0; // factor; // This factor must be zero
                cds[0] = cd;
                width_factors[0] = width_factor;
                height_factors[0] = height_factor;
                start_height_factors[0] = start_height_factor;

                Real64 factor{fields.at("opening_factor_2")};
                cd = 1.0;
                if (fields.find("discharge_coefficient_for_opening_factor_2") != fields.end()) {
                    cd = fields.at("discharge_coefficient_for_opening_factor_2").get<Real64>();
                }
                width_factor = 1.0;
                if (fields.find("width_factor_for_opening_factor_2") != fields.end()) {
                    width_factor = fields.at("width_factor_for_opening_factor_2").get<Real64>();
                }
                height_factor = 1.0;
                if (fields.find("height_factor_for_opening_factor_2") != fields.end()) {
                    height_factor = fields.at("height_factor_for_opening_factor_2").get<Real64>();
                }
                start_height_factor = 0.0;
                if (fields.find("start_height_factor_for_opening_factor_2") != fields.end()) {
                    start_height_factor = fields.at("start_height_factor_for_opening_factor_2").get<Real64>();
                }

                factors[1] = factor;
                cds[1] = cd;
                width_factors[1] = width_factor;
                height_factors[1] = height_factor;
                start_height_factors[1] = start_height_factor;

                if (N >= 3) {
                    factor = fields.at("opening_factor_3").get<Real64>();
                    cd = 0.0;
                    if (fields.find("discharge_coefficient_for_opening_factor_3") != fields.end()) {
                        cd = fields.at("discharge_coefficient_for_opening_factor_3").get<Real64>();
                    }
                    width_factor = 0.0;
                    if (fields.find("width_factor_for_opening_factor_3") != fields.end()) {
                        width_factor = fields.at("width_factor_for_opening_factor_3").get<Real64>();
                    }
                    height_factor = 0.0;
                    if (fields.find("height_factor_for_opening_factor_3") != fields.end()) {
                        height_factor = fields.at("height_factor_for_opening_factor_3").get<Real64>();
                    }
                    start_height_factor = 0.0;
                    if (fields.find("start_height_factor_for_opening_factor_3") != fields.end()) {
                        start_height_factor = fields.at("start_height_factor_for_opening_factor_3").get<Real64>();
                    }

                    factors[2] = factor;
                    cds[2] = cd;
                    width_factors[2] = width_factor;
                    height_factors[2] = height_factor;
                    start_height_factors[2] = start_height_factor;

                    if (N >= 4) {
                        factor = fields.at("opening_factor_4").get<Real64>();
                        cd = 0.0;
                        if (fields.find("discharge_coefficient_for_opening_factor_4") != fields.end()) {
                            cd = fields.at("discharge_coefficient_for_opening_factor_4").get<Real64>();
                        }
                        width_factor = 0.0;
                        if (fields.find("width_factor_for_opening_factor_4") != fields.end()) {
                            width_factor = fields.at("width_factor_for_opening_factor_4").get<Real64>();
                        }
                        height_factor = 0.0;
                        if (fields.find("height_factor_for_opening_factor_4") != fields.end()) {
                            height_factor = fields.at("height_factor_for_opening_factor_4").get<Real64>();
                        }
                        start_height_factor = 0.0;
                        if (fields.find("start_height_factor_for_opening_factor_4") != fields.end()) {
                            start_height_factor = fields.at("start_height_factor_for_opening_factor_4").get<Real64>();
                        }

                        factors[3] = factor;
                        cds[3] = cd;
                        width_factors[3] = width_factor;
                        height_factors[3] = height_factor;
                        start_height_factors[3] = start_height_factor;
                    }
                }

                MultizoneCompDetOpeningData(i).name = thisObjectName; // Name of large detailed opening component
                MultizoneCompDetOpeningData(i).FlowCoef = coeff;      // Air Mass Flow Coefficient When Window or Door Is Closed
                MultizoneCompDetOpeningData(i).FlowExpo = expnt;      // Air Mass Flow exponent When Window or Door Is Closed
                MultizoneCompDetOpeningData(i).TypeName = LVOstring;  // Large vertical opening type
                MultizoneCompDetOpeningData(i).LVOType = LVOtype;     // Large vertical opening type number
                MultizoneCompDetOpeningData(i).LVOValue = extra;      // Extra crack length for LVO type 1 with multiple openable
                                                                      // parts, or Height of pivoting axis for LVO type 2

                MultizoneCompDetOpeningData(i).NumFac = N; // Number of Opening Factor Values

                MultizoneCompDetOpeningData(i).OpenFac1 = factors[0];                // Opening factor #1
                MultizoneCompDetOpeningData(i).DischCoeff1 = cds[0];                 // Discharge coefficient for opening factor #1
                MultizoneCompDetOpeningData(i).WidthFac1 = width_factors[0];         // Width factor for for Opening factor #1
                MultizoneCompDetOpeningData(i).HeightFac1 = height_factors[0];       // Height factor for opening factor #1
                MultizoneCompDetOpeningData(i).StartHFac1 = start_height_factors[0]; // Start height factor for opening factor #1
                MultizoneCompDetOpeningData(i).OpenFac2 = factors[1];                // Opening factor #2
                MultizoneCompDetOpeningData(i).DischCoeff2 = cds[1];                 // Discharge coefficient for opening factor #2
                MultizoneCompDetOpeningData(i).WidthFac2 = width_factors[1];         // Width factor for for Opening factor #2
                MultizoneCompDetOpeningData(i).HeightFac2 = height_factors[1];       // Height factor for opening factor #2
                MultizoneCompDetOpeningData(i).StartHFac2 = start_height_factors[1]; // Start height factor for opening factor #2

                MultizoneCompDetOpeningData(i).OpenFac3 = 0.0;    // Opening factor #3
                MultizoneCompDetOpeningData(i).DischCoeff3 = 0.0; // Discharge coefficient for opening factor #3
                MultizoneCompDetOpeningData(i).WidthFac3 = 0.0;   // Width factor for for Opening factor #3
                MultizoneCompDetOpeningData(i).HeightFac3 = 0.0;  // Height factor for opening factor #3
                MultizoneCompDetOpeningData(i).StartHFac3 = 0.0;  // Start height factor for opening factor #3
                MultizoneCompDetOpeningData(i).OpenFac4 = 0.0;    // Opening factor #4
                MultizoneCompDetOpeningData(i).DischCoeff4 = 0.0; // Discharge coefficient for opening factor #4
                MultizoneCompDetOpeningData(i).WidthFac4 = 0.0;   // Width factor for for Opening factor #4
                MultizoneCompDetOpeningData(i).HeightFac4 = 0.0;  // Height factor for opening factor #4
                MultizoneCompDetOpeningData(i).StartHFac4 = 0.0;  // Start height factor for opening factor #4
                if (N == 2) {
                    if (factors[1] != 1.0) {
                        ShowWarningError(m_state, format("{}: {} = {}", RoutineName, CurrentModuleObject, thisObjectName));
                        ShowContinueError(
                            m_state,
                            "..This object specifies that only 3 opening factors will be used. So, the value of Opening Factor #2 is set to 1.0.");
                        ShowContinueError(m_state, format("..Input value was {:.2R}", MultizoneCompDetOpeningData(i).OpenFac2));
                        MultizoneCompDetOpeningData(i).OpenFac2 = 1.0;
                    }
                } else if (N >= 3) {
                    MultizoneCompDetOpeningData(i).OpenFac3 = factors[2];                // Opening factor #3
                    MultizoneCompDetOpeningData(i).DischCoeff3 = cds[2];                 // Discharge coefficient for opening factor #3
                    MultizoneCompDetOpeningData(i).WidthFac3 = width_factors[2];         // Width factor for for Opening factor #3
                    MultizoneCompDetOpeningData(i).HeightFac3 = height_factors[2];       // Height factor for opening factor #3
                    MultizoneCompDetOpeningData(i).StartHFac3 = start_height_factors[2]; // Start height factor for opening factor #3
                    if (N >= 4) {
                        MultizoneCompDetOpeningData(i).OpenFac4 = factors[3]; // Opening factor #4
                        if (factors[3] != 1.0) {
                            ShowWarningError(m_state, format("{}: {} = {}", RoutineName, CurrentModuleObject, thisObjectName));
                            ShowContinueError(m_state,
                                              "..This object specifies that 4 opening factors will be used. So, the value of Opening Factor #4 "
                                              "is set to 1.0.");
                            ShowContinueError(m_state, format("..Input value was {:.2R}", MultizoneCompDetOpeningData(i).OpenFac4));
                            MultizoneCompDetOpeningData(i).OpenFac4 = 1.0;
                        }
                        MultizoneCompDetOpeningData(i).DischCoeff4 = cds[3];                 // Discharge coefficient for opening factor #4
                        MultizoneCompDetOpeningData(i).WidthFac4 = width_factors[3];         // Width factor for for Opening factor #4
                        MultizoneCompDetOpeningData(i).HeightFac4 = height_factors[3];       // Height factor for opening factor #4
                        MultizoneCompDetOpeningData(i).StartHFac4 = start_height_factors[3]; // Start height factor for opening factor #4
                    } else {
                        if (factors[2] != 1.0) {
                            ShowWarningError(m_state, format("{}: {} = {}", RoutineName, CurrentModuleObject, thisObjectName));
                            ShowContinueError(m_state,
                                              "..This object specifies that only 3 opening factors will be used. So, the value of Opening Factor #3 "
                                              "is set to 1.0.");
                            ShowContinueError(m_state, format("..Input value was {:.2R}", MultizoneCompDetOpeningData(i).OpenFac3));
                            MultizoneCompDetOpeningData(i).OpenFac3 = 1.0;
                        }
                    }
                }

                // Sanity checks, check sum of Height Factor and the Start Height Factor
                if (MultizoneCompDetOpeningData(i).HeightFac1 + MultizoneCompDetOpeningData(i).StartHFac1 > 1.0) {
                    ShowSevereError(m_state, format("{}: {} = {}", RoutineName, CurrentModuleObject, thisObjectName));
                    ShowContinueError(
                        m_state, "..The sum of Height Factor for Opening Factor 1 and Start Height Factor for Opening Factor 1 is greater than 1.0");
                    success = false;
                }
                if (MultizoneCompDetOpeningData(i).HeightFac2 + MultizoneCompDetOpeningData(i).StartHFac2 > 1.0) {
                    ShowSevereError(m_state, format("{}: {} = {}", RoutineName, CurrentModuleObject, thisObjectName));
                    ShowContinueError(
                        m_state, "..The sum of Height Factor for Opening Factor 2 and Start Height Factor for Opening Factor 2 is greater than 1.0");
                    success = false;
                }
                if (MultizoneCompDetOpeningData(i).NumFac > 2) {
                    if (MultizoneCompDetOpeningData(i).OpenFac2 >= MultizoneCompDetOpeningData(i).OpenFac3) {
                        ShowSevereError(m_state, format("{}: {} = {}", RoutineName, CurrentModuleObject, thisObjectName));
                        ShowContinueError(m_state, "..The value of Opening Factor #2 >= the value of Opening Factor #3");
                        success = false;
                    }
                    if (MultizoneCompDetOpeningData(i).HeightFac3 + MultizoneCompDetOpeningData(i).StartHFac3 > 1.0) {
                        ShowSevereError(m_state, format("{}: {} = {}", RoutineName, CurrentModuleObject, thisObjectName));
                        ShowContinueError(
                            m_state,
                            "..The sum of Height Factor for Opening Factor 3 and Start Height Factor for Opening Factor 3 is greater than 1.0");
                        success = false;
                    }
                    if (MultizoneCompDetOpeningData(i).NumFac == 4) {
                        if (MultizoneCompDetOpeningData(i).OpenFac3 >= MultizoneCompDetOpeningData(i).OpenFac4) {
                            ShowSevereError(m_state, format("{}: {} = {}", RoutineName, CurrentModuleObject, thisObjectName));
                            ShowContinueError(m_state, "..The value of Opening Factor #3 >= the value of Opening Factor #4");
                            success = false;
                        }
                        if (MultizoneCompDetOpeningData(i).HeightFac4 + MultizoneCompDetOpeningData(i).StartHFac4 > 1.0) {
                            ShowSevereError(m_state, format("{}: {} = {}", RoutineName, CurrentModuleObject, thisObjectName));
                            ShowContinueError(
                                m_state,
                                "..The sum of Height Factor for Opening Factor 4 and Start Height Factor for Opening Factor 4 is greater than 1.0");
                            success = false;
                        }
                    }
                }

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(thisObjectName) == elements.end()) {
                    elements[thisObjectName] = &MultizoneCompDetOpeningData(i); // Yet another workaround
                } else {
                    ShowSevereError(
                        m_state,
                        format("{}: {}: Duplicated airflow element names are found = \"{}\".", RoutineName, CurrentModuleObject, thisObjectName));
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork simulation simple openings
        CurrentModuleObject = "AirflowNetwork:MultiZone:Component:SimpleOpening";
        AirflowNetworkNumOfSimOpenings =
            m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                               // Temporary workaround
            MultizoneCompSimpleOpeningData.allocate(AirflowNetworkNumOfSimOpenings); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 coeff{fields.at("air_mass_flow_coefficient_when_opening_is_closed")};
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent_when_opening_is_closed") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent_when_opening_is_closed").get<Real64>();
                }
                Real64 diff{fields.at("minimum_density_difference_for_two_way_flow")};
                Real64 dischargeCoeff{fields.at("discharge_coefficient")};

                MultizoneCompSimpleOpeningData(i).name = thisObjectName;       // Name of large simple opening component
                MultizoneCompSimpleOpeningData(i).FlowCoef = coeff;            // Air Mass Flow Coefficient When Window or Door Is Closed
                MultizoneCompSimpleOpeningData(i).FlowExpo = expnt;            // Air Mass Flow exponent When Window or Door Is Closed
                MultizoneCompSimpleOpeningData(i).MinRhoDiff = diff;           // Minimum density difference for two-way flow
                MultizoneCompSimpleOpeningData(i).DischCoeff = dischargeCoeff; // Discharge coefficient at full opening

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(thisObjectName) == elements.end()) {
                    elements[thisObjectName] = &MultizoneCompSimpleOpeningData(i); // Yet another workaround
                } else {
                    ShowSevereError(
                        m_state,
                        format("{}: {}: Duplicated airflow element names are found = \"{}\".", RoutineName, CurrentModuleObject, thisObjectName));
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork simulation horizontal openings
        CurrentModuleObject = "AirflowNetwork:MultiZone:Component:HorizontalOpening";
        AirflowNetworkNumOfHorOpenings =
            m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                            // Temporary workaround
            MultizoneCompHorOpeningData.allocate(AirflowNetworkNumOfHorOpenings); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 coeff{fields.at("air_mass_flow_coefficient_when_opening_is_closed")};
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent_when_opening_is_closed") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent_when_opening_is_closed").get<Real64>();
                }
                Real64 angle{90.0};
                if (fields.find("sloping_plane_angle") != fields.end()) {
                    angle = fields.at("sloping_plane_angle").get<Real64>();
                }
                Real64 dischargeCoeff{fields.at("discharge_coefficient")};

                MultizoneCompHorOpeningData(i).name = thisObjectName;       // Name of large simple opening component
                MultizoneCompHorOpeningData(i).FlowCoef = coeff;            // Air Mass Flow Coefficient When Window or Door Is Closed
                MultizoneCompHorOpeningData(i).FlowExpo = expnt;            // Air Mass Flow exponent When Window or Door Is Closed
                MultizoneCompHorOpeningData(i).Slope = angle;               // Sloping plane angle
                MultizoneCompHorOpeningData(i).DischCoeff = dischargeCoeff; // Discharge coefficient at full opening

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(thisObjectName) == elements.end()) {
                    elements[thisObjectName] = &MultizoneCompHorOpeningData(i); // Yet another workaround
                } else {
                    ShowSevereError(
                        m_state,
                        format("{}: {}: Duplicated airflow element names are found = \"{}\".", RoutineName, CurrentModuleObject, thisObjectName));
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // *** Read AirflowNetwork simulation surface effective leakage area component
        CurrentModuleObject = "AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea";
        AirflowNetworkNumOfSurELA =
            m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                   // Temporary workaround
            MultizoneSurfaceELAData.allocate(AirflowNetworkNumOfSurELA); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());

                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 ela{fields.at("effective_leakage_area")};
                Real64 cd{1.0};
                if (fields.find("discharge_coefficient") != fields.end()) {
                    cd = fields.at("discharge_coefficient").get<Real64>();
                }
                Real64 dp{4.0};
                if (fields.find("reference_pressure_difference") != fields.end()) {
                    dp = fields.at("reference_pressure_difference").get<Real64>();
                }
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent").get<Real64>();
                }

                MultizoneSurfaceELAData(i).name = thisObjectName; // Name of surface effective leakage area component
                MultizoneSurfaceELAData(i).ELA = ela;             // Effective leakage area
                MultizoneSurfaceELAData(i).DischCoeff = cd;       // Discharge coefficient
                MultizoneSurfaceELAData(i).RefDeltaP = dp;        // Reference pressure difference
                MultizoneSurfaceELAData(i).FlowExpo = expnt;      // Air Mass Flow exponent
                MultizoneSurfaceELAData(i).TestDeltaP = 0.0;      // Testing pressure difference
                MultizoneSurfaceELAData(i).TestDisCoef = 0.0;     // Testing Discharge coefficient

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(thisObjectName) == elements.end()) {
                    elements[thisObjectName] = &MultizoneSurfaceELAData(i); // Yet another workaround
                } else {
                    ShowSevereError(
                        m_state,
                        format("{}: {}: Duplicated airflow element names are found = \"{}\".", RoutineName, CurrentModuleObject, thisObjectName));
                    success = false;
                }

                ++i;
            }
        }

        // *** Read AirflowNetwork simulation specified flow components
        CurrentModuleObject = "AirflowNetwork:MultiZone:SpecifiedFlowRate";
        AirflowNetworkNumOfSFR =
            m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i_mass = 0; // Temporary workaround that increasingly looks like the long term solution
            int i_vol = 0;
            auto &instancesValue = instances.value();

            instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());

                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 flow_rate{fields.at("air_flow_value")};
                bool is_mass_flow = true;
                if (fields.find("air_flow_units") != fields.end()) {
                    if (fields.at("air_flow_units") != "MassFlow") {
                        is_mass_flow = false;
                    }
                }

                // Check for name overlaps
                if (elements.find(thisObjectName) != elements.end()) {
                    ShowSevereError(
                        m_state,
                        format("{}: {}: Duplicated airflow element names are found = \"{}\".", RoutineName, CurrentModuleObject, thisObjectName));
                    success = false;
                }

                if (is_mass_flow) {
                    SpecifiedMassFlowData.emplace_back();
                    SpecifiedMassFlowData[i_mass].name = thisObjectName;
                    SpecifiedMassFlowData[i_mass].mass_flow = flow_rate;
                    ++i_mass;
                } else {
                    SpecifiedVolumeFlowData.emplace_back();
                    SpecifiedVolumeFlowData[i_vol].name = thisObjectName;
                    SpecifiedVolumeFlowData[i_vol].volume_flow = flow_rate;
                    ++i_vol;
                }
            }
            for (auto &afe : SpecifiedMassFlowData) {
                elements[afe.name] = &afe; // Yet another workaround
            }
            for (auto &afe : SpecifiedVolumeFlowData) {
                elements[afe.name] = &afe; // Yet another workaround
            }
        }

        // Read AirflowNetwork Distribution system component: duct leakage
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:Leak";
        DisSysNumOfLeaks = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                     // Temporary workaround
            DisSysCompLeakData.allocate(DisSysNumOfLeaks); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 coeff{fields.at("air_mass_flow_coefficient")};
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent").get<Real64>();
                }

                DisSysCompLeakData(i).name = thisObjectName; // Name of duct leak component
                DisSysCompLeakData(i).FlowCoef = coeff;      // Air Mass Flow Coefficient
                DisSysCompLeakData(i).FlowExpo = expnt;      // Air Mass Flow exponent

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(thisObjectName) == elements.end()) {
                    elements[thisObjectName] = &DisSysCompLeakData(i); // Yet another workaround
                } else {
                    ShowSevereError(
                        m_state,
                        format("{}: {}: Duplicated airflow element names are found = \"{}\".", RoutineName, CurrentModuleObject, thisObjectName));
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork Distribution system component: duct effective leakage ratio
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:LeakageRatio";
        DisSysNumOfELRs = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                   // Temporary workaround
            DisSysCompELRData.allocate(DisSysNumOfELRs); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 elr{fields.at("effective_leakage_ratio")};
                Real64 maxflow{fields.at("maximum_flow_rate")};
                Real64 dp{fields.at("reference_pressure_difference")};
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent").get<Real64>();
                }

                DisSysCompELRData(i).name = thisObjectName;                             // Name of duct effective leakage ratio component
                DisSysCompELRData(i).ELR = elr;                                         // Value of effective leakage ratio
                DisSysCompELRData(i).FlowRate = maxflow * m_state.dataEnvrn->StdRhoAir; // Maximum airflow rate
                DisSysCompELRData(i).RefPres = dp;                                      // Reference pressure difference
                DisSysCompELRData(i).FlowExpo = expnt;                                  // Air Mass Flow exponent

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(thisObjectName) == elements.end()) {
                    elements[thisObjectName] = &DisSysCompELRData(i); // Yet another workaround
                } else {
                    ShowSevereError(
                        m_state,
                        format("{}: {}: Duplicated airflow element names are found = \"{}\".", RoutineName, CurrentModuleObject, thisObjectName));
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork Distribution system component: duct
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:Duct";
        DisSysNumOfDucts = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                     // Temporary workaround
            DisSysCompDuctData.allocate(DisSysNumOfDucts); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 L{fields.at("duct_length")};
                Real64 D{fields.at("hydraulic_diameter")};
                Real64 A{fields.at("cross_section_area")};
                Real64 e{0.0009};
                if (fields.find("surface_roughness") != fields.end()) {
                    e = fields.at("surface_roughness").get<Real64>();
                }
                Real64 dlc{0.0};
                if (fields.find("coefficient_for_local_dynamic_loss_due_to_fitting") != fields.end()) {
                    dlc = fields.at("coefficient_for_local_dynamic_loss_due_to_fitting").get<Real64>();
                }
                Real64 U{0.943};
                if (fields.find("heat_transmittance_coefficient_u_factor_for_duct_wall_construction") != fields.end()) {
                    U = fields.at("heat_transmittance_coefficient_u_factor_for_duct_wall_construction").get<Real64>();
                }
                Real64 Um{0.001};
                if (fields.find("overall_moisture_transmittance_coefficient_from_air_to_air") != fields.end()) {
                    Um = fields.at("overall_moisture_transmittance_coefficient_from_air_to_air").get<Real64>();
                }
                Real64 hout{0.0};
                if (fields.find("outside_convection_coefficient") != fields.end()) {
                    hout = fields.at("outside_convection_coefficient").get<Real64>();
                }
                Real64 hin{0.0};
                if (fields.find("inside_convection_coefficient") != fields.end()) {
                    hin = fields.at("inside_convection_coefficient").get<Real64>();
                }

                DisSysCompDuctData(i).name = thisObjectName;   // Name of duct effective leakage ratio component
                DisSysCompDuctData(i).L = L;                   // Duct length [m]
                DisSysCompDuctData(i).hydraulicDiameter = D;   // Hydraulic diameter [m]
                DisSysCompDuctData(i).A = A;                   // Cross section area [m2]
                DisSysCompDuctData(i).roughness = e;           // Surface roughness [m]
                DisSysCompDuctData(i).TurDynCoef = dlc;        // Turbulent dynamic loss coefficient
                DisSysCompDuctData(i).UThermConduct = U;       // Conduction heat transmittance [W/m2.K]
                DisSysCompDuctData(i).UMoisture = Um;          // Overall moisture transmittance [kg/m2]
                DisSysCompDuctData(i).OutsideConvCoeff = hout; // Outside convection coefficient [W/m2.K]
                DisSysCompDuctData(i).InsideConvCoeff = hin;   // Inside convection coefficient [W/m2.K]
                DisSysCompDuctData(i).MThermal = 0.0;          // Thermal capacity [J/K]
                DisSysCompDuctData(i).MMoisture = 0.0;         // Moisture capacity [kg]
                DisSysCompDuctData(i).LamDynCoef = 64.0;       // Laminar dynamic loss coefficient
                DisSysCompDuctData(i).LamFriCoef = dlc;        // Laminar friction loss coefficient
                DisSysCompDuctData(i).InitLamCoef = 128.0;     // Coefficient of linear initialization
                DisSysCompDuctData(i).RelRough = e / D;        // e/D: relative roughness
                DisSysCompDuctData(i).RelL = L / D;            // L/D: relative length
                DisSysCompDuctData(i).A1 = 1.14 - 0.868589 * std::log(DisSysCompDuctData(i).RelRough); // 1.14 - 0.868589*ln(e/D)
                DisSysCompDuctData(i).g = DisSysCompDuctData(i).A1;                                    // 1/sqrt(Darcy friction factor)

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(thisObjectName) == elements.end()) {
                    elements[thisObjectName] = &DisSysCompDuctData(i); // Yet another workaround
                } else {
                    ShowSevereError(
                        m_state,
                        format("{}: {}: Duplicated airflow element names are found = \"{}\".", RoutineName, CurrentModuleObject, thisObjectName));
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork Distribution system component: constant volume fan
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:Fan";
        DisSysNumOfCVFs = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject);
        if (DisSysNumOfCVFs > 0 && DisSysNumOfCVFs != m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "AirLoopHVAC")) {
            ShowSevereError(m_state, format("The number of entered AirflowNetwork:Distribution:Component:Fan objects is {}", DisSysNumOfCVFs));
            ShowSevereError(m_state,
                            format("The number of entered AirLoopHVAC objects is {}",
                                   m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "AirLoopHVAC")));
            ShowContinueError(m_state, "Both numbers should be equal. Please check your inputs.");
            success = false;
        }

        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                   // Temporary workaround
            DisSysCompCVFData.allocate(DisSysNumOfCVFs); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                std::string fan_name = Util::makeUPPER(fields.at("fan_name").get<std::string>());

                int fanIndex;
                Real64 flowRate = 0.0;
                int inletNode;
                int outletNode;

                HVAC::FanType fanType = static_cast<HVAC::FanType>(
                    getEnumValue(HVAC::fanTypeNamesUC, Util::makeUPPER(fields.at("supply_fan_object_type").get<std::string>())));

                HVAC::FanType fanType2 = HVAC::FanType::Invalid;

                if (fanType == HVAC::FanType::SystemModel) {
                    fanIndex = Fans::GetFanIndex(m_state, fan_name);
                    if (fanIndex < 0) {
                        ShowSevereError(m_state, "...occurs in " + CurrentModuleObject + " = " + DisSysCompCVFData(i).name);
                        success = false;
                    } else {
                        auto *fanSys = dynamic_cast<Fans::FanSystem *>(m_state.dataFans->fans(fanIndex));
                        assert(fanSys != nullptr);
                        flowRate = fanSys->maxAirFlowRate * m_state.dataEnvrn->StdRhoAir;
                        DisSysCompCVFData(i).FanModelFlag = true;
                        inletNode = fanSys->inletNodeNum;
                        outletNode = fanSys->outletNodeNum;
                        if (fanSys->speedControl == Fans::SpeedControl::Continuous) {
                            fanType2 = HVAC::FanType::VAV;
                            VAVSystem = true;
                        } else {
                            fanType2 = HVAC::FanType::OnOff;
                        }
                        supplyFanType = fanType2;
                    }

                } else {
                    fanIndex = GetFanIndex(m_state, fan_name);

                    if (fanIndex == 0) {
                        ErrorObjectHeader eoh{RoutineName, CurrentModuleObject, DisSysCompCVFData(i).name};
                        ShowSevereItemNotFound(m_state, eoh, "Fan Name", fan_name);
                        success = false;
                    }

                    auto *fan = m_state.dataFans->fans(fanIndex);
                    flowRate = fan->maxAirFlowRate * m_state.dataEnvrn->StdRhoAir;

                    fanType2 = fan->type;
                    supplyFanType = fanType2;
                }

                if (!(fanType2 == HVAC::FanType::Constant || fanType2 == HVAC::FanType::OnOff || fanType2 == HVAC::FanType::VAV)) {
                    ShowSevereError(
                        m_state,
                        format("{}The Supply Fan Object Type in {} = {} is not a valid fan type.", RoutineName, CurrentModuleObject, thisObjectName));
                    ShowContinueError(m_state, "Valid fan types are  Fan:ConstantVolume, Fan:OnOff, Fan:VariableVolume, or Fan:SystemModel.");
                    success = false;
                } else {
                    if (fanType == HVAC::FanType::Constant && fanType2 == HVAC::FanType::OnOff) {
                        ShowSevereError(m_state, "The Supply Fan Object Type defined in " + CurrentModuleObject + " is Fan:ConstantVolume");
                        ShowContinueError(m_state, "The Supply Fan Object Type defined in an AirLoopHVAC is Fan:OnOff");
                        success = false;
                    } else if (fanType == HVAC::FanType::OnOff && fanType2 == HVAC::FanType::Constant) {
                        ShowSevereError(m_state, "The Supply Fan Object Type defined in " + CurrentModuleObject + " is Fan:SimpleOnOff");
                        ShowContinueError(m_state, "The Supply Fan Object Type defined in an AirLoopHVAC is Fan:ConstantVolume");
                        success = false;
                    }
                }
                bool ErrorsFound{false};
                auto *fan = m_state.dataFans->fans(fanIndex);
                if (fanType2 == HVAC::FanType::Constant) {
                    inletNode = fan->inletNodeNum;
                    outletNode = fan->outletNodeNum;
                }
                if (fanType2 == HVAC::FanType::OnOff && !DisSysCompCVFData(i).FanModelFlag) {
                    inletNode = fan->inletNodeNum;
                    outletNode = fan->outletNodeNum;
                }
                if (fanType2 == HVAC::FanType::VAV && !DisSysCompCVFData(i).FanModelFlag) {
                    inletNode = fan->inletNodeNum;
                    outletNode = fan->outletNodeNum;
                    VAVSystem = true;
                }

                if (ErrorsFound) {
                    success = false;
                }

                DisSysCompCVFData(i).name = fan_name; // Name of duct effective leakage ratio component
                DisSysCompCVFData(i).Ctrl = 1.0;      // Control ratio
                DisSysCompCVFData(i).FanIndex = fanIndex;
                DisSysCompCVFData(i).FlowRate = flowRate;
                DisSysCompCVFData(i).fanType = fanType2;
                DisSysCompCVFData(i).InletNode = inletNode;
                DisSysCompCVFData(i).OutletNode = outletNode;

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(fan_name) == elements.end()) {
                    elements[fan_name] = &DisSysCompCVFData(i); // Yet another workaround
                } else {
                    ShowSevereError(
                        m_state, format("{}: {}: Duplicated airflow element names are found = \"{}\".", RoutineName, CurrentModuleObject, fan_name));
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork Distribution system component: coil
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:Coil";
        DisSysNumOfCoils = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                     // Temporary workaround
            DisSysCompCoilData.allocate(DisSysNumOfCoils); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                // auto const &thisObjectName = Util::makeUPPER(instance.key());
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                std::string coil_name = fields.at("coil_name").get<std::string>();
                std::string coil_type = fields.at("coil_object_type").get<std::string>();
                Real64 L{fields.at("air_path_length")};
                Real64 D{fields.at("air_path_hydraulic_diameter")};

                DisSysCompCoilData(i).name = Util::makeUPPER(coil_name); // Name of associated EPlus coil component
                DisSysCompCoilData(i).EPlusType = coil_type;             // coil type
                DisSysCompCoilData(i).L = L;                             // Air path length
                DisSysCompCoilData(i).hydraulicDiameter = D;             // Air path hydraulic diameter

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(DisSysCompCoilData(i).name) == elements.end()) {
                    elements[DisSysCompCoilData(i).name] = &DisSysCompCoilData(i); // Yet another workaround
                } else {
                    ShowSevereError(m_state,
                                    format("{}: {}: Duplicated airflow element names are found = \"{}\".",
                                           RoutineName,
                                           CurrentModuleObject,
                                           DisSysCompCoilData(i).name));
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork Distribution system component: heat exchanger
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:HeatExchanger";
        DisSysNumOfHXs = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                 // Temporary workaround
            DisSysCompHXData.allocate(DisSysNumOfHXs); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                // auto const &thisObjectName = Util::makeUPPER(instance.key());
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                std::string hx_name = fields.at("heatexchanger_name").get<std::string>();
                std::string hx_type = fields.at("heatexchanger_object_type").get<std::string>();
                Real64 L{fields.at("air_path_length")};
                Real64 D{fields.at("air_path_hydraulic_diameter")};

                DisSysCompHXData(i).name = Util::makeUPPER(hx_name); // Name of associated EPlus heat exchange component
                DisSysCompHXData(i).EPlusType = hx_type;             // coil type
                DisSysCompHXData(i).L = L;                           // Air path length
                DisSysCompHXData(i).hydraulicDiameter = D;           // Air path hydraulic diameter
                DisSysCompHXData(i).CoilParentExists = HVACHXAssistedCoolingCoil::VerifyHeatExchangerParent(m_state, hx_type, hx_name);

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(DisSysCompHXData(i).name) == elements.end()) {
                    elements[DisSysCompHXData(i).name] = &DisSysCompHXData(i); // Yet another workaround
                } else {
                    ShowSevereError(m_state,
                                    format("{}: {}: Duplicated airflow element names are found = \"{}\".",
                                           RoutineName,
                                           CurrentModuleObject,
                                           DisSysCompHXData(i).name));
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }
                ++i;
            }
        }

        // Read AirflowNetwork Distribution system component: terminal unit
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:TerminalUnit";
        DisSysNumOfTermUnits = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                             // Temporary workaround
            DisSysCompTermUnitData.allocate(DisSysNumOfTermUnits); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                // auto const &thisObjectName = Util::makeUPPER(instance.key());
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                std::string tu_name = fields.at("terminal_unit_name").get<std::string>();
                std::string tu_type = fields.at("terminal_unit_object_type").get<std::string>();
                Real64 L{fields.at("air_path_length")};
                Real64 D{fields.at("air_path_hydraulic_diameter")};

                DisSysCompTermUnitData(i).name = Util::makeUPPER(tu_name); // Name of associated EPlus coil component
                DisSysCompTermUnitData(i).EPlusType = tu_type;             // Terminal unit type
                DisSysCompTermUnitData(i).L = L;                           // Air path length
                DisSysCompTermUnitData(i).hydraulicDiameter = D;           // Air path hydraulic diameter

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(DisSysCompTermUnitData(i).name) == elements.end()) {
                    elements[DisSysCompTermUnitData(i).name] = &DisSysCompTermUnitData(i); // Yet another workaround
                } else {
                    ShowSevereError(m_state,
                                    format("{}: {}: Duplicated airflow element names are found = \"{}\".",
                                           RoutineName,
                                           CurrentModuleObject,
                                           DisSysCompTermUnitData(i).name));
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Get input data of constant pressure drop component
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:ConstantPressureDrop";
        DisSysNumOfCPDs = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject); // Temporary workaround
        instances = m_state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != m_state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                   // Temporary workaround
            DisSysCompCPDData.allocate(DisSysNumOfCPDs); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = Util::makeUPPER(instance.key());
                m_state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 dp{fields.at("pressure_difference_across_the_component")};

                DisSysCompCPDData(i).name = thisObjectName; // Name of constant pressure drop component
                DisSysCompCPDData(i).A = 1.0;               // cross section area
                DisSysCompCPDData(i).DP = dp;               // Pressure difference across the component

                // Add the element to the lookup table, check for name overlaps
                if (elements.find(thisObjectName) == elements.end()) {
                    elements[thisObjectName] = &DisSysCompCPDData(i); // Yet another workaround
                } else {
                    ShowSevereError(
                        m_state,
                        format("{}: {}: Duplicated airflow element names are found = \"{}\".", RoutineName, CurrentModuleObject, thisObjectName));
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        return success;
    }

    void Solver::get_input()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Aug. 2003
        //       MODIFIED       Aug. 2005
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reads inputs of air distribution system

        // Using/Aliasing
        using Curve::GetCurveIndex;
        using DataLoopNode::ObjectIsParent;
        using HVACHXAssistedCoolingCoil::VerifyHeatExchangerParent;
        using MixedAir::GetOAMixerNumber;
        using NodeInputManager::GetOnlySingleNode;
        using OutAirNodeManager::SetOutAirNodes;
        using RoomAir::GetRAFNNodeNum;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("AirflowNetwork::Solver::get_input: "); // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int n;
        int k;
        int count;
        bool NodeFound;
        bool found;
        int NumAPL;
        Array1D_string CompName(2);
        std::string SimAirNetworkKey;

        // Declare variables used in this subroutine for debug purpose
        Array1D_int ZoneCheck;
        Array1D_int ZoneBCCheck;

        int NumAlphas;  // Number of Alphas for each GetObjectItem call
        int NumNumbers; // Number of Numbers for each GetObjectItem call
        int IOStatus;   // Used in GetObjectItem
        std::string CurrentModuleObject;
        Array1D_string Alphas;         // Alpha input items for object
        Array1D_string cAlphaFields;   // Alpha field names
        Array1D_string cNumericFields; // Numeric field names
        Array1D<Real64> Numbers;       // Numeric input items for object
        Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.
        int MaxNums(0);                // Maximum number of numeric input fields
        int MaxAlphas(0);              // Maximum number of alpha input fields
        int TotalArgs(0);              // Total number of alpha and numeric arguments (max) for a
        Real64 minHeight;
        Real64 maxHeight;
        Real64 baseratio;

        auto &Node(m_state.dataLoopNodes->Node);

        // Formats
        static constexpr std::string_view Format_110(
            "! <AirflowNetwork Model:Control>, No Multizone or Distribution/Multizone with Distribution/Multizone "
            "without Distribution/Multizone with Distribution only during Fan Operation\n");
        static constexpr std::string_view Format_120("AirflowNetwork Model:Control,{}\n");

        // Set the maximum numbers of input fields
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            m_state, "AirflowNetwork:SimulationControl", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(m_state, "AirflowNetwork:MultiZone:Zone", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            m_state, "AirflowNetwork:MultiZone:Surface", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            m_state, "AirflowNetwork:MultiZone:Component:DetailedOpening", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            m_state, "AirflowNetwork:MultiZone:ExternalNode", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            m_state, "AirflowNetwork:MultiZone:WindPressureCoefficientArray", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            m_state, "AirflowNetwork:MultiZone:WindPressureCoefficientValues", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            m_state, "AirflowNetwork:Distribution:Node", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            m_state, "AirflowNetwork:Distribution:DuctViewFactors", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            m_state, "AirflowNetwork:Distribution:Linkage", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            m_state, "AirflowNetwork:OccupantVentilationControl", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(m_state, "AirflowNetwork:IntraZone:Node", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            m_state, "AirflowNetwork:IntraZone:Linkage", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            m_state, "AirflowNetwork:ZoneControl:PressureController", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        m_state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            m_state, "AirflowNetwork:Distribution:DuctSizing", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        Alphas.allocate(MaxAlphas);
        cAlphaFields.allocate(MaxAlphas);
        cNumericFields.allocate(MaxNums);
        Numbers.dimension(MaxNums, 0.0);
        lAlphaBlanks.dimension(MaxAlphas, true);
        lNumericBlanks.dimension(MaxNums, true);

        bool ErrorsFound = false;

        auto &Zone(m_state.dataHeatBal->Zone);

        int NumDuctLossConduction = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "Duct:Loss:Conduction");
        int NumDuctLossLeakage = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "Duct:Loss:Leakage");
        int NumDuctLossMakeupAir = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "Duct:Loss:MakeupAir");
        if ((NumDuctLossConduction + NumDuctLossLeakage + NumDuctLossMakeupAir) > 0) {
            simulation_control.DuctLoss = true;
        }

        // Read AirflowNetwork OccupantVentilationControl before reading other AirflowNetwork objects, so that this object can be called by other
        // simple ventilation objects
        CurrentModuleObject = "AirflowNetwork:OccupantVentilationControl";
        AirflowNetworkNumOfOccuVentCtrls = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject);
        if (AirflowNetworkNumOfOccuVentCtrls > 0) {
            OccupantVentilationControl.allocate(AirflowNetworkNumOfOccuVentCtrls);
            for (int i = 1; i <= AirflowNetworkNumOfOccuVentCtrls; ++i) {
                m_state.dataInputProcessing->inputProcessor->getObjectItem(m_state,
                                                                           CurrentModuleObject,
                                                                           i,
                                                                           Alphas,
                                                                           NumAlphas,
                                                                           Numbers,
                                                                           NumNumbers,
                                                                           IOStatus,
                                                                           lNumericBlanks,
                                                                           lAlphaBlanks,
                                                                           cAlphaFields,
                                                                           cNumericFields);
                OccupantVentilationControl(i).Name = Alphas(1); // Name of object
                OccupantVentilationControl(i).MinOpeningTime = Numbers(1);
                if (OccupantVentilationControl(i).MinOpeningTime < 0.0) {
                    // Code will never be executed, validation will catch invalid input
                    ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(1) + " < 0.0");
                    ShowContinueError(m_state,
                                      format("..Input value = {:.1R}, Value will be reset to 0.0", OccupantVentilationControl(i).MinOpeningTime));
                    ShowContinueError(m_state, "..for " + cAlphaFields(1) + " = \"" + OccupantVentilationControl(i).Name);
                    OccupantVentilationControl(i).MinOpeningTime = 0.0;
                }
                OccupantVentilationControl(i).MinClosingTime = Numbers(2);
                if (OccupantVentilationControl(i).MinClosingTime < 0.0) {
                    // Code will never be executed, validation will catch invalid input
                    ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(2) + " < 0.0");
                    ShowContinueError(m_state,
                                      format("..Input value = {:.1R}, Value will be reset to 0.0", OccupantVentilationControl(i).MinClosingTime));
                    ShowContinueError(m_state, "..for " + cAlphaFields(1) + " = \"" + OccupantVentilationControl(i).Name);
                    OccupantVentilationControl(i).MinClosingTime = 0.0;
                }
                if (NumAlphas == 1 && NumNumbers == 2) {
                    OccupantVentilationControl(i).MinTimeControlOnly = true;
                }
                if (!lAlphaBlanks(2)) {
                    OccupantVentilationControl(i).ComfortLowTempCurveName = Alphas(2);
                    OccupantVentilationControl(i).ComfortLowTempCurveNum = GetCurveIndex(m_state, Alphas(2)); // convert curve name to number
                    if (OccupantVentilationControl(i).ComfortLowTempCurveNum == 0) {
                        OccupantVentilationControl(i).MinTimeControlOnly = true;
                        ShowWarningError(m_state,
                                         format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(2) +
                                             " not found = " + OccupantVentilationControl(i).ComfortLowTempCurveName);
                        ShowContinueError(m_state, "..for specified " + cAlphaFields(1) + " = " + Alphas(1));
                        ShowContinueError(
                            m_state,
                            "Thermal comfort will not be performed and minimum opening and closing times are checked only. Simulation continues.");
                    } else {
                        ErrorsFound |= Curve::CheckCurveDims(m_state,
                                                             OccupantVentilationControl(i).ComfortLowTempCurveNum, // Curve index
                                                             {1},                                                  // Valid dimensions
                                                             RoutineName,                                          // Routine name
                                                             CurrentModuleObject,                                  // Object Type
                                                             OccupantVentilationControl(i).Name,                   // Object Name
                                                             cAlphaFields(2));                                     // Field Name
                    }
                }
                if (!lAlphaBlanks(3)) {
                    OccupantVentilationControl(i).ComfortHighTempCurveName = Alphas(3);
                    OccupantVentilationControl(i).ComfortHighTempCurveNum = GetCurveIndex(m_state, Alphas(3)); // convert curve name to number
                    if (OccupantVentilationControl(i).ComfortHighTempCurveNum > 0) {
                        ErrorsFound |= Curve::CheckCurveDims(m_state,
                                                             OccupantVentilationControl(i).ComfortHighTempCurveNum, // Curve index
                                                             {1},                                                   // Valid dimensions
                                                             RoutineName,                                           // Routine name
                                                             CurrentModuleObject,                                   // Object Type
                                                             OccupantVentilationControl(i).Name,                    // Object Name
                                                             cAlphaFields(3));                                      // Field Name
                    } else {
                        ShowWarningError(m_state,
                                         format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(3) +
                                             " not found = " + OccupantVentilationControl(i).ComfortHighTempCurveName);
                        ShowContinueError(m_state, "..for specified " + cAlphaFields(1) + " = " + Alphas(1));
                        ShowContinueError(m_state, "A single curve of thermal comfort low temperature is used only. Simulation continues.");
                    }
                }
                if (OccupantVentilationControl(i).ComfortHighTempCurveNum > 0) {
                    OccupantVentilationControl(i).ComfortBouPoint = Numbers(3);
                    if (OccupantVentilationControl(i).ComfortBouPoint < 0.0) {
                        // Code will never be executed, validation will catch invalid input
                        ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(3) + " < 0.0");
                        ShowContinueError(
                            m_state,
                            format("..Input value = {:.1R}, Value will be reset to 10.0 as default", OccupantVentilationControl(i).ComfortBouPoint));
                        ShowContinueError(m_state, "..for " + cAlphaFields(1) + " = \"" + OccupantVentilationControl(i).Name);
                        OccupantVentilationControl(i).ComfortBouPoint = 10.0;
                    }
                }
                // Check continuity of both curves at boundary point
                if (OccupantVentilationControl(i).ComfortLowTempCurveNum > 0 && OccupantVentilationControl(i).ComfortHighTempCurveNum) {
                    if (std::abs(CurveValue(m_state, OccupantVentilationControl(i).ComfortLowTempCurveNum, Numbers(3)) -
                                 CurveValue(m_state, OccupantVentilationControl(i).ComfortHighTempCurveNum, Numbers(3))) > 0.1) {
                        ShowSevereError(m_state,
                                        format(RoutineName) + CurrentModuleObject +
                                            " object: The difference of both curve values at boundary point > 0.1");
                        ShowContinueError(m_state, "Both curve names are = " + cAlphaFields(2) + " and " + cAlphaFields(3));
                        ShowContinueError(m_state,
                                          format("The input value of {} = {:.1R}", cNumericFields(3), OccupantVentilationControl(i).ComfortBouPoint));
                        ErrorsFound = true;
                    }
                }
                if (!lNumericBlanks(4)) {
                    OccupantVentilationControl(i).MaxPPD = Numbers(4);
                    if (OccupantVentilationControl(i).MaxPPD < 0.0 || OccupantVentilationControl(i).MaxPPD > 100.0) {
                        // Code will never be executed, validation will catch invalid input
                        ShowWarningError(m_state,
                                         format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(4) + " beyond 0.0 and 100.0");
                        ShowContinueError(
                            m_state, format("..Input value = {:.1R}, Value will be reset to 10.0 as default", OccupantVentilationControl(i).MaxPPD));
                        ShowContinueError(m_state, "..for " + cAlphaFields(1) + " = \"" + OccupantVentilationControl(i).Name);
                        OccupantVentilationControl(i).MaxPPD = 10.0;
                    }
                }
                if (!lAlphaBlanks(4)) {
                    if (Util::SameString(Alphas(4), "Yes")) {
                        OccupantVentilationControl(i).OccupancyCheck = true;
                    } else if (Util::SameString(Alphas(4), "No")) {
                        OccupantVentilationControl(i).OccupancyCheck = false;
                    } else {
                        // Code will never be executed, validation will catch invalid input
                        ShowSevereError(m_state,
                                        format(RoutineName) + CurrentModuleObject + "=\"" + Alphas(1) + "\" invalid " + cAlphaFields(2) + "=\"" +
                                            Alphas(2) + "\" illegal key.");
                        ShowContinueError(m_state, "Valid keys are: Yes or No");
                        ErrorsFound = true;
                    }
                }
                if (!lAlphaBlanks(5)) {
                    OccupantVentilationControl(i).OpeningProbSchName = Alphas(5); // a schedule name for opening probability
                    OccupantVentilationControl(i).openingProbSched = Sched::GetSchedule(m_state, OccupantVentilationControl(i).OpeningProbSchName);
                    if (OccupantVentilationControl(i).openingProbSched == nullptr) {
                        ShowSevereError(m_state,
                                        format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(5) +
                                            " not found = " + OccupantVentilationControl(i).OpeningProbSchName);
                        ShowContinueError(m_state, "..for specified " + cAlphaFields(1) + " = " + Alphas(1));
                        ErrorsFound = true;
                    }
                }
                if (!lAlphaBlanks(6)) {
                    OccupantVentilationControl(i).ClosingProbSchName = Alphas(6); // a schedule name for closing probability
                    OccupantVentilationControl(i).closingProbSched = Sched::GetSchedule(m_state, OccupantVentilationControl(i).ClosingProbSchName);
                    if (OccupantVentilationControl(i).closingProbSched == nullptr) {
                        ShowSevereError(m_state,
                                        format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(6) +
                                            " not found = " + OccupantVentilationControl(i).ClosingProbSchName);
                        ShowContinueError(m_state, "..for specified " + cAlphaFields(1) + " = " + Alphas(1));
                        ErrorsFound = true;
                    }
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(m_state, format("{}Errors found getting inputs. Previous error(s) cause program termination.", RoutineName));
        }

        // *** Read AirflowNetwork simulation parameters
        CurrentModuleObject = "AirflowNetwork:SimulationControl";
        NumAirflowNetwork = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject);
        if (NumAirflowNetwork == 0) {
            if (m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "AirflowNetwork:MultiZone:Zone") >= 1 &&
                m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "AirflowNetwork:MultiZone:Surface") >= 2) {
                control_defaulted = true;
                simulation_control.name = "AFNDefaultControl";
                simulation_control.type = ControlType::MultizoneWithoutDistribution;
                simulation_control.WPCCntr = "SURFACEAVERAGECALCULATION";
                simulation_control.HeightOption = "OPENINGHEIGHT";
                simulation_control.BldgType = "LOWRISE";
                simulation_control.InitType = "ZERONODEPRESSURES";
                simulation_control.temperature_height_dependence = false;
                simulation_control.solver = SimulationControl::Solver::SkylineLU;
                // Use default values for numerical fields
                simulation_control.maximum_iterations = 500;
                simulation_control.relative_convergence_tolerance = 1.E-4;
                simulation_control.absolute_convergence_tolerance = 1.E-6;
                simulation_control.convergence_acceleration_limit = -0.5;
                simulation_control.azimuth = 0.0;
                simulation_control.aspect_ratio = 1.0;
                simulation_control.MaxPressure = 500.0; // Maximum pressure difference by default
                SimAirNetworkKey = "MultizoneWithoutDistribution";
                simulation_control.InitFlag = 1;
                ShowWarningError(m_state, format("{}{} object is not found ", RoutineName, CurrentModuleObject));
                ShowContinueError(m_state, "..The default behaviour values are assigned. Please see details in Input Output Reference.");
            } else if (simulation_control.DuctLoss) {
                print(m_state.files.eio, Format_110);
                print(m_state.files.eio, Format_120, "Duct loss simulation without AFN");
            } else {
                simulation_control.type = ControlType::NoMultizoneOrDistribution;
                print(m_state.files.eio, Format_110);
                print(m_state.files.eio, Format_120, "NoMultizoneOrDistribution");
                return;
            }
        }
        if (NumAirflowNetwork > 1) {
            ShowFatalError(m_state, format("{}Only one (\"1\") {} object per simulation is allowed.", RoutineName, CurrentModuleObject));
        }

        if (!control_defaulted && !simulation_control.DuctLoss) {
            m_state.dataInputProcessing->inputProcessor->getObjectItem(m_state,
                                                                       CurrentModuleObject,
                                                                       NumAirflowNetwork,
                                                                       Alphas,
                                                                       NumAlphas,
                                                                       Numbers,
                                                                       NumNumbers,
                                                                       IOStatus,
                                                                       lNumericBlanks,
                                                                       lAlphaBlanks,
                                                                       cAlphaFields,
                                                                       cNumericFields);

            simulation_control.name = Alphas(1);
            simulation_control.WPCCntr = Alphas(3);
            simulation_control.HeightOption = Alphas(4);
            simulation_control.BldgType = Alphas(5);

            // Retrieve flag allowing the support of zone equipment
            simulation_control.allow_unsupported_zone_equipment = false;
            if (Util::SameString(Alphas(9), "Yes")) {
                simulation_control.allow_unsupported_zone_equipment = true;
            }

            // Find a flag for possible combination of vent and distribution system
            // This SELECT_CASE_var will go on input refactor, no need to fix
            {
                auto const SELECT_CASE_var(Util::makeUPPER(Alphas(2)));
                if (SELECT_CASE_var == "NOMULTIZONEORDISTRIBUTION") {
                    simulation_control.type = ControlType::NoMultizoneOrDistribution;
                    SimAirNetworkKey = "NoMultizoneOrDistribution";
                } else if (SELECT_CASE_var == "MULTIZONEWITHOUTDISTRIBUTION") {
                    simulation_control.type = ControlType::MultizoneWithoutDistribution;
                    SimAirNetworkKey = "MultizoneWithoutDistribution";
                } else if (SELECT_CASE_var == "MULTIZONEWITHDISTRIBUTIONONLYDURINGFANOPERATION") {
                    simulation_control.type = ControlType::MultizoneWithDistributionOnlyDuringFanOperation;
                    SimAirNetworkKey = "MultizoneWithDistributionOnlyDuringFanOperation";
                } else { // if (SELECT_CASE_var == "MULTIZONEWITHDISTRIBUTION") {
                    simulation_control.type = ControlType::MultizoneWithDistribution;
                    SimAirNetworkKey = "MultizoneWithDistribution";
                }
            }
        }

        // Determine a convenience boolean or two to simplify the checking
        // The first one is true if distribution is simulated, replaces some > and >= comparisons
        // SimulateAirflowNetwork > AirflowNetworkControlMultizone -> type == ControlType::MultizoneWithDistributionOnlyDuringFanOperation
        //                                                            type == ControlType::MultizoneWithDistribution
        // SimulateAirflowNetwork >= AirflowNetworkControlSimpleADS -> type == ControlType::MultizoneWithDistributionOnlyDuringFanOperation
        //                                                             type == ControlType::MultizoneWithDistribution
        distribution_simulated = simulation_control.type == ControlType::MultizoneWithDistributionOnlyDuringFanOperation ||
                                 simulation_control.type == ControlType::MultizoneWithDistribution;
        // This one is true if the multizone simulation is ALWAYS done
        multizone_always_simulated =
            simulation_control.type == ControlType::MultizoneWithDistribution || simulation_control.type == ControlType::MultizoneWithoutDistribution;

        // Check the number of primary air loops
        if (!simulation_control.DuctLoss) {
            if (distribution_simulated) {
                NumAPL = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "AirLoopHVAC");
                if (NumAPL > 0) {
                    LoopPartLoadRatio.allocate(NumAPL);
                    LoopOnOffFanRunTimeFraction.allocate(NumAPL);
                    LoopOnOffFlag.allocate(NumAPL);
                    LoopPartLoadRatio = 0.0;
                    LoopOnOffFanRunTimeFraction = 0.0;
                    LoopOnOffFlag = false;
                }
            }
            print(m_state.files.eio, Format_110);
            print(m_state.files.eio, Format_120, SimAirNetworkKey);

            if (control_defaulted) {
                cAlphaFields(2) = "AirflowNetwork Control";
            }

            // there are any objects from infiltration, ventilation, mixing and cross mixing
            if (simulation_control.type == ControlType::NoMultizoneOrDistribution ||
                simulation_control.type == ControlType::MultizoneWithDistributionOnlyDuringFanOperation) {
                if (m_state.dataHeatBal->TotInfiltration + m_state.dataHeatBal->TotVentilation + m_state.dataHeatBal->TotMixing +
                        m_state.dataHeatBal->TotCrossMixing + m_state.dataHeatBal->TotZoneAirBalance +
                        m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "ZoneEarthtube") +
                        m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "ZoneThermalChimney") +
                        m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "ZoneCoolTower:Shower") ==
                    0) {
                    ShowWarningError(m_state, format("{}{} = \"{}\"", RoutineName, cAlphaFields(2), SimAirNetworkKey));
                    ShowContinueError(
                        m_state,
                        "..but there are no Infiltration, Ventilation, Mixing, Cross Mixing or ZoneAirBalance objects. The simulation continues...");
                }
            }

            // Check whether a user wants to perform SIMPLE calculation only or not
            if (simulation_control.type == ControlType::NoMultizoneOrDistribution) {
                return;
            }

            if (multizone_always_simulated) {
                if (m_state.dataHeatBal->TotInfiltration > 0) {
                    ShowWarningError(m_state, format("{}{} object, ", RoutineName, CurrentModuleObject));
                    ShowContinueError(
                        m_state, "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneInfiltration:* objects are present.");
                    ShowContinueError(m_state, "..ZoneInfiltration objects will not be simulated.");
                }
                if (m_state.dataHeatBal->TotVentilation > 0) {
                    ShowWarningError(m_state, format("{}{} object, ", RoutineName, CurrentModuleObject));
                    ShowContinueError(
                        m_state, "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneVentilation:* objects are present.");
                    ShowContinueError(m_state, "..ZoneVentilation objects will not be simulated.");
                }
                if (m_state.dataHeatBal->TotMixing > 0) {
                    ShowWarningError(m_state, format("{}{} object, ", RoutineName, CurrentModuleObject));
                    ShowContinueError(m_state,
                                      "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneMixing objects are present.");
                    ShowContinueError(m_state, "..ZoneMixing objects will not be simulated.");
                }
                if (m_state.dataHeatBal->TotCrossMixing > 0) {
                    ShowWarningError(m_state, format("{}{} object, ", RoutineName, CurrentModuleObject));
                    ShowContinueError(m_state,
                                      "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneCrossMixing objects are present.");
                    ShowContinueError(m_state, "..ZoneCrossMixing objects will not be simulated.");
                }
                if (m_state.dataHeatBal->TotZoneAirBalance > 0) {
                    ShowWarningError(m_state, format("{}{} object, ", RoutineName, CurrentModuleObject));
                    ShowContinueError(m_state,
                                      "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey +
                                          "\" and ZoneAirBalance:OutdoorAir objects are present.");
                    ShowContinueError(m_state, "..ZoneAirBalance:OutdoorAir objects will not be simulated.");
                }
                if (m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "ZoneEarthtube") > 0) {
                    ShowWarningError(m_state, format("{}{} object, ", RoutineName, CurrentModuleObject));
                    ShowContinueError(m_state,
                                      "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneEarthtube objects are present.");
                    ShowContinueError(m_state, "..ZoneEarthtube objects will not be simulated.");
                }
                if (m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "ZoneThermalChimney") > 0) {
                    ShowWarningError(m_state, format("{}{} object, ", RoutineName, CurrentModuleObject));
                    ShowContinueError(
                        m_state, "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneThermalChimney objects are present.");
                    ShowContinueError(m_state, "..ZoneThermalChimney objects will not be simulated.");
                }
                if (m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "ZoneCoolTower:Shower") > 0) {
                    ShowWarningError(m_state, format("{}{} object, ", RoutineName, CurrentModuleObject));
                    ShowContinueError(
                        m_state, "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneCoolTower:Shower objects are present.");
                    ShowContinueError(m_state, "..ZoneCoolTower:Shower objects will not be simulated.");
                }
            }

            SetOutAirNodes(m_state);
            if (!control_defaulted) {
                bool SimObjectError = false;
                if (Util::SameString(simulation_control.WPCCntr, "Input")) {
                    simulation_control.iWPCCnt = iWPCCntr::Input;
                    if (lAlphaBlanks(4)) {
                        ShowSevereError(m_state, format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(3) + " = INPUT.");
                        ShowContinueError(m_state, ".." + cAlphaFields(4) + " was not entered.");
                        ErrorsFound = true;
                        SimObjectError = true;
                    } else {
                        if (!(Util::SameString(simulation_control.HeightOption, "ExternalNode") ||
                              Util::SameString(simulation_control.HeightOption, "OpeningHeight"))) {
                            ShowSevereError(m_state,
                                            format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(4) + " = " + Alphas(4) +
                                                " is invalid.");
                            ShowContinueError(m_state,
                                              "Valid choices are ExternalNode or OpeningHeight. " + CurrentModuleObject + ": " + cAlphaFields(1) +
                                                  " = " + simulation_control.name);
                            ErrorsFound = true;
                            SimObjectError = true;
                        }
                    }
                } else if (Util::SameString(simulation_control.WPCCntr, "SurfaceAverageCalculation")) {
                    simulation_control.iWPCCnt = iWPCCntr::SurfAvg;
                    if (!(Util::SameString(simulation_control.BldgType, "LowRise") || Util::SameString(simulation_control.BldgType, "HighRise"))) {
                        ShowSevereError(
                            m_state, format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(5) + " = " + Alphas(5) + " is invalid.");
                        ShowContinueError(m_state,
                                          "Valid choices are LowRise or HighRise. " + CurrentModuleObject + ": " + cAlphaFields(1) + " = " +
                                              simulation_control.name);
                        ErrorsFound = true;
                        SimObjectError = true;
                    }
                    for (k = 1; k <= m_state.dataLoopNodes->NumOfNodes; ++k) {
                        if (Node(k).IsLocalNode) {
                            ShowSevereError(m_state, format(RoutineName) + "Invalid " + cAlphaFields(3) + "=" + Alphas(3));
                            ShowContinueError(
                                m_state,
                                "A local air node is defined to INPUT the wind pressure coefficient curve, while Wind Pressure Coefficient "
                                "Type is set to SurfaceAverageCalculation.");
                            ShowContinueError(m_state, "It requires  the Wind Pressure Coefficient Type be set to INPUT to use the local air node.");
                            ErrorsFound = true;
                            SimObjectError = true;
                            break;
                        }
                    }
                } else {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(3) + " = " + simulation_control.WPCCntr +
                                        " is not valid.");
                    ShowContinueError(
                        m_state, "Valid choices are Input or SurfaceAverageCalculation. " + CurrentModuleObject + " = " + simulation_control.name);
                    ErrorsFound = true;
                    SimObjectError = true;
                }

                simulation_control.InitType = Alphas(6);
                if (Util::SameString(simulation_control.InitType, "LinearInitializationMethod")) {
                    simulation_control.InitFlag = 0;
                } else if (Util::SameString(simulation_control.InitType, "ZeroNodePressures")) {
                    simulation_control.InitFlag = 1;
                } else if (Util::SameString(simulation_control.InitType, "0")) {
                    simulation_control.InitFlag = 0;
                } else if (Util::SameString(simulation_control.InitType, "1")) {
                    simulation_control.InitFlag = 1;
                } else {
                    // Code will never be executed, validation will catch invalid input
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(6) + " = " + Alphas(6) + " is invalid.");
                    ShowContinueError(m_state,
                                      "Valid choices are LinearInitializationMethod or ZeroNodePressures. " + CurrentModuleObject + " = " +
                                          simulation_control.name);
                    ErrorsFound = true;
                    SimObjectError = true;
                }

                if (!lAlphaBlanks(7) && Util::SameString(Alphas(7), "Yes")) {
                    simulation_control.temperature_height_dependence = true;
                }

                if (lAlphaBlanks(8)) {
                    simulation_control.solver = SimulationControl::Solver::SkylineLU;
                } else if (Util::SameString(Alphas(8), "SkylineLU")) {
                    simulation_control.solver = SimulationControl::Solver::SkylineLU;
                } else if (Util::SameString(Alphas(8), "ConjugateGradient")) {
                    simulation_control.solver = SimulationControl::Solver::ConjugateGradient;
                } else {
                    simulation_control.solver = SimulationControl::Solver::SkylineLU;
                    ShowWarningError(m_state, format("{}{} object, ", RoutineName, CurrentModuleObject));
                    ShowContinueError(m_state, "..Specified " + cAlphaFields(8) + " = \"" + Alphas(8) + "\" is unrecognized.");
                    ShowContinueError(m_state, "..Default value \"SkylineLU\" will be used.");
                }

                // Get inputs for duct sizing
                simulation_control.autosize_ducts = false;
                if (NumAlphas == 10) {
                    if (Util::SameString(Alphas(10), "YES")) {
                        simulation_control.autosize_ducts = true;
                        if (simulation_control.type == ControlType::MultizoneWithDistribution) {
                            if (NumAPL > 1) {
                                ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, ");
                                ShowContinueError(
                                    m_state,
                                    format("The number of AirLoopHAVC is greater than 1. The current requirement for Duct Sizing requires a "
                                           "single AirLoopHVAC."));
                                ShowContinueError(m_state, format("..Duct sizing is not performed"));
                                simulation_control.autosize_ducts = false;
                            }
                        }
                    }
                }

                if (SimObjectError) {
                    ShowFatalError(
                        m_state,
                        format("{}Errors found getting {} object. Previous error(s) cause program termination.", RoutineName, CurrentModuleObject));
                }

                simulation_control.maximum_iterations = static_cast<int>(Numbers(1));
                simulation_control.relative_convergence_tolerance = Numbers(2);
                simulation_control.absolute_convergence_tolerance = Numbers(3);
                simulation_control.convergence_acceleration_limit = Numbers(4);
                simulation_control.azimuth = Numbers(5);
                simulation_control.aspect_ratio = Numbers(6);
                simulation_control.MaxPressure = 500.0; // Maximum pressure difference by default
            }
        }

        CurrentModuleObject = "AirflowNetwork:Distribution:DuctSizing";
        int NumDuctSizing = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject);
        if (NumDuctSizing > 1) {
            ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, ");
            ShowContinueError(
                m_state,
                format("The number of AirflowNetwork:Distribution:DuctSizing is greater than 1. The current requirement for Duct Sizing requires a "
                       "single object."));
            ShowContinueError(m_state, format("..Duct sizing is not performed"));
            simulation_control.autosize_ducts = false;
        } else if (simulation_control.autosize_ducts && NumDuctSizing == 0) {
            ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, ");
            ShowContinueError(
                m_state,
                format("The number of AirflowNetwork:Distribution:DuctSizing is not avalable. The current requirement for Duct Sizing requires a "
                       "single object."));
            ShowContinueError(m_state, format("..Duct sizing is not performed"));
            simulation_control.autosize_ducts = false;
        }
        if (simulation_control.autosize_ducts && NumDuctSizing == 1) {
            m_state.dataInputProcessing->inputProcessor->getObjectItem(m_state,
                                                                       CurrentModuleObject,
                                                                       NumDuctSizing,
                                                                       Alphas,
                                                                       NumAlphas,
                                                                       Numbers,
                                                                       NumNumbers,
                                                                       IOStatus,
                                                                       lNumericBlanks,
                                                                       lAlphaBlanks,
                                                                       cAlphaFields,
                                                                       cNumericFields);

            simulation_control.ductSizing.name = Alphas(1);
            if (Util::SameString(Alphas(2), Util::makeUPPER("MaximumVelocity"))) {
                simulation_control.ductSizing.method = DuctSizingMethod::MaxVelocity;
            } else if (Util::SameString(Alphas(2), Util::makeUPPER("PressureLoss"))) {
                simulation_control.ductSizing.method = DuctSizingMethod::PressureLoss;
            } else if (Util::SameString(Alphas(2), Util::makeUPPER("PressureLossWithMaximumVelocity"))) {
                simulation_control.ductSizing.method = DuctSizingMethod::VelocityAndLoss;
            } else {
                ShowSevereError(m_state, format("{} {} object, {} = {}  is invalid.", RoutineName, CurrentModuleObject, cAlphaFields(2), Alphas(2)));
                ShowContinueError(m_state,
                                  format("Valid choices are MaximumVelocity, PressureLoss, and PressureLossWithMaximumVelocity. {}: {} = {}",
                                         CurrentModuleObject,
                                         cAlphaFields(1),
                                         Alphas(1)));
                ErrorsFound = true;
            }
            if (simulation_control.type != ControlType::MultizoneWithDistribution) {
                ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, ");
                ShowContinueError(m_state,
                                  format("Although {} = \"{}\" is entered, but {} is not MultizoneWithoutDistribution.",
                                         cAlphaFields(10),
                                         Alphas(10),
                                         cAlphaFields(2)));
                ShowContinueError(m_state, format("..Duct sizing is not performed"));
                simulation_control.autosize_ducts = false;
            }
            simulation_control.ductSizing.factor = Numbers(1);
            simulation_control.ductSizing.max_velocity = Numbers(2);
            simulation_control.ductSizing.supply_trunk_pressure_loss = Numbers(3);
            simulation_control.ductSizing.supply_branch_pressure_loss = Numbers(4);
            simulation_control.ductSizing.return_trunk_pressure_loss = Numbers(5);
            simulation_control.ductSizing.return_branch_pressure_loss = Numbers(6);
        }

        // *** Read AirflowNetwork simulation zone data
        CurrentModuleObject = "AirflowNetwork:MultiZone:Zone";
        AirflowNetworkNumOfZones = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject);
        if (AirflowNetworkNumOfZones > 0) {
            MultizoneZoneData.allocate(AirflowNetworkNumOfZones);
            AirflowNetworkZoneFlag.dimension(m_state.dataGlobal->NumOfZones, false); // AirflowNetwork zone flag
            for (int i = 1; i <= AirflowNetworkNumOfZones; ++i) {
                m_state.dataInputProcessing->inputProcessor->getObjectItem(m_state,
                                                                           CurrentModuleObject,
                                                                           i,
                                                                           Alphas,
                                                                           NumAlphas,
                                                                           Numbers,
                                                                           NumNumbers,
                                                                           IOStatus,
                                                                           lNumericBlanks,
                                                                           lAlphaBlanks,
                                                                           cAlphaFields,
                                                                           cNumericFields);

                ErrorObjectHeader eoh{RoutineName, CurrentModuleObject, Alphas(1)};

                MultizoneZoneData(i).ZoneName = Alphas(1); // Name of Associated EnergyPlus Thermal Zone
                if (!lAlphaBlanks(2)) {
                    MultizoneZoneData(i).VentControl = Alphas(2); // Ventilation Control Mode: "Temperature", "Enthalpy",
                }
                // "ASHRAE55ADAPTIVE", "CEN15251AdaptiveComfort,
                // "Constant", or "NoVent"
                MultizoneZoneData(i).VentTempControlSchName = Alphas(3); // Name of ventilation temperature control schedule
                MultizoneZoneData(i).OpenFactor = Numbers(1);            // Limit Value on Multiplier for Modulating Venting Open Factor,
                // Not applicable if Vent Control Mode = CONSTANT or NOVENT
                MultizoneZoneData(i).LowValueTemp = Numbers(2); // Lower Value on Inside/Outside Temperature Difference
                // for Modulating the Venting Open Factor with temp control
                MultizoneZoneData(i).UpValueTemp = Numbers(3); // Upper Value on Inside/Outside Temperature Difference
                // for Modulating the Venting Open Factor with temp control
                MultizoneZoneData(i).LowValueEnth = Numbers(4); // Lower Value on Inside/Outside Temperature Difference
                // for Modulating the Venting Open Factor with Enthalpy control
                MultizoneZoneData(i).UpValueEnth = Numbers(5); // Upper Value on Inside/Outside Temperature Difference
                // for Modulating the Venting Open Factor with Enthalpy control
                MultizoneZoneData(i).VentCtrNum = VentControlType::None;
                MultizoneZoneData(i).SingleSidedCpType = Alphas(5);
                MultizoneZoneData(i).BuildWidth = Numbers(6);

                if (!lAlphaBlanks(6)) {
                    MultizoneZoneData(i).OccupantVentilationControlName = Alphas(6);
                    MultizoneZoneData(i).OccupantVentilationControlNum =
                        Util::FindItemInList(MultizoneZoneData(i).OccupantVentilationControlName, OccupantVentilationControl);
                    if (MultizoneZoneData(i).OccupantVentilationControlNum == 0) {
                        ShowSevereError(m_state,
                                        format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(6) +
                                            " not found = " + MultizoneZoneData(i).OccupantVentilationControlName);
                        ShowContinueError(m_state, "..for specified " + cAlphaFields(1) + " = " + Alphas(1));
                        ErrorsFound = true;
                    }
                }
                if (Util::SameString(MultizoneZoneData(i).VentControl, "Temperature")) {
                    MultizoneZoneData(i).VentCtrNum = VentControlType::Temp;
                }
                if (Util::SameString(MultizoneZoneData(i).VentControl, "Enthalpy")) {
                    MultizoneZoneData(i).VentCtrNum = VentControlType::Enth;
                }
                if (Util::SameString(MultizoneZoneData(i).VentControl, "Constant")) {
                    MultizoneZoneData(i).VentCtrNum = VentControlType::Const;
                }
                if (Util::SameString(MultizoneZoneData(i).VentControl, "ASHRAE55Adaptive")) {
                    MultizoneZoneData(i).VentCtrNum = VentControlType::ASH55;
                }
                if (Util::SameString(MultizoneZoneData(i).VentControl, "CEN15251Adaptive")) {
                    MultizoneZoneData(i).VentCtrNum = VentControlType::CEN15251;
                }
                if (Util::SameString(MultizoneZoneData(i).VentControl, "NoVent")) {
                    MultizoneZoneData(i).VentCtrNum = VentControlType::NoVent;
                }

                if (MultizoneZoneData(i).VentCtrNum < NumOfVentCtrTypes) {
                    if (NumAlphas >= 4 && (!lAlphaBlanks(4))) {
                        MultizoneZoneData(i).VentAvailSchName = Alphas(4);
                        if ((MultizoneZoneData(i).ventAvailSched = Sched::GetSchedule(m_state, MultizoneZoneData(i).VentAvailSchName)) == nullptr) {
                            ShowSevereItemNotFound(m_state, eoh, cAlphaFields(4), Alphas(4));
                            ErrorsFound = true;
                        }
                    }
                } else {
                    MultizoneZoneData(i).VentAvailSchName = std::string();
                    MultizoneZoneData(i).ventAvailSched = Sched::GetScheduleAlwaysOn(m_state);
                }
            }
        } else if (!simulation_control.DuctLoss) {
            ShowSevereError(m_state,
                            format(RoutineName) + "For an AirflowNetwork Simulation, at least one " + CurrentModuleObject +
                                " object is required but none were found.");
            ShowFatalError(
                m_state, format("{}Errors found getting {} object. Previous error(s) cause program termination.", RoutineName, CurrentModuleObject));
        }

        // ==> Zone data validation
        for (int i = 1; i <= AirflowNetworkNumOfZones; ++i) {
            // Zone name validation
            ErrorObjectHeader eoh{RoutineName, CurrentModuleObject, MultizoneZoneData(i).ZoneName};

            MultizoneZoneData(i).ZoneNum = Util::FindItemInList(MultizoneZoneData(i).ZoneName, Zone);
            if (MultizoneZoneData(i).ZoneNum == 0) {
                ShowSevereItemNotFound(m_state, eoh, "Zone Name", MultizoneZoneData(i).ZoneName);
                ErrorsFound = true;
            } else {
                AirflowNetworkZoneFlag(MultizoneZoneData(i).ZoneNum) = true;
                MultizoneZoneData(i).Height = Zone(MultizoneZoneData(i).ZoneNum).Centroid.z; // Nodal height
            }
            if (MultizoneZoneData(i).VentCtrNum == VentControlType::None) {
                ShowSevereError(m_state,
                                format(RoutineName) + CurrentModuleObject + " object, invalid " + cAlphaFields(2) + " = " +
                                    MultizoneZoneData(i).VentControl);
                ShowContinueError(m_state, "Valid choices are Temperature, Enthalpy, Constant, or NoVent");
                ShowContinueError(m_state, ".. in " + cAlphaFields(1) + " = \"" + MultizoneZoneData(i).ZoneName + "\"");
                ErrorsFound = true;
            }

            if (Util::SameString(MultizoneZoneData(i).VentControl, "Temperature") ||
                Util::SameString(MultizoneZoneData(i).VentControl, "Enthalpy")) { // Already converted this to an enum, why compare strings?
                // .or. &
                // Util::SameString(MultizoneZoneData(i)%VentControl,'ASHRAE55Adaptive') .or. &
                // Util::SameString(MultizoneZoneData(i)%VentControl,'CEN15251Adaptive')) then
                if (MultizoneZoneData(i).VentTempControlSchName.empty()) {
                    ShowSevereEmptyField(m_state, eoh, cAlphaFields(3), cAlphaFields(2), Alphas(2));
                    ErrorsFound = true;
                } else if ((MultizoneZoneData(i).ventTempControlSched = Sched::GetSchedule(m_state, MultizoneZoneData(i).VentTempControlSchName)) ==
                           nullptr) {
                    ShowSevereItemNotFound(m_state, eoh, cAlphaFields(3), Alphas(3));
                    ErrorsFound = true;
                }
            } else {
                MultizoneZoneData(i).ventTempControlSched = nullptr;
                if (!MultizoneZoneData(i).VentTempControlSchName.empty()) {
                    ShowWarningNonEmptyField(m_state, eoh, cAlphaFields(3), cAlphaFields(2), Alphas(2));
                    MultizoneZoneData(i).VentTempControlSchName = std::string();
                }
            }
            if (MultizoneZoneData(i).OpenFactor > 1.0 || MultizoneZoneData(i).OpenFactor < 0.0) {
                // Code will never be executed, validation will catch invalid input
                ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(1) + " is out of range [0.0,1.0]");
                ShowContinueError(m_state, format("..Input value = {:.2R}, Value will be set to 1.0", MultizoneZoneData(i).OpenFactor));
                MultizoneZoneData(i).OpenFactor = 1.0;
            }

            {
                // These SELECT_CASE_vars will go on input refactor, no need to fix
                auto const SELECT_CASE_var(Util::makeUPPER(MultizoneZoneData(i).VentControl));
                if (SELECT_CASE_var == "TEMPERATURE") { // checks on Temperature control
                    if (MultizoneZoneData(i).LowValueTemp < 0.0) {
                        // Code will never be executed, validation will catch invalid input
                        ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(2) + " < 0.0");
                        ShowContinueError(m_state, format("..Input value = {:.1R}, Value will be set to 0.0", MultizoneZoneData(i).LowValueTemp));
                        ShowContinueError(m_state, "..for " + cAlphaFields(1) + " = \"" + MultizoneZoneData(i).ZoneName);
                        MultizoneZoneData(i).LowValueTemp = 0.0;
                    }
                    if (MultizoneZoneData(i).LowValueTemp >= 100.0) {
                        // Code will never be executed, validation will catch invalid input
                        ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(2) + " >= 100.0");
                        ShowContinueError(m_state, format("..Input value = {:.1R}, Value will be reset to 0.0", MultizoneZoneData(i).LowValueTemp));
                        ShowContinueError(m_state, "..for " + cAlphaFields(1) + " = \"" + MultizoneZoneData(i).ZoneName);
                        MultizoneZoneData(i).LowValueTemp = 0.0;
                    }
                    if (MultizoneZoneData(i).UpValueTemp <= MultizoneZoneData(i).LowValueTemp) {
                        ShowWarningError(m_state,
                                         format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(3) + " <= " + cNumericFields(2));
                        ShowContinueError(m_state,
                                          format("..Input value for {} = {:.1R}, Value will be reset to 100.0",
                                                 cNumericFields(3),
                                                 MultizoneZoneData(i).UpValueTemp));
                        ShowContinueError(m_state, "..for " + cAlphaFields(1) + " = \"" + MultizoneZoneData(i).ZoneName);
                        MultizoneZoneData(i).UpValueTemp = 100.0;
                    }

                } else if (SELECT_CASE_var == "ENTHALPY") { // checks for Enthalpy control
                    if (MultizoneZoneData(i).LowValueEnth < 0.0) {
                        // Code will never be executed, validation will catch invalid input
                        ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(4) + " < 0.0");
                        ShowContinueError(m_state, format("..Input value = {:.1R}, Value will be reset to 0.0", MultizoneZoneData(i).LowValueEnth));
                        ShowContinueError(m_state, "..for " + cAlphaFields(1) + " = \"" + MultizoneZoneData(i).ZoneName);
                        MultizoneZoneData(i).LowValueEnth = 0.0;
                    }
                    if (MultizoneZoneData(i).LowValueEnth >= 300000.0) {
                        // Code will never be executed, validation will catch invalid input
                        ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(4) + " >= 300000.0");
                        ShowContinueError(m_state, format("..Input value = {:.1R}, Value will be reset to 0.0.", MultizoneZoneData(i).LowValueEnth));
                        ShowContinueError(m_state, "..for " + cAlphaFields(1) + " = \"" + MultizoneZoneData(i).ZoneName);
                        MultizoneZoneData(i).LowValueEnth = 0.0;
                    }
                    if (MultizoneZoneData(i).UpValueEnth <= MultizoneZoneData(i).LowValueEnth) {
                        ShowWarningError(m_state,
                                         format("{}{} object, {} <= {}", RoutineName, CurrentModuleObject, cNumericFields(5), cNumericFields(4)));
                        ShowContinueError(m_state,
                                          format("..Input value for {}= {:.1R}, Value will be reset to 300000.0",
                                                 cNumericFields(5),
                                                 MultizoneZoneData(i).UpValueEnth));
                        ShowContinueError(m_state, "..for " + cAlphaFields(1) + " = \"" + MultizoneZoneData(i).ZoneName);
                        MultizoneZoneData(i).UpValueEnth = 300000.0;
                    }
                } else if (SELECT_CASE_var == "ASHRAE55ADAPTIVE") {
                    // Check that for the given zone, there is a people object for which ASHRAE 55 calculations are carried out
                    MultizoneZoneData(i).ASH55PeopleInd = Solver::get_people_index(MultizoneZoneData(i).ZoneNum, VentControlType::ASH55, ErrorsFound);
                } else if (SELECT_CASE_var == "CEN15251ADAPTIVE") {
                    // Check that for the given zone, there is a people object for which CEN-15251 calculations are carried out
                    MultizoneZoneData(i).CEN15251PeopleInd =
                        Solver::get_people_index(MultizoneZoneData(i).ZoneNum, VentControlType::CEN15251, ErrorsFound);
                } else {
                }
            }
        }

        // *** Read AirflowNetwork external node
        if (simulation_control.iWPCCnt == iWPCCntr::Input) {
            // Wind coefficient == Surface-Average does not need inputs of external nodes
            AirflowNetworkNumOfExtNode =
                m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "AirflowNetwork:MultiZone:ExternalNode");
            if (m_state.dataGlobal->AnyLocalEnvironmentsInModel) {
                AirflowNetworkNumOfOutAirNode = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, "OutdoorAir:Node");
                AirflowNetworkNumOfExtNode += AirflowNetworkNumOfOutAirNode;
            }

            if (AirflowNetworkNumOfExtNode > 0) {
                MultizoneExternalNodeData.allocate(AirflowNetworkNumOfExtNode);
                CurrentModuleObject = "AirflowNetwork:MultiZone:ExternalNode";
                for (int i = 1; i <= AirflowNetworkNumOfExtNode - AirflowNetworkNumOfOutAirNode; ++i) {
                    m_state.dataInputProcessing->inputProcessor->getObjectItem(m_state,
                                                                               CurrentModuleObject,
                                                                               i,
                                                                               Alphas,
                                                                               NumAlphas,
                                                                               Numbers,
                                                                               NumNumbers,
                                                                               IOStatus,
                                                                               lNumericBlanks,
                                                                               lAlphaBlanks,
                                                                               cAlphaFields,
                                                                               cNumericFields);
                    MultizoneExternalNodeData(i).Name = Alphas(1);    // Name of external node
                    MultizoneExternalNodeData(i).height = Numbers(1); // Nodal height
                    if (Util::SameString(simulation_control.HeightOption, "ExternalNode") && lNumericBlanks(1)) {
                        ShowWarningError(m_state,
                                         format(RoutineName) + CurrentModuleObject + " object =" + Alphas(1) + ". The input of " + cNumericFields(1) +
                                             " is required, but a blank is found.");
                        ShowContinueError(m_state, format("The default value is assigned as {:.1R}", Numbers(1)));
                    }
                    MultizoneExternalNodeData(i).ExtNum = AirflowNetworkNumOfZones + i;            // External node number
                    MultizoneExternalNodeData(i).curve = Curve::GetCurveIndex(m_state, Alphas(2)); // Wind pressure curve
                    if (MultizoneExternalNodeData(i).curve == 0) {
                        ShowSevereError(m_state, format(RoutineName) + "Invalid " + cAlphaFields(2) + "=" + Alphas(2));
                        ShowContinueError(m_state, "Entered in " + CurrentModuleObject + '=' + Alphas(1));
                        ErrorsFound = true;
                    }
                    if (NumAlphas >= 3 && !lAlphaBlanks(3)) { // Symmetric curve
                        if (Util::SameString(Alphas(3), "Yes")) {
                            MultizoneExternalNodeData(i).symmetricCurve = true;
                        } else if (!Util::SameString(Alphas(3), "No")) {
                            ShowWarningError(
                                m_state, format(RoutineName) + CurrentModuleObject + " object, Invalid input " + cAlphaFields(3) + " = " + Alphas(3));
                            ShowContinueError(m_state, "The default value is assigned as No.");
                        }
                    }
                    if (NumAlphas == 4 && !lAlphaBlanks(4)) { // Relative or absolute wind angle
                        if (Util::SameString(Alphas(4), "Relative")) {
                            MultizoneExternalNodeData(i).useRelativeAngle = true;
                        } else if (!Util::SameString(Alphas(4), "Absolute")) {
                            // Code will never be executed, validation will catch invalid input
                            ShowWarningError(
                                m_state, format(RoutineName) + CurrentModuleObject + " object, Invalid input " + cAlphaFields(4) + " = " + Alphas(4));
                            ShowContinueError(m_state, "The default value is assigned as Absolute.");
                        }
                    }
                }
                if (m_state.dataGlobal->AnyLocalEnvironmentsInModel) {

                    CurrentModuleObject = "OutdoorAir:Node";
                    for (int i = AirflowNetworkNumOfExtNode - AirflowNetworkNumOfOutAirNode + 1; i <= AirflowNetworkNumOfExtNode; ++i) {
                        m_state.dataInputProcessing->inputProcessor->getObjectItem(m_state,
                                                                                   CurrentModuleObject,
                                                                                   i - (AirflowNetworkNumOfExtNode - AirflowNetworkNumOfOutAirNode),
                                                                                   Alphas,
                                                                                   NumAlphas,
                                                                                   Numbers,
                                                                                   NumNumbers,
                                                                                   IOStatus,
                                                                                   lNumericBlanks,
                                                                                   lAlphaBlanks,
                                                                                   cAlphaFields,
                                                                                   cNumericFields);
                        // HACK: Need to verify name is unique between "OutdoorAir:Node" and "AirflowNetwork:MultiZone:ExternalNode"

                        if (NumAlphas > 5 && !lAlphaBlanks(6)) { // Wind pressure curve
                            MultizoneExternalNodeData(i).curve = GetCurveIndex(m_state, Alphas(6));
                            if (MultizoneExternalNodeData(i).curve == 0) {
                                ShowSevereError(m_state, format(RoutineName) + "Invalid " + cAlphaFields(6) + "=" + Alphas(6));
                                ShowContinueError(m_state, "Entered in " + CurrentModuleObject + '=' + Alphas(1));
                                ErrorsFound = true;
                            }
                        }

                        if (NumAlphas > 6 && !lAlphaBlanks(7)) { // Symmetric curve
                            if (Util::SameString(Alphas(7), "Yes")) {
                                MultizoneExternalNodeData(i).symmetricCurve = true;
                            } else if (!Util::SameString(Alphas(7), "No")) {
                                ShowWarningError(m_state,
                                                 format(RoutineName) + CurrentModuleObject + " object, Invalid input " + cAlphaFields(7) + " = " +
                                                     Alphas(7));
                                ShowContinueError(m_state, "The default value is assigned as No.");
                            }
                        }

                        if (NumAlphas > 7 && !lAlphaBlanks(8)) { // Relative or absolute wind angle
                            if (Util::SameString(Alphas(8), "Relative")) {
                                MultizoneExternalNodeData(i).useRelativeAngle = true;
                            } else if (!Util::SameString(Alphas(8), "Absolute")) {
                                ShowWarningError(m_state,
                                                 format(RoutineName) + CurrentModuleObject + " object, Invalid input " + cAlphaFields(8) + " = " +
                                                     Alphas(8));
                                ShowContinueError(m_state, "The default value is assigned as Absolute.");
                            }
                        }

                        MultizoneExternalNodeData(i).Name = Alphas(1); // Name of external node
                        int NodeNum = GetOnlySingleNode(m_state,
                                                        Alphas(1),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::OutdoorAirNode,
                                                        "AirflowNetwork:Multizone:Surface",
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::Inlet,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        ObjectIsParent);
                        MultizoneExternalNodeData(i).OutAirNodeNum = NodeNum;               // Name of outdoor air node
                        MultizoneExternalNodeData(i).height = Node(NodeNum).Height;         // Nodal height
                        MultizoneExternalNodeData(i).ExtNum = AirflowNetworkNumOfZones + i; // External node number
                    }
                }
            } else {
                ShowSevereError(m_state,
                                format(RoutineName) + "An " + CurrentModuleObject +
                                    " object is required but not found when Wind Pressure Coefficient Type = Input.");
                ErrorsFound = true;
            }
        }

        // *** Read AirflowNetwork element data
        ErrorsFound = ErrorsFound || !get_element_input();

        // *** Read AirflowNetwork simulation surface data
        CurrentModuleObject = "AirflowNetwork:MultiZone:Surface";
        AirflowNetworkNumOfSurfaces = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject);
        if (AirflowNetworkNumOfSurfaces > 0) {
            MultizoneSurfaceData.allocate(AirflowNetworkNumOfSurfaces);
            for (int i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
                m_state.dataInputProcessing->inputProcessor->getObjectItem(m_state,
                                                                           CurrentModuleObject,
                                                                           i,
                                                                           Alphas,
                                                                           NumAlphas,
                                                                           Numbers,
                                                                           NumNumbers,
                                                                           IOStatus,
                                                                           lNumericBlanks,
                                                                           lAlphaBlanks,
                                                                           cAlphaFields,
                                                                           cNumericFields);
                MultizoneSurfaceData(i).SurfName = Alphas(1);    // Name of Associated EnergyPlus surface
                MultizoneSurfaceData(i).OpeningName = Alphas(2); // Name of crack or opening component,
                // either simple or detailed large opening, or crack
                MultizoneSurfaceData(i).ExternalNodeName = Alphas(3); // Name of external node, but not used at WPC="INPUT"
                if (Util::FindItemInList(Alphas(3), MultizoneExternalNodeData) &&
                    m_state.afn->MultizoneExternalNodeData(Util::FindItemInList(Alphas(3), MultizoneExternalNodeData)).curve == 0) {
                    ShowSevereError(m_state, format(RoutineName) + "Invalid " + cAlphaFields(3) + "=" + Alphas(3));
                    ShowContinueError(m_state,
                                      "A valid wind pressure coefficient curve name is required but not found when Wind Pressure "
                                      "Coefficient Type = Input.");
                    ErrorsFound = true;
                }
                MultizoneSurfaceData(i).Factor = Numbers(1); // Crack Actual Value or Window Open Factor for Ventilation
                if (MultizoneSurfaceData(i).Factor > 1.0 || MultizoneSurfaceData(i).Factor <= 0.0) {
                    ShowWarningError(m_state,
                                     format(RoutineName) + CurrentModuleObject + " object=" + MultizoneSurfaceData(i).SurfName + ", " +
                                         cNumericFields(1) + " is out of range (0.0,1.0]");
                    ShowContinueError(m_state, format("..Input value = {:.2R}, Value will be set to 1.0", MultizoneSurfaceData(i).Factor));
                    MultizoneSurfaceData(i).Factor = 1.0;
                }
                // Get input of ventilation control and associated data
                if (NumAlphas >= 4) {
                    // Ventilation Control Mode: "TEMPERATURE", "ENTHALPY",
                    //   "CONSTANT", "ZONELEVEL", "NOVENT", "ADJACENTTEMPERATURE",
                    //   or "ADJACENTENTHALPY"
                    if (!lAlphaBlanks(4)) {
                        MultizoneSurfaceData(i).VentControl = Alphas(4);
                    }
                    // Name of ventilation temperature control schedule
                    if (!lAlphaBlanks(5)) {
                        MultizoneSurfaceData(i).VentTempControlSchName = Alphas(5);
                    }
                    {
                        // This SELECT_CASE_var will go on input refactor, no need to fix
                        auto const SELECT_CASE_var(Util::makeUPPER(MultizoneSurfaceData(i).VentControl));
                        if (SELECT_CASE_var == "TEMPERATURE") {
                            MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::Temp;
                            MultizoneSurfaceData(i).IndVentControl = true;
                        } else if (SELECT_CASE_var == "ENTHALPY") {
                            MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::Enth;
                            MultizoneSurfaceData(i).IndVentControl = true;
                        } else if (SELECT_CASE_var == "CONSTANT") {
                            MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::Const;
                            MultizoneSurfaceData(i).IndVentControl = true;
                        } else if (SELECT_CASE_var == "ASHRAE55ADAPTIVE") {
                            MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::ASH55;
                            MultizoneSurfaceData(i).IndVentControl = true;
                        } else if (SELECT_CASE_var == "CEN15251ADAPTIVE") {
                            MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::CEN15251;
                            MultizoneSurfaceData(i).IndVentControl = true;
                        } else if (SELECT_CASE_var == "NOVENT") {
                            MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::NoVent;
                            MultizoneSurfaceData(i).IndVentControl = true;
                        } else if (SELECT_CASE_var == "ZONELEVEL") {
                            MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::ZoneLevel;
                            MultizoneSurfaceData(i).IndVentControl = false;
                        } else if (SELECT_CASE_var == "ADJACENTTEMPERATURE") {
                            MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::AdjTemp;
                            MultizoneSurfaceData(i).IndVentControl = true;
                        } else if (SELECT_CASE_var == "ADJACENTENTHALPY") {
                            MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::AdjEnth;
                            MultizoneSurfaceData(i).IndVentControl = true;
                        } else {
                            ShowSevereError(m_state, format(RoutineName) + CurrentModuleObject + " object, Invalid " + cAlphaFields(4));
                            ShowContinueError(m_state,
                                              ".." + cAlphaFields(1) + " = " + MultizoneSurfaceData(i).SurfName + ", Specified " + cAlphaFields(4) +
                                                  " = " + Alphas(4));
                            ShowContinueError(m_state,
                                              "..The valid choices are \"Temperature\", \"Enthalpy\", \"Constant\", \"NoVent\", \"ZoneLevel\", "
                                              "\"AdjancentTemperature\" or \"AdjacentEnthalpy\"");
                            ErrorsFound = true;
                        }
                    }
                }
                MultizoneSurfaceData(i).ModulateFactor = Numbers(2); // Limit Value on Multiplier for Modulating Venting Open Factor
                MultizoneSurfaceData(i).LowValueTemp = Numbers(3);   // Lower temperature value for modulation of temperature control
                MultizoneSurfaceData(i).UpValueTemp = Numbers(4);    // Upper temperature value for modulation of temperature control
                MultizoneSurfaceData(i).LowValueEnth = Numbers(5);   // Lower Enthalpy value for modulation of Enthalpy control
                MultizoneSurfaceData(i).UpValueEnth = Numbers(6);    // Lower Enthalpy value for modulation of Enthalpy control
                if (MultizoneSurfaceData(i).VentSurfCtrNum < 4 || MultizoneSurfaceData(i).VentSurfCtrNum == VentControlType::AdjTemp ||
                    MultizoneSurfaceData(i).VentSurfCtrNum == VentControlType::AdjEnth) {
                    if (!lAlphaBlanks(6)) {
                        MultizoneSurfaceData(i).VentAvailSchName = Alphas(6); // Name of ventilation availability schedule
                    }
                }
                if (!lAlphaBlanks(7)) {
                    MultizoneSurfaceData(i).OccupantVentilationControlName = Alphas(7);
                    MultizoneSurfaceData(i).OccupantVentilationControlNum =
                        Util::FindItemInList(MultizoneSurfaceData(i).OccupantVentilationControlName, OccupantVentilationControl);
                    if (MultizoneSurfaceData(i).OccupantVentilationControlNum == 0) {
                        ShowSevereError(m_state,
                                        format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(7) +
                                            " not found = " + MultizoneSurfaceData(i).OccupantVentilationControlName);
                        ShowContinueError(m_state, "..for specified " + cAlphaFields(1) + " = " + Alphas(1));
                        ErrorsFound = true;
                    }
                }
                // Get data of polygonal surface
                if (!lAlphaBlanks(8)) {
                    if (Alphas(8) == "POLYGONHEIGHT") {
                        MultizoneSurfaceData(i).EquivRecMethod = EquivRec::Height;
                    } else if (Alphas(8) == "BASESURFACEASPECTRATIO") {
                        MultizoneSurfaceData(i).EquivRecMethod = EquivRec::BaseAspectRatio;
                    } else if (Alphas(8) == "USERDEFINEDASPECTRATIO") {
                        MultizoneSurfaceData(i).EquivRecMethod = EquivRec::UserAspectRatio;
                    } else {
                        ShowSevereError(m_state, format(RoutineName) + CurrentModuleObject + " object, Invalid " + cAlphaFields(8));
                        ShowContinueError(m_state,
                                          ".." + cAlphaFields(1) + " = " + MultizoneSurfaceData(i).SurfName + ", Specified " + cAlphaFields(8) +
                                              " = " + Alphas(8));
                        ShowContinueError(m_state,
                                          "..The valid choices are \"PolygonHeight\", \"BaseSurfaceAspectRatio\", or \"UserDefinedAspectRatio\"");
                        ErrorsFound = true;
                    }
                } else {
                    MultizoneSurfaceData(i).EquivRecMethod = EquivRec::Height;
                }
                if (!lNumericBlanks(7)) {
                    MultizoneSurfaceData(i).EquivRecUserAspectRatio = Numbers(7);
                } else {
                    MultizoneSurfaceData(i).EquivRecUserAspectRatio = 1.0;
                }
            }
        } else if (!simulation_control.DuctLoss) {
            ShowSevereError(m_state, format(RoutineName) + "An " + CurrentModuleObject + " object is required but not found.");
            ErrorsFound = true;
        }

        // remove extra OutdoorAir:Node, not assigned to External Node Name
        if (m_state.dataGlobal->AnyLocalEnvironmentsInModel && AirflowNetworkNumOfOutAirNode > 0) {
            for (int i = AirflowNetworkNumOfExtNode - AirflowNetworkNumOfOutAirNode + 1; i <= AirflowNetworkNumOfExtNode; ++i) {
                found = false;
                for (int j = 1; j <= AirflowNetworkNumOfSurfaces; ++j) {
                    if (Util::SameString(MultizoneSurfaceData(j).ExternalNodeName, MultizoneExternalNodeData(i).Name)) {
                        found = true;
                    }
                }
                if (!found) {
                    if (i < AirflowNetworkNumOfExtNode) {
                        for (k = i; k <= AirflowNetworkNumOfExtNode - 1; ++k) {
                            MultizoneExternalNodeData(k).Name = MultizoneExternalNodeData(k + 1).Name;
                            MultizoneExternalNodeData(k).OutAirNodeNum = MultizoneExternalNodeData(k + 1).OutAirNodeNum;
                            MultizoneExternalNodeData(k).height = MultizoneExternalNodeData(k + 1).height;
                            MultizoneExternalNodeData(k).ExtNum = MultizoneExternalNodeData(k + 1).ExtNum - 1;
                        }
                        i -= 1;
                    }
                    AirflowNetworkNumOfOutAirNode -= 1;
                    AirflowNetworkNumOfExtNode -= 1;
                    MultizoneExternalNodeData.resize(AirflowNetworkNumOfExtNode);
                }
            }
        }

        // ==> Validate AirflowNetwork simulation surface data
        NumOfExtNodes = 0;
        for (int i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
            // Check a valid surface defined earlier
            MultizoneSurfaceData(i).SurfNum = Util::FindItemInList(MultizoneSurfaceData(i).SurfName, m_state.dataSurface->Surface);
            if (MultizoneSurfaceData(i).SurfNum == 0) {
                ShowSevereError(m_state,
                                format(RoutineName) + CurrentModuleObject + " object, Invalid " + cAlphaFields(1) +
                                    " given = " + MultizoneSurfaceData(i).SurfName);
                ShowFatalError(m_state, format("{}Errors found getting inputs. Previous error(s) cause program termination.", RoutineName));
            }
            if (!m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).HeatTransSurf &&
                !m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).IsAirBoundarySurf) {
                ShowSevereError(m_state, format(RoutineName) + CurrentModuleObject + " object");
                ShowContinueError(m_state,
                                  "..The surface specified must be a heat transfer surface. Invalid " + cAlphaFields(1) + " = " +
                                      MultizoneSurfaceData(i).SurfName);
                ErrorsFound = true;
                continue;
            }
            // Ensure an interior surface does not face itself
            if (m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCond >= 1) {
                // Check the surface is a subsurface or not
                if (m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).BaseSurf == MultizoneSurfaceData(i).SurfNum) {
                    if (MultizoneSurfaceData(i).SurfNum == m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCond) {
                        ShowSevereError(m_state, format(RoutineName) + CurrentModuleObject + " object");
                        ShowContinueError(m_state,
                                          "..The surface facing itself is not allowed. Invalid " + cAlphaFields(1) + " = " +
                                              MultizoneSurfaceData(i).SurfName);
                        ErrorsFound = true;
                    }
                } else {
                    if (m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).BaseSurf ==
                        m_state.dataSurface->Surface(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).BaseSurf).ExtBoundCond) {
                        ShowSevereError(m_state, format(RoutineName) + CurrentModuleObject + " object");
                        ShowContinueError(m_state,
                                          "..The base surface facing itself is not allowed. Invalid " + cAlphaFields(1) + " = " +
                                              MultizoneSurfaceData(i).SurfName);
                        ErrorsFound = true;
                    }
                }
            }
            // Ensure zones defined in inside and outside environment are used in the object of AIRFLOWNETWORK:MULTIZONE:ZONE
            found = false;
            n = m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Zone;
            int j;
            for (j = 1; j <= AirflowNetworkNumOfZones; ++j) {
                if (MultizoneZoneData(j).ZoneNum == n) {
                    found = true;
                    break;
                }
            }
            // find a surface geometry
            MultizoneSurfaceData(i).Height = m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Height;
            MultizoneSurfaceData(i).Width = m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Width;
            MultizoneSurfaceData(i).CHeight = m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Centroid.z;
            if (found) {
                MultizoneSurfaceData(i).NodeNums[0] = j;
            } else {
                ShowSevereError(m_state,
                                format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(1) + " = " + MultizoneSurfaceData(i).SurfName);
                ShowContinueError(m_state,
                                  "..Zone for inside surface must be defined in a AirflowNetwork:MultiZone:Zone object.  Could not find Zone = " +
                                      Zone(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Zone).Name);
                ShowFatalError(m_state, format("{}Errors found getting inputs. Previous error(s) cause program termination.", RoutineName));
            }

            // Calculate equivalent width and height
            if (m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Sides != 4) {
                MultizoneSurfaceData(i).NonRectangular = true;
                if (MultizoneSurfaceData(i).EquivRecMethod == EquivRec::Height) {
                    if (m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Tilt < 1.0 ||
                        m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Tilt > 179.0) { // horizontal surface
                        // check base surface shape
                        if (m_state.dataSurface->Surface(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).BaseSurf).Sides == 4) {
                            baseratio = m_state.dataSurface->Surface(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).BaseSurf).Width /
                                        m_state.dataSurface->Surface(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).BaseSurf).Height;
                            MultizoneSurfaceData(i).Width = sqrt(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Area * baseratio);
                            MultizoneSurfaceData(i).Height =
                                m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Area / MultizoneSurfaceData(i).Width;
                            if (m_state.dataGlobal->DisplayExtraWarnings) {
                                ShowWarningError(m_state,
                                                 format(RoutineName) + CurrentModuleObject + " object = " + MultizoneSurfaceData(i).SurfName);
                                ShowContinueError(m_state,
                                                  "The entered choice of Equivalent Rectangle Method is PolygonHeight. This choice is not valid for "
                                                  "a horizontal surface.");
                                ShowContinueError(m_state, "The BaseSurfaceAspectRatio choice is used. Simulation continues.");
                            }
                        } else {
                            MultizoneSurfaceData(i).Width = sqrt(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Area *
                                                                 MultizoneSurfaceData(i).EquivRecUserAspectRatio);
                            MultizoneSurfaceData(i).Height =
                                m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Area / MultizoneSurfaceData(i).Width;
                            // add warning
                            if (m_state.dataGlobal->DisplayExtraWarnings) {
                                ShowWarningError(m_state,
                                                 format(RoutineName) + CurrentModuleObject + " object = " + MultizoneSurfaceData(i).SurfName);
                                ShowContinueError(m_state,
                                                  "The entered choice of Equivalent Rectangle Method is PolygonHeight. This choice is not valid for "
                                                  "a horizontal surface with a polygonal base surface.");
                                ShowContinueError(m_state, "The default aspect ratio at 1 is used. Simulation continues.");
                            }
                        }
                    } else {
                        minHeight = min(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(1).z,
                                        m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(2).z);
                        maxHeight = max(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(1).z,
                                        m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(2).z);
                        for (j = 3; j <= m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Sides; ++j) {
                            minHeight = min(minHeight,
                                            min(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(j - 1).z,
                                                m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(j).z));
                            maxHeight = max(maxHeight,
                                            max(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(j - 1).z,
                                                m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(j).z));
                        }
                        if (maxHeight > minHeight) {
                            MultizoneSurfaceData(i).Height = maxHeight - minHeight;
                            MultizoneSurfaceData(i).Width =
                                m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Area / (maxHeight - minHeight);
                        }
                    }
                }
                if (MultizoneSurfaceData(i).EquivRecMethod == EquivRec::BaseAspectRatio) {
                    if (m_state.dataSurface->Surface(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).BaseSurf).Sides == 4) {
                        baseratio = m_state.dataSurface->Surface(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).BaseSurf).Width /
                                    m_state.dataSurface->Surface(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).BaseSurf).Height;
                        MultizoneSurfaceData(i).Width = sqrt(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Area * baseratio);
                        MultizoneSurfaceData(i).Height =
                            m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Area / MultizoneSurfaceData(i).Width;
                    } else {
                        minHeight = min(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(1).z,
                                        m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(2).z);
                        maxHeight = max(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(1).z,
                                        m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(2).z);
                        for (j = 3; j <= m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Sides; ++j) {
                            minHeight = min(minHeight,
                                            min(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(j - 1).z,
                                                m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(j).z));
                            maxHeight = max(maxHeight,
                                            max(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(j - 1).z,
                                                m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Vertex(j).z));
                        }
                        if (maxHeight > minHeight) {
                            MultizoneSurfaceData(i).Height = maxHeight - minHeight;
                            MultizoneSurfaceData(i).Width =
                                m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Area / (maxHeight - minHeight);
                            // add warning
                            if (m_state.dataGlobal->DisplayExtraWarnings) {
                                ShowWarningError(m_state,
                                                 format(RoutineName) + CurrentModuleObject + " object = " + MultizoneSurfaceData(i).SurfName);
                                ShowContinueError(m_state,
                                                  "The entered choice of Equivalent Rectangle Method is BaseSurfaceAspectRatio. This choice is not "
                                                  "valid for a polygonal base surface.");
                                ShowContinueError(m_state, "The PolygonHeight choice is used. Simulation continues.");
                            }
                        } else {
                            MultizoneSurfaceData(i).Width = sqrt(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Area *
                                                                 MultizoneSurfaceData(i).EquivRecUserAspectRatio);
                            MultizoneSurfaceData(i).Height =
                                m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Area / MultizoneSurfaceData(i).Width;
                            // add warning
                            if (m_state.dataGlobal->DisplayExtraWarnings) {
                                ShowWarningError(m_state,
                                                 format(RoutineName) + CurrentModuleObject + " object = " + MultizoneSurfaceData(i).SurfName);
                                ShowContinueError(m_state,
                                                  "The entered choice of Equivalent Rectangle Method is BaseSurfaceAspectRatio. This choice is not "
                                                  "valid for a horizontal surface with a polygonal base surface.");
                                ShowContinueError(m_state, "The default aspect ratio at 1 is used. Simulation continues.");
                            }
                        }
                    }
                }
                if (MultizoneSurfaceData(i).EquivRecMethod == EquivRec::UserAspectRatio) {
                    MultizoneSurfaceData(i).Width =
                        sqrt(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Area * MultizoneSurfaceData(i).EquivRecUserAspectRatio);
                    MultizoneSurfaceData(i).Height =
                        m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Area / MultizoneSurfaceData(i).Width;
                }
            }

            // Get the number of external surfaces
            if (m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCond == ExternalEnvironment ||
                (m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCond == OtherSideCoefNoCalcExt &&
                 m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtWind)) {
                ++AirflowNetworkNumOfExtSurfaces;
            }

            // Outside face environment
            if (simulation_control.iWPCCnt == iWPCCntr::Input) {
                n = m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCond;
                if (n == ExternalEnvironment ||
                    (n == OtherSideCoefNoCalcExt && m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtWind)) {
                    ++NumOfExtNodes;
                    if (AirflowNetworkNumOfExtNode > 0) {
                        found = false;
                        for (j = 1; j <= AirflowNetworkNumOfExtNode; ++j) {
                            if (Util::SameString(MultizoneSurfaceData(i).ExternalNodeName, MultizoneExternalNodeData(j).Name)) {
                                MultizoneSurfaceData(i).NodeNums[1] = MultizoneExternalNodeData(j).ExtNum;
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            ShowSevereError(m_state,
                                            format(RoutineName) + CurrentModuleObject + ": Invalid " + cAlphaFields(3) + " = " +
                                                MultizoneSurfaceData(i).ExternalNodeName);
                            ShowContinueError(m_state, "A valid " + cAlphaFields(3) + " is required when Wind Pressure Coefficient Type = Input");
                            ErrorsFound = true;
                        }
                    } else {
                        //          MultizoneSurfaceData(i)%NodeNums[1] =
                        //          AirflowNetworkNumOfZones+NumOfExtNodes
                    }
                    continue;
                } else {
                    if (n < ExternalEnvironment &&
                        !(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCond == OtherSideCoefNoCalcExt &&
                          m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtWind)) {
                        ShowSevereError(m_state,
                                        format(RoutineName) + CurrentModuleObject + ": Invalid " + cAlphaFields(1) + " = " +
                                            MultizoneSurfaceData(i).SurfName);
                        ShowContinueError(m_state, "This type of surface (has ground, etc exposure) cannot be used in the AiflowNetwork model.");
                        ErrorsFound = true;
                    }
                }
                found = false;
                for (j = 1; j <= AirflowNetworkNumOfZones; ++j) {
                    if (MultizoneZoneData(j).ZoneNum == m_state.dataSurface->Surface(n).Zone) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    MultizoneSurfaceData(i).NodeNums[1] = j;
                } else {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(1) + " = " +
                                        MultizoneSurfaceData(i).SurfName);
                    ShowContinueError(
                        m_state,
                        "..Zone for outside surface must be defined in a AirflowNetwork:MultiZone:Zone object.  Could not find Zone = " +
                            Zone(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Zone).Name);
                    ErrorsFound = true;
                    continue;
                }
            }
            if (Util::SameString(simulation_control.WPCCntr, "SurfaceAverageCalculation")) {
                n = m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCond;
                if (n >= 1) { // exterior boundary condition is a surface
                    found = false;
                    for (j = 1; j <= AirflowNetworkNumOfZones; ++j) {
                        if (MultizoneZoneData(j).ZoneNum == m_state.dataSurface->Surface(n).Zone) {
                            found = true;
                            break;
                        }
                    }
                    if (found) {
                        MultizoneSurfaceData(i).NodeNums[1] = j;
                    } else {
                        ShowSevereError(m_state, format(RoutineName) + CurrentModuleObject + " = " + MultizoneSurfaceData(i).SurfName);
                        ShowContinueError(m_state,
                                          "An adjacent zone = " + Zone(m_state.dataSurface->Surface(n).Zone).Name +
                                              " is not described in AIRFLOWNETWORK:MULTIZONE:ZONE");
                        ErrorsFound = true;
                        continue;
                    }
                }
            }
            if (!(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCond == -2 &&
                  m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtWind)) {
                if (MultizoneSurfaceData(i).NodeNums[1] == 0 && m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCond < 0) {
                    ShowSevereError(m_state, format(RoutineName) + CurrentModuleObject + " = " + MultizoneSurfaceData(i).SurfName);
                    ShowContinueError(m_state,
                                      "Outside boundary condition and object are " +
                                          cExtBoundCondition(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCond) + " and " +
                                          m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCondName + ".");
                    ShowContinueError(m_state, "The outside boundary condition must be exposed to either the outside or an adjacent zone.");
                    ErrorsFound = true;
                    continue;
                }
            }
        }

        // write outputs in eio file
        found = true;
        for (int i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
            if (MultizoneSurfaceData(i).NonRectangular) {
                if (found) {
                    print(m_state.files.eio,
                          "! <AirflowNetwork Model:Equivalent Rectangle Surface>, Name, Equivalent Height {{m}}, Equivalent Width {{m}} "
                          "AirflowNetwork "
                          "Model:Equivalent Rectangle\n");
                    found = false;
                }
                print(m_state.files.eio,
                      "AirflowNetwork Model:Equivalent Rectangle Surface, {}, {:.2R},{:.2R}\n",
                      MultizoneSurfaceData(i).SurfName,
                      MultizoneSurfaceData(i).Height,
                      MultizoneSurfaceData(i).Width);
            }
        }

        // Validate adjacent temperature and Enthalpy control for an interior surface only
        for (int i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
            if (MultizoneSurfaceData(i).VentSurfCtrNum == VentControlType::AdjTemp) {
                if (!(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCond >= 1)) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(1) + " = " +
                                        MultizoneSurfaceData(i).SurfName);
                    ShowContinueError(m_state, "..AdjacentTemperature venting control must be defined for an interzone surface.");
                    ErrorsFound = true;
                }
            }
            if (MultizoneSurfaceData(i).VentSurfCtrNum == VentControlType::AdjEnth) {
                if (!(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCond >= 1)) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(1) + " = " +
                                        MultizoneSurfaceData(i).SurfName);
                    ShowContinueError(m_state, "..AdjacentEnthalpy venting control must be defined for an interzone surface.");
                    ErrorsFound = true;
                }
            }
        }

        // Ensure the number of external node = the number of external surface with HeightOption choice = OpeningHeight
        if (Util::SameString(simulation_control.HeightOption, "OpeningHeight") && simulation_control.iWPCCnt == iWPCCntr::Input) {
            if (AirflowNetworkNumOfExtSurfaces != AirflowNetworkNumOfExtNode) {
                ShowSevereError(m_state,
                                format(RoutineName) +
                                    "When the choice of Height Selection for Local Wind Speed Calculation is OpeningHeight, the number of external "
                                    "surfaces defined in " +
                                    CurrentModuleObject + " objects ");
                ShowContinueError(m_state, "has to be equal to the number of AirflowNetwork:MultiZone:ExternalNode objects.");
                ShowContinueError(m_state,
                                  format("The entered number of external nodes is {}. The entered number of external surfaces is {}.",
                                         AirflowNetworkNumOfExtNode,
                                         AirflowNetworkNumOfExtSurfaces));
                ErrorsFound = true;
            }
        }

        // Read AirflowNetwork simulation detailed openings
        // Moved into getAirflowElementInput

        // Validate opening component and assign opening dimension
        if (AirflowNetworkNumOfDetOpenings > 0) {
            for (int i = 1; i <= AirflowNetworkNumOfDetOpenings; ++i) {
                found = false;
                for (int j = 1; j <= AirflowNetworkNumOfSurfaces; ++j) {
                    if (MultizoneCompDetOpeningData(i).name == MultizoneSurfaceData(j).OpeningName) {
                        //           MultizoneCompDetOpeningData(i)%Width =
                        //           Surface(MultizoneSurfaceData(j)%SurfNum)%Width
                        //           MultizoneCompDetOpeningData(i)%Height =
                        //           Surface(MultizoneSurfaceData(j)%SurfNum)%Height
                        found = true;
                    }
                }
            }
        }

        // Read AirflowNetwork simulation simple openings
        // Moved into getAirflowElementInput

        // Read AirflowNetwork simulation horizontal openings
        // Moved into getAirflowElementInput

        // Check status of control level for each surface with an opening
        CurrentModuleObject = "AirflowNetwork:MultiZone:Surface";
        for (int i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
            if (MultizoneSurfaceData(i).SurfNum == 0) {
                continue;
            }
            bool has_Opening{false}; // Why use array constructor?

            ErrorObjectHeader eoh{RoutineName, CurrentModuleObject, MultizoneSurfaceData(i).SurfName};

            // This is terrible, should not do it this way
            auto afe = elements.find(MultizoneSurfaceData(i).OpeningName);
            if (afe != elements.end()) {
                auto type = afe->second->type();
                has_Opening = (type == ComponentType::DOP) || (type == ComponentType::SOP) || (type == ComponentType::HOP);
            }
            // Obtain schedule number and check surface shape
            if (has_Opening) {
                if (m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).Sides == 3) {
                    ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + "=\"" + MultizoneSurfaceData(i).SurfName + "\".");
                    ShowContinueError(m_state,
                                      "The opening is a Triangular subsurface. A rectangular subsurface will be used with equivalent "
                                      "width and height.");
                }
                // Venting controls are not allowed for an air boundary surface
                if ((m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).IsAirBoundarySurf) &&
                    (MultizoneSurfaceData(i).VentSurfCtrNum != VentControlType::Const)) {
                    ShowWarningError(m_state,
                                     format(RoutineName) + CurrentModuleObject + "=\"" + MultizoneSurfaceData(i).SurfName +
                                         "\" is an air boundary surface.");
                    ShowContinueError(m_state, "Ventilation Control Mode = " + Alphas(4) + " is not valid. Resetting to Constant.");
                    MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::Const;
                    MultizoneSurfaceData(i).IndVentControl = true;
                }

                if (MultizoneSurfaceData(i).VentAvailSchName.empty()) {
                    MultizoneSurfaceData(i).ventAvailSched = Sched::GetScheduleAlwaysOn(m_state);
                } else if ((MultizoneSurfaceData(i).ventAvailSched = Sched::GetSchedule(m_state, MultizoneSurfaceData(i).VentAvailSchName)) ==
                           nullptr) {
                    ShowSevereItemNotFound(m_state, eoh, "Venting Schedule", MultizoneSurfaceData(i).VentAvailSchName);
                    ErrorsFound = true;
                } else if (m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).IsAirBoundarySurf) {
                    ShowWarningNonEmptyField(m_state, eoh, "Venting Availbility Schedule");
                    ShowContinueError(m_state, "Venting is always available for air-boundary surfaces.");
                    MultizoneSurfaceData(i).ventAvailSched = Sched::GetScheduleAlwaysOn(m_state);
                    MultizoneSurfaceData(i).VentAvailSchName = "";
                }

                switch (MultizoneSurfaceData(i).VentSurfCtrNum) {
                case VentControlType::Temp:
                case VentControlType::AdjTemp: {
                    if (MultizoneSurfaceData(i).VentTempControlSchName.empty()) {
                        ShowSevereEmptyField(m_state, eoh, "Ventilation Schedule", "Ventinlation Control", "Temperature");
                        ErrorsFound = true;
                    } else if ((MultizoneSurfaceData(i).ventTempControlSched =
                                    Sched::GetSchedule(m_state, MultizoneSurfaceData(i).VentTempControlSchName)) == nullptr) {
                        ShowSevereItemNotFound(m_state, eoh, "Ventilation Schedule", MultizoneSurfaceData(i).VentTempControlSchName);
                        ErrorsFound = true;
                    }
                    if (MultizoneSurfaceData(i).LowValueTemp < 0.0) {
                        ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, Low Temperature difference value < 0.0d0");
                        ShowContinueError(m_state, format("..Input value={:.1R}, Value will be reset to 0.0.", MultizoneSurfaceData(i).LowValueTemp));
                        ShowContinueError(m_state, "..for Surface = \"" + MultizoneSurfaceData(i).SurfName + "\"");
                        MultizoneSurfaceData(i).LowValueTemp = 0.0;
                    }
                    if (MultizoneSurfaceData(i).LowValueTemp >= 100.0) {
                        ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, Low Temperature difference value >= 100.0d0");
                        ShowContinueError(m_state,
                                          format("..Input value = {:.1R}, Value will be reset to 0.0", MultizoneSurfaceData(i).LowValueTemp));
                        ShowContinueError(m_state, "..for Surface = \"" + MultizoneSurfaceData(i).SurfName + "\"");
                        MultizoneZoneData(i).LowValueTemp = 0.0;
                    }
                    if (MultizoneSurfaceData(i).UpValueTemp <= MultizoneSurfaceData(i).LowValueTemp) {
                        ShowWarningError(
                            m_state, format(RoutineName) + CurrentModuleObject + " object, Upper Temperature <= Lower Temperature difference value.");
                        ShowContinueError(m_state,
                                          format("..Input value = {:.1R}, Value will be reset to 100.0", MultizoneSurfaceData(i).UpValueTemp));
                        ShowContinueError(m_state, "..for Surface = \"" + MultizoneSurfaceData(i).SurfName + "\"");
                        MultizoneSurfaceData(i).UpValueTemp = 100.0;
                    }

                } break;

                case VentControlType::Enth:
                case VentControlType::AdjEnth: {
                    if (MultizoneSurfaceData(i).VentTempControlSchName.empty()) {
                        ShowSevereEmptyField(m_state, eoh, "Ventilation Schedule", "Ventilation Control", "Enthalpy");
                        ErrorsFound = true;
                    } else if ((MultizoneSurfaceData(i).ventTempControlSched =
                                    Sched::GetSchedule(m_state, MultizoneSurfaceData(i).VentTempControlSchName)) == nullptr) {
                        ShowSevereItemNotFound(m_state, eoh, "Ventilation Schedule", MultizoneSurfaceData(i).VentTempControlSchName);
                        ErrorsFound = true;
                    }
                    if (MultizoneSurfaceData(i).LowValueEnth < 0.0) {
                        ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, Low Enthalpy difference value < 0.0d0");
                        ShowContinueError(m_state,
                                          format("..Input value = {:.1R}, Value will be reset to 0.0", MultizoneSurfaceData(i).LowValueEnth));
                        ShowContinueError(m_state, "..for Surface = \"" + MultizoneSurfaceData(i).SurfName + "\"");
                        MultizoneSurfaceData(i).LowValueEnth = 0.0;
                    }
                    if (MultizoneSurfaceData(i).LowValueEnth >= 300000.0) {
                        ShowWarningError(m_state, format(RoutineName) + CurrentModuleObject + " object, Low Enthalpy difference value >= 300000.0");
                        ShowContinueError(m_state,
                                          format("..Input value = {:.1R}, Value will be reset to 0.0", MultizoneSurfaceData(i).LowValueEnth));
                        ShowContinueError(m_state, "..for Surface = \"" + MultizoneSurfaceData(i).SurfName + "\"");
                        MultizoneZoneData(i).LowValueEnth = 0.0;
                    }
                    if (MultizoneSurfaceData(i).UpValueEnth <= MultizoneSurfaceData(i).LowValueEnth) {
                        ShowWarningError(m_state,
                                         format(RoutineName) + CurrentModuleObject + " object, Upper Enthalpy <= Lower Enthalpy difference value.");
                        ShowContinueError(m_state,
                                          format("..Input value = {:.1R}, Value will be set to 300000.0", MultizoneSurfaceData(i).UpValueEnth));
                        ShowContinueError(m_state, "..for Surface = \"" + MultizoneSurfaceData(i).SurfName + "\"");
                        MultizoneSurfaceData(i).UpValueEnth = 300000.0;
                    }

                } break;

                case VentControlType::Const:
                case VentControlType::NoVent:
                case VentControlType::ZoneLevel:
                case VentControlType::ASH55:
                case VentControlType::CEN15251: {
                    MultizoneSurfaceData(i).ventTempControlSched = nullptr;
                    MultizoneSurfaceData(i).VentTempControlSchName = "";
                    if (MultizoneSurfaceData(i).VentSurfCtrNum == VentControlType::ASH55 ||
                        MultizoneSurfaceData(i).VentSurfCtrNum == VentControlType::CEN15251) {
                        int zoneNum = MultizoneSurfaceData(i).NodeNums[0];
                        if (zoneNum > 0) {
                            int mzPtr = 0;
                            for (int mzIndex = 1; mzIndex <= AirflowNetworkNumOfZones; ++mzIndex) {
                                if (zoneNum == MultizoneZoneData(mzIndex).ZoneNum) {
                                    mzPtr = mzIndex;
                                    break;
                                }
                            }
                            if (MultizoneSurfaceData(i).VentSurfCtrNum == VentControlType::ASH55) {
                                MultizoneZoneData(mzPtr).ASH55PeopleInd =
                                    Solver::get_people_index(MultizoneZoneData(mzPtr).ZoneNum, VentControlType::ASH55, ErrorsFound);
                            }
                            if (MultizoneSurfaceData(i).VentSurfCtrNum == VentControlType::CEN15251) {
                                MultizoneZoneData(mzPtr).CEN15251PeopleInd =
                                    Solver::get_people_index(MultizoneZoneData(mzPtr).ZoneNum, VentControlType::CEN15251, ErrorsFound);
                            }
                        } else {
                            ShowSevereError(m_state,
                                            format(RoutineName) + "No zone number was found for AFN Surface " + MultizoneSurfaceData(i).SurfName);
                            ShowContinueError(m_state,
                                              "Check the " + CurrentModuleObject + " and Surface objects to verify they are connected to a zone.");
                        }
                    }
                } break;

                default:
                    break;
                }
            }
        }

        // Validate opening component and assign opening dimension
        if (AirflowNetworkNumOfSimOpenings > 0) {
            for (int i = 1; i <= AirflowNetworkNumOfSimOpenings; ++i) {
                found = false;
                for (int j = 1; j <= AirflowNetworkNumOfSurfaces; ++j) {
                    if (MultizoneCompSimpleOpeningData(i).name == MultizoneSurfaceData(j).OpeningName) {
                        //           MultizoneCompSimpleOpeningData(i)%Width =
                        //           Surface(MultizoneSurfaceData(j)%SurfNum)%Width
                        //           MultizoneCompSimpleOpeningData(i)%Height =
                        //           Surface(MultizoneSurfaceData(j)%SurfNum)%Height
                        found = true;
                    }
                }
            }
        }

        // Calculate CP values
        if (Util::SameString(simulation_control.WPCCntr, "SurfaceAverageCalculation")) {
            calculate_Cps();
            // Ensure automatic generation is OK
            n = 0;
            for (int j = 1; j <= 5; ++j) {
                found = false;
                for (int i = 1; i <= AirflowNetworkNumOfExtNode; ++i) {
                    if (MultizoneExternalNodeData(i).facadeNum == j) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    ++n;
                }
                if (j == 5 && (!found)) {
                    found = true;
                    if (m_state.dataGlobal->DisplayExtraWarnings) {
                        ShowWarningError(m_state,
                                         format(RoutineName) +
                                             "SurfaceAverageCalculation is entered for field = Wind Pressure Coefficient Type, but no roof "
                                             "surface is defined using an AirflowNetwork:MultiZone:Surface object.");
                        ShowContinueError(m_state, "Reconsider if this is your modeling intent. Simulation continues.");
                    }
                }
            }
            if (n < 5 && m_state.dataGlobal->DisplayExtraWarnings) {
                ShowWarningError(m_state, format(RoutineName) + "SurfaceAverageCalculation is entered for field = Wind Pressure Coefficient Type.");
                ShowContinueError(m_state,
                                  "The AirflowNetwork model provides wind pressure coefficients for 4 vertical exterior orientations and "
                                  "1 horizontal roof.");
                ShowContinueError(m_state,
                                  format(" There are only {} exterior surface orientations defined in this input file using "
                                         "AirflowNetwork:MultiZone:Surface objects.",
                                         n));
                ShowContinueError(m_state, "Reconsider if this is your modeling intent. Simulation continues.");
            }
        }

        // Assign external node height
        if (Util::SameString(simulation_control.WPCCntr, "SurfaceAverageCalculation") ||
            Util::SameString(simulation_control.HeightOption, "OpeningHeight")) {
            for (int i = 1; i <= AirflowNetworkNumOfExtNode; ++i) {
                for (int j = 1; j <= AirflowNetworkNumOfSurfaces; ++j) {
                    if (m_state.dataSurface->Surface(MultizoneSurfaceData(j).SurfNum).ExtBoundCond == ExternalEnvironment ||
                        (m_state.dataSurface->Surface(MultizoneSurfaceData(j).SurfNum).ExtBoundCond == OtherSideCoefNoCalcExt &&
                         m_state.dataSurface->Surface(MultizoneSurfaceData(j).SurfNum).ExtWind)) {
                        if (Util::SameString(MultizoneSurfaceData(j).ExternalNodeName, MultizoneExternalNodeData(i).Name)) {
                            MultizoneExternalNodeData(i).height = m_state.dataSurface->Surface(MultizoneSurfaceData(j).SurfNum).Centroid.z;
                            break;
                        }
                    }
                }
            }
        }

        // Assign external node azimuth, should consider combining this with the above to avoid the repeated search
        for (int i = 1; i <= AirflowNetworkNumOfExtNode; ++i) {
            for (int j = 1; j <= AirflowNetworkNumOfSurfaces; ++j) {
                if (m_state.dataSurface->Surface(MultizoneSurfaceData(j).SurfNum).ExtBoundCond == ExternalEnvironment ||
                    (m_state.dataSurface->Surface(MultizoneSurfaceData(j).SurfNum).ExtBoundCond == OtherSideCoefNoCalcExt &&
                     m_state.dataSurface->Surface(MultizoneSurfaceData(j).SurfNum).ExtWind)) {
                    if (Util::SameString(MultizoneSurfaceData(j).ExternalNodeName, MultizoneExternalNodeData(i).Name)) {
                        MultizoneExternalNodeData(i).azimuth = m_state.dataSurface->Surface(MultizoneSurfaceData(j).SurfNum).Azimuth;
                        break;
                    }
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(m_state, format("{}Errors found getting inputs. Previous error(s) cause program termination.", RoutineName));
        }

        // Write wind pressure coefficients in the EIO file
        if (!simulation_control.DuctLoss) {
            print(m_state.files.eio, "! <AirflowNetwork Model:Wind Direction>, Wind Direction #1 to n (degree)\n");
            print(m_state.files.eio, "AirflowNetwork Model:Wind Direction, ");

            int numWinDirs = 11;
            Real64 angleDelta = 30.0;
            if (AirflowNetworkNumOfSingleSideZones > 0) {
                numWinDirs = 35;
                angleDelta = 10.0;
            }

            for (int i = 0; i < numWinDirs; ++i) {
                print(m_state.files.eio, "{:.1R},", i * angleDelta);
            }
            print(m_state.files.eio, "{:.1R}\n", numWinDirs * angleDelta);

            print(m_state.files.eio,
                  "! <AirflowNetwork Model:Wind Pressure Coefficients>, Name, Wind Pressure Coefficients #1 to n (dimensionless)\n");

            // The old version used to write info with single-sided natural ventilation specific labeling, this version no longer does that.
            std::set<int> curves;
            for (int i = 1; i <= AirflowNetworkNumOfExtNode; ++i) {
                curves.insert(MultizoneExternalNodeData(i).curve);
            }
            for (auto index : curves) {
                print(m_state.files.eio, "AirflowNetwork Model:Wind Pressure Coefficients, {}, ", Curve::GetCurveName(m_state, index));

                for (int j = 0; j < numWinDirs; ++j) {
                    print(m_state.files.eio, "{:.2R},", Curve::CurveValue(m_state, index, j * angleDelta));
                }
                print(m_state.files.eio, "{:.2R}\n", Curve::CurveValue(m_state, index, numWinDirs * angleDelta));
            }

            if (AirflowNetworkNumOfSingleSideZones > 0) {
                for (int i = 1; i <= AirflowNetworkNumOfZones; ++i) {
                    if (MultizoneZoneData(i).SingleSidedCpType == "ADVANCED") {
                        print(m_state.files.eio,
                              "AirflowNetwork: Advanced Single-Sided Model: Difference in Opening Wind Pressure Coefficients (DeltaCP), ");
                        print(m_state.files.eio, "{}, ", MultizoneZoneData(i).ZoneName);
                        for (unsigned j = 1; j <= EPDeltaCP(i).WindDir.size() - 1; ++j) {
                            print(m_state.files.eio, "{:.2R},", EPDeltaCP(i).WindDir(j));
                        }
                        print(m_state.files.eio, "{:.2R}\n", EPDeltaCP(i).WindDir(static_cast<int>(EPDeltaCP(i).WindDir.size())));
                    }
                }
            }

            // If no zone object, exit
            if (AirflowNetworkNumOfZones == 0) {
                ShowFatalError(m_state, format("{}Errors found getting inputs. Previous error(s) cause program termination.", RoutineName));
            }
            // If zone node number =0, exit.
            for (int j = 1; j <= AirflowNetworkNumOfSurfaces; ++j) {
                if (MultizoneSurfaceData(j).NodeNums[0] == 0 && ErrorsFound) {
                    ShowFatalError(m_state, format("{}Errors found getting inputs. Previous error(s) cause program termination.", RoutineName));
                }
                if (MultizoneSurfaceData(j).NodeNums[1] == 0 && ErrorsFound) {
                    ShowFatalError(m_state, format("{}Errors found getting inputs. Previous error(s) cause program termination.", RoutineName));
                }
            }
        }

        // Ensure at least two surfaces are exposed to a zone
        ZoneCheck.allocate(AirflowNetworkNumOfZones);
        ZoneBCCheck.allocate(AirflowNetworkNumOfZones);
        ZoneCheck = 0;
        ZoneBCCheck = 0;
        CurrentModuleObject = "AirflowNetwork:MultiZone:Surface";
        for (int j = 1; j <= AirflowNetworkNumOfSurfaces; ++j) {
            if (MultizoneSurfaceData(j).NodeNums[0] <= AirflowNetworkNumOfZones) {
                ++ZoneCheck(MultizoneSurfaceData(j).NodeNums[0]);
                ZoneBCCheck(MultizoneSurfaceData(j).NodeNums[0]) = MultizoneSurfaceData(j).NodeNums[1];
            }
            if (MultizoneSurfaceData(j).NodeNums[1] <= AirflowNetworkNumOfZones) {
                ++ZoneCheck(MultizoneSurfaceData(j).NodeNums[1]);
                ZoneBCCheck(MultizoneSurfaceData(j).NodeNums[1]) = MultizoneSurfaceData(j).NodeNums[0];
            }
        }
        for (int i = 1; i <= AirflowNetworkNumOfZones; ++i) {
            if (ZoneCheck(i) == 0) {
                ShowSevereError(m_state, format(RoutineName) + "AirflowNetwork:Multizone:Zone = " + MultizoneZoneData(i).ZoneName);
                ShowContinueError(m_state, " does not have any surfaces defined in " + CurrentModuleObject);
                ShowContinueError(m_state, "Each zone should have at least two surfaces defined in " + CurrentModuleObject);
                ErrorsFound = true;
            }
            if (ZoneCheck(i) == 1) {
                ShowSevereError(m_state, format(RoutineName) + "AirflowNetwork:Multizone:Zone = " + MultizoneZoneData(i).ZoneName);
                ShowContinueError(m_state, " has only one surface defined in " + CurrentModuleObject);
                ShowContinueError(m_state, " Each zone should have at least two surfaces defined in " + CurrentModuleObject);
                ErrorsFound = true;
            }
            if (ZoneCheck(i) > 1) {
                bool SurfaceFound = false;
                for (int j = 1; j <= AirflowNetworkNumOfSurfaces; ++j) {
                    if (MultizoneSurfaceData(j).NodeNums[0] == i) {
                        if (ZoneBCCheck(i) != MultizoneSurfaceData(j).NodeNums[1]) {
                            SurfaceFound = true;
                            break;
                        }
                    }
                    if (MultizoneSurfaceData(j).NodeNums[1] == i) {
                        if (ZoneBCCheck(i) != MultizoneSurfaceData(j).NodeNums[0]) {
                            SurfaceFound = true;
                            break;
                        }
                    }
                }
                if (!SurfaceFound) {
                    ShowWarningError(m_state, format(RoutineName) + "AirflowNetwork:Multizone:Zone = " + MultizoneZoneData(i).ZoneName);
                    ShowContinueError(m_state,
                                      "has more than one surface defined in " + CurrentModuleObject + ", but has the same boundary conditions");
                    ShowContinueError(m_state, "Please check inputs of " + CurrentModuleObject);
                }
            }
        }
        ZoneCheck.deallocate();
        ZoneBCCheck.deallocate();

        // Validate CP Value number
        if (simulation_control.iWPCCnt == iWPCCntr::Input) { // Surface-Average does not need inputs of external nodes
            // Ensure different curve is used to avoid a single side boundary condition
            found = false;
            bool differentAngle = false;
            for (int j = 2; j <= AirflowNetworkNumOfExtNode; ++j) {
                if (MultizoneExternalNodeData(j - 1).curve != MultizoneExternalNodeData(j).curve) {
                    found = true;
                    break;
                } else {
                    // If the curves are the same, then check to see if the azimuths are different
                    if (MultizoneExternalNodeData(j - 1).azimuth != MultizoneExternalNodeData(j).azimuth) {
                        differentAngle = MultizoneExternalNodeData(j - 1).symmetricCurve || MultizoneExternalNodeData(j).symmetricCurve;
                    }
                }
            }
            if (!found && !differentAngle) {
                ShowSevereError(m_state,
                                "The same Wind Pressure Coefficient Curve name is used in all AirflowNetwork:MultiZone:ExternalNode objects.");
                ShowContinueError(
                    m_state, "Please input at least two different Wind Pressure Coefficient Curve names to avoid single side boundary condition.");
                ErrorsFound = true;
            }
        }

        // Assign occupant ventilation control number from zone to surface
        for (int i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
            int j = MultizoneSurfaceData(i).SurfNum;
            auto const &surf = m_state.dataSurface->Surface(j);
            if (surf.OriginalClass == SurfaceClass::Window || surf.OriginalClass == SurfaceClass::Door ||
                surf.OriginalClass == SurfaceClass::GlassDoor) {
                for (n = 1; n <= AirflowNetworkNumOfZones; ++n) {
                    if (MultizoneZoneData(n).ZoneNum == m_state.dataSurface->Surface(j).Zone) {
                        if (MultizoneZoneData(n).OccupantVentilationControlNum > 0 && MultizoneSurfaceData(i).OccupantVentilationControlNum == 0) {
                            MultizoneSurfaceData(i).OccupantVentilationControlNum = MultizoneZoneData(n).OccupantVentilationControlNum;
                        }
                    }
                }
            }
        }

        // Read AirflowNetwork Intra zone node
        CurrentModuleObject = "AirflowNetwork:IntraZone:Node";
        IntraZoneNumOfNodes = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject);
        if (IntraZoneNumOfNodes > 0) {
            IntraZoneNodeData.allocate(IntraZoneNumOfNodes);
            for (int i = 1; i <= IntraZoneNumOfNodes; ++i) {
                m_state.dataInputProcessing->inputProcessor->getObjectItem(m_state,
                                                                           CurrentModuleObject,
                                                                           i,
                                                                           Alphas,
                                                                           NumAlphas,
                                                                           Numbers,
                                                                           NumNumbers,
                                                                           IOStatus,
                                                                           lNumericBlanks,
                                                                           lAlphaBlanks,
                                                                           cAlphaFields,
                                                                           cNumericFields);
                IntraZoneNodeData(i).Name = Alphas(1);         // Name of node
                IntraZoneNodeData(i).RAFNNodeName = Alphas(2); // Name of RoomAir node
                IntraZoneNodeData(i).Height = Numbers(1);      // Nodal height
                // verify RoomAir model node names(May be too early to check and move to another subroutine)
                bool Errorfound1;
                GetRAFNNodeNum(
                    m_state, IntraZoneNodeData(i).RAFNNodeName, IntraZoneNodeData(i).ZoneNum, IntraZoneNodeData(i).RAFNNodeNum, Errorfound1);
                if (Errorfound1) {
                    ErrorsFound = true;
                }
                if (IntraZoneNodeData(i).RAFNNodeNum == 0) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + "' invalid name " + cAlphaFields(2) + "='" +
                                        Alphas(2));
                    ErrorsFound = true;
                }
                IntraZoneNodeData(i).AFNZoneNum =
                    Util::FindItemInList(Alphas(3), MultizoneZoneData, &MultizoneZoneProp::ZoneName, AirflowNetworkNumOfZones);
                if (MultizoneZoneData(IntraZoneNodeData(i).AFNZoneNum).RAFNNodeNum == 0) {
                    GetRAFNNodeNum(m_state,
                                   MultizoneZoneData(IntraZoneNodeData(i).AFNZoneNum).ZoneName,
                                   IntraZoneNodeData(i).ZoneNum,
                                   MultizoneZoneData(IntraZoneNodeData(i).AFNZoneNum).RAFNNodeNum,
                                   Errorfound1);
                }
                if (IntraZoneNodeData(i).ZoneNum == 0) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + "' the Zone is not defined for " +
                                        cAlphaFields(3) + "='" + Alphas(3));
                    ErrorsFound = true;
                }
            }
        }

        // check model compatibility
        if (IntraZoneNumOfNodes > 0) {
            if (!Util::SameString(SimAirNetworkKey, "MultizoneWithoutDistribution")) {
                ShowSevereError(m_state,
                                format(RoutineName) + CurrentModuleObject +
                                    " model requires Simulation Control = MultizoneWithoutDistribution, while the input choice is " +
                                    SimAirNetworkKey + ".");
                ErrorsFound = true;
                ShowFatalError(
                    m_state,
                    format("{}Errors found getting {} object. Previous error(s) cause program termination.", RoutineName, CurrentModuleObject));
            }
        }

        NumOfNodesIntraZone = IntraZoneNumOfNodes;
        // check zone node
        IntraZoneNumOfZones = 0;
        for (int i = 1; i <= AirflowNetworkNumOfZones; ++i) {
            if (MultizoneZoneData(i).RAFNNodeNum > 0) {
                IntraZoneNumOfZones += 1;
            }
        }

        // Override error check due to RoomAirNode for the time being

        // Read AirflowNetwork Intra linkage
        CurrentModuleObject = "AirflowNetwork:IntraZone:Linkage";
        IntraZoneNumOfLinks = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject);
        if (IntraZoneNumOfLinks > 0) {
            IntraZoneLinkageData.allocate(IntraZoneNumOfLinks);
            UniqueAirflowNetworkSurfaceName.reserve(IntraZoneNumOfLinks);
            for (int i = 1; i <= IntraZoneNumOfLinks; ++i) {
                m_state.dataInputProcessing->inputProcessor->getObjectItem(m_state,
                                                                           CurrentModuleObject,
                                                                           i,
                                                                           Alphas,
                                                                           NumAlphas,
                                                                           Numbers,
                                                                           NumNumbers,
                                                                           IOStatus,
                                                                           lNumericBlanks,
                                                                           lAlphaBlanks,
                                                                           cAlphaFields,
                                                                           cNumericFields);
                IntraZoneLinkageData(i).Name = Alphas(1); // Name of linkage
                IntraZoneLinkageData(i).NodeNames[0] = Alphas(2);
                IntraZoneLinkageData(i).NodeHeights[0] = 0.0;
                IntraZoneLinkageData(i).NodeNames[1] = Alphas(3);
                IntraZoneLinkageData(i).NodeHeights[1] = 0.0;
                IntraZoneLinkageData(i).CompName = Alphas(4);
                if (!lAlphaBlanks(5)) {
                    // Perform simple test first.The comprehensive input validation will occur later
                    // Check valid surface name
                    IntraZoneLinkageData(i).SurfaceName = Alphas(5);
                    IntraZoneLinkageData(i).LinkNum =
                        Util::FindItemInList(Alphas(5), MultizoneSurfaceData, &MultizoneSurfaceProp::SurfName, AirflowNetworkNumOfSurfaces);
                    if (IntraZoneLinkageData(i).LinkNum == 0) {
                        ShowSevereError(m_state,
                                        format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + "': Invalid " + cAlphaFields(5) +
                                            " given = " + Alphas(5) + " in AirflowNetwork:MultiZone:Surface objects");
                        ErrorsFound = true;
                    }
                    GlobalNames::VerifyUniqueInterObjectName(
                        m_state, UniqueAirflowNetworkSurfaceName, Alphas(5), CurrentModuleObject, cAlphaFields(5), ErrorsFound);
                }
                if (Util::SameString(Alphas(2), Alphas(3))) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + "': Invalid inputs of both node name with " +
                                        Alphas(2) + " = " + Alphas(3));
                    ErrorsFound = true;
                }
                // Check valid node names
                IntraZoneLinkageData(i).NodeNums[0] = Util::FindItemInList(Alphas(2), IntraZoneNodeData, IntraZoneNumOfNodes);
                if (IntraZoneLinkageData(i).NodeNums[0] == 0) {
                    IntraZoneLinkageData(i).NodeNums[0] =
                        Util::FindItemInList(Alphas(2), MultizoneZoneData, &MultizoneZoneProp::ZoneName, AirflowNetworkNumOfZones);
                    IntraZoneLinkageData(i).NodeHeights[0] = Zone(MultizoneZoneData(IntraZoneLinkageData(i).NodeNums[0]).ZoneNum).Centroid.z;
                    if (IntraZoneLinkageData(i).NodeNums[0] == 0) {
                        ShowSevereError(m_state,
                                        format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + "': Invalid " + cAlphaFields(2) +
                                            " given = " + Alphas(2) + " in AirflowNetwork:IntraZone:Node and AirflowNetwork:MultiZone:Zone objects");
                        ErrorsFound = true;
                    }
                } else {
                    IntraZoneLinkageData(i).NodeHeights[0] = IntraZoneNodeData(IntraZoneLinkageData(i).NodeNums[0]).Height;
                    IntraZoneLinkageData(i).NodeNums[0] = IntraZoneLinkageData(i).NodeNums[0] + AirflowNetworkNumOfZones + AirflowNetworkNumOfExtNode;
                }
                IntraZoneLinkageData(i).NodeNums[1] = Util::FindItemInList(Alphas(3), IntraZoneNodeData, IntraZoneNumOfNodes);
                if (IntraZoneLinkageData(i).NodeNums[1] == 0) {
                    IntraZoneLinkageData(i).NodeNums[1] =
                        Util::FindItemInList(Alphas(3), MultizoneZoneData, &MultizoneZoneProp::ZoneName, AirflowNetworkNumOfZones);
                    if (IntraZoneLinkageData(i).NodeNums[1] > 0) {
                        IntraZoneLinkageData(i).NodeHeights[1] = Zone(MultizoneZoneData(IntraZoneLinkageData(i).NodeNums[1]).ZoneNum).Centroid.z;
                    } else {
                        if (simulation_control.iWPCCnt == iWPCCntr::Input) { // Surface-Average does not need inputs of external nodes
                            IntraZoneLinkageData(i).NodeNums[1] = MultizoneSurfaceData(IntraZoneLinkageData(i).LinkNum).NodeNums[1];
                            if (IntraZoneLinkageData(i).NodeNums[1] == 0) {
                                ShowSevereError(m_state,
                                                format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + "': Invalid " + cAlphaFields(3) +
                                                    " given = " + Alphas(3) +
                                                    " in AirflowNetwork:IntraZone:Node or AirflowNetwork:MultiZone:Zone or "
                                                    "AirflowNetwork:MultiZone:ExternalNode objects");
                                ErrorsFound = true;
                            }
                        }
                        if (simulation_control.iWPCCnt == iWPCCntr::SurfAvg) {
                            if (!lAlphaBlanks(3)) {
                                ShowWarningError(m_state,
                                                 format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + " The input of " + cAlphaFields(3) +
                                                     " is not needed, ");
                                ShowContinueError(m_state,
                                                  " since AirflowNetwork Wind Pressure Coefficient Type = SURFACE-AVERAGE CALCULATION. The "
                                                  "simulation continues...");
                            }
                            IntraZoneLinkageData(i).NodeNums[1] = MultizoneSurfaceData(IntraZoneLinkageData(i).LinkNum).NodeNums[1];
                        }
                    }
                } else {
                    IntraZoneLinkageData(i).NodeHeights[1] = IntraZoneNodeData(IntraZoneLinkageData(i).NodeNums[1]).Height;
                    IntraZoneLinkageData(i).NodeNums[1] = IntraZoneLinkageData(i).NodeNums[1] + AirflowNetworkNumOfZones + AirflowNetworkNumOfExtNode;
                }
                // Ensure the both linked nodes for a surface are not zone nodes.One of nodes has to be an intrazone node
                if (IntraZoneLinkageData(i).NodeNums[1] <= AirflowNetworkNumOfZones &&
                    IntraZoneLinkageData(i).NodeNums[0] <= AirflowNetworkNumOfZones) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + "': Invalid node inputs " + Alphas(2) + " and " +
                                        Alphas(3) + " are zone nodes");
                    ErrorsFound = true;
                }
                if (IntraZoneLinkageData(i).NodeNums[0] <= AirflowNetworkNumOfZones &&
                    IntraZoneLinkageData(i).NodeNums[1] > AirflowNetworkNumOfZones + AirflowNetworkNumOfExtNode && lAlphaBlanks(5)) {
                    if (IntraZoneLinkageData(i).NodeNums[0] !=
                        m_state.afn->IntraZoneNodeData(IntraZoneLinkageData(i).NodeNums[1] - AirflowNetworkNumOfZones - AirflowNetworkNumOfExtNode)
                            .AFNZoneNum) {
                        ShowSevereError(
                            m_state,
                            format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + ": Invalid zone inputs between Node and Link " +
                                Alphas(2) + " and " +
                                m_state.afn->MultizoneZoneData(IntraZoneNodeData(IntraZoneLinkageData(i).NodeNums[0]).AFNZoneNum).ZoneName);
                        ErrorsFound = true;
                    }
                }
                if (IntraZoneLinkageData(i).NodeNums[1] <= AirflowNetworkNumOfZones &&
                    IntraZoneLinkageData(i).NodeNums[0] > AirflowNetworkNumOfZones + AirflowNetworkNumOfExtNode && lAlphaBlanks(5)) {
                    if (IntraZoneLinkageData(i).NodeNums[1] !=
                        m_state.afn->IntraZoneNodeData(IntraZoneLinkageData(i).NodeNums[0] - AirflowNetworkNumOfZones - AirflowNetworkNumOfExtNode)
                            .AFNZoneNum) {
                        ShowSevereError(
                            m_state,
                            format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + ": Invalid zone inputs between Node and Link " +
                                Alphas(3) + " and " +
                                m_state.afn->MultizoneZoneData(IntraZoneNodeData(IntraZoneLinkageData(i).NodeNums[1]).AFNZoneNum).ZoneName);
                        ErrorsFound = true;
                    }
                }
            }

            // Reset the number of intrazone links for a given surface
            NumOfLinksIntraZone = IntraZoneNumOfLinks;
            for (int i = 1; i <= IntraZoneNumOfLinks; ++i) {
                int j = IntraZoneLinkageData(i).LinkNum;
                if (j > 0) {
                    // Revise data in multizone object
                    NumOfLinksIntraZone = NumOfLinksIntraZone - 1;
                    if (m_state.dataSurface->Surface(MultizoneSurfaceData(j).SurfNum).ExtBoundCond == 0) {
                        // Exterior surface NodeNums[1] should be equal
                        if (IntraZoneLinkageData(i).NodeNums[0] > AirflowNetworkNumOfZones + AirflowNetworkNumOfExtNode) {
                            MultizoneSurfaceData(j).RAFNflag = true;
                            MultizoneSurfaceData(j).ZonePtr = MultizoneSurfaceData(j).NodeNums[0];
                            MultizoneSurfaceData(j).NodeNums[0] = IntraZoneLinkageData(i).NodeNums[0];
                        } else if (IntraZoneLinkageData(i).NodeNums[1] > AirflowNetworkNumOfZones + AirflowNetworkNumOfExtNode) {
                            MultizoneSurfaceData(j).RAFNflag = true;
                            MultizoneSurfaceData(j).ZonePtr = MultizoneSurfaceData(j).NodeNums[0];
                            MultizoneSurfaceData(j).NodeNums[0] = IntraZoneLinkageData(i).NodeNums[1];
                        } else {
                            ShowSevereError(m_state,
                                            format(RoutineName) + "The InterZone link is not found between AirflowNetwork:IntraZone:Linkage =" +
                                                IntraZoneLinkageData(i).Name +
                                                " and AirflowNetwork:Multizone:Surface = " + MultizoneSurfaceData(j).SurfName);
                            ErrorsFound = true;
                        }
                    } else {
                        // Interior surface
                        if (IntraZoneLinkageData(i).NodeNums[0] > AirflowNetworkNumOfZones + AirflowNetworkNumOfExtNode &&
                            IntraZoneLinkageData(i).NodeNums[1] > AirflowNetworkNumOfZones + AirflowNetworkNumOfExtNode) {
                            MultizoneSurfaceData(j).RAFNflag = true;
                            if (MultizoneZoneData(MultizoneSurfaceData(j).NodeNums[0]).ZoneNum ==
                                m_state.afn
                                    ->IntraZoneNodeData(IntraZoneLinkageData(i).NodeNums[0] - AirflowNetworkNumOfZones - AirflowNetworkNumOfExtNode)
                                    .ZoneNum) {
                                MultizoneSurfaceData(j).ZonePtr = MultizoneSurfaceData(j).NodeNums[0];
                                MultizoneSurfaceData(j).NodeNums[0] = IntraZoneLinkageData(i).NodeNums[0];
                                MultizoneSurfaceData(j).NodeNums[1] = IntraZoneLinkageData(i).NodeNums[1];
                            } else {
                                MultizoneSurfaceData(j).ZonePtr = MultizoneSurfaceData(j).NodeNums[0];
                                MultizoneSurfaceData(j).NodeNums[0] = IntraZoneLinkageData(i).NodeNums[1];
                                MultizoneSurfaceData(j).NodeNums[1] = IntraZoneLinkageData(i).NodeNums[0];
                            }
                        } else if (IntraZoneLinkageData(i).NodeNums[0] > AirflowNetworkNumOfZones + AirflowNetworkNumOfExtNode) {
                            MultizoneSurfaceData(j).RAFNflag = true;
                            if (IntraZoneLinkageData(i).NodeNums[1] == MultizoneSurfaceData(j).NodeNums[0]) {
                                MultizoneSurfaceData(j).NodeNums[1] = IntraZoneLinkageData(i).NodeNums[0];
                            } else if (IntraZoneLinkageData(i).NodeNums[1] == MultizoneSurfaceData(j).NodeNums[1]) {
                                MultizoneSurfaceData(j).ZonePtr = MultizoneSurfaceData(j).NodeNums[0];
                                MultizoneSurfaceData(j).NodeNums[0] = IntraZoneLinkageData(i).NodeNums[0];
                            } else {
                                ShowSevereError(m_state,
                                                format(RoutineName) + "The InterZone link is not found between AirflowNetwork:IntraZone:Linkage =" +
                                                    IntraZoneLinkageData(i).Name +
                                                    " and AirflowNetwork:Multizone:Surface = " + MultizoneSurfaceData(j).SurfName);
                                ErrorsFound = true;
                            }
                        } else if (IntraZoneLinkageData(i).NodeNums[1] > AirflowNetworkNumOfZones + AirflowNetworkNumOfExtNode) {
                            MultizoneSurfaceData(j).RAFNflag = true;
                            if (IntraZoneLinkageData(i).NodeNums[0] == MultizoneSurfaceData(j).NodeNums[0]) {
                                MultizoneSurfaceData(j).NodeNums[1] = IntraZoneLinkageData(i).NodeNums[1];
                            } else if (IntraZoneLinkageData(i).NodeNums[0] == MultizoneSurfaceData(j).NodeNums[1]) {
                                MultizoneSurfaceData(j).ZonePtr = MultizoneSurfaceData(j).NodeNums[0];
                                MultizoneSurfaceData(j).NodeNums[0] = IntraZoneLinkageData(i).NodeNums[1];
                            } else {
                                ShowSevereError(m_state,
                                                format(RoutineName) + "The InterZone link is not found between AirflowNetwork:IntraZone:Linkage =" +
                                                    IntraZoneLinkageData(i).Name +
                                                    " and AirflowNetwork:Multizone:Surface = " + MultizoneSurfaceData(j).SurfName);
                                ErrorsFound = true;
                            }
                        }
                    }
                }
            }
            // Remove links with surface defined in Multizone : Surface objects
            if (NumOfLinksIntraZone < IntraZoneNumOfLinks) {
                int link = 1;
                while (link <= NumOfLinksIntraZone) {
                    if (IntraZoneLinkageData(link).LinkNum > 0) {
                        if (m_state.dataGlobal->DisplayExtraWarnings) {
                            ShowWarningError(m_state,
                                             format(RoutineName) + CurrentModuleObject + "='" + IntraZoneLinkageData(link).Name +
                                                 " is reomoved from the list due to the surface conncetion from Intrazone to Interzone.");
                        }
                        for (int j = link; j <= IntraZoneNumOfLinks - 1; ++j) {
                            IntraZoneLinkageData(j) = IntraZoneLinkageData(j + 1);
                        }
                    }
                    if (IntraZoneLinkageData(link).LinkNum == 0) {
                        link = link + 1;
                    }
                }
                if (IntraZoneLinkageData(link).LinkNum > 0) {
                    if (m_state.dataGlobal->DisplayExtraWarnings) {
                        ShowWarningError(m_state,
                                         format(RoutineName) + CurrentModuleObject + "='" + IntraZoneLinkageData(link).Name +
                                             " is removed from the list due to the surface connection from Intrazone to Interzone.");
                    }
                }
            }
        }

        // Read AirflowNetwork Distribution system node
        CurrentModuleObject = "AirflowNetwork:Distribution:Node";
        DisSysNumOfNodes = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject);
        if (DisSysNumOfNodes > 0) {
            DisSysNodeData.allocate(DisSysNumOfNodes);
            for (int i = 1; i <= DisSysNumOfNodes; ++i) {
                m_state.dataInputProcessing->inputProcessor->getObjectItem(m_state,
                                                                           CurrentModuleObject,
                                                                           i,
                                                                           Alphas,
                                                                           NumAlphas,
                                                                           Numbers,
                                                                           NumNumbers,
                                                                           IOStatus,
                                                                           lNumericBlanks,
                                                                           lAlphaBlanks,
                                                                           cAlphaFields,
                                                                           cNumericFields);
                DisSysNodeData(i).Name = Alphas(1);      // Name of node
                DisSysNodeData(i).EPlusName = Alphas(2); // Name of associated EnergyPlus node
                DisSysNodeData(i).EPlusType = Alphas(3); // Name of associated EnergyPlus type
                DisSysNodeData(i).Height = Numbers(1);   // Nodal height
                DisSysNodeData(i).EPlusNodeNum = 0;      // EPlus node number
                // verify EnergyPlus object type
                if (Util::SameString(Alphas(3), "AirLoopHVAC:ZoneMixer") || Util::SameString(Alphas(3), "AirLoopHVAC:ZoneSplitter") ||
                    Util::SameString(Alphas(3), "AirLoopHVAC:OutdoorAirSystem") || Util::SameString(Alphas(3), "OAMixerOutdoorAirStreamNode") ||
                    Util::SameString(Alphas(3), "OutdoorAir:NodeList") || Util::SameString(Alphas(3), "OutdoorAir:Node") ||
                    Util::SameString(Alphas(3), "Other") || lAlphaBlanks(3)) {
                } else if (simulation_control.DuctLoss && Util::SameString(Alphas(3), "Zone")) {
                } else {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + "=\"" + Alphas(1) + "\" invalid " + cAlphaFields(3) + "=\"" +
                                        Alphas(3) + "\" illegal key.");
                    ShowContinueError(m_state,
                                      "Valid keys are: AirLoopHVAC:ZoneMixer, AirLoopHVAC:ZoneSplitter, AirLoopHVAC:OutdoorAirSystem, "
                                      "OAMixerOutdoorAirStreamNode, OutdoorAir:NodeList, OutdoorAir:Node or Other.");
                    ErrorsFound = true;
                }
                // Avoid duplication of EPlusName
                if (!simulation_control.DuctLoss) {
                    for (int j = 1; j < i; ++j) {
                        if (!Util::SameString(Alphas(2), "")) {
                            if (Util::SameString(DisSysNodeData(j).EPlusName, Alphas(2))) {
                                ShowSevereError(m_state,
                                                format(RoutineName) + CurrentModuleObject + "=\"" + Alphas(1) + "\" Duplicated " + cAlphaFields(2) +
                                                    "=\"" + Alphas(2) + "\". Please make a correction.");
                                ErrorsFound = true;
                            }
                        }
                    }
                }
            }
        } else {
            if (distribution_simulated) {
                ShowSevereError(m_state, format(RoutineName) + "An " + CurrentModuleObject + " object is required but not found.");
                ErrorsFound = true;
            }
        }

        CurrentModuleObject = "AirflowNetwork:Distribution:Component:Duct";
        if (DisSysNumOfDucts == 0) {
            if (distribution_simulated) {
                ShowSevereError(m_state, format(RoutineName) + "An " + CurrentModuleObject + " object is required but not found.");
                ErrorsFound = true;
            }
        }

        // Read AirflowNetwork distribution system component: DuctViewFactors
        CurrentModuleObject = "AirflowNetwork:Distribution:DuctViewFactors";
        DisSysNumOfDuctViewFactors = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject);
        if (DisSysNumOfDuctViewFactors > 0) {
            AirflowNetworkLinkageViewFactorData.allocate(DisSysNumOfDuctViewFactors);
            for (int i = 1; i <= DisSysNumOfDuctViewFactors; ++i) {
                m_state.dataInputProcessing->inputProcessor->getObjectItem(m_state,
                                                                           CurrentModuleObject,
                                                                           i,
                                                                           Alphas,
                                                                           NumAlphas,
                                                                           Numbers,
                                                                           NumNumbers,
                                                                           IOStatus,
                                                                           lNumericBlanks,
                                                                           lAlphaBlanks,
                                                                           cAlphaFields,
                                                                           cNumericFields);

                auto &this_VF_object(AirflowNetworkLinkageViewFactorData(i));

                this_VF_object.LinkageName = Alphas(1); // Name of linkage

                // Surface exposure fraction
                if (Numbers(2) > 1) {
                    ShowWarningError(m_state,
                                     "Duct surface exposure fraction greater than 1. Check input in: " + CurrentModuleObject + " " +
                                         this_VF_object.LinkageName);
                    ShowContinueError(m_state, "Using value of 1 for surface exposure fraction");
                    this_VF_object.DuctExposureFraction = 1;
                } else if (Numbers(2) < 0) {
                    ShowWarningError(
                        m_state, "Surface exposure fraction less than 0. Check input in: " + CurrentModuleObject + " " + this_VF_object.LinkageName);
                    ShowContinueError(m_state, "Using value of 0 for surface exposure fraction");
                    this_VF_object.DuctExposureFraction = 0;
                } else {
                    this_VF_object.DuctExposureFraction = Numbers(1);
                }

                // Duct surface emittance
                if (Numbers(2) > 1) {
                    ShowWarningError(
                        m_state, "Duct surface emittance greater than 1. Check input in: " + CurrentModuleObject + " " + this_VF_object.LinkageName);
                    ShowContinueError(m_state, "Using value of 1 for surface emittance");
                    this_VF_object.DuctEmittance = 1;
                } else if (Numbers(2) < 0) {
                    ShowWarningError(
                        m_state, "Surface exposure fraction less than 0. Check input in: " + CurrentModuleObject + " " + this_VF_object.LinkageName);
                    ShowContinueError(m_state, "Using value of 0 for surface exposure fraction");
                    this_VF_object.DuctEmittance = 0;
                } else {
                    this_VF_object.DuctEmittance = Numbers(2);
                }

                this_VF_object.ObjectNum = i;

                int numSurfaces = NumAlphas - 1;

                this_VF_object.LinkageSurfaceData.allocate(numSurfaces);

                for (int surfNum = 1; surfNum < NumAlphas; ++surfNum) {
                    this_VF_object.LinkageSurfaceData(surfNum).SurfaceName = Alphas(surfNum + 1); // Surface name
                    this_VF_object.LinkageSurfaceData(surfNum).SurfaceNum = Util::FindItemInList(Alphas(surfNum + 1), m_state.dataSurface->Surface);

                    if (this_VF_object.LinkageSurfaceData(surfNum).SurfaceNum == 0) {
                        ShowFatalError(
                            m_state, "Surface " + Alphas(surfNum + 1) + " not found. See: " + CurrentModuleObject + " " + this_VF_object.LinkageName);
                    }

                    // Surface view factor
                    if (!m_state.dataSurface->Surface(this_VF_object.LinkageSurfaceData(surfNum).SurfaceNum).HeatTransSurf) {
                        ShowWarningError(m_state,
                                         "Surface=" + Alphas(surfNum + 1) + " is not a heat transfer surface. Check input in: " +
                                             CurrentModuleObject + " " + this_VF_object.LinkageName);
                        ShowContinueError(m_state, "Using value of 0 for view factor");
                        this_VF_object.LinkageSurfaceData(surfNum).ViewFactor = 0;
                    } else if (Numbers(surfNum + 2) > 1) {
                        ShowWarningError(m_state,
                                         "View factor for surface " + Alphas(surfNum + 1) +
                                             " greater than 1. Check input in: " + CurrentModuleObject + " " + this_VF_object.LinkageName);
                        ShowContinueError(m_state, "Using value of 1 for view factor");
                        this_VF_object.LinkageSurfaceData(surfNum).ViewFactor = 1;
                    } else if (Numbers(surfNum + 2) < 0) {
                        ShowWarningError(m_state,
                                         "View factor for surface " + Alphas(surfNum + 1) + " less than 0. Check input in: " + CurrentModuleObject +
                                             " " + this_VF_object.LinkageName);
                        ShowContinueError(m_state, "Using value of 0 for view factor");
                        this_VF_object.LinkageSurfaceData(surfNum).ViewFactor = 0;
                    } else {
                        this_VF_object.LinkageSurfaceData(surfNum).ViewFactor = Numbers(surfNum + 2);
                    }
                }
            }
        }

        CurrentModuleObject = "AirflowNetwork:Distribution:Component:Fan";
        if (DisSysNumOfCVFs == 0) {
            if (distribution_simulated) {
                ShowSevereError(m_state, format(RoutineName) + "An " + CurrentModuleObject + " object is required but not found.");
                ErrorsFound = true;
            }
        }

        // Read PressureController
        CurrentModuleObject = "AirflowNetwork:ZoneControl:PressureController";
        NumOfPressureControllers = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject);
        if (NumOfPressureControllers > 1) {
            ShowSevereError(m_state,
                            format(RoutineName) + "More " + CurrentModuleObject + " are found. Currently only one( \"1\") " + CurrentModuleObject +
                                " object per simulation is allowed when using AirflowNetwork Distribution Systems.");
            ShowFatalError(
                m_state, format("{}Errors found getting {} object. Previous error(s) cause program termination.", RoutineName, CurrentModuleObject));
        }

        if (NumOfPressureControllers > 0) {
            PressureControllerData.allocate(NumOfPressureControllers);
            for (int i = 1; i <= NumOfPressureControllers; ++i) {
                m_state.dataInputProcessing->inputProcessor->getObjectItem(m_state,
                                                                           CurrentModuleObject,
                                                                           i,
                                                                           Alphas,
                                                                           NumAlphas,
                                                                           Numbers,
                                                                           NumNumbers,
                                                                           IOStatus,
                                                                           lNumericBlanks,
                                                                           lAlphaBlanks,
                                                                           cAlphaFields,
                                                                           cNumericFields);

                ErrorObjectHeader eoh{RoutineName, CurrentModuleObject, Alphas(1)};

                PressureControllerData(i).Name = Alphas(1);     // Object Name
                PressureControllerData(i).ZoneName = Alphas(2); // Zone name
                PressureControllerData(i).ZoneNum = Util::FindItemInList(Alphas(2), Zone);
                PressureControllerData(i).AFNNodeNum =
                    Util::FindItemInList(Alphas(2), MultizoneZoneData, &MultizoneZoneProp::ZoneName, AirflowNetworkNumOfZones);
                if (PressureControllerData(i).ZoneNum == 0) {
                    ShowSevereError(m_state, format(RoutineName) + CurrentModuleObject + " object, invalid " + cAlphaFields(2) + " given.");
                    ShowContinueError(m_state, "..invalid " + cAlphaFields(2) + " = \"" + PressureControllerData(i).ZoneName + "\"");
                    ErrorsFound = true;
                }

                PressureControllerData(i).ControlObjectType = Alphas(3); // Control Object Type
                PressureControllerData(i).ControlObjectName = Alphas(4); // Control Object Name

                {
                    // This SELECT_CASE_var will go on input refactor, no need to fix
                    auto const SELECT_CASE_var(Util::makeUPPER(Alphas(3)));
                    if (SELECT_CASE_var == "AIRFLOWNETWORK:MULTIZONE:COMPONENT:ZONEEXHAUSTFAN") {
                        PressureControllerData(i).ControlTypeSet = PressureCtrlExhaust;
                    } else if (SELECT_CASE_var == "AIRFLOWNETWORK:DISTRIBUTION:COMPONENT:RELIEFAIRFLOW") {
                        PressureControllerData(i).ControlTypeSet = PressureCtrlRelief;
                    } else { // Error
                        ShowSevereError(m_state,
                                        format(RoutineName) + CurrentModuleObject + " object, The entered choice for " + cAlphaFields(3) +
                                            " is not valid = \"" + PressureControllerData(i).Name + "\"");
                        ShowContinueError(m_state,
                                          "Valid choices are "
                                          "\"AirflowNetwork:MultiZone:Component:ZoneExhaustFan\",\"AirflowNetwork:Distribution:Component:"
                                          "ReliefAirFlow\"");
                        ShowContinueError(m_state, "The input choice is " + Alphas(3));
                        ErrorsFound = true;
                    }
                }

                if (PressureControllerData(i).ControlTypeSet == PressureCtrlExhaust) {
                    // This is not great
                    bool is_EXF{false};
                    auto afe = elements.find(Alphas(4));
                    if (afe != elements.end()) {
                        is_EXF = afe->second->type() == ComponentType::EXF;
                    }
                    if (!is_EXF) {
                        ShowSevereError(m_state, format(RoutineName) + CurrentModuleObject + " object, an invalid name is given:");
                        ShowContinueError(m_state, ".. invalid " + cAlphaFields(4) + " = \"" + Alphas(4) + "\".");
                        ErrorsFound = true;
                    }
                }
                if (PressureControllerData(i).ControlTypeSet == PressureCtrlRelief) {
                    // This is not great
                    bool is_REL{false};
                    auto afe = elements.find(Alphas(4));
                    if (afe != elements.end()) {
                        is_REL = afe->second->type() == ComponentType::REL;
                    }
                    if (!is_REL) {
                        ShowSevereError(m_state, format(RoutineName) + CurrentModuleObject + " object, an invalid name is given:");
                        ShowContinueError(m_state, ".. invalid " + cAlphaFields(4) + " = \"" + Alphas(4) + "\".");
                        ErrorsFound = true;
                    }
                }

                if (lAlphaBlanks(5)) {
                    PressureControllerData(i).availSched = Sched::GetScheduleAlwaysOn(m_state);
                } else if ((PressureControllerData(i).availSched = Sched::GetSchedule(m_state, Alphas(5))) == nullptr) {
                    ShowSevereItemNotFound(m_state, eoh, cAlphaFields(5), Alphas(5));
                    ErrorsFound = true;
                }

                if ((PressureControllerData(i).presSetpointSched = Sched::GetSchedule(m_state, Alphas(6))) == nullptr) {
                    ShowSevereItemNotFound(m_state, eoh, cAlphaFields(6), Alphas(6));
                    ErrorsFound = true;
                }
            }
        }

        // Assign numbers of nodes and linkages
        if (simulation_control.type != ControlType::NoMultizoneOrDistribution) {
            if (simulation_control.iWPCCnt == iWPCCntr::Input) {
                NumOfNodesMultiZone = AirflowNetworkNumOfZones + AirflowNetworkNumOfExtNode;
            } else {
                NumOfNodesMultiZone = AirflowNetworkNumOfZones + NumOfExtNodes;
            }
            NumOfLinksMultiZone = AirflowNetworkNumOfSurfaces;
            AirflowNetworkNumOfNodes = NumOfNodesMultiZone;
            if (NumOfNodesIntraZone > 0) {
                AirflowNetworkNumOfNodes = AirflowNetworkNumOfNodes + NumOfNodesIntraZone;
            }
            AirflowNetworkNumOfLinks = NumOfLinksMultiZone;
            if (NumOfLinksIntraZone > 0) {
                AirflowNetworkNumOfLinks = AirflowNetworkNumOfLinks + NumOfLinksIntraZone;
            }
        }
        if (distribution_simulated || simulation_control.DuctLoss) {
            AirflowNetworkNumOfNodes = NumOfNodesMultiZone + DisSysNumOfNodes + NumOfNodesIntraZone;
        }

        // Assign node data
        AirflowNetworkNodeData.allocate(AirflowNetworkNumOfNodes);
        // Zone node
        for (int i = 1; i <= AirflowNetworkNumOfZones; ++i) {
            AirflowNetworkNodeData(i).Name = MultizoneZoneData(i).ZoneName;
            AirflowNetworkNodeData(i).NodeTypeNum = 0;
            AirflowNetworkNodeData(i).EPlusZoneNum = MultizoneZoneData(i).ZoneNum;
            AirflowNetworkNodeData(i).NodeHeight = MultizoneZoneData(i).Height;
        }
        // External node
        if (simulation_control.iWPCCnt == iWPCCntr::Input) {
            for (int i = AirflowNetworkNumOfZones + 1; i <= NumOfNodesMultiZone; ++i) {
                AirflowNetworkNodeData(i).Name = MultizoneExternalNodeData(i - AirflowNetworkNumOfZones).Name;
                AirflowNetworkNodeData(i).NodeTypeNum = 1;
                AirflowNetworkNodeData(i).EPlusZoneNum = 0;
                AirflowNetworkNodeData(i).NodeHeight = MultizoneExternalNodeData(i - AirflowNetworkNumOfZones).height;
                AirflowNetworkNodeData(i).ExtNodeNum = i - AirflowNetworkNumOfZones;
                AirflowNetworkNodeData(i).OutAirNodeNum = MultizoneExternalNodeData(i - AirflowNetworkNumOfZones).OutAirNodeNum;
            }
        } else { // Surface-Average input
            for (int i = AirflowNetworkNumOfZones + 1; i <= NumOfNodesMultiZone; ++i) {
                n = i - AirflowNetworkNumOfZones;
                AirflowNetworkNodeData(i).Name = MultizoneExternalNodeData(n).Name;
                AirflowNetworkNodeData(i).NodeTypeNum = 1;
                AirflowNetworkNodeData(i).EPlusZoneNum = 0;
                AirflowNetworkNodeData(i).ExtNodeNum = n;
            }
        }

        // Intrazone node
        if (NumOfNodesIntraZone > 0) {
            for (int i = NumOfNodesMultiZone + 1; i <= NumOfNodesMultiZone + NumOfNodesIntraZone; ++i) {
                n = i - NumOfNodesMultiZone;
                AirflowNetworkNodeData(i).Name = IntraZoneNodeData(n).Name;
                AirflowNetworkNodeData(i).NodeTypeNum = 0;
                AirflowNetworkNodeData(i).EPlusZoneNum = IntraZoneNodeData(n).ZoneNum;
                AirflowNetworkNodeData(i).NodeHeight = IntraZoneNodeData(n).Height;
                AirflowNetworkNodeData(i).RAFNNodeNum = IntraZoneNodeData(n).RAFNNodeNum;
            }
            for (int i = 1; i <= AirflowNetworkNumOfZones; ++i) {
                if (MultizoneZoneData(i).RAFNNodeNum > 0) {
                    AirflowNetworkNodeData(i).RAFNNodeNum = MultizoneZoneData(i).RAFNNodeNum;
                }
            }
        }
        NumOfNodesMultiZone = NumOfNodesMultiZone + NumOfNodesIntraZone;

        // Check whether Distribution system is simulated
        if (AirflowNetworkNumOfNodes > NumOfNodesMultiZone) {
            // Search node types: OAMixerOutdoorAirStreamNode, OutdoorAir:NodeList, and OutdoorAir:Node
            int j = 0;
            bool OAMixerExist = false;
            for (int i = NumOfNodesMultiZone + 1; i <= AirflowNetworkNumOfNodes; ++i) {
                if (Util::SameString(DisSysNodeData(i - NumOfNodesMultiZone).EPlusType, "OAMixerOutdoorAirStreamNode")) {
                    ++j;
                    OAMixerExist = true;
                }
                if (Util::SameString(DisSysNodeData(i - NumOfNodesMultiZone).EPlusType, "OutdoorAir:NodeList")) {
                    ++j;
                }
                if (Util::SameString(DisSysNodeData(i - NumOfNodesMultiZone).EPlusType, "OutdoorAir:Node")) {
                    ++j;
                }
            }

            for (int i = NumOfNodesMultiZone + 1; i <= AirflowNetworkNumOfNodes; ++i) {
                AirflowNetworkNodeData(i).Name = DisSysNodeData(i - NumOfNodesMultiZone).Name;
                AirflowNetworkNodeData(i).NodeTypeNum = 0;
                AirflowNetworkNodeData(i).EPlusZoneNum = 0;
                AirflowNetworkNodeData(i).NodeHeight = DisSysNodeData(i - NumOfNodesMultiZone).Height;
                AirflowNetworkNodeData(i).EPlusNodeNum = DisSysNodeData(i - NumOfNodesMultiZone).EPlusNodeNum;
                // Get mixer information
                if (Util::SameString(DisSysNodeData(i - NumOfNodesMultiZone).EPlusType, "AirLoopHVAC:ZoneMixer")) {
                    AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::MIX;
                }
                // Get splitter information
                if (Util::SameString(DisSysNodeData(i - NumOfNodesMultiZone).EPlusType, "AirLoopHVAC:ZoneSplitter")) {
                    AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::SPL;
                }
                // Get outside air system information
                if (Util::SameString(DisSysNodeData(i - NumOfNodesMultiZone).EPlusType, "AirLoopHVAC:OutdoorAirSystem")) {
                    AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::OAN;
                }
                // Get OA system inlet information 'OAMixerOutdoorAirStreamNode' was specified as an outdoor air node implicitly
                if (Util::SameString(DisSysNodeData(i - NumOfNodesMultiZone).EPlusType, "OAMixerOutdoorAirStreamNode")) {
                    AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::EXT;
                    AirflowNetworkNodeData(i).ExtNodeNum = AirflowNetworkNumOfExtNode + 1;
                    AirflowNetworkNodeData(i).NodeTypeNum = 1;
                }
                if (Util::SameString(DisSysNodeData(i - NumOfNodesMultiZone).EPlusType, "OutdoorAir:NodeList") ||
                    Util::SameString(DisSysNodeData(i - NumOfNodesMultiZone).EPlusType, "OutdoorAir:Node")) {
                    if (j > 1 && OAMixerExist) {
                        AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::EXT;
                        AirflowNetworkNodeData(i).ExtNodeNum = AirflowNetworkNumOfExtNode + 1;
                        AirflowNetworkNodeData(i).NodeTypeNum = 1;
                    } else if (j > 0 && !OAMixerExist) {
                        AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::EXT;
                        AirflowNetworkNodeData(i).ExtNodeNum = AirflowNetworkNumOfExtNode + 1;
                        AirflowNetworkNodeData(i).NodeTypeNum = 1;
                    } else {
                        ShowSevereError(m_state,
                                        format(RoutineName) + "AirflowNetwork:Distribution:Node: The outdoor air node is found at " +
                                            AirflowNetworkNodeData(i).Name);
                        ShowContinueError(m_state,
                                          "The node with Component Object Type = OAMixerOutdoorAirStreamNode is not found. Please check inputs.");
                        ErrorsFound = true;
                    }
                }
                // Get Zone node for Duct Loss calculation
                if (simulation_control.DuctLoss && Util::SameString(DisSysNodeData(i - NumOfNodesMultiZone).EPlusType, "Zone")) {
                    AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::ZON;
                    if (Util::FindItemInList(AirflowNetworkNodeData(i).Name, Zone) == 0) {
                        ShowSevereError(m_state,
                                        format(RoutineName) + "AirflowNetwork:Distribution:Node object, Component Object Type or Node Type = Zone");
                        ShowContinueError(m_state, "..invalid Zone name = \"" + DisSysNodeData(i - NumOfNodesMultiZone).Name + "\"");
                        ErrorsFound = true;
                    } else {
                        AirflowNetworkNodeData(i).EPlusZoneNum = Util::FindItemInList(AirflowNetworkNodeData(i).Name, Zone);
                    }
                }
            }
        }

        // Start to assembly AirflowNetwork Components
        AirflowNetworkNumOfComps = AirflowNetworkNumOfDetOpenings + AirflowNetworkNumOfSimOpenings + AirflowNetworkNumOfSurCracks +
                                   AirflowNetworkNumOfSurELA + DisSysNumOfLeaks + DisSysNumOfELRs + DisSysNumOfDucts + DisSysNumOfDampers +
                                   DisSysNumOfCVFs + DisSysNumOfDetFans + DisSysNumOfCPDs + DisSysNumOfCoils + DisSysNumOfTermUnits +
                                   AirflowNetworkNumOfExhFan + DisSysNumOfHXs + AirflowNetworkNumOfHorOpenings + NumOfOAFans + NumOfReliefFans +
                                   AirflowNetworkNumOfSFR;
        AirflowNetworkCompData.allocate(AirflowNetworkNumOfComps);

        for (int i = 1; i <= AirflowNetworkNumOfDetOpenings; ++i) { // Detailed opening component
            AirflowNetworkCompData(i).Name = MultizoneCompDetOpeningData(i).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::DOP;
            AirflowNetworkCompData(i).TypeNum = i;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
        }

        int j = AirflowNetworkNumOfDetOpenings;
        for (int i = 1 + j; i <= AirflowNetworkNumOfSimOpenings + j; ++i) { // Simple opening component
            n = i - j;
            AirflowNetworkCompData(i).Name = MultizoneCompSimpleOpeningData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::SOP;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
        }

        j += AirflowNetworkNumOfSimOpenings;
        for (int i = 1 + j; i <= AirflowNetworkNumOfSurCracks + j; ++i) { // Surface crack component
            n = i - j;
            AirflowNetworkCompData(i).Name = MultizoneSurfaceCrackData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::SCR;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
        }

        j += AirflowNetworkNumOfSurCracks;
        for (int i = 1 + j; i <= AirflowNetworkNumOfSurELA + j; ++i) { // Surface crack component
            n = i - j;
            AirflowNetworkCompData(i).Name = MultizoneSurfaceELAData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::SEL;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
        }

        j += AirflowNetworkNumOfSurELA;
        for (int i = 1 + j; i <= AirflowNetworkNumOfExhFan + j; ++i) { // Zone exhaust fan component
            n = i - j;
            AirflowNetworkCompData(i).Name = MultizoneCompExhaustFanData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::EXF;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
        }

        j += AirflowNetworkNumOfExhFan;
        for (int i = 1 + j; i <= AirflowNetworkNumOfHorOpenings + j; ++i) { // Distribution system crack component
            n = i - j;
            AirflowNetworkCompData(i).Name = MultizoneCompHorOpeningData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::HOP;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
        }

        j += AirflowNetworkNumOfHorOpenings;
        for (int i = 1 + j; i <= DisSysNumOfLeaks + j; ++i) { // Distribution system crack component
            n = i - j;
            AirflowNetworkCompData(i).Name = DisSysCompLeakData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::PLR;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
        }

        j += DisSysNumOfLeaks;
        for (int i = 1 + j; i <= DisSysNumOfELRs + j; ++i) { // Distribution system effective leakage ratio component
            n = i - j;
            AirflowNetworkCompData(i).Name = DisSysCompELRData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::ELR;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
        }

        j += DisSysNumOfELRs;
        for (int i = 1 + j; i <= DisSysNumOfDucts + j; ++i) { // Distribution system effective leakage ratio component
            n = i - j;
            AirflowNetworkCompData(i).Name = DisSysCompDuctData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::DWC;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
        }

        j += DisSysNumOfDucts;
        for (int i = 1 + j; i <= DisSysNumOfDampers + j; ++i) { // Distribution system effective leakage ratio component
            n = i - j;
            AirflowNetworkCompData(i).Name = DisSysCompDamperData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::DMP;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
        }

        j += DisSysNumOfDampers;
        for (int i = 1 + j; i <= DisSysNumOfCVFs + j; ++i) { // Distribution system constant volume fan component
            n = i - j;
            AirflowNetworkCompData(i).Name = DisSysCompCVFData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::CVF;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
            AirflowNetworkCompData(i).EPlusTypeNum = iEPlusComponentType::FAN;
        }

        j += DisSysNumOfCVFs;
        for (int i = 1 + j; i <= DisSysNumOfDetFans + j; ++i) { // Distribution system fan component
            n = i - j;
            AirflowNetworkCompData(i).Name = DisSysCompDetFanData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::FAN;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
            AirflowNetworkCompData(i).EPlusTypeNum = iEPlusComponentType::FAN;
        }

        j += DisSysNumOfDetFans;
        for (int i = 1 + j; i <= DisSysNumOfCPDs + j; ++i) { // Distribution system constant pressure drop component
            n = i - j;
            AirflowNetworkCompData(i).Name = DisSysCompCPDData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::CPD;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
        }

        j += DisSysNumOfCPDs;
        for (int i = 1 + j; i <= DisSysNumOfCoils + j; ++i) { // Distribution system coil component
            n = i - j;
            AirflowNetworkCompData(i).Name = DisSysCompCoilData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::COI;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
            AirflowNetworkCompData(i).EPlusTypeNum = iEPlusComponentType::COI;
        }

        j += DisSysNumOfCoils;
        for (int i = 1 + j; i <= DisSysNumOfTermUnits + j; ++i) { // Terminal unit component
            n = i - j;
            AirflowNetworkCompData(i).Name = DisSysCompTermUnitData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::TMU;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
            AirflowNetworkCompData(i).EPlusTypeNum = iEPlusComponentType::RHT;
        }

        j += DisSysNumOfTermUnits;
        for (int i = 1 + j; i <= DisSysNumOfHXs + j; ++i) { // Distribution system heat exchanger component
            n = i - j;
            AirflowNetworkCompData(i).Name = DisSysCompHXData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::HEX;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
            AirflowNetworkCompData(i).EPlusTypeNum = iEPlusComponentType::HEX;
        }

        j += DisSysNumOfHXs;
        for (int i = 1 + j; i <= NumOfOAFans + j; ++i) { // OA fan component
            n = i - j;
            AirflowNetworkCompData(i).Name = DisSysCompOutdoorAirData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::OAF;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
        }

        j += NumOfOAFans;
        for (int i = 1 + j; i <= NumOfReliefFans + j; ++i) { // OA fan component
            n = i - j;
            AirflowNetworkCompData(i).Name = DisSysCompReliefAirData(n).name;
            compnum[AirflowNetworkCompData(i).Name] = i;
            AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::REL;
            AirflowNetworkCompData(i).TypeNum = n;
            AirflowNetworkCompData(i).EPlusName = "";
            AirflowNetworkCompData(i).EPlusCompName = "";
            AirflowNetworkCompData(i).EPlusType = "";
            AirflowNetworkCompData(i).CompNum = i;
        }

        // This is also a bit of a hack to keep things working, this needs to be removed ASAP
        j += NumOfReliefFans;
        int ii = 1 + j;
        int type_i = 1;
        for (auto const &el : SpecifiedMassFlowData) {
            AirflowNetworkCompData(ii).Name = el.name;
            compnum[el.name] = ii;
            AirflowNetworkCompData(ii).CompTypeNum = iComponentTypeNum::SMF;
            AirflowNetworkCompData(ii).TypeNum = type_i;
            AirflowNetworkCompData(ii).EPlusName = "";
            AirflowNetworkCompData(ii).EPlusCompName = "";
            AirflowNetworkCompData(ii).EPlusType = "";
            AirflowNetworkCompData(ii).CompNum = ii;
            ++ii;
            ++type_i;
        }

        type_i = 1;
        for (auto const &el : SpecifiedVolumeFlowData) {
            AirflowNetworkCompData(ii).Name = el.name;
            compnum[el.name] = ii;
            AirflowNetworkCompData(ii).CompTypeNum = iComponentTypeNum::SVF;
            AirflowNetworkCompData(ii).TypeNum = type_i;
            AirflowNetworkCompData(ii).EPlusName = "";
            AirflowNetworkCompData(ii).EPlusCompName = "";
            AirflowNetworkCompData(ii).EPlusType = "";
            AirflowNetworkCompData(ii).CompNum = ii;
            ++ii;
            ++type_i;
        }

        // Assign linkage data

        // Read AirflowNetwork linkage data
        CurrentModuleObject = "AirflowNetwork:Distribution:Linkage";
        DisSysNumOfLinks = m_state.dataInputProcessing->inputProcessor->getNumObjectsFound(m_state, CurrentModuleObject);
        if (DisSysNumOfLinks > 0 && distribution_simulated) { // Multizone + Distribution
            AirflowNetworkNumOfLinks = NumOfLinksMultiZone + DisSysNumOfLinks;
            AirflowNetworkLinkageData.allocate(DisSysNumOfLinks + AirflowNetworkNumOfSurfaces);
        } else if (simulation_control.DuctLoss) {
            AirflowNetworkNumOfLinks = DisSysNumOfLinks;
            AirflowNetworkLinkageData.allocate(DisSysNumOfLinks);
        } else { // Multizone + IntraZone only
            //    AirflowNetworkLinkageData.allocate( AirflowNetworkNumOfSurfaces );
            AirflowNetworkLinkageData.allocate(AirflowNetworkNumOfLinks);
        }

        // Assign Multizone linkage based on surfaces, by assuming every surface has a crack or opening
        j = 0;
        for (count = 1; count <= AirflowNetworkNumOfSurfaces; ++count) {
            if (MultizoneSurfaceData(count).SurfNum == 0) {
                continue;
            }
            AirflowNetworkLinkageData(count).Name = MultizoneSurfaceData(count).SurfName;
            AirflowNetworkLinkageData(count).NodeNums[0] = MultizoneSurfaceData(count).NodeNums[0];
            AirflowNetworkLinkageData(count).NodeNums[1] = MultizoneSurfaceData(count).NodeNums[1];
            AirflowNetworkLinkageData(count).CompName = MultizoneSurfaceData(count).OpeningName;
            AirflowNetworkLinkageData(count).ZoneNum = 0;
            AirflowNetworkLinkageData(count).LinkNum = count;
            AirflowNetworkLinkageData(count).NodeHeights[0] = MultizoneSurfaceData(count).CHeight;
            AirflowNetworkLinkageData(count).NodeHeights[1] = MultizoneSurfaceData(count).CHeight;
            if (!m_state.dataSurface->WorldCoordSystem) {
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(count).NodeNums[0]).EPlusZoneNum > 0) {
                    AirflowNetworkLinkageData(count).NodeHeights[0] -=
                        Zone(AirflowNetworkNodeData(AirflowNetworkLinkageData(count).NodeNums[0]).EPlusZoneNum).OriginZ;
                }
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(count).NodeNums[1]).EPlusZoneNum > 0) {
                    AirflowNetworkLinkageData(count).NodeHeights[1] -=
                        Zone(AirflowNetworkNodeData(AirflowNetworkLinkageData(count).NodeNums[1]).EPlusZoneNum).OriginZ;
                }
            }
            // Find component number
            auto afe = elements.find(AirflowNetworkLinkageData(count).CompName);
            if (afe != elements.end()) {
                // found = false;
                // for (i = 1; i <= AirflowNetworkNumOfComps; ++i) {
                AirflowNetworkLinkageData(count).element = afe->second;
                // Get CompTypeNum here, this is a hack to hold us over until the introspection is dealt with
                auto compnum_iter = compnum.find(AirflowNetworkLinkageData(count).CompName);
                assert(compnum_iter != compnum.end());
                int compnum = compnum_iter->second;
                AirflowNetworkLinkageData(count).CompNum = compnum;

                auto const &surf = m_state.dataSurface->Surface(MultizoneSurfaceData(count).SurfNum);

                switch (AirflowNetworkLinkageData(count).element->type()) {
                case ComponentType::DOP: {
                    // if (AirflowNetworkLinkageData(count).CompName ==
                    // AirflowNetworkCompData(i).Name) {
                    //    AirflowNetworkLinkageData(count).CompNum = i;
                    //    found = true;
                    //    if (AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::DOP) {
                    ++j;
                    AirflowNetworkLinkageData(count).DetOpenNum = j;
                    MultizoneSurfaceData(count).Multiplier = surf.Multiplier;
                    if (surf.Tilt < 10.0 || surf.Tilt > 170.0) {
                        ShowWarningError(m_state, "An AirflowNetwork:Multizone:Surface object has an air-flow opening corresponding to");
                        ShowContinueError(m_state, "window or door = " + MultizoneSurfaceData(count).SurfName + ", which is within ");
                        ShowContinueError(m_state, "10 deg of being horizontal. Airflows through large horizontal openings are poorly");
                        ShowContinueError(m_state, "modeled in the AirflowNetwork model resulting in only one-way airflow.");
                    }
                    if (!(surf.OriginalClass == SurfaceClass::Window || surf.OriginalClass == SurfaceClass::GlassDoor ||
                          surf.OriginalClass == SurfaceClass::Door || surf.IsAirBoundarySurf)) {
                        ShowSevereError(m_state,
                                        format(RoutineName) +
                                            "AirflowNetworkComponent: The opening must be assigned to a window, door, glassdoor or air boundary at " +
                                            AirflowNetworkLinkageData(count).Name);
                        ErrorsFound = true;
                    }

                    if (surf.OriginalClass == SurfaceClass::Door || surf.OriginalClass == SurfaceClass::GlassDoor) {
                        if (MultizoneCompDetOpeningData(AirflowNetworkCompData(compnum).TypeNum).LVOType == 2) {
                            ShowSevereError(m_state,
                                            format(RoutineName) +
                                                "AirflowNetworkComponent: The opening with horizontally pivoted type must be assigned to a "
                                                "window surface at " +
                                                AirflowNetworkLinkageData(count).Name);
                            ErrorsFound = true;
                        }
                    }
                } break;
                case ComponentType::SOP: {
                    // if (AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::SOP) {
                    MultizoneSurfaceData(count).Multiplier = surf.Multiplier;
                    if (surf.Tilt < 10.0 || surf.Tilt > 170.0) {
                        ShowSevereError(m_state, "An AirflowNetwork:Multizone:Surface object has an air-flow opening corresponding to");
                        ShowContinueError(m_state, "window or door = " + MultizoneSurfaceData(count).SurfName + ", which is within");
                        ShowContinueError(m_state, "10 deg of being horizontal. Airflows through horizontal openings are not allowed.");
                        ShowContinueError(m_state, "AirflowNetwork:Multizone:Component:SimpleOpening = " + AirflowNetworkCompData(compnum).Name);
                        ErrorsFound = true;
                    }

                    if (!(surf.OriginalClass == SurfaceClass::Window || surf.OriginalClass == SurfaceClass::GlassDoor ||
                          surf.OriginalClass == SurfaceClass::Door || surf.IsAirBoundarySurf)) {
                        ShowSevereError(m_state,
                                        format(RoutineName) +
                                            "AirflowNetworkComponent: The opening must be assigned to a window, door, glassdoor or air boundary at " +
                                            AirflowNetworkLinkageData(count).Name);
                        ErrorsFound = true;
                    }
                } break;
                case ComponentType::HOP: {
                    // if (AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::HOP) {
                    MultizoneSurfaceData(count).Multiplier = surf.Multiplier;
                    // Get linkage height from upper and lower zones
                    if (MultizoneZoneData(AirflowNetworkLinkageData(count).NodeNums[0]).ZoneNum > 0) {
                        AirflowNetworkLinkageData(count).NodeHeights[0] =
                            Zone(MultizoneZoneData(AirflowNetworkLinkageData(count).NodeNums[0]).ZoneNum).Centroid.z;
                    }
                    if (AirflowNetworkLinkageData(count).NodeNums[1] <= AirflowNetworkNumOfZones) {
                        if (MultizoneZoneData(AirflowNetworkLinkageData(count).NodeNums[1]).ZoneNum > 0) {
                            AirflowNetworkLinkageData(count).NodeHeights[1] =
                                Zone(m_state.afn->MultizoneZoneData(AirflowNetworkLinkageData(count).NodeNums[1]).ZoneNum).Centroid.z;
                        }
                    }
                    if (AirflowNetworkLinkageData(count).NodeNums[1] > AirflowNetworkNumOfZones) {
                        ShowSevereError(m_state,
                                        format(RoutineName) +
                                            "AirflowNetworkComponent: The horizontal opening must be located between two thermal zones at " +
                                            AirflowNetworkLinkageData(count).Name);
                        ShowContinueError(m_state, "This component is exposed to outdoors.");
                        ErrorsFound = true;
                    } else {
                        if (!(MultizoneZoneData(AirflowNetworkLinkageData(count).NodeNums[0]).ZoneNum > 0 &&
                              MultizoneZoneData(AirflowNetworkLinkageData(count).NodeNums[1]).ZoneNum > 0)) {
                            ShowSevereError(m_state,
                                            format(RoutineName) +
                                                "AirflowNetworkComponent: The horizontal opening must be located between two thermal zones at " +
                                                AirflowNetworkLinkageData(count).Name);
                            ErrorsFound = true;
                        }
                    }
                    if (!(surf.Tilt > 170.0 && surf.Tilt < 190.0) && !(surf.Tilt > -10.0 && surf.Tilt < 10.0)) {
                        ShowWarningError(m_state, "An AirflowNetwork:Multizone:Surface object has an air-flow opening corresponding to");
                        ShowContinueError(m_state, "window or door = " + MultizoneSurfaceData(count).SurfName + ", which is above");
                        ShowContinueError(m_state, "10 deg of being horizontal. Airflows through non-horizontal openings are not modeled");
                        ShowContinueError(m_state,
                                          "with the object of AirflowNetwork:Multizone:Component:HorizontalOpening = " +
                                              AirflowNetworkCompData(compnum).Name);
                    }
                    if (!(surf.OriginalClass == SurfaceClass::Window || surf.OriginalClass == SurfaceClass::GlassDoor ||
                          surf.OriginalClass == SurfaceClass::Door || surf.IsAirBoundarySurf)) {
                        ShowSevereError(m_state,
                                        format(RoutineName) +
                                            "AirflowNetworkComponent: The opening must be assigned to a window, door, glassdoor or air boundary at " +
                                            AirflowNetworkLinkageData(count).Name);
                        ErrorsFound = true;
                    }
                } break;
                default:
                    // Nothing to do here
                    break;
                }
            } else {
                ShowSevereError(m_state,
                                format(RoutineName) + CurrentModuleObject + ": The component is not defined in " +
                                    AirflowNetworkLinkageData(count).Name);
                ErrorsFound = true;
            }
        }

        // Assign intrazone links
        for (count = 1 + AirflowNetworkNumOfSurfaces; count <= NumOfLinksIntraZone + AirflowNetworkNumOfSurfaces; ++count) {
            AirflowNetworkLinkageData(count).Name = IntraZoneLinkageData(count - AirflowNetworkNumOfSurfaces).Name;
            AirflowNetworkLinkageData(count).NodeNums[0] = IntraZoneLinkageData(count - AirflowNetworkNumOfSurfaces).NodeNums[0];
            AirflowNetworkLinkageData(count).NodeNums[1] = IntraZoneLinkageData(count - AirflowNetworkNumOfSurfaces).NodeNums[1];
            AirflowNetworkLinkageData(count).CompName = IntraZoneLinkageData(count - AirflowNetworkNumOfSurfaces).CompName;
            AirflowNetworkLinkageData(count).ZoneNum = 0;
            AirflowNetworkLinkageData(count).LinkNum = count;
            AirflowNetworkLinkageData(count).NodeHeights[0] = IntraZoneLinkageData(count - AirflowNetworkNumOfSurfaces).NodeHeights[0];
            AirflowNetworkLinkageData(count).NodeHeights[1] = IntraZoneLinkageData(count - AirflowNetworkNumOfSurfaces).NodeHeights[1];
            // Find component number
            auto afe = elements.find(AirflowNetworkLinkageData(count).CompName);
            if (afe != elements.end()) {
                AirflowNetworkLinkageData(count).element = afe->second;
                // Get CompTypeNum here, this is a hack to hold us over until the introspection is dealt with
                auto compnum_iter = compnum.find(AirflowNetworkLinkageData(count).CompName);
                assert(compnum_iter != compnum.end());
                int compnum = compnum_iter->second;
                AirflowNetworkLinkageData(count).CompNum = compnum;
                if (AirflowNetworkLinkageData(count).element->type() != ComponentType::SCR &&
                    AirflowNetworkLinkageData(count).element->type() != ComponentType::SEL) {

                    ShowSevereError(m_state,
                                    format(RoutineName) + AirflowNetworkLinkageData(count).CompName + ": The component is not allowed in " +
                                        AirflowNetworkLinkageData(count).Name);
                    ShowContinueError(m_state,
                                      "The allowed component type is either AirflowNetwork:MultiZone:Surface:Crack or "
                                      "AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea.");
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(m_state,
                                format(RoutineName) + AirflowNetworkLinkageData(count).CompName + ": The component is not defined in " +
                                    AirflowNetworkLinkageData(count).Name);
                ErrorsFound = true;
            }
        }

        // Reset AirflowNetworkNumOfSurfaces by including NumOfLinksIntraZone
        AirflowNetworkNumOfSurfaces = AirflowNetworkNumOfSurfaces + NumOfLinksIntraZone;
        if (NumOfLinksIntraZone > 0) {
            NumOfLinksMultiZone = AirflowNetworkNumOfSurfaces;
        }

        // Assign AirflowNetwork info in RoomAirflowNetworkZoneInfo
        if (NumOfNodesIntraZone > 0) {
            for (int i = 1; i <= NumOfNodesMultiZone; ++i) {
                n = AirflowNetworkNodeData(i).EPlusZoneNum;
                AirflowNetworkNodeData(i).NumOfLinks = 0;
                if (n > 0 && AirflowNetworkNodeData(i).RAFNNodeNum > 0) {
                    m_state.dataRoomAir->AFNZoneInfo(n).Node(AirflowNetworkNodeData(i).RAFNNodeNum).AFNNodeID = i;
                    for (j = 1; j <= AirflowNetworkNumOfSurfaces; ++j) {
                        if (AirflowNetworkLinkageData(j).NodeNums[0] == i) {
                            AirflowNetworkNodeData(i).NumOfLinks = AirflowNetworkNodeData(i).NumOfLinks + 1;
                        } else if (AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                            AirflowNetworkNodeData(i).NumOfLinks = AirflowNetworkNodeData(i).NumOfLinks + 1;
                        } else {
                        }
                    }
                }
                if (AirflowNetworkNodeData(i).RAFNNodeNum > 0) {
                    for (j = 1; j <= m_state.dataRoomAir->AFNZoneInfo(n).NumOfAirNodes; ++j) {
                        if (m_state.dataRoomAir->AFNZoneInfo(n).Node(j).AFNNodeID == i) {
                            m_state.dataRoomAir->AFNZoneInfo(n).Node(j).NumOfAirflowLinks = AirflowNetworkNodeData(i).NumOfLinks;
                            m_state.dataRoomAir->AFNZoneInfo(n).Node(j).Link.allocate(AirflowNetworkNodeData(i).NumOfLinks);
                            k = 1;
                            for (int m = 1; m <= AirflowNetworkNumOfSurfaces; ++m) {
                                if (AirflowNetworkLinkageData(m).NodeNums[0] == i) {
                                    m_state.dataRoomAir->AFNZoneInfo(n).Node(j).Link(k).AFNSimuID = m;
                                    m_state.dataRoomAir->AFNZoneInfo(n).Node(j).Link(k).AFNDataID = m;
                                    k = k + 1;
                                    if (k > AirflowNetworkNodeData(i).NumOfLinks) {
                                        break;
                                    }
                                }
                                if (AirflowNetworkLinkageData(m).NodeNums[1] == i) {
                                    m_state.dataRoomAir->AFNZoneInfo(n).Node(j).Link(k).AFNSimuID = m;
                                    m_state.dataRoomAir->AFNZoneInfo(n).Node(j).Link(k).AFNDataID = m;
                                    k = k + 1;
                                    if (k > AirflowNetworkNodeData(i).NumOfLinks) {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (DisSysNumOfLinks > 0 && (distribution_simulated || simulation_control.DuctLoss)) { // Distribution

            for (auto &e : AirflowNetworkLinkageData) {
                e.ZoneNum = 0;
            }

            for (count = AirflowNetworkNumOfSurfaces + 1; count <= AirflowNetworkNumOfLinks; ++count) {

                m_state.dataInputProcessing->inputProcessor->getObjectItem(m_state,
                                                                           CurrentModuleObject,
                                                                           count - AirflowNetworkNumOfSurfaces,
                                                                           Alphas,
                                                                           NumAlphas,
                                                                           Numbers,
                                                                           NumNumbers,
                                                                           IOStatus,
                                                                           lNumericBlanks,
                                                                           lAlphaBlanks,
                                                                           cAlphaFields,
                                                                           cNumericFields);
                AirflowNetworkLinkageData(count).Name = Alphas(1);
                AirflowNetworkLinkageData(count).NodeNames[0] = Alphas(2);
                AirflowNetworkLinkageData(count).NodeHeights[0] = 0.0;
                AirflowNetworkLinkageData(count).NodeNames[1] = Alphas(3);
                AirflowNetworkLinkageData(count).NodeHeights[1] = 0.0;
                AirflowNetworkLinkageData(count).CompName = Alphas(4);
                AirflowNetworkLinkageData(count).ZoneName = Alphas(5);
                AirflowNetworkLinkageData(count).LinkNum = count;

                for (int i = 1; i <= DisSysNumOfDuctViewFactors; ++i) {
                    if (AirflowNetworkLinkageData(count).Name == AirflowNetworkLinkageViewFactorData(i).LinkageName) {
                        AirflowNetworkLinkageData(count).LinkageViewFactorObjectNum = AirflowNetworkLinkageViewFactorData(i).ObjectNum;
                        break;
                    }
                }

                if (!lAlphaBlanks(5)) {
                    AirflowNetworkLinkageData(count).ZoneNum = Util::FindItemInList(AirflowNetworkLinkageData(count).ZoneName, Zone);
                    if (AirflowNetworkLinkageData(count).ZoneNum == 0) {
                        ShowSevereError(m_state,
                                        format(RoutineName) + CurrentModuleObject + ": Invalid " + cAlphaFields(5) +
                                            " given = " + AirflowNetworkLinkageData(count).ZoneName);
                        ErrorsFound = true;
                    }
                }
                if (Alphas(2) == Alphas(3)) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + ", " + cAlphaFields(2) + " = " + cAlphaFields(3) + " in " +
                                        AirflowNetworkLinkageData(count).Name);
                    ErrorsFound = true;
                }
                // Find component number
                auto afe = elements.find(AirflowNetworkLinkageData(count).CompName);
                if (afe != elements.end()) {
                    AirflowNetworkLinkageData(count).element = afe->second;

                    // Get CompTypeNum here, this is a hack to hold us over until the introspection is dealt with
                    auto compnum_iter = compnum.find(AirflowNetworkLinkageData(count).CompName);
                    assert(compnum_iter != compnum.end());
                    int compnum = compnum_iter->second;
                    AirflowNetworkLinkageData(count).CompNum = compnum;
                } else {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + ": The " + cAlphaFields(4) + " is not defined in " +
                                        AirflowNetworkLinkageData(count).Name);
                    ErrorsFound = true;
                }
                // Find Node number
                found = false;
                for (int i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
                    if (AirflowNetworkLinkageData(count).NodeNames[0] == AirflowNetworkNodeData(i).Name) {
                        AirflowNetworkLinkageData(count).NodeNums[0] = i;
                        AirflowNetworkLinkageData(count).NodeHeights[0] += AirflowNetworkNodeData(i).NodeHeight;
                        found = true;
                        break;
                    }
                }
                if (!found && !simulation_control.DuctLoss) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + ": The " + cAlphaFields(2) + " is not found in the node data " +
                                        AirflowNetworkLinkageData(count).Name);
                    ErrorsFound = true;
                }
                for (int i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
                    if (AirflowNetworkLinkageData(count).NodeNames[1] == AirflowNetworkNodeData(i).Name) {
                        AirflowNetworkLinkageData(count).NodeNums[1] = i;
                        AirflowNetworkLinkageData(count).NodeHeights[1] += AirflowNetworkNodeData(i).NodeHeight;
                        found = true;
                        break;
                    }
                }
                if (!found && !simulation_control.DuctLoss) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + ": The " + cAlphaFields(3) + " is not found in the node data " +
                                        AirflowNetworkLinkageData(count).Name);
                    ErrorsFound = true;
                }
            }
        } else {

            if (distribution_simulated) {
                ShowSevereError(m_state, format(RoutineName) + "An " + CurrentModuleObject + " object is required but not found.");
                ErrorsFound = true;
            }
        }

        if (simulation_control.DuctLoss) {
            // Assign node num based on Distribution node
            for (int i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
                for (int j = 1; j <= DisSysNumOfNodes; ++j) {
                    if (Util::SameString(AirflowNetworkLinkageData(i).NodeNames[0], DisSysNodeData(j).Name)) {
                        AirflowNetworkLinkageData(i).NodeNums[0] = j;
                    }
                    if (Util::SameString(AirflowNetworkLinkageData(i).NodeNames[1], DisSysNodeData(j).Name)) {
                        AirflowNetworkLinkageData(i).NodeNums[1] = j;
                    }
                }
                if (AirflowNetworkLinkageData(i).NodeNums[0] == 0) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + ": The " + AirflowNetworkLinkageData(i).NodeNames[0] +
                                        " is not found in the AirflowNetwork:Distribution:Node. " + AirflowNetworkLinkageData(i).Name);
                    ErrorsFound = true;
                }
                if (AirflowNetworkLinkageData(i).NodeNums[1] == 0) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + ": The " + AirflowNetworkLinkageData(i).NodeNames[1] +
                                        " is not found in the AirflowNetwork:Distribution:Node. " + AirflowNetworkLinkageData(i).Name);
                    ErrorsFound = true;
                }
            }
            if (ErrorsFound) {
                ShowFatalError(m_state, format("{}Errors found getting inputs. Previous error(s) cause program termination.", RoutineName));
            }

            return;
        }
        // Ensure no duplicated names in AirflowNetwork component objects
        //        for (i = 1; i <= AirflowNetworkNumOfComps; ++i) {
        //            for (j = i + 1; j <= AirflowNetworkNumOfComps; ++j) {
        //                if (Util::SameString(AirflowNetworkCompData(i).Name,
        //        AirflowNetworkCompData(j).Name)) {
        //                    // SurfaceAirflowLeakageNames
        //                    if (i <= 4 && j <= 4) {
        //                        if (AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::DOP)
        //                            CompName(1) = "AirflowNetwork:MultiZone:Component:DetailedOpening";
        //                        if (AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::SOP)
        //                            CompName(1) = "AirflowNetwork:MultiZone:Component:SimpleOpening";
        //                        if (AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::SCR) CompName(1) =
        //        "AirflowNetwork:MultiZone:Surface:Crack"; if (AirflowNetworkCompData(i).CompTypeNum ==
        //        iComponentTypeNum::SEL) CompName(1) = "AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea"; if
        //        (AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::DOP) CompName(2) =
        //        "AirflowNetwork:MultiZone:Component:DetailedOpening"; if (AirflowNetworkCompData(j).CompTypeNum ==
        //        iComponentTypeNum::SOP) CompName(2) = "AirflowNetwork:MultiZone:Component:SimpleOpening"; if
        //        (AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::SCR) CompName(2) =
        //        "AirflowNetwork:MultiZone:Surface:Crack"; if (AirflowNetworkCompData(j).CompTypeNum ==
        //        iComponentTypeNum::SEL) CompName(2) = "AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea"; ShowSevereError(m_state, RoutineName
        //        + "Duplicated component names are found = " + AirflowNetworkCompData(i).Name); ShowContinueError(m_state,
        //        "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2)); ErrorsFound = true;
        //                    }
        //                    // Distribution component
        //                    if (i > 4 && j > 4) {
        //                        if (AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::PLR) CompName(1) =
        //        "AirflowNetwork:Distribution:Component:Leak"; if (AirflowNetworkCompData(i).CompTypeNum ==
        //        iComponentTypeNum::DWC) CompName(1) = "AirflowNetwork:Distribution:Component:Duct"; if
        //        (AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::ELR) CompName(1) =
        //        "AirflowNetwork:Distribution:Component:LeakageRatio"; if (AirflowNetworkCompData(i).CompTypeNum ==
        //        iComponentTypeNum::DMP) CompName(1) = "AIRFLOWNETWORK:DISTRIBUTION:COMPONENT DAMPER"; if
        //        (AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::CVF) CompName(1) =
        //        "AirflowNetwork:Distribution:Component:Fan"; if (AirflowNetworkCompData(i).CompTypeNum ==
        //        iComponentTypeNum::CPD) CompName(1) = "AirflowNetwork:Distribution:Component:ConstantPressureDrop"; if
        //        (AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::COI) CompName(1) =
        //        "AirflowNetwork:Distribution:Component:Coil"; if (AirflowNetworkCompData(i).CompTypeNum ==
        //        iComponentTypeNum::TMU) CompName(1) = "AirflowNetwork:Distribution:Component:TerminalUnit"; if
        //        (AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::HEX) CompName(1) =
        //        "AirflowNetwork:Distribution:Component:HeatExchanger"; if (AirflowNetworkCompData(j).CompTypeNum ==
        //        iComponentTypeNum::PLR) CompName(2) = "AirflowNetwork:Distribution:Component:Leak"; if
        //        (AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::DWC) CompName(2) =
        //        "AirflowNetwork:Distribution:Component:Duct"; if (AirflowNetworkCompData(j).CompTypeNum ==
        //        iComponentTypeNum::ELR) CompName(2) = "AirflowNetwork:Distribution:Component:LeakageRatio"; if
        //        (AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::DMP) CompName(2) =
        //        "AIRFLOWNETWORK:DISTRIBUTION:COMPONENT DAMPER"; if (AirflowNetworkCompData(j).CompTypeNum ==
        //        iComponentTypeNum::CVF) CompName(2) = "AirflowNetwork:Distribution:Component:Fan"; if
        //        (AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::CPD) CompName(2) =
        //        "AirflowNetwork:Distribution:Component:ConstantPressureDrop"; if (AirflowNetworkCompData(j).CompTypeNum
        //        == iComponentTypeNum::COI) CompName(2) = "AirflowNetwork:Distribution:Component:Coil"; if
        //        (AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::TMU) CompName(2) =
        //        "AirflowNetwork:Distribution:Component:TerminalUnit"; if (AirflowNetworkCompData(j).CompTypeNum ==
        //        iComponentTypeNum::HEX) CompName(2) = "AirflowNetwork:Distribution:Component:HeatExchanger"; ShowSevereError(m_state,
        //        format(RoutineName) + "Duplicated component names are found = " + AirflowNetworkCompData(i).Name);
        //        ShowContinueError(m_state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
        //        ErrorsFound = true;
        //                    }
        //                }
        //            }
        //        }

        // Node and component validation
        for (count = 1; count <= AirflowNetworkNumOfLinks; ++count) {
            NodeFound = false;
            for (int i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
                if (i == AirflowNetworkLinkageData(count).NodeNums[0]) {
                    NodeFound = true;
                    break;
                }
            }
            if (!NodeFound) {
                if (count <= AirflowNetworkNumOfSurfaces) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + AirflowNetworkLinkageData(count).NodeNames[0] +
                                        " in AIRFLOWNETWORK:MULTIZONE:SURFACE = " + AirflowNetworkLinkageData(count).Name + " is not found");
                } else {
                    ShowSevereError(m_state,
                                    format(RoutineName) + AirflowNetworkLinkageData(count).NodeNames[0] +
                                        " in AIRFLOWNETWORK:DISTRIBUTION:LINKAGE = " + AirflowNetworkLinkageData(count).Name +
                                        " is not found in AIRFLOWNETWORK:DISTRIBUTION:NODE objects.");
                }
                ErrorsFound = true;
            }
            NodeFound = false;
            for (int i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
                if (i == AirflowNetworkLinkageData(count).NodeNums[1]) {
                    NodeFound = true;
                    break;
                }
            }
            if (!NodeFound) {
                if (count <= AirflowNetworkNumOfSurfaces) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + AirflowNetworkLinkageData(count).NodeNames[0] +
                                        " in AIRFLOWNETWORK:MULTIZONE:SURFACE = " + AirflowNetworkLinkageData(count).Name + " is not found");
                } else {
                    ShowSevereError(m_state,
                                    format(RoutineName) + AirflowNetworkLinkageData(count).NodeNames[1] +
                                        " in AIRFLOWNETWORK:DISTRIBUTION:LINKAGE = " + AirflowNetworkLinkageData(count).Name +
                                        " is not found in AIRFLOWNETWORK:DISTRIBUTION:NODE objects.");
                }
                ErrorsFound = true;
            }
            bool CompFound = false;
            for (int i = 1; i <= AirflowNetworkNumOfComps; ++i) {
                if (i == AirflowNetworkLinkageData(count).CompNum) {
                    CompFound = true;
                }
            }
            if (!CompFound) {
                ShowSevereError(m_state,
                                format(RoutineName) + "Component = " + AirflowNetworkLinkageData(count).CompName +
                                    " in AIRFLOWNETWORK:DISTRIBUTION:LINKAGE = " + AirflowNetworkLinkageData(count).Name +
                                    " is not found in AirflowNetwork Component Data objects.");
                ErrorsFound = true;
            }
        }

        // Ensure every AirflowNetworkNode is used in AirflowNetworkLinkage
        for (count = 1; count <= AirflowNetworkNumOfNodes; ++count) {
            bool NodeFound1 = false;
            bool NodeFound2 = false;
            for (int i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
                if (count == AirflowNetworkLinkageData(i).NodeNums[0]) {
                    NodeFound1 = true;
                }
                if (count == AirflowNetworkLinkageData(i).NodeNums[1]) {
                    NodeFound2 = true;
                }
            }
            if ((!NodeFound1) && count > NumOfNodesMultiZone && AirflowNetworkNodeData(count).ExtNodeNum == 0) {
                ShowSevereError(m_state,
                                format(RoutineName) + "AIRFLOWNETWORK:DISTRIBUTION:NODE = " + AirflowNetworkNodeData(count).Name +
                                    " is not found as Node 1 Name in AIRFLOWNETWORK:DISTRIBUTION:LINKAGE");
                ShowContinueError(m_state,
                                  "Each non-external AIRFLOWNETWORK:DISTRIBUTION:NODE has to be defined as Node 1 once in "
                                  "AIRFLOWNETWORK:DISTRIBUTION:LINKAGE");
                ErrorsFound = true;
            }
            if ((!NodeFound2) && count > NumOfNodesMultiZone && AirflowNetworkNodeData(count).ExtNodeNum == 0) {
                ShowSevereError(m_state,
                                format(RoutineName) + "AIRFLOWNETWORK:DISTRIBUTION:NODE = " + AirflowNetworkNodeData(count).Name +
                                    " is not found as Node 2 Name in AIRFLOWNETWORK:DISTRIBUTION:LINKAGE");
                ShowContinueError(m_state,
                                  "Each non-external AIRFLOWNETWORK:DISTRIBUTION:NODE has to be defined as Node 2 once in "
                                  "AIRFLOWNETWORK:DISTRIBUTION:LINKAGE");
                ErrorsFound = true;
            }
            if ((!NodeFound1) && (!NodeFound2) && count > NumOfNodesMultiZone && AirflowNetworkNodeData(count).ExtNodeNum > 0) {
                ShowSevereError(m_state,
                                format(RoutineName) + "AIRFLOWNETWORK:DISTRIBUTION:NODE = " + AirflowNetworkNodeData(count).Name +
                                    " is not found as Node 1 Name or Node 2 Name in AIRFLOWNETWORK:DISTRIBUTION:LINKAGE");
                ShowContinueError(m_state, "This external AIRFLOWNETWORK:DISTRIBUTION:NODE has to be defined in AIRFLOWNETWORK:DISTRIBUTION:LINKAGE");
                ErrorsFound = true;
            }
        }

        // Ensure there is at least one node defined as EXTERNAL node
        NodeFound = false;
        for (count = 1; count <= AirflowNetworkNumOfNodes; ++count) {
            if (AirflowNetworkNodeData(count).ExtNodeNum > 0) {
                NodeFound = true;
            }
        }
        if (!NodeFound) {
            ShowSevereError(m_state,
                            format(RoutineName) +
                                "No External Nodes found in AirflowNetwork:Multizone:ExternalNode. There must be at least 1 external node defined.");
            ErrorsFound = true;
        }

        if (simulation_control.iWPCCnt == iWPCCntr::Input) {
            for (count = 1; count <= AirflowNetworkNumOfSurfaces; ++count) {
                if (AirflowNetworkLinkageData(count).NodeNums[0] == 0) {
                    ShowSevereError(m_state,
                                    "The surface is not found in AIRFLOWNETWORK:MULTIZONE:SURFACE = " + AirflowNetworkLinkageData(count).Name);
                    ErrorsFound = true;
                }
                if (AirflowNetworkLinkageData(count).NodeNums[1] == 0) {
                    ShowSevereError(m_state,
                                    "The external node is not found in AIRFLOWNETWORK:MULTIZONE:SURFACE = " + AirflowNetworkLinkageData(count).Name);
                    ErrorsFound = true;
                }
            }
        }

        // Provide a warning when a door component is assigned as envelope leakage
        //        if (!ErrorsFound) {
        //            for (count = 1; count <= AirflowNetworkNumOfSurfaces; ++count) {
        //                if
        //        (AirflowNetworkNodeData(AirflowNetworkLinkageData(count).NodeNums[0]).ExtNodeNum
        //        > 0 &&
        //                    AirflowNetworkNodeData(AirflowNetworkLinkageData(count).NodeNums[1]).EPlusZoneNum
        //        > 0 && AirflowNetworkLinkageData(count).CompNum > 0) { if
        //        (AirflowNetworkCompData(AirflowNetworkLinkageData(count).CompNum).CompTypeNum
        //        == iComponentTypeNum::SOP) {
        //                    }
        //                }
        //                if
        //        (AirflowNetworkNodeData(AirflowNetworkLinkageData(count).NodeNums[1]).ExtNodeNum
        //        > 0 &&
        //                    AirflowNetworkNodeData(AirflowNetworkLinkageData(count).NodeNums[0]).EPlusZoneNum
        //        > 0 && AirflowNetworkLinkageData(count).CompNum > 0) { if
        //        (AirflowNetworkCompData(AirflowNetworkLinkageData(count).CompNum).CompTypeNum
        //        == iComponentTypeNum::SOP) {
        //                    }
        //                }
        //            }
        //        }

        // Ensure the name of each heat exchanger is shown either once or twice in the field of
        if (distribution_simulated) {
            for (int i = 1; i <= DisSysNumOfHXs; ++i) {
                count = 0;
                for (j = 1; j <= AirflowNetworkNumOfLinks; ++j) {
                    if (Util::SameString(AirflowNetworkLinkageData(j).CompName, DisSysCompHXData(i).name)) {
                        ++count;
                    }
                }

                if (DisSysCompHXData(i).CoilParentExists && count != 2) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + "The inputs of component name field as a heat exchanger in "
                                                          "AIRFLOWNETWORK:DISTRIBUTION:LINKAGE is not correct");
                    ShowContinueError(m_state,
                                      "The entered name of heat exchanger is " + DisSysCompHXData(i).name +
                                          " in AirflowNetwork:Distribution:Component:HeatExchanger objects");
                    ShowContinueError(m_state, format("The correct appearance number is 2. The entered appearance number is {}", count));
                    ErrorsFound = true;
                }
                if ((!DisSysCompHXData(i).CoilParentExists) && count != 1) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + "The inputs of component name field as a heat exchanger in "
                                                          "AIRFLOWNETWORK:DISTRIBUTION:LINKAGE is not correct");
                    ShowContinueError(m_state,
                                      "The entered name of heat exchanger is " + DisSysCompHXData(i).name +
                                          " in AirflowNetwork:Distribution:Component:HeatExchanger objects");
                    ShowContinueError(m_state, format("The correct appearance number is 1. The entered appearance number is {}", count));
                    ErrorsFound = true;
                }
            }
        }

        // Check node assignments using AirflowNetwork:Distribution:Component:OutdoorAirFlow or
        // AirflowNetwork:Distribution:Component:ReliefAirFlow
        for (count = AirflowNetworkNumOfSurfaces + 1; count <= AirflowNetworkNumOfLinks; ++count) {
            int i = AirflowNetworkLinkageData(count).CompNum;
            j = AirflowNetworkLinkageData(count).NodeNums[0];
            k = AirflowNetworkLinkageData(count).NodeNums[1];

            if (AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::OAF) {
                if (!Util::SameString(DisSysNodeData(j - NumOfNodesMultiZone).EPlusType, "OAMixerOutdoorAirStreamNode")) {
                    ShowSevereError(m_state,
                                    format(RoutineName) +
                                        "AirflowNetwork:Distribution:Linkage: When the component type is "
                                        "AirflowNetwork:Distribution:Component:OutdoorAirFlow at " +
                                        AirflowNetworkNodeData(j).Name + ",");
                    ShowContinueError(
                        m_state, "the component type in the first node should be OAMixerOutdoorAirStreamNode at " + AirflowNetworkNodeData(j).Name);
                    ErrorsFound = true;
                }
                if (!Util::SameString(DisSysNodeData(k - NumOfNodesMultiZone).EPlusType, "AirLoopHVAC:OutdoorAirSystem")) {
                    ShowSevereError(m_state,
                                    format(RoutineName) +
                                        "AirflowNetwork:Distribution:Linkage: When the component type is "
                                        "AirflowNetwork:Distribution:Component:OutdoorAirFlow at " +
                                        AirflowNetworkNodeData(k).Name + ",");
                    ShowContinueError(m_state,
                                      "the component object type in the second node should be AirLoopHVAC:OutdoorAirSystem at " +
                                          AirflowNetworkNodeData(k).Name);
                    ErrorsFound = true;
                }
            }

            if (AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::REL) {
                if (!Util::SameString(DisSysNodeData(j - NumOfNodesMultiZone).EPlusType, "AirLoopHVAC:OutdoorAirSystem")) {
                    ShowSevereError(m_state,
                                    format(RoutineName) +
                                        "AirflowNetwork:Distribution:Linkage: When the component type is "
                                        "AirflowNetwork:Distribution:Component:OutdoorAirFlow at " +
                                        AirflowNetworkNodeData(j).Name + ",");
                    ShowContinueError(m_state,
                                      "the component object type in the first node should be AirLoopHVAC:OutdoorAirSystem at " +
                                          AirflowNetworkNodeData(j).Name);
                    ErrorsFound = true;
                }
                if (!Util::SameString(DisSysNodeData(k - NumOfNodesMultiZone).EPlusType, "OAMixerOutdoorAirStreamNode")) {
                    ShowSevereError(m_state,
                                    format(RoutineName) +
                                        "AirflowNetwork:Distribution:Linkage: When the component type is "
                                        "AirflowNetwork:Distribution:Component:OutdoorAirFlow at " +
                                        AirflowNetworkNodeData(k).Name + ",");
                    ShowContinueError(
                        m_state, "the component type in the second node should be OAMixerOutdoorAirStreamNode at " + AirflowNetworkNodeData(k).Name);
                    ErrorsFound = true;
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(m_state, format("{}Errors found getting inputs. Previous error(s) cause program termination.", RoutineName));
        }

        Alphas.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        Numbers.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (!ErrorsFound) {
            allocate_and_initialize();
        }
    }

    void Solver::initialize()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Aug. 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes variables of additional zone loads caused by ADS.

        // USE STATEMENTS:
        auto &TimeStepSys = m_state.dataHVACGlobal->TimeStepSys;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int ZoneNum;
        auto &Zone(m_state.dataHeatBal->Zone);

        if (initializeOneTimeFlag) {
            exchangeData.allocate(m_state.dataGlobal->NumOfZones); // AirflowNetwork exchange data due to air-forced system
            for (i = 1; i <= DisSysNumOfCVFs; i++) {
                if (DisSysCompCVFData(i).fanType == HVAC::FanType::OnOff) {
                    multiExchangeData.allocate(m_state.dataGlobal->NumOfZones);
                    break;
                }
            }

            initializeOneTimeFlag = false;
            if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                for (i = 1; i <= m_state.dataGlobal->NumOfZones; ++i) {
                    SetupOutputVariable(m_state,
                                        "AFN Zone Outdoor Air Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        exchangeData(i).SumMHr,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        Zone(i).Name);
                    SetupOutputVariable(m_state,
                                        "AFN Zone Mixing Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        exchangeData(i).SumMMHr,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        Zone(i).Name);
                    SetupOutputVariable(m_state,
                                        "AFN Zone Outdoor Air CO2 Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        exchangeData(i).SumMHrCO,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        Zone(i).Name);
                    SetupOutputVariable(m_state,
                                        "AFN Zone Mixing CO2 Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        exchangeData(i).SumMMHrCO,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        Zone(i).Name);
                    SetupOutputVariable(m_state,
                                        "AFN Zone Total CO2 Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        exchangeData(i).TotalCO2,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        Zone(i).Name);
                }
            }
            if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                for (i = 1; i <= m_state.dataGlobal->NumOfZones; ++i) {
                    if (!m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        SetupOutputVariable(m_state,
                                            "AFN Zone Outdoor Air Mass Flow Rate",
                                            Constant::Units::kg_s,
                                            exchangeData(i).SumMHr,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            Zone(i).Name);
                        SetupOutputVariable(m_state,
                                            "AFN Zone Mixing Mass Flow Rate",
                                            Constant::Units::kg_s,
                                            exchangeData(i).SumMMHr,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            Zone(i).Name);
                    }
                    SetupOutputVariable(m_state,
                                        "AFN Zone Outdoor Air Generic Air Contaminant Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        exchangeData(i).SumMHrGC,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        Zone(i).Name);
                    SetupOutputVariable(m_state,
                                        "AFN Zone Mixing Generic Air Contaminant Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        exchangeData(i).SumMMHrGC,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        Zone(i).Name);
                    SetupOutputVariable(m_state,
                                        "AFN Zone Total Generic Air Contaminant Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        exchangeData(i).TotalGC,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        Zone(i).Name);
                }
            }
        }

        if (m_state.dataGlobal->BeginEnvrnFlag && initializeMyEnvrnFlag) {
            // Assign node values
            for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
                AirflowNetworkNodeSimu(i).TZ = 23.0;
                AirflowNetworkNodeSimu(i).WZ = 0.00084;
                AirflowNetworkNodeSimu(i).PZ = 0.0;
                AirflowNetworkNodeSimu(i).TZlast = AirflowNetworkNodeSimu(i).TZ;
                AirflowNetworkNodeSimu(i).WZlast = AirflowNetworkNodeSimu(i).WZ;
                if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                    AirflowNetworkNodeSimu(i).CO2Z = m_state.dataContaminantBalance->OutdoorCO2;
                    AirflowNetworkNodeSimu(i).CO2Zlast = AirflowNetworkNodeSimu(i).CO2Z;
                }
                if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                    AirflowNetworkNodeSimu(i).GCZ = m_state.dataContaminantBalance->OutdoorGC;
                    AirflowNetworkNodeSimu(i).GCZlast = AirflowNetworkNodeSimu(i).GCZ;
                }
                if (AirflowNetworkNodeData(i).RAFNNodeNum > 0) {
                    ZoneNum = AirflowNetworkNodeData(i).EPlusZoneNum;
                    m_state.dataRoomAir->AFNZoneInfo(ZoneNum).Node(AirflowNetworkNodeData(i).RAFNNodeNum).AirTemp = 23.0;
                    m_state.dataRoomAir->AFNZoneInfo(ZoneNum).Node(AirflowNetworkNodeData(i).RAFNNodeNum).HumRat = 0.0;
                }
            }

            for (i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
                AirflowNetworkLinkSimu(i).FLOW = 0.0;
                AirflowNetworkLinkSimu(i).FLOW2 = 0.0;
            }

            for (i = 1; i <= m_state.dataGlobal->NumOfZones; ++i) {
                ANZT(i) = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(i).MAT;
                ANZW(i) = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(i).airHumRat;
                if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                    ANCO(i) = m_state.dataContaminantBalance->ZoneAirCO2(i);
                }
                if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                    ANGC(i) = m_state.dataContaminantBalance->ZoneAirGC(i);
                }
            }
            if (AirflowNetworkNumOfOccuVentCtrls > 0) {
                for (i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
                    if (MultizoneSurfaceData(i).OccupantVentilationControlNum > 0) {
                        MultizoneSurfaceData(i).PrevOpeningstatus = AirflowNetwork::OpenStatus::FreeOperation;
                        MultizoneSurfaceData(i).CloseElapsedTime = 0.0;
                        MultizoneSurfaceData(i).OpenElapsedTime = 0.0;
                        MultizoneSurfaceData(i).OpeningStatus = AirflowNetwork::OpenStatus::FreeOperation;
                        MultizoneSurfaceData(i).OpeningProbStatus = AirflowNetwork::ProbabilityCheck::NoAction;
                        MultizoneSurfaceData(i).ClosingProbStatus = 0;
                    }
                }
            }

            initializeMyEnvrnFlag = false;
        }
        if (!m_state.dataGlobal->BeginEnvrnFlag) {
            initializeMyEnvrnFlag = true;
            if (simulation_control.type != ControlType::NoMultizoneOrDistribution) {
                if (RollBackFlag) {
                    for (i = 1; i <= m_state.dataGlobal->NumOfZones; ++i) {
                        ANZT(i) = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(i).XMAT[0];
                        ANZW(i) = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(i).WPrevZoneTS[0];
                        if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                            ANCO(i) = m_state.dataContaminantBalance->CO2ZoneTimeMinus1(i);
                        }
                        if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                            ANGC(i) = m_state.dataContaminantBalance->GCZoneTimeMinus1(i);
                        }
                    }
                } else {
                    for (i = 1; i <= m_state.dataGlobal->NumOfZones; ++i) {
                        ANZT(i) = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(i).MAT;
                        ANZW(i) = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(i).airHumRat;
                        if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                            ANCO(i) = m_state.dataContaminantBalance->ZoneAirCO2(i);
                        }
                        if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                            ANGC(i) = m_state.dataContaminantBalance->ZoneAirGC(i);
                        }
                    }
                }

                for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
                    if (AirflowNetworkNodeData(i).EPlusZoneNum > 0) {
                        AirflowNetworkNodeSimu(i).TZ = ANZT(AirflowNetworkNodeData(i).EPlusZoneNum);
                        AirflowNetworkNodeSimu(i).WZ = ANZW(AirflowNetworkNodeData(i).EPlusZoneNum);
                        if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                            AirflowNetworkNodeSimu(i).CO2Z = ANCO(AirflowNetworkNodeData(i).EPlusZoneNum);
                        }
                        if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                            AirflowNetworkNodeSimu(i).GCZ = ANGC(AirflowNetworkNodeData(i).EPlusZoneNum);
                        }
                    }
                    if (AirflowNetworkNodeData(i).ExtNodeNum > 0) {
                        if (AirflowNetworkNodeData(i).OutAirNodeNum > 0 &&
                            m_state.dataLoopNodes->Node(AirflowNetworkNodeData(i).OutAirNodeNum).IsLocalNode) {
                            AirflowNetworkNodeSimu(i).TZ = m_state.dataLoopNodes->Node(AirflowNetworkNodeData(i).OutAirNodeNum).OutAirDryBulb;
                            AirflowNetworkNodeSimu(i).WZ = m_state.dataLoopNodes->Node(AirflowNetworkNodeData(i).OutAirNodeNum).HumRat;
                        } else {
                            AirflowNetworkNodeSimu(i).TZ = AirflowNetwork::OutDryBulbTempAt(m_state, AirflowNetworkNodeData(i).NodeHeight);
                            AirflowNetworkNodeSimu(i).WZ = m_state.dataEnvrn->OutHumRat;
                        }

                        if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                            AirflowNetworkNodeSimu(i).CO2Z = m_state.dataContaminantBalance->OutdoorCO2;
                        }
                        if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                            AirflowNetworkNodeSimu(i).GCZ = m_state.dataContaminantBalance->OutdoorGC;
                        }
                    }

                    if (AirflowNetworkNodeData(i).RAFNNodeNum > 0) {
                        ZoneNum = AirflowNetworkNodeData(i).EPlusZoneNum;
                        if (m_state.dataRoomAir->AFNZoneInfo(ZoneNum).Node(AirflowNetworkNodeData(i).RAFNNodeNum).AFNNodeID == i) {
                            AirflowNetworkNodeSimu(i).TZ =
                                m_state.dataRoomAir->AFNZoneInfo(ZoneNum).Node(AirflowNetworkNodeData(i).RAFNNodeNum).AirTemp;
                            AirflowNetworkNodeSimu(i).WZ =
                                m_state.dataRoomAir->AFNZoneInfo(ZoneNum).Node(AirflowNetworkNodeData(i).RAFNNodeNum).HumRat;
                        }
                    }
                }
            }
        }

        for (auto &e : exchangeData) {
            e.TotalSen = 0.0;
            e.TotalLat = 0.0;
            e.MultiZoneSen = 0.0;
            e.MultiZoneLat = 0.0;
            e.LeakSen = 0.0;
            e.LeakLat = 0.0;
            e.CondSen = 0.0;
            e.DiffLat = 0.0;
            e.RadGain = 0.0;
        }
        if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
            for (auto &e : exchangeData) {
                e.TotalCO2 = 0.0;
            }
        }
        if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            for (auto &e : exchangeData) {
                e.TotalGC = 0.0;
            }
        }

        // Occupant ventilation control
        Real64 CurrentEndTime = m_state.dataGlobal->CurrentTime + m_state.dataHVACGlobal->SysTimeElapsed;
        if (CurrentEndTime > CurrentEndTimeLast && TimeStepSys >= TimeStepSysLast) {
            for (i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
                if (i > AirflowNetworkNumOfSurfaces - NumOfLinksIntraZone) {
                    continue;
                }
                if (MultizoneSurfaceData(i).OccupantVentilationControlNum > 0) {
                    MultizoneSurfaceData(i).PrevOpeningstatus = MultizoneSurfaceData(i).OpeningStatus;
                    MultizoneSurfaceData(i).OpenFactorLast = MultizoneSurfaceData(i).OpenFactor;
                    if (MultizoneSurfaceData(i).OpenFactor > 0.0) {
                        MultizoneSurfaceData(i).OpenElapsedTime += (CurrentEndTime - CurrentEndTimeLast) * 60.0;
                        MultizoneSurfaceData(i).CloseElapsedTime = 0.0;
                    } else {
                        MultizoneSurfaceData(i).OpenElapsedTime = 0.0;
                        MultizoneSurfaceData(i).CloseElapsedTime += (CurrentEndTime - CurrentEndTimeLast) * 60.0;
                    }
                    int j = MultizoneSurfaceData(i).SurfNum;
                    OccupantVentilationControl(MultizoneSurfaceData(i).OccupantVentilationControlNum)
                        .calc(m_state,
                              m_state.dataSurface->Surface(j).Zone,
                              MultizoneSurfaceData(i).OpenElapsedTime,
                              MultizoneSurfaceData(i).CloseElapsedTime,
                              MultizoneSurfaceData(i).OpeningStatus,
                              MultizoneSurfaceData(i).OpeningProbStatus,
                              MultizoneSurfaceData(i).ClosingProbStatus);
                    if (MultizoneSurfaceData(i).OpeningStatus == AirflowNetwork::OpenStatus::MinCheckForceOpen) {
                        MultizoneSurfaceData(i).OpenFactor = MultizoneSurfaceData(i).OpenFactorLast;
                    }
                    if (MultizoneSurfaceData(i).OpeningStatus == AirflowNetwork::OpenStatus::MinCheckForceClose) {
                        MultizoneSurfaceData(i).OpenFactor = 0.0;
                    }
                }
            }
        }
        TimeStepSysLast = TimeStepSys;
        CurrentEndTimeLast = CurrentEndTime;
    }

    void Solver::allocate_and_initialize()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Aug. 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes variables and allocates dynamic arrays.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int SurfNum;
        auto &Zone(m_state.dataHeatBal->Zone);

        AirflowNetworkNodeSimu.allocate(AirflowNetworkNumOfNodes); // Node simulation variable in air distribution system
        AirflowNetworkLinkSimu.allocate(AirflowNetworkNumOfLinks); // Link simulation variable in air distribution system
        linkReport.allocate(AirflowNetworkNumOfLinks);             // Report link simulation variable in air distribution system

        for (i = 1; i <= DisSysNumOfCVFs; i++) {
            if (DisSysCompCVFData(i).fanType == HVAC::FanType::OnOff) {
                nodeReport.allocate(AirflowNetworkNumOfZones);
                linkReport1.allocate(AirflowNetworkNumOfSurfaces);
                break;
            }
        }

        MA.allocate(AirflowNetworkNumOfNodes * AirflowNetworkNumOfNodes);
        MV.allocate(AirflowNetworkNumOfNodes);
        IVEC.allocate(AirflowNetworkNumOfNodes + 20);

        AirflowNetworkReportData.allocate(m_state.dataGlobal->NumOfZones); // Report variables
        AirflowNetworkZnRpt.allocate(m_state.dataGlobal->NumOfZones);      // Report variables

        ANZT.allocate(m_state.dataGlobal->NumOfZones); // Local zone air temperature for rollback use
        ANZW.allocate(m_state.dataGlobal->NumOfZones); // Local zone humidity ratio for rollback use
        if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
            ANCO.allocate(m_state.dataGlobal->NumOfZones); // Local zone CO2 for rollback use
        }
        if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            ANGC.allocate(m_state.dataGlobal->NumOfZones); // Local zone generic contaminant for rollback use
        }
        allocate();

        bool OnOffFanFlag = false;
        for (i = 1; i <= DisSysNumOfCVFs; i++) {
            if (DisSysCompCVFData(i).fanType == HVAC::FanType::OnOff) {
                OnOffFanFlag = true;
            }
        }

        // CurrentModuleObject='AirflowNetwork Simulations'
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            SetupOutputVariable(m_state,
                                "AFN Node Temperature",
                                Constant::Units::C,
                                AirflowNetworkNodeSimu(i).TZ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                AirflowNetworkNodeData(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Node Humidity Ratio",
                                Constant::Units::kgWater_kgDryAir,
                                AirflowNetworkNodeSimu(i).WZ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                AirflowNetworkNodeData(i).Name);
            if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                SetupOutputVariable(m_state,
                                    "AFN Node CO2 Concentration",
                                    Constant::Units::ppm,
                                    AirflowNetworkNodeSimu(i).CO2Z,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    AirflowNetworkNodeData(i).Name);
            }
            if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                SetupOutputVariable(m_state,
                                    "AFN Node Generic Air Contaminant Concentration",
                                    Constant::Units::ppm,
                                    AirflowNetworkNodeSimu(i).GCZ,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    AirflowNetworkNodeData(i).Name);
            }
            if (!(supplyFanType == HVAC::FanType::OnOff && i <= AirflowNetworkNumOfZones)) {
                SetupOutputVariable(m_state,
                                    "AFN Node Total Pressure",
                                    Constant::Units::Pa,
                                    AirflowNetworkNodeSimu(i).PZ,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    AirflowNetworkNodeData(i).Name);
            }
            if (AirflowNetworkNodeData(i).ExtNodeNum > 0) {
                SetupOutputVariable(m_state,
                                    "AFN Node Wind Pressure",
                                    Constant::Units::Pa,
                                    AirflowNetworkNodeSimu(i).PZ,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    AirflowNetworkNodeData(i).Name);
            }
        }

        for (i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
            if (!(supplyFanType == HVAC::FanType::OnOff && i <= AirflowNetworkNumOfSurfaces)) {
                SetupOutputVariable(m_state,
                                    "AFN Linkage Node 1 to Node 2 Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    linkReport(i).FLOW,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    AirflowNetworkLinkageData(i).Name);
                SetupOutputVariable(m_state,
                                    "AFN Linkage Node 2 to Node 1 Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    linkReport(i).FLOW2,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    AirflowNetworkLinkageData(i).Name);
                SetupOutputVariable(m_state,
                                    "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    linkReport(i).VolFLOW,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    AirflowNetworkLinkageData(i).Name);
                SetupOutputVariable(m_state,
                                    "AFN Linkage Node 2 to Node 1 Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    linkReport(i).VolFLOW2,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    AirflowNetworkLinkageData(i).Name);
                SetupOutputVariable(m_state,
                                    "AFN Linkage Node 1 to Node 2 Pressure Difference",
                                    Constant::Units::Pa,
                                    AirflowNetworkLinkSimu(i).DP,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    AirflowNetworkLinkageData(i).Name);
            }
        }

        for (i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
            if (AirflowNetworkLinkageData(i).element == nullptr) {
                // This is not great
                continue;
            }
            if (AirflowNetworkLinkageData(i).element->type() == ComponentType::DOP ||
                AirflowNetworkLinkageData(i).element->type() == ComponentType::SOP ||
                AirflowNetworkLinkageData(i).element->type() == ComponentType::HOP) {
                SurfNum = MultizoneSurfaceData(i).SurfNum;
                SetupOutputVariable(m_state,
                                    "AFN Surface Venting Window or Door Opening Factor",
                                    Constant::Units::None,
                                    MultizoneSurfaceData(i).OpenFactor,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    MultizoneSurfaceData(i).SurfName);
                if (m_state.dataGlobal->AnyEnergyManagementSystemInModel) {
                    SetupEMSActuator(m_state,
                                     "AirFlow Network Window/Door Opening",
                                     MultizoneSurfaceData(i).SurfName,
                                     "Venting Opening Factor",
                                     "[Fraction]",
                                     MultizoneSurfaceData(i).EMSOpenFactorActuated,
                                     MultizoneSurfaceData(i).EMSOpenFactor);
                }
                SetupOutputVariable(m_state,
                                    "AFN Surface Venting Window or Door Opening Modulation Multiplier",
                                    Constant::Units::None,
                                    m_state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum),
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    m_state.dataSurface->Surface(SurfNum).Name);
                SetupOutputVariable(m_state,
                                    "AFN Surface Venting Inside Setpoint Temperature",
                                    Constant::Units::C,
                                    m_state.dataSurface->SurfWinInsideTempForVentingRep(SurfNum),
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    m_state.dataSurface->Surface(SurfNum).Name);
                SetupOutputVariable(m_state,
                                    "AFN Surface Venting Availability Status",
                                    Constant::Units::None,
                                    m_state.dataSurface->SurfWinVentingAvailabilityRep(SurfNum),
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    m_state.dataSurface->Surface(SurfNum).Name);
                if (MultizoneSurfaceData(i).OccupantVentilationControlNum > 0) {
                    SetupOutputVariable(m_state,
                                        "AFN Surface Venting Window or Door Opening Factor at Previous Time Step",
                                        Constant::Units::None,
                                        MultizoneSurfaceData(i).OpenFactorLast,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        MultizoneSurfaceData(i).SurfName);
                    SetupOutputVariable(m_state,
                                        "AFN Surface Opening Elapsed Time",
                                        Constant::Units::min,
                                        MultizoneSurfaceData(i).OpenElapsedTime,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        MultizoneSurfaceData(i).SurfName);
                    SetupOutputVariable(m_state,
                                        "AFN Surface Closing Elapsed Time",
                                        Constant::Units::min,
                                        MultizoneSurfaceData(i).CloseElapsedTime,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        MultizoneSurfaceData(i).SurfName);
                    SetupOutputVariable(m_state,
                                        "AFN Surface Opening Status at Previous Time Step",
                                        Constant::Units::None,
                                        MultizoneSurfaceData(i).PrevOpeningstatus,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        MultizoneSurfaceData(i).SurfName);
                    SetupOutputVariable(m_state,
                                        "AFN Surface Opening Status",
                                        Constant::Units::None,
                                        MultizoneSurfaceData(i).OpeningStatus,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        MultizoneSurfaceData(i).SurfName);
                    SetupOutputVariable(m_state,
                                        "AFN Surface Opening Probability Status",
                                        Constant::Units::None,
                                        MultizoneSurfaceData(i).OpeningProbStatus,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        MultizoneSurfaceData(i).SurfName);
                    SetupOutputVariable(m_state,
                                        "AFN Surface Closing Probability Status",
                                        Constant::Units::None,
                                        MultizoneSurfaceData(i).ClosingProbStatus,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        MultizoneSurfaceData(i).SurfName);
                }
            }
        }

        for (i = 1; i <= m_state.dataGlobal->NumOfZones; ++i) {
            // Multizone losses due to force air systems
            SetupOutputVariable(m_state,
                                "AFN Zone Infiltration Sensible Heat Gain Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).MultiZoneInfiSenGainW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Infiltration Sensible Heat Gain Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).MultiZoneInfiSenGainJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Ventilation Sensible Heat Gain Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).MultiZoneVentSenGainW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Ventilation Sensible Heat Gain Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).MultiZoneVentSenGainJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Mixing Sensible Heat Gain Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).MultiZoneMixSenGainW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Mixing Sensible Heat Gain Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).MultiZoneMixSenGainJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Infiltration Sensible Heat Loss Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).MultiZoneInfiSenLossW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Infiltration Sensible Heat Loss Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).MultiZoneInfiSenLossJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Ventilation Sensible Heat Loss Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).MultiZoneVentSenLossW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Ventilation Sensible Heat Loss Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).MultiZoneVentSenLossJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Mixing Sensible Heat Loss Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).MultiZoneMixSenLossW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Mixing Sensible Heat Loss Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).MultiZoneMixSenLossJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Infiltration Latent Heat Gain Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).MultiZoneInfiLatGainW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Infiltration Latent Heat Gain Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).MultiZoneInfiLatGainJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Infiltration Latent Heat Loss Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).MultiZoneInfiLatLossW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Infiltration Latent Heat Loss Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).MultiZoneInfiLatLossJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Ventilation Latent Heat Gain Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).MultiZoneVentLatGainW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Ventilation Latent Heat Gain Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).MultiZoneVentLatGainJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Ventilation Latent Heat Loss Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).MultiZoneVentLatLossW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Ventilation Latent Heat Loss Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).MultiZoneVentLatLossJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Mixing Latent Heat Gain Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).MultiZoneMixLatGainW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Mixing Latent Heat Gain Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).MultiZoneMixLatGainJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Mixing Latent Heat Loss Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).MultiZoneMixLatLossW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Mixing Latent Heat Loss Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).MultiZoneInfiLatLossJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            // Supply leak losses due to force air systems
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Leaked Air Sensible Heat Gain Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).LeakSenGainW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Leaked Air Sensible Heat Gain Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).LeakSenGainJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Leaked Air Sensible Heat Loss Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).LeakSenLossW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Leaked Air Sensible Heat Loss Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).LeakSenLossJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Leaked Air Latent Heat Gain Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).LeakLatGainW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Leaked Air Latent Heat Gain Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).LeakLatGainJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Leaked Air Latent Heat Loss Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).LeakLatLossW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Leaked Air Latent Heat Loss Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).LeakLatLossJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            // Conduction losses due to force air systems
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Conduction Sensible Heat Gain Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).CondSenGainW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Conduction Sensible Heat Gain Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).CondSenGainJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Conduction Sensible Heat Loss Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).CondSenLossW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Conduction Sensible Heat Loss Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).CondSenLossJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Diffusion Latent Heat Gain Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).DiffLatGainW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Diffusion Latent Heat Gain Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).DiffLatGainJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Diffusion Latent Heat Loss Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).DiffLatLossW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Diffusion Latent Heat Loss Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).DiffLatLossJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            // Radiation losses due to forced air systems
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Radiation Heat Gain Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).RadGainW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Radiation Sensible Heat Gain Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).RadGainJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Radiation Heat Loss Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).RadLossW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Duct Radiation Sensible Heat Loss Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).RadLossJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            // Total losses due to force air systems
            SetupOutputVariable(m_state,
                                "AFN Distribution Sensible Heat Gain Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).TotalSenGainW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Distribution Sensible Heat Gain Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).TotalSenGainJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Distribution Sensible Heat Loss Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).TotalSenLossW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Distribution Sensible Heat Loss Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).TotalSenLossJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Distribution Latent Heat Gain Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).TotalLatGainW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Distribution Latent Heat Gain Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).TotalLatGainJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Distribution Latent Heat Loss Rate",
                                Constant::Units::W,
                                AirflowNetworkReportData(i).TotalLatLossW,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Distribution Latent Heat Loss Energy",
                                Constant::Units::J,
                                AirflowNetworkReportData(i).TotalLatLossJ,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
        }

        for (i = 1; i <= m_state.dataGlobal->NumOfZones; ++i) {
            SetupOutputVariable(m_state,
                                "AFN Zone Infiltration Volume",
                                Constant::Units::m3,
                                AirflowNetworkZnRpt(i).InfilVolume,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Infiltration Mass",
                                Constant::Units::kg,
                                AirflowNetworkZnRpt(i).InfilMass,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Infiltration Air Change Rate",
                                Constant::Units::ach,
                                AirflowNetworkZnRpt(i).InfilAirChangeRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Ventilation Volume",
                                Constant::Units::m3,
                                AirflowNetworkZnRpt(i).VentilVolume,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Ventilation Mass",
                                Constant::Units::kg,
                                AirflowNetworkZnRpt(i).VentilMass,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Ventilation Air Change Rate",
                                Constant::Units::ach,
                                AirflowNetworkZnRpt(i).VentilAirChangeRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Mixing Volume",
                                Constant::Units::m3,
                                AirflowNetworkZnRpt(i).MixVolume,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Mixing Mass",
                                Constant::Units::kg,
                                AirflowNetworkZnRpt(i).MixMass,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(i).Name);

            SetupOutputVariable(m_state,
                                "AFN Zone Exfiltration Heat Transfer Rate",
                                Constant::Units::W,
                                AirflowNetworkZnRpt(i).ExfilTotalLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Exfiltration Sensible Heat Transfer Rate",
                                Constant::Units::W,
                                AirflowNetworkZnRpt(i).ExfilSensiLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(m_state,
                                "AFN Zone Exfiltration Latent Heat Transfer Rate",
                                Constant::Units::W,
                                AirflowNetworkZnRpt(i).ExfilLatentLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(i).Name);
        }

        if (OnOffFanFlag) {
            for (i = 1; i <= AirflowNetworkNumOfZones; ++i) {
                SetupOutputVariable(m_state,
                                    "AFN Zone Average Pressure",
                                    Constant::Units::Pa,
                                    nodeReport(i).PZ,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    Zone(i).Name);
                SetupOutputVariable(m_state,
                                    "AFN Zone On Cycle Pressure",
                                    Constant::Units::Pa,
                                    nodeReport(i).PZON,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    Zone(i).Name);
                SetupOutputVariable(m_state,
                                    "AFN Zone Off Cycle Pressure",
                                    Constant::Units::Pa,
                                    nodeReport(i).PZOFF,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    Zone(i).Name);
            }
            for (i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
                SetupOutputVariable(m_state,
                                    "AFN Linkage Node 1 to 2 Average Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    linkReport1(i).FLOW,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    MultizoneSurfaceData(i).SurfName);
                SetupOutputVariable(m_state,
                                    "AFN Linkage Node 2 to 1 Average Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    linkReport1(i).FLOW2,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    MultizoneSurfaceData(i).SurfName);
                SetupOutputVariable(m_state,
                                    "AFN Linkage Node 1 to 2 Average Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    linkReport1(i).VolFLOW,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    MultizoneSurfaceData(i).SurfName);
                SetupOutputVariable(m_state,
                                    "AFN Linkage Node 2 to 1 Average Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    linkReport1(i).VolFLOW2,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    MultizoneSurfaceData(i).SurfName);
                SetupOutputVariable(m_state,
                                    "AFN Surface Average Pressure Difference",
                                    Constant::Units::Pa,
                                    linkReport1(i).DP,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    MultizoneSurfaceData(i).SurfName);
                SetupOutputVariable(m_state,
                                    "AFN Surface On Cycle Pressure Difference",
                                    Constant::Units::Pa,
                                    linkReport1(i).DPON,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    MultizoneSurfaceData(i).SurfName);
                SetupOutputVariable(m_state,
                                    "AFN Surface Off Cycle Pressure Difference",
                                    Constant::Units::Pa,
                                    linkReport1(i).DPOFF,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    MultizoneSurfaceData(i).SurfName);
            }
        }

        // Assign node reference height
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            if (!simulation_control.temperature_height_dependence) {
                AirflowNetworkNodeData(i).NodeHeight = 0.0;
            }
            int ZoneNum = AirflowNetworkNodeData(i).EPlusZoneNum;
            if (ZoneNum > 0) {
                if (m_state.dataSurface->WorldCoordSystem) {
                    AirflowNetworkNodeData(i).NodeHeight = 0.0;
                } else {
                    AirflowNetworkNodeData(i).NodeHeight = Zone(ZoneNum).OriginZ;
                }
            }
        }
    }

    void Solver::calculate_balance()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs simulations of nodal pressures and linkage airflows.

        // Using/Aliasing
        using General::SolveRoot;
        using HVAC::VerySmallMassFlow;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int j;
        int n;
        int NodeNum;
        Real64 GlobalOpenFactor;
        Real64 ZonePressure1;
        Real64 ZonePressure2;
        Real64 PressureSet;
        Real64 LocalAzimuth;
        Real64 LocalWindSpeed;
        Real64 LocalWindDir;
        Real64 LocalHumRat;
        Real64 LocalDryBulb;
        Array1D<Real64> Par; // Pressure setpoint
        Real64 constexpr ErrorToler(0.00001);
        int constexpr MaxIte(20);
        int SolFla;
        Real64 MinExhaustMassFlowrate;
        Real64 MaxExhaustMassFlowrate;
        Real64 MinReliefMassFlowrate;
        Real64 MaxReliefMassFlowrate;
        int AirLoopNum;

        auto &Node(m_state.dataLoopNodes->Node);

        // Validate supply and return connections
        if (CalcAirflowNetworkAirBalanceOneTimeFlag) {
            CalcAirflowNetworkAirBalanceOneTimeFlag = false;
            if (CalcAirflowNetworkAirBalanceErrorsFound) {
                ShowFatalError(m_state, "GetAirflowNetworkInput: Program terminates for preceding reason(s).");
            }
        }

        for (n = 1; n <= ActualNumOfNodes; ++n) {
            if (AirflowNetworkNodeData(n).NodeTypeNum == 0) {
                AirflowNetworkNodeSimu(n).PZ = 0.0;
            } else {
                // Assigning ambient conditions to external nodes
                i = AirflowNetworkNodeData(n).ExtNodeNum;
                if (i > 0) {
                    AirflowNetworkNodeSimu(n).TZ = OutDryBulbTempAt(m_state, AirflowNetworkNodeData(n).NodeHeight);
                    AirflowNetworkNodeSimu(n).WZ = m_state.dataEnvrn->OutHumRat;
                    if (i <= AirflowNetworkNumOfExtNode) {
                        if (MultizoneExternalNodeData(i).OutAirNodeNum == 0) {
                            LocalWindSpeed = DataEnvironment::WindSpeedAt(m_state, MultizoneExternalNodeData(i).height);
                            LocalDryBulb = OutDryBulbTempAt(m_state, AirflowNetworkNodeData(n).NodeHeight);
                            LocalAzimuth = MultizoneExternalNodeData(i).azimuth;
                            AirflowNetworkNodeSimu(n).PZ = calculate_wind_pressure(MultizoneExternalNodeData(i).curve,
                                                                                   MultizoneExternalNodeData(i).symmetricCurve,
                                                                                   MultizoneExternalNodeData(i).useRelativeAngle,
                                                                                   LocalAzimuth,
                                                                                   LocalWindSpeed,
                                                                                   m_state.dataEnvrn->WindDir,
                                                                                   LocalDryBulb,
                                                                                   m_state.dataEnvrn->OutHumRat);
                        } else {
                            // If and outdoor air node object is defined as the External Node Name in AirflowNetwork:MultiZone:Surface,
                            // the node object requires to define the Wind Pressure Coefficient Curve Name.
                            NodeNum = MultizoneExternalNodeData(i).OutAirNodeNum;
                            LocalWindSpeed = Node((NodeNum)).OutAirWindSpeed;
                            LocalWindDir = Node((NodeNum)).OutAirWindDir;
                            LocalHumRat = Node((NodeNum)).HumRat;
                            LocalDryBulb = Node((NodeNum)).OutAirDryBulb;
                            LocalAzimuth = MultizoneExternalNodeData(i).azimuth;
                            AirflowNetworkNodeSimu(n).PZ = calculate_wind_pressure(MultizoneExternalNodeData(i).curve,
                                                                                   MultizoneExternalNodeData(i).symmetricCurve,
                                                                                   MultizoneExternalNodeData(i).useRelativeAngle,
                                                                                   LocalAzimuth,
                                                                                   LocalWindSpeed,
                                                                                   LocalWindDir,
                                                                                   LocalDryBulb,
                                                                                   LocalHumRat);
                            AirflowNetworkNodeSimu(n).TZ = LocalDryBulb;
                            AirflowNetworkNodeSimu(n).WZ = LocalHumRat;
                        }
                    }

                } else {
                    ShowSevereError(m_state,
                                    "GetAirflowNetworkInput: AIRFLOWNETWORK:DISTRIBUTION:NODE: Invalid external node = " +
                                        AirflowNetworkNodeData(n).Name);
                    CalcAirflowNetworkAirBalanceErrorsFound = true;
                }
            }
        }

        for (i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
            if (i > AirflowNetworkNumOfSurfaces - NumOfLinksIntraZone) {
                continue;
            }
            if (AirflowNetworkLinkageData(i).element->type() == ComponentType::SCR) {
                AirflowNetworkLinkageData(i).control = MultizoneSurfaceData(i).Factor;
            }
            if (MultizoneSurfaceData(i).OccupantVentilationControlNum == 0) {
                MultizoneSurfaceData(i).OpenFactor = 0.0;
            }
            j = MultizoneSurfaceData(i).SurfNum;
            auto const &surf = m_state.dataSurface->Surface(j);
            if (surf.OriginalClass == SurfaceClass::Window || surf.OriginalClass == SurfaceClass::Door ||
                surf.OriginalClass == SurfaceClass::GlassDoor || surf.IsAirBoundarySurf) {
                if (MultizoneSurfaceData(i).OccupantVentilationControlNum > 0) {
                    if (MultizoneSurfaceData(i).OpeningStatus == OpenStatus::FreeOperation) {
                        if (MultizoneSurfaceData(i).OpeningProbStatus == ProbabilityCheck::ForceChange) {
                            MultizoneSurfaceData(i).OpenFactor = MultizoneSurfaceData(i).Factor;
                        } else if (MultizoneSurfaceData(i).ClosingProbStatus == ProbabilityCheck::ForceChange) {
                            MultizoneSurfaceData(i).OpenFactor = 0.0;
                        } else if (MultizoneSurfaceData(i).ClosingProbStatus == ProbabilityCheck::KeepStatus ||
                                   MultizoneSurfaceData(i).OpeningProbStatus == ProbabilityCheck::KeepStatus) {
                            MultizoneSurfaceData(i).OpenFactor = MultizoneSurfaceData(i).OpenFactorLast;
                        } else {
                            venting_control(i, MultizoneSurfaceData(i).OpenFactor);
                        }
                    }
                } else {
                    venting_control(i, MultizoneSurfaceData(i).OpenFactor);
                }
                MultizoneSurfaceData(i).OpenFactor *= MultizoneSurfaceData(i).WindModifier;
                if (MultizoneSurfaceData(i).HybridVentClose) {
                    MultizoneSurfaceData(i).OpenFactor = 0.0;
                    if (m_state.dataSurface->SurfWinVentingOpenFactorMultRep(j) > 0.0) {
                        m_state.dataSurface->SurfWinVentingOpenFactorMultRep(j) = 0.0;
                    }
                }
                if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::DOP ||
                    AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::SOP ||
                    AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::HOP) {
                    if (AirflowNetworkFanActivated && distribution_simulated && MultizoneSurfaceData(i).OpenFactor > 0.0 &&
                        (m_state.dataSurface->Surface(j).ExtBoundCond == ExternalEnvironment ||
                         (m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCond == OtherSideCoefNoCalcExt &&
                          m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtWind)) &&
                        !m_state.dataGlobal->WarmupFlag) {
                        // Exterior Large opening only
                        ++MultizoneSurfaceData(i).ExtLargeOpeningErrCount;
                        if (MultizoneSurfaceData(i).ExtLargeOpeningErrCount < 2) {
                            ShowWarningError(m_state,
                                             "AirflowNetwork: The window or door is open during HVAC system operation " +
                                                 MultizoneSurfaceData(i).SurfName);
                            ShowContinueError(m_state, format("The window or door opening factor is {:.2R}", MultizoneSurfaceData(i).OpenFactor));
                            ShowContinueErrorTimeStamp(m_state, "");
                        } else {
                            ShowRecurringWarningErrorAtEnd(m_state,
                                                           "AirFlowNetwork: " + MultizoneSurfaceData(i).SurfName +
                                                               " The window or door is open during HVAC system operation error continues...",
                                                           MultizoneSurfaceData(i).ExtLargeOpeningErrIndex,
                                                           MultizoneSurfaceData(i).OpenFactor,
                                                           MultizoneSurfaceData(i).OpenFactor);
                        }
                    }
                }
                if (MultizoneSurfaceData(i).OpenFactor > 1.0) {
                    ++MultizoneSurfaceData(i).OpenFactorErrCount;
                    if (MultizoneSurfaceData(i).OpenFactorErrCount < 2) {
                        ShowWarningError(m_state,
                                         "AirflowNetwork: The window or door opening factor is greater than 1.0 " + MultizoneSurfaceData(i).SurfName);
                        ShowContinueErrorTimeStamp(m_state, "");
                    } else {
                        ShowRecurringWarningErrorAtEnd(m_state,
                                                       "AirFlowNetwork: " + MultizoneSurfaceData(i).SurfName +
                                                           " The window or door opening factor is greater than 1.0 error continues...",
                                                       MultizoneSurfaceData(i).OpenFactorErrIndex,
                                                       MultizoneSurfaceData(i).OpenFactor,
                                                       MultizoneSurfaceData(i).OpenFactor);
                    }
                }
            }
        }

        // Check if the global ventilation control is applied or not
        GlobalOpenFactor = -1.0;
        for (i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
            if (i > AirflowNetworkNumOfSurfaces - NumOfLinksIntraZone) {
                continue;
            }
            if (MultizoneSurfaceData(i).HybridCtrlMaster) {
                GlobalOpenFactor = MultizoneSurfaceData(i).OpenFactor;
                break;
            }
        }
        if (GlobalOpenFactor >= 0.0) {
            for (i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
                if (i > AirflowNetworkNumOfSurfaces - NumOfLinksIntraZone) {
                    continue;
                }
                j = MultizoneSurfaceData(i).SurfNum;
                auto const &surf = m_state.dataSurface->Surface(j);
                if (surf.OriginalClass == SurfaceClass::Window || surf.OriginalClass == SurfaceClass::Door ||
                    surf.OriginalClass == SurfaceClass::GlassDoor) {
                    if (MultizoneSurfaceData(i).HybridCtrlGlobal) {
                        MultizoneSurfaceData(i).OpenFactor = GlobalOpenFactor;
                    }
                }
            }
        }

        if (!Par.allocated()) {
            Par.allocate(1);
            Par = 0.0;
        }

        PressureSetFlag = 0;

        if (NumOfPressureControllers == 1) {
            if (PressureControllerData(1).availSched == nullptr) {
                PressureSetFlag = PressureControllerData(1).ControlTypeSet;
            } else if (PressureControllerData(1).availSched->getCurrentVal() > 0.0) {
                PressureSetFlag = PressureControllerData(1).ControlTypeSet;
            }
            if (PressureSetFlag > 0) {
                PressureSet = PressureControllerData(1).presSetpointSched->getCurrentVal();
            }
        }

        initialize_calculation();

        if (!(PressureSetFlag > 0 && AirflowNetworkFanActivated)) {
            airmov();
        } else if (PressureSetFlag == PressureCtrlExhaust) {
            AirLoopNum = AirflowNetworkNodeData(PressureControllerData(1).AFNNodeNum).AirLoopNum;
            MinExhaustMassFlowrate = 2.0 * VerySmallMassFlow;
            MaxExhaustMassFlowrate = Node(PressureControllerData(1).OANodeNum).MassFlowRate;
            if (m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode == HVAC::FanOp::Cycling &&
                m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio > 0.0) {
                MaxExhaustMassFlowrate = MaxExhaustMassFlowrate / m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio;
            }
            ExhaustFanMassFlowRate = MinExhaustMassFlowrate;
            airmov();
            ZonePressure1 = AirflowNetworkNodeSimu(PressureControllerData(1).AFNNodeNum).PZ;
            if (ZonePressure1 <= PressureSet) {
                // The highest pressure due to minimum flow rate could not reach Pressure set, bypass pressure set calculation
                if (!m_state.dataGlobal->WarmupFlag) {
                    if (ErrCountLowPre == 0) {
                        ++ErrCountLowPre;
                        ShowWarningError(m_state,
                                         "The calculated pressure with minimum exhaust fan rate is lower than the pressure setpoint. The pressure "
                                         "control is unable to perform.");
                        ShowContinueErrorTimeStamp(m_state,
                                                   format("Calculated pressure = {:.2R}[Pa], Pressure setpoint ={:.2R}", ZonePressure1, PressureSet));
                    } else {
                        ++ErrCountLowPre;
                        ShowRecurringWarningErrorAtEnd(m_state,
                                                       AirflowNetworkNodeData(PressureControllerData(1).AFNNodeNum).Name +
                                                           ": The AFN model continues not to perform pressure control due to lower zone pressure...",
                                                       ErrIndexLowPre,
                                                       ZonePressure1,
                                                       ZonePressure1);
                    }
                }
            } else {
                ExhaustFanMassFlowRate = MaxExhaustMassFlowrate;
                airmov();
                ZonePressure2 = AirflowNetworkNodeSimu(PressureControllerData(1).AFNNodeNum).PZ;
                if (ZonePressure2 >= PressureSet) {
                    // The lowest pressure due to maximum flow rate is still higher than Pressure set, bypass pressure set calculation
                    if (!m_state.dataGlobal->WarmupFlag) {
                        if (ErrCountHighPre == 0) {
                            ++ErrCountHighPre;
                            ShowWarningError(m_state,
                                             "The calculated pressure with maximum exhaust fan rate is higher than the pressure setpoint. The "
                                             "pressure control is unable to perform.");
                            ShowContinueErrorTimeStamp(
                                m_state, format("Calculated pressure = {:.2R}[Pa], Pressure setpoint = {:.2R}", ZonePressure2, PressureSet));
                        } else {
                            ++ErrCountHighPre;
                            ShowRecurringWarningErrorAtEnd(
                                m_state,
                                AirflowNetworkNodeData(PressureControllerData(1).AFNNodeNum).Name +
                                    ": The AFN model continues not to perform pressure control due to higher zone pressure...",
                                ErrIndexHighPre,
                                ZonePressure2,
                                ZonePressure2);
                        }
                    }
                } else {
                    //    if ( ZonePressure1 > PressureSet && ZonePressure2 < PressureSet ) {
                    auto &thisState = m_state; // can't use m_state directly in the capture list, just create a reference
                    auto f = [&thisState, PressureSet](Real64 const ControllerMassFlowRate) {
                        return AFNPressureResidual(thisState, ControllerMassFlowRate, PressureSet);
                    };
                    General::SolveRoot(
                        m_state, ErrorToler, MaxIte, SolFla, ExhaustFanMassFlowRate, f, MinExhaustMassFlowrate, MaxExhaustMassFlowrate);
                    if (SolFla == -1) {
                        if (!m_state.dataGlobal->WarmupFlag) {
                            if (ErrCountVar == 0) {
                                ++ErrCountVar;
                                ShowWarningError(m_state, "Iteration limit exceeded pressure setpoint using an exhaust fan. Simulation continues.");
                                ShowContinueErrorTimeStamp(m_state, format("Exhaust fan flow rate = {:.4R}", ExhaustFanMassFlowRate));
                            } else {
                                ++ErrCountVar;
                                ShowRecurringWarningErrorAtEnd(m_state,
                                                               PressureControllerData(1).Name +
                                                                   "\": Iteration limit warning exceeding pressure setpoint continues...",
                                                               ErrIndexVar,
                                                               ExhaustFanMassFlowRate,
                                                               ExhaustFanMassFlowRate);
                            }
                        }
                    } else if (SolFla == -2) {
                        ShowFatalError(m_state,
                                       "Zone pressure control failed using an exhaust fan: no solution is reached, for " +
                                           PressureControllerData(1).Name);
                    }
                }
            }
        } else { // PressureCtrlRelief - Pressure control type is Relief Flow
            AirLoopNum = AirflowNetworkNodeData(PressureControllerData(1).AFNNodeNum).AirLoopNum;
            MinReliefMassFlowrate = 2.0 * VerySmallMassFlow;
            MaxReliefMassFlowrate = Node(PressureControllerData(1).OANodeNum).MassFlowRate;
            if (m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode == HVAC::FanOp::Cycling &&
                m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio > 0.0) {
                MaxReliefMassFlowrate = MaxReliefMassFlowrate / m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio;
            }
            ReliefMassFlowRate = MinReliefMassFlowrate;
            initialize_calculation();
            airmov();
            ZonePressure1 = AirflowNetworkNodeSimu(PressureControllerData(1).AFNNodeNum).PZ;

            if (ZonePressure1 <= PressureSet) {
                // The highest pressure due to minimum flow rate could not reach Pressure set, bypass pressure set calculation
                if (!m_state.dataGlobal->WarmupFlag) {
                    if (ErrCountLowPre == 0) {
                        ++ErrCountLowPre;
                        ShowWarningError(m_state,
                                         "The calculated pressure with minimum relief air rate is lower than the pressure setpoint. The pressure "
                                         "control is unable to perform.");
                        ShowContinueErrorTimeStamp(m_state,
                                                   format("Calculated pressure = {:.2R}[Pa], Pressure setpoint ={:.2R}", ZonePressure1, PressureSet));
                    } else {
                        ++ErrCountLowPre;
                        ShowRecurringWarningErrorAtEnd(m_state,
                                                       AirflowNetworkNodeData(PressureControllerData(1).AFNNodeNum).Name +
                                                           ": The AFN model continues not to perform pressure control due to lower zone pressure...",
                                                       ErrIndexLowPre,
                                                       ZonePressure1,
                                                       ZonePressure1);
                    }
                }
            } else {
                ReliefMassFlowRate = MaxReliefMassFlowrate;
                initialize_calculation();
                airmov();
                ZonePressure2 = AirflowNetworkNodeSimu(PressureControllerData(1).AFNNodeNum).PZ;
                if (ZonePressure2 >= PressureSet) {
                    // The lowest pressure due to maximum flow rate is still higher than Pressure set, bypass pressure set calculation
                    if (!m_state.dataGlobal->WarmupFlag) {
                        if (ErrCountHighPre == 0) {
                            ++ErrCountHighPre;
                            ShowWarningError(m_state,
                                             "The calculated pressure with maximum relief air rate is higher than the pressure setpoint. The "
                                             "pressure control is unable to perform.");
                            ShowContinueErrorTimeStamp(
                                m_state, format("Calculated pressure = {:.2R}[Pa], Pressure setpoint = {:.2R}", ZonePressure2, PressureSet));
                        } else {
                            ++ErrCountHighPre;
                            ShowRecurringWarningErrorAtEnd(
                                m_state,
                                AirflowNetworkNodeData(PressureControllerData(1).AFNNodeNum).Name +
                                    ": The AFN model continues not to perform pressure control due to higher zone pressure...",
                                ErrIndexHighPre,
                                ZonePressure2,
                                ZonePressure2);
                        }
                    }
                } else {
                    //    if ( ZonePressure1 > PressureSet && ZonePressure2 < PressureSet ) {
                    auto &thisState = m_state; // can't use m_state directly in the capture list, just create a reference
                    auto f = [&thisState, PressureSet](Real64 const ControllerMassFlowRate) {
                        return AFNPressureResidual(thisState, ControllerMassFlowRate, PressureSet);
                    };
                    General::SolveRoot(m_state, ErrorToler, MaxIte, SolFla, ReliefMassFlowRate, f, MinReliefMassFlowrate, MaxReliefMassFlowrate);
                    if (SolFla == -1) {
                        if (!m_state.dataGlobal->WarmupFlag) {
                            if (ErrCountVar == 0) {
                                ++ErrCountVar;
                                ShowWarningError(m_state, "Iteration limit exceeded pressure setpoint using relief air. Simulation continues.");
                                ShowContinueErrorTimeStamp(m_state, format("Relief air flow rate = {:.4R}", ReliefMassFlowRate));
                            } else {
                                ++ErrCountVar;
                                ShowRecurringWarningErrorAtEnd(m_state,
                                                               PressureControllerData(1).Name +
                                                                   "\": Iteration limit warning exceeding pressure setpoint continues...",
                                                               ErrIndexVar,
                                                               ReliefMassFlowRate,
                                                               ReliefMassFlowRate);
                            }
                        }
                    } else if (SolFla == -2) {
                        ShowFatalError(
                            m_state, "Zone pressure control failed using relief air: no solution is reached, for " + PressureControllerData(1).Name);
                    }
                }
            }
        }
    }

    Real64 AFNPressureResidual(EnergyPlusData &state,
                               Real64 const ControllerMassFlowRate, // Pressure setpoint
                               Real64 PressureSet)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   April 2016
        //       MODIFIED       NA
        //       RE-ENGINEERED  NA

        // PURPOSE OF THIS FUNCTION:
        //  Calculates residual function ((ZonePressure - PressureSet)/PressureSet)

        // METHODOLOGY EMPLOYED:
        //  Calls AIRMOV to get the pressure in the controlled zone and calculates the residual as defined above

        // Return value
        Real64 AFNPressureResidual;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 ZonePressure;

        if (state.afn->PressureSetFlag == PressureCtrlExhaust) {
            state.afn->ExhaustFanMassFlowRate = ControllerMassFlowRate;
        }

        if (state.afn->PressureSetFlag == PressureCtrlRelief) {
            state.afn->ReliefMassFlowRate = ControllerMassFlowRate;
        }
        auto &solver = state.afn;
        solver->initialize_calculation();
        solver->airmov();

        ZonePressure = state.afn->AirflowNetworkNodeSimu(state.afn->PressureControllerData(1).AFNNodeNum).PZ;

        if (PressureSet != 0.0) {
            AFNPressureResidual = (ZonePressure - PressureSet) / PressureSet;
        } else {
            AFNPressureResidual = (ZonePressure - PressureSet);
        }
        return AFNPressureResidual;
    }

    static int makeTable(EnergyPlusData &state, const std::string &name, const int gridIndex, const std::vector<Real64> &y)
    {
        // Add a new table and performance curve
        std::string contextString = "CalcWindPressureCoeffs: Creating table \"" + name + "\"";
        std::pair<EnergyPlusData *, std::string> callbackPair{&state, contextString};
        state.dataCurveManager->btwxtManager.setLoggingContext(&callbackPair);

        Curve::Curve *curve = Curve::AddCurve(state, name);

        curve->numDims = 1;

        curve->curveType = Curve::CurveType::BtwxtTableLookup;

        curve->inputLimits[0].min = 0.0;
        curve->inputLimits[0].minPresent = true;
        curve->inputLimits[0].max = 360.0;
        curve->inputLimits[0].maxPresent = true;

        curve->TableIndex = gridIndex;
        curve->GridValueIndex = state.dataCurveManager->btwxtManager.addOutputValues(gridIndex, y);

        return curve->Num;
    }

    void Solver::calculate_Cps()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   May 2003
        //       MODIFIED       Revised by L. Gu, Nov. 2005, to meet requirements of AirflowNetwork
        //       MODIFIED       Revised by L. Gu, Dec. 2008, to set the number of external nodes based on
        //                      the number of external surfaces
        //       MODIFIED       Revised by J. DeGraw, Feb. 2017, to use tables
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates surface-average wind pressure coefficients for
        // the walls and roof of a rectangular building.

        // METHODOLOGY EMPLOYED:
        // Interpolates correlations between surface-average wind pressure coefficient and wind direction based on
        // measurements (see REFERENCES). Applicable only to rectangular buildings.

        // REFERENCES:
        // For low-rise buildings: M.V. Swami and S. Chandra, Correlations for Pressure Distribution
        // on Buildings and Calculation of Natural-Ventilation Airflow. ASHRAE Transactions 94 (1): 243-266.
        // For high-rise buildings: 2001 ASHRAE Fundamentals Handbook, p. 16.5, Fig. 7, "Surface Averaged
        // Wall Pressure Coefficients for Tall Buildings" and p.16.6, Fig. 9, "Surface Averaged Roof Pressure
        // Coefficients for Tall Buildings; from R.E. Akins, J.A. Peterka, and J.E. Cermak. 1979.
        // Averaged Pressure Coefficients for Rectangular Buildings. Wind Engineering. Proc. Fifth
        // International Conference 7:369-80, Fort Collins, CO. Pergamon Press, NY.

        // Using/Aliasing
        using namespace DataSurfaces;

        //  index 1 is wind incidence angle (0,30,60,...,300,330 deg)
        //  index 2 is side ratio (0.25,1.0,4.0)
        // Surface-averaged wind-pressure coefficient array for walls
        static constexpr std::array<std::array<Real64, 12>, 3> CPHighRiseWall = {{
            {0.60, 0.54, 0.23, -0.25, -0.61, -0.55, -0.51, -0.55, -0.61, -0.25, 0.23, 0.54},
            {0.60, 0.48, 0.04, -0.56, -0.56, -0.42, -0.37, -0.42, -0.56, -0.56, 0.04, 0.48},
            {0.60, 0.44, -0.26, -0.70, -0.53, -0.32, -0.22, -0.32, -0.53, -0.70, -0.26, 0.44},
        }};

        //  index 1 is wind incidence angle (0,30,60,...,300,330 deg)
        //  index 2 is side ratio (0.25,0.5,1.0)
        // Surface-averaged wind-pressure coefficient array for roof
        static constexpr std::array<std::array<Real64, 12>, 3> CPHighRiseRoof = {{
            {-0.28, -0.69, -0.72, -0.76, -0.72, -0.69, -0.28, -0.69, -0.72, -0.76, -0.72, -0.69},
            {-0.47, -0.52, -0.70, -0.76, -0.70, -0.52, -0.47, -0.52, -0.70, -0.76, -0.70, -0.52},
            {-0.70, -0.55, -0.55, -0.70, -0.55, -0.55, -0.70, -0.55, -0.55, -0.70, -0.55, -0.55},
        }};

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int FacadeNum;         // Facade number
        int ExtNum;            // External number
        int AFNZnNum;          // Zone number
        Real64 SideRatio;      // For vertical facades, width of facade / width of adjacent facade
        Real64 SR;             // SideRatio restricted to 0.25 to 4.0 range
        Real64 SideRatioFac;   // LOG(SideRatio)
        Real64 IncRad;         // IncAng in radians
        int IAng;              // Incidence angle index; used in interpolation
        Real64 DelAng;         // Incidence angle difference; used in interpolation
        Real64 WtAng;          // Incidence angle weighting factor; used in interpolation
        int ISR;               // Side ratio index, for interpolation
        Real64 WtSR;           // Side ratio weighting factor; used in interpolation
        int SurfNum;           // Surface number
        int SurfDatNum;        // Surface data number
        Real64 SurfAng;        // Azimuth angle of surface normal (degrees clockwise from North)
        int FacadeNumThisSurf; // Facade number for a particular surface
        Real64 AngDiff;        // Angle difference between wind and surface direction (deg)
        Real64 AngDiffMin;     // Minimum angle difference between wind and surface direction (deg)
        std::vector<int> curveIndex = {0, 0, 0, 0, 0};

        // Facade azimuth angle
        for (FacadeNum = 1; FacadeNum <= 4; ++FacadeNum) {
            FacadeAng(FacadeNum) = m_state.afn->simulation_control.azimuth + (FacadeNum - 1) * 90.0;
            if (FacadeAng(FacadeNum) >= 360.0) {
                FacadeAng(FacadeNum) -= 360.0;
            }
        }

        FacadeAng(5) = simulation_control.azimuth + 90.0;

        // Create AirflowNetwork external node objects -- one for each of the external surfaces

        MultizoneExternalNodeData.allocate(AirflowNetworkNumOfExtSurfaces);
        AirflowNetworkNumOfExtNode = AirflowNetworkNumOfExtSurfaces;
        NumOfExtNodes = AirflowNetworkNumOfExtSurfaces;
        for (ExtNum = 1; ExtNum <= NumOfExtNodes; ++ExtNum) {
            MultizoneExternalNodeData(ExtNum).ExtNum = AirflowNetworkNumOfZones + ExtNum;
            MultizoneExternalNodeData(ExtNum).Name = format("ExtNode{:4}", ExtNum);
        }

        // Associate each external node with SurfaceData

        ExtNum = 0;
        for (SurfDatNum = 1; SurfDatNum <= AirflowNetworkNumOfSurfaces; ++SurfDatNum) {
            if (SurfDatNum > AirflowNetworkNumOfSurfaces - NumOfLinksIntraZone) {
                continue;
            }
            SurfNum = MultizoneSurfaceData(SurfDatNum).SurfNum;
            if (SurfNum == 0) {
                continue; // Error caught earlier
            }
            if (m_state.dataSurface->Surface(SurfNum).ExtBoundCond == ExternalEnvironment ||
                (m_state.dataSurface->Surface(SurfNum).ExtBoundCond == OtherSideCoefNoCalcExt && m_state.dataSurface->Surface(SurfNum).ExtWind)) {
                ++ExtNum;
                if (m_state.dataSurface->Surface(SurfNum).Tilt >= 45.0) { // "Vertical" surface
                    SurfAng = m_state.dataSurface->Surface(SurfNum).Azimuth;
                    FacadeNumThisSurf = 1;
                    AngDiffMin = std::abs(SurfAng - FacadeAng(1));
                    if (AngDiffMin > 359.0) {
                        AngDiffMin = std::abs(AngDiffMin - 360.0);
                    }
                    for (FacadeNum = 2; FacadeNum <= 4; ++FacadeNum) {
                        AngDiff = std::abs(SurfAng - FacadeAng(FacadeNum));
                        if (AngDiff > 359.0) {
                            AngDiff = std::abs(AngDiff - 360.0);
                        }
                        if (AngDiff < AngDiffMin) {
                            AngDiffMin = AngDiff;
                            FacadeNumThisSurf = FacadeNum;
                        }
                    }
                    MultizoneExternalNodeData(ExtNum).facadeNum = FacadeNumThisSurf;
                } else { // "Roof" surface
                    MultizoneExternalNodeData(ExtNum).facadeNum = 5;
                }
                MultizoneSurfaceData(SurfDatNum).NodeNums[1] = MultizoneExternalNodeData(ExtNum).ExtNum;
                MultizoneSurfaceData(SurfDatNum).ExternalNodeName = MultizoneExternalNodeData(ExtNum).Name;
            }
        }

        // Check if using the advanced single sided model
        for (AFNZnNum = 1; AFNZnNum <= AirflowNetworkNumOfZones; ++AFNZnNum) {
            if (MultizoneZoneData(AFNZnNum).SingleSidedCpType == "ADVANCED") {
                ++AirflowNetworkNumOfSingleSideZones;
            }
        }

        std::vector<Real64> dirs30 = {0, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330, 360};
        std::vector<Btwxt::GridAxis> dirs30Axes;
        dirs30Axes.emplace_back(dirs30,
                                "30 Degree Increment",
                                Btwxt::InterpolationMethod::linear,
                                Btwxt::ExtrapolationMethod::linear,
                                std::pair<double, double>{0.0, 360.0});

        auto dirs30GridIndex = m_state.dataCurveManager->btwxtManager.addGrid("30 Degree Increments", dirs30Axes);

        if (AirflowNetworkNumOfSingleSideZones == 0) { // do the standard surface average coefficient calculation
            // Create the array of wind directions

            // Create a curve for each facade
            for (FacadeNum = 1; FacadeNum <= 5; ++FacadeNum) {
                if (FacadeNum == 1 || FacadeNum == 3 || FacadeNum == 5) {
                    SideRatio = simulation_control.aspect_ratio;
                } else { // FacadeNum = 2 or 4
                    SideRatio = 1.0 / simulation_control.aspect_ratio;
                }
                if (Util::SameString(simulation_control.BldgType, "HighRise") && FacadeNum != 5) {
                    SideRatio = 1.0 / SideRatio;
                }
                SideRatioFac = std::log(SideRatio);
                std::vector<Real64> vals(13);
                for (int windDirNum = 1; windDirNum <= 12; ++windDirNum) {
                    Real64 WindAng = (windDirNum - 1) * 30.0;
                    IncAng = std::abs(WindAng - FacadeAng(FacadeNum));
                    if (IncAng > 180.0) {
                        IncAng = 360.0 - IncAng;
                    }
                    IAng = int(IncAng / 30.0) + 1;
                    DelAng = mod(IncAng, 30.0);
                    WtAng = 1.0 - DelAng / 30.0;

                    // Wind-pressure coefficients for vertical facades, low-rise building

                    if (Util::SameString(simulation_control.BldgType, "LowRise") && FacadeNum <= 4) {
                        IncRad = IncAng * Constant::DegToRad;
                        Real64 const cos_IncRad_over_2(std::cos(IncRad / 2.0));
                        vals[windDirNum - 1] = 0.6 * std::log(1.248 - 0.703 * std::sin(IncRad / 2.0) - 1.175 * pow_2(std::sin(IncRad)) +
                                                              0.131 * pow_3(std::sin(2.0 * IncRad * SideRatioFac)) + 0.769 * cos_IncRad_over_2 +
                                                              0.07 * pow_2(SideRatioFac * std::sin(IncRad / 2.0)) + 0.717 * pow_2(cos_IncRad_over_2));
                    }

                    // Wind-pressure coefficients for vertical facades, high-rise building

                    else if (Util::SameString(simulation_control.BldgType, "HighRise") && FacadeNum <= 4) {
                        SR = min(max(SideRatio, 0.25), 4.0);
                        if (SR >= 0.25 && SR < 1.0) {
                            ISR = 1;
                            WtSR = (1.0 - SR) / 0.75;
                        } else { // 1.0 <= SR <= 4.0
                            ISR = 2;
                            WtSR = (4.0 - SR) / 3.0;
                        }
                        vals[windDirNum - 1] = WtSR * (WtAng * CPHighRiseWall[ISR - 1][IAng - 1] + (1.0 - WtAng) * CPHighRiseWall[ISR - 1][IAng]) +
                                               (1.0 - WtSR) * (WtAng * CPHighRiseWall[ISR][IAng - 1] + (1.0 - WtAng) * CPHighRiseWall[ISR][IAng]);
                    }

                    // Wind-pressure coefficients for roof (assumed same for low-rise and high-rise buildings)

                    else if ((Util::SameString(simulation_control.BldgType, "HighRise") ||
                              Util::SameString(simulation_control.BldgType, "LowRise")) &&
                             FacadeNum == 5) {
                        SR = min(max(SideRatio, 0.25), 1.0);
                        if (SR >= 0.25 && SR < 0.5) {
                            ISR = 1;
                            WtSR = (0.5 - SR) / 0.25;
                        } else { // 0.5 <= SR <= 1.0
                            ISR = 2;
                            WtSR = (1.0 - SR) / 0.5;
                        }
                        vals[windDirNum - 1] = WtSR * (WtAng * CPHighRiseRoof[ISR - 1][IAng - 1] + (1.0 - WtAng) * CPHighRiseRoof[ISR - 1][IAng]) +
                                               (1.0 - WtSR) * (WtAng * CPHighRiseRoof[ISR][IAng - 1] + (1.0 - WtAng) * CPHighRiseRoof[ISR][IAng]);
                    }

                } // End of wind direction loop
                // Add new table
                vals[12] = vals[0]; // Enforce periodicity
                curveIndex[FacadeNum - 1] = AirflowNetwork::makeTable(m_state, format("!WPCTABLE{}", FacadeNum), dirs30GridIndex, vals);
            } // End of facade number loop

        } else { //-calculate the advanced single sided wind pressure coefficients

            // Calculate the wind pressure coefficients vs. wind direction for each external node
            // The wind pressure coeffients are stored temporarily in the "valsByFacade" vector and then
            // converted into a table near the end of this else. There will be at least seven profiles
            // (four sides plus one roof plus two for each pair of windows). The name is thus a little
            // misleading, as it isn't really the values by facade once you get beyond the first five.
            std::vector<std::vector<Real64>> valsByFacade(5);
            for (FacadeNum = 0; FacadeNum < 4; ++FacadeNum) {
                valsByFacade[FacadeNum] = std::vector<Real64>(36);
            }
            FacadeNum = 4;
            valsByFacade[FacadeNum] = std::vector<Real64>(12);
            for (FacadeNum = 1; FacadeNum <= 4; ++FacadeNum) {
                if (FacadeNum == 1 || FacadeNum == 3) {
                    SideRatio = simulation_control.aspect_ratio;
                } else { // FacadeNum = 2 or 4
                    SideRatio = 1.0 / simulation_control.aspect_ratio;
                }
                if (Util::SameString(simulation_control.BldgType, "HighRise")) {
                    SideRatio = 1.0 / SideRatio;
                }
                SideRatioFac = std::log(SideRatio);
                for (int windDirNum = 1; windDirNum <= 36; ++windDirNum) {
                    Real64 WindAng = (windDirNum - 1) * 10.0;
                    IncAng = std::abs(WindAng - FacadeAng(FacadeNum));
                    if (IncAng > 180.0) {
                        IncAng = 360.0 - IncAng;
                    }
                    // IAng = int(IncAng / 10.0) + 1;
                    DelAng = mod(IncAng, 10.0);
                    WtAng = 1.0 - DelAng / 10.0;
                    // Wind-pressure coefficients for vertical facades, low-rise building
                    IncRad = IncAng * Constant::DegToRad;
                    valsByFacade[FacadeNum - 1][windDirNum - 1] =
                        0.6 * std::log(1.248 - 0.703 * std::sin(IncRad / 2.0) - 1.175 * pow_2(std::sin(IncRad)) +
                                       0.131 * pow_3(std::sin(2.0 * IncRad * SideRatioFac)) + 0.769 * std::cos(IncRad / 2.0) +
                                       0.07 * pow_2(SideRatioFac * std::sin(IncRad / 2.0)) + 0.717 * pow_2(std::cos(IncRad / 2.0)));
                } // End of wind direction loop
            } // End of facade number loop
            // Add a roof
            FacadeNum = 5;
            SR = min(max(SideRatio, 0.25), 1.0);
            if (SR >= 0.25 && SR < 0.5) {
                ISR = 1;
                WtSR = (0.5 - SR) / 0.25;
            } else { // 0.5 <= SR <= 1.0
                ISR = 2;
                WtSR = (1.0 - SR) / 0.5;
            }
            for (int windDirNum = 1; windDirNum <= 12; ++windDirNum) {
                Real64 WindAng = (windDirNum - 1) * 30.0;
                IncAng = std::abs(WindAng - FacadeAng(FacadeNum));
                if (IncAng > 180.0) {
                    IncAng = 360.0 - IncAng;
                }
                IAng = int(IncAng / 30.0) + 1;
                DelAng = mod(IncAng, 30.0);
                WtAng = 1.0 - DelAng / 30.0;
                // Wind-pressure coefficients for roof (assumed same for low-rise and high-rise buildings)
                valsByFacade[FacadeNum - 1][windDirNum - 1] =
                    WtSR * (WtAng * CPHighRiseRoof[ISR - 1][IAng - 1] + (1.0 - WtAng) * CPHighRiseRoof[ISR - 1][IAng]) +
                    (1.0 - WtSR) * (WtAng * CPHighRiseRoof[ISR][IAng - 1] + (1.0 - WtAng) * CPHighRiseRoof[ISR][IAng]);
            }
            single_sided_Cps(valsByFacade); // run the advanced single sided subroutine if at least one zone calls for it
            // Resize the curve index array
            curveIndex.resize(valsByFacade.size());
            // Create the curves

            std::vector<Real64> dirs10 = {0,   10,  20,  30,  40,  50,  60,  70,  80,  90,  100, 110, 120, 130, 140, 150, 160, 170, 180,
                                          190, 200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300, 310, 320, 330, 340, 350, 360};

            std::vector<Btwxt::GridAxis> dirs10Axes;
            dirs10Axes.emplace_back(dirs10,
                                    "10 Degree Increments",
                                    Btwxt::InterpolationMethod::linear,
                                    Btwxt::ExtrapolationMethod::linear,
                                    std::pair<double, double>{0.0, 360.0});

            auto dirs10GridIndex = m_state.dataCurveManager->btwxtManager.addGrid("10 Degree Increments", dirs10Axes);

            for (FacadeNum = 1; FacadeNum <= 4; ++FacadeNum) {
                valsByFacade[FacadeNum - 1].push_back(valsByFacade[FacadeNum - 1][0]); // Enforce periodicity
                curveIndex[FacadeNum - 1] =
                    AirflowNetwork::makeTable(m_state, format("!SSWPCTABLEFACADE{}", FacadeNum), dirs10GridIndex, valsByFacade[FacadeNum - 1]);
            }
            FacadeNum = 5;
            valsByFacade[FacadeNum - 1].push_back(valsByFacade[FacadeNum - 1][0]); // Enforce periodicity
            curveIndex[FacadeNum - 1] =
                AirflowNetwork::makeTable(m_state, format("!SSWPCTABLEFACADE{}", FacadeNum), dirs30GridIndex, valsByFacade[FacadeNum - 1]);
            for (unsigned facadeNum = 6; facadeNum <= valsByFacade.size(); ++facadeNum) {
                valsByFacade[facadeNum - 1].push_back(valsByFacade[facadeNum - 1][0]); // Enforce periodicity
                curveIndex[facadeNum - 1] =
                    AirflowNetwork::makeTable(m_state, format("!SSWPCTABLE{}", facadeNum), dirs10GridIndex, valsByFacade[facadeNum - 1]);
            }
        }
        // Connect the external nodes to the new curves
        for (ExtNum = 1; ExtNum <= NumOfExtNodes; ++ExtNum) {
            MultizoneExternalNodeData(ExtNum).curve = curveIndex[MultizoneExternalNodeData(ExtNum).facadeNum - 1];
        }
    }

    Real64 Solver::calculate_wind_pressure(int const curve,           // Curve index, change this to pointer after curve refactor
                                           bool const symmetricCurve, // True if the curve is symmetric (0 to 180)
                                           bool const relativeAngle,  // True if the Cp curve angle is measured relative to the surface
                                           Real64 const azimuth,      // Azimuthal angle of surface
                                           Real64 const windSpeed,    // Wind velocity
                                           Real64 const windDir,      // Wind direction
                                           Real64 const dryBulbTemp,  // Air node dry bulb temperature
                                           Real64 const humRat        // Air node humidity ratio
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       Jason DeGraw, Feb. 2017, modify to use curves
        //       MODIFIED       Xuan Luo, Aug. 2017, modify to use local air condition
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates surface wind pressure based on given CP values

        // REFERENCES:
        // COMIS Fundamentals

        // Return value is wind pressure[Pa]

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 angle(windDir);
        Real64 rho; // Outdoor air density
        Real64 Cp;  // Cp value at given wind direction

        // Calculate outdoor density
        rho = PsyRhoAirFnPbTdbW(m_state, m_state.dataEnvrn->OutBaroPress, dryBulbTemp, humRat);

        // Calculate pressure coefficient
        if (relativeAngle) {
            angle = angle - azimuth;
            if (angle < 0.0) {
                angle += 360.0;
            }
        }
        if (symmetricCurve) {
            if (angle > 180.0) {
                angle = 360.0 - angle;
            }
        }
        Cp = Curve::CurveValue(m_state, curve, angle);

        return Cp * 0.5 * rho * windSpeed * windSpeed;
    }

    Real64 Solver::duct_inside_convection_resistance(Real64 const Tair, // Average air temperature
                                                     Real64 const mdot, // Mass flow rate
                                                     Real64 const Dh,   // Hydraulic diameter
                                                     Real64 const hIn   // User defined convection coefficient
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell, Tony Fontanini
        //       DATE WRITTEN   Feb. 2017
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates duct inside convection coefficients

        // REFERENCES:
        // ASTM C1340
        // Jakob, F.E.,  Fischer, R.D., Flanigan, L.J. 1987. "Experimental Validation of the Duct Submodel for the SP43 Simulation Model."
        // ASHRAE Trans. pp 1499-1514.

        Real64 hIn_final = 0;

        if (hIn == 0) {

            Real64 Tair_IP = Tair * 1.8 + 32.0;     // Convert C to F
            Real64 mdot_IP = mdot * 2.20462 * 3600; // Convert kg/s to lb/hr
            Real64 Dh_IP = Dh * 3.28084;            // Convert m to ft
            Real64 Ai_IP = pow_2(Dh_IP) * Constant::Pi / 4;

            Real64 CorrelationCoeff = 0.00368 + 1.5e-6 * (Tair_IP - 80);
            Real64 MassFlux = mdot_IP / Ai_IP; // lb/hr-ft2

            Real64 DuctInsideConvCoeff_IP = CorrelationCoeff * pow(MassFlux, 0.8) / pow(Dh_IP, 0.2); // BTU/hr-ft2-F

            hIn_final = DuctInsideConvCoeff_IP * pow_2(3.28084) * 1.8 * 1055.06 / 3600; // Convert BTU/hr-ft2-F to W/m2-K

        } else {
            hIn_final = hIn;
        }

        if (hIn_final == 0) {
            return 0;
        } else {
            return 1 / hIn_final;
        }
    }

    Real64 Solver::duct_outside_convection_resistance(Real64 const Ts,   // Surface temperature
                                                      Real64 const Tamb, // Free air temperature
                                                      Real64 const Wamb, // Free air humidity ratio
                                                      Real64 const Pamb, // Free air barometric pressure
                                                      Real64 const Dh,   // Hydraulic diameter
                                                      int const ZoneNum, // Zone number
                                                      Real64 const hOut  // User defined convection coefficient
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell, Tony Fontanini
        //       DATE WRITTEN   Feb. 2017
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates duct outside convection coefficients

        // REFERENCES:
        // ASTM C1340

        Real64 k = properties.thermal_conductivity(Ts);
        auto &Zone(m_state.dataHeatBal->Zone);

        Real64 hOut_final = 0;

        if (hOut == 0) {

            // Free convection
            Real64 Pr = properties.prandtl_number(Pamb, (Ts + Tamb) / 2, Wamb);
            Real64 KinVisc = properties.kinematic_viscosity(Pamb, (Ts + Tamb) / 2, Wamb);
            Real64 Beta = 2.0 / ((Tamb + Constant::Kelvin) + (Ts + Constant::Kelvin));
            Real64 Gr = Constant::Gravity * Beta * std::abs(Ts - Tamb) * pow_3(Dh) / pow_2(KinVisc);
            Real64 Ra = Gr * Pr;
            Real64 Nu_free(0);

            if (Ra < 10e9) {
                Nu_free = 0.53 * pow(Ra, 0.25);
            } else {
                Nu_free = 0.13 * pow(Ra, 0.333);
            }

            Real64 V = 0;
            // Forced convection
            if (ZoneNum > 0) {
                Real64 ACH = zone_OA_change_rate(ZoneNum); // Zone air change rate [1/hr]
                Real64 Vol = Zone(ZoneNum).Volume;         // Zone volume [m3]
                V = pow(Vol, 0.333) * ACH / 3600;          // Average air speed in zone [m/s]
            } else {
                V = m_state.dataEnvrn->WindSpeed;
            }

            Real64 Re = V * Dh / KinVisc; // Reynolds number
            Real64 c = 0;
            Real64 n = 0;

            if (Re <= 4) {
                c = 0.989;
                n = 0.33;
            } else if (4 < Re && Re <= 40) {
                c = 0.911;
                n = 0.385;
            } else if (40 < Re && Re <= 4000) {
                c = 0.683;
                n = 0.466;
            } else if (4000 < Re && Re <= 40000) {
                c = 0.193;
                n = 0.618;
            } else if (40000 < Re) {
                c = 0.0266;
                n = 0.805;
            }

            Real64 Nu_forced = c * pow(Re, n) * pow(Pr, 0.333);

            Real64 Nu_combined = pow(pow_3(Nu_free) + pow_3(Nu_forced), 0.333);
            hOut_final = Nu_combined * k / Dh;

        } else {
            hOut_final = hOut;
        }

        if (hOut_final == 0) {
            return 0;
        } else {
            return 1 / hOut_final;
        }
    }

    void Solver::calculate_heat_balance()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  Revised based on Subroutine CalcADSHeatBalance

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs AirflowNetwork thermal simulations.

        // USE STATEMENTS:
        auto &TimeStepSys = m_state.dataHVACGlobal->TimeStepSys;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int LF;
        int LT;
        int CompNum;
        int NF;
        int NT;
        iComponentTypeNum CompTypeNum;
        int ExtNodeNum;
        Real64 Ei;
        Real64 DirSign;
        Real64 Tamb;
        Real64 Wamb;
        Real64 Pamb;
        Real64 CpAir;
        Real64 TZON;
        Real64 load;
        int ZoneNum;

        auto &Node(m_state.dataLoopNodes->Node);

        MA = 0.0;
        MV = 0.0;

        for (i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
            CompNum = AirflowNetworkLinkageData(i).CompNum;
            CompTypeNum = AirflowNetworkCompData(CompNum).CompTypeNum;
            std::string CompName = AirflowNetworkCompData(CompNum).EPlusName;
            CpAir = PsyCpAirFnW((AirflowNetworkNodeSimu(AirflowNetworkLinkageData(i).NodeNums[0]).WZ +
                                 AirflowNetworkNodeSimu(AirflowNetworkLinkageData(i).NodeNums[1]).WZ) /
                                2.0);
            // Calculate duct conduction loss
            if (CompTypeNum == iComponentTypeNum::DWC && CompName == std::string()) { // Duct element only
                int TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                // Fatal error when return flow is opposite to the desired direction
                if (AirflowNetworkLinkSimu(i).FLOW == 0.0 && AirflowNetworkLinkSimu(i).FLOW2 > 0.0) {
                    if (LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum)) {
                        ShowSevereError(m_state,
                                        "AirflowNetwork: The airflow direction is opposite to the intended direction (from node 1 to node 2) in "
                                        "AirflowNetwork:Distribution:Linkage = " +
                                            AirflowNetworkLinkageData(i).Name);
                        ShowContinueErrorTimeStamp(m_state, "");
                        ShowContinueError(m_state,
                                          "The sum of the airflows entering the zone is greater than the airflows leaving the zone (e.g., wind "
                                          "and stack effect).");
                        ShowContinueError(m_state,
                                          "Please check wind speed or reduce values of \"Window/Door Opening Factor, or Crack Factor\" defined in "
                                          "AirflowNetwork:MultiZone:Surface objects.");
                        //                    ShowFatalError(state,  "AirflowNetwork: The previous error causes termination." );
                    }
                }

                if (AirflowNetworkLinkageData(i).ZoneNum < 0) {
                    ExtNodeNum = AirflowNetworkLinkageData(i).NodeNums[1];
                    if (AirflowNetworkNodeData(ExtNodeNum).OutAirNodeNum > 0 && Node(AirflowNetworkNodeData(ExtNodeNum).OutAirNodeNum).IsLocalNode) {
                        Tamb = Node(AirflowNetworkNodeData(ExtNodeNum).OutAirNodeNum).OutAirDryBulb;
                        Wamb = Node(AirflowNetworkNodeData(ExtNodeNum).OutAirNodeNum).HumRat;
                    } else {
                        Tamb = OutDryBulbTempAt(m_state, AirflowNetworkNodeData(ExtNodeNum).NodeHeight);
                        Wamb = m_state.dataEnvrn->OutHumRat;
                    }
                } else if (AirflowNetworkLinkageData(i).ZoneNum == 0) {
                    Tamb = AirflowNetworkNodeSimu(LT).TZ;
                    Wamb = AirflowNetworkNodeSimu(LT).WZ;
                } else {
                    Tamb = ANZT(AirflowNetworkLinkageData(i).ZoneNum);
                    Wamb = ANZW(AirflowNetworkLinkageData(i).ZoneNum);
                }

                Pamb = m_state.dataEnvrn->OutBaroPress;

                Real64 constexpr tolerance = 0.001;
                Real64 UThermal(10); // Initialize. This will get updated.
                Real64 UThermal_iter = 0;
                Real64 Tsurr = Tamb;
                Real64 Tsurr_K = Tsurr + Constant::Kelvin;
                Real64 Tin = AirflowNetworkNodeSimu(LF).TZ;
                Real64 TDuctSurf = (Tamb + Tin) / 2.0;
                Real64 TDuctSurf_K = TDuctSurf + Constant::Kelvin;
                Real64 DuctSurfArea = DisSysCompDuctData(TypeNum).L * DisSysCompDuctData(TypeNum).hydraulicDiameter * Constant::Pi;

                // If user defined view factors not present, calculate air-to-air heat transfer
                if (AirflowNetworkLinkageData(i).LinkageViewFactorObjectNum == 0) {

                    // Calculate convection coefficient if one or both not present
                    if (DisSysCompDuctData(TypeNum).InsideConvCoeff == 0 && DisSysCompDuctData(TypeNum).OutsideConvCoeff == 0) {
                        while (std::abs(UThermal - UThermal_iter) > tolerance) {
                            UThermal_iter = UThermal;

                            Real64 RThermConvIn = duct_inside_convection_resistance(Tin,
                                                                                    AirflowNetworkLinkSimu(i).FLOW,
                                                                                    DisSysCompDuctData(TypeNum).hydraulicDiameter,
                                                                                    DisSysCompDuctData(TypeNum).InsideConvCoeff);
                            Real64 RThermConvOut = duct_outside_convection_resistance(TDuctSurf,
                                                                                      Tamb,
                                                                                      Wamb,
                                                                                      Pamb,
                                                                                      DisSysCompDuctData(TypeNum).hydraulicDiameter,
                                                                                      AirflowNetworkLinkageData(i).ZoneNum,
                                                                                      DisSysCompDuctData(TypeNum).OutsideConvCoeff);
                            Real64 RThermConduct = 1.0 / DisSysCompDuctData(TypeNum).UThermConduct;
                            Real64 RThermTotal = RThermConvIn + RThermConvOut + RThermConduct;
                            UThermal = pow(RThermTotal, -1);

                            // Duct conduction, assuming effectiveness = 1 - exp(-NTU)
                            Ei = General::epexp(-UThermal * DuctSurfArea, (DirSign * AirflowNetworkLinkSimu(i).FLOW * CpAir));
                            Real64 QCondDuct = std::abs(AirflowNetworkLinkSimu(i).FLOW) * CpAir * (Tamb - Tin) * (1 - Ei);

                            TDuctSurf = Tamb - QCondDuct * RThermConvOut / DuctSurfArea;
                        }
                    } else { // Air-to-air only. U and h values are all known
                        Real64 RThermConvIn = duct_inside_convection_resistance(Tin,
                                                                                AirflowNetworkLinkSimu(i).FLOW,
                                                                                DisSysCompDuctData(TypeNum).hydraulicDiameter,
                                                                                DisSysCompDuctData(TypeNum).InsideConvCoeff);
                        Real64 RThermConvOut = duct_outside_convection_resistance(TDuctSurf,
                                                                                  Tamb,
                                                                                  Wamb,
                                                                                  Pamb,
                                                                                  DisSysCompDuctData(TypeNum).hydraulicDiameter,
                                                                                  AirflowNetworkLinkageData(i).ZoneNum,
                                                                                  DisSysCompDuctData(TypeNum).OutsideConvCoeff);
                        Real64 RThermConduct = 1.0 / DisSysCompDuctData(TypeNum).UThermConduct;
                        Real64 RThermTotal = RThermConvIn + RThermConvOut + RThermConduct;
                        UThermal = pow(RThermTotal, -1);
                    }

                    Tsurr = Tamb;

                } else { // Air-to-air + radiation heat transfer

                    auto &VFObj(AirflowNetworkLinkageViewFactorData(AirflowNetworkLinkageData(i).LinkageViewFactorObjectNum));
                    VFObj.QRad = 0;
                    VFObj.QConv = 0;

                    Real64 Tin_ave = Tin;
                    Real64 hOut = 0;

                    while (std::abs(UThermal - UThermal_iter) > tolerance) {
                        UThermal_iter = UThermal;

                        Real64 RThermConvIn = duct_inside_convection_resistance(Tin_ave,
                                                                                AirflowNetworkLinkSimu(i).FLOW,
                                                                                DisSysCompDuctData(TypeNum).hydraulicDiameter,
                                                                                DisSysCompDuctData(TypeNum).InsideConvCoeff);
                        Real64 RThermConvOut = duct_outside_convection_resistance(TDuctSurf,
                                                                                  Tamb,
                                                                                  Wamb,
                                                                                  Pamb,
                                                                                  DisSysCompDuctData(TypeNum).hydraulicDiameter,
                                                                                  AirflowNetworkLinkageData(i).ZoneNum,
                                                                                  DisSysCompDuctData(TypeNum).OutsideConvCoeff);

                        if (RThermConvOut > 0.0) {
                            hOut = 1 / RThermConvOut;
                        }

                        Real64 RThermConduct = 1.0 / DisSysCompDuctData(TypeNum).UThermConduct;

                        Real64 hrjTj_sum = 0;
                        Real64 hrj_sum = 0;

                        for (int j = 1; j <= VFObj.LinkageSurfaceData.u(); ++j) {

                            int ZoneSurfNum = VFObj.LinkageSurfaceData(j).SurfaceNum;

                            Real64 TSurfj = m_state.dataHeatBalSurf->SurfOutsideTempHist(1)(ZoneSurfNum);
                            Real64 TSurfj_K = TSurfj + Constant::Kelvin;

                            Real64 ZoneSurfEmissivity =
                                m_state.dataConstruction->Construct(m_state.dataSurface->Surface(ZoneSurfNum).Construction).InsideAbsorpThermal;
                            Real64 ZoneSurfArea = m_state.dataSurface->Surface(ZoneSurfNum).Area;

                            Real64 DuctEmissivity = VFObj.DuctEmittance;
                            Real64 DuctExposureFrac = VFObj.DuctExposureFraction;
                            Real64 DuctToZoneSurfViewFactor = VFObj.LinkageSurfaceData(j).ViewFactor;

                            Real64 DuctSurfResistance = (1 - DuctEmissivity) / (DuctExposureFrac * DuctSurfArea * DuctEmissivity);
                            Real64 SpaceResistance = 1 / (DuctExposureFrac * DuctSurfArea * DuctToZoneSurfViewFactor);
                            Real64 ZoneSurfResistance = (1 - ZoneSurfEmissivity) / (ZoneSurfArea * ZoneSurfEmissivity);

                            VFObj.LinkageSurfaceData(j).SurfaceResistanceFactor =
                                Constant::StefanBoltzmann / (DuctSurfResistance + SpaceResistance + ZoneSurfResistance);

                            Real64 hrj = VFObj.LinkageSurfaceData(j).SurfaceResistanceFactor * (TDuctSurf_K + TSurfj_K) *
                                         (pow_2(TDuctSurf_K) + pow_2(TSurfj_K)) / DuctSurfArea;

                            hrjTj_sum += hrj * TSurfj;
                            hrj_sum += hrj;
                        }

                        Tsurr = (hOut * Tamb + hrjTj_sum) / (hOut + hrj_sum); // Surroundings temperature [C]
                        Tsurr_K = Tsurr + Constant::Kelvin;

                        Real64 RThermTotal = RThermConvIn + RThermConduct + 1 / (hOut + hrj_sum);
                        UThermal = pow(RThermTotal, -1);

                        Real64 NTU = UThermal * DuctSurfArea / (DirSign * AirflowNetworkLinkSimu(i).FLOW * CpAir);
                        Tin_ave = Tsurr + (Tin - Tsurr) * (1 / NTU) * (1 - exp(-NTU));

                        TDuctSurf = Tin_ave - UThermal * (RThermConvIn + RThermConduct) * (Tin_ave - Tsurr);
                        TDuctSurf_K = TDuctSurf + Constant::Kelvin;
                    }

                    for (int j = 1; j <= VFObj.LinkageSurfaceData.u(); ++j) {
                        int ZoneSurfNum = VFObj.LinkageSurfaceData(j).SurfaceNum;
                        Real64 TSurfj = m_state.dataHeatBalSurf->SurfOutsideTempHist(1)(ZoneSurfNum);
                        Real64 TSurfj_K = TSurfj + Constant::Kelvin;
                        VFObj.LinkageSurfaceData(j).SurfaceRadLoad = VFObj.LinkageSurfaceData(j).SurfaceResistanceFactor *
                                                                     (pow_4(TDuctSurf_K) - pow_4(TSurfj_K)); // Radiant load for this surface [W]
                        int SurfNum = VFObj.LinkageSurfaceData(j).SurfaceNum;
                        Real64 ZoneSurfaceArea = m_state.dataSurface->Surface(SurfNum).Area;
                        m_state.dataHeatBalFanSys->QRadSurfAFNDuct(SurfNum) += VFObj.LinkageSurfaceData(j).SurfaceRadLoad * TimeStepSys *
                                                                               Constant::rSecsInHour /
                                                                               ZoneSurfaceArea; // Energy to each surface per unit area [J/m2]
                        VFObj.QRad += VFObj.LinkageSurfaceData(j).SurfaceRadLoad; // Total radiant load from all surfaces for this system timestep [W]
                    }

                    VFObj.QConv = hOut * DuctSurfArea * (TDuctSurf - Tamb);
                    UThermal = (VFObj.QRad + VFObj.QConv) / (DuctSurfArea * std::abs(Tsurr - Tin));
                }

                if (!LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum) && AirflowNetworkLinkSimu(i).FLOW <= 0.0) {
                    Ei = General::epexp(-UThermal * DuctSurfArea, (AirflowNetworkLinkSimu(i).FLOW2 * CpAir));
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW2) * CpAir * Ei;
                    MV(LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2) * Tsurr * (1.0 - Ei) * CpAir;
                } else {
                    Ei = General::epexp(-UThermal * DuctSurfArea, (DirSign * AirflowNetworkLinkSimu(i).FLOW * CpAir));
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW) * CpAir * Ei;
                    MV(LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW) * Tsurr * (1.0 - Ei) * CpAir;
                }
            }
            if (CompTypeNum == iComponentTypeNum::TMU) { // Reheat unit: SINGLE DUCT:CONST VOLUME:REHEAT
                int TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                Ei = General::epexp(-0.001 * DisSysCompTermUnitData(TypeNum).L * DisSysCompTermUnitData(TypeNum).hydraulicDiameter * Constant::Pi,
                                    (DirSign * AirflowNetworkLinkSimu(i).FLOW * CpAir));
                Tamb = AirflowNetworkNodeSimu(LT).TZ;
                if (!LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum) && AirflowNetworkLinkSimu(i).FLOW <= 0.0) {
                    Ei = General::epexp(-0.001 * DisSysCompTermUnitData(TypeNum).L * DisSysCompTermUnitData(TypeNum).hydraulicDiameter * Constant::Pi,
                                        (AirflowNetworkLinkSimu(i).FLOW2 * CpAir));
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW2) * CpAir * Ei;
                    MV(LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2) * Tamb * (1.0 - Ei) * CpAir;
                } else {
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW) * CpAir * Ei;
                    MV(LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW) * Tamb * (1.0 - Ei) * CpAir;
                }
            }
            if (CompTypeNum == iComponentTypeNum::COI) { // heating or cooling coil
                // TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
            }
            // Calculate temp in a constant pressure drop element
            if (CompTypeNum == iComponentTypeNum::CPD && CompName == std::string()) { // constant pressure element only
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) {                           // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                }
                if (!LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum) && AirflowNetworkLinkSimu(i).FLOW <= 0.0) {
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                } else {
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                }
                MV(LT) = 0.0;
            }
            // Calculate return leak
            if ((CompTypeNum == iComponentTypeNum::PLR || CompTypeNum == iComponentTypeNum::ELR) && CompName == std::string()) {
                // Return leak element only
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusZoneNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                }
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).ExtNodeNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                }
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusZoneNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                }
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).ExtNodeNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                }
            }
            // Check reheat unit or coil
            if (AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::RHT && (!AirflowNetworkLinkageData(i).VAVTermDamper)) {
                NF = 0;
                NT = 0;
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusNodeNum > 0) {
                    NF = AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusNodeNum;
                }
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusNodeNum > 0) {
                    NT = AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusNodeNum;
                }
                if ((NF == 0) || (NT == 0)) {
                    ShowFatalError(m_state,
                                   "Node number in the primary air loop is not found in AIRFLOWNETWORK:DISTRIBUTION:NODE = " +
                                       AirflowNetworkLinkageData(i).Name);
                }
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    load = Node(NT).Temp - Node(NF).Temp;
                } else {
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    load = Node(NF).Temp - Node(NT).Temp;
                }
                CpAir = PsyCpAirFnW(Node(NT).HumRat);
                MV(LT) += AirflowNetworkLinkSimu(i).FLOW * CpAir * load;
            }
        }

        // Prescribe temperature for EPlus nodes
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            bool found = false;
            bool OANode = false;
            for (int j = 1; j <= AirflowNetworkNumOfLinks; ++j) {
                if (AirflowNetworkLinkageData(j).NodeNums[0] == i || AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                    CompNum = AirflowNetworkLinkageData(j).CompNum;
                    if (AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::RHT && (!AirflowNetworkLinkageData(j).VAVTermDamper)) {
                        found = true;
                        break;
                    }
                    // Overwrite fan outlet node
                    if (AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::FAN && AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                        found = false;
                        break;
                    }
                    // Overwrite return connection outlet
                    if (AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::RCN) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                    if (AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::SCN &&
                        AirflowNetworkLinkageData(j).NodeNums[1] == i) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                }
                if (AirflowNetworkLinkageData(j).NodeNums[1] == i &&
                    AirflowNetworkNodeData(AirflowNetworkLinkageData(j).NodeNums[0]).EPlusTypeNum == iEPlusNodeType::OAN) {
                    OANode = true;
                    break;
                }
            }
            if (found) {
                continue;
            }
            if (AirflowNetworkNodeData(i).EPlusZoneNum == 0 && AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::ZIN) {
                continue;
            }
            int j = AirflowNetworkNodeData(i).EPlusNodeNum;

            if (j > 0 &&
                (AirflowNetworkNodeData(i).EPlusZoneNum > 0 || AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::FOU ||
                 AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::COU || AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::HXO)) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = Node(j).Temp * 1.0e10;
            }
            if (j > 0 && OANode) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = Node(j).Temp * 1.0e10;
            }
            if (AirflowNetworkNodeData(i).EPlusZoneNum > 0 && MA((i - 1) * AirflowNetworkNumOfNodes + i) < 0.9e10) {
                ZoneNum = AirflowNetworkNodeData(i).EPlusZoneNum;
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = ANZT(ZoneNum) * 1.0e10;
            }
            if (AirflowNetworkNodeData(i).ExtNodeNum > 0 && MA((i - 1) * AirflowNetworkNumOfNodes + i) < 0.9e10) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                if (AirflowNetworkNodeData(i).OutAirNodeNum > 0) {
                    MV(i) = Node(AirflowNetworkNodeData(i).OutAirNodeNum).OutAirDryBulb * 1.0e10;
                } else {
                    MV(i) = OutDryBulbTempAt(m_state, AirflowNetworkNodeData(i).NodeHeight) * 1.0e10;
                }
            }
            if (AirflowNetworkNodeData(i).RAFNNodeNum > 0 && MA((i - 1) * AirflowNetworkNumOfNodes + i) < 0.9e10) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                ZoneNum = AirflowNetworkNodeData(i).EPlusZoneNum;
                if (m_state.dataRoomAir->AFNZoneInfo(ZoneNum).Node(AirflowNetworkNodeData(i).RAFNNodeNum).AFNNodeID == i) {
                    MV(i) = m_state.dataRoomAir->AFNZoneInfo(ZoneNum).Node(AirflowNetworkNodeData(i).RAFNNodeNum).AirTemp * 1.0e10;
                }
            }
        }

        // Assign node value to distribution nodes with fan off
        for (i = 1 + NumOfNodesMultiZone; i <= AirflowNetworkNumOfNodes; ++i) {
            int j = AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0 && !LoopOnOffFlag(AirflowNetworkNodeData(i).AirLoopNum) && MA((i - 1) * AirflowNetworkNumOfNodes + i) < 1.0e9) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = Node(j).Temp * 1.0e10;
            }
            if (j == 0 && i > NumOfNodesMultiZone && !LoopOnOffFlag(AirflowNetworkNodeData(i).AirLoopNum)) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = AirflowNetworkNodeSimu(i).TZlast * 1.0e10;
            }
        }

        // Check singularity
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            if (MA((i - 1) * AirflowNetworkNumOfNodes + i) < 1.0e-6) {
                if (i > NumOfNodesMultiZone && !LoopOnOffFlag(AirflowNetworkNodeData(i).AirLoopNum)) {
                    MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                    MV(i) = AirflowNetworkNodeSimu(i).TZlast * 1.0e10;
                } else {
                    ShowFatalError(m_state,
                                   "CalcAirflowNetworkHeatBalance: A diagonal entity is zero in AirflowNetwork matrix at node " +
                                       AirflowNetworkNodeData(i).Name);
                }
            }
        }

        // Get an inverse matrix
        mrxinv(AirflowNetworkNumOfNodes);

        // Calculate node temperatures
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            TZON = 0.0;
            for (int j = 1; j <= AirflowNetworkNumOfNodes; ++j) {
                TZON += MA((i - 1) * AirflowNetworkNumOfNodes + j) * MV(j);
            }
            AirflowNetworkNodeSimu(i).TZ = TZON;
        }
    }

    void Solver::calculate_moisture_balance()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  Revised based on Subroutine CalcADSMoistureBalance

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs AirflowNetwork moisture simulations.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int j;
        int LF;
        int LT;
        int CompNum;
        int NF;
        int NT;
        iComponentTypeNum CompTypeNum;
        int TypeNum;
        Real64 Ei;
        Real64 DirSign;
        Real64 Wamb;
        Real64 WZON;
        Real64 load;
        int ZoneNum;

        auto &Node(m_state.dataLoopNodes->Node);

        MA = 0.0;
        MV = 0.0;
        for (i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
            CompNum = AirflowNetworkLinkageData(i).CompNum;
            CompTypeNum = AirflowNetworkCompData(CompNum).CompTypeNum;
            std::string CompName = AirflowNetworkCompData(CompNum).EPlusName;
            // Calculate duct moisture diffusion loss
            if (CompTypeNum == iComponentTypeNum::DWC && CompName == std::string()) { // Duct component only
                TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                Ei = General::epexp(-DisSysCompDuctData(TypeNum).UMoisture * DisSysCompDuctData(TypeNum).L *
                                        DisSysCompDuctData(TypeNum).hydraulicDiameter * Constant::Pi,
                                    (DirSign * AirflowNetworkLinkSimu(i).FLOW));
                if (AirflowNetworkLinkageData(i).ZoneNum < 0) {
                    Wamb = m_state.dataEnvrn->OutHumRat;
                } else if (AirflowNetworkLinkageData(i).ZoneNum == 0) {
                    Wamb = AirflowNetworkNodeSimu(LT).WZ;
                } else {
                    Wamb = ANZW(AirflowNetworkLinkageData(i).ZoneNum);
                }
                if (!LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum) && AirflowNetworkLinkSimu(i).FLOW <= 0.0) {
                    Ei = General::epexp(-DisSysCompDuctData(TypeNum).UMoisture * DisSysCompDuctData(TypeNum).L *
                                            DisSysCompDuctData(TypeNum).hydraulicDiameter * Constant::Pi,
                                        (AirflowNetworkLinkSimu(i).FLOW2));
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW2) * Ei;
                    MV(LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2) * Wamb * (1.0 - Ei);
                } else {
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW) * Ei;
                    MV(LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW) * Wamb * (1.0 - Ei);
                }
            }
            if (CompTypeNum == iComponentTypeNum::TMU) { // Reheat unit: SINGLE DUCT:CONST VOLUME:REHEAT
                TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                Ei = General::epexp(-0.0001 * DisSysCompTermUnitData(TypeNum).L * DisSysCompTermUnitData(TypeNum).hydraulicDiameter * Constant::Pi,
                                    (DirSign * AirflowNetworkLinkSimu(i).FLOW));
                Wamb = AirflowNetworkNodeSimu(LT).WZ;
                if (!LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum) && AirflowNetworkLinkSimu(i).FLOW <= 0.0) {

                    Ei =
                        General::epexp(-0.0001 * DisSysCompTermUnitData(TypeNum).L * DisSysCompTermUnitData(TypeNum).hydraulicDiameter * Constant::Pi,
                                       (AirflowNetworkLinkSimu(i).FLOW2));
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW2) * Ei;
                    MV(LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2) * Wamb * (1.0 - Ei);
                } else {
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW) * Ei;
                    MV(LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW) * Wamb * (1.0 - Ei);
                }
            }
            if (CompTypeNum == iComponentTypeNum::COI) { // heating or cooling coil
                TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
            }
            // Calculate temp in a constant pressure drop component
            if (CompTypeNum == iComponentTypeNum::CPD && CompName == std::string()) { // constant pressure element only
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) {                           // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                }
                if (!LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum) && AirflowNetworkLinkSimu(i).FLOW <= 0.0) {
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                } else {
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW);
                }
                MV(LT) = 0.0;
            }
            // Calculate return leak
            if ((CompTypeNum == iComponentTypeNum::PLR || CompTypeNum == iComponentTypeNum::ELR) && CompName == std::string()) {
                // Return leak component only
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusZoneNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW);
                }
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).ExtNodeNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW);
                }
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusZoneNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                }
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).ExtNodeNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                }
            }
            // Check reheat unit
            if (AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::RHT && (!AirflowNetworkLinkageData(i).VAVTermDamper)) {
                NF = 0;
                NT = 0;
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusNodeNum > 0) {
                    NF = AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusNodeNum;
                }
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusNodeNum > 0) {
                    NT = AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusNodeNum;
                }
                if ((NF == 0) || (NT == 0)) {
                    ShowFatalError(m_state,
                                   "Node number in the primary air loop is not found in AIRFLOWNETWORK:DISTRIBUTION:NODE = " +
                                       AirflowNetworkLinkageData(i).Name);
                }
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    load = Node(NT).HumRat - Node(NF).HumRat;
                } else {
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    load = Node(NF).HumRat - Node(NT).HumRat;
                }
                MV(LT) += AirflowNetworkLinkSimu(i).FLOW * load;
            }
        }

        // Prescribe temperature for EPlus nodes
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            bool found = false;
            bool OANode = false;
            for (j = 1; j <= AirflowNetworkNumOfLinks; ++j) {
                if (AirflowNetworkLinkageData(j).NodeNums[0] == i || AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                    CompNum = AirflowNetworkLinkageData(j).CompNum;
                    if (AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::RHT && (!AirflowNetworkLinkageData(j).VAVTermDamper)) {
                        found = true;
                        break;
                    }
                    // Overwrite fan outlet node
                    if (AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::FAN && AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                        found = false;
                        break;
                    }
                    // Overwrite return connection outlet
                    if (AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::RCN) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                    if (AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::SCN &&
                        AirflowNetworkLinkageData(j).NodeNums[1] == i) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                }
                if (AirflowNetworkLinkageData(j).NodeNums[1] == i &&
                    AirflowNetworkNodeData(AirflowNetworkLinkageData(j).NodeNums[0]).EPlusTypeNum == iEPlusNodeType::OAN) {
                    OANode = true;
                    break;
                }
            }
            if (found) {
                continue;
            }
            if (AirflowNetworkNodeData(i).EPlusZoneNum == 0 && AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::ZIN) {
                continue;
            }
            j = AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0 &&
                (AirflowNetworkNodeData(i).EPlusZoneNum > 0 || AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::FOU ||
                 AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::COU || AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::HXO)) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = Node(j).HumRat * 1.0e10;
            }
            if (j > 0 && OANode) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = Node(j).HumRat * 1.0e10;
            }
            if (AirflowNetworkNodeData(i).EPlusZoneNum > 0 && MA((i - 1) * AirflowNetworkNumOfNodes + i) < 0.9e10) {
                ZoneNum = AirflowNetworkNodeData(i).EPlusZoneNum;
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = ANZW(ZoneNum) * 1.0e10;
            }
            if (AirflowNetworkNodeData(i).ExtNodeNum > 0) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = m_state.dataEnvrn->OutHumRat * 1.0e10;
            }
            if (AirflowNetworkNodeData(i).RAFNNodeNum > 0 && MA((i - 1) * AirflowNetworkNumOfNodes + i) < 0.9e10) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                ZoneNum = AirflowNetworkNodeData(i).EPlusZoneNum;
                if (m_state.dataRoomAir->AFNZoneInfo(ZoneNum).Node(AirflowNetworkNodeData(i).RAFNNodeNum).AFNNodeID == i) {
                    MV(i) = m_state.dataRoomAir->AFNZoneInfo(ZoneNum).Node(AirflowNetworkNodeData(i).RAFNNodeNum).HumRat * 1.0e10;
                }
            }
        }

        // Assign node value to distribution nodes with fan off
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            j = AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0 && !LoopOnOffFlag(AirflowNetworkNodeData(i).AirLoopNum) && MA((i - 1) * AirflowNetworkNumOfNodes + i) < 1.0e9) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = Node(j).HumRat * 1.0e10;
            }
            if (j == 0 && i > NumOfNodesMultiZone && !LoopOnOffFlag(AirflowNetworkNodeData(i).AirLoopNum)) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = AirflowNetworkNodeSimu(i).WZlast * 1.0e10;
            }
        }

        // Check singularity
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            if (MA((i - 1) * AirflowNetworkNumOfNodes + i) < 1.0e-8) {
                ShowFatalError(m_state,
                               "CalcAirflowNetworkMoisBalance: A diagonal entity is zero in AirflowNetwork matrix at node " +
                                   AirflowNetworkNodeData(i).Name);
            }
        }

        // Get an inverse matrix
        mrxinv(AirflowNetworkNumOfNodes);

        // Calculate node temperatures
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            WZON = 0.0;
            for (j = 1; j <= AirflowNetworkNumOfNodes; ++j) {
                WZON += MA((i - 1) * AirflowNetworkNumOfNodes + j) * MV(j);
            }
            AirflowNetworkNodeSimu(i).WZ = WZON;
        }
    }

    void Solver::calculate_CO2_balance()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   June. 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  Revised based on Subroutine CalcAirflowNetworkMoisBalance

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs AirflowNetwork CO2 simulations.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int j;
        int LF;
        int LT;
        int CompNum;
        iComponentTypeNum CompTypeNum;
        Real64 DirSign;
        Real64 COZN;
        int ZoneNum;

        MA = 0.0;
        MV = 0.0;
        for (i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
            CompNum = AirflowNetworkLinkageData(i).CompNum;
            CompTypeNum = AirflowNetworkCompData(CompNum).CompTypeNum;
            std::string CompName = AirflowNetworkCompData(CompNum).EPlusName;
            // Calculate duct moisture diffusion loss
            if (CompTypeNum == iComponentTypeNum::DWC && CompName == std::string()) { // Duct component only
                // TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW);
            }
            if (CompTypeNum == iComponentTypeNum::TMU) { // Reheat unit: SINGLE DUCT:CONST VOLUME:REHEAT
                // TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW);
            }
            if (CompTypeNum == iComponentTypeNum::COI) { // heating or cooling coil
                // TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
            }
            // Calculate temp in a constant pressure drop component
            if (CompTypeNum == iComponentTypeNum::CPD && CompName == std::string()) { // constant pressure element only
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) {                           // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                }
                MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW);
                MV(LT) = 0.0;
            }
            // Calculate return leak
            if ((CompTypeNum == iComponentTypeNum::PLR || CompTypeNum == iComponentTypeNum::ELR) && CompName == std::string()) {
                // Return leak component only
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusZoneNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW);
                }
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).ExtNodeNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW);
                }
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusZoneNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                }
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).ExtNodeNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                }
            }
        }

        // Prescribe temperature for EPlus nodes
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            bool found = false;
            bool OANode = false;
            for (j = 1; j <= AirflowNetworkNumOfLinks; ++j) {
                if (AirflowNetworkLinkageData(j).NodeNums[0] == i || AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                    CompNum = AirflowNetworkLinkageData(j).CompNum;
                    if (AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::RHT && (!AirflowNetworkLinkageData(j).VAVTermDamper)) {
                        found = true;
                        break;
                    }
                    // Overwrite fan outlet node
                    if (AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::FAN && AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                        found = false;
                        break;
                    }
                    // Overwrite return connection outlet
                    if (AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::RCN) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                    if (AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::SCN &&
                        AirflowNetworkLinkageData(j).NodeNums[1] == i) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                }
                if (AirflowNetworkLinkageData(j).NodeNums[1] == i &&
                    AirflowNetworkNodeData(AirflowNetworkLinkageData(j).NodeNums[0]).EPlusTypeNum == iEPlusNodeType::OAN) {
                    OANode = true;
                    break;
                }
            }
            if (found) {
                continue;
            }
            if (AirflowNetworkNodeData(i).EPlusZoneNum == 0 && AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::ZIN) {
                continue;
            }
            j = AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0 &&
                (AirflowNetworkNodeData(i).EPlusZoneNum > 0 || AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::FOU ||
                 AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::COU || AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::HXO)) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = m_state.dataLoopNodes->Node(j).CO2 * 1.0e10;
            }
            if (j > 0 && OANode) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = m_state.dataLoopNodes->Node(j).CO2 * 1.0e10;
            }
            if (AirflowNetworkNodeData(i).EPlusZoneNum > 0 && MA((i - 1) * AirflowNetworkNumOfNodes + i) < 0.9e10) {
                ZoneNum = AirflowNetworkNodeData(i).EPlusZoneNum;
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = ANCO(ZoneNum) * 1.0e10;
            }
            if (AirflowNetworkNodeData(i).ExtNodeNum > 0) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = m_state.dataContaminantBalance->OutdoorCO2 * 1.0e10;
            }
        }

        // Assign node value to distribution nodes with fan off
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            j = AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0 && !LoopOnOffFlag(AirflowNetworkNodeData(i).AirLoopNum) && MA((i - 1) * AirflowNetworkNumOfNodes + i) < 1.0e9) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = m_state.dataLoopNodes->Node(j).CO2 * 1.0e10;
            }
            if (j == 0 && i > NumOfNodesMultiZone && !LoopOnOffFlag(AirflowNetworkNodeData(i).AirLoopNum)) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = AirflowNetworkNodeSimu(i).CO2Zlast * 1.0e10;
            }
        }

        // Check singularity
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            if (MA((i - 1) * AirflowNetworkNumOfNodes + i) < 1.0e-6) {
                ShowFatalError(m_state,
                               "CalcAirflowNetworkCO2Balance: A diagonal entity is zero in AirflowNetwork matrix at node " +
                                   AirflowNetworkNodeData(i).Name);
            }
        }

        // Get an inverse matrix
        mrxinv(AirflowNetworkNumOfNodes);

        // Calculate node temperatures
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            COZN = 0.0;
            for (j = 1; j <= AirflowNetworkNumOfNodes; ++j) {
                COZN += MA((i - 1) * AirflowNetworkNumOfNodes + j) * MV(j);
            }
            AirflowNetworkNodeSimu(i).CO2Z = COZN;
        }
    }

    void Solver::calculate_GC_balance()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Jan. 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  Revised based on Subroutine CalcAirflowNetworkCO2Balance

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs AirflowNetwork generic contaminant simulations.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int j;
        int LF;
        int LT;
        int CompNum;
        iComponentTypeNum CompTypeNum;
        Real64 DirSign;
        Real64 COZN;
        int ZoneNum;

        MA = 0.0;
        MV = 0.0;
        for (i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
            CompNum = AirflowNetworkLinkageData(i).CompNum;
            CompTypeNum = AirflowNetworkCompData(CompNum).CompTypeNum;
            std::string_view CompName = AirflowNetworkCompData(CompNum).EPlusName;
            // Calculate duct moisture diffusion loss
            if (CompTypeNum == iComponentTypeNum::DWC && CompName.empty()) { // Duct component only
                // TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW);
            }
            if (CompTypeNum == iComponentTypeNum::TMU) { // Reheat unit: SINGLE DUCT:CONST VOLUME:REHEAT
                // TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW);
            }
            if (CompTypeNum == iComponentTypeNum::COI) { // heating or cooling coil
                // TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
            }
            // Calculate temp in a constant pressure drop component
            if (CompTypeNum == iComponentTypeNum::CPD && CompName == std::string()) { // constant pressure element only
                if (AirflowNetworkLinkSimu(i).FLOW > 0.0) {                           // flow direction is the same as input from node 1 to node 2
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                }
                MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW);
                MV(LT) = 0.0;
            }
            // Calculate return leak
            if ((CompTypeNum == iComponentTypeNum::PLR || CompTypeNum == iComponentTypeNum::ELR) && CompName == std::string()) {
                // Return leak component only
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusZoneNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW);
                }
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).ExtNodeNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = AirflowNetworkLinkageData(i).NodeNums[1];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW);
                }
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusZoneNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                }
                if ((AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).ExtNodeNum > 0) &&
                    (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusZoneNum == 0) && (AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = AirflowNetworkLinkageData(i).NodeNums[0];
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LT) += std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                    MA((LT - 1) * AirflowNetworkNumOfNodes + LF) = -std::abs(AirflowNetworkLinkSimu(i).FLOW2);
                }
            }
        }

        // Prescribe temperature for EPlus nodes
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            bool found = false;
            bool OANode = false;
            for (j = 1; j <= AirflowNetworkNumOfLinks; ++j) {
                if (AirflowNetworkLinkageData(j).NodeNums[0] == i || AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                    CompNum = AirflowNetworkLinkageData(j).CompNum;
                    if (AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::RHT && (!AirflowNetworkLinkageData(j).VAVTermDamper)) {
                        found = true;
                        break;
                    }
                    // Overwrite fan outlet node
                    if (AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::FAN && AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                        found = false;
                        break;
                    }
                    // Overwrite return connection outlet
                    if (AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::RCN) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                    if (AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::SCN &&
                        AirflowNetworkLinkageData(j).NodeNums[1] == i) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                }
                if (AirflowNetworkLinkageData(j).NodeNums[1] == i &&
                    AirflowNetworkNodeData(AirflowNetworkLinkageData(j).NodeNums[0]).EPlusTypeNum == iEPlusNodeType::OAN) {
                    OANode = true;
                    break;
                }
            }
            if (found) {
                continue;
            }
            if (AirflowNetworkNodeData(i).EPlusZoneNum == 0 && AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::ZIN) {
                continue;
            }
            j = AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0 &&
                (AirflowNetworkNodeData(i).EPlusZoneNum > 0 || AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::FOU ||
                 AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::COU || AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::HXO)) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = m_state.dataLoopNodes->Node(j).GenContam * 1.0e10;
            }
            if (j > 0 && OANode) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = m_state.dataLoopNodes->Node(j).GenContam * 1.0e10;
            }
            if (AirflowNetworkNodeData(i).EPlusZoneNum > 0 && MA((i - 1) * AirflowNetworkNumOfNodes + i) < 0.9e10) {
                ZoneNum = AirflowNetworkNodeData(i).EPlusZoneNum;
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = ANGC(ZoneNum) * 1.0e10;
            }
            if (AirflowNetworkNodeData(i).ExtNodeNum > 0) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = m_state.dataContaminantBalance->OutdoorGC * 1.0e10;
            }
        }

        // Assign node value to distribution nodes with fan off
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            j = AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0 && !LoopOnOffFlag(AirflowNetworkNodeData(i).AirLoopNum) && MA((i - 1) * AirflowNetworkNumOfNodes + i) < 1.0e9) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = m_state.dataLoopNodes->Node(j).GenContam * 1.0e10;
            }
            if (j == 0 && i > NumOfNodesMultiZone && !LoopOnOffFlag(AirflowNetworkNodeData(i).AirLoopNum)) {
                MA((i - 1) * AirflowNetworkNumOfNodes + i) = 1.0e10;
                MV(i) = AirflowNetworkNodeSimu(i).GCZlast * 1.0e10;
            }
        }

        // Check singularity
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            if (MA((i - 1) * AirflowNetworkNumOfNodes + i) < 1.0e-6) {
                ShowFatalError(m_state,
                               "CalcAirflowNetworkGCBalance: A diagonal entity is zero in AirflowNetwork matrix at node " +
                                   AirflowNetworkNodeData(i).Name);
            }
        }

        // Get an inverse matrix
        mrxinv(AirflowNetworkNumOfNodes);

        // Calculate node temperatures
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            COZN = 0.0;
            for (j = 1; j <= AirflowNetworkNumOfNodes; ++j) {
                COZN += MA((i - 1) * AirflowNetworkNumOfNodes + j) * MV(j);
            }
            AirflowNetworkNodeSimu(i).GCZ = COZN;
        }
    }

    void Solver::mrxinv(int const NORDER)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  Revised based on Subroutine ADSINV

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine inverses a matrix

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int j;
        int K;
        int M;
        Real64 R1;
        Real64 S;

        IVEC = 0;
        for (i = 1; i <= NORDER; ++i) {
            IVEC(i + 20) = i;
        }
        for (i = 1; i <= NORDER; ++i) {
            R1 = 0.0;
            M = i;
            for (j = i; j <= NORDER; ++j) {
                if (std::abs(R1) < std::abs(MA((i - 1) * NORDER + j))) {
                    M = j;
                    R1 = MA((i - 1) * NORDER + j);
                }
            }
            if (i != M) {
                K = IVEC(M + 20);
                IVEC(M + 20) = IVEC(i + 20);
                IVEC(i + 20) = K;
                for (j = 1; j <= NORDER; ++j) {
                    S = MA((j - 1) * NORDER + i);
                    MA((j - 1) * NORDER + i) = MA((j - 1) * NORDER + M);
                    MA((j - 1) * NORDER + M) = S;
                }
            }
            MA((i - 1) * NORDER + i) = 1.0;
            for (j = 1; j <= NORDER; ++j) {
                MA((i - 1) * NORDER + j) /= R1;
            }
            for (j = 1; j <= NORDER; ++j) {
                if (i == j) {
                    continue;
                }
                R1 = MA((j - 1) * NORDER + i);
                if (std::abs(R1) <= 1.0E-20) {
                    continue;
                }
                MA((j - 1) * NORDER + i) = 0.0;
                for (K = 1; K <= NORDER; ++K) {
                    MA((j - 1) * NORDER + K) -= R1 * MA((i - 1) * NORDER + K);
                }
            }
        }
        for (i = 1; i <= NORDER; ++i) {
            if (IVEC(i + 20) == i) {
                continue;
            }
            M = i;
            while (NORDER > M) {
                ++M;
                if (IVEC(M + 20) == i) {
                    break;
                }
            }
            IVEC(M + 20) = IVEC(i + 20);
            for (j = 1; j <= NORDER; ++j) {
                R1 = MA((i - 1) * NORDER + j);
                MA((i - 1) * NORDER + j) = MA((M - 1) * NORDER + j);
                MA((M - 1) * NORDER + j) = R1;
            }
            IVEC(i + 20) = i;
        }
    }

    void Solver::report()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   2/1/04
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reports outputs of air distribution systems

        // Using/Aliasing
        auto &NumPrimaryAirSys = m_state.dataHVACGlobal->NumPrimaryAirSys;
        auto &TimeStepSys = m_state.dataHVACGlobal->TimeStepSys;

        auto &Zone(m_state.dataHeatBal->Zone);

        // SUBROUTINE PARAMETER DEFINITIONS:
        constexpr Real64 Lam(2.5e6); // Heat of vaporization (J/kg)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int n;
        int M;
        int ZN1;
        int ZN2;
        Real64 AirDensity;
        Real64 CpAir;
        Real64 Tamb;
        Real64 hg; // latent heat of vaporization
        Real64 ReportingConstant;
        Real64 ReportingFraction;
        int AirLoopNum;
        int FanNum;
        Real64 RepOnOffFanRunTimeFraction;

        if (simulation_control.type == ControlType::NoMultizoneOrDistribution) {
            return;
        }

        if (!onetime) {
            onceZoneFlag.dimension(m_state.dataGlobal->NumOfZones, false);
            onceSurfFlag.dimension(AirflowNetworkNumOfLinks, false);
            onetime = true;
        }
        ReportingConstant = TimeStepSys * Constant::rSecsInHour;

        m_state.dataHeatBal->ZoneTotalExfiltrationHeatLoss = 0.0;

        for (auto &e : AirflowNetworkReportData) {
            e.MultiZoneInfiSenGainW = 0.0;
            e.MultiZoneInfiSenGainJ = 0.0;
            e.MultiZoneInfiSenLossW = 0.0;
            e.MultiZoneInfiSenLossJ = 0.0;
            e.MultiZoneInfiLatGainW = 0.0;
            e.MultiZoneInfiLatGainJ = 0.0;
            e.MultiZoneInfiLatLossW = 0.0;
            e.MultiZoneInfiLatLossJ = 0.0;
            e.MultiZoneVentSenGainW = 0.0;
            e.MultiZoneVentSenGainJ = 0.0;
            e.MultiZoneVentSenLossW = 0.0;
            e.MultiZoneVentSenLossJ = 0.0;
            e.MultiZoneVentLatGainW = 0.0;
            e.MultiZoneVentLatGainJ = 0.0;
            e.MultiZoneVentLatLossW = 0.0;
            e.MultiZoneVentLatLossJ = 0.0;
            e.MultiZoneMixSenGainW = 0.0;
            e.MultiZoneMixSenGainJ = 0.0;
            e.MultiZoneMixSenLossW = 0.0;
            e.MultiZoneMixSenLossJ = 0.0;
            e.MultiZoneMixLatGainW = 0.0;
            e.MultiZoneMixLatGainJ = 0.0;
            e.MultiZoneMixLatLossW = 0.0;
            e.MultiZoneMixLatLossJ = 0.0;
            e.LeakSenGainW = 0.0;
            e.LeakSenGainJ = 0.0;
            e.LeakSenLossW = 0.0;
            e.LeakSenLossJ = 0.0;
            e.LeakLatGainW = 0.0;
            e.LeakLatGainJ = 0.0;
            e.LeakLatLossW = 0.0;
            e.LeakLatLossJ = 0.0;
            e.CondSenGainW = 0.0;
            e.CondSenGainJ = 0.0;
            e.CondSenLossW = 0.0;
            e.CondSenLossJ = 0.0;
            e.DiffLatGainW = 0.0;
            e.DiffLatGainJ = 0.0;
            e.DiffLatLossW = 0.0;
            e.DiffLatLossJ = 0.0;
            e.RadGainW = 0.0;
            e.RadGainJ = 0.0;
            e.RadLossW = 0.0;
            e.RadLossJ = 0.0;
            e.TotalSenGainW = 0.0;
            e.TotalSenGainJ = 0.0;
            e.TotalSenLossW = 0.0;
            e.TotalSenLossJ = 0.0;
            e.TotalLatGainW = 0.0;
            e.TotalLatGainJ = 0.0;
            e.TotalLatLossW = 0.0;
            e.TotalLatLossJ = 0.0;
        }

        // Calculate sensible and latent loads in each zone from multizone airflows
        if (multizone_always_simulated ||
            (simulation_control.type == ControlType::MultizoneWithDistributionOnlyDuringFanOperation && AirflowNetworkFanActivated)) {
            for (i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) { // Multizone airflow energy
                n = AirflowNetworkLinkageData(i).NodeNums[0];
                M = AirflowNetworkLinkageData(i).NodeNums[1];
                ZN1 = AirflowNetworkNodeData(n).EPlusZoneNum;
                ZN2 = AirflowNetworkNodeData(M).EPlusZoneNum;
                // Find a linkage from a zone to outdoors
                if (ZN1 > 0 && ZN2 == 0) {
                    auto &zn1HB = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZN1);
                    if (m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).SurfLinkedOutAirNode > 0) {
                        Tamb = m_state.dataSurface->SurfOutDryBulbTemp(MultizoneSurfaceData(i).SurfNum);
                        CpAir = PsyCpAirFnW(Psychrometrics::PsyWFnTdbTwbPb(m_state,
                                                                           Tamb,
                                                                           m_state.dataSurface->SurfOutWetBulbTemp(MultizoneSurfaceData(i).SurfNum),
                                                                           m_state.dataEnvrn->OutBaroPress));
                    } else {
                        Tamb = Zone(ZN1).OutDryBulbTemp;
                        CpAir = PsyCpAirFnW(m_state.dataEnvrn->OutHumRat);
                    }
                    hg = Psychrometrics::PsyHgAirFnWTdb(zn1HB.airHumRat, zn1HB.MAT);

                    if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::SCR ||
                        AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::SEL) {
                        if (Tamb > zn1HB.MAT) {
                            AirflowNetworkReportData(ZN1).MultiZoneInfiSenGainW += (AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (Tamb - zn1HB.MAT));
                            AirflowNetworkReportData(ZN1).MultiZoneInfiSenGainJ +=
                                (AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (Tamb - zn1HB.MAT)) * ReportingConstant;
                        } else {
                            AirflowNetworkReportData(ZN1).MultiZoneInfiSenLossW += (AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (zn1HB.MAT - Tamb));
                            AirflowNetworkReportData(ZN1).MultiZoneInfiSenLossJ +=
                                (AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (zn1HB.MAT - Tamb)) * ReportingConstant;
                        }
                        if (m_state.dataEnvrn->OutHumRat > zn1HB.airHumRat) {
                            AirflowNetworkReportData(ZN1).MultiZoneInfiLatGainW +=
                                (AirflowNetworkLinkSimu(i).FLOW2 * (m_state.dataEnvrn->OutHumRat - zn1HB.airHumRat)) * hg;
                            AirflowNetworkReportData(ZN1).MultiZoneInfiLatGainJ +=
                                (AirflowNetworkLinkSimu(i).FLOW2 * (m_state.dataEnvrn->OutHumRat - zn1HB.airHumRat)) * hg * ReportingConstant;
                        } else {
                            AirflowNetworkReportData(ZN1).MultiZoneInfiLatLossW +=
                                (AirflowNetworkLinkSimu(i).FLOW2 * (zn1HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * hg;
                            AirflowNetworkReportData(ZN1).MultiZoneInfiLatLossJ +=
                                (AirflowNetworkLinkSimu(i).FLOW2 * (zn1HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * hg * ReportingConstant;
                        }
                    } else {
                        if (Tamb > zn1HB.MAT) {
                            AirflowNetworkReportData(ZN1).MultiZoneVentSenGainW += (AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (Tamb - zn1HB.MAT));
                            AirflowNetworkReportData(ZN1).MultiZoneVentSenGainJ +=
                                (AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (Tamb - zn1HB.MAT)) * ReportingConstant;
                        } else {
                            AirflowNetworkReportData(ZN1).MultiZoneVentSenLossW += (AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (zn1HB.MAT - Tamb));
                            AirflowNetworkReportData(ZN1).MultiZoneVentSenLossJ +=
                                (AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (zn1HB.MAT - Tamb)) * ReportingConstant;
                        }
                        if (m_state.dataEnvrn->OutHumRat > zn1HB.airHumRat) {
                            AirflowNetworkReportData(ZN1).MultiZoneVentLatGainW +=
                                (AirflowNetworkLinkSimu(i).FLOW2 * (m_state.dataEnvrn->OutHumRat - zn1HB.airHumRat)) * hg;
                            AirflowNetworkReportData(ZN1).MultiZoneVentLatGainJ +=
                                (AirflowNetworkLinkSimu(i).FLOW2 * (m_state.dataEnvrn->OutHumRat - zn1HB.airHumRat)) * hg * ReportingConstant;
                        } else {
                            AirflowNetworkReportData(ZN1).MultiZoneVentLatLossW +=
                                (AirflowNetworkLinkSimu(i).FLOW2 * (zn1HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * hg;
                            AirflowNetworkReportData(ZN1).MultiZoneVentLatLossJ +=
                                (AirflowNetworkLinkSimu(i).FLOW2 * (zn1HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * hg * ReportingConstant;
                        }
                    }
                }
                if (ZN1 == 0 && ZN2 > 0) {
                    auto &zn2HB = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZN2);
                    if (m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).SurfLinkedOutAirNode > 0) {
                        Tamb = m_state.dataSurface->SurfOutDryBulbTemp(MultizoneSurfaceData(i).SurfNum);
                        CpAir = PsyCpAirFnW(Psychrometrics::PsyWFnTdbTwbPb(m_state,
                                                                           Tamb,
                                                                           m_state.dataSurface->SurfOutWetBulbTemp(MultizoneSurfaceData(i).SurfNum),
                                                                           m_state.dataEnvrn->OutBaroPress));
                    } else {
                        Tamb = Zone(ZN2).OutDryBulbTemp;
                        CpAir = PsyCpAirFnW(m_state.dataEnvrn->OutHumRat);
                    }
                    hg = Psychrometrics::PsyHgAirFnWTdb(zn2HB.airHumRat, zn2HB.MAT);

                    if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::SCR ||
                        AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::SEL) {
                        if (Tamb > zn2HB.MAT) {
                            AirflowNetworkReportData(ZN2).MultiZoneInfiSenGainW += (AirflowNetworkLinkSimu(i).FLOW * CpAir * (Tamb - zn2HB.MAT));
                            AirflowNetworkReportData(ZN2).MultiZoneInfiSenGainJ +=
                                (AirflowNetworkLinkSimu(i).FLOW * CpAir * (Tamb - zn2HB.MAT)) * ReportingConstant;
                        } else {
                            AirflowNetworkReportData(ZN2).MultiZoneInfiSenLossW += (AirflowNetworkLinkSimu(i).FLOW * CpAir * (zn2HB.MAT - Tamb));
                            AirflowNetworkReportData(ZN2).MultiZoneInfiSenLossJ +=
                                (AirflowNetworkLinkSimu(i).FLOW * CpAir * (zn2HB.MAT - Tamb)) * ReportingConstant;
                        }
                        if (m_state.dataEnvrn->OutHumRat > zn2HB.airHumRat) {
                            AirflowNetworkReportData(ZN2).MultiZoneInfiLatGainW +=
                                (AirflowNetworkLinkSimu(i).FLOW * (m_state.dataEnvrn->OutHumRat - zn2HB.airHumRat)) * hg;
                            AirflowNetworkReportData(ZN2).MultiZoneInfiLatGainJ +=
                                (AirflowNetworkLinkSimu(i).FLOW * (m_state.dataEnvrn->OutHumRat - zn2HB.airHumRat)) * hg * ReportingConstant;
                        } else {
                            AirflowNetworkReportData(ZN2).MultiZoneInfiLatLossW +=
                                (AirflowNetworkLinkSimu(i).FLOW * (zn2HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * hg;
                            AirflowNetworkReportData(ZN2).MultiZoneInfiLatLossJ +=
                                (AirflowNetworkLinkSimu(i).FLOW * (zn2HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * hg * ReportingConstant;
                        }
                    } else {
                        if (Tamb > zn2HB.MAT) {
                            AirflowNetworkReportData(ZN2).MultiZoneVentSenGainW += (AirflowNetworkLinkSimu(i).FLOW * CpAir * (Tamb - zn2HB.MAT));
                            AirflowNetworkReportData(ZN2).MultiZoneVentSenGainJ +=
                                (AirflowNetworkLinkSimu(i).FLOW * CpAir * (Tamb - zn2HB.MAT)) * ReportingConstant;
                        } else {
                            AirflowNetworkReportData(ZN2).MultiZoneVentSenLossW += (AirflowNetworkLinkSimu(i).FLOW * CpAir * (zn2HB.MAT - Tamb));
                            AirflowNetworkReportData(ZN2).MultiZoneVentSenLossJ +=
                                (AirflowNetworkLinkSimu(i).FLOW * CpAir * (zn2HB.MAT - Tamb)) * ReportingConstant;
                        }
                        if (m_state.dataEnvrn->OutHumRat > zn2HB.airHumRat) {
                            AirflowNetworkReportData(ZN2).MultiZoneVentLatGainW +=
                                (AirflowNetworkLinkSimu(i).FLOW * (m_state.dataEnvrn->OutHumRat - zn2HB.airHumRat)) * hg;
                            AirflowNetworkReportData(ZN2).MultiZoneVentLatGainJ +=
                                (AirflowNetworkLinkSimu(i).FLOW * (m_state.dataEnvrn->OutHumRat - zn2HB.airHumRat)) * hg * ReportingConstant;
                        } else {
                            AirflowNetworkReportData(ZN2).MultiZoneVentLatLossW +=
                                (AirflowNetworkLinkSimu(i).FLOW * (zn2HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * hg;
                            AirflowNetworkReportData(ZN2).MultiZoneVentLatLossJ +=
                                (AirflowNetworkLinkSimu(i).FLOW * (zn2HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * hg * ReportingConstant;
                        }
                    }
                }

                if (ZN1 > 0 && ZN2 > 0) {
                    auto const &zn1HB = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZN1);
                    auto const &zn2HB = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZN2);
                    CpAir = PsyCpAirFnW((zn1HB.airHumRat + zn2HB.airHumRat) / 2.0);
                    hg = Psychrometrics::PsyHgAirFnWTdb((zn1HB.airHumRat + zn2HB.airHumRat) / 2.0, (zn1HB.MAT + zn2HB.MAT) / 2.0);
                    if (zn1HB.MAT > zn2HB.MAT) {
                        AirflowNetworkReportData(ZN2).MultiZoneMixSenGainW += (AirflowNetworkLinkSimu(i).FLOW * CpAir * (zn1HB.MAT - zn2HB.MAT));
                        AirflowNetworkReportData(ZN2).MultiZoneMixSenGainJ +=
                            (AirflowNetworkLinkSimu(i).FLOW * CpAir * (zn1HB.MAT - zn2HB.MAT)) * ReportingConstant;
                    } else {
                        AirflowNetworkReportData(ZN2).MultiZoneMixSenLossW += (AirflowNetworkLinkSimu(i).FLOW * CpAir * (zn2HB.MAT - zn1HB.MAT));
                        AirflowNetworkReportData(ZN2).MultiZoneMixSenLossJ +=
                            (AirflowNetworkLinkSimu(i).FLOW * CpAir * (zn2HB.MAT - zn1HB.MAT)) * ReportingConstant;
                    }
                    if (zn1HB.airHumRat > zn2HB.airHumRat) {
                        AirflowNetworkReportData(ZN2).MultiZoneMixLatGainW +=
                            (AirflowNetworkLinkSimu(i).FLOW * (zn1HB.airHumRat - zn2HB.airHumRat)) * hg;
                        AirflowNetworkReportData(ZN2).MultiZoneMixLatGainJ +=
                            (AirflowNetworkLinkSimu(i).FLOW * (zn1HB.airHumRat - zn2HB.airHumRat)) * hg * ReportingConstant;
                    } else {
                        AirflowNetworkReportData(ZN2).MultiZoneMixLatLossW +=
                            (AirflowNetworkLinkSimu(i).FLOW * (zn2HB.airHumRat - zn1HB.airHumRat)) * hg;
                        AirflowNetworkReportData(ZN2).MultiZoneMixLatLossJ +=
                            (AirflowNetworkLinkSimu(i).FLOW * (zn2HB.airHumRat - zn1HB.airHumRat)) * hg * ReportingConstant;
                    }
                    if (zn2HB.MAT > zn1HB.MAT) {
                        AirflowNetworkReportData(ZN1).MultiZoneMixSenGainW += (AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (zn2HB.MAT - zn1HB.MAT));
                        AirflowNetworkReportData(ZN1).MultiZoneMixSenGainJ +=
                            (AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (zn2HB.MAT - zn1HB.MAT)) * ReportingConstant;
                    } else {
                        AirflowNetworkReportData(ZN1).MultiZoneMixSenLossW += (AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (zn1HB.MAT - zn2HB.MAT));
                        AirflowNetworkReportData(ZN1).MultiZoneMixSenLossJ +=
                            (AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (zn1HB.MAT - zn2HB.MAT)) * ReportingConstant;
                    }
                    if (zn2HB.airHumRat > zn1HB.airHumRat) {
                        AirflowNetworkReportData(ZN1).MultiZoneMixLatGainW +=
                            (AirflowNetworkLinkSimu(i).FLOW2 * (zn2HB.airHumRat - zn1HB.airHumRat)) * hg;
                        AirflowNetworkReportData(ZN1).MultiZoneMixLatGainJ +=
                            (AirflowNetworkLinkSimu(i).FLOW2 * (zn2HB.airHumRat - zn1HB.airHumRat)) * hg * ReportingConstant;
                    } else {
                        AirflowNetworkReportData(ZN1).MultiZoneMixLatLossW +=
                            std::abs(AirflowNetworkLinkSimu(i).FLOW2 * (zn1HB.airHumRat - zn2HB.airHumRat)) * hg;
                        AirflowNetworkReportData(ZN1).MultiZoneMixLatLossJ +=
                            (AirflowNetworkLinkSimu(i).FLOW2 * (zn1HB.airHumRat - zn2HB.airHumRat)) * hg * ReportingConstant;
                    }
                }
            }
        }

        // Assign data for report
        if (distribution_simulated) {
            for (i = 1; i <= m_state.dataGlobal->NumOfZones; ++i) {
                if (exchangeData(i).LeakSen > 0.0) {
                    AirflowNetworkReportData(i).LeakSenGainW = exchangeData(i).LeakSen;
                    AirflowNetworkReportData(i).LeakSenGainJ = exchangeData(i).LeakSen * ReportingConstant;
                } else {
                    AirflowNetworkReportData(i).LeakSenLossW = -exchangeData(i).LeakSen;
                    AirflowNetworkReportData(i).LeakSenLossJ = -exchangeData(i).LeakSen * ReportingConstant;
                }
                if (exchangeData(i).LeakLat > 0.0) {
                    AirflowNetworkReportData(i).LeakLatGainW = exchangeData(i).LeakLat * Lam;
                    AirflowNetworkReportData(i).LeakLatGainJ = exchangeData(i).LeakLat * Lam * ReportingConstant;
                } else {
                    AirflowNetworkReportData(i).LeakLatLossW = -exchangeData(i).LeakLat * Lam;
                    AirflowNetworkReportData(i).LeakLatLossJ = -exchangeData(i).LeakLat * Lam * ReportingConstant;
                }
                if (exchangeData(i).CondSen > 0.0) {
                    AirflowNetworkReportData(i).CondSenGainW = exchangeData(i).CondSen;
                    AirflowNetworkReportData(i).CondSenGainJ = exchangeData(i).CondSen * ReportingConstant;
                } else {
                    AirflowNetworkReportData(i).CondSenLossW = -exchangeData(i).CondSen;
                    AirflowNetworkReportData(i).CondSenLossJ = -exchangeData(i).CondSen * ReportingConstant;
                }
                if (exchangeData(i).DiffLat > 0.0) {
                    AirflowNetworkReportData(i).DiffLatGainW = exchangeData(i).DiffLat * Lam;
                    AirflowNetworkReportData(i).DiffLatGainJ = exchangeData(i).DiffLat * Lam * ReportingConstant;
                } else {
                    AirflowNetworkReportData(i).DiffLatLossW = -exchangeData(i).DiffLat * Lam;
                    AirflowNetworkReportData(i).DiffLatLossJ = -exchangeData(i).DiffLat * Lam * ReportingConstant;
                }
                if (exchangeData(i).RadGain < 0.0) {
                    AirflowNetworkReportData(i).RadGainW = -exchangeData(i).RadGain;
                    AirflowNetworkReportData(i).RadGainJ = -exchangeData(i).RadGain * ReportingConstant;
                } else {
                    AirflowNetworkReportData(i).RadLossW = exchangeData(i).RadGain;
                    AirflowNetworkReportData(i).RadLossJ = exchangeData(i).RadGain * ReportingConstant;
                }
                if (exchangeData(i).TotalSen > 0.0) {
                    AirflowNetworkReportData(i).TotalSenGainW = exchangeData(i).TotalSen;
                    AirflowNetworkReportData(i).TotalSenGainJ = exchangeData(i).TotalSen * ReportingConstant;
                } else {
                    AirflowNetworkReportData(i).TotalSenLossW = -exchangeData(i).TotalSen;
                    AirflowNetworkReportData(i).TotalSenLossJ = -exchangeData(i).TotalSen * ReportingConstant;
                }
                if (exchangeData(i).TotalLat > 0.0) {
                    AirflowNetworkReportData(i).TotalLatGainW = exchangeData(i).TotalLat * Lam;
                    AirflowNetworkReportData(i).TotalLatGainJ = exchangeData(i).TotalLat * Lam * ReportingConstant;
                } else {
                    AirflowNetworkReportData(i).TotalLatLossW = -exchangeData(i).TotalLat * Lam;
                    AirflowNetworkReportData(i).TotalLatLossJ = -exchangeData(i).TotalLat * Lam * ReportingConstant;
                }
            }
        }

        // Zone report

        for (auto &e : AirflowNetworkZnRpt) {
            e.InfilVolume = 0.0;
            e.InfilMass = 0.0;
            e.InfilAirChangeRate = 0.0;
            e.VentilVolume = 0.0;
            e.VentilMass = 0.0;
            e.VentilAirChangeRate = 0.0;
            e.MixVolume = 0.0;
            e.MixMass = 0.0;
        }

        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            if (DisSysNumOfCVFs == 0) {
                continue;
            }
            for (FanNum = 1; FanNum <= DisSysNumOfCVFs; ++FanNum) {
                if (DisSysCompCVFData(FanNum).AirLoopNum == AirLoopNum) {
                    break;
                }
            }
            if (DisSysCompCVFData(FanNum).fanType == HVAC::FanType::OnOff && LoopOnOffFanRunTimeFraction(AirLoopNum) < 1.0 &&
                LoopOnOffFanRunTimeFraction(AirLoopNum) > 0.0) {
                // ON Cycle calculation
                onceZoneFlag = false;
                for (i = 1; i <= m_state.dataGlobal->NumOfZones; ++i) {
                    if (AirflowNetworkNodeData(i).AirLoopNum > 0 && AirflowNetworkNodeData(i).AirLoopNum != AirLoopNum) {
                        continue;
                    }
                    if (AirflowNetworkNodeData(i).AirLoopNum == AirLoopNum) {
                        RepOnOffFanRunTimeFraction = LoopOnOffFanRunTimeFraction(AirLoopNum);
                    }
                    if (AirflowNetworkNodeData(i).AirLoopNum == 0) {
                        RepOnOffFanRunTimeFraction = MaxOnOffFanRunTimeFraction;
                    }
                    if (AirflowNetworkNodeData(i).AirLoopNum == 0 && onceZoneFlag(i)) {
                        continue;
                    }
                    AirflowNetworkReportData(i).MultiZoneInfiSenGainW *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneInfiSenGainJ *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneInfiSenLossW *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneInfiSenLossJ *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneInfiLatGainW *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneInfiLatGainJ *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneInfiLatLossW *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneInfiLatLossJ *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneVentSenGainW *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneVentSenGainJ *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneVentSenLossW *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneVentSenLossJ *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneVentLatGainW *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneVentLatGainJ *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneVentLatLossW *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneVentLatLossJ *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneMixSenGainW *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneMixSenGainJ *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneMixSenLossW *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneMixSenLossJ *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneMixLatGainW *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneMixLatGainJ *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneMixLatLossW *= RepOnOffFanRunTimeFraction;
                    AirflowNetworkReportData(i).MultiZoneMixLatLossJ *= RepOnOffFanRunTimeFraction;
                    if (AirflowNetworkNodeData(i).AirLoopNum == 0) {
                        onceZoneFlag(i) = true;
                    }
                }
                // Off Cycle addon
                onceSurfFlag = false;
                for (i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) { // Multizone airflow energy
                    n = AirflowNetworkLinkageData(i).NodeNums[0];
                    M = AirflowNetworkLinkageData(i).NodeNums[1];
                    ZN1 = AirflowNetworkNodeData(n).EPlusZoneNum;
                    ZN2 = AirflowNetworkNodeData(M).EPlusZoneNum;
                    // Find a linkage from a zone to outdoors
                    if (ZN1 > 0 && ZN2 == 0) {
                        auto const &zn1HB = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZN1);
                        if (AirflowNetworkNodeData(n).AirLoopNum > 0 && AirflowNetworkNodeData(n).AirLoopNum != AirLoopNum) {
                            continue;
                        }
                        if (AirflowNetworkNodeData(n).AirLoopNum == AirLoopNum) {
                            RepOnOffFanRunTimeFraction = LoopOnOffFanRunTimeFraction(AirLoopNum);
                        }
                        if (AirflowNetworkNodeData(n).AirLoopNum == 0) {
                            RepOnOffFanRunTimeFraction = MaxOnOffFanRunTimeFraction;
                        }
                        if (AirflowNetworkNodeData(n).AirLoopNum == 0 && onceSurfFlag(i)) {
                            continue;
                        }
                        ReportingFraction = (1.0 - RepOnOffFanRunTimeFraction);
                        Tamb = Zone(ZN1).OutDryBulbTemp;
                        CpAir = PsyCpAirFnW(m_state.dataEnvrn->OutHumRat);
                        if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::SCR ||
                            AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::SEL) {
                            if (Tamb > zn1HB.MAT) {
                                AirflowNetworkReportData(ZN1).MultiZoneInfiSenGainW +=
                                    (linkReport1(i).FLOW2OFF * CpAir * (Tamb - zn1HB.MAT)) * (1.0 - RepOnOffFanRunTimeFraction);
                                AirflowNetworkReportData(ZN1).MultiZoneInfiSenGainJ +=
                                    (linkReport1(i).FLOW2OFF * CpAir * (Tamb - zn1HB.MAT)) * ReportingConstant * ReportingFraction;
                            } else {
                                AirflowNetworkReportData(ZN1).MultiZoneInfiSenLossW +=
                                    (linkReport1(i).FLOW2OFF * CpAir * (zn1HB.MAT - Tamb)) * (1.0 - RepOnOffFanRunTimeFraction);
                                AirflowNetworkReportData(ZN1).MultiZoneInfiSenLossJ +=
                                    (linkReport1(i).FLOW2OFF * CpAir * (zn1HB.MAT - Tamb)) * ReportingConstant * ReportingFraction;
                            }
                            if (m_state.dataEnvrn->OutHumRat > zn1HB.airHumRat) {
                                AirflowNetworkReportData(ZN1).MultiZoneInfiLatGainW +=
                                    (linkReport1(i).FLOW2OFF * (m_state.dataEnvrn->OutHumRat - zn1HB.airHumRat)) * ReportingFraction;
                                AirflowNetworkReportData(ZN1).MultiZoneInfiLatGainJ +=
                                    (linkReport1(i).FLOW2OFF * (m_state.dataEnvrn->OutHumRat - zn1HB.airHumRat)) * ReportingConstant *
                                    ReportingFraction;
                            } else {
                                AirflowNetworkReportData(ZN1).MultiZoneInfiLatLossW +=
                                    (linkReport1(i).FLOW2OFF * (zn1HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * ReportingFraction;
                                AirflowNetworkReportData(ZN1).MultiZoneInfiLatLossJ +=
                                    (linkReport1(i).FLOW2OFF * (zn1HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * ReportingConstant *
                                    ReportingFraction;
                            }
                        } else {
                            if (Tamb > zn1HB.MAT) {
                                AirflowNetworkReportData(ZN1).MultiZoneVentSenGainW +=
                                    (linkReport1(i).FLOW2OFF * CpAir * (Tamb - zn1HB.MAT)) * (1.0 - RepOnOffFanRunTimeFraction);
                                AirflowNetworkReportData(ZN1).MultiZoneVentSenGainJ +=
                                    (linkReport1(i).FLOW2OFF * CpAir * (Tamb - zn1HB.MAT)) * ReportingConstant * ReportingFraction;
                            } else {
                                AirflowNetworkReportData(ZN1).MultiZoneVentSenLossW +=
                                    (linkReport1(i).FLOW2OFF * CpAir * (zn1HB.MAT - Tamb)) * (1.0 - RepOnOffFanRunTimeFraction);
                                AirflowNetworkReportData(ZN1).MultiZoneVentSenLossJ +=
                                    (linkReport1(i).FLOW2OFF * CpAir * (zn1HB.MAT - Tamb)) * ReportingConstant * ReportingFraction;
                            }
                            if (m_state.dataEnvrn->OutHumRat > zn1HB.airHumRat) {
                                AirflowNetworkReportData(ZN1).MultiZoneVentLatGainW +=
                                    (linkReport1(i).FLOW2OFF * (m_state.dataEnvrn->OutHumRat - zn1HB.airHumRat)) * ReportingFraction;
                                AirflowNetworkReportData(ZN1).MultiZoneVentLatGainJ +=
                                    (linkReport1(i).FLOW2OFF * (m_state.dataEnvrn->OutHumRat - zn1HB.airHumRat)) * ReportingConstant *
                                    ReportingFraction;
                            } else {
                                AirflowNetworkReportData(ZN1).MultiZoneVentLatLossW +=
                                    (linkReport1(i).FLOW2OFF * (zn1HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * ReportingFraction;
                                AirflowNetworkReportData(ZN1).MultiZoneVentLatLossJ +=
                                    (linkReport1(i).FLOW2OFF * (zn1HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * ReportingConstant *
                                    ReportingFraction;
                            }
                        }
                        if (AirflowNetworkNodeData(n).AirLoopNum == 0) {
                            onceSurfFlag(i) = true;
                        }
                    }
                    if (ZN1 == 0 && ZN2 > 0) {
                        auto const &zn2HB = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZN2);
                        if (AirflowNetworkNodeData(M).AirLoopNum > 0 && AirflowNetworkNodeData(M).AirLoopNum != AirLoopNum) {
                            continue;
                        }
                        if (AirflowNetworkNodeData(M).AirLoopNum == AirLoopNum) {
                            RepOnOffFanRunTimeFraction = LoopOnOffFanRunTimeFraction(AirLoopNum);
                        }
                        if (AirflowNetworkNodeData(M).AirLoopNum == 0) {
                            RepOnOffFanRunTimeFraction = MaxOnOffFanRunTimeFraction;
                        }
                        if (AirflowNetworkNodeData(M).AirLoopNum == 0 && onceSurfFlag(i)) {
                            continue;
                        }
                        ReportingFraction = (1.0 - RepOnOffFanRunTimeFraction);
                        Tamb = Zone(ZN2).OutDryBulbTemp;
                        CpAir = PsyCpAirFnW(m_state.dataEnvrn->OutHumRat);
                        if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::SCR ||
                            AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::SEL) {
                            if (Tamb > zn2HB.MAT) {
                                AirflowNetworkReportData(ZN2).MultiZoneInfiSenGainW +=
                                    (linkReport1(i).FLOWOFF * CpAir * (Tamb - zn2HB.MAT)) * ReportingFraction;
                                AirflowNetworkReportData(ZN2).MultiZoneInfiSenGainJ +=
                                    (linkReport1(i).FLOWOFF * CpAir * (Tamb - zn2HB.MAT)) * ReportingConstant * ReportingFraction;
                            } else {
                                AirflowNetworkReportData(ZN2).MultiZoneInfiSenLossW +=
                                    (linkReport1(i).FLOWOFF * CpAir * (zn2HB.MAT - Tamb)) * ReportingFraction;
                                AirflowNetworkReportData(ZN2).MultiZoneInfiSenLossJ +=
                                    (linkReport1(i).FLOWOFF * CpAir * (zn2HB.MAT - Tamb)) * ReportingConstant * ReportingFraction;
                            }
                            if (m_state.dataEnvrn->OutHumRat > zn2HB.airHumRat) {
                                AirflowNetworkReportData(ZN2).MultiZoneInfiLatGainW +=
                                    (linkReport1(i).FLOWOFF * (m_state.dataEnvrn->OutHumRat - zn2HB.airHumRat)) * ReportingFraction;
                                AirflowNetworkReportData(ZN2).MultiZoneInfiLatGainJ +=
                                    (linkReport1(i).FLOWOFF * (m_state.dataEnvrn->OutHumRat - zn2HB.airHumRat)) * ReportingConstant *
                                    ReportingFraction;
                            } else {
                                AirflowNetworkReportData(ZN2).MultiZoneInfiLatLossW +=
                                    (linkReport1(i).FLOWOFF * (zn2HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * ReportingFraction;
                                AirflowNetworkReportData(ZN2).MultiZoneInfiLatLossJ +=
                                    (linkReport1(i).FLOWOFF * (zn2HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * ReportingConstant *
                                    ReportingFraction;
                            }
                        } else {
                            if (Tamb > zn2HB.MAT) {
                                AirflowNetworkReportData(ZN2).MultiZoneVentSenGainW +=
                                    (linkReport1(i).FLOWOFF * CpAir * (Tamb - zn2HB.MAT)) * ReportingFraction;
                                AirflowNetworkReportData(ZN2).MultiZoneVentSenGainJ +=
                                    (linkReport1(i).FLOWOFF * CpAir * (Tamb - zn2HB.MAT)) * ReportingConstant * ReportingFraction;
                            } else {
                                AirflowNetworkReportData(ZN2).MultiZoneVentSenLossW +=
                                    (linkReport1(i).FLOWOFF * CpAir * (zn2HB.MAT - Tamb)) * ReportingFraction;
                                AirflowNetworkReportData(ZN2).MultiZoneVentSenLossJ +=
                                    (linkReport1(i).FLOWOFF * CpAir * (zn2HB.MAT - Tamb)) * ReportingConstant * ReportingFraction;
                            }
                            if (m_state.dataEnvrn->OutHumRat > zn2HB.airHumRat) {
                                AirflowNetworkReportData(ZN2).MultiZoneVentLatGainW +=
                                    (linkReport1(i).FLOWOFF * (m_state.dataEnvrn->OutHumRat - zn2HB.airHumRat)) * ReportingFraction;
                                AirflowNetworkReportData(ZN2).MultiZoneVentLatGainJ +=
                                    (linkReport1(i).FLOWOFF * (m_state.dataEnvrn->OutHumRat - zn2HB.airHumRat)) * ReportingConstant *
                                    ReportingFraction;
                            } else {
                                AirflowNetworkReportData(ZN2).MultiZoneVentLatLossW +=
                                    (linkReport1(i).FLOWOFF * (zn2HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * ReportingFraction;
                                AirflowNetworkReportData(ZN2).MultiZoneVentLatLossJ +=
                                    (linkReport1(i).FLOWOFF * (zn2HB.airHumRat - m_state.dataEnvrn->OutHumRat)) * ReportingConstant *
                                    ReportingFraction;
                            }
                        }
                        if (AirflowNetworkNodeData(M).AirLoopNum == 0) {
                            onceSurfFlag(i) = true;
                        }
                    }

                    if (ZN1 > 0 && ZN2 > 0) {
                        auto &zn1HB = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZN1);
                        auto &zn2HB = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZN2);
                        ReportingFraction = (1.0 - MaxOnOffFanRunTimeFraction);
                        CpAir = PsyCpAirFnW(zn1HB.airHumRat);
                        if (zn1HB.MAT > zn2HB.MAT) {
                            AirflowNetworkReportData(ZN2).MultiZoneMixSenGainW +=
                                (linkReport1(i).FLOWOFF * CpAir * (zn1HB.MAT - zn2HB.MAT)) * ReportingFraction;
                            AirflowNetworkReportData(ZN2).MultiZoneMixSenGainJ +=
                                (linkReport1(i).FLOWOFF * CpAir * (zn1HB.MAT - zn2HB.MAT)) * ReportingConstant * ReportingFraction;
                        } else {
                            AirflowNetworkReportData(ZN2).MultiZoneMixSenLossW +=
                                (linkReport1(i).FLOWOFF * CpAir * (zn2HB.MAT - zn1HB.MAT)) * ReportingFraction;
                            AirflowNetworkReportData(ZN2).MultiZoneMixSenLossJ +=
                                (linkReport1(i).FLOWOFF * CpAir * (zn2HB.MAT - zn1HB.MAT)) * ReportingConstant * ReportingFraction;
                        }
                        if (zn1HB.airHumRat > zn2HB.airHumRat) {
                            AirflowNetworkReportData(ZN2).MultiZoneMixLatGainW +=
                                (linkReport1(i).FLOWOFF * (zn1HB.airHumRat - zn2HB.airHumRat)) * ReportingFraction;
                            AirflowNetworkReportData(ZN2).MultiZoneMixLatGainJ +=
                                (linkReport1(i).FLOWOFF * (zn1HB.airHumRat - zn2HB.airHumRat)) * ReportingConstant * ReportingFraction;
                        } else {
                            AirflowNetworkReportData(ZN2).MultiZoneMixLatLossW +=
                                (linkReport1(i).FLOWOFF * (zn2HB.airHumRat - zn1HB.airHumRat)) * ReportingFraction;
                            AirflowNetworkReportData(ZN2).MultiZoneMixLatLossJ +=
                                (linkReport1(i).FLOWOFF * (zn2HB.airHumRat - zn1HB.airHumRat)) * ReportingConstant * ReportingFraction;
                        }
                        CpAir = PsyCpAirFnW(zn2HB.airHumRat);
                        if (zn2HB.MAT > zn1HB.MAT) {
                            AirflowNetworkReportData(ZN1).MultiZoneMixSenGainW +=
                                (linkReport1(i).FLOW2OFF * CpAir * (zn2HB.MAT - zn1HB.MAT)) * ReportingFraction;
                            AirflowNetworkReportData(ZN1).MultiZoneMixSenGainJ +=
                                (linkReport1(i).FLOW2OFF * CpAir * (zn2HB.MAT - zn1HB.MAT)) * ReportingConstant * ReportingFraction;
                        } else {
                            AirflowNetworkReportData(ZN1).MultiZoneMixSenLossW +=
                                (linkReport1(i).FLOW2OFF * CpAir * (zn1HB.MAT - zn2HB.MAT)) * ReportingFraction;
                            AirflowNetworkReportData(ZN1).MultiZoneMixSenLossJ +=
                                (linkReport1(i).FLOW2OFF * CpAir * (zn1HB.MAT - zn2HB.MAT)) * ReportingConstant * ReportingFraction;
                        }

                        if (zn2HB.airHumRat > zn1HB.airHumRat) {
                            AirflowNetworkReportData(ZN1).MultiZoneMixLatGainW +=
                                (linkReport1(i).FLOW2OFF * (zn2HB.airHumRat - zn1HB.airHumRat)) * ReportingFraction;
                            AirflowNetworkReportData(ZN1).MultiZoneMixLatGainJ +=
                                (linkReport1(i).FLOW2OFF * (zn2HB.airHumRat - zn1HB.airHumRat)) * ReportingConstant * ReportingFraction;
                        } else {
                            AirflowNetworkReportData(ZN1).MultiZoneMixLatLossW +=
                                std::abs(linkReport1(i).FLOW2OFF * (zn1HB.airHumRat - zn2HB.airHumRat)) * ReportingFraction;
                            AirflowNetworkReportData(ZN1).MultiZoneMixLatLossJ +=
                                (linkReport1(i).FLOW2OFF * (zn1HB.airHumRat - zn2HB.airHumRat)) * ReportingConstant * ReportingFraction;
                        }
                    }
                }
            }
        }

        if (!multizone_always_simulated) {
            return;
        }

        for (i = 1; i <= m_state.dataGlobal->NumOfZones; ++i) { // Start of zone loads report variable update loop ...
            auto &thisZoneHB = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(i);
            Tamb = Zone(i).OutDryBulbTemp;
            CpAir = PsyCpAirFnW(thisZoneHB.airHumRatAvg);
            AirDensity = PsyRhoAirFnPbTdbW(m_state, m_state.dataEnvrn->OutBaroPress, thisZoneHB.MAT, thisZoneHB.airHumRatAvg);

            AirflowNetworkZnRpt(i).InfilMass = (exchangeData(i).SumMCp / CpAir) * ReportingConstant;
            AirflowNetworkZnRpt(i).InfilVolume = AirflowNetworkZnRpt(i).InfilMass / AirDensity;
            AirflowNetworkZnRpt(i).InfilAirChangeRate = AirflowNetworkZnRpt(i).InfilVolume / (TimeStepSys * Zone(i).Volume);
            AirflowNetworkZnRpt(i).VentilMass = (exchangeData(i).SumMVCp / CpAir) * ReportingConstant;
            AirflowNetworkZnRpt(i).VentilVolume = AirflowNetworkZnRpt(i).VentilMass / AirDensity;
            AirflowNetworkZnRpt(i).VentilAirChangeRate = AirflowNetworkZnRpt(i).VentilVolume / (TimeStepSys * Zone(i).Volume);
            AirflowNetworkZnRpt(i).MixMass = (exchangeData(i).SumMMCp / CpAir) * ReportingConstant;
            AirflowNetworkZnRpt(i).MixVolume = AirflowNetworkZnRpt(i).MixMass / AirDensity;
            // save values for predefined report
            Real64 stdDensAFNInfilVolume = AirflowNetworkZnRpt(i).InfilMass / m_state.dataEnvrn->StdRhoAir;
            Real64 stdDensAFNNatVentVolume = AirflowNetworkZnRpt(i).VentilMass / m_state.dataEnvrn->StdRhoAir;
            m_state.dataHeatBal->ZonePreDefRep(i).AFNVentVolStdDen = stdDensAFNNatVentVolume;
            m_state.dataHeatBal->ZonePreDefRep(i).AFNVentVolTotalStdDen += stdDensAFNNatVentVolume;
            m_state.dataHeatBal->ZonePreDefRep(i).AFNInfilVolTotalStdDen += stdDensAFNInfilVolume;
            if (m_state.dataHeatBal->ZonePreDefRep(i).isOccupied) {
                m_state.dataHeatBal->ZonePreDefRep(i).AFNVentVolTotalOccStdDen += stdDensAFNNatVentVolume;
                m_state.dataHeatBal->ZonePreDefRep(i).AFNInfilVolTotalOccStdDen += stdDensAFNInfilVolume;
                m_state.dataHeatBal->ZonePreDefRep(i).AFNInfilVolTotalOcc +=
                    (AirflowNetworkZnRpt(i).InfilVolume + AirflowNetworkZnRpt(i).VentilVolume) * Zone(i).Multiplier * Zone(i).ListMultiplier;
                if ((AirflowNetworkZnRpt(i).InfilVolume + AirflowNetworkZnRpt(i).VentilVolume) <
                    m_state.dataHeatBal->ZonePreDefRep(i).AFNInfilVolMin) {
                    m_state.dataHeatBal->ZonePreDefRep(i).AFNInfilVolMin =
                        (AirflowNetworkZnRpt(i).InfilVolume + AirflowNetworkZnRpt(i).VentilVolume) * Zone(i).Multiplier * Zone(i).ListMultiplier;
                }
            }

            Real64 H2OHtOfVap = Psychrometrics::PsyHgAirFnWTdb(m_state.dataEnvrn->OutHumRat, Zone(i).OutDryBulbTemp);
            AirflowNetworkZnRpt(i).InletMass = 0;
            AirflowNetworkZnRpt(i).OutletMass = 0;
            if (m_state.dataZoneEquip->ZoneEquipConfig(i).IsControlled) {
                for (int j = 1; j <= m_state.dataZoneEquip->ZoneEquipConfig(i).NumInletNodes; ++j) {
                    AirflowNetworkZnRpt(i).InletMass +=
                        m_state.dataLoopNodes->Node(m_state.dataZoneEquip->ZoneEquipConfig(i).InletNode(j)).MassFlowRate * ReportingConstant;
                }
                for (int j = 1; j <= m_state.dataZoneEquip->ZoneEquipConfig(i).NumExhaustNodes; ++j) {
                    AirflowNetworkZnRpt(i).OutletMass +=
                        m_state.dataLoopNodes->Node(m_state.dataZoneEquip->ZoneEquipConfig(i).ExhaustNode(j)).MassFlowRate * ReportingConstant;
                }
                for (int j = 1; j <= m_state.dataZoneEquip->ZoneEquipConfig(i).NumReturnNodes; ++j) {
                    AirflowNetworkZnRpt(i).OutletMass +=
                        m_state.dataLoopNodes->Node(m_state.dataZoneEquip->ZoneEquipConfig(i).ReturnNode(j)).MassFlowRate * ReportingConstant;
                }
            }
            AirflowNetworkZnRpt(i).ExfilMass = AirflowNetworkZnRpt(i).InfilMass + AirflowNetworkZnRpt(i).VentilMass + AirflowNetworkZnRpt(i).MixMass +
                                               AirflowNetworkZnRpt(i).InletMass - AirflowNetworkZnRpt(i).OutletMass;
            AirflowNetworkZnRpt(i).ExfilSensiLoss = AirflowNetworkZnRpt(i).ExfilMass / ReportingConstant * (thisZoneHB.MAT - Tamb) * CpAir;
            AirflowNetworkZnRpt(i).ExfilLatentLoss =
                AirflowNetworkZnRpt(i).ExfilMass / ReportingConstant * (thisZoneHB.airHumRat - m_state.dataEnvrn->OutHumRat) * H2OHtOfVap;
            AirflowNetworkZnRpt(i).ExfilTotalLoss = AirflowNetworkZnRpt(i).ExfilSensiLoss + AirflowNetworkZnRpt(i).ExfilLatentLoss;

            m_state.dataHeatBal->ZoneTotalExfiltrationHeatLoss += AirflowNetworkZnRpt(i).ExfilTotalLoss * ReportingConstant;
        } // ... end of zone loads report variable update loop.

        // Rewrite AirflowNetwork airflow rate
        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            if (DisSysNumOfCVFs == 0) {
                continue;
            }
            for (FanNum = 1; FanNum <= DisSysNumOfCVFs; ++FanNum) {
                if (DisSysCompCVFData(FanNum).AirLoopNum == AirLoopNum) {
                    break;
                }
            }
            onceSurfFlag = false;

            for (i = 1; i <= NumOfLinksMultiZone; ++i) {
                if (onceSurfFlag(i)) {
                    continue;
                }
                if (DisSysCompCVFData(FanNum).AirLoopNum == AirLoopNum) {
                    Tamb = OutDryBulbTempAt(m_state, AirflowNetworkLinkageData(i).NodeHeights[0]);
                    AirDensity = PsyRhoAirFnPbTdbW(m_state, m_state.dataEnvrn->OutBaroPress, Tamb, m_state.dataEnvrn->OutHumRat);
                    if (DisSysCompCVFData(FanNum).fanType == HVAC::FanType::OnOff && LoopOnOffFanRunTimeFraction(AirLoopNum) < 1.0 &&
                        LoopOnOffFanRunTimeFraction(AirLoopNum) > 0.0) {
                        linkReport(i).VolFLOW = linkReport1(i).FLOW / AirDensity;
                        linkReport(i).VolFLOW2 = linkReport1(i).FLOW2 / AirDensity;
                    } else {
                        linkReport(i).VolFLOW = linkReport(i).FLOW / AirDensity;
                        linkReport(i).VolFLOW2 = linkReport(i).FLOW2 / AirDensity;
                    }
                    onceSurfFlag(i) = true;
                }
            }

            if (AirflowNetworkNumOfLinks > NumOfLinksMultiZone) {
                for (i = NumOfLinksMultiZone + 1; i <= AirflowNetworkNumOfLinks; ++i) {
                    if (onceSurfFlag(i)) {
                        continue;
                    }
                    if (DisSysCompCVFData(FanNum).AirLoopNum == AirLoopNum) {
                        n = AirflowNetworkLinkageData(i).NodeNums[0];
                        M = AirflowNetworkLinkageData(i).NodeNums[1];
                        AirDensity =
                            PsyRhoAirFnPbTdbW(m_state,
                                              (AirflowNetworkNodeSimu(n).PZ + AirflowNetworkNodeSimu(M).PZ) / 2.0 + m_state.dataEnvrn->OutBaroPress,
                                              (AirflowNetworkNodeSimu(n).TZ + AirflowNetworkNodeSimu(M).TZ) / 2.0,
                                              (AirflowNetworkNodeSimu(n).WZ + AirflowNetworkNodeSimu(M).WZ) / 2.0);
                        if (DisSysCompCVFData(FanNum).fanType == HVAC::FanType::OnOff && LoopOnOffFanRunTimeFraction(AirLoopNum) < 1.0 &&
                            LoopOnOffFanRunTimeFraction(AirLoopNum) > 0.0) {
                            linkReport(i).VolFLOW = linkReport(i).FLOW / AirDensity * (1.0 - LoopOnOffFanRunTimeFraction(AirLoopNum));
                            linkReport(i).VolFLOW2 = linkReport(i).FLOW2 / AirDensity * (1.0 - LoopOnOffFanRunTimeFraction(AirLoopNum));
                            onceSurfFlag(i) = true;
                        } else {
                            linkReport(i).VolFLOW = linkReport(i).FLOW / AirDensity;
                            linkReport(i).VolFLOW2 = linkReport(i).FLOW2 / AirDensity;
                        }
                    }
                }
            }
        }
    }

    void Solver::update(ObjexxFCL::Optional_bool_const FirstHVACIteration) // True when solution technique on first iteration
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   12/10/05
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine update variables used in the AirflowNetwork model.

        // Using/Aliasing
        auto &NumPrimaryAirSys = m_state.dataHVACGlobal->NumPrimaryAirSys;
        using HVAC::VerySmallMassFlow;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int n;
        int M;
        int ZN1;
        int ZN2;
        int Node1;
        int Node2;
        Real64 CpAir;
        Real64 Qsen;
        Real64 Qlat;
        Real64 AirDensity;
        Real64 Tamb;
        Real64 PartLoadRatio;
        Real64 OnOffRatio;
        Real64 NodeMass;
        Real64 AFNMass;

        auto &Zone(m_state.dataHeatBal->Zone);
        auto &Node(m_state.dataLoopNodes->Node);

        for (auto &e : exchangeData) {
            e.SumMCp = 0.0;
            e.SumMCpT = 0.0;
            e.SumMVCp = 0.0;
            e.SumMVCpT = 0.0;
            e.SumMHr = 0.0;
            e.SumMHrW = 0.0;
            e.SumMMCp = 0.0;
            e.SumMMCpT = 0.0;
            e.SumMMHr = 0.0;
            e.SumMMHrW = 0.0;
        }
        if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
            for (auto &e : exchangeData) {
                e.SumMHrCO = 0.0;
                e.SumMMHrCO = 0.0;
            }
        }
        if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            for (auto &e : exchangeData) {
                e.SumMHrGC = 0.0;
                e.SumMMHrGC = 0.0;
            }
        }

        // Calculate sensible and latent loads in each zone from multizone airflows
        if (multizone_always_simulated ||
            (simulation_control.type == ControlType::MultizoneWithDistributionOnlyDuringFanOperation && AirflowNetworkFanActivated)) {
            for (int i = 1; i <= NumOfLinksMultiZone; ++i) { // Multizone airflow energy
                n = AirflowNetworkLinkageData(i).NodeNums[0];
                M = AirflowNetworkLinkageData(i).NodeNums[1];
                ZN1 = AirflowNetworkNodeData(n).EPlusZoneNum;
                ZN2 = AirflowNetworkNodeData(M).EPlusZoneNum;
                if (ZN1 > 0 && ZN2 == 0) {
                    // Find a linkage from outdoors to this zone
                    Tamb = Zone(ZN1).OutDryBulbTemp;
                    CpAir = PsyCpAirFnW(m_state.dataEnvrn->OutHumRat);
                    if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::SCR ||
                        AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::SEL) {
                        exchangeData(ZN1).SumMCp += AirflowNetworkLinkSimu(i).FLOW2 * CpAir;
                        exchangeData(ZN1).SumMCpT += AirflowNetworkLinkSimu(i).FLOW2 * CpAir * Tamb;
                    } else {
                        exchangeData(ZN1).SumMVCp += AirflowNetworkLinkSimu(i).FLOW2 * CpAir;
                        exchangeData(ZN1).SumMVCpT += AirflowNetworkLinkSimu(i).FLOW2 * CpAir * Tamb;
                    }
                    exchangeData(ZN1).SumMHr += AirflowNetworkLinkSimu(i).FLOW2;
                    exchangeData(ZN1).SumMHrW += AirflowNetworkLinkSimu(i).FLOW2 * m_state.dataEnvrn->OutHumRat;
                    if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        exchangeData(ZN1).SumMHrCO += AirflowNetworkLinkSimu(i).FLOW2 * m_state.dataContaminantBalance->OutdoorCO2;
                    }
                    if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        exchangeData(ZN1).SumMHrGC += AirflowNetworkLinkSimu(i).FLOW2 * m_state.dataContaminantBalance->OutdoorGC;
                    }
                }
                if (ZN1 == 0 && ZN2 > 0) {
                    // Find a linkage from outdoors to this zone
                    Tamb = Zone(ZN2).OutDryBulbTemp;
                    CpAir = PsyCpAirFnW(m_state.dataEnvrn->OutHumRat);
                    if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::SCR ||
                        AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::SEL) {
                        exchangeData(ZN2).SumMCp += AirflowNetworkLinkSimu(i).FLOW * CpAir;
                        exchangeData(ZN2).SumMCpT += AirflowNetworkLinkSimu(i).FLOW * CpAir * Tamb;
                    } else {
                        exchangeData(ZN2).SumMVCp += AirflowNetworkLinkSimu(i).FLOW * CpAir;
                        exchangeData(ZN2).SumMVCpT += AirflowNetworkLinkSimu(i).FLOW * CpAir * Tamb;
                    }
                    exchangeData(ZN2).SumMHr += AirflowNetworkLinkSimu(i).FLOW;
                    exchangeData(ZN2).SumMHrW += AirflowNetworkLinkSimu(i).FLOW * m_state.dataEnvrn->OutHumRat;
                    if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        exchangeData(ZN2).SumMHrCO += AirflowNetworkLinkSimu(i).FLOW * m_state.dataContaminantBalance->OutdoorCO2;
                    }
                    if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        exchangeData(ZN2).SumMHrGC += AirflowNetworkLinkSimu(i).FLOW * m_state.dataContaminantBalance->OutdoorGC;
                    }
                }
                if (ZN1 > 0 && ZN2 > 0) {
                    // Find a linkage from outdoors to this zone
                    CpAir = PsyCpAirFnW(ANZW(ZN1));
                    exchangeData(ZN2).SumMMCp += AirflowNetworkLinkSimu(i).FLOW * CpAir;
                    exchangeData(ZN2).SumMMCpT += AirflowNetworkLinkSimu(i).FLOW * CpAir * ANZT(ZN1);
                    exchangeData(ZN2).SumMMHr += AirflowNetworkLinkSimu(i).FLOW;
                    exchangeData(ZN2).SumMMHrW += AirflowNetworkLinkSimu(i).FLOW * ANZW(ZN1);
                    if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        exchangeData(ZN2).SumMMHrCO += AirflowNetworkLinkSimu(i).FLOW * ANCO(ZN1);
                    }
                    if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        exchangeData(ZN2).SumMMHrGC += AirflowNetworkLinkSimu(i).FLOW * ANGC(ZN1);
                    }
                    CpAir = PsyCpAirFnW(ANZW(ZN2));
                    exchangeData(ZN1).SumMMCp += AirflowNetworkLinkSimu(i).FLOW2 * CpAir;
                    exchangeData(ZN1).SumMMCpT += AirflowNetworkLinkSimu(i).FLOW2 * CpAir * ANZT(ZN2);
                    exchangeData(ZN1).SumMMHr += AirflowNetworkLinkSimu(i).FLOW2;
                    exchangeData(ZN1).SumMMHrW += AirflowNetworkLinkSimu(i).FLOW2 * ANZW(ZN2);
                    if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        exchangeData(ZN1).SumMMHrCO += AirflowNetworkLinkSimu(i).FLOW2 * ANCO(ZN2);
                    }
                    if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        exchangeData(ZN1).SumMMHrGC += AirflowNetworkLinkSimu(i).FLOW2 * ANGC(ZN2);
                    }
                }
            }
        }
        // End of update of multizone airflow calculations

        // Initialize these values
        for (auto &e : exchangeData) {
            e.LeakSen = 0.0;
            e.CondSen = 0.0;
            e.LeakLat = 0.0;
            e.DiffLat = 0.0;
            e.MultiZoneSen = 0.0;
            e.MultiZoneLat = 0.0;
            e.RadGain = 0.0;
        }

        // Rewrite AirflowNetwork airflow rate
        for (int i = 1; i <= NumOfLinksMultiZone; ++i) {
            Tamb = OutDryBulbTempAt(m_state, AirflowNetworkLinkageData(i).NodeHeights[0]);
            AirDensity = PsyRhoAirFnPbTdbW(m_state, m_state.dataEnvrn->OutBaroPress, Tamb, m_state.dataEnvrn->OutHumRat);
            AirflowNetworkLinkSimu(i).VolFLOW = AirflowNetworkLinkSimu(i).FLOW / AirDensity;
            AirflowNetworkLinkSimu(i).VolFLOW2 = AirflowNetworkLinkSimu(i).FLOW2 / AirDensity;
        }

        for (std::size_t i = 0; i < linkReport.size(); ++i) {
            auto &r = linkReport[i];
            auto const &s = AirflowNetworkLinkSimu[i];
            r.FLOW = s.FLOW;
            r.FLOW2 = s.FLOW2;
            r.VolFLOW = s.VolFLOW;
            r.VolFLOW2 = s.VolFLOW2;
        }

        // Save zone loads from multizone calculation for later summation
        bool OnOffFanFlag = false;
        for (int i = 1; i <= DisSysNumOfCVFs; i++) {
            if (DisSysCompCVFData(i).fanType == HVAC::FanType::OnOff) {
                OnOffFanFlag = true;
                break;
            }
        }
        if (present(FirstHVACIteration)) {
            if (FirstHVACIteration && OnOffFanFlag) {
                multiExchangeData = exchangeData;
                for (int i = 1; i <= AirflowNetworkNumOfZones; ++i) {
                    nodeReport(i).PZ = AirflowNetworkNodeSimu(i).PZ;
                    nodeReport(i).PZOFF = AirflowNetworkNodeSimu(i).PZ;
                    nodeReport(i).PZON = 0.0;
                }
                for (int i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
                    linkReport1(i).FLOW = AirflowNetworkLinkSimu(i).FLOW;
                    linkReport1(i).FLOW2 = AirflowNetworkLinkSimu(i).FLOW2;
                    linkReport1(i).VolFLOW = AirflowNetworkLinkSimu(i).VolFLOW;
                    linkReport1(i).VolFLOW2 = AirflowNetworkLinkSimu(i).VolFLOW2;
                    linkReport1(i).FLOWOFF = AirflowNetworkLinkSimu(i).FLOW;
                    linkReport1(i).FLOW2OFF = AirflowNetworkLinkSimu(i).FLOW2;
                    linkReport1(i).VolFLOWOFF = AirflowNetworkLinkSimu(i).VolFLOW;
                    linkReport1(i).VolFLOW2OFF = AirflowNetworkLinkSimu(i).VolFLOW2;
                    linkReport1(i).DP = AirflowNetworkLinkSimu(i).DP;
                    linkReport1(i).DPOFF = AirflowNetworkLinkSimu(i).DP;
                    linkReport1(i).DPON = 0.0;
                }
            }
        }

        if (!AirflowNetworkFanActivated && distribution_simulated) {
            for (int i = NumOfNodesMultiZone + NumOfNodesIntraZone + 1; i <= AirflowNetworkNumOfNodes; ++i) {
                AirflowNetworkNodeSimu(i).PZ = 0.0;
            }
            for (int i = AirflowNetworkNumOfSurfaces + 1; i <= AirflowNetworkNumOfLinks; ++i) {
                AirflowNetworkLinkSimu(i).DP = 0.0;
                linkReport(i).FLOW = 0.0;
                linkReport(i).FLOW2 = 0.0;
                linkReport(i).VolFLOW = 0.0;
                linkReport(i).VolFLOW2 = 0.0;
            }
        }

        if (!(AirflowNetworkFanActivated && distribution_simulated)) {
            return;
        }

        if (distribution_simulated) {
            for (int i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) { // Multizone airflow energy
                n = AirflowNetworkLinkageData(i).NodeNums[0];
                M = AirflowNetworkLinkageData(i).NodeNums[1];
                ZN1 = AirflowNetworkNodeData(n).EPlusZoneNum;
                ZN2 = AirflowNetworkNodeData(M).EPlusZoneNum;
                // Find a linkage from a zone to outdoors
                if (ZN1 > 0 && ZN2 == 0) {
                    Tamb = Zone(ZN1).OutDryBulbTemp;
                    CpAir = PsyCpAirFnW(m_state.dataEnvrn->OutHumRat);
                    exchangeData(ZN1).MultiZoneSen += AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (Tamb - ANZT(ZN1));
                    exchangeData(ZN1).MultiZoneLat += AirflowNetworkLinkSimu(i).FLOW2 * (m_state.dataEnvrn->OutHumRat - ANZW(ZN1));
                }
                if (ZN1 == 0 && ZN2 > 0) {
                    Tamb = Zone(ZN2).OutDryBulbTemp;
                    CpAir = PsyCpAirFnW(m_state.dataEnvrn->OutHumRat);
                    exchangeData(ZN2).MultiZoneSen += AirflowNetworkLinkSimu(i).FLOW * CpAir * (Tamb - ANZT(ZN2));
                    exchangeData(ZN2).MultiZoneLat += AirflowNetworkLinkSimu(i).FLOW * (m_state.dataEnvrn->OutHumRat - ANZW(ZN2));
                }

                if (ZN1 > 0 && ZN2 > 0) {
                    if (AirflowNetworkLinkSimu(i).FLOW > 0) { // Flow from ZN1 to ZN2
                        CpAir = PsyCpAirFnW(ANZW(ZN1));
                        exchangeData(ZN2).MultiZoneSen += AirflowNetworkLinkSimu(i).FLOW * CpAir * (ANZT(ZN1) - ANZT(ZN2));
                        exchangeData(ZN2).MultiZoneLat += AirflowNetworkLinkSimu(i).FLOW * (ANZW(ZN1) - ANZW(ZN2));
                        CpAir = PsyCpAirFnW(ANZW(ZN2));
                        exchangeData(ZN1).MultiZoneSen += std::abs(AirflowNetworkLinkSimu(i).FLOW2) * CpAir * (ANZT(ZN2) - ANZT(ZN1));
                        exchangeData(ZN1).MultiZoneLat += std::abs(AirflowNetworkLinkSimu(i).FLOW2) * (ANZW(ZN2) - ANZW(ZN1));
                    } else {
                        CpAir = PsyCpAirFnW(ANZW(ZN2));
                        exchangeData(ZN1).MultiZoneSen += std::abs(AirflowNetworkLinkSimu(i).FLOW2) * CpAir * (ANZT(ZN2) - ANZT(ZN1));
                        exchangeData(ZN1).MultiZoneLat += std::abs(AirflowNetworkLinkSimu(i).FLOW2) * (ANZW(ZN2) - ANZW(ZN1));
                    }
                }
            }
        }

        int AirLoopNum;
        int FanNum;
        Real64 MaxPartLoadRatio = 0.0;
        Real64 OnOffFanRunTimeFraction = 0.0;
        MaxOnOffFanRunTimeFraction = 0.0;
        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            MaxPartLoadRatio = max(MaxPartLoadRatio, m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio);
            MaxOnOffFanRunTimeFraction = max(MaxOnOffFanRunTimeFraction, LoopOnOffFanRunTimeFraction(AirLoopNum));
        }
        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            for (FanNum = 1; FanNum <= DisSysNumOfCVFs; ++FanNum) {
                if (DisSysCompCVFData(FanNum).AirLoopNum == AirLoopNum) {
                    break;
                }
            }
            PartLoadRatio = 1.0;
            LoopPartLoadRatio(AirLoopNum) = 1.0;
            OnOffFanRunTimeFraction = 1.0;
            LoopOnOffFanRunTimeFraction(AirLoopNum) = 1.0;
            // Calculate the part load ratio, can't be greater than 1 for a simple ONOFF fan
            if (DisSysCompCVFData(FanNum).fanType == HVAC::FanType::OnOff &&
                Node(DisSysCompCVFData(FanNum).InletNode).MassFlowRate > VerySmallMassFlow &&
                m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode == HVAC::FanOp::Cycling) {
                // Hard code here
                PartLoadRatio = m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio;
                LoopPartLoadRatio(AirLoopNum) = m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio;
                OnOffFanRunTimeFraction = max(m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopHeatingCoilMaxRTF,
                                              m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopOnOffFanRTF,
                                              m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopDXCoilRTF);
                LoopOnOffFanRunTimeFraction(AirLoopNum) = max(m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopHeatingCoilMaxRTF,
                                                              m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopOnOffFanRTF,
                                                              m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopDXCoilRTF);
            }
            m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopHeatingCoilMaxRTF = 0.0;

            if (DisSysCompCVFData(FanNum).fanType == HVAC::FanType::OnOff && LoopPartLoadRatio(AirLoopNum) < 1.0) {
                for (std::size_t i = 0; i < linkReport.size(); ++i) {
                    auto &r(linkReport[i]);
                    auto const &s(AirflowNetworkLinkSimu[i]);
                    auto const &t(AirflowNetworkLinkageData[i]);
                    if (t.AirLoopNum == AirLoopNum) {
                        r.FLOW = s.FLOW * LoopPartLoadRatio(AirLoopNum);
                        r.FLOW2 = s.FLOW2 * LoopPartLoadRatio(AirLoopNum);
                        r.VolFLOW = s.VolFLOW * LoopPartLoadRatio(AirLoopNum);
                        r.VolFLOW2 = s.VolFLOW2 * LoopPartLoadRatio(AirLoopNum);
                    }
                    if (t.AirLoopNum == 0) {
                        r.FLOW = s.FLOW * MaxPartLoadRatio;
                        r.FLOW2 = s.FLOW2 * MaxPartLoadRatio;
                        r.VolFLOW = s.VolFLOW * MaxPartLoadRatio;
                        r.VolFLOW2 = s.VolFLOW2 * MaxPartLoadRatio;
                    }
                }
            }
        }

        // One time warning
        if (UpdateAirflowNetworkMyOneTimeFlag) {
            for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
                for (FanNum = 1; FanNum <= DisSysNumOfCVFs; ++FanNum) {
                    if (DisSysCompCVFData(FanNum).AirLoopNum == AirLoopNum) {
                        break;
                    }
                }
                if (DisSysCompCVFData(FanNum).fanType == HVAC::FanType::OnOff &&
                    m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode == HVAC::FanOp::Continuous) {
                    OnOffRatio = std::abs((m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOnMassFlowrate -
                                           m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOffMassFlowrate) /
                                          m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOnMassFlowrate);
                    if (OnOffRatio > 0.1) {
                        ShowWarningError(m_state,
                                         "The absolute percent difference of supply air mass flow rate between HVAC operation and No HVAC operation "
                                         "is above 10% with fan operation mode = ContFanCycCoil.");
                        ShowContinueError(m_state,
                                          "The added zone loads using the AirflowNetwork model may not be accurate because the zone loads are "
                                          "calculated based on the mass flow rate during HVAC operation.");
                        ShowContinueError(
                            m_state,
                            format("The mass flow rate during HVAC operation = {:.2R} The mass flow rate during no HVAC operation = {:.2R}",
                                   m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOnMassFlowrate,
                                   m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOffMassFlowrate));
                        UpdateAirflowNetworkMyOneTimeFlag = false;
                    }
                }
            }
        }

        // Check mass flow differences in the zone inlet zones and splitter nodes between node and AFN links
        if (UpdateAirflowNetworkMyOneTimeFlag1) {
            if ((!VAVSystem) && m_state.dataGlobal->DisplayExtraWarnings) {
                bool WriteFlag = false;
                for (int i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
                    Node1 = AirflowNetworkLinkageData(i).NodeNums[0];
                    Node2 = AirflowNetworkLinkageData(i).NodeNums[1];
                    if (AirflowNetworkNodeData(Node1).EPlusTypeNum == iEPlusNodeType::SPI ||
                        AirflowNetworkNodeData(Node2).EPlusTypeNum == iEPlusNodeType::SPO ||
                        AirflowNetworkNodeData(Node2).EPlusTypeNum == iEPlusNodeType::ZIN) {
                        int Node3;
                        if (AirflowNetworkNodeData(Node1).EPlusTypeNum == iEPlusNodeType::SPI) {
                            Node3 = Node1;
                        } else {
                            Node3 = Node2;
                        }
                        if (AirflowNetworkNodeData(Node2).EPlusTypeNum == iEPlusNodeType::ZIN) {
                            if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).EPlusTypeNum == iEPlusComponentType::Invalid) {
                                continue;
                            }
                        }
                        NodeMass = Node(AirflowNetworkNodeData(Node3).EPlusNodeNum).MassFlowRate;
                        AFNMass = AirflowNetworkLinkSimu(i).FLOW;
                        if (NodeMass > 0.0 && AFNMass > NodeMass + 0.01) {
                            ShowWarningError(m_state,
                                             "The mass flow rate difference is found between System Node = '" +
                                                 m_state.dataLoopNodes->NodeID(AirflowNetworkNodeData(Node3).EPlusNodeNum) + "' and AFN Link = '" +
                                                 AirflowNetworkLinkageData(i).Name + "'.");
                            ShowContinueError(m_state,
                                              format("The system node max mass flow rate = {:.3R} kg/s. The AFN node mass flow rate = {:.3R} kg.s.",
                                                     NodeMass,
                                                     AFNMass));
                            WriteFlag = true;
                        }
                    }
                }
                UpdateAirflowNetworkMyOneTimeFlag1 = false;
                if (WriteFlag) {
                    ShowWarningError(m_state,
                                     "Please adjust the rate of Maximum Air Flow Rate field in the terminal objects or duct pressure resistance.");
                }
            } else {
                UpdateAirflowNetworkMyOneTimeFlag1 = false;
            }
        }

        // Assign airflows to EPLus nodes
        for (int i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
            if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::DWC ||
                AirflowNetworkLinkageData(i).VAVTermDamper) {
                // Exclude envelope leakage Crack element
                Node1 = AirflowNetworkLinkageData(i).NodeNums[0];
                Node2 = AirflowNetworkLinkageData(i).NodeNums[1];

                int j = AirflowNetworkNodeData(Node1).EPlusNodeNum;
                if (j > 0 && AirflowNetworkNodeData(Node1).EPlusZoneNum == 0) {
                    Node(j).MassFlowRate = AirflowNetworkLinkSimu(i).FLOW * LoopPartLoadRatio(AirflowNetworkNodeData(Node1).AirLoopNum);
                    if (!LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum)) {
                        Node(j).MassFlowRate = 0.0;
                    }
                    if (!(AirflowNetworkNodeData(Node1).EPlusTypeNum == iEPlusNodeType::DIN ||
                          AirflowNetworkNodeData(Node1).EPlusTypeNum == iEPlusNodeType::DOU)) {
                        Node(j).MassFlowRateMaxAvail = AirflowNetworkLinkSimu(i).FLOW * LoopPartLoadRatio(AirflowNetworkNodeData(Node1).AirLoopNum);
                        Node(j).MassFlowRateMax = AirflowNetworkLinkSimu(i).FLOW;
                    }
                }

                j = AirflowNetworkNodeData(Node2).EPlusNodeNum;
                if (j > 0) {
                    Node(j).MassFlowRate = AirflowNetworkLinkSimu(i).FLOW * LoopPartLoadRatio(AirflowNetworkNodeData(Node2).AirLoopNum);
                    if (!LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum)) {
                        Node(j).MassFlowRate = 0.0;
                    }
                    if (!(AirflowNetworkNodeData(Node2).EPlusTypeNum == iEPlusNodeType::DIN ||
                          AirflowNetworkNodeData(Node2).EPlusTypeNum == iEPlusNodeType::DOU)) {
                        Node(j).MassFlowRateMaxAvail = AirflowNetworkLinkSimu(i).FLOW * LoopPartLoadRatio(AirflowNetworkNodeData(Node2).AirLoopNum);
                        Node(j).MassFlowRateMax = AirflowNetworkLinkSimu(i).FLOW;
                    }
                }
            }
        }

        // Assign AirflowNetwork nodal values to Node array
        for (int i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            int j = AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0) {
                Node(j).Enthalpy = PsyHFnTdbW(AirflowNetworkNodeSimu(i).TZ, AirflowNetworkNodeSimu(i).WZ);
                Node(j).Temp = AirflowNetworkNodeSimu(i).TZ;
                Node(j).HumRat = AirflowNetworkNodeSimu(i).WZ;
                if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                    Node(j).CO2 = AirflowNetworkNodeSimu(i).CO2Z;
                }
                if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                    Node(j).GenContam = AirflowNetworkNodeSimu(i).GCZ;
                }
            }
        }

        // Calculate sensible loads from forced air flow
        for (int i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
            Node1 = AirflowNetworkLinkageData(i).NodeNums[0];
            Node2 = AirflowNetworkLinkageData(i).NodeNums[1];
            CpAir = PsyCpAirFnW((AirflowNetworkNodeSimu(Node1).WZ + AirflowNetworkNodeSimu(Node2).WZ) / 2.0);
            // Calculate sensible loads from duct conduction losses and loads from duct radiation
            if (AirflowNetworkLinkageData(i).ZoneNum > 0 &&
                AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::DWC) {
                Qsen = AirflowNetworkLinkSimu(i).FLOW * CpAir * (AirflowNetworkNodeSimu(Node2).TZ - AirflowNetworkNodeSimu(Node1).TZ);
                if (AirflowNetworkLinkageData(i).LinkageViewFactorObjectNum != 0) {
                    auto const &DuctRadObj(AirflowNetworkLinkageViewFactorData(AirflowNetworkLinkageData(i).LinkageViewFactorObjectNum));
                    Qsen -= DuctRadObj.QRad;
                    exchangeData(AirflowNetworkLinkageData(i).ZoneNum).RadGain -= DuctRadObj.QRad;
                }
                // When the Airloop is shut off, no duct sensible losses
                if (!LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum)) {
                    Qsen = 0.0;
                }
                exchangeData(AirflowNetworkLinkageData(i).ZoneNum).CondSen -= Qsen;
            }
            // Calculate sensible leakage losses
            if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::PLR ||
                AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::ELR) {
                // Calculate supply leak sensible losses
                if ((AirflowNetworkNodeData(Node2).EPlusZoneNum > 0) && (AirflowNetworkNodeData(Node1).EPlusNodeNum == 0) &&
                    (AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    ZN2 = AirflowNetworkNodeData(Node2).EPlusZoneNum;
                    Qsen = AirflowNetworkLinkSimu(i).FLOW * CpAir * (AirflowNetworkNodeSimu(Node1).TZ - AirflowNetworkNodeSimu(Node2).TZ);
                    if (!LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum)) {
                        Qsen = 0.0;
                    }
                    exchangeData(ZN2).LeakSen += Qsen;
                }
                if ((AirflowNetworkNodeData(Node1).EPlusZoneNum > 0) && (AirflowNetworkNodeData(Node2).EPlusNodeNum == 0) &&
                    (AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    ZN1 = AirflowNetworkNodeData(Node1).EPlusZoneNum;
                    Qsen = AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (AirflowNetworkNodeSimu(Node2).TZ - AirflowNetworkNodeSimu(Node1).TZ);
                    if (!LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum)) {
                        Qsen = 0.0;
                    }
                    exchangeData(ZN1).LeakSen += Qsen;
                }
            }
        }

        // Calculate latent loads from forced air flow
        for (int i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
            Node1 = AirflowNetworkLinkageData(i).NodeNums[0];
            Node2 = AirflowNetworkLinkageData(i).NodeNums[1];
            // Calculate latent loads from duct conduction losses
            if (AirflowNetworkLinkageData(i).ZoneNum > 0 &&
                AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::DWC) {
                Qlat = AirflowNetworkLinkSimu(i).FLOW * (AirflowNetworkNodeSimu(Node2).WZ - AirflowNetworkNodeSimu(Node1).WZ);
                if (!LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum)) {
                    Qlat = 0.0;
                }
                exchangeData(AirflowNetworkLinkageData(i).ZoneNum).DiffLat -= Qlat;
            }
            // Calculate latent leakage losses
            if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::PLR ||
                AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::ELR) {
                // Calculate supply leak latent losses
                if ((AirflowNetworkNodeData(Node2).EPlusZoneNum > 0) && (AirflowNetworkNodeData(Node1).EPlusNodeNum == 0) &&
                    (AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    ZN2 = AirflowNetworkNodeData(Node2).EPlusZoneNum;
                    Qlat = AirflowNetworkLinkSimu(i).FLOW * (AirflowNetworkNodeSimu(Node1).WZ - AirflowNetworkNodeSimu(Node2).WZ);
                    if (!LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum)) {
                        Qlat = 0.0;
                    }
                    exchangeData(ZN2).LeakLat += Qlat;
                    if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        exchangeData(ZN2).TotalCO2 +=
                            AirflowNetworkLinkSimu(i).FLOW * (AirflowNetworkNodeSimu(Node1).CO2Z - AirflowNetworkNodeSimu(Node2).CO2Z);
                    }
                    if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        exchangeData(ZN2).TotalGC +=
                            AirflowNetworkLinkSimu(i).FLOW * (AirflowNetworkNodeSimu(Node1).GCZ - AirflowNetworkNodeSimu(Node2).GCZ);
                    }
                }
                if ((AirflowNetworkNodeData(Node1).EPlusZoneNum > 0) && (AirflowNetworkNodeData(Node2).EPlusNodeNum == 0) &&
                    (AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    ZN1 = AirflowNetworkNodeData(Node1).EPlusZoneNum;
                    Qlat = AirflowNetworkLinkSimu(i).FLOW2 * (AirflowNetworkNodeSimu(Node2).WZ - AirflowNetworkNodeSimu(Node1).WZ);
                    if (!LoopOnOffFlag(AirflowNetworkLinkageData(i).AirLoopNum)) {
                        Qlat = 0.0;
                    }
                    exchangeData(ZN1).LeakLat += Qlat;
                    if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        exchangeData(ZN1).TotalCO2 +=
                            AirflowNetworkLinkSimu(i).FLOW2 * (AirflowNetworkNodeSimu(Node2).CO2Z - AirflowNetworkNodeSimu(Node1).CO2Z);
                    }
                    if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        exchangeData(ZN1).TotalGC +=
                            AirflowNetworkLinkSimu(i).FLOW2 * (AirflowNetworkNodeSimu(Node2).GCZ - AirflowNetworkNodeSimu(Node1).GCZ);
                    }
                }
            }
        }

        // Sum all the loads
        for (int i = 1; i <= m_state.dataGlobal->NumOfZones; ++i) {
            exchangeData(i).TotalSen = exchangeData(i).LeakSen + exchangeData(i).CondSen + exchangeData(i).RadGain;
            exchangeData(i).TotalLat = exchangeData(i).LeakLat + exchangeData(i).DiffLat;
        }

        // Simple ONOFF fan
        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            for (FanNum = 1; FanNum <= DisSysNumOfCVFs; ++FanNum) {
                if (DisSysCompCVFData(FanNum).AirLoopNum == AirLoopNum) {
                    break;
                }
            }
            if (DisSysCompCVFData(FanNum).fanType == HVAC::FanType::OnOff && OnOffFanRunTimeFraction < 1.0) {
                for (int i = 1; i <= m_state.dataGlobal->NumOfZones; ++i) {
                    exchangeData(i).MultiZoneSen *= OnOffFanRunTimeFraction;
                    exchangeData(i).MultiZoneLat *= OnOffFanRunTimeFraction;
                    exchangeData(i).LeakSen *= OnOffFanRunTimeFraction;
                    exchangeData(i).LeakLat *= OnOffFanRunTimeFraction;
                    exchangeData(i).CondSen *= OnOffFanRunTimeFraction;
                    exchangeData(i).DiffLat *= OnOffFanRunTimeFraction;
                    exchangeData(i).RadGain *= OnOffFanRunTimeFraction;
                    exchangeData(i).TotalSen *= OnOffFanRunTimeFraction;
                    exchangeData(i).TotalLat *= OnOffFanRunTimeFraction;
                    exchangeData(i).SumMCp *= OnOffFanRunTimeFraction;
                    exchangeData(i).SumMCpT *= OnOffFanRunTimeFraction;
                    exchangeData(i).SumMVCp *= OnOffFanRunTimeFraction;
                    exchangeData(i).SumMVCpT *= OnOffFanRunTimeFraction;
                    exchangeData(i).SumMHr *= OnOffFanRunTimeFraction;
                    exchangeData(i).SumMHrW *= OnOffFanRunTimeFraction;
                    exchangeData(i).SumMMCp *= OnOffFanRunTimeFraction;
                    exchangeData(i).SumMMCpT *= OnOffFanRunTimeFraction;
                    exchangeData(i).SumMMHr *= OnOffFanRunTimeFraction;
                    exchangeData(i).SumMMHrW *= OnOffFanRunTimeFraction;
                    if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        exchangeData(i).SumMHrCO *= OnOffFanRunTimeFraction;
                        exchangeData(i).SumMMHrCO *= OnOffFanRunTimeFraction;
                    }
                    if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        exchangeData(i).SumMHrGC *= OnOffFanRunTimeFraction;
                        exchangeData(i).SumMMHrGC *= OnOffFanRunTimeFraction;
                    }
                }
                if (m_state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode == HVAC::FanOp::Cycling) {
                    for (int i = 1; i <= m_state.dataGlobal->NumOfZones; ++i) {
                        exchangeData(i).SumMCp += multiExchangeData(i).SumMCp * (1.0 - OnOffFanRunTimeFraction);
                        exchangeData(i).SumMCpT += multiExchangeData(i).SumMCpT * (1.0 - OnOffFanRunTimeFraction);
                        exchangeData(i).SumMVCp += multiExchangeData(i).SumMVCp * (1.0 - OnOffFanRunTimeFraction);
                        exchangeData(i).SumMVCpT += multiExchangeData(i).SumMVCpT * (1.0 - OnOffFanRunTimeFraction);
                        exchangeData(i).SumMHr += multiExchangeData(i).SumMHr * (1.0 - OnOffFanRunTimeFraction);
                        exchangeData(i).SumMHrW += multiExchangeData(i).SumMHrW * (1.0 - OnOffFanRunTimeFraction);
                        exchangeData(i).SumMMCp += multiExchangeData(i).SumMMCp * (1.0 - OnOffFanRunTimeFraction);
                        exchangeData(i).SumMMCpT += multiExchangeData(i).SumMMCpT * (1.0 - OnOffFanRunTimeFraction);
                        exchangeData(i).SumMMHr += multiExchangeData(i).SumMMHr * (1.0 - OnOffFanRunTimeFraction);
                        exchangeData(i).SumMMHrW += multiExchangeData(i).SumMMHrW * (1.0 - OnOffFanRunTimeFraction);
                        if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                            exchangeData(i).SumMHrCO += multiExchangeData(i).SumMHrCO * (1.0 - OnOffFanRunTimeFraction);
                            exchangeData(i).SumMMHrCO += multiExchangeData(i).SumMMHrCO * (1.0 - OnOffFanRunTimeFraction);
                        }
                        if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                            exchangeData(i).SumMHrGC += multiExchangeData(i).SumMHrGC * (1.0 - OnOffFanRunTimeFraction);
                            exchangeData(i).SumMMHrGC += multiExchangeData(i).SumMMHrGC * (1.0 - OnOffFanRunTimeFraction);
                        }
                    }
                }
            }

            if (DisSysCompCVFData(FanNum).fanType == HVAC::FanType::OnOff) {
                for (int i = 1; i <= AirflowNetworkNumOfZones; ++i) {
                    if (AirflowNetworkNodeData(i).AirLoopNum == AirLoopNum) {
                        nodeReport(i).PZ = AirflowNetworkNodeSimu(i).PZ * LoopPartLoadRatio(AirLoopNum) +
                                           nodeReport(i).PZOFF * (1.0 - LoopPartLoadRatio(AirLoopNum));
                        nodeReport(i).PZON = AirflowNetworkNodeSimu(i).PZ;
                    }
                }
                for (int i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
                    PartLoadRatio = MaxPartLoadRatio;
                    for (int j = 1; j <= AirflowNetworkNumOfZones; ++j) {
                        if (MultizoneZoneData(j).ZoneNum == MultizoneSurfaceData(i).ZonePtr) {
                            if (AirflowNetworkNodeData(j).AirLoopNum == AirLoopNum) {
                                PartLoadRatio = LoopPartLoadRatio(AirLoopNum);
                                break;
                            }
                        }
                    }
                    linkReport1(i).FLOW = AirflowNetworkLinkSimu(i).FLOW * PartLoadRatio + linkReport1(i).FLOWOFF * (1.0 - PartLoadRatio);
                    linkReport1(i).FLOW2 = AirflowNetworkLinkSimu(i).FLOW2 * PartLoadRatio + linkReport1(i).FLOW2OFF * (1.0 - PartLoadRatio);
                    linkReport1(i).VolFLOW = AirflowNetworkLinkSimu(i).VolFLOW * PartLoadRatio + linkReport1(i).VolFLOWOFF * (1.0 - PartLoadRatio);
                    linkReport1(i).VolFLOW2 = AirflowNetworkLinkSimu(i).VolFLOW2 * PartLoadRatio + linkReport1(i).VolFLOW2OFF * (1.0 - PartLoadRatio);
                    linkReport1(i).DP = AirflowNetworkLinkSimu(i).DP * PartLoadRatio + linkReport1(i).DPOFF * (1.0 - PartLoadRatio);
                    linkReport1(i).DPON = AirflowNetworkLinkSimu(i).DP;
                }
            }
        }

        // Save values
        for (int i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            AirflowNetworkNodeSimu(i).TZlast = AirflowNetworkNodeSimu(i).TZ;
            AirflowNetworkNodeSimu(i).WZlast = AirflowNetworkNodeSimu(i).WZ;
            if (m_state.dataContaminantBalance->Contaminant.CO2Simulation) {
                AirflowNetworkNodeSimu(i).CO2Zlast = AirflowNetworkNodeSimu(i).CO2Z;
            }
            if (m_state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                AirflowNetworkNodeSimu(i).GCZlast = AirflowNetworkNodeSimu(i).GCZ;
            }
        }
    }

    void Solver::venting_control(int const i,       // AirflowNetwork surface number
                                 Real64 &OpenFactor // Window or door opening factor (used to calculate airflow)
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   April 2003
        //       MODIFIED       Feb 2004, FCW: allow venting control of interior window/door
        //       MODIFIED       Nov. 2005, LG: to fit the requirement for AirflowNetwork Model
        //       RE-ENGINEERED

        // PURPOSE OF THIS SUBROUTINE:
        // Determines the venting opening factor for an exterior or interior window or door
        // as determined by the venting control method.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 VentTemp;                // Venting temperature (C)
        Real64 ZoneAirEnthalpy;         // Enthalpy of zone air (J/kg)
        Real64 OpenFactorMult;          // Window/door opening modulation multiplier on venting open factor
        Real64 DelTemp;                 // Inside-outside air temperature difference (K)
        Real64 DelEnthal;               // Inside-outside air enthalpy difference (J/kg)
        int IZ;                         // AirflowNetwork zone number
        int ZoneNum;                    // EnergyPlus zone number
        int SurfNum;                    // Heat transfer surface number
        Real64 LimValVentOpenFacMult;   // Limiting value of venting opening factor multiplier
        Real64 LowerValInOutTempDiff;   // Lower value of inside/outside temperature difference for opening factor modulation
        Real64 UpperValInOutTempDiff;   // Upper value of inside/outside temperature difference for opening factor modulation
        Real64 LowerValInOutEnthalDiff; // Lower value of inside/outside enthalpy difference for opening factor modulation
        Real64 UpperValInOutEnthalDiff; // Upper value of inside/outside enthalpy difference for opening factor modulation
        bool VentingAllowed;            // True if venting schedule allows venting
        int VentCtrlNum;                // Venting control strategy 1: Temperature control; 2: Enthalpy control
        Real64 VentingSchVal;           // Current time step value of venting schedule
        Real64 Tamb;                    // Outdoor dry bulb temperature at surface centroid height
        int PeopleInd;

        if (MultizoneSurfaceData(i).EMSOpenFactorActuated) { // EMS sets value to use
            OpenFactor = MultizoneSurfaceData(i).EMSOpenFactor;
            SurfNum = MultizoneSurfaceData(i).SurfNum;
            if (MultizoneSurfaceData(i).Factor > 0.0) {
                m_state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = OpenFactor / MultizoneSurfaceData(i).Factor;
            } else {
                m_state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = OpenFactor;
            }
            return;
        }

        SurfNum = MultizoneSurfaceData(i).SurfNum;

        m_state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = -1.0;

        // Get venting temperature and venting strategy for exterior window or door
        // and determine whether venting is allowed

        m_state.dataSurface->SurfWinVentingAvailabilityRep(SurfNum) = 1.0;
        VentingAllowed = true;
        IZ = MultizoneSurfaceData(i).NodeNums[0];
        // Revise for RoomAirflowNetwork model
        if (MultizoneSurfaceData(i).RAFNflag) {
            IZ = MultizoneSurfaceData(i).ZonePtr;
        }
        ZoneNum = MultizoneZoneData(IZ).ZoneNum;

        // Note in the following that individual venting control for a window/door takes
        // precedence over zone-level control
        if (MultizoneSurfaceData(i).IndVentControl) {
            VentTemp = MultizoneSurfaceData(i).ventTempControlSched ? MultizoneSurfaceData(i).ventTempControlSched->getCurrentVal() : 0.0;
            VentCtrlNum = MultizoneSurfaceData(i).VentSurfCtrNum;
            if (MultizoneSurfaceData(i).ventAvailSched != nullptr) {
                VentingSchVal = MultizoneSurfaceData(i).ventAvailSched->getCurrentVal();
                if (VentingSchVal <= 0.0) {
                    VentingAllowed = false;
                    m_state.dataSurface->SurfWinVentingAvailabilityRep(SurfNum) = 0.0;
                }
            }
        } else {
            // Zone level only by Gu on Nov. 8, 2005
            VentTemp = MultizoneZoneData(IZ).ventTempControlSched ? MultizoneZoneData(IZ).ventTempControlSched->getCurrentVal() : 0.0;
            VentCtrlNum = MultizoneZoneData(IZ).VentCtrNum;
            if (MultizoneZoneData(IZ).ventAvailSched != nullptr) {
                VentingSchVal = MultizoneZoneData(IZ).ventAvailSched->getCurrentVal();
                if (VentingSchVal <= 0.0) {
                    VentingAllowed = false;
                    m_state.dataSurface->SurfWinVentingAvailabilityRep(SurfNum) = 0.0;
                }
            }
        }

        m_state.dataSurface->SurfWinInsideTempForVentingRep(SurfNum) = VentTemp;
        OpenFactor = 0.0;

        // Venting based on inside-outside air temperature difference

        if ((VentCtrlNum == VentControlType::Temp || VentCtrlNum == VentControlType::AdjTemp) && VentingAllowed) {
            Tamb = m_state.dataSurface->SurfOutDryBulbTemp(SurfNum);
            // Check whether this surface is an interior wall or not. If Yes, use adjacent zone conditions
            if (VentCtrlNum == VentControlType::AdjTemp && MultizoneSurfaceData(i).IndVentControl) {
                Tamb = ANZT(MultizoneZoneData(MultizoneSurfaceData(i).NodeNums[1]).ZoneNum);
            }
            if (ANZT(ZoneNum) > Tamb && ANZT(ZoneNum) > VentTemp) {
                OpenFactor = MultizoneSurfaceData(i).Factor;
                m_state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = 1.0;
                // Modulation of OpenFactor
                if (MultizoneSurfaceData(i).IndVentControl) {
                    LimValVentOpenFacMult = MultizoneSurfaceData(i).ModulateFactor;
                    LowerValInOutTempDiff = MultizoneSurfaceData(i).LowValueTemp;
                    UpperValInOutTempDiff = MultizoneSurfaceData(i).UpValueTemp;
                } else {
                    LimValVentOpenFacMult = MultizoneZoneData(IZ).OpenFactor;
                    LowerValInOutTempDiff = MultizoneZoneData(IZ).LowValueTemp;
                    UpperValInOutTempDiff = MultizoneZoneData(IZ).UpValueTemp;
                }
                if (LimValVentOpenFacMult != 1.0) {
                    DelTemp = ANZT(ZoneNum) - Tamb;
                    if (DelTemp <= LowerValInOutTempDiff) {
                        OpenFactorMult = 1.0;
                    } else if (DelTemp >= UpperValInOutTempDiff) {
                        OpenFactorMult = LimValVentOpenFacMult;
                    } else {
                        OpenFactorMult =
                            LimValVentOpenFacMult +
                            ((UpperValInOutTempDiff - DelTemp) / (UpperValInOutTempDiff - LowerValInOutTempDiff)) * (1 - LimValVentOpenFacMult);
                    }
                    OpenFactor *= OpenFactorMult;
                    m_state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = OpenFactorMult;
                }
            } else {
                OpenFactor = 0.0;
                m_state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = -1.0;
            }
        }

        // Venting based on inside-outside air enthalpy difference

        if ((VentCtrlNum == VentControlType::Enth || VentCtrlNum == VentControlType::AdjEnth) && VentingAllowed) {
            ZoneAirEnthalpy = PsyHFnTdbW(ANZT(ZoneNum), ANZW(ZoneNum));
            // Check whether this surface is an interior wall or not. If Yes, use adjacent zone conditions
            if (VentCtrlNum == VentControlType::AdjEnth && MultizoneSurfaceData(i).IndVentControl) {
                m_state.dataEnvrn->OutEnthalpy = PsyHFnTdbW(ANZT(MultizoneZoneData(MultizoneSurfaceData(i).NodeNums[1]).ZoneNum),
                                                            ANZW(MultizoneZoneData(MultizoneSurfaceData(i).NodeNums[1]).ZoneNum));
            }
            if (ZoneAirEnthalpy > m_state.dataEnvrn->OutEnthalpy && ANZT(ZoneNum) > VentTemp) {
                OpenFactor = MultizoneSurfaceData(i).Factor;
                // Modulation of OpenFactor
                if (MultizoneSurfaceData(i).IndVentControl) {
                    LimValVentOpenFacMult = MultizoneSurfaceData(i).ModulateFactor;
                    LowerValInOutEnthalDiff = MultizoneSurfaceData(i).LowValueEnth;
                    UpperValInOutEnthalDiff = MultizoneSurfaceData(i).UpValueEnth;
                } else {
                    LimValVentOpenFacMult = MultizoneZoneData(IZ).OpenFactor;
                    LowerValInOutEnthalDiff = MultizoneZoneData(IZ).LowValueEnth;
                    UpperValInOutEnthalDiff = MultizoneZoneData(IZ).UpValueEnth;
                }
                m_state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = 1.0;

                if (LimValVentOpenFacMult != 1.0) {
                    DelEnthal = ZoneAirEnthalpy - m_state.dataEnvrn->OutEnthalpy;
                    if (DelEnthal <= LowerValInOutEnthalDiff) {
                        OpenFactorMult = 1.0;
                    } else if (DelEnthal >= UpperValInOutEnthalDiff) {
                        OpenFactorMult = LimValVentOpenFacMult;
                    } else {
                        OpenFactorMult =
                            LimValVentOpenFacMult + ((UpperValInOutEnthalDiff - DelEnthal) / (UpperValInOutEnthalDiff - LowerValInOutEnthalDiff)) *
                                                        (1 - LimValVentOpenFacMult);
                    }
                    OpenFactor *= OpenFactorMult;
                    m_state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = OpenFactorMult;
                }
            } else {
                OpenFactor = 0.0;
                m_state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = -1.0;
            }
        }

        // Constant venting (opening factor as specified in IDF) - C-PH - added by Philip Haves 3/8/01
        // subject to venting availability

        if (VentCtrlNum == VentControlType::Const && VentingAllowed) { // Constant
            OpenFactor = MultizoneSurfaceData(i).Factor;
            m_state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = 1.0;
        }

        if (VentCtrlNum == VentControlType::ASH55) {
            if (VentingAllowed && (!m_state.dataGlobal->BeginEnvrnFlag) && (!m_state.dataGlobal->WarmupFlag)) {
                PeopleInd = MultizoneZoneData(IZ).ASH55PeopleInd;
                if (PeopleInd > 0 && m_state.dataThermalComforts->ThermalComfortData(PeopleInd).ThermalComfortAdaptiveASH5590 != -1) {
                    if (m_state.dataThermalComforts->ThermalComfortData(PeopleInd).ThermalComfortOpTemp >
                        m_state.dataThermalComforts->ThermalComfortData(PeopleInd).TComfASH55) {
                        OpenFactor = MultizoneSurfaceData(i).Factor;
                        m_state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = 1.0;
                    } else {
                        OpenFactor = 0.0;
                    }
                } else {
                    OpenFactor = 0.0;
                }
            } else {
                OpenFactor = 0.0;
            }
        }

        if (VentCtrlNum == VentControlType::CEN15251) {
            if (VentingAllowed && (!m_state.dataGlobal->BeginEnvrnFlag) && (!m_state.dataGlobal->WarmupFlag)) {
                PeopleInd = MultizoneZoneData(IZ).CEN15251PeopleInd;
                if (PeopleInd > 0 && m_state.dataThermalComforts->ThermalComfortData(PeopleInd).ThermalComfortAdaptiveCEN15251CatI != -1) {
                    if (m_state.dataThermalComforts->ThermalComfortData(PeopleInd).ThermalComfortOpTemp >
                        m_state.dataThermalComforts->ThermalComfortData(PeopleInd).TComfCEN15251) {
                        OpenFactor = MultizoneSurfaceData(i).Factor;
                        m_state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = 1.0;
                    } else {
                        OpenFactor = 0.0;
                    }
                } else {
                    OpenFactor = 0.0;
                }
            } else {
                OpenFactor = 0.0;
            }
        }

        // No venting, i.e, window/door always closed - added YJH 8 Aug 02

        if (VentCtrlNum == VentControlType::NoVent) { // Novent
            OpenFactor = 0.0;
            m_state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = -1.0;
        }
    }

    void Solver::validate_distribution()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       L. Gu, Jan. 2009: allow a desuperheater coil and three heat exchangers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine validates the inputs of distribution system, since node data from a primary airloop
        // are not available in the first call during reading input data of airflownetwork objects.
        // Note: this routine shouldn't be called more than once

        // Using/Aliasing
        using BranchNodeConnections::GetNodeConnectionType;
        using MixedAir::GetNumOAMixers;
        using MixedAir::GetOAMixerInletNodeNumber;
        using MixedAir::GetOAMixerReliefNodeNumber;
        using SingleDuct::GetHVACSingleDuctSysIndex;
        using namespace DataLoopNode;
        auto &NumPrimaryAirSys = m_state.dataHVACGlobal->NumPrimaryAirSys;
        using DXCoils::SetDXCoilAirLoopNumber;
        using HeatingCoils::SetHeatingCoilAirLoopNumber;
        using HVACStandAloneERV::GetStandAloneERVNodeNumber;
        using HVACVariableRefrigerantFlow::getVRFTUNodeNumber;
        using SplitterComponent::GetSplitterNodeNumbers;
        using SplitterComponent::GetSplitterOutletNumber;
        using UnitarySystems::getUnitarySystemNodeNumber;
        using WaterThermalTanks::GetHeatPumpWaterHeaterNodeNumber;
        using WindowAC::getWindowACNodeNumber;
        using ZoneDehumidifier::GetZoneDehumidifierNodeNumber;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("AirflowNetwork::Solver::validate_distribution: "); // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int n;
        bool LocalError;
        Array1D_bool NodeFound;

        bool ErrorsFound(false);
        bool IsNotOK(false);
        bool errFlag(false);
        EPVector<DataLoopNode::ConnectionType> NodeConnectionType; // Specifies the type of node connection
        std::string CurrentModuleObject;

        bool hpwhFound(false);            // Flag for HPWH identification
        bool standaloneERVFound(false);   // Flag for Standalone ERV (ZoneHVAC:EnergyRecoveryVentilator) identification
        bool packagedUnitaryFound(false); // Flag for packaged unitary systems (ZoneHVAC:PackagedTerminalAirConditioner,
                                          // ZoneHVAC:PackagedTerminalHeatPump, ZoneHVAC:WaterToAirHeatPump) identification
        bool vrfTUFound(false);
        bool windowACFound(false); // Flag for Window AC (ZoneHVAC:WindowAirConditioner) identification

        // Validate supply and return connections
        NodeFound.dimension(m_state.dataLoopNodes->NumOfNodes, false);
        // Validate inlet and outlet nodes for zone exhaust fans
        for (int i = 1; i <= AirflowNetworkNumOfExhFan; ++i) {
            NodeFound(MultizoneCompExhaustFanData(i).InletNode) = true;
            NodeFound(MultizoneCompExhaustFanData(i).OutletNode) = true;
        }
        // Validate EPlus Node names and types
        for (int i = 1; i <= DisSysNumOfNodes; ++i) {
            if (Util::SameString(DisSysNodeData(i).EPlusName, "") || Util::SameString(DisSysNodeData(i).EPlusName, "Other")) {
                continue;
            }
            LocalError = false;
            for (int j = 1; j <= m_state.dataLoopNodes->NumOfNodes; ++j) { // NodeID
                if (DisSysNodeData(i).EPlusName == m_state.dataLoopNodes->NodeID(j)) {
                    DisSysNodeData(i).AirLoopNum = get_airloop_number(j);
                    if (DisSysNodeData(i).AirLoopNum == 0) {
                        ShowSevereError(m_state,
                                        format(RoutineName) + "The Node or Component Name defined in " + DisSysNodeData(i).Name +
                                            " is not found in the AirLoopHVAC.");
                        ShowContinueError(m_state,
                                          "The entered name is " + DisSysNodeData(i).EPlusName + " in an AirflowNetwork:Distribution:Node object.");
                        ErrorsFound = true;
                    }
                    DisSysNodeData(i).EPlusNodeNum = j;
                    AirflowNetworkNodeData(NumOfNodesMultiZone + i).EPlusNodeNum = j;
                    AirflowNetworkNodeData(NumOfNodesMultiZone + i).AirLoopNum = DisSysNodeData(i).AirLoopNum;
                    NodeFound(j) = true;
                    LocalError = true;
                    break;
                }
            }
            // Check outdoor air node
            if (Util::SameString(DisSysNodeData(i).EPlusType, "OutdoorAir:NodeList") ||
                Util::SameString(DisSysNodeData(i).EPlusType, "OutdoorAir:Node")) {
                if (!LocalError) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + "The Node or Component Name defined in " + DisSysNodeData(i).Name +
                                        " is not found in the " + DisSysNodeData(i).EPlusType);
                    ShowContinueError(m_state,
                                      "The entered name is " + DisSysNodeData(i).EPlusName + " in an AirflowNetwork:Distribution:Node object.");
                    ErrorsFound = true;
                }
            }
            if (DisSysNodeData(i).EPlusNodeNum == 0) {
                ShowSevereError(m_state,
                                format(RoutineName) +
                                    "Primary Air Loop Node is not found in AIRFLOWNETWORK:DISTRIBUTION:NODE = " + DisSysNodeData(i).Name);
                ErrorsFound = true;
            }
        }

        // Determine node numbers for zone inlets and outlets
        for (int i = 1; i <= m_state.dataGlobal->NumOfZones; ++i) {
            if (!m_state.dataZoneEquip->ZoneEquipConfig(i).IsControlled) {
                continue;
            }
            for (int j = 1; j <= m_state.dataZoneEquip->ZoneEquipConfig(i).NumInletNodes; ++j) {
                for (int k = 1; k <= AirflowNetworkNumOfNodes; ++k) {
                    if (m_state.dataZoneEquip->ZoneEquipConfig(i).InletNode(j) == AirflowNetworkNodeData(k).EPlusNodeNum) {
                        AirflowNetworkNodeData(k).EPlusTypeNum = iEPlusNodeType::ZIN;
                        break;
                    }
                }
            }
            for (int j = 1; j <= m_state.dataZoneEquip->ZoneEquipConfig(i).NumReturnNodes; ++j) {
                for (int k = 1; k <= AirflowNetworkNumOfNodes; ++k) {
                    if (m_state.dataZoneEquip->ZoneEquipConfig(i).ReturnNode(j) == AirflowNetworkNodeData(k).EPlusNodeNum) {
                        AirflowNetworkNodeData(k).EPlusTypeNum = iEPlusNodeType::ZOU;
                        break;
                    }
                }
            }
        }

        // Eliminate node not related to AirLoopHVAC
        for (int k = 1; k <= m_state.dataBranchNodeConnections->NumOfNodeConnections; ++k) {
            if (NodeFound(m_state.dataBranchNodeConnections->NodeConnections(k).NodeNumber)) {
                continue;
            }
            if (m_state.dataBranchNodeConnections->NodeConnections(k).FluidStream == NodeInputManager::CompFluidStream::Secondary) {
                NodeFound(m_state.dataBranchNodeConnections->NodeConnections(k).NodeNumber) = true;
            }
        }

        // Eliminate nodes with fluidtype = water
        for (int k = 1; k <= m_state.dataLoopNodes->NumOfNodes; ++k) {
            if (NodeFound(k)) {
                continue;
            }
            if (m_state.dataLoopNodes->Node(k).FluidType == DataLoopNode::NodeFluidType::Water) {
                NodeFound(k) = true;
            }
        }

        // Eliminate local external air node for network
        for (int k = 1; k <= m_state.dataLoopNodes->NumOfNodes; ++k) {
            if (NodeFound(k)) {
                continue;
            }
            if (m_state.dataLoopNodes->Node(k).IsLocalNode) {
                NodeFound(k) = true;
            }
        }

        // Ensure all the nodes used in Eplus are a subset of AirflowNetwork Nodes
        for (int i = 1; i <= m_state.dataLoopNodes->NumOfNodes; ++i) {
            if (NodeFound(i)) {
                continue;
            }
            // Skip the inlet and outlet nodes of zone dehumidifiers
            if (GetZoneDehumidifierNodeNumber(m_state, i)) {
                NodeFound(i) = true;
            }

            if (simulation_control.allow_unsupported_zone_equipment) {
                // Skip HPWH nodes that don't have to be included in the AFN
                if (GetHeatPumpWaterHeaterNodeNumber(m_state, i)) {
                    NodeFound(i) = true;
                    hpwhFound = true;
                }

                // Skip Standalone ERV nodes that don't have to be included in the AFN
                if (GetStandAloneERVNodeNumber(m_state, i)) {
                    NodeFound(i) = true;
                    standaloneERVFound = true;
                }

                // Skip zonal unitary system based nodes that don't have to be included in the AFN
                if (getUnitarySystemNodeNumber(m_state, i)) {
                    NodeFound(i) = true;
                    packagedUnitaryFound = true;
                }

                // Skip zonal vrf terminal nodes that don't have to be included in the AFN
                if (getVRFTUNodeNumber(m_state, i)) {
                    NodeFound(i) = true;
                    vrfTUFound = true;
                }

                // Skip Window AC with no OA
                if (getWindowACNodeNumber(m_state, i)) {
                    NodeFound(i) = true;
                    windowACFound = true;
                }
            }

            for (int zoneNum = 1; zoneNum <= m_state.dataGlobal->NumOfZones; ++zoneNum) {
                if (!m_state.dataZoneEquip->ZoneEquipConfig(zoneNum).IsControlled) {
                    continue;
                }
                if (m_state.dataZoneEquip->ZoneEquipConfig(zoneNum).ZoneNode == i) {
                    if (zoneNum > AirflowNetworkNumOfNodes) {
                        ShowSevereError(m_state,
                                        format(RoutineName) + "'" + m_state.dataLoopNodes->NodeID(i) +
                                            "' is not defined as an AirflowNetwork:Distribution:Node object.");
                        ShowContinueError(
                            m_state, "This Node is the zone air node for Zone '" + m_state.dataZoneEquip->ZoneEquipConfig(zoneNum).ZoneName + "'.");
                        ErrorsFound = true;
                    } else {
                        NodeFound(i) = true;
                        for (int iZone = 1; iZone <= AirflowNetworkNumOfZones; iZone++) {
                            if (MultizoneZoneData(iZone).ZoneNum == zoneNum) {
                                AirflowNetworkNodeData(iZone).EPlusNodeNum = i;
                            }
                        }
                    }
                    break;
                }
            }

            //   skip nodes that are not part of an airflow network

            //     DX COIL CONDENSER NODE TEST:
            //     Outside air nodes are used for DX coil condenser inlet nodes, these are specified in an outside air node or
            //     OutdoorAir:NodeList object (and classified with NodeConnectionType as OutsideAir). In addition,
            //     this same node is specified in a Coil:DX:CoolingBypassFactorEmpirical object (and classified with
            //     NodeConnectionType as OutsideAirReference). In the NodeConnectionType structure, both of these nodes have a
            //     unique index but have the same node number. The Outside Air Node will usually be listed first. Search for all
            //     indexes with the same node number and check if it is classified as NodeConnectionType = OutsideAirReference.
            //     Mark this node as found since it is not used in an airflownetwork simulation.
            //     Example (using AirflowNetwork_MultiZone_SmallOffice.idf with a single OA Mixer):
            //             (the example shown below is identical to AirflowNetwork_SimpleHouse.idf with no OA Mixer except
            //              that the NodeConnections indexes are (7) and (31), respectively and the NodeNumber = 6)
            //   The GetNodeConnectionType CALL below returns DataLoopNode::NodeConnectionType::OutsideAir = 7 and
            //   DataLoopNode::NodeConnectionType::OutsideAirReference = 14.
            //     NodeConnections info from OUTSIDE AIR NODE object read:
            //     NodeConnections(9)NodeNumber      = 10
            //     NodeConnections(9)NodeName        = ACDXCOIL 1 CONDENSER NODE
            //     NodeConnections(9)ObjectType      = OUTSIDE AIR NODE
            //     NodeConnections(9)ObjectName      = OUTSIDE AIR NODE
            //     NodeConnections(9)ConnectionType  = OutsideAir
            //     NodeConnections info from Coil:DX:CoolingBypassFactorEmpirical object read:
            //     NodeConnections(64)NodeNumber     = 10
            //     NodeConnections(64)NodeName       = ACDXCOIL 1 CONDENSER NODE
            //     NodeConnections(64)ObjectType     = COIL:DX:COOLINGBYPASSFACTOREMPIRICAL
            //     NodeConnections(64)ObjectName     = ACDXCOIL 1
            //     NodeConnections(64)ConnectionType = OutsideAirReference

            errFlag = false;
            GetNodeConnectionType(m_state, i, NodeConnectionType, errFlag); // Gets all connection types for a given node number
            if (errFlag) {
                ShowContinueError(m_state, "...occurs in Airflow Network simulation.");
            } else {
                //   skip nodes for air cooled condensers
                for (int j = 1; j <= isize(NodeConnectionType); ++j) {
                    if (NodeConnectionType(j) == DataLoopNode::ConnectionType::OutsideAirReference) {
                        NodeFound(i) = true;
                    }
                }
            }

            if (!NodeFound(i)) {
                // Check if this node is the OA relief node. For the time being, OA relief node is not used
                if (GetNumOAMixers(m_state) > 1) {
                    //                        ShowSevereError(m_state,  format(RoutineName) + "Only one OutdoorAir:Mixer is allowed in the
                    // AirflowNetwork model." );                         ErrorsFound = true;
                    int OAFanNum;
                    int OARelNum;
                    int OAMixerNum;

                    for (OAFanNum = 1; OAFanNum <= NumOfOAFans; ++OAFanNum) {
                        DisSysCompOutdoorAirData(OAFanNum).InletNode =
                            GetOAMixerInletNodeNumber(m_state, DisSysCompOutdoorAirData(OAFanNum).OAMixerNum);
                        //                            NodeFound( DisSysCompOutdoorAirData( OAFanNum ).InletNode
                        //                            ) = true;
                    }
                    for (OARelNum = 1; OARelNum <= NumOfReliefFans; ++OARelNum) {
                        DisSysCompReliefAirData(OARelNum).OutletNode =
                            GetOAMixerInletNodeNumber(m_state, DisSysCompReliefAirData(OARelNum).OAMixerNum);
                        //                            NodeFound( DisSysCompOutdoorAirData( OAFanNum ).InletNode
                        //                            ) = true;
                    }
                    // Check NodeFound status
                    for (OAMixerNum = 1; OAMixerNum <= GetNumOAMixers(m_state); ++OAMixerNum) {
                        if (i == GetOAMixerReliefNodeNumber(m_state, OAMixerNum)) {
                            NodeFound(i) = true;
                            break;
                        } else if (i == GetOAMixerInletNodeNumber(m_state, OAMixerNum)) {
                            NodeFound(i) = true;
                            break;
                        } else {
                            if (OAMixerNum == GetNumOAMixers(m_state)) {
                                ShowSevereError(m_state,
                                                format(RoutineName) + "'" + m_state.dataLoopNodes->NodeID(i) +
                                                    "' is not defined as an AirflowNetwork:Distribution:Node object.");
                                ErrorsFound = true;
                            }
                        }
                    }
                } else if (GetNumOAMixers(m_state) == 0) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + "'" + m_state.dataLoopNodes->NodeID(i) +
                                        "' is not defined as an AirflowNetwork:Distribution:Node object.");
                    ErrorsFound = true;
                } else {
                    // TODO: I fail to see how you could enter this block given than NumOAMixers (returned by GetNumOAMixers())
                    // is initialized to zero, and we check above if '> 0' or '== 0'
                    if (NumOfOAFans == 1 && DisSysCompOutdoorAirData(1).InletNode == 0) {
                        DisSysCompOutdoorAirData(1).InletNode = GetOAMixerInletNodeNumber(m_state, 1);
                    }
                    if (NumOfReliefFans == 1 && DisSysCompReliefAirData(1).OutletNode == 0) {
                        DisSysCompReliefAirData(1).OutletNode = GetOAMixerInletNodeNumber(m_state, 1);
                    }
                    if (i == GetOAMixerReliefNodeNumber(m_state, 1)) {
                        NodeFound(i) = true;
                    } else if (i == GetOAMixerInletNodeNumber(m_state, 1)) {
                        NodeFound(i) = true;
                    } else {
                        ShowSevereError(m_state,
                                        format(RoutineName) + "'" + m_state.dataLoopNodes->NodeID(i) +
                                            "' is not defined as an AirflowNetwork:Distribution:Node object.");
                        ErrorsFound = true;
                    }
                }
            }
        }
        if (hpwhFound) {
            ShowWarningError(m_state,
                             format(RoutineName) + "Heat pump water heater is simulated along with an AirflowNetwork but is not included in "
                                                   "the AirflowNetwork.");
        }
        if (standaloneERVFound) {
            ShowWarningError(m_state,
                             format(RoutineName) + "A ZoneHVAC:EnergyRecoveryVentilator is simulated along with an AirflowNetwork but is not "
                                                   "included in the AirflowNetwork.");
        }
        if (packagedUnitaryFound) {
            ShowWarningError(m_state,
                             format(RoutineName) + "A ZoneHVAC:PackagedTerminalAirConditioner, ZoneHVAC:PackagedTerminalHeatPump, or "
                                                   "ZoneHVAC:WaterToAirHeatPump is simulated along with an AirflowNetwork but is not "
                                                   "included in the AirflowNetwork.");
        }
        if (vrfTUFound) {
            ShowWarningError(m_state,
                             format(RoutineName) +
                                 "A ZoneHVAC:TerminalUnit:VariableRefrigerantFlow is simulated along with an AirflowNetwork but is not "
                                 "included in the AirflowNetwork.");
        }
        if (windowACFound) {
            ShowWarningError(m_state,
                             format(RoutineName) + "A ZoneHVAC:WindowAirConditioner is simulated along with an AirflowNetwork but is not "
                                                   "included in the AirflowNetwork.");
        }
        NodeFound.deallocate();

        // Assign AirLoop Number to every node and linkage
        // Zone first
        for (int i = 1; i <= AirflowNetworkNumOfZones; i++) {
            for (int j = 1; j <= m_state.dataGlobal->NumOfZones; j++) {
                if (!m_state.dataZoneEquip->ZoneEquipConfig(j).IsControlled) {
                    continue;
                }
                if ((MultizoneZoneData(i).ZoneNum == j) && (m_state.dataZoneEquip->ZoneEquipConfig(j).NumInletNodes > 0)) {
                    // No multiple Airloop
                    AirflowNetworkNodeData(i).AirLoopNum = m_state.dataZoneEquip->ZoneEquipConfig(j).InletNodeAirLoopNum(1);
                }
            }
        }
        // Air Distribution system
        for (int i = AirflowNetworkNumOfSurfaces + 1; i <= AirflowNetworkNumOfLinks; ++i) {
            int j = AirflowNetworkLinkageData(i).NodeNums[0];
            int k = AirflowNetworkLinkageData(i).NodeNums[1];
            if (AirflowNetworkNodeData(j).AirLoopNum == 0 && AirflowNetworkNodeData(k).AirLoopNum == 0) {
                // Error messaage
                ShowSevereError(m_state,
                                format(RoutineName) + "AIRFLOWNETWORK:DISTRIBUTION:LINKAGE = " + AirflowNetworkLinkageData(i).Name +
                                    " is not valid for AirLoopNum assignment");
                ShowContinueError(m_state,
                                  "AirLoopNum is not found in both nodes for the linkage: " + AirflowNetworkLinkageData(i).NodeNames[0] + " and " +
                                      AirflowNetworkLinkageData(i).NodeNames[1]);
                ShowContinueError(m_state,
                                  "Please ensure one of two AIRFLOWNETWORK:DISTRIBUTION:NODEs in the first AIRFLOWNETWORK:DISTRIBUTION:LINKAGE "
                                  "object should be defined as EnergyPlus NodeID.");
                ErrorsFound = true;
            }
            if (AirflowNetworkNodeData(j).AirLoopNum > 0 && AirflowNetworkNodeData(k).AirLoopNum == 0) {
                AirflowNetworkNodeData(k).AirLoopNum = AirflowNetworkNodeData(j).AirLoopNum;
            }
            if (AirflowNetworkNodeData(j).AirLoopNum == 0 && AirflowNetworkNodeData(k).AirLoopNum > 0) {
                AirflowNetworkNodeData(j).AirLoopNum = AirflowNetworkNodeData(k).AirLoopNum;
            }
            if (AirflowNetworkNodeData(j).AirLoopNum == AirflowNetworkNodeData(k).AirLoopNum) {
                AirflowNetworkLinkageData(i).AirLoopNum = AirflowNetworkNodeData(j).AirLoopNum;
            }
            if (AirflowNetworkNodeData(j).AirLoopNum != AirflowNetworkNodeData(k).AirLoopNum && AirflowNetworkNodeData(j).AirLoopNum > 0 &&
                AirflowNetworkNodeData(k).AirLoopNum > 0) {
                AirflowNetworkLinkageData(i).AirLoopNum = AirflowNetworkNodeData(j).AirLoopNum;
                ShowSevereError(m_state,
                                "The AirLoopNum defined in both AIRFLOWNETWORK:DISTRIBUTION:NODE objects in " + AirflowNetworkLinkageData(i).Name +
                                    " are not the same. Please make sure both nodes should be listed in the same AirLoop as a valid linkage.");
                ShowContinueError(m_state,
                                  "AirLoop defined in " + AirflowNetworkNodeData(j).Name + " is " +
                                      m_state.dataAirSystemsData->PrimaryAirSystems(AirflowNetworkNodeData(j).AirLoopNum).Name +
                                      ", and AirLoop defined in " + AirflowNetworkNodeData(k).Name + " is " +
                                      m_state.dataAirSystemsData->PrimaryAirSystems(AirflowNetworkNodeData(k).AirLoopNum).Name);
                ErrorsFound = true;
            }
            // Set AirLoopNum to fans and coils
            if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).EPlusTypeNum == iEPlusComponentType::FAN) {
                n = m_state.afn->DisSysCompCVFData(AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).TypeNum).FanIndex;
                m_state.afn->DisSysCompCVFData(AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).TypeNum).AirLoopNum =
                    AirflowNetworkLinkageData(i).AirLoopNum;
                if (m_state.afn->DisSysCompCVFData(AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).TypeNum).FanModelFlag) {
                    m_state.dataFans
                        ->fans(m_state.afn->DisSysCompCVFData(m_state.afn->AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).TypeNum)
                                   .FanIndex)
                        ->airLoopNum = AirflowNetworkLinkageData(i).AirLoopNum;
                    m_state.dataFans
                        ->fans(m_state.afn->DisSysCompCVFData(m_state.afn->AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).TypeNum)
                                   .FanIndex)
                        ->isAFNFan = true;
                } else {
                    m_state.dataFans->fans(n)->airLoopNum = AirflowNetworkLinkageData(i).AirLoopNum;
                    m_state.dataFans->fans(n)->isAFNFan = true;
                }
            }
            if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).EPlusTypeNum == iEPlusComponentType::COI) {
                m_state.afn->DisSysCompCoilData(AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).TypeNum).AirLoopNum =
                    AirflowNetworkLinkageData(i).AirLoopNum;
            }
        }

        // Validate coil name and type
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:Coil";
        MultiSpeedHPIndicator = 0;
        for (int i = 1; i <= DisSysNumOfCoils; ++i) {
            {
                auto const SELECT_CASE_var(Util::makeUPPER(DisSysCompCoilData(i).EPlusType));

                if (SELECT_CASE_var == "COIL:COOLING:DX") {
                    ValidateComponent(m_state, "Coil:Cooling:DX", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        // Replace the convenience function with in-place code
                        std::string mycoil = DisSysCompCoilData(i).name;
                        auto it = std::find_if(m_state.dataCoilCoolingDX->coilCoolingDXs.begin(),
                                               m_state.dataCoilCoolingDX->coilCoolingDXs.end(),
                                               [&mycoil](const CoilCoolingDX &coil) { return coil.name == mycoil; });
                        if (it != m_state.dataCoilCoolingDX->coilCoolingDXs.end()) {
                            // Set the airloop number on the CoilCoolingDX object, which is used to collect the runtime fraction
                            it->airLoopNum = DisSysCompCoilData(i).AirLoopNum;
                        } else {
                            ShowSevereError(m_state, "SetDXCoilAirLoopNumber: Could not find Coil \"Name=\"" + DisSysCompCoilData(i).name + "\"");
                        }
                    }
                } else if (SELECT_CASE_var == "COIL:COOLING:DX:SINGLESPEED") {
                    ValidateComponent(
                        m_state, "Coil:Cooling:DX:SingleSpeed", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetDXCoilAirLoopNumber(m_state, DisSysCompCoilData(i).name, DisSysCompCoilData(i).AirLoopNum);
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:DX:SINGLESPEED") {
                    ValidateComponent(
                        m_state, "Coil:Heating:DX:SingleSpeed", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetDXCoilAirLoopNumber(m_state, DisSysCompCoilData(i).name, DisSysCompCoilData(i).AirLoopNum);
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:FUEL") {
                    ValidateComponent(m_state, "Coil:Heating:Fuel", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetHeatingCoilAirLoopNumber(m_state, DisSysCompCoilData(i).name, DisSysCompCoilData(i).AirLoopNum, ErrorsFound);
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:ELECTRIC") {
                    ValidateComponent(
                        m_state, "Coil:Heating:Electric", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetHeatingCoilAirLoopNumber(m_state, DisSysCompCoilData(i).name, DisSysCompCoilData(i).AirLoopNum, ErrorsFound);
                    }

                } else if (SELECT_CASE_var == "COIL:COOLING:WATER") {
                    ValidateComponent(m_state, "Coil:Cooling:Water", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:WATER") {
                    ValidateComponent(m_state, "Coil:Heating:Water", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "COIL:COOLING:WATER:DETAILEDGEOMETRY") {
                    ValidateComponent(m_state,
                                      "Coil:Cooling:Water:DetailedGeometry",
                                      DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE") {
                    ValidateComponent(m_state,
                                      "Coil:Cooling:DX:TwoStageWithHumidityControlMode",
                                      DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetDXCoilAirLoopNumber(m_state, DisSysCompCoilData(i).name, DisSysCompCoilData(i).AirLoopNum);
                    }

                } else if (SELECT_CASE_var == "COIL:COOLING:DX:MULTISPEED") {
                    ValidateComponent(
                        m_state, "Coil:Cooling:DX:MultiSpeed", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    ++MultiSpeedHPIndicator;
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetDXCoilAirLoopNumber(m_state, DisSysCompCoilData(i).name, DisSysCompCoilData(i).AirLoopNum);
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:DX:MULTISPEED") {
                    ValidateComponent(
                        m_state, "Coil:Heating:DX:MultiSpeed", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    ++MultiSpeedHPIndicator;
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetDXCoilAirLoopNumber(m_state, DisSysCompCoilData(i).name, DisSysCompCoilData(i).AirLoopNum);
                    }

                } else if (SELECT_CASE_var == "COIL:COOLING:DX:VARIABLESPEED") {
                    ValidateComponent(
                        m_state, "Coil:Cooling:DX:VariableSpeed", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    ++MultiSpeedHPIndicator;
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetDXCoilAirLoopNumber(m_state, DisSysCompCoilData(i).name, DisSysCompCoilData(i).AirLoopNum);
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:DX:VARIABLESPEED") {
                    ValidateComponent(
                        m_state, "Coil:Heating:DX:VariableSpeed", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    ++MultiSpeedHPIndicator;
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetDXCoilAirLoopNumber(m_state, DisSysCompCoilData(i).name, DisSysCompCoilData(i).AirLoopNum);
                    }
                } else if (SELECT_CASE_var == "COIL:COOLING:WATERTOAIRHEATPUMP:EQUATIONFIT") {
                    ValidateComponent(m_state,
                                      "Coil:Cooling:WaterToAirHeatPump:EquationFit",
                                      DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:WATERTOAIRHEATPUMP:EQUATIONFIT") {
                    ValidateComponent(m_state,
                                      "Coil:Heating:WaterToAirHeatPump:EquationFit",
                                      DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }
                } else if (SELECT_CASE_var == "COIL:COOLING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT") {
                    ValidateComponent(m_state,
                                      "Coil:Cooling:WaterToAirHeatPump:VariableSpeedEquationFit",
                                      DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT") {
                    ValidateComponent(m_state,
                                      "Coil:Heating:WaterToAirHeatPump:VariableSpeedEquationFit",
                                      DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:DESUPERHEATER") {
                    ValidateComponent(
                        m_state, "Coil:Heating:Desuperheater", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "COIL:COOLING:DX:TWOSPEED") {
                    ValidateComponent(
                        m_state, "Coil:Cooling:DX:TwoSpeed", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetDXCoilAirLoopNumber(m_state, DisSysCompCoilData(i).name, DisSysCompCoilData(i).AirLoopNum);
                    }
                } else if (SELECT_CASE_var == "COIL:HEATING:ELECTRIC:MULTISTAGE") {
                    ValidateComponent(
                        m_state, "Coil:Heating:Electric:MultiStage", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetHeatingCoilAirLoopNumber(m_state, DisSysCompCoilData(i).name, DisSysCompCoilData(i).AirLoopNum, ErrorsFound);
                    }
                } else if (SELECT_CASE_var == "COIL:HEATING:GAS:MULTISTAGE") {
                    ValidateComponent(
                        m_state, "Coil:Heating:Gas:MultiStage", DisSysCompCoilData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetHeatingCoilAirLoopNumber(m_state, DisSysCompCoilData(i).name, DisSysCompCoilData(i).AirLoopNum, ErrorsFound);
                    }
                } else {
                    ShowSevereError(m_state, format(RoutineName) + CurrentModuleObject + " Invalid coil type = " + DisSysCompCoilData(i).name);
                    ErrorsFound = true;
                }
            }
        }

        // Validate terminal unit name and type
        for (int i = 1; i <= DisSysNumOfTermUnits; ++i) {
            if (Util::SameString(DisSysCompTermUnitData(i).EPlusType, "AirTerminal:SingleDuct:ConstantVolume:Reheat") ||
                Util::SameString(DisSysCompTermUnitData(i).EPlusType, "AirTerminal:SingleDuct:VAV:Reheat")) {
                LocalError = false;
                if (Util::SameString(DisSysCompTermUnitData(i).EPlusType, "AirTerminal:SingleDuct:ConstantVolume:Reheat")) {
                    GetHVACSingleDuctSysIndex(
                        m_state, DisSysCompTermUnitData(i).name, n, LocalError, "AirflowNetwork:Distribution:Component:TerminalUnit");
                }
                if (Util::SameString(DisSysCompTermUnitData(i).EPlusType, "AirTerminal:SingleDuct:VAV:Reheat")) {
                    GetHVACSingleDuctSysIndex(m_state,
                                              DisSysCompTermUnitData(i).name,
                                              n,
                                              LocalError,
                                              "AirflowNetwork:Distribution:Component:TerminalUnit",
                                              DisSysCompTermUnitData(i).DamperInletNode,
                                              DisSysCompTermUnitData(i).DamperOutletNode);
                }
                if (LocalError) {
                    ErrorsFound = true;
                }
                if (VAVSystem) {
                    for (int j = 1; j <= DisSysNumOfCVFs; j++) {
                        if (DisSysCompCVFData(j).fanType == HVAC::FanType::VAV) {
                            if (DisSysCompCVFData(j).AirLoopNum == DisSysCompTermUnitData(i).AirLoopNum &&
                                !Util::SameString(DisSysCompTermUnitData(i).EPlusType, "AirTerminal:SingleDuct:VAV:Reheat")) {
                                ShowSevereError(m_state,
                                                format(RoutineName) + CurrentModuleObject +
                                                    " Invalid terminal type for a VAV system = " + DisSysCompTermUnitData(i).name);
                                ShowContinueError(m_state, "The input type = " + DisSysCompTermUnitData(i).EPlusType);
                                ShowContinueError(m_state, "A VAV system requires all terminal units with type = AirTerminal:SingleDuct:VAV:Reheat");
                                ErrorsFound = true;
                            }
                        }
                    }
                }
            } else {
                ShowSevereError(m_state,
                                format(RoutineName) + "AIRFLOWNETWORK:DISTRIBUTION:COMPONENT TERMINAL UNIT: Invalid Terminal unit type = " +
                                    DisSysCompTermUnitData(i).name);
                ErrorsFound = true;
            }
        }

        // Validate heat exchanger name and type
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:HeatExchanger";
        for (int i = 1; i <= DisSysNumOfHXs; ++i) {
            {
                auto const SELECT_CASE_var(Util::makeUPPER(DisSysCompHXData(i).EPlusType));

                if (SELECT_CASE_var == "HEATEXCHANGER:AIRTOAIR:FLATPLATE") {
                    ValidateComponent(
                        m_state, "HeatExchanger:AirToAir:FlatPlate", DisSysCompHXData(i).name, IsNotOK, format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "HEATEXCHANGER:AIRTOAIR:SENSIBLEANDLATENT") {
                    ValidateComponent(m_state,
                                      "HeatExchanger:AirToAir:SensibleAndLatent",
                                      DisSysCompHXData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "HEATEXCHANGER:DESICCANT:BALANCEDFLOW") {
                    ValidateComponent(m_state,
                                      "HeatExchanger:Desiccant:BalancedFlow",
                                      DisSysCompHXData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else {
                    ShowSevereError(m_state,
                                    format(RoutineName) + CurrentModuleObject + " Invalid heat exchanger type = " + DisSysCompHXData(i).EPlusType);
                    ErrorsFound = true;
                }
            }
        }

        // Assign supply and return connection
        for (int j = 1; j <= NumPrimaryAirSys; ++j) {
            int S1 = 0;
            int S2 = 0;
            int R1 = 0;
            int R2 = 0;
            for (int i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
                if (AirflowNetworkNodeData(i).EPlusNodeNum == m_state.dataAirLoop->AirToZoneNodeInfo(j).AirLoopSupplyNodeNum(1)) {
                    S1 = i;
                }
                if (AirflowNetworkNodeData(i).EPlusNodeNum == m_state.dataAirLoop->AirToZoneNodeInfo(j).ZoneEquipSupplyNodeNum(1)) {
                    S2 = i;
                }
                if (AirflowNetworkNodeData(i).EPlusNodeNum == m_state.dataAirLoop->AirToZoneNodeInfo(j).ZoneEquipReturnNodeNum(1)) {
                    R1 = i;
                }
                if (AirflowNetworkNodeData(i).EPlusNodeNum == m_state.dataAirLoop->AirToZoneNodeInfo(j).AirLoopReturnNodeNum(1)) {
                    R2 = i;
                }
            }
            for (int i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
                if (AirflowNetworkLinkageData(i).NodeNums[0] == R1 && AirflowNetworkLinkageData(i).NodeNums[1] == R2) {
                    AirflowNetworkLinkageData(i).ConnectionFlag = iEPlusComponentType::RCN;
                }
                if (AirflowNetworkLinkageData(i).NodeNums[0] == S1 && AirflowNetworkLinkageData(i).NodeNums[1] == S2) {
                    AirflowNetworkLinkageData(i).ConnectionFlag = iEPlusComponentType::SCN;
                }
            }
        }

        // Assign fan inlet and outlet node, and coil outlet
        for (int i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
            int j = AirflowNetworkLinkageData(i).CompNum;
            if (AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::CVF) {
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusTypeNum == iEPlusNodeType::Invalid) {
                    AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusTypeNum = iEPlusNodeType::FIN;
                }
                AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusTypeNum = iEPlusNodeType::FOU;
            }
            if (AirflowNetworkCompData(j).EPlusTypeNum == iEPlusComponentType::COI) {
                AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusTypeNum = iEPlusNodeType::COU;
            }
            if (AirflowNetworkCompData(j).EPlusTypeNum == iEPlusComponentType::HEX) {
                AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusTypeNum = iEPlusNodeType::HXO;
            }
            if (AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::TMU) {
                if (DisSysCompTermUnitData(AirflowNetworkCompData(j).TypeNum).DamperInletNode > 0) {
                    if (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusNodeNum ==
                            DisSysCompTermUnitData(AirflowNetworkCompData(j).TypeNum).DamperInletNode &&
                        AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusNodeNum ==
                            DisSysCompTermUnitData(AirflowNetworkCompData(j).TypeNum).DamperOutletNode) {
                        AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusTypeNum = iEPlusNodeType::DIN;
                        AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusTypeNum = iEPlusNodeType::DOU;
                        AirflowNetworkLinkageData(i).VAVTermDamper = true;
                    }
                }
            }
        }

        // Validate the position of constant pressure drop component
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:ConstantPressureDrop";
        for (int i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
            if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::CPD) {
                for (int j = 1; j <= AirflowNetworkNumOfLinks; ++j) {
                    if (AirflowNetworkLinkageData(i).NodeNums[0] == AirflowNetworkLinkageData(j).NodeNums[1]) {
                        if (AirflowNetworkCompData(AirflowNetworkLinkageData(j).CompNum).CompTypeNum != iComponentTypeNum::DWC) {
                            ShowSevereError(m_state,
                                            format(RoutineName) + "An " + CurrentModuleObject + " object (" + AirflowNetworkLinkageData(i).CompName +
                                                ')');
                            ShowContinueError(m_state, "must connect a duct component upstream and not " + AirflowNetworkLinkageData(j).Name);
                            ErrorsFound = true;
                        }
                    }
                }
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusTypeNum == iEPlusNodeType::SPL) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" + AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(m_state,
                                      "does not allow a AirLoopHVAC:ZoneSplitter node = " +
                                          AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).Name);
                    ErrorsFound = true;
                }
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusTypeNum == iEPlusNodeType::SPL) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" + AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(m_state,
                                      "does not allow a AirLoopHVAC:ZoneSplitter node = " +
                                          AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).Name);
                    ErrorsFound = true;
                }
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusTypeNum == iEPlusNodeType::MIX) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" + AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(m_state,
                                      "does not allow a AirLoopHVAC:ZoneMixer node = " +
                                          AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).Name);
                    ErrorsFound = true;
                }
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusTypeNum == iEPlusNodeType::MIX) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" + AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(m_state,
                                      "does not allow a AirLoopHVAC:ZoneMixer node = " +
                                          AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).Name);
                    ErrorsFound = true;
                }
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusNodeNum > 0) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" + AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(m_state,
                                      "does not allow to connect an EnergyPlus node = " +
                                          AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).Name);
                    ErrorsFound = true;
                }
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusNodeNum > 0) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" + AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(m_state,
                                      "does not allow to connect an EnergyPlus node = " +
                                          AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).Name);
                    ErrorsFound = true;
                }
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusZoneNum > 0) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" + AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(m_state,
                                      "does not allow to connect an EnergyPlus zone = " +
                                          AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).Name);
                    ErrorsFound = true;
                }
                if (AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).EPlusZoneNum > 0) {
                    ShowSevereError(m_state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" + AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(m_state,
                                      "does not allow to connect an EnergyPlus zone = " +
                                          AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[1]).Name);
                    ErrorsFound = true;
                }
            }
        }

        for (int i = NumOfNodesMultiZone + 1; i <= AirflowNetworkNumOfNodes; ++i) {
            if (AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::SPL) {
                LocalError = false;
                int j = GetSplitterOutletNumber(m_state, "", 1, LocalError);
                SplitterNodeNumbers.allocate(j + 2);
                SplitterNodeNumbers = GetSplitterNodeNumbers(m_state, "", 1, LocalError);
                if (LocalError) {
                    ErrorsFound = true;
                }
            }
        }

        // Assigning inlet and outlet nodes for a splitter
        for (int i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            if (AirflowNetworkNodeData(i).EPlusNodeNum == SplitterNodeNumbers(1)) {
                if (AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::Invalid) {
                    AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::SPI;
                }
            }
            for (int j = 1; j <= SplitterNodeNumbers(2); ++j) {
                if (AirflowNetworkNodeData(i).EPlusNodeNum == SplitterNodeNumbers(j + 2)) {
                    if (AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::Invalid) {
                        AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::SPO;
                    }
                }
            }
        }

        // Add additional output variables
        if (DisSysNumOfCVFs > 1) {
            bool OnOffFanFlag = false;
            for (int i = 1; i <= DisSysNumOfCVFs; i++) {
                if (DisSysCompCVFData(i).fanType == HVAC::FanType::OnOff && !DisSysCompCVFData(i).FanModelFlag) {
                    OnOffFanFlag = true;
                    break;
                }
                if (DisSysCompCVFData(i).FanModelFlag && DisSysCompCVFData(i).fanType == HVAC::FanType::OnOff) {
                    int fanIndex = Fans::GetFanIndex(m_state, DisSysCompCVFData(i).name);
                    if (m_state.dataFans->fans(fanIndex)->airPathFlag) {
                        DisSysCompCVFData(i).fanType = HVAC::FanType::Constant;
                    } else {
                        OnOffFanFlag = true;
                        break;
                    }
                }
            }
            if (OnOffFanFlag) {
                for (int j = 1; j <= AirflowNetworkNumOfZones; ++j) {
                    if (!m_state.dataZoneEquip->ZoneEquipConfig(AirflowNetworkNodeData(j).EPlusZoneNum).IsControlled) {
                        continue;
                    }
                    for (int i = 1; i <= DisSysNumOfCVFs; i++) {
                        if (DisSysCompCVFData(i).AirLoopNum == AirflowNetworkNodeData(j).AirLoopNum &&
                            DisSysCompCVFData(i).fanType != HVAC::FanType::OnOff) {
                            SetupOutputVariable(m_state,
                                                "AFN Node Total Pressure",
                                                Constant::Units::Pa,
                                                AirflowNetworkNodeSimu(j).PZ,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                AirflowNetworkNodeData(j).Name);
                        }
                    }
                }
                for (int i = 1; i <= NumOfLinksMultiZone; ++i) {
                    if (!m_state.dataZoneEquip->ZoneEquipConfig(AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).EPlusZoneNum)
                             .IsControlled) {
                        continue;
                    }
                    for (int j = 1; j <= DisSysNumOfCVFs; j++) {
                        if (DisSysCompCVFData(j).AirLoopNum == AirflowNetworkNodeData(AirflowNetworkLinkageData(i).NodeNums[0]).AirLoopNum &&
                            DisSysCompCVFData(j).fanType != HVAC::FanType::OnOff) {
                            SetupOutputVariable(m_state,
                                                "AFN Linkage Node 1 to Node 2 Mass Flow Rate",
                                                Constant::Units::kg_s,
                                                linkReport(i).FLOW,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                AirflowNetworkLinkageData(i).Name);
                            SetupOutputVariable(m_state,
                                                "AFN Linkage Node 2 to Node 1 Mass Flow Rate",
                                                Constant::Units::kg_s,
                                                linkReport(i).FLOW2,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                AirflowNetworkLinkageData(i).Name);
                            SetupOutputVariable(m_state,
                                                "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
                                                Constant::Units::m3_s,
                                                linkReport(i).VolFLOW,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                AirflowNetworkLinkageData(i).Name);
                            SetupOutputVariable(m_state,
                                                "AFN Linkage Node 2 to Node 1 Volume Flow Rate",
                                                Constant::Units::m3_s,
                                                linkReport(i).VolFLOW2,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                AirflowNetworkLinkageData(i).Name);
                            SetupOutputVariable(m_state,
                                                "AFN Linkage Node 1 to Node 2 Pressure Difference",
                                                Constant::Units::Pa,
                                                AirflowNetworkLinkSimu(i).DP,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                AirflowNetworkLinkageData(i).Name);
                        }
                    }
                }
            }
        }
        bool FanModelConstFlag = false;
        for (int i = 1; i <= DisSysNumOfCVFs; i++) {
            if (DisSysCompCVFData(i).FanModelFlag) {
                int fanIndex = Fans::GetFanIndex(m_state, DisSysCompCVFData(i).name); // What is this accomplishing here?
                if (DisSysCompCVFData(i).fanType == HVAC::FanType::OnOff && m_state.dataFans->fans(fanIndex)->airPathFlag) {
                    DisSysCompCVFData(i).fanType = HVAC::FanType::Constant;
                    supplyFanType = HVAC::FanType::Constant;
                    FanModelConstFlag = true;
                    break;
                }
            }
        }
        if (FanModelConstFlag) {
            for (int i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
                if (supplyFanType == HVAC::FanType::Constant) {
                    SetupOutputVariable(m_state,
                                        "AFN Linkage Node 1 to Node 2 Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        linkReport(i).FLOW,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        AirflowNetworkLinkageData(i).Name);
                    SetupOutputVariable(m_state,
                                        "AFN Linkage Node 2 to Node 1 Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        linkReport(i).FLOW2,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        AirflowNetworkLinkageData(i).Name);
                    SetupOutputVariable(m_state,
                                        "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
                                        Constant::Units::m3_s,
                                        linkReport(i).VolFLOW,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        AirflowNetworkLinkageData(i).Name);
                    SetupOutputVariable(m_state,
                                        "AFN Linkage Node 2 to Node 1 Volume Flow Rate",
                                        Constant::Units::m3_s,
                                        linkReport(i).VolFLOW2,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        AirflowNetworkLinkageData(i).Name);
                    SetupOutputVariable(m_state,
                                        "AFN Linkage Node 1 to Node 2 Pressure Difference",
                                        Constant::Units::Pa,
                                        AirflowNetworkLinkSimu(i).DP,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        AirflowNetworkLinkageData(i).Name);
                }
            }
        }

        // Add AirLoopNum to pressure control object
        for (int i = 1; i <= NumOfPressureControllers; ++i) {
            for (int j = 1; j <= m_state.dataGlobal->NumOfZones; ++j) {
                if (PressureControllerData(i).ZoneNum == j) {
                    for (int k = 1; k <= m_state.dataZoneEquip->ZoneEquipConfig(j).NumInletNodes; ++k) {
                        if (m_state.dataZoneEquip->ZoneEquipConfig(j).InletNodeAirLoopNum(k) > 0) {
                            PressureControllerData(i).AirLoopNum = m_state.dataZoneEquip->ZoneEquipConfig(j).InletNodeAirLoopNum(k);
                            if (PressureControllerData(i).ControlTypeSet == PressureCtrlRelief) {
                                PressureControllerData(i).OANodeNum =
                                    m_state.dataAirSystemsData->PrimaryAirSystems(PressureControllerData(i).AirLoopNum).OAMixOAInNodeNum;
                                for (n = 1; n <= NumOfReliefFans; ++n) {
                                    if (DisSysCompReliefAirData(n).OutletNode == PressureControllerData(i).OANodeNum) {
                                        DisSysCompReliefAirData(n).PressCtrlNum = i;
                                    }
                                }
                            }
                            if (PressureControllerData(i).ControlTypeSet == PressureCtrlExhaust) {
                                PressureControllerData(i).OANodeNum =
                                    m_state.dataZoneEquip->ZoneEquipConfig(PressureControllerData(i).ZoneNum).ExhaustNode(1);
                                for (n = 1; n <= AirflowNetworkNumOfExhFan; ++n) {
                                    if (MultizoneCompExhaustFanData(n).EPlusZoneNum == PressureControllerData(i).ZoneNum) {
                                        MultizoneCompExhaustFanData(n).PressCtrlNum = i;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Check number of fans specified in an AirLoop #6748
        int BranchNum;
        int NumOfFans;
        std::string FanNames;
        for (BranchNum = 1; BranchNum <= m_state.dataAirSystemsData->PrimaryAirSystems(1).NumBranches; ++BranchNum) {
            NumOfFans = 0;
            FanNames = "";
            for (int CompNum = 1; CompNum <= m_state.dataAirSystemsData->PrimaryAirSystems(1).Branch(BranchNum).TotalComponents; ++CompNum) {
                if (Util::SameString(m_state.dataAirSystemsData->PrimaryAirSystems(1).Branch(BranchNum).Comp(CompNum).TypeOf, "Fan:ConstantVolume") ||
                    Util::SameString(m_state.dataAirSystemsData->PrimaryAirSystems(1).Branch(BranchNum).Comp(CompNum).TypeOf, "Fan:OnOff") ||
                    Util::SameString(m_state.dataAirSystemsData->PrimaryAirSystems(1).Branch(BranchNum).Comp(CompNum).TypeOf, "Fan:VariableVolume")) {
                    NumOfFans++;
                    if (NumOfFans > 1) {
                        FanNames += m_state.dataAirSystemsData->PrimaryAirSystems(1).Branch(BranchNum).Comp(CompNum).Name;
                        break;
                    } else {
                        FanNames += m_state.dataAirSystemsData->PrimaryAirSystems(1).Branch(BranchNum).Comp(CompNum).Name + ",";
                    }
                }
            }
            if (NumOfFans > 1) {
                break;
            }
        }
        if (NumOfFans > 1) {
            ShowSevereError(m_state,
                            format(RoutineName) + "An AirLoop branch, " + m_state.dataAirSystemsData->PrimaryAirSystems(1).Branch(BranchNum).Name +
                                ", has two or more fans: " + FanNames);
            ShowContinueError(m_state,
                              "The AirflowNetwork model allows a single supply fan in an AirLoop only. Please make changes in the input "
                              "file accordingly.");
            ErrorsFound = true;
        }

        if (ErrorsFound) {
            ShowFatalError(m_state, format("{}Program terminates for preceding reason(s).", RoutineName));
        }
    }

    void Solver::validate_fan_flowrate()
    {

        // Catch a fan flow rate from EPlus input file and add a flag for VAV terminal damper
        for (int i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
            switch (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum) {
            case iComponentTypeNum::CVF: { // 'CVF'
                int typeNum = AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).TypeNum;
                if (DisSysCompCVFData(typeNum).fanType == HVAC::FanType::VAV) {
                    DisSysCompCVFData(typeNum).MaxAirMassFlowRate =
                        m_state.dataFans->fans(DisSysCompCVFData(typeNum).FanIndex)->maxAirFlowRate * m_state.dataEnvrn->StdRhoAir;
                }
            } break;
            case iComponentTypeNum::FAN:
            case iComponentTypeNum::SOP:
            case iComponentTypeNum::TMU:
                break;
            default:
                break;
            }
        }
    }

    void Solver::validate_exhaust_fan_input()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Dec. 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine validate zone exhaust fan and associated surface

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("AirflowNetwork::Solver::validate_exhaust_fan_input: "); // include trailing blank space

        // Validate supply and return connections
        if (ValidateExhaustFanInputOneTimeFlag) {
            bool ErrorsFound = false;
            std::string const CurrentModuleObject = "AirflowNetwork:MultiZone:Component:ZoneExhaustFan";
            if (std::any_of(m_state.dataZoneEquip->ZoneEquipConfig.begin(),
                            m_state.dataZoneEquip->ZoneEquipConfig.end(),
                            [](DataZoneEquipment::EquipConfiguration const &e) { return e.IsControlled; })) {
                AirflowNetworkZoneExhaustFan.dimension(m_state.dataGlobal->NumOfZones, false);
            }
            // Ensure the number of exhaust fan defined in the AirflowNetwork model matches the number of Zone Exhaust Fan objects
            if (NumOfExhaustFans != AirflowNetworkNumOfExhFan) {
                ShowSevereError(
                    m_state,
                    format("{}The number of {} is not equal to the number of Fan:ZoneExhaust fans defined in ZoneHVAC:EquipmentConnections",
                           RoutineName,
                           CurrentModuleObject));
                ShowContinueError(m_state, format("The number of {} is {}", CurrentModuleObject, AirflowNetworkNumOfExhFan));
                ShowContinueError(m_state,
                                  format("The number of Zone exhaust fans defined in ZoneHVAC:EquipmentConnections is {}", NumOfExhaustFans));
                ErrorsFound = true;
            }

            for (int i = 1; i <= AirflowNetworkNumOfExhFan; ++i) {
                // Get zone number
                for (int j = 1; j <= m_state.dataGlobal->NumOfZones; ++j) {
                    if (!m_state.dataZoneEquip->ZoneEquipConfig(j).IsControlled) {
                        continue;
                    }
                    for (int k = 1; k <= m_state.dataZoneEquip->ZoneEquipConfig(j).NumExhaustNodes; ++k) {
                        if (m_state.dataZoneEquip->ZoneEquipConfig(j).ExhaustNode(k) == MultizoneCompExhaustFanData(i).InletNode) {
                            MultizoneCompExhaustFanData(i).EPlusZoneNum = j;
                            break;
                        }
                    }
                }
                if (MultizoneCompExhaustFanData(i).EPlusZoneNum == 0) {
                    ShowSevereError(m_state,
                                    format("{}Zone name in {} = {} does not match the zone name in ZoneHVAC:EquipmentConnections",
                                           RoutineName,
                                           CurrentModuleObject,
                                           MultizoneCompExhaustFanData(i).name));
                    ErrorsFound = true;
                }
                // Ensure a surface using zone exhaust fan to expose to the same zone
                bool found = false;
                int j;
                for (j = 1; j <= AirflowNetworkNumOfSurfaces; ++j) {
                    if (Util::SameString(MultizoneSurfaceData(j).OpeningName, MultizoneCompExhaustFanData(i).name)) {
                        found = true;
                        if (m_state.dataSurface->Surface(MultizoneSurfaceData(j).SurfNum).ExtBoundCond != ExternalEnvironment &&
                            !(m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtBoundCond == OtherSideCoefNoCalcExt &&
                              m_state.dataSurface->Surface(MultizoneSurfaceData(i).SurfNum).ExtWind)) {
                            ShowSevereError(m_state,
                                            format("{}The surface using {} is not an exterior surface: {}",
                                                   RoutineName,
                                                   CurrentModuleObject,
                                                   MultizoneSurfaceData(j).SurfName));
                            ErrorsFound = true;
                        }
                        break;
                    }
                }
                if (!found) {
                    ShowSevereError(m_state, CurrentModuleObject + "  = " + MultizoneCompExhaustFanData(i).name + " is defined and never used.");
                    ErrorsFound = true;
                } else {
                    if (MultizoneCompExhaustFanData(i).EPlusZoneNum != m_state.dataSurface->Surface(MultizoneSurfaceData(j).SurfNum).Zone) {
                        ShowSevereError(m_state,
                                        format("{}Zone name in {} = {} does not match the zone name",
                                               RoutineName,
                                               CurrentModuleObject,
                                               MultizoneCompExhaustFanData(i).name));
                        ShowContinueError(m_state, "the surface is exposed to " + m_state.dataSurface->Surface(MultizoneSurfaceData(j).SurfNum).Name);
                        ErrorsFound = true;
                    } else {
                        AirflowNetworkZoneExhaustFan(MultizoneCompExhaustFanData(i).EPlusZoneNum) = true;
                    }
                }
            }

            // Ensure all zone exhaust fans are defined
            for (int j = 1; j <= m_state.dataGlobal->NumOfZones; ++j) {
                if (!m_state.dataZoneEquip->ZoneEquipConfig(j).IsControlled) {
                    continue;
                }
                for (int EquipTypeNum = 1; EquipTypeNum <= m_state.dataZoneEquip->ZoneEquipList(j).NumOfEquipTypes; ++EquipTypeNum) {
                    if (m_state.dataZoneEquip->ZoneEquipList(j).EquipType(EquipTypeNum) == DataZoneEquipment::ZoneEquipType::ExhaustFan) {
                        bool found = false;
                        for (int k = 1; k <= m_state.dataZoneEquip->ZoneEquipConfig(j).NumExhaustNodes; ++k) {
                            for (int i = 1; i <= AirflowNetworkNumOfExhFan; ++i) {
                                if (m_state.dataZoneEquip->ZoneEquipConfig(j).ExhaustNode(k) == MultizoneCompExhaustFanData(i).InletNode) {
                                    MultizoneCompExhaustFanData(i).EPlusZoneNum = j;
                                    found = true;
                                }
                            }
                        }
                        if (!found) {
                            ShowSevereError(m_state, format("{}Fan:ZoneExhaust is not defined in {}", RoutineName, CurrentModuleObject));
                            ShowContinueError(
                                m_state,
                                format("The inlet node of the {} Fan:ZoneExhaust is not defined in the {}'s ZoneHVAC:EquipmentConnections",
                                       m_state.dataZoneEquip->ZoneEquipList(j).EquipName,
                                       m_state.dataZoneEquip->ZoneEquipConfig(j).ZoneName));
                            ErrorsFound = true;
                        }
                    }
                }
            }

            ValidateExhaustFanInputOneTimeFlag = false;
            if (ErrorsFound) {
                ShowFatalError(m_state, format("{}Program terminates for preceding reason(s).", RoutineName));
            }
        } // End if OneTimeFlag_FindFirstLastPtr
    }

    int Solver::get_people_index(int const zoneNum, int ventCtrlNum, bool &errorFound)
    {
        int indexResult = 0; // result/reset

        if (zoneNum > 0) { // Find the people data index that matches this zone (if a zone was actually found)
            for (int j = 1; j <= m_state.dataHeatBal->TotPeople; ++j) {
                if (zoneNum == m_state.dataHeatBal->People(j).ZonePtr) {
                    if ((m_state.dataHeatBal->People(j).AdaptiveASH55 && ventCtrlNum == VentControlType::ASH55) ||
                        (m_state.dataHeatBal->People(j).AdaptiveCEN15251 && ventCtrlNum == VentControlType::CEN15251)) {
                        indexResult = j;
                        break;
                    }
                }
            }
        }
        if (indexResult == 0) { // Can result from either not find a zone match in the People statements or not finding a zone
            errorFound = true;
            if (ventCtrlNum == VentControlType::ASH55) {
                ShowSevereError(m_state,
                                "ASHRAE55 ventilation control for zone " + MultizoneZoneData(zoneNum).ZoneName +
                                    " requires connection to a people object that uses ASHRAE55 model calculations.");
            } else if (ventCtrlNum == VentControlType::CEN15251) {
                ShowSevereError(m_state,
                                "CEN15251 ventilation control for zone " + MultizoneZoneData(zoneNum).ZoneName +
                                    " requires connection to a people object that uses CEN15251 model calculations.");
            }
        }
        return indexResult;
    }

    void Solver::hybrid_ventilation_control()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Dec. 2006
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone hybrid ventilation managers

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs hybrid ventilation control

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr IndividualCtrlType(0);                                                                   // Individual window or door control
        int constexpr GlobalCtrlType(1);                                                                       // Global window or door control
        static constexpr std::string_view RoutineName("AirflowNetwork::Solver::hybrid_ventilation_control: "); // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int SysAvailNum;       // Hybrid ventilation control number
        int ControlledZoneNum; // Controlled zone number
        int ANSurfaceNum;      // AirflowNetwork Surface Number
        int SurfNum;           // Surface number
        int ControlType;       // Hybrid ventilation control type: 0 individual; 1 global

        for (auto &e : MultizoneSurfaceData) {
            e.HybridVentClose = false;
            e.HybridCtrlGlobal = false;
            e.HybridCtrlMaster = false;
            e.WindModifier = 1.0;
        }
        ControlType = IndividualCtrlType;

        for (SysAvailNum = 1; SysAvailNum <= m_state.dataAvail->NumHybridVentSysAvailMgrs; ++SysAvailNum) {
            auto &hybridVentMgr = m_state.dataAvail->HybridVentData(SysAvailNum);
            int AirLoopNum = hybridVentMgr.AirLoopNum;
            ventCtrlStatus = hybridVentMgr.ctrlStatus;
            if (hybridVentMgr.afnControlTypeSched != nullptr) {
                ControlType = static_cast<int>(hybridVentMgr.afnControlTypeSched->getCurrentVal());
            }
            bool Found = false; // Logical to indicate whether a master surface is found or not
            int ActualZoneNum = 0;
            for (ControlledZoneNum = 1; ControlledZoneNum <= m_state.dataGlobal->NumOfZones; ++ControlledZoneNum) {
                if (!m_state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).IsControlled) {
                    continue;
                }
                // Ensure all the zones served by this AirLoopHVAC to be controlled by the hybrid ventilation
                for (int zoneInNode = 1; zoneInNode <= m_state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    if (AirLoopNum > 0) {
                        if (AirLoopNum == m_state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode)) {
                            ActualZoneNum = ControlledZoneNum;
                            break;
                        }
                    } else {
                        if (hybridVentMgr.ControlledZoneNum == ControlledZoneNum) {
                            ActualZoneNum = hybridVentMgr.ControlledZoneNum;
                        }
                    }
                }
                if (ActualZoneNum > 0) {
                    for (ANSurfaceNum = 1; ANSurfaceNum <= AirflowNetworkNumOfSurfaces; ++ANSurfaceNum) {
                        SurfNum = MultizoneSurfaceData(ANSurfaceNum).SurfNum;
                        auto const &surf = m_state.dataSurface->Surface(SurfNum);

                        if (surf.Zone == ActualZoneNum) {
                            if (ventCtrlStatus == Avail::VentCtrlStatus::Close) {
                                MultizoneSurfaceData(ANSurfaceNum).HybridVentClose = true;
                            } else {
                                if (hybridVentMgr.WindModifier >= 0) {
                                    MultizoneSurfaceData(ANSurfaceNum).WindModifier = hybridVentMgr.WindModifier;
                                }
                                if (ControlType == GlobalCtrlType) {
                                    MultizoneSurfaceData(ANSurfaceNum).HybridCtrlGlobal = true;
                                    if (hybridVentMgr.Master == ActualZoneNum) {
                                        if ((surf.OriginalClass == SurfaceClass::Window || surf.OriginalClass == SurfaceClass::Door ||
                                             surf.OriginalClass == SurfaceClass::GlassDoor) &&
                                            surf.ExtBoundCond == ExternalEnvironment) {
                                            MultizoneSurfaceData(ANSurfaceNum).HybridCtrlMaster = true;
                                            Found = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (ControlType == GlobalCtrlType && !Found && !m_state.dataGlobal->WarmupFlag && ventCtrlStatus != Avail::VentCtrlStatus::Close) {
                ++HybridGlobalErrCount;
                if (HybridGlobalErrCount < 2) {
                    ShowWarningError(m_state,
                                     format("{}The hybrid ventilation control schedule value indicates global control in the controlled zone = {}",
                                            RoutineName,
                                            m_state.dataHeatBal->Zone(hybridVentMgr.Master).Name));
                    ShowContinueError(m_state,
                                      "The exterior surface containing an opening component in the controlled zone is not found.  No global control "
                                      "will not be modeled.");
                    ShowContinueError(m_state, "The individual control is assumed.");
                    ShowContinueErrorTimeStamp(m_state, "");
                } else {
                    ShowRecurringWarningErrorAtEnd(
                        m_state,
                        format("{}The hybrid ventilation control requires a global control. The individual control continues...", RoutineName),
                        HybridGlobalErrIndex,
                        double(ControlType),
                        double(ControlType));
                }
            }
        }
    }

    void Solver::single_sided_Cps(std::vector<std::vector<Real64>> &valsByFacade, int numWindDir)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Sam Brunswick
        //       DATE WRITTEN   September 2013
        //       MODIFIED       Revised by J. DeGraw, May 2017, to use tables
        //       RE-ENGINEERED  n/a

        // PURPOSE OF THIS SUBROUTINE:
        // Modify the wind pressure coefficients for single sided ventilation.

        // Using/Aliasing
        using namespace DataEnvironment;

        // Locals
        int windDirNum;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int AFNZnNum; // counters
        int SrfNum;
        int ExtOpenNum;
        int ZnNum;
        int DetOpenNum; // row index of surface in MultizoneCompDetOpeningData
        int SimOpenNum; // row index of surface in MultizoneCompSimOpeningData
        int MZDZoneNum; // row index of surface zone in MultizoneZoneData
        Real64 X1;
        Real64 Y1;
        Real64 X2;
        Real64 Y2;
        Real64 ZoneAng1;
        Real64 ZoneAng2;
        Real64 ZoneAngDiff;
        Array1D<Real64> ZoneAng;        // Azimuth angle of the exterior wall of the zone
        Array1D<Real64> PiFormula;      // Formula for the mean pressure difference
        Array1D<Real64> SigmaFormula;   // Formula for the fluctuating pressure difference
        Array1D<Real64> Sprime;         // The dimensionless ratio of the window separation to the building width
        Array1D<Real64> CPV1;           // Wind pressure coefficient for the first opening in the zone
        Array1D<Real64> CPV2;           // Wind pressure coefficient for the second opening in the zone
        Array1D_int NumofExtSurfInZone; // List of the number of exterior openings in each zone

        struct AFNExtSurfacesProp // External opening information
        {
            // Members
            int SurfNum;          // row index of the external opening in the Surface array
            std::string SurfName; // Surface name
            int MSDNum;           // row index of the external opening in the MultizoneSurfaceData array
            int ZoneNum;          // EnergyPlus zone number
            int MZDZoneNum;       // row index of the zone in the MultizoneZoneData array
            int ExtNodeNum;       // External node number; = row index in MultizoneExternalNodeData array +
                                  // AirflowNetworkNumOfZones
            std::string ZoneName; // EnergyPlus zone name
            int facadeNum;
            int curve;                     // wind pressure coefficient curve index
            iComponentTypeNum CompTypeNum; // Opening type (detailed, simple, etc.)
            Real64 NodeHeight;             // Elevation of the opening node
            Real64 OpeningArea;            // Opening area (=Height*Width)
            Real64 Height;                 // Opening height = MultizoneSurfaceData()%Height
            Real64 Width;                  // Opening width  = MultizoneSurfaceData()%Width
            Real64 DischCoeff;             // Opening discharge coefficient

            // Default Constructor
            AFNExtSurfacesProp()
                : SurfNum(0), MSDNum(0), ZoneNum(0), MZDZoneNum(0), ExtNodeNum(0), facadeNum(0), curve(0), CompTypeNum(iComponentTypeNum::Invalid),
                  NodeHeight(0.0), OpeningArea(0.0), Height(0.0), Width(0.0), DischCoeff(0.0)
            {
            }
        };

        // Object Data
        Array1D<AFNExtSurfacesProp> AFNExtSurfaces; // Surface numbers of all exterior openings
        // Count the total number of exterior simple and detailed openings and the number in each zone
        // Verify that each zone with "ADVANCED" single sided wind pressure coefficients has exactly two openings.
        // If it doesn't have two openings, change "ADVANCED" to "STANDARD"
        NumofExtSurfInZone.dimension(AirflowNetworkNumOfZones, 0);
        for (AFNZnNum = 1; AFNZnNum <= AirflowNetworkNumOfZones; ++AFNZnNum) {
            if (MultizoneZoneData(AFNZnNum).SingleSidedCpType == "ADVANCED") {
                for (SrfNum = 1; SrfNum <= AirflowNetworkNumOfSurfaces; ++SrfNum) {
                    if (m_state.dataSurface->Surface(MultizoneSurfaceData(SrfNum).SurfNum).ExtBoundCond ==
                        ExternalEnvironment) { // check if outdoor boundary condition
                        MZDZoneNum = Util::FindItemInList(m_state.dataSurface->Surface(MultizoneSurfaceData(SrfNum).SurfNum).ZoneName,
                                                          MultizoneZoneData,
                                                          &MultizoneZoneProp::ZoneName);
                        if (MZDZoneNum == AFNZnNum) {
                            // This is terrible, should not do it this way
                            auto afe = elements.find(MultizoneSurfaceData(SrfNum).OpeningName);
                            if (afe != elements.end()) {
                                auto type = afe->second->type();
                                if (type == ComponentType::DOP) {
                                    ++AFNNumOfExtOpenings;
                                    ++NumofExtSurfInZone(AFNZnNum);
                                } else if (type == ComponentType::SOP) {
                                    ++AFNNumOfExtOpenings;
                                    ++NumofExtSurfInZone(AFNZnNum);
                                }
                            }
                        }
                    }
                }
                if (NumofExtSurfInZone(AFNZnNum) == 0) {
                    ShowWarningError(m_state,
                                     "AirflowNetwork:Multizone:Zone = " + MultizoneZoneData(AFNZnNum).ZoneName +
                                         " has single side wind pressure coefficient type \"ADVANCED\", but has no exterior "
                                         "AirflowNetwork:MultiZone:Component:DetailedOpening and/or AirflowNetwork:MultiZone:Component:SimpleOpening "
                                         "objects.");
                    ShowContinueError(m_state,
                                      "Zones must have exactly two exterior openings in order for the \"ADVANCED\" single sided wind pressure "
                                      "coefficient model to be used.");
                    ShowContinueError(m_state,
                                      "The wind pressure coefficient model for this zone will be set to \"STANDARD\" and simulation continues.");
                    MultizoneZoneData(AFNZnNum).SingleSidedCpType = "STANDARD";
                } else if (NumofExtSurfInZone(AFNZnNum) == 1) {
                    ShowWarningError(m_state, "AirflowNetwork:Multizone:Zone = " + MultizoneZoneData(AFNZnNum).ZoneName);
                    ShowContinueError(m_state,
                                      "has single side wind pressure coefficient type \"ADVANCED\", but has only one exterior "
                                      "AirflowNetwork:MultiZone:Component:DetailedOpening and/or "
                                      "AirflowNetwork:MultiZone:Component:SimpleOpening objects.");
                    ShowContinueError(m_state,
                                      "Zones must have exactly two openings in order for the \"ADVANCED\" single side wind pressure coefficient "
                                      "model to be used.");
                    ShowContinueError(m_state,
                                      "The wind pressure coefficient model for this zone will be set to \"STANDARD\" and simulation continues.");
                    MultizoneZoneData(AFNZnNum).SingleSidedCpType = "STANDARD";
                } else if (NumofExtSurfInZone(AFNZnNum) > 2) {
                    ShowWarningError(m_state,
                                     format("AirflowNetwork:Multizone:Zone = {} has single side wind pressure coefficient type "
                                            "\"ADVANCED\", but has {} exterior "
                                            "AirflowNetwork:MultiZone:Component:DetailedOpening and/or "
                                            "AirflowNetwork:MultiZone:Component:SimpleOpening objects.",
                                            MultizoneZoneData(AFNZnNum).ZoneName,
                                            NumofExtSurfInZone(AFNZnNum)));
                    ShowContinueError(m_state,
                                      "Zones must have exactly two openings in order for the \"ADVANCED\" single side wind pressure coefficient "
                                      "model to be used.");
                    ShowContinueError(m_state,
                                      "The wind pressure coefficient model for this zone will be set to \"STANDARD\" and simulation continues.");
                    MultizoneZoneData(AFNZnNum).SingleSidedCpType = "STANDARD";
                }
            }
        }
        if (AFNNumOfExtOpenings == 0) {
            return;
        }
        // Recount the number of single sided zones
        AirflowNetworkNumOfSingleSideZones = 0;
        for (AFNZnNum = 1; AFNZnNum <= AirflowNetworkNumOfZones; ++AFNZnNum) {
            if (MultizoneZoneData(AFNZnNum).SingleSidedCpType == "ADVANCED") {
                ++AirflowNetworkNumOfSingleSideZones;
            }
        }
        if (AirflowNetworkNumOfSingleSideZones == 0) {
            return; // Bail if no zones call for the advanced single sided model.
        }
        // Recount the number of detailed and simple exterior openings in zones with "ADVANCED" single sided wind pressure coefficients
        AFNNumOfExtOpenings = 0;
        for (SrfNum = 1; SrfNum <= AirflowNetworkNumOfSurfaces; ++SrfNum) {
            MZDZoneNum = Util::FindItemInList(
                m_state.dataSurface->Surface(MultizoneSurfaceData(SrfNum).SurfNum).ZoneName, MultizoneZoneData, &MultizoneZoneProp::ZoneName);
            if (MultizoneZoneData(MZDZoneNum).SingleSidedCpType == "ADVANCED") {
                if (m_state.dataSurface->Surface(MultizoneSurfaceData(SrfNum).SurfNum).ExtBoundCond ==
                    ExternalEnvironment) { // check if outdoor boundary condition
                    // This is terrible, should not do it this way
                    auto afe = elements.find(MultizoneSurfaceData(SrfNum).OpeningName);
                    if (afe != elements.end()) {
                        auto type = afe->second->type();
                        if (type == ComponentType::DOP) {
                            ++AFNNumOfExtOpenings;
                        } else if (type == ComponentType::SOP) {
                            ++AFNNumOfExtOpenings;
                        }
                    }
                }
            }
        }
        AFNExtSurfaces.allocate(AFNNumOfExtOpenings);
        // Create array of properties for all the exterior single sided openings
        ExtOpenNum = 1;
        for (SrfNum = 1; SrfNum <= AirflowNetworkNumOfSurfaces; ++SrfNum) {
            if (m_state.dataSurface->Surface(MultizoneSurfaceData(SrfNum).SurfNum).ExtBoundCond == ExternalEnvironment) {
                if (AirflowNetworkNumOfDetOpenings > 0) {
                    DetOpenNum = Util::FindItemInList(
                        MultizoneSurfaceData(SrfNum).OpeningName, MultizoneCompDetOpeningData, &AirflowNetwork::DetailedOpening::name);
                    MZDZoneNum = Util::FindItemInList(
                        m_state.dataSurface->Surface(MultizoneSurfaceData(SrfNum).SurfNum).ZoneName, MultizoneZoneData, &MultizoneZoneProp::ZoneName);
                    if (MultizoneZoneData(MZDZoneNum).SingleSidedCpType == "ADVANCED") {
                        if (DetOpenNum > 0) {
                            AFNExtSurfaces(ExtOpenNum).MSDNum = SrfNum;
                            AFNExtSurfaces(ExtOpenNum).SurfNum = MultizoneSurfaceData(SrfNum).SurfNum;
                            AFNExtSurfaces(ExtOpenNum).NodeHeight = m_state.dataSurface->Surface(AFNExtSurfaces(ExtOpenNum).SurfNum).Centroid.z;
                            AFNExtSurfaces(ExtOpenNum).SurfName = m_state.dataSurface->Surface(MultizoneSurfaceData(SrfNum).SurfNum).Name;
                            AFNExtSurfaces(ExtOpenNum).ZoneNum = m_state.dataSurface->Surface(MultizoneSurfaceData(SrfNum).SurfNum).Zone;
                            AFNExtSurfaces(ExtOpenNum).ZoneName = m_state.dataSurface->Surface(MultizoneSurfaceData(SrfNum).SurfNum).ZoneName;
                            AFNExtSurfaces(ExtOpenNum).MZDZoneNum =
                                Util::FindItemInList(AFNExtSurfaces(ExtOpenNum).ZoneName, MultizoneZoneData, &MultizoneZoneProp::ZoneName);
                            AFNExtSurfaces(ExtOpenNum).CompTypeNum = iComponentTypeNum::DOP;
                            AFNExtSurfaces(ExtOpenNum).Height = MultizoneSurfaceData(SrfNum).Height;
                            AFNExtSurfaces(ExtOpenNum).Width = MultizoneSurfaceData(SrfNum).Width;
                            AFNExtSurfaces(ExtOpenNum).OpeningArea =
                                MultizoneSurfaceData(SrfNum).Width * MultizoneSurfaceData(SrfNum).Height * MultizoneSurfaceData(SrfNum).OpenFactor;
                            AFNExtSurfaces(ExtOpenNum).ExtNodeNum = MultizoneSurfaceData(ExtOpenNum).NodeNums[1];
                            AFNExtSurfaces(ExtOpenNum).facadeNum =
                                MultizoneExternalNodeData(AFNExtSurfaces(ExtOpenNum).ExtNodeNum - AirflowNetworkNumOfZones).facadeNum;
                            AFNExtSurfaces(ExtOpenNum).curve =
                                MultizoneExternalNodeData(AFNExtSurfaces(ExtOpenNum).ExtNodeNum - AirflowNetworkNumOfZones).curve;
                            AFNExtSurfaces(ExtOpenNum).DischCoeff = MultizoneCompDetOpeningData(DetOpenNum).DischCoeff2;
                            ++ExtOpenNum;
                        }
                    }
                } else if (AirflowNetworkNumOfSimOpenings > 0) {
                    SimOpenNum = Util::FindItemInList(
                        MultizoneSurfaceData(SrfNum).OpeningName, MultizoneCompSimpleOpeningData, &AirflowNetwork::SimpleOpening::name);
                    if (SimOpenNum > 0) {
                        AFNExtSurfaces(ExtOpenNum).MSDNum = SrfNum;
                        AFNExtSurfaces(ExtOpenNum).SurfNum = MultizoneSurfaceData(SrfNum).SurfNum;
                        AFNExtSurfaces(ExtOpenNum).SurfName = m_state.dataSurface->Surface(MultizoneSurfaceData(SrfNum).SurfNum).Name;
                        AFNExtSurfaces(ExtOpenNum).ZoneNum = m_state.dataSurface->Surface(MultizoneSurfaceData(SrfNum).SurfNum).Zone;
                        AFNExtSurfaces(ExtOpenNum).ZoneName = m_state.dataSurface->Surface(MultizoneSurfaceData(SrfNum).SurfNum).ZoneName;
                        AFNExtSurfaces(ExtOpenNum).MZDZoneNum =
                            Util::FindItemInList(AFNExtSurfaces(ExtOpenNum).ZoneName, MultizoneZoneData, &MultizoneZoneProp::ZoneName);
                        AFNExtSurfaces(ExtOpenNum).CompTypeNum = iComponentTypeNum::SOP;
                        AFNExtSurfaces(ExtOpenNum).Height = MultizoneSurfaceData(SrfNum).Height;
                        AFNExtSurfaces(ExtOpenNum).Width = MultizoneSurfaceData(SrfNum).Width;
                        AFNExtSurfaces(ExtOpenNum).OpeningArea =
                            MultizoneSurfaceData(SrfNum).Width * MultizoneSurfaceData(SrfNum).Height * MultizoneSurfaceData(SrfNum).OpenFactor;
                        AFNExtSurfaces(ExtOpenNum).ExtNodeNum = MultizoneSurfaceData(ExtOpenNum).NodeNums[1];
                        AFNExtSurfaces(ExtOpenNum).curve =
                            MultizoneExternalNodeData(AFNExtSurfaces(ExtOpenNum).ExtNodeNum - AirflowNetworkNumOfZones).curve;
                        AFNExtSurfaces(ExtOpenNum).DischCoeff = MultizoneCompSimpleOpeningData(SimOpenNum).DischCoeff;
                        ++ExtOpenNum;
                    }
                }
            }
        }
        // Calculate the azimuth and the coordinates of the centroid of each opening.
        // Calculate Sprime and DeltaCp for each zone.
        PiFormula.allocate(numWindDir);
        SigmaFormula.allocate(numWindDir);
        DeltaCp.allocate(AirflowNetworkNumOfZones);
        EPDeltaCP.allocate(AirflowNetworkNumOfZones);
        Sprime.allocate(AirflowNetworkNumOfZones);
        ZoneAng.allocate(AirflowNetworkNumOfZones);
        for (ZnNum = 1; ZnNum <= AirflowNetworkNumOfZones; ++ZnNum) {
            DeltaCp(ZnNum).WindDir.allocate(numWindDir);
            EPDeltaCP(ZnNum).WindDir.allocate(numWindDir);
            for (windDirNum = 1; windDirNum <= numWindDir; ++windDirNum) {
                DeltaCp(ZnNum).WindDir(windDirNum) = 0.0;
                EPDeltaCP(ZnNum).WindDir(windDirNum) = 0.0;
            }
        }
        Sprime = 0.0;
        ZoneAng = 0.0;
        for (ZnNum = 1; ZnNum <= AirflowNetworkNumOfZones; ++ZnNum) {
            if (MultizoneZoneData(ZnNum).SingleSidedCpType == "ADVANCED") {
                OpenNuminZone = 1;
                for (ExtOpenNum = 1; ExtOpenNum <= AFNNumOfExtOpenings; ++ExtOpenNum) {
                    if (OpenNuminZone > 2) {
                        break; // Tuned
                    }
                    if (AFNExtSurfaces(ExtOpenNum).MZDZoneNum == ZnNum) {
                        if (OpenNuminZone == 1) {
                            X1 = m_state.dataSurface->Surface(AFNExtSurfaces(ExtOpenNum).SurfNum).Centroid.x;
                            Y1 = m_state.dataSurface->Surface(AFNExtSurfaces(ExtOpenNum).SurfNum).Centroid.y;
                            ZoneAng1 = m_state.dataSurface->Surface(AFNExtSurfaces(ExtOpenNum).SurfNum).Azimuth;
                            ++OpenNuminZone;
                        } else if (OpenNuminZone == 2) {
                            X2 = m_state.dataSurface->Surface(AFNExtSurfaces(ExtOpenNum).SurfNum).Centroid.x;
                            Y2 = m_state.dataSurface->Surface(AFNExtSurfaces(ExtOpenNum).SurfNum).Centroid.y;
                            ZoneAng2 = m_state.dataSurface->Surface(AFNExtSurfaces(ExtOpenNum).SurfNum).Azimuth;
                            ++OpenNuminZone;
                        }
                    }
                }
                ZoneAngDiff = ZoneAng1 - ZoneAng2;
                if (ZoneAngDiff > 0.01) {
                    ShowWarningError(m_state,
                                     "AirflowNetwork:Multizone:Zone = " + MultizoneZoneData(AFNZnNum).ZoneName +
                                         " has single side wind pressure coefficient type \"ADVANCED\", but has openings which are not coplanar.");
                    ShowContinueError(m_state, "The openings should be coplanar for the model to be valid. Simulation Continues.");
                }
                ZoneAng(ZnNum) = ZoneAng1;
                Sprime(ZnNum) = std::sqrt(pow_2(X1 - X2) + pow_2(Y1 - Y2)) / MultizoneZoneData(ZnNum).BuildWidth;
                // Calculate DeltaCp for each wind direction for each zone
                for (windDirNum = 1; windDirNum <= numWindDir; ++windDirNum) {
                    m_state.dataEnvrn->WindDir = (windDirNum - 1) * 10.0;
                    Real64 WindAng = (windDirNum - 1) * 10.0;
                    IncAng = std::abs(WindAng - ZoneAng(ZnNum));
                    if (std::abs(IncAng) > 180.0) {
                        IncAng -= 360.0;
                    }
                    if (Util::SameString(simulation_control.WPCCntr, "SurfaceAverageCalculation")) {
                        if (std::abs(IncAng) <= 67.5) {
                            PiFormula(windDirNum) = 0.44 * sign(std::sin(2.67 * std::abs(IncAng) * Constant::Pi / 180.0), IncAng);
                        } else if (std::abs(IncAng) <= 180.0) {
                            PiFormula(windDirNum) = -0.69 * sign(std::sin((288 - 1.6 * std::abs(IncAng)) * Constant::Pi / 180.0), IncAng);
                        }
                        SigmaFormula(windDirNum) = 0.423 - 0.00163 * std::abs(IncAng);
                        DeltaCp(ZnNum).WindDir(windDirNum) =
                            (0.02 + (0.346 * std::abs(PiFormula(windDirNum)) + 0.084 * SigmaFormula(windDirNum)) * Sprime(ZnNum));
                    }
                }
            }
        }

        // Calculate the single sided Cp arrays from DeltaCp for each single sided opening
        CPV1.allocate(numWindDir); // These two arrays should probably be removed
        CPV2.allocate(numWindDir);
        CPV1 = 0.0;
        CPV2 = 0.0;
        SrfNum = 6;
        for (ZnNum = 1; ZnNum <= AirflowNetworkNumOfZones; ++ZnNum) {
            if (MultizoneZoneData(ZnNum).SingleSidedCpType == "ADVANCED") {
                OpenNuminZone = 1;
                for (ExtOpenNum = 1; ExtOpenNum <= AFNNumOfExtOpenings; ++ExtOpenNum) {
                    if (OpenNuminZone > 2) {
                        break; // Tuned
                    }
                    if (AFNExtSurfaces(ExtOpenNum).MZDZoneNum == ZnNum) {
                        Real64 const VelRatio_2(std::pow(10.0 / AFNExtSurfaces(ExtOpenNum).NodeHeight, 2.0 * m_state.dataEnvrn->SiteWindExp));
                        Real64 const AFNEExtSurface_fac(0.5 * (1.0 / pow_2(AFNExtSurfaces(ExtOpenNum).DischCoeff)));
                        if (OpenNuminZone == 1) {
                            std::vector<Real64> cpvalues(numWindDir);
                            for (windDirNum = 1; windDirNum <= numWindDir; ++windDirNum) {
                                Real64 unmodifiedValue = valsByFacade[AFNExtSurfaces(ExtOpenNum).facadeNum - 1][windDirNum - 1] +
                                                         AFNEExtSurface_fac * DeltaCp(ZnNum).WindDir(windDirNum);
                                cpvalues[windDirNum - 1] = CPV1(windDirNum) = VelRatio_2 * unmodifiedValue;
                            }
                            valsByFacade.push_back(cpvalues);
                            MultizoneExternalNodeData(AFNExtSurfaces(ExtOpenNum).ExtNodeNum - AirflowNetworkNumOfZones).facadeNum = SrfNum;
                            ++OpenNuminZone;
                            ++SrfNum;
                        } else if (OpenNuminZone == 2) {
                            std::vector<Real64> cpvalues(numWindDir);
                            for (windDirNum = 1; windDirNum <= numWindDir; ++windDirNum) {
                                Real64 unmodifiedValue = valsByFacade[AFNExtSurfaces(ExtOpenNum).facadeNum - 1][windDirNum - 1] -
                                                         AFNEExtSurface_fac * DeltaCp(ZnNum).WindDir(windDirNum);
                                cpvalues[windDirNum - 1] = CPV2(windDirNum) = VelRatio_2 * unmodifiedValue;
                                EPDeltaCP(ZnNum).WindDir(windDirNum) = std::abs(CPV2(windDirNum) - CPV1(windDirNum));
                            }
                            valsByFacade.push_back(cpvalues);
                            MultizoneExternalNodeData(AFNExtSurfaces(ExtOpenNum).ExtNodeNum - AirflowNetworkNumOfZones).facadeNum = SrfNum;
                            ++OpenNuminZone;
                            ++SrfNum;
                        }
                    }
                }
            }
        }
        // Rewrite the CPVNum for all nodes that correspond with a simple or detailed opening
        // Does this loop really do anything?
        for (ZnNum = 1; ZnNum <= AirflowNetworkNumOfZones; ++ZnNum) {
            OpenNuminZone = 1;
            for (ExtOpenNum = 1; ExtOpenNum <= AFNNumOfExtOpenings; ++ExtOpenNum) {
                if (AFNExtSurfaces(ExtOpenNum).MZDZoneNum == ZnNum) {
                    if (OpenNuminZone == 1) {
                        ++OpenNuminZone;
                    } else if (OpenNuminZone == 2) {
                        ++OpenNuminZone;
                    }
                }
            }
        }
    }

    Real64 Solver::zone_OA_change_rate(int const ZoneNum) // hybrid ventilation system controlled zone number
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   May. 2007

        // PURPOSE OF THIS SUBROUTINE:
        // This function outputs air change per hour in a given zone

        auto &TimeStepSys = m_state.dataHVACGlobal->TimeStepSys;
        auto &thisZoneHB = m_state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);

        Real64 CpAir = PsyCpAirFnW(thisZoneHB.airHumRat);
        Real64 RhoAir = PsyRhoAirFnPbTdbW(m_state, m_state.dataEnvrn->OutBaroPress, thisZoneHB.MAT, thisZoneHB.airHumRat);
        Real64 InfilVolume = ((exchangeData(ZoneNum).SumMCp + exchangeData(ZoneNum).SumMVCp) / CpAir / RhoAir) * TimeStepSys * Constant::rSecsInHour;
        Real64 ACH = InfilVolume / (TimeStepSys * m_state.dataHeatBal->Zone(ZoneNum).Volume);

        return ACH;
    }

    int Solver::get_airloop_number(int const NodeNumber) // Get air loop number for each distribution node and linkage
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Feb. 2018

        // PURPOSE OF THIS SUBROUTINE:
        // This function outputs an AirLoopNum based on node number

        // Using/Aliasing
        using BranchNodeConnections::GetChildrenData;
        using BranchNodeConnections::GetNumChildren;
        using BranchNodeConnections::IsParentObject;
        auto &NumPrimaryAirSys = m_state.dataHVACGlobal->NumPrimaryAirSys;
        using SingleDuct::GetHVACSingleDuctSysIndex;

        // Return value
        int AirLoopNumber = 0;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int BranchNum;
        int NumOfNodes;
        int NodeNum;
        int OutNum;
        int SupAirPath;
        int SupAirPathOutNodeNum;
        int CtrlZoneNum;
        int ZoneInNum;
        int ZoneOutNum;
        int AirLoopNum;
        int TUNum = 0;
        int TermNum = 0;
        bool LocalError;
        int NumOfComp;
        int NumOfSubComp;
        bool ErrorsFound;
        int NumOfSubSubComp;

        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            // Check OAMixer OA inlet node
            if (NodeNumber == m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).OAMixOAInNodeNum) {
                return AirLoopNum;
            }
            // Check branch
            for (BranchNum = 1; BranchNum <= m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches; ++BranchNum) {
                NumOfNodes = m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).TotalNodes;
                for (NodeNum = 1; NodeNum <= NumOfNodes; ++NodeNum) {
                    if (NodeNumber == m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).NodeNum(NodeNum)) {
                        return AirLoopNum;
                    }
                }
                for (NumOfComp = 1; NumOfComp <= m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).TotalComponents;
                     ++NumOfComp) {
                    if (NodeNumber == m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(NumOfComp).NodeNumIn) {
                        return AirLoopNum;
                    }
                    if (NodeNumber == m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(NumOfComp).NodeNumOut) {
                        return AirLoopNum;
                    }
                    if (m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(NumOfComp).NumSubComps == 0) {
                        DataLoopNode::ConnectionObjectType TypeOfComp = static_cast<DataLoopNode::ConnectionObjectType>(EnergyPlus::getEnumValue(
                            BranchNodeConnections::ConnectionObjectTypeNamesUC,
                            m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(NumOfComp).TypeOf));
                        std::string const &NameOfComp =
                            m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(NumOfComp).Name;
                        if (IsParentObject(m_state, TypeOfComp, NameOfComp)) {

                            int NumChildren = GetNumChildren(m_state, TypeOfComp, NameOfComp);
                            EPVector<DataLoopNode::ConnectionObjectType> SubCompTypes;
                            Array1D_string SubCompNames;
                            Array1D_string InletNodeNames;
                            Array1D_int InletNodeNumbers;
                            Array1D_string OutletNodeNames;
                            Array1D_int OutletNodeNumbers;

                            SubCompTypes.allocate(NumChildren);
                            SubCompNames.allocate(NumChildren);
                            InletNodeNames.allocate(NumChildren);
                            InletNodeNumbers.allocate(NumChildren);
                            OutletNodeNames.allocate(NumChildren);
                            OutletNodeNumbers.allocate(NumChildren);

                            GetChildrenData(m_state,
                                            TypeOfComp,
                                            NameOfComp,
                                            NumChildren,
                                            SubCompTypes,
                                            SubCompNames,
                                            InletNodeNames,
                                            InletNodeNumbers,
                                            OutletNodeNames,
                                            OutletNodeNumbers,
                                            ErrorsFound);

                            for (NumOfSubComp = 1; NumOfSubComp <= NumChildren; ++NumOfSubComp) {
                                if (NodeNumber == InletNodeNumbers(NumOfSubComp)) {
                                    SubCompTypes.deallocate();
                                    SubCompNames.deallocate();
                                    InletNodeNames.deallocate();
                                    InletNodeNumbers.deallocate();
                                    OutletNodeNames.deallocate();
                                    OutletNodeNumbers.deallocate();
                                    return AirLoopNum;
                                }
                                if (NodeNumber == OutletNodeNumbers(NumOfSubComp)) {
                                    SubCompTypes.deallocate();
                                    SubCompNames.deallocate();
                                    InletNodeNames.deallocate();
                                    InletNodeNumbers.deallocate();
                                    OutletNodeNames.deallocate();
                                    OutletNodeNumbers.deallocate();
                                    return AirLoopNum;
                                }
                            }
                            for (NumOfSubComp = 1; NumOfSubComp <= NumChildren; ++NumOfSubComp) {
                                DataLoopNode::ConnectionObjectType TypeOfSubComp = SubCompTypes(NumOfSubComp);
                                std::string NameOfSubComp = SubCompNames(NumOfSubComp);
                                if (IsParentObject(m_state, TypeOfSubComp, NameOfSubComp)) {

                                    int NumGrandChildren = GetNumChildren(m_state, TypeOfSubComp, NameOfSubComp);
                                    EPVector<DataLoopNode::ConnectionObjectType> SubSubCompTypes;
                                    Array1D_string SubSubCompNames;
                                    Array1D_string SubSubInletNodeNames;
                                    Array1D_int SubSubInletNodeNumbers;
                                    Array1D_string SubSubOutletNodeNames;
                                    Array1D_int SubSubOutletNodeNumbers;

                                    SubSubCompTypes.allocate(NumGrandChildren);
                                    SubSubCompNames.allocate(NumGrandChildren);
                                    SubSubInletNodeNames.allocate(NumGrandChildren);
                                    SubSubInletNodeNumbers.allocate(NumGrandChildren);
                                    SubSubOutletNodeNames.allocate(NumGrandChildren);
                                    SubSubOutletNodeNumbers.allocate(NumGrandChildren);

                                    GetChildrenData(m_state,
                                                    TypeOfSubComp,
                                                    NameOfSubComp,
                                                    NumGrandChildren,
                                                    SubSubCompTypes,
                                                    SubSubCompNames,
                                                    SubSubInletNodeNames,
                                                    SubSubInletNodeNumbers,
                                                    SubSubOutletNodeNames,
                                                    SubSubOutletNodeNumbers,
                                                    ErrorsFound);
                                    for (int SubSubCompNum = 1; SubSubCompNum <= NumGrandChildren; ++SubSubCompNum) {
                                        if (NodeNumber == SubSubInletNodeNumbers(SubSubCompNum)) {
                                            SubSubCompTypes.deallocate();
                                            SubSubCompNames.deallocate();
                                            SubSubInletNodeNames.deallocate();
                                            SubSubInletNodeNumbers.deallocate();
                                            SubSubOutletNodeNames.deallocate();
                                            SubSubOutletNodeNumbers.deallocate();
                                            SubCompTypes.deallocate();
                                            SubCompNames.deallocate();
                                            InletNodeNames.deallocate();
                                            InletNodeNumbers.deallocate();
                                            OutletNodeNames.deallocate();
                                            OutletNodeNumbers.deallocate();
                                            return AirLoopNum;
                                        }
                                        if (NodeNumber == SubSubOutletNodeNumbers(SubSubCompNum)) {
                                            SubSubCompTypes.deallocate();
                                            SubSubCompNames.deallocate();
                                            SubSubInletNodeNames.deallocate();
                                            SubSubInletNodeNumbers.deallocate();
                                            SubSubOutletNodeNames.deallocate();
                                            SubSubOutletNodeNumbers.deallocate();
                                            SubCompTypes.deallocate();
                                            SubCompNames.deallocate();
                                            InletNodeNames.deallocate();
                                            InletNodeNumbers.deallocate();
                                            OutletNodeNames.deallocate();
                                            OutletNodeNumbers.deallocate();
                                            return AirLoopNum;
                                        }
                                    }
                                    SubSubCompTypes.deallocate();
                                    SubSubCompNames.deallocate();
                                    SubSubInletNodeNames.deallocate();
                                    SubSubInletNodeNumbers.deallocate();
                                    SubSubOutletNodeNames.deallocate();
                                    SubSubOutletNodeNumbers.deallocate();
                                }
                            }

                            SubCompTypes.deallocate();
                            SubCompNames.deallocate();
                            InletNodeNames.deallocate();
                            InletNodeNumbers.deallocate();
                            OutletNodeNames.deallocate();
                            OutletNodeNumbers.deallocate();
                        }
                    } else {
                        for (NumOfSubComp = 1;
                             NumOfSubComp <= m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(NumOfComp).NumSubComps;
                             ++NumOfSubComp) {
                            if (NodeNumber == m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                  .Branch(BranchNum)
                                                  .Comp(NumOfComp)
                                                  .SubComp(NumOfSubComp)
                                                  .NodeNumIn) {
                                return AirLoopNum;
                            }
                            if (NodeNumber == m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                  .Branch(BranchNum)
                                                  .Comp(NumOfComp)
                                                  .SubComp(NumOfSubComp)
                                                  .NodeNumOut) {
                                return AirLoopNum;
                            }
                            for (NumOfSubSubComp = 1; NumOfSubSubComp <= m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                                             .Branch(BranchNum)
                                                                             .Comp(NumOfComp)
                                                                             .SubComp(NumOfSubComp)
                                                                             .NumSubSubComps;
                                 ++NumOfSubSubComp) {
                                if (NodeNumber == m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                      .Branch(BranchNum)
                                                      .Comp(NumOfComp)
                                                      .SubComp(NumOfSubComp)
                                                      .SubSubComp(NumOfSubSubComp)
                                                      .NodeNumIn) {
                                    return AirLoopNum;
                                }
                                if (NodeNumber == m_state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                      .Branch(BranchNum)
                                                      .Comp(NumOfComp)
                                                      .SubComp(NumOfSubComp)
                                                      .SubSubComp(NumOfSubSubComp)
                                                      .NodeNumOut) {
                                    return AirLoopNum;
                                }
                            }
                        }
                    }
                }
            }

            // Check connection between supply and demand
            for (OutNum = 1; OutNum <= m_state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumSupplyNodes; ++OutNum) {
                // AirLoop supply outlet node
                if (m_state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).AirLoopSupplyNodeNum(OutNum) == NodeNumber) {
                    return AirLoopNum;
                }
                if (m_state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(OutNum) == NodeNumber) {
                    return AirLoopNum;
                }
                // supply path
                for (SupAirPath = 1; SupAirPath <= m_state.dataZoneEquip->NumSupplyAirPaths; ++SupAirPath) {
                    if (m_state.dataZoneEquip->SupplyAirPath(SupAirPath).InletNodeNum ==
                        m_state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(OutNum)) {
                        for (SupAirPathOutNodeNum = 1; SupAirPathOutNodeNum <= m_state.dataZoneEquip->SupplyAirPath(SupAirPath).NumOutletNodes;
                             ++SupAirPathOutNodeNum) {
                            if (m_state.dataZoneEquip->SupplyAirPath(SupAirPath).OutletNode(SupAirPathOutNodeNum) == NodeNumber) {
                                return AirLoopNum;
                            }
                            for (TUNum = 1; TUNum <= DisSysNumOfTermUnits; ++TUNum) {
                                if (Util::SameString(DisSysCompTermUnitData(TUNum).EPlusType, "AirTerminal:SingleDuct:VAV:Reheat")) {
                                    LocalError = false;
                                    GetHVACSingleDuctSysIndex(m_state,
                                                              DisSysCompTermUnitData(TUNum).name,
                                                              TermNum,
                                                              LocalError,
                                                              "AirflowNetwork:Distribution:Component:TerminalUnit",
                                                              DisSysCompTermUnitData(TUNum).DamperInletNode,
                                                              DisSysCompTermUnitData(TUNum).DamperOutletNode);
                                    if (m_state.dataZoneEquip->SupplyAirPath(SupAirPath).OutletNode(SupAirPathOutNodeNum) ==
                                        DisSysCompTermUnitData(TUNum).DamperInletNode) {
                                        if (DisSysCompTermUnitData(TUNum).DamperOutletNode == NodeNumber) {
                                            DisSysCompTermUnitData(TUNum).AirLoopNum = AirLoopNum;
                                            return AirLoopNum;
                                        }
                                    }
                                    if (LocalError) {
                                    }
                                }
                            }
                        }
                    }
                }
                // return path
                if (m_state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).AirLoopReturnNodeNum(OutNum) == NodeNumber) {
                    return AirLoopNum;
                }
                if (m_state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipReturnNodeNum(OutNum) == NodeNumber) {
                    return AirLoopNum;
                }
                for (int retPathNum = 1; retPathNum <= m_state.dataZoneEquip->NumReturnAirPaths; ++retPathNum) {
                    if (m_state.dataZoneEquip->ReturnAirPath(retPathNum).OutletNodeNum ==
                        m_state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipReturnNodeNum(1)) {
                        if (m_state.dataZoneEquip->ReturnAirPath(retPathNum).OutletNodeNum == NodeNumber) {
                            return AirLoopNum;
                        }
                    }
                }
                // Supply inlet node

                // Terminal damper node
            }
        }

        for (CtrlZoneNum = 1; CtrlZoneNum <= m_state.dataGlobal->NumOfZones; ++CtrlZoneNum) {
            if (!m_state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).IsControlled) {
                continue;
            }
            for (ZoneInNum = 1; ZoneInNum <= m_state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).NumInletNodes; ++ZoneInNum) {
                if (m_state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNode(ZoneInNum) == NodeNumber) {
                    return m_state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNodeAirLoopNum(ZoneInNum);
                }
            }
            for (ZoneOutNum = 1; ZoneOutNum <= m_state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).NumReturnNodes; ++ZoneOutNum) {
                if (m_state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).ReturnNode(ZoneOutNum) == NodeNumber) {
                    return m_state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).ReturnNodeAirLoopNum(ZoneOutNum);
                }
            }
        }

        return AirLoopNumber;
    }

    void Solver::SizeDucts()
    {
        Real64 constexpr EPS(0.001);
        int constexpr MaxIte(500);
        Real64 constexpr MinVelocity(0.5);  // minimum airflow velocity (m/s)
        Real64 constexpr MaxVelocity(20.0); // maximum airflow velocity (m/s)

        int NodeLoopSupply = 0;
        int NodeLoopReturn = 0;
        int NodeSplitter = 0;
        int NodeMixer = 0;
        int NodeZoneIntlet = 0;
        int NodeZoneReturn = 0;
        int AFNNodeNum;
        int AFNLinkNum;
        int AFNLinkNum1;
        Real64 SumLength = 0.0;
        Real64 DynamicLoss = 0.0;
        Real64 MaxRough = 0.0;
        bool DuctSizingSTFlag;
        bool DuctSizingSBFlag;
        bool DuctSizingRTFlag;
        bool DuctSizingRBFlag;
        Real64 hydraulicDiameter = 0.0;
        Real64 SupplyTrunkD = 0.0;
        Real64 SupplyTrunkArea = 0.0;
        Real64 SupplyBranchD = 0.0;
        Real64 SupplyBranchArea = 0.0;
        Real64 ReturnTrunkD = 0.0;
        Real64 ReturnTrunkArea = 0.0;
        Real64 ReturnBranchD = 0.0;
        Real64 ReturnBranchArea = 0.0;
        Real64 MdotBranch = 0.0;
        int SolFla = 0;

        int NumOfCtrlZones = 0;
        for (int ZoneNum = 1; ZoneNum <= m_state.dataGlobal->NumOfZones; ++ZoneNum) {
            if (!m_state.dataZoneEquip->ZoneEquipConfig(ZoneNum).IsControlled) {
                continue;
            }
            NumOfCtrlZones++;
            for (int EquipTypeNum = 1; EquipTypeNum <= m_state.dataZoneEquip->ZoneEquipList(ZoneNum).NumOfEquipTypes; ++EquipTypeNum) {
                if (m_state.dataZoneEquip->ZoneEquipList(ZoneNum).EquipType(EquipTypeNum) == DataZoneEquipment::ZoneEquipType::AirDistributionUnit) {
                    int AirDistUnitNum = m_state.dataZoneEquip->ZoneEquipList(ZoneNum).EquipIndex(EquipTypeNum);
                    MdotBranch = m_state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).MassFlowRateTU;
                    break;
                }
            }
        }
        if (NumOfCtrlZones != 1) {
            ShowWarningError(m_state, "AirflowNetwork Duct Sizing: The current restriction is limited to a single controlled zone only");
            ShowContinueError(m_state, format("The number of controlled zone is {}", NumOfCtrlZones));
            ShowContinueError(m_state, "..Duct sizing is not performed");
            simulation_control.autosize_ducts = false;
            simulation_control.iWPCCnt = iWPCCntr::Input;
            simulation_control.allow_unsupported_zone_equipment = false;
        }
        Real64 factor = simulation_control.ductSizing.factor;

        NodeLoopSupply = m_state.dataAirLoop->AirToZoneNodeInfo(1).ZoneEquipSupplyNodeNum(1);
        NodeLoopReturn = m_state.dataAirLoop->AirToZoneNodeInfo(1).ZoneEquipReturnNodeNum(1);
        for (AFNNodeNum = 1; AFNNodeNum <= AirflowNetworkNumOfNodes; AFNNodeNum++) {
            if (AirflowNetworkNodeData(AFNNodeNum).EPlusTypeNum == iEPlusNodeType::SPL) {
                NodeSplitter = AFNNodeNum;
            } else if (AirflowNetworkNodeData(AFNNodeNum).EPlusTypeNum == iEPlusNodeType::MIX) {
                NodeMixer = AFNNodeNum;
            } else if (AirflowNetworkNodeData(AFNNodeNum).EPlusNodeNum == m_state.dataAirLoop->AirToZoneNodeInfo(1).ZoneEquipSupplyNodeNum(1)) {
                NodeLoopSupply = AFNNodeNum;
            } else if (AirflowNetworkNodeData(AFNNodeNum).EPlusNodeNum == m_state.dataAirLoop->AirToZoneNodeInfo(1).ZoneEquipReturnNodeNum(1)) {
                NodeLoopReturn = AFNNodeNum;
            } else if (AirflowNetworkNodeData(AFNNodeNum).EPlusTypeNum == iEPlusNodeType::ZIN) {
                NodeZoneIntlet = AFNNodeNum;
            } else if (AirflowNetworkNodeData(AFNNodeNum).EPlusTypeNum == iEPlusNodeType::ZOU) {
                NodeZoneReturn = AFNNodeNum;
            }
        }

        // find trunk ducts
        DuctSizingSTFlag = false;
        DuctSizingSBFlag = false;
        int CompNum = 0;
        int TypeNum = 0;

        for (AFNLinkNum = 1; AFNLinkNum <= AirflowNetworkNumOfLinks; AFNLinkNum++) {
            CompNum = AirflowNetworkLinkageData(AFNLinkNum).CompNum;
            iComponentTypeNum CompTypeNum = AirflowNetworkCompData(CompNum).CompTypeNum;
            SumLength = 0.0;
            DynamicLoss = 0.0;
            MaxRough = 0.0;
            // supply duct trunk
            if (AirflowNetworkLinkageData(AFNLinkNum).NodeNums[0] == NodeLoopSupply) {
                if (AirflowNetworkLinkageData(AFNLinkNum).NodeNums[1] == NodeSplitter) {
                    // A single trunk duct
                    if (CompTypeNum == iComponentTypeNum::DWC && AirflowNetworkLinkageData(AFNLinkNum).ZoneNum > 0) {
                        AirflowNetworkLinkageData(AFNLinkNum).ductLineType = DuctLineType::SupplyTrunk;
                        TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                        auto const &disSysCompDuct = DisSysCompDuctData(TypeNum);
                        SumLength = disSysCompDuct.L;
                        MaxRough = disSysCompDuct.roughness;
                        DynamicLoss = disSysCompDuct.TurDynCoef;
                        DuctSizingSTFlag = true;
                    }
                } else {
                    int NodeNum1 = AirflowNetworkLinkageData(AFNLinkNum).NodeNums[1];
                    int CompNum1;
                    iComponentTypeNum CompTypeNum1;
                    CompNum1 = AirflowNetworkLinkageData(AFNLinkNum).CompNum;
                    CompTypeNum1 = AirflowNetworkCompData(CompNum1).CompTypeNum;
                    if (CompTypeNum1 == iComponentTypeNum::DWC) {
                        AirflowNetworkLinkageData(AFNLinkNum).ductLineType = DuctLineType::SupplyTrunk;
                        TypeNum = AirflowNetworkCompData(CompNum1).TypeNum;
                        auto const &disSysCompDuct = DisSysCompDuctData(TypeNum);
                        SumLength += disSysCompDuct.L;
                        MaxRough = max(MaxRough, disSysCompDuct.roughness);
                        DynamicLoss += disSysCompDuct.TurDynCoef;
                        // NodeNum1 = AirflowNetworkLinkageData(AFNLinkNum).NodeNums[0];
                        DuctSizingSBFlag = true;
                    }
                    while (NodeNum1 != NodeSplitter) {
                        for (AFNLinkNum1 = 1; AFNLinkNum1 <= AirflowNetworkNumOfLinks; AFNLinkNum1++) {
                            if (NodeNum1 != AirflowNetworkLinkageData(AFNLinkNum1).NodeNums[0]) {
                                continue;
                            }
                            if (AirflowNetworkCompData(AirflowNetworkLinkageData(AFNLinkNum1).CompNum).CompTypeNum != iComponentTypeNum::DWC) {
                                continue;
                            }
                            if (NodeNum1 == AirflowNetworkLinkageData(AFNLinkNum1).NodeNums[0]) {
                                CompNum1 = AirflowNetworkLinkageData(AFNLinkNum1).CompNum;
                                CompTypeNum1 = AirflowNetworkCompData(CompNum1).CompTypeNum;
                                if (CompTypeNum1 == iComponentTypeNum::DWC) {
                                    AirflowNetworkLinkageData(AFNLinkNum1).ductLineType = DuctLineType::SupplyTrunk;
                                    TypeNum = AirflowNetworkCompData(CompNum1).TypeNum;
                                    auto const &disSysCompDuct = DisSysCompDuctData(TypeNum);
                                    SumLength += disSysCompDuct.L;
                                    MaxRough = max(MaxRough, disSysCompDuct.roughness);
                                    DynamicLoss += disSysCompDuct.TurDynCoef;
                                    NodeNum1 = AirflowNetworkLinkageData(AFNLinkNum1).NodeNums[1];
                                    DuctSizingSTFlag = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (DuctSizingSTFlag) {
                    Real64 Velocity = 0.0;
                    Real64 flowrate = DisSysCompCVFData(1).FlowRate / m_state.dataEnvrn->StdRhoAir;
                    if (simulation_control.ductSizing.method == DuctSizingMethod::MaxVelocity) {
                        SupplyTrunkD = sqrt(4.0 * flowrate / simulation_control.ductSizing.max_velocity / Constant::Pi);
                        SupplyTrunkArea = SupplyTrunkD * SupplyTrunkD / 4.0 * Constant::Pi;
                    } else {
                        Real64 MaxDiameter = sqrt(4.0 * flowrate / MinVelocity / Constant::Pi);
                        Real64 MinDiameter = sqrt(4.0 * flowrate / MaxVelocity / Constant::Pi);
                        auto &thisState = m_state; // can't use m_state directly in the capture list, just create a reference
                        Real64 const deltaP = simulation_control.ductSizing.supply_trunk_pressure_loss;
                        Real64 const MassFlowRate = DisSysCompCVFData(1).FlowRate;
                        auto f = [&thisState, deltaP, MassFlowRate, SumLength, DynamicLoss, MaxRough](Real64 const D) {
                            return DuctDResidual(thisState, D, deltaP, MassFlowRate, SumLength, DynamicLoss, MaxRough);
                        };
                        General::SolveRoot(m_state, EPS, MaxIte, SolFla, hydraulicDiameter, f, MinDiameter, MaxDiameter);
                        if (SolFla == -1) {
                            if (!m_state.dataGlobal->WarmupFlag) {
                                if (ErrCountDuct == 0) {
                                    ++ErrCountDuct; // TODO: Why is the error count shared among all heat pump units?
                                    ShowWarningError(m_state,
                                                     "AirflowNetwork Duct Autosizing: Iteration limit exceeded calculating Supply Duct Trunk size.");
                                    ShowContinueErrorTimeStamp(m_state, format("Supply Duct Hydronic Diameter={:.2R}", hydraulicDiameter));
                                } else {
                                    ++ErrCountDuct;
                                    ShowRecurringWarningErrorAtEnd(
                                        m_state,
                                        "AirflowNetwork Duct Autosizing: Iteration limit warning exceeding Supply Duct Trunk "
                                        "size. Supply Trunk is calculated using velocity at 5m/s. Simulation continues...",
                                        ErrIndexDuct,
                                        hydraulicDiameter,
                                        hydraulicDiameter);
                                }
                            }
                        } else if (SolFla == -2) {
                            ShowFatalError(
                                m_state,
                                "Duct Autosizing for Supply Trunk calculation failed: iteration limits exceeded. Supply Trunk is calculated "
                                "using velocity at 5m/s.");
                        }
                        if (SolFla < 0) {
                            SupplyTrunkD = sqrt(4.0 * flowrate / 5.0 / Constant::Pi) * factor;
                        } else {
                            SupplyTrunkD = hydraulicDiameter * factor;
                        }
                        SupplyTrunkArea = SupplyTrunkD * SupplyTrunkD / 4.0 * Constant::Pi;
                        Velocity = flowrate / SupplyTrunkArea;
                    }
                    if (simulation_control.ductSizing.method == DuctSizingMethod::VelocityAndLoss) {
                        if (Velocity > simulation_control.ductSizing.max_velocity) {
                            SupplyTrunkD = sqrt(4.0 * flowrate / simulation_control.ductSizing.max_velocity / Constant::Pi);
                            SupplyTrunkArea = SupplyTrunkD * SupplyTrunkD / 4.0 * Constant::Pi;
                            ShowWarningError(
                                m_state, "AirflowNetwork Duct Sizing: Duct Sizing Method = PressureLossWithMaximumVelocity for Supply Trunk size");
                            ShowContinueError(
                                m_state,
                                format("The Maximum Airflow Velocity at {:.1R} is less than calculated velosity at {:.1R} using PressureLoss",
                                       simulation_control.ductSizing.max_velocity,
                                       Velocity));
                            ShowContinueError(m_state, "..The Maximum Airflow Velocity is used to calculate Supply Trunk Diameter");
                        }
                    }
                }
            }
            // supply duct branch
            if (AirflowNetworkLinkageData(AFNLinkNum).NodeNums[1] == NodeZoneIntlet) {
                SumLength = 0.0;
                DynamicLoss = 0.0;
                MaxRough = 0.0;
                if (AirflowNetworkLinkageData(AFNLinkNum).NodeNums[0] == NodeSplitter) {
                    // A single branch duct
                    if (CompTypeNum == iComponentTypeNum::DWC && AirflowNetworkLinkageData(AFNLinkNum).ZoneNum > 0) {
                        AirflowNetworkLinkageData(AFNLinkNum).ductLineType = DuctLineType::SupplyTrunk;
                        TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                        auto const &disSysCompDuct = DisSysCompDuctData(TypeNum);
                        SumLength = disSysCompDuct.L;
                        MaxRough = disSysCompDuct.roughness;
                        DynamicLoss = disSysCompDuct.TurDynCoef;
                        DuctSizingSTFlag = true;
                    }
                } else {
                    int NodeNum1 = AirflowNetworkLinkageData(AFNLinkNum).NodeNums[0];
                    int CompNum1;
                    iComponentTypeNum CompTypeNum1;
                    CompNum1 = AirflowNetworkLinkageData(AFNLinkNum).CompNum;
                    CompTypeNum1 = AirflowNetworkCompData(CompNum1).CompTypeNum;
                    if (CompTypeNum1 == iComponentTypeNum::DWC) {
                        AirflowNetworkLinkageData(AFNLinkNum).ductLineType = DuctLineType::SupplyBranch;
                        TypeNum = AirflowNetworkCompData(CompNum1).TypeNum;
                        auto const &disSysCompDuct = DisSysCompDuctData(TypeNum);
                        SumLength += disSysCompDuct.L;
                        MaxRough = max(MaxRough, disSysCompDuct.roughness);
                        DynamicLoss += disSysCompDuct.TurDynCoef;
                        DuctSizingSBFlag = true;
                    }
                    while (NodeNum1 != NodeSplitter) {
                        for (AFNLinkNum1 = 1; AFNLinkNum1 <= AirflowNetworkNumOfLinks; AFNLinkNum1++) {
                            if (NodeNum1 != AirflowNetworkLinkageData(AFNLinkNum1).NodeNums[1]) {
                                continue;
                            }
                            if (AirflowNetworkCompData(AirflowNetworkLinkageData(AFNLinkNum1).CompNum).CompTypeNum != iComponentTypeNum::DWC) {
                                continue;
                            }
                            if (NodeNum1 == AirflowNetworkLinkageData(AFNLinkNum1).NodeNums[1]) {
                                CompNum1 = AirflowNetworkLinkageData(AFNLinkNum1).CompNum;
                                CompTypeNum1 = AirflowNetworkCompData(CompNum1).CompTypeNum;
                                if (CompTypeNum1 == iComponentTypeNum::DWC) {
                                    AirflowNetworkLinkageData(AFNLinkNum1).ductLineType = DuctLineType::SupplyBranch;
                                    TypeNum = AirflowNetworkCompData(CompNum1).TypeNum;
                                    auto const &disSysCompDuct = DisSysCompDuctData(TypeNum);
                                    SumLength += disSysCompDuct.L;
                                    MaxRough = max(MaxRough, disSysCompDuct.roughness);
                                    DynamicLoss += disSysCompDuct.TurDynCoef;
                                    NodeNum1 = AirflowNetworkLinkageData(AFNLinkNum1).NodeNums[0];
                                    DuctSizingSBFlag = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (DuctSizingSBFlag) {
                    SolFla = 0;
                    Real64 Velocity;
                    Real64 flowrate = MdotBranch / m_state.dataEnvrn->StdRhoAir;
                    if (simulation_control.ductSizing.method == DuctSizingMethod::MaxVelocity) {
                        SupplyBranchD = sqrt(4.0 * flowrate / simulation_control.ductSizing.max_velocity / Constant::Pi);
                        SupplyBranchArea = SupplyBranchD * SupplyBranchD / 4.0 * Constant::Pi;
                    } else {
                        Real64 MaxDiameter = sqrt(4.0 * flowrate / MinVelocity / Constant::Pi);
                        Real64 MinDiameter = sqrt(4.0 * flowrate / MaxVelocity / Constant::Pi);
                        auto &thisState = m_state; // can't use m_state directly in the capture list, just create a reference
                        Real64 const deltaP = simulation_control.ductSizing.supply_branch_pressure_loss;
                        auto f = [&thisState, deltaP, MdotBranch, SumLength, DynamicLoss, MaxRough](Real64 const D) {
                            return DuctDResidual(thisState, D, deltaP, MdotBranch, SumLength, DynamicLoss, MaxRough);
                        };
                        General::SolveRoot(m_state, EPS, MaxIte, SolFla, hydraulicDiameter, f, MinDiameter, MaxDiameter);
                        if (SolFla == -1) {
                            if (!m_state.dataGlobal->WarmupFlag) {
                                if (ErrCountDuct == 0) {
                                    ++ErrCountDuct; // TODO: Why is the error count shared among all heat pump units?
                                    ShowWarningError(m_state,
                                                     "AirflowNetwork Duct Autosizing: Iteration limit exceeded calculating Supply Duct Branch size.");
                                    ShowContinueErrorTimeStamp(m_state, format("Supply Duct Hydronic Diameter={:.2R}", hydraulicDiameter));
                                } else {
                                    ++ErrCountDuct;
                                    ShowRecurringWarningErrorAtEnd(
                                        m_state,
                                        "AirflowNetwork Duct Autosizing: Iteration limit warning exceeding Supply Duct Branch "
                                        "size. Supply Branch is calculated using velocity at 5m/s. Simulation continues...",
                                        ErrIndexDuct,
                                        hydraulicDiameter,
                                        hydraulicDiameter);
                                }
                            }
                        } else if (SolFla == -2) {
                            ShowFatalError(
                                m_state,
                                "Duct Autosizing for Supply Branch calculation failed: iteration limits exceeded. Supply Branch is calculated "
                                "using velocity at 5m/s.");
                        }
                        if (SolFla < 0) {
                            SupplyBranchD = sqrt(4.0 * flowrate / 5.0 / Constant::Pi) * factor;
                        } else {
                            SupplyBranchD = hydraulicDiameter * factor;
                        }
                        SupplyBranchArea = SupplyBranchD * SupplyBranchD / 4.0 * Constant::Pi;
                        Velocity = flowrate / SupplyBranchArea;
                    }
                    if (simulation_control.ductSizing.method == DuctSizingMethod::VelocityAndLoss) {
                        if (Velocity > simulation_control.ductSizing.max_velocity) {
                            SupplyBranchD = sqrt(4.0 * flowrate / simulation_control.ductSizing.max_velocity / Constant::Pi);
                            SupplyBranchArea = SupplyBranchD * SupplyBranchD / 4.0 * Constant::Pi;
                            ShowWarningError(
                                m_state, "AirflowNetwork Duct Sizing: Duct Sizing Method = PressureLossWithMaximumVelocity for Supply Branch size");
                            ShowContinueError(
                                m_state,
                                format("The Maximum Airflow Velocity at {:.1R} is less than calculated velosity at {:.1R} using PressureLoss",
                                       simulation_control.ductSizing.max_velocity,
                                       Velocity));
                            ShowContinueError(m_state, "..The Maximum Airflow Velocity is used to calculate Supply Branch Diameter");
                        }
                    }
                }
            }

            DuctSizingRTFlag = false;
            DuctSizingRBFlag = false;

            // return duct trunk
            if (AirflowNetworkLinkageData(AFNLinkNum).NodeNums[1] == NodeLoopReturn) {
                SumLength = 0.0;
                DynamicLoss = 0.0;
                MaxRough = 0.0;
                if (AirflowNetworkLinkageData(AFNLinkNum).NodeNums[0] == NodeMixer) {
                    // A single branch duct
                    if (CompTypeNum == iComponentTypeNum::DWC && AirflowNetworkLinkageData(AFNLinkNum).ZoneNum > 0) {
                        AirflowNetworkLinkageData(AFNLinkNum).ductLineType = DuctLineType::ReturnTrunk;
                        TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                        auto const &disSysCompDuct = DisSysCompDuctData(TypeNum);
                        SumLength = disSysCompDuct.L;
                        MaxRough = disSysCompDuct.roughness;
                        DynamicLoss = disSysCompDuct.TurDynCoef;
                        DuctSizingRTFlag = true;
                    }
                } else {
                    int NodeNum1 = AirflowNetworkLinkageData(AFNLinkNum).NodeNums[0];
                    int CompNum1;
                    iComponentTypeNum CompTypeNum1;
                    CompNum1 = AirflowNetworkLinkageData(AFNLinkNum).CompNum;
                    CompTypeNum1 = AirflowNetworkCompData(CompNum1).CompTypeNum;
                    if (CompTypeNum1 == iComponentTypeNum::DWC) {
                        AirflowNetworkLinkageData(AFNLinkNum).ductLineType = DuctLineType::ReturnTrunk;
                        TypeNum = AirflowNetworkCompData(CompNum1).TypeNum;
                        auto const &disSysCompDuct = DisSysCompDuctData(TypeNum);
                        SumLength += disSysCompDuct.L;
                        MaxRough = max(MaxRough, disSysCompDuct.roughness);
                        DynamicLoss += disSysCompDuct.TurDynCoef;
                        DuctSizingRTFlag = true;
                    }
                    while (NodeNum1 != NodeMixer) {
                        for (AFNLinkNum1 = 1; AFNLinkNum1 <= AirflowNetworkNumOfLinks; AFNLinkNum1++) {
                            if (NodeNum1 != AirflowNetworkLinkageData(AFNLinkNum1).NodeNums[1]) {
                                continue;
                            }
                            if (AirflowNetworkCompData(AirflowNetworkLinkageData(AFNLinkNum1).CompNum).CompTypeNum != iComponentTypeNum::DWC) {
                                continue;
                            }
                            if (NodeNum1 == AirflowNetworkLinkageData(AFNLinkNum1).NodeNums[1]) {
                                CompNum1 = AirflowNetworkLinkageData(AFNLinkNum1).CompNum;
                                CompTypeNum1 = AirflowNetworkCompData(CompNum1).CompTypeNum;
                                if (CompTypeNum1 == iComponentTypeNum::DWC) {
                                    AirflowNetworkLinkageData(AFNLinkNum1).ductLineType = DuctLineType::ReturnTrunk;
                                    TypeNum = AirflowNetworkCompData(CompNum1).TypeNum;
                                    auto const &disSysCompDuct = DisSysCompDuctData(TypeNum);
                                    SumLength += disSysCompDuct.L;
                                    MaxRough = max(MaxRough, disSysCompDuct.roughness);
                                    DynamicLoss += disSysCompDuct.TurDynCoef;
                                    NodeNum1 = AirflowNetworkLinkageData(AFNLinkNum1).NodeNums[0];
                                    DuctSizingRTFlag = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (DuctSizingRTFlag) {
                    SolFla = 0;
                    Real64 Velocity;
                    Real64 flowrate = DisSysCompCVFData(1).FlowRate / m_state.dataEnvrn->StdRhoAir;
                    if (simulation_control.ductSizing.method == DuctSizingMethod::MaxVelocity) {
                        ReturnTrunkD = sqrt(4.0 * flowrate / simulation_control.ductSizing.max_velocity / Constant::Pi);
                        ReturnTrunkArea = ReturnTrunkD * ReturnTrunkD / 4.0 * Constant::Pi;
                    } else {
                        Real64 MaxDiameter = sqrt(4.0 * flowrate / MinVelocity / Constant::Pi);
                        Real64 MinDiameter = sqrt(4.0 * flowrate / MaxVelocity / Constant::Pi);
                        auto &thisState = m_state; // can't use m_state directly in the capture list, just create a reference
                        Real64 const deltaP = simulation_control.ductSizing.return_trunk_pressure_loss;
                        Real64 const massFlowRate = DisSysCompCVFData(1).FlowRate;
                        auto f = [&thisState, deltaP, massFlowRate, SumLength, DynamicLoss, MaxRough](Real64 const D) {
                            return DuctDResidual(thisState, D, deltaP, massFlowRate, SumLength, DynamicLoss, MaxRough);
                        };
                        General::SolveRoot(m_state, EPS, MaxIte, SolFla, hydraulicDiameter, f, MinDiameter, MaxDiameter);
                        if (SolFla == -1) {
                            if (!m_state.dataGlobal->WarmupFlag) {
                                if (ErrCountDuct == 0) {
                                    ++ErrCountDuct; // TODO: Why is the error count shared among all heat pump units?
                                    ShowWarningError(m_state,
                                                     "AirflowNetwork Duct Autosizing: Iteration limit exceeded calculating Return Duct Trunk size.");
                                    ShowContinueErrorTimeStamp(m_state, format("Return Duct Hydronic Diameter={:.2R}", hydraulicDiameter));
                                } else {
                                    ++ErrCountDuct;
                                    ShowRecurringWarningErrorAtEnd(
                                        m_state,
                                        "AirflowNetwork Duct Autosizing: Iteration limit warning exceeding Return Duct Trunk "
                                        "size. Supply Branch is calculated using velocity at 5m/s. Simulation continues...",
                                        ErrIndexDuct,
                                        hydraulicDiameter,
                                        hydraulicDiameter);
                                }
                            }
                        } else if (SolFla == -2) {
                            ShowFatalError(
                                m_state,
                                "Duct Autosizing for Return Trunk calculation failed: iteration limits exceeded. Return Trunk is calculated "
                                "using velocity at 5m/s.");
                        }
                        if (SolFla < 0) {
                            ReturnTrunkD = sqrt(4.0 * flowrate / 5.0 / Constant::Pi) * factor;
                        } else {
                            ReturnTrunkD = hydraulicDiameter * factor;
                        }
                        ReturnTrunkArea = ReturnTrunkD * ReturnTrunkD / 4.0 * Constant::Pi;
                        Velocity = flowrate / SupplyBranchArea;
                    }
                    if (simulation_control.ductSizing.method == DuctSizingMethod::VelocityAndLoss) {
                        if (Velocity > simulation_control.ductSizing.max_velocity) {
                            ReturnTrunkD = sqrt(4.0 * flowrate / simulation_control.ductSizing.max_velocity / Constant::Pi);
                            ReturnTrunkArea = ReturnTrunkD * ReturnTrunkD / 4.0 * Constant::Pi;
                            ShowWarningError(
                                m_state, "AirflowNetwork Duct Sizing: Duct Sizing Method = PressureLossWithMaximumVelocity for Return Trunk size");
                            ShowContinueError(
                                m_state,
                                format("The Maximum Airflow Velocity at {:.1R} is less than calculated velosity at {:.1R} using PressureLoss",
                                       simulation_control.ductSizing.max_velocity,
                                       Velocity));
                            ShowContinueError(m_state, "..The Maximum Airflow Velocity is used to calculate Return Trunk Diameter");
                        }
                    }
                }
            }

            // return duct branch
            if (AirflowNetworkLinkageData(AFNLinkNum).NodeNums[0] == NodeZoneReturn) {
                SumLength = 0.0;
                DynamicLoss = 0.0;
                MaxRough = 0.0;
                if (AirflowNetworkLinkageData(AFNLinkNum).NodeNums[1] == NodeMixer) {
                    // A single trunk duct
                    if (CompTypeNum == iComponentTypeNum::DWC && AirflowNetworkLinkageData(AFNLinkNum).ZoneNum > 0) {
                        AirflowNetworkLinkageData(AFNLinkNum).ductLineType = DuctLineType::ReturnBranch;
                        TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
                        auto const &disSysCompDuct = DisSysCompDuctData(TypeNum);
                        SumLength = disSysCompDuct.L;
                        MaxRough = disSysCompDuct.roughness;
                        DynamicLoss = disSysCompDuct.TurDynCoef;
                        DuctSizingRBFlag = true;
                    }
                } else {
                    int NodeNum1 = AirflowNetworkLinkageData(AFNLinkNum).NodeNums[1];
                    int CompNum1;
                    iComponentTypeNum CompTypeNum1;
                    CompNum1 = AirflowNetworkLinkageData(AFNLinkNum).CompNum;
                    CompTypeNum1 = AirflowNetworkCompData(CompNum1).CompTypeNum;
                    if (CompTypeNum1 == iComponentTypeNum::DWC) {
                        AirflowNetworkLinkageData(AFNLinkNum).ductLineType = DuctLineType::ReturnBranch;
                        TypeNum = AirflowNetworkCompData(CompNum1).TypeNum;
                        auto const &disSysCompDuct = DisSysCompDuctData(TypeNum);
                        SumLength += disSysCompDuct.L;
                        MaxRough = max(MaxRough, disSysCompDuct.roughness);
                        DynamicLoss += disSysCompDuct.TurDynCoef;
                        DuctSizingRBFlag = true;
                    }
                    while (NodeNum1 != NodeMixer) {
                        for (AFNLinkNum1 = 1; AFNLinkNum1 <= AirflowNetworkNumOfLinks; AFNLinkNum1++) {
                            if (NodeNum1 != AirflowNetworkLinkageData(AFNLinkNum1).NodeNums[0]) {
                                continue;
                            }
                            if (AirflowNetworkCompData(AirflowNetworkLinkageData(AFNLinkNum1).CompNum).CompTypeNum != iComponentTypeNum::DWC) {
                                continue;
                            }
                            if (NodeNum1 == AirflowNetworkLinkageData(AFNLinkNum1).NodeNums[0]) {
                                CompNum1 = AirflowNetworkLinkageData(AFNLinkNum1).CompNum;
                                CompTypeNum1 = AirflowNetworkCompData(CompNum1).CompTypeNum;
                                if (CompTypeNum1 == iComponentTypeNum::DWC) {
                                    AirflowNetworkLinkageData(AFNLinkNum1).ductLineType = DuctLineType::ReturnBranch;
                                    TypeNum = AirflowNetworkCompData(CompNum1).TypeNum;
                                    auto const &disSysCompDuct = DisSysCompDuctData(TypeNum);
                                    SumLength += disSysCompDuct.L;
                                    MaxRough = max(MaxRough, disSysCompDuct.roughness);
                                    DynamicLoss += disSysCompDuct.TurDynCoef;
                                    NodeNum1 = AirflowNetworkLinkageData(AFNLinkNum1).NodeNums[1];
                                    DuctSizingRBFlag = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (DuctSizingRBFlag) {
                    SolFla = 0;
                    Real64 Velocity;
                    Real64 flowrate = MdotBranch / m_state.dataEnvrn->StdRhoAir;
                    if (simulation_control.ductSizing.method == DuctSizingMethod::MaxVelocity) {
                        ReturnBranchD = sqrt(4.0 * flowrate / simulation_control.ductSizing.max_velocity / Constant::Pi);
                        ReturnBranchArea = ReturnBranchD * ReturnBranchD / 4.0 * Constant::Pi;
                    } else {
                        Real64 MaxDiameter = sqrt(4.0 * flowrate / MinVelocity / Constant::Pi);
                        Real64 MinDiameter = sqrt(4.0 * flowrate / MaxVelocity / Constant::Pi);
                        auto &thisState = m_state; // can't use m_state directly in the capture list, just create a reference
                        Real64 const deltaP = simulation_control.ductSizing.return_branch_pressure_loss;
                        auto f = [&thisState, deltaP, MdotBranch, SumLength, DynamicLoss, MaxRough](Real64 const D) {
                            return DuctDResidual(thisState, D, deltaP, MdotBranch, SumLength, DynamicLoss, MaxRough);
                        };
                        General::SolveRoot(m_state, EPS, MaxIte, SolFla, hydraulicDiameter, f, MinDiameter, MaxDiameter);
                        if (SolFla == -1) {
                            if (!m_state.dataGlobal->WarmupFlag) {
                                if (ErrCountDuct == 0) {
                                    ++ErrCountDuct; // TODO: Why is the error count shared among all heat pump units?
                                    ShowWarningError(m_state,
                                                     "AirflowNetwork Duct Autosizing: Iteration limit exceeded calculating Return Duct Branch size.");
                                    ShowContinueErrorTimeStamp(m_state, format("Return Duct Hydronic Diameter={:.2R}", hydraulicDiameter));
                                } else {
                                    ++ErrCountDuct;
                                    ShowRecurringWarningErrorAtEnd(
                                        m_state,
                                        "AirflowNetwork Duct Autosizing: Iteration limit warning exceeding Supply Duct Branch "
                                        "size. Return Branch is calculated using velocity at 5m/s. Simulation continues...",
                                        ErrIndexDuct,
                                        hydraulicDiameter,
                                        hydraulicDiameter);
                                }
                            }
                        } else if (SolFla == -2) {
                            ShowFatalError(
                                m_state,
                                "Duct Autosizing for Return Branch calculation failed: iteration limits exceeded. Return Branch is calculated "
                                "using velocity at 5m/s.");
                        }
                        if (SolFla < 0) {
                            ReturnBranchD = sqrt(4.0 * flowrate / 5.0 / Constant::Pi) * factor;
                        } else {
                            ReturnBranchD = hydraulicDiameter * factor;
                        }
                        ReturnBranchArea = ReturnBranchD * ReturnBranchD / 4.0 * Constant::Pi;
                        Velocity = flowrate / ReturnBranchArea;
                    }
                    if (simulation_control.ductSizing.method == DuctSizingMethod::VelocityAndLoss) {
                        if (Velocity > simulation_control.ductSizing.max_velocity) {
                            ReturnBranchD = sqrt(4.0 * flowrate / simulation_control.ductSizing.max_velocity / Constant::Pi);
                            ReturnBranchArea = ReturnBranchD * ReturnBranchD / 4.0 * Constant::Pi;
                            ShowWarningError(
                                m_state, "AirflowNetwork Duct Sizing: Duct Sizing Method = PressureLossWithMaximumVelocity for Return Branch size");
                            ShowContinueError(
                                m_state,
                                format("The Maximum Airflow Velocity at {:.1R} is less than calculated velosity at {:.1R} using PressureLoss",
                                       simulation_control.ductSizing.max_velocity,
                                       Velocity));
                            ShowContinueError(m_state, "..The Maximum Airflow Velocity is used to calculate Return Branch Diameter");
                        }
                    }
                }
            }
        }
        // Assign autosize values in Duct element
        for (AFNLinkNum = 1; AFNLinkNum <= AirflowNetworkNumOfLinks; AFNLinkNum++) {
            CompNum = AirflowNetworkLinkageData(AFNLinkNum).CompNum;
            TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
            auto &disSysCompDuct = DisSysCompDuctData(TypeNum);
            if (AirflowNetworkLinkageData(AFNLinkNum).ductLineType == DuctLineType::SupplyTrunk) {
                disSysCompDuct.hydraulicDiameter = SupplyTrunkD;
                disSysCompDuct.A = SupplyTrunkArea;
                disSysCompDuct.RelRough = disSysCompDuct.roughness / SupplyTrunkD;       // e/D: relative roughness
                disSysCompDuct.RelL = disSysCompDuct.L / SupplyTrunkD;                   // L/D: relative length
                disSysCompDuct.A1 = 1.14 - 0.868589 * std::log(disSysCompDuct.RelRough); // 1.14 - 0.868589*ln(e/D)
                disSysCompDuct.g = disSysCompDuct.A1;                                    // 1/sqrt(Darcy friction factor)
            } else if (AirflowNetworkLinkageData(AFNLinkNum).ductLineType == DuctLineType::SupplyBranch) {
                disSysCompDuct.hydraulicDiameter = SupplyBranchD;
                disSysCompDuct.A = SupplyBranchArea;
                disSysCompDuct.RelRough = disSysCompDuct.roughness / SupplyBranchD;      // e/D: relative roughness
                disSysCompDuct.RelL = disSysCompDuct.L / SupplyBranchD;                  // L/D: relative length
                disSysCompDuct.A1 = 1.14 - 0.868589 * std::log(disSysCompDuct.RelRough); // 1.14 - 0.868589*ln(e/D)
                disSysCompDuct.g = disSysCompDuct.A1;                                    // 1/sqrt(Darcy friction factor)
            } else if (AirflowNetworkLinkageData(AFNLinkNum).ductLineType == DuctLineType::ReturnTrunk) {
                disSysCompDuct.hydraulicDiameter = ReturnTrunkD;
                disSysCompDuct.A = ReturnTrunkArea;
                disSysCompDuct.RelRough = disSysCompDuct.roughness / ReturnTrunkD;       // e/D: relative roughness
                disSysCompDuct.RelL = disSysCompDuct.L / ReturnTrunkD;                   // L/D: relative length
                disSysCompDuct.A1 = 1.14 - 0.868589 * std::log(disSysCompDuct.RelRough); // 1.14 - 0.868589*ln(e/D)
                disSysCompDuct.g = disSysCompDuct.A1;                                    // 1/sqrt(Darcy friction factor)
            } else if (AirflowNetworkLinkageData(AFNLinkNum).ductLineType == DuctLineType::ReturnBranch) {
                disSysCompDuct.hydraulicDiameter = ReturnBranchD;
                disSysCompDuct.A = ReturnBranchArea;
                disSysCompDuct.RelRough = disSysCompDuct.roughness / ReturnBranchD;      // e/D: relative roughness
                disSysCompDuct.RelL = disSysCompDuct.L / ReturnBranchD;                  // L/D: relative length
                disSysCompDuct.A1 = 1.14 - 0.868589 * std::log(disSysCompDuct.RelRough); // 1.14 - 0.868589*ln(e/D)
                disSysCompDuct.g = disSysCompDuct.A1;                                    // 1/sqrt(Darcy friction factor)
            }
        }

        // Print data in eio
        print(m_state.files.eio,
              "! <AirflowNetwork Model:Duct Autosizing>, Linkage Name, Duct Type, Duct Name, Duct Hydraunic Diameter, Duct Cross Section Area\n");

        // Assign autosize values in Duct element
        for (AFNLinkNum = 1; AFNLinkNum <= AirflowNetworkNumOfLinks; AFNLinkNum++) {
            CompNum = AirflowNetworkLinkageData(AFNLinkNum).CompNum;
            TypeNum = AirflowNetworkCompData(CompNum).TypeNum;
            auto &disSysCompDuct = DisSysCompDuctData(TypeNum);
            if (AirflowNetworkLinkageData(AFNLinkNum).ductLineType == DuctLineType::SupplyTrunk) {
                print(m_state.files.eio,
                      "AirflowNetwork Model:Duct Autosizing, {}, Supply Trunk, {}, ",
                      AirflowNetworkLinkageData(AFNLinkNum).Name,
                      disSysCompDuct.name);
                print(m_state.files.eio, "{:.4R},{:.4R}\n", SupplyTrunkD, SupplyTrunkArea);
            }
            if (AirflowNetworkLinkageData(AFNLinkNum).ductLineType == DuctLineType::SupplyBranch) {
                print(m_state.files.eio,
                      "AirflowNetwork Model:Duct Autosizing, {}, Supply Branch, {}, ",
                      AirflowNetworkLinkageData(AFNLinkNum).Name,
                      disSysCompDuct.name);
                print(m_state.files.eio, "{:.4R},{:.4R}\n", SupplyBranchD, SupplyBranchArea);
            }
            if (AirflowNetworkLinkageData(AFNLinkNum).ductLineType == DuctLineType::ReturnTrunk) {
                print(m_state.files.eio,
                      "AirflowNetwork Model:Duct Autosizing, {}, Return Trunk, {}, ",
                      AirflowNetworkLinkageData(AFNLinkNum).Name,
                      disSysCompDuct.name);
                print(m_state.files.eio, "{:.4R},{:.4R}\n", ReturnTrunkD, ReturnTrunkArea);
            }
            if (AirflowNetworkLinkageData(AFNLinkNum).ductLineType == DuctLineType::ReturnBranch) {
                print(m_state.files.eio,
                      "AirflowNetwork Model:Duct Autosizing, {}, Return Branch, {}, ",
                      AirflowNetworkLinkageData(AFNLinkNum).Name,
                      disSysCompDuct.name);
                print(m_state.files.eio, "{:.4R},{:.4R}\n", ReturnBranchD, ReturnBranchArea);
            }
        }
    }

    Real64 Solver::CalcDuctDiameter(Real64 hydraulicDiameter, Real64 DeltaP, Real64 MassFlowrate, Real64 TotalL, Real64 TotalLossCoe, Real64 MaxRough)
    {
        Real64 CalcDeltaP = 0.0;

        Real64 A;
        Real64 A0;
        Real64 A1;
        Real64 A2;
        Real64 B;
        Real64 D;
        Real64 S2;
        Real64 CDM;
        Real64 FL; // friction factor for laminar flow.
        Real64 FT; // friction factor for turbulent flow.
        Real64 FTT;
        Real64 RE; // Reynolds number.
        Real64 ed;
        Real64 ld;
        Real64 g;
        Real64 AA1;
        Real64 velocity;
        Real64 constexpr LamDynCoef(64.0);
        Real64 constexpr LamFriCoef(0.001);
        Real64 constexpr EPS(0.001);
        Real64 constexpr C(0.868589);

        //// Initial guess with 5 m/s
        Real64 flowrate = MassFlowrate / m_state.dataEnvrn->StdRhoAir;

        ed = MaxRough / hydraulicDiameter;
        ld = TotalL / hydraulicDiameter;
        g = 1.14 - 0.868589 * std::log(ed);
        AA1 = g;
        A = hydraulicDiameter * hydraulicDiameter / 4.0 * Constant::Pi;
        Real64 viscosity{AirflowNetwork::AIRDYNAMICVISCOSITY_CONSTEXPR(20)};
        velocity = flowrate / A;

        if (LamFriCoef >= 0.001) {
            A2 = LamFriCoef / (2.0 * m_state.dataEnvrn->StdRhoAir * A * A);
            A1 = (viscosity * LamDynCoef * ld) / (2.0 * m_state.dataEnvrn->StdRhoAir * A * hydraulicDiameter);
            A0 = -DeltaP;
            CDM = std::sqrt(A1 * A1 - 4.0 * A2 * A0);
            FL = (CDM - A1) / (2.0 * A2);
            CDM = 1.0 / CDM;
        } else {
            CDM = (2.0 * m_state.dataEnvrn->StdRhoAir * A * hydraulicDiameter) / (viscosity * LamDynCoef * ld);
            FL = CDM * DeltaP;
        }

        // CDM = (2.0 * m_state.dataEnvrn->StdRhoAir * A * hydraulicDiameter) / (viscosity * LamDynCoef * ld);
        // FL = CDM * DeltaP;

        RE = FL * hydraulicDiameter / (viscosity * A);
        S2 = std::sqrt(2.0 * m_state.dataEnvrn->StdRhoAir * DeltaP) * A;
        FTT = S2 / std::sqrt(ld / pow_2(g) + TotalLossCoe);
        while (true) {
            FT = FTT;
            B = (9.3 * viscosity * A) / (FT * MaxRough);
            D = 1.0 + g * B;
            g -= (g - AA1 + C * std::log(D)) / (1.0 + C * B / D);
            FTT = S2 / std::sqrt(ld / pow_2(g) + TotalLossCoe);
            if (std::abs(FTT - FT) / FTT < EPS) {
                break;
            }
        }
        FT = FTT;

        Real64 f = 1.0 / (g * g);

        // CalcDeltaP = ((FT * TotalL) / hydraulicDiameter + TotalLossCoe) * (m_state.dataEnvrn->StdRhoAir * velocity * velocity / 2.0);
        CalcDeltaP = ((f * TotalL) / hydraulicDiameter + TotalLossCoe) * (m_state.dataEnvrn->StdRhoAir * velocity * velocity / 2.0);

        return CalcDeltaP;
    }

    Real64 DuctDResidual(EnergyPlusData &state,
                         Real64 D, // duct diameter
                         Real64 DeltaP,
                         Real64 MassFlowrate,
                         Real64 TotalL,
                         Real64 TotalLossCoe,
                         Real64 MaxRough)
    {
        Real64 CalcDeltaP = state.afn->CalcDuctDiameter(D, DeltaP, MassFlowrate, TotalL, TotalLossCoe, MaxRough);
        return (CalcDeltaP - DeltaP) / DeltaP;
    }

    void OccupantVentilationControlProp::calc(EnergyPlusData &state,
                                              int const ZoneNum,
                                              Real64 const TimeOpenDuration,
                                              Real64 const TimeCloseDuration,
                                              int &OpeningStatus,
                                              int &OpeningProbStatus,
                                              int &ClosingProbStatus)
    {

        Real64 Tcomfort;    // Thermal comfort temperature
        Real64 ComfortBand; // Thermal comfort band
        Real64 Toperative;  // Operative temperature
        Real64 OutDryBulb;  // Outdoor dry-bulb temperature

        auto &Zone(state.dataHeatBal->Zone);

        if (TimeOpenDuration > 0) {
            if (TimeOpenDuration >= MinOpeningTime) {
                OpeningStatus = OpenStatus::FreeOperation; // free operation
            } else {
                OpeningStatus = OpenStatus::MinCheckForceOpen; // forced to open
            }
        }
        if (TimeCloseDuration > 0) {
            if (TimeCloseDuration >= MinClosingTime) {
                OpeningStatus = OpenStatus::FreeOperation; // free operation
            } else {
                OpeningStatus = OpenStatus::MinCheckForceClose; // forced to close
            }
        }

        if (MinTimeControlOnly) {
            return;
        }

        if (Zone(ZoneNum).LinkedOutAirNode > 0) {
            OutDryBulb = Zone(ZoneNum).OutDryBulbTemp;
        } else {
            OutDryBulb = OutDryBulbTempAt(state, Zone(ZoneNum).Centroid.z);
        }

        if (OutDryBulb < ComfortBouPoint) {
            Tcomfort = CurveValue(state, ComfortLowTempCurveNum, OutDryBulb);
        } else {
            Tcomfort = CurveValue(state, ComfortHighTempCurveNum, OutDryBulb);
        }
        ComfortBand = -0.0028 * (100 - MaxPPD) * (100 - MaxPPD) + 0.3419 * (100 - MaxPPD) - 6.6275;
        Toperative = 0.5 * (state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT +
                            state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MRT);

        if (Toperative > (Tcomfort + ComfortBand)) {
            if (opening_probability(state, ZoneNum, TimeCloseDuration)) {
                OpeningProbStatus = ProbabilityCheck::ForceChange;
                ; // forced to open
            } else {
                OpeningProbStatus = ProbabilityCheck::KeepStatus; // Keep previous status
            }
        } else {
            OpeningProbStatus = ProbabilityCheck::NoAction; // free operation
        }

        if (Toperative < (Tcomfort - ComfortBand)) {
            if (closing_probability(state, TimeOpenDuration)) {
                ClosingProbStatus = ProbabilityCheck::ForceChange; // forced to close
            } else {
                ClosingProbStatus = ProbabilityCheck::KeepStatus; // Keep previous status
            }
        } else {
            ClosingProbStatus = ProbabilityCheck::NoAction; // free operation
        }
    }

    bool
    OccupantVentilationControlProp::opening_probability(EnergyPlusData &state,
                                                        int const ZoneNum,
                                                        Real64 const TimeCloseDuration) // function to perform calculations of opening probability
    {
        Real64 SchValue;
        Real64 RandomValue;
        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
        auto &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(ZoneNum);

        if (TimeCloseDuration < MinClosingTime) {
            return false;
        }
        if (OccupancyCheck) {
            if (state.dataHeatBal->ZoneIntGain(ZoneNum).NOFOCC <= 0.0) {
                return false;
            }
        }

        switch (state.dataHeatBalFanSys->TempControlType(ZoneNum)) {
        case HVAC::SetptType::SingleHeat: {
            if (thisZoneHB.MAT <= zoneTstatSetpt.setptLo) {
                return false;
            }
        } break;

        case HVAC::SetptType::SingleCool: {
            if (thisZoneHB.MAT >= zoneTstatSetpt.setptHi) {
                return false;
            }
        } break;
        case HVAC::SetptType::SingleHeatCool: {
            return false;
        } break;

        case HVAC::SetptType::DualHeatCool: {
            if (thisZoneHB.MAT < zoneTstatSetpt.setptLo || thisZoneHB.MAT > zoneTstatSetpt.setptHi) {
                return false;
            }
        } break;

        default: {
        } break;
        } // switch

        if (openingProbSched == nullptr) {
            return true;
        } else {
            SchValue = openingProbSched->getCurrentVal();
            RandomValue = Real64(rand()) / RAND_MAX;
            if (SchValue > RandomValue) {
                return true;
            } else {
                return false;
            }
        }
    }

    bool OccupantVentilationControlProp::closing_probability(EnergyPlusData &state,
                                                             Real64 const TimeOpenDuration) // function to perform calculations of closing probability
    {
        Real64 SchValue;
        Real64 RandomValue;

        if (TimeOpenDuration < MinOpeningTime) {
            return false;
        }
        if (closingProbSched == nullptr) {
            return true;
        } else {
            SchValue = closingProbSched->getCurrentVal();
            RandomValue = Real64(rand()) / RAND_MAX;
            if (SchValue > RandomValue) {
                return true;
            } else {
                return false;
            }
        }
    }

    void Solver::allocate()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Aug. 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine allocates dynamic arrays for AirflowNetworkSolver.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        iComponentTypeNum j;
        int n;

        // Formats

        // Assume a network to simulate multizone airflow is a subset of the network to simulate air distribution system.
        // Network array size is allocated based on the network of air distribution system.
        // If multizone airflow is simulated only, the array size is allocated based on the multizone network.

        auto &NetworkNumOfLinks = ActualNumOfLinks;
        auto &NetworkNumOfNodes = ActualNumOfNodes;

        NetworkNumOfLinks = AirflowNetworkNumOfLinks;
        NetworkNumOfNodes = AirflowNetworkNumOfNodes;

        AFECTL.allocate(NetworkNumOfLinks);
        AFLOW2.allocate(NetworkNumOfLinks);
        AFLOW.allocate(NetworkNumOfLinks);
        PW.allocate(NetworkNumOfLinks);
        PS.allocate(NetworkNumOfLinks);

        // TZ.allocate(NetworkNumOfNodes);
        // WZ.allocate(NetworkNumOfNodes);
        PZ.allocate(NetworkNumOfNodes);
        // RHOZ.allocate(NetworkNumOfNodes);
        // SQRTDZ.allocate(NetworkNumOfNodes);
        // VISCZ.allocate(NetworkNumOfNodes);
        SUMAF.allocate(NetworkNumOfNodes);

        for (int it = 0; it <= NetworkNumOfNodes + 1; ++it) {
            node_states.emplace_back(properties.density(101325.0, 20.0, 0.0));
        }

        ID.allocate(NetworkNumOfNodes);
        IK.allocate(NetworkNumOfNodes + 1);
#ifdef SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS
        newIK.allocate(NetworkNumOfNodes + 1);
#endif
        AD.allocate(NetworkNumOfNodes);
        SUMF.allocate(NetworkNumOfNodes);

        n = 0;
        for (i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
            j = AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum;
            if (j == iComponentTypeNum::DOP) {
                ++n;
            }
        }

        dos.allocate(AirflowNetworkNumOfLinks, n);

        PB = 101325.0;
        //   LIST = 5
        // LIST = 0;

        for (n = 1; n <= NetworkNumOfNodes; ++n) {
            ID(n) = n;
        }
        for (i = 1; i <= NetworkNumOfLinks; ++i) {
            AFECTL(i) = 1.0;
            AFLOW(i) = 0.0;
            AFLOW2(i) = 0.0;
        }

        for (i = 1; i <= NetworkNumOfNodes; ++i) {
            // TZ(i) = AirflowNetworkNodeSimu(i).TZ;
            // WZ(i) = AirflowNetworkNodeSimu(i).WZ;
            PZ(i) = AirflowNetworkNodeSimu(i).PZ;
            node_states[i].temperature = AirflowNetworkNodeSimu(i).TZ;
            node_states[i].humidity_ratio = AirflowNetworkNodeSimu(i).WZ;
            // properties[i].pressure = AirflowNetworkNodeSimu(i).PZ;
        }

        // Assign linkage values
        for (i = 1; i <= NetworkNumOfLinks; ++i) {
            PW(i) = 0.0;
        }
        // Write an ouput file used for AIRNET input
        /*
        if (LIST >= 5) {
            Unit11 = GetNewUnitNumber();
            ObjexxFCL::gio::open(Unit11, DataStringGlobals::eplusADSFileName);
            for (i = 1; i <= NetworkNumOfNodes; ++i) {
                ObjexxFCL::gio::write(Unit11, Format_901) << i << state.afn->AirflowNetworkNodeData(i).NodeTypeNum <<
        state.afn->AirflowNetworkNodeData(i).NodeHeight << TZ(i)
                                               << PZ(i);
            }
            ObjexxFCL::gio::write(Unit11, Format_900) << 0;
            for (i = 1; i <= AirflowNetworkNumOfComps; ++i) {
                j = state.afn->AirflowNetworkCompData(i).TypeNum;
                {
                    auto const SELECT_CASE_var(state.afn->AirflowNetworkCompData(i).CompTypeNum);
                    if (SELECT_CASE_var == CompTypeNum_PLR) { //'PLR'  Power law component
                        //              WRITE(Unit11,902)
        state.afn->AirflowNetworkCompData(i)%CompNum,1,DisSysCompLeakData(j)%FlowCoef, &
                        //                  DisSysCompLeakData(j)%FlowCoef,DisSysCompLeakData(j)%FlowCoef,DisSysCompLeakData(j)%FlowExpo
                    } else if (SELECT_CASE_var == CompTypeNum_SCR) { //'SCR'  Surface crack component
                        ObjexxFCL::gio::write(Unit11, Format_902) << state.afn->AirflowNetworkCompData(i).CompNum << 1 <<
        MultizoneSurfaceCrackData(j).FlowCoef
                                                       << MultizoneSurfaceCrackData(j).FlowCoef << MultizoneSurfaceCrackData(j).FlowCoef
                                                       << MultizoneSurfaceCrackData(j).FlowExpo;
                    } else if (SELECT_CASE_var == CompTypeNum_DWC) { //'DWC' Duct component
                        //              WRITE(Unit11,902)
        state.afn->AirflowNetworkCompData(i)%CompNum,2,DisSysCompDuctData(j)%L,DisSysCompDuctData(j)%D, &
                        //                               DisSysCompDuctData(j)%A,DisSysCompDuctData(j)%Rough
                        //              WRITE(Unit11,903) DisSysCompDuctData(i)%TurDynCoef,DisSysCompDuctData(j)%LamFriCoef, &
                        //                               DisSysCompDuctData(j)%LamFriCoef,DisSysCompDuctData(j)%InitLamCoef
                        //           CASE (CompTypeNum_CVF) ! 'CVF' Constant volume fan component
                        //              WRITE(Unit11,904) state.afn->AirflowNetworkCompData(i)%CompNum,4,DisSysCompCVFData(j)%FlowRate
                    } else if (SELECT_CASE_var == CompTypeNum_EXF) { // 'EXF' Zone exhaust fan
                        ObjexxFCL::gio::write(Unit11, Format_904) << state.afn->AirflowNetworkCompData(i).CompNum << 4 <<
        MultizoneCompExhaustFanData(j).FlowRate; } else {
                    }
                }
            }
            ObjexxFCL::gio::write(Unit11, Format_900) << 0;
            for (i = 1; i <= NetworkNumOfLinks; ++i) {
                gio::write(Unit11, Format_910) << i << state.afn->AirflowNetworkLinkageData(i).NodeNums[0] <<
        state.afn->AirflowNetworkLinkageData(i).NodeHeights[0]
                                               << state.afn->AirflowNetworkLinkageData(i).NodeNums[1] <<
        state.afn->AirflowNetworkLinkageData(i).NodeHeights[1]
                                               << state.afn->AirflowNetworkLinkageData(i).CompNum << 0 << 0;
            }
            ObjexxFCL::gio::write(Unit11, Format_900) << 0;
        }
        */
        setsky();

        // SETSKY figures out the IK stuff -- which is why E+ doesn't allocate AU until here
#ifdef SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS
        //   ! only printing to screen, can be commented
        //   print*, "SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS is defined"
        //   write(*,'(2(a,i8))') "AllocateAirflowNetworkData: after SETSKY, allocating AU.  NetworkNumOfNodes=", &
        //        NetworkNumOfNodes, " IK(NetworkNumOfNodes+1)= NNZE=", IK(NetworkNumOfNodes+1)
        //   print*, " NetworkNumOfLinks=", NetworkNumOfLinks
        // allocate same size as others -- this will be maximum  !noel
        newAU.allocate(IK(NetworkNumOfNodes + 1));
#endif

        // noel, GNU says the AU is indexed above its upper bound
        // ALLOCATE(AU(IK(NetworkNumOfNodes+1)-1))
        AU.allocate(IK(NetworkNumOfNodes + 1));
    }

    void Solver::initialize_calculation()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Aug. 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes variables for AirflowNetworkSolver.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        for (int i = 1; i <= ActualNumOfNodes; ++i) {
            ID(i) = i;
        }
        for (int i = 1; i <= ActualNumOfLinks; ++i) {
            AFECTL(i) = 1.0;
            AFLOW(i) = 0.0;
            AFLOW2(i) = 0.0;
        }

        for (int i = 1; i <= ActualNumOfNodes; ++i) {
            // TZ(i) = AirflowNetworkNodeSimu(i).TZ;
            // WZ(i) = AirflowNetworkNodeSimu(i).WZ;
            PZ(i) = AirflowNetworkNodeSimu(i).PZ;
            node_states[i].temperature = AirflowNetworkNodeSimu(i).TZ;
            node_states[i].humidity_ratio = AirflowNetworkNodeSimu(i).WZ;
            // properties[i].pressure = AirflowNetworkNodeSimu(i).PZ;
        }
    }

    void Solver::setsky()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   1998
        //       MODIFIED       Feb. 2006 (L. Gu) to meet requirements of AirflowNetwork
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine sets up the "IK" array describing the sparse matrix [A] in skyline
        //     form by using the location matrix.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // AIRNET

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // IK(K) - pointer to the top of column/row "K".
        int i;
        int j;
        int k;
        int L;
        int M;
        int N1;
        int N2;

        // Initialize "IK".
        for (i = 1; i <= ActualNumOfNodes + 1; ++i) {
            IK(i) = 0;
        }
        // Determine column heights.
        for (M = 1; M <= ActualNumOfLinks; ++M) {
            j = AirflowNetworkLinkageData(M).NodeNums[1];
            if (j == 0) {
                continue;
            }
            L = ID(j);
            i = AirflowNetworkLinkageData(M).NodeNums[0];
            k = ID(i);
            N1 = std::abs(L - k);
            N2 = max(k, L);
            IK(N2) = max(IK(N2), N1);
        }
        // Convert heights to column addresses.
        j = IK(1);
        IK(1) = 1;
        for (k = 1; k <= ActualNumOfNodes; ++k) {
            i = IK(k + 1);
            IK(k + 1) = IK(k) + j;
            j = i;
        }
    }

    void Solver::airmov()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNETf
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is a driver for AIRNET to calculate nodal pressures and linkage airflows

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int m;
        int n;
        int ITER;

        // Formats

        // static ObjexxFCL::gio::Fmt Format_900("(,/,11X,'i    n    m       DP',12x,'F1',12X,'F2')");
        // static ObjexxFCL::gio::Fmt Format_901("(1X,A6,3I5,3F14.6)");
        // static ObjexxFCL::gio::Fmt Format_902("(,/,11X,'n       P',12x,'sumF')");
        // static ObjexxFCL::gio::Fmt Format_903("(1X,A6,I5,3F14.6)");
        // static ObjexxFCL::gio::Fmt Format_907("(,/,' CPU seconds for ',A,F12.3)");

        auto &NetworkNumOfLinks = ActualNumOfLinks;
        auto &NetworkNumOfNodes = ActualNumOfNodes;

        // Initialize pressure for pressure control and for Initialization Type = LinearInitializationMethod
        if ((simulation_control.InitFlag == 0) || (PressureSetFlag > 0 && AirflowNetworkFanActivated)) {
            for (n = 1; n <= NetworkNumOfNodes; ++n) {
                if (AirflowNetworkNodeData(n).NodeTypeNum == 0) {
                    PZ(n) = 0.0;
                }
            }
        }
        // Compute zone air properties.
        for (n = 1; n <= NetworkNumOfNodes; ++n) {
            node_states[n].density =
                properties.density(m_state.dataEnvrn->StdBaroPress + PZ(n), node_states[n].temperature, node_states[n].humidity_ratio);
            // RHOZ(n) = PsyRhoAirFnPbTdbW(StdBaroPress + PZ(n), TZ(n), WZ(n));
            if (AirflowNetworkNodeData(n).ExtNodeNum > 0) {
                node_states[n].density =
                    properties.density(m_state.dataEnvrn->StdBaroPress + PZ(n), m_state.dataEnvrn->OutDryBulbTemp, m_state.dataEnvrn->OutHumRat);
                node_states[n].temperature = m_state.dataEnvrn->OutDryBulbTemp;
                node_states[n].humidity_ratio = m_state.dataEnvrn->OutHumRat;
            }
            node_states[n].sqrt_density = std::sqrt(node_states[n].density);
            node_states[n].viscosity = 1.71432e-5 + 4.828e-8 * node_states[n].temperature;
            // if (LIST >= 2) ObjexxFCL::gio::write(outputFile, Format_903) << "D,V:" << n << properties[n].density << properties[n].viscosity;
        }
        // Compute stack pressures.
        for (i = 1; i <= NetworkNumOfLinks; ++i) {
            n = AirflowNetworkLinkageData(i).NodeNums[0];
            m = AirflowNetworkLinkageData(i).NodeNums[1];
            if (AFLOW(i) > 0.0) {
                PS(i) = 9.80 * (node_states[n].density * (AirflowNetworkNodeData(n).NodeHeight - AirflowNetworkNodeData(m).NodeHeight) +
                                m_state.afn->AirflowNetworkLinkageData(i).NodeHeights[1] * (node_states[m].density - node_states[n].density));
            } else if (AFLOW(i) < 0.0) {
                PS(i) = 9.80 * (node_states[m].density * (AirflowNetworkNodeData(n).NodeHeight - AirflowNetworkNodeData(m).NodeHeight) +
                                AirflowNetworkLinkageData(i).NodeHeights[0] * (node_states[m].density - node_states[n].density));
            } else {
                PS(i) = 4.90 * ((node_states[n].density + node_states[m].density) *
                                    (AirflowNetworkNodeData(n).NodeHeight - AirflowNetworkNodeData(m).NodeHeight) +
                                (AirflowNetworkLinkageData(i).NodeHeights[0] + AirflowNetworkLinkageData(i).NodeHeights[1]) *
                                    (node_states[m].density - node_states[n].density));
            }
        }

        // Calculate pressure field in a large opening
        dos.pstack(m_state, node_states, PZ);
        solvzp(ITER);

        // Report element flows and zone pressures.
        for (n = 1; n <= NetworkNumOfNodes; ++n) {
            SUMAF(n) = 0.0;
        }
        // if (LIST >= 1) ObjexxFCL::gio::write(outputFile, Format_900);
        for (i = 1; i <= NetworkNumOfLinks; ++i) {
            n = AirflowNetworkLinkageData(i).NodeNums[0];
            m = AirflowNetworkLinkageData(i).NodeNums[1];
            // if (LIST >= 1) {
            //    gio::write(outputFile, Format_901) << "Flow: " << i << n << m << AirflowNetworkLinkSimu(i).DP << AFLOW(i) << AFLOW2(i);
            //}
            if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::HOP) {
                SUMAF(n) = SUMAF(n) - AFLOW(i);
                SUMAF(m) += AFLOW(i);
            } else {
                SUMAF(n) = SUMAF(n) - AFLOW(i) - AFLOW2(i);
                SUMAF(m) += AFLOW(i) + AFLOW2(i);
            }
        }
        // for (n = 1; n <= NetworkNumOfNodes; ++n) {
        //    if (LIST >= 1) gio::write(outputFile, Format_903) << "Room: " << n << PZ(n) << SUMAF(n) << properties[n].temperature;
        //}

        for (i = 1; i <= NetworkNumOfLinks; ++i) {
            if (AFLOW2(i) != 0.0) {
            }
            if (AFLOW(i) > 0.0) {
                AirflowNetworkLinkSimu(i).FLOW = AFLOW(i);
                AirflowNetworkLinkSimu(i).FLOW2 = 0.0;
            } else {
                AirflowNetworkLinkSimu(i).FLOW = 0.0;
                AirflowNetworkLinkSimu(i).FLOW2 = -AFLOW(i);
            }
            if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::HOP) {
                if (AFLOW(i) > 0.0) {
                    AirflowNetworkLinkSimu(i).FLOW = AFLOW(i) + AFLOW2(i);
                    AirflowNetworkLinkSimu(i).FLOW2 = AFLOW2(i);
                } else {
                    AirflowNetworkLinkSimu(i).FLOW = AFLOW2(i);
                    AirflowNetworkLinkSimu(i).FLOW2 = -AFLOW(i) + AFLOW2(i);
                }
            }
            if (AirflowNetworkLinkageData(i).DetOpenNum > 0) {
                if (AFLOW2(i) != 0.0) {
                    AirflowNetworkLinkSimu(i).FLOW = AFLOW(i) + AFLOW2(i);
                    AirflowNetworkLinkSimu(i).FLOW2 = AFLOW2(i);
                }
            }
            if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum == iComponentTypeNum::SOP && AFLOW2(i) != 0.0) {
                if (AFLOW(i) >= 0.0) {
                    AirflowNetworkLinkSimu(i).FLOW = AFLOW(i);
                    AirflowNetworkLinkSimu(i).FLOW2 = std::abs(AFLOW2(i));
                } else {
                    AirflowNetworkLinkSimu(i).FLOW = std::abs(AFLOW2(i));
                    AirflowNetworkLinkSimu(i).FLOW2 = -AFLOW(i);
                }
            }
        }

        for (i = 1; i <= NetworkNumOfNodes; ++i) {
            AirflowNetworkNodeSimu(i).PZ = PZ(i);
        }
    }

    void Solver::solvzp(int &ITER) // number of iterations
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves zone pressures by modified Newton-Raphson iteration

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        auto &NetworkNumOfNodes = ActualNumOfNodes;
        // auto &NetworkNumOfLinks = ActualNumOfLinks;

        // Argument array dimensioning (these used to be arguments, need to also test newAU and newIK)
        EP_SIZE_CHECK(IK, NetworkNumOfNodes + 1);
        EP_SIZE_CHECK(AD, NetworkNumOfNodes);
        EP_SIZE_CHECK(AU, IK(NetworkNumOfNodes + 1));

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // noel GNU says AU is being indexed beyound bounds
        // REAL(r64), INTENT(INOUT) :: AU(IK(NetworkNumOfNodes+1)-1) ! the upper triangle of [A] before and after factoring

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        //     NNZE   - number of nonzero entries in the "AU" array.
        //     LFLAG   - if = 1, use laminar relationship (initialization).
        //     I       - element number.
        //     N       - number of node/zone 1.
        //     M       - number of node/zone 2.
        //     F       - flows through the element (kg/s).
        //     DF      - partial derivatives:  DF/DP.
        //     NF      - number of flows, 1 or 2.
        //     SUMF    - sum of flows into node/zone.
        //     CCF     - current pressure correction (Pa).
        //     PCF     - previous pressure correction (Pa).
        //     CEF     - convergence enhancement factor.
        int n;
        int NNZE;
        int NSYM;
        bool LFLAG;
        int CONVG;
        int ACCEL;
        Array1D<Real64> PCF(NetworkNumOfNodes);
        Array1D<Real64> CEF(NetworkNumOfNodes);
        Real64 C;
        Real64 SSUMF;
        Real64 SSUMAF;
        Real64 ACC0;
        Real64 ACC1;
        Array1D<Real64> CCF(NetworkNumOfNodes);

        // auto &outputFile = std::cout;

        ACC1 = 0.0;
        ACCEL = 0;
        NSYM = 0;
        NNZE = IK(NetworkNumOfNodes + 1) - 1;
        // if (LIST >= 2) print(outputFile, "Initialization{:16}{:16}{:16}\n", NetworkNumOfNodes, NetworkNumOfLinks, NNZE);
        ITER = 0;

        for (n = 1; n <= NetworkNumOfNodes; ++n) {
            PCF(n) = 0.0;
            CEF(n) = 0.0;
        }

        if (simulation_control.InitFlag != 1) {
            // Initialize node/zone pressure values by assuming only linear relationship between
            // airflows and pressure drops.
            LFLAG = true;
            filjac(NNZE, LFLAG);
            for (n = 1; n <= NetworkNumOfNodes; ++n) {
                if (AirflowNetworkNodeData(n).NodeTypeNum == 0) {
                    PZ(n) = SUMF(n);
                }
            }
            // Data dump.
//            if (LIST >= 3) {
//                DUMPVD("AD:", AD, NetworkNumOfNodes, outputFile);
//                DUMPVD("AU:", AU, NNZE, outputFile);
//                DUMPVR("AF:", SUMF, NetworkNumOfNodes, outputFile);
//            }
// Solve linear system for approximate PZ.
#ifdef SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS
            facsky(newAU, AD, newAU, newIK, NetworkNumOfNodes, NSYM);     // noel
            slvsky(newAU, AD, newAU, PZ, newIK, NetworkNumOfNodes, NSYM); // noel
#else
            facsky(AU, AD, AU, IK, NetworkNumOfNodes, NSYM);
            slvsky(AU, AD, AU, PZ, IK, NetworkNumOfNodes, NSYM);
#endif
            // if (LIST >= 2) DUMPVD("PZ:", PZ, NetworkNumOfNodes, outputFile);
        }
        // Solve nonlinear airflow network equations by modified Newton's method.
        while (ITER < simulation_control.maximum_iterations) {
            LFLAG = false;
            ++ITER;
            //            if (LIST >= 2) {
            //                print(outputFile, "Begin iteration {}\n", ITER);
            //            }
            // Set up the Jacobian matrix.
            filjac(NNZE, LFLAG);
            // Data dump.
            //            if (LIST >= 3) {
            //                DUMPVR("SUMF:", SUMF, NetworkNumOfNodes, outputFile);
            //                DUMPVR("SUMAF:", SUMAF, NetworkNumOfNodes, outputFile);
            //            }
            // Check convergence.
            CONVG = 1;
            SSUMF = 0.0;
            SSUMAF = 0.0;
            for (n = 1; n <= NetworkNumOfNodes; ++n) {
                SSUMF += std::abs(SUMF(n));
                SSUMAF += SUMAF(n);
                if (CONVG == 1) {
                    if (std::abs(SUMF(n)) <= simulation_control.absolute_convergence_tolerance) {
                        continue;
                    }
                    if (std::abs(SUMF(n) / SUMAF(n)) > simulation_control.relative_convergence_tolerance) {
                        CONVG = 0;
                    }
                }
            }
            ACC0 = ACC1;
            if (SSUMAF > 0.0) {
                ACC1 = SSUMF / SSUMAF;
            }
            if (CONVG == 1 && ITER > 1) {
                return;
            }
            if (ITER >= simulation_control.maximum_iterations) {
                break;
            }
            // Data dump.
            //            if (LIST >= 3) {
            //                DUMPVD("AD:", AD, NetworkNumOfNodes, outputFile);
            //                DUMPVD("AU:", AU, NNZE, outputFile);
            //            }
            // Solve AA * CCF = SUMF.
            for (n = 1; n <= NetworkNumOfNodes; ++n) {
                CCF(n) = SUMF(n);
            }
#ifdef SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS
            facsky(newAU, AD, newAU, newIK, NetworkNumOfNodes, NSYM);      // noel
            slvsky(newAU, AD, newAU, CCF, newIK, NetworkNumOfNodes, NSYM); // noel
#else
            facsky(AU, AD, AU, IK, NetworkNumOfNodes, NSYM);
            slvsky(AU, AD, AU, CCF, IK, NetworkNumOfNodes, NSYM);
#endif
            // Revise PZ (Steffensen iteration on the N-R correction factors to handle oscillating corrections).
            if (ACCEL == 1) {
                ACCEL = 0;
            } else {
                if (ITER > 2 && ACC1 > 0.5 * ACC0) {
                    ACCEL = 1;
                }
            }
            for (n = 1; n <= NetworkNumOfNodes; ++n) {
                if (AirflowNetworkNodeData(n).NodeTypeNum == 1) {
                    continue;
                }
                CEF(n) = 1.0;
                if (ACCEL == 1) {
                    C = CCF(n) / PCF(n);
                    if (C < simulation_control.convergence_acceleration_limit) {
                        CEF(n) = 1.0 / (1.0 - C);
                    }
                    C = CCF(n) * CEF(n);
                } else {
                    //            IF (CCF(N) .EQ. 0.0d0) CCF(N)=TINY(CCF(N))  ! 1.0E-40
                    if (CCF(n) == 0.0) {
                        CCF(n) = Constant::rTinyValue; // 1.0E-40 (Epsilon)
                    }
                    PCF(n) = CCF(n);
                    C = CCF(n);
                }
                if (std::abs(C) > simulation_control.MaxPressure) {
                    CEF(n) *= simulation_control.MaxPressure / std::abs(C);
                    PZ(n) -= CCF(n) * CEF(n);
                } else {
                    PZ(n) -= C;
                }
            }
            // Data revision dump.
            //            if (LIST >= 2) {
            //                for (n = 1; n <= NetworkNumOfNodes; ++n) {
            //                    if (state.afn->AirflowNetworkNodeData(n).NodeTypeNum == 0) {
            //                        print(outputFile, "Rev: {:5}{:3}{:14.6E} {:8.4F}{:24.14F}", n, SUMF(n), CCF(n), CEF(n), PZ(n));
            //                    }
            //                }
            //            }
        }

        // Error termination.
        ShowSevereError(m_state, "Too many iterations (SOLVZP) in Airflow Network simulation");
        ++ExtLargeOpeningErrCount;
        if (ExtLargeOpeningErrCount < 2) {
            ShowWarningError(m_state,
                             "AirflowNetwork: SOLVER, Changing values for initialization flag, Relative airflow convergence, Absolute airflow "
                             "convergence, Convergence acceleration limit or Maximum Iteration Number may solve the problem.");
            ShowContinueErrorTimeStamp(m_state, "");
            ShowContinueError(m_state,
                              "..Iterations=" + std::to_string(ITER) + ", Max allowed=" + std::to_string(simulation_control.maximum_iterations));
            ShowFatalError(m_state, "AirflowNetwork: SOLVER, The previous error causes termination.");
        } else {
            ShowRecurringWarningErrorAtEnd(
                m_state, "AirFlowNetwork: Too many iterations (SOLVZP) in AirflowNetwork simulation continues.", ExtLargeOpeningErrIndex);
        }
    }

    void Solver::filjac(int const NNZE,  // number of nonzero entries in the "AU" array.
                        bool const LFLAG // if = 1, use laminar relationship (initialization).
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine creates matrices for solution of flows

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        // I       - component number.
        // N       - number of node/zone 1.
        // M       - number of node/zone 2.
        // F       - flows through the element (kg/s).
        // DF      - partial derivatives:  DF/DP.
        // NF      - number of flows, 1 or 2.
        int i;
        int j;
        int n;
        int FLAG;
        int NF;
#ifdef SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS
        int LHK; // noel
        int JHK;
        int JHK1;
        int newsum;
        int newh;
        int ispan;
        int thisIK;
        bool allZero; // noel
#endif
        Array1D<Real64> X(4);
        Real64 DP;
        std::array<Real64, 2> F{{0.0, 0.0}};
        std::array<Real64, 2> DF{{0.0, 0.0}};

        auto &NetworkNumOfLinks = ActualNumOfLinks;
        auto &NetworkNumOfNodes = ActualNumOfNodes;

        for (n = 1; n <= NetworkNumOfNodes; ++n) {
            SUMF(n) = 0.0;
            SUMAF(n) = 0.0;
            if (AirflowNetworkNodeData(n).NodeTypeNum == 1) {
                AD(n) = 1.0;
            } else {
                AD(n) = 0.0;
            }
        }
        for (n = 1; n <= NNZE; ++n) {
            AU(n) = 0.0;
        }
        // Loop(s) to calculate control, etc.
        // Set up the Jacobian matrix.
        for (i = 1; i <= NetworkNumOfLinks; ++i) {
            if (AirflowNetworkLinkageData(i).element == nullptr) {
                continue;
            }
            n = AirflowNetworkLinkageData(i).NodeNums[0];
            int m = AirflowNetworkLinkageData(i).NodeNums[1];
            //!!! Check array of DP. DpL is used for multizone air flow calculation only
            //!!! and is not for forced air calculation
            if (i > NumOfLinksMultiZone) {
                DP = PZ(n) - PZ(m) + PS(i) + PW(i);
            } else {
                DP = PZ(n) - PZ(m) + dos.DpL(i, 1) + PW(i);
            }
            Real64 multiplier = 1.0;
            Real64 control = AirflowNetworkLinkageData(i).control;
            // if (LIST >= 4) ObjexxFCL::gio::write(outputFile, Format_901) << "PS:" << i << n << M << PS(i) << PW(i) << AirflowNetworkLinkSimu(i).DP;
            j = AirflowNetworkLinkageData(i).CompNum;

            NF = AirflowNetworkLinkageData(i).element->calculate(m_state, LFLAG, DP, i, multiplier, control, node_states[n], node_states[m], F, DF);
            if (AirflowNetworkLinkageData(i).element->type() == ComponentType::CPD && DP != 0.0) {
                DP = DisSysCompCPDData(AirflowNetworkCompData(j).TypeNum).DP;
            }

            AirflowNetworkLinkSimu(i).DP = DP;
            AFLOW(i) = F[0];
            AFLOW2(i) = 0.0;
            if (AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::DOP) {
                AFLOW2(i) = F[1];
            }
            // if (LIST >= 3) ObjexxFCL::gio::write(outputFile, Format_901) << " NRi:" << i << n << M << AirflowNetworkLinkSimu(i).DP << F[0] <<
            // DF[0];
            FLAG = 1;
            if (AirflowNetworkNodeData(n).NodeTypeNum == 0) {
                ++FLAG;
                X(1) = DF[0];
                X(2) = -DF[0];
                SUMF(n) += F[0];
                SUMAF(n) += std::abs(F[0]);
            }
            if (AirflowNetworkNodeData(m).NodeTypeNum == 0) {
                FLAG += 2;
                X(4) = DF[0];
                X(3) = -DF[0];
                SUMF(m) -= F[0];
                SUMAF(m) += std::abs(F[0]);
            }
            if (FLAG != 1) {
                filsky(X, AirflowNetworkLinkageData(i).NodeNums, IK, AU, AD, FLAG);
            }
            if (NF == 1) {
                continue;
            }
            AFLOW2(i) = F[1];
            // if (LIST >= 3) ObjexxFCL::gio::write(outputFile, Format_901) << " NRj:" << i << n << m << AirflowNetworkLinkSimu(i).DP << F[1] <<
            // DF[1];
            FLAG = 1;
            if (AirflowNetworkNodeData(n).NodeTypeNum == 0) {
                ++FLAG;
                X(1) = DF[1];
                X(2) = -DF[1];
                SUMF(n) += F[1];
                SUMAF(n) += std::abs(F[1]);
            }
            if (AirflowNetworkNodeData(m).NodeTypeNum == 0) {
                FLAG += 2;
                X(4) = DF[1];
                X(3) = -DF[1];
                SUMF(m) -= F[1];
                SUMAF(m) += std::abs(F[1]);
            }
            if (FLAG != 1) {
                filsky(X, AirflowNetworkLinkageData(i).NodeNums, IK, AU, AD, FLAG);
            }
        }

#ifdef SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS

        // After the matrix values have been set, we can look at them and see if any columns are filled with zeros.
        // If they are, let's remove them from the matrix -- but only for the purposes of doing the solve.
        // They way I do this is building a separate IK array (newIK) that simply changes the column heights.
        // So the affected SOLVEs would use this newIK and nothing else changes.
        for (n = 1; n <= NetworkNumOfNodes + 1; ++n) {
            newIK(n) = IK(n);
            // print*, " NetworkNumOfNodes  n=", n, " IK(n)=", IK(n)
        }

        newsum = IK(2) - IK(1); // always 0?

        JHK = 1;
        for (n = 2; n <= NetworkNumOfNodes; ++n) {
            JHK1 = IK(n + 1); // starts at IK(3)-IK(2)
            LHK = JHK1 - JHK;
            if (LHK <= 0) {
                newIK(n + 1) = newIK(n);
                continue;
            }
            // write(*,'(4(a,i8))') "n=", n, " ik=", ik(n), " JHK=", JHK, " LHK=", LHK

            // is the entire column zero?  noel
            allZero = true;
            for (i = 0; i <= LHK - 1; ++i) {
                if (AU(JHK + i) != 0.0) {
                    allZero = false;
                    break;
                }
            }

            newh = LHK;
            if (allZero) {
                // print*, "allzero n=", n
                newh = 0;
            } else {
                // DO i=0,LHK-1
                //   write(*, '(2(a,i8),a, f15.3)') "  n=", n, " i=", i, " AU(JHK+i)=", AU(JHK+i)
                // enddo
            }
            newIK(n + 1) = newIK(n) + newh;
            newsum += newh;

            // do i = LHK-1,0, -1
            //   write(*, '(2(a,i8),a, f15.3)') "  n=", n, " i=", i, " AU(JHK+i)=", AU(JHK+i)
            // enddo
            JHK = JHK1;
        }

        // this is just a print to screen, is not necessary
        //     if (firstTime) then
        //        write(*, '(2(a,i8))') " After SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS: newsum=", newsum, " oldsum=", IK(NetworkNumOfNodes+1)
        //        firstTime=.FALSE.
        //     endif

        // Now fill newAU from AU, using newIK
        for (n = 2; n <= NetworkNumOfNodes; ++n) {
            thisIK = newIK(n);
            ispan = newIK(n + 1) - thisIK;

            if (ispan <= 0) {
                continue;
            }
            for (i = 0; i <= ispan - 1; ++i) {
                newAU(thisIK + i) = AU(IK(n) + i);
            }
        }
#endif
    }

    void Solver::facsky(Array1D<Real64> &AU,   // the upper triangle of [A] before and after factoring
                        Array1D<Real64> &AD,   // the main diagonal of [A] before and after factoring
                        Array1D<Real64> &AL,   // the lower triangle of [A] before and after factoring
                        const Array1D_int &IK, // pointer to the top of column/row "K"
                        int const NEQ,         // number of equations
                        int const NSYM         // symmetry:  0 = symmetric matrix, 1 = non-symmetric
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  This subroutine is revised from FACSKY developed by George Walton, NIST

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs L-U factorization of a skyline ordered matrix, [A]
        // The algorithm has been restructured for clarity.
        // Note dependence on compiler for optimizing the inner do loops.

        // METHODOLOGY EMPLOYED:
        //     L-U factorization of a skyline ordered matrix, [A], used for
        //     solution of simultaneous linear algebraic equations [A] * X = B.
        //     No pivoting!  No scaling!  No warnings!!!
        //     Related routines:  SLVSKY, SETSKY, FILSKY.

        // REFERENCES:
        //     Algorithm is described in "The Finite Element Method Displayed",
        //     by G. Dhatt and G. Touzot, John Wiley & Sons, New York, 1984.

        // USE STATEMENTS:
        // na

        // Argument array dimensioning
        EP_SIZE_CHECK(IK, ActualNumOfNodes + 1);
        EP_SIZE_CHECK(AU, IK(ActualNumOfNodes + 1));
        EP_SIZE_CHECK(AD, ActualNumOfNodes);
        EP_SIZE_CHECK(AL, IK(ActualNumOfNodes + 1) - 1);

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // noel, GNU says the AU is indexed above its upper bound
        // REAL(r64), INTENT(INOUT) :: AU(IK(NetworkNumOfNodes+1)-1) ! the upper triangle of [A] before and after factoring

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int JHK;
        int LHK1;
        int IMIN;
        int IMIN1;
        int JHJ;
        int JHJ1;
        int IC;
        int i;
        int j;
        int k;
        Real64 T1;
        Real64 T2;
        Real64 SDOT;
        Real64 SUMD;

        AD(1) = 1.0 / AD(1);
        JHK = 1;
        for (k = 2; k <= NEQ; ++k) {
            SUMD = 0.0;
            int JHK1 = IK(k + 1);
            int LHK = JHK1 - JHK;
            if (LHK > 0) {
                LHK1 = LHK - 1;
                IMIN = k - LHK1;
                IMIN1 = IMIN - 1;
                if (NSYM == 1) {
                    AL(JHK) *= AD(IMIN1);
                }
                if (LHK1 != 0) {
                    JHJ = IK(IMIN);
                    if (NSYM == 0) {
                        for (j = 1; j <= LHK1; ++j) {
                            JHJ1 = IK(IMIN + j);
                            IC = min(j, JHJ1 - JHJ);
                            if (IC > 0) {
                                SDOT = 0.0;
                                for (i = 0; i <= IC - 1; ++i) {
                                    SDOT += AU(JHJ1 - IC + i) * AU(JHK + j - IC + i);
                                }
                                AU(JHK + j) -= SDOT;
                            }
                            JHJ = JHJ1;
                        }
                    } else {
                        for (j = 1; j <= LHK1; ++j) {
                            JHJ1 = IK(IMIN + j);
                            IC = min(j, JHJ1 - JHJ);
                            SDOT = 0.0;
                            if (IC > 0) {
                                for (i = 0; i <= IC - 1; ++i) {
                                    SDOT += AL(JHJ1 - IC + i) * AU(JHK + j - IC + i);
                                }
                                AU(JHK + j) -= SDOT;
                                SDOT = 0.0;
                                for (i = 0; i <= IC - 1; ++i) {
                                    SDOT += AU(JHJ1 - IC + i) * AL(JHK + j - IC + i);
                                }
                            }
                            AL(JHK + j) = (AL(JHK + j) - SDOT) * AD(IMIN1 + j);
                            JHJ = JHJ1;
                        }
                    }
                }
                if (NSYM == 0) {
                    for (i = 0; i <= LHK1; ++i) {
                        T1 = AU(JHK + i);
                        T2 = T1 * AD(IMIN1 + i);
                        AU(JHK + i) = T2;
                        SUMD += T1 * T2;
                    }
                } else {
                    for (i = 0; i <= LHK1; ++i) {
                        SUMD += AU(JHK + i) * AL(JHK + i);
                    }
                }
            }
            if (AD(k) - SUMD == 0.0) {
                ShowSevereError(m_state, "AirflowNetworkSolver: L-U factorization in Subroutine FACSKY.");
                ShowContinueError(
                    m_state,
                    "The denominator used in L-U factorizationis equal to 0.0 at node = " + m_state.afn->AirflowNetworkNodeData(k).Name + '.');
                ShowContinueError(
                    m_state, "One possible cause is that this node may not be connected directly, or indirectly via airflow network connections ");
                ShowContinueError(
                    m_state, "(e.g., AirflowNetwork:Multizone:SurfaceCrack, AirflowNetwork:Multizone:Component:SimpleOpening, etc.), to an external");
                ShowContinueError(m_state, "node (AirflowNetwork:MultiZone:Surface).");
                ShowContinueError(m_state,
                                  "Please send your input file and weather file to EnergyPlus support/development team for further investigation.");
                ShowFatalError(m_state, "Preceding condition causes termination.");
            }
            AD(k) = 1.0 / (AD(k) - SUMD);
            JHK = JHK1;
        }
    }

    void Solver::slvsky(const Array1D<Real64> &AU, // the upper triangle of [A] before and after factoring
                        const Array1D<Real64> &AD, // the main diagonal of [A] before and after factoring
                        const Array1D<Real64> &AL, // the lower triangle of [A] before and after factoring
                        Array1D<Real64> &B,        // "B" vector (input); "X" vector (output).
                        const Array1D_int &IK,     // pointer to the top of column/row "K"
                        int const NEQ,             // number of equations
                        int const NSYM             // symmetry:  0 = symmetric matrix, 1 = non-symmetric
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  This subroutine is revised from CLVSKY developed by George Walton, NIST

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves simultaneous linear algebraic equations [A] * X = B
        // using L-U factored skyline form of [A] from "FACSKY"

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Argument array dimensioning
        EP_SIZE_CHECK(IK, ActualNumOfNodes + 1);
        EP_SIZE_CHECK(AU, IK(ActualNumOfNodes + 1));
        EP_SIZE_CHECK(AD, ActualNumOfNodes);
        EP_SIZE_CHECK(AL, IK(ActualNumOfNodes + 1) - 1);
        EP_SIZE_CHECK(B, ActualNumOfNodes);

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // noel, GNU says the AU is indexed above its upper bound
        // REAL(r64), INTENT(INOUT) :: AU(IK(NetworkNumOfNodes+1)-1) ! the upper triangle of [A] before and after factoring

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        int i;
        int JHK;
        int JHK1;
        int k;
        Real64 SDOT;
        Real64 T1;

        JHK = 1;
        for (k = 2; k <= NEQ; ++k) {
            JHK1 = IK(k + 1);
            int LHK = JHK1 - JHK;
            if (LHK <= 0) {
                continue;
            }
            SDOT = 0.0;
            if (NSYM == 0) {
                for (i = 0; i <= LHK - 1; ++i) {
                    SDOT += AU(JHK + i) * B(k - LHK + i);
                }
            } else {
                for (i = 0; i <= LHK - 1; ++i) {
                    SDOT += AL(JHK + i) * B(k - LHK + i);
                }
            }
            B(k) -= SDOT;
            JHK = JHK1;
        }
        if (NSYM == 0) {
            for (k = 1; k <= NEQ; ++k) {
                B(k) *= AD(k);
            }
        }
        k = NEQ + 1;
        JHK1 = IK(k);
        while (k != 1) {
            --k;
            if (NSYM == 1) {
                B(k) *= AD(k);
            }
            if (k == 1) {
                break;
            }
            //        IF(K.EQ.1) RETURN
            JHK = IK(k);
            T1 = B(k);
            for (i = 0; i <= JHK1 - JHK - 1; ++i) {
                B(k - JHK1 + JHK + i) -= AU(JHK + i) * T1;
            }
            JHK1 = JHK;
        }
    }

    void Solver::filsky(const Array1D<Real64> &X,    // element array (row-wise sequence)
                        std::array<int, 2> const LM, // location matrix
                        const Array1D_int &IK,       // pointer to the top of column/row "K"
                        Array1D<Real64> &AU,         // the upper triangle of [A] before and after factoring
                        Array1D<Real64> &AD,         // the main diagonal of [A] before and after factoring
                        int const FLAG               // mode of operation
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  This subroutine is revised from FILSKY developed by George Walton, NIST

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine adds element array "X" to the sparse skyline matrix [A]

        // Argument array dimensioning
        EP_SIZE_CHECK(X, 4);
        EP_SIZE_CHECK(IK, ActualNumOfNodes + 1);
        EP_SIZE_CHECK(AU, IK(ActualNumOfNodes + 1));
        EP_SIZE_CHECK(AD, ActualNumOfNodes);

        // SUBROUTINE ARGUMENT DEFINITIONS:
        // noel, GNU says the AU is indexed above its upper bound
        // REAL(r64), INTENT(INOUT) :: AU(IK(NetworkNumOfNodes+1)-1) ! the upper triangle of [A] before and after factoring

        // K = row number, L = column number.
        if (FLAG > 1) {
            int k = LM[0];
            int L = LM[1];
            if (FLAG == 4) {
                AD(k) += X(1);
                if (k < L) {
                    int j = IK(L + 1) - L + k;
                    AU(j) += X(2);
                } else {
                    int j = IK(k + 1) - k + L;
                    AU(j) += X(3);
                }
                AD(L) += X(4);
            } else if (FLAG == 3) {
                AD(L) += X(4);
            } else if (FLAG == 2) {
                AD(k) += X(1);
            }
        }
    }

} // namespace AirflowNetwork

} // namespace EnergyPlus
