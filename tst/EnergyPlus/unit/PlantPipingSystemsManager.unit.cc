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

// EnergyPlus::PlantPipingSystemsManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// Fixture Headers
#include "Fixtures/EnergyPlusFixture.hh"

// EnergyPlus Headers
#include <EnergyPlus/ConfiguredFunctions.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantPipingSystemsManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>

using namespace EnergyPlus;
using namespace PlantPipingSystemsManager;
using Material::GetMaterialData;
using SurfaceGeometry::GetOSCMData;

TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_CheckInputs_CorrectInputs)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Slab,",
        "CoupledSlab,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM, !- Name of Floor Boundary Condition Model",
        "InGrade,       !- Slab Location (InGrade/OnGrade)",
        "Dummy Material,    !- Slab Material Name",
        "Yes,           !- Horizontal Insulation (Yes/No)",
        "Dummy Material,    !- Horizontal Insulation Material Name",
        "Full,          !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        ",              !- Perimeter insulation width (m)",
        "Yes,           !- Vertical Insulation (Yes/No)",
        "Dummy Material,    !- Vertical Insulation Name",
        "1.5,           !- Vertical perimeter insulation depth from surface (m)",
        "Hourly;        !- Domain Simulation Interval. (Timestep/Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound);

    EXPECT_FALSE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_CheckInputs_BadOSCMName)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Slab,",
        "CoupledSlab,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSC,  !- Name of Floor Boundary Condition Model",
        "InGrade,       !- Slab Location (InGrade/OnGrade)",
        "Dummy Material,    !- Slab Material Name",
        "Yes,           !- Horizontal Insulation (Yes/No)",
        "Dummy Material,    !- Horizontal Insulation Material Name",
        "Full,          !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        ",              !- Perimeter insulation width (m)",
        "Yes,           !- Vertical Insulation (Yes/No)",
        "Dummy Material,    !- Vertical Insulation Name",
        "1.5,           !- Vertical perimeter insulation depth from surface (m)",
        "Hourly;        !- Domain Simulation Interval. (Timestep/Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_CheckInputs_BadSlabLocation)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Slab,",
        "CoupledSlab,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM, !- Name of Floor Boundary Condition Model",
        "InGade,        !- Slab Location (InGrade/OnGrade)",
        "Dummy Material,    !- Slab Material Name",
        "Yes,           !- Horizontal Insulation (Yes/No)",
        "Dummy Material,    !- Horizontal Insulation Material Name",
        "Full,          !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        ",              !- Perimeter insulation width (m)",
        "Yes,           !- Vertical Insulation (Yes/No)",
        "Dummy Material,    !- Vertical Insulation Name",
        "1.5,           !- Vertical perimeter insulation depth from surface (m)",
        "Hourly;        !- Domain Simulation Interval. (Timestep/Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    EXPECT_FALSE(process_idf(idf_objects, false));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_CheckInputs_BadSlabMaterialName)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Slab,",
        "CoupledSlab,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM, !- Name of Floor Boundary Condition Model",
        "InGrade,       !- Slab Location (InGrade/OnGrade)",
        "Dummy Materia, !- Slab Material Name",
        "Yes,           !- Horizontal Insulation (Yes/No)",
        "Dummy Material,    !- Horizontal Insulation Material Name",
        "Full,          !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        ",              !- Perimeter insulation width (m)",
        "Yes,           !- Vertical Insulation (Yes/No)",
        "Dummy Material,    !- Vertical Insulation Name",
        "1.5,           !- Vertical perimeter insulation depth from surface (m)",
        "Hourly;        !- Domain Simulation Interval. (Timestep/Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_CheckInputs_BadHorizInsSelection)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Slab,",
        "CoupledSlab,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM, !- Name of Floor Boundary Condition Model",
        "InGrade,       !- Slab Location (InGrade/OnGrade)",
        "Dummy Material,    !- Slab Material Name",
        "Ye,            !- Horizontal Insulation (Yes/No)",
        "Dummy Material,    !- Horizontal Insulation Material Name",
        "Full,          !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        ",              !- Perimeter insulation width (m)",
        "Yes,           !- Vertical Insulation (Yes/No)",
        "Dummy Material,    !- Vertical Insulation Name",
        "1.5,           !- Vertical perimeter insulation depth from surface (m)",
        "Hourly;        !- Domain Simulation Interval. (Timestep/Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    EXPECT_FALSE(process_idf(idf_objects, false));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_CheckInputs_BadHorizInsMaterialName)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Slab,",
        "CoupledSlab,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM, !- Name of Floor Boundary Condition Model",
        "InGrade,       !- Slab Location (InGrade/OnGrade)",
        "Dummy Material,    !- Slab Material Name",
        "Yes,           !- Horizontal Insulation (Yes/No)",
        "Dummy Materal, !- Horizontal Insulation Material Name",
        "Full,          !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        ",              !- Perimeter insulation width (m)",
        "Yes,           !- Vertical Insulation (Yes/No)",
        "Dummy Material,    !- Vertical Insulation Name",
        "1.5,           !- Vertical perimeter insulation depth from surface (m)",
        "Hourly;        !- Domain Simulation Interval. (Timestep/Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_CheckInputs_BadHorizInsExtentsSelection)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Slab,",
        "CoupledSlab,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM, !- Name of Floor Boundary Condition Model",
        "InGrade,       !- Slab Location (InGrade/OnGrade)",
        "Dummy Material,    !- Slab Material Name",
        "Yes,           !- Horizontal Insulation (Yes/No)",
        "Dummy Material,    !- Horizontal Insulation Material Name",
        "Ful,           !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        ",              !- Perimeter insulation width (m)",
        "Yes,           !- Vertical Insulation (Yes/No)",
        "Dummy Material,    !- Vertical Insulation Name",
        "1.5,           !- Vertical perimeter insulation depth from surface (m)",
        "Hourly;        !- Domain Simulation Interval. (Timestep/Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    EXPECT_FALSE(process_idf(idf_objects, false));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_CheckInputs_PerimeterInsulationWidth)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Slab,",
        "CoupledSlab,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM, !- Name of Floor Boundary Condition Model",
        "InGrade,       !- Slab Location (InGrade/OnGrade)",
        "Dummy Material,    !- Slab Material Name",
        "Yes,           !- Horizontal Insulation (Yes/No)",
        "Dummy Material,    !- Horizontal Insulation Material Name",
        "Perimeter,         !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        ",              !- Perimeter insulation width (m)",
        "Yes,           !- Vertical Insulation (Yes/No)",
        "Dummy Material,    !- Vertical Insulation Name",
        "1.5,           !- Vertical perimeter insulation depth from surface (m)",
        "Hourly;        !- Domain Simulation Interval. (Timestep/Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_CheckInputs_BadVertInsSelection)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Slab,",
        "CoupledSlab,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM, !- Name of Floor Boundary Condition Model",
        "InGrade,       !- Slab Location (InGrade/OnGrade)",
        "Dummy Material,    !- Slab Material Name",
        "Yes,           !- Horizontal Insulation (Yes/No)",
        "Dummy Material,    !- Horizontal Insulation Material Name",
        "Full,          !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        ",              !- Perimeter insulation width (m)",
        "Ye,            !- Vertical Insulation (Yes/No)",
        "Dummy Material,    !- Vertical Insulation Name",
        "1.5,           !- Vertical perimeter insulation depth from surface (m)",
        "Hourly;        !- Domain Simulation Interval. (Timestep/Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    EXPECT_FALSE(process_idf(idf_objects, false));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_CheckInputs_BadVertInsMaterialName)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Slab,",
        "CoupledSlab,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM, !- Name of Floor Boundary Condition Model",
        "InGrade,       !- Slab Location (InGrade/OnGrade)",
        "Dummy Material,    !- Slab Material Name",
        "Yes,           !- Horizontal Insulation (Yes/No)",
        "Dummy Material,    !- Horizontal Insulation Material Name",
        "Full,          !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        ",              !- Perimeter insulation width (m)",
        "Yes,           !- Vertical Insulation (Yes/No)",
        "Dummy Materia, !- Vertical Insulation Name",
        "1.5,           !- Vertical perimeter insulation depth from surface (m)",
        "Hourly;        !- Domain Simulation Interval. (Timestep/Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_CheckInputs_BadVertInsDepth)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Slab,",
        "CoupledSlab,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM, !- Name of Floor Boundary Condition Model",
        "InGrade,       !- Slab Location (InGrade/OnGrade)",
        "Dummy Material,    !- Slab Material Name",
        "Yes,           !- Horizontal Insulation (Yes/No)",
        "Dummy Material,    !- Horizontal Insulation Material Name",
        "Full,          !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        ",              !- Perimeter insulation width (m)",
        "Yes,           !- Vertical Insulation (Yes/No)",
        "Dummy Material,    !- Vertical Insulation Name",
        ",          !- Vertical perimeter insulation depth from surface (m)",
        "Hourly;        !- Domain Simulation Interval. (Timestep/Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

#ifdef GET_OUT // TimeStepSelection is not used
TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_CheckInputs_BadTimeStepSelection)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Slab,",
        "CoupledSlab,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM, !- Name of Floor Boundary Condition Model",
        "InGrade,       !- Slab Location (InGrade/OnGrade)",
        "Dummy Material,    !- Slab Material Name",
        "Yes,           !- Horizontal Insulation (Yes/No)",
        "Dummy Material,    !- Horizontal Insulation Material Name",
        "Full,          !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        ",              !- Perimeter insulation width (m)",
        "Yes,           !- Vertical Insulation (Yes/No)",
        "Dummy Material,    !- Vertical Insulation Name",
        "1.5,           !- Vertical perimeter insulation depth from surface (m)",
        "Hourl;     !- Domain Simulation Interval. (Timestep/Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    EXPECT_FALSE(process_idf(idf_objects, false));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}
#endif // GET_OUT

TEST_F(EnergyPlusFixture, SiteGroundDomainBasement_CheckInputs_CorrectInputs)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Basement,",
        "CoupledBasement,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Horizontal Underfloor Insulation Present (Yes/No)",
        "Dummy Material,        !- Basement Horizontal Insulation Underfloor Material Name",
        "Perimeter,             !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        "1,                     !- Perimeter width (m)",
        "2.5,                   !- Depth of Basement Wall In Ground Domain {m}",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Wall Vertical Insulation Present(Yes/No)",
        "Dummy Material,        !- Basement Wall Vertical Insulation Material Name",
        "2.3,                   !- Vertical insulation depth from surface (m)",
        "timestep;              !- Domain Update interval. (Timestep, Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadBasementInputs(*state, 1, 1, errorsFound);

    EXPECT_FALSE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainBasement_CheckInputs_BadOSCMName)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Basement,",
        "CoupledBasement,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSC,      !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Horizontal Underfloor Insulation Present (Yes/No)",
        "Dummy Material,        !- Basement Horizontal Insulation Underfloor Material Name",
        "Perimeter,             !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        "1,                     !- Perimeter width (m)",
        "2.5,                   !- Depth of Basement Wall In Ground Domain {m}",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Wall Vertical Insulation Present(Yes/No)",
        "Dummy Material,        !- Basement Wall Vertical Insulation Material Name",
        "2.3,                   !- Vertical insulation depth from surface (m)",
        "timestep;              !- Domain Update interval. (Timestep, Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadBasementInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainBasement_CheckInputs_BadHorizInsSelection)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Basement,",
        "CoupledBasement,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Ye,                    !- Basement Horizontal Underfloor Insulation Present (Yes/No)",
        "Dummy Material,        !- Basement Horizontal Insulation Underfloor Material Name",
        "Perimeter,             !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        "1,                     !- Perimeter width (m)",
        "2.5,                   !- Depth of Basement Wall In Ground Domain {m}",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Wall Vertical Insulation Present(Yes/No)",
        "Dummy Material,        !- Basement Wall Vertical Insulation Material Name",
        "2.3,                   !- Vertical insulation depth from surface (m)",
        "timestep;              !- Domain Update interval. (Timestep, Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    EXPECT_FALSE(process_idf(idf_objects, false));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadBasementInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainBasement_CheckInputs_BadHorizInsMaterial)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Basement,",
        "CoupledBasement,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Horizontal Underfloor Insulation Present (Yes/No)",
        "Dummy Materia,         !- Basement Horizontal Insulation Underfloor Material Name",
        "Perimeter,             !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        "1,                     !- Perimeter width (m)",
        "2.5,                   !- Depth of Basement Wall In Ground Domain {m}",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Wall Vertical Insulation Present(Yes/No)",
        "Dummy Material,        !- Basement Wall Vertical Insulation Material Name",
        "2.3,                   !- Vertical insulation depth from surface (m)",
        "timestep;              !- Domain Update interval. (Timestep, Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadBasementInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainBasement_CheckInputs_BadHorizInsExtents)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Basement,",
        "CoupledBasement,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Horizontal Underfloor Insulation Present (Yes/No)",
        "Dummy Material,            !- Basement Horizontal Insulation Underfloor Material Name",
        "Perimete,              !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        "1,                     !- Perimeter width (m)",
        "2.5,                   !- Depth of Basement Wall In Ground Domain {m}",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Wall Vertical Insulation Present(Yes/No)",
        "Dummy Material,        !- Basement Wall Vertical Insulation Material Name",
        "2.3,                   !- Vertical insulation depth from surface (m)",
        "timestep;              !- Domain Update interval. (Timestep, Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    EXPECT_FALSE(process_idf(idf_objects, false));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadBasementInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainBasement_CheckInputs_BadBasementDepth)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Basement,",
        "CoupledBasement,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Horizontal Underfloor Insulation Present (Yes/No)",
        "Dummy Material,        !- Basement Horizontal Insulation Underfloor Material Name",
        "Perimeter,             !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        "1,                     !- Perimeter width (m)",
        "25,                    !- Depth of Basement Wall In Ground Domain {m}",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Wall Vertical Insulation Present(Yes/No)",
        "Dummy Material,        !- Basement Wall Vertical Insulation Material Name",
        "2.3,                   !- Vertical insulation depth from surface (m)",
        "timestep;              !- Domain Update interval. (Timestep, Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadBasementInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainBasement_CheckInputs_BadFloorOSCMName)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Basement,",
        "CoupledBasement,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Horizontal Underfloor Insulation Present (Yes/No)",
        "Dummy Material,        !- Basement Horizontal Insulation Underfloor Material Name",
        "Perimeter,             !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        "1,                     !- Perimeter width (m)",
        "2.5,                   !- Depth of Basement Wall In Ground Domain {m}",
        "GroundCoupledOSC,      !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Wall Vertical Insulation Present(Yes/No)",
        "Dummy Material,        !- Basement Wall Vertical Insulation Material Name",
        "2.3,                   !- Vertical insulation depth from surface (m)",
        "timestep;              !- Domain Update interval. (Timestep, Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadBasementInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainBasement_CheckInputs_BadVertInsSelection)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Basement,",
        "CoupledBasement,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Horizontal Underfloor Insulation Present (Yes/No)",
        "Dummy Material,        !- Basement Horizontal Insulation Underfloor Material Name",
        "Perimeter,             !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        "1,                     !- Perimeter width (m)",
        "2.5,                   !- Depth of Basement Wall In Ground Domain {m}",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Ye,                    !- Basement Wall Vertical Insulation Present(Yes/No)",
        "Dummy Material,        !- Basement Wall Vertical Insulation Material Name",
        "2.3,                   !- Vertical insulation depth from surface (m)",
        "timestep;              !- Domain Update interval. (Timestep, Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    EXPECT_FALSE(process_idf(idf_objects, false));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadBasementInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainBasement_CheckInputs_BadVertInsName)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Basement,",
        "CoupledBasement,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Horizontal Underfloor Insulation Present (Yes/No)",
        "Dummy Material,        !- Basement Horizontal Insulation Underfloor Material Name",
        "Perimeter,             !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        "1,                     !- Perimeter width (m)",
        "2.5,                   !- Depth of Basement Wall In Ground Domain {m}",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Wall Vertical Insulation Present(Yes/No)",
        "Dummy Materia,         !- Basement Wall Vertical Insulation Material Name",
        "2.3,                   !- Vertical insulation depth from surface (m)",
        "timestep;              !- Domain Update interval. (Timestep, Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadBasementInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}

#ifdef GET_OUT // TimeStep is not actually used
TEST_F(EnergyPlusFixture, SiteGroundDomainBasement_CheckInputs_BadTimestepSelection)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Basement,",
        "CoupledBasement,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Horizontal Underfloor Insulation Present (Yes/No)",
        "Dummy Material,        !- Basement Horizontal Insulation Underfloor Material Name",
        "Perimeter,             !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        "1,                     !- Perimeter width (m)",
        "2.5,                   !- Depth of Basement Wall In Ground Domain {m}",
        "GroundCoupledOSCM,     !- Name of Basement Floor Boundary Condition Model",
        "Yes,                   !- Basement Wall Vertical Insulation Present(Yes/No)",
        "Dummy Material,        !- Basement Wall Vertical Insulation Material Name",
        "2.3,                   !- Vertical insulation depth from surface (m)",
        "timeste;               !- Domain Update interval. (Timestep, Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",
        "Material,",
        "Dummy Material, !- Name",
        "MediumRough,   !- Roughness",
        "0.1397,        !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",
    });

    EXPECT_FALSE(process_idf(idf_objects, false));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadBasementInputs(*state, 1, 1, errorsFound);

    EXPECT_TRUE(errorsFound);
}
#endif // GET_OUT

TEST_F(EnergyPlusFixture, PipingSystemFullSimulation)
{

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Basement,",
        "  CoupledBasement, !- Name",
        "  5,               !- Ground Domain Depth {m}",
        "  1,               !- Aspect Ratio",
        "  5,               !- Domain Perimeter Offset {m}",
        "  1.8,         !- Soil Thermal Conductivity {W/m-K}",
        "  3200,            !- Soil Density {kg/m3}",
        "  836,         !- Soil Specific Heat {J/kg-K}",
        "  30,          !- Soil Moisture Content Volume Fraction {percent}",
        "  50,          !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "  Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Type of Undisturbed Ground Temperature Model",
        "  KATemps,     !- Name of Undisturbed Ground Temperature Model",
        "  1,               !- Evapotranspiration Ground Cover Parameter",
        "  GroundCoupledOSCM,       !- Name of Basement Floor Boundary Condition Model",
        "  Yes,                 !- Basement Horizontal Underfloor Insulation Present (Yes/No)",
        "  Dummy Material,      !- Basement Horizontal Insulation Underfloor Material Name",
        "  Perimeter,               !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        "  1,                       !- Perimeter width (m)",
        "  2.5,                 !- Depth of Basement Wall In Ground Domain {m}",
        "  GroundCoupledOSCM,       !- Name of Basement Floor Boundary Condition Model",
        "  Yes,                 !- Basement Wall Vertical Insulation Present(Yes/No)",
        "  Dummy Material,      !- Basement Wall Vertical Insulation Material Name",
        "  2.3,                 !- Vertical insulation depth from surface (m)",
        "  timestep;                !- Domain Update interval. (Timestep, Hourly)",

        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "  KATemps,     !- Name of object",
        "  1.8,         !- Soil Thermal Conductivity {W/m-K}",
        "  3200,            !- Soil Density {kg/m3}",
        "  836,         !- Soil Specific Heat {J/kg-K}",
        "  15.5,            !- Annual average surface temperature {C}",
        "  12.8,            !- Annual amplitude of surface temperature {delta C}",
        "  17.3;            !- Phase shift of minimum surface temperature {days}",

        "SurfaceProperty:OtherSideConditionsModel,",
        "  GroundCoupledOSCM,       !- Name",
        "  GroundCoupledSurface;    !- Type of Modeling",
        "  Material,",
        "  Dummy Material, !- Name",
        "  MediumRough, !- Roughness",
        "  0.1397,      !- Thickness {m}",
        "  1.8,         !- Conductivity {W/m-K}",
        "  2400,            !- Density {kg/m3}",
        "  750,         !- Specific Heat {J/kg-K}",
        "  0.9,         !- Thermal Absorptance",
        "  0.65,            !- Solar Absorptance",
        "  0.65;            !- Visible Absorptance",

        "PipingSystem:Underground:Domain,",
        "  My Piping System,        !- Name",
        "  4,                       !- Xmax {m}",
        "  2.5,                     !- Ymax {m}",
        "  75,                      !- Zmax {m}",
        "  2,                       !- X-Direction Mesh Density Parameter",
        "  Uniform,                 !- X-Direction Mesh Type",
        "  ,                        !- X-Direction Geometric Coefficient",
        "  2,                       !- Y-Direction Mesh Density Parameter",
        "  Uniform,                 !- Y-Direction Mesh Type",
        "  ,                        !- Y-Direction Geometric Coefficient",
        "  6,                       !- Z-Direction Mesh Density Parameter",
        "  Uniform,                 !- Z-Direction Mesh Type",
        "  ,                        !- Z-Direction Geometric Coefficient",
        "  1.08,                    !- Soil Thermal Conductivity {W/m-K}",
        "  962,                     !- Soil Density {kg/m3}",
        "  2576,                    !- Soil Specific Heat {J/kg-K}",
        "  30,                      !- Soil Moisture Content Volume Fraction {percent}",
        "  50,                      !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "  Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
        "  KATemps,                 !- Undisturbed Ground Temperature Model Name",
        "  No,                      !- This Domain Includes Basement Surface Interaction",
        "  ,                        !- Width of Basement Floor in Ground Domain {m}",
        "  ,                        !- Depth of Basement Wall In Ground Domain {m}",
        "  ,                        !- Shift Pipe X Coordinates By Basement Width",
        "  ,                        !- Name of Basement Wall Boundary Condition Model",
        "  ,                        !- Name of Basement Floor Boundary Condition Model",
        "  0.005,                   !- Convergence Criterion for the Outer Cartesian Domain Iteration Loop {deltaC}",
        "  100,                     !- Maximum Iterations in the Outer Cartesian Domain Iteration Loop",
        "  0.408,                   !- Evapotranspiration Ground Cover Parameter",
        "  1,                       !- Number of Pipe Circuits Entered for this Domain",
        "  My Pipe Circuit;         !- Pipe Circuit 1",

        "PipingSystem:Underground:PipeCircuit,",
        "  My Pipe Circuit,         !- Name",
        "  0.3895,                  !- Pipe Thermal Conductivity {W/m-K}",
        "  641,                     !- Pipe Density {kg/m3}",
        "  2405,                    !- Pipe Specific Heat {J/kg-K}",
        "  0.016,                   !- Pipe Inner Diameter {m}",
        "  0.02667,                 !- Pipe Outer Diameter {m}",
        "  0.004,                   !- Design Flow Rate {m3/s}",
        "  Plant Supply Intermediate Node,  !- Circuit Inlet Node",
        "  Plant Supply Outlet Node,!- Circuit Outlet Node",
        "  0.001,                   !- Convergence Criterion for the Inner Radial Iteration Loop {deltaC}",
        "  100,                     !- Maximum Iterations in the Inner Radial Iteration Loop",
        "  2,                       !- Number of Soil Nodes in the Inner Radial Near Pipe Mesh Region",
        "  0.03,                    !- Radial Thickness of Inner Radial Near Pipe Mesh Region",
        "  2,                       !- Number of Pipe Segments Entered for this Pipe Circuit",
        "  Segment 1,               !- Pipe Segment 1",
        "  Segment 2;               !- Pipe Segment 2",

        "PipingSystem:Underground:PipeSegment,",
        "  Segment 1,               !- Name",
        "  1.95,                    !- X Position {m}",
        "  1.25,                    !- Y Position {m}",
        "  IncreasingZ;             !- Flow Direction",

        "PipingSystem:Underground:PipeSegment,",
        "  Segment 2,               !- Name",
        "  2.05,                    !- X Position {m}",
        "  1.25,                    !- Y Position {m}",
        "  DecreasingZ;             !- Flow Direction",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Setup the plant itself manually
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::PipingSystemPipeCircuit;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Name = "MY PIPE CIRCUIT";
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumIn = 1;

    // Dummy surface
    state->dataSurface->TotSurfaces = 1;
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    // Other necessary inputs
    bool errorsFound = false;
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    // first call the factory, it will call GetInput
    bool initLoopEquip = true;
    PlantComponent *thisCircuit =
        PlantPipingSystemsManager::Circuit::factory(*state, DataPlant::PlantEquipmentType::PipingSystemPipeCircuit, "MY PIPE CIRCUIT");

    EXPECT_EQ(2u, state->dataPlantPipingSysMgr->domains.size());

    EXPECT_TRUE(state->dataPlantPipingSysMgr->domains[0].HasAPipeCircuit);
    EXPECT_EQ(2, state->dataPlantPipingSysMgr->domains[0].Mesh.X.RegionMeshCount);
    EXPECT_EQ(2, state->dataPlantPipingSysMgr->domains[0].Mesh.Y.RegionMeshCount);
    EXPECT_EQ(6, state->dataPlantPipingSysMgr->domains[0].Mesh.Z.RegionMeshCount);

    EXPECT_FALSE(state->dataPlantPipingSysMgr->domains[1].HasAPipeCircuit);
    EXPECT_EQ(4, state->dataPlantPipingSysMgr->domains[1].Mesh.X.RegionMeshCount);
    EXPECT_EQ(4, state->dataPlantPipingSysMgr->domains[1].Mesh.Y.RegionMeshCount);
    EXPECT_EQ(4, state->dataPlantPipingSysMgr->domains[1].Mesh.Z.RegionMeshCount);

    // second call, turn off initLoopEquip so it tries to do a simulation
    initLoopEquip = false;
    EnergyPlus::PlantLocation myLocation = EnergyPlus::PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    Real64 curLoad = 0.0;
    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->SOLCOS(3) = 1.0;
    thisCircuit->simulate(*state, myLocation, true, curLoad, true);

    // we can also try to call from the Domain side
    PlantPipingSystemsManager::SimulateGroundDomains(*state, false);
}

// * Asserts that all objects that defined both an inside and an outside diameters have correct input (inside < outside)
// * - PipingSystem:Underground:PipeCircuit
// * - GroundHeatExchanger:HorizontalTrench

TEST_F(EnergyPlusFixture, PipingSystem_Check_Correct_Pipe_Diameters)
{

    std::string const idf_objects = delimited_string({

        "PipingSystem:Underground:Domain,",
        "  My Piping System,        !- Name",
        "  4,                       !- Xmax {m}",
        "  2.5,                     !- Ymax {m}",
        "  75,                      !- Zmax {m}",
        "  2,                       !- X-Direction Mesh Density Parameter",
        "  Uniform,                 !- X-Direction Mesh Type",
        "  ,                        !- X-Direction Geometric Coefficient",
        "  2,                       !- Y-Direction Mesh Density Parameter",
        "  Uniform,                 !- Y-Direction Mesh Type",
        "  ,                        !- Y-Direction Geometric Coefficient",
        "  6,                       !- Z-Direction Mesh Density Parameter",
        "  Uniform,                 !- Z-Direction Mesh Type",
        "  ,                        !- Z-Direction Geometric Coefficient",
        "  1.08,                    !- Soil Thermal Conductivity {W/m-K}",
        "  962,                     !- Soil Density {kg/m3}",
        "  2576,                    !- Soil Specific Heat {J/kg-K}",
        "  30,                      !- Soil Moisture Content Volume Fraction {percent}",
        "  50,                      !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "  Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
        "  KATemps,                 !- Undisturbed Ground Temperature Model Name",
        "  No,                      !- This Domain Includes Basement Surface Interaction",
        "  ,                        !- Width of Basement Floor in Ground Domain {m}",
        "  ,                        !- Depth of Basement Wall In Ground Domain {m}",
        "  ,                        !- Shift Pipe X Coordinates By Basement Width",
        "  ,                        !- Name of Basement Wall Boundary Condition Model",
        "  ,                        !- Name of Basement Floor Boundary Condition Model",
        "  0.005,                   !- Convergence Criterion for the Outer Cartesian Domain Iteration Loop {deltaC}",
        "  100,                     !- Maximum Iterations in the Outer Cartesian Domain Iteration Loop",
        "  0.408,                   !- Evapotranspiration Ground Cover Parameter",
        "  1,                       !- Number of Pipe Circuits Entered for this Domain",
        "  My Pipe Circuit;         !- Pipe Circuit 1",

        "PipingSystem:Underground:PipeCircuit,",
        "  My Pipe Circuit,         !- Name",
        "  0.3895,                  !- Pipe Thermal Conductivity {W/m-K}",
        "  641,                     !- Pipe Density {kg/m3}",
        "  2405,                    !- Pipe Specific Heat {J/kg-K}",

        // HERE we define wrong diameters
        "  0.016,                   !- Pipe Inner Diameter {m}",
        "  0.012,                   !- Pipe Outer Diameter {m}",

        "  0.004,                   !- Design Flow Rate {m3/s}",
        "  Plant Supply PipeCircuit Inlet Node,  !- Circuit Inlet Node",
        "  Plant Supply PipeCircuit Outlet Node,!- Circuit Outlet Node",
        "  0.001,                   !- Convergence Criterion for the Inner Radial Iteration Loop {deltaC}",
        "  100,                     !- Maximum Iterations in the Inner Radial Iteration Loop",
        "  2,                       !- Number of Soil Nodes in the Inner Radial Near Pipe Mesh Region",
        "  0.03,                    !- Radial Thickness of Inner Radial Near Pipe Mesh Region",
        "  2,                       !- Number of Pipe Segments Entered for this Pipe Circuit",
        "  Segment 1,               !- Pipe Segment 1",
        "  Segment 2;               !- Pipe Segment 2",

        "PipingSystem:Underground:PipeSegment,",
        "  Segment 1,               !- Name",
        "  1.95,                    !- X Position {m}",
        "  1.25,                    !- Y Position {m}",
        "  IncreasingZ;             !- Flow Direction",

        "PipingSystem:Underground:PipeSegment,",
        "  Segment 2,               !- Name",
        "  2.05,                    !- X Position {m}",
        "  1.25,                    !- Y Position {m}",
        "  DecreasingZ;             !- Flow Direction",

        "GroundHeatExchanger:HorizontalTrench,",
        "  GHX Horizontal Trench,   !- Name",
        "  Plant Supply Trench Inlet Node,  !- Inlet Node Name",
        "  Plant Supply Trench Outlet Node,!- Outlet Node Name",
        "  0.004,                   !- Design Flow Rate {m3/s}",
        "  75,                      !- Trench Length in Pipe Axial Direction {m}",
        "  2,                       !- Number of Trenches",
        "  2.0,                     !- Horizontal Spacing Between Pipes {m}",

        // HERE we define wrong diameters
        "  0.015,                   !- Pipe Inner Diameter {m}",
        "  0.011,                   !- Pipe Outer Diameter {m}",

        "  1.25,                    !- Burial Depth {m}",
        "  1.08,                    !- Soil Thermal Conductivity {W/m-K}",
        "  962,                     !- Soil Density {kg/m3}",
        "  2576,                    !- Soil Specific Heat {J/kg-K}",
        "  0.3895,                  !- Pipe Thermal Conductivity {W/m-K}",
        "  641,                     !- Pipe Density {kg/m3}",
        "  2405,                    !- Pipe Specific Heat {J/kg-K}",
        "  30,                      !- Soil Moisture Content Percent {percent}",
        "  50,                      !- Soil Moisture Content Percent at Saturation {percent}",
        "  Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
        "  KATemps,                 !- Undisturbed Ground Temperature Model Name",
        "  0.408;                   !- Evapotranspiration Ground Cover Parameter",

        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "  KATemps,                 !- Name",
        "  1.08,                    !- Soil Thermal Conductivity {W/m-K}",
        "  962,                     !- Soil Density {kg/m3}",
        "  2576,                    !- Soil Specific Heat {J/kg-K}",
        "  15.5,                    !- Average Soil Surface Temperature {C}",
        "  12.8,                    !- Average Amplitude of Surface Temperature {deltaC}",
        "  17.3;                    !- Phase Shift of Minimum Surface Temperature {days}",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound = false;
    PlantPipingSystemsManager::ReadPipeCircuitInputs(*state, ErrorsFound);
    EXPECT_TRUE(ErrorsFound);

    std::string error_string = delimited_string({
        R"(   ** Severe  ** ReadPipeCircuitInputs:PipingSystem:Underground:PipeCircuit="MY PIPE CIRCUIT", invalid Pipe Outer Diameter="0.012", Condition: Outer diameter must be greater than inner diameter.)",
        R"(   ** Severe  ** ReadPipeCircuitInputs: GroundHeatExchanger:HorizontalTrench="GHX HORIZONTAL TRENCH" has invalid pipe diameters.)",
        R"(   **   ~~~   ** Outer diameter [0.011] must be greater than inner diameter [0.015].)",
    });
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, PipingSystem_SiteGroundDomainUsingNoMassMatTest)
{

    bool TestResult;
    bool ExpectedResult;
    Real64 Thickness;
    int MaterialIndex;

    Material::MaterialBase *p = new Material::MaterialBase;
    state->dataMaterial->materials.push_back(p);

    // Test 1: Material has a valid thickness and is not R-only, result should be false
    MaterialIndex = 1;
    auto *thisMaterial = state->dataMaterial->materials(MaterialIndex);
    thisMaterial->ROnly = false;
    Thickness = 0.01;
    ExpectedResult = false;
    TestResult = SiteGroundDomainUsingNoMassMat(*state, Thickness, MaterialIndex);

    EXPECT_EQ(TestResult, ExpectedResult);

    // Test 2a: Material has a valid thickness but is R-only, result should be true
    //         Note that generally this case would not be encountered in EnergyPlus
    thisMaterial->ROnly = true;
    Thickness = 0.01;
    ExpectedResult = true;
    TestResult = SiteGroundDomainUsingNoMassMat(*state, Thickness, MaterialIndex);

    EXPECT_EQ(TestResult, ExpectedResult);

    // Test 2b: Material does not have a valid thickness but is not R-only, result should be true
    //         Note that generally this case would not be encountered in EnergyPlus
    thisMaterial->ROnly = false;
    Thickness = 0.0;
    ExpectedResult = true;
    TestResult = SiteGroundDomainUsingNoMassMat(*state, Thickness, MaterialIndex);

    EXPECT_EQ(TestResult, ExpectedResult);

    // Test 3: Material does not have a valid thickness and is not R-only, result should be true
    thisMaterial->ROnly = true;
    Thickness = 0.0;
    ExpectedResult = true;
    TestResult = SiteGroundDomainUsingNoMassMat(*state, Thickness, MaterialIndex);

    EXPECT_EQ(TestResult, ExpectedResult);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_Fix_HorizInsDepth)
{
    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Slab,",
        "CoupledSlab,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM, !- Name of Floor Boundary Condition Model",
        "InGrade,       !- Slab Location (InGrade/OnGrade)",
        "SlabMaterial,  !- Slab Material Name",
        "Yes,           !- Horizontal Insulation (Yes/No)",
        "HorizInsulation,   !- Horizontal Insulation Material Name",
        "Full,          !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        ",              !- Perimeter insulation width (m)",
        "Yes,           !- Vertical Insulation (Yes/No)",
        "VertiInsulation,   !- Vertical Insulation Name",
        "0.5,           !- Vertical perimeter insulation depth from surface (m)",
        "Hourly;        !- Domain Simulation Interval. (Timestep/Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",

        "Material,",
        "SlabMaterial,  !- Name",
        "MediumRough,   !- Roughness",
        "0.2,           !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",

        "Material,",
        "HorizInsulation,         !- Name",
        "Rough,                   !- Roughness",
        "0.1,                     !- Thickness {m}",
        "0.04,                    !- Conductivity {W/m-K}",
        "15,                      !- Density {kg/m3}",
        "1300,                    !- Specific Heat {J/kg-K}",
        "0.9,                     !- Thermal Absorptance",
        "0.6,                     !- Solar Absorptance",
        "0.6;                     !- Visible Absorptance",

        "Material,",
        "VertiInsulation,         !- Name",
        "Rough,                   !- Roughness",
        "0.15,                    !- Thickness {m}",
        "0.04,                    !- Conductivity {W/m-K}",
        "15,                      !- Density {kg/m3}",
        "1300,                    !- Specific Heat {J/kg-K}",
        "0.9,                     !- Thermal Absorptance",
        "0.6,                     !- Solar Absorptance",
        "0.6;                     !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound);

    // 2021-08: revised the case so no more error complains
    EXPECT_FALSE(errorsFound);

    auto &theDomain = state->dataPlantPipingSysMgr->domains[0];

    theDomain.createPartitionCenterList(*state);

    // Take the logic in the code about how the partition positions were calculated
    // and how they were affected by the horizontal thickness
    Real64 insyLoc = theDomain.Extents.yMax - theDomain.VertInsDepth + theDomain.HorizInsThickness / 2.0;
    Real64 slabBot = theDomain.Extents.yMax - theDomain.SlabThickness - theDomain.HorizInsThickness / 2.0;
    Real64 totalWid = theDomain.HorizInsThickness;

    int partySize = theDomain.Partitions.Y.size();

    Real64 err_tol = 1.0e-4;

    // check horizontal partitions for this case
    EXPECT_NEAR(theDomain.Partitions.Y[partySize - 2].rDimension, 4.55, err_tol);
    EXPECT_NEAR(theDomain.Partitions.Y[partySize - 2].rDimension, insyLoc, err_tol);

    EXPECT_NEAR(theDomain.Partitions.Y[partySize - 2].TotalWidth, 0.10, err_tol);
    EXPECT_NEAR(theDomain.Partitions.Y[partySize - 2].TotalWidth, totalWid, err_tol);

    EXPECT_TRUE(theDomain.Partitions.Y[partySize - 2].partitionType == PartitionType::VertInsLowerEdge);

    EXPECT_NEAR(theDomain.Partitions.Y[partySize - 1].rDimension, 4.75, err_tol);
    EXPECT_NEAR(theDomain.Partitions.Y[partySize - 1].rDimension, slabBot, err_tol);

    EXPECT_NEAR(theDomain.Partitions.Y[partySize - 1].TotalWidth, 0.10, err_tol);
    EXPECT_NEAR(theDomain.Partitions.Y[partySize - 1].TotalWidth, totalWid, err_tol);

    EXPECT_TRUE(theDomain.Partitions.Y[partySize - 1].partitionType == PartitionType::UnderFloor);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_Fix_HorizInsDepth_Test2)
{

    // Test another configuration using a different Horizontal Insulation thickness
    // with an Horizontal Insulation thickness of 0.5m

    std::string const idf_objects = delimited_string({
        "Site:GroundDomain:Slab,",
        "CoupledSlab,   !- Name",
        "5,             !- Ground Domain Depth {m}",
        "1,             !- Aspect Ratio",
        "5,             !- Domain Perimeter Offset {m}",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "30,            !- Soil Moisture Content Volume Fraction {percent}",
        "50,            !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,    !- Type of Undisturbed Ground Temperature Model",
        "KATemps,       !- Name of Undisturbed Ground Temperature Model",
        "1,             !- Evapotranspiration Ground Cover Parameter",
        "GroundCoupledOSCM, !- Name of Floor Boundary Condition Model",
        "InGrade,       !- Slab Location (InGrade/OnGrade)",
        "SlabMaterial,  !- Slab Material Name",
        "Yes,           !- Horizontal Insulation (Yes/No)",
        "HorizInsulation,   !- Horizontal Insulation Material Name",
        "Full,          !- Full Horizontal or Perimeter Only (Full/Perimeter)",
        ",              !- Perimeter insulation width (m)",
        "Yes,           !- Vertical Insulation (Yes/No)",
        "VertiInsulation,   !- Vertical Insulation Name",
        "0.5,           !- Vertical perimeter insulation depth from surface (m)",
        "Hourly;        !- Domain Simulation Interval. (Timestep/Hourly)",
        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "KATemps,       !- Name of object",
        "1.8,           !- Soil Thermal Conductivity {W/m-K}",
        "3200,          !- Soil Density {kg/m3}",
        "836,           !- Soil Specific Heat {J/kg-K}",
        "15.5,          !- Annual average surface temperature {C}",
        "12.8,          !- Annual amplitude of surface temperature {delta C}",
        "17.3;          !- Phase shift of minimum surface temperature {days}",
        "SurfaceProperty:OtherSideConditionsModel,",
        "GroundCoupledOSCM,     !- Name",
        "GroundCoupledSurface;  !- Type of Modeling",

        "Material,",
        "SlabMaterial,  !- Name",
        "MediumRough,   !- Roughness",
        "0.2,           !- Thickness {m}",
        "1.8,           !- Conductivity {W/m-K}",
        "2400,          !- Density {kg/m3}",
        "750,           !- Specific Heat {J/kg-K}",
        "0.9,           !- Thermal Absorptance",
        "0.65,          !- Solar Absorptance",
        "0.65;          !- Visible Absorptance",

        "Material,",
        "HorizInsulation,         !- Name",
        "Rough,                   !- Roughness",
        "0.2,                     !- Thickness {m}",
        "0.04,                    !- Conductivity {W/m-K}",
        "15,                      !- Density {kg/m3}",
        "1300,                    !- Specific Heat {J/kg-K}",
        "0.9,                     !- Thermal Absorptance",
        "0.6,                     !- Solar Absorptance",
        "0.6;                     !- Visible Absorptance",

        "Material,",
        "VertiInsulation,         !- Name",
        "Rough,                   !- Roughness",
        "0.15,                    !- Thickness {m}",
        "0.04,                    !- Conductivity {W/m-K}",
        "15,                      !- Density {kg/m3}",
        "1300,                    !- Specific Heat {J/kg-K}",
        "0.9,                     !- Thermal Absorptance",
        "0.6,                     !- Solar Absorptance",
        "0.6;                     !- Visible Absorptance",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    bool errorsFound = false;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound);

    // 2021-08: revised the case so no more error complains
    EXPECT_FALSE(errorsFound);

    auto &theDomain = state->dataPlantPipingSysMgr->domains[0];

    theDomain.createPartitionCenterList(*state);

    // Take the logic in the code about how the partition positions were calculated
    // and how they were affected by the horizontal thickness
    Real64 insyLoc = theDomain.Extents.yMax - theDomain.VertInsDepth + theDomain.HorizInsThickness / 2.0;
    Real64 slabBot = theDomain.Extents.yMax - theDomain.SlabThickness - theDomain.HorizInsThickness / 2.0;
    Real64 totalWid = theDomain.HorizInsThickness;

    int partySize = theDomain.Partitions.Y.size();

    Real64 err_tol = 1.0e-4;

    // check horizontal partitions for the new case
    // and expect that the results should be different from the previous case

    EXPECT_NEAR(theDomain.Partitions.Y[partySize - 2].rDimension, insyLoc, err_tol);
    // This is expected to be different from Test 1
    Real64 insyLoc_case1 = 4.55;
    EXPECT_FALSE((insyLoc - insyLoc_case1) <= err_tol && (insyLoc - insyLoc_case1) >= -err_tol);

    EXPECT_NEAR(theDomain.Partitions.Y[partySize - 2].TotalWidth, totalWid, err_tol);
    Real64 totalWid_case1 = 0.10;
    // This is expected to be different from Test 1
    EXPECT_FALSE((totalWid - totalWid_case1) <= err_tol && (totalWid - totalWid_case1) >= -err_tol);

    EXPECT_TRUE(theDomain.Partitions.Y[partySize - 2].partitionType == PartitionType::VertInsLowerEdge);

    EXPECT_NEAR(theDomain.Partitions.Y[partySize - 1].rDimension, slabBot, err_tol);
    // This is expected to be different from Test 1
    Real64 slabBot_case1 = 4.75;
    EXPECT_FALSE((slabBot - slabBot_case1) <= err_tol && (slabBot - slabBot_case1) >= -err_tol);

    EXPECT_NEAR(theDomain.Partitions.Y[partySize - 1].TotalWidth, totalWid, err_tol);

    EXPECT_TRUE(theDomain.Partitions.Y[partySize - 1].partitionType == PartitionType::UnderFloor);
}

TEST_F(EnergyPlusFixture, SiteGroundDomainSlab_FiniteDifference)
{

    std::string const idf_objects = delimited_string({

        "Site:GroundTemperature:Undisturbed:FiniteDifference,",
        "  Ground,                  !- Name",
        "  1,                       !- Soil Thermal Conductivity {W/m-K}",
        "  1250,                    !- Soil Density {kg/m3}",
        "  1200,                    !- Soil Specific Heat {J/kg-K}",
        "  30,                      !- Soil Moisture Content Volume Fraction {percent}",
        "  50,                      !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "  ;                        !- Evapotranspiration Ground Cover Parameter {dimensionless}",

        "Site:GroundDomain:Slab,",
        "  Slab,                    !- Name",
        "  10,                      !- Ground Domain Depth {m}",
        "  1,                       !- Aspect Ratio",
        "  5,                       !- Perimeter Offset {m}",
        "  ,                        !- Soil Thermal Conductivity {W/m-K}",
        "  2800,                    !- Soil Density {kg/m3}",
        "  850,                     !- Soil Specific Heat {J/kg-K}",
        "  30,                      !- Soil Moisture Content Volume Fraction {percent}",
        "  50,                      !- Soil Moisture Content Volume Fraction at Saturation {percent}",
        "  Site:GroundTemperature:Undisturbed:FiniteDifference,  !- Undisturbed Ground Temperature Model Type",
        "  Ground,                  !- Undisturbed Ground Temperature Model Name",
        "  ,                        !- Evapotranspiration Ground Cover Parameter",
        "  GroundDomain,            !- Slab Boundary Condition Model Name",
        "  OnGrade,                 !- Slab Location",
        "  ,                        !- Slab Material Name",
        "  No,                      !- Horizontal Insulation",
        "  ,                        !- Horizontal Insulation Material Name",
        "  Full,                    !- Horizontal Insulation Extents",
        "  ,                        !- Perimeter Insulation Width {m}",
        "  No,                      !- Vertical Insulation",
        "  ,                        !- Vertical Insulation Material Name",
        "  ,                        !- Vertical Insulation Depth {m}",
        "  Hourly,                  !- Simulation Timestep",
        "  ,                        !- Geometric Mesh Coefficient",
        "  6;                       !- Mesh Density Parameter",

        "SurfaceProperty:OtherSideConditionsModel,",
        "  GroundDomain,            !- Name",
        "  GroundCoupledSurface;    !- Type of Modeling",

        "RunPeriod,",
        "  January,                 !- Name",
        "  1,                       !- Begin Month",
        "  1,                       !- Begin Day of Month",
        "  ,                        !- Begin Year",
        "  1,                       !- End Month",
        "  31,                      !- End Day of Month",
        "  ,                        !- End Year",
        "  Tuesday,                 !- Day of Week for Start Day",
        "  Yes,                     !- Use Weather File Holidays and Special Days",
        "  Yes,                     !- Use Weather File Daylight Saving Period",
        "  No,                      !- Apply Weekend Holiday Rule",
        "  Yes,                     !- Use Weather File Rain Indicators",
        "  Yes;                     !- Use Weather File Snow Indicators",
    });

    EXPECT_TRUE(process_idf(idf_objects));

    // Set an actual weather file to Chicago EPW
    state->dataWeather->WeatherFileExists = true;
    state->files.inputWeatherFilePath.filePath = fs::path(configured_source_directory()) / "weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw";

    // Read the project data, such as Timestep
    state->dataGlobal->BeginSimFlag = true;
    SimulationManager::GetProjectData(*state);
    EXPECT_EQ(state->dataGlobal->TimeStepsInHour, 4);

    // Needed to avoid crash in SetupSimulation (from ElectricPowerServiceManager.hh)
    createFacilityElectricPowerServiceObject(*state);

    bool errorsFound = false;
    SimulationManager::SetupSimulation(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    EXPECT_EQ(state->dataWeather->NumOfEnvrn, 1);
    EXPECT_EQ(state->dataEnvrn->TotDesDays, 0);
    EXPECT_EQ(state->dataWeather->TotRunPers, 1);

    // Dummy surface
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).OSCMPtr = 1;
    state->dataSurface->Surface(1).Area = 100;

    // Other necessary inputs
    GetOSCMData(*state);
    GetMaterialData(*state, errorsFound);

    state->dataPlantPipingSysMgr->domains.resize(1);
    EXPECT_NO_THROW(ReadZoneCoupledDomainInputs(*state, 1, 1, errorsFound));

    EXPECT_FALSE(errorsFound);
    EXPECT_TRUE(compare_err_stream(""));
}

TEST_F(EnergyPlusFixture, calcFluxWeightingFactorTest)
{
    Real64 weightedFlux;
    Real64 uniformFlux;
    Real64 actualResult;
    Real64 expectedResult;
    Real64 constexpr allowedTolerance = 0.00001;

    // Test 1a: Everything normal and positive, simple calculation.
    weightedFlux = 1.0;
    uniformFlux = 1.0;
    expectedResult = 1.0;
    actualResult = calcFluxWeightingFactor(weightedFlux, uniformFlux);
    EXPECT_NEAR(actualResult, expectedResult, allowedTolerance);

    // Test 1b: Everything normal and negative, simple calculation.
    weightedFlux = -0.1;
    uniformFlux = -0.5;
    expectedResult = 0.2;
    actualResult = calcFluxWeightingFactor(weightedFlux, uniformFlux);
    EXPECT_NEAR(actualResult, expectedResult, allowedTolerance);

    // Test 2: Zero uniform flux--the condition that caused the problem.  Return a unity value.
    weightedFlux = 1.0;
    uniformFlux = 0.0;
    expectedResult = 1.0;
    actualResult = calcFluxWeightingFactor(weightedFlux, uniformFlux);
    EXPECT_NEAR(actualResult, expectedResult, allowedTolerance);
}
