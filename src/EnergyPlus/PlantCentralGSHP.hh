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

    enum class CondenserType
    {
        Invalid = -1,
        WaterCooled,
        SmartMixing,
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
        Real64 Temp;                 // {C}
        Real64 TempMin;              // {C}
        Real64 TempSetPoint;         // SensedNodeFlagValue ! {C}
        Real64 MassFlowRate;         // {kg/s}
        Real64 MassFlowRateMin;      // {kg/s}
        Real64 MassFlowRateMax;      // SensedNodeFlagValue ! {kg/s}
        Real64 MassFlowRateMinAvail; // {kg/s}
        Real64 MassFlowRateMaxAvail; // {kg/s}
        Real64 MassFlowRateSetPoint; // {kg/s}
        Real64 MassFlowRateRequest;  // {kg/s}

        CGSHPNodeData()
            : Temp(0.0), TempMin(0.0), TempSetPoint(0.0), MassFlowRate(0.0), MassFlowRateMin(0.0), MassFlowRateMax(0.0), MassFlowRateMinAvail(0.0),
              MassFlowRateMaxAvail(0.0), MassFlowRateSetPoint(0.0), MassFlowRateRequest(0.0)
        {
        }
    };

    struct WrapperComponentSpecs
    {
        std::string WrapperPerformanceObjectType; // Component type
        std::string WrapperComponentName;         // Component name
        int WrapperPerformanceObjectIndex;        // Component index in the input array
        int WrapperIdenticalObjectNum;            // Number of identical objects
        Sched::Schedule *chSched = nullptr;       // schedule

        WrapperComponentSpecs() : WrapperPerformanceObjectIndex(0), WrapperIdenticalObjectNum(0)
        {
        }
    };

    struct CHReportVars
    {
        int CurrentMode;                  // 0-off; 1-cooling only; 2-heating-only; 3-simutaneouls heat/cool
        Real64 ChillerPartLoadRatio;      // Chiller PLR (Load/Capacity)
        Real64 ChillerCyclingRatio;       // Chiller cycling ratio (time on/time step)
        Real64 ChillerFalseLoad;          // Chiller false load over and above water side load [J]
        Real64 ChillerFalseLoadRate;      // Chiller false load rate over and above water side load [W]
        Real64 CoolingPower;              // Chiller power, W
        Real64 HeatingPower;              // Chiller power, W
        Real64 QEvap;                     // Evaporator heat transfer rate [W]
        Real64 QCond;                     // Condenser heat transfer rate [W]
        Real64 CoolingEnergy;             // Chiller electric consumption [J]
        Real64 HeatingEnergy;             // Chiller electric consumption [J]
        Real64 EvapEnergy;                // Evaporator heat transfer energy [J]
        Real64 CondEnergy;                // Condenser heat transfer energy [J]
        Real64 CondInletTemp;             // Condenser inlet temperature [C]
        Real64 EvapInletTemp;             // Evaporator inlet temperature [C]
        Real64 CondOutletTemp;            // Condenser outlet temperature [C]
        Real64 EvapOutletTemp;            // Evaporator outlet temperature [C]
        Real64 Evapmdot;                  // Evaporator mass flow rate [kg/s]
        Real64 Condmdot;                  // Condenser mass flow rate [kg/s]
        Real64 ActualCOP;                 // Coefficient of performance
        Real64 ChillerCapFT;              // Chiller capacity curve output value
        Real64 ChillerEIRFT;              // Chiller EIRFT curve output value
        Real64 ChillerEIRFPLR;            // Chiller EIRFPLR curve output value
        Real64 CondenserFanPowerUse;      // Air-cooled condenser fan power [W]
        Real64 CondenserFanEnergy;        // Air-cooled condenser fan energy [J]
        Real64 ChillerPartLoadRatioSimul; // Chiller PLR (Load/Capacity) for simul clg/htg mode
        Real64 ChillerCyclingRatioSimul;  // Chiller cycling ratio (time on/time step) for simul clg/htg mode
        Real64 ChillerFalseLoadSimul;     // Chiller false load for simul clg/htg mode [J]
        Real64 ChillerFalseLoadRateSimul; // Chiller false load rate for simul clg/htg mode [W]
        Real64 CoolingPowerSimul;         // Chiller power for simul clg/htg mode [W]
        Real64 QEvapSimul;                // Evaporator heat transfer rate for simul clg/htg mode [W]
        Real64 QCondSimul;                // Evaporator heat transfer rate for simul clg/htg mode [W]
        Real64 CoolingEnergySimul;        // Chiller electric consumption for simul clg/htg mode [J]
        Real64 EvapEnergySimul;           // Evaporator heat transfer energy for simul clg/htg mode [J]
        Real64 CondEnergySimul;           // Condenser heat transfer energy for simul clg/htg mode [J]
        Real64 EvapInletTempSimul;        // Evaporator inlet temperature for simul clg/htg mode [C]
        Real64 EvapOutletTempSimul;       // Evaporator outlet temperature for simul clg/htg mode [C]
        Real64 EvapmdotSimul;             // Evaporator mass flow rate for simul clg/htg mode [kg/s]
        Real64 CondInletTempSimul;        // Condenser inlet temperature for simul clg/htg mode [C]
        Real64 CondOutletTempSimul;       // Condenser outlet temperature for simul clg/htg mode [C]
        Real64 CondmdotSimul;             // Condenser mass flow rate for simul clg/htg mode [kg/s]
        Real64 ChillerCapFTSimul;         // Chiller capacity curve output value for simul clg/htg mode
        Real64 ChillerEIRFTSimul;         // Chiller EIRFT curve output value for simul clg/htg mode
        Real64 ChillerEIRFPLRSimul;       // Chiller EIRFPLR curve output value for simul clg/htg mode

        CHReportVars()
            : CurrentMode(0), ChillerPartLoadRatio(0.0), ChillerCyclingRatio(0.0), ChillerFalseLoad(0.0), ChillerFalseLoadRate(0.0),
              CoolingPower(0.0), HeatingPower(0.0), QEvap(0.0), QCond(0.0), CoolingEnergy(0.0), HeatingEnergy(0.0), EvapEnergy(0.0), CondEnergy(0.0),
              CondInletTemp(0.0), EvapInletTemp(0.0), CondOutletTemp(0.0), EvapOutletTemp(0.0), Evapmdot(0.0), Condmdot(0.0), ActualCOP(0.0),
              ChillerCapFT(0.0), ChillerEIRFT(0.0), ChillerEIRFPLR(0.0), CondenserFanPowerUse(0.0), CondenserFanEnergy(0.0),
              ChillerPartLoadRatioSimul(0.0), ChillerCyclingRatioSimul(0.0), ChillerFalseLoadSimul(0.0), ChillerFalseLoadRateSimul(0.0),
              CoolingPowerSimul(0.0), QEvapSimul(0.0), QCondSimul(0.0), CoolingEnergySimul(0.0), EvapEnergySimul(0.0), CondEnergySimul(0.0),
              EvapInletTempSimul(0.0), EvapOutletTempSimul(0.0), EvapmdotSimul(0.0), CondInletTempSimul(0.0), CondOutletTempSimul(0.0),
              CondmdotSimul(0.0), ChillerCapFTSimul(0.0), ChillerEIRFTSimul(0.0), ChillerEIRFPLRSimul(0.0)
        {
        }
    };

    struct ChillerHeaterSpecs
    {
        std::string Name;                         // Name of the Chiller Heater object
        CondenserModeTemperature CondModeCooling; // Cooling mode temperature curve input variable
        CondenserModeTemperature CondModeHeating; // Clg/Htg mode temperature curve input variable
        CondenserModeTemperature CondMode;        // Current mode temperature curve input variable
        bool ConstantFlow;                        // True if this is a Constant Flow Chiller
        bool VariableFlow;                        // True if this is a Variable Flow Chiller
        bool CoolSetPointSetToLoop;               // True if the setpoint is missing at the outlet node
        bool HeatSetPointSetToLoop;               // True if the setpoint is missing at the outlet node
        bool CoolSetPointErrDone;                 // true if setpoint warning issued
        bool HeatSetPointErrDone;                 // true if setpoint warning issued
        bool PossibleSubcooling;                  // flag to indicate chiller is doing less cooling that requested
        int ChillerHeaterNum;                     // Chiller heater number
        CondenserType condenserType;              // Type of Condenser - only water cooled is allowed
        int ChillerCapFTCoolingIDX;               // Cooling capacity function of temperature curve index
        int ChillerEIRFTCoolingIDX;               // Elec Input to Cooling Output ratio function of temperature curve index
        int ChillerEIRFPLRCoolingIDX;             // Elec Input to cooling output ratio function of PLR curve index
        int ChillerCapFTHeatingIDX;               // Clg/Htg capacity function of temperature curve index
        int ChillerEIRFTHeatingIDX;               // Elec Input to Clg/Htg Output ratio function of temperature curve index
        int ChillerEIRFPLRHeatingIDX;             // Elec Input to Clg/Htg output ratio function of PLR curve index
        int ChillerCapFTIDX;                      // Capacity function of temperature curve index
        int ChillerEIRFTIDX;                      // Elec Input to demand output ratio function of temperature curve index
        int ChillerEIRFPLRIDX;                    // Elec Input to demand output ratio function of PLR curve index
        int EvapInletNodeNum;                     // Node number on the inlet side of the plant (evaporator side)
        int EvapOutletNodeNum;                    // Node number on the outlet side of the plant (evaporator side)
        int CondInletNodeNum;                     // Node number on the inlet side of the condenser
        int CondOutletNodeNum;                    // Node number on the outlet side of the condenser
        int ChillerCapFTError;                    // Used for negative capacity as a function of temp warnings
        int ChillerCapFTErrorIndex;               // Used for negative capacity as a function of temp warnings
        int ChillerEIRFTError;                    // Used for negative EIR as a function of temp warnings
        int ChillerEIRFTErrorIndex;               // Used for negative EIR as a function of temp warnings
        int ChillerEIRFPLRError;                  // Used for negative EIR as a function of PLR warnings
        int ChillerEIRFPLRErrorIndex;             // Used for negative EIR as a function of PLR warnings
        int ChillerEIRRefTempErrorIndex;          // Used for reference temperature problems
        int DeltaTErrCount;                       // Evaporator delta T equals 0 for variable flow chiller warning messages
        int DeltaTErrCountIndex;                  // Index to evaporator delta T = 0 for variable flow chiller warning messages
        int CondMassFlowIndex;                    // Index to condenser mass flow rate
        Real64 RefCapCooling;                     // Reference cooling-mode evaporator capacity [W]
        bool RefCapCoolingWasAutoSized;           // true if reference cooling capacity was autosize on input
        Real64 RefCOPCooling;                     // Reference cooling-mode COP
        Real64 TempRefEvapOutCooling;             // Reference cooling-mode evaporator leaving temperature [C]
        Real64 TempRefCondInCooling;              // Reference cooling-mode condenser entering temperature [C]
        Real64 TempRefCondOutCooling;             // Reference cooling-mode condenser leaving temperature [C]
        Real64 MaxPartLoadRatCooling;             // Maximum Part load ratio in cooling mode
        Real64 OptPartLoadRatCooling;             // Optimum Part load ratio in cooling mode
        Real64 MinPartLoadRatCooling;             // minimum Part load ratio in cooling mode
        Real64 ClgHtgToCoolingCapRatio;           // ratio of clg/htg-mode evaporator capacity to cooling-mode evap. cap
        Real64 ClgHtgtoCogPowerRatio;             // ratio of clg/htg-mode evaporator power to cooling-mode evap. power
        Real64 RefCapClgHtg;                      // Reference clg/htg-mode evaporator capacity [W]
        Real64 RefCOPClgHtg;                      // Reference clg/htg-mode COP
        Real64 RefPowerClgHtg;                    // Reference clg/htg-mode evaporator power [W]
        Real64 TempRefEvapOutClgHtg;              // Reference clg/htg-mode evaporator leaving temperature [C]
        Real64 TempRefCondInClgHtg;               // Reference clg/htg-mode condenser entering temperature [C]
        Real64 TempRefCondOutClgHtg;              // Reference clg/htg-mode condenser leaving temperature [C]
        Real64 TempLowLimitEvapOut;               // Low temperature shut off [C]
        Real64 MaxPartLoadRatClgHtg;              // Maximum Part load ratio in simultaneous heating/cooling mode
        Real64 OptPartLoadRatClgHtg;              // Optimum Part load ratio in simultaneous heating/cooling mode
        Real64 MinPartLoadRatClgHtg;              // minimum Part load ratio in simultaneous heating/cooling mode
        CGSHPNodeData EvapInletNode;              // Chiller heater evaperator inlet node
        CGSHPNodeData EvapOutletNode;             // Chiller heater evaperator outlet node
        CGSHPNodeData CondInletNode;              // Chiller heater condenser inlet node
        CGSHPNodeData CondOutletNode;             // Chiller heater condenser outlet node
        Real64 EvapVolFlowRate;                   // Reference water volumetric flow rate through the evaporator [m3/s]
        bool EvapVolFlowRateWasAutoSized;         // true if evaporator flow rate was autosize on input
        Real64 tmpEvapVolFlowRate;                // temporary ref water vol flow rate for intermediate sizing [m3/s]
        Real64 CondVolFlowRate;                   // Reference water volumetric flow rate through the condenser [m3/s]
        bool CondVolFlowRateWasAutoSized;         // true if condenser flow rate was autosize on input
        Real64 tmpCondVolFlowRate;                // temporary ref water vol flow rate for intermediate sizing [m3/s]
        Real64 CondMassFlowRateMax;               // Reference water mass flow rate through condenser [kg/s]
        Real64 EvapMassFlowRateMax;               // Reference water mass flow rate through evaporator [kg/s]
        Real64 Evapmdot;                          // Evaporator mass flow rate [kg/s]
        Real64 Condmdot;                          // Condenser mass flow rate [kg/s]
        Real64 DesignHotWaterVolFlowRate;         // Design hot water volumetric flow rate through the condenser [m3/s]
        Real64 OpenMotorEff;                      // Open chiller motor efficiency [fraction, 0 to 1]
        Real64 SizFac;                            // sizing factor
        Real64 RefCap;                            // Reference evaporator capacity [W]
        Real64 RefCOP;                            // Reference COP
        Real64 TempRefEvapOut;                    // Reference evaporator leaving temperature [C]
        Real64 TempRefCondIn;                     // Reference condenser entering temperature [C]
        Real64 TempRefCondOut;                    // Reference condenser leaving temperature [C]
        Real64 OptPartLoadRat;                    // Optimal operating fraction of full load
        Real64 ChillerEIRFPLRMin;                 // Minimum value of PLR from EIRFPLR curve
        Real64 ChillerEIRFPLRMax;                 // Maximum value of PLR from EIRFPLR curve
        CHReportVars Report;

        ChillerHeaterSpecs()
            : ConstantFlow(false), VariableFlow(false), CoolSetPointSetToLoop(false), HeatSetPointSetToLoop(false), CoolSetPointErrDone(false),
              HeatSetPointErrDone(false), PossibleSubcooling(false), ChillerHeaterNum(1), condenserType(CondenserType::Invalid),
              ChillerCapFTCoolingIDX(0), ChillerEIRFTCoolingIDX(0), ChillerEIRFPLRCoolingIDX(0), ChillerCapFTHeatingIDX(0), ChillerEIRFTHeatingIDX(0),
              ChillerEIRFPLRHeatingIDX(0), ChillerCapFTIDX(0), ChillerEIRFTIDX(0), ChillerEIRFPLRIDX(0), EvapInletNodeNum(0), EvapOutletNodeNum(0),
              CondInletNodeNum(0), CondOutletNodeNum(0), ChillerCapFTError(0), ChillerCapFTErrorIndex(0), ChillerEIRFTError(0),
              ChillerEIRFTErrorIndex(0), ChillerEIRFPLRError(0), ChillerEIRFPLRErrorIndex(0), ChillerEIRRefTempErrorIndex(0), DeltaTErrCount(0),
              DeltaTErrCountIndex(0), CondMassFlowIndex(0), RefCapCooling(0.0), RefCapCoolingWasAutoSized(false), RefCOPCooling(0.0),
              TempRefEvapOutCooling(0.0), TempRefCondInCooling(0.0), TempRefCondOutCooling(0.0), MaxPartLoadRatCooling(0.0),
              OptPartLoadRatCooling(0.0), MinPartLoadRatCooling(0.0), ClgHtgToCoolingCapRatio(0.0), ClgHtgtoCogPowerRatio(0.0), RefCapClgHtg(0.0),
              RefCOPClgHtg(0.0), RefPowerClgHtg(0.0), TempRefEvapOutClgHtg(0.0), TempRefCondInClgHtg(0.0), TempRefCondOutClgHtg(0.0),
              TempLowLimitEvapOut(0.0), MaxPartLoadRatClgHtg(0.0), OptPartLoadRatClgHtg(0.0), MinPartLoadRatClgHtg(0.0), EvapVolFlowRate(0.0),
              EvapVolFlowRateWasAutoSized(false), tmpEvapVolFlowRate(0.0), CondVolFlowRate(0.0), CondVolFlowRateWasAutoSized(false),
              tmpCondVolFlowRate(0.0), CondMassFlowRateMax(0.0), EvapMassFlowRateMax(0.0), Evapmdot(0.0), Condmdot(0.0),
              DesignHotWaterVolFlowRate(0.0), OpenMotorEff(0.0), SizFac(0.0), RefCap(0.0), RefCOP(0.0), TempRefEvapOut(0.0), TempRefCondIn(0.0),
              TempRefCondOut(0.0), OptPartLoadRat(0.0), ChillerEIRFPLRMin(0.0), ChillerEIRFPLRMax(0.0)
        {
        }
    };

    struct WrapperReportVars
    {
        Real64 Power;                  // Wrapper power, W
        Real64 QCHW;                   // Chilled water heat transfer rate [W]
        Real64 QHW;                    // Hot Water heat transfer rate [W]
        Real64 QGLHE;                  // Geo-field heat transfer rate [W]
        Real64 TotElecCooling;         // Wrapper cooling electric consumption [J]
        Real64 TotElecHeating;         // Wrapper heating electric consumption [J]
        Real64 CoolingEnergy;          // Chilled water heat transfer energy [J]
        Real64 HeatingEnergy;          // Hot Water heat transfer energy [J]
        Real64 GLHEEnergy;             // Geo-field heat transfer energy [J]
        Real64 TotElecCoolingPwr;      // Wrapper cooling electric consumption rate [W]
        Real64 TotElecHeatingPwr;      // Wrapper heating electric consumption rate [W]
        Real64 CoolingRate;            // Chilled water heat transfer rate [W]
        Real64 HeatingRate;            // Hot Water heat transfer rate [W]
        Real64 GLHERate;               // Geo-field heat transfer rate [W]
        Real64 CHWInletTemp;           // Chilled water inlet temperature [C]
        Real64 HWInletTemp;            // Hot water inlet temperature [C]
        Real64 GLHEInletTemp;          // Geo-field inlet temperature [C]
        Real64 CHWOutletTemp;          // Chilled water Outlet temperature [C]
        Real64 HWOutletTemp;           // Hot water Outlet temperature [C]
        Real64 GLHEOutletTemp;         // Geo-field Outlet temperature [C]
        Real64 CHWmdot;                // Chilled water mass flow rate [kg/s]
        Real64 HWmdot;                 // Hot water mass flow rate [kg/s]
        Real64 GLHEmdot;               // Geo-field mass flow rate [kg/s]
        Real64 TotElecCoolingSimul;    // Wrapper cooling electric consumption [J]
        Real64 CoolingEnergySimul;     // Chilled water heat transfer energy [J]
        Real64 TotElecCoolingPwrSimul; // Wrapper cooling electric consumption rate [W]
        Real64 CoolingRateSimul;       // Chilled water heat transfer rate [W]
        Real64 CHWInletTempSimul;      // Chilled water inlet temperature [C]
        Real64 GLHEInletTempSimul;     // Geo-field inlet temperature [C]
        Real64 CHWOutletTempSimul;     // Chilled water Outlet temperature [C]
        Real64 GLHEOutletTempSimul;    // Geo-field Outlet temperature [C]
        Real64 CHWmdotSimul;           // Chilled water mass flow rate [kg/s]
        Real64 GLHEmdotSimul;          // Geo-field mass flow rate [kg/s]

        WrapperReportVars()
            : Power(0.0), QCHW(0.0), QHW(0.0), QGLHE(0.0), TotElecCooling(0.0), TotElecHeating(0.0), CoolingEnergy(0.0), HeatingEnergy(0.0),
              GLHEEnergy(0.0), TotElecCoolingPwr(0.0), TotElecHeatingPwr(0.0), CoolingRate(0.0), HeatingRate(0.0), GLHERate(0.0), CHWInletTemp(0.0),
              HWInletTemp(0.0), GLHEInletTemp(0.0), CHWOutletTemp(0.0), HWOutletTemp(0.0), GLHEOutletTemp(0.0), CHWmdot(0.0), HWmdot(0.0),
              GLHEmdot(0.0), TotElecCoolingSimul(0.0), CoolingEnergySimul(0.0), TotElecCoolingPwrSimul(0.0), CoolingRateSimul(0.0),
              CHWInletTempSimul(0.0), GLHEInletTempSimul(0.0), CHWOutletTempSimul(0.0), GLHEOutletTempSimul(0.0), CHWmdotSimul(0.0),
              GLHEmdotSimul(0.0)
        {
        }
    };

    struct WrapperSpecs : PlantComponent
    {
        std::string Name;                               // User identifier
        bool VariableFlowCH;                            // True if all chiller heaters are variable flow control
        Sched::Schedule *ancillaryPowerSched = nullptr; // Schedule value for ancillary power control
        Sched::Schedule *chSched = nullptr;             // Schedule value for individual chiller heater control
        CondenserType ControlMode;                      // SmartMixing or FullyMixing
        int CHWInletNodeNum;                            // Node number on the inlet side of the plant (Chilled Water side)
        int CHWOutletNodeNum;                           // Node number on the outlet side of the plant (Chilled Water side)
        int HWInletNodeNum;                             // Node number on the inlet side of the plant (Hot Water side)
        int HWOutletNodeNum;                            // Node number on the outlet side of the plant (Hot Water side)
        int GLHEInletNodeNum;                           // Node number on the inlet side of the plant (GLHE Water side)
        int GLHEOutletNodeNum;                          // Node number on the outlet side of the plant (GLHE Water side)
        int NumOfComp;                                  // Number of Components under the wrapper
        Real64 CHWMassFlowRate;                         // Chilled water mass flow rate
        Real64 HWMassFlowRate;                          // Hot water mass flow rate
        Real64 GLHEMassFlowRate;                        // Condenser water mass flow rate
        Real64 CHWMassFlowRateMax;                      // Maximum chilled water mass flow rate
        Real64 HWMassFlowRateMax;                       // Maximum hot water mass flow rate
        Real64 GLHEMassFlowRateMax;                     // Maximum condenser water mass flow rate
        Real64 WrapperCoolingLoad;                      // Cooling demand for the central heat pump system
        Real64 WrapperHeatingLoad;                      // Heating demand for the central heat pump system
        Real64 AncillaryPower;                          // Wrapper Ancillary Power
        Array1D<WrapperComponentSpecs> WrapperComp;
        Array1D<ChillerHeaterSpecs> ChillerHeater; // Dimension to number of machines
        bool CoolSetPointErrDone;                  // true if setpoint warning issued
        bool HeatSetPointErrDone;                  // true if setpoint warning issued
        bool CoolSetPointSetToLoop;                // True if the setpoint is missing at the outlet node
        bool HeatSetPointSetToLoop;                // True if the setpoint is missing at the outlet node
        int ChillerHeaterNums;                     // Total number of chiller heater units
        PlantLocation CWPlantLoc;                  // Chilled water plant loop component index
        PlantLocation HWPlantLoc;                  // Hot water plant loop component index
        PlantLocation GLHEPlantLoc;                // Geo-field water plant loop component index
        int CHWMassFlowIndex;                      // Chilled water flow index
        int HWMassFlowIndex;                       // Hot water flow index
        int GLHEMassFlowIndex;                     // Condenser side flow index
        Real64 SizingFactor;                       // Sizing factor to adjust the capacity
        Real64 CHWVolFlowRate;                     // Chilled water volume flow rate [kg/s]
        Real64 HWVolFlowRate;                      // Hot water volume flow rate [kg/s]
        Real64 GLHEVolFlowRate;                    // Geo-field volume flow rate [kg/s]
        bool MyWrapperFlag;
        bool MyWrapperEnvrnFlag;
        bool SimulClgDominant;
        bool SimulHtgDominant;
        WrapperReportVars Report;
        bool setupOutputVarsFlag;
        bool mySizesReported;

        WrapperSpecs()
            : VariableFlowCH(false), ControlMode(CondenserType::Invalid), CHWInletNodeNum(0), CHWOutletNodeNum(0), HWInletNodeNum(0),
              HWOutletNodeNum(0), GLHEInletNodeNum(0), GLHEOutletNodeNum(0), NumOfComp(0), CHWMassFlowRate(0.0), HWMassFlowRate(0.0),
              GLHEMassFlowRate(0.0), CHWMassFlowRateMax(0.0), HWMassFlowRateMax(0.0), GLHEMassFlowRateMax(0.0), WrapperCoolingLoad(0.0),
              WrapperHeatingLoad(0.0), AncillaryPower(0.0), CoolSetPointErrDone(false), HeatSetPointErrDone(false), CoolSetPointSetToLoop(false),
              HeatSetPointSetToLoop(false), ChillerHeaterNums(0), CWPlantLoc{}, HWPlantLoc{}, GLHEPlantLoc{}, CHWMassFlowIndex(0), HWMassFlowIndex(0),
              GLHEMassFlowIndex(0), SizingFactor(1.0), CHWVolFlowRate(0.0), HWVolFlowRate(0.0), GLHEVolFlowRate(0.0), MyWrapperFlag(true),
              MyWrapperEnvrnFlag(true), SimulClgDominant(false), SimulHtgDominant(false), setupOutputVarsFlag(true), mySizesReported(false)
        {
        }

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
                                 Real64 const Cp);

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
        this->getWrapperInputFlag = true;
        this->numWrappers = 0;
        this->numChillerHeaters = 0;
        this->ChillerCapFT = 0.0;
        this->ChillerEIRFT = 0.0;
        this->ChillerEIRFPLR = 0.0;
        this->ChillerPartLoadRatio = 0.0;
        this->ChillerCyclingRatio = 0.0;
        this->ChillerFalseLoadRate = 0.0;
        this->Wrapper.deallocate();
        this->ChillerHeater.deallocate();
    }
};

} // namespace EnergyPlus

#endif
