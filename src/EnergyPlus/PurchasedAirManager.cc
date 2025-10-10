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
// #include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/CoolingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/HeatingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/PurchasedAirManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZonePlenum.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::PurchasedAirManager {

// Module containing data and routines dealing with Ideal Loads Air System (formerly PURCHASED AIR).

// MODULE INFORMATION:
//       AUTHOR         Russ Taylor
//       DATE WRITTEN   May 1997
//       MODIFIED       Fred Buhl Dec 1999
//                      B. Griffith Dec 2006. added OA lookup function, moved getinputflag up to Module
//                      M. Witte June 2011, add new features including DCV, economizer, dehumidification and humidification
//                      NOTE: MJW Sep 13, 2011:  Still need to review checks for negative loads and impossible supply temps???
//                           There are no Deallocate statements in here - should there be?
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms required to simulate the
// Zone Ideal Loads Air System component. This component supplies hot or cold air
// at a fixed or variable temperature to a zone to meet the zone load.
// With the June 2011 enhancements it will also supply outdoor air with optional demand-controlled ventilation
// and economizer controls, plus new options for controlling zone humidity.

// METHODOLOGY EMPLOYED:
// The user can choose via input the max/min hot and cold supply air
// temperature and humidity ratio. The air mass flow rate is chosen
// to meet the (remaining) zone load or based on the outdoor air flow requirement.
// If the outdoor air flow sets the flow rate, the supply air temperature and
// humidity ratio are adjusted to meet the zone load.

// Using/Aliasing
using Psychrometrics::PsyCpAirFnW;
using Psychrometrics::PsyHFnTdbW;
using Psychrometrics::PsyRhoAirFnPbTdbW;
using Psychrometrics::PsyTdbFnHW;
using Psychrometrics::PsyTsatFnHPb;
using Psychrometrics::PsyWFnTdbH;
using Psychrometrics::PsyWFnTdbRhPb;

// Delta humidity ratio limit, 0.00025 equals delta between 45F dewpoint and 46F dewpoint
// used to prevent dividing by near zero
Real64 constexpr SmallDeltaHumRat(0.00025);

constexpr std::array<std::string_view, (int)LimitType::Num> limitTypeNames = {
    "NoLimit", "LimitFlowRate", "LimitCapacity", "LimitFlowRateAndCapacity"};
constexpr std::array<std::string_view, (int)LimitType::Num> limitTypeNamesUC = {
    "NOLIMIT", "LIMITFLOWRATE", "LIMITCAPACITY", "LIMITFLOWRATEANDCAPACITY"};

constexpr std::array<std::string_view, (int)HumControl::Num> humControlNames = {
    "None", "ConstantSensibleHeatRatio", "Humidistat", "ConstantSupplyHumidityRatio"};
constexpr std::array<std::string_view, (int)HumControl::Num> humControlNamesUC = {
    "NONE", "CONSTANTSENSIBLEHEATRATIO", "HUMIDISTAT", "CONSTANTSUPPLYHUMIDITYRATIO"};

constexpr std::array<std::string_view, (int)DCV::Num> dcvNames = {"None", "OccupancySchedule", "CO2SetPoint"};
constexpr std::array<std::string_view, (int)DCV::Num> dcvNamesUC = {"NONE", "OCCUPANCYSCHEDULE", "CO2SETPOINT"};

constexpr std::array<std::string_view, (int)Econ::Num> econNames = {"NoEconomizer", "DifferentialDryBulb", "DifferentialEnthalpy"};
constexpr std::array<std::string_view, (int)Econ::Num> econNamesUC = {"NOECONOMIZER", "DIFFERENTIALDRYBULB", "DIFFERENTIALENTHALPY"};

constexpr std::array<std::string_view, (int)HeatRecovery::Num> heatRecoveryNames = {"None", "Sensible", "Enthalpy"};
constexpr std::array<std::string_view, (int)HeatRecovery::Num> heatRecoveryNamesUC = {"NONE", "SENSIBLE", "ENTHALPY"};

void SimPurchasedAir(EnergyPlusData &state,
                     std::string const &PurchAirName,
                     Real64 &SysOutputProvided,
                     Real64 &MoistOutputProvided, // Moisture output provided (kg/s), dehumidification = negative
                     bool const FirstHVACIteration,
                     int const ControlledZoneNum,
                     int &CompIndex)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   May 1997
    //       MODIFIED       Don Shirey, Aug 2009 (LatOutputProvided - now MoistOutputProvided)
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages Purchased Air component simulation.
    // It is called from SimZoneEquipment in the ZoneEquipmentManager
    // at the system time step.

    int PurchAirNum;

    if (state.dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(state);
        state.dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }

    // Find the correct PurchasedAir Equipment
    if (CompIndex == 0) {
        PurchAirNum = Util::FindItemInList(PurchAirName, state.dataPurchasedAirMgr->PurchAir);
        if (PurchAirNum == 0) {
            ShowFatalError(state, format("SimPurchasedAir: Unit not found={}", PurchAirName));
        }
        CompIndex = PurchAirNum;
    } else {
        PurchAirNum = CompIndex;
        if (PurchAirNum > state.dataPurchasedAirMgr->NumPurchAir || PurchAirNum < 1) {
            ShowFatalError(state,
                           format("SimPurchasedAir:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                  PurchAirNum,
                                  state.dataPurchasedAirMgr->NumPurchAir,
                                  PurchAirName));
        }
        if (state.dataPurchasedAirMgr->CheckEquipName(PurchAirNum)) {
            if (PurchAirName != state.dataPurchasedAirMgr->PurchAir(PurchAirNum).Name) {
                ShowFatalError(state,
                               format("SimPurchasedAir: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                      PurchAirNum,
                                      PurchAirName,
                                      state.dataPurchasedAirMgr->PurchAir(PurchAirNum).Name));
            }
            state.dataPurchasedAirMgr->CheckEquipName(PurchAirNum) = false;
        }
    }

    InitPurchasedAir(state, PurchAirNum, ControlledZoneNum);

    CalcPurchAirLoads(state, PurchAirNum, SysOutputProvided, MoistOutputProvided, ControlledZoneNum);

    UpdatePurchasedAir(state, PurchAirNum, FirstHVACIteration);

    ReportPurchasedAir(state, PurchAirNum);
}

void GetPurchasedAir(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   June 1997
    //       MODIFIED       M. Witte, June 2011, add new features including DCV, economizer, dehumidification
    //                                           and humidification controls
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Get the input data for the Purchased Air objects.
    // Set up output variables.

    // Using/Aliasing
    using NodeInputManager::CheckUniqueNodeNames;
    using NodeInputManager::EndUniqueNodeCheck;
    using NodeInputManager::GetOnlySingleNode;
    using NodeInputManager::InitUniqueNodeCheck;
    using OutAirNodeManager::CheckAndAddAirNodeNumber;
    using namespace DataLoopNode;
    using ZonePlenum::GetReturnPlenumIndex;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    static constexpr std::string_view RoutineName("GetPurchasedAir: "); // include trailing blank space
    static constexpr std::string_view routineName = "GetPurchasedAir";

    bool ErrorsFound(false); // If errors detected in input
    auto &s_ipsc = state.dataIPShortCut;

    s_ipsc->cCurrentModuleObject = "ZoneHVAC:IdealLoadsAirSystem";

    auto &PurchAir(state.dataPurchasedAirMgr->PurchAir);

    state.dataPurchasedAirMgr->NumPurchAir = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    PurchAir.allocate(state.dataPurchasedAirMgr->NumPurchAir);
    state.dataPurchasedAirMgr->CheckEquipName.allocate(state.dataPurchasedAirMgr->NumPurchAir);
    state.dataPurchasedAirMgr->PurchAirNumericFields.allocate(state.dataPurchasedAirMgr->NumPurchAir);
    state.dataPurchasedAirMgr->CheckEquipName = true;

    if (state.dataPurchasedAirMgr->NumPurchAir > 0) {
        int NumAlphas;
        int NumNums;
        int IOStat;
        InitUniqueNodeCheck(state, s_ipsc->cCurrentModuleObject);
        for (int PurchAirNum = 1; PurchAirNum <= state.dataPurchasedAirMgr->NumPurchAir; ++PurchAirNum) {
            PurchAir(PurchAirNum).cObjectName = s_ipsc->cCurrentModuleObject;

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     PurchAirNum,
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

            state.dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames.allocate(NumNums);
            state.dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames = "";
            state.dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames = s_ipsc->cNumericFieldNames;

            PurchAir(PurchAirNum).Name = s_ipsc->cAlphaArgs(1);
            // get optional  availability schedule
            if (s_ipsc->lAlphaFieldBlanks(2)) {
                PurchAir(PurchAirNum).availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((PurchAir(PurchAirNum).availSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                ErrorsFound = true;
            }
            // Purchased air supply air node is an outlet node
            PurchAir(PurchAirNum).ZoneSupplyAirNodeNum = GetOnlySingleNode(state,
                                                                           s_ipsc->cAlphaArgs(3),
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::ZoneHVACIdealLoadsAirSystem,
                                                                           s_ipsc->cAlphaArgs(1),
                                                                           DataLoopNode::NodeFluidType::Air,
                                                                           DataLoopNode::ConnectionType::Outlet,
                                                                           NodeInputManager::CompFluidStream::Primary,
                                                                           ObjectIsNotParent);
            bool UniqueNodeError = false;
            CheckUniqueNodeNames(state, s_ipsc->cAlphaFieldNames(3), UniqueNodeError, s_ipsc->cAlphaArgs(3), s_ipsc->cAlphaArgs(1));
            if (UniqueNodeError) {
                ErrorsFound = true;
            }
            // If new (optional) exhaust air node name is present, then register it as inlet
            if (!s_ipsc->lAlphaFieldBlanks(4)) {
                if (s_ipsc->lAlphaFieldBlanks(5)) {
                    PurchAir(PurchAirNum).ZoneExhaustAirNodeNum = GetOnlySingleNode(state,
                                                                                    s_ipsc->cAlphaArgs(4),
                                                                                    ErrorsFound,
                                                                                    DataLoopNode::ConnectionObjectType::ZoneHVACIdealLoadsAirSystem,
                                                                                    s_ipsc->cAlphaArgs(1),
                                                                                    DataLoopNode::NodeFluidType::Air,
                                                                                    DataLoopNode::ConnectionType::Inlet,
                                                                                    NodeInputManager::CompFluidStream::Primary,
                                                                                    ObjectIsNotParent);
                } else {
                    PurchAir(PurchAirNum).ZoneExhaustAirNodeNum = GetOnlySingleNode(state,
                                                                                    s_ipsc->cAlphaArgs(4),
                                                                                    ErrorsFound,
                                                                                    DataLoopNode::ConnectionObjectType::ZoneHVACIdealLoadsAirSystem,
                                                                                    s_ipsc->cAlphaArgs(1),
                                                                                    DataLoopNode::NodeFluidType::Air,
                                                                                    DataLoopNode::ConnectionType::Outlet,
                                                                                    NodeInputManager::CompFluidStream::Primary,
                                                                                    ObjectIsNotParent);
                }
                UniqueNodeError = false;
                CheckUniqueNodeNames(state, s_ipsc->cAlphaFieldNames(4), UniqueNodeError, s_ipsc->cAlphaArgs(4), s_ipsc->cAlphaArgs(1));
                if (UniqueNodeError) {
                    ErrorsFound = true;
                }
            }
            if (!s_ipsc->lAlphaFieldBlanks(5)) {
                PurchAir(PurchAirNum).PlenumExhaustAirNodeNum = GetOnlySingleNode(state,
                                                                                  s_ipsc->cAlphaArgs(5),
                                                                                  ErrorsFound,
                                                                                  DataLoopNode::ConnectionObjectType::ZoneHVACIdealLoadsAirSystem,
                                                                                  s_ipsc->cAlphaArgs(1),
                                                                                  DataLoopNode::NodeFluidType::Air,
                                                                                  DataLoopNode::ConnectionType::Inlet,
                                                                                  NodeInputManager::CompFluidStream::Primary,
                                                                                  ObjectIsNotParent);
            }
            PurchAir(PurchAirNum).MaxHeatSuppAirTemp = s_ipsc->rNumericArgs(1);
            PurchAir(PurchAirNum).MinCoolSuppAirTemp = s_ipsc->rNumericArgs(2);
            PurchAir(PurchAirNum).MaxHeatSuppAirHumRat = s_ipsc->rNumericArgs(3);
            PurchAir(PurchAirNum).MinCoolSuppAirHumRat = s_ipsc->rNumericArgs(4);

            PurchAir(PurchAirNum).HeatingLimit = static_cast<LimitType>(getEnumValue(limitTypeNamesUC, s_ipsc->cAlphaArgs(6)));
            switch (PurchAir(PurchAirNum).HeatingLimit) {

            case LimitType::None: {
            } break;

            case LimitType::FlowRate: {
                if (s_ipsc->lNumericFieldBlanks(5)) {
                    PurchAir(PurchAirNum).HeatingLimit = LimitType::None;
                }
            } break;

            case LimitType::Capacity: {
                if (s_ipsc->lNumericFieldBlanks(6)) {
                    PurchAir(PurchAirNum).HeatingLimit = LimitType::None;
                }
            } break;

            case LimitType::FlowRateAndCapacity: {
                if (s_ipsc->lNumericFieldBlanks(5) && s_ipsc->lNumericFieldBlanks(6)) {
                    PurchAir(PurchAirNum).HeatingLimit = LimitType::None;
                } else if (s_ipsc->lNumericFieldBlanks(5)) {
                    PurchAir(PurchAirNum).HeatingLimit = LimitType::Capacity;
                } else if (s_ipsc->lNumericFieldBlanks(6)) {
                    PurchAir(PurchAirNum).HeatingLimit = LimitType::FlowRate;
                }
            } break;

            default: {
                ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6));
                ErrorsFound = true;
            } break;
            }

            PurchAir(PurchAirNum).MaxHeatVolFlowRate = s_ipsc->rNumericArgs(5);
            PurchAir(PurchAirNum).MaxHeatSensCap = s_ipsc->rNumericArgs(6);

            PurchAir(PurchAirNum).CoolingLimit = static_cast<LimitType>(getEnumValue(limitTypeNamesUC, s_ipsc->cAlphaArgs(7)));

            switch (PurchAir(PurchAirNum).CoolingLimit) {
            case LimitType::None: {
            } break;

            case LimitType::FlowRate: {
                if (s_ipsc->lNumericFieldBlanks(7)) {
                    PurchAir(PurchAirNum).CoolingLimit = LimitType::None;
                }
            } break;

            case LimitType::Capacity: {
                if (s_ipsc->lNumericFieldBlanks(8)) {
                    PurchAir(PurchAirNum).CoolingLimit = LimitType::None;
                }
            } break;

            case LimitType::FlowRateAndCapacity: {
                if (s_ipsc->lNumericFieldBlanks(7) && s_ipsc->lNumericFieldBlanks(8)) {
                    PurchAir(PurchAirNum).CoolingLimit = LimitType::None;
                } else if (s_ipsc->lNumericFieldBlanks(7)) {
                    PurchAir(PurchAirNum).CoolingLimit = LimitType::Capacity;
                } else if (s_ipsc->lNumericFieldBlanks(8)) {
                    PurchAir(PurchAirNum).CoolingLimit = LimitType::FlowRate;
                }
            } break;

            default: {
                ShowSevereInvalidKey(state,
                                     eoh,
                                     s_ipsc->cAlphaFieldNames(6),
                                     s_ipsc->cAlphaArgs(6),
                                     "Valid entries are None, ConstantSensibleHeatRatio, Humidistat, or ConstantSupplyHumidityRatio");
                ErrorsFound = true;
            } break;
            }

            PurchAir(PurchAirNum).MaxCoolVolFlowRate = s_ipsc->rNumericArgs(7);
            PurchAir(PurchAirNum).MaxCoolTotCap = s_ipsc->rNumericArgs(8);

            // get optional heating availability schedule
            if (s_ipsc->lAlphaFieldBlanks(8)) {
                PurchAir(PurchAirNum).heatAvailSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((PurchAir(PurchAirNum).heatAvailSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(8))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(8), s_ipsc->cAlphaArgs(8));
                ErrorsFound = true;
            }

            // get optional cooling availability schedule
            if (s_ipsc->lAlphaFieldBlanks(9)) {
                PurchAir(PurchAirNum).coolAvailSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((PurchAir(PurchAirNum).coolAvailSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(9))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(9), s_ipsc->cAlphaArgs(9));
                ErrorsFound = true;
            }

            // get Dehumidification control type
            PurchAir(PurchAirNum).DehumidCtrlType = static_cast<HumControl>(getEnumValue(humControlNamesUC, s_ipsc->cAlphaArgs(10)));
            if (PurchAir(PurchAirNum).DehumidCtrlType == HumControl::Invalid) {
                ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(10), s_ipsc->cAlphaArgs(10));
                ErrorsFound = true;
            }

            PurchAir(PurchAirNum).CoolSHR = s_ipsc->rNumericArgs(9);

            // get Humidification control type
            PurchAir(PurchAirNum).HumidCtrlType = static_cast<HumControl>(getEnumValue(humControlNamesUC, s_ipsc->cAlphaArgs(11)));
            if (PurchAir(PurchAirNum).HumidCtrlType == HumControl::Invalid ||
                PurchAir(PurchAirNum).HumidCtrlType == HumControl::ConstantSensibleHeatRatio) {
                ShowSevereInvalidKey(state,
                                     eoh,
                                     s_ipsc->cAlphaFieldNames(11),
                                     s_ipsc->cAlphaArgs(11),
                                     "Valid entries are None, Humidistat, or ConstantSupplyHumidityRatio");
                ErrorsFound = true;
            }

            // get Design specification outdoor air object
            if (!s_ipsc->lAlphaFieldBlanks(12)) {
                PurchAir(PurchAirNum).OARequirementsPtr = Util::FindItemInList(s_ipsc->cAlphaArgs(12), state.dataSize->OARequirements);
                if (PurchAir(PurchAirNum).OARequirementsPtr == 0) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(12), s_ipsc->cAlphaArgs(12));
                    ErrorsFound = true;
                } else {
                    PurchAir(PurchAirNum).OutdoorAir = true;
                }
            }

            // If outdoor air specified, then get Outdoor air inlet node and other outdoor air inputs
            if (PurchAir(PurchAirNum).OutdoorAir) {
                if (s_ipsc->lAlphaFieldBlanks(13)) {
                    // If there is outdoor air and outdoor air inlet node is blank, then create one
                    if (len(s_ipsc->cAlphaArgs(1)) < Constant::MaxNameLength - 23) { // protect against long name leading to > 100 chars
                        s_ipsc->cAlphaArgs(13) = s_ipsc->cAlphaArgs(1) + " OUTDOOR AIR INLET NODE";
                    } else {
                        s_ipsc->cAlphaArgs(13) = s_ipsc->cAlphaArgs(1).substr(0, 75) + " OUTDOOR AIR INLET NODE";
                    }
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        ShowWarningError(state, format("{}{}=\"{} blank field", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                        ShowContinueError(state,
                                          format("{} is blank, but there is outdoor air requested for this system.", s_ipsc->cAlphaFieldNames(13)));
                        ShowContinueError(state, format("Creating node name ={}", s_ipsc->cAlphaArgs(13)));
                    }
                }
                // Register OA node
                PurchAir(PurchAirNum).OutdoorAirNodeNum = GetOnlySingleNode(state,
                                                                            s_ipsc->cAlphaArgs(13),
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::ZoneHVACIdealLoadsAirSystem,
                                                                            s_ipsc->cAlphaArgs(1),
                                                                            DataLoopNode::NodeFluidType::Air,
                                                                            DataLoopNode::ConnectionType::Outlet,
                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                            ObjectIsNotParent);
                // Check if OA node is initialized in OutdoorAir:Node or OutdoorAir:Nodelist
                bool IsOANodeListed; // Flag for OA node name listed in OutdoorAir:Node or Nodelist
                CheckAndAddAirNodeNumber(state, PurchAir(PurchAirNum).OutdoorAirNodeNum, IsOANodeListed);
                if ((!IsOANodeListed) && state.dataGlobal->DisplayExtraWarnings) {
                    ShowWarningError(state, format("{}{}=\"{} missing data", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state,
                                      format("{} does not appear in an OutdoorAir:NodeList or as an OutdoorAir:Node.", s_ipsc->cAlphaArgs(13)));
                    ShowContinueError(state, format("Adding OutdoorAir:Node={}", s_ipsc->cAlphaArgs(13)));
                }
                UniqueNodeError = false;
                CheckUniqueNodeNames(state, s_ipsc->cAlphaFieldNames(13), UniqueNodeError, s_ipsc->cAlphaArgs(13), s_ipsc->cAlphaArgs(1));
                if (UniqueNodeError) {
                    ErrorsFound = true;
                }

                // get Demand controlled ventilation type
                PurchAir(PurchAirNum).DCVType = static_cast<DCV>(getEnumValue(dcvNamesUC, s_ipsc->cAlphaArgs(14)));
                if (PurchAir(PurchAirNum).DCVType == DCV::Invalid) {
                    ShowSevereInvalidKey(state,
                                         eoh,
                                         s_ipsc->cAlphaFieldNames(14),
                                         s_ipsc->cAlphaArgs(14),
                                         "Valid entries are None, OccupancySchedule, or CO2Setpoint");
                    ErrorsFound = true;
                } else if (PurchAir(PurchAirNum).DCVType == DCV::CO2SetPoint && !state.dataContaminantBalance->Contaminant.CO2Simulation) {
                    PurchAir(PurchAirNum).DCVType = DCV::None;
                    ShowWarningError(state, format("{}{}=\"{} invalid data", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("{}={} but CO2 simulation is not active.", s_ipsc->cAlphaFieldNames(14), s_ipsc->cAlphaArgs(14)));
                    ShowContinueError(state, format("Resetting {} to NoDCV", s_ipsc->cAlphaFieldNames(14)));
                    ShowContinueError(state,
                                      "To activate CO2 simulation, use ZoneAirContaminantBalance object and specify \"Carbon Dioxide "
                                      "Concentration\"=\"Yes\".");
                }

                // get Outdoor air economizer type
                PurchAir(PurchAirNum).EconomizerType = static_cast<Econ>(getEnumValue(econNamesUC, s_ipsc->cAlphaArgs(15)));
                if (PurchAir(PurchAirNum).EconomizerType == Econ::Invalid) {
                    ShowSevereInvalidKey(state,
                                         eoh,
                                         s_ipsc->cAlphaFieldNames(15),
                                         s_ipsc->cAlphaArgs(15),
                                         "Valid entries are NoEconomizer, DifferentialDryBulb, or DifferentialEnthalpy");
                    ErrorsFound = true;
                }

                // get Outdoor air heat recovery type and effectiveness
                PurchAir(PurchAirNum).HtRecType = static_cast<HeatRecovery>(getEnumValue(heatRecoveryNamesUC, s_ipsc->cAlphaArgs(16)));
                if (PurchAir(PurchAirNum).HtRecType == HeatRecovery::Invalid) {
                    ShowSevereInvalidKey(
                        state, eoh, s_ipsc->cAlphaFieldNames(16), s_ipsc->cAlphaArgs(16), "Valid entries are None, Sensible, or Enthalpy");
                    ErrorsFound = true;
                }

            } else { // No outdoorair
                PurchAir(PurchAirNum).DCVType = DCV::None;
                PurchAir(PurchAirNum).EconomizerType = Econ::NoEconomizer;
                PurchAir(PurchAirNum).HtRecType = HeatRecovery::None;
            }

            PurchAir(PurchAirNum).HtRecSenEff = s_ipsc->rNumericArgs(10);
            PurchAir(PurchAirNum).HtRecLatEff = s_ipsc->rNumericArgs(11);

            for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                    continue;
                }
                for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumInletNodes; ++NodeNum) {
                    if (PurchAir(PurchAirNum).ZoneSupplyAirNodeNum == state.dataZoneEquip->ZoneEquipConfig(CtrlZone).InletNode(NodeNum)) {
                        PurchAir(PurchAirNum).ZonePtr = CtrlZone;
                    }
                }
            }

            PurchAir(PurchAirNum).HVACSizingIndex = 0;
            if (!s_ipsc->lAlphaFieldBlanks(17)) {
                PurchAir(PurchAirNum).HVACSizingIndex = Util::FindItemInList(s_ipsc->cAlphaArgs(17), state.dataSize->ZoneHVACSizing);
                if (PurchAir(PurchAirNum).HVACSizingIndex == 0) {
                    ShowSevereError(state, format("{} = {} not found.", s_ipsc->cAlphaFieldNames(17), s_ipsc->cAlphaArgs(17)));
                    ShowContinueError(state, format("Occurs in {} = {}", s_ipsc->cCurrentModuleObject, PurchAir(PurchAirNum).Name));
                    ErrorsFound = true;
                }
            }

            // initialize the calculated and report values
            PurchAir(PurchAirNum).MaxHeatMassFlowRate = 0.0;
            PurchAir(PurchAirNum).MaxCoolMassFlowRate = 0.0;
            PurchAir(PurchAirNum).SenHeatEnergy = 0.0;
            PurchAir(PurchAirNum).LatHeatEnergy = 0.0;
            PurchAir(PurchAirNum).TotHeatEnergy = 0.0;
            PurchAir(PurchAirNum).SenCoolEnergy = 0.0;
            PurchAir(PurchAirNum).LatCoolEnergy = 0.0;
            PurchAir(PurchAirNum).TotCoolEnergy = 0.0;
            PurchAir(PurchAirNum).ZoneSenHeatEnergy = 0.0;
            PurchAir(PurchAirNum).ZoneLatHeatEnergy = 0.0;
            PurchAir(PurchAirNum).ZoneTotHeatEnergy = 0.0;
            PurchAir(PurchAirNum).ZoneSenCoolEnergy = 0.0;
            PurchAir(PurchAirNum).ZoneLatCoolEnergy = 0.0;
            PurchAir(PurchAirNum).ZoneTotCoolEnergy = 0.0;
            PurchAir(PurchAirNum).OASenHeatEnergy = 0.0;
            PurchAir(PurchAirNum).OALatHeatEnergy = 0.0;
            PurchAir(PurchAirNum).OATotHeatEnergy = 0.0;
            PurchAir(PurchAirNum).OASenCoolEnergy = 0.0;
            PurchAir(PurchAirNum).OALatCoolEnergy = 0.0;
            PurchAir(PurchAirNum).OATotCoolEnergy = 0.0;
            PurchAir(PurchAirNum).HtRecSenHeatEnergy = 0.0;
            PurchAir(PurchAirNum).HtRecLatHeatEnergy = 0.0;
            PurchAir(PurchAirNum).HtRecTotHeatEnergy = 0.0;
            PurchAir(PurchAirNum).HtRecSenCoolEnergy = 0.0;
            PurchAir(PurchAirNum).HtRecLatCoolEnergy = 0.0;
            PurchAir(PurchAirNum).HtRecTotCoolEnergy = 0.0;
            PurchAir(PurchAirNum).SenHeatRate = 0.0;
            PurchAir(PurchAirNum).LatHeatRate = 0.0;
            PurchAir(PurchAirNum).TotHeatRate = 0.0;
            PurchAir(PurchAirNum).SenCoolRate = 0.0;
            PurchAir(PurchAirNum).LatCoolRate = 0.0;
            PurchAir(PurchAirNum).TotCoolRate = 0.0;
            PurchAir(PurchAirNum).ZoneSenHeatRate = 0.0;
            PurchAir(PurchAirNum).ZoneLatHeatRate = 0.0;
            PurchAir(PurchAirNum).ZoneTotHeatRate = 0.0;
            PurchAir(PurchAirNum).ZoneSenCoolRate = 0.0;
            PurchAir(PurchAirNum).ZoneLatCoolRate = 0.0;
            PurchAir(PurchAirNum).ZoneTotCoolRate = 0.0;
            PurchAir(PurchAirNum).OASenHeatRate = 0.0;
            PurchAir(PurchAirNum).OALatHeatRate = 0.0;
            PurchAir(PurchAirNum).OATotHeatRate = 0.0;
            PurchAir(PurchAirNum).OASenCoolRate = 0.0;
            PurchAir(PurchAirNum).OALatCoolRate = 0.0;
            PurchAir(PurchAirNum).OATotCoolRate = 0.0;
            PurchAir(PurchAirNum).HtRecSenHeatRate = 0.0;
            PurchAir(PurchAirNum).HtRecLatHeatRate = 0.0;
            PurchAir(PurchAirNum).HtRecTotHeatRate = 0.0;
            PurchAir(PurchAirNum).HtRecSenCoolRate = 0.0;
            PurchAir(PurchAirNum).HtRecLatCoolRate = 0.0;
            PurchAir(PurchAirNum).HtRecTotCoolRate = 0.0;

            PurchAir(PurchAirNum).OutdoorAirMassFlowRate = 0.0;
            PurchAir(PurchAirNum).OutdoorAirVolFlowRateStdRho = 0.0;
            PurchAir(PurchAirNum).SupplyAirMassFlowRate = 0.0;
            PurchAir(PurchAirNum).SupplyAirVolFlowRateStdRho = 0.0;
        }
        EndUniqueNodeCheck(state, s_ipsc->cCurrentModuleObject);
    }

    for (int PurchAirNum = 1; PurchAirNum <= state.dataPurchasedAirMgr->NumPurchAir; ++PurchAirNum) {

        // Setup Output variables
        //    energy variables
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Sensible Heating Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).SenHeatEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Latent Heating Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).LatHeatEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Total Heating Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).TotHeatEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name,
                            Constant::eResource::DistrictHeatingWater,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Heating);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Sensible Cooling Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).SenCoolEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Latent Cooling Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).LatCoolEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Total Cooling Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).TotCoolEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name,
                            Constant::eResource::DistrictCooling,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Cooling);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Zone Sensible Heating Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).ZoneSenHeatEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Zone Latent Heating Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).ZoneLatHeatEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Zone Total Heating Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).ZoneTotHeatEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Zone Sensible Cooling Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).ZoneSenCoolEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Zone Latent Cooling Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).ZoneLatCoolEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Zone Total Cooling Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).ZoneTotCoolEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Outdoor Air Sensible Heating Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).OASenHeatEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Outdoor Air Latent Heating Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).OALatHeatEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Outdoor Air Total Heating Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).OATotHeatEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Outdoor Air Sensible Cooling Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).OASenCoolEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Outdoor Air Latent Cooling Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).OALatCoolEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Outdoor Air Total Cooling Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).OATotCoolEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Heat Recovery Sensible Heating Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).HtRecSenHeatEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Heat Recovery Latent Heating Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).HtRecLatHeatEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Heat Recovery Total Heating Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).HtRecTotHeatEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Heat Recovery Sensible Cooling Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).HtRecSenCoolEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Heat Recovery Latent Cooling Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).HtRecLatCoolEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Heat Recovery Total Cooling Energy",
                            Constant::Units::J,
                            PurchAir(PurchAirNum).HtRecTotCoolEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);

        //    rate variables
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Sensible Heating Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).SenHeatRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Latent Heating Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).LatHeatRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Total Heating Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).TotHeatRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Sensible Cooling Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).SenCoolRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Latent Cooling Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).LatCoolRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Total Cooling Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).TotCoolRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Zone Sensible Heating Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).ZoneSenHeatRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Zone Latent Heating Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).ZoneLatHeatRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Zone Total Heating Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).ZoneTotHeatRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Zone Sensible Cooling Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).ZoneSenCoolRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Zone Latent Cooling Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).ZoneLatCoolRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Zone Total Cooling Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).ZoneTotCoolRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Outdoor Air Sensible Heating Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).OASenHeatRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Outdoor Air Latent Heating Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).OALatHeatRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Outdoor Air Total Heating Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).OATotHeatRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Outdoor Air Sensible Cooling Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).OASenCoolRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Outdoor Air Latent Cooling Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).OALatCoolRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Outdoor Air Total Cooling Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).OATotCoolRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Heat Recovery Sensible Heating Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).HtRecSenHeatRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Heat Recovery Latent Heating Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).HtRecLatHeatRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Heat Recovery Total Heating Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).HtRecTotHeatRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Heat Recovery Sensible Cooling Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).HtRecSenCoolRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Heat Recovery Latent Cooling Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).HtRecLatCoolRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Heat Recovery Total Cooling Rate",
                            Constant::Units::W,
                            PurchAir(PurchAirNum).HtRecTotCoolRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);

        SetupOutputVariable(state,
                            "Zone Ideal Loads Economizer Active Time",
                            Constant::Units::hr,
                            PurchAir(PurchAirNum).TimeEconoActive,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Heat Recovery Active Time",
                            Constant::Units::hr,
                            PurchAir(PurchAirNum).TimeHtRecActive,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            PurchAir(PurchAirNum).Name);

        SetupOutputVariable(state,
                            "Zone Ideal Loads Hybrid Ventilation Available Status",
                            Constant::Units::None,
                            (int &)PurchAir(PurchAirNum).availStatus,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);

        // air flows
        SetupOutputVariable(state,
                            "Zone Ideal Loads Outdoor Air Mass Flow Rate",
                            Constant::Units::kg_s,
                            PurchAir(PurchAirNum).OutdoorAirMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Outdoor Air Standard Density Volume Flow Rate",
                            Constant::Units::m3_s,
                            PurchAir(PurchAirNum).OutdoorAirVolFlowRateStdRho,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Mass Flow Rate",
                            Constant::Units::kg_s,
                            PurchAir(PurchAirNum).SupplyAirMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Standard Density Volume Flow Rate",
                            Constant::Units::m3_s,
                            PurchAir(PurchAirNum).SupplyAirVolFlowRateStdRho,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);

        // Supply Air temperature
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Temperature",
                            Constant::Units::C,
                            PurchAir(PurchAirNum).SupplyTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        // Supply Air Humidity Ratio
        SetupOutputVariable(state,
                            "Zone Ideal Loads Supply Air Humidity Ratio",
                            Constant::Units::kgWater_kgDryAir,
                            PurchAir(PurchAirNum).SupplyHumRat,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);

        // Mixed Air temperature
        SetupOutputVariable(state,
                            "Zone Ideal Loads Mixed Air Temperature",
                            Constant::Units::C,
                            PurchAir(PurchAirNum).MixedAirTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);
        // Mixed Air Humidity Ratio
        SetupOutputVariable(state,
                            "Zone Ideal Loads Mixed Air Humidity Ratio",
                            Constant::Units::kgWater_kgDryAir,
                            PurchAir(PurchAirNum).MixedAirHumRat,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            PurchAir(PurchAirNum).Name);

        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            SetupEMSActuator(state,
                             "Ideal Loads Air System",
                             PurchAir(PurchAirNum).Name,
                             "Air Mass Flow Rate",
                             "[kg/s]",
                             PurchAir(PurchAirNum).EMSOverrideMdotOn,
                             PurchAir(PurchAirNum).EMSValueMassFlowRate);
            SetupEMSActuator(state,
                             "Ideal Loads Air System",
                             PurchAir(PurchAirNum).Name,
                             "Outdoor Air Mass Flow Rate",
                             "[kg/s]",
                             PurchAir(PurchAirNum).EMSOverrideOAMdotOn,
                             PurchAir(PurchAirNum).EMSValueOAMassFlowRate);
            SetupEMSActuator(state,
                             "Ideal Loads Air System",
                             PurchAir(PurchAirNum).Name,
                             "Air Temperature",
                             "[C]",
                             PurchAir(PurchAirNum).EMSOverrideSupplyTempOn,
                             PurchAir(PurchAirNum).EMSValueSupplyTemp);
            SetupEMSActuator(state,
                             "Ideal Loads Air System",
                             PurchAir(PurchAirNum).Name,
                             "Air Humidity Ratio",
                             "[kgWater/kgDryAir]",
                             PurchAir(PurchAirNum).EMSOverrideSupplyHumRatOn,
                             PurchAir(PurchAirNum).EMSValueSupplyHumRat);
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("{}Errors found in input. Preceding conditions cause termination.", RoutineName));
    }
}

void InitPurchasedAir(EnergyPlusData &state, int const PurchAirNum, int const ControlledZoneNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   Nov 1997

    // PURPOSE OF THIS SUBROUTINE:
    // Initialize the PurchAir data structure.

    // Using/Aliasing
    using DataZoneEquipment::CheckZoneEquipmentList;
    using General::FindNumberInList;
    using ZonePlenum::GetReturnPlenumIndex;
    using ZonePlenum::GetReturnPlenumName;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool UnitOn; // simple checks for error

    // Do the Begin Simulation initializations
    if (state.dataPurchasedAirMgr->InitPurchasedAirMyOneTimeFlag) {
        state.dataPurchasedAirMgr->InitPurchasedAirMyEnvrnFlag.allocate(state.dataPurchasedAirMgr->NumPurchAir);
        state.dataPurchasedAirMgr->InitPurchasedAirMySizeFlag.allocate(state.dataPurchasedAirMgr->NumPurchAir);
        state.dataPurchasedAirMgr->InitPurchasedAirOneTimeUnitInitsDone.allocate(state.dataPurchasedAirMgr->NumPurchAir);
        state.dataPurchasedAirMgr->InitPurchasedAirMyEnvrnFlag = true;
        state.dataPurchasedAirMgr->InitPurchasedAirMySizeFlag = true;
        state.dataPurchasedAirMgr->InitPurchasedAirOneTimeUnitInitsDone = false;
        state.dataPurchasedAirMgr->InitPurchasedAirMyOneTimeFlag = false;
    }

    // need to check all units to see if they are on Zone Equipment List or issue warning
    if (!state.dataPurchasedAirMgr->InitPurchasedAirZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
        state.dataPurchasedAirMgr->InitPurchasedAirZoneEquipmentListChecked = true;
        for (int Loop = 1; Loop <= state.dataPurchasedAirMgr->NumPurchAir; ++Loop) {
            auto &PurchAirLoop = state.dataPurchasedAirMgr->PurchAir(Loop);

            // link with return plenum if used (i.e., PlenumExhaustAirNodeNum will be non-zero)
            if (PurchAirLoop.PlenumExhaustAirNodeNum > 0) {
                PurchAirLoop.ReturnPlenumIndex = GetReturnPlenumIndex(state, PurchAirLoop.PlenumExhaustAirNodeNum);
                if (PurchAirLoop.ReturnPlenumIndex > 0) {
                    GetReturnPlenumName(state, PurchAirLoop.ReturnPlenumIndex, PurchAirLoop.ReturnPlenumName);
                    InitializePlenumArrays(state, Loop);
                } else {
                    ShowSevereError(state,
                                    format("InitPurchasedAir: {} = {} cannot find ZoneHVAC:ReturnPlenum.  It will not be simulated.",
                                           PurchAirLoop.cObjectName,
                                           PurchAirLoop.Name));
                }
            }

            if (CheckZoneEquipmentList(state, PurchAirLoop.cObjectName, PurchAirLoop.Name)) {
                continue;
            }
            ShowSevereError(state,
                            format("InitPurchasedAir: {} = {} is not on any ZoneHVAC:EquipmentList.  It will not be simulated.",
                                   PurchAirLoop.cObjectName,
                                   PurchAirLoop.Name));
        }
    }

    auto &PurchAir = state.dataPurchasedAirMgr->PurchAir(PurchAirNum);
    // one time inits for each unit - links PurchAirNum with static input data from ControlledZoneNum and ActualZoneNum
    if (!state.dataPurchasedAirMgr->InitPurchasedAirOneTimeUnitInitsDone(PurchAirNum)) {
        state.dataPurchasedAirMgr->InitPurchasedAirOneTimeUnitInitsDone(PurchAirNum) = true;

        // Is the supply node really a zone inlet node?
        // this check has to be done here because of SimPurchasedAir passing in ControlledZoneNum
        int SupplyNodeNum = PurchAir.ZoneSupplyAirNodeNum;
        if (SupplyNodeNum > 0) {
            int NodeIndex = FindNumberInList(SupplyNodeNum,
                                             state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNode,
                                             state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes);
            if (NodeIndex == 0) {
                ShowSevereError(state, format("InitPurchasedAir: In {} = {}", PurchAir.cObjectName, PurchAir.Name));
                ShowContinueError(state,
                                  format("Zone Supply Air Node Name={} is not a zone inlet node.", state.dataLoopNodes->NodeID(SupplyNodeNum)));
                ShowContinueError(
                    state,
                    format("Check ZoneHVAC:EquipmentConnections for zone={}", state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneName));
                ShowFatalError(state, "Preceding condition causes termination.");
            }
        }

        // Set recirculation node number
        // If exhaust node is specified, then recirculation is exhaust node, otherwise use zone return node
        // this check has to be done here because of SimPurchasedAir passing in ControlledZoneNum
        bool UseReturnNode = false;
        if (PurchAir.ZoneExhaustAirNodeNum > 0) {
            int ExhaustNodeNum = PurchAir.ZoneExhaustAirNodeNum;
            int NodeIndex = FindNumberInList(ExhaustNodeNum,
                                             state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ExhaustNode,
                                             state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumExhaustNodes);
            if (NodeIndex == 0) {
                ShowSevereError(state, format("InitPurchasedAir: In {} = {}", PurchAir.cObjectName, PurchAir.Name));
                ShowContinueError(state,
                                  format("Zone Exhaust Air Node Name={} is not a zone exhaust node.", state.dataLoopNodes->NodeID(ExhaustNodeNum)));
                ShowContinueError(
                    state,
                    format("Check ZoneHVAC:EquipmentConnections for zone={}", state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneName));
                ShowContinueError(state, "Zone return air node will be used for ideal loads recirculation air.");
                UseReturnNode = true;
            } else {
                PurchAir.ZoneRecircAirNodeNum = PurchAir.ZoneExhaustAirNodeNum;
            }
        } else {
            UseReturnNode = true;
        }
        if (UseReturnNode) {
            if (state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumReturnNodes == 1) {
                PurchAir.ZoneRecircAirNodeNum = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ReturnNode(1);
            } else if (state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumReturnNodes > 1) {
                ShowWarningError(state, format("InitPurchasedAir: In {} = {}", PurchAir.cObjectName, PurchAir.Name));
                ShowContinueError(state,
                                  "No Zone Exhaust Air Node Name has been specified for this system and the zone has more than one Return Air Node.");
                ShowContinueError(state,
                                  format("Using the first return air node ={}",
                                         state.dataLoopNodes->NodeID(state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ReturnNode(1))));
            } else {
                ShowFatalError(state, format("InitPurchasedAir: In {} = {}", PurchAir.cObjectName, PurchAir.Name));
                ShowContinueError(
                    state,
                    " Invalid recirculation node. No exhaust or return node has been specified for this zone in ZoneHVAC:EquipmentConnections.");
                ShowFatalError(state, "Preceding condition causes termination.");
            }
        }
        // If there is OA and economizer is active, then there must be a limit on cooling flow rate
        if (PurchAir.OutdoorAir && (PurchAir.EconomizerType != Econ::NoEconomizer)) {
            if ((PurchAir.CoolingLimit == LimitType::None) || (PurchAir.CoolingLimit == LimitType::Capacity)) {
                ShowSevereError(state, format("InitPurchasedAir: In {} = {}", PurchAir.cObjectName, PurchAir.Name));
                ShowContinueError(state, "There is outdoor air with economizer active but there is no limit on cooling air flow rate.");
                ShowContinueError(state,
                                  "Cooling Limit must be set to LimitFlowRate or LimitFlowRateAndCapacity, and Maximum Cooling Air Flow Rate "
                                  "must be set to a value or autosize.");
                ShowContinueError(state, "Simulation will proceed with no limit on outdoor air flow rate.");
            }
        }
    }

    if (!state.dataGlobal->SysSizingCalc && state.dataPurchasedAirMgr->InitPurchasedAirMySizeFlag(PurchAirNum)) {

        SizePurchasedAir(state, PurchAirNum);

        state.dataPurchasedAirMgr->InitPurchasedAirMySizeFlag(PurchAirNum) = false;
    }

    // Do the Begin Environment initializations
    if (state.dataGlobal->BeginEnvrnFlag && state.dataPurchasedAirMgr->InitPurchasedAirMyEnvrnFlag(PurchAirNum)) {

        if ((PurchAir.HeatingLimit == LimitType::FlowRate) || (PurchAir.HeatingLimit == LimitType::FlowRateAndCapacity)) {
            PurchAir.MaxHeatMassFlowRate = state.dataEnvrn->StdRhoAir * PurchAir.MaxHeatVolFlowRate;
        } else {
            PurchAir.MaxHeatMassFlowRate = 0.0;
        }
        if ((PurchAir.CoolingLimit == LimitType::FlowRate) || (PurchAir.CoolingLimit == LimitType::FlowRateAndCapacity)) {
            PurchAir.MaxCoolMassFlowRate = state.dataEnvrn->StdRhoAir * PurchAir.MaxCoolVolFlowRate;
        } else {
            PurchAir.MaxCoolMassFlowRate = 0.0;
        }
        state.dataPurchasedAirMgr->InitPurchasedAirMyEnvrnFlag(PurchAirNum) = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataPurchasedAirMgr->InitPurchasedAirMyEnvrnFlag(PurchAirNum) = true;
    }

    auto const &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(ControlledZoneNum);

    // These initializations are done every iteration
    // check that supply air temps can meet the zone thermostat setpoints
    if (PurchAir.MinCoolSuppAirTemp > zoneTstatSetpt.setptHi && zoneTstatSetpt.setptHi != 0 && PurchAir.CoolingLimit == LimitType::None) {
        // Check if the unit is scheduled off
        UnitOn = true;
        //        IF (PurchAir(PurchAirNum)%AvailSchedPtr > 0) THEN
        if (PurchAir.availSched->getCurrentVal() <= 0) {
            UnitOn = false;
        }
        //        END IF
        // Check if cooling available
        bool CoolOn = true;
        //        IF (PurchAir(PurchAirNum)%CoolSchedPtr > 0) THEN
        if (PurchAir.coolAvailSched->getCurrentVal() <= 0) {
            CoolOn = false;
        }
        //        END IF
        if (UnitOn && CoolOn) {
            if (PurchAir.CoolErrIndex == 0) {
                ShowSevereError(state,
                                format("InitPurchasedAir: For {} = {} serving Zone {}",
                                       PurchAir.cObjectName,
                                       PurchAir.Name,
                                       state.dataHeatBal->Zone(ControlledZoneNum).Name));
                ShowContinueError(state,
                                  format("..the minimum supply air temperature for cooling [{:.2R}] is greater than the zone cooling mean air "
                                         "temperature (MAT) setpoint [{:.2R}].",
                                         PurchAir.MinCoolSuppAirTemp,
                                         zoneTstatSetpt.setptHi));
                ShowContinueError(state, "..For operative and comfort thermostat controls, the MAT setpoint is computed.");
                ShowContinueError(state, "..This error may indicate that the mean radiant temperature or another comfort factor is too warm.");
                ShowContinueError(state, "Unit availability is nominally ON and Cooling availability is nominally ON.");
                ShowContinueError(state, format("Limit Cooling Capacity Type={}", limitTypeNames[(int)PurchAir.CoolingLimit]));
                // could check for optemp control or comfort control here
                ShowContinueErrorTimeStamp(state, "");
            }
            ShowRecurringSevereErrorAtEnd(state,
                                          "InitPurchasedAir: For " + PurchAir.cObjectName + " = " + PurchAir.Name + " serving Zone " +
                                              state.dataHeatBal->Zone(ControlledZoneNum).Name +
                                              ", the minimum supply air temperature for cooling error continues",
                                          PurchAir.CoolErrIndex,
                                          PurchAir.MinCoolSuppAirTemp,
                                          PurchAir.MinCoolSuppAirTemp,
                                          _,
                                          "C",
                                          "C");
        }
    }

    if (PurchAir.MaxHeatSuppAirTemp < zoneTstatSetpt.setptLo && zoneTstatSetpt.setptLo != 0 && PurchAir.HeatingLimit == LimitType::None) {
        // Check if the unit is scheduled off
        UnitOn = true;
        //        IF (PurchAir(PurchAirNum)%AvailSchedPtr > 0) THEN
        if (PurchAir.availSched->getCurrentVal() <= 0) {
            UnitOn = false;
        }
        //        END IF
        // Check if heating and cooling available

        bool HeatOn = true;
        //        IF (PurchAir(PurchAirNum)%HeatSchedPtr > 0) THEN
        if (PurchAir.heatAvailSched->getCurrentVal() <= 0) {
            HeatOn = false;
        }
        //        END IF
        if (UnitOn && HeatOn) {
            if (PurchAir.HeatErrIndex == 0) {
                ShowSevereMessage(state,
                                  format("InitPurchasedAir: For {} = {} serving Zone {}",
                                         PurchAir.cObjectName,
                                         PurchAir.Name,
                                         state.dataHeatBal->Zone(ControlledZoneNum).Name));
                ShowContinueError(state,
                                  format("..the maximum supply air temperature for heating [{:.2R}] is less than the zone mean air temperature "
                                         "heating setpoint [{:.2R}].",
                                         PurchAir.MaxHeatSuppAirTemp,
                                         zoneTstatSetpt.setptLo));
                ShowContinueError(state, "..For operative and comfort thermostat controls, the MAT setpoint is computed.");
                ShowContinueError(state, "..This error may indicate that the mean radiant temperature or another comfort factor is too cold.");
                ShowContinueError(state, "Unit availability is nominally ON and Heating availability is nominally ON.");
                ShowContinueError(state, format("Limit Heating Capacity Type={}", limitTypeNames[(int)PurchAir.HeatingLimit]));
                // could check for optemp control or comfort control here
                ShowContinueErrorTimeStamp(state, "");
            }
            ShowRecurringSevereErrorAtEnd(state,
                                          "InitPurchasedAir: For " + PurchAir.cObjectName + " = " + PurchAir.Name + " serving Zone " +
                                              state.dataHeatBal->Zone(ControlledZoneNum).Name +
                                              ", maximum supply air temperature for heating error continues",
                                          PurchAir.HeatErrIndex,
                                          PurchAir.MaxHeatSuppAirTemp,
                                          PurchAir.MaxHeatSuppAirTemp,
                                          _,
                                          "C",
                                          "C");
        }
    }
    //      IF (ErrorsFound .and. .not. WarmupFlag) THEN
    //        CALL ShowFatalError(state, 'Preceding conditions cause termination.')
    //      ENDIF
}

void SizePurchasedAir(EnergyPlusData &state, int const PurchAirNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   April 2003
    //       MODIFIED       M. Witte, June 2011, add sizing for new capacity fields
    //                      August 2013 Daeho Kang, add component sizing table entries
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for sizing Purchased Air Components for which flow rates have not been
    // specified in the input.

    // METHODOLOGY EMPLOYED:
    // Obtains flow rates from the zone sizing arrays.

    // Using/Aliasing
    using namespace DataSizing;
    using HVAC::CoolingCapacitySizing;
    using HVAC::HeatingAirflowSizing;
    using HVAC::HeatingCapacitySizing;
    using Psychrometrics::CPCW;
    using Psychrometrics::CPHW;
    using Psychrometrics::PsyCpAirFnW;
    using Psychrometrics::PsyHFnTdbW;
    using Psychrometrics::RhoH2O;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("SizePurchasedAir: "); // include trailing blank space

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 MaxHeatVolFlowRateDes;     // Autosized maximum heating air flow for reporting
    Real64 MaxHeatVolFlowRateUser;    // Hardsized maximum heating air flow for reporting
    Real64 MaxCoolVolFlowRateDes;     // Autosized maximum cooling air flow for reporting
    Real64 MaxCoolVolFlowRateUser;    // Hardsized maximum cooling air flow for reporting
    Real64 MaxHeatSensCapDes;         // Autosized maximum sensible heating capacity for reporting
    Real64 MaxHeatSensCapUser;        // Hardsized maximum sensible heating capacity for reporting
    Real64 MaxCoolTotCapDes;          // Autosized maximum sensible cooling capacity for reporting
    Real64 MaxCoolTotCapUser;         // Hardsized maximum sensible cooling capacity for reporting
    std::string CompName;             // component name
    std::string CompType;             // component type
    Real64 TempSize;                  // autosized value of coil input field
                                      // FractionOfAutosizedHeatingCapacity )
    Real64 CoolingAirVolFlowDes(0.0); // cooling supply air flow rate
    Real64 HeatingAirVolFlowDes(0.0); // heating supply air flow rate

    auto &PurchAir = state.dataPurchasedAirMgr->PurchAir(PurchAirNum);

    MaxHeatVolFlowRateDes = 0.0;
    MaxHeatVolFlowRateUser = 0.0;
    MaxCoolVolFlowRateDes = 0.0;
    MaxCoolVolFlowRateUser = 0.0;
    MaxHeatSensCapDes = 0.0;
    MaxHeatSensCapUser = 0.0;
    MaxCoolTotCapDes = 0.0;
    MaxCoolTotCapUser = 0.0;

    state.dataSize->ZoneHeatingOnlyFan = false;
    state.dataSize->ZoneCoolingOnlyFan = false;
    CompType = PurchAir.cObjectName;
    CompName = PurchAir.Name;

    if (state.dataSize->CurZoneEqNum > 0) {
        auto &ZoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);
        std::string SizingString; // input field sizing description (e.g., Nominal Capacity)
        int FieldNum;             // IDD numeric field number where input field description is found
        bool PrintFlag;           // TRUE when sizing information is reported in the eio file
        bool ErrorsFound = false;
        if (PurchAir.HVACSizingIndex > 0) {
            state.dataSize->DataZoneNumber = PurchAir.ZonePtr;
            int zoneHVACIndex = PurchAir.HVACSizingIndex;
            int SAFMethod;       // supply air flow rate sizing method (SupplyAirFlowRate, FlowPerFloorArea, FractionOfAutosizedCoolingAirflow,
                                 // FractionOfAutosizedHeatingAirflow, HeatingCapacitySizing, etc.)
            int CapSizingMethod; // capacity sizing methods (HeatingDesignCapacity, CapacityPerFloorArea, FractionOfAutosizedCoolingCapacity, and
            int SizingMethod;    // Integer representation of sizing method name (e.g., CoolingAirflowSizing, HeatingAirflowSizing,
                                 // CoolingCapacitySizing)

            FieldNum = 5; // N5 , \field Maximum Heating Air Flow Rate
            PrintFlag = true;
            SizingString = state.dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(FieldNum) + " [m3/s]";
            if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingSAFMethod > 0) {
                SizingMethod = HeatingAirflowSizing;
                state.dataSize->ZoneHeatingOnlyFan = true;
                SAFMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingSAFMethod;
                ZoneEqSizing.SizingMethod(SizingMethod) = SAFMethod;
                if (SAFMethod == SupplyAirFlowRate || SAFMethod == FlowPerFloorArea || SAFMethod == FractionOfAutosizedHeatingAirflow) {
                    if (SAFMethod == SupplyAirFlowRate) {
                        if ((state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow == AutoSize) &&
                            ((PurchAir.HeatingLimit == LimitType::FlowRate) || (PurchAir.HeatingLimit == LimitType::FlowRateAndCapacity))) {
                            TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                            HeatingAirFlowSizer sizingHeatingAirFlow;
                            sizingHeatingAirFlow.overrideSizingString(SizingString);
                            // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                            sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                            HeatingAirVolFlowDes = sizingHeatingAirFlow.size(state, TempSize, ErrorsFound);
                        } else {
                            if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow > 0.0) {
                                HeatingAirFlowSizer sizingHeatingAirFlow;
                                sizingHeatingAirFlow.overrideSizingString(SizingString);
                                // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                                sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                HeatingAirVolFlowDes =
                                    sizingHeatingAirFlow.size(state, state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow, ErrorsFound);
                            }
                        }
                    } else if (SAFMethod == FlowPerFloorArea) {
                        ZoneEqSizing.SystemAirFlow = true;
                        ZoneEqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow *
                                                  state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                        TempSize = ZoneEqSizing.AirVolFlow;
                        state.dataSize->DataScalableSizingON = true;
                        HeatingAirFlowSizer sizingHeatingAirFlow;
                        sizingHeatingAirFlow.overrideSizingString(SizingString);
                        // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                        sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        HeatingAirVolFlowDes = sizingHeatingAirFlow.size(state, TempSize, ErrorsFound);
                    } else if (SAFMethod == FractionOfAutosizedHeatingAirflow) {
                        state.dataSize->DataFracOfAutosizedHeatingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                        if ((state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow == AutoSize) &&
                            ((PurchAir.HeatingLimit == LimitType::FlowRate) || (PurchAir.HeatingLimit == LimitType::FlowRateAndCapacity))) {
                            TempSize = AutoSize;
                            state.dataSize->DataScalableSizingON = true;
                            HeatingAirFlowSizer sizingHeatingAirFlow;
                            sizingHeatingAirFlow.overrideSizingString(SizingString);
                            // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                            sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                            HeatingAirVolFlowDes = sizingHeatingAirFlow.size(state, TempSize, ErrorsFound);
                        }

                    } else {
                        // Invalid sizing method
                    }
                } else if (SAFMethod == FlowPerHeatingCapacity) {
                    SizingMethod = HeatingCapacitySizing;
                    TempSize = AutoSize;
                    PrintFlag = false;
                    if ((state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow == AutoSize) &&
                        ((PurchAir.HeatingLimit == LimitType::FlowRate) || (PurchAir.HeatingLimit == LimitType::FlowRateAndCapacity))) {
                        TempSize = AutoSize;
                        state.dataSize->DataScalableSizingON = true;
                        HeatingCapacitySizer sizerHeatingCapacity;
                        sizerHeatingCapacity.overrideSizingString(SizingString);
                        sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        state.dataSize->DataAutosizedHeatingCapacity = sizerHeatingCapacity.size(state, TempSize, ErrorsFound);
                        state.dataSize->DataFlowPerHeatingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                        SizingMethod = HeatingAirflowSizing;
                        PrintFlag = true;
                        TempSize = AutoSize;
                        HeatingAirFlowSizer sizingHeatingAirFlow;
                        sizingHeatingAirFlow.overrideSizingString(SizingString);
                        // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                        sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        HeatingAirVolFlowDes = sizingHeatingAirFlow.size(state, TempSize, ErrorsFound);
                    }
                }
                MaxHeatVolFlowRateDes = max(0.0, HeatingAirVolFlowDes);
                PurchAir.MaxHeatVolFlowRate = MaxHeatVolFlowRateDes;
                state.dataSize->ZoneHeatingOnlyFan = false;

                CapSizingMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingCapMethod;
                ZoneEqSizing.CapSizingMethod = CapSizingMethod;
                if (CapSizingMethod == HeatingDesignCapacity || CapSizingMethod == CapacityPerFloorArea ||
                    CapSizingMethod == FractionOfAutosizedHeatingCapacity) {
                    if (CapSizingMethod == HeatingDesignCapacity) {
                        if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity > 0.0) {
                            ZoneEqSizing.HeatingCapacity = true;
                            ZoneEqSizing.DesHeatingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                        }
                        TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                    } else if (CapSizingMethod == CapacityPerFloorArea) {
                        ZoneEqSizing.HeatingCapacity = true;
                        ZoneEqSizing.DesHeatingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity *
                                                      state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                        state.dataSize->DataScalableSizingON = true;
                    } else if (CapSizingMethod == FractionOfAutosizedHeatingCapacity) {
                        state.dataSize->DataFracOfAutosizedHeatingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                        TempSize = AutoSize;
                    }
                }
                SizingMethod = HeatingCapacitySizing;
                SizingString = "";
                state.dataSize->ZoneHeatingOnlyFan = true;
                PrintFlag = false;
                HeatingCapacitySizer sizerHeatingCapacity;
                sizerHeatingCapacity.overrideSizingString(SizingString);
                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                MaxHeatSensCapDes = sizerHeatingCapacity.size(state, TempSize, ErrorsFound);
                state.dataSize->ZoneHeatingOnlyFan = false;
                if (MaxHeatSensCapDes < HVAC::SmallLoad) {
                    MaxHeatSensCapDes = 0.0;
                }
                if (PurchAir.MaxHeatSensCap > 0.0 && MaxHeatSensCapDes > 0.0) {
                    MaxHeatSensCapUser = PurchAir.MaxHeatSensCap;
                    BaseSizer::reportSizerOutput(state,
                                                 PurchAir.cObjectName,
                                                 PurchAir.Name,
                                                 "Design Size Maximum Sensible Heating Capacity [W]",
                                                 MaxHeatSensCapDes,
                                                 "User-Specified Maximum Sensible Heating Capacity [W]",
                                                 MaxHeatSensCapUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(MaxHeatSensCapDes - MaxHeatSensCapUser) / MaxHeatSensCapUser) > state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(
                                state,
                                format("SizePurchasedAir: Potential issue with equipment sizing for {} {}", PurchAir.cObjectName, PurchAir.Name));
                            ShowContinueError(state, format("...User-Specified Maximum Sensible Heating Capacity of {:.2R} [W]", MaxHeatSensCapUser));
                            ShowContinueError(
                                state, format("...differs from Design Size Maximum Sensible Heating Capacity of {:.2R} [W]", MaxHeatSensCapDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }

            PrintFlag = true;
            if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingSAFMethod > 0) {
                state.dataSize->ZoneCoolingOnlyFan = true;
                SAFMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingSAFMethod;
                ZoneEqSizing.SizingMethod(SizingMethod) = SAFMethod;
                if (SAFMethod == SupplyAirFlowRate || SAFMethod == FlowPerFloorArea || SAFMethod == FractionOfAutosizedCoolingAirflow) {
                    if (SAFMethod == SupplyAirFlowRate) {
                        if ((state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow == AutoSize) &&
                            ((PurchAir.CoolingLimit == LimitType::FlowRate) || (PurchAir.CoolingLimit == LimitType::FlowRateAndCapacity) ||
                             (PurchAir.OutdoorAir && PurchAir.EconomizerType != Econ::NoEconomizer))) {
                            TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                            CoolingAirFlowSizer sizingCoolingAirFlow;
                            sizingCoolingAirFlow.overrideSizingString(SizingString);
                            sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                            CoolingAirVolFlowDes = sizingCoolingAirFlow.size(state, TempSize, ErrorsFound);
                        } else {
                            if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow > 0.0) {
                                CoolingAirVolFlowDes = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                                CoolingAirFlowSizer sizingCoolingAirFlow;
                                sizingCoolingAirFlow.overrideSizingString(SizingString);
                                sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                CoolingAirVolFlowDes = sizingCoolingAirFlow.size(state, CoolingAirVolFlowDes, ErrorsFound);
                            }
                        }
                    } else if (SAFMethod == FlowPerFloorArea) {
                        ZoneEqSizing.SystemAirFlow = true;
                        ZoneEqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow *
                                                  state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                        TempSize = ZoneEqSizing.AirVolFlow;
                        state.dataSize->DataScalableSizingON = true;
                        CoolingAirFlowSizer sizingCoolingAirFlow;
                        std::string stringOverride = "Maximum Cooling Air Flow Rate [m3/s]";
                        if (state.dataGlobal->isEpJSON) {
                            stringOverride = "maximum_cooling_air_flow_rate [m3/s]";
                        }
                        sizingCoolingAirFlow.overrideSizingString(stringOverride);
                        // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                        sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        CoolingAirVolFlowDes = sizingCoolingAirFlow.size(state, TempSize, ErrorsFound);
                    } else if (SAFMethod == FractionOfAutosizedCoolingAirflow) {
                        if ((state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow == AutoSize) &&
                            ((PurchAir.CoolingLimit == LimitType::FlowRate) || (PurchAir.CoolingLimit == LimitType::FlowRateAndCapacity) ||
                             (PurchAir.OutdoorAir && PurchAir.EconomizerType != Econ::NoEconomizer))) {
                            state.dataSize->DataFracOfAutosizedCoolingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                            TempSize = AutoSize;
                            state.dataSize->DataScalableSizingON = true;
                            CoolingAirFlowSizer sizingCoolingAirFlow;
                            std::string stringOverride = "Maximum Cooling Air Flow Rate [m3/s]";
                            if (state.dataGlobal->isEpJSON) {
                                stringOverride = "maximum_cooling_air_flow_rate [m3/s]";
                            }
                            sizingCoolingAirFlow.overrideSizingString(stringOverride);
                            // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                            sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                            CoolingAirVolFlowDes = sizingCoolingAirFlow.size(state, TempSize, ErrorsFound);
                        }
                    } else {
                        // Invalid scalable sizing method
                    }
                } else if (SAFMethod == FlowPerCoolingCapacity) {
                    if ((state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow == AutoSize) &&
                        ((PurchAir.CoolingLimit == LimitType::FlowRate) || (PurchAir.CoolingLimit == LimitType::FlowRateAndCapacity) ||
                         (PurchAir.OutdoorAir && PurchAir.EconomizerType != Econ::NoEconomizer))) {
                        SizingMethod = CoolingCapacitySizing;
                        TempSize = AutoSize;
                        PrintFlag = false;
                        CoolingCapacitySizer sizerCoolingCapacity;
                        sizerCoolingCapacity.overrideSizingString(SizingString);
                        sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        state.dataSize->DataAutosizedCoolingCapacity = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                        state.dataSize->DataFlowPerCoolingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                        PrintFlag = true;
                        TempSize = AutoSize;
                        state.dataSize->DataScalableSizingON = true;
                        CoolingAirFlowSizer sizingCoolingAirFlow;
                        std::string stringOverride = "Maximum Cooling Air Flow Rate [m3/s]";
                        if (state.dataGlobal->isEpJSON) {
                            stringOverride = "maximum_cooling_air_flow_rate [m3/s]";
                        }
                        sizingCoolingAirFlow.overrideSizingString(stringOverride);
                        // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                        sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        CoolingAirVolFlowDes = sizingCoolingAirFlow.size(state, TempSize, ErrorsFound);
                    }
                }
                MaxCoolVolFlowRateDes = max(0.0, CoolingAirVolFlowDes);
                PurchAir.MaxCoolVolFlowRate = MaxCoolVolFlowRateDes;
                state.dataSize->ZoneCoolingOnlyFan = false;
                state.dataSize->DataScalableSizingON = false;

                CapSizingMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingCapMethod;
                ZoneEqSizing.CapSizingMethod = CapSizingMethod;
                if (CapSizingMethod == CoolingDesignCapacity || CapSizingMethod == CapacityPerFloorArea ||
                    CapSizingMethod == FractionOfAutosizedCoolingCapacity) {
                    if (CapSizingMethod == CoolingDesignCapacity) {
                        if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity > 0.0) {
                            ZoneEqSizing.CoolingCapacity = true;
                            ZoneEqSizing.DesCoolingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
                        } else {
                            state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolMassFlow;
                        }
                        TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
                    } else if (CapSizingMethod == CapacityPerFloorArea) {
                        ZoneEqSizing.CoolingCapacity = true;
                        ZoneEqSizing.DesCoolingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity *
                                                      state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                        state.dataSize->DataScalableSizingON = true;
                    } else if (CapSizingMethod == FractionOfAutosizedCoolingCapacity) {
                        state.dataSize->DataFracOfAutosizedHeatingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
                        state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolMassFlow;
                        TempSize = AutoSize;
                    }
                }
                // SizingMethod = CoolingCapacitySizing;
                SizingString = "";
                state.dataSize->ZoneCoolingOnlyFan = true;
                PrintFlag = false;
                TempSize = PurchAir.MaxCoolTotCap;
                CoolingCapacitySizer sizerCoolingCapacity;
                sizerCoolingCapacity.overrideSizingString(SizingString);
                sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                MaxCoolTotCapDes = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                state.dataSize->ZoneCoolingOnlyFan = false;
                if (MaxCoolTotCapDes < HVAC::SmallLoad) {
                    MaxCoolTotCapDes = 0.0;
                }
                if (PurchAir.MaxCoolTotCap > 0.0 && MaxCoolTotCapDes > 0.0) {
                    MaxCoolTotCapUser = PurchAir.MaxCoolTotCap;
                    BaseSizer::reportSizerOutput(state,
                                                 PurchAir.cObjectName,
                                                 PurchAir.Name,
                                                 "Design Size Maximum Total Cooling Capacity [W]",
                                                 MaxCoolTotCapDes,
                                                 "User-Specified Maximum Total Cooling Capacity [W]",
                                                 MaxCoolTotCapUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(MaxCoolTotCapDes - MaxCoolTotCapUser) / MaxCoolTotCapUser) > state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(
                                state,
                                format("SizePurchasedAir: Potential issue with equipment sizing for {} {}", PurchAir.cObjectName, PurchAir.Name));
                            ShowContinueError(state, format("User-Specified Maximum Total Cooling Capacity of {:.2R} [W]", MaxCoolTotCapUser));
                            ShowContinueError(state,
                                              format("differs from Design Size Maximum Total Cooling Capacity of {:.2R} [W]", MaxCoolTotCapDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }

        } else {
            // SizingString = "Maximum Heating Air Flow Rate [m3/s]";
            // SizingMethod = HeatingAirflowSizing;
            FieldNum = 5;
            SizingString = state.dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(FieldNum) + " [m3/s]";
            bool IsAutoSize = false;
            PrintFlag = true;
            if ((PurchAir.MaxHeatVolFlowRate == AutoSize) &&
                ((PurchAir.HeatingLimit == LimitType::FlowRate) || (PurchAir.HeatingLimit == LimitType::FlowRateAndCapacity))) {
                IsAutoSize = true;
            }
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
                if (PurchAir.MaxHeatVolFlowRate > 0.0) {
                    HeatingAirFlowSizer sizingHeatingAirFlow;
                    sizingHeatingAirFlow.overrideSizingString(SizingString);
                    // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                    sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    PurchAir.MaxHeatVolFlowRate = sizingHeatingAirFlow.size(state, PurchAir.MaxHeatVolFlowRate, ErrorsFound);
                }
                MaxHeatVolFlowRateDes = 0.0;
            } else {
                state.dataSize->ZoneHeatingOnlyFan = true;
                TempSize = PurchAir.MaxHeatVolFlowRate;
                HeatingAirFlowSizer sizingHeatingAirFlow;
                sizingHeatingAirFlow.overrideSizingString(SizingString);
                // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                MaxHeatVolFlowRateDes = sizingHeatingAirFlow.size(state, PurchAir.MaxHeatVolFlowRate, ErrorsFound);
                PurchAir.MaxHeatVolFlowRate = MaxHeatVolFlowRateDes;
                state.dataSize->ZoneHeatingOnlyFan = false;
            }

            IsAutoSize = false;
            // SizingMethod = HeatingCapacitySizing;
            FieldNum = 6; // N6, \field Maximum Sensible Heating Capacity
            SizingString = state.dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(FieldNum) + " [m3/s]";
            if ((PurchAir.MaxHeatSensCap == AutoSize) &&
                ((PurchAir.HeatingLimit == LimitType::Capacity) || (PurchAir.HeatingLimit == LimitType::FlowRateAndCapacity))) {
                IsAutoSize = true;
            }
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
                if (PurchAir.MaxHeatSensCap > 0.0) {
                    HeatingCapacitySizer sizerHeatingCapacity;
                    sizerHeatingCapacity.overrideSizingString(SizingString);
                    sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    MaxHeatSensCapDes = sizerHeatingCapacity.size(state, PurchAir.MaxHeatSensCap, ErrorsFound);
                }
            } else {
                TempSize = PurchAir.MaxHeatSensCap;
                ZoneEqSizing.OAVolFlow = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA;
                state.dataSize->ZoneHeatingOnlyFan = true;
                PrintFlag = false;
                HeatingCapacitySizer sizerHeatingCapacity;
                sizerHeatingCapacity.overrideSizingString(SizingString);
                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                MaxHeatSensCapDes = sizerHeatingCapacity.size(state, TempSize, ErrorsFound);
                state.dataSize->ZoneHeatingOnlyFan = false;
            }
            if (MaxHeatSensCapDes < HVAC::SmallLoad) {
                MaxHeatSensCapDes = 0.0;
            }
            if (IsAutoSize) {
                PurchAir.MaxHeatSensCap = MaxHeatSensCapDes;
                BaseSizer::reportSizerOutput(
                    state, PurchAir.cObjectName, PurchAir.Name, "Design Size Maximum Sensible Heating Capacity [W]", MaxHeatSensCapDes);
                // If there is OA, check if sizing calcs have OA>0, throw warning if not
                if ((PurchAir.OutdoorAir) && (state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA == 0.0)) {
                    ShowWarningError(state, format("InitPurchasedAir: In {} = {}", PurchAir.cObjectName, PurchAir.Name));
                    ShowContinueError(state, "There is outdoor air specified in this object, but the design outdoor air flow rate for this ");
                    ShowContinueError(state, "zone is zero. The Maximum Sensible Heating Capacity will be autosized for zero outdoor air flow. ");
                    ShowContinueError(state,
                                      format("Check the outdoor air specifications in the Sizing:Zone object for zone {}.",
                                             state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).ZoneName));
                }
            } else {
                if (PurchAir.MaxHeatSensCap > 0.0 && MaxHeatSensCapDes > 0.0) {
                    MaxHeatSensCapUser = PurchAir.MaxHeatSensCap;
                    BaseSizer::reportSizerOutput(state,
                                                 PurchAir.cObjectName,
                                                 PurchAir.Name,
                                                 "Design Size Maximum Sensible Heating Capacity [W]",
                                                 MaxHeatSensCapDes,
                                                 "User-Specified Maximum Sensible Heating Capacity [W]",
                                                 MaxHeatSensCapUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(MaxHeatSensCapDes - MaxHeatSensCapUser) / MaxHeatSensCapUser) > state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(
                                state,
                                format("SizePurchasedAir: Potential issue with equipment sizing for {} {}", PurchAir.cObjectName, PurchAir.Name));
                            ShowContinueError(state, format("...User-Specified Maximum Sensible Heating Capacity of {:.2R} [W]", MaxHeatSensCapUser));
                            ShowContinueError(
                                state, format("...differs from Design Size Maximum Sensible Heating Capacity of {:.2R} [W]", MaxHeatSensCapDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }

            PrintFlag = true;
            IsAutoSize = false;
            if ((PurchAir.MaxCoolVolFlowRate == AutoSize) &&
                ((PurchAir.CoolingLimit == LimitType::FlowRate) || (PurchAir.CoolingLimit == LimitType::FlowRateAndCapacity) ||
                 (PurchAir.OutdoorAir && PurchAir.EconomizerType != Econ::NoEconomizer))) {
                IsAutoSize = true;
            }
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
                if (PurchAir.MaxCoolVolFlowRate > 0.0) {
                    CoolingAirFlowSizer sizingCoolingAirFlow;
                    std::string stringOverride = "Maximum Cooling Air Flow Rate [m3/s]";
                    if (state.dataGlobal->isEpJSON) {
                        stringOverride = "maximum_cooling_air_flow_rate [m3/s]";
                    }
                    sizingCoolingAirFlow.overrideSizingString(stringOverride);
                    // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                    sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    PurchAir.MaxCoolVolFlowRate = sizingCoolingAirFlow.size(state, PurchAir.MaxCoolVolFlowRate, ErrorsFound);
                }
            } else {
                state.dataSize->ZoneCoolingOnlyFan = true;
                TempSize = PurchAir.MaxCoolVolFlowRate;
                CoolingAirFlowSizer sizingCoolingAirFlow;
                std::string stringOverride = "Maximum Cooling Air Flow Rate [m3/s]";
                if (state.dataGlobal->isEpJSON) {
                    stringOverride = "maximum_cooling_air_flow_rate [m3/s]";
                }
                sizingCoolingAirFlow.overrideSizingString(stringOverride);
                // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                MaxCoolVolFlowRateDes = sizingCoolingAirFlow.size(state, TempSize, ErrorsFound);
                PurchAir.MaxCoolVolFlowRate = MaxCoolVolFlowRateDes;
                state.dataSize->ZoneCoolingOnlyFan = false;
            }

            IsAutoSize = false;
            //  SizingMethod = CoolingCapacitySizing;
            FieldNum = 8; // N8, \field Maximum Total Cooling Capacity
            SizingString = state.dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(FieldNum) + " [m3/s]";
            if ((PurchAir.MaxCoolTotCap == AutoSize) &&
                ((PurchAir.CoolingLimit == LimitType::Capacity) || (PurchAir.CoolingLimit == LimitType::FlowRateAndCapacity))) {
                IsAutoSize = true;
            }
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
                if (PurchAir.MaxCoolTotCap > 0.0) {
                    CoolingCapacitySizer sizerCoolingCapacity;
                    sizerCoolingCapacity.overrideSizingString(SizingString);
                    sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    PurchAir.MaxCoolTotCap = sizerCoolingCapacity.size(state, PurchAir.MaxCoolTotCap, ErrorsFound);
                }
            } else {
                state.dataSize->ZoneCoolingOnlyFan = true;
                ZoneEqSizing.OAVolFlow = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA;
                PrintFlag = false;
                TempSize = PurchAir.MaxCoolTotCap;
                CoolingCapacitySizer sizerCoolingCapacity;
                sizerCoolingCapacity.overrideSizingString(SizingString);
                sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                MaxCoolTotCapDes = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                state.dataSize->ZoneCoolingOnlyFan = false;
            }
            if (MaxCoolTotCapDes < HVAC::SmallLoad) {
                MaxCoolTotCapDes = 0.0;
            }
            if (IsAutoSize) {
                PurchAir.MaxCoolTotCap = MaxCoolTotCapDes;
                BaseSizer::reportSizerOutput(
                    state, PurchAir.cObjectName, PurchAir.Name, "Design Size Maximum Total Cooling Capacity [W]", MaxCoolTotCapDes);
                // If there is OA, check if sizing calcs have OA>0, throw warning if not
                if ((PurchAir.OutdoorAir) && (state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA == 0.0)) {
                    ShowWarningError(state, format("SizePurchasedAir: In {} = {}", PurchAir.cObjectName, PurchAir.Name));
                    ShowContinueError(state, "There is outdoor air specified in this object, but the design outdoor air flow rate for this ");
                    ShowContinueError(state, "zone is zero. The Maximum Total Cooling Capacity will be autosized for zero outdoor air flow. ");
                    ShowContinueError(state,
                                      format("Check the outdoor air specifications in the Sizing:Zone object for zone {}.",
                                             state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).ZoneName));
                }
            } else {
                if (PurchAir.MaxCoolTotCap > 0.0 && MaxCoolTotCapDes > 0.0) {
                    MaxCoolTotCapUser = PurchAir.MaxCoolTotCap;
                    BaseSizer::reportSizerOutput(state,
                                                 PurchAir.cObjectName,
                                                 PurchAir.Name,
                                                 "Design Size Maximum Total Cooling Capacity [W]",
                                                 MaxCoolTotCapDes,
                                                 "User-Specified Maximum Total Cooling Capacity [W]",
                                                 MaxCoolTotCapUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(MaxCoolTotCapDes - MaxCoolTotCapUser) / MaxCoolTotCapUser) > state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(
                                state,
                                format("SizePurchasedAir: Potential issue with equipment sizing for {} {}", PurchAir.cObjectName, PurchAir.Name));
                            ShowContinueError(state, format("User-Specified Maximum Total Cooling Capacity of {:.2R} [W]", MaxCoolTotCapUser));
                            ShowContinueError(state,
                                              format("differs from Design Size Maximum Total Cooling Capacity of {:.2R} [W]", MaxCoolTotCapDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }
        }
    }
}

void CalcPurchAirLoads(EnergyPlusData &state,
                       int const PurchAirNum,
                       Real64 &SysOutputProvided,   // Sensible output provided [W] cooling = negative
                       Real64 &MoistOutputProvided, // Moisture output provided [kg/s] dehumidification = negative
                       int const ControlledZoneNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   Nov 1997
    //       MODIFIED       Shirey, Aug 2009 (LatOutputProvided - now MoistOutputProvided)
    //                      M. Witte June 2011, add new features including DCV, economizer, dehumidification
    //                          and humidification,
    //                      July 2012, Chandan Sharma - FSEC: Added hybrid ventilation manager
    //       RE-ENGINEERED  na

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("CalcPurchAirLoads");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int InNodeNum; // Ideal loads supply node to zone
    //         INTEGER   :: ExhNodeNum        ! Ideal loads exhaust node from zone
    int ZoneNodeNum;                   // Zone air node
    int OANodeNum;                     // Outdoor air inlet node
    int RecircNodeNum;                 // Return air or zone exhaust node
    OpMode OperatingMode;              // current operating mode, Off, Heat, Cool, or DeadBand
    Real64 SupplyMassFlowRate;         // System supply air mass flow rate [kg/s]
    Real64 SupplyMassFlowRateForHumid; // System supply air mass flow rate required to meet humidification load [kg/s]
    Real64 SupplyMassFlowRateForDehum; // System supply air mass flow rate required to meet dehumidification load [kg/s]
    Real64 SupplyMassFlowRateForCool;  // System supply air mass flow rate required to meet sensible cooling load[kg/s]
    Real64 SupplyMassFlowRateForHeat;  // System supply air mass flow rate required to meet sensible heating load[kg/s]
    Real64 SupplyHumRatForHumid;       // Supply air humidity ratio require to meet the humidification load [kgWater/kgDryAir]
    Real64 SupplyHumRatForDehum;       // Supply air humidity ratio require to meet the dehumidification load [kgWater/kgDryAir]
    Real64 OAMassFlowRate;             // Outdoor air mass flow rate [kg/s]
    Real64 OAVolFlowRate;              // Outdoor air volume flow rate at standard density [m3/s]
    Real64 MinOASensOutput;            // Minimum Outdoor air sensible output [W], <0 means OA is cooler than zone air
    Real64 MinOALatOutput;             // Minimum Outdoor air moisture load [kg/s]
    Real64 SensOutput;                 // Sensible output [W] (positive means heating, negative means cooling)
    Real64 HeatSensOutput;             // Heating sensible output [W]
    Real64 CoolSensOutput;             // Cooling sensible output [W] (positive value means cooling)
    Real64 LatOutput;                  // Latent output [W] (positive value means humidification, negative means dehumidification)
    Real64 CoolLatOutput;              // Cooling latent output [W] (positive value means dehumidification)
    Real64 CoolTotOutput;              // Cooling total output [W] (positive value means cooling)
    Real64 DeltaT;                     // Delta temperature - reused in multiple places
    Real64 DeltaHumRat;                // Delta humidity ratio - reused in multiple places
    Real64 QZnHeatSP;                  // Load required to meet heating setpoint [W] (>0 is a heating load)
    Real64 QZnCoolSP;                  // Load required to meet cooling setpoint [W] (<0 is a cooling load)
    Real64 MdotZnHumidSP;              // Load required to meet humidifying setpoint [kgWater/s] (>0 = a humidify load)
    Real64 MdotZnDehumidSP;            // Load required to meet dehumidifying setpoint [kgWater/s] (<0 = a dehumidify load)
    bool UnitOn;
    bool HeatOn;             // Flag for heating and humidification availability schedule, true if heating is on
    bool CoolOn;             // Flag for cooling and dehumidification availability schedule, true if cooling is on
    bool EconoOn;            // Flag for economizer operation, true if economizer is on
    Real64 SupplyHumRatOrig; // Supply inlet to zone humidity ratio before saturation check [kgWater/kgDryAir]
    Real64 SupplyHumRatSat;  // Supply inlet to zone humidity ratio saturation at SupplyTemp [kgWater/kgDryAir]
    Real64 SupplyEnthalpy;   // Supply inlet to zone enthalpy [J/kg]
    Real64 MixedAirEnthalpy; // Mixed air enthalpy [J/kg]
    Real64 CpAir;            // Specific heat [J/kg-C] reused in multiple places
    //         REAL(r64) :: SpecHumOut   ! Specific humidity ratio of outlet air (kg moisture / kg moist air)
    //         REAL(r64) :: SpecHumIn    ! Specific humidity ratio of inlet [zone] air (kg moisture / kg moist air)

    auto &PurchAir = state.dataPurchasedAirMgr->PurchAir(PurchAirNum);

    // Sign convention: SysOutputProvided <0 Supply air is heated on entering zone (zone is cooled)
    //                  SysOutputProvided >0 Supply air is cooled on entering zone (zone is heated)
    InNodeNum = PurchAir.ZoneSupplyAirNodeNum;
    ZoneNodeNum = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;
    OANodeNum = PurchAir.OutdoorAirNodeNum;
    RecircNodeNum = PurchAir.ZoneRecircAirNodeNum;
    SupplyMassFlowRate = 0.0;
    OAMassFlowRate = 0.0;
    PurchAir.MinOAMassFlowRate = 0.0;
    PurchAir.TimeEconoActive = 0.0;
    PurchAir.TimeHtRecActive = 0.0;
    SysOutputProvided = 0.0;
    MoistOutputProvided = 0.0;
    CoolSensOutput = 0.0;
    CoolLatOutput = 0.0;
    CoolTotOutput = 0.0;
    HeatSensOutput = 0.0;
    LatOutput = 0.0;

    // default unit to ON
    UnitOn = true;
    EconoOn = false;
    // get current zone requirements
    QZnHeatSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToHeatSP;
    QZnCoolSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToCoolSP;

    if (allocated(state.dataAvail->ZoneComp)) {
        auto &availMgr = state.dataAvail->ZoneComp(DataZoneEquipment::ZoneEquipType::PurchasedAir).ZoneCompAvailMgrs(PurchAirNum);
        availMgr.ZoneNum = ControlledZoneNum;
        PurchAir.availStatus = availMgr.availStatus;
        // Check if the hybrid ventilation availability manager is turning the unit off
        if (PurchAir.availStatus == Avail::Status::ForceOff) {
            UnitOn = false;
        }
    }

    // Check if the unit is scheduled off
    if (PurchAir.availSched->getCurrentVal() <= 0) {
        UnitOn = false;
    }
    //         END IF
    // Check if heating and cooling available
    HeatOn = true;
    if (PurchAir.heatAvailSched->getCurrentVal() <= 0) {
        HeatOn = false;
    }

    CoolOn = true;
    if (PurchAir.coolAvailSched->getCurrentVal() <= 0) {
        CoolOn = false;
    }
    //         END IF

    if (UnitOn) {
        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ControlledZoneNum);
        // Calculate current minimum outdoor air flow rate based on design OA specifications and DCV or CO2 control
        CalcPurchAirMinOAMassFlow(state, PurchAirNum, ControlledZoneNum, OAMassFlowRate);

        // EMS override point  Purch air outdoor air massflow rate.....
        if (PurchAir.EMSOverrideOAMdotOn) {
            OAMassFlowRate = PurchAir.EMSValueOAMassFlowRate;
        }

        // Calculate minimum outdoor air sensible and latent load
        if (PurchAir.OutdoorAir) {
            CpAir = PsyCpAirFnW(state.dataLoopNodes->Node(OANodeNum).HumRat);
            MinOASensOutput = OAMassFlowRate * CpAir * (state.dataLoopNodes->Node(OANodeNum).Temp - state.dataLoopNodes->Node(ZoneNodeNum).Temp);
            MinOALatOutput = OAMassFlowRate * (state.dataLoopNodes->Node(OANodeNum).HumRat - state.dataLoopNodes->Node(ZoneNodeNum).HumRat);
        } else {
            MinOASensOutput = 0.0;
            MinOALatOutput = 0.0;
        }
        // SupplyMassFlowRate = OAMassFlowRate;

        // Check if cooling of the supply air stream is required

        // Cooling operation
        if ((MinOASensOutput >= QZnCoolSP) && (state.dataHeatBalFanSys->TempControlType(ControlledZoneNum) != HVAC::SetptType::SingleHeat)) {
            OperatingMode = OpMode::Cool;
            // Calculate supply mass flow, temp and humidity with the following constraints:
            //  Min cooling supply temp
            //  Max total cooling capacity
            //  Max cooling airflow
            //  Min cooling supply humrat  (and Max heating supply humrat)
            //  Min OA mass flow rate

            // Check if OA flow rate greater than max cooling airflow limit
            if (((PurchAir.CoolingLimit == LimitType::FlowRate) || (PurchAir.CoolingLimit == LimitType::FlowRateAndCapacity)) &&
                (OAMassFlowRate > PurchAir.MaxCoolMassFlowRate)) {
                OAVolFlowRate = OAMassFlowRate / state.dataEnvrn->StdRhoAir;
                if (PurchAir.OAFlowMaxCoolOutputError < 1) {
                    ++PurchAir.OAFlowMaxCoolOutputError;
                    ShowWarningError(state,
                                     format("{} \"{}\" Requested outdoor air flow rate = {:.5T} [m3/s] exceeds limit.",
                                            PurchAir.cObjectName,
                                            PurchAir.Name,
                                            OAVolFlowRate));
                    ShowContinueError(state,
                                      format(" Will be reduced to the Maximum Cooling Air Flow Rate = {:.5T} [m3/s]", PurchAir.MaxCoolVolFlowRate));
                    ShowContinueErrorTimeStamp(state, "");
                } else {
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        PurchAir.cObjectName + " \"" + PurchAir.Name +
                            "\" Requested outdoor air flow rate [m3/s] reduced to Maximum Cooling Air Flow Rate warning continues...",
                        PurchAir.OAFlowMaxCoolOutputIndex,
                        OAVolFlowRate);
                }
                OAMassFlowRate = PurchAir.MaxCoolMassFlowRate;

            } else {
                // Model economizer
                if (PurchAir.EconomizerType != Econ::NoEconomizer) {
                    if (((PurchAir.EconomizerType == Econ::DifferentialDryBulb) &&
                         (state.dataLoopNodes->Node(OANodeNum).Temp < state.dataLoopNodes->Node(PurchAir.ZoneRecircAirNodeNum).Temp)) ||
                        ((PurchAir.EconomizerType == Econ::DifferentialEnthalpy) &&
                         (state.dataLoopNodes->Node(OANodeNum).Enthalpy < state.dataLoopNodes->Node(PurchAir.ZoneRecircAirNodeNum).Enthalpy))) {

                        // Calculate supply MassFlowRate based on sensible load but limit to Max Cooling Supply Air Flow Rate if specified
                        CpAir = PsyCpAirFnW(thisZoneHB.airHumRat);
                        DeltaT = (state.dataLoopNodes->Node(OANodeNum).Temp - state.dataLoopNodes->Node(ZoneNodeNum).Temp);
                        if (DeltaT < -HVAC::SmallTempDiff) {
                            SupplyMassFlowRate = QZnCoolSP / CpAir / DeltaT;
                            if (((PurchAir.CoolingLimit == LimitType::FlowRate) || (PurchAir.CoolingLimit == LimitType::FlowRateAndCapacity)) &&
                                (PurchAir.MaxCoolMassFlowRate > 0.0)) {
                                SupplyMassFlowRate = min(max(SupplyMassFlowRate, 0.0), PurchAir.MaxCoolMassFlowRate);
                            }
                            if (SupplyMassFlowRate > OAMassFlowRate) {
                                EconoOn = true;
                                OAMassFlowRate = SupplyMassFlowRate;
                                PurchAir.TimeEconoActive = state.dataHVACGlobal->TimeStepSys;
                            }
                        }
                    }
                }
            }

            // Determine supply mass flow rate
            // Mass flow rate to meet sensible load, at Minimum Cooling Supply Air Temperature
            SupplyMassFlowRateForCool = 0.0;
            if (CoolOn) {
                CpAir = PsyCpAirFnW(thisZoneHB.airHumRat);
                DeltaT = (PurchAir.MinCoolSuppAirTemp - state.dataLoopNodes->Node(ZoneNodeNum).Temp);
                if (DeltaT < -HVAC::SmallTempDiff) {
                    SupplyMassFlowRateForCool = QZnCoolSP / CpAir / DeltaT;
                }
            }

            // Mass flow rate to meet dehumidification load, if applicable, at Minimum Cooling Supply Humidity Ratio
            SupplyMassFlowRateForDehum = 0.0;
            if (CoolOn) {
                if (PurchAir.DehumidCtrlType == HumControl::Humidistat) {
                    MdotZnDehumidSP = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlledZoneNum).RemainingOutputReqToDehumidSP;
                    DeltaHumRat = (PurchAir.MinCoolSuppAirHumRat - state.dataLoopNodes->Node(ZoneNodeNum).HumRat);
                    if ((DeltaHumRat < -SmallDeltaHumRat) && (MdotZnDehumidSP < 0.0)) {
                        SupplyMassFlowRateForDehum = MdotZnDehumidSP / DeltaHumRat;
                    }
                }
            }

            // Mass flow rate to meet humidification load, if applicable, at Maximum Heating Supply Humidity Ratio
            // This section is the cooling section, so humidification should activate only if humidification control = humidistat
            //   and if dehumidification control = humidistat or none
            SupplyMassFlowRateForHumid = 0.0;
            if (HeatOn) {
                if (PurchAir.HumidCtrlType == HumControl::Humidistat) {
                    if ((PurchAir.DehumidCtrlType == HumControl::Humidistat) || (PurchAir.DehumidCtrlType == HumControl::None)) {
                        MdotZnHumidSP = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlledZoneNum).RemainingOutputReqToHumidSP;
                        DeltaHumRat = (PurchAir.MaxHeatSuppAirHumRat - state.dataLoopNodes->Node(ZoneNodeNum).HumRat);
                        if ((DeltaHumRat > SmallDeltaHumRat) && (MdotZnHumidSP > 0.0)) {
                            SupplyMassFlowRateForHumid = MdotZnHumidSP / DeltaHumRat;
                        }
                    }
                }
            }

            // If cooling capacity is limited to zero, SupplyMassFlowRate* should be set to zero
            if (((PurchAir.CoolingLimit == LimitType::Capacity) || (PurchAir.CoolingLimit == LimitType::FlowRateAndCapacity)) &&
                (PurchAir.MaxCoolTotCap == 0)) {
                SupplyMassFlowRateForCool = 0;
                SupplyMassFlowRateForDehum = 0;
                SupplyMassFlowRateForHumid = 0;
            }

            // Supply mass flow is greatest of these, but limit to cooling max flow rate, if applicable
            SupplyMassFlowRate = max(0.0, OAMassFlowRate, SupplyMassFlowRateForCool, SupplyMassFlowRateForDehum, SupplyMassFlowRateForHumid);
            // EMS override point  Purch air massflow rate..... but only if unit is on, i.e. SupplyMassFlowRate>0.0
            if (PurchAir.EMSOverrideMdotOn) {
                SupplyMassFlowRate = PurchAir.EMSValueMassFlowRate;
                OAMassFlowRate = min(OAMassFlowRate, SupplyMassFlowRate);
            }
            if (((PurchAir.CoolingLimit == LimitType::FlowRate) || (PurchAir.CoolingLimit == LimitType::FlowRateAndCapacity)) &&
                (PurchAir.MaxCoolMassFlowRate > 0.0)) {
                SupplyMassFlowRate = min(SupplyMassFlowRate, PurchAir.MaxCoolMassFlowRate);
            }

            if (SupplyMassFlowRate <= HVAC::VerySmallMassFlow) {
                SupplyMassFlowRate = 0.0;
            }

            // Calculate mixed air conditions
            CalcPurchAirMixedAir(state,
                                 PurchAirNum,
                                 OAMassFlowRate,
                                 SupplyMassFlowRate,
                                 PurchAir.MixedAirTemp,
                                 PurchAir.MixedAirHumRat,
                                 MixedAirEnthalpy,
                                 OperatingMode);

            // Calculate supply air conditions using final massflow rate, imposing capacity limits if specified
            // If capacity limits are exceeded, keep massflow rate where it is and adjust supply temp
            // In general, in the cooling section, don't let SupplyTemp be set to something that results in heating
            if (SupplyMassFlowRate > 0.0) {
                // Calculate supply temp at SupplyMassFlowRate and recheck limit on Minimum Cooling Supply Air Temperature
                CpAir = PsyCpAirFnW(thisZoneHB.airHumRat);
                PurchAir.SupplyTemp = QZnCoolSP / (CpAir * SupplyMassFlowRate) + state.dataLoopNodes->Node(ZoneNodeNum).Temp;
                PurchAir.SupplyTemp = max(PurchAir.SupplyTemp, PurchAir.MinCoolSuppAirTemp);
                // This is the cooling mode, so SupplyTemp can't be more than MixedAirTemp
                PurchAir.SupplyTemp = min(PurchAir.SupplyTemp, PurchAir.MixedAirTemp);
                PurchAir.SupplyHumRat = PurchAir.MixedAirHumRat;
                SupplyEnthalpy = PsyHFnTdbW(PurchAir.SupplyTemp, PurchAir.SupplyHumRat);

                // Check sensible load vs max total cooling capacity, if specified, and adjust supply temp before applying humidity controls
                // Will check again later, too
                if ((PurchAir.CoolingLimit == LimitType::Capacity) || (PurchAir.CoolingLimit == LimitType::FlowRateAndCapacity)) {
                    CpAir = PsyCpAirFnW(PurchAir.MixedAirHumRat);
                    CoolSensOutput = SupplyMassFlowRate * (MixedAirEnthalpy - SupplyEnthalpy);
                    if (CoolSensOutput >= PurchAir.MaxCoolTotCap) {
                        CoolSensOutput = PurchAir.MaxCoolTotCap;
                        SupplyEnthalpy = MixedAirEnthalpy - CoolSensOutput / SupplyMassFlowRate;
                        PurchAir.SupplyTemp = PsyTdbFnHW(SupplyEnthalpy, PurchAir.SupplyHumRat);
                        // This is the cooling mode, so SupplyTemp can't be more than MixedAirTemp
                        PurchAir.SupplyTemp = min(PurchAir.SupplyTemp, PurchAir.MixedAirTemp);
                    } // Capacity limit exceeded
                }

                // Set supply humidity ratio for cooling/dehumidification
                PurchAir.SupplyHumRat = PurchAir.MixedAirHumRat;
                switch (PurchAir.DehumidCtrlType) {
                case HumControl::None: {
                    PurchAir.SupplyHumRat = PurchAir.MixedAirHumRat; // Unnecessary line?
                } break;
                case HumControl::ConstantSensibleHeatRatio: {
                    // SHR = CoolSensOutput/CoolTotOutput
                    // CoolTotOutput = CoolSensOutput/SHR
                    CpAir = PsyCpAirFnW(PurchAir.MixedAirHumRat);
                    CoolSensOutput = SupplyMassFlowRate * CpAir * (PurchAir.MixedAirTemp - PurchAir.SupplyTemp);
                    CoolTotOutput = CoolSensOutput / PurchAir.CoolSHR;
                    SupplyEnthalpy = MixedAirEnthalpy - CoolTotOutput / SupplyMassFlowRate;
                    //  Limit for overdrying (avoid Pysch errors which occur if SupplyEnthalpy is too low for SupplyTemp)
                    SupplyEnthalpy = max(SupplyEnthalpy, PsyHFnTdbW(PurchAir.SupplyTemp, 0.00001));
                    PurchAir.SupplyHumRat = min(PurchAir.SupplyHumRat, PsyWFnTdbH(state, PurchAir.SupplyTemp, SupplyEnthalpy, RoutineName));
                    // Apply min cooling humidity ratio limit
                    PurchAir.SupplyHumRat = max(PurchAir.SupplyHumRat, PurchAir.MinCoolSuppAirHumRat);
                    // But don't let it be higher than incoming MixedAirHumRat
                    PurchAir.SupplyHumRat = min(PurchAir.SupplyHumRat, PurchAir.MixedAirHumRat);
                } break;
                case HumControl::Humidistat: {
                    MdotZnDehumidSP = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlledZoneNum).RemainingOutputReqToDehumidSP;
                    SupplyHumRatForDehum = MdotZnDehumidSP / SupplyMassFlowRate + state.dataLoopNodes->Node(ZoneNodeNum).HumRat;
                    SupplyHumRatForDehum = max(SupplyHumRatForDehum, PurchAir.MinCoolSuppAirHumRat);
                    PurchAir.SupplyHumRat = min(PurchAir.MixedAirHumRat, SupplyHumRatForDehum);
                } break;
                case HumControl::ConstantSupplyHumidityRatio: {
                    PurchAir.SupplyHumRat = PurchAir.MinCoolSuppAirHumRat;
                } break;
                default: {
                    PurchAir.SupplyHumRat = PurchAir.MixedAirHumRat;
                } break;
                }

                // Check supply humidity ratio for humidification (SupplyHumRatForHum should always be < SupplyHumRatForDehum)
                // This section is the cooling section, so humidification should activate only if humidification control = humidistat
                //   and if dehumidification control = humidistat or none
                if (HeatOn) {
                    if (PurchAir.HumidCtrlType == HumControl::Humidistat) {
                        if ((PurchAir.DehumidCtrlType == HumControl::Humidistat) || (PurchAir.DehumidCtrlType == HumControl::None)) {
                            MdotZnHumidSP = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlledZoneNum).RemainingOutputReqToHumidSP;
                            SupplyHumRatForHumid = MdotZnHumidSP / SupplyMassFlowRate + state.dataLoopNodes->Node(ZoneNodeNum).HumRat;
                            SupplyHumRatForHumid = min(SupplyHumRatForHumid, PurchAir.MaxHeatSuppAirHumRat);
                            PurchAir.SupplyHumRat = max(PurchAir.SupplyHumRat, SupplyHumRatForHumid);
                        }
                    }
                }

                //   Limit supply humidity ratio to saturation at supply outlet temp

                SupplyHumRatOrig = PurchAir.SupplyHumRat;
                SupplyHumRatSat = PsyWFnTdbRhPb(state, PurchAir.SupplyTemp, 1.0, state.dataEnvrn->OutBaroPress, RoutineName);
                PurchAir.SupplyHumRat = min(SupplyHumRatOrig, SupplyHumRatSat);
                SupplyEnthalpy = PsyHFnTdbW(PurchAir.SupplyTemp, PurchAir.SupplyHumRat);

                // Check max total Cooling capacity, if specified
                if ((PurchAir.CoolingLimit == LimitType::Capacity) || (PurchAir.CoolingLimit == LimitType::FlowRateAndCapacity)) {
                    // If dehumidifying, compare total cooling to the limit
                    if (PurchAir.SupplyHumRat < PurchAir.MixedAirHumRat) { // Dehumidifying
                        CoolTotOutput = SupplyMassFlowRate * (MixedAirEnthalpy - SupplyEnthalpy);
                        if ((CoolTotOutput) > PurchAir.MaxCoolTotCap) {
                            CoolTotOutput = PurchAir.MaxCoolTotCap;
                            SupplyEnthalpy = MixedAirEnthalpy - CoolTotOutput / SupplyMassFlowRate;
                            // Adjust output based on dehumidification control type
                            switch (PurchAir.DehumidCtrlType) {
                            case HumControl::ConstantSensibleHeatRatio: {
                                // Adjust both supply temp and humidity ratio to maintain SHR
                                // SHR = CoolSensOutput/CoolTotOutput
                                // CoolSensOutput = SHR*CoolTotOutput
                                CpAir = PsyCpAirFnW(PurchAir.MixedAirHumRat);
                                CoolSensOutput = CoolTotOutput * PurchAir.CoolSHR;
                                PurchAir.SupplyTemp = PurchAir.MixedAirTemp - CoolSensOutput / (CpAir * SupplyMassFlowRate);
                                // This is the cooling mode, so SupplyTemp can't be more than MixedAirTemp
                                PurchAir.SupplyTemp = min(PurchAir.SupplyTemp, PurchAir.MixedAirTemp);
                                //  Limit for overdrying (avoid Pysch errors which occur if SupplyEnthalpy is too low for SupplyTemp)
                                SupplyEnthalpy = max(SupplyEnthalpy, PsyHFnTdbW(PurchAir.SupplyTemp, 0.00001));
                                PurchAir.SupplyHumRat = PsyWFnTdbH(state, PurchAir.SupplyTemp, SupplyEnthalpy, RoutineName);
                            } break;
                            case HumControl::Humidistat: {
                                // Keep supply temp and adjust humidity ratio to reduce load
                                PurchAir.SupplyHumRat = PsyWFnTdbH(state, PurchAir.SupplyTemp, SupplyEnthalpy, RoutineName);
                            } break;
                            case HumControl::None:
                            case HumControl::ConstantSupplyHumidityRatio: {
                                // Keep humidity ratio and adjust supply temp
                                // Check if latent output exceeds capacity
                                CpAir = PsyCpAirFnW(PurchAir.MixedAirHumRat);
                                CoolSensOutput = SupplyMassFlowRate * CpAir * (PurchAir.MixedAirTemp - PurchAir.SupplyTemp);
                                CoolLatOutput = CoolTotOutput - CoolSensOutput;
                                if (CoolLatOutput >= PurchAir.MaxCoolTotCap) {
                                    PurchAir.SupplyTemp = PurchAir.MixedAirTemp;
                                    PurchAir.SupplyHumRat = PsyWFnTdbH(state, PurchAir.SupplyTemp, SupplyEnthalpy, RoutineName);
                                    CoolLatOutput = PurchAir.MaxCoolTotCap;
                                } else {
                                    PurchAir.SupplyTemp = PsyTdbFnHW(SupplyEnthalpy, PurchAir.SupplyHumRat);
                                    // This is the cooling mode, so SupplyTemp can't be more than MixedAirTemp
                                    PurchAir.SupplyTemp = min(PurchAir.SupplyTemp, PurchAir.MixedAirTemp);
                                }
                            } break;
                            default:
                                break;
                            }
                            // Limit supply humidity ratio to saturation at supply outlet temp
                            // If saturation exceeded, then honor capacity limit and set to dew point at supply enthalpy

                            SupplyHumRatOrig = PurchAir.SupplyHumRat;
                            SupplyHumRatSat = PsyWFnTdbRhPb(state, PurchAir.SupplyTemp, 1.0, state.dataEnvrn->OutBaroPress, RoutineName);
                            if (SupplyHumRatSat < SupplyHumRatOrig) {
                                PurchAir.SupplyTemp = PsyTsatFnHPb(state, SupplyEnthalpy, state.dataEnvrn->OutBaroPress, RoutineName);

                                // This is the cooling mode, so SupplyTemp can't be more than MixedAirTemp
                                PurchAir.SupplyTemp = min(PurchAir.SupplyTemp, PurchAir.MixedAirTemp);
                                PurchAir.SupplyHumRat = PsyWFnTdbH(state, PurchAir.SupplyTemp, SupplyEnthalpy, RoutineName);
                                SupplyEnthalpy = PsyHFnTdbW(PurchAir.SupplyTemp, PurchAir.SupplyHumRat);
                                // CpAir = PsyCpAirFnW(MixedAirHumRat)
                                // CoolSensOutput = SupplyMassFlowRate * CpAir * (MixedAirTemp - SupplyTemp)
                                // CoolTotOutput = SupplyMassFlowRate * (MixedAirEnthalpy - SupplyEnthalpy)
                            }
                        } // Capacity limit exceeded
                    } else { // Not dehumidifying
                        // If not dehumidifying, compare sensible cooling to the limit
                        // This section will only increase supply temp, so no need to recheck for super-saturation
                        CpAir = PsyCpAirFnW(PurchAir.MixedAirHumRat);
                        CoolSensOutput = SupplyMassFlowRate * CpAir * (PurchAir.MixedAirTemp - PurchAir.SupplyTemp);
                        if (CoolSensOutput >= PurchAir.MaxCoolTotCap) {
                            CoolSensOutput = PurchAir.MaxCoolTotCap;
                            PurchAir.SupplyTemp = PurchAir.MixedAirTemp - CoolSensOutput / (SupplyMassFlowRate * CpAir);
                        } // Capacity limit exceeded
                    } // Dehumidifying or not
                } // Capacity limit active

            } else { // SupplyMassFlowRate is zero
                SupplyEnthalpy = MixedAirEnthalpy;
                PurchAir.SupplyHumRat = PurchAir.MixedAirHumRat;
                PurchAir.SupplyTemp = PurchAir.MixedAirTemp;
                CoolSensOutput = 0.0;
                CoolTotOutput = 0.0;
            }
            // Heating or no-load operation
        } else { // Heating or no-load case
            if ((MinOASensOutput < QZnHeatSP) && (state.dataHeatBalFanSys->TempControlType(ControlledZoneNum) != HVAC::SetptType::SingleCool)) {
                OperatingMode = OpMode::Heat;
            } else { // DeadBand mode shuts off heat recovery and economizer
                OperatingMode = OpMode::DeadBand;
            }
            // Calculate supply mass flow, temp and humidity with the following constraints:
            //  Max heating supply temp
            //  Max sensible heating capacity
            //  Max heating airflow
            //  Max heating supply humrat (and Min cooling supply humrat)
            //  Min OA mass flow rate

            // Check if OA flow rate greater than max heating airflow limit
            if (((PurchAir.HeatingLimit == LimitType::FlowRate) || (PurchAir.HeatingLimit == LimitType::FlowRateAndCapacity)) &&
                (OAMassFlowRate > PurchAir.MaxHeatMassFlowRate)) {
                OAVolFlowRate = OAMassFlowRate / state.dataEnvrn->StdRhoAir;
                if (PurchAir.OAFlowMaxHeatOutputError < 1) {
                    ++PurchAir.OAFlowMaxHeatOutputError;
                    ShowWarningError(state,
                                     format("{} \"{}\" Requested outdoor air flow rate = {:.5T} [m3/s] exceeds limit.",
                                            PurchAir.cObjectName,
                                            PurchAir.Name,
                                            OAVolFlowRate));
                    ShowContinueError(state,
                                      format(" Will be reduced to the Maximum Heating Air Flow Rate = {:.5T} [m3/s]", PurchAir.MaxHeatVolFlowRate));
                    ShowContinueErrorTimeStamp(state, "");
                } else {
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        PurchAir.cObjectName + " \"" + PurchAir.Name +
                            "\" Requested outdoor air flow rate [m3/s] reduced to Maximum Heating Air Flow Rate warning continues...",
                        PurchAir.OAFlowMaxHeatOutputIndex,
                        OAVolFlowRate);
                }
                OAMassFlowRate = PurchAir.MaxHeatMassFlowRate;
            }

            // SupplyMassFlowRate = OAMassFlowRate;

            // Determine supply mass flow rate
            // Mass flow rate to meet sensible load, at Minimum Cooling Supply Air Temperature
            SupplyMassFlowRateForHeat = 0.0;
            if ((HeatOn) && (OperatingMode == OpMode::Heat)) {
                CpAir = PsyCpAirFnW(thisZoneHB.airHumRat);
                DeltaT = (PurchAir.MaxHeatSuppAirTemp - state.dataLoopNodes->Node(ZoneNodeNum).Temp);
                if (DeltaT > HVAC::SmallTempDiff) {
                    SupplyMassFlowRateForHeat = QZnHeatSP / CpAir / DeltaT;
                }
            }

            // Mass flow rate to meet dehumidification load, if applicable, at Minimum Cooling Supply Humidity Ratio
            // This section is the heating/deadband section, so dehumidification should activate
            //   only if dehumidification control = humidistat
            //   and if humidification control = humidistat or none or if operating in deadband mode
            SupplyMassFlowRateForDehum = 0.0;
            if (CoolOn) {
                if (PurchAir.DehumidCtrlType == HumControl::Humidistat) {
                    if ((PurchAir.HumidCtrlType == HumControl::Humidistat) || (PurchAir.HumidCtrlType == HumControl::None) ||
                        (OperatingMode == OpMode::DeadBand)) {
                        MdotZnDehumidSP = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlledZoneNum).RemainingOutputReqToDehumidSP;
                        DeltaHumRat = (PurchAir.MinCoolSuppAirHumRat - state.dataLoopNodes->Node(ZoneNodeNum).HumRat);
                        if ((DeltaHumRat < -SmallDeltaHumRat) && (MdotZnDehumidSP < 0.0)) {
                            SupplyMassFlowRateForDehum = MdotZnDehumidSP / DeltaHumRat;
                        }
                    }
                }
            }

            // Mass flow rate to meet humidification load, if applicable, at Maximum Heating Supply Humidity Ratio
            SupplyMassFlowRateForHumid = 0.0;
            if (HeatOn) {
                if (PurchAir.HumidCtrlType == HumControl::Humidistat) {
                    MdotZnHumidSP = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlledZoneNum).RemainingOutputReqToHumidSP;
                    DeltaHumRat = (PurchAir.MaxHeatSuppAirHumRat - state.dataLoopNodes->Node(ZoneNodeNum).HumRat);
                    if ((DeltaHumRat > SmallDeltaHumRat) && (MdotZnHumidSP > 0.0)) {
                        SupplyMassFlowRateForHumid = MdotZnHumidSP / DeltaHumRat;
                    }
                }
            }

            // If heating capacity is limited to zero, SupplyMassFlowRate* should be set to zero
            if (((PurchAir.HeatingLimit == LimitType::Capacity) || (PurchAir.HeatingLimit == LimitType::FlowRateAndCapacity)) &&
                (PurchAir.MaxHeatSensCap == 0)) {
                SupplyMassFlowRateForHeat = 0;
                SupplyMassFlowRateForDehum = 0;
                SupplyMassFlowRateForHumid = 0;
            }

            // Supply mass flow is greatest of these, but limit to heating max flow rate, if applicable
            SupplyMassFlowRate = max(0.0, OAMassFlowRate, SupplyMassFlowRateForHeat, SupplyMassFlowRateForDehum, SupplyMassFlowRateForHumid);
            // EMS override point  Purch air massflow rate..... but only if unit is on, i.e. SupplyMassFlowRate>0.0
            if (PurchAir.EMSOverrideMdotOn) {
                SupplyMassFlowRate = PurchAir.EMSValueMassFlowRate;
                OAMassFlowRate = min(OAMassFlowRate, SupplyMassFlowRate);
            }
            if (((PurchAir.HeatingLimit == LimitType::FlowRate) || (PurchAir.HeatingLimit == LimitType::FlowRateAndCapacity)) &&
                (PurchAir.MaxHeatMassFlowRate > 0.0)) {
                SupplyMassFlowRate = min(SupplyMassFlowRate, PurchAir.MaxHeatMassFlowRate);
            }

            if (SupplyMassFlowRate <= HVAC::VerySmallMassFlow) {
                SupplyMassFlowRate = 0.0;
            }

            // Calculate mixed air conditions
            CalcPurchAirMixedAir(state,
                                 PurchAirNum,
                                 OAMassFlowRate,
                                 SupplyMassFlowRate,
                                 PurchAir.MixedAirTemp,
                                 PurchAir.MixedAirHumRat,
                                 MixedAirEnthalpy,
                                 OperatingMode);

            // Calculate supply air conditions using final massflow rate, imposing capacity limits if specified
            // If capacity limits are exceeded, keep massflow rate where it is and adjust supply temp
            if (SupplyMassFlowRate > 0.0) {
                if ((HeatOn) && (OperatingMode == OpMode::Heat)) {
                    // Calculate supply temp at SupplyMassFlowRate and check limit on Maximum Heating Supply Air Temperature
                    CpAir = PsyCpAirFnW(thisZoneHB.airHumRat);
                    PurchAir.SupplyTemp = QZnHeatSP / (CpAir * SupplyMassFlowRate) + state.dataLoopNodes->Node(ZoneNodeNum).Temp;
                    PurchAir.SupplyTemp = min(PurchAir.SupplyTemp, PurchAir.MaxHeatSuppAirTemp);
                    // This is the heating mode, so SupplyTemp can't be less than MixedAirTemp
                    PurchAir.SupplyTemp = max(PurchAir.SupplyTemp, PurchAir.MixedAirTemp);
                    // Check max heating capacity, if specified
                    if ((PurchAir.HeatingLimit == LimitType::Capacity) || (PurchAir.HeatingLimit == LimitType::FlowRateAndCapacity)) {
                        CpAir = PsyCpAirFnW(PurchAir.MixedAirHumRat);
                        HeatSensOutput = SupplyMassFlowRate * CpAir * (PurchAir.SupplyTemp - PurchAir.MixedAirTemp);
                        if (HeatSensOutput > PurchAir.MaxHeatSensCap) {
                            PurchAir.SupplyTemp = PurchAir.MaxHeatSensCap / (SupplyMassFlowRate * CpAir) + PurchAir.MixedAirTemp;
                            HeatSensOutput = PurchAir.MaxHeatSensCap;
                        }
                    }
                } else { // Heat is off or operating mode is deadband (i.e. don't do any heating)
                    PurchAir.SupplyTemp = PurchAir.MixedAirTemp;
                }

                // Set supply humidity ratio first for heating/humidification
                PurchAir.SupplyHumRat = PurchAir.MixedAirHumRat;
                switch (PurchAir.HumidCtrlType) {
                case HumControl::None: {
                    PurchAir.SupplyHumRat = PurchAir.MixedAirHumRat;
                } break;
                case HumControl::Humidistat: {
                    MdotZnHumidSP = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlledZoneNum).RemainingOutputReqToHumidSP;
                    SupplyHumRatForHumid = MdotZnHumidSP / SupplyMassFlowRate + state.dataLoopNodes->Node(ZoneNodeNum).HumRat;
                    SupplyHumRatForHumid = min(SupplyHumRatForHumid, PurchAir.MaxHeatSuppAirHumRat);
                    PurchAir.SupplyHumRat = max(PurchAir.SupplyHumRat, SupplyHumRatForHumid);
                } break;
                case HumControl::ConstantSupplyHumidityRatio: {
                    if (OperatingMode == OpMode::Heat) {
                        // If this results in dehumidification, must check cooling capacity limit
                        if (PurchAir.MixedAirHumRat > PurchAir.MaxHeatSuppAirHumRat) {
                            if ((PurchAir.CoolingLimit == LimitType::Capacity) || (PurchAir.CoolingLimit == LimitType::FlowRateAndCapacity)) {
                                PurchAir.SupplyHumRat = PurchAir.MaxHeatSuppAirHumRat;
                                SupplyEnthalpy = PsyHFnTdbW(PurchAir.SupplyTemp, PurchAir.SupplyHumRat);
                                CoolTotOutput = SupplyMassFlowRate * (MixedAirEnthalpy - SupplyEnthalpy);
                                CpAir = PsyCpAirFnW(PurchAir.MixedAirHumRat);
                                CoolSensOutput = SupplyMassFlowRate * CpAir * (PurchAir.MixedAirTemp - PurchAir.SupplyTemp);
                                CoolLatOutput = CoolTotOutput - CoolSensOutput;
                                if (CoolLatOutput >= PurchAir.MaxCoolTotCap) {
                                    CoolLatOutput = PurchAir.MaxCoolTotCap;
                                    CoolTotOutput = CoolSensOutput + CoolLatOutput;
                                    SupplyEnthalpy = MixedAirEnthalpy - CoolTotOutput / SupplyMassFlowRate;
                                    PurchAir.SupplyHumRat = PsyWFnTdbH(state, PurchAir.SupplyTemp, SupplyEnthalpy, RoutineName);
                                }
                            } else {
                                PurchAir.SupplyHumRat = PurchAir.MaxHeatSuppAirHumRat;
                            }
                        } else {
                            PurchAir.SupplyHumRat = PurchAir.MaxHeatSuppAirHumRat;
                        }
                    } else {
                        PurchAir.SupplyHumRat = PurchAir.MixedAirHumRat;
                    }
                } break;
                default: {
                    PurchAir.SupplyHumRat = PurchAir.MixedAirHumRat;
                } break;
                }
                // SupplyEnthalpy = PsyHFnTdbW(PurchAir.SupplyTemp, PurchAir.SupplyHumRat);

                // Check supply humidity ratio for dehumidification (SupplyHumRatForHumid should always be < SupplyHumRatForDehum)
                // This section is the heating/deadband section, so dehumidification should activate
                //   only if dehumidification control = humidistat
                //   and if humidification control = humidistat or none or if operating in deadband mode
                if (CoolOn) {
                    if (PurchAir.DehumidCtrlType == HumControl::Humidistat) {
                        if ((PurchAir.HumidCtrlType == HumControl::Humidistat) || (PurchAir.HumidCtrlType == HumControl::None) ||
                            (OperatingMode == OpMode::DeadBand)) {
                            MdotZnDehumidSP = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlledZoneNum).RemainingOutputReqToDehumidSP;
                            SupplyHumRatForDehum = MdotZnDehumidSP / SupplyMassFlowRate + state.dataLoopNodes->Node(ZoneNodeNum).HumRat;
                            SupplyHumRatForDehum = max(SupplyHumRatForDehum, PurchAir.MinCoolSuppAirHumRat);
                            PurchAir.SupplyHumRat = min(PurchAir.SupplyHumRat, SupplyHumRatForDehum);
                            SupplyEnthalpy = PsyHFnTdbW(PurchAir.SupplyTemp, PurchAir.SupplyHumRat);
                            if (PurchAir.SupplyHumRat < PurchAir.MixedAirHumRat) {
                                // At this point, the system is heating or deadband but dehumidifying, check max cooling cap limit
                                CpAir = PsyCpAirFnW(PurchAir.MixedAirHumRat);
                                SensOutput = SupplyMassFlowRate * CpAir * (PurchAir.SupplyTemp - PurchAir.MixedAirTemp);
                                LatOutput = SupplyMassFlowRate * (SupplyEnthalpy - MixedAirEnthalpy) - SensOutput;
                                if ((PurchAir.CoolingLimit == LimitType::Capacity) || (PurchAir.CoolingLimit == LimitType::FlowRateAndCapacity)) {
                                    if (LatOutput > PurchAir.MaxCoolTotCap) {
                                        LatOutput = PurchAir.MaxCoolTotCap;
                                        SupplyEnthalpy = MixedAirEnthalpy + (LatOutput + SensOutput) / SupplyMassFlowRate;
                                        PurchAir.SupplyHumRat = PsyWFnTdbH(state, PurchAir.SupplyTemp, SupplyEnthalpy, RoutineName);
                                    }
                                }
                            }
                        }
                    }
                }

                //   Limit supply humidity ratio to saturation at supply outlet temp

                SupplyHumRatOrig = PurchAir.SupplyHumRat;
                PurchAir.SupplyHumRat =
                    min(PurchAir.SupplyHumRat, PsyWFnTdbRhPb(state, PurchAir.SupplyTemp, 1.0, state.dataEnvrn->OutBaroPress, RoutineName));
                SupplyEnthalpy = PsyHFnTdbW(PurchAir.SupplyTemp, PurchAir.SupplyHumRat);

            } else { // SupplyMassFlowRate is zero
                SupplyEnthalpy = MixedAirEnthalpy;
                PurchAir.SupplyHumRat = PurchAir.MixedAirHumRat;
                PurchAir.SupplyTemp = PurchAir.MixedAirTemp;
                HeatSensOutput = 0.0;
            }

        } // Cooling or heating required

        if (SupplyMassFlowRate > 0.0) {
            // EMS override point  Purch air supply temp and humidity ratio ..... but only if unit is on, SupplyMassFlowRate>0.0
            if (PurchAir.EMSOverrideSupplyTempOn) {
                PurchAir.SupplyTemp = PurchAir.EMSValueSupplyTemp;
            }
            if (PurchAir.EMSOverrideSupplyHumRatOn) {
                PurchAir.SupplyHumRat = PurchAir.EMSValueSupplyHumRat;
            }
            SupplyEnthalpy = PsyHFnTdbW(PurchAir.SupplyTemp, PurchAir.SupplyHumRat);

            // compute coil loads
            if ((PurchAir.SupplyHumRat == PurchAir.MixedAirHumRat) && (PurchAir.SupplyTemp == PurchAir.MixedAirTemp)) {
                // If no change in humrat or temp, then set loads to zero
                PurchAir.SenCoilLoad = 0.0;
                PurchAir.LatCoilLoad = 0.0;
            } else if ((PurchAir.SupplyHumRat == PurchAir.MixedAirHumRat) && (PurchAir.SupplyTemp != PurchAir.MixedAirTemp)) {
                // If no change in humrat, then set latent load to zero and use enthalpies to calculate sensible load
                PurchAir.SenCoilLoad = SupplyMassFlowRate * (SupplyEnthalpy - MixedAirEnthalpy);
                PurchAir.LatCoilLoad = 0.0;
            } else {
                CpAir = PsyCpAirFnW(PurchAir.MixedAirHumRat);
                PurchAir.SenCoilLoad = SupplyMassFlowRate * CpAir * (PurchAir.SupplyTemp - PurchAir.MixedAirTemp);
                PurchAir.LatCoilLoad = SupplyMassFlowRate * (SupplyEnthalpy - MixedAirEnthalpy) - PurchAir.SenCoilLoad;
            }

            // Apply heating and cooling availability schedules to sensible load
            if (((PurchAir.SenCoilLoad > 0.0) && !HeatOn) || ((PurchAir.SenCoilLoad < 0.0) && !CoolOn)) {
                // Coil is off
                PurchAir.SenCoilLoad = 0.0;
                PurchAir.SupplyTemp = PurchAir.MixedAirTemp;
            }

            // Apply heating and cooling availability schedules to latent load
            if (((PurchAir.LatCoilLoad > 0.0) && !HeatOn) || ((PurchAir.LatCoilLoad < 0.0) && !CoolOn)) {
                // Coil is off
                PurchAir.LatCoilLoad = 0.0;
                PurchAir.SupplyHumRat = PurchAir.MixedAirHumRat;
            }

            // Double-check if saturation exceeded, then throw warning, shouldn't happen here, don't reset, just warn

            SupplyHumRatOrig = PurchAir.SupplyHumRat;
            SupplyHumRatSat = PsyWFnTdbRhPb(state, PurchAir.SupplyTemp, 1.0, state.dataEnvrn->OutBaroPress, RoutineName);

            DeltaHumRat = SupplyHumRatOrig - SupplyHumRatSat;
            if (DeltaHumRat > SmallDeltaHumRat) {
                if (PurchAir.SaturationOutputError < 1) {
                    ++PurchAir.SaturationOutputError;
                    ShowWarningError(state,
                                     format("{} \"{}\" Supply humidity ratio = {:.5T} exceeds saturation limit {:.5T} [kgWater/kgDryAir]",
                                            PurchAir.cObjectName,
                                            PurchAir.Name,
                                            SupplyHumRatOrig,
                                            SupplyHumRatSat));
                    ShowContinueError(state, " Simulation continuing . . . ");
                    ShowContinueErrorTimeStamp(state, "");
                } else {
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        PurchAir.cObjectName + " \"" + PurchAir.Name +
                            "\" Supply humidity ratio exceeds saturation limit warning continues, delta max/min [kgWater/kgDryAir]...",
                        PurchAir.SaturationOutputIndex,
                        DeltaHumRat,
                        DeltaHumRat);
                }
            }

            SupplyEnthalpy = PsyHFnTdbW(PurchAir.SupplyTemp, PurchAir.SupplyHumRat);

            CpAir = PsyCpAirFnW(thisZoneHB.airHumRat);
            SysOutputProvided = SupplyMassFlowRate * CpAir * (PurchAir.SupplyTemp - state.dataLoopNodes->Node(ZoneNodeNum).Temp);
            MoistOutputProvided = SupplyMassFlowRate * (PurchAir.SupplyHumRat - state.dataLoopNodes->Node(ZoneNodeNum).HumRat); // Latent rate, kg/s

            PurchAir.SenOutputToZone = SysOutputProvided;
            PurchAir.LatOutputToZone =
                SupplyMassFlowRate * (SupplyEnthalpy - state.dataLoopNodes->Node(ZoneNodeNum).Enthalpy) - PurchAir.SenOutputToZone;

            CpAir = PsyCpAirFnW(thisZoneHB.airHumRat);
            if (PurchAir.OutdoorAir) {
                PurchAir.OASenOutput =
                    OAMassFlowRate * CpAir * (state.dataLoopNodes->Node(OANodeNum).Temp - state.dataLoopNodes->Node(ZoneNodeNum).Temp);
                PurchAir.OALatOutput =
                    OAMassFlowRate * (state.dataLoopNodes->Node(OANodeNum).Enthalpy - state.dataLoopNodes->Node(ZoneNodeNum).Enthalpy) -
                    PurchAir.OASenOutput;
            } else {
                PurchAir.OASenOutput = 0.0;
                PurchAir.OALatOutput = 0.0;
            }
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                if (PurchAir.OutdoorAir) {
                    state.dataLoopNodes->Node(InNodeNum).CO2 = ((SupplyMassFlowRate - OAMassFlowRate) * state.dataLoopNodes->Node(RecircNodeNum).CO2 +
                                                                OAMassFlowRate * state.dataLoopNodes->Node(OANodeNum).CO2) /
                                                               SupplyMassFlowRate;
                } else {
                    state.dataLoopNodes->Node(InNodeNum).CO2 = state.dataLoopNodes->Node(RecircNodeNum).CO2;
                }
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                if (PurchAir.OutdoorAir) {
                    state.dataLoopNodes->Node(InNodeNum).GenContam =
                        ((SupplyMassFlowRate - OAMassFlowRate) * state.dataLoopNodes->Node(RecircNodeNum).GenContam +
                         OAMassFlowRate * state.dataLoopNodes->Node(OANodeNum).GenContam) /
                        SupplyMassFlowRate;
                } else {
                    state.dataLoopNodes->Node(InNodeNum).GenContam = state.dataLoopNodes->Node(RecircNodeNum).GenContam;
                }
            }
        } else { // SupplyMassFlowRate = 0.0
            SysOutputProvided = 0.0;
            MoistOutputProvided = 0.0;

            PurchAir.SenOutputToZone = 0.0;
            PurchAir.LatOutputToZone = 0.0;
            PurchAir.SenCoilLoad = 0.0;
            PurchAir.LatCoilLoad = 0.0;
            PurchAir.OASenOutput = 0.0;
            PurchAir.OALatOutput = 0.0;

            PurchAir.MixedAirTemp = state.dataLoopNodes->Node(RecircNodeNum).Temp;
            PurchAir.MixedAirHumRat = state.dataLoopNodes->Node(RecircNodeNum).HumRat;
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {

                state.dataLoopNodes->Node(InNodeNum).CO2 = state.dataLoopNodes->Node(ZoneNodeNum).CO2;
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                state.dataLoopNodes->Node(InNodeNum).GenContam = state.dataLoopNodes->Node(ZoneNodeNum).GenContam;
            }
        }

        state.dataLoopNodes->Node(InNodeNum).Temp = PurchAir.SupplyTemp;
        state.dataLoopNodes->Node(InNodeNum).HumRat = PurchAir.SupplyHumRat;
        state.dataLoopNodes->Node(InNodeNum).Enthalpy = SupplyEnthalpy;
        state.dataLoopNodes->Node(InNodeNum).MassFlowRate = SupplyMassFlowRate;
        if (PurchAir.OutdoorAir) {
            state.dataLoopNodes->Node(OANodeNum).MassFlowRate = OAMassFlowRate;
        }

    } else { // purchased air OFF

        SysOutputProvided = 0.0;
        MoistOutputProvided = 0.0;
        SupplyMassFlowRate = 0.0;
        OAMassFlowRate = 0.0;
        state.dataLoopNodes->Node(InNodeNum).Temp = state.dataLoopNodes->Node(ZoneNodeNum).Temp;
        state.dataLoopNodes->Node(InNodeNum).HumRat = state.dataLoopNodes->Node(ZoneNodeNum).HumRat;
        state.dataLoopNodes->Node(InNodeNum).Enthalpy = state.dataLoopNodes->Node(ZoneNodeNum).Enthalpy;
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            state.dataLoopNodes->Node(InNodeNum).CO2 = state.dataLoopNodes->Node(ZoneNodeNum).CO2;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            state.dataLoopNodes->Node(InNodeNum).GenContam = state.dataLoopNodes->Node(ZoneNodeNum).GenContam;
        }

        state.dataLoopNodes->Node(InNodeNum).MassFlowRate = 0.0;
        if (PurchAir.OutdoorAir) {
            state.dataLoopNodes->Node(OANodeNum).MassFlowRate = 0.0;
        }
        PurchAir.SenHeatRate = 0.0;
        PurchAir.SenCoolRate = 0.0;
        PurchAir.TotCoolRate = 0.0;

        PurchAir.SenOutputToZone = 0.0;
        PurchAir.LatOutputToZone = 0.0;
        PurchAir.SenCoilLoad = 0.0;
        PurchAir.LatCoilLoad = 0.0;
        PurchAir.OASenOutput = 0.0;
        PurchAir.OALatOutput = 0.0;
        PurchAir.MixedAirTemp = state.dataLoopNodes->Node(RecircNodeNum).Temp;
        PurchAir.MixedAirHumRat = state.dataLoopNodes->Node(RecircNodeNum).HumRat;
        PurchAir.SupplyTemp = state.dataLoopNodes->Node(InNodeNum).Temp;
        PurchAir.SupplyHumRat = state.dataLoopNodes->Node(InNodeNum).HumRat;
    }

    PurchAir.OutdoorAirMassFlowRate = OAMassFlowRate;
    PurchAir.OutdoorAirVolFlowRateStdRho = OAMassFlowRate / state.dataEnvrn->StdRhoAir;
    PurchAir.SupplyAirMassFlowRate = SupplyMassFlowRate;

    PurchAir.SupplyAirVolFlowRateStdRho = SupplyMassFlowRate / state.dataEnvrn->StdRhoAir;

    if (PurchAir.PlenumExhaustAirNodeNum > 0) {
        state.dataLoopNodes->Node(PurchAir.PlenumExhaustAirNodeNum).MassFlowRate = SupplyMassFlowRate;
    }
    state.dataLoopNodes->Node(RecircNodeNum).MassFlowRate = SupplyMassFlowRate;
}

void CalcPurchAirMinOAMassFlow(EnergyPlusData &state,
                               int const PurchAirNum, // index to ideal loads unit
                               int const ZoneNum,     // index to zone
                               Real64 &OAMassFlowRate // outside air mass flow rate [kg/s] from volume flow using std density
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         M. Witte (GARD)
    //       DATE WRITTEN   Jun 2011 (taken from HVACSingleDuctSystem.cc and adapted for Ideal Loads System)

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the amount of outside air required based on optional user input.
    // Zone multipliers have been applied in GetInput.

    // METHODOLOGY EMPLOYED:
    // User input defines method used to calculate OA.

    // FUNCTION PARAMETER DEFINITIONS:
    bool constexpr UseMinOASchFlag = true; // Always use min OA schedule in calculations.

    auto &PurchAir = state.dataPurchasedAirMgr->PurchAir(PurchAirNum);

    if (PurchAir.OutdoorAir) {
        bool UseOccSchFlag; // TRUE = use actual occupancy, FALSE = use total zone people

        if (PurchAir.DCVType == DCV::OccupancySchedule) {
            UseOccSchFlag = true;
        } else {
            UseOccSchFlag = false;
        }
        Real64 OAVolumeFlowRate =
            DataSizing::calcDesignSpecificationOutdoorAir(state, PurchAir.OARequirementsPtr, ZoneNum, UseOccSchFlag, UseMinOASchFlag);
        OAMassFlowRate = OAVolumeFlowRate * state.dataEnvrn->StdRhoAir;

        // If DCV with CO2SetPoint then check required OA flow to meet CO2 setpoint
        if (PurchAir.DCVType == DCV::CO2SetPoint) {
            OAMassFlowRate = max(OAMassFlowRate, state.dataContaminantBalance->ZoneSysContDemand(ZoneNum).OutputRequiredToCO2SP);
        }

        if (OAMassFlowRate <= HVAC::VerySmallMassFlow) {
            OAMassFlowRate = 0.0;
        }

    } else { // No outdoor air
        OAMassFlowRate = 0.0;
    }
    PurchAir.MinOAMassFlowRate = OAMassFlowRate;
}

void CalcPurchAirMixedAir(EnergyPlusData &state,
                          int const PurchAirNum,           // index to ideal loads unit
                          Real64 const OAMassFlowRate,     // outside air mass flow rate [kg/s]
                          Real64 const SupplyMassFlowRate, // supply air mass flow rate [kg/s]
                          Real64 &MixedAirTemp,            // Mixed air dry bulb temperature [C]
                          Real64 &MixedAirHumRat,          // Mixed air humidity ratio [kgWater/kgDryAir]
                          Real64 &MixedAirEnthalpy,        // Mixed air enthalpy [J/kg]
                          OpMode const OperatingMode       // current operating mode, Off, Heating, Cooling, or DeadBand
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         M. Witte (GARD)
    //       DATE WRITTEN   Sep 2011
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the mixed air conditions, accounting for heat recovery.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("CalcPurchAirMixedAir");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int RecircNodeNum;           // Zone return air node
    int OANodeNum;               // Outdoor air inlet node
    Real64 RecircTemp;           // Recirculated air from zone dry bulb temperature [C]
    Real64 RecircHumRat;         // Recirculated air from zone humidity ratio [kgWater/kgDryAir]
    Real64 RecircEnthalpy;       // Recirculated air from zone enthalpy [J/kg]
    Real64 RecircMassFlowRate;   // Recirculated air mass flow rate [kg/s]
    Real64 OAInletTemp;          // Outdoor air inlet dry bulb temperature [C]
    Real64 OAInletHumRat;        // Outdoor air inlet humidity ratio [kgWater/kgDryAir]
    Real64 OAInletEnthalpy;      // Outdoor air inlet enthalpy [J/kg]
    Real64 OAAfterHtRecTemp;     // Outdoor air after heat recovery to mixing box dry bulb temperature [C]
    Real64 OAAfterHtRecHumRat;   // Outdoor air after heat recovery to mixing box humidity ratio [kgWater/kgDryAir]
    Real64 OAAfterHtRecEnthalpy; // Outdoor air after heat recovery to mixing box enthalpy [J/kg]
    bool HeatRecOn;
    Real64 CpAir; // Specific heat [J/kg-C] reused in multiple places

    auto &PurchAir = state.dataPurchasedAirMgr->PurchAir(PurchAirNum);

    // Initializations
    OANodeNum = PurchAir.OutdoorAirNodeNum;
    RecircNodeNum = PurchAir.ZoneRecircAirNodeNum;

    RecircMassFlowRate = 0.0;
    RecircTemp = state.dataLoopNodes->Node(RecircNodeNum).Temp;
    RecircHumRat = state.dataLoopNodes->Node(RecircNodeNum).HumRat;
    RecircEnthalpy = state.dataLoopNodes->Node(RecircNodeNum).Enthalpy;
    if (PurchAir.OutdoorAir) {
        OAInletTemp = state.dataLoopNodes->Node(OANodeNum).Temp;
        OAInletHumRat = state.dataLoopNodes->Node(OANodeNum).HumRat;
        OAInletEnthalpy = state.dataLoopNodes->Node(OANodeNum).Enthalpy;
        OAAfterHtRecTemp = OAInletTemp;
        OAAfterHtRecHumRat = OAInletHumRat;
        OAAfterHtRecEnthalpy = OAInletEnthalpy;
    } else {
        OAInletTemp = 0.0;
        OAInletHumRat = 0.0;
        OAInletEnthalpy = 0.0;
        OAAfterHtRecTemp = OAInletTemp;
        OAAfterHtRecHumRat = OAInletHumRat;
        OAAfterHtRecEnthalpy = OAInletEnthalpy;
    }
    HeatRecOn = false;

    if (PurchAir.OutdoorAir && (OAMassFlowRate > 0.0)) {
        // Determine if heat recovery is beneficial
        if (PurchAir.HtRecType == HeatRecovery::Sensible) {
            if ((OperatingMode == OpMode::Heat) && (RecircTemp > OAInletTemp)) {
                HeatRecOn = true;
            }
            if ((OperatingMode == OpMode::Cool) && (RecircTemp < OAInletTemp)) {
                HeatRecOn = true;
            }
        }
        if (PurchAir.HtRecType == HeatRecovery::Enthalpy) {
            if ((OperatingMode == OpMode::Heat) && (RecircEnthalpy > OAInletEnthalpy)) {
                HeatRecOn = true;
            }
            if ((OperatingMode == OpMode::Cool) && (RecircEnthalpy < OAInletEnthalpy)) {
                HeatRecOn = true;
            }
        }
        // Calculate heat recovery if active
        if (HeatRecOn) {
            PurchAir.TimeHtRecActive = state.dataHVACGlobal->TimeStepSys;
            OAAfterHtRecTemp = OAInletTemp + PurchAir.HtRecSenEff * (RecircTemp - OAInletTemp);
            if (PurchAir.HtRecType == HeatRecovery::Enthalpy) {
                OAAfterHtRecHumRat = OAInletHumRat + PurchAir.HtRecLatEff * (RecircHumRat - OAInletHumRat);
            }
            OAAfterHtRecEnthalpy = PsyHFnTdbW(OAAfterHtRecTemp, OAAfterHtRecHumRat);
            //   Check for saturation in supply outlet and reset temp, then humidity ratio at constant enthalpy
            if (PsyTsatFnHPb(state, OAAfterHtRecEnthalpy, state.dataEnvrn->OutBaroPress, RoutineName) > OAAfterHtRecTemp) {
                OAAfterHtRecTemp = PsyTsatFnHPb(state, OAAfterHtRecEnthalpy, state.dataEnvrn->OutBaroPress, RoutineName);
                OAAfterHtRecHumRat = PsyWFnTdbH(state, OAAfterHtRecTemp, OAAfterHtRecEnthalpy, RoutineName);
            }
        }

        if (SupplyMassFlowRate > OAMassFlowRate) {
            RecircMassFlowRate = SupplyMassFlowRate - OAMassFlowRate;
            MixedAirEnthalpy =
                (RecircMassFlowRate * state.dataLoopNodes->Node(RecircNodeNum).Enthalpy + OAMassFlowRate * OAAfterHtRecEnthalpy) / SupplyMassFlowRate;
            MixedAirHumRat =
                (RecircMassFlowRate * state.dataLoopNodes->Node(RecircNodeNum).HumRat + OAMassFlowRate * OAAfterHtRecHumRat) / SupplyMassFlowRate;
            // Mixed air temperature is calculated from the mixed air enthalpy and humidity ratio.
            MixedAirTemp = PsyTdbFnHW(MixedAirEnthalpy, MixedAirHumRat);
        } else {
            RecircMassFlowRate = 0.0;
            MixedAirEnthalpy = OAAfterHtRecEnthalpy;
            MixedAirHumRat = OAAfterHtRecHumRat;
            MixedAirTemp = OAAfterHtRecTemp;
        }

        // Calculate OA and heat recovery sensible and latent rates
        CpAir = PsyCpAirFnW(OAInletHumRat);
        PurchAir.HtRecSenOutput = OAMassFlowRate * CpAir * (OAAfterHtRecTemp - OAInletTemp);
        PurchAir.HtRecLatOutput = OAMassFlowRate * (OAAfterHtRecEnthalpy - OAInletEnthalpy) - PurchAir.HtRecSenOutput;

    } else { // No outdoor air
        RecircMassFlowRate = SupplyMassFlowRate;
        MixedAirTemp = RecircTemp;
        MixedAirHumRat = RecircHumRat;
        MixedAirEnthalpy = RecircEnthalpy;
        PurchAir.HtRecSenOutput = 0.0;
        PurchAir.HtRecLatOutput = 0.0;
    }
}

void UpdatePurchasedAir(EnergyPlusData &state, int const PurchAirNum, bool const FirstHVACIteration)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         M. J. Witte
    //       DATE WRITTEN   Sep 2011
    //       MODIFIED       R. Raustad, July 2017, added return plenum
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Update node data for Ideal Loads (purchased air) system

    // USE STATEMENTS:
    using ZonePlenum::SimAirZonePlenum;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    // na
    bool FirstCall;
    bool SupPathInletChanged;

    FirstCall = true;            // just used to avoid redundant calculations
    SupPathInletChanged = false; // don't care if something changes

    auto &PurchAir = state.dataPurchasedAirMgr->PurchAir(PurchAirNum);

    if (PurchAir.ReturnPlenumIndex > 0) {

        // if connected to a return plenum, set the flag that this ideal loads air system was simulated
        state.dataPurchasedAirMgr->PurchAirPlenumArrays(PurchAir.ReturnPlenumIndex).IsSimulated(PurchAir.PurchAirArrayIndex) = true;

        // if all ideal loads air systems connected to the same plenum have been simulated, simulate the zone air plenum
        if (all(state.dataPurchasedAirMgr->PurchAirPlenumArrays(PurchAir.ReturnPlenumIndex).IsSimulated)) {
            SimAirZonePlenum(state,
                             PurchAir.ReturnPlenumName,
                             DataZoneEquipment::AirLoopHVACZone::ReturnPlenum,
                             PurchAir.ReturnPlenumIndex,
                             FirstHVACIteration,
                             FirstCall,
                             SupPathInletChanged);
            // reset this plenums flags for next iteration
            state.dataPurchasedAirMgr->PurchAirPlenumArrays(PurchAir.ReturnPlenumIndex).IsSimulated = false;
        }
    }
}

void ReportPurchasedAir(EnergyPlusData &state, int const PurchAirNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   Nov 1997
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate values of report variables, if necessary.

    // Using/Aliasing
    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    auto &PurchAir = state.dataPurchasedAirMgr->PurchAir(PurchAirNum);

    // Sort out heating and cooling rates
    PurchAir.SenHeatRate = max(PurchAir.SenCoilLoad, 0.0);
    PurchAir.SenCoolRate = std::abs(min(PurchAir.SenCoilLoad, 0.0));
    PurchAir.LatHeatRate = max(PurchAir.LatCoilLoad, 0.0);
    PurchAir.LatCoolRate = std::abs(min(PurchAir.LatCoilLoad, 0.0));
    PurchAir.TotHeatRate = PurchAir.SenHeatRate + PurchAir.LatHeatRate;
    PurchAir.TotCoolRate = PurchAir.SenCoolRate + PurchAir.LatCoolRate;

    PurchAir.ZoneSenHeatRate = max(PurchAir.SenOutputToZone, 0.0);
    PurchAir.ZoneSenCoolRate = std::abs(min(PurchAir.SenOutputToZone, 0.0));
    PurchAir.ZoneLatHeatRate = max(PurchAir.LatOutputToZone, 0.0);
    PurchAir.ZoneLatCoolRate = std::abs(min(PurchAir.LatOutputToZone, 0.0));
    PurchAir.ZoneTotHeatRate = PurchAir.ZoneSenHeatRate + PurchAir.ZoneLatHeatRate;
    PurchAir.ZoneTotCoolRate = PurchAir.ZoneSenCoolRate + PurchAir.ZoneLatCoolRate;

    // Sort out outdoor air "loads"
    // OASenOutput = Outdoor air sensible output relative to zone conditions [W], <0 means OA is cooler than zone air
    // OALatOutput  = Outdoor air latent output relative to zone conditions [W], <0 means OA is drier than zone air
    if (PurchAir.SenCoilLoad > 0.0) { // Heating is active
        PurchAir.OASenHeatRate = std::abs(min(PurchAir.OASenOutput, 0.0));
    } else {
        PurchAir.OASenHeatRate = 0.0;
    }
    if (PurchAir.SenCoilLoad < 0.0) { // Cooling is active
        PurchAir.OASenCoolRate = max(PurchAir.OASenOutput, 0.0);
    } else {
        PurchAir.OASenCoolRate = 0.0;
    }
    if (PurchAir.LatCoilLoad > 0.0) { // Humidification is active
        PurchAir.OALatHeatRate = std::abs(min(PurchAir.OALatOutput, 0.0));
    } else {
        PurchAir.OALatHeatRate = 0.0;
    }
    if (PurchAir.LatCoilLoad < 0.0) { // Dehumidification is active
        PurchAir.OALatCoolRate = max(PurchAir.OALatOutput, 0.0);
    } else {
        PurchAir.OALatCoolRate = 0.0;
    }

    PurchAir.OATotHeatRate = PurchAir.OASenHeatRate + PurchAir.OALatHeatRate;
    PurchAir.OATotCoolRate = PurchAir.OASenCoolRate + PurchAir.OALatCoolRate;

    PurchAir.HtRecSenHeatRate = max(PurchAir.HtRecSenOutput, 0.0);
    PurchAir.HtRecSenCoolRate = std::abs(min(PurchAir.HtRecSenOutput, 0.0));
    PurchAir.HtRecLatHeatRate = max(PurchAir.HtRecLatOutput, 0.0);
    PurchAir.HtRecLatCoolRate = std::abs(min(PurchAir.HtRecLatOutput, 0.0));
    PurchAir.HtRecTotHeatRate = PurchAir.HtRecSenHeatRate + PurchAir.HtRecLatHeatRate;
    PurchAir.HtRecTotCoolRate = PurchAir.HtRecSenCoolRate + PurchAir.HtRecLatCoolRate;

    PurchAir.SenHeatEnergy = PurchAir.SenHeatRate * TimeStepSysSec;
    PurchAir.SenCoolEnergy = PurchAir.SenCoolRate * TimeStepSysSec;
    PurchAir.LatHeatEnergy = PurchAir.LatHeatRate * TimeStepSysSec;
    PurchAir.LatCoolEnergy = PurchAir.LatCoolRate * TimeStepSysSec;
    PurchAir.TotHeatEnergy = PurchAir.TotHeatRate * TimeStepSysSec;
    PurchAir.TotCoolEnergy = PurchAir.TotCoolRate * TimeStepSysSec;

    PurchAir.ZoneSenHeatEnergy = PurchAir.ZoneSenHeatRate * TimeStepSysSec;
    PurchAir.ZoneSenCoolEnergy = PurchAir.ZoneSenCoolRate * TimeStepSysSec;
    PurchAir.ZoneLatHeatEnergy = PurchAir.ZoneLatHeatRate * TimeStepSysSec;
    PurchAir.ZoneLatCoolEnergy = PurchAir.ZoneLatCoolRate * TimeStepSysSec;
    PurchAir.ZoneTotHeatEnergy = PurchAir.ZoneTotHeatRate * TimeStepSysSec;
    PurchAir.ZoneTotCoolEnergy = PurchAir.ZoneTotCoolRate * TimeStepSysSec;

    PurchAir.OASenHeatEnergy = PurchAir.OASenHeatRate * TimeStepSysSec;
    PurchAir.OASenCoolEnergy = PurchAir.OASenCoolRate * TimeStepSysSec;
    PurchAir.OALatHeatEnergy = PurchAir.OALatHeatRate * TimeStepSysSec;
    PurchAir.OALatCoolEnergy = PurchAir.OALatCoolRate * TimeStepSysSec;
    PurchAir.OATotHeatEnergy = PurchAir.OATotHeatRate * TimeStepSysSec;
    PurchAir.OATotCoolEnergy = PurchAir.OATotCoolRate * TimeStepSysSec;

    PurchAir.HtRecSenHeatEnergy = PurchAir.HtRecSenHeatRate * TimeStepSysSec;
    PurchAir.HtRecSenCoolEnergy = PurchAir.HtRecSenCoolRate * TimeStepSysSec;
    PurchAir.HtRecLatHeatEnergy = PurchAir.HtRecLatHeatRate * TimeStepSysSec;
    PurchAir.HtRecLatCoolEnergy = PurchAir.HtRecLatCoolRate * TimeStepSysSec;
    PurchAir.HtRecTotHeatEnergy = PurchAir.HtRecTotHeatRate * TimeStepSysSec;
    PurchAir.HtRecTotCoolEnergy = PurchAir.HtRecTotCoolRate * TimeStepSysSec;
}

Real64 GetPurchasedAirOutAirMassFlow(EnergyPlusData &state, int const PurchAirNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         B Griffith
    //       DATE WRITTEN   Dec  2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // lookup function for OA inlet mass flow for ventilation rate reporting

    // METHODOLOGY EMPLOYED:
    // most analogous functions look up an outside air node but this function
    // gets the actual mass flow of outdoor air, following the features of the model

    if (state.dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(state);
        state.dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }
    return state.dataPurchasedAirMgr->PurchAir(PurchAirNum).OutdoorAirMassFlowRate;
}

int GetPurchasedAirZoneInletAirNode(EnergyPlusData &state, int const PurchAirNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         B Griffith
    //       DATE WRITTEN   Dec  2006
    //       MODIFIED       Adapted for purchased air by M.J. Witte, Oct 2013
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // lookup function for zone inlet node for ventilation rate reporting

    if (state.dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(state);
        state.dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }

    int GetPurchasedAirZoneInletAirNode = 0;
    if (PurchAirNum > 0 && PurchAirNum <= state.dataPurchasedAirMgr->NumPurchAir) {
        GetPurchasedAirZoneInletAirNode = state.dataPurchasedAirMgr->PurchAir(PurchAirNum).ZoneSupplyAirNodeNum;
    }

    return GetPurchasedAirZoneInletAirNode;
}

int GetPurchasedAirReturnAirNode(EnergyPlusData &state, int const PurchAirNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         B Griffith
    //       DATE WRITTEN   Dec  2006
    //       MODIFIED       Adapted for purchased air by M.J. Witte, Oct 2013
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // lookup function for recirculation air node for ventilation rate reporting

    if (state.dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(state);
        state.dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }

    int GetPurchasedAirReturnAirNode = 0;
    if (PurchAirNum > 0 && PurchAirNum <= state.dataPurchasedAirMgr->NumPurchAir) {
        GetPurchasedAirReturnAirNode = state.dataPurchasedAirMgr->PurchAir(PurchAirNum).ZoneRecircAirNodeNum;
    }

    return GetPurchasedAirReturnAirNode;
}

int getPurchasedAirIndex(EnergyPlusData &state, std::string_view PurchAirName)
{
    if (state.dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(state);
        state.dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }

    for (int PurchAirNum = 1; PurchAirNum <= state.dataPurchasedAirMgr->NumPurchAir; ++PurchAirNum) {
        if (Util::SameString(state.dataPurchasedAirMgr->PurchAir(PurchAirNum).Name, PurchAirName)) {
            return PurchAirNum;
        }
    }

    return 0;
}

Real64 GetPurchasedAirMixedAirTemp(EnergyPlusData &state, int const PurchAirNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         B Griffith
    //       DATE WRITTEN   Dec  2006
    //       MODIFIED       Adapted for purchased air by M.J. Witte, Oct 2013
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // lookup function for mixed air Temp for ventilation rate reporting

    // METHODOLOGY EMPLOYED:
    // most analogous functions look up an outside air node but this function
    // gets the actual mass flow of outdoor air, following the features of the model

    if (state.dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(state);
        state.dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }

    return state.dataPurchasedAirMgr->PurchAir(PurchAirNum).MixedAirTemp;
}

Real64 GetPurchasedAirMixedAirHumRat(EnergyPlusData &state, int const PurchAirNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         B Griffith
    //       DATE WRITTEN   Dec  2006
    //       MODIFIED       Adapted for purchased air by M.J. Witte, Oct 2013
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // lookup function for mixed air HumRat for ventilation rate reporting

    // METHODOLOGY EMPLOYED:
    // most analogous functions look up an outside air node but this function
    // gets the actual mass flow of outdoor air, following the features of the model

    if (state.dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(state);
        state.dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }

    return state.dataPurchasedAirMgr->PurchAir(PurchAirNum).MixedAirHumRat;
}

bool CheckPurchasedAirForReturnPlenum(EnergyPlusData &state, int const ReturnPlenumIndex)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         R Raustad
    //       DATE WRITTEN   July  2017

    // PURPOSE OF THIS FUNCTION:
    // lookup function to check if return plenum is used

    // Return value
    bool CheckPurchasedAirForReturnPlenum;

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    int PurchAirNum;

    if (state.dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(state);
        state.dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }

    CheckPurchasedAirForReturnPlenum = false;
    for (PurchAirNum = 1; PurchAirNum <= state.dataPurchasedAirMgr->NumPurchAir; ++PurchAirNum) {
        if (ReturnPlenumIndex != state.dataPurchasedAirMgr->PurchAir(PurchAirNum).ReturnPlenumIndex) {
            continue;
        }
        CheckPurchasedAirForReturnPlenum = true;
    }

    return CheckPurchasedAirForReturnPlenum;
}

void InitializePlenumArrays(EnergyPlusData &state, int const PurchAirNum)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         R Raustad
    //       DATE WRITTEN   July  2017

    // PURPOSE OF THIS FUNCTION:
    // to initialize arrays needed to manage ideal load air system used with return plenums
    //
    // Example:
    // NumPlenumArrays = 2 (same as there are two ZoneHVAC:ReturnPlenums objects connected to two or more ideal loads air systems
    // In this example ideal loads air system #4 is not connected to a zone return plenum
    //
    // ZoneHVAC:ReturnPlenum( 1 ) = ReturnPlenum1 is not connected to any ideal loads air systems
    // ZoneHVAC:ReturnPlenum( 2 ) = ReturnPlenum2 is connected to PurchAirPlenumArrays( 1 )
    // ZoneHVAC:ReturnPlenum( 3 ) = ReturnPlenum3 is connected to PurchAirPlenumArrays( 2 )
    //
    // PurchAirPlenumArrays( 1 )
    //   PurchAirPlenumArrays( 1 ).NumPurchAir = 2, there are 2 ideal loads air systems connected to this plenum
    //      PurchAirPlenumArrays( 1 ).PurchAirArray( 1 ) = 1, ideal loads air system #1 is attached to this plenum
    //      PurchAirPlenumArrays( 1 ).PurchAirArray( 2 ) = 3, ideal loads air system #3 is attached to this plenum
    //      PurchAirPlenumArrays( 1 ).IsSimulated( 1 ) = true, ideal loads air system #1 has been simulated this iteration
    //      PurchAirPlenumArrays( 1 ).IsSimulated( 2 ) = false, ideal loads air system #3 has not yet been simulated this iteration
    //
    //      Ideal loads air systems keep track of which plenum they are connected to
    //      PurchAir( 1 ).PlenumArrayIndex = 1
    //      PurchAir( 1 ).ReturnPlenumName = ReturnPlenum2;
    //      PurchAir( 3 ).PlenumArrayIndex = 1
    //      PurchAir( 3 ).ReturnPlenumName = ReturnPlenum2;
    //
    //      The ideal loads air systems also keep track of which item they are in the int and bool arrays
    //      PurchAir( 1 ).PurchAirArrayIndex = 1
    //      PurchAir( 3 ).PurchAirArrayIndex = 2
    //
    // PurchAirPlenumArrays( 2 )
    //   PurchAirPlenumArrays( 2 ).NumPurchAir = 3, there are 3 ideal loads air systems connected to this plenum
    //      PurchAirPlenumArrays( 2 ).PurchAirArray( 1 ) = 2, ideal loads air system #2 is attached to this plenum
    //      PurchAirPlenumArrays( 2 ).PurchAirArray( 2 ) = 5, ideal loads air system #5 is attached to this plenum
    //      PurchAirPlenumArrays( 2 ).PurchAirArray( 3 ) = 6, ideal loads air system #6 is attached to this plenum
    //      PurchAirPlenumArrays( 2 ).IsSimulated( 1 ) = true, ideal loads air system #4 has been simulated this iteration
    //      PurchAirPlenumArrays( 2 ).IsSimulated( 2 ) = false, ideal loads air system #5 has not yet been simulated this iteration
    //      PurchAirPlenumArrays( 2 ).IsSimulated( 3 ) = false, ideal loads air system #6 has not yet been simulated this iteration
    //
    //      Ideal loads air systems keep track of which plenum they are connected to
    //      PurchAir( 2 ).PlenumArrayIndex = 2;
    //      PurchAir( 2 ).ReturnPlenumName = ReturnPlenum3;
    //      PurchAir( 5 ).PlenumArrayIndex = 2;
    //      PurchAir( 5 ).ReturnPlenumName = ReturnPlenum3;
    //      PurchAir( 6 ).PlenumArrayIndex = 2;
    //      PurchAir( 6 ).ReturnPlenumName = ReturnPlenum3;
    //
    //      The ideal loads air systems also keep track of which item they are in the int and bool arrays
    //      PurchAir( 2 ).PurchAirArrayIndex = 1;
    //      PurchAir( 5 ).PurchAirArrayIndex = 2;
    //      PurchAir( 6 ).PurchAirArrayIndex = 3;
    //
    //      Given these connections, the data in the IsSimulated array can be set (or checked) according to this syntax:
    //
    //      Each time an ideal loads air system is simulated the IsSimulated flag is set to true
    //      PurchAirPlenumArrays( PurchAir( PurchNum ).PlenumArrayIndex ).IsSimulated( PurchAir( PurchNum ).PurchAirArrayIndex ) = true;
    //
    //     if all ideal loads air systems connected to the same plenum have been simulated, simulate the zone air return plenum (once per set of
    //     ideal loads systems) if ( all( PurchAirPlenumArrays( PurchAir( PurchAirNum ).ReturnPlenumIndex ).IsSimulated ) ) {
    //         SimAirZonePlenum( PurchAir( PurchAirNum ).ReturnPlenumName, DataZoneEquipment::ZoneReturnPlenum_Type, PurchAir( PurchAirNum
    //         ).ReturnPlenumIndex, FirstHVACIteration, FirstCall, SupPathInletChanged ); reset all IsSimulated flags for next iteration
    //         PurchAirPlenumArrays( PurchAir( PurchAirNum ).ReturnPlenumIndex ).IsSimulated = false;
    //     }

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    int ReturnPlenumIndex;        // index to ZoneHVAC:ReturnPlenum object
    bool PlenumNotFound;          // logical to determine if same plenum is used by other ideal loads air systems
    Array1D_int TempPurchArray;   // temporary array used for dynamic allocation
    Array1D_bool TempIsSimulated; // temporary array used for dynamic allocation

    // index to ZoneHVAC:ReturnPlenum object
    ReturnPlenumIndex = state.dataPurchasedAirMgr->PurchAir(PurchAirNum).ReturnPlenumIndex;
    PlenumNotFound = true;

    // if first time through, set up arrays
    if (!state.dataPurchasedAirMgr->PurchAirPlenumArrays.allocated()) {

        // the ideal loads air system keeps track of which item this system is in a list
        state.dataPurchasedAirMgr->PurchAir(PurchAirNum).PurchAirArrayIndex = 1;
        // keep track of how many arrays (i.e., how many different plenums are attached to different ideal loads air systems
        state.dataPurchasedAirMgr->NumPlenumArrays = 1;

        // allocate new array
        state.dataPurchasedAirMgr->PurchAirPlenumArrays.allocate(state.dataPurchasedAirMgr->NumPlenumArrays);
        // set counter for how many ideal loads air systems are attached to this plenum
        state.dataPurchasedAirMgr->PurchAirPlenumArrays(state.dataPurchasedAirMgr->NumPlenumArrays).NumPurchAir =
            1; // keeps track of how many ideal loads air system are connected to this return plenum
        // keep track of which plenum this is ( i.e., PurchAirPlenumArrays(1) is ZoneHVAC:ReturnPlenum #4 )
        state.dataPurchasedAirMgr->PurchAirPlenumArrays(state.dataPurchasedAirMgr->NumPlenumArrays).ReturnPlenumIndex =
            ReturnPlenumIndex; // stores index of return plenum (e.g., 4 of 5)
        // allocate array holding index to one or more ideal loads air systems
        state.dataPurchasedAirMgr->PurchAirPlenumArrays(state.dataPurchasedAirMgr->NumPlenumArrays).PurchAirArray.allocate(1);
        // allocate boolean to keep track of whether or not this ideal loads air system has been simulated
        state.dataPurchasedAirMgr->PurchAirPlenumArrays(state.dataPurchasedAirMgr->NumPlenumArrays).IsSimulated.allocate(1);
        // save the data
        state.dataPurchasedAirMgr->PurchAirPlenumArrays(state.dataPurchasedAirMgr->NumPlenumArrays).PurchAirArray(1) = PurchAirNum;
        state.dataPurchasedAirMgr->PurchAirPlenumArrays(state.dataPurchasedAirMgr->NumPlenumArrays).IsSimulated(1) = false;

    } else {

        // find the correct index to PurchAirPlenumArrays
        for (int ReturnPlenumNum = 1; ReturnPlenumNum <= state.dataPurchasedAirMgr->NumPlenumArrays; ++ReturnPlenumNum) {
            if (ReturnPlenumIndex != state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum).ReturnPlenumIndex) {
                continue;
            }

            // allocate temporary arrays and save existing data
            TempPurchArray.allocate(state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum).NumPurchAir);
            TempIsSimulated.allocate(state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum).NumPurchAir);
            // these are the  member arrays in an existing PurchAirPlenumArrays
            TempPurchArray = state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum).PurchAirArray;
            TempIsSimulated = state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum).IsSimulated;

            // if this array has been used before, we need to increase member array space to save new PurchAir data
            state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum).NumPurchAir += 1;
            // save the location of this ideal loads air system in the member arrays
            state.dataPurchasedAirMgr->PurchAir(PurchAirNum).PurchAirArrayIndex =
                state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum).NumPurchAir;

            // allocate more space, this will wipe out data previously stored
            state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum)
                .PurchAirArray.allocate(state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum).NumPurchAir);
            state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum)
                .IsSimulated.allocate(state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum).NumPurchAir);

            // re-initialize previous data
            for (int Loop = 1; Loop < state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum).NumPurchAir; ++Loop) {
                state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum).PurchAirArray(Loop) = TempPurchArray(Loop);
                state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum).IsSimulated(Loop) = TempIsSimulated(Loop);
            }
            // delete temporary array
            TempPurchArray.deallocate();
            TempIsSimulated.deallocate();

            // save new data in expanded member array
            state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum)
                .PurchAirArray(state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum).NumPurchAir) = PurchAirNum;
            state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum)
                .IsSimulated(state.dataPurchasedAirMgr->PurchAirPlenumArrays(ReturnPlenumNum).NumPurchAir) = false;

            PlenumNotFound = false;
            break;
        }

        if (PlenumNotFound) {

            // need to allocate additional space for new plenum array
            // keep track of how many arrays (i.e., how many different plenums are attached to different ideal loads air systems)
            state.dataPurchasedAirMgr->NumPlenumArrays += 1;

            // allocate temporary array and save existing data
            state.dataPurchasedAirMgr->TempPurchAirPlenumArrays.allocate(state.dataPurchasedAirMgr->NumPlenumArrays);
            for (int Loop = 1; Loop < state.dataPurchasedAirMgr->NumPlenumArrays; ++Loop) {
                state.dataPurchasedAirMgr->TempPurchAirPlenumArrays(Loop).NumPurchAir =
                    state.dataPurchasedAirMgr->PurchAirPlenumArrays(Loop).NumPurchAir;
                state.dataPurchasedAirMgr->TempPurchAirPlenumArrays(Loop).ReturnPlenumIndex =
                    state.dataPurchasedAirMgr->PurchAirPlenumArrays(Loop).ReturnPlenumIndex;
                state.dataPurchasedAirMgr->TempPurchAirPlenumArrays(Loop).PurchAirArray.allocate(
                    state.dataPurchasedAirMgr->PurchAirPlenumArrays(Loop).NumPurchAir);
                state.dataPurchasedAirMgr->TempPurchAirPlenumArrays(Loop).IsSimulated.allocate(
                    state.dataPurchasedAirMgr->PurchAirPlenumArrays(Loop).NumPurchAir);
                for (int Loop2 = 1; Loop2 <= state.dataPurchasedAirMgr->PurchAirPlenumArrays(Loop).NumPurchAir; ++Loop2) {
                    state.dataPurchasedAirMgr->TempPurchAirPlenumArrays(Loop).PurchAirArray(Loop2) =
                        state.dataPurchasedAirMgr->PurchAirPlenumArrays(Loop).PurchAirArray(Loop2);
                    state.dataPurchasedAirMgr->TempPurchAirPlenumArrays(Loop).IsSimulated(Loop2) =
                        state.dataPurchasedAirMgr->PurchAirPlenumArrays(Loop).IsSimulated(Loop2);
                }
            }

            // delete primary array (probably could just re-allocate, but this is only done a few times per simulation)
            state.dataPurchasedAirMgr->PurchAirPlenumArrays.deallocate();
            // reallocate to new size
            state.dataPurchasedAirMgr->PurchAirPlenumArrays.allocate(state.dataPurchasedAirMgr->NumPlenumArrays);

            // allocate member arrays to same size as before
            for (int Loop = 1; Loop < state.dataPurchasedAirMgr->NumPlenumArrays; ++Loop) {
                state.dataPurchasedAirMgr->PurchAirPlenumArrays(Loop).PurchAirArray.allocate(
                    state.dataPurchasedAirMgr->TempPurchAirPlenumArrays(Loop).NumPurchAir);
                state.dataPurchasedAirMgr->PurchAirPlenumArrays(Loop).IsSimulated.allocate(
                    state.dataPurchasedAirMgr->TempPurchAirPlenumArrays(Loop).NumPurchAir);
            }

            // save the data
            state.dataPurchasedAirMgr->PurchAirPlenumArrays = state.dataPurchasedAirMgr->TempPurchAirPlenumArrays;
            // delete temporary data
            state.dataPurchasedAirMgr->TempPurchAirPlenumArrays.deallocate();

            // save the index to where this ideal loads air system data is stored
            state.dataPurchasedAirMgr->PurchAir(PurchAirNum).PurchAirArrayIndex = 1;
            // save the number of ideal loads air systems stored in these arrays
            state.dataPurchasedAirMgr->PurchAirPlenumArrays(state.dataPurchasedAirMgr->NumPlenumArrays).NumPurchAir = 1;
            // save the index the the ZoneHVAC:ReturnPlenum
            state.dataPurchasedAirMgr->PurchAirPlenumArrays(state.dataPurchasedAirMgr->NumPlenumArrays).ReturnPlenumIndex = ReturnPlenumIndex;
            // allocate member array and store data
            state.dataPurchasedAirMgr->PurchAirPlenumArrays(state.dataPurchasedAirMgr->NumPlenumArrays).PurchAirArray.allocate(1);
            state.dataPurchasedAirMgr->PurchAirPlenumArrays(state.dataPurchasedAirMgr->NumPlenumArrays).PurchAirArray(1) = PurchAirNum;
            // allocate member array and store data
            state.dataPurchasedAirMgr->PurchAirPlenumArrays(state.dataPurchasedAirMgr->NumPlenumArrays).IsSimulated.allocate(1);
            state.dataPurchasedAirMgr->PurchAirPlenumArrays(state.dataPurchasedAirMgr->NumPlenumArrays).IsSimulated(1) = false;
        }
    }
}

} // namespace EnergyPlus::PurchasedAirManager
