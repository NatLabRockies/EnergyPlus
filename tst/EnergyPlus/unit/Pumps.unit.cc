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

// EnergyPlus::Pumps Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Pumps.hh>
#include <EnergyPlus/SizingManager.hh>

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, HeaderedVariableSpeedPumpSizingPowerTest)
{
    std::string const idf_objects = delimited_string({
        "HeaderedPumps:VariableSpeed,",
        "Chilled Water Headered Pumps,  !- Name",
        "CW Supply Inlet Node,    !- Inlet Node Name",
        "CW Pumps Outlet Node,    !- Outlet Node Name",
        "0.001,                   !- Total Design Flow Rate {m3/s}",
        "2,                       !- Number of Pumps in Bank",
        "SEQUENTIAL,              !- Flow Sequencing Control Scheme",
        "100000,                  !- Design Pump Head {Pa}",
        "autosize,                !- Design Power Consumption {W}",
        "0.8,                     !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "0.1,                     !- Minimum Flow Rate Fraction",
        "INTERMITTENT,            !- Pump Control Type",
        "CoolingPumpAvailSched,   !- Pump Flow Rate Schedule Name",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        "PowerPerFlowPerPressure, !- Design Power Sizing Method",
        ",                        !- Design Electric Power per Unit Flow Rate",
        "1.3,                     !- Design Shaft Power per Unit Flow Rate per Unit Head",
        "Pump Energy;             !- End-Use Subcategory",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 162.5, 0.0001);
    EXPECT_EQ(state->dataPumps->PumpEquip(1).EndUseSubcategoryName, "Pump Energy");
}

TEST_F(EnergyPlusFixture, HeaderedVariableSpeedPumpSizingPower22W_per_gpm)
{
    std::string const idf_objects = delimited_string({
        "HeaderedPumps:VariableSpeed,",
        "Chilled Water Headered Pumps,  !- Name",
        "CW Supply Inlet Node,    !- Inlet Node Name",
        "CW Pumps Outlet Node,    !- Outlet Node Name",
        "0.001,                   !- Total Design Flow Rate {m3/s}",
        "2,                       !- Number of Pumps in Bank",
        "SEQUENTIAL,              !- Flow Sequencing Control Scheme",
        "100000,                  !- Design Pump Head {Pa}",
        "autosize,                !- Design Power Consumption {W}",
        "0.8,                     !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "0.1,                     !- Minimum Flow Rate Fraction",
        "INTERMITTENT,            !- Pump Control Type",
        "CoolingPumpAvailSched,   !- Pump Flow Rate Schedule Name",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        "PowerPerFlow,            !- Design Power Sizing Method",
        ",                        !- Design Electric Power per Unit Flow Rate",
        ";                        !- Design Shaft Power per Unit Flow Rate per Unit Head",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 348.7011, 0.0001);
}

TEST_F(EnergyPlusFixture, HeaderedVariableSpeedPumpSizingPowerDefault)
{
    std::string const idf_objects = delimited_string({
        "HeaderedPumps:VariableSpeed,",
        "Chilled Water Headered Pumps,  !- Name",
        "CW Supply Inlet Node,    !- Inlet Node Name",
        "CW Pumps Outlet Node,    !- Outlet Node Name",
        "0.001,                   !- Total Design Flow Rate {m3/s}",
        "2,                       !- Number of Pumps in Bank",
        "SEQUENTIAL,              !- Flow Sequencing Control Scheme",
        ",                        !- Design Pump Head {Pa}",
        "autosize,                !- Design Power Consumption {W}",
        ",                        !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "0.1,                     !- Minimum Flow Rate Fraction",
        "INTERMITTENT,            !- Pump Control Type",
        "CoolingPumpAvailSched,   !- Pump Flow Rate Schedule Name",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        ",                        !- Design Power Sizing Method",
        ",                        !- Design Electric Power per Unit Flow Rate",
        ";                        !- Design Shaft Power per Unit Flow Rate per Unit Head",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 255.4872, 0.0001);
}

TEST_F(EnergyPlusFixture, HeaderedConstantSpeedPumpSizingPowerTest)
{
    std::string const idf_objects = delimited_string({
        "HeaderedPumps:ConstantSpeed,",
        "Chilled Water Headered Pumps,  !- Name",
        "CW Supply Inlet Node,    !- Inlet Node Name",
        "CW Pumps Outlet Node,    !- Outlet Node Name",
        "0.001,                   !- Total Design Flow Rate {m3/s}",
        "2,                       !- Number of Pumps in Bank",
        "SEQUENTIAL,              !- Flow Sequencing Control Scheme",
        "100000,                  !- Design Pump Head {Pa}",
        "autosize,                !- Design Power Consumption {W}",
        "0.8,                     !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "INTERMITTENT,            !- Pump Control Type",
        "CoolingPumpAvailSched,   !- Pump Flow Rate Schedule Name",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        "PowerPerFlowPerPressure, !- Design Power Sizing Method",
        ",                        !- Design Electric Power per Unit Flow Rate",
        "1.3,                     !- Design Shaft Power per Unit Flow Rate per Unit Head",
        "Pump Energy;             !- End-Use Subcategory",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 162.5, 0.0001);
    EXPECT_EQ(state->dataPumps->PumpEquip(1).EndUseSubcategoryName, "Pump Energy");
}

TEST_F(EnergyPlusFixture, HeaderedConstantSpeedPumpSizingPower19W_per_gpm)
{
    std::string const idf_objects = delimited_string({
        "HeaderedPumps:ConstantSpeed,",
        "Chilled Water Headered Pumps,  !- Name",
        "CW Supply Inlet Node,    !- Inlet Node Name",
        "CW Pumps Outlet Node,    !- Outlet Node Name",
        "0.001,                   !- Total Design Flow Rate {m3/s}",
        "2,                       !- Number of Pumps in Bank",
        "SEQUENTIAL,              !- Flow Sequencing Control Scheme",
        ",                        !- Design Pump Head {Pa}",
        "autosize,                !- Design Power Consumption {W}",
        ",                        !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "INTERMITTENT,            !- Pump Control Type",
        "CoolingPumpAvailSched,   !- Pump Flow Rate Schedule Name",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        "PowerPerFlow,            !- Design Power Sizing Method",
        "301156.1,                !- Design Electric Power per Unit Flow Rate",
        ";                        !- Design Shaft Power per Unit Flow Rate per Unit Head",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 301.1561, 0.0001);
}

TEST_F(EnergyPlusFixture, HeaderedConstantSpeedPumpSizingPowerDefault)
{
    std::string const idf_objects = delimited_string({
        "HeaderedPumps:ConstantSpeed,",
        "Chilled Water Headered Pumps,  !- Name",
        "CW Supply Inlet Node,    !- Inlet Node Name",
        "CW Pumps Outlet Node,    !- Outlet Node Name",
        "0.001,                   !- Total Design Flow Rate {m3/s}",
        "2,                       !- Number of Pumps in Bank",
        "SEQUENTIAL,              !- Flow Sequencing Control Scheme",
        ",                        !- Design Pump Head {Pa}",
        "autosize,                !- Design Power Consumption {W}",
        ",                        !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "INTERMITTENT,            !- Pump Control Type",
        "CoolingPumpAvailSched,   !- Pump Flow Rate Schedule Name",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        ",                        !- Design Power Sizing Method",
        ",                        !- Design Electric Power per Unit Flow Rate",
        ";                        !- Design Shaft Power per Unit Flow Rate per Unit Head",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 255.4872, 0.0001);
}

TEST_F(EnergyPlusFixture, VariableSpeedPumpSizingMinVolFlowRate)
{
    std::string const idf_objects = delimited_string({
        "Pump:VariableSpeed,",
        "CoolSys1 Pump,           !- Name",
        "CoolSys1 Supply Inlet Node,  !- Inlet Node Name",
        "CoolSys1 Pump-CoolSys1 ChillerNodeviaConnector,  !- Outlet Node Name",
        "0.001,                !- Design Flow Rate {m3/s}",
        "100000,                  !- Design Pump Head {Pa}",
        "AUTOSIZE,                !- Design Power Consumption {W}",
        "0.8,                     !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "autosize,                !- Minimum Flow Rate {m3/s}",
        "Intermittent,            !- Pump Control Type",
        ",                        !- Pump Flow Rate Schedule Name",
        ",                        !- Pump Curve Name",
        ",                        !- Impeller Diameter",
        ",                        !- VFD Control Type",
        ",                        !- Pump rpm Schedule Name",
        ",                        !- Minimum Pressure Schedule",
        ",                        !- Maximum Pressure Schedule",
        ",                        !- Minimum RPM Schedule",
        ",                        !- Maximum RPM Schedule",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        "PowerPerFlowPerPressure, !- Design Power Sizing Method",
        ",                        !- Design Electric Power per Unit Flow Rate",
        "1.3,                     !- Design Shaft Power per Unit Flow Rate per Unit Head",
        "0.3,                     !- Design Minimum Flow Rate Sizing Factor",
        "Pump Energy;             !- End-Use Subcategory",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).MinVolFlowRate, DataSizing::AutoSize, 0.000001);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).MinVolFlowRate, 0.0003, 0.00001);
    EXPECT_EQ(state->dataPumps->PumpEquip(1).EndUseSubcategoryName, "Pump Energy");
}

TEST_F(EnergyPlusFixture, VariableSpeedPumpSizingPowerPerPressureTest)
{
    std::string const idf_objects = delimited_string({
        "Pump:VariableSpeed,",
        "CoolSys1 Pump,           !- Name",
        "CoolSys1 Supply Inlet Node,  !- Inlet Node Name",
        "CoolSys1 Pump-CoolSys1 ChillerNodeviaConnector,  !- Outlet Node Name",
        "0.001,                !- Design Flow Rate {m3/s}",
        "100000,                  !- Design Pump Head {Pa}",
        "AUTOSIZE,                !- Design Power Consumption {W}",
        "0.8,                     !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "autosize,                !- Minimum Flow Rate {m3/s}",
        "Intermittent,            !- Pump Control Type",
        ",                        !- Pump Flow Rate Schedule Name",
        ",                        !- Pump Curve Name",
        ",                        !- Impeller Diameter",
        ",                        !- VFD Control Type",
        ",                        !- Pump rpm Schedule Name",
        ",                        !- Minimum Pressure Schedule",
        ",                        !- Maximum Pressure Schedule",
        ",                        !- Minimum RPM Schedule",
        ",                        !- Maximum RPM Schedule",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        "PowerPerFlowPerPressure, !- Design Power Sizing Method",
        ",                        !- Design Electric Power per Unit Flow Rate",
        "1.3,                     !- Design Shaft Power per Unit Flow Rate per Unit Head",
        ";                        !- Design Minimum Flow Rate Sizing Factor",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 162.5, 0.0001);
}

TEST_F(EnergyPlusFixture, VariableSpeedPumpSizingPowerDefault)
{
    std::string const idf_objects = delimited_string({
        "Pump:VariableSpeed,",
        "CoolSys1 Pump,           !- Name",
        "CoolSys1 Supply Inlet Node,  !- Inlet Node Name",
        "CoolSys1 Pump-CoolSys1 ChillerNodeviaConnector,  !- Outlet Node Name",
        "0.001,                   !- Design Flow Rate {m3/s}",
        ",                        !- Design Pump Head {Pa}",
        "AUTOSIZE,                !- Design Power Consumption {W}",
        ",                        !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "autosize,                !- Minimum Flow Rate {m3/s}",
        "Intermittent,            !- Pump Control Type",
        ",                        !- Pump Flow Rate Schedule Name",
        ",                        !- Pump Curve Name",
        ",                        !- Impeller Diameter",
        ",                        !- VFD Control Type",
        ",                        !- Pump rpm Schedule Name",
        ",                        !- Minimum Pressure Schedule",
        ",                        !- Maximum Pressure Schedule",
        ",                        !- Minimum RPM Schedule",
        ",                        !- Maximum RPM Schedule",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        ",                        !- Design Power Sizing Method",
        ",                        !- Design Electric Power per Unit Flow Rate",
        ",                        !- Design Shaft Power per Unit Flow Rate per Unit Head",
        ";                        !- Design Minimum Flow Rate Sizing Factor",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 255.4872, 0.0001);
}

TEST_F(EnergyPlusFixture, VariableSpeedPumpSizingPower22W_per_GPM)
{
    std::string const idf_objects = delimited_string({
        "Pump:VariableSpeed,",
        "CoolSys1 Pump,           !- Name",
        "CoolSys1 Supply Inlet Node,  !- Inlet Node Name",
        "CoolSys1 Pump-CoolSys1 ChillerNodeviaConnector,  !- Outlet Node Name",
        "0.001,                   !- Design Flow Rate {m3/s}",
        "179352,                  !- Design Pump Head {Pa}",
        "AUTOSIZE,                !- Design Power Consumption {W}",
        "0.9,                     !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "autosize,                !- Minimum Flow Rate {m3/s}",
        "Intermittent,            !- Pump Control Type",
        ",                        !- Pump Flow Rate Schedule Name",
        ",                        !- Pump Curve Name",
        ",                        !- Impeller Diameter",
        ",                        !- VFD Control Type",
        ",                        !- Pump rpm Schedule Name",
        ",                        !- Minimum Pressure Schedule",
        ",                        !- Maximum Pressure Schedule",
        ",                        !- Minimum RPM Schedule",
        ",                        !- Maximum RPM Schedule",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        "PowerPerFlow,            !- Design Power Sizing Method",
        ",                        !- Design Electric Power per Unit Flow Rate",
        ",                        !- Design Shaft Power per Unit Flow Rate per Unit Head",
        "0.0;                     !- Design Minimum Flow Rate Sizing Factor",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 348.7011, 0.0001);
}

TEST_F(EnergyPlusFixture, ConstantSpeedPumpSizingPower19W_per_gpm)
{
    std::string const idf_objects = delimited_string({
        "Pump:ConstantSpeed,",
        "TowerWaterSys Pump,      !- Name",
        "TowerWaterSys Supply Inlet Node,  !- Inlet Node Name",
        "TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector,  !- Outlet Node Name",
        "0.001,                   !- Design Flow Rate {m3/s}",
        "179352,                  !- Design Pump Head {Pa}",
        "AUTOSIZE,                !- Design Power Consumption {W}",
        "0.87,                    !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "Intermittent,            !- Pump Control Type",
        ",                        !- Pump Flow Rate Schedule Name",
        ",                        !- Pump Curve Name",
        ",                        !- Impeller Diameter",
        ",                        !- Rotational Speed",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        "PowerPerFlow,            !- Design Power Sizing Method",
        "301156.1,                !- Design Electric Power per Unit Flow Rate",
        ",                        !- Design Shaft Power per Unit Flow Rate per Unit Head",
        "Pump Energy;             !- End-Use Subcategory",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 301.1561, 0.0001);
    EXPECT_EQ(state->dataPumps->PumpEquip(1).EndUseSubcategoryName, "Pump Energy");
}

TEST_F(EnergyPlusFixture, ConstantSpeedPumpSizingPowerPerPressureTest)
{
    std::string const idf_objects = delimited_string({
        "Pump:ConstantSpeed,",
        "TowerWaterSys Pump,      !- Name",
        "TowerWaterSys Supply Inlet Node,  !- Inlet Node Name",
        "TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector,  !- Outlet Node Name",
        "0.001,                   !- Design Flow Rate {m3/s}",
        "100000,                  !- Design Pump Head {Pa}",
        "AUTOSIZE,                !- Design Power Consumption {W}",
        "0.8,                     !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "Intermittent,            !- Pump Control Type",
        ",                        !- Pump Flow Rate Schedule Name",
        ",                        !- Pump Curve Name",
        ",                        !- Impeller Diameter",
        ",                        !- Rotational Speed",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        "PowerPerFlowPerPressure, !- Design Power Sizing Method",
        ",                        !- Design Electric Power per Unit Flow Rate",
        "1.3;                     !- Design Shaft Power per Unit Flow Rate per Unit Head",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 162.5, 0.0001);
}

TEST_F(EnergyPlusFixture, ConstantSpeedPumpSizingPowerDefaults)
{
    std::string const idf_objects = delimited_string({
        "Pump:ConstantSpeed,",
        "TowerWaterSys Pump,      !- Name",
        "TowerWaterSys Supply Inlet Node,  !- Inlet Node Name",
        "TowerWaterSys Pump-TowerWaterSys CoolTowerNodeviaConnector,  !- Outlet Node Name",
        "0.001,                   !- Design Flow Rate {m3/s}",
        ",                        !- Design Pump Head {Pa}",
        "AUTOSIZE,                !- Design Power Consumption {W}",
        ",                        !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "Intermittent,            !- Pump Control Type",
        ",                        !- Pump Flow Rate Schedule Name",
        ",                        !- Pump Curve Name",
        ",                        !- Impeller Diameter",
        ",                        !- Rotational Speed",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        ",                        !- Design Power Sizing Method",
        ",                        !- Design Electric Power per Unit Flow Rate",
        ";                        !- Design Shaft Power per Unit Flow Rate per Unit Head",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 255.4872, 0.0001);
}

TEST_F(EnergyPlusFixture, CondensatePumpSizingPowerDefaults)
{
    std::string const idf_objects = delimited_string({
        "Pump:VariableSpeed:Condensate,",
        "Steam Boiler Plant Steam Circ Pump,  !- Name",
        "Steam Boiler Plant Steam Supply Inlet Node,  !- Inlet Node Name",
        "Steam Boiler Plant Steam Pump Outlet Node,  !- Outlet Node Name",
        "1.0,                     !- Design Flow Rate {m3/s}",
        ",                        !- Design Pump Head {Pa}",
        "autosize,                !- Design Power Consumption {W}",
        ",                        !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "0,                       !- Coefficient 4 of the Part Load Performance Curve",
        ",                        !- Pump Flow Rate Schedule Name",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        ",                        !- Design Power Sizing Method",
        ",                        !- Design Electric Power per Unit Flow Rate",
        ",                        !- Design Shaft Power per Unit Flow Rate per Unit Head",
        "Pump Energy;             !- End-Use Subcategory",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 153.3, 0.1);
    EXPECT_EQ(state->dataPumps->PumpEquip(1).EndUseSubcategoryName, "Pump Energy");
}

TEST_F(EnergyPlusFixture, CondensatePumpSizingPower19W_per_gpm)
{
    std::string const idf_objects = delimited_string({
        "Pump:VariableSpeed:Condensate,",
        "Steam Boiler Plant Steam Circ Pump,  !- Name",
        "Steam Boiler Plant Steam Supply Inlet Node,  !- Inlet Node Name",
        "Steam Boiler Plant Steam Pump Outlet Node,  !- Outlet Node Name",
        "1.0,                     !- Design Flow Rate {m3/s}",
        "179352,                  !- Design Pump Head {Pa}",
        "autosize,                !- Design Power Consumption {W}",
        "0.9,                     !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "0,                       !- Coefficient 4 of the Part Load Performance Curve",
        ",                        !- Pump Flow Rate Schedule Name",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        "PowerPerFlow,            !- Design Power Sizing Method",
        "301156.1,                !- Design Electric Power per Unit Flow Rate",
        ";                        !- Design Shaft Power per Unit Flow Rate per Unit Head",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 180.7, 0.1);
}

TEST_F(EnergyPlusFixture, CondensatePumpSizingPowerTest)
{
    std::string const idf_objects = delimited_string({
        "Pump:VariableSpeed:Condensate,",
        "Steam Boiler Plant Steam Circ Pump,  !- Name",
        "Steam Boiler Plant Steam Supply Inlet Node,  !- Inlet Node Name",
        "Steam Boiler Plant Steam Pump Outlet Node,  !- Outlet Node Name",
        "1.0,                     !- Design Flow Rate {m3/s}",
        "100000,                  !- Design Pump Head {Pa}",
        "autosize,                !- Design Power Consumption {W}",
        "0.8,                     !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "0,                       !- Coefficient 4 of the Part Load Performance Curve",
        ",                        !- Pump Flow Rate Schedule Name",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        "PowerPerFlowPerPressure, !- Design Power Sizing Method",
        ",                        !- Design Electric Power per Unit Flow Rate",
        "1.3;                     !- Design Shaft Power per Unit Flow Rate per Unit Head",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomPowerUse, 97.5, 0.1);
}

// Test for https://github.com/NREL/EnergyPlus/issues/6164
// The 'Design Minimum Flow Rate' is over the 'Design Maximum Flow Rate'
TEST_F(EnergyPlusFixture, VariableSpeedPump_MinFlowGreaterThanMax)
{
    std::string const idf_objects = delimited_string({
        "Pump:VariableSpeed,",
        "  supply inlet pump,       !- Name",
        "  Node supply inlet in,    !- Inlet Node Name",
        "  Node supply inlet out,   !- Outlet Node Name",
        "  0.001,                   !- Design Maximum Flow Rate {m3/s}",
        "  1793520,                 !- Design Pump Head {Pa}",
        "  2237,                    !- Design Power Consumption {W}",
        "  0.9,                     !- Motor Efficiency",
        "  ,                        !- Fraction of Motor Inefficiencies to Fluid Stream",
        "  ,                        !- Coefficient 1 of the Part Load Performance Curve",
        "  1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "  ,                        !- Coefficient 3 of the Part Load Performance Curve",
        "  ,                        !- Coefficient 4 of the Part Load Performance Curve",
        "  0.002,                   !- Design Minimum Flow Rate {m3/s}",
        "  Continuous,              !- Pump Control Type",
        "  ,                        !- Pump Flow Rate Schedule Name",
        "  ,                        !- Pump Curve Name",
        "  ,                        !- Impeller Diameter {m}",
        "  ,                        !- VFD Control Type",
        "  ,                        !- Pump rpm Schedule Name",
        "  ,                        !- Minimum Pressure Schedule",
        "  ,                        !- Maximum Pressure Schedule",
        "  ,                        !- Minimum RPM Schedule",
        "  ,                        !- Maximum RPM Schedule",
        "  ,                        !- Zone Name",
        "  ,                        !- Skin Loss Radiative Fraction",
        "  PowerPerFlowPerPressure, !- Design Power Sizing Method",
        "  348701.1,                !- Design Electric Power per Unit Flow Rate {W/(m3/s)}",
        "  1.282051282,             !- Design Shaft Power per Unit Flow Rate per Unit Head {W/((m3/s)-Pa)}",
        "  ;                        !- Design Minimum Flow Rate Fraction",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);

    std::string const error_string = delimited_string({
        "   ** Warning ** GetPumpInput: Pump:VariableSpeed=\"SUPPLY INLET PUMP\", Invalid 'Design Minimum Flow Rate'",
        "   **   ~~~   ** Entered Value=[2.00000E-003] is above or too close (equal) to the Design Maximum Flow Rate=[1.00000E-003].",
        "   **   ~~~   ** Reseting value of 'Design Minimum Flow Rate' to the value of 99% of 'Design Maximum Flow Rate'.",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));

    // Should have reset the min to 99% of the the max
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).MinVolFlowRate, 0.001 * 0.99, 0.00001);
}

// Test for https://github.com/NREL/EnergyPlus/issues/9427
// The 'Design Minimum Flow Rate' is equal to the 'Design Maximum Flow Rate' or greater than 99% of max
TEST_F(EnergyPlusFixture, VariableSpeedPump_MinFlowEqualToMax)
{
    std::string const idf_objects = delimited_string({
        "Pump:VariableSpeed,",
        "  supply inlet pump,       !- Name",
        "  Node supply inlet in,    !- Inlet Node Name",
        "  Node supply inlet out,   !- Outlet Node Name",
        "  0.001,                   !- Design Maximum Flow Rate {m3/s}",
        "  1793520,                 !- Design Pump Head {Pa}",
        "  2237,                    !- Design Power Consumption {W}",
        "  0.9,                     !- Motor Efficiency",
        "  ,                        !- Fraction of Motor Inefficiencies to Fluid Stream",
        "  ,                        !- Coefficient 1 of the Part Load Performance Curve",
        "  1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "  ,                        !- Coefficient 3 of the Part Load Performance Curve",
        "  ,                        !- Coefficient 4 of the Part Load Performance Curve",
        "  0.000995,                !- Design Minimum Flow Rate {m3/s}",
        "  Intermittent,            !- Pump Control Type",
        "  ,                        !- Pump Flow Rate Schedule Name",
        "  ,                        !- Pump Curve Name",
        "  ,                        !- Impeller Diameter {m}",
        "  ,                        !- VFD Control Type",
        "  ,                        !- Pump rpm Schedule Name",
        "  ,                        !- Minimum Pressure Schedule",
        "  ,                        !- Maximum Pressure Schedule",
        "  ,                        !- Minimum RPM Schedule",
        "  ,                        !- Maximum RPM Schedule",
        "  ,                        !- Zone Name",
        "  ,                        !- Skin Loss Radiative Fraction",
        "  PowerPerFlowPerPressure, !- Design Power Sizing Method",
        "  348701.1,                !- Design Electric Power per Unit Flow Rate {W/(m3/s)}",
        "  1.282051282,             !- Design Shaft Power per Unit Flow Rate per Unit Head {W/((m3/s)-Pa)}",
        "  ;                        !- Design Minimum Flow Rate Fraction",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    Pumps::GetPumpInput(*state);

    std::string const error_string = delimited_string({
        "   ** Warning ** GetPumpInput: Pump:VariableSpeed=\"SUPPLY INLET PUMP\", Invalid 'Design Minimum Flow Rate'",
        "   **   ~~~   ** Entered Value=[9.95000E-004] is above or too close (equal) to the Design Maximum Flow Rate=[1.00000E-003].",
        "   **   ~~~   ** Reseting value of 'Design Minimum Flow Rate' to the value of 99% of 'Design Maximum Flow Rate'.",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));

    // Should have reset the min to 99% of the the max, max not impacted
    Real64 expectedAnswer = 0.99 * 0.001;
    Real64 constexpr allowableTolerance = 0.00001;
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).MinVolFlowRate, expectedAnswer, allowableTolerance);
    expectedAnswer = 0.001;
    EXPECT_NEAR(state->dataPumps->PumpEquip(1).NomVolFlowRate, expectedAnswer, allowableTolerance);
}

TEST_F(EnergyPlusFixture, HeaderedVariableSpeedPumpEMSPressureTest)
{
    std::string const idf_objects = delimited_string({
        "HeaderedPumps:VariableSpeed,",
        "Chilled Water Headered Pumps,  !- Name",
        "CW Supply Inlet Node,    !- Inlet Node Name",
        "CW Pumps Outlet Node,    !- Outlet Node Name",
        "0.001,                   !- Total Design Flow Rate {m3/s}",
        "2,                       !- Number of Pumps in Bank",
        "SEQUENTIAL,              !- Flow Sequencing Control Scheme",
        "100000,                  !- Design Pump Head {Pa}",
        "autosize,                !- Design Power Consumption {W}",
        "0.8,                     !- Motor Efficiency",
        "0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "0.1,                     !- Minimum Flow Rate Fraction",
        "INTERMITTENT,            !- Pump Control Type",
        "CoolingPumpAvailSched,   !- Pump Flow Rate Schedule Name",
        ",                        !- Zone Name",
        ",                        !- Skin Loss Radiative Fraction",
        "PowerPerFlowPerPressure, !- Design Power Sizing Method",
        ",                        !- Design Electric Power per Unit Flow Rate",
        "1.3,                     !- Design Shaft Power per Unit Flow Rate per Unit Head",
        "Pump Energy;             !- End-Use Subcategory",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    int thisBranchNum = 1;
    DataPlant::LoopSideLocation thisLoopSideNum = DataPlant::LoopSideLocation::Supply;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(thisLoopSideNum).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(thisLoopSideNum).Branch(thisBranchNum).Comp.allocate(1);

    Pumps::GetPumpInput(*state);
    Pumps::SizePump(*state, 1);
    Real64 massflowrate = 1.0;
    state->dataPumps->PumpEquip(1).EMSPressureOverrideOn = true;
    state->dataPumps->PumpEquip(1).plantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;
    state->dataPumps->PumpEquip(1).plantLoc.loopNum = 1;
    state->dataPumps->PumpEquip(1).plantLoc.branchNum = 1;
    state->dataPumps->PumpEquip(1).plantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataPumps->PumpEquip(1).plantLoc);
    state->dataPumps->PumpEquip(1).MassFlowRateMax = massflowrate;
    bool PumpRunning = true;

    state->dataLoopNodes->Node(1).MassFlowRate = massflowrate;
    state->dataLoopNodes->Node(1).MassFlowRateMinAvail = massflowrate;
    state->dataLoopNodes->Node(1).MassFlowRateMin = massflowrate;
    state->dataLoopNodes->Node(1).MassFlowRateMax = massflowrate;
    state->dataLoopNodes->Node(2).MassFlowRateMaxAvail = massflowrate;
    state->dataLoopNodes->Node(1).MassFlowRateMaxAvail = massflowrate;

    state->dataPumps->PumpEquip(1).PumpEffic = 0.8;
    state->dataPumps->PumpEquip(1).EMSPressureOverrideValue = 200.0;

    Pumps::CalcPumps(*state, 1, massflowrate, PumpRunning);

    EXPECT_NEAR(state->dataPumps->PumpEquip(1).Power, 0.1563, 0.0001);
}

} // namespace EnergyPlus
