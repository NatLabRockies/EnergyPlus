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

#ifndef ChillerReformulatedEIR_hh_INCLUDED
#define ChillerReformulatedEIR_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace ChillerReformulatedEIR {

    enum class PLR
    {
        Invalid = -1,
        LeavingCondenserWaterTemperature, // Type 1_LeavingCondenserWaterTemperature
        Lift,                             // Type 2_Lift
        Num
    };

    struct ReformulatedEIRChillerSpecs : PlantComponent
    {
        // Members
        std::string Name;        // User identifier
        int TypeNum = 0;         // plant loop type identifier
        std::string CAPFTName;   // CAPFT curve name
        std::string EIRFTName;   // EIRFT curve name
        std::string EIRFPLRName; // EIRPLR curve name
        DataPlant::CondenserType CondenserType =
            DataPlant::CondenserType::Invalid; // Type of Condenser. Water Cooled is the only available option for now
        PLR PartLoadCurveType = PLR::Invalid;  // Part Load Ratio Curve Type: 1_LeavingCondenserWaterTemperature; 2_Lift
        Real64 RefCap = 0.0;                   // Reference capacity of the chiller [W]
        bool RefCapWasAutoSized = false;       // reference capacity was autosized on input
        Real64 RefCOP = 0.0;                   // Reference coefficient of performance [W/W]
        DataPlant::FlowMode FlowMode = DataPlant::FlowMode::Invalid;
        DataPlant::CondenserFlowControl CondenserFlowControl = DataPlant::CondenserFlowControl::Invalid;
        bool ModulatedFlowSetToLoop = false;      // True if the setpoint is missing at the outlet node
        bool ModulatedFlowErrDone = false;        // true if setpoint warning issued
        Real64 EvapVolFlowRate = 0.0;             // Reference water volumetric flow rate through the evaporator [m3/s]
        bool EvapVolFlowRateWasAutoSized = false; // true if previous was autosize input
        Real64 EvapMassFlowRateMax = 0.0;         // Reference water mass flow rate through evaporator [kg/s]
        Real64 CondVolFlowRate = 0.0;             // Reference water volumetric flow rate through the condenser [m3/s]
        bool CondVolFlowRateWasAutoSized = false; // true if previous was set to autosize on input
        Real64 CondMassFlowRateMax = 0.0;         // Reference water mass flow rate through condenser [kg/s]
        Real64 CompPowerToCondenserFrac = 0.0;    // Fraction of compressor electric power rejected by condenser [0 to 1]
        int EvapInletNodeNum = 0;                 // Node number on the inlet side of the plant (evaporator side)
        int EvapOutletNodeNum = 0;                // Node number on the outlet side of the plant (evaporator side)
        int CondInletNodeNum = 0;                 // Node number on the inlet side of the condenser
        int CondOutletNodeNum = 0;                // Node number on the outlet side of the condenser
        Real64 MinPartLoadRat = 0.0;              // Minimum allowed operating fraction of full load
        Real64 MaxPartLoadRat = 0.0;              // Maximum allowed operating fraction of full load
        Real64 OptPartLoadRat = 0.0;              // Optimal operating fraction of full load
        Real64 MinUnloadRat = 0.0;                // Minimum unloading ratio
        Real64 TempRefCondIn = 0.0;               // The reference secondary loop fluid temperature at the
        // chiller condenser side inlet for the reformulated chiller [C]
        Real64 TempRefCondOut = 0.0; // The reference secondary loop fluid temperature at the
        // chiller condenser side outlet for the reformulated chiller [C]
        Real64 TempRefEvapOut = 0.0; // The reference primary loop fluid
        // temperature at the chiller evaporator side outlet [C]
        Real64 TempLowLimitEvapOut = 0.0;                  // Low temperature shut off [C]
        Real64 DesignHeatRecVolFlowRate = 0.0;             // Design water volumetric flow rate through heat recovery loop [m3/s]
        bool DesignHeatRecVolFlowRateWasAutoSized = false; // true if previous input was autosize
        Real64 DesignHeatRecMassFlowRate = 0.0;            // Design water mass flow rate through heat recovery loop [kg/s]
        Real64 SizFac = 0.0;                               // sizing factor
        bool HeatRecActive = false;                        // True when entered Heat Rec Vol Flow Rate > 0
        int HeatRecInletNodeNum = 0;                       // Node number for the heat recovery inlet side of the condenser
        int HeatRecOutletNodeNum = 0;                      // Node number for the heat recovery outlet side of the condenser
        Real64 HeatRecCapacityFraction = 0.0;              // user input for heat recovery capacity fraction []
        Real64 HeatRecMaxCapacityLimit = 0.0;              // Capacity limit for Heat recovery, one time calc [W]
        int HeatRecSetPointNodeNum = 0;                    // index for system node with the heat recover leaving setpoint
        Sched::Schedule *heatRecInletLimitSched = nullptr; // index for schedule for the inlet high limit for heat recovery operation
        int ChillerCapFTIndex = 0;                         // Index for the total cooling capacity modifier curve
        // (function of leaving evaporator and condenser water temperatures)
        int ChillerEIRFTIndex = 0; // Index for the energy input ratio modifier curve
        // (function of leaving evaporator and condenser water temperatures)
        int ChillerEIRFPLRIndex = 0; // Index for the energy input ratio vs part-load ratio curve
        // (function of leaving condenser water temperature and part-load ratio)
        int ChillerCapFTError = 0;           // Used for negative capacity as a function of temp warnings
        int ChillerCapFTErrorIndex = 0;      // Used for negative capacity as a function of temp warnings
        int ChillerEIRFTError = 0;           // Used for negative EIR as a function of temp warnings
        int ChillerEIRFTErrorIndex = 0;      // Used for negative EIR as a function of temp warnings
        int ChillerEIRFPLRError = 0;         // Used for negative EIR as a function of PLR warnings
        int ChillerEIRFPLRErrorIndex = 0;    // Used for negative EIR as a function of PLR warnings
        Real64 ChillerCAPFTXTempMin = 0.0;   // Minimum value of CAPFT curve X variable [C]
        Real64 ChillerCAPFTXTempMax = 0.0;   // Maximum value of CAPFT curve X variable [C]
        Real64 ChillerCAPFTYTempMin = 0.0;   // Minimum value of CAPFT curve Y variable [C]
        Real64 ChillerCAPFTYTempMax = 0.0;   // Maximum value of CAPFT curve Y variable [C]
        Real64 ChillerEIRFTXTempMin = 0.0;   // Minimum value of EIRFT curve X variable [C]
        Real64 ChillerEIRFTXTempMax = 0.0;   // Maximum value of EIRFT curve X variable [C]
        Real64 ChillerEIRFTYTempMin = 0.0;   // Minimum value of EIRFT curve Y variable [C]
        Real64 ChillerEIRFTYTempMax = 0.0;   // Maximum value of EIRFT curve Y variable [C]
        Real64 ChillerEIRFPLRTempMin = 0.0;  // Minimum value of EIRFPLR curve condenser outlet temperature [C]
        Real64 ChillerEIRFPLRTempMax = 0.0;  // Maximum value of EIRFPLR curve condenser outlet temperature [C]
        Real64 ChillerEIRFPLRPLRMin = 0.0;   // Minimum value of EIRFPLR curve part-load ratio
        Real64 ChillerEIRFPLRPLRMax = 0.0;   // Maximum value of EIRFPLR curve part-load ratio
        Real64 ChillerLiftNomMin = 0.0;      // Minimum value of EIRFPLR curve Normalized Chiller lift
        Real64 ChillerLiftNomMax = 10.0;     // Maximum value of EIRFPLR curve Normalized Chiller lift
        Real64 ChillerTdevNomMin = 0.0;      // Minimum value of EIRFPLR curve Normalized Tdev
        Real64 ChillerTdevNomMax = 10.0;     // Maximum value of EIRFPLR curve Normalized Tdev
        int CAPFTXIter = 0;                  // Iteration counter for evaporator outlet temperature CAPFT warning messages
        int CAPFTXIterIndex = 0;             // Index for evaporator outlet temperature CAPFT warning messages
        int CAPFTYIter = 0;                  // Iteration counter for condenser outlet temperature CAPFT warning messages
        int CAPFTYIterIndex = 0;             // Index for condenser outlet temperature CAPFT warning messages
        int EIRFTXIter = 0;                  // Iteration counter for evaporator outlet temperature EIRFT warning messages
        int EIRFTXIterIndex = 0;             // Index for evaporator outlet temperature EIRFT warning messages
        int EIRFTYIter = 0;                  // Iteration counter for condenser outlet temperature EIRFT warning messages
        int EIRFTYIterIndex = 0;             // Index for condenser outlet temperature EIRFT warning messages
        int EIRFPLRTIter = 0;                // Iteration counter for condenser outlet temperature EIRFPLR warning messages
        int EIRFPLRTIterIndex = 0;           // Index for condenser outlet temperature EIRFPLR warning messages
        int EIRFPLRPLRIter = 0;              // Iteration counter for part-load ratio EIRFPLR warning messages
        int EIRFPLRPLRIterIndex = 0;         // Index for part-load ratio EIRFPLR warning messages
        bool FaultyChillerSWTFlag = false;   // True if the chiller has SWT sensor fault
        int FaultyChillerSWTIndex = 0;       // Index of the fault object corresponding to the chiller
        Real64 FaultyChillerSWTOffset = 0.0; // Chiller SWT sensor offset
        int IterLimitExceededNum = 0;        // Iteration limit exceeded for RegulaFalsi routine
        int IterLimitErrIndex = 0;           // Index to iteration limit warning for RegulaFalsi routine
        int IterFailed = 0;                  // Iteration limit failed for RegulaFalsi routine
        int IterFailedIndex = 0;             // Index to iteration limit failed for RegulaFalsi routine
        int DeltaTErrCount = 0;              // Evaporator delta T equals 0 for variable flow chiller warning messages
        int DeltaTErrCountIndex = 0;         // Index to evaporator delta T = 0 for variable flow chiller warning messages
        PlantLocation CWPlantLoc;            // chilled water plant loop component index
        PlantLocation CDPlantLoc;            // condenser water plant loop component index
        PlantLocation HRPlantLoc;            // heat recovery water plant loop component index
        int CondMassFlowIndex = 0;
        bool PossibleSubcooling = false; // flag to indicate chiller is doing less cooling that requested
        // Operational fault parameters
        bool FaultyChillerFoulingFlag = false;   // True if the chiller has fouling fault
        int FaultyChillerFoulingIndex = 0;       // Index of the fault object corresponding to the chiller
        Real64 FaultyChillerFoulingFactor = 1.0; // Chiller fouling factor
        std::string EndUseSubcategory;           // identifier use for the end use subcategory
        bool MyEnvrnFlag = true;
        bool MyInitFlag = true;
        bool MySizeFlag = true;
        Real64 ChillerCondAvgTemp = 0.0;   // average condenser temp for curves with Heat recovery [C]
        Real64 ChillerFalseLoadRate = 0.0; // Chiller false load over and above water side load [J]
        Real64 ChillerCyclingRatio = 0.0;  // Chiller cycling ratio (time on/time step)
        Real64 ChillerPartLoadRatio = 0.0; // Chiller PLR (Load/Capacity)
        Real64 ChillerEIRFPLR = 0.0;       // Chiller EIRFPLR curve output value
        Real64 ChillerEIRFT = 0.0;         // Chiller EIRFT curve output value
        Real64 ChillerCapFT = 0.0;         // Chiller capacity curve output value
        Real64 HeatRecOutletTemp = 0.0;
        Real64 QHeatRecovery = 0.0; // Heat recovered from water-cooled condenser [W]
        Real64 QCondenser = 0.0;
        Real64 QEvaporator = 0.0;                 // Evaporator heat transfer rate [W]
        Real64 Power = 0.0;                       // Chiller power [W]
        Real64 EvapOutletTemp = 0.0;              // Evaporator outlet temperature [C]
        Real64 CondOutletTemp = 0.0;              // Condenser outlet temperature [C]
        Real64 EvapMassFlowRate = 0.0;            // Evaporator mass flow rate [kg/s]
        Real64 CondMassFlowRate = 0.0;            // Condenser mass flow rate [kg/s]
        Real64 ChillerFalseLoad = 0.0;            // Chiller false load over and above water side load [W]
        Real64 Energy = 0.0;                      // Chiller electric consumption [J]
        Real64 EvapEnergy = 0.0;                  // Evaporator heat transfer energy [J]
        Real64 CondEnergy = 0.0;                  // Condenser heat transfer energy [J]
        Real64 CondInletTemp = 0.0;               // Condenser inlet temperature [C]
        Real64 EvapInletTemp = 0.0;               // Evaporator inlet temperature [C]
        Real64 ActualCOP = 0.0;                   // Coefficient of performance
        Real64 EnergyHeatRecovery = 0.0;          // Energy recovered from water-cooled condenser [J]
        Real64 HeatRecInletTemp = 0.0;            // Heat reclaim inlet temperature [C]
        Real64 HeatRecMassFlow = 0.0;             // Heat reclaim mass flow rate [kg/s]
        int ChillerCondLoopFlowFLoopPLRIndex = 0; // Condenser loop flow rate fraction function of loop PLR
        int CondDT = 0;                           // Temperature difference across condenser
        Sched::Schedule *condDTSched = nullptr;   // Temperature difference across condenser schedule
        Real64 MinCondFlowRatio = 0.2;            // Minimum condenser flow fraction
        DataBranchAirLoopPlant::ControlType EquipFlowCtrl = DataBranchAirLoopPlant::ControlType::Invalid;
        Real64 VSBranchPumpMinLimitMassFlowCond = 0.0;
        bool VSBranchPumpFoundCond = false;
        bool VSLoopPumpFoundCond = false;

        // thermosiphon model
        int thermosiphonTempCurveIndex = 0;
        Real64 thermosiphonMinTempDiff = 0.0;
        int thermosiphonStatus = 0;

        static ReformulatedEIRChillerSpecs *factory(EnergyPlusData &state, std::string const &objectName);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void getDesignCapacities(
            EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad) override;

        void getDesignTemperatures(Real64 &TempDesCondIn, Real64 &TempDesEvapOut) override;

        void getSizingFactor(Real64 &sizFac) override;

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation) override;

        void oneTimeInit(EnergyPlusData &state) override;

        void initialize(EnergyPlusData &state, bool RunFlag, Real64 MyLoad);

        void setupOutputVars(EnergyPlusData &state);

        void size(EnergyPlusData &state);

        void control(EnergyPlusData &state, Real64 &MyLoad, bool RunFlag, bool FirstIteration);

        void calculate(EnergyPlusData &state, Real64 &MyLoad, bool RunFlag, Real64 FalsiCondOutTemp);

        void calcHeatRecovery(EnergyPlusData &state, Real64 &QCond, Real64 CondMassFlow, Real64 condInletTemp, Real64 &QHeatRec);

        void update(EnergyPlusData &state, Real64 MyLoad, bool RunFlag);

        void checkMinMaxCurveBoundaries(EnergyPlusData &state, bool FirstIteration);

        bool thermosiphonDisabled(EnergyPlusData &state);
    };

    void GetElecReformEIRChillerInput(EnergyPlusData &state);

} // namespace ChillerReformulatedEIR

struct ChillerReformulatedEIRData : BaseGlobalStruct
{
    bool GetInputREIR = true;
    Array1D<ChillerReformulatedEIR::ReformulatedEIRChillerSpecs> ElecReformEIRChiller;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) ChillerReformulatedEIRData();
    }
};

} // namespace EnergyPlus

#endif
