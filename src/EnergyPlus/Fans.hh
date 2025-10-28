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

#ifndef Fans_hh_INCLUDED
#define Fans_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Fans {

    // Fan Minimum Flow Fraction Input Method
    enum class MinFlowFracMethod
    {
        Invalid = -1,
        MinFrac,
        FixedMin,
        Num
    };

    enum class AvailManagerMode
    {
        Invalid = -1,
        Coupled,
        Decoupled,
        Num
    };

    static constexpr std::array<std::string_view, (int)AvailManagerMode::Num> availManagerModeNamesUC = {"COUPLED", "DECOUPLED"};

    struct FanBase
    {
        FanBase()
        {
        }

        virtual ~FanBase() = default;

        virtual void set_size(EnergyPlusData &state) = 0;

        virtual void init(EnergyPlusData &state) = 0;
        virtual void simulate(EnergyPlusData &state,
                              bool const FirstHVACIteration,
                              ObjexxFCL::Optional<Real64 const> speedRatio = _,       // Flow fraction in operating mode 1
                              ObjexxFCL::Optional<Real64 const> pressureRise = _,     // Pressure difference to use for DeltaPress
                              ObjexxFCL::Optional<Real64 const> flowFraction = _,     // Flow fraction in operating mode 1
                              ObjexxFCL::Optional<Real64 const> massFlowRate1 = _,    // Mass flow rate in operating mode 1 [kg/s]
                              ObjexxFCL::Optional<Real64 const> runTimeFraction1 = _, // Run time fraction in operating mode 1
                              ObjexxFCL::Optional<Real64 const> massFlowRate2 = _,    // Mass flow rate in operating mode 2 [kg/s]
                              ObjexxFCL::Optional<Real64 const> runTimeFraction2 = _, // Run time fraction in operating mode 2
                              ObjexxFCL::Optional<Real64 const> pressureRise2 = _     // Pressure difference to use for operating mode 2
        );

        virtual void update(EnergyPlusData &state) = 0;

        virtual void report(EnergyPlusData &state) = 0;

        virtual Real64 getDesignHeatGain(EnergyPlusData &state, Real64 const FanVolFlow) = 0;

        virtual void getInputsForDesignHeatGain(EnergyPlusData &state,
                                                Real64 &deltaP,
                                                Real64 &motEff,
                                                Real64 &totEff,
                                                Real64 &motInAirFrac,
                                                Real64 &fanShaftPow,
                                                Real64 &motInPower,
                                                bool &fanCompModel) = 0;
        // Members
        std::string Name;                            // Name of the fan
        HVAC::FanType type = HVAC::FanType::Invalid; // DataHVACGlobals fan type

        bool envrnFlag = true;  // initialize to true
        bool sizingFlag = true; // initialize to true, set to false after sizing routine

        std::string endUseSubcategoryName;

        Sched::Schedule *availSched = nullptr; // Pointer to the availability schedule
        int inletNodeNum = 0;
        int outletNodeNum = 0;
        int airLoopNum = 0;
        bool airPathFlag = false; // Yes, this fan is a part of airpath
        bool isAFNFan = false;    // Is fan part of and AirFlowNetwork distribution system

        Real64 maxAirFlowRate = 0.0; // Max Specified Volume Flow Rate of Fan [m3/sec]
        Real64 minAirFlowRate = 0.0; // Max Specified Volume Flow Rate of Fan [m3/sec]
        bool maxAirFlowRateIsAutosized = false;

        Real64 deltaPress = 0.0; // Delta Pressure Across the Fan [N/m2]
        Real64 deltaTemp = 0.0;  // Temp Rise across the Fan [C]

        Real64 totalEff = 0.0; // Fan total system efficiency (fan*belt*motor*VFD)

        Real64 motorEff = 0.0;       // Fan motor efficiency
        Real64 motorInAirFrac = 0.0; // Fraction of motor heat entering air stream

        // report variables
        Real64 totalPower = 0.0;  // Power of the Fan being Simulated [W]
        Real64 totalEnergy = 0.0; // Fan energy in [J]
                                  //    Real64 fanRuntimeFraction; // Fraction of the timestep that the fan operates

        Real64 powerLossToAir = 0.0; // fan heat gain into process air [W]

        Real64 inletAirMassFlowRate = 0.0; // MassFlow through the Fan being Simulated [kg/Sec]
        Real64 outletAirMassFlowRate = 0.0;

        Real64 maxAirMassFlowRate = 0.0; // Max flow rate of fan in kg/sec
                                         //    Real64 m_minAirMassFlowRate; // Min flow rate of fan in kg/sec
                                         //    int fanMinAirFracMethod; // parameter for what method is used for min flow fraction
                                         //    Real64 fanFixedMin; // Absolute minimum fan air flow [m3/s]
        Real64 minAirMassFlowRate = 0.0; // Min flow rate of fan in kg/sec

        // Mass Flow Rate Control Variables
        Real64 massFlowRateMaxAvail = 0.0;
        Real64 massFlowRateMinAvail = 0.0;
        Real64 rhoAirStdInit = 0.0;

        Real64 inletAirTemp = 0.0;
        Real64 outletAirTemp = 0.0;
        Real64 inletAirHumRat = 0.0;
        Real64 outletAirHumRat = 0.0;
        Real64 inletAirEnthalpy = 0.0;
        Real64 outletAirEnthalpy = 0.0;

        // Faults
        bool faultyFilterFlag = false; // Indicate whether there is a fouling air filter corresponding to the fan
        int faultyFilterIndex = 0;     // Index of the fouling air filter corresponding to the fan

        // EMS
        bool EMSMaxAirFlowRateOverrideOn = false; // if true, EMS wants to override fan size for Max Volume Flow Rate
        Real64 EMSMaxAirFlowRateValue = 0.0;      // EMS value to use for override of  Max Volume Flow Rate
        bool EMSMaxMassFlowOverrideOn = false;    // if true, then EMS is calling to override mass flow
        Real64 EMSAirMassFlowValue = 0.0;         // value EMS is directing to use [kg/s]
        bool EMSPressureOverrideOn = false;       // if true, then EMS is calling to override
        Real64 EMSPressureValue = 0.0;            // EMS value for Delta Pressure Across the Fan [Pa]
        bool EMSTotalEffOverrideOn = false;       // if true, then EMS is calling to override
        Real64 EMSTotalEffValue = 0.0;            // EMS value for total efficiency of the Fan, fraction on 0..1

        std::string sizingPrefix;
    };

    enum class VFDEffType
    {
        Invalid = -1,
        Speed,
        Power,
        Num
    };

    static constexpr std::array<std::string_view, (int)VFDEffType::Num> vfdEffTypeNamesUC = {"SPEED", "POWER"};

    struct FanComponent : public FanBase
    {
        void set_size(EnergyPlusData &state);

        void init(EnergyPlusData &state);

        Real64 getDesignHeatGain(EnergyPlusData &state, Real64 const FanVolFlow);

        void getInputsForDesignHeatGain(EnergyPlusData &state,
                                        Real64 &deltaP,
                                        Real64 &motEff,
                                        Real64 &totEff,
                                        Real64 &motInAirFrac,
                                        Real64 &fanShaftPow,
                                        Real64 &motInPower,
                                        bool &fanCompModel);

        void update(EnergyPlusData &state);

        void report(EnergyPlusData &state);

        void simulateConstant(EnergyPlusData &state);

        void simulateVAV(EnergyPlusData &state, ObjexxFCL::Optional<Real64 const> PressureRise = _);

        void simulateOnOff(EnergyPlusData &state, ObjexxFCL::Optional<Real64 const> SpeedRatio = _);

        void simulateZoneExhaust(EnergyPlusData &state);

        void simulateComponentModel(EnergyPlusData &state);

        Real64 runtimeFrac = 0.0;

        MinFlowFracMethod minAirFracMethod = MinFlowFracMethod::MinFrac; // parameter for what method is used for min flow fraction

        Real64 minFrac = 0.0;                                  // Minimum fan air flow fraction
        Real64 fixedMin = 0.0;                                 // Absolute minimum fan air flow [m3/s]
        std::array<Real64, 5> coeffs{0.0, 0.0, 0.0, 0.0, 0.0}; // Fan Part Load Coefficients to match fan type
        // Mass Flow Rate Control Variables

        int nightVentPerfNum = 0;
        int powerRatioAtSpeedRatioCurveNum = 0;
        int effRatioCurveNum = 0;
        bool oneTimePowerRatioCheck = true;          // one time flag used for error message
        bool oneTimeEffRatioCheck = true;            // one time flag used for error message
        Real64 wheelDia = 0.0;                       // Fan wheel outer diameter [m]
        Real64 outletArea = 0.0;                     // Fan outlet area [m2]
        Real64 maxEff = 0.0;                         // Fan maximum static efficiency [-]
        Real64 eulerMaxEff = 0.0;                    // Euler number at fan maximum static efficiency [-]
        Real64 maxDimFlow = 0.0;                     // Fan maximum dimensionless airflow [-]
        Real64 shaftPowerMax = 0.0;                  // Fan shaft maximum input power [W]
        Real64 sizingFactor = 0.0;                   // Fan sizing factor [-]
        Real64 pulleyDiaRatio = 0.0;                 // Motor/fan pulley diameter ratio [-]
        Real64 beltMaxTorque = 0.0;                  // Belt maximum torque [N-m]
        Real64 beltSizingFactor = 0.0;               // Belt sizing factor [-]
        Real64 beltTorqueTrans = 0.0;                // Belt fractional torque transition Region 1-2 [-]
        Real64 motorMaxSpeed = 0.0;                  // Motor maximum speed [rpm]
        Real64 motorMaxOutPower = 0.0;               // Motor maximum output power [W]
        Real64 motorSizingFactor = 0.0;              // Motor sizing factor [-]
        VFDEffType vfdEffType = VFDEffType::Invalid; // VFD efficiency type [Speed or Power]
        Real64 vfdMaxOutPower = 0.0;                 // VFD maximum output power [W]
        Real64 vfdSizingFactor = 0.0;                // VFD sizing factor [-]
        int pressRiseCurveNum = 0;                   // Fan pressure rise curve index
        int pressResetCurveNum = 0;                  // Duct static pressure reset curve index
        int plTotalEffNormCurveNum = 0;              // Fan part-load efficiency (normal) curve index
        int plTotalEffStallCurveNum = 0;             // Fan part-load efficiency (stall) curve index
        int dimFlowNormCurveNum = 0;                 // Fan dimensionless airflow (normal) curve index
        int dimFlowStallCurveNum = 0;                // Fan dimensionless airflow (stall) curve index
        int beltMaxEffCurveNum = 0;                  // Belt maximum efficiency curve index
        int plBeltEffReg1CurveNum = 0;               // Belt part-load efficiency (Region 1) curve index
        int plBeltEffReg2CurveNum = 0;               // Belt part-load efficiency (Region 2) curve index
        int plBeltEffReg3CurveNum = 0;               // Belt part-load efficiency (Region 3) curve index
        int motorMaxEffCurveNum = 0;                 // Motor maximum efficiency curve index
        int plMotorEffCurveNum = 0;                  // Motor part-load efficiency curve index
        int vfdEffCurveNum = 0;                      // VFD efficiency curve index
        Real64 deltaPressTot = 0.0;                  // Total pressure rise across fan [N/m2]
        Real64 airPower = 0.0;                       // Air power for fan being Simulated [W]
        Real64 fanSpeed = 0.0;                       // Fan shaft rotational speed [rpm]
        Real64 fanTorque = 0.0;                      // Fan shaft torque [N-m]
        Real64 wheelEff = 0.0;                       // Fan efficiency (mechanical)
        Real64 shaftPower = 0.0;                     // Shaft input power for fan being Simulated [W]
        Real64 beltMaxEff = 0.0;                     // Belt maximum efficiency (mechanical)
        Real64 beltEff = 0.0;                        // Belt efficiency (mechanical)
        Real64 beltInputPower = 0.0;                 // Belt input power for fan being Simulated [W]
        Real64 motorMaxEff = 0.0;                    // Motor maximum efficiency (electrical)
        Real64 motorInputPower = 0.0;                // Motor input power for fan being Simulated [W]
        Real64 vfdEff = 0.0;                         // VFD efficiency (electrical)
        Real64 vfdInputPower = 0.0;                  // VFD input power for fan being Simulated [W]
        // zone exhaust fan
        Sched::Schedule *flowFracSched = nullptr;                      // schedule index flow rate modifier schedule
        AvailManagerMode availManagerMode = AvailManagerMode::Invalid; // mode for how exhaust fan should react to availability managers
        Sched::Schedule *minTempLimitSched = nullptr;                  // schedule index minimum temperature limit
        Sched::Schedule *balancedFractSched = nullptr;                 // schedule index portion recirculated
        Real64 unbalancedOutletMassFlowRate = 0.0;
        Real64 balancedOutletMassFlowRate = 0.0;
        Real64 designPointFEI = 0.0; // Fan Energy Index for the fan at the design operating point
    };

    struct NightVentPerfData
    {
        // Members
        std::string FanName;             // Name of the fan that will use this data
        Real64 FanEff = 0.0;             // Fan total efficiency; motor and mechanical
        Real64 DeltaPress = 0.0;         // Delta Pressure Across the Fan [N/m2]
        Real64 MaxAirFlowRate = 0.0;     // Max Specified Volume Flow Rate of Fan [m3/s]
        Real64 MaxAirMassFlowRate = 0.0; // Max flow rate of fan in kg/sec
        Real64 MotEff = 0.0;             // Fan motor efficiency
        Real64 MotInAirFrac = 0.0;       // Fraction of motor heat entering air stream
    };

    void GetFanInput(EnergyPlusData &state);

    int GetFanIndex(EnergyPlusData &state, std::string const &FanName);

    Real64 CalFaultyFanAirFlowReduction(EnergyPlusData &state,
                                        std::string const &FanName,    // Name of the Fan
                                        Real64 FanDesignAirFlowRate,   // Fan Design Volume Flow Rate [m3/s]
                                        Real64 FanDesignDeltaPress,    // Fan Design Delta Pressure [Pa]
                                        Real64 FanFaultyDeltaPressInc, // Increase of Fan Delta Pressure in the Faulty Case [Pa]
                                        int FanCurvePtr                // Fan Curve Pointer
    );

    enum class PowerSizing
    {
        Invalid = -1,
        PerFlow,
        PerFlowPerPressure,
        TotalEfficiencyAndPressure,
        Num
    };

    static constexpr std::array<std::string_view, (int)PowerSizing::Num> powerSizingNamesUC = {
        "POWERPERFLOW", "POWERPERFLOWPERPRESSURE", "TOTALEFFICIENCYANDPRESSURE"};

    enum class HeatLossDest
    {
        Invalid = -1,
        Zone,
        Outside,
        Num
    };

    enum class SpeedControl : int
    {
        // TODO: enum check
        Invalid = -1,
        Discrete,
        Continuous,
        Num
    };

    static constexpr std::array<std::string_view, (int)SpeedControl::Num> speedControlNames = {"Discrete", "Continuous"};

    static constexpr std::array<std::string_view, (int)SpeedControl::Num> speedControlNamesUC = {"DISCRETE", "CONTINUOUS"};

    class FanSystem : public FanBase
    {
    public: // Methods
        FanSystem()
        {
        }
        // Destructor
        ~FanSystem()
        {
        }

        // Copy Constructor
        FanSystem(FanSystem const &) = default;

        Real64 getDesignTemperatureRise(EnergyPlusData &state) const;

        Real64 getDesignHeatGain(EnergyPlusData &state, Real64 const FanVolFlow);

        void getInputsForDesignHeatGain(EnergyPlusData &state,
                                        Real64 &deltaP,
                                        Real64 &motEff,
                                        Real64 &totEff,
                                        Real64 &motInAirFrac,
                                        Real64 &fanShaftPow,
                                        Real64 &motInPower,
                                        bool &fanCompModel);

        SpeedControl speedControl = SpeedControl::Invalid; // Discrete or Continuous speed control method
        Real64 designElecPower = 0.0;                      // design electric power consumption [W]
        int powerModFuncFlowFracCurveNum = 0;              // pointer to performance curve or table
        int numSpeeds = 0;                                 // input for how many speed levels for discrete fan
        std::vector<Real64> massFlowAtSpeed;
        std::vector<Real64> flowFracAtSpeed; // array of flow fractions for speed levels

        // Mass Flow Rate Control Variables
        bool isSecondaryDriver = false; // true if this fan is used to augment flow and may pass air when off.

        // FEI
        static Real64 report_fei(EnergyPlusData &state, Real64 const designFlowRate, Real64 const designElecPower, Real64 const designDeltaPress);

        void init(EnergyPlusData &state);

        void set_size(EnergyPlusData &state);

        void
        calcSimpleSystemFan(EnergyPlusData &state,
                            ObjexxFCL::Optional<Real64 const> flowFraction, // Flow fraction for entire timestep (not used if flow ratios are present)
                            ObjexxFCL::Optional<Real64 const> pressureRise, // Pressure difference to use for DeltaPress
                            ObjexxFCL::Optional<Real64 const> flowRatio1,   // Flow ratio in operating mode 1
                            ObjexxFCL::Optional<Real64 const> runTimeFrac1, // Run time fraction in operating mode 1
                            ObjexxFCL::Optional<Real64 const> flowRatio2,   // Flow ratio in operating mode 2
                            ObjexxFCL::Optional<Real64 const> runTimeFrac2, // Run time fraction in operating mode 2
                            ObjexxFCL::Optional<Real64 const> pressureRise2 // Pressure difference to use for operating mode 2
        );

        void update(EnergyPlusData &state);

        void report(EnergyPlusData &state);

    public:
        // data
        Real64 minPowerFlowFrac = 0.0; // Minimum fan air flow fraction for power calculation
        bool designElecPowerWasAutosized = false;
        PowerSizing powerSizingMethod = PowerSizing::Invalid; // sizing method for design electric power, three options
        Real64 elecPowerPerFlowRate = 0.0;                    // scaling factor for PowerPerFlow method
        Real64 elecPowerPerFlowRatePerPressure = 0.0;         // scaling factor for PowerPerFlowPerPressure
        Real64 nightVentPressureDelta = 0.0;                  // fan pressure rise during night ventilation mode
        Real64 nightVentFlowFraction = 0.0;                   // fan's flow fraction during night ventilation mode, not used
        int zoneNum = 0;                                      // zone index for motor heat losses as internal gains
        Real64 zoneRadFract = 0.0;                            // thermal radiation split for motor losses
        HeatLossDest heatLossDest = HeatLossDest::Invalid;    // enum for where motor loss go
        Real64 qdotConvZone = 0.0;                            // fan power lost to surrounding zone by convection to air (W)
        Real64 qdotRadZone = 0.0;                             // fan power lost to surrounding zone by radiation to zone surfaces(W)
        std::vector<Real64> powerFracAtSpeed;                 // array of power fractions for speed levels
        std::vector<bool> powerFracInputAtSpeed;
        // calculation variables
        std::vector<Real64> totalEffAtSpeed;
        std::vector<Real64> runtimeFracAtSpeed;

        Real64 designPointFEI = 0.0; // Fan Energy Index for the fan at the design operating point

    }; // class FanSystem

} // namespace Fans

struct FansData : BaseGlobalStruct
{
    int NumNightVentPerf = 0;    // number of FAN:NIGHT VENT PERFORMANCE objects found in the input
    bool GetFanInputFlag = true; // Flag set to make sure you get input once
    bool MyOneTimeFlag = true;
    bool ZoneEquipmentListChecked = false;

    Array1D<Fans::NightVentPerfData> NightVentPerf;
    int ErrCount = 0;

    Array1D<Fans::FanBase *> fans;
    std::map<std::string, int> fanMap;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        for (int i = 1; i <= (int)fans.size(); ++i) {
            delete fans(i);
        }

        fans.clear();
        fanMap.clear();

        new (this) FansData();
    }
};

} // namespace EnergyPlus

#endif
