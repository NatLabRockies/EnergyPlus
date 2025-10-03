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
#include <ObjexxFCL/member.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/UFADManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace RoomAir {

    // Module containing the routines dealing with the UnderFloor Air
    // Distribution zone model

    // MODULE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   August 2005

    // PURPOSE OF THIS MODULE:
    // Encapsulate the routines that do the simulation of the UCSD UFAD non-uniform
    // zone models

    // METHODOLOGY EMPLOYED:
    // 2-node zone model with the node heights varying as a function of internal loads
    // and supply air flow (and other factors)

    // REFERENCES:
    // See the EnergyPlus Engineering Reference and the PhD thesis of Anna Liu, UC San Diego

    // OTHER NOTES:
    // na

    // Using/Aliasing
    using namespace DataLoopNode;
    using namespace DataEnvironment;
    using namespace DataHeatBalance;
    using namespace DataHeatBalSurface;
    using namespace DataSurfaces;
    using Convect::CalcDetailedHcInForDVModel;

    void ManageUFAD(EnergyPlusData &state,
                    int const ZoneNum,               // index number for the specified zone
                    RoomAirModel const ZoneModelType // type of zone model; UCSDUFI = 6
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August, 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Manages the simulation of the 2-node nonuniform zone models for underfloor air
        // distribution systems (UFAD). Called from RoomAirManager, ManageAirModel

        // METHODOLOGY EMPLOYED:
        // uses Init and Calc routines in the standard EPlus manner to manage the calculation
        // Note that much of the initialization is done in RoomAirManager, SharedDVCVUFDataInit

        // Using/Aliasing
        using namespace DataLoopNode;
        using namespace DataEnvironment;
        using namespace DataHeatBalance;
        using namespace DataHeatBalSurface;
        using namespace DataSurfaces;
        using Convect::CalcDetailedHcInForDVModel;

        // input was obtained in RoomAirManager, GetUFADIntZoneData

        InitUFAD(state, ZoneNum, ZoneModelType); // initialize some module variables

        switch (ZoneModelType) {
        case RoomAirModel::UFADInt: { // UCSD UFAD interior zone model
            // simulate room airflow using the UCSDUFI model
            CalcUFADInt(state, ZoneNum);
        } break;
        case RoomAirModel::UFADExt: { // UCSD UFAD exterior zone model
            // simulate room airflow using the UCSDUFE model
            CalcUFADExt(state, ZoneNum);
        } break;
        default:
            break;
        }
    }

    void InitUFAD(EnergyPlusData &state,
                  int const ZoneNum,
                  RoomAirModel const ZoneModelType // type of zone model; UCSDUFI = 6
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2005

        // PURPOSE OF THIS SUBROUTINE:
        // initialize arrays & variables used by the UCSD UFAD zone models

        // METHODOLOGY EMPLOYED:
        // Note that much of the initialization is done in RoomAirManager, SharedDVCVUFDataInit

        Real64 NumShadesDown(0.0);

        // Do the one time initializations
        if (state.dataUFADManager->MyOneTimeFlag) {
            state.dataUFADManager->HeightFloorSubzoneTop = 0.2;
            state.dataUFADManager->ThickOccupiedSubzoneMin = 0.2;
            state.dataUFADManager->HeightIntMassDefault = 2.0;
            state.dataUFADManager->MyOneTimeFlag = false;
            state.dataUFADManager->MySizeFlag.dimension(state.dataGlobal->NumOfZones, true);
        }

        if (state.dataUFADManager->MySizeFlag(ZoneNum)) {
            SizeUFAD(state, ZoneNum, ZoneModelType);
            state.dataUFADManager->MySizeFlag(ZoneNum) = false;
        }

        // initialize these variables every timestep

        state.dataUFADManager->HeightIntMass = state.dataUFADManager->HeightIntMassDefault;
        state.dataRoomAir->ZoneUFADGamma(ZoneNum) = 0.0;
        state.dataRoomAir->ZoneUFADPowInPlumes(ZoneNum) = 0.0;
        NumShadesDown = 0.0;
        for (int Ctd = state.dataRoomAir->PosZ_Window(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Window(ZoneNum).end; ++Ctd) {
            int SurfNum = state.dataRoomAir->APos_Window(Ctd);
            if (SurfNum == 0) {
                continue;
            }
            auto const &surf = state.dataSurface->Surface(SurfNum);
            if (surf.ExtBoundCond == ExternalEnvironment || surf.ExtBoundCond == OtherSideCoefNoCalcExt ||
                surf.ExtBoundCond == OtherSideCoefCalcExt || surf.ExtBoundCond == OtherSideCondModeledExt) {
                if (ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum))) {
                    ++NumShadesDown;
                }
            }
        }
        if (ZoneModelType == RoomAirModel::UFADExt) {
            auto &zoneUE = state.dataRoomAir->ZoneUFAD(state.dataRoomAir->ZoneUFADPtr(ZoneNum));
            zoneUE.ShadeDown = (zoneUE.NumExtWin > 1.0) && (NumShadesDown / zoneUE.NumExtWin >= 0.5);
        }
    }

    void SizeUFAD(EnergyPlusData &state,
                  int const ZoneNum,
                  RoomAirModel const model // type of zone model; UCSDUFI = 6
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2005

        // PURPOSE OF THIS SUBROUTINE:
        // set some smart defaults for UFAD systems

        // METHODOLOGY EMPLOYED:
        // use data from Center for Built Environment

        using DataSizing::AutoSize;

        // This is for both UFADInt and UFADExt
        auto &zoneU = state.dataRoomAir->ZoneUFAD(state.dataRoomAir->ZoneUFADPtr(ZoneNum));

        std::string_view cCMO = (model == RoomAirModel::UFADExt) ? "RoomAirSettings:UnderFloorAirDistributionExterior"
                                                                 : "RoomAirSettings:UnderFloorAirDistributionInterior";

        Real64 NumberOfOccupants = 0.0;
        for (auto const &people : state.dataHeatBal->People) {
            if (people.ZonePtr == ZoneNum) {
                NumberOfOccupants += people.NumberOfPeople;
            }
        }

        if (model == RoomAirModel::UFADExt) {
            // calculate total window width in zone
            for (int Ctd = state.dataRoomAir->PosZ_Window(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Window(ZoneNum).end; ++Ctd) {
                int SurfNum = state.dataRoomAir->APos_Window(Ctd);
                if (SurfNum == 0) {
                    continue;
                }
                auto &surf = state.dataSurface->Surface(SurfNum);
                if (surf.ExtBoundCond == ExternalEnvironment || surf.ExtBoundCond == OtherSideCoefNoCalcExt ||
                    surf.ExtBoundCond == OtherSideCoefCalcExt || surf.ExtBoundCond == OtherSideCondModeledExt) {
                    zoneU.WinWidth += surf.Width;
                    ++zoneU.NumExtWin;
                }
            }
            if (zoneU.WinWidth <= 0.0) {
                ShowWarningError(
                    state,
                    format("For RoomAirSettings:UnderFloorAirDistributionExterior for Zone {} there are no exterior windows.", zoneU.ZoneName));
                ShowContinueError(state, "  The zone will be treated as a UFAD interior zone");
            }
        } // if (model == RoomAirModel::UFADExt)

        if (zoneU.DiffArea == AutoSize) {
            constexpr std::array<Real64, (int)Diffuser::Num> diffArea = {0.0075, 0.035, 0.0060, 0.03, 0.0075};
            zoneU.DiffArea = diffArea[(int)zoneU.DiffuserType];
            // 4 ft x 4 inches; 75 cfm per linear foot; area is .025 m2/m

            BaseSizer::reportSizerOutput(state, cCMO, zoneU.ZoneName, "Design effective area of diffuser", zoneU.DiffArea);
        }
        if (zoneU.DiffAngle == AutoSize) {
            constexpr std::array<Real64, (int)Diffuser::Num> diffAngle = {28.0, 45.0, 73.0, 15.0, 28.0};
            zoneU.DiffAngle = diffAngle[(int)zoneU.DiffuserType];

            BaseSizer::reportSizerOutput(state, cCMO, zoneU.ZoneName, "Angle between diffuser slots and the vertical", zoneU.DiffAngle);
        }
        if (zoneU.TransHeight == AutoSize) {
            zoneU.CalcTransHeight = true;
            zoneU.TransHeight = 0.0;
        } else {
            zoneU.CalcTransHeight = false;
        }

        if (zoneU.DiffuserType != Diffuser::Custom &&
            (zoneU.A_Kc != Constant::AutoCalculate || zoneU.B_Kc != Constant::AutoCalculate || zoneU.C_Kc != Constant::AutoCalculate ||
             zoneU.D_Kc != Constant::AutoCalculate || zoneU.E_Kc != Constant::AutoCalculate)) {
            ShowWarningError(state,
                             format("For {} for Zone {}, input for Coefficients A - E will be "
                                    "ignored when Floor Diffuser Type = {}.",
                                    cCMO,
                                    zoneU.ZoneName,
                                    diffuserNamesUC[(int)zoneU.DiffuserType]));
            ShowContinueError(state, "  To input these Coefficients, use Floor Diffuser Type = Custom.");
        }

        if (zoneU.DiffuserType == Diffuser::Swirl) {
            zoneU.A_Kc = 0.0;
            zoneU.B_Kc = 0.0;
            zoneU.C_Kc = 0.6531;
            zoneU.D_Kc = 0.0069;
            zoneU.E_Kc = -0.00004;
        } else if (zoneU.DiffuserType == Diffuser::VarArea) {
            zoneU.A_Kc = 0.0;
            zoneU.B_Kc = 0.0;
            zoneU.C_Kc = (model == RoomAirModel::UFADExt) ? 0.83 : 0.88;
            zoneU.D_Kc = 0.0;
            zoneU.E_Kc = 0.0;
        } else if (zoneU.DiffuserType == Diffuser::DisplVent) {
            zoneU.A_Kc = 0.0;
            zoneU.B_Kc = 0.0;
            zoneU.C_Kc = 0.67;
            zoneU.D_Kc = 0.0;
            zoneU.E_Kc = 0.0;
        } else if (zoneU.DiffuserType == Diffuser::LinBarGrille) {
            zoneU.A_Kc = 0.0;
            zoneU.B_Kc = 0.0;
            zoneU.C_Kc = (model == RoomAirModel::UFADExt) ? 0.8214 : 0.8;
            zoneU.D_Kc = (model == RoomAirModel::UFADExt) ? -0.0263 : 0.0;
            zoneU.E_Kc = (model == RoomAirModel::UFADExt) ? 0.0014 : 0.0;
        } else if (zoneU.A_Kc == Constant::AutoCalculate || zoneU.B_Kc == Constant::AutoCalculate || zoneU.C_Kc == Constant::AutoCalculate ||
                   zoneU.D_Kc == Constant::AutoCalculate || zoneU.E_Kc == Constant::AutoCalculate) {
            ShowFatalError(state,
                           format("For {} for Zone {}, input for Coefficients A - E must be "
                                  "specified when Floor Diffuser Type = Custom.",
                                  cCMO,
                                  zoneU.ZoneName));
        }

        if (zoneU.PowerPerPlume == Constant::AutoCalculate) {

            zoneU.PowerPerPlume = sumUFADConvGainPerPlume(state, ZoneNum, NumberOfOccupants);

            BaseSizer::reportSizerOutput(state, cCMO, zoneU.ZoneName, "Power per plume [W]", zoneU.PowerPerPlume);

            if (zoneU.DiffusersPerZone == AutoSize) {
                zoneU.DiffusersPerZone = (NumberOfOccupants > 0.0) ? NumberOfOccupants : 1.0;
                BaseSizer::reportSizerOutput(state, cCMO, zoneU.ZoneName, "Number of diffusers per zone", zoneU.DiffusersPerZone);
            }
        }

        if (zoneU.DiffusersPerZone == AutoSize) {
            zoneU.DiffusersPerZone = (NumberOfOccupants > 0.0) ? NumberOfOccupants : 1.0;

            BaseSizer::reportSizerOutput(state, cCMO, zoneU.ZoneName, "Number of diffusers per zone", zoneU.DiffusersPerZone);
        }
    }

    Real64 sumUFADConvGainPerPlume(EnergyPlusData const &state, int const zoneNum, Real64 const numOccupants)
    {
        Real64 zoneElecConv(0.0); // zone elec equip design convective gain [W]
        for (auto const &zoneElectric : state.dataHeatBal->ZoneElectric) {
            if (zoneElectric.ZonePtr == zoneNum) {
                zoneElecConv += zoneElectric.DesignLevel * zoneElectric.FractionConvected;
            }
        }

        Real64 zoneGasConv(0.0); // zone gas equip design convective gain [W]
        for (auto const &zoneGas : state.dataHeatBal->ZoneGas) {
            if (zoneGas.ZonePtr == zoneNum) {
                zoneGasConv += zoneGas.DesignLevel * zoneGas.FractionConvected;
            }
        }

        Real64 zoneOthEqConv(0.0); // zone other equip design convective gain [W]
        for (auto const &zoneOtherEq : state.dataHeatBal->ZoneOtherEq) {
            if (zoneOtherEq.ZonePtr == zoneNum) {
                zoneOthEqConv += zoneOtherEq.DesignLevel * zoneOtherEq.FractionConvected;
            }
        }

        Real64 zoneHWEqConv(0.0); // zone hot water equip design convective gain [W]
        for (auto const &zoneHWEq : state.dataHeatBal->ZoneHWEq) {
            if (zoneHWEq.ZonePtr == zoneNum) {
                zoneHWEqConv += zoneHWEq.DesignLevel * zoneHWEq.FractionConvected;
            }
        }

        Real64 zoneSteamEqConv(0.0); // zone steam equip design convective gain [W]
        for (auto const &zoneSteamEq : state.dataHeatBal->ZoneSteamEq) {
            if (zoneSteamEq.ZonePtr == zoneNum) {
                zoneSteamEqConv += zoneSteamEq.DesignLevel * zoneSteamEq.FractionConvected;
            }
        }

        Real64 numPlumes = (numOccupants > 0.0) ? numOccupants : 1.0;

        return (numOccupants * 73.0 + zoneElecConv + zoneGasConv + zoneOthEqConv + zoneHWEqConv + zoneSteamEqConv) / numPlumes;
    }

    void HcUFAD(EnergyPlusData &state, int const ZoneNum, Real64 const FractionHeight, UFADConvCoef &ufadCC)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         G. Carrilho da Graca
        //       DATE WRITTEN   February 2004

        // PURPOSE OF THIS SUBROUTINE:
        // Main subroutine for convection calculation in the UCSD Displacement Ventilation model.
        // It calls CalcDetailedHcInForDVModel for convection coefficient
        // initial calculations and averages the final result comparing the position of the surface with
        // the interface subzone height.

        // Using/Aliasing
        using namespace DataEnvironment;
        using namespace DataHeatBalance;

        // Is the air flow model for this zone set to UCSDDV Displacement Ventilation?
        if (!state.dataRoomAir->IsZoneUFAD(ZoneNum)) {
            return;
        }

        ufadCC.HAT_MX = 0.0;    // HAT_MX Convection Coefficient times Area times Temperature for the upper subzone
        ufadCC.HAT_MXWin = 0.0; // HAT_MX Convection Coefficient times Area times Temperature for the upper subzone (windows only)
        ufadCC.HA_MX = 0.0;     // HA_MX Convection Coefficient times Area for the upper subzone
        ufadCC.HA_MXWin = 0.0;  // HA_MX Convection Coefficient times Area for the upper subzone (windows only)
        ufadCC.HAT_OC = 0.0;    // HAT_OC Convection Coefficient times Area times Temperature for the lower subzone
        ufadCC.HAT_OCWin = 0.0; // HAT_OC Convection Coefficient times Area times Temperature for the lower subzone (windows only)
        ufadCC.HA_OC = 0.0;     // HA_OC Convection Coefficient times Area for the lower subzone
        ufadCC.HA_OCWin = 0.0;  // HA_OC Convection Coefficient times Area for the lower subzone (windows only)
        ufadCC.HAT_FLOOR = 0.0; // HAT_FLOOR Convection Coefficient times Area times Temperature for the floor(?) subzone
        ufadCC.HA_FLOOR = 0.0;  // HA_FLOOR Convection Coefficient times Area for the floor(?) subzone

        Real64 zoneCeilingHeight1 = state.dataRoomAir->ZoneCeilingHeight1(ZoneNum);
        Real64 zoneCeilingHeight2 = state.dataRoomAir->ZoneCeilingHeight2(ZoneNum);

        Real64 LayH = FractionHeight * (zoneCeilingHeight2 - zoneCeilingHeight1); // Height of the Occupied/Mixed subzone interface

        // WALL Hc, HA and HAT calculation
        for (int Ctd = state.dataRoomAir->PosZ_Wall(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Wall(ZoneNum).end; ++Ctd) {
            int SurfNum = state.dataRoomAir->APos_Wall(Ctd);
            if (SurfNum == 0) {
                continue;
            }

            auto &surf = state.dataSurface->Surface(SurfNum);
            state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
            state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];
            Real64 ZSupSurf = maxval(surf.Vertex, &Vector::z) - zoneCeilingHeight1; // highest height for this surface
            Real64 ZInfSurf = minval(surf.Vertex, &Vector::z) - zoneCeilingHeight1; // lowest height for this surface

            // The Wall surface is in the upper subzone
            if (ZInfSurf > LayH) {
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                state.dataRoomAir->HWall(Ctd) = state.dataRoomAir->UFADHcIn(SurfNum);
                ufadCC.HAT_MX += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HWall(Ctd);
                ufadCC.HA_MX += surf.Area * state.dataRoomAir->HWall(Ctd);
            }

            // The Wall surface is in the lower subzone
            if (ZSupSurf < LayH) {
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                state.dataRoomAir->HWall(Ctd) = state.dataRoomAir->UFADHcIn(SurfNum);
                ufadCC.HAT_OC += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HWall(Ctd);
                ufadCC.HA_OC += surf.Area * state.dataRoomAir->HWall(Ctd);
            }

            if (std::abs(ZInfSurf - ZSupSurf) < 1.e-10) {
                ShowSevereError(state, "RoomAirModelUFAD:HcUCSDUF: Surface values will cause divide by zero.");
                ShowContinueError(state, format("Zone=\"{}\", Surface=\"{}\".", state.dataHeatBal->Zone(surf.Zone).Name, surf.Name));
                ShowContinueError(state, format("ZInfSurf=[{:.4R}], LayH=[{:.4R}].", ZInfSurf, LayH));
                ShowContinueError(state, format("ZSupSurf=[{:.4R}], LayH=[{:.4R}].", ZSupSurf, LayH));
                ShowFatalError(state, "...Previous condition causes termination.");
            }

            // The Wall surface is partially in upper and partially in lower subzone
            if (ZInfSurf <= LayH && ZSupSurf >= LayH) {
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                Real64 HLU = state.dataRoomAir->UFADHcIn(SurfNum); // Convection coefficient for the upper area of surface
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                Real64 HLD = state.dataRoomAir->UFADHcIn(SurfNum); // Convection coefficient for the lower area of surface
                Real64 TmedDV = ((ZSupSurf - LayH) * state.dataRoomAir->ZTMX(ZoneNum) + (LayH - ZInfSurf) * state.dataRoomAir->ZTOC(ZoneNum)) /
                                (ZSupSurf - ZInfSurf); // Average temperature for DV
                state.dataRoomAir->HWall(Ctd) = ((LayH - ZInfSurf) * HLD + (ZSupSurf - LayH) * HLU) / (ZSupSurf - ZInfSurf);
                ufadCC.HAT_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * state.dataHeatBalSurf->SurfTempIn(SurfNum) * HLU;
                ufadCC.HA_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * HLU;
                ufadCC.HAT_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * state.dataHeatBalSurf->SurfTempIn(SurfNum) * HLD;
                ufadCC.HA_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * HLD;
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = TmedDV;
            }

            state.dataRoomAir->UFADHcIn(SurfNum) = state.dataRoomAir->HWall(Ctd);

        } // END WALL

        // WINDOW Hc, HA and HAT CALCULATION
        for (int Ctd = state.dataRoomAir->PosZ_Window(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Window(ZoneNum).end; ++Ctd) {
            int SurfNum = state.dataRoomAir->APos_Window(Ctd);
            if (SurfNum == 0) {
                continue;
            }

            auto &surf = state.dataSurface->Surface(SurfNum);
            state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
            state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];

            if (surf.Tilt > 10.0 && surf.Tilt < 170.0) { // Window Wall
                Real64 ZSupSurf = maxval(surf.Vertex, &Vector::z) - zoneCeilingHeight1;
                Real64 ZInfSurf = minval(surf.Vertex, &Vector::z) - zoneCeilingHeight1;

                if (ZInfSurf > LayH) {
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                    state.dataRoomAir->HWindow(Ctd) = state.dataRoomAir->UFADHcIn(SurfNum);
                    ufadCC.HAT_MX += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HWindow(Ctd);
                    ufadCC.HA_MX += surf.Area * state.dataRoomAir->HWindow(Ctd);
                    ufadCC.HAT_MXWin += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HWindow(Ctd);
                    ufadCC.HA_MXWin += surf.Area * state.dataRoomAir->HWindow(Ctd);
                }

                if (ZSupSurf < LayH) {
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                    state.dataRoomAir->HWindow(Ctd) = state.dataRoomAir->UFADHcIn(SurfNum);
                    ufadCC.HAT_OC += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HWindow(Ctd);
                    ufadCC.HA_OC += surf.Area * state.dataRoomAir->HWindow(Ctd);
                    ufadCC.HAT_OCWin += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HWindow(Ctd);
                    ufadCC.HA_OCWin += surf.Area * state.dataRoomAir->HWindow(Ctd);
                }

                if (ZInfSurf <= LayH && ZSupSurf >= LayH) {
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                    Real64 HLU = state.dataRoomAir->UFADHcIn(SurfNum); // Convection coefficient for the upper area of surface
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                    CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                    Real64 HLD = state.dataRoomAir->UFADHcIn(SurfNum); // Convection coefficient for the lower area of surface
                    Real64 TmedDV = ((ZSupSurf - LayH) * state.dataRoomAir->ZTMX(ZoneNum) + (LayH - ZInfSurf) * state.dataRoomAir->ZTOC(ZoneNum)) /
                                    (ZSupSurf - ZInfSurf); // Average temperature

                    state.dataRoomAir->HWindow(Ctd) = ((LayH - ZInfSurf) * HLD + (ZSupSurf - LayH) * HLU) / (ZSupSurf - ZInfSurf);
                    ufadCC.HAT_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * state.dataHeatBalSurf->SurfTempIn(SurfNum) * HLU;
                    ufadCC.HA_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * HLU;
                    ufadCC.HAT_MXWin += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * state.dataHeatBalSurf->SurfTempIn(SurfNum) * HLU;
                    ufadCC.HA_MXWin += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * HLU;
                    ufadCC.HAT_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * state.dataHeatBalSurf->SurfTempIn(SurfNum) * HLD;
                    ufadCC.HA_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * HLD;
                    ufadCC.HAT_OCWin += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * state.dataHeatBalSurf->SurfTempIn(SurfNum) * HLD;
                    ufadCC.HA_OCWin += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * HLD;
                    state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = TmedDV;
                }
            }

            if (surf.Tilt <= 10.0) { // Window Ceiling
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                state.dataRoomAir->HWindow(Ctd) = state.dataRoomAir->UFADHcIn(SurfNum);
                ufadCC.HAT_MX += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HWindow(Ctd);
                ufadCC.HA_MX += surf.Area * state.dataRoomAir->HWindow(Ctd);
            } else if (surf.Tilt >= 170.0) { // Window Floor
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                state.dataRoomAir->HWindow(Ctd) = state.dataRoomAir->UFADHcIn(SurfNum);
                ufadCC.HAT_OC += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HWindow(Ctd);
                ufadCC.HA_OC += surf.Area * state.dataRoomAir->HWindow(Ctd);
            }

            state.dataRoomAir->UFADHcIn(SurfNum) = state.dataRoomAir->HWindow(Ctd);

        } // END WINDOW

        // DOOR Hc, HA and HAT CALCULATION
        for (int Ctd = state.dataRoomAir->PosZ_Door(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Door(ZoneNum).end; ++Ctd) { // DOOR
            int SurfNum = state.dataRoomAir->APos_Door(Ctd);
            if (SurfNum == 0) {
                continue;
            }
            auto &surf = state.dataSurface->Surface(SurfNum);
            state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
            state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];

            Real64 ZSupSurf = maxval(surf.Vertex, &Vector::z) - zoneCeilingHeight1;
            Real64 ZInfSurf = minval(surf.Vertex, &Vector::z) - zoneCeilingHeight1;

            if (ZInfSurf > LayH) {
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                state.dataRoomAir->HDoor(Ctd) = state.dataRoomAir->UFADHcIn(SurfNum);
                ufadCC.HAT_MX += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HDoor(Ctd);
                ufadCC.HA_MX += surf.Area * state.dataRoomAir->HDoor(Ctd);
            }

            if (ZSupSurf < LayH) {
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                state.dataRoomAir->HDoor(Ctd) = state.dataRoomAir->UFADHcIn(SurfNum);
                ufadCC.HAT_OC += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HDoor(Ctd);
                ufadCC.HA_OC += surf.Area * state.dataRoomAir->HDoor(Ctd);
            }

            if (ZInfSurf <= LayH && ZSupSurf >= LayH) {
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                Real64 HLU = state.dataRoomAir->UFADHcIn(SurfNum); // Convection coefficient for the upper area of surface
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                Real64 HLD = state.dataRoomAir->UFADHcIn(SurfNum); // Convection coefficient for the lower area of surface
                Real64 TmedDV = ((ZSupSurf - LayH) * state.dataRoomAir->ZTMX(ZoneNum) + (LayH - ZInfSurf) * state.dataRoomAir->ZTOC(ZoneNum)) /
                                (ZSupSurf - ZInfSurf); // Average temperature
                state.dataRoomAir->HDoor(Ctd) = ((LayH - ZInfSurf) * HLD + (ZSupSurf - LayH) * HLU) / (ZSupSurf - ZInfSurf);
                ufadCC.HAT_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * state.dataHeatBalSurf->SurfTempIn(SurfNum) * HLU;
                ufadCC.HA_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * HLU;
                ufadCC.HAT_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * state.dataHeatBalSurf->SurfTempIn(SurfNum) * HLD;
                ufadCC.HA_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * HLD;
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = TmedDV;
            }

            state.dataRoomAir->UFADHcIn(SurfNum) = state.dataRoomAir->HDoor(Ctd);

        } // END DOOR

        // INTERNAL Hc, HA and HAT CALCULATION
        state.dataUFADManager->HeightIntMass = min(state.dataUFADManager->HeightIntMassDefault, (zoneCeilingHeight2 - zoneCeilingHeight1));
        for (int Ctd = state.dataRoomAir->PosZ_Internal(ZoneNum).beg; Ctd <= state.dataRoomAir->PosZ_Internal(ZoneNum).end; ++Ctd) {
            int SurfNum = state.dataRoomAir->APos_Internal(Ctd);
            if (SurfNum == 0) {
                continue;
            }

            auto const &surf = state.dataSurface->Surface(SurfNum);
            state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
            state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];
            Real64 ZSupSurf = state.dataUFADManager->HeightIntMass;
            Real64 ZInfSurf = 0.0;

            if (ZSupSurf < LayH) {
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                state.dataRoomAir->HInternal(Ctd) = state.dataRoomAir->UFADHcIn(SurfNum);
                ufadCC.HAT_OC += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HInternal(Ctd);
                ufadCC.HA_OC += surf.Area * state.dataRoomAir->HInternal(Ctd);
            }

            if (ZInfSurf <= LayH && ZSupSurf >= LayH) {
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTMX(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                Real64 HLU = state.dataRoomAir->UFADHcIn(SurfNum); // Convection coefficient for the upper area of surface
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTOC(ZoneNum);
                CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
                Real64 HLD = state.dataRoomAir->UFADHcIn(SurfNum); // Convection coefficient for the lower area of surface
                Real64 TmedDV = ((ZSupSurf - LayH) * state.dataRoomAir->ZTMX(ZoneNum) + (LayH - ZInfSurf) * state.dataRoomAir->ZTOC(ZoneNum)) /
                                (ZSupSurf - ZInfSurf); // Average temperature
                state.dataRoomAir->HInternal(Ctd) = ((LayH - ZInfSurf) * HLD + (ZSupSurf - LayH) * HLU) / (ZSupSurf - ZInfSurf);
                ufadCC.HAT_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * state.dataHeatBalSurf->SurfTempIn(SurfNum) * HLU;
                ufadCC.HA_MX += surf.Area * (ZSupSurf - LayH) / (ZSupSurf - ZInfSurf) * HLU;
                ufadCC.HAT_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * state.dataHeatBalSurf->SurfTempIn(SurfNum) * HLD;
                ufadCC.HA_OC += surf.Area * (LayH - ZInfSurf) / (ZSupSurf - ZInfSurf) * HLD;
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = TmedDV;
            }

            state.dataRoomAir->UFADHcIn(SurfNum) = state.dataRoomAir->HInternal(Ctd);
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
            CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
            state.dataRoomAir->HCeiling(Ctd) = state.dataRoomAir->UFADHcIn(SurfNum);
            ufadCC.HAT_MX += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HCeiling(Ctd);
            ufadCC.HA_MX += surf.Area * state.dataRoomAir->HCeiling(Ctd);
            state.dataRoomAir->UFADHcIn(SurfNum) = state.dataRoomAir->HCeiling(Ctd);
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
            CalcDetailedHcInForDVModel(state, SurfNum, state.dataHeatBalSurf->SurfTempIn, state.dataRoomAir->UFADHcIn);
            state.dataRoomAir->HFloor(Ctd) = state.dataRoomAir->UFADHcIn(SurfNum);
            ufadCC.HAT_OC += surf.Area * state.dataHeatBalSurf->SurfTempIn(SurfNum) * state.dataRoomAir->HFloor(Ctd);
            ufadCC.HA_OC += surf.Area * state.dataRoomAir->HFloor(Ctd);
            state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataRoomAir->ZTFloor(ZoneNum);
            state.dataRoomAir->UFADHcIn(SurfNum) = state.dataRoomAir->HFloor(Ctd);
        } // END FLOOR
    }

    static constexpr std::array<DataHeatBalance::IntGainType, 51> IntGainTypesOccupied = {
        DataHeatBalance::IntGainType::People,
        DataHeatBalance::IntGainType::WaterHeaterMixed,
        DataHeatBalance::IntGainType::WaterHeaterStratified,
        DataHeatBalance::IntGainType::ThermalStorageChilledWaterMixed,
        DataHeatBalance::IntGainType::ThermalStorageChilledWaterStratified,
        DataHeatBalance::IntGainType::ElectricEquipment,
        DataHeatBalance::IntGainType::ElectricEquipmentITEAirCooled,
        DataHeatBalance::IntGainType::GasEquipment,
        DataHeatBalance::IntGainType::HotWaterEquipment,
        DataHeatBalance::IntGainType::SteamEquipment,
        DataHeatBalance::IntGainType::OtherEquipment,
        DataHeatBalance::IntGainType::IndoorGreen,
        DataHeatBalance::IntGainType::ZoneBaseboardOutdoorTemperatureControlled,
        DataHeatBalance::IntGainType::GeneratorFuelCell,
        DataHeatBalance::IntGainType::WaterUseEquipment,
        DataHeatBalance::IntGainType::GeneratorMicroCHP,
        DataHeatBalance::IntGainType::ElectricLoadCenterTransformer,
        DataHeatBalance::IntGainType::ElectricLoadCenterInverterSimple,
        DataHeatBalance::IntGainType::ElectricLoadCenterInverterFunctionOfPower,
        DataHeatBalance::IntGainType::ElectricLoadCenterInverterLookUpTable,
        DataHeatBalance::IntGainType::ElectricLoadCenterStorageBattery,
        DataHeatBalance::IntGainType::ElectricLoadCenterStorageLiIonNmcBattery,
        DataHeatBalance::IntGainType::ElectricLoadCenterStorageSimple,
        DataHeatBalance::IntGainType::PipeIndoor,
        DataHeatBalance::IntGainType::RefrigerationCase,
        DataHeatBalance::IntGainType::RefrigerationCompressorRack,
        DataHeatBalance::IntGainType::RefrigerationSystemAirCooledCondenser,
        DataHeatBalance::IntGainType::RefrigerationSystemSuctionPipe,
        DataHeatBalance::IntGainType::RefrigerationSecondaryReceiver,
        DataHeatBalance::IntGainType::RefrigerationSecondaryPipe,
        DataHeatBalance::IntGainType::RefrigerationWalkIn,
        DataHeatBalance::IntGainType::RefrigerationTransSysAirCooledGasCooler,
        DataHeatBalance::IntGainType::RefrigerationTransSysSuctionPipeMT,
        DataHeatBalance::IntGainType::RefrigerationTransSysSuctionPipeLT,
        DataHeatBalance::IntGainType::Pump_VarSpeed,
        DataHeatBalance::IntGainType::Pump_ConSpeed,
        DataHeatBalance::IntGainType::Pump_Cond,
        DataHeatBalance::IntGainType::PumpBank_VarSpeed,
        DataHeatBalance::IntGainType::PumpBank_ConSpeed,
        DataHeatBalance::IntGainType::PlantComponentUserDefined,
        DataHeatBalance::IntGainType::CoilUserDefined,
        DataHeatBalance::IntGainType::ZoneHVACForcedAirUserDefined,
        DataHeatBalance::IntGainType::AirTerminalUserDefined,
        DataHeatBalance::IntGainType::PackagedTESCoilTank,
        DataHeatBalance::IntGainType::SecCoolingDXCoilSingleSpeed,
        DataHeatBalance::IntGainType::SecHeatingDXCoilSingleSpeed,
        DataHeatBalance::IntGainType::SecCoolingDXCoilTwoSpeed,
        DataHeatBalance::IntGainType::SecCoolingDXCoilMultiSpeed,
        DataHeatBalance::IntGainType::SecHeatingDXCoilMultiSpeed,
        DataHeatBalance::IntGainType::ElectricLoadCenterConverter,
        DataHeatBalance::IntGainType::FanSystemModel};

    static constexpr std::array<DataHeatBalance::IntGainType, 2> IntGainTypesUpSubzone = {DataHeatBalance::IntGainType::DaylightingDeviceTubular,
                                                                                          DataHeatBalance::IntGainType::Lights};

    // Explicitly list internal gains not applicable for UFAD
    // Explicitly list internal gains not applicable for Displacement Vent
    [[maybe_unused]] static constexpr std::array<DataHeatBalance::IntGainType, 2> ExcludedIntGainTypes = {
        DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkCarbonDioxide,
        DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkGenericContam};

    void CalcUFADInt(EnergyPlusData &state, int const ZoneNum) // index number for the specified zone
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2005
        //       MODIFIED       Brent Griffith June 2008 for new interpolation and time history
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Using the UCSD UFAD interior zone model, this subroutine calculates the  occupied subzone height,
        // surface heat transfer coefficients, the occupied subzone temperature, and the upper subzone temperature.

        // METHODOLOGY EMPLOYED:
        // The zone is divided into 2 subzones with a variable transition height.

        // REFERENCES:
        // The model is described in the EnergyPlus Engineering Reference in Anna Liu's UCSD PhD thesis.

        // Using/Aliasing
        using Psychrometrics::PsyCpAirFnW;
        using Psychrometrics::PsyRhoAirFnPbTdbW;
        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        Real64 HeightFrac; // Fractional height of transition between occupied and upper subzones
        Real64 Gamma;      // dimensionless height parameter; higher gamma means interface height will be
        // higher, smaller gamma means interface height will be lower.
        Real64 ZTAveraged;

        // Exact solution or Euler method
        if (state.dataHeatBal->ZoneAirSolutionAlgo != DataHeatBalance::SolutionAlgo::ThirdOrder) {
            if (state.dataHVACGlobal->ShortenTimeStepSysRoomAir && TimeStepSys < state.dataGlobal->TimeStepZone) {
                if (state.dataHVACGlobal->PreviousTimeStep < state.dataGlobal->TimeStepZone) {
                    state.dataRoomAir->Zone1OC(ZoneNum) = state.dataRoomAir->ZoneM2OC(ZoneNum);
                    state.dataRoomAir->Zone1MX(ZoneNum) = state.dataRoomAir->ZoneM2MX(ZoneNum);
                } else {
                    state.dataRoomAir->Zone1OC(ZoneNum) = state.dataRoomAir->ZoneMXOC(ZoneNum);
                    state.dataRoomAir->Zone1MX(ZoneNum) = state.dataRoomAir->ZoneMXMX(ZoneNum);
                }
            } else {
                state.dataRoomAir->Zone1OC(ZoneNum) = state.dataRoomAir->ZTOC(ZoneNum);
                state.dataRoomAir->Zone1MX(ZoneNum) = state.dataRoomAir->ZTMX(ZoneNum);
            }
        }

        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
        bool MIXFLAG = false;
        state.dataRoomAir->UFADHcIn = state.dataHeatBalSurf->SurfHConvInt;
        Real64 SumSysMCp = 0.0;  // Sum of system mass flow rate * specific heat for this zone [W/K]
        Real64 SumSysMCpT = 0.0; // Sum of system mass flow rate * specific heat * temperature for this zone [W]
        Real64 TSupK = 0.0;      // supply temperature [K]
        Real64 SumSysM = 0.0;    // Sum of systems mass flow rate [kg/s]
        Real64 TotSysFlow = 0.0; // [m3/s]
        int ZoneMult = state.dataHeatBal->Zone(ZoneNum).Multiplier * state.dataHeatBal->Zone(ZoneNum).ListMultiplier;
        Real64 CeilingHeight = state.dataRoomAir->ZoneCeilingHeight2(ZoneNum) - state.dataRoomAir->ZoneCeilingHeight1(ZoneNum);

        auto &zoneU = state.dataRoomAir->ZoneUFAD(state.dataRoomAir->ZoneUFADPtr(ZoneNum));
        Real64 HeightThermostat = zoneU.ThermostatHeight;         // height of the thermostat above the floor [m]
        Real64 HeightComfort = zoneU.ComfortHeight;               // height at which comfort temperature is calculated
        Real64 TempDiffCritRep = zoneU.TempTrigger;               // Minimum temperature difference between upper and occupied subzones for reporting
        Real64 DiffArea = zoneU.DiffArea;                         // diffuser effective area [m2]
        Real64 ThrowAngle = Constant::DegToRad * zoneU.DiffAngle; // diffuser slot angle relative to vertical [radians]
        Real64 SourceHeight = 0.0;                                // height of plume sources above the floor [m]
        Real64 NumDiffusers = zoneU.DiffusersPerZone;
        Real64 PowerPerPlume = zoneU.PowerPerPlume;
        // gains from occupants, task lighting, elec equip, gas equip, other equip, hot water equip, steam equip,
        // baseboards (nonthermostatic), water heater skin loss
        Real64 ConvGainsOccSubzone = InternalHeatGains::SumInternalConvectionGainsByTypes(state, ZoneNum, IntGainTypesOccupied);

        // Add heat to return air if zonal system (no return air) or cycling system (return air frequently very
        // low or zero)
        if (state.dataHeatBal->Zone(ZoneNum).NoHeatToReturnAir) {
            ConvGainsOccSubzone += InternalHeatGains::SumReturnAirConvectionGainsByTypes(state, ZoneNum, IntGainTypesOccupied);
        }

        // Add convection from pool cover to occupied region
        ConvGainsOccSubzone += state.dataHeatBalFanSys->SumConvPool(ZoneNum);

        // gains from lights (ceiling), tubular daylighting devices, high temp radiant heaters

        Real64 ConvGainsUpSubzone = InternalHeatGains::SumInternalConvectionGainsByTypes(state, ZoneNum, IntGainTypesUpSubzone);
        ConvGainsUpSubzone += state.dataHeatBalFanSys->SumConvHTRadSys(ZoneNum);
        if (state.dataHeatBal->Zone(ZoneNum).NoHeatToReturnAir) {
            ConvGainsUpSubzone += InternalHeatGains::SumReturnAirConvectionGainsByTypes(state, ZoneNum, IntGainTypesUpSubzone);
        }

        // Make sure all types of internal gains have been gathered
        assert((int)(size(IntGainTypesOccupied) + size(IntGainTypesUpSubzone) + size(ExcludedIntGainTypes)) ==
               (int)DataHeatBalance::IntGainType::Num);

        Real64 ConvGains = ConvGainsOccSubzone + ConvGainsUpSubzone + thisZoneHB.SysDepZoneLoadsLagged;
        Real64 ZoneEquipConfigNum = zoneU.ZoneEquipPtr;
        if (ZoneEquipConfigNum > 0) {
            auto const &zoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(ZoneEquipConfigNum);
            for (int InNodeIndex = 1; InNodeIndex <= zoneEquipConfig.NumInletNodes; ++InNodeIndex) {
                Real64 NodeTemp = state.dataLoopNodes->Node(zoneEquipConfig.InletNode(InNodeIndex)).Temp;
                Real64 MassFlowRate = state.dataLoopNodes->Node(zoneEquipConfig.InletNode(InNodeIndex)).MassFlowRate;
                Real64 CpAir = PsyCpAirFnW(thisZoneHB.airHumRat);
                SumSysMCp += MassFlowRate * CpAir;
                SumSysMCpT += MassFlowRate * CpAir * NodeTemp;
                TotSysFlow += MassFlowRate / PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, NodeTemp, thisZoneHB.airHumRat);
                TSupK += MassFlowRate * NodeTemp;
                SumSysM += MassFlowRate;
            }
            if (TotSysFlow > 0.0) {
                TSupK = TSupK / SumSysM + Constant::Kelvin;
            } else {
                TSupK = 0.0;
            }
        }
        // mass flow rate * specific heat for this zone for infiltration, ventilation, mixing [W/K]
        Real64 SumMCp = thisZoneHB.MCPI + thisZoneHB.MCPV + thisZoneHB.MCPM + thisZoneHB.MCPE + thisZoneHB.MCPC + thisZoneHB.MDotCPOA;
        // mass flow rate * specific heat* temp for this zone for infiltration, ventilation, mixing [W]
        Real64 SumMCpT = thisZoneHB.MCPTI + thisZoneHB.MCPTV + thisZoneHB.MCPTM + thisZoneHB.MCPTE + thisZoneHB.MCPTC +
                         thisZoneHB.MDotCPOA * state.dataHeatBal->Zone(ZoneNum).OutDryBulbTemp;
        Real64 MCp_Total = SumMCp + SumSysMCp;    // total mass flow rate * specific heat for this zone [W/K]
        Real64 MCpT_Total = SumMCpT + SumSysMCpT; // total mass flow rate * specific heat* temp for this zone [W]
        // For the York MIT diffusers (variable area) the area varies with the flow rate. Assume 400 ft/min velocity
        // at the diffuser, and a design flow rate of 150 cfm (.0708 m3/s). Then the design area for each diffuser is
        // 150 ft3/min / 400 ft/min = .375 ft2 = .035 m2. This is adjusted each time step by
        // (TotSysFlow/(NumDiffusers*.0708))*.035
        if (zoneU.DiffuserType == Diffuser::VarArea) {
            DiffArea = 0.035 * TotSysFlow / (0.0708 * NumDiffusers);
        }
        // initial estimate of convective transfer from surfaces; assume HeightFrac is 0.5.
        UFADConvCoef ufadCC;
        HcUFAD(state, ZoneNum, 0.5, ufadCC);
        Real64 PowerInPlumes = ConvGains + ufadCC.HAT_OC - ufadCC.HA_OC * state.dataRoomAir->ZTOC(ZoneNum) + ufadCC.HAT_MX -
                               ufadCC.HA_MX * state.dataRoomAir->ZTMX(ZoneNum);

        Real64 NumberOfPlumes = (PowerPerPlume > 0.0 && PowerInPlumes > 0.0) ? (PowerInPlumes / PowerPerPlume) : 1.0;
        Real64 NumDiffusersPerPlume = (PowerPerPlume > 0.0 && PowerInPlumes > 0.0) ? (NumDiffusers / NumberOfPlumes) : 1.0;

        if ((PowerInPlumes <= 0.0) || (TotSysFlow == 0.0) || (TSupK - Constant::Kelvin) > thisZoneHB.MAT) {
            // The system will mix
            HeightFrac = 0.0;
        } else {
            Gamma = std::pow(TotSysFlow * std::cos(ThrowAngle), 1.5) /
                    (NumberOfPlumes * std::pow(NumDiffusersPerPlume * DiffArea, 1.25) * std::sqrt(0.0281 * 0.001 * PowerInPlumes));
            if (zoneU.CalcTransHeight) {
                HeightFrac = (std::sqrt(NumDiffusersPerPlume * DiffArea) * (7.43 * std::log(Gamma) - 1.35) + 0.5 * SourceHeight) / CeilingHeight;
            } else {
                HeightFrac = zoneU.TransHeight / CeilingHeight;
            }
            HeightFrac = max(0.0, min(1.0, HeightFrac));
            for (int Ctd = 1; Ctd <= 4; ++Ctd) {
                HcUFAD(state, ZoneNum, HeightFrac, ufadCC);
                PowerInPlumes = ConvGains + ufadCC.HAT_OC - ufadCC.HA_OC * state.dataRoomAir->ZTOC(ZoneNum) + ufadCC.HAT_MX -
                                ufadCC.HA_MX * state.dataRoomAir->ZTMX(ZoneNum);
                if (PowerPerPlume > 0.0 && PowerInPlumes > 0.0) {
                    NumberOfPlumes = PowerInPlumes / PowerPerPlume;
                    NumDiffusersPerPlume = NumDiffusers / NumberOfPlumes;
                } else {
                    NumberOfPlumes = 1.0;
                    NumDiffusersPerPlume = 1.0;
                }
                if (PowerInPlumes <= 0.0) {
                    break;
                }
                Gamma = std::pow(TotSysFlow * std::cos(ThrowAngle), 1.5) /
                        (NumberOfPlumes * std::pow(NumDiffusersPerPlume * DiffArea, 1.25) * std::sqrt(0.0281 * 0.001 * PowerInPlumes));
                if (zoneU.CalcTransHeight) {
                    HeightFrac = (std::sqrt(NumDiffusersPerPlume * DiffArea) * (7.43 * std::log(Gamma) - 1.35) + 0.5 * SourceHeight) / CeilingHeight;
                } else {
                    HeightFrac = zoneU.TransHeight / CeilingHeight;
                }
                HeightFrac = max(0.0, min(1.0, HeightFrac));
                state.dataRoomAir->HeightTransition(ZoneNum) = HeightFrac * CeilingHeight;
                Real64 GainsFrac = zoneU.A_Kc * std::pow(Gamma, zoneU.B_Kc) + zoneU.C_Kc + zoneU.D_Kc * Gamma + zoneU.E_Kc * pow_2(Gamma);
                GainsFrac = max(0.6, min(GainsFrac, 1.0));
                state.dataRoomAir->AIRRATOC(ZoneNum) =
                    state.dataHeatBal->Zone(ZoneNum).Volume *
                    (state.dataRoomAir->HeightTransition(ZoneNum) - min(state.dataRoomAir->HeightTransition(ZoneNum), 0.2)) / CeilingHeight *
                    state.dataHeatBal->Zone(ZoneNum).ZoneVolCapMultpSens *
                    PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, state.dataRoomAir->MATOC(ZoneNum), thisZoneHB.airHumRat) *
                    PsyCpAirFnW(thisZoneHB.airHumRat) / TimeStepSysSec;
                state.dataRoomAir->AIRRATMX(ZoneNum) =
                    state.dataHeatBal->Zone(ZoneNum).Volume * (CeilingHeight - state.dataRoomAir->HeightTransition(ZoneNum)) / CeilingHeight *
                    state.dataHeatBal->Zone(ZoneNum).ZoneVolCapMultpSens *
                    PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, state.dataRoomAir->MATMX(ZoneNum), thisZoneHB.airHumRat) *
                    PsyCpAirFnW(thisZoneHB.airHumRat) / TimeStepSysSec;

                if (state.dataHVACGlobal->UseZoneTimeStepHistory) {
                    state.dataRoomAir->ZTMOC(ZoneNum)[2] = state.dataRoomAir->XMATOC(ZoneNum)[2];
                    state.dataRoomAir->ZTMOC(ZoneNum)[1] = state.dataRoomAir->XMATOC(ZoneNum)[1];
                    state.dataRoomAir->ZTMOC(ZoneNum)[0] = state.dataRoomAir->XMATOC(ZoneNum)[0];

                    state.dataRoomAir->ZTMMX(ZoneNum)[2] = state.dataRoomAir->XMATMX(ZoneNum)[2];
                    state.dataRoomAir->ZTMMX(ZoneNum)[1] = state.dataRoomAir->XMATMX(ZoneNum)[1];
                    state.dataRoomAir->ZTMMX(ZoneNum)[0] = state.dataRoomAir->XMATMX(ZoneNum)[0];

                } else {
                    state.dataRoomAir->ZTMOC(ZoneNum)[2] = state.dataRoomAir->DSXMATOC(ZoneNum)[2];
                    state.dataRoomAir->ZTMOC(ZoneNum)[1] = state.dataRoomAir->DSXMATOC(ZoneNum)[1];
                    state.dataRoomAir->ZTMOC(ZoneNum)[0] = state.dataRoomAir->DSXMATOC(ZoneNum)[0];

                    state.dataRoomAir->ZTMMX(ZoneNum)[2] = state.dataRoomAir->DSXMATMX(ZoneNum)[2];
                    state.dataRoomAir->ZTMMX(ZoneNum)[1] = state.dataRoomAir->DSXMATMX(ZoneNum)[1];
                    state.dataRoomAir->ZTMMX(ZoneNum)[0] = state.dataRoomAir->DSXMATMX(ZoneNum)[0];
                }

                Real64 AirCap = state.dataRoomAir->AIRRATOC(ZoneNum);
                Real64 TempHistTerm = AirCap * (3.0 * state.dataRoomAir->ZTMOC(ZoneNum)[0] - (3.0 / 2.0) * state.dataRoomAir->ZTMOC(ZoneNum)[1] +
                                                (1.0 / 3.0) * state.dataRoomAir->ZTMOC(ZoneNum)[2]);
                // Formerly CoefSumha, coef in zone temp equation with dimensions of h*A
                Real64 TempDepCoef = GainsFrac * ufadCC.HA_OC + MCp_Total;
                Real64 TempIndCoef = GainsFrac * (ConvGains + ufadCC.HAT_OC + ufadCC.HAT_MX - ufadCC.HA_MX * state.dataRoomAir->ZTMX(ZoneNum)) +
                                     MCpT_Total + thisZoneHB.NonAirSystemResponse / ZoneMult;
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    state.dataRoomAir->ZTOC(ZoneNum) =
                        (TempHistTerm + GainsFrac * (ConvGains + ufadCC.HAT_OC + ufadCC.HAT_MX - ufadCC.HA_MX * state.dataRoomAir->ZTMX(ZoneNum)) +
                         MCpT_Total + thisZoneHB.NonAirSystemResponse / ZoneMult) /
                        ((11.0 / 6.0) * AirCap + GainsFrac * ufadCC.HA_OC + MCp_Total);
                } break;
                case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                    if (TempDepCoef == 0.0) { // B=0
                        state.dataRoomAir->ZTOC(ZoneNum) = state.dataRoomAir->Zone1OC(ZoneNum) + TempIndCoef / AirCap;
                    } else {
                        state.dataRoomAir->ZTOC(ZoneNum) =
                            (state.dataRoomAir->Zone1OC(ZoneNum) - TempIndCoef / TempDepCoef) * std::exp(min(700.0, -TempDepCoef / AirCap)) +
                            TempIndCoef / TempDepCoef;
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
                                         (1.0 / 3.0) * state.dataRoomAir->ZTMMX(ZoneNum)[2]);
                TempDepCoef = (1.0 - GainsFrac) * ufadCC.HA_MX + MCp_Total;
                TempIndCoef = (1.0 - GainsFrac) * (ConvGains + ufadCC.HAT_OC + ufadCC.HAT_MX - ufadCC.HA_OC * state.dataRoomAir->ZTOC(ZoneNum)) +
                              state.dataRoomAir->ZTOC(ZoneNum) * MCp_Total;
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    state.dataRoomAir->ZTMX(ZoneNum) =
                        (TempHistTerm +
                         (1.0 - GainsFrac) * (ConvGains + ufadCC.HAT_OC + ufadCC.HAT_MX - ufadCC.HA_OC * state.dataRoomAir->ZTOC(ZoneNum)) +
                         state.dataRoomAir->ZTOC(ZoneNum) * MCp_Total) /
                        ((11.0 / 6.0) * AirCap + (1.0 - GainsFrac) * ufadCC.HA_MX + MCp_Total);
                } break;
                case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                    if (TempDepCoef == 0.0) { // B=0
                        state.dataRoomAir->ZTMX(ZoneNum) = state.dataRoomAir->Zone1MX(ZoneNum) + TempIndCoef / AirCap;
                    } else {
                        state.dataRoomAir->ZTMX(ZoneNum) =
                            (state.dataRoomAir->Zone1MX(ZoneNum) - TempIndCoef / TempDepCoef) * std::exp(min(700.0, -TempDepCoef / AirCap)) +
                            TempIndCoef / TempDepCoef;
                    }
                } break;
                case DataHeatBalance::SolutionAlgo::EulerMethod: {
                    state.dataRoomAir->ZTMX(ZoneNum) = (AirCap * state.dataRoomAir->Zone1MX(ZoneNum) + TempIndCoef) / (AirCap + TempDepCoef);
                } break;
                default:
                    break;
                }
                state.dataRoomAir->ZTFloor(ZoneNum) = state.dataRoomAir->ZTOC(ZoneNum);
            }
            if (PowerInPlumes <= 0.0) {
                HeightFrac = 0.0;
                state.dataRoomAir->AirModel(ZoneNum).SimAirModel = false;
                state.dataRoomAir->ZoneUFADGamma(ZoneNum) = 0.0;
                state.dataRoomAir->ZoneUFADPowInPlumes(ZoneNum) = 0.0;
            } else {
                state.dataRoomAir->AirModel(ZoneNum).SimAirModel = true;
                state.dataRoomAir->ZoneUFADGamma(ZoneNum) = Gamma;
                state.dataRoomAir->ZoneUFADPowInPlumes(ZoneNum) = PowerInPlumes;
            }
        }

        //=============================== M I X E D  Calculation ==============================================
        if (state.dataRoomAir->ZTMX(ZoneNum) < state.dataRoomAir->ZTOC(ZoneNum) || MCp_Total <= 0.0 ||
            HeightFrac * CeilingHeight < state.dataUFADManager->ThickOccupiedSubzoneMin) {
            MIXFLAG = true;
            HeightFrac = 0.0;
            state.dataRoomAir->AvgTempGrad(ZoneNum) = 0.0;
            state.dataRoomAir->MaxTempGrad(ZoneNum) = 0.0;
            state.dataRoomAir->AirModel(ZoneNum).SimAirModel = false;
            Real64 AirCap = thisZoneHB.AirPowerCap;
            Real64 TempHistTerm = AirCap * (3.0 * thisZoneHB.ZTM[0] - (3.0 / 2.0) * thisZoneHB.ZTM[1] + (1.0 / 3.0) * thisZoneHB.ZTM[2]);

            for (int Ctd = 1; Ctd <= 3; ++Ctd) {
                Real64 TempDepCoef = ufadCC.HA_MX + ufadCC.HA_OC + MCp_Total;
                Real64 const thisZoneT1 = thisZoneHB.T1;
                // Formerly CoefSumhat, coef in zone temp equation with dimensions of h*A(T1
                Real64 TempIndCoef = ConvGains + ufadCC.HAT_MX + ufadCC.HAT_OC + MCpT_Total;
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    ZTAveraged = (TempHistTerm + ConvGains + ufadCC.HAT_MX + ufadCC.HAT_OC + MCpT_Total) /
                                 ((11.0 / 6.0) * AirCap + ufadCC.HA_MX + ufadCC.HA_OC + MCp_Total);
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
                HcUFAD(state, ZoneNum, HeightFrac, ufadCC);
                TempDepCoef = ufadCC.HA_MX + ufadCC.HA_OC + MCp_Total;
                TempIndCoef = ConvGains + ufadCC.HAT_MX + ufadCC.HAT_OC + MCpT_Total;
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    ZTAveraged = (TempHistTerm + ConvGains + ufadCC.HAT_MX + ufadCC.HAT_OC + MCpT_Total) /
                                 ((11.0 / 6.0) * AirCap + ufadCC.HA_MX + ufadCC.HA_OC + MCp_Total);
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
        Real64 HeightUpSubzoneAve = (CeilingHeight + state.dataRoomAir->HeightTransition(ZoneNum)) / 2.0;
        Real64 HeightOccupiedSubzoneAve = state.dataRoomAir->HeightTransition(ZoneNum) / 2.0;
        // Comfort temperature

        if (MIXFLAG) {
            state.dataRoomAir->TCMF(ZoneNum) = ZTAveraged;
        } else {
            if (HeightComfort < HeightOccupiedSubzoneAve) {
                state.dataRoomAir->TCMF(ZoneNum) = state.dataRoomAir->ZTOC(ZoneNum);
            } else if (HeightComfort >= HeightOccupiedSubzoneAve && HeightComfort < HeightUpSubzoneAve) {
                state.dataRoomAir->TCMF(ZoneNum) = (state.dataRoomAir->ZTOC(ZoneNum) * (HeightUpSubzoneAve - HeightComfort) +
                                                    state.dataRoomAir->ZTMX(ZoneNum) * (HeightComfort - HeightOccupiedSubzoneAve)) /
                                                   (HeightUpSubzoneAve - HeightOccupiedSubzoneAve);
            } else if (HeightComfort >= HeightUpSubzoneAve && HeightComfort <= CeilingHeight) {
                state.dataRoomAir->TCMF(ZoneNum) = state.dataRoomAir->ZTMX(ZoneNum);
            } else {
                ShowFatalError(state,
                               format("UFAD comfort height is above ceiling or below floor in Zone: {}", state.dataHeatBal->Zone(ZoneNum).Name));
            }
        }

        // Temperature at the thermostat/temperature control sensor

        if (MIXFLAG) {
            state.dataHeatBalFanSys->TempTstatAir(ZoneNum) = ZTAveraged;
        } else {
            if (HeightThermostat < HeightOccupiedSubzoneAve) {
                state.dataHeatBalFanSys->TempTstatAir(ZoneNum) = state.dataRoomAir->ZTOC(ZoneNum);
            } else if (HeightThermostat >= HeightOccupiedSubzoneAve && HeightThermostat < HeightUpSubzoneAve) {
                state.dataHeatBalFanSys->TempTstatAir(ZoneNum) = (state.dataRoomAir->ZTOC(ZoneNum) * (HeightUpSubzoneAve - HeightThermostat) +
                                                                  state.dataRoomAir->ZTMX(ZoneNum) * (HeightThermostat - HeightOccupiedSubzoneAve)) /
                                                                 (HeightUpSubzoneAve - HeightOccupiedSubzoneAve);
            } else if (HeightThermostat >= HeightUpSubzoneAve && HeightThermostat <= CeilingHeight) {
                state.dataHeatBalFanSys->TempTstatAir(ZoneNum) = state.dataRoomAir->ZTMX(ZoneNum);
            } else {
                ShowFatalError(state,
                               format("Underfloor air distribution thermostat height is above ceiling or below floor in Zone: {}",
                                      state.dataHeatBal->Zone(ZoneNum).Name));
            }
        }

        // Temperature gradients
        if ((HeightUpSubzoneAve - HeightOccupiedSubzoneAve) > 0.1) {
            state.dataRoomAir->AvgTempGrad(ZoneNum) =
                (state.dataRoomAir->ZTMX(ZoneNum) - state.dataRoomAir->ZTOC(ZoneNum)) / (HeightUpSubzoneAve - HeightOccupiedSubzoneAve);
        } else {
            state.dataRoomAir->AvgTempGrad(ZoneNum) = 0.0;
        }

        if (MIXFLAG) {
            state.dataRoomAir->ZoneUFADMixedFlag(ZoneNum) = 1;
            state.dataRoomAir->AirModel(ZoneNum).SimAirModel = false;
        } else {
            state.dataRoomAir->ZoneUFADMixedFlag(ZoneNum) = 0;
            state.dataRoomAir->AirModel(ZoneNum).SimAirModel = true;
        }

        if (ZoneEquipConfigNum > 0) {
            int ZoneNodeNum = state.dataHeatBal->Zone(ZoneNum).SystemZoneNodeNumber;
            state.dataLoopNodes->Node(ZoneNodeNum).Temp = state.dataRoomAir->ZTMX(ZoneNum);
        }

        if (MIXFLAG) {
            state.dataRoomAir->Phi(ZoneNum) = 1.0;
        } else {
            state.dataRoomAir->Phi(ZoneNum) =
                (state.dataRoomAir->ZTOC(ZoneNum) - (TSupK - Constant::Kelvin)) / (state.dataRoomAir->ZTMX(ZoneNum) - (TSupK - Constant::Kelvin));
        }

        // Mixed for reporting purposes
        if ((MIXFLAG) || ((state.dataRoomAir->ZTMX(ZoneNum) - state.dataRoomAir->ZTOC(ZoneNum)) < TempDiffCritRep)) {
            state.dataRoomAir->ZoneUFADMixedFlagRep(ZoneNum) = 1.0;
            state.dataRoomAir->HeightTransition(ZoneNum) = 0.0;
            state.dataRoomAir->AvgTempGrad(ZoneNum) = 0.0;
        } else {
            state.dataRoomAir->ZoneUFADMixedFlagRep(ZoneNum) = 0.0;
        }
    }

    void CalcUFADExt(EnergyPlusData &state, int const ZoneNum) // index number for the specified zone
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   January 2006
        //       MODIFIED       Brent Griffith June 2008 for new interpolation and time history
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Using the UCSD UFAD exterior zone model, this subroutine calculates the  occupied subzone height,
        // surface heat transfer coefficients, the occupied subzone temperature, and the upper subzone temperature.

        // METHODOLOGY EMPLOYED:
        // The zone is divided into 2 subzones with a variable transition height.

        // REFERENCES:
        // The model is described in the EnergyPlus Engineering Reference in Anna Liu's UCSD PhD thesis.

        // Using/Aliasing
        using Psychrometrics::PsyCpAirFnW;
        using Psychrometrics::PsyRhoAirFnPbTdbW;
        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        using InternalHeatGains::SumInternalConvectionGainsByTypes;
        using InternalHeatGains::SumReturnAirConvectionGainsByTypes;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        Real64 PowerInPlumesPerMeter; // Power in Plumes per meter of window length [W/m]
        Real64 ZTAveraged;

        // Exact solution or Euler method
        if (state.dataHeatBal->ZoneAirSolutionAlgo != DataHeatBalance::SolutionAlgo::ThirdOrder) {
            if (state.dataHVACGlobal->ShortenTimeStepSysRoomAir && TimeStepSys < state.dataGlobal->TimeStepZone) {
                if (state.dataHVACGlobal->PreviousTimeStep < state.dataGlobal->TimeStepZone) {
                    state.dataRoomAir->Zone1OC(ZoneNum) = state.dataRoomAir->ZoneM2OC(ZoneNum);
                    state.dataRoomAir->Zone1MX(ZoneNum) = state.dataRoomAir->ZoneM2MX(ZoneNum);
                } else {
                    state.dataRoomAir->Zone1OC(ZoneNum) = state.dataRoomAir->ZoneMXOC(ZoneNum);
                    state.dataRoomAir->Zone1MX(ZoneNum) = state.dataRoomAir->ZoneMXMX(ZoneNum);
                }
            } else {
                state.dataRoomAir->Zone1OC(ZoneNum) = state.dataRoomAir->ZTOC(ZoneNum);
                state.dataRoomAir->Zone1MX(ZoneNum) = state.dataRoomAir->ZTMX(ZoneNum);
            }
        }

        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
        Real64 HeightFrac = 0.0; // Fractional height of transition between occupied and upper subzones
        bool MIXFLAG = false;
        state.dataRoomAir->UFADHcIn = state.dataHeatBalSurf->SurfHConvInt;
        Real64 SumSysMCp = 0.0;     // Sum of system mass flow rate * specific heat for this zone [W/K]
        Real64 SumSysMCpT = 0.0;    // Sum of system mass flow rate * specific heat * temperature for this zone [W]
        Real64 TotSysFlow = 0.0;    // [m3/s]
        Real64 TSupK = 0.0;         // supply temperature [K]
        Real64 SumSysM = 0.0;       // Sum of systems mass flow rate [kg/s]
        Real64 PowerInPlumes = 0.0; // [W]
        Real64 Gamma = 0.0;         // dimensionless height parameter; higher gamma means interface height will be
        // higher, smaller gamma means interface height will be lower.
        int ZoneMult = state.dataHeatBal->Zone(ZoneNum).Multiplier * state.dataHeatBal->Zone(ZoneNum).ListMultiplier;
        Real64 CeilingHeight = state.dataRoomAir->ZoneCeilingHeight2(ZoneNum) - state.dataRoomAir->ZoneCeilingHeight1(ZoneNum);

        auto &zoneU = state.dataRoomAir->ZoneUFAD(state.dataRoomAir->ZoneUFADPtr(ZoneNum));
        Real64 HeightThermostat = zoneU.ThermostatHeight;         // height of the thermostat above the floor [m]
        Real64 HeightComfort = zoneU.ComfortHeight;               // height at which comfort temperature is calculated
        Real64 TempDiffCritRep = zoneU.TempTrigger;               // Minimum temperature difference between upper and occupied subzones for reporting
        Real64 DiffArea = zoneU.DiffArea;                         // diffuser effective area [m2]
        Real64 ThrowAngle = Constant::DegToRad * zoneU.DiffAngle; // diffuser slot angle relative to vertical [radians]
        Real64 SourceHeight = zoneU.HeatSrcHeight;                // height of plume sources above the floor [m]
        Real64 NumDiffusers = zoneU.DiffusersPerZone;
        Real64 PowerPerPlume = zoneU.PowerPerPlume;
        // gains from occupants, task lighting, elec equip, gas equip, other equip, hot water equip, steam equip,
        // baseboards (nonthermostatic), water heater skin loss
        Real64 ConvGainsOccSubzone = SumInternalConvectionGainsByTypes(state, ZoneNum, IntGainTypesOccupied);

        // Add heat to return air if zonal system (no return air) or cycling system (return air frequently very
        // low or zero)
        if (state.dataHeatBal->Zone(ZoneNum).NoHeatToReturnAir) {
            ConvGainsOccSubzone += SumReturnAirConvectionGainsByTypes(state, ZoneNum, IntGainTypesOccupied);
        }

        // Add convection from pool cover to occupied region
        ConvGainsOccSubzone += state.dataHeatBalFanSys->SumConvPool(ZoneNum);

        // gains from lights (ceiling), tubular daylighting devices, high temp radiant heaters
        Real64 ConvGainsUpSubzone =
            SumInternalConvectionGainsByTypes(state, ZoneNum, IntGainTypesUpSubzone); // convective heat gains into the upper subzone [W]
        ConvGainsUpSubzone += state.dataHeatBalFanSys->SumConvHTRadSys(ZoneNum);
        if (state.dataHeatBal->Zone(ZoneNum).NoHeatToReturnAir) {
            ConvGainsUpSubzone += SumReturnAirConvectionGainsByTypes(state, ZoneNum, IntGainTypesUpSubzone);
        }
        Real64 ConvGains =
            ConvGainsOccSubzone + ConvGainsUpSubzone + thisZoneHB.SysDepZoneLoadsLagged; // total zone convective gains (excluding surfaces) [W]
        int ZoneEquipConfigNum = zoneU.ZoneEquipPtr;
        if (ZoneEquipConfigNum > 0) {
            auto const &zoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(ZoneEquipConfigNum);
            for (int InNodeIndex = 1; InNodeIndex <= zoneEquipConfig.NumInletNodes; ++InNodeIndex) {
                Real64 NodeTemp = state.dataLoopNodes->Node(zoneEquipConfig.InletNode(InNodeIndex)).Temp;
                Real64 MassFlowRate = state.dataLoopNodes->Node(zoneEquipConfig.InletNode(InNodeIndex)).MassFlowRate;
                Real64 CpAir = PsyCpAirFnW(thisZoneHB.airHumRat);
                SumSysMCp += MassFlowRate * CpAir;
                SumSysMCpT += MassFlowRate * CpAir * NodeTemp;
                TotSysFlow += MassFlowRate / PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, NodeTemp, thisZoneHB.airHumRat);
                TSupK += MassFlowRate * NodeTemp;
                SumSysM += MassFlowRate;
            }
            if (TotSysFlow > 0.0) {
                TSupK = TSupK / SumSysM + Constant::Kelvin;
            } else {
                TSupK = 0.0;
            }
        }

        // mass flow rate * specific heat for this zone for infiltration, ventilation, mixing [W/K]
        Real64 SumMCp = thisZoneHB.MCPI + thisZoneHB.MCPV + thisZoneHB.MCPM + thisZoneHB.MDotCPOA;
        // mass flow rate * specific heat* temp for this zone for infiltration, ventilation, mixing [W]
        Real64 SumMCpT =
            thisZoneHB.MCPTI + thisZoneHB.MCPTV + thisZoneHB.MCPTM + thisZoneHB.MDotCPOA * state.dataHeatBal->Zone(ZoneNum).OutDryBulbTemp;

        Real64 MCp_Total = SumMCp + SumSysMCp;    // total mass flow rate * specific heat for this zone [W/K]
        Real64 MCpT_Total = SumMCpT + SumSysMCpT; // total mass flow rate * specific heat* temp for this zone [W]

        // For the York MIT diffusers (variable area) the area varies with the flow rate. Assume 400 ft/min velocity
        // at the diffuser, and a design flow rate of 150 cfm (.0708 m3/s). Then the design area for each diffuser is
        // 150 ft3/min / 400 ft/min = .375 ft2 = .035 m2. This is adjusted each time step by
        //               (TotSysFlow/(NumDiffusers*.0708))*.035
        if (zoneU.DiffuserType == Diffuser::VarArea) {
            DiffArea = 0.035 * TotSysFlow / (0.0708 * NumDiffusers);
        }
        // initial estimate of convective transfer from surfaces; assume HeightFrac is 0.5.
        UFADConvCoef ufadCC;
        HcUFAD(state, ZoneNum, 0.5, ufadCC);
        Real64 ConvGainsWindows = ufadCC.HAT_MXWin + ufadCC.HAT_OCWin - // ZoneEquipConfig index for this UFAD zone
                                  ufadCC.HA_MXWin * state.dataRoomAir->ZTMX(ZoneNum) - ufadCC.HA_OCWin * state.dataRoomAir->ZTOC(ZoneNum);
        PowerInPlumes = ConvGains + ufadCC.HAT_OC - ufadCC.HA_OC * state.dataRoomAir->ZTOC(ZoneNum) + ufadCC.HAT_MX -
                        ufadCC.HA_MX * state.dataRoomAir->ZTMX(ZoneNum);
        // NumberOfPlumes = PowerInPlumes / PowerPerPlume
        Real64 NumberOfPlumes = (PowerPerPlume > 0.0 && PowerInPlumes > 0.0) ? (PowerInPlumes / PowerPerPlume) : 1.0;
        Real64 NumDiffusersPerPlume = (PowerPerPlume > 0.0 && PowerInPlumes > 0.0) ? (NumDiffusers / NumberOfPlumes) : 1.0;

        if ((PowerInPlumes <= 0.0) || (TotSysFlow == 0.0) || (TSupK - Constant::Kelvin) > thisZoneHB.MAT) {
            // The system will mix
            HeightFrac = 0.0;
        } else {
            if (PowerInPlumes > 0.0) {
                if (zoneU.WinWidth > 0.0) { // exterior zone formula
                    PowerInPlumesPerMeter = PowerInPlumes / zoneU.WinWidth;
                    Gamma =
                        (TotSysFlow * std::cos(ThrowAngle)) / (NumDiffusers * DiffArea * std::pow(0.0281 * 0.001 * PowerInPlumesPerMeter, 0.333333));
                } else { // interior zone formula
                    Gamma = std::pow(TotSysFlow * std::cos(ThrowAngle), 1.5) /
                            (NumberOfPlumes * std::pow(NumDiffusersPerPlume * DiffArea, 1.25) * std::sqrt(0.0281 * 0.001 * PowerInPlumes));
                }
            } else {
                Gamma = 1000.0;
            }
            if (zoneU.CalcTransHeight) {
                if (zoneU.WinWidth > 0.0) { // use exterior zone formula
                    HeightFrac = (std::sqrt(DiffArea) * (11.03 * std::log(Gamma) - 10.73) + 0.5 * SourceHeight) / CeilingHeight;
                } else { // use interior zone formula
                    HeightFrac = (std::sqrt(NumDiffusersPerPlume * DiffArea) * (7.43 * std::log(Gamma) - 1.35) + 0.5 * SourceHeight) / CeilingHeight;
                }
            } else {
                HeightFrac = zoneU.TransHeight / CeilingHeight;
            }
            HeightFrac = max(0.0, min(1.0, HeightFrac));
            Real64 GainsFrac = zoneU.A_Kc * std::pow(Gamma, zoneU.B_Kc) + zoneU.C_Kc + zoneU.D_Kc * Gamma + zoneU.E_Kc * pow_2(Gamma);
            GainsFrac = max(0.7, min(GainsFrac, 1.0));
            if (zoneU.ShadeDown) {
                GainsFrac -= 0.2;
            }
            state.dataRoomAir->ZoneUFADPowInPlumes(ZoneNum) = PowerInPlumes;
            for (int Ctd = 1; Ctd <= 4; ++Ctd) {
                HcUFAD(state, ZoneNum, HeightFrac, ufadCC);
                ConvGainsWindows = ufadCC.HAT_MXWin + ufadCC.HAT_OCWin - ufadCC.HA_MXWin * state.dataRoomAir->ZTMX(ZoneNum) -
                                   ufadCC.HA_OCWin * state.dataRoomAir->ZTOC(ZoneNum);
                ConvGainsWindows = max(ConvGainsWindows, 0.0);
                PowerInPlumes = ConvGains + ufadCC.HAT_OC - ufadCC.HA_OC * state.dataRoomAir->ZTOC(ZoneNum) + ufadCC.HAT_MX -
                                ufadCC.HA_MX * state.dataRoomAir->ZTMX(ZoneNum);
                // NumberOfPlumes = PowerInPlumes / PowerPerPlume
                NumberOfPlumes = 1.0;
                if (PowerInPlumes <= 0.0) {
                    break;
                }
                if (zoneU.WinWidth > 0.0) { // use exterior zone formula
                    PowerInPlumesPerMeter = PowerInPlumes / zoneU.WinWidth;
                    Gamma =
                        (TotSysFlow * std::cos(ThrowAngle)) / (NumDiffusers * DiffArea * std::pow(0.0281 * 0.001 * PowerInPlumesPerMeter, 0.333333));
                } else { // use interior zone formula
                    Gamma = std::pow(TotSysFlow * std::cos(ThrowAngle), 1.5) /
                            (NumberOfPlumes * std::pow(NumDiffusersPerPlume * DiffArea, 1.25) * std::sqrt(0.0281 * 0.001 * PowerInPlumes));
                }
                if (zoneU.CalcTransHeight) {
                    if (zoneU.WinWidth > 0.0) { // exterior zone formula
                        HeightFrac = (std::sqrt(DiffArea) * (11.03 * std::log(Gamma) - 10.73) + 0.5 * SourceHeight) / CeilingHeight;
                    } else { // interior zone formula
                        HeightFrac =
                            (std::sqrt(NumDiffusersPerPlume * DiffArea) * (7.43 * std::log(Gamma) - 1.35) + 0.5 * SourceHeight) / CeilingHeight;
                    }
                } else {
                    HeightFrac = zoneU.TransHeight / CeilingHeight;
                }
                HeightFrac = min(1.0, HeightFrac);
                state.dataRoomAir->HeightTransition(ZoneNum) = HeightFrac * CeilingHeight;
                Real64 GainsFrac = zoneU.A_Kc * std::pow(Gamma, zoneU.B_Kc) + zoneU.C_Kc + zoneU.D_Kc * Gamma + zoneU.E_Kc * pow_2(Gamma);
                GainsFrac = max(0.7, min(GainsFrac, 1.0));
                if (zoneU.ShadeDown) {
                    GainsFrac -= 0.2;
                }
                state.dataRoomAir->AIRRATOC(ZoneNum) =
                    state.dataHeatBal->Zone(ZoneNum).Volume *
                    (state.dataRoomAir->HeightTransition(ZoneNum) - min(state.dataRoomAir->HeightTransition(ZoneNum), 0.2)) / CeilingHeight *
                    state.dataHeatBal->Zone(ZoneNum).ZoneVolCapMultpSens *
                    PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, state.dataRoomAir->MATOC(ZoneNum), thisZoneHB.airHumRat) *
                    PsyCpAirFnW(thisZoneHB.airHumRat) / TimeStepSysSec;
                state.dataRoomAir->AIRRATMX(ZoneNum) =
                    state.dataHeatBal->Zone(ZoneNum).Volume * (CeilingHeight - state.dataRoomAir->HeightTransition(ZoneNum)) / CeilingHeight *
                    state.dataHeatBal->Zone(ZoneNum).ZoneVolCapMultpSens *
                    PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, state.dataRoomAir->MATMX(ZoneNum), thisZoneHB.airHumRat) *
                    PsyCpAirFnW(thisZoneHB.airHumRat) / TimeStepSysSec;

                if (state.dataHVACGlobal->UseZoneTimeStepHistory) {
                    state.dataRoomAir->ZTMOC(ZoneNum)[2] = state.dataRoomAir->XMATOC(ZoneNum)[2];
                    state.dataRoomAir->ZTMOC(ZoneNum)[1] = state.dataRoomAir->XMATOC(ZoneNum)[1];
                    state.dataRoomAir->ZTMOC(ZoneNum)[0] = state.dataRoomAir->XMATOC(ZoneNum)[0];

                    state.dataRoomAir->ZTMMX(ZoneNum)[2] = state.dataRoomAir->XMATMX(ZoneNum)[2];
                    state.dataRoomAir->ZTMMX(ZoneNum)[1] = state.dataRoomAir->XMATMX(ZoneNum)[1];
                    state.dataRoomAir->ZTMMX(ZoneNum)[0] = state.dataRoomAir->XMATMX(ZoneNum)[0];

                } else {
                    state.dataRoomAir->ZTMOC(ZoneNum)[2] = state.dataRoomAir->DSXMATOC(ZoneNum)[2];
                    state.dataRoomAir->ZTMOC(ZoneNum)[1] = state.dataRoomAir->DSXMATOC(ZoneNum)[1];
                    state.dataRoomAir->ZTMOC(ZoneNum)[0] = state.dataRoomAir->DSXMATOC(ZoneNum)[0];

                    state.dataRoomAir->ZTMMX(ZoneNum)[2] = state.dataRoomAir->DSXMATMX(ZoneNum)[2];
                    state.dataRoomAir->ZTMMX(ZoneNum)[1] = state.dataRoomAir->DSXMATMX(ZoneNum)[1];
                    state.dataRoomAir->ZTMMX(ZoneNum)[0] = state.dataRoomAir->DSXMATMX(ZoneNum)[0];
                }

                Real64 AirCap = state.dataRoomAir->AIRRATOC(ZoneNum);
                Real64 TempHistTerm = AirCap * (3.0 * state.dataRoomAir->ZTMOC(ZoneNum)[0] - (3.0 / 2.0) * state.dataRoomAir->ZTMOC(ZoneNum)[1] +
                                                (1.0 / 3.0) * state.dataRoomAir->ZTMOC(ZoneNum)[2]);
                Real64 TempDepCoef = GainsFrac * ufadCC.HA_OC + MCp_Total;
                Real64 TempIndCoef = GainsFrac * (ConvGains + ufadCC.HAT_OC + ufadCC.HAT_MX - ufadCC.HA_MX * state.dataRoomAir->ZTMX(ZoneNum)) +
                                     MCpT_Total + thisZoneHB.NonAirSystemResponse / ZoneMult;
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    state.dataRoomAir->ZTOC(ZoneNum) =
                        (TempHistTerm + GainsFrac * (ConvGains + ufadCC.HAT_OC + ufadCC.HAT_MX - ufadCC.HA_MX * state.dataRoomAir->ZTMX(ZoneNum)) +
                         MCpT_Total + thisZoneHB.NonAirSystemResponse / ZoneMult) /
                        ((11.0 / 6.0) * AirCap + GainsFrac * ufadCC.HA_OC + MCp_Total);
                } break;
                case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                    if (TempDepCoef == 0.0) { // B=0
                        state.dataRoomAir->ZTOC(ZoneNum) = state.dataRoomAir->Zone1OC(ZoneNum) + TempIndCoef / AirCap;
                    } else {
                        state.dataRoomAir->ZTOC(ZoneNum) =
                            (state.dataRoomAir->Zone1OC(ZoneNum) - TempIndCoef / TempDepCoef) * std::exp(min(700.0, -TempDepCoef / AirCap)) +
                            TempIndCoef / TempDepCoef;
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
                                         (1.0 / 3.0) * state.dataRoomAir->ZTMMX(ZoneNum)[2]);
                TempDepCoef = (1.0 - GainsFrac) * ufadCC.HA_MX + MCp_Total;
                TempIndCoef = (1.0 - GainsFrac) * (ConvGains + ufadCC.HAT_OC + ufadCC.HAT_MX - ufadCC.HA_OC * state.dataRoomAir->ZTOC(ZoneNum)) +
                              state.dataRoomAir->ZTOC(ZoneNum) * MCp_Total;
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    state.dataRoomAir->ZTMX(ZoneNum) =
                        (TempHistTerm +
                         (1.0 - GainsFrac) * (ConvGains + ufadCC.HAT_OC + ufadCC.HAT_MX - ufadCC.HA_OC * state.dataRoomAir->ZTOC(ZoneNum)) +
                         state.dataRoomAir->ZTOC(ZoneNum) * MCp_Total) /
                        ((11.0 / 6.0) * AirCap + (1.0 - GainsFrac) * ufadCC.HA_MX + MCp_Total);
                } break;
                case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                    if (TempDepCoef == 0.0) { // B=0
                        state.dataRoomAir->ZTMX(ZoneNum) = state.dataRoomAir->Zone1MX(ZoneNum) + TempIndCoef / AirCap;
                    } else {
                        state.dataRoomAir->ZTMX(ZoneNum) =
                            (state.dataRoomAir->Zone1MX(ZoneNum) - TempIndCoef / TempDepCoef) * std::exp(min(700.0, -TempDepCoef / AirCap)) +
                            TempIndCoef / TempDepCoef;
                    }
                } break;
                case DataHeatBalance::SolutionAlgo::EulerMethod: {
                    state.dataRoomAir->ZTMX(ZoneNum) = (AirCap * state.dataRoomAir->Zone1MX(ZoneNum) + TempIndCoef) / (AirCap + TempDepCoef);
                } break;
                default:
                    break;
                }
                state.dataRoomAir->ZTFloor(ZoneNum) = state.dataRoomAir->ZTOC(ZoneNum);
            }
            if (PowerInPlumes <= 0.0) {
                HeightFrac = 0.0;
                state.dataRoomAir->AirModel(ZoneNum).SimAirModel = false;
                state.dataRoomAir->ZoneUFADGamma(ZoneNum) = 0.0;
                state.dataRoomAir->ZoneUFADPowInPlumes(ZoneNum) = 0.0;
                state.dataRoomAir->ZoneUFADPowInPlumesfromWindows(ZoneNum) = 0.0;
            } else {
                state.dataRoomAir->AirModel(ZoneNum).SimAirModel = true;
                state.dataRoomAir->ZoneUFADGamma(ZoneNum) = Gamma;
                state.dataRoomAir->ZoneUFADPowInPlumes(ZoneNum) = PowerInPlumes;
                state.dataRoomAir->ZoneUFADPowInPlumesfromWindows(ZoneNum) = ConvGainsWindows;
            }
        }

        //=============================== M I X E D  Calculation ==============================================
        if (state.dataRoomAir->ZTMX(ZoneNum) < state.dataRoomAir->ZTOC(ZoneNum) || MCp_Total <= 0.0 ||
            HeightFrac * CeilingHeight < state.dataUFADManager->ThickOccupiedSubzoneMin) {
            MIXFLAG = true;
            HeightFrac = 0.0;
            Real64 const thisZoneT1 = thisZoneHB.T1;

            state.dataRoomAir->AvgTempGrad(ZoneNum) = 0.0;
            state.dataRoomAir->MaxTempGrad(ZoneNum) = 0.0;
            state.dataRoomAir->AirModel(ZoneNum).SimAirModel = false;
            Real64 AirCap = thisZoneHB.AirPowerCap;
            Real64 TempHistTerm = AirCap * (3.0 * thisZoneHB.ZTM[0] - (3.0 / 2.0) * thisZoneHB.ZTM[1] + (1.0 / 3.0) * thisZoneHB.ZTM[2]);

            for (int Ctd = 1; Ctd <= 3; ++Ctd) {
                Real64 TempDepCoef = ufadCC.HA_MX + ufadCC.HA_OC + MCp_Total;
                Real64 TempIndCoef = ConvGains + ufadCC.HAT_MX + ufadCC.HAT_OC + MCpT_Total;
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    ZTAveraged = (TempHistTerm + ConvGains + ufadCC.HAT_MX + ufadCC.HAT_OC + MCpT_Total) /
                                 ((11.0 / 6.0) * AirCap + ufadCC.HA_MX + ufadCC.HA_OC + MCp_Total);
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
                HcUFAD(state, ZoneNum, HeightFrac, ufadCC);
                TempDepCoef = ufadCC.HA_MX + ufadCC.HA_OC + MCp_Total;
                TempIndCoef = ConvGains + ufadCC.HAT_MX + ufadCC.HAT_OC + MCpT_Total;
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    ZTAveraged = (TempHistTerm + ConvGains + ufadCC.HAT_MX + ufadCC.HAT_OC + MCpT_Total) /
                                 ((11.0 / 6.0) * AirCap + ufadCC.HA_MX + ufadCC.HA_OC + MCp_Total);
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

        Real64 HeightUpSubzoneAve = (CeilingHeight + state.dataRoomAir->HeightTransition(ZoneNum)) / 2.0;
        Real64 HeightOccupiedSubzoneAve = state.dataRoomAir->HeightTransition(ZoneNum) / 2.0;
        // Comfort temperature

        if (MIXFLAG) {
            state.dataRoomAir->TCMF(ZoneNum) = ZTAveraged;
        } else {
            if (HeightComfort < HeightOccupiedSubzoneAve) {
                state.dataRoomAir->TCMF(ZoneNum) = state.dataRoomAir->ZTOC(ZoneNum);
            } else if (HeightComfort >= HeightOccupiedSubzoneAve && HeightComfort < HeightUpSubzoneAve) {
                state.dataRoomAir->TCMF(ZoneNum) = (state.dataRoomAir->ZTOC(ZoneNum) * (HeightUpSubzoneAve - HeightComfort) +
                                                    state.dataRoomAir->ZTMX(ZoneNum) * (HeightComfort - HeightOccupiedSubzoneAve)) /
                                                   (HeightUpSubzoneAve - HeightOccupiedSubzoneAve);
            } else if (HeightComfort >= HeightUpSubzoneAve && HeightComfort <= CeilingHeight) {
                state.dataRoomAir->TCMF(ZoneNum) = state.dataRoomAir->ZTMX(ZoneNum);
            } else {
                ShowFatalError(state,
                               format("UFAD comfort height is above ceiling or below floor in Zone: {}", state.dataHeatBal->Zone(ZoneNum).Name));
            }
        }

        // Temperature at the thermostat/temperature control sensor

        if (MIXFLAG) {
            state.dataHeatBalFanSys->TempTstatAir(ZoneNum) = ZTAveraged;
        } else {
            if (HeightThermostat < HeightOccupiedSubzoneAve) {
                state.dataHeatBalFanSys->TempTstatAir(ZoneNum) = state.dataRoomAir->ZTOC(ZoneNum);
            } else if (HeightThermostat >= HeightOccupiedSubzoneAve && HeightThermostat < HeightUpSubzoneAve) {
                state.dataHeatBalFanSys->TempTstatAir(ZoneNum) = (state.dataRoomAir->ZTOC(ZoneNum) * (HeightUpSubzoneAve - HeightThermostat) +
                                                                  state.dataRoomAir->ZTMX(ZoneNum) * (HeightThermostat - HeightOccupiedSubzoneAve)) /
                                                                 (HeightUpSubzoneAve - HeightOccupiedSubzoneAve);
            } else if (HeightThermostat >= HeightUpSubzoneAve && HeightThermostat <= CeilingHeight) {
                state.dataHeatBalFanSys->TempTstatAir(ZoneNum) = state.dataRoomAir->ZTMX(ZoneNum);
            } else {
                ShowFatalError(state,
                               format("Underfloor air distribution thermostat height is above ceiling or below floor in Zone: {}",
                                      state.dataHeatBal->Zone(ZoneNum).Name));
            }
        }

        // Temperature gradients
        if ((HeightUpSubzoneAve - HeightOccupiedSubzoneAve) > 0.1) {
            state.dataRoomAir->AvgTempGrad(ZoneNum) =
                (state.dataRoomAir->ZTMX(ZoneNum) - state.dataRoomAir->ZTOC(ZoneNum)) / (HeightUpSubzoneAve - HeightOccupiedSubzoneAve);
        } else {
            state.dataRoomAir->AvgTempGrad(ZoneNum) = 0.0;
        }

        if (MIXFLAG) {
            state.dataRoomAir->ZoneUFADMixedFlag(ZoneNum) = 1;
            state.dataRoomAir->AirModel(ZoneNum).SimAirModel = false;
        } else {
            state.dataRoomAir->ZoneUFADMixedFlag(ZoneNum) = 0;
            state.dataRoomAir->AirModel(ZoneNum).SimAirModel = true;
        }

        if (ZoneEquipConfigNum > 0) {
            int ZoneNodeNum = state.dataHeatBal->Zone(ZoneNum).SystemZoneNodeNumber;
            state.dataLoopNodes->Node(ZoneNodeNum).Temp = state.dataRoomAir->ZTMX(ZoneNum);
        }

        if (MIXFLAG) {
            state.dataRoomAir->Phi(ZoneNum) = 1.0;
        } else {
            state.dataRoomAir->Phi(ZoneNum) =
                (state.dataRoomAir->ZTOC(ZoneNum) - (TSupK - Constant::Kelvin)) / (state.dataRoomAir->ZTMX(ZoneNum) - (TSupK - Constant::Kelvin));
        }

        // Mixed for reporting purposes
        if ((MIXFLAG) || ((state.dataRoomAir->ZTMX(ZoneNum) - state.dataRoomAir->ZTOC(ZoneNum)) < TempDiffCritRep)) {
            state.dataRoomAir->ZoneUFADMixedFlagRep(ZoneNum) = 1.0;
            state.dataRoomAir->HeightTransition(ZoneNum) = 0.0;
            state.dataRoomAir->AvgTempGrad(ZoneNum) = 0.0;
        } else {
            state.dataRoomAir->ZoneUFADMixedFlagRep(ZoneNum) = 0.0;
        }
    }

} // namespace RoomAir
} // namespace EnergyPlus
