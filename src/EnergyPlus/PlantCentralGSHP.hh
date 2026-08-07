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

#ifndef PlantCentralGSHP_hh_INCLUDED
#define PlantCentralGSHP_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PlantCentralGSHP {

    enum class CurrentMode
    {
        Invalid = -1,
        Off,
        CoolingOnly,
        HeatingOnly,
        HeatRecovery,
        CoolingDominant,
        HeatingDominant,
        Num
    };

    enum class CondenserModeTemperature
    {
        Invalid = -1,
        EnteringCondenser,
        LeavingCondenser,
        Num
    };

    struct CGSHPNodeData
    {
        // Members
        Real64 Temp = 0.0;                 // {C}
        Real64 TempMin = 0.0;              // {C}
        Real64 TempSetPoint = 0.0;         // SensedNodeFlagValue ! {C}
        Real64 MassFlowRate = 0.0;         // {kg/s}
        Real64 MassFlowRateMin = 0.0;      // {kg/s}
        Real64 MassFlowRateMax = 0.0;      // SensedNodeFlagValue ! {kg/s}
        Real64 MassFlowRateMinAvail = 0.0; // {kg/s}
        Real64 MassFlowRateMaxAvail = 0.0; // {kg/s}
        Real64 MassFlowRateSetPoint = 0.0; // {kg/s}
        Real64 MassFlowRateRequest = 0.0;  // {kg/s}
    };

    struct WrapperComponentSpecs
    {
        std::string WrapperPerformanceObjectType; // Component type
        std::string WrapperComponentName;         // Component name
        int WrapperPerformanceObjectIndex = 0;    // Component index in the input array
        int WrapperIdenticalObjectNum = 0;        // Number of identical objects
        Sched::Schedule *chSched = nullptr;       // schedule
    };

    struct CHReportVars
    {
        CurrentMode currentMode = CurrentMode::Invalid; // 0-off; 1-CoolingOnly; 2-HeatingOnly; 3-HeatRecovery; 4-CoolingDominant; 5-HeatingDominant
        Real64 ChillerPartLoadRatio = 0.0;              // Chiller PLR (Load/Capacity)
        Real64 ChillerCyclingRatio = 0.0;               // Chiller cycling ratio (time on/time step)
        Real64 ChillerFalseLoad = 0.0;                  // Chiller false load over and above water side load [J]
        Real64 ChillerFalseLoadRate = 0.0;              // Chiller false load rate over and above water side load [W]
        Real64 CoolingPower = 0.0;                      // Chiller power, W
        Real64 HeatingPower = 0.0;                      // Chiller power, W
        Real64 QEvap = 0.0;                             // Evaporator heat transfer rate [W]
        Real64 QCond = 0.0;                             // Condenser heat transfer rate [W]
        Real64 CoolingEnergy = 0.0;                     // Chiller electric consumption [J]
        Real64 HeatingEnergy = 0.0;                     // Chiller electric consumption [J]
        Real64 EvapEnergy = 0.0;                        // Evaporator heat transfer energy [J]
        Real64 CondEnergy = 0.0;                        // Condenser heat transfer energy [J]
        Real64 CondInletTemp = 0.0;                     // Condenser inlet temperature [C]
        Real64 EvapInletTemp = 0.0;                     // Evaporator inlet temperature [C]
        Real64 CondOutletTemp = 0.0;                    // Condenser outlet temperature [C]
        Real64 EvapOutletTemp = 0.0;                    // Evaporator outlet temperature [C]
        Real64 Evapmdot = 0.0;                          // Evaporator mass flow rate [kg/s]
        Real64 Condmdot = 0.0;                          // Condenser mass flow rate [kg/s]
        Real64 ActualCOP = 0.0;                         // Coefficient of performance
        Real64 ChillerCapFT = 0.0;                      // Chiller capacity curve output value
        Real64 ChillerEIRFT = 0.0;                      // Chiller EIRFT curve output value
        Real64 ChillerEIRFPLR = 0.0;                    // Chiller EIRFPLR curve output value
        Real64 CondenserFanPowerUse = 0.0;              // Air-cooled condenser fan power [W]
        Real64 CondenserFanEnergy = 0.0;                // Air-cooled condenser fan energy [J]
        Real64 ChillerPartLoadRatioSimul = 0.0;         // Chiller PLR (Load/Capacity) for simul clg/htg mode
        Real64 ChillerCyclingRatioSimul = 0.0;          // Chiller cycling ratio (time on/time step) for simul clg/htg mode
        Real64 ChillerFalseLoadSimul = 0.0;             // Chiller false load for simul clg/htg mode [J]
        Real64 ChillerFalseLoadRateSimul = 0.0;         // Chiller false load rate for simul clg/htg mode [W]
        Real64 CoolingPowerSimul = 0.0;                 // Chiller power for simul clg/htg mode [W]
        Real64 QEvapSimul = 0.0;                        // Evaporator heat transfer rate for simul clg/htg mode [W]
        Real64 QCondSimul = 0.0;                        // Evaporator heat transfer rate for simul clg/htg mode [W]
        Real64 CoolingEnergySimul = 0.0;                // Chiller electric consumption for simul clg/htg mode [J]
        Real64 EvapEnergySimul = 0.0;                   // Evaporator heat transfer energy for simul clg/htg mode [J]
        Real64 CondEnergySimul = 0.0;                   // Condenser heat transfer energy for simul clg/htg mode [J]
        Real64 EvapInletTempSimul = 0.0;                // Evaporator inlet temperature for simul clg/htg mode [C]
        Real64 EvapOutletTempSimul = 0.0;               // Evaporator outlet temperature for simul clg/htg mode [C]
        Real64 EvapmdotSimul = 0.0;                     // Evaporator mass flow rate for simul clg/htg mode [kg/s]
        Real64 CondInletTempSimul = 0.0;                // Condenser inlet temperature for simul clg/htg mode [C]
        Real64 CondOutletTempSimul = 0.0;               // Condenser outlet temperature for simul clg/htg mode [C]
        Real64 CondmdotSimul = 0.0;                     // Condenser mass flow rate for simul clg/htg mode [kg/s]
        Real64 ChillerCapFTSimul = 0.0;                 // Chiller capacity curve output value for simul clg/htg mode
        Real64 ChillerEIRFTSimul = 0.0;                 // Chiller EIRFT curve output value for simul clg/htg mode
        Real64 ChillerEIRFPLRSimul = 0.0;               // Chiller EIRFPLR curve output value for simul clg/htg mode
    };

    struct ChillerHeaterSpecs
    {
        std::string Name;                                                             // Name of the Chiller Heater object
        CondenserModeTemperature CondModeCooling = CondenserModeTemperature::Invalid; // Cooling mode temperature curve input variable
        CondenserModeTemperature CondModeHeating = CondenserModeTemperature::Invalid; // Clg/Htg mode temperature curve input variable
        CondenserModeTemperature CondMode = CondenserModeTemperature::Invalid;        // Current mode temperature curve input variable
        bool ConstantFlow = false;                                                    // True if this is a Constant Flow Chiller
        bool VariableFlow = false;                                                    // True if this is a Variable Flow Chiller
        bool CoolSetPointSetToLoop = false;                                           // True if the setpoint is missing at the outlet node
        bool HeatSetPointSetToLoop = false;                                           // True if the setpoint is missing at the outlet node
        bool CoolSetPointErrDone = false;                                             // true if setpoint warning issued
        bool HeatSetPointErrDone = false;                                             // true if setpoint warning issued
        bool PossibleSubcooling = false;                                              // flag to indicate chiller is doing less cooling that requested
        int ChillerHeaterNum = 1;                                                     // Chiller heater number
        int ChillerCapFTCoolingIDX = 0;                                               // Cooling capacity function of temperature curve index
        int ChillerEIRFTCoolingIDX = 0;           // Elec Input to Cooling Output ratio function of temperature curve index
        int ChillerEIRFPLRCoolingIDX = 0;         // Elec Input to cooling output ratio function of PLR curve index
        int ChillerCapFTHeatingIDX = 0;           // Clg/Htg capacity function of temperature curve index
        int ChillerEIRFTHeatingIDX = 0;           // Elec Input to Clg/Htg Output ratio function of temperature curve index
        int ChillerEIRFPLRHeatingIDX = 0;         // Elec Input to Clg/Htg output ratio function of PLR curve index
        int ChillerCapFTIDX = 0;                  // Capacity function of temperature curve index
        int ChillerEIRFTIDX = 0;                  // Elec Input to demand output ratio function of temperature curve index
        int ChillerEIRFPLRIDX = 0;                // Elec Input to demand output ratio function of PLR curve index
        int EvapInletNodeNum = 0;                 // Node number on the inlet side of the plant (evaporator side)
        int EvapOutletNodeNum = 0;                // Node number on the outlet side of the plant (evaporator side)
        int CondInletNodeNum = 0;                 // Node number on the inlet side of the condenser
        int CondOutletNodeNum = 0;                // Node number on the outlet side of the condenser
        int ChillerCapFTError = 0;                // Used for negative capacity as a function of temp warnings
        int ChillerCapFTErrorIndex = 0;           // Used for negative capacity as a function of temp warnings
        int ChillerEIRFTError = 0;                // Used for negative EIR as a function of temp warnings
        int ChillerEIRFTErrorIndex = 0;           // Used for negative EIR as a function of temp warnings
        int ChillerEIRFPLRError = 0;              // Used for negative EIR as a function of PLR warnings
        int ChillerEIRFPLRErrorIndex = 0;         // Used for negative EIR as a function of PLR warnings
        int ChillerEIRRefTempErrorIndex = 0;      // Used for reference temperature problems
        int DeltaTErrCount = 0;                   // Evaporator delta T equals 0 for variable flow chiller warning messages
        int DeltaTErrCountIndex = 0;              // Index to evaporator delta T = 0 for variable flow chiller warning messages
        int CondMassFlowIndex = 0;                // Index to condenser mass flow rate
        Real64 RefCapCooling = 0.0;               // Reference cooling-mode evaporator capacity [W]
        bool RefCapCoolingWasAutoSized = false;   // true if reference cooling capacity was autosize on input
        Real64 RefCOPCooling = 0.0;               // Reference cooling-mode COP
        Real64 TempRefEvapOutCooling = 0.0;       // Reference cooling-mode evaporator leaving temperature [C]
        Real64 TempRefCondInCooling = 0.0;        // Reference cooling-mode condenser entering temperature [C]
        Real64 TempRefCondOutCooling = 0.0;       // Reference cooling-mode condenser leaving temperature [C]
        Real64 MaxPartLoadRatCooling = 0.0;       // Maximum Part load ratio in cooling mode
        Real64 OptPartLoadRatCooling = 0.0;       // Optimum Part load ratio in cooling mode
        Real64 MinPartLoadRatCooling = 0.0;       // minimum Part load ratio in cooling mode
        Real64 ClgHtgToCoolingCapRatio = 0.0;     // ratio of clg/htg-mode evaporator capacity to cooling-mode evap. cap
        Real64 ClgHtgtoCogPowerRatio = 0.0;       // ratio of clg/htg-mode evaporator power to cooling-mode evap. power
        Real64 RefCapClgHtg = 0.0;                // Reference clg/htg-mode evaporator capacity [W]
        Real64 RefCOPClgHtg = 0.0;                // Reference clg/htg-mode COP
        Real64 RefPowerClgHtg = 0.0;              // Reference clg/htg-mode evaporator power [W]
        Real64 TempRefEvapOutClgHtg = 0.0;        // Reference clg/htg-mode evaporator leaving temperature [C]
        Real64 TempRefCondInClgHtg = 0.0;         // Reference clg/htg-mode condenser entering temperature [C]
        Real64 TempRefCondOutClgHtg = 0.0;        // Reference clg/htg-mode condenser leaving temperature [C]
        Real64 TempLowLimitEvapOut = 0.0;         // Low temperature shut off [C]
        Real64 MaxPartLoadRatClgHtg = 0.0;        // Maximum Part load ratio in simultaneous heating/cooling mode
        Real64 OptPartLoadRatClgHtg = 0.0;        // Optimum Part load ratio in simultaneous heating/cooling mode
        Real64 MinPartLoadRatClgHtg = 0.0;        // minimum Part load ratio in simultaneous heating/cooling mode
        CGSHPNodeData EvapInletNode = {};         // Chiller heater evaperator inlet node
        CGSHPNodeData EvapOutletNode = {};        // Chiller heater evaperator outlet node
        CGSHPNodeData CondInletNode = {};         // Chiller heater condenser inlet node
        CGSHPNodeData CondOutletNode = {};        // Chiller heater condenser outlet node
        Real64 EvapVolFlowRate = 0.0;             // Reference water volumetric flow rate through the evaporator [m3/s]
        bool EvapVolFlowRateWasAutoSized = false; // true if evaporator flow rate was autosize on input
        Real64 tmpEvapVolFlowRate = 0.0;          // temporary ref water vol flow rate for intermediate sizing [m3/s]
        Real64 CondVolFlowRate = 0.0;             // Reference water volumetric flow rate through the condenser [m3/s]
        bool CondVolFlowRateWasAutoSized = false; // true if condenser flow rate was autosize on input
        Real64 tmpCondVolFlowRate = 0.0;          // temporary ref water vol flow rate for intermediate sizing [m3/s]
        Real64 CondMassFlowRateMax = 0.0;         // Reference water mass flow rate through condenser [kg/s]
        Real64 EvapMassFlowRateMax = 0.0;         // Reference water mass flow rate through evaporator [kg/s]
        Real64 Evapmdot = 0.0;                    // Evaporator mass flow rate [kg/s]
        Real64 Condmdot = 0.0;                    // Condenser mass flow rate [kg/s]
        Real64 DesignHotWaterVolFlowRate = 0.0;   // Design hot water volumetric flow rate through the condenser [m3/s]
        Real64 OpenMotorEff = 0.0;                // Open chiller motor efficiency [fraction, 0 to 1]
        Real64 SizFac = 0.0;                      // sizing factor
        Real64 RefCap = 0.0;                      // Reference evaporator capacity [W]
        Real64 RefCOP = 0.0;                      // Reference COP
        Real64 TempRefEvapOut = 0.0;              // Reference evaporator leaving temperature [C]
        Real64 TempRefCondIn = 0.0;               // Reference condenser entering temperature [C]
        Real64 TempRefCondOut = 0.0;              // Reference condenser leaving temperature [C]
        Real64 OptPartLoadRat = 0.0;              // Optimal operating fraction of full load
        Real64 ChillerEIRFPLRMin = 0.0;           // Minimum value of PLR from EIRFPLR curve
        Real64 ChillerEIRFPLRMax = 0.0;           // Maximum value of PLR from EIRFPLR curve
        CHReportVars Report;
    };

    struct WrapperReportVars
    {
        Real64 Power = 0.0;                  // Wrapper power, W
        Real64 QCHW = 0.0;                   // Chilled water heat transfer rate [W]
        Real64 QHW = 0.0;                    // Hot Water heat transfer rate [W]
        Real64 QGLHE = 0.0;                  // Geo-field heat transfer rate [W]
        Real64 TotElecCooling = 0.0;         // Wrapper cooling electric consumption [J]
        Real64 TotElecHeating = 0.0;         // Wrapper heating electric consumption [J]
        Real64 CoolingEnergy = 0.0;          // Chilled water heat transfer energy [J]
        Real64 HeatingEnergy = 0.0;          // Hot Water heat transfer energy [J]
        Real64 GLHEEnergy = 0.0;             // Geo-field heat transfer energy [J]
        Real64 TotElecCoolingPwr = 0.0;      // Wrapper cooling electric consumption rate [W]
        Real64 TotElecHeatingPwr = 0.0;      // Wrapper heating electric consumption rate [W]
        Real64 CoolingRate = 0.0;            // Chilled water heat transfer rate [W]
        Real64 HeatingRate = 0.0;            // Hot Water heat transfer rate [W]
        Real64 GLHERate = 0.0;               // Geo-field heat transfer rate [W]
        Real64 CHWInletTemp = 0.0;           // Chilled water inlet temperature [C]
        Real64 HWInletTemp = 0.0;            // Hot water inlet temperature [C]
        Real64 GLHEInletTemp = 0.0;          // Geo-field inlet temperature [C]
        Real64 CHWOutletTemp = 0.0;          // Chilled water Outlet temperature [C]
        Real64 HWOutletTemp = 0.0;           // Hot water Outlet temperature [C]
        Real64 GLHEOutletTemp = 0.0;         // Geo-field Outlet temperature [C]
        Real64 CHWmdot = 0.0;                // Chilled water mass flow rate [kg/s]
        Real64 HWmdot = 0.0;                 // Hot water mass flow rate [kg/s]
        Real64 GLHEmdot = 0.0;               // Geo-field mass flow rate [kg/s]
        Real64 TotElecCoolingSimul = 0.0;    // Wrapper cooling electric consumption [J]
        Real64 CoolingEnergySimul = 0.0;     // Chilled water heat transfer energy [J]
        Real64 TotElecCoolingPwrSimul = 0.0; // Wrapper cooling electric consumption rate [W]
        Real64 CoolingRateSimul = 0.0;       // Chilled water heat transfer rate [W]
        Real64 CHWInletTempSimul = 0.0;      // Chilled water inlet temperature [C]
        Real64 GLHEInletTempSimul = 0.0;     // Geo-field inlet temperature [C]
        Real64 CHWOutletTempSimul = 0.0;     // Chilled water Outlet temperature [C]
        Real64 GLHEOutletTempSimul = 0.0;    // Geo-field Outlet temperature [C]
        Real64 CHWmdotSimul = 0.0;           // Chilled water mass flow rate [kg/s]
        Real64 GLHEmdotSimul = 0.0;          // Geo-field mass flow rate [kg/s]
    };

    struct WrapperSpecs : PlantComponent
    {
        std::string Name;                               // User identifier
        bool VariableFlowCH = false;                    // True if all chiller heaters are variable flow control
        Sched::Schedule *ancillaryPowerSched = nullptr; // Schedule value for ancillary power control
        Sched::Schedule *chSched = nullptr;             // Schedule value for individual chiller heater control
        int CHWInletNodeNum = 0;                        // Node number on the inlet side of the plant (Chilled Water side)
        int CHWOutletNodeNum = 0;                       // Node number on the outlet side of the plant (Chilled Water side)
        int HWInletNodeNum = 0;                         // Node number on the inlet side of the plant (Hot Water side)
        int HWOutletNodeNum = 0;                        // Node number on the outlet side of the plant (Hot Water side)
        int GLHEInletNodeNum = 0;                       // Node number on the inlet side of the plant (GLHE Water side)
        int GLHEOutletNodeNum = 0;                      // Node number on the outlet side of the plant (GLHE Water side)
        int CoolSetPointTempNode = 0;                   // Node number of the cooling setpoint temperature node
        int HeatSetPointTempNode = 0;                   // Node number of the heating setpoint temperature node
        bool EvapOutletMinTempCheck = true;             // True if minimum evaporator outlet temperature warning is enabled
        int NumOfComp = 0;                              // Number of Components under the wrapper
        Real64 CHWMassFlowRate = 0.0;                   // Chilled water mass flow rate
        Real64 HWMassFlowRate = 0.0;                    // Hot water mass flow rate
        Real64 GLHEMassFlowRate = 0.0;                  // Condenser water mass flow rate
        Real64 CHWMassFlowRateMax = 0.0;                // Maximum chilled water mass flow rate
        Real64 HWMassFlowRateMax = 0.0;                 // Maximum hot water mass flow rate
        Real64 GLHEMassFlowRateMax = 0.0;               // Maximum condenser water mass flow rate
        Real64 WrapperCoolingLoad = 0.0;                // Cooling demand for the central heat pump system
        Real64 WrapperHeatingLoad = 0.0;                // Heating demand for the central heat pump system
        Real64 AncillaryPower = 0.0;                    // Wrapper Ancillary Power
        Array1D<WrapperComponentSpecs> WrapperComp;
        Array1D<ChillerHeaterSpecs> ChillerHeater; // Dimension to number of machines
        bool CoolSetPointErrDone = false;          // true if setpoint warning issued
        bool HeatSetPointErrDone = false;          // true if setpoint warning issued
        int ChillerHeaterNums = 0;                 // Total number of chiller heater units
        PlantLocation CWPlantLoc = {};             // Chilled water plant loop component index
        PlantLocation HWPlantLoc = {};             // Hot water plant loop component index
        PlantLocation GLHEPlantLoc = {};           // Geo-field water plant loop component index
        int CHWMassFlowIndex = 0;                  // Chilled water flow index
        int HWMassFlowIndex = 0;                   // Hot water flow index
        int GLHEMassFlowIndex = 0;                 // Condenser side flow index
        Real64 SizingFactor = 1.0;                 // Sizing factor to adjust the capacity
        Real64 CHWVolFlowRate = 0.0;               // Chilled water volume flow rate [kg/s]
        Real64 HWVolFlowRate = 0.0;                // Hot water volume flow rate [kg/s]
        Real64 GLHEVolFlowRate = 0.0;              // Geo-field volume flow rate [kg/s]
        bool MyWrapperFlag = true;
        bool MyWrapperEnvrnFlag = true;
        bool SimulClgDominant = false;
        bool SimulHtgDominant = false;
        WrapperReportVars Report;
        bool setupOutputVarsFlag = true;
        bool mySizesReported = false;

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void getSizingFactor(Real64 &SizFac) override;

        void getDesignCapacities(
            EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad) override;

        void setupOutputVars(EnergyPlusData &state);

        void initialize(EnergyPlusData &state,
                        Real64 MyLoad, // Demand Load
                        int LoopNum    // Loop Number Index
        );

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void SizeWrapper(EnergyPlusData &state);

        void CalcWrapperModel(EnergyPlusData &state, Real64 &MyLoad, int LoopNum);

        void CalcChillerModel(EnergyPlusData &state);

        void CalcChillerHeaterModel(EnergyPlusData &state);

        void adjustChillerHeaterCondFlowTemp(EnergyPlusData &state,
                                             Real64 &QCondenser,
                                             Real64 &CondMassFlowRate,
                                             Real64 &CondOutletTemp,
                                             Real64 const CondInletTemp,
                                             Real64 const CondDeltaTemp);

        void adjustChillerHeaterEvapFlowTemp(
            EnergyPlusData &state, Real64 const qEvaporator, Real64 &evapMassFlowRate, Real64 &evapOutletTemp, Real64 const evapInletTemp);

        Real64
        setChillerHeaterCondTemp(EnergyPlusData &state, int const numChillerHeater, Real64 const condEnteringTemp, Real64 const condLeavingTemp);

        Real64 calcChillerCapFT(EnergyPlusData &state, int const numChillerHeater, Real64 const evapOutletTemp, Real64 const condTemp);

        void checkEvapOutletTemp(EnergyPlusData &state,
                                 int const numChillerHeater,
                                 Real64 &evapOutletTemp,
                                 Real64 const lowTempLimitEout,
                                 Real64 evapInletTemp,
                                 Real64 &qEvaporator,
                                 Real64 const evapMassFlowRate,
                                 Real64 const Cp,
                                 CurrentMode const mode);

        void calcPLRAndCyclingRatio(EnergyPlusData &state,
                                    Real64 const availChillerCap,
                                    Real64 &actualPartLoadRatio,
                                    Real64 const minPartLoadRatio,
                                    Real64 const maxPartLoadRatio,
                                    Real64 const qEvaporator,
                                    Real64 &frac);

        void UpdateChillerHeaterRecords(EnergyPlusData &state);

        void UpdateChillerRecords(EnergyPlusData &state);

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation) override;

        void oneTimeInit_new(EnergyPlusData &state) override;

        void oneTimeInit(EnergyPlusData &state) override;
    };

    void GetWrapperInput(EnergyPlusData &state);

    void GetChillerHeaterInput(EnergyPlusData &state);

} // namespace PlantCentralGSHP

struct PlantCentralGSHPData : BaseGlobalStruct
{

    bool getWrapperInputFlag = true;   // When TRUE, calls subroutine to read input file.
    int numWrappers = 0;               // Number of Wrappers specified in input
    int numChillerHeaters = 0;         // Number of Chiller/heaters specified in input
    Real64 ChillerCapFT = 0.0;         // Chiller/heater capacity fraction (evaluated as a function of temperature)
    Real64 ChillerEIRFT = 0.0;         // Chiller/heater electric input ratio (EIR = 1 / COP) as a function of temperature
    Real64 ChillerEIRFPLR = 0.0;       // Chiller/heater EIR as a function of part-load ratio (PLR)
    Real64 ChillerPartLoadRatio = 0.0; // Chiller/heater part-load ratio (PLR)
    Real64 ChillerCyclingRatio = 0.0;  // Chiller/heater cycling ratio
    Real64 ChillerFalseLoadRate = 0.0; // Chiller/heater false load over and above the water-side load [W]
    EPVector<PlantCentralGSHP::WrapperSpecs> Wrapper;
    EPVector<PlantCentralGSHP::ChillerHeaterSpecs> ChillerHeater;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) PlantCentralGSHPData();
    }
};

} // namespace EnergyPlus

#endif
