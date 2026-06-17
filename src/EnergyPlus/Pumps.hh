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

#ifndef Pumps_hh_INCLUDED
#define Pumps_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
// #include <EnergyPlus/FluidProperties.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Pumps {

    // Using/Aliasing

    // Data
    // MODULE PARAMETER DEFINITIONS:

    enum class PumpControlType
    {
        Invalid = -1,
        Continuous,   // Pump control type (pump always running)
        Intermittent, // Pump control type (pump runs only when there is a demand)
        Num
    };

    enum class ControlTypeVFD
    {
        Invalid = -1,
        VFDManual,    // VFD control type (Scheduled RPM)
        VFDAutomatic, // VFD control type (Variable RPM according to flow request)
        Num
    };

    enum class PumpBankControlSeq
    {
        Invalid = -1,
        OptimalScheme,    // Control sequencing for pump bank
        SequentialScheme, // Control sequencing for pump bank
        UserDefined,      // Control sequencing for pump bank
        Num
    };

    enum class PumpType : int
    {
        Invalid = -1,
        VarSpeed,
        ConSpeed,
        Cond,
        Bank_VarSpeed,
        Bank_ConSpeed,
        Num
    };

    enum class PowerSizingMethod
    {
        Invalid = -1,
        SizePowerPerFlow,
        SizePowerPerFlowPerPressure,
        Num
    };

    struct PumpVFDControlData
    {
        // Members
        std::string Name;
        Sched::Schedule *manualRPMSched = nullptr;
        Sched::Schedule *lowerPsetSched = nullptr;
        Sched::Schedule *upperPsetSched = nullptr;
        Sched::Schedule *minRPMSched = nullptr;
        Sched::Schedule *maxRPMSched = nullptr;
        ControlTypeVFD VFDControlType = ControlTypeVFD::Invalid; // VFDControlType
        Real64 MaxRPM = 0.0;                                     // Maximum RPM range value - schedule limit
        Real64 MinRPM = 0.0;                                     // Minimum RPM range value - schedule limit
        Real64 PumpActualRPM = 0.0;                              // RPM recalculated from final flow through the loop
    };

    struct PumpSpecs
    {
        // Members
        std::string Name;                      // user identifier
        PumpType pumpType = PumpType::Invalid; // pump type enumerator, based on local parameter values, used to identify
        // index in the cPumpTypes string array to do error reporting
        DataPlant::PlantEquipmentType TypeOf_Num = DataPlant::PlantEquipmentType::Invalid; // pump type of number in reference to the dataplant values
        PlantLocation plantLoc = {0, DataPlant::LoopSideLocation::Invalid, 0, 0};
        PumpControlType PumpControl = PumpControlType::Invalid;            // Integer equivalent of PumpControlType
        Sched::Schedule *flowRateSched = nullptr;                          // Flow rate modifier schedule, blank/missing --> AlwaysOn
        int InletNodeNum = 0;                                              // Node number on the inlet side of the plant
        int OutletNodeNum = 0;                                             // Node number on the outlet side of the plant
        PumpBankControlSeq SequencingScheme = PumpBankControlSeq::Invalid; // Optimal, Sequential, User-Defined
        // Fluid::RefrigProps *fluid = nullptr;                                                // Index for Fluid Properties
        int NumPumpsInBank = 0;                       // Node number on the inlet side of the plant
        int PowerErrIndex1 = 0;                       // for recurring errors
        int PowerErrIndex2 = 0;                       // for recurring errors
        Real64 MinVolFlowRateFrac = 0.0;              // minimum schedule value fraction modifier
        Real64 NomVolFlowRate = 0.0;                  // design nominal capacity of Pump
        bool NomVolFlowRateWasAutoSized = false;      // true if previous was autosize on input
        Real64 MassFlowRateMax = 0.0;                 // design nominal capacity of Pump
        bool EMSMassFlowOverrideOn = false;           // if true, then EMS is calling to override flow requests.
        Real64 EMSMassFlowValue = 0.0;                // EMS value to use for mass flow rate [kg/s]
        Real64 NomSteamVolFlowRate = 0.0;             // For Steam Pump
        bool NomSteamVolFlowRateWasAutoSized = false; // true if steam volume flow rate was autosize on input
        Real64 MinVolFlowRate = 0.0;                  // For a Variable Flow Pump this is the minimum capacity during operation.
        bool minVolFlowRateWasAutosized = false;      // true if minimum flow rate was autosize on input
        Real64 MassFlowRateMin = 0.0;                 // For a Variable Flow Pump this is the minimum capacity during operation.
        Real64 NomPumpHead = 0.0;                     // design nominal head pressure of Pump, [Pa]
        bool EMSPressureOverrideOn = false;           // if true, EMS is calling to override pump pressure
        Real64 EMSPressureOverrideValue = 0.0;        // EMS value to use for pressure [Pa]
        Real64 NomPowerUse = 0.0;                     // design nominal capacity of Pump
        bool NomPowerUseWasAutoSized = false;         // true if power was autosize on input
        PowerSizingMethod powerSizingMethod = PowerSizingMethod::SizePowerPerFlowPerPressure; // which method is used for sizing nominal power use
        Real64 powerPerFlowScalingFactor = 348701.1;                                          // design electric power per unit flow rate (22 W/gpm)
        Real64 powerPerFlowPerPressureScalingFactor = (1 / .78);   // design shaft power per unit flow rate per unit head (legacy impeller efficiency)
        Real64 MotorEffic = 0.0;                                   // efficiency of the motor
        Real64 PumpEffic = 0.0;                                    // efficiency of the pump
        Real64 FracMotorLossToFluid = 0.0;                         // ?????
        Real64 Energy = 0.0;                                       // Energy consumed
        Real64 Power = 0.0;                                        // Power used
        std::array<Real64, 4> PartLoadCoef = {0.0, 0.0, 0.0, 0.0}; // Pump Curve Coefficients
        int PressureCurve_Index = 0;                               // Pointer to a pump coefficient curve
        Real64 PumpMassFlowRateMaxRPM = 0.0;                       // Mass flow rate calculated from maximum rpm
        Real64 PumpMassFlowRateMinRPM = 0.0;                       // Mass flow rate calculated from minimum rpm
        Real64 MinPhiValue = 0.0;                                  // Minimum value of Phi (from CurveManager)
        Real64 MaxPhiValue = 0.0;                                  // Maximum value of Phi (from CurveManager)
        Real64 ImpellerDiameter = 0.0;                             // Pump Impeller Diameter [m]
        Real64 RotSpeed_RPM = 0.0;                                 // Rotational speed used for input in revs/min
        Real64 RotSpeed = 0.0;                                     // Rotational speed for calculations in revs/sec
        bool PumpInitFlag = true;
        bool PumpOneTimeFlag = true;
        bool CheckEquipName = true;
        bool HasVFD = false;
        PumpVFDControlData VFD;
        bool OneTimePressureWarning = true;
        bool HeatLossesToZone = false;        // if true then pump losses added to surrounding zone
        int ZoneNum = 0;                      // index for zone surrounding pump
        Real64 SkinLossRadFraction = 0.0;     // radiative split for skin losses to zone
        bool LoopSolverOverwriteFlag = false; // loop solver overwrite for determining pump minimum flow rate
        std::string EndUseSubcategoryName;
    };

    struct ReportVars
    {
        // Members
        int NumPumpsOperating = 0;          // Used in pump bank. reports how many pumps are ON
        Real64 PumpMassFlowRate = 0.0;      // Mass flow rate of the pump
        Real64 PumpHeattoFluid = 0.0;       // Heat transfer from pump to fluid (W)
        Real64 PumpHeattoFluidEnergy = 0.0; // Pump Energy dissipated into fluid stream
        Real64 OutletTemp = 0.0;            // Pump outlet temperature
        Real64 ShaftPower = 0.0;            // Power input at the shaft
        Real64 ZoneTotalGainRate = 0.0;     // total pump skin losses to zone (W)
        Real64 ZoneTotalGainEnergy = 0.0;   // total pump skin losses to zone energy (J)
        Real64 ZoneConvGainRate = 0.0;      // pump skin losses convecting to zone air (W)
        Real64 ZoneRadGainRate = 0.0;       // pump skin losses radiating to inside of zone (W)
    };

    void SimPumps(EnergyPlusData &state,
                  std::string const &PumpName, // Name of pump to be managed
                  int const LoopNum,           // Plant loop number
                  Real64 const FlowRequest,    // requested flow from adjacent demand side
                  bool &PumpRunning,           // .TRUE. if the loop pump is actually operating
                  int &PumpIndex,
                  Real64 &PumpHeat);

    void GetPumpInput(EnergyPlusData &state);

    void InitializePumps(EnergyPlusData &state, int const PumpNum);

    void SetupPumpMinMaxFlows(EnergyPlusData &state, int const LoopNum, int const PumpNum);

    void CalcPumps(EnergyPlusData &state, int const PumpNum, Real64 const FlowRequest, bool &PumpRunning);

    void SizePump(EnergyPlusData &state, int const PumpNum);

    void ReportPumps(EnergyPlusData &state, int const PumpNum);

    void PumpDataForTable(EnergyPlusData &state, int const NumPump);

    void GetRequiredMassFlowRate(EnergyPlusData &state,
                                 int const LoopNum,
                                 int const PumpNum,
                                 Real64 const InletNodeMassFlowRate,
                                 Real64 &ActualFlowRate,
                                 Real64 &PumpMinMassFlowRateVFDRange,
                                 Real64 &PumpMaxMassFlowRateVFDRange);

} // namespace Pumps

struct PumpsData : BaseGlobalStruct
{

    int NumPumps = 0;         // Num Pumps (used in pump bank)
    int NumPumpsRunning = 0;  // Num of pumps ON (used in pump bank)
    int NumPumpsFullLoad = 0; // Num pumps running at full load (used in pump bank)
    bool GetInputFlag = true;
    Real64 PumpMassFlowRate = 0.0; // mass flow rate at pump inlet node
    Real64 PumpHeattoFluid = 0.0;  // Pump Power dissipated in fluid stream
    Real64 Power = 0.0;            // Pump Electric power
    Real64 ShaftPower = 0.0;       // Power passing through pump shaft
    EPVector<Pumps::PumpSpecs> PumpEquip;
    EPVector<Pumps::ReportVars> PumpEquipReport;
    std::unordered_map<std::string, std::string> PumpUniqueNames;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumPumps = 0;
        this->NumPumpsRunning = 0;
        this->NumPumpsFullLoad = 0;
        this->GetInputFlag = true;
        this->PumpMassFlowRate = 0.0;
        this->PumpHeattoFluid = 0.0;
        this->Power = 0.0;
        this->ShaftPower = 0.0;
        this->PumpEquip.deallocate();
        this->PumpEquipReport.deallocate();
        this->PumpUniqueNames.clear();
    }
};

} // namespace EnergyPlus

#endif
