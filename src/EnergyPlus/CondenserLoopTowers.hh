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

#ifndef CondenserLoopTowers_hh_INCLUDED
#define CondenserLoopTowers_hh_INCLUDED

// C++ Headers
#include <unordered_map>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace CondenserLoopTowers {

    enum class ModelType
    {
        // Empirical Model Type
        Invalid = -1,
        CoolToolsXFModel,
        CoolToolsUserDefined,
        YorkCalcModel,
        YorkCalcUserDefined,
        Num
    };

    enum class EvapLoss
    {
        Invalid = -1,
        UserFactor,
        MoistTheory,
        Num
    };

    enum class Blowdown
    {
        Invalid = -1,
        Concentration,
        Schedule,
        Num
    };

    enum class PIM
    {
        Invalid = -1,
        NominalCapacity,
        UFactor,
        Num
    };

    enum class CapacityCtrl
    {
        Invalid = -1,
        FanCycling,
        FluidBypass,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(CapacityCtrl::Num)> CapacityCtrlNamesUC{"FANCYCLING", "FLUIDBYPASS"};

    enum class CellCtrl
    {
        Invalid = -1,
        MinCell,
        MaxCell,
        Num
    };

    struct CoolingTower : PlantComponent
    {
        // Members
        std::string Name; // User identifier
        DataPlant::PlantEquipmentType TowerType = DataPlant::PlantEquipmentType::Invalid;
        PIM PerformanceInputMethod_Num = PIM::Invalid; // Method of entering tower performance: UA and Design Water
        //  Flow Rate, or Nominal Capacity
        std::string ModelCoeffObjectName;                 // Cooling Tower:Variable Speed Model Coefficient Object name
        bool Available = true;                            // need an array of logicals--load identifiers of available equipment
        bool ON = true;                                   // Simulate the machine at it's operating part load ratio
        Real64 DesignWaterFlowRate = 0.0;                 // Design water flow rate through the tower [m3/s]
        bool DesignWaterFlowRateWasAutoSized = false;     // true if previous was autosize on input
        Real64 DesignWaterFlowPerUnitNomCap = 0.0;        // scalable sizing factor for water flow per capacity [m3/s/W]
        Real64 DesWaterMassFlowRate = 0.0;                // Design water flow rate through the entire tower [kg/s]
        Real64 DesWaterMassFlowRatePerCell = 0.0;         // Design water flow rate per cell [Kg/s]
        Real64 HighSpeedAirFlowRate = 0.0;                // Air flow rate through tower at high speed [m3/s]
        bool HighSpeedAirFlowRateWasAutoSized = false;    // true if previous was autosize on input
        Real64 DesignAirFlowPerUnitNomCap = 0.0;          // scalable sizing factor for air flow per capacity [m3/s/W]
        bool DefaultedDesignAirFlowScalingFactor = false; // true if user left input field blank for DesignAirFlowPerUnitNomCap
        Real64 HighSpeedFanPower = 0.0;                   // Fan power at high fan speed [W]
        bool HighSpeedFanPowerWasAutoSized = false;       // true if fan power was autosize on input
        Real64 DesignFanPowerPerUnitNomCap = 0.0;         // scalable sizing factor for fan power per capacity [W/W]

        Real64 HighSpeedTowerUA = 0.0;                // UA of tower at high fan speed [W/C]
        bool HighSpeedTowerUAWasAutoSized = false;    // true if previous was autosize on input
        Real64 LowSpeedAirFlowRate = 0.0;             // Air flow rate through tower at low speed [m3/s]
        bool LowSpeedAirFlowRateWasAutoSized = false; // true if previous was autosize on input
        Real64 LowSpeedAirFlowRateSizingFactor = 0.0; // sizing factor for low speed flow rate [ ]
        Real64 LowSpeedFanPower = 0.0;                // Fan power at low fan speed [W]
        bool LowSpeedFanPowerWasAutoSized = false;    // true if low speed fan power was autosized on input
        Real64 LowSpeedFanPowerSizingFactor = 0.0;    // sizing factor for low speed fan power []
        Real64 LowSpeedTowerUA = 0.0;                 // UA of tower at low fan speed [W/C]
        bool LowSpeedTowerUAWasAutoSized = false;     // ture if low speed UA was autosize on input
        Real64 LowSpeedTowerUASizingFactor = 0.0;     // sizing factor for UA at low fan speed []
        Real64 FreeConvAirFlowRate = 0.0;             // Air flow rate through tower with fan off [m3/s]
        bool FreeConvAirFlowRateWasAutoSized = false; // true if previous was autosize on input
        Real64 FreeConvAirFlowRateSizingFactor = 0.0; // sizing factor for air flow at free conv []
        Real64 FreeConvTowerUA = 0.0;                 // UA of tower with fan off [W/C]
        bool FreeConvTowerUAWasAutoSized = false;     // true if previous was autosize on input
        Real64 FreeConvTowerUASizingFactor = 0.0;     // sizing factor for UA at fre convection []
        Real64 DesignInletWB = 0.0;                   // Design inlet air wet-bulb temperature (C)
        Real64 DesignApproach = 0.0;                  // Design approach (outlet water temp minus inlet air wet-bulb temp (C)
        Real64 DesignRange = 0.0;                     // Design range temperature (inlet water temp minus outlet water temp (C)
        Real64 MinimumVSAirFlowFrac = 0.0;            // Min air flow ratio (used for VS tower only, point where free conv occurs)
        Real64 CalibratedWaterFlowRate = 0.0;         // Water flow ratio required for model calibration
        Real64 BasinHeaterPowerFTempDiff = 0.0;       // Basin heater capacity per degree C below setpoint (W/C)
        Real64 BasinHeaterSetPointTemp = 0.0;         // setpoint temperature for basin heater operation (C)
        Real64 MakeupWaterDrift = 0.0;                // Makeup water flow rate fraction due to drift
        Real64 FreeConvectionCapacityFraction = 0.0;  // Percentage of tower capacity in free convection regime
        Real64 TowerMassFlowRateMultiplier = 0.0;     // Maximum tower flow rate is this multiplier times design flow rate
        Real64 HeatRejectCapNomCapSizingRatio = 1.25; // ratio of actual cap to nominal capacity []
        Real64 TowerNominalCapacity = 0.0;            // Nominal capacity of the tower [W] with entering water at 35C (95F),
        //  leaving water at 29.44C (85F), entering air at 25.56C (78F) wet-bulb
        //  temp and 35C (95F) dry-bulb temp, and water flow
        //  rate of 5.382E-8 m3/s per watt (3 gpm/ton)
        bool TowerNominalCapacityWasAutoSized = false; // true if tower nominal capacity was autosized on input
        Real64 TowerLowSpeedNomCap = 0.0;              // Nominal capacity of the tower [W] with entering water at 35C (95F),
        //  leaving water at 29.44C (85F), entering air at 25.56C (78F) wet-bulb
        //  temp and 35C (95F) dry-bulb temp, and water flow
        //  rate of 5.382E-8 m3/s per nominal capacity watt (3 gpm/ton)
        bool TowerLowSpeedNomCapWasAutoSized = false; // true if previous was autosize on input
        Real64 TowerLowSpeedNomCapSizingFactor = 0.0; // sizing factor for low speed capacity []
        Real64 TowerFreeConvNomCap = 0.0;             // Nominal capacity of the tower [W] with entering water at 35C (95F),
        //  leaving water at 29.44C (85F), entering air at 25.56C (78F) wet-bulb
        //  temp and 35C (95F) dry-bulb temp, and water flow
        //  rate of 5.382E-8 m3/s per nominal capacity watt (3 gpm/ton)
        bool TowerFreeConvNomCapWasAutoSized = false;  // true if previous was autosize on Input
        Real64 TowerFreeConvNomCapSizingFactor = 0.0;  // sizing factor for free conv capacity []
        Real64 SizFac = 0.0;                           // sizing factor
        int WaterInletNodeNum = 0;                     // Node number on the water inlet side of the tower
        int WaterOutletNodeNum = 0;                    // Node number on the water outlet side of the tower
        int OutdoorAirInletNodeNum = 0;                // Node number of outdoor air inlet for the tower
        ModelType TowerModelType = ModelType::Invalid; // Type of empirical model (1=CoolTools)
        int FanPowerfAirFlowCurve = 0;                 // Index to fan power correlation curve for VS Towers
        Sched::Schedule *blowDownSched = nullptr;      // Pointer to blow down schedule
        Sched::Schedule *basinHeaterSched = nullptr;   // Pointer to basin heater schedule
        int HighMassFlowErrorCount = 0;                // Counter when mass flow rate is > Design*TowerMassFlowRateMultiplier
        int HighMassFlowErrorIndex = 0;                // Index for high mass flow recurring error message
        int OutletWaterTempErrorCount = 0;             // Counter when outlet water temperature is < minimum allowed temperature
        int OutletWaterTempErrorIndex = 0;             // Index for outlet water temperature recurring error message
        int SmallWaterMassFlowErrorCount = 0;          // Counter when water mass flow rate is very small
        int SmallWaterMassFlowErrorIndex = 0;          // Index for very small water mass flow rate recurring error message
        int WMFRLessThanMinAvailErrCount = 0;          // Counter when water mass flow rate is less than minimum available
        int WMFRLessThanMinAvailErrIndex = 0;          // Index for water mass flow rate less than minavail recurring message
        int WMFRGreaterThanMaxAvailErrCount = 0;       // Counter when water mass flow rate is greater than minimum available
        int WMFRGreaterThanMaxAvailErrIndex = 0;       // Index for water mass flow rate > minavail recurring message
        int CoolingTowerAFRRFailedCount = 0;           // Counter for air flow rate ratio out of bounds error
        int CoolingTowerAFRRFailedIndex = 0;           // Index for air flow rate ratio out of bounds error
        int SpeedSelected = 0;                         // speed of the two-speed fan selected (0:ON;1:LOW;2:HIGH)
        // fluid bypass
        CapacityCtrl CapacityControl = CapacityCtrl::Invalid; // Type of capacity control for single speed cooling tower:
        //  0 - FanCycling, 1 - FluidBypass
        Real64 BypassFraction = 0.0; // Fraction of fluid bypass as a ratio of total fluid flow
        //  through the tower sump
        // multi cell tower
        int NumCell = 0; // Number of cells in the cooling tower
        CellCtrl cellCtrl = CellCtrl::MaxCell;
        int NumCellOn = 0;            // number of cells working
        Real64 MinFracFlowRate = 0.0; // Minimal fraction of design flow/cell allowable
        Real64 MaxFracFlowRate = 0.0; // Maximal ratio of design flow/cell allowable
        // begin water system interactions
        EvapLoss EvapLossMode = EvapLoss::MoistTheory;   // sets how tower water evaporation is modeled
        Real64 UserEvapLossFactor = 0.0;                 // simple model [%/Delt C]
        Real64 DriftLossFraction = 0.008;                // default value is 0.008%
        Blowdown BlowdownMode = Blowdown::Concentration; // sets how tower water blowdown is modeled
        Real64 ConcentrationRatio = 3.0;                 // ratio of solids in blowdown vs make up water
        Sched::Schedule *blowdownSched = nullptr;        // index "pointer" to schedule of blowdown in [m3/s]
        bool SuppliedByWaterSystem = false;
        int WaterTankID = 0;          // index "pointer" to WaterStorage structure
        int WaterTankDemandARRID = 0; // index "pointer" to demand array inside WaterStorage structure
        // end water system variables
        // loop topology variables
        PlantLocation plantLoc;
        // Merkel VS model curves
        int UAModFuncAirFlowRatioCurvePtr = 0;   // curve index for UA modifier as a function of air flow ratio
        int UAModFuncWetBulbDiffCurvePtr = 0;    // curve index for UA modifier as a function of local wetbulb
        int UAModFuncWaterFlowRatioCurvePtr = 0; // curve index for UA modifier as a function of water flow ratio
        bool SetpointIsOnOutlet = false;         // if true look to outlet node of tower, if flase look to overall loop setpoint
        int VSMerkelAFRErrorIter = 0;            // error counter for regula falsi failed with max iterations, vs merkel model
        int VSMerkelAFRErrorIterIndex = 0;       // recurring error index for regula falsi failed with max iterations, vs merkel model
        int VSMerkelAFRErrorFail = 0;            // error counter for regula falsi failed with limits exceeded, vs merkel model
        int VSMerkelAFRErrorFailIndex = 0;       // recurring error index for regula falsi failed with limits exceeded, vs merkel model
        Real64 DesInletWaterTemp = 0.0;          // design tower inlet water temperature (C)
        Real64 DesOutletWaterTemp = 0.0;         // design tower outlet water temperature (C)
        Real64 DesInletAirDBTemp = 0.0;          // design tower inlet air dry-bulb temperature (C)
        bool TowerInletCondsAutoSize = false;    // true if tower inlet condition is autosized or defaulted to autosize
        // Operational fault parameters
        bool FaultyCondenserSWTFlag = false;   // True if the condenser has SWT sensor fault
        int FaultyCondenserSWTIndex = 0;       // Index of the fault object corresponding to the condenser
        Real64 FaultyCondenserSWTOffset = 0.0; // Condenser SWT sensor offset
        bool FaultyTowerFoulingFlag = false;   // True if the tower has fouling fault
        int FaultyTowerFoulingIndex = 0;       // Index of the fouling fault object corresponding to the condenser
        Real64 FaultyTowerFoulingFactor = 1.0; // Tower fouling factor
        std::string EndUseSubcategory;         // identifier use for the end use subcategory
        bool envrnFlag = true;
        bool oneTimeFlag = true;
        Real64 TimeStepSysLast = 0.0;    // last system time step (used to check for downshifting)
        Real64 CurrentEndTimeLast = 0.0; // end time of time step for last simulation time step

        // From module level variables, apparently the module AirFlowRateRatio was used slightly different from the struct's AirFlowRatio variable
        //  so removing this caused diffs that I did not spend time investigating...they might be fine diffs, check some time later
        Real64 airFlowRateRatio = 0.0;

        // From TowerInletConds struct
        Real64 WaterTemp = 0.0;  // Tower water inlet temperature (C)
        Real64 AirTemp = 0.0;    // Tower air inlet dry-bulb temperature (C)
        Real64 AirWetBulb = 0.0; // Tower air inlet wet-bulb temperature (C)
        Real64 AirPress = 0.0;   // Tower air barometric pressure
        Real64 AirHumRat = 0.0;  // Tower air inlet humidity ratio (kg/kg)

        // From ReportVars struct
        Real64 InletWaterTemp = 0.0;         // Tower inlet water temperature (C)
        Real64 OutletWaterTemp = 0.0;        // Tower outlet water temperature (C)
        Real64 WaterMassFlowRate = 0.0;      // Tower water mass flow rate (m3/s)
        Real64 Qactual = 0.0;                // Tower heat rejection rate (W)
        Real64 FanPower = 0.0;               // Tower fan power (W)
        Real64 FanEnergy = 0.0;              // Tower fan energy consumption (J)
        Real64 AirFlowRatio = 0.0;           // Air flow ratio through variable speed cooling tower
        Real64 BasinHeaterPower = 0.0;       // Basin heater power (W)
        Real64 BasinHeaterConsumption = 0.0; // Basin heater energy consumption (J)
        Real64 WaterUsage = 0.0;             // Tower water usage (m3/s)
        Real64 WaterAmountUsed = 0.0;        // Tower make up water usage (m3)
        Real64 FanCyclingRatio = 0.0;        // cycling ratio of tower fan when min fan speed provide too much capacity (for VFD)
        Real64 EvaporationVdot = 0.0;
        Real64 EvaporationVol = 0.0;
        Real64 DriftVdot = 0.0;
        Real64 DriftVol = 0.0;
        Real64 BlowdownVdot = 0.0;
        Real64 BlowdownVol = 0.0;
        Real64 MakeUpVdot = 0.0;
        Real64 MakeUpVol = 0.0;
        Real64 TankSupplyVdot = 0.0;
        Real64 TankSupplyVol = 0.0;
        Real64 StarvedMakeUpVdot = 0.0;
        Real64 StarvedMakeUpVol = 0.0;
        Real64 coolingTowerApproach = 0.0;
        Real64 coolingTowerRange = 0.0;

        // From VSTower struct - for Variable speed towers only
        std::array<Real64, 35> Coeff = {0.0}; // - model coefficients
        bool FoundModelCoeff = false;         // - TRUE if model is calibratable
        Real64 MinInletAirWBTemp = 0.0;       // - model limit for min inlet air WB temp
        Real64 MaxInletAirWBTemp = 0.0;       // - model limit for max inlet air WB temp
        Real64 MinRangeTemp = 0.0;            // - model limit for min range temp
        Real64 MaxRangeTemp = 0.0;            // - model limit for max range temp
        Real64 MinApproachTemp = 0.0;         // - model limit for min approach temp
        Real64 MaxApproachTemp = 0.0;         // - model limit for max approach temp
        Real64 MinWaterFlowRatio = 0.0;       // - model limit for min water flow rate ratio
        Real64 MaxWaterFlowRatio = 0.0;       // - model limit for max water flow rate ratio
        Real64 MaxLiquidToGasRatio = 0.0;     // - model limit for max liquid to gas ratio
        int VSErrorCountFlowFrac = 0;         // - counter if water flow rate ratio limits are exceeded
        int VSErrorCountWFRR = 0;             // - counter if water flow rate ratio limits are exceeded
        int VSErrorCountIAWB = 0;             // - counter if inlet air wet-bulb temperature limits are exceeded
        int VSErrorCountTR = 0;               // - counter if tower range temperature limits are exceeded
        int VSErrorCountTRCalc = 0;           // - counter if tower range temperature could not be calculated
        int VSErrorCountTA = 0;               // - counter if tower approach temperature limits are exceeded
        int ErrIndexFlowFrac = 0;             // - index to recurring error structure for liquid to gas ratio
        int ErrIndexWFRR = 0;                 // - index to recurring error structure for water flow rate ratio
        int ErrIndexIAWB = 0;                 // - index to recurring error structure for inlet air WB
        int ErrIndexTR = 0;                   // - index to recurring error structure for tower range
        int ErrIndexTRCalc = 0;               // - index to recurring error structure for tower range
        int ErrIndexTA = 0;                   // - index to recurring error structure for tower approach
        int ErrIndexLG = 0;                   // - index to recurring error structure for tower liquid/gas ratio
        //- Tr = Range temperature
        std::string TrBuffer1; // - buffer to print Tr warning messages on following time step
        std::string TrBuffer2; // - buffer to print Tr warning messages on following time step
        std::string TrBuffer3; // - buffer to print Tr warning messages on following time step
        //- Twb = Wet-bulb temperature
        std::string TwbBuffer1; // - buffer to print Twb warning messages on following time step
        std::string TwbBuffer2; // - buffer to print Twb warning messages on following time step
        std::string TwbBuffer3; // - buffer to print Twb warning messages on following time step
        //- Ta = Approach temperature
        std::string TaBuffer1; // - buffer to print Ta warning messages on following time step
        std::string TaBuffer2; // - buffer to print Ta warning messages on following time step
        std::string TaBuffer3; // - buffer to print Ta warning messages on following time step
        //- WFRR = Water flow rate ratio
        std::string WFRRBuffer1; // - buffer to print WFRR warning messages on following time step
        std::string WFRRBuffer2; // - buffer to print WFRR warning messages on following time step
        std::string WFRRBuffer3; // - buffer to print WFRR warning messages on following time step
        //- LG = Liquid to gas ratio
        std::string LGBuffer1;               // - buffer to print LG warning messages on following time step
        std::string LGBuffer2;               // - buffer to print LG warning messages on following time step
        bool PrintTrMessage = false;         // - flag to print Tr error message
        bool PrintTwbMessage = false;        // - flag to print Twb error message
        bool PrintTaMessage = false;         // - flag to print Ta error message
        bool PrintWFRRMessage = false;       // - flag to print WFRR error message
        bool PrintLGMessage = false;         // - flag to print liquid-gas ratio error message
        Real64 TrLast = 0.0;                 // value of Tr when warning occurred (passed to Recurring Warning)
        Real64 TwbLast = 0.0;                // value of Twb when warning occurred (passed to Recurring Warning)
        Real64 TaLast = 0.0;                 // value of Ta when warning occurred (passed to Recurring Warning)
        Real64 WaterFlowRateRatioLast = 0.0; // value of WFRR when warning occurred (passed to Recurring Warn)
        Real64 LGLast = 0.0;                 // value of LG when warning occurred (passed to Recurring Warn)

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void getDesignCapacities(EnergyPlusData &state,
                                 [[maybe_unused]] const PlantLocation &calledFromLocation,
                                 Real64 &MaxLoad,
                                 Real64 &MinLoad,
                                 Real64 &OptLoad) override;

        void getSizingFactor(Real64 &SizFac) override;

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation) override;

        void oneTimeInit(EnergyPlusData &state) override;

        void initEachEnvironment(EnergyPlusData &state);

        void initialize(EnergyPlusData &state);

        void setupOutputVariables(EnergyPlusData &state);

        void SizeTower(EnergyPlusData &state);

        void SizeVSMerkelTower(EnergyPlusData &state);

        void calculateSingleSpeedTower(EnergyPlusData &state, Real64 &MyLoad, bool RunFlag);

        void calculateTwoSpeedTower(EnergyPlusData &state, Real64 &MyLoad, bool RunFlag);

        void calculateMerkelVariableSpeedTower(EnergyPlusData &state, Real64 &MyLoad, bool RunFlag);

        void calculateVariableSpeedTower(EnergyPlusData &state, Real64 &MyLoad, bool RunFlag);

        Real64 calculateSimpleTowerOutletTemp(EnergyPlusData &state, Real64 waterMassFlowRate, Real64 AirFlowRate, Real64 UAdesign);

        Real64 calculateVariableTowerOutletTemp(EnergyPlusData &state,
                                                Real64 WaterFlowRateRatio,    // current water flow rate ratio (capped if applicable)
                                                Real64 airFlowRateRatioLocal, // current air flow rate ratio
                                                Real64 Twb                    // current inlet air wet-bulb temperature (C, capped if applicable)
        );

        void calculateWaterUsage(EnergyPlusData &state);

        Real64 calculateVariableSpeedApproach(EnergyPlusData &state,
                                              Real64 PctWaterFlow,      // Water flow ratio of cooling tower
                                              Real64 airFlowRatioLocal, // Air flow ratio of cooling tower
                                              Real64 Twb,               // Inlet air wet-bulb temperature [C]
                                              Real64 Tr                 // Cooling tower range (outlet water temp minus inlet air wet-bulb temp) [C]
        );

        void checkModelBounds(EnergyPlusData &state,
                              Real64 Twb,                      // current inlet air wet-bulb temperature (C)
                              Real64 Tr,                       // requested range temperature for current time step (C)
                              Real64 Ta,                       // requested approach temperature for current time step (C)
                              Real64 WaterFlowRateRatio,       // current water flow rate ratio at water inlet node
                              Real64 &TwbCapped,               // bounded value of inlet air wet-bulb temperature (C)
                              Real64 &TrCapped,                // bounded value of range temperature (C)
                              Real64 &TaCapped,                // bounded value of approach temperature (C)
                              Real64 &WaterFlowRateRatioCapped // bounded value of water flow rate ratio
        );

        void update(EnergyPlusData &state);

        void report(EnergyPlusData &state, bool RunFlag);

        void checkMassFlowAndLoad(EnergyPlusData &state, Real64 MyLoad, bool RunFlag, bool &returnFlagSet);

        static CoolingTower *factory(EnergyPlusData &state, std::string_view objectName);

        Real64 getDynamicMaxCapacity(EnergyPlusData &state) override;
    };

    void GetTowerInput(EnergyPlusData &state);

} // namespace CondenserLoopTowers

struct CondenserLoopTowersData : BaseGlobalStruct
{
    bool GetInput = true;
    Array1D<CondenserLoopTowers::CoolingTower> towers; // dimension to number of machines

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) CondenserLoopTowersData();
    }
};

} // namespace EnergyPlus

#endif
