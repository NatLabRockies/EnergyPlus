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
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/member.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataMoistureBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceHAMTManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace HeatBalanceHAMTManager {

    // MODULE INFORMATION:
    //       AUTHOR      Phillip Biddulph
    //       DATE WRITTEN   June 2008
    //       MODIFIED
    //       Bug fixes to make sure HAMT can cope with data limits  ! PDB August 2009
    //       RE-ENGINEERED

    // PURPOSE OF THIS MODULE:
    // Calculate, record and report the one dimentional heat and moisture transfer
    // through a surface given the material composition of the building surface and
    // the external and internal Temperatures and Relative Humidities.

    // METHODOLOGY EMPLOYED:
    // Each surface is split into "cells", where all characteristics are initiallised.
    // Cells are matched and links created in the initialisation routine.
    // The internal and external "surfaces" of the surface are virtual cells to allow for the
    // input of heat and vapor via heat transfer coefficients, radiation,
    // and vapor transfer coefficients
    // Uses Forward (implicit) finite difference alogorithm. Heat transfer is caclulated first,
    // with the option of including the latent heat, then liquid and vapor transfer. The process is ittereated.
    // Once the temperatures have converged the internal surface
    // temperature and vapor densities are passed back to EnergyPlus.

    // Temperatures and relative humidities are updated once EnergyPlus has checked that
    // the zone temperatures have converged.

    // REFERENCES:
    // K?zel, H.M. (1995) Simultaneous Heat and Moisture Transport in Building Components.
    // One- and two-dimensional calculation using simple parameters. IRB Verlag 1995
    // Holman, J.P. (2002) Heat Transfer, Ninth Edition. McGraw-Hill
    // Winterton, R.H.S. (1997) Heat Transfer. (Oxford Chemistry Primers; 50) Oxford University Press
    // Kumar Kumaran, M. (1996) IEA ANNEX 24, Final Report, Volume 3

    // USE STATEMENTS:

    // Using/Aliasing
    using namespace DataSurfaces;
    using DataHeatBalSurface::MinSurfaceTempLimit;
    using DataHeatBalSurface::MinSurfaceTempLimitBeforeFatal;
    using namespace DataHeatBalance;
    using namespace Psychrometrics;

    void ManageHeatBalHAMT(EnergyPlusData &state, int const SurfNum, Real64 &SurfTempInTmp, Real64 &TempSurfOutTmp)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Phillip Biddulph
        //       DATE WRITTEN   June 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Manages the Heat and Moisture Transfer calculations.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        if (state.dataHeatBalHAMTMgr->OneTimeFlag) {
            state.dataHeatBalHAMTMgr->OneTimeFlag = false;
            DisplayString(state, "Initialising Heat and Moisture Transfer Model");
            GetHeatBalHAMTInput(state);
            InitHeatBalHAMT(state);
        }

        CalcHeatBalHAMT(state, SurfNum, SurfTempInTmp, TempSurfOutTmp);
    }

    void GetHeatBalHAMTInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Phillip Biddulph
        //       DATE WRITTEN   June 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // gets input for the HAMT model
        static constexpr std::string_view routineName = "GetHeatBalHAMTInput";

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const cHAMTObject1("MaterialProperty:HeatAndMoistureTransfer:Settings");
        static std::string const cHAMTObject2("MaterialProperty:HeatAndMoistureTransfer:SorptionIsotherm");
        static std::string const cHAMTObject3("MaterialProperty:HeatAndMoistureTransfer:Suction");
        static std::string const cHAMTObject4("MaterialProperty:HeatAndMoistureTransfer:Redistribution");
        static std::string const cHAMTObject5("MaterialProperty:HeatAndMoistureTransfer:Diffusion");
        static std::string const cHAMTObject6("MaterialProperty:HeatAndMoistureTransfer:ThermalConductivity");
        static std::string const cHAMTObject7("SurfaceProperties:VaporCoefficients");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        Array1D_string AlphaArray;
        Array1D_string cAlphaFieldNames;
        Array1D_string cNumericFieldNames;

        Array1D_bool lAlphaBlanks;
        Array1D_bool lNumericBlanks;

        Array1D<Real64> NumArray;

        Real64 avdata;

        int MaxNums;
        int MaxAlphas;
        int NumParams;
        int NumNums;
        int NumAlphas;
        int status;
        int Numid;

        int HAMTitems;
        int vtcsid;

        bool ErrorsFound;

        auto &s_ip = state.dataInputProcessing->inputProcessor;
        auto &s_mat = state.dataMaterial;

        state.dataHeatBalHAMTMgr->watertot.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->surfrh.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->surfextrh.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->surftemp.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->surfexttemp.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->surfvp.allocate(state.dataSurface->TotSurfaces);

        state.dataHeatBalHAMTMgr->firstcell.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->lastcell.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->Extcell.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->ExtRadcell.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->ExtConcell.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->ExtSkycell.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->ExtGrncell.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->Intcell.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->IntConcell.allocate(state.dataSurface->TotSurfaces);

        state.dataHeatBalHAMTMgr->extvtc.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->intvtc.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->extvtcflag.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->intvtcflag.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalHAMTMgr->MyEnvrnFlag.allocate(state.dataSurface->TotSurfaces);

        state.dataHeatBalHAMTMgr->extvtc = -1.0;
        state.dataHeatBalHAMTMgr->intvtc = -1.0;
        state.dataHeatBalHAMTMgr->extvtcflag = false;
        state.dataHeatBalHAMTMgr->intvtcflag = false;
        state.dataHeatBalHAMTMgr->MyEnvrnFlag = true;

        state.dataHeatBalHAMTMgr->latswitch = true;
        state.dataHeatBalHAMTMgr->rainswitch = true;

        MaxAlphas = 0;
        MaxNums = 0;
        s_ip->getObjectDefMaxArgs(state, cHAMTObject1, NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNums = max(MaxNums, NumNums);
        s_ip->getObjectDefMaxArgs(state, cHAMTObject2, NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNums = max(MaxNums, NumNums);
        s_ip->getObjectDefMaxArgs(state, cHAMTObject3, NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNums = max(MaxNums, NumNums);
        s_ip->getObjectDefMaxArgs(state, cHAMTObject4, NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNums = max(MaxNums, NumNums);
        s_ip->getObjectDefMaxArgs(state, cHAMTObject5, NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNums = max(MaxNums, NumNums);
        s_ip->getObjectDefMaxArgs(state, cHAMTObject6, NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNums = max(MaxNums, NumNums);
        s_ip->getObjectDefMaxArgs(state, cHAMTObject7, NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNums = max(MaxNums, NumNums);

        ErrorsFound = false;

        AlphaArray.allocate(MaxAlphas);
        cAlphaFieldNames.allocate(MaxAlphas);
        cNumericFieldNames.allocate(MaxNums);
        NumArray.dimension(MaxNums, 0.0);
        lAlphaBlanks.dimension(MaxAlphas, false);
        lNumericBlanks.dimension(MaxNums, false);

        HAMTitems = s_ip->getNumObjectsFound(state, cHAMTObject1); // MaterialProperty:HeatAndMoistureTransfer:Settings
        for (int item = 1; item <= HAMTitems; ++item) {
            s_ip->getObjectItem(state,
                                cHAMTObject1,
                                item,
                                AlphaArray,
                                NumAlphas,
                                NumArray,
                                NumNums,
                                status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFieldNames,
                                cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cHAMTObject1, AlphaArray(1)};
            int matNum = Material::GetMaterialNum(state, AlphaArray(1));

            if (matNum == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(1), AlphaArray(1));
                ShowContinueError(state, "The basic material must be defined in addition to specifying HeatAndMoistureTransfer properties.");
                ErrorsFound = true;
                continue;
            }

            auto *mat = s_mat->materials(matNum);

            if (mat->group != Material::Group::Regular) {
                ShowSevereCustom(state, eoh, format("{} = \"{}\" is not a regular material.", cAlphaFieldNames(1), AlphaArray(1)));
                ErrorsFound = true;
                continue;
            }

            if (mat->ROnly) {
                ShowWarningError(state,
                                 format("{} {}=\"{}\" is defined as an R-only value material.", cHAMTObject1, cAlphaFieldNames(1), AlphaArray(1)));
                continue;
            }

            auto *matHAMT = new MaterialHAMT;
            matHAMT->Material::MaterialBase::operator=(*mat); // deep copy

            delete mat;
            s_mat->materials(matNum) = matHAMT;

            matHAMT->hasHAMT = true;
            matHAMT->Porosity = NumArray(1);
            matHAMT->iwater = NumArray(2);
        }

        HAMTitems = s_ip->getNumObjectsFound(state, cHAMTObject2); // MaterialProperty:HeatAndMoistureTransfer:SorptionIsotherm
        for (int item = 1; item <= HAMTitems; ++item) {
            s_ip->getObjectItem(state,
                                cHAMTObject2,
                                item,
                                AlphaArray,
                                NumAlphas,
                                NumArray,
                                NumNums,
                                status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFieldNames,
                                cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cHAMTObject2, AlphaArray(1)};
            int matNum = Material::GetMaterialNum(state, AlphaArray(1));

            if (matNum == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(1), AlphaArray(1));
                ShowContinueError(state, "The basic material must be defined in addition to specifying HeatAndMoistureTransfer properties.");
                ErrorsFound = true;
                continue;
            }

            auto *mat = s_mat->materials(matNum);
            if (!mat->hasHAMT) {
                ShowSevereCustom(state, eoh, format("{} is not defined for {} = \"{}\"", cHAMTObject1, cAlphaFieldNames(1), AlphaArray(1)));
                ErrorsFound = true;
                continue;
            }

            auto *matHAMT = dynamic_cast<MaterialHAMT *>(mat);
            assert(matHAMT != nullptr);

            Numid = 1;

            matHAMT->niso = int(NumArray(Numid));

            for (int iso = 1; iso <= matHAMT->niso; ++iso) {
                matHAMT->isorh(iso) = NumArray(++Numid);
                matHAMT->isodata(iso) = NumArray(++Numid);
            }

            ++matHAMT->niso;
            matHAMT->isorh(matHAMT->niso) = rhmax;
            matHAMT->isodata(matHAMT->niso) = matHAMT->Porosity * wdensity;

            ++matHAMT->niso;
            matHAMT->isorh(matHAMT->niso) = 0.0;
            matHAMT->isodata(matHAMT->niso) = 0.0;

            // check the isotherm

            // - First sort
            for (int jj = 1; jj <= matHAMT->niso - 1; ++jj) {
                for (int ii = jj + 1; ii <= matHAMT->niso; ++ii) {
                    if (matHAMT->isorh(jj) > matHAMT->isorh(ii)) {

                        Real64 dumrh = matHAMT->isorh(jj);
                        Real64 dumdata = matHAMT->isodata(jj);

                        matHAMT->isorh(jj) = matHAMT->isorh(ii);
                        matHAMT->isodata(jj) = matHAMT->isodata(ii);

                        matHAMT->isorh(ii) = dumrh;
                        matHAMT->isodata(ii) = dumdata;
                    }
                }
            }

            //- Now make sure the data rises
            bool isoerrrise = false;
            for (int ii = 1; ii <= 100; ++ii) {
                bool avflag = true;
                for (int jj = 1; jj <= matHAMT->niso - 1; ++jj) {
                    if (matHAMT->isodata(jj) > matHAMT->isodata(jj + 1)) {
                        isoerrrise = true;
                        avdata = (matHAMT->isodata(jj) + matHAMT->isodata(jj + 1)) / 2.0;
                        matHAMT->isodata(jj) = avdata;
                        matHAMT->isodata(jj + 1) = avdata;
                        avflag = false;
                    }
                }
                if (avflag) {
                    break;
                }
            }
            if (isoerrrise) {
                ShowWarningError(state, format("{}: data not rising - Check material {}", cHAMTObject2, matHAMT->Name));
                ShowContinueError(state, "Isotherm data has been fixed, and the simulation continues.");
            }
        }

        HAMTitems = s_ip->getNumObjectsFound(state, cHAMTObject3); // MaterialProperty:HeatAndMoistureTransfer:Suction
        for (int item = 1; item <= HAMTitems; ++item) {
            s_ip->getObjectItem(state,
                                cHAMTObject3,
                                item,
                                AlphaArray,
                                NumAlphas,
                                NumArray,
                                NumNums,
                                status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFieldNames,
                                cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cHAMTObject3, AlphaArray(1)};
            int matNum = Material::GetMaterialNum(state, AlphaArray(1));

            if (matNum == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(1), AlphaArray(1));
                ShowContinueError(state, "The basic material must be defined in addition to specifying HeatAndMoistureTransfer properties.");
                ErrorsFound = true;
                continue;
            }

            auto *mat = s_mat->materials(matNum);
            if (!mat->hasHAMT) {
                ShowSevereCustom(state, eoh, format("{} is not defined for {} = \"{}\"", cHAMTObject1, cAlphaFieldNames(1), AlphaArray(1)));
                ErrorsFound = true;
                continue;
            }

            auto *matHAMT = dynamic_cast<MaterialHAMT *>(mat);
            assert(matHAMT != nullptr);

            Numid = 1;

            matHAMT->nsuc = NumArray(Numid);
            for (int suc = 1; suc <= matHAMT->nsuc; ++suc) {
                matHAMT->sucwater(suc) = NumArray(++Numid);
                matHAMT->sucdata(suc) = NumArray(++Numid);
            }

            ++matHAMT->nsuc;
            matHAMT->sucwater(matHAMT->nsuc) = matHAMT->isodata(matHAMT->niso);
            matHAMT->sucdata(matHAMT->nsuc) = matHAMT->sucdata(matHAMT->nsuc - 1);
        }

        HAMTitems = s_ip->getNumObjectsFound(state, cHAMTObject4); // MaterialProperty:HeatAndMoistureTransfer:Redistribution
        for (int item = 1; item <= HAMTitems; ++item) {
            s_ip->getObjectItem(state,
                                cHAMTObject4,
                                item,
                                AlphaArray,
                                NumAlphas,
                                NumArray,
                                NumNums,
                                status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFieldNames,
                                cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cHAMTObject4, AlphaArray(1)};
            int matNum = Material::GetMaterialNum(state, AlphaArray(1));
            if (matNum == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(1), AlphaArray(1));
                ShowContinueError(state, "The basic material must be defined in addition to specifying HeatAndMoistureTransfer properties.");
                ErrorsFound = true;
                continue;
            }

            auto *mat = s_mat->materials(matNum);
            if (!mat->hasHAMT) {
                ShowSevereCustom(state, eoh, format("{} is not defined for {} = \"{}\"", cHAMTObject1, cAlphaFieldNames(1), AlphaArray(1)));
                ErrorsFound = true;
                continue;
            }

            auto *matHAMT = dynamic_cast<MaterialHAMT *>(mat);
            assert(matHAMT != nullptr);

            Numid = 1;

            matHAMT->nred = NumArray(Numid);
            for (int red = 1; red <= matHAMT->nred; ++red) {
                matHAMT->redwater(red) = NumArray(++Numid);
                matHAMT->reddata(red) = NumArray(++Numid);
            }

            ++matHAMT->nred;
            matHAMT->redwater(matHAMT->nred) = matHAMT->isodata(matHAMT->niso);
            matHAMT->reddata(matHAMT->nred) = matHAMT->reddata(matHAMT->nred - 1);
        }

        HAMTitems = s_ip->getNumObjectsFound(state, cHAMTObject5); // MaterialProperty:HeatAndMoistureTransfer:Diffusion
        for (int item = 1; item <= HAMTitems; ++item) {
            s_ip->getObjectItem(state,
                                cHAMTObject5,
                                item,
                                AlphaArray,
                                NumAlphas,
                                NumArray,
                                NumNums,
                                status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFieldNames,
                                cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cHAMTObject5, AlphaArray(1)};
            int matNum = Material::GetMaterialNum(state, AlphaArray(1));
            if (matNum == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(1), AlphaArray(1));
                ShowContinueError(state, "The basic material must be defined in addition to specifying HeatAndMoistureTransfer properties.");
                ErrorsFound = true;
                continue;
            }

            auto *mat = s_mat->materials(matNum);
            if (!mat->hasHAMT) {
                ShowSevereCustom(state, eoh, format("{} is not defined for {} = \"{}\"", cHAMTObject1, cAlphaFieldNames(1), AlphaArray(1)));
                ErrorsFound = true;
                continue;
            }

            auto *matHAMT = dynamic_cast<MaterialHAMT *>(mat);
            assert(matHAMT != nullptr);

            Numid = 1;

            matHAMT->nmu = NumArray(Numid);
            if (matHAMT->nmu > 0) {
                for (int mu = 1; mu <= matHAMT->nmu; ++mu) {
                    matHAMT->murh(mu) = NumArray(++Numid);
                    matHAMT->mudata(mu) = NumArray(++Numid);
                }

                ++matHAMT->nmu;
                matHAMT->murh(matHAMT->nmu) = matHAMT->isorh(matHAMT->niso);
                matHAMT->mudata(matHAMT->nmu) = matHAMT->mudata(matHAMT->nmu - 1);
            }
        }

        HAMTitems = s_ip->getNumObjectsFound(state, cHAMTObject6); // MaterialProperty:HeatAndMoistureTransfer:ThermalConductivity
        for (int item = 1; item <= HAMTitems; ++item) {
            s_ip->getObjectItem(state,
                                cHAMTObject6,
                                item,
                                AlphaArray,
                                NumAlphas,
                                NumArray,
                                NumNums,
                                status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFieldNames,
                                cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cHAMTObject6, AlphaArray(1)};
            int matNum = Material::GetMaterialNum(state, AlphaArray(1));
            if (matNum == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(1), AlphaArray(1));
                ShowContinueError(state, "The basic material must be defined in addition to specifying HeatAndMoistureTransfer properties.");
                ErrorsFound = true;
                continue;
            }

            auto *mat = s_mat->materials(matNum);
            if (!mat->hasHAMT) {
                ShowSevereCustom(state, eoh, format("{} is not defined for {} = \"{}\"", cHAMTObject1, cAlphaFieldNames(1), AlphaArray(1)));
                ErrorsFound = true;
                continue;
            }

            auto *matHAMT = dynamic_cast<MaterialHAMT *>(mat);
            assert(matHAMT != nullptr);

            Numid = 1;

            matHAMT->ntc = NumArray(Numid);
            if (matHAMT->ntc > 0) {
                for (int tc = 1; tc <= matHAMT->ntc; ++tc) {
                    ++Numid;
                    matHAMT->tcwater(tc) = NumArray(Numid);
                    ++Numid;
                    matHAMT->tcdata(tc) = NumArray(Numid);
                }

                ++matHAMT->ntc;
                matHAMT->tcwater(matHAMT->ntc) = matHAMT->isodata(matHAMT->niso);
                matHAMT->tcdata(matHAMT->ntc) = matHAMT->tcdata(matHAMT->ntc - 1);
            }
        }

        // Vapor Transfer coefficients
        HAMTitems = s_ip->getNumObjectsFound(state, cHAMTObject7); // SurfaceProperties:VaporCoefficients
        for (int item = 1; item <= HAMTitems; ++item) {
            s_ip->getObjectItem(state,
                                cHAMTObject7,
                                item,
                                AlphaArray,
                                NumAlphas,
                                NumArray,
                                NumNums,
                                status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFieldNames,
                                cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cHAMTObject7, AlphaArray(1)};
            vtcsid = Util::FindItemInList(AlphaArray(1), state.dataSurface->Surface);
            if (vtcsid == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(1), AlphaArray(1));
                ShowContinueError(state, "The basic material must be defined in addition to specifying HeatAndMoistureTransfer properties.");
                ErrorsFound = true;
                continue;
            }

            if (AlphaArray(2) == "YES") {
                state.dataHeatBalHAMTMgr->extvtcflag(vtcsid) = true;
                state.dataHeatBalHAMTMgr->extvtc(vtcsid) = NumArray(1);
            }

            if (AlphaArray(3) == "YES") {
                state.dataHeatBalHAMTMgr->intvtcflag(vtcsid) = true;
                state.dataHeatBalHAMTMgr->intvtc(vtcsid) = NumArray(2);
            }
        }

        AlphaArray.deallocate();
        cAlphaFieldNames.deallocate();
        cNumericFieldNames.deallocate();
        NumArray.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, "GetHeatBalHAMTInput: Errors found getting input.  Program terminates.");
        }
    }

    void InitHeatBalHAMT(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Phillip Biddulph
        //       DATE WRITTEN   June 2008
        //       MODIFIED       B. Griffith, Aug 2012 for surface-specific algorithms
        //       RE-ENGINEERED  na

        // Using/Aliasing
        using General::ScanForReports;

        // Locals
        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr adjdist(0.00005); // Allowable distance between two cells, also used as limit on cell length
        static constexpr std::string_view RoutineName("InitCombinedHeatAndMoistureFiniteElement: ");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int sid;
        int conid;
        int errorCount;

        Real64 runor;
        Real64 testlen;
        Real64 waterd; // water density
        bool DoReport;

        auto &s_mat = state.dataMaterial;
        auto &s_hbh = state.dataHeatBalHAMTMgr;

        s_hbh->deltat = state.dataGlobal->TimeStepZone * 3600.0;

        // Check the materials information and work out how many cells are required.
        errorCount = 0;
        s_hbh->TotCellsMax = 0;
        for (int sid = 1; sid <= state.dataSurface->TotSurfaces; ++sid) {
            auto const &surf = state.dataSurface->Surface(sid);
            if (surf.Class == SurfaceClass::Window) {
                continue;
            }
            if (surf.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::HAMT) {
                continue;
            }

            if (surf.Construction == 0) {
                continue;
            }
            auto const &constr = state.dataConstruction->Construct(surf.Construction);

            for (int lid = 1; lid <= constr.TotLayers; ++lid) {
                auto *mat = s_mat->materials(constr.LayerPoint(lid));
                if (mat->ROnly) {
                    ShowSevereError(state, format("{}Construction={} cannot contain R-only value materials.", RoutineName, constr.Name));
                    ShowContinueError(state, format("Reference Material=\"{}\".", mat->Name));
                    ++errorCount;
                    continue;
                }

                auto *matHAMT = dynamic_cast<MaterialHAMT *>(mat);
                assert(matHAMT != nullptr);

                if (matHAMT->nmu < 0) {
                    ShowSevereError(state, format("{}Construction={}", RoutineName, constr.Name));
                    ShowContinueError(
                        state,
                        format("Reference Material=\"{}\" does not have required Water Vapor Diffusion Resistance Factor (mu) data.", matHAMT->Name));
                    ++errorCount;
                }

                if (matHAMT->niso < 0) {
                    ShowSevereError(state, format("{}Construction={}", RoutineName, constr.Name));
                    ShowContinueError(state, format("Reference Material=\"{}\" does not have required isotherm data.", matHAMT->Name));
                    ++errorCount;
                }
                if (matHAMT->nsuc < 0) {
                    ShowSevereError(state, format("{}Construction={}", RoutineName, constr.Name));
                    ShowContinueError(
                        state, format("Reference Material=\"{}\" does not have required liquid transport coefficient (suction) data.", mat->Name));
                    ++errorCount;
                }
                if (matHAMT->nred < 0) {
                    ShowSevereError(state, format("{}Construction={}", RoutineName, constr.Name));
                    ShowContinueError(
                        state,
                        format("Reference Material=\"{}\" does not have required liquid transport coefficient (redistribution) data.", mat->Name));
                    ++errorCount;
                }
                if (matHAMT->ntc < 0) {
                    if (mat->Conductivity > 0) {
                        ShowWarningError(state, format("{}Construction={}", RoutineName, constr.Name));
                        ShowContinueError(
                            state, format("Reference Material=\"{}\" does not have thermal conductivity data. Using fixed value.", matHAMT->Name));
                        matHAMT->ntc = 2;
                        matHAMT->tcwater(1) = 0.0;
                        matHAMT->tcdata(1) = matHAMT->Conductivity;
                        matHAMT->tcwater(2) = matHAMT->isodata(matHAMT->niso);
                        matHAMT->tcdata(2) = matHAMT->Conductivity;
                    } else {
                        ShowSevereError(state, format("{}Construction={}", RoutineName, constr.Name));
                        ShowContinueError(state,
                                          format("Reference Material=\"{}\" does not have required thermal conductivity data.", matHAMT->Name));
                        ++errorCount;
                    }
                }

                // convert material water content to RH

                waterd = matHAMT->iwater * matHAMT->Density;
                interp(matHAMT->niso, matHAMT->isodata, matHAMT->isorh, waterd, matHAMT->irh);

                matHAMT->divs = int(matHAMT->Thickness / matHAMT->divsize) + matHAMT->divmin;
                if (matHAMT->divs > matHAMT->divmax) {
                    matHAMT->divs = matHAMT->divmax;
                }
                // Check length of cell - reduce number of divisions if necessary
                Real64 const sin_negPIOvr2 = std::sin(-Constant::Pi / 2.0);
                while (true) {
                    testlen = matHAMT->Thickness *
                              ((std::sin(Constant::Pi * (-1.0 / double(matHAMT->divs)) - Constant::Pi / 2.0) / 2.0) - (sin_negPIOvr2 / 2.0));
                    if (testlen > adjdist) {
                        break;
                    }
                    --matHAMT->divs;
                    if (matHAMT->divs < 1) {
                        ShowSevereError(state, format("{}Construction={}", RoutineName, constr.Name));
                        ShowContinueError(state, format("Reference Material=\"{}\" is too thin.", matHAMT->Name));
                        ++errorCount;
                        break;
                    }
                }
                s_hbh->TotCellsMax += matHAMT->divs;
            }
            s_hbh->TotCellsMax += 7;
        }

        if (errorCount > 0) {
            ShowFatalError(state, "CombinedHeatAndMoistureFiniteElement: Incomplete data to start solution, program terminates.");
        }

        // Make the cells and initialize
        s_hbh->cells.allocate(s_hbh->TotCellsMax);
        for (auto &e : s_hbh->cells) {
            e.adjs = -1;
            e.adjsl = -1;
        }

        int cid = 0;

        // Set up surface cell structure
        for (int sid = 1; sid <= state.dataSurface->TotSurfaces; ++sid) {
            auto &surf = state.dataSurface->Surface(sid);
            if (!surf.HeatTransSurf) {
                continue;
            }
            if (surf.Class == SurfaceClass::Window) {
                continue;
            }
            if (surf.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::HAMT) {
                continue;
            }
            // Boundary Cells
            runor = -0.02;
            // Air Convection Cell
            ++cid;
            s_hbh->firstcell(sid) = cid;
            s_hbh->ExtConcell(sid) = cid;
            auto &airConvCell = s_hbh->cells(cid);
            airConvCell.rh = 0.0;
            airConvCell.sid = sid;
            airConvCell.length(1) = 0.01;
            airConvCell.origin(1) = airConvCell.length(1) / 2.0 + runor;

            // Air Radiation Cell
            ++cid;
            s_hbh->ExtRadcell(sid) = cid;
            auto &airRadCell = s_hbh->cells(cid);
            airRadCell.rh = 0.0;
            airRadCell.sid = sid;
            airRadCell.length(1) = 0.01;
            airRadCell.origin(1) = airRadCell.length(1) / 2.0 + runor;

            // Sky Cell
            ++cid;
            s_hbh->ExtSkycell(sid) = cid;
            auto &skyCell = s_hbh->cells(cid);
            skyCell.rh = 0.0;
            skyCell.sid = sid;
            skyCell.length(1) = 0.01;
            skyCell.origin(1) = skyCell.length(1) / 2.0 + runor;

            // Ground Cell
            ++cid;
            s_hbh->ExtGrncell(sid) = cid;
            auto &groundCell = s_hbh->cells(cid);
            groundCell.rh = 0.0;
            groundCell.sid = sid;
            groundCell.length(1) = 0.01;
            groundCell.origin(1) = groundCell.length(1) / 2.0 + runor;
            runor += groundCell.length(1);

            // External Virtual Cell
            ++cid;
            s_hbh->Extcell(sid) = cid;
            auto &extVirtCell = s_hbh->cells(cid);
            extVirtCell.rh = 0.0;
            extVirtCell.sid = sid;
            extVirtCell.length(1) = 0.01;
            extVirtCell.origin(1) = extVirtCell.length(1) / 2.0 + runor;
            runor += extVirtCell.length(1);

            // Material Cells
            auto const &constr = state.dataConstruction->Construct(surf.Construction);
            for (int lid = 1; lid <= constr.TotLayers; ++lid) {
                auto const *mat = dynamic_cast<const MaterialHAMT *>(s_mat->materials(constr.LayerPoint(lid)));
                assert(mat != nullptr);

                for (int did = 1; did <= mat->divs; ++did) {
                    ++cid;

                    auto &matCell = s_hbh->cells(cid);
                    matCell.matid = mat->Num;
                    matCell.sid = sid;

                    matCell.temp = mat->itemp;
                    matCell.tempp1 = mat->itemp;
                    matCell.tempp2 = mat->itemp;

                    matCell.rh = mat->irh;
                    matCell.rhp1 = mat->irh;
                    matCell.rhp2 = mat->irh;

                    matCell.density = mat->Density;
                    matCell.spech = mat->SpecHeat;

                    // Make cells smaller near the surface
                    matCell.length(1) =
                        mat->Thickness * ((std::sin(Constant::Pi * (-double(did) / double(mat->divs)) - Constant::Pi / 2.0) / 2.0) -
                                          (std::sin(Constant::Pi * (-double(did - 1) / double(mat->divs)) - Constant::Pi / 2.0) / 2.0));

                    matCell.origin(1) = runor + matCell.length(1) / 2.0;
                    runor += matCell.length(1);

                    matCell.volume = matCell.length(1) * state.dataSurface->Surface(sid).Area;
                }
            }

            // Interior Virtual Cell
            ++cid;
            s_hbh->Intcell(sid) = cid;
            auto &intVirtCell = s_hbh->cells(cid);
            intVirtCell.sid = sid;
            intVirtCell.rh = 0.0;
            intVirtCell.length(1) = 0.01;
            intVirtCell.origin(1) = intVirtCell.length(1) / 2.0 + runor;
            runor += intVirtCell.length(1);

            // Air Convection Cell
            ++cid;
            s_hbh->lastcell(sid) = cid;
            s_hbh->IntConcell(sid) = cid;
            auto &airConvCell2 = s_hbh->cells(cid);
            airConvCell2.rh = 0.0;
            airConvCell2.sid = sid;
            airConvCell2.length(1) = 0.01;
            airConvCell2.origin(1) = airConvCell2.length(1) / 2.0 + runor;
        }

        // Find adjacent cells.
        for (int cid1 = 1; cid1 <= s_hbh->TotCellsMax; ++cid1) {
            for (int cid2 = 1; cid2 <= s_hbh->TotCellsMax; ++cid2) {
                if (cid1 == cid2) {
                    continue;
                }

                auto &cell1 = s_hbh->cells(cid1);
                auto &cell2 = s_hbh->cells(cid2);

                if (cell1.sid != cell2.sid) {
                    continue;
                }

                Real64 high1 = cell1.origin(1) + cell1.length(1) / 2.0;
                Real64 low2 = cell2.origin(1) - cell2.length(1) / 2.0;
                if (std::abs(low2 - high1) < adjdist) {
                    int adj1 = 0;
                    for (int ii = 1; ii <= adjmax; ++ii) {
                        ++adj1;
                        if (cell1.adjs(adj1) == -1) {
                            break;
                        }
                    }
                    int adj2 = 0;
                    for (int ii = 1; ii <= adjmax; ++ii) {
                        ++adj2;
                        if (cell2.adjs(adj2) == -1) {
                            break;
                        }
                    }
                    cell1.adjs(adj1) = cid2;
                    cell2.adjs(adj2) = cid1;

                    cell1.adjsl(adj1) = adj2;
                    cell2.adjsl(adj2) = adj1;

                    sid = cell1.sid;
                    cell1.overlap(adj1) = state.dataSurface->Surface(sid).Area;
                    cell2.overlap(adj2) = state.dataSurface->Surface(sid).Area;
                    cell1.dist(adj1) = cell1.length(1) / 2.0;
                    cell2.dist(adj2) = cell2.length(1) / 2.0;
                }
            }
        }

        // Reset surface virtual cell origins and volumes. Initialize report variables.
        static constexpr std::string_view Format_1966("! <HAMT cells>, Surface Name, Construction Name, Cell Numbers\n");
        print(state.files.eio, Format_1966);
        static constexpr std::string_view Format_1965("! <HAMT origins>, Surface Name, Construction Name, Cell origins (m) \n");
        print(state.files.eio, Format_1965);
        // cCurrentModuleObject='MaterialProperty:HeatAndMoistureTransfer:*'
        for (int sid = 1; sid <= state.dataSurface->TotSurfaces; ++sid) {
            if (!state.dataSurface->Surface(sid).HeatTransSurf) {
                continue;
            }
            if (state.dataSurface->Surface(sid).Class == SurfaceClass::Window) {
                continue;
            }
            if (state.dataSurface->Surface(sid).HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::HAMT) {
                continue;
            }
            s_hbh->cells(s_hbh->Extcell(sid)).origin(1) += s_hbh->cells(s_hbh->Extcell(sid)).length(1) / 2.0;
            s_hbh->cells(s_hbh->Intcell(sid)).origin(1) -= s_hbh->cells(s_hbh->Intcell(sid)).length(1) / 2.0;
            s_hbh->cells(s_hbh->Extcell(sid)).volume = 0.0;
            s_hbh->cells(s_hbh->Intcell(sid)).volume = 0.0;
            s_hbh->watertot(sid) = 0.0;
            s_hbh->surfrh(sid) = 0.0;
            s_hbh->surfextrh(sid) = 0.0;
            s_hbh->surftemp(sid) = 0.0;
            s_hbh->surfexttemp(sid) = 0.0;
            s_hbh->surfvp(sid) = 0.0;
            SetupOutputVariable(state,
                                "HAMT Surface Average Water Content Ratio",
                                Constant::Units::kg_kg,
                                s_hbh->watertot(sid),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataSurface->Surface(sid).Name);
            SetupOutputVariable(state,
                                "HAMT Surface Inside Face Temperature",
                                Constant::Units::C,
                                s_hbh->surftemp(sid),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataSurface->Surface(sid).Name);
            SetupOutputVariable(state,
                                "HAMT Surface Inside Face Relative Humidity",
                                Constant::Units::Perc,
                                s_hbh->surfrh(sid),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataSurface->Surface(sid).Name);
            SetupOutputVariable(state,
                                "HAMT Surface Inside Face Vapor Pressure",
                                Constant::Units::Pa,
                                s_hbh->surfvp(sid),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataSurface->Surface(sid).Name);
            SetupOutputVariable(state,
                                "HAMT Surface Outside Face Temperature",
                                Constant::Units::C,
                                s_hbh->surfexttemp(sid),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataSurface->Surface(sid).Name);
            SetupOutputVariable(state,
                                "HAMT Surface Outside Face Relative Humidity",
                                Constant::Units::Perc,
                                s_hbh->surfextrh(sid),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataSurface->Surface(sid).Name);

            // write cell origins to initialization output file
            conid = state.dataSurface->Surface(sid).Construction;
            print(state.files.eio, "HAMT cells, {},{}", state.dataSurface->Surface(sid).Name, state.dataConstruction->Construct(conid).Name);
            for (int concell = 1, concell_end = s_hbh->Intcell(sid) - s_hbh->Extcell(sid) + 1; concell <= concell_end; ++concell) {
                print(state.files.eio, ",{:4}", concell);
            }
            print(state.files.eio, "\n");
            print(state.files.eio, "HAMT origins,{},{}", state.dataSurface->Surface(sid).Name, state.dataConstruction->Construct(conid).Name);
            for (int cellid = s_hbh->Extcell(sid); cellid <= s_hbh->Intcell(sid); ++cellid) {
                print(state.files.eio, ",{:10.7F}", s_hbh->cells(cellid).origin(1));
            }
            print(state.files.eio, "\n");

            for (int cellid = s_hbh->Extcell(sid), concell = 1; cellid <= s_hbh->Intcell(sid); ++cellid, ++concell) {
                SetupOutputVariable(state,
                                    format("HAMT Surface Temperature Cell {}", concell),
                                    Constant::Units::C,
                                    s_hbh->cells(cellid).temp,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataSurface->Surface(sid).Name);
            }
            for (int cellid = s_hbh->Extcell(sid), concell = 1; cellid <= s_hbh->Intcell(sid); ++cellid, ++concell) {
                SetupOutputVariable(state,
                                    format("HAMT Surface Water Content Cell {}", concell),
                                    Constant::Units::kg_kg,
                                    s_hbh->cells(cellid).wreport,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataSurface->Surface(sid).Name);
            }
            for (int cellid = s_hbh->Extcell(sid), concell = 1; cellid <= s_hbh->Intcell(sid); ++cellid, ++concell) {
                SetupOutputVariable(state,
                                    format("HAMT Surface Relative Humidity Cell {}", concell),
                                    Constant::Units::Perc,
                                    s_hbh->cells(cellid).rhp,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataSurface->Surface(sid).Name);
            }
        }

        ScanForReports(state, "Constructions", DoReport, "Constructions");
        if (DoReport) {

            static constexpr std::string_view Format_108("! <Material Nominal Resistance>, Material Name,  Nominal R\n");
            print(state.files.eio, Format_108);

            for (auto const *mat : s_mat->materials) {
                static constexpr std::string_view Format_111("Material Nominal Resistance,{},{:.4R}\n");
                print(state.files.eio, Format_111, mat->Name, mat->NominalR);
            }
        }
    }

    void CalcHeatBalHAMT(EnergyPlusData &state, int const sid, Real64 &SurfTempInTmp, Real64 &TempSurfOutTmp)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Phillip Biddulph
        //       DATE WRITTEN   June 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // To calculate the heat and moisture transfer through the surface

        // Using/Aliasing
        using DataSurfaces::OtherSideCondModeledExt;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const HAMTExt("HAMT-Ext");
        static std::string const HAMTInt("HAMT-Int");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 SurfTempInP;
        Real64 RhoIn;
        Real64 RhoOut;
        Real64 torsum;
        Real64 oorsum;
        Real64 phioosum;
        Real64 phiorsum;
        Real64 vpoosum;
        Real64 vporsum;
        Real64 rhr1;
        Real64 rhr2;
        Real64 wcap;
        Real64 thermr1;
        Real64 thermr2;
        Real64 tcap;
        Real64 qvp;
        Real64 vaporr1;
        Real64 vaporr2;
        Real64 vpdiff;
        Real64 sumtp1;
        Real64 tempmax;
        Real64 tempmin;

        int itter;

        Real64 denominator;

        auto &s_mat = state.dataMaterial;
        auto &s_hbh = state.dataHeatBalHAMTMgr;

        if (state.dataGlobal->BeginEnvrnFlag && s_hbh->MyEnvrnFlag(sid)) {
            auto &extCell = s_hbh->cells(s_hbh->Extcell(sid));
            extCell.rh = 0.0;
            extCell.rhp1 = 0.0;
            extCell.rhp2 = 0.0;

            extCell.temp = 10.0;
            extCell.tempp1 = 10.0;
            extCell.tempp2 = 10.0;

            auto &intCell = s_hbh->cells(s_hbh->Intcell(sid));
            intCell.rh = 0.0;
            intCell.rhp1 = 0.0;
            intCell.rhp2 = 0.0;

            intCell.temp = 10.0;
            intCell.tempp1 = 10.0;
            intCell.tempp2 = 10.0;

            for (int cid = s_hbh->Extcell(sid) + 1; cid <= s_hbh->Intcell(sid) - 1; ++cid) {
                auto &cell = s_hbh->cells(cid);
                auto const *mat = dynamic_cast<const MaterialHAMT *>(s_mat->materials(cell.matid));
                assert(mat != nullptr);
                cell.temp = mat->itemp;
                cell.tempp1 = mat->itemp;
                cell.tempp2 = mat->itemp;

                cell.rh = mat->irh;
                cell.rhp1 = mat->irh;
                cell.rhp2 = mat->irh;
            }
            s_hbh->MyEnvrnFlag(sid) = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            s_hbh->MyEnvrnFlag(sid) = true;
        }

        auto &extCell = s_hbh->cells(s_hbh->Extcell(sid));
        auto &extRadCell = s_hbh->cells(s_hbh->ExtRadcell(sid));
        auto &extSkyCell = s_hbh->cells(s_hbh->ExtSkycell(sid));
        auto &extGrnCell = s_hbh->cells(s_hbh->ExtGrncell(sid));
        auto &extConCell = s_hbh->cells(s_hbh->ExtConcell(sid));

        // Set all the boundary values
        extRadCell.temp = state.dataMstBal->TempOutsideAirFD(sid);
        extConCell.temp = state.dataMstBal->TempOutsideAirFD(sid);
        Real64 spaceMAT = state.dataZoneTempPredictorCorrector->spaceHeatBalance(state.dataSurface->Surface(sid).spaceNum).MAT;
        if (state.dataSurface->Surface(sid).ExtBoundCond == OtherSideCondModeledExt) {
            // CR8046 switch modeled rad temp for sky temp.
            extSkyCell.temp = state.dataSurface->OSCM(state.dataSurface->Surface(sid).OSCMPtr).TRad;
            extCell.Qadds = 0.0; // eliminate incident shortwave on underlying surface
        } else {
            extSkyCell.temp = state.dataEnvrn->SkyTemp;
            extCell.Qadds = state.dataSurface->Surface(sid).Area * state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(sid);
        }

        extGrnCell.temp = state.dataMstBal->TempOutsideAirFD(sid);
        RhoOut = state.dataMstBal->RhoVaporAirOut(sid);

        // Special case when the surface is an internal mass
        if (state.dataSurface->Surface(sid).ExtBoundCond == sid) {
            extConCell.temp = spaceMAT;
            RhoOut = state.dataMstBal->RhoVaporAirIn(sid);
        }

        RhoIn = state.dataMstBal->RhoVaporAirIn(sid);

        extRadCell.htc = state.dataMstBal->HAirFD(sid);
        extConCell.htc = state.dataMstBal->HConvExtFD(sid);
        extSkyCell.htc = state.dataMstBal->HSkyFD(sid);
        extGrnCell.htc = state.dataMstBal->HGrndFD(sid);

        auto &intCell = s_hbh->cells(s_hbh->Intcell(sid));
        auto &intConCell = s_hbh->cells(s_hbh->IntConcell(sid));

        intConCell.temp = spaceMAT;
        intConCell.htc = state.dataMstBal->HConvInFD(sid);

        intCell.Qadds = state.dataSurface->Surface(sid).Area *
                        (state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(sid) + state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(sid) +
                         state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(sid) + state.dataHeatBal->SurfQdotRadIntGainsInPerArea(sid) +
                         state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(sid));

        extConCell.rh = PsyRhFnTdbRhov(state, extConCell.temp, RhoOut, HAMTExt);
        intConCell.rh = PsyRhFnTdbRhov(state, intConCell.temp, RhoIn, HAMTInt);

        if (extConCell.rh > rhmax) {
            extConCell.rh = rhmax;
        }
        if (intConCell.rh > rhmax) {
            intConCell.rh = rhmax;
        }

        // PDB August 2009 Start! Correction for when no vapour transfer coefficient have been defined.
        if (s_hbh->extvtcflag(sid)) {
            extConCell.vtc = s_hbh->extvtc(sid);
        } else {
            if (extConCell.rh > 0) {
                extConCell.vtc =
                    state.dataMstBal->HMassConvExtFD(sid) * RhoOut / (PsyPsatFnTemp(state, state.dataMstBal->TempOutsideAirFD(sid)) * extConCell.rh);
            } else {
                extConCell.vtc = 10000.0;
            }
        }

        if (s_hbh->intvtcflag(sid)) {
            intConCell.vtc = s_hbh->intvtc(sid);
            state.dataMstBal->HMassConvInFD(sid) = intConCell.vtc * PsyPsatFnTemp(state, spaceMAT) * intConCell.rh / RhoIn;
        } else {
            if (intConCell.rh > 0) {
                intConCell.vtc = state.dataMstBal->HMassConvInFD(sid) * RhoIn / (PsyPsatFnTemp(state, spaceMAT) * intConCell.rh);
            } else {
                intConCell.vtc = 10000.0;
            }
        }
        // PDB August 2009 End

        // Initialise
        for (int cid = s_hbh->firstcell(sid); cid <= s_hbh->Extcell(sid) - 1; ++cid) {
            auto &cell = s_hbh->cells(cid);
            cell.tempp1 = cell.temp;
            cell.tempp2 = cell.temp;
            cell.rhp1 = cell.rh;
            cell.rhp2 = cell.rh;
        }
        for (int cid = s_hbh->Intcell(sid) + 1; cid <= s_hbh->lastcell(sid); ++cid) {
            auto &cell = s_hbh->cells(cid);
            cell.tempp1 = cell.temp;
            cell.tempp2 = cell.temp;
            cell.rhp1 = cell.rh;
            cell.rhp2 = cell.rh;
        }

        itter = 0;
        while (true) {
            ++itter;
            // Update Moisture values

            for (int cid = s_hbh->firstcell(sid); cid <= s_hbh->lastcell(sid); ++cid) {
                auto &cell = s_hbh->cells(cid);
                cell.vp = RHtoVP(state, cell.rh, cell.temp);
                cell.vpp1 = RHtoVP(state, cell.rhp1, cell.tempp1);
                cell.vpsat = PsyPsatFnTemp(state, cell.tempp1);
                if (cell.matid > 0) {
                    auto const *mat = dynamic_cast<const MaterialHAMT *>(s_mat->materials(cell.matid));
                    assert(mat != nullptr);
                    interp(mat->niso, mat->isorh, mat->isodata, cell.rhp1, cell.water, cell.dwdphi);
                    if (state.dataEnvrn->IsRain && s_hbh->rainswitch) {
                        interp(mat->nsuc, mat->sucwater, mat->sucdata, cell.water, cell.dw);
                    } else {
                        interp(mat->nred, mat->redwater, mat->reddata, cell.water, cell.dw);
                    }
                    interp(mat->nmu, mat->murh, mat->mudata, cell.rhp1, cell.mu);
                    interp(mat->ntc, mat->tcwater, mat->tcdata, cell.water, cell.wthermalc);
                }
            }

            // Calculate Heat and Vapor resistances,
            for (int cid = s_hbh->Extcell(sid); cid <= s_hbh->Intcell(sid); ++cid) {
                torsum = 0.0;
                oorsum = 0.0;
                vpdiff = 0.0;
                auto &cell = s_hbh->cells(cid);
                for (int ii = 1; ii <= adjmax; ++ii) {
                    int adj = cell.adjs(ii);
                    int adjl = cell.adjsl(ii);
                    if (adj == -1) {
                        break;
                    }

                    if (cell.htc > 0) {
                        thermr1 = 1.0 / (cell.overlap(ii) * cell.htc);
                    } else if (cell.matid > 0) {
                        thermr1 = cell.dist(ii) / (cell.overlap(ii) * cell.wthermalc);
                    } else {
                        thermr1 = 0.0;
                    }

                    if (cell.vtc > 0) {
                        vaporr1 = 1.0 / (cell.overlap(ii) * cell.vtc);
                    } else if (cell.matid > 0) {
                        vaporr1 = (cell.dist(ii) * cell.mu) / (cell.overlap(ii) * WVDC(cell.tempp1, state.dataEnvrn->OutBaroPress));
                    } else {
                        vaporr1 = 0.0;
                    }

                    auto &adjCell = s_hbh->cells(adj);
                    if (adjCell.htc > 0) {
                        thermr2 = 1.0 / (cell.overlap(ii) * adjCell.htc);
                    } else if (adjCell.matid > 0) {
                        thermr2 = adjCell.dist(adjl) / (cell.overlap(ii) * adjCell.wthermalc);
                    } else {
                        thermr2 = 0.0;
                    }

                    if (adjCell.vtc > 0) {
                        vaporr2 = 1.0 / (cell.overlap(ii) * adjCell.vtc);
                    } else if (adjCell.matid > 0) {
                        vaporr2 = adjCell.mu * adjCell.dist(adjl) / (WVDC(adjCell.tempp1, state.dataEnvrn->OutBaroPress) * cell.overlap(ii));
                    } else {
                        vaporr2 = 0.0;
                    }

                    if (thermr1 + thermr2 > 0) {
                        oorsum += 1.0 / (thermr1 + thermr2);
                        torsum += adjCell.tempp1 / (thermr1 + thermr2);
                    }
                    if (vaporr1 + vaporr2 > 0) {
                        vpdiff += (adjCell.vp - cell.vp) / (vaporr1 + vaporr2);
                    }
                }

                // Calculate Heat Capacitance
                tcap = ((cell.density * cell.spech + cell.water * wspech) * cell.volume);

                // calculate the latent heat if wanted and check for divergence
                qvp = 0.0;
                if ((cell.matid > 0) && (s_hbh->latswitch)) {
                    qvp = vpdiff * whv;
                }
                if (std::abs(qvp) > qvplim) {
                    if (!state.dataGlobal->WarmupFlag) {
                        ++s_hbh->qvpErrCount;
                        if (s_hbh->qvpErrCount < 16) {
                            ShowWarningError(
                                state, format("HeatAndMoistureTransfer: Large Latent Heat for Surface {}", state.dataSurface->Surface(sid).Name));
                        } else {
                            ShowRecurringWarningErrorAtEnd(state, "HeatAndMoistureTransfer: Large Latent Heat Errors ", s_hbh->qvpErrReport);
                        }
                    }
                    qvp = 0.0;
                }

                // Calculate the temperature for the next time step
                cell.tempp1 = (torsum + qvp + cell.Qadds + (tcap * cell.temp / s_hbh->deltat)) / (oorsum + (tcap / s_hbh->deltat));
            }

            // Check for silly temperatures
            tempmax = maxval(s_hbh->cells, &subcell::tempp1);
            tempmin = minval(s_hbh->cells, &subcell::tempp1);
            if (tempmax > state.dataHeatBalSurf->MaxSurfaceTempLimit) {
                if (!state.dataGlobal->WarmupFlag) {
                    if (state.dataSurface->SurfHighTempErrCount(sid) == 0) {
                        ShowSevereMessage(
                            state,
                            format("HAMT: Temperature (high) out of bounds ({:.2R}) for surface={}", tempmax, state.dataSurface->Surface(sid).Name));
                        ShowContinueErrorTimeStamp(state, "");
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "HAMT: Temperature Temperature (high) out of bounds; Surface=" +
                                                       state.dataSurface->Surface(sid).Name,
                                                   state.dataSurface->SurfHighTempErrCount(sid),
                                                   tempmax,
                                                   tempmax,
                                                   _,
                                                   "C",
                                                   "C");
                }
            }
            if (tempmax > state.dataHeatBalSurf->MaxSurfaceTempLimitBeforeFatal) {
                if (!state.dataGlobal->WarmupFlag) {
                    ShowSevereError(state,
                                    format("HAMT: HAMT: Temperature (high) out of bounds ( {:.2R}) for surface={}",
                                           tempmax,
                                           state.dataSurface->Surface(sid).Name));
                    ShowContinueErrorTimeStamp(state, "");
                    ShowFatalError(state, "Program terminates due to preceding condition.");
                }
            }
            if (tempmin < MinSurfaceTempLimit) {
                if (!state.dataGlobal->WarmupFlag) {
                    if (state.dataSurface->SurfHighTempErrCount(sid) == 0) {
                        ShowSevereMessage(
                            state,
                            format("HAMT: Temperature (low) out of bounds ({:.2R}) for surface={}", tempmin, state.dataSurface->Surface(sid).Name));
                        ShowContinueErrorTimeStamp(state, "");
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "HAMT: Temperature Temperature (high) out of bounds; Surface=" +
                                                       state.dataSurface->Surface(sid).Name,
                                                   state.dataSurface->SurfHighTempErrCount(sid),
                                                   tempmin,
                                                   tempmin,
                                                   _,
                                                   "C",
                                                   "C");
                }
            }
            if (tempmin < MinSurfaceTempLimitBeforeFatal) {
                if (!state.dataGlobal->WarmupFlag) {
                    ShowSevereError(state,
                                    format("HAMT: HAMT: Temperature (low) out of bounds ( {:.2R}) for surface={}",
                                           tempmin,
                                           state.dataSurface->Surface(sid).Name));
                    ShowContinueErrorTimeStamp(state, "");
                    ShowFatalError(state, "Program terminates due to preceding condition.");
                }
            }

            // Calculate the liquid and vapor resisitances
            for (int cid = s_hbh->Extcell(sid); cid <= s_hbh->Intcell(sid); ++cid) {
                phioosum = 0.0;
                phiorsum = 0.0;
                vpoosum = 0.0;
                vporsum = 0.0;

                auto &cell = s_hbh->cells(cid);
                for (int ii = 1; ii <= adjmax; ++ii) {
                    int adj = cell.adjs(ii);
                    int adjl = cell.adjsl(ii);
                    if (adj == -1) {
                        break;
                    }

                    if (cell.vtc > 0) {
                        vaporr1 = 1.0 / (cell.overlap(ii) * cell.vtc);
                    } else if (cell.matid > 0) {
                        vaporr1 = (cell.dist(ii) * cell.mu) / (cell.overlap(ii) * WVDC(cell.tempp1, state.dataEnvrn->OutBaroPress));
                    } else {
                        vaporr1 = 0.0;
                    }

                    auto &adjCell = s_hbh->cells(adj);
                    if (adjCell.vtc > 0) {
                        vaporr2 = 1.0 / (cell.overlap(ii) * adjCell.vtc);
                    } else if (adjCell.matid > 0) {
                        vaporr2 = (adjCell.dist(adjl) * adjCell.mu) / (cell.overlap(ii) * WVDC(adjCell.tempp1, state.dataEnvrn->OutBaroPress));
                    } else {
                        vaporr2 = 0.0;
                    }
                    if (vaporr1 + vaporr2 > 0) {
                        vpoosum += 1.0 / (vaporr1 + vaporr2);
                        vporsum += (adjCell.vpp1 / (vaporr1 + vaporr2));
                    }

                    if ((cell.dw > 0) && (cell.dwdphi > 0)) {
                        rhr1 = cell.dist(ii) / (cell.overlap(ii) * cell.dw * cell.dwdphi);
                    } else {
                        rhr1 = 0.0;
                    }
                    if ((adjCell.dw > 0) && (adjCell.dwdphi > 0)) {
                        rhr2 = adjCell.dist(adjl) / (cell.overlap(ii) * adjCell.dw * adjCell.dwdphi);
                    } else {
                        rhr2 = 0.0;
                    }

                    //             IF(rhr1+rhr2>0)THEN
                    if (rhr1 * rhr2 > 0) {
                        phioosum += 1.0 / (rhr1 + rhr2);
                        phiorsum += (adjCell.rhp1 / (rhr1 + rhr2));
                    }
                }

                // Moisture Capacitance
                if (cell.dwdphi > 0.0) {
                    wcap = cell.dwdphi * cell.volume;
                } else {
                    wcap = 0.0;
                }

                // Calculate the RH for the next time step
                denominator = (phioosum + vpoosum * cell.vpsat + wcap / s_hbh->deltat);
                if (denominator != 0.0) {
                    cell.rhp1 = (phiorsum + vporsum + (wcap * cell.rh) / s_hbh->deltat) / denominator;
                } else {
                    ShowSevereError(state, "CalcHeatBalHAMT: demoninator in calculating RH is zero.  Check material properties for accuracy.");
                    ShowContinueError(state, format("...Problem occurs in Material=\"{}\".", s_mat->materials(cell.matid)->Name));
                    ShowFatalError(state, "Program terminates due to preceding condition.");
                }

                if (cell.rhp1 > rhmax) {
                    cell.rhp1 = rhmax;
                }
            }

            // Check for convergence or too many itterations
            sumtp1 = 0.0;
            for (int cid = s_hbh->Extcell(sid); cid <= s_hbh->Intcell(sid); ++cid) {
                auto const &cell = s_hbh->cells(cid);
                if (sumtp1 < std::abs(cell.tempp2 - cell.tempp1)) {
                    sumtp1 = std::abs(cell.tempp2 - cell.tempp1);
                }
            }
            if (sumtp1 < convt) {
                break;
            }
            if (itter > ittermax) {
                break;
            }
            for (int cid = s_hbh->firstcell(sid); cid <= s_hbh->lastcell(sid); ++cid) {
                auto &cell = s_hbh->cells(cid);
                cell.tempp2 = cell.tempp1;
                cell.rhp2 = cell.rhp1;
            }
        }

        // report back to CalcHeatBalanceInsideSurf
        TempSurfOutTmp = extCell.tempp1;
        SurfTempInTmp = intCell.tempp1;

        SurfTempInP = intCell.rhp1 * PsyPsatFnTemp(state, intCell.tempp1);

        state.dataMstBal->RhoVaporSurfIn(sid) = SurfTempInP / (461.52 * (spaceMAT + Constant::Kelvin));
    }

    void UpdateHeatBalHAMT(EnergyPlusData &state, int const sid)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Phillip Biddulph
        //       DATE WRITTEN   June 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // The zone heat balance equation has converged, so now the HAMT values are to be fixed
        // ready for the next itteration.
        // Fill all the report variables

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 watermass;
        Real64 matmass;
        // unused1208    REAL(r64), SAVE :: InOld=0.0D0
        // unused1208    REAL(r64), SAVE :: OutOld=0.0D0

        auto &s_hbh = state.dataHeatBalHAMTMgr;

        // Update Temperatures and RHs. Calculate report variables
        matmass = 0.0;
        watermass = 0.0;
        for (int cid = s_hbh->firstcell(sid); cid <= s_hbh->lastcell(sid); ++cid) {
            auto &cell = s_hbh->cells(cid);
            // fix HAMT values for this surface
            cell.temp = cell.tempp1;
            cell.rh = cell.rhp1;
            cell.rhp = cell.rh * 100.0;
            if (cell.density > 0.0) {
                cell.wreport = cell.water / cell.density;
                watermass += (cell.water * cell.volume);
                matmass += (cell.density * cell.volume);
            }
        }

        s_hbh->watertot(sid) = 0.0;
        if (matmass > 0) {
            s_hbh->watertot(sid) = watermass / matmass;
        }

        s_hbh->surfrh(sid) = 100.0 * s_hbh->cells(s_hbh->Intcell(sid)).rh;
        s_hbh->surfextrh(sid) = 100.0 * s_hbh->cells(s_hbh->Extcell(sid)).rh;
        s_hbh->surftemp(sid) = s_hbh->cells(s_hbh->Intcell(sid)).temp;
        s_hbh->surfexttemp(sid) = s_hbh->cells(s_hbh->Extcell(sid)).temp;
        s_hbh->surfvp(sid) = RHtoVP(state, s_hbh->cells(s_hbh->Intcell(sid)).rh, s_hbh->cells(s_hbh->Intcell(sid)).temp);
    }

    void interp(int const ndata,
                const Array1D<Real64> &xx,
                const Array1D<Real64> &yy,
                Real64 const invalue,
                Real64 &outvalue,
                ObjexxFCL::Optional<Real64> outgrad)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Phillip Biddulph
        //       DATE WRITTEN   June 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // To find a value by searching an array and interpolating between two coordinates
        // Also returns the gradient if required.

        // METHODOLOGY EMPLOYED:
        // Simple search

        // Argument array dimensioning
        EP_SIZE_CHECK(xx, ndata);
        EP_SIZE_CHECK(yy, ndata);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 xxlow;
        Real64 xxhigh;
        Real64 yylow;
        Real64 yyhigh;
        Real64 mygrad;

        mygrad = 0.0;
        outvalue = 0.0;

        if (ndata > 1) {
            xxlow = xx(1);
            yylow = yy(1);
            for (int step = 2; step <= ndata; ++step) {
                xxhigh = xx(step);
                yyhigh = yy(step);
                if (invalue <= xxhigh) {
                    break;
                }
                xxlow = xxhigh;
                yylow = yyhigh;
            }

            if (xxhigh > xxlow) {
                mygrad = (yyhigh - yylow) / (xxhigh - xxlow);
                outvalue = (invalue - xxlow) * mygrad + yylow;
                // PDB August 2009 bug fix
            } else if (std::abs(xxhigh - xxlow) < 0.0000000001) {
                outvalue = yylow;
            }
        }

        if (present(outgrad)) {
            // return gradient if required
            outgrad = mygrad;
        }
    }

    Real64 RHtoVP(EnergyPlusData &state, Real64 const RH, Real64 const Temperature)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Phillip Biddulph
        //       DATE WRITTEN   June 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Convert Relative Humidity and Temperature to Vapor Pressure

        // Return value
        Real64 RHtoVP;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 VPSat;

        VPSat = PsyPsatFnTemp(state, Temperature);

        RHtoVP = RH * VPSat;

        return RHtoVP;
    }

    Real64 WVDC(Real64 const Temperature, Real64 const ambp)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Phillip Biddulph
        //       DATE WRITTEN   June 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // To calculate the Water Vapor Diffusion Coefficient in air
        // using the temperature and ambient atmospheric pressor

        // REFERENCES:
        // K?zel, H.M. (1995) Simultaneous Heat and Moisture Transport in Building Components.
        // One- and two-dimensional calculation using simple parameters. IRB Verlag 1995

        // Return value
        Real64 WVDC;

        WVDC = (2.e-7 * std::pow(Temperature + Constant::Kelvin, 0.81)) / ambp;

        return WVDC;
    }

    //                                 COPYRIGHT NOTICE

    //     Portions Copyright (c) University College London 2007.  All rights
    //     reserved.

    //     UCL LEGAL NOTICE
    //     Neither UCL, members of UCL nor any person or organisation acting on
    //     behalf of either:

    //     A. Makes any warranty of representation, express or implied with
    //        respect to the accuracy, completeness, or usefulness of the
    //        information contained in this program, including any warranty of
    //        merchantability or fitness of any purpose with respect to the
    //        program, or that the use of any information disclosed in this
    //        program may not infringe privately-owned rights, or

    //     B. Assumes any liability with respect to the use of, or for any and
    //        all damages resulting from the use of the program or any portion
    //        thereof or any information disclosed therein.

} // namespace HeatBalanceHAMTManager

} // namespace EnergyPlus
