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

#ifndef ZoneTempPredictorCorrector_hh_INCLUDED
#define ZoneTempPredictorCorrector_hh_INCLUDED

// C++ Headers
#include <string>
#include <unordered_set>
#include <vector>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace ZoneTempPredictorCorrector {

    struct ZoneSetptScheds
    {
        std::string Name; // Name of the zone
        Sched::Schedule *heatSched = nullptr;
        Sched::Schedule *coolSched = nullptr;
    };

    struct AdaptiveComfortDailySetPointSchedule
    {
        bool initialized = false;
        Array1D<Real64> ThermalComfortAdaptiveASH55_Upper_90;
        Array1D<Real64> ThermalComfortAdaptiveASH55_Upper_80;
        Array1D<Real64> ThermalComfortAdaptiveASH55_Central;
        Array1D<Real64> ThermalComfortAdaptiveCEN15251_Upper_I;
        Array1D<Real64> ThermalComfortAdaptiveCEN15251_Upper_II;
        Array1D<Real64> ThermalComfortAdaptiveCEN15251_Upper_III;
        Array1D<Real64> ThermalComfortAdaptiveCEN15251_Central;
    };

    struct SumHATOutput
    {
        // Output results from calSumHAT
        Real64 sumIntGain = 0.0;
        Real64 sumHA = 0.0;
        Real64 sumHATsurf = 0.0;
        Real64 sumHATref = 0.0;
    };

    struct ZoneSpaceHeatBalanceData
    {
        // This entire struct is re-initialized during the simulation, so no static data may be stored here (e.g. zone or space characteristics)

        // Zone or space air drybulb temperature conditions
        Real64 MAT = DataHeatBalance::ZoneInitialTemp;      // Mean Air Temperature at end of zone time step [C]
        Real64 MRT = DataHeatBalance::ZoneInitialTemp;      // Mean Radiant Temperature using the ZoneMRTCalculation definition defined by the input object [C]
        Real64 stdMRT = DataHeatBalance::ZoneInitialTemp;  // "Standard" Mean Radiant Temperature using the standard area-emissivity weighting [C]
        Real64 ZTAV = DataHeatBalance::ZoneInitialTemp;     // Air Temperature Averaged over the zone time step (during HVAC Time Steps)
        Real64 ZT = DataHeatBalance::ZoneInitialTemp;       // Air Temperature Averaged over the system time step
        Real64 ZTAVComf = DataHeatBalance::ZoneInitialTemp; // Air Temperature Averaged used in thermal comfort models (currently Fanger model only) -
                                                            // TODO: lagged? could MAT be used instead?
        Real64 XMPT = DataHeatBalance::ZoneInitialTemp;     // Air temperature at previous system time step
        std::array<Real64, 4> XMAT = {DataHeatBalance::ZoneInitialTemp,
                                      DataHeatBalance::ZoneInitialTemp,
                                      DataHeatBalance::ZoneInitialTemp,
                                      DataHeatBalance::ZoneInitialTemp}; // Temporary air temperature history
        std::array<Real64, 4> DSXMAT = {DataHeatBalance::ZoneInitialTemp,
                                        DataHeatBalance::ZoneInitialTemp,
                                        DataHeatBalance::ZoneInitialTemp,
                                        DataHeatBalance::ZoneInitialTemp}; // Down Stepped air temperature history storage
        // Exact and Euler solutions
        Real64 TMX = DataHeatBalance::ZoneInitialTemp; // Temporary air temperature to test convergence in Exact and Euler method
        Real64 TM2 = DataHeatBalance::ZoneInitialTemp; // Temporary air temperature at timestep t-2 in Exact and Euler method
        Real64 T1 = 0.0;                               // Air temperature at the previous time step used in Exact and Euler method

        // Zone or space air moisture conditions
        Real64 airHumRat = 0.01;        // Air Humidity Ratio
        Real64 airHumRatAvg = 0.01;     // Air Humidity Ratio averaged over the zone time step
        Real64 airHumRatTemp = 0.01;    // Temporary air humidity ratio at time plus 1
        Real64 airHumRatAvgComf = 0.01; // Air Humidity Ratio averaged over the zone time
                                        // step used in thermal comfort models (currently Fang model only)
                                        // TODO: lagged? could ZoneAirHumRatAvg be used instead?

        std::array<Real64, 4> WPrevZoneTS = {0.0, 0.0, 0.0, 0.0};   // Air Humidity Ratio zone time step history
        std::array<Real64, 4> DSWPrevZoneTS = {0.0, 0.0, 0.0, 0.0}; // DownStepped Air Humidity Ratio zone time step history for 3rd order derivative
        Real64 WTimeMinusP = 0.0;                                   // Air Humidity Ratio at previous system time step
        // Exact and Euler solutions
        Real64 WMX = 0.0; // Temporary humidity ratio to test convergence in Exact and Euler method
        Real64 WM2 = 0.0; // Temporary humidity ratio at timestep t-2 in Exact and Euler method
        Real64 W1 = 0.0;  // Zone/space humidity ratio at the previous time step used in Exact and Euler method

        std::array<Real64, 4> ZTM = {
            0.0, 0.0, 0.0, 0.0}; // air temperature at previous 3 zone timesteps (sized to 4 to be compatible with other similar arrays)
        std::array<Real64, 4> WPrevZoneTSTemp = {0.0, 0.0, 0.0, 0.0}; // Temporary Air Humidity Ratio zone time step history (4th term not used)

        Real64 SumIntGain = 0.0; // Sum of convective internal gains
        Real64 SumHA = 0.0;      // Sum of Hc*Area
        Real64 SumHATsurf = 0.0; // Sum of Hc*Area*Tsurf
        Real64 SumHATref = 0.0;  // Sum of Hc*Area*Tref= 0.0; for ceiling diffuser convection correlation
        Real64 SumMCp = 0.0;     // Sum of MassFlowRate*Cp
        Real64 SumMCpT = 0.0;    // Sum of MassFlowRate*Cp*T
        Real64 SumSysMCp = 0.0;  // Sum of air system MassFlowRate*Cp
        Real64 SumSysMCpT = 0.0; // Sum of air system MassFlowRate*Cp*T
        Real64 SumIntGainExceptPeople = 0.0;

        // Moisture variables to carry info from HB to the Zone Temp Predictor-Corrector for Fan System
        Real64 SumHmAW = 0.0;   // SUM OF ZONE AREA*Moist CONVECTION COEFF*INSIDE Humidity Ratio
        Real64 SumHmARa = 0.0;  // SUM OF ZONE AREA*Moist CONVECTION COEFF*Rho Air
        Real64 SumHmARaW = 0.0; // SUM OF ZONE AREA*Moist CONVECTION COEFF*Rho Air* Inside Humidity Ration
        Real64 SumHmARaZ = 0.0;

        Real64 TempDepCoef = 0.0; // Temperature dependent coefficient
        Real64 TempIndCoef = 0.0; // Temperature ndependent coefficient
        Real64 TempHistoryTerm = 0.0;

        Real64 MCPI = 0.0;                   // INFILTRATION MASS FLOW * AIR SPECIFIC HEAT
        Real64 MCPTI = 0.0;                  // INFILTRATION MASS FLOW * AIR CP * AIR TEMPERATURE
        Real64 MCPV = 0.0;                   // VENTILATION MASS FLOW * AIR SPECIFIC HEAT
        Real64 MCPTV = 0.0;                  // VENTILATION MASS FLOW * AIR CP * AIR TEMPERATURE
        Real64 MCPM = 0.0;                   // Mixing MASS FLOW * AIR SPECIFIC HEAT
        Real64 MCPTM = 0.0;                  // Mixing MASS FLOW * AIR CP * AIR TEMPERATURE
        Real64 MCPE = 0.0;                   // EARTHTUBE MASS FLOW * AIR SPECIFIC HEAT
        Real64 EAMFL = 0.0;                  // OUTDOOR AIR MASS FLOW for EarthTube
        Real64 EAMFLxHumRat = 0.0;           // OUTDOOR AIR MASS FLOW * Humidity Ratio for EarthTube (water vapor mass flow)
        Real64 MCPTE = 0.0;                  // EARTHTUBE MASS FLOW * AIR CP * AIR TEMPERATURE
        Real64 MCPC = 0.0;                   // COOLTOWER MASS FLOW * AIR SPECIFIC HEAT
        Real64 CTMFL = 0.0;                  // OUTDOOR AIR MASS FLOW for cooltower
        Real64 MCPTC = 0.0;                  // COOLTOWER MASS FLOW * AIR CP * AIR TEMPERATURE
        Real64 ThermChimAMFL = 0.0;          // OUTDOOR AIR MASS FLOW for THERMALCHIMNEY
        Real64 MCPTThermChim = 0.0;          // THERMALCHIMNEY MASS FLOW * AIR SPECIFIC HEAT
        Real64 MCPThermChim = 0.0;           // THERMALCHIMNEY MASS FLOW * AIR CP * AIR TEMPERATURE
        Real64 latentGain = 0.0;             // Latent Energy from each Zone (People, equipment)
        Real64 latentGainExceptPeople = 0.0; // Added for hybrid model -- Latent Energy from each Zone (equipment)
        Real64 OAMFL = 0.0;                  // OUTDOOR AIR MASS FLOW (kg/s) for infiltration
        Real64 VAMFL = 0.0;                  // OUTDOOR AIR MASS FLOW (kg/s) for ventilation
        Real64 NonAirSystemResponse = 0.0;   // Convective heat addition rate from non forced air
        // equipment such as baseboards plus heat from lights to
        Real64 SysDepZoneLoads = 0.0; // Convective heat addition or subtraction rate from sources that
        // depend on what is happening with the HVAC system. Such as:
        // heat gain from lights to return air when return flow = 0= 0.0; heat gain
        // from air flow windows to return air when return air flow = 0= 0.0;
        // and heat removed by return air from refrigeration cases when
        // return air flow = 0.
        Real64 SysDepZoneLoadsLagged = 0.0; // SysDepZoneLoads saved to be added to zone heat balance next
        // HVAC time step
        Real64 MDotCPOA = 0.0; // Airbalance MASS FLOW * AIR SPECIFIC HEAT used at Air Balance Method = Quadrature in the ZoneAirBalance:OutdoorAir
        Real64 MDotOA = 0.0;   // Airbalance MASS FLOW rate used at Air Balance Method = Quadrature in the ZoneAirBalance:OutdoorAir
        Real64 MixingMAT = DataHeatBalance::ZoneInitialTemp; // Air temperature for mixing
        Real64 MixingHumRat = 0.01;                          // Air humidity ratio for mixing
        Real64 MixingMassFlowZone = 0.0;                     // Mixing MASS FLOW (kg/s)
        Real64 MixingMassFlowXHumRat = 0.0;                  // Mixing MASS FLOW * Humidity Ratio

        Real64 setPointLast = 0.0;
        Real64 tempIndLoad = 0.0;
        Real64 tempDepLoad = 0.0;
        Real64 airRelHum = 0.0;   // Zone relative humidity in percent
        Real64 AirPowerCap = 0.0; // "air power capacity"  Vol*VolMult*rho*Cp/timestep [W/degK]
        int hmThermalMassMultErrIndex = 0;

        virtual ~ZoneSpaceHeatBalanceData() = default;

        void beginEnvironmentInit(EnergyPlusData &state);

        void setUpOutputVars(EnergyPlusData &state, std::string_view prefix, std::string const &key);

        void predictSystemLoad(EnergyPlusData &state,
                               bool shortenTimeStepSys,
                               bool useZoneTimeStepHistory, // if true then use zone timestep history, if false use system time step
                               Real64 priorTimeStep,        // the old value for timestep length is passed for possible use in interpolating
                               int zoneNum,
                               int spaceNum = 0);

        void calcPredictedSystemLoad(EnergyPlusData &state, Real64 RAFNFrac, int zoneNum, int spaceNum = 0);

        void calcZoneOrSpaceSums(EnergyPlusData &state,
                                 bool CorrectorFlag, // Corrector call flag
                                 int zoneNum,
                                 int spaceNum = 0);

        virtual SumHATOutput calcSumHAT(EnergyPlusData &state, int zoneNum, int spaceNum) = 0;

        void updateTemperatures(
            EnergyPlusData &state, bool ShortenTimeStepSys, bool UseZoneTimeStepHistory, Real64 PriorTimeStep, int zoneNum, int spaceNum = 0);

        Real64 correctAirTemp(EnergyPlusData &state,
                              bool useZoneTimeStepHistory, // if true then use zone timestep history, if false use system time step history
                              int zoneNum,
                              int spaceNum = 0);

        void correctHumRat(EnergyPlusData &state, int zoneNum, int spaceNum = 0);

        void calcPredictedHumidityRatio(EnergyPlusData &state, Real64 RAFNFrac, int zoneNum, int spaceNum = 0);

        void pushZoneTimestepHistory(EnergyPlusData &state, int zoneNum, int spaceNum = 0);

        void pushSystemTimestepHistory(EnergyPlusData &state, int zoneNum, int spaceNum = 0);

        void revertZoneTimestepHistory(EnergyPlusData &state, int zoneNum, int spaceNum = 0);
    };

    struct ZoneHeatBalanceData : ZoneSpaceHeatBalanceData
    {
        SumHATOutput calcSumHAT(EnergyPlusData &state, int zoneNum, [[maybe_unused]] int spaceNum) override;
    };

    struct SpaceHeatBalanceData : ZoneSpaceHeatBalanceData
    {
        SumHATOutput calcSumHAT(EnergyPlusData &state, int zoneNum, int spaceNum) override;
    };

    // Functions

    void ManageZoneAirUpdates(EnergyPlusData &state,
                              DataHeatBalFanSys::PredictorCorrectorCtrl UpdateType, // Can be iGetZoneSetPoints, iPredictStep, iCorrectStep
                              Real64 &ZoneTempChange,                               // Temp change in zone air btw previous and current timestep
                              bool ShortenTimeStepSys,
                              bool UseZoneTimeStepHistory, // if true then use zone timestep history, if false use system time step
                              Real64 PriorTimeStep         // the old value for timestep length is passed for possible use in interpolating
    );

    void GetZoneAirSetPoints(EnergyPlusData &state);

    void InitZoneAirSetPoints(EnergyPlusData &state);

    void PredictSystemLoads(EnergyPlusData &state,
                            bool ShortenTimeStepSys,
                            bool UseZoneTimeStepHistory, // if true then use zone timestep history, if false use system time step
                            Real64 PriorTimeStep         // the old value for timestep length is passed for possible use in interpolating
    );

    void CalcZoneAirTempSetPoints(EnergyPlusData &state);

    void CalculateMonthlyRunningAverageDryBulb(EnergyPlusData &state, Array1D<Real64> &runningAverageASH, Array1D<Real64> &runningAverageCEN);

    void
    CalculateAdaptiveComfortSetPointSchl(EnergyPlusData &state, Array1D<Real64> const &runningAverageASH, Array1D<Real64> const &runningAverageCEN);

    Real64 correctZoneAirTemps(EnergyPlusData &state,
                               bool useZoneTimeStepHistory // if true then use zone timestep history, if false use system time step history
    );

    void PushZoneTimestepHistories(EnergyPlusData &state);

    void PushSystemTimestepHistories(EnergyPlusData &state);

    void RevertZoneTimestepHistories(EnergyPlusData &state);

    void DownInterpolate4HistoryValues(Real64 OldTimeStep,
                                       Real64 NewTimeStep,
                                       Real64 oldVal0,
                                       Real64 oldVal1,
                                       Real64 oldVal2,
                                       Real64 &newVal0,
                                       Real64 &newVal1,
                                       Real64 &newVal2,
                                       Real64 &newVal3,
                                       Real64 &newVal4);

    Real64
    DownInterpolate4HistoryValues(Real64 OldTimeStep, Real64 NewTimeStep, std::array<Real64, 4> const &oldVals, std::array<Real64, 4> &newVals);

    void InverseModelTemperature(EnergyPlusData &state,
                                 int ZoneNum,                   // Zone number
                                 Real64 SumIntGain,             // Zone sum of convective internal gains
                                 Real64 SumIntGainExceptPeople, // Zone sum of convective internal gains except for people
                                 Real64 SumHA,                  // Zone sum of Hc*Area
                                 Real64 SumHATsurf,             // Zone sum of Hc*Area*Tsurf
                                 Real64 SumHATref,              // Zone sum of Hc*Area*Tref, for ceiling diffuser convection correlation
                                 Real64 SumMCp,                 // Zone sum of MassFlowRate*Cp
                                 Real64 SumMCpT,                // Zone sum of MassFlowRate*Cp*T
                                 Real64 SumSysMCp,              // Zone sum of air system MassFlowRate*Cp
                                 Real64 SumSysMCpT,             // Zone sum of air system MassFlowRate*Cp*T
                                 Real64 AirCap                  // Formerly CoefAirrat, coef in zone temp eqn with dim of "air power capacity"rd
    );

    void processInverseModelMultpHM(EnergyPlusData &state,
                                    Real64 &multiplierHM, // Hybrid model thermal mass multiplier
                                    Real64 &multSumHM,    // Sum of Hybrid model thermal mass multipliers
                                    Real64 &countSumHM,   // Count of number of points in sum
                                    Real64 &multAvgHM,    // Average of hybrid model mass multiplier
                                    int zoneNum           // Zone number for the hybrid model
    );

    void InverseModelHumidity(EnergyPlusData &state,
                              int ZoneNum,                   // Zone number
                              Real64 LatentGain,             // Zone sum of latent gain
                              Real64 LatentGainExceptPeople, // Zone sum of latent gain except for people
                              Real64 ZoneMassFlowRate,       // Zone air mass flow rate
                              Real64 MoistureMassFlowRate,   // Zone moisture mass flow rate
                              Real64 H2OHtOfVap,             // Heat of vaporization of air
                              Real64 RhoAir                  // Air density
    );

    void CalcZoneComponentLoadSums(EnergyPlusData &state,
                                   int ZoneNum, // Zone number
                                   ZoneTempPredictorCorrector::ZoneSpaceHeatBalanceData *thisHB,
                                   DataHeatBalance::AirReportVars &thisAirRpt);

    bool VerifyThermostatInZone(EnergyPlusData &state, std::string const &ZoneName); // Zone to verify

    bool VerifyControlledZoneForThermostat(EnergyPlusData &state, std::string const &ZoneName); // Zone to verify

    void DetectOscillatingZoneTemp(EnergyPlusData &state);

    void AdjustAirSetPointsforOpTempCntrl(EnergyPlusData &state, int TempControlledZoneID, int ActualZoneNum, Real64 &ZoneAirSetPoint);

    void AdjustOperativeSetPointsforAdapComfort(EnergyPlusData &state, int TempControlledZoneID, Real64 &ZoneAirSetPoint);

    void CalcZoneAirComfortSetPoints(EnergyPlusData &state);

    void GetComfortSetPoints(EnergyPlusData &state,
                             int PeopleNum,
                             int ComfortControlNum,
                             Real64 PMVSet,
                             Real64 &Tset // drybulb setpoint temperature for a given PMV value
    );

    void AdjustCoolingSetPointforTempAndHumidityControl(EnergyPlusData &state,
                                                        int TempControlledZoneID,
                                                        int ActualZoneNum // controlled zone actual zone number
    );

    void OverrideAirSetPointsforEMSCntrl(EnergyPlusData &state);

    void FillPredefinedTableOnThermostatSetpoints(EnergyPlusData &state);

    void FillPredefinedTableOnThermostatSchedules(EnergyPlusData &state);

    std::tuple<Real64, int, std::string>
    temperatureAndCountInSch(EnergyPlusData &state, int scheduleIndex, bool isSummer, int dayOfWeek, int hourOfDay);

} // namespace ZoneTempPredictorCorrector

struct ZoneTempPredictorCorrectorData : BaseGlobalStruct
{
    std::array<int, (int)HVAC::SetptType::Num> NumTempControls = {0};
    std::array<int, (int)HVAC::SetptType::Num> NumComfortControls = {0};

    // Number of zone with staged controlled objects
    int NumStageCtrZone = 0;
    // Number of zone with onoff thermostat
    int NumOnOffCtrZone = 0;

    // Zone temperature history - used only for oscillation test
    Array2D<Real64> ZoneTempHist;
    Array1D<Real64> ZoneTempOscillate;
    Array1D<Real64> ZoneTempOscillateDuringOccupancy;
    Array1D<Real64> ZoneTempOscillateInDeadband;
    Real64 AnyZoneTempOscillate = 0.0;
    Real64 AnyZoneTempOscillateDuringOccupancy = 0.0;
    Real64 AnyZoneTempOscillateInDeadband = 0.0;
    Real64 AnnualAnyZoneTempOscillate = 0.0;
    Real64 AnnualAnyZoneTempOscillateDuringOccupancy = 0.0;
    Real64 AnnualAnyZoneTempOscillateInDeadband = 0.0;
    bool OscillationVariablesNeeded = false;

    bool InitZoneAirSetPointsOneTimeFlag = true;
    bool SetupOscillationOutputFlag = true;

    // Object Data
    std::unordered_set<std::string> HumidityControlZoneUniqueNames;
    std::array<Array1D<ZoneTempPredictorCorrector::ZoneSetptScheds>, (int)HVAC::SetptType::Num> tempSetptScheds;
    std::array<Array1D<ZoneTempPredictorCorrector::ZoneSetptScheds>, (int)HVAC::SetptType::Num> comfortSetptScheds;
    ZoneTempPredictorCorrector::AdaptiveComfortDailySetPointSchedule AdapComfortDailySetPointSchedule;

    std::array<Real64, 7> AdapComfortSetPointSummerDesDay = {-1};

    bool CalcZoneAirComfortSetPointsFirstTimeFlag = true; // Flag set to make sure you get input once
    bool MyEnvrnFlag = true;
    bool MyDayFlag = true;
    bool ErrorsFound = false;
    bool ControlledZonesChecked = false;

    int IterLimitExceededNum1 = 0;
    int IterLimitErrIndex1 = 0;
    int IterLimitExceededNum2 = 0;
    int IterLimitErrIndex2 = 0;

    EPVector<ZoneTempPredictorCorrector::ZoneHeatBalanceData> zoneHeatBalance;
    EPVector<ZoneTempPredictorCorrector::SpaceHeatBalanceData> spaceHeatBalance;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) ZoneTempPredictorCorrectorData();
    }
};

} // namespace EnergyPlus

#endif
