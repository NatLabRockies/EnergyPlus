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

#ifndef DataContaminantBalance_hh_INCLUDED
#define DataContaminantBalance_hh_INCLUDED

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

namespace DataContaminantBalance {

    struct ContaminantData
    {
        // Members
        bool SimulateContaminants = false;              // A logical flag to determine whether any contaminants are simulated or not
        bool CO2Simulation = false;                     // CO2 simulation flag
        Sched::Schedule *CO2OutdoorSched = nullptr;     // CO2 outdoor level schedule
        bool GenericContamSimulation = false;           // Generic contaminant simulation flag
        Sched::Schedule *genericOutdoorSched = nullptr; // Generic contaminant outdoor level schedule
    };

    struct ZoneContControls
    {
        // Members
        std::string Name;     // Name of the contaminant controller
        std::string ZoneName; // Name of the zone
        int ActualZoneNum = 0;
        Sched::Schedule *availSched = nullptr;      // Availability schedule
        Sched::Schedule *setptSched = nullptr;      // Setpoint schedule
        int NumOfZones = 0;                         // Number of controlled zones in the same airloop
        Array1D_int ControlZoneNum;                 // Controlled zone number
        Sched::Schedule *zoneMinCO2Sched = nullptr; // Minimum CO2 concentration schedule
        Sched::Schedule *zoneMaxCO2Sched = nullptr; // Maximum CO2 concentration schedule

        Sched::Schedule *zoneContamControllerSched = nullptr; // Index for this schedule
        Sched::Schedule *genericContamAvailSched = nullptr;   // Availability Schedule name for generic contamiant
        Sched::Schedule *genericContamSetptSched = nullptr;   // Schedule which determines the generic contaminant setpoint
    };

    struct ZoneSystemContaminantDemandData // Contaminent loads to be met (kg air per second)
    {
        // Members
        Real64 OutputRequiredToCO2SP = 0.0;     // Load required to meet CO2 setpoint
        Real64 RemainingOutputReqToCO2SP = 0.0; // Remaining load required to meet CO2 setpoint
        Real64 OutputRequiredToGCSP = 0.0;      // Load required to meet generic contaminant setpoint
        Real64 RemainingOutputReqToGCSP = 0.0;  // Remaining load required to meet generic contaminant setpoint
    };

    struct ZoneContamGenericDataConstant
    {
        // Members
        std::string Name;                             // Name of the constant generic contaminant source and sink
        std::string ZoneName;                         // Name of the zone
        int ActualZoneNum = 0;                        // Zone number
        Real64 GenerateRate = 0.0;                    // Generic contaminant design generation rate [m3/s]
        Sched::Schedule *generateRateSched = nullptr; // Generic contaminant design generation rate schedule pointer
        Real64 RemovalCoef = 0.0;                     // Generic contaminant design removal coefficient [m3/s]
        Sched::Schedule *removalCoefSched = nullptr;  // Generic contaminant design removal coefficient schedule pointer
        Real64 GenRate = 0.0;                         // Generic contaminant design generation rate [m3/s] for reporting
    };

    struct ZoneContamGenericDataPDriven
    {
        // Members
        std::string Name;                                 // Name of the pressure driven generic contaminant source and sink
        std::string SurfName;                             // Name of the surface
        int SurfNum = 0;                                  // Surface number
        Real64 GenRateCoef = 0.0;                         // Generic contaminant design generation rate coefficient [m3/s]
        Sched::Schedule *generateRateCoefSched = nullptr; // Generic contaminant design generation rate schedule
        Real64 Expo = 0.0;                                // Generic contaminant exponent []
        Real64 GenRate = 0.0;                             // Generic contaminant design generation rate [m3/s] for reporting
    };

    struct ZoneContamGenericDataCutoff
    {
        // Members
        std::string Name;                             // Name of the cutoff generic contaminant source and sink
        std::string ZoneName;                         // Name of the zone
        int ActualZoneNum = 0;                        // Zone number
        Real64 GenerateRate = 0.0;                    // Generic contaminant design generation rate [m3/s]
        Sched::Schedule *generateRateSched = nullptr; // Generic contaminant design generation rate schedule pointer
        Real64 CutoffValue = 0.0;                     // Cutoff value [ppm]
        Real64 GenRate = 0.0;                         // Generic contaminant design generation rate [m3/s] for reporting
    };

    struct ZoneContamGenericDataDecay
    {
        // Members
        std::string Name;                         // Name of the decay generic contaminant source and sink
        std::string ZoneName;                     // Name of the zone
        int ActualZoneNum = 0;                    // Zone number
        Real64 InitEmitRate = 0.0;                // Generic contaminant design generation rate [m3/s]
        Sched::Schedule *emitRateSched = nullptr; // Generic contaminant emission rate schedule pointer
        Real64 Time = 0.0;                        // Time since the styart of emission [s]
        Real64 DelayTime = 0.0;                   // Delay time constant [s]
        Real64 GenRate = 0.0;                     // Generic contaminant design generation rate [m3/s] for reporting
    };

    struct ZoneContamGenericDataBLDiff
    {
        // Members
        std::string Name; // Name of the boundary layer diffusion generic contaminant source
        // and sink
        std::string SurfName;                      // Name of the surface
        int SurfNum = 0;                           // Surface number
        Real64 TransCoef = 0.0;                    // Generic contaminant mass transfer coefficient [m/s]
        Sched::Schedule *transCoefSched = nullptr; // Generic contaminant mass transfer coefficient schedule pointer
        Real64 HenryCoef = 0.0;                    // Generic contaminant Henry adsorption constant or
        // partition coefficient []
        Real64 GenRate = 0.0; // Generic contaminant design generation rate [m3/s] for reporting
    };

    struct ZoneContamGenericDataDVS
    {
        // Members
        std::string Name;                         // Name of the deposition velocity generic contaminant sink
        std::string SurfName;                     // Name of the surface
        int SurfNum = 0;                          // Surface number
        Real64 DepoVelo = 0.0;                    // Generic contaminant deposition velocity [m/s]
        Sched::Schedule *depoVeloSched = nullptr; // Generic contaminant deposition velocity sink schedule pointer
        Real64 GenRate = 0.0;                     // Generic contaminant design generation rate [m3/s] for reporting
    };

    struct ZoneContamGenericDataDRS
    {
        // Members
        std::string Name;                         // Name of the deposition rate generic contaminant sink
        std::string ZoneName;                     // Name of the zone
        int ActualZoneNum = 0;                    // Zone number
        Real64 DepoRate = 0.0;                    // Generic contaminant deposition rate [m/s]
        Sched::Schedule *depoRateSched = nullptr; // Generic contaminant deposition rate sink schedule
        Real64 GenRate = 0.0;                     // Generic contaminant design generation rate [m3/s] for reporting
    };

} // namespace DataContaminantBalance

struct ContaminantBalanceData : BaseGlobalStruct
{
    Array1D<Real64> ZoneCO2SetPoint;
    Array1D<Real64> CO2PredictedRate;

    Array1D<Real64> ZoneCO2Gain;             // CO2 gain from each Zone (People, equipment)
    Array1D<Real64> ZoneCO2GainFromPeople;   // CO2 gain from each Zone (From People only)
    Array1D<Real64> ZoneCO2GainExceptPeople; // Added for hybrid model, CO2 gain from each Zone (except People)

    // Zone Air Contaminant conditions variables
    Array1D<Real64> ZoneAirCO2Avg;       // AIR CO2 averaged over the zone time step
    Array1D<Real64> ZoneAirCO2;          // AIR CO2
    Array1D<Real64> CO2ZoneTimeMinus1;   // CO2 history terms for 3rd order derivative
    Array1D<Real64> CO2ZoneTimeMinus2;   // Time Minus 2 Zone Time Steps Term
    Array1D<Real64> CO2ZoneTimeMinus3;   // Time Minus 3 Zone Time Steps Term
    Array1D<Real64> CO2ZoneTimeMinus4;   // Time Minus 4 Zone Time Steps Term
    Array1D<Real64> DSCO2ZoneTimeMinus1; // DownStepped CO2 history terms for 3rd order derivative
    Array1D<Real64> DSCO2ZoneTimeMinus2; // DownStepped Time Minus 2 Zone Time Steps Term
    Array1D<Real64> DSCO2ZoneTimeMinus3; // DownStepped Time Minus 3 Zone Time Steps Term
    Array1D<Real64> DSCO2ZoneTimeMinus4; // DownStepped Time Minus 4 Zone Time Steps Term

    Array1D<Real64> ZoneAirCO2Temp;        // Temp zone air CO2 at time plus 1
    Array1D<Real64> CO2ZoneTimeMinus1Temp; // Zone air CO2 at previous timestep
    Array1D<Real64> CO2ZoneTimeMinus2Temp; // Zone air CO2 at timestep T-2
    Array1D<Real64> CO2ZoneTimeMinus3Temp; // Zone air CO2 at timestep T-3
    Array1D<Real64> ZoneAirCO2Old;         // Last Time Steps Zone AIR Humidity Ratio

    Array1D<Real64> ZoneCO2MX; // TEMPORARY ZONE CO2 TO TEST CONVERGENCE in Exact and Euler method
    Array1D<Real64> ZoneCO2M2; // TEMPORARY ZONE CO2 at timestep t-2 in Exact and Euler method
    Array1D<Real64> ZoneCO21;  // Zone CO2 at the previous time step used in Exact and Euler method

    Array1D<Real64> CONTRAT;           // Zone CO2 at the previous time step used in Exact and Euler method
    Array1D<Real64> MixingMassFlowCO2; // Mixing MASS FLOW * CO2
    Real64 OutdoorCO2 = 0.0;           // Outdoor CO2 level

    Array1D<Real64> ZoneAirDensityCO; // Mixing MASS FLOW * CO2
    Array1D<Real64> AZ;
    Array1D<Real64> BZ;
    Array1D<Real64> CZ;

    Array1D<Real64> ZoneGCSetPoint;
    Array1D<Real64> GCPredictedRate;

    Array1D<Real64> ZoneGCGain; // Generic contaminant gain from each Zone (People, equipment)

    // Zone Air Contaminant conditions variables
    Array1D<Real64> ZoneAirGCAvg;       // AIR generic contaminant averaged over the zone time step
    Array1D<Real64> ZoneAirGC;          // AIR generic contaminant
    Array1D<Real64> GCZoneTimeMinus1;   // Generic contaminant history terms for 3rd order derivative
    Array1D<Real64> GCZoneTimeMinus2;   // Time Minus 2 Zone Time Steps Term
    Array1D<Real64> GCZoneTimeMinus3;   // Time Minus 3 Zone Time Steps Term
    Array1D<Real64> GCZoneTimeMinus4;   // Time Minus 4 Zone Time Steps Term
    Array1D<Real64> DSGCZoneTimeMinus1; // DownStepped generic contaminant history terms for 3rd order
    // derivative
    Array1D<Real64> DSGCZoneTimeMinus2; // DownStepped Time Minus 2 Zone Time Steps Term
    Array1D<Real64> DSGCZoneTimeMinus3; // DownStepped Time Minus 3 Zone Time Steps Term
    Array1D<Real64> DSGCZoneTimeMinus4; // DownStepped Time Minus 4 Zone Time Steps Term

    Array1D<Real64> ZoneAirGCTemp;        // Temp zone air generic contaminant at time plus 1
    Array1D<Real64> GCZoneTimeMinus1Temp; // Zone air generic contaminant at previous timestep
    Array1D<Real64> GCZoneTimeMinus2Temp; // Zone air generic contaminant at timestep T-2
    Array1D<Real64> GCZoneTimeMinus3Temp; // Zone air generic contaminant at timestep T-3
    Array1D<Real64> ZoneAirGCOld;         // Last Time Steps Zone AIR generic contaminant

    Array1D<Real64> ZoneGCMX; // TEMPORARY ZONE CO2 TO TEST CONVERGENCE in Exact and Euler method
    Array1D<Real64> ZoneGCM2; // TEMPORARY ZONE CO2 at timestep t-2 in Exact and Euler method
    Array1D<Real64> ZoneGC1;  // Zone CO2 at the previous time step used in Exact and Euler method

    Array1D<Real64> CONTRATGC; // Zone generic contaminant at the previous time step used in
    // Exact and Euler method

    Array1D<Real64> MixingMassFlowGC; // Mixing MASS FLOW * generic contaminant

    Real64 OutdoorGC = 0.0; // Outdoor generic contaminant level

    Array1D<Real64> ZoneAirDensityGC; // Mixing MASS FLOW * generic contaminant
    Array1D<Real64> AZGC;
    Array1D<Real64> BZGC;
    Array1D<Real64> CZGC;

    Array1D<DataContaminantBalance::ZoneSystemContaminantDemandData> ZoneSysContDemand;
    DataContaminantBalance::ContaminantData
        Contaminant; // A logical flag to determine whether any contaminants are simulated or not | CO2 simulation flag | CO2 outdoor
    // level schedule pointer | Generic contaminant simulation flag | Generic contaminant outdoor level schedule pointer
    Array1D<DataContaminantBalance::ZoneContControls> ContaminantControlledZone;
    Array1D<DataContaminantBalance::ZoneContamGenericDataConstant> ZoneContamGenericConstant;
    Array1D<DataContaminantBalance::ZoneContamGenericDataPDriven> ZoneContamGenericPDriven;
    Array1D<DataContaminantBalance::ZoneContamGenericDataCutoff> ZoneContamGenericCutoff;
    Array1D<DataContaminantBalance::ZoneContamGenericDataDecay> ZoneContamGenericDecay;
    Array1D<DataContaminantBalance::ZoneContamGenericDataBLDiff> ZoneContamGenericBLDiff;
    Array1D<DataContaminantBalance::ZoneContamGenericDataDVS> ZoneContamGenericDVS;
    Array1D<DataContaminantBalance::ZoneContamGenericDataDRS> ZoneContamGenericDRS;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) ContaminantBalanceData();
    }
};

} // namespace EnergyPlus

#endif
