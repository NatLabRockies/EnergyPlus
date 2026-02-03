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

#ifndef DataHeatBalFanSys_hh_INCLUDED
#define DataHeatBalFanSys_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace DataHeatBalFanSys {

    // Data
    // -only module should be available to other modules and routines.
    // Thus, all variables in this module must be PUBLIC.

    constexpr Real64 MaxRadHeatFlux = 4000.0; // [W/m2] max limit for radiant heat flux at a surface due to HVAC equipment

    enum class PredictorCorrectorCtrl
    {
        Invalid = -1,
        GetZoneSetPoints,
        PredictStep,
        CorrectStep,
        RevertZoneTimestepHistories,
        PushZoneTimestepHistories,
        PushSystemTimestepHistories,
        Num
    };

    struct ZoneComfortControlsFangerData
    {
        // Members
        int FangerType = 0;      // Index for Fanger type
        Real64 LowPMV = 0.0;     // Low PMV value
        Real64 HighPMV = 0.0;    // High PMV Value
        int DualPMVErrCount = 0; // Dual PMV setpoint error count
        int DualPMVErrIndex = 0; // Dual PMV setpoint error index
    };

    struct SurfQRadFromHVACData
    {
        Real64 HTRadSys;       // Current radiant heat flux to surface from high temperature radiant heaters
        Real64 HWBaseboard;    // Current radiant heat flux to surface from hot water baseboard heaters
        Real64 SteamBaseboard; // Current radiant heat flux to surface from steam baseboard heaters
        Real64 ElecBaseboard;  // Current radiant heat flux to surface from electric baseboard heaters
        Real64 CoolingPanel;   // Current radiant heat flux to surface from simple cooling panels
    };

    struct ZoneTstatSetpt
    {
        Real64 setpt = 0.0;
        Real64 setptAdapComfortCool = 0.0;
        Real64 setptHi = 0.0;
        Real64 setptLo = 0.0;
        Real64 setptHiAver = 0.0;
        Real64 setptLoAver = 0.0;
    };

} // namespace DataHeatBalFanSys

struct HeatBalFanSysData : BaseGlobalStruct
{
    Array1D<Real64> SumConvHTRadSys;             // Sum of convection to zone air from hi temp radiant heaters
    Array1D<Real64> SumLatentHTRadSys;           // Sum of latent gains from hi temp radiant heaters
    Array1D<Real64> SumConvPool;                 // Sum of convection to zone air from pools
    Array1D<Real64> SumLatentPool;               // Sum of latent gains from pools
    Array1D<Real64> ZoneQdotRadHVACToPerson;     // Sum of radiant gains to people from all radiant HVAC sources
    Array1D<Real64> ZoneQHTRadSysToPerson;       // Sum of radiant gains to people from hi temp radiant heaters
    Array1D<Real64> ZoneQHWBaseboardToPerson;    // Sum of radiant gains to people from hot water baseboard heaters
    Array1D<Real64> ZoneQSteamBaseboardToPerson; // Sum of radiant gains to people from steam baseboard heaters
    Array1D<Real64> ZoneQElecBaseboardToPerson;  // Sum of radiant gains to people from electric baseboard heaters
    Array1D<Real64> ZoneQCoolingPanelToPerson;   // Sum of radiant losses to people from cooling panels

    // Zone air drybulb conditions variables
    Array1D<Real64> TempTstatAir; // temperature of air near the thermo stat

    Array1D_bool ZoneMassBalanceFlag;  // zone mass flow balance flag
    Array1D_bool ZoneInfiltrationFlag; // Zone Infiltration flag
    Array1D_int ZoneReOrder;           // zone number reordered for zone mass balance

    // REAL Variables for the Heat Balance Simulation

    Array1D<Real64> QRadSysSource;     // Current source/sink for a particular surface (radiant sys)
    Array1D<Real64> TCondFDSourceNode; // Temperature of source/sink location in surface from CondFD algo
    Array1D<Real64> QPVSysSource;      // Current source/sink for a surface (integrated PV sys)

    Array1D<Real64> CTFTsrcConstPart; // Constant Outside Portion of the CTF calculation of
    // temperature at source
    Array1D<Real64> CTFTuserConstPart; // Constant Outside Portion of the CTF calculation of
    // temperature at the user specified location
    EPVector<DataHeatBalFanSys::SurfQRadFromHVACData> surfQRadFromHVAC; // Radiant heat flux to surface from radiant HVAC equipment
    Array1D<Real64> QRadSurfAFNDuct;                                    // Current radiant heat flux at a surface due to radiation from AFN ducts
    Array1D<Real64> QPoolSurfNumerator;  // Current pool heat flux impact at the surface (numerator of surface heat balance)
    Array1D<Real64> PoolHeatTransCoefs;  // Current pool heat transfer coefficients (denominator of surface heat balance)
    Array1D<Real64> RadSysTiHBConstCoef; // Inside heat balance coefficient that is constant
    Array1D<Real64> RadSysTiHBToutCoef;  // Inside heat balance coefficient that modifies Toutside
    Array1D<Real64> RadSysTiHBQsrcCoef;  // Inside heat balance coefficient that modifies source/sink
    Array1D<Real64> RadSysToHBConstCoef; // Outside heat balance coefficient that is constant
    Array1D<Real64> RadSysToHBTinCoef;   // Outside heat balance coefficient that modifies Toutside
    Array1D<Real64> RadSysToHBQsrcCoef;  // Outside heat balance coefficient that modifies source/sink

    Array1D<DataHeatBalFanSys::ZoneTstatSetpt> zoneTstatSetpts;

    EPVector<Real64> LoadCorrectionFactor; // PH 3/3/04

    // Hybrid Modeling
    Array1D<Real64> PreviousMeasuredZT1;     // Measured zone air temperature at previous timestep1
    Array1D<Real64> PreviousMeasuredZT2;     // Measured zone air temperature at previous timestep2
    Array1D<Real64> PreviousMeasuredZT3;     // Measured zone air temperature at previous timestep3
    Array1D<Real64> PreviousMeasuredHumRat1; // Hybrid model zone humidity ratio at previous timestep
    Array1D<Real64> PreviousMeasuredHumRat2; // Hybrid model zone humidity ratio at previous timestep
    Array1D<Real64> PreviousMeasuredHumRat3; // Hybrid model zone humidity ratio at previous timestep
    EPVector<HVAC::SetptType> TempControlType;
    EPVector<int> TempControlTypeRpt;
    EPVector<HVAC::SetptType> ComfortControlType;
    EPVector<int> ComfortControlTypeRpt;

    Array2D<bool> CrossedColdThreshRepPeriod;
    Array2D<bool> CrossedHeatThreshRepPeriod;

    Array2D<std::vector<Real64>> ZoneHeatIndexHourBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneHeatIndexOccuHourBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneHeatIndexOccupiedHourBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneHumidexHourBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneHumidexOccuHourBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneHumidexOccupiedHourBinsRepPeriod;
    Array2D<Real64> lowSETLongestHoursRepPeriod;
    Array2D<Real64> highSETLongestHoursRepPeriod;
    Array2D<int> lowSETLongestStartRepPeriod;
    Array2D<int> highSETLongestStartRepPeriod;
    Array2D<std::vector<Real64>> ZoneColdHourOfSafetyBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneHeatHourOfSafetyBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneUnmetDegreeHourBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneDiscomfortWtExceedOccuHourBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneCO2LevelHourBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneCO2LevelOccuHourBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneCO2LevelOccupiedHourBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneLightingLevelHourBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneLightingLevelOccuHourBinsRepPeriod;
    Array2D<std::vector<Real64>> ZoneLightingLevelOccupiedHourBinsRepPeriod;

    Array2D<std::vector<Real64>> ZoneLowSETHoursRepPeriod;
    Array2D<std::vector<Real64>> ZoneHighSETHoursRepPeriod;

    int PierceSETerrorIndex = 0;
    int PMVerrorIndex = 0;

    EPVector<DataHeatBalFanSys::ZoneComfortControlsFangerData> ZoneComfortControlsFanger;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) HeatBalFanSysData();
    }
};

} // namespace EnergyPlus

#endif
