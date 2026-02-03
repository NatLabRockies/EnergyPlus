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

#ifndef Humidifiers_hh_INCLUDED
#define Humidifiers_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Humidifiers {

    static constexpr std::array<std::string_view, 2> HumidifierType = {"Humidifier:Steam:Electric", "Humidifier:Steam:Gas"};

    enum class HumidType
    {
        Invalid = -1,
        Electric,
        Gas,
        Num
    };

    enum class InletWaterTemp
    {
        Invalid = -1,
        Fixed,
        Variable,
        Num
    };
    static constexpr std::array<std::string_view, static_cast<int>(InletWaterTemp::Num)> inletWaterTempsUC = {"FIXEDINLETWATERTEMPERATURE",
                                                                                                              "VARIABLEINLETWATERTEMPERATURE"};

    class HumidifierData
    {
    private:
    public:
        // Members
        std::string Name;                      // unique name of component
        HumidType HumType;                     // Pointer to Humidifier in list of humidifiers
        int EquipIndex;                        // Pointer to Humidifier in list of humidifiers
        Sched::Schedule *availSched = nullptr; // availability schedule
        Real64 NomCapVol;                      // nominal capacity [m3/s of water]
        Real64 NomCap;                         // nominal capacity [kg/s of water]
        Real64 NomPower;                       // power consumption at full output [watts]
        Real64 ThermalEffRated;                // rated thermal efficiency of the gas fired humidifier [-]
        Real64 CurMakeupWaterTemp;             // makeup water temperature from main water [C]
        int EfficiencyCurvePtr;                // index to efficiency curve
        InletWaterTemp InletWaterTempOption;   // type inlet water temperature fixed or variable
        Real64 FanPower;                       // nominal fan power [watts]
        Real64 StandbyPower;                   // standby power consumption [watts]
        int AirInNode;                         // air inlet node of humidifier
        int AirOutNode;                        // air outlet node of humidifier
        Real64 AirInTemp;                      // inlet air temperature [C]
        Real64 AirInHumRat;                    // inlet air humidity ratio [kg water / kg air]
        Real64 AirInEnthalpy;                  // inlet air specific enthalpy [J/kg]
        Real64 AirInMassFlowRate;              // inlet air mass flow rate [kg/s]
        Real64 AirOutTemp;                     // outlet air temperature [C]
        Real64 AirOutHumRat;                   // outlet air humidity ratio [kg water / kg air]
        Real64 AirOutEnthalpy;                 // outlet air specific enthalpy [J/kg]
        Real64 AirOutMassFlowRate;             // outlet air mass flow rate [kg/s]
        Real64 HumRatSet;                      // humidity ratio setpoint [kg water / kg air]
        Real64 WaterAdd;                       // water output (and consumption) [kg/s]
        Real64 ElecUseEnergy;                  // electricity consumption [J]
        Real64 ElecUseRate;                    // electricity consumption [W]
        Real64 WaterCons;                      // water consumption in cubic meters
        Real64 WaterConsRate;                  // water consumption rate in m3/s
        bool SuppliedByWaterSystem;            // true means there is storage tank, otherwise mains
        int WaterTankID;                       // index pointer to water storage tank
        int WaterTankDemandARRID;              // index pointer to WaterStorage Demand arrays.
        Real64 TankSupplyVdot;
        Real64 TankSupplyVol;
        Real64 StarvedSupplyVdot;
        Real64 StarvedSupplyVol;
        int TankSupplyID; // index pointer to WaterStorage supply arrays.
        bool MySizeFlag;
        bool MyEnvrnFlag;
        bool MySetPointCheckFlag;
        // report variables for gas humidifier
        Real64 ThermalEff;       // current actual thermal efficiency gas humidifier [-]
        Real64 GasUseRate;       // gas consumption rate [W]
        Real64 GasUseEnergy;     // gas energy consumption [J]
        Real64 AuxElecUseRate;   // auxiliary electric power input [W]
        Real64 AuxElecUseEnergy; //  auxiliary electric energy consumption [J]'

        // Default Constructor
        HumidifierData()
            : HumType(HumidType::Invalid), EquipIndex(0), NomCapVol(0.0), NomCap(0.0), NomPower(0.0), ThermalEffRated(1.0), CurMakeupWaterTemp(0.0),
              EfficiencyCurvePtr(0), InletWaterTempOption(InletWaterTemp::Invalid), FanPower(0.0), StandbyPower(0.0), AirInNode(0), AirOutNode(0),
              AirInTemp(0.0), AirInHumRat(0.0), AirInEnthalpy(0.0), AirInMassFlowRate(0.0), AirOutTemp(0.0), AirOutHumRat(0.0), AirOutEnthalpy(0.0),
              AirOutMassFlowRate(0.0), HumRatSet(0.0), WaterAdd(0.0), ElecUseEnergy(0.0), ElecUseRate(0.0), WaterCons(0.0), WaterConsRate(0.0),
              SuppliedByWaterSystem(false), WaterTankID(0), WaterTankDemandARRID(0), TankSupplyVdot(0.0), TankSupplyVol(0.0), StarvedSupplyVdot(0.0),
              StarvedSupplyVol(0.0), TankSupplyID(0), MySizeFlag(true), MyEnvrnFlag(true), MySetPointCheckFlag(true), ThermalEff(0.0),
              GasUseRate(0.0), GasUseEnergy(0.0), AuxElecUseRate(0.0), AuxElecUseEnergy(0.0)
        {
        }

        void InitHumidifier(EnergyPlusData &state); // number of the current humidifier being simulated

        void SizeHumidifier(EnergyPlusData &state); // number of the current humidifier being sized

        void ControlHumidifier(EnergyPlusData &state,
                               Real64 &WaterAddNeeded // moisture addition rate needed to meet minimum humidity ratio setpoint [kg/s]
        );

        void CalcElecSteamHumidifier(EnergyPlusData &state, Real64 const WaterAddNeeded // moisture addition rate set by controller [kg/s]
        );

        void CalcGasSteamHumidifier(EnergyPlusData &state, Real64 const WaterAddNeeded // moisture addition rate set by controller [kg/s]
        );

        void UpdateReportWaterSystem(EnergyPlusData &state); // number of the current humidifier being simulated

        void UpdateHumidifier(EnergyPlusData &state); // number of the current humidifier being simulated

        void ReportHumidifier(EnergyPlusData &state); // number of the current humidifier being simulated
    };

    void SimHumidifier(EnergyPlusData &state,
                       std::string_view CompName,     // name of the humidifier unit
                       bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                       int &CompIndex                 // Pointer to Humidifier Unit
    );

    void GetHumidifierInput(EnergyPlusData &state);

    int GetAirInletNodeNum(EnergyPlusData &state, std::string const &HumidifierName, bool &ErrorsFound);

    int GetAirOutletNodeNum(EnergyPlusData &state, std::string const &HumidifierName, bool &ErrorsFound);

} // namespace Humidifiers

struct HumidifiersData : BaseGlobalStruct
{
    int NumHumidifiers = 0;   // number of humidifiers of all types
    int NumElecSteamHums = 0; // number of electric steam humidifiers
    int NumGasSteamHums = 0;  // number of electric steam humidifiers
    Array1D_bool CheckEquipName;
    bool GetInputFlag = true; // moved up from a static function variable

    // Object Data
    Array1D<Humidifiers::HumidifierData> Humidifier;
    std::unordered_map<std::string, std::string> HumidifierUniqueNames;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumHumidifiers = 0;
        this->NumElecSteamHums = 0;
        this->NumGasSteamHums = 0;
        this->CheckEquipName.clear();
        this->GetInputFlag = true;
        this->Humidifier.clear();
        this->HumidifierUniqueNames.clear();
    }
};

} // namespace EnergyPlus

#endif
