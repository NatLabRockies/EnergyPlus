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

#ifndef UnitVentilator_hh_INCLUDED
#define UnitVentilator_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace UnitVentilator {

    enum class CoilsUsed
    {
        Invalid = -1,
        None,
        Both,
        Heating,
        Cooling,
        Num
    };
    enum class HeatCoilType
    {
        Invalid = -1,
        Electric,
        Gas,
        Water,
        Steam,
        Num
    };
    enum class CoolCoilType
    {
        Invalid = -1,
        Water,
        Detailed,
        HXAssisted,
        Num
    };
    enum class OAControl
    {
        Invalid = -1,
        VariablePercent,
        FixedTemperature,
        FixedAmount,
        Num
    };

    struct UnitVentilatorData
    {
        std::string Name; // name of unit
        Sched::Schedule *availSched = nullptr;
        int AirInNode = 0;     // inlet air node number
        int AirOutNode = 0;    // outlet air node number
        int FanOutletNode = 0; // outlet node number for fan exit
        // (assumes fan is upstream of heating coil)
        HVAC::FanType fanType = HVAC::FanType::Invalid; // Fan type number (see DataHVACGlobals)
        std::string FanName;                            // name of fan
        int Fan_Index = 0;
        Sched::Schedule *fanOpModeSched = nullptr; // index to fan operating mode schedule
        Sched::Schedule *fanAvailSched = nullptr;
        HVAC::FanOp fanOp = HVAC::FanOp::Invalid; // mode of operation; 1=cycling fan, cycling coil, 2=continuous fan, cycling coil
        int ControlCompTypeNum = 0;
        int CompErrIndex = 0;
        Real64 MaxAirVolFlow = 0.0;                   // m3/s
        Real64 MaxAirMassFlow = 0.0;                  // kg/s
        OAControl OAControlType = OAControl::Invalid; // type of control; options are VARIABLE PERCENT and FIXED TEMPERATURE
        Sched::Schedule *minOASched = nullptr;
        Sched::Schedule *maxOASched = nullptr;
        Sched::Schedule *tempSched = nullptr;
        int OutsideAirNode = 0;                         // outside air node number
        int AirReliefNode = 0;                          // relief air node number
        int OAMixerOutNode = 0;                         // outlet node after the outside air mixer (inlet to coils if present)
        Real64 OutAirVolFlow = 0.0;                     // m3/s
        Real64 OutAirMassFlow = 0.0;                    // kg/s
        Real64 MinOutAirVolFlow = 0.0;                  // m3/s
        Real64 MinOutAirMassFlow = 0.0;                 // kg/s
        CoilsUsed CoilOption = CoilsUsed::Invalid;      // type of coil option; options are BOTH, HEATING, COOLING, AND NONE
        bool HCoilPresent = false;                      // .TRUE. if unit ventilator has a heating coil
        HeatCoilType HCoilType = HeatCoilType::Invalid; // type of heating coil (water, gas, electric, etc.)
        std::string HCoilName;                          // name of heating coil
        std::string HCoilTypeCh;                        // type of heating coil character string (same as type on idf file).
        int HCoil_Index = 0;
        DataPlant::PlantEquipmentType HeatingCoilType = DataPlant::PlantEquipmentType::Invalid;

        Fluid::RefrigProps *HCoil_fluid = nullptr;
        Sched::Schedule *hCoilSched = nullptr;

        Real64 HCoilSchedValue = 0.0;
        Real64 MaxVolHotWaterFlow = 0.0; // m3/s
        Real64 MaxVolHotSteamFlow = 0.0; // m3/s
        Real64 MaxHotWaterFlow = 0.0;    // kg/s
        Real64 MaxHotSteamFlow = 0.0;
        Real64 MinHotSteamFlow = 0.0;
        Real64 MinVolHotWaterFlow = 0.0; // m3/s
        Real64 MinVolHotSteamFlow = 0.0; // m3/s
        Real64 MinHotWaterFlow = 0.0;    // kg/s
        int HotControlNode = 0;          // hot water control node
        int HotCoilOutNodeNum = 0;       // outlet of coil
        Real64 HotControlOffset = 0.0;   // control tolerance
        PlantLocation HWplantLoc;        // index for plant location for hot water coil
        bool CCoilPresent = false;       // .TRUE. if unit ventilator has a cooling coil
        std::string CCoilName;           // name of cooling coil
        std::string CCoilTypeCh;         // type of cooling coil as character string (same as on idf file)
        int CCoil_Index = 0;
        std::string CCoilPlantName; // name of cooling coil for plant
        std::string CCoilPlantType; // type of cooling coil for plant
        DataPlant::PlantEquipmentType CoolingCoilType = DataPlant::PlantEquipmentType::Invalid;
        CoolCoilType CCoilType = CoolCoilType::Invalid;
        Sched::Schedule *cCoilSched = nullptr;
        Real64 CCoilSchedValue = 0.0;
        Real64 MaxVolColdWaterFlow = 0.0; // m3/s
        Real64 MaxColdWaterFlow = 0.0;    // kg/s
        Real64 MinVolColdWaterFlow = 0.0; // m3/s
        Real64 MinColdWaterFlow = 0.0;    // kg/s
        int ColdControlNode = 0;          // chilled water control node
        int ColdCoilOutNodeNum = 0;       // chilled water coil out node
        Real64 ColdControlOffset = 0.0;   // control tolerance
        PlantLocation CWPlantLoc;         // index for plant component for chilled water coil
        // Report data
        Real64 HeatPower = 0.0;  // unit heating output in watts
        Real64 HeatEnergy = 0.0; // unit heating output in J
        Real64 TotCoolPower = 0.0;
        Real64 TotCoolEnergy = 0.0;
        Real64 SensCoolPower = 0.0;
        Real64 SensCoolEnergy = 0.0;
        Real64 ElecPower = 0.0;
        Real64 ElecEnergy = 0.0;
        std::string AvailManagerListName; // Name of an availability manager list object
        Avail::Status availStatus = Avail::Status::NoAction;
        Real64 FanPartLoadRatio = 0.0;                          // fan part-load ratio for time step
        Real64 PartLoadFrac = 0.0;                              // unit ventilator part-load ratio for time step
        int ZonePtr = 0;                                        // pointer to a zone served by a unit ventilator
        int HVACSizingIndex = 0;                                // index of a HVACSizing object for a unit ventilator
        bool ATMixerExists = false;                             // True if there is an ATMixer
        std::string ATMixerName;                                // name of air mixer
        int ATMixerIndex = 0;                                   // index to the air mixer
        HVAC::MixerType ATMixerType = HVAC::MixerType::Invalid; // 1 = inlet side mixer, 2 = supply side mixer
        int ATMixerPriNode = 0;                                 // primary inlet air node number for the mixer
        int ATMixerSecNode = 0;                                 // secondary air inlet node number for the mixer
        int ATMixerOutNode = 0;                                 // outlet air node number for the mixer
        bool FirstPass = true;                                  // detects first time through for resetting sizing data

        UnitVentilatorData() = default;
        ~UnitVentilatorData()
        {
        }
    };

    struct UnitVentNumericFieldData
    {
        // Members
        Array1D_string FieldNames;

        UnitVentNumericFieldData() = default;
        ~UnitVentNumericFieldData()
        {
        }
    };

    void SimUnitVentilator(EnergyPlusData &state,
                           std::string_view CompName,     // name of the fan coil unit
                           int const ZoneNum,             // number of zone being served
                           bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                           Real64 &PowerMet,              // Sensible power supplied (W)
                           Real64 &LatOutputProvided,     // Latent add/removal supplied by window AC (kg/s), dehumid = negative
                           int &CompIndex);

    void GetUnitVentilatorInput(EnergyPlusData &state);

    void InitUnitVentilator(EnergyPlusData &state,
                            int const UnitVentNum,         // index for the current unit ventilator
                            bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                            int const ZoneNum              // number of zone being served
    );

    void SizeUnitVentilator(EnergyPlusData &state, int const UnitVentNum);

    void CalcUnitVentilator(EnergyPlusData &state,
                            int &UnitVentNum,              // number of the current fan coil unit being simulated
                            int const ZoneNum,             // number of zone being served
                            bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                            Real64 &PowerMet,              // Sensible power supplied (W)
                            Real64 &LatOutputProvided      // Latent power supplied (kg/s), negative = dehumidification
    );

    void CalcUnitVentilatorComponents(EnergyPlusData &state,
                                      int const UnitVentNum,         // Unit index in unit ventilator array
                                      bool const FirstHVACIteration, // flag for 1st HVAV iteration in the time step
                                      Real64 &LoadMet,               // load met by unit (watts)
                                      ObjexxFCL::Optional<HVAC::FanOp const> fanOp = HVAC::FanOp::Continuous, // Fan Type
                                      ObjexxFCL::Optional<Real64 const> PartLoadFrac = 1.0                    // Part Load Ratio of coil and fan
    );

    void SimUnitVentOAMixer(EnergyPlusData &state,
                            int const UnitVentNum,  // Unit index in unit ventilator array
                            HVAC::FanOp const fanOp // unit ventilator fan operating mode
    );

    void ReportUnitVentilator(EnergyPlusData &state, int const UnitVentNum); // Unit index in unit ventilator array

    int GetUnitVentilatorOutAirNode(EnergyPlusData &state, int const UnitVentNum);

    int GetUnitVentilatorZoneInletAirNode(EnergyPlusData &state, int const UnitVentNum);

    int GetUnitVentilatorMixedAirNode(EnergyPlusData &state, int const UnitVentNum);

    int GetUnitVentilatorReturnAirNode(EnergyPlusData &state, int const UnitVentNum);

    int getUnitVentilatorIndex(EnergyPlusData &state, std::string_view CompName);

    Real64 SetOAMassFlowRateForCoolingVariablePercent(EnergyPlusData &state,
                                                      int const UnitVentNum,     // Unit Ventilator index number
                                                      Real64 const MinOAFrac,    // Minimum Outside Air Fraction
                                                      Real64 const MassFlowRate, // Design Outside Air Mass Flow Rate
                                                      Real64 const MaxOAFrac,    // Maximum Outside Air Fraction
                                                      Real64 const Tinlet,       // Inlet Temperature to Unit or Zone Temperature
                                                      Real64 const Toutdoor      // Outdoor Air Temperature
    );

    void CalcMdotCCoilCycFan(EnergyPlusData &state,
                             Real64 &mdot,              // mass flow rate
                             Real64 &QCoilReq,          // Remaining cooling coil load
                             Real64 const QZnReq,       // Zone load to setpoint
                             int const UnitVentNum,     // Unit Ventilator index
                             Real64 const PartLoadRatio // Part load ratio for unit ventilator
    );

} // namespace UnitVentilator

struct UnitVentilatorsData : BaseGlobalStruct
{

    // Current Module Unit type
    std::string const cMO_UnitVentilator = "ZoneHVAC:UnitVentilator";

    bool HCoilOn = false;        // TRUE if the heating coil  = gas or electric especially) should be running
    int NumOfUnitVents = 0;      // Number of unit ventilators in the input file
    Real64 OAMassFlowRate = 0.0; // Outside air mass flow rate for the unit ventilator
    Real64 QZnReq = 0.0;         // heating or cooling needed by zone [watts]
    Array1D_bool MySizeFlag;
    bool GetUnitVentilatorInputFlag = true; // First time, input is "gotten"
    Array1D_bool CheckEquipName;

    EPVector<UnitVentilator::UnitVentilatorData> UnitVent;
    EPVector<UnitVentilator::UnitVentNumericFieldData> UnitVentNumericFields;

    bool MyOneTimeFlag = true;
    bool ZoneEquipmentListChecked = false; // True after the Zone Equipment List has been checked for items

    Array1D_bool MyEnvrnFlag;
    Array1D_bool MyPlantScanFlag;
    Array1D_bool MyZoneEqFlag;

    int ATMixOutNode = 0;   // outlet node of ATM Mixer
    int ATMixerPriNode = 0; // primary air node of ATM Mixer
    int ZoneNode = 0;       // zone node

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->HCoilOn = false;
        this->NumOfUnitVents = 0;
        this->OAMassFlowRate = 0.0;
        this->QZnReq = 0.0;
        this->GetUnitVentilatorInputFlag = true;
        this->MySizeFlag.deallocate();
        this->CheckEquipName.deallocate();
        this->UnitVent.deallocate();
        this->UnitVentNumericFields.deallocate();
        this->MyOneTimeFlag = true;
        this->ZoneEquipmentListChecked = false;
        this->MyEnvrnFlag.deallocate();
        this->MyPlantScanFlag.deallocate();
        this->MyZoneEqFlag.deallocate();
        this->ATMixOutNode = 0;
        this->ATMixerPriNode = 0;
        this->ZoneNode = 0;
    }

    // Default Constructor
    UnitVentilatorsData() = default;
    ~UnitVentilatorsData()
    {
    }
};
} // namespace EnergyPlus

#endif
