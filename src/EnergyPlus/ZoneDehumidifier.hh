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

#ifndef ZoneDehumidifier_hh_INCLUDED
#define ZoneDehumidifier_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Curve {
    struct Curve;
}

namespace ZoneDehumidifier {

    enum class CondensateOutlet
    { // Water Systems
        Invalid = -1,
        Discarded, // Default mode where water is "lost"
        ToTank,    // Collect coil condensate from air and store in water storage tank
        Num
    };

    struct ZoneDehumidifierParams
    {
        // input data and others required during calculations
        std::string Name;                                                     // Name of unit
        std::string UnitType;                                                 // Type of unit
        Sched::Schedule *availSched = nullptr;                                // availability schedule
        Real64 RatedWaterRemoval = 0.0;                                       // Rated water removal [liters/day]
        Real64 RatedEnergyFactor = 0.0;                                       // Rated energy factor [liters/kWh]
        Real64 RatedAirVolFlow = 0.0;                                         // Rated air flow rate through the dehumidifier [m3/s]
        Real64 RatedAirMassFlow = 0.0;                                        // Rated air mass flow rate through the dehumidifier [kg/s]
        Real64 MinInletAirTemp = 0.0;                                         // Minimum dry-bulb temperature for dehumidifier operation [C]
        Real64 MaxInletAirTemp = 0.0;                                         // Maximum dry-bulb temperature for dehumidifier operation [C]
        Real64 InletAirMassFlow = 0.0;                                        // Inlet air mass flow rate for the time step being simulated [kg/s]
        Real64 OutletAirEnthalpy = 0.0;                                       // Dehumidifier outlet air enthalpy [J/kg]
        Real64 OutletAirHumRat = 0.0;                                         // Dehumidifier outlet air humidity ratio [kg/kg]
        Real64 OffCycleParasiticLoad = 0.0;                                   // Off Cycle Parasitic Load, user input [W]
        int AirInletNodeNum = 0;                                              // Inlet air node number
        int AirOutletNodeNum = 0;                                             // Outlet air node number
        Curve::Curve *WaterRemovalCurve = nullptr;                            // Water removal curve
        int WaterRemovalCurveErrorCount = 0;                                  // Count number of times water removal curve returns a negative value
        int WaterRemovalCurveErrorIndex = 0;                                  // Index for negative value water removal factor recurring messages
        Curve::Curve *EnergyFactorCurve = nullptr;                            // Energy factor curve
        int EnergyFactorCurveErrorCount = 0;                                  // Count number of times energy factor curve returns negative value
        int EnergyFactorCurveErrorIndex = 0;                                  // Index for negative value energy factor recurring messages
        Curve::Curve *PartLoadCurve = nullptr;                                // Index for part load curve
        int LowPLFErrorCount = 0;                                             // Count number of times PLF < 0.7
        int LowPLFErrorIndex = 0;                                             // Index for PLF < 0.7 recurring warning messages
        int HighPLFErrorCount = 0;                                            // Count number of times PLF > 1.0
        int HighPLFErrorIndex = 0;                                            // Index for PLF > 1.0 recurring warning messages
        int HighRTFErrorCount = 0;                                            // Count number of times RTF > 1.0
        int HighRTFErrorIndex = 0;                                            // Index for RTF > 1.0 recurring warning messages
        int PLFPLRErrorCount = 0;                                             // Count number of times PLF < PLR
        int PLFPLRErrorIndex = 0;                                             // Index for PLF < PLR recurring warning messages
        CondensateOutlet CondensateCollectMode = CondensateOutlet::Discarded; // Where does water come from
        std::string CondensateCollectName;                                    // Name of water storage (collection) tank
        int CondensateTankID = 0;                                             // Condensate collection tank ID number
        int CondensateTankSupplyARRID = 0;                                    // Condensate collection tank supply ID number
        // Report data
        Real64 SensHeatingRate = 0.0;            // Zone Dehumidifier Sensible Heating Rate [W]
        Real64 SensHeatingEnergy = 0.0;          // Zone Dehumidifier Sensible Heating Energy [J]
        Real64 WaterRemovalRate = 0.0;           // Zone Dehumidifier Water Removal Rate [kg/s]
        Real64 WaterRemoved = 0.0;               // Zone Dehumidifier Water Removed [kg]
        Real64 ElecPower = 0.0;                  // Zone Dehumidifier Electric Power [W]
        Real64 ElecConsumption = 0.0;            // Zone Dehumidifier Electric Consumption [J]
        Real64 DehumidPLR = 0.0;                 // Zone Dehumidifier Part-Load Ratio [-]
        Real64 DehumidRTF = 0.0;                 // Zone Dehumidifier Runtime Fraction [-]
        Real64 DehumidCondVolFlowRate = 0.0;     // Zone Dehumidifier Condensate Volumetric Flow Rate [m3/s]
        Real64 DehumidCondVol = 0.0;             // Zone Dehumidifier Condensate Volume [m3]
        Real64 OutletAirTemp = 0.0;              // Zone Dehumidifier Outlet Air Temperature [C]
        Real64 OffCycleParasiticElecPower = 0.0; // Zone Dehumidifier Off-Cycle Parasitic Electric Power [W]
        Real64 OffCycleParasiticElecCons = 0.0;  // Zone Dehumidifier Off-Cycle Parasitic Electric Consumption [J]
        bool MyEnvrnFlag = true;
        bool MySetPointCheckFlag = true;
        bool CheckEquipName = true;
        bool ZoneEquipmentListChecked = false;
    };

    void SimZoneDehumidifier(EnergyPlusData &state,
                             std::string const &CompName, // Name of the zone dehumidifier
                             int ZoneNum,                 // Number of zone being served
                             bool FirstHVACIteration,     // TRUE if 1st HVAC simulation of system timestep
                             Real64 &QSensOut,            // Sensible capacity delivered to zone (W)
                             Real64 &QLatOut,             // Latent capacity delivered to zone (kg/s), dehumidify = negative
                             int &CompIndex               // Index to the zone dehumidifier
    );

    void GetZoneDehumidifierInput(EnergyPlusData &state);

    void InitZoneDehumidifier(EnergyPlusData &state, int ZoneDehumNum, int ZoneNum); // Number of the current zone dehumidifier and serving zone

    void CalcZoneDehumidifier(EnergyPlusData &state,
                              int ZoneDehumNum,       // Index number of the current zone dehumidifier being simulated
                              Real64 QZnDehumidReq,   // Dehumidification load to be met (kg/s), negative value means dehumidification load
                              Real64 &SensibleOutput, // Sensible (heating) output (W), sent to load predictor for next simulation time step
                              Real64 &LatentOutput    // Latent (dehumidification) output provided (kg/s)
    );

    void UpdateZoneDehumidifier(EnergyPlusData &state, int ZoneDehumNum); // Number of the current zone dehumidifier being simulated

    void ReportZoneDehumidifier(EnergyPlusData &state, int DehumidNum); // Index of the current zone dehumidifier being simulated

    bool GetZoneDehumidifierNodeNumber(EnergyPlusData &state, int NodeNumber); // Node being tested

    int getZoneDehumidifierIndex(EnergyPlusData &state, std::string_view CompName);

} // namespace ZoneDehumidifier

struct ZoneDehumidifierData : BaseGlobalStruct
{
    bool GetInputFlag = true; // Set to FALSE after first time input is "gotten"
    EPVector<ZoneDehumidifier::ZoneDehumidifierParams> ZoneDehumid;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) ZoneDehumidifierData();
    }
};

} // namespace EnergyPlus

#endif
