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

#ifndef UnitHeater_hh_INCLUDED
#define UnitHeater_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace UnitHeater {

    struct UnitHeaterData
    {
        // Members
        // Input data
        std::string Name;                      // name of unit
        Sched::Schedule *availSched = nullptr; // availability schedule
        int AirInNode;                         // inlet air node number
        int AirOutNode;                        // outlet air node number
        HVAC::FanType fanType;                 // Fan type number (see DataHVACGlobals)
        std::string FanName;                   // name of fan
        int Fan_Index;
        Sched::Schedule *fanOpModeSched = nullptr; // fan operating mode schedule
        Sched::Schedule *fanAvailSched = nullptr;  // fan availability schedule
        int ControlCompTypeNum;
        int CompErrIndex;
        Real64 MaxAirVolFlow;                   // m3/s
        Real64 MaxAirMassFlow;                  // kg/s
        std::string FanOperatesDuringNoHeating; // Indicates whether fan operates or not during no heating
        int FanOutletNode;                      // outlet node number for fan exit
        // (assumes fan is upstream of heating coil)
        HVAC::FanOp fanOp = HVAC::FanOp::Invalid;              // mode of operation; 1=cycling fan, cycling coil, 2=continuous fan, cycling coil
        HVAC::CoilType heatCoilType = HVAC::CoilType::Invalid; // type of heating coil (water, gas, electric, etc.)
        std::string HCoilTypeCh;                               // actual object name
        std::string HCoilName;                                 // name of heating coil
        int HCoil_Index;
        DataPlant::PlantEquipmentType HeatingCoilType;
        Fluid::RefrigProps *HCoil_fluid = nullptr;
        Real64 MaxVolHotWaterFlow; // m3/s
        Real64 MaxVolHotSteamFlow; // m3/s
        Real64 MaxHotWaterFlow;    // kg/s
        Real64 MaxHotSteamFlow;    // m3/s
        Real64 MinVolHotWaterFlow; // m3/s
        Real64 MinVolHotSteamFlow; // m3/s
        Real64 MinHotWaterFlow;    // kg/s
        Real64 MinHotSteamFlow;    // kg/s
        int HotControlNode;        // hot water control node, inlet of coil
        Real64 HotControlOffset;   // control tolerance
        int HotCoilOutNodeNum;     // outlet of coil
        PlantLocation HWplantLoc;  // Location of plant component for hot plant coil
        Real64 PartLoadFrac;       // part load fraction for the unit
        // Report data
        Real64 HeatPower;  // unit heating output in watts
        Real64 HeatEnergy; // unit heating output in J
        Real64 ElecPower;
        Real64 ElecEnergy;
        std::string AvailManagerListName; // Name of an availability manager list object
        Avail::Status availStatus = Avail::Status::NoAction;
        bool FanOffNoHeating;    // True when fan is on during no heating load
        Real64 FanPartLoadRatio; // fan part-load ratio for time step
        int ZonePtr;             // pointer to a zone served by a unit heater
        int HVACSizingIndex;     // index of a HVACSizing object for a unit heater
        bool FirstPass;          // detects first time through for resetting sizing data

        General::SolveRootStats solveRootStats;

        // Default Constructor
        UnitHeaterData()
            : AirInNode(0), AirOutNode(0), fanType(HVAC::FanType::Invalid), Fan_Index(0), ControlCompTypeNum(0), CompErrIndex(0), MaxAirVolFlow(0.0),
              MaxAirMassFlow(0.0), FanOutletNode(0), HCoil_Index(0), HeatingCoilType(DataPlant::PlantEquipmentType::Invalid), MaxVolHotWaterFlow(0.0),
              MaxVolHotSteamFlow(0.0), MaxHotWaterFlow(0.0), MaxHotSteamFlow(0.0), MinVolHotWaterFlow(0.0), MinVolHotSteamFlow(0.0),
              MinHotWaterFlow(0.0), MinHotSteamFlow(0.0), HotControlNode(0), HotControlOffset(0.0), HotCoilOutNodeNum(0), HWplantLoc{},
              PartLoadFrac(0.0), HeatPower(0.0), HeatEnergy(0.0), ElecPower(0.0), ElecEnergy(0.0), FanOffNoHeating(false), FanPartLoadRatio(0.0),
              ZonePtr(0), HVACSizingIndex(0), FirstPass(true)
        {
        }
    };

    struct UnitHeatNumericFieldData
    {
        // Members
        Array1D_string FieldNames;

        // Default Constructor
        UnitHeatNumericFieldData()
        {
        }
    };

    void SimUnitHeater(EnergyPlusData &state,
                       std::string_view CompName,     // name of the fan coil unit
                       int const ZoneNum,             // number of zone being served
                       bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                       Real64 &PowerMet,              // Sensible power supplied (W)
                       Real64 &LatOutputProvided,     // Latent add/removal supplied by window AC (kg/s), dehumid = negative
                       int &CompIndex);

    void GetUnitHeaterInput(EnergyPlusData &state);

    void InitUnitHeater(EnergyPlusData &state,
                        int const UnitHeatNum,        // index for the current unit heater
                        int const ZoneNum,            // number of zone being served
                        bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    );

    void SizeUnitHeater(EnergyPlusData &state, int const UnitHeatNum);

    void CalcUnitHeater(EnergyPlusData &state,
                        int &UnitHeatNum,              // number of the current fan coil unit being simulated
                        int const ZoneNum,             // number of zone being served
                        bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                        Real64 &PowerMet,              // Sensible power supplied (W)
                        Real64 &LatOutputProvided      // Latent power supplied (kg/s), negative = dehumidification
    );

    void CalcUnitHeaterComponents(EnergyPlusData &state,
                                  int const UnitHeatNum,                             // Unit index in unit heater array
                                  bool const FirstHVACIteration,                     // flag for 1st HVAV iteration in the time step
                                  Real64 &LoadMet,                                   // load met by unit (watts)
                                  HVAC::FanOp const fanOp = HVAC::FanOp::Continuous, // fan operating mode
                                  Real64 const PartLoadRatio = 1.0                   // part-load ratio
    );

    // SUBROUTINE UpdateUnitHeater

    // No update routine needed in this module since all of the updates happen on
    // the Node derived type directly and these updates are done by other routines.

    // END SUBROUTINE UpdateUnitHeater

    void ReportUnitHeater(EnergyPlusData &state, int const UnitHeatNum); // Unit index in unit heater array

    int getUnitHeaterIndex(EnergyPlusData &state, std::string_view CompName);
} // namespace UnitHeater

struct UnitHeatersData : BaseGlobalStruct
{

    // MODULE PARAMETER DEFINITIONS
    std::string const cMO_UnitHeater = "ZoneHVAC:UnitHeater";

    // Character parameters for outside air control types:

    bool HCoilOn;       // TRUE if the heating coil (gas or electric especially) should be running
    int NumOfUnitHeats; // Number of unit heaters in the input file
    Real64 QZnReq;      // heating or cooling needed by zone [watts]
    Array1D_bool MySizeFlag;
    Array1D_bool CheckEquipName;

    bool InitUnitHeaterOneTimeFlag = true;
    bool GetUnitHeaterInputFlag = true;
    bool ZoneEquipmentListChecked = false; // True after the Zone Equipment List has been checked for items
    bool SetMassFlowRateToZero = false;    // TRUE when mass flow rates need to be set to zero

    // Object Data
    EPVector<UnitHeater::UnitHeaterData> UnitHeat;
    EPVector<UnitHeater::UnitHeatNumericFieldData> UnitHeatNumericFields;

    Array1D_bool MyEnvrnFlag;
    Array1D_bool MyPlantScanFlag;
    Array1D_bool MyZoneEqFlag; // used to set up zone equipment availability managers

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->HCoilOn = false;
        this->NumOfUnitHeats = 0;
        this->QZnReq = 0.0;
        this->MySizeFlag.deallocate();
        this->CheckEquipName.deallocate();
        this->UnitHeat.deallocate();
        this->UnitHeatNumericFields.deallocate();
        this->InitUnitHeaterOneTimeFlag = true;
        this->GetUnitHeaterInputFlag = true;
        this->ZoneEquipmentListChecked = false;
        this->SetMassFlowRateToZero = false;
        this->MyEnvrnFlag.deallocate();
        this->MyPlantScanFlag.deallocate();
        this->MyZoneEqFlag.deallocate();
    }

    // Default Constructor
    UnitHeatersData() = default;
};

} // namespace EnergyPlus

#endif
