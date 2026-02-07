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

#ifndef DataZoneEnergyDemands_hh_INCLUDED
#define DataZoneEnergyDemands_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace DataZoneEnergyDemands {

    struct ZoneSystemDemandData // Sensible cooling/heating loads to be met (watts) or Humidification/dehumidification loads to be met (kg water per
                                // second)
    {
        Real64 RemainingOutputRequired = 0.0;      // The load each equipment sees as what is remaining with load fractions applied [W] (multiplied)
        Real64 UnadjRemainingOutputRequired = 0.0; // The total unadjusted load remaining to be met [W] (multiplied)
        Real64 TotalOutputRequired = 0.0;          // The total output required from all equipment [W] (multiplied)
        int NumZoneEquipment = 0;                  // count of zone equipment for this zone, from ZoneHVAC:EquipmentList
        Real64 SupplyAirAdjustFactor = 1.0;        // supply air adjustment factor due to the cap of
        // zone maximum outdoor air fraction
        int StageNum = 0; // The stage number when staged thermostate is used:
        // 0 no load, >0 Heating stage, <0 Cooling stage

        virtual ~ZoneSystemDemandData() = default;

        virtual void beginEnvironmentInit() = 0;

        virtual void setUpOutputVars(
            EnergyPlusData &state, std::string_view prefix, std::string const &name, bool staged, bool attachMeters, int zoneMult, int listMult) = 0;
    };

    struct ZoneSystemSensibleDemand : ZoneSystemDemandData // Sensible cooling/heating loads to be met (watts)
    {
        Real64 OutputRequiredToHeatingSP = 0.0; // Load required to meet heating setpoint (>0 is a heating load) [W] (multiplied)
        Real64 OutputRequiredToCoolingSP = 0.0; // Load required to meet cooling setpoint (<0 is a cooling load) [W] (multiplied)
        Real64 RemainingOutputReqToHeatSP =
            0.0; // Remaining load required to meet heating setpoint with load fractions applied (>0 is a heating load) [W] (multiplied)
        Real64 RemainingOutputReqToCoolSP =
            0.0; // Remaining load required to meet cooling setpoint with load fractions applied (<0 is a cooling load) [W] (multiplied)
        Real64 UnadjRemainingOutputReqToHeatSP =
            0.0; // Remaining unadjusted load required to meet heating setpoint (>0 is a heating load) [W] (multiplied)
        Real64 UnadjRemainingOutputReqToCoolSP =
            0.0; // Remaining unadjusted load required to meet cooling setpoint (<0 is a cooling load) [W] (multiplied)
        EPVector<Real64> SequencedOutputRequired;            // load required to meet setpoint by sequence [W] (multiplied)
        EPVector<Real64> SequencedOutputRequiredToHeatingSP; // load required to meet heating setpoint by sequence [W] (multiplied)
        EPVector<Real64> SequencedOutputRequiredToCoolingSP; // load required to meet cooling setpoint by sequence [W] (multiplied)
        Real64 predictedRate = 0.0;                          // Predicted sensible load [W] (unmultiplied)
        Real64 predictedHSPRate = 0.0;                       // Predicted sensible load to heating setpoint [W] (unmultiplied)
        Real64 predictedCSPRate = 0.0;                       // Predicted sensible load to cooling setpoint [W] (unmultiplied)
        Real64 airSysHeatRate = 0.0;                         // sensible heating rate [W] (unmultiplied)
        Real64 airSysCoolRate = 0.0;                         // sensible cooling rate [W] (unmultiplied)
        Real64 airSysHeatEnergy = 0.0;                       // sensible heating energy [J] (unmultiplied)
        Real64 airSysCoolEnergy = 0.0;                       // sensible cooling energy [J] (unmultiplied)

        void beginEnvironmentInit() override;

        void setUpOutputVars(EnergyPlusData &state,
                             std::string_view prefix,
                             std::string const &name,
                             bool staged,
                             bool attachMeters,
                             int zoneMult,
                             int listMult) override;

        void reportZoneAirSystemSensibleLoads(EnergyPlusData &state, Real64 SNLoad);

        void reportSensibleLoadsZoneMultiplier(
            EnergyPlusData &state, int zoneNum, Real64 totalLoad, Real64 loadToHeatingSetPoint, Real64 loadToCoolingSetPoint);
    };
    struct ZoneSystemMoistureDemand : ZoneSystemDemandData // Humidification/dehumidification loads to be met (kg water per second)
    {
        Real64 OutputRequiredToHumidifyingSP = 0.0; // Load required to meet humidifying setpoint (>0 = a humidify load) [kgWater/s] (multiplied)
        Real64 OutputRequiredToDehumidifyingSP =
            0.0;                                  // Load required to meet dehumidifying setpoint (<0 = a dehumidify load) [kgWater/s] (multiplied)
        Real64 RemainingOutputReqToHumidSP = 0.0; // Remaining load required to meet humidifying setpoint with load fractions applied
        // (>0 is a humidify load) [kgWater/s] (multiplied)
        Real64 RemainingOutputReqToDehumidSP = 0.0; // Remaining load required to meet dehumidifying setpoint with load fractions applied
        // (<0 is a dehumidify load) [kgWater/s] (multiplied)
        Real64 UnadjRemainingOutputReqToHumidSP = 0.0; // Remaining unadjusted load required to meet humidifying setpoint
        // (>0 is a humidify load) [kgWater/s] (multiplied)
        Real64 UnadjRemainingOutputReqToDehumidSP = 0.0; // Remaining unadjusted load required to meet dehumidifying setpoint
        // (<0 is a dehumidify load) [kgWater/s] (multiplied)
        EPVector<Real64> SequencedOutputRequired;            // load required to meet setpoint by sequence [kgWater/s] (multiplied)
        EPVector<Real64> SequencedOutputRequiredToHumidSP;   // load required to meet humidify setpoint by sequence [kgWater/s] (multiplied)
        EPVector<Real64> SequencedOutputRequiredToDehumidSP; // load required to meet dehumidify setpoint by sequence [kgWater/s] (multiplied)
        Real64 predictedRate = 0.0;                          // Predicted moisture load to setpoint [kgWater/s] (unmultiplied)
        Real64 predictedHumSPRate = 0.0;                     // Predicted latent load to humidification setpoint [kgWater/s] (unmultiplied)
        Real64 predictedDehumSPRate = 0.0;                   // Predicted latent load to dehumidification setpoint [kgWater/s] (unmultiplied)
        Real64 airSysHeatRate = 0.0;                         // air system latent heating rate [W] (unmultiplied)
        Real64 airSysCoolRate = 0.0;                         // air system latent cooling rate [W] (unmultiplied)
        Real64 airSysHeatEnergy = 0.0;                       // air system latent heating energy [J] (unmultiplied)
        Real64 airSysCoolEnergy = 0.0;                       // latent cooling energy [J] (unmultiplied)
        Real64 airSysSensibleHeatRatio = 0.0;                // air system SHR []
        Real64 vaporPressureDifference = 0.0;                // vapor pressure depression [Pa]

        void beginEnvironmentInit() override;

        void setUpOutputVars(EnergyPlusData &state,
                             std::string_view prefix,
                             std::string const &name,
                             [[maybe_unused]] bool staged = false,
                             [[maybe_unused]] bool attachMeters = false,
                             [[maybe_unused]] int zoneMult = 0,
                             [[maybe_unused]] int listMult = 0) override;

        void reportZoneAirSystemMoistureLoads(EnergyPlusData &state, Real64 latentGain, Real64 sensibleLoad, Real64 vaporPressureDiff);

        void reportMoistLoadsZoneMultiplier(
            EnergyPlusData &state, int zoneNum, Real64 totalLoad, Real64 loadToHumidifySetPoint, Real64 loadToDehumidifySetPoint);
    };

} // namespace DataZoneEnergyDemands

struct DataZoneEnergyDemandsData : BaseGlobalStruct
{
    Array1D_bool DeadBandOrSetback;    // true if zone temperature is in the thermostat deadband before any heating / cooling done
    Array1D_bool Setback;              // true if zone temperature has increased from previous setting
    Array1D_bool CurDeadBandOrSetback; // same as above except updated after each piece of zone equipment in a zone is simulated
    EPVector<DataZoneEnergyDemands::ZoneSystemSensibleDemand> ZoneSysEnergyDemand;
    EPVector<DataZoneEnergyDemands::ZoneSystemMoistureDemand> ZoneSysMoistureDemand;
    EPVector<DataZoneEnergyDemands::ZoneSystemSensibleDemand> spaceSysEnergyDemand;
    EPVector<DataZoneEnergyDemands::ZoneSystemMoistureDemand> spaceSysMoistureDemand;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) DataZoneEnergyDemandsData();
    }
};

} // namespace EnergyPlus

#endif
