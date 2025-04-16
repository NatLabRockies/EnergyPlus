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

// EnergyPlus::RoomAirModelUserTempPattern Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <AirflowNetwork/Elements.hpp>
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/CrossVentMgr.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/HeatBalanceAirManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

using namespace EnergyPlus;
using namespace RoomAir;

TEST_F(EnergyPlusFixture, CrossVentMgr_EvolveParaUCSDCV_Test)
{

    // set up all conditions entering the EvolveParaUCSDCV when using the Issue #5520 test file on hitcount 9925 (where it used to crash)
    state->dataGlobal->NumOfZones = 2;
    int MaxSurf = 2;

    state->dataRoomAir->RecInflowRatio.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(state->dataGlobal->NumOfZones);

    state->dataRoomAir->AFNSurfaceCrossVent.allocate({0, MaxSurf}, state->dataGlobal->NumOfZones);
    state->dataRoomAir->AFNSurfaceCrossVent(1, 1) = 1;
    state->dataRoomAir->AFNSurfaceCrossVent(0, 1) = 1;
    state->dataRoomAir->AFNSurfaceCrossVent(0, 2) = 2;

    state->afn->MultizoneSurfaceData.allocate(MaxSurf);
    state->afn->MultizoneSurfaceData(1).surface_number = 6;
    state->afn->MultizoneSurfaceData(1).OpenFactor = 1.;
    state->afn->MultizoneSurfaceData(2).surface_number = 9;
    state->afn->MultizoneSurfaceData(2).OpenFactor = 1.;

    state->dataSurface->Surface.allocate(10);
    state->dataSurface->Surface(6).Zone = 1;
    state->dataSurface->Surface(6).Azimuth = 0.0;
    state->dataSurface->Surface(6).BaseSurf = 5;
    state->dataSurface->Surface(5).Sides = 4;
    state->dataSurface->Surface(5).Centroid.x = 13.143481000000001;
    state->dataSurface->Surface(5).Centroid.y = 13.264719000000003;
    state->dataSurface->Surface(5).Centroid.z = 1.6002000000000001;
    state->dataSurface->Surface(7).Sides = 4;
    state->dataSurface->Surface(7).Centroid.x = 25.415490999999996;
    state->dataSurface->Surface(7).Centroid.y = 7.1687189999999994;
    state->dataSurface->Surface(7).Centroid.z = 1.6763999999999999;
    state->dataSurface->Surface(8).Sides = 4;
    state->dataSurface->Surface(8).Centroid.x = 13.223490999999997;
    state->dataSurface->Surface(8).Centroid.y = 1.0727189999999998;
    state->dataSurface->Surface(8).Centroid.z = 1.6763999999999999;
    state->dataSurface->Surface(10).Sides = 4;
    state->dataSurface->Surface(10).Centroid.x = 1.0314909999999999;
    state->dataSurface->Surface(10).Centroid.y = 7.1687189999999994;
    state->dataSurface->Surface(10).Centroid.z = 1.6763999999999999;
    state->dataSurface->SurfOutDryBulbTemp.allocate(10);
    state->dataSurface->SurfOutWindSpeed.allocate(10);
    state->dataSurface->SurfOutDryBulbTemp = 0.0;
    state->dataSurface->SurfOutWindSpeed = 0.0;

    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Volume = 996.75300003839993;
    state->dataHeatBal->Zone(1).FloorArea = 297.28972800000003;

    state->afn->AirflowNetworkLinkSimu.allocate(1);
    state->afn->AirflowNetworkLinkSimu(1).VolFLOW2 = 27.142934345451458;

    state->dataEnvrn->WindDir = 271.66666666666669;

    state->dataRoomAir->AirModel.allocate(state->dataGlobal->NumOfZones);

    state->afn->AirflowNetworkLinkageData.allocate(2);
    state->afn->AirflowNetworkLinkageData(1).CompNum = 1;
    state->afn->AirflowNetworkLinkageData(2).CompNum = 1;

    state->afn->AirflowNetworkCompData.allocate(3);
    state->afn->AirflowNetworkCompData(1).TypeNum = 1;
    state->afn->AirflowNetworkCompData(1).CompTypeNum = AirflowNetwork::AirflowElementType::DOP;
    state->afn->AirflowNetworkCompData(2).TypeNum = 1;
    state->afn->AirflowNetworkCompData(2).CompTypeNum = AirflowNetwork::AirflowElementType::SCR;
    state->afn->AirflowNetworkCompData(3).TypeNum = 2;
    state->afn->AirflowNetworkCompData(3).CompTypeNum = AirflowNetwork::AirflowElementType::SOP;

    state->dataRoomAir->SurfParametersCrossDispVent.allocate(2);
    state->dataRoomAir->SurfParametersCrossDispVent(1).Width = 22.715219999999999;
    state->dataRoomAir->SurfParametersCrossDispVent(1).Height = 1.3715999999999999;
    state->dataRoomAir->SurfParametersCrossDispVent(2).Width = 22.869143999999999;
    state->dataRoomAir->SurfParametersCrossDispVent(2).Height = 1.3715999999999999;

    state->dataRoomAir->CrossVentJetRecFlows.allocate({0, MaxSurf}, 1);

    state->dataRoomAir->PosZ_Wall.allocate(1);
    state->dataRoomAir->PosZ_Wall(1).beg = 1;
    state->dataRoomAir->PosZ_Wall(1).end = 4;

    state->dataRoomAir->APos_Wall.allocate(12);
    state->dataRoomAir->APos_Wall(1) = 5;
    state->dataRoomAir->APos_Wall(2) = 7;
    state->dataRoomAir->APos_Wall(3) = 8;
    state->dataRoomAir->APos_Wall(4) = 10;

    state->dataRoomAir->Droom.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->Droom(1) = 13.631070390838719;
    state->dataRoomAir->Dstar.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->Ain.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->ZoneCrossVent.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->ZoneCrossVent(1).ZonePtr = 1;
    state->dataRoomAir->JetRecAreaRatio.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->Ujet.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->Urec.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->Qrec.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->Qtot.allocate(state->dataGlobal->NumOfZones);
    state->dataRoomAir->Tin.allocate(state->dataGlobal->NumOfZones);

    EvolveParaCrossVent(*state, 1);

    EXPECT_NEAR(27.14, state->dataRoomAir->CrossVentJetRecFlows(1, 1).Fin, 0.01);
    EXPECT_NEAR(0.871, state->dataRoomAir->CrossVentJetRecFlows(1, 1).Uin, 0.001);
    EXPECT_NEAR(0.000, state->dataRoomAir->CrossVentJetRecFlows(1, 1).Vjet, 0.001);
    EXPECT_NEAR(0.243, state->dataRoomAir->CrossVentJetRecFlows(1, 1).Yjet, 0.001);
    EXPECT_NEAR(0.279, state->dataRoomAir->CrossVentJetRecFlows(1, 1).Ujet, 0.001);
    EXPECT_NEAR(0.070, state->dataRoomAir->CrossVentJetRecFlows(1, 1).Yrec, 0.001);
    EXPECT_NEAR(0.080, state->dataRoomAir->CrossVentJetRecFlows(1, 1).Urec, 0.001);
    EXPECT_NEAR(0.466, state->dataRoomAir->CrossVentJetRecFlows(1, 1).YQrec, 0.001);
    EXPECT_NEAR(0.535, state->dataRoomAir->CrossVentJetRecFlows(1, 1).Qrec, 0.001);
}
