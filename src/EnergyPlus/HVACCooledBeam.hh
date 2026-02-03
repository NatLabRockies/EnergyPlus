// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

#ifndef HVACCooledBeam_hh_INCLUDED
#define HVACCooledBeam_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

namespace HVACCooledBeam {

    enum class CooledBeamType
    {
        Invalid = -1,
        Passive,
        Active,
        Num
    };

    Real64 constexpr NomMassFlowPerBeam(0.07); // nominal water mass flow rate per beam [kg/s]
    Real64 constexpr MinWaterVel(0.2);         // minimum water velocity [m/s]
    Real64 constexpr Coeff2(10000.0);

    struct CoolBeamData
    {
        // Members
        // input data
        std::string Name;                      // name of unit
        std::string UnitType;                  // type of unit = AirTerminal:SingleDuct:ConstantVolume:CooledBeam
        int UnitType_Num;                      // index to type of unit = 1 (there's only 1 type so far)
        std::string CBTypeString;              // type of cooled beam: active | passive
        CooledBeamType CBType;                 // index to type of cooled beam
        Sched::Schedule *availSched = nullptr; // availability schedule
        Real64 MaxAirVolFlow;                  // m3/s (autosizable)
        Real64 MaxAirMassFlow;                 // kg/s
        Real64 MaxCoolWaterVolFlow;            // m3/s
        Real64 MaxCoolWaterMassFlow;           // kg/s
        int AirInNode;                         // unit air inlet node number
        int AirOutNode;                        // unit air outlet node number
        int CWInNode;                          // chilled water inlet node
        int CWOutNode;                         // chilled water outlet node
        int ADUNum;                            // index of corresponding air distribution unit
        Real64 NumBeams;                       // number of beams in the zone
        Real64 BeamLength;                     // length of individual beam [m]
        Real64 DesInletWaterTemp;              // design inlet water temperature [C]
        Real64 DesOutletWaterTemp;             // design outlet water Temperature [c]
        Real64 CoilArea;                       // coil surface area per coil length [m2/m]
        Real64 a;                              // model parameter a
        Real64 n1;                             // model parameter n0
        Real64 n2;                             // model parameter n1
        Real64 n3;                             // model parameter n2
        Real64 a0;                             // model parameter a0
        Real64 K1;                             // model parameter K1
        Real64 n;                              // model parameter n
        Real64 Kin;                            // Coefficient of Induction Kin
        Real64 InDiam;                         // Leaving Pipe Inside Diameter
        // time step variables
        Real64 TWIn;                // current inlet water temperature [C]
        Real64 TWOut;               // current outlet water temperature [C]
        Real64 EnthWaterOut;        // current outlet water enthalpy [J/kg]
        Real64 BeamFlow;            // supply air flow per beam [m3/s]
        Real64 CoolWaterMassFlow;   // chilled water mass flow rate [kg/s]
        Real64 BeamCoolingEnergy;   // Cooled beam cooling energy of all beams in the zone [J]
        Real64 BeamCoolingRate;     // Cooled beam cooling rate of all beams in the zone [W]
        Real64 SupAirCoolingEnergy; // Total cooling energy from supply air [J]
        Real64 SupAirCoolingRate;   // Total cooling rate from supply air [W]
        Real64 SupAirHeatingEnergy; // Total cooling energy from supply air [J]
        Real64 SupAirHeatingRate;   // Total cooling rate from supply air [W]
        PlantLocation CWPlantLoc;   // cooling water plant loop component index
        int CBLoadReSimIndex;
        int CBMassFlowReSimIndex;
        int CBWaterOutletTempReSimIndex;
        int CtrlZoneNum;           // control zone index
        int ctrlZoneInNodeIndex;   // which controlled zone inlet node number corresponds with this unit
        int AirLoopNum;            // air loop index that terminal is attached to
        Real64 OutdoorAirFlowRate; // zone outdoor air volume flow rate
        bool MyEnvrnFlag;
        bool MySizeFlag;
        bool PlantLoopScanFlag;

        // Default Constructor
        CoolBeamData()
            : UnitType_Num(0), CBType(CooledBeamType::Invalid), MaxAirVolFlow(0.0), MaxAirMassFlow(0.0), MaxCoolWaterVolFlow(0.0),
              MaxCoolWaterMassFlow(0.0), AirInNode(0), AirOutNode(0), CWInNode(0), CWOutNode(0), ADUNum(0), NumBeams(0.0), BeamLength(0.0),
              DesInletWaterTemp(0.0), DesOutletWaterTemp(0.0), CoilArea(0.0), a(0.0), n1(0.0), n2(0.0), n3(0.0), a0(0.0), K1(0.0), n(0.0), Kin(0.0),
              InDiam(0.0), TWIn(0.0), TWOut(0.0), EnthWaterOut(0.0), BeamFlow(0.0), CoolWaterMassFlow(0.0), BeamCoolingEnergy(0.0),
              BeamCoolingRate(0.0), SupAirCoolingEnergy(0.0), SupAirCoolingRate(0.0), SupAirHeatingEnergy(0.0), SupAirHeatingRate(0.0), CWPlantLoc{},
              CBLoadReSimIndex(0), CBMassFlowReSimIndex(0), CBWaterOutletTempReSimIndex(0), CtrlZoneNum(0), ctrlZoneInNodeIndex(0), AirLoopNum(0),
              OutdoorAirFlowRate(0.0), MyEnvrnFlag(true), MySizeFlag(true), PlantLoopScanFlag(true)
        {
        }

        void CalcOutdoorAirVolumeFlowRate(EnergyPlusData &state);
        void reportTerminalUnit(EnergyPlusData &state);
    };

    void SimCoolBeam(EnergyPlusData &state,
                     std::string_view CompName, // name of the cooled beam unit
                     bool FirstHVACIteration,   // TRUE if first HVAC iteration in time step
                     int ZoneNum,               // index of zone served by the unit
                     int ZoneNodeNum,           // zone node number of zone served by the unit
                     int &CompIndex,            // which cooled beam unit in data structure
                     Real64 &NonAirSysOutput    // convective cooling by the beam system [W]
    );

    void GetCoolBeams(EnergyPlusData &state);

    void InitCoolBeam(EnergyPlusData &state,
                      int CBNum,              // number of the current cooled beam unit being simulated
                      bool FirstHVACIteration // TRUE if first air loop solution this HVAC step
    );

    void SizeCoolBeam(EnergyPlusData &state, int CBNum);

    void ControlCoolBeam(EnergyPlusData &state,
                         int CBNum,               // number of the current unit being simulated
                         int ZoneNum,             // number of zone being served
                         int ZoneNodeNum,         // zone node number
                         bool FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                         Real64 &NonAirSysOutput  // convective cooling by the beam system [W]
    );

    void CalcCoolBeam(EnergyPlusData &state,
                      int CBNum,       // Unit index
                      int ZoneNode,    // zone node number
                      Real64 CWFlow,   // cold water flow [kg/s]
                      Real64 &LoadMet, // load met by unit [W]
                      Real64 &TWOut    // chilled water outlet temperature [C]
    );

    void UpdateCoolBeam(EnergyPlusData &state, int CBNum);

    void ReportCoolBeam(EnergyPlusData &state, int CBNum);

} // namespace HVACCooledBeam

struct HVACCooledBeamData : BaseGlobalStruct
{
    Array1D_bool CheckEquipName;
    int NumCB;
    Array1D<HVACCooledBeam::CoolBeamData> CoolBeam;
    bool GetInputFlag = true;              // First time, input is "gotten"
    bool ZoneEquipmentListChecked = false; // True after the Zone Equipment List has been checked for items

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->CheckEquipName.clear();
        this->NumCB = 0;
        this->CoolBeam.clear();
        this->GetInputFlag = true;
        this->ZoneEquipmentListChecked = false;
    }
};

} // namespace EnergyPlus

#endif
