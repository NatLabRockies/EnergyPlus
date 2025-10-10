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

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/Coils/CoilCoolingDXCurveFitOperatingMode.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/OutputReportTabular.hh>

#include "../Coils/CoilCoolingDXFixture.hh"

using namespace EnergyPlus;

TEST_F(CoilCoolingDXTest, CoilCoolingDXCurveFitModeInput)
{
    std::string idf_objects = this->getModeObjectString("mode1", 2); // What is going on here?
    EXPECT_TRUE(process_idf(idf_objects, false));
    state->init_state(*state);
    CoilCoolingDXCurveFitOperatingMode thisMode(*state, "mode1");
    EXPECT_EQ("MODE1", thisMode.name);
    EXPECT_EQ("MODE1SPEED1", thisMode.speeds[0].name);
}

TEST_F(CoilCoolingDXTest, CoilCoolingDXCurveFitOperatingMode_Sizing)
{

    state->dataSQLiteProcedures->sqlite->createSQLiteSimulationsRecord(1, "EnergyPlus Version", "Current Time");

    std::string idf_objects = delimited_string({

        "Coil:Cooling:DX,",
        "  Coil Cooling DX 1,                      !- Name",
        "  Air Loop HVAC Unitary System 5 Fan - Cooling Coil Node, !- Evaporator Inlet Node Name",
        "  Air Loop HVAC Unitary System 5 Cooling Coil - Heating Coil Node, !- Evaporator Outlet Node Name",
        "  Always On Discrete,                     !- Availability Schedule Name",
        "  ,                                       !- Condenser Zone Name",
        "  Coil Cooling DX 1 Condenser Inlet Node, !- Condenser Inlet Node Name",
        "  Coil Cooling DX 1 Condenser Outlet Node, !- Condenser Outlet Node Name",
        "  Coil Cooling DX Curve Fit Performance 1; !- Performance Object Name",
        "",
        "Coil:Cooling:DX:CurveFit:Performance,",
        "  Coil Cooling DX Curve Fit Performance 1, !- Name",
        "  0,                                      !- Crankcase Heater Capacity {W}",
        "  ,                                       !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "  -25,                                    !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "  10,                                     !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "  773.3,                                  !- Unit Internal Static Air Pressure {Pa}",
        "  Discrete,                               !- Capacity Control Method",
        "  0,                                      !- Evaporative Condenser Basin Heater Capacity {W/K}",
        "  2,                                      !- Evaporative Condenser Basin Heater Setpoint Temperature {C}",
        "  Always On Discrete,                     !- Evaporative Condenser Basin Heater Operating Schedule Name",
        "  Electricity,                            !- Compressor Fuel Type",
        "  Coil Cooling DX Curve Fit Operating Mode 1; !- Base Operating Mode",

        "Coil:Cooling:DX:CurveFit:OperatingMode,",
        "  Coil Cooling DX Curve Fit Operating Mode 1, !- Name",
        "  Autosize,                               !- Rated Gross Total Cooling Capacity {W}",
        "  Autosize,                                    !- Rated Evaporator Air Flow Rate {m3/s}",
        "  Autosize,                               !- Rated Condenser Air Flow Rate {m3/s}",
        "  0,                                      !- Maximum Cycling Rate {cycles/hr}",
        "  0,                                      !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "  0,                                      !- Latent Capacity Time Constant {s}",
        "  0,                                      !- Nominal Time for Condensate Removal to Begin {s}",
        "  No,                                     !- Apply Latent Degradation to Speeds Greater than 1",
        "  EvaporativelyCooled,                    !- Condenser Type",
        "  Autosize,                               !- Nominal Evaporative Condenser Pump Power {W}",
        "  1,                                      !- Nominal Speed Number",
        "  Coil Cooling DX Curve Fit Speed 1;      !- Speed Name 1",
    });
    idf_objects += this->getSpeedObjectString("Coil Cooling DX Curve Fit Speed 1");
    EXPECT_TRUE(process_idf(idf_objects, false));
    state->init_state(*state);

    CoilCoolingDXCurveFitOperatingMode thisMode(*state, "Coil Cooling DX Curve Fit Operating Mode 1");
    EXPECT_ENUM_EQ(CoilCoolingDXCurveFitOperatingMode::CondenserType::EVAPCOOLED, thisMode.condenserType);
    EXPECT_EQ(DataSizing::AutoSize, thisMode.ratedEvapAirFlowRate);
    EXPECT_EQ(DataSizing::AutoSize, thisMode.ratedGrossTotalCap);
    EXPECT_EQ(DataSizing::AutoSize, thisMode.ratedCondAirFlowRate);
    EXPECT_EQ(DataSizing::AutoSize, thisMode.nominalEvaporativePumpPower);

    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->SysSizPeakDDNum.allocate(1);

    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataSize->CurZoneEqNum = 1;
    state->dataEnvrn->StdRhoAir = 1.0; // Prevent divide by zero in ReportSizingManager
    state->dataEnvrn->StdBaroPress = 101325.0;

    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesignSizeFromParent = false;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(25);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod(HVAC::SystemAirflowSizing) = DataSizing::SupplyAirFlowRate;

    Real64 ratedEvapAirFlowRate = 1.005;

    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = ratedEvapAirFlowRate;

    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 30.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInHumRat = 0.001;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesTemp = 15.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesHumRat = 0.0006;

    thisMode.size(*state);

    EXPECT_EQ(ratedEvapAirFlowRate, thisMode.ratedEvapAirFlowRate);
    Real64 ratedGrossTotalCap = thisMode.ratedGrossTotalCap;
    EXPECT_EQ(ratedGrossTotalCap, thisMode.ratedGrossTotalCap);
    // Total Capacity * 0.000114 m3/s/w (850 cfm/ton)
    Real64 ratedCondAirFlowRate = 0.000114 * ratedGrossTotalCap;
    EXPECT_EQ(ratedCondAirFlowRate, thisMode.ratedCondAirFlowRate);
    // Total Capacity * 0.004266 w/w (15 W/ton)
    Real64 nominalEvaporativePumpPower = 0.004266 * ratedGrossTotalCap;
    EXPECT_EQ(nominalEvaporativePumpPower, thisMode.nominalEvaporativePumpPower);

    // Now check output tables to ensure that we also get the right units etc
    const std::string compType = "Coil:Cooling:DX:CurveFit:OperatingMode";
    const std::string compName = thisMode.name;
    EXPECT_EQ(compName, "COIL COOLING DX CURVE FIT OPERATING MODE 1");

    struct TestQuery
    {
        // clang-format off
        TestQuery(std::string t_description, std::string t_units, Real64 t_value)
            : description(t_description), units(t_units), expectedValue(t_value),
              displayString("Description='" + description + "'; Units='" + units + "'") {};
        // clang-format on

        const std::string description;
        const std::string units;
        const Real64 expectedValue;
        const std::string displayString;
    };

    std::vector<TestQuery> testQueries({
        TestQuery("Design Size Rated Evaporator Air Flow Rate", "m3/s", ratedEvapAirFlowRate),
        TestQuery("Design Size Rated Gross Total Cooling Capacity", "W", ratedGrossTotalCap),
        TestQuery("Design Size Rated Condenser Air Flow Rate", "m3/s", ratedCondAirFlowRate),
        TestQuery("Design Size Nominal Evaporative Condenser Pump Power", "W", nominalEvaporativePumpPower),
    });

    for (auto &testQuery : testQueries) {

        std::string query("SELECT Value From ComponentSizes"
                          "  WHERE CompType = '" +
                          compType +
                          "'"
                          "  AND CompName = '" +
                          compName +
                          "'"
                          "  AND Description = '" +
                          testQuery.description + "'" + "  AND Units = '" + testQuery.units + "'");

        // execAndReturnFirstDouble returns -10000.0 if not found
        Real64 return_val = SQLiteFixture::execAndReturnFirstDouble(query);

        if (return_val < 0) {
            EXPECT_TRUE(false) << "Query returned nothing for " << testQuery.displayString;
        } else {
            EXPECT_NEAR(testQuery.expectedValue, return_val, 0.01) << "Failed for " << testQuery.displayString;
        }
    }
}

TEST_F(CoilCoolingDXTest, CoilCoolingDXCurveFitCrankcaseHeaterCurve)
{
    std::string idf_objects = delimited_string(
        {"Coil:Cooling:DX,",
         "  Coil Cooling DX 1,                      !- Name",
         "  Air Loop HVAC Unitary System 5 Fan - Cooling Coil Node, !- Evaporator Inlet Node Name",
         "  Air Loop HVAC Unitary System 5 Cooling Coil - Heating Coil Node, !- Evaporator Outlet Node Name",
         "  ,                                       !- Availability Schedule Name",
         "  ,                                       !- Condenser Zone Name",
         "  Coil Cooling DX 1 Condenser Inlet Node, !- Condenser Inlet Node Name",
         "  Coil Cooling DX 1 Condenser Outlet Node, !- Condenser Outlet Node Name",
         "  Coil Cooling DX Curve Fit Performance 1; !- Performance Object Name",

         "Coil:Cooling:DX:CurveFit:Performance,",
         "  Coil Cooling DX Curve Fit Performance 1, !- Name",
         "  10,                                      !- Crankcase Heater Capacity {W}",
         "heaterCapCurve,                           !- Crankcase Heater Capacity Function of Outdoor Temperature Curve Name",
         "  -25,                                    !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
         "  10,                                     !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
         "  773.3,                                  !- Unit Internal Static Air Pressure {Pa}",
         "  Discrete,                               !- Capacity Control Method",
         "  0,                                      !- Evaporative Condenser Basin Heater Capacity {W/K}",
         "  2,                                      !- Evaporative Condenser Basin Heater Setpoint Temperature {C}",
         "  ,                                       !- Evaporative Condenser Basin Heater Operating Schedule Name",
         "  Electricity,                            !- Compressor Fuel Type",
         "  Coil Cooling DX Curve Fit Operating Mode 1, !- Base Operating Mode",
         ",",
         ";",

         "Curve:Linear,",
         "heaterCapCurve,          !- Name",
         "10.0,                    !- Coefficient1 Constant",
         "2.,                      !- Coefficient2 x",
         "-10.0,                    !- Minimum Value of x",
         "70;                      !- Maximum Value of x",

         "Coil:Cooling:DX:CurveFit:OperatingMode,",
         "  Coil Cooling DX Curve Fit Operating Mode 1, !- Name",
         "  Autosize,                               !- Rated Gross Total Cooling Capacity {W}",
         "  Autosize,                                    !- Rated Evaporator Air Flow Rate {m3/s}",
         "  Autosize,                               !- Rated Condenser Air Flow Rate {m3/s}",
         "  0,                                      !- Maximum Cycling Rate {cycles/hr}",
         "  0,                                      !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
         "  0,                                      !- Latent Capacity Time Constant {s}",
         "  0,                                      !- Nominal Time for Condensate Removal to Begin {s}",
         "  No,                                     !- Apply Latent Degradation to Speeds Greater than 1",
         "  EvaporativelyCooled,                    !- Condenser Type",
         "  Autosize,                               !- Nominal Evaporative Condenser Pump Power {W}",
         "  1,                                      !- Nominal Speed Number",
         "  Coil Cooling DX Curve Fit Speed 1;      !- Speed Name 1"});

    idf_objects += this->getSpeedObjectString("Coil Cooling DX Curve Fit Speed 1");
    EXPECT_TRUE(process_idf(idf_objects, false));
    state->init_state(*state);

    int coilIndex = CoilCoolingDX::factory(*state, "Coil Cooling DX 1");
    auto &thisCoil(state->dataCoilCoolingDX->coilCoolingDXs[coilIndex]);
    EXPECT_EQ("COIL COOLING DX 1", thisCoil.name);
    EXPECT_EQ("COIL COOLING DX CURVE FIT PERFORMANCE 1", thisCoil.performance->name);

    HVAC::CoilMode coilMode = HVAC::CoilMode::Normal;
    Real64 PLR = 1.0;
    int speedNum = 1;
    Real64 speedRatio = 1.0;
    HVAC::FanOp fanOp = HVAC::FanOp::Cycling;
    bool singleMode = false;
    state->dataEnvrn->OutDryBulbTemp = 1.0;
    // thisCoil.simulate(*state, useAlternateMode, PLR, speedNum, speedRatio, fanOpMode, singleMode);
    auto &evapInletNode = state->dataLoopNodes->Node(thisCoil.evapInletNodeIndex);
    auto &evapOutletNode = state->dataLoopNodes->Node(thisCoil.evapOutletNodeIndex);
    auto &condInletNode = state->dataLoopNodes->Node(thisCoil.condInletNodeIndex);
    auto &condOutletNode = state->dataLoopNodes->Node(thisCoil.condOutletNodeIndex);
    Real64 LoadSHR = 0.0;
    thisCoil.performance->simulate(
        *state, evapInletNode, evapOutletNode, coilMode, speedNum, speedRatio, fanOp, condInletNode, condOutletNode, singleMode, LoadSHR);
    EXPECT_EQ(thisCoil.performance->crankcaseHeaterPower, 120.0);
}
