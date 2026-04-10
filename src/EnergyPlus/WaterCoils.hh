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

#ifndef WaterCoils_hh_INCLUDED
#define WaterCoils_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array2A.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HVACControllers.hh>
#include <EnergyPlus/Plant/DataPlant.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace WaterCoils {

    constexpr int MaxPolynomOrder = 4;
    constexpr int MaxOrderedPairs = 60;
    constexpr Real64 PolyConvgTol = 1.E-05;
    constexpr Real64 MinWaterMassFlowFrac = 0.000001;
    constexpr Real64 MinAirMassFlow = 0.001;

    enum class CoilModel
    {
        Invalid = -1,
        HeatingSimple,
        CoolingSimple,
        CoolingDetailed,
        Num
    };

    struct WaterCoilEquipConditions
    {
        // Members
        std::string Name;                            // Name of the WaterCoil
        std::string WaterCoilTypeA;                  // Type of WaterCoil ie. Heating or Cooling
        std::string WaterCoilModelA;                 // Type of WaterCoil ie. Simple, Detailed, etc.
        DataPlant::PlantEquipmentType WaterCoilType; // Type of WaterCoil ie. Heating or Cooling
        CoilModel WaterCoilModel;                    // Type of WaterCoil ie. Simple, Detailed, etc.
        Sched::Schedule *availSched = nullptr;       // availability schedule
        bool RequestingAutoSize;                     // True if this coil has appropriate autosize fields
        Real64 InletAirMassFlowRate;                 // MassFlow through the WaterCoil being Simulated [kg/s]
        Real64 OutletAirMassFlowRate;                // MassFlow through the WaterCoil being Simulated[kg/s]
        Real64 InletAirTemp;                         // Inlet Air Temperature Operating Condition [C]
        Real64 OutletAirTemp;                        // Outlet Air Temperature Operating Condition [C]
        Real64 InletAirHumRat;                       // Inlet Air Humidity Ratio Operating Condition
        Real64 OutletAirHumRat;                      // Outlet Air Humidity Ratio Calculated Condition
        Real64 InletAirEnthalpy;                     // Inlet Air enthalpy [J/kg]
        Real64 OutletAirEnthalpy;                    // Outlet Air enthalpy [J/kg]
        Real64 TotWaterCoilLoad;                     // Total Load on the Coil [W]
        Real64 SenWaterCoilLoad;                     // Sensible Load on the Coil [W]
        Real64 TotWaterHeatingCoilEnergy;            // Total Heating Coil energy of the Coil [J]
        Real64 TotWaterCoolingCoilEnergy;            // Total Cooling Coil energy of the Coil [J]
        Real64 SenWaterCoolingCoilEnergy;            // Sensible Cooling Coil energy of the Coil [J]
        Real64 DesWaterHeatingCoilRate;              // Design Heating Coil Rate used for sizing [W]
        Real64 TotWaterHeatingCoilRate;              // Total Heating Coil Rate on the Coil [W]
        Real64 DesWaterCoolingCoilRate;              // Design Cooling Coil Rate used for sizing [W]
        Real64 TotWaterCoolingCoilRate;              // Total Cooling Coil Rate on the Coil [W]
        Real64 SenWaterCoolingCoilRate;              // Sensible Cooling Coil Rate on the Coil [W]
        Real64 UACoil;                               // WaterCoil UA Value
        Real64 LeavingRelHum;                        // Simple Coil Latent Model requires User input for leaving RH
        Real64 DesiredOutletTemp;
        Real64 DesiredOutletHumRat;
        Real64 InletWaterTemp;          // Inlet Water Temperature [C]
        Real64 OutletWaterTemp;         // Outlet Water Temperature [C]
        Real64 InletWaterMassFlowRate;  // Inlet Water Mass Flow Rate [Kg/s]
        Real64 OutletWaterMassFlowRate; // Outlet Water Mass Flow Rate [Kg/s]
        Real64 MaxWaterVolFlowRate;     // Maximum water Volume flow rate [m3/s]
        Real64 MaxWaterMassFlowRate;    // Maximum water mass flow rate [Kg/s]
        Real64 InletWaterEnthalpy;      // Inlet Water Enthalpy
        Real64 OutletWaterEnthalpy;     // Outlet Water Enthalpy
        // These are the additional Geometry and Design Variables for Detailed Flat Fin Coil
        Real64 TubeOutsideSurfArea; // Tube Primary Surface Area
        Real64 TotTubeInsideArea;   // Total Tube inside Surface Area
        Real64 FinSurfArea;         // Fin Surface Area
        Real64 MinAirFlowArea;
        Real64 CoilDepth;
        Real64 FinDiam; // Fin Diameter or the Coil Height
        Real64 FinThickness;
        Real64 TubeInsideDiam;  // Inner diameter of Tubes
        Real64 TubeOutsideDiam; // Outer Diameter of the Tubes
        Real64 TubeThermConductivity;
        Real64 FinThermConductivity;
        Real64 FinSpacing; // Fin Spacing or Distance
        Real64 TubeDepthSpacing;
        int NumOfTubeRows;
        int NumOfTubesPerRow;
        // BEGIN calculated parameters for detailed flat fin coil
        Real64 EffectiveFinDiam;
        Real64 TotCoilOutsideSurfArea;
        Real64 CoilEffectiveInsideDiam;
        Real64 GeometryCoef1;
        Real64 GeometryCoef2;
        Array1D<Real64> DryFinEfficncyCoef;
        Real64 SatEnthlCurveConstCoef;
        Real64 SatEnthlCurveSlope;
        Real64 EnthVsTempCurveAppxSlope;
        Real64 EnthVsTempCurveConst;
        Real64 MeanWaterTempSaved;
        Real64 InWaterTempSaved;
        Real64 OutWaterTempSaved;
        Real64 SurfAreaWetSaved;
        Real64 SurfAreaWetFraction;
        // END calculated parameters for detailed flat fin coil
        // Design Input Variables to the Design Detailed Simple inputs model
        Real64 DesInletWaterTemp;   // Entering water temperature at Design(C)
        Real64 DesAirVolFlowRate;   // Entering Air Volume Flow Rate Design (m3/s)
        Real64 DesInletAirTemp;     // Entering air dry bulb temperature at Design(C)
        Real64 DesInletAirHumRat;   // Entering air humidity ratio at design conditions
        Real64 DesTotWaterCoilLoad; // Total heat transfer rate at Design(Watt)
        Real64 DesSenWaterCoilLoad; // Sensible heat transfer rate at Design(Watt)
        // BEGIN calculated parameters for Design Detailed Simple inputs model
        Real64 DesAirMassFlowRate;   // Design Air MassFlow through the WaterCoil [kg/Sec]
        Real64 UACoilTotal;          // Overall external dry UA (W/C)
        Real64 UACoilInternal;       // Overall internal UA(W/C)
        Real64 UACoilExternal;       // Overall external heat transfer coefficient(W/C)
        Real64 UACoilInternalDes;    // Overall design internal UA(W/C)
        Real64 UACoilExternalDes;    // Overall design external heat transfer coefficient(W/C)
        Real64 DesOutletAirTemp;     // Leaving air temperature at rating(C)
        Real64 DesOutletAirHumRat;   // Humidity ratio of air leaving at design capacity.
        Real64 DesOutletWaterTemp;   // Temp of Liquid Leaving the Coil at design Capacity
        int HeatExchType;            // Heat exchanger configuration, default to Cross Flow
        int CoolingCoilAnalysisMode; // Mode Of analysis, Simple=1 and Detailed =2
        //    Simple= AllWet-AllDry, Detailed= PartWet-PartDry
        Real64 UACoilInternalPerUnitArea; // Internal overall heat transfer coefficient(W/m2 C)
        Real64 UAWetExtPerUnitArea;       // External overall heat transfer coefficient(W/m2 C)
        Real64 UADryExtPerUnitArea;       // External overall heat transfer coefficient(W/m2 C)
        Real64 SurfAreaWetFractionSaved;  // Previous saved value, for numerical efficiency.
        // END calculated parameters for Design Inputs Detailed coil
        // variables for simple heating coil with variable UA
        Real64 UACoilVariable;                 // WaterCoil UA value when variable (simple heating coil only)
        Real64 RatioAirSideToWaterSideConvect; // "r" value for coil,
        Real64 AirSideNominalConvect;          // nominal rating point air side convection term (fin_effic*(hc*A))
        Real64 LiquidSideNominalConvect;       // nominal rating point water side convection term (hc*A)
        int Control;                           // Const Vol =1;  Variable Vol = 2
        int AirInletNodeNum;
        int AirOutletNodeNum;
        int WaterInletNodeNum;
        int WaterOutletNodeNum;
        PlantLocation WaterPlantLoc; // Plant location struct
        // begin variables for Water System interactions
        int CondensateCollectMode;         // where does water come from
        std::string CondensateCollectName; // name of water source e.g. water storage tank
        int CondensateTankID;              // index "pointer" to Storage TAnk array WaterStorage
        int CondensateTankSupplyARRID;     // index point to supply Vdot array in WaterStorage
        Real64 CondensateVdot;             // rate of water condensation from air stream [m3/s]
        Real64 CondensateVol;              // amount of water condensed from air stream [m3]
        // end variables for water system interactions
        // COIL:Water:SimpleHeating Coil Performance Input Method
        int CoilPerfInpMeth; // 1 = UA and Design Water Flow Rate; 2 = Nominal Capacity

        // Operational fault parameters
        bool FaultyCoilFoulingFlag;     // True if the coil has fouling fault
        int FaultyCoilFoulingIndex;     // Index of the fault object corresponding to the coil
        Real64 FaultyCoilFoulingFactor; // Coil fouling factor [m2K/W]
        Real64 OriginalUACoilVariable;
        Real64 OriginalUACoilExternal;
        Real64 OriginalUACoilInternal;

        bool DesiccantRegenerationCoil; // true if it is a regeneration air heating coil defined in Desiccant Dehumidifier system
        int DesiccantDehumNum;          // index to desiccant dehumidifier object
        Real64 DesignWaterDeltaTemp;    // water deltaT for coil sizing [K]
        bool UseDesignWaterDeltaTemp;   // is true, the DesignWaterDeltaTemp is used for sizing coil design water flow rate
        std::string ControllerName;     // controller name used by water coil
        int ControllerIndex;            // controller index used by water coil
        bool reportCoilFinalSizes;      // one time report of sizes to coil summary report
        bool AirLoopDOASFlag;           // True when this coil is used AirLoopDOAS
        bool heatRecoveryCoil;          // is true when coils are connected to each other to create a heat recovery loop

        General::SolveRootStats solveRootStats{};
        // Default Constructor
        WaterCoilEquipConditions()
            : WaterCoilType(DataPlant::PlantEquipmentType::Invalid), WaterCoilModel(CoilModel::Invalid), RequestingAutoSize(false),
              InletAirMassFlowRate(0.0), OutletAirMassFlowRate(0.0), InletAirTemp(0.0), OutletAirTemp(0.0), InletAirHumRat(0.0), OutletAirHumRat(0.0),
              InletAirEnthalpy(0.0), OutletAirEnthalpy(0.0), TotWaterCoilLoad(0.0), SenWaterCoilLoad(0.0), TotWaterHeatingCoilEnergy(0.0),
              TotWaterCoolingCoilEnergy(0.0), SenWaterCoolingCoilEnergy(0.0), DesWaterHeatingCoilRate(0.0), TotWaterHeatingCoilRate(0.0),
              DesWaterCoolingCoilRate(0.0), TotWaterCoolingCoilRate(0.0), SenWaterCoolingCoilRate(0.0), UACoil(0.0), LeavingRelHum(0.0),
              DesiredOutletTemp(0.0), DesiredOutletHumRat(0.0), InletWaterTemp(0.0), OutletWaterTemp(0.0), InletWaterMassFlowRate(0.0),
              OutletWaterMassFlowRate(0.0), MaxWaterVolFlowRate(0.0), MaxWaterMassFlowRate(0.0), InletWaterEnthalpy(0.0), OutletWaterEnthalpy(0.0),
              TubeOutsideSurfArea(0.0), TotTubeInsideArea(0.0), FinSurfArea(0.0), MinAirFlowArea(0.0), CoilDepth(0.0), FinDiam(0.0),
              FinThickness(0.0), TubeInsideDiam(0.0), TubeOutsideDiam(0.0), TubeThermConductivity(0.0), FinThermConductivity(0.0), FinSpacing(0.0),
              TubeDepthSpacing(0.0), NumOfTubeRows(0), NumOfTubesPerRow(0), EffectiveFinDiam(0.0), TotCoilOutsideSurfArea(0.0),
              CoilEffectiveInsideDiam(0.0), GeometryCoef1(0.0), GeometryCoef2(0.0), DryFinEfficncyCoef(5, 0.0), SatEnthlCurveConstCoef(0.0),
              SatEnthlCurveSlope(0.0), EnthVsTempCurveAppxSlope(0.0), EnthVsTempCurveConst(0.0), MeanWaterTempSaved(0.0), InWaterTempSaved(0.0),
              OutWaterTempSaved(0.0), SurfAreaWetSaved(0.0), SurfAreaWetFraction(0.0), DesInletWaterTemp(0.0), DesAirVolFlowRate(0.0),
              DesInletAirTemp(0.0), DesInletAirHumRat(0.0), DesTotWaterCoilLoad(0.0), DesSenWaterCoilLoad(0.0), DesAirMassFlowRate(0.0),
              UACoilTotal(0.0), UACoilInternal(0.0), UACoilExternal(0.0), UACoilInternalDes(0.0), UACoilExternalDes(0.0), DesOutletAirTemp(0.0),
              DesOutletAirHumRat(0.0), DesOutletWaterTemp(0.0), HeatExchType(0), CoolingCoilAnalysisMode(0), UACoilInternalPerUnitArea(0.0),
              UAWetExtPerUnitArea(0.0), UADryExtPerUnitArea(0.0), SurfAreaWetFractionSaved(0.0), UACoilVariable(0.0),
              RatioAirSideToWaterSideConvect(1.0), AirSideNominalConvect(0.0), LiquidSideNominalConvect(0.0), Control(0), AirInletNodeNum(0),
              AirOutletNodeNum(0), WaterInletNodeNum(0), WaterOutletNodeNum(0), WaterPlantLoc{}, CondensateCollectMode(1001), CondensateTankID(0),
              CondensateTankSupplyARRID(0), CondensateVdot(0.0), CondensateVol(0.0), CoilPerfInpMeth(0), FaultyCoilFoulingFlag(false),
              FaultyCoilFoulingIndex(0), FaultyCoilFoulingFactor(0.0), OriginalUACoilVariable(0.0), OriginalUACoilExternal(0.0),
              OriginalUACoilInternal(0.0), DesiccantRegenerationCoil(false), DesiccantDehumNum(0), DesignWaterDeltaTemp(0.0),
              UseDesignWaterDeltaTemp(false), ControllerIndex(0), reportCoilFinalSizes(true), AirLoopDOASFlag(false), heatRecoveryCoil(false)
        {
        }
    };

    struct WaterCoilNumericFieldData
    {
        // Members
        Array1D_string FieldNames;

        // Default Constructor
        WaterCoilNumericFieldData()
        {
        }
    };

    // Functions

    void SimulateWaterCoilComponents(EnergyPlusData &state,
                                     std::string_view CompName,
                                     bool const FirstHVACIteration,
                                     int &CompIndex,
                                     ObjexxFCL::Optional<Real64> QActual = _,
                                     ObjexxFCL::Optional<HVAC::FanOp const> fanOp = _,
                                     ObjexxFCL::Optional<Real64 const> PartLoadRatio = _);

    void GetWaterCoilInput(EnergyPlusData &state);

    void InitWaterCoil(EnergyPlusData &state, int const CoilNum, bool const FirstHVACIteration);

    void                                                          // refactor for coil report
    CalcAdjustedCoilUA(EnergyPlusData &state, int const CoilNum); // refactor for coil report

    void SizeWaterCoil(EnergyPlusData &state, int const CoilNum);

    void CalcSimpleHeatingCoil(EnergyPlusData &state,
                               int const CoilNum,          // index to heating coil
                               HVAC::FanOp const fanOp,    // fan operating mode
                               Real64 const PartLoadRatio, // part-load ratio of heating coil
                               int const CalcMode          // 1 = design calc; 2 = simulation calculation
    );

    void CalcDetailFlatFinCoolingCoil(EnergyPlusData &state,
                                      int const CoilNum,
                                      int const CalcMode,
                                      HVAC::FanOp const fanOp,   // fan operating mode
                                      Real64 const PartLoadRatio // part-load ratio of heating coil
    );

    void CoolingCoil(EnergyPlusData &state,
                     int const CoilNum,
                     bool const FirstHVACIteration,
                     int const CalcMode,
                     HVAC::FanOp const fanOp,   // fan operating mode
                     Real64 const PartLoadRatio // part-load ratio of heating coil
    );

    void CoilCompletelyDry(EnergyPlusData &state,
                           int const CoilNum,
                           Real64 const WaterTempIn,  // Entering water temperature
                           Real64 const AirTempIn,    // Entering air dry bulb temperature
                           Real64 const CoilUA,       // Overall heat transfer coefficient
                           Real64 &OutletWaterTemp,   // Leaving water temperature
                           Real64 &OutletAirTemp,     // Leaving air dry bulb temperature
                           Real64 &OutletAirHumRat,   // Leaving air humidity ratio
                           Real64 &Q,                 // Heat transfer rate
                           HVAC::FanOp const fanOp,   // fan operating mode
                           Real64 const PartLoadRatio // part-load ratio of heating coil
    );

    // Coil Completely Wet Subroutine for Cooling Coil

    void CoilCompletelyWet(EnergyPlusData &state,
                           int const CoilNum,            // Number of Coil
                           Real64 const WaterTempIn,     // Water temperature IN to this function (C)
                           Real64 const AirTempIn,       // Air dry bulb temperature IN to this function(C)
                           Real64 const AirHumRat,       // Air Humidity Ratio IN to this function (C)
                           Real64 const UAInternalTotal, // Internal overall heat transfer coefficient(W/m2 C)
                           Real64 const UAExternalTotal, // External overall heat transfer coefficient(W/m2 C)
                           Real64 &OutletWaterTemp,      // Leaving water temperature (C)
                           Real64 &OutletAirTemp,        // Leaving air dry bulb temperature(C)
                           Real64 &OutletAirHumRat,      // Leaving air humidity ratio
                           Real64 &TotWaterCoilLoad,     // Total heat transfer rate(W)
                           Real64 &SenWaterCoilLoad,     // Sensible heat transfer rate(W)
                           Real64 &SurfAreaWetFraction,  // Fraction of surface area wet
                           Real64 &AirInletCoilSurfTemp, // Surface temperature at air entrance(C)
                           HVAC::FanOp const fanOp,      // fan operating mode
                           Real64 const PartLoadRatio    // part-load ratio of heating coil
    );

    // Coil Part Wet Part Dry Subroutine for Cooling Coil

    void CoilPartWetPartDry(EnergyPlusData &state,
                            int const CoilNum,             // Number of Coil
                            bool const FirstHVACIteration, // Saving Old values
                            Real64 const InletWaterTemp,   // Entering liquid temperature(C)
                            Real64 const InletAirTemp,     // Entering air dry bulb temperature(C)
                            Real64 const AirDewPointTemp,  // Entering air dew point(C)
                            Real64 &OutletWaterTemp,       // Leaving liquid temperature(C)
                            Real64 &OutletAirTemp,         // Leaving air dry bulb temperature(C)
                            Real64 &OutletAirHumRat,       // Leaving air humidity ratio
                            Real64 &TotWaterCoilLoad,      // Total heat transfer rate (W)
                            Real64 &SenWaterCoilLoad,      // Sensible heat transfer rate (W)
                            Real64 &SurfAreaWetFraction,   // Fraction of surface area wet
                            HVAC::FanOp const fanOp,       // fan operating mode
                            Real64 const PartLoadRatio     // part-load ratio of heating coil
    );

    // Calculating coil UA for Cooling Coil

    Real64 CalcCoilUAbyEffectNTU(EnergyPlusData &state,
                                 int const CoilNum,
                                 Real64 const CapacityStream1,     // Capacity rate of stream1.(W/C)
                                 Real64 const EnergyInStreamOne,   // Inlet state of stream1.(C)
                                 Real64 const CapacityStream2,     // Capacity rate of stream2.(W/C)
                                 Real64 const EnergyInStreamTwo,   // Inlet state of stream2.(C)
                                 Real64 const DesTotalHeatTransfer // Heat transfer rate(W)
    );

    // Calculating coil outlet stream conditions and coil UA for Cooling Coil

    void CoilOutletStreamCondition(EnergyPlusData &state,
                                   int const CoilNum,
                                   Real64 const CapacityStream1,   // Capacity rate of stream1(W/C)
                                   Real64 const EnergyInStreamOne, // Inlet state of stream1 (C)
                                   Real64 const CapacityStream2,   // Capacity rate of stream2 (W/C)
                                   Real64 const EnergyInStreamTwo, // Inlet state of stream2 (C)
                                   Real64 const CoilUA,            // Heat transfer rateW)
                                   Real64 &EnergyOutStreamOne,     // Outlet state of stream1 (C)
                                   Real64 &EnergyOutStreamTwo      // Outlet state of stream2 (C)
    );

    // Subroutine for calculating outlet condition if coil is wet , for Cooling Coil

    void WetCoilOutletCondition(EnergyPlusData &state,
                                int const CoilNum,
                                Real64 const AirTempIn,      // Entering air dry bulb temperature(C)
                                Real64 const EnthAirInlet,   // Entering air enthalpy(J/kg)
                                Real64 const EnthAirOutlet,  // Leaving air enthalpy(J/kg)
                                Real64 const UACoilExternal, // Heat transfer coefficient for external surface (W/C)
                                Real64 &OutletAirTemp,       // Leaving air dry bulb temperature(C)
                                Real64 &OutletAirHumRat,     // Leaving air humidity ratio
                                Real64 &SenWaterCoilLoad     // Sensible heat transfer rate(W)
    );

    void UpdateWaterCoil(EnergyPlusData &state, int const CoilNum);

    void ReportWaterCoil(EnergyPlusData &state, int const CoilNum);

    void CalcDryFinEffCoef(EnergyPlusData &state, Real64 const OutTubeEffFinDiamRatio, Array1D<Real64> &PolynomCoef);

    void CalcIBesselFunc(Real64 const BessFuncArg, int const BessFuncOrd, Real64 &IBessFunc, int &ErrorCode);

    void CalcKBesselFunc(Real64 const BessFuncArg, int const BessFuncOrd, Real64 &KBessFunc, int &ErrorCode);

    void CalcPolynomCoef(EnergyPlusData &state, Array2<Real64> const &OrderedPair, Array1D<Real64> &PolynomCoef);

    // Iterate Routine for Cooling Coil

    void CoilAreaFracIter(Real64 &NewSurfAreaWetFrac,       // Out Value of variable
                          Real64 const SurfAreaFracCurrent, // Driver Value
                          Real64 const ErrorCurrent,        // Objective Function
                          Real64 &SurfAreaFracPrevious,     // First Previous value of Surf Area Fraction
                          Real64 &ErrorPrevious,            // First Previous value of error
                          Real64 &SurfAreaFracLast,         // Second Previous value of Surf Area Fraction
                          Real64 &ErrorLast,                // Second Previous value of error
                          int const IterNum,                // Number of Iterations
                          int &icvg                         // Iteration convergence flag
    );

    void CheckWaterCoilSchedule(EnergyPlusData &state, std::string_view CompName, Real64 &Value, int &CompIndex);

    Real64 GetCoilMaxWaterFlowRate(EnergyPlusData &state,
                                   std::string_view CoilType,   // must match coil types in this module
                                   std::string const &CoilName, // must match coil names for the coil type
                                   bool &ErrorsFound            // set to true if problem
    );

    int GetCoilInletNode(EnergyPlusData &state,
                         std::string_view CoilType,   // must match coil types in this module
                         std::string const &CoilName, // must match coil names for the coil type
                         bool &ErrorsFound            // set to true if problem
    );

    int GetCoilOutletNode(EnergyPlusData &state,
                          std::string_view CoilType,   // must match coil types in this module
                          std::string const &CoilName, // must match coil names for the coil type
                          bool &ErrorsFound            // set to true if problem
    );

    int GetCoilWaterInletNode(EnergyPlusData &state,
                              std::string_view CoilType,   // must match coil types in this module
                              std::string const &CoilName, // must match coil names for the coil type
                              bool &ErrorsFound            // set to true if problem
    );

    int GetCoilWaterOutletNode(EnergyPlusData &state,
                               std::string_view CoilType,   // must match coil types in this module
                               std::string const &CoilName, // must match coil names for the coil type
                               bool &ErrorsFound            // set to true if problem
    );

    void SetCoilDesFlow(EnergyPlusData &state,
                        std::string_view CoilType,   // must match coil types in this module
                        std::string const &CoilName, // must match coil names for the coil type
                        Real64 const CoilDesFlow,    // coil volumetric air flow rate [m3/s]
                        bool &ErrorsFound            // set to true if problem
    );

    Real64 GetWaterCoilDesAirFlow(EnergyPlusData &state,
                                  std::string const &CoilType, // must match coil types in this module
                                  std::string const &CoilName, // must match coil names for the coil type
                                  bool &ErrorsFound            // set to true if problem
    );

    void CheckActuatorNode(EnergyPlusData &state,
                           int const ActuatorNodeNum,                    // input actuator node number
                           DataPlant::PlantEquipmentType &WaterCoilType, // Cooling or Heating or 0
                           bool &NodeNotFound                            // true if matching water inlet node not found
    );

    void CheckForSensorAndSetPointNode(EnergyPlusData &state,
                                       int const SensorNodeNum,                    // controller sensor node number
                                       HVACControllers::CtrlVarType ControlledVar, // controlled variable type
                                       bool &NodeNotFound                          // true if matching air outlet node not found
    );

    Real64 TdbFnHRhPb(EnergyPlusData &state,
                      Real64 const H,  // specific enthalpy {J/kg}
                      Real64 const RH, // relative humidity value (0.0-1.0)
                      Real64 const PB  // barometric pressure {Pascals}
    );

    Real64 EstimateHEXSurfaceArea(EnergyPlusData &state, int const CoilNum); // coil number, [-]

    int GetWaterCoilIndex(EnergyPlusData &state,
                          std::string_view CoilType,   // must match coil types in this module
                          std::string const &CoilName, // must match coil names for the coil type
                          bool &ErrorsFound            // set to true if problem
    );
    int GetCompIndex(EnergyPlusData &state, CoilModel coilType, std::string_view const coilName);

    Real64 GetWaterCoilCapacity(EnergyPlusData &state,
                                std::string_view const CoilType, // must match coil types in this module
                                std::string const &CoilName,     // must match coil names for the coil type
                                bool &ErrorsFound                // set to true if problem
    );

    void UpdateWaterToAirCoilPlantConnection(EnergyPlusData &state,
                                             DataPlant::PlantEquipmentType const CoilType,
                                             std::string const &CoilName,
                                             int const EquipFlowCtrl,                    // Flow control mode for the equipment
                                             int const LoopNum,                          // Plant loop index for where called from
                                             const DataPlant::LoopSideLocation LoopSide, // Plant loop side index for where called from
                                             int &CompIndex,                             // Chiller number pointer
                                             bool const FirstHVACIteration,
                                             bool const &InitLoopEquip // If not zero, calculate the max load for operating conditions
    );

    Sched::Schedule *GetWaterCoilAvailSched(EnergyPlusData &state,
                                            std::string_view const coilType, // must match coil types in this module
                                            std::string const &CoilName,     // must match coil names for the coil type
                                            bool &ErrorsFound                // set to true if problem
    );

    // sets data to a coil that is used as a regeneration air heating coil in
    // desiccant dehumidification system
    void SetWaterCoilData(EnergyPlusData &state,
                          int const CoilNum,                                      // index of hot water heating Coil
                          bool &ErrorsFound,                                      // Set to true if certain errors found
                          ObjexxFCL::Optional_bool DesiccantRegenerationCoil = _, // Flag that this coil is used as regeneration air heating coil
                          ObjexxFCL::Optional_int DesiccantDehumIndex = _,        // Index for the desiccant dehum system where this caoil is used
                          ObjexxFCL::Optional_bool heatRecoveryCoil = _           // true if water coil is connected to heat recovery loop
    );

    // estimate heating coil design inlet water temperature for autosizing UA-value
    void EstimateCoilInletWaterTemp(EnergyPlusData &state,
                                    int const CoilNum,                // index to heating coil
                                    HVAC::FanOp const fanOp,          // fan operating mode
                                    Real64 const PartLoadRatio,       // part-load ratio of heating coil
                                    Real64 const UAMax,               // maximum UA-Value
                                    Real64 &DesCoilInletWaterTempUsed // estimated coil design inlet water temperature
    );

} // namespace WaterCoils

struct WaterCoilsData : BaseGlobalStruct
{

    // Parameters for Heat Exchanger Configuration
    int const CounterFlow;
    int const CrossFlow;
    int const SimpleAnalysis;
    int const DetailedAnalysis;

    // Water Systems
    int const CondensateDiscarded; // default mode where water is "lost"
    int const CondensateToTank;    // collect coil condensate from air and store in water storage tank

    // Parameters for COIL:Water:SimpleHeating Coil Performance Input Method
    int const UAandFlow; // for Coil Performance Input Method = UA and Design Water Flow Rate
    int const NomCap;    // for Coil Performance Input Method = Nominal Capacity

    // Parameters Subroutine CoolingCoil: design calc or simulation calc.
    int const DesignCalc; // ignore on/off check in CoolingCoil
    int const SimCalc;    // pay attention to on/off check in CoolingCoil

    // DERIVED TYPE DEFINITIONS

    // MODULE VARIABLE DECLARATIONS:
    int NumWaterCoils; // The Number of WaterCoils found in the Input
    Array1D_bool MySizeFlag;
    Array1D_bool MyUAAndFlowCalcFlag;
    Array1D_bool MyCoilDesignFlag;
    Array1D_bool CoilWarningOnceFlag;
    Array1D_int WaterTempCoolCoilErrs;        // error counting for detailed coils
    Array1D_int PartWetCoolCoilErrs;          // error counting for detailed coils
    bool GetWaterCoilsInputFlag;              // Flag set to make sure you get input once
    bool WaterCoilControllerCheckOneTimeFlag; // flg used to check water coil controller
    Array1D_bool CheckEquipName;

    bool InitWaterCoilOneTimeFlag;

    EPVector<WaterCoils::WaterCoilEquipConditions> WaterCoil;
    EPVector<WaterCoils::WaterCoilNumericFieldData> WaterCoilNumericFields;

    Real64 TOutNew = 0.0;            // reset outlet air temperature for Coil:Cooling:Water
    Real64 WOutNew = 0.0;            // reset outlet air humidity ratio for Coil:Cooling:Water
    Array1D<Real64> DesCpAir;        // CpAir at Design Inlet Air Temp
    Array1D<Real64> DesUARangeCheck; // Value for range check based on Design Inlet Air Humidity Ratio
    Array1D_bool MyEnvrnFlag;
    Array1D_bool MyCoilReportFlag;
    Array1D_bool PlantLoopScanFlag;
    Array1D<Real64> CoefSeries = Array1D<Real64>(5); // Tuned Changed to static: High call count: Set before use
    bool NoSatCurveIntersect = false;                // TRUE if failed to find apparatus dew-point
    bool BelowInletWaterTemp = false;                // TRUE if apparatus dew-point below design inlet water temperature
    bool CBFTooLarge = false;                        // TRUE if the coil bypass factor is unrealistically large
    bool NoExitCondReset = false;                    // TRUE if exit condition reset is not to be done
    Real64 RatedLatentCapacity = 0.0;                // latent cooling capacity at the rating point [W]
    Real64 RatedSHR = 0.0;                           // sensible heat ratio at the rating point
    Real64 CapacitanceWater = 0.0;                   // capacitance of the water stream [W/K]
    Real64 CMin = 0.0;                               // minimum capacitance of 2 streams [W/K]
    Real64 CoilEffectiveness = 0.0;                  // effectiveness of the coil (rated)
    Real64 SurfaceArea = 0.0;                        // heat exchanger surface area, [m2]
    Real64 UATotal = 0.0;                            // heat exchanger UA total, [W/C]
    Array1D_bool RptCoilHeaderFlag = Array1D_bool(2, true);
    Array2D<Real64> OrderedPair = Array2D<Real64>(WaterCoils::MaxOrderedPairs, 2);
    Array2D<Real64> OrdPairSum = Array2D<Real64>(10, 2);
    Array2D<Real64> OrdPairSumMatrix = Array2D<Real64>(10, 10);

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumWaterCoils = 0;
        this->InitWaterCoilOneTimeFlag = true;
        this->MySizeFlag.deallocate();
        this->MyUAAndFlowCalcFlag.deallocate();
        this->MyCoilDesignFlag.deallocate();
        this->CoilWarningOnceFlag.deallocate();
        this->WaterTempCoolCoilErrs.deallocate();
        this->PartWetCoolCoilErrs.deallocate();
        this->GetWaterCoilsInputFlag = true;
        this->CheckEquipName.deallocate();
        this->WaterCoil.deallocate();
        this->WaterCoilNumericFields.deallocate();
        this->WaterCoilControllerCheckOneTimeFlag = true;
        this->TOutNew = 0.0;
        this->WOutNew = 0.0;
        this->DesCpAir.deallocate();
        this->DesUARangeCheck.deallocate();
        this->MyEnvrnFlag.deallocate();
        this->MyCoilReportFlag.deallocate();
        this->PlantLoopScanFlag.deallocate();
        this->CoefSeries = Array1D<Real64>(5);
        this->NoSatCurveIntersect = false;
        this->BelowInletWaterTemp = false;
        this->CBFTooLarge = false;
        this->NoExitCondReset = false;
        this->RatedLatentCapacity = 0.0;
        this->RatedSHR = 0.0;
        this->CapacitanceWater = 0.0;
        this->CMin = 0.0;
        this->CoilEffectiveness = 0.0;
        this->SurfaceArea = 0.0;
        this->UATotal = 0.0;
        this->RptCoilHeaderFlag = Array1D_bool(2, true);
        this->OrderedPair = Array2D<Real64>(WaterCoils::MaxOrderedPairs, 2);
        this->OrdPairSum = Array2D<Real64>(10, 2);
        this->OrdPairSumMatrix = Array2D<Real64>(10, 10);
    }

    // Default Constructor
    WaterCoilsData()
        : CounterFlow(1), CrossFlow(2), SimpleAnalysis(1), DetailedAnalysis(2), CondensateDiscarded(1001), CondensateToTank(1002), UAandFlow(1),
          NomCap(2), DesignCalc(1), SimCalc(2), NumWaterCoils(0), GetWaterCoilsInputFlag(true), WaterCoilControllerCheckOneTimeFlag(true),
          InitWaterCoilOneTimeFlag(true)
    {
    }
};

} // namespace EnergyPlus

#endif
