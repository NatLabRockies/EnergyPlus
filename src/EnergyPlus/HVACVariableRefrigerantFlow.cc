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
#include <array>
#include <cassert>
#include <cmath>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/All_Simple_Sizing.hh>
#include <EnergyPlus/Autosizing/CoolingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/HeatingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/SystemAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/WaterHeatingCapacitySizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACVariableRefrigerantFlow.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/UnitarySystem.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/WaterManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::HVACVariableRefrigerantFlow {
// Module containing the Variable Refrigerant Flow (VRF or VRV) simulation routines

// MODULE INFORMATION:
//       AUTHOR         Richard Raustad, FSEC
//       DATE WRITTEN   August 2010
//       MODIFIED       Apr 2012, R. Raustad, FSEC, Added Heat Recovery Operating Mode
//                      Jul 2015, RP Zhang, XF Pang, LBNL, Added a new physics based VRF model applicable for Fluid Temperature Control
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms required to
// manage the VRF System Component

using namespace DataZoneEnergyDemands;
using namespace Psychrometrics;
using namespace DataPlant;

constexpr std::array<std::string_view, (int)TUType::Num> tuTypeNames = {"ZoneHVAC:TerminalUnit:VariableRefrigerantFlow"};

void SimulateVRF(EnergyPlusData &state,
                 std::string_view CompName,
                 bool const FirstHVACIteration,
                 int const ZoneNum,
                 int &CompIndex,
                 bool &HeatingActive,
                 bool &CoolingActive,
                 [[maybe_unused]] int const OAUnitNum,         // If the system is an equipment of OutdoorAirUnit
                 [[maybe_unused]] Real64 const OAUCoilOutTemp, // the coil inlet temperature of OutdoorAirUnit
                 [[maybe_unused]] bool const ZoneEquipment,    // TRUE if called as zone equipment
                 Real64 &SysOutputProvided,
                 Real64 &LatOutputProvided)
{

    // SUBROUTINE INFORMATION:
    // AUTHOR         Richard Raustad, FSEC
    // DATE WRITTEN   August 2010
    // MODIFIED       Jul 2015, RP Zhang (LBNL), XF Pang (LBNL), Y Yura (Daikin Inc). Add a physics-based VRF model applicable for Fluid
    // Temperature Control RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages VRF terminal unit simulation.

    // METHODOLOGY EMPLOYED:
    // Simulate all terminal units
    // Once all terminal units have been simulated, simulate VRF condenser

    int VRFTUNum;             // current VRF system terminal unit index
    int VRFCondenser;         // index to VRF AC system object - AirConditioner:VariableRefrigerantFlow
    int TUListNum;            // index to VRF AC system terminal unit list
    int IndexToTUInTUList;    // index to pointer in VRF AC system terminal unit list
    Real64 OnOffAirFlowRatio; // ratio of compressor ON airflow to average airflow over timestep
    int DXCoolingCoilIndex;   // index to this terminal units DX cooling coil
    int DXHeatingCoilIndex;   // index to this terminal units DX heating coil
    Real64 QZnReq;

    // Obtains and Allocates VRF system related parameters from input file
    if (state.dataHVACVarRefFlow->GetVRFInputFlag) { // First time subroutine has been entered
        GetVRFInput(state);
        state.dataHVACVarRefFlow->GetVRFInputFlag = false;
    }

    // CompIndex accounting
    if (CompIndex == 0) {
        VRFTUNum = Util::FindItemInList(CompName, state.dataHVACVarRefFlow->VRFTU);
        if (VRFTUNum == 0) {
            ShowFatalError(state, format("SimulateVRF: VRF Terminal Unit not found={}", CompName));
        }
        CompIndex = VRFTUNum;

    } else {
        VRFTUNum = CompIndex;
        if (VRFTUNum > state.dataHVACVarRefFlow->NumVRFTU || VRFTUNum < 1) {
            ShowFatalError(state,
                           format("SimulateVRF: Invalid CompIndex passed={}, Number of VRF Terminal Units = {}, VRF Terminal Unit name = {}",
                                  VRFTUNum,
                                  state.dataHVACVarRefFlow->NumVRFTU,
                                  CompName));
        }
        if (state.dataHVACVarRefFlow->CheckEquipName(VRFTUNum)) {
            if (!CompName.empty() && CompName != state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name) {
                ShowFatalError(state,
                               format("SimulateVRF: Invalid CompIndex passed={}, VRF Terminal Unit name={}, stored VRF TU Name for that index={}",
                                      VRFTUNum,
                                      CompName,
                                      state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
            }
            state.dataHVACVarRefFlow->CheckEquipName(VRFTUNum) = false;
        }
    }

    // the VRF condenser index
    VRFCondenser = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFSysNum;

    if ((state.dataHVACVarRefFlow->VRF(VRFCondenser).CondenserType == DataHeatBalance::RefrigCondenserType::Water) &&
        (state.dataHVACVarRefFlow->VRF(VRFCondenser).checkPlantCondTypeOneTime)) {
        // scan for loop connection data
        bool errFlag = false;
        PlantUtilities::ScanPlantLoopsForObject(state,
                                                state.dataHVACVarRefFlow->VRF(VRFCondenser).Name,
                                                state.dataHVACVarRefFlow->VRF(VRFCondenser).VRFType,
                                                state.dataHVACVarRefFlow->VRF(VRFCondenser).SourcePlantLoc,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                state.dataHVACVarRefFlow->VRF(VRFCondenser).CondenserNodeNum,
                                                _);

        if (errFlag) {
            ShowSevereError(state, "GetVRFInput: Error scanning for plant loop data");
        }

        state.dataHVACVarRefFlow->VRF(VRFCondenser).checkPlantCondTypeOneTime = false;
    }

    // the terminal unit list object index
    TUListNum = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TUListIndex;
    // the entry number in the terminal unit list (which item in the terminal unit list, e.g. second in list)
    IndexToTUInTUList = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).IndexToTUInTUList;
    // index to cooling coil (coil is optional but at least one must be present)
    DXCoolingCoilIndex = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolCoilIndex;
    // index to heating coil (coil is optional but at least one must be present)
    DXHeatingCoilIndex = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatCoilIndex;
    QZnReq = 0.0;

    // Initialize terminal unit
    InitVRF(state, VRFTUNum, ZoneNum, FirstHVACIteration, OnOffAirFlowRatio, QZnReq); // Initialize all VRFTU related parameters

    // Simulate terminal unit
    SimVRF(state, VRFTUNum, FirstHVACIteration, OnOffAirFlowRatio, SysOutputProvided, LatOutputProvided, QZnReq);

    // mark this terminal unit as simulated
    state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).IsSimulated(IndexToTUInTUList) = true;

    // keep track of individual coil loads
    if (DXCoolingCoilIndex > 0) {
        state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(IndexToTUInTUList) =
            state.dataDXCoils->DXCoilTotalCooling(DXCoolingCoilIndex);
    } else {
        state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(IndexToTUInTUList) = 0.0;
    }
    if (DXHeatingCoilIndex > 0) {
        state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(IndexToTUInTUList) =
            state.dataDXCoils->DXCoilTotalHeating(DXHeatingCoilIndex);
    } else {
        state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(IndexToTUInTUList) = 0.0;
    }

    // Report the current VRF terminal unit
    ReportVRFTerminalUnit(state, VRFTUNum);

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TotalCoolingRate > 0.0) {
        CoolingActive = true;
    }
    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TotalHeatingRate > 0.0) {
        HeatingActive = true;
    }

    // make sure all TU in a list are able to get simulated, otherwise condenser is never simulated **
    // either fatal on GetInput, or keep track of unused TU's and set their respective flag to TRUE **
    // after all VRF terminal units have been simulated, call the VRF condenser model
    if (all(state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).IsSimulated)) {

        if (state.dataHVACVarRefFlow->VRF(VRFCondenser).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
            state.dataHVACVarRefFlow->VRF(VRFCondenser).CalcVRFCondenser_FluidTCtrl(state, FirstHVACIteration);
        } else {
            // Algorithm Type: VRF model based on system curve
            CalcVRFCondenser(state, VRFCondenser);
        }

        ReportVRFCondenser(state, VRFCondenser);

        if (state.dataHVACVarRefFlow->VRF(VRFCondenser).CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
            UpdateVRFCondenser(state, VRFCondenser);
        }
    }
}

PlantComponent *VRFCondenserEquipment::factory(EnergyPlusData &state, std::string const &objectName)
{
    // Process the input data if it hasn't been done already
    if (state.dataHVACVarRefFlow->GetVRFInputFlag) {
        GetVRFInput(state);
        state.dataHVACVarRefFlow->GetVRFInputFlag = false;
    }
    // Now look for this object in the list
    for (auto &obj : state.dataHVACVarRefFlow->VRF) {
        if (obj.Name == objectName) {
            return &obj;
        }
    }
    // If we didn't find it, fatal
    ShowFatalError(state, format("LocalVRFCondenserFactory: Error getting inputs for object named: {}", objectName)); // LCOV_EXCL_LINE
    // Shut up the compiler
    return nullptr; // LCOV_EXCL_LINE
}

void VRFCondenserEquipment::onInitLoopEquip(EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation)
{
    this->SizeVRFCondenser(state);
}

void VRFCondenserEquipment::getDesignCapacities([[maybe_unused]] EnergyPlusData &state,
                                                [[maybe_unused]] const PlantLocation &calledFromLocation,
                                                Real64 &MaxLoad,
                                                Real64 &MinLoad,
                                                Real64 &OptLoad)
{
    MinLoad = 0.0;
    MaxLoad = max(this->CoolingCapacity, this->HeatingCapacity); // greater of cooling and heating capacity
    OptLoad = max(this->CoolingCapacity,
                  this->HeatingCapacity); // connects to single loop, need to switch between cooling/heating capacity?
}

void VRFCondenserEquipment::simulate(EnergyPlusData &state,
                                     const PlantLocation &calledFromLocation,
                                     bool FirstHVACIteration,
                                     [[maybe_unused]] Real64 &CurLoad,
                                     [[maybe_unused]] bool RunFlag)
{
    if (calledFromLocation.loopNum == this->SourcePlantLoc.loopNum) { // condenser loop
        PlantUtilities::UpdateChillerComponentCondenserSide(state,
                                                            this->SourcePlantLoc.loopNum,
                                                            this->SourcePlantLoc.loopSideNum,
                                                            PlantEquipmentType::HeatPumpVRF,
                                                            this->CondenserNodeNum,
                                                            this->CondenserOutletNodeNum,
                                                            this->QCondenser,
                                                            this->CondenserInletTemp,
                                                            this->CondenserSideOutletTemp,
                                                            this->WaterCondenserMassFlow,
                                                            FirstHVACIteration);
    } else {
        ShowFatalError(state, format("SimVRFCondenserPlant:: Invalid loop connection {}", cVRFTypes(VRF_HeatPump)));
    }
}

void CalcVRFCondenser(EnergyPlusData &state, int const VRFCond)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         R. Raustad, FSEC
    //       DATE WRITTEN   September 2010

    // PURPOSE OF THIS SUBROUTINE:
    // Model the interactions of VRF terminal units with a single variable-speed condenser.
    // The terminal units are simulated first, and then the condenser is simulated.
    // If terminal units require more capacity than can be delivered by condenser, a limit is set.

    using Curve::CurveValue;
    using PlantUtilities::SetComponentFlowRate;
    using Psychrometrics::RhoH2O;

    static constexpr std::string_view RoutineName("VRFCondenser");

    int NumTU; // loop counter

    Real64 TotCoolCapTempModFac;      // cooling CAPFT curve output
    Real64 TotHeatCapTempModFac;      // heating CAPFT curve output
    Real64 TotCoolEIRTempModFac;      // cooling EIRFT curve output
    Real64 TotHeatEIRTempModFac;      // heating EIRFT curve output
    Real64 InletAirWetBulbC;          // coil inlet air wet-bulb temperature (C)
    Real64 InletAirDryBulbC;          // coil inlet air dry-bulb temperature (C)
    Real64 CondInletTemp(0.0);        // condenser inlet air temperature (C)
    Real64 CondInletHumRat;           // condenser inlet air humidity ratio (kg/kg)
    Real64 OutdoorDryBulb;            // outdoor dry-bulb temperature (C)
    Real64 OutdoorHumRat;             // outdoor humidity ratio (kg/kg)
    Real64 OutdoorPressure;           // outdoor pressure (Pa)
    Real64 OutdoorWetBulb;            // outdoor wet-bulb temperature (C)
    Real64 CoolOABoundary;            // output of cooling boundary curve (outdoor temperature, C)
    Real64 HeatOABoundary;            // output of heating boundary curve (outdoor temperature, C)
    Real64 EIRFPLRModFac;             // EIRFPLR curve output
    Real64 UpperStageCompressorRatio; // used for crankcase heater power calculation
    Real64 RhoAir;                    // Density of air [kg/m3]
    Real64 RhoWater;                  // Density of water [kg/m3]
    Real64 CpCond;                    // Specific Heat of water [J/kg-k]
    Real64 CondAirMassFlow;           // Condenser air mass flow rate [kg/s]
    Real64 CondWaterMassFlow;         // Condenser water mass flow rate [kg/s]
    Real64 PartLoadFraction;          // Part load fraction from PLFFPLR curve
    Real64 VRFRTF;                    // VRF runtime fraction when cycling below MINPLR
    Real64 OutdoorCoilT;              // Outdoor coil temperature (C)
    Real64 OutdoorCoildw;             // Outdoor coil delta w assuming coil temp of OutdoorCoilT (kg/kg)
    Real64 FractionalDefrostTime;     // Fraction of time step system is in defrost
    Real64 HeatingCapacityMultiplier; // Multiplier for heating capacity when system is in defrost
    Real64 InputPowerMultiplier;      // Multiplier for power when system is in defrost
    Real64 LoadDueToDefrost;          // Additional load due to defrost
    Real64 DefrostEIRTempModFac;      // EIR modifier for defrost (function of entering drybulb, outside wetbulb)
    Real64 HRCAPFTConst;              // stead-state capacity fraction
    Real64 HRInitialCapFrac;          // Fractional cooling degradation at the start of heat recovery from cooling mode
    Real64 HRCapTC;                   // Time constant used to recover from initial degradation in cooling heat recovery
    Real64 HREIRFTConst;              // stead-state EIR fraction
    Real64 HRInitialEIRFrac;          // Fractional cooling degradation at the start of heat recovery from cooling mode
    Real64 HREIRTC;                   // Time constant used to recover from initial degradation in cooling heat recovery
    Real64 CurrentEndTime;            // end time of current time step
    Real64 SUMultiplier;              // multiplier for simulating mode changes
    Real64 CondPower;                 // condenser power [W]
    Real64 CondCapacity;              // condenser heat rejection [W]
    Real64 CondOutletTemp;            // Outlet temperature from VRF condenser [C]
    Real64 TotPower;                  // total condenser power use [W]
    bool HRHeatRequestFlag;           // flag indicating VRF TU could operate in heating mode
    bool HRCoolRequestFlag;           // flag indicating VRF TU could operate in cooling mode

    auto &vrf = state.dataHVACVarRefFlow->VRF(VRFCond);

    // variable initializations
    int TUListNum = vrf.ZoneTUListPtr;
    int NumTUInList = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).NumTUInList;
    int NumTUInCoolingMode = 0;            // number of terminal units actually cooling
    int NumTUInHeatingMode = 0;            // number of terminal units actually heating
    Real64 TUCoolingLoad = 0.0;            // sum of TU's cooling coil load (W)
    Real64 TUHeatingLoad = 0.0;            // sum of TU's heating coil load (W)
    Real64 TUParasiticPower = 0.0;         // total terminal unit parasitic power (W)
    Real64 TUFanPower = 0.0;               // total terminal unit fan power (W)
    Real64 CoolingPLR = 0.0;               // condenser cooling PLR
    Real64 HeatingPLR = 0.0;               // condenser heating PLR
    Real64 CyclingRatio = 1.0;             // cycling ratio of condenser's compressors
    Real64 SumCoolInletWB = 0.0;           // sum of active TU's DX cooling coil inlet air wet-bulb temperature
    Real64 SumHeatInletDB = 0.0;           // sum of active TU's DX heating coil inlet air dry-bulb temperature
    Real64 SumHeatInletWB = 0.0;           // sum of active TU's DX heating coil inlet air wet-bulb temperature
    Real64 TotalCondCoolingCapacity = 0.0; // total available condenser cooling capacity (W)
    Real64 TotalCondHeatingCapacity = 0.0; // total available condenser heating capacity (W)
    Real64 TotalTUCoolingCapacity = 0.0;   // sum of TU's cooling capacity including piping losses (W)
    Real64 TotalTUHeatingCapacity = 0.0;   // sum of TU's heating capacity including piping losses (W)

    vrf.ElecCoolingPower = 0.0;
    vrf.ElecHeatingPower = 0.0;
    vrf.CrankCaseHeaterPower = 0.0;
    vrf.EvapCondPumpElecPower = 0.0;
    vrf.EvapWaterConsumpRate = 0.0;
    vrf.DefrostPower = 0.0;
    vrf.OperatingCoolingCOP = 0.0;
    vrf.OperatingHeatingCOP = 0.0;
    vrf.OperatingCOP = 0.0;
    vrf.SCHE = 0.0;
    vrf.BasinHeaterPower = 0.0;
    vrf.VRFHeatRec = 0.0;

    // set condenser entering air conditions
    if (vrf.CondenserNodeNum != 0) {
        OutdoorDryBulb = state.dataLoopNodes->Node(vrf.CondenserNodeNum).Temp;
        if (vrf.CondenserType != DataHeatBalance::RefrigCondenserType::Water) {
            OutdoorHumRat = state.dataLoopNodes->Node(vrf.CondenserNodeNum).HumRat;
            OutdoorPressure = state.dataLoopNodes->Node(vrf.CondenserNodeNum).Press;
            OutdoorWetBulb = state.dataLoopNodes->Node(vrf.CondenserNodeNum).OutAirWetBulb;
        } else {
            OutdoorHumRat = state.dataEnvrn->OutHumRat;
            OutdoorPressure = state.dataEnvrn->OutBaroPress;
            OutdoorWetBulb = state.dataEnvrn->OutWetBulbTemp;
        }
    } else {
        OutdoorDryBulb = state.dataEnvrn->OutDryBulbTemp;
        OutdoorHumRat = state.dataEnvrn->OutHumRat;
        OutdoorPressure = state.dataEnvrn->OutBaroPress;
        OutdoorWetBulb = state.dataEnvrn->OutWetBulbTemp;
    }

    if (vrf.CondenserType == DataHeatBalance::RefrigCondenserType::Air) {
        CondInletTemp = OutdoorDryBulb; // Outdoor dry-bulb temp
    } else if (vrf.CondenserType == DataHeatBalance::RefrigCondenserType::Evap) {
        RhoAir = PsyRhoAirFnPbTdbW(state, OutdoorPressure, OutdoorDryBulb, OutdoorHumRat);
        CondAirMassFlow = RhoAir * vrf.EvapCondAirVolFlowRate;
        // (Outdoor wet-bulb temp from DataEnvironment) + (1.0-EvapCondEffectiveness) * (drybulb - wetbulb)
        CondInletTemp = OutdoorWetBulb + (OutdoorDryBulb - OutdoorWetBulb) * (1.0 - vrf.EvapCondEffectiveness);
        CondInletHumRat = PsyWFnTdbTwbPb(state, CondInletTemp, OutdoorWetBulb, OutdoorPressure);
    } else if (vrf.CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
        CondInletTemp = OutdoorDryBulb; // node inlet temp from above
        OutdoorWetBulb = CondInletTemp; // for watercooled
        CondWaterMassFlow = vrf.WaterCondenserDesignMassFlow;
    } else {
        assert(false);
    }
    vrf.CondenserInletTemp = CondInletTemp;

    // sum loads on TU coils
    for (NumTU = 1; NumTU <= NumTUInList; ++NumTU) {
        TUCoolingLoad += state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU);
        TUHeatingLoad += state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(NumTU);
    }

    vrf.TUCoolingLoad = TUCoolingLoad;
    vrf.TUHeatingLoad = TUHeatingLoad;

    // no need to do anything else if the terminal units are off
    if (TUCoolingLoad == 0.0 && TUHeatingLoad == 0.0) {
        vrf.SUMultiplier = 0.0;
        vrf.VRFCondPLR = 0.0;
        vrf.VRFCondRTF = 0.0;
        vrf.VRFCondCyclingRatio = 0.0;
        vrf.QCondenser = 0.0;
        vrf.TotalCoolingCapacity = 0.0;
        vrf.TotalHeatingCapacity = 0.0;
        vrf.OperatingMode = 0.0;
        vrf.HRHeatingActive = false;
        vrf.HRCoolingActive = false;
        state.dataHVACVarRefFlow->CurrentEndTimeLast = double((state.dataGlobal->DayOfSim - 1) * 24) + state.dataGlobal->CurrentTime -
                                                       state.dataGlobal->TimeStepZone + state.dataHVACGlobal->SysTimeElapsed;
        if (vrf.CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
            state.dataHVACVarRefFlow->CondenserWaterMassFlowRate = 0.0;
            SetComponentFlowRate(
                state, state.dataHVACVarRefFlow->CondenserWaterMassFlowRate, vrf.CondenserNodeNum, vrf.CondenserOutletNodeNum, vrf.SourcePlantLoc);
            vrf.WaterCondenserMassFlow = state.dataHVACVarRefFlow->CondenserWaterMassFlowRate;
            vrf.CondenserSideOutletTemp = CondInletTemp;
        }
        return;
    }

    // switch modes if summed coil capacity shows opposite operating mode
    // if total TU heating exceeds total TU cooling * ( 1 + 1/COP) then system is in heating mode
    if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond) && TUHeatingLoad > (TUCoolingLoad * (1.0 + 1.0 / vrf.CoolingCOP))) {
        state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = true;
        state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
        vrf.ModeChange = true;
        if (!state.dataHVACVarRefFlow->LastModeHeating(VRFCond)) {
            state.dataHVACVarRefFlow->LastModeHeating(VRFCond) = true;
            // reset heat recovery startup timer
            vrf.HRTimer = 0.0;
            vrf.HRHeatingActive = false;
            vrf.HRCoolingActive = false;
        }
    } else if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond) && (TUCoolingLoad * (1.0 + 1.0 / vrf.CoolingCOP)) > TUHeatingLoad) {
        state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = true;
        state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
        vrf.ModeChange = true;
        if (!state.dataHVACVarRefFlow->LastModeCooling(VRFCond)) {
            state.dataHVACVarRefFlow->LastModeCooling(VRFCond) = true;
            // reset heat recovery startup timer
            vrf.HRTimer = 0.0;
            vrf.HRHeatingActive = false;
            vrf.HRCoolingActive = false;
        }
    } else if (TUCoolingLoad > 0.0 && TUHeatingLoad > 0.0 &&
               ((state.dataHVACVarRefFlow->CoolingLoad(VRFCond) && state.dataHVACVarRefFlow->LastModeHeating(VRFCond)) ||
                (state.dataHVACVarRefFlow->HeatingLoad(VRFCond) && state.dataHVACVarRefFlow->LastModeCooling(VRFCond)))) {
        vrf.ModeChange = true;
        // reset heat recovery startup timer
        vrf.HRTimer = 0.0;
        vrf.HRHeatingActive = false;
        vrf.HRCoolingActive = false;
    }

    // loop through TU's and calculate average inlet conditions for active coils
    for (NumTU = 1; NumTU <= NumTUInList; ++NumTU) {
        int TUIndex = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
        int CoolCoilIndex = state.dataHVACVarRefFlow->VRFTU(TUIndex).CoolCoilIndex;
        int HeatCoilIndex = state.dataHVACVarRefFlow->VRFTU(TUIndex).HeatCoilIndex;
        TUParasiticPower +=
            state.dataHVACVarRefFlow->VRFTU(TUIndex).ParasiticCoolElecPower + state.dataHVACVarRefFlow->VRFTU(TUIndex).ParasiticHeatElecPower;
        TUFanPower += state.dataHVACVarRefFlow->VRFTU(TUIndex).FanPower;

        if (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) > 0.0) {
            SumCoolInletWB += state.dataDXCoils->DXCoilCoolInletAirWBTemp(CoolCoilIndex) *
                              state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) / TUCoolingLoad;
            ++NumTUInCoolingMode;
        }
        if (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) > 0.0) {
            SumHeatInletDB += state.dataDXCoils->DXCoilHeatInletAirDBTemp(HeatCoilIndex) *
                              state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) / TUHeatingLoad;
            SumHeatInletWB += state.dataDXCoils->DXCoilHeatInletAirWBTemp(HeatCoilIndex) *
                              state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) / TUHeatingLoad;
            ++NumTUInHeatingMode;
        }
    }

    bool CoolingCoilAvailableFlag = any(state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).CoolingCoilAvailable);
    bool HeatingCoilAvailableFlag = any(state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).HeatingCoilAvailable);

    // calculate capacities and energy use
    if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond) && CoolingCoilAvailableFlag) {
        InletAirWetBulbC = SumCoolInletWB;
        TotCoolCapTempModFac = CurveValue(state, vrf.CoolCapFT, InletAirWetBulbC, CondInletTemp);
        TotCoolEIRTempModFac = CurveValue(state, vrf.CoolEIRFT, InletAirWetBulbC, CondInletTemp);

        // recalculate cooling Cap and EIR curve output if using boundary curve along with dual Cap and EIR curves.
        if (vrf.CoolBoundaryCurvePtr > 0) {
            CoolOABoundary = CurveValue(state, vrf.CoolBoundaryCurvePtr, InletAirWetBulbC);
            if (OutdoorDryBulb > CoolOABoundary) {
                if (vrf.CoolCapFTHi > 0) {
                    TotCoolCapTempModFac = CurveValue(state, vrf.CoolCapFTHi, InletAirWetBulbC, CondInletTemp);
                }
            }
        }
        if (vrf.EIRCoolBoundaryCurvePtr > 0) {
            CoolOABoundary = CurveValue(state, vrf.EIRCoolBoundaryCurvePtr, InletAirWetBulbC);
            if (OutdoorDryBulb > CoolOABoundary) {
                if (vrf.CoolEIRFTHi > 0) {
                    TotCoolEIRTempModFac = CurveValue(state, vrf.CoolEIRFTHi, InletAirWetBulbC, CondInletTemp);
                }
            }
        }

        //   Warn user if curve output goes negative
        if (TotCoolCapTempModFac < 0.0) {
            if (!state.dataGlobal->WarmupFlag && NumTUInCoolingMode > 0) {
                if (vrf.CoolCapFTErrorIndex == 0) {
                    ShowSevereMessage(state, format("{} \"{}\":", cVRFTypes(VRF_HeatPump), vrf.Name));
                    ShowContinueError(
                        state,
                        format(" Cooling Capacity Modifier curve (function of temperature) output is negative ({:.3T}).", TotCoolCapTempModFac));
                    ShowContinueError(state,
                                      format(" Negative value occurs using an outdoor air temperature of {:.1T} C and an average indoor air "
                                             "wet-bulb temperature of {:.1T} C.",
                                             CondInletTemp,
                                             InletAirWetBulbC));
                    ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(
                    state,
                    format("{} \"{}\": Cooling Capacity Modifier curve (function of temperature) output is negative warning continues...",
                           PlantEquipTypeNames[static_cast<int>(PlantEquipmentType::HeatPumpVRF)],
                           vrf.Name),
                    vrf.CoolCapFTErrorIndex,
                    TotCoolCapTempModFac,
                    TotCoolCapTempModFac);
                TotCoolCapTempModFac = 0.0;
            }
        }

        //   Warn user if curve output goes negative
        if (TotCoolEIRTempModFac < 0.0) {
            if (!state.dataGlobal->WarmupFlag && NumTUInCoolingMode > 0) {
                if (vrf.EIRFTempCoolErrorIndex == 0) {
                    ShowSevereMessage(state, format("{} \"{}\":", cVRFTypes(VRF_HeatPump), vrf.Name));
                    ShowContinueError(state,
                                      format(" Cooling Energy Input Ratio Modifier curve (function of temperature) output is negative ({:.3T}).",
                                             TotCoolEIRTempModFac));
                    ShowContinueError(state,
                                      format(" Negative value occurs using an outdoor air temperature of {:.1T} C and an average indoor air "
                                             "wet-bulb temperature of {:.1T} C.",
                                             CondInletTemp,
                                             InletAirWetBulbC));
                    ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(
                    state,
                    format("{} \"{}\": Cooling Energy Input Ratio Modifier curve (function of temperature) output is negative warning continues...",
                           PlantEquipTypeNames[static_cast<int>(PlantEquipmentType::HeatPumpVRF)],
                           vrf.Name),
                    vrf.EIRFTempCoolErrorIndex,
                    TotCoolEIRTempModFac,
                    TotCoolEIRTempModFac);
                TotCoolEIRTempModFac = 0.0;
            }
        }

        TotalCondCoolingCapacity = vrf.CoolingCapacity * state.dataHVACVarRefFlow->CoolCombinationRatio(VRFCond) * TotCoolCapTempModFac;
        TotalTUCoolingCapacity = TotalCondCoolingCapacity * vrf.PipingCorrectionCooling;

        if (TotalCondCoolingCapacity > 0.0) {
            CoolingPLR = (TUCoolingLoad / vrf.PipingCorrectionCooling) / TotalCondCoolingCapacity;
        } else {
            CoolingPLR = 0.0;
        }

        //   Warn user if curve output goes negative
        if (TotCoolCapTempModFac < 0.0) {
            if (!state.dataGlobal->WarmupFlag && NumTUInCoolingMode > 0) {
                if (vrf.CoolCapFTErrorIndex == 0) {
                    ShowSevereMessage(state, format("{} \"{}\":", cVRFTypes(VRF_HeatPump), vrf.Name));
                    ShowContinueError(
                        state,
                        format(" Cooling Capacity Modifier curve (function of temperature) output is negative ({:.3T}).", TotCoolCapTempModFac));
                    ShowContinueError(state,
                                      format(" Negative value occurs using an outdoor air temperature of {:.1T} C and an average indoor air "
                                             "wet-bulb temperature of {:.1T} C.",
                                             CondInletTemp,
                                             InletAirWetBulbC));
                    ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(
                    state,
                    format("{} \"{}\": Cooling Capacity Modifier curve (function of temperature) output is negative warning continues...",
                           PlantEquipTypeNames[static_cast<int>(PlantEquipmentType::HeatPumpVRF)],
                           vrf.Name),
                    vrf.CoolCapFTErrorIndex,
                    TotCoolCapTempModFac,
                    TotCoolCapTempModFac);
                TotCoolCapTempModFac = 0.0;
            }
        }
        //   Warn user if curve output goes negative
        if (TotCoolEIRTempModFac < 0.0) {
            if (!state.dataGlobal->WarmupFlag && NumTUInCoolingMode > 0) {
                if (vrf.EIRFTempCoolErrorIndex == 0) {
                    ShowSevereMessage(state, format("{} \"{}\":", cVRFTypes(VRF_HeatPump), vrf.Name));
                    ShowContinueError(state,
                                      format(" Cooling Energy Input Ratio Modifier curve (function of temperature) output is negative ({:.3T}).",
                                             TotCoolEIRTempModFac));
                    ShowContinueError(state,
                                      format(" Negative value occurs using an outdoor air temperature of {:.1T} C and an average indoor air "
                                             "wet-bulb temperature of {:.1T} C.",
                                             CondInletTemp,
                                             InletAirWetBulbC));
                    ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(
                    state,
                    format("{} \"{}\": Cooling Energy Input Ratio Modifier curve (function of temperature) output is negative warning continues...",
                           PlantEquipTypeNames[static_cast<int>(PlantEquipmentType::HeatPumpVRF)],
                           vrf.Name),
                    vrf.EIRFTempCoolErrorIndex,
                    TotCoolEIRTempModFac,
                    TotCoolEIRTempModFac);
                TotCoolEIRTempModFac = 0.0;
            }
        }

    } else if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond) && HeatingCoilAvailableFlag) {
        InletAirDryBulbC = SumHeatInletDB;
        InletAirWetBulbC = SumHeatInletWB;
        switch (vrf.HeatingPerformanceOATType) {
        case HVAC::OATType::DryBulb: {
            TotHeatCapTempModFac = CurveValue(state, vrf.HeatCapFT, InletAirDryBulbC, CondInletTemp);
            TotHeatEIRTempModFac = CurveValue(state, vrf.HeatEIRFT, InletAirDryBulbC, CondInletTemp);
        } break;
        case HVAC::OATType::WetBulb: {
            TotHeatCapTempModFac = CurveValue(state, vrf.HeatCapFT, InletAirDryBulbC, OutdoorWetBulb);
            TotHeatEIRTempModFac = CurveValue(state, vrf.HeatEIRFT, InletAirDryBulbC, OutdoorWetBulb);
        } break;
        default: {
            TotHeatCapTempModFac = 1.0;
            TotHeatEIRTempModFac = 1.0;
        } break;
        }
        // recalculate heating Cap and EIR curve output if using boundary curve along with dual Cap and EIR curves.
        if (vrf.HeatBoundaryCurvePtr > 0) {
            HeatOABoundary = CurveValue(state, vrf.HeatBoundaryCurvePtr, InletAirDryBulbC);
            switch (vrf.HeatingPerformanceOATType) {
            case HVAC::OATType::DryBulb: {
                if (OutdoorDryBulb > HeatOABoundary) {
                    if (vrf.HeatCapFTHi > 0) {
                        TotHeatCapTempModFac = CurveValue(state, vrf.HeatCapFTHi, InletAirDryBulbC, CondInletTemp);
                    }
                }
            } break;
            case HVAC::OATType::WetBulb: {
                if (OutdoorWetBulb > HeatOABoundary) {
                    if (vrf.HeatCapFTHi > 0) {
                        TotHeatCapTempModFac = CurveValue(state, vrf.HeatCapFTHi, InletAirDryBulbC, OutdoorWetBulb);
                    }
                }
            } break;
            default: {
                TotHeatCapTempModFac = 1.0;
            } break;
            }
        }
        if (vrf.EIRHeatBoundaryCurvePtr > 0) {
            HeatOABoundary = CurveValue(state, vrf.EIRHeatBoundaryCurvePtr, InletAirDryBulbC);
            switch (vrf.HeatingPerformanceOATType) {
            case HVAC::OATType::DryBulb: {
                if (OutdoorDryBulb > HeatOABoundary) {
                    if (vrf.HeatEIRFTHi > 0) {
                        TotHeatEIRTempModFac = CurveValue(state, vrf.HeatEIRFTHi, InletAirDryBulbC, CondInletTemp);
                    }
                }
            } break;
            case HVAC::OATType::WetBulb: {
                if (OutdoorWetBulb > HeatOABoundary) {
                    if (vrf.HeatEIRFTHi > 0) {
                        TotHeatEIRTempModFac = CurveValue(state, vrf.HeatEIRFTHi, InletAirDryBulbC, OutdoorWetBulb);
                    }
                }
            } break;
            default: {
                TotHeatEIRTempModFac = 1.0;
            } break;
            }
        }

        //   Warn user if curve output goes negative
        if (TotHeatCapTempModFac < 0.0) {
            if (!state.dataGlobal->WarmupFlag && NumTUInHeatingMode > 0) {
                if (vrf.HeatCapFTErrorIndex == 0) {
                    ShowSevereMessage(state, format("{} \"{}\":", cVRFTypes(VRF_HeatPump), vrf.Name));
                    ShowContinueError(
                        state,
                        format(" Heating Capacity Modifier curve (function of temperature) output is negative ({:.3T}).", TotHeatCapTempModFac));

                    switch (vrf.HeatingPerformanceOATType) {
                    case HVAC::OATType::DryBulb: {
                        ShowContinueError(state,
                                          format(" Negative value occurs using an outdoor air temperature of {:.1T} C and an average indoor air "
                                                 "dry-bulb temperature of {:.1T} C.",
                                                 CondInletTemp,
                                                 InletAirDryBulbC));
                    } break;
                    case HVAC::OATType::WetBulb: {
                        ShowContinueError(state,
                                          format(" Negative value occurs using an outdoor air wet-bulb temperature of {:.1T} C and an average "
                                                 "indoor air wet-bulb temperature of {:.1T} C.",
                                                 OutdoorWetBulb,
                                                 InletAirWetBulbC));
                    } break;
                    default:
                        // should never get here
                        break;
                    }

                    ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(
                    state,
                    format("{} \"{}\": Heating Capacity Ratio Modifier curve (function of temperature) output is negative warning continues...",
                           PlantEquipTypeNames[static_cast<int>(PlantEquipmentType::HeatPumpVRF)],
                           vrf.Name),
                    vrf.HeatCapFTErrorIndex,
                    TotHeatCapTempModFac,
                    TotHeatCapTempModFac);
                TotHeatCapTempModFac = 0.0;
            }
        }
        //   Warn user if curve output goes negative
        if (TotHeatEIRTempModFac < 0.0) {
            if (!state.dataGlobal->WarmupFlag && NumTUInHeatingMode > 0) {
                if (vrf.EIRFTempHeatErrorIndex == 0) {
                    ShowSevereMessage(state, format("{} \"{}\":", cVRFTypes(VRF_HeatPump), vrf.Name));
                    ShowContinueError(state,
                                      format(" Heating Energy Input Ratio Modifier curve (function of temperature) output is negative ({:.3T}).",
                                             TotHeatEIRTempModFac));
                    switch (vrf.HeatingPerformanceOATType) {
                    case HVAC::OATType::DryBulb: {
                        ShowContinueError(state,
                                          format(" Negative value occurs using an outdoor air dry-bulb temperature of {:.1T} C and an "
                                                 "average indoor air dry-bulb temperature of {:.1T} C.",
                                                 CondInletTemp,
                                                 InletAirDryBulbC));
                    } break;
                    case HVAC::OATType::WetBulb: {
                        ShowContinueError(state,
                                          format(" Negative value occurs using an outdoor air wet-bulb temperature of {:.1T} C and an "
                                                 "average indoor air wet-bulb temperature of {:.1T} C.",
                                                 OutdoorWetBulb,
                                                 InletAirWetBulbC));
                    } break;
                    default:
                        break;
                    }
                    ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
                }
                ShowRecurringWarningErrorAtEnd(
                    state,
                    format("{} \"{}\": Heating Energy Input Ratio Modifier curve (function of temperature) output is negative warning continues...",
                           PlantEquipTypeNames[static_cast<int>(PlantEquipmentType::HeatPumpVRF)],
                           vrf.Name),
                    vrf.EIRFTempHeatErrorIndex,
                    TotHeatEIRTempModFac,
                    TotHeatEIRTempModFac);
                TotHeatEIRTempModFac = 0.0;
            }
        }

        // Initializing defrost adjustment factors
        LoadDueToDefrost = 0.0;
        HeatingCapacityMultiplier = 1.0;
        FractionalDefrostTime = 0.0;
        InputPowerMultiplier = 1.0;

        // Check outdoor temperature to determine of defrost is active
        if (OutdoorDryBulb <= vrf.MaxOATDefrost && vrf.CondenserType != DataHeatBalance::RefrigCondenserType::Water) {

            // Calculating adjustment factors for defrost
            // Calculate delta w through outdoor coil by assuming a coil temp of 0.82*DBT-9.7(F) per DOE2.1E
            OutdoorCoilT = 0.82 * OutdoorDryBulb - 8.589;
            OutdoorCoildw = max(1.0e-6, (OutdoorHumRat - PsyWFnTdpPb(state, OutdoorCoilT, OutdoorPressure)));

            // Calculate defrost adjustment factors depending on defrost control type
            if (vrf.DefrostControl == StandardRatings::HPdefrostControl::Timed) {
                FractionalDefrostTime = vrf.DefrostFraction;
                if (FractionalDefrostTime > 0.0) {
                    HeatingCapacityMultiplier = 0.909 - 107.33 * OutdoorCoildw;
                    InputPowerMultiplier = 0.90 - 36.45 * OutdoorCoildw;
                }
            } else { // else defrost control is on-demand
                FractionalDefrostTime = 1.0 / (1.0 + 0.01446 / OutdoorCoildw);
                HeatingCapacityMultiplier = 0.875 * (1.0 - FractionalDefrostTime);
                InputPowerMultiplier = 0.954 * (1.0 - FractionalDefrostTime);
            }

            if (FractionalDefrostTime > 0.0) {
                // Calculate defrost adjustment factors depending on defrost control strategy
                if (vrf.DefrostStrategy == StandardRatings::DefrostStrat::ReverseCycle) {
                    LoadDueToDefrost = (0.01 * FractionalDefrostTime) * (7.222 - OutdoorDryBulb) * (vrf.HeatingCapacity / 1.01667);
                    DefrostEIRTempModFac = CurveValue(state, vrf.DefrostEIRPtr, max(15.555, InletAirWetBulbC), max(15.555, OutdoorDryBulb));

                    //         Warn user if curve output goes negative
                    if (DefrostEIRTempModFac < 0.0) {
                        if (!state.dataGlobal->WarmupFlag) {
                            if (vrf.DefrostHeatErrorIndex == 0) {
                                ShowSevereMessage(state, format("{} \"{}\":", cVRFTypes(VRF_HeatPump), vrf.Name));
                                ShowContinueError(
                                    state,
                                    format(" Defrost Energy Input Ratio Modifier curve (function of temperature) output is negative ({:.3T}).",
                                           DefrostEIRTempModFac));
                                ShowContinueError(state,
                                                  format(" Negative value occurs using an outdoor air dry-bulb temperature of {:.1T} C and an "
                                                         "average indoor air wet-bulb temperature of {:.1T} C.",
                                                         OutdoorDryBulb,
                                                         InletAirWetBulbC));
                                ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
                            }
                            ShowRecurringWarningErrorAtEnd(state,
                                                           format("{} \"{}\": Defrost Energy Input Ratio Modifier curve (function of temperature) "
                                                                  "output is negative warning continues...",
                                                                  PlantEquipTypeNames[static_cast<int>(PlantEquipmentType::HeatPumpVRF)],
                                                                  vrf.Name),
                                                           vrf.DefrostHeatErrorIndex,
                                                           DefrostEIRTempModFac,
                                                           DefrostEIRTempModFac);
                            DefrostEIRTempModFac = 0.0;
                        }
                    }

                    vrf.DefrostPower = DefrostEIRTempModFac * (vrf.HeatingCapacity / 1.01667) * FractionalDefrostTime;

                } else { // Defrost strategy is resistive
                    vrf.DefrostPower = vrf.DefrostCapacity * FractionalDefrostTime;
                }
            }
        }

        TotalCondHeatingCapacity =
            vrf.HeatingCapacity * state.dataHVACVarRefFlow->HeatCombinationRatio(VRFCond) * TotHeatCapTempModFac * HeatingCapacityMultiplier;
        TotalTUHeatingCapacity = TotalCondHeatingCapacity * vrf.PipingCorrectionHeating;
        if (TotalCondHeatingCapacity > 0.0) {
            HeatingPLR = (TUHeatingLoad / vrf.PipingCorrectionHeating) / TotalCondHeatingCapacity;
            HeatingPLR += (LoadDueToDefrost * HeatingPLR) / TotalCondHeatingCapacity;
        } else {
            HeatingPLR = 0.0;
        }
    }

    vrf.VRFCondPLR = max(CoolingPLR, HeatingPLR);
    Real64 tmpVRFCondPLR = 0.0;
    if (CoolingPLR > 0.0 || HeatingPLR > 0.0) {
        tmpVRFCondPLR = max(vrf.MinPLR, vrf.VRFCondPLR);
    }

    HRHeatRequestFlag = any(state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).HRHeatRequest);
    HRCoolRequestFlag = any(state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).HRCoolRequest);
    HREIRFTConst = 1.0;
    Real64 HREIRAdjustment = 1.0;

    if (!state.dataGlobal->DoingSizing && !state.dataGlobal->WarmupFlag) {
        if (HRHeatRequestFlag && HRCoolRequestFlag) {
            // determine operating mode change
            if (!vrf.HRCoolingActive && !vrf.HRHeatingActive) {
                vrf.ModeChange = true;
                vrf.HRTimer = 0.0;
            }
            if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond)) {
                if (vrf.HRHeatingActive && !vrf.HRCoolingActive) {
                    vrf.HRModeChange = true;
                }
                vrf.HRCoolingActive = true;
                vrf.HRHeatingActive = false;
                int HRCAPFT = vrf.HRCAPFTCool; // Index to cool capacity as a function of temperature\PLR curve for heat recovery
                if (HRCAPFT > 0) {
                    //         VRF(VRFCond)%HRCAPFTCoolConst = 0.9d0 ! initialized to 0.9
                    if (state.dataCurveManager->curves(vrf.HRCAPFTCool)->numDims == 2) { // Curve type for HRCAPFTCool
                        vrf.HRCAPFTCoolConst = CurveValue(state, HRCAPFT, InletAirWetBulbC, CondInletTemp);
                    } else {
                        vrf.HRCAPFTCoolConst = CurveValue(state, HRCAPFT, tmpVRFCondPLR);
                    }
                }
                HRCAPFTConst = vrf.HRCAPFTCoolConst;
                HRInitialCapFrac = vrf.HRInitialCoolCapFrac; // Fractional cooling degradation at the start of heat recovery from cooling mode
                HRCapTC = vrf.HRCoolCapTC;                   // Time constant used to recover from initial degradation in cooling heat recovery

                int HREIRFT = vrf.HREIRFTCool; // Index to cool EIR as a function of temperature curve for heat recovery
                if (HREIRFT > 0) {
                    //         VRF(VRFCond)%HREIRFTCoolConst = 1.1d0 ! initialized to 1.1
                    if (state.dataCurveManager->curves(vrf.HREIRFTCool)->numDims == 2) { // Curve type for HREIRFTCool
                        vrf.HREIRFTCoolConst = CurveValue(state, HREIRFT, InletAirWetBulbC, CondInletTemp);
                    } else {
                        vrf.HREIRFTCoolConst = CurveValue(state, HREIRFT, tmpVRFCondPLR);
                    }
                }
                HREIRFTConst = vrf.HREIRFTCoolConst;
                HRInitialEIRFrac = vrf.HRInitialCoolEIRFrac; // Fractional cooling degradation at the start of heat recovery from cooling mode
                HREIRTC = vrf.HRCoolEIRTC;                   // Time constant used to recover from initial degradation in cooling heat recovery
            } else if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) {
                if (!vrf.HRHeatingActive && vrf.HRCoolingActive) {
                    vrf.HRModeChange = true;
                }
                vrf.HRCoolingActive = false;
                vrf.HRHeatingActive = true;
                int HRCAPFT = vrf.HRCAPFTHeat; // Index to heat capacity as a function of temperature\PLR curve for heat recovery
                if (HRCAPFT > 0) {
                    //         VRF(VRFCond)%HRCAPFTHeatConst = 1.1d0 ! initialized to 1.1
                    if (state.dataCurveManager->curves(vrf.HRCAPFTHeat)->numDims == 2) { // Curve type for HRCAPFTCool
                        switch (vrf.HeatingPerformanceOATType) {
                        case HVAC::OATType::DryBulb: {
                            vrf.HRCAPFTHeatConst = CurveValue(state, HRCAPFT, InletAirDryBulbC, CondInletTemp);
                        } break;
                        case HVAC::OATType::WetBulb: {
                            vrf.HRCAPFTHeatConst = CurveValue(state, HRCAPFT, InletAirDryBulbC, OutdoorWetBulb);
                        } break;
                        default: {
                            vrf.HRCAPFTHeatConst = 1.0;
                        } break;
                        }
                    } else {
                        vrf.HRCAPFTHeatConst = CurveValue(state, HRCAPFT, tmpVRFCondPLR);
                    }
                }
                HRCAPFTConst = vrf.HRCAPFTHeatConst;
                HRInitialCapFrac = vrf.HRInitialHeatCapFrac; // Fractional heating degradation at the start of heat recovery from cooling mode
                HRCapTC = vrf.HRHeatCapTC;                   // Time constant used to recover from initial degradation in heating heat recovery

                int HREIRFT = vrf.HREIRFTHeat; // Index to cool EIR as a function of temperature curve for heat recovery
                if (HREIRFT > 0) {
                    //         VRF(VRFCond)%HREIRFTCoolConst = 1.1d0 ! initialized to 1.1
                    if (state.dataCurveManager->curves(vrf.HREIRFTHeat)->numDims == 2) { // Curve type for HREIRFTHeat
                        switch (vrf.HeatingPerformanceOATType) {
                        case HVAC::OATType::DryBulb: {
                            vrf.HREIRFTHeatConst = CurveValue(state, HREIRFT, InletAirDryBulbC, CondInletTemp);
                        } break;
                        case HVAC::OATType::WetBulb: {
                            vrf.HREIRFTHeatConst = CurveValue(state, HREIRFT, InletAirDryBulbC, OutdoorWetBulb);
                        } break;
                        default: {
                            vrf.HREIRFTHeatConst = 1.0;
                        } break;
                        }
                    } else {
                        vrf.HREIRFTHeatConst = CurveValue(state, HREIRFT, tmpVRFCondPLR);
                    }
                }
                HREIRFTConst = vrf.HREIRFTHeatConst;
                HRInitialEIRFrac = vrf.HRInitialHeatEIRFrac; // Fractional heating degradation at the start of heat recovery from heating mode
                HREIRTC = vrf.HRHeatEIRTC;                   // Time constant used to recover from initial degradation in heating heat recovery
            } else {
                //   zone thermostats satisfied, condenser is off. Set values anyway
                HRCAPFTConst = 1.0;
                HRInitialCapFrac = 1.0;
                HRCapTC = 1.0;
                HREIRFTConst = 1.0;
                HRInitialEIRFrac = 1.0;
                HREIRTC = 1.0;
                if (vrf.HRHeatingActive || vrf.HRCoolingActive) {
                    vrf.HRModeChange = true;
                }
                vrf.HRCoolingActive = false;
                vrf.HRHeatingActive = false;
            }

        } else { // IF(HRHeatRequestFlag .AND. HRCoolRequestFlag)THEN -- Heat recovery turned off
            HRCAPFTConst = 1.0;
            HRInitialCapFrac = 1.0;
            HRCapTC = 0.0;
            HREIRFTConst = 1.0;
            HRInitialEIRFrac = 1.0;
            HREIRTC = 0.0;
            vrf.HRModeChange = false;
            vrf.HRCoolingActive = false;
            vrf.HRHeatingActive = false;
            vrf.HRTimer = 0.0;
        }

        // Calculate the capacity modification factor (SUMultiplier) for the HR mode transition period
        CurrentEndTime = double((state.dataGlobal->DayOfSim - 1) * 24) + state.dataGlobal->CurrentTime - state.dataGlobal->TimeStepZone +
                         state.dataHVACGlobal->SysTimeElapsed;

        if (vrf.ModeChange || vrf.HRModeChange) {
            if (vrf.HRCoolingActive && vrf.HRTimer == 0.0) {
                vrf.HRTimer = state.dataHVACVarRefFlow->CurrentEndTimeLast;
            } else if (vrf.HRHeatingActive && vrf.HRTimer == 0.0) {
                vrf.HRTimer = state.dataHVACVarRefFlow->CurrentEndTimeLast;
            } else if (!vrf.HRCoolingActive && !vrf.HRHeatingActive) {
                vrf.HRTimer = 0.0;
            }
        }

        vrf.HRTime = max(0.0, CurrentEndTime - vrf.HRTimer);
        if (vrf.HRTime < (HRCapTC * 5.0)) {
            if (HRCapTC > 0.0) {
                SUMultiplier = min(1.0, 1.0 - std::exp(-vrf.HRTime / HRCapTC));
            } else {
                SUMultiplier = 1.0;
            }
        } else {
            SUMultiplier = 1.0;
            vrf.ModeChange = false;
            vrf.HRModeChange = false;
        }
        vrf.SUMultiplier = SUMultiplier;

        state.dataHVACVarRefFlow->CurrentEndTimeLast = CurrentEndTime;

        if (vrf.HeatRecoveryUsed && vrf.HRCoolingActive) {
            TotalCondCoolingCapacity *= HRCAPFTConst;
            TotalCondCoolingCapacity =
                HRInitialCapFrac * TotalCondCoolingCapacity + (1.0 - HRInitialCapFrac) * TotalCondCoolingCapacity * SUMultiplier;
            TotalTUCoolingCapacity = TotalCondCoolingCapacity * vrf.PipingCorrectionCooling;
            if (TotalCondCoolingCapacity > 0.0) {
                CoolingPLR = min(1.0, (TUCoolingLoad / vrf.PipingCorrectionCooling) / TotalCondCoolingCapacity);
            } else {
                CoolingPLR = 0.0;
            }
            HREIRAdjustment = HRInitialEIRFrac + (HREIRFTConst - HRInitialEIRFrac) * SUMultiplier;
            vrf.VRFHeatRec = TUHeatingLoad;
        } else if (vrf.HeatRecoveryUsed && vrf.HRHeatingActive) {
            TotalCondHeatingCapacity *= HRCAPFTConst;
            TotalCondHeatingCapacity =
                HRInitialCapFrac * TotalCondHeatingCapacity + (1.0 - HRInitialCapFrac) * TotalCondHeatingCapacity * SUMultiplier;
            TotalTUHeatingCapacity = TotalCondHeatingCapacity * vrf.PipingCorrectionHeating;
            if (TotalCondHeatingCapacity > 0.0) {
                HeatingPLR = min(1.0, (TUHeatingLoad / vrf.PipingCorrectionHeating) / TotalCondHeatingCapacity);
            } else {
                HeatingPLR = 0.0;
            }
            HREIRAdjustment = HRInitialEIRFrac + (HREIRFTConst - HRInitialEIRFrac) * SUMultiplier;
            vrf.VRFHeatRec = TUCoolingLoad;
        }
        vrf.VRFCondPLR = max(CoolingPLR, HeatingPLR);
    }

    if (vrf.MinPLR > 0.0) {
        CyclingRatio = min(1.0, vrf.VRFCondPLR / vrf.MinPLR);
        if (vrf.VRFCondPLR < vrf.MinPLR && vrf.VRFCondPLR > 0.0) {
            vrf.VRFCondPLR = vrf.MinPLR;
            if (CoolingPLR > 0.0) {
                CoolingPLR = vrf.MinPLR; // also adjust local PLR variables
            }
            if (HeatingPLR > 0.0) {
                HeatingPLR = vrf.MinPLR; // also adjust local PLR variables
            }
        }
    }
    vrf.VRFCondCyclingRatio = CyclingRatio; // report variable for cycling rate
    vrf.TotalCoolingCapacity = TotalCondCoolingCapacity * CoolingPLR * CyclingRatio;
    vrf.TotalHeatingCapacity = TotalCondHeatingCapacity * HeatingPLR * CyclingRatio;

    vrf.OperatingMode = 0; // report variable for heating or cooling mode
    EIRFPLRModFac = 1.0;
    VRFRTF = 0.0;
    // cooling and heating is optional (only one may exist), if so then performance curve for missing coil are not required
    if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond) && CoolingPLR > 0.0) {
        vrf.OperatingMode = ModeCoolingOnly;
        if (CoolingPLR > 1.0) {
            if (vrf.CoolEIRFPLR2 > 0) {
                EIRFPLRModFac = CurveValue(state, vrf.CoolEIRFPLR2, max(vrf.MinPLR, CoolingPLR));
            }
        } else {
            if (vrf.CoolEIRFPLR1 > 0) {
                EIRFPLRModFac = CurveValue(state, vrf.CoolEIRFPLR1, max(vrf.MinPLR, CoolingPLR));
            }
        }

        if (EIRFPLRModFac < 0.0) {
            if (vrf.CoolEIRFPLRErrorIndex == 0) {
                ShowSevereMessage(state, fmt::format("{} \"{}\":", std::string(cVRFTypes(VRF_HeatPump)), vrf.Name));
                ShowContinueError(state, format(" Cooling EIR Modifier curve (function of PLR) output is negative ({:.3T}).", EIRFPLRModFac));
                ShowContinueError(state, format(" Negative value occurs using a cooling Part Load Ratio (PLR) of {:.2T}.", CoolingPLR));
                ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
            }
            ShowRecurringWarningErrorAtEnd(
                state,
                fmt::format("{} \"{}\": Cooling EIR Modifier curve (function of PLR) output is negative warning continues...",
                            PlantEquipTypeNames[static_cast<int>(PlantEquipmentType::HeatPumpVRF)],
                            vrf.Name),
                vrf.CoolEIRFPLRErrorIndex,
                EIRFPLRModFac,
                EIRFPLRModFac);
            EIRFPLRModFac = 0.0;
        }

        // find part load fraction to calculate RTF
        if (vrf.CoolPLFFPLR > 0) {
            PartLoadFraction = max(0.7, CurveValue(state, vrf.CoolPLFFPLR, CyclingRatio));
        } else {
            PartLoadFraction = 1.0;
        }
        VRFRTF = min(1.0, (CyclingRatio / PartLoadFraction));

        vrf.ElecCoolingPower = (vrf.RatedCoolingPower * TotCoolCapTempModFac) * TotCoolEIRTempModFac * EIRFPLRModFac * HREIRAdjustment * VRFRTF;
    }
    if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond) && HeatingPLR > 0.0) {
        vrf.OperatingMode = ModeHeatingOnly;
        if (HeatingPLR > 1.0) {
            if (vrf.HeatEIRFPLR2 > 0) {
                EIRFPLRModFac = CurveValue(state, vrf.HeatEIRFPLR2, max(vrf.MinPLR, HeatingPLR));
            }
        } else {
            if (vrf.HeatEIRFPLR1 > 0) {
                EIRFPLRModFac = CurveValue(state, vrf.HeatEIRFPLR1, max(vrf.MinPLR, HeatingPLR));
            }
        }

        if (EIRFPLRModFac < 0.0) {
            if (vrf.HeatEIRFPLRErrorIndex == 0) {
                ShowSevereMessage(state, fmt::format("{} \"{}\":", std::string(cVRFTypes(VRF_HeatPump)), vrf.Name));
                ShowContinueError(state, format(" Heating EIR Modifier curve (function of PLR) output is negative ({:.3T}).", EIRFPLRModFac));
                ShowContinueError(state, format(" Negative value occurs using a heating Part Load Ratio (PLR) of {:.2T}.", HeatingPLR));
                ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
            }
            ShowRecurringWarningErrorAtEnd(
                state,
                fmt::format("{} \"{}\": Heating EIR Modifier curve (function of PLR) output is negative warning continues...",
                            PlantEquipTypeNames[static_cast<int>(PlantEquipmentType::HeatPumpVRF)],
                            vrf.Name),
                vrf.HeatEIRFPLRErrorIndex,
                EIRFPLRModFac,
                EIRFPLRModFac);
            EIRFPLRModFac = 0.0;
        }

        // find part load fraction to calculate RTF
        if (vrf.HeatPLFFPLR > 0) {
            PartLoadFraction = max(0.7, CurveValue(state, vrf.HeatPLFFPLR, CyclingRatio));
        } else {
            PartLoadFraction = 1.0;
        }
        VRFRTF = min(1.0, (CyclingRatio / PartLoadFraction));

        vrf.ElecHeatingPower =
            (vrf.RatedHeatingPower * TotHeatCapTempModFac) * TotHeatEIRTempModFac * EIRFPLRModFac * HREIRAdjustment * VRFRTF * InputPowerMultiplier;
    }
    // adjust defrost power based on RTF
    vrf.DefrostPower *= VRFRTF;
    vrf.VRFCondRTF = VRFRTF;

    // calculate crankcase heater power
    if (vrf.MaxOATCCHeater > OutdoorDryBulb) {
        // calculate crankcase heater power
        vrf.CrankCaseHeaterPower = vrf.CCHeaterPower * (1.0 - VRFRTF);
        if (vrf.NumCompressors > 1) {
            UpperStageCompressorRatio = (1.0 - vrf.CompressorSizeRatio) / (vrf.NumCompressors - 1);
            for (int Stage = 1; Stage <= vrf.NumCompressors - 2; ++Stage) {
                if (vrf.VRFCondPLR < (vrf.CompressorSizeRatio + Stage * UpperStageCompressorRatio)) {
                    vrf.CrankCaseHeaterPower += vrf.CCHeaterPower;
                }
            }
        }
    } else {
        vrf.CrankCaseHeaterPower = 0.0;
    }

    CondCapacity = max(vrf.TotalCoolingCapacity, vrf.TotalHeatingCapacity);
    CondPower = max(vrf.ElecCoolingPower, vrf.ElecHeatingPower);
    if (vrf.ElecCoolingPower > 0.0) {
        vrf.QCondenser = CondCapacity + CondPower - vrf.TUHeatingLoad / vrf.PipingCorrectionHeating;
    } else if (vrf.ElecHeatingPower > 0.0) {
        vrf.QCondenser = -CondCapacity + CondPower + vrf.TUCoolingLoad / vrf.PipingCorrectionCooling;
    } else {
        vrf.QCondenser = 0.0;
    }

    if (vrf.CondenserType == DataHeatBalance::RefrigCondenserType::Evap) {
        // Calculate basin heater power
        CalcBasinHeaterPower(state, vrf.BasinHeaterPowerFTempDiff, vrf.basinHeaterSched, vrf.BasinHeaterSetPointTemp, vrf.BasinHeaterPower);
        vrf.BasinHeaterPower *= (1.0 - VRFRTF);

        // calculate evaporative condenser pump power and water consumption
        if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond) && CoolingPLR > 0.0) {
            //******************
            // WATER CONSUMPTION IN m3 OF WATER FOR DIRECT
            // H2O [m3/s] = Delta W[kgWater/kgDryAir]*Mass Flow Air[kgDryAir/s]
            //                    /RhoWater [kgWater/m3]
            //******************
            RhoWater = RhoH2O(OutdoorDryBulb);
            vrf.EvapWaterConsumpRate = (CondInletHumRat - OutdoorHumRat) * CondAirMassFlow / RhoWater * vrf.VRFCondPLR;
            vrf.EvapCondPumpElecPower = vrf.EvapCondPumpPower * VRFRTF;
        }
    } else if (vrf.CondenserType == DataHeatBalance::RefrigCondenserType::Water) {

        if (CondCapacity > 0.0) {
            state.dataHVACVarRefFlow->CondenserWaterMassFlowRate = CondWaterMassFlow;
        } else {
            state.dataHVACVarRefFlow->CondenserWaterMassFlowRate = 0.0;
        }
        SetComponentFlowRate(
            state, state.dataHVACVarRefFlow->CondenserWaterMassFlowRate, vrf.CondenserNodeNum, vrf.CondenserOutletNodeNum, vrf.SourcePlantLoc);

        // should be the same as above just entering this function
        //            VRF( VRFCond ).CondenserInletTemp = state.dataLoopNodes->Node(VRF(VRFCond).CondenserNodeNum).Temp;
        vrf.WaterCondenserMassFlow = state.dataLoopNodes->Node(vrf.CondenserNodeNum).MassFlowRate;

        CpCond = state.dataPlnt->PlantLoop(vrf.SourcePlantLoc.loopNum).glycol->getSpecificHeat(state, vrf.CondenserInletTemp, RoutineName);
        if (CondWaterMassFlow > 0.0) {
            CondOutletTemp = vrf.QCondenser / (CondWaterMassFlow * CpCond) + CondInletTemp;
        } else {
            CondOutletTemp = CondInletTemp;
        }
        vrf.CondenserSideOutletTemp = CondOutletTemp;
    }

    // calculate operating COP
    if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond) && CoolingPLR > 0.0) {
        if (vrf.ElecCoolingPower != 0.0) {
            // this calc should use delivered capacity, not condenser capacity, use VRF(VRFCond)%TUCoolingLoad
            vrf.OperatingCoolingCOP =
                (vrf.TotalCoolingCapacity) / (vrf.ElecCoolingPower + vrf.CrankCaseHeaterPower + vrf.EvapCondPumpElecPower + vrf.DefrostPower);
        } else {
            vrf.OperatingCoolingCOP = 0.0;
        }
    }
    if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond) && HeatingPLR > 0.0) {
        if (vrf.ElecHeatingPower != 0.0) {
            // this calc should use delivered capacity, not condenser capacity, use VRF(VRFCond)%TUHeatingLoad
            vrf.OperatingHeatingCOP =
                (vrf.TotalHeatingCapacity) / (vrf.ElecHeatingPower + vrf.CrankCaseHeaterPower + vrf.EvapCondPumpElecPower + vrf.DefrostPower);
        } else {
            vrf.OperatingHeatingCOP = 0.0;
        }
    }

    TotPower = TUParasiticPower + TUFanPower + vrf.ElecHeatingPower + vrf.ElecCoolingPower + vrf.CrankCaseHeaterPower + vrf.EvapCondPumpElecPower +
               vrf.DefrostPower;
    if (TotPower > 0.0) {
        vrf.OperatingCOP = (vrf.TUCoolingLoad + vrf.TUHeatingLoad) / TotPower;
        vrf.SCHE = vrf.OperatingCOP * 3.412141633; // see StandardRatings::ConvFromSIToIP
    }

    // limit the TU capacity when the condenser is maxed out on capacity
    // I think this next line will make the max cap report variable match the coil objects, will probably change the answer though
    //  IF(CoolingLoad(VRFCond) .AND. NumTUInCoolingMode .GT. 0 .AND. MaxCoolingCapacity(VRFCond) == MaxCap)THEN
    if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond) && NumTUInCoolingMode > 0) {

        //   IF TU capacity is greater than condenser capacity find maximum allowed TU capacity (i.e., conserve energy)
        if (TUCoolingLoad > TotalTUCoolingCapacity) {
            LimitTUCapacity(state,
                            VRFCond,
                            NumTUInList,
                            TotalTUCoolingCapacity,
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad,
                            state.dataHVACVarRefFlow->MaxCoolingCapacity(VRFCond),
                            TotalTUHeatingCapacity,
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad,
                            state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond));
        }
    } else if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond) && NumTUInHeatingMode > 0) {
        //   IF TU capacity is greater than condenser capacity
        if (TUHeatingLoad > TotalTUHeatingCapacity) {
            LimitTUCapacity(state,
                            VRFCond,
                            NumTUInList,
                            TotalTUHeatingCapacity,
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad,
                            state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond),
                            TotalTUCoolingCapacity,
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad,
                            state.dataHVACVarRefFlow->MaxCoolingCapacity(VRFCond));
        }
    } else {
    }
}

void GetVRFInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   August 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Manages GetInput processing and program termination

    // METHODOLOGY EMPLOYED:
    // Calls "Get" routines to read in data.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetVRFInput: "); // include trailing blank space

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool ErrorsFound(false); // If errors detected in input

    GetVRFInputData(state, ErrorsFound);

    if (ErrorsFound) {
        ShowFatalError(
            state,
            format("{}Errors found in getting AirConditioner:VariableRefrigerantFlow system input. Preceding condition(s) causes termination.",
                   RoutineName));
    }
}

void GetVRFInputData(EnergyPlusData &state, bool &ErrorsFound)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   August 2010
    //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Obtains input data for VRF systems and stores it in data structures

    using namespace DataLoopNode;
    using BranchNodeConnections::SetUpCompSets;
    using BranchNodeConnections::TestCompSet;
    using Curve::checkCurveIsNormalizedToOne;
    using Curve::CurveValue;
    using Curve::GetCurveIndex;
    using DXCoils::GetDXCoilIndex;

    using DataSizing::AutoSize;
    using DXCoils::GetCoilCondenserInletNode;
    using DXCoils::GetCoilTypeNum;
    using DXCoils::GetDXCoilCapFTCurveIndex;
    using DXCoils::GetDXCoilName;
    using DXCoils::RatedInletAirTempHeat;
    using DXCoils::RatedInletWetBulbTemp;
    using DXCoils::RatedOutdoorAirTemp;
    using DXCoils::RatedOutdoorAirTempHeat;
    using DXCoils::RatedOutdoorWetBulbTempHeat;
    using DXCoils::SetDXCoolingCoilData;
    using MixedAir::GetOAMixerNodeNumbers;
    using NodeInputManager::GetOnlySingleNode;
    using OutAirNodeManager::CheckOutAirNodeNumber;
    using SingleDuct::GetATMixer;
    using WaterManager::SetupTankDemandComponent;
    using WaterManager::SetupTankSupplyComponent;

    static constexpr std::string_view RoutineName("GetVRFInput: "); // include trailing blank space
    static constexpr std::string_view routineName = "GetVRFInput";

    std::string cCurrentModuleObject;

    Array1D_int OANodeNums(4); // Node numbers of OA mixer (OA, EA, RA, MA)

    // InputProcessor routines
    int NumParams = 0; // Number of arguments
    int NumAlphas = 0; // Number of alpha arguments
    int NumNums = 0;   // Number of real arguments
    int IOStat;        // Status
    bool errFlag;      // error flag for mining functions
    bool IsNotOK;      // Flag to verify name

    // Compute for allocation
    int MaxAlphas = 0;
    int MaxNumbers = 0;

    {
        // Terminal Units

        // The number of VRF constant volume TUs (anticipating different types of TU's)
        int NumVRFCTU = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow");
        if (NumVRFCTU > 0) {
            state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
                state, "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow", NumParams, NumAlphas, NumNums);
            MaxAlphas = max(MaxAlphas, NumAlphas);
            MaxNumbers = max(MaxNumbers, NumNums);
        }

        state.dataHVACVarRefFlow->NumVRFTU = NumVRFCTU;
        if (state.dataHVACVarRefFlow->NumVRFTU > 0) {
            state.dataHVACVarRefFlow->VRFTU.allocate(state.dataHVACVarRefFlow->NumVRFTU);
            state.dataHVACVarRefFlow->CheckEquipName.dimension(state.dataHVACVarRefFlow->NumVRFTU, true);
            state.dataHVACVarRefFlow->VRFTUNumericFields.allocate(state.dataHVACVarRefFlow->NumVRFTU);
        }
    }

    {
        // VRF AirConditioners

        state.dataHVACVarRefFlow->NumVRFCond_SysCurve =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirConditioner:VariableRefrigerantFlow");
        if (state.dataHVACVarRefFlow->NumVRFCond_SysCurve > 0) {
            state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
                state, "AirConditioner:VariableRefrigerantFlow", NumParams, NumAlphas, NumNums);
            MaxAlphas = max(MaxAlphas, NumAlphas);
            MaxNumbers = max(MaxNumbers, NumNums);
        }

        state.dataHVACVarRefFlow->NumVRFCond_FluidTCtrl_HP =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl");
        if (state.dataHVACVarRefFlow->NumVRFCond_FluidTCtrl_HP > 0) {
            state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
                state, "AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl", NumParams, NumAlphas, NumNums);
            MaxAlphas = max(MaxAlphas, NumAlphas);
            MaxNumbers = max(MaxNumbers, NumNums);
        }

        state.dataHVACVarRefFlow->NumVRFCond_FluidTCtrl_HR =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl:HR");
        if (state.dataHVACVarRefFlow->NumVRFCond_FluidTCtrl_HR > 0) {
            state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
                state, "AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl:HR", NumParams, NumAlphas, NumNums);
            MaxAlphas = max(MaxAlphas, NumAlphas);
            MaxNumbers = max(MaxNumbers, NumNums);
        }

        state.dataHVACVarRefFlow->NumVRFCond = state.dataHVACVarRefFlow->NumVRFCond_SysCurve + state.dataHVACVarRefFlow->NumVRFCond_FluidTCtrl_HP +
                                               state.dataHVACVarRefFlow->NumVRFCond_FluidTCtrl_HR;

        if (state.dataHVACVarRefFlow->NumVRFCond > 0) {
            state.dataHVACVarRefFlow->VRF.allocate(state.dataHVACVarRefFlow->NumVRFCond);
            state.dataHVACVarRefFlow->VrfUniqueNames.reserve(static_cast<unsigned>(state.dataHVACVarRefFlow->NumVRFCond));
            state.dataHVACVarRefFlow->MaxCoolingCapacity.allocate(state.dataHVACVarRefFlow->NumVRFCond);
            state.dataHVACVarRefFlow->MaxHeatingCapacity.allocate(state.dataHVACVarRefFlow->NumVRFCond);
            state.dataHVACVarRefFlow->CoolCombinationRatio.allocate(state.dataHVACVarRefFlow->NumVRFCond);
            state.dataHVACVarRefFlow->HeatCombinationRatio.allocate(state.dataHVACVarRefFlow->NumVRFCond);
            state.dataHVACVarRefFlow->MaxCoolingCapacity = Constant::MaxCap;
            state.dataHVACVarRefFlow->MaxHeatingCapacity = Constant::MaxCap;
            state.dataHVACVarRefFlow->CoolCombinationRatio = 1.0;
            state.dataHVACVarRefFlow->HeatCombinationRatio = 1.0;
        }
    }

    {
        // ZoneTerminalUnitList

        state.dataHVACVarRefFlow->NumVRFTULists = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneTerminalUnitList");
        if (state.dataHVACVarRefFlow->NumVRFTULists > 0) {
            state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "ZoneTerminalUnitList", NumParams, NumAlphas, NumNums);
            MaxAlphas = max(MaxAlphas, NumAlphas);
            MaxNumbers = max(MaxNumbers, NumNums);

            state.dataHVACVarRefFlow->TerminalUnitList.allocate(state.dataHVACVarRefFlow->NumVRFTULists);
        }
    }

    Array1D_string cAlphaFieldNames;
    cAlphaFieldNames.allocate(MaxAlphas);
    Array1D_string cAlphaArgs;
    cAlphaArgs.allocate(MaxAlphas);
    Array1D_bool lAlphaFieldBlanks;
    lAlphaFieldBlanks.dimension(MaxAlphas, false);

    Array1D_string cNumericFieldNames;
    cNumericFieldNames.allocate(MaxNumbers);
    Array1D<Real64> rNumericArgs;
    rNumericArgs.dimension(MaxNumbers, 0.0);
    Array1D_bool lNumericFieldBlanks;
    lNumericFieldBlanks.dimension(MaxNumbers, false);

    // read all terminal unit list objects
    cCurrentModuleObject = "ZoneTerminalUnitList";
    for (int TUListNum = 1; TUListNum <= state.dataHVACVarRefFlow->NumVRFTULists; ++TUListNum) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 TUListNum,
                                                                 cAlphaArgs,
                                                                 NumAlphas,
                                                                 rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericFieldBlanks,
                                                                 lAlphaFieldBlanks,
                                                                 cAlphaFieldNames,
                                                                 cNumericFieldNames);
        Util::IsNameEmpty(state, cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

        auto &thisTUList = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum);
        thisTUList.Name = cAlphaArgs(1);
        thisTUList.NumTUInList = NumAlphas - 1;
        thisTUList.ZoneTUPtr.allocate(thisTUList.NumTUInList);
        thisTUList.ZoneTUName.allocate(thisTUList.NumTUInList);
        thisTUList.IsSimulated.allocate(thisTUList.NumTUInList);
        thisTUList.TotalCoolLoad.allocate(thisTUList.NumTUInList);
        thisTUList.TotalHeatLoad.allocate(thisTUList.NumTUInList);
        thisTUList.CoolingCoilPresent.allocate(thisTUList.NumTUInList);
        thisTUList.HeatingCoilPresent.allocate(thisTUList.NumTUInList);
        thisTUList.TerminalUnitNotSizedYet.allocate(thisTUList.NumTUInList);
        thisTUList.HRHeatRequest.allocate(thisTUList.NumTUInList);
        thisTUList.HRCoolRequest.allocate(thisTUList.NumTUInList);
        thisTUList.CoolingCoilAvailable.allocate(thisTUList.NumTUInList);
        thisTUList.HeatingCoilAvailable.allocate(thisTUList.NumTUInList);
        thisTUList.coolingCoilAvailScheds.allocate(thisTUList.NumTUInList);
        thisTUList.heatingCoilAvailScheds.allocate(thisTUList.NumTUInList);
        thisTUList.ZoneTUPtr = 0;
        thisTUList.IsSimulated = false;
        thisTUList.TotalCoolLoad = 0.0;
        thisTUList.TotalHeatLoad = 0.0;
        thisTUList.CoolingCoilPresent = true;
        thisTUList.HeatingCoilPresent = true;
        thisTUList.TerminalUnitNotSizedYet = true;
        thisTUList.HRHeatRequest = false;
        thisTUList.HRCoolRequest = false;
        thisTUList.CoolingCoilAvailable = false;
        thisTUList.HeatingCoilAvailable = false;
        thisTUList.coolingCoilAvailScheds = nullptr;
        thisTUList.heatingCoilAvailScheds = nullptr;

        for (int TUNum = 1; TUNum <= thisTUList.NumTUInList; ++TUNum) {
            thisTUList.ZoneTUName(TUNum) = cAlphaArgs(TUNum + 1);
        }
    }

    auto checkCurveMinMaxOutput = [&state](int CurveIndex) -> std::array<Real64, 4> {
        Real64 MinCurveVal = 999.0;
        Real64 MaxCurveVal = -999.0;
        Real64 MinCurvePLR = 0.0;
        Real64 MaxCurvePLR = 1.0;

        for (int i = 0; i <= 100; ++i) { // 0 to 1.0 with 0.01 increment
            const Real64 CurveInput = i / 100.0;
            Real64 CurveVal = CurveValue(state, CurveIndex, CurveInput);
            if (CurveVal < MinCurveVal) {
                MinCurveVal = CurveVal;
                MinCurvePLR = CurveInput;
            }
            if (CurveVal > MaxCurveVal) {
                MaxCurveVal = CurveVal;
                MaxCurvePLR = CurveInput;
            }
        }
        return {MinCurvePLR, MinCurveVal, MaxCurvePLR, MaxCurveVal};
    };

    // read all VRF condenser objects: Algorithm Type 1_system curve based model
    cCurrentModuleObject = "AirConditioner:VariableRefrigerantFlow";
    for (int VRFNum = 1; VRFNum <= state.dataHVACVarRefFlow->NumVRFCond_SysCurve; ++VRFNum) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 VRFNum,
                                                                 cAlphaArgs,
                                                                 NumAlphas,
                                                                 rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericFieldBlanks,
                                                                 lAlphaFieldBlanks,
                                                                 cAlphaFieldNames,
                                                                 cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};
        GlobalNames::VerifyUniqueInterObjectName(
            state, state.dataHVACVarRefFlow->VrfUniqueNames, cAlphaArgs(1), cCurrentModuleObject, cAlphaFieldNames(1), ErrorsFound);

        auto &thisVrfSys = state.dataHVACVarRefFlow->VRF(VRFNum);
        thisVrfSys.Name = cAlphaArgs(1);
        thisVrfSys.VRFSystemTypeNum = VRF_HeatPump;
        thisVrfSys.VRFAlgorithmType = AlgorithmType::SysCurve;

        if (lAlphaFieldBlanks(2)) {
            thisVrfSys.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisVrfSys.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
            ErrorsFound = true;
        }

        thisVrfSys.CoolingCapacity = rNumericArgs(1);
        thisVrfSys.CoolingCOP = rNumericArgs(2);
        thisVrfSys.MinOATCooling = rNumericArgs(3);
        thisVrfSys.MaxOATCooling = rNumericArgs(4);

        thisVrfSys.CoolCapFT = GetCurveIndex(state, cAlphaArgs(3));
        if (thisVrfSys.CoolCapFT > 0) {
            // Verify Curve Object, only legal type is biquadratic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.CoolCapFT, // Curve index
                                                 {2},                  // Valid dimensions
                                                 RoutineName,          // Routine name
                                                 cCurrentModuleObject, // Object Type
                                                 thisVrfSys.Name,      // Object Name
                                                 cAlphaFieldNames(3)); // Field Name

            if (!ErrorsFound) {
                checkCurveIsNormalizedToOne(state,
                                            std::string{RoutineName} + cCurrentModuleObject,
                                            thisVrfSys.Name,
                                            thisVrfSys.CoolCapFT,
                                            cAlphaFieldNames(3),
                                            cAlphaArgs(3),
                                            RatedInletWetBulbTemp,
                                            RatedOutdoorAirTemp);
            }
        }

        thisVrfSys.CoolBoundaryCurvePtr = GetCurveIndex(state, cAlphaArgs(4));
        if (thisVrfSys.CoolBoundaryCurvePtr > 0) {
            // Verify Curve Object, only legal type is linear, quadratic, or cubic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.CoolBoundaryCurvePtr, // Curve index
                                                 {1},                             // Valid dimensions
                                                 RoutineName,                     // Routine name
                                                 cCurrentModuleObject,            // Object Type
                                                 thisVrfSys.Name,                 // Object Name
                                                 cAlphaFieldNames(4));            // Field Name
        }

        thisVrfSys.CoolCapFTHi = GetCurveIndex(state, cAlphaArgs(5));
        if (thisVrfSys.CoolCapFTHi > 0) {
            // Verify Curve Object, only legal type is biquadratic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.CoolCapFTHi, // Curve index
                                                 {2},                    // Valid dimensions
                                                 RoutineName,            // Routine name
                                                 cCurrentModuleObject,   // Object Type
                                                 thisVrfSys.Name,        // Object Name
                                                 cAlphaFieldNames(5));   // Field Name
        }

        thisVrfSys.CoolEIRFT = GetCurveIndex(state, cAlphaArgs(6));
        if (thisVrfSys.CoolEIRFT > 0) {
            // Verify Curve Object, only legal type is biquadratic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.CoolEIRFT, // Curve index
                                                 {2},                  // Valid dimensions
                                                 RoutineName,          // Routine name
                                                 cCurrentModuleObject, // Object Type
                                                 thisVrfSys.Name,      // Object Name
                                                 cAlphaFieldNames(6)); // Field Name
        }

        thisVrfSys.EIRCoolBoundaryCurvePtr = GetCurveIndex(state, cAlphaArgs(7));
        if (thisVrfSys.EIRCoolBoundaryCurvePtr > 0) {
            // Verify Curve Object, only legal type is linear, quadratic, or cubic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.EIRCoolBoundaryCurvePtr, // Curve index
                                                 {1},                                // Valid dimensions
                                                 RoutineName,                        // Routine name
                                                 cCurrentModuleObject,               // Object Type
                                                 thisVrfSys.Name,                    // Object Name
                                                 cAlphaFieldNames(7));               // Field Name
        }

        thisVrfSys.CoolEIRFTHi = GetCurveIndex(state, cAlphaArgs(8));
        if (thisVrfSys.CoolEIRFTHi > 0) {
            // Verify Curve Object, only legal type is biquadratic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.CoolEIRFTHi, // Curve index
                                                 {2},                    // Valid dimensions
                                                 RoutineName,            // Routine name
                                                 cCurrentModuleObject,   // Object Type
                                                 thisVrfSys.Name,        // Object Name
                                                 cAlphaFieldNames(8));   // Field Name
        }

        thisVrfSys.CoolEIRFPLR1 = GetCurveIndex(state, cAlphaArgs(9));
        if (thisVrfSys.CoolEIRFPLR1 > 0) {
            // Verify Curve Object, only legal type is linear, quadratic, or cubic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.CoolEIRFPLR1, // Curve index
                                                 {1},                     // Valid dimensions
                                                 RoutineName,             // Routine name
                                                 cCurrentModuleObject,    // Object Type
                                                 thisVrfSys.Name,         // Object Name
                                                 cAlphaFieldNames(9));    // Field Name
        }

        thisVrfSys.CoolEIRFPLR2 = GetCurveIndex(state, cAlphaArgs(10));
        if (thisVrfSys.CoolEIRFPLR2 > 0) {
            // Verify Curve Object, only legal type is linear, quadratic, or cubic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.CoolEIRFPLR2, // Curve index
                                                 {1},                     // Valid dimensions
                                                 RoutineName,             // Routine name
                                                 cCurrentModuleObject,    // Object Type
                                                 thisVrfSys.Name,         // Object Name
                                                 cAlphaFieldNames(10));   // Field Name
        }

        thisVrfSys.CoolCombRatioPTR = GetCurveIndex(state, cAlphaArgs(11));
        if (thisVrfSys.CoolCombRatioPTR > 0) {
            // Verify Curve Object, only legal type is linear, quadratic, or cubic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.CoolCombRatioPTR, // Curve index
                                                 {1},                         // Valid dimensions
                                                 RoutineName,                 // Routine name
                                                 cCurrentModuleObject,        // Object Type
                                                 thisVrfSys.Name,             // Object Name
                                                 cAlphaFieldNames(11));       // Field Name
        }

        thisVrfSys.CoolPLFFPLR = GetCurveIndex(state, cAlphaArgs(12));
        if (thisVrfSys.CoolPLFFPLR > 0) {
            // Verify Curve Object, only legal type is linear, quadratic, or cubic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.CoolPLFFPLR, // Curve index
                                                 {1},                    // Valid dimensions
                                                 RoutineName,            // Routine name
                                                 cCurrentModuleObject,   // Object Type
                                                 thisVrfSys.Name,        // Object Name
                                                 cAlphaFieldNames(12));  // Field Name
            if (!ErrorsFound) {
                //     Test PLF curve minimum and maximum. Cap if less than 0.7 or greater than 1.0.
                auto [MinCurvePLR, MinCurveVal, MaxCurvePLR, MaxCurveVal] = checkCurveMinMaxOutput(thisVrfSys.CoolPLFFPLR);

                if (MinCurveVal < 0.7) {
                    ShowWarningError(state, format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisVrfSys.Name));
                    ShowContinueError(state, format("...{}=\"{}\" has out of range values.", cAlphaFieldNames(12), cAlphaArgs(12)));
                    ShowContinueError(state,
                                      format("...Curve minimum must be >= 0.7, curve min at PLR = {:.2T} is {:.3T}", MinCurvePLR, MinCurveVal));
                    ShowContinueError(state, "...Setting curve minimum to 0.7 and simulation continues.");
                    Curve::SetCurveOutputMinValue(state, thisVrfSys.CoolPLFFPLR, ErrorsFound, 0.7);
                }

                if (MaxCurveVal > 1.0) {
                    ShowWarningError(state, format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisVrfSys.Name));
                    ShowContinueError(state, format("...{}=\"{}\" has out of range values.", cAlphaFieldNames(12), cAlphaArgs(12)));
                    ShowContinueError(state,
                                      format("...Curve maximum must be <= 1.0, curve max at PLR = {:.2T} is {:.3T}", MaxCurvePLR, MaxCurveVal));
                    ShowContinueError(state, "...Setting curve maximum to 1.0 and simulation continues.");
                    Curve::SetCurveOutputMaxValue(state, thisVrfSys.CoolPLFFPLR, ErrorsFound, 1.0);
                }
            }
        }

        thisVrfSys.HeatingCapacity = rNumericArgs(5);
        thisVrfSys.HeatingCapacitySizeRatio = rNumericArgs(6);
        if (!lNumericFieldBlanks(6) && thisVrfSys.HeatingCapacity == AutoSize) {
            thisVrfSys.LockHeatingCapacity = true;
        }
        thisVrfSys.HeatingCOP = rNumericArgs(7);
        thisVrfSys.MinOATHeating = rNumericArgs(8);
        thisVrfSys.MaxOATHeating = rNumericArgs(9);
        if (thisVrfSys.MinOATHeating >= thisVrfSys.MaxOATHeating) {
            ShowSevereError(state, format("{}, \"{}\"", cCurrentModuleObject, thisVrfSys.Name));
            ShowContinueError(state,
                              format("... {} ({:.3T}) must be less than maximum ({:.3T}).",
                                     cNumericFieldNames(8),
                                     thisVrfSys.MinOATHeating,
                                     thisVrfSys.MaxOATHeating));
            ErrorsFound = true;
        }

        thisVrfSys.HeatCapFT = GetCurveIndex(state, cAlphaArgs(13));
        if (thisVrfSys.HeatCapFT > 0) {
            // Verify Curve Object, only legal type is biquadratic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.HeatCapFT,  // Curve index
                                                 {2},                   // Valid dimensions
                                                 RoutineName,           // Routine name
                                                 cCurrentModuleObject,  // Object Type
                                                 thisVrfSys.Name,       // Object Name
                                                 cAlphaFieldNames(13)); // Field Name

            if (!ErrorsFound) {
                if (Util::SameString(cAlphaArgs(19), "WETBULBTEMPERATURE")) {
                    checkCurveIsNormalizedToOne(state,
                                                std::string{RoutineName} + cCurrentModuleObject,
                                                thisVrfSys.Name,
                                                thisVrfSys.HeatCapFT,
                                                cAlphaFieldNames(13),
                                                cAlphaArgs(13),
                                                RatedInletAirTempHeat,
                                                RatedOutdoorWetBulbTempHeat);
                } else if (Util::SameString(cAlphaArgs(19), "DRYBULBTEMPERATURE")) {
                    checkCurveIsNormalizedToOne(state,
                                                std::string{RoutineName} + cCurrentModuleObject,
                                                thisVrfSys.Name,
                                                thisVrfSys.HeatCapFT,
                                                cAlphaFieldNames(13),
                                                cAlphaArgs(13),
                                                RatedInletAirTempHeat,
                                                RatedOutdoorAirTempHeat);
                }
            }
        }

        thisVrfSys.HeatBoundaryCurvePtr = GetCurveIndex(state, cAlphaArgs(14));
        if (thisVrfSys.HeatBoundaryCurvePtr > 0) {
            // Verify Curve Object, only legal type is linear, quadratic, or cubic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.HeatBoundaryCurvePtr, // Curve index
                                                 {1},                             // Valid dimensions
                                                 RoutineName,                     // Routine name
                                                 cCurrentModuleObject,            // Object Type
                                                 thisVrfSys.Name,                 // Object Name
                                                 cAlphaFieldNames(14));           // Field Name
        }

        thisVrfSys.HeatCapFTHi = GetCurveIndex(state, cAlphaArgs(15));
        if (thisVrfSys.HeatCapFTHi > 0) {
            // Verify Curve Object, only legal type is biquadratic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.HeatCapFTHi, // Curve index
                                                 {2},                    // Valid dimensions
                                                 RoutineName,            // Routine name
                                                 cCurrentModuleObject,   // Object Type
                                                 thisVrfSys.Name,        // Object Name
                                                 cAlphaFieldNames(15));  // Field Name
        }

        thisVrfSys.HeatEIRFT = GetCurveIndex(state, cAlphaArgs(16));
        if (thisVrfSys.HeatEIRFT > 0) {
            // Verify Curve Object, only legal type is biquadratic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.HeatEIRFT,  // Curve index
                                                 {2},                   // Valid dimensions
                                                 RoutineName,           // Routine name
                                                 cCurrentModuleObject,  // Object Type
                                                 thisVrfSys.Name,       // Object Name
                                                 cAlphaFieldNames(16)); // Field Name
        }

        thisVrfSys.EIRHeatBoundaryCurvePtr = GetCurveIndex(state, cAlphaArgs(17));
        if (thisVrfSys.EIRHeatBoundaryCurvePtr > 0) {
            // Verify Curve Object, only legal type is linear, quadratic, or cubic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.EIRHeatBoundaryCurvePtr, // Curve index
                                                 {1},                                // Valid dimensions
                                                 RoutineName,                        // Routine name
                                                 cCurrentModuleObject,               // Object Type
                                                 thisVrfSys.Name,                    // Object Name
                                                 cAlphaFieldNames(17));              // Field Name
        }

        thisVrfSys.HeatEIRFTHi = GetCurveIndex(state, cAlphaArgs(18));
        if (thisVrfSys.HeatEIRFTHi > 0) {
            // Verify Curve Object, only legal type is biquadratic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.HeatEIRFTHi, // Curve index
                                                 {2},                    // Valid dimensions
                                                 RoutineName,            // Routine name
                                                 cCurrentModuleObject,   // Object Type
                                                 thisVrfSys.Name,        // Object Name
                                                 cAlphaFieldNames(18));  // Field Name
        }

        if (Util::SameString(cAlphaArgs(19), "WETBULBTEMPERATURE")) {
            thisVrfSys.HeatingPerformanceOATType = HVAC::OATType::WetBulb;
        } else if (Util::SameString(cAlphaArgs(19), "DRYBULBTEMPERATURE")) {
            thisVrfSys.HeatingPerformanceOATType = HVAC::OATType::DryBulb;
        } else {
            ShowSevereError(
                state,
                format(
                    "{}, \"{}\" illegal {} input for this object = {}", cCurrentModuleObject, thisVrfSys.Name, cAlphaFieldNames(19), cAlphaArgs(19)));
            ShowContinueError(state, "... input must be WETBULBTEMPERATURE or DRYBULBTEMPERATURE.");
            ErrorsFound = true;
        }

        thisVrfSys.HeatEIRFPLR1 = GetCurveIndex(state, cAlphaArgs(20));
        if (thisVrfSys.HeatEIRFPLR1 > 0) {
            // Verify Curve Object, only legal type is linear, quadratic, or cubic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.HeatEIRFPLR1, // Curve index
                                                 {1},                     // Valid dimensions
                                                 RoutineName,             // Routine name
                                                 cCurrentModuleObject,    // Object Type
                                                 thisVrfSys.Name,         // Object Name
                                                 cAlphaFieldNames(20));   // Field Name
        }

        thisVrfSys.HeatEIRFPLR2 = GetCurveIndex(state, cAlphaArgs(21));
        if (thisVrfSys.HeatEIRFPLR2 > 0) {
            // Verify Curve Object, only legal type is linear, quadratic, or cubic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.HeatEIRFPLR2, // Curve index
                                                 {1},                     // Valid dimensions
                                                 RoutineName,             // Routine name
                                                 cCurrentModuleObject,    // Object Type
                                                 thisVrfSys.Name,         // Object Name
                                                 cAlphaFieldNames(21));   // Field Name
        }

        thisVrfSys.HeatCombRatioPTR = GetCurveIndex(state, cAlphaArgs(22));
        if (thisVrfSys.HeatCombRatioPTR > 0) {
            // Verify Curve Object, only legal type is linear, quadratic, or cubic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.HeatCombRatioPTR, // Curve index
                                                 {1},                         // Valid dimensions
                                                 RoutineName,                 // Routine name
                                                 cCurrentModuleObject,        // Object Type
                                                 thisVrfSys.Name,             // Object Name
                                                 cAlphaFieldNames(22));       // Field Name
        }
        thisVrfSys.HeatPLFFPLR = GetCurveIndex(state, cAlphaArgs(23));
        if (thisVrfSys.HeatPLFFPLR > 0) {
            // Verify Curve Object, only legal type is linear, quadratic, or cubic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.HeatPLFFPLR, // Curve index
                                                 {1},                    // Valid dimensions
                                                 RoutineName,            // Routine name
                                                 cCurrentModuleObject,   // Object Type
                                                 thisVrfSys.Name,        // Object Name
                                                 cAlphaFieldNames(23));  // Field Name

            if (!ErrorsFound) {
                auto [MinCurvePLR, MinCurveVal, MaxCurvePLR, MaxCurveVal] = checkCurveMinMaxOutput(thisVrfSys.HeatPLFFPLR);

                if (MinCurveVal < 0.7) {
                    ShowWarningError(state, format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisVrfSys.Name));
                    ShowContinueError(state, format("...{}=\"{}\" has out of range values.", cAlphaFieldNames(23), cAlphaArgs(23)));
                    ShowContinueError(state,
                                      format("...Curve minimum must be >= 0.7, curve min at PLR = {:.2T} is {:.3T}", MinCurvePLR, MinCurveVal));
                    ShowContinueError(state, "...Setting curve minimum to 0.7 and simulation continues.");
                    Curve::SetCurveOutputMinValue(state, thisVrfSys.HeatPLFFPLR, ErrorsFound, 0.7);
                }

                if (MaxCurveVal > 1.0) {
                    ShowWarningError(state, format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisVrfSys.Name));
                    ShowContinueError(state, format("...{}=\"{}\" has out of range values.", cAlphaFieldNames(23), cAlphaArgs(23)));
                    ShowContinueError(state,
                                      format("...Curve maximum must be <= 1.0, curve max at PLR = {:.2T} is {:.3T}", MaxCurvePLR, MaxCurveVal));
                    ShowContinueError(state, "...Setting curve maximum to 1.0 and simulation continues.");
                    Curve::SetCurveOutputMaxValue(state, thisVrfSys.HeatPLFFPLR, ErrorsFound, 1.0);
                }
            }
        }

        thisVrfSys.MinPLR = rNumericArgs(10);
        Real64 minEIRfLowPLRXInput = 0.0;
        Real64 maxEIRfLowPLRXInput = 0.0;

        if (thisVrfSys.CoolEIRFPLR1 > 0) {
            Curve::GetCurveMinMaxValues(state, thisVrfSys.CoolEIRFPLR1, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
            if (minEIRfLowPLRXInput > thisVrfSys.MinPLR) {
                ShowWarningError(state, format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisVrfSys.Name));
                ShowContinueError(state, format("...{} = {} has out of range value.", cAlphaFieldNames(9), cAlphaArgs(9)));
                ShowContinueError(state,
                                  format("...Curve minimum value of X = {:.3T} must be <= Minimum Heat Pump Part-Load Ratio = {:.3T}.",
                                         minEIRfLowPLRXInput,
                                         thisVrfSys.MinPLR));
                ErrorsFound = true;
            }
            if (maxEIRfLowPLRXInput < 1.0) {
                ShowWarningError(state, format("{}{}=\"{}\", suspicious", RoutineName, cCurrentModuleObject, thisVrfSys.Name));
                ShowContinueError(state, format("...{} = {} has unexpected value.", cAlphaFieldNames(9), cAlphaArgs(9)));
                ShowContinueError(state,
                                  format("...Curve maximum value of X = {:.3T} should be 1 and will result in lower energy use than expected.",
                                         maxEIRfLowPLRXInput));
            }
            minEIRfLowPLRXInput = 0.0;
            maxEIRfLowPLRXInput = 0.0;
        }
        if (thisVrfSys.HeatEIRFPLR1 > 0) {
            Curve::GetCurveMinMaxValues(state, thisVrfSys.HeatEIRFPLR1, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
            if (minEIRfLowPLRXInput > thisVrfSys.MinPLR) {
                ShowWarningError(state, format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, thisVrfSys.Name));
                ShowContinueError(state, format("...{} = {} has out of range value.", cAlphaFieldNames(20), cAlphaArgs(20)));
                ShowContinueError(state,
                                  format("...Curve minimum value of X = {:.3T} must be <= Minimum Heat Pump Part-Load Ratio = {:.3T}.",
                                         minEIRfLowPLRXInput,
                                         thisVrfSys.MinPLR));
                ErrorsFound = true;
            }
            if (maxEIRfLowPLRXInput < 1.0) {
                ShowWarningError(state, format("{}{}=\"{}\", suspicious", RoutineName, cCurrentModuleObject, thisVrfSys.Name));
                ShowContinueError(state, format("...{} = {} has unexpected value.", cAlphaFieldNames(20), cAlphaArgs(20)));
                ShowContinueError(state,
                                  format("...Curve maximum value of X = {:.3T} should be 1 and will result in lower energy use than expected.",
                                         maxEIRfLowPLRXInput));
            }
        }

        thisVrfSys.MasterZonePtr = Util::FindItemInList(cAlphaArgs(24), state.dataHeatBal->Zone);

        thisVrfSys.ThermostatPriority = static_cast<ThermostatCtrlType>(getEnumValue(ThermostatCtrlTypeUC, cAlphaArgs(25)));

        if (thisVrfSys.ThermostatPriority == ThermostatCtrlType::MasterThermostatPriority) {
            if (thisVrfSys.MasterZonePtr == 0) {
                ShowSevereError(state, format("{} = \"{}\"", cCurrentModuleObject, thisVrfSys.Name));
                ShowContinueError(state, format("{} must be entered when {} = {}", cAlphaFieldNames(24), cAlphaFieldNames(25), cAlphaArgs(25)));
                ErrorsFound = true;
            }
        } else if (thisVrfSys.ThermostatPriority == ThermostatCtrlType::Invalid) {
            ShowSevereError(state, format("{} = \"{}\"", cCurrentModuleObject, thisVrfSys.Name));
            ShowContinueError(state, format("Illegal {} = {}", cAlphaFieldNames(25), cAlphaArgs(25)));
            ErrorsFound = true;
        }

        if (thisVrfSys.ThermostatPriority == ThermostatCtrlType::ScheduledPriority) {
            if (lAlphaFieldBlanks(26)) {
                ShowSevereEmptyField(state, eoh, cAlphaFieldNames(26), cAlphaFieldNames(25), cAlphaArgs(25));
            } else if ((thisVrfSys.prioritySched = Sched::GetSchedule(state, cAlphaArgs(26))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(26), cAlphaArgs(26));
                ErrorsFound = true;
            }
        }

        thisVrfSys.ZoneTUListPtr = Util::FindItemInList(cAlphaArgs(27), state.dataHVACVarRefFlow->TerminalUnitList);
        if (thisVrfSys.ZoneTUListPtr == 0) {
            ShowSevereError(state, format("{} = \"{}\"", cCurrentModuleObject, thisVrfSys.Name));
            ShowContinueError(state, format("{} = {} not found.", cAlphaFieldNames(27), cAlphaArgs(27)));
            ErrorsFound = true;
        }

        thisVrfSys.HeatRecoveryUsed = false;
        if (!lAlphaFieldBlanks(28)) {
            if (Util::SameString(cAlphaArgs(28), "No")) {
                thisVrfSys.HeatRecoveryUsed = false;
            } else if (Util::SameString(cAlphaArgs(28), "Yes")) {
                thisVrfSys.HeatRecoveryUsed = true;
            } else {
                ShowSevereError(state, format("{} = \"{}\"", cCurrentModuleObject, thisVrfSys.Name));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFieldNames(28), cAlphaArgs(28)));
                ErrorsFound = true;
            }
        }

        thisVrfSys.EquivPipeLngthCool = rNumericArgs(11);
        thisVrfSys.VertPipeLngth = rNumericArgs(12);
        thisVrfSys.PCFLengthCoolPtr = GetCurveIndex(state, cAlphaArgs(29));
        if (thisVrfSys.PCFLengthCoolPtr > 0) {
            // Verify Curve Object, only legal type is linear, quadratic, cubic, or biquadratic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.PCFLengthCoolPtr, // Curve index
                                                 {1, 2},                      // Valid dimensions  // MULTIPLECURVEDIMS
                                                 RoutineName,                 // Routine name
                                                 cCurrentModuleObject,        // Object Type
                                                 thisVrfSys.Name,             // Object Name
                                                 cAlphaFieldNames(29));       // Field Name
        }
        thisVrfSys.PCFHeightCool = rNumericArgs(13);

        thisVrfSys.EquivPipeLngthHeat = rNumericArgs(14);
        thisVrfSys.PCFLengthHeatPtr = GetCurveIndex(state, cAlphaArgs(30));
        if (thisVrfSys.PCFLengthHeatPtr > 0) {
            // Verify Curve Object, only legal type is linear, quadratic, cubic, or biquadratic
            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 thisVrfSys.PCFLengthHeatPtr, // Curve index
                                                 {1, 2},                      // Valid dimensions  // MULTIPLECURVEDIMS
                                                 RoutineName,                 // Routine name
                                                 cCurrentModuleObject,        // Object Type
                                                 thisVrfSys.Name,             // Object Name
                                                 cAlphaFieldNames(30));       // Field Name
        }

        thisVrfSys.PCFHeightHeat = rNumericArgs(15);

        thisVrfSys.CCHeaterPower = rNumericArgs(16);
        thisVrfSys.NumCompressors = rNumericArgs(17);
        thisVrfSys.CompressorSizeRatio = rNumericArgs(18);
        thisVrfSys.MaxOATCCHeater = rNumericArgs(19);

        if (!lAlphaFieldBlanks(31)) {
            thisVrfSys.DefrostStrategy = static_cast<StandardRatings::DefrostStrat>(getEnumValue(StandardRatings::DefrostStratUC, cAlphaArgs(31)));
            if (thisVrfSys.DefrostStrategy == StandardRatings::DefrostStrat::Invalid) {
                ShowSevereError(state,
                                format("{}, \"{}\" {} not found: {}", cCurrentModuleObject, thisVrfSys.Name, cAlphaFieldNames(31), cAlphaArgs(31)));
                ErrorsFound = true;
            }
        } else {
            thisVrfSys.DefrostStrategy = StandardRatings::DefrostStrat::ReverseCycle;
        }

        if (!lAlphaFieldBlanks(32)) {
            thisVrfSys.DefrostControl =
                static_cast<StandardRatings::HPdefrostControl>(getEnumValue(StandardRatings::HPdefrostControlUC, cAlphaArgs(32)));

            if (thisVrfSys.DefrostControl == StandardRatings::HPdefrostControl::Invalid) {
                ShowSevereError(state,
                                format("{}, \"{}\" {} not found: {}", cCurrentModuleObject, thisVrfSys.Name, cAlphaFieldNames(32), cAlphaArgs(32)));

                ErrorsFound = true;
            }
        } else {
            thisVrfSys.DefrostControl = StandardRatings::HPdefrostControl::Timed;
        }

        if (!lAlphaFieldBlanks(33)) {
            thisVrfSys.DefrostEIRPtr = GetCurveIndex(state, cAlphaArgs(33));
            if (thisVrfSys.DefrostEIRPtr > 0) {
                // Verify Curve Object, expected type is BiQuadratic
                ErrorsFound |= Curve::CheckCurveDims(state,
                                                     thisVrfSys.DefrostEIRPtr, // Curve index
                                                     {2},                      // Valid dimensions
                                                     RoutineName,              // Routine name
                                                     cCurrentModuleObject,     // Object Type
                                                     thisVrfSys.Name,          // Object Name
                                                     cAlphaFieldNames(33));    // Field Name
            } else {
                if (thisVrfSys.DefrostStrategy == StandardRatings::DefrostStrat::ReverseCycle) {
                    ShowSevereError(
                        state, format("{}, \"{}\" {} not found: {}", cCurrentModuleObject, thisVrfSys.Name, cAlphaFieldNames(33), cAlphaArgs(33)));
                    ErrorsFound = true;
                }
            }
        } else {
            if (thisVrfSys.DefrostStrategy == StandardRatings::DefrostStrat::ReverseCycle) {
                ShowSevereError(state,
                                format("{}, \"{}\" {} not found: {}", cCurrentModuleObject, thisVrfSys.Name, cAlphaFieldNames(33), cAlphaArgs(33)));
                ErrorsFound = true;
            }
        }

        thisVrfSys.DefrostFraction = rNumericArgs(20);
        thisVrfSys.DefrostCapacity = rNumericArgs(21);
        if (thisVrfSys.DefrostCapacity == 0.0 && thisVrfSys.DefrostStrategy == StandardRatings::DefrostStrat::Resistive) {
            ShowWarningError(
                state,
                format("{}, \"{}\" {} = 0.0 for defrost strategy = RESISTIVE.", cCurrentModuleObject, thisVrfSys.Name, cNumericFieldNames(21)));
        }

        thisVrfSys.MaxOATDefrost = rNumericArgs(22);

        if (!lAlphaFieldBlanks(35)) {
            if (Util::SameString(cAlphaArgs(34), "AirCooled")) {
                thisVrfSys.CondenserType = DataHeatBalance::RefrigCondenserType::Air;
            }
            if (Util::SameString(cAlphaArgs(34), "EvaporativelyCooled")) {
                thisVrfSys.CondenserType = DataHeatBalance::RefrigCondenserType::Evap;
            }
            if (Util::SameString(cAlphaArgs(34), "WaterCooled")) {
                thisVrfSys.CondenserType = DataHeatBalance::RefrigCondenserType::Water;
                thisVrfSys.VRFType = PlantEquipmentType::HeatPumpVRF;
                if (thisVrfSys.HeatingPerformanceOATType == HVAC::OATType::WetBulb) {
                    ShowSevereError(state, format("{} = {}", cCurrentModuleObject, thisVrfSys.Name));
                    ShowContinueError(state, format("{} = {}", cAlphaFieldNames(34), cAlphaArgs(34)));
                    ShowContinueError(state, format("Illegal {} input for this object = {}", cAlphaFieldNames(19), cAlphaArgs(19)));
                    ShowContinueError(state, "... input must be DRYBULBTEMPERATURE when Condenser Type is WaterCooled.");
                    ShowContinueError(state, format("... {} will be reset to DRYBULBTEMPERATURE and simulation continues.", cAlphaFieldNames(19)));
                }
            }
            if (thisVrfSys.CondenserType == DataHeatBalance::RefrigCondenserType::Invalid) {
                ShowSevereError(state, cCurrentModuleObject + " = " + thisVrfSys.Name);
                ShowContinueError(state, "Illegal " + cAlphaFieldNames(34) + " = " + cAlphaArgs(34));
                ErrorsFound = true;
            }
        } else {
            thisVrfSys.CondenserType = DataHeatBalance::RefrigCondenserType::Air;
        }

        // outdoor condenser node
        if (lAlphaFieldBlanks(35)) {
            thisVrfSys.CondenserNodeNum = 0;
        } else {
            switch (thisVrfSys.CondenserType) {
            case DataHeatBalance::RefrigCondenserType::Air:
            case DataHeatBalance::RefrigCondenserType::Evap: {
                thisVrfSys.CondenserNodeNum = GetOnlySingleNode(state,
                                                                cAlphaArgs(35),
                                                                ErrorsFound,
                                                                DataLoopNode::ConnectionObjectType::AirConditionerVariableRefrigerantFlow,
                                                                thisVrfSys.Name,
                                                                DataLoopNode::NodeFluidType::Air,
                                                                DataLoopNode::ConnectionType::OutsideAirReference,
                                                                NodeInputManager::CompFluidStream::Primary,
                                                                ObjectIsNotParent);
                if (!CheckOutAirNodeNumber(state, thisVrfSys.CondenserNodeNum)) {
                    ShowSevereError(state,
                                    format("{}, \"{}\" {} not a valid Outdoor Air Node = {}",
                                           cCurrentModuleObject,
                                           thisVrfSys.Name,
                                           cAlphaFieldNames(35),
                                           cAlphaArgs(35)));
                    ShowContinueError(state, "...node name does not appear in an OutdoorAir:NodeList or as an OutdoorAir:Node.");
                    ErrorsFound = true;
                }
            } break;
            case DataHeatBalance::RefrigCondenserType::Water: {
                thisVrfSys.CondenserNodeNum = GetOnlySingleNode(state,
                                                                cAlphaArgs(35),
                                                                ErrorsFound,
                                                                DataLoopNode::ConnectionObjectType::AirConditionerVariableRefrigerantFlow,
                                                                thisVrfSys.Name,
                                                                DataLoopNode::NodeFluidType::Water,
                                                                DataLoopNode::ConnectionType::Inlet,
                                                                NodeInputManager::CompFluidStream::Secondary,
                                                                ObjectIsNotParent);
            } break;
            default:
                break;
            }
        }

        if (!lAlphaFieldBlanks(36) && thisVrfSys.CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
            thisVrfSys.CondenserOutletNodeNum = GetOnlySingleNode(state,
                                                                  cAlphaArgs(36),
                                                                  ErrorsFound,
                                                                  DataLoopNode::ConnectionObjectType::AirConditionerVariableRefrigerantFlow,
                                                                  thisVrfSys.Name,
                                                                  DataLoopNode::NodeFluidType::Water,
                                                                  DataLoopNode::ConnectionType::Outlet,
                                                                  NodeInputManager::CompFluidStream::Secondary,
                                                                  ObjectIsNotParent);
            TestCompSet(state, cCurrentModuleObject, thisVrfSys.Name, cAlphaArgs(35), cAlphaArgs(36), "Condenser Water Nodes");
        } else if (lAlphaFieldBlanks(36) && thisVrfSys.CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
            ShowSevereError(state, format("{}, \"{}\" {} is blank.", cCurrentModuleObject, thisVrfSys.Name, cAlphaFieldNames(36)));
            ShowContinueError(state, "...node name must be entered when Condenser Type = WaterCooled.");
            ErrorsFound = true;
        }

        if (lAlphaFieldBlanks(23)) {
            if (thisVrfSys.CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
                ShowSevereError(state, format("{}, \"{}\" {} is blank.", cCurrentModuleObject, thisVrfSys.Name, cNumericFieldNames(23)));
                ShowContinueError(state, format("...input is required when {} = {}", cAlphaFieldNames(34), cAlphaArgs(34)));
                ErrorsFound = true;
            }
        } else {
            thisVrfSys.WaterCondVolFlowRate = rNumericArgs(23);
        }
        thisVrfSys.EvapCondEffectiveness = rNumericArgs(24);
        thisVrfSys.EvapCondAirVolFlowRate = rNumericArgs(25);
        thisVrfSys.EvapCondPumpPower = rNumericArgs(26);

        // Get Water System tank connections
        // A37, \field Supply Water Storage Tank Name
        thisVrfSys.EvapWaterSupplyName = cAlphaArgs(37);
        if (lAlphaFieldBlanks(37)) {
            thisVrfSys.EvapWaterSupplyMode = EvapWaterSupply::FromMains;
        } else {
            thisVrfSys.EvapWaterSupplyMode = EvapWaterSupply::FromTank;
            SetupTankDemandComponent(state,
                                     thisVrfSys.Name,
                                     cCurrentModuleObject,
                                     thisVrfSys.EvapWaterSupplyName,
                                     ErrorsFound,
                                     thisVrfSys.EvapWaterSupTankID,
                                     thisVrfSys.EvapWaterTankDemandARRID);
        }

        //   Basin heater power as a function of temperature must be greater than or equal to 0
        thisVrfSys.BasinHeaterPowerFTempDiff = rNumericArgs(27);
        if (rNumericArgs(27) < 0.0) {
            ShowSevereError(state, format("{}, \"{}\" {} must be >= 0", cCurrentModuleObject, thisVrfSys.Name, cNumericFieldNames(27)));
            ErrorsFound = true;
        }

        thisVrfSys.BasinHeaterSetPointTemp = rNumericArgs(28);
        if (thisVrfSys.BasinHeaterPowerFTempDiff > 0.0) {
            if (NumNums < 27) {
                thisVrfSys.BasinHeaterSetPointTemp = 2.0;
            }
            if (thisVrfSys.BasinHeaterSetPointTemp < 2.0) {
                ShowWarningError(
                    state,
                    format(
                        "{}, \"{}\" {} is less than 2 deg C. Freezing could occur.", cCurrentModuleObject, thisVrfSys.Name, cNumericFieldNames(28)));
            }
        }

        if (!lAlphaFieldBlanks(38)) {
            if ((thisVrfSys.basinHeaterSched = Sched::GetSchedule(state, cAlphaArgs(38))) == nullptr) {
                ShowWarningItemNotFound(
                    state, eoh, cAlphaFieldNames(38), cAlphaArgs(38), "Basin heater will be available to operate throughout the simulation.");
            }
        }

        if (!lAlphaFieldBlanks(39)) {
            // A39; \field Fuel type, Validate fuel type input
            if ((thisVrfSys.fuel = static_cast<Constant::eFuel>(getEnumValue(Constant::eFuelNamesUC, cAlphaArgs(39)))) == Constant::eFuel::Invalid) {
                ShowSevereInvalidKey(state, eoh, cAlphaFieldNames(39), cAlphaArgs(39));
                ErrorsFound = true;
            }
        }

        if (thisVrfSys.HeatRecoveryUsed) {
            if (lAlphaFieldBlanks(29)) {
                thisVrfSys.MinOATHeatRecovery = max(thisVrfSys.MinOATCooling, thisVrfSys.MinOATHeating);
            } else {
                thisVrfSys.MinOATHeatRecovery = rNumericArgs(29);
                if (thisVrfSys.MinOATHeatRecovery < thisVrfSys.MinOATCooling || thisVrfSys.MinOATHeatRecovery < thisVrfSys.MinOATHeating) {
                    ShowWarningError(state,
                                     format("{} = \"{}\", {} is less than the minimum temperature in heat pump mode.",
                                            cCurrentModuleObject,
                                            thisVrfSys.Name,
                                            cNumericFieldNames(29)));
                    ShowContinueError(state, format("...{} = {:.2T} C", cNumericFieldNames(29), thisVrfSys.MinOATHeatRecovery));
                    ShowContinueError(state, format("...Minimum Outdoor Temperature in Cooling Mode = {:.2T} C", thisVrfSys.MinOATCooling));
                    ShowContinueError(state, format("...Minimum Outdoor Temperature in Heating Mode = {:.2T} C", thisVrfSys.MinOATHeating));
                    ShowContinueError(state,
                                      "...Minimum Outdoor Temperature in Heat Recovery Mode reset to greater of cooling or heating minimum "
                                      "temperature and simulation continues.");
                    thisVrfSys.MinOATHeatRecovery = max(thisVrfSys.MinOATCooling, thisVrfSys.MinOATHeating);
                    ShowContinueError(state, format("... adjusted {} = {:.2T} C", cNumericFieldNames(29), thisVrfSys.MinOATHeatRecovery));
                }
            }
            if (lAlphaFieldBlanks(30)) {
                thisVrfSys.MaxOATHeatRecovery = min(thisVrfSys.MaxOATCooling, thisVrfSys.MaxOATHeating);
            } else {
                thisVrfSys.MaxOATHeatRecovery = rNumericArgs(30);
                if (thisVrfSys.MaxOATHeatRecovery > thisVrfSys.MaxOATCooling || thisVrfSys.MaxOATHeatRecovery > thisVrfSys.MaxOATHeating) {
                    ShowWarningError(state,
                                     format("{} = \"{}\", {} is greater than the maximum temperature in heat pump mode.",
                                            cCurrentModuleObject,
                                            thisVrfSys.Name,
                                            cNumericFieldNames(30)));
                    ShowContinueError(state, format("...{} = {:.2T} C", cNumericFieldNames(30), thisVrfSys.MaxOATHeatRecovery));
                    ShowContinueError(state, format("...Maximum Outdoor Temperature in Cooling Mode = {:.2T} C", thisVrfSys.MaxOATCooling));
                    ShowContinueError(state, format("...Maximum Outdoor Temperature in Heating Mode = {:.2T} C", thisVrfSys.MaxOATHeating));
                    ShowContinueError(state,
                                      "...Maximum Outdoor Temperature in Heat Recovery Mode reset to lesser of cooling or heating minimum "
                                      "temperature and simulation continues.");
                    thisVrfSys.MaxOATHeatRecovery = min(thisVrfSys.MaxOATCooling, thisVrfSys.MaxOATHeating);
                    ShowContinueError(state, format("... adjusted {} = {:.2T} C", cNumericFieldNames(30), thisVrfSys.MaxOATHeatRecovery));
                }
            }

            thisVrfSys.HRCAPFTCool = GetCurveIndex(state, cAlphaArgs(40));
            if (thisVrfSys.HRCAPFTCool > 0) {
                // Verify Curve Object, only legal type is bi-quadratic or linear, quadratic, or cubic
                ErrorsFound |= Curve::CheckCurveDims(state,
                                                     thisVrfSys.HRCAPFTCool, // Curve index
                                                     {1, 2},                 // Valid dimensions  // MULTIPLECURVEDIMS
                                                     RoutineName,            // Routine name
                                                     cCurrentModuleObject,   // Object Type
                                                     thisVrfSys.Name,        // Object Name
                                                     cAlphaFieldNames(40));  // Field Name
            }
            if (!lNumericFieldBlanks(31)) {
                thisVrfSys.HRInitialCoolCapFrac = rNumericArgs(31);
            }
            thisVrfSys.HRCoolCapTC = rNumericArgs(32);
            thisVrfSys.HREIRFTCool = GetCurveIndex(state, cAlphaArgs(41));
            if (thisVrfSys.HREIRFTCool > 0) {
                // Verify Curve Object, only legal type is bi-quadratic or linear, quadratic, or cubic
                ErrorsFound |= Curve::CheckCurveDims(state,
                                                     thisVrfSys.HREIRFTCool, // Curve index
                                                     {1, 2},                 // Valid dimensions  // MULTIPLECURVEDIMS
                                                     RoutineName,            // Routine name
                                                     cCurrentModuleObject,   // Object Type
                                                     thisVrfSys.Name,        // Object Name
                                                     cAlphaFieldNames(41));  // Field Name
            }
            thisVrfSys.HRInitialCoolEIRFrac = rNumericArgs(33);
            thisVrfSys.HRCoolEIRTC = rNumericArgs(34);

            //  INTEGER      :: HRCAPFTHeat                =0   ! Index to heat capacity as a function of temperature curve for heat recovery
            //  REAL(r64)    :: HRInitialHeatCapFrac       =0.0d0 ! Fractional heating degradation at the start of heat recovery from heating mode
            //  REAL(r64)    :: HRHeatCapTC                =0.0d0 ! Time constant used to recover from initial degradation in heating heat
            //  recovery
            thisVrfSys.HRCAPFTHeat = GetCurveIndex(state, cAlphaArgs(42));
            if (thisVrfSys.HRCAPFTHeat > 0) {
                // Verify Curve Object, only legal type is bi-quadratic or linear, quadratic, or cubic
                ErrorsFound |= Curve::CheckCurveDims(state,
                                                     thisVrfSys.HRCAPFTHeat, // Curve index
                                                     {1, 2},                 // Valid dimensions  // MULTIPLECURVEDIMS
                                                     RoutineName,            // Routine name
                                                     cCurrentModuleObject,   // Object Type
                                                     thisVrfSys.Name,        // Object Name
                                                     cAlphaFieldNames(42));  // Field Name
            }
            thisVrfSys.HRInitialHeatCapFrac = rNumericArgs(35);
            thisVrfSys.HRHeatCapTC = rNumericArgs(36);

            //  INTEGER      :: HREIRFTHeat                =0   ! Index to heat EIR as a function of temperature curve for heat recovery
            //  REAL(r64)    :: HRInitialHeatEIRFrac       =0.0d0 ! Fractional EIR degradation at the start of heat recovery from heating mode
            //  REAL(r64)    :: HRHeatEIRTC                =0.0d0 ! Time constant used to recover from initial degradation in heating heat
            //  recovery
            thisVrfSys.HREIRFTHeat = GetCurveIndex(state, cAlphaArgs(43));
            if (thisVrfSys.HREIRFTHeat > 0) {
                // Verify Curve Object, only legal type is bi-quadratic or linear, quadratic, or cubic
                ErrorsFound |= Curve::CheckCurveDims(state,
                                                     thisVrfSys.HREIRFTHeat, // Curve index
                                                     {1, 2},                 // Valid dimensions  // MULTIPLECURVEDIMS
                                                     RoutineName,            // Routine name
                                                     cCurrentModuleObject,   // Object Type
                                                     thisVrfSys.Name,        // Object Name
                                                     cAlphaFieldNames(43));  // Field Name
            }
            thisVrfSys.HRInitialHeatEIRFrac = rNumericArgs(37);
            thisVrfSys.HRHeatEIRTC = rNumericArgs(38);
        }
    }

    // Read all VRF condenser objects: Algorithm Type 2_physics based model (VRF-FluidTCtrl-HP)
    cCurrentModuleObject = "AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl";
    for (int thisNum = 1; thisNum <= state.dataHVACVarRefFlow->NumVRFCond_FluidTCtrl_HP; ++thisNum) {

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 thisNum,
                                                                 cAlphaArgs,
                                                                 NumAlphas,
                                                                 rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericFieldBlanks,
                                                                 lAlphaFieldBlanks,
                                                                 cAlphaFieldNames,
                                                                 cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

        GlobalNames::VerifyUniqueInterObjectName(
            state, state.dataHVACVarRefFlow->VrfUniqueNames, cAlphaArgs(1), cCurrentModuleObject, cAlphaFieldNames(1), ErrorsFound);

        int VRFNum = state.dataHVACVarRefFlow->NumVRFCond_SysCurve + thisNum;
        auto &thisVrfFluidCtrl = state.dataHVACVarRefFlow->VRF(VRFNum);
        thisVrfFluidCtrl.Name = cAlphaArgs(1);
        thisVrfFluidCtrl.VRFSystemTypeNum = VRF_HeatPump;
        thisVrfFluidCtrl.VRFAlgorithmType = AlgorithmType::FluidTCtrl;
        thisVrfFluidCtrl.fuel = Constant::eFuel::Electricity;

        if (lAlphaFieldBlanks(2)) {
            thisVrfFluidCtrl.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisVrfFluidCtrl.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
            ErrorsFound = true;
        }

        thisVrfFluidCtrl.ZoneTUListPtr =
            Util::FindItemInList(cAlphaArgs(3), state.dataHVACVarRefFlow->TerminalUnitList, state.dataHVACVarRefFlow->NumVRFTULists);
        if (thisVrfFluidCtrl.ZoneTUListPtr == 0) {
            ShowSevereError(state, cCurrentModuleObject + " = \"" + thisVrfFluidCtrl.Name + "\"");
            ShowContinueError(state, cAlphaFieldNames(3) + " = " + cAlphaArgs(3) + " not found.");
            ErrorsFound = true;
        }

        // Refrigerant type
        thisVrfFluidCtrl.refrigName = cAlphaArgs(4);
        thisVrfFluidCtrl.refrig = Fluid::GetRefrig(state, thisVrfFluidCtrl.refrigName);
        if (thisVrfFluidCtrl.refrig == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(4), cAlphaArgs(4));
            ErrorsFound = true;
        }

        thisVrfFluidCtrl.RatedEvapCapacity = rNumericArgs(1);
        thisVrfFluidCtrl.RatedCompPowerPerCapcity = rNumericArgs(2);
        thisVrfFluidCtrl.RatedCompPower = thisVrfFluidCtrl.RatedCompPowerPerCapcity * thisVrfFluidCtrl.RatedEvapCapacity;
        thisVrfFluidCtrl.CoolingCapacity = thisVrfFluidCtrl.RatedEvapCapacity;
        thisVrfFluidCtrl.RatedHeatCapacity = thisVrfFluidCtrl.RatedEvapCapacity * (1 + thisVrfFluidCtrl.RatedCompPowerPerCapcity);
        thisVrfFluidCtrl.HeatingCapacity = thisVrfFluidCtrl.RatedHeatCapacity;

        // Reference system COP
        thisVrfFluidCtrl.CoolingCOP = 1 / thisVrfFluidCtrl.RatedCompPowerPerCapcity;
        thisVrfFluidCtrl.HeatingCOP = 1 / thisVrfFluidCtrl.RatedCompPowerPerCapcity + 1;

        // OA temperature range for VRF-HP operations
        thisVrfFluidCtrl.MinOATCooling = rNumericArgs(3);
        thisVrfFluidCtrl.MaxOATCooling = rNumericArgs(4);
        thisVrfFluidCtrl.MinOATHeating = rNumericArgs(5);
        thisVrfFluidCtrl.MaxOATHeating = rNumericArgs(6);
        if (thisVrfFluidCtrl.MinOATCooling >= thisVrfFluidCtrl.MaxOATCooling) {
            ShowSevereError(state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrl.Name + "\"");
            ShowContinueError(state,
                              format("... {} ({:.3T}) must be less than maximum ({:.3T}).",
                                     cNumericFieldNames(3),
                                     thisVrfFluidCtrl.MinOATCooling,
                                     thisVrfFluidCtrl.MaxOATCooling));
            ErrorsFound = true;
        }
        if (thisVrfFluidCtrl.MinOATHeating >= thisVrfFluidCtrl.MaxOATHeating) {
            ShowSevereError(state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrl.Name + "\"");
            ShowContinueError(state,
                              format("... {} ({:.3T}) must be less than maximum ({:.3T}).",
                                     cNumericFieldNames(5),
                                     thisVrfFluidCtrl.MinOATHeating,
                                     thisVrfFluidCtrl.MaxOATHeating));
            ErrorsFound = true;
        }

        // Reference OU SH/SC
        thisVrfFluidCtrl.SH = rNumericArgs(7);
        thisVrfFluidCtrl.SC = rNumericArgs(8);

        if (Util::SameString(cAlphaArgs(5), "VariableTemp")) {
            thisVrfFluidCtrl.AlgorithmIUCtrl = 1;
        } else if (Util::SameString(cAlphaArgs(5), "ConstantTemp")) {
            thisVrfFluidCtrl.AlgorithmIUCtrl = 2;
        } else {
            thisVrfFluidCtrl.AlgorithmIUCtrl = 1;
        }

        // Reference IU Te/Tc for IU Control Algorithm: ConstantTemp
        thisVrfFluidCtrl.EvapTempFixed = rNumericArgs(9);
        thisVrfFluidCtrl.CondTempFixed = rNumericArgs(10);

        // Bounds of Te/Tc for IU Control Algorithm: VariableTemp
        thisVrfFluidCtrl.IUEvapTempLow = rNumericArgs(11);
        thisVrfFluidCtrl.IUEvapTempHigh = rNumericArgs(12);
        thisVrfFluidCtrl.IUCondTempLow = rNumericArgs(13);
        thisVrfFluidCtrl.IUCondTempHigh = rNumericArgs(14);
        if (thisVrfFluidCtrl.IUEvapTempLow >= thisVrfFluidCtrl.IUEvapTempHigh) {
            ShowSevereError(state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrl.Name + "\"");
            ShowContinueError(state,
                              format("... {} ({:.3T}) must be less than maximum ({:.3T}).",
                                     cNumericFieldNames(11),
                                     thisVrfFluidCtrl.IUEvapTempLow,
                                     thisVrfFluidCtrl.IUEvapTempHigh));
            ErrorsFound = true;
        }
        if (thisVrfFluidCtrl.IUCondTempLow >= thisVrfFluidCtrl.IUCondTempHigh) {
            ShowSevereError(state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrl.Name + "\"");
            ShowContinueError(state,
                              format("... {} ({:.3T}) must be less than maximum ({:.3T}).",
                                     cNumericFieldNames(13),
                                     thisVrfFluidCtrl.IUCondTempLow,
                                     thisVrfFluidCtrl.IUCondTempHigh));
            ErrorsFound = true;
        }

        // Get OU fan data
        thisVrfFluidCtrl.RatedOUFanPowerPerCapcity = rNumericArgs(15);
        thisVrfFluidCtrl.OUAirFlowRatePerCapcity = rNumericArgs(16);
        thisVrfFluidCtrl.RatedOUFanPower = thisVrfFluidCtrl.RatedOUFanPowerPerCapcity * thisVrfFluidCtrl.RatedEvapCapacity;
        thisVrfFluidCtrl.OUAirFlowRate = thisVrfFluidCtrl.OUAirFlowRatePerCapcity * thisVrfFluidCtrl.RatedEvapCapacity;

        // OUEvapTempCurve
        int indexOUEvapTempCurve = GetCurveIndex(state, cAlphaArgs(6)); // convert curve name to index number
        // Verify curve name and type
        if (indexOUEvapTempCurve == 0) {
            if (lAlphaFieldBlanks(6)) {
                ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrl.Name + "\", missing");
                ShowContinueError(state, "...required " + cAlphaFieldNames(6) + " is blank.");
            } else {
                ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrl.Name + "\", invalid");
                ShowContinueError(state, "...not found " + cAlphaFieldNames(6) + "=\"" + cAlphaArgs(6) + "\".");
            }
            ErrorsFound = true;
        } else {
            {
                if (state.dataCurveManager->curves(indexOUEvapTempCurve)->curveType == Curve::CurveType::Quadratic) {
                    thisVrfFluidCtrl.C1Te = state.dataCurveManager->curves(indexOUEvapTempCurve)->coeff[0];
                    thisVrfFluidCtrl.C2Te = state.dataCurveManager->curves(indexOUEvapTempCurve)->coeff[1];
                    thisVrfFluidCtrl.C3Te = state.dataCurveManager->curves(indexOUEvapTempCurve)->coeff[2];

                } else {
                    ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrl.Name + "\", invalid");
                    ShowContinueError(state,
                                      format("...illegal {} type for this object = {}",
                                             cAlphaFieldNames(6),
                                             Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(indexOUEvapTempCurve)->curveType)]));
                    ShowContinueError(state, "... Curve type must be Quadratic.");
                    ErrorsFound = true;
                }
            }
        }

        // OUCondTempCurve
        int indexOUCondTempCurve = GetCurveIndex(state, cAlphaArgs(7)); // convert curve name to index number
        // Verify curve name and type
        if (indexOUCondTempCurve == 0) {
            if (lAlphaFieldBlanks(7)) {
                ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrl.Name + "\", missing");
                ShowContinueError(state, "...required " + cAlphaFieldNames(7) + " is blank.");
            } else {
                ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrl.Name + "\", invalid");
                ShowContinueError(state, "...not found " + cAlphaFieldNames(7) + "=\"" + cAlphaArgs(7) + "\".");
            }
            ErrorsFound = true;
        } else {
            {
                if (state.dataCurveManager->curves(indexOUCondTempCurve)->curveType == Curve::CurveType::Quadratic) {
                    thisVrfFluidCtrl.C1Tc = state.dataCurveManager->curves(indexOUCondTempCurve)->coeff[0];
                    thisVrfFluidCtrl.C2Tc = state.dataCurveManager->curves(indexOUCondTempCurve)->coeff[1];
                    thisVrfFluidCtrl.C3Tc = state.dataCurveManager->curves(indexOUCondTempCurve)->coeff[2];

                } else {
                    ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrl.Name + "\", invalid");
                    ShowContinueError(state,
                                      format("...illegal {} type for this object = {}",
                                             cAlphaFieldNames(7),
                                             Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(indexOUCondTempCurve)->curveType)]));
                    ShowContinueError(state, "... Curve type must be Quadratic.");
                    ErrorsFound = true;
                }
            }
        }

        // Pipe parameters
        thisVrfFluidCtrl.RefPipDiaSuc = rNumericArgs(17);
        thisVrfFluidCtrl.RefPipDiaDis = rNumericArgs(17);
        thisVrfFluidCtrl.RefPipLen = rNumericArgs(18);
        thisVrfFluidCtrl.RefPipEquLen = rNumericArgs(19);
        thisVrfFluidCtrl.RefPipHei = rNumericArgs(20);
        thisVrfFluidCtrl.RefPipInsThi = rNumericArgs(21);
        thisVrfFluidCtrl.RefPipInsCon = rNumericArgs(22);

        // Check the RefPipEquLen
        if (lNumericFieldBlanks(19) && !lNumericFieldBlanks(18)) {
            thisVrfFluidCtrl.RefPipEquLen = 1.2 * thisVrfFluidCtrl.RefPipLen;
            ShowWarningError(
                state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrl.Name + "\", \" " + cNumericFieldNames(19) + "\" is calculated based on");
            ShowContinueError(state, "...the provided \"" + cNumericFieldNames(18) + "\" value.");
        }
        if (thisVrfFluidCtrl.RefPipEquLen < thisVrfFluidCtrl.RefPipLen) {
            thisVrfFluidCtrl.RefPipEquLen = 1.2 * thisVrfFluidCtrl.RefPipLen;
            ShowWarningError(state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrl.Name + "\", invalid \" " + cNumericFieldNames(19) + "\" value.");
            ShowContinueError(state, "...Equivalent length of main pipe should be greater than or equal to the actual length.");
            ShowContinueError(state, format("...The value is recalculated based on the provided \"{}\" value.", cNumericFieldNames(18)));
        }

        // Crank case
        thisVrfFluidCtrl.CCHeaterPower = rNumericArgs(23);
        thisVrfFluidCtrl.NumCompressors = rNumericArgs(24);
        thisVrfFluidCtrl.CompressorSizeRatio = rNumericArgs(25);
        thisVrfFluidCtrl.MaxOATCCHeater = rNumericArgs(26);

        // Defrost
        if (!lAlphaFieldBlanks(8)) {
            if (Util::SameString(cAlphaArgs(8), "ReverseCycle")) {
                thisVrfFluidCtrl.DefrostStrategy = StandardRatings::DefrostStrat::ReverseCycle;
            }
            if (Util::SameString(cAlphaArgs(8), "Resistive")) {
                thisVrfFluidCtrl.DefrostStrategy = StandardRatings::DefrostStrat::Resistive;
            }
            if (thisVrfFluidCtrl.DefrostStrategy == StandardRatings::DefrostStrat::Invalid) {
                ShowSevereError(state,
                                cCurrentModuleObject + ", \"" + thisVrfFluidCtrl.Name + "\" " + cAlphaFieldNames(8) + " not found: " + cAlphaArgs(8));
                ErrorsFound = true;
            }
        } else {
            thisVrfFluidCtrl.DefrostStrategy = StandardRatings::DefrostStrat::ReverseCycle;
        }

        if (!lAlphaFieldBlanks(9)) {
            if (Util::SameString(cAlphaArgs(9), "Timed")) {
                thisVrfFluidCtrl.DefrostControl = StandardRatings::HPdefrostControl::Timed;
            }
            if (Util::SameString(cAlphaArgs(9), "OnDemand")) {
                thisVrfFluidCtrl.DefrostControl = StandardRatings::HPdefrostControl::OnDemand;
            }
            if (thisVrfFluidCtrl.DefrostControl == StandardRatings::HPdefrostControl::Invalid) {
                ShowSevereError(state,
                                cCurrentModuleObject + ", \"" + thisVrfFluidCtrl.Name + "\" " + cAlphaFieldNames(9) + " not found: " + cAlphaArgs(9));
                ErrorsFound = true;
            }
        } else {
            thisVrfFluidCtrl.DefrostControl = StandardRatings::HPdefrostControl::Timed;
        }

        if (!lAlphaFieldBlanks(10)) {
            thisVrfFluidCtrl.DefrostEIRPtr = GetCurveIndex(state, cAlphaArgs(10));
            if (thisVrfFluidCtrl.DefrostEIRPtr > 0) {
                // Verify Curve Object, expected type is BiQuadratic
                ErrorsFound |= Curve::CheckCurveDims(state,
                                                     thisVrfFluidCtrl.DefrostEIRPtr, // Curve index
                                                     {2},                            // Valid dimensions
                                                     RoutineName,                    // Routine name
                                                     cCurrentModuleObject,           // Object Type
                                                     thisVrfFluidCtrl.Name,          // Object Name
                                                     cAlphaFieldNames(10));          // Field Name
            } else {
                if (thisVrfFluidCtrl.DefrostStrategy == StandardRatings::DefrostStrat::ReverseCycle &&
                    thisVrfFluidCtrl.DefrostControl == StandardRatings::HPdefrostControl::OnDemand) {
                    ShowSevereError(
                        state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrl.Name + "\" " + cAlphaFieldNames(10) + " not found:" + cAlphaArgs(10));
                    ErrorsFound = true;
                }
            }
        } else {
            if (thisVrfFluidCtrl.DefrostStrategy == StandardRatings::DefrostStrat::ReverseCycle &&
                thisVrfFluidCtrl.DefrostControl == StandardRatings::HPdefrostControl::OnDemand) {
                ShowSevereError(
                    state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrl.Name + "\" " + cAlphaFieldNames(10) + " not found:" + cAlphaArgs(10));
                ErrorsFound = true;
            }
        }

        thisVrfFluidCtrl.DefrostFraction = rNumericArgs(27);
        thisVrfFluidCtrl.DefrostCapacity = rNumericArgs(28);
        thisVrfFluidCtrl.MaxOATDefrost = rNumericArgs(29);
        if (thisVrfFluidCtrl.DefrostCapacity == 0.0 && thisVrfFluidCtrl.DefrostStrategy == StandardRatings::DefrostStrat::Resistive) {
            ShowWarningError(state,
                             cCurrentModuleObject + ", \"" + thisVrfFluidCtrl.Name + "\" " + cNumericFieldNames(28) +
                                 " = 0.0 for defrost strategy = RESISTIVE.");
        }

        thisVrfFluidCtrl.CompMaxDeltaP = rNumericArgs(30);

        //@@ The control type
        std::string ThermostatPriorityType = "LoadPriority"; // cAlphaArgs( 25 )
        if (Util::SameString(ThermostatPriorityType, "LoadPriority")) {
            thisVrfFluidCtrl.ThermostatPriority = ThermostatCtrlType::LoadPriority;
        } else if (Util::SameString(ThermostatPriorityType, "ZonePriority")) {
            thisVrfFluidCtrl.ThermostatPriority = ThermostatCtrlType::ZonePriority;
        } else if (Util::SameString(ThermostatPriorityType, "ThermostatOffsetPriority")) {
            thisVrfFluidCtrl.ThermostatPriority = ThermostatCtrlType::ThermostatOffsetPriority;
        } else if (Util::SameString(ThermostatPriorityType, "Scheduled")) {
            thisVrfFluidCtrl.ThermostatPriority = ThermostatCtrlType::ScheduledPriority;
        } else if (Util::SameString(ThermostatPriorityType, "MasterThermostatPriority")) {
            thisVrfFluidCtrl.ThermostatPriority = ThermostatCtrlType::MasterThermostatPriority;
            if (thisVrfFluidCtrl.MasterZonePtr == 0) {
                ShowSevereError(state, cCurrentModuleObject + " = \"" + thisVrfFluidCtrl.Name + "\"");
                //** ShowContinueError(state,  cAlphaFieldNames( 24 ) + " must be entered when " + cAlphaFieldNames( 25 ) + " = " + cAlphaArgs( 25 )
                //);
                ErrorsFound = true;
            }
        } else {
            ShowSevereError(state, cCurrentModuleObject + " = " + thisVrfFluidCtrl.Name);
            // ShowContinueError(state,  "Illegal " + cAlphaFieldNames( 25 ) + " = " + cAlphaArgs( 25 ) );
            ErrorsFound = true;
        }

        // The new VRF model is Air cooled
        thisVrfFluidCtrl.CondenserType = DataHeatBalance::RefrigCondenserType::Air;
        thisVrfFluidCtrl.CondenserNodeNum = 0;

        // Evaporative Capacity & Compressor Power Curves corresponding to each Loading Index / compressor speed
        int NumOfCompSpd = rNumericArgs(31);
        thisVrfFluidCtrl.CompressorSpeed.dimension(NumOfCompSpd);
        thisVrfFluidCtrl.OUCoolingCAPFT.dimension(NumOfCompSpd);
        thisVrfFluidCtrl.OUCoolingPWRFT.dimension(NumOfCompSpd);
        int Count1Index = 31; // the index of the last numeric field before compressor speed entries
        int Count2Index = 9;  // the index of the last alpha field before capacity/power curves
        for (int NumCompSpd = 1; NumCompSpd <= NumOfCompSpd; NumCompSpd++) {
            thisVrfFluidCtrl.CompressorSpeed(NumCompSpd) = rNumericArgs(Count1Index + NumCompSpd);

            // Evaporating Capacity Curve
            if (!lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd)) {
                int indexOUEvapCapCurve = GetCurveIndex(state, cAlphaArgs(Count2Index + 2 * NumCompSpd)); // convert curve name to index number
                if (indexOUEvapCapCurve == 0) {                                                           // Verify curve name and type
                    if (lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd)) {
                        ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrl.Name + "\", missing");
                        ShowContinueError(state, "...required " + cAlphaFieldNames(Count2Index + 2 * NumCompSpd) + " is blank.");
                    } else {
                        ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrl.Name + "\", invalid");
                        ShowContinueError(state,
                                          format("...not found {}=\"{}\".",
                                                 cAlphaFieldNames(Count2Index + 2 * NumCompSpd),
                                                 cAlphaArgs(Count2Index + 2 * NumCompSpd)));
                    }
                    ErrorsFound = true;
                } else {
                    ErrorsFound |= Curve::CheckCurveDims(state,
                                                         indexOUEvapCapCurve,                             // Curve index
                                                         {2},                                             // Valid dimensions
                                                         RoutineName,                                     // Routine name
                                                         cCurrentModuleObject,                            // Object Type
                                                         thisVrfFluidCtrl.Name,                           // Object Name
                                                         cAlphaFieldNames(Count2Index + 2 * NumCompSpd)); // Field Name

                    if (!ErrorsFound) {
                        thisVrfFluidCtrl.OUCoolingCAPFT(NumCompSpd) = indexOUEvapCapCurve;
                    }
                }
            }

            // Compressor Power Curve
            if (!lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd + 1)) {
                int indexOUCompPwrCurve = GetCurveIndex(state, cAlphaArgs(Count2Index + 2 * NumCompSpd + 1)); // convert curve name to index number
                if (indexOUCompPwrCurve == 0) {                                                               // Verify curve name and type
                    if (lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd + 1)) {
                        ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrl.Name + "\", missing");
                        ShowContinueError(state, "...required " + cAlphaFieldNames(Count2Index + 2 * NumCompSpd + 1) + " is blank.");
                    } else {
                        ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrl.Name + "\", invalid");
                        ShowContinueError(state,
                                          format("...not found {}=\"{}\".",
                                                 cAlphaFieldNames(Count2Index + 2 * NumCompSpd + 1),
                                                 cAlphaArgs(Count2Index + 2 * NumCompSpd + 1)));
                    }
                    ErrorsFound = true;
                } else {
                    ErrorsFound |= Curve::CheckCurveDims(state,
                                                         indexOUCompPwrCurve,                                 // Curve index
                                                         {2},                                                 // Valid dimensions
                                                         RoutineName,                                         // Routine name
                                                         cCurrentModuleObject,                                // Object Type
                                                         thisVrfFluidCtrl.Name,                               // Object Name
                                                         cAlphaFieldNames(Count2Index + 2 * NumCompSpd + 1)); // Field Name

                    if (!ErrorsFound) {
                        thisVrfFluidCtrl.OUCoolingPWRFT(NumCompSpd) = indexOUCompPwrCurve;
                    }
                }
            }
        }
    }

    // Read all VRF condenser objects: Algorithm Type 2_physics based model (VRF-FluidTCtrl-HR)
    cCurrentModuleObject = "AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl:HR";
    for (int thisNum = 1; thisNum <= state.dataHVACVarRefFlow->NumVRFCond_FluidTCtrl_HR; ++thisNum) {

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 thisNum,
                                                                 cAlphaArgs,
                                                                 NumAlphas,
                                                                 rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericFieldBlanks,
                                                                 lAlphaFieldBlanks,
                                                                 cAlphaFieldNames,
                                                                 cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

        GlobalNames::VerifyUniqueInterObjectName(
            state, state.dataHVACVarRefFlow->VrfUniqueNames, cAlphaArgs(1), cCurrentModuleObject, cAlphaFieldNames(1), ErrorsFound);

        int VRFNum = state.dataHVACVarRefFlow->NumVRFCond_SysCurve + state.dataHVACVarRefFlow->NumVRFCond_FluidTCtrl_HP + thisNum;
        auto &thisVrfFluidCtrlHR = state.dataHVACVarRefFlow->VRF(VRFNum);

        thisVrfFluidCtrlHR.Name = cAlphaArgs(1);

        thisVrfFluidCtrlHR.ThermostatPriority = ThermostatCtrlType::LoadPriority;
        thisVrfFluidCtrlHR.HeatRecoveryUsed = true;
        thisVrfFluidCtrlHR.VRFSystemTypeNum = VRF_HeatPump;
        thisVrfFluidCtrlHR.VRFAlgorithmType = AlgorithmType::FluidTCtrl;
        thisVrfFluidCtrlHR.fuel = Constant::eFuel::Electricity;

        if (lAlphaFieldBlanks(2)) {
            thisVrfFluidCtrlHR.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisVrfFluidCtrlHR.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
            ErrorsFound = true;
        }

        thisVrfFluidCtrlHR.ZoneTUListPtr =
            Util::FindItemInList(cAlphaArgs(3), state.dataHVACVarRefFlow->TerminalUnitList, state.dataHVACVarRefFlow->NumVRFTULists);
        if (thisVrfFluidCtrlHR.ZoneTUListPtr == 0) {
            ShowSevereError(state, cCurrentModuleObject + " = \"" + thisVrfFluidCtrlHR.Name + "\"");
            ShowContinueError(state, cAlphaFieldNames(3) + " = " + cAlphaArgs(3) + " not found.");
            ErrorsFound = true;
        }

        // Refrigerant type
        thisVrfFluidCtrlHR.refrigName = cAlphaArgs(4);
        if ((thisVrfFluidCtrlHR.refrig = Fluid::GetRefrig(state, thisVrfFluidCtrlHR.refrigName)) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(4), cAlphaArgs(4));
            ErrorsFound = true;
        }

        thisVrfFluidCtrlHR.RatedEvapCapacity = rNumericArgs(1);
        thisVrfFluidCtrlHR.RatedCompPowerPerCapcity = rNumericArgs(2);
        thisVrfFluidCtrlHR.RatedCompPower = thisVrfFluidCtrlHR.RatedCompPowerPerCapcity * thisVrfFluidCtrlHR.RatedEvapCapacity;
        thisVrfFluidCtrlHR.CoolingCapacity = thisVrfFluidCtrlHR.RatedEvapCapacity;
        thisVrfFluidCtrlHR.HeatingCapacity = thisVrfFluidCtrlHR.RatedEvapCapacity * (1 + thisVrfFluidCtrlHR.RatedCompPowerPerCapcity);
        thisVrfFluidCtrlHR.RatedHeatCapacity = thisVrfFluidCtrlHR.HeatingCapacity;

        // Reference system COP
        thisVrfFluidCtrlHR.CoolingCOP = 1 / thisVrfFluidCtrlHR.RatedCompPowerPerCapcity;
        thisVrfFluidCtrlHR.HeatingCOP = 1 / thisVrfFluidCtrlHR.RatedCompPowerPerCapcity + 1;

        // OA temperature range for VRF-HP operations
        thisVrfFluidCtrlHR.MinOATCooling = rNumericArgs(3);
        thisVrfFluidCtrlHR.MaxOATCooling = rNumericArgs(4);
        thisVrfFluidCtrlHR.MinOATHeating = rNumericArgs(5);
        thisVrfFluidCtrlHR.MaxOATHeating = rNumericArgs(6);
        thisVrfFluidCtrlHR.MinOATHeatRecovery = rNumericArgs(7);
        thisVrfFluidCtrlHR.MaxOATHeatRecovery = rNumericArgs(8);
        if (thisVrfFluidCtrlHR.MinOATCooling >= thisVrfFluidCtrlHR.MaxOATCooling) {
            ShowSevereError(state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrlHR.Name + "\"");
            ShowContinueError(state,
                              format("... {} ({:.3T}) must be less than maximum ({:.3T}).",
                                     cNumericFieldNames(3),
                                     thisVrfFluidCtrlHR.MinOATCooling,
                                     thisVrfFluidCtrlHR.MaxOATCooling));
            ErrorsFound = true;
        }
        if (thisVrfFluidCtrlHR.MinOATHeating >= thisVrfFluidCtrlHR.MaxOATHeating) {
            ShowSevereError(state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrlHR.Name + "\"");
            ShowContinueError(state,
                              format("... {} ({:.3T}) must be less than maximum ({:.3T}).",
                                     cNumericFieldNames(5),
                                     thisVrfFluidCtrlHR.MinOATHeating,
                                     thisVrfFluidCtrlHR.MaxOATHeating));
            ErrorsFound = true;
        }
        if (thisVrfFluidCtrlHR.MinOATHeatRecovery >= thisVrfFluidCtrlHR.MaxOATHeatRecovery) {
            ShowSevereError(state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrlHR.Name + "\"");
            ShowContinueError(state,
                              format("... {} ({:.3T}) must be less than maximum ({:.3T}).",
                                     cNumericFieldNames(7),
                                     thisVrfFluidCtrlHR.MinOATHeating,
                                     thisVrfFluidCtrlHR.MaxOATHeating));
            ErrorsFound = true;
        }
        if (thisVrfFluidCtrlHR.MinOATHeatRecovery < thisVrfFluidCtrlHR.MinOATCooling &&
            thisVrfFluidCtrlHR.MinOATHeatRecovery < thisVrfFluidCtrlHR.MinOATHeating) {
            ShowWarningError(state,
                             cCurrentModuleObject + " = \"" + thisVrfFluidCtrlHR.Name + "\", " + cNumericFieldNames(7) +
                                 " is less than the minimum temperature in heat pump mode.");
            ShowContinueError(state, format("...{} = {:.2T} C", cNumericFieldNames(7), thisVrfFluidCtrlHR.MinOATHeatRecovery));
            ShowContinueError(state, format("...Minimum Outdoor Temperature in Cooling Mode = {:.2T} C", thisVrfFluidCtrlHR.MinOATCooling));
            ShowContinueError(state, format("...Minimum Outdoor Temperature in Heating Mode = {:.2T} C", thisVrfFluidCtrlHR.MinOATHeating));
            ShowContinueError(state,
                              "...Minimum Outdoor Temperature in Heat Recovery Mode reset to lesser of cooling or heating minimum temperature "
                              "and simulation continues.");
            thisVrfFluidCtrlHR.MinOATHeatRecovery = min(thisVrfFluidCtrlHR.MinOATCooling, thisVrfFluidCtrlHR.MinOATHeating);
            ShowContinueError(state, format("... adjusted {} = {:.2T} C", cNumericFieldNames(7), thisVrfFluidCtrlHR.MinOATHeatRecovery));
        }
        if (thisVrfFluidCtrlHR.MaxOATHeatRecovery > thisVrfFluidCtrlHR.MaxOATCooling &&
            thisVrfFluidCtrlHR.MaxOATHeatRecovery > thisVrfFluidCtrlHR.MaxOATHeating) {
            ShowWarningError(state,
                             cCurrentModuleObject + " = \"" + thisVrfFluidCtrlHR.Name + "\", " + cNumericFieldNames(8) +
                                 " is greater than the maximum temperature in heat pump mode.");
            ShowContinueError(state, format("...{} = {:.2T} C", cNumericFieldNames(8), thisVrfFluidCtrlHR.MaxOATHeatRecovery));
            ShowContinueError(state, format("...Maximum Outdoor Temperature in Cooling Mode = {:.2T} C", thisVrfFluidCtrlHR.MaxOATCooling));
            ShowContinueError(state, format("...Maximum Outdoor Temperature in Heating Mode = {:.2T} C", thisVrfFluidCtrlHR.MaxOATHeating));
            ShowContinueError(state,
                              "...Maximum Outdoor Temperature in Heat Recovery Mode reset to greater of cooling or heating maximum temperature "
                              "and simulation continues.");
            thisVrfFluidCtrlHR.MaxOATHeatRecovery = max(thisVrfFluidCtrlHR.MaxOATCooling, thisVrfFluidCtrlHR.MaxOATHeating);
            ShowContinueError(state, format("... adjusted {} = {:.2T} C", cNumericFieldNames(8), thisVrfFluidCtrlHR.MaxOATHeatRecovery));
        }

        // IU Control Type
        if (Util::SameString(cAlphaArgs(5), "VariableTemp")) {
            thisVrfFluidCtrlHR.AlgorithmIUCtrl = 1;
        } else if (Util::SameString(cAlphaArgs(5), "ConstantTemp")) {
            thisVrfFluidCtrlHR.AlgorithmIUCtrl = 2;
        } else {
            thisVrfFluidCtrlHR.AlgorithmIUCtrl = 1;
        }

        // Reference IU Te/Tc for IU Control Algorithm: ConstantTemp
        thisVrfFluidCtrlHR.EvapTempFixed = rNumericArgs(9);
        thisVrfFluidCtrlHR.CondTempFixed = rNumericArgs(10);

        // Bounds of Te/Tc for IU Control Algorithm: VariableTemp
        thisVrfFluidCtrlHR.IUEvapTempLow = rNumericArgs(11);
        thisVrfFluidCtrlHR.IUEvapTempHigh = rNumericArgs(12);
        thisVrfFluidCtrlHR.IUCondTempLow = rNumericArgs(13);
        thisVrfFluidCtrlHR.IUCondTempHigh = rNumericArgs(14);
        if (thisVrfFluidCtrlHR.IUEvapTempLow >= thisVrfFluidCtrlHR.IUEvapTempHigh) {
            ShowSevereError(state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrlHR.Name + "\"");
            ShowContinueError(state,
                              format("... {} ({:.3T}) must be less than maximum ({:.3T}).",
                                     cNumericFieldNames(11),
                                     thisVrfFluidCtrlHR.IUEvapTempLow,
                                     thisVrfFluidCtrlHR.IUEvapTempHigh));
            ErrorsFound = true;
        }
        if (thisVrfFluidCtrlHR.IUCondTempLow >= thisVrfFluidCtrlHR.IUCondTempHigh) {
            ShowSevereError(state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrlHR.Name + "\"");
            ShowContinueError(state,
                              format("... {} ({:.3T}) must be less than maximum ({:.3T}).",
                                     cNumericFieldNames(13),
                                     thisVrfFluidCtrlHR.IUCondTempLow,
                                     thisVrfFluidCtrlHR.IUCondTempHigh));
            ErrorsFound = true;
        }

        // Reference OU SH/SC
        thisVrfFluidCtrlHR.SH = rNumericArgs(15);
        thisVrfFluidCtrlHR.SC = rNumericArgs(16);
        if (thisVrfFluidCtrlHR.SH > 20) {
            ShowWarningError(state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrlHR.Name + "\", \" " + cNumericFieldNames(15));
            ShowContinueError(state, "...is higher than 20C, which is usually the maximum of normal range.");
        }
        if (thisVrfFluidCtrlHR.SC > 20) {
            ShowWarningError(state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrlHR.Name + "\", \" " + cNumericFieldNames(15));
            ShowContinueError(state, "...is higher than 20C, which is usually the maximum of normal range.");
        }

        // OU Heat Exchanger Rated Bypass Factor
        thisVrfFluidCtrlHR.RateBFOUEvap = rNumericArgs(17);
        thisVrfFluidCtrlHR.RateBFOUCond = rNumericArgs(18);

        // Difference between Outdoor Unit Te and OAT during Simultaneous Heating and Cooling operations
        thisVrfFluidCtrlHR.DiffOUTeTo = rNumericArgs(19);

        // HR OU Heat Exchanger Capacity Ratio
        thisVrfFluidCtrlHR.HROUHexRatio = rNumericArgs(20);

        // Get OU fan data
        thisVrfFluidCtrlHR.RatedOUFanPowerPerCapcity = rNumericArgs(21);
        thisVrfFluidCtrlHR.OUAirFlowRatePerCapcity = rNumericArgs(22);
        thisVrfFluidCtrlHR.RatedOUFanPower = thisVrfFluidCtrlHR.RatedOUFanPowerPerCapcity * thisVrfFluidCtrlHR.RatedEvapCapacity;
        thisVrfFluidCtrlHR.OUAirFlowRate = thisVrfFluidCtrlHR.OUAirFlowRatePerCapcity * thisVrfFluidCtrlHR.RatedEvapCapacity;

        // OUEvapTempCurve
        int indexOUEvapTempCurve = GetCurveIndex(state, cAlphaArgs(6)); // convert curve name to index number
        // Verify curve name and type
        if (indexOUEvapTempCurve == 0) {
            if (lAlphaFieldBlanks(6)) {
                ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrlHR.Name + "\", missing");
                ShowContinueError(state, "...required " + cAlphaFieldNames(6) + " is blank.");
            } else {
                ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrlHR.Name + "\", invalid");
                ShowContinueError(state, "...not found " + cAlphaFieldNames(6) + "=\"" + cAlphaArgs(6) + "\".");
            }
            ErrorsFound = true;
        } else {
            if (state.dataCurveManager->curves(indexOUEvapTempCurve)->curveType == Curve::CurveType::Quadratic) {
                thisVrfFluidCtrlHR.C1Te = state.dataCurveManager->curves(indexOUEvapTempCurve)->coeff[0];
                thisVrfFluidCtrlHR.C2Te = state.dataCurveManager->curves(indexOUEvapTempCurve)->coeff[1];
                thisVrfFluidCtrlHR.C3Te = state.dataCurveManager->curves(indexOUEvapTempCurve)->coeff[2];
            } else {
                ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrlHR.Name + "\", invalid");
                ShowContinueError(state,
                                  format("...illegal {} type for this object = {}",
                                         cAlphaFieldNames(6),
                                         Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(indexOUEvapTempCurve)->curveType)]));
                ShowContinueError(state, "... Curve type must be Quadratic.");
                ErrorsFound = true;
            }
        }

        // OUCondTempCurve
        int indexOUCondTempCurve = GetCurveIndex(state, cAlphaArgs(7)); // convert curve name to index number
        // Verify curve name and type
        if (indexOUCondTempCurve == 0) {
            if (lAlphaFieldBlanks(7)) {
                ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrlHR.Name + "\", missing");
                ShowContinueError(state, "...required " + cAlphaFieldNames(7) + " is blank.");
            } else {
                ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrlHR.Name + "\", invalid");
                ShowContinueError(state, "...not found " + cAlphaFieldNames(7) + "=\"" + cAlphaArgs(7) + "\".");
            }
            ErrorsFound = true;
        } else {
            if (state.dataCurveManager->curves(indexOUCondTempCurve)->curveType == Curve::CurveType::Quadratic) {
                thisVrfFluidCtrlHR.C1Tc = state.dataCurveManager->curves(indexOUCondTempCurve)->coeff[0];
                thisVrfFluidCtrlHR.C2Tc = state.dataCurveManager->curves(indexOUCondTempCurve)->coeff[1];
                thisVrfFluidCtrlHR.C3Tc = state.dataCurveManager->curves(indexOUCondTempCurve)->coeff[2];
            } else {
                ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrlHR.Name + "\", invalid");
                ShowContinueError(state,
                                  format("...illegal {} type for this object = {}",
                                         cAlphaFieldNames(7),
                                         Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(indexOUCondTempCurve)->curveType)]));
                ShowContinueError(state, "... Curve type must be Quadratic.");
                ErrorsFound = true;
            }
        }

        // Pipe parameters
        thisVrfFluidCtrlHR.RefPipDiaSuc = rNumericArgs(23);
        thisVrfFluidCtrlHR.RefPipDiaDis = rNumericArgs(24);
        thisVrfFluidCtrlHR.RefPipLen = rNumericArgs(25);
        thisVrfFluidCtrlHR.RefPipEquLen = rNumericArgs(26);
        thisVrfFluidCtrlHR.RefPipHei = rNumericArgs(27);
        thisVrfFluidCtrlHR.RefPipInsThi = rNumericArgs(28);
        thisVrfFluidCtrlHR.RefPipInsCon = rNumericArgs(29);

        // Check the RefPipEquLen
        if (lNumericFieldBlanks(26) && !lNumericFieldBlanks(25)) {
            thisVrfFluidCtrlHR.RefPipEquLen = 1.2 * thisVrfFluidCtrlHR.RefPipLen;
            ShowWarningError(
                state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrlHR.Name + "\", \" " + cNumericFieldNames(26) + "\" is calculated based on");
            ShowContinueError(state, "...the provided \"" + cNumericFieldNames(25) + "\" value.");
        }
        if (thisVrfFluidCtrlHR.RefPipEquLen < thisVrfFluidCtrlHR.RefPipLen) {
            thisVrfFluidCtrlHR.RefPipEquLen = 1.2 * thisVrfFluidCtrlHR.RefPipLen;
            ShowWarningError(state,
                             cCurrentModuleObject + ", \"" + thisVrfFluidCtrlHR.Name + "\", invalid \" " + cNumericFieldNames(26) + "\" value.");
            ShowContinueError(state, "...Equivalent length of main pipe should be greater than or equal to the actual length.");
            ShowContinueError(state, format("...The value is recalculated based on the provided \"{}\" value.", cNumericFieldNames(25)));
        }

        // Crank case
        thisVrfFluidCtrlHR.CCHeaterPower = rNumericArgs(30);
        thisVrfFluidCtrlHR.NumCompressors = rNumericArgs(31);
        thisVrfFluidCtrlHR.CompressorSizeRatio = rNumericArgs(32);
        thisVrfFluidCtrlHR.MaxOATCCHeater = rNumericArgs(33);

        // Defrost
        if (!lAlphaFieldBlanks(8)) {
            if (Util::SameString(cAlphaArgs(8), "ReverseCycle")) {
                thisVrfFluidCtrlHR.DefrostStrategy = StandardRatings::DefrostStrat::ReverseCycle;
            }
            if (Util::SameString(cAlphaArgs(8), "Resistive")) {
                thisVrfFluidCtrlHR.DefrostStrategy = StandardRatings::DefrostStrat::Resistive;
            }
            if (thisVrfFluidCtrlHR.DefrostStrategy == StandardRatings::DefrostStrat::Invalid) {
                ShowSevereError(
                    state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrlHR.Name + "\" " + cAlphaFieldNames(8) + " not found: " + cAlphaArgs(8));
                ErrorsFound = true;
            }
        } else {
            thisVrfFluidCtrlHR.DefrostStrategy = StandardRatings::DefrostStrat::ReverseCycle;
        }

        if (!lAlphaFieldBlanks(9)) {
            if (Util::SameString(cAlphaArgs(9), "Timed")) {
                thisVrfFluidCtrlHR.DefrostControl = StandardRatings::HPdefrostControl::Timed;
            }
            if (Util::SameString(cAlphaArgs(9), "OnDemand")) {
                thisVrfFluidCtrlHR.DefrostControl = StandardRatings::HPdefrostControl::OnDemand;
            }
            if (thisVrfFluidCtrlHR.DefrostControl == StandardRatings::HPdefrostControl::Invalid) {
                ShowSevereError(
                    state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrlHR.Name + "\" " + cAlphaFieldNames(9) + " not found: " + cAlphaArgs(9));
                ErrorsFound = true;
            }
        } else {
            thisVrfFluidCtrlHR.DefrostControl = StandardRatings::HPdefrostControl::Timed;
        }

        if (!lAlphaFieldBlanks(10)) {
            thisVrfFluidCtrlHR.DefrostEIRPtr = GetCurveIndex(state, cAlphaArgs(10));
            if (thisVrfFluidCtrlHR.DefrostEIRPtr > 0) {
                // Verify Curve Object, expected type is BiQuadratic
                ErrorsFound |= Curve::CheckCurveDims(state,
                                                     thisVrfFluidCtrlHR.DefrostEIRPtr, // Curve index
                                                     {2},                              // Valid dimensions
                                                     RoutineName,                      // Routine name
                                                     cCurrentModuleObject,             // Object Type
                                                     thisVrfFluidCtrlHR.Name,          // Object Name
                                                     cAlphaFieldNames(10));            // Field Name
            } else {
                if (thisVrfFluidCtrlHR.DefrostStrategy == StandardRatings::DefrostStrat::ReverseCycle &&
                    thisVrfFluidCtrlHR.DefrostControl == StandardRatings::HPdefrostControl::OnDemand) {
                    ShowSevereError(state,
                                    cCurrentModuleObject + ", \"" + thisVrfFluidCtrlHR.Name + "\" " + cAlphaFieldNames(10) +
                                        " not found:" + cAlphaArgs(10));
                    ErrorsFound = true;
                }
            }
        } else {
            if (thisVrfFluidCtrlHR.DefrostStrategy == StandardRatings::DefrostStrat::ReverseCycle &&
                thisVrfFluidCtrlHR.DefrostControl == StandardRatings::HPdefrostControl::OnDemand) {
                ShowSevereError(
                    state, cCurrentModuleObject + ", \"" + thisVrfFluidCtrlHR.Name + "\" " + cAlphaFieldNames(10) + " not found:" + cAlphaArgs(10));
                ErrorsFound = true;
            }
        }

        thisVrfFluidCtrlHR.DefrostFraction = rNumericArgs(34);
        thisVrfFluidCtrlHR.DefrostCapacity = rNumericArgs(35);
        thisVrfFluidCtrlHR.MaxOATDefrost = rNumericArgs(36);
        if (thisVrfFluidCtrlHR.DefrostCapacity == 0.0 && thisVrfFluidCtrlHR.DefrostStrategy == StandardRatings::DefrostStrat::Resistive) {
            ShowWarningError(state,
                             cCurrentModuleObject + ", \"" + thisVrfFluidCtrlHR.Name + "\" " + cNumericFieldNames(35) +
                                 " = 0.0 for defrost strategy = RESISTIVE.");
        }

        // HR mode transition
        thisVrfFluidCtrlHR.HRInitialCoolCapFrac = rNumericArgs(37);
        thisVrfFluidCtrlHR.HRCoolCapTC = rNumericArgs(38);
        thisVrfFluidCtrlHR.HRInitialCoolEIRFrac = rNumericArgs(39);
        thisVrfFluidCtrlHR.HRCoolEIRTC = rNumericArgs(40);
        thisVrfFluidCtrlHR.HRInitialHeatCapFrac = rNumericArgs(41);
        thisVrfFluidCtrlHR.HRHeatCapTC = rNumericArgs(42);
        thisVrfFluidCtrlHR.HRInitialHeatEIRFrac = rNumericArgs(43);
        thisVrfFluidCtrlHR.HRHeatEIRTC = rNumericArgs(44);

        // Compressor configuration
        thisVrfFluidCtrlHR.CompMaxDeltaP = rNumericArgs(45);
        thisVrfFluidCtrlHR.EffCompInverter = rNumericArgs(46);
        thisVrfFluidCtrlHR.CoffEvapCap = rNumericArgs(47);

        // The new VRF model is Air cooled
        thisVrfFluidCtrlHR.CondenserType = DataHeatBalance::RefrigCondenserType::Air;
        thisVrfFluidCtrlHR.CondenserNodeNum = 0;

        // Evaporative Capacity & Compressor Power Curves corresponding to each Loading Index / compressor speed
        int NumOfCompSpd = rNumericArgs(48);
        thisVrfFluidCtrlHR.CompressorSpeed.dimension(NumOfCompSpd);
        thisVrfFluidCtrlHR.OUCoolingCAPFT.dimension(NumOfCompSpd);
        thisVrfFluidCtrlHR.OUCoolingPWRFT.dimension(NumOfCompSpd);
        int Count1Index = 48; // the index of the last numeric field before compressor speed entries
        int Count2Index = 9;  // the index of the last alpha field before capacity/power curves
        for (int NumCompSpd = 1; NumCompSpd <= NumOfCompSpd; NumCompSpd++) {
            thisVrfFluidCtrlHR.CompressorSpeed(NumCompSpd) = rNumericArgs(Count1Index + NumCompSpd);

            // Evaporating Capacity Curve
            if (!lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd)) {
                int indexOUEvapCapCurve = GetCurveIndex(state, cAlphaArgs(Count2Index + 2 * NumCompSpd)); // convert curve name to index number
                if (indexOUEvapCapCurve == 0) {                                                           // Verify curve name and type
                    if (lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd)) {
                        ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrlHR.Name + "\", missing");
                        ShowContinueError(state, "...required " + cAlphaFieldNames(Count2Index + 2 * NumCompSpd) + " is blank.");
                    } else {
                        ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrlHR.Name + "\", invalid");
                        ShowContinueError(state,
                                          format("...not found {}=\"{}\".",
                                                 cAlphaFieldNames(Count2Index + 2 * NumCompSpd),
                                                 cAlphaArgs(Count2Index + 2 * NumCompSpd)));
                    }
                    ErrorsFound = true;
                } else {
                    ErrorsFound |= Curve::CheckCurveDims(state,
                                                         indexOUEvapCapCurve,                             // Curve index
                                                         {2},                                             // Valid dimensions
                                                         RoutineName,                                     // Routine name
                                                         cCurrentModuleObject,                            // Object Type
                                                         thisVrfFluidCtrlHR.Name,                         // Object Name
                                                         cAlphaFieldNames(Count2Index + 2 * NumCompSpd)); // Field Name

                    if (!ErrorsFound) {
                        thisVrfFluidCtrlHR.OUCoolingCAPFT(NumCompSpd) = indexOUEvapCapCurve;
                    }
                }
            }

            // Compressor Power Curve
            if (!lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd + 1)) {
                int indexOUCompPwrCurve = GetCurveIndex(state, cAlphaArgs(Count2Index + 2 * NumCompSpd + 1)); // convert curve name to index number
                if (indexOUCompPwrCurve == 0) {                                                               // Verify curve name and type
                    if (lAlphaFieldBlanks(Count2Index + 2 * NumCompSpd + 1)) {
                        ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrlHR.Name + "\", missing");
                        ShowContinueError(state, "...required " + cAlphaFieldNames(Count2Index + 2 * NumCompSpd + 1) + " is blank.");
                    } else {
                        ShowSevereError(state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVrfFluidCtrlHR.Name + "\", invalid");
                        ShowContinueError(state,
                                          format("...not found {}=\"{}\".",
                                                 cAlphaFieldNames(Count2Index + 2 * NumCompSpd + 1),
                                                 cAlphaArgs(Count2Index + 2 * NumCompSpd + 1)));
                    }
                    ErrorsFound = true;
                } else {
                    ErrorsFound |= Curve::CheckCurveDims(state,
                                                         indexOUCompPwrCurve,                                 // Curve index
                                                         {2},                                                 // Valid dimensions
                                                         RoutineName,                                         // Routine name
                                                         cCurrentModuleObject,                                // Object Type
                                                         thisVrfFluidCtrlHR.Name,                             // Object Name
                                                         cAlphaFieldNames(Count2Index + 2 * NumCompSpd + 1)); // Field Name

                    if (!ErrorsFound) {
                        thisVrfFluidCtrlHR.OUCoolingPWRFT(NumCompSpd) = indexOUCompPwrCurve;
                    }
                }
            }
        }
    }

    cCurrentModuleObject = "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow";
    for (int VRFTUNum = 1; VRFTUNum <= state.dataHVACVarRefFlow->NumVRFTU; ++VRFTUNum) {

        //     initialize local node number variables
        int CCoilInletNodeNum = 0;
        int CCoilOutletNodeNum = 0;
        int HCoilInletNodeNum = 0;
        int HCoilOutletNodeNum = 0;
        OANodeNums = 0;

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 VRFTUNum,
                                                                 cAlphaArgs,
                                                                 NumAlphas,
                                                                 rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericFieldBlanks,
                                                                 lAlphaFieldBlanks,
                                                                 cAlphaFieldNames,
                                                                 cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

        state.dataHVACVarRefFlow->VRFTUNumericFields(VRFTUNum).FieldNames.allocate(NumNums);
        state.dataHVACVarRefFlow->VRFTUNumericFields(VRFTUNum).FieldNames = cNumericFieldNames;
        Util::IsNameEmpty(state, cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

        auto &thisVrfTU = state.dataHVACVarRefFlow->VRFTU(VRFTUNum);
        thisVrfTU.Name = cAlphaArgs(1);
        for (int NumList = 1; NumList <= state.dataHVACVarRefFlow->NumVRFTULists; ++NumList) {
            int ZoneTerminalUnitListNum = Util::FindItemInList(thisVrfTU.Name,
                                                               state.dataHVACVarRefFlow->TerminalUnitList(NumList).ZoneTUName,
                                                               state.dataHVACVarRefFlow->TerminalUnitList(NumList).NumTUInList);
            if (ZoneTerminalUnitListNum > 0) {
                thisVrfTU.IndexToTUInTUList = ZoneTerminalUnitListNum;
                state.dataHVACVarRefFlow->TerminalUnitList(NumList).ZoneTUPtr(ZoneTerminalUnitListNum) = VRFTUNum;
                thisVrfTU.TUListIndex = NumList;
                break;
            }
        }
        if (thisVrfTU.TUListIndex == 0) {
            ShowSevereError(state, cCurrentModuleObject + " = " + thisVrfTU.Name);
            ShowContinueError(state, "Terminal unit not found on any ZoneTerminalUnitList.");
            ErrorsFound = true;
        }

        for (int NumCond = 1; NumCond <= state.dataHVACVarRefFlow->NumVRFCond; ++NumCond) {
            if (state.dataHVACVarRefFlow->VRF(NumCond).ZoneTUListPtr != thisVrfTU.TUListIndex) {
                continue;
            }
            thisVrfTU.VRFSysNum = NumCond;
            break;
        }
        thisVrfTU.type = TUType::ConstantVolume;
        if (lAlphaFieldBlanks(2)) {
            thisVrfTU.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisVrfTU.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
            ErrorsFound = true;
        }

        thisVrfTU.VRFTUInletNodeNum = GetOnlySingleNode(state,
                                                        cAlphaArgs(3),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::ZoneHVACTerminalUnitVariableRefrigerantFlow,
                                                        thisVrfTU.Name,
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::Inlet,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        ObjectIsParent);

        thisVrfTU.VRFTUOutletNodeNum = GetOnlySingleNode(state,
                                                         cAlphaArgs(4),
                                                         ErrorsFound,
                                                         DataLoopNode::ConnectionObjectType::ZoneHVACTerminalUnitVariableRefrigerantFlow,
                                                         thisVrfTU.Name,
                                                         DataLoopNode::NodeFluidType::Air,
                                                         DataLoopNode::ConnectionType::Outlet,
                                                         NodeInputManager::CompFluidStream::Primary,
                                                         ObjectIsParent);

        thisVrfTU.MaxCoolAirVolFlow = rNumericArgs(1);
        thisVrfTU.MaxNoCoolAirVolFlow = rNumericArgs(2);
        thisVrfTU.MaxHeatAirVolFlow = rNumericArgs(3);
        thisVrfTU.MaxNoHeatAirVolFlow = rNumericArgs(4);
        thisVrfTU.CoolOutAirVolFlow = rNumericArgs(5);
        thisVrfTU.HeatOutAirVolFlow = rNumericArgs(6);
        thisVrfTU.NoCoolHeatOutAirVolFlow = rNumericArgs(7);

        if (lAlphaFieldBlanks(5)) {
            thisVrfTU.fanOp = HVAC::FanOp::Continuous;
        } else if ((thisVrfTU.fanOpModeSched = Sched::GetSchedule(state, cAlphaArgs(5))) == nullptr) {
            ShowWarningItemNotFound(
                state, eoh, cAlphaFieldNames(5), cAlphaArgs(5), "Defaulting to constant fan operating mode and simulation continues.");
            thisVrfTU.fanOp = HVAC::FanOp::Continuous;
        }

        thisVrfTU.fanPlace = static_cast<HVAC::FanPlace>(getEnumValue(HVAC::fanPlaceNamesUC, cAlphaArgs(6)));
        assert(thisVrfTU.fanPlace != HVAC::FanPlace::Invalid);

        if (!lAlphaFieldBlanks(7) && !lAlphaFieldBlanks(8)) {
            // Get fan data
            std::string FanName = cAlphaArgs(8);

            thisVrfTU.fanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, cAlphaArgs(7)));

            if (thisVrfTU.fanType != HVAC::FanType::OnOff && thisVrfTU.fanType != HVAC::FanType::Constant &&
                thisVrfTU.fanType != HVAC::FanType::VAV && thisVrfTU.fanType != HVAC::FanType::SystemModel) {
                ShowSevereInvalidKey(state,
                                     eoh,
                                     cAlphaFieldNames(7),
                                     cAlphaArgs(7),
                                     "Only types Fan:ConstantVolume, Fan:VAV, Fan:OnOff, and Fan:SystemModel are supported");
                ErrorsFound = true;
            }

            if ((thisVrfTU.FanIndex = Fans::GetFanIndex(state, FanName)) == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(8), FanName);
                ErrorsFound = true;

            } else if (thisVrfTU.fanType != state.dataFans->fans(thisVrfTU.FanIndex)->type) {
                ShowSevereError(state, cCurrentModuleObject + " = " + thisVrfTU.Name);
                ShowContinueError(state, "Fan type specified = " + cAlphaArgs(7));
                ShowContinueError(state, format("Actual type of fan {} = {}", FanName, HVAC::fanTypeNames[(int)thisVrfTU.fanType]));
                ErrorsFound = true;
            }

            if (thisVrfTU.VRFSysNum > 0) {
                // VRFTU Supply Air Fan Object Type must be Fan:VariableVolume if VRF Algorithm Type is AlgorithmTypeFluidTCtrl
                if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).VRFAlgorithmType == AlgorithmType::FluidTCtrl &&
                    !(thisVrfTU.fanType == HVAC::FanType::VAV || thisVrfTU.fanType == HVAC::FanType::SystemModel)) {
                    ShowSevereError(state, cCurrentModuleObject + " = " + thisVrfTU.Name);
                    ShowContinueError(state, "Fan type specified = " + cAlphaArgs(7));
                    ShowContinueError(
                        state, "Fan Object Type must be Fan:VariableVolume if VRF AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl");
                    ShowContinueError(state, "is used to model VRF outdoor unit.");
                    ErrorsFound = true;
                }
                // VRFTU Supply Air Fan Object Type must be Fan:OnOff or Fan:ConstantVolume if VRF Algorithm Type is AlgorithmTypeSysCurve
                if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).VRFAlgorithmType == AlgorithmType::SysCurve &&
                    !(thisVrfTU.fanType == HVAC::FanType::OnOff || thisVrfTU.fanType == HVAC::FanType::Constant ||
                      thisVrfTU.fanType == HVAC::FanType::SystemModel)) {
                    ShowSevereError(state, cCurrentModuleObject + " = " + thisVrfTU.Name);
                    ShowContinueError(state, "Fan type specified = " + cAlphaArgs(7));
                    ShowContinueError(state,
                                      "Fan Object Type must be Fan:SystemModel, Fan:OnOff, or Fan:ConstantVolume if VRF "
                                      "AirConditioner:VariableRefrigerantFlow");
                    ShowContinueError(state, "is used to model VRF outdoor unit.");
                    ErrorsFound = true;
                }
            }

            auto *fan = state.dataFans->fans(thisVrfTU.FanIndex);

            thisVrfTU.fanInletNode = fan->inletNodeNum;
            thisVrfTU.fanOutletNode = fan->outletNodeNum;

            Real64 FanVolFlowRate = fan->maxAirFlowRate;
            thisVrfTU.ActualFanVolFlowRate = FanVolFlowRate;
            int FanInletNodeNum = fan->inletNodeNum;
            int FanOutletNodeNum = fan->outletNodeNum;
            thisVrfTU.fanAvailSched = fan->availSched;

            // Check fan's schedule for cycling fan operation if constant volume fan is used
            if (thisVrfTU.fanOpModeSched != nullptr && thisVrfTU.fanType == HVAC::FanType::Constant) {
                if (!thisVrfTU.fanOpModeSched->checkMinMaxVals(state, Clusive::Ex, 0.0, Clusive::In, 1.0)) {
                    Sched::ShowSevereBadMinMax(state,
                                               eoh,
                                               cAlphaFieldNames(5),
                                               cAlphaArgs(5),
                                               Clusive::Ex,
                                               0.0,
                                               Clusive::In,
                                               1.0,
                                               format("For fan type = {}, operating mode must be continuous (schedule values > 0).",
                                                      HVAC::fanTypeNames[(int)HVAC::FanType::Constant]));
                    ErrorsFound = true;
                }
            } // IF (FanType_Num == HVAC::FanType_SimpleOnOff .OR. FanType_Num == HVAC::FanType_SimpleConstVolume)THEN

            // Add TU to component sets array
            SetUpCompSets(state,
                          cCurrentModuleObject,
                          thisVrfTU.Name,
                          HVAC::fanTypeNames[(int)thisVrfTU.fanType],
                          FanName,
                          state.dataLoopNodes->NodeID(FanInletNodeNum),
                          state.dataLoopNodes->NodeID(FanOutletNodeNum));

        } else {
            thisVrfTU.fanPlace = HVAC::FanPlace::Invalid; // reset fan placement when fan is not used so as not to call the fan
            thisVrfTU.fanAvailSched =
                Sched::GetScheduleAlwaysOn(state); // A missing fan is the same as a fan that is always on for availability purposes
        }

        // Get OA mixer data
        std::string OAMixerType = cAlphaArgs(9);

        if (!lAlphaFieldBlanks(10)) {
            thisVrfTU.OAMixerName = cAlphaArgs(10);
            errFlag = false;
            OANodeNums = GetOAMixerNodeNumbers(state, thisVrfTU.OAMixerName, errFlag);

            //       OANodeNums(1) = OAMixer(OAMixerNum)%InletNode
            //       OANodeNums(2) = OAMixer(OAMixerNum)%RelNode
            //       OANodeNums(3) = OAMixer(OAMixerNum)%RetNode
            //       OANodeNums(4) = OAMixer(OAMixerNum)%MixNode

            if (errFlag) {
                ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                ErrorsFound = true;
            } else {
                thisVrfTU.OAMixerUsed = true;
            }
            thisVrfTU.VRFTUOAMixerOANodeNum = OANodeNums(1);
            thisVrfTU.VRFTUOAMixerRelNodeNum = OANodeNums(2);
            thisVrfTU.VRFTUOAMixerRetNodeNum = OANodeNums(3);
            thisVrfTU.VRFTUOAMixerMixedNodeNum = OANodeNums(4);
        }

        // Get DX cooling coil data
        std::string DXCoolingCoilType = cAlphaArgs(11);

        errFlag = false;
        thisVrfTU.DXCoolCoilType_Num = GetCoilTypeNum(state, DXCoolingCoilType, cAlphaArgs(12), errFlag, false);
        if (thisVrfTU.DXCoolCoilType_Num == 0) {
            thisVrfTU.CoolingCoilPresent = false;
            if (thisVrfTU.TUListIndex > 0 && thisVrfTU.IndexToTUInTUList > 0) {
                state.dataHVACVarRefFlow->TerminalUnitList(thisVrfTU.TUListIndex).CoolingCoilPresent(thisVrfTU.IndexToTUInTUList) = false;
            }
        } else {
            if (thisVrfTU.VRFSysNum > 0) {
                if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                    // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control

                    if (Util::SameString(HVAC::cAllCoilTypes(thisVrfTU.DXCoolCoilType_Num), HVAC::cAllCoilTypes(HVAC::CoilVRF_FluidTCtrl_Cooling))) {
                        errFlag = false;
                        if (thisVrfTU.TUListIndex > 0 && thisVrfTU.IndexToTUInTUList > 0) {
                            state.dataHVACVarRefFlow->TerminalUnitList(thisVrfTU.TUListIndex).coolingCoilAvailScheds(thisVrfTU.IndexToTUInTUList) =
                                DXCoils::GetDXCoilAvailSched(state, DXCoolingCoilType, cAlphaArgs(12), errFlag);
                        }
                        GetDXCoilIndex(
                            state, cAlphaArgs(12), thisVrfTU.CoolCoilIndex, errFlag, HVAC::cAllCoilTypes(HVAC::CoilVRF_FluidTCtrl_Cooling));
                        CCoilInletNodeNum =
                            DXCoils::GetCoilInletNode(state, HVAC::cAllCoilTypes(HVAC::CoilVRF_FluidTCtrl_Cooling), cAlphaArgs(12), errFlag);
                        CCoilOutletNodeNum =
                            DXCoils::GetCoilOutletNode(state, HVAC::cAllCoilTypes(HVAC::CoilVRF_FluidTCtrl_Cooling), cAlphaArgs(12), errFlag);
                        thisVrfTU.coolCoilAirInNode = CCoilInletNodeNum;
                        thisVrfTU.coolCoilAirOutNode = CCoilOutletNodeNum;

                        if (errFlag) {
                            ShowContinueError(state, "...occurs in " + cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                        }

                        if (thisVrfTU.VRFSysNum > 0) {
                            SetDXCoolingCoilData(
                                state, thisVrfTU.CoolCoilIndex, ErrorsFound, _, state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).CondenserType);
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.CoolCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).CondenserNodeNum);
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.CoolCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).MaxOATCCHeater);
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.CoolCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).MinOATCooling);
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.CoolCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).MaxOATCooling);

                            state.dataDXCoils->DXCoil(thisVrfTU.CoolCoilIndex).VRFIUPtr = VRFTUNum;
                            state.dataDXCoils->DXCoil(thisVrfTU.CoolCoilIndex).VRFOUPtr = thisVrfTU.VRFSysNum;
                            state.dataDXCoils->DXCoil(thisVrfTU.CoolCoilIndex).SupplyFanIndex = thisVrfTU.FanIndex;

                            if (thisVrfTU.FanIndex > 0) {
                                state.dataDXCoils->DXCoil(thisVrfTU.CoolCoilIndex).RatedAirVolFlowRate(1) =
                                    state.dataFans->fans(thisVrfTU.FanIndex)->maxAirFlowRate;
                            } else {
                                state.dataDXCoils->DXCoil(thisVrfTU.CoolCoilIndex).RatedAirVolFlowRate(1) = AutoSize;
                            }

                        } else {
                            ShowSevereError(state, cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                            ShowContinueError(
                                state, "... when checking " + HVAC::cAllCoilTypes(thisVrfTU.DXCoolCoilType_Num) + " \"" + cAlphaArgs(12) + "\"");
                            ShowContinueError(state, "... terminal unit not connected to condenser.");
                            ShowContinueError(state, "... check that terminal unit is specified in a terminal unit list object.");
                            ShowContinueError(state,
                                              "... also check that the terminal unit list name is specified in an "
                                              "AirConditioner:VariableRefrigerantFlow object.");
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                        ShowContinueError(state, "... illegal " + cAlphaFieldNames(12) + " = " + cAlphaArgs(12));
                        ErrorsFound = true;
                    }

                } else {
                    // Algorithm Type: VRF model based on system curve

                    if (Util::SameString(HVAC::cAllCoilTypes(thisVrfTU.DXCoolCoilType_Num), HVAC::cAllCoilTypes(HVAC::CoilVRF_Cooling))) {
                        if (thisVrfTU.TUListIndex > 0 && thisVrfTU.IndexToTUInTUList > 0) {
                            state.dataHVACVarRefFlow->TerminalUnitList(thisVrfTU.TUListIndex).coolingCoilAvailScheds(thisVrfTU.IndexToTUInTUList) =
                                DXCoils::GetDXCoilAvailSched(state, DXCoolingCoilType, cAlphaArgs(12), errFlag);
                        } else {
                            thisVrfTU.CoolingCoilPresent = false;
                        }
                        errFlag = false;
                        GetDXCoilIndex(state, cAlphaArgs(12), thisVrfTU.CoolCoilIndex, errFlag, HVAC::cAllCoilTypes(HVAC::CoilVRF_Cooling));
                        CCoilInletNodeNum = DXCoils::GetCoilInletNode(state, HVAC::cAllCoilTypes(HVAC::CoilVRF_Cooling), cAlphaArgs(12), errFlag);
                        CCoilOutletNodeNum = DXCoils::GetCoilOutletNode(state, HVAC::cAllCoilTypes(HVAC::CoilVRF_Cooling), cAlphaArgs(12), errFlag);
                        thisVrfTU.coolCoilAirInNode = CCoilInletNodeNum;
                        thisVrfTU.coolCoilAirOutNode = CCoilOutletNodeNum;

                        if (errFlag) {
                            ShowContinueError(state, "...occurs in " + cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                        }

                        SetDXCoolingCoilData(
                            state, thisVrfTU.CoolCoilIndex, ErrorsFound, _, state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).CondenserType);
                        SetDXCoolingCoilData(
                            state, thisVrfTU.CoolCoilIndex, ErrorsFound, _, _, state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).CondenserNodeNum);
                        SetDXCoolingCoilData(
                            state, thisVrfTU.CoolCoilIndex, ErrorsFound, _, _, _, state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).MaxOATCCHeater);
                        SetDXCoolingCoilData(state,
                                             thisVrfTU.CoolCoilIndex,
                                             ErrorsFound,
                                             _,
                                             _,
                                             _,
                                             _,
                                             state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).MinOATCooling);
                        SetDXCoolingCoilData(state,
                                             thisVrfTU.CoolCoilIndex,
                                             ErrorsFound,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).MaxOATCooling);

                    } else {
                        ShowSevereError(state, cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                        ShowContinueError(state, "... illegal " + cAlphaFieldNames(12) + " = " + cAlphaArgs(12));
                        ErrorsFound = true;
                    }
                }
            } else {
                ShowSevereError(state, cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                ShowContinueError(state, "... when checking " + HVAC::cAllCoilTypes(thisVrfTU.DXCoolCoilType_Num) + " \"" + cAlphaArgs(12) + "\"");
                ShowContinueError(state, "... terminal unit not connected to condenser.");
                ShowContinueError(state, "... check that terminal unit is specified in a terminal unit list object.");
                ShowContinueError(
                    state, "... also check that the terminal unit list name is specified in an AirConditioner:VariableRefrigerantFlow object.");
                ErrorsFound = true;
            }
        }

        // Get DX heating coil data
        std::string DXHeatingCoilType = cAlphaArgs(13);

        // Get the heating to cooling sizing ratio input before writing to DX heating coil data
        if (!lNumericFieldBlanks(10)) {
            thisVrfTU.HeatingCapacitySizeRatio = rNumericArgs(10);
        }

        errFlag = false;
        thisVrfTU.DXHeatCoilType_Num = GetCoilTypeNum(state, DXHeatingCoilType, cAlphaArgs(14), errFlag, false);
        if (thisVrfTU.DXHeatCoilType_Num == 0) {
            thisVrfTU.HeatingCoilPresent = false;
            if (thisVrfTU.TUListIndex > 0 && thisVrfTU.IndexToTUInTUList > 0) {
                state.dataHVACVarRefFlow->TerminalUnitList(thisVrfTU.TUListIndex).HeatingCoilPresent(thisVrfTU.IndexToTUInTUList) = false;
            }
        } else {
            if (thisVrfTU.VRFSysNum > 0) {
                if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                    // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control

                    if (Util::SameString(HVAC::cAllCoilTypes(thisVrfTU.DXHeatCoilType_Num), HVAC::cAllCoilTypes(HVAC::CoilVRF_FluidTCtrl_Heating))) {
                        errFlag = false;
                        if (thisVrfTU.TUListIndex > 0 && thisVrfTU.IndexToTUInTUList > 0) {
                            state.dataHVACVarRefFlow->TerminalUnitList(thisVrfTU.TUListIndex).heatingCoilAvailScheds(thisVrfTU.IndexToTUInTUList) =
                                DXCoils::GetDXCoilAvailSched(state, DXHeatingCoilType, cAlphaArgs(14), errFlag);
                        }
                        GetDXCoilIndex(
                            state, cAlphaArgs(14), thisVrfTU.HeatCoilIndex, errFlag, HVAC::cAllCoilTypes(HVAC::CoilVRF_FluidTCtrl_Heating));
                        HCoilInletNodeNum =
                            DXCoils::GetCoilInletNode(state, HVAC::cAllCoilTypes(HVAC::CoilVRF_FluidTCtrl_Heating), cAlphaArgs(14), errFlag);
                        HCoilOutletNodeNum =
                            DXCoils::GetCoilOutletNode(state, HVAC::cAllCoilTypes(HVAC::CoilVRF_FluidTCtrl_Heating), cAlphaArgs(14), errFlag);
                        thisVrfTU.heatCoilAirInNode = HCoilInletNodeNum;
                        thisVrfTU.heatCoilAirOutNode = HCoilOutletNodeNum;

                        if (errFlag) {
                            ShowContinueError(state, "...occurs in " + cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                        }

                        if (thisVrfTU.VRFSysNum > 0) {
                            SetDXCoolingCoilData(
                                state, thisVrfTU.HeatCoilIndex, ErrorsFound, _, state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).CondenserType);
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).CondenserNodeNum);
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).MaxOATCCHeater);
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).MinOATHeating);
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).MaxOATHeating);
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).HeatingPerformanceOATType);
                            // Set defrost controls in child object to trip child object defrost calculations
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).DefrostStrategy);
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).DefrostControl);
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).DefrostEIRPtr);
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).DefrostFraction);
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).MaxOATDefrost);
                            // If defrost is disabled in the VRF condenser, it must be disabled in the DX coil
                            // Defrost primarily handled in parent object, set defrost capacity to 1 to avoid autosizing.
                            // Defrost capacity is used for nothing more than setting defrost power/consumption report
                            // variables which are not reported. The coil's defrost algorithm IS used to derate the coil
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 1.0); // DefrostCapacity=1.0

                            state.dataDXCoils->DXCoil(thisVrfTU.HeatCoilIndex).VRFIUPtr = VRFTUNum;
                            state.dataDXCoils->DXCoil(thisVrfTU.HeatCoilIndex).VRFOUPtr = thisVrfTU.VRFSysNum;
                            state.dataDXCoils->DXCoil(thisVrfTU.HeatCoilIndex).SupplyFanIndex = thisVrfTU.FanIndex;

                            if (thisVrfTU.FanIndex > 0) {
                                state.dataDXCoils->DXCoil(thisVrfTU.HeatCoilIndex).RatedAirVolFlowRate(1) =
                                    state.dataFans->fans(thisVrfTU.FanIndex)->maxAirFlowRate;
                            } else {
                                state.dataDXCoils->DXCoil(thisVrfTU.HeatCoilIndex).RatedAirVolFlowRate(1) = AutoSize;
                            }

                            // Terminal unit heating to cooling sizing ratio has precedence over VRF system sizing ratio
                            if (thisVrfTU.HeatingCapacitySizeRatio > 1.0) {
                                SetDXCoolingCoilData(state,
                                                     thisVrfTU.HeatCoilIndex,
                                                     ErrorsFound,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     thisVrfTU.HeatingCapacitySizeRatio);
                            } else if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).HeatingCapacitySizeRatio > 1.0) {
                                SetDXCoolingCoilData(state,
                                                     thisVrfTU.HeatCoilIndex,
                                                     ErrorsFound,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     _,
                                                     state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).HeatingCapacitySizeRatio);
                            }
                        } else {
                            ShowSevereError(state, cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                            ShowContinueError(
                                state, "... when checking " + HVAC::cAllCoilTypes(thisVrfTU.DXHeatCoilType_Num) + " \"" + cAlphaArgs(14) + "\"");
                            ShowContinueError(state, "... terminal unit not connected to condenser.");
                            ShowContinueError(state, "... check that terminal unit is specified in a terminal unit list object.");
                            ShowContinueError(state,
                                              "... also check that the terminal unit list name is specified in an "
                                              "AirConditioner:VariableRefrigerantFlow object.");
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                        ShowContinueError(state, "... illegal " + cAlphaFieldNames(14) + " = " + cAlphaArgs(14));
                        ErrorsFound = true;
                    }

                } else {
                    // Algorithm Type: VRF model based on system curve
                    if (Util::SameString(HVAC::cAllCoilTypes(thisVrfTU.DXHeatCoilType_Num), HVAC::cAllCoilTypes(HVAC::CoilVRF_Heating))) {
                        if (thisVrfTU.TUListIndex > 0 && thisVrfTU.IndexToTUInTUList > 0) {
                            state.dataHVACVarRefFlow->TerminalUnitList(thisVrfTU.TUListIndex).heatingCoilAvailScheds(thisVrfTU.IndexToTUInTUList) =
                                DXCoils::GetDXCoilAvailSched(state, DXHeatingCoilType, cAlphaArgs(14), errFlag);
                        } else {
                            thisVrfTU.HeatingCoilPresent = false;
                        }
                        errFlag = false;
                        GetDXCoilIndex(state, cAlphaArgs(14), thisVrfTU.HeatCoilIndex, errFlag, HVAC::cAllCoilTypes(HVAC::CoilVRF_Heating));
                        HCoilInletNodeNum = DXCoils::GetCoilInletNode(state, HVAC::cAllCoilTypes(HVAC::CoilVRF_Heating), cAlphaArgs(14), errFlag);
                        HCoilOutletNodeNum = DXCoils::GetCoilOutletNode(state, HVAC::cAllCoilTypes(HVAC::CoilVRF_Heating), cAlphaArgs(14), errFlag);
                        thisVrfTU.heatCoilAirInNode = HCoilInletNodeNum;
                        thisVrfTU.heatCoilAirOutNode = HCoilOutletNodeNum;

                        if (errFlag) {
                            ShowContinueError(state, "...occurs in " + cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                        }

                        SetDXCoolingCoilData(
                            state, thisVrfTU.HeatCoilIndex, ErrorsFound, _, state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).CondenserType);
                        SetDXCoolingCoilData(
                            state, thisVrfTU.HeatCoilIndex, ErrorsFound, _, _, state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).CondenserNodeNum);
                        SetDXCoolingCoilData(
                            state, thisVrfTU.HeatCoilIndex, ErrorsFound, _, _, _, state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).MaxOATCCHeater);
                        SetDXCoolingCoilData(state,
                                             thisVrfTU.HeatCoilIndex,
                                             ErrorsFound,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).MinOATHeating);
                        SetDXCoolingCoilData(state,
                                             thisVrfTU.HeatCoilIndex,
                                             ErrorsFound,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).HeatingPerformanceOATType);
                        // Set defrost controls in child object to trip child object defrost calculations
                        SetDXCoolingCoilData(state,
                                             thisVrfTU.HeatCoilIndex,
                                             ErrorsFound,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).DefrostStrategy);
                        SetDXCoolingCoilData(state,
                                             thisVrfTU.HeatCoilIndex,
                                             ErrorsFound,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).DefrostControl);
                        SetDXCoolingCoilData(state,
                                             thisVrfTU.HeatCoilIndex,
                                             ErrorsFound,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).DefrostEIRPtr);
                        SetDXCoolingCoilData(state,
                                             thisVrfTU.HeatCoilIndex,
                                             ErrorsFound,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).DefrostFraction);
                        SetDXCoolingCoilData(state,
                                             thisVrfTU.HeatCoilIndex,
                                             ErrorsFound,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).MaxOATDefrost);
                        // If defrost is disabled in the VRF condenser, it must be disabled in the DX coil
                        // Defrost primarily handled in parent object, set defrost capacity to 1 to avoid autosizing.
                        // Defrost capacity is used for nothing more than setting defrost power/consumption report
                        // variables which are not reported. The coil's defrost algorithm IS used to derate the coil
                        SetDXCoolingCoilData(state,
                                             thisVrfTU.HeatCoilIndex,
                                             ErrorsFound,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             _,
                                             1.0); // DefrostCapacity=1.0
                        // Terminal unit heating to cooling sizing ratio has precedence over VRF system sizing ratio
                        if (thisVrfTU.HeatingCapacitySizeRatio > 1.0) {
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 thisVrfTU.HeatingCapacitySizeRatio);
                        } else if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).HeatingCapacitySizeRatio > 1.0) {
                            SetDXCoolingCoilData(state,
                                                 thisVrfTU.HeatCoilIndex,
                                                 ErrorsFound,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 _,
                                                 state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).HeatingCapacitySizeRatio);
                        }
                        // Check VRF DX heating coil heating capacity as a function of temperature performance curve. Only report here for
                        // biquadratic curve type.
                        if (thisVrfTU.VRFSysNum > 0 && thisVrfTU.HeatCoilIndex > 0 &&
                            state.dataCurveManager->curves(GetDXCoilCapFTCurveIndex(state, thisVrfTU.HeatCoilIndex, ErrorsFound))->numDims == 2) {
                            if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).HeatingPerformanceOATType == HVAC::OATType::WetBulb) {
                                checkCurveIsNormalizedToOne(
                                    state,
                                    "GetDXCoils: " + HVAC::cAllCoilTypes(thisVrfTU.DXHeatCoilType_Num),
                                    DXCoils::GetDXCoilName(
                                        state, thisVrfTU.HeatCoilIndex, ErrorsFound, HVAC::cAllCoilTypes(thisVrfTU.DXHeatCoilType_Num)),
                                    GetDXCoilCapFTCurveIndex(state, thisVrfTU.HeatCoilIndex, ErrorsFound),
                                    "Heating Capacity Ratio Modifier Function of Temperature Curve Name",
                                    Curve::GetCurveName(state, GetDXCoilCapFTCurveIndex(state, thisVrfTU.HeatCoilIndex, ErrorsFound)),
                                    RatedInletAirTempHeat,
                                    RatedOutdoorWetBulbTempHeat);
                            } else if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).HeatingPerformanceOATType == HVAC::OATType::DryBulb) {
                                checkCurveIsNormalizedToOne(
                                    state,
                                    "GetDXCoils: " + HVAC::cAllCoilTypes(thisVrfTU.DXHeatCoilType_Num),
                                    DXCoils::GetDXCoilName(
                                        state, thisVrfTU.HeatCoilIndex, ErrorsFound, HVAC::cAllCoilTypes(thisVrfTU.DXHeatCoilType_Num)),
                                    GetDXCoilCapFTCurveIndex(state, thisVrfTU.HeatCoilIndex, ErrorsFound),
                                    "Heating Capacity Ratio Modifier Function of Temperature Curve Name",
                                    Curve::GetCurveName(state, GetDXCoilCapFTCurveIndex(state, thisVrfTU.HeatCoilIndex, ErrorsFound)),
                                    RatedInletAirTempHeat,
                                    RatedOutdoorAirTempHeat);
                            }
                        }

                    } else {
                        ShowSevereError(state, cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                        ShowContinueError(state, "... illegal " + cAlphaFieldNames(14) + " = " + cAlphaArgs(14));
                        ErrorsFound = true;
                    }
                }
            } else {
                ShowSevereError(state, cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                ShowContinueError(state, "... when checking " + HVAC::cAllCoilTypes(thisVrfTU.DXHeatCoilType_Num) + " \"" + cAlphaArgs(14) + "\"");
                ShowContinueError(state, "... terminal unit not connected to condenser.");
                ShowContinueError(state, "... check that terminal unit is specified in a terminal unit list object.");
                ShowContinueError(
                    state, "... also check that the terminal unit list name is specified in an AirConditioner:VariableRefrigerantFlow object.");
                ErrorsFound = true;
            }
        }

        if (!thisVrfTU.CoolingCoilPresent && thisVrfTU.DXCoolCoilType_Num == 0 && !thisVrfTU.HeatingCoilPresent &&
            thisVrfTU.DXHeatCoilType_Num == 0) {
            ShowSevereError(state, cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
            ShowContinueError(state, "... no valid coils entered for this terminal unit. Simulation will not proceed.");
            ErrorsFound = true;
        }

        if (!lAlphaFieldBlanks(15)) {
            thisVrfTU.AvailManagerListName = cAlphaArgs(15);
        }
        thisVrfTU.ParasiticElec = rNumericArgs(8);
        thisVrfTU.ParasiticOffElec = rNumericArgs(9);

        thisVrfTU.HVACSizingIndex = 0;
        if (!lAlphaFieldBlanks(16)) {
            thisVrfTU.HVACSizingIndex = Util::FindItemInList(cAlphaArgs(16), state.dataSize->ZoneHVACSizing);
            if (thisVrfTU.HVACSizingIndex == 0) {
                ShowSevereError(state, cAlphaFieldNames(16) + " = " + cAlphaArgs(16) + " not found.");
                ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                ErrorsFound = true;
            }
        }

        // supplemental heating coil
        if (!lAlphaFieldBlanks(17) && !lAlphaFieldBlanks(18)) {

            thisVrfTU.SuppHeatCoilType = cAlphaArgs(17);
            thisVrfTU.SuppHeatCoilName = cAlphaArgs(18);

            errFlag = false;
            if (Util::SameString(thisVrfTU.SuppHeatCoilType, "Coil:Heating:Water")) {
                thisVrfTU.SuppHeatCoilType_Num = HVAC::Coil_HeatingWater;
            } else if (Util::SameString(thisVrfTU.SuppHeatCoilType, "Coil:Heating:Steam")) {
                thisVrfTU.SuppHeatCoilType_Num = HVAC::Coil_HeatingSteam;
            } else if (Util::SameString(thisVrfTU.SuppHeatCoilType, "Coil:Heating:Fuel") ||
                       Util::SameString(thisVrfTU.SuppHeatCoilType, "Coil:Heating:Electric")) {
                thisVrfTU.SuppHeatCoilType_Num =
                    HeatingCoils::GetHeatingCoilTypeNum(state, thisVrfTU.SuppHeatCoilType, thisVrfTU.SuppHeatCoilName, errFlag);
            }

            thisVrfTU.SuppHeatingCoilPresent = true;

            if (thisVrfTU.SuppHeatCoilType_Num == HVAC::Coil_HeatingGasOrOtherFuel || thisVrfTU.SuppHeatCoilType_Num == HVAC::Coil_HeatingElectric) {
                errFlag = false;
                thisVrfTU.SuppHeatCoilType_Num =
                    HeatingCoils::GetHeatingCoilTypeNum(state, thisVrfTU.SuppHeatCoilType, thisVrfTU.SuppHeatCoilName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                    ErrorsFound = true;
                } else {
                    ValidateComponent(state, thisVrfTU.SuppHeatCoilType, thisVrfTU.SuppHeatCoilName, IsNotOK, cCurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                        ErrorsFound = true;
                    } else { // mine data from supplemental heating coil
                        // Get the supplemental heating coil index
                        thisVrfTU.SuppHeatCoilIndex =
                            HeatingCoils::GetHeatingCoilIndex(state, thisVrfTU.SuppHeatCoilType, thisVrfTU.SuppHeatCoilName, IsNotOK);
                        if (IsNotOK) {
                            ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                            ErrorsFound = true;
                        }
                        // Get the design supplemental heating capacity
                        errFlag = false;
                        thisVrfTU.DesignSuppHeatingCapacity =
                            HeatingCoils::GetCoilCapacity(state, thisVrfTU.SuppHeatCoilType, thisVrfTU.SuppHeatCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                            ErrorsFound = true;
                        }
                        // Get the supplemental heating Coil air inlet node
                        errFlag = false;
                        thisVrfTU.SuppHeatCoilAirInletNode =
                            HeatingCoils::GetCoilInletNode(state, thisVrfTU.SuppHeatCoilType, thisVrfTU.SuppHeatCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                            ErrorsFound = true;
                        }
                        // Get the supplemental heating Coil air outlet node
                        errFlag = false;
                        thisVrfTU.SuppHeatCoilAirOutletNode =
                            HeatingCoils::GetCoilOutletNode(state, thisVrfTU.SuppHeatCoilType, thisVrfTU.SuppHeatCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                            ErrorsFound = true;
                        }
                    } // IF (IsNotOK) THEN
                }

            } else if (thisVrfTU.SuppHeatCoilType_Num == HVAC::Coil_HeatingWater) {

                ValidateComponent(state, thisVrfTU.SuppHeatCoilType, thisVrfTU.SuppHeatCoilName, IsNotOK, cCurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                    ErrorsFound = true;
                } else { // mine data from heating coil object

                    // Get the supplemental heating coil water Inlet or control node number
                    errFlag = false;
                    thisVrfTU.SuppHeatCoilFluidInletNode =
                        WaterCoils::GetCoilWaterInletNode(state, "Coil:Heating:Water", thisVrfTU.SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                        ErrorsFound = true;
                    }
                    // Get the supplemental heating coil hot water max volume flow rate
                    errFlag = false;
                    thisVrfTU.SuppHeatCoilFluidMaxFlow =
                        WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", thisVrfTU.SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                        ErrorsFound = true;
                    }
                    // Get the supplemental heating Coil air inlet node
                    errFlag = false;
                    thisVrfTU.SuppHeatCoilAirInletNode =
                        WaterCoils::GetCoilInletNode(state, "Coil:Heating:Water", thisVrfTU.SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                        ErrorsFound = true;
                    }
                    // Get the supplemental heating coil air outlet node
                    errFlag = false;
                    thisVrfTU.SuppHeatCoilAirOutletNode =
                        WaterCoils::GetCoilOutletNode(state, "Coil:Heating:Water", thisVrfTU.SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                        ErrorsFound = true;
                    }
                }

            } else if (thisVrfTU.SuppHeatCoilType_Num == HVAC::Coil_HeatingSteam) {

                ValidateComponent(state, thisVrfTU.SuppHeatCoilType, thisVrfTU.SuppHeatCoilName, IsNotOK, cCurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                    ErrorsFound = true;
                } else { // mine data from supplemental heating coil object
                    errFlag = false;
                    thisVrfTU.SuppHeatCoilIndex = SteamCoils::GetSteamCoilIndex(state, "COIL:HEATING:STEAM", thisVrfTU.SuppHeatCoilName, errFlag);
                    if (thisVrfTU.SuppHeatCoilIndex == 0) {
                        ShowSevereError(state, cCurrentModuleObject + " = " + thisVrfTU.Name);
                        ErrorsFound = true;
                    }
                    // Get the supplemental heating Coil steam inlet node number
                    errFlag = false;
                    thisVrfTU.SuppHeatCoilFluidInletNode =
                        SteamCoils::GetCoilSteamInletNode(state, "Coil:Heating:Steam", thisVrfTU.SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                        ErrorsFound = true;
                    }
                    // Get the supplemental heating coil steam max volume flow rate
                    thisVrfTU.SuppHeatCoilFluidMaxFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, thisVrfTU.SuppHeatCoilIndex, errFlag);
                    if (thisVrfTU.SuppHeatCoilFluidMaxFlow > 0.0) {
                        Real64 TempSteamIn = 100.0;
                        Real64 SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, TempSteamIn, 1.0, RoutineName);
                        thisVrfTU.SuppHeatCoilFluidMaxFlow =
                            SteamCoils::GetCoilMaxSteamFlowRate(state, thisVrfTU.SuppHeatCoilIndex, errFlag) * SteamDensity;
                    }
                    // Get the supplemental heating coil air inlet node
                    errFlag = false;
                    thisVrfTU.SuppHeatCoilAirInletNode =
                        SteamCoils::GetCoilAirInletNode(state, thisVrfTU.SuppHeatCoilIndex, thisVrfTU.SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                        ErrorsFound = true;
                    }
                    // Get the supplemental heating coil air outlet node
                    errFlag = false;
                    thisVrfTU.SuppHeatCoilAirOutletNode =
                        SteamCoils::GetCoilAirOutletNode(state, thisVrfTU.SuppHeatCoilIndex, thisVrfTU.SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, "Occurs in " + cCurrentModuleObject + " = " + thisVrfTU.Name);
                        ErrorsFound = true;
                    }
                }
            }
        } else { // if (!lAlphaFieldBlanks(17) && !lAlphaFieldBlanks(18)) {
            if (!lAlphaFieldBlanks(17) && lAlphaFieldBlanks(18)) {
                ShowWarningError(state, cCurrentModuleObject + " = " + thisVrfTU.Name + "\"");
                ShowContinueError(state, "...Supplemental heating coil type = " + cAlphaArgs(17));
                ShowContinueError(state, "...But missing the associated supplemental heating coil name. ");
                ShowContinueError(state, "...The supplemental heating coil will not be simulated. ");
            }
            if (lAlphaFieldBlanks(17) && !lAlphaFieldBlanks(18)) {
                ShowWarningError(state, cCurrentModuleObject + " = " + thisVrfTU.Name + "\"");
                ShowContinueError(state, "...Supplemental heating coil name = " + cAlphaArgs(18));
                ShowContinueError(state, "...But missing the associated supplemental heating coil type. ");
                ShowContinueError(state, "...The supplemental heating coil will not be simulated. ");
            }
        }

        if (!lAlphaFieldBlanks(19)) {
            thisVrfTU.ZoneNum = Util::FindItemInList(cAlphaArgs(19), state.dataHeatBal->Zone);
            if (thisVrfTU.ZoneNum == 0) {
                ShowSevereError(state, cCurrentModuleObject + " = " + cAlphaArgs(1));
                ShowContinueError(state, "Illegal " + cAlphaFieldNames(19) + " = " + cAlphaArgs(19));
                ErrorsFound = true;
            }
        }

        auto &vrfTU = state.dataHVACVarRefFlow->VRFTU(VRFTUNum);
        if (!lAlphaFieldBlanks(20) && !lAlphaFieldBlanks(21)) {
            vrfTU.DesignSpecMultispeedHPType = cAlphaArgs(20);
            vrfTU.DesignSpecMultispeedHPName = cAlphaArgs(21);
            vrfTU.DesignSpecMSHPIndex = UnitarySystems::getDesignSpecMSHPIndex(state, cAlphaArgs(21));
            auto const &designSpecFan = state.dataUnitarySystems->designSpecMSHP[vrfTU.DesignSpecMSHPIndex];
            if (vrfTU.DXCoolCoilType_Num == HVAC::CoilVRF_Cooling) {
                int NumSpeeds = designSpecFan.numOfSpeedCooling;
                vrfTU.NumOfSpeedCooling = NumSpeeds;
                vrfTU.CoolVolumeFlowRate.resize(NumSpeeds + 1);
                vrfTU.CoolMassFlowRate.resize(NumSpeeds + 1);
                if (vrfTU.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                    Real64 AirFlowRate = vrfTU.MaxCoolAirVolFlow;
                    for (int i = 1; i <= vrfTU.NumOfSpeedCooling; ++i) {
                        if (state.dataUnitarySystems->designSpecMSHP[vrfTU.DesignSpecMSHPIndex].coolingVolFlowRatio[i] == DataSizing::AutoSize) {
                            vrfTU.CoolVolumeFlowRate[i] = double(i) / double(vrfTU.NumOfSpeedCooling) * AirFlowRate;
                        } else {
                            vrfTU.CoolVolumeFlowRate[i] =
                                state.dataUnitarySystems->designSpecMSHP[vrfTU.DesignSpecMSHPIndex].coolingVolFlowRatio[i] * AirFlowRate;
                        }
                        vrfTU.CoolMassFlowRate[i] = vrfTU.CoolVolumeFlowRate[i] * state.dataEnvrn->StdRhoAir;
                    }
                }
            }
            if (vrfTU.DXHeatCoilType_Num == HVAC::CoilVRF_Heating) {
                int NumSpeeds = designSpecFan.numOfSpeedHeating;
                vrfTU.NumOfSpeedHeating = NumSpeeds;
                vrfTU.HeatVolumeFlowRate.resize(NumSpeeds + 1);
                vrfTU.HeatMassFlowRate.resize(NumSpeeds + 1);
                if (vrfTU.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                    Real64 AirFlowRate = vrfTU.MaxHeatAirVolFlow;
                    for (int i = 1; i <= vrfTU.NumOfSpeedHeating; ++i) {
                        if (state.dataUnitarySystems->designSpecMSHP[vrfTU.DesignSpecMSHPIndex].heatingVolFlowRatio[i] == DataSizing::AutoSize) {
                            vrfTU.HeatVolumeFlowRate[i] = double(i) / double(vrfTU.NumOfSpeedHeating) * AirFlowRate;
                        } else {
                            vrfTU.HeatVolumeFlowRate[i] =
                                state.dataUnitarySystems->designSpecMSHP[vrfTU.DesignSpecMSHPIndex].heatingVolFlowRatio[i] * AirFlowRate;
                        }
                        vrfTU.HeatMassFlowRate[i] = vrfTU.HeatVolumeFlowRate[i] * state.dataEnvrn->StdRhoAir;
                    }
                }
            }
        } else {
            if (vrfTU.fanType == HVAC::FanType::SystemModel) {
                auto *fanSystem = dynamic_cast<Fans::FanSystem *>(state.dataFans->fans(vrfTU.FanIndex));
                assert(fanSystem != nullptr);

                if (fanSystem->speedControl == Fans::SpeedControl::Discrete) {
                    if (fanSystem->numSpeeds > 1) {
                        if (vrfTU.DXCoolCoilType_Num == HVAC::CoilVRF_Cooling) {
                            vrfTU.NumOfSpeedCooling = fanSystem->numSpeeds;
                            vrfTU.CoolVolumeFlowRate.resize(fanSystem->numSpeeds + 1);
                            vrfTU.CoolMassFlowRate.resize(fanSystem->numSpeeds + 1);
                        }
                        if (vrfTU.DXHeatCoilType_Num == HVAC::CoilVRF_Heating) {
                            vrfTU.NumOfSpeedHeating = fanSystem->numSpeeds;
                            vrfTU.HeatVolumeFlowRate.resize(fanSystem->numSpeeds + 1);
                            vrfTU.HeatMassFlowRate.resize(fanSystem->numSpeeds + 1);
                        }
                        ShowWarningError(state,
                                         cCurrentModuleObject + " = " + thisVrfTU.Name + " with Fan:SystemModel is used in  " + cAlphaArgs(8) + "\"");
                        ShowContinueError(state, format("...The number of speed = {:.0R}.", double(fanSystem->numSpeeds)));
                        ShowContinueError(state, "...Multiple speed fan will be applied to this unit. The speed number is determined by load.");
                    }
                }
            }
        }

        // set supplemental heating coil operation temperature limits
        if (thisVrfTU.SuppHeatingCoilPresent) {
            // Set maximum supply air temperature for supplemental heating coil
            if (NumNums < 11) {
                thisVrfTU.MaxSATFromSuppHeatCoil = DataSizing::AutoSize;
            } else {
                thisVrfTU.MaxSATFromSuppHeatCoil = rNumericArgs(11);
            }
            // set maximum outdoor dry-bulb temperature for supplemental heating coil operation
            if (NumNums < 12) {
                thisVrfTU.MaxOATSuppHeatingCoil = 21.0;
            } else {
                thisVrfTU.MaxOATSuppHeatingCoil = rNumericArgs(12);
            }
        }

        // Add cooling coil to component sets array
        if (thisVrfTU.CoolingCoilPresent) {

            SetUpCompSets(state,
                          cCurrentModuleObject,
                          thisVrfTU.Name,
                          HVAC::cAllCoilTypes(thisVrfTU.DXCoolCoilType_Num),
                          cAlphaArgs(12),
                          state.dataLoopNodes->NodeID(CCoilInletNodeNum),
                          state.dataLoopNodes->NodeID(CCoilOutletNodeNum));
            //     set heating coil present flag
            SetDXCoolingCoilData(
                state, thisVrfTU.CoolCoilIndex, ErrorsFound, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, thisVrfTU.HeatingCoilPresent);

            //   check that curve types are present in VRF Condenser if cooling coil is present in terminal unit (can be blank)
            //   all curves are checked for correct type if a curve name is entered in the VRF condenser object. Check that the
            //   curve is present if the corresponding coil is entered in the terminal unit.
            if (thisVrfTU.VRFSysNum > 0) {

                if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).VRFAlgorithmType != AlgorithmType::FluidTCtrl) {

                    if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).CoolingCapacity <= 0 &&
                        state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).CoolingCapacity != AutoSize) {
                        ShowSevereError(state, cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                        ShowContinueError(state,
                                          "...This terminal unit contains a cooling coil and rated cooling capacity is also required in the "
                                          "associated condenser object.");
                        ShowContinueError(state,
                                          "...Rated Cooling Capacity must also be specified for condenser = " +
                                              std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).VRFSystemTypeNum)) + " \"" +
                                              state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).Name + "\".");
                        ErrorsFound = true;
                    }
                }
            }
        }

        // Add heating coil to component sets array
        if (thisVrfTU.HeatingCoilPresent) {

            SetUpCompSets(state,
                          cCurrentModuleObject,
                          thisVrfTU.Name,
                          HVAC::cAllCoilTypes(thisVrfTU.DXHeatCoilType_Num),
                          cAlphaArgs(14),
                          state.dataLoopNodes->NodeID(HCoilInletNodeNum),
                          state.dataLoopNodes->NodeID(HCoilOutletNodeNum));
            //     set cooling coil present flag
            SetDXCoolingCoilData(
                state, thisVrfTU.HeatCoilIndex, ErrorsFound, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, thisVrfTU.CoolingCoilPresent);

            if (thisVrfTU.VRFSysNum > 0) {

                if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).VRFAlgorithmType != AlgorithmType::FluidTCtrl) {

                    if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).HeatingCapacity <= 0 &&
                        state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).HeatingCapacity != AutoSize) {
                        ShowSevereError(state, cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                        ShowContinueError(state,
                                          "...This terminal unit contains a heating coil and rated heating capacity is also required in the "
                                          "associated condenser object.");
                        ShowContinueError(state,
                                          "...Rated Heating Capacity must also be specified for condenser = " +
                                              std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).VRFSystemTypeNum)) + " \"" +
                                              state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).Name + "\".");
                        ErrorsFound = true;
                    }

                    if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).HeatCapFT == 0) {
                        ShowSevereError(state, cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                        ShowContinueError(state,
                                          "...This terminal unit contains a heating coil and heating performance curves are also required in the "
                                          "associated condenser object.");
                        ShowContinueError(
                            state,
                            "...Heating Capacity Ratio Modifier Function of Low Temperature Curve must also be specified for condenser = " +
                                std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).VRFSystemTypeNum)) + " \"" +
                                state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).Name + "\".");
                        ErrorsFound = true;
                    }

                    if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).HeatEIRFT == 0) {
                        ShowSevereError(state, cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                        ShowContinueError(state,
                                          "...This terminal unit contains a heating coil and heating performance curves are also required in the "
                                          "associated condenser object.");
                        ShowContinueError(
                            state,
                            "...Heating Energy Input Ratio Modifier Function of Low Temperature Curve must also be specified for condenser = " +
                                std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).VRFSystemTypeNum)) + " \"" +
                                state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).Name + "\".");
                        ErrorsFound = true;
                    }

                    if (state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).HeatEIRFPLR1 == 0) {
                        ShowSevereError(state, cCurrentModuleObject + " \"" + thisVrfTU.Name + "\"");
                        ShowContinueError(state,
                                          "...This terminal unit contains a heating coil and heating performance curves are also required in the "
                                          "associated condenser object.");
                        ShowContinueError(state,
                                          "...Heating Energy Input Ratio Modifier Function of Low Part-Load Ratio Curve must also be specified "
                                          "for condenser = " +
                                              std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).VRFSystemTypeNum)) + " \"" +
                                              state.dataHVACVarRefFlow->VRF(thisVrfTU.VRFSysNum).Name + "\".");
                    }
                }
            }
        }

        // Add supplemental heating coil to component sets array
        if (thisVrfTU.SuppHeatingCoilPresent) {
            SetUpCompSets(state,
                          cCurrentModuleObject,
                          thisVrfTU.Name,
                          HVAC::cAllCoilTypes(thisVrfTU.SuppHeatCoilType_Num),
                          thisVrfTU.SuppHeatCoilName,
                          state.dataLoopNodes->NodeID(thisVrfTU.SuppHeatCoilAirInletNode),
                          state.dataLoopNodes->NodeID(thisVrfTU.SuppHeatCoilAirOutletNode));
        }
        // Set up component set for OA mixer - use OA node and Mixed air node
        if (thisVrfTU.OAMixerUsed) {
            SetUpCompSets(state,
                          cCurrentModuleObject,
                          thisVrfTU.Name,
                          "UNDEFINED",
                          thisVrfTU.OAMixerName,
                          state.dataLoopNodes->NodeID(OANodeNums(1)),
                          state.dataLoopNodes->NodeID(OANodeNums(4)));
        }

        // Get AirTerminal mixer data
        GetATMixer(state,
                   thisVrfTU.Name,
                   thisVrfTU.ATMixerName,
                   thisVrfTU.ATMixerIndex,
                   thisVrfTU.ATMixerType,
                   thisVrfTU.ATMixerPriNode,
                   thisVrfTU.ATMixerSecNode,
                   thisVrfTU.ATMixerOutNode,
                   thisVrfTU.VRFTUOutletNodeNum);
        if (thisVrfTU.ATMixerType == HVAC::MixerType::InletSide || thisVrfTU.ATMixerType == HVAC::MixerType::SupplySide) {
            thisVrfTU.ATMixerExists = true;
        }
        // check that the VRF TU have local outside air and DOA
        if (thisVrfTU.ATMixerExists && OANodeNums(4) > 0) {
            ShowSevereError(
                state, cCurrentModuleObject + " = \"" + thisVrfTU.Name + "\". VRF terminal unit has local as well as central outdoor air specified");
            ErrorsFound = true;
        }

        // for ZoneHVAC check that TU inlet node is a zone exhaust node otherwise ZoneAirNode and ZoneNum = 0
        if (!thisVrfTU.ATMixerExists || thisVrfTU.ATMixerType == HVAC::MixerType::SupplySide) {
            for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                    continue;
                }
                for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumExhaustNodes; ++NodeNum) {
                    if (thisVrfTU.VRFTUInletNodeNum == state.dataZoneEquip->ZoneEquipConfig(CtrlZone).ExhaustNode(NodeNum)) {
                        thisVrfTU.ZoneAirNode = state.dataZoneEquip->ZoneEquipConfig(CtrlZone).ZoneNode;
                        thisVrfTU.ZoneNum = CtrlZone;
                        break;
                    }
                }
            }
        } else if (thisVrfTU.ATMixerType == HVAC::MixerType::InletSide) {
            for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                    continue;
                }
                for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumInletNodes; ++NodeNum) {
                    if (thisVrfTU.VRFTUOutletNodeNum == state.dataZoneEquip->ZoneEquipConfig(CtrlZone).InletNode(NodeNum)) {
                        thisVrfTU.ZoneAirNode = state.dataZoneEquip->ZoneEquipConfig(CtrlZone).ZoneNode;
                        thisVrfTU.ZoneNum = CtrlZone;
                        break;
                    }
                }
            }
        }
        CheckVRFTUNodeConnections(state, VRFTUNum, ErrorsFound);
    } // end Number of VRF Terminal Unit Loop

    //   perform additional error checking
    for (auto const &thisTUList : state.dataHVACVarRefFlow->TerminalUnitList) {
        for (int VRFTUNum = 1; VRFTUNum <= thisTUList.NumTUInList; ++VRFTUNum) {
            int const tuPtr = thisTUList.ZoneTUPtr(VRFTUNum);
            if (tuPtr == 0) {
                // TU name in zone terminal unit list not found
                ShowSevereError(state, format("ZoneTerminalUnitList \"{}\"", thisTUList.Name));
                ShowContinueError(state, format("...Zone Terminal Unit = {} improperly connected to system.", thisTUList.ZoneTUName(VRFTUNum)));
                ShowContinueError(state, "...either the ZoneHVAC:TerminalUnit:VariableRefrigerantFlow object does not exist,");
                ShowContinueError(state, "...the ZoneHVAC:TerminalUnit:VariableRefrigerantFlow object name is misspelled,");
                ShowContinueError(state, "...or the ZoneTerminalUnitList object is not named in an AirConditioner:VariableRefrigerantFlow object.");
                ErrorsFound = true;
            } else {
                int const sysNum = state.dataHVACVarRefFlow->VRFTU(tuPtr).VRFSysNum;
                if (sysNum > 0) {
                    auto &thisVRFSys = state.dataHVACVarRefFlow->VRF(sysNum);
                    if (thisTUList.NumTUInList == 1 && thisVRFSys.VRFAlgorithmType == AlgorithmType::SysCurve) {
                        if (thisVRFSys.HeatRecoveryUsed) {
                            ShowWarningError(state, format("ZoneTerminalUnitList \"{}\"", thisTUList.Name));
                            ShowWarningError(state, "...Only 1 Terminal Unit connected to system and heat recovery is selected.");
                            ShowContinueError(state, format("...Heat recovery will be disabled for {}.", thisVRFSys.Name));
                            thisVRFSys.HeatRecoveryUsed = false;
                        }
                    }
                }
            }
        }
    }

    //   warn when number of ZoneTerminalUnitList different from number of AirConditioner:VariableRefrigerantFlow
    if (state.dataHVACVarRefFlow->NumVRFTULists != state.dataHVACVarRefFlow->NumVRFCond) {
        ShowSevereError(state,
                        format("The number of AirConditioner:VariableRefrigerantFlow objects ({}) does not match the number of "
                               "ZoneTerminalUnitList objects ({}).",
                               state.dataHVACVarRefFlow->NumVRFCond,
                               state.dataHVACVarRefFlow->NumVRFTULists));
        for (int NumCond = 1; NumCond <= state.dataHVACVarRefFlow->NumVRFCond; ++NumCond) {
            if (state.dataHVACVarRefFlow->VRF(NumCond).ZoneTUListPtr > 0) {
                ShowContinueError(state,
                                  format("...AirConditioner:VariableRefrigerantFlow = {} specifies Zone Terminal Unit List Name = {}",
                                         state.dataHVACVarRefFlow->VRF(NumCond).Name,
                                         state.dataHVACVarRefFlow->TerminalUnitList(state.dataHVACVarRefFlow->VRF(NumCond).ZoneTUListPtr).Name));
            } else {
                ShowContinueError(state,
                                  format("...AirConditioner:VariableRefrigerantFlow = {} Zone Terminal Unit List Name not found.",
                                         state.dataHVACVarRefFlow->VRF(NumCond).Name));
            }
        }
        ShowContinueError(state, "...listing ZoneTerminalUnitList objects.");
        for (int NumList = 1; NumList <= state.dataHVACVarRefFlow->NumVRFTULists; ++NumList) {
            ShowContinueError(state, "...ZoneTerminalUnitList = " + state.dataHVACVarRefFlow->TerminalUnitList(NumList).Name);
        }
        ErrorsFound = true;
    }

    // Set up output variables
    for (int VRFTUNum = 1; VRFTUNum <= state.dataHVACVarRefFlow->NumVRFTU; ++VRFTUNum) {
        auto &thisVrfTU = state.dataHVACVarRefFlow->VRFTU(VRFTUNum);
        if (thisVrfTU.CoolingCoilPresent) {
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Cooling Electricity Rate",
                                Constant::Units::W,
                                thisVrfTU.ParasiticCoolElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrfTU.Name);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Cooling Electricity Energy",
                                Constant::Units::J,
                                thisVrfTU.ParasiticElecCoolConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisVrfTU.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Total Cooling Rate",
                                Constant::Units::W,
                                thisVrfTU.TotalCoolingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrfTU.Name);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Sensible Cooling Rate",
                                Constant::Units::W,
                                thisVrfTU.SensibleCoolingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrfTU.Name);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Latent Cooling Rate",
                                Constant::Units::W,
                                thisVrfTU.LatentCoolingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrfTU.Name);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Total Cooling Energy",
                                Constant::Units::J,
                                thisVrfTU.TotalCoolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisVrfTU.Name);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Sensible Cooling Energy",
                                Constant::Units::J,
                                thisVrfTU.SensibleCoolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisVrfTU.Name);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Latent Cooling Energy",
                                Constant::Units::J,
                                thisVrfTU.LatentCoolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisVrfTU.Name);
        }
        if (thisVrfTU.HeatingCoilPresent) {
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Heating Electricity Rate",
                                Constant::Units::W,
                                thisVrfTU.ParasiticHeatElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrfTU.Name);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Heating Electricity Energy",
                                Constant::Units::J,
                                thisVrfTU.ParasiticElecHeatConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisVrfTU.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Total Heating Rate",
                                Constant::Units::W,
                                thisVrfTU.TotalHeatingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrfTU.Name);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Sensible Heating Rate",
                                Constant::Units::W,
                                thisVrfTU.SensibleHeatingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrfTU.Name);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Latent Heating Rate",
                                Constant::Units::W,
                                thisVrfTU.LatentHeatingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrfTU.Name);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Total Heating Energy",
                                Constant::Units::J,
                                thisVrfTU.TotalHeatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisVrfTU.Name);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Sensible Heating Energy",
                                Constant::Units::J,
                                thisVrfTU.SensibleHeatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisVrfTU.Name);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Latent Heating Energy",
                                Constant::Units::J,
                                thisVrfTU.LatentHeatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisVrfTU.Name);
        }
        SetupOutputVariable(state,
                            "Zone VRF Air Terminal Fan Availability Status",
                            Constant::Units::None,
                            thisVrfTU.availStatus,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrfTU.Name);
        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            SetupEMSActuator(state,
                             "Variable Refrigerant Flow Terminal Unit",
                             thisVrfTU.Name,
                             "Part Load Ratio",
                             "[fraction]",
                             thisVrfTU.EMSOverridePartLoadFrac,
                             thisVrfTU.EMSValueForPartLoadFrac);
        }
        if (thisVrfTU.NumOfSpeedCooling > 1 || thisVrfTU.NumOfSpeedHeating > 1) {
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Multispeed Fan Cycling Ratio",
                                Constant::Units::None,
                                thisVrfTU.CycRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrfTU.Name);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Multispeed Fan Speed Ratio",
                                Constant::Units::None,
                                thisVrfTU.SpeedRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrfTU.Name);
            SetupOutputVariable(state,
                                "Zone VRF Air Terminal Multispeed Fan Speed Level",
                                Constant::Units::None,
                                thisVrfTU.SpeedNum,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrfTU.Name);
        }
    }

    for (int NumCond = 1; NumCond <= state.dataHVACVarRefFlow->NumVRFCond; ++NumCond) {
        auto &thisVrf = state.dataHVACVarRefFlow->VRF(NumCond);
        std::string_view const sFuelType = Constant::eFuelNames[static_cast<int>(thisVrf.fuel)];
        SetupOutputVariable(state,
                            "VRF Heat Pump Total Cooling Rate",
                            Constant::Units::W,
                            thisVrf.TotalCoolingCapacity,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);
        SetupOutputVariable(state,
                            "VRF Heat Pump Total Heating Rate",
                            Constant::Units::W,
                            thisVrf.TotalHeatingCapacity,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);
        SetupOutputVariable(state,
                            format("VRF Heat Pump Cooling {} Rate", sFuelType),
                            Constant::Units::W,
                            thisVrf.ElecCoolingPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);
        SetupOutputVariable(state,
                            format("VRF Heat Pump Cooling {} Energy", sFuelType),
                            Constant::Units::J,
                            thisVrf.CoolElecConsumption,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisVrf.Name,
                            Constant::eFuel2eResource[(int)thisVrf.fuel],
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Cooling);
        SetupOutputVariable(state,
                            format("VRF Heat Pump Heating {} Rate", sFuelType),
                            Constant::Units::W,
                            thisVrf.ElecHeatingPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);
        SetupOutputVariable(state,
                            format("VRF Heat Pump Heating {} Energy", sFuelType),
                            Constant::Units::J,
                            thisVrf.HeatElecConsumption,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisVrf.Name,
                            Constant::eFuel2eResource[(int)thisVrf.fuel],
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Heating);

        SetupOutputVariable(state,
                            "VRF Heat Pump Cooling COP",
                            Constant::Units::None,
                            thisVrf.OperatingCoolingCOP,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);
        SetupOutputVariable(state,
                            "VRF Heat Pump Heating COP",
                            Constant::Units::None,
                            thisVrf.OperatingHeatingCOP,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);
        SetupOutputVariable(state,
                            "VRF Heat Pump COP",
                            Constant::Units::None,
                            thisVrf.OperatingCOP,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);

        if (thisVrf.VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
            // For VRF_FluidTCtrl Model
            SetupOutputVariable(state,
                                "VRF Heat Pump Compressor Electricity Rate",
                                Constant::Units::W,
                                thisVrf.Ncomp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Outdoor Unit Fan Power",
                                Constant::Units::W,
                                thisVrf.OUFanPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Compressor Rotating Speed",
                                Constant::Units::rev_min,
                                thisVrf.CompActSpeed,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Indoor Unit Evaporating Temperature",
                                Constant::Units::C,
                                thisVrf.IUEvaporatingTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Outdoor Unit Condensing Temperature",
                                Constant::Units::C,
                                thisVrf.CondensingTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Indoor Unit Condensing Temperature",
                                Constant::Units::C,
                                thisVrf.IUCondensingTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Outdoor Unit Evaporating Temperature",
                                Constant::Units::C,
                                thisVrf.EvaporatingTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Cooling Capacity at Max Compressor Speed",
                                Constant::Units::W,
                                thisVrf.CoolingCapacity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Heating Capacity at Max Compressor Speed",
                                Constant::Units::W,
                                thisVrf.HeatingCapacity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Indoor Unit Piping Correction for Cooling",
                                Constant::Units::None,
                                thisVrf.PipingCorrectionCooling,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Indoor Unit Piping Correction for Heating",
                                Constant::Units::None,
                                thisVrf.PipingCorrectionHeating,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Outdoor Unit Evaporator Heat Extract Rate",
                                Constant::Units::W,
                                thisVrf.OUEvapHeatRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Outdoor Unit Condenser Heat Release Rate",
                                Constant::Units::W,
                                thisVrf.OUCondHeatRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);

        } else {
            // For VRF_SysCurve Model
            SetupOutputVariable(state,
                                "VRF Heat Pump Maximum Capacity Cooling Rate",
                                Constant::Units::W,
                                state.dataHVACVarRefFlow->MaxCoolingCapacity(NumCond),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Maximum Capacity Heating Rate",
                                Constant::Units::W,
                                state.dataHVACVarRefFlow->MaxHeatingCapacity(NumCond),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
        }

        if (thisVrf.DefrostStrategy == StandardRatings::DefrostStrat::Resistive ||
            (thisVrf.DefrostStrategy == StandardRatings::DefrostStrat::ReverseCycle && thisVrf.fuel == Constant::eFuel::Electricity)) {
            SetupOutputVariable(state,
                                "VRF Heat Pump Defrost Electricity Rate",
                                Constant::Units::W,
                                thisVrf.DefrostPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Defrost Electricity Energy",
                                Constant::Units::J,
                                thisVrf.DefrostConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisVrf.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);

        } else { // defrost energy applied to fuel type
            SetupOutputVariable(state,
                                format("VRF Heat Pump Defrost {} Rate", sFuelType),
                                Constant::Units::W,
                                thisVrf.DefrostPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                format("VRF Heat Pump Defrost {} Energy", sFuelType),
                                Constant::Units::J,
                                thisVrf.DefrostConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisVrf.Name,
                                Constant::eFuel2eResource[(int)thisVrf.fuel],
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);
        }

        SetupOutputVariable(state,
                            "VRF Heat Pump Part Load Ratio",
                            Constant::Units::None,
                            thisVrf.VRFCondPLR,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);
        SetupOutputVariable(state,
                            "VRF Heat Pump Runtime Fraction",
                            Constant::Units::None,
                            thisVrf.VRFCondRTF,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);
        SetupOutputVariable(state,
                            "VRF Heat Pump Cycling Ratio",
                            Constant::Units::None,
                            thisVrf.VRFCondCyclingRatio,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);

        SetupOutputVariable(state,
                            "VRF Heat Pump Operating Mode",
                            Constant::Units::None,
                            thisVrf.OperatingMode,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);
        SetupOutputVariable(state,
                            "VRF Heat Pump Condenser Inlet Temperature",
                            Constant::Units::C,
                            thisVrf.CondenserInletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);

        SetupOutputVariable(state,
                            "VRF Heat Pump Crankcase Heater Electricity Rate",
                            Constant::Units::W,
                            thisVrf.CrankCaseHeaterPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);
        SetupOutputVariable(state,
                            "VRF Heat Pump Crankcase Heater Electricity Energy",
                            Constant::Units::J,
                            thisVrf.CrankCaseHeaterElecConsumption,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisVrf.Name,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Cooling);
        SetupOutputVariable(state,
                            "VRF Heat Pump Terminal Unit Cooling Load Rate",
                            Constant::Units::W,
                            thisVrf.TUCoolingLoad,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);
        SetupOutputVariable(state,
                            "VRF Heat Pump Terminal Unit Heating Load Rate",
                            Constant::Units::W,
                            thisVrf.TUHeatingLoad,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisVrf.Name);
        if (thisVrf.HeatRecoveryUsed) {
            SetupOutputVariable(state,
                                "VRF Heat Pump Heat Recovery Status Change Multiplier",
                                Constant::Units::None,
                                thisVrf.SUMultiplier,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Simultaneous Cooling and Heating Efficiency",
                                Constant::Units::Btu_h_W,
                                thisVrf.SCHE,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Heat Recovery Rate",
                                Constant::Units::W,
                                thisVrf.VRFHeatRec,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Heat Recovery Energy",
                                Constant::Units::J,
                                thisVrf.VRFHeatEnergyRec,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisVrf.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRecovery);
        }

        if (thisVrf.CondenserType == DataHeatBalance::RefrigCondenserType::Evap) {
            SetupOutputVariable(state,
                                "VRF Heat Pump Evaporative Condenser Water Use Volume",
                                Constant::Units::m3,
                                thisVrf.EvapWaterConsumpRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisVrf.Name,
                                Constant::eResource::Water,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling);
            SetupOutputVariable(state,
                                "VRF Heat Pump Evaporative Condenser Pump Electricity Rate",
                                Constant::Units::W,
                                thisVrf.EvapCondPumpElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Evaporative Condenser Pump Electricity Energy",
                                Constant::Units::J,
                                thisVrf.EvapCondPumpElecConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisVrf.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling);

            if (thisVrf.BasinHeaterPowerFTempDiff > 0.0) {
                SetupOutputVariable(state,
                                    "VRF Heat Pump Basin Heater Electricity Rate",
                                    Constant::Units::W,
                                    thisVrf.BasinHeaterPower,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisVrf.Name);
                SetupOutputVariable(state,
                                    "VRF Heat Pump Basin Heater Electricity Energy",
                                    Constant::Units::J,
                                    thisVrf.BasinHeaterConsumption,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    thisVrf.Name,
                                    Constant::eResource::Electricity,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::Cooling);
            }

        } else if (thisVrf.CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
            SetupOutputVariable(state,
                                "VRF Heat Pump Condenser Outlet Temperature",
                                Constant::Units::C,
                                thisVrf.CondenserSideOutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Condenser Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisVrf.WaterCondenserMassFlow,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Condenser Heat Transfer Rate",
                                Constant::Units::W,
                                thisVrf.QCondenser,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisVrf.Name);
            SetupOutputVariable(state,
                                "VRF Heat Pump Condenser Heat Transfer Energy",
                                Constant::Units::J,
                                thisVrf.QCondEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisVrf.Name);
        }

        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            SetupEMSActuator(state,
                             "Variable Refrigerant Flow Heat Pump",
                             thisVrf.Name,
                             "Operating Mode",
                             "[integer]",
                             thisVrf.EMSOverrideHPOperatingMode,
                             thisVrf.EMSValueForHPOperatingMode);
        }
    }
}

void CheckVRFTUNodeConnections(EnergyPlusData &state, int const VRFTUNum, bool &ErrorsFound)
{

    constexpr static std::string_view cTerminalUnitType("ZoneHVAC:TerminalUnit:VariableRefrigerantFlow");
    auto const &nodeID = state.dataLoopNodes->NodeID;
    auto &vrfTU = state.dataHVACVarRefFlow->VRFTU(VRFTUNum);
    std::string const cTUName(vrfTU.Name);
    bool const CoolingCoilPresent = vrfTU.CoolingCoilPresent;
    bool const HeatingCoilPresent = vrfTU.HeatingCoilPresent;
    bool const SuppHeatingCoilPresent = vrfTU.SuppHeatingCoilPresent;
    HVAC::FanPlace const fanPlace = vrfTU.fanPlace;
    bool const FanPresent = fanPlace != HVAC::FanPlace::Invalid;
    bool const OAMixerUsed = vrfTU.OAMixerUsed;
    int const VRFTUInletNodeNum = vrfTU.VRFTUInletNodeNum;
    int const VRFTUOutletNodeNum = vrfTU.VRFTUOutletNodeNum;
    int const coolCoilAirInNode = vrfTU.coolCoilAirInNode;
    int const coolCoilAirOutNode = vrfTU.coolCoilAirOutNode;
    int const heatCoilAirInNode = vrfTU.heatCoilAirInNode;
    int const heatCoilAirOutNode = vrfTU.heatCoilAirOutNode;
    int const fanInletNode = vrfTU.fanInletNode;
    int const fanOutletNode = vrfTU.fanOutletNode;
    int const SuppHeatCoilAirInletNode = vrfTU.SuppHeatCoilAirInletNode;
    int const SuppHeatCoilAirOutletNode = vrfTU.SuppHeatCoilAirOutletNode;
    int const VRFTUOAMixerRetNodeNum = vrfTU.VRFTUOAMixerRetNodeNum;
    int const VRFTUOAMixerMixedNodeNum = vrfTU.VRFTUOAMixerMixedNodeNum;

    // check that TU object internal nodes (TU inlet to TU outlet) are correctly connected
    // the following is checked regardless of fan placement
    if (CoolingCoilPresent && HeatingCoilPresent) {
        if (coolCoilAirOutNode != heatCoilAirInNode) {
            ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
            ShowContinueError(state, "The cooling coil air outlet node name must match the heating coil air inlet node name.");
            if (coolCoilAirOutNode > 0 && heatCoilAirInNode > 0) {
                ShowContinueError(state, format("... Cooling coil air outlet node = {}", nodeID(coolCoilAirOutNode)));
                ShowContinueError(state, format("... Heating coil air inlet node  = {}", nodeID(heatCoilAirInNode)));
            }
            ErrorsFound = true;
        }
    }

    // check the TU inlet node name with the first component
    if (fanPlace == HVAC::FanPlace::DrawThru || !FanPresent) {
        if (OAMixerUsed) {
            if (VRFTUInletNodeNum != VRFTUOAMixerRetNodeNum) {
                ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
                ShowContinueError(state,
                                  "... For draw thru or no fan when an OA mixer is specified the terminal unit "
                                  "inlet node name must match the OA mixer return air stream node name.");
                if (VRFTUInletNodeNum > 0 && VRFTUOAMixerRetNodeNum > 0) {
                    ShowContinueError(state, format("... Terminal unit inlet node name = {}.", nodeID(VRFTUInletNodeNum)));
                    ShowContinueError(state, format("... OA mixer return air stream node name = {}.", nodeID(VRFTUOAMixerRetNodeNum)));
                }
                ErrorsFound = true;
            }
            // check mixer outlet with next component
            if (CoolingCoilPresent) {
                if (VRFTUOAMixerMixedNodeNum != coolCoilAirInNode) {
                    ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
                    ShowContinueError(state,
                                      "... For draw thru or no fan when an OA mixer is specified and a cooling coil is present "
                                      "the OA mixer mixed air node name must match the cooling coil inlet node name.");
                    if (VRFTUOAMixerMixedNodeNum > 0 && coolCoilAirInNode > 0) {
                        ShowContinueError(state, format("... OA mixer mixed air node name = {}.", nodeID(VRFTUOAMixerMixedNodeNum)));
                        ShowContinueError(state, format("... Cooling coil inlet node name = {}.", nodeID(coolCoilAirInNode)));
                    }
                    ErrorsFound = true;
                }
            } else if (HeatingCoilPresent) {
                if (VRFTUOAMixerMixedNodeNum != heatCoilAirInNode) {
                    ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
                    ShowContinueError(state,
                                      "... For draw thru or no fan when an OA mixer is specified and a cooling coil is not present "
                                      "the OA mixer mixed air node name must match the heating coil inlet node name.");
                    if (VRFTUOAMixerMixedNodeNum > 0 && heatCoilAirInNode > 0) {
                        ShowContinueError(state, format("... OA mixer mixed air node name = {}.", nodeID(VRFTUOAMixerMixedNodeNum)));
                        ShowContinueError(state, format("... Heating coil inlet node name = {}.", nodeID(heatCoilAirInNode)));
                    }
                    ErrorsFound = true;
                }
            }
        } else { // OAMixer not used
            if (CoolingCoilPresent) {
                if (VRFTUInletNodeNum != coolCoilAirInNode) {
                    ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
                    ShowContinueError(
                        state,
                        "... For draw thru or no fan when no OA mixer is specified and a cooling coil is present the terminal unit inlet "
                        "node name must match the cooling coil inlet node name.");
                    if (VRFTUInletNodeNum > 0 && coolCoilAirInNode > 0) {
                        ShowContinueError(state, format("... Terminal unit inlet node name = {}.", nodeID(VRFTUInletNodeNum)));
                        ShowContinueError(state, format("... Cooling coil inlet node name = {}.", nodeID(coolCoilAirInNode)));
                    }
                    ErrorsFound = true;
                }
            } else if (HeatingCoilPresent) {
                if (VRFTUInletNodeNum != heatCoilAirInNode) {
                    ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
                    ShowContinueError(state,
                                      "... For draw thru or no fan when no cooling coil or OA mixer is specified the terminal unit inlet "
                                      "node name must match the heating coil inlet node name.");
                    if (VRFTUInletNodeNum > 0 && heatCoilAirInNode > 0) {
                        ShowContinueError(state, format("... Terminal unit inlet node name = {}.", nodeID(VRFTUInletNodeNum)));
                        ShowContinueError(state, format("... Heating coil inlet node name = {}.", nodeID(heatCoilAirInNode)));
                    }
                    ErrorsFound = true;
                }
            }
        }
    }
    if (fanPlace == HVAC::FanPlace::BlowThru && !OAMixerUsed) {
        if (VRFTUInletNodeNum != fanInletNode) {
            ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
            ShowContinueError(state,
                              "... For blow thru fan when no OA mixer is specified the terminal unit inlet "
                              "node name must match the fan inlet node name.");
            if (VRFTUInletNodeNum > 0 && fanInletNode > 0) {
                ShowContinueError(state, format("... Terminal unit inlet node name = {}.", nodeID(VRFTUInletNodeNum)));
                ShowContinueError(state, format("... Fan inlet node name = {}.", nodeID(fanInletNode)));
            }
            ErrorsFound = true;
        }
    } else if (OAMixerUsed) { // when OA mixer is used TU inlet = OAMixer return node regardless of fan placement
        if (VRFTUInletNodeNum != VRFTUOAMixerRetNodeNum) {
            ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
            ShowContinueError(state,
                              "... When an OA mixer is specified the terminal unit inlet "
                              "node name must match the OA mixer return node name.");
            if (VRFTUInletNodeNum > 0 && VRFTUOAMixerRetNodeNum > 0) {
                ShowContinueError(state, format("... Terminal unit inlet node name = {}.", nodeID(VRFTUInletNodeNum)));
                ShowContinueError(state, format("... Fan inlet node name = {}.", nodeID(VRFTUOAMixerRetNodeNum)));
            }
            ErrorsFound = true;
        }
    }
    // check the next component
    if (CoolingCoilPresent) {
        if (fanPlace == HVAC::FanPlace::BlowThru) {
            if (fanOutletNode != coolCoilAirInNode) {
                ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
                ShowContinueError(state,
                                  "... For blow thru fan when a cooling coil is present "
                                  "fan outlet node name must match the cooling coil inlet node name.");
                if (fanOutletNode > 0 && coolCoilAirInNode > 0) {
                    ShowContinueError(state, format("... The fan outlet node name = {}.", nodeID(fanOutletNode)));
                    ShowContinueError(state, format("... Cooling coil inlet node name = {}.", nodeID(coolCoilAirInNode)));
                }
                ErrorsFound = true;
            }
        }
        if (!HeatingCoilPresent && fanPlace == HVAC::FanPlace::DrawThru) {
            if (coolCoilAirOutNode != fanInletNode) {
                ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
                ShowContinueError(state,
                                  "... For draw thru fan when a heating coil is not present "
                                  "the cooling coil outlet node name must match the fan inlet node name.");
                if (coolCoilAirOutNode > 0 && fanInletNode > 0) {
                    ShowContinueError(state, format("... Cooling coil outlet node name = {}.", nodeID(coolCoilAirOutNode)));
                    ShowContinueError(state, format("... The fan inlet node name = {}.", nodeID(fanInletNode)));
                }
                ErrorsFound = true;
            }
        }
    }
    if (HeatingCoilPresent) {
        if (fanPlace == HVAC::FanPlace::DrawThru) {
            if (heatCoilAirOutNode != fanInletNode) {
                ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
                ShowContinueError(state,
                                  "... For draw thru fan when a heating coil is present "
                                  "the heating coil outlet node name must match the fan inlet node name.");
                if (heatCoilAirOutNode > 0 && fanInletNode > 0) {
                    ShowContinueError(state, format("... Heating coil outlet node name = {}.", nodeID(heatCoilAirOutNode)));
                    ShowContinueError(state, format("... The fan inlet node name = {}.", nodeID(fanInletNode)));
                }
                ErrorsFound = true;
            }
        }
    }
    if (SuppHeatingCoilPresent) {
        if (SuppHeatCoilAirOutletNode != VRFTUOutletNodeNum) {
            ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
            ShowContinueError(state, "... The supplemental heating coil outlet node name must match the terminal unit outlet node name.");
            if (SuppHeatCoilAirOutletNode > 0 && VRFTUOutletNodeNum > 0) {
                ShowContinueError(state, format("... Supplemental heating coil outlet node name = {}.", nodeID(SuppHeatCoilAirOutletNode)));
                ShowContinueError(state, format("... Terminal unit outlet node name = {}.", nodeID(VRFTUOutletNodeNum)));
            }
            ErrorsFound = true;
        }
        if (fanPlace == HVAC::FanPlace::DrawThru) {
            if (fanOutletNode != SuppHeatCoilAirInletNode) {
                ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
                ShowContinueError(state,
                                  "... For draw thru fan when a supplemental heating coil is present "
                                  "the fan outlet node name must match the supplemental heating coil inlet node name.");
                if (fanOutletNode > 0 && SuppHeatCoilAirInletNode > 0) {
                    ShowContinueError(state, format("... Fan outlet node name = {}.", nodeID(fanOutletNode)));
                    ShowContinueError(state, format("... Supplemental heating coil inlet node name = {}.", nodeID(SuppHeatCoilAirInletNode)));
                }
                ErrorsFound = true;
            }
        } else {
            if (heatCoilAirOutNode != SuppHeatCoilAirInletNode) {
                ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
                ShowContinueError(state,
                                  "... For blow thru or no fan when a supplemental heating coil is present the heating "
                                  "coil outlet node name must match the supplemental heating coil inlet node name.");
                if (heatCoilAirOutNode > 0 && SuppHeatCoilAirInletNode > 0) {
                    ShowContinueError(state, format("... Heating coil outlet node name = {}.", nodeID(heatCoilAirOutNode)));
                    ShowContinueError(state, format("... Supplemental heating coil inlet node name = {}.", nodeID(SuppHeatCoilAirInletNode)));
                }
                ErrorsFound = true;
            }
        }
    } else if (CoolingCoilPresent && !HeatingCoilPresent && (fanPlace == HVAC::FanPlace::BlowThru || !FanPresent)) {
        if (coolCoilAirOutNode != VRFTUOutletNodeNum) {
            ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
            ShowContinueError(state,
                              "... For blow through or no fan and no heating or supplemental heating coil the cooling coil outlet node name must "
                              "match the terminal unit outlet node name.");
            if (coolCoilAirOutNode > 0 && VRFTUOutletNodeNum > 0) {
                ShowContinueError(state, format("... Cooling coil outlet node name = {}.", nodeID(coolCoilAirOutNode)));
                ShowContinueError(state, format("... Terminal unit outlet node name = {}.", nodeID(VRFTUOutletNodeNum)));
            }
            ErrorsFound = true;
        }
        if (fanPlace == HVAC::FanPlace::DrawThru) {
            if (fanOutletNode != VRFTUOutletNodeNum) {
                ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
                ShowContinueError(state,
                                  "... For draw through fan and no supplemental heating coil the fan outlet node name must "
                                  "match the terminal unit outlet node name.");
                if (fanOutletNode > 0 && VRFTUOutletNodeNum > 0) {
                    ShowContinueError(state, format("... Fan outlet node name = {}.", nodeID(fanOutletNode)));
                    ShowContinueError(state, format("... Terminal unit outlet node name = {}.", nodeID(VRFTUOutletNodeNum)));
                }
                ErrorsFound = true;
            }
        }
    } else if (fanPlace == HVAC::FanPlace::DrawThru) {
        if (fanOutletNode != VRFTUOutletNodeNum) {
            ShowSevereError(state, fmt::format("{}=\"{}\",", cTerminalUnitType, cTUName));
            ShowContinueError(state,
                              "... For draw through fan and no supplemental heating coil the fan outlet node name must "
                              "match the terminal unit outlet node name.");
            if (fanOutletNode > 0 && VRFTUOutletNodeNum > 0) {
                ShowContinueError(state, format("... Fan outlet node name = {}.", nodeID(fanOutletNode)));
                ShowContinueError(state, format("... Terminal unit outlet node name = {}.", nodeID(VRFTUOutletNodeNum)));
            }
            ErrorsFound = true;
        }
    }
}

void InitVRF(EnergyPlusData &state, int const VRFTUNum, int const ZoneNum, bool const FirstHVACIteration, Real64 &OnOffAirFlowRatio, Real64 &QZnReq)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   August 2010
    //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for initializations of the VRF Components.

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger initializations.

    using DataSizing::AutoSize;
    using DataZoneEquipment::CheckZoneEquipmentList;

    using PlantUtilities::InitComponentNodes;
    using SingleDuct::SimATMixer;

    static constexpr std::string_view RoutineName("InitVRF");

    int InNode;                  // TU inlet node
    int OutNode;                 // TU outlet node
    int OutsideAirNode;          // TU mixer outside air inlet node
    int NumTULoop;               // loop counter, number of TU's in list
    int ELLoop;                  // loop counter, number of zone equipment lists
    int ListLoop;                // loop counter, number of equipment is each list
    int VRFCond;                 // index to VRF condenser
    int TUIndex;                 // index to TU
    int TUListNum;               // index to VRF AC system terminal unit list
    int TUListIndex;             // pointer to TU list for this VRF system
    int IndexToTUInTUList;       // index to TU in TerminalUnilList
    Real64 RhoAir;               // air density at InNode
    Real64 CurrentEndTime;       // end time of current time step
    Real64 TimeStepSysLast(0.0); // system time step on last time step
    Real64 TempOutput;           // Sensible output of TU
    Real64 LoadToCoolingSP;      // thermostat load to cooling setpoint (W)
    Real64 LoadToHeatingSP;      // thermostat load to heating setpoint (W)
    bool EnableSystem;           // use to turn on secondary operating mode if OA temp limits exceeded
    bool ErrorsFound;            // flag returned from mining call
    Real64 rho;                  // density of water (kg/m3)
    Real64 OutsideDryBulbTemp;   // Outdoor air temperature at external node height
    bool errFlag;                // local error flag
    Real64 SuppHeatCoilLoad;     // additional heating required by supplemental heater (W)
    Real64 SuppHeatCoilCapacity; // supplemental heating coil size (W)

    // ALLOCATE and Initialize subroutine variables
    if (state.dataHVACVarRefFlow->MyOneTimeFlag) {

        state.dataHVACVarRefFlow->MyEnvrnFlag.allocate(state.dataHVACVarRefFlow->NumVRFTU);
        state.dataHVACVarRefFlow->MySizeFlag.allocate(state.dataHVACVarRefFlow->NumVRFTU);
        state.dataHVACVarRefFlow->MyVRFFlag.allocate(state.dataHVACVarRefFlow->NumVRFTU);
        state.dataHVACVarRefFlow->MyZoneEqFlag.allocate(state.dataHVACVarRefFlow->NumVRFTU);
        state.dataHVACVarRefFlow->MyBeginTimeStepFlag.allocate(state.dataHVACVarRefFlow->NumVRFCond);
        state.dataHVACVarRefFlow->MaxDeltaT.allocate(state.dataHVACVarRefFlow->NumVRFCond);
        state.dataHVACVarRefFlow->MinDeltaT.allocate(state.dataHVACVarRefFlow->NumVRFCond);
        state.dataHVACVarRefFlow->LastModeCooling.allocate(state.dataHVACVarRefFlow->NumVRFCond);
        state.dataHVACVarRefFlow->LastModeHeating.allocate(state.dataHVACVarRefFlow->NumVRFCond);
        state.dataHVACVarRefFlow->HeatingLoad.allocate(state.dataHVACVarRefFlow->NumVRFCond);
        state.dataHVACVarRefFlow->CoolingLoad.allocate(state.dataHVACVarRefFlow->NumVRFCond);
        state.dataHVACVarRefFlow->NumCoolingLoads.allocate(state.dataHVACVarRefFlow->NumVRFCond);
        state.dataHVACVarRefFlow->SumCoolingLoads.allocate(state.dataHVACVarRefFlow->NumVRFCond);
        state.dataHVACVarRefFlow->NumHeatingLoads.allocate(state.dataHVACVarRefFlow->NumVRFCond);
        state.dataHVACVarRefFlow->SumHeatingLoads.allocate(state.dataHVACVarRefFlow->NumVRFCond);
        state.dataHVACVarRefFlow->MyVRFCondFlag.allocate(state.dataHVACVarRefFlow->NumVRFCond);
        state.dataHVACVarRefFlow->MyEnvrnFlag = true;
        state.dataHVACVarRefFlow->MySizeFlag = true;
        state.dataHVACVarRefFlow->MyVRFFlag = true;
        state.dataHVACVarRefFlow->MyZoneEqFlag = true;
        state.dataHVACVarRefFlow->MyBeginTimeStepFlag = true;
        state.dataHVACVarRefFlow->MaxDeltaT = 0.0;
        state.dataHVACVarRefFlow->MinDeltaT = 0.0;
        state.dataHVACVarRefFlow->LastModeCooling = false;
        state.dataHVACVarRefFlow->LastModeHeating = true;
        state.dataHVACVarRefFlow->NumCoolingLoads = 0;
        state.dataHVACVarRefFlow->SumCoolingLoads = 0.0;
        state.dataHVACVarRefFlow->NumHeatingLoads = 0;
        state.dataHVACVarRefFlow->SumHeatingLoads = 0.0;

        state.dataHVACVarRefFlow->MyOneTimeFlag = false;
        state.dataHVACVarRefFlow->MyVRFCondFlag = true;

    } // IF (MyOneTimeFlag) THEN

    // identify VRF condenser connected to this TU
    VRFCond = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFSysNum;
    TUListIndex = state.dataHVACVarRefFlow->VRF(VRFCond).ZoneTUListPtr;
    InNode = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum;
    OutNode = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOutletNodeNum;
    OutsideAirNode = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum;
    IndexToTUInTUList = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).IndexToTUInTUList;

    SuppHeatCoilCapacity = 0.0;
    SuppHeatCoilLoad = 0.0;
    LoadToCoolingSP = 0.0;
    LoadToHeatingSP = 0.0;
    ErrorsFound = false;
    bool SetPointErrorFlag = false;

    // set condenser inlet temp, used as surrogate for OAT (used to check limits of operation)
    if (state.dataHVACVarRefFlow->VRF(VRFCond).CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
        OutsideDryBulbTemp = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRF(VRFCond).CondenserNodeNum).Temp;
    } else {
        if (OutsideAirNode == 0) {
            OutsideDryBulbTemp = state.dataEnvrn->OutDryBulbTemp;
        } else {
            OutsideDryBulbTemp = state.dataLoopNodes->Node(OutsideAirNode).Temp;
        }
    }

    if (allocated(state.dataAvail->ZoneComp)) {
        auto &availMgr = state.dataAvail->ZoneComp(DataZoneEquipment::ZoneEquipType::VariableRefrigerantFlowTerminal).ZoneCompAvailMgrs(VRFTUNum);
        if (state.dataHVACVarRefFlow->MyZoneEqFlag(VRFTUNum)) { // initialize the name of each availability manager list and zone number
            availMgr.AvailManagerListName = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).AvailManagerListName;
            availMgr.ZoneNum = ZoneNum;
            state.dataHVACVarRefFlow->MyZoneEqFlag(VRFTUNum) = false;
        }
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).availStatus = availMgr.availStatus;
    }

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MySuppCoilPlantScanFlag && allocated(state.dataPlnt->PlantLoop)) {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilType_Num == HVAC::Coil_HeatingWater) {
            // hot water supplemental heating coil
            errFlag = false;
            PlantUtilities::ScanPlantLoopsForObject(state,
                                                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilName,
                                                    PlantEquipmentType::CoilWaterSimpleHeating,
                                                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilPlantLoc,
                                                    errFlag,
                                                    _,
                                                    _,
                                                    _,
                                                    _,
                                                    _);

            WaterCoils::SetCoilDesFlow(state,
                                       HVAC::cAllCoilTypes(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilType_Num),
                                       state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilName,
                                       state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow,
                                       errFlag);

            if (errFlag) {
                ShowFatalError(state, format("{}: Program terminated for previous conditions.", RoutineName));
            }
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow = WaterCoils::GetCoilMaxWaterFlowRate(
                state, "Coil:Heating:Water", state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilName, ErrorsFound);

            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow > 0.0) {
                rho = state.dataPlnt->PlantLoop(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilPlantLoc.loopNum)
                          .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow =
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow * rho;
            }

            // fill fluid outlet node for hot water coil SuppHeatCoilFluidOutletNode
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidOutletNode =
                DataPlant::CompData::getPlantComponent(state, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilPlantLoc).NodeNumOut;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MySuppCoilPlantScanFlag = false;

        } else if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilType_Num == HVAC::Coil_HeatingSteam) {
            // steam supplemental heating coil
            errFlag = false;
            PlantUtilities::ScanPlantLoopsForObject(state,
                                                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilName,
                                                    PlantEquipmentType::CoilSteamAirHeating,
                                                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilPlantLoc,
                                                    errFlag,
                                                    _,
                                                    _,
                                                    _,
                                                    _,
                                                    _);
            if (errFlag) {
                ShowFatalError(state, format("{}: Program terminated for previous conditions.", RoutineName));
            }
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow =
                SteamCoils::GetCoilMaxSteamFlowRate(state, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilIndex, ErrorsFound);
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow > 0.0) {
                Real64 TempSteamIn = 100.0;
                Real64 SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, TempSteamIn, 1.0, RoutineName);
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow =
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow * SteamDensity;
            }

            // fill fluid outlet node for steam coil SuppHeatCoilFluidOutletNode
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidOutletNode =
                DataPlant::CompData::getPlantComponent(state, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilPlantLoc).NodeNumOut;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MySuppCoilPlantScanFlag = false;

        } else { // VRF terminal unit not connected to plant
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MySuppCoilPlantScanFlag = false;
        }
    } else if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MySuppCoilPlantScanFlag && !state.dataGlobal->AnyPlantInModel) {
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MySuppCoilPlantScanFlag = false;
    }

    // one-time check to see if VRF TU's are on ZoneHVAC:EquipmentList or AirloopHVAC or issue warning
    if (state.dataHVACVarRefFlow->ZoneEquipmentListNotChecked) {
        if (state.dataAirLoop->AirLoopInputsFilled) {
            state.dataHVACVarRefFlow->ZoneEquipmentListNotChecked = false;
        }
        bool AirLoopFound = false;
        bool errorsFound = false;
        bool AirNodeFound = false;
        int ctrlZoneNum = 0;
        std::string const cCurrentModuleObject = "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow";
        for (TUListNum = 1; TUListNum <= state.dataHVACVarRefFlow->NumVRFTULists; ++TUListNum) {
            for (NumTULoop = 1; NumTULoop <= state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).NumTUInList; ++NumTULoop) {
                AirLoopFound = false; // reset for next TU
                ctrlZoneNum = 0;      // reset for next TU
                TUIndex = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTULoop);
                std::string const thisObjectName = state.dataHVACVarRefFlow->VRFTU(TUIndex).Name;
                if (state.dataHVACVarRefFlow->VRFTU(TUIndex).isInZone) {
                    goto EquipList_exit; // already found previously
                }
                for (ELLoop = 1; ELLoop <= state.dataGlobal->NumOfZones; ++ELLoop) { // NumOfZoneEquipLists
                    if (state.dataZoneEquip->ZoneEquipList(ELLoop).Name == "") {
                        continue; // dimensioned by NumOfZones.  Only valid ones have names.
                    }
                    for (ListLoop = 1; ListLoop <= state.dataZoneEquip->ZoneEquipList(ELLoop).NumOfEquipTypes; ++ListLoop) {
                        if (!Util::SameString(state.dataZoneEquip->ZoneEquipList(ELLoop).EquipTypeName(ListLoop),
                                              tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(TUIndex).type])) {
                            continue;
                        }
                        if (!Util::SameString(state.dataZoneEquip->ZoneEquipList(ELLoop).EquipName(ListLoop),
                                              state.dataHVACVarRefFlow->VRFTU(TUIndex).Name)) {
                            continue;
                        }
                        state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneNum = ELLoop;
                        state.dataHVACVarRefFlow->VRFTU(TUIndex).isInZone = true;
                        if (state.dataHVACVarRefFlow->VRF(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFSysNum).MasterZonePtr == ELLoop) {
                            state.dataHVACVarRefFlow->VRF(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFSysNum).MasterZoneTUIndex = TUIndex;
                        }
                        if (state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneAirNode == 0) {
                            bool ZoneNodeNotFound = true;
                            for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                                if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                                    continue;
                                }
                                for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumExhaustNodes; ++NodeNum) {
                                    if (state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUInletNodeNum ==
                                        state.dataZoneEquip->ZoneEquipConfig(CtrlZone).ExhaustNode(NodeNum)) {
                                        state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneAirNode =
                                            state.dataZoneEquip->ZoneEquipConfig(CtrlZone).ZoneNode;
                                        ZoneNodeNotFound = false;
                                        break;
                                    }
                                }
                                for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumInletNodes; ++NodeNum) {
                                    if (state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUOutletNodeNum ==
                                        state.dataZoneEquip->ZoneEquipConfig(CtrlZone).InletNode(NodeNum)) {
                                        state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneAirNode =
                                            state.dataZoneEquip->ZoneEquipConfig(CtrlZone).ZoneNode;
                                        ZoneNodeNotFound = false;
                                        break;
                                    }
                                }
                                if (!ZoneNodeNotFound) {
                                    break;
                                }
                            }
                            if (ZoneNodeNotFound) {
                                ShowSevereError(state,
                                                format("ZoneHVAC:TerminalUnit:VariableRefrigerantFlow \"{}\" Zone terminal unit air inlet node name "
                                                       "must be the same as a zone inlet or exhaust node name.",
                                                       state.dataHVACVarRefFlow->VRFTU(TUIndex).Name));
                                ShowContinueError(state,
                                                  "... Zone inlet and exhaust node name is specified in ZoneHVAC:EquipmentConnections object.");
                                ShowContinueError(state,
                                                  format("... Zone terminal unit inlet node name  = {}",
                                                         state.dataLoopNodes->NodeID(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUInletNodeNum)));
                                ShowContinueError(state,
                                                  format("... Zone terminal unit outlet node name = {}",
                                                         state.dataLoopNodes->NodeID(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUOutletNodeNum)));
                            }
                        }
                        goto EquipList_exit;
                    }
                }
                // check if the TU is connected to an air loop
                if (!state.dataHVACVarRefFlow->VRFTU(TUIndex).isInAirLoop) {
                    for (int AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
                        for (int BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches; ++BranchNum) {
                            for (int CompNum = 1;
                                 CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).TotalComponents;
                                 ++CompNum) {
                                if (Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).Name,
                                                     thisObjectName) &&
                                    Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).TypeOf,
                                                     cCurrentModuleObject)) {
                                    state.dataHVACVarRefFlow->VRFTU(TUIndex).airLoopNum = AirLoopNum;
                                    AirLoopFound = true;
                                    state.dataHVACVarRefFlow->VRFTU(TUIndex).isInAirLoop = true;
                                    BranchNodeConnections::TestCompSet(
                                        state,
                                        cCurrentModuleObject,
                                        thisObjectName,
                                        state.dataLoopNodes->NodeID(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUInletNodeNum),
                                        state.dataLoopNodes->NodeID(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUOutletNodeNum),
                                        "Air Nodes");
                                    if (state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneNum > 0) {
                                        state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneAirNode =
                                            state.dataZoneEquip->ZoneEquipConfig(state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneNum).ZoneNode;
                                        int ControlledZoneNum = state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneNum;
                                        for (int TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TstatZoneNum) {
                                            if (state.dataZoneCtrls->TempControlledZone(TstatZoneNum).ActualZoneNum !=
                                                state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneNum) {
                                                continue;
                                            }
                                            state.dataHVACVarRefFlow->VRF(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFSysNum).MasterZoneTUIndex =
                                                TUIndex;
                                            AirNodeFound = true;
                                            ctrlZoneNum = ControlledZoneNum;
                                            goto EquipList_exit;
                                        }
                                        for (int TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumComfortControlledZones; ++TstatZoneNum) {
                                            if (state.dataZoneCtrls->ComfortControlledZone(TstatZoneNum).ActualZoneNum !=
                                                state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneNum) {
                                                continue;
                                            }
                                            state.dataHVACVarRefFlow->VRF(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFSysNum).MasterZoneTUIndex =
                                                TUIndex;
                                            AirNodeFound = true;
                                            ctrlZoneNum = ControlledZoneNum;
                                            goto EquipList_exit;
                                        }
                                        if (!AirNodeFound && state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneNum > 0) {
                                            ShowSevereError(state, format("Input errors for {}:{}", cCurrentModuleObject, thisObjectName));
                                            ShowContinueError(state, "Did not find Air node (Zone with Thermostat or Thermal Comfort Thermostat).");
                                            // ShowContinueError(state, format("specified Controlling Zone or Thermostat Location name = {}{}", //,
                                            // loc_controlZoneName));
                                            errorsFound = true;
                                        }
                                    } else if (AirLoopFound) { // control zone name not entered in TU object input
                                        state.dataHVACVarRefFlow->VRFTU(TUIndex).isSetPointControlled = true;
                                    }
                                }
                                if (AirLoopFound) {
                                    break;
                                }
                            }
                            if (AirLoopFound) {
                                break;
                            }
                        }
                        if (AirLoopFound) {
                            break;
                        }
                    }
                }

                // check if the TU is connected to an outside air system
                if (!AirLoopFound && !state.dataHVACVarRefFlow->VRFTU(TUIndex).isInOASys) {
                    for (int OASysNum = 1; OASysNum <= state.dataAirLoop->NumOASystems; ++OASysNum) {
                        for (int OACompNum = 1; OACompNum <= state.dataAirLoop->OutsideAirSys(OASysNum).NumComponents; ++OACompNum) {
                            if (!Util::SameString(state.dataAirLoop->OutsideAirSys(OASysNum).ComponentName(OACompNum),
                                                  state.dataHVACVarRefFlow->VRFTU(TUIndex).Name) ||
                                !Util::SameString(state.dataAirLoop->OutsideAirSys(OASysNum).ComponentType(OACompNum), cCurrentModuleObject)) {
                                continue;
                            }
                            state.dataHVACVarRefFlow->VRFTU(TUIndex).airLoopNum = 0; // need air loop number here?
                            state.dataHVACVarRefFlow->VRFTU(TUIndex).isInOASys = true;
                            AirLoopFound = true;
                            state.dataHVACVarRefFlow->VRFTU(TUIndex).isSetPointControlled = true;
                            // user may have inadvertently entered a zone name in the OA system TU object
                            state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneNum = 0;
                            state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneAirNode = 0;
                            BranchNodeConnections::TestCompSet(
                                state,
                                cCurrentModuleObject,
                                thisObjectName,
                                state.dataLoopNodes->NodeID(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUInletNodeNum),
                                state.dataLoopNodes->NodeID(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUOutletNodeNum),
                                "Air Nodes");
                            goto EquipList_exit;
                        }
                    }
                }
            EquipList_exit:;
                if (ctrlZoneNum > 0) {
                    int inletNodeADUNum = 0;
                    DataZoneEquipment::ZoneEquipType sysType_Num = DataZoneEquipment::ZoneEquipType::Invalid;
                    std::string sysName = "";
                    for (int inletNode = 1; inletNode <= state.dataZoneEquip->ZoneEquipConfig(ctrlZoneNum).NumInletNodes; inletNode++) {
                        if (state.dataZoneEquip->ZoneEquipConfig(ctrlZoneNum).InletNodeAirLoopNum(inletNode) !=
                            state.dataHVACVarRefFlow->VRFTU(TUIndex).airLoopNum) {
                            continue;
                        }
                        inletNodeADUNum = state.dataZoneEquip->ZoneEquipConfig(ctrlZoneNum).InletNodeADUNum(inletNode);
                        if (inletNodeADUNum > 0 && inletNodeADUNum <= (int)state.dataDefineEquipment->AirDistUnit.size()) {
                            sysType_Num = DataZoneEquipment::ZoneEquipType::AirDistributionUnit;
                            sysName = state.dataDefineEquipment->AirDistUnit(inletNodeADUNum).Name;
                            break;
                        }
                    }
                    if (inletNodeADUNum > 0) {
                        if (state.dataZoneEquip->ZoneEquipConfig(ctrlZoneNum).EquipListIndex > 0) {
                            for (int EquipNum = 1;
                                 EquipNum <=
                                 state.dataZoneEquip->ZoneEquipList(state.dataZoneEquip->ZoneEquipConfig(ctrlZoneNum).EquipListIndex).NumOfEquipTypes;
                                 ++EquipNum) {
                                if ((state.dataZoneEquip->ZoneEquipList(state.dataZoneEquip->ZoneEquipConfig(ctrlZoneNum).EquipListIndex)
                                         .EquipType(EquipNum) != sysType_Num) ||
                                    state.dataZoneEquip->ZoneEquipList(state.dataZoneEquip->ZoneEquipConfig(ctrlZoneNum).EquipListIndex)
                                            .EquipName(EquipNum) != sysName) {
                                    continue;
                                }
                                state.dataHVACVarRefFlow->VRFTU(TUIndex).zoneSequenceCoolingNum =
                                    state.dataZoneEquip->ZoneEquipList(state.dataZoneEquip->ZoneEquipConfig(ctrlZoneNum).EquipListIndex)
                                        .CoolingPriority(EquipNum);
                                state.dataHVACVarRefFlow->VRFTU(TUIndex).zoneSequenceHeatingNum =
                                    state.dataZoneEquip->ZoneEquipList(state.dataZoneEquip->ZoneEquipConfig(ctrlZoneNum).EquipListIndex)
                                        .HeatingPriority(EquipNum);
                                break;
                            }
                        }
                    } else {
                        ShowSevereError(state, format("Input errors for {}:{}", cCurrentModuleObject, thisObjectName));
                        ShowContinueError(state, "Did not find ZoneHVAC:EquipmentList connected to this VRF terminal unit.");
                        errorsFound = true;
                    }
                }

                // Find the number of zones (zone Inlet nodes) attached to an air loop from the air loop number
                if (AirLoopFound || state.dataHVACVarRefFlow->VRFTU(TUIndex).isInAirLoop) {
                    int NumAirLoopZones = 0;
                    bool initLoadBasedControlFlowFracFlagReady = false;
                    Real64 initLoadBasedControlCntrlZoneTerminalUnitMassFlowRateMax = 0.0;
                    if (allocated(state.dataAirLoop->AirToZoneNodeInfo) && state.dataHVACVarRefFlow->VRFTU(TUIndex).airLoopNum > 0) {
                        NumAirLoopZones = state.dataAirLoop->AirToZoneNodeInfo(state.dataHVACVarRefFlow->VRFTU(TUIndex).airLoopNum).NumZonesCooled +
                                          state.dataAirLoop->AirToZoneNodeInfo(state.dataHVACVarRefFlow->VRFTU(TUIndex).airLoopNum).NumZonesHeated;
                    }
                    if (allocated(state.dataAirLoop->AirToZoneNodeInfo)) {
                        initLoadBasedControlFlowFracFlagReady = true;
                        for (int ZoneInSysIndex = 1; ZoneInSysIndex <= NumAirLoopZones; ++ZoneInSysIndex) {
                            // zone inlet nodes for cooling
                            if (state.dataAirLoop->AirToZoneNodeInfo(state.dataHVACVarRefFlow->VRFTU(TUIndex).airLoopNum).NumZonesCooled > 0) {
                                if (state.dataAirLoop->AirToZoneNodeInfo(state.dataHVACVarRefFlow->VRFTU(TUIndex).airLoopNum)
                                        .TermUnitCoolInletNodes(ZoneInSysIndex) == -999) {
                                    // the data structure for the zones inlet nodes has not been filled
                                    initLoadBasedControlFlowFracFlagReady = false;
                                } else {
                                    int ZoneInletNodeNum = state.dataAirLoop->AirToZoneNodeInfo(state.dataHVACVarRefFlow->VRFTU(TUIndex).airLoopNum)
                                                               .TermUnitCoolInletNodes(ZoneInSysIndex);
                                    if (state.dataLoopNodes->Node(ZoneInletNodeNum).MassFlowRateMax == -999.0) {
                                        // the node mass flow rate has not been set
                                        initLoadBasedControlFlowFracFlagReady = false;
                                    }
                                }
                            }
                            // zone inlet nodes for heating
                            if (state.dataAirLoop->AirToZoneNodeInfo(state.dataHVACVarRefFlow->VRFTU(TUIndex).airLoopNum).NumZonesHeated > 0) {
                                if (state.dataAirLoop->AirToZoneNodeInfo(state.dataHVACVarRefFlow->VRFTU(TUIndex).airLoopNum)
                                        .TermUnitHeatInletNodes(ZoneInSysIndex) == -999) {
                                    // the data structure for the zones inlet nodes has not been filled
                                    initLoadBasedControlFlowFracFlagReady = false;
                                } else {
                                    int ZoneInletNodeNum = state.dataAirLoop->AirToZoneNodeInfo(state.dataHVACVarRefFlow->VRFTU(TUIndex).airLoopNum)
                                                               .TermUnitHeatInletNodes(ZoneInSysIndex);
                                    if (state.dataLoopNodes->Node(ZoneInletNodeNum).MassFlowRateMax == -999.0) {
                                        // the node mass flow rate has not been set
                                        initLoadBasedControlFlowFracFlagReady = false;
                                    }
                                }
                            }
                        }
                    }
                    if (allocated(state.dataAirLoop->AirToZoneNodeInfo) && initLoadBasedControlFlowFracFlagReady) {
                        Real64 SumOfMassFlowRateMax = 0.0; // initialize the sum of the maximum flows
                        for (int ZoneInSysIndex = 1; ZoneInSysIndex <= NumAirLoopZones; ++ZoneInSysIndex) {
                            int ZoneInletNodeNum = state.dataAirLoop->AirToZoneNodeInfo(state.dataHVACVarRefFlow->VRFTU(TUIndex).airLoopNum)
                                                       .TermUnitCoolInletNodes(ZoneInSysIndex);
                            SumOfMassFlowRateMax += state.dataLoopNodes->Node(ZoneInletNodeNum).MassFlowRateMax;
                            if (state.dataAirLoop->AirToZoneNodeInfo(state.dataHVACVarRefFlow->VRFTU(TUIndex).airLoopNum)
                                    .CoolCtrlZoneNums(ZoneInSysIndex) == state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneNum) {
                                initLoadBasedControlCntrlZoneTerminalUnitMassFlowRateMax =
                                    state.dataLoopNodes->Node(ZoneInletNodeNum).MassFlowRateMax;
                            }
                        }
                        if (SumOfMassFlowRateMax != 0.0 && state.dataAirLoop->AirLoopInputsFilled) {
                            if (initLoadBasedControlCntrlZoneTerminalUnitMassFlowRateMax >= HVAC::SmallAirVolFlow) {
                                state.dataHVACVarRefFlow->VRFTU(TUIndex).controlZoneMassFlowFrac =
                                    initLoadBasedControlCntrlZoneTerminalUnitMassFlowRateMax / SumOfMassFlowRateMax;
                                BaseSizer::reportSizerOutput(state,
                                                             tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(TUIndex).type],
                                                             state.dataHVACVarRefFlow->VRFTU(TUIndex).Name,
                                                             "Fraction of Supply Air Flow That Goes Through the Controlling Zone",
                                                             state.dataHVACVarRefFlow->VRFTU(TUIndex).controlZoneMassFlowFrac);
                                state.dataHVACVarRefFlow->VRFTU(TUIndex).isSetPointControlled = false; // redundant
                            } else {
                                if (state.dataHVACVarRefFlow->VRFTU(TUIndex).isInAirLoop && state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneNum == 0 &&
                                    state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneAirNode == 0) {
                                    // TU must be set point controlled and use constant fan mode (or coil out T won't change with PLR/air flow)
                                    state.dataHVACVarRefFlow->VRFTU(TUIndex).isSetPointControlled = true;
                                    if (state.dataHVACVarRefFlow->VRFTU(TUIndex).fanOpModeSched != nullptr) {
                                        if (state.dataHVACVarRefFlow->VRFTU(TUIndex).fanOpModeSched->getCurrentVal() == 0.0) {
                                            ShowSevereError(state,
                                                            format("{} = {}",
                                                                   tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(TUIndex).type],
                                                                   state.dataHVACVarRefFlow->VRFTU(TUIndex).Name));
                                            ShowContinueError(state,
                                                              "When using set point control, fan operating mode must be continuous (fan "
                                                              "operating mode schedule values > 0).");
                                            ShowContinueError(state,
                                                              format("Error found in Supply Air Fan Operating Mode Schedule Name = {}",
                                                                     state.dataHVACVarRefFlow->VRFTU(TUIndex).fanOpModeSched->Name));
                                            ShowContinueError(state, "...schedule values must be (>0., <=1.)");
                                            ErrorsFound = true;
                                        }
                                    }
                                } else {
                                    ShowSevereError(state,
                                                    format("{} = {}",
                                                           tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(TUIndex).type],
                                                           state.dataHVACVarRefFlow->VRFTU(TUIndex).Name));
                                    ShowContinueError(state, " The Fraction of Supply Air Flow That Goes Through the Controlling Zone is set to 1.");
                                    state.dataHVACVarRefFlow->VRFTU(TUIndex).controlZoneMassFlowFrac = 1.0;
                                    BaseSizer::reportSizerOutput(state,
                                                                 tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(TUIndex).type],
                                                                 state.dataHVACVarRefFlow->VRFTU(TUIndex).Name,
                                                                 "Fraction of Supply Air Flow That Goes Through the Controlling Zone",
                                                                 state.dataHVACVarRefFlow->VRFTU(TUIndex).controlZoneMassFlowFrac);
                                }
                            }
                        } else if (state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneNum == 0) {
                            // TU must be set point controlled and use constant fan mode (or coil outlet T won't change with PLR/air flow rate)
                            // TU inlet air flow rate is also determined by OA system, not TU
                            state.dataHVACVarRefFlow->VRFTU(TUIndex).isSetPointControlled = true;
                        }
                    }
                }

                if (state.dataHVACVarRefFlow->VRFTU(TUIndex).isInZone && state.dataAirLoop->AirLoopInputsFilled) {
                    if (state.dataHVACVarRefFlow->VRFTU(TUIndex).fanPlace == HVAC::FanPlace::Invalid) {
                        ShowSevereError(state,
                                        format("ZoneHVAC:TerminalUnit:VariableRefrigerantFlow = {}", state.dataHVACVarRefFlow->VRFTU(TUIndex).Name));
                        ShowContinueError(state, "Illegal Supply Air Fan Placement.");
                        ErrorsFound = true;
                    }
                    if (state.dataHVACVarRefFlow->VRFTU(TUIndex).FanIndex == 0) {
                        ShowSevereError(state,
                                        format("ZoneHVAC:TerminalUnit:VariableRefrigerantFlow = {}", state.dataHVACVarRefFlow->VRFTU(TUIndex).Name));
                        ShowContinueError(state, "VRF Terminal Unit fan is required when used as zone equipment.");
                        ErrorsFound = true;
                    }
                }

                if (state.dataHVACVarRefFlow->VRFTU(TUIndex).isSetPointControlled && state.dataAirLoop->AirLoopInputsFilled) {
                    bool missingSetPoint = false;
                    Real64 TUOutNodeSP = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUOutletNodeNum).TempSetPoint;
                    Real64 coolCoilOutNodeSP = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).coolCoilAirOutNode).TempSetPoint;
                    Real64 heatCoilOutNodeSP = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).heatCoilAirOutNode).TempSetPoint;
                    // SP can be at outlet of TU or at outlet of coils
                    // if supp heat coil is present, a SP must be at the outlet of the TU
                    if (state.dataHVACVarRefFlow->VRFTU(TUIndex).SuppHeatingCoilPresent) {
                        if (TUOutNodeSP == DataLoopNode::SensedNodeFlagValue) {
                            missingSetPoint = true;
                        }
                    } else {
                        if (state.dataHVACVarRefFlow->VRFTU(TUIndex).fanPlace == HVAC::FanPlace::DrawThru) {
                            // then SP must be at TU outlet
                            if (TUOutNodeSP == DataLoopNode::SensedNodeFlagValue) {
                                missingSetPoint = true;
                            }
                            // or at coil outlet nodes
                            if (missingSetPoint) {
                                if (coolCoilOutNodeSP != DataLoopNode::SensedNodeFlagValue &&
                                    heatCoilOutNodeSP != DataLoopNode::SensedNodeFlagValue) {
                                    missingSetPoint = false;
                                }
                            }
                        } else {
                            // else fan is blow thru or missing
                            if (TUOutNodeSP == DataLoopNode::SensedNodeFlagValue) {
                                missingSetPoint = true;
                            }
                        }
                    }
                    if (missingSetPoint) {
                        if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                            ShowSevereError(state,
                                            format("ZoneHVAC:TerminalUnit:VariableRefrigerantFlow: Missing temperature setpoint for {}",
                                                   state.dataHVACVarRefFlow->VRFTU(TUIndex).Name));
                            ShowContinueError(state, "...use a Setpoint Manager to establish a setpoint at the TU or coil(s) outlet node.");
                            ErrorsFound = true;
                        } else if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                            bool SPNotFound = false;
                            EMSManager::CheckIfNodeSetPointManagedByEMS(
                                state, state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUOutletNodeNum, HVAC::CtrlVarType::Temp, SetPointErrorFlag);
                            SPNotFound = SPNotFound || SetPointErrorFlag;
                            EMSManager::CheckIfNodeSetPointManagedByEMS(
                                state, state.dataHVACVarRefFlow->VRFTU(TUIndex).coolCoilAirOutNode, HVAC::CtrlVarType::Temp, SetPointErrorFlag);
                            SPNotFound = SPNotFound || SetPointErrorFlag;
                            EMSManager::CheckIfNodeSetPointManagedByEMS(
                                state, state.dataHVACVarRefFlow->VRFTU(TUIndex).heatCoilAirOutNode, HVAC::CtrlVarType::Temp, SetPointErrorFlag);
                            SPNotFound = SPNotFound || SetPointErrorFlag;

                            // We disable the check at end (if API), because one of the nodes is enough, so there's an almost certainty
                            // that it will throw as you're unlikely going to actuate all three nodes
                            // It's not ideal, but it's better to let slide a bad condition rather than throw false positives...
                            state.dataLoopNodes->NodeSetpointCheck(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUOutletNodeNum)
                                .needsSetpointChecking = false;
                            state.dataLoopNodes->NodeSetpointCheck(state.dataHVACVarRefFlow->VRFTU(TUIndex).coolCoilAirOutNode)
                                .needsSetpointChecking = false;
                            state.dataLoopNodes->NodeSetpointCheck(state.dataHVACVarRefFlow->VRFTU(TUIndex).heatCoilAirOutNode)
                                .needsSetpointChecking = false;

                            if (SPNotFound && state.dataAirLoop->AirLoopInputsFilled) {
                                ShowSevereError(
                                    state,
                                    format("ZoneHVAC:TerminalUnit:VariableRefrigerantFlow: Missing temperature setpoint for unitary system = {}",
                                           state.dataHVACVarRefFlow->VRFTU(TUIndex).Name));
                                ShowContinueError(state, "...use a Setpoint Manager to establish a setpoint at the TU or coil(s) outlet node.");
                                ShowContinueError(state, "...or use an EMS actuator to establish a temperature setpoint at the coil control node.");
                                ErrorsFound = true;
                            }
                        }
                    }
                }

                if (state.dataHVACVarRefFlow->VRFTU(TUIndex).isInAirLoop || state.dataHVACVarRefFlow->VRFTU(TUIndex).isInOASys ||
                    state.dataHVACVarRefFlow->VRFTU(TUIndex).isInZone) {
                    continue;
                }
                if (!state.dataAirLoop->AirLoopInputsFilled) {
                    continue;
                }
                ShowSevereError(state,
                                format("InitVRF: VRF Terminal Unit = [{},{}] is not on any ZoneHVAC:EquipmentList, AirloopHVAC or "
                                       "AirLoopHVAC:OutdoorAirSystem:EquipmentList.  It will not be simulated.",
                                       tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(TUIndex).type],
                                       state.dataHVACVarRefFlow->VRFTU(TUIndex).Name));
                ShowContinueError(state, "...The VRF AC System associated with this terminal unit may also not be simulated.");
            }
        }

        // TU inlet node must be the same as a zone exhaust node and the OA Mixer return node
        // check that TU inlet node is a zone exhaust node.
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInZone &&
            (!state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerExists ||
             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerType == HVAC::MixerType::SupplySide)) {
            bool ZoneNodeNotFound = true;
            for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                    continue;
                }
                for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumExhaustNodes; ++NodeNum) {
                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum ==
                        state.dataZoneEquip->ZoneEquipConfig(CtrlZone).ExhaustNode(NodeNum)) {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneAirNode = state.dataZoneEquip->ZoneEquipConfig(CtrlZone).ZoneNode;
                        ZoneNodeNotFound = false;
                        break;
                    }
                }
                if (!ZoneNodeNotFound) {
                    break;
                }
            }
            if (ZoneNodeNotFound && !state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInAirLoop) {
                ShowSevereError(state,
                                format("{} \"{}\" Zone terminal unit air inlet node name must be the same as a zone exhaust node name.",
                                       cCurrentModuleObject,
                                       state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                ShowContinueError(state, "... Zone exhaust node name is specified in ZoneHVAC:EquipmentConnections object.");
                ShowContinueError(state,
                                  format("... Zone terminal unit inlet node name = {}",
                                         state.dataLoopNodes->NodeID(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum)));
                ErrorsFound = true;
            }
        }
        // check OA Mixer return node
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInZone && !state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerExists &&
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerUsed) {
            Array1D_int OANodeNums = MixedAir::GetOAMixerNodeNumbers(state, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerName, errFlag);
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum != OANodeNums(3)) {
                ShowSevereError(
                    state,
                    format("{} \"{}\" Zone terminal unit air inlet node name must be the same as the OutdoorAir:Mixer return air node name.",
                           cCurrentModuleObject,
                           state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                ShowContinueError(state,
                                  format("... Zone terminal unit air inlet node name = {}",
                                         state.dataLoopNodes->NodeID(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum)));
                ShowContinueError(state, format("... OutdoorAir:Mixer return air node name = {}", state.dataLoopNodes->NodeID(OANodeNums(3))));
                ErrorsFound = true;
            }
        }
        // check that TU outlet node is a zone inlet node.
        if ((state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInZone &&
             (!state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerExists ||
              state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerType == HVAC::MixerType::InletSide))) {
            bool ZoneNodeNotFound = true;
            for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                    continue;
                }
                for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumInletNodes; ++NodeNum) {
                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOutletNodeNum ==
                        state.dataZoneEquip->ZoneEquipConfig(CtrlZone).InletNode(NodeNum)) {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneAirNode = state.dataZoneEquip->ZoneEquipConfig(CtrlZone).ZoneNode;
                        ZoneNodeNotFound = false;
                        break;
                    }
                }
                if (!ZoneNodeNotFound) {
                    break;
                }
            }
            if (ZoneNodeNotFound) {
                ShowSevereError(state,
                                format("{} \"{}\" Zone terminal unit air outlet node name must be the same as a zone inlet node name.",
                                       cCurrentModuleObject,
                                       state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                ShowContinueError(state, "... Zone inlet node name is specified in ZoneHVAC:EquipmentConnections object.");
                ShowContinueError(state,
                                  format("... Zone terminal unit outlet node name = {}",
                                         state.dataLoopNodes->NodeID(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOutletNodeNum)));
                ErrorsFound = true;
            }
        }

        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInZone && state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerExists) {
            //   check that OA flow in cooling must be set to zero when connected to DOAS
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow != 0) {
                ShowWarningError(state, format("{} = {}", cCurrentModuleObject, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                ShowContinueError(state, format(".. Cooling Outdoor Air Flow Rate must be zero when {}", cCurrentModuleObject));
                ShowContinueError(state, "..object is connected to central dedicated outdoor air system via AirTerminal:SingleDuct:Mixer");
                ShowContinueError(state, ".. Cooling Outdoor Air Flow Rate is set to 0 and simulation continues.");
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow = 0;
            }
            //   check that OA flow in heating must be set to zero when connected to DOAS
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow != 0) {
                ShowWarningError(state, format("{} = {}", cCurrentModuleObject, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                ShowContinueError(state, format(".. Heating Outdoor Air Flow Rate must be zero when {}", cCurrentModuleObject));
                ShowContinueError(state, "..object is connected to central dedicated outdoor air system via AirTerminal:SingleDuct:Mixer");
                ShowContinueError(state, ".. Heating Outdoor Air Flow Rate is set to 0 and simulation continues.");
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow = 0;
            }
            //   check that OA flow in no cooling and no heating must be set to zero when connected to DOAS
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow != 0) {
                ShowWarningError(state, format("{} = {}", cCurrentModuleObject, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                ShowContinueError(state, format(".. No Load Outdoor Air Flow Rate must be zero when {}", cCurrentModuleObject));
                ShowContinueError(state, "..object is connected to central dedicated outdoor air system via AirTerminal:SingleDuct:Mixer");
                ShowContinueError(state, ".. No Load Outdoor Air Flow Rate is set to 0 and simulation continues.");
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow = 0;
            }
        }
    } // IF(ZoneEquipmentListNotChecked)THEN

    // Size TU
    if (state.dataHVACVarRefFlow->MySizeFlag(VRFTUNum)) {
        if (!state.dataGlobal->ZoneSizingCalc && !state.dataGlobal->SysSizingCalc) {
            SizeVRF(state, VRFTUNum);
            state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).TerminalUnitNotSizedYet(IndexToTUInTUList) = false;
            state.dataHVACVarRefFlow->MySizeFlag(VRFTUNum) = false;
        } // IF ( .NOT. ZoneSizingCalc) THEN
    } // IF (MySizeFlag(VRFTUNum)) THEN

    // Do the Begin Environment initializations
    if (state.dataGlobal->BeginEnvrnFlag && state.dataHVACVarRefFlow->MyEnvrnFlag(VRFTUNum)) {

        // Change the Volume Flow Rates to Mass Flow Rates

        RhoAir = state.dataEnvrn->StdRhoAir;
        // set the mass flow rates from the input volume flow rates
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow = RhoAir * state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow;
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirMassFlow = RhoAir * state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow;
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow = RhoAir * state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow;
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirMassFlow = RhoAir * state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow;
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirMassFlow = RhoAir * state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirVolFlow;
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirMassFlow = RhoAir * state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirVolFlow;
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow =
            RhoAir * state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow;
        // set the node max and min mass flow rates
        // outside air mixer is optional, check that node num > 0
        if (OutsideAirNode > 0) {
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMax =
                max(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirMassFlow, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirMassFlow);
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMin = 0.0;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMinAvail = 0.0;
        }
        state.dataLoopNodes->Node(OutNode).MassFlowRateMax =
            max(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow);
        state.dataLoopNodes->Node(OutNode).MassFlowRateMin = 0.0;
        state.dataLoopNodes->Node(OutNode).MassFlowRateMinAvail = 0.0;
        state.dataLoopNodes->Node(InNode).MassFlowRateMax =
            max(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow);
        state.dataLoopNodes->Node(InNode).MassFlowRateMin = 0.0;
        state.dataLoopNodes->Node(InNode).MassFlowRateMinAvail = 0.0;
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerRelNodeNum > 0) {
            state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerRelNodeNum).MassFlowRateMinAvail = 0.0;
        }

        state.dataHVACVarRefFlow->MyEnvrnFlag(VRFTUNum) = false;

        if (state.dataHVACVarRefFlow->VRF(VRFCond).CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
            rho = state.dataPlnt->PlantLoop(state.dataHVACVarRefFlow->VRF(VRFCond).SourcePlantLoc.loopNum)
                      .glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
            state.dataHVACVarRefFlow->VRF(VRFCond).WaterCondenserDesignMassFlow = state.dataHVACVarRefFlow->VRF(VRFCond).WaterCondVolFlowRate * rho;

            InitComponentNodes(state,
                               0.0,
                               state.dataHVACVarRefFlow->VRF(VRFCond).WaterCondenserDesignMassFlow,
                               state.dataHVACVarRefFlow->VRF(VRFCond).CondenserNodeNum,
                               state.dataHVACVarRefFlow->VRF(VRFCond).CondenserOutletNodeNum);
        }
        //    IF(MyVRFCondFlag(VRFCond))THEN
        state.dataHVACVarRefFlow->VRF(VRFCond).HRTimer = 0.0;
        state.dataHVACVarRefFlow->VRF(VRFCond).ModeChange = false;
        state.dataHVACVarRefFlow->VRF(VRFCond).HRModeChange = false;
        state.dataHVACVarRefFlow->MyVRFCondFlag(VRFCond) = false;
        //    END IF

        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode > 0) {
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilType_Num == HVAC::Coil_HeatingWater) {
                if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow == DataSizing::AutoSize) {
                    WaterCoils::SimulateWaterCoilComponents(state,
                                                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilName,
                                                            FirstHVACIteration,
                                                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilIndex);
                    // design hot water volume flow rate
                    Real64 CoilMaxVolFlowRate = WaterCoils::GetCoilMaxWaterFlowRate(
                        state, "Coil:Heating:Water", state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilName, ErrorsFound);
                    if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                        rho = state.dataPlnt->PlantLoop(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilPlantLoc.loopNum)
                                  .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow = CoilMaxVolFlowRate * rho;
                    }
                }
            }

            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilType_Num == HVAC::Coil_HeatingSteam) {
                if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow == DataSizing::AutoSize) {
                    SteamCoils::SimulateSteamCoilComponents(state,
                                                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilName,
                                                            FirstHVACIteration,
                                                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilIndex,
                                                            1.0);
                    // design steam volume flow rate
                    Real64 CoilMaxVolFlowRate =
                        SteamCoils::GetCoilMaxSteamFlowRate(state, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilIndex, ErrorsFound);
                    if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                        Real64 TempSteamIn = 100.0;
                        Real64 SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, TempSteamIn, 1.0, RoutineName);
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow = CoilMaxVolFlowRate * SteamDensity;
                    }
                }
            }
            // init water/steam coils min and max flow rates
            InitComponentNodes(state,
                               0.0,
                               state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow,
                               state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode,
                               state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidOutletNode);
        }

        // the first time an air loop VRF TU is simulated set isSimulated = true so that the TU initialization
        // will occur with the first TU simulated this time step. Zone VRF TUs are called during sizing which, if air
        // loop TUs are included, alters when all TUs appear to have been simulated. Also, BeginEnvrnFlag is true multiple
        // times during the simulation, reset each time to avoid a different order during sizing and simulation
        if (state.dataHVACVarRefFlow->TerminalUnitList(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TUListIndex).reset_isSimulatedFlags) {
            // if no TUs are in the air loop or outdoor air system they will all be simulated during ManageZoneEquipment
            // and there is no need to adjust the order of simulation (i.e., when isSimulated are all true for a given system)
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInAirLoop || state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInOASys) {
                state.dataHVACVarRefFlow->TerminalUnitList(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TUListIndex).IsSimulated = true;
                state.dataHVACVarRefFlow->TerminalUnitList(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TUListIndex).reset_isSimulatedFlags = false;
            }
        }

    } // IF (BeginEnvrnFlag .and. MyEnvrnFlag(VRFTUNum)) THEN

    // reset environment flag for next environment
    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataHVACVarRefFlow->MyEnvrnFlag(VRFTUNum) = true;
        state.dataHVACVarRefFlow->MyVRFCondFlag(VRFCond) = true;
        state.dataHVACVarRefFlow->TerminalUnitList(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TUListIndex).reset_isSimulatedFlags = true;
    }

    // If all VRF Terminal Units on this VRF AC System have been simulated, reset the IsSimulated flag
    // The condenser will be simulated after all terminal units have been simulated (see Sub SimulateVRF)
    if (all(state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).IsSimulated)) {
        //   this should be the first time through on the next iteration. All TU's and condenser have been simulated.
        //   reset simulation flag for each terminal unit
        state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).IsSimulated = false;
        //     after all TU's have been simulated, reset operating mode flag if necessary
        if (state.dataHVACVarRefFlow->LastModeHeating(VRFCond) && state.dataHVACVarRefFlow->CoolingLoad(VRFCond)) {
            state.dataHVACVarRefFlow->LastModeCooling(VRFCond) = true;
            state.dataHVACVarRefFlow->LastModeHeating(VRFCond) = false;
            //        SwitchedMode(VRFCond)    = .TRUE.
        }
        if (state.dataHVACVarRefFlow->LastModeCooling(VRFCond) && state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) {
            state.dataHVACVarRefFlow->LastModeHeating(VRFCond) = true;
            state.dataHVACVarRefFlow->LastModeCooling(VRFCond) = false;
            //        SwitchedMode(VRFCond)    = .TRUE.
        }
    } // IF(ALL(TerminalUnitList(VRFTU(VRFTUNum)%TUListIndex)%IsSimulated))THEN

    // get operating capacity of water and steam coil
    if (FirstHVACIteration) {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode > 0) {
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilType_Num == HVAC::Coil_HeatingWater) {
                //     set hot water full flow rate for sizing
                Real64 mdot = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow;
                PlantUtilities::SetComponentFlowRate(state,
                                                     mdot,
                                                     state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode,
                                                     state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidOutletNode,
                                                     state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilPlantLoc);

                //     simulate water coil to find operating capacity
                WaterCoils::SimulateWaterCoilComponents(state,
                                                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilName,
                                                        FirstHVACIteration,
                                                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilIndex,
                                                        SuppHeatCoilCapacity);
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSuppHeatingCapacity = SuppHeatCoilCapacity;
            } // from iF VRFTU(VRFTUNum).SuppHeatCoilType_Num == HVAC::Coil_HeatingWater
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilType_Num == HVAC::Coil_HeatingSteam) {
                //     set hot water full flow rate for sizing
                Real64 mdot = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow;
                PlantUtilities::SetComponentFlowRate(state,
                                                     mdot,
                                                     state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode,
                                                     state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidOutletNode,
                                                     state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilPlantLoc);

                //     simulate steam coil to find operating capacity
                SteamCoils::SimulateSteamCoilComponents(state,
                                                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilName,
                                                        FirstHVACIteration,
                                                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilIndex,
                                                        1.0,
                                                        ErrorsFound); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                SuppHeatCoilCapacity =
                    SteamCoils::GetCoilCapacity(state, "Coil:Heating:Steam", state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilName, ErrorsFound);

                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSuppHeatingCapacity = SuppHeatCoilCapacity;
            } // from if VRFTU( VRFTUNum ).SuppHeatCoilType_Num == HVAC::Coil_HeatingSteam
        }
    }
    // initialize water/steam coil inlet flow rate to zero
    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode > 0) {
        Real64 mdot = 0.0;
        PlantUtilities::SetComponentFlowRate(state,
                                             mdot,
                                             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode,
                                             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidOutletNode,
                                             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilPlantLoc);
    }

    // one-time checks of flow rate vs fan flow rate
    if (state.dataHVACVarRefFlow->MyVRFFlag(VRFTUNum)) {
        if (!state.dataGlobal->ZoneSizingCalc && !state.dataGlobal->SysSizingCalc) {
            auto &vrfTU = state.dataHVACVarRefFlow->VRFTU(VRFTUNum);
            if (vrfTU.fanPlace != HVAC::FanPlace::Invalid) { // was > 0 (is 0 invalid?)
                if (vrfTU.ActualFanVolFlowRate != AutoSize) {

                    if (vrfTU.fanType == HVAC::FanType::SystemModel) {
                        auto *fanSystem = dynamic_cast<Fans::FanSystem *>(state.dataFans->fans(vrfTU.FanIndex));
                        if (fanSystem->speedControl == Fans::SpeedControl::Discrete &&
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex < 0) {
                            if (fanSystem->numSpeeds > 1) {
                                if (vrfTU.DXCoolCoilType_Num == HVAC::CoilVRF_Cooling) {
                                    if (vrfTU.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                                        for (int i = 1; i <= vrfTU.NumOfSpeedCooling; ++i) {
                                            vrfTU.CoolMassFlowRate[i] = fanSystem->massFlowAtSpeed[i - 1];
                                        }
                                    }
                                }
                                if (vrfTU.DXHeatCoilType_Num == HVAC::CoilVRF_Heating) {
                                    if (vrfTU.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                                        for (int i = 1; i <= vrfTU.NumOfSpeedCooling; ++i) {
                                            vrfTU.HeatMassFlowRate[i] = fanSystem->massFlowAtSpeed[i - 1];
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow >
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate) {
                        ShowWarningError(state,
                                         format("InitVRF: VRF Terminal Unit = [{}, \"{}\"]",
                                                tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                        ShowContinueError(state, "... has Supply Air Flow Rate During Cooling Operation > Max Fan Volume Flow Rate, should be <=");
                        ShowContinueError(state,
                                          format("... Supply Air Flow Rate During Cooling Operation = {:.4R} m3/s",
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow));
                        ShowContinueError(state,
                                          format("... Max Fan Volume Flow Rate                      = {:.4R} m3/s",
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate));
                        ShowContinueError(
                            state, "...the supply air flow rate during cooling operation will be reduced to match and the simulation continues.");
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate;
                    }

                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirVolFlow >
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate) {
                        ShowWarningError(state,
                                         format("InitVRF: VRF Terminal Unit = [{}, \"{}\"]",
                                                tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                        ShowContinueError(state, "... has Supply Air Flow Rate When No Cooling is Needed > Max Fan Volume Flow Rate, should be <=");
                        ShowContinueError(state,
                                          format("... Supply Air Flow Rate When No Cooling is Needed = {:.4R} m3/s",
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirVolFlow));
                        ShowContinueError(state,
                                          format("... Max Fan Volume Flow Rate                       = {:.4R} m3/s",
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate));
                        ShowContinueError(
                            state, "...the supply air flow rate when no cooling is needed will be reduced to match and the simulation continues.");
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirVolFlow =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate;
                    }

                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow > state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow) {
                        ShowWarningError(state,
                                         format("InitVRF: VRF Terminal Unit = [{}, \"{}\"]",
                                                tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                        ShowContinueError(
                            state,
                            "...The Outdoor Air Flow Rate During Cooling Operation exceeds the Supply Air Flow Rate During Cooling Operation.");
                        ShowContinueError(state,
                                          format("...Outdoor Air Flow Rate During Cooling Operation = {:.4R} m3/s",
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow));
                        ShowContinueError(state,
                                          format("... Supply Air Flow Rate During Cooling Operation = {:.4R} m3/s",
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow));
                        ShowContinueError(state, "...the outdoor air flow rate will be reduced to match and the simulation continues.");
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow;
                    }

                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow >
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate) {
                        ShowWarningError(state,
                                         format("InitVRF: VRF Terminal Unit = [{}, \"{}\"]",
                                                tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                        ShowContinueError(state, "... has Supply Air Flow Rate During Heating Operation > Max Fan Volume Flow Rate, should be <=");
                        ShowContinueError(state,
                                          format("... Supply Air Flow Rate During Heating Operation = {:.4R} m3/s",
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow));
                        ShowContinueError(state,
                                          format("... Max Fan Volume Flow Rate                      = {:.4R} m3/s",
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate));
                        ShowContinueError(
                            state, "...the supply air flow rate during cooling operation will be reduced to match and the simulation continues.");
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate;
                    }

                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirVolFlow >
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate) {
                        ShowWarningError(state,
                                         format("InitVRF: VRF Terminal Unit = [{}, \"{}\"]",
                                                tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                        ShowContinueError(state, "... has Supply Air Flow Rate When No Heating is Needed > Max Fan Volume Flow Rate, should be <=");
                        ShowContinueError(state,
                                          format("... Supply Air Flow Rate When No Heating is Needed = {:.4R} m3/s",
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirVolFlow));
                        ShowContinueError(state,
                                          format("... Max Fan Volume Flow Rate                       = {:.4R} m3/s",
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate));
                        ShowContinueError(
                            state, "...the supply air flow rate when no cooling is needed will be reduced to match and the simulation continues.");
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirVolFlow =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate;
                    }

                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow > state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow) {
                        ShowWarningError(state,
                                         format("InitVRF: VRF Terminal Unit = [{}, \"{}\"]",
                                                tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                        ShowContinueError(
                            state,
                            "...The Outdoor Air Flow Rate During Heating Operation exceeds the Supply Air Flow Rate During Heating Operation.");
                        ShowContinueError(state,
                                          format("...Outdoor Air Flow Rate During Heating Operation = {:.4R} m3/s",
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow));
                        ShowContinueError(state,
                                          format("... Supply Air Flow Rate During Heating Operation = {:.4R} m3/s",
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow));
                        ShowContinueError(state, "...the outdoor air flow rate will be reduced to match and the simulation continues.");
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow;
                    }

                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow >
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate) {
                        ShowWarningError(state,
                                         format("InitVRF: VRF Terminal Unit = [{}, \"{}\"]",
                                                tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                        ShowContinueError(
                            state, "... has a Outdoor Air Flow Rate When No Cooling or Heating is Needed > Max Fan Volume Flow Rate, should be <=");
                        ShowContinueError(state,
                                          format("... Outdoor Air Flow Rate When No Cooling or Heating is Needed = {:.4R} m3/s",
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow));
                        ShowContinueError(state,
                                          format("... Max Fan Volume Flow Rate                                   = {:.4R} m3/s",
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate));
                        ShowContinueError(state,
                                          "...the outdoor air flow rate when no cooling or heating is needed will be reduced to match and the "
                                          "simulation continues.");
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate;
                    }

                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate > 0.0) {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatingSpeedRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow /
                                                                                      state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate;
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolingSpeedRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow /
                                                                                      state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate;
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoHeatingSpeedRatio =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirVolFlow /
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate;
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolingSpeedRatio =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirVolFlow /
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate;
                    }

                    state.dataHVACVarRefFlow->MyVRFFlag(VRFTUNum) = false;
                } else {
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ActualFanVolFlowRate =
                        state.dataFans->fans(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).FanIndex)->maxAirFlowRate;
                }
            } else {
                state.dataHVACVarRefFlow->MyVRFFlag(VRFTUNum) = false;
            }
        }
    } // IF(MyVRFFlag(VRFTUNum))THEN

    // calculate end time of current time step to determine if max capacity reset is required
    CurrentEndTime = double((state.dataGlobal->DayOfSim - 1) * 24) + state.dataGlobal->CurrentTime - state.dataGlobal->TimeStepZone +
                     state.dataHVACGlobal->SysTimeElapsed;

    // Initialize the maximum allowed terminal unit capacity. Total terminal unit capacity must not
    // exceed the available condenser capacity. This variable is used to limit the terminal units
    // providing more capacity than allowed. Example: TU loads are 1-ton, 2-ton, 3-ton, and 4-ton connected
    // to a condenser having only 9-tons available. This variable will be set to 3-tons and the 4-ton
    // terminal unit will be limited to 3-tons (see SimVRFCondenser where this variable is calculated).
    if (CurrentEndTime > state.dataHVACVarRefFlow->CurrentEndTimeLast || TimeStepSysLast > state.dataHVACGlobal->TimeStepSys ||
        (FirstHVACIteration && state.dataHVACVarRefFlow->MyBeginTimeStepFlag(VRFCond))) {
        state.dataHVACVarRefFlow->MaxCoolingCapacity(VRFCond) = Constant::MaxCap;
        state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond) = Constant::MaxCap;
        state.dataHVACVarRefFlow->MyBeginTimeStepFlag(VRFCond) = false;
    }

    if (!FirstHVACIteration) {
        state.dataHVACVarRefFlow->MyBeginTimeStepFlag(VRFCond) = true;
    }

    // Do the following initializations (every time step).

    TimeStepSysLast = state.dataHVACGlobal->TimeStepSys;
    state.dataHVACVarRefFlow->CurrentEndTimeLast = CurrentEndTime;

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOpModeSched != nullptr) {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOpModeSched->getCurrentVal() == 0.0) {
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOp = HVAC::FanOp::Cycling;
        } else {
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOp = HVAC::FanOp::Continuous;
        }
    }

    // if condenser is off, all terminal unit coils are off
    if (state.dataHVACVarRefFlow->VRF(VRFCond).availSched->getCurrentVal() == 0.0) {
        state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
        state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
    } else {

        //*** Operating Mode Initialization done at beginning of each iteration ***!
        //*** assumes all TU's and Condenser were simulated last iteration ***!
        //*** this code is done ONCE each iteration when all TU's IsSimulated flag is FALSE ***!
        // Determine operating mode prior to simulating any terminal units connected to a VRF condenser
        // this should happen at the beginning of a time step where all TU's are polled to see what
        // mode the heat pump condenser will operate in
        if (!any(state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).IsSimulated)) {
            InitializeOperatingMode(state, FirstHVACIteration, VRFCond, TUListIndex, OnOffAirFlowRatio);
        }
        //*** End of Operating Mode Initialization done at beginning of each iteration ***!

        // disable VRF system when outside limits of operation based on OAT
        EnableSystem = false; // flag used to switch operating modes when OAT is outside operating limits
        if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond)) {
            if ((OutsideDryBulbTemp < state.dataHVACVarRefFlow->VRF(VRFCond).MinOATCooling ||
                 OutsideDryBulbTemp > state.dataHVACVarRefFlow->VRF(VRFCond).MaxOATCooling) &&
                any(state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).CoolingCoilPresent)) {
                state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
                // test if heating load exists, account for thermostat control type
                switch (state.dataHVACVarRefFlow->VRF(VRFCond).ThermostatPriority) {
                case ThermostatCtrlType::LoadPriority:
                case ThermostatCtrlType::ZonePriority: {
                    if (state.dataHVACVarRefFlow->SumHeatingLoads(VRFCond) > 0.0) {
                        EnableSystem = true;
                    }
                } break;
                case ThermostatCtrlType::ThermostatOffsetPriority: {
                    if (state.dataHVACVarRefFlow->MinDeltaT(VRFCond) < 0.0) {
                        EnableSystem = true;
                    }
                } break;
                case ThermostatCtrlType::ScheduledPriority:
                case ThermostatCtrlType::MasterThermostatPriority: {
                    // can't switch modes if scheduled (i.e., would be switching to unscheduled mode)
                    // or master TSTAT used (i.e., master zone only has a specific load - can't switch)
                } break;
                default:
                    break;
                }
                if (EnableSystem) {
                    if ((OutsideDryBulbTemp >= state.dataHVACVarRefFlow->VRF(VRFCond).MinOATHeating &&
                         OutsideDryBulbTemp <= state.dataHVACVarRefFlow->VRF(VRFCond).MaxOATHeating) &&
                        any(state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HeatingCoilPresent)) {
                        state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = true;
                    } else {
                        if (any(state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).CoolingCoilAvailable)) {
                            if (state.dataHVACVarRefFlow->VRF(VRFCond).CoolingMaxTempLimitIndex == 0) {
                                ShowWarningMessage(state,
                                                   format("{} \"{}\".",
                                                          cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum),
                                                          state.dataHVACVarRefFlow->VRF(VRFCond).Name));
                                ShowContinueError(state,
                                                  "...InitVRF: VRF Heat Pump Min/Max Operating Temperature in Cooling Mode Limits have been "
                                                  "exceeded and VRF system is disabled.");
                                if (state.dataHVACVarRefFlow->VRF(VRFCond).CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
                                    ShowContinueError(state,
                                                      format("... Outdoor Unit Inlet Water Temperature           = {:.3T}", OutsideDryBulbTemp));
                                } else {
                                    ShowContinueError(state,
                                                      format("... Outdoor Unit Inlet Air Temperature                 = {:.3T}", OutsideDryBulbTemp));
                                }
                                ShowContinueError(state,
                                                  format("... Cooling Minimum Outdoor Unit Inlet Temperature = {:.3T}",
                                                         state.dataHVACVarRefFlow->VRF(VRFCond).MinOATCooling));
                                ShowContinueError(state,
                                                  format("... Cooling Maximum Outdoor Unit Inlet Temperature = {:.3T}",
                                                         state.dataHVACVarRefFlow->VRF(VRFCond).MaxOATCooling));
                                ShowContinueErrorTimeStamp(state, "... Check VRF Heat Pump Min/Max Outdoor Temperature in Cooling Mode limits.");
                            }
                            ShowRecurringWarningErrorAtEnd(state,
                                                           std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)) + " \"" +
                                                               state.dataHVACVarRefFlow->VRF(VRFCond).Name +
                                                               "\" -- Exceeded VRF Heat Pump min/max cooling temperature limit error continues...",
                                                           state.dataHVACVarRefFlow->VRF(VRFCond).CoolingMaxTempLimitIndex,
                                                           OutsideDryBulbTemp,
                                                           OutsideDryBulbTemp);
                        }
                    }
                } else {
                    if (any(state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).CoolingCoilAvailable)) {
                        if (state.dataHVACVarRefFlow->VRF(VRFCond).CoolingMaxTempLimitIndex == 0) {
                            ShowWarningMessage(state,
                                               format("{} \"{}\".",
                                                      cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum),
                                                      state.dataHVACVarRefFlow->VRF(VRFCond).Name));
                            ShowContinueError(state,
                                              "...InitVRF: VRF Heat Pump Min/Max Operating Temperature in Cooling Mode Limits have been exceeded "
                                              "and VRF system is disabled.");
                            if (state.dataHVACVarRefFlow->VRF(VRFCond).CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
                                ShowContinueError(state, format("... Outdoor Unit Inlet Water Temperature           = {:.3T}", OutsideDryBulbTemp));
                            } else {
                                ShowContinueError(state,
                                                  format("... Outdoor Unit Inlet Air Temperature                 = {:.3T}", OutsideDryBulbTemp));
                            }
                            ShowContinueError(state,
                                              format("... Cooling Minimum Outdoor Unit Inlet Temperature = {:.3T}",
                                                     state.dataHVACVarRefFlow->VRF(VRFCond).MinOATCooling));
                            ShowContinueError(state,
                                              format("... Cooling Maximum Outdoor Unit Inlet Temperature = {:.3T}",
                                                     state.dataHVACVarRefFlow->VRF(VRFCond).MaxOATCooling));
                            ShowContinueErrorTimeStamp(state, "... Check VRF Heat Pump Min/Max Outdoor Temperature in Cooling Mode limits.");
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)) + " \"" +
                                                           state.dataHVACVarRefFlow->VRF(VRFCond).Name +
                                                           "\" -- Exceeded VRF Heat Pump min/max cooling temperature limit error continues...",
                                                       state.dataHVACVarRefFlow->VRF(VRFCond).CoolingMaxTempLimitIndex,
                                                       OutsideDryBulbTemp,
                                                       OutsideDryBulbTemp);
                    }
                }
            }
        } else if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) {
            if ((OutsideDryBulbTemp < state.dataHVACVarRefFlow->VRF(VRFCond).MinOATHeating ||
                 OutsideDryBulbTemp > state.dataHVACVarRefFlow->VRF(VRFCond).MaxOATHeating) &&
                any(state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HeatingCoilPresent)) {
                state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
                // test if cooling load exists, account for thermostat control type
                switch (state.dataHVACVarRefFlow->VRF(VRFCond).ThermostatPriority) {
                case ThermostatCtrlType::LoadPriority:
                case ThermostatCtrlType::ZonePriority: {
                    if (state.dataHVACVarRefFlow->SumCoolingLoads(VRFCond) < 0.0) {
                        EnableSystem = true;
                    }
                } break;
                case ThermostatCtrlType::ThermostatOffsetPriority: {
                    if (state.dataHVACVarRefFlow->MaxDeltaT(VRFCond) > 0.0) {
                        EnableSystem = true;
                    }
                } break;
                case ThermostatCtrlType::ScheduledPriority:
                case ThermostatCtrlType::MasterThermostatPriority: {
                } break;
                default:
                    break;
                }
                if (EnableSystem) {
                    if ((OutsideDryBulbTemp >= state.dataHVACVarRefFlow->VRF(VRFCond).MinOATCooling &&
                         OutsideDryBulbTemp <= state.dataHVACVarRefFlow->VRF(VRFCond).MaxOATCooling) &&
                        any(state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).CoolingCoilPresent)) {
                        state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = true;
                    } else {
                        if (any(state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HeatingCoilAvailable)) {
                            if (state.dataHVACVarRefFlow->VRF(VRFCond).HeatingMaxTempLimitIndex == 0) {
                                ShowWarningMessage(state,
                                                   format("{} \"{}\".",
                                                          cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum),
                                                          state.dataHVACVarRefFlow->VRF(VRFCond).Name));
                                ShowContinueError(state,
                                                  "...InitVRF: VRF Heat Pump Min/Max Operating Temperature in Heating Mode Limits have been "
                                                  "exceeded and VRF system is disabled.");
                                if (state.dataHVACVarRefFlow->VRF(VRFCond).CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
                                    ShowContinueError(state,
                                                      format("... Outdoor Unit Inlet Water Temperature           = {:.3T}", OutsideDryBulbTemp));
                                } else {
                                    ShowContinueError(state,
                                                      format("... Outdoor Unit Inlet Air Temperature             = {:.3T}", OutsideDryBulbTemp));
                                }
                                ShowContinueError(state,
                                                  format("... Heating Minimum Outdoor Unit Inlet Temperature = {:.3T}",
                                                         state.dataHVACVarRefFlow->VRF(VRFCond).MinOATHeating));
                                ShowContinueError(state,
                                                  format("... Heating Maximum Outdoor Unit Inlet Temperature = {:.3T}",
                                                         state.dataHVACVarRefFlow->VRF(VRFCond).MaxOATHeating));
                                ShowContinueErrorTimeStamp(state, "... Check VRF Heat Pump Min/Max Outdoor Temperature in Heating Mode limits.");
                            }
                            ShowRecurringWarningErrorAtEnd(state,
                                                           std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)) + " \"" +
                                                               state.dataHVACVarRefFlow->VRF(VRFCond).Name +
                                                               "\" -- Exceeded VRF Heat Pump min/max heating temperature limit error continues...",
                                                           state.dataHVACVarRefFlow->VRF(VRFCond).HeatingMaxTempLimitIndex,
                                                           OutsideDryBulbTemp,
                                                           OutsideDryBulbTemp);
                        }
                    }
                } else {
                    if (any(state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HeatingCoilAvailable)) {
                        if (state.dataHVACVarRefFlow->VRF(VRFCond).HeatingMaxTempLimitIndex == 0) {
                            ShowWarningMessage(state,
                                               format("{} \"{}\".",
                                                      cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum),
                                                      state.dataHVACVarRefFlow->VRF(VRFCond).Name));
                            ShowContinueError(state,
                                              "...InitVRF: VRF Heat Pump Min/Max Operating Temperature in Heating Mode Limits have been exceeded "
                                              "and VRF system is disabled.");
                            if (state.dataHVACVarRefFlow->VRF(VRFCond).CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
                                ShowContinueError(state, format("... Outdoor Unit Inlet Water Temperature           = {:.3T}", OutsideDryBulbTemp));
                            } else {
                                ShowContinueError(state, format("... Outdoor Unit Inlet Air Temperature             = {:.3T}", OutsideDryBulbTemp));
                            }
                            ShowContinueError(state,
                                              format("... Heating Minimum Outdoor Unit Inlet Temperature = {:.3T}",
                                                     state.dataHVACVarRefFlow->VRF(VRFCond).MinOATHeating));
                            ShowContinueError(state,
                                              format("... Heating Maximum Outdoor Unit Inlet Temperature = {:.3T}",
                                                     state.dataHVACVarRefFlow->VRF(VRFCond).MaxOATHeating));
                            ShowContinueErrorTimeStamp(state, "... Check VRF Heat Pump Min/Max Outdoor Temperature in Heating Mode limits.");
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)) + " \"" +
                                                           state.dataHVACVarRefFlow->VRF(VRFCond).Name +
                                                           "\" -- Exceeded VRF Heat Pump min/max heating temperature limit error continues...",
                                                       state.dataHVACVarRefFlow->VRF(VRFCond).HeatingMaxTempLimitIndex,
                                                       OutsideDryBulbTemp,
                                                       OutsideDryBulbTemp);
                    }
                }
            }
        }

    } // IF (GetCurrentScheduleValue(state, VRF(VRFCond)%SchedPtr) .EQ. 0.0) THEN

    // initialize terminal unit flow rate
    if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond) ||
        (state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed &&
         state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList))) {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerUsed) {
            state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirMassFlow;
        } else {
            if (!state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInOASys) {
                state.dataLoopNodes->Node(InNode).MassFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow;
            }
        }
    } else if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond) ||
               (state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed &&
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList))) {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerUsed) {
            state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirMassFlow;
        } else {
            if (!state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInOASys) {
                state.dataLoopNodes->Node(InNode).MassFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow;
            }
        }
    } else {
        if (state.dataHVACVarRefFlow->LastModeCooling(VRFCond)) {
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerUsed) {
                state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate =
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
                state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            } else {
                if (!state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInOASys) {
                    state.dataLoopNodes->Node(InNode).MassFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
                }
            }
        } else if (state.dataHVACVarRefFlow->LastModeHeating(VRFCond)) {
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerUsed) {
                state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate =
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
                state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            } else {
                if (!state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInOASys) {
                    state.dataLoopNodes->Node(InNode).MassFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
                }
            }
        }
    }

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerExists) {
        // There is an air terminal mixer
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerType == HVAC::MixerType::InletSide) { // if there is an inlet side air terminal mixer
            // set the primary air inlet mass flow rate
            state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).MassFlowRate =
                min(state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).MassFlowRateMaxAvail,
                    state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate);
            // now calculate the the mixer outlet air conditions (and the secondary air inlet flow rate). The mixer outlet flow rate has already
            // been set above (it is the "inlet" node flow rate)
            SimATMixer(state,
                       state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerName,
                       FirstHVACIteration,
                       state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerIndex);
        }
    } else {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerUsed) {
            MixedAir::SimOAMixer(
                state, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerName, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerIndex);
        }
    }
    OnOffAirFlowRatio = 1.0;

    // these flags are used in Subroutine CalcVRF to turn on the correct coil (heating or cooling)
    // valid operating modes
    // Heat Pump (heat recovery flags are set to FALSE):
    // CoolingLoad(VRFCond) - TU can only operate in this mode if heat recovery is not used and there is a cooling load
    // HeatingLoad(VRFCond) - TU can only operate in this mode if heat recovery is not used and there is a heating load
    // Heat Recovery (heat pump flags are set same as for Heat Pump operation):
    // TerminalUnitList(TUListIndex)%HRCoolRequest(IndexToTUInTUList) - TU will operate in this mode if heat recovery is used
    // TerminalUnitList(TUListIndex)%HRHeatRequest(IndexToTUInTUList) - TU will operate in this mode if heat recovery is used

    getVRFTUZoneLoad(state, VRFTUNum, QZnReq, LoadToHeatingSP, LoadToCoolingSP, false);

    if (std::abs(QZnReq) < HVAC::SmallLoad) {
        QZnReq = 0.0;
    }
    // set initial terminal unit operating mode for heat recovery
    // operating mode for non-heat recovery set above using CoolingLoad(VRFCond) or HeatingLoad(VRFCond) variables
    // first turn off terminal unit
    state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
    state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
    // then set according to LoadToXXXXingSP variables
    if (LoadToCoolingSP < -1.0 * HVAC::SmallLoad) {
        if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond) ||
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) { // don't allow heat recovery if control logic dictates unit is off
            state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = true;
        }
    }
    if (LoadToHeatingSP > HVAC::SmallLoad) {
        if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond) ||
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) { // don't allow heat recovery if control logic dictates unit is off
            state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
        }
    }
    if (LoadToCoolingSP > 0.0 && LoadToHeatingSP < 0.0) {
        QZnReq = 0.0;
    }

    // next check for overshoot when constant fan mode is used
    // check operating load to see if OA will overshoot setpoint temperature when constant fan mode is used
    if ((state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOp == HVAC::FanOp::Continuous || state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerExists) &&
        !state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isSetPointControlled) {
        SetCompFlowRate(state, VRFTUNum, VRFCond, true);

        if (state.dataHVACVarRefFlow->VRF(VRFCond).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
        } else {
            // Algorithm Type: VRF model based on system curve
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF(
                state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
        }

        // If the Terminal Unit has a net cooling capacity (TempOutput < 0) and
        // the zone temp is above the Tstat heating setpoint (QToHeatSetPt < 0)
        // see if the terminal unit operation will exceed the setpoint
        // 4 tests here to cover all possibilities:
        // IF(TempOutput < 0.0d0 .AND. LoadToHeatingSP .LT. 0.0d0)THEN
        // ELSE IF(TempOutput .GT. 0.0d0 .AND. LoadToCoolingSP .GT. 0.0d0)THEN
        // ELSE IF(TempOutput .GT. 0.0d0 .AND. LoadToCoolingSP .LT. 0.0d0)THEN
        // ELSE IF(TempOutput < 0.0d0 .AND. LoadToHeatingSP .GT. 0.0d0)THEN
        // END IF
        // could compress these to 2 complex IF's but logic inside each would get more complex
        if (TempOutput < 0.0 && LoadToHeatingSP < 0.0) {
            // If the net cooling capacity overshoots the heating setpoint count as heating load
            if (TempOutput < LoadToHeatingSP) {
                // Don't count as heating load unless mode is allowed. Also check for floating zone.
                if (state.dataHeatBalFanSys->TempControlType(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum) != HVAC::SetptType::SingleCool &&
                    state.dataHeatBalFanSys->TempControlType(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum) != HVAC::SetptType::Uncontrolled) {
                    if (!state.dataHVACVarRefFlow->LastModeHeating(VRFCond)) {
                        // system last operated in cooling mode, change air flows and repeat coil off capacity test
                        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerUsed) {
                            state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate =
                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                            state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum).MassFlowRate =
                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirMassFlow;
                            MixedAir::SimOAMixer(
                                state, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerName, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerIndex);
                        } else {
                            state.dataLoopNodes->Node(InNode).MassFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                        }

                        if (state.dataHVACVarRefFlow->VRF(VRFCond).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                                state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                        } else {
                            // Algorithm Type: VRF model based on system curve
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF(
                                state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                        }

                        // if zone temp will overshoot, pass the LoadToHeatingSP as the load to meet
                        if (TempOutput < LoadToHeatingSP) {
                            QZnReq = LoadToHeatingSP;
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                        }
                    } else {
                        // last mode was heating, zone temp will overshoot heating setpoint, reset QznReq to LoadtoHeatingSP
                        QZnReq = LoadToHeatingSP;
                        state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
                        state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                    }
                }
            } else if (TempOutput > LoadToCoolingSP && LoadToCoolingSP < 0.0) {
                //       If the net cooling capacity does not meet the zone cooling load enable cooling
                QZnReq = LoadToCoolingSP;
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = true;
            } else if (TempOutput < LoadToCoolingSP && LoadToCoolingSP < 0.0) {
                //       If the net cooling capacity meets the zone cooling load but does not overshoot heating setpoint
                QZnReq = 0.0;
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
            }
            //     If the terminal unit has a net heating capacity and the zone temp is below the Tstat cooling setpoint
            //     see if the terminal unit operation will exceed the setpoint
        } else if (TempOutput > 0.0 && LoadToCoolingSP > 0.0) {
            //       If the net heating capacity overshoots the cooling setpoint count as cooling load
            if (TempOutput > LoadToCoolingSP) {
                //         Don't count as cooling load unless mode is allowed. Also check for floating zone.
                if (state.dataHeatBalFanSys->TempControlType(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum) != HVAC::SetptType::SingleHeat &&
                    state.dataHeatBalFanSys->TempControlType(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum) != HVAC::SetptType::Uncontrolled) {
                    if (!state.dataHVACVarRefFlow->LastModeCooling(VRFCond)) {
                        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerUsed) {
                            state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate =
                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow;
                            state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum).MassFlowRate =
                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirMassFlow;
                            MixedAir::SimOAMixer(
                                state, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerName, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerIndex);
                        } else {
                            state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate =
                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow;
                        }

                        if (state.dataHVACVarRefFlow->VRF(VRFCond).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                                state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                        } else {
                            // Algorithm Type: VRF model based on system curve
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF(
                                state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                        }

                        // if zone temp will overshoot, pass the LoadToCoolingSP as the load to meet
                        if (TempOutput > LoadToCoolingSP) {
                            QZnReq = LoadToCoolingSP;
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = true;
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                        }
                    } else {
                        QZnReq = LoadToCoolingSP;
                        state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = true;
                        state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                    }
                }
            } else if (TempOutput < LoadToHeatingSP) {
                //         Don't count as heating load unless mode is allowed. Also check for floating zone.
                if (state.dataHeatBalFanSys->TempControlType(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum) != HVAC::SetptType::SingleCool &&
                    state.dataHeatBalFanSys->TempControlType(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum) != HVAC::SetptType::Uncontrolled) {
                    if (!state.dataHVACVarRefFlow->LastModeHeating(VRFCond)) {
                        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerUsed) {
                            state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate =
                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                            state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum).MassFlowRate =
                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirMassFlow;
                            MixedAir::SimOAMixer(
                                state, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerName, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerIndex);
                        } else {
                            state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate =
                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                        }

                        if (state.dataHVACVarRefFlow->VRF(VRFCond).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                                state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                        } else {
                            // Algorithm Type: VRF model based on system curve
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF(
                                state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                        }

                        // if zone temp will overshoot, pass the LoadToHeatingSP as the load to meet
                        if (TempOutput < LoadToHeatingSP) {
                            QZnReq = LoadToHeatingSP;
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                        }
                    } else {
                        QZnReq = LoadToHeatingSP;
                        state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
                        state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                    }
                }
            } else if (TempOutput > LoadToHeatingSP && TempOutput < LoadToCoolingSP) {
                //         If the net capacity does not overshoot either setpoint
                QZnReq = 0.0;
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
            } else {
                //         If the net heating capacity meets the zone heating load but does not overshoot cooling setpoint
                QZnReq = 0.0;
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
            }
            //     If the terminal unit has a net heating capacity and the zone temp is above the Tstat cooling setpoint
            //     see if the terminal unit operation will exceed the setpoint
        } else if (TempOutput > 0.0 && LoadToCoolingSP < 0.0) {
            //       If the net heating capacity overshoots the cooling setpoint count as cooling load
            //       Don't count as cooling load unless mode is allowed. Also check for floating zone.
            if (state.dataHeatBalFanSys->TempControlType(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum) != HVAC::SetptType::SingleHeat &&
                state.dataHeatBalFanSys->TempControlType(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum) != HVAC::SetptType::Uncontrolled) {
                if (!state.dataHVACVarRefFlow->LastModeCooling(VRFCond)) {
                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerUsed) {
                        state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow;
                        state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum).MassFlowRate =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirMassFlow;
                        MixedAir::SimOAMixer(
                            state, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerName, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerIndex);
                    } else {
                        state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow;
                    }

                    if (state.dataHVACVarRefFlow->VRF(VRFCond).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                            state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    } else {
                        // Algorithm Type: VRF model based on system curve
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF(
                            state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    }

                    // if zone temp will overshoot, pass the LoadToCoolingSP as the load to meet
                    if (TempOutput > LoadToCoolingSP) {
                        QZnReq = LoadToCoolingSP;
                        state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = true;
                        state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                    }
                    // last mode was cooling, zone temp will overshoot cooling setpoint, reset QznReq to LoadtoCoolingSP
                } else {
                    QZnReq = LoadToCoolingSP;
                    state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = true;
                    state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                }
            }
            // If the Terminal Unit has a net cooling capacity (TempOutput < 0) and
            // the zone temp is below the Tstat heating setpoint (QToHeatSetPt > 0)
            // see if the terminal unit operation will exceed the setpoint
        } else if (TempOutput < 0.0 && LoadToHeatingSP > 0.0) {
            // Don't count as heating load unless mode is allowed. Also check for floating zone.
            if (state.dataHeatBalFanSys->TempControlType(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum) != HVAC::SetptType::SingleCool &&
                state.dataHeatBalFanSys->TempControlType(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum) != HVAC::SetptType::Uncontrolled) {
                if (!state.dataHVACVarRefFlow->LastModeHeating(VRFCond)) {
                    // system last operated in cooling mode, change air flows and repeat coil off capacity test
                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerUsed) {
                        state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum).MassFlowRate =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                        state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum).MassFlowRate =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirMassFlow;
                        MixedAir::SimOAMixer(
                            state, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerName, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerIndex);
                    } else {
                        state.dataLoopNodes->Node(InNode).MassFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow;
                    }

                    if (state.dataHVACVarRefFlow->VRF(VRFCond).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                            state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    } else {
                        // Algorithm Type: VRF model based on system curve
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF(
                            state, VRFTUNum, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    }

                    // if zone temp will overshoot, pass the LoadToHeatingSP as the load to meet
                    if (TempOutput < LoadToHeatingSP) {
                        QZnReq = LoadToHeatingSP;
                        state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
                        state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                    }
                } else {
                    // last mode was heating, zone temp will overshoot heating setpoint, reset QznReq to LoadtoHeatingSP
                    QZnReq = LoadToHeatingSP;
                    state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
                    state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
                }
            }
        }
        // test that the system is active if constant fan logic enables system when thermostat control logic did not
        if (!state.dataHVACVarRefFlow->CoolingLoad(VRFCond) && !state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) {
            if (state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) &&
                OutsideDryBulbTemp >= state.dataHVACVarRefFlow->VRF(VRFCond).MinOATCooling &&
                OutsideDryBulbTemp <= state.dataHVACVarRefFlow->VRF(VRFCond).MaxOATCooling) {
                state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = true;
            } else if (state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) &&
                       OutsideDryBulbTemp >= state.dataHVACVarRefFlow->VRF(VRFCond).MinOATHeating &&
                       OutsideDryBulbTemp <= state.dataHVACVarRefFlow->VRF(VRFCond).MaxOATHeating) {
                state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = true;
            }
        }
    } // IF(VRFTU(VRFTUNum)%fanOp == HVAC::FanOp::Continuous)THEN

    if (state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed) {
        if (OutsideDryBulbTemp < state.dataHVACVarRefFlow->VRF(VRFCond).MinOATHeatRecovery ||
            OutsideDryBulbTemp > state.dataHVACVarRefFlow->VRF(VRFCond).MaxOATHeatRecovery) {
            if ((any(state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest) && state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) ||
                (any(state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest) && state.dataHVACVarRefFlow->CoolingLoad(VRFCond))) {
                if (state.dataHVACVarRefFlow->VRF(VRFCond).HRMaxTempLimitIndex == 0) {
                    ShowWarningMessage(state,
                                       format("{} \"{}\".",
                                              cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum),
                                              state.dataHVACVarRefFlow->VRF(VRFCond).Name));
                    ShowContinueError(state,
                                      "...InitVRF: VRF Heat Pump Min/Max Outdoor Temperature in Heat Recovery Mode Limits have been exceeded and "
                                      "VRF heat recovery is disabled.");
                    ShowContinueError(state, format("... Outdoor Dry-Bulb Temperature                       = {:.3T}", OutsideDryBulbTemp));
                    ShowContinueError(state,
                                      format("... Heat Recovery Minimum Outdoor Dry-Bulb Temperature = {:.3T}",
                                             state.dataHVACVarRefFlow->VRF(VRFCond).MinOATHeatRecovery));
                    ShowContinueError(state,
                                      format("... Heat Recovery Maximum Outdoor Dry-Bulb Temperature = {:.3T}",
                                             state.dataHVACVarRefFlow->VRF(VRFCond).MaxOATHeatRecovery));
                    ShowContinueErrorTimeStamp(state, "... Check VRF Heat Pump Min/Max Outdoor Temperature in Heat Recovery Mode limits.");
                    ShowContinueError(state, "...the system will operate in heat pump mode when applicable.");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)) + " \"" +
                                                   state.dataHVACVarRefFlow->VRF(VRFCond).Name +
                                                   "\" -- Exceeded VRF Heat Recovery min/max outdoor temperature limit error continues...",
                                               state.dataHVACVarRefFlow->VRF(VRFCond).HRMaxTempLimitIndex,
                                               OutsideDryBulbTemp,
                                               OutsideDryBulbTemp);
            }
            // Allow heat pump mode to operate if within limits
            if (OutsideDryBulbTemp < state.dataHVACVarRefFlow->VRF(VRFCond).MinOATCooling ||
                OutsideDryBulbTemp > state.dataHVACVarRefFlow->VRF(VRFCond).MaxOATCooling) {
                // Disable cooling mode only, heating model will still be allowed
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
            }
            if (OutsideDryBulbTemp < state.dataHVACVarRefFlow->VRF(VRFCond).MinOATHeating ||
                OutsideDryBulbTemp > state.dataHVACVarRefFlow->VRF(VRFCond).MaxOATHeating) {
                // Disable heating mode only, cooling model will still be allowed
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
            }
        }
    } else {
        state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest = false;
        state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest = false;
    }

    // Override operating mode when using EMS
    // this logic seems suspect, uses a "just run it on" mentality. Nee to test using EMS.
    if (state.dataHVACVarRefFlow->VRF(VRFCond).EMSOverrideHPOperatingMode) {
        if (state.dataHVACVarRefFlow->VRF(VRFCond).EMSValueForHPOperatingMode == 0.0) { // Off
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
            state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
            state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
        } else if (state.dataHVACVarRefFlow->VRF(VRFCond).EMSValueForHPOperatingMode == 1.0) { // Cooling
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = true;
            QZnReq = LoadToCoolingSP;
            if (state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed) {
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = false;
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = true;
            }
        } else if (state.dataHVACVarRefFlow->VRF(VRFCond).EMSValueForHPOperatingMode == 2.0) { // Heating
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = true;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
            QZnReq = LoadToHeatingSP;
            if (state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed) {
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList) = true;
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList) = false;
            }
        } else {
            if (state.dataHVACVarRefFlow->VRF(VRFCond).HPOperatingModeErrorIndex == 0) {
                ShowWarningMessage(state,
                                   format("{} \"{}\".",
                                          cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum),
                                          state.dataHVACVarRefFlow->VRF(VRFCond).Name));
                ShowContinueError(
                    state,
                    format("...InitVRF: Illegal HP operating mode = {:.0T}", state.dataHVACVarRefFlow->VRF(VRFCond).EMSValueForHPOperatingMode));
                ShowContinueError(state, "...InitVRF: VRF HP operating mode will not be controlled by EMS.");
            }
            ShowRecurringWarningErrorAtEnd(state,
                                           std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)) + " \"" +
                                               state.dataHVACVarRefFlow->VRF(VRFCond).Name + "\" -- Illegal HP operating mode error continues...",
                                           state.dataHVACVarRefFlow->VRF(VRFCond).HPOperatingModeErrorIndex,
                                           state.dataHVACVarRefFlow->VRF(VRFCond).EMSValueForHPOperatingMode,
                                           state.dataHVACVarRefFlow->VRF(VRFCond).EMSValueForHPOperatingMode);
        }
    }

    // set the TU flow rate. Check for heat recovery operation first, these will be FALSE if HR is not used.
    if (state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList)) {
        state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow;
        state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
        state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirMassFlow;
        state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
        state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolingSpeedRatio;
        state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolingSpeedRatio;
    } else if (state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList)) {
        state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow;
        state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
        state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirMassFlow;
        state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
        state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatingSpeedRatio;
        state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoHeatingSpeedRatio;
    } else if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond) && QZnReq != 0.0) {
        state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow;
        state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
        state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirMassFlow;
        state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
        state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolingSpeedRatio;
        state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolingSpeedRatio;
    } else if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond) && QZnReq != 0.0) {
        state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow;
        state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
        state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirMassFlow;
        state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
        state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatingSpeedRatio;
        state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoHeatingSpeedRatio;
    } else {
        if (state.dataHVACVarRefFlow->LastModeCooling(VRFCond)) {
            state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
            state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
            state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirMassFlow;
            state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolingSpeedRatio;
            state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolingSpeedRatio;
        }
        if (state.dataHVACVarRefFlow->LastModeHeating(VRFCond)) {
            state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
            state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
            state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirMassFlow;
            state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoHeatingSpeedRatio;
            state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoHeatingSpeedRatio;
        }
        state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
    }

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOp == HVAC::FanOp::Cycling) {
        state.dataHVACVarRefFlow->CompOffMassFlow = 0.0;
        state.dataHVACVarRefFlow->OACompOffMassFlow = 0.0;
        state.dataHVACVarRefFlow->CompOffFlowRatio = 0.0;
    }

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedHeating > 0 || state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling > 0) {
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SpeedNum = 0;
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SpeedRatio = 0.0;
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CycRatio = 0.0;
    }

    SetAverageAirFlow(state, VRFTUNum, 0.0, OnOffAirFlowRatio);

    if (ErrorsFound) {
        ShowFatalError(state,
                       format("{}: Errors found in getting ZoneHVAC:TerminalUnit:VariableRefrigerantFlow system input. Preceding condition(s) "
                              "causes termination.",
                              RoutineName));
    }
}

void SetCompFlowRate(EnergyPlusData &state, int const VRFTUNum, int const VRFCond, bool const UseCurrentMode)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   June 2011
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for calling VRF terminal units during Init to initialize flow rate
    // while looping through all terminal units connected to a specific condenser.
    // This allows polling of capacities for all terminal units.
    // Since the heat pump can only heat or cool, a single operating mode is chosen for each condenser.

    // METHODOLOGY EMPLOYED:
    // Initializes flow rates for a specific terminal unit.

    int IndexToTUInTUList; // - index to TU in specific list for this VRF system
    int TUListIndex;       // index to TU list for this VRF system

    IndexToTUInTUList = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).IndexToTUInTUList;
    TUListIndex = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TUListIndex;

    // uses current operating mode to set flow rate (after mode is set)
    if (state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList)) {
        state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow;
        state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
        state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirMassFlow;
        state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
        state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolingSpeedRatio;
        state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolingSpeedRatio;
    } else if (state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList)) {
        state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow;
        state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
        state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirMassFlow;
        state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
        state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatingSpeedRatio;
        state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoHeatingSpeedRatio;
    } else if (UseCurrentMode) { // uses current operating mode to set flow rate (after mode is set)
        if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond)) {
            state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow;
            state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
            state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirMassFlow;
            state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolingSpeedRatio;
            state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolingSpeedRatio;
        } else if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) {
            state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow;
            state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
            state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirMassFlow;
            state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatingSpeedRatio;
            state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoHeatingSpeedRatio;
        } else if (state.dataHVACVarRefFlow->LastModeCooling(VRFCond)) { // if NOT cooling or heating then use last mode
            state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow;
            state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
            state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirMassFlow;
            state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolingSpeedRatio;
            state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolingSpeedRatio;
        } else if (state.dataHVACVarRefFlow->LastModeHeating(VRFCond)) { // if NOT cooling or heating then use last mode
            state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow;
            state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
            state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirMassFlow;
            state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatingSpeedRatio;
            state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoHeatingSpeedRatio;
        } else { // should not happen so just set to cooling flow rate
            state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow;
            state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
            state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirMassFlow;
            state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolingSpeedRatio;
            state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolingSpeedRatio;
        }
    } else { // uses previous operating mode to set flow rate (used for looping through each TU in Init before mode is set)
        if (state.dataHVACVarRefFlow->LastModeCooling(VRFCond)) {
            state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow;
            state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
            state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirMassFlow;
            state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolingSpeedRatio;
            state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolingSpeedRatio;
        } else if (state.dataHVACVarRefFlow->LastModeHeating(VRFCond)) {
            state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirMassFlow;
            state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirMassFlow;
            state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirMassFlow;
            state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatingSpeedRatio;
            state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoHeatingSpeedRatio;
        } else { // should not happen so just set to cooling flow rate
            state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirMassFlow;
            state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirMassFlow;
            state.dataHVACVarRefFlow->OACompOnMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirMassFlow;
            state.dataHVACVarRefFlow->OACompOffMassFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirMassFlow;
            state.dataHVACVarRefFlow->CompOnFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolingSpeedRatio;
            state.dataHVACVarRefFlow->CompOffFlowRatio = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolingSpeedRatio;
        }
    }

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOp == HVAC::FanOp::Cycling) {
        state.dataHVACVarRefFlow->CompOffMassFlow = 0.0;
        state.dataHVACVarRefFlow->OACompOffMassFlow = 0.0;
        state.dataHVACVarRefFlow->CompOffFlowRatio = 0.0;
    }
}

void SizeVRF(EnergyPlusData &state, int const VRFTUNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   August 2010
    //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
    //                      B Nigusse, FSEC, added scalable sizing
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for sizing VRF Components for which inputs have not been
    // specified in the input.

    // METHODOLOGY EMPLOYED:
    // Obtains flow rates from the zone or system sizing arrays.

    using namespace DataSizing;
    using Curve::CurveValue;
    using HVAC::CoolingAirflowSizing;
    using HVAC::CoolingCapacitySizing;
    using HVAC::HeatingAirflowSizing;
    using HVAC::HeatingCapacitySizing;

    using PlantUtilities::RegisterPlantCompDesignFlow;

    static constexpr std::string_view RoutineName("SizeVRF: "); // include trailing blank space

    auto &CheckVRFCombinationRatio = state.dataHVACVarRefFlow->CheckVRFCombinationRatio;
    Real64 TUCoolingCapacity;           // total terminal unit cooling capacity
    Real64 TUHeatingCapacity;           // total terminal unit heating capacity
    int VRFCond;                        // index to VRF condenser
    Real64 OnOffAirFlowRat;             // temporary variable used when sizing coils
    Real64 DXCoilCap;                   // capacity of DX cooling coil (W)
    bool IsAutoSize;                    // Indicator to autosize
    Real64 MaxCoolAirVolFlowDes;        // Autosized supply air during cooling for reporting
    Real64 MaxCoolAirVolFlowUser;       // Hardsized supply air during cooling for reporting
    Real64 MaxHeatAirVolFlowDes;        // Autosized supply air during heating for reporting
    Real64 MaxHeatAirVolFlowUser;       // Hardsized supply air during heating for reporting
    Real64 MaxNoCoolAirVolFlowDes;      // Autosized supply air flow when no cooling is needed for reporting
    Real64 MaxNoCoolAirVolFlowUser;     // Hardsized supply air flow when no cooling is needed for reporting
    Real64 MaxNoHeatAirVolFlowDes;      // Autosized supply air flow when no heating is needed for reporting
    Real64 MaxNoHeatAirVolFlowUser;     // Hardsized supply air flow when no heating is needed for reporting
    Real64 CoolOutAirVolFlowDes;        // Autosized outdoor air flow during cooling for reporting
    Real64 CoolOutAirVolFlowUser;       // Hardsized outdoor air flow during cooling for reporting
    Real64 HeatOutAirVolFlowDes;        // Autosized outdoor air flow during heating for reporting
    Real64 HeatOutAirVolFlowUser;       // Hardsized outdoor air flow during heating for reporting
    Real64 NoCoolHeatOutAirVolFlowDes;  // Autosized outdoor air when unconditioned for reporting
    Real64 NoCoolHeatOutAirVolFlowUser; // Hardsized outdoor air when unconditioned for reporting
    Real64 CoolingCapacityDes;          // Autosized cooling capacity for reporting
    Real64 CoolingCapacityUser;         // Hardsized cooling capacity for reporting
    Real64 HeatingCapacityDes;          // Autosized heating capacity for reporting
    Real64 HeatingCapacityUser;         // Hardsized heating capacity for reporting
    Real64 DefrostCapacityDes;          // Autosized defrost heater capacity for reporting
    Real64 DefrostCapacityUser;         // Hardsized defrost heater capacity for reporting
    Real64 EvapCondAirVolFlowRateDes;   // Autosized evaporative condenser flow for reporting
    Real64 EvapCondAirVolFlowRateUser;  // Hardsized evaporative condenser flow for reporting
    Real64 EvapCondPumpPowerDes;        // Autosized evaporative condenser pump power for reporting
    Real64 EvapCondPumpPowerUser;       // Hardsized evaporative condenser pump power for reporting

    std::string CompName;     // component name
    std::string CompType;     // component type
    std::string SizingString; // input field sizing description (e.g., Nominal Capacity)
    Real64 TempSize;          // autosized value of coil input field
    int FieldNum = 2;         // IDD numeric field number where input field description is found
    bool PrintFlag = true;    // TRUE when sizing information is reported in the eio file

    auto &ZoneEqSizing = state.dataSize->ZoneEqSizing;

    DataSizing::ZoneEqSizingData *select_EqSizing(nullptr);

    // sweep specific data into one pointer to avoid if statements throughout this subroutine
    if (state.dataSize->CurOASysNum > 0) {
        select_EqSizing = &state.dataSize->OASysEqSizing(state.dataSize->CurOASysNum);
    } else if (state.dataSize->CurSysNum > 0) {
        select_EqSizing = &state.dataSize->UnitarySysEqSizing(state.dataSize->CurSysNum);
    } else if (state.dataSize->CurZoneEqNum > 0) {
        select_EqSizing = &ZoneEqSizing(state.dataSize->CurZoneEqNum);
        state.dataSize->ZoneEqUnitarySys = true;
    } else {
        assert(false);
    }
    // Object Data, points to specific array
    DataSizing::ZoneEqSizingData &EqSizing(*select_EqSizing);

    // can't hurt to initialize these going in, probably redundant
    EqSizing.AirFlow = false;
    EqSizing.CoolingAirFlow = false;
    EqSizing.HeatingAirFlow = false;
    EqSizing.AirVolFlow = 0.0;
    EqSizing.CoolingAirVolFlow = 0.0;
    EqSizing.HeatingAirVolFlow = 0.0;
    EqSizing.Capacity = false;
    EqSizing.CoolingCapacity = false;
    EqSizing.HeatingCapacity = false;
    EqSizing.DesCoolingLoad = 0.0;
    EqSizing.DesHeatingLoad = 0.0;
    EqSizing.OAVolFlow = 0.0;

    VRFCond = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFSysNum;
    MaxCoolAirVolFlowDes = 0.0;
    MaxCoolAirVolFlowUser = 0.0;
    MaxHeatAirVolFlowDes = 0.0;
    MaxHeatAirVolFlowUser = 0.0;
    MaxNoCoolAirVolFlowDes = 0.0;
    MaxNoCoolAirVolFlowUser = 0.0;
    MaxNoHeatAirVolFlowDes = 0.0;
    MaxNoHeatAirVolFlowUser = 0.0;
    CoolOutAirVolFlowDes = 0.0;
    CoolOutAirVolFlowUser = 0.0;
    HeatOutAirVolFlowDes = 0.0;
    HeatOutAirVolFlowUser = 0.0;
    NoCoolHeatOutAirVolFlowDes = 0.0;
    NoCoolHeatOutAirVolFlowUser = 0.0;
    CoolingCapacityDes = 0.0;
    CoolingCapacityUser = 0.0;
    HeatingCapacityDes = 0.0;
    HeatingCapacityUser = 0.0;
    DefrostCapacityDes = 0.0;
    DefrostCapacityUser = 0.0;
    EvapCondAirVolFlowRateDes = 0.0;
    EvapCondAirVolFlowRateUser = 0.0;
    EvapCondPumpPowerDes = 0.0;
    EvapCondPumpPowerUser = 0.0;

    state.dataSize->DataScalableSizingON = false;
    state.dataSize->DataScalableCapSizingON = false;
    state.dataSize->DataFracOfAutosizedCoolingAirflow = 1.0;
    state.dataSize->DataFracOfAutosizedHeatingAirflow = 1.0;
    state.dataSize->DataFracOfAutosizedCoolingCapacity = 1.0;
    state.dataSize->DataFracOfAutosizedHeatingCapacity = 1.0;
    state.dataSize->SuppHeatCap = 0.0;

    if (state.dataHVACVarRefFlow->MyOneTimeSizeFlag) {
        // initialize the environment and sizing flags
        CheckVRFCombinationRatio.dimension(state.dataHVACVarRefFlow->NumVRFCond, true);
        state.dataHVACVarRefFlow->MyOneTimeSizeFlag = false;
    }

    CompType = tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type];
    CompName = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name;
    state.dataSize->DataZoneNumber = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum;

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).FanIndex > 0) {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInZone) {
            state.dataSize->DataFanType = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanType;
            state.dataSize->DataFanIndex = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).FanIndex;
            state.dataSize->DataFanPlacement = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanPlace;
        } else if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInAirLoop) {
            state.dataAirSystemsData->PrimaryAirSystems(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).airLoopNum).supFanType =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanType;
            state.dataAirSystemsData->PrimaryAirSystems(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).airLoopNum).supFanNum =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).FanIndex;
            state.dataAirSystemsData->PrimaryAirSystems(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).airLoopNum).supFanPlace =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanPlace;
        }
    }

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HVACSizingIndex > 0) {
        // initialize OA flow for sizing other inputs (e.g., capacity)
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow == AutoSize) {
            EqSizing.OAVolFlow = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA;
        } else {
            EqSizing.OAVolFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow;
        }
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow != AutoSize) {
            EqSizing.OAVolFlow = max(EqSizing.OAVolFlow, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow);
        }
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerExists &&
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInZone) { // set up ATMixer conditions for scalable capacity sizing
            EqSizing.OAVolFlow = 0.0;                             // Equipment OA flow should always be 0 when ATMixer is used
            SingleDuct::setATMixerSizingProperties(state,
                                                   state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerIndex,
                                                   state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum,
                                                   state.dataSize->CurZoneEqNum);
        }

        int zoneHVACIndex = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HVACSizingIndex;

        // Integer representation of sizing method name (e.g., CoolingAirflowSizing, HeatingAirflowSizing, CoolingCapacitySizing,
        // HeatingCapacitySizing, etc.)
        int SizingMethod = CoolingAirflowSizing;
        PrintFlag = true;
        bool errorsFound = false;
        // supply air flow rate sizing method (SupplyAirFlowRate, FlowPerFloorArea, FractionOfAutosizedCoolingAirflow,
        // FractionOfAutosizedHeatingAirflow ...)
        int SAFMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingSAFMethod;
        EqSizing.SizingMethod(SizingMethod) = SAFMethod;
        if (SAFMethod == SupplyAirFlowRate || SAFMethod == FlowPerFloorArea || SAFMethod == FractionOfAutosizedCoolingAirflow) {
            if (SAFMethod == SupplyAirFlowRate) {
                if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow > 0.0) {
                    EqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                    EqSizing.SystemAirFlow = true;
                }
                TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
            } else if (SAFMethod == FlowPerFloorArea) {
                EqSizing.SystemAirFlow = true;
                EqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow *
                                      state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                TempSize = ZoneEqSizing(state.dataSize->CurZoneEqNum).AirVolFlow;
                state.dataSize->DataScalableSizingON = true;
            } else if (SAFMethod == FractionOfAutosizedCoolingAirflow) {
                state.dataSize->DataFracOfAutosizedCoolingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                TempSize = AutoSize;
                state.dataSize->DataScalableSizingON = true;
            } else {
                TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
            }

            CoolingAirFlowSizer sizingCoolingAirFlow;
            std::string stringOverride = "Cooling Supply Air Flow Rate [m3/s]";
            if (state.dataGlobal->isEpJSON) {
                stringOverride = "cooling_supply_air_flow_rate [m3/s]";
            }
            sizingCoolingAirFlow.overrideSizingString(stringOverride);
            // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
            sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow = sizingCoolingAirFlow.size(state, TempSize, errorsFound);

        } else if (SAFMethod == FlowPerCoolingCapacity) {
            SizingMethod = CoolingCapacitySizing; // either this isn't needed or needs to be assigned to EqSizing
            TempSize = AutoSize;
            PrintFlag = false;
            state.dataSize->DataScalableSizingON = true;
            state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow;
            if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingCapMethod == FractionOfAutosizedCoolingCapacity) {
                state.dataSize->DataFracOfAutosizedCoolingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
            }
            CoolingCapacitySizer sizerCoolingCapacity;
            sizerCoolingCapacity.overrideSizingString(SizingString);
            sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            state.dataSize->DataAutosizedCoolingCapacity = sizerCoolingCapacity.size(state, TempSize, errorsFound);
            state.dataSize->DataFlowPerCoolingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
            PrintFlag = true;
            TempSize = AutoSize;
            CoolingAirFlowSizer sizingCoolingAirFlow;
            std::string stringOverride = "Cooling Supply Air Flow Rate [m3/s]";
            if (state.dataGlobal->isEpJSON) {
                stringOverride = "cooling_supply_air_flow_rate [m3/s]";
            }
            sizingCoolingAirFlow.overrideSizingString(stringOverride);
            // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
            sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow = sizingCoolingAirFlow.size(state, TempSize, errorsFound);
        }
        // Multispeed Fan cooling flow sizing
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling > 0) {
            Real64 AirFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow;
            for (int i = 1; i <= state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling; ++i) {
                if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex > -1) {
                    if (state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex]
                            .coolingVolFlowRatio[i] == DataSizing::AutoSize) {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[i] =
                            double(i) / double(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling) * AirFlowRate;
                    } else {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[i] =
                            state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex]
                                .coolingVolFlowRatio[i] *
                            AirFlowRate;
                    }
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolMassFlowRate[i] =
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[i] * state.dataEnvrn->StdRhoAir;
                } else {
                    auto *fanSystem = dynamic_cast<Fans::FanSystem *>(state.dataFans->fans(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).FanIndex));
                    assert(fanSystem != nullptr);
                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolMassFlowRate[i] == 0.0 && !fanSystem->massFlowAtSpeed.empty()) {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolMassFlowRate[i] = fanSystem->massFlowAtSpeed[i - 1];
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[i] =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolMassFlowRate[i] / state.dataEnvrn->StdRhoAir;
                    }
                }
            }
        }

        SizingMethod = HeatingAirflowSizing;
        FieldNum = 3; // N3, \field Supply Air Flow Rate During Heating Operation
        PrintFlag = true;
        SizingString = state.dataHVACVarRefFlow->VRFTUNumericFields(VRFTUNum).FieldNames(FieldNum) + " [m3/s]";
        SAFMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingSAFMethod;
        EqSizing.SizingMethod(SizingMethod) = SAFMethod;
        if (SAFMethod == SupplyAirFlowRate || SAFMethod == FlowPerFloorArea || SAFMethod == FractionOfAutosizedHeatingAirflow) {
            if (SAFMethod == SupplyAirFlowRate) {
                if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow > 0.0) {
                    EqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                    EqSizing.SystemAirFlow = true;
                }
                TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
            } else if (SAFMethod == FlowPerFloorArea) {
                EqSizing.SystemAirFlow = true;
                EqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow *
                                      state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                TempSize = ZoneEqSizing(state.dataSize->CurZoneEqNum).AirVolFlow;
                state.dataSize->DataScalableSizingON = true;
            } else if (SAFMethod == FractionOfAutosizedHeatingAirflow) {
                state.dataSize->DataFracOfAutosizedHeatingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                TempSize = AutoSize;
                state.dataSize->DataScalableSizingON = true;
            } else {
                TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
            }
            errorsFound = false;
            HeatingAirFlowSizer sizingHeatingAirFlow;
            sizingHeatingAirFlow.overrideSizingString(SizingString);
            // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
            sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow = sizingHeatingAirFlow.size(state, TempSize, errorsFound);
        } else if (SAFMethod == FlowPerHeatingCapacity) {
            SizingMethod = HeatingCapacitySizing; // either this isn't needed or needs to be assigned to EqSizing
            TempSize = AutoSize;
            PrintFlag = false;
            state.dataSize->DataScalableSizingON = true;
            state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow;
            if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingCapMethod == FractionOfAutosizedHeatingCapacity) {
                state.dataSize->DataFracOfAutosizedHeatingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
            }
            errorsFound = false;
            HeatingCapacitySizer sizerHeatingCapacity;
            sizerHeatingCapacity.overrideSizingString(SizingString);
            sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            state.dataSize->DataAutosizedHeatingCapacity = sizerHeatingCapacity.size(state, TempSize, errorsFound);
            state.dataSize->DataFlowPerHeatingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
            SizingMethod = HeatingAirflowSizing; // either this isn't needed or needs to be assigned to EqSizing
            PrintFlag = true;
            TempSize = AutoSize;
            errorsFound = false;
            HeatingAirFlowSizer sizingHeatingAirFlow;
            sizingHeatingAirFlow.overrideSizingString(SizingString);
            // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
            sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow = sizingHeatingAirFlow.size(state, TempSize, errorsFound);
        }
        // Multispeed Fan heating flow sizing
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedHeating > 0) {
            Real64 AirFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow;
            for (int i = 1; i <= state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedHeating; ++i) {
                if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex > -1) {
                    if (state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex]
                            .heatingVolFlowRatio[i] == DataSizing::AutoSize) {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate[i] =
                            double(i) / double(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedHeating) * AirFlowRate;
                    } else {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate[i] =
                            state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex]
                                .heatingVolFlowRatio[i] *
                            AirFlowRate;
                    }
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatMassFlowRate[i] =
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate[i] * state.dataEnvrn->StdRhoAir;
                } else {
                    auto *fanSystem = dynamic_cast<Fans::FanSystem *>(state.dataFans->fans(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).FanIndex));
                    assert(fanSystem != nullptr);
                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatMassFlowRate[i] == 0.0 && !fanSystem->massFlowAtSpeed.empty()) {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatMassFlowRate[i] = fanSystem->massFlowAtSpeed[i - 1];
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate[i] =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatMassFlowRate[i] / state.dataEnvrn->StdRhoAir;
                    }
                }
            }
        }

        PrintFlag = true;
        SAFMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).NoCoolHeatSAFMethod;
        EqSizing.SizingMethod(SizingMethod) = SAFMethod;
        if ((SAFMethod == SupplyAirFlowRate) || (SAFMethod == FlowPerFloorArea) || (SAFMethod == FractionOfAutosizedHeatingAirflow) ||
            (SAFMethod == FractionOfAutosizedCoolingAirflow)) {
            if (SAFMethod == SupplyAirFlowRate) {
                if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow > 0.0) {
                    EqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                    EqSizing.SystemAirFlow = true;
                }
                TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
            } else if (SAFMethod == FlowPerFloorArea) {
                EqSizing.SystemAirFlow = true;
                EqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow *
                                      state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                TempSize = ZoneEqSizing(state.dataSize->CurZoneEqNum).AirVolFlow;
                state.dataSize->DataScalableSizingON = true;
            } else if (SAFMethod == FractionOfAutosizedCoolingAirflow) {
                state.dataSize->DataFracOfAutosizedCoolingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                state.dataSize->DataFracOfAutosizedHeatingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                TempSize = AutoSize;
                state.dataSize->DataScalableSizingON = true;
            } else if (SAFMethod == FractionOfAutosizedHeatingAirflow) {
                state.dataSize->DataFracOfAutosizedCoolingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                state.dataSize->DataFracOfAutosizedHeatingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                TempSize = AutoSize;
                state.dataSize->DataScalableSizingON = true;
            } else {
                TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
            }
            CoolingAirFlowSizer sizingCoolingAirFlow;
            std::string stringOverride = "No Cooling Supply Air Flow Rate [m3/s]";
            if (state.dataGlobal->isEpJSON) {
                stringOverride = "no_cooling_supply_air_flow_rate [m3/s]";
            }
            sizingCoolingAirFlow.overrideSizingString(stringOverride);
            // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
            sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling > 0) {
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirVolFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[1];
                sizingCoolingAirFlow.reportSizerOutput(state,
                                                       sizingCoolingAirFlow.compType,
                                                       sizingCoolingAirFlow.compName,
                                                       "Design Size " + stringOverride,
                                                       state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirVolFlow);
            } else {
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirVolFlow = sizingCoolingAirFlow.size(state, TempSize, errorsFound);
            }
        }

        SizingMethod = HeatingAirflowSizing;
        FieldNum = 4; // N4, \field Supply Air Flow Rate When No Heating is Needed
        PrintFlag = true;
        SizingString = state.dataHVACVarRefFlow->VRFTUNumericFields(VRFTUNum).FieldNames(FieldNum) + " [m3/s]";
        SAFMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).NoCoolHeatSAFMethod;
        EqSizing.SizingMethod(SizingMethod) = SAFMethod;
        if ((SAFMethod == SupplyAirFlowRate) || (SAFMethod == FlowPerFloorArea) || (SAFMethod == FractionOfAutosizedHeatingAirflow) ||
            (SAFMethod == FractionOfAutosizedCoolingAirflow)) {
            if (SAFMethod == SupplyAirFlowRate) {
                if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow > 0.0) {
                    EqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                    EqSizing.SystemAirFlow = true;
                }
                TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
            } else if (SAFMethod == FlowPerFloorArea) {
                EqSizing.SystemAirFlow = true;
                EqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow *
                                      state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                TempSize = ZoneEqSizing(state.dataSize->CurZoneEqNum).AirVolFlow;
                state.dataSize->DataScalableSizingON = true;
            } else if (SAFMethod == FractionOfAutosizedHeatingAirflow) {
                state.dataSize->DataFracOfAutosizedCoolingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                state.dataSize->DataFracOfAutosizedHeatingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                TempSize = AutoSize;
                state.dataSize->DataScalableSizingON = true;
            } else if (SAFMethod == FractionOfAutosizedCoolingAirflow) {
                state.dataSize->DataFracOfAutosizedCoolingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                state.dataSize->DataFracOfAutosizedHeatingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
                TempSize = AutoSize;
                state.dataSize->DataScalableSizingON = true;
            } else {
                TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxNoCoolHeatAirVolFlow;
            }
            errorsFound = false;
            HeatingAirFlowSizer sizingNoHeatingAirFlow;
            sizingNoHeatingAirFlow.overrideSizingString(SizingString);
            // sizingNoHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
            sizingNoHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedHeating > 0) {
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirVolFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate[1];
                sizingNoHeatingAirFlow.reportSizerOutput(state,
                                                         sizingNoHeatingAirFlow.compType,
                                                         sizingNoHeatingAirFlow.compName,
                                                         "Design Size " + SizingString,
                                                         state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirVolFlow);
            } else {
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirVolFlow = sizingNoHeatingAirFlow.size(state, TempSize, errorsFound);
            }
        }

        // initialize capacity sizing variables: cooling
        initCapSizingVars(state,
                          CoolingCapacitySizing,
                          state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingCapMethod,
                          EqSizing.SizingMethod(SizingMethod),
                          state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity,
                          EqSizing.CoolingCapacity,
                          EqSizing.DesCoolingLoad,
                          state.dataSize->DataScalableCapSizingON,
                          state.dataSize->DataFracOfAutosizedCoolingCapacity);

        // initialize capacity sizing variables: heating
        initCapSizingVars(state,
                          HeatingCapacitySizing,
                          state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingCapMethod,
                          EqSizing.SizingMethod(SizingMethod),
                          state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity,
                          EqSizing.HeatingCapacity,
                          EqSizing.DesHeatingLoad,
                          state.dataSize->DataScalableCapSizingON,
                          state.dataSize->DataFracOfAutosizedHeatingCapacity);

    } else {
        // no scalable sizing method has been specified. Sizing proceeds using the method
        // specified in the zoneHVAC object

        PrintFlag = true;

        TempSize = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow;
        bool errorsFound = false;
        CoolingAirFlowSizer sizingCoolingAirFlow;
        std::string stringOverride = "Cooling Supply Air Flow Rate [m3/s]";
        if (state.dataGlobal->isEpJSON) {
            stringOverride = "cooling_supply_air_flow_rate [m3/s]";
        }
        sizingCoolingAirFlow.overrideSizingString(stringOverride);
        // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
        sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow = sizingCoolingAirFlow.size(state, TempSize, errorsFound);
        // Multispeed Fan cooling flow sizing
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling > 0) {
            Real64 AirFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow;
            for (int i = 1; i <= state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling; ++i) {
                if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex > -1) {
                    if (state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex]
                            .coolingVolFlowRatio[i] == DataSizing::AutoSize) {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[i] =
                            double(i) / double(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling) * AirFlowRate;
                    } else {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[i] =
                            state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex]
                                .coolingVolFlowRatio[i] *
                            AirFlowRate;
                    }
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolMassFlowRate[i] =
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[i] * state.dataEnvrn->StdRhoAir;
                } else {
                    auto *fanSystem = dynamic_cast<Fans::FanSystem *>(state.dataFans->fans(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).FanIndex));
                    assert(fanSystem != nullptr);
                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolMassFlowRate[i] == 0.0 && !fanSystem->massFlowAtSpeed.empty()) {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolMassFlowRate[i] = fanSystem->massFlowAtSpeed[i - 1];
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[i] =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolMassFlowRate[i] / state.dataEnvrn->StdRhoAir;
                    }
                }
            }
        }

        FieldNum = 3; // N3, \field Supply Air Flow Rate During Heating Operation
        SizingString = state.dataHVACVarRefFlow->VRFTUNumericFields(VRFTUNum).FieldNames(FieldNum) + " [m3/s]";
        TempSize = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow;
        errorsFound = false;
        HeatingAirFlowSizer sizingHeatingAirFlow;
        sizingHeatingAirFlow.overrideSizingString(SizingString);
        // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
        sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow = sizingHeatingAirFlow.size(state, TempSize, errorsFound);
        // Multispeed Fan heating flow sizing
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedHeating > 0) {
            Real64 AirFlowRate = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow;
            for (int i = 1; i <= state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedHeating; ++i) {
                if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex > -1) {
                    if (state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex]
                            .heatingVolFlowRatio[i] == DataSizing::AutoSize) {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate[i] =
                            double(i) / double(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedHeating) * AirFlowRate;
                    } else {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate[i] =
                            state.dataUnitarySystems->designSpecMSHP[state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex]
                                .heatingVolFlowRatio[i] *
                            AirFlowRate;
                    }
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatMassFlowRate[i] =
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate[i] * state.dataEnvrn->StdRhoAir;
                } else {
                    auto *fanSystem = dynamic_cast<Fans::FanSystem *>(state.dataFans->fans(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).FanIndex));
                    assert(fanSystem != nullptr);
                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatMassFlowRate[i] == 0.0 && !fanSystem->massFlowAtSpeed.empty()) {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatMassFlowRate[i] = fanSystem->massFlowAtSpeed[i - 1];
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate[i] =
                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatMassFlowRate[i] / state.dataEnvrn->StdRhoAir;
                    }
                }
            }
        }

        errorsFound = false;
        SystemAirFlowSizer sizerSystemAirFlow;
        std::string sizingString = "No Cooling Supply Air Flow Rate [m3/s]";
        sizerSystemAirFlow.overrideSizingString(sizingString);
        sizerSystemAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling > 0) {
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirVolFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[1];
            sizerSystemAirFlow.reportSizerOutput(state,
                                                 sizerSystemAirFlow.compType,
                                                 sizerSystemAirFlow.compName,
                                                 "Design Size " + sizingString,
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirVolFlow);
        } else {
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirVolFlow =
                sizerSystemAirFlow.size(state, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoCoolAirVolFlow, errorsFound);
        }

        SystemAirFlowSizer sizerSystemAirFlow2;
        sizingString = "No Heating Supply Air Flow Rate [m3/s]";
        sizerSystemAirFlow2.overrideSizingString(sizingString);
        sizerSystemAirFlow2.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedHeating > 0) {
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirVolFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate[1];
            sizerSystemAirFlow.reportSizerOutput(state,
                                                 sizerSystemAirFlow.compType,
                                                 sizerSystemAirFlow.compName,
                                                 "Design Size " + sizingString,
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirVolFlow);
        } else {
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirVolFlow =
                sizerSystemAirFlow2.size(state, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxNoHeatAirVolFlow, errorsFound);
        }
    }
    IsAutoSize = false;
    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow == AutoSize) {
        IsAutoSize = true;
    }
    if (state.dataSize->CurZoneEqNum > 0) {
        if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow > 0.0) {
                BaseSizer::reportSizerOutput(state,
                                             tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name,
                                             "User-Specified Outdoor Air Flow Rate During Cooling Operation [m3/s]",
                                             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow);
            }
        } else {
            CheckZoneSizing(state, tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type], state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name);
            CoolOutAirVolFlowDes =
                min(state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow);
            if (CoolOutAirVolFlowDes < HVAC::SmallAirVolFlow) {
                CoolOutAirVolFlowDes = 0.0;
            }

            if (IsAutoSize) {
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow = CoolOutAirVolFlowDes;
                BaseSizer::reportSizerOutput(state,
                                             tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name,
                                             "Design Size Outdoor Air Flow Rate During Cooling Operation [m3/s]",
                                             CoolOutAirVolFlowDes);
            } else {
                if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow > 0.0 && CoolOutAirVolFlowDes > 0.0) {
                    CoolOutAirVolFlowUser = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow;
                    BaseSizer::reportSizerOutput(state,
                                                 tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name,
                                                 "Design Size Outdoor Air Flow Rate During Cooling Operation [m3/s]",
                                                 CoolOutAirVolFlowDes,
                                                 "User-Specified Outdoor Air Flow Rate During Cooling Operation [m3/s]",
                                                 CoolOutAirVolFlowUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(CoolOutAirVolFlowDes - CoolOutAirVolFlowUser) / CoolOutAirVolFlowUser) >
                            state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state,
                                        format("SizeVRF: Potential issue with equipment sizing for {} {}",
                                               tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                               state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                            ShowContinueError(
                                state,
                                format("User-Specified Outdoor Air Flow Rate During Cooling Operation of {:.5R} [m3/s]", CoolOutAirVolFlowUser));
                            ShowContinueError(state,
                                              format("differs from Design Size Outdoor Air Flow Rate During Cooling Operation of {:.5R} [m3/s]",
                                                     CoolOutAirVolFlowDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }
        }
    } else {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow == DataSizing::AutoSize) {
            if (state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).OASysExists) {
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow = 0.0;
            } else {
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow =
                    min(state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesOutAirVolFlow,
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow);
            }
            BaseSizer::reportSizerOutput(state,
                                         tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                         state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name,
                                         "Design Size Outdoor Air Flow Rate During Cooling Operation [m3/s]",
                                         state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow);
        }
    }

    IsAutoSize = false;
    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow == AutoSize) {
        IsAutoSize = true;
    }
    if (state.dataSize->CurZoneEqNum > 0) {
        if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow > 0.0) {
                BaseSizer::reportSizerOutput(state,
                                             tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name,
                                             "Outdoor Air Flow Rate During Heating Operation [m3/s]",
                                             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow);
            }
        } else {
            CheckZoneSizing(state, tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type], state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name);
            HeatOutAirVolFlowDes =
                min(state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow);
            if (HeatOutAirVolFlowDes < HVAC::SmallAirVolFlow) {
                HeatOutAirVolFlowDes = 0.0;
            }

            if (IsAutoSize) {
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow = HeatOutAirVolFlowDes;
                BaseSizer::reportSizerOutput(state,
                                             tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name,
                                             "Design Size Outdoor Air Flow Rate During Heating Operation [m3/s]",
                                             HeatOutAirVolFlowDes);
            } else {
                if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow > 0.0 && HeatOutAirVolFlowDes > 0.0) {
                    HeatOutAirVolFlowUser = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow;
                    BaseSizer::reportSizerOutput(state,
                                                 tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name,
                                                 "Design Size Outdoor Air Flow Rate During Heating Operation [m3/s]",
                                                 HeatOutAirVolFlowDes,
                                                 "User-Specified Outdoor Air Flow Rate During Heating Operation [m3/s]",
                                                 HeatOutAirVolFlowUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(HeatOutAirVolFlowDes - HeatOutAirVolFlowUser) / HeatOutAirVolFlowUser) >
                            state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state,
                                        format("SizeVRF: Potential issue with equipment sizing for {} {}",
                                               tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                               state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                            ShowContinueError(
                                state,
                                format("User-Specified Outdoor Air Flow Rate During Heating Operation of {:.5R} [m3/s]", HeatOutAirVolFlowUser));
                            ShowContinueError(state,
                                              format("differs from Design Size Outdoor Air Flow Rate During Heating Operation of {:.5R} [m3/s]",
                                                     HeatOutAirVolFlowDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }
        }
    } else {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow == DataSizing::AutoSize) {
            if (state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).OASysExists) {
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow = 0.0;
            } else {
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow =
                    min(state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesOutAirVolFlow,
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow);
            }
            BaseSizer::reportSizerOutput(state,
                                         tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                         state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name,
                                         "Design Size Outdoor Air Flow Rate During Heating Operation [m3/s]",
                                         state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow);
        }
    }
    EqSizing.OAVolFlow =
        max(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow);

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerExists &&
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInZone) { // set up ATMixer conditions for use in component sizing
        EqSizing.OAVolFlow = 0.0;                             // Equipment OA flow should always be 0 when ATMixer is used
        SingleDuct::setATMixerSizingProperties(state,
                                               state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerIndex,
                                               state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum,
                                               state.dataSize->CurZoneEqNum);
    }

    IsAutoSize = false;
    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow == AutoSize) {
        IsAutoSize = true;
    }
    if (state.dataSize->CurZoneEqNum > 0) {
        if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow > 0.0) {
                BaseSizer::reportSizerOutput(state,
                                             tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name,
                                             "User-Specified Outdoor Air Flow Rate When No Cooling or Heating is Needed [m3/s]",
                                             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow);
            }
        } else {
            CheckZoneSizing(state, tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type], state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name);
            NoCoolHeatOutAirVolFlowDes = min(state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA,
                                             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow,
                                             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow);
            if (NoCoolHeatOutAirVolFlowDes < HVAC::SmallAirVolFlow) {
                NoCoolHeatOutAirVolFlowDes = 0.0;
            }

            if (IsAutoSize) {
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow = NoCoolHeatOutAirVolFlowDes;
                BaseSizer::reportSizerOutput(state,
                                             tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                             state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name,
                                             "Design Size Outdoor Air Flow Rate When No Cooling or Heating is Needed [m3/s]",
                                             NoCoolHeatOutAirVolFlowDes);
            } else {
                if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow > 0.0 && NoCoolHeatOutAirVolFlowDes > 0.0) {
                    NoCoolHeatOutAirVolFlowUser = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow;
                    BaseSizer::reportSizerOutput(state,
                                                 tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                                 state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name,
                                                 "Design Size Outdoor Air Flow Rate When No Cooling or Heating is Needed [m3/s]",
                                                 NoCoolHeatOutAirVolFlowDes,
                                                 "User-Specified Outdoor Air Flow Rate When No Cooling or Heating is Needed [m3/s]",
                                                 NoCoolHeatOutAirVolFlowUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(NoCoolHeatOutAirVolFlowDes - NoCoolHeatOutAirVolFlowUser) / NoCoolHeatOutAirVolFlowUser) >
                            state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state,
                                        format("SizeVRF: Potential issue with equipment sizing for {} {}",
                                               tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                               state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name));
                            ShowContinueError(state,
                                              format("User-Specified Outdoor Air Flow Rate When No Cooling or Heating is Needed of {:.5R} [m3/s]",
                                                     NoCoolHeatOutAirVolFlowUser));
                            ShowContinueError(
                                state,
                                format("differs from Design Size Outdoor Air Flow Rate When No Cooling or Heating is Needed of {:.5R} [m3/s]",
                                       NoCoolHeatOutAirVolFlowDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }
        }
    } else {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow == DataSizing::AutoSize) {
            if (state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).OASysExists) {
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow = 0.0;
            } else {
                if (!(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedCooling > 0 &&
                      state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NumOfSpeedHeating > 0)) {
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow =
                        min(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow);
                } else {
                    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSpecMSHPIndex > -1) {
                        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow =
                            min(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatVolumeFlowRate[1],
                                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolVolumeFlowRate[1]);
                    }
                }
            }
            BaseSizer::reportSizerOutput(state,
                                         tuTypeNames[(int)state.dataHVACVarRefFlow->VRFTU(VRFTUNum).type],
                                         state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name,
                                         "Design Size Outdoor Air Flow Rate When No Cooling or Heating Heating is Needed [m3/s]",
                                         state.dataHVACVarRefFlow->VRFTU(VRFTUNum).NoCoolHeatOutAirVolFlow);
        }
    }

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatingCoilPresent) {
        bool ErrorsFound = false;
        TempSize = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxSATFromSuppHeatCoil;
        MaxHeaterOutletTempSizer sizerMaxHeaterOutTemp;
        std::string stringOverride = "Maximum Supply Air Temperature from Supplemental Heater [C]";
        if (state.dataGlobal->isEpJSON) {
            stringOverride = "maximum_supply_air_temperature_from_supplemental_heater [C]";
        }
        sizerMaxHeaterOutTemp.overrideSizingString(stringOverride);
        sizerMaxHeaterOutTemp.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxSATFromSuppHeatCoil = sizerMaxHeaterOutTemp.size(state, TempSize, ErrorsFound);
    }

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilType_Num == HVAC::Coil_HeatingWater) {
        bool ErrorsFound = false;
        WaterCoils::SetCoilDesFlow(state,
                                   state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilType,
                                   state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilName,
                                   state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow,
                                   ErrorsFound);
    }

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatingCoilPresent) {
        CompType = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilType;
        CompName = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilName;
        PrintFlag = false; // why isn't this being reported?
        TempSize = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSuppHeatingCapacity;
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilType_Num == HVAC::Coil_HeatingWater) {
            // sizing result should always be reported
            if (TempSize == DataSizing::AutoSize) {
                WaterHeatingCapacitySizer sizerWaterHeatingCapacity;
                bool ErrorsFound = false;
                std::string stringOverride = "Supplemental Heating Coil Nominal Capacity [W]";
                if (state.dataGlobal->isEpJSON) {
                    stringOverride = "supplemental_heating_coil_nominal_capacity [W]";
                }
                sizerWaterHeatingCapacity.overrideSizingString(stringOverride);
                sizerWaterHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSuppHeatingCapacity = sizerWaterHeatingCapacity.size(state, TempSize, ErrorsFound);
            }
        } else {
            SizingString = "Supplemental Heating Coil Nominal Capacity [W]";
            if (TempSize == DataSizing::AutoSize) {
                bool errorsFound = false;
                HeatingCapacitySizer sizerHeatingCapacity;
                sizerHeatingCapacity.overrideSizingString(SizingString);
                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).DesignSuppHeatingCapacity = sizerHeatingCapacity.size(state, TempSize, errorsFound);
            }
        }
    }

    EqSizing.CoolingAirFlow = true;
    EqSizing.CoolingAirVolFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxCoolAirVolFlow;
    EqSizing.HeatingAirFlow = true;
    EqSizing.HeatingAirVolFlow = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).MaxHeatAirVolFlow;

    if (CheckVRFCombinationRatio(VRFCond)) {
        OnOffAirFlowRat = 1.0;
        // set up the outside air data for sizing the DX coils
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInZone) {
            state.dataSize->ZoneEqDXCoil = true;
        }
        if (state.dataSize->CurZoneEqNum > 0) {
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow > 0.0 ||
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow > 0.0) {
                EqSizing.OAVolFlow =
                    max(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolOutAirVolFlow, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatOutAirVolFlow);
            } else {
                EqSizing.OAVolFlow = 0.0;
            }
        } else {
            EqSizing.OAVolFlow = 0.0;
        }

        Real64 SuppHeatCoilLoad = 0.0;
        // simulate the TU to size the coils
        if (state.dataHVACVarRefFlow->VRF(VRFCond).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
            // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                state, VRFTUNum, true, 0.0, TUCoolingCapacity, OnOffAirFlowRat, SuppHeatCoilLoad);
        } else {
            // Algorithm Type: VRF model based on system curve
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF(state, VRFTUNum, true, 0.0, TUCoolingCapacity, OnOffAirFlowRat, SuppHeatCoilLoad);
        }

        //    ZoneEqDXCoil = .FALSE.
        TUCoolingCapacity = 0.0;
        TUHeatingCapacity = 0.0;
        bool FoundAll = true;
        bool errFlag; // temporary variable used for error checking
        int TUListNum = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TUListIndex;
        for (int NumTU = 1; NumTU <= state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).NumTUInList; ++NumTU) {
            int TUIndex = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
            if (state.dataHVACVarRefFlow->VRFTU(TUIndex).CoolCoilIndex > 0) {
                DXCoilCap = DXCoils::GetCoilCapacityByIndexType(state,
                                                                state.dataHVACVarRefFlow->VRFTU(TUIndex).CoolCoilIndex,
                                                                state.dataHVACVarRefFlow->VRFTU(TUIndex).DXCoolCoilType_Num,
                                                                errFlag);
                TUCoolingCapacity += DXCoilCap;
                if (DXCoilCap == AutoSize) {
                    FoundAll = false;
                    break;
                }
            }
            if (state.dataHVACVarRefFlow->VRFTU(TUIndex).HeatCoilIndex > 0) {
                DXCoilCap = DXCoils::GetCoilCapacityByIndexType(state,
                                                                state.dataHVACVarRefFlow->VRFTU(TUIndex).HeatCoilIndex,
                                                                state.dataHVACVarRefFlow->VRFTU(TUIndex).DXHeatCoilType_Num,
                                                                errFlag);
                TUHeatingCapacity += DXCoilCap;
                if (DXCoilCap == AutoSize) {
                    FoundAll = false;
                    break;
                }
            }
        }

        if (FoundAll && (state.dataHVACVarRefFlow->VRF(VRFCond).VRFAlgorithmType == AlgorithmType::SysCurve)) {
            // Size VRF rated cooling/heating capacity (VRF-SysCurve Model)

            // Size VRF( VRFCond ).CoolingCapacity
            IsAutoSize = false;
            if (state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCapacity == AutoSize) {
                IsAutoSize = true;
            }
            CoolingCapacityDes = TUCoolingCapacity;
            if (IsAutoSize) {
                state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCapacity = CoolingCapacityDes;
                BaseSizer::reportSizerOutput(state,
                                             std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)),
                                             state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                                             "Design Size Rated Total Cooling Capacity (gross) [W]",
                                             CoolingCapacityDes);
            } else {
                if (state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCapacity > 0.0 && CoolingCapacityDes > 0.0) {
                    CoolingCapacityUser = state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCapacity;
                    BaseSizer::reportSizerOutput(state,
                                                 std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)),
                                                 state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                                                 "Design Size Rated Total Cooling Capacity (gross) [W]",
                                                 CoolingCapacityDes,
                                                 "User-Specified Rated Total Cooling Capacity (gross) [W]",
                                                 CoolingCapacityUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(CoolingCapacityDes - CoolingCapacityUser) / CoolingCapacityUser) > state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state,
                                        format("SizeVRF: Potential issue with equipment sizing for {} {}",
                                               cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum),
                                               state.dataHVACVarRefFlow->VRFTU(VRFCond).Name));
                            ShowContinueError(state,
                                              format("User-Specified Rated Total Cooling Capacity (gross) of {:.2R} [W]", CoolingCapacityUser));
                            ShowContinueError(
                                state, format("differs from Design Size Rated Total Cooling Capacity (gross) of {:.2R} [W]", CoolingCapacityDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }

            if (state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCapacity > 0.0) {
                state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCombinationRatio =
                    TUCoolingCapacity / state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCapacity;
            }

            // Size VRF( VRFCond ).HeatingCapacity
            IsAutoSize = false;
            if (state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCapacity == AutoSize) {
                IsAutoSize = true;
            }
            if (state.dataHVACVarRefFlow->VRF(VRFCond).LockHeatingCapacity) {
                HeatingCapacityDes =
                    state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCapacity * state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCapacitySizeRatio;
            } else {
                HeatingCapacityDes = TUHeatingCapacity;
            }
            if (IsAutoSize) {
                state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCapacity = HeatingCapacityDes;
                BaseSizer::reportSizerOutput(state,
                                             std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)),
                                             state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                                             "Design Size Rated Total Heating Capacity [W]",
                                             HeatingCapacityDes);
            } else {
                if (state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCapacity > 0.0 && HeatingCapacityDes > 0.0) {
                    HeatingCapacityUser = state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCapacity;
                    BaseSizer::reportSizerOutput(state,
                                                 std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)),
                                                 state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                                                 "Design Size Rated Total Heating Capacity [W]",
                                                 HeatingCapacityDes,
                                                 "User-Specified Rated Total Heating Capacity [W]",
                                                 HeatingCapacityUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(HeatingCapacityDes - HeatingCapacityUser) / HeatingCapacityUser) > state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state,
                                        format("SizeVRF: Potential issue with equipment sizing for {} {}",
                                               cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum),
                                               state.dataHVACVarRefFlow->VRFTU(VRFCond).Name));
                            ShowContinueError(state, format("User-Specified Rated Total Heating Capacity of {:.2R} [W]", HeatingCapacityUser));
                            ShowContinueError(state,
                                              format("differs from Design Size Rated Total Heating Capacity of {:.2R} [W]", HeatingCapacityDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }

            if (state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCapacity > 0.0) {
                state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCombinationRatio =
                    TUHeatingCapacity / state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCapacity;
            }

            // calculate the piping correction factors only once
            if (state.dataHVACVarRefFlow->VRF(VRFCond).PCFLengthCoolPtr > 0) {
                {
                    if (state.dataCurveManager->curves(state.dataHVACVarRefFlow->VRF(VRFCond).PCFLengthCoolPtr)->numDims == 2) {
                        state.dataHVACVarRefFlow->VRF(VRFCond).PipingCorrectionCooling =
                            min(1.0,
                                max(0.5,
                                    CurveValue(state,
                                               state.dataHVACVarRefFlow->VRF(VRFCond).PCFLengthCoolPtr,
                                               state.dataHVACVarRefFlow->VRF(VRFCond).EquivPipeLngthCool,
                                               state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCombinationRatio) +
                                        state.dataHVACVarRefFlow->VRF(VRFCond).VertPipeLngth * state.dataHVACVarRefFlow->VRF(VRFCond).PCFHeightCool));
                    } else {
                        state.dataHVACVarRefFlow->VRF(VRFCond).PipingCorrectionCooling =
                            min(1.0,
                                max(0.5,
                                    CurveValue(state,
                                               state.dataHVACVarRefFlow->VRF(VRFCond).PCFLengthCoolPtr,
                                               state.dataHVACVarRefFlow->VRF(VRFCond).EquivPipeLngthCool) +
                                        state.dataHVACVarRefFlow->VRF(VRFCond).VertPipeLngth * state.dataHVACVarRefFlow->VRF(VRFCond).PCFHeightCool));
                    }
                }
            } else {
                state.dataHVACVarRefFlow->VRF(VRFCond).PipingCorrectionCooling = min(
                    1.0,
                    max(0.5, (1.0 + state.dataHVACVarRefFlow->VRF(VRFCond).VertPipeLngth * state.dataHVACVarRefFlow->VRF(VRFCond).PCFHeightCool)));
            }

            if (state.dataHVACVarRefFlow->VRF(VRFCond).PCFLengthHeatPtr > 0) {
                {
                    if (state.dataCurveManager->curves(state.dataHVACVarRefFlow->VRF(VRFCond).PCFLengthHeatPtr)->numDims == 2) {
                        state.dataHVACVarRefFlow->VRF(VRFCond).PipingCorrectionHeating =
                            min(1.0,
                                max(0.5,
                                    CurveValue(state,
                                               state.dataHVACVarRefFlow->VRF(VRFCond).PCFLengthHeatPtr,
                                               state.dataHVACVarRefFlow->VRF(VRFCond).EquivPipeLngthHeat,
                                               state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCombinationRatio) +
                                        state.dataHVACVarRefFlow->VRF(VRFCond).VertPipeLngth * state.dataHVACVarRefFlow->VRF(VRFCond).PCFHeightHeat));
                    } else {
                        state.dataHVACVarRefFlow->VRF(VRFCond).PipingCorrectionHeating =
                            min(1.0,
                                max(0.5,
                                    CurveValue(state,
                                               state.dataHVACVarRefFlow->VRF(VRFCond).PCFLengthHeatPtr,
                                               state.dataHVACVarRefFlow->VRF(VRFCond).EquivPipeLngthHeat) +
                                        state.dataHVACVarRefFlow->VRF(VRFCond).VertPipeLngth * state.dataHVACVarRefFlow->VRF(VRFCond).PCFHeightHeat));
                    }
                }
            } else {
                state.dataHVACVarRefFlow->VRF(VRFCond).PipingCorrectionHeating = min(
                    1.0,
                    max(0.5, (1.0 + state.dataHVACVarRefFlow->VRF(VRFCond).VertPipeLngth * state.dataHVACVarRefFlow->VRF(VRFCond).PCFHeightHeat)));
            }

            state.dataHVACVarRefFlow->VRF(VRFCond).RatedCoolingPower =
                state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCapacity / state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCOP;
            state.dataHVACVarRefFlow->VRF(VRFCond).RatedHeatingPower =
                state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCapacity / state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCOP;

            if (state.dataHVACVarRefFlow->VRF(VRFCond).CoolCombRatioPTR > 0) {
                state.dataHVACVarRefFlow->CoolCombinationRatio(VRFCond) = CurveValue(
                    state, state.dataHVACVarRefFlow->VRF(VRFCond).CoolCombRatioPTR, state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCombinationRatio);
            } else {
                state.dataHVACVarRefFlow->CoolCombinationRatio(VRFCond) = 1.0;
            }

            if (state.dataHVACVarRefFlow->VRF(VRFCond).HeatCombRatioPTR > 0) {
                state.dataHVACVarRefFlow->HeatCombinationRatio(VRFCond) = CurveValue(
                    state, state.dataHVACVarRefFlow->VRF(VRFCond).HeatCombRatioPTR, state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCombinationRatio);
            } else {
                state.dataHVACVarRefFlow->HeatCombinationRatio(VRFCond) = 1.0;
            }
        }

        if (FoundAll && (state.dataHVACVarRefFlow->VRF(VRFCond).VRFAlgorithmType == AlgorithmType::FluidTCtrl)) {
            // Size VRF rated evaporative capacity (VRF-FluidTCtrl Model)
            // Set piping correction factors to 1.0 here for reporting to eio output - recalculated every time step in
            // VRFCondenserEquipment::CalcVRFCondenser_FluidTCtrl
            state.dataHVACVarRefFlow->VRF(VRFCond).PipingCorrectionCooling = 1.0;
            state.dataHVACVarRefFlow->VRF(VRFCond).PipingCorrectionHeating = 1.0;

            // Size VRF( VRFCond ).RatedEvapCapacity
            IsAutoSize = false;
            if (state.dataHVACVarRefFlow->VRF(VRFCond).RatedEvapCapacity == AutoSize) {
                IsAutoSize = true;
            }

            CoolingCapacityDes = TUCoolingCapacity;
            HeatingCapacityDes = TUHeatingCapacity;

            if (IsAutoSize) {
                // RatedEvapCapacity
                state.dataHVACVarRefFlow->VRF(VRFCond).RatedEvapCapacity =
                    max(CoolingCapacityDes, HeatingCapacityDes / (1 + state.dataHVACVarRefFlow->VRF(VRFCond).RatedCompPowerPerCapcity));

                // Other parameters dependent on RatedEvapCapacity
                state.dataHVACVarRefFlow->VRF(VRFCond).RatedCompPower =
                    state.dataHVACVarRefFlow->VRF(VRFCond).RatedCompPowerPerCapcity * state.dataHVACVarRefFlow->VRF(VRFCond).RatedEvapCapacity;
                state.dataHVACVarRefFlow->VRF(VRFCond).RatedOUFanPower =
                    state.dataHVACVarRefFlow->VRF(VRFCond).RatedOUFanPowerPerCapcity * state.dataHVACVarRefFlow->VRF(VRFCond).RatedEvapCapacity;
                state.dataHVACVarRefFlow->VRF(VRFCond).OUAirFlowRate =
                    state.dataHVACVarRefFlow->VRF(VRFCond).OUAirFlowRatePerCapcity * state.dataHVACVarRefFlow->VRF(VRFCond).RatedEvapCapacity;

                state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCapacity = state.dataHVACVarRefFlow->VRF(VRFCond).RatedEvapCapacity;
                state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCapacity =
                    state.dataHVACVarRefFlow->VRF(VRFCond).RatedEvapCapacity * (1 + state.dataHVACVarRefFlow->VRF(VRFCond).RatedCompPowerPerCapcity);

                BaseSizer::reportSizerOutput(state,
                                             std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)),
                                             state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                                             "Design Size Rated Total Heating Capacity [W]",
                                             state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCapacity);
                BaseSizer::reportSizerOutput(state,
                                             std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)),
                                             state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                                             "Design Size Rated Total Cooling Capacity (gross) [W]",
                                             state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCapacity);
            } else {
                CoolingCapacityUser = state.dataHVACVarRefFlow->VRF(VRFCond).RatedEvapCapacity;
                HeatingCapacityUser = state.dataHVACVarRefFlow->VRF(VRFCond).RatedHeatCapacity;

                BaseSizer::reportSizerOutput(state,
                                             std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)),
                                             state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                                             "Design Size Rated Total Cooling Capacity (gross) [W]",
                                             CoolingCapacityDes,
                                             "User-Specified Rated Total Cooling Capacity (gross) [W]",
                                             CoolingCapacityUser);
                BaseSizer::reportSizerOutput(state,
                                             std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)),
                                             state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                                             "Design Size Rated Total Heating Capacity [W]",
                                             HeatingCapacityDes,
                                             "User-Specified Rated Total Heating Capacity [W]",
                                             HeatingCapacityUser);

                if (state.dataGlobal->DisplayExtraWarnings) {
                    if ((std::abs(CoolingCapacityDes - CoolingCapacityUser) / CoolingCapacityUser) > state.dataSize->AutoVsHardSizingThreshold) {
                        ShowMessage(state,
                                    format("SizeVRF: Potential issue with equipment sizing for {} {}",
                                           cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum),
                                           state.dataHVACVarRefFlow->VRFTU(VRFCond).Name));
                        ShowContinueError(state, format("User-Specified Rated Total Cooling Capacity (gross) of {:.2R} [W]", CoolingCapacityUser));
                        ShowContinueError(state,
                                          format("differs from Design Size Rated Total Cooling Capacity (gross) of {:.2R} [W]", CoolingCapacityDes));
                        ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                        ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                    }

                    if ((std::abs(HeatingCapacityDes - HeatingCapacityUser) / HeatingCapacityUser) > state.dataSize->AutoVsHardSizingThreshold) {
                        ShowMessage(state,
                                    format("SizeVRF: Potential issue with equipment sizing for {} {}",
                                           cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum),
                                           state.dataHVACVarRefFlow->VRFTU(VRFCond).Name));
                        ShowContinueError(state, format("User-Specified Rated Total Heating Capacity of {:.2R} [W]", HeatingCapacityUser));
                        ShowContinueError(state, format("differs from Design Size Rated Total Heating Capacity of {:.2R} [W]", HeatingCapacityDes));
                        ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                        ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                    }
                }
            }
        }

        if (FoundAll) {
            // autosize resistive defrost heater capacity
            IsAutoSize = false;
            if (state.dataHVACVarRefFlow->VRF(VRFCond).DefrostCapacity == AutoSize) {
                IsAutoSize = true;
            }
            if (state.dataHVACVarRefFlow->VRF(VRFCond).DefrostStrategy == StandardRatings::DefrostStrat::Resistive) {
                DefrostCapacityDes = state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCapacity;
            } else {
                DefrostCapacityDes = 0.0;
            }
            if (IsAutoSize) {
                state.dataHVACVarRefFlow->VRF(VRFCond).DefrostCapacity = DefrostCapacityDes;
                BaseSizer::reportSizerOutput(state,
                                             std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)),
                                             state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                                             "Design Size Resistive Defrost Heater Capacity [W]",
                                             DefrostCapacityDes);
            } else {
                if (state.dataHVACVarRefFlow->VRF(VRFCond).DefrostCapacity > 0.0 && DefrostCapacityDes > 0.0) {
                    DefrostCapacityUser = state.dataHVACVarRefFlow->VRF(VRFCond).DefrostCapacity;
                    BaseSizer::reportSizerOutput(state,
                                                 std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)),
                                                 state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                                                 "Design Size Resistive Defrost Heater Capacity [W]",
                                                 DefrostCapacityDes,
                                                 "User-Specified Resistive Defrost Heater Capacity",
                                                 DefrostCapacityUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(DefrostCapacityDes - DefrostCapacityUser) / DefrostCapacityUser) > state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state,
                                        format("SizeVRF: Potential issue with equipment sizing for {} {}",
                                               cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum),
                                               state.dataHVACVarRefFlow->VRFTU(VRFCond).Name));
                            ShowContinueError(state, format("User-Specified Resistive Defrost Heater Capacity of {:.2R} [W]", DefrostCapacityUser));
                            ShowContinueError(state,
                                              format("differs from Design Size Resistive Defrost Heater Capacity of {:.2R} [W]", DefrostCapacityDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }

            IsAutoSize = false;
            if (state.dataHVACVarRefFlow->VRF(VRFCond).EvapCondAirVolFlowRate == AutoSize) {
                IsAutoSize = true;
            }
            // Auto-size condenser air flow to Total Capacity * 0.000114 m3/s/w (850 cfm/ton)
            EvapCondAirVolFlowRateDes = state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCapacity * 0.000114;
            if (IsAutoSize) {
                state.dataHVACVarRefFlow->VRF(VRFCond).EvapCondAirVolFlowRate = EvapCondAirVolFlowRateDes;
                BaseSizer::reportSizerOutput(state,
                                             std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)),
                                             state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                                             "Design Size Evaporative Condenser Air Flow Rate [m3/s]",
                                             EvapCondAirVolFlowRateDes);
            } else {
                if (state.dataHVACVarRefFlow->VRF(VRFCond).EvapCondAirVolFlowRate > 0.0 && EvapCondAirVolFlowRateDes > 0.0) {
                    EvapCondAirVolFlowRateUser = state.dataHVACVarRefFlow->VRF(VRFCond).EvapCondAirVolFlowRate;
                    BaseSizer::reportSizerOutput(state,
                                                 std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)),
                                                 state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                                                 "Design Size Evaporative Condenser Air Flow Rate [m3/s]",
                                                 EvapCondAirVolFlowRateDes,
                                                 "User-Specified Evaporative Condenser Air Flow Rate [m3/s]",
                                                 EvapCondAirVolFlowRateUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(EvapCondAirVolFlowRateDes - EvapCondAirVolFlowRateUser) / EvapCondAirVolFlowRateUser) >
                            state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state,
                                        format("SizeVRF: Potential issue with equipment sizing for {} {}",
                                               cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum),
                                               state.dataHVACVarRefFlow->VRFTU(VRFCond).Name));
                            ShowContinueError(
                                state, format("User-Specified Evaporative Condenser Air Flow Rate of {:.5R} [m3/s]", EvapCondAirVolFlowRateUser));
                            ShowContinueError(
                                state,
                                format("differs from Design Size Evaporative Condenser Air Flow Rate of {:.5R} [m3/s]", EvapCondAirVolFlowRateDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }

            IsAutoSize = false;
            if (state.dataHVACVarRefFlow->VRF(VRFCond).EvapCondPumpPower == AutoSize) {
                IsAutoSize = true;
            }
            // Auto-size evap condenser pump power to Total Capacity * 0.004266 w/w (15 w/ton)
            EvapCondPumpPowerDes = state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCapacity * 0.004266;
            if (IsAutoSize) {
                state.dataHVACVarRefFlow->VRF(VRFCond).EvapCondPumpPower = EvapCondPumpPowerDes;
                BaseSizer::reportSizerOutput(state,
                                             std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)),
                                             state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                                             "Design Size Evaporative Condenser Pump Rated Power Consumption [W]",
                                             EvapCondPumpPowerDes);

            } else {
                if (state.dataHVACVarRefFlow->VRF(VRFCond).EvapCondPumpPower > 0.0 && EvapCondPumpPowerDes > 0.0) {
                    EvapCondPumpPowerUser = state.dataHVACVarRefFlow->VRF(VRFCond).EvapCondPumpPower;
                    BaseSizer::reportSizerOutput(state,
                                                 std::string(cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum)),
                                                 state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                                                 "Design Size Evaporative Condenser Pump Rated Power Consumption [W]",
                                                 EvapCondPumpPowerDes,
                                                 "User-Specified Evaporative Condenser Pump Rated Power Consumption [W]",
                                                 EvapCondPumpPowerUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(EvapCondPumpPowerDes - EvapCondPumpPowerUser) / EvapCondPumpPowerUser) >
                            state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state,
                                        format("SizeVRF: Potential issue with equipment sizing for {} {}",
                                               cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum),
                                               state.dataHVACVarRefFlow->VRFTU(VRFCond).Name));
                            ShowContinueError(
                                state,
                                format("User-Specified Evaporative Condenser Pump Rated Power Consumption of {:.2R} [W]", EvapCondPumpPowerUser));
                            ShowContinueError(state,
                                              format("differs from Design Size Evaporative Condenser Pump Rated Power Consumption of {:.2R} [W]",
                                                     EvapCondPumpPowerDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }

            // Report to eio other information not related to autosizing
            if (state.dataHVACVarRefFlow->MyOneTimeEIOFlag) {
                static constexpr std::string_view Format_990(
                    "! <VRF System Information>, VRF System Type, VRF System Name, VRF System Cooling Combination Ratio, VRF "
                    "System Heating Combination Ratio, VRF System Cooling Piping Correction Factor, VRF System Heating Piping "
                    "Correction Factor\n");
                print(state.files.eio, Format_990);
                state.dataHVACVarRefFlow->MyOneTimeEIOFlag = false;
            }
            static constexpr std::string_view Format_991(" VRF System Information, {}, {}, {:.5R}, {:.5R}, {:.5R}, {:.5R}\n");
            print(state.files.eio,
                  Format_991,
                  cVRFTypes(state.dataHVACVarRefFlow->VRF(VRFCond).VRFSystemTypeNum),
                  state.dataHVACVarRefFlow->VRF(VRFCond).Name,
                  state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCombinationRatio,
                  state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCombinationRatio,
                  state.dataHVACVarRefFlow->VRF(VRFCond).PipingCorrectionCooling,
                  state.dataHVACVarRefFlow->VRF(VRFCond).PipingCorrectionHeating);

            CheckVRFCombinationRatio(VRFCond) = false;
        }
    }

    state.dataSize->DataScalableCapSizingON = false;
}

void initCapSizingVars(EnergyPlusData &state,
                       int sizingMethod,
                       int capSizingMethod,
                       int &eqSizingMethod,
                       Real64 scaledCapacity,
                       bool &modeCapacity,
                       Real64 &designLoad,
                       bool &scalableCapSizingOn,
                       Real64 &fracOfAutosizedCapacity)
{
    using namespace DataSizing;
    using HVAC::CoolingCapacitySizing;
    using HVAC::HeatingCapacitySizing;

    eqSizingMethod = capSizingMethod;
    if (capSizingMethod == CoolingDesignCapacity || capSizingMethod == HeatingDesignCapacity || capSizingMethod == CapacityPerFloorArea ||
        capSizingMethod == FractionOfAutosizedCoolingCapacity || capSizingMethod == FractionOfAutosizedHeatingCapacity) {
        if ((capSizingMethod == CoolingDesignCapacity && sizingMethod == CoolingCapacitySizing) ||
            (capSizingMethod == HeatingDesignCapacity && sizingMethod == HeatingCapacitySizing)) {
            if (scaledCapacity > 0.0) {
                modeCapacity = true;
                designLoad = scaledCapacity;
            }
        } else if (capSizingMethod == CapacityPerFloorArea) {
            modeCapacity = true;
            designLoad = scaledCapacity * state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
            scalableCapSizingOn = true;
        } else if ((capSizingMethod == FractionOfAutosizedCoolingCapacity && sizingMethod == CoolingCapacitySizing) ||
                   (capSizingMethod == FractionOfAutosizedHeatingCapacity && sizingMethod == HeatingCapacitySizing)) {
            fracOfAutosizedCapacity = scaledCapacity;
            scalableCapSizingOn = true;
        }
    }
}

void VRFCondenserEquipment::SizeVRFCondenser(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   August 2012
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for sizing VRF Condenser.

    // METHODOLOGY EMPLOYED:
    // Set water-cooled plant flow rates.

    static constexpr std::string_view RoutineName("SizeVRFCondenser");

    Real64 rho;                // local fluid density [kg/m3]
    Real64 Cp;                 // local fluid specific heat [J/kg-k]
    Real64 tmpCondVolFlowRate; // local condenser design volume flow rate [m3/s]

    // save the design water flow rate for use by the water loop sizing algorithms
    if (this->CondenserType == DataHeatBalance::RefrigCondenserType::Water) {

        bool ErrorsFound = false;
        if (this->WaterCondVolFlowRate == DataSizing::AutoSize) {
            int PltSizCondNum = 0;
            if (this->SourcePlantLoc.loopNum > 0) {
                PltSizCondNum = state.dataPlnt->PlantLoop(this->SourcePlantLoc.loopNum).PlantSizNum;
            }
            if (PltSizCondNum > 0) {
                rho = state.dataPlnt->PlantLoop(this->SourcePlantLoc.loopNum)
                          .glycol->getDensity(state, state.dataSize->PlantSizData(PltSizCondNum).ExitTemp, RoutineName);

                Cp = state.dataPlnt->PlantLoop(this->SourcePlantLoc.loopNum)
                         .glycol->getSpecificHeat(state, state.dataSize->PlantSizData(PltSizCondNum).ExitTemp, RoutineName);
                tmpCondVolFlowRate =
                    max(this->CoolingCapacity, this->HeatingCapacity) / (state.dataSize->PlantSizData(PltSizCondNum).DeltaT * Cp * rho);
                if (this->HeatingCapacity != DataSizing::AutoSize && this->CoolingCapacity != DataSizing::AutoSize) {
                    this->WaterCondVolFlowRate = tmpCondVolFlowRate;
                    BaseSizer::reportSizerOutput(state,
                                                 "AirConditioner:VariableRefrigerantFlow",
                                                 this->Name,
                                                 "Design Condenser Water Flow Rate [m3/s]",
                                                 this->WaterCondVolFlowRate);
                }

                rho = state.dataPlnt->PlantLoop(this->SourcePlantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
                this->WaterCondenserDesignMassFlow = this->WaterCondVolFlowRate * rho;
                PlantUtilities::InitComponentNodes(
                    state, 0.0, this->WaterCondenserDesignMassFlow, this->CondenserNodeNum, this->CondenserOutletNodeNum);

            } else {
                ShowSevereError(state, "Autosizing of condenser water flow rate requires a condenser loop Sizing:Plant object");
                ShowContinueError(state, format("... occurs in AirConditioner:VariableRefrigerantFlow object={}", this->Name));
                ShowContinueError(state, "... plant loop name must be referenced in Sizing:Plant object");
                ErrorsFound = true;
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Preceding sizing errors cause program termination");
        }

        PlantUtilities::RegisterPlantCompDesignFlow(state, this->CondenserNodeNum, this->WaterCondVolFlowRate);
    }
}

void SimVRF(EnergyPlusData &state,
            int const VRFTUNum,
            bool const FirstHVACIteration,
            Real64 &OnOffAirFlowRatio,
            Real64 &SysOutputProvided,
            Real64 &LatOutputProvided,
            Real64 const QZnReq)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   August 2010
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine simulates the VRF TU's.

    // METHODOLOGY EMPLOYED:
    // Simulate terminal unit to meet zone load.

    Real64 PartLoadRatio(1.0);
    Real64 SuppHeatCoilLoad(0.0); // supplemental heating coil load (W)

    if (state.dataHVACVarRefFlow->VRF(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFSysNum).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ControlVRF_FluidTCtrl(
            state, VRFTUNum, QZnReq, FirstHVACIteration, PartLoadRatio, OnOffAirFlowRatio, SuppHeatCoilLoad);
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
            state, VRFTUNum, FirstHVACIteration, PartLoadRatio, SysOutputProvided, OnOffAirFlowRatio, SuppHeatCoilLoad, LatOutputProvided);
        if (PartLoadRatio ==
            0.0) { // set coil inlet conditions when coil does not operate. Inlet conditions are set in ControlVRF_FluidTCtrl when PLR=1
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolingCoilPresent) {
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).coilInNodeT =
                    state.dataLoopNodes->Node(state.dataDXCoils->DXCoil(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolCoilIndex).AirInNode).Temp;
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).coilInNodeW =
                    state.dataLoopNodes->Node(state.dataDXCoils->DXCoil(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolCoilIndex).AirInNode).HumRat;
            } else {
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).coilInNodeT =
                    state.dataLoopNodes->Node(state.dataDXCoils->DXCoil(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatCoilIndex).AirInNode).Temp;
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).coilInNodeW =
                    state.dataLoopNodes->Node(state.dataDXCoils->DXCoil(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatCoilIndex).AirInNode).HumRat;
            }
        }
        // CalcVRF( VRFTUNum, FirstHVACIteration, PartLoadRatio, SysOutputProvided, OnOffAirFlowRatio, LatOutputProvided );
    } else {
        // Algorithm Type: VRF model based on system curve
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ControlVRF(
            state, VRFTUNum, QZnReq, FirstHVACIteration, PartLoadRatio, OnOffAirFlowRatio, SuppHeatCoilLoad);
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF(
            state, VRFTUNum, FirstHVACIteration, PartLoadRatio, SysOutputProvided, OnOffAirFlowRatio, SuppHeatCoilLoad, LatOutputProvided);
    }

    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TerminalUnitSensibleRate = SysOutputProvided;
    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TerminalUnitLatentRate = LatOutputProvided;
}

void VRFTerminalUnitEquipment::ControlVRF(EnergyPlusData &state,
                                          int const VRFTUNum,            // Index to VRF terminal unit
                                          Real64 const QZnReq,           // Index to zone number
                                          bool const FirstHVACIteration, // flag for 1st HVAC iteration in the time step
                                          Real64 &PartLoadRatio,         // unit part load ratio
                                          Real64 &OnOffAirFlowRatio,     // ratio of compressor ON airflow to AVERAGE airflow over timestep
                                          Real64 &SuppHeatCoilLoad       // supplemental heating coil load (W)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   July 2005

    // PURPOSE OF THIS SUBROUTINE:
    // Determine the part load fraction of the heat pump for this time step.

    // METHODOLOGY EMPLOYED:
    // Use RegulaFalsi technique to iterate on part-load ratio until convergence is achieved.

    PartLoadRatio = 0.0;
    state.dataHVACVarRefFlow->LoopDXCoolCoilRTF = 0.0;
    state.dataHVACVarRefFlow->LoopDXHeatCoilRTF = 0.0;
    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatPartLoadRatio = 0.0;

    // The RETURNS here will jump back to SimVRF where the CalcVRF routine will simulate with latest PLR

    // do nothing else if TU is scheduled off
    if (this->availSched->getCurrentVal() == 0.0) {
        return;
    }

    // do nothing if TU has no load (TU will be modeled using PLR=0)
    if (QZnReq == 0.0) {
        return;
    }

    // Set EMS value for PLR and return
    if (this->EMSOverridePartLoadFrac) {
        PartLoadRatio = this->EMSValueForPartLoadFrac;
        return;
    }

    // Get result when DX coil is operating at the minimum PLR (1E-20) if not otherwise specified
    PartLoadRatio = this->MinOperatingPLR;

    this->ControlVRFToLoad(state, VRFTUNum, QZnReq, FirstHVACIteration, PartLoadRatio, OnOffAirFlowRatio, SuppHeatCoilLoad);
}

void VRFTerminalUnitEquipment::ControlVRFToLoad(EnergyPlusData &state,
                                                int const VRFTUNum,
                                                Real64 const QZnReq,
                                                bool const FirstHVACIteration,
                                                Real64 &PartLoadRatio,
                                                Real64 &OnOffAirFlowRatio,
                                                Real64 &SuppHeatCoilLoad)
{

    int constexpr MaxIte(500);        // maximum number of iterations
    Real64 constexpr MinPLF(0.0);     // minimum part load factor allowed
    Real64 constexpr ErrorTol(0.001); // tolerance for RegulaFalsi iterations

    int VRFCond = this->VRFSysNum;
    Real64 FullOutput = 0.0;   // unit full output when compressor is operating [W]
    Real64 TempOutput = 0.0;   // unit output when iteration limit exceeded [W]
    Real64 TempMinPLR = 0.0;   // min PLR used in Regula Falsi call
    Real64 TempMaxPLR = 0.0;   // max PLR used in Regula Falsi call
    Real64 NoCompOutput = 0.0; // output when no active compressor [W]
    bool VRFCoolingMode = state.dataHVACVarRefFlow->CoolingLoad(VRFCond);
    bool VRFHeatingMode = state.dataHVACVarRefFlow->HeatingLoad(VRFCond);
    int IndexToTUInTUList = this->IndexToTUInTUList;
    auto &thisVRFCond = state.dataHVACVarRefFlow->VRF(VRFCond);
    int TUListIndex = thisVRFCond.ZoneTUListPtr;
    bool HRCoolingMode = state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList);
    bool HRHeatingMode = state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList);
    auto &thisVRFTU = state.dataHVACVarRefFlow->VRFTU(VRFTUNum);

    if (thisVRFCond.VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
        this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, PartLoadRatio, NoCompOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
    } else {
        // Algorithm Type: VRF model based on system curve
        this->CalcVRF(state, VRFTUNum, FirstHVACIteration, PartLoadRatio, NoCompOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
    }

    bool DXCoolingCoilOprCtrl = true;

    if (VRFCoolingMode && HRHeatingMode) {
        // IF the system is in cooling mode, but the terminal unit requests heating (heat recovery)
        if (NoCompOutput >= QZnReq) {
            PartLoadRatio = 0.0;
            return;
        }
    } else if (VRFHeatingMode && HRCoolingMode) {
        // IF the system is in heating mode, but the terminal unit requests cooling (heat recovery)
        if (NoCompOutput <= QZnReq) {
            PartLoadRatio = 0.0;
            return;
        }
    } else if (VRFCoolingMode || HRCoolingMode) {
        // IF the system is in cooling mode and/or the terminal unit requests cooling
        if (NoCompOutput <= QZnReq) {
            DXCoolingCoilOprCtrl = false;
            if (!this->SuppHeatingCoilPresent || HRCoolingMode) {
                PartLoadRatio = 0.0;
                return;
            }
        }
    } else if (VRFHeatingMode || HRHeatingMode) {
        // IF the system is in heating mode and/or the terminal unit requests heating
        if (NoCompOutput >= QZnReq) {
            PartLoadRatio = 0.0;
            return;
        }
    }

    // Otherwise the coil needs to turn on. Get full load result
    PartLoadRatio = 1.0;
    if (!DXCoolingCoilOprCtrl) {
        PartLoadRatio = 0.0;
    }
    if (thisVRFCond.VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
        this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, PartLoadRatio, FullOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
    } else {
        // Algorithm Type: VRF model based on system curve
        if (this->NumOfSpeedHeating > 0 && VRFHeatingMode) {
            this->SpeedNum = this->NumOfSpeedHeating;
            this->SpeedRatio = 1.0;
            this->CycRatio = 1.0;
        }
        if (this->NumOfSpeedCooling > 0 && VRFCoolingMode) {
            this->SpeedNum = this->NumOfSpeedCooling;
            this->SpeedRatio = 1.0;
            this->CycRatio = 1.0;
        }
        this->CalcVRF(state, VRFTUNum, FirstHVACIteration, PartLoadRatio, FullOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
    }

    // set supplemental heating coil calculation if the condition requires
    if (this->SuppHeatingCoilPresent) {
        if (this->isSetPointControlled) {
            auto const &thisSuppHeatCoilAirInletNode = state.dataLoopNodes->Node(this->SuppHeatCoilAirInletNode);
            if (this->suppTempSetPoint > thisSuppHeatCoilAirInletNode.Temp) {
                Real64 mDot = thisSuppHeatCoilAirInletNode.MassFlowRate;
                Real64 Tin = thisSuppHeatCoilAirInletNode.Temp;
                Real64 Win = thisSuppHeatCoilAirInletNode.HumRat;
                Real64 CpAirIn = Psychrometrics::PsyCpAirFnW(Win);
                SuppHeatCoilLoad = mDot * CpAirIn * (this->suppTempSetPoint - Tin);
                this->SuppHeatingCoilLoad = SuppHeatCoilLoad;
            } else {
                SuppHeatCoilLoad = 0.0;
            }
        } else {
            // not sure why FirstHVAC has anything to do with this but that was already here
            // another branch should test removing FirstHVACIteration to get same answer each iteration
            if (!FirstHVACIteration &&
                ((QZnReq > HVAC::SmallLoad && QZnReq > FullOutput) || (((QZnReq - NoCompOutput) > HVAC::SmallLoad) && QZnReq <= 0.0))) {
                Real64 ZoneLoad = 0.0;
                Real64 LoadToHeatingSP = 0.0;
                Real64 LoadToCoolingSP = 0.0;
                getVRFTUZoneLoad(state, VRFTUNum, ZoneLoad, LoadToHeatingSP, LoadToCoolingSP, false);
                if ((QZnReq - NoCompOutput) > HVAC::SmallLoad && QZnReq <= 0.0) {
                    if (LoadToHeatingSP < 0.0 && QZnReq == 0.0) {
                        SuppHeatCoilLoad = max(0.0, LoadToHeatingSP - FullOutput);
                    } else {
                        SuppHeatCoilLoad = max(0.0, QZnReq - FullOutput);
                    }
                } else if (FullOutput < (LoadToHeatingSP - HVAC::SmallLoad) && LoadToHeatingSP > 0.0) {
                    if (QZnReq > 0.0 && (NoCompOutput - QZnReq) >= HVAC::SmallLoad) {
                        SuppHeatCoilLoad = 0.0;
                    } else {
                        SuppHeatCoilLoad = max(0.0, LoadToHeatingSP - FullOutput);
                    }
                } else {
                    SuppHeatCoilLoad = 0.0;
                }
            } else {
                SuppHeatCoilLoad = 0.0;
            }
        }
        if (this->DesignSuppHeatingCapacity > 0.0) {
            this->SuppHeatPartLoadRatio = min(1.0, SuppHeatCoilLoad / this->DesignSuppHeatingCapacity);
        }
    } else { // does it matter what these are if there is no supp heater?
        SuppHeatCoilLoad = 0.0;
        this->SuppHeatPartLoadRatio = 0.0;
    }

    if ((VRFCoolingMode && !thisVRFCond.HeatRecoveryUsed) || (thisVRFCond.HeatRecoveryUsed && HRCoolingMode)) {
        // Since we are cooling, we expect FullOutput < NoCompOutput
        // If the QZnReq <= FullOutput the unit needs to run full out
        if (QZnReq <= FullOutput) {
            // if no coil present in terminal unit, no need to reset PLR?
            if (thisVRFTU.CoolingCoilPresent && DXCoolingCoilOprCtrl) {
                PartLoadRatio = 1.0;
                // the zone set point could be exceeded if set point control is used so protect against that
                if (this->isSetPointControlled) {
                    if (state.dataLoopNodes->Node(this->coolCoilAirOutNode).Temp > this->coilTempSetPoint) {
                        return;
                    }
                } else {
                    return;
                }
            } else {
                PartLoadRatio = 0.0;
                return;
            }
        }
    } else if ((VRFHeatingMode && !thisVRFCond.HeatRecoveryUsed) || (thisVRFCond.HeatRecoveryUsed && HRHeatingMode)) {
        // Since we are heating, we expect FullOutput > NoCompOutput
        // If the QZnReq >= FullOutput the unit needs to run full out
        if (QZnReq >= FullOutput) {
            // if no coil present in terminal unit, no need reset PLR?
            if (this->HeatingCoilPresent) {
                PartLoadRatio = 1.0;
                // the zone set point could be exceeded if set point control is used so protect against that
                if (this->isSetPointControlled) {
                    if (state.dataLoopNodes->Node(this->heatCoilAirOutNode).Temp < this->coilTempSetPoint) {
                        return;
                    }
                } else {
                    return;
                }
            } else {
                PartLoadRatio = 0.0;
                return;
            }
        }
    } else {
        // VRF terminal unit is off
        // shouldn't actually get here
        PartLoadRatio = 0.0;
        return;
    }

    // The coil will not operate at PLR=0 or PLR=1, calculate the operating part-load ratio

    if ((VRFHeatingMode || HRHeatingMode) || ((VRFCoolingMode && DXCoolingCoilOprCtrl) || HRCoolingMode)) {

        int NumOfSpeed = 1;
        if (this->NumOfSpeedHeating > 1 && ((VRFHeatingMode || HRHeatingMode))) {
            NumOfSpeed = this->NumOfSpeedHeating;
        }
        if (this->NumOfSpeedCooling > 1 && ((VRFCoolingMode || HRCoolingMode))) {
            NumOfSpeed = this->NumOfSpeedCooling;
        }

        for (int SpeedNum = 1; SpeedNum <= NumOfSpeed; ++SpeedNum) {

            if (NumOfSpeed > 1) {
                this->SpeedNum = SpeedNum;
                this->CalcVRF(state, VRFTUNum, FirstHVACIteration, 1.0, FullOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                if ((VRFHeatingMode || HRHeatingMode) && QZnReq >= FullOutput) {
                    continue;
                }
                if ((VRFCoolingMode || HRCoolingMode) && QZnReq <= FullOutput) {
                    continue;
                }
            }

            if (SpeedNum == 1) {
                this->SpeedRatio = 0.0;
            }
            int SolFla = 0; // Flag of RegulaFalsi solver
            auto f = [&state, VRFTUNum, FirstHVACIteration, QZnReq, OnOffAirFlowRatio](Real64 const PartLoadRatio) {
                Real64 QZnReqTemp = QZnReq; // denominator representing zone load (W)
                Real64 ActualOutput;        // delivered capacity of VRF terminal unit
                Real64 SuppHeatCoilLoad = 0.0;
                bool setPointControlled = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isSetPointControlled;
                Real64 nonConstOnOffAirFlowRatio = OnOffAirFlowRatio;

                if (state.dataHVACVarRefFlow->VRF(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFSysNum).VRFAlgorithmType ==
                    AlgorithmType::FluidTCtrl) {
                    // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                        state, VRFTUNum, FirstHVACIteration, PartLoadRatio, ActualOutput, nonConstOnOffAirFlowRatio, SuppHeatCoilLoad);
                } else {
                    // Algorithm Type: VRF model based on system curve
                    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF(
                        state, VRFTUNum, FirstHVACIteration, PartLoadRatio, ActualOutput, nonConstOnOffAirFlowRatio, SuppHeatCoilLoad);
                }

                if (setPointControlled) {
                    Real64 outletNodeT = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOutletNodeNum).Temp;
                    return (outletNodeT - state.dataHVACVarRefFlow->VRFTU(VRFTUNum).coilTempSetPoint);
                } else {
                    if (std::abs(QZnReq) < 100.0) {
                        QZnReqTemp = sign(100.0, QZnReq);
                    }
                    return (ActualOutput - QZnReq) / QZnReqTemp;
                }
            };
            General::SolveRoot(state, ErrorTol, MaxIte, SolFla, PartLoadRatio, f, 0.0, 1.0);
            if (SpeedNum == 1) {
                if (this->NumOfSpeedCooling > 1 || this->NumOfSpeedHeating > 1) {
                    this->CycRatio = PartLoadRatio;
                }
                this->SpeedRatio = 0.0;
                if (SolFla > 0 && PartLoadRatio <= 1.0) {
                    break;
                }
            } else {
                this->CycRatio = 1.0;
                this->SpeedRatio = PartLoadRatio;
                if (SolFla > 0 && PartLoadRatio <= 1.0) {
                    break;
                }
            }

            if (SolFla == -1) {
                //     Very low loads may not converge quickly. Tighten PLR boundary and try again.
                TempMaxPLR = -0.1;
                bool ContinueIter = true;
                while (ContinueIter && TempMaxPLR < 1.0) {
                    TempMaxPLR += 0.1;

                    if (thisVRFCond.VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                        this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, TempMaxPLR, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    } else {
                        // Algorithm Type: VRF model based on system curve
                        this->CalcVRF(state, VRFTUNum, FirstHVACIteration, TempMaxPLR, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    }

                    if (VRFHeatingMode && TempOutput > QZnReq) {
                        ContinueIter = false;
                    }
                    if (VRFCoolingMode && TempOutput < QZnReq) {
                        ContinueIter = false;
                    }
                }
                TempMinPLR = TempMaxPLR;
                ContinueIter = true;
                while (ContinueIter && TempMinPLR > 0.0) {
                    TempMaxPLR = TempMinPLR;
                    TempMinPLR -= 0.01;

                    if (thisVRFCond.VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                        this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, TempMinPLR, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    } else {
                        // Algorithm Type: VRF model based on system curve
                        this->CalcVRF(state, VRFTUNum, FirstHVACIteration, TempMinPLR, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    }

                    if (VRFHeatingMode && TempOutput < QZnReq) {
                        ContinueIter = false;
                    }
                    if (VRFCoolingMode && TempOutput > QZnReq) {
                        ContinueIter = false;
                    }
                }
                General::SolveRoot(state, ErrorTol, MaxIte, SolFla, PartLoadRatio, f, TempMinPLR, TempMaxPLR);
                if (SolFla == -1) {
                    if (!FirstHVACIteration && !state.dataGlobal->WarmupFlag) {
                        if (this->IterLimitExceeded == 0) {
                            ShowWarningMessage(state, format("{} \"{}\"", tuTypeNames[(int)this->type], this->Name));
                            ShowContinueError(
                                state,
                                format(" Iteration limit exceeded calculating terminal unit part-load ratio, maximum iterations = {}", MaxIte));
                            ShowContinueErrorTimeStamp(state, format(" Part-load ratio returned = {:.3R}", PartLoadRatio));

                            if (thisVRFCond.VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                                // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                                this->CalcVRF_FluidTCtrl(
                                    state, VRFTUNum, FirstHVACIteration, PartLoadRatio, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                            } else {
                                // Algorithm Type: VRF model based on system curve
                                this->CalcVRF(state, VRFTUNum, FirstHVACIteration, PartLoadRatio, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                            }

                            ShowContinueError(state, format(" Load requested = {:.5T}, Load delivered = {:.5T}", QZnReq, TempOutput));
                            ShowRecurringWarningErrorAtEnd(state,
                                                           format("{} \"{}\" -- Terminal unit Iteration limit exceeded error continues...",
                                                                  tuTypeNames[(int)this->type],
                                                                  this->Name),
                                                           this->IterLimitExceeded);
                        } else {
                            ShowRecurringWarningErrorAtEnd(state,
                                                           format("{} \"{}\" -- Terminal unit Iteration limit exceeded error continues...",
                                                                  tuTypeNames[(int)this->type],
                                                                  this->Name),
                                                           this->IterLimitExceeded);
                        }
                    }
                } else if (SolFla == -2) {
                    if (!FirstHVACIteration && !state.dataGlobal->WarmupFlag) {
                        if (thisVRFTU.FirstIterfailed == 0) {
                            ShowWarningMessage(state, format("{} \"{}\"", tuTypeNames[(int)this->type], this->Name));
                            ShowContinueError(state, "Terminal unit part-load ratio calculation failed: PLR limits of 0 to 1 exceeded");
                            ShowContinueError(state, "Please fill out a bug report and forward to the EnergyPlus support group.");
                            ShowContinueErrorTimeStamp(state, "");
                            ShowRecurringWarningErrorAtEnd(
                                state,
                                format("{} \"{}\" -- Terminal unit part-load ratio limits of 0 to 1 exceeded error continues...",
                                       tuTypeNames[(int)this->type],
                                       this->Name),
                                this->FirstIterfailed);
                        } else {
                            ShowRecurringWarningErrorAtEnd(
                                state,
                                format("{} \"{}\" -- Terminal unit part-load ratio limits of 0 to 1 exceeded error continues...",
                                       tuTypeNames[(int)this->type],
                                       this->Name),
                                thisVRFTU.FirstIterfailed);
                        }
                    }
                    PartLoadRatio = max(MinPLF, std::abs(QZnReq - NoCompOutput) / std::abs(FullOutput - NoCompOutput));
                }
            } else if (SolFla == -2) {
                if (!FirstHVACIteration && !state.dataGlobal->WarmupFlag) {
                    if (thisVRFTU.FirstIterfailed == 0) {
                        ShowWarningMessage(state, format("{} \"{}\"", tuTypeNames[(int)this->type], this->Name));

                        ShowContinueError(state, "Terminal unit part-load ratio calculation failed: PLR limits of 0 to 1 exceeded");
                        ShowContinueError(state, "Please fill out a bug report and forward to the EnergyPlus support group.");
                        ShowContinueErrorTimeStamp(state, "");
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            format("{} \"{}\" -- Terminal unit part-load ratio limits of 0 to 1 exceeded error continues...",
                                   tuTypeNames[(int)this->type],
                                   " \"" + this->Name),
                            this->FirstIterfailed);
                    } else {
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            format("{} \"{}\" -- Terminal unit part-load ratio limits of 0 to 1 exceeded error continues...",
                                   tuTypeNames[(int)this->type],
                                   this->Name),
                            this->FirstIterfailed);
                    }
                }
                if (FullOutput - NoCompOutput == 0.0) {
                    PartLoadRatio = 0.0;
                } else {
                    PartLoadRatio = min(1.0, max(MinPLF, std::abs(QZnReq - NoCompOutput) / std::abs(FullOutput - NoCompOutput)));
                }
            }
        }
    }
}

void VRFTerminalUnitEquipment::CalcVRF(EnergyPlusData &state,
                                       int const VRFTUNum,                           // Unit index in VRF terminal unit array
                                       bool const FirstHVACIteration,                // flag for 1st HVAC iteration in the time step
                                       Real64 const PartLoadRatio,                   // compressor part load fraction
                                       Real64 &LoadMet,                              // load met by unit (W)
                                       Real64 &OnOffAirFlowRatio,                    // ratio of ON air flow to average air flow
                                       Real64 &SuppHeatCoilLoad,                     // supplemental heating coil load (W)
                                       ObjexxFCL::Optional<Real64> LatOutputProvided // delivered latent capacity (kgWater/s)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   July 2005
    //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Simulate the components making up the VRF terminal unit.

    // METHODOLOGY EMPLOYED:
    // Simulates the unit components sequentially in the air flow direction.

    using DXCoils::SimDXCoil;
    using SingleDuct::SimATMixer;
    using SteamCoils::SimulateSteamCoilComponents;
    using WaterCoils::SimulateWaterCoilComponents;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int VRFTUOutletNodeNum; // TU air outlet node
    int VRFTUInletNodeNum;  // TU air inlet node
    Real64 AirMassFlow;     // total supply air mass flow [m3/s]
    HVAC::FanOp fanOp;      // fan operating mode, HVAC::FanOp::Cycling or HVAC::FanOp::Continuous
    int VRFCond;            // index to VRF condenser
    Real64 SpecHumOut(0.0); // specific humidity ratio at outlet node
    Real64 SpecHumIn(0.0);  // specific humidity ratio at inlet node
    int TUListIndex;        // index to TU list for this VRF system
    int IndexToTUInTUList;  // index to TU in specific list for the VRF system
    int ZoneNode;           // Zone node of VRFTU is serving

    VRFCond = this->VRFSysNum;
    TUListIndex = state.dataHVACVarRefFlow->VRF(VRFCond).ZoneTUListPtr;
    IndexToTUInTUList = this->IndexToTUInTUList;
    VRFTUOutletNodeNum = this->VRFTUOutletNodeNum;
    VRFTUInletNodeNum = this->VRFTUInletNodeNum;
    fanOp = this->fanOp;
    ZoneNode = this->ZoneAirNode;

    // Set inlet air mass flow rate based on PLR and compressor on/off air flow rates
    SetAverageAirFlow(state, VRFTUNum, PartLoadRatio, OnOffAirFlowRatio);

    AirMassFlow = state.dataLoopNodes->Node(VRFTUInletNodeNum).MassFlowRate;
    if (this->ATMixerExists) {
        // There is an air terminal mixer
        state.dataHVACVarRefFlow->ATMixOutNode = this->ATMixerOutNode;
        if (this->ATMixerType == HVAC::MixerType::InletSide) { // if there is an inlet side air terminal mixer
            // set the primary air inlet mass flow rate
            state.dataLoopNodes->Node(this->ATMixerPriNode).MassFlowRate =
                min(state.dataLoopNodes->Node(this->ATMixerPriNode).MassFlowRateMaxAvail, state.dataLoopNodes->Node(VRFTUInletNodeNum).MassFlowRate);
            // now calculate the the mixer outlet air conditions (and the secondary air inlet flow rate). The mixer outlet flow rate has already
            // been set above (it is the "inlet" node flow rate)
            SimATMixer(state, this->ATMixerName, FirstHVACIteration, this->ATMixerIndex);
        }
    } else {
        // ATMixOutNode = 0;
        if (this->OAMixerUsed) {
            MixedAir::SimOAMixer(state, this->OAMixerName, this->OAMixerIndex);
        }
    }
    // if blow through, simulate fan then coils
    if (this->fanPlace == HVAC::FanPlace::BlowThru) {
        if (this->fanType == HVAC::FanType::SystemModel) {
            if (OnOffAirFlowRatio > 0.0) {
                state.dataFans->fans(this->FanIndex)->simulate(state, FirstHVACIteration, _, _);
            } else {
                state.dataFans->fans(this->FanIndex)->simulate(state, FirstHVACIteration, _, _, PartLoadRatio);
            }
        } else {
            state.dataFans->fans(this->FanIndex)->simulate(state, FirstHVACIteration, state.dataHVACVarRefFlow->FanSpeedRatio);
        }
    }

    if (this->CoolingCoilPresent) {
        // above condition for heat pump mode, below condition for heat recovery mode
        if ((!state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed && state.dataHVACVarRefFlow->CoolingLoad(VRFCond)) ||
            (state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed &&
             state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList))) {
            SimDXCoil(state,
                      "",
                      HVAC::CompressorOp::On,
                      FirstHVACIteration,
                      this->CoolCoilIndex,
                      fanOp,
                      PartLoadRatio,
                      OnOffAirFlowRatio,
                      _,
                      state.dataHVACVarRefFlow->MaxCoolingCapacity(VRFCond),
                      state.dataHVACVarRefFlow->VRF(this->VRFSysNum).VRFCondCyclingRatio);
        } else { // cooling coil is off
            SimDXCoil(state, "", HVAC::CompressorOp::Off, FirstHVACIteration, this->CoolCoilIndex, fanOp, 0.0, OnOffAirFlowRatio);
        }
        state.dataHVACVarRefFlow->LoopDXCoolCoilRTF = state.dataAirLoop->LoopDXCoilRTF;
    } else {
        state.dataHVACVarRefFlow->LoopDXCoolCoilRTF = 0.0;
    }

    if (this->HeatingCoilPresent) {
        // above condition for heat pump mode, below condition for heat recovery mode
        if ((!state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed && state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) ||
            (state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed &&
             state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList))) {
            SimDXCoil(state,
                      "",
                      HVAC::CompressorOp::Off,
                      FirstHVACIteration,
                      this->HeatCoilIndex,
                      fanOp,
                      PartLoadRatio,
                      OnOffAirFlowRatio,
                      _,
                      state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond));
        } else {
            SimDXCoil(state, "", HVAC::CompressorOp::Off, FirstHVACIteration, this->HeatCoilIndex, fanOp, 0.0, OnOffAirFlowRatio, _);
        }
        state.dataHVACVarRefFlow->LoopDXHeatCoilRTF = state.dataAirLoop->LoopDXCoilRTF;
    } else {
        state.dataHVACVarRefFlow->LoopDXHeatCoilRTF = 0.0;
    }

    // if draw through, simulate coils then fan
    if (this->fanPlace == HVAC::FanPlace::DrawThru) {
        if (this->fanType == HVAC::FanType::SystemModel) {
            if (OnOffAirFlowRatio > 0.0) {
                state.dataFans->fans(this->FanIndex)->simulate(state, FirstHVACIteration, _, _);
            } else {
                state.dataFans->fans(this->FanIndex)->simulate(state, FirstHVACIteration, _, _, PartLoadRatio);
            }

        } else {
            state.dataFans->fans(this->FanIndex)->simulate(state, FirstHVACIteration, state.dataHVACVarRefFlow->FanSpeedRatio);
        }
    }

    // track fan power per terminal unit for calculating COP
    this->FanPower = (this->FanIndex == 0) ? 0.0 : state.dataFans->fans(this->FanIndex)->totalPower;

    // run supplemental heating coil
    if (this->SuppHeatingCoilPresent) {
        Real64 SuppPLR = this->SuppHeatPartLoadRatio;
        this->CalcVRFSuppHeatingCoil(state, VRFTUNum, FirstHVACIteration, SuppPLR, SuppHeatCoilLoad);
        if ((state.dataLoopNodes->Node(this->SuppHeatCoilAirOutletNode).Temp > this->MaxSATFromSuppHeatCoil) && SuppPLR > 0.0) {
            // adjust the heating load to maximum allowed
            Real64 MaxHeatCoilLoad = this->HeatingCoilCapacityLimit(state, this->SuppHeatCoilAirInletNode, this->MaxSATFromSuppHeatCoil);
            this->CalcVRFSuppHeatingCoil(state, VRFTUNum, FirstHVACIteration, SuppPLR, MaxHeatCoilLoad);
            SuppHeatCoilLoad = MaxHeatCoilLoad;
        }
    }

    Real64 LatentLoadMet = 0.0; // latent load delivered [kgWater/s]
    Real64 TempOut = 0.0;
    Real64 TempIn = 0.0;
    if (this->ATMixerExists) {
        if (this->ATMixerType == HVAC::MixerType::SupplySide) {
            // Air terminal supply side mixer, calculate supply side mixer output
            SimATMixer(state, this->ATMixerName, FirstHVACIteration, this->ATMixerIndex);
            TempOut = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->ATMixOutNode).Temp;
            SpecHumOut = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->ATMixOutNode).HumRat;
            AirMassFlow = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->ATMixOutNode).MassFlowRate;
        } else {
            // Air terminal inlet side mixer
            TempOut = state.dataLoopNodes->Node(VRFTUOutletNodeNum).Temp;
            SpecHumOut = state.dataLoopNodes->Node(VRFTUOutletNodeNum).HumRat;
        }
        TempIn = state.dataLoopNodes->Node(ZoneNode).Temp;
        SpecHumIn = state.dataLoopNodes->Node(ZoneNode).HumRat;
    } else {
        TempOut = state.dataLoopNodes->Node(VRFTUOutletNodeNum).Temp;
        SpecHumOut = state.dataLoopNodes->Node(VRFTUOutletNodeNum).HumRat;
        if (ZoneNode > 0) {
            TempIn = state.dataLoopNodes->Node(ZoneNode).Temp;
            SpecHumIn = state.dataLoopNodes->Node(ZoneNode).HumRat;
        } else {
            TempIn = state.dataLoopNodes->Node(VRFTUInletNodeNum).Temp;
            SpecHumIn = state.dataLoopNodes->Node(VRFTUInletNodeNum).HumRat;
        }
    }
    // calculate sensible load met using delta enthalpy
    Real64 TotalOutput = AirMassFlow * (Psychrometrics::PsyHFnTdbW(TempOut, SpecHumOut) -
                                        Psychrometrics::PsyHFnTdbW(TempIn, SpecHumIn));         // total addition/removal rate, {W};
    LoadMet = AirMassFlow * PsyDeltaHSenFnTdb2W2Tdb1W1(TempOut, SpecHumOut, TempIn, SpecHumIn); // sensible {W}
    LatentLoadMet = TotalOutput - LoadMet;
    if (present(LatOutputProvided)) {
        //   CR9155 Remove specific humidity calculations
        LatOutputProvided = LatentLoadMet;
    }
}

void ReportVRFTerminalUnit(EnergyPlusData &state, int const VRFTUNum) // index to VRF terminal unit
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   August 2010
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the report variables for the VRF Terminal Units.

    using namespace DataSizing;

    Real64 TotalConditioning;    // - sum of sensible and latent rates
    Real64 SensibleConditioning; // - sensible rate
    Real64 LatentConditioning;   // - latent rate
    Real64 ReportingConstant;    // - used to convert watts to joules
    int VRFCond;                 // - index to VRF condenser
    int TUListIndex;             // - index to terminal unit list
    int IndexToTUInTUList;       // - index to the TU in the list
    bool HRHeatRequestFlag;      // - indicates TU could be in heat mode
    bool HRCoolRequestFlag;      // - indicates TU could be in cool mode

    VRFCond = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFSysNum;
    TUListIndex = state.dataHVACVarRefFlow->VRF(VRFCond).ZoneTUListPtr;
    IndexToTUInTUList = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).IndexToTUInTUList;
    HRHeatRequestFlag = state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList);
    HRCoolRequestFlag = state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList);
    ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;

    // account for terminal unit parasitic On/Off power use
    // account for heat recovery first since these flags will be FALSE otherwise, each TU may have different operating mode

    if (HRCoolRequestFlag) {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolingCoilPresent) {
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticCoolElecPower =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElec * state.dataHVACVarRefFlow->LoopDXCoolCoilRTF +
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticOffElec * (1.0 - state.dataHVACVarRefFlow->LoopDXCoolCoilRTF);
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecCoolConsumption =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticCoolElecPower * ReportingConstant;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticHeatElecPower = 0.0;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecHeatConsumption = 0.0;
        } else {
            // cooling parasitic power report variable is not even available when there is no cooling coil, report for heating
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticHeatElecPower = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticOffElec;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecHeatConsumption =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticHeatElecPower * ReportingConstant;
        }
    } else if (HRHeatRequestFlag) {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatingCoilPresent) {
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticCoolElecPower = 0.0;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecCoolConsumption = 0.0;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticHeatElecPower =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElec * state.dataHVACVarRefFlow->LoopDXHeatCoilRTF +
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticOffElec * (1.0 - state.dataHVACVarRefFlow->LoopDXHeatCoilRTF);
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecHeatConsumption =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticHeatElecPower * ReportingConstant;
        } else {
            // heating parasitic power report variable is not even available when there is no heating coil, report for cooling
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticCoolElecPower = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticOffElec;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecCoolConsumption =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticCoolElecPower * ReportingConstant;
        }
    } else if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond) ||
               (!state.dataHVACVarRefFlow->HeatingLoad(VRFCond) &&
                state.dataHVACVarRefFlow->LastModeCooling(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFSysNum))) {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolingCoilPresent) {
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticCoolElecPower =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElec * state.dataHVACVarRefFlow->LoopDXCoolCoilRTF +
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticOffElec * (1.0 - state.dataHVACVarRefFlow->LoopDXCoolCoilRTF);
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecCoolConsumption =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticCoolElecPower * ReportingConstant;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticHeatElecPower = 0.0;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecHeatConsumption = 0.0;
        } else {
            // cooling parasitic power report variable is not even available when there is no cooling coil, report for heating
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticHeatElecPower = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticOffElec;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecHeatConsumption =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticHeatElecPower * ReportingConstant;
        }
    } else if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond) ||
               (!state.dataHVACVarRefFlow->CoolingLoad(VRFCond) &&
                state.dataHVACVarRefFlow->LastModeHeating(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFSysNum))) {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatingCoilPresent) {
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticCoolElecPower = 0.0;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecCoolConsumption = 0.0;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticHeatElecPower =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElec * state.dataHVACVarRefFlow->LoopDXHeatCoilRTF +
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticOffElec * (1.0 - state.dataHVACVarRefFlow->LoopDXHeatCoilRTF);
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecHeatConsumption =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticHeatElecPower * ReportingConstant;
        } else {
            // heating parasitic power report variable is not even available when there is no heating coil, report for cooling
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticCoolElecPower = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticOffElec;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecCoolConsumption =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticCoolElecPower * ReportingConstant;
        }
    } else {
        // happens when there is no cooling or heating load
        if (!state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CoolingCoilPresent) {
            // report all for heating
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticHeatElecPower = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticOffElec;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecHeatConsumption =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticHeatElecPower * ReportingConstant;
        } else if (!state.dataHVACVarRefFlow->VRFTU(VRFTUNum).HeatingCoilPresent) {
            // report all for cooling
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticCoolElecPower = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticOffElec;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecCoolConsumption =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticCoolElecPower * ReportingConstant;
        } else {
            // split parasitic between both reporting variables
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticCoolElecPower = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticOffElec / 2.0;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecCoolConsumption =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticCoolElecPower * ReportingConstant;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticHeatElecPower = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticOffElec / 2.0;
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticElecHeatConsumption =
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ParasiticHeatElecPower * ReportingConstant;
        }
    }

    SensibleConditioning = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TerminalUnitSensibleRate;
    LatentConditioning = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TerminalUnitLatentRate;
    Real64 TempOut = 0.0;
    Real64 TempIn = 0.0;
    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerExists) {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerType == HVAC::MixerType::SupplySide) {
            // Air terminal supply side mixer
            TempOut = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerOutNode).Temp;
            TempIn = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneAirNode).Temp;
        } else {
            // Air terminal inlet side mixer
            TempOut = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOutletNodeNum).Temp;
            TempIn = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneAirNode).Temp;
        }
    } else {
        TempOut = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOutletNodeNum).Temp;
        TempIn = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).Temp;
    }
    TotalConditioning = SensibleConditioning + LatentConditioning;

    if (TotalConditioning <= 0.0) {
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TotalCoolingRate = std::abs(TotalConditioning);
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TotalHeatingRate = 0.0;
    } else {
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TotalCoolingRate = 0.0;
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TotalHeatingRate = TotalConditioning;
    }
    if (SensibleConditioning <= 0.0) {
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SensibleCoolingRate = std::abs(SensibleConditioning);
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SensibleHeatingRate = 0.0;
    } else {
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SensibleCoolingRate = 0.0;
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SensibleHeatingRate = SensibleConditioning;
    }
    if (LatentConditioning <= 0.0) {
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).LatentCoolingRate = std::abs(LatentConditioning);
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).LatentHeatingRate = 0.0;
    } else {
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).LatentCoolingRate = 0.0;
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).LatentHeatingRate = LatentConditioning;
    }
    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TotalCoolingEnergy = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TotalCoolingRate * ReportingConstant;
    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SensibleCoolingEnergy =
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SensibleCoolingRate * ReportingConstant;
    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).LatentCoolingEnergy = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).LatentCoolingRate * ReportingConstant;
    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TotalHeatingEnergy = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).TotalHeatingRate * ReportingConstant;
    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SensibleHeatingEnergy =
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SensibleHeatingRate * ReportingConstant;
    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).LatentHeatingEnergy = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).LatentHeatingRate * ReportingConstant;

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).firstPass) {
        if (!state.dataHVACVarRefFlow->MySizeFlag(VRFTUNum)) {
            DataSizing::resetHVACSizingGlobals(state, state.dataSize->CurZoneEqNum, 0, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).firstPass);
        }
    }

    // reset to 1 in case blow through fan configuration (fan resets to 1, but for blow thru fans coil sets back down < 1)
    state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
}

void ReportVRFCondenser(EnergyPlusData &state, int const VRFCond) // index to VRF condensing unit
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   August 2010
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the report variables for the VRF Condenser.

    Real64 ReportingConstant; // - conversion constant for energy

    ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;

    //   calculate VRF condenser power/energy use
    state.dataHVACVarRefFlow->VRF(VRFCond).CoolElecConsumption = state.dataHVACVarRefFlow->VRF(VRFCond).ElecCoolingPower * ReportingConstant;
    state.dataHVACVarRefFlow->VRF(VRFCond).HeatElecConsumption = state.dataHVACVarRefFlow->VRF(VRFCond).ElecHeatingPower * ReportingConstant;

    state.dataHVACVarRefFlow->VRF(VRFCond).DefrostConsumption = state.dataHVACVarRefFlow->VRF(VRFCond).DefrostPower * ReportingConstant;
    state.dataHVACVarRefFlow->VRF(VRFCond).BasinHeaterConsumption = state.dataHVACVarRefFlow->VRF(VRFCond).BasinHeaterPower * ReportingConstant;

    state.dataHVACVarRefFlow->VRF(VRFCond).EvapCondPumpElecConsumption =
        state.dataHVACVarRefFlow->VRF(VRFCond).EvapCondPumpElecPower * ReportingConstant;
    state.dataHVACVarRefFlow->VRF(VRFCond).CrankCaseHeaterElecConsumption =
        state.dataHVACVarRefFlow->VRF(VRFCond).CrankCaseHeaterPower * ReportingConstant;

    state.dataHVACVarRefFlow->VRF(VRFCond).QCondEnergy = state.dataHVACVarRefFlow->VRF(VRFCond).QCondenser * ReportingConstant;
    state.dataHVACVarRefFlow->VRF(VRFCond).VRFHeatEnergyRec = state.dataHVACVarRefFlow->VRF(VRFCond).VRFHeatRec * ReportingConstant;
}

void UpdateVRFCondenser(EnergyPlusData &state, int const VRFCond) // index to VRF condensing unit
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   May 2012
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the node data for the VRF Condenser.

    int CondenserOutletNode; // - outlet node for VRF water-cooled condenser

    CondenserOutletNode = state.dataHVACVarRefFlow->VRF(VRFCond).CondenserOutletNodeNum;

    state.dataLoopNodes->Node(CondenserOutletNode).Temp = state.dataHVACVarRefFlow->VRF(VRFCond).CondenserSideOutletTemp;

    state.dataLoopNodes->Node(CondenserOutletNode).MassFlowRate = state.dataHVACVarRefFlow->CondenserWaterMassFlowRate;
    state.dataLoopNodes->Node(CondenserOutletNode).MassFlowRateMaxAvail = state.dataLoopNodes->Node(CondenserOutletNode).MassFlowRateMaxAvail;
    state.dataLoopNodes->Node(CondenserOutletNode).MassFlowRateMinAvail = state.dataLoopNodes->Node(CondenserOutletNode).MassFlowRateMinAvail;
}

void isVRFCoilPresent(EnergyPlusData &state, std::string_view VRFTUName, bool &CoolCoilPresent, bool &HeatCoilPresent)
{

    if (state.dataHVACVarRefFlow->GetVRFInputFlag) {
        GetVRFInput(state);
        state.dataHVACVarRefFlow->GetVRFInputFlag = false;
    }

    int WhichVRFTU =
        Util::FindItemInList(VRFTUName, state.dataHVACVarRefFlow->VRFTU, &VRFTerminalUnitEquipment::Name, state.dataHVACVarRefFlow->NumVRFTU);
    if (WhichVRFTU != 0) {
        CoolCoilPresent = state.dataHVACVarRefFlow->VRFTU(WhichVRFTU).CoolingCoilPresent;
        HeatCoilPresent = state.dataHVACVarRefFlow->VRFTU(WhichVRFTU).HeatingCoilPresent;
    } else {
        ShowSevereError(state, format("isVRFCoilPresent: Could not find VRF TU = \"{}\"", VRFTUName));
    }
}

//        End of Reporting subroutines for the Module
// *****************************************************************************

// Utility subroutines for the Module

void SetAverageAirFlow(EnergyPlusData &state,
                       int const VRFTUNum,         // Unit index
                       Real64 const PartLoadRatio, // unit part load ratio
                       Real64 &OnOffAirFlowRatio   // ratio of compressor ON airflow to average airflow over timestep
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   August 2010
    //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Set the average air mass flow rates using the part load fraction of the heat pump for this time step
    // Set OnOffAirFlowRatio to be used by DX coils
    auto &s_vrf = state.dataHVACVarRefFlow;
    auto &vrfTu = s_vrf->VRFTU(VRFTUNum);

    int InletNode;                   // inlet node number
    int OutsideAirNode;              // outside air node number
    int AirRelNode;                  // relief air node number
    Real64 AverageUnitMassFlow(0.0); // average supply air mass flow rate over time step
    Real64 AverageOAMassFlow(0.0);   // average outdoor air mass flow rate over time step

    InletNode = vrfTu.VRFTUInletNodeNum;
    OutsideAirNode = vrfTu.VRFTUOAMixerOANodeNum;
    AirRelNode = vrfTu.VRFTUOAMixerRelNodeNum;

    if (vrfTu.fanOp == HVAC::FanOp::Cycling && vrfTu.SpeedNum == 0) {
        Real64 partLoadRat = PartLoadRatio;
        if (partLoadRat == 0.0 && vrfTu.SuppHeatPartLoadRatio > 0.0) {
            partLoadRat = vrfTu.SuppHeatPartLoadRatio;
        }
        AverageUnitMassFlow = (partLoadRat * s_vrf->CompOnMassFlow) + ((1 - partLoadRat) * s_vrf->CompOffMassFlow);
        AverageOAMassFlow = (partLoadRat * s_vrf->OACompOnMassFlow) + ((1 - partLoadRat) * s_vrf->OACompOffMassFlow);
    } else if (vrfTu.SpeedNum == 0) {
        if (PartLoadRatio == 0.0) {
            // set the average OA air flow to off compressor values if the compressor PartLoadRatio is zero
            AverageUnitMassFlow = s_vrf->CompOffMassFlow;
            AverageOAMassFlow = s_vrf->OACompOffMassFlow;
        } else {
            AverageUnitMassFlow = s_vrf->CompOnMassFlow;
            AverageOAMassFlow = s_vrf->OACompOnMassFlow;
        }
    } else if (vrfTu.SpeedNum == 1) {
        if (s_vrf->CoolingLoad(vrfTu.VRFSysNum)) {
            AverageUnitMassFlow = vrfTu.CoolMassFlowRate[vrfTu.SpeedNum] * PartLoadRatio + (1.0 - PartLoadRatio) * s_vrf->CompOffMassFlow;
        } else if (s_vrf->HeatingLoad(vrfTu.VRFSysNum)) {
            AverageUnitMassFlow = vrfTu.HeatMassFlowRate[vrfTu.SpeedNum] * PartLoadRatio + (1.0 - PartLoadRatio) * s_vrf->CompOffMassFlow;
        }
    } else if (s_vrf->CoolingLoad(vrfTu.VRFSysNum)) {
        AverageUnitMassFlow =
            vrfTu.CoolMassFlowRate[vrfTu.SpeedNum] * PartLoadRatio + (1.0 - PartLoadRatio) * vrfTu.CoolMassFlowRate[vrfTu.SpeedNum - 1];
    } else if (s_vrf->HeatingLoad(vrfTu.VRFSysNum)) {
        AverageUnitMassFlow =
            vrfTu.HeatMassFlowRate[vrfTu.SpeedNum] * PartLoadRatio + (1.0 - PartLoadRatio) * vrfTu.HeatMassFlowRate[vrfTu.SpeedNum - 1];
    }

    if (vrfTu.SpeedNum == 0) {
        if (s_vrf->CompOffFlowRatio > 0.0) {
            s_vrf->FanSpeedRatio = (PartLoadRatio * s_vrf->CompOnFlowRatio) + ((1 - PartLoadRatio) * s_vrf->CompOffFlowRatio);
        } else {
            s_vrf->FanSpeedRatio = s_vrf->CompOnFlowRatio;
        }
    }

    // if the terminal unit and fan are scheduled on then set flow rate
    if (vrfTu.availSched->getCurrentVal() > 0.0 && (vrfTu.fanAvailSched->getCurrentVal() > 0.0 || state.dataHVACGlobal->TurnFansOn) &&
        !state.dataHVACGlobal->TurnFansOff) {

        // so for sure OA system TUs should use inlet node flow rate, don't overwrite inlet node flow rate
        // could there be a reason for air loops to use inlet node flow? Possibly when VAV TUs used?
        if (!vrfTu.isInOASys) {
            state.dataLoopNodes->Node(InletNode).MassFlowRate = AverageUnitMassFlow;
        }
        if (!vrfTu.isInOASys) {
            state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AverageUnitMassFlow;
        }
        if (OutsideAirNode > 0) {
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate = AverageOAMassFlow;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMaxAvail = AverageOAMassFlow;
            state.dataLoopNodes->Node(AirRelNode).MassFlowRate = AverageOAMassFlow;
            state.dataLoopNodes->Node(AirRelNode).MassFlowRateMaxAvail = AverageOAMassFlow;
        }
        if (AverageUnitMassFlow > 0.0) {
            OnOffAirFlowRatio = s_vrf->CompOnMassFlow / AverageUnitMassFlow;
        } else {
            OnOffAirFlowRatio = 0.0;
        }

    } else { // terminal unit and/or fan is off
        if (!vrfTu.isInOASys) {
            state.dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
            OnOffAirFlowRatio = 0.0;
        }
        if (OutsideAirNode > 0) {
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(AirRelNode).MassFlowRate = 0.0;
        }
    }
}

void InitializeOperatingMode(EnergyPlusData &state,
                             bool const FirstHVACIteration, // flag for first time through HVAC systems
                             int const VRFCond,             // Condenser Unit index
                             int const TUListNum,           // Condenser Unit terminal unit list
                             Real64 &OnOffAirFlowRatio      // ratio of on to off flow rate
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   July 2012 (Moved from InitVRF)
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Scans each zone coil and determines the load based on control
    // Moved from Init to clean up and localize code segments

    Real64 ZoneDeltaT;       // zone temperature difference from setpoint
    Real64 SPTempHi;         // thermostat setpoint high
    Real64 SPTempLo;         // thermostat setpoint low
    int NumTU;               // loop counter, number of TU's in list
    Real64 ZoneLoad;         // current zone load (W)
    Real64 LoadToCoolingSP;  // thermostat load to cooling setpoint (W)
    Real64 LoadToHeatingSP;  // thermostat load to heating setpoint (W)
    Real64 TempOutput;       // terminal unit output [W]
    Real64 SuppHeatCoilLoad; // supplemental heating coil load

    state.dataHVACVarRefFlow->MaxDeltaT = 0.0;
    state.dataHVACVarRefFlow->MinDeltaT = 0.0;
    state.dataHVACVarRefFlow->NumCoolingLoads = 0;
    state.dataHVACVarRefFlow->SumCoolingLoads = 0.0;
    state.dataHVACVarRefFlow->NumHeatingLoads = 0;
    state.dataHVACVarRefFlow->SumHeatingLoads = 0.0;
    SuppHeatCoilLoad = 0.0;

    state.dataHVACVarRefFlow->NumCoolingLoads(VRFCond) = 0;
    state.dataHVACVarRefFlow->NumHeatingLoads(VRFCond) = 0;
    state.dataHVACVarRefFlow->SumCoolingLoads(VRFCond) = 0.0;
    state.dataHVACVarRefFlow->SumHeatingLoads(VRFCond) = 0.0;
    state.dataHVACVarRefFlow->MaxDeltaT(VRFCond) = 0.0;
    state.dataHVACVarRefFlow->MinDeltaT(VRFCond) = 0.0;
    ZoneDeltaT = 0.0;
    state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
    state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
    state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).CoolingCoilAvailable = false;
    state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).HeatingCoilAvailable = false;
    // loop through all TU's to find operating mode. Be careful not to mix loop counters with current TU/Cond index
    for (NumTU = 1; NumTU <= state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).NumTUInList; ++NumTU) {
        // make sure TU's have been sized before looping through each one of them to determine operating mode
        if (any(state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TerminalUnitNotSizedYet)) {
            break;
        }
        int TUIndex = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);

        //       check to see if coil is present
        if (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).CoolingCoilPresent(NumTU)) {
            //         now check to see if coil is scheduled off
            if (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).coolingCoilAvailScheds(NumTU)->getCurrentVal() > 0.0) {
                state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).CoolingCoilAvailable(NumTU) = true;
            }
        }

        //       check to see if coil is present
        if (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).HeatingCoilPresent(NumTU)) {
            //         now check to see if coil is scheduled off
            if (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).heatingCoilAvailScheds(NumTU)->getCurrentVal() > 0.0) {
                state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).HeatingCoilAvailable(NumTU) = true;
            }
        }

        if (state.dataHVACVarRefFlow->VRFTU(TUIndex).isSetPointControlled) {
            // set point temperature may only reside at the TU outlet node
            Real64 coolCoilTempSetPoint = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUOutletNodeNum).TempSetPoint;
            state.dataHVACVarRefFlow->VRFTU(TUIndex).suppTempSetPoint = coolCoilTempSetPoint;
            Real64 heatCoilTempSetPoint = coolCoilTempSetPoint;
            // adjust coil control for fan heat when set point is at outlet node
            Real64 coolfanDeltaT = 0.0;
            Real64 heatfanDeltaT = 0.0;
            if (state.dataHVACVarRefFlow->VRFTU(TUIndex).fanPlace == HVAC::FanPlace::DrawThru) {
                if (state.dataHVACVarRefFlow->VRFTU(TUIndex).fanOutletNode > 0) {
                    coolfanDeltaT = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).fanOutletNode).Temp -
                                    state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).fanInletNode).Temp;
                }
            }
            heatfanDeltaT = coolfanDeltaT;
            // or the set point could be placed at either or both coils, update both if necessary
            if (state.dataHVACVarRefFlow->VRFTU(TUIndex).CoolingCoilPresent) {
                if (state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).coolCoilAirOutNode).TempSetPoint !=
                    DataLoopNode::SensedNodeFlagValue) {
                    coolCoilTempSetPoint = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).coolCoilAirOutNode).TempSetPoint;
                    //// should we adjust for fan heat or not? What if it's a mixed air SP that already adjusts for fan heat?
                    // coolfanDeltaT = 0.0;
                }
            }
            if (state.dataHVACVarRefFlow->VRFTU(TUIndex).HeatingCoilPresent) {
                if (state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).heatCoilAirOutNode).TempSetPoint !=
                    DataLoopNode::SensedNodeFlagValue) {
                    heatCoilTempSetPoint = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).heatCoilAirOutNode).TempSetPoint;
                    //// should we adjust for fan heat or not? What if it's a mixed air SP that already adjusts for fan heat?
                    // heatfanDeltaT = 0.0;
                }
            }
            // set a flow rate and simulate ATMixer/OASystem if needed
            if (FirstHVACIteration) {
                SetAverageAirFlow(state, TUIndex, 1.0, OnOffAirFlowRatio);
                if (state.dataHVACVarRefFlow->VRFTU(TUIndex).ATMixerExists) {
                    // There is an air terminal mixer
                    if (state.dataHVACVarRefFlow->VRFTU(TUIndex).ATMixerType ==
                        HVAC::MixerType::InletSide) { // if there is an inlet side air terminal mixer
                                                      // set the primary air inlet mass flow rate
                        state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).ATMixerPriNode).MassFlowRate =
                            min(state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).ATMixerPriNode).MassFlowRateMaxAvail,
                                state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUInletNodeNum).MassFlowRate);
                        // now calculate the the mixer outlet air conditions (and the secondary air inlet flow rate). The mixer outlet flow rate
                        // has already been set above (it is the "inlet" node flow rate)
                        SingleDuct::SimATMixer(state,
                                               state.dataHVACVarRefFlow->VRFTU(TUIndex).ATMixerName,
                                               FirstHVACIteration,
                                               state.dataHVACVarRefFlow->VRFTU(TUIndex).ATMixerIndex);
                    }
                } else {
                    // simulate OA Mixer
                    if (state.dataHVACVarRefFlow->VRFTU(TUIndex).OAMixerUsed) {
                        MixedAir::SimOAMixer(
                            state, state.dataHVACVarRefFlow->VRFTU(TUIndex).OAMixerName, state.dataHVACVarRefFlow->VRFTU(TUIndex).OAMixerIndex);
                    }
                }
            }
            // identify a coil inlet temperature
            if (state.dataHVACVarRefFlow->VRFTU(TUIndex).CoolingCoilPresent) {
                state.dataHVACVarRefFlow->VRFTU(TUIndex).coilInNodeT =
                    state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).coolCoilAirInNode).Temp;
                state.dataHVACVarRefFlow->VRFTU(TUIndex).coilInNodeW =
                    state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).coolCoilAirInNode).HumRat;
            } else {
                state.dataHVACVarRefFlow->VRFTU(TUIndex).coilInNodeT =
                    state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).heatCoilAirInNode).Temp;
                state.dataHVACVarRefFlow->VRFTU(TUIndex).coilInNodeW =
                    state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).heatCoilAirInNode).HumRat;
            }
            Real64 coilInletTemp = state.dataHVACVarRefFlow->VRFTU(TUIndex).coilInNodeT;
            Real64 coilInletHumRat = state.dataHVACVarRefFlow->VRFTU(TUIndex).coilInNodeW;
            Real64 coilInletMassFlow = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUInletNodeNum).MassFlowRate;
            state.dataHVACVarRefFlow->VRFTU(TUIndex).coolSPActive = false;
            state.dataHVACVarRefFlow->VRFTU(TUIndex).heatSPActive = false;

            if ((heatCoilTempSetPoint - coilInletTemp - heatfanDeltaT) > HVAC::SmallTempDiff) { // heating
                Real64 CpAirIn = Psychrometrics::PsyCpAirFnW(coilInletHumRat);
                ZoneLoad = coilInletMassFlow * CpAirIn * (heatCoilTempSetPoint - coilInletTemp - heatfanDeltaT);
                state.dataHVACVarRefFlow->VRFTU(TUIndex).heatSPActive = true;
                state.dataHVACVarRefFlow->VRFTU(TUIndex).heatLoadToSP = ZoneLoad;
                ++state.dataHVACVarRefFlow->NumHeatingLoads(VRFCond);
                state.dataHVACVarRefFlow->SumHeatingLoads(VRFCond) += ZoneLoad;
                state.dataHVACVarRefFlow->MinDeltaT(VRFCond) = min(state.dataHVACVarRefFlow->MinDeltaT(VRFCond), -1.0);
                state.dataHVACVarRefFlow->VRFTU(TUIndex).coilTempSetPoint = heatCoilTempSetPoint - heatfanDeltaT;
            } else if ((coilInletTemp - coolCoilTempSetPoint - coolfanDeltaT) > HVAC::SmallTempDiff) { // cooling
                Real64 CpAirIn = Psychrometrics::PsyCpAirFnW(coilInletHumRat);
                ZoneLoad = coilInletMassFlow * CpAirIn * (coolCoilTempSetPoint - coilInletTemp - coolfanDeltaT);
                state.dataHVACVarRefFlow->VRFTU(TUIndex).coolSPActive = true;
                state.dataHVACVarRefFlow->VRFTU(TUIndex).coolLoadToSP = ZoneLoad;
                ++state.dataHVACVarRefFlow->NumCoolingLoads(VRFCond);
                state.dataHVACVarRefFlow->SumCoolingLoads(VRFCond) += ZoneLoad;
                state.dataHVACVarRefFlow->MaxDeltaT(VRFCond) = max(state.dataHVACVarRefFlow->MaxDeltaT(VRFCond), 1.0);
                state.dataHVACVarRefFlow->VRFTU(TUIndex).coilTempSetPoint = coolCoilTempSetPoint - coolfanDeltaT;
            }
        } else { // else is not set point controlled
            //     Constant fan systems are tested for ventilation load to determine if load to be met changes.
            //     more logic may be needed here, what is the OA flow rate, was last mode heating or cooling, what control is used, etc...

            int ThisZoneNum = state.dataHVACVarRefFlow->VRFTU(TUIndex).ZoneNum;
            getVRFTUZoneLoad(state, TUIndex, ZoneLoad, LoadToHeatingSP, LoadToCoolingSP, true);

            if (state.dataHVACVarRefFlow->VRF(VRFCond).ThermostatPriority == ThermostatCtrlType::ThermostatOffsetPriority) {
                //         for TSTATPriority, just check difference between zone temp and thermostat setpoint
                if (ThisZoneNum > 0) {
                    auto const &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ThisZoneNum);
                    auto const &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(ThisZoneNum);
                    SPTempHi = zoneTstatSetpt.setptHi;
                    SPTempLo = zoneTstatSetpt.setptLo;

                    switch (state.dataHeatBalFanSys->TempControlType(ThisZoneNum)) {
                    case HVAC::SetptType::Uncontrolled:
                        // MaxDeltaT denotes cooling, MinDeltaT denotes heating
                        break;
                    case HVAC::SetptType::SingleHeat:
                        // if heating load, ZoneDeltaT will be negative
                        ZoneDeltaT = min(0.0, thisZoneHB.ZT - SPTempLo);
                        state.dataHVACVarRefFlow->MinDeltaT(VRFCond) = min(state.dataHVACVarRefFlow->MinDeltaT(VRFCond), ZoneDeltaT);
                        break;
                    case HVAC::SetptType::SingleCool:
                        // if cooling load, ZoneDeltaT will be positive
                        ZoneDeltaT = max(0.0, thisZoneHB.ZT - SPTempHi);
                        state.dataHVACVarRefFlow->MaxDeltaT(VRFCond) = max(state.dataHVACVarRefFlow->MaxDeltaT(VRFCond), ZoneDeltaT);
                        break;
                    case HVAC::SetptType::SingleHeatCool:
                        ZoneDeltaT = thisZoneHB.ZT - SPTempHi; //- SPTempHi and SPTempLo are same value
                        if (ZoneDeltaT > 0.0) {
                            state.dataHVACVarRefFlow->MaxDeltaT(VRFCond) = max(state.dataHVACVarRefFlow->MaxDeltaT(VRFCond), ZoneDeltaT);
                        } else {
                            state.dataHVACVarRefFlow->MinDeltaT(VRFCond) = min(state.dataHVACVarRefFlow->MinDeltaT(VRFCond), ZoneDeltaT);
                        }
                        break;
                    case HVAC::SetptType::DualHeatCool:
                        if (thisZoneHB.ZT - SPTempHi > 0.0) {
                            ZoneDeltaT = max(0.0, thisZoneHB.ZT - SPTempHi);
                            state.dataHVACVarRefFlow->MaxDeltaT(VRFCond) = max(state.dataHVACVarRefFlow->MaxDeltaT(VRFCond), ZoneDeltaT);
                        } else if (SPTempLo - thisZoneHB.ZT > 0.0) {
                            ZoneDeltaT = min(0.0, thisZoneHB.ZT - SPTempLo);
                            state.dataHVACVarRefFlow->MinDeltaT(VRFCond) = min(state.dataHVACVarRefFlow->MinDeltaT(VRFCond), ZoneDeltaT);
                        }
                        break;
                    default:
                        break;
                    }
                }
            } else if (state.dataHVACVarRefFlow->VRF(VRFCond).ThermostatPriority == ThermostatCtrlType::LoadPriority ||
                       state.dataHVACVarRefFlow->VRF(VRFCond).ThermostatPriority == ThermostatCtrlType::ZonePriority) {
                if (state.dataHVACVarRefFlow->VRFTU(TUIndex).fanOp == HVAC::FanOp::Continuous) {
                    SetCompFlowRate(state, TUIndex, VRFCond);

                    if (state.dataHVACVarRefFlow->VRF(VRFCond).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                        state.dataHVACVarRefFlow->VRFTU(TUIndex).CalcVRF_FluidTCtrl(
                            state, TUIndex, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    } else {
                        // Algorithm Type: VRF model based on system curve
                        state.dataHVACVarRefFlow->VRFTU(TUIndex).CalcVRF(
                            state, TUIndex, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                    }

                    //           If the Terminal Unit has a net cooling capacity (NoCompOutput < 0) and
                    //           the zone temp is above the Tstat heating setpoint (QToHeatSetPt < 0)
                    if (TempOutput < 0.0 && LoadToHeatingSP < 0.0) {
                        //             If the net cooling capacity overshoots the heating setpoint count as heating load
                        if (TempOutput < LoadToHeatingSP) {
                            //               Don't count as heating load unless mode is allowed. Also check for floating zone.
                            if (state.dataHeatBalFanSys->TempControlType(ThisZoneNum) != HVAC::SetptType::SingleCool &&
                                state.dataHeatBalFanSys->TempControlType(ThisZoneNum) != HVAC::SetptType::Uncontrolled) {
                                if (!state.dataHVACVarRefFlow->LastModeHeating(VRFCond)) {
                                    // if last mode was cooling, make sure heating flow rate is used
                                    if (state.dataHVACVarRefFlow->VRFTU(TUIndex).OAMixerUsed) {
                                        state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUOAMixerRetNodeNum).MassFlowRate =
                                            state.dataHVACVarRefFlow->VRFTU(TUIndex).MaxHeatAirMassFlow;
                                        state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUOAMixerOANodeNum).MassFlowRate =
                                            state.dataHVACVarRefFlow->VRFTU(TUIndex).HeatOutAirMassFlow;
                                        MixedAir::SimOAMixer(state,
                                                             state.dataHVACVarRefFlow->VRFTU(TUIndex).OAMixerName,
                                                             state.dataHVACVarRefFlow->VRFTU(TUIndex).OAMixerIndex);
                                    } else {
                                        state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUInletNodeNum).MassFlowRate =
                                            state.dataHVACVarRefFlow->VRFTU(TUIndex).MaxHeatAirMassFlow;
                                    }

                                    // recalculate using correct flow rate
                                    if (state.dataHVACVarRefFlow->VRF(VRFCond).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                                        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                                        state.dataHVACVarRefFlow->VRFTU(TUIndex).CalcVRF_FluidTCtrl(
                                            state, TUIndex, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                                    } else {
                                        // Algorithm Type: VRF model based on system curve
                                        state.dataHVACVarRefFlow->VRFTU(TUIndex).CalcVRF(
                                            state, TUIndex, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                                    }

                                    if (TempOutput < LoadToHeatingSP) {
                                        ++state.dataHVACVarRefFlow->NumHeatingLoads(VRFCond);
                                        // sum heating load on condenser, not total zone heating load
                                        state.dataHVACVarRefFlow->SumHeatingLoads(VRFCond) += (LoadToHeatingSP - TempOutput);
                                    }
                                } else {
                                    ++state.dataHVACVarRefFlow->NumHeatingLoads(VRFCond);
                                    // sum heating load on condenser, not total zone heating load
                                    state.dataHVACVarRefFlow->SumHeatingLoads(VRFCond) += (LoadToHeatingSP - TempOutput);
                                }
                            }
                        } else if (TempOutput < ZoneLoad) {
                            //             If the net cooling capacity meets the zone cooling load but does not overshoot heating setpoint, turn
                            //             off coil do nothing, the zone will float
                        } else if (ZoneLoad < 0.0) {
                            //               still a cooling load
                            ++state.dataHVACVarRefFlow->NumCoolingLoads(VRFCond);
                            // sum cooling load on condenser, not total zone cooling load
                            state.dataHVACVarRefFlow->SumCoolingLoads(VRFCond) += (LoadToCoolingSP - TempOutput);
                        }

                        //           If the terminal unit has a net heating capacity and the zone temp is below the Tstat cooling setpoint
                    } else if (TempOutput > 0.0 && LoadToCoolingSP > 0.0) {
                        //             If the net heating capacity overshoots the cooling setpoint count as cooling load
                        if (TempOutput > LoadToCoolingSP) {
                            //               Don't count as cooling load unless mode is allowed. Also check for floating zone.
                            if (state.dataHeatBalFanSys->TempControlType(ThisZoneNum) != HVAC::SetptType::SingleHeat &&
                                state.dataHeatBalFanSys->TempControlType(ThisZoneNum) != HVAC::SetptType::Uncontrolled) {
                                if (!state.dataHVACVarRefFlow->LastModeCooling(VRFCond)) {
                                    if (state.dataHVACVarRefFlow->VRFTU(TUIndex).OAMixerUsed) {
                                        state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUOAMixerRetNodeNum).MassFlowRate =
                                            state.dataHVACVarRefFlow->VRFTU(TUIndex).MaxCoolAirMassFlow;
                                        state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUOAMixerOANodeNum).MassFlowRate =
                                            state.dataHVACVarRefFlow->VRFTU(TUIndex).CoolOutAirMassFlow;
                                        MixedAir::SimOAMixer(state,
                                                             state.dataHVACVarRefFlow->VRFTU(TUIndex).OAMixerName,
                                                             state.dataHVACVarRefFlow->VRFTU(TUIndex).OAMixerIndex);
                                    } else {
                                        state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(TUIndex).VRFTUInletNodeNum).MassFlowRate =
                                            state.dataHVACVarRefFlow->VRFTU(TUIndex).MaxCoolAirMassFlow;
                                    }

                                    if (state.dataHVACVarRefFlow->VRF(VRFCond).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                                        // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                                        state.dataHVACVarRefFlow->VRFTU(TUIndex).CalcVRF_FluidTCtrl(
                                            state, TUIndex, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                                    } else {
                                        // Algorithm Type: VRF model based on system curve
                                        state.dataHVACVarRefFlow->VRFTU(TUIndex).CalcVRF(
                                            state, TUIndex, FirstHVACIteration, 0.0, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
                                    }

                                    if (TempOutput > LoadToCoolingSP) {
                                        ++state.dataHVACVarRefFlow->NumCoolingLoads(VRFCond);
                                        state.dataHVACVarRefFlow->SumCoolingLoads(VRFCond) += (LoadToCoolingSP - TempOutput);
                                    }
                                } else {
                                    ++state.dataHVACVarRefFlow->NumCoolingLoads(VRFCond);
                                    state.dataHVACVarRefFlow->SumCoolingLoads(VRFCond) += (LoadToCoolingSP - TempOutput);
                                }
                            }
                        } else if (TempOutput > ZoneLoad) {
                            // do nothing, zone will float
                        } else if (ZoneLoad > 0.0) {
                            ++state.dataHVACVarRefFlow->NumHeatingLoads(VRFCond);
                            state.dataHVACVarRefFlow->SumHeatingLoads(VRFCond) += ZoneLoad;
                        }
                        //           ELSE there is no overshoot and the zone has a valid cooling load
                    } else if (ZoneLoad < 0.0) {
                        ++state.dataHVACVarRefFlow->NumCoolingLoads(VRFCond);
                        state.dataHVACVarRefFlow->SumCoolingLoads(VRFCond) += ZoneLoad;
                        // ELSE there is no overshoot and the zone has a valid heating load
                    } else if (ZoneLoad > 0.0) {
                        ++state.dataHVACVarRefFlow->NumHeatingLoads(VRFCond);
                        state.dataHVACVarRefFlow->SumHeatingLoads(VRFCond) += ZoneLoad;
                    }
                } else { // is cycling fan
                    if (ZoneLoad > 0.0) {
                        ++state.dataHVACVarRefFlow->NumHeatingLoads(VRFCond);
                        state.dataHVACVarRefFlow->SumHeatingLoads(VRFCond) += ZoneLoad;
                    } else if (ZoneLoad < 0.0) {
                        ++state.dataHVACVarRefFlow->NumCoolingLoads(VRFCond);
                        state.dataHVACVarRefFlow->SumCoolingLoads(VRFCond) += ZoneLoad;
                    }
                }
            }
        }
    }

    // Determine operating mode based on VRF type and thermostat control selection
    switch (state.dataHVACVarRefFlow->VRF(VRFCond).ThermostatPriority) {
    case ThermostatCtrlType::ThermostatOffsetPriority: {
        if (state.dataHVACVarRefFlow->MaxDeltaT(VRFCond) > std::abs(state.dataHVACVarRefFlow->MinDeltaT(VRFCond)) &&
            state.dataHVACVarRefFlow->MaxDeltaT(VRFCond) > 0.0) {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = true;
        } else if (state.dataHVACVarRefFlow->MaxDeltaT(VRFCond) < std::abs(state.dataHVACVarRefFlow->MinDeltaT(VRFCond)) &&
                   state.dataHVACVarRefFlow->MinDeltaT(VRFCond) < 0.0) {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = true;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
        } else {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
        }
    } break;
    case ThermostatCtrlType::LoadPriority: {
        if (state.dataHVACVarRefFlow->SumHeatingLoads(VRFCond) > std::abs(state.dataHVACVarRefFlow->SumCoolingLoads(VRFCond)) &&
            state.dataHVACVarRefFlow->SumHeatingLoads(VRFCond) > 0.0) {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = true;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
        } else if (state.dataHVACVarRefFlow->SumHeatingLoads(VRFCond) <= std::abs(state.dataHVACVarRefFlow->SumCoolingLoads(VRFCond)) &&
                   state.dataHVACVarRefFlow->SumCoolingLoads(VRFCond) < 0.0) {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = true;
        } else {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
        }
    } break;
    case ThermostatCtrlType::ZonePriority: {
        if (state.dataHVACVarRefFlow->NumHeatingLoads(VRFCond) > state.dataHVACVarRefFlow->NumCoolingLoads(VRFCond) &&
            state.dataHVACVarRefFlow->NumHeatingLoads(VRFCond) > 0) {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = true;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
        } else if (state.dataHVACVarRefFlow->NumHeatingLoads(VRFCond) <= state.dataHVACVarRefFlow->NumCoolingLoads(VRFCond) &&
                   state.dataHVACVarRefFlow->NumCoolingLoads(VRFCond) > 0) {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = true;
        } else {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
        }
    } break;
    case ThermostatCtrlType::ScheduledPriority: {
        if (state.dataHVACVarRefFlow->VRF(VRFCond).prioritySched->getCurrentVal() == 0) {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = true;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
        } else if (state.dataHVACVarRefFlow->VRF(VRFCond).prioritySched->getCurrentVal() == 1) {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = true;
        } else {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
        }
    } break;
    case ThermostatCtrlType::MasterThermostatPriority: {
        ZoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataHVACVarRefFlow->VRF(VRFCond).MasterZonePtr).RemainingOutputRequired /
                   state.dataHVACVarRefFlow->VRFTU(state.dataHVACVarRefFlow->VRF(VRFCond).MasterZoneTUIndex).controlZoneMassFlowFrac;
        if (state.dataHVACVarRefFlow->VRFTU(state.dataHVACVarRefFlow->VRF(VRFCond).MasterZoneTUIndex).fanOp == HVAC::FanOp::Continuous) {
            SetCompFlowRate(state, state.dataHVACVarRefFlow->VRF(VRFCond).MasterZoneTUIndex, VRFCond);

            if (state.dataHVACVarRefFlow->VRF(VRFCond).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                state.dataHVACVarRefFlow->VRFTU(state.dataHVACVarRefFlow->VRF(VRFCond).MasterZoneTUIndex)
                    .CalcVRF_FluidTCtrl(state,
                                        state.dataHVACVarRefFlow->VRF(VRFCond).MasterZoneTUIndex,
                                        FirstHVACIteration,
                                        0.0,
                                        TempOutput,
                                        OnOffAirFlowRatio,
                                        SuppHeatCoilLoad);
            } else {
                // Algorithm Type: VRF model based on system curve
                state.dataHVACVarRefFlow->VRFTU(state.dataHVACVarRefFlow->VRF(VRFCond).MasterZoneTUIndex)
                    .CalcVRF(state,
                             state.dataHVACVarRefFlow->VRF(VRFCond).MasterZoneTUIndex,
                             FirstHVACIteration,
                             0.0,
                             TempOutput,
                             OnOffAirFlowRatio,
                             SuppHeatCoilLoad);
            }

            LoadToCoolingSP =
                state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataHVACVarRefFlow->VRF(VRFCond).MasterZonePtr).OutputRequiredToCoolingSP /
                state.dataHVACVarRefFlow->VRFTU(state.dataHVACVarRefFlow->VRF(VRFCond).MasterZoneTUIndex).controlZoneMassFlowFrac;
            LoadToHeatingSP =
                state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataHVACVarRefFlow->VRF(VRFCond).MasterZonePtr).OutputRequiredToHeatingSP /
                state.dataHVACVarRefFlow->VRFTU(state.dataHVACVarRefFlow->VRF(VRFCond).MasterZoneTUIndex).controlZoneMassFlowFrac;
            if (TempOutput < LoadToHeatingSP) {
                state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
                state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = true;
            } else if (TempOutput > LoadToCoolingSP) {
                state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = true;
                state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
            } else {
                state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
                state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
            }
        } else if (ZoneLoad > 0.0) {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = true;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
        } else if (ZoneLoad < 0.0) {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = true;
        } else {
            state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
            state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
        }
    } break;
    case ThermostatCtrlType::FirstOnPriority: {
        // na
    } break;
    default:
        break;
    }

    // limit to one possible mode
    if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond) && state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) {
        state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
    }
}

void LimitTUCapacity(EnergyPlusData &state,
                     int const VRFCond,               // Condenser Unit index
                     int const NumTUInList,           // Number of terminal units in list
                     Real64 const StartingCapacity,   // temporary variable holding condenser capacity [W]
                     const Array1D<Real64> &CapArray, // Array of coil capacities in either cooling or heating mode [W]
                     Real64 &MaxLimit,                // Maximum terminal unit capacity for coils in same operating mode [W]
                     Real64 const AltCapacity,        // temporary variable holding heat recovery capacity [W]
                     const Array1D<Real64> &AltArray, // Array of coil capacities of heat recovery [W]
                     Real64 &AltLimit                 // Maximum terminal unit capacity of heat recovery coils [W]
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   July 2012 (Moved from InitVRF)
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate the maximum allowed terminal unit capacity. Total terminal unit capacity must not
    // exceed the available condenser capacity. This variable, MaxCapacity (passed out to MaxCoolingCapacity
    // or MaxHeatingCapacity), is used to limit the terminal units providing more capacity than allowed.
    // Example: TU loads are 1-ton, 2-ton, 3-ton, and 4-ton connected to a condenser having only 9-tons available.
    // This variable is will be set to 3-tons and the 4-ton terminal unit will be limited to 3-tons
    // (see InitVRF where this variable is reset and CalcVRF where the call to the DX coils passes this argument).

    // METHODOLOGY EMPLOYED:
    // The coils are simulated and summed. This value is compared to the available capacity. If the summed
    // TU capacity is greater than the available capacity, limit the TU's with the highest capacity so that
    // the TU capacity equals the available capacity. The report variable Variable Refrigerant Flow Heat Pump
    // Maximum Terminal Unit Cool/Heating Capacity holds the value for maximum TU capacity. This value may not
    // match the maximum individual coil capacity exactly since the available capacity uses a load weighted
    // average WB temperature to calculate available capacity. When the TU's are limited, this weighting changes.
    // The extra iterations required for these values to converge is considered excessive.
    // If the global flag SimZoneEquipment could be set for 1 additional iteration, these variables would
    // converge more closely (setting this global flag is not yet implemented).

    Real64 RemainingCapacity; // decrement capacity counter to find limiting TU capacity [W]

    // limit TU coil capacity to be equal to the condenser capacity (piping losses already accounted for)
    LimitCoilCapacity(NumTUInList, StartingCapacity, CapArray, MaxLimit);

    // ** add in logic to limit coils operating opposite to mode when heat recovery is used
    // ** this is a hard one since we are here because the system is overloaded. That means
    // ** that we do not know at this point the actual operating capacity or compressor power.
    if (state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed) {
        if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond)) {
            RemainingCapacity = StartingCapacity * (1 + 1 / state.dataHVACVarRefFlow->VRF(VRFCond).CoolingCOP);
            if (AltCapacity > RemainingCapacity) {
                LimitCoilCapacity(NumTUInList, RemainingCapacity, AltArray, AltLimit);
            }
        }
        if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) {
            RemainingCapacity = StartingCapacity / (1 + 1 / state.dataHVACVarRefFlow->VRF(VRFCond).HeatingCOP);
            if (AltCapacity > RemainingCapacity) {
                LimitCoilCapacity(NumTUInList, RemainingCapacity, AltArray, AltLimit);
            }
        }
    }
}

void LimitCoilCapacity(int const NumTUInList,           // Number of terminal units in list
                       Real64 const TotalCapacity,      // temporary variable holding condenser capacity [W]
                       const Array1D<Real64> &CapArray, // Array of coil capacities in either cooling or heating mode [W]
                       Real64 &MaxLimit                 // Maximum terminal unit capacity for coils in same operating mode [W]
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   July 2012 (Moved from InitVRF)
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate the maximum allowed terminal unit capacity. Total terminal unit capacity must not
    // exceed the available condenser capacity. This variable, MaxCapacity (passed out to MaxCoolingCapacity
    // or MaxHeatingCapacity), is used to limit the terminal units providing more capacity than allowed.
    // Example: TU loads are 1-ton, 2-ton, 3-ton, and 4-ton connected to a condenser having only 9-tons available.
    // This variable is will be set to 3-tons and the 4-ton terminal unit will be limited to 3-tons
    // (see InitVRF where this variable is reset and CalcVRF where the call to the DX coils passes this argument).

    // METHODOLOGY EMPLOYED:
    // The coils are simulated and summed. This value is compared to the available capacity. If the summed
    // TU capacity is greater than the available capacity, limit the TU's with the highest capacity so that
    // the TU capacity equals the available capacity. The report variable Variable Refrigerant Flow Heat Pump
    // Maximum Terminal Unit Cool/Heating Capacity holds the value for maximum TU capacity. This value may not
    // match the maximum individual coil capacity exactly since the available capacity uses a load weighted
    // average WB temperature to calculate available capacity. When the TU's are limited, this weighting changes.
    // The extra iterations required for these values to converge is considered excessive.
    // If the global flag SimZoneEquipment could be set for 1 additional iteration, these variables would
    // converge more closely (setting this global flag is not yet implemented).

    int NumTU;                                   // loop counter
    int TempTUIndex;                             // temp variable used to find max terminal unit limit
    int MinOutputIndex;                          // index to TU with lowest load
    Real64 MinOutput;                            // used when finding TU "max" capacity limit
    Real64 RemainingCapacity;                    // decrement capacity counter to find limiting TU capacity [W]
    Array1D<Real64> Temp(NumTUInList, CapArray); // temporary array for processing terminal units
    Array1D<Real64> Temp2(NumTUInList, Temp);    // temporary array for processing terminal units

    RemainingCapacity = TotalCapacity;

    // sort TU capacity from lowest to highest
    for (TempTUIndex = 1; TempTUIndex <= NumTUInList; ++TempTUIndex) {
        MinOutput = Constant::MaxCap;
        for (NumTU = 1; NumTU <= NumTUInList; ++NumTU) {
            if (Temp2(NumTU) < MinOutput) {
                MinOutput = Temp2(NumTU);
                Temp(TempTUIndex) = MinOutput;
                MinOutputIndex = NumTU;
            }
        }
        Temp2(MinOutputIndex) = Constant::MaxCap;
    }

    // find limit of "terminal unit" capacity so that sum of all TU's does not exceed condenser capacity
    // if the terminal unit capacity multiplied by number of remaining TU's does not exceed remaining available, subtract and cycle
    for (TempTUIndex = 1; TempTUIndex <= NumTUInList; ++TempTUIndex) {
        if ((Temp(TempTUIndex) * (NumTUInList - TempTUIndex + 1)) < RemainingCapacity) {
            RemainingCapacity -= Temp(TempTUIndex);
            continue;
        } else {
            // if it does exceed, limit is found
            MaxLimit = RemainingCapacity / (NumTUInList - TempTUIndex + 1);
            break;
        }
    }
}

int GetVRFTUOutAirNode(EnergyPlusData &state, int const VRFTUNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         R. Raustad (copy of B Griffith routine)
    //       DATE WRITTEN   Jan  2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // lookup function for VRF terminal unit OA inlet node

    if (state.dataHVACVarRefFlow->GetVRFInputFlag) {
        GetVRFInput(state);
        state.dataHVACVarRefFlow->GetVRFInputFlag = false;
    }

    if (VRFTUNum > 0 && VRFTUNum <= state.dataHVACVarRefFlow->NumVRFTU) {
        return state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum;
    } else {
        return 0;
    }
}

int GetVRFTUZoneInletAirNode(EnergyPlusData &state, int const VRFTUNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         R. Raustad (copy of B Griffith routine)
    //       DATE WRITTEN   Jan  2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // lookup function for VRF terminal unit zone inlet node

    if (state.dataHVACVarRefFlow->GetVRFInputFlag) {
        GetVRFInput(state);
        state.dataHVACVarRefFlow->GetVRFInputFlag = false;
    }

    if (VRFTUNum > 0 && VRFTUNum <= state.dataHVACVarRefFlow->NumVRFTU) {
        return state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOutletNodeNum;
    } else {
        return 0;
    }
}

int GetVRFTUOutAirNodeFromName(EnergyPlusData &state, std::string const &VRFTUName, bool &errorsFound)
{
    int NodeNum; // return value of node number

    if (state.dataHVACVarRefFlow->GetVRFInputFlag) {
        GetVRFInput(state);
        state.dataHVACVarRefFlow->GetVRFInputFlag = false;
    }

    int WhichVRFTU =
        Util::FindItemInList(VRFTUName, state.dataHVACVarRefFlow->VRFTU, &VRFTerminalUnitEquipment::Name, state.dataHVACVarRefFlow->NumVRFTU);
    if (WhichVRFTU != 0) {
        NodeNum = state.dataHVACVarRefFlow->VRFTU(WhichVRFTU).VRFTUOutletNodeNum;
    } else {
        ShowSevereError(state, format("GetVRFTUOutAirNodeFromName: Could not find VRF TU = \"{}\"", VRFTUName));
        errorsFound = true;
        NodeNum = 0;
    }

    return NodeNum;
}

int GetVRFTUInAirNodeFromName(EnergyPlusData &state, std::string const &VRFTUName, bool &errorsFound)
{
    int NodeNum; // return value of node number

    if (state.dataHVACVarRefFlow->GetVRFInputFlag) {
        GetVRFInput(state);
        state.dataHVACVarRefFlow->GetVRFInputFlag = false;
    }

    int WhichVRFTU =
        Util::FindItemInList(VRFTUName, state.dataHVACVarRefFlow->VRFTU, &VRFTerminalUnitEquipment::Name, state.dataHVACVarRefFlow->NumVRFTU);
    if (WhichVRFTU != 0) {
        NodeNum = state.dataHVACVarRefFlow->VRFTU(WhichVRFTU).VRFTUInletNodeNum;
    } else {
        ShowSevereError(state, format("GetVRFTUInAirNodeFromName: Could not find VRF TU = \"{}\"", VRFTUName));
        errorsFound = true;
        NodeNum = 0;
    }

    return NodeNum;
}

int GetVRFTUMixedAirNode(EnergyPlusData &state, int const VRFTUNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         R. Raustad (copy of B Griffith routine)
    //       DATE WRITTEN   Jan  2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // lookup function for VRF terminal unit mixed air node

    if (state.dataHVACVarRefFlow->GetVRFInputFlag) {
        GetVRFInput(state);
        state.dataHVACVarRefFlow->GetVRFInputFlag = false;
    }

    if (VRFTUNum > 0 && VRFTUNum <= state.dataHVACVarRefFlow->NumVRFTU) {
        return state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerOANodeNum;
    } else {
        return 0;
    }
}

int GetVRFTUReturnAirNode(EnergyPlusData &state, int const VRFTUNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         R. Raustad (copy of B Griffith routine)
    //       DATE WRITTEN   Jan  2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // lookup function for VRF terminal unit return air node

    if (state.dataHVACVarRefFlow->GetVRFInputFlag) {
        GetVRFInput(state);
        state.dataHVACVarRefFlow->GetVRFInputFlag = false;
    }

    if (VRFTUNum > 0 && VRFTUNum <= state.dataHVACVarRefFlow->NumVRFTU) {
        return state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOAMixerRetNodeNum;
    } else {
        return 0;
    }
}

int getEqIndex(EnergyPlusData &state, std::string_view VRFTUName)
{
    if (state.dataHVACVarRefFlow->GetVRFInputFlag) {
        GetVRFInput(state);
        state.dataHVACVarRefFlow->GetVRFInputFlag = false;
    }

    for (int VRFTUNum = 1; VRFTUNum <= state.dataHVACVarRefFlow->NumVRFTU; VRFTUNum++) {
        if (Util::SameString(VRFTUName, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).Name)) {
            return VRFTUNum;
        }
    }
    return 0;
}

void getVRFTUZoneLoad(
    EnergyPlusData &state, int const VRFTUNum, Real64 &zoneLoad, Real64 &LoadToHeatingSP, Real64 &LoadToCoolingSP, bool const InitFlag)
{

    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).zoneSequenceCoolingNum > 0 &&
        state.dataHVACVarRefFlow->VRFTU(VRFTUNum).zoneSequenceHeatingNum > 0 && state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isInAirLoop) {
        // air loop equipment uses sequenced variables
        LoadToCoolingSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum)
                              .SequencedOutputRequiredToCoolingSP(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).zoneSequenceCoolingNum) /
                          state.dataHVACVarRefFlow->VRFTU(VRFTUNum).controlZoneMassFlowFrac;
        LoadToHeatingSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum)
                              .SequencedOutputRequiredToHeatingSP(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).zoneSequenceHeatingNum) /
                          state.dataHVACVarRefFlow->VRFTU(VRFTUNum).controlZoneMassFlowFrac;
        if (LoadToHeatingSP > 0.0 && LoadToCoolingSP > 0.0 &&
            state.dataHeatBalFanSys->TempControlType(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum) != HVAC::SetptType::SingleCool) {
            zoneLoad = LoadToHeatingSP;
        } else if (LoadToHeatingSP > 0.0 && LoadToCoolingSP > 0.0 &&
                   state.dataHeatBalFanSys->TempControlType(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum) == HVAC::SetptType::SingleCool) {
            zoneLoad = 0.0;
        } else if (LoadToHeatingSP < 0.0 && LoadToCoolingSP < 0.0 &&
                   state.dataHeatBalFanSys->TempControlType(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum) != HVAC::SetptType::SingleHeat) {
            zoneLoad = LoadToCoolingSP;
        } else if (LoadToHeatingSP < 0.0 && LoadToCoolingSP < 0.0 &&
                   state.dataHeatBalFanSys->TempControlType(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum) == HVAC::SetptType::SingleHeat) {
            zoneLoad = 0.0;
        } else if (LoadToHeatingSP <= 0.0 && LoadToCoolingSP >= 0.0) {
            zoneLoad = 0.0;
        }
    } else if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum > 0) {
        // zone equipment uses Remaining* variables
        if (InitFlag) {
            // this will need more investigation. Using Remaining* variable during the initial load calculation seems wrong.
            // This may also have implications when VRF TUs are in the air loop or if SP control is used
            // another question is whether initialization of the operating mode should look at TotalOutputRequired or RemainingOutputRequired
            zoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum).RemainingOutputRequired /
                       state.dataHVACVarRefFlow->VRFTU(VRFTUNum).controlZoneMassFlowFrac;
            LoadToCoolingSP =
                state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum).OutputRequiredToCoolingSP /
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).controlZoneMassFlowFrac;
            LoadToHeatingSP =
                state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum).OutputRequiredToHeatingSP /
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).controlZoneMassFlowFrac;
        } else {
            zoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum).RemainingOutputRequired /
                       state.dataHVACVarRefFlow->VRFTU(VRFTUNum).controlZoneMassFlowFrac;
            LoadToCoolingSP =
                state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum).RemainingOutputReqToCoolSP /
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).controlZoneMassFlowFrac;
            LoadToHeatingSP =
                state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneNum).RemainingOutputReqToHeatSP /
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).controlZoneMassFlowFrac;
        }
    } else if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isSetPointControlled) {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).coolSPActive) {
            LoadToCoolingSP = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).coolLoadToSP;
            zoneLoad = LoadToCoolingSP;
            LoadToHeatingSP = 0.0;
        }
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).heatSPActive) {
            LoadToHeatingSP = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).heatLoadToSP;
            zoneLoad = LoadToHeatingSP;
            LoadToCoolingSP = 0.0;
        }
    }
}

bool getVRFTUNodeNumber(EnergyPlusData &state, int const nodeNumber)
{
    for (int vrfTUIndex = 1; vrfTUIndex <= state.dataHVACVarRefFlow->NumVRFTU; ++vrfTUIndex) {
        auto const &vrfTU = state.dataHVACVarRefFlow->VRFTU(vrfTUIndex);

        bool noVrfTUOutdoorAir = false;
        if (vrfTU.CoolOutAirVolFlow == 0 && vrfTU.HeatOutAirVolFlow == 0 && vrfTU.NoCoolHeatOutAirVolFlow == 0) {
            noVrfTUOutdoorAir = true;
        }

        if (noVrfTUOutdoorAir &&
            (nodeNumber == vrfTU.VRFTUInletNodeNum || nodeNumber == vrfTU.VRFTUOutletNodeNum || nodeNumber == vrfTU.fanInletNode ||
             nodeNumber == vrfTU.fanOutletNode || nodeNumber == vrfTU.heatCoilAirOutNode || nodeNumber == vrfTU.coolCoilAirOutNode ||
             nodeNumber == vrfTU.VRFTUOAMixerOANodeNum || nodeNumber == vrfTU.VRFTUOAMixerRelNodeNum || nodeNumber == vrfTU.VRFTUOAMixerRetNodeNum ||
             nodeNumber == vrfTU.VRFTUOAMixerMixedNodeNum || nodeNumber == vrfTU.SuppHeatCoilAirInletNode ||
             nodeNumber == vrfTU.SuppHeatCoilAirOutletNode)) {
            return true;
        }
    }
    return false;
}

void VRFCondenserEquipment::CalcVRFIUTeTc_FluidTCtrl(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         RP Zhang (LBNL), XF Pang (LBNL), Y Yura (Daikin Inc)
    //       DATE WRITTEN   June 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //       This subroutine is part of the new VRF model based on physics, applicable for Fluid Temperature Control.
    //       This subroutine determines the VRF evaporating temperature at cooling mode and the condensing temperature
    //       at heating mode. This is the indoor unit side analysis.

    // METHODOLOGY EMPLOYED:
    //       There are two options to calculate the IU Te/Tc: (1) HighSensible method analyzes the conditions of each IU
    //       and then decide and Te/Tc that can satisfy all the zones (2) TeTcConstant method uses fixed values provided
    //       by the user.

    // Followings for FluidTCtrl Only
    Array1D<Real64> EvapTemp;
    Array1D<Real64> CondTemp;
    Real64 IUMinEvapTemp;
    Real64 IUMaxCondTemp;

    int TUListNum = this->ZoneTUListPtr;
    EvapTemp.allocate(state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).NumTUInList);
    CondTemp.allocate(state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).NumTUInList);
    IUMinEvapTemp = 100.0;
    IUMaxCondTemp = 0.0;

    if (this->AlgorithmIUCtrl == 1) {
        // 1. HighSensible: analyze the conditions of each IU

        for (int i = 1; i <= state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).NumTUInList; i++) {
            int VRFTUNum = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(i);
            // analyze the conditions of each IU
            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRFIUVariableTeTc(state, EvapTemp(i), CondTemp(i));

            // select the Te/Tc that can satisfy all the zones
            IUMinEvapTemp = min(IUMinEvapTemp, EvapTemp(i), this->IUEvapTempHigh);
            IUMaxCondTemp = max(IUMaxCondTemp, CondTemp(i), this->IUCondTempLow);
        }

        this->IUEvaporatingTemp = max(IUMinEvapTemp, this->IUEvapTempLow);
        this->IUCondensingTemp = min(IUMaxCondTemp, this->IUCondTempHigh);

    } else {
        // 2. TeTcConstant: use fixed values provided by the user
        this->IUEvaporatingTemp = this->EvapTempFixed;
        this->IUCondensingTemp = this->CondTempFixed;
    }
}

void VRFTerminalUnitEquipment::CalcVRFIUVariableTeTc(EnergyPlusData &state,
                                                     Real64 &EvapTemp, // evaporating temperature
                                                     Real64 &CondTemp  // condensing temperature
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Xiufeng Pang, LBNL
    //       DATE WRITTEN   Feb 2014
    //       MODIFIED       Jul 2015, RP Zhang, LBNL, Modify the bounds of the Te/Tc
    //       MODIFIED       Nov 2015, RP Zhang, LBNL, take into account OA in Te/Tc determination
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //       Calculate the VRF IU Te (cooling mode) and Tc (heating mode), given zonal loads.

    // METHODOLOGY EMPLOYED:
    //       A new physics based VRF model applicable for Fluid Temperature Control.

    using namespace DataZoneEnergyDemands;
    using Psychrometrics::PsyHFnTdbW;
    using SingleDuct::SimATMixer;

    int CoolCoilNum;             // index to the VRF Cooling DX coil to be simulated
    int HeatCoilNum;             // index to the VRF Heating DX coil to be simulated
    int IndexToTUInTUList;       // index to TU in specific list for the VRF system
    int TUListIndex;             // index to TU list for this VRF system
    int VRFNum;                  // index to VRF that the VRF Terminal Unit serves
    int VRFInletNode;            // VRF inlet node number
    Real64 BFC;                  // Bypass factor at the cooling mode (-)
    Real64 BFH;                  // Bypass factor at the heating mode (-)
    Real64 C1Tevap;              // Coefficient for indoor unit coil evaporating temperature curve (-)
    Real64 C2Tevap;              // Coefficient for indoor unit coil evaporating temperature curve (-)
    Real64 C3Tevap;              // Coefficient for indoor unit coil evaporating temperature curve (-)
    Real64 C1Tcond;              // Coefficient for indoor unit coil condensing temperature curve (-)
    Real64 C2Tcond;              // Coefficient for indoor unit coil condensing temperature curve (-)
    Real64 C3Tcond;              // Coefficient for indoor unit coil condensing temperature curve (-)
    Real64 CondTempMin;          // Min condensing temperature (C)
    Real64 CondTempMax;          // Max condensing temperature, correspond to the maximum heating capacity (C)
    Real64 DeltaT;               // Difference between evaporating/condensing temperature and coil surface temperature (C)
    Real64 EvapTempMax;          // Max evaporating temperature (C)
    Real64 EvapTempMin;          // Min evaporating temperature, correspond to the maximum cooling capacity (C)
    Real64 Garate;               // Nominal air mass flow rate
    Real64 H_coil_in;            // Air enthalpy at the coil inlet (kJ/kg)
    Real64 QZnReqSenCoolingLoad; // Zone required sensible cooling load (W)
    Real64 QZnReqSenHeatingLoad; // Zone required sensible heating load (W)
    Real64 RHsat;                // Relative humidity of the air at saturated condition(-)
    Real64 SH;                   // Super heating degrees (C)
    Real64 SC;                   // Subcooling degrees (C)
    Real64 T_coil_in;            // Temperature of the air at the coil inlet, after absorbing the heat released by fan (C)
    Real64 T_TU_in;              // Air temperature at the indoor unit inlet (C)
    Real64 Tout;                 // Air temperature at the indoor unit outlet (C)
    Real64 Th2;                  // Air temperature at the coil surface (C)
    Real64 W_coil_in;            // coil inlet air humidity ratio [kg/kg]
    Real64 W_TU_in;              // Air humidity ratio at the indoor unit inlet[kg/kg]

    // Get the equipment/zone index corresponding to the VRFTU
    CoolCoilNum = this->CoolCoilIndex;
    HeatCoilNum = this->HeatCoilIndex;
    VRFNum = this->VRFSysNum;
    TUListIndex = state.dataHVACVarRefFlow->VRF(VRFNum).ZoneTUListPtr;
    IndexToTUInTUList = this->IndexToTUInTUList;

    // Bounds of Te/Tc for VRF IU Control Algorithm: VariableTemp
    EvapTempMin = state.dataHVACVarRefFlow->VRF(VRFNum).IUEvapTempLow;
    EvapTempMax = state.dataHVACVarRefFlow->VRF(VRFNum).IUEvapTempHigh;
    CondTempMin = state.dataHVACVarRefFlow->VRF(VRFNum).IUCondTempLow;
    CondTempMax = state.dataHVACVarRefFlow->VRF(VRFNum).IUCondTempHigh;

    // Coefficients describing coil performance
    SH = state.dataDXCoils->DXCoil(CoolCoilNum).SH;
    SC = state.dataDXCoils->DXCoil(HeatCoilNum).SC;
    C1Tevap = state.dataDXCoils->DXCoil(CoolCoilNum).C1Te;
    C2Tevap = state.dataDXCoils->DXCoil(CoolCoilNum).C2Te;
    C3Tevap = state.dataDXCoils->DXCoil(CoolCoilNum).C3Te;
    C1Tcond = state.dataDXCoils->DXCoil(HeatCoilNum).C1Tc;
    C2Tcond = state.dataDXCoils->DXCoil(HeatCoilNum).C2Tc;
    C3Tcond = state.dataDXCoils->DXCoil(HeatCoilNum).C3Tc;

    VRFInletNode = this->VRFTUInletNodeNum;
    T_TU_in = state.dataLoopNodes->Node(VRFInletNode).Temp;
    W_TU_in = state.dataLoopNodes->Node(VRFInletNode).HumRat;
    T_coil_in = this->coilInNodeT;
    W_coil_in = this->coilInNodeW;

    Garate = state.dataHVACVarRefFlow->CompOnMassFlow;
    H_coil_in = PsyHFnTdbW(T_coil_in, W_coil_in);
    RHsat = 0.98;
    BFC = 0.0592;
    BFH = 0.136;
    Real64 ZoneLoad = 0.0;
    Real64 LoadToHeatingSP = 0.0;
    Real64 LoadToCoolingSP = 0.0;

    // 1. COOLING Mode
    if ((Garate > 0.0) && ((!state.dataHVACVarRefFlow->VRF(VRFNum).HeatRecoveryUsed && state.dataHVACVarRefFlow->CoolingLoad(VRFNum)) ||
                           (state.dataHVACVarRefFlow->VRF(VRFNum).HeatRecoveryUsed &&
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList)))) {
        // 1.1) Cooling coil is running
        getVRFTUZoneLoad(state, IndexToTUInTUList, ZoneLoad, LoadToHeatingSP, LoadToCoolingSP, false);
        QZnReqSenCoolingLoad = max(0.0, -1.0 * LoadToCoolingSP);
        Tout = T_TU_in - QZnReqSenCoolingLoad * 1.2 / Garate / 1005;
        Th2 = T_coil_in - (T_coil_in - Tout) / (1 - BFC);
        DeltaT = C3Tevap * SH * SH + C2Tevap * SH + C1Tevap;
        EvapTemp = max(min((Th2 - DeltaT), EvapTempMax), EvapTempMin);

    } else {
        // 1.2) Cooling coil is not running
        EvapTemp = T_coil_in;
    }

    // 2. HEATING Mode
    if ((Garate > 0.0) && ((!state.dataHVACVarRefFlow->VRF(VRFNum).HeatRecoveryUsed && state.dataHVACVarRefFlow->HeatingLoad(VRFNum)) ||
                           (state.dataHVACVarRefFlow->VRF(VRFNum).HeatRecoveryUsed &&
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList)))) {
        // 2.1) Heating coil is running
        getVRFTUZoneLoad(state, IndexToTUInTUList, ZoneLoad, LoadToHeatingSP, LoadToCoolingSP, false);
        QZnReqSenHeatingLoad = max(0.0, LoadToHeatingSP);
        Tout = T_TU_in + QZnReqSenHeatingLoad / Garate / 1005;
        Th2 = T_coil_in + (Tout - T_coil_in) / (1 - BFH);
        DeltaT = C3Tcond * SC * SC + C2Tcond * SC + C1Tcond;
        CondTemp = max(min((Th2 + DeltaT), CondTempMax), CondTempMin);
    } else {
        // 2.2) Heating coil is not running
        CondTemp = T_coil_in;
    }
}

void VRFCondenserEquipment::CalcVRFCondenser_FluidTCtrl(EnergyPlusData &state, const bool FirstHVACIteration)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         RP Zhang (LBNL), XF Pang (LBNL), Y Yura (Daikin Inc)
    //       DATE WRITTEN   June 2015
    //       MODIFIED       Feb 2016, RP Zhang, add the control logics for VRF-HR operations
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //       This subroutine is part of the new VRF model based on physics, applicable for Fluid Temperature Control.
    //       This is adapted from subroutine CalcVRFCondenser, which is part of the VRF model based on system curves.
    //       This subroutine models the interactions of VRF indoor units with the outdoor unit.
    //       The indoor terminal units are simulated first, and then the outdoor unit is simulated.

    // METHODOLOGY EMPLOYED:
    //       A new physics based VRF model applicable for Fluid Temperature Control.

    using Curve::CurveValue;
    using General::SolveRoot;

    using PlantUtilities::SetComponentFlowRate;
    using Psychrometrics::RhoH2O;

    static constexpr std::string_view RoutineName("CalcVRFCondenser_FluidTCtrl");

    int VRFCond;            // index to VRF condenser
    int TUListNum;          // index to TU List
    int NumTUInList;        // number of terminal units is list
    int NumTU;              // index for loop on terminal units
    int TUIndex;            // Index to terminal unit
    int CoolCoilIndex;      // index to cooling coil in terminal unit
    int HeatCoilIndex;      // index to heating coil in terminal unit
    int NumTUInCoolingMode; // number of terminal units actually cooling
    int NumTUInHeatingMode; // number of terminal units actually heating

    Real64 TUParasiticPower;          // total terminal unit parasitic power (W)
    Real64 TUFanPower;                // total terminal unit fan power (W)
    Real64 InletAirWetBulbC;          // coil inlet air wet-bulb temperature (C)
    Real64 InletAirDryBulbC;          // coil inlet air dry-bulb temperature (C)
    Real64 CondInletTemp;             // condenser inlet air temperature (C)
    Real64 OutdoorDryBulb;            // outdoor dry-bulb temperature (C)
    Real64 OutdoorHumRat;             // outdoor humidity ratio (kg/kg)
    Real64 OutdoorPressure;           // outdoor pressure (Pa)
    Real64 OutdoorWetBulb;            // outdoor wet-bulb temperature (C)
    Real64 SumCoolInletWB;            // sum of active TU's DX cooling coil inlet air wet-bulb temperature
    Real64 SumHeatInletDB;            // sum of active TU's DX heating coil inlet air dry-bulb temperature
    Real64 SumHeatInletWB;            // sum of active TU's DX heating coil inlet air wet-bulb temperature
    Real64 TotalTUCoolingCapacity;    // sum of TU's cooling capacity (W)
    Real64 TotalTUHeatingCapacity;    // sum of TU's heating capacity (W)
    Real64 TotalCondCoolingCapacity;  // total available condenser cooling capacity (W)
    Real64 TotalCondHeatingCapacity;  // total available condenser heating capacity (W)
    Real64 CoolingPLR;                // condenser cooling PLR
    Real64 HeatingPLR;                // condenser heating PLR
    Real64 CyclingRatio;              // cycling ratio of condenser's compressors
    int Stage;                        // used for crankcase heater power calculation
    Real64 UpperStageCompressorRatio; // used for crankcase heater power calculation
    Real64 RhoAir;                    // Density of air [kg/m3]
    Real64 PartLoadFraction;          // Part load fraction from PLFFPLR curve
    Real64 VRFRTF;                    // VRF runtime fraction when cycling below MINPLR
    Real64 OutdoorCoilT;              // Outdoor coil temperature (C)
    Real64 OutdoorCoildw;             // Outdoor coil delta w assuming coil temp of OutdoorCoilT (kg/kg)
    Real64 FractionalDefrostTime;     // Fraction of time step system is in defrost
    Real64 HeatingCapacityMultiplier; // Multiplier for heating capacity when system is in defrost
    Real64 InputPowerMultiplier;      // Multiplier for power when system is in defrost
    Real64 LoadDueToDefrost;          // Additional load due to defrost
    Real64 DefrostEIRTempModFac;      // EIR modifier for defrost (function of entering drybulb, outside wetbulb)
    Real64 HRInitialCapFrac;          // Fractional cooling degradation at the start of heat recovery from cooling mode
    Real64 HRCapTC;                   // Time constant used to recover from initial degradation in cooling heat recovery
    Real64 HRInitialEIRFrac;          // Fractional cooling degradation at the start of heat recovery from cooling mode
    Real64 HREIRTC;                   // Time constant used to recover from initial degradation in cooling heat recovery
    Real64 CurrentEndTime;            // end time of current time step
    Real64 SUMultiplier;              // multiplier for simulating mode changes
    Real64 CondPower;                 // condenser power [W]
    Real64 CondCapacity;              // condenser heat rejection [W]
    Real64 TotPower;                  // total condenser power use [W]
    bool HRHeatRequestFlag;           // flag indicating VRF TU could operate in heating mode
    bool HRCoolRequestFlag;           // flag indicating VRF TU could operate in cooling mode

    // Followings for VRF FluidTCtrl Only
    int Counter;                     // index for iterations [-]
    int NumIteHIUIn;                 // index for HIU calculation iterations [-]
    int NumOfCompSpdInput;           // Number of compressor speed input by the user [-]
    Real64 CompSpdActual;            // Actual compressor running speed [rps]
    Real64 C_cap_operation;          // Compressor capacity modification algorithm_modified Cap [-]
    Real64 CompEvaporatingCAPSpdMin; // evaporating capacity at the lowest compressor speed [W]
    Real64 CompEvaporatingCAPSpdMax; // evaporating capacity at the highest compressor speed [W]
    Real64 CompEvaporatingPWRSpdMin; // compressor power at the lowest compressor speed [W]
    Real64 CompEvaporatingPWRSpdMax; // compressor power at the highest compressor speed [W]
    Real64 CapMaxTe;                 // maximum Te during operation, for capacity calculations [C]
    Real64 CapMinTe;                 // minimum Te during operation, for capacity calculations [C]
    Real64 CapMinPe;                 // minimum Pe during operation, for capacity calculations [Pa]
    Real64 CapMaxTc;                 // maximum Tc during operation, for capacity calculations [C]
    Real64 CapMaxPc;                 // maximum Pc during operation, for capacity calculations [Pa]
    Real64 CapMinTc;                 // minimum Tc during operation, for capacity calculations [C]
    Real64 CapMinPc;                 // minimum Pc during operation, for capacity calculations [Pa]
    Real64 h_IU_evap_in;             // enthalpy of IU evaporator at inlet [kJ/kg]
    Real64 h_IU_evap_in_new;         // enthalpy of IU evaporator at inlet (new) [kJ/kg]
    Real64 h_IU_evap_in_low;         // enthalpy of IU evaporator at inlet (low) [kJ/kg]
    Real64 h_IU_evap_in_up;          // enthalpy of IU evaporator at inlet (up) [kJ/kg]
    Real64 h_IU_evap_out;            // enthalpy of IU evaporator at outlet [kJ/kg]
    Real64 h_IU_evap_out_i;          // enthalpy of IU evaporator at outlet (individual) [kJ/kg]
    Real64 h_IU_cond_in;             // enthalpy of IU condenser at inlet [kJ/kg]
    Real64 h_IU_cond_in_low;         // enthalpy of IU condenser at inlet (low) [kJ/kg]
    Real64 h_IU_cond_in_up;          // enthalpy of IU condenser at inlet (up) [kJ/kg]
    Real64 h_IU_cond_out;            // enthalpy of IU condenser at outlet [kJ/kg]
    Real64 h_IU_cond_out_i;          // enthalpy of IU condenser at outlet (individual) [kJ/kg]
    Real64 h_IU_cond_out_ave;        // average enthalpy of the refrigerant leaving IU condensers [kJ/kg]
    Real64 h_IU_PLc_out;             // enthalpy of refrigerant at the outlet of IU evaporator side main pipe, after piping loss (c) [kJ/kg]
    Real64 h_comp_in;                // enthalpy of refrigerant at compressor inlet, after piping loss (c) [kJ/kg]
    Real64 h_comp_in_new;            // enthalpy of refrigerant at compressor inlet (new) [kJ/kg]
    Real64 h_comp_out;               // enthalpy of refrigerant at compressor outlet [kJ/kg]
    Real64 h_comp_out_new;           // enthalpy of refrigerant at compressor outlet (new) [kJ/kg]
    Real64 m_air;                    // OU coil air mass flow rate [kg/s]
    Real64 m_ref_IU_cond;            // mass flow rate of Refrigerant through IU condensers [kg/s]
    Real64 m_ref_IU_cond_i;          // mass flow rate of Refrigerant through an individual IU condenser [kg/s]
    Real64 m_ref_IU_evap;            // mass flow rate of Refrigerant through IU evaporators [kg/s]
    Real64 m_ref_IU_evap_i;          // mass flow rate of Refrigerant through an individual IU evaporator [kg/s]
    Real64 m_ref_OU_evap;            // mass flow rate of Refrigerant through OU evaporator [kg/s]
    Real64 m_ref_OU_cond;            // mass flow rate of Refrigerant through OU condenser [kg/s]
    Real64 Ncomp;                    // compressor power [W]
    Real64 Ncomp_new;                // compressor power for temporary use in iterations [W]
    Real64 P_comp_in;                // pressure of refrigerant at IU condenser outlet [Pa]
    Real64 Pcond;                    // VRF condensing pressure [Pa]
    Real64 Pevap;                    // VRF evaporating pressure [Pa]
    Real64 Pdischarge;               // VRF compressor discharge pressure [Pa]
    Real64 Psuction;                 // VRF compressor suction pressure [Pa]
    Real64 Pipe_DeltP_c;             // Piping Loss Algorithm Parameter: Pipe pressure drop (c) [Pa]
    Real64 Pipe_DeltP_h;             // Piping Loss Algorithm Parameter: Pipe pressure drop (h) [Pa]
    Real64 Pipe_Q_c;                 // Piping Loss Algorithm Parameter: Heat loss (c) [W]
    Real64 Pipe_Q_h;                 // Piping Loss Algorithm Parameter: Heat loss (h) [W]
    Real64 Q_c_TU_PL;                // Cooling load to be met at cooling mode, including the piping loss(W)
    Real64 Q_h_TU_PL;                // Heating load to be met at heating mode, including the piping loss (W)
    Real64 Q_h_OU;                   // outdoor unit condenser heat release (cooling mode) [W]
    Real64 Q_c_OU;                   // outdoor unit evaporator heat extract (heating mode) [W]
    Real64 RefMaxPc;                 // maximum refrigerant condensing pressure [Pa]
    Real64 RefMinTe;                 // Minimum refrigerant evaporating temperature [Pa]
    Real64 RefMinPe;                 // Minimum refrigerant evaporating pressure [Pa]
    Real64 RefPLow;                  // Low Pressure Value for Ps (>0.0) [Pa]
    Real64 RefPHigh;                 // High Pressure Value for Ps (max in tables) [Pa]
    Real64 RefTLow;                  // Low Temperature Value for Ps (max in tables) [C]
    Real64 RefTHigh;                 // High Temperature Value for Ps (max in tables) [C]
    Real64 RefTSat;                  // Saturated temperature of the refrigerant. Used to check whether the refrigerant is in the superheat area [C]
    Real64 SC_IU_merged;             // Piping Loss Algorithm Parameter: average subcooling degrees after the indoor units [C]
    Real64 SH_IU_merged;             // Piping Loss Algorithm Parameter: average super heating degrees after the indoor units [C]
    Real64 SC_OU;                    // subcooling degrees at OU condenser [C]
    Real64 SH_OU;                    // super heating degrees at OU evaporator [C]
    Real64 SH_Comp;                  // Temperature difference between compressor inlet node and Tsuction [C]
    Real64 T_comp_in;                // temperature of refrigerant at compressor inlet, after piping loss (c) [C]
    Real64 TU_HeatingLoad;           // Heating load from terminal units, excluding heating loss [W]
    Real64 TU_CoolingLoad;           // Cooling load from terminal units, excluding heating loss [W]
    Real64 Tdischarge;               // VRF Compressor discharge refrigerant temperature [C]
    Real64 Tsuction;                 // VRF compressor suction refrigerant temperature [C]
    Real64 Tolerance;                // Tolerance for condensing temperature calculation [C]
    Real64 Tfs;                      // Temperature of the air at the coil surface [C]
    Array1D<Real64> CompEvaporatingPWRSpd; // Array for the compressor power at certain speed [W]
    Array1D<Real64> CompEvaporatingCAPSpd; // Array for the evaporating capacity at certain speed [W]

    // variable initializations
    TUListNum = this->ZoneTUListPtr;
    NumTUInList = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).NumTUInList;
    VRFCond = state.dataHVACVarRefFlow->VRFTU(state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(1)).VRFSysNum;
    TU_CoolingLoad = 0.0;
    TU_HeatingLoad = 0.0;
    TUParasiticPower = 0.0;
    TUFanPower = 0.0;
    CoolingPLR = 0.0;
    HeatingPLR = 0.0;
    CyclingRatio = 1.0;
    SumCoolInletWB = 0.0;
    SumHeatInletDB = 0.0;
    SumHeatInletWB = 0.0;
    TotalCondCoolingCapacity = 0.0;
    TotalCondHeatingCapacity = 0.0;
    TotalTUCoolingCapacity = 0.0;
    TotalTUHeatingCapacity = 0.0;
    NumTUInCoolingMode = 0;
    NumTUInHeatingMode = 0;
    Tolerance = 0.05;
    Counter = 1;
    NumIteHIUIn = 1;
    this->ElecCoolingPower = 0.0;
    this->ElecHeatingPower = 0.0;
    this->CrankCaseHeaterPower = 0.0;
    this->EvapCondPumpElecPower = 0.0; // for EvaporativelyCooled condenser
    this->EvapWaterConsumpRate = 0.0;
    this->DefrostPower = 0.0;
    this->OperatingCoolingCOP = 0.0;
    this->OperatingHeatingCOP = 0.0;
    this->OperatingCOP = 0.0;
    this->SCHE = 0.0;
    this->BasinHeaterPower = 0.0;
    this->CondensingTemp = 60.0; // OutDryBulbTemp;
    this->VRFHeatRec = 0.0;

    // Refrigerant data
    RefMinTe = -15;
    RefMaxPc = 4000000.0;
    RefMinPe = this->refrig->getSatPressure(state, RefMinTe, RoutineName);
    RefTLow = this->refrig->PsLowTempValue;   // High Temperature Value for Ps (max in tables)
    RefTHigh = this->refrig->PsHighTempValue; // High Temperature Value for Ps (max in tables)
    RefPLow = this->refrig->PsLowPresValue;   // Low Pressure Value for Ps (>0.0)
    RefPHigh = this->refrig->PsHighPresValue; // High Pressure Value for Ps (max in tables)

    // sum loads on TU coils
    for (NumTU = 1; NumTU <= NumTUInList; ++NumTU) {
        TU_CoolingLoad += state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU);
        TU_HeatingLoad += state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(NumTU);
        TUParasiticPower +=
            state.dataHVACVarRefFlow->VRFTU(state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU)).ParasiticCoolElecPower +
            state.dataHVACVarRefFlow->VRFTU(state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU)).ParasiticHeatElecPower;
        TUFanPower += state.dataHVACVarRefFlow->VRFTU(state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU)).FanPower;
    }
    this->TUCoolingLoad = TU_CoolingLoad; // this is cooling coil load, not terminal unit load
    this->TUHeatingLoad = TU_HeatingLoad; // this is heating coil load, not terminal unit load

    // loop through TU's and calculate average inlet conditions for active coils
    for (NumTU = 1; NumTU <= NumTUInList; ++NumTU) {
        TUIndex = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
        CoolCoilIndex = state.dataHVACVarRefFlow->VRFTU(TUIndex).CoolCoilIndex;
        HeatCoilIndex = state.dataHVACVarRefFlow->VRFTU(TUIndex).HeatCoilIndex;

        if (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) > 0.0) {
            SumCoolInletWB += state.dataDXCoils->DXCoilCoolInletAirWBTemp(CoolCoilIndex) *
                              state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) / TU_CoolingLoad;
            ++NumTUInCoolingMode;
        }
        if (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) > 0.0) {
            SumHeatInletDB += state.dataDXCoils->DXCoilHeatInletAirDBTemp(HeatCoilIndex) *
                              state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) / TU_HeatingLoad;
            SumHeatInletWB += state.dataDXCoils->DXCoilHeatInletAirWBTemp(HeatCoilIndex) *
                              state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) / TU_HeatingLoad;
            ++NumTUInHeatingMode;
        }
    }

    // set condenser entering air conditions (Outdoor air conditions)
    if (this->CondenserNodeNum != 0) {
        OutdoorDryBulb = state.dataLoopNodes->Node(this->CondenserNodeNum).Temp;
        if (this->CondenserType != DataHeatBalance::RefrigCondenserType::Water) {
            OutdoorHumRat = state.dataLoopNodes->Node(this->CondenserNodeNum).HumRat;
            OutdoorPressure = state.dataLoopNodes->Node(this->CondenserNodeNum).Press;
            OutdoorWetBulb = state.dataLoopNodes->Node(this->CondenserNodeNum).OutAirWetBulb;
        } else {
            OutdoorHumRat = state.dataEnvrn->OutHumRat;
            OutdoorPressure = state.dataEnvrn->OutBaroPress;
            OutdoorWetBulb = state.dataEnvrn->OutWetBulbTemp;
        }
    } else {
        OutdoorDryBulb = state.dataEnvrn->OutDryBulbTemp;
        OutdoorHumRat = state.dataEnvrn->OutHumRat;
        OutdoorPressure = state.dataEnvrn->OutBaroPress;
        OutdoorWetBulb = state.dataEnvrn->OutWetBulbTemp;
    }
    RhoAir = PsyRhoAirFnPbTdbW(state, OutdoorPressure, OutdoorDryBulb, OutdoorHumRat);

    CondInletTemp = OutdoorDryBulb; // this->CondenserType == AirCooled
    this->CondenserInletTemp = CondInletTemp;

    //*************
    // VRF-HP MODES:
    //     1. Cooling
    //     2. Heating
    //     3. No running
    // VRF-HR MODES:
    //     1. Cooling Only
    //     2. Cooling Dominant w/o HR Loss
    //     3. Cooling Dominant w/ HR Loss
    //     4. Heating Dominant w/ HR Loss
    //     5. Heating Dominant w/o HR Loss
    //     6. Heating Only
    //     7. No running

    // Flag for VRF-HR Operations
    if (TU_HeatingLoad > 0) {
        HRHeatRequestFlag = true;
    } else {
        state.dataHVACVarRefFlow->HeatingLoad(VRFCond) = false;
        HRHeatRequestFlag = false;
    }
    if (TU_CoolingLoad > 0) {
        HRCoolRequestFlag = true;
    } else {
        state.dataHVACVarRefFlow->CoolingLoad(VRFCond) = false;
        HRCoolRequestFlag = false;
    }

    // Initialization for Ncomp iterations
    NumOfCompSpdInput = this->CompressorSpeed.size();
    CompEvaporatingPWRSpd.dimension(NumOfCompSpdInput);
    CompEvaporatingCAPSpd.dimension(NumOfCompSpdInput);
    this->OperatingMode = 0; // report variable for heating or cooling mode

    // 1. VRF-HP Cooling Mode .OR. VRF-HR Mode_1
    if ((!this->HeatRecoveryUsed && state.dataHVACVarRefFlow->CoolingLoad(VRFCond)) ||
        (this->HeatRecoveryUsed && !HRHeatRequestFlag && HRCoolRequestFlag)) {

        this->OperatingMode = ModeCoolingOnly;
        this->VRFOperationSimPath = 10;

        // Initialization of VRF-FluidTCtrl Model
        Q_c_TU_PL = TU_CoolingLoad;

        // Evaporator (IU side) operational parameters
        Pevap = this->refrig->getSatPressure(state, this->IUEvaporatingTemp, RoutineName);
        Psuction = Pevap;
        this->EvaporatingTemp =
            this->IUEvaporatingTemp; // GetSatTemperatureRefrig(state,  this->RefrigerantName, max( min( Pevap, RefPHigh ), RefPLow
                                     // ), RefrigerantIndex, RoutineName );

        // Condenser (OU side) operation ranges
        CapMaxPc = min(Psuction + this->CompMaxDeltaP, RefMaxPc);
        CapMaxTc = this->refrig->getSatTemperature(state, max(min(CapMaxPc, RefPHigh), RefPLow), RoutineName);
        CapMinTc = OutdoorDryBulb + this->SC;
        CapMinPc = this->refrig->getSatPressure(state, CapMinTc, RoutineName);

        // Evaporator (IU side) operation ranges
        CapMinPe = max(CapMinPc - this->CompMaxDeltaP, RefMinPe);
        CapMinTe = this->refrig->getSatTemperature(state, max(min(CapMinPe, RefPHigh), RefPLow), RoutineName);

        // Evaporative capacity ranges
        CompEvaporatingCAPSpdMin = this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(1), CapMinTc, CapMinTe);
        CompEvaporatingPWRSpdMin = this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(1), CapMinTc, CapMinTe);
        CompEvaporatingCAPSpdMax = this->CoffEvapCap * this->RatedEvapCapacity *
                                   CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), this->CondensingTemp, this->IUEvaporatingTemp);
        CompEvaporatingPWRSpdMax =
            this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(NumOfCompSpdInput), this->CondensingTemp, this->IUEvaporatingTemp);

        // Initialization for h_IU_evap_in iterations (Label12)
        h_IU_evap_in_low = this->refrig->getSatEnthalpy(state, OutdoorDryBulb - this->SC, 0.0, RoutineName);  // Tc = Tamb
        h_IU_evap_in_up = this->refrig->getSatEnthalpy(state, CapMaxTc - this->SC, 0.0, RoutineName);         // Tc = CapMaxTc
        h_IU_evap_in = this->refrig->getSatEnthalpy(state, OutdoorDryBulb + 10 - this->SC, 0.0, RoutineName); // Tc = Tamb+10

        NumIteHIUIn = 1;
        bool converged_12;
        do {
            m_ref_IU_evap = 0;
            h_IU_evap_out = 0;
            h_IU_evap_out_i = 0;
            m_ref_IU_evap_i = 0;
            SH_IU_merged = 0;

            // Calculate total IU refrigerant flow rate and SH_IU_merged
            if (Q_c_TU_PL > CompEvaporatingCAPSpdMax) {
                // Required load is beyond the max system capacity

                RefTSat = this->refrig->getSatTemperature(state, max(min(Pevap, RefPHigh), RefPLow), RoutineName);
                h_IU_evap_out = this->refrig->getSupHeatEnthalpy(
                    state, max(RefTSat, this->IUEvaporatingTemp + 3), max(min(Pevap, RefPHigh), RefPLow), RoutineName);
                SH_IU_merged = 3;
                m_ref_IU_evap = TU_CoolingLoad / (h_IU_evap_out - h_IU_evap_in);

            } else {

                for (NumTU = 1; NumTU <= NumTUInList; NumTU++) { // Calc total refrigerant flow rate
                    if (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) > 0) {
                        TUIndex = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
                        CoolCoilIndex = state.dataHVACVarRefFlow->VRFTU(TUIndex).CoolCoilIndex;

                        RefTSat = this->refrig->getSatTemperature(state, max(min(Pevap, RefPHigh), RefPLow), RoutineName);
                        h_IU_evap_out_i = this->refrig->getSupHeatEnthalpy(
                            state,
                            max(RefTSat, this->IUEvaporatingTemp + state.dataDXCoils->DXCoil(CoolCoilIndex).ActualSH),
                            max(min(Pevap, RefPHigh), RefPLow),
                            RoutineName);

                        if (h_IU_evap_out_i > h_IU_evap_in) {
                            m_ref_IU_evap_i = (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) <= 0.0)
                                                  ? 0.0
                                                  : (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) /
                                                     (h_IU_evap_out_i - h_IU_evap_in)); // Ref Flow Rate in the IU( kg/s )
                            m_ref_IU_evap = m_ref_IU_evap + m_ref_IU_evap_i;
                            h_IU_evap_out = h_IU_evap_out + m_ref_IU_evap_i * h_IU_evap_out_i;
                            SH_IU_merged = SH_IU_merged + m_ref_IU_evap_i * state.dataDXCoils->DXCoil(CoolCoilIndex).ActualSH;
                        }
                    }
                }
                if (m_ref_IU_evap > 0) {
                    h_IU_evap_out = h_IU_evap_out / m_ref_IU_evap;
                    SH_IU_merged = SH_IU_merged / m_ref_IU_evap;
                } else {
                    RefTSat = this->refrig->getSatTemperature(state, max(min(Pevap, RefPHigh), RefPLow), RoutineName);
                    h_IU_evap_out = this->refrig->getSupHeatEnthalpy(
                        state, max(RefTSat, this->IUEvaporatingTemp + 3), max(min(Pevap, RefPHigh), RefPLow), RoutineName);
                    SH_IU_merged = 3;
                    m_ref_IU_evap = TU_CoolingLoad / (h_IU_evap_out - h_IU_evap_in);
                }
            }

            // *Calculate piping loss
            this->VRFOU_PipeLossC(state,
                                  m_ref_IU_evap,
                                  max(min(Pevap, RefPHigh), RefPLow),
                                  h_IU_evap_out,
                                  SH_IU_merged,
                                  OutdoorDryBulb,
                                  Pipe_Q_c,
                                  Pipe_DeltP_c,
                                  h_comp_in);
            Tsuction = this->refrig->getSatTemperature(state, max(min(Pevap - Pipe_DeltP_c, RefPHigh), RefPLow), RoutineName);
            Psuction = Pevap - Pipe_DeltP_c; // This Psuction is used for rps > min; will be updated for rps = min

            // Perform iteration to calculate T_comp_in
            T_comp_in = this->refrig->getSupHeatTemp(
                state, max(min(Pevap - Pipe_DeltP_c, RefPHigh), RefPLow), h_comp_in, Tsuction + 3, Tsuction + 30, RoutineName);
            SH_Comp = T_comp_in - Tsuction; // This is used for rps > min; will be updated for rps = min

            Q_c_TU_PL = TU_CoolingLoad + Pipe_Q_c;
            Q_h_OU = Q_c_TU_PL + CompEvaporatingPWRSpdMin;

            // *Calculate capacity modification factor
            C_cap_operation = this->VRFOU_CapModFactor(
                state, h_comp_in, h_IU_evap_in, max(min(Psuction, RefPHigh), RefPLow), Tsuction + SH_Comp, Tsuction + 8, CapMinTc - 5);

            // Iteration_Ncomp: Perform iterations to calculate Ncomp (Label10)
            Counter = 1;
            Ncomp = TU_CoolingLoad / this->CoolingCOP;
            Ncomp_new = Ncomp;
            bool converged_10;
            do {
                Q_h_OU = Q_c_TU_PL + Ncomp_new; // Ncomp_new may be updated during Iteration_Ncomp Label10

                // *VRF OU TeTc calculations
                m_air = this->OUAirFlowRate * RhoAir;
                SC_OU = this->SC;
                this->VRFOU_TeTc(
                    state, HXOpMode::CondMode, Q_h_OU, SC_OU, m_air, OutdoorDryBulb, OutdoorHumRat, OutdoorPressure, Tfs, this->CondensingTemp);
                this->CondensingTemp = min(CapMaxTc, this->CondensingTemp);
                this->SC = SC_OU;

                // *VEF OU Compressor Simulation at cooling mode: Specify the compressor speed and power consumption
                this->VRFOU_CalcCompC(state,
                                      TU_CoolingLoad,
                                      Tsuction,
                                      this->CondensingTemp,
                                      Psuction,
                                      T_comp_in,
                                      h_comp_in,
                                      h_IU_evap_in,
                                      Pipe_Q_c,
                                      CapMaxTc,
                                      Q_h_OU,
                                      CompSpdActual,
                                      Ncomp,
                                      CyclingRatio);

                converged_10 = (std::abs(Ncomp - Ncomp_new) <= (Tolerance * Ncomp_new)) || (Counter >= 30);
                if (!converged_10) {
                    Ncomp_new = Ncomp;
                }
            } while (!converged_10);

            // Update h_IU_evap_in in iterations Label12
            h_IU_evap_in_new = this->refrig->getSatEnthalpy(state, this->CondensingTemp - this->SC, 0.0, RoutineName);
            converged_12 = !((std::abs(h_IU_evap_in - h_IU_evap_in_new) > Tolerance * h_IU_evap_in) && (h_IU_evap_in < h_IU_evap_in_up) &&
                             (h_IU_evap_in > h_IU_evap_in_low));
            h_IU_evap_in = h_IU_evap_in_new;
            NumIteHIUIn = NumIteHIUIn + 1;
        } while (!converged_12);
        if ((std::abs(h_IU_evap_in - h_IU_evap_in_new) > Tolerance * h_IU_evap_in)) {
            h_IU_evap_in = 0.5 * (h_IU_evap_in_low + h_IU_evap_in_up);
        } else if (h_IU_evap_in > h_IU_evap_in_up) {
            h_IU_evap_in = h_IU_evap_in_up;
        } else if (h_IU_evap_in < h_IU_evap_in_low) {
            h_IU_evap_in = h_IU_evap_in_low;
        } else {
            h_IU_evap_in = (h_IU_evap_in + h_IU_evap_in_new) / 2;
        }

        // Key outputs of this subroutine
        Ncomp *= CyclingRatio;
        Q_h_OU *= CyclingRatio;
        this->CompActSpeed = max(CompSpdActual, 0.0);
        this->Ncomp = max(Ncomp, 0.0) / this->EffCompInverter;   // 0.95 is the efficiency of the compressor inverter, can come from IDF //@minor
        this->OUFanPower = this->RatedOUFanPower * CyclingRatio; //@ * pow_3( CondFlowRatio )
        this->VRFCondCyclingRatio = CyclingRatio;                // report variable for cycling rate

        Tdischarge = this->CondensingTemp; // outdoor unit condensing temperature
        this->CoolingCapacity =
            this->CoffEvapCap * this->RatedEvapCapacity *
            CurveValue(
                state, this->OUCoolingCAPFT(NumOfCompSpdInput), Tdischarge, Tsuction); // Include the piping loss, at the highest compressor speed
        this->PipingCorrectionCooling = TU_CoolingLoad / (TU_CoolingLoad + Pipe_Q_c);
        state.dataHVACVarRefFlow->MaxCoolingCapacity(VRFCond) = this->CoolingCapacity; // for report, maximum evaporating capacity of the system

        this->HeatingCapacity = 0.0;         // Include the piping loss
        this->PipingCorrectionHeating = 1.0; // 1 means no piping loss
        state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond) = Constant::MaxCap;

        this->OUCondHeatRate = Q_h_OU;
        this->OUEvapHeatRate = 0;
        this->IUCondHeatRate = 0;
        this->IUEvapHeatRate = TU_CoolingLoad;

        // 2. VRF-HP Heating Mode .OR. VRF-HR Mode_6
    } else if ((!this->HeatRecoveryUsed && state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) ||
               (this->HeatRecoveryUsed && !HRCoolRequestFlag && HRHeatRequestFlag)) {

        this->OperatingMode = ModeHeatingOnly;
        this->VRFOperationSimPath = 60;

        // Initialization of VRF-FluidTCtrl Model
        Q_h_TU_PL = TU_HeatingLoad;
        Ncomp = TU_HeatingLoad / this->HeatingCOP;
        this->CondensingTemp = this->IUCondensingTemp;

        // Evaporative capacity ranges_Max
        CapMaxTe = OutdoorDryBulb - this->SH;
        CompEvaporatingCAPSpdMax = this->CoffEvapCap * this->RatedEvapCapacity *
                                   CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), this->IUCondensingTemp, CapMaxTe);
        CompEvaporatingPWRSpdMax =
            this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(NumOfCompSpdInput), this->IUCondensingTemp, CapMaxTe);

        // Initialization of h_comp_out iterations (Label23)
        Pcond = this->refrig->getSatPressure(state, 40.0, RoutineName);
        RefTSat = this->refrig->getSatTemperature(state, Pcond, RoutineName);
        h_IU_cond_in_up = this->refrig->getSupHeatEnthalpy(state, max(RefTSat, min(this->IUCondensingTemp + 50, RefTHigh)), Pcond, RoutineName);
        h_IU_cond_in_low = this->refrig->getSatEnthalpy(state, this->IUCondensingTemp, 1.0, RoutineName); // Quality=1
        h_IU_cond_in = h_IU_cond_in_low;

        bool converged_23;
        do {
            m_ref_IU_cond = 0;
            h_IU_cond_out_ave = 0;
            SC_IU_merged = 0;

            // Calculate total refrigerant flow rate
            if (Q_h_TU_PL > CompEvaporatingCAPSpdMax + CompEvaporatingPWRSpdMax) {
                // Required load is beyond the max system capacity

                h_IU_cond_out =
                    this->refrig->getSatEnthalpy(state,
                                                 this->refrig->getSatTemperature(state, max(min(Pcond, RefPHigh), RefPLow), RoutineName) - 5.0,
                                                 0.0,
                                                 RoutineName); // Quality=0
                h_IU_cond_out_ave = h_IU_cond_out;
                SC_IU_merged = 5;
                m_ref_IU_cond = TU_HeatingLoad / (h_IU_cond_in - h_IU_cond_out);

            } else {
                for (NumTU = 1; NumTU <= NumTUInList; NumTU++) {
                    if (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) > 0) {
                        TUIndex = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
                        HeatCoilIndex = state.dataHVACVarRefFlow->VRFTU(TUIndex).HeatCoilIndex;
                        h_IU_cond_out_i =
                            this->refrig->getSatEnthalpy(state,
                                                         this->refrig->getSatTemperature(state, max(min(Pcond, RefPHigh), RefPLow), RoutineName) -
                                                             state.dataDXCoils->DXCoil(HeatCoilIndex).ActualSC,
                                                         0.0,
                                                         RoutineName); // Quality=0
                        m_ref_IU_cond_i =
                            (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) <= 0.0)
                                ? 0.0
                                : (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) / (h_IU_cond_in - h_IU_cond_out_i));
                        m_ref_IU_cond = m_ref_IU_cond + m_ref_IU_cond_i;
                        h_IU_cond_out_ave = h_IU_cond_out_ave + m_ref_IU_cond_i * h_IU_cond_out_i;
                        SC_IU_merged = SC_IU_merged + m_ref_IU_cond_i * state.dataDXCoils->DXCoil(HeatCoilIndex).ActualSC;
                    }
                }
                if (m_ref_IU_cond > 0) {
                    h_IU_cond_out_ave = h_IU_cond_out_ave / m_ref_IU_cond; // h_merge
                    SC_IU_merged = SC_IU_merged / m_ref_IU_cond;
                } else {
                    h_IU_cond_out_ave =
                        this->refrig->getSatEnthalpy(state,
                                                     this->refrig->getSatTemperature(state, max(min(Pcond, RefPHigh), RefPLow), RoutineName) - 5.0,
                                                     0.0,
                                                     RoutineName); // Quality=0
                    SC_IU_merged = 5;
                    m_ref_IU_cond = TU_HeatingLoad / (h_IU_cond_in - h_IU_cond_out_ave);
                }
            }

            // *Calculate piping loss
            this->VRFOU_PipeLossH(
                state, m_ref_IU_cond, max(min(Pcond, RefPHigh), RefPLow), h_IU_cond_in, OutdoorDryBulb, Pipe_Q_h, Pipe_DeltP_h, h_comp_out);

            Pdischarge = max(Pcond + Pipe_DeltP_h, Pcond); // affected by piping loss
            Tdischarge = this->refrig->getSatTemperature(state, max(min(Pdischarge, RefPHigh), RefPLow), RoutineName);

            // Evaporative capacity ranges_Min
            // suction pressure lower bound need to be no less than both terms in the following
            CapMinPe = max(Pdischarge - this->CompMaxDeltaP, RefMinPe);
            CapMinTe = this->refrig->getSatTemperature(state, max(min(CapMinPe, RefPHigh), RefPLow), RoutineName);
            CompEvaporatingCAPSpdMin = this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(1), Tdischarge, CapMinTe);
            CompEvaporatingPWRSpdMin = this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(1), Tdischarge, CapMinTe);

            Q_h_TU_PL = TU_HeatingLoad + Pipe_Q_h;
            Q_c_OU = max(0.0, Q_h_TU_PL - CompEvaporatingPWRSpdMin);

            // *Calculate capacity modification factor
            RefTSat = this->refrig->getSatTemperature(state, max(min(CapMinPe, RefPHigh), RefPLow), RoutineName);
            h_comp_in =
                this->refrig->getSupHeatEnthalpy(state, max(RefTSat, CapMinTe + this->SH), max(min(CapMinPe, RefPHigh), RefPLow), RoutineName);
            C_cap_operation = this->VRFOU_CapModFactor(state,
                                                       h_comp_in,
                                                       h_IU_cond_out_ave,
                                                       max(min(CapMinPe, RefPHigh), RefPLow),
                                                       CapMinTe + this->SH,
                                                       CapMinTe + 8,
                                                       this->IUCondensingTemp - 5);

            Real64 CompEvaporatingCAPSpdMaxCurrentTsuc =
                this->CoffEvapCap * this->RatedEvapCapacity *
                CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), Tdischarge, this->EvaporatingTemp);
            Real64 CompEvaporatingPWRSpdMaxCurrentTsuc =
                this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(NumOfCompSpdInput), Tdischarge, this->EvaporatingTemp);
            if (CompEvaporatingCAPSpdMin > CompEvaporatingCAPSpdMaxCurrentTsuc) {
                if (this->CondenserCapErrIdx == 0) {
                    ShowSevereMessage(state, format("{} \"{}\":", cVRFTypes(VRF_HeatPump), this->Name));
                    ShowContinueErrorTimeStamp(state,
                                               format(" Evaporative Capacity at max speed is smaller than evaporative capacity at min speed, "
                                                      "{:.3T} < {:.3T}",
                                                      CompEvaporatingCAPSpdMaxCurrentTsuc,
                                                      CompEvaporatingCAPSpdMin));
                }
                ShowRecurringSevereErrorAtEnd(
                    state,
                    format("\"{}\" - Evaporative Capacity at max speed is smaller than evaporative capacity at min speed ", this->Name),
                    this->CondenserCapErrIdx,
                    CompEvaporatingCAPSpdMaxCurrentTsuc - CompEvaporatingCAPSpdMin,
                    CompEvaporatingCAPSpdMaxCurrentTsuc - CompEvaporatingCAPSpdMin);
            }
            if ((Q_c_OU * C_cap_operation) > CompEvaporatingCAPSpdMaxCurrentTsuc) {
                // this branch resolves the issue of supplemental heating coil turning on when compressor speed is not at the highest
                Q_c_OU = CompEvaporatingCAPSpdMaxCurrentTsuc;
                CompSpdActual = this->CompressorSpeed(NumOfCompSpdInput);
                Ncomp = CompEvaporatingPWRSpdMaxCurrentTsuc;
                m_air = this->OUAirFlowRate * RhoAir;
                SH_OU = this->SH;
                this->VRFOU_TeTc(
                    state, HXOpMode::EvapMode, Q_c_OU, SH_OU, m_air, OutdoorDryBulb, OutdoorHumRat, OutdoorPressure, Tfs, this->EvaporatingTemp);
                break;
            } else {
                // CompEvaporatingCAPSpdMin < (Q_c_OU * C_cap_operation) <= CompEvaporatingCAPSpdMaxCurrentTsuc + CompEvaporatingPWRSpdMaxCurrentTsuc
                // Required heating load is greater than or equal to the min heating capacity

                // Iteration_Ncomp: Perform iterations to calculate Ncomp (Label20)
                Counter = 1;
                bool converged_20;
                do {
                    Ncomp_new = Ncomp;
                    Q_c_OU = max(0.0, Q_h_TU_PL - Ncomp);

                    // *VRF OU Te calculations
                    m_air = this->OUAirFlowRate * RhoAir;
                    SH_OU = this->SH;
                    this->VRFOU_TeTc(
                        state, HXOpMode::EvapMode, Q_c_OU, SH_OU, m_air, OutdoorDryBulb, OutdoorHumRat, OutdoorPressure, Tfs, this->EvaporatingTemp);
                    this->SH = SH_OU;

                    // *VRF OU Compressor Simulation at heating mode: Specify the compressor speed and power consumption
                    this->VRFOU_CalcCompH(state,
                                          TU_HeatingLoad,
                                          this->EvaporatingTemp,
                                          Tdischarge,
                                          h_IU_cond_out_ave,
                                          this->IUCondensingTemp,
                                          CapMinTe,
                                          Tfs,
                                          Pipe_Q_h,
                                          Q_c_OU,
                                          CompSpdActual,
                                          Ncomp_new,
                                          CyclingRatio);

                    converged_20 = (std::abs(Ncomp_new - Ncomp) <= (Tolerance * Ncomp)) || (Counter >= 30);
                    Counter = Counter + 1;
                    if (!converged_20) {
                        Ncomp = Ncomp_new;
                    }
                } while (!converged_20);

                // Update h_comp_out in iteration Label23
                P_comp_in = this->refrig->getSatPressure(state, this->EvaporatingTemp, RoutineName);
                RefTSat = this->refrig->getSatTemperature(state, max(min(P_comp_in, RefPHigh), RefPLow), RoutineName);
                h_comp_in_new = this->refrig->getSupHeatEnthalpy(
                    state, max(RefTSat, this->SH + this->EvaporatingTemp), max(min(P_comp_in, RefPHigh), RefPLow), RoutineName);
                h_comp_out_new = Ncomp_new / m_ref_IU_cond + h_comp_in_new;

                converged_23 = !((std::abs(h_comp_out - h_comp_out_new) > Tolerance * h_comp_out) && (h_IU_cond_in < h_IU_cond_in_up));
                if (!converged_23) {
                    h_IU_cond_in = h_IU_cond_in + 0.1 * (h_IU_cond_in_up - h_IU_cond_in_low);
                } else {
                    if (h_IU_cond_in > h_IU_cond_in_up) {
                        h_IU_cond_in = 0.5 * (h_IU_cond_in_up + h_IU_cond_in_low);
                    }
                    Ncomp = Ncomp_new;
                    break;
                }
            }
        } while (!converged_23);

        // Key outputs of this subroutine
        Q_c_OU *= CyclingRatio;
        this->CompActSpeed = max(CompSpdActual, 0.0);
        this->Ncomp = max(Ncomp, 0.0) / this->EffCompInverter;
        this->OUFanPower = this->RatedOUFanPower * CyclingRatio;
        this->VRFCondCyclingRatio = CyclingRatio;

        Tsuction = this->EvaporatingTemp; // Outdoor unit evaporating temperature
        this->HeatingCapacity =
            this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), Tdischarge, Tsuction) +
            this->RatedCompPower * CurveValue(state,
                                              this->OUCoolingPWRFT(NumOfCompSpdInput),
                                              Tdischarge,
                                              Tsuction); // Include the piping loss, at the highest compressor speed
        this->PipingCorrectionHeating = TU_HeatingLoad / (TU_HeatingLoad + Pipe_Q_h);
        state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond) =
            this->HeatingCapacity; // for report, maximum condensing capacity the system can provide

        this->CoolingCapacity = 0.0; // Include the piping loss
        this->PipingCorrectionCooling = 1.0;
        state.dataHVACVarRefFlow->MaxCoolingCapacity(VRFCond) = Constant::MaxCap; // for report

        this->OUCondHeatRate = 0;
        this->OUEvapHeatRate = Q_c_OU;
        this->IUCondHeatRate = TU_HeatingLoad;
        this->IUEvapHeatRate = 0;

        // 3. VRF-HR Mode_2-5, Simultaneous Heating and Cooling
    } else if (this->HeatRecoveryUsed && HRCoolRequestFlag && HRHeatRequestFlag) {

        this->OperatingMode = ModeCoolingAndHeating;

        // Initialization of VRF-FluidTCtrl Model
        Q_c_TU_PL = TU_CoolingLoad;
        Q_h_TU_PL = TU_HeatingLoad;

        // Evaporator (IU side) operational parameters
        Pevap = this->refrig->getSatPressure(state, this->IUEvaporatingTemp, RoutineName);
        Psuction = Pevap;
        this->EvaporatingTemp = this->IUEvaporatingTemp;

        // Condenser (OU side) operation ranges
        CapMaxPc = min(Psuction + this->CompMaxDeltaP, RefMaxPc);
        CapMaxTc = this->refrig->getSatTemperature(state, max(min(CapMaxPc, RefPHigh), RefPLow), RoutineName);
        CapMinTc = OutdoorDryBulb + this->SC;
        CapMinPc = this->refrig->getSatPressure(state, CapMinTc, RoutineName);

        // Evaporator (IU side) operation ranges
        CapMinPe = max(CapMinPc - this->CompMaxDeltaP, RefMinPe);
        CapMinTe = this->refrig->getSatTemperature(state, max(min(CapMinPe, RefPHigh), RefPLow), RoutineName);

        //===**h_comp_out Iteration Starts

        // Initialization of h_comp_out iterations (Label230)
        {
            Pcond = this->refrig->getSatPressure(state, this->IUCondensingTemp, RoutineName);
            Real64 Pcond_temp = this->refrig->getSatPressure(state, 40.0, RoutineName);
            RefTSat = this->refrig->getSatTemperature(state, Pcond_temp, RoutineName);
            h_IU_cond_in_up =
                this->refrig->getSupHeatEnthalpy(state, max(RefTSat, min(this->IUCondensingTemp + 50, RefTHigh)), Pcond_temp, RoutineName);
            h_IU_cond_in_low = this->refrig->getSatEnthalpy(state, this->IUCondensingTemp, 1.0, RoutineName); // Quality=1
            h_IU_cond_in = h_IU_cond_in_low;
        }

        bool converged_230;
        do {
            // *PL-h: Calculate total refrigerant flow rate
            m_ref_IU_cond = 0;
            h_IU_cond_out_ave = 0;
            SC_IU_merged = 0;
            for (NumTU = 1; NumTU <= NumTUInList; NumTU++) {
                if (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) > 0) {
                    TUIndex = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
                    HeatCoilIndex = state.dataHVACVarRefFlow->VRFTU(TUIndex).HeatCoilIndex;
                    h_IU_cond_out_i =
                        this->refrig->getSatEnthalpy(state,
                                                     this->refrig->getSatTemperature(state, max(min(Pcond, RefPHigh), RefPLow), RoutineName) -
                                                         state.dataDXCoils->DXCoil(HeatCoilIndex).ActualSC,
                                                     0.0,
                                                     RoutineName); // Quality=0
                    m_ref_IU_cond_i =
                        (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) <= 0.0)
                            ? 0.0
                            : (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad(NumTU) / (h_IU_cond_in - h_IU_cond_out_i));
                    m_ref_IU_cond = m_ref_IU_cond + m_ref_IU_cond_i;
                    h_IU_cond_out_ave = h_IU_cond_out_ave + m_ref_IU_cond_i * h_IU_cond_out_i;
                    SC_IU_merged = SC_IU_merged + m_ref_IU_cond_i * state.dataDXCoils->DXCoil(HeatCoilIndex).ActualSC;
                }
            }
            if (m_ref_IU_cond > 0) {
                h_IU_cond_out_ave = h_IU_cond_out_ave / m_ref_IU_cond;
                SC_IU_merged = SC_IU_merged / m_ref_IU_cond;
            } else {
                h_IU_cond_out_ave =
                    this->refrig->getSatEnthalpy(state,
                                                 this->refrig->getSatTemperature(state, max(min(Pcond, RefPHigh), RefPLow), RoutineName) - 5.0,
                                                 0.0,
                                                 RoutineName); // Quality=0
                SC_IU_merged = 5;
                m_ref_IU_cond = TU_HeatingLoad / (h_IU_cond_in - h_IU_cond_out_ave);
            }

            // *PL-h: Calculate piping loss
            this->VRFOU_PipeLossH(
                state, m_ref_IU_cond, max(min(Pcond, RefPHigh), RefPLow), h_IU_cond_in, OutdoorDryBulb, Pipe_Q_h, Pipe_DeltP_h, h_comp_out);
            Pdischarge = max(Pcond + Pipe_DeltP_h, Pcond); // affected by piping loss
            Tdischarge = this->refrig->getSatTemperature(state, max(min(Pdischarge, RefPHigh), RefPLow), RoutineName);
            Q_h_TU_PL = TU_HeatingLoad + Pipe_Q_h;

            // *PL-c: Calculate total IU refrigerant flow rate and SH_IU_merged
            h_IU_evap_in = h_IU_cond_out_ave;
            m_ref_IU_evap = 0;
            h_IU_evap_out = 0;
            SH_IU_merged = 0;
            for (NumTU = 1; NumTU <= NumTUInList; NumTU++) { // Calc total refrigerant flow rate
                if (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) > 0) {
                    TUIndex = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
                    CoolCoilIndex = state.dataHVACVarRefFlow->VRFTU(TUIndex).CoolCoilIndex;

                    RefTSat = this->refrig->getSatTemperature(state, max(min(Pevap, RefPHigh), RefPLow), RoutineName);
                    h_IU_evap_out_i =
                        this->refrig->getSupHeatEnthalpy(state,
                                                         max(RefTSat, this->IUEvaporatingTemp + state.dataDXCoils->DXCoil(CoolCoilIndex).ActualSH),
                                                         max(min(Pevap, RefPHigh), RefPLow),
                                                         RoutineName);

                    if (h_IU_evap_out_i > h_IU_evap_in) {
                        m_ref_IU_evap_i = (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) <= 0.0)
                                              ? 0.0
                                              : (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) /
                                                 (h_IU_evap_out_i - h_IU_evap_in)); // Ref Flow Rate in the IU( kg/s )
                        m_ref_IU_evap = m_ref_IU_evap + m_ref_IU_evap_i;
                        h_IU_evap_out = h_IU_evap_out + m_ref_IU_evap_i * h_IU_evap_out_i;
                        SH_IU_merged = SH_IU_merged + m_ref_IU_evap_i * state.dataDXCoils->DXCoil(CoolCoilIndex).ActualSH;
                    }
                }
            }
            if (m_ref_IU_evap > 0) {
                h_IU_evap_out = h_IU_evap_out / m_ref_IU_evap;
                SH_IU_merged = SH_IU_merged / m_ref_IU_evap;
            } else {
                RefTSat = this->refrig->getSatTemperature(state, max(min(Pevap, RefPHigh), RefPLow), RoutineName);
                h_IU_evap_out = this->refrig->getSupHeatEnthalpy(
                    state, max(RefTSat, this->IUEvaporatingTemp + 3), max(min(Pevap, RefPHigh), RefPLow), RoutineName);
                SH_IU_merged = 3;
                m_ref_IU_evap = TU_CoolingLoad / (h_IU_evap_out - h_IU_evap_in);
            }

            // *PL-c: Calculate piping loss
            this->VRFOU_PipeLossC(state,
                                  m_ref_IU_evap,
                                  max(min(Pevap, RefPHigh), RefPLow),
                                  h_IU_evap_out,
                                  SH_IU_merged,
                                  OutdoorDryBulb,
                                  Pipe_Q_c,
                                  Pipe_DeltP_c,
                                  h_IU_PLc_out);
            Psuction = min(Pevap - Pipe_DeltP_c, Pevap); // This Psuction is used for rps > min; will be updated for rps = min
            Tsuction = this->refrig->getSatTemperature(state, max(min(Psuction, RefPHigh), RefPLow), RoutineName);
            h_comp_in = h_IU_PLc_out;
            Q_c_TU_PL = TU_CoolingLoad + Pipe_Q_c;

            //**OU operations: Determine VRF-HR OU system operational mode
            //  Determine the operational mode of the VRF-HR system, given the terminal unit side load conditions.
            //  A number of OU side operational parameters are also calculated here, including:
            //  (1) OU evaporator load Q_c_OU, (2) OU condenser load Q_h_OU,
            //  (3) m_ref_OU_evap, (4) m_ref_OU_cond
            //  Note that Te and Te' may be updated here, and thus IU evaporator side piping loss recalculations.
            //  Then a number of operational parameters need to be updated, including:
            //  (1) IU evaporating temperature Te (2) OU evaporating temperature Te' etc (3) m_ref_IU_evap
            //  (4) Pipe_Q_c (5) h_IU_PLc_out (6) h_comp_in
            //*VRF OU Compressor Simulation at HR mode: Specify the compressor speed and power consumption
            {
                Real64 Pipe_Q_c_new = Pipe_Q_c;
                Real64 Tsuction_new = Tsuction;
                Real64 Te_new = this->IUEvaporatingTemp;
                Real64 N_fan_OU;

                this->VRFHR_OU_HR_Mode(state,
                                       h_IU_evap_in,
                                       h_comp_out,
                                       Q_c_TU_PL,
                                       Q_h_TU_PL,
                                       Tdischarge,
                                       Tsuction_new,
                                       Te_new,
                                       h_comp_in,
                                       h_IU_PLc_out,
                                       Pipe_Q_c_new,
                                       Q_c_OU,
                                       Q_h_OU,
                                       m_ref_IU_evap,
                                       m_ref_OU_evap,
                                       m_ref_OU_cond,
                                       N_fan_OU,
                                       CompSpdActual,
                                       Ncomp);

                // parameter update
                Tsuction = Tsuction_new;
                Pipe_Q_c = Pipe_Q_c_new;
                this->OUFanPower = N_fan_OU;
                this->IUEvaporatingTemp = Te_new;
            }

            //* Update h_comp_out in iteration (Label230)
            h_comp_out_new = Ncomp / (m_ref_IU_evap + m_ref_OU_evap) + h_comp_in;

            converged_230 = !((std::abs(h_comp_out - h_comp_out_new) > Tolerance * h_comp_out) && (h_IU_cond_in < h_IU_cond_in_up));
            h_IU_cond_in = h_IU_cond_in + 0.1 * (h_IU_cond_in_up - h_IU_cond_in_low);
        } while (!converged_230);
        if (h_IU_cond_in > h_IU_cond_in_up) {
            h_IU_cond_in = 0.5 * (h_IU_cond_in_up + h_IU_cond_in_low);
        }

        //===**h_comp_out Iteration Ends (Label230)

        // Key outputs of this subroutine
        this->CompActSpeed = max(CompSpdActual, 0.0);
        this->Ncomp = max(Ncomp, 0.0) / this->EffCompInverter;
        this->VRFCondCyclingRatio = 1.0;

        this->HeatingCapacity =
            this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), Tdischarge, Tsuction) +
            this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(NumOfCompSpdInput), Tdischarge, Tsuction); // Include the piping loss
        state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond) =
            this->HeatingCapacity; // for report, maximum heating capacity of the system, at the highest compressor speed
        this->PipingCorrectionHeating = TU_HeatingLoad / Q_h_TU_PL;

        this->CoolingCapacity =
            this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), Tdischarge, Tsuction);
        state.dataHVACVarRefFlow->MaxCoolingCapacity(VRFCond) =
            this->CoolingCapacity; // for report, maximum evaporating capacity of the system, at the highest compressor speed
        this->PipingCorrectionCooling = TU_CoolingLoad / Q_c_TU_PL;

        this->CondensingTemp = Tdischarge; // OU condensing temperature
        this->EvaporatingTemp = Tsuction;  // OU evaporating temperature

        this->OUCondHeatRate = Q_h_OU;
        this->OUEvapHeatRate = Q_c_OU;
        this->IUCondHeatRate = TU_HeatingLoad;
        this->IUEvapHeatRate = TU_CoolingLoad;

        // 4. Stop running
    } else {

        this->OperatingMode = 0;
        this->VRFOperationSimPath = 0;

        this->Ncomp = 0.0;
        this->CompActSpeed = 0.0;
        this->OUFanPower = 0.0;
        this->VRFCondCyclingRatio = 0.0;

        this->HeatingCapacity = 0.0;                                              // Include the piping loss
        this->PipingCorrectionHeating = 1.0;                                      // 1 means no piping loss
        state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond) = Constant::MaxCap; // yujie: default value is MaxCap = 1e+20, not 0

        this->CoolingCapacity = 0.0; // Include the piping loss
        this->PipingCorrectionCooling = 1.0;
        state.dataHVACVarRefFlow->MaxCoolingCapacity(VRFCond) = Constant::MaxCap; // for report

        this->CondensingTemp = state.dataEnvrn->OutDryBulbTemp;
        this->EvaporatingTemp = state.dataEnvrn->OutDryBulbTemp;

        this->OUCondHeatRate = 0.0;
        this->OUEvapHeatRate = 0.0;
        this->IUCondHeatRate = 0.0;
        this->IUEvapHeatRate = 0.0;
    }

    // calculate capacities and energy use
    if (((!this->HeatRecoveryUsed && state.dataHVACVarRefFlow->CoolingLoad(VRFCond)) || (this->HeatRecoveryUsed && HRCoolRequestFlag)) &&
        state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).CoolingCoilPresent(NumTUInList)) {
        InletAirWetBulbC = SumCoolInletWB;

        // From the VRF_FluidTCtrl model
        TotalCondCoolingCapacity = this->CoolingCapacity;
        TotalTUCoolingCapacity = TotalCondCoolingCapacity * this->PipingCorrectionCooling;

        if (TotalCondCoolingCapacity > 0.0) {
            CoolingPLR = min(1.0, (this->TUCoolingLoad / this->PipingCorrectionCooling) / TotalCondCoolingCapacity);
        } else {
            CoolingPLR = 0.0;
        }
    }
    if (((!this->HeatRecoveryUsed && state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) || (this->HeatRecoveryUsed && HRHeatRequestFlag)) &&
        state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).HeatingCoilPresent(NumTUInList)) {
        InletAirDryBulbC = SumHeatInletDB;
        InletAirWetBulbC = SumHeatInletWB;

        // Initializing defrost adjustment factors
        LoadDueToDefrost = 0.0;
        HeatingCapacityMultiplier = 1.0;
        FractionalDefrostTime = 0.0;
        InputPowerMultiplier = 1.0;

        // Check outdoor temperature to determine of defrost is active
        if (OutdoorDryBulb <= this->MaxOATDefrost && this->CondenserType != DataHeatBalance::RefrigCondenserType::Water) {

            // Calculating adjustment factors for defrost
            // Calculate delta w through outdoor coil by assuming a coil temp of 0.82*DBT-9.7(F) per DOE2.1E
            OutdoorCoilT = 0.82 * OutdoorDryBulb - 8.589;
            OutdoorCoildw = max(1.0e-6, (OutdoorHumRat - PsyWFnTdpPb(state, OutdoorCoilT, OutdoorPressure)));

            // Calculate defrost adjustment factors depending on defrost control type
            if (this->DefrostControl == StandardRatings::HPdefrostControl::Timed) {
                FractionalDefrostTime = this->DefrostFraction;
                if (FractionalDefrostTime > 0.0) {
                    HeatingCapacityMultiplier = 0.909 - 107.33 * OutdoorCoildw;
                    InputPowerMultiplier = 0.90 - 36.45 * OutdoorCoildw;
                }
            } else { // else defrost control is on-demand
                FractionalDefrostTime = 1.0 / (1.0 + 0.01446 / OutdoorCoildw);
                HeatingCapacityMultiplier = 0.875 * (1.0 - FractionalDefrostTime);
                InputPowerMultiplier = 0.954 * (1.0 - FractionalDefrostTime);
            }

            if (FractionalDefrostTime > 0.0) {
                // Calculate defrost adjustment factors depending on defrost control strategy
                if (this->DefrostStrategy == StandardRatings::DefrostStrat::ReverseCycle &&
                    this->DefrostControl == StandardRatings::HPdefrostControl::OnDemand) {
                    LoadDueToDefrost = (0.01 * FractionalDefrostTime) * (7.222 - OutdoorDryBulb) * (this->HeatingCapacity / 1.01667);
                    DefrostEIRTempModFac = CurveValue(state, this->DefrostEIRPtr, max(15.555, InletAirWetBulbC), max(15.555, OutdoorDryBulb));

                    //         Warn user if curve output goes negative
                    if (DefrostEIRTempModFac < 0.0) {
                        if (!state.dataGlobal->WarmupFlag) {
                            if (this->DefrostHeatErrorIndex == 0) {
                                ShowSevereMessage(state, format("{} \"{}\":", cVRFTypes(VRF_HeatPump), this->Name));
                                ShowContinueError(
                                    state,
                                    format(" Defrost Energy Input Ratio Modifier curve (function of temperature) output is negative ({:.3T}).",
                                           DefrostEIRTempModFac));
                                ShowContinueError(state,
                                                  format(" Negative value occurs using an outdoor air dry-bulb temperature of {:.1T} C and an "
                                                         "average indoor air wet-bulb temperature of {:.1T} C.",
                                                         OutdoorDryBulb,
                                                         InletAirWetBulbC));
                                ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
                            }
                            ShowRecurringWarningErrorAtEnd(state,
                                                           format("{} \"{}\": Defrost Energy Input Ratio Modifier curve (function of temperature) "
                                                                  "output is negative warning continues...",
                                                                  PlantEquipTypeNames[static_cast<int>(PlantEquipmentType::HeatPumpVRF)],
                                                                  this->Name),
                                                           this->DefrostHeatErrorIndex,
                                                           DefrostEIRTempModFac,
                                                           DefrostEIRTempModFac);
                            DefrostEIRTempModFac = 0.0;
                        }
                    }

                    this->DefrostPower = DefrostEIRTempModFac * (this->HeatingCapacity / 1.01667) * FractionalDefrostTime;

                } else { // Defrost strategy is resistive
                    this->DefrostPower = this->DefrostCapacity * FractionalDefrostTime;
                }
            } else { // Defrost is not active because FractionalDefrostTime = 0.0
                this->DefrostPower = 0.0;
            }
        }

        // From the VRF_FluidTCtrl model
        TotalCondHeatingCapacity = this->HeatingCapacity;
        TotalTUHeatingCapacity = TotalCondHeatingCapacity * this->PipingCorrectionHeating;

        if (TotalCondHeatingCapacity > 0.0) {
            HeatingPLR = min(1.0, (this->TUHeatingLoad / this->PipingCorrectionHeating) / TotalCondHeatingCapacity);
            HeatingPLR += (LoadDueToDefrost * HeatingPLR) / TotalCondHeatingCapacity;
        } else {
            HeatingPLR = 0.0;
        }
    }

    this->VRFCondPLR = max(CoolingPLR, HeatingPLR);

    // For VRF-HR Operations
    HRInitialCapFrac = 1.0;
    HRInitialEIRFrac = 1.0;
    HRCapTC = 0.0;
    HREIRTC = 0.0;
    if (!state.dataGlobal->DoingSizing && !state.dataGlobal->WarmupFlag) {
        if (HRHeatRequestFlag && HRCoolRequestFlag) { // Simultaneous Heating and Cooling operations for HR system
            // determine operating mode change: (1) ModeChange (2) HRCoolingActive (3) HRHeatingActive
            if (!this->HRCoolingActive && !this->HRHeatingActive) {
                this->ModeChange = true;
            }
            if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond)) {
                if (this->HRHeatingActive && !this->HRCoolingActive) {
                    this->HRModeChange = true;
                }
                this->HRCoolingActive = true;
                this->HRHeatingActive = false;

                HRInitialCapFrac = this->HRInitialCoolCapFrac; // Fractional cooling degradation at the start of heat recovery from cooling mode
                HRCapTC = this->HRCoolCapTC;                   // Time constant used to recover from initial degradation in cooling heat recovery

                HRInitialEIRFrac = this->HRInitialCoolEIRFrac; // Fractional cooling degradation at the start of heat recovery from cooling mode
                HREIRTC = this->HRCoolEIRTC;                   // Time constant used to recover from initial degradation in cooling heat recovery

            } else if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) {
                if (!this->HRHeatingActive && this->HRCoolingActive) {
                    this->HRModeChange = true;
                }
                this->HRCoolingActive = false;
                this->HRHeatingActive = true;

                HRInitialCapFrac = this->HRInitialHeatCapFrac; // Fractional heating degradation at the start of heat recovery from cooling mode
                HRCapTC = this->HRHeatCapTC;                   // Time constant used to recover from initial degradation in heating heat recovery

                HRInitialEIRFrac = this->HRInitialHeatEIRFrac; // Fractional heating degradation at the start of heat recovery from heating mode
                HREIRTC = this->HRHeatEIRTC;                   // Time constant used to recover from initial degradation in heating heat recovery

            } else {
                // zone thermostats satisfied, condenser is off. Set values anyway
                // HRCAPFTConst = 1.0;
                HRInitialCapFrac = 1.0;
                HRCapTC = 1.0;
                // HREIRFTConst = 1.0;
                HRInitialEIRFrac = 1.0;
                HREIRTC = 1.0;
                if (this->HRHeatingActive || this->HRCoolingActive) {
                    this->HRModeChange = true;
                }
                this->HRCoolingActive = false;
                this->HRHeatingActive = false;
            }

        } else { // IF(HRHeatRequestFlag .AND. HRCoolRequestFlag)THEN -- Heat recovery turned off
            HRInitialCapFrac = 1.0;
            HRCapTC = 0.0;
            HRInitialEIRFrac = 1.0;
            HREIRTC = 0.0;
            this->HRModeChange = false;
            this->HRCoolingActive = false;
            this->HRHeatingActive = false;
        }

        // Calculate the capacity modification factor (SUMultiplier) for the HR mode transition period
        {
            CurrentEndTime = double((state.dataGlobal->DayOfSim - 1) * 24) + state.dataGlobal->CurrentTime - state.dataGlobal->TimeStepZone +
                             state.dataHVACGlobal->SysTimeElapsed;

            if (this->ModeChange || this->HRModeChange) {
                if (this->HRCoolingActive && this->HRTimer == 0.0) {
                    this->HRTimer = state.dataHVACVarRefFlow->CurrentEndTimeLast;
                } else if (this->HRHeatingActive && this->HRTimer == 0.0) {
                    this->HRTimer = state.dataHVACVarRefFlow->CurrentEndTimeLast;
                } else if (!this->HRCoolingActive && !this->HRHeatingActive) {
                    this->HRTimer = 0.0;
                }
            }

            this->HRTime = max(0.0, CurrentEndTime - this->HRTimer);
            if (this->HRTime < (HRCapTC * 5.0)) {
                if (HRCapTC > 0.0) {
                    SUMultiplier = min(1.0, 1.0 - std::exp(-this->HRTime / HRCapTC));
                } else {
                    SUMultiplier = 1.0;
                }
            } else {
                SUMultiplier = 1.0;
                this->ModeChange = false;
                this->HRModeChange = false;
            }
            this->SUMultiplier = SUMultiplier;

            state.dataHVACVarRefFlow->CurrentEndTimeLast = CurrentEndTime;
        }

        // Modify HR capacity for the transition period
        {
            if (this->HeatRecoveryUsed && this->HRCoolingActive) {
                TotalCondCoolingCapacity =
                    HRInitialCapFrac * TotalCondCoolingCapacity + (1.0 - HRInitialCapFrac) * TotalCondCoolingCapacity * SUMultiplier;
                TotalTUCoolingCapacity = TotalCondCoolingCapacity * this->PipingCorrectionCooling;
                if (TotalCondCoolingCapacity > 0.0) {
                    CoolingPLR = min(1.0, (this->TUCoolingLoad / this->PipingCorrectionCooling) / TotalCondCoolingCapacity);
                } else {
                    CoolingPLR = 0.0;
                }
                this->VRFHeatRec = this->TUHeatingLoad;
            } else if (this->HeatRecoveryUsed && this->HRHeatingActive) {
                TotalCondHeatingCapacity =
                    HRInitialCapFrac * TotalCondHeatingCapacity + (1.0 - HRInitialCapFrac) * TotalCondHeatingCapacity * SUMultiplier;
                TotalTUHeatingCapacity = TotalCondHeatingCapacity * this->PipingCorrectionHeating;
                if (TotalCondHeatingCapacity > 0.0) {
                    HeatingPLR = min(1.0, (this->TUHeatingLoad / this->PipingCorrectionHeating) / TotalCondHeatingCapacity);
                } else {
                    HeatingPLR = 0.0;
                }
                this->VRFHeatRec = this->TUCoolingLoad;
            }

            this->VRFCondPLR = max(CoolingPLR, HeatingPLR);
        }
    }

    this->TotalCoolingCapacity = TotalCondCoolingCapacity * CoolingPLR;
    this->TotalHeatingCapacity = TotalCondHeatingCapacity * HeatingPLR;

    if (this->MinPLR > 0.0) {
        bool const plrTooLow = this->VRFCondPLR < this->MinPLR;
        bool const plrGreaterThanZero = this->VRFCondPLR > 0.0;
        if (plrTooLow && plrGreaterThanZero) {
            this->VRFCondPLR = this->MinPLR;
        }
    }

    VRFRTF = 0.0;
    // VRF Cooling and Heating Electric Power (output variables)
    if (this->OperatingMode == ModeCoolingOnly) {
        PartLoadFraction = 1.0;
        VRFRTF = min(1.0, (CyclingRatio / PartLoadFraction));

        this->ElecCoolingPower = state.dataHVACVarRefFlow->VRF(VRFCond).Ncomp + this->OUFanPower;
        this->ElecHeatingPower = 0;

    } else if (this->OperatingMode == ModeHeatingOnly) {
        PartLoadFraction = 1.0;
        VRFRTF = min(1.0, (CyclingRatio / PartLoadFraction));

        this->ElecCoolingPower = 0;
        this->ElecHeatingPower = this->Ncomp + this->OUFanPower;

    } else if (this->OperatingMode == ModeCoolingAndHeating) {
        PartLoadFraction = 1.0;
        VRFRTF = min(1.0, (CyclingRatio / PartLoadFraction));

        this->ElecCoolingPower = (this->Ncomp + this->OUFanPower) * this->IUEvapHeatRate / (this->IUCondHeatRate + this->IUEvapHeatRate);
        this->ElecHeatingPower = (this->Ncomp + this->OUFanPower) * this->IUCondHeatRate / (this->IUCondHeatRate + this->IUEvapHeatRate);

    } else {
        this->ElecCoolingPower = 0;
        this->ElecHeatingPower = 0;
    }
    this->VRFCondRTF = VRFRTF;
    this->DefrostPower *= VRFRTF;

    // Calculate CrankCaseHeaterPower: VRF Heat Pump Crankcase Heater Electric Power [W]
    if (this->MaxOATCCHeater > OutdoorDryBulb) {
        // calculate crankcase heater power
        this->CrankCaseHeaterPower = this->CCHeaterPower * (1.0 - VRFRTF);
        if (this->NumCompressors > 1) {
            UpperStageCompressorRatio = (1.0 - this->CompressorSizeRatio) / (this->NumCompressors - 1);
            for (Stage = 1; Stage <= this->NumCompressors - 2; ++Stage) {
                if (this->VRFCondPLR < (this->CompressorSizeRatio + Stage * UpperStageCompressorRatio)) {
                    this->CrankCaseHeaterPower += this->CCHeaterPower;
                }
            }
        }
    } else {
        this->CrankCaseHeaterPower = 0.0;
    }

    // Calculate QCondenser: VRF Heat Pump Condenser Heat Transfer Rate [W]
    CondCapacity = max(this->TotalCoolingCapacity, this->TotalHeatingCapacity) * VRFRTF;
    CondPower = max(this->ElecCoolingPower, this->ElecHeatingPower);
    if (this->ElecHeatingPower > 0.0) {
        this->QCondenser = CondCapacity + CondPower - this->TUHeatingLoad / this->PipingCorrectionHeating;
    } else if (this->ElecCoolingPower > 0.0) {
        this->QCondenser = -CondCapacity + CondPower + this->TUCoolingLoad / this->PipingCorrectionCooling;
    } else {
        this->QCondenser = 0.0;
    }
    // if ( this->CondenserType == HVAC::EvapCooled )

    // Calculate OperatingHeatingCOP & OperatingCoolingCOP: VRF Heat Pump Operating COP []
    if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond) && CoolingPLR > 0.0) {
        if (this->ElecCoolingPower != 0.0) {
            // this calc should use delivered capacity, not condenser capacity, use VRF(VRFCond).TUCoolingLoad
            this->OperatingCoolingCOP = (this->TotalCoolingCapacity) /
                                        (this->ElecCoolingPower + this->CrankCaseHeaterPower + this->EvapCondPumpElecPower + this->DefrostPower);
        } else {
            this->OperatingCoolingCOP = 0.0;
        }
    }
    if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond) && HeatingPLR > 0.0) {
        // this calc should use delivered capacity, not condenser capacity, use VRF(VRFCond).TUHeatingLoad
        if (this->ElecHeatingPower != 0.0) {
            this->OperatingHeatingCOP = (this->TotalHeatingCapacity) /
                                        (this->ElecHeatingPower + this->CrankCaseHeaterPower + this->EvapCondPumpElecPower + this->DefrostPower);
        } else {
            this->OperatingHeatingCOP = 0.0;
        }
    }

    TotPower = TUParasiticPower + TUFanPower + this->ElecHeatingPower + this->ElecCoolingPower + this->CrankCaseHeaterPower +
               this->EvapCondPumpElecPower + this->DefrostPower;
    if (TotPower > 0.0) {
        this->OperatingCOP = (this->TUCoolingLoad + this->TUHeatingLoad) / TotPower;
        this->SCHE = this->OperatingCOP * 3.412141633; // see StandardRatings::ConvFromSIToIP
    }

    // limit the TU capacity when the condenser is maxed out on capacity
    // I think this next line will make the max cap report variable match the coil objects, will probably change the answer though
    //  IF(CoolingLoad(VRFCond) .AND. NumTUInCoolingMode .GT. 0 .AND. MaxCoolingCapacity(VRFCond) == MaxCap)THEN
    if (state.dataHVACVarRefFlow->CoolingLoad(VRFCond) && NumTUInCoolingMode > 0) {

        //   IF TU capacity is greater than condenser capacity find maximum allowed TU capacity (i.e., conserve energy)
        if (TU_CoolingLoad > TotalTUCoolingCapacity) {
            LimitTUCapacity(state,
                            VRFCond,
                            NumTUInList,
                            TotalTUCoolingCapacity,
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad,
                            state.dataHVACVarRefFlow->MaxCoolingCapacity(VRFCond),
                            TotalTUHeatingCapacity,
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad,
                            state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond));
        }
    } else if (state.dataHVACVarRefFlow->HeatingLoad(VRFCond) && NumTUInHeatingMode > 0) {
        //   IF TU capacity is greater than condenser capacity
        if (TU_HeatingLoad > TotalTUHeatingCapacity) {
            LimitTUCapacity(state,
                            VRFCond,
                            NumTUInList,
                            TotalTUHeatingCapacity,
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalHeatLoad,
                            state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond),
                            TotalTUCoolingCapacity,
                            state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad,
                            state.dataHVACVarRefFlow->MaxCoolingCapacity(VRFCond));
        }
    } else {
    }

    // Calculate the IU Te/Tc for the next time step
    this->CalcVRFIUTeTc_FluidTCtrl(state);
    // update coil and IU evaporating temperature, also keep coil RTF updated with the condenser side cycling ratio, for the FluidTCtrl model
    for (int VRFTUNum = 1; VRFTUNum <= state.dataHVACVarRefFlow->NumVRFTU; ++VRFTUNum) {
        auto const &thisTU = state.dataHVACVarRefFlow->VRFTU(VRFTUNum);
        auto &coolingCoil = state.dataDXCoils->DXCoil(thisTU.CoolCoilIndex);
        if (this->adjustedTe && (!FirstHVACIteration)) {
            coolingCoil.EvaporatingTemp = this->EvaporatingTemp;
            this->IUEvaporatingTemp = this->EvaporatingTemp;
        }

        int PLF;
        if (coolingCoil.PLFFPLR(1) > 0 && this->VRFCondCyclingRatio < 1.0) {
            PLF = Curve::CurveValue(state, coolingCoil.PLFFPLR(1), this->VRFCondCyclingRatio); // Calculate part-load factor
        } else {
            PLF = 1.0;
        }
        if (coolingCoil.TotalCoolingEnergyRate > 0.0) {
            coolingCoil.CoolingCoilRuntimeFraction = this->VRFCondCyclingRatio / PLF;
        }
    }
}

void VRFTerminalUnitEquipment::ControlVRF_FluidTCtrl(EnergyPlusData &state,
                                                     int const VRFTUNum,            // Index to VRF terminal unit
                                                     Real64 const QZnReq,           // Index to zone number
                                                     bool const FirstHVACIteration, // flag for 1st HVAC iteration in the time step
                                                     Real64 &PartLoadRatio,         // unit part load ratio
                                                     Real64 &OnOffAirFlowRatio,     // ratio of compressor ON airflow to AVERAGE airflow over timestep
                                                     Real64 &SuppHeatCoilLoad       // supplemental heating coil load (W)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rongpeng Zhang
    //       DATE WRITTEN   Nov 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Determine the coil load and part load ratio, given the zone load
    // Determine the air mass flow rate corresponding to the coil load of the heat pump for this time step

    // METHODOLOGY EMPLOYED:
    // Use RegulaFalsi technique to iterate on part-load ratio until convergence is achieved.

    using General::SolveRoot;

    int constexpr MaxIte(500);        // maximum number of iterations
    Real64 constexpr MinPLF(0.0);     // minimum part load factor allowed
    Real64 constexpr ErrorTol(0.001); // tolerance for RegulaFalsi iterations

    Real64 FullOutput;     // unit full output when compressor is operating [W]
    Real64 TempOutput;     // unit output when iteration limit exceeded [W]
    Real64 NoCompOutput;   // output when no active compressor [W]
    Real64 TempMinPLR;     // min PLR used in Regula Falsi call
    Real64 TempMaxPLR;     // max PLR used in Regula Falsi call
    int VRFCond;           // index to VRF condenser
    int IndexToTUInTUList; // index to TU in specific list for the VRF system
    int TUListIndex;       // index to TU list for this VRF system
    bool VRFCoolingMode;
    bool VRFHeatingMode;
    bool HRCoolingMode;
    bool HRHeatingMode;

    PartLoadRatio = 0.0;
    state.dataHVACVarRefFlow->LoopDXCoolCoilRTF = 0.0;
    state.dataHVACVarRefFlow->LoopDXHeatCoilRTF = 0.0;
    state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatPartLoadRatio = 0.0;
    VRFCond = this->VRFSysNum;
    IndexToTUInTUList = this->IndexToTUInTUList;
    auto &thisVRFCond = state.dataHVACVarRefFlow->VRF(VRFCond);
    TUListIndex = thisVRFCond.ZoneTUListPtr;
    VRFCoolingMode = state.dataHVACVarRefFlow->CoolingLoad(VRFCond);
    VRFHeatingMode = state.dataHVACVarRefFlow->HeatingLoad(VRFCond);
    HRCoolingMode = state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList);
    HRHeatingMode = state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList);
    auto &thisVRFTU = state.dataHVACVarRefFlow->VRFTU(VRFTUNum);

    // The RETURNS here will jump back to SimVRF where the CalcVRF routine will simulate with latest PLR

    // do nothing else if TU is scheduled off
    if (this->availSched->getCurrentVal() == 0.0) {
        return;
    }

    // Block the following statement: QZnReq==0 doesn't mean QCoilReq==0 due to possible OA mixer operation. zrp_201511
    // do nothing if TU has no load (TU will be modeled using PLR=0)
    // if ( QZnReq == 0.0 ) return;

    // Set EMS value for PLR and return
    if (this->EMSOverridePartLoadFrac) {
        PartLoadRatio = this->EMSValueForPartLoadFrac;
        return;
    }

    // Get result when DX coil is off
    PartLoadRatio = 0.0;
    bool DXCoolingCoilOprCtrl = true;

    // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
    this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, 0.0, NoCompOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);

    if (VRFCoolingMode && HRHeatingMode) {
        // IF the system is in cooling mode, but the terminal unit requests heating (heat recovery)
        if (NoCompOutput >= QZnReq) {
            return;
        }
    } else if (VRFHeatingMode && HRCoolingMode) {
        // IF the system is in heating mode, but the terminal unit requests cooling (heat recovery)
        if (NoCompOutput <= QZnReq) {
            return;
        }
    } else if (VRFCoolingMode || HRCoolingMode) {
        // IF the system is in cooling mode and/or the terminal unit requests cooling
        if (NoCompOutput <= QZnReq && ((QZnReq <= 0.0) || (QZnReq >= HVAC::SmallLoad && !HRCoolingMode))) {
            DXCoolingCoilOprCtrl = false;
            if (!this->SuppHeatingCoilPresent) {
                return;
            }
        }
    } else if (VRFHeatingMode || HRHeatingMode) {
        // IF the system is in heating mode and/or the terminal unit requests heating
        if (NoCompOutput >= QZnReq) {
            return;
        }
    }

    // Otherwise the coil needs to turn on. Get full load result
    PartLoadRatio = 1.0;
    if (!DXCoolingCoilOprCtrl) {
        PartLoadRatio = 0.0;
    }
    this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, PartLoadRatio, FullOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);
    if (this->CoolingCoilPresent) {
        auto const &thisAirInNode = state.dataLoopNodes->Node(state.dataDXCoils->DXCoil(this->CoolCoilIndex).AirInNode);
        this->coilInNodeT = thisAirInNode.Temp;
        this->coilInNodeW = thisAirInNode.HumRat;
    } else {
        auto const &thisAirInNode = state.dataLoopNodes->Node(state.dataDXCoils->DXCoil(this->HeatCoilIndex).AirInNode);
        this->coilInNodeT = thisAirInNode.Temp;
        this->coilInNodeW = thisAirInNode.HumRat;
    }

    // set supplemental heating coil calculation if the condition requires
    if (this->SuppHeatingCoilPresent) {
        auto const &thisSuppHeatCoilAirInletNode = state.dataLoopNodes->Node(this->SuppHeatCoilAirInletNode);
        if (((QZnReq > HVAC::SmallLoad && QZnReq > FullOutput) || (((QZnReq - NoCompOutput) > HVAC::SmallLoad) && QZnReq <= 0.0)) ||
            (this->isSetPointControlled && this->suppTempSetPoint > thisSuppHeatCoilAirInletNode.Temp)) {
            Real64 ZoneLoad = 0.0;
            Real64 LoadToHeatingSP = 0.0;
            Real64 LoadToCoolingSP = 0.0;
            if (this->isSetPointControlled) {
                Real64 mDot = thisSuppHeatCoilAirInletNode.MassFlowRate;
                Real64 Tin = thisSuppHeatCoilAirInletNode.Temp;
                Real64 Win = thisSuppHeatCoilAirInletNode.HumRat;
                Real64 CpAirIn = Psychrometrics::PsyCpAirFnW(Win);
                SuppHeatCoilLoad = mDot * CpAirIn * (this->suppTempSetPoint - Tin);
                this->SuppHeatingCoilLoad = SuppHeatCoilLoad;
                if (this->DesignSuppHeatingCapacity > 0.0) {
                    this->SuppHeatPartLoadRatio = min(1.0, SuppHeatCoilLoad / this->DesignSuppHeatingCapacity);
                }
            } else {
                getVRFTUZoneLoad(state, VRFTUNum, ZoneLoad, LoadToHeatingSP, LoadToCoolingSP, false);
                if (((FullOutput < (LoadToHeatingSP - HVAC::SmallLoad) || ((QZnReq - NoCompOutput) > HVAC::SmallLoad && QZnReq <= 0.0))) &&
                    !FirstHVACIteration) {
                    if ((QZnReq - NoCompOutput) > HVAC::SmallLoad && QZnReq <= 0.0) {
                        if (LoadToHeatingSP < 0.0 && QZnReq == 0.0) {
                            SuppHeatCoilLoad = max(0.0, LoadToHeatingSP - FullOutput);
                        } else {
                            SuppHeatCoilLoad = max(0.0, QZnReq - FullOutput);
                        }
                    } else {
                        if (QZnReq > 0.0 && (NoCompOutput - QZnReq) >= HVAC::SmallLoad) {
                            SuppHeatCoilLoad = 0.0;
                        } else {
                            SuppHeatCoilLoad = max(0.0, LoadToHeatingSP - FullOutput);
                        }
                    }
                    this->SuppHeatingCoilLoad = SuppHeatCoilLoad;
                    if (this->DesignSuppHeatingCapacity > 0.0) {
                        this->SuppHeatPartLoadRatio = min(1.0, SuppHeatCoilLoad / this->DesignSuppHeatingCapacity);
                    }
                } else {
                    SuppHeatCoilLoad = 0.0;
                    this->SuppHeatPartLoadRatio = 0.0;
                }
            }
        } else {
            SuppHeatCoilLoad = 0.0;
            this->SuppHeatPartLoadRatio = 0.0;
        }
    } else {
        SuppHeatCoilLoad = 0.0;
        this->SuppHeatPartLoadRatio = 0.0;
    }

    if ((VRFCoolingMode && !thisVRFCond.HeatRecoveryUsed) || (thisVRFCond.HeatRecoveryUsed && HRCoolingMode)) {
        // Since we are cooling, we expect FullOutput < NoCompOutput
        // If the QZnReq <= FullOutput the unit needs to run full out
        if (QZnReq <= FullOutput) {
            // if no coil present in terminal unit, no need to reset PLR?
            if (thisVRFTU.CoolingCoilPresent) {
                PartLoadRatio = 1.0;
                // the zone set point could be exceeded if set point control is used so protect against that
                if (this->isSetPointControlled) {
                    if (state.dataLoopNodes->Node(this->coolCoilAirOutNode).Temp > this->coilTempSetPoint) {
                        return;
                    }
                } else {
                    if (QZnReq >= 0.0 && FullOutput >= 0.0) {
                        PartLoadRatio = 0.0;
                    }
                    return;
                }
            } else {
                PartLoadRatio = 0.0;
                return;
            }
        } else {
            if (QZnReq == 0.0 && (FullOutput < 0.0 && NoCompOutput < FullOutput)) {
                PartLoadRatio = 0.0;
                return;
            }
        }
    } else if ((VRFHeatingMode && !thisVRFCond.HeatRecoveryUsed) || (thisVRFCond.HeatRecoveryUsed && HRHeatingMode)) {
        // Since we are heating, we expect FullOutput > NoCompOutput
        // If the QZnReq >= FullOutput the unit needs to run full out
        if (QZnReq >= FullOutput) {
            // if no coil present in terminal unit, no need reset PLR?
            if (this->HeatingCoilPresent) {
                PartLoadRatio = 1.0;
                // the zone set point could be exceeded if set point control is used so protect against that
                if (this->isSetPointControlled) {
                    if (state.dataLoopNodes->Node(this->heatCoilAirOutNode).Temp < this->coilTempSetPoint) {
                        return;
                    }
                } else {
                    return;
                }
            } else {
                PartLoadRatio = 0.0;
                return;
            }
        }
    } else {
        // VRF terminal unit is off
        // shouldn't actually get here
        PartLoadRatio = 0.0;
        return;
    }

    // The coil will not operate at PLR=0 or PLR=1, calculate the operating part-load ratio

    if ((VRFHeatingMode || HRHeatingMode) || ((VRFCoolingMode && DXCoolingCoilOprCtrl) || HRCoolingMode)) {
        int SolFla; // Flag of RegulaFalsi solver
        auto f = [&state, VRFTUNum, FirstHVACIteration, QZnReq, OnOffAirFlowRatio](Real64 const PartLoadRatio) {
            Real64 QZnReqTemp = QZnReq;    // denominator representing zone load (W)
            Real64 ActualOutput;           // delivered capacity of VRF terminal unit
            Real64 SuppHeatCoilLoad = 0.0; // supplemental heating coil load (W)
            bool setPointControlled = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).isSetPointControlled;
            Real64 nonConstOnOffAirFlowRatio = OnOffAirFlowRatio;

            if (state.dataHVACVarRefFlow->VRF(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFSysNum).VRFAlgorithmType == AlgorithmType::FluidTCtrl) {
                // Algorithm Type: VRF model based on physics, applicable for Fluid Temperature Control
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF_FluidTCtrl(
                    state, VRFTUNum, FirstHVACIteration, PartLoadRatio, ActualOutput, nonConstOnOffAirFlowRatio, SuppHeatCoilLoad);
            } else {
                // Algorithm Type: VRF model based on system curve
                state.dataHVACVarRefFlow->VRFTU(VRFTUNum).CalcVRF(
                    state, VRFTUNum, FirstHVACIteration, PartLoadRatio, ActualOutput, nonConstOnOffAirFlowRatio, SuppHeatCoilLoad);
            }

            if (setPointControlled) {
                Real64 outletNodeT = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUOutletNodeNum).Temp;
                return (outletNodeT - state.dataHVACVarRefFlow->VRFTU(VRFTUNum).coilTempSetPoint);
            } else {
                if (std::abs(QZnReq) < 100.0) {
                    QZnReqTemp = sign(100.0, QZnReq);
                }
                return (ActualOutput - QZnReq) / QZnReqTemp;
            }
        };
        SolveRoot(state, ErrorTol, MaxIte, SolFla, PartLoadRatio, f, 0.0, 1.0);
        if (SolFla == -1) {
            //     Very low loads may not converge quickly. Tighten PLR boundary and try again.
            TempMaxPLR = -0.1;
            bool ContinueIter = true;
            while (ContinueIter && TempMaxPLR < 1.0) {
                TempMaxPLR += 0.1;

                this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, TempMaxPLR, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);

                if (VRFHeatingMode && TempOutput > QZnReq) {
                    ContinueIter = false;
                }
                if (VRFCoolingMode && TempOutput < QZnReq) {
                    ContinueIter = false;
                }
            }
            TempMinPLR = TempMaxPLR;
            ContinueIter = true;
            while (ContinueIter && TempMinPLR > 0.0) {
                TempMaxPLR = TempMinPLR;
                TempMinPLR -= 0.01;

                this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, TempMaxPLR, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);

                if (VRFHeatingMode && TempOutput < QZnReq) {
                    ContinueIter = false;
                }
                if (VRFCoolingMode && TempOutput > QZnReq) {
                    ContinueIter = false;
                }
            }

            SolveRoot(state, ErrorTol, MaxIte, SolFla, PartLoadRatio, f, TempMinPLR, TempMaxPLR);
            if (SolFla == -1) {
                if (!FirstHVACIteration && !state.dataGlobal->WarmupFlag) {
                    if (this->IterLimitExceeded == 0) {
                        ShowWarningMessage(state, format("{} \"{}\"", tuTypeNames[(int)this->type], this->Name));
                        ShowContinueError(
                            state, format(" Iteration limit exceeded calculating terminal unit part-load ratio, maximum iterations = {}", MaxIte));
                        ShowContinueErrorTimeStamp(state, format(" Part-load ratio returned = {:.3R}", PartLoadRatio));

                        this->CalcVRF_FluidTCtrl(state, VRFTUNum, FirstHVACIteration, TempMinPLR, TempOutput, OnOffAirFlowRatio, SuppHeatCoilLoad);

                        ShowContinueError(state, format(" Load requested = {:.5T}, Load delivered = {:.5T}", QZnReq, TempOutput));
                        ShowRecurringWarningErrorAtEnd(state,
                                                       format("{} \"{}\" -- Terminal unit Iteration limit exceeded error continues...",
                                                              tuTypeNames[(int)this->type],
                                                              this->Name),
                                                       this->IterLimitExceeded);
                    } else {
                        ShowRecurringWarningErrorAtEnd(state,
                                                       format("{} \"{}\" -- Terminal unit Iteration limit exceeded error continues...",
                                                              tuTypeNames[(int)this->type],
                                                              this->Name),
                                                       this->IterLimitExceeded);
                    }
                }
            } else if (SolFla == -2) {
                PartLoadRatio = max(MinPLF, std::abs(QZnReq - NoCompOutput) / std::abs(FullOutput - NoCompOutput));
            }
        } else if (SolFla == -2) {
            if (FullOutput - NoCompOutput == 0.0) {
                PartLoadRatio = 0.0;
            } else {
                PartLoadRatio = min(1.0, max(MinPLF, std::abs(QZnReq - NoCompOutput) / std::abs(FullOutput - NoCompOutput)));
            }
        }
    }
}

void VRFTerminalUnitEquipment::CalcVRF_FluidTCtrl(EnergyPlusData &state,
                                                  int const VRFTUNum,                           // Index to VRF terminal unit
                                                  bool const FirstHVACIteration,                // flag for 1st HVAC iteration in the time step
                                                  Real64 const PartLoadRatio,                   // compressor part load fraction
                                                  Real64 &LoadMet,                              // load met by unit (W)
                                                  Real64 &OnOffAirFlowRatio,                    // ratio of ON air flow to average air flow
                                                  Real64 &SuppHeatCoilLoad,                     // supplemental heating coil load (W)
                                                  ObjexxFCL::Optional<Real64> LatOutputProvided // delivered latent capacity (W)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         RP Zhang (LBNL), XF Pang (LBNL), Y Yura (Daikin Inc)
    //       DATE WRITTEN   June 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //         This subroutine is part of the new VRF model based on physics, applicable for Fluid Temperature Control.
    //         This is adapted from subroutine CalcVRF, which is part of the VRF model based on system curves.
    //         This subroutine simulates the components making up the VRF indoor terminal unit.

    // METHODOLOGY EMPLOYED:
    //        A new physics based VRF model applicable for Fluid Temperature Control.
    using DXCoils::SimDXCoil;
    using SingleDuct::SimATMixer;
    using SteamCoils::SimulateSteamCoilComponents;
    using WaterCoils::SimulateWaterCoilComponents;

    int VRFTUOutletNodeNum; // TU air outlet node
    int VRFTUInletNodeNum;  // TU air inlet node
    Real64 AirMassFlow;     // total supply air mass flow [m3/s]
    HVAC::FanOp fanOp;      // fan operating mode, HVAC::FanOp::Cycling or HVAC::FanOp::Continuous
    int VRFCond;            // index to VRF condenser
    Real64 SpecHumOut;      // specific humidity ratio at outlet node
    Real64 SpecHumIn;       // specific humidity ratio at inlet node
    int TUListIndex;        // index to TU list for this VRF system
    int IndexToTUInTUList;  // index to TU in specific list for the VRF system
    Real64 EvapTemp;        // evaporating temperature
    Real64 CondTemp;        // condensing temperature
    int ZoneNode;           // Zone node of VRFTU is serving

    VRFCond = this->VRFSysNum;
    TUListIndex = state.dataHVACVarRefFlow->VRF(VRFCond).ZoneTUListPtr;
    IndexToTUInTUList = this->IndexToTUInTUList;
    VRFTUOutletNodeNum = this->VRFTUOutletNodeNum;
    VRFTUInletNodeNum = this->VRFTUInletNodeNum;
    fanOp = this->fanOp;
    EvapTemp = state.dataHVACVarRefFlow->VRF(VRFCond).IUEvaporatingTemp;
    CondTemp = state.dataHVACVarRefFlow->VRF(VRFCond).IUCondensingTemp;
    ZoneNode = this->ZoneAirNode;

    // Set inlet air mass flow rate based on PLR and compressor on/off air flow rates
    if (PartLoadRatio == 0) {
        // only provide required OA when coil is off
        state.dataHVACVarRefFlow->CompOnMassFlow = state.dataHVACVarRefFlow->OACompOnMassFlow;
        state.dataHVACVarRefFlow->CompOffMassFlow = state.dataHVACVarRefFlow->OACompOffMassFlow;
    } else {
        // identify the air flow rate corresponding to the coil load
        if (this->HeatingCoilPresent && state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond) < Constant::MaxCap) {
            // Only fix heating only mode for now
            state.dataHVACVarRefFlow->CompOnMassFlow = CalVRFTUAirFlowRate_FluidTCtrl(
                state, VRFTUNum, PartLoadRatio, FirstHVACIteration, state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond));
        } else {
            state.dataHVACVarRefFlow->CompOnMassFlow = CalVRFTUAirFlowRate_FluidTCtrl(state, VRFTUNum, PartLoadRatio, FirstHVACIteration, _);
        }
    }
    SetAverageAirFlow(state, VRFTUNum, PartLoadRatio, OnOffAirFlowRatio);
    AirMassFlow = state.dataLoopNodes->Node(VRFTUInletNodeNum).MassFlowRate;

    if (this->ATMixerExists) {
        // There is an air terminal mixer
        state.dataHVACVarRefFlow->ATMixOutNode2 = this->ATMixerOutNode;
        if (this->ATMixerType == HVAC::MixerType::InletSide) { // if there is an inlet side air terminal mixer
            // set the primary air inlet mass flow rate
            state.dataLoopNodes->Node(this->ATMixerPriNode).MassFlowRate =
                min(state.dataLoopNodes->Node(this->ATMixerPriNode).MassFlowRateMaxAvail, state.dataLoopNodes->Node(VRFTUInletNodeNum).MassFlowRate);
            // now calculate the the mixer outlet air conditions (and the secondary air inlet flow rate). The mixer outlet flow rate has already
            // been set above (it is the "inlet" node flow rate)
            SimATMixer(state, this->ATMixerName, FirstHVACIteration, this->ATMixerIndex);
            // inlet side ATMixer can change the VRF TU inlet condition and therefore the operating air flow rate
            if (this->fanOp == HVAC::FanOp::Cycling && PartLoadRatio > 0) {
                if (this->HeatingCoilPresent && state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond) < Constant::MaxCap) {
                    // Only fix heating only mode for now
                    state.dataHVACVarRefFlow->CompOnMassFlow = CalVRFTUAirFlowRate_FluidTCtrl(
                        state, VRFTUNum, PartLoadRatio, FirstHVACIteration, state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond));
                } else {
                    state.dataHVACVarRefFlow->CompOnMassFlow = CalVRFTUAirFlowRate_FluidTCtrl(state, VRFTUNum, PartLoadRatio, FirstHVACIteration, _);
                }
                if (std::abs(state.dataHVACVarRefFlow->CompOnMassFlow - AirMassFlow) > HVAC::SmallMassFlow) {
                    SetAverageAirFlow(state, VRFTUNum, PartLoadRatio, OnOffAirFlowRatio);
                    AirMassFlow = state.dataLoopNodes->Node(VRFTUInletNodeNum).MassFlowRate;
                    // set the primary air inlet mass flow rate
                    state.dataLoopNodes->Node(this->ATMixerPriNode).MassFlowRate =
                        min(state.dataLoopNodes->Node(this->ATMixerPriNode).MassFlowRateMaxAvail,
                            state.dataLoopNodes->Node(VRFTUInletNodeNum).MassFlowRate);
                    SimATMixer(state, this->ATMixerName, FirstHVACIteration, this->ATMixerIndex);
                }
            }
        }
    } else {
        state.dataHVACVarRefFlow->ATMixOutNode2 = 0;
        // simulate OA Mixer
        if (this->OAMixerUsed) {
            MixedAir::SimOAMixer(state, this->OAMixerName, this->OAMixerIndex);
        }
    }
    // if blow through, simulate fan then coils
    if (this->fanPlace == HVAC::FanPlace::BlowThru) {
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanType == HVAC::FanType::SystemModel) {
            if (OnOffAirFlowRatio > 0.0) {
                state.dataFans->fans(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).FanIndex)->simulate(state, FirstHVACIteration, _, _);
            } else {
                state.dataFans->fans(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).FanIndex)->simulate(state, FirstHVACIteration, _, _, PartLoadRatio);
            }
        } else {
            state.dataFans->fans(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).FanIndex)
                ->simulate(state, FirstHVACIteration, state.dataHVACVarRefFlow->FanSpeedRatio);
        }
    }
    if (this->CoolingCoilPresent) {
        // above condition for heat pump mode, below condition for heat recovery mode
        if ((!state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed && state.dataHVACVarRefFlow->CoolingLoad(VRFCond)) ||
            (state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed &&
             state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList))) {
            SimDXCoil(state,
                      "",
                      HVAC::CompressorOp::On,
                      FirstHVACIteration,
                      this->CoolCoilIndex,
                      fanOp,
                      PartLoadRatio,
                      _,
                      _,
                      state.dataHVACVarRefFlow->MaxCoolingCapacity(VRFCond),
                      state.dataHVACVarRefFlow->VRF(this->VRFSysNum).VRFCondCyclingRatio);
        } else { // cooling coil is off
            SimDXCoil(state, "", HVAC::CompressorOp::Off, FirstHVACIteration, this->CoolCoilIndex, fanOp, 0.0, _);
        }
        state.dataHVACVarRefFlow->LoopDXCoolCoilRTF = state.dataAirLoop->LoopDXCoilRTF;
    } else {
        state.dataHVACVarRefFlow->LoopDXCoolCoilRTF = 0.0;
    }

    if (this->HeatingCoilPresent) {
        // above condition for heat pump mode, below condition for heat recovery mode
        if ((!state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed && state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) ||
            (state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed &&
             state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList))) {
            SimDXCoil(state,
                      "",
                      HVAC::CompressorOp::On,
                      FirstHVACIteration,
                      this->HeatCoilIndex,
                      fanOp,
                      PartLoadRatio,
                      _,
                      _,
                      state.dataHVACVarRefFlow->MaxHeatingCapacity(VRFCond));
        } else {
            SimDXCoil(state, "", HVAC::CompressorOp::Off, FirstHVACIteration, this->HeatCoilIndex, fanOp, 0.0, _);
        }
        state.dataHVACVarRefFlow->LoopDXHeatCoilRTF = state.dataAirLoop->LoopDXCoilRTF;
    } else {
        state.dataHVACVarRefFlow->LoopDXHeatCoilRTF = 0.0;
    }

    Real64 OnOffFanPartLoadFraction = 1.0;
    if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOp == HVAC::FanOp::Cycling) {
        OnOffFanPartLoadFraction = state.dataHVACGlobal->OnOffFanPartLoadFraction;
    }
    // if draw through, simulate coils then fan
    if (this->fanPlace == HVAC::FanPlace::DrawThru) {
        auto *fan = state.dataFans->fans(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).FanIndex);
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanType == HVAC::FanType::SystemModel) {
            if (OnOffAirFlowRatio > 0.0) {
                fan->simulate(state, FirstHVACIteration, _, _, _, fan->inletAirMassFlowRate, OnOffFanPartLoadFraction, 0, 0, _);
            } else {
                fan->simulate(state, FirstHVACIteration, _, _, PartLoadRatio);
            }
        } else {
            fan->simulate(state, FirstHVACIteration, state.dataHVACVarRefFlow->FanSpeedRatio);
        }
    }

    // track fan power per terminal unit for calculating COP
    this->FanPower = state.dataFans->fans(this->FanIndex)->totalPower;

    // run supplemental heating coil
    if (this->SuppHeatingCoilPresent) {
        Real64 SuppPLR = this->SuppHeatPartLoadRatio;
        this->CalcVRFSuppHeatingCoil(state, VRFTUNum, FirstHVACIteration, SuppPLR, SuppHeatCoilLoad);
        if ((state.dataLoopNodes->Node(this->SuppHeatCoilAirOutletNode).Temp > this->MaxSATFromSuppHeatCoil) && SuppPLR > 0.0) {
            // adjust the heating load to maximum allowed
            Real64 MaxHeatCoilLoad = this->HeatingCoilCapacityLimit(state, this->SuppHeatCoilAirInletNode, this->MaxSATFromSuppHeatCoil);
            this->CalcVRFSuppHeatingCoil(state, VRFTUNum, FirstHVACIteration, SuppPLR, MaxHeatCoilLoad);
            SuppHeatCoilLoad = MaxHeatCoilLoad;
        }
    }

    Real64 LatentLoadMet = 0.0;
    Real64 TempOut = 0.0;
    Real64 TempIn = 0.0;
    if (this->ATMixerExists) {
        if (this->ATMixerType == HVAC::MixerType::SupplySide) {
            // Air terminal supply side mixer, calculate supply side mixer output
            SimATMixer(state, this->ATMixerName, FirstHVACIteration, this->ATMixerIndex);
            TempOut = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->ATMixOutNode2).Temp;
            SpecHumOut = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->ATMixOutNode2).HumRat;
            AirMassFlow = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->ATMixOutNode2).MassFlowRate;
        } else {
            // Air terminal inlet side mixer
            TempOut = state.dataLoopNodes->Node(VRFTUOutletNodeNum).Temp;
            SpecHumOut = state.dataLoopNodes->Node(VRFTUOutletNodeNum).HumRat;
        }
        TempIn = state.dataLoopNodes->Node(ZoneNode).Temp;
        SpecHumIn = state.dataLoopNodes->Node(ZoneNode).HumRat;
    } else {
        TempOut = state.dataLoopNodes->Node(VRFTUOutletNodeNum).Temp;
        SpecHumOut = state.dataLoopNodes->Node(VRFTUOutletNodeNum).HumRat;
        if (ZoneNode > 0) {
            TempIn = state.dataLoopNodes->Node(ZoneNode).Temp;
            SpecHumIn = state.dataLoopNodes->Node(ZoneNode).HumRat;
        } else {
            TempIn = state.dataLoopNodes->Node(VRFTUInletNodeNum).Temp;
            SpecHumIn = state.dataLoopNodes->Node(VRFTUInletNodeNum).HumRat;
        }
    }
    // calculate sensible load met using delta enthalpy
    Real64 TotalOutput = AirMassFlow * (Psychrometrics::PsyHFnTdbW(TempOut, SpecHumOut) -
                                        Psychrometrics::PsyHFnTdbW(TempIn, SpecHumIn));         // total addition/removal rate, {W};
    LoadMet = AirMassFlow * PsyDeltaHSenFnTdb2W2Tdb1W1(TempOut, SpecHumOut, TempIn, SpecHumIn); // sensible {W}
    LatentLoadMet = TotalOutput - LoadMet;
    if (present(LatOutputProvided)) {
        LatOutputProvided = LatentLoadMet;
    }
}

Real64 VRFTerminalUnitEquipment::CalVRFTUAirFlowRate_FluidTCtrl(EnergyPlusData &state,
                                                                int const VRFTUNum,                          // Index to VRF terminal unit
                                                                Real64 PartLoadRatio,                        // part load ratio of the coil
                                                                [[maybe_unused]] bool FirstHVACIteration,    // FirstHVACIteration flag
                                                                ObjexxFCL::Optional<Real64 const> MaxHeatCap // maximum allowed heating capacity
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rongpeng Zhang, LBNL
    //       DATE WRITTEN   Nov 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    //  This function determines the TU airflow rate corresponding to the coil load.
    //  This is used to address the coupling between OA mixer simulation and VRF-FluidTCtrl coil simulation.

    // METHODOLOGY EMPLOYED:
    //  VRF-FluidTCtrl TU airflow rate is determined by the control logic of VRF-FluidTCtrl coil to match the
    //  coil load. This is affected by the coil inlet conditions. However, the airflow rate will affect the
    //  OA mixer simulation, which leads to different coil inlet conditions. So, there is a coupling issue here.

    using General::SolveRoot;

    Real64 AirMassFlowRate; // air mass flow rate of the coil (kg/s)

    int constexpr Mode(1);           // Performance mode for MultiMode DX coil. Always 1 for other coil types
    int constexpr MaxIte(500);       // maximum number of iterations
    int DXCoilNum;                   // index to DX Coil
    int IndexToTUInTUList;           // index to TU in specific list for the VRF system
    int SolFla;                      // Flag of RegulaFalsi solver
    int TUListIndex;                 // index to TU list for this VRF system
    int VRFCond;                     // index to VRF condenser
    Real64 constexpr ErrorTol(0.01); // tolerance for RegulaFalsi iterations
    Real64 FanSpdRatio;              // ratio of required and rated air flow rate
    Real64 FanSpdRatioMin;           // min fan speed ratio
    Real64 FanSpdRatioMax;           // min fan speed ratio
    Real64 QCoilReq;                 // required coil load (W)
    Real64 QCoilAct;                 // actual coil load (W)
    Real64 TeTc;                     // evaporating temperature or condensing temperature for VRF indoor unit(C)

    VRFCond = this->VRFSysNum;
    TUListIndex = state.dataHVACVarRefFlow->VRF(VRFCond).ZoneTUListPtr;
    IndexToTUInTUList = this->IndexToTUInTUList;

    if ((!state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed && state.dataHVACVarRefFlow->CoolingLoad(VRFCond)) ||
        (state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed &&
         state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRCoolRequest(IndexToTUInTUList))) {
        // VRF terminal unit is on cooling mode
        DXCoilNum = this->CoolCoilIndex;
        QCoilReq = -PartLoadRatio * state.dataDXCoils->DXCoil(DXCoilNum).RatedTotCap(Mode);
        TeTc = state.dataHVACVarRefFlow->VRF(VRFCond).IUEvaporatingTemp;

        // For HR operations, Te is lower than the outdoor air temperature because of outdoor evaporator operations
        // The difference is usually 2-3C according to the engineering experience. 2 is used here for a slightly bigger fan flow rate.
        if (state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed) {
            TeTc = min(TeTc, state.dataEnvrn->OutDryBulbTemp - 2);
        }

    } else if ((!state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed && state.dataHVACVarRefFlow->HeatingLoad(VRFCond)) ||
               (state.dataHVACVarRefFlow->VRF(VRFCond).HeatRecoveryUsed &&
                state.dataHVACVarRefFlow->TerminalUnitList(TUListIndex).HRHeatRequest(IndexToTUInTUList))) {
        // VRF terminal unit is on heating mode
        DXCoilNum = this->HeatCoilIndex;
        Real64 RatedCapacity = state.dataDXCoils->DXCoil(DXCoilNum).RatedTotCap(Mode);
        if (present(MaxHeatCap)) {
            RatedCapacity = min(MaxHeatCap, RatedCapacity);
        }
        QCoilReq = PartLoadRatio * RatedCapacity;
        TeTc = state.dataHVACVarRefFlow->VRF(VRFCond).IUCondensingTemp;

    } else {
        // VRF terminal unit is off
        QCoilAct = 0.0;
        AirMassFlowRate = max(state.dataHVACVarRefFlow->OACompOnMassFlow, 0.0);
        return AirMassFlowRate;
    }

    // minimum airflow rate
    if (state.dataDXCoils->DXCoil(DXCoilNum).RatedAirMassFlowRate(Mode) > 0.0) {
        FanSpdRatioMin = min(state.dataHVACVarRefFlow->OACompOnMassFlow / state.dataDXCoils->DXCoil(DXCoilNum).RatedAirMassFlowRate(Mode), 1.0);
    } else {
        // VRF terminal unit is off
        QCoilAct = 0.0;
        AirMassFlowRate = max(state.dataHVACVarRefFlow->OACompOnMassFlow, 0.0);
        return AirMassFlowRate;
    }

    FanSpdRatioMax = 1.0;

    auto f = [&state, VRFTUNum, DXCoilNum, QCoilReq, TeTc, PartLoadRatio](Real64 const FanSpdRatio) {
        using DXCoils::ControlVRFIUCoil;
        using Psychrometrics::PsyHFnTdbW;
        using SingleDuct::SimATMixer;

        int constexpr Mode(1);  // Performance mode for MultiMode DX coil. Always 1 for other coil types
        int VRFCond;            // index to VRF condenser
        int VRFInletNode;       // VRF inlet node number
        Real64 FanSpdRatioBase; // baseline FanSpdRatio for VRFTUAirFlowResidual
        Real64 FanSpdRatioAct;  // calculated FanSpdRatio for VRFTUAirFlowResidual
        Real64 QCoilAct;        // actual coil load [W]
        Real64 temp;            // for temporary use
        Real64 Tin;             // coil inlet air temperature [C]
        Real64 Win;             // coil inlet air humidity ratio [kg/kg]
        Real64 Hin;             // coil inlet air enthalpy
        Real64 Wout;            // coil outlet air humidity ratio
        Real64 Tout;            // coil outlet air temperature
        Real64 Hout;            // coil outlet air enthalpy
        Real64 SHact;           // coil actual SH
        Real64 SCact;           // coil actual SC

        VRFCond = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFSysNum;
        VRFInletNode = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum;

        if (std::abs(FanSpdRatio) < 0.01) {
            FanSpdRatioBase = sign(0.01, FanSpdRatio);
        } else {
            FanSpdRatioBase = FanSpdRatio;
        }

        // Set inlet air mass flow rate based on PLR and compressor on/off air flow rates
        state.dataHVACVarRefFlow->CompOnMassFlow = FanSpdRatio * state.dataDXCoils->DXCoil(DXCoilNum).RatedAirMassFlowRate(Mode);
        SetAverageAirFlow(state, VRFTUNum, PartLoadRatio, temp);
        Tin = state.dataLoopNodes->Node(VRFInletNode).Temp;
        Win = state.dataLoopNodes->Node(VRFInletNode).HumRat;

        // Simulation the OAMixer if there is any
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerUsed) {
            MixedAir::SimOAMixer(
                state, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerName, state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerIndex);
            int OAMixNode = state.dataMixedAir->OAMixer(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).OAMixerIndex).MixNode;
            Tin = state.dataLoopNodes->Node(OAMixNode).Temp;
            Win = state.dataLoopNodes->Node(OAMixNode).HumRat;
        }
        // Simulate the blow-through fan if there is any
        if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanPlace == HVAC::FanPlace::BlowThru) {
            auto *fan = state.dataFans->fans(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).FanIndex);
            if (state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanType == HVAC::FanType::SystemModel) {
                if (temp > 0) {
                    fan->simulate(state, false, _, _);
                } else {
                    fan->simulate(state, false, _, _, PartLoadRatio);
                }
            } else {
                fan->simulate(state, false, state.dataHVACVarRefFlow->FanSpeedRatio);
            }
            Tin = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOutletNode).Temp;
            Win = state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOutletNode).HumRat;
        }

        // Call the coil control logic to determine the air flow rate to match the given coil load
        ControlVRFIUCoil(
            state, DXCoilNum, QCoilReq, Tin, Win, TeTc, state.dataHVACVarRefFlow->OACompOnMassFlow, FanSpdRatioAct, Wout, Tout, Hout, SHact, SCact);

        Hin = PsyHFnTdbW(Tin, Win);
        QCoilAct = FanSpdRatioAct * state.dataDXCoils->DXCoil(DXCoilNum).RatedAirMassFlowRate(Mode) *
                   (Hout - Hin); // positive for heating, negative for cooling

        return (FanSpdRatioAct - FanSpdRatio);
    };

    SolveRoot(state, ErrorTol, MaxIte, SolFla, FanSpdRatio, f, FanSpdRatioMin, FanSpdRatioMax);
    if (SolFla < 0) {
        FanSpdRatio = FanSpdRatioMax; // over capacity
    }

    AirMassFlowRate = FanSpdRatio * state.dataDXCoils->DXCoil(DXCoilNum).RatedAirMassFlowRate(Mode);

    return AirMassFlowRate;
}

Real64 CompResidual_FluidTCtrl(EnergyPlusData &state,
                               Real64 T_dis,
                               Real64 CondHeat,
                               int CAPFT,
                               Real64 const T_suc // Compressor suction temperature Te' [C]
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Xiufeng Pang (XP)
    //       DATE WRITTEN   Mar 2013
    //       MODIFIED       Jul 2015, RP Zhang, LBNL
    //       RE-ENGINEERED
    //
    // PURPOSE OF THIS FUNCTION:
    //       Calculates residual function ((VRV terminal unit cooling output - Zone sensible cooling load)
    //
    using Curve::CurveValue;

    Real64 CAPSpd; // Evaporative capacity of the compressor at a given spd[W]
    Real64 CompResidual;

    CAPSpd = CurveValue(state, CAPFT, T_dis, T_suc);
    CompResidual = (CondHeat - CAPSpd) / CAPSpd;

    return CompResidual;
}

void VRFCondenserEquipment::VRFOU_TeTc(EnergyPlusData &state,
                                       HXOpMode const OperationMode, // Mode 0 for running as condenser, 1 for evaporator
                                       Real64 const Q_coil,          // // OU coil heat release at cooling mode or heat extract at heating mode [W]
                                       Real64 const SHSC,            // SC for OU condenser or SH for OU evaporator [C]
                                       Real64 const m_air,           // OU coil air mass flow rate [kg/s]
                                       Real64 const T_coil_in,       // Temperature of air at OU coil inlet [C]
                                       Real64 const W_coil_in,       // Humidity ratio of air at OU coil inlet [kg/kg]
                                       Real64 const OutdoorPressure, // Outdoor air pressure [Pa]
                                       Real64 &T_coil_surf,          // Air temperature at coil surface [C]
                                       Real64 &TeTc                  // VRF Tc at cooling mode, or Te at heating mode [C]
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rongpeng Zhang, LBNL
    //       DATE WRITTEN   Jan 2016
    //       MODIFIED       na
    //
    //       RE-ENGINEERED  na
    //
    // PURPOSE OF THIS SUBROUTINE:
    //        Calculate the VRF OU refrigerant side temperature, i.e., condensing temperature
    //        at cooling mode, or evaporating temperature at heating mode, given the coil heat
    //        release/extract amount and air side parameters.
    //
    // METHODOLOGY EMPLOYED:
    //        This is part of the physics based VRF model applicable for Fluid Temperature Control.
    //

    Real64 BF;              // VRF OU bypass  [-]
    Real64 deltaT;          // Difference between Te/Tc and air temperature at coil surface [C]
    Real64 h_coil_in;       // Enthalpy of air at OU coil inlet [C]
    Real64 h_coil_out;      // Enthalpy of air at OU coil outlet [C]
    Real64 T_coil_out;      // Air temperature at coil outlet [C]
    Real64 T_coil_surf_sat; // Saturated air temperature at coil surface [C]
    Real64 W_coil_surf_sat; // Humidity ratio of saturated air at coil surface [kg/kg]

    if (OperationMode == HXOpMode::CondMode) {
        // IU Cooling: OperationMode 0

        if (m_air <= 0) {
            TeTc = this->CondensingTemp;
            ShowSevereMessage(state, format(" Unreasonable outdoor unit airflow rate ({:.3T} ) for \"{}\":", m_air, this->Name));
            ShowContinueError(state, " This cannot be used to calculate outdoor unit refrigerant temperature.");
            ShowContinueError(state, format(" Default condensing temperature is used: {:.3T}", TeTc));
        }

        BF = this->RateBFOUCond; // 0.219;
        T_coil_out = T_coil_in + Q_coil / 1005.0 / m_air;
        T_coil_surf = T_coil_in + (T_coil_out - T_coil_in) / (1 - BF);

        deltaT = this->C3Tc * pow_2(SHSC) + this->C2Tc * SHSC + this->C1Tc;

        TeTc = T_coil_surf + deltaT;

    } else if (OperationMode == HXOpMode::EvapMode) {
        // IU Heating: OperationMode 1

        if (m_air <= 0) {
            TeTc = this->EvaporatingTemp;
            ShowSevereMessage(state, format(" Unreasonable outdoor unit airflow rate ({:.3T} ) for \"{}\":", m_air, this->Name));
            ShowContinueError(state, " This cannot be used to calculate outdoor unit refrigerant temperature.");
            ShowContinueError(state, format(" Default condensing temperature is used: {:.3T}", TeTc));
        }

        BF = this->RateBFOUEvap; // 0.45581;
        h_coil_in = PsyHFnTdbW(T_coil_in, W_coil_in);
        h_coil_out = h_coil_in - Q_coil / m_air / (1 - BF);
        h_coil_out = max(0.01, h_coil_out);

        T_coil_surf_sat = PsyTsatFnHPb(state, h_coil_out, OutdoorPressure, "VRFOU_TeTc");
        W_coil_surf_sat = PsyWFnTdbH(state, T_coil_surf_sat, h_coil_out, "VRFOU_TeTc");

        if (W_coil_surf_sat < W_coil_in) {
            // There is dehumidification
            T_coil_surf = T_coil_surf_sat;
        } else {
            // No dehumidification
            T_coil_surf = PsyTdbFnHW(h_coil_out, W_coil_in);
        }

        deltaT = this->C3Te * pow_2(SHSC) + this->C2Te * SHSC + this->C1Te;

        TeTc = T_coil_surf - deltaT;
    }
}

Real64 VRFCondenserEquipment::VRFOU_Cap(EnergyPlusData &state,
                                        HXOpMode const OperationMode, // Mode 0 for running as condenser, 1 for evaporator
                                        Real64 const TeTc,            // VRF Tc at cooling mode, or Te at heating mode [C]
                                        Real64 const SHSC,            // SC for OU condenser or SH for OU evaporator [C]
                                        Real64 const m_air,           // OU coil air mass flow rate [kg/s]
                                        Real64 const T_coil_in,       // Temperature of air at OU coil inlet [C]
                                        Real64 const W_coil_in        // Humidity ratio of air at OU coil inlet [kg/kg]
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rongpeng Zhang, LBNL
    //       DATE WRITTEN   Jan 2016
    //       MODIFIED       na
    //
    //       RE-ENGINEERED  na
    //
    // PURPOSE OF THIS SUBROUTINE:
    //        Calculate the VRF OU load, given refrigerant side temperature, i.e., condensing temperature
    //        and SC for condenser, or evaporating temperature and SH for evaporator.
    //
    // METHODOLOGY EMPLOYED:
    //        This is part of the physics based VRF model applicable for Fluid Temperature Control.

    Real64 BF;              // VRF OU bypass [-]
    Real64 deltaT;          // Difference between Te/Tc and air temperature at coil surface [C]
    Real64 h_coil_in;       // Enthalpy of air at OU coil inlet [C]
    Real64 h_coil_out;      // Enthalpy of air at OU coil outlet [C]
    Real64 Q_coil;          // OU coil heat release at cooling mode or heat extract at heating mode [W]
    Real64 T_coil_out;      // Air temperature at coil outlet [C]
    Real64 T_coil_surf;     // Air temperature at coil surface [C]
    Real64 W_coil_surf_sat; // Humidity ratio of saturated air at coil surface [kg/kg]

    Q_coil = 0.0;

    if (OperationMode == HXOpMode::CondMode) {
        // IU Cooling: OperationMode 0
        if (m_air <= 0) {
            ShowSevereMessage(state, format(" Unreasonable outdoor unit airflow rate ({:.3T} ) for \"{}\":", m_air, this->Name));
            ShowContinueError(state, " This cannot be used to calculate outdoor unit capacity.");
        }

        BF = this->RateBFOUCond; // 0.219;
        deltaT = this->C3Tc * pow_2(SHSC) + this->C2Tc * SHSC + this->C1Tc;
        T_coil_surf = TeTc - deltaT;
        T_coil_out = T_coil_in + (T_coil_surf - T_coil_in) * (1 - BF);
        Q_coil = (T_coil_out - T_coil_in) * 1005.0 * m_air;

    } else if (OperationMode == HXOpMode::EvapMode) {
        // IU Heating: OperationMode 1
        if (m_air <= 0) {
            ShowSevereMessage(state, format(" Unreasonable outdoor unit airflow rate ({:.3T} ) for \"{}\":", m_air, this->Name));
            ShowContinueError(state, " This cannot be used to calculate outdoor unit capacity.");
        }

        BF = this->RateBFOUEvap; // 0.45581;
        deltaT = this->C3Te * pow_2(SHSC) + this->C2Te * SHSC + this->C1Te;
        T_coil_surf = TeTc + deltaT;

        // saturated humidity ratio corresponding to T_coil_surf
        W_coil_surf_sat = PsyWFnTdpPb(state, T_coil_surf, state.dataEnvrn->OutBaroPress);

        if (W_coil_surf_sat < W_coil_in) {
            // There is dehumidification, W_coil_out = W_coil_surf_sat
            h_coil_out = PsyHFnTdbW(T_coil_surf, W_coil_surf_sat);
        } else {
            // No dehumidification, W_coil_out = W_coil_in
            h_coil_out = PsyHFnTdbW(T_coil_surf, W_coil_in);
        }
        h_coil_out = max(0.01, h_coil_out);
        h_coil_in = PsyHFnTdbW(T_coil_in, W_coil_in);
        Q_coil = (h_coil_in - h_coil_out) * m_air * (1 - BF); // bypass airflow should not be included here

    } else {
        // Should not come here
        ShowSevereMessage(state, format(" Unreasonable outdoor unit operational mode for \"{}\":", this->Name));
        ShowContinueError(state, " The operational mode is not correctly set in the function VRFOU_Cap.");
    }

    return Q_coil;
}

Real64 VRFCondenserEquipment::VRFOU_FlowRate(EnergyPlusData &state,
                                             HXOpMode const OperationMode, // Mode 0 for running as condenser, 1 for evaporator
                                             Real64 const TeTc,            // VRF Tc at cooling mode, or Te at heating mode [C]
                                             Real64 const SHSC,            // SC for OU condenser or SH for OU evaporator [C]
                                             Real64 const Q_coil,          // absolute value of OU coil heat release or heat extract [W]
                                             Real64 const T_coil_in,       // Temperature of air at OU coil inlet [C]
                                             Real64 const W_coil_in        // Humidity ratio of air at OU coil inlet [kg/kg]
) const
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rongpeng Zhang, LBNL
    //       DATE WRITTEN   Mar 2016
    //       MODIFIED       na
    //
    //       RE-ENGINEERED  na
    //
    // PURPOSE OF THIS SUBROUTINE:
    //        Calculate the outdoor unit fan flow rate, given VRF OU load and refrigerant side temperature, i.e.,
    //        condensing temperature and SC for condenser, or evaporating temperature and SH for evaporator.
    //
    // METHODOLOGY EMPLOYED:
    //        This is part of the physics based VRF model applicable for Fluid Temperature Control.

    Real64 BF;              // VRF OU bypass [-]
    Real64 deltaT;          // Difference between Te/Tc and air temperature at coil surface [C]
    Real64 h_coil_in;       // Enthalpy of air at OU coil inlet [C]
    Real64 h_coil_out;      // Enthalpy of air at OU coil outlet [C]
    Real64 m_air;           // OU coil air mass flow rate [kg/s]
    Real64 T_coil_out;      // Air temperature at coil outlet [C]
    Real64 T_coil_surf;     // Air temperature at coil surface [C]
    Real64 W_coil_surf_sat; // Humidity ratio of saturated air at coil surface [kg/kg]

    m_air = 0.0;

    if (OperationMode == HXOpMode::CondMode) {
        // IU Cooling: OperationMode 0

        BF = this->RateBFOUCond; // 0.219;
        deltaT = this->C3Tc * pow_2(SHSC) + this->C2Tc * SHSC + this->C1Tc;
        T_coil_surf = TeTc - deltaT;
        T_coil_out = T_coil_in + (T_coil_surf - T_coil_in) * (1 - BF);
        m_air = Q_coil / (T_coil_out - T_coil_in) / 1005.0;

    } else if (OperationMode == HXOpMode::EvapMode) {
        // IU Heating: OperationMode 1

        BF = this->RateBFOUEvap; // 0.45581;
        deltaT = this->C3Te * pow_2(SHSC) + this->C2Te * SHSC + this->C1Te;
        T_coil_surf = TeTc + deltaT;

        // saturated humidity ratio corresponding to T_coil_surf
        W_coil_surf_sat = PsyWFnTdpPb(state, T_coil_surf, state.dataEnvrn->OutBaroPress);

        if (W_coil_surf_sat < W_coil_in) {
            // There is dehumidification, W_coil_out = W_coil_surf_sat
            h_coil_out = PsyHFnTdbW(T_coil_surf, W_coil_surf_sat);
        } else {
            // No dehumidification, W_coil_out = W_coil_in
            h_coil_out = PsyHFnTdbW(T_coil_surf, W_coil_in);
        }
        h_coil_out = max(0.01, h_coil_out);
        h_coil_in = PsyHFnTdbW(T_coil_in, W_coil_in);
        m_air = Q_coil / (h_coil_in - h_coil_out) / (1 - BF);

    } else {
        // Should not come here
        ShowSevereMessage(state, format(" Unreasonable outdoor unit operational mode for \"{}\":", this->Name));
        ShowContinueError(state, " The operational mode is not correctly set in the function VRFOU_Cap.");
    }

    return m_air;
}

Real64 VRFCondenserEquipment::VRFOU_SCSH(EnergyPlusData &state,
                                         HXOpMode const OperationMode, // Mode 0 for running as condenser, 1 for evaporator
                                         Real64 const Q_coil,          // // OU coil heat release at cooling mode or heat extract at heating mode [W]
                                         Real64 const TeTc,            // VRF Tc at cooling mode, or Te at heating mode [C]
                                         Real64 const m_air,           // OU coil air mass flow rate [kg/s]
                                         Real64 const T_coil_in,       // Temperature of air at OU coil inlet [C]
                                         Real64 const W_coil_in,       // Humidity ratio of air at OU coil inlet [kg/kg]
                                         Real64 const OutdoorPressure  // Outdoor air pressure [Pa]
) const
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rongpeng Zhang, LBNL
    //       DATE WRITTEN   Jan 2016
    //       MODIFIED       na
    //
    //       RE-ENGINEERED  na
    //
    // PURPOSE OF THIS SUBROUTINE:
    //        Calculate the SC for OU condenser, or SH for OU evaporator, given
    //        VRF OU load and refrigerant side temperature, i.e., condensing temperature
    //        for condenser, or evaporating temperature for evaporator.
    //
    // METHODOLOGY EMPLOYED:
    //        This is part of the physics based VRF model applicable for Fluid Temperature Control.

    Real64 BF;              // VRF OU bypass [-]
    Real64 deltaT;          // Difference between Te/Tc and air temperature at coil surface [C]
    Real64 h_coil_in;       // Enthalpy of air at OU coil inlet [C]
    Real64 h_coil_out;      // Enthalpy of air at OU coil outlet [C]
    Real64 SHSC;            // SC for OU condenser, or SH for OU evaporator
    Real64 T_coil_out;      // Air temperature at coil outlet [C]
    Real64 T_coil_surf;     // Air temperature at coil surface [C]
    Real64 T_coil_surf_sat; // Saturated air temperature at coil surface [C]
    Real64 W_coil_surf_sat; // Humidity ratio of saturated air at coil surface [kg/kg]

    SHSC = 0.0;

    if (OperationMode == HXOpMode::CondMode) {
        // Cooling: OperationMode 0
        if (m_air <= 0) {
            ShowSevereMessage(state, format(" Unreasonable outdoor unit airflow rate ({:.3T} ) for \"{}\":", m_air, this->Name));
            ShowContinueError(state, " This cannot be used to calculate outdoor unit subcooling.");
        }

        BF = this->RateBFOUCond; // 0.219;
        T_coil_out = T_coil_in + Q_coil / 1005.0 / m_air;
        T_coil_surf = T_coil_in + (T_coil_out - T_coil_in) / (1 - BF);
        deltaT = TeTc - T_coil_surf;

        // SC_OU
        if (this->C3Tc == 0) {
            SHSC = -(this->C1Tc - deltaT) / this->C2Tc;
        } else {
            SHSC = (-this->C2Tc + std::pow((pow_2(this->C2Tc) - 4 * (this->C1Tc - deltaT) * this->C3Tc), 0.5)) / (2 * this->C3Tc);
        }

    } else if (OperationMode == HXOpMode::EvapMode) {
        // Heating: OperationMode 1
        if (m_air <= 0) {
            ShowSevereMessage(state, format(" Unreasonable outdoor unit airflow rate ({:.3T} ) for \"{}\":", m_air, this->Name));
            ShowContinueError(state, " This cannot be used to calculate outdoor unit super heating.");
        }

        BF = this->RateBFOUEvap; // 0.45581;
        h_coil_in = PsyHFnTdbW(T_coil_in, W_coil_in);
        h_coil_out = h_coil_in - Q_coil / m_air / (1 - BF);
        h_coil_out = max(0.01, h_coil_out);

        T_coil_surf_sat = PsyTsatFnHPb(state, h_coil_out, OutdoorPressure, "VRFOU_TeTc");
        W_coil_surf_sat = PsyWFnTdbH(state, T_coil_surf_sat, h_coil_out, "VRFOU_TeTc");

        if (W_coil_surf_sat < W_coil_in) {
            // There is dehumidification
            T_coil_surf = T_coil_surf_sat;
        } else {
            // No dehumidification
            T_coil_surf = PsyTdbFnHW(h_coil_out, W_coil_in);
        }

        deltaT = T_coil_surf - TeTc;

        // SH_OU
        if (this->C3Te == 0) {
            SHSC = -(this->C1Te - deltaT) / this->C2Te;
        } else {
            SHSC = (-this->C2Te + std::pow((pow_2(this->C2Te) - 4 * (this->C1Te - deltaT) * this->C3Te), 0.5)) / (2 * this->C3Te);
        }

    } else {
        // Should not come here
        ShowSevereMessage(state, format(" Unreasonable outdoor unit operational mode for \"{}\":", this->Name));
        ShowContinueError(state, " The operational mode is not correctly set in the function VRFOU_Cap.");
    }

    return SHSC;
}

Real64 VRFCondenserEquipment::VRFOU_CapModFactor(
    EnergyPlusData &state,
    Real64 const h_comp_in_real, // Enthalpy of refrigerant at the compressor inlet at real conditions [kJ/kg]
    Real64 const h_evap_in_real, // Enthalpy of refrigerant at the evaporator inlet at real conditions [kJ/kg]
    Real64 const P_evap_real,    // Evaporative pressure at real conditions [Pa]
    Real64 const T_comp_in_real, // Temperature of the refrigerant at the compressor inlet at real conditions [C]
    Real64 const T_comp_in_rate, // Temperature of the refrigerant at the compressor inlet at rated conditions [C]
    Real64 const T_cond_out_rate // Temperature of the refrigerant at the condenser outlet at rated conditions [C]
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rongpeng Zhang
    //       DATE WRITTEN   Nov 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate capacity modification factor for the compressors at Outdoor Unit.
    // This factor is used to modify the system evaporative capacity, by describing
    // the difference between rated conditions and real conditions.

    // METHODOLOGY EMPLOYED:
    // This is part of the VRF-FluidTCtrl Model.

    Real64 C_cap_density;   // Compressor capacity modification algorithm_modified flow rate [-]
    Real64 C_cap_enthalpy;  // Compressor capacity modification algorithm_modified enthalpy difference [-]
    Real64 C_cap_operation; // Compressor capacity modification algorithm_modified Cap [-]
    Real64 RefTSat;         // Saturated temperature of the refrigerant. Used to check whether the refrigerant is in the superheat area [C].
    Real64 h_evap_out_rate; // enthalpy of refrigerant at the evaporator outlet at rated conditions [kJ/kg]
    Real64 h_evap_in_rate;  // enthalpy of refrigerant at the evaporator inlet at rated conditions [kJ/kg]
    Real64 density_rate;    // density of refrigerant at rated conditions [kg/m3]
    Real64 density_real;    // density of refrigerant at rated conditions [kg/m3]

    static constexpr std::string_view RoutineName("VRFOU_CapModFactor");

    // Saturated temperature at real evaporating pressure
    RefTSat = this->refrig->getSatTemperature(state, P_evap_real, RoutineName);

    // Enthalpy at rated conditions
    h_evap_out_rate = this->refrig->getSupHeatEnthalpy(state, max(RefTSat, T_comp_in_rate), P_evap_real, RoutineName);
    h_evap_in_rate = this->refrig->getSatEnthalpy(state, T_cond_out_rate, 0.0, RoutineName);

    // Density calculations
    density_rate = this->refrig->getSupHeatDensity(state, T_comp_in_rate, P_evap_real, RoutineName);
    density_real = this->refrig->getSupHeatDensity(state, T_comp_in_real, P_evap_real, RoutineName);

    // Modification factor calculations
    if (density_real > 0) {
        C_cap_density = density_rate / density_real;
    } else {
        C_cap_density = 1.0;
    }

    if ((h_comp_in_real - h_evap_in_real) > 0) {
        C_cap_enthalpy = std::abs(h_evap_out_rate - h_evap_in_rate) / std::abs(h_comp_in_real - h_evap_in_real);
    } else {
        C_cap_enthalpy = 1.0;
    }

    C_cap_operation = C_cap_density * C_cap_enthalpy;

    return C_cap_operation;
}

void VRFCondenserEquipment::VRFOU_TeModification(
    EnergyPlusData &state,
    Real64 const Te_up,          // Upper bound of Te during iteration, i.e., Te before reduction [C]
    Real64 const Te_low,         // Lower bound of Te during iteration, i.e., the given suction temperature Te' [C]
    Real64 const Pipe_h_IU_in,   // Piping Loss Algorithm Parameter: enthalpy of IU at inlet [kJ/kg]
    Real64 const OutdoorDryBulb, // outdoor dry-bulb temperature [C]
    Real64 &Te_update,           // Updated Te that can generate the required Tsuction [C]
    Real64 &Pe_update,           // Piping Loss Algorithm Parameter: evaporating pressure assumed for iterations [Pa]
    Real64 &Pipe_m_ref,          // Piping Loss Algorithm Parameter: Refrigerant mass flow rate [kg/s]
    Real64 &Pipe_h_IU_out,       // Piping Loss Algorithm Parameter: enthalpy of IU at outlet [kJ/kg]
    Real64 &Pipe_SH_merged       // Piping Loss Algorithm Parameter: Average SH after the indoor units [C]
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rongpeng Zhang
    //       DATE WRITTEN   Jan 2016
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This is part of the low load modification algorithm for the VRF-FluidTCtrl model. It aims
    // to find a new Te (Te_update) that can generate a new compressor suction temperature (Tsuction) equaling
    // to the given compressor suction temperature (Te_low). This requires the re-calculate of piping loss.

    // METHODOLOGY EMPLOYED:
    // This is part of the VRF-FluidTCtrl Model.

    int CoolCoilIndex;      // index to cooling coil in terminal unit
    int NumTUInList;        // number of terminal units is list
    int NumTeIte;           // counter for Te calculation iterations [-]
    int TUListNum;          // index to TU List
    int TUIndex;            // Index to terminal unit
    Real64 MaxNumTeIte;     // Piping Loss Algorithm Parameter: max number of iterations for Te [-]
    Real64 Pipe_h_comp_in;  // Piping Loss Algorithm Parameter: Enthalpy after piping loss (compressor inlet) [kJ/kg]
    Real64 Pipe_DeltP;      // Piping Loss Algorithm Parameter: Pipe pressure drop [Pa]
    Real64 Pipe_Q;          // Piping Loss Algorithm Parameter: Heat loss [W]
    Real64 Pipe_m_ref_i;    // Piping Loss Algorithm Parameter: Refrigerant mass flow rate for a individual IU[kg/s]
    Real64 Pipe_h_IU_out_i; // Piping Loss Algorithm Parameter: enthalpy of IU at outlet (individual) [kJ/kg]
    Real64 RefTSat;         // Saturated temperature of the refrigerant [C]
    Real64 RefPLow;         // Low Pressure Value for Ps (>0.0) [Pa]
    Real64 RefPHigh;        // High Pressure Value for Ps (max in tables) [Pa]
    Real64 SH_IU_update;    // Modified SH for VRF IU [C]
    Real64 Te_ItePreci;     // Precision of iterations for Te [C]he superheat area [C]
    Real64 Tfs;             // Temperature of the air at the coil surface [C]]
    Real64 Tsuction;        // VRF compressor suction refrigerant temperature [Pa]

    static constexpr std::string_view RoutineName("VRFOU_TeModification");

    // variable initializations
    TUListNum = this->ZoneTUListPtr;
    RefPLow = this->refrig->PsLowPresValue;
    RefPHigh = this->refrig->PsHighPresValue;
    NumTUInList = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).NumTUInList;

    // Initialization of Te iterations (Label11)
    NumTeIte = 1;
    Te_ItePreci = 0.1;
    MaxNumTeIte = (Te_up - Te_low) / Te_ItePreci + 1; // upper bound and lower bound of Te iterations
    Te_update = Te_up - Te_ItePreci;

    bool converged_11;
    // The following is triggered in VariableRefrigerantFlow_FluidTCtrl_HR_5Zone.idf
    do {
        Pipe_m_ref = 0; // Total Ref Flow Rate( kg/s )
        Pipe_h_IU_out = 0;
        Pipe_h_IU_out_i = 0;
        Pipe_m_ref_i = 0;
        Pipe_SH_merged = 0;
        Pe_update = this->refrig->getSatPressure(state, Te_update, RoutineName);

        // Re-calculate total refrigerant flow rate, with updated SH
        for (int NumTU = 1; NumTU <= NumTUInList; NumTU++) {
            if (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) > 0) {
                TUIndex = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
                CoolCoilIndex = state.dataHVACVarRefFlow->VRFTU(TUIndex).CoolCoilIndex;

                // The IU coil surface temperature should be the same.
                Tfs = Te_up + (this->C3Te * pow_2(state.dataDXCoils->DXCoil(CoolCoilIndex).ActualSH) +
                               this->C2Te * state.dataDXCoils->DXCoil(CoolCoilIndex).ActualSH + this->C1Te);

                // SH_IU_update is the updated SH for a specific IU
                if (this->C3Te == 0) {
                    SH_IU_update = -(this->C1Te - Tfs + Te_update) / this->C2Te;
                } else {
                    SH_IU_update =
                        (-this->C2Te + std::pow((pow_2(this->C2Te) - 4 * (this->C1Te - Tfs + Te_update) * this->C3Te), 0.5)) / (2 * this->C3Te);
                }

                RefTSat = this->refrig->getSatTemperature(state, Pe_update, RoutineName);
                Pipe_h_IU_out_i = this->refrig->getSupHeatEnthalpy(state,
                                                                   max(RefTSat, Te_update + SH_IU_update),
                                                                   Pe_update,
                                                                   RoutineName); // hB_i for the IU

                if (Pipe_h_IU_out_i > Pipe_h_IU_in) {
                    Pipe_m_ref_i =
                        (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) <= 0.0)
                            ? 0.0
                            : (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) / (Pipe_h_IU_out_i - Pipe_h_IU_in));
                    Pipe_m_ref = Pipe_m_ref + Pipe_m_ref_i;
                    Pipe_SH_merged = Pipe_SH_merged + Pipe_m_ref_i * SH_IU_update;
                    Pipe_h_IU_out = Pipe_h_IU_out + Pipe_m_ref_i * Pipe_h_IU_out_i;
                }
            }
        }
        if (Pipe_m_ref > 0) {
            Pipe_h_IU_out = Pipe_h_IU_out / Pipe_m_ref;
            Pipe_SH_merged = Pipe_SH_merged / Pipe_m_ref;
        } else {
            Pipe_SH_merged = this->SH;
            RefTSat = this->refrig->getSatTemperature(state, Pe_update, RoutineName);
            Pipe_h_IU_out = this->refrig->getSupHeatEnthalpy(state, max(RefTSat, Te_update + Pipe_SH_merged), Pe_update, RoutineName);
        }

        // Re-calculate piping loss
        this->VRFOU_PipeLossC(state, Pipe_m_ref, Pe_update, Pipe_h_IU_out, Pipe_SH_merged, OutdoorDryBulb, Pipe_Q, Pipe_DeltP, Pipe_h_comp_in);

        Tsuction = this->refrig->getSatTemperature(state, max(min(Pe_update - Pipe_DeltP, RefPHigh), RefPLow), RoutineName);
        converged_11 = !((std::abs(Tsuction - Te_low) > 0.5) && (Te_update < Te_up) && (Te_update > Te_low) && (NumTeIte < MaxNumTeIte));
        Te_update = Te_update - 0.1;
        NumTeIte = NumTeIte + 1;
    } while (!converged_11);

    if (std::abs(Tsuction - Te_low) > 0.5) {
        NumTeIte = 999;
        Tsuction = Te_low;
        Pipe_SH_merged = 3.0;
        Te_update = Te_low + 1;
    }
}

void VRFCondenserEquipment::VRFOU_CompSpd(
    EnergyPlusData &state,
    Real64 const Q_req,        // Required capacity [W]
    HXOpMode const Q_type,     // Required capacity type:  0 for condenser, 1 for evaporator
    Real64 const T_suction,    // Compressor suction temperature Te' [C]
    Real64 const T_discharge,  // Compressor discharge temperature Tc' [C]
    Real64 const h_IU_evap_in, // Enthalpy of IU at inlet, for C_cap_operation calculation [kJ/kg]
    Real64 const h_comp_in,    // Enthalpy after piping loss (compressor inlet), for C_cap_operation calculation [kJ/kg]
    Real64 &CompSpdActual      // Actual compressor running speed [rps]
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rongpeng Zhang, LBNL
    //       DATE WRITTEN   Feb 2016
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //       This subroutine specifies the compressor speed at given operational conditions to meet the evaporator or condenser capacity provided.

    // METHODOLOGY EMPLOYED:
    //        This is part of the VRF-FluidTCtrl Model.

    using Curve::CurveValue;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    int CounterCompSpdTemp;                // Index for the compressor speed level[-]
    int CompSpdLB;                         // index for Compressor speed low bound [-]
    int CompSpdUB;                         // index for Compressor speed up bound [-]
    int NumOfCompSpdInput;                 // Number of compressor speed input by the user [-]
    Real64 C_cap_operation;                // Compressor capacity modification algorithm_modified Cap [-]
    Real64 P_suction;                      // Compressor suction pressure Pe' [Pa]
    Real64 Q_evap_req;                     // Required evaporative capacity [W]
    Real64 Q_cond_req;                     // Required evaporative capacity [W]
    Real64 RefPLow;                        // Low Pressure Value for Ps (>0.0) [Pa]
    Real64 RefPHigh;                       // High Pressure Value for Ps (max in tables) [Pa]
    Real64 SH_Comp;                        // Temperature between compressor inlet temperature and evaporative temperature Te' [C]
    Real64 T_comp_in;                      // Refrigerant temperature at compressor inlet (after piping loss) [C]
    Array1D<Real64> CompEvaporatingPWRSpd; // Array for the compressor power at certain speed [W]
    Array1D<Real64> CompEvaporatingCAPSpd; // Array for the evaporating capacity at certain speed [W]

    static constexpr std::string_view RoutineName("VRFOU_CompSpd");

    // variable initializations: component index
    RefPLow = this->refrig->PsLowPresValue;
    RefPHigh = this->refrig->PsHighPresValue;

    // variable initializations: compressor
    NumOfCompSpdInput = this->CompressorSpeed.size();
    CompEvaporatingPWRSpd.dimension(NumOfCompSpdInput);
    CompEvaporatingCAPSpd.dimension(NumOfCompSpdInput);

    // variable initializations: system operational parameters
    P_suction = this->refrig->getSatPressure(state, T_suction, RoutineName);
    T_comp_in = this->refrig->getSupHeatTemp(state, max(min(P_suction, RefPHigh), RefPLow), h_comp_in, T_suction + 3, T_suction + 30, RoutineName);
    SH_Comp = T_comp_in - T_suction;

    // Calculate capacity modification factor
    C_cap_operation = this->VRFOU_CapModFactor(
        state, h_comp_in, h_IU_evap_in, max(min(P_suction, RefPHigh), RefPLow), T_suction + SH_Comp, T_suction + 8, T_discharge - 5);

    if (Q_type == HXOpMode::EvapMode) {
        // Capacity to meet is for evaporator

        Q_evap_req = Q_req;

        for (CounterCompSpdTemp = 1; CounterCompSpdTemp <= NumOfCompSpdInput; CounterCompSpdTemp++) {
            // Iteration to find the VRF speed that can meet the required load, Iteration DoName1

            CompEvaporatingPWRSpd(CounterCompSpdTemp) =
                this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(CounterCompSpdTemp), T_discharge, T_suction);
            CompEvaporatingCAPSpd(CounterCompSpdTemp) =
                this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(CounterCompSpdTemp), T_discharge, T_suction);

            if (Q_evap_req * C_cap_operation <= CompEvaporatingCAPSpd(CounterCompSpdTemp)) {
                // Compressor speed stage CounterCompSpdTemp need not to be increased, finish Iteration DoName1

                if (CounterCompSpdTemp > 1) {

                    CompSpdLB = CounterCompSpdTemp - 1;
                    CompSpdUB = CounterCompSpdTemp;

                    CompSpdActual = this->CompressorSpeed(CompSpdLB) + (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB)) /
                                                                           (CompEvaporatingCAPSpd(CompSpdUB) - CompEvaporatingCAPSpd(CompSpdLB)) *
                                                                           (Q_evap_req * C_cap_operation - CompEvaporatingCAPSpd(CompSpdLB));

                } else {
                    CompSpdActual = this->CompressorSpeed(1) * (Q_evap_req * C_cap_operation) / CompEvaporatingCAPSpd(1);
                }

                break; // EXIT DoName1
            }
        } // End: Iteration DoName1

        if (CounterCompSpdTemp > NumOfCompSpdInput) {
            CompSpdActual = this->CompressorSpeed(NumOfCompSpdInput);
        }

    } else {
        // Capacity to meet is for condenser

        // derivation process
        // let
        // k be the speed level
        // C be the short form for C_cap_operation
        // PWR(.) be the short form of CompEvaporatingPWRSpd(.)
        // CAP(.) be the short form of CompEvaporatingCAPSpd(.)
        // spd(.) be the short form of this->CompressorSpeed(.)
        // deltaPWR be PWR(k) - PWR(k - 1)
        // deltaCAP be CAP(k) - CAP(k - 1)
        //
        // compressor heat = deltaPWR * r + PWR(k - 1)
        // so
        // Q_cond_req = Q_req = Q_evap_req + deltaPWR * r + PWR(k - 1) <---- eq 1
        //
        // we also know this from the CompSpdActual calculation equation that
        // CompSpdActual = Spd(k - 1) + deltaSpd / deltaCAP * (Q_evap_req * C - CAP(k - 1))
        // so we call the following r
        // r = (Q_evap_req * C - CAP(k - 1)) / deltaCAP
        // so
        // CompSpdActual = Spd(k - 1) + deltaSpd * r
        //
        // arranging terms in eq 1
        // Q_cond_req - deltaPWR * r - PWR(k - 1) = Q_evap_req
        // (Q_cond_req - deltaPWR * r - PWR(k - 1)) * C - CAP(k - 1) = Q_evap_req * C - CAP(k - 1)
        // ((Q_cond_req - deltaPWR * r - PWR(k - 1)) * C - CAP(k - 1)) / deltaCAP = (Q_evap_req * C - CAP(k - 1)) / deltaCAP
        // ((Q_cond_req - deltaPWR * r - PWR(k - 1)) * C - CAP(k - 1)) / deltaCAP = r
        // (Q_cond_req - deltaPWR * r - PWR(k - 1)) * C - CAP(k - 1) = deltaCAP * r
        // (Q_cond_req - PWR(k - 1)) * C - CAP(k - 1) = deltaCAP * r + deltaPWR * r * C
        // so
        // r = ((Q_cond_req - PWR(k - 1)) * C - CAP(k - 1)) / (deltaCAP + deltaPWR * C)
        //   = ((Q_cond_req - PWR(k - 1)) - CAP(k - 1)/C) / (deltaCAP/C + deltaPWR)
        //
        // in the special case where k = 1, then k - 1, PWR(k - 1) and CAP(k - 1) will all be 0
        // r = ((Q_cond_req) * C) / (CAP(1) - PWR(1) * C)
        //   = (Q_cond_req) / (CAP(1)/C - PWR(1))

        Q_cond_req = Q_req;

        CounterCompSpdTemp = 1;
        CompEvaporatingPWRSpd(1) = this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(1), T_discharge, T_suction);
        CompEvaporatingCAPSpd(1) = this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(1), T_discharge, T_suction);
        Real64 r = (Q_cond_req * C_cap_operation) / (CompEvaporatingCAPSpd(1) + CompEvaporatingPWRSpd(1) * C_cap_operation);
        if (r < 1.0) {
            CompSpdActual = this->CompressorSpeed(1) * r;
            Q_evap_req = Q_cond_req - CompEvaporatingPWRSpd(1) * r;
        } else {
            for (CounterCompSpdTemp = 2; CounterCompSpdTemp <= NumOfCompSpdInput; CounterCompSpdTemp++) {
                CompEvaporatingPWRSpd(CounterCompSpdTemp) =
                    this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(CounterCompSpdTemp), T_discharge, T_suction);
                CompEvaporatingCAPSpd(CounterCompSpdTemp) =
                    this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(CounterCompSpdTemp), T_discharge, T_suction);
                CompSpdLB = CounterCompSpdTemp - 1;
                CompSpdUB = CounterCompSpdTemp;
                Real64 deltaCAP = CompEvaporatingCAPSpd(CompSpdUB) - CompEvaporatingCAPSpd(CompSpdLB);
                Real64 deltaPWR = CompEvaporatingPWRSpd(CompSpdUB) - CompEvaporatingPWRSpd(CompSpdLB);
                Real64 r = ((Q_cond_req - CompEvaporatingPWRSpd(CompSpdLB)) * C_cap_operation - CompEvaporatingCAPSpd(CompSpdLB)) /
                           (deltaCAP + deltaPWR * C_cap_operation);
                if (r < 1.0) {
                    CompSpdActual = this->CompressorSpeed(CompSpdLB) + (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB)) * r;
                    Q_evap_req = Q_cond_req - deltaPWR * r - CompEvaporatingPWRSpd(CompSpdLB);
                    break;
                }
            }
        } // End: Iteration DoName1

        if (CounterCompSpdTemp > NumOfCompSpdInput) {
            CompSpdActual = this->CompressorSpeed(NumOfCompSpdInput);
        }
    }
}

void VRFCondenserEquipment::VRFOU_CompCap(
    EnergyPlusData &state,
    Real64 const CompSpdActual, // Given compressor speed
    Real64 const T_suction,     // Compressor suction temperature Te' [C]
    Real64 const T_discharge,   // Compressor discharge temperature Tc' [C]
    Real64 const h_IU_evap_in,  // Enthalpy of IU at inlet, for C_cap_operation calculation [kJ/kg]
    Real64 const h_comp_in,     // Enthalpy after piping loss (compressor inlet), for C_cap_operation calculation [kJ/kg]
    Real64 &Q_c_tot,            // Compressor evaporative capacity [W]
    Real64 &Ncomp               // Compressor power [W]
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rongpeng Zhang, LBNL
    //       DATE WRITTEN   Feb 2016
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //       This subroutine specifies the compressor performance (power and capacity) at given compressor speed and operational conditions.

    // METHODOLOGY EMPLOYED:
    //       This is part of the VRF-FluidTCtrl Model.

    using Curve::CurveValue;
    int CounterCompSpdTemp;                // Index for the compressor speed level[-]
    int CompSpdLB;                         // index for Compressor speed low bound [-]
    int CompSpdUB;                         // index for Compressor speed up bound [-]
    int NumOfCompSpdInput;                 // Number of compressor speed input by the user [-]
    Real64 C_cap_operation;                // Compressor capacity modification algorithm_modified Cap [-]
    Real64 P_suction;                      // Compressor suction pressure Pe' [Pa]
    Real64 Q_evap_sys;                     // evaporative capacity [W]
    Real64 RefPLow;                        // Low Pressure Value for Ps (>0.0) [Pa]
    Real64 RefPHigh;                       // High Pressure Value for Ps (max in tables) [Pa]
    Real64 SH_Comp;                        // Temperature between compressor inlet temperature and evaporative temperature Te' [C]
    Real64 T_comp_in;                      // Refrigerant temperature at compressor inlet (after piping loss) [C]
    Array1D<Real64> CompEvaporatingPWRSpd; // Array for the compressor power at certain speed [W]
    Array1D<Real64> CompEvaporatingCAPSpd; // Array for the evaporating capacity at certain speed [W]

    static constexpr std::string_view RoutineName("VRFOU_CompCap");

    // variable initializations: component index
    RefPLow = this->refrig->PsLowPresValue;
    RefPHigh = this->refrig->PsHighPresValue;

    // variable initializations: compressor
    NumOfCompSpdInput = this->CompressorSpeed.size();
    CompEvaporatingPWRSpd.dimension(NumOfCompSpdInput);
    CompEvaporatingCAPSpd.dimension(NumOfCompSpdInput);

    for (CounterCompSpdTemp = 1; CounterCompSpdTemp <= NumOfCompSpdInput; CounterCompSpdTemp++) {

        CompEvaporatingPWRSpd(CounterCompSpdTemp) =
            this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(CounterCompSpdTemp), T_discharge, T_suction);
        CompEvaporatingCAPSpd(CounterCompSpdTemp) =
            this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(CounterCompSpdTemp), T_discharge, T_suction);

        if (CompSpdActual <= this->CompressorSpeed(CounterCompSpdTemp)) {
            // Compressor speed stage CounterCompSpdTemp need not to be increased, finish Iteration DoName1

            if (CounterCompSpdTemp > 1) {

                CompSpdLB = CounterCompSpdTemp - 1;
                CompSpdUB = CounterCompSpdTemp;

                Q_evap_sys = CompEvaporatingCAPSpd(CompSpdLB) + (CompEvaporatingCAPSpd(CompSpdUB) - CompEvaporatingCAPSpd(CompSpdLB)) *
                                                                    (CompSpdActual - this->CompressorSpeed(CompSpdLB)) /
                                                                    (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB));
                Ncomp = CompEvaporatingPWRSpd(CompSpdLB) + (CompEvaporatingPWRSpd(CompSpdUB) - CompEvaporatingPWRSpd(CompSpdLB)) *
                                                               (CompSpdActual - this->CompressorSpeed(CompSpdLB)) /
                                                               (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB));

            } else {
                Q_evap_sys = CompEvaporatingCAPSpd(1) * CompSpdActual / this->CompressorSpeed(1);
                Ncomp = CompEvaporatingPWRSpd(1) * CompSpdActual / this->CompressorSpeed(1);
            }

            break;
        }
    }

    if (CounterCompSpdTemp > NumOfCompSpdInput) {
        Q_evap_sys = CompEvaporatingCAPSpd(NumOfCompSpdInput);
        Ncomp = CompEvaporatingPWRSpd(NumOfCompSpdInput);
    }

    // variable initializations: system operational parameters
    P_suction = this->refrig->getSatPressure(state, T_suction, RoutineName);
    T_comp_in = this->refrig->getSupHeatTemp(state, max(min(P_suction, RefPHigh), RefPLow), h_comp_in, T_suction + 3, T_suction + 30, RoutineName);
    SH_Comp = T_comp_in - T_suction;

    // Calculate capacity modification factor
    C_cap_operation = this->VRFOU_CapModFactor(
        state, h_comp_in, h_IU_evap_in, max(min(P_suction, RefPHigh), RefPLow), T_suction + SH_Comp, T_suction + 8, T_discharge - 5);
    C_cap_operation = min(1.5, max(0.5, C_cap_operation));
    Q_c_tot = Q_evap_sys / C_cap_operation;
}

void VRFCondenserEquipment::VRFOU_CalcCompC(EnergyPlusData &state,
                                            Real64 TU_load,            // Indoor unit cooling load [W]
                                            Real64 T_suction,          // Compressor suction temperature Te' [C]
                                            Real64 T_discharge,        // Compressor discharge temperature Tc' [C]
                                            Real64 P_suction,          // Compressor suction pressure Pe' [Pa]
                                            Real64 Pipe_T_comp_in,     // Refrigerant temperature at compressor inlet (after piping loss) [C]
                                            Real64 Pipe_h_comp_in,     // Enthalpy after piping loss (compressor inlet) [kJ/kg]
                                            Real64 Pipe_h_IU_in,       // Enthalpy of IU at inlet [kJ/kg]
                                            Real64 Pipe_Q,             // Piping Loss Algorithm Parameter: Heat loss [W]
                                            Real64 MaxOutdoorUnitTc,   // The maximum temperature that Tc can be at heating mode [C]
                                            Real64 &OUCondHeatRelease, // Condenser heat release (cooling mode) [W]
                                            Real64 &CompSpdActual,     // Actual compressor running speed [rps]
                                            Real64 &Ncomp,             // Compressor power [W]
                                            Real64 &CyclingRatio       // Cycling Ratio [W]
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Xiufeng Pang
    //       DATE WRITTEN   Feb 2014
    //       MODIFIED       Rongpeng Zhang, Jan 2016
    //       MODIFIED       Yujie Xu, Sep 2023
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine simulates the compressor performance at given operational conditions (cooling mode). More specifically, it specifies
    // the compressor speed to provide sufficient evaporative capacity, and calculate the power of the compressor running at the specified
    // speed. Note that it may be needed to manipulate the operational conditions to further adjust system capacity at low load conditions.
    // The low load modification logics are different for cooling mode and heating mode.

    // METHODOLOGY EMPLOYED:
    // This is part of the VRF-FluidTCtrl Model.

    using Curve::CurveValue;

    using General::SolveRoot;

    int CounterCompSpdTemp;                // Index for the compressor speed level[-]
    int CompSpdLB;                         // index for Compressor speed low bound [-]
    int CompSpdUB;                         // index for Compressor speed up bound [-]
    int CoolCoilIndex;                     // index to cooling coil in terminal unit
    int MaxIter(500);                      // max iteration number allowed [-]
    int NumOfCompSpdInput;                 // Number of compressor speed input by the user [-]
    int NumIteCcap;                        // counter for Ccap calculation iterations [-]
    int NumIteTe;                          // counter for Te calculation iterations [-]
    int NumTUInList;                       // number of terminal units is list
    int SolFla;                            // Slove flag for SolveRoot [-]
    int TUListNum;                         // index to TU List
    int TUIndex;                           // Index to terminal unit
    Real64 Cap_Eva0;                       // Evaporating capacity calculated based on physics model, used in the iterations [W]
    Real64 Cap_Eva1;                       // Evaporating capacity calculated by curves, used in the iterations [W]
    Real64 CapDiff;                        // Evaporating capacity difference used in the iterations [W]
    Real64 C_cap_operation;                // Compressor capacity modification algorithm_modified Cap [-]
    Real64 C_cap_operation0;               // Compressor capacity modification algorithm_modified Cap, for temporary use [-]
    Real64 SmallLoadTe;                    // Updated suction temperature at small load conditions (Te') [C]
    Real64 Modifi_SH;                      // Temperature between compressor inlet temperature and evaporative temperature Te' [C]
    Real64 MaxNumIteTe;                    // Piping Loss Algorithm Parameter: max number of iterations for Te [-]
    Real64 MinOutdoorUnitTe;               // The minimum temperature that Te can be at cooling mode (only used for calculating Min capacity)
    Real64 MinOutdoorUnitPe;               // The minimum pressure that Pe can be at cooling mode (only used for calculating Min capacity)
    Real64 MinRefriPe;                     // Minimum refrigerant evaporating pressure [Pa]
    Real64 Modifi_SHin;                    // Compressor power modification algorithm_modified SH for IDU [C]
    Real64 P_discharge;                    // VRF compressor discharge pressure [Pa]
    Real64 Pipe_m_ref;                     // Piping Loss Algorithm Parameter: Refrigerant mass flow rate [kg/s]
    Real64 Pipe_DeltP;                     // Piping Loss Algorithm Parameter: Pipe pressure drop [Pa]
    Real64 Pipe_Q0;                        // Compressor capacity modification algorithm_modified Pipe_Q, for temporary use [W]
    Real64 Pipe_m_ref_i;                   // Piping Loss Algorithm Parameter: Refrigerant mass flow rate for a individual IU[kg/s]
    Real64 Pipe_h_IU_out;                  // Piping Loss Algorithm Parameter: enthalpy of IU at outlet [kJ/kg]
    Real64 Pipe_h_IU_out_i;                // Piping Loss Algorithm Parameter: enthalpy of IU at outlet (individual) [kJ/kg]
    Real64 Pipe_Pe_assumed;                // Piping Loss Algorithm Parameter: evaporating pressure assumed for iterations[Pa]
    Real64 Pipe_SH_merged;                 // Piping Loss Algorithm Parameter: average super heating degrees after the indoor units [C]
    Real64 Pipe_Te_assumed;                // Piping Loss Algorithm Parameter: evaporating temperature assumed for iterations[C]
    Real64 Q_evap_req;                     // Required evaporative capacity [W]
    Real64 RefTSat;                        // Saturated temperature of the refrigerant [C]
    Real64 RefPLow;                        // Low Pressure Value for Ps (>0.0) [Pa]
    Real64 RefPHigh;                       // High Pressure Value for Ps (max in tables) [Pa]
    Real64 T_discharge_new;                // Condensing temperature, for temporary use in iterations [C]
    Real64 Tfs;                            // Temperature of the air at the coil surface [C]]
    Real64 constexpr Tolerance(0.05);      // Tolerance for condensing temperature calculation [C]
    Real64 constexpr TeTol(0.5);           // Tolerance for the difference between Te and SmallLoadTe
    Array1D<Real64> CompEvaporatingPWRSpd; // Array for the compressor power at certain speed [W]
    Array1D<Real64> CompEvaporatingCAPSpd; // Array for the evaporating capacity at certain speed [W]

    static constexpr std::string_view RoutineName("VRFOU_CalcCompC");

    // variable initializations
    NumOfCompSpdInput = this->CompressorSpeed.size();
    CompEvaporatingPWRSpd.dimension(NumOfCompSpdInput);
    CompEvaporatingCAPSpd.dimension(NumOfCompSpdInput);
    Q_evap_req = TU_load + Pipe_Q;

    TUListNum = this->ZoneTUListPtr;
    RefPLow = this->refrig->PsLowPresValue;
    RefPHigh = this->refrig->PsHighPresValue;
    NumTUInList = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).NumTUInList;

    Modifi_SH = Pipe_T_comp_in - T_suction;

    // set condenser entering air conditions (Outdoor air conditions)
    Real64 OutdoorDryBulb = state.dataEnvrn->OutDryBulbTemp;
    Real64 OutdoorHumRat = state.dataEnvrn->OutHumRat;
    Real64 OutdoorPressure = state.dataEnvrn->OutBaroPress;
    Real64 RhoAir = PsyRhoAirFnPbTdbW(state, OutdoorPressure, OutdoorDryBulb, OutdoorHumRat);

    // Calculate capacity modification factor
    C_cap_operation = this->VRFOU_CapModFactor(
        state, Pipe_h_comp_in, Pipe_h_IU_in, max(min(P_suction, RefPHigh), RefPLow), T_suction + Modifi_SH, T_suction + 8, T_discharge - 5);

    this->adjustedTe = false;
    for (CounterCompSpdTemp = 1; CounterCompSpdTemp <= NumOfCompSpdInput; CounterCompSpdTemp++) {
        // Iteration to find the VRF speed that can meet the required load, Iteration DoName1

        CompEvaporatingPWRSpd(CounterCompSpdTemp) =
            this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(CounterCompSpdTemp), T_discharge, T_suction);
        CompEvaporatingCAPSpd(CounterCompSpdTemp) =
            this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(CounterCompSpdTemp), T_discharge, T_suction);

        if (Q_evap_req * C_cap_operation <= CompEvaporatingCAPSpd(CounterCompSpdTemp)) {
            // Compressor speed stage CounterCompSpdTemp need not to be increased, finish Iteration DoName1

            if (CounterCompSpdTemp > 1) { // Since: if( CounterCompSpdTemp <= 1 )
                // Compressor speed > min

                CompSpdLB = CounterCompSpdTemp - 1;
                CompSpdUB = CounterCompSpdTemp;

                CompSpdActual = this->CompressorSpeed(CompSpdLB) + (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB)) /
                                                                       (CompEvaporatingCAPSpd(CompSpdUB) - CompEvaporatingCAPSpd(CompSpdLB)) *
                                                                       (Q_evap_req * C_cap_operation - CompEvaporatingCAPSpd(CompSpdLB));

                Ncomp = CompEvaporatingPWRSpd(CompSpdLB) + (CompEvaporatingPWRSpd(CompSpdUB) - CompEvaporatingPWRSpd(CompSpdLB)) /
                                                               (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB)) *
                                                               (CompSpdActual - this->CompressorSpeed(CompSpdLB));
                break; // EXIT DoName1

            } else {
                // Compressor runs at the min speed
                // Low Load Modification Algorithm for cooling (IU side modification)

                // Initialization of NumIteCcap iterations (Label13)
                Pipe_Q0 = Pipe_Q;
                C_cap_operation0 = C_cap_operation;
                T_discharge_new = T_discharge;
                NumIteCcap = 1;

                // Update the C_cap_operation
                bool converged_13;
                do {
                    Q_evap_req = TU_load + Pipe_Q0; // Pipe_Q0 is updated during the iteration
                    Pipe_h_IU_in = this->refrig->getSatEnthalpy(state, T_discharge_new - this->SC, 0.0, RoutineName);
                    CompSpdActual = this->CompressorSpeed(1);
                    Real64 CondHeat = Q_evap_req * C_cap_operation0 / this->RatedEvapCapacity; // 150130 To be confirmed
                    int CAPFT = this->OUCoolingCAPFT(CounterCompSpdTemp);

                    P_discharge = this->refrig->getSatPressure(state, T_discharge, RoutineName);
                    MinRefriPe = this->refrig->getSatPressure(state, -15, RoutineName);
                    MinOutdoorUnitPe = max(P_discharge - this->CompMaxDeltaP, MinRefriPe);
                    MinOutdoorUnitTe = this->refrig->getSatTemperature(state, max(min(MinOutdoorUnitPe, RefPHigh), RefPLow), RoutineName);
                    // Te can't be smaller than user input lower bound
                    MinOutdoorUnitTe = max(this->IUEvapTempLow, MinOutdoorUnitTe);

                    auto f = [&state, T_discharge_new, CondHeat, CAPFT](Real64 const T_suc) {
                        return CompResidual_FluidTCtrl(state, T_discharge_new, CondHeat, CAPFT, T_suc);
                    };

                    General::SolveRoot(state, 1.0e-3, MaxIter, SolFla, SmallLoadTe, f, MinOutdoorUnitTe,
                                       T_suction); // SmallLoadTe is the updated Te'
                    if (SolFla == -1) {
                        // show error not converging
                        ShowWarningMessage(state, format("{}: low load Te adjustment failed for {}", RoutineName, this->Name));
                        ShowContinueErrorTimeStamp(state, "");
                        ShowContinueError(state, format("  Iteration limit [{}] exceeded in calculating OU evaporating temperature", MaxIter));
                    } else if (SolFla == -2) {
                        this->LowLoadTeError++;
                        if (LowLoadTeError < 5) {
                            ShowWarningMessage(state,
                                               format("{}: no Te solution was found for {} f({})={} and f({})={} are the same sign",
                                                      RoutineName,
                                                      this->Name,
                                                      MinOutdoorUnitTe,
                                                      f(MinOutdoorUnitTe),
                                                      T_suction,
                                                      f(T_suction)));
                            ShowContinueErrorTimeStamp(state, "");
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       "Low load calculation Te solution not found as end points have the same sign",
                                                       this->LowLoadTeErrorIndex,
                                                       SolFla,
                                                       SolFla);
                        if (f(T_suction) < 0) {
                            // demand < capacity at both endpoints of the Te range, assuming f(x) is roughly monotonic than this is the low load case
                            // TeTol is added to prevent the final updated Te to go out of bounds
                            SmallLoadTe = MinOutdoorUnitTe + TeTol; // MinOutdoorUnitTe; //SmallLoadTe( Te'_new ) is constant during iterations
                        } else {
                            // demand > capacity at both endpoints of the Te range, take the end point x where f(x) is closer to zero
                            if (f(MinOutdoorUnitTe) > f(T_suction)) { // f(T_suction > 0, not equal as SolFla will not be -2
                                SmallLoadTe = T_suction;
                            } else {
                                SmallLoadTe = MinOutdoorUnitTe;
                            }
                        }
                    }

                    // Get an updated Te corresponding to the updated Te'
                    // VRFOU_TeModification( VRFCond, this->EvaporatingTemp, SmallLoadTe, Pipe_h_IU_in, OutdoorDryBulb, Pipe_Te_assumed,
                    // Pipe_Pe_assumed, Pipe_m_ref, Pipe_SH_merged );
                    {
                        // Initialization of Iteration_Te (Label11)
                        // i.e., find a new Te (Pipe_Te_assumed) that can generate a new T_suction equaling to SmallLoadTe.
                        // This requires the re-calculate of piping loss.
                        NumIteTe = 1;
                        MaxNumIteTe = (this->EvaporatingTemp - SmallLoadTe) / 0.1 + 1; // upper bound and lower bound of Te iterations
                        Pipe_Te_assumed = this->EvaporatingTemp - 0.1;
                        this->adjustedTe = true;

                        bool converged_11_2;
                        do {
                            Pipe_m_ref = 0; // Total Ref Flow Rate( kg/s )

                            // Re-calculate Piping loss due to the Te and SH updates
                            Pipe_h_IU_out = 0;
                            Pipe_h_IU_out_i = 0;
                            Pipe_m_ref_i = 0;
                            Pipe_SH_merged = 0;
                            Pipe_Pe_assumed = this->refrig->getSatPressure(state, Pipe_Te_assumed, RoutineName);

                            // Re-calculate total refrigerant flow rate, with updated SH
                            for (int NumTU = 1; NumTU <= NumTUInList; NumTU++) {
                                if (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) > 0) {
                                    TUIndex = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
                                    CoolCoilIndex = state.dataHVACVarRefFlow->VRFTU(TUIndex).CoolCoilIndex;

                                    Tfs = this->EvaporatingTemp + (this->C3Te * pow_2(state.dataDXCoils->DXCoil(CoolCoilIndex).ActualSH) +
                                                                   this->C2Te * state.dataDXCoils->DXCoil(CoolCoilIndex).ActualSH + this->C1Te);

                                    // Modifi_SH is the updated SH for a specific IU
                                    if (this->C3Te == 0) {
                                        Modifi_SHin = -(this->C1Te - Tfs + Pipe_Te_assumed) / this->C2Te; // 150130 Modifi_SH>Modifi_SHin
                                    } else {
                                        Modifi_SHin = (-this->C2Te +
                                                       std::pow((pow_2(this->C2Te) - 4 * (this->C1Te - Tfs + Pipe_Te_assumed) * this->C3Te), 0.5)) /
                                                      (2 * this->C3Te);
                                    }

                                    RefTSat = this->refrig->getSatTemperature(state, max(min(Pipe_Pe_assumed, RefPHigh), RefPLow), RoutineName);
                                    Pipe_h_IU_out_i = this->refrig->getSupHeatEnthalpy(state,
                                                                                       max(RefTSat, Pipe_Te_assumed + Modifi_SHin),
                                                                                       max(min(Pipe_Pe_assumed, RefPHigh), RefPLow),
                                                                                       RoutineName);

                                    if (Pipe_h_IU_out_i > Pipe_h_IU_in) {
                                        Real64 min_speed_capacity = this->CoffEvapCap * this->RatedEvapCapacity *
                                                                    CurveValue(state, this->OUCoolingCAPFT(1), T_discharge, T_suction);
                                        Pipe_m_ref_i = (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU) <= 0.0)
                                                           ? 0.0
                                                           // refrigerant flow rate do not get smaller when the compressor needs to cycle.
                                                           : max(min_speed_capacity,
                                                                 (state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).TotalCoolLoad(NumTU)) /
                                                                     (Pipe_h_IU_out_i - Pipe_h_IU_in));
                                        Pipe_m_ref = Pipe_m_ref + Pipe_m_ref_i;
                                        Pipe_SH_merged = Pipe_SH_merged + Pipe_m_ref_i * Modifi_SHin;
                                        Pipe_h_IU_out = Pipe_h_IU_out + Pipe_m_ref_i * Pipe_h_IU_out_i;
                                    }
                                }
                            }
                            if (Pipe_m_ref > 0) {
                                Pipe_h_IU_out = Pipe_h_IU_out / Pipe_m_ref;
                                Pipe_SH_merged = Pipe_SH_merged / Pipe_m_ref;
                            } else {
                                Pipe_SH_merged = this->SH;
                                RefTSat = this->refrig->getSatTemperature(state, max(min(Pipe_Pe_assumed, RefPHigh), RefPLow), RoutineName);
                                Pipe_h_IU_out = this->refrig->getSupHeatEnthalpy(
                                    state, max(RefTSat, Pipe_Te_assumed + Pipe_SH_merged), max(min(Pipe_Pe_assumed, RefPHigh), RefPLow), RoutineName);
                            }

                            // Re-calculate piping loss
                            this->VRFOU_PipeLossC(state,
                                                  Pipe_m_ref,
                                                  max(min(Pipe_Pe_assumed, RefPHigh), RefPLow),
                                                  Pipe_h_IU_out,
                                                  Pipe_SH_merged,
                                                  OutdoorDryBulb,
                                                  Pipe_Q,
                                                  Pipe_DeltP,
                                                  Pipe_h_comp_in);

                            T_suction =
                                this->refrig->getSatTemperature(state, max(min(Pipe_Pe_assumed - Pipe_DeltP, RefPHigh), RefPLow), RoutineName);

                            converged_11_2 = !((std::abs(T_suction - SmallLoadTe) > TeTol) && (Pipe_Te_assumed < this->EvaporatingTemp) &&
                                               (Pipe_Te_assumed > SmallLoadTe) && (NumIteTe < MaxNumIteTe));
                            Pipe_Te_assumed = Pipe_Te_assumed - 0.1;
                            NumIteTe = NumIteTe + 1;
                        } while (!converged_11_2);

                        if (std::abs(T_suction - SmallLoadTe) > TeTol) {
                            NumIteTe = 999;
                            T_suction = SmallLoadTe;
                            Pipe_SH_merged = 3.0;
                            Pipe_Te_assumed = SmallLoadTe + 1;
                        }
                        // Iteration_Te End
                    }

                    // Perform iteration to calculate Pipe_T_comp_in( Te'+SH' )
                    Pipe_T_comp_in = this->refrig->getSupHeatTemp(
                        state, max(min(Pipe_Pe_assumed - Pipe_DeltP, RefPHigh), RefPLow), Pipe_h_comp_in, T_suction + 3, T_suction + 30, RoutineName);

                    Modifi_SH = Pipe_T_comp_in - T_suction;
                    P_suction = Pipe_Pe_assumed - Pipe_DeltP;
                    OUCondHeatRelease = TU_load + Pipe_Q + Ncomp; // Pipe_Q is changed when T_suction is changed -> Tc is also changed

                    // *VRF OU Tc calculations
                    this->VRFOU_TeTc(state,
                                     HXOpMode::CondMode,
                                     OUCondHeatRelease,
                                     this->SC,
                                     this->OUAirFlowRate * RhoAir,
                                     OutdoorDryBulb,
                                     OutdoorHumRat,
                                     OutdoorPressure,
                                     Tfs,
                                     T_discharge);
                    T_discharge = min(MaxOutdoorUnitTc, T_discharge);

                    // *Calculate capacity modification factor
                    C_cap_operation = this->VRFOU_CapModFactor(state,
                                                               Pipe_h_comp_in,
                                                               Pipe_h_IU_in,
                                                               max(min(P_suction, RefPHigh), RefPLow),
                                                               T_suction + Modifi_SH,
                                                               T_suction + 8,
                                                               T_discharge - 5);

                    Cap_Eva0 = (TU_load + Pipe_Q) * C_cap_operation; // New Pipe_Q & C_cap_operation
                    Cap_Eva1 = this->CoffEvapCap * this->RatedEvapCapacity *
                               CurveValue(state, this->OUCoolingCAPFT(CounterCompSpdTemp), T_discharge, T_suction); // New Tc
                    CapDiff = std::abs(Cap_Eva1 - Cap_Eva0);
                    converged_13 = !((CapDiff > (Tolerance * Cap_Eva0)) && (NumIteCcap < 30));
                    NumIteCcap = NumIteCcap + 1;
                    Pipe_Q0 = Pipe_Q;
                    C_cap_operation0 = C_cap_operation;
                    T_discharge_new = T_discharge;
                } while (!converged_13);

                // when it gets here, either NumIteCcap = 30 or CapDiff > (Tolerance * Cap_Eva0)
                if (CapDiff > (Tolerance * Cap_Eva0) && (Cap_Eva1 - Cap_Eva0) >= 0.0) {
                    NumIteCcap = 999;
                    CyclingRatio = Cap_Eva0 / Cap_Eva1;
                } else {
                    CyclingRatio = 1.0;
                }

                Ncomp = this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(CounterCompSpdTemp), T_discharge, T_suction);
                OUCondHeatRelease = Ncomp + Cap_Eva1;

                this->CondensingTemp = T_discharge; // OU Tc' is updated due to OUCondHeatRelease updates, which is caused by IU Te' updates
                                                    // during low load conditions
                this->EvaporatingTemp = T_suction;
                this->IUEvaporatingTemp = T_suction;

                break; // EXIT DoName1

            } // End: if( CounterCompSpdTemp <= 1 ) Low load modification

        } // End: if( Q_evap_req <= CompEvaporatingCAPSpd( CounterCompSpdTemp ) )

    } // End: Iteration DoName1

    if (CounterCompSpdTemp > NumOfCompSpdInput) {
        // Required load is beyond the maximum system capacity
        CompEvaporatingCAPSpd(NumOfCompSpdInput) =
            this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), T_discharge, T_suction);
        OUCondHeatRelease = Ncomp + CompEvaporatingCAPSpd(NumOfCompSpdInput);
        CompSpdActual = this->CompressorSpeed(NumOfCompSpdInput);
        Ncomp = CompEvaporatingPWRSpd(NumOfCompSpdInput);
    }
}

void VRFCondenserEquipment::VRFOU_CalcCompH(
    EnergyPlusData &state,
    Real64 TU_load,            // Indoor unit heating load [W]
    Real64 T_suction,          // Compressor suction temperature Te' [C]
    Real64 T_discharge,        // Compressor discharge temperature Tc' [C]
    Real64 Pipe_h_out_ave,     // Average Enthalpy of the refrigerant leaving IUs [kJ/kg]
    Real64 IUMaxCondTemp,      // VRV IU condensing temperature, max among all indoor units [C]
    Real64 MinOutdoorUnitTe,   // The minimum temperature that OU Te can be at cooling mode (only used for calculating Min capacity)
    Real64 Tfs,                // Temperature of the air at the OU evaporator coil surface [C]]
    Real64 Pipe_Q,             // Piping Loss Algorithm Parameter: Heat loss [W]
    Real64 &OUEvapHeatExtract, // Condenser heat release (cooling mode) [W]
    Real64 &CompSpdActual,     // Actual compressor running speed [rps]
    Real64 &Ncomp,             // Compressor power [W]
    Real64 &CyclingRatio       // Compressor cycling ratio
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Xiufeng Pang
    //       DATE WRITTEN   Feb 2014
    //       MODIFIED       Rongpeng Zhang, Jan 2016
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine simulates the compressor performance at given proportional conditions (heating mode). More specifically, it specifies
    // the compressor speed to provide sufficient evaporative capacity, and calculate the power of the compressor running at the specified
    // speed. Note that it may be needed to manipulate the operational conditions to further adjust system capacity at low load conditions.
    // The low load modification logics are different for cooling mode and heating mode.

    // METHODOLOGY EMPLOYED:
    // This is part of the VRF-FluidTCtrl Model.

    using Curve::CurveValue;
    using General::SolveRoot;

    int CounterCompSpdTemp;                // Index for the compressor speed level[-]
    int CompSpdLB;                         // index for Compressor speed low bound [-]
    int CompSpdUB;                         // index for Compressor speed up bound [-]
    int MaxIter(500);                      // max iteration number allowed [-]
    int NumOfCompSpdInput;                 // Number of compressor speed input by the user [-]
    int NumIteCcap;                        // counter for Ccap calculation iterations [-]
    int NumTUInList;                       // number of terminal units is list
    int SolFla;                            // Solve flag for SolveRoot [-]
    int TUListNum;                         // index to TU List
    Real64 Cap_Eva0;                       // Evaporating capacity calculated based on physics model, used in the iterations [W]
    Real64 Cap_Eva1;                       // Evaporating capacity calculated by curves, used in the iterations [W]
    Real64 CapDiff;                        // Evaporating capacity difference used in the iterations [W]
    Real64 C_cap_operation;                // Compressor capacity modification algorithm_modified Cap [-]
    Real64 SmallLoadTe;                    // Updated suction temperature at small load conditions (Te') [C]
    Real64 Modifi_SH;                      // Temperature between compressor inlet temperature and evaporative temperature Te' [C]
    Real64 MinOutdoorUnitPe;               // The minimum pressure that Pe can be at cooling mode (only used for calculating Min capacity)
    Real64 Modifi_Pe;                      // Compressor power modification algorithm_modified Pe [Pa]
    Real64 Pipe_h_comp_in;                 // Piping Loss Algorithm Parameter: Enthalpy after piping loss (compressor inlet) [kJ/kg]
    Real64 Q_evap_req;                     // Required evaporative capacity [W]
    Real64 RefTSat;                        // Saturated temperature of the refrigerant [C]
    Real64 RefPLow;                        // Low Pressure Value for Ps (>0.0) [Pa]
    Real64 RefPHigh;                       // High Pressure Value for Ps (max in tables) [Pa]
    Real64 constexpr Tolerance(0.05);      // Tolerance for condensing temperature calculation [C}
    Array1D<Real64> CompEvaporatingPWRSpd; // Array for the compressor power at certain speed [W]
    Array1D<Real64> CompEvaporatingCAPSpd; // Array for the evaporating capacity at certain speed [W]

    static constexpr std::string_view RoutineName("VRFOU_CalcCompH");

    // variable initializations
    NumOfCompSpdInput = this->CompressorSpeed.size();
    CompEvaporatingPWRSpd.dimension(NumOfCompSpdInput);
    CompEvaporatingCAPSpd.dimension(NumOfCompSpdInput);
    Q_evap_req = TU_load + Pipe_Q - Ncomp;

    TUListNum = this->ZoneTUListPtr;
    RefPLow = this->refrig->PsLowPresValue;
    RefPHigh = this->refrig->PsHighPresValue;
    NumTUInList = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).NumTUInList;

    // Calculate capacity modification factor
    MinOutdoorUnitPe = this->refrig->getSatPressure(state, T_suction, RoutineName);
    RefTSat = this->refrig->getSatTemperature(state, max(min(MinOutdoorUnitPe, RefPHigh), RefPLow), RoutineName);
    Pipe_h_comp_in =
        this->refrig->getSupHeatEnthalpy(state, max(RefTSat, T_suction + this->SH), max(min(MinOutdoorUnitPe, RefPHigh), RefPLow), RoutineName);
    C_cap_operation = this->VRFOU_CapModFactor(
        state, Pipe_h_comp_in, Pipe_h_out_ave, max(min(MinOutdoorUnitPe, RefPHigh), RefPLow), T_suction + this->SH, T_suction + 8, IUMaxCondTemp - 5);

    // Perform iterations to find the compressor speed that can meet the required heating load, Iteration DoName2
    for (CounterCompSpdTemp = 1; CounterCompSpdTemp <= NumOfCompSpdInput; CounterCompSpdTemp++) {

        CompEvaporatingPWRSpd(CounterCompSpdTemp) =
            this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(CounterCompSpdTemp), T_discharge, T_suction);
        CompEvaporatingCAPSpd(CounterCompSpdTemp) =
            this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(CounterCompSpdTemp), T_discharge, T_suction);

        if ((Q_evap_req * C_cap_operation) <= CompEvaporatingCAPSpd(CounterCompSpdTemp)) {
            // Compressor Capacity is greater than the required, finish Iteration DoName2

            if (CounterCompSpdTemp > 1) {
                // Compressor runs at higher speed than min speed
                CompSpdLB = CounterCompSpdTemp - 1;
                CompSpdUB = CounterCompSpdTemp;

                CompSpdActual = this->CompressorSpeed(CompSpdLB) + (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB)) /
                                                                       (CompEvaporatingCAPSpd(CompSpdUB) - CompEvaporatingCAPSpd(CompSpdLB)) *
                                                                       (Q_evap_req * C_cap_operation - CompEvaporatingCAPSpd(CompSpdLB));
                Modifi_SH = this->SH;
                Ncomp = CompEvaporatingPWRSpd(CompSpdLB) + (CompEvaporatingPWRSpd(CompSpdUB) - CompEvaporatingPWRSpd(CompSpdLB)) /
                                                               (this->CompressorSpeed(CompSpdUB) - this->CompressorSpeed(CompSpdLB)) *
                                                               (CompSpdActual - this->CompressorSpeed(CompSpdLB));

                break; // EXIT DoName2

            } else {
                // Compressor runs at the min speed
                // Low Load Modifications

                NumIteCcap = 1;
                bool converged_19;
                // triggered in VariableRefrigerantFlow_FluidTCtrl_HR_5Zone.idf
                do {
                    Q_evap_req = max(0.0, TU_load + Pipe_Q - Ncomp);

                    // Update Te'( SmallLoadTe ) to meet the required evaporator capacity
                    CompSpdActual = this->CompressorSpeed(1);
                    Real64 CondHeat = Q_evap_req * C_cap_operation / this->RatedEvapCapacity;
                    int CAPFT = this->OUCoolingCAPFT(CounterCompSpdTemp);

                    auto f = [&state, T_discharge, CondHeat, CAPFT](Real64 const T_suc) {
                        return CompResidual_FluidTCtrl(state, T_discharge, CondHeat, CAPFT, T_suc);
                    };
                    General::SolveRoot(state, 1.0e-3, MaxIter, SolFla, SmallLoadTe, f, MinOutdoorUnitTe, T_suction);
                    if (SolFla < 0) {
                        SmallLoadTe = MinOutdoorUnitTe;
                    }

                    T_suction = SmallLoadTe;

                    // Update SH and Pe to calculate Modification Factor, which is used to update rps to for N_comp calculations
                    if (this->C3Te == 0) {
                        Modifi_SH = -(this->C1Te - Tfs + T_suction) / this->C2Te;
                    } else {
                        if ((pow_2(this->C2Te) - 4 * (this->C1Te - Tfs + T_suction) * this->C3Te) < 0.0) {
                            Modifi_SH = this->C2Te / (-2 * (this->C1Te - Tfs + T_suction));
                        } else {
                            Modifi_SH = (-this->C2Te + std::pow((pow_2(this->C2Te) - 4 * (this->C1Te - Tfs + T_suction) * this->C3Te), 0.5)) /
                                        (2 * this->C3Te);
                        }
                    }

                    Modifi_Pe = this->refrig->getSatPressure(state, T_suction, RoutineName);

                    // Calculate capacity modification factor
                    RefTSat = this->refrig->getSatTemperature(state, max(min(Modifi_Pe, RefPHigh), RefPLow), RoutineName);
                    Pipe_h_comp_in = this->refrig->getSupHeatEnthalpy(
                        state, max(RefTSat, T_suction + Modifi_SH), max(min(Modifi_Pe, RefPHigh), RefPLow), RoutineName);
                    C_cap_operation = this->VRFOU_CapModFactor(state,
                                                               Pipe_h_comp_in,
                                                               Pipe_h_out_ave,
                                                               max(min(Modifi_Pe, RefPHigh), RefPLow),
                                                               T_suction + Modifi_SH,
                                                               T_suction + 8,
                                                               IUMaxCondTemp - 5);

                    Cap_Eva0 = Q_evap_req * C_cap_operation;
                    Cap_Eva1 = this->CoffEvapCap * this->RatedEvapCapacity *
                               CurveValue(state, this->OUCoolingCAPFT(CounterCompSpdTemp), T_discharge, T_suction);
                    CapDiff = std::abs(Cap_Eva1 - Cap_Eva0);
                    converged_19 = (CapDiff <= (Tolerance * Cap_Eva0)) || (NumIteCcap >= 30);
                    NumIteCcap = NumIteCcap + 1;
                } while (!converged_19);
                if (CapDiff > (Tolerance * Cap_Eva0) && (Cap_Eva1 - Cap_Eva0) >= 0.0) {
                    NumIteCcap = 999;
                    CyclingRatio = Cap_Eva0 / Cap_Eva1;
                } else {
                    CyclingRatio = 1.0;
                }

                Ncomp = this->RatedCompPower * CurveValue(state, this->OUCoolingPWRFT(CounterCompSpdTemp), T_discharge, T_suction) * CyclingRatio;
                // Cap_Eva1 is the updated compressor min speed capacity
                OUEvapHeatExtract = Cap_Eva1;
                this->EvaporatingTemp = T_suction;
                this->CondensingTemp = T_discharge;
                this->IUCondensingTemp = T_discharge;
                break; // EXIT DoName2

            } // End: if( CounterCompSpdTemp <= 1 ) Low load modification

        } // End: if( Q_evap_req <= CompEvaporatingCAPSpd( CounterCompSpdTemp ) )

    } // End: Iteration DoName2

    if (CounterCompSpdTemp > NumOfCompSpdInput) {
        // Required heating load is beyond the maximum system capacity
        CompEvaporatingCAPSpd(NumOfCompSpdInput) =
            this->CoffEvapCap * this->RatedEvapCapacity * CurveValue(state, this->OUCoolingCAPFT(NumOfCompSpdInput), T_discharge, T_suction);
        OUEvapHeatExtract = CompEvaporatingCAPSpd(NumOfCompSpdInput);
        CompSpdActual = this->CompressorSpeed(NumOfCompSpdInput);
        Ncomp = CompEvaporatingPWRSpd(NumOfCompSpdInput);
    }
}

void VRFCondenserEquipment::VRFHR_OU_HR_Mode(EnergyPlusData &state,
                                             Real64 const h_IU_evap_in, // enthalpy of IU evaporator at inlet [kJ/kg]
                                             Real64 const h_comp_out,   // enthalpy of refrigerant at compressor outlet [kJ/kg]
                                             Real64 const Q_c_TU_PL,    // IU evaporator load, including piping loss [W]
                                             Real64 const Q_h_TU_PL,    // IU condenser load, including piping loss [W]
                                             Real64 const Tdischarge,   // VRF Compressor discharge refrigerant temperature [C]
                                             Real64 &Tsuction,          // VRF compressor suction refrigerant temperature [C]
                                             Real64 &Te_update,         // updated evaporating temperature, only updated when Tsuction is updated [C]
                                             Real64 &h_comp_in,         // enthalpy of refrigerant at compressor inlet [kJ/kg]
                                             Real64 &h_IU_PLc_out,  // enthalpy of refrigerant at the outlet of IU evaporator side main pipe [kJ/kg]
                                             Real64 &Pipe_Q_c,      // IU evaporator side piping loss [W]
                                             Real64 &Q_c_OU,        // OU evaporator load [W]
                                             Real64 &Q_h_OU,        // OU condenser load [W]
                                             Real64 &m_ref_IU_evap, // mass flow rate of Refrigerant through IU evaporators [kg/s]
                                             Real64 &m_ref_OU_evap, // mass flow rate of Refrigerant through OU evaporator [kg/s]
                                             Real64 &m_ref_OU_cond, // mass flow rate of Refrigerant through OU condenser [kg/s]
                                             Real64 &N_fan_OU,      // outdoor unit fan power [W]
                                             Real64 &CompSpdActual, // Actual compressor running speed [rps]
                                             Real64 &Ncomp          // compressor power [W]
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rongpeng Zhang, LBNL
    //       DATE WRITTEN   Jan 2016
    //       MODIFIED       na
    //
    //       RE-ENGINEERED  na
    //
    // PURPOSE OF THIS SUBROUTINE:
    //        Determine the operational mode of the VRF-HR system, given the terminal unit side load conditions.
    //        Compressor and OU hex performance are analyzed for each mode.
    //        A number of OU side operational parameters are also calculated here, including:
    //        (1) OU evaporator load Q_c_OU (2) OU condenser load Q_h_OU (3) OU fan energy consumption
    //        (4) OU compressor speed and energy consumption
    //        Note that Te and Te' may be updated here, and thus IU evaporator side piping loss recalculations.
    //        Then a number of operational parameters need to be updated, including:
    //        (1) IU evaporating temperature Te (2) OU evaporating temperature Te' etc.
    //
    // METHODOLOGY EMPLOYED:
    //        This is part of the physics based VRF model applicable for Fluid Temperature Control.

    using General::SolveRoot;

    Real64 constexpr ErrorTol(0.1); // tolerance for RegulaFalsi iterations
    int constexpr MaxIte(100);      // maximum number of iterations
    int HRMode(0);                  // HR operational mode [W]
    int HRMode_sub(0);              // HR operational mode (sub) [W]
    int SolFla;                     // Flag of RegulaFalsi solver
    Real64 C_OU_HexRatio;           // capacity ratio between the OU condenser and OU evaporator [-]
    Real64 m_air_rated;             // OU coil air mass flow rate [kg/s]
    Real64 m_air_evap;              // OU evaporator air mass flow rate [kg/s]
    Real64 m_air_cond;              // OU condenser air mass flow rate [kg/s]
    Real64 m_air_evap_rated;        // Rated OU evaporator air mass flow rate [kg/s]
    Real64 N_fan_OU_evap(0);        // OU evaporator air mass flow rate [kg/s]
    Real64 N_fan_OU_cond(0);        // OU condenser air mass flow rate [kg/s]
    Real64 RhoAir;                  // outdoor air density [kg/m3]
    Real64 Q_c_tot;                 // Total evaporator capacity [W]
    Real64 Q_h_tot;                 // Total condenser capacity [W]
    Real64 Pipe_Q_c_new;            // IU evaporator side piping loss (new), updated because of Te update [W]
    Real64 rps1_evap;               // compressor speed satisfying IU cooling load
    Real64 rps2_cond;               // compressor speed satisfying IU heating load
    Real64 RefPLow;                 // Low Pressure Value for Ps (>0.0) [Pa]
    Real64 RefPHigh;                // High Pressure Value for Ps (max in tables) [Pa]
    Real64 Tfs;                     // temperature of the air at coil surface [C]
    Real64 Tolerance(0.05);         // Tolerance for condensing temperature calculation [C}
    Real64 Tsuction_new;            // VRF compressor suction refrigerant temperature (new) [C]

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    static constexpr std::string_view RoutineName("VRFHR_OU_Mode");

    // Initialization: operational parameters
    RhoAir = PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, state.dataEnvrn->OutDryBulbTemp, state.dataEnvrn->OutHumRat);
    m_air_rated = this->OUAirFlowRate * RhoAir;
    C_OU_HexRatio = this->HROUHexRatio;

    // Initializations: component index
    RefPLow = this->refrig->PsLowPresValue;
    RefPHigh = this->refrig->PsHighPresValue;

    // **Q_OU: HR mode determination
    //     HRMode-1. Cooling Only
    //     HRMode-2. Cooling Dominant w/o HR Loss
    //     HRMode-3. Cooling Dominant w/ HR Loss
    //     HRMode-4. Heating Dominant w/ HR Loss
    //     HRMode-5. Heating Dominant w/o HR Loss
    //     HRMode-6. Heating Only
    //     HRMode-7. OU Hex not running
    {

        bool FlagMode5;   // true if compressor speed satisfying IU cooling load < that satisfying IU heating load
        bool FlagToLower; // true if To-5 is lower than the Tsuction determined by IU part
        Real64 temp_Tsuction;

        // Determine FlagToLower
        if (state.dataEnvrn->OutDryBulbTemp - this->DiffOUTeTo < Tsuction) {
            temp_Tsuction = state.dataEnvrn->OutDryBulbTemp - this->DiffOUTeTo;
            FlagToLower = true;
        } else {
            temp_Tsuction = Tsuction;
            FlagToLower = false;
        }

        // Calculate compressor speed satisfying IU loads: rps1_evap & rps2_cond
        this->VRFOU_CompSpd(state, Q_c_TU_PL, HXOpMode::EvapMode, temp_Tsuction, Tdischarge, h_IU_evap_in, h_IU_PLc_out, rps1_evap);
        this->VRFOU_CompSpd(state, Q_h_TU_PL, HXOpMode::CondMode, temp_Tsuction, Tdischarge, h_IU_evap_in, h_IU_PLc_out, rps2_cond);

        // Determine FlagMode5
        if (rps1_evap <= rps2_cond) {
            FlagMode5 = true;
        } else {
            FlagMode5 = false;
        }

        // Determine HR Mode
        if (FlagMode5) {
            HRMode = 5;
            if (FlagToLower) {
                HRMode_sub = 1;
            } else {
                HRMode_sub = 2;
            }
        } else {

            if (FlagToLower) {
                HRMode = 3; // Mode 3&4 share the same logics below
            } else {
                HRMode = 2;
            }
        }

        this->VRFOperationSimPath = HRMode * 10 + HRMode_sub;
    }

    // **Simulate outdoor unit and compressor performance, including
    // (1) compressor spd/power (2) OU hex capacity (3) OU fan flow rate and power
    // Tsuction/Te may also need updates
    if (HRMode == 5 && HRMode_sub == 2) {

        CompSpdActual = rps2_cond; // constant in this mode
        // Tsuction = Te'_iu < OutDryBulbTemp - 5; constant in this mode

        // compressor: Ncomp & Q_c_tot
        this->VRFOU_CompCap(state, CompSpdActual, Tsuction, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_tot, Ncomp);

        // OU hex capacity
        Q_c_OU = Q_c_tot - Q_c_TU_PL;
        Q_h_OU = 0;

        // OU fan flow rate and power
        m_air_evap =
            this->VRFOU_FlowRate(state, HXOpMode::EvapMode, Tsuction, this->SH, Q_c_OU, state.dataEnvrn->OutDryBulbTemp, state.dataEnvrn->OutHumRat);
        m_air_evap_rated = m_air_rated;
        N_fan_OU_evap = this->RatedOUFanPower * m_air_evap / m_air_evap_rated;
        N_fan_OU_cond = 0;

    } else if (HRMode == 5 && HRMode_sub == 1) {

        // local parameters
        int Counter_Iter_Ncomp;
        bool Flag_Iter_Ncomp(true); // Flag to perform iterations
        Real64 Ncomp_ini;
        Real64 Ncomp_new;
        Real64 Q_c_tot_temp;
        Real64 Q_c_OU_temp;

        //===**Ncomp Iterations

        // initialization: Ncomp_ini, CompSpdActual
        Counter_Iter_Ncomp = 1;
        CompSpdActual = rps2_cond;
        Tsuction_new = state.dataEnvrn->OutDryBulbTemp - this->DiffOUTeTo;
        Pipe_Q_c_new = Pipe_Q_c;

        this->VRFOU_CompCap(state, CompSpdActual, Tsuction_new, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_tot, Ncomp_ini);

        while (Flag_Iter_Ncomp) {

            Q_c_tot_temp = Q_h_TU_PL - Ncomp_ini; // Q_h_OU = 0
            Q_c_OU_temp = Q_c_tot_temp - Q_c_TU_PL;

            // Tsuction_new updated based on OU evaporator air-side calculations (Tsuction_new < To)
            m_air_evap_rated = m_air_rated;
            this->VRFOU_TeTc(state,
                             HXOpMode::EvapMode,
                             Q_c_OU_temp,
                             this->SH,
                             m_air_evap_rated,
                             state.dataEnvrn->OutDryBulbTemp,
                             state.dataEnvrn->OutHumRat,
                             state.dataEnvrn->OutBaroPress,
                             Tfs,
                             Tsuction_new);
            Tsuction_new = min(Tsuction_new, Tsuction); // should be lower than Tsuction_IU

            // Calculate updated rps corresponding to updated Tsuction_new and Q_c_tot_temp
            this->VRFOU_CompSpd(state, Q_c_tot_temp, HXOpMode::EvapMode, Tsuction_new, Tdischarge, h_IU_evap_in, h_comp_in, CompSpdActual);

            // Calculate Ncomp_new, using updated CompSpdActual and Tsuction_new
            this->VRFOU_CompCap(state, CompSpdActual, Tsuction_new, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_tot_temp, Ncomp_new);

            if ((std::abs(Ncomp_new - Ncomp_ini) > (Tolerance * Ncomp_ini)) && (Counter_Iter_Ncomp < 30)) {
                Ncomp_ini = 0.5 * Ncomp_ini + 0.5 * Ncomp_new;
                Counter_Iter_Ncomp = Counter_Iter_Ncomp + 1;
                continue;
            }

            Flag_Iter_Ncomp = false;
        }

        // Ncomp Iterations Update
        Ncomp = Ncomp_new;
        Q_c_tot = Q_c_tot_temp;

        if (Tsuction_new < Tsuction) {
            // Need to update the Tsuction, and thus update Te_update & Pipe_Q_c_new.
            // Iteration continues.

            // temporary parameters
            Real64 Pe_update;
            Real64 Pipe_SH_merged;
            Real64 Pipe_DeltP;
            Real64 Pipe_h_IU_out;

            // Get an updated Te (Te_update) corresponding to the updated Te' (Tsuction_new). PL_c is re-performed.
            this->VRFOU_TeModification(state,
                                       this->EvaporatingTemp,
                                       Tsuction_new,
                                       h_IU_evap_in,
                                       state.dataEnvrn->OutDryBulbTemp,
                                       Te_update,
                                       Pe_update,
                                       m_ref_IU_evap,
                                       Pipe_h_IU_out,
                                       Pipe_SH_merged);

            // Re-calculate piping loss, update Pipe_Q_c_new
            this->VRFOU_PipeLossC(state,
                                  m_ref_IU_evap,
                                  Pe_update,
                                  Pipe_h_IU_out,
                                  Pipe_SH_merged,
                                  state.dataEnvrn->OutDryBulbTemp,
                                  Pipe_Q_c_new,
                                  Pipe_DeltP,
                                  h_IU_PLc_out);

            Tsuction = Tsuction_new;
            Pipe_Q_c = Pipe_Q_c_new;
        }

        // No need to update the Tsuction.

        //===**Ncomp Iteration Ends (Label200)

        // OU hex capacity
        Q_c_OU = Q_c_tot - Q_c_TU_PL;
        Q_h_OU = 0;

        // OU fan power
        N_fan_OU_evap = this->RatedOUFanPower;
        N_fan_OU_cond = 0;

    } else if (HRMode == 3) { // Mode3 & Mode4 share the same algorithm

        // local parameters
        Real64 Ncomp_new;
        Real64 Q_c_tot_temp;
        Real64 Q_c_OU_temp;
        Real64 Tsuction_LB = state.dataEnvrn->OutDryBulbTemp - this->DiffOUTeTo;
        Real64 Tsuction_HB = Tsuction;

        // compressor speed is fixed in this mode
        CompSpdActual = rps1_evap; // constant in this mode
        m_air_evap_rated = m_air_rated * (1 - C_OU_HexRatio);
        m_air_evap = m_air_evap_rated; // may be updated

        // perform iterations to calculate Te at the given compressor speed and operational conditions
        {

            auto f = [&state, this, CompSpdActual, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_TU_PL, m_air_evap_rated](Real64 const Te) {
                int VRFCond = state.dataHVACVarRefFlow->VRFTU(state.dataHVACVarRefFlow->TerminalUnitList(this->ZoneTUListPtr).ZoneTUPtr(1))
                                  .VRFSysNum; // VRFCond;

                Real64 Ncomp_temp;   // compressor power [W]
                Real64 Q_c_tot_temp; // total evaporator load, including piping loss [W]
                Real64 Q_c_OU_temp;  // OU evaporator load, including piping loss [W]
                Real64 Te_new;       // newly calculated OU evaporating temperature
                Real64 Tfs;          // OU evaporator coil surface temperature [C]

                state.dataHVACVarRefFlow->VRF(VRFCond).VRFOU_CompCap(
                    state, CompSpdActual, Te, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_tot_temp, Ncomp_temp);
                Q_c_OU_temp = Q_c_tot_temp - Q_c_TU_PL;

                // Tsuction_new calculated based on OU evaporator air-side calculations (Tsuction_new < To)
                state.dataHVACVarRefFlow->VRF(VRFCond).VRFOU_TeTc(state,
                                                                  HXOpMode::EvapMode,
                                                                  Q_c_OU_temp,
                                                                  state.dataHVACVarRefFlow->VRF(VRFCond).SH,
                                                                  m_air_evap_rated,
                                                                  state.dataEnvrn->OutDryBulbTemp,
                                                                  state.dataEnvrn->OutHumRat,
                                                                  state.dataEnvrn->OutBaroPress,
                                                                  Tfs,
                                                                  Te_new);

                return Te_new - Te;
            };

            General::SolveRoot(state, ErrorTol, MaxIte, SolFla, Tsuction_new, f, Tsuction_LB, Tsuction_HB);
            if (SolFla < 0) {
                Tsuction_new = Tsuction_LB;
            }

            // Update Q_c_tot_temp using updated Tsuction_new
            this->VRFOU_CompCap(state, CompSpdActual, Tsuction_new, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_tot_temp, Ncomp_new);
            Q_c_OU_temp = Q_c_tot_temp - Q_c_TU_PL;

            // Iterations_Te Update
            Ncomp = Ncomp_new;
            Tsuction = Tsuction_new;
            Q_c_tot = Q_c_tot_temp;
            Q_c_OU = Q_c_OU_temp;
        }

        if (Tsuction >= Tsuction_HB) {
            // modify m_air_evap to adjust OU evaporator capacity;
            // update Ncomp, Q_c_OU, m_air_evap

            Tsuction = Tsuction_HB;

            // Q_c_tot
            this->VRFOU_CompCap(state, CompSpdActual, Tsuction_new, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_tot, Ncomp);
            Q_c_OU = Q_c_tot - Q_c_TU_PL;

            // OU evaporator fan flow rate and power
            m_air_evap = this->VRFOU_FlowRate(
                state, HXOpMode::EvapMode, Tsuction, this->SH, Q_c_OU_temp, state.dataEnvrn->OutDryBulbTemp, state.dataEnvrn->OutHumRat);

        } else {
            // Need to update Te_update & Pipe_Q_c_new, corresponding to Tsuction update.

            // temporary parameters
            Real64 Pe_update;
            Real64 Pipe_SH_merged;
            Real64 Pipe_DeltP;
            Real64 Pipe_h_IU_out;

            // Get an updated Te (Te_update) corresponding to the updated Te' (Tsuction_new). PL_c is re-performed.
            this->VRFOU_TeModification(state,
                                       this->EvaporatingTemp,
                                       Tsuction_new,
                                       h_IU_evap_in,
                                       state.dataEnvrn->OutDryBulbTemp,
                                       Te_update,
                                       Pe_update,
                                       m_ref_IU_evap,
                                       Pipe_h_IU_out,
                                       Pipe_SH_merged);

            // Re-calculate piping loss, update Pipe_Q_c_new
            this->VRFOU_PipeLossC(state,
                                  m_ref_IU_evap,
                                  Pe_update,
                                  Pipe_h_IU_out,
                                  Pipe_SH_merged,
                                  state.dataEnvrn->OutDryBulbTemp,
                                  Pipe_Q_c_new,
                                  Pipe_DeltP,
                                  h_IU_PLc_out);
            Pipe_Q_c = Pipe_Q_c_new;
        }

        // Q_h_ou
        Q_h_tot = Q_c_tot + Ncomp;
        Q_h_OU = Q_h_tot - Q_h_TU_PL;

        // OU condenser fan flow rate and power
        m_air_cond = this->VRFOU_FlowRate(
            state, HXOpMode::CondMode, Tdischarge, this->SC, Q_h_OU, state.dataEnvrn->OutDryBulbTemp, state.dataEnvrn->OutHumRat);

        // OU fan power
        N_fan_OU_evap = this->RatedOUFanPower * m_air_evap / m_air_rated;
        N_fan_OU_cond = this->RatedOUFanPower * m_air_cond / m_air_rated;

    } else if (HRMode == 2) {

        CompSpdActual = rps1_evap; // constant in this mode
        // Tsuction = Te'_iu < OutDryBulbTemp - 5; constant in this mode

        // compressor: Ncomp & Q_c_tot
        this->VRFOU_CompCap(state, CompSpdActual, Tsuction, Tdischarge, h_IU_evap_in, h_comp_in, Q_c_tot, Ncomp);

        // OU hex capacity
        Q_h_tot = Q_c_tot + Ncomp;
        Q_h_OU = Q_h_tot - Q_h_TU_PL;
        Q_c_OU = 0;

        // OU fan flow rate and power
        m_air_cond = this->VRFOU_FlowRate(
            state, HXOpMode::CondMode, Tdischarge, this->SC, Q_h_OU, state.dataEnvrn->OutDryBulbTemp, state.dataEnvrn->OutHumRat);
        N_fan_OU_cond = this->RatedOUFanPower * m_air_cond / m_air_rated;
        N_fan_OU_evap = 0;

    } else {
        Ncomp = 0;
        CompSpdActual = 0;
        Q_c_OU = 0;
        Q_h_OU = 0;
        N_fan_OU_evap = 0;
        N_fan_OU_cond = 0;
    }

    // OU fan power
    N_fan_OU = N_fan_OU_evap + N_fan_OU_cond;

    // Calculate the m_ref_OU_evap & m_ref_OU_cond, with updated Tsuction
    {
        Real64 h_OU_evap_in;  // enthalpy of OU evaporator at inlet [kJ/kg]
        Real64 h_OU_evap_out; // enthalpy of OU evaporator at outlet [kJ/kg]
        Real64 h_OU_cond_in;  // enthalpy of OU condenser at inlet [kJ/kg]
        Real64 h_OU_cond_out; // enthalpy of OU condenser at outlet [kJ/kg]

        Real64 Psuction = this->refrig->getSatPressure(state, Tsuction, RoutineName);

        // enthalpy of OU evaporator/condenser inlets and outlets
        h_OU_evap_in = h_IU_evap_in;
        h_OU_cond_in = h_comp_out;
        h_OU_evap_out = this->refrig->getSupHeatEnthalpy(state, Tsuction + this->SH, max(min(Psuction, RefPHigh), RefPLow), RoutineName);
        h_OU_cond_out = this->refrig->getSatEnthalpy(state, Tdischarge - this->SC, 0.0, RoutineName);

        if ((Q_c_OU == 0) || (h_OU_evap_out - h_OU_evap_in) <= 0) {
            m_ref_OU_evap = 0;
        } else {
            m_ref_OU_evap = Q_c_OU / (h_OU_evap_out - h_OU_evap_in);
        }

        if ((Q_h_OU == 0) || (h_OU_cond_in - h_OU_cond_out <= 0)) {
            m_ref_OU_cond = 0;
        } else {
            m_ref_OU_cond = Q_h_OU / (h_OU_cond_in - h_OU_cond_out);
        }

        // Calculate the parameters of refrigerant at compressor inlet, which is
        // a combination of refrigerant from IU evaporators and OU evaporator
        if ((m_ref_OU_evap + m_ref_IU_evap) > 0) {
            h_comp_in = (m_ref_OU_evap * h_OU_evap_out + m_ref_IU_evap * h_IU_PLc_out) / (m_ref_OU_evap + m_ref_IU_evap);
        }
    }
}

void VRFCondenserEquipment::VRFOU_PipeLossC(
    EnergyPlusData &state,
    Real64 const Pipe_m_ref,     // Refrigerant mass flow rate [kg/s]
    Real64 const Pevap,          // VRF evaporating pressure [Pa]
    Real64 const Pipe_h_IU_out,  // Enthalpy of IU at outlet [kJ/kg]
    Real64 const Pipe_SH_merged, // Average super heating degrees after the indoor units [C]
    Real64 const OutdoorDryBulb, // outdoor dry-bulb temperature (C)
    Real64 &Pipe_Q,              // unit part load ratio
    Real64 &Pipe_DeltP,          // ratio of compressor ON airflow to AVERAGE airflow over timestep
    Real64 &Pipe_h_comp_in       // Piping Loss Algorithm Parameter: Enthalpy after piping loss (compressor inlet) [kJ/kg]
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rongpeng Zhang
    //       DATE WRITTEN   Nov 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Determine the piping loss of the refrigerant, including both the heat loss and pressure drop.
    // This happens at VRF cooling mode, within the Main Pipe connecting Outdoor Unit to Indoor Units.

    // METHODOLOGY EMPLOYED:
    // Use a physics based piping loss model.

    using General::SolveRoot;

    int TUListNum;      // index to TU List
    int NumTUInList;    // number of terminal units is list
    int NumIUActivated; // number of the used indoor units [-]

    Real64 Pipe_v_ref;            // Piping Loss Algorithm Parameter: Refrigerant velocity [m/s]
    Real64 Pipe_T_room;           // Piping Loss Algorithm Parameter: Average Room Temperature [C]
    Real64 Pipe_Num_Re;           // Piping Loss Algorithm Parameter: refrigerant Re Number [-]
    Real64 Pipe_Num_Pr;           // Piping Loss Algorithm Parameter: refrigerant Pr Number [-]
    Real64 Pipe_Num_Nu;           // Piping Loss Algorithm Parameter: refrigerant Nu Number [-]
    Real64 Pipe_Num_St;           // Piping Loss Algorithm Parameter: refrigerant St Number [-]
    Real64 Pipe_Coe_k1;           // Piping Loss Algorithm Parameter: coefficients [-]
    Real64 Pipe_Coe_k2;           // Piping Loss Algorithm Parameter: coefficients [-]
    Real64 Pipe_Coe_k3;           // Piping Loss Algorithm Parameter: coefficients [-]
    Real64 Pipe_cp_ref;           // Piping Loss Algorithm_[kJ/kg/K]
    Real64 Pipe_conductivity_ref; // Piping Loss Algorithm: refrigerant conductivity [W/m/K]
    Real64 Pipe_viscosity_ref;    // Piping Loss Algorithm Parameter: refrigerant viscosity [MuPa*s]
    Real64 Ref_Coe_v1;            // Piping Loss Algorithm Parameter: coefficient to calculate Pipe_viscosity_ref [-]
    Real64 Ref_Coe_v2;            // Piping Loss Algorithm Parameter: coefficient to calculate Pipe_viscosity_ref [-]
    Real64 Ref_Coe_v3;            // Piping Loss Algorithm Parameter: coefficient to calculate Pipe_viscosity_ref [-]
    Real64 RefPipInsH;            // Heat transfer coefficient for calculating piping loss [W/m2K]

    static constexpr std::string_view RoutineName("VRFOU_PipeLossC");

    TUListNum = this->ZoneTUListPtr;
    NumTUInList = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).NumTUInList;
    Pipe_conductivity_ref = this->RefPipInsCon;

    RefPipInsH = 9.3;
    Pipe_cp_ref = 1.6;

    // Refrigerant data
    Real64 RefPLow = this->refrig->PsLowPresValue;   // Low Pressure Value for Ps (>0.0)
    Real64 RefPHigh = this->refrig->PsHighPresValue; // High Pressure Value for Ps (max in tables)

    // Calculate Pipe_T_room
    Pipe_T_room = 0;
    NumIUActivated = 0;
    for (int NumTU = 1; NumTU <= NumTUInList; ++NumTU) {
        int TUIndex = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
        int CoilIndex = state.dataHVACVarRefFlow->VRFTU(TUIndex).CoolCoilIndex;

        if (state.dataDXCoils->DXCoil(CoilIndex).TotalCoolingEnergyRate > 0.0) {
            Pipe_T_room = Pipe_T_room + state.dataDXCoils->DXCoil(CoilIndex).InletAirTemp;
            NumIUActivated = NumIUActivated + 1;
        }
    }
    if (NumIUActivated > 0) {
        Pipe_T_room = Pipe_T_room / NumIUActivated;
    } else {
        Pipe_T_room = 24;
    }

    if (Pipe_m_ref > 0) {
        if (this->RefPipDiaSuc <= 0) {
            this->RefPipDiaSuc = 0.025;
        }

        Ref_Coe_v1 = Pevap / 1000000 / 4.926;
        Ref_Coe_v2 = Pipe_h_IU_out / 383.5510343;
        Ref_Coe_v3 = (this->EvaporatingTemp + Pipe_SH_merged + 273.15) / 344.39;

        Pipe_viscosity_ref = 4.302 * Ref_Coe_v1 + 0.81622 * pow_2(Ref_Coe_v1) - 120.98 * Ref_Coe_v2 + 139.17 * pow_2(Ref_Coe_v2) +
                             118.76 * Ref_Coe_v3 + 81.04 * pow_2(Ref_Coe_v3) + 5.7858 * Ref_Coe_v1 * Ref_Coe_v2 - 8.3817 * Ref_Coe_v1 * Ref_Coe_v3 -
                             218.48 * Ref_Coe_v2 * Ref_Coe_v3 + 21.58;
        if (Pipe_viscosity_ref <= 0) {
            Pipe_viscosity_ref = 16.26; // default superheated vapor viscosity data (MuPa*s) at T=353.15 K, P=2MPa
        }

        Pipe_v_ref = Pipe_m_ref / (Constant::Pi * pow_2(this->RefPipDiaSuc) * 0.25) /
                     this->refrig->getSupHeatDensity(state, this->EvaporatingTemp + Pipe_SH_merged, max(min(Pevap, RefPHigh), RefPLow), RoutineName);
        Pipe_Num_Re = Pipe_m_ref / (Constant::Pi * pow_2(this->RefPipDiaSuc) * 0.25) * this->RefPipDiaSuc / Pipe_viscosity_ref * 1000000;
        Pipe_Num_Pr = Pipe_viscosity_ref * Pipe_cp_ref * 0.001 / Pipe_conductivity_ref;
        Pipe_Num_Nu = 0.023 * std::pow(Pipe_Num_Re, 0.8) * std::pow(Pipe_Num_Pr, 0.3);
        Pipe_Num_St = Pipe_Num_Nu / Pipe_Num_Re / Pipe_Num_Pr;

        Pipe_DeltP = max(
            0.0,
            8 * Pipe_Num_St * std::pow(Pipe_Num_Pr, 0.6667) * this->RefPipEquLen / this->RefPipDiaSuc *
                    this->refrig->getSupHeatDensity(state, this->EvaporatingTemp + Pipe_SH_merged, max(min(Pevap, RefPHigh), RefPLow), RoutineName) *
                    pow_2(Pipe_v_ref) / 2 -
                this->RefPipHei *
                    this->refrig->getSupHeatDensity(state, this->EvaporatingTemp + Pipe_SH_merged, max(min(Pevap, RefPHigh), RefPLow), RoutineName) *
                    9.80665);

        Pipe_Coe_k1 = Pipe_Num_Nu * Pipe_viscosity_ref;
        Pipe_Coe_k3 = RefPipInsH * (this->RefPipDiaSuc + 2 * this->RefPipInsThi);
        if (this->RefPipInsThi >= 0.0) {
            Pipe_Coe_k2 = 2 * this->RefPipInsCon / std::log1p(2 * this->RefPipInsThi / this->RefPipDiaSuc);
        } else {
            Pipe_Coe_k2 = 9999.9;
        }

        Pipe_Q = max(0.0,
                     (Constant::Pi * this->RefPipLen) * (OutdoorDryBulb / 2 + Pipe_T_room / 2 - this->EvaporatingTemp - Pipe_SH_merged) /
                         (1 / Pipe_Coe_k1 + 1 / Pipe_Coe_k2 + 1 / Pipe_Coe_k3));

        Pipe_h_comp_in = Pipe_h_IU_out + Pipe_Q / Pipe_m_ref;
        // when Pipe_m_ref is close to 0, there will still be Pipe_Q, Pipe_Q / Pipe_m_ref will blow up
        // At low flow rate (stationary flow)
        // Pipe_h_comp_in = Pipe_h_IU_out + Cp * deltaT
        // where deltaT is the refrigerant temperature change over time t,
        // deltaT = Pipe_Q * t / (refrigerant_mass_in_pipe * Cp)
        // so Pipe_h_comp_in = Pipe_h_IU_out + Cp * Pipe_Q / refrigerant_mass_in_pipe
        // assume 0.001 is the cutoff Re for stationary flow
        if (Pipe_Num_Re < 0.001) { // low flow rate calculation
            Pipe_h_comp_in =
                Pipe_h_IU_out + Pipe_Q * state.dataHVACGlobal->TimeStepSysSec /
                                    ((Constant::Pi * std::pow(this->RefPipDiaSuc / 2, 2) * this->RefPipLen *
                                      this->refrig->getSupHeatDensity(
                                          state, this->EvaporatingTemp + Pipe_SH_merged, max(min(Pevap, RefPHigh), RefPLow), RoutineName)));
        }

    } else {
        Pipe_DeltP = 0;
        Pipe_Q = 0;
        Pipe_h_comp_in = Pipe_h_IU_out;
    }
}

void VRFCondenserEquipment::VRFOU_PipeLossH(
    EnergyPlusData &state,
    Real64 const Pipe_m_ref,     // Refrigerant mass flow rate [kg/s]
    Real64 const Pcond,          // VRF condensing pressure [Pa]
    Real64 const Pipe_h_IU_in,   // Enthalpy of IU at outlet [kJ/kg]
    Real64 const OutdoorDryBulb, // outdoor dry-bulb temperature (C)
    Real64 &Pipe_Q,              // unit part load ratio
    Real64 &Pipe_DeltP,          // ratio of compressor ON airflow to AVERAGE airflow over timestep
    Real64 &Pipe_h_comp_out      // Piping Loss Algorithm Parameter: Enthalpy before piping loss (compressor outlet) [kJ/kg]
) const
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rongpeng Zhang
    //       DATE WRITTEN   Nov 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Determine the piping loss of the refrigerant, including both the heat loss and pressure drop.
    // This happens at VRF cooling mode, within the Main Pipe connecting Outdoor Unit to Indoor Units.

    // METHODOLOGY EMPLOYED:
    // Use a physics based piping loss model.

    using General::SolveRoot;

    int TUListNum;      // index to TU List
    int NumTUInList;    // number of terminal units is list
    int NumIUActivated; // number of the used indoor units [-]

    Real64 Pipe_v_ref;            // Piping Loss Algorithm Parameter: Refrigerant velocity [m/s]
    Real64 Pipe_T_room;           // Piping Loss Algorithm Parameter: Average Room Temperature [C]
    Real64 Pipe_T_IU_in;          // Piping Loss Algorithm Parameter: Average Refrigerant Temperature [C]
    Real64 Pipe_Num_Re;           // Piping Loss Algorithm Parameter: refrigerant Re Number [-]
    Real64 Pipe_Num_Pr;           // Piping Loss Algorithm Parameter: refrigerant Pr Number [-]
    Real64 Pipe_Num_Nu;           // Piping Loss Algorithm Parameter: refrigerant Nu Number [-]
    Real64 Pipe_Num_St;           // Piping Loss Algorithm Parameter: refrigerant St Number [-]
    Real64 Pipe_Coe_k1;           // Piping Loss Algorithm Parameter: coefficients [-]
    Real64 Pipe_Coe_k2;           // Piping Loss Algorithm Parameter: coefficients [-]
    Real64 Pipe_Coe_k3;           // Piping Loss Algorithm Parameter: coefficients [-]
    Real64 Pipe_cp_ref;           // Piping Loss Algorithm_[kJ/kg/K]
    Real64 Pipe_conductivity_ref; // Piping Loss Algorithm: refrigerant conductivity [W/m/K]
    Real64 Pipe_viscosity_ref;    // Piping Loss Algorithm Parameter: refrigerant viscosity [MuPa*s]
    Real64 Ref_Coe_v1;            // Piping Loss Algorithm Parameter: coefficient to calculate Pipe_viscosity_ref [-]
    Real64 Ref_Coe_v2;            // Piping Loss Algorithm Parameter: coefficient to calculate Pipe_viscosity_ref [-]
    Real64 Ref_Coe_v3;            // Piping Loss Algorithm Parameter: coefficient to calculate Pipe_viscosity_ref [-]
    Real64 RefPipInsH;            // Heat transfer coefficient for calculating piping loss [W/m2K]

    static constexpr std::string_view RoutineName("VRFOU_PipeLossH");

    TUListNum = this->ZoneTUListPtr;
    NumTUInList = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).NumTUInList;
    Pipe_conductivity_ref = this->RefPipInsCon;

    RefPipInsH = 9.3;
    Pipe_cp_ref = 1.6;

    // Refrigerant data
    Real64 RefTHigh = this->refrig->PsHighTempValue; // High Temperature Value for Ps (max in tables)
    Real64 RefPLow = this->refrig->PsLowPresValue;   // Low Pressure Value for Ps (>0.0)
    Real64 RefPHigh = this->refrig->PsHighPresValue; // High Pressure Value for Ps (max in tables)
    Real64 RefTSat = this->refrig->getSatTemperature(state, max(min(Pcond, RefPHigh), RefPLow), RoutineName);

    // Perform iteration to calculate Pipe_T_IU_in, given P and h
    Pipe_T_IU_in = this->refrig->getSupHeatTemp(state,
                                                max(min(Pcond, RefPHigh), RefPLow),
                                                Pipe_h_IU_in,
                                                max(this->IUCondensingTemp, RefTSat),
                                                min(this->IUCondensingTemp + 50, RefTHigh),
                                                RoutineName);
    Pipe_T_IU_in = min(RefTHigh, Pipe_T_IU_in);

    // Calculate average room temperature
    Pipe_T_room = 0;
    NumIUActivated = 0;
    for (int NumTU = 1; NumTU <= NumTUInList; ++NumTU) {
        int TUIndex = state.dataHVACVarRefFlow->TerminalUnitList(TUListNum).ZoneTUPtr(NumTU);
        int CoilIndex = state.dataHVACVarRefFlow->VRFTU(TUIndex).HeatCoilIndex;

        if (state.dataDXCoils->DXCoil(CoilIndex).TotalHeatingEnergyRate > 0.0) {
            Pipe_T_room = Pipe_T_room + state.dataDXCoils->DXCoil(CoilIndex).InletAirTemp;
            NumIUActivated = NumIUActivated + 1;
        }
    }
    if (NumIUActivated > 0) {
        Pipe_T_room = Pipe_T_room / NumIUActivated;
    } else {
        Pipe_T_room = 18;
    }

    // Calculate piping loss
    if (Pipe_m_ref > 0) {
        Ref_Coe_v1 = Pcond / 1000000 / 4.926;
        Ref_Coe_v2 = Pipe_h_IU_in / 383.5510343;
        Ref_Coe_v3 = (Pipe_T_IU_in + 273.15) / 344.39;
        Pipe_viscosity_ref = 4.302 * Ref_Coe_v1 + 0.81622 * pow_2(Ref_Coe_v1) - 120.98 * Ref_Coe_v2 + 139.17 * pow_2(Ref_Coe_v2) +
                             118.76 * Ref_Coe_v3 + 81.04 * pow_2(Ref_Coe_v3) + 5.7858 * Ref_Coe_v1 * Ref_Coe_v2 - 8.3817 * Ref_Coe_v1 * Ref_Coe_v3 -
                             218.48 * Ref_Coe_v2 * Ref_Coe_v3 + 21.58;
        if (Pipe_viscosity_ref <= 0) {
            Pipe_viscosity_ref = 16.26; // default superheated vapor viscosity data (MuPa*s) at T=353.15 K, P=2MPa
        }

        Pipe_v_ref = Pipe_m_ref / (Constant::Pi * pow_2(this->RefPipDiaDis) * 0.25) /
                     this->refrig->getSupHeatDensity(state, Pipe_T_IU_in, max(min(Pcond, RefPHigh), RefPLow), RoutineName);
        Pipe_Num_Re = Pipe_m_ref / (Constant::Pi * pow_2(this->RefPipDiaDis) * 0.25) * this->RefPipDiaDis / Pipe_viscosity_ref * 1000000;
        Pipe_Num_Pr = Pipe_viscosity_ref * Pipe_cp_ref * 0.001 / Pipe_conductivity_ref;
        Pipe_Num_Nu = 0.023 * std::pow(Pipe_Num_Re, 0.8) * std::pow(Pipe_Num_Pr, 0.4);
        Pipe_Num_St = Pipe_Num_Nu / Pipe_Num_Re / Pipe_Num_Pr;

        Pipe_Coe_k1 = Pipe_Num_Nu * Pipe_viscosity_ref;
        Pipe_Coe_k2 = this->RefPipInsCon * (this->RefPipDiaDis + this->RefPipInsThi) / this->RefPipInsThi;
        Pipe_Coe_k3 = RefPipInsH * (this->RefPipDiaDis + 2 * this->RefPipInsThi);

        Pipe_Q = max(0.0,
                     (Constant::Pi * this->RefPipLen) * (Pipe_T_IU_in - OutdoorDryBulb / 2 - Pipe_T_room / 2) /
                         (1 / Pipe_Coe_k1 + 1 / Pipe_Coe_k2 + 1 / Pipe_Coe_k3)); // [W]
        Pipe_DeltP = max(
            0.0,
            8 * Pipe_Num_St * std::pow(Pipe_Num_Pr, 0.6667) * this->RefPipEquLen / this->RefPipDiaDis *
                    this->refrig->getSupHeatDensity(state, Pipe_T_IU_in, max(min(Pcond, RefPHigh), RefPLow), RoutineName) * pow_2(Pipe_v_ref) / 2 -
                this->RefPipHei * this->refrig->getSupHeatDensity(state, Pipe_T_IU_in, max(min(Pcond, RefPHigh), RefPLow), RoutineName) * 9.80665);

        Pipe_h_comp_out = Pipe_h_IU_in + Pipe_Q / Pipe_m_ref;

    } else {
        Pipe_DeltP = 0;
        Pipe_Q = 0;
        Pipe_h_comp_out = Pipe_h_IU_in;
    }
}
void VRFCondenserEquipment::oneTimeInit([[maybe_unused]] EnergyPlusData &state)
{
}
void VRFCondenserEquipment::oneTimeInit_new([[maybe_unused]] EnergyPlusData &state)
{
}

void VRFTerminalUnitEquipment::CalcVRFSuppHeatingCoil(EnergyPlusData &state,
                                                      int const VRFTUNum,            // index of vrf terminal unit
                                                      bool const FirstHVACIteration, // True when first HVAC iteration
                                                      Real64 const PartLoadRatio,    // coil operating part-load ratio
                                                      Real64 &SuppCoilLoad           // supp heating coil load max (W)
)
{

    // PURPOSE OF THIS SUBROUTINE:
    // Manages VRF terminal unit supplemental heaters simulation.

    // Locals
    // subroutine parameter definitions:
    int constexpr MaxIte(500);   // Maximum number of iterations for solver
    Real64 constexpr Acc(1.e-3); // Accuracy of solver result

    // local variable declaration:
    Real64 SuppHeatCoilLoad; // load passed to supplemental heating coil (W)
    Real64 QActual;          // actual coil output (W)
    Real64 PartLoadFrac;     // temporary PLR variable

    QActual = 0.0;
    PartLoadFrac = 0.0;
    SuppHeatCoilLoad = 0.0;

    // simulate gas, electric, hot water, and steam heating coils
    if (state.dataEnvrn->OutDryBulbTemp <= this->MaxOATSuppHeatingCoil) {
        SuppHeatCoilLoad = SuppCoilLoad;
    } else {
        SuppHeatCoilLoad = 0.0;
    }

    switch (this->SuppHeatCoilType_Num) {
    case HVAC::Coil_HeatingGasOrOtherFuel:
    case HVAC::Coil_HeatingElectric: {
        HeatingCoils::SimulateHeatingCoilComponents(
            state, this->SuppHeatCoilName, FirstHVACIteration, SuppHeatCoilLoad, this->SuppHeatCoilIndex, QActual, true, this->fanOp, PartLoadRatio);
        SuppHeatCoilLoad = QActual;
    } break;
    case HVAC::Coil_HeatingWater: {
        if (SuppHeatCoilLoad > HVAC::SmallLoad) {
            //     see if HW coil has enough capacity to meet the load
            Real64 mdot = this->SuppHeatCoilFluidMaxFlow;
            state.dataLoopNodes->Node(this->SuppHeatCoilFluidInletNode).MassFlowRate = mdot;
            //     simulate hot water coil to find the full flow operating capacity
            WaterCoils::SimulateWaterCoilComponents(
                state, this->SuppHeatCoilName, FirstHVACIteration, this->SuppHeatCoilIndex, QActual, this->fanOp, PartLoadRatio);
            if (QActual > SuppHeatCoilLoad) {
                int SolFla; // Flag of solver, num iterations if >0, else error index
                auto f = [&state, VRFTUNum, FirstHVACIteration, SuppHeatCoilLoad](Real64 const PartLoadFrac) {
                    Real64 QActual = 0.0; // actual heating load delivered [W]
                    Real64 mdot = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow * PartLoadFrac;
                    state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode).MassFlowRate = mdot;
                    WaterCoils::SimulateWaterCoilComponents(state,
                                                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilName,
                                                            FirstHVACIteration,
                                                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilIndex,
                                                            QActual,
                                                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOp,
                                                            PartLoadFrac);
                    if (std::abs(SuppHeatCoilLoad) == 0.0) {
                        return (QActual - SuppHeatCoilLoad) / 100.0;
                    } else {
                        return (QActual - SuppHeatCoilLoad) / SuppHeatCoilLoad;
                    }
                };
                General::SolveRoot(state, Acc, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                this->SuppHeatPartLoadRatio = PartLoadFrac;
            } else {
                this->SuppHeatPartLoadRatio = 1.0;
                SuppHeatCoilLoad = QActual;
            }
        } else {
            this->SuppHeatPartLoadRatio = 0.0;
            Real64 mdot = 0.0;
            SuppHeatCoilLoad = 0.0;
            PlantUtilities::SetComponentFlowRate(
                state, mdot, this->SuppHeatCoilFluidInletNode, this->SuppHeatCoilFluidOutletNode, this->SuppHeatCoilPlantLoc);
        }
        //     simulate water heating coil
        WaterCoils::SimulateWaterCoilComponents(
            state, this->SuppHeatCoilName, FirstHVACIteration, this->SuppHeatCoilIndex, SuppHeatCoilLoad, this->fanOp, this->SuppHeatPartLoadRatio);
    } break;
    case HVAC::Coil_HeatingSteam: {
        //     simulate steam heating coil
        Real64 mdot = this->SuppHeatCoilFluidMaxFlow * PartLoadRatio;
        state.dataLoopNodes->Node(this->SuppHeatCoilFluidInletNode).MassFlowRate = mdot;
        SteamCoils::SimulateSteamCoilComponents(
            state, this->SuppHeatCoilName, FirstHVACIteration, this->SuppHeatCoilIndex, SuppHeatCoilLoad, QActual, this->fanOp, PartLoadRatio);
        SuppHeatCoilLoad = QActual;
    } break;
    default:
        break;
    }

    SuppCoilLoad = SuppHeatCoilLoad;
}

Real64 VRFTerminalUnitEquipment::HotWaterHeatingCoilResidual(EnergyPlusData &state,
                                                             Real64 const PartLoadFrac,     // water heating coil part-load ratio
                                                             std::vector<Real64> const &Par // par(1) = index to current VRF terminal unit
)
{

    // PURPOSE OF THIS FUNCTION:
    // Calculates supplemental hot water heating coils load fraction residual [(QActual - Load)/Load]
    // hot water Coil output depends on the part load ratio which is being varied to drive the load
    // fraction residual to zero.

    // METHODOLOGY EMPLOYED:
    // runs Coil:Heating:Water component object to get the actual heating load delivered [W] at a
    // given part load ratio and calculates the residual as defined above

    // Return value
    Real64 Residuum; // Residual to be minimized to zero

    // local variables declaration:
    int VRFTUNum = int(Par[1]);       // index to current terminal unit simulated
    bool FirstHVACIteration = Par[2]; // 0 flag if it first HVAC iteration, or else 1
    Real64 SuppHeatCoilLoad = Par[3]; // supplemental heating coil load to be met [W]
    Real64 QActual = 0.0;             // actual heating load delivered [W]

    // Real64 mdot = min(state.dataLoopNodes->Node(VRFTU(VRFTUNum).SuppHeatCoilFluidOutletNode).MassFlowRateMaxAvail,
    //                  VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow * PartLoadFrac);

    Real64 mdot = state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidMaxFlow * PartLoadFrac;
    state.dataLoopNodes->Node(state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilFluidInletNode).MassFlowRate = mdot;
    WaterCoils::SimulateWaterCoilComponents(state,
                                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilName,
                                            FirstHVACIteration,
                                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).SuppHeatCoilIndex,
                                            QActual,
                                            state.dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOp,
                                            PartLoadFrac);

    if (std::abs(SuppHeatCoilLoad) == 0.0) {
        Residuum = (QActual - SuppHeatCoilLoad) / 100.0;
    } else {
        Residuum = (QActual - SuppHeatCoilLoad) / SuppHeatCoilLoad;
    }

    return Residuum;
}

Real64 VRFTerminalUnitEquipment::HeatingCoilCapacityLimit(
    EnergyPlusData &state,
    Real64 const HeatCoilAirInletNode, // supplemental heating coil air inlet node
    Real64 const HeatCoilMaxSATAllowed // supplemental heating coil maximum supply air temperature allowed [C]
)
{
    // PURPOSE OF THIS FUNCTION:
    // Calculates supplemental heating coils maximum heating capacity allowed based on the maximum
    // supply air temperature limit specified.

    // METHODOLOGY EMPLOYED:
    // ( m_dot_air * Cp_air_avg * DeltaT_air_across_heating_coil) [W]

    // Return value
    Real64 HeatCoilCapacityAllowed; // heating coil maximum capacity that can be delivered at current time [W]

    Real64 MDotAir = state.dataLoopNodes->Node(HeatCoilAirInletNode).MassFlowRate;
    Real64 CpAirIn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(HeatCoilAirInletNode).HumRat);
    Real64 HCDeltaT = max(0.0, HeatCoilMaxSATAllowed - state.dataLoopNodes->Node(HeatCoilAirInletNode).Temp);
    HeatCoilCapacityAllowed = MDotAir * CpAirIn * HCDeltaT;

    return HeatCoilCapacityAllowed;
}

} // namespace EnergyPlus::HVACVariableRefrigerantFlow
