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

#ifndef PlantChillers_hh_INCLUDED
#define PlantChillers_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantComponent.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PlantChillers {

    struct BaseChillerSpecs : PlantComponent // NOTE: This base class is abstract, derived classes must override pure virtual methods
    {
        // Members
        std::string Name;      // user identifier
        Real64 MinPartLoadRat; // (GT MIN) min allowed operating frac full load
        Real64 MaxPartLoadRat; // (GT MAX) max allowed operating frac full load
        Real64 OptPartLoadRat; // (GT BEST) optimal operating frac full load
        Real64 TempDesCondIn;  // C - (GT ADJTC(1)The design secondary loop fluid
        // temperature at the chiller condenser side inlet
        Real64 TempRiseCoef;                         // (GT ADJTC(2)) correction factor for off ChillDesign oper.
        Real64 TempDesEvapOut;                       // C - (GT ADJTC(3)The design primary loop fluid
        DataPlant::CondenserType CondenserType;      // Type of Condenser - Air or Water Cooled
        Real64 NomCap;                               // design nominal capacity of chiller
        bool NomCapWasAutoSized;                     // true if NomCap was autosize on input
        Real64 COP;                                  // COP
        DataPlant::FlowMode FlowMode;                // one of 3 modes for component flow during operation
        bool ModulatedFlowSetToLoop;                 // True if the setpoint is missing at the outlet node
        bool ModulatedFlowErrDone;                   // true if setpoint warning issued
        bool HRSPErrDone;                            // TRUE if set point warning issued for heat recovery loop
        int EvapInletNodeNum;                        // Node number on the inlet side of the plant
        int EvapOutletNodeNum;                       // Node number on the outlet side of the plant
        int CondInletNodeNum;                        // Node number on the inlet side of the condenser
        int CondOutletNodeNum;                       // Node number on the outlet side of the condenser
        Real64 EvapVolFlowRate;                      // m**3/s - design nominal water volumetric flow rate through the evaporator
        bool EvapVolFlowRateWasAutoSized;            // true if autosized design evap flow rate on input
        Real64 EvapMassFlowRateMax;                  // kg/s - design water mass flow rate through evaporator
        Real64 CondVolFlowRate;                      // m**3/s - design nominal water volumetric flow rate through the condenser
        bool CondVolFlowRateWasAutoSized;            // true if previous was autosized
        Real64 CondMassFlowRateMax;                  // kg/s - design water mass flow rate through condenser
        PlantLocation CWPlantLoc;                    // chilled water plant loop component index
        PlantLocation CDPlantLoc;                    // condenser water plant loop component index
        Real64 SizFac;                               // sizing factor
        Real64 BasinHeaterPowerFTempDiff;            // Basin heater capacity per degree C below setpoint (W/C)
        Real64 BasinHeaterSetPointTemp;              // Setpoint temperature for basin heater operation (C)
        Sched::Schedule *basinHeaterSched = nullptr; // basin heater schedule
        int ErrCount1;                               // for recurring error messages
        int ErrCount2;                               // for recurring error messages
        std::string MsgBuffer1;                      // - buffer to print warning messages on following time step
        std::string MsgBuffer2;                      // - buffer to print warning messages on following time step
        Real64 MsgDataLast;                          // value of data when warning occurred (passed to Recurring Warn)
        bool PrintMessage;                           // logical to determine if message is valid
        int MsgErrorCount;                           // number of occurrences of warning
        bool CheckEquipName;
        bool PossibleSubcooling; // flag to indicate chiller is doing less cooling that requested
        int CondMassFlowIndex;
        // Operational fault parameters
        bool FaultyChillerSWTFlag;         // True if the chiller has SWT sensor fault
        int FaultyChillerSWTIndex;         // Index of the fault object corresponding to the chiller
        Real64 FaultyChillerSWTOffset;     // Chiller SWT sensor offset
        bool FaultyChillerFoulingFlag;     // True if the chiller has fouling fault
        int FaultyChillerFoulingIndex;     // Index of the fault object corresponding to the chiller
        Real64 FaultyChillerFoulingFactor; // Chiller fouling factor
        bool MyFlag;
        bool MyEnvrnFlag;
        Real64 TimeStepSysLast;
        Real64 CurrentEndTimeLast;
        Real64 CondMassFlowRate;  // Kg/s - condenser mass flow rate, water side
        Real64 EvapMassFlowRate;  // Kg/s - evaporator mass flow rate, water side
        Real64 CondOutletTemp;    // C - condenser outlet temperature, air or water side
        Real64 EvapOutletTemp;    // C - evaporator outlet temperature, water side
        Real64 QEvaporator;       // W - rate of heat transfer to the evaporator coil
        Real64 QCondenser;        // W - rate of heat transfer to the condenser coil
        Real64 Energy;            // J - chiller energy use
        Real64 EvaporatorEnergy;  // J - rate of heat transfer to the evaporator coil
        Real64 CondenserEnergy;   // J - rate of heat transfer to the condenser coil
        Real64 QHeatRecovered;    // W - rate of heat transfer to the Heat Recovery coil
        Real64 HeatRecOutletTemp; // C - Heat Rec outlet temperature, water side
        Real64 AvgCondSinkTemp;   // condenser temperature value for use in curves [C]
        Real64 BasinHeaterPower;  // Basin heater power (W)
        Real64 Power;
        Real64 CondInletTemp;
        Real64 EvapInletTemp;
        Real64 BasinHeaterConsumption; // Basin heater energy consumption (J)
        DataPlant::PlantEquipmentType ChillerType;

        // Default Constructor
        BaseChillerSpecs()
            : MinPartLoadRat(0.0), MaxPartLoadRat(1.0), OptPartLoadRat(1.0), TempDesCondIn(0.0), TempRiseCoef(0.0), TempDesEvapOut(0.0),
              CondenserType(DataPlant::CondenserType::WaterCooled), NomCap(0.0), NomCapWasAutoSized(false), COP(0.0),
              FlowMode(DataPlant::FlowMode::Invalid), ModulatedFlowSetToLoop(false), ModulatedFlowErrDone(false), HRSPErrDone(false),
              EvapInletNodeNum(0), EvapOutletNodeNum(0), CondInletNodeNum(0), CondOutletNodeNum(0), EvapVolFlowRate(0.0),
              EvapVolFlowRateWasAutoSized(false), EvapMassFlowRateMax(0.0), CondVolFlowRate(0.0), CondVolFlowRateWasAutoSized(false),
              CondMassFlowRateMax(0.0), CWPlantLoc{}, CDPlantLoc{}, SizFac(0.0), BasinHeaterPowerFTempDiff(0.0), BasinHeaterSetPointTemp(0.0),
              ErrCount1(0), ErrCount2(0), MsgDataLast(0.0), PrintMessage(false), MsgErrorCount(0), CheckEquipName(true), PossibleSubcooling(false),
              CondMassFlowIndex(0), FaultyChillerSWTFlag(false), FaultyChillerSWTIndex(0), FaultyChillerSWTOffset(0.0),
              FaultyChillerFoulingFlag(false), FaultyChillerFoulingIndex(0), FaultyChillerFoulingFactor(1.0), MyFlag(true), MyEnvrnFlag(true),
              TimeStepSysLast(0.0), CurrentEndTimeLast(0.0), CondMassFlowRate(0.0), EvapMassFlowRate(0.0), CondOutletTemp(0.0),
              EvapOutletTemp(0.0),    // C - evaporator outlet temperature, water side
              QEvaporator(0.0),       // W - rate of heat transfer to the evaporator coil
              QCondenser(0.0),        // W - rate of heat transfer to the condenser coil
              Energy(0.0),            // J - chiller energy use
              EvaporatorEnergy(0.0),  // J - rate of heat transfer to the evaporator coil
              CondenserEnergy(0.0),   // J - rate of heat transfer to the condenser coil
              QHeatRecovered(0.0),    // W - rate of heat transfer to the Heat Recovery coil
              HeatRecOutletTemp(0.0), // C - Heat Rec outlet temperature, water side
              AvgCondSinkTemp(0.0),   // condenser temperature value for use in curves [C]
              BasinHeaterPower(0.0),  // Basin heater power (W)
              Power(0.0), CondInletTemp(0.0), EvapInletTemp(0.0), BasinHeaterConsumption(0.0), ChillerType(DataPlant::PlantEquipmentType::Invalid)

        {
        }

        void getDesignCapacities(EnergyPlusData &state,
                                 [[maybe_unused]] const PlantLocation &calledFromLocation,
                                 [[maybe_unused]] Real64 &MaxLoad,
                                 [[maybe_unused]] Real64 &MinLoad,
                                 [[maybe_unused]] Real64 &OptLoad) override;

        void getSizingFactor([[maybe_unused]] Real64 &SizFac) override;

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation) override;

        void getDesignTemperatures([[maybe_unused]] Real64 &TempDesCondIn, [[maybe_unused]] Real64 &TempDesEvapOut) override;

        virtual void initialize(EnergyPlusData &state, bool RunFlag, Real64 MyLoad) = 0;

        virtual void size(EnergyPlusData &state) = 0;
    };

    struct ElectricChillerSpecs : BaseChillerSpecs
    {
        // Members
        // temperature at the chiller evaporator side outlet
        Array1D<Real64> CapRatCoef;                        // (Electric RCAVC() ) coeff of cap ratio poly fit
        Array1D<Real64> PowerRatCoef;                      // (Electric ADJEC() ) coeff of power rat poly fit
        Array1D<Real64> FullLoadCoef;                      // (Electric RPWRC() ) coeff of full load poly. fit
        Real64 TempLowLimitEvapOut;                        // C - low temperature shut off
        Real64 DesignHeatRecVolFlowRate;                   // m3/s, Design Water mass flow rate through heat recovery loop
        bool DesignHeatRecVolFlowRateWasAutoSized;         // true if previous was input autosize.
        Real64 DesignHeatRecMassFlowRate;                  // kg/s, Design Water mass flow rate through heat recovery loop
        bool HeatRecActive;                                // True entered Heat Rec Vol Flow Rate >0
        int HeatRecInletNodeNum;                           // Node number on the heat recovery inlet side of the condenser
        int HeatRecOutletNodeNum;                          // Node number on the heat recovery outlet side of the condenser
        Real64 HeatRecCapacityFraction;                    // user input for heat recovery capacity fraction []
        Real64 HeatRecMaxCapacityLimit;                    // Capacity limit for Heat recovery, one time calc [W]
        int HeatRecSetPointNodeNum;                        // index for system node with the heat recover leaving setpoint
        Sched::Schedule *heatRecInletLimitSched = nullptr; // schedule for the inlet high limit for heat recovery operation
        PlantLocation HRPlantLoc;                          // heat recovery water plant loop component index
        std::string EndUseSubcategory;                     // identifier use for the end use subcategory
        Real64 CondOutletHumRat;                           // kg/kg - condenser outlet humidity ratio, air side
        Real64 ActualCOP;
        Real64 QHeatRecovery;
        Real64 EnergyHeatRecovery;
        Real64 HeatRecInletTemp; // (HeatRecOutletTemp is in base already)
        Real64 HeatRecMdot;
        Real64 ChillerCondAvgTemp; // the effective condenser temperature for chiller performance [C]
        Real64 partLoadRatio = 0.0;
        Real64 cyclingRatio = 0.0;

        // thermosiphon model
        int thermosiphonTempCurveIndex = 0;
        Real64 thermosiphonMinTempDiff = 0.0;
        int thermosiphonStatus = 0;

        // Default Constructor
        ElectricChillerSpecs()
            : CapRatCoef(3, 0.0), PowerRatCoef(3, 0.0), FullLoadCoef(3, 0.0), TempLowLimitEvapOut(0.0), DesignHeatRecVolFlowRate(0.0),
              DesignHeatRecVolFlowRateWasAutoSized(false), DesignHeatRecMassFlowRate(0.0), HeatRecActive(false), HeatRecInletNodeNum(0),
              HeatRecOutletNodeNum(0), HeatRecCapacityFraction(0.0), HeatRecMaxCapacityLimit(0.0), HeatRecSetPointNodeNum(0), HRPlantLoc{},
              CondOutletHumRat(0.0), ActualCOP(0.0), QHeatRecovery(0.0), EnergyHeatRecovery(0.0), HeatRecInletTemp(0.0), HeatRecMdot(0.0),
              ChillerCondAvgTemp(0.0)
        {
        }

        static void getInput(EnergyPlusData &state);

        void setupOutputVariables(EnergyPlusData &state);

        static ElectricChillerSpecs *factory(EnergyPlusData &state, std::string const &chillerName);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void initialize(EnergyPlusData &state, bool RunFlag, Real64 MyLoad) override;

        void size(EnergyPlusData &state) override;

        void calculate(EnergyPlusData &state,
                       Real64 &MyLoad,                                   // operating load
                       bool RunFlag,                                     // TRUE when chiller operating
                       DataBranchAirLoopPlant::ControlType EquipFlowCtrl // Flow control mode for the equipment
        );

        void update(EnergyPlusData &state,
                    Real64 MyLoad, // current load
                    bool RunFlag   // TRUE if chiller operating
        );

        void calcHeatRecovery(EnergyPlusData &state,
                              Real64 &QCond,        // current condenser load
                              Real64 CondMassFlow,  // current condenser Mass Flow
                              Real64 condInletTemp, // current condenser Inlet Temp
                              Real64 &QHeatRec      // amount of heat recovered
        );

        void oneTimeInit(EnergyPlusData &state) override;

        bool thermosiphonDisabled(EnergyPlusData &state);
    };

    struct EngineDrivenChillerSpecs : BaseChillerSpecs
    {
        // temperature at the chiller evaporator side outlet
        Constant::eFuel FuelType = Constant::eFuel::Invalid;
        Array1D<Real64> CapRatCoef;                // (EngineDriven RCAVC() ) coeff of cap ratio poly fit
        Array1D<Real64> PowerRatCoef;              // (EngineDriven ADJEC() ) coeff of power rat poly fit
        Array1D<Real64> FullLoadCoef;              // (EngineDriven RPWRC() ) coeff of full load poly. fit
        Real64 TempLowLimitEvapOut;                // C - low temperature shut off
        int ClngLoadtoFuelCurve;                   // Coeff of Shaft Power to Fuel Energy Input Coeff Poly Fit
        int RecJacHeattoFuelCurve;                 // Curve Index for Ratio of Recoverable Jacket Heat to
        int RecLubeHeattoFuelCurve;                // Curve Index for Ratio of Recoverable Lube Oil Heat to
        int TotExhausttoFuelCurve;                 // Curve Index for Total Exhaust heat Input to Fuel Energy Input Coeffs Poly Fit
        Real64 ExhaustTemp;                        // (TEXDC) Exhaust Gas Temp to Fuel Energy Input
        int ExhaustTempCurve;                      // Curve Index for Exhaust Gas Temp to Fuel Energy Input Coeffs Poly Fit
        Real64 UA;                                 // (UACDC) exhaust gas Heat Exchanger UA to Capacity
        Array1D<Real64> UACoef;                    // Heat Exchanger UA Coeffs Poly Fit
        Real64 MaxExhaustperPowerOutput;           // MAX EXHAUST FLOW PER W DSL POWER OUTPUT COEFF
        Real64 DesignMinExitGasTemp;               // Steam Saturation Temperature
        Real64 FuelHeatingValue;                   // Heating Value of Fuel in kJ/kg
        Real64 DesignHeatRecVolFlowRate;           // m3/s, Design Water mass flow rate through heat recovery loop
        bool DesignHeatRecVolFlowRateWasAutoSized; // true if user input was autosize for heat recover design flow rate
        Real64 DesignHeatRecMassFlowRate;          // kg/s, Design Water mass flow rate through heat recovery loop
        bool HeatRecActive;                        // True entered Heat Rec Vol Flow Rate >0
        int HeatRecInletNodeNum;                   // Node number on the heat recovery inlet side of the condenser
        int HeatRecOutletNodeNum;                  // Node number on the heat recovery outlet side of the condenser
        Real64 HeatRecCapacityFraction;            // user input for heat recovery capacity fraction []
        Real64 HeatRecMaxTemp;                     // Max Temp that can be produced in heat recovery
        PlantLocation HRPlantLoc;                  // heat recovery water plant loop component index

        // engine driven:
        Real64 HeatRecInletTemp;    // Inlet Temperature of the heat recovery fluid
        Real64 HeatRecMdotActual;   // Heat Recovery Loop Mass flow rate
        Real64 QTotalHeatRecovered; // total heat recovered (W)
        Real64 QJacketRecovered;    // heat recovered from jacket (W)
        Real64 QLubeOilRecovered;   // heat recovered from lube (W)
        Real64 QExhaustRecovered;   // exhaust gas heat recovered (W)
        Real64 FuelEnergyUseRate;   // Fuel Energy used (W)
        Real64 TotalHeatEnergyRec;  // total heat recovered (J)
        Real64 JacketEnergyRec;     // heat recovered from jacket (J)
        Real64 LubeOilEnergyRec;    // heat recovered from lube (J)
        Real64 ExhaustEnergyRec;    // exhaust gas heat recovered (J)
        Real64 FuelEnergy;          // Fuel Energy used (J)
        Real64 FuelMdot;            // Fuel Amount used (Kg/s)
        Real64 ExhaustStackTemp;    // Exhaust Stack Temperature (C)

        Real64 HeatRecMdot; // Heat Recovery Loop Mass flow rate (kg/s)
        Real64 FuelCOP;     // Fuel COP [delivered cooling rate/fuel energy input rate] (W/W)

        // Default Constructor
        EngineDrivenChillerSpecs()
            : CapRatCoef(3, 0.0), PowerRatCoef(3, 0.0), FullLoadCoef(3, 0.0), TempLowLimitEvapOut(0.0), ClngLoadtoFuelCurve(0),
              RecJacHeattoFuelCurve(0), RecLubeHeattoFuelCurve(0), TotExhausttoFuelCurve(0), ExhaustTemp(0.0), ExhaustTempCurve(0), UA(0.0),
              UACoef(2, 0.0), MaxExhaustperPowerOutput(0.0), DesignMinExitGasTemp(0.0), FuelHeatingValue(0.0), DesignHeatRecVolFlowRate(0.0),
              DesignHeatRecVolFlowRateWasAutoSized(false), DesignHeatRecMassFlowRate(0.0), HeatRecActive(false), HeatRecInletNodeNum(0),
              HeatRecOutletNodeNum(0), HeatRecCapacityFraction(0.0), HeatRecMaxTemp(0.0), HRPlantLoc{}, HeatRecInletTemp(0.0), HeatRecMdotActual(0.0),
              QTotalHeatRecovered(0.0), QJacketRecovered(0.0),

              // engine driven:
              QLubeOilRecovered(0.0), QExhaustRecovered(0.0), FuelEnergyUseRate(0.0), TotalHeatEnergyRec(0.0), JacketEnergyRec(0.0),
              LubeOilEnergyRec(0.0), ExhaustEnergyRec(0.0), FuelEnergy(0.0), FuelMdot(0.0), ExhaustStackTemp(0.0), HeatRecMdot(0.0), FuelCOP(0.0)
        {
        }

        static EngineDrivenChillerSpecs *factory(EnergyPlusData &state, std::string const &chillerName);

        static void getInput(EnergyPlusData &state);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void setupOutputVariables(EnergyPlusData &state);

        void initialize(EnergyPlusData &state, bool RunFlag, Real64 MyLoad) override;

        void size(EnergyPlusData &state) override;

        void calculate(EnergyPlusData &state,
                       Real64 &MyLoad,                                   // operating load
                       bool RunFlag,                                     // TRUE when chiller operating
                       DataBranchAirLoopPlant::ControlType EquipFlowCtrl // Flow control mode for the equipment
        );

        void calcHeatRecovery(EnergyPlusData &state,
                              Real64 EnergyRecovered, // Amount of heat recovered
                              Real64 &HeatRecRatio    // Max Heat recovery ratio
        );

        void update(EnergyPlusData &state,
                    Real64 MyLoad, // current load
                    bool RunFlag   // TRUE if chiller operating
        );

        void oneTimeInit(EnergyPlusData &state) override;
    };

    struct GTChillerSpecs : BaseChillerSpecs
    {
        // Members
        Constant::eFuel FuelType = Constant::eFuel::Invalid;
        Array1D<Real64> CapRatCoef;   // (GT RCAVC() ) coeff of cap ratio poly fit
        Array1D<Real64> PowerRatCoef; // (GT ADJEC() ) coeff of power rat poly fit
        Array1D<Real64> FullLoadCoef; // (GT RPWRC() ) coeff of full load poly. fit
        Real64 TempLowLimitEvapOut;   // C - low temperature shut off
        // "special" GT chiller input parameters
        Real64 FuelEnergyIn;                      // (EFUEL) Amount of Fuel Energy Required to run gas turbine
        Array1D<Real64> PLBasedFuelInputCoef;     // (FUL1GC) Part Load Ratio Based Fuel Input Coefficients Poly Fit
        Array1D<Real64> TempBasedFuelInputCoef;   // (FUL2GC) Ambient Temperature Based Fuel Input Coeff Poly Fit
        Real64 ExhaustFlow;                       // (FEX) Exhaust Gas Flow Rate cubic meters per second
        Array1D<Real64> ExhaustFlowCoef;          // (FEXGC) Exhaust Gas Flow Rate Input Coef Poly Fit
        Real64 ExhaustTemp;                       // (TEX) Exhaust Gas Temperature in C
        Array1D<Real64> PLBasedExhaustTempCoef;   // (TEX1GC) Part Load Ratio Based Exhaust Temperature Input Coeffs Poly Fit
        Array1D<Real64> TempBasedExhaustTempCoef; // (TEX2GC) Ambient Temperature Based Exhaust Gas Temp to
        // Fuel Energy Input Coeffs Poly Fit
        Real64 HeatRecLubeEnergy;                  // (ELUBE) Recoverable Lube Oil Energy
        Real64 HeatRecLubeRate;                    // (ELUBE) Recoverable Lube Oil Rate of Recovery (W)
        Array1D<Real64> HeatRecLubeEnergyCoef;     // (ELUBEGC)  Recoverable Lube Oil Energy Input Coef Poly Fit
        Real64 UAtoCapRat;                         // (UACGC) Heat Exchanger UA to Capacity
        Array1D<Real64> UAtoCapCoef;               // Heat Exchanger UA to Capacity Coeffs Poly Fit
        Real64 GTEngineCapacity;                   // Capacity of GT Unit attached to Chiller
        bool GTEngineCapacityWasAutoSized;         // true if previous field was autosize on input
        Real64 MaxExhaustperGTPower;               // Max Exhaust Flow per KW Power Out
        Real64 DesignSteamSatTemp;                 // Steam Saturation Temperature
        Real64 ExhaustStackTemp;                   // Temperature of Exhaust Gases
        int HeatRecInletNodeNum;                   // Node number on the heat recovery inlet side of the condenser
        int HeatRecOutletNodeNum;                  // Node number on the heat recovery outlet side of the condenser
        Real64 HeatRecInletTemp;                   // Inlet Temperature of the heat recovery fluid (HeatRecOutletTemp is in base already)
        Real64 HeatRecMdot;                        // Heat Recovery Loop Mass flow rate
        Real64 DesignHeatRecVolFlowRate;           // m3/s, Design Water mass flow rate through heat recovery loop
        bool DesignHeatRecVolFlowRateWasAutoSized; // true if previous field was autosize on input
        Real64 DesignHeatRecMassFlowRate;          // kg/s, Design Water mass flow rate through heat recovery loop
        bool HeatRecActive;                        // True entered Heat Rec Vol Flow Rate >0
        Real64 FuelHeatingValue;                   // Heating Value of Fuel in kJ/kg
        Real64 HeatRecCapacityFraction;            // user input for heat recovery capacity fraction []
        Real64 engineCapacityScalar;               // user input for engine efficiency for sizing GTEngineCapacity []
        Real64 HeatRecMaxTemp;                     // Max Temp that can be produced in heat recovery
        PlantLocation HRPlantLoc;                  // heat recovery water plant loop component index

        Real64 FuelEnergyUsed;     // Fuel Energy used
        Real64 FuelEnergyUsedRate; // Fuel energy used rate (fuel consumption rate)
        Real64 FuelMassUsed;       // Fuel Amount used
        Real64 FuelMassUsedRate;   // Fuel amount used (fuel Mass consumption rate)
        Real64 FuelCOP;            // Fuel coefficient of performance (Qevap/FuelEnergyUsedRate)

        // Default Constructor
        GTChillerSpecs()
            : CapRatCoef(3, 0.0), PowerRatCoef(3, 0.0), FullLoadCoef(3, 0.0), TempLowLimitEvapOut(0.0), FuelEnergyIn(0.0),
              PLBasedFuelInputCoef(3, 0.0), TempBasedFuelInputCoef(3, 0.0), ExhaustFlow(0.0), ExhaustFlowCoef(3, 0.0), ExhaustTemp(0.0),
              PLBasedExhaustTempCoef(3, 0.0), TempBasedExhaustTempCoef(3, 0.0), HeatRecLubeEnergy(0.0), HeatRecLubeRate(0.0),
              HeatRecLubeEnergyCoef(3, 0.0), UAtoCapRat(0.0), UAtoCapCoef(3, 0.0), GTEngineCapacity(0.0), GTEngineCapacityWasAutoSized(false),
              MaxExhaustperGTPower(0.0), DesignSteamSatTemp(0.0), ExhaustStackTemp(0.0), HeatRecInletNodeNum(0), HeatRecOutletNodeNum(0),
              HeatRecInletTemp(0.0), HeatRecMdot(0.0), DesignHeatRecVolFlowRate(0.0), DesignHeatRecVolFlowRateWasAutoSized(false),
              DesignHeatRecMassFlowRate(0.0), HeatRecActive(false), FuelHeatingValue(0.0), HeatRecCapacityFraction(0.0), engineCapacityScalar(0.35),
              HeatRecMaxTemp(0.0), HRPlantLoc{}, FuelEnergyUsed(0.0), FuelEnergyUsedRate(0.0), FuelMassUsed(0.0), FuelMassUsedRate(0.0), FuelCOP(0.0)
        {
        }

        static GTChillerSpecs *factory(EnergyPlusData &state, std::string const &chillerName);

        static void getInput(EnergyPlusData &state);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void setupOutputVariables(EnergyPlusData &state);

        void initialize(EnergyPlusData &state, bool RunFlag, Real64 MyLoad) override;

        void size(EnergyPlusData &state) override;

        void calculate(EnergyPlusData &state,
                       Real64 &MyLoad,                                   // operating load
                       bool RunFlag,                                     // TRUE when chiller operating
                       DataBranchAirLoopPlant::ControlType EquipFlowCtrl // Flow control mode for the equipment
        );

        void update(EnergyPlusData &state,
                    Real64 MyLoad, // current load
                    bool RunFlag   // TRUE if chiller operating
        );

        void oneTimeInit(EnergyPlusData &state) override;
    };

    struct ConstCOPChillerSpecs : BaseChillerSpecs
    {
        // Members
        Real64 ActualCOP;
        Real64 partLoadRatio = 0.0;
        Real64 cyclingRatio = 1.0;

        // thermosiphon model
        int thermosiphonTempCurveIndex = 0;
        Real64 thermosiphonMinTempDiff = 0.0;
        int thermosiphonStatus = 0;

        // Default Constructor
        ConstCOPChillerSpecs() : ActualCOP(0.0)
        {
        }

        static ConstCOPChillerSpecs *factory(EnergyPlusData &state, std::string const &chillerName);

        static void getInput(EnergyPlusData &state);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void setupOutputVariables(EnergyPlusData &state);

        void initialize(EnergyPlusData &state, bool RunFlag, Real64 MyLoad) override;

        void size(EnergyPlusData &state) override;

        void calculate(EnergyPlusData &state, Real64 &MyLoad, bool RunFlag, DataBranchAirLoopPlant::ControlType EquipFlowCtrl);

        void update(EnergyPlusData &state, Real64 MyLoad, bool RunFlag);

        void oneTimeInit(EnergyPlusData &state) override;

        bool thermosiphonDisabled(EnergyPlusData &state);
    };
} // namespace PlantChillers

struct PlantChillersData : BaseGlobalStruct
{

    int NumElectricChillers = 0;
    int NumEngineDrivenChillers = 0;
    int NumGTChillers = 0;
    int NumConstCOPChillers = 0;

    bool GetEngineDrivenInput = true;
    bool GetElectricInput = true;
    bool GetGasTurbineInput = true;
    bool GetConstCOPInput = true;

    EPVector<PlantChillers::ElectricChillerSpecs> ElectricChiller;
    EPVector<PlantChillers::EngineDrivenChillerSpecs> EngineDrivenChiller;
    EPVector<PlantChillers::GTChillerSpecs> GTChiller;
    EPVector<PlantChillers::ConstCOPChillerSpecs> ConstCOPChiller;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        NumElectricChillers = 0;
        NumEngineDrivenChillers = 0;
        NumGTChillers = 0;
        NumConstCOPChillers = 0;
        GetEngineDrivenInput = true;
        GetElectricInput = true;
        GetGasTurbineInput = true;
        GetConstCOPInput = true;
        ElectricChiller.deallocate();
        EngineDrivenChiller.deallocate();
        GTChiller.deallocate();
        ConstCOPChiller.deallocate();
    }
};

} // namespace EnergyPlus

#endif
