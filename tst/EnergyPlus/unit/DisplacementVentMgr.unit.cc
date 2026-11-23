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

// EnergyPlus::RoomAirModelUserTempPattern Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DisplacementVentMgr.hh>
#include <EnergyPlus/HeatBalanceAirManager.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::DataSurfaces;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::RoomAir;

TEST_F(EnergyPlusFixture, DisplacementVentMgr_HcUCSDDV_Door_Test)
{

    // set up all conditions entering the HcUCSDDV for Issue #5533 to cause NAN when a floor door is entered
    // 3 door types are tested: wall, floor and ceiling
    state->dataGlobal->NumOfZones = 1;
    int TotSurfaces = 3;

    state->dataRoomAir->IsZoneDispVent3Node.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->IsZoneDispVent3Node(1) = true;
    state->dataSurface->Surface.allocate(TotSurfaces);
    state->dataHeatBal->SurfTempEffBulkAir.allocate(TotSurfaces);
    state->dataHeatBalSurf->SurfTempIn.allocate(TotSurfaces);
    state->dataRoomAir->DispVent3NodeHcIn.allocate(TotSurfaces);
    state->dataRoomAir->ZTMX.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->ZTOC.allocate(state->dataGlobal->NumOfZones);

    // Surface 1 Vertical wall
    state->dataSurface->Surface(1).Name = "Class1_Wall_6_0_0_0_0_0_Subsurface";
    state->dataSurface->Surface(1).Class = SurfaceClass::Door;
    state->dataSurface->Surface(1).Zone = 1;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(1).BaseSurf = 1;
    state->dataSurface->Surface(1).Sides = 4;
    state->dataSurface->Surface(1).Area = 10;
    state->dataSurface->Surface(1).Azimuth = 0;
    state->dataSurface->Surface(1).Tilt = 90;
    state->dataSurface->Surface(1).Vertex.allocate(state->dataSurface->Surface(1).Sides);
    state->dataSurface->Surface(1).Vertex(1).x = -11.57740998;
    state->dataSurface->Surface(1).Vertex(1).y = -12.31054602;
    state->dataSurface->Surface(1).Vertex(1).z = 4.9804;
    state->dataSurface->Surface(1).Vertex(2).x = -10.95990365;
    state->dataSurface->Surface(1).Vertex(2).y = -12.31054602;
    state->dataSurface->Surface(1).Vertex(2).z = 4.9804;
    state->dataSurface->Surface(1).Vertex(3).x = -10.95990365;
    state->dataSurface->Surface(1).Vertex(3).y = -12.31054602;
    state->dataSurface->Surface(1).Vertex(3).z = 5.54536;
    state->dataSurface->Surface(1).Vertex(4).x = -11.57740998;
    state->dataSurface->Surface(1).Vertex(4).y = -12.31054602;
    state->dataSurface->Surface(1).Vertex(4).z = 5.54536;

    // Surface 2 Door floor
    state->dataSurface->Surface(2).Name = "Class1_Floor_9_0_8_0_8_0_Subsurface";
    state->dataSurface->Surface(2).Class = SurfaceClass::Door;
    state->dataSurface->Surface(2).Zone = 1;
    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(2).Construction = 1;
    state->dataSurface->Surface(2).BaseSurf = 1;
    state->dataSurface->Surface(2).Sides = 4;
    state->dataSurface->Surface(2).Area = 10;
    state->dataSurface->Surface(2).Azimuth = 0;
    state->dataSurface->Surface(2).Tilt = 180;
    state->dataSurface->Surface(2).Vertex.allocate(state->dataSurface->Surface(1).Sides);
    state->dataSurface->Surface(2).Vertex(1).x = -4.80163552;
    state->dataSurface->Surface(2).Vertex(1).y = -9.86732154;
    state->dataSurface->Surface(2).Vertex(1).z = 4.9784;
    state->dataSurface->Surface(2).Vertex(2).x = -13.28288246;
    state->dataSurface->Surface(2).Vertex(2).y = -9.86732154;
    state->dataSurface->Surface(2).Vertex(2).z = 4.9784;
    state->dataSurface->Surface(2).Vertex(3).x = -13.28288246;
    state->dataSurface->Surface(2).Vertex(3).y = -1.66421151;
    state->dataSurface->Surface(2).Vertex(3).z = 4.9784;
    state->dataSurface->Surface(2).Vertex(4).x = -4.80163552;
    state->dataSurface->Surface(2).Vertex(4).y = -1.66421151;
    state->dataSurface->Surface(2).Vertex(4).z = 4.9784;

    // Surface 3 Door ceiling
    state->dataSurface->Surface(3).Name = "Class1_Ceiling_0_0_9_0_9_0_Subsurface";
    state->dataSurface->Surface(3).HeatTransSurf = true;
    state->dataSurface->Surface(3).Zone = 1;
    state->dataSurface->Surface(3).Class = SurfaceClass::Door;
    state->dataSurface->Surface(3).Construction = 1;
    state->dataSurface->Surface(3).BaseSurf = 1;
    state->dataSurface->Surface(3).Sides = 4;
    state->dataSurface->Surface(3).Area = 10;
    state->dataSurface->Surface(3).Azimuth = 0;
    state->dataSurface->Surface(3).Tilt = 0;
    state->dataSurface->Surface(3).Vertex.allocate(state->dataSurface->Surface(1).Sides);
    state->dataSurface->Surface(3).Vertex(1).x = -12.19542308;
    state->dataSurface->Surface(3).Vertex(1).y = -9.84254602;
    state->dataSurface->Surface(3).Vertex(1).z = 8.5343999852;
    state->dataSurface->Surface(3).Vertex(2).x = -3.83980708;
    state->dataSurface->Surface(3).Vertex(2).y = -9.84254602;
    state->dataSurface->Surface(3).Vertex(2).z = 8.5343999852;
    state->dataSurface->Surface(3).Vertex(3).x = -3.83980708;
    state->dataSurface->Surface(3).Vertex(3).y = -1.48693002;
    state->dataSurface->Surface(3).Vertex(3).z = 8.5343999852;
    state->dataSurface->Surface(3).Vertex(4).x = -12.19542308;
    state->dataSurface->Surface(3).Vertex(4).y = -1.48693002;
    state->dataSurface->Surface(3).Vertex(4).z = 8.5343999852;

    state->dataSurface->surfIntConv.allocate(TotSurfaces);
    std::fill(state->dataSurface->surfIntConv.begin(), state->dataSurface->surfIntConv.end(), SurfIntConv());
    state->dataSurface->SurfTAirRef.dimension(TotSurfaces, 0);
    state->dataSurface->SurfTAirRefRpt.dimension(TotSurfaces, 0);

    state->dataRoomAir->AirModel.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->AirModel(1).AirModel = RoomAir::RoomAirModel::DispVent3Node;

    state->dataRoomAir->APos_Wall.allocate(TotSurfaces);
    state->dataRoomAir->APos_Floor.allocate(TotSurfaces);
    state->dataRoomAir->APos_Ceiling.allocate(TotSurfaces);
    state->dataRoomAir->PosZ_Wall.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->PosZ_Floor.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->PosZ_Ceiling.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->APos_Window.allocate(TotSurfaces);
    state->dataRoomAir->APos_Door.allocate(TotSurfaces);
    state->dataRoomAir->APos_Internal.allocate(TotSurfaces);
    state->dataRoomAir->PosZ_Window.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->PosZ_Door.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->PosZ_Internal.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->HCeiling.allocate(TotSurfaces);
    state->dataRoomAir->HWall.allocate(TotSurfaces);
    state->dataRoomAir->HFloor.allocate(TotSurfaces);
    state->dataRoomAir->HInternal.allocate(TotSurfaces);
    state->dataRoomAir->HWindow.allocate(TotSurfaces);
    state->dataRoomAir->HDoor.allocate(TotSurfaces);

    state->dataRoomAir->ZoneCeilingHeight1.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->ZoneCeilingHeight2.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->ZoneCeilingHeight1(1) = 4.9784;
    state->dataRoomAir->ZoneCeilingHeight2(1) = 4.9784;

    // Arrays initializations
    state->dataRoomAir->APos_Wall = 0;
    state->dataRoomAir->APos_Floor = 0;
    state->dataRoomAir->APos_Ceiling = 0;
    state->dataRoomAir->PosZ_Wall(1).beg = 1;
    state->dataRoomAir->PosZ_Wall(1).end = 0;
    state->dataRoomAir->PosZ_Floor(1).beg = 1;
    state->dataRoomAir->PosZ_Floor(1).end = 0;
    state->dataRoomAir->PosZ_Ceiling(1).beg = 1;
    state->dataRoomAir->PosZ_Ceiling(1).end = 0;
    state->dataRoomAir->APos_Window = 0;
    state->dataRoomAir->APos_Door = 0;
    state->dataRoomAir->APos_Internal = 0;
    state->dataRoomAir->PosZ_Window(1).beg = 1;
    state->dataRoomAir->PosZ_Window(1).end = 0;
    state->dataRoomAir->PosZ_Door(1).beg = 1;
    state->dataRoomAir->PosZ_Door(1).end = 3;
    state->dataRoomAir->PosZ_Internal(1).beg = 1;
    state->dataRoomAir->PosZ_Internal(1).end = 0;
    state->dataRoomAir->HCeiling = 0.0;
    state->dataRoomAir->HWall = 0.0;
    state->dataRoomAir->HFloor = 0.0;
    state->dataRoomAir->HInternal = 0.0;
    state->dataRoomAir->HWindow = 0.0;
    state->dataRoomAir->HDoor = 0.0;

    state->dataRoomAir->ZoneCrossVent.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->ZoneCrossVent(1).ZonePtr = 1;
    state->dataRoomAir->PosZ_Door(1).beg = 1;
    state->dataRoomAir->PosZ_Door(1).end = 3;
    state->dataRoomAir->APos_Door(1) = 1;
    state->dataRoomAir->APos_Door(2) = 2;
    state->dataRoomAir->APos_Door(3) = 3;

    state->dataRoomAir->ZTMX(1) = 20.0;
    state->dataRoomAir->ZTOC(1) = 21.0;
    state->dataHeatBalSurf->SurfTempIn(1) = 23.0;
    state->dataHeatBalSurf->SurfTempIn(2) = 23.0;
    state->dataHeatBalSurf->SurfTempIn(3) = 23.0;

    HcDispVent3Node(*state, 1, 0.5);

    EXPECT_NEAR(1.889346, state->dataRoomAir->DispVent3NodeHcIn(1), 0.0001);
    EXPECT_NEAR(1.650496, state->dataRoomAir->DispVent3NodeHcIn(2), 0.0001);
    EXPECT_NEAR(1.889346, state->dataRoomAir->DispVent3NodeHcIn(3), 0.0001);
    EXPECT_NEAR(379.614212, state->dataDispVentMgr->HAT_OC, 0.0001);
    EXPECT_NEAR(16.504965, state->dataDispVentMgr->HA_OC, 0.0001);
    EXPECT_NEAR(869.099591, state->dataDispVentMgr->HAT_MX, 0.0001);
    EXPECT_NEAR(37.786938, state->dataDispVentMgr->HA_MX, 0.0001);

    state->dataRoomAir->IsZoneDispVent3Node.deallocate();
    state->dataSurface->Surface.deallocate();
    state->dataHeatBal->SurfTempEffBulkAir.deallocate();
    state->dataHeatBalSurf->SurfTempIn.deallocate();
    state->dataRoomAir->DispVent3NodeHcIn.deallocate();
    state->dataRoomAir->ZTMX.deallocate();
    state->dataRoomAir->ZTOC.deallocate();
    state->dataRoomAir->AirModel.deallocate();

    state->dataRoomAir->APos_Wall.deallocate();
    state->dataRoomAir->APos_Floor.deallocate();
    state->dataRoomAir->APos_Ceiling.deallocate();
    state->dataRoomAir->PosZ_Wall.deallocate();
    state->dataRoomAir->PosZ_Floor.deallocate();
    state->dataRoomAir->PosZ_Ceiling.deallocate();
    state->dataRoomAir->APos_Window.deallocate();
    state->dataRoomAir->APos_Door.deallocate();
    state->dataRoomAir->APos_Internal.deallocate();
    state->dataRoomAir->PosZ_Window.deallocate();
    state->dataRoomAir->PosZ_Door.deallocate();
    state->dataRoomAir->PosZ_Internal.deallocate();
    state->dataRoomAir->HCeiling.deallocate();
    state->dataRoomAir->HWall.deallocate();
    state->dataRoomAir->HFloor.deallocate();
    state->dataRoomAir->HInternal.deallocate();
    state->dataRoomAir->HWindow.deallocate();
    state->dataRoomAir->HDoor.deallocate();
    state->dataRoomAir->ZoneCeilingHeight1.deallocate();
    state->dataRoomAir->ZoneCeilingHeight2.deallocate();
    state->dataRoomAir->ZoneCrossVent.deallocate();
}

TEST_F(EnergyPlusFixture, DVThirdOrderFloorTempCalculation)
{
    Real64 constexpr tempHistoryTerm = 0; // no history
    Real64 constexpr HAT_floor = 20;
    Real64 constexpr HA_floor = 1;
    Real64 constexpr MCpT_Total = 40;
    Real64 constexpr MCp_Total = 2;
    Real64 constexpr occupiedTemp = 25;
    Real64 constexpr nonAirSystemResponse = 0;
    Real64 constexpr zoneMultiplier = 1;
    Real64 constexpr airCap = 100;

    Real64 temp = calculateThirdOrderFloorTemperature(
        tempHistoryTerm, HAT_floor, HA_floor, MCpT_Total, MCp_Total, occupiedTemp, nonAirSystemResponse, zoneMultiplier, airCap);
    EXPECT_NEAR(0.4799, temp, 0.0001);
}
