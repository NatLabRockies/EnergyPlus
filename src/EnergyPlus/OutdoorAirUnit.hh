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

#ifndef OutdoorAirUnit_hh_INCLUDED
#define OutdoorAirUnit_hh_INCLUDED

#include <unordered_set>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACSystems.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace OutdoorAirUnit {

    enum class CompType : int
    {
        Invalid = -1,
        WaterCoil_Cooling,       // "COIL:COOLING:WATER",
        WaterCoil_SimpleHeat,    // "COIL:HEATING:WATER",
        SteamCoil_AirHeat,       // "COIL:HEATING:STEAM",
        Coil_ElectricHeat,       // "COIL:HEATING:ELECTRIC",
        WaterCoil_DetailedCool,  // "COIL:COOLING:WATER:DETAILEDGEOMETRY",
        WaterCoil_CoolingHXAsst, // "COILSYSTEM:COOLING:WATER:HEATEXCHANGERASSISTED",
        Coil_GasHeat,            // "COIL:HEATING:FUEL",
        DXSystem,                // "COILSYSTEM:COOLING:DX",
        HeatXchngrFP,            // "HEATEXCHANGER:AIRTOAIR:FLATPLATE",
        HeatXchngrSL,            // "HEATEXCHANGER:AIRTOAIR:SENSIBLEANDLATENT",
        Desiccant,               // "DEHUMIDIFIER:DESICCANT:NOFANS",
        DXHeatPumpSystem,        // "COILSYSTEM:HEATING:DX",
        UnitarySystemModel,      // "AIRLOOPHVAC:UNITARYSYSTEM",
        Num
    };

    static constexpr std::array<std::string_view, static_cast<int>(CompType::Num)> CompTypeNames{
        "Coil:Cooling:Water",
        "Coil:Heating:Water",
        "Coil:Heating:Steam",
        "Coil:Heating:Electric",
        "Coil:Cooling:Water:DetailedGeometry",
        "CoilSystem:Cooling:Water:HeatExchangerAssisted",
        "Coil:Heating:Fuel",
        "CoilSystem:Cooling:DX",
        "HeatExchanger:AirToAir:FlatPlate",
        "HeatExchanger:AirToAir:SensibleAndLatent",
        "Dehumidifier:Desiccant:NoFans",
        "CoilSystem:Heating:DX",
        "AirLoopHVAC:UnitarySystem",
    };

    static constexpr std::array<std::string_view, static_cast<int>(CompType::Num)> CompTypeNamesUC{

        "COIL:COOLING:WATER",
        "COIL:HEATING:WATER",
        "COIL:HEATING:STEAM",
        "COIL:HEATING:ELECTRIC",
        "COIL:COOLING:WATER:DETAILEDGEOMETRY",
        "COILSYSTEM:COOLING:WATER:HEATEXCHANGERASSISTED",
        "COIL:HEATING:FUEL",
        "COILSYSTEM:COOLING:DX",
        "HEATEXCHANGER:AIRTOAIR:FLATPLATE",
        "HEATEXCHANGER:AIRTOAIR:SENSIBLEANDLATENT",
        "DEHUMIDIFIER:DESICCANT:NOFANS",
        "COILSYSTEM:HEATING:DX",
        "AIRLOOPHVAC:UNITARYSYSTEM",
    };

    enum class OAUnitCtrlType
    {
        Invalid = -1,
        Neutral,
        Unconditioned,
        Temperature,
        Num
    };

    enum class Operation
    {
        Invalid = -1,
        HeatingMode, // normal heating coil operation
        CoolingMode, // normal cooling coil operation
        NeutralMode, // signal coil shouldn't run
        Num
    };

    struct OAEquipList
    {
        // Members
        // Equipment List Data
        std::string ComponentName;
        CompType Type;      // Parameterized Component Types this module can address
        int ComponentIndex; // Which one in list -- updated by routines called from here
        HVACSystemData *compPointer = nullptr;
        int CoilAirInletNode;
        int CoilAirOutletNode;
        int CoilWaterInletNode;
        int CoilWaterOutletNode;
        DataPlant::PlantEquipmentType CoilType;
        PlantLocation plantLoc;
        int FluidIndex = 0;
        Real64 MaxVolWaterFlow;
        Real64 MaxWaterMassFlow;
        Real64 MinVolWaterFlow;
        Real64 MinWaterMassFlow;
        bool FirstPass;
        // End Of Equipment list data

        // Default Constructor
        OAEquipList()
            : Type(CompType::Invalid), ComponentIndex(0), CoilAirInletNode(0), CoilAirOutletNode(0), CoilWaterInletNode(0), CoilWaterOutletNode(0),
              CoilType(DataPlant::PlantEquipmentType::Invalid), plantLoc{}, MaxVolWaterFlow(0.0), MaxWaterMassFlow(0.0), MinVolWaterFlow(0.0),
              MinWaterMassFlow(0.0), FirstPass(true)
        {
        }
    };

    struct OAUnitData
    {
        // Members
        // Input data
        std::string Name;                      // name of unit
        Sched::Schedule *availSched = nullptr; // availability
        std::string ZoneName;                  // Name of zone the system is serving
        int ZonePtr;                           // Point to this zone in the Zone derived type
        int ZoneNodeNum;                       // index of zone air node in node structure
        std::string UnitControlType;           // Control type for the system
        // (Neutral and setpoint temperatrue)
        OAUnitCtrlType controlType;                  // Unit Control type indicator
        int AirInletNode;                            // inlet air node number
        int AirOutletNode;                           // outlet air node number
        std::string SFanName;                        // name of supply fan
        int SFan_Index;                              // index in fan structure
        HVAC::FanType supFanType;                    // type of fan in cFanTypes
        Sched::Schedule *supFanAvailSched = nullptr; // supply fan availability sched from fan object
        HVAC::FanPlace supFanPlace;                  // fan placement; blow through and draw through
        Real64 FanCorTemp;                           // correction temperature
        bool FanEffect;                              // .TRUE. if unit has a fan type of draw through
        int SFanOutletNode;                          // supply fan outlet node number
        std::string ExtFanName;                      // name of exhaust fan
        int ExtFan_Index;                            // index in fan structure
        HVAC::FanType extFanType;                    // type of fan in cFanTypes
        Sched::Schedule *extFanAvailSched = nullptr; // exhaust fan availability sched from fan object
        bool ExtFan;                                 // true if there is an exhaust fan
        Sched::Schedule *outAirSched = nullptr;      // schedule of fraction for outside air (all controls)
        int OutsideAirNode;                          // outside air node number
        Real64 OutAirVolFlow;                        // m3/s
        Real64 OutAirMassFlow;                       // kg/s
        Real64 ExtAirVolFlow;                        // m3/s
        Real64 ExtAirMassFlow;                       // kg/s
        Sched::Schedule *extAirSched = nullptr;      // schedule of fraction for exhaust air
        Real64 SMaxAirMassFlow;                      // kg/s
        Real64 EMaxAirMassFlow;                      // kg/s
        Real64 SFanMaxAirVolFlow;                    // m3/s
        Real64 EFanMaxAirVolFlow;                    // m3/s
        Sched::Schedule *hiCtrlTempSched = nullptr;  // Schedule name for the High Control Air temperature
        Sched::Schedule *loCtrlTempSched = nullptr;  // Schedule name for the Low Control Air temperature
        Operation OperatingMode;                     // operating condition( NeutralMode, HeatingMode, CoolingMode)
        int ControlCompTypeNum;
        int CompErrIndex;
        Real64 AirMassFlow; // kg/s
        bool FlowError;     // flow error flag
        int NumComponents;
        std::string ComponentListName;
        Real64 CompOutSetTemp; // component outlet setpoint temperature
        Avail::Status availStatus = Avail::Status::NoAction;
        std::string AvailManagerListName; // Name of an availability manager list object
        Array1D<OAEquipList> OAEquip;
        // Report data
        Real64 TotCoolingRate;    // Rate of total cooling delivered to the zone [W]
        Real64 TotCoolingEnergy;  // Total cooling energy delivered by the OAU supply air to the zone [J]
        Real64 SensCoolingRate;   // Rate of sensible cooling delivered to the zone [W]
        Real64 SensCoolingEnergy; // Sensible cooling energy delivered by the OAU supply air to the zone [J]
        Real64 LatCoolingRate;    // Rate of latent cooling delivered to the zone [W]
        Real64 LatCoolingEnergy;  // Latent cooling energy delivered by the OAU supply air to the zone [J]
        Real64 ElecFanRate;       // Total electric use rate (power) for supply/exhaust fans [W]
        Real64 ElecFanEnergy;     // Electric energy use for supply fan and exhaust fan [J]
        Real64 SensHeatingEnergy; // sensible heating energy delivered by the ERV supply air to the zone [J]
        Real64 SensHeatingRate;   // rate of sensible heating delivered to the zone [W]
        Real64 LatHeatingEnergy;  // latent heating energy delivered by the ERV supply air to the zone [J]
        Real64 LatHeatingRate;    // rate of latent heating delivered to the zone [W]
        Real64 TotHeatingEnergy;  // total heating energy delivered by the ERV supply air to the zone [J]
        Real64 TotHeatingRate;    // rate of total heating delivered to the zone [W]
        bool FirstPass;           // detects first time through for resetting sizing data

        // Default Constructor
        OAUnitData()
            : ZonePtr(0), ZoneNodeNum(0), controlType(OAUnitCtrlType::Invalid), AirInletNode(0), AirOutletNode(0), SFan_Index(0),
              supFanType(HVAC::FanType::Invalid), supFanPlace(HVAC::FanPlace::Invalid), FanCorTemp(0.0), FanEffect(false), SFanOutletNode(0),
              ExtFan_Index(0), extFanType(HVAC::FanType::Invalid), ExtFan(false), OutsideAirNode(0), OutAirVolFlow(0.0), OutAirMassFlow(0.0),
              ExtAirVolFlow(0.0), ExtAirMassFlow(0.0), SMaxAirMassFlow(0.0), EMaxAirMassFlow(0.0), SFanMaxAirVolFlow(0.0), EFanMaxAirVolFlow(0.0),
              OperatingMode(Operation::Invalid), ControlCompTypeNum(0), CompErrIndex(0), AirMassFlow(0.0), FlowError(false), NumComponents(0),
              CompOutSetTemp(0.0), TotCoolingRate(0.0), TotCoolingEnergy(0.0), SensCoolingRate(0.0), SensCoolingEnergy(0.0), LatCoolingRate(0.0),
              LatCoolingEnergy(0.0), ElecFanRate(0.0), ElecFanEnergy(0.0), SensHeatingEnergy(0.0), SensHeatingRate(0.0), LatHeatingEnergy(0.0),
              LatHeatingRate(0.0), TotHeatingEnergy(0.0), TotHeatingRate(0.0), FirstPass(true)
        {
        }
    };

    void SimOutdoorAirUnit(EnergyPlusData &state,
                           std::string_view CompName, // name of the outdoor air unit
                           int ZoneNum,               // number of zone being served
                           bool FirstHVACIteration,   // TRUE if 1st HVAC simulation of system timestep
                           Real64 &PowerMet,          // Sensible power supplied (W)
                           Real64 &LatOutputProvided, // Latent add/removal supplied by window AC (kg/s), dehumid = negative
                           int &CompIndex);

    void GetOutdoorAirUnitInputs(EnergyPlusData &state);

    void InitOutdoorAirUnit(EnergyPlusData &state,
                            int OAUnitNum,          // index for the current outdoor air unit
                            int ZoneNum,            // number of zone being served
                            bool FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    );

    void SizeOutdoorAirUnit(EnergyPlusData &state, int OAUnitNum);

    void CalcOutdoorAirUnit(EnergyPlusData &state,
                            int &OAUnitNum,           // number of the current unit being simulated
                            int ZoneNum,              // number of zone being served
                            bool FirstHVACIteration,  // TRUE if 1st HVAC simulation of system timestep
                            Real64 &PowerMet,         // power supplied
                            Real64 &LatOutputProvided // Latent power supplied (kg/s), negative = dehumidification
    );

    void SimZoneOutAirUnitComps(EnergyPlusData &state, int OAUnitNum, bool FirstHVACIteration);

    void SimOutdoorAirEquipComps(EnergyPlusData &state,
                                 int OAUnitNum,                // actual outdoor air unit num
                                 std::string_view EquipType,   // the component type
                                 std::string const &EquipName, // the component Name
                                 int EquipNum,
                                 CompType CompTypeNum, // Component Type -- Integerized for this module
                                 bool FirstHVACIteration,
                                 int &CompIndex,
                                 bool Sim // if TRUE, simulate component
    );

    void CalcOAUnitCoilComps(EnergyPlusData &state,
                             int CompNum, // actual outdoor air unit num
                             bool FirstHVACIteration,
                             int EquipIndex, // Component Type -- Integerized for this module
                             Real64 &LoadMet);

    void ReportOutdoorAirUnit(EnergyPlusData &state,
                              int OAUnitNum); // Index for the outdoor air unit under consideration within the derived types

    int GetOutdoorAirUnitOutAirNode(EnergyPlusData &state, int OAUnitNum);

    int GetOutdoorAirUnitZoneInletNode(EnergyPlusData &state, int OAUnitNum);

    int GetOutdoorAirUnitReturnAirNode(EnergyPlusData &state, int OAUnitNum);

    int getOutdoorAirUnitEqIndex(EnergyPlusData &state, std::string_view EquipName);
} // namespace OutdoorAirUnit

struct OutdoorAirUnitData : BaseGlobalStruct
{
    int NumOfOAUnits = 0;        // Number of outdoor air unit in the input file
    Real64 OAMassFlowRate = 0.0; // Outside air mass flow rate for the zone outdoor air unit
    Array1D_bool MyOneTimeErrorFlag;
    bool GetOutdoorAirUnitInputFlag = true; // Flag set to make sure you get input once
    Array1D_bool MySizeFlag;
    Array1D_bool CheckEquipName;
    Array1D<OutdoorAirUnit::OAUnitData> OutAirUnit;
    bool MyOneTimeFlag = true;
    bool ZoneEquipmentListChecked = false;
    std::unordered_set<std::string> SupplyFanUniqueNames;
    std::unordered_set<std::string> ExhaustFanUniqueNames;
    std::unordered_set<std::string> ComponentListUniqueNames;
    Array1D_bool MyEnvrnFlag;
    Array1D_bool MyPlantScanFlag;
    Array1D_bool MyZoneEqFlag; // used to set up zone equipment availability managers
    bool HeatActive = false;
    bool CoolActive = false;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) OutdoorAirUnitData();
    }
};

} // namespace EnergyPlus

// namespace EnergyPlus

#endif
