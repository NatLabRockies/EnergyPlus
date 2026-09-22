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

// EnergyPlus::Stand alone unit test of Issue4347; i.e., CalcHWBaseboard NTU-eff calculation

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/ScheduleManager.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, GetMaterialDataReadVarAbsorptance)
{
    std::string const idf_objects = delimited_string({
        "MaterialProperty:VariableAbsorptance,",
        "variableThermal_wall_1,  !- Name",
        "WALL_1,                  !- Reference Material Name",
        "SurfaceTemperature,      !- Control Signal",
        "THERMAL_ABSORPTANCE_TABLE, !- Thermal Absorptance Function Name",
        ",                        !- Thermal Absorptance Schedule Name",
        "SOLAR_ABSORPTANCE_CURVE, !- Solar Absorptance Function Name",
        ";                        !- Solar Absorptance Schedule Name",

        "MaterialProperty:VariableAbsorptance,",
        "variableSolar_wall_2,    !- Name",
        "WALL_2,                  !- Reference Material Name",
        "SurfaceReceivedSolarRadiation, !- Control Signal",
        ",                        !- Thermal Absorptance Function Name",
        ",                        !- Thermal Absorptance Schedule Name",
        "SOLAR_ABSORPTANCE_CURVE, !- Solar Absorptance Function Name",
        ";                        !- Solar Absorptance Schedule Name",

        "MaterialProperty:VariableAbsorptance,",
        "variableBoth_wall_3,     !- Name",
        "WALL_3,                  !- Reference Material Name",
        "Scheduled,               !- Control Signal",
        ",                        !- Thermal Absorptance Function Name",
        "ABS_SCH,                 !- Thermal Absorptance Schedule Name",
        ",                        !- Solar Absorptance Function Name",
        "ABS_SCH;                 !- Solar Absorptance Schedule Name",

        "MaterialProperty:VariableAbsorptance,",
        "variableBoth_wall_4,     !- Name",
        "WALL_4,                  !- Reference Material Name",
        "Scheduled,               !- Control Signal",
        ",                        !- Thermal Absorptance Function Name",
        "ABS_SCH,                 !- Thermal Absorptance Schedule Name",
        ",                        !- Solar Absorptance Function Name",
        "ABS_SCH,                 !- Solar Absorptance Schedule Name",
        "SurfaceTemperature,      !- Control Signal Inside Face",
        "THERMAL_ABSORPTANCE_TABLE, !- Thermal Absorptance Function Name Inside Face",
        ",                        !- Thermal Absorptance Schedule Name Inside Face",
        "SOLAR_ABSORPTANCE_CURVE, !- Solar Absorptance Function Name Inside Face",
        ";                        !- Solar Absorptance Schedule Name Inside Face",

        "ScheduleTypeLimits,",
        "  Fraction,                 !- Name",
        "  0,                        !- Lower Limit Value",
        "  1,                        !- Upper Limit Value",
        "  Continuous,               !- Numeric Type",
        "  Dimensionless;            !- Unit Type",

        "Schedule:Constant,",
        "    ABS_SCH,                    !- Name",
        "    Fraction,                   !- Schedule Type Limits Name",
        "    0.9;                        !- Hourly Value",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    auto &s_mat = state->dataMaterial;

    auto *mat1 = new Material::MaterialBase;
    mat1->Name = "WALL_1";
    mat1->group = Material::Group::Regular;
    s_mat->materials.push_back(mat1);
    mat1->Num = s_mat->materials.isize();
    s_mat->materialMap.insert_or_assign(mat1->Name, mat1->Num);

    auto *mat2 = new Material::MaterialBase;
    mat2->Name = "WALL_2";
    mat2->group = Material::Group::Regular;
    s_mat->materials.push_back(mat2);
    mat2->Num = s_mat->materials.isize();
    s_mat->materialMap.insert_or_assign(mat2->Name, mat2->Num);

    auto *mat3 = new Material::MaterialBase;
    mat3->Name = "WALL_3";
    mat3->group = Material::Group::Regular;
    s_mat->materials.push_back(mat3);
    mat3->Num = s_mat->materials.isize();
    s_mat->materialMap.insert_or_assign(mat3->Name, mat3->Num);

    auto *mat4 = new Material::MaterialBase;
    mat4->Name = "WALL_4";
    mat4->group = Material::Group::Regular;
    s_mat->materials.push_back(mat4);
    mat4->Num = s_mat->materials.isize();
    s_mat->materialMap.insert_or_assign(mat4->Name, mat4->Num);

    [[maybe_unused]] auto *curve1 = Curve::AddCurve(*state, "THERMAL_ABSORPTANCE_TABLE");
    [[maybe_unused]] auto *curve2 = Curve::AddCurve(*state, "SOLAR_ABSORPTANCE_CURVE");

    bool errors_found(false);
    Material::GetVariableAbsorptanceInput(*state, errors_found);
    EXPECT_ENUM_EQ(mat1->absorpVarCtrlSignalOut, Material::VariableAbsCtrlSignal::SurfaceTemperature);
    EXPECT_EQ(mat1->absorpThermalVarCurveOut->Num, 1);
    EXPECT_EQ(mat1->absorpSolarVarCurveOut->Num, 2);
    EXPECT_ENUM_EQ(mat2->absorpVarCtrlSignalOut, Material::VariableAbsCtrlSignal::SurfaceReceivedSolarRadiation);
    EXPECT_EQ(mat2->absorpSolarVarCurveOut->Num, 2);
    EXPECT_ENUM_EQ(mat3->absorpVarCtrlSignalOut, Material::VariableAbsCtrlSignal::Scheduled);
    EXPECT_NE(mat3->absorpThermalVarSchedOut, nullptr);
    EXPECT_NE(mat3->absorpSolarVarSchedOut, nullptr);
    // Blank inside-face fields preserve legacy behavior: only the outside face varies.
    EXPECT_ENUM_EQ(mat1->absorpVarCtrlSignalIn, Material::VariableAbsCtrlSignal::Invalid);
    EXPECT_EQ(mat1->absorpThermalVarCurveIn, nullptr);
    EXPECT_EQ(mat1->absorpSolarVarCurveIn, nullptr);
    EXPECT_ENUM_EQ(mat2->absorpVarCtrlSignalIn, Material::VariableAbsCtrlSignal::Invalid);
    EXPECT_EQ(mat2->absorpThermalVarCurveIn, nullptr);
    EXPECT_EQ(mat2->absorpSolarVarCurveIn, nullptr);
    EXPECT_ENUM_EQ(mat3->absorpVarCtrlSignalIn, Material::VariableAbsCtrlSignal::Invalid);
    EXPECT_EQ(mat3->absorpThermalVarSchedIn, nullptr);
    EXPECT_EQ(mat3->absorpSolarVarSchedIn, nullptr);
    // Tests to see if different things get picked up at the outside and the inside (Case 4)
    EXPECT_ENUM_EQ(mat4->absorpVarCtrlSignalOut, Material::VariableAbsCtrlSignal::Scheduled);
    EXPECT_NE(mat4->absorpThermalVarSchedOut, nullptr);
    EXPECT_NE(mat4->absorpSolarVarSchedOut, nullptr);
    EXPECT_ENUM_EQ(mat4->absorpVarCtrlSignalIn, Material::VariableAbsCtrlSignal::SurfaceTemperature);
    EXPECT_EQ(mat4->absorpThermalVarCurveIn->Num, 1);
    EXPECT_EQ(mat4->absorpSolarVarCurveIn->Num, 2);

    std::string idf_objects_bad_inputs = delimited_string({
        "MaterialProperty:VariableAbsorptance,",
        "variableThermal_wall_1,  !- Name",
        "WALL_1,                  !- Reference Material Name",
        "SurfaceTemperature,      !- Control Signal",
        ",                        !- Thermal Absorptance Function Name",
        ",                        !- Thermal Absorptance Schedule Name",
        ",                        !- Solar Absorptance Function Name",
        ";                        !- Solar Absorptance Schedule Name",
    });

    ASSERT_TRUE(process_idf(idf_objects_bad_inputs));

    // empty function
    Material::GetVariableAbsorptanceInput(*state, errors_found);
    compare_err_stream(
        "   ** Severe  ** MaterialProperty:VariableAbsorptance: Non-schedule control signal is chosen but both outside face thermal and solar "
        "absorptance table or curve are undefined, for object VARIABLETHERMAL_WALL_1\n");

    idf_objects_bad_inputs = delimited_string({
        "MaterialProperty:VariableAbsorptance,",
        "variableThermal_wall_1,  !- Name",
        "WALL_1,                  !- Reference Material Name",
        "Scheduled,               !- Control Signal",
        ",                        !- Thermal Absorptance Function Name",
        "ABS_SCH,                 !- Thermal Absorptance Schedule Name",
        ",                        !- Solar Absorptance Function Name",
        "ABS_SCH,                 !- Solar Absorptance Schedule Name",
        "SurfaceTemperature,      !- Control Signal Inside Face",
        ",                        !- Thermal Absorptance Function Name Inside Face",
        ",                        !- Thermal Absorptance Schedule Name Inside Face",
        ",                        !- Solar Absorptance Function Name Inside Face",
        ";                        !- Solar Absorptance Schedule Name Inside Face",
    });

    ASSERT_TRUE(process_idf(idf_objects_bad_inputs));

    // empty function
    Material::GetVariableAbsorptanceInput(*state, errors_found);
    compare_err_stream(
        "   ** Severe  ** MaterialProperty:VariableAbsorptance: Non-schedule control signal is chosen but both inside face thermal and solar "
        "absorptance table or curve are undefined, for object VARIABLETHERMAL_WALL_1\n");

    // outside control variable is surface temperature but solar absorptance has schedule
    idf_objects_bad_inputs = delimited_string({
        "MaterialProperty:VariableAbsorptance,",
        "variableThermal_wall_1,  !- Name",
        "WALL_1,                  !- Reference Material Name",
        "SurfaceTemperature,      !- Control Signal",
        ",                        !- Thermal Absorptance Function Name",
        "ABS_SCH,                 !- Thermal Absorptance Schedule Name",
        "SOLAR_ABSORPTANCE_CURVE, !- Solar Absorptance Function Name",
        ";                        !- Solar Absorptance Schedule Name",
    });
    ASSERT_TRUE(process_idf(idf_objects_bad_inputs));
    Material::GetVariableAbsorptanceInput(*state, errors_found);
    compare_err_stream(
        "   ** Warning ** MaterialProperty:VariableAbsorptance: Non-schedule control signal is chosen. Outside face thermal or solar absorptance "
        "schedule name is going to be ignored, for object VARIABLETHERMAL_WALL_1\n",
        true);

    // inside control variable is surface temperature but solar absorptance uses schedule
    idf_objects_bad_inputs = delimited_string({
        "MaterialProperty:VariableAbsorptance,",
        "variableThermal_wall_1,  !- Name",
        "WALL_1,                  !- Reference Material Name",
        "Scheduled,               !- Control Signal",
        ",                        !- Thermal Absorptance Function Name",
        "ABS_SCH,                 !- Thermal Absorptance Schedule Name",
        ",                        !- Solar Absorptance Function Name",
        "ABS_SCH,                 !- Solar Absorptance Schedule Name",
        "Scheduled,               !- Control Signal Inside Face",
        ",                        !- Thermal Absorptance Function Name Inside Face",
        ",                        !- Thermal Absorptance Schedule Name Inside Face",
        ",                        !- Solar Absorptance Function Name Inside Face",
        ";                        !- Solar Absorptance Schedule Name Inside Face",
    });
    ASSERT_TRUE(process_idf(idf_objects_bad_inputs));
    Material::GetVariableAbsorptanceInput(*state, errors_found);
    compare_err_stream(
        "   ** Severe  ** MaterialProperty:VariableAbsorptance: Control signal \"Scheduled\" is chosen but both inside face thermal and solar "
        "absorptance schedules are undefined, for object VARIABLETHERMAL_WALL_1\n",
        true);

    // outside control variable is surface temperature but solar absorptance has schedule
    idf_objects_bad_inputs = delimited_string({
        "MaterialProperty:VariableAbsorptance,",
        "variableThermal_wall_1,  !- Name",
        "WALL_1,                  !- Reference Material Name",
        "Scheduled,      !- Control Signal",
        ",                        !- Thermal Absorptance Function Name",
        "ABS_SCH,                 !- Thermal Absorptance Schedule Name",
        "SOLAR_ABSORPTANCE_CURVE, !- Solar Absorptance Function Name",
        ";                        !- Solar Absorptance Schedule Name",
    });
    ASSERT_TRUE(process_idf(idf_objects_bad_inputs));
    Material::GetVariableAbsorptanceInput(*state, errors_found);
    compare_err_stream(
        "   ** Warning ** MaterialProperty:VariableAbsorptance: Control signal \"Scheduled\" is chosen. Outside face thermal or solar absorptance "
        "function name is going to be ignored, for object VARIABLETHERMAL_WALL_1\n",
        true);

    // inside control variable is surface temperature but solar absorptance has schedule
    idf_objects_bad_inputs = delimited_string({
        "MaterialProperty:VariableAbsorptance,",
        "variableThermal_wall_1,  !- Name",
        "WALL_1,                  !- Reference Material Name",
        "Scheduled,               !- Control Signal",
        ",                        !- Thermal Absorptance Function Name",
        "ABS_SCH,                 !- Thermal Absorptance Schedule Name",
        ",                        !- Solar Absorptance Function Name",
        "ABS_SCH,                 !- Solar Absorptance Schedule Name",
        "Scheduled,               !- Control Signal Inside Face",
        ",                        !- Thermal Absorptance Function Name Inside Face",
        "ABS_SCH,                 !- Thermal Absorptance Schedule Name Inside Face",
        "SOLAR_ABSORPTANCE_CURVE, !- Solar Absorptance Function Name Inside Face",
        ";                        !- Solar Absorptance Schedule Name Inside Face",
    });
    ASSERT_TRUE(process_idf(idf_objects_bad_inputs));
    Material::GetVariableAbsorptanceInput(*state, errors_found);
    compare_err_stream(
        "   ** Warning ** MaterialProperty:VariableAbsorptance: Control signal \"Scheduled\" is chosen. Inside face thermal or solar absorptance "
        "function name is going to be ignored, for object VARIABLETHERMAL_WALL_1\n",
        true);

    // wrong reference material
    idf_objects_bad_inputs = delimited_string({
        "MaterialProperty:VariableAbsorptance,",
        "variableThermal_wall_1,  !- Name",
        "WALL_0,                  !- Reference Material Name",
        "SurfaceTemperature,      !- Control Signal",
        "THERMAL_ABSORPTANCE_TABLE, !- Thermal Absorptance Function Name",
        ",                        !- Thermal Absorptance Schedule Name",
        "SOLAR_ABSORPTANCE_CURVE, !- Solar Absorptance Function Name",
        ";                        !- Solar Absorptance Schedule Name",
    });
    ASSERT_TRUE(process_idf(idf_objects_bad_inputs));
    Material::GetVariableAbsorptanceInput(*state, errors_found);
    compare_err_stream("   ** Severe  ** GetVariableAbsorptanceInput: MaterialProperty:VariableAbsorptance = VARIABLETHERMAL_WALL_1\n   **   ~~~   "
                       "** Reference Material Name = WALL_0, item not found.\n",
                       true);

    // wrong material group
    idf_objects_bad_inputs = delimited_string({
        "MaterialProperty:VariableAbsorptance,",
        "variableThermal_wall_1,  !- Name",
        "WALL_1,                  !- Reference Material Name",
        "SurfaceTemperature,      !- Control Signal",
        "THERMAL_ABSORPTANCE_TABLE, !- Thermal Absorptance Function Name",
        ",                        !- Thermal Absorptance Schedule Name",
        "SOLAR_ABSORPTANCE_CURVE, !- Solar Absorptance Function Name",
        ";                        !- Solar Absorptance Schedule Name",
    });
    ASSERT_TRUE(process_idf(idf_objects_bad_inputs));
    mat1->group = Material::Group::Glass;
    Material::GetVariableAbsorptanceInput(*state, errors_found);
    compare_err_stream("   ** Severe  ** MaterialProperty:VariableAbsorptance: Reference Material is not appropriate type for Thermal/Solar "
                       "Absorptance properties, material=WALL_1, must have regular properties (Thermal/Solar Absorptance)\n",
                       true);
}

TEST_F(EnergyPlusFixture, GetMaterialDataConstructAbsorpTest)
{
    std::string const idf_objects = delimited_string({
        "  Material,",
        "    MatOutSet,    !- Name",
        "    MediumSmooth, !- Roughness",
        "    0.2,          !- Thickness {m}",
        "    0.1,          !- Conductivity {W/m-K}",
        "    1000,         !- Density {kg/m3}",
        "    800,          !- Specific Heat {J/kg-K}",
        "    0.9,          !- Thermal Absorptance",
        "    0.8,          !- Solar Absorptance",
        "    0.7;          !- Visible Absorptance",

        "  Material,",
        "    MatOaISet,    !- Name",
        "    MediumSmooth, !- Roughness",
        "    0.3,          !- Thickness {m}",
        "    0.4,          !- Conductivity {W/m-K}",
        "    1000,         !- Density {kg/m3}",
        "    800,          !- Specific Heat {J/kg-K}",
        "    0.6,          !- Thermal Absorptance",
        "    0.5,          !- Solar Absorptance",
        "    0.4,          !- Visible Absorptance",
        "    0.3,          !- Thermal Absorptance Inside Face",
        "    0.2,          !- Solar Absorptance Inside Face",
        "    0.1;          !- Visible Absorptance Inside Face",

        "  Material:NoMass,",
        "    nmMatOutSet, !- Name",
        "    Rough,       !- Roughness",
        "    0.777,       !- Thermal Resistance {m2-K/W}",
        "    0.95,        !- Thermal Absorptance",
        "    0.85,        !- Solar Absorptance",
        "    0.75;        !- Visible Absorptance",

        "  Material:NoMass,",
        "    nmMatOaISet, !- Name",
        "    Rough,       !- Roughness",
        "    0.222,       !- Thermal Resistance {m2-K/W}",
        "    0.65,        !- Thermal Absorptance",
        "    0.55,        !- Solar Absorptance",
        "    0.45,        !- Visible Absorptance",
        "    0.35,        !- Thermal Absorptance Inside Face",
        "    0.25,        !- Solar Absorptance Inside Face",
        "    0.15;        !- Visible Absorptance Inside Face",

        "  Construction,",
        "    OneLayerReg1, !- Name",
        "    MatOutSet;   !- Outside Layer",

        "  Construction,",
        "    OneLayerReg2, !- Name",
        "    MatOaISet;   !- Outside Layer",

        "  Construction,",
        "    OneLayerNM1, !- Name",
        "    nmMatOutSet; !- Outside Layer",

        "  Construction,",
        "    OneLayerNM2, !- Name",
        "    nmMatOaISet; !- Outside Layer",

        "  Construction,",
        "    TwoLayerReg, !- Name",
        "    MatOutSet,   !- Outside Layer",
        "    MatOaISet;   !- Layer 2",

        "  Construction,",
        "    TwoLayerNM,  !- Name",
        "    nmMatOutSet, !- Outside Layer",
        "    nmMatOaISet; !- Layer 2",
    });

    bool errorsFound = false;
    Real64 constexpr tolerance = 0.000001;

    ASSERT_TRUE(process_idf(idf_objects));

    EnergyPlus::Material::GetMaterialData(*state, errorsFound);

    auto &dataMat = state->dataMaterial;

    // Test the reading in of the material data
    EXPECT_EQ("NMMATOUTSET", dataMat->materials(3)->Name);
    EXPECT_EQ("NMMATOAISET", dataMat->materials(4)->Name);
    EXPECT_NEAR(dataMat->materials(1)->AbsorpThermalOut, 0.9, tolerance);
    EXPECT_NEAR(dataMat->materials(1)->AbsorpSolarOut, 0.8, tolerance);
    EXPECT_NEAR(dataMat->materials(1)->AbsorpVisibleOut, 0.7, tolerance);
    EXPECT_NEAR(dataMat->materials(1)->AbsorpThermalIn, 0.9, tolerance);
    EXPECT_NEAR(dataMat->materials(1)->AbsorpSolarIn, 0.8, tolerance);
    EXPECT_NEAR(dataMat->materials(1)->AbsorpVisibleIn, 0.7, tolerance);
    EXPECT_NEAR(dataMat->materials(2)->AbsorpThermalOut, 0.6, tolerance);
    EXPECT_NEAR(dataMat->materials(2)->AbsorpSolarOut, 0.5, tolerance);
    EXPECT_NEAR(dataMat->materials(2)->AbsorpVisibleOut, 0.4, tolerance);
    EXPECT_NEAR(dataMat->materials(2)->AbsorpThermalIn, 0.3, tolerance);
    EXPECT_NEAR(dataMat->materials(2)->AbsorpSolarIn, 0.2, tolerance);
    EXPECT_NEAR(dataMat->materials(2)->AbsorpVisibleIn, 0.1, tolerance);
    EXPECT_NEAR(dataMat->materials(3)->AbsorpThermalOut, 0.95, tolerance);
    EXPECT_NEAR(dataMat->materials(3)->AbsorpSolarOut, 0.85, tolerance);
    EXPECT_NEAR(dataMat->materials(3)->AbsorpVisibleOut, 0.75, tolerance);
    EXPECT_NEAR(dataMat->materials(3)->AbsorpThermalIn, 0.95, tolerance);
    EXPECT_NEAR(dataMat->materials(3)->AbsorpSolarIn, 0.85, tolerance);
    EXPECT_NEAR(dataMat->materials(3)->AbsorpVisibleIn, 0.75, tolerance);
    EXPECT_NEAR(dataMat->materials(4)->AbsorpThermalOut, 0.65, tolerance);
    EXPECT_NEAR(dataMat->materials(4)->AbsorpSolarOut, 0.55, tolerance);
    EXPECT_NEAR(dataMat->materials(4)->AbsorpVisibleOut, 0.45, tolerance);
    EXPECT_NEAR(dataMat->materials(4)->AbsorpThermalIn, 0.35, tolerance);
    EXPECT_NEAR(dataMat->materials(4)->AbsorpSolarIn, 0.25, tolerance);
    EXPECT_NEAR(dataMat->materials(4)->AbsorpVisibleIn, 0.15, tolerance);

    // Read the constructions and then test that the Construct data is being set correctly
    HeatBalanceManager::GetConstructData(*state, errorsFound);
    for (int constructNum = 1; constructNum <= 6; constructNum++) {
        DataHeatBalance::CheckAndSetConstructionProperties(*state, constructNum, errorsFound);
    }
    auto &dataCon = state->dataConstruction;
    EXPECT_NEAR(dataCon->Construct(1).OutsideAbsorpThermal, 0.9, tolerance);
    EXPECT_NEAR(dataCon->Construct(1).OutsideAbsorpSolar, 0.8, tolerance);
    EXPECT_NEAR(dataCon->Construct(1).OutsideAbsorpVis, 0.7, tolerance);
    EXPECT_NEAR(dataCon->Construct(1).InsideAbsorpThermal, 0.9, tolerance);
    EXPECT_NEAR(dataCon->Construct(1).InsideAbsorpSolar, 0.8, tolerance);
    EXPECT_NEAR(dataCon->Construct(1).InsideAbsorpVis, 0.7, tolerance);
    EXPECT_NEAR(dataCon->Construct(2).OutsideAbsorpThermal, 0.6, tolerance);
    EXPECT_NEAR(dataCon->Construct(2).OutsideAbsorpSolar, 0.5, tolerance);
    EXPECT_NEAR(dataCon->Construct(2).OutsideAbsorpVis, 0.4, tolerance);
    EXPECT_NEAR(dataCon->Construct(2).InsideAbsorpThermal, 0.3, tolerance);
    EXPECT_NEAR(dataCon->Construct(2).InsideAbsorpSolar, 0.2, tolerance);
    EXPECT_NEAR(dataCon->Construct(2).InsideAbsorpVis, 0.1, tolerance);
    EXPECT_NEAR(dataCon->Construct(3).OutsideAbsorpThermal, 0.95, tolerance);
    EXPECT_NEAR(dataCon->Construct(3).OutsideAbsorpSolar, 0.85, tolerance);
    EXPECT_NEAR(dataCon->Construct(3).OutsideAbsorpVis, 0.75, tolerance);
    EXPECT_NEAR(dataCon->Construct(3).InsideAbsorpThermal, 0.95, tolerance);
    EXPECT_NEAR(dataCon->Construct(3).InsideAbsorpSolar, 0.85, tolerance);
    EXPECT_NEAR(dataCon->Construct(3).InsideAbsorpVis, 0.75, tolerance);
    EXPECT_NEAR(dataCon->Construct(4).OutsideAbsorpThermal, 0.65, tolerance);
    EXPECT_NEAR(dataCon->Construct(4).OutsideAbsorpSolar, 0.55, tolerance);
    EXPECT_NEAR(dataCon->Construct(4).OutsideAbsorpVis, 0.45, tolerance);
    EXPECT_NEAR(dataCon->Construct(4).InsideAbsorpThermal, 0.35, tolerance);
    EXPECT_NEAR(dataCon->Construct(4).InsideAbsorpSolar, 0.25, tolerance);
    EXPECT_NEAR(dataCon->Construct(4).InsideAbsorpVis, 0.15, tolerance);
    EXPECT_NEAR(dataCon->Construct(5).OutsideAbsorpThermal, 0.9, tolerance);
    EXPECT_NEAR(dataCon->Construct(5).OutsideAbsorpSolar, 0.8, tolerance);
    EXPECT_NEAR(dataCon->Construct(5).OutsideAbsorpVis, 0.7, tolerance);
    EXPECT_NEAR(dataCon->Construct(5).InsideAbsorpThermal, 0.3, tolerance);
    EXPECT_NEAR(dataCon->Construct(5).InsideAbsorpSolar, 0.2, tolerance);
    EXPECT_NEAR(dataCon->Construct(5).InsideAbsorpVis, 0.1, tolerance);
    EXPECT_NEAR(dataCon->Construct(6).OutsideAbsorpThermal, 0.95, tolerance);
    EXPECT_NEAR(dataCon->Construct(6).OutsideAbsorpSolar, 0.85, tolerance);
    EXPECT_NEAR(dataCon->Construct(6).OutsideAbsorpVis, 0.75, tolerance);
    EXPECT_NEAR(dataCon->Construct(6).InsideAbsorpThermal, 0.35, tolerance);
    EXPECT_NEAR(dataCon->Construct(6).InsideAbsorpSolar, 0.25, tolerance);
    EXPECT_NEAR(dataCon->Construct(6).InsideAbsorpVis, 0.15, tolerance);
}

TEST_F(EnergyPlusFixture, GetMaterialDataTracksInsideFaceAbsorptanceInputPresence)
{
    std::string const idf_objects = delimited_string({
        "Material,",
        "  Bulk Only,           !- Name",
        "  Rough,               !- Roughness",
        "  0.1,                 !- Thickness {m}",
        "  0.5,                 !- Conductivity {W/m-K}",
        "  800,                 !- Density {kg/m3}",
        "  900,                 !- Specific Heat {J/kg-K}",
        "  0.9,                 !- Thermal Absorptance",
        "  0.7,                 !- Solar Absorptance",
        "  0.6;                 !- Visible Absorptance",

        "Material,",
        "  Equal Thermal,       !- Name",
        "  Rough,               !- Roughness",
        "  0.1,                 !- Thickness {m}",
        "  0.5,                 !- Conductivity {W/m-K}",
        "  800,                 !- Density {kg/m3}",
        "  900,                 !- Specific Heat {J/kg-K}",
        "  0.8,                 !- Thermal Absorptance",
        "  0.6,                 !- Solar Absorptance",
        "  0.5,                 !- Visible Absorptance",
        "  0.8;                 !- Thermal Absorptance Inside Face",

        "Material:NoMass,",
        "  Equal Solar,         !- Name",
        "  Rough,               !- Roughness",
        "  0.5,                 !- Thermal Resistance {m2-K/W}",
        "  0.85,                !- Thermal Absorptance",
        "  0.65,                !- Solar Absorptance",
        "  0.55,                !- Visible Absorptance",
        "  ,                    !- Thermal Absorptance Inside Face",
        "  0.65,                !- Solar Absorptance Inside Face",
        "  0.55;                !- Visible Absorptance Inside Face",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    bool errorsFound = false;
    Material::GetMaterialData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    auto const *bulkOnly = state->dataMaterial->materials(Material::GetMaterialNum(*state, "BULK ONLY"));
    EXPECT_FALSE(bulkOnly->hasAbsorpThermalInputIn);
    EXPECT_FALSE(bulkOnly->hasAbsorpSolarInputIn);
    EXPECT_FALSE(bulkOnly->hasAbsorpVisibleInputIn);

    auto const *equalThermal = state->dataMaterial->materials(Material::GetMaterialNum(*state, "EQUAL THERMAL"));
    EXPECT_TRUE(equalThermal->hasAbsorpThermalInputIn);
    EXPECT_FALSE(equalThermal->hasAbsorpSolarInputIn);
    EXPECT_FALSE(equalThermal->hasAbsorpVisibleInputIn);
    EXPECT_DOUBLE_EQ(equalThermal->AbsorpThermalOut, equalThermal->AbsorpThermalIn);

    auto const *equalSolar = state->dataMaterial->materials(Material::GetMaterialNum(*state, "EQUAL SOLAR"));
    EXPECT_FALSE(equalSolar->hasAbsorpThermalInputIn);
    EXPECT_TRUE(equalSolar->hasAbsorpSolarInputIn);
    EXPECT_TRUE(equalSolar->hasAbsorpVisibleInputIn);
    EXPECT_DOUBLE_EQ(equalSolar->AbsorpSolarOut, equalSolar->AbsorpSolarIn);
    EXPECT_DOUBLE_EQ(equalSolar->AbsorpVisibleOut, equalSolar->AbsorpVisibleIn);
}

TEST_F(EnergyPlusFixture, GetMaterialDataWindowShadingFaceAbsorptances)
{
    std::string const idf_objects = delimited_string({
        "WindowMaterial:Shade,",
        "  Test Shade,              !- Name",
        "  0.2,                     !- Solar Transmittance",
        "  0.3,                     !- Solar Reflectance",
        "  0.2,                     !- Visible Transmittance",
        "  0.3,                     !- Visible Reflectance",
        "  0.8,                     !- Infrared Hemispherical Emissivity",
        "  0.0,                     !- Infrared Transmittance",
        "  0.01,                    !- Thickness",
        "  0.1,                     !- Conductivity",
        "  0.05,                    !- Shade to Glass Distance",
        "  0.0,                     !- Top Opening Multiplier",
        "  0.0,                     !- Bottom Opening Multiplier",
        "  0.0,                     !- Left-Side Opening Multiplier",
        "  0.0,                     !- Right-Side Opening Multiplier",
        "  0.0;                     !- Airflow Permeability",

        "WindowMaterial:Screen,",
        "  Test Screen,             !- Name",
        "  ModelAsDiffuse,          !- Reflected Beam Transmittance Accounting Method",
        "  0.2,                     !- Diffuse Solar Reflectance",
        "  0.2,                     !- Diffuse Visible Reflectance",
        "  0.8,                     !- Thermal Hemispherical Emissivity",
        "  200.0,                   !- Conductivity",
        "  0.002,                   !- Screen Material Spacing",
        "  0.001,                   !- Screen Material Diameter",
        "  0.025,                   !- Screen to Glass Distance",
        "  0.0,                     !- Top Opening Multiplier",
        "  0.0,                     !- Bottom Opening Multiplier",
        "  0.0,                     !- Left Side Opening Multiplier",
        "  0.0,                     !- Right Side Opening Multiplier",
        "  5;                       !- Angle of Resolution for Screen Transmittance Output Map",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    bool errorsFound = false;
    Material::GetMaterialData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    auto const *shade = state->dataMaterial->materials(Material::GetMaterialNum(*state, "TEST SHADE"));
    EXPECT_DOUBLE_EQ(0.5, shade->AbsorpSolarOut);
    EXPECT_DOUBLE_EQ(shade->AbsorpSolarOut, shade->AbsorpSolarIn);
    EXPECT_DOUBLE_EQ(shade->AbsorpSolarInputOut, shade->AbsorpSolarInputIn);

    auto const *screen = state->dataMaterial->materials(Material::GetMaterialNum(*state, "TEST SCREEN"));
    EXPECT_DOUBLE_EQ(0.6, screen->AbsorpThermalOut);
    EXPECT_DOUBLE_EQ(screen->AbsorpThermalOut, screen->AbsorpThermalIn);
    EXPECT_DOUBLE_EQ(screen->AbsorpThermalInputOut, screen->AbsorpThermalInputIn);
    EXPECT_DOUBLE_EQ(screen->AbsorpSolarOut, screen->AbsorpSolarIn);
    EXPECT_DOUBLE_EQ(screen->AbsorpSolarInputOut, screen->AbsorpSolarInputIn);
    EXPECT_DOUBLE_EQ(screen->AbsorpVisibleOut, screen->AbsorpVisibleIn);
    EXPECT_DOUBLE_EQ(screen->AbsorpVisibleInputOut, screen->AbsorpVisibleInputIn);
}
