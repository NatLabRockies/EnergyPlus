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

// EnergyPlus::ICS collector un-allocated collector data bug fix test

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/TranspiredCollector.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::DataSurfaces;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataHeatBalSurface;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::DataEnvironment;

TEST_F(EnergyPlusFixture, ICSSolarCollectorTest_CalcPassiveExteriorBaffleGapTest)
{
    // ICS collector un-allocated collector data bug fix test.  This unit test
    // does not test ICS collector performance but it does test a bug fix for
    // issue #4723 (crash) occurred due to unallocated ICS collector data.
    // ! Collector.allocated()

    int constexpr NumOfSurf(1);
    int SurfNum;
    int ZoneNum;
    int ConstrNum;
    int MatNum;

    state->init_state(*state);

    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->SkyTemp = 24.0;
    state->dataEnvrn->IsRain = false;
    MatNum = 1;
    ZoneNum = 1;
    SurfNum = 1;
    ConstrNum = 1;
    // allocate surface variable data
    state->dataSurface->Surface.allocate(NumOfSurf);
    state->dataSurface->SurfOutDryBulbTemp.allocate(NumOfSurf);
    state->dataSurface->SurfOutWetBulbTemp.allocate(NumOfSurf);
    state->dataSurface->SurfOutWindSpeed.allocate(NumOfSurf);
    state->dataSurface->SurfOutWindDir.allocate(NumOfSurf);
    state->dataSurface->Surface(SurfNum).Area = 10.0;
    state->dataSurface->SurfOutDryBulbTemp(SurfNum) = 20.0;
    state->dataSurface->SurfOutWetBulbTemp(SurfNum) = 15.0;
    state->dataSurface->SurfOutWindSpeed(SurfNum) = 3.0;
    state->dataSurface->Surface(SurfNum).Construction = ConstrNum;
    state->dataSurface->Surface(SurfNum).BaseSurf = SurfNum;
    state->dataSurface->Surface(SurfNum).Zone = ZoneNum;
    state->dataSurface->Surface(SurfNum).ExtWind = false;
    state->dataSurface->SurfIsICS.allocate(NumOfSurf);
    state->dataSurface->SurfICSPtr.allocate(NumOfSurf);
    state->dataSurface->SurfIsICS(SurfNum) = true;

    // allocate construction variable data
    state->dataConstruction->Construct.allocate(ConstrNum);
    state->dataConstruction->Construct(ConstrNum).LayerPoint.allocate(MatNum);
    state->dataConstruction->Construct(ConstrNum).LayerPoint(MatNum) = 1;
    auto *p = new Material::MaterialBase;
    state->dataMaterial->materials.push_back(p);
    p->AbsorpThermal = 0.8;
    // allocate exterior vented cavity variable data
    state->dataHeatBal->ExtVentedCavity.allocate(1);
    state->dataHeatBal->ExtVentedCavity(NumOfSurf).SurfPtrs.allocate(NumOfSurf);
    state->dataHeatBal->ExtVentedCavity(NumOfSurf).SurfPtrs(NumOfSurf) = 1;
    // allocate zone variable data
    state->dataHeatBal->Zone.allocate(ZoneNum);
    state->dataHeatBal->Zone(ZoneNum).ExtConvAlgo = Convect::HcExt::ASHRAESimple;
    // allocate surface temperature variable data
    state->dataHeatBalSurf->SurfOutsideTempHist.allocate(1);
    state->dataHeatBalSurf->SurfOutsideTempHist(1).allocate(NumOfSurf);
    state->dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) = 22.0;
    // allocate solar incident radiation variable data
    state->dataHeatBal->SurfQRadSWOutIncident.allocate(1);
    state->dataHeatBal->SurfQRadSWOutIncident(1) = 0.0;
    // set user defined conv. coeff. calculation to false
    state->dataConvect->GetUserSuppliedConvectionCoeffs = false;
    state->dataHeatBalSurf->SurfWinCoeffAdjRatio.dimension(NumOfSurf, 1.0);
    state->dataSurface->surfExtConv.allocate(NumOfSurf);
    state->dataSurface->surfExtConv(SurfNum).model = Convect::HcExt::SetByZone;
    state->dataSurface->SurfEMSOverrideExtConvCoef.allocate(NumOfSurf);
    state->dataSurface->SurfEMSOverrideExtConvCoef(1) = false;
    auto &surface = state->dataSurface->Surface(SurfNum);
    surface.IsSurfPropertyGndSurfacesDefined = false;
    surface.UseSurfPropertyGndSurfTemp = false;
    surface.UseSurfPropertyGndSurfRefl = false;
    surface.SurfHasSurroundingSurfProperty = false;

    // SurfPtr( 1 ); // Array of indexes pointing to Surface structure in DataSurfaces
    Real64 constexpr VentArea(0.1);  // Area available for venting the gap [m2]
    Real64 constexpr Cv(0.1);        // Orifice coefficient for volume-based discharge, wind-driven [--]
    Real64 constexpr Cd(0.5);        // Orifice coefficient for discharge,  buoyancy-driven [--]
    Real64 constexpr HdeltaNPL(3.0); // Height difference from neutral pressure level [m]
    Real64 constexpr SolAbs(0.75);   // solar absorptivity of baffle [--]
    Real64 constexpr AbsExt(0.8);    // thermal absorptance/emittance of baffle material [--]
    Real64 constexpr Tilt(0.283);    // Tilt of gap [Degrees]
    Real64 constexpr AspRat(0.9);    // aspect ratio of gap  Height/gap [--]
    Real64 constexpr GapThick(0.05); // Thickness of air space between baffle and underlying heat transfer surface
    Material::SurfaceRoughness Roughness(Material::SurfaceRoughness::VeryRough); // Roughness index (1-6), see DataHeatBalance parameters
    Real64 QdotSource(0.0);                                                      // Source/sink term, e.g. electricity exported from solar cell [W]
    Real64 TsBaffle(20.0);                                                       // Temperature of baffle (both sides) use lagged value on input [C]
    Real64 TaGap(22.0); // Temperature of air gap (assumed mixed) use lagged value on input [C]
    Real64 HcGapRpt;    // gap convection coefficient [W/m2C]
    Real64 HrGapRpt;    // gap radiation coefficient [W/m2C]
    Real64 IscRpt;      //
    Real64 MdotVentRpt; // gap air mass flow rate [kg/s]
    Real64 VdotWindRpt; // gap wind driven air volume flow rate [m3/s]
    Real64 VdotBuoyRpt; // gap buoyancy driven volume flow rate [m3/s]

    // call to test fix to resolve crash
    TranspiredCollector::CalcPassiveExteriorBaffleGap(*state,
                                                      state->dataHeatBal->ExtVentedCavity(1).SurfPtrs,
                                                      VentArea,
                                                      Cv,
                                                      Cd,
                                                      HdeltaNPL,
                                                      SolAbs,
                                                      AbsExt,
                                                      Tilt,
                                                      AspRat,
                                                      GapThick,
                                                      Roughness,
                                                      QdotSource,
                                                      TsBaffle,
                                                      TaGap,
                                                      HcGapRpt,
                                                      HrGapRpt,
                                                      IscRpt,
                                                      MdotVentRpt,
                                                      VdotWindRpt,
                                                      VdotBuoyRpt);

    EXPECT_NEAR(21.862, TsBaffle, 0.001);
    EXPECT_NEAR(1.692, HcGapRpt, 0.001);
    EXPECT_NEAR(3.694, HrGapRpt, 0.001);
    EXPECT_NEAR(0.036, MdotVentRpt, 0.001);

    // deallocated variables
    state->dataSurface->Surface.deallocate();
    state->dataConstruction->Construct(ConstrNum).LayerPoint.deallocate();
    state->dataConstruction->Construct.deallocate();

    for (int i = 1; i <= state->dataMaterial->materials.isize(); ++i) {
        delete state->dataMaterial->materials(i);
    }

    state->dataMaterial->materials.deallocate();
    state->dataHeatBal->ExtVentedCavity(NumOfSurf).SurfPtrs.deallocate();
    state->dataHeatBal->ExtVentedCavity.deallocate();
    state->dataHeatBal->Zone.deallocate();
    state->dataHeatBal->SurfQRadSWOutIncident.deallocate();
}
