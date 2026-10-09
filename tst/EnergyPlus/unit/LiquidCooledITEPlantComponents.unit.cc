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

// EnergyPlus::LiquidCooledITEPlantComponents Unit Tests

// C++ Headers
#include <algorithm>
#include <cmath>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/LiquidCooledITEPlantComponents.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

using namespace LiquidCooledITEPlantComponents;

namespace {
    constexpr Real64 designInletTemp = 30.0;    // Sizing:Plant design loop exit (cold plate inlet) temperature [C]
    constexpr Real64 loopDeltaT = 10.0;         // Sizing:Plant loop design temperature difference [K]
    constexpr Real64 nominalResistance = 0.025; // nominal thermal resistance [K/W]
    constexpr Real64 maxCaseTemp = 85.0;        // maximum case temperature [C]
    constexpr Real64 targetCaseTemp = 80.0;     // target case operating temperature [C]

    // A plant loop with the cold plate alone on the demand side, and a cold plate with both flow rates autosized
    void setupColdPlate(EnergyPlusData &state, CoilCoolingITEColdPlateData &coldPlate, ThermalResistanceMethod const method)
    {
        state.init_state(state);

        state.dataPlnt->TotNumLoops = 1;
        state.dataPlnt->PlantLoop.allocate(1);
        auto &loop = state.dataPlnt->PlantLoop(1);
        loop.FluidName = "WATER";
        loop.glycol = Fluid::GetWater(state);
        loop.MaxVolFlowRate = DataSizing::AutoSize;
        loop.PlantSizNum = 1;
        auto &demandSide = loop.LoopSide(DataPlant::LoopSideLocation::Demand);
        demandSide.TotalBranches = 1;
        demandSide.Branch.allocate(1);
        demandSide.Branch(1).TotalComponents = 1;
        demandSide.Branch(1).Comp.allocate(1);

        state.dataSize->PlantSizData.allocate(1);
        state.dataSize->PlantSizData(1).ExitTemp = designInletTemp;
        state.dataSize->PlantSizData(1).DeltaT = loopDeltaT;

        state.dataLoopNodes->Node.allocate(2);
        state.dataLoopNodes->Node(1).Temp = designInletTemp;
        state.dataLoopNodes->Node(1).MassFlowRateMax = 10.0;
        state.dataLoopNodes->Node(1).MassFlowRateMaxAvail = 10.0;

        coldPlate.name = "COLD PLATE";
        coldPlate.plantLoc.loopNum = 1;
        coldPlate.plantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
        coldPlate.plantLoc.branchNum = 1;
        coldPlate.plantLoc.compNum = 1;
        coldPlate.plantLoc.loop = &loop;
        coldPlate.plantLoc.side = &demandSide;
        coldPlate.plantLoc.branch = &demandSide.Branch(1);
        coldPlate.plantLoc.comp = &demandSide.Branch(1).Comp(1);
        coldPlate.myPlantScanFlag = false;
        coldPlate.availabilitySchedule = Sched::GetScheduleAlwaysOn(state);
        coldPlate.inletNode = 1;
        coldPlate.outletNode = 2;
        coldPlate.thermalResistanceMethod = method;
        coldPlate.thermalResistance = nominalResistance;
        coldPlate.maximumCaseTemperature = maxCaseTemp;
        coldPlate.targetCaseOperatingTemperature = targetCaseTemp;
        coldPlate.nominalFlowRate = DataSizing::AutoSize;
        coldPlate.nominalFlowRateWasAutoSized = true;
        coldPlate.maximumFlowRate = DataSizing::AutoSize;
        coldPlate.maximumFlowRateWasAutoSized = true;

        coldPlate.onInitLoopEquip(state, coldPlate.plantLoc);
    }

    // Hard-size the maximum flow rate to a fraction of the (autosized) nominal flow rate
    void capMaximumFlow(EnergyPlusData &state, CoilCoolingITEColdPlateData &coldPlate, Real64 const fractionOfNominal)
    {
        coldPlate.maximumFlowRateWasAutoSized = false;
        coldPlate.maximumFlowRate = fractionOfNominal * coldPlate.nominalFlowRate;
        coldPlate.setMassFlowRates(state);
    }
} // namespace

TEST_F(EnergyPlusFixture, CoilCoolingITEColdPlate_StandardMethod)
{
    CoilCoolingITEColdPlateData coldPlate;
    setupColdPlate(*state, coldPlate, ThermalResistanceMethod::Standard);

    auto *glycol = state->dataPlnt->PlantLoop(1).glycol;
    Real64 const rho = glycol->getDensity(*state, designInletTemp, "UnitTest");
    Real64 const cp = glycol->getSpecificHeat(*state, designInletTemp, "UnitTest");

    // Sizing: the design load is (Ttarget - Tin) / R, carried by the loop design temperature difference
    Real64 const designLoad = (targetCaseTemp - designInletTemp) / nominalResistance;
    Real64 const designFlow = designLoad / (rho * cp * loopDeltaT);
    EXPECT_NEAR(designFlow, coldPlate.nominalFlowRate, 1.0e-10);
    EXPECT_NEAR(designFlow, coldPlate.maximumFlowRate, 1.0e-10);
    EXPECT_NEAR(designFlow * rho, coldPlate.nominalMassFlowRate, 1.0e-8);

    // Constant flow: the nominal flow is requested, the case temperature is Tin + q * R
    coldPlate.flowMode = DataPlant::FlowMode::Constant;
    coldPlate.loadFromITEquipment = 1500.0;
    coldPlate.doPhysics(*state);
    EXPECT_NEAR(coldPlate.nominalMassFlowRate, coldPlate.massFlowRate, 1.0e-8);
    EXPECT_NEAR(1500.0, coldPlate.heatRemovedByFluid, 1.0e-6);
    EXPECT_NEAR(0.0, coldPlate.zoneHeatGainRate, 1.0e-6);
    EXPECT_NEAR(designInletTemp + 1500.0 / (coldPlate.massFlowRate * cp), coldPlate.outletTemp, 1.0e-6);
    EXPECT_NEAR(designInletTemp + 1500.0 * nominalResistance, coldPlate.caseTemperature, 1.0e-6);

    // Variable flow: the capacity at the target case temperature is min(1/R, mdot*cp) * (Ttarget - Tin),
    // so a load below the design load is met when mdot*cp = load / (Ttarget - Tin) and the coolant leaves at the case temperature
    coldPlate.flowMode = DataPlant::FlowMode::Variable;
    coldPlate.loadFromITEquipment = 1000.0;
    coldPlate.doPhysics(*state);
    EXPECT_NEAR(1000.0 / (targetCaseTemp - designInletTemp) / cp, coldPlate.massFlowRate, 1.0e-5);
    EXPECT_NEAR(1000.0, coldPlate.heatRemovedByFluid, 1.0e-6);
    EXPECT_NEAR(0.0, coldPlate.zoneHeatGainRate, 1.0e-6);
    EXPECT_NEAR(targetCaseTemp, coldPlate.caseTemperature, 1.0e-3);
    EXPECT_NEAR(targetCaseTemp, coldPlate.outletTemp, 1.0e-3);

    // Maximum flow: the load cannot be met, so the maximum flow is used and the remainder goes to the zone
    capMaximumFlow(*state, coldPlate, 0.1);
    coldPlate.loadFromITEquipment = 2000.0;
    coldPlate.doPhysics(*state);
    Real64 const maxCapacity = std::min(1.0 / nominalResistance, coldPlate.maximumMassFlowRate * cp) * (maxCaseTemp - designInletTemp);
    ASSERT_LT(maxCapacity, 2000.0);
    EXPECT_NEAR(coldPlate.maximumMassFlowRate, coldPlate.massFlowRate, 1.0e-8);
    EXPECT_NEAR(maxCapacity, coldPlate.heatRemovedByFluid, 1.0e-6);
    EXPECT_NEAR(2000.0 - maxCapacity, coldPlate.zoneHeatGainRate, 1.0e-6);
    EXPECT_NEAR(maxCaseTemp, coldPlate.caseTemperature, 1.0e-6);
}

TEST_F(EnergyPlusFixture, CoilCoolingITEColdPlate_LMTDMethod)
{
    CoilCoolingITEColdPlateData coldPlate;
    setupColdPlate(*state, coldPlate, ThermalResistanceMethod::LMTD);

    auto *glycol = state->dataPlnt->PlantLoop(1).glycol;
    Real64 const rho = glycol->getDensity(*state, designInletTemp, "UnitTest");
    Real64 const cp = glycol->getSpecificHeat(*state, designInletTemp, "UnitTest");

    // Sizing: the design load is LMTD / R, carried by the loop design temperature difference
    Real64 const designOutletTemp = designInletTemp + loopDeltaT;
    Real64 const caseToOutlet = targetCaseTemp - designOutletTemp;
    Real64 const caseToInlet = targetCaseTemp - designInletTemp;
    Real64 const lmtd = (caseToOutlet - caseToInlet) / std::log(caseToOutlet / caseToInlet);
    Real64 const designLoad = lmtd / nominalResistance;
    Real64 const designFlow = designLoad / (rho * cp * loopDeltaT);
    EXPECT_NEAR(designFlow, coldPlate.nominalFlowRate, 1.0e-10);
    EXPECT_NEAR(designFlow, coldPlate.maximumFlowRate, 1.0e-10);
    EXPECT_NEAR(designFlow * rho, coldPlate.nominalMassFlowRate, 1.0e-8);

    // Constant flow at the design load: the sizing and the operation are consistent, so the case is at the target temperature
    // and the coolant leaves at the design outlet temperature
    coldPlate.flowMode = DataPlant::FlowMode::Constant;
    coldPlate.loadFromITEquipment = designLoad;
    coldPlate.doPhysics(*state);
    EXPECT_NEAR(coldPlate.nominalMassFlowRate, coldPlate.massFlowRate, 1.0e-8);
    EXPECT_NEAR(designLoad, coldPlate.heatRemovedByFluid, 1.0e-6);
    EXPECT_NEAR(0.0, coldPlate.zoneHeatGainRate, 1.0e-6);
    EXPECT_NEAR(designOutletTemp, coldPlate.outletTemp, 1.0e-6);
    EXPECT_NEAR(targetCaseTemp, coldPlate.caseTemperature, 1.0e-6);

    // Variable flow at the design load: the flow rate that holds the case at the target temperature is the nominal flow rate.
    // The maximum flow is above the nominal flow so that the load is not exactly at the capacity limit.
    coldPlate.flowMode = DataPlant::FlowMode::Variable;
    capMaximumFlow(*state, coldPlate, 2.0);
    coldPlate.doPhysics(*state);
    EXPECT_NEAR(coldPlate.nominalMassFlowRate, coldPlate.massFlowRate, 1.0e-5);
    EXPECT_NEAR(designLoad, coldPlate.heatRemovedByFluid, 1.0e-6);
    EXPECT_NEAR(0.0, coldPlate.zoneHeatGainRate, 1.0e-6);
    EXPECT_NEAR(targetCaseTemp, coldPlate.caseTemperature, 1.0e-3);

    // Maximum flow: the load cannot be met, so the maximum flow is used and the remainder goes to the zone
    capMaximumFlow(*state, coldPlate, 0.25);
    coldPlate.doPhysics(*state);
    Real64 const capacityRate = coldPlate.maximumMassFlowRate * cp;
    Real64 const maxCapacity = capacityRate * (1.0 - std::exp(-1.0 / (capacityRate * nominalResistance))) * (maxCaseTemp - designInletTemp);
    ASSERT_LT(maxCapacity, designLoad);
    EXPECT_NEAR(coldPlate.maximumMassFlowRate, coldPlate.massFlowRate, 1.0e-8);
    EXPECT_NEAR(maxCapacity, coldPlate.heatRemovedByFluid, 1.0e-6);
    EXPECT_NEAR(designLoad - maxCapacity, coldPlate.zoneHeatGainRate, 1.0e-6);
    EXPECT_NEAR(maxCaseTemp, coldPlate.caseTemperature, 1.0e-6);
}

} // namespace EnergyPlus
