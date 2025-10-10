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

// C++ Headers
#include <cassert>
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/CrossVentMgr.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace RoomAir {

    // MODULE INFORMATION:
    //       AUTHOR         G. Carrilho da Graca
    //       DATE WRITTEN   October 2004

    // PURPOSE OF THIS MODULE:
    // Routines that implement the UCSD Cross Ventilation

    using namespace DataEnvironment;
    using namespace DataHeatBalance;
    using namespace DataHeatBalSurface;
    using namespace DataSurfaces;
    using Convect::CalcDetailedHcInForDVModel;

    Real64 constexpr Cjet1(1.873);     // First correlation constant for the jet velocity
    Real64 constexpr Cjet2(0.243);     // Second correlation constant for the jet velocity
    Real64 constexpr Crec1(0.591);     // First correlation constant for the recirculation velocity
    Real64 constexpr Crec2(0.070);     // Second correlation constant for the recirculation velocity
    Real64 constexpr CjetTemp(0.849);  // Correlation constant for the jet temperature rise
    Real64 constexpr CrecTemp(1.385);  // Correlation constant for the recirculation temperature rise
    Real64 constexpr CrecFlow1(0.415); // First correlation constant for the recirculation flow rate
    Real64 constexpr CrecFlow2(0.466); // Second correlation constant for the recirculation flow rate

    void ManageCrossVent(EnergyPlusData &state,
                         int const ZoneNum) // index number for the specified zone
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         G. Carrilho da Graca
        //       DATE WRITTEN   October 2004

        // PURPOSE OF THIS SUBROUTINE:
        //   manage the UCSD Cross Ventilation model

        InitCrossVent(state, ZoneNum);

        // perform Cross Ventilation model calculations
        CalcCrossVent(state, ZoneNum);
    }

    void InitCrossVent(EnergyPlusData &state, int const ZoneNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         G. Carrilho da Graca
        //       DATE WRITTEN   October 2004

        // PURPOSE OF THIS SUBROUTINE:
        // Low Energy Cooling by Ventilation initialization subroutine.
        // All the data preparation needed to run the LECV models.
        // The subroutines sets up arrays with the locations in the main EnergyPlus surface array of
        // ceiling, windows, doors and walls. The zone maximum and minimum height is calculated.

        // Do the one time initializations
        if (state.dataCrossVentMgr->InitUCSDCV_MyOneTimeFlag) {
            state.dataCrossVentMgr->InitUCSDCV_MyEnvrnFlag.dimension(state.dataGlobal->NumOfZones, true);
            state.dataCrossVentMgr->InitUCSDCV_MyOneTimeFlag = false;
        }

        // Do the begin environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataCrossVentMgr->InitUCSDCV_MyEnvrnFlag(ZoneNum)) {
            state.dataCrossVentMgr->InitUCSDCV_MyEnvrnFlag(ZoneNum) = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataCrossVentMgr->InitUCSDCV_MyEnvrnFlag(ZoneNum) = true;
        }
    }

    void HcCrossVent(EnergyPlusData &state, int const ZoneNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         G. Carrilho da Graca
        //       DATE WRITTEN   October 2004
        //       MODIFIED       8/2013 - Sam Brunswick
        //                      To improve convection coefficient calculation

        // PURPOSE OF THIS SUBROUTINE:
        // Main subroutine for convection calculation in the UCSD Cross Ventilation model.
        // It calls CalcDetailedHcInForDVModel for convection coefficient
        // initial calculations and averages the final result comparing the position of the surface with
        // the interface subzone height.

        // Initialize HAT and HA
        state.dataCrossVentMgr->HAT_J = 0.0;
        state.dataCrossVentMgr->HAT_R = 0.0;
        state.dataCrossVentMgr->HA_J = 0.0;
        state.dataCrossVentMgr->HA_R = 0.0;

        // Is the air flow model for this zone set to UCSDCV Cross Ventilation?
        if (state.dataRoomAir->IsZoneCrossVent(ZoneNum)) {

            Real64 zoneJetRecAreaRatio = state.dataRoomAir->JetRecAreaRatio(ZoneNum);

            // WALL Hc, HA and HAT calculation
            for (int Ctd = state.dataRoomAir->PosZ_Wall(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Wall(ZoneNum).end; ++Ctd) {
                int SurfNum = state.dataRoomAir->APos_Wall(Ctd);
                if (SurfNum == 0) {
                    continue;
                }

                auto const &surf = state.dataSurface->Surface(SurfNum);
                state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
                state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTREC(ZoneNum);
                CalcDetailedHcInForDVModel(
                    state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->CrossVentHcIn, state.dataRoomAir->Urec);
                state.dataRoomAir->HWall(Ctd) = state.dataRoomAir->CrossVentHcIn(SurfNum);
                state.dataCrossVentMgr->HAT_R += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HWall(Ctd);
                state.dataCrossVentMgr->HA_R += surf.Area * state.dataRoomAir->HWall(Ctd);
            } // END WALL
            // WINDOW Hc, HA and HAT CALCULATION
            for (int Ctd = state.dataRoomAir->PosZ_Window(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Window(ZoneNum).end; ++Ctd) {
                int SurfNum = state.dataRoomAir->APos_Window(Ctd);
                if (SurfNum == 0) {
                    continue;
                }

                auto const &surf = state.dataSurface->Surface(SurfNum);
                state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
                state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];
                if (surf.Tilt > 10.0 && surf.Tilt < 170.0) { // Window Wall
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTREC(ZoneNum);
                    CalcDetailedHcInForDVModel(
                        state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->CrossVentHcIn, state.dataRoomAir->Urec);
                    state.dataRoomAir->HWindow(Ctd) = state.dataRoomAir->CrossVentHcIn(SurfNum);
                    state.dataCrossVentMgr->HAT_R += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HWindow(Ctd);
                    state.dataCrossVentMgr->HA_R += surf.Area * state.dataRoomAir->HWindow(Ctd);
                }
                if (surf.Tilt <= 10.0) { // Window Ceiling
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTJET(ZoneNum);
                    CalcDetailedHcInForDVModel(
                        state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->CrossVentHcIn, state.dataRoomAir->Ujet);
                    Real64 Hjet = state.dataRoomAir->CrossVentHcIn(SurfNum);
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTREC(ZoneNum);
                    CalcDetailedHcInForDVModel(
                        state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->CrossVentHcIn, state.dataRoomAir->Urec);
                    Real64 Hrec = state.dataRoomAir->CrossVentHcIn(SurfNum);
                    state.dataRoomAir->HWindow(Ctd) = zoneJetRecAreaRatio * Hjet + (1 - zoneJetRecAreaRatio) * Hrec;
                    state.dataCrossVentMgr->HAT_R += surf.Area * (1.0 - zoneJetRecAreaRatio) * state.dataHeatBalSurf->SurfTempIn(SurfNum) * Hrec;
                    state.dataCrossVentMgr->HA_R += surf.Area * (1.0 - zoneJetRecAreaRatio) * Hrec;
                    state.dataCrossVentMgr->HAT_J += surf.Area * zoneJetRecAreaRatio * state.dataHeatBalSurf->SurfTempIn(SurfNum) * Hjet;
                    state.dataCrossVentMgr->HA_J += surf.Area * zoneJetRecAreaRatio * Hjet;
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) =
                        zoneJetRecAreaRatio * state.dataRoomAir->ZTJET(ZoneNum) + (1 - zoneJetRecAreaRatio) * state.dataRoomAir->ZTREC(ZoneNum);
                }
                if (surf.Tilt >= 170.0) { // Window Floor
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTJET(ZoneNum);
                    CalcDetailedHcInForDVModel(
                        state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->CrossVentHcIn, state.dataRoomAir->Ujet);
                    Real64 Hjet = state.dataRoomAir->CrossVentHcIn(SurfNum);
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTREC(ZoneNum);
                    CalcDetailedHcInForDVModel(
                        state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->CrossVentHcIn, state.dataRoomAir->Urec);
                    Real64 Hrec = state.dataRoomAir->CrossVentHcIn(SurfNum);
                    state.dataRoomAir->HWindow(Ctd) = zoneJetRecAreaRatio * Hjet + (1 - zoneJetRecAreaRatio) * Hrec;
                    state.dataCrossVentMgr->HAT_R += surf.Area * (1.0 - zoneJetRecAreaRatio) * state.dataHeatBalSurf->SurfTempIn(SurfNum) * Hrec;
                    state.dataCrossVentMgr->HA_R += surf.Area * (1.0 - zoneJetRecAreaRatio) * Hrec;
                    state.dataCrossVentMgr->HAT_J += surf.Area * zoneJetRecAreaRatio * state.dataHeatBalSurf->SurfTempIn(SurfNum) * Hjet;
                    state.dataCrossVentMgr->HA_J += surf.Area * zoneJetRecAreaRatio * Hjet;
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) =
                        zoneJetRecAreaRatio * state.dataRoomAir->ZTJET(ZoneNum) + (1 - zoneJetRecAreaRatio) * state.dataRoomAir->ZTREC(ZoneNum);
                }
                state.dataRoomAir->CrossVentHcIn(SurfNum) = state.dataRoomAir->HWindow(Ctd);
            } // END WINDOW
            // DOOR Hc, HA and HAT CALCULATION
            for (int Ctd = state.dataRoomAir->PosZ_Door(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Door(ZoneNum).end; ++Ctd) { // DOOR
                int SurfNum = state.dataRoomAir->APos_Door(Ctd);
                if (SurfNum == 0) {
                    continue;
                }

                auto const &surf = state.dataSurface->Surface(SurfNum);
                state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
                state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTREC(ZoneNum);
                CalcDetailedHcInForDVModel(
                    state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->CrossVentHcIn, state.dataRoomAir->Urec);
                state.dataRoomAir->HDoor(Ctd) = state.dataRoomAir->CrossVentHcIn(SurfNum);
                state.dataCrossVentMgr->HAT_R += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HDoor(Ctd);
                state.dataCrossVentMgr->HA_R += surf.Area * state.dataRoomAir->HDoor(Ctd);
            } // END DOOR

            // INTERNAL Hc, HA and HAT CALCULATION
            for (int Ctd = state.dataRoomAir->PosZ_Internal(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Internal(ZoneNum).end; ++Ctd) {
                int SurfNum = state.dataRoomAir->APos_Internal(Ctd);
                if (SurfNum == 0) {
                    continue;
                }

                auto const &surf = state.dataSurface->Surface(SurfNum);
                state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
                state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTREC(ZoneNum);
                CalcDetailedHcInForDVModel(
                    state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->CrossVentHcIn, state.dataRoomAir->Urec);
                state.dataRoomAir->HInternal(Ctd) = state.dataRoomAir->CrossVentHcIn(SurfNum);
                state.dataCrossVentMgr->HAT_R += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HInternal(Ctd);
                state.dataCrossVentMgr->HA_R += surf.Area * state.dataRoomAir->HInternal(Ctd);
            } // END INTERNAL

            // CEILING Hc, HA and HAT CALCULATION
            for (int Ctd = state.dataRoomAir->PosZ_Ceiling(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Ceiling(ZoneNum).end; ++Ctd) {
                int SurfNum = state.dataRoomAir->APos_Ceiling(Ctd);
                if (SurfNum == 0) {
                    continue;
                }

                auto const &surf = state.dataSurface->Surface(SurfNum);
                state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
                state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTJET(ZoneNum);
                CalcDetailedHcInForDVModel(
                    state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->CrossVentHcIn, state.dataRoomAir->Ujet);
                Real64 Hjet = state.dataRoomAir->CrossVentHcIn(SurfNum);
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTREC(ZoneNum);
                CalcDetailedHcInForDVModel(
                    state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->CrossVentHcIn, state.dataRoomAir->Urec);
                Real64 Hrec = state.dataRoomAir->CrossVentHcIn(SurfNum);

                state.dataRoomAir->HCeiling(Ctd) = zoneJetRecAreaRatio * Hjet + (1 - zoneJetRecAreaRatio) * Hrec;
                state.dataCrossVentMgr->HAT_R += surf.Area * (1 - zoneJetRecAreaRatio) * state.dataHeatBalSurf->SurfTempIn(SurfNum) * Hrec;
                state.dataCrossVentMgr->HA_R += surf.Area * (1 - zoneJetRecAreaRatio) * Hrec;
                state.dataCrossVentMgr->HAT_J += surf.Area * zoneJetRecAreaRatio * state.dataHeatBalSurf->SurfTempIn(SurfNum) * Hjet;
                state.dataCrossVentMgr->HA_J += surf.Area * zoneJetRecAreaRatio * Hjet;
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) =
                    zoneJetRecAreaRatio * state.dataRoomAir->ZTJET(ZoneNum) + (1 - zoneJetRecAreaRatio) * state.dataRoomAir->ZTREC(ZoneNum);
                state.dataRoomAir->CrossVentHcIn(SurfNum) = state.dataRoomAir->HCeiling(Ctd);
            } // END CEILING
            // FLOOR Hc, HA and HAT CALCULATION
            for (int Ctd = state.dataRoomAir->PosZ_Floor(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Floor(ZoneNum).end; ++Ctd) {
                int SurfNum = state.dataRoomAir->APos_Floor(Ctd);
                if (SurfNum == 0) {
                    continue;
                }

                auto const &surf = state.dataSurface->Surface(SurfNum);
                state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
                state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTJET(ZoneNum);
                CalcDetailedHcInForDVModel(
                    state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->CrossVentHcIn, state.dataRoomAir->Ujet);
                Real64 Hjet = state.dataRoomAir->CrossVentHcIn(SurfNum);
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTREC(ZoneNum);
                CalcDetailedHcInForDVModel(
                    state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->CrossVentHcIn, state.dataRoomAir->Urec);
                Real64 Hrec = state.dataRoomAir->CrossVentHcIn(SurfNum);
                state.dataRoomAir->HFloor(Ctd) = zoneJetRecAreaRatio * Hjet + (1 - zoneJetRecAreaRatio) * Hrec;
                state.dataCrossVentMgr->HAT_R += surf.Area * (1 - zoneJetRecAreaRatio) * state.dataHeatBalSurf->SurfTempIn(SurfNum) * Hrec;
                state.dataCrossVentMgr->HA_R += surf.Area * (1 - zoneJetRecAreaRatio) * Hrec;
                state.dataCrossVentMgr->HAT_J += surf.Area * zoneJetRecAreaRatio * state.dataHeatBalSurf->SurfTempIn(SurfNum) * Hjet;
                state.dataCrossVentMgr->HA_J += surf.Area * zoneJetRecAreaRatio * Hjet;
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) =
                    zoneJetRecAreaRatio * state.dataRoomAir->ZTJET(ZoneNum) + (1 - zoneJetRecAreaRatio) * state.dataRoomAir->ZTREC(ZoneNum);
                state.dataRoomAir->CrossVentHcIn(SurfNum) = state.dataRoomAir->HFloor(Ctd);
            } // END FLOOR
        }
    }

    void EvolveParaCrossVent(EnergyPlusData &state, int const ZoneNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         G. Carrilho da Graca
        //       DATE WRITTEN   October 2004
        //       MODIFIED       8/2013 - Sam Brunswick
        //                      To incorporate an improved model
        //                      and add modeling of multiple jets

        // PURPOSE OF THIS SUBROUTINE:
        // Subroutine for parameter actualization in the UCSD Cross Ventilation model.

        Real64 constexpr MinUin(0.2);

        Real64 Uin;            // Inflow air velocity [m/s]
        Real64 CosPhi;         // Angle (in degrees) between the wind and the outward normal of the dominant surface
        Real64 SurfNorm;       // Outward normal of surface
        Real64 SumToZone(0.0); // Sum of velocities through
        Real64 MaxFlux(0.0);
        int MaxSurf(0);
        Real64 ActiveSurfNum;
        int NSides;   // Number of sides in surface
        Real64 Wroom; // Room width
        Real64 Aroom; // Room area cross section

        assert(state.dataRoomAir->AirModel.allocated());
        state.dataRoomAir->RecInflowRatio(ZoneNum) = 0.0;
        auto const &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);

        // Identify the dominant aperture:
        MaxSurf = state.dataRoomAir->AFNSurfaceCrossVent(1, ZoneNum);
        int const surfNum = state.afn->MultizoneSurfaceData(MaxSurf).SurfNum;
        auto const &thisSurface = state.dataSurface->Surface(surfNum);

        int afnSurfNum1 = state.dataRoomAir->AFNSurfaceCrossVent(1, ZoneNum);

        if (thisSurface.Zone == ZoneNum) {
            // this is a direct airflow network aperture
            SumToZone = state.afn->AirflowNetworkLinkSimu(afnSurfNum1).VolFLOW2;
            MaxFlux = state.afn->AirflowNetworkLinkSimu(afnSurfNum1).VolFLOW2;
        } else {
            // this is an indirect airflow network aperture
            SumToZone = state.afn->AirflowNetworkLinkSimu(afnSurfNum1).VolFLOW;
            MaxFlux = state.afn->AirflowNetworkLinkSimu(afnSurfNum1).VolFLOW;
        }

        for (int Ctd2 = 2; Ctd2 <= state.dataRoomAir->AFNSurfaceCrossVent(0, ZoneNum); ++Ctd2) {
            int afnSurfNum = state.dataRoomAir->AFNSurfaceCrossVent(Ctd2, ZoneNum);
            if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(afnSurfNum).SurfNum).Zone == ZoneNum) {
                if (state.afn->AirflowNetworkLinkSimu(afnSurfNum).VolFLOW2 > MaxFlux) {
                    MaxFlux = state.afn->AirflowNetworkLinkSimu(afnSurfNum).VolFLOW2;
                    MaxSurf = afnSurfNum;
                }
                SumToZone += state.afn->AirflowNetworkLinkSimu(afnSurfNum).VolFLOW2;
            } else {
                if (state.afn->AirflowNetworkLinkSimu(afnSurfNum).VolFLOW > MaxFlux) {
                    MaxFlux = state.afn->AirflowNetworkLinkSimu(afnSurfNum).VolFLOW;
                    MaxSurf = afnSurfNum;
                }
                SumToZone += state.afn->AirflowNetworkLinkSimu(afnSurfNum).VolFLOW;
            }
        }

        // Check if wind direction is within +/- 90 degrees of the outward normal of the dominant surface
        SurfNorm = thisSurface.Azimuth;
        CosPhi = std::cos((state.dataEnvrn->WindDir - SurfNorm) * Constant::DegToRad);
        if (CosPhi <= 0) {
            state.dataRoomAir->AirModel(ZoneNum).SimAirModel = false;
            auto flows(state.dataRoomAir->CrossVentJetRecFlows(_, ZoneNum)); // This is an array slice, need to get rid of this (THIS_AUTO_OK)
            for (int i = 1, u = flows.u(); i <= u; ++i) {
                auto &e(flows(i));
                e.Ujet = e.Urec = 0.0;
            }
            state.dataRoomAir->Urec(ZoneNum) = 0.0;
            state.dataRoomAir->Ujet(ZoneNum) = 0.0;
            state.dataRoomAir->Qrec(ZoneNum) = 0.0;
            if (thisSurface.ExtBoundCond > 0) {
                state.dataRoomAir->Tin(ZoneNum) =
                    state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataSurface->Surface(thisSurface.ExtBoundCond).Zone).MAT;
            } else if (thisSurface.ExtBoundCond == ExternalEnvironment) {
                state.dataRoomAir->Tin(ZoneNum) = state.dataSurface->SurfOutDryBulbTemp(surfNum);
            } else if (thisSurface.ExtBoundCond == Ground) {
                state.dataRoomAir->Tin(ZoneNum) = state.dataSurface->SurfOutDryBulbTemp(surfNum);
            } else if (thisSurface.ExtBoundCond == OtherSideCoefNoCalcExt || thisSurface.ExtBoundCond == OtherSideCoefCalcExt) {
                auto &thisOSC = state.dataSurface->OSC(thisSurface.OSCPtr);
                thisOSC.OSCTempCalc =
                    (thisOSC.ZoneAirTempCoef * thisZoneHB.MAT + thisOSC.ExtDryBulbCoef * state.dataSurface->SurfOutDryBulbTemp(surfNum) +
                     thisOSC.ConstTempCoef * thisOSC.ConstTemp +
                     thisOSC.GroundTempCoef * state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface] +
                     thisOSC.WindSpeedCoef * state.dataSurface->SurfOutWindSpeed(surfNum) * state.dataSurface->SurfOutDryBulbTemp(surfNum));
                state.dataRoomAir->Tin(ZoneNum) = thisOSC.OSCTempCalc;
            } else {
                state.dataRoomAir->Tin(ZoneNum) = state.dataSurface->SurfOutDryBulbTemp(surfNum);
            }
            return;
        }

        // Calculate the opening area for all apertures
        for (int Ctd = 1; Ctd <= state.dataRoomAir->AFNSurfaceCrossVent(0, ZoneNum); ++Ctd) {
            auto &jetRecFlows = state.dataRoomAir->CrossVentJetRecFlows(Ctd, ZoneNum);
            auto const &surfParams = state.dataRoomAir->SurfParametersCrossDispVent(Ctd);
            int cCompNum = state.afn->AirflowNetworkLinkageData(Ctd).CompNum;
            if (state.afn->AirflowNetworkCompData(cCompNum).CompTypeNum == AirflowNetwork::iComponentTypeNum::DOP) {
                jetRecFlows.Area = surfParams.Width * surfParams.Height * state.afn->MultizoneSurfaceData(Ctd).OpenFactor;
            } else if (state.afn->AirflowNetworkCompData(cCompNum).CompTypeNum == AirflowNetwork::iComponentTypeNum::SCR) {
                jetRecFlows.Area = surfParams.Width * surfParams.Height;
            } else {
                ShowSevereError(
                    state, "RoomAirModelCrossVent:EvolveParaUCSDCV: Illegal leakage component referenced in the cross ventilation room air model");
                ShowContinueError(state,
                                  format("Surface {} in zone {} uses leakage component {}",
                                         state.afn->AirflowNetworkLinkageData(Ctd).Name,
                                         state.dataHeatBal->Zone(ZoneNum).Name,
                                         state.afn->AirflowNetworkLinkageData(Ctd).CompName));
                ShowContinueError(state, "Only leakage component types AirflowNetwork:MultiZone:Component:DetailedOpening and ");
                ShowContinueError(state, "AirflowNetwork:MultiZone:Surface:Crack can be used with the cross ventilation room air model");
                ShowFatalError(state, "Previous severe error causes program termination");
            }
        }

        // Calculate Droom, Wroom, Dstar
        // Droom the distance between the average point of the base surface of the airflow network Surface (if the base surface
        // is a Window or Door it looks for the second base surface).
        // Dstar is Droom corrected for wind angle

        Vector3<Real64> baseCentroid;

        Wroom = state.dataHeatBal->Zone(ZoneNum).Volume / state.dataHeatBal->Zone(ZoneNum).FloorArea;
        auto const &baseSurface = state.dataSurface->Surface(thisSurface.BaseSurf);
        if ((baseSurface.Sides == 3) || (baseSurface.Sides == 4)) {
            baseCentroid = baseSurface.Centroid;
        } else {
            // If the surface has more than 4 vertex then average the vertex coordinates in X, Y and Z.
            NSides = baseSurface.Sides;
            assert(NSides > 0);
            baseCentroid = {0.0, 0.0, 0.0};
            for (int i = 1; i <= NSides; ++i) {
                baseCentroid += baseSurface.Vertex(i);
            }
            baseCentroid /= double(NSides);
        }

        Vector3<Real64> wallCentroid;
        Real64 const Wroom_2(pow_2(Wroom));
        for (int Ctd = state.dataRoomAir->PosZ_Wall(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Wall(ZoneNum).end; ++Ctd) {
            if ((state.dataSurface->Surface(state.dataRoomAir->APos_Wall(Ctd)).Sides == 3) ||
                (state.dataSurface->Surface(state.dataRoomAir->APos_Wall(Ctd)).Sides == 4)) {
                wallCentroid = state.dataSurface->Surface(state.dataRoomAir->APos_Wall(Ctd)).Centroid;
            } else {
                NSides = state.dataSurface->Surface(state.dataRoomAir->APos_Wall(Ctd)).Sides;
                assert(NSides > 0);
                wallCentroid = {0.0, 0.0, 0.0};
                for (int i = 1; i <= NSides; ++i) {
                    wallCentroid += state.dataSurface->Surface(state.dataRoomAir->APos_Wall(Ctd)).Vertex(i);
                }
                wallCentroid /= double(NSides);
            }
            double DroomTemp =
                std::sqrt(pow_2(baseCentroid.x - wallCentroid.x) + pow_2(baseCentroid.y - wallCentroid.y) + pow_2(baseCentroid.z - wallCentroid.z));
            if (DroomTemp > state.dataRoomAir->Droom(ZoneNum)) {
                state.dataRoomAir->Droom(ZoneNum) = DroomTemp;
            }
            state.dataRoomAir->Dstar(ZoneNum) =
                min(state.dataRoomAir->Droom(ZoneNum) / CosPhi, std::sqrt(Wroom_2 + pow_2(state.dataRoomAir->Droom(ZoneNum))));
        }

        // Room area
        Aroom = state.dataHeatBal->Zone(ZoneNum).Volume / state.dataRoomAir->Droom(ZoneNum);

        // Populate an array of inflow volume fluxes (Fin) for all apertures in the zone
        // Calculate inflow velocity (%Uin) for each aperture in the zone
        for (int Ctd = 1; Ctd <= state.dataRoomAir->AFNSurfaceCrossVent(0, ZoneNum); ++Ctd) {
            auto &jetRecFlows = state.dataRoomAir->CrossVentJetRecFlows(Ctd, ZoneNum);
            if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(Ctd).SurfNum).Zone == ZoneNum) {
                // this is a direct airflow network aperture
                jetRecFlows.Fin = state.afn->AirflowNetworkLinkSimu(state.dataRoomAir->AFNSurfaceCrossVent(Ctd, ZoneNum)).VolFLOW2;
            } else {
                // this is an indirect airflow network aperture
                jetRecFlows.Fin = state.afn->AirflowNetworkLinkSimu(state.dataRoomAir->AFNSurfaceCrossVent(Ctd, ZoneNum)).VolFLOW;
            }
            if (jetRecFlows.Area != 0) {
                jetRecFlows.Uin = jetRecFlows.Fin / jetRecFlows.Area;
            } else {
                jetRecFlows.Uin = 0.0;
            }
        }

        // Verify if Uin is higher than minimum for each aperture
        // Create a flow flag for each aperture
        // Calculate the total area of all active apertures
        ActiveSurfNum = 0.0;
        state.dataRoomAir->Ain(ZoneNum) = 0.0;
        for (int Ctd = 1; Ctd <= state.dataRoomAir->AFNSurfaceCrossVent(0, ZoneNum); ++Ctd) {
            auto &jetRecFlows = state.dataRoomAir->CrossVentJetRecFlows(Ctd, ZoneNum);
            jetRecFlows.FlowFlag = (int)(jetRecFlows.Uin > MinUin);

            ActiveSurfNum += jetRecFlows.FlowFlag;
            state.dataRoomAir->Ain(ZoneNum) += jetRecFlows.Area * jetRecFlows.FlowFlag;
        }

        // Verify if any of the apertures have minimum flow
        if (ActiveSurfNum == 0) {
            state.dataRoomAir->AirModel(ZoneNum).SimAirModel = false;
            if (thisSurface.ExtBoundCond > 0) {
                state.dataRoomAir->Tin(ZoneNum) =
                    state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataSurface->Surface(thisSurface.ExtBoundCond).Zone).MAT;
            } else if (thisSurface.ExtBoundCond == ExternalEnvironment) {
                state.dataRoomAir->Tin(ZoneNum) = state.dataSurface->SurfOutDryBulbTemp(surfNum);
            } else if (thisSurface.ExtBoundCond == Ground) {
                state.dataRoomAir->Tin(ZoneNum) = state.dataSurface->SurfOutDryBulbTemp(surfNum);
            } else if (thisSurface.ExtBoundCond == OtherSideCoefNoCalcExt || thisSurface.ExtBoundCond == OtherSideCoefCalcExt) {
                auto &thisOSC = state.dataSurface->OSC(thisSurface.OSCPtr);
                thisOSC.OSCTempCalc =
                    (thisOSC.ZoneAirTempCoef * thisZoneHB.MAT + thisOSC.ExtDryBulbCoef * state.dataSurface->SurfOutDryBulbTemp(surfNum) +
                     thisOSC.ConstTempCoef * thisOSC.ConstTemp +
                     thisOSC.GroundTempCoef * state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface] +
                     thisOSC.WindSpeedCoef * state.dataSurface->SurfOutWindSpeed(surfNum) * state.dataSurface->SurfOutDryBulbTemp(surfNum));
                state.dataRoomAir->Tin(ZoneNum) = thisOSC.OSCTempCalc;
            } else {
                state.dataRoomAir->Tin(ZoneNum) = state.dataSurface->SurfOutDryBulbTemp(surfNum);
            }
            state.dataRoomAir->Urec(ZoneNum) = 0.0;
            state.dataRoomAir->Ujet(ZoneNum) = 0.0;
            state.dataRoomAir->Qrec(ZoneNum) = 0.0;
            auto flows(state.dataRoomAir->CrossVentJetRecFlows(_, ZoneNum)); // This is an array slice, need to get rid of this (THIS_AUTO_OK)
            for (int i = 1, u = flows.u(); i <= u; ++i) {
                auto &e(flows(i));
                e.Ujet = e.Urec = 0.0;
            }
            return;
        }

        // Calculate Uin, the area weighted average velocity of all the active apertures in the zone
        // Calculate Qtot, the total volumetric flow rate through all active openings in the zone
        Uin = 0.0;

        for (int Ctd = 1; Ctd <= state.dataRoomAir->AFNSurfaceCrossVent(0, ZoneNum); ++Ctd) {
            auto const &jetRecFlows = state.dataRoomAir->CrossVentJetRecFlows(Ctd, ZoneNum);
            Uin += jetRecFlows.Area * jetRecFlows.Uin * jetRecFlows.FlowFlag / state.dataRoomAir->Ain(ZoneNum);
        }

        // Verify if Uin is higher than minimum:
        if (Uin < MinUin) {
            state.dataRoomAir->AirModel(ZoneNum).SimAirModel = false;
            state.dataRoomAir->Urec(ZoneNum) = 0.0;
            state.dataRoomAir->Ujet(ZoneNum) = 0.0;
            state.dataRoomAir->Qrec(ZoneNum) = 0.0;
            state.dataRoomAir->RecInflowRatio(ZoneNum) = 0.0;
            auto flows(state.dataRoomAir->CrossVentJetRecFlows(_, ZoneNum)); // This is an array slice, need to get rid of this (THIS_AUTO_OK)
            for (int i = 1, u = flows.u(); i <= u; ++i) {
                auto &e(flows(i));
                e.Ujet = e.Urec = 0.0;
            }
            if (thisSurface.ExtBoundCond > 0) {
                state.dataRoomAir->Tin(ZoneNum) =
                    state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataSurface->Surface(thisSurface.ExtBoundCond).Zone).MAT;
            } else if (thisSurface.ExtBoundCond == ExternalEnvironment) {
                state.dataRoomAir->Tin(ZoneNum) = state.dataSurface->SurfOutDryBulbTemp(surfNum);
            } else if (thisSurface.ExtBoundCond == Ground) {
                state.dataRoomAir->Tin(ZoneNum) = state.dataSurface->SurfOutDryBulbTemp(surfNum);
            } else if (thisSurface.ExtBoundCond == OtherSideCoefNoCalcExt || thisSurface.ExtBoundCond == OtherSideCoefCalcExt) {
                auto &thisOSC = state.dataSurface->OSC(thisSurface.OSCPtr);
                thisOSC.OSCTempCalc =
                    (thisOSC.ZoneAirTempCoef * thisZoneHB.MAT + thisOSC.ExtDryBulbCoef * state.dataSurface->SurfOutDryBulbTemp(surfNum) +
                     thisOSC.ConstTempCoef * thisOSC.ConstTemp +
                     thisOSC.GroundTempCoef * state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface] +
                     thisOSC.WindSpeedCoef * state.dataSurface->SurfOutWindSpeed(surfNum) * state.dataSurface->SurfOutDryBulbTemp(surfNum));
                state.dataRoomAir->Tin(ZoneNum) = thisOSC.OSCTempCalc;

            } else {
                state.dataRoomAir->Tin(ZoneNum) = state.dataSurface->SurfOutDryBulbTemp(surfNum);
            }
            return;
        }

        // Evaluate parameter that determines whether recirculations are present
        for (int Ctd = 1; Ctd <= state.dataRoomAir->TotCrossVent; ++Ctd) {
            if (ZoneNum == state.dataRoomAir->ZoneCrossVent(Ctd).ZonePtr) {
                if (state.dataRoomAir->Ain(ZoneNum) / Aroom > 1.0 / 2.0) {
                    state.dataRoomAir->JetRecAreaRatio(ZoneNum) = 1.0;
                } else {
                    state.dataRoomAir->JetRecAreaRatio(ZoneNum) = std::sqrt(state.dataRoomAir->Ain(ZoneNum) / Aroom);
                }
            }
        }

        state.dataRoomAir->AirModel(ZoneNum).SimAirModel = true;
        // Calculate jet and recirculation velocities for all active apertures
        state.dataRoomAir->Ujet(ZoneNum) = 0.0;
        state.dataRoomAir->Urec(ZoneNum) = 0.0;
        state.dataRoomAir->Qrec(ZoneNum) = 0.0;
        state.dataRoomAir->Qtot(ZoneNum) = 0.0;
        auto flows(state.dataRoomAir->CrossVentJetRecFlows(_, ZoneNum)); // This is an array slice, need to get rid of this (THIS_AUTO_OK)
        for (int i = 1, u = flows.u(); i <= u; ++i) {
            auto &e(flows(i));
            e.Ujet = e.Urec = e.Qrec = 0.0;
        }
        for (int Ctd = 1; Ctd <= state.dataRoomAir->AFNSurfaceCrossVent(0, ZoneNum); ++Ctd) {
            auto &jetRecFlows = state.dataRoomAir->CrossVentJetRecFlows(Ctd, ZoneNum);
            if (jetRecFlows.Uin == 0) {
                continue;
            }

            Real64 dstarexp = max(state.dataRoomAir->Dstar(ZoneNum) / (6.0 * std::sqrt(jetRecFlows.Area)), 1.0);
            jetRecFlows.Vjet = jetRecFlows.Uin * std::sqrt(jetRecFlows.Area) * 6.3 * std::log(dstarexp) / state.dataRoomAir->Dstar(ZoneNum);
            jetRecFlows.Yjet = Cjet1 * std::sqrt(jetRecFlows.Area / Aroom) * jetRecFlows.Vjet / jetRecFlows.Uin + Cjet2;
            jetRecFlows.Yrec = Crec1 * std::sqrt(jetRecFlows.Area / Aroom) * jetRecFlows.Vjet / jetRecFlows.Uin + Crec2;
            jetRecFlows.YQrec = CrecFlow1 * std::sqrt(jetRecFlows.Area * Aroom) * jetRecFlows.Vjet / jetRecFlows.Uin + CrecFlow2;
            jetRecFlows.Ujet = jetRecFlows.FlowFlag * jetRecFlows.Yjet / jetRecFlows.Uin;
            jetRecFlows.Urec = jetRecFlows.FlowFlag * jetRecFlows.Yrec / jetRecFlows.Uin;
            jetRecFlows.Qrec = jetRecFlows.FlowFlag * jetRecFlows.YQrec / jetRecFlows.Uin;
            state.dataRoomAir->Ujet(ZoneNum) += jetRecFlows.Area * jetRecFlows.Ujet / state.dataRoomAir->Ain(ZoneNum);
            state.dataRoomAir->Urec(ZoneNum) += jetRecFlows.Area * jetRecFlows.Urec / state.dataRoomAir->Ain(ZoneNum);
            state.dataRoomAir->Qrec(ZoneNum) += jetRecFlows.Qrec;
            state.dataRoomAir->Qtot(ZoneNum) += jetRecFlows.Fin * jetRecFlows.FlowFlag;
            state.dataRoomAir->Urec(ZoneNum) += jetRecFlows.Area * jetRecFlows.Urec / state.dataRoomAir->Ain(ZoneNum);
        }

        // Ratio between recirculation flow rate and total inflow rate
        if (state.dataRoomAir->Qtot(ZoneNum) != 0) {
            state.dataRoomAir->RecInflowRatio(ZoneNum) = state.dataRoomAir->Qrec(ZoneNum) / state.dataRoomAir->Qtot(ZoneNum);
        } else {
            state.dataRoomAir->RecInflowRatio(ZoneNum) = 0.0;
        }

        // Set Tin based on external conditions of the dominant aperture
        if (thisSurface.ExtBoundCond <= 0) {
            if (thisSurface.ExtBoundCond == ExternalEnvironment) {
                state.dataRoomAir->Tin(ZoneNum) = state.dataSurface->SurfOutDryBulbTemp(surfNum);
            } else if (thisSurface.ExtBoundCond == Ground) {
                state.dataRoomAir->Tin(ZoneNum) = state.dataSurface->SurfOutDryBulbTemp(surfNum);
            } else if (thisSurface.ExtBoundCond == OtherSideCoefNoCalcExt || thisSurface.ExtBoundCond == OtherSideCoefCalcExt) {
                auto &thisOSC = state.dataSurface->OSC(thisSurface.OSCPtr);
                thisOSC.OSCTempCalc =
                    (thisOSC.ZoneAirTempCoef * thisZoneHB.MAT + thisOSC.ExtDryBulbCoef * state.dataSurface->SurfOutDryBulbTemp(surfNum) +
                     thisOSC.ConstTempCoef * thisOSC.ConstTemp +
                     thisOSC.GroundTempCoef * state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface] +
                     thisOSC.WindSpeedCoef * state.dataSurface->SurfOutWindSpeed(surfNum) * state.dataSurface->SurfOutDryBulbTemp(surfNum));
                state.dataRoomAir->Tin(ZoneNum) = thisOSC.OSCTempCalc;
            } else {
                state.dataRoomAir->Tin(ZoneNum) = state.dataSurface->SurfOutDryBulbTemp(surfNum);
            }
        } else {
            // adiabatic surface
            if (surfNum == thisSurface.ExtBoundCond) {
                int NodeNum1 = state.afn->AirflowNetworkLinkageData(MaxSurf).NodeNums[0];
                int NodeNum2 = state.afn->AirflowNetworkLinkageData(MaxSurf).NodeNums[1];
                if (thisSurface.Zone == ZoneNum) {
                    if (state.afn->AirflowNetworkNodeData(NodeNum1).EPlusZoneNum <= 0) {
                        state.dataRoomAir->Tin(ZoneNum) = state.dataSurface->SurfOutDryBulbTemp(surfNum);
                    } else if (state.dataRoomAir->AirModel(state.afn->AirflowNetworkNodeData(NodeNum1).EPlusZoneNum).AirModel ==
                               RoomAir::RoomAirModel::CrossVent) {
                        state.dataRoomAir->Tin(ZoneNum) =
                            state.dataRoomAir->RoomOutflowTemp(state.afn->AirflowNetworkNodeData(NodeNum1).EPlusZoneNum);
                    } else {
                        state.dataRoomAir->Tin(ZoneNum) =
                            state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum1).EPlusZoneNum).MAT;
                    }

                } else {

                    if (state.afn->AirflowNetworkNodeData(NodeNum2).EPlusZoneNum <= 0) {
                        state.dataRoomAir->Tin(ZoneNum) = state.dataSurface->SurfOutDryBulbTemp(surfNum);
                    } else if (state.dataRoomAir->AirModel(state.afn->AirflowNetworkNodeData(NodeNum2).EPlusZoneNum).AirModel ==
                               RoomAir::RoomAirModel::CrossVent) {
                        state.dataRoomAir->Tin(ZoneNum) =
                            state.dataRoomAir->RoomOutflowTemp(state.afn->AirflowNetworkNodeData(NodeNum2).EPlusZoneNum);
                    } else {
                        state.dataRoomAir->Tin(ZoneNum) =
                            state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.afn->AirflowNetworkNodeData(NodeNum2).EPlusZoneNum).MAT;
                    }
                }
            } else if ((thisSurface.Zone == ZoneNum) &&
                       (state.dataRoomAir->AirModel(state.dataSurface->Surface(thisSurface.ExtBoundCond).Zone).AirModel ==
                        RoomAir::RoomAirModel::CrossVent)) {
                state.dataRoomAir->Tin(ZoneNum) = state.dataRoomAir->RoomOutflowTemp(state.dataSurface->Surface(thisSurface.ExtBoundCond).Zone);
            } else if ((thisSurface.Zone != ZoneNum) &&
                       (state.dataRoomAir->AirModel(thisSurface.Zone).AirModel == RoomAir::RoomAirModel::CrossVent)) {
                state.dataRoomAir->Tin(ZoneNum) = state.dataRoomAir->RoomOutflowTemp(surfNum);
            } else {
                if (thisSurface.Zone == ZoneNum) {
                    state.dataRoomAir->Tin(ZoneNum) =
                        state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataSurface->Surface(thisSurface.ExtBoundCond).Zone).MAT;
                } else {
                    state.dataRoomAir->Tin(ZoneNum) = state.dataZoneTempPredictorCorrector->zoneHeatBalance(thisSurface.Zone).MAT;
                }
            }
        }
    }

    void CalcCrossVent(EnergyPlusData &state,
                       int const ZoneNum) // Which Zonenum
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         G. Carrilho da Graca
        //       DATE WRITTEN   October 2004
        //       MODIFIED       8/2013 - Sam Brunswick
        //                      To incorporate improved temperature calculations

        // PURPOSE OF THIS SUBROUTINE:
        // Subroutine for cross ventilation modelling.

        // REFERENCES:
        // Model developed by Paul Linden (UCSD), G. Carrilho da Graca (UCSD) and P. Haves (LBL).
        // Work funded by the California Energy Comission. More information on the model can found in:
        // "Simplified Models for Heat Transfer in Rooms" G. Carrilho da Graca, Ph.D. thesis UCSD. December 2003.

        auto const &zone = state.dataHeatBal->Zone(ZoneNum);

        Real64 GainsFrac = 0.0;                                  // Fraction of lower subzone internal gains that mix as opposed to forming plumes
        Real64 ZoneMult = zone.Multiplier * zone.ListMultiplier; // total zone multiplier
        auto const &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);

        for (int Ctd = 1; Ctd <= state.dataRoomAir->TotCrossVent; ++Ctd) {
            if (ZoneNum == state.dataRoomAir->ZoneCrossVent(Ctd).ZonePtr) {
                GainsFrac = state.dataRoomAir->ZoneCrossVent(Ctd).gainsSched->getCurrentVal();
            }
        }

        // Total convective gains in the room
        Real64 ConvGains = InternalHeatGains::zoneSumAllInternalConvectionGains(state, ZoneNum);
        ConvGains += state.dataHeatBalFanSys->SumConvHTRadSys(ZoneNum) + state.dataHeatBalFanSys->SumConvPool(ZoneNum) +
                     thisZoneHB.SysDepZoneLoadsLagged + thisZoneHB.NonAirSystemResponse / ZoneMult;

        // Add heat to return air if zonal system (no return air) or cycling system (return air frequently very low or zero)
        if (zone.NoHeatToReturnAir) {
            Real64 RetAirConvGain = InternalHeatGains::zoneSumAllReturnAirConvectionGains(state, ZoneNum, 0);
            ConvGains += RetAirConvGain;
        }

        Real64 ConvGainsJet = ConvGains * GainsFrac;         // Total convective gains released in jet subzone
        Real64 ConvGainsRec = ConvGains * (1.0 - GainsFrac); // Total convective gains released in recirculation subzone
        Real64 MCp_Total = thisZoneHB.MCPI + thisZoneHB.MCPV + thisZoneHB.MCPM + thisZoneHB.MCPE + thisZoneHB.MCPC + thisZoneHB.MDotCPOA;
        Real64 MCpT_Total =
            thisZoneHB.MCPTI + thisZoneHB.MCPTV + thisZoneHB.MCPTM + thisZoneHB.MCPTE + thisZoneHB.MCPTC + thisZoneHB.MDotCPOA * zone.OutDryBulbTemp;

        if (state.afn->simulation_control.type == AirflowNetwork::ControlType::MultizoneWithoutDistribution) {
            MCp_Total = state.afn->exchangeData(ZoneNum).SumMCp + state.afn->exchangeData(ZoneNum).SumMVCp + state.afn->exchangeData(ZoneNum).SumMMCp;
            MCpT_Total =
                state.afn->exchangeData(ZoneNum).SumMCpT + state.afn->exchangeData(ZoneNum).SumMVCpT + state.afn->exchangeData(ZoneNum).SumMMCpT;
        }

        EvolveParaCrossVent(state, ZoneNum);
        // Real64 L = state.dataRoomAir->Droom(ZoneNum);

        if (state.dataRoomAir->AirModel(ZoneNum).SimAirModel) {
            //=============================== CROSS VENTILATION  Calculation ==============================================
            state.dataRoomAir->ZoneCrossVentIsMixing(ZoneNum) = 0.0;
            state.dataRoomAir->ZoneCrossVentHasREC(ZoneNum) = 1.0;
            for (int Ctd = 1; Ctd <= 4; ++Ctd) {
                HcCrossVent(state, ZoneNum);
                if (state.dataRoomAir->JetRecAreaRatio(ZoneNum) != 1.0) {
                    state.dataRoomAir->ZTREC(ZoneNum) =
                        (ConvGainsRec * CrecTemp + CrecTemp * state.dataCrossVentMgr->HAT_R + state.dataRoomAir->Tin(ZoneNum) * MCp_Total) /
                        (CrecTemp * state.dataCrossVentMgr->HA_R + MCp_Total);
                }
                state.dataRoomAir->ZTJET(ZoneNum) = (ConvGainsJet * CjetTemp + ConvGainsRec * CjetTemp + CjetTemp * state.dataCrossVentMgr->HAT_J +
                                                     CjetTemp * state.dataCrossVentMgr->HAT_R + state.dataRoomAir->Tin(ZoneNum) * MCp_Total -
                                                     CjetTemp * state.dataCrossVentMgr->HA_R * state.dataRoomAir->ZTREC(ZoneNum)) /
                                                    (CjetTemp * state.dataCrossVentMgr->HA_J + MCp_Total);
                state.dataRoomAir->RoomOutflowTemp(ZoneNum) =
                    (ConvGainsJet + ConvGainsRec + state.dataCrossVentMgr->HAT_J + state.dataCrossVentMgr->HAT_R +
                     state.dataRoomAir->Tin(ZoneNum) * MCp_Total - state.dataCrossVentMgr->HA_J * state.dataRoomAir->ZTJET(ZoneNum) -
                     state.dataCrossVentMgr->HA_R * state.dataRoomAir->ZTREC(ZoneNum)) /
                    MCp_Total;
            }
            if (state.dataRoomAir->JetRecAreaRatio(ZoneNum) == 1.0) {
                state.dataRoomAir->ZoneCrossVentHasREC(ZoneNum) = 0.0;
                state.dataRoomAir->ZTREC(ZoneNum) = state.dataRoomAir->RoomOutflowTemp(ZoneNum);
                state.dataRoomAir->ZTREC(ZoneNum) = state.dataRoomAir->ZTJET(ZoneNum);
                state.dataRoomAir->ZTREC(ZoneNum) = state.dataRoomAir->ZTJET(ZoneNum);
            }
            // If temperature increase is above 1.5C then go to mixing
            if (state.dataRoomAir->RoomOutflowTemp(ZoneNum) - state.dataRoomAir->Tin(ZoneNum) > 1.5) {
                state.dataRoomAir->ZoneCrossVentIsMixing(ZoneNum) = 1.0;
                state.dataRoomAir->ZoneCrossVentHasREC(ZoneNum) = 0.0;
                state.dataRoomAir->AirModel(ZoneNum).SimAirModel = false;
                state.dataRoomAir->Ujet(ZoneNum) = 0.0;
                state.dataRoomAir->Urec(ZoneNum) = 0.0;
                state.dataRoomAir->Qrec(ZoneNum) = 0.0;
                state.dataRoomAir->RecInflowRatio(ZoneNum) = 0.0;
                for (auto &e : state.dataRoomAir->CrossVentJetRecFlows) {
                    e.Ujet = 0.0;
                    e.Urec = 0.0;
                }
                for (int Ctd = 1; Ctd <= 3; ++Ctd) {
                    Real64 ZTAveraged = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT;
                    state.dataRoomAir->RoomOutflowTemp(ZoneNum) = ZTAveraged;
                    state.dataRoomAir->ZTJET(ZoneNum) = ZTAveraged;
                    state.dataRoomAir->ZTREC(ZoneNum) = ZTAveraged;
                    state.dataRoomAir->RoomOutflowTemp(ZoneNum) = ZTAveraged;
                    state.dataRoomAir->ZTREC(ZoneNum) = ZTAveraged;
                    state.dataRoomAir->ZTJET(ZoneNum) = ZTAveraged;
                    state.dataRoomAir->ZTREC(ZoneNum) = ZTAveraged;
                    HcCrossVent(state, ZoneNum);
                    ZTAveraged = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT;
                    state.dataRoomAir->RoomOutflowTemp(ZoneNum) = ZTAveraged;
                    state.dataRoomAir->ZTJET(ZoneNum) = ZTAveraged;
                    state.dataRoomAir->ZTREC(ZoneNum) = ZTAveraged;
                    state.dataRoomAir->RoomOutflowTemp(ZoneNum) = ZTAveraged;
                    state.dataRoomAir->ZTREC(ZoneNum) = ZTAveraged;
                    state.dataRoomAir->ZTJET(ZoneNum) = ZTAveraged;
                    state.dataRoomAir->ZTREC(ZoneNum) = ZTAveraged;
                }
            }
        } else {
            //=============================== M I X E D  Calculation ======================================================
            state.dataRoomAir->ZoneCrossVentIsMixing(ZoneNum) = 1.0;
            state.dataRoomAir->ZoneCrossVentHasREC(ZoneNum) = 0.0;
            state.dataRoomAir->Ujet(ZoneNum) = 0.0;
            state.dataRoomAir->Urec(ZoneNum) = 0.0;
            state.dataRoomAir->Qrec(ZoneNum) = 0.0;
            state.dataRoomAir->RecInflowRatio(ZoneNum) = 0.0;
            for (auto &e : state.dataRoomAir->CrossVentJetRecFlows) {
                e.Ujet = 0.0;
                e.Urec = 0.0;
            }
            for (int Ctd = 1; Ctd <= 3; ++Ctd) {
                Real64 ZTAveraged = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT;
                state.dataRoomAir->RoomOutflowTemp(ZoneNum) = ZTAveraged;
                state.dataRoomAir->ZTJET(ZoneNum) = ZTAveraged;
                state.dataRoomAir->ZTREC(ZoneNum) = ZTAveraged;
                state.dataRoomAir->RoomOutflowTemp(ZoneNum) = ZTAveraged;
                state.dataRoomAir->ZTREC(ZoneNum) = ZTAveraged;
                state.dataRoomAir->ZTJET(ZoneNum) = ZTAveraged;
                state.dataRoomAir->ZTREC(ZoneNum) = ZTAveraged;
                HcCrossVent(state, ZoneNum);
                ZTAveraged = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT;
                state.dataRoomAir->RoomOutflowTemp(ZoneNum) = ZTAveraged;
                state.dataRoomAir->ZTJET(ZoneNum) = ZTAveraged;
                state.dataRoomAir->ZTREC(ZoneNum) = ZTAveraged;
                state.dataRoomAir->RoomOutflowTemp(ZoneNum) = ZTAveraged;
                state.dataRoomAir->ZTREC(ZoneNum) = ZTAveraged;
                state.dataRoomAir->ZTJET(ZoneNum) = ZTAveraged;
                state.dataRoomAir->ZTREC(ZoneNum) = ZTAveraged;
            }
        }
    }

} // namespace RoomAir

} // namespace EnergyPlus
