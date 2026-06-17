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

// EnergyPlus::ElectricPowerServiceManager Unit Tests
#include <map>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/PVWatts.hh>
#include <EnergyPlus/WeatherManager.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, PVWattsGenerator_Constructor)
{
    using namespace PVWatts;

    PVWattsGenerator pvw(*state, "PVArray", 4000.0, ModuleType::STANDARD, ArrayType::FIXED_ROOF_MOUNTED);
    EXPECT_DOUBLE_EQ(4000.0, pvw.getDCSystemCapacity());
    EXPECT_ENUM_EQ(ModuleType::STANDARD, pvw.getModuleType());
    EXPECT_ENUM_EQ(ArrayType::FIXED_ROOF_MOUNTED, pvw.getArrayType());
    EXPECT_DOUBLE_EQ(0.14, pvw.getSystemLosses());
    EXPECT_ENUM_EQ(GeometryType::TILT_AZIMUTH, pvw.getGeometryType());
    EXPECT_DOUBLE_EQ(20.0, pvw.getTilt());
    EXPECT_DOUBLE_EQ(180.0, pvw.getAzimuth());
    EXPECT_DOUBLE_EQ(0.4, pvw.getGroundCoverageRatio());

    ASSERT_THROW(PVWattsGenerator pvw2(
                     *state, "", -1000.0, ModuleType::PREMIUM, ArrayType::FIXED_OPEN_RACK, 1.1, GeometryType::TILT_AZIMUTH, 91.0, 360.0, 0, -0.1),
                 std::runtime_error);
    std::string const error_string = delimited_string({"   ** Severe  ** PVWatts: name cannot be blank.",
                                                       "   ** Severe  ** PVWatts: DC system capacity must be greater than zero.",
                                                       "   ** Severe  ** PVWatts: Invalid system loss value 1.10",
                                                       "   ** Severe  ** PVWatts: Invalid tilt: 91.00",
                                                       "   ** Severe  ** PVWatts: Invalid azimuth: 360.00",
                                                       "   ** Severe  ** PVWatts: Invalid ground coverage ratio: -0.10",
                                                       "   **  Fatal  ** Errors found in getting PVWatts input",
                                                       "   ...Summary of Errors that led to program termination:",
                                                       "   ..... Reference severe error count=6",
                                                       "   ..... Last severe error=PVWatts: Invalid ground coverage ratio: -0.10"});
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, PVWattsGenerator_GetInputs)
{
    using namespace PVWatts;
    const std::string idfTxt = delimited_string({"Generator:PVWatts,",
                                                 "PVWattsArray1,",
                                                 "5,",
                                                 "4000,",
                                                 "Premium,",
                                                 "OneAxis,",
                                                 ",",
                                                 ",",
                                                 ",",
                                                 ";",
                                                 "Generator:PVWatts,",
                                                 "PVWattsArray2,",
                                                 "5,",
                                                 "4000,",
                                                 "Premium,",
                                                 "OneAxis,",
                                                 ",",
                                                 ",",
                                                 ",",
                                                 ",",
                                                 ",",
                                                 ";",
                                                 "Generator:PVWatts,",
                                                 "PVWattsArray3,",
                                                 "5,",
                                                 "4000,",
                                                 "Premium,",
                                                 "OneAxis,",
                                                 ",",
                                                 ",",
                                                 "21,",
                                                 "175,",
                                                 ",",
                                                 "0.5;",
                                                 "Output:Variable,*,Generator Produced DC Electricity Rate,timestep;"});
    process_idf(idfTxt);
    EXPECT_FALSE(has_err_output());
    auto pvw1 = PVWattsGenerator::createFromIdfObj(*state, 1);
    EXPECT_ENUM_EQ(pvw1->getModuleType(), ModuleType::PREMIUM);
    EXPECT_ENUM_EQ(pvw1->getArrayType(), ArrayType::ONE_AXIS);
    EXPECT_DOUBLE_EQ(0.4, pvw1->getGroundCoverageRatio());
    auto pvw2 = PVWattsGenerator::createFromIdfObj(*state, 2);
    EXPECT_DOUBLE_EQ(0.4, pvw2->getGroundCoverageRatio());
    auto pvw3 = PVWattsGenerator::createFromIdfObj(*state, 3);
    EXPECT_DOUBLE_EQ(175.0, pvw3->getAzimuth());
    EXPECT_DOUBLE_EQ(21.0, pvw3->getTilt());
    EXPECT_DOUBLE_EQ(0.5, pvw3->getGroundCoverageRatio());
}

TEST_F(EnergyPlusFixture, PVWattsGenerator_GetInputsFailure)
{
    using namespace PVWatts;
    const std::string idfTxt = delimited_string({"Generator:PVWatts,",
                                                 "PVWattsArray1,",
                                                 "5,",
                                                 "4000,",
                                                 "Primo,",          // misspelled
                                                 "FixedRoofMount,", // misspelled
                                                 ",",
                                                 "asdf,",
                                                 ",",
                                                 ";",
                                                 "Output:Variable,*,Generator Produced DC Electricity Rate,timestep;"});
    EXPECT_FALSE(process_idf(idfTxt, false));
    ASSERT_THROW(PVWattsGenerator::createFromIdfObj(*state, 1), std::runtime_error);
    std::string const error_string = delimited_string(
        {"   ** Severe  ** <root>[Generator:PVWatts][PVWattsArray1][array_geometry_type] - \"asdf\" - Failed to match against any enum values.",
         "   ** Severe  ** <root>[Generator:PVWatts][PVWattsArray1][array_type] - \"FixedRoofMount\" - Failed to match against any enum values.",
         "   ** Severe  ** <root>[Generator:PVWatts][PVWattsArray1][module_type] - \"Primo\" - Failed to match against any enum values.",
         "   ** Severe  ** PVWatts: Invalid Module Type: PRIMO",
         "   ** Severe  ** PVWatts: Invalid Array Type: FIXEDROOFMOUNT",
         "   ** Severe  ** PVWatts: Invalid Geometry Type: ASDF",
         "   **  Fatal  ** Errors found in getting PVWatts input",
         "   ...Summary of Errors that led to program termination:",
         "   ..... Reference severe error count=6",
         "   ..... Last severe error=PVWatts: Invalid Geometry Type: ASDF"});
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, PVWattsGenerator_Calc)
{
    using namespace PVWatts;
    // USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw
    // 6/15 at 7am
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->TimeStepZone = 1.0;
    state->dataHVACGlobal->TimeStepSys = 1.0;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    state->dataGlobal->BeginTimeStepFlag = true;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataGlobal->TimeStepsInHour = 1;
    Weather::AllocateWeatherData(*state); // gets us the albedo array initialized
    state->dataEnvrn->Year = 1986;
    state->dataEnvrn->Month = 6;
    state->dataEnvrn->DayOfMonth = 15;
    state->dataGlobal->HourOfDay = 8; // 8th hour of day, 7-8am
    state->dataWeather->WeatherFileLatitude = 33.45;
    state->dataWeather->WeatherFileLongitude = -111.98;
    state->dataWeather->WeatherFileTimeZone = -7;
    state->dataEnvrn->BeamSolarRad = 728;
    state->dataEnvrn->DifSolarRad = 70;
    state->dataEnvrn->WindSpeed = 3.1;
    state->dataEnvrn->OutDryBulbTemp = 31.7;

    PVWattsGenerator pvwa(*state, "PVWattsArrayA", 4000.0, ModuleType::STANDARD, ArrayType::FIXED_ROOF_MOUNTED);
    pvwa.setCellTemperature(30.345);
    pvwa.setPlaneOfArrayIrradiance(92.257);
    pvwa.calc(*state);
    Real64 generatorPower, generatorEnergy, thermalPower, thermalEnergy;
    pvwa.getResults(generatorPower, generatorEnergy, thermalPower, thermalEnergy);
    EXPECT_DOUBLE_EQ(thermalPower, 0.0);
    EXPECT_DOUBLE_EQ(thermalEnergy, 0.0);
    EXPECT_NEAR(generatorPower, 884.137, 0.5);
    EXPECT_NEAR(generatorEnergy, generatorPower * 60 * 60, 1);

    PVWattsGenerator pvwb(*state, "PVWattsArrayB", 3000.0, ModuleType::PREMIUM, ArrayType::ONE_AXIS, 0.16, GeometryType::TILT_AZIMUTH, 25.0, 100.);
    pvwb.setCellTemperature(38.620);
    pvwb.setPlaneOfArrayIrradiance(478.641);
    pvwb.calc(*state);
    pvwb.getResults(generatorPower, generatorEnergy, thermalPower, thermalEnergy);
    EXPECT_DOUBLE_EQ(thermalPower, 0.0);
    EXPECT_DOUBLE_EQ(thermalEnergy, 0.0);
    EXPECT_NEAR(generatorPower, 1609.812, 0.5);
    EXPECT_NEAR(generatorEnergy, generatorPower * 60 * 60, 1);

    PVWattsGenerator pvwc(
        *state, "PVWattsArrayC", 1000.0, ModuleType::THIN_FILM, ArrayType::FIXED_OPEN_RACK, 0.1, GeometryType::TILT_AZIMUTH, 30.0, 140.);
    pvwc.setCellTemperature(33.764);
    pvwc.setPlaneOfArrayIrradiance(255.213);
    pvwc.calc(*state);
    pvwc.getResults(generatorPower, generatorEnergy, thermalPower, thermalEnergy);
    EXPECT_DOUBLE_EQ(thermalPower, 0.0);
    EXPECT_DOUBLE_EQ(thermalEnergy, 0.0);
    EXPECT_NEAR(generatorPower, 433.109, 0.5);
    EXPECT_NEAR(generatorEnergy, generatorPower * 60 * 60, 1);

    PVWattsGenerator pvwd(
        *state, "PVWattsArrayD", 5500.0, ModuleType::STANDARD, ArrayType::ONE_AXIS_BACKTRACKING, 0.05, GeometryType::TILT_AZIMUTH, 34.0, 180.);
    pvwd.setCellTemperature(29.205);
    pvwd.setPlaneOfArrayIrradiance(36.799);
    pvwd.calc(*state);
    pvwd.getResults(generatorPower, generatorEnergy, thermalPower, thermalEnergy);
    EXPECT_DOUBLE_EQ(thermalPower, 0.0);
    EXPECT_DOUBLE_EQ(thermalEnergy, 0.0);
    EXPECT_NEAR(generatorPower, 2524.947, 0.5);
    EXPECT_NEAR(generatorEnergy, generatorPower * 60 * 60, 1);

    PVWattsGenerator pvwe(*state, "PVWattsArrayE", 3800.0, ModuleType::PREMIUM, ArrayType::TWO_AXIS, 0.08, GeometryType::TILT_AZIMUTH, 34.0, 180.);
    pvwe.setCellTemperature(42.229);
    pvwe.setPlaneOfArrayIrradiance(647.867);
    pvwe.calc(*state);
    pvwe.getResults(generatorPower, generatorEnergy, thermalPower, thermalEnergy);
    EXPECT_DOUBLE_EQ(thermalPower, 0.0);
    EXPECT_DOUBLE_EQ(thermalEnergy, 0.0);
    EXPECT_NEAR(generatorPower, 2759.937, 0.5);
    EXPECT_NEAR(generatorEnergy, generatorPower * 60 * 60, 1);
}

TEST_F(EnergyPlusFixture, PVWattsInverter_Constructor)
{
    const std::string idfTxt = delimited_string({"ElectricLoadCenter:Distribution,",
                                                 "ELC1,",
                                                 "GeneratorList1,",
                                                 "Baseload,",
                                                 ",",
                                                 ",",
                                                 ",",
                                                 "DirectCurrentWithInverter,",
                                                 "Inverter1;",
                                                 "ElectricLoadCenter:Inverter:PVWatts,",
                                                 "Inverter1,",
                                                 "1.10,",
                                                 "0.96;",
                                                 "ElectricLoadCenter:Generators,",
                                                 "GeneratorList1,",
                                                 "PVWattsArray1,",
                                                 "Generator:PVWatts,",
                                                 "1500,",
                                                 ",",
                                                 ",",
                                                 "PVWattsArray2,",
                                                 "Generator:PVWatts,",
                                                 "2500,",
                                                 ",",
                                                 ";",
                                                 "Generator:PVWatts,",
                                                 "PVWattsArray1,",
                                                 "5,",
                                                 "1500,",
                                                 "Standard,",
                                                 "OneAxis,",
                                                 ",",
                                                 ",",
                                                 ",",
                                                 ";",
                                                 "Generator:PVWatts,",
                                                 "PVWattsArray2,",
                                                 "5,",
                                                 "2500,",
                                                 "Standard,",
                                                 "OneAxis,",
                                                 ",",
                                                 ",",
                                                 ",",
                                                 ",",
                                                 ",",
                                                 ";"});
    ASSERT_TRUE(process_idf(idfTxt));
    state->init_state(*state);

    auto eplc(ElectPowerLoadCenter(*state, 1));
    ASSERT_TRUE(eplc.inverterPresent);
    EXPECT_DOUBLE_EQ(eplc.inverterObj->pvWattsDCCapacity(), 4000.0);
    state->dataHVACGlobal->TimeStepSys = 1.0;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    eplc.inverterObj->simulate(*state, 884.018);
    EXPECT_NEAR(eplc.inverterObj->aCPowerOut(), 842.527, 0.001);
}

TEST_F(EnergyPlusFixture, PVWatts_SSC_NaNFailure)
{
    const std::string idfTxt = delimited_string({

        "Shading:Site:Detailed,",
        "  FlatSurfaceShadetoEast,  !- Name",
        "  ,                        !- Transmittance Schedule Name",
        "  4,                       !- Number of Vertices",
        "  0.0,25.0,12.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "  0.0,20.00,12.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "  5.0,20.00,12.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "  5.0,25.0,12.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "Generator:PVWatts,",
        "  PVWatts3,                !- Name",
        "  5,                       !- PVWatts Version",
        "  3000,                    !- DC System Capacity {W}",
        "  Premium,                 !- Module Type",
        "  FixedOpenRack,           !- Array Type",
        "  0.14,                    !- System Losses",
        "  Surface,                 !- Array Geometry Type",
        "  ,                        !- Tilt Angle {deg}",
        "  ,                        !- Azimuth Angle {deg}",
        "  FlatSurfaceShadetoEast;  !- Surface Name",
    });

    const std::string name = "PVArray";
    constexpr Real64 dcSystemCapacity = 4000.0;
    constexpr auto moduleType = PVWatts::ModuleType::STANDARD;
    constexpr auto arrayType = PVWatts::ArrayType::FIXED_ROOF_MOUNTED;
    constexpr Real64 systemLosses = 0.14;
    constexpr auto geometryType = PVWatts::GeometryType::TILT_AZIMUTH;
    constexpr Real64 tilt = 0.0;
    constexpr Real64 azimuth = 180.0;
    constexpr size_t surfaceNum = 0;
    constexpr Real64 groundCoverageRatio = 0.4;

    // USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw
    // 6/15 at 7am
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->TimeStepZone = 1.0;
    state->dataHVACGlobal->TimeStepSys = 1.0;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    state->dataGlobal->BeginTimeStepFlag = true;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataGlobal->TimeStepsInHour = 1;
    Weather::AllocateWeatherData(*state); // gets us the albedo array initialized
    state->dataEnvrn->Year = 1986;
    state->dataEnvrn->Month = 6;
    state->dataEnvrn->DayOfMonth = 15;
    state->dataGlobal->HourOfDay = 8; // 8th hour of day, 7-8am
    state->dataWeather->WeatherFileLatitude = 33.45;
    state->dataWeather->WeatherFileLongitude = -111.98;
    state->dataWeather->WeatherFileTimeZone = -7;
    state->dataEnvrn->BeamSolarRad = 728;
    state->dataEnvrn->DifSolarRad = 70;
    state->dataEnvrn->WindSpeed = 3.1;
    state->dataEnvrn->OutDryBulbTemp = 31.7;

    // The TimeStepZone etc must be setp before calling the PVWattsGenerator constructor because it sets them on the ssc_data_t struct
    PVWatts::PVWattsGenerator pvw(
        *state, name, dcSystemCapacity, moduleType, arrayType, systemLosses, geometryType, tilt, azimuth, surfaceNum, groundCoverageRatio);
    EXPECT_DOUBLE_EQ(dcSystemCapacity, pvw.getDCSystemCapacity());
    EXPECT_ENUM_EQ(moduleType, pvw.getModuleType());
    EXPECT_ENUM_EQ(arrayType, pvw.getArrayType());
    EXPECT_DOUBLE_EQ(systemLosses, pvw.getSystemLosses());
    EXPECT_ENUM_EQ(geometryType, pvw.getGeometryType());
    EXPECT_DOUBLE_EQ(tilt, pvw.getTilt());
    EXPECT_DOUBLE_EQ(azimuth, pvw.getAzimuth());
    EXPECT_DOUBLE_EQ(groundCoverageRatio, pvw.getGroundCoverageRatio());

    pvw.setCellTemperature(30.345);
    pvw.setPlaneOfArrayIrradiance(92.257);
    pvw.calc(*state);

    Real64 generatorPower, generatorEnergy, thermalPower, thermalEnergy;
    pvw.getResults(generatorPower, generatorEnergy, thermalPower, thermalEnergy);
    EXPECT_FALSE(std::isnan(generatorPower));
    EXPECT_FALSE(std::isnan(generatorEnergy));
    EXPECT_FALSE(std::isnan(thermalPower));
    EXPECT_FALSE(std::isnan(thermalEnergy));
    EXPECT_DOUBLE_EQ(thermalPower, 0.0);
    EXPECT_DOUBLE_EQ(thermalEnergy, 0.0);
    EXPECT_NEAR(generatorPower, 1117.75, 0.5);
    EXPECT_NEAR(generatorEnergy, generatorPower * 60 * 60, 1);
}
