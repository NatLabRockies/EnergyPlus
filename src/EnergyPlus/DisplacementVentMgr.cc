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
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/member.functions.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DisplacementVentMgr.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace RoomAir {

    // MODULE INFORMATION:
    //       AUTHOR         G. Carrilho da Graca
    //       DATE WRITTEN   February 2004
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // Routines that implement the UCSD Displacement Ventilation

    // Using/Aliasing
    using namespace DataLoopNode;
    using namespace DataEnvironment;
    using namespace DataHeatBalance;
    using namespace DataHeatBalSurface;
    using namespace DataSurfaces;
    using Convect::CalcDetailedHcInForDVModel;

    void ManageDispVent3Node(EnergyPlusData &state, int const ZoneNum) // index number for the specified zone
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         G. Carrilho da Graca
        //       DATE WRITTEN   February 2004

        // PURPOSE OF THIS SUBROUTINE:
        //   manage the UCSD Displacement Ventilation model

        // initialize Displacement Ventilation model
        InitDispVent3Node(state, ZoneNum);

        // perform Displacement Ventilation model calculations
        CalcDispVent3Node(state, ZoneNum);
    }

    //**************************************************************************************************

    void InitDispVent3Node(EnergyPlusData &state, int const ZoneNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         G. Carrilho da Graca
        //       DATE WRITTEN   February 2004
        //       MODIFIED       -
        //       RE-ENGINEERED  -

        // PURPOSE OF THIS SUBROUTINE:
        // Low Energy Cooling by Ventilation initialization subroutine.
        // All the data preparation needed to run the LECV models.
        // The subroutines sets up arrays with the locations in the main EnergyPlus surface array of
        // ceiling, windows, doors and walls. The zone maximum and minimum height is calculated.

        // Do the one time initializations
        if (state.dataDispVentMgr->InitUCSDDVMyOneTimeFlag) {
            state.dataDispVentMgr->MyEnvrnFlag.dimension(state.dataGlobal->NumOfZones, true);
            state.dataDispVentMgr->HeightFloorSubzoneTop = 0.2;
            state.dataDispVentMgr->ThickOccupiedSubzoneMin = 0.2;
            state.dataDispVentMgr->HeightIntMassDefault = 2.0;
            state.dataDispVentMgr->InitUCSDDVMyOneTimeFlag = false;
        }

        // Do the begin environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataDispVentMgr->MyEnvrnFlag(ZoneNum)) {
            state.dataDispVentMgr->HAT_MX = 0.0;
            state.dataDispVentMgr->HAT_OC = 0.0;
            state.dataDispVentMgr->HA_MX = 0.0;
            state.dataDispVentMgr->HA_OC = 0.0;
            state.dataDispVentMgr->HAT_FLOOR = 0.0;
            state.dataDispVentMgr->HA_FLOOR = 0.0;
            state.dataDispVentMgr->MyEnvrnFlag(ZoneNum) = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataDispVentMgr->MyEnvrnFlag(ZoneNum) = true;
        }

        // initialize these module variables every timestep
        state.dataDispVentMgr->HeightIntMass = state.dataDispVentMgr->HeightIntMassDefault;
    }

    //**************************************************************************************************

    void HcDispVent3Node(EnergyPlusData &state, int const ZoneNum, Real64 const FractionHeight)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         G. Carrilho da Graca
        //       DATE WRITTEN   February 2004
        //       MODIFIED       -
        //       RE-ENGINEERED  -

        // PURPOSE OF THIS SUBROUTINE:
        // Main subroutine for convection calculation in the UCSD Displacement Ventilation model.
        // It calls CalcDetailedHcInForDVModel for convection coefficient
        // initial calculations and averages the final result comparing the position of the surface with
        // the interface subzone height.

        // Using/Aliasing
        using namespace DataEnvironment;
        using namespace DataHeatBalance;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 HLD;      // Convection coefficient for the lower area of surface
        Real64 TmedDV;   // Average temperature for DV
        Real64 Z1;       // auxiliary var for lowest height
        Real64 Z2;       // auxiliary var for highest height
        Real64 ZSupSurf; // highest height for this surface
        Real64 ZInfSurf; // lowest height for this surface
        Real64 HLU;      // Convection coefficient for the upper area of surface
        Real64 LayH;     // Height of the Occupied/Mixed subzone interface
        Real64 LayFrac;  // Fraction height of the Occupied/Mixed subzone interface

        state.dataDispVentMgr->HAT_MX = 0.0;
        state.dataDispVentMgr->HAT_OC = 0.0;
        state.dataDispVentMgr->HA_MX = 0.0;
        state.dataDispVentMgr->HA_OC = 0.0;
        state.dataDispVentMgr->HAT_FLOOR = 0.0;
        state.dataDispVentMgr->HA_FLOOR = 0.0;
        auto &SurfTempIn(state.dataHeatBalSurf->SurfTempIn);

        // Is the air flow model for this zone set to UCSDDV Displacement Ventilation?
        if (state.dataRoomAir->IsZoneDispVent3Node(ZoneNum)) {
            LayFrac = FractionHeight;
            LayH = FractionHeight * (state.dataRoomAir->ZoneCeilingHeight2(ZoneNum) - state.dataRoomAir->ZoneCeilingHeight1(ZoneNum));
            // WALL Hc, HA and HAT calculation
            for (int Ctd = state.dataRoomAir->PosZ_Wall(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Wall(ZoneNum).end; ++Ctd) {
                int SurfNum = state.dataRoomAir->APos_Wall(Ctd);
                if (SurfNum == 0) {
                    continue;
                }

                auto const &surf = state.dataSurface->Surface(SurfNum);
                state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
                state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];
                Z1 = minval(surf.Vertex, &Vector::z);
                Z2 = maxval(surf.Vertex, &Vector::z);
                ZSupSurf = Z2 - state.dataRoomAir->ZoneCeilingHeight1(ZoneNum);
                ZInfSurf = Z1 - state.dataRoomAir->ZoneCeilingHeight1(ZoneNum);

                // The Wall surface is in the upper subzone
                if (ZInfSurf > LayH) {
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                    state.dataRoomAir->HWall(Ctd) = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                    state.dataDispVentMgr->HAT_MX += surf.Area * SurfTempIn(SurfNum) * state.dataRoomAir->HWall(Ctd);
                    state.dataDispVentMgr->HA_MX += surf.Area * state.dataRoomAir->HWall(Ctd);
                }

                // The Wall surface is in the lower subzone
                if (ZSupSurf < LayH) {
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                    state.dataRoomAir->HWall(Ctd) = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                    state.dataDispVentMgr->HAT_OC += surf.Area * SurfTempIn(SurfNum) * state.dataRoomAir->HWall(Ctd);
                    state.dataDispVentMgr->HA_OC += surf.Area * state.dataRoomAir->HWall(Ctd);
                }

                // The Wall surface is partially in upper and partially in lower subzone
                if (ZInfSurf <= LayH && ZSupSurf >= LayH) {
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                    HLU = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                    HLD = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                    TmedDV = ((ZSupSurf - LayH) * state.dataRoomAir->ZTMX(ZoneNum) + (LayH - ZInfSurf) * state.dataRoomAir->ZTOC(ZoneNum)) /
                             (ZSupSurf - ZInfSurf);
                    state.dataRoomAir->HWall(Ctd) = ((LayH - ZInfSurf) * HLD + (ZSupSurf - LayH) * HLU) / (ZSupSurf - ZInfSurf);
                    state.dataDispVentMgr->HAT_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * SurfTempIn(SurfNum) * HLU;
                    state.dataDispVentMgr->HA_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * HLU;
                    state.dataDispVentMgr->HAT_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * SurfTempIn(SurfNum) * HLD;
                    state.dataDispVentMgr->HA_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * HLD;
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = TmedDV;
                }

                state.dataRoomAir->DispVent3NodeHcIn(SurfNum) = state.dataRoomAir->HWall(Ctd);

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
                    Z1 = minval(surf.Vertex, &Vector::z);
                    Z2 = maxval(surf.Vertex, &Vector::z);
                    ZSupSurf = Z2 - state.dataRoomAir->ZoneCeilingHeight1(ZoneNum);
                    ZInfSurf = Z1 - state.dataRoomAir->ZoneCeilingHeight1(ZoneNum);

                    if (ZInfSurf > LayH) {
                        state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                        CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                        state.dataRoomAir->HWindow(Ctd) = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                        state.dataDispVentMgr->HAT_MX += surf.Area * SurfTempIn(SurfNum) * state.dataRoomAir->HWindow(Ctd);
                        state.dataDispVentMgr->HA_MX += surf.Area * state.dataRoomAir->HWindow(Ctd);
                    }

                    if (ZSupSurf < LayH) {
                        state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                        CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                        state.dataRoomAir->HWindow(Ctd) = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                        state.dataDispVentMgr->HAT_OC += surf.Area * SurfTempIn(SurfNum) * state.dataRoomAir->HWindow(Ctd);
                        state.dataDispVentMgr->HA_OC += surf.Area * state.dataRoomAir->HWindow(Ctd);
                    }

                    if (ZInfSurf <= LayH && ZSupSurf >= LayH) {
                        state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                        CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                        HLU = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                        state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                        CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                        HLD = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                        TmedDV = ((ZSupSurf - LayH) * state.dataRoomAir->ZTMX(ZoneNum) + (LayH - ZInfSurf) * state.dataRoomAir->ZTOC(ZoneNum)) /
                                 (ZSupSurf - ZInfSurf);
                        state.dataRoomAir->HWindow(Ctd) = ((LayH - ZInfSurf) * HLD + (ZSupSurf - LayH) * HLU) / (ZSupSurf - ZInfSurf);
                        state.dataDispVentMgr->HAT_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * SurfTempIn(SurfNum) * HLU;
                        state.dataDispVentMgr->HA_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * HLU;
                        state.dataDispVentMgr->HAT_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * SurfTempIn(SurfNum) * HLD;
                        state.dataDispVentMgr->HA_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * HLD;
                        state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = TmedDV;
                    }
                }

                if (surf.Tilt <= 10.0) { // Window Ceiling
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                    state.dataRoomAir->HWindow(Ctd) = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                    state.dataDispVentMgr->HAT_MX += surf.Area * SurfTempIn(SurfNum) * state.dataRoomAir->HWindow(Ctd);
                    state.dataDispVentMgr->HA_MX += surf.Area * state.dataRoomAir->HWindow(Ctd);
                }

                if (surf.Tilt >= 170.0) { // Window Floor
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                    state.dataRoomAir->HWindow(Ctd) = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                    state.dataDispVentMgr->HAT_OC += surf.Area * SurfTempIn(SurfNum) * state.dataRoomAir->HWindow(Ctd);
                    state.dataDispVentMgr->HA_OC += surf.Area * state.dataRoomAir->HWindow(Ctd);
                }

                state.dataRoomAir->DispVent3NodeHcIn(SurfNum) = state.dataRoomAir->HWindow(Ctd);

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
                if (surf.Tilt > 10.0 && surf.Tilt < 170.0) { // Door Wall
                    Z1 = minval(surf.Vertex, &Vector::z);
                    Z2 = maxval(surf.Vertex, &Vector::z);
                    ZSupSurf = Z2 - state.dataRoomAir->ZoneCeilingHeight1(ZoneNum);
                    ZInfSurf = Z1 - state.dataRoomAir->ZoneCeilingHeight1(ZoneNum);

                    if (ZInfSurf > LayH) {
                        state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                        CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                        state.dataRoomAir->HDoor(Ctd) = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                        state.dataDispVentMgr->HAT_MX += surf.Area * SurfTempIn(SurfNum) * state.dataRoomAir->HDoor(Ctd);
                        state.dataDispVentMgr->HA_MX += surf.Area * state.dataRoomAir->HDoor(Ctd);
                    }

                    if (ZSupSurf < LayH) {
                        state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                        CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                        state.dataRoomAir->HDoor(Ctd) = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                        state.dataDispVentMgr->HAT_OC += surf.Area * SurfTempIn(SurfNum) * state.dataRoomAir->HDoor(Ctd);
                        state.dataDispVentMgr->HA_OC += surf.Area * state.dataRoomAir->HDoor(Ctd);
                    }

                    if (ZInfSurf <= LayH && ZSupSurf >= LayH) {
                        state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                        CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                        HLU = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                        state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                        CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                        HLD = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                        TmedDV = ((ZSupSurf - LayH) * state.dataRoomAir->ZTMX(ZoneNum) + (LayH - ZInfSurf) * state.dataRoomAir->ZTOC(ZoneNum)) /
                                 (ZSupSurf - ZInfSurf);
                        state.dataRoomAir->HDoor(Ctd) = ((LayH - ZInfSurf) * HLD + (ZSupSurf - LayH) * HLU) / (ZSupSurf - ZInfSurf);
                        state.dataDispVentMgr->HAT_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * SurfTempIn(SurfNum) * HLU;
                        state.dataDispVentMgr->HA_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * HLU;
                        state.dataDispVentMgr->HAT_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * SurfTempIn(SurfNum) * HLD;
                        state.dataDispVentMgr->HA_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * HLD;
                        state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = TmedDV;
                    }
                }

                if (surf.Tilt <= 10.0) { // Door Ceiling
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                    state.dataRoomAir->HDoor(Ctd) = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                    state.dataDispVentMgr->HAT_MX += surf.Area * SurfTempIn(SurfNum) * state.dataRoomAir->HDoor(Ctd);
                    state.dataDispVentMgr->HA_MX += surf.Area * state.dataRoomAir->HDoor(Ctd);
                }

                if (surf.Tilt >= 170.0) { // Door Floor
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                    state.dataRoomAir->HDoor(Ctd) = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                    state.dataDispVentMgr->HAT_OC += surf.Area * SurfTempIn(SurfNum) * state.dataRoomAir->HDoor(Ctd);
                    state.dataDispVentMgr->HA_OC += surf.Area * state.dataRoomAir->HDoor(Ctd);
                }

                state.dataRoomAir->DispVent3NodeHcIn(SurfNum) = state.dataRoomAir->HDoor(Ctd);

            } // END DOOR

            // INTERNAL Hc, HA and HAT CALCULATION
            state.dataDispVentMgr->HeightIntMass =
                min(state.dataDispVentMgr->HeightIntMassDefault,
                    (state.dataRoomAir->ZoneCeilingHeight2(ZoneNum) - state.dataRoomAir->ZoneCeilingHeight1(ZoneNum)));
            for (int Ctd = state.dataRoomAir->PosZ_Internal(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Internal(ZoneNum).end; ++Ctd) {
                int SurfNum = state.dataRoomAir->APos_Internal(Ctd);
                if (SurfNum == 0) {
                    continue;
                }

                auto const &surf = state.dataSurface->Surface(SurfNum);
                state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
                state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];
                ZSupSurf = state.dataDispVentMgr->HeightIntMass;
                ZInfSurf = 0.0;

                if (ZSupSurf < LayH) {
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                    state.dataRoomAir->HInternal(Ctd) = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                    state.dataDispVentMgr->HAT_OC += surf.Area * SurfTempIn(SurfNum) * state.dataRoomAir->HInternal(Ctd);
                    state.dataDispVentMgr->HA_OC += surf.Area * state.dataRoomAir->HInternal(Ctd);
                }

                if (ZInfSurf <= LayH && ZSupSurf >= LayH) {
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                    HLU = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                    HLD = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                    TmedDV = ((ZSupSurf - LayH) * state.dataRoomAir->ZTMX(ZoneNum) + (LayH - ZInfSurf) * state.dataRoomAir->ZTOC(ZoneNum)) /
                             (ZSupSurf - ZInfSurf);
                    state.dataRoomAir->HInternal(Ctd) = ((LayH - ZInfSurf) * HLD + (ZSupSurf - LayH) * HLU) / (ZSupSurf - ZInfSurf);
                    state.dataDispVentMgr->HAT_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * SurfTempIn(SurfNum) * HLU;
                    state.dataDispVentMgr->HA_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * HLU;
                    state.dataDispVentMgr->HAT_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * SurfTempIn(SurfNum) * HLD;
                    state.dataDispVentMgr->HA_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * HLD;
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = TmedDV;
                }

                state.dataRoomAir->DispVent3NodeHcIn(SurfNum) = state.dataRoomAir->HInternal(Ctd);
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
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                state.dataRoomAir->HCeiling(Ctd) = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                state.dataDispVentMgr->HAT_MX += surf.Area * SurfTempIn(SurfNum) * state.dataRoomAir->HCeiling(Ctd);
                state.dataDispVentMgr->HA_MX += surf.Area * state.dataRoomAir->HCeiling(Ctd);
                state.dataRoomAir->DispVent3NodeHcIn(SurfNum) = state.dataRoomAir->HCeiling(Ctd);
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
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTFloor(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, SurfTempIn, state.dataRoomAir->DispVent3NodeHcIn);
                state.dataRoomAir->HFloor(Ctd) = state.dataRoomAir->DispVent3NodeHcIn(SurfNum);
                state.dataDispVentMgr->HAT_FLOOR += surf.Area * SurfTempIn(SurfNum) * state.dataRoomAir->HFloor(Ctd);
                state.dataDispVentMgr->HA_FLOOR += surf.Area * state.dataRoomAir->HFloor(Ctd);
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTFloor(ZoneNum);
                state.dataRoomAir->DispVent3NodeHcIn(SurfNum) = state.dataRoomAir->HFloor(Ctd);
            } // END FLOOR
        }
    }

    //**************************************************************************************************

    Real64 calculateThirdOrderFloorTemperature(Real64 temperatureHistoryTerm,
                                               Real64 HAT_floor,
                                               Real64 HA_floor,
                                               Real64 MCpT_Total,
                                               Real64 MCp_Total,
                                               Real64 occupiedTemp,
                                               Real64 nonAirSystemResponse,
                                               Real64 zoneMultiplier,
                                               Real64 airCap)
    {
        const Real64 elevenOverSix = 11.0 / 6.0;
        return (temperatureHistoryTerm + HAT_floor + MCpT_Total + 0.6 * occupiedTemp * MCp_Total + nonAirSystemResponse / zoneMultiplier) /
               (elevenOverSix * airCap + HA_floor + 1.6 * MCp_Total);
    }

    void CalcDispVent3Node(EnergyPlusData &state, int const ZoneNum) // Which Zonenum
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         G. Carrilho da Graca
        //       DATE WRITTEN   February 2004
        //       MODIFIED       Brent Griffith June 2008 for new interpolation and time history
        //       RE-ENGINEERED  -

        // PURPOSE OF THIS SUBROUTINE:
        // Subroutine for displacement ventilation modelling.
        // This subroutine calculates the mixed subzone height, surface heat transfer coefficients and
        // room air equivalent temperatures and three space temperatures (floor subzone, occupied zone and upper,
        // mixed subzone temperature)

        // REFERENCES:
        // Model developed by Paul Linden (UCSD), G. Carrilho da Graca (UCSD) and P. Haves (LBL).
        // Work funded by the California Energy Comission. More information on the model can found in:
        // "Simplified Models for Heat Transfer in Rooms" G. Carrilho da Graca, Ph.D. thesis UCSD. December 2003.

        // Using/Aliasing
        using namespace DataEnvironment;
        using namespace DataHeatBalance;

        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        using Psychrometrics::PsyCpAirFnW;
        using Psychrometrics::PsyRhoAirFnPbTdbW;

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 const OneThird(1.0 / 3.0);
        Real64 const MinFlow_pow_fac(std::pow(1.0 / 24.55 * 1.0, 1.0 / 0.6));

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 HeightFrac;               // Fractional height of transition between occupied and mixed subzones
        Real64 GainsFrac;                // Fraction of lower subzone internal gains that mix as opposed to forming plumes
        Real64 ConvGains;                // Total convective gains in the room
        Real64 ConvGainsOccupiedSubzone; // Total convective gains released in occupied subzone
        Real64 ConvGainsMixedSubzone;    // Total convective gains released in mixed subzone
        Real64 MCp_Total;                // Total capacity rate into the zone - assumed to enter at low level
        Real64 ZTAveraged;
        Real64 TempDiffCritRep; // Minimum temperature difference between mixed and occupied subzones for reporting
        bool MIXFLAG;
        Real64 MinFlow;
        Real64 NumPLPP; // Number of plumes per person
        Real64 MTGAUX;
        int ZoneEquipConfigNum;
        Real64 PowerInPlumes;
        Real64 SumSysMCp;
        Real64 SumSysMCpT;
        Real64 NodeTemp;
        Real64 MassFlowRate;
        Real64 CpAir;
        Real64 MCpT_Total;
        Real64 NumberOfPlumes;
        Real64 SumMCp;
        Real64 SumMCpT;
        Real64 TempHistTerm;
        Real64 PowerPerPlume;
        Real64 HeightMixedSubzoneAve;    // Height of center of mixed air subzone
        Real64 HeightOccupiedSubzoneAve; // Height of center of occupied air subzone
        Real64 HeightFloorSubzoneAve;    // Height of center of floor air subzone
        Real64 HeightThermostat;         // Height of center of thermostat/temperature control sensor
        Real64 HeightComfort;            // Height at which air temperature value is used to calculate comfort
        Real64 CeilingHeight;
        Real64 ZoneMult; // total zone multiplier
        int FlagApertures;

        auto &TempDepCoef = state.dataDispVentMgr->TempDepCoef;
        auto &TempIndCoef = state.dataDispVentMgr->TempIndCoef;

        Real64 RetAirGain;
        assert(state.dataRoomAir->AirModel.allocated());

        // Exact solution or Euler method
        if (state.dataHeatBal->ZoneAirSolutionAlgo != DataHeatBalance::SolutionAlgo::ThirdOrder) {
            if (state.dataHVACGlobal->ShortenTimeStepSysRoomAir && TimeStepSys < state.dataGlobal->TimeStepZone) {
                if (state.dataHVACGlobal->PreviousTimeStep < state.dataGlobal->TimeStepZone) {
                    state.dataRoomAir->Zone1Floor(ZoneNum) = state.dataRoomAir->ZoneM2Floor(ZoneNum);
                    state.dataRoomAir->Zone1OC(ZoneNum) = state.dataRoomAir->ZoneM2OC(ZoneNum);
                    state.dataRoomAir->Zone1MX(ZoneNum) = state.dataRoomAir->ZoneM2MX(ZoneNum);
                } else {
                    state.dataRoomAir->Zone1Floor(ZoneNum) = state.dataRoomAir->ZoneMXFloor(ZoneNum);
                    state.dataRoomAir->Zone1OC(ZoneNum) = state.dataRoomAir->ZoneMXOC(ZoneNum);
                    state.dataRoomAir->Zone1MX(ZoneNum) = state.dataRoomAir->ZoneMXMX(ZoneNum);
                }
            } else {
                state.dataRoomAir->Zone1Floor(ZoneNum) = state.dataRoomAir->ZTFloor(ZoneNum);
                state.dataRoomAir->Zone1OC(ZoneNum) = state.dataRoomAir->ZTOC(ZoneNum);
                state.dataRoomAir->Zone1MX(ZoneNum) = state.dataRoomAir->ZTMX(ZoneNum);
            }
        }

        auto &zone = state.dataHeatBal->Zone(ZoneNum);

        MIXFLAG = false;
        FlagApertures = 1;
        state.dataRoomAir->DispVent3NodeHcIn = state.dataHeatBalSurf->SurfHConvInt;
        CeilingHeight = state.dataRoomAir->ZoneCeilingHeight2(ZoneNum) - state.dataRoomAir->ZoneCeilingHeight1(ZoneNum);
        ZoneMult = zone.Multiplier * zone.ListMultiplier;
        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);

        for (int Ctd = 1; Ctd <= state.dataRoomAir->TotDispVent3Node; ++Ctd) {
            auto &zoneDV3N = state.dataRoomAir->ZoneDispVent3Node(Ctd);
            if (ZoneNum == zoneDV3N.ZonePtr) {
                GainsFrac = zoneDV3N.gainsSched->getCurrentVal();
                NumPLPP = zoneDV3N.NumPlumesPerOcc;
                HeightThermostat = zoneDV3N.ThermostatHeight;
                HeightComfort = zoneDV3N.ComfortHeight;
                TempDiffCritRep = zoneDV3N.TempTrigger;
            }
        }

        ConvGainsOccupiedSubzone = InternalHeatGains::SumInternalConvectionGainsByTypes(state, ZoneNum, IntGainTypesOccupied);

        ConvGainsOccupiedSubzone += 0.5 * thisZoneHB.SysDepZoneLoadsLagged;

        // Add heat to return air if zonal system (no return air) or cycling system (return air frequently very
        // low or zero)
        if (zone.NoHeatToReturnAir) {
            RetAirGain = InternalHeatGains::SumReturnAirConvectionGainsByTypes(state, ZoneNum, IntGainTypesOccupied);
            ConvGainsOccupiedSubzone += RetAirGain;
        }

        ConvGainsMixedSubzone = InternalHeatGains::SumInternalConvectionGainsByTypes(state, ZoneNum, IntGainTypesMixedSubzone);
        ConvGainsMixedSubzone += state.dataHeatBalFanSys->SumConvHTRadSys(ZoneNum) + state.dataHeatBalFanSys->SumConvPool(ZoneNum) +
                                 0.5 * thisZoneHB.SysDepZoneLoadsLagged;
        if (zone.NoHeatToReturnAir) {
            RetAirGain = InternalHeatGains::SumReturnAirConvectionGainsByTypes(state, ZoneNum, IntGainTypesMixedSubzone);
            ConvGainsMixedSubzone += RetAirGain;
        }

        ConvGains = ConvGainsOccupiedSubzone + ConvGainsMixedSubzone;

        // Make sure all types of internal gains have been gathered
        assert((int)(size(IntGainTypesOccupied) + size(IntGainTypesMixedSubzone) + size(ExcludedIntGainTypes)) ==
               (int)DataHeatBalance::IntGainType::Num);

        //=================== Entering air system temperature and flow====================
        SumSysMCp = 0.0;
        SumSysMCpT = 0.0;
        // Check to make sure if this is a controlled zone and determine ZoneEquipConfigNum
        ZoneEquipConfigNum = ZoneNum;
        if (state.dataZoneEquip->ZoneEquipConfig(ZoneEquipConfigNum).IsControlled) {
            for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(ZoneEquipConfigNum).NumInletNodes; ++NodeNum) {
                NodeTemp = state.dataLoopNodes->Node(state.dataZoneEquip->ZoneEquipConfig(ZoneEquipConfigNum).InletNode(NodeNum)).Temp;
                MassFlowRate = state.dataLoopNodes->Node(state.dataZoneEquip->ZoneEquipConfig(ZoneEquipConfigNum).InletNode(NodeNum)).MassFlowRate;
                CpAir = PsyCpAirFnW(thisZoneHB.airHumRat);
                SumSysMCp += MassFlowRate * CpAir;
                SumSysMCpT += MassFlowRate * CpAir * NodeTemp;
            }
        }

        SumMCp = thisZoneHB.MCPI + thisZoneHB.MCPV + thisZoneHB.MCPM + thisZoneHB.MCPE + thisZoneHB.MCPC + thisZoneHB.MDotCPOA;
        SumMCpT =
            thisZoneHB.MCPTI + thisZoneHB.MCPTV + thisZoneHB.MCPTM + thisZoneHB.MCPTE + thisZoneHB.MCPTC + thisZoneHB.MDotCPOA * zone.OutDryBulbTemp;
        if (state.afn->simulation_control.type == AirflowNetwork::ControlType::MultizoneWithoutDistribution) {
            SumMCp = state.afn->exchangeData(ZoneNum).SumMCp + state.afn->exchangeData(ZoneNum).SumMVCp + state.afn->exchangeData(ZoneNum).SumMMCp;
            SumMCpT =
                state.afn->exchangeData(ZoneNum).SumMCpT + state.afn->exchangeData(ZoneNum).SumMVCpT + state.afn->exchangeData(ZoneNum).SumMMCpT;
        }

        MCp_Total = SumMCp + SumSysMCp;
        MCpT_Total = SumMCpT + SumSysMCpT;

        if (state.dataHeatBal->TotPeople > 0) {
            int NumberOfOccupants = 0;
            NumberOfPlumes = 0.0;
            for (int Ctd = 1; Ctd <= state.dataHeatBal->TotPeople; ++Ctd) {
                if (state.dataHeatBal->People(Ctd).ZonePtr == ZoneNum) {
                    NumberOfOccupants +=
                        state.dataHeatBal->People(Ctd).NumberOfPeople; // *GetCurrentScheduleValue(state, People(Ctd)%NumberOfPeoplePtr)
                    NumberOfPlumes = NumberOfOccupants * NumPLPP;
                }
            }
            if (NumberOfPlumes == 0.0) {
                NumberOfPlumes = 1.0;
            }
            PowerInPlumes = (1.0 - GainsFrac) * ConvGainsOccupiedSubzone;
            PowerPerPlume = PowerInPlumes / NumberOfPlumes;
        } else {
            NumberOfPlumes = 1.0;
            PowerInPlumes = (1.0 - GainsFrac) * ConvGainsOccupiedSubzone;
            PowerPerPlume = PowerInPlumes / NumberOfPlumes;
        }

        // When AirflowNetwork is used verify if bottom apertures are inflowing and upper apertures are
        // outflowing. The lower apertures have to be located below 0.8m and the upper apertures
        // have to be located above 1.8m.

        if (state.afn->NumOfLinksMultiZone > 0) {
            for (int Loop = 1; Loop <= state.dataRoomAir->AFNSurfaceCrossVent(0, ZoneNum); ++Loop) {
                // direct AirflowNetwork surface
                int afnSurfNum = state.dataRoomAir->AFNSurfaceCrossVent(Loop, ZoneNum);
                auto const &surfParams = state.dataRoomAir->SurfParametersCrossDispVent(afnSurfNum);
                auto const &afnLinkSimu = state.afn->AirflowNetworkLinkSimu(afnSurfNum);
                auto const &afnMzSurfData = state.afn->MultizoneSurfaceData(afnSurfNum);
                auto const &afnMzSurf = state.dataSurface->Surface(afnMzSurfData.SurfNum);
                if (afnMzSurf.Zone == ZoneNum) {

                    if ((surfParams.Zmax < 0.8 && afnLinkSimu.VolFLOW > 0)) {
                        FlagApertures = 0;
                        break;
                    }
                    if (surfParams.Zmin > 1.8 && afnLinkSimu.VolFLOW2 > 0) {
                        FlagApertures = 0;
                        break;
                    }

                    if ((surfParams.Zmin > 0.8 && surfParams.Zmin < 1.8) || (surfParams.Zmax > 0.8 && surfParams.Zmax < 1.8)) {
                        FlagApertures = 0;
                        break;
                    }
                    // indirect AirflowNetwork surface; this is an interzone surface
                } else {
                    auto const &afnZone = state.dataHeatBal->Zone(afnMzSurf.Zone);
                    if (surfParams.Zmax + afnZone.OriginZ - zone.OriginZ < 0.8 && afnLinkSimu.VolFLOW2 > 0) {
                        FlagApertures = 0;
                        break;
                    }
                    if (surfParams.Zmin + afnZone.OriginZ - zone.OriginZ > 1.8 && afnLinkSimu.VolFLOW > 0) {
                        FlagApertures = 0;
                        break;
                    }
                    if ((surfParams.Zmin + afnZone.OriginZ - zone.OriginZ > 0.8 && surfParams.Zmin + afnZone.OriginZ - zone.OriginZ < 1.8) ||
                        (surfParams.Zmax + afnZone.OriginZ - zone.OriginZ > 0.8 && surfParams.Zmax + afnZone.OriginZ - zone.OriginZ < 1.8)) {
                        FlagApertures = 0;
                        break;
                    }
                }
            }
        }

        if ((PowerInPlumes == 0.0) || (MCpT_Total == 0.0) || FlagApertures == 0) {
            // The system will mix
            HeightFrac = 0.0;
        } else {
            Real64 const plume_fac(NumberOfPlumes * std::pow(PowerPerPlume, OneThird));
            HeightFrac = min(24.55 * std::pow(MCp_Total * 0.000833 / plume_fac, 0.6) / CeilingHeight, 1.0);
            for (int Ctd = 1; Ctd <= 4; ++Ctd) {
                HcDispVent3Node(state, ZoneNum, HeightFrac);
                // HeightFrac = min( 24.55 * std::pow( MCp_Total * 0.000833 / ( NumberOfPlumes * std::pow( PowerPerPlume, OneThird ) ), 0.6 ) /
                // CeilingHeight, 1.0 ); //Tuned This does not vary in loop  EPTeam-replaces above (cause diffs)      HeightFrac =
                // MIN(24.55d0*(MCp_Total*0.000833d0/(NumberOfPlumes*PowerPerPlume**(1.0d0/3.d0)))**0.6 / CeilingHeight , 1.0d0)
                state.dataRoomAir->HeightTransition(ZoneNum) = HeightFrac * CeilingHeight;
                state.dataRoomAir->AIRRATFloor(ZoneNum) =
                    zone.Volume * min(state.dataRoomAir->HeightTransition(ZoneNum), state.dataDispVentMgr->HeightFloorSubzoneTop) / CeilingHeight *
                    zone.ZoneVolCapMultpSens *
                    PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, state.dataRoomAir->MATFloor(ZoneNum), thisZoneHB.airHumRat) *
                    PsyCpAirFnW(thisZoneHB.airHumRat) / TimeStepSysSec;
                state.dataRoomAir->AIRRATOC(ZoneNum) =
                    zone.Volume * (state.dataRoomAir->HeightTransition(ZoneNum) - min(state.dataRoomAir->HeightTransition(ZoneNum), 0.2)) /
                    CeilingHeight * zone.ZoneVolCapMultpSens *
                    PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, state.dataRoomAir->MATOC(ZoneNum), thisZoneHB.airHumRat) *
                    PsyCpAirFnW(thisZoneHB.airHumRat) / TimeStepSysSec;
                state.dataRoomAir->AIRRATMX(ZoneNum) =
                    zone.Volume * (CeilingHeight - state.dataRoomAir->HeightTransition(ZoneNum)) / CeilingHeight * zone.ZoneVolCapMultpSens *
                    PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, state.dataRoomAir->MATMX(ZoneNum), thisZoneHB.airHumRat) *
                    PsyCpAirFnW(thisZoneHB.airHumRat) / TimeStepSysSec;

                if (state.dataHVACGlobal->UseZoneTimeStepHistory) {
                    state.dataRoomAir->ZTMFloor(ZoneNum)[2] = state.dataRoomAir->XMATFloor(ZoneNum)[2];
                    state.dataRoomAir->ZTMFloor(ZoneNum)[1] = state.dataRoomAir->XMATFloor(ZoneNum)[1];
                    state.dataRoomAir->ZTMFloor(ZoneNum)[0] = state.dataRoomAir->XMATFloor(ZoneNum)[0];

                    state.dataRoomAir->ZTMOC(ZoneNum)[2] = state.dataRoomAir->XMATOC(ZoneNum)[2];
                    state.dataRoomAir->ZTMOC(ZoneNum)[1] = state.dataRoomAir->XMATOC(ZoneNum)[1];
                    state.dataRoomAir->ZTMOC(ZoneNum)[0] = state.dataRoomAir->XMATOC(ZoneNum)[0];

                    state.dataRoomAir->ZTMMX(ZoneNum)[2] = state.dataRoomAir->XMATMX(ZoneNum)[2];
                    state.dataRoomAir->ZTMMX(ZoneNum)[1] = state.dataRoomAir->XMATMX(ZoneNum)[1];
                    state.dataRoomAir->ZTMMX(ZoneNum)[0] = state.dataRoomAir->XMATMX(ZoneNum)[0];

                } else {
                    state.dataRoomAir->ZTMFloor(ZoneNum)[2] = state.dataRoomAir->DSXMATFloor(ZoneNum)[2];
                    state.dataRoomAir->ZTMFloor(ZoneNum)[1] = state.dataRoomAir->DSXMATFloor(ZoneNum)[1];
                    state.dataRoomAir->ZTMFloor(ZoneNum)[0] = state.dataRoomAir->DSXMATFloor(ZoneNum)[0];

                    state.dataRoomAir->ZTMOC(ZoneNum)[2] = state.dataRoomAir->DSXMATOC(ZoneNum)[2];
                    state.dataRoomAir->ZTMOC(ZoneNum)[1] = state.dataRoomAir->DSXMATOC(ZoneNum)[1];
                    state.dataRoomAir->ZTMOC(ZoneNum)[0] = state.dataRoomAir->DSXMATOC(ZoneNum)[0];

                    state.dataRoomAir->ZTMMX(ZoneNum)[2] = state.dataRoomAir->DSXMATMX(ZoneNum)[2];
                    state.dataRoomAir->ZTMMX(ZoneNum)[1] = state.dataRoomAir->DSXMATMX(ZoneNum)[1];
                    state.dataRoomAir->ZTMMX(ZoneNum)[0] = state.dataRoomAir->DSXMATMX(ZoneNum)[0];
                }

                Real64 AirCap = state.dataRoomAir->AIRRATFloor(ZoneNum);
                TempHistTerm = AirCap * (3.0 * state.dataRoomAir->ZTMFloor(ZoneNum)[0] - (3.0 / 2.0) * state.dataRoomAir->ZTMFloor(ZoneNum)[1] +
                                         OneThird * state.dataRoomAir->ZTMFloor(ZoneNum)[2]);
                TempDepCoef = state.dataDispVentMgr->HA_FLOOR + MCp_Total;
                TempIndCoef = state.dataDispVentMgr->HAT_FLOOR + MCpT_Total + thisZoneHB.NonAirSystemResponse / ZoneMult;
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    state.dataRoomAir->ZTFloor(ZoneNum) = calculateThirdOrderFloorTemperature(TempHistTerm,
                                                                                              state.dataDispVentMgr->HAT_FLOOR,
                                                                                              state.dataDispVentMgr->HA_FLOOR,
                                                                                              MCpT_Total,
                                                                                              MCp_Total,
                                                                                              state.dataRoomAir->ZTOC(ZoneNum),
                                                                                              thisZoneHB.NonAirSystemResponse,
                                                                                              ZoneMult,
                                                                                              AirCap);
                } break;
                case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                    if (TempDepCoef == 0.0) { // B=0
                        state.dataRoomAir->ZTFloor(ZoneNum) = state.dataRoomAir->Zone1Floor(ZoneNum) + TempIndCoef / AirCap;
                    } else {
                        state.dataRoomAir->ZTFloor(ZoneNum) =
                            (state.dataRoomAir->Zone1Floor(ZoneNum) - TempIndCoef / TempDepCoef) * std::exp(min(700.0, -TempDepCoef / AirCap)) +
                            TempIndCoef / TempDepCoef;
                    }
                } break;
                case DataHeatBalance::SolutionAlgo::EulerMethod: {
                    state.dataRoomAir->ZTFloor(ZoneNum) = (AirCap * state.dataRoomAir->Zone1Floor(ZoneNum) + TempIndCoef) / (AirCap + TempDepCoef);
                } break;
                default:
                    break;
                }
                AirCap = state.dataRoomAir->AIRRATOC(ZoneNum);
                TempHistTerm = AirCap * (3.0 * state.dataRoomAir->ZTMOC(ZoneNum)[0] - (3.0 / 2.0) * state.dataRoomAir->ZTMOC(ZoneNum)[1] +
                                         OneThird * state.dataRoomAir->ZTMOC(ZoneNum)[2]);
                TempDepCoef = state.dataDispVentMgr->HA_OC + MCp_Total;
                TempIndCoef = ConvGainsOccupiedSubzone * GainsFrac + state.dataDispVentMgr->HAT_OC + state.dataRoomAir->ZTFloor(ZoneNum) * MCp_Total;
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    state.dataRoomAir->ZTOC(ZoneNum) = (TempHistTerm + ConvGainsOccupiedSubzone * GainsFrac + state.dataDispVentMgr->HAT_OC +
                                                        1.6 * state.dataRoomAir->ZTFloor(ZoneNum) * MCp_Total) /
                                                       ((11.0 / 6.0) * AirCap + state.dataDispVentMgr->HA_OC + 1.6 * MCp_Total);
                } break;
                case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                    if (TempDepCoef == 0.0) { // B=0
                        state.dataRoomAir->ZTOC(ZoneNum) = state.dataRoomAir->Zone1OC(ZoneNum) + TempIndCoef / AirCap;
                    } else {
                        if (AirCap == 0.0) {
                            state.dataRoomAir->ZTOC(ZoneNum) = TempIndCoef / TempDepCoef;
                        } else {
                            state.dataRoomAir->ZTOC(ZoneNum) =
                                (state.dataRoomAir->Zone1OC(ZoneNum) - TempIndCoef / TempDepCoef) * std::exp(min(700.0, -TempDepCoef / AirCap)) +
                                TempIndCoef / TempDepCoef;
                        }
                    }
                } break;
                case DataHeatBalance::SolutionAlgo::EulerMethod: {
                    state.dataRoomAir->ZTOC(ZoneNum) = (AirCap * state.dataRoomAir->Zone1OC(ZoneNum) + TempIndCoef) / (AirCap + TempDepCoef);
                } break;
                default:
                    break;
                }
                AirCap = state.dataRoomAir->AIRRATMX(ZoneNum);
                TempHistTerm = AirCap * (3.0 * state.dataRoomAir->ZTMMX(ZoneNum)[0] - (3.0 / 2.0) * state.dataRoomAir->ZTMMX(ZoneNum)[1] +
                                         OneThird * state.dataRoomAir->ZTMMX(ZoneNum)[2]);
                TempDepCoef = state.dataDispVentMgr->HA_MX + MCp_Total;
                TempIndCoef = ConvGainsOccupiedSubzone * (1.0 - GainsFrac) + ConvGainsMixedSubzone + state.dataDispVentMgr->HAT_MX +
                              state.dataRoomAir->ZTOC(ZoneNum) * MCp_Total;
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    state.dataRoomAir->ZTMX(ZoneNum) = (TempHistTerm + ConvGainsOccupiedSubzone * (1.0 - GainsFrac) + ConvGainsMixedSubzone +
                                                        state.dataDispVentMgr->HAT_MX + state.dataRoomAir->ZTOC(ZoneNum) * MCp_Total) /
                                                       ((11.0 / 6.0) * AirCap + state.dataDispVentMgr->HA_MX + MCp_Total);
                } break;
                case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                    if (TempDepCoef == 0.0) { // B=0
                        state.dataRoomAir->ZTMX(ZoneNum) = state.dataRoomAir->Zone1MX(ZoneNum) + TempIndCoef / AirCap;
                    } else {
                        if (AirCap == 0.0) {
                            state.dataRoomAir->ZTMX(ZoneNum) = TempIndCoef / TempDepCoef;
                        } else {
                            state.dataRoomAir->ZTMX(ZoneNum) =
                                (state.dataRoomAir->Zone1MX(ZoneNum) - TempIndCoef / TempDepCoef) * std::exp(min(700.0, -TempDepCoef / AirCap)) +
                                TempIndCoef / TempDepCoef;
                        }
                    }
                } break;
                case DataHeatBalance::SolutionAlgo::EulerMethod: {
                    state.dataRoomAir->ZTMX(ZoneNum) = (AirCap * state.dataRoomAir->Zone1MX(ZoneNum) + TempIndCoef) / (AirCap + TempDepCoef);
                } break;
                default:
                    break;
                }
            }

            // MinFlow for interface layer at z = 1.0
            MinFlow = MinFlow_pow_fac * plume_fac;
            // EPTeam above replaces (cause diffs?)   MinFlow = (1.0d0/24.55d0*1.0d0)**(1.0d0/0.6d0)*NumberOfPlumes*PowerPerPlume**(1.0/3.0)
            if (MinFlow != 0.0) {
                state.dataRoomAir->FracMinFlow(ZoneNum) = MCp_Total * 0.000833 / MinFlow;
            } else {
                state.dataRoomAir->FracMinFlow(ZoneNum) = 9.999;
            }
            state.dataRoomAir->AirModel(ZoneNum).SimAirModel = true;
        }

        //=============================== M I X E D  Calculation ==============================================
        if (state.dataRoomAir->ZTMX(ZoneNum) < state.dataRoomAir->ZTOC(ZoneNum) || MCp_Total <= 0.0 ||
            HeightFrac * CeilingHeight < (state.dataDispVentMgr->HeightFloorSubzoneTop + state.dataDispVentMgr->ThickOccupiedSubzoneMin)) {
            MIXFLAG = true;
            HeightFrac = 0.0;
            state.dataRoomAir->AvgTempGrad(ZoneNum) = 0.0;
            state.dataRoomAir->MaxTempGrad(ZoneNum) = 0.0;
            state.dataRoomAir->AirModel(ZoneNum).SimAirModel = false;
            Real64 const thisZoneT1 = thisZoneHB.T1;
            Real64 AirCap = thisZoneHB.AirPowerCap;
            TempHistTerm = AirCap * (3.0 * thisZoneHB.ZTM[0] - (3.0 / 2.0) * thisZoneHB.ZTM[1] + OneThird * thisZoneHB.ZTM[2]);

            for (int Ctd = 1; Ctd <= 3; ++Ctd) {
                TempDepCoef = state.dataDispVentMgr->HA_MX + state.dataDispVentMgr->HA_OC + state.dataDispVentMgr->HA_FLOOR + MCp_Total;
                TempIndCoef =
                    ConvGains + state.dataDispVentMgr->HAT_MX + state.dataDispVentMgr->HAT_OC + state.dataDispVentMgr->HAT_FLOOR + MCpT_Total;
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    ZTAveraged = (TempHistTerm + ConvGains + state.dataDispVentMgr->HAT_MX + state.dataDispVentMgr->HAT_OC +
                                  state.dataDispVentMgr->HAT_FLOOR + MCpT_Total) /
                                 ((11.0 / 6.0) * AirCap + state.dataDispVentMgr->HA_MX + state.dataDispVentMgr->HA_OC +
                                  state.dataDispVentMgr->HA_FLOOR + MCp_Total);
                } break;
                case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                    if (TempDepCoef == 0.0) { // B=0
                        ZTAveraged = thisZoneT1 + TempIndCoef / AirCap;
                    } else {
                        ZTAveraged =
                            (thisZoneT1 - TempIndCoef / TempDepCoef) * std::exp(min(700.0, -TempDepCoef / AirCap)) + TempIndCoef / TempDepCoef;
                    }
                } break;
                case DataHeatBalance::SolutionAlgo::EulerMethod: {
                    ZTAveraged = (AirCap * thisZoneT1 + TempIndCoef) / (AirCap + TempDepCoef);
                } break;
                default:
                    break;
                }
                state.dataRoomAir->ZTOC(ZoneNum) = ZTAveraged;
                state.dataRoomAir->ZTMX(ZoneNum) = ZTAveraged;
                state.dataRoomAir->ZTFloor(ZoneNum) = ZTAveraged;
                HcDispVent3Node(state, ZoneNum, HeightFrac);
                TempDepCoef = state.dataDispVentMgr->HA_MX + state.dataDispVentMgr->HA_OC + state.dataDispVentMgr->HA_FLOOR + MCp_Total;
                TempIndCoef =
                    ConvGains + state.dataDispVentMgr->HAT_MX + state.dataDispVentMgr->HAT_OC + state.dataDispVentMgr->HAT_FLOOR + MCpT_Total;
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    ZTAveraged = (TempHistTerm + ConvGains + state.dataDispVentMgr->HAT_MX + state.dataDispVentMgr->HAT_OC +
                                  state.dataDispVentMgr->HAT_FLOOR + MCpT_Total) /
                                 ((11.0 / 6.0) * AirCap + state.dataDispVentMgr->HA_MX + state.dataDispVentMgr->HA_OC +
                                  state.dataDispVentMgr->HA_FLOOR + MCp_Total);
                } break;
                case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                    if (TempDepCoef == 0.0) { // B=0
                        ZTAveraged = thisZoneT1 + TempIndCoef / AirCap;
                    } else {
                        ZTAveraged =
                            (thisZoneT1 - TempIndCoef / TempDepCoef) * std::exp(min(700.0, -TempDepCoef / AirCap)) + TempIndCoef / TempDepCoef;
                    }
                } break;
                case DataHeatBalance::SolutionAlgo::EulerMethod: {
                    ZTAveraged = (AirCap * thisZoneT1 + TempIndCoef) / (AirCap + TempDepCoef);
                } break;
                default:
                    break;
                }
                state.dataRoomAir->ZTOC(ZoneNum) = ZTAveraged;
                state.dataRoomAir->ZTMX(ZoneNum) = ZTAveraged;
                state.dataRoomAir->ZTFloor(ZoneNum) = ZTAveraged;
            }
        }
        //=========================================================================================

        // Comfort temperature and temperature at the thermostat/temperature control sensor

        state.dataRoomAir->HeightTransition(ZoneNum) = HeightFrac * CeilingHeight;
        HeightMixedSubzoneAve = (CeilingHeight + state.dataRoomAir->HeightTransition(ZoneNum)) / 2.0;
        HeightOccupiedSubzoneAve = (state.dataDispVentMgr->HeightFloorSubzoneTop + state.dataRoomAir->HeightTransition(ZoneNum)) / 2.0;
        HeightFloorSubzoneAve = state.dataDispVentMgr->HeightFloorSubzoneTop / 2.0;

        // Comfort temperature

        if (MIXFLAG) {
            state.dataRoomAir->TCMF(ZoneNum) = ZTAveraged;
        } else {
            if (HeightComfort >= 0.0 && HeightComfort < HeightFloorSubzoneAve) {
                ShowWarningError(state, format("Displacement ventilation comfort height is in floor subzone in Zone: {}", zone.Name));
                state.dataRoomAir->TCMF(ZoneNum) = state.dataRoomAir->ZTFloor(ZoneNum);
            } else if (HeightComfort >= HeightFloorSubzoneAve && HeightComfort < HeightOccupiedSubzoneAve) {
                state.dataRoomAir->TCMF(ZoneNum) = (state.dataRoomAir->ZTFloor(ZoneNum) * (HeightOccupiedSubzoneAve - HeightComfort) +
                                                    state.dataRoomAir->ZTOC(ZoneNum) * (HeightComfort - HeightFloorSubzoneAve)) /
                                                   (HeightOccupiedSubzoneAve - HeightFloorSubzoneAve);
                //!      TCMF(ZoneNum) = (ZTFloor(ZoneNum) * (HeightOccupiedSubzoneAve - HeightComfort) &
                //!                    + ZTMX(ZoneNum) * (HeightComfort - HeightFloorSubzoneAve)) &
                //!                    / (HeightOccupiedSubzoneAve - HeightFloorSubzoneAve)
            } else if (HeightComfort >= HeightOccupiedSubzoneAve && HeightComfort < HeightMixedSubzoneAve) {
                state.dataRoomAir->TCMF(ZoneNum) = (state.dataRoomAir->ZTOC(ZoneNum) * (HeightMixedSubzoneAve - HeightComfort) +
                                                    state.dataRoomAir->ZTMX(ZoneNum) * (HeightComfort - HeightOccupiedSubzoneAve)) /
                                                   (HeightMixedSubzoneAve - HeightOccupiedSubzoneAve);
            } else if (HeightComfort >= HeightMixedSubzoneAve && HeightComfort <= CeilingHeight) {
                state.dataRoomAir->TCMF(ZoneNum) = state.dataRoomAir->ZTMX(ZoneNum);
            } else {
                ShowFatalError(state, format("Displacement ventilation comfort height is above ceiling or below floor in Zone: {}", zone.Name));
            }
        }

        // Temperature at the thermostat/temperature control sensor

        if (MIXFLAG) {
            state.dataHeatBalFanSys->TempTstatAir(ZoneNum) = ZTAveraged;
        } else {
            if (HeightThermostat >= 0.0 && HeightThermostat < HeightFloorSubzoneAve) {
                ShowWarningError(state, format("Displacement thermostat is in floor subzone in Zone: {}", zone.Name));
                state.dataHeatBalFanSys->TempTstatAir(ZoneNum) = state.dataRoomAir->ZTFloor(ZoneNum);
            } else if (HeightThermostat >= HeightFloorSubzoneAve && HeightThermostat < HeightOccupiedSubzoneAve) {
                state.dataHeatBalFanSys->TempTstatAir(ZoneNum) =
                    (state.dataRoomAir->ZTFloor(ZoneNum) * (HeightOccupiedSubzoneAve - HeightThermostat) +
                     state.dataRoomAir->ZTOC(ZoneNum) * (HeightThermostat - HeightFloorSubzoneAve)) /
                    (HeightOccupiedSubzoneAve - HeightFloorSubzoneAve);
                //!      TempTstatAir(ZoneNum) = (ZTFloor(ZoneNum) * (HeightOccupiedSubzoneAve - HeightThermostat) &
                //!                    + ZTMX(ZoneNum) * (HeightThermostat - HeightFloorSubzoneAve)) &
                //!                    / (HeightOccupiedSubzoneAve - HeightFloorSubzoneAve)
            } else if (HeightThermostat >= HeightOccupiedSubzoneAve && HeightThermostat < HeightMixedSubzoneAve) {
                state.dataHeatBalFanSys->TempTstatAir(ZoneNum) = (state.dataRoomAir->ZTOC(ZoneNum) * (HeightMixedSubzoneAve - HeightThermostat) +
                                                                  state.dataRoomAir->ZTMX(ZoneNum) * (HeightThermostat - HeightOccupiedSubzoneAve)) /
                                                                 (HeightMixedSubzoneAve - HeightOccupiedSubzoneAve);
            } else if (HeightThermostat >= HeightMixedSubzoneAve && HeightThermostat <= CeilingHeight) {
                state.dataHeatBalFanSys->TempTstatAir(ZoneNum) = state.dataRoomAir->ZTMX(ZoneNum);
            } else {
                ShowFatalError(state, format("Displacement ventilation thermostat height is above ceiling or below floor in Zone: {}", zone.Name));
            }
        }

        // Temperature gradients

        if ((HeightMixedSubzoneAve - HeightFloorSubzoneAve) > 0.1) {
            state.dataRoomAir->AvgTempGrad(ZoneNum) =
                (state.dataRoomAir->ZTMX(ZoneNum) - state.dataRoomAir->ZTFloor(ZoneNum)) / (HeightMixedSubzoneAve - HeightFloorSubzoneAve);
        } else {
            state.dataRoomAir->AvgTempGrad(ZoneNum) = -9.999;
        }
        if ((HeightOccupiedSubzoneAve - HeightFloorSubzoneAve) > 0.1) {
            state.dataRoomAir->MaxTempGrad(ZoneNum) =
                (state.dataRoomAir->ZTOC(ZoneNum) - state.dataRoomAir->ZTFloor(ZoneNum)) / (HeightOccupiedSubzoneAve - HeightFloorSubzoneAve);
        } else {
            state.dataRoomAir->MaxTempGrad(ZoneNum) = -9.999;
        }
        if ((HeightMixedSubzoneAve - HeightOccupiedSubzoneAve) > 0.1) {
            MTGAUX = (state.dataRoomAir->ZTMX(ZoneNum) - state.dataRoomAir->ZTOC(ZoneNum)) / (HeightMixedSubzoneAve - HeightOccupiedSubzoneAve);
        } else {
            MTGAUX = -9.999;
        }

        if (MTGAUX > state.dataRoomAir->MaxTempGrad(ZoneNum)) {
            state.dataRoomAir->MaxTempGrad(ZoneNum) = MTGAUX;
        }

        if (MIXFLAG) {
            state.dataRoomAir->ZoneDispVent3NodeMixedFlag(ZoneNum) = 1;
            state.dataRoomAir->AirModel(ZoneNum).SimAirModel = false;
        } else {
            state.dataRoomAir->ZoneDispVent3NodeMixedFlag(ZoneNum) = 0;
            state.dataRoomAir->AirModel(ZoneNum).SimAirModel = true;
        }

        if (state.dataZoneEquip->ZoneEquipConfig(ZoneNum).IsControlled) {
            int ZoneNodeNum = zone.SystemZoneNodeNumber;
            state.dataLoopNodes->Node(ZoneNodeNum).Temp = state.dataRoomAir->ZTMX(ZoneNum);
        }

        // Mixed for reporting purposes
        if ((MIXFLAG) || ((state.dataRoomAir->ZTMX(ZoneNum) - state.dataRoomAir->ZTOC(ZoneNum)) < TempDiffCritRep)) {
            state.dataRoomAir->ZoneDispVent3NodeMixedFlagRep(ZoneNum) = 1.0;
            state.dataRoomAir->FracMinFlow(ZoneNum) = -1.0;
            state.dataRoomAir->HeightTransition(ZoneNum) = -9.999;
            state.dataRoomAir->AvgTempGrad(ZoneNum) = -9.999;
            state.dataRoomAir->MaxTempGrad(ZoneNum) = -9.999;
        } else {
            state.dataRoomAir->ZoneDispVent3NodeMixedFlagRep(ZoneNum) = 0.0;
        }
    }

} // namespace RoomAir
} // namespace EnergyPlus
