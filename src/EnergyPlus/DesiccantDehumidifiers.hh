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

#ifndef DesiccantDehumidifiers_hh_INCLUDED
#define DesiccantDehumidifiers_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DesiccantDehumidifiers {

    enum class DesicDehumType // Desiccant dehumidifier type
    {
        Invalid = -1,
        Solid,
        Generic,
        Num
    };

    enum class DesicDehumCtrlType
    {
        Invalid = -1,
        FixedHumratBypass, // FIXED LEAVING HUMRAT SETPOINT:BYPASS
        NodeHumratBypass,  // NODE LEAVING HUMRAT SETPOINT:BYPASS
        Num
    };

    enum class Selection
    {
        Invalid = -1,
        No,  // Condenser waste heat NOT reclaimed for desiccant regeneration
        Yes, // Condenser waste heat reclaimed for desiccant regeneration
        Num
    };

    enum class PerformanceModel
    {
        Invalid = -1,
        Default,
        UserCurves,
        Num
    };

    //  Desiccant heat exchanger type
    int constexpr BalancedHX(1); // HeatExchanger:Desiccant:BalancedFlow = 1

    struct DesiccantDehumidifierData
    {
        // Members
        // User Input data
        std::string Name;                                       // unique name of component
        std::string Sched;                                      // name of availability schedule
        HVAC::CoilType regenCoilType = HVAC::CoilType::Invalid; // type of regen coil
        std::string RegenCoilName;                              // name of regen coil
        std::string RegenFanName;                               // name of regen fan
        PerformanceModel PerformanceModel_Num;                  // type of performance model, default or user curves
        int ProcAirInNode;                                      // process air inlet node of dehumidifier
        int ProcAirOutNode;                                     // process air outlet node of dehumidifier
        int RegenAirInNode;                                     // regen air inlet node of dehumidifier
        // (initially set to conditions entering regen heating coil)
        int RegenAirOutNode;            // regen air outlet node of dehumidifier
        int RegenFanInNode;             // regen fan inlet node
        DesicDehumCtrlType controlType; // type of controls
        Real64 HumRatSet;               // humidity ratio setpoint [kg water / kg air]
        Real64 NomProcAirVolFlow;       // nominal process air flow rate [m3/s]
        Real64 NomProcAirVel;           // nominal process air velocity [m/s]
        Real64 NomRotorPower;           // rotor power consumption at full output [W]
        int RegenCoilIndex;             // Index for regen coil
        int RegenFanIndex;              // Index for regen fan
        HVAC::FanType regenFanType;     // Fan type number (see DataHVACGlobals)
        int ProcDryBulbCurvefTW;        // number of process leaving dry bulb f(edb,ew) curve
        int ProcDryBulbCurvefV;         // number of process leaving dry bulb f(v) curve
        int ProcHumRatCurvefTW;         // number of process leaving humidity ratio f(edb,ew) curve
        int ProcHumRatCurvefV;          // number of process leaving humidity ratio f(v) curve
        int RegenEnergyCurvefTW;        // number of regen energy f(edb,ew) curve
        int RegenEnergyCurvefV;         // number of regen energy f(v) curve
        int RegenVelCurvefTW;           // number of regen velocity f(edb,ew) curve
        int RegenVelCurvefV;            // number of regen velocity f(v) curve
        Real64 NomRegenTemp;            // nominal regen temperature for regen energy curve [C]
        // Possible future inputs, hardwired for now depending on which performance model is in use, unit off if out of bounds
        Real64 MinProcAirInTemp;   // min allowable process inlet air temperature [C]
        Real64 MaxProcAirInTemp;   // max allowable process inlet air temperature [C]
        Real64 MinProcAirInHumRat; // min allowable process inlet air humidity ratio [kg water / kg air]
        Real64 MaxProcAirInHumRat; // max allowable process inlet air humidity ratio [kg water / kg air]
        // Internal Data
        Sched::Schedule *availSched = nullptr; // availability schedule
        Real64 NomProcAirMassFlow;             // nominal process air mass flow rate [kg/s]
        Real64 NomRegenAirMassFlow;            // nominal regeneration air mass flow rate [kg/s]
        Real64 ProcAirInTemp;                  // process inlet air temperature [C]
        Real64 ProcAirInHumRat;                // process inlet air humidity ratio [kg water / kg air]
        Real64 ProcAirInEnthalpy;              // process inlet air specific enthalpy [J/kg]
        Real64 ProcAirInMassFlowRate;          // process inlet air mass flow rate [kg/s]
        Real64 ProcAirOutTemp;                 // process outlet air temperature [C]
        Real64 ProcAirOutHumRat;               // process outlet air humidity ratio [kg water / kg air]
        Real64 ProcAirOutEnthalpy;             // process outlet air specific enthalpy [J/kg]
        Real64 ProcAirOutMassFlowRate;         // process outlet air mass flow rate [kg/s]
        Real64 RegenAirInTemp;                 // regen inlet air temperature [C]
        Real64 RegenAirInHumRat;               // regen inlet air humidity ratio [kg water / kg air]
        Real64 RegenAirInEnthalpy;             // regen inlet air specific enthalpy [J/kg]
        Real64 RegenAirInMassFlowRate;         // regen inlet air mass flow rate [kg/s]
        Real64 RegenAirVel;                    // regen air velocity [m/s]
        std::string DehumType;                 // Type of desiccant dehumidifier
        DesicDehumType DehumTypeCode;          // Type of desiccant dehumidifier, integer code
        Real64 WaterRemove;                    // water removed [kg]
        Real64 WaterRemoveRate;                // water removal rate [kg/s]
        Real64 SpecRegenEnergy;                // specific regen energy [J/kg of water removed]
        Real64 QRegen;                         // regen energy rate requested from regen coil [W]
        Real64 RegenEnergy;                    // regen energy requested from regen coil [J]
        Real64 ElecUseEnergy;                  // electricity consumption [J]
        Real64 ElecUseRate;                    // electricity consumption rate [W]
        Real64 PartLoad;                       // fraction of dehumidification capacity required to meet setpoint
        int RegenCapErrorIndex1;               // recurring error message index for insufficient regen coil capacity
        int RegenCapErrorIndex2;               // recurring error message index for insufficient regen coil capacity
        int RegenCapErrorIndex3;               // recurring error message index for insufficient regen coil capacity
        int RegenCapErrorIndex4;               // recurring error message index for insufficient regen coil capacity
        int RegenFanErrorIndex1;               // recurring error message index for incorrect regen fan flow
        int RegenFanErrorIndex2;               // recurring error message index for incorrect regen fan flow
        int RegenFanErrorIndex3;               // recurring error message index for incorrect regen fan flow
        int RegenFanErrorIndex4;               // recurring error message index for incorrect regen fan flow
        // structure elements unique to generic desiccant dehumidifier
        std::string HXType;                                    // type of desiccant heat exchanger
        std::string HXName;                                    // name of desiccant heat exchanger
        int HXTypeNum;                                         // parameter number of desiccant heat exchanger
        std::string ExhaustFanCurveObject;                     // exhaust fan curve object
        std::string CoolingCoilType;                           // type of cooling coil used with desiccant heat exchanger
        std::string CoolingCoilName;                           // name of cooling coil used with desiccant heat exchanger
        HVAC::CoilType coolCoilType = HVAC::CoilType::Invalid; // type of cooling coil, DataHVACGlobals coil type constants
        Selection Preheat;                                     // determine condenser waste heat usage for pre heating regen air
        Real64 RegenSetPointTemp;                              // heating set-point for regeneration air [C]
        Real64 ExhaustFanMaxVolFlowRate;                       // exhaust fan maximum allowable air flow rate [m3/s]
        Real64 ExhaustFanMaxMassFlowRate;                      // exhaust fan maximum allowable air mass flow rate [kg/s]
        Real64 ExhaustFanMaxPower;                             // exhaust fan maximum allowable power [W]
        Real64 ExhaustFanPower;                                // exhaust fan power for reporting [W]
        Real64 ExhaustFanElecConsumption;                      // exhaust fan electric consumption for reporting [J]
        Real64 CompanionCoilCapacity;                          // DX coil capacity for dehumidifier companion cooling coil [W]
        HVAC::FanPlace regenFanPlace;                          // placement of the fan used for regeneration air flow
        int ControlNodeNum;                                    // node number of control node
        int ExhaustFanCurveIndex;                              // exhaust fan curve object index
        int CompIndex;                                         // index of HX component to call simheatrecovery
        int CoolingCoilOutletNode;                             // node number of cooling coil outlet node
        int RegenFanOutNode;                                   // fan outlet node number mined from regen fan object
        int RegenCoilInletNode;                                // regen heating coil inlet node number mined from regen heater object
        int RegenCoilOutletNode;                               // regen heating coil outlet node number mined from regen heater object
        int HXProcInNode;                                      // process inlet node num mined from desiccant heat exchanger object
        int HXProcOutNode;                                     // process outlet node num mined from desiccant heat exchanger object
        int HXRegenInNode;                                     // regen inlet node number mined from desiccant heat exchanger object
        int HXRegenOutNode;                                    // regen outlet node number mined from desiccant heat exchanger object
        int CondenserInletNode;                                // regen outlet node number mined from desiccant heat exchanger object
        int DXCoilIndex;                                       // DX Coil index mined from coil object
        int ErrCount;                                          // error count
        int ErrIndex1;                                         // error index
        Selection CoilUpstreamOfProcessSide;                   // used to determine if process inlet is pre-cooled
        bool RegenInletIsOutsideAirNode;                       // regen inlet is connected to an outside air node
        int CoilControlNode;                                   // heating coil hot water or steam inlet node
        int CoilOutletNode;                                    // outlet node for water coil
        PlantLocation plantLoc;                                // plant loop component location for water heating coil
        int HotWaterCoilMaxIterIndex;                          // Index to recurring warning message
        int HotWaterCoilMaxIterIndex2;                         // Index to recurring warning message
        Real64 MaxCoilFluidFlow;                               // hot water or steam mass flow rate regen. heating coil [kg/s]
        Real64 RegenCoilCapacity;                              // hot water or steam coil operating capacity [W]

        // Default Constructor
        DesiccantDehumidifierData()
            : PerformanceModel_Num(PerformanceModel::Invalid), ProcAirInNode(0), ProcAirOutNode(0), RegenAirInNode(0), RegenAirOutNode(0),
              RegenFanInNode(0), controlType(DesicDehumCtrlType::Invalid), HumRatSet(0.0), NomProcAirVolFlow(0.0), NomProcAirVel(0.0),
              NomRotorPower(0.0), RegenCoilIndex(0), RegenFanIndex(0), regenFanType(HVAC::FanType::Invalid), ProcDryBulbCurvefTW(0),
              ProcDryBulbCurvefV(0), ProcHumRatCurvefTW(0), ProcHumRatCurvefV(0), RegenEnergyCurvefTW(0), RegenEnergyCurvefV(0), RegenVelCurvefTW(0),
              RegenVelCurvefV(0), NomRegenTemp(121.0), MinProcAirInTemp(-73.3), MaxProcAirInTemp(65.6), MinProcAirInHumRat(0.0),
              MaxProcAirInHumRat(0.21273), NomProcAirMassFlow(0.0), NomRegenAirMassFlow(0.0), ProcAirInTemp(0.0), ProcAirInHumRat(0.0),
              ProcAirInEnthalpy(0.0), ProcAirInMassFlowRate(0.0), ProcAirOutTemp(0.0), ProcAirOutHumRat(0.0), ProcAirOutEnthalpy(0.0),
              ProcAirOutMassFlowRate(0.0), RegenAirInTemp(0.0), RegenAirInHumRat(0.0), RegenAirInEnthalpy(0.0), RegenAirInMassFlowRate(0.0),
              RegenAirVel(0.0), DehumTypeCode(DesicDehumType::Invalid), WaterRemove(0.0), WaterRemoveRate(0.0), SpecRegenEnergy(0.0), QRegen(0.0),
              RegenEnergy(0.0), ElecUseEnergy(0.0), ElecUseRate(0.0), PartLoad(0.0), RegenCapErrorIndex1(0), RegenCapErrorIndex2(0),
              RegenCapErrorIndex3(0), RegenCapErrorIndex4(0), RegenFanErrorIndex1(0), RegenFanErrorIndex2(0), RegenFanErrorIndex3(0),
              RegenFanErrorIndex4(0), HXTypeNum(0), Preheat(Selection::Invalid), RegenSetPointTemp(0.0), ExhaustFanMaxVolFlowRate(0.0),
              ExhaustFanMaxMassFlowRate(0.0), ExhaustFanMaxPower(0.0), ExhaustFanPower(0.0), ExhaustFanElecConsumption(0.0),
              CompanionCoilCapacity(0.0), regenFanPlace(HVAC::FanPlace::Invalid), ControlNodeNum(0), ExhaustFanCurveIndex(0), CompIndex(0),
              CoolingCoilOutletNode(0), RegenFanOutNode(0), RegenCoilInletNode(0), RegenCoilOutletNode(0), HXProcInNode(0), HXProcOutNode(0),
              HXRegenInNode(0), HXRegenOutNode(0), CondenserInletNode(0), DXCoilIndex(0), ErrCount(0), ErrIndex1(0),
              CoilUpstreamOfProcessSide(Selection::Invalid), RegenInletIsOutsideAirNode(false), CoilControlNode(0), CoilOutletNode(0),
              HotWaterCoilMaxIterIndex(0), HotWaterCoilMaxIterIndex2(0), MaxCoilFluidFlow(0.0), RegenCoilCapacity(0.0)
        {
        }
    };

    // Functions

    void SimDesiccantDehumidifier(EnergyPlusData &state,
                                  std::string const &CompName, // name of the dehumidifier unit
                                  bool FirstHVACIteration,     // TRUE if 1st HVAC simulation of system timestep
                                  int &CompIndex);

    void GetDesiccantDehumidifierInput(EnergyPlusData &state);

    void InitDesiccantDehumidifier(EnergyPlusData &state,
                                   int DesicDehumNum,      // number of the current dehumidifier being simulated
                                   bool FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    );

    void ControlDesiccantDehumidifier(EnergyPlusData &state,
                                      int DesicDehumNum,      // number of the current dehumidifier being simulated
                                      Real64 &HumRatNeeded,   // process air leaving humidity ratio set by controller [kg water/kg air]
                                      bool FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep !unused1208
    );

    void CalcSolidDesiccantDehumidifier(EnergyPlusData &state,
                                        int DesicDehumNum,      // number of the current dehumidifier being simulated
                                        Real64 HumRatNeeded,    // process air leaving humidity ratio set by controller [kgWater/kgDryAir]
                                        bool FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    );

    void CalcGenericDesiccantDehumidifier(EnergyPlusData &state,
                                          int DesicDehumNum,      // number of the current dehumidifier being simulated
                                          Real64 HumRatNeeded,    // process air leaving humidity ratio set by controller [kg water/kg air]
                                          bool FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    );

    void UpdateDesiccantDehumidifier(EnergyPlusData &state, int DesicDehumNum); // number of the current dehumidifier being simulated

    void ReportDesiccantDehumidifier(EnergyPlusData &state, int DesicDehumNum); // number of the current dehumidifier being simulated

    void CalcNonDXHeatingCoils(EnergyPlusData &state,
                               int DesicDehumNum,                               // Desiccant dehumidifier unit index
                               bool FirstHVACIteration,                         // flag for first HVAC iteration in the time step
                               Real64 RegenCoilLoad,                            // heating coil load to be met (Watts)
                               ObjexxFCL::Optional<Real64> RegenCoilLoadmet = _ // heating load met
    );

    int GetProcAirInletNodeNum(EnergyPlusData &state, std::string const &DesicDehumName, bool &ErrorsFound);

    int GetProcAirOutletNodeNum(EnergyPlusData &state, std::string const &DesicDehumName, bool &ErrorsFound);

} // namespace DesiccantDehumidifiers

struct DesiccantDehumidifiersData : BaseGlobalStruct
{

    int NumDesicDehums = 0;                    // number of desiccant dehumidifiers of all types
    int NumSolidDesicDehums = 0;               // number of solid desiccant dehumidifiers
    int NumGenericDesicDehums = 0;             // number of generic desiccant dehumidifiers
    bool GetInputDesiccantDehumidifier = true; // First time, input is "gotten"
    bool InitDesiccantDehumidifierOneTimeFlag = true;
    bool MySetPointCheckFlag = true; // I think this actually needs to be a vector or a member variable on the struct, not just a single bool
    Array1D<DesiccantDehumidifiers::DesiccantDehumidifierData> DesicDehum;
    std::unordered_map<std::string, std::string> UniqueDesicDehumNames;

    Array1D_bool MyEnvrnFlag;
    Array1D_bool MyPlantScanFlag; // Used for init plant component for heating coils
    Real64 QRegen = 0.0;          // required coil load passed to sim heating coil routine (W)

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) DesiccantDehumidifiersData();
    }
};

} // namespace EnergyPlus

#endif
