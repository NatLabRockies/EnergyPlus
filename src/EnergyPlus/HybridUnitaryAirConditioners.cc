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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/HybridEvapCoolingModel.hh>
#include <EnergyPlus/HybridUnitaryAirConditioners.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

#define TEMP_CURVE 0
#define W_CURVE 1
#define POWER_CURVE 2

namespace EnergyPlus::HybridUnitaryAirConditioners {

// Begin routines for zone HVAC Hybrid Evaporative cooler unit
void SimZoneHybridUnitaryAirConditioners(EnergyPlusData &state,
                                         std::string_view CompName,      // name of the packaged terminal heat pump
                                         int const ZoneNum,              // number of zone being served
                                         Real64 &SensibleOutputProvided, // sensible capacity delivered to zone cooling is negative
                                         Real64 &LatentOutputProvided,   // Latent add/removal  (kg/s), dehumid = negative
                                         int &CompIndex                  // index to zone hvac unit
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Spencer Maxwell Dutton
    //       DATE WRITTEN   October 2017
    //       MODIFIED
    //       RE-ENGINEERED  na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int CompNum;
    if (state.dataHybridUnitaryAC->GetInputZoneHybridEvap) {
        bool errorsfound = false;
        GetInputZoneHybridUnitaryAirConditioners(state, errorsfound);
        state.dataHybridUnitaryAC->GetInputZoneHybridEvap = false;
    }

    if (CompIndex == 0) {
        CompNum = Util::FindItemInList(CompName, state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner);
        if (CompNum == 0) {
            ShowFatalError(state, "SimZoneHybridUnitaryAirConditioners: ZoneHVAC:HybridUnitaryHVAC not found.");
        }
        CompIndex = CompNum;
    } else {
        CompNum = CompIndex;
        if (CompNum < 1 || CompNum > state.dataHybridUnitaryAC->NumZoneHybridEvap) {
            ShowFatalError(
                state,
                EnergyPlus::format("SimZoneHybridUnitaryAirConditioners: Invalid CompIndex passed={}, Number of units ={}, Entered Unit name = {}",
                                   CompNum,
                                   state.dataHybridUnitaryAC->NumZoneHybridEvap,
                                   CompName));
        }
        if (state.dataHybridUnitaryAC->CheckZoneHybridEvapName(CompNum)) {
            if (CompName != state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(CompNum).Name) {
                ShowFatalError(
                    state,
                    EnergyPlus::format(
                        "SimZoneHybridUnitaryAirConditioners: Invalid CompIndex passed={}, Unit name={}, stored unit name for that index={}",
                        CompNum,
                        CompName,
                        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(CompNum).Name));
            }
            state.dataHybridUnitaryAC->CheckZoneHybridEvapName(CompNum) = false;
        }
    }
    try {
        InitZoneHybridUnitaryAirConditioners(state, CompNum, ZoneNum);
    } catch (int e) {
        ShowFatalError(state,
                       EnergyPlus::format("An exception occurred in InitZoneHybridUnitaryAirConditioners{}, Unit name={}, stored unit name for that "
                                          "index={}. Please check idf.",
                                          CompNum,
                                          CompName,
                                          state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(CompNum).Name));
        return;
    }
    try {
        CalcZoneHybridUnitaryAirConditioners(state, CompNum, ZoneNum, SensibleOutputProvided, LatentOutputProvided);
    } catch (int e) {
        ShowFatalError(state,
                       EnergyPlus::format("An exception occurred in CalcZoneHybridUnitaryAirConditioners{}, Unit name={}, stored unit name for that "
                                          "index={}. Please check idf.",
                                          CompNum,
                                          CompName,
                                          state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(CompNum).Name));
        return;
    }
    try {
        ReportZoneHybridUnitaryAirConditioners(state, CompNum);
    } catch (int e) {
        ShowFatalError(
            state,
            EnergyPlus::format("An exception occurred in ReportZoneHybridUnitaryAirConditioners{}, Unit name={}, stored unit name for that "
                               "index={}. Please check idf.",
                               CompNum,
                               CompName,
                               state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(CompNum).Name));
        return;
    }
}

void InitZoneHybridUnitaryAirConditioners(EnergyPlusData &state,
                                          int const UnitNum, // unit number
                                          int const ZoneNum  // number of zone being served
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Spencer Maxwell Dutton
    //       DATE WRITTEN   October 2017
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //

    // METHODOLOGY EMPLOYED:
    //

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace Psychrometrics;
    using DataZoneEquipment::CheckZoneEquipmentList;

    // Locals
    int InletNode;

    if (state.dataHybridUnitaryAC->HybridCoolOneTimeFlag) {
        // These three look like they are not used anywhere
        state.dataHybridUnitaryAC->MySizeFlag.dimension(state.dataHybridUnitaryAC->NumZoneHybridEvap, true);
        state.dataHybridUnitaryAC->MyEnvrnFlag.dimension(state.dataHybridUnitaryAC->NumZoneHybridEvap, true);
        state.dataHybridUnitaryAC->MyFanFlag.dimension(state.dataHybridUnitaryAC->NumZoneHybridEvap, true);

        state.dataHybridUnitaryAC->MyZoneEqFlag.dimension(state.dataHybridUnitaryAC->NumZoneHybridEvap, true);
        state.dataHybridUnitaryAC->HybridCoolOneTimeFlag = false;
    }
    if (!state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).Initialized) {
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).Initialize(ZoneNum);
    }
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).RequestedLoadToHeatingSetpoint = 0;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).RequestedLoadToCoolingSetpoint = 0;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).RequestedHumidificationMass = 0;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).RequestedHumidificationLoad = 0;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).RequestedHumidificationEnergy = 0;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).RequestedDeHumidificationMass = 0;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).RequestedDeHumidificationLoad = 0;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).RequestedDeHumidificationEnergy = 0;

    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).UnitTotalCoolingRate = 0.0;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).UnitTotalCoolingEnergy = 0.0;

    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).UnitSensibleCoolingRate = 0.0;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).UnitSensibleCoolingEnergy = 0.0;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).UnitLatentCoolingRate = 0.0;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).UnitLatentCoolingEnergy = 0.0;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).availStatus = Avail::Status::NoAction;

    // set the availability status based on the availability manager list name
    if (allocated(state.dataAvail->ZoneComp)) {
        auto &availMgr = state.dataAvail->ZoneComp(DataZoneEquipment::ZoneEquipType::HybridEvaporativeCooler).ZoneCompAvailMgrs(UnitNum);
        if (state.dataHybridUnitaryAC->MyZoneEqFlag(UnitNum)) { // initialize the name of each availability manager list and zone number
            availMgr.AvailManagerListName = state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).AvailManagerListName;
            availMgr.ZoneNum = ZoneNum;
            state.dataHybridUnitaryAC->MyZoneEqFlag(UnitNum) = false;
        }
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).availStatus = availMgr.availStatus;
    }

    // need to check all zone outdoor air control units to see if they are on Zone Equipment List or issue warning
    if (!state.dataHybridUnitaryAC->ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
        state.dataHybridUnitaryAC->ZoneEquipmentListChecked = true;
        for (int Loop = 1; Loop <= state.dataHybridUnitaryAC->NumZoneHybridEvap; ++Loop) {
            if (CheckZoneEquipmentList(state, "ZoneHVAC:HybridUnitaryHVAC", state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(Loop).Name)) {
                state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(Loop).ZoneNodeNum = state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ZoneNode;
            } else {
                ShowSevereError(
                    state,
                    EnergyPlus::format("InitZoneHybridUnitaryAirConditioners: ZoneHVAC:HybridUnitaryHVAC = {}, is not on any ZoneHVAC:EquipmentList. "
                                       " It will not be simulated.",
                                       state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(Loop).Name));
            }
        }
    }

    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InitializeModelParams();
    // Do the following initializations (every time step): This should be the info from
    // the previous components outlets or the node data in this section.

    // Transfer the node data to EvapCond data structure
    InletNode = state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletNode;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletMassFlowRate = state.dataLoopNodes->Node(InletNode).MassFlowRate;

    // Set the inlet volumetric flow rate from the mass flow rate
    if (state.dataEnvrn->StdRhoAir > 1) {
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletVolumetricFlowRate =
            state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletMassFlowRate / state.dataEnvrn->StdRhoAir;
    } else {
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletVolumetricFlowRate =
            state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletMassFlowRate / 1.225;
    }

    // Set all of the inlet state variables from the inlet nodes
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletTemp = state.dataLoopNodes->Node(InletNode).Temp;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletHumRat = state.dataLoopNodes->Node(InletNode).HumRat;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletEnthalpy = state.dataLoopNodes->Node(InletNode).Enthalpy;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletPressure = state.dataLoopNodes->Node(InletNode).Press;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletRH =
        PsyRhFnTdbWPb(state,
                      state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletTemp,
                      state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletHumRat,
                      state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletPressure,
                      "InitZoneHybridUnitaryAirConditioners");

    // Set default outlet state to inlet states, just to be safe
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletTemp =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletTemp;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletHumRat =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletHumRat;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletEnthalpy =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletEnthalpy;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletPressure =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletPressure;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletRH =
        PsyRhFnTdbWPb(state,
                      state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletTemp,
                      state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletHumRat,
                      state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletPressure,
                      "InitZoneHybridUnitaryAirConditioners");
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletMassFlowRate =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletMassFlowRate;

    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletTemp =
        state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryInletNode).Temp;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletHumRat =
        state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryInletNode).HumRat;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletEnthalpy =
        state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryInletNode).Enthalpy;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletPressure =
        state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryInletNode).Press;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletRH =
        PsyRhFnTdbWPb(state,
                      state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletTemp,
                      state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletHumRat,
                      state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletPressure,
                      "InitZoneHybridUnitaryAirConditioners");
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletMassFlowRate =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SupplyVentilationAir;

    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecOutletTemp =
        state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryInletNode).Temp;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecOutletHumRat =
        state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryInletNode).HumRat;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecOutletEnthalpy =
        state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryInletNode).Enthalpy;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecOutletPressure =
        state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryInletNode).Press;
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecOutletRH =
        PsyRhFnTdbWPb(state,
                      state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletTemp,
                      state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletHumRat,
                      state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletPressure,
                      "InitZoneHybridUnitaryAirConditioners");
}

void CalcZoneHybridUnitaryAirConditioners(EnergyPlusData &state,
                                          int const UnitNum,              // unit number
                                          int const ZoneNum,              // number of zone being served
                                          Real64 &SensibleOutputProvided, // sensible capacity delivered to zone cooling negative
                                          Real64 &LatentOutputProvided    // Latent add/removal  (kg/s), dehumid = negative
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Spencer Maxwell Dutton
    //       DATE WRITTEN   October 2017
    //       MODIFIED
    //       RE-ENGINEERED  na

    using namespace Psychrometrics;

    Real64 EnvDryBulbT, AirTempRoom, EnvRelHumm, RoomRelHum, DesignMinVR;

    Real64 ZoneCoolingLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum)
                                 .RemainingOutputReqToCoolSP; // Remaining load required to meet cooling setpoint (<0 is a cooling load)
    Real64 ZoneHeatingLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum)
                                 .RemainingOutputReqToHeatSP; // Remaining load required to meet heating setpoint (>0 is a heating load)
    Real64 OutputRequiredToHumidify =
        state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ZoneNum)
            .OutputRequiredToHumidifyingSP; // Load required to meet humidifying setpoint (>0 = a humidify load) [kgWater/s]

    Real64 OutputRequiredToDehumidify =
        state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ZoneNum)
            .OutputRequiredToDehumidifyingSP; // Load required to meet dehumidifying setpoint (<0 = a dehumidify load)  [kgWater/s]

    SensibleOutputProvided = 0;
    LatentOutputProvided = 0;
    // taking class members out of the object and then using them in the calculation is odd but its for clarity with unit testing.
    EnvDryBulbT = state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletTemp; // degrees C
    AirTempRoom = state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletTemp;    // degrees C
    EnvRelHumm = state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletRH;    // RH
    RoomRelHum = state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletRH;       // RH

    bool UseOccSchFlag = true;
    bool UseMinOASchFlag = true;

    DesignMinVR = DataSizing::calcDesignSpecificationOutdoorAir(state,
                                                                state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OARequirementsPtr,
                                                                ZoneNum,
                                                                UseOccSchFlag,
                                                                UseMinOASchFlag); //[m3/s]
    Real64 DesignMinVRMassFlow = 0;
    if (state.dataEnvrn->StdRhoAir > 1) {
        DesignMinVRMassFlow = DesignMinVR * state.dataEnvrn->StdRhoAir;
    } else {
        DesignMinVRMassFlow = DesignMinVR * 1.225;
    }
    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).doStep(
        state, ZoneCoolingLoad, ZoneHeatingLoad, OutputRequiredToHumidify, OutputRequiredToDehumidify, DesignMinVRMassFlow);
    SensibleOutputProvided = -state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).QSensZoneOut; // cooling negative

    LatentOutputProvided = -state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).QLatentZoneOutMass; // dehumidification negative kg/s
}

void ReportZoneHybridUnitaryAirConditioners(EnergyPlusData &state, int const UnitNum)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Spencer Maxwell Dutton
    //       DATE WRITTEN   October 2017
    //       MODIFIED
    //       RE-ENGINEERED  na

    // Using/Aliasing
    using namespace Psychrometrics;

    state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).PrimaryMode =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).PrimaryMode;

    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletNode).Temp =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletTemp;
    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletNode).HumRat =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletHumRat;
    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletNode).MassFlowRate =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletMassFlowRate;
    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletNode).Enthalpy =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).InletEnthalpy;

    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletNode).Temp =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletTemp;
    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletNode).HumRat =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletHumRat;
    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletNode).MassFlowRate =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletMassFlowRate;
    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletNode).Enthalpy =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).OutletEnthalpy;

    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryInletNode).Temp =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletTemp;
    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryInletNode).HumRat =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletHumRat;
    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryInletNode).Enthalpy =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletEnthalpy;
    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryInletNode).MassFlowRate =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecInletMassFlowRate;

    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryOutletNode).Temp =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecOutletTemp;
    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryOutletNode).HumRat =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecOutletHumRat;
    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryOutletNode).Enthalpy =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecOutletEnthalpy;
    state.dataLoopNodes->Node(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecondaryOutletNode).MassFlowRate =
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitNum).SecOutletMassFlowRate;
}

void GetInputZoneHybridUnitaryAirConditioners(EnergyPlusData &state, bool &Errors)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Spencer Maxwell Dutton
    //       DATE WRITTEN   October 2017
    //       MODIFIED
    //       RE-ENGINEERED  na

    // Using/Aliasing
    using Node::GetOnlySingleNode;
    using Node::SetUpCompSets;
    using Node::TestCompSet;
    std::string cCurrentModuleObject; // Object type for getting and error messages
    Array1D_string Alphas;            // Alpha items for object
    Array1D<Real64> Numbers;          // Numeric items for object
    Array1D_string cAlphaFields;      // Alpha field names
    Array1D_string cNumericFields;    // Numeric field names
    Array1D_bool lAlphaBlanks;        // Logical array, alpha field input BLANK = .TRUE.
    Array1D_bool lNumericBlanks;      // Logical array, numeric field input BLANK = .TRUE.
    int NumAlphas;                    // Number of Alphas for each GetObjectItem call
    int NumNumbers;                   // Number of Numbers for each GetObjectItem call
    int NumFields;                    // Total number of fields in object
    bool ErrorsFound(false);          // Set to true if errors in input, fatal at end of routine
    int UnitLoop;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view routineName = "GetInputZoneHybridUnitaryAirConditioners";

    cCurrentModuleObject = "ZoneHVAC:HybridUnitaryHVAC";
    state.dataHybridUnitaryAC->NumZoneHybridEvap = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumFields, NumAlphas, NumNumbers);
    int MaxNumbers = max(0, NumNumbers); // Maximum number of numeric fields in all objects
    int MaxAlphas = max(0, NumAlphas);   // Maximum number of alpha fields in all objects
    Alphas.allocate(MaxAlphas);
    Numbers.dimension(MaxNumbers, 0.0);
    cAlphaFields.allocate(MaxAlphas);
    cNumericFields.allocate(MaxNumbers);
    lAlphaBlanks.dimension(MaxAlphas, true);
    lNumericBlanks.dimension(MaxNumbers, true);

    if (state.dataHybridUnitaryAC->NumZoneHybridEvap > 0) {
        state.dataHybridUnitaryAC->CheckZoneHybridEvapName.dimension(state.dataHybridUnitaryAC->NumZoneHybridEvap, true);
        state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner.allocate(state.dataHybridUnitaryAC->NumZoneHybridEvap);
        int IOStatus = 0;

        for (UnitLoop = 1; UnitLoop <= state.dataHybridUnitaryAC->NumZoneHybridEvap; ++UnitLoop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     UnitLoop,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, Alphas(1)};
            auto &hybridUnitaryAC = state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitLoop);

            // A1, \field Name
            hybridUnitaryAC.Name = Alphas(1);
            // A2, \field Availability Schedule Name
            if (lAlphaBlanks(2)) {
                hybridUnitaryAC.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((hybridUnitaryAC.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }
            // A3, \field Availability Manager List Name
            if (!lAlphaBlanks(3)) {
                hybridUnitaryAC.AvailManagerListName = Alphas(3);
            }

            // A4, \field Minimum Supply Air Temperature Schedule Named
            if (lAlphaBlanks(4)) {
            } else if ((hybridUnitaryAC.TsaMinSched = Sched::GetSchedule(state, Alphas(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(4), Alphas(4));
                ErrorsFound = true;
            }

            // A5, \field Maximum Supply Air Temperature Schedule Name
            if (lAlphaBlanks(5)) {
            } else if ((hybridUnitaryAC.TsaMaxSched = Sched::GetSchedule(state, Alphas(5))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(5), Alphas(5));
                ErrorsFound = true;
            }

            // A6, \field Minimum Supply Air Humidity Ratio Schedule Name
            if (lAlphaBlanks(6)) {
            } else if ((hybridUnitaryAC.RHsaMinSched = Sched::GetSchedule(state, Alphas(6))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(6), Alphas(6));
                ErrorsFound = true;
            }

            // A7, \field Maximum Supply Air Humidity Ratio Schedule Name
            if (lAlphaBlanks(7)) {
            } else if ((hybridUnitaryAC.RHsaMaxSched = Sched::GetSchedule(state, Alphas(7))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(7), Alphas(7));
                ErrorsFound = true;
            }

            // A8, \field Method to Choose Value of Controlled Inputs

            // A9, \field Return Air Node Name
            // A10, \field Outdoor Air Node Name
            // A11, \field Supply Air Node Name
            // A12, \field Relief Node Name
            hybridUnitaryAC.InletNode = GetOnlySingleNode(state,
                                                          Alphas(9),
                                                          ErrorsFound,
                                                          Node::ConnectionObjectType::ZoneHVACHybridUnitaryHVAC,
                                                          Alphas(1),
                                                          Node::FluidType::Air,
                                                          Node::ConnectionType::Inlet,
                                                          Node::CompFluidStream::Primary,
                                                          Node::ObjectIsNotParent);
            hybridUnitaryAC.SecondaryInletNode = GetOnlySingleNode(state,
                                                                   Alphas(10),
                                                                   ErrorsFound,
                                                                   Node::ConnectionObjectType::ZoneHVACHybridUnitaryHVAC,
                                                                   Alphas(1),
                                                                   Node::FluidType::Air,
                                                                   Node::ConnectionType::OutsideAir,
                                                                   Node::CompFluidStream::Primary,
                                                                   Node::ObjectIsNotParent);
            hybridUnitaryAC.OutletNode = GetOnlySingleNode(state,
                                                           Alphas(11),
                                                           ErrorsFound,
                                                           Node::ConnectionObjectType::ZoneHVACHybridUnitaryHVAC,
                                                           Alphas(1),
                                                           Node::FluidType::Air,
                                                           Node::ConnectionType::Outlet,
                                                           Node::CompFluidStream::Primary,
                                                           Node::ObjectIsNotParent);
            hybridUnitaryAC.SecondaryOutletNode = GetOnlySingleNode(state,
                                                                    Alphas(12),
                                                                    ErrorsFound,
                                                                    Node::ConnectionObjectType::ZoneHVACHybridUnitaryHVAC,
                                                                    Alphas(1),
                                                                    Node::FluidType::Air,
                                                                    Node::ConnectionType::ReliefAir,
                                                                    Node::CompFluidStream::Primary,
                                                                    Node::ObjectIsNotParent);
            TestCompSet(state, cCurrentModuleObject, Alphas(1), Alphas(9), Alphas(11), "Hybrid Evap Air Zone Nodes");
            TestCompSet(state, cCurrentModuleObject, Alphas(1), Alphas(10), Alphas(12), "Hybrid Evap Air Zone Secondary Nodes");

            // N1, \field System Maximum Supply AirFlow Rate
            hybridUnitaryAC.SystemMaximumSupplyAirFlowRate = Numbers(1);

            // N2, \field External Static Pressure at System Maximum Supply Air Flow Rate

            // In each time step, the result for system power, fan power, gas use, water user, or supply airflow rate will be determined as :
            // TableValue * SysMaxSupply * ScalingFactor
            // A13, \field Fan Heat Included in Lookup Tables
            hybridUnitaryAC.FanHeatGain = false;
            if (!lAlphaBlanks(13)) {
                if (Util::SameString(Alphas(13), "Yes")) {
                    hybridUnitaryAC.FanHeatGain = false;
                } else if (Util::SameString(Alphas(13), "No")) {
                    hybridUnitaryAC.FanHeatGain = true;
                } else {
                    ShowSevereError(state, EnergyPlus::format("{} = {}", cCurrentModuleObject, hybridUnitaryAC.Name));
                    ShowContinueError(state, EnergyPlus::format("Illegal {} = {}", cAlphaFields(13), Alphas(13)));
                    ErrorsFound = true;
                }
            }
            // A14, \field Fan Heat Gain Location
            if (!lAlphaBlanks(14)) {
                hybridUnitaryAC.FanHeatGainLocation = Alphas(14);
            }
            // N3, \field Fan Heat in Air Stream Fraction
            hybridUnitaryAC.FanHeatInAirFrac = Numbers(3);
            // N4, \field Scaling Factor
            hybridUnitaryAC.ScalingFactor = Numbers(4);
            // the two numbers above are used to generate a overall scaling factor
            hybridUnitaryAC.ScaledSystemMaximumSupplyAirVolumeFlowRate = Numbers(1) * Numbers(4);
            if (state.dataEnvrn->StdRhoAir > 1) {
                // SystemMaximumSupplyAirFlowRate*ScalingFactor*AirDensity;
                hybridUnitaryAC.ScaledSystemMaximumSupplyAirMassFlowRate =
                    hybridUnitaryAC.ScaledSystemMaximumSupplyAirVolumeFlowRate * state.dataEnvrn->StdRhoAir;
            } else {
                hybridUnitaryAC.ScaledSystemMaximumSupplyAirMassFlowRate = hybridUnitaryAC.ScaledSystemMaximumSupplyAirVolumeFlowRate * 1.225;
            }

            // N5, \field Minimum Time Between Mode Change
            // A15, \field First fuel type
            if (lAlphaBlanks(15)) {
                // The original code never checks if this field is empty, but adding this check leads to a unit test failure.
                // ShowSevereEmptyField(state, eoh, cAlphaFields(15));
                // ErrorsFound = true;
            } else if ((hybridUnitaryAC.firstFuel = static_cast<Constant::eFuel>(
                            getEnumValue(Constant::eFuelNamesUC, Util::makeUPPER(Alphas(15))))) == Constant::eFuel::Invalid) {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(15), Alphas(15));
                ErrorsFound = true;
            }

            // A16, \field Second fuel type
            if (!lAlphaBlanks(16) && Alphas(16) != "NONE" &&
                (hybridUnitaryAC.secondFuel = static_cast<Constant::eFuel>(getEnumValue(Constant::eFuelNamesUC, Util::makeUPPER(Alphas(16))))) ==
                    Constant::eFuel::Invalid) {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(16), Alphas(16));
                ErrorsFound = true;
            }

            // A17, \field Third fuel type
            if (!lAlphaBlanks(17) && Alphas(17) != "NONE" &&
                (hybridUnitaryAC.thirdFuel = static_cast<Constant::eFuel>(getEnumValue(Constant::eFuelNamesUC, Util::makeUPPER(Alphas(17))))) ==
                    Constant::eFuel::Invalid) {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(17), Alphas(17));
                ErrorsFound = true;
            }

            // A18, \field Objective Function Minimizes
            if (!lAlphaBlanks(18) && (hybridUnitaryAC.ObjectiveFunction = static_cast<HybridEvapCoolingModel::ObjectiveFunctionType>(
                                          getEnumValue(HybridEvapCoolingModel::objectiveFunctionNamesUC, Util::makeUPPER(Alphas(18))))) ==
                                         HybridEvapCoolingModel::ObjectiveFunctionType::Invalid) {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(18), Alphas(18));
                ErrorsFound = true;
            }

            // A19, \ OA requirement pointer
            if (!lAlphaBlanks(19)) {
                hybridUnitaryAC.OARequirementsPtr = Util::FindItemInList(Alphas(19), state.dataSize->OARequirements);
                if (hybridUnitaryAC.OARequirementsPtr == 0) {
                    ShowSevereError(state, EnergyPlus::format("{}: {} = {} invalid data", routineName, cCurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, EnergyPlus::format("Invalid-not found {}=\"{}\".", cAlphaFields(19), Alphas(19)));
                    ErrorsFound = true;
                } else {
                    hybridUnitaryAC.OutdoorAir = true;
                }
            }

            int FirstModeAlphaNumber = 20;
            int NumberOfAlphasPerMode = 9;
            int Numberofoperatingmodes = 0;
            for (int i = FirstModeAlphaNumber; i <= NumAlphas; i = i + NumberOfAlphasPerMode) {
                if (!lAlphaBlanks(i)) {
                    ++Numberofoperatingmodes;
                } else {
                    break;
                }
            }

            for (int modeIter = 0; modeIter <= Numberofoperatingmodes - 1; ++modeIter) {
                ErrorsFound = hybridUnitaryAC.ParseMode(state, Alphas, cAlphaFields, Numbers, cNumericFields, lAlphaBlanks, cCurrentModuleObject);
                if (ErrorsFound) {
                    ShowFatalError(state, EnergyPlus::format("{}: Errors found parsing modes", routineName));
                    ShowContinueError(state, "... Preceding condition causes termination.");
                    break;
                }
            }
            // add the ZoneHVAC:HybridUnitaryHVAC Scaled Maximum Supply Air Volume Flow Rate to the Component Sizing Report Summary
            BaseSizer::reportSizerOutput(state,
                                         cCurrentModuleObject,
                                         hybridUnitaryAC.Name,
                                         "Scaled Maximum Supply Air Volume Flow Rate [m3/s]",
                                         hybridUnitaryAC.ScaledSystemMaximumSupplyAirVolumeFlowRate);
        }
    }

    // setup output variables
    for (UnitLoop = 1; UnitLoop <= state.dataHybridUnitaryAC->NumZoneHybridEvap; ++UnitLoop) {

        auto &hybridUnitaryAC = state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitLoop);

        SetUpCompSets(state,
                      cCurrentModuleObject,
                      hybridUnitaryAC.Name,
                      cCurrentModuleObject,
                      hybridUnitaryAC.Name,
                      state.dataLoopNodes->NodeID(hybridUnitaryAC.InletNode),
                      state.dataLoopNodes->NodeID(hybridUnitaryAC.OutletNode));

        SetUpCompSets(state,
                      cCurrentModuleObject,
                      hybridUnitaryAC.Name,
                      cCurrentModuleObject,
                      hybridUnitaryAC.Name,
                      state.dataLoopNodes->NodeID(hybridUnitaryAC.SecondaryInletNode),
                      state.dataLoopNodes->NodeID(hybridUnitaryAC.SecondaryOutletNode));

        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC System Total Cooling Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.SystemTotalCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC System Total Cooling Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.SystemTotalCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name,
                            Constant::eResource::EnergyTransfer,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::CoolingCoils);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC System Sensible Cooling Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.SystemSensibleCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC System Sensible Cooling Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.SystemSensibleCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC System Latent Cooling Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.SystemLatentCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC System Latent Cooling Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.SystemLatentCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name);

        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Zone Total Cooling Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.UnitTotalCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Zone Total Cooling Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.UnitTotalCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Zone Sensible Cooling Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.UnitSensibleCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Zone Sensible Cooling Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.UnitSensibleCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Zone Latent Cooling Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.UnitLatentCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Zone Latent Cooling Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.UnitLatentCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name);

        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC System Total Heating Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.SystemTotalHeatingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC System Total Heating Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.SystemTotalHeatingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name,
                            Constant::eResource::EnergyTransfer,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::HeatingCoils);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC System Sensible Heating Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.SystemSensibleHeatingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC System Sensible Heating Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.SystemSensibleHeatingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC System Latent Heating Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.SystemLatentHeatingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC System Latent Heating Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.SystemLatentHeatingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name);

        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Zone Total Heating Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.UnitTotalHeatingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Zone Total Heating Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.UnitTotalHeatingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Zone Sensible Heating Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.UnitSensibleHeatingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Zone Sensible Heating Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.UnitSensibleHeatingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Zone Latent Heating Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.UnitLatentHeatingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Zone Latent Heating Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.UnitLatentHeatingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name);

        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Predicted Sensible Load to Setpoint Heat Transfer Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.RequestedLoadToCoolingSetpoint,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Error Code",
                            Constant::Units::None,
                            hybridUnitaryAC.ErrorCode,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);

        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Supply Air Temperature",
                            Constant::Units::C,
                            hybridUnitaryAC.OutletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Return Air Temperature",
                            Constant::Units::C,
                            hybridUnitaryAC.InletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Outdoor Air Temperature",
                            Constant::Units::C,
                            hybridUnitaryAC.SecInletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Relief Air Temperature",
                            Constant::Units::C,
                            hybridUnitaryAC.SecOutletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);

        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Supply Air Humidity Ratio",
                            Constant::Units::kgWater_kgDryAir,
                            hybridUnitaryAC.OutletHumRat,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Return Air Humidity Ratio",
                            Constant::Units::kgWater_kgDryAir,
                            hybridUnitaryAC.InletHumRat,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Outdoor Air Humidity Ratio",
                            Constant::Units::kgWater_kgDryAir,
                            hybridUnitaryAC.SecInletHumRat,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Relief Air Humidity Ratio",
                            Constant::Units::kgWater_kgDryAir,
                            hybridUnitaryAC.SecOutletHumRat,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);

        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Supply Air Relative Humidity",
                            Constant::Units::Perc,
                            hybridUnitaryAC.OutletRH,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Return Air Relative Humidity",
                            Constant::Units::Perc,
                            hybridUnitaryAC.InletRH,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Outdoor Air Relative Humidity",
                            Constant::Units::Perc,
                            hybridUnitaryAC.SecInletRH,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Relief Air Relative Humidity",
                            Constant::Units::Perc,
                            hybridUnitaryAC.SecOutletRH,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);

        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Supply Air Mass Flow Rate",
                            Constant::Units::kg_s,
                            hybridUnitaryAC.OutletMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Supply Air Standard Density Volume Flow Rate",
                            Constant::Units::m3_s,
                            hybridUnitaryAC.OutletVolumetricFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Return Air Mass Flow Rate",
                            Constant::Units::kg_s,
                            hybridUnitaryAC.InletMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Return Air Standard Density Volume Flow Rate",
                            Constant::Units::m3_s,
                            hybridUnitaryAC.InletVolumetricFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Relief Air Mass Flow Rate",
                            Constant::Units::kg_s,
                            hybridUnitaryAC.SecOutletMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Relief Air Standard Density Volume Flow Rate",
                            Constant::Units::m3_s,
                            hybridUnitaryAC.SecOutletVolumetricFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Ventilation Air Standard Density Volume Flow Rate",
                            Constant::Units::m3_s,
                            hybridUnitaryAC.SupplyVentilationVolume,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Electricity Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.FinalElectricalPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Electricity Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.FinalElectricalEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Cooling,
                            "Hybrid HVAC Cooling");

        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Requested Outdoor Air Ventilation Mass Flow Rate",
                            Constant::Units::kg_s,
                            hybridUnitaryAC.MinOA_Msa,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Ventilation Air Mass Flow Rate",
                            Constant::Units::kg_s,
                            hybridUnitaryAC.SupplyVentilationAir,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Availability Status",
                            Constant::Units::None,
                            hybridUnitaryAC.UnitOn,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Outdoor Air Fraction",
                            Constant::Units::None,
                            hybridUnitaryAC.averageOSAF,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);

        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Dehumidification Load to Humidistat Setpoint Moisture Transfer Rate",
                            Constant::Units::kg_s,
                            hybridUnitaryAC.RequestedDeHumidificationMass,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Dehumidification Load to Humidistat Setpoint Heat Transfer Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.RequestedDeHumidificationLoad,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Dehumidification Load to Humidistat Setpoint Heat Transfer Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.RequestedDeHumidificationEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);

        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Humidification Load to Humidistat Setpoint Moisture Transfer Rate",
                            Constant::Units::kg_s,
                            hybridUnitaryAC.RequestedHumidificationMass,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Humidification Load to Humidistat Setpoint Heat Transfer Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.RequestedHumidificationLoad,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Humidification Load to Humidistat Setpoint Heat Transfer Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.RequestedHumidificationEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);

        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Supply Fan Electricity Rate",
                            Constant::Units::W,
                            hybridUnitaryAC.SupplyFanElectricPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Supply Fan Electricity Energy",
                            Constant::Units::J,
                            hybridUnitaryAC.SupplyFanElectricEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Fans,
                            "Hybrid HVAC Fans");
        if (hybridUnitaryAC.secondFuel != Constant::eFuel::Invalid) {
            SetupOutputVariable(state,
                                "Zone Hybrid Unitary HVAC Secondary Fuel Consumption Rate",
                                Constant::Units::W,
                                hybridUnitaryAC.SecondaryFuelConsumptionRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                hybridUnitaryAC.Name);
            SetupOutputVariable(state,
                                "Zone Hybrid Unitary HVAC Secondary Fuel Consumption",
                                Constant::Units::J,
                                hybridUnitaryAC.SecondaryFuelConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                hybridUnitaryAC.Name,
                                Constant::eFuel2eResource[(int)hybridUnitaryAC.secondFuel],
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling,
                                "Hybrid HVAC Cooling");
        }
        if (hybridUnitaryAC.thirdFuel != Constant::eFuel::Invalid) {
            SetupOutputVariable(state,
                                "Zone Hybrid Unitary HVAC Third Fuel Consumption Rate",
                                Constant::Units::W,
                                hybridUnitaryAC.ThirdFuelConsumptionRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                hybridUnitaryAC.Name);
            SetupOutputVariable(state,
                                "Zone Hybrid Unitary HVAC Third Fuel Consumption",
                                Constant::Units::J,
                                hybridUnitaryAC.ThirdFuelConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                hybridUnitaryAC.Name,
                                Constant::eFuel2eResource[(int)hybridUnitaryAC.thirdFuel],
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling,
                                "Hybrid HVAC Cooling");
        }
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Water Consumption Rate",
                            Constant::Units::kgWater_s,
                            hybridUnitaryAC.WaterConsumptionRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC Water Consumption",
                            Constant::Units::m3,
                            hybridUnitaryAC.WaterConsumption,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            hybridUnitaryAC.Name,
                            Constant::eResource::Water,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Cooling,
                            "Hybrid HVAC Cooling");
        SetupOutputVariable(state,
                            "Zone Hybrid Unitary HVAC External Static Pressure",
                            Constant::Units::Pa,
                            hybridUnitaryAC.ExternalStaticPressure,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            hybridUnitaryAC.Name);

        if (hybridUnitaryAC.FanHeatGain) {
            SetupOutputVariable(state,
                                "Zone Hybrid Unitary HVAC Fan Rise in Air Temperature",
                                Constant::Units::deltaC,
                                hybridUnitaryAC.FanHeatTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                hybridUnitaryAC.Name);
            SetupOutputVariable(state,
                                "Zone Hybrid Unitary HVAC Fan Heat Gain to Air",
                                Constant::Units::W,
                                hybridUnitaryAC.PowerLossToAir,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                hybridUnitaryAC.Name);
        }

        int index = 0;

        for (auto &thisSetting : hybridUnitaryAC.CurrentOperatingSettings) {
            SetupOutputVariable(state,
                                EnergyPlus::format("Zone Hybrid Unitary HVAC Runtime Fraction in Setting {}", index),
                                Constant::Units::None,
                                thisSetting.Runtime_Fraction,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                hybridUnitaryAC.Name);
            SetupOutputVariable(state,
                                EnergyPlus::format("Zone Hybrid Unitary HVAC Mode in Setting {}", index),
                                Constant::Units::None,
                                thisSetting.Mode,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                hybridUnitaryAC.Name);
            SetupOutputVariable(state,
                                EnergyPlus::format("Zone Hybrid Unitary HVAC Outdoor Air Fraction in Setting {}", index),
                                Constant::Units::kg_s,
                                thisSetting.Outdoor_Air_Fraction,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                hybridUnitaryAC.Name);
            SetupOutputVariable(state,
                                EnergyPlus::format("Zone Hybrid Unitary HVAC Supply Air Mass Flow Rate in Setting {}", index),
                                Constant::Units::kg_s,
                                thisSetting.Unscaled_Supply_Air_Mass_Flow_Rate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                hybridUnitaryAC.Name);
            SetupOutputVariable(state,
                                EnergyPlus::format("Zone Hybrid Unitary HVAC Supply Air Mass Flow Rate Ratio in Setting {}", index),
                                Constant::Units::None,
                                thisSetting.Supply_Air_Mass_Flow_Rate_Ratio,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                hybridUnitaryAC.Name);
            index++;
        }
    }
    Errors = ErrorsFound;
    if (ErrorsFound) {
        ShowFatalError(state, EnergyPlus::format("{}: Errors found in getting input.", routineName));
        ShowContinueError(state, "... Preceding condition causes termination.");
    }
}
int GetHybridUnitaryACOutAirNode(EnergyPlusData &state, int const CompNum)
{
    if (state.dataHybridUnitaryAC->GetInputZoneHybridEvap) {
        bool errorsfound = false;
        GetInputZoneHybridUnitaryAirConditioners(state, errorsfound);
        state.dataHybridUnitaryAC->GetInputZoneHybridEvap = false;
    }

    int GetHybridUnitaryACOutAirNode;
    GetHybridUnitaryACOutAirNode = 0;
    if (CompNum > 0 && CompNum <= state.dataHybridUnitaryAC->NumZoneHybridEvap) {
        GetHybridUnitaryACOutAirNode = state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(CompNum).SecondaryInletNode;
    }

    return GetHybridUnitaryACOutAirNode;
}

int GetHybridUnitaryACZoneInletNode(EnergyPlusData &state, int const CompNum)
{
    if (state.dataHybridUnitaryAC->GetInputZoneHybridEvap) {
        bool errorsfound = false;
        GetInputZoneHybridUnitaryAirConditioners(state, errorsfound);
        state.dataHybridUnitaryAC->GetInputZoneHybridEvap = false;
    }

    int GetHybridUnitaryACZoneInletNode;
    GetHybridUnitaryACZoneInletNode = 0;
    if (CompNum > 0 && CompNum <= state.dataHybridUnitaryAC->NumZoneHybridEvap) {
        GetHybridUnitaryACZoneInletNode = state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(CompNum).OutletNode;
    }

    return GetHybridUnitaryACZoneInletNode;
}

int GetHybridUnitaryACReturnAirNode(EnergyPlusData &state, int const CompNum)
{
    if (state.dataHybridUnitaryAC->GetInputZoneHybridEvap) {
        bool errorsfound = false;
        GetInputZoneHybridUnitaryAirConditioners(state, errorsfound);
        state.dataHybridUnitaryAC->GetInputZoneHybridEvap = false;
    }

    int GetHybridUnitaryACReturnAirNode;
    GetHybridUnitaryACReturnAirNode = 0;
    if (CompNum > 0 && CompNum <= state.dataHybridUnitaryAC->NumZoneHybridEvap) {
        GetHybridUnitaryACReturnAirNode = state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(CompNum).InletNode;
    }

    return GetHybridUnitaryACReturnAirNode;
}

int getHybridUnitaryACIndex(EnergyPlusData &state, std::string_view CompName)
{
    if (state.dataHybridUnitaryAC->GetInputZoneHybridEvap) {
        bool errFlag = false;
        GetInputZoneHybridUnitaryAirConditioners(state, errFlag);
        state.dataHybridUnitaryAC->GetInputZoneHybridEvap = false;
    }

    for (int UnitLoop = 1; UnitLoop <= state.dataHybridUnitaryAC->NumZoneHybridEvap; ++UnitLoop) {
        if (Util::SameString(state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(UnitLoop).Name, CompName)) {
            return UnitLoop;
        }
    }

    return 0;
}

//*****************************************************************************************

} // namespace EnergyPlus::HybridUnitaryAirConditioners
