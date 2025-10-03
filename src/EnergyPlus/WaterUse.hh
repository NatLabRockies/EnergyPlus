// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
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

#ifndef WaterUse_hh_INCLUDED
#define WaterUse_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace WaterUse {

    enum class HeatRecovHX
    {
        Invalid = -1,
        Ideal,
        CounterFlow,
        CrossFlow,
        Num
    };

    enum class HeatRecovConfig
    {
        Invalid = -1,
        Plant,
        Equipment,
        PlantAndEquip,
        Num
    };

    struct WaterEquipmentType
    {
        std::string Name; // Name of DHW
        std::string EndUseSubcatName;
        int Connections = 0;                          // Index for WATER USE CONNECTIONS object
        Real64 PeakVolFlowRate = 0.0;                 // Peak volumetric flow rate, also water consumption rate (m3/s)
        Sched::Schedule *flowRateFracSched = nullptr; // pointer to schedule object // Now it is
        Real64 ColdVolFlowRate = 0.0;
        Real64 HotVolFlowRate = 0.0;
        Real64 TotalVolFlowRate = 0.0; // Volumetric flow rate, also water consumption rate (m3/s)
        Real64 ColdMassFlowRate = 0.0;
        Real64 HotMassFlowRate = 0.0;
        Real64 TotalMassFlowRate = 0.0; // Mass flow rate (kg/s)
        Real64 DrainMassFlowRate = 0.0;
        Sched::Schedule *coldTempSched = nullptr;   // schedule object
        Sched::Schedule *hotTempSched = nullptr;    // schedule object
        Sched::Schedule *targetTempSched = nullptr; // schedule object
        Real64 ColdTemp = 0.0;                      // Cold supply water temperature (C)
        Real64 HotTemp = 0.0;                       // Hot supply water temperature (C)
        Real64 TargetTemp = 0.0;                    // Target (mixed) water temperature (C)
        Real64 MixedTemp = 0.0;                     // Actual outlet (mixed) water temperature (C)
        Real64 DrainTemp = 0.0;
        int CWHWTempErrorCount = 0;                   // - counter if hot water temp is less than cold water temp
        int CWHWTempErrIndex = 0;                     // - index to recurring error structure for hot water temp
        int TargetHWTempErrorCount = 0;               // - counter for target water temp error
        int TargetHWTempErrIndex = 0;                 // - index to recurring error structure for target water temp
        int TargetCWTempErrorCount = 0;               // - counter for target water temp error
        int TargetCWTempErrIndex = 0;                 // - index to recurring error structure for target water temp
        int Zone = 0;                                 // Index for zone object
        Sched::Schedule *sensibleFracSched = nullptr; // schedule object
        Real64 SensibleRate = 0.0;
        Real64 SensibleEnergy = 0.0;
        Real64 SensibleRateNoMultiplier = 0.0;
        Sched::Schedule *latentFracSched = nullptr; // schedule object
        Real64 LatentRate = 0.0;
        Real64 LatentEnergy = 0.0;
        Real64 LatentRateNoMultiplier = 0.0;
        Real64 MoistureRate = 0.0;
        Real64 MoistureMass = 0.0;
        Real64 ColdVolume = 0.0;  // Water consumption (m3)
        Real64 HotVolume = 0.0;   // Water consumption (m3)
        Real64 TotalVolume = 0.0; // Water consumption (m3)
        Real64 Power = 0.0;       // Heating rate required to meet the mixed water temperature (W)
        Real64 Energy = 0.0;      // Heating energy required to meet the mixed water temperature (J)
        bool setupMyOutputVars = true;
        bool allowHotControl = false; // only certain configurations will use hot water flow and/or hot water temp schedule

        // Reset Some Values to Zeros
        void reset()
        {
            SensibleRate = 0.0;
            SensibleEnergy = 0.0;
            LatentRate = 0.0;
            LatentEnergy = 0.0;
            MixedTemp = 0.0;
            TotalMassFlowRate = 0.0;
            DrainTemp = 0.0;
        }

        void CalcEquipmentFlowRates(EnergyPlusData &state);

        void CalcEquipmentDrainTemp(EnergyPlusData &state);

        void setupOutputVars(EnergyPlusData &state);

        void FillPredefinedTable(EnergyPlusData &state);
    };

    struct WaterConnectionsType : PlantComponent
    {
        std::string Name;        // Name of DHW
        bool Init = true;        // Flag for initialization:  TRUE means do the init
        bool StandAlone = false; // Flag for operation with no plant connections
        int InletNode = 0;       // Hot water demand node
        int OutletNode = 0;      // Cold water supply node
        int SupplyTankNum = 0;
        int RecoveryTankNum = 0;
        int TankDemandID = 0; // array to request flow from supply tank
        int TankSupplyID = 0; // array to send flow to recovery tank
        bool HeatRecovery = false;
        HeatRecovHX HeatRecoveryHX = HeatRecovHX::Ideal;
        HeatRecovConfig HeatRecoveryConfig = HeatRecovConfig::Plant;
        Real64 HXUA = 0.0;
        Real64 Effectiveness = 0.0;
        Real64 RecoveryRate = 0.0;
        Real64 RecoveryEnergy = 0.0;
        Real64 TankMassFlowRate = 0.0;  // Mass flow rate (kg/s)
        Real64 ColdMassFlowRate = 0.0;  // Mass flow rate (kg/s)  cold = mains + tank
        Real64 HotMassFlowRate = 0.0;   // Mass flow rate (kg/s)
        Real64 TotalMassFlowRate = 0.0; // Mass flow rate (kg/s) total = cold + hot
        Real64 DrainMassFlowRate = 0.0;
        Real64 RecoveryMassFlowRate = 0.0;
        Real64 PeakVolFlowRate = 0.0;  // Volumetric flow rate, also water consumption rate (m3/s)
        Real64 TankVolFlowRate = 0.0;  // Volumetric flow rate, also water consumption rate (m3/s)
        Real64 ColdVolFlowRate = 0.0;  // Volumetric flow rate, also water consumption rate (m3/s)
        Real64 HotVolFlowRate = 0.0;   // Volumetric flow rate, also water consumption rate (m3/s)
        Real64 TotalVolFlowRate = 0.0; // Volumetric flow rate, also water consumption rate (m3/s)
        Real64 DrainVolFlowRate = 0.0;
        Real64 PeakMassFlowRate = 0.0;            // Peak Mass flow rate for MassFlowRateMax
        Sched::Schedule *coldTempSched = nullptr; // schedule object
        Sched::Schedule *hotTempSched = nullptr;  // schedule object
        Real64 TankTemp = 0.0;                    // Cold supply water temperature (C)
        Real64 ColdSupplyTemp = 0.0;              // cold from mains, schedule, or tank, depending
        Real64 ColdTemp = 0.0;                    // Cold supply water temperature (C)  actual cold (could be reheated)
        Real64 HotTemp = 0.0;                     // Hot supply water temperature (C)
        Real64 DrainTemp = 0.0;
        Real64 RecoveryTemp = 0.0;
        Real64 ReturnTemp = 0.0;
        Real64 WasteTemp = 0.0;
        Real64 TempError = 0.0;
        Real64 TankVolume = 0.0;  // Water consumption (m3)
        Real64 ColdVolume = 0.0;  // Water consumption (m3)
        Real64 HotVolume = 0.0;   // Water consumption (m3)
        Real64 TotalVolume = 0.0; // Water consumption (m3)
        Real64 Power = 0.0;       // Heating rate required to raise temperature from cold to hot (W)
        Real64 Energy = 0.0;      // Heating energy required to raise temperature from cold to hot (J)
        int NumWaterEquipment = 0;
        int MaxIterationsErrorIndex = 0; // recurring error index
        Array1D_int myWaterEquipArr;
        PlantLocation plantLoc{};
        bool MyEnvrnFlag = true;

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void InitConnections(EnergyPlusData &state);

        void CalcConnectionsFlowRates(EnergyPlusData &state, bool FirstHVACIteration);

        void CalcConnectionsDrainTemp(EnergyPlusData &state);

        void CalcConnectionsHeatRecovery(EnergyPlusData &state);

        void UpdateWaterConnections(EnergyPlusData &state);

        void ReportWaterUse(EnergyPlusData &state);

        void setupOutputVars([[maybe_unused]] EnergyPlusData &state);

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;

        void FillPredefinedTable(EnergyPlusData &state);
    };

    void SimulateWaterUse(EnergyPlusData &state, bool FirstHVACIteration);

    void GetWaterUseInput(EnergyPlusData &state);

    void ReportStandAloneWaterUse(EnergyPlusData &state);

    void CalcWaterUseZoneGains(EnergyPlusData &state);

    Real64 calcH2ODensity(EnergyPlusData &state);

} // namespace WaterUse

struct WaterUseData : BaseGlobalStruct
{

    int numWaterEquipment = 0;
    int numWaterConnections = 0;
    bool getWaterUseInputFlag = true;
    bool calcRhoH2O = true;
    bool MyEnvrnFlagLocal = true;
    Real64 rhoH2OStd = 1000.0;
    Array1D_bool CheckEquipName;
    EPVector<WaterUse::WaterEquipmentType> WaterEquipment;
    EPVector<WaterUse::WaterConnectionsType> WaterConnections;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) WaterUseData();
    }
};

} // namespace EnergyPlus

#endif
