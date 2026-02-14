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

#ifndef CoolTower_hh_INCLUDED
#define CoolTower_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace CoolTower {

    enum class FlowCtrl
    {
        Invalid = -1,
        FlowSchedule,
        WindDriven,
        Num
    };

    enum class WaterSupplyMode
    {
        Invalid = -1,
        FromMains,
        FromTank,
        Num
    };

    struct CoolTowerParams
    {
        // Members
        std::string Name;                                                  // The component name
        std::string CompType;                                              // Type of component
        Sched::Schedule *availSched = nullptr;                             // schedule
        int ZonePtr = 0;                                                   // Index to zone
        int spacePtr = 0;                                                  // Index to space (if applicable)
        Sched::Schedule *pumpSched = nullptr;                              // Index to schedule for water pump
        FlowCtrl FlowCtrlType = FlowCtrl::Invalid;                         // Type of cooltower operation
        WaterSupplyMode CoolTWaterSupplyMode = WaterSupplyMode::FromMains; // Type of water source
        std::string CoolTWaterSupplyName;                                  // Name of water source
        int CoolTWaterSupTankID;                                           // Index to water storage tank
        int CoolTWaterTankDemandARRID;                                     // Index to water storage demand
        Real64 TowerHeight = 0.0;                                          // Effective cooltower height in m
        Real64 OutletArea = 0.0;                                           // Outlet area where conditioned air comes in m2
        Real64 OutletVelocity = 0.0;                                       // Outlet velocity of the cooltower in m/s
        Real64 MaxAirVolFlowRate = 0.0;                                    // Maximum allowable airflow in m3/s
        Real64 AirMassFlowRate = 0.0;                                      // Air mass flow rate in kg/s
        Real64 CoolTAirMass = 0.0;                                         // Air mass in kg
        Real64 MinZoneTemp = 0.0;                                          // Lower temperature limit to prevent over cooling in C
        Real64 FracWaterLoss = 0.0;                                        // Fraction of estimated blowdown and drift water
        Real64 FracFlowSched = 0.0;                                        // Fraction of airflow loss
        Real64 MaxWaterFlowRate = 0.0;                                     // Maximum limit of water flow rate in m3/s
        Real64 ActualWaterFlowRate = 0.0;                                  // Actual water mass flow rate in m3/s
        Real64 RatedPumpPower = 0.0;                                       // Rated power consumption for water pump serving the cooltower in watts
        Real64 SenHeatLoss = 0.0;                                          // Sensible heat loss in Joules
        Real64 SenHeatPower = 0.0;                                         // Sensible heat loss rate in watts
        Real64 LatHeatLoss = 0.0;                                          // Latent heat loss in Joules
        Real64 LatHeatPower = 0.0;                                         // Latent heat loss rate in watts
        Real64 AirVolFlowRate = 0.0;                                       // Air flow rate in m3/s
        Real64 AirVolFlowRateStd = 0.0;                                    // Air flow rate in m3/s at standard conditions
        Real64 CoolTAirVol = 0.0;                                          // Air volume in m3
        Real64 ActualAirVolFlowRate = 0.0;                                 // Actual air flow rate in m3/s
        Real64 InletDBTemp = 0.0;                                          // Outdoor dry bulb temperature in C
        Real64 InletWBTemp = 0.0;                                          // Outdoor wet bulb temperature in C
        Real64 InletHumRat = 0.0;                                          // Outdoor humidity ratio
        Real64 OutletTemp = 0.0;                                           // Dry bulb temperature at cooltower exit in C
        Real64 OutletHumRat = 0.0;                                         // Humidity ratio at cooltower exit
        Real64 CoolTWaterConsumpRate = 0.0;                                // Total water consumption during the processes in m3/s
        Real64 CoolTWaterStarvMakeupRate = 0.0;                            // Water provided from the mains (m3/s)
        Real64 CoolTWaterStarvMakeup = 0.0;                                // Water provided from the mains
        Real64 CoolTWaterConsump = 0.0;                                    // Total water consumption in m3
        Real64 PumpElecPower = 0.0;                                        // Pump power in watts
        Real64 PumpElecConsump = 0.0;                                      // Pump energy consumption in Joules
    };

    void ManageCoolTower(EnergyPlusData &state);

    void GetCoolTower(EnergyPlusData &state);

    void CalcCoolTower(EnergyPlusData &state);

    void UpdateCoolTower(EnergyPlusData &state);

    void ReportCoolTower(EnergyPlusData &state);

} // namespace CoolTower

struct CoolTowerData : BaseGlobalStruct
{

    bool GetInputFlag = true;
    Array1D<CoolTower::CoolTowerParams> CoolTowerSys;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) CoolTowerData();
    }
};

} // namespace EnergyPlus

#endif
