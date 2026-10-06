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

#ifndef LiquidCooledITEPlantComponents_hh_INCLUDED
#define LiquidCooledITEPlantComponents_hh_INCLUDED

// C++ headers
#include <string>
#include <vector>

// EnergyPlus headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

struct EnergyPlusData;

namespace LiquidCooledITEPlantComponents {

    enum class ThermalResistanceMethod
    {
        Invalid = -1,
        Standard,
        LMTD,
        Num
    };

    struct CoilCoolingITEColdPlateData : public EnergyPlus::PlantComponent
    {
        // --- Identity / location ---
        std::string name;
        PlantLocation plantLoc;

        // --- Input parameters ---
        Sched::Schedule *availabilitySchedule = nullptr;
        Sched::Schedule *itLoadSchedule = nullptr; // optional additive IT load [W]
        ThermalResistanceMethod thermalResistanceMethod = ThermalResistanceMethod::Standard;
        DataPlant::FlowMode flowMode = DataPlant::FlowMode::Constant;
        Real64 thermalResistance = 0.0;              // nominal cold-plate thermal resistance [K/W]
        Real64 maximumCaseTemperature = 0.0;         // maximum allowable chip case temperature [C]
        Real64 targetCaseOperatingTemperature = 0.0; // target chip case temperature used for sizing and variable-flow control [C]
        Real64 nominalFlowRate = 0.0;                // nominal volumetric coolant flow rate [m3/s]
        Real64 maximumFlowRate = 0.0;                // maximum volumetric coolant flow rate [m3/s]
        Real64 auxPower = 0.0;                       // rated auxiliary electric power [W]
        std::string endUseSubcategory = "General";   // ABUPS end-use sub-category label
        int inletNode = 0;
        int outletNode = 0;
        int thermalResistanceModifierCurveIndex = 0;

        bool myPlantScanFlag = true;           // false once ScanPlantLoopsForObject has run
        bool mySizingFlag = true;              // false once sizeColdPlate has finalized
        bool myEnvrnFlag = true;               // reset to true each BeginEnvrnFlag cycle
        Real64 loadFromITEquipment = 0.0;      // IT load set by the paired ElectricEquipment:ITE:LiquidCooled object [W]
        Real64 heatRemovedByFluid = 0.0;       // heat actually transferred to the coolant [W]
        Real64 heatRemovedByFluidEnergy = 0.0; // [J]
        Real64 zoneHeatGainRate = 0.0;         // unmet load returned to zone heat balance [W]
        Real64 zoneHeatGainEnergy = 0.0;       // [J]
        Real64 caseTemperature = 0.0;          // computed chip case temperature [C]
        Real64 inletTemp = 0.0;                // coolant inlet temperature [C]
        Real64 outletTemp = 0.0;               // coolant outlet temperature [C]
        Real64 massFlowRate = 0.0;             // coolant mass flow rate [kg/s]
        Real64 auxElecPower = 0.0;             // auxiliary electric power (0 when unavailable) [W]
        Real64 auxElecEnergy = 0.0;            // [J]

        virtual ~CoilCoolingITEColdPlateData() = default;
        CoilCoolingITEColdPlateData() = default;

        void
        simulate(EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag) override;
        void onInitLoopEquip(EnergyPlusData &state, const PlantLocation &calledFromLocation) override;
        void oneTimeInit(EnergyPlusData &state) override;
        void sizeColdPlate(EnergyPlusData &state);
        void doPhysics(EnergyPlusData &state);
        void report(EnergyPlusData &state);
        static void processInputForCoilCoolingITEColdPlate(EnergyPlusData &state);
        static PlantComponent *factory(EnergyPlusData &state, const std::string &objectName);
        Real64 getDesignLoad(EnergyPlusData &state, Real64 inletFluidTemperature, Real64 outletFluidTemperature);
        Real64 getThermalResistanceModifier(EnergyPlusData &state, Real64 flowRatio) const;
        void setupOutputVariables(EnergyPlusData &state);
    };

} // namespace LiquidCooledITEPlantComponents

struct LiquidCooledITEColdPlatesData : BaseGlobalStruct
{
    std::vector<LiquidCooledITEPlantComponents::CoilCoolingITEColdPlateData> coldPlates;
    bool getInputs = true;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) LiquidCooledITEColdPlatesData();
    }
};

} // namespace EnergyPlus

#endif // LiquidCooledITEPlantComponents_hh_INCLUDED