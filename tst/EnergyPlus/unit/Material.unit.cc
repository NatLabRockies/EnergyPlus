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

// EnergyPlus::Stand alone unit test of Issue4347; i.e., CalcHWBaseboard NTU-eff calculation

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
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
        "SurfaceReceivedSolarRadiation,      !- Control Signal",
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

    [[maybe_unused]] auto *curve1 = Curve::AddCurve(*state, "THERMAL_ABSORPTANCE_TABLE");
    [[maybe_unused]] auto *curve2 = Curve::AddCurve(*state, "SOLAR_ABSORPTANCE_CURVE");

    bool errors_found(false);
    Material::GetVariableAbsorptanceInput(*state, errors_found);
    EXPECT_ENUM_EQ(mat1->absorpVarCtrlSignal, Material::VariableAbsCtrlSignal::SurfaceTemperature);
    EXPECT_EQ(mat1->absorpThermalVarCurve->Num, 1);
    EXPECT_EQ(mat1->absorpSolarVarCurve->Num, 2);
    EXPECT_ENUM_EQ(mat2->absorpVarCtrlSignal, Material::VariableAbsCtrlSignal::SurfaceReceivedSolarRadiation);
    EXPECT_EQ(mat2->absorpSolarVarCurve->Num, 2);
    EXPECT_ENUM_EQ(mat3->absorpVarCtrlSignal, Material::VariableAbsCtrlSignal::Scheduled);
    EXPECT_NE(mat3->absorpThermalVarSched, nullptr);
    EXPECT_NE(mat3->absorpSolarVarSched, nullptr);

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
    compare_err_stream("   ** Severe  ** MaterialProperty:VariableAbsorptance: Non-schedule control signal is chosen but both thermal and solar "
                       "absorptance table or "
                       "curve are undefined, for object VARIABLETHERMAL_WALL_1\n");

    // control variable is surface temperature but solar absorptance uses schedule
    idf_objects_bad_inputs = delimited_string({
        "MaterialProperty:VariableAbsorptance,",
        "variableThermal_wall_1,  !- Name",
        "WALL_1,                  !- Reference Material Name",
        "Scheduled,               !- Control Signal",
        ",                        !- Thermal Absorptance Function Name",
        ",                        !- Thermal Absorptance Schedule Name",
        ",                        !- Solar Absorptance Function Name",
        ";                        !- Solar Absorptance Schedule Name",
    });
    ASSERT_TRUE(process_idf(idf_objects_bad_inputs));
    Material::GetVariableAbsorptanceInput(*state, errors_found);
    compare_err_stream("   ** Severe  ** MaterialProperty:VariableAbsorptance: Control signal \"Scheduled\" is chosen but both thermal and solar "
                       "absorptance schedules are undefined, for object "
                       "VARIABLETHERMAL_WALL_1\n",
                       true);

    // control variable is surface temperature but solar absorptance has schedule
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
    compare_err_stream("   ** Warning ** MaterialProperty:VariableAbsorptance: Non-schedule control signal is chosen. Thermal or solar absorptance "
                       "schedule name is going to be "
                       "ignored, for object VARIABLETHERMAL_WALL_1\n",
                       true);

    // control variable is surface temperature but solar absorptance has schedule
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
    compare_err_stream("   ** Warning ** MaterialProperty:VariableAbsorptance: Control signal \"Scheduled\" is chosen. Thermal or solar absorptance "
                       "function name is going to be "
                       "ignored, for object VARIABLETHERMAL_WALL_1\n",
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
