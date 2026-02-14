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

#ifndef DemandManager_hh_INCLUDED
#define DemandManager_hh_INCLUDED

// C++ Headers
#include <unordered_map>

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

namespace DemandManager {

    // MODULE PARAMETER DEFINITIONS:
    enum class ManagerType
    {
        Invalid = -1,
        ExtLights,
        Lights,
        ElecEquip,
        Thermostats,
        Ventilation,
        Num
    };

    enum class ManagePriorityType
    {
        Invalid = -1,
        Sequential,
        Optimal,
        All,
        Num
    };

    enum class ManagerLimit
    {
        Invalid = -1,
        Off,
        Fixed,
        Variable,
        ReductionRatio,
        Num
    };

    enum class ManagerSelection
    {
        Invalid = -1,
        All,
        Many,
        One,
        Num
    };

    enum class DemandAction
    {
        Invalid = -1,
        CheckCanReduce,
        SetLimit,
        ClearLimit,
        Num
    };

    // Types
    struct DemandManagerListData
    {
        // Members
        std::string Name;                        // Name of DEMAND MANAGER LIST
        int Meter;                               // Index to meter to demand limit
        Sched::Schedule *limitSched = nullptr;   // Schedule for demand limit
        Real64 SafetyFraction;                   // Multiplier applied to demand limit schedule
        Sched::Schedule *billingSched = nullptr; // Schedule for billing month periods
        Real64 BillingPeriod;                    // Current billing period value
        Sched::Schedule *peakSched = nullptr;    // Schedule index for billing month periods
        int AveragingWindow;                     // Number of timesteps for averaging demand window
        Array1D<Real64> History;                 // Demand window history
        ManagePriorityType ManagerPriority;      // Indicator for priority (SEQUENTIAL, OPTIMAL, ALL)
        int NumOfManager = 0;                    // Number of DEMAND MANAGERs
        Array1D_int Manager;                     // Indexes for DEMAND MANAGERs
        Real64 MeterDemand;                      // Meter demand at this timestep
        Real64 AverageDemand;                    // Current demand over the demand window
        Real64 PeakDemand;                       // Peak demand in the billing month so far
        Real64 ScheduledLimit;                   // Scheduled demand limit
        Real64 DemandLimit;                      // Scheduled demand limit * Safety Fraction
        Real64 AvoidedDemand;                    // Demand avoided by active DEMAND MANAGERs
        Real64 OverLimit;                        // Amount that demand limit is exceeded
        Real64 OverLimitDuration;                // Number of hours that demand limit is exceeded

        // Default Constructor
        DemandManagerListData()
            : Meter(0), SafetyFraction(1.0), BillingPeriod(0.0), AveragingWindow(1), ManagerPriority(ManagePriorityType::Invalid), MeterDemand(0.0),
              AverageDemand(0.0), PeakDemand(0.0), ScheduledLimit(0.0), DemandLimit(0.0), AvoidedDemand(0.0), OverLimit(0.0), OverLimitDuration(0.0)
        {
        }
    };

    struct DemandManagerData
    {
        // Members
        std::string Name;                      // Name of DEMAND MANAGER
        ManagerType Type;                      // Type of DEMAND MANAGER (:LIGHTS, :ELECTRICEQUIPMENT, etc.)
        int DemandManagerList;                 // Reference to parent DEMAND MANAGER LIST for error checking
        bool CanReduceDemand;                  // Flag to indicate whether manager can reduce demand
        Sched::Schedule *availSched = nullptr; // Availability Schedule
        bool Available;                        // Availability flag
        bool Activate;                         // Flag to activate the manager
        bool Active;                           // Flag to indicate that the manager is active
        ManagerLimit LimitControl;
        ManagerSelection SelectionControl;
        int LimitDuration;       // Minimum duration of demand manager activity (min)
        int ElapsedTime;         // Elapsed time for the demand manager activity (min)
        int RotationDuration;    // Rotation duration (min)
        int ElapsedRotationTime; // Elapsed time for the current rotation (min)
        int RotatedLoadNum;      // Index for rotated load
        Real64 LowerLimit;       // Lowest demand limit as fraction of design level
        // Lowest heating setpoint for thermostats
        Real64 UpperLimit; // Not used for demand limit
        // Highest cooling setpoint for thermostats
        int NumOfLoads;   // Number of load objects
        Array1D_int Load; // Pointers to load objects

        // Additional fields related to DemandManager:Ventilation
        Real64 FixedRate;      // m3 per person
        Real64 ReductionRatio; // % of reduction

        // Default Constructor
        DemandManagerData()
            : Type(ManagerType::Invalid), DemandManagerList(0), CanReduceDemand(false), Available(false), Activate(false), Active(false),
              LimitControl(ManagerLimit::Invalid), SelectionControl(ManagerSelection::Invalid), LimitDuration(0), ElapsedTime(0), RotationDuration(0),
              ElapsedRotationTime(0), RotatedLoadNum(0), LowerLimit(0.0), UpperLimit(0.0), NumOfLoads(0), FixedRate(0.0), ReductionRatio(0.0)
        {
        }
    };

    void ManageDemand(EnergyPlusData &state);

    void SimulateDemandManagerList(EnergyPlusData &state,
                                   int ListNum,
                                   bool &ResimExt, // Flag to resimulate the exterior energy use simulation
                                   bool &ResimHB,  // Flag to resimulate the heat balance simulation (including HVAC)
                                   bool &ResimHVAC // Flag to resimulate the HVAC simulation
    );

    void GetDemandManagerListInput(EnergyPlusData &state);

    void GetDemandManagerInput(EnergyPlusData &state);

    void SurveyDemandManagers(EnergyPlusData &state);

    void ActivateDemandManagers(EnergyPlusData &state);

    void UpdateDemandManagers(EnergyPlusData &state);

    void ReportDemandManagerList(EnergyPlusData &state, int ListNum);

    void LoadInterface(EnergyPlusData &state, DemandAction Action, int MgrNum, int LoadPtr, bool &CanReduceDemand);

    void InitDemandManagers(EnergyPlusData &state);

} // namespace DemandManager

struct DemandManagerData : BaseGlobalStruct
{
    int NumDemandManagerList = 0;
    int NumDemandMgr = 0;
    int DemandManagerExtIterations = 0;
    int DemandManagerHBIterations = 0;
    int DemandManagerHVACIterations = 0;
    bool GetInput = true; // Flag to prevent input from being read multiple times
    Array1D<DemandManager::DemandManagerListData> DemandManagerList;
    Array1D<DemandManager::DemandManagerData> DemandMgr;
    std::unordered_map<std::string, std::string> UniqueDemandMgrNames;
    bool ClearHistory = true;
    bool BeginDemandSim = true;
    bool ResimHVAC = true;
    bool ResimHB = true;
    bool ResimExt = true;
    bool firstTime = true;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        NumDemandManagerList = 0;
        NumDemandMgr = 0;
        DemandManagerExtIterations = 0;
        DemandManagerHBIterations = 0;
        DemandManagerHVACIterations = 0;
        GetInput = true;
        DemandManagerList.deallocate();
        DemandMgr.deallocate();
        UniqueDemandMgrNames.clear();
        ClearHistory = true;
        BeginDemandSim = true;
        ResimHVAC = true;
        ResimHB = true;
        ResimExt = true;
        firstTime = true;
    }
};

} // namespace EnergyPlus

#endif
