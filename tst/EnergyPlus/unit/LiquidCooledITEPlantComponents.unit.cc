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
#include <format>
#include <string>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHeatBalance.hh>
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

    // The coolant is a 25% propylene glycol mixture. The fluid is the only input object used; the plant and the cold plate are set up in code.
    constexpr char const *pg25Idf = "FluidProperties:GlycolConcentration,\n  PG25, PropyleneGlycol, , 0.25;\n";

    // A load schedule with a low value until 08:00, the peak value until 16:00 and a high value after that, so that neither the first
    // nor the last value of the schedule is its maximum (the compact schedule keeps the names upper case)
    std::string loadSchedule(std::string const &name, Real64 const low, Real64 const peak, Real64 const high)
    {
        return std::format(
            "Schedule:Compact,{},,Through: 12/31,For: AllDays,Until: 08:00,{},Until: 16:00,{},Until: 24:00,{};\n", name, low, peak, high);
    }

    // A plant loop with the cold plate alone on the demand side, and a cold plate with both flow rates autosized
    void setupColdPlate(EnergyPlusData &state,
                        CoilCoolingITEColdPlateData &coldPlate,
                        ThermalResistanceMethod const method,
                        Real64 const sizingFactor = 1.0,
                        NominalFlowSizingBasis const basis = NominalFlowSizingBasis::ColdPlateCapacity,
                        std::string const &loadScheduleName = "")
    {
        state.init_state(state);

        state.dataPlnt->TotNumLoops = 1;
        state.dataPlnt->PlantLoop.allocate(1);
        auto &loop = state.dataPlnt->PlantLoop(1);
        loop.FluidName = "PG25";
        loop.glycol = Fluid::GetGlycol(state, "PG25");
        EXPECT_NE(nullptr, loop.glycol);
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
        coldPlate.sizingFactor = sizingFactor;
        coldPlate.nominalFlowSizingBasis = basis;
        coldPlate.itLoadSchedule = loadScheduleName.empty() ? nullptr : Sched::GetSchedule(state, loadScheduleName);

        coldPlate.onInitLoopEquip(state, coldPlate.plantLoc);
    }

    // Hard-size the maximum flow rate to a fraction of the (autosized) nominal flow rate
    void capMaximumFlow(EnergyPlusData &state, CoilCoolingITEColdPlateData &coldPlate, Real64 const fractionOfNominal)
    {
        coldPlate.maximumFlowRateWasAutoSized = false;
        coldPlate.maximumFlowRate = fractionOfNominal * coldPlate.nominalFlowRate;
        coldPlate.setMassFlowRates(state);
    }

    // Size the same cold plate again with another basis, load schedule and sizing factor, both flow rates autosized
    void resizeColdPlate(EnergyPlusData &state,
                         CoilCoolingITEColdPlateData &coldPlate,
                         NominalFlowSizingBasis const basis,
                         std::string const &loadScheduleName,
                         Real64 const sizingFactor = 1.0)
    {
        coldPlate.nominalFlowSizingBasis = basis;
        coldPlate.itLoadSchedule = loadScheduleName.empty() ? nullptr : Sched::GetSchedule(state, loadScheduleName);
        coldPlate.sizingFactor = sizingFactor;
        coldPlate.nominalFlowRate = DataSizing::AutoSize;
        coldPlate.nominalFlowRateWasAutoSized = true;
        coldPlate.maximumFlowRate = DataSizing::AutoSize;
        coldPlate.maximumFlowRateWasAutoSized = true;
        coldPlate.sizeColdPlate(state);
    }
} // namespace

TEST_F(EnergyPlusFixture, CoilCoolingITEColdPlate_StandardMethod)
{
    ASSERT_TRUE(process_idf(std::string(pg25Idf) + loadSchedule("Peak1500", 500.0, 1500.0, 800.0) + loadSchedule("Peak3000", 1000.0, 3000.0, 2000.0) +
                            loadSchedule("Zero", 0.0, 0.0, 0.0)));
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
    Real64 const constantFlow = coldPlate.massFlowRate;

    // Variable flow: the capacity at the target case temperature is min(1/R, mdot*cp) * (Ttarget - Tin),
    // so a load below the design load is met when mdot*cp = load / (Ttarget - Tin) and the coolant leaves at the case temperature
    coldPlate.flowMode = DataPlant::FlowMode::Variable;
    coldPlate.loadFromITEquipment = 1000.0;
    coldPlate.doPhysics(*state);
    EXPECT_LT(coldPlate.massFlowRate, constantFlow); // at part load the variable flow is below the constant flow
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

    // Case temperature without a load or without cooling: it cannot be calculated, so the last value is held
    capMaximumFlow(*state, coldPlate, 1.0);
    coldPlate.flowMode = DataPlant::FlowMode::Variable;
    coldPlate.loadFromITEquipment = 1000.0;
    coldPlate.doPhysics(*state);
    ASSERT_NEAR(targetCaseTemp, coldPlate.caseTemperature, 1.0e-3);
    // (a) Idle: nothing changes, and 0 is not reported
    coldPlate.loadFromITEquipment = 0.0;
    coldPlate.doPhysics(*state);
    EXPECT_NEAR(targetCaseTemp, coldPlate.caseTemperature, 1.0e-3);
    EXPECT_NEAR(0.0, coldPlate.massFlowRate, 1.0e-12);
    // (b) A load but the cold plate is not available: all of the load goes to the zone, the case temperature is held
    coldPlate.availabilitySchedule = Sched::GetScheduleAlwaysOff(*state);
    coldPlate.loadFromITEquipment = 1500.0;
    coldPlate.doPhysics(*state);
    EXPECT_NEAR(1500.0, coldPlate.zoneHeatGainRate, 1.0e-9);
    EXPECT_NEAR(targetCaseTemp, coldPlate.caseTemperature, 1.0e-3);
    coldPlate.availabilitySchedule = Sched::GetScheduleAlwaysOn(*state);
    // (c) Available again: the case temperature is calculated again, and is then the held value when the load stops
    coldPlate.flowMode = DataPlant::FlowMode::Constant;
    coldPlate.doPhysics(*state);
    EXPECT_NEAR(designInletTemp + 1500.0 * nominalResistance, coldPlate.caseTemperature, 1.0e-6);
    coldPlate.loadFromITEquipment = 0.0;
    coldPlate.doPhysics(*state);
    EXPECT_NEAR(designInletTemp + 1500.0 * nominalResistance, coldPlate.caseTemperature, 1.0e-6);
    // (d) The start of an environment resets the case temperature to the target case operating temperature
    state->dataGlobal->BeginEnvrnFlag = true;
    coldPlate.myEnvrnFlag = true;
    Real64 curLoad = 0.0;
    coldPlate.simulate(*state, coldPlate.plantLoc, true, curLoad, true);
    EXPECT_NEAR(targetCaseTemp, coldPlate.caseTemperature, 1.0e-9);
    state->dataGlobal->BeginEnvrnFlag = false;

    // MaximumITELoad: the design load is the maximum of the load schedule, limited to the heat transferred at the maximum case temperature
    constexpr Real64 loadLimit = (maxCaseTemp - designInletTemp) / nominalResistance; // 2,200 W
    (void)has_err_output(true);
    (void)has_eio_output(true);

    // (a) The schedule maximum (1,500 W, mid-day) is below the limit: no limit, no warning
    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    resizeColdPlate(*state, coldPlate, NominalFlowSizingBasis::MaximumITELoad, "PEAK1500");
    EXPECT_NEAR(1500.0 / (rho * cp * loopDeltaT), coldPlate.nominalFlowRate, 1.0e-10);
    EXPECT_NEAR(coldPlate.nominalFlowRate, coldPlate.maximumFlowRate, 1.0e-10);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_TRUE(compare_eio_stream_substring("Design Size Design Load [W], 1500", true));

    // (b) The schedule maximum (3,000 W) is above the limit: the sizing load is limited, and the warning is issued only for the final sizes
    state->dataPlnt->PlantFinalSizesOkayToReport = false;
    resizeColdPlate(*state, coldPlate, NominalFlowSizingBasis::MaximumITELoad, "PEAK3000");
    coldPlate.sizeColdPlate(*state);
    EXPECT_NEAR(loadLimit / (rho * cp * loopDeltaT), coldPlate.nominalFlowRate, 1.0e-10);
    EXPECT_NEAR(coldPlate.nominalFlowRate, coldPlate.maximumFlowRate, 1.0e-10);
    EXPECT_FALSE(has_err_output(true));
    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    coldPlate.sizeColdPlate(*state);
    EXPECT_TRUE(compare_err_stream_substring("cannot transfer the maximum IT Equipment Load Schedule value (3000.00 W)", false));
    EXPECT_TRUE(compare_err_stream_substring("limited to 2200.00 W. The design load used for autosizing was limited", true));
    EXPECT_TRUE(compare_eio_stream_substring("Design Size Design Load [W], 2200", true));
    EXPECT_NEAR(loadLimit / (rho * cp * loopDeltaT), coldPlate.nominalFlowRate, 1.0e-10); // flows are still set

    // (c) A hard-sized nominal flow rate is compared with the load-based design flow rate
    coldPlate.nominalFlowRateWasAutoSized = false;
    coldPlate.nominalFlowRate = 0.5 * 1500.0 / (rho * cp * loopDeltaT);
    coldPlate.nominalFlowSizingBasis = NominalFlowSizingBasis::MaximumITELoad;
    coldPlate.itLoadSchedule = Sched::GetSchedule(*state, "PEAK1500");
    coldPlate.sizeColdPlate(*state);
    // The design flow rate in the comparison is the load-based one (1,500 W), not the capacity-based one (2,000 W)
    EXPECT_TRUE(
        compare_eio_stream_substring(std::format("Design Size Nominal Liquid Flow Rate [m3/s], {:.6G}", 1500.0 / (rho * cp * loopDeltaT)), false));
    EXPECT_EQ(std::string::npos, state->files.eio.get_output().find(std::format("Design Size Nominal Liquid Flow Rate [m3/s], {:.6G}", designFlow)));
    EXPECT_TRUE(
        compare_eio_stream_substring(std::format("User-Specified Nominal Liquid Flow Rate [m3/s], {:.6G}", coldPlate.nominalFlowRate), false));
    EXPECT_TRUE(compare_eio_stream_substring("Design Size Design Load [W], 1500", true));
    EXPECT_FALSE(has_err_output(true));

    // (d) With a hard-sized nominal flow rate a schedule that is 0 everywhere is only warned about: the comparison is skipped
    coldPlate.itLoadSchedule = Sched::GetSchedule(*state, "ZERO");
    coldPlate.sizeColdPlate(*state);
    EXPECT_TRUE(compare_err_stream_substring("The design flow rate is not calculated for comparison", true));
    EXPECT_EQ(std::string::npos, state->files.eio.get_output().find("Design Size Nominal Liquid Flow Rate"));
    EXPECT_TRUE(compare_eio_stream_substring("User-Specified Nominal Liquid Flow Rate [m3/s]", true));
}

TEST_F(EnergyPlusFixture, CoilCoolingITEColdPlate_SizingFactor)
{
    ASSERT_TRUE(process_idf(std::string(pg25Idf) + loadSchedule("Peak4000", 1000.0, 4000.0, 2000.0) +
                            loadSchedule("Peak10000", 1000.0, 10000.0, 2000.0) + loadSchedule("Zero", 0.0, 0.0, 0.0)));
    constexpr Real64 sizingFactor = 0.25; // e.g. four identical cold plates sharing the load

    CoilCoolingITEColdPlateData coldPlate;
    setupColdPlate(*state, coldPlate, ThermalResistanceMethod::Standard, sizingFactor);

    auto *glycol = state->dataPlnt->PlantLoop(1).glycol;
    Real64 const rho = glycol->getDensity(*state, designInletTemp, "UnitTest");
    Real64 const cp = glycol->getSpecificHeat(*state, designInletTemp, "UnitTest");

    // The autosized flow rates are the full design flow rate scaled by the sizing factor
    Real64 const designLoad = (targetCaseTemp - designInletTemp) / nominalResistance;
    Real64 const designFlow = designLoad / (rho * cp * loopDeltaT);
    EXPECT_NEAR(sizingFactor * designFlow, coldPlate.nominalFlowRate, 1.0e-10);
    EXPECT_NEAR(sizingFactor * designFlow, coldPlate.maximumFlowRate, 1.0e-10);
    EXPECT_NEAR(sizingFactor * designFlow * rho, coldPlate.nominalMassFlowRate, 1.0e-8);

    // MaximumITELoad: the sizing factor multiplies the schedule maximum, then the heat transferred at the maximum case temperature is the limit
    constexpr Real64 loadLimit = (maxCaseTemp - designInletTemp) / nominalResistance; // 2,200 W
    // (a) 0.25 * 4,000 W = 1,000 W is below the limit
    resizeColdPlate(*state, coldPlate, NominalFlowSizingBasis::MaximumITELoad, "PEAK4000", sizingFactor);
    EXPECT_NEAR(1000.0 / (rho * cp * loopDeltaT), coldPlate.nominalFlowRate, 1.0e-10);
    // (b) 0.25 * 10,000 W = 2,500 W is above the limit, which shows that the limit is applied after the sizing factor
    resizeColdPlate(*state, coldPlate, NominalFlowSizingBasis::MaximumITELoad, "PEAK10000", sizingFactor);
    EXPECT_NEAR(loadLimit / (rho * cp * loopDeltaT), coldPlate.nominalFlowRate, 1.0e-10);
    // (c) A schedule that is 0 everywhere is a fatal error when the nominal flow rate is autosized
    ASSERT_THROW(resizeColdPlate(*state, coldPlate, NominalFlowSizingBasis::MaximumITELoad, "ZERO", sizingFactor), std::runtime_error);
    EXPECT_TRUE(compare_err_stream_substring("requires an IT Equipment Load Schedule with a maximum value greater than 0 W", true));
}

TEST_F(EnergyPlusFixture, CoilCoolingITEColdPlate_LMTDMethod)
{
    // The linear modifier curve is 1.0 at a flow ratio of 1, 0.6 at 3 and negative above 6
    ASSERT_TRUE(
        process_idf(std::string(pg25Idf) + loadSchedule("Peak2100", 500.0, 2100.0, 1000.0) + "Curve:Linear,NegativeAboveSix,1.2,-0.2,0.0,10.0;\n"));
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

    // Variable flow at half the design load: the flow rate drops below the constant (nominal) flow rate and the case stays at the target
    coldPlate.loadFromITEquipment = 0.5 * designLoad;
    coldPlate.doPhysics(*state);
    EXPECT_LT(coldPlate.massFlowRate, coldPlate.nominalMassFlowRate);
    EXPECT_NEAR(0.5 * designLoad, coldPlate.heatRemovedByFluid, 1.0e-6);
    EXPECT_NEAR(0.0, coldPlate.zoneHeatGainRate, 1.0e-6);
    EXPECT_NEAR(targetCaseTemp, coldPlate.caseTemperature, 1.0e-3);

    // Maximum flow: the load cannot be met, so the maximum flow is used and the remainder goes to the zone
    coldPlate.loadFromITEquipment = designLoad;
    capMaximumFlow(*state, coldPlate, 0.25);
    coldPlate.doPhysics(*state);
    Real64 const capacityRate = coldPlate.maximumMassFlowRate * cp;
    Real64 const maxCapacity = capacityRate * (1.0 - std::exp(-1.0 / (capacityRate * nominalResistance))) * (maxCaseTemp - designInletTemp);
    ASSERT_LT(maxCapacity, designLoad);
    EXPECT_NEAR(coldPlate.maximumMassFlowRate, coldPlate.massFlowRate, 1.0e-8);
    EXPECT_NEAR(maxCapacity, coldPlate.heatRemovedByFluid, 1.0e-6);
    EXPECT_NEAR(designLoad - maxCapacity, coldPlate.zoneHeatGainRate, 1.0e-6);
    EXPECT_NEAR(maxCaseTemp, coldPlate.caseTemperature, 1.0e-6);

    // MaximumITELoad with a 2,100 W schedule maximum, below the 2,200 W limit
    (void)has_err_output(true);
    state->dataPlnt->PlantFinalSizesOkayToReport = true;

    // (a) At the autosized maximum flow rate the LMTD effectiveness is low: Tcase = Tin + dT / eps = 87.7 C, above the maximum case temperature
    resizeColdPlate(*state, coldPlate, NominalFlowSizingBasis::MaximumITELoad, "PEAK2100");
    EXPECT_NEAR(2100.0 / (rho * cp * loopDeltaT), coldPlate.nominalFlowRate, 1.0e-10);
    EXPECT_TRUE(compare_err_stream_substring("cannot transfer the maximum IT Equipment Load Schedule value (2100.00 W)", false));
    EXPECT_TRUE(compare_err_stream_substring("The design load used for autosizing was not limited", true));

    // (b) A hard-sized maximum flow rate of 3 times the nominal flow rate lowers the case temperature needed to 84.2 C: no warning
    coldPlate.maximumFlowRateWasAutoSized = false;
    coldPlate.maximumFlowRate = 3.0 * coldPlate.nominalFlowRate;
    coldPlate.sizeColdPlate(*state);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_NEAR(3.0 * coldPlate.nominalFlowRate * rho, coldPlate.maximumMassFlowRate, 1.0e-8);
    EXPECT_NEAR(2100.0 / (rho * cp * loopDeltaT), coldPlate.nominalFlowRate, 1.0e-10); // the flows are still set

    // (c) A loop design return temperature at or above the maximum case temperature is warned about
    state->dataSize->PlantSizData(1).DeltaT = maxCaseTemp - designInletTemp + 5.0;
    resizeColdPlate(*state, coldPlate, NominalFlowSizingBasis::MaximumITELoad, "PEAK2100");
    EXPECT_TRUE(compare_err_stream_substring("is not greater than the Sizing:Plant loop design return temperature", true));

    // (d) A modifier curve that is positive at the nominal flow but negative at the maximum flow: the thermal resistance is 0 or negative
    // there, a fatal error in the sizing check and in operation (the design resistance at a flow ratio of 1 is valid)
    state->dataSize->PlantSizData(1).DeltaT = loopDeltaT;
    coldPlate.thermalResistanceModifierCurveIndex = Curve::GetCurveIndex(*state, "NEGATIVEABOVESIX");
    ASSERT_GT(coldPlate.thermalResistanceModifierCurveIndex, 0);
    resizeColdPlate(*state, coldPlate, NominalFlowSizingBasis::MaximumITELoad, "PEAK2100");
    coldPlate.maximumFlowRateWasAutoSized = false;
    coldPlate.maximumFlowRate = 7.0 * coldPlate.nominalFlowRate;
    ASSERT_THROW(coldPlate.sizeColdPlate(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream_substring("is 0 or negative", true));
    // Operation: the load is above what one plate removes below the negative range, so the flow search reaches it
    coldPlate.setMassFlowRates(*state);
    coldPlate.flowMode = DataPlant::FlowMode::Variable;
    coldPlate.loadFromITEquipment = 2100.0;
    ASSERT_THROW(coldPlate.doPhysics(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream_substring("is 0 or negative", true));
}

TEST_F(EnergyPlusFixture, CoilCoolingITEColdPlate_InputChecks)
{
    // Three cold plates: MaximumITELoad without a load schedule, ColdPlateCapacity with a negative load schedule, and MaximumITELoad with a
    // negative load schedule
    auto const plate = [](std::string const &name, std::string const &loadScheduleName, std::string const &basis) {
        return std::format(
            "Coil:Cooling:ITE:ColdPlate,{},,{} Inlet,{} Outlet,0.025,85,80,,Standard,Autosize,Autosize,0,Zone One,VariableFlow,General,{},1,{};\n",
            name,
            name,
            name,
            loadScheduleName,
            basis);
    };
    ASSERT_TRUE(process_idf(std::string(pg25Idf) + loadSchedule("Negative", -100.0, 1500.0, 800.0) + plate("Plate1", "", "MaximumITELoad") +
                            plate("Plate2", "Negative", "ColdPlateCapacity") + plate("Plate3", "Negative", "MaximumITELoad")));
    state->init_state(*state);
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Name = "ZONE ONE";

    ASSERT_THROW(CoilCoolingITEColdPlateData::processInputForCoilCoolingITEColdPlate(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream_substring(
        "Nominal Flow Rate Sizing Basis = MaximumITELoad requires an IT Equipment Load Schedule Name for object \"PLATE1\"", false));
    EXPECT_TRUE(compare_err_stream_substring("has a negative value for object \"PLATE2\"", false));
    EXPECT_TRUE(compare_err_stream_substring("has a negative value for object \"PLATE3\"", false));
    EXPECT_TRUE(compare_err_stream_substring("Errors found in processing input for Coil:Cooling:ITE:ColdPlate", true));
}

TEST_F(EnergyPlusFixture, CoilCoolingITEColdPlate_Multipliers)
{
    // One cold plate object standing for N = nIte * nZone identical plates: 5 racks (IT equipment multiplier) in 2 copies of the zone
    ASSERT_TRUE(process_idf(pg25Idf));
    CoilCoolingITEColdPlateData coldPlate;
    setupColdPlate(*state, coldPlate, ThermalResistanceMethod::Standard);
    coldPlate.auxPower = 30.0;

    auto *glycol = state->dataPlnt->PlantLoop(1).glycol;
    Real64 const cp = glycol->getSpecificHeat(*state, designInletTemp, "UnitTest");
    constexpr Real64 plates = 10.0;

    // Reference results of one plate (N = 1) in constant flow at 1,500 W and in variable flow at 1,000 W
    coldPlate.flowMode = DataPlant::FlowMode::Constant;
    coldPlate.loadFromITEquipment = 1500.0;
    coldPlate.doPhysics(*state);
    Real64 const constantMassFlow = coldPlate.massFlowRate;
    Real64 const constantHeat = coldPlate.heatRemovedByFluid;
    Real64 const constantCaseTemp = coldPlate.caseTemperature;
    Real64 const constantOutletTemp = coldPlate.outletTemp;
    EXPECT_NEAR(30.0, coldPlate.auxElecPower, 1.0e-9);
    coldPlate.flowMode = DataPlant::FlowMode::Variable;
    coldPlate.loadFromITEquipment = 1000.0;
    coldPlate.doPhysics(*state);
    Real64 const variableMassFlow = coldPlate.massFlowRate;
    Real64 const variableHeat = coldPlate.heatRemovedByFluid;
    Real64 const variableCaseTemp = coldPlate.caseTemperature;
    Real64 const variableOutletTemp = coldPlate.outletTemp;
    ASSERT_GT(variableMassFlow, 0.0);

    // Sizing: the flow rates stay per plate, the flow registered with the plant is that of all plates
    Real64 const nominalFlowPerPlate = coldPlate.nominalFlowRate;
    coldPlate.nIte = 5.0;
    coldPlate.nZone = 2.0;
    coldPlate.plateZoneMultiplier = 2.0;
    resizeColdPlate(*state, coldPlate, NominalFlowSizingBasis::ColdPlateCapacity, "");
    EXPECT_NEAR(nominalFlowPerPlate, coldPlate.nominalFlowRate, 1.0e-12);
    EXPECT_NEAR(coldPlate.nominalFlowRate, coldPlate.maximumFlowRate, 1.0e-12);
    Real64 registeredFlow = 0.0;
    for (int i = 1; i <= state->dataSize->SaveNumPlantComps; ++i) {
        if (state->dataSize->CompDesWaterFlow(i).SupNode == coldPlate.inletNode) {
            registeredFlow = state->dataSize->CompDesWaterFlow(i).DesVolFlowRate;
        }
    }
    EXPECT_NEAR(plates * nominalFlowPerPlate, registeredFlow, 1.0e-12);

    // Constant flow: the load of the equipment (7,500 W, includes the 5 racks) is 1,500 W per plate. The plant sees 10 plates,
    // the case and coolant outlet temperatures are those of one plate and nothing is added to the zone
    coldPlate.flowMode = DataPlant::FlowMode::Constant;
    coldPlate.loadFromITEquipment = 5.0 * 1500.0;
    coldPlate.doPhysics(*state);
    EXPECT_NEAR(plates * constantMassFlow, coldPlate.massFlowRate, 1.0e-9);
    EXPECT_NEAR(plates * constantHeat, coldPlate.heatRemovedByFluid, 1.0e-6);
    EXPECT_NEAR(constantCaseTemp, coldPlate.caseTemperature, 1.0e-9);
    EXPECT_NEAR(constantOutletTemp, coldPlate.outletTemp, 1.0e-9);
    EXPECT_NEAR(0.0, coldPlate.zoneHeatGainRate, 1.0e-9);
    EXPECT_NEAR(plates * 30.0, coldPlate.auxElecPower, 1.0e-9);

    // Variable flow: 5,000 W of equipment load is 1,000 W per plate
    coldPlate.flowMode = DataPlant::FlowMode::Variable;
    coldPlate.loadFromITEquipment = 5.0 * 1000.0;
    coldPlate.doPhysics(*state);
    EXPECT_NEAR(plates * variableMassFlow, coldPlate.massFlowRate, 1.0e-6);
    EXPECT_NEAR(plates * variableHeat, coldPlate.heatRemovedByFluid, 1.0e-6);
    EXPECT_NEAR(variableCaseTemp, coldPlate.caseTemperature, 1.0e-6);
    EXPECT_NEAR(variableOutletTemp, coldPlate.outletTemp, 1.0e-6);
    EXPECT_NEAR(0.0, coldPlate.zoneHeatGainRate, 1.0e-6);

    // Saturation: 10,000 W of equipment load is 2,000 W per plate, which one plate cannot remove at its (capped) maximum flow
    capMaximumFlow(*state, coldPlate, 0.1);
    coldPlate.loadFromITEquipment = 5.0 * 2000.0;
    coldPlate.doPhysics(*state);
    Real64 const maxCapacity = std::min(1.0 / nominalResistance, coldPlate.maximumMassFlowRate * cp) * (maxCaseTemp - designInletTemp);
    Real64 const shortfallPerPlate = 2000.0 - maxCapacity;
    ASSERT_GT(shortfallPerPlate, 0.0);
    EXPECT_NEAR(plates * coldPlate.maximumMassFlowRate, coldPlate.massFlowRate, 1.0e-8);
    EXPECT_NEAR(plates * maxCapacity, coldPlate.heatRemovedByFluid, 1.0e-6);
    EXPECT_NEAR(maxCaseTemp, coldPlate.caseTemperature, 1.0e-6);
    // The zone heat gain is per copy of the zone that receives it: the total shortfall divided by its multiplier
    EXPECT_NEAR(plates * shortfallPerPlate / 2.0, coldPlate.zoneHeatGainRate, 1.0e-6); // 2 copies: 1,500 W each when the shortfall is 300 W
    coldPlate.plateZoneMultiplier = 1.0;
    coldPlate.doPhysics(*state);
    EXPECT_NEAR(plates * shortfallPerPlate, coldPlate.zoneHeatGainRate, 1.0e-6); // 1 copy: all of the shortfall of both zone copies
    coldPlate.plateZoneMultiplier = 2.0;

    // Not available: all of the load goes to the zone, per copy
    coldPlate.availabilitySchedule = Sched::GetScheduleAlwaysOff(*state);
    coldPlate.doPhysics(*state);
    EXPECT_NEAR(0.0, coldPlate.massFlowRate, 1.0e-12);
    EXPECT_NEAR(0.0, coldPlate.heatRemovedByFluid, 1.0e-12);
    EXPECT_NEAR(plates * 2000.0 / 2.0, coldPlate.zoneHeatGainRate, 1.0e-6);
    EXPECT_NEAR(0.0, coldPlate.auxElecPower, 1.0e-12);
    coldPlate.availabilitySchedule = Sched::GetScheduleAlwaysOn(*state);

    // Flow-limited loop: the plant delivers only half of the nominal flow of ONE plate (one twentieth per plate): the per-plate
    // physics uses the flow that was delivered, divided by the number of plates
    coldPlate.flowMode = DataPlant::FlowMode::Constant;
    coldPlate.maximumFlowRateWasAutoSized = true;
    coldPlate.maximumFlowRate = coldPlate.nominalFlowRate;
    coldPlate.setMassFlowRates(*state);
    state->dataPlnt->PlantLoop(1).MaxVolFlowRate = 1.0; // a sized loop, so that the plant limits the flow of the component
    state->dataLoopNodes->Node(coldPlate.inletNode).MassFlowRateMaxAvail = 0.5 * coldPlate.nominalMassFlowRate;
    coldPlate.loadFromITEquipment = 5.0 * 1500.0;
    coldPlate.doPhysics(*state);
    Real64 const deliveredPerPlate = 0.5 * coldPlate.nominalMassFlowRate / plates;
    Real64 const limitedCapacity = std::min(1.0 / nominalResistance, deliveredPerPlate * cp) * (maxCaseTemp - designInletTemp);
    ASSERT_LT(limitedCapacity, 1500.0);
    EXPECT_NEAR(0.5 * coldPlate.nominalMassFlowRate, coldPlate.massFlowRate, 1.0e-9);
    EXPECT_NEAR(plates * limitedCapacity, coldPlate.heatRemovedByFluid, 1.0e-6);
    EXPECT_NEAR(plates * (1500.0 - limitedCapacity) / 2.0, coldPlate.zoneHeatGainRate, 1.0e-6);
}

TEST_F(EnergyPlusFixture, CoilCoolingITEColdPlate_ZoneMultiplierInput)
{
    // A cold plate without paired IT equipment in a zone with multipliers stands for every copy of the zone
    ASSERT_TRUE(process_idf(std::string(pg25Idf) +
                            "Coil:Cooling:ITE:ColdPlate,Plate1,,Plate1 Inlet,Plate1 "
                            "Outlet,0.025,85,80,,Standard,Autosize,Autosize,0,Zone One,VariableFlow,General,,1,ColdPlateCapacity;\n"));
    state->init_state(*state);
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Name = "ZONE ONE";
    state->dataHeatBal->Zone(1).Multiplier = 3;
    state->dataHeatBal->Zone(1).ListMultiplier = 2;

    CoilCoolingITEColdPlateData::processInputForCoilCoolingITEColdPlate(*state);
    ASSERT_EQ(1u, state->dataLiquidCooledITE->coldPlates.size());
    auto const &coldPlate = state->dataLiquidCooledITE->coldPlates[0];
    EXPECT_EQ(6.0, coldPlate.plateZoneMultiplier);
    EXPECT_EQ(6.0, coldPlate.nZone);
    EXPECT_EQ(1.0, coldPlate.nIte);
    EXPECT_EQ(6.0, coldPlate.plateCount());
}

TEST_F(EnergyPlusFixture, CoilCoolingITEColdPlate_FollowMultipliersInput)
{
    // Follow IT Equipment Multipliers = None: N is 1 in a zone without multipliers
    auto const plate = [](std::string const &name, std::string const &zone, std::string const &follow) {
        return std::format("Coil:Cooling:ITE:ColdPlate,{},,{} Inlet,{} "
                           "Outlet,0.025,85,80,,Standard,Autosize,Autosize,0,{},VariableFlow,General,,1,ColdPlateCapacity,{};\n",
                           name,
                           name,
                           name,
                           zone,
                           follow);
    };
    ASSERT_TRUE(process_idf(std::string(pg25Idf) + plate("Plate1", "Zone One", "None") +
                            plate("Plate2", "Zone One", "ITEMultiplierAndZoneMultiplier") +
                            plate("Plate3", "Zone Two", "ITEMultiplierAndZoneMultiplier") + plate("Plate4", "Zone Two", "None")));
    state->init_state(*state);
    state->dataGlobal->NumOfZones = 2;
    state->dataHeatBal->Zone.allocate(2);
    state->dataHeatBal->Zone(1).Name = "ZONE ONE";
    state->dataHeatBal->Zone(2).Name = "ZONE TWO";
    state->dataHeatBal->Zone(2).Multiplier = 3;

    // None in a zone with a zone multiplier greater than 1 is a severe error
    ASSERT_THROW(CoilCoolingITEColdPlateData::processInputForCoilCoolingITEColdPlate(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream_substring(
        "Follow IT Equipment Multipliers = None is not allowed for object \"PLATE4\" because zone \"ZONE TWO\" has a zone "
        "multiplier of 3.",
        false));
    EXPECT_FALSE(compare_err_stream_substring("object \"PLATE1\"", false, false));
    EXPECT_FALSE(compare_err_stream_substring("object \"PLATE3\"", false, false));

    // The plates in the unmultiplied zone and the plate that follows the multipliers in the multiplied zone
    auto const &plates = state->dataLiquidCooledITE->coldPlates;
    ASSERT_EQ(4u, plates.size());
    EXPECT_EQ(FollowMultipliers::None, plates[0].followMultipliers);
    EXPECT_EQ(1.0, plates[0].plateCount());
    EXPECT_EQ(FollowMultipliers::ITEMultiplierAndZoneMultiplier, plates[1].followMultipliers);
    EXPECT_EQ(1.0, plates[1].plateCount());
    EXPECT_EQ(3.0, plates[2].plateCount());
    EXPECT_EQ(3.0, plates[2].plateZoneMultiplier);
}

} // namespace EnergyPlus
