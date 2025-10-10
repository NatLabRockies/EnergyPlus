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
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBSDFWindow.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/DataWindowEquivalentLayer.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WindowEquivalentLayer.hh>

namespace EnergyPlus::WindowEquivalentLayer {

// MODULE INFORMATION
//       AUTHOR         Bereket A. Nigusse, FSEC/UCF
//       DATE WRITTEN   May 2013

// PURPOSE OF THIS MODULE:
//  Manages the equivalent layer (ASHWAT) window model optical and thermal
//  calculations
// METHODOLOGY EMPLOYED:
//  Uses net radiation method to calculate the optical properties of a multi-layer
//  window construction. Most of the routines in this module were adopted directly
//  from ASHRAE 1311-RP.
// REFERENCES:
// John L. Wright,    Charles S. Barnaby, Michael R. Collins, and Nathan A. Kotey.
// Improving Cooling Load Calculations for Fenestration with Shading Devices
// ASHRAE 1311-RP, Final Report, February 11, 2009.
// Edwards, D.K. 1977.  Solar absorption by each element in an absorber-coverglass
//  array,Technical Note, Solar Energy, Vol. 19, pp. 401-402.
// Kotey, N. A., J. L. Wright, and M. R. Collins.  2008.  "Determining Longwave
//  RadiativeProperties of Flat Shading Materials," 33rd Annual SESCI / 3rd CSBC
//  Conference Proceedings, Fredericton, NB.
// Kotey, N.A., Wright, J.L., M. R. Collins. 2009a. "Determination of Angle-Dependent
//  SolarOptical Properties of Roller Blind Materials," drafted for submission to
//  ASHRAE Transactions, Vol. 115, Pt. 1.

// Kotey, N.A., Wright, J.L., M. R. Collins. 2009b. "Determination of Angle-Dependent
//  Solar Optical Properties of Drapery Fabrics," in review, ASHRAE Transactions,
//  Vol. 115, Pt. 2.
// Wright, J. L.  2008.  "Calculating Centre-Glass Performance Indices of Glazing
//  Systems with Shading Devices," ASHRAE Transactions, Vol. 114, Pt. 2.
// Wright, J. L., N. Y. T. Huang, and M. R. Collins.  2008.  "Thermal Resistance
//  of a Window with an Enclosed Venetian Blind: A Simplified Model,"
//  ASHRAE Transactions, Vol. 114, Pt. 1.

// Yahoda, D. S. and J. L. Wright.  2004.  "Methods for Calculating the Effective
//  Longwave Radiative Properties of a Venetian Blind Layer," ASHRAE Transactions,
//  Vol. 110, Pt. 1., pp. 463-473.
// Yahoda, D. S. and J. L. Wright.  2005.  "Methods for Calculating the Effective
//  Solar-Optical Properties of a Venetian Blind Layer," ASHRAE Transactions,
//  Vol. 111, Pt. 1, pp. 572-586.
// Yahoda, D. S. and J. L. Wright.  2004.  "Heat Transfer Analysis of a Between-Panes
//  Venetian Blind Using Effective Longwave Radiative Properties," ASHRAE Transactions,
//  Vol. 110, Pt. 1., pp. 455-462.
// Using/Aliasing
using namespace DataHeatBalance;
using namespace DataSurfaces;

// constexpr std::array<std::string_view, (int)Orientation::Num> orientationNamesUC = {"HORIZONTAL", "VERTICAL"};

void InitEquivalentLayerWindowCalculations(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Bereket Nigusse
    //       DATE WRITTEN   May 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Initializes the optical properties for the Equivalent Layer (ASHWAT) Window model
    // METHODOLOGY EMPLOYED:
    // Gets the EquivalentLayer Window Layers Inputs.  Fills in the derived data type
    // based on the inputs specified.

    if (state.dataWindowEquivLayer->TotWinEquivLayerConstructs < 1) {
        return;
    }
    if (!allocated(state.dataWindowEquivLayer->CFS)) {
        state.dataWindowEquivLayer->CFS.allocate(state.dataWindowEquivLayer->TotWinEquivLayerConstructs);
    }
    if (!allocated(state.dataWindowEquivalentLayer->EQLDiffPropFlag)) {
        state.dataWindowEquivalentLayer->EQLDiffPropFlag.allocate(state.dataWindowEquivLayer->TotWinEquivLayerConstructs);
    }
    if (!allocated(state.dataWindowEquivalentLayer->CFSDiffAbsTrans)) {
        state.dataWindowEquivalentLayer->CFSDiffAbsTrans.allocate(2, CFSMAXNL + 1, state.dataWindowEquivLayer->TotWinEquivLayerConstructs);
    }

    state.dataWindowEquivalentLayer->EQLDiffPropFlag = true;
    state.dataWindowEquivalentLayer->CFSDiffAbsTrans = 0.0;

    for (int ConstrNum = 1; ConstrNum <= state.dataHeatBal->TotConstructs; ++ConstrNum) {
        if (!state.dataConstruction->Construct(ConstrNum).TypeIsWindow) {
            continue;
        }
        if (!state.dataConstruction->Construct(ConstrNum).WindowTypeEQL) {
            continue; // skip if not equivalent layer window
        }

        SetEquivalentLayerWindowProperties(state, ConstrNum);

    } //  end do for TotConstructs

    for (int SurfNum : state.dataSurface->AllHTWindowSurfaceList) {
        if (!state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).WindowTypeEQL) {
            continue;
        }

        state.dataSurface->SurfWinWindowModelType(SurfNum) = WindowModel::EQL;

    } //  end do for SurfNum
}

void SetEquivalentLayerWindowProperties(EnergyPlusData &state, int const ConstrNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Bereket Nigusse
    //       DATE WRITTEN   May 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Populates the the equivalent layer window model optical and thermal
    // properties, fills default values and shades geometrical calculations

    // METHODOLOGY EMPLOYED:
    // uses some routine developed for ASHRAE RP-1311 (ASHWAT Model)

    Array2D<Real64> SysAbs1(2, CFSMAXNL + 1); // layers absorptance and system transmittance

    auto &s_mat = state.dataMaterial;

    if (!allocated(state.dataWindowEquivLayer->CFSLayers)) {
        state.dataWindowEquivLayer->CFSLayers.allocate(state.dataConstruction->Construct(ConstrNum).TotLayers);
    }

    int sLayer = 0; // glazing and shade layers (non-gas layers) index
    int gLayer = 0; // gap layer index
    int EQLNum = state.dataConstruction->Construct(ConstrNum).EQLConsPtr;

    auto &CFS = state.dataWindowEquivLayer->CFS;

    CFS(EQLNum).Name = state.dataConstruction->Construct(ConstrNum).Name;

    for (int Layer = 1; Layer <= state.dataConstruction->Construct(ConstrNum).TotLayers; ++Layer) {

        Material::Group group1 = s_mat->materials(state.dataConstruction->Construct(ConstrNum).LayerPoint(1))->group;
        if (group1 != Material::Group::GlassEQL && group1 != Material::Group::ShadeEQL && group1 != Material::Group::DrapeEQL &&
            group1 != Material::Group::ScreenEQL && group1 != Material::Group::BlindEQL && group1 != Material::Group::WindowGapEQL) {
            continue;
        }

        int MaterNum = state.dataConstruction->Construct(ConstrNum).LayerPoint(Layer);
        auto const *mat = s_mat->materials(MaterNum);

        if (mat->group == Material::Group::BlindEQL) {
            auto const *thisMaterial = dynamic_cast<Material::MaterialBlindEQL const *>(mat);
            assert(thisMaterial != nullptr);

            ++sLayer;
            CFS(EQLNum).L(sLayer).Name = thisMaterial->Name;
            // longwave property input
            CFS(EQLNum).L(sLayer).LWP_MAT.EPSLF = thisMaterial->TAR.IR.Ft.Emi;
            CFS(EQLNum).L(sLayer).LWP_MAT.EPSLB = thisMaterial->TAR.IR.Bk.Emi;
            CFS(EQLNum).L(sLayer).LWP_MAT.TAUL = thisMaterial->TAR.IR.Ft.Tra;

            CFS(EQLNum).VBLayerPtr = sLayer;
            if (thisMaterial->SlatOrientation == DataWindowEquivalentLayer::Orientation::Horizontal) {
                CFS(EQLNum).L(sLayer).LTYPE = LayerType::VBHOR;
            } else if (thisMaterial->SlatOrientation == DataWindowEquivalentLayer::Orientation::Vertical) {
                CFS(EQLNum).L(sLayer).LTYPE = LayerType::VBVER;
            }
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSFBD = thisMaterial->TAR.Sol.Ft.Bm[0].DfRef;
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSBBD = thisMaterial->TAR.Sol.Bk.Bm[0].DfRef;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSFBD = thisMaterial->TAR.Sol.Ft.Bm[0].DfTra;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSBBD = thisMaterial->TAR.Sol.Bk.Bm[0].DfTra;

            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSFDD = thisMaterial->TAR.Sol.Ft.Df.Ref;
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSBDD = thisMaterial->TAR.Sol.Bk.Df.Ref;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUS_DD = thisMaterial->TAR.Sol.Ft.Df.Tra;
            CFS(EQLNum).L(sLayer).PHI_DEG = thisMaterial->SlatAngle;
            CFS(EQLNum).L(sLayer).CNTRL = static_cast<int>(thisMaterial->slatAngleType);
            CFS(EQLNum).L(sLayer).S = thisMaterial->SlatSeparation;
            CFS(EQLNum).L(sLayer).W = thisMaterial->SlatWidth;
            CFS(EQLNum).L(sLayer).C = thisMaterial->SlatCrown;

        } else if (mat->group == Material::Group::GlassEQL) {
            auto const *thisMaterial = dynamic_cast<Material::MaterialGlassEQL const *>(mat);
            assert(thisMaterial != nullptr);
            // glazing
            ++sLayer;
            CFS(EQLNum).L(sLayer).Name = thisMaterial->Name;
            // longwave property input
            CFS(EQLNum).L(sLayer).LWP_MAT.EPSLF = thisMaterial->TAR.IR.Ft.Emi;
            CFS(EQLNum).L(sLayer).LWP_MAT.EPSLB = thisMaterial->TAR.IR.Bk.Emi;
            CFS(EQLNum).L(sLayer).LWP_MAT.TAUL = thisMaterial->TAR.IR.Ft.Tra;

            CFS(EQLNum).L(sLayer).LTYPE = LayerType::GLAZE;
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSFBB = thisMaterial->TAR.Sol.Ft.Bm[0].BmRef;
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSBBB = thisMaterial->TAR.Sol.Bk.Bm[0].BmRef;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSFBB = thisMaterial->TAR.Sol.Ft.Bm[0].BmTra;

            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSFBD = thisMaterial->TAR.Sol.Ft.Bm[0].DfRef;
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSBBD = thisMaterial->TAR.Sol.Bk.Bm[0].DfRef;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSFBD = thisMaterial->TAR.Sol.Ft.Bm[0].DfTra;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSBBD = thisMaterial->TAR.Sol.Bk.Bm[0].DfTra;

            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSFDD = thisMaterial->TAR.Sol.Ft.Df.Ref;
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSBDD = thisMaterial->TAR.Sol.Bk.Df.Ref;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUS_DD = thisMaterial->TAR.Sol.Ft.Df.Tra;

        } else if (mat->group == Material::Group::ShadeEQL) {
            auto const *thisMaterial = dynamic_cast<Material::MaterialShadeEQL const *>(mat);
            assert(thisMaterial != nullptr);
            // roller blind
            ++sLayer;
            CFS(EQLNum).L(sLayer).Name = thisMaterial->Name;
            // longwave property input
            CFS(EQLNum).L(sLayer).LWP_MAT.EPSLF = thisMaterial->TAR.IR.Ft.Emi;
            CFS(EQLNum).L(sLayer).LWP_MAT.EPSLB = thisMaterial->TAR.IR.Bk.Emi;
            CFS(EQLNum).L(sLayer).LWP_MAT.TAUL = thisMaterial->TAR.IR.Ft.Tra;

            CFS(EQLNum).L(sLayer).LTYPE = LayerType::ROLLB;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSFBB = thisMaterial->TAR.Sol.Ft.Bm[0].BmTra;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSBBB = thisMaterial->TAR.Sol.Bk.Bm[0].BmTra;
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSFBD = thisMaterial->TAR.Sol.Ft.Bm[0].DfRef;
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSBBD = thisMaterial->TAR.Sol.Bk.Bm[0].DfRef;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSFBD = thisMaterial->TAR.Sol.Ft.Bm[0].DfTra;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSBBD = thisMaterial->TAR.Sol.Bk.Bm[0].DfTra;

        } else if (mat->group == Material::Group::DrapeEQL) {
            auto const *thisMaterial = dynamic_cast<Material::MaterialDrapeEQL const *>(mat);
            assert(thisMaterial != nullptr);
            // drapery fabric
            ++sLayer;
            CFS(EQLNum).L(sLayer).Name = thisMaterial->Name;
            // longwave property input
            CFS(EQLNum).L(sLayer).LWP_MAT.EPSLF = thisMaterial->TAR.IR.Ft.Emi;
            CFS(EQLNum).L(sLayer).LWP_MAT.EPSLB = thisMaterial->TAR.IR.Bk.Emi;
            CFS(EQLNum).L(sLayer).LWP_MAT.TAUL = thisMaterial->TAR.IR.Ft.Tra;

            CFS(EQLNum).L(sLayer).LTYPE = LayerType::DRAPE;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSFBB = thisMaterial->TAR.Sol.Ft.Bm[0].BmTra;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSBBB = thisMaterial->TAR.Sol.Bk.Bm[0].BmTra;
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSFBD = thisMaterial->TAR.Sol.Ft.Bm[0].DfRef;
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSBBD = thisMaterial->TAR.Sol.Bk.Bm[0].DfRef;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSFBD = thisMaterial->TAR.Sol.Ft.Bm[0].DfTra;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSBBD = thisMaterial->TAR.Sol.Bk.Bm[0].DfTra;

            CFS(EQLNum).L(sLayer).S = thisMaterial->pleatedLength;
            CFS(EQLNum).L(sLayer).W = thisMaterial->pleatedWidth;
            // init diffuse SWP to force default derivation
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSFDD = -1.0;
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSBDD = -1.0;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUS_DD = -1.0;

        } else if (mat->group == Material::Group::ScreenEQL) {
            auto const *thisMaterial = dynamic_cast<Material::MaterialScreenEQL const *>(mat);
            assert(thisMaterial != nullptr);
            // insect screen
            ++sLayer;
            CFS(EQLNum).L(sLayer).Name = thisMaterial->Name;
            // longwave property input
            CFS(EQLNum).L(sLayer).LWP_MAT.EPSLF = thisMaterial->TAR.IR.Ft.Emi;
            CFS(EQLNum).L(sLayer).LWP_MAT.EPSLB = thisMaterial->TAR.IR.Bk.Emi;
            CFS(EQLNum).L(sLayer).LWP_MAT.TAUL = thisMaterial->TAR.IR.Ft.Tra;

            CFS(EQLNum).L(sLayer).LTYPE = LayerType::INSCRN;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSFBB = thisMaterial->TAR.Sol.Ft.Bm[0].BmTra;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSBBB = thisMaterial->TAR.Sol.Bk.Bm[0].BmTra;
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSFBD = thisMaterial->TAR.Sol.Ft.Bm[0].DfRef;
            CFS(EQLNum).L(sLayer).SWP_MAT.RHOSBBD = thisMaterial->TAR.Sol.Bk.Bm[0].DfRef;

            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSFBD = thisMaterial->TAR.Sol.Ft.Bm[0].DfTra;
            CFS(EQLNum).L(sLayer).SWP_MAT.TAUSBBD = thisMaterial->TAR.Sol.Bk.Bm[0].DfTra;
            // wire geometry
            CFS(EQLNum).L(sLayer).S = thisMaterial->wireSpacing;
            CFS(EQLNum).L(sLayer).W = thisMaterial->wireDiameter;

        } else if (mat->group == Material::Group::WindowGapEQL) {
            auto const *matGas = dynamic_cast<Material::MaterialGasMix const *>(mat);
            assert(matGas != nullptr);

            // Gap or Gas Layer
            ++gLayer;

            CFS(EQLNum).G(gLayer).Name = matGas->Name;
            // previously the values of the levels are 1-3, now it's 0-2
            CFS(EQLNum).G(gLayer).GTYPE = (int)matGas->gapVentType + 1;
            CFS(EQLNum).G(gLayer).TAS = matGas->Thickness;

            auto const &gas = matGas->gases[0];
            CFS(EQLNum).G(gLayer).FG.Name = Material::gasTypeNames[(int)gas.type];
            CFS(EQLNum).G(gLayer).FG.AK = gas.con.c0;
            CFS(EQLNum).G(gLayer).FG.BK = gas.con.c1;
            CFS(EQLNum).G(gLayer).FG.CK = gas.con.c2;
            CFS(EQLNum).G(gLayer).FG.ACP = gas.cp.c0;
            CFS(EQLNum).G(gLayer).FG.BCP = gas.cp.c1;
            CFS(EQLNum).G(gLayer).FG.CCP = gas.cp.c2;
            CFS(EQLNum).G(gLayer).FG.AVISC = gas.cp.c0;
            CFS(EQLNum).G(gLayer).FG.BVISC = gas.cp.c1;
            CFS(EQLNum).G(gLayer).FG.CVISC = gas.cp.c2;
            CFS(EQLNum).G(gLayer).FG.MHAT = gas.wght;
            // fills gas density and effective gap thickness
            BuildGap(state, CFS(EQLNum).G(gLayer), CFS(EQLNum).G(gLayer).GTYPE, CFS(EQLNum).G(gLayer).TAS);
        } else {
            ++sLayer;
            CFS(EQLNum).L(sLayer).Name = mat->Name;
            CFS(EQLNum).L(sLayer).LTYPE = LayerType::NONE;
        }
        // beam beam transmittance is the same for front and back side
        CFS(EQLNum).L(sLayer).SWP_MAT.TAUSBBB = CFS(EQLNum).L(sLayer).SWP_MAT.TAUSFBB;
        CFS(EQLNum).NL = sLayer;

        // checks optical properties and fill in default values for diffuse optical
        // properties by calculating from other optical inputs, also fills in geometrical inputs
        CheckAndFixCFSLayer(state, CFS(EQLNum).L(sLayer));

    } // end do for Construct(ConstrNum)%TotLayers

    // Finalize CFS after get input.  Correct effective gap thickness for VB
    FinalizeCFS(state, CFS(EQLNum));

    // get total solid layers (glazing layers + shade layers)
    state.dataConstruction->Construct(ConstrNum).TotSolidLayers = CFS(EQLNum).NL;

    // Calculate layers diffuse absorptance and system diffuse transmittance
    CalcEQLWindowOpticalProperty(state, CFS(EQLNum), SolarArrays::DIFF, SysAbs1, 0.0, 0.0, 0.0);
    state.dataConstruction->Construct(ConstrNum).TransDiffFrontEQL = SysAbs1(1, CFS(EQLNum).NL + 1);
    state.dataWindowEquivalentLayer->CFSDiffAbsTrans(_, _, EQLNum) = SysAbs1;
    state.dataConstruction->Construct(ConstrNum).AbsDiffFrontEQL({1, CFSMAXNL}) = SysAbs1(1, {1, CFSMAXNL});
    state.dataConstruction->Construct(ConstrNum).AbsDiffBackEQL({1, CFSMAXNL}) = SysAbs1(2, {1, CFSMAXNL});
    // get construction front and back diffuse effective reflectance
    state.dataConstruction->Construct(ConstrNum).ReflectSolDiffFront = CFS(EQLNum).L(1).SWP_EL.RHOSFDD;
    state.dataConstruction->Construct(ConstrNum).ReflectSolDiffBack = CFS(EQLNum).L(CFS(EQLNum).NL).SWP_EL.RHOSBDD;
    // calculate U-Value, SHGC and Normal Transmittance of EQL Window
    CalcEQLWindowStandardRatings(state, ConstrNum);

    if (CFSHasControlledShade(state, CFS(EQLNum)) > 0) {
        CFS(EQLNum).ISControlled = true; // is controlled
    }

    // set internal face emissivity
    state.dataConstruction->Construct(ConstrNum).InsideAbsorpThermal = EffectiveEPSLB(CFS(EQLNum));
}

void CalcEQLWindowUvalue(EnergyPlusData &state,
                         CFSTY const &FS, // CFS to be calculated
                         Real64 &UNFRC    // NFRC U-factor, W/m2-K
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT/Chip Barnaby
    //       DATE WRITTEN   Last Modified February 2008
    //       MODIFIED       Bereket Nigusse, May 2013
    //                      Replaced inside convection calculation with ISO Std 15099

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates U-value of equivalent layer window at standard
    // fenestration winter rating conditions

    // METHODOLOGY EMPLOYED:
    // uses routine developed for ASHRAE RP-1311 (ASHWAT Model)
    // NFRC rated *HEATING* U-factor or Winter Rating Condition
    // tin = 294.15d0   ! Inside air temperature (69.8F, 21.0C)
    // tout = 255.15d0  ! Outside air temperature (-0.4F, -18C)
    // hcout = 26.d0    ! Outside convective film conductance at 5.5 m/s (12.3 mph)
    //                  ! wind speed (the value used in Window 5)
    // BeamSolarInc = 0.0

    Real64 constexpr Height(1.0); // window height, m
    Real64 constexpr TOUT(-18.0); // outdoor air temperature, C
    Real64 constexpr TIN(21.0);   // indoor air temperature, C
    static constexpr std::string_view RoutineName("CalcEQLWindowUvalue: ");

    Real64 U;    // U-factor, W/m2-K
    Real64 UOld; // U-factor during previous iteration step, W/m2-K
    Real64 HXO;  // outdoor combined conv+rad surf coeff, W/m2-K
    Real64 HXI;  // indoor combined conf+rad surf coeff, W/m2-K
    Real64 HRO;  // outdoor side radiation surf coeff, W/m2-K
    Real64 HCO;  // outdoor side convection surf coeff, W/m2-K
    Real64 HRI;  // indoor side radiation surf coeff, W/m2-K
    Real64 HCI;  // indoor side convection surf coeff, W/m2-K
    Real64 TGO;
    Real64 TGI;
    Real64 TGIK;
    Real64 TIK;
    Real64 DT; // temperature difference, K
    Real64 EO; // outside face effective emissivity, (-)
    Real64 EI; // inside face effective emissivity, (-)
    int I;     // index

    bool CFSURated = false; // false if U-Value calculation failed

    // Initial guess value for combined conductance
    HXO = 29.0; // 1/FenROut
    HXI = 7.0;  // 1/FenRIn
    HCO = 26.0;
    HCI = 3.0; // Initial guess

    DT = TIN - TOUT;               // note DT == 0 detected in CFSUFactor()
    EO = FS.L(1).LWP_EL.EPSLF;     // emissivities outside
    EI = FS.L(FS.NL).LWP_EL.EPSLB; // emissivities inside
    U = 5.0 / FS.NL;               // initial guess

    // Iterate: find surface temperature, update coeffs, converge to U
    for (I = 1; I <= 10; ++I) {
        TGO = TOUT + U * DT / HXO; // update glazing surface temps
        TGI = TIN - U * DT / HXI;
        HRO = Constant::StefanBoltzmann * EO * (pow_2(TGO + Constant::Kelvin) + pow_2(TOUT + Constant::Kelvin)) *
              ((TGO + Constant::Kelvin) + (TOUT + Constant::Kelvin));
        HRI = Constant::StefanBoltzmann * EI * (pow_2(TGI + Constant::Kelvin) + pow_2(TIN + Constant::Kelvin)) *
              ((TGI + Constant::Kelvin) + (TIN + Constant::Kelvin));
        // HCI = HIC_ASHRAE( Height, TGI, TI)  ! BAN June 2103 Replaced with ISO Std 15099
        TGIK = TGI + Constant::Kelvin;
        TIK = TIN + Constant::Kelvin;
        HCI = HCInWindowStandardRatings(state, Height, TGIK, TIK);
        if (HCI < 0.001) {
            break;
        }
        HXI = HCI + HRI;
        HXO = HCO + HRO;
        UOld = U;
        if (!CFSUFactor(state, FS, TOUT, HCO, TIN, HCI, U)) {
            break;
        }
        if (I > 1 && FEQX(U, UOld, 0.001)) {
            CFSURated = true;
            break;
        }
    }
    if (!CFSURated) {
        ShowWarningMessage(state, format("{}Fenestration U-Value calculation failed for {}", RoutineName, FS.Name));
        ShowContinueError(state, format("...Calculated U-value = {:.4T}", U));
        ShowContinueError(state, "...Check consistency of inputs");
    }
    UNFRC = U;
}

void CalcEQLWindowSHGCAndTransNormal(EnergyPlusData &state,
                                     CFSTY const &FS,    // fenestration system
                                     Real64 &SHGCSummer, // solar heat gain coefficient
                                     Real64 &TransNormal // transmittance at normal incidence
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Bereket Nigusse
    //       DATE WRITTEN   May 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates SHGC and Normal Transmittance of equivalent layer fenestration.

    // METHODOLOGY EMPLOYED:
    // Uses routine developed for ASHRAE RP-1311 (ASHWAT Model)
    // Summer Window Rating Conditions
    // tin = 297.15d0         ! indoor air condition (75.2F,  24.0C)
    // tout = 305.15d0        ! Outside air temperature (89.6F, 32C)
    // hcout = 15.d0          ! Outside convective film conductance at 2.8 m/s (6.2 mph) wind speed
    // BeamSolarInc = 783.0d0 ! Direct normal incident solar radiation, W/m2

    constexpr Real64 TOL(0.01);
    constexpr Real64 TIN(297.15);
    constexpr Real64 TOUT(305.15);
    constexpr Real64 BeamSolarInc(783.0);
    static constexpr std::string_view RoutineName("CalcEQLWindowSHGCAndTransNormal: ");

    Real64 HCOUT;
    Real64 TRMOUT;
    Real64 TRMIN;
    Real64 HCIN;
    Array1D<Real64> QOCF(CFSMAXNL);
    Array1D<Real64> JB({0, CFSMAXNL});
    Array1D<Real64> JF({1, CFSMAXNL + 1});
    Array1D<Real64> T(CFSMAXNL);
    Array1D<Real64> Q({0, CFSMAXNL});
    Array1D<Real64> H({0, CFSMAXNL + 1});
    Array2D<Real64> Abs1(2, CFSMAXNL + 1);
    Real64 QOCFRoom;
    Real64 UCG;
    Real64 SHGC;
    Real64 IncA;
    Real64 VProfA;
    Real64 HProfA;
    int NL;
    int I;

    // Object Data
    Array1D<CFSSWP> SWP_ON(CFSMAXNL);

    NL = FS.NL;
    IncA = 0.0;
    VProfA = 0.0;
    HProfA = 0.0;
    Abs1 = 0.0;
    HCIN = 3.0; // Initial guess
    HCOUT = 15.0;
    if (FS.L(1).LTYPE == LayerType::ROLLB || FS.L(1).LTYPE == LayerType::DRAPE || FS.L(1).LTYPE == LayerType::INSCRN ||
        FS.L(1).LTYPE == LayerType::VBHOR || FS.L(1).LTYPE == LayerType::VBVER) { // Exterior Roller Blind Present | Exterior Drape Fabric | Exterior
                                                                                  // Insect Screen Present | Exterior Venetian Blind Present
        // Reduced convection coefficient due to external attachment
        HCOUT = 12.25;
    }

    // Temperatures
    TRMOUT = TOUT;
    TRMIN = TIN;

    //  Convert direct-normal solar properties for beam incidence to current incident angle
    for (I = 1; I <= NL; ++I) {
        ASHWAT_OffNormalProperties(state, FS.L(I), IncA, VProfA, HProfA, SWP_ON(I));
    }
    ASHWAT_Solar(FS.NL, SWP_ON, state.dataWindowEquivLayer->SWP_ROOMBLK, 1.0, 0.0, 0.0, Abs1(1, {1, FS.NL + 1}), Abs1(2, {1, FS.NL + 1}));
    TransNormal = Abs1(1, NL + 1);

    // Calculate SHGC using net radiation method (ASHWAT Model)
    bool CFSSHGC = ASHWAT_ThermalRatings(state,
                                         FS,
                                         TIN,
                                         TOUT,
                                         HCIN,
                                         HCOUT,
                                         TRMOUT,
                                         TRMIN,
                                         BeamSolarInc,
                                         BeamSolarInc * Abs1(1, {1, NL + 1}),
                                         TOL,
                                         QOCF,
                                         QOCFRoom,
                                         T,
                                         Q,
                                         JF,
                                         JB,
                                         H,
                                         UCG,
                                         SHGC,
                                         true);

    if (!CFSSHGC) {
        ShowWarningMessage(state, format("{}Solar heat gain coefficient calculation failed for {}", RoutineName, FS.Name));
        ShowContinueError(state, format("...Calculated SHGC = {:.4T}", SHGC));
        ShowContinueError(state, format("...Calculated U-Value = {:.4T}", UCG));
        ShowContinueError(state, "...Check consistency of inputs.");
        return;
    }
    SHGCSummer = SHGC;
}

void CalcEQLWindowOpticalProperty(EnergyPlusData &state,
                                  CFSTY &FS,                      // fenestration system
                                  SolarArrays const DiffBeamFlag, // isDIFF: calc diffuse properties
                                  Array2A<Real64> Abs1,
                                  Real64 const IncA,   // angle of incidence, radians
                                  Real64 const VProfA, // inc solar vertical profile angle, radians
                                  Real64 const HProfA  // inc solar horizontal profile angle, radians
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         University of WaterLoo
    //       MODIFIED       Bereket Nigusse, May 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates absorptance for each layer, and transmittance of the
    // fenestration for beam and diffuse solar radiation

    // METHODOLOGY EMPLOYED:
    // uses routine developed for ASHRAE RP-1311 (ASHWAT Model). Uses net radiation method.

    // Argument array dimensioning
    Abs1.dim(2, CFSMAXNL + 1);

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    // else: isBEAM
    // returned: layer abs for unit (1 W/m2) incident
    //   if beam, Abs1( :, 1) = abs for IncA
    //            Abs1( :, 2) = trans Beam-Diffuse only
    //   if diff, Abs1( :, 1) = abs for outside diff
    //            Abs1( :, 2) = abs for inside diff
    //   + = up-from-horizontal
    //   + = west-of-normal
    // convect coefficients, W/m2-K

    // Object Data
    Array1D<CFSSWP> SWP_ON(CFSMAXNL);

    int NL = FS.NL;
    Abs1 = 0.0;

    if (FS.ISControlled) { // at least 1 controlled layer found
        for (int iL = 1; iL <= NL; ++iL) {
            // If there is shade control (Venetian Blind Only).
            if (IsControlledShade(state, FS.L(iL))) {
                DoShadeControl(state, FS.L(iL), IncA, VProfA, HProfA);
            }
        }
    }

    if (DiffBeamFlag != SolarArrays::DIFF) {
        //  Beam: Convert direct-normal solar properties to off-normal properties
        for (int I = 1; I <= NL; ++I) {
            ASHWAT_OffNormalProperties(state, FS.L(I), IncA, VProfA, HProfA, SWP_ON(I));
        }
        ASHWAT_Solar(FS.NL, SWP_ON, state.dataWindowEquivLayer->SWP_ROOMBLK, 1.0, 0.0, 0.0, Abs1(1, {1, FS.NL + 1}), Abs1(2, {1, FS.NL + 1}));
    } else {
        // diffuse
        Array1D<CFSSWP> const SWP_EL(FS.L.ma(&CFSLAYER::SWP_EL)); // Autodesk:F2C++ Can't slice a member array so we create a temporary: Inefficient
        ASHWAT_Solar(FS.NL, SWP_EL, state.dataWindowEquivLayer->SWP_ROOMBLK, 0.0, 1.0, 0.0, Abs1(1, {1, FS.NL + 1}));
        ASHWAT_Solar(FS.NL, SWP_EL, state.dataWindowEquivLayer->SWP_ROOMBLK, 0.0, 0.0, 1.0, Abs1(2, {1, FS.NL + 1}));
        // CFSFenProp = LOK1 .AND. LOK2
    }
}

void EQLWindowSurfaceHeatBalance(EnergyPlusData &state,
                                 int const SurfNum,       // Surface number
                                 Real64 const HcOut,      // outside convection coefficient at this timestep, W/m2K
                                 Real64 &SurfInsideTemp,  // Inside window surface temperature (innermost face) [C]
                                 Real64 &SurfOutsideTemp, // Outside surface temperature (C)
                                 Real64 &SurfOutsideEmiss,
                                 DataBSDFWindow::Condition const CalcCondition // Calculation condition (summer, winter or no condition)
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Bereket Nigusse
    //       DATE WRITTEN   May 2013

    // PURPOSE OF THIS SUBROUTINE:
    // performs surface heat balance and returns in the inside and outside surface temperatures

    // METHODOLOGY EMPLOYED:
    // uses the solar-thermal routine developed for ASHRAE RP-1311 (ASHWAT Model).

    using Psychrometrics::PsyCpAirFnW;
    using Psychrometrics::PsyTdpFnWPb;

    Real64 constexpr TOL(0.0001); // convergence tolerance

    int NL; // Number of layers
    Real64 TIN(0);
    Real64 TRMIN;
    Real64 Tout(0);
    Real64 TRMOUT;
    Real64 QCONV;
    Array1D<Real64> QOCF(CFSMAXNL);
    Real64 QOCFRoom;
    Array1D<Real64> JB({0, CFSMAXNL});
    Array1D<Real64> JF({1, CFSMAXNL + 1});
    Array1D<Real64> T(CFSMAXNL);
    Array1D<Real64> Q({0, CFSMAXNL});
    Array1D<Real64> H({0, CFSMAXNL + 1});
    Array1D<Real64> QAllSWwinAbs({1, CFSMAXNL + 1});

    int EQLNum;    // equivalent layer window index
    int ConstrNum; // Construction number

    Real64 LWAbsIn;  // effective long wave absorptance/emissivity back side
    Real64 LWAbsOut; // effective long wave absorptance/emissivity front side
    Real64 outir(0);
    Real64 rmir;
    Real64 Ebout;
    Real64 QXConv;              // extra convective gain from this surface
    Real64 TaIn(0);             // zone air temperature
    Real64 tsky;                // sky temperature
    Real64 HcIn;                // inside convection coefficient at this timestep, W/m2K
    Real64 ConvHeatFlowNatural; // Convective heat flow from gap between glass and interior shade or blind (W)
    Real64 NetIRHeatGainWindow; // net radiation gain from the window surface to the zone (W)
    Real64 ConvHeatGainWindow;  // net convection heat gain from inside surface of window to zone air (W)
    LayerType InSideLayerType;  // interior shade type

    Real64 SrdSurfTempAbs; // Absolute temperature of a surrounding surface
    Real64 OutSrdIR;

    if (CalcCondition != DataBSDFWindow::Condition::Invalid) {
        return;
    }

    ConstrNum = state.dataSurface->Surface(SurfNum).Construction;
    QXConv = 0.0;
    ConvHeatFlowNatural = 0.0;

    EQLNum = state.dataConstruction->Construct(ConstrNum).EQLConsPtr;
    HcIn = state.dataHeatBalSurf->SurfHConvInt(SurfNum); // windows inside surface convective film conductance

    if (CalcCondition == DataBSDFWindow::Condition::Invalid) {
        int SurfNumAdj = state.dataSurface->Surface(SurfNum).ExtBoundCond;
        Real64 RefAirTemp = state.dataSurface->Surface(SurfNum).getInsideAirTemperature(state, SurfNum);
        TaIn = RefAirTemp;
        TIN = TaIn + Constant::Kelvin; // Inside air temperature, K

        // now get "outside" air temperature
        if (SurfNumAdj > 0) {
            // this is interzone window. the outside condition is determined from the adjacent zone
            // condition
            int enclNumAdj = state.dataSurface->Surface(SurfNumAdj).RadEnclIndex;
            RefAirTemp = state.dataSurface->Surface(SurfNumAdj).getInsideAirTemperature(state, SurfNumAdj);
            Tout = RefAirTemp + Constant::Kelvin; // outside air temperature
            tsky = state.dataViewFactor->EnclRadInfo(enclNumAdj).MRT +
                   Constant::Kelvin; // TODO this misses IR from sources such as high temp radiant and baseboards

            // The IR radiance of this window's "exterior" surround is the IR radiance
            // from surfaces and high-temp radiant sources in the adjacent zone
            outir = state.dataSurface->SurfWinIRfromParentZone(SurfNumAdj) + state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(SurfNumAdj) +
                    state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNumAdj);

        } else { // Exterior window (ExtBoundCond = 0)
                 // Calculate LWR from surrounding surfaces if defined for an exterior window
            OutSrdIR = 0;
            if (state.dataGlobal->AnyLocalEnvironmentsInModel) {
                if (state.dataSurface->Surface(SurfNum).SurfHasSurroundingSurfProperty) {
                    SrdSurfTempAbs = state.dataSurface->Surface(SurfNum).SrdSurfTemp + Constant::Kelvin;
                    OutSrdIR = Constant::StefanBoltzmann * state.dataSurface->Surface(SurfNum).ViewFactorSrdSurfs * pow_4(SrdSurfTempAbs);
                }
            }
            if (state.dataSurface->Surface(SurfNum).ExtWind) { // Window is exposed to wind (and possibly rain)
                if (state.dataEnvrn->IsRain) {                 // Raining: since wind exposed, outside window surface gets wet
                    Tout = state.dataSurface->SurfOutWetBulbTemp(SurfNum) + Constant::Kelvin;
                } else { // Dry
                    Tout = state.dataSurface->SurfOutDryBulbTemp(SurfNum) + Constant::Kelvin;
                }
            } else { // Window not exposed to wind
                Tout = state.dataSurface->SurfOutDryBulbTemp(SurfNum) + Constant::Kelvin;
            }
            tsky = state.dataEnvrn->SkyTempKelvin;
            Ebout = Constant::StefanBoltzmann * pow_4(Tout);
            // ASHWAT model may be slightly different
            outir = state.dataSurface->Surface(SurfNum).ViewFactorSkyIR *
                        (state.dataSurface->SurfAirSkyRadSplit(SurfNum) * Constant::StefanBoltzmann * pow_4(tsky) +
                         (1.0 - state.dataSurface->SurfAirSkyRadSplit(SurfNum)) * Ebout) +
                    state.dataSurface->Surface(SurfNum).ViewFactorGroundIR * Ebout + OutSrdIR;
        }
    }
    // Outdoor conditions
    TRMOUT = root_4(outir / Constant::StefanBoltzmann); // it is in Kelvin scale
    // indoor conditions
    LWAbsIn = EffectiveEPSLB(state.dataWindowEquivLayer->CFS(EQLNum));  // windows inside face effective thermal emissivity
    LWAbsOut = EffectiveEPSLF(state.dataWindowEquivLayer->CFS(EQLNum)); // windows outside face effective thermal emissivity
    SurfOutsideEmiss = LWAbsOut;
    // Indoor mean radiant temperature.
    // IR incident on window from zone surfaces and high-temp radiant sources
    rmir = state.dataSurface->SurfWinIRfromParentZone(SurfNum) + state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(SurfNum) +
           state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum);
    TRMIN = root_4(rmir / Constant::StefanBoltzmann); // TODO check model equation.

    NL = state.dataWindowEquivLayer->CFS(EQLNum).NL;
    QAllSWwinAbs({1, NL + 1}) = state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, {1, NL + 1});
    //  Solve energy balance(s) for temperature at each node/layer and
    //  heat flux, including components, between each pair of nodes/layers
    ASHWAT_ThermalCalc(state,
                       state.dataWindowEquivLayer->CFS(EQLNum),
                       TIN,
                       Tout,
                       HcIn,
                       HcOut,
                       TRMOUT,
                       TRMIN,
                       QAllSWwinAbs({1, NL + 1}),
                       TOL,
                       QOCF,
                       QOCFRoom,
                       T,
                       Q,
                       JF,
                       JB,
                       H);

    // effective surface temperature is set to surface temperature calculated
    // by the fenestration layers temperature solver
    SurfInsideTemp = T(NL) - Constant::Kelvin;
    // Convective to room
    QCONV = H(NL) * (T(NL) - TIN);
    // Other convective = total conv - standard model prediction
    QXConv = QCONV - HcIn * (SurfInsideTemp - TaIn);
    // Save the extra convection term. This term is added to the zone air heat
    // balance equation
    state.dataSurface->SurfWinOtherConvHeatGain(SurfNum) = state.dataSurface->Surface(SurfNum).Area * QXConv;
    SurfOutsideTemp = T(1) - Constant::Kelvin;
    // Various reporting calculations
    InSideLayerType = state.dataWindowEquivLayer->CFS(EQLNum).L(NL).LTYPE;
    if (InSideLayerType == LayerType::GLAZE) {
        ConvHeatFlowNatural = 0.0;
    } else {
        ConvHeatFlowNatural = state.dataSurface->Surface(SurfNum).Area * QOCFRoom;
    }
    state.dataSurface->SurfWinEffInsSurfTemp(SurfNum) = SurfInsideTemp;
    NetIRHeatGainWindow =
        state.dataSurface->Surface(SurfNum).Area * LWAbsIn * (Constant::StefanBoltzmann * pow_4(SurfInsideTemp + Constant::Kelvin) - rmir);
    ConvHeatGainWindow = state.dataSurface->Surface(SurfNum).Area * HcIn * (SurfInsideTemp - TaIn);
    // Window heat gain (or loss) is calculated here
    state.dataSurface->SurfWinHeatGain(SurfNum) =
        state.dataSurface->SurfWinTransSolar(SurfNum) + ConvHeatGainWindow + NetIRHeatGainWindow + ConvHeatFlowNatural -
        (state.dataSurface->SurfWinLossSWZoneToOutWinRep(SurfNum) +
         state.dataHeatBalSurf->SurfWinInitialDifSolInTrans(SurfNum) * state.dataSurface->Surface(SurfNum).Area);
    state.dataSurface->SurfWinConvHeatFlowNatural(SurfNum) = ConvHeatFlowNatural;
    state.dataSurface->SurfWinGainConvShadeToZoneRep(SurfNum) = ConvHeatGainWindow;
    state.dataSurface->SurfWinGainIRGlazToZoneRep(SurfNum) = NetIRHeatGainWindow;
    state.dataSurface->SurfWinGainIRShadeToZoneRep(SurfNum) = NetIRHeatGainWindow;
    if (InSideLayerType == LayerType::GLAZE) {
        // no interior shade
        state.dataSurface->SurfWinGainIRShadeToZoneRep(SurfNum) = 0.0;
    } else {
        // Interior shade exists
        state.dataSurface->SurfWinGainIRGlazToZoneRep(SurfNum) = 0.0;
    }
}

void OPENNESS_LW(Real64 const OPENNESS, // shade openness (=tausbb at normal incidence)
                 Real64 const EPSLW0,   // apparent LW emittance of shade at 0 openness
                 Real64 const TAULW0,   // apparent LW transmittance of shade at 0 openness
                 Real64 &EPSLW,         // returned: effective LW emittance of shade
                 Real64 &TAULW          // returned: effective LW transmittance of shade
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright and Nathan Kotey, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    // Modifies long wave properties for shade types characterized by openness.
    // Applies to shade type: insect screen, roller blind, drape fabric

    //   (= wire or thread emittance)
    //   typical (default) values
    //      dark insect screen = .93
    //      metallic insect screen = .32
    //      roller blinds = .91
    //      drape fabric = .87
    //   typical (default) values
    //      dark insect screen = .02
    //      metallic insect screen = .19
    //      roller blinds = .05
    //      drape fabric = .05

    EPSLW = EPSLW0 * (1.0 - OPENNESS);
    TAULW = TAULW0 * (1.0 - OPENNESS) + OPENNESS;
}

Real64 P01(EnergyPlusData &state,
           Real64 const P,             // property
           std::string_view const WHAT // identifier for err msg
)
{
    //       AUTHOR         ASHRAE 1311-RP
    //       MODIFIED       Bereket Nigusse, May 2013
    //                      Added error messages

    // PURPOSE OF THIS FUNCTION:
    // Constrains property to range 0 - 1

    static constexpr std::string_view RoutineName("P01: ");

    if (P < -0.05 || P > 1.05) {
        ShowWarningMessage(state, format("{}property value should have been between 0 and 1", RoutineName));
        ShowContinueError(state, format("{}=:  property value is ={:.4T}", WHAT, P));
        if (P < 0.0) {
            ShowContinueError(state, "property value is reset to 0.0");
        } else if (P > 1.0) {
            ShowContinueError(state, "property value is reset to 1.0");
        }
    }
    return max(0.0, min(1.0, P));
}

Real64
HEMINT(EnergyPlusData &state,
       std::function<Real64(EnergyPlusData &state, Real64 const THETA, int const OPT, const Array1D<Real64> &)> F, // property integrand function
       int const F_Opt,           // options passed to F() (hipRHO, hipTAU)
       const Array1D<Real64> &F_P // parameters passed to F()
)
{
    //       AUTHOR         ASHRAE 1311-RP

    // PURPOSE OF THIS FUNCTION:
    // Romberg Integration of Property function over hemispherical dome
    // METHODOLOGY EMPLOYED:
    //  Romberg Integration.

    // Argument array dimensioning
    EP_SIZE_CHECK(F_P, state.dataWindowEquivalentLayer->hipDIM);

    constexpr Real64 KMAX(8); // max steps
    int const NPANMAX(std::pow(2, KMAX));
    Real64 constexpr TOL(0.0005); // convergence tolerance
    static constexpr std::string_view RoutineName("HEMINT");

    Array2D<Real64> T(KMAX, KMAX);
    Real64 FX;
    Real64 X;
    Real64 DX;
    Real64 DIFF;
    int K;

    Real64 X1 = 0.0; // integration limits
    Real64 X2 = Constant::PiOvr2;
    int nPan = 1;
    Real64 SUM = 0.0;
    for (K = 1; K <= KMAX; ++K) {
        DX = (X2 - X1) / nPan;
        int iPX = NPANMAX / nPan;
        for (int I = 0; I <= nPan; ++I) {
            if (K == 1 || mod(I * iPX, iPX * 2) != 0) {
                //   evaluate integrand function for new X values
                //   2 * sin( x) * cos( x) covers hemisphere with single integral
                X = X1 + I * DX;
                FX = 2.0 * std::sin(X) * std::cos(X) * F(state, X, F_Opt, F_P);
                if (K == 1) {
                    FX /= 2.0;
                }
                SUM += FX;
            }
        }

        T(K, 1) = DX * SUM;
        // trapezoid result - i.e., first column Romberg entry
        // Now complete the row
        if (K > 1) {
            for (int L = 2; L <= K; ++L) {
                Real64 const pow_4_L_1(std::pow(4.0, L - 1));
                T(K, L) = (pow_4_L_1 * T(K, L - 1) - T(K - 1, L - 1)) / (pow_4_L_1 - 1.0);
            }
            //    check for convergence
            //    do 8 panels minimum, else can miss F() features
            if (nPan >= 8) {
                DIFF = std::abs(T(K, K) - T(K - 1, K - 1));
                if (DIFF < TOL) {
                    break;
                }
            }
        }
        nPan *= 2;
    }
    if (K > KMAX) {
        K = KMAX;
    }
    return P01(state, T(K, K), RoutineName);
}

void RB_DIFF(EnergyPlusData &state,
             Real64 const RHO_BT0, // normal incidence beam-total reflectance
             Real64 const TAU_BT0, // normal incidence beam-total transmittance
             Real64 const TAU_BB0, // normal incidence beam-beam transmittance
             Real64 &RHO_DD,       // returned: diffuse-diffuse reflectance
             Real64 &TAU_DD        // returned: diffuse-diffuse transmittance
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright and Nathan Kotey, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates roller blind diffuse-diffuse solar optical properties by integrating
    // the corresponding properties over the hemisphere

    static constexpr std::string_view RoutineName("RB_DIFF: ");

    Array1D<Real64> P(state.dataWindowEquivalentLayer->hipDIM);
    Real64 SumRefAndTran; // sum of the reflectance and transmittance

    RHO_DD = RHO_BT0;
    P(state.dataWindowEquivalentLayer->hipRHO_BT0) = RHO_BT0;
    P(state.dataWindowEquivalentLayer->hipTAU_BT0) = TAU_BT0;
    P(state.dataWindowEquivalentLayer->hipTAU_BB0) = TAU_BB0;

    TAU_DD = HEMINT(state, RB_F, 0, P);

    if (RHO_DD + TAU_DD > 1.0) {
        SumRefAndTran = RHO_DD + TAU_DD;
        ShowWarningMessage(state, format("{}Roller blind diffuse-diffuse properties are inconsistent", RoutineName));
        ShowContinueError(state, format("...The diffuse-diffuse reflectance = {:.4T}", RHO_DD));
        ShowContinueError(state, format("...The diffuse-diffuse transmittance = {:.4T}", TAU_DD));
        ShowContinueError(state, format("...Sum of diffuse reflectance and transmittance = {:.4T}", SumRefAndTran));
        ShowContinueError(state, "...This sum cannot be > 1.0. Transmittance will be reset to 1 minus reflectance");
        TAU_DD = 1.0 - RHO_DD;
    }
}

Real64 RB_F(EnergyPlusData &state,
            Real64 const THETA,             // incidence angle, radians
            [[maybe_unused]] int const OPT, // options (unused)
            const Array1D<Real64> &P        // parameters
)
{
    //       AUTHOR         ASHRAE 1311-RP

    // PURPOSE OF THIS FUNCTION:
    //  Roller blind integrand

    // Argument array dimensioning
    EP_SIZE_CHECK(P, state.dataWindowEquivalentLayer->hipDIM);

    Real64 RHO_BD;
    Real64 TAU_BB;
    Real64 TAU_BD;

    RB_BEAM(state,
            THETA,
            P(state.dataWindowEquivalentLayer->hipRHO_BT0),
            P(state.dataWindowEquivalentLayer->hipTAU_BT0),
            P(state.dataWindowEquivalentLayer->hipTAU_BB0),
            RHO_BD,
            TAU_BB,
            TAU_BD);

    return TAU_BB + TAU_BD;
}

void RB_BEAM(EnergyPlusData &state,
             Real64 const xTHETA,  // angle of incidence, radians (0 - PI/2)
             Real64 const RHO_BT0, // normal incidence beam-total front reflectance
             Real64 const TAU_BT0, // normal incidence beam-total transmittance
             Real64 const TAU_BB0, // normal incidence beam-beam transmittance
             Real64 &RHO_BD,       // returned: beam-diffuse front reflectance
             Real64 &TAU_BB,       // returned: beam-beam transmittance
             Real64 &TAU_BD        // returned: beam-diffuse transmittance
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the roller blind off-normal properties using semi-empirical relations

    // SUBROUTINE ARGUMENT DEFINITIONS:
    //   TAU_BT0 = TAU_BB0 + TAU_BD0
    //   (openness)

    static constexpr std::string_view ContextName("RB_BEAM TauBD");

    Real64 THETA;        // working angle of incidence (limited < 90 deg)
    Real64 TAUM0;        // apparent blind material transmittance at normal incidence
    Real64 THETA_CUTOFF; // cutoff angle, radians (angle beyond which total transmittance goes to zero)
    Real64 TAUBT_EXPO;   // exponent in the beam-total transmittance model
    Real64 TAUBB_EXPO;   // exponent in the beam-beam transmittance model
    Real64 TAU_BT;       // beam-total transmittance

    THETA = min(89.99 * Constant::DegToRad, xTHETA);

    if (TAU_BB0 > 0.9999) {
        TAU_BB = 1.0;
        TAU_BT = 1.0;
    } else {
        // beam total
        TAUM0 = min(1.0, (TAU_BT0 - TAU_BB0) / (1.0 - TAU_BB0));
        if (TAUM0 <= 0.33) {
            TAUBT_EXPO = 0.133 * std::pow(TAUM0 + 0.003, -0.467);
        } else {
            TAUBT_EXPO = 0.33 * (1.0 - TAUM0);
        }
        TAU_BT = TAU_BT0 * std::pow(std::cos(THETA), TAUBT_EXPO); // always 0 - 1

        Real64 const cos_TAU_BB0(std::cos(TAU_BB0 * Constant::PiOvr2));
        THETA_CUTOFF = Constant::DegToRad * (90.0 - 25.0 * cos_TAU_BB0);
        if (THETA >= THETA_CUTOFF) {
            TAU_BB = 0.0;
        } else {
            TAUBB_EXPO = 0.6 * std::pow(cos_TAU_BB0, 0.3);
            TAU_BB = TAU_BB0 * std::pow(std::cos(Constant::PiOvr2 * THETA / THETA_CUTOFF), TAUBB_EXPO);
            // BB correlation can produce results slightly larger than BT
            // Enforce consistency
            TAU_BB = min(TAU_BT, TAU_BB);
        }
    }

    RHO_BD = RHO_BT0;
    TAU_BD = P01(state, TAU_BT - TAU_BB, ContextName);
}

void IS_DIFF(EnergyPlusData &state,
             Real64 const RHO_BT0, // normal incidence beam-total reflectance
             Real64 const TAU_BT0, // normal incidence beam-total transmittance
             Real64 const TAU_BB0, // normal incidence beam-beam transmittance
             Real64 &RHO_DD,       // returned: diffuse-diffuse reflectance
             Real64 &TAU_DD        // returned: diffuse-diffuse transmittance
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates insect screen diffuse-diffuse solar optical properties by integrating
    // the corresponding properties over the hemisphere

    // SUBROUTINE ARGUMENT DEFINITIONS:
    //   TAU_BT0 = TAU_BB0 + TAU_BD0
    Array1D<Real64> P(state.dataWindowEquivalentLayer->hipDIM);

    static constexpr std::string_view RoutineName("IS_DIFF: ");

    Real64 SumRefAndTran;

    P(state.dataWindowEquivalentLayer->hipRHO_BT0) = RHO_BT0;
    P(state.dataWindowEquivalentLayer->hipTAU_BT0) = TAU_BT0;
    P(state.dataWindowEquivalentLayer->hipTAU_BB0) = TAU_BB0;

    RHO_DD = HEMINT(state, IS_F, state.dataWindowEquivalentLayer->hipRHO, P);
    TAU_DD = HEMINT(state, IS_F, state.dataWindowEquivalentLayer->hipTAU, P);

    if (RHO_DD + TAU_DD > 1.0) {
        SumRefAndTran = RHO_DD + TAU_DD;
        ShowWarningMessage(state, format("{}Calculated insect screen diffuse-diffuse properties are inconsistent", RoutineName));
        ShowContinueError(state, format("...The diffuse-diffuse reflectance = {:.4T}", RHO_DD));
        ShowContinueError(state, format("...The diffuse-diffuse transmittance = {:.4T}", TAU_DD));
        ShowContinueError(state, format("...Sum of diffuse reflectance and transmittance = {:.4T}", SumRefAndTran));
        ShowContinueError(state, "...This sum cannot be > 1.0. Transmittance will be reset to 1 minus reflectance");
        TAU_DD = 1.0 - RHO_DD;
    }
}

Real64 IS_F(EnergyPlusData &state,
            Real64 const THETA,      // incidence angle, radians
            int const OPT,           // options (1=reflectance, 2=transmittance)
            const Array1D<Real64> &P // parameters
)
{
    //       AUTHOR         ASHRAE 1311-RP

    // PURPOSE OF THIS FUNCTION:
    //  Insect screen integrand

    // Argument array dimensioning
    EP_SIZE_CHECK(P, state.dataWindowEquivalentLayer->hipDIM);

    Real64 RHO_BD;
    Real64 TAU_BB;
    Real64 TAU_BD;

    IS_BEAM(state,
            THETA,
            P(state.dataWindowEquivalentLayer->hipRHO_BT0),
            P(state.dataWindowEquivalentLayer->hipTAU_BT0),
            P(state.dataWindowEquivalentLayer->hipTAU_BB0),
            RHO_BD,
            TAU_BB,
            TAU_BD);

    if (OPT == state.dataWindowEquivalentLayer->hipRHO) {
        return RHO_BD;
    } else if (OPT == state.dataWindowEquivalentLayer->hipTAU) {
        return TAU_BB + TAU_BD;
    } else {
        return -1.0;
    }
}

void IS_BEAM(EnergyPlusData &state,
             Real64 const xTHETA,  // incidence angle, radians (0 - PI/2)
             Real64 const RHO_BT0, // beam-total reflectance
             Real64 const TAU_BT0, // beam-total transmittance at normal incidence
             Real64 const TAU_BB0, // beam-beam transmittance at normal incidence
             Real64 &RHO_BD,       // returned: beam-diffuse reflectance
             Real64 &TAU_BB,       // returned: beam-beam transmittance
             Real64 &TAU_BD        // returned: beam-diffuse transmittance
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates insect screen off-normal solar optical properties
    // using semi-empirical relations.

    // SUBROUTINE ARGUMENT DEFINITIONS:
    //   TAU_BTO = TAU_BB0 + TAU_BD0

    static constexpr std::string_view RhoBD_Name("IS_BEAM RhoBD");
    static constexpr std::string_view TauBB_Name("IS_BEAM TauBB");
    static constexpr std::string_view TauBT_Name("IS_BEAM TauBT");
    static constexpr std::string_view TauBD_Name("IS_BEAM TauBD");

    Real64 THETA_CUTOFF; // cutoff angle, radians (beyond which TAU_BB = 0)
    Real64 B;            // working temp
    Real64 RHO_W;        // apparent wire reflectance
    Real64 RHO_BT90;     // beam-total reflectance at 90 deg incidence
    Real64 TAU_BT;       // beam-total transmittance

    Real64 const THETA(min(89.99 * Constant::DegToRad, xTHETA)); // working incident angle, radians
    Real64 const COSTHETA(std::cos(THETA));

    RHO_W = RHO_BT0 / max(0.00001, 1.0 - TAU_BB0);
    B = -0.45 * std::log(max(RHO_W, 0.01));

    RHO_BT90 = RHO_BT0 + (1.0 - RHO_BT0) * (0.35 * RHO_W);

    RHO_BD = P01(state, RHO_BT0 + (RHO_BT90 - RHO_BT0) * (1.0 - std::pow(COSTHETA, B)), RhoBD_Name);

    if (TAU_BT0 < 0.00001) {
        TAU_BB = 0.0;
        TAU_BT = 0.0;
    } else {
        THETA_CUTOFF = std::acos(IS_DSRATIO(TAU_BB0));

        if (THETA >= THETA_CUTOFF) {
            TAU_BB = 0.0;
        } else {
            B = -0.45 * std::log(max(TAU_BB0, 0.01)) + 0.1;
            TAU_BB = P01(state, TAU_BB0 * std::pow(std::cos(Constant::PiOvr2 * THETA / THETA_CUTOFF), B), TauBB_Name);
        }

        B = -0.65 * std::log(max(TAU_BT0, 0.01)) + 0.1;
        TAU_BT = P01(state, TAU_BT0 * std::pow(COSTHETA, B), TauBT_Name);
    }

    TAU_BD = P01(state, TAU_BT - TAU_BB, TauBD_Name);
}

Real64 IS_OPENNESS(Real64 const D, // wire diameter
                   Real64 const S  // wire spacing
)
{
    //       AUTHOR         ASHRAE 1311-RP

    // PURPOSE OF THIS FUNCTION:
    //  Returns openness from wire geometry.

    if (S > 0.0) {
        return pow_2(max(S - D, 0.0) / S);
    } else {
        return 0.0;
    }
}

Real64 IS_DSRATIO(Real64 const OPENNESS) // openness
{
    //       AUTHOR         ASHRAE 1311-RP

    // PURPOSE OF THIS FUNCTION:
    //  Returns ratio of diameter to spacing

    if (OPENNESS > 0.0) {
        return 1.0 - min(std::sqrt(OPENNESS), 1.0);
    } else {
        return 0.0;
    }
}

void FM_DIFF(EnergyPlusData &state,
             Real64 const RHO_BT0, // fabric beam-total reflectance at normal incidence
             Real64 const TAU_BT0, // fabric beam-total transmittance at normal incidence
             Real64 const TAU_BB0, // forward facing fabric beam-beam transmittance at normal incidence
             Real64 &RHO_DD,       // returned: fabric diffuse-diffuse reflectance
             Real64 &TAU_DD        // returned: fabric diffuse-diffuse transmittance
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates drape fabric diffuse-diffuse solar optical properties by integrating
    // the corresponding beam properties over the hemisphere.

    // SUBROUTINE ARGUMENT DEFINITIONS:
    //   (TAU_BT0 = TAU_BB0 + TAU_BD0)
    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("FM_DIFF: ");

    Real64 TAU_BD0;
    Array1D<Real64> P(state.dataWindowEquivalentLayer->hipDIM);
    Real64 SumRefAndTran;

    TAU_BD0 = TAU_BT0 - TAU_BB0;

    P(state.dataWindowEquivalentLayer->hipRHO_BT0) = RHO_BT0;
    P(state.dataWindowEquivalentLayer->hipTAU_BT0) = TAU_BT0;
    P(state.dataWindowEquivalentLayer->hipTAU_BB0) = TAU_BB0;

    RHO_DD = HEMINT(state, FM_F, state.dataWindowEquivalentLayer->hipRHO, P);
    TAU_DD = HEMINT(state, FM_F, state.dataWindowEquivalentLayer->hipTAU, P);

    if (RHO_DD + TAU_DD > 1.0) {
        SumRefAndTran = RHO_DD + TAU_DD;
        ShowWarningMessage(state, format("{}Calculated drape fabric diffuse-diffuse properties are inconsistent", RoutineName));
        ShowContinueError(state, format("...The diffuse-diffuse reflectance = {:.4T}", RHO_DD));
        ShowContinueError(state, format("...The diffuse-diffuse transmittance = {:.4T}", TAU_DD));
        ShowContinueError(state, format("...Sum of diffuse reflectance and transmittance = {:.4T}", SumRefAndTran));
        ShowContinueError(state, "...This sum cannot be > 1.0. Transmittance will be reset to 1 minus reflectance");
        TAU_DD = 1.0 - RHO_DD;
    }
}

Real64 FM_F(EnergyPlusData &state,
            Real64 const THETA,      // incidence angle, radians
            int const Opt,           // options (hipRHO, hipTAU)
            const Array1D<Real64> &P // parameters
)
{
    //       AUTHOR         ASHRAE 1311-RP

    // PURPOSE OF THIS FUNCTION:
    //  Drape fabric property integrand.

    // Argument array dimensioning
    EP_SIZE_CHECK(P, state.dataWindowEquivalentLayer->hipDIM);

    Real64 RHO_BD;
    Real64 TAU_BB;
    Real64 TAU_BD;

    FM_BEAM(state,
            THETA,
            P(state.dataWindowEquivalentLayer->hipRHO_BT0),
            P(state.dataWindowEquivalentLayer->hipTAU_BT0),
            P(state.dataWindowEquivalentLayer->hipTAU_BB0),
            RHO_BD,
            TAU_BB,
            TAU_BD);

    if (Opt == state.dataWindowEquivalentLayer->hipRHO) {
        return RHO_BD;
    } else if (Opt == state.dataWindowEquivalentLayer->hipTAU) {
        return TAU_BB + TAU_BD;
    } else {
        return -1.0;
    }
}

void FM_BEAM(EnergyPlusData &state,
             Real64 const xTHETA,  // incidence angle, radians (0 - PI/2)
             Real64 const RHO_BT0, // fabric beam-total reflectance
             Real64 const TAU_BT0, // fabric beam-total transmittance at normal incidence
             Real64 const TAU_BB0, // fabric beam-beam transmittance at normal incidence
             Real64 &RHO_BD,       // returned: fabric beam-diffuse reflectance
             Real64 &TAU_BB,       // returned: fabric beam-beam transmittance
             Real64 &TAU_BD        // returned: fabric beam-diffuse transmittance
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the solar optical properties of a fabric for beam radiation incident
    // on the forward facing surface using optical properties at normal incidence and
    // semi-empirical relations.

    // SUBROUTINE ARGUMENT DEFINITIONS:
    //   TAU_BTO = TAU_BB0 + TAU_BD0
    //   = openness

    Real64 THETA; // working incident angle, radians
    Real64 R;     // working temps
    Real64 B;
    Real64 RHO_Y;    // apparent yarn reflectance
    Real64 RHO_BT90; // beam-total reflectance at 90 deg incidence
    Real64 TAU_BT;   // beam-total transmittance

    THETA = std::abs(max(-89.99 * Constant::DegToRad, min(89.99 * Constant::DegToRad, xTHETA)));
    // limit -89.99 - +89.99
    // by symmetry, optical properties same at +/- theta
    Real64 const COSTHETA(std::cos(THETA));

    RHO_Y = RHO_BT0 / max(0.00001, 1.0 - TAU_BB0);
    R = 0.7 * std::pow(RHO_Y, 0.7);
    RHO_BT90 = RHO_BT0 + (1.0 - RHO_BT0) * R;
    B = 0.6;
    RHO_BD = P01(state, RHO_BT0 + (RHO_BT90 - RHO_BT0) * (1.0 - std::pow(COSTHETA, B)), "FM_BEAM RhoBD");

    if (TAU_BT0 < 0.00001) {
        TAU_BB = 0.0;
        TAU_BD = 0.0;
    } else {
        B = max(-0.5 * std::log(max(TAU_BB0, 0.01)), 0.35);
        TAU_BB = TAU_BB0 * std::pow(COSTHETA, B);

        B = max(-0.5 * std::log(max(TAU_BT0, 0.01)), 0.35);
        TAU_BT = TAU_BT0 * std::pow(COSTHETA, B);

        TAU_BD = P01(state, TAU_BT - TAU_BB, "FM_BEAM TauBD");
    }
}

void PD_LW(EnergyPlusData &state,
           Real64 const S,               // pleat spacing (> 0)
           Real64 const W,               // pleat depth (>=0, same units as S)
           Real64 const OPENNESS_FABRIC, // fabric openness, 0-1 (=tausbb at normal incidence)
           Real64 const EPSLWF0_FABRIC,  // fabric LW front emittance at 0 openness
           Real64 const EPSLWB0_FABRIC,  // fabric LW back emittance at 0 openness
           Real64 const TAULW0_FABRIC,   // fabric LW transmittance at 0 openness
           Real64 &EPSLWF_PD,            // returned: drape front effective LW emittance
           Real64 &TAULW_PD              // returned: drape effective LW transmittance
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  Calculates the effective longwave emittance and transmittance of a drapery layer

    // SUBROUTINE ARGUMENT DEFINITIONS:
    //    typical (default) = 0.92
    //    typical (default) = 0.92
    //    nearly always 0
    static constexpr std::string_view RhoLWF_Name("PD_LW RhoLWF");
    static constexpr std::string_view RhoLWB_Name("PD_LW RhoLWB");
    static constexpr std::string_view EpsLWF_Name("PD_LW EpsLWF");

    Real64 RHOLWF_FABRIC;
    Real64 RHOLWB_FABRIC;
    Real64 TAULW_FABRIC;
    Real64 EPSLWF_FABRIC;
    Real64 EPSLWB_FABRIC;
    Real64 TAULX;
    Real64 RHOLWF_PD;

    OPENNESS_LW(OPENNESS_FABRIC, EPSLWF0_FABRIC, TAULW0_FABRIC, EPSLWF_FABRIC, TAULW_FABRIC);
    OPENNESS_LW(OPENNESS_FABRIC, EPSLWB0_FABRIC, TAULW0_FABRIC, EPSLWB_FABRIC, TAULX);

    RHOLWF_FABRIC = P01(state, 1.0 - EPSLWF_FABRIC - TAULW_FABRIC, RhoLWF_Name);
    RHOLWB_FABRIC = P01(state, 1.0 - EPSLWB_FABRIC - TAULW_FABRIC, RhoLWB_Name);

    PD_DIFF(state, S, W, RHOLWF_FABRIC, RHOLWB_FABRIC, TAULW_FABRIC, RHOLWF_PD, TAULW_PD);

    EPSLWF_PD = P01(state, 1.0 - TAULW_PD - RHOLWF_PD, EpsLWF_Name);
}

void PD_DIFF(EnergyPlusData &state,
             Real64 const S,        // pleat spacing (> 0)
             Real64 const W,        // pleat depth (>=0, same units as S)
             Real64 const RHOFF_DD, // fabric front diffuse-diffuse reflectance
             Real64 const RHOBF_DD, // fabric back diffuse-diffuse reflectance
             Real64 const TAUF_DD,  // fabric diffuse-diffuse transmittance
             Real64 &RHOFDD,        // returned: drape diffuse-diffuse reflectance
             Real64 &TAUFDD         // returned: drape diffuse-diffuse transmittance
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  Calculates the effective diffuse transmittance and reflectance of a drapery layer.
    //  Used for both LW and solar diffuse.
    // METHODOLOGY EMPLOYED:
    // Eight surface flat-fabric model with rectangular enclosure. If you want the back-side
    // reflectance call this routine a second time with reversed front and back properties

    constexpr int N(6);
    static constexpr std::string_view TauDD_Name("PD_DIFF TauDD");
    static constexpr std::string_view RhoDD_Name("PD_DIFF RhoDD");

    Real64 AK; // length of diagonal strings of the rectangular enclosure
    Real64 CG;
    Real64 F12; // shape factors
    Real64 F14;
    Real64 F32;
    Real64 F21;
    Real64 F31;
    Real64 F34;
    Real64 F24;
    Real64 F41;
    Real64 F42;
    Real64 F57;
    Real64 F56;
    Real64 F58;
    Real64 F67;
    Real64 F65;
    Real64 F68;
    Real64 F75;
    Real64 F76;
    Real64 F78;
    Real64 F85;
    Real64 F87;
    Real64 F86;
    Real64 J1; // radiosity, surface i
    Real64 J2;
    Real64 J4;
    Real64 J7;
    Real64 J6;
    Real64 J8;
    Real64 G1; // irradiance, surface i
    Real64 G3;
    Real64 G5;
    Real64 G7;
    Array2D<Real64> A(N + 2, N);
    Array1D<Real64> XSOL(N);

    if (W / S < state.dataWindowEquivalentLayer->SMALL_ERROR) {
        // flat drape (no pleats)
        RHOFDD = RHOFF_DD;
        TAUFDD = TAUF_DD;
        return;
    }

    // SOLVE FOR DIAGONAL STRINGS AND SHAPE FACTORS

    AK = std::sqrt(S * S + W * W);
    CG = AK;
    F12 = (S + W - AK) / (2.0 * S);
    F14 = (S + W - CG) / (2.0 * S);
    F32 = F14;
    F31 = (AK + CG - 2.0 * W) / (2.0 * S);
    F34 = F12;
    F21 = (S + W - AK) / (2.0 * W);
    F24 = (AK + CG - 2.0 * S) / (2.0 * W);
    F41 = (S + W - CG) / (2.0 * W);
    F42 = F24;
    F57 = F31;
    F56 = F12;
    F58 = F14;
    F75 = F31;
    F76 = F32;
    F78 = F34;
    F67 = F41;
    F65 = F21;
    F68 = F24;
    F85 = F41;
    F87 = F21;
    F86 = F42;

    A = 0.0;    // INITIALIZE RADIOSITY MATRIX COEFFICIENTS
    XSOL = 0.0; // INITIALIZE SOLUTION VECTOR COEFFICIENTS

    // POPULATE THE COEFFICIENTS OF THE RADIOSITY MATRIX

    A(1, 1) = 1.0;
    A(2, 1) = -RHOBF_DD * F12;
    A(3, 1) = -RHOBF_DD * F14;
    A(4, 1) = 0.0;
    A(5, 1) = 0.0;
    A(6, 1) = 0.0;
    A(7, 1) = TAUF_DD;
    A(1, 2) = -RHOBF_DD * F21;
    A(2, 2) = 1.0;
    A(3, 2) = -RHOBF_DD * F24;
    A(4, 2) = -TAUF_DD * F87;
    A(5, 2) = -TAUF_DD * F86;
    A(6, 2) = 0.0;
    A(7, 2) = TAUF_DD * F85;
    A(1, 3) = -RHOBF_DD * F41;
    A(2, 3) = -RHOBF_DD * F42;
    A(3, 3) = 1.0;
    A(4, 3) = -TAUF_DD * F67;
    A(5, 3) = 0.0;
    A(6, 3) = -TAUF_DD * F68;
    A(7, 3) = TAUF_DD * F65;
    A(1, 4) = 0.0;
    A(2, 4) = 0.0;
    A(3, 4) = 0.0;
    A(4, 4) = 1.0;
    A(5, 4) = -RHOFF_DD * F76;
    A(6, 4) = -RHOFF_DD * F78;
    A(7, 4) = RHOFF_DD * F75;
    A(1, 5) = -TAUF_DD * F41;
    A(2, 5) = -TAUF_DD * F42;
    A(3, 5) = 0.0;
    A(4, 5) = -RHOFF_DD * F67;
    A(5, 5) = 1.0;
    A(6, 5) = -RHOFF_DD * F68;
    A(7, 5) = RHOFF_DD * F65;
    A(1, 6) = -TAUF_DD * F21;
    A(2, 6) = 0.0;
    A(3, 6) = -TAUF_DD * F24;
    A(4, 6) = -RHOFF_DD * F87;
    A(5, 6) = -RHOFF_DD * F86;
    A(6, 6) = 1.0;
    A(7, 6) = RHOFF_DD * F85;

    SOLMATS(N, A, XSOL);

    J1 = XSOL(1);
    J2 = XSOL(2);
    J4 = XSOL(3);
    J7 = XSOL(4);
    J6 = XSOL(5);
    J8 = XSOL(6);

    G1 = F12 * J2 + F14 * J4;
    G3 = F32 * J2 + F31 * J1 + F34 * J4;
    G5 = F57 * J7 + F56 * J6 + F58 * J8;
    G7 = F75 + F76 * J6 + F78 * J8;

    TAUFDD = P01(state, (G3 + TAUF_DD * G7) / 2.0, TauDD_Name);
    RHOFDD = P01(state, (RHOFF_DD + TAUF_DD * G1 + G5) / 2.0, RhoDD_Name);
}

void PD_BEAM(EnergyPlusData &state,
             Real64 const S,         // pleat spacing (> 0)
             Real64 const W,         // pleat depth (>=0, same units as S)
             Real64 const OHM_V_RAD, // vertical profile angle, radians +=above horiz
             Real64 const OHM_H_RAD, // horizontal profile angle, radians=clockwise when viewed from above
             Real64 const RHOFF_BT0, // beam total reflectance front (outside)
             Real64 const TAUFF_BB0, // beam beam transmittance front (outside)
             Real64 const TAUFF_BD0, // beam diffuse transmittance front (outside)
             Real64 const RHOFF_DD,  // diffuse-diffuse reflectance front (outside)
             Real64 const TAUFF_DD,  // diffuse-diffuse transmittance front (outside)
             Real64 const RHOBF_BT0, // beam total reflectance back (inside)
             Real64 const TAUBF_BB0, // beam beam total transmittance back (inside)
             Real64 const TAUBF_BD0, // beam diffuse transmittance back (inside)
             Real64 const RHOBF_DD,  // diffuse-diffuse reflectance front (outside)
             Real64 const TAUBF_DD,  // diffuse-diffuse transmittance front (outside)
             Real64 &RHO_BD,         // returned: drape front beam-diffuse reflectance
             Real64 &TAU_BB,         // returned: drape beam-beam transmittance
             Real64 &TAU_BD          // returned: drape beam-diffuse transmittance
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  calculates the effective front-side solar optical properties of a drapery layer.
    // METHODOLOGY EMPLOYED:
    // Pleated drape flat-fabric model with rectangular enclosure

    Real64 DE; // length of directly illuminated surface on side of pleat that
    //   is open on front (same units as S and W)
    Real64 EF;      // length of pleat side shaded surface (W-DE) (same units as S and W)
    Real64 OMEGA_V; // profile angles limited to +/- PI/2
    Real64 OMEGA_H;
    Real64 TAUFF_BT0;
    Real64 TAUBF_BT0;
    Real64 THETA_PARL; // beam incidence angles on pleat surface parallel / perpendicular
    Real64 THETA_PERP;
    // to window plane
    Real64 RHOFF_BT_PARL;
    Real64 TAUFF_BB_PARL;
    Real64 TAUFF_BD_PARL;
    Real64 RHOBF_BT_PARL;
    Real64 TAUBF_BB_PARL;
    Real64 TAUBF_BD_PARL;
    Real64 RHOFF_BT_PERP;
    Real64 TAUFF_BB_PERP;
    Real64 TAUFF_BD_PERP;
    Real64 RHOBF_BT_PERP;
    Real64 TAUBF_BB_PERP;
    Real64 TAUBF_BD_PERP;

    OMEGA_V = std::abs(max(-89.5 * Constant::DegToRad, min(89.5 * Constant::DegToRad, OHM_V_RAD)));
    OMEGA_H = std::abs(max(-89.5 * Constant::DegToRad, min(89.5 * Constant::DegToRad, OHM_H_RAD)));
    // limit profile angles -89.5 - +89.5
    // by symmetry, properties same for +/- profile angle

    // incidence angles on pleat front/back (_PARL) and sides (_PERP)
    Real64 const tan_OMEGA_V(std::tan(OMEGA_V));
    Real64 const cos_OMEGA_H(std::cos(OMEGA_H));
    Real64 const sin_OMEGA_H(std::sin(OMEGA_H));
    THETA_PARL = std::acos(std::abs(std::cos(std::atan(tan_OMEGA_V * cos_OMEGA_H)) * cos_OMEGA_H));
    THETA_PERP = std::acos(std::abs(std::cos(std::atan(tan_OMEGA_V * sin_OMEGA_H)) * sin_OMEGA_H));

    // off-normal fabric properties, front surface
    TAUFF_BT0 = TAUFF_BB0 + TAUFF_BD0;
    FM_BEAM(state, THETA_PARL, RHOFF_BT0, TAUFF_BT0, TAUFF_BB0, RHOFF_BT_PARL, TAUFF_BB_PARL, TAUFF_BD_PARL);
    if (W / S < state.dataWindowEquivalentLayer->SMALL_ERROR) {
        // flat drape (no pleats) -- return fabric properties
        RHO_BD = RHOFF_BT_PARL;
        TAU_BD = TAUFF_BD_PARL;
        TAU_BB = TAUFF_BB_PARL;
        return;
    }

    FM_BEAM(state, THETA_PERP, RHOFF_BT0, TAUFF_BT0, TAUFF_BB0, RHOFF_BT_PERP, TAUFF_BB_PERP, TAUFF_BD_PERP);

    // Off-normal fabric properties, back surface
    TAUBF_BT0 = TAUBF_BB0 + TAUBF_BD0;
    FM_BEAM(state, THETA_PARL, RHOBF_BT0, TAUBF_BT0, TAUBF_BB0, RHOBF_BT_PARL, TAUBF_BB_PARL, TAUBF_BD_PARL);
    FM_BEAM(state, THETA_PERP, RHOBF_BT0, TAUBF_BT0, TAUBF_BB0, RHOBF_BT_PERP, TAUBF_BB_PERP, TAUBF_BD_PERP);

    DE = S * std::abs(cos_OMEGA_H / max(0.000001, sin_OMEGA_H));
    EF = W - DE;

    // select geometric case
    if (DE < W - state.dataWindowEquivalentLayer->SMALL_ERROR) {
        // illuminated length less than pleat depth
        if (DE < EF - state.dataWindowEquivalentLayer->SMALL_ERROR) {
            // illum < shade
            PD_BEAM_CASE_I(S,
                           W,
                           OMEGA_H,
                           DE,
                           RHOFF_BT_PARL,
                           TAUFF_BB_PARL,
                           TAUFF_BD_PARL,
                           RHOBF_BT_PARL,
                           TAUBF_BB_PARL,
                           TAUBF_BD_PARL,
                           RHOFF_BT_PERP,
                           TAUFF_BB_PERP,
                           TAUFF_BD_PERP,
                           RHOBF_BT_PERP,
                           TAUBF_BB_PERP,
                           TAUBF_BD_PERP,
                           RHOBF_DD,
                           RHOFF_DD,
                           TAUFF_DD,
                           TAUBF_DD,
                           RHO_BD,
                           TAU_BD,
                           TAU_BB);
        } else if (DE <= EF + state.dataWindowEquivalentLayer->SMALL_ERROR) {
            // illum and shade equal
            PD_BEAM_CASE_II(S,
                            W,
                            OMEGA_H,
                            DE,
                            RHOFF_BT_PARL,
                            TAUFF_BB_PARL,
                            TAUFF_BD_PARL,
                            RHOBF_BT_PARL,
                            TAUBF_BB_PARL,
                            TAUBF_BD_PARL,
                            RHOFF_BT_PERP,
                            TAUFF_BB_PERP,
                            TAUFF_BD_PERP,
                            RHOBF_BT_PERP,
                            TAUBF_BB_PERP,
                            TAUBF_BD_PERP,
                            RHOBF_DD,
                            RHOFF_DD,
                            TAUFF_DD,
                            TAUBF_DD,
                            RHO_BD,
                            TAU_BD,
                            TAU_BB);
        } else {
            // illum > shade
            PD_BEAM_CASE_III(S,
                             W,
                             OMEGA_H,
                             DE,
                             RHOFF_BT_PARL,
                             TAUFF_BB_PARL,
                             TAUFF_BD_PARL,
                             RHOBF_BT_PARL,
                             TAUBF_BB_PARL,
                             TAUBF_BD_PARL,
                             RHOFF_BT_PERP,
                             TAUFF_BB_PERP,
                             TAUFF_BD_PERP,
                             RHOBF_BT_PERP,
                             TAUBF_BB_PERP,
                             TAUBF_BD_PERP,
                             RHOBF_DD,
                             RHOFF_DD,
                             TAUFF_DD,
                             TAUBF_DD,
                             RHO_BD,
                             TAU_BD,
                             TAU_BB);
        }
    } else if (DE <= W + state.dataWindowEquivalentLayer->SMALL_ERROR) {
        // illum length same as pleat depth
        PD_BEAM_CASE_IV(S,
                        W,
                        OMEGA_H,
                        DE,
                        RHOFF_BT_PARL,
                        TAUFF_BB_PARL,
                        TAUFF_BD_PARL,
                        RHOBF_BT_PARL,
                        TAUBF_BB_PARL,
                        TAUBF_BD_PARL,
                        RHOFF_BT_PERP,
                        TAUFF_BB_PERP,
                        TAUFF_BD_PERP,
                        RHOBF_BT_PERP,
                        TAUBF_BB_PERP,
                        TAUBF_BD_PERP,
                        RHOBF_DD,
                        RHOFF_DD,
                        TAUFF_DD,
                        TAUBF_DD,
                        RHO_BD,
                        TAU_BD,
                        TAU_BB);
    } else if (DE < 9000.0 * S) {
        // some direct illum on pleat back
        PD_BEAM_CASE_V(S,
                       W,
                       OMEGA_H,
                       DE,
                       RHOFF_BT_PARL,
                       TAUFF_BB_PARL,
                       TAUFF_BD_PARL,
                       RHOBF_BT_PARL,
                       TAUBF_BB_PARL,
                       TAUBF_BD_PARL,
                       RHOFF_BT_PERP,
                       TAUFF_BB_PERP,
                       TAUFF_BD_PERP,
                       RHOBF_BT_PERP,
                       TAUBF_BB_PERP,
                       TAUBF_BD_PERP,
                       RHOBF_DD,
                       RHOFF_DD,
                       TAUFF_DD,
                       TAUBF_DD,
                       RHO_BD,
                       TAU_BD,
                       TAU_BB);
    } else {
        // beam parallel to pleat sides (no direct illum on pleat back)
        PD_BEAM_CASE_VI(S,
                        W,
                        OMEGA_H,
                        DE,
                        RHOFF_BT_PARL,
                        TAUFF_BB_PARL,
                        TAUFF_BD_PARL,
                        RHOBF_BT_PARL,
                        TAUBF_BB_PARL,
                        TAUBF_BD_PARL,
                        RHOFF_BT_PERP,
                        TAUFF_BB_PERP,
                        TAUFF_BD_PERP,
                        RHOBF_BT_PERP,
                        TAUBF_BB_PERP,
                        TAUBF_BD_PERP,
                        RHOBF_DD,
                        RHOFF_DD,
                        TAUFF_DD,
                        TAUBF_DD,
                        RHO_BD,
                        TAU_BD,
                        TAU_BB);
    }
}

void PD_BEAM_CASE_I(Real64 const S,                        // pleat spacing (> 0)
                    Real64 const W,                        // pleat depth (>=0, same units as S)
                    [[maybe_unused]] Real64 const OMEGA_H, // horizontal profile angle, radians
                    Real64 const DE,                       // width of illumination on pleat bottom (same units as S)
                    Real64 const RHOFF_BT_PARL,
                    Real64 const TAUFF_BB_PARL,
                    Real64 const TAUFF_BD_PARL,
                    [[maybe_unused]] Real64 const RHOBF_BT_PARL,
                    [[maybe_unused]] Real64 const TAUBF_BB_PARL,
                    [[maybe_unused]] Real64 const TAUBF_BD_PARL,
                    Real64 const RHOFF_BT_PERP,
                    Real64 const TAUFF_BB_PERP,
                    Real64 const TAUFF_BD_PERP,
                    Real64 const RHOBF_BT_PERP,
                    Real64 const TAUBF_BB_PERP,
                    Real64 const TAUBF_BD_PERP,
                    Real64 const RHOBF_DD, // fabric back diffuse-diffuse reflectance
                    Real64 const RHOFF_DD, // fabric front diffuse-diffuse reflectance
                    Real64 const TAUFF_DD, // fabric front diffuse-diffuse transmittance
                    Real64 const TAUBF_DD, // fabric back diffuse-diffuse transmittance
                    Real64 &RHO_BD,        // returned: drape front beam-diffuse reflectance
                    Real64 &TAU_BD,        // returned: drape front beam-diffuse transmittance
                    Real64 &TAU_BB         // returned: drape front beam-beam transmittance
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  calculates the effective front-side solar optical properties of a drapery layer.
    // METHODOLOGY EMPLOYED:
    // FOURTEEN SURFACE FLAT-FABRIC MODEL WITH RECTANGULAR ENCLOSURE

    // SUBROUTINE ARGUMENT DEFINITIONS:
    // fabric properties at current (off-normal) incidence
    //   _PARL = surface parallel to window (pleat top/bot)
    //   _PERP = surface perpendicular to window (pleat side)

    int constexpr N(12);

    Real64 TAUBF_BT_PERP;
    Real64 AB; // lengths of surfaces and diagonal strings
    Real64 GN;
    Real64 NP;
    Real64 GP;
    Real64 NK;
    Real64 PK;
    Real64 BC;
    Real64 AN;
    Real64 AP;
    Real64 AK;
    Real64 BG;
    Real64 BP;
    Real64 CG;
    Real64 BK;
    Real64 CP;
    Real64 CN;
    Real64 Z1_BB; // beam source terms
    Real64 Z7_BB;
    Real64 Z1_BD; // diffuse source terms due to incident beam radiation
    Real64 Z2_BD;
    Real64 Z7_BD;
    Real64 Z3_BD;
    Real64 Z9_BD;
    Real64 Z13_BD;
    Real64 Z14_BD;
    // shape factors
    Real64 F12;
    Real64 F13;
    Real64 F14;
    Real64 F16;
    Real64 F17;
    Real64 F21;
    Real64 F25;
    Real64 F26;
    Real64 F27;
    Real64 F31;
    Real64 F35;
    Real64 F36;
    Real64 F37;
    Real64 F41;
    Real64 F45;
    Real64 F46;
    Real64 F47;
    Real64 F51;
    Real64 F52;
    Real64 F53;
    Real64 F54;
    Real64 F56;
    Real64 F57;
    Real64 F61;
    Real64 F62;
    Real64 F63;
    Real64 F64;
    Real64 F71;
    Real64 F72;
    Real64 F73;
    Real64 F74;
    Real64 F89;
    Real64 F810;
    Real64 F811;
    Real64 F812;
    Real64 F813;
    Real64 F814;
    Real64 F911;
    Real64 F912;
    Real64 F913;
    Real64 F914;
    Real64 F1011;
    Real64 F1012;
    Real64 F1013;
    Real64 F1014;
    Real64 F119;
    Real64 F1110;
    Real64 F1112;
    Real64 F1113;
    Real64 F1114;
    Real64 F129;
    Real64 F1210;
    Real64 F1211;
    Real64 F139;
    Real64 F1310;
    Real64 F1311;
    Real64 F149;
    Real64 F1410;
    Real64 F1411;
    Real64 J1; // radiosity, surface i
    Real64 J2;
    Real64 J3;
    Real64 J4;
    Real64 J6;
    Real64 J7;
    Real64 J9;
    Real64 J10;
    Real64 J11;
    Real64 J12;
    Real64 J13;
    Real64 J14;
    Real64 G1; // irradiance, surface i
    Real64 G5;
    Real64 G8;
    Real64 G11;
    Array2D<Real64> A(N + 2, N); // coefficients of the radiosity equations matrix
    Array1D<Real64> XSOL(N);     // solution vector (obtained after solving the radiosity equations matrix)

    TAUBF_BT_PERP = TAUBF_BD_PERP + TAUBF_BB_PERP;

    AB = DE;
    GN = DE;
    NP = DE;
    GP = 2.0 * DE;
    NK = W - DE;
    PK = W - 2.0 * DE;
    BC = NK;
    AN = std::sqrt(S * S + DE * DE);
    AP = std::sqrt(S * S + GP * GP);
    AK = std::sqrt(W * W + S * S);
    BG = AN;
    BP = AN;
    CG = AK;
    BK = std::sqrt(S * S + BC * BC);
    CP = std::sqrt(S * S + PK * PK);
    CN = std::sqrt(S * S + NK * NK);

    Z1_BB = TAUFF_BB_PARL;
    Z1_BD = TAUFF_BD_PARL;
    Z2_BD = Z1_BB * RHOBF_BT_PERP * S / GN;
    Z7_BB = TAUFF_BB_PERP * S / DE;
    Z7_BD = TAUFF_BD_PERP * S / DE;
    Z3_BD = Z7_BB * RHOBF_BT_PERP;
    Z9_BD = RHOFF_BT_PERP * S / DE;
    Z13_BD = Z7_BB * TAUBF_BT_PERP;
    Z14_BD = Z1_BB * TAUBF_BT_PERP * S / GN;

    F12 = (S + GN - AN) / (2.0 * S);
    F13 = (AN + GP - (GN + AP)) / (2.0 * S);
    F14 = (AP + W - (GP + AK)) / (2.0 * S);
    F16 = (W + BG - (AB + CG)) / (2.0 * S);
    F17 = (S + AB - BG) / (2.0 * S);
    F21 = (S + GN - AN) / (2.0 * GN);
    F25 = (W + CN - (CG + NK)) / (2.0 * GN);
    F26 = (CG + S - (BG + CN)) / (2.0 * GN);
    F27 = (AN + BG - 2.0 * S) / (2.0 * GN);
    F31 = (AN + GP - (GN + AP)) / (2.0 * NP);
    F35 = (NK + CP - (CN + PK)) / (2.0 * NP);
    F36 = (CN + BP - (S + CP)) / (2.0 * NP);
    F37 = (S + AP - (AN + BP)) / (2.0 * NP);
    F41 = (W + AP - (GP + AK)) / (2.0 * PK);
    F45 = (S + PK - CP) / (2.0 * PK);
    F46 = (CP + BK - (S + BP)) / (2.0 * PK);
    F47 = (BP + AK - (AP + BK)) / (2.0 * PK);
    F51 = (AK + CG - 2.0 * W) / (2.0 * S);
    F52 = (W + CN - (CG + NK)) / (2.0 * S);
    F53 = (NK + CP - (CN + PK)) / (2.0 * S);
    F54 = (S + PK - CP) / (2.0 * S);
    F56 = (S + BC - BK) / (2.0 * S);
    F57 = (W + BK - (BC + AK)) / (2.0 * S);
    F61 = (W + BG - (AB + CG)) / (2.0 * BC);
    F62 = (S + CG - (BG + CN)) / (2.0 * BC);
    F63 = (CN + BP - (S + CP)) / (2.0 * BC);
    F64 = (BK + CP - (S + BP)) / (2.0 * BC);
    F71 = F21;
    F72 = F27;
    F73 = F37;
    F74 = (BP + AK - (BK + AP)) / (2.0 * AB);
    F89 = F12;
    F810 = F16;
    F811 = F51;
    F812 = F14;
    F813 = F13;
    F814 = F12;
    F911 = F25;
    F912 = F74;
    F913 = F73;
    F914 = F27;
    F1011 = (BC + S - BK) / (2.0 * BC);
    F1012 = F64;
    F1013 = F63;
    F1014 = F62;
    F119 = F57;
    F1110 = F56;
    F1112 = F54;
    F1113 = F53;
    F1114 = F52;
    F129 = F47;
    F1210 = F46;
    F1211 = F45;
    F139 = F37;
    F1310 = F36;
    F1311 = F35;
    F149 = F27;
    F1410 = F26;
    F1411 = F25;

    A = 0.0;    // INITIALIZE RADIOSITY MATRIX COEFFICIENTS
    XSOL = 0.0; // INITIALIZE SOLUTION VECTOR COEFFICIENTS

    // POPULATE THE COEFFICIENTS OF THE RADIOSITY MATRIX

    A(1, 1) = 1.0;
    A(2, 1) = -RHOBF_DD * F12;
    A(3, 1) = -RHOBF_DD * F13;
    A(4, 1) = -RHOBF_DD * F14;
    A(5, 1) = -RHOBF_DD * F16;
    A(6, 1) = -RHOBF_DD * F17;
    A(7, 1) = 0.0;
    A(8, 1) = 0.0;
    A(9, 1) = 0.0;
    A(10, 1) = 0.0;
    A(11, 1) = 0.0;
    A(12, 1) = 0.0;
    A(13, 1) = Z1_BD;
    A(1, 2) = -RHOBF_DD * F21;
    A(2, 2) = 1.0;
    A(3, 2) = 0.0;
    A(4, 2) = 0.0;
    A(5, 2) = -RHOBF_DD * F26;
    A(6, 2) = -RHOBF_DD * F27;
    A(7, 2) = -TAUFF_DD * F149;
    A(8, 2) = -TAUFF_DD * F1410;
    A(9, 2) = -TAUFF_DD * F1411;
    A(10, 2) = 0.0;
    A(11, 2) = 0.0;
    A(12, 2) = 0.0;
    A(13, 2) = Z2_BD;
    A(1, 3) = -RHOBF_DD * F31;
    A(2, 3) = 0.0;
    A(3, 3) = 1.0;
    A(4, 3) = 0.0;
    A(5, 3) = -RHOBF_DD * F36;
    A(6, 3) = -RHOBF_DD * F37;
    A(7, 3) = -TAUFF_DD * F139;
    A(8, 3) = -TAUFF_DD * F1310;
    A(9, 3) = -TAUFF_DD * F1311;
    A(10, 3) = 0.0;
    A(11, 3) = 0.0;
    A(12, 3) = 0.0;
    A(13, 3) = Z3_BD;
    A(1, 4) = -RHOBF_DD * F41;
    A(2, 4) = 0.0;
    A(3, 4) = 0.0;
    A(4, 4) = 1.0;
    A(5, 4) = -RHOBF_DD * F46;
    A(6, 4) = -RHOBF_DD * F47;
    A(7, 4) = -TAUFF_DD * F129;
    A(8, 4) = -TAUFF_DD * F1210;
    A(9, 4) = -TAUFF_DD * F1211;
    A(10, 4) = 0.0;
    A(11, 4) = 0.0;
    A(12, 4) = 0.0;
    A(13, 4) = 0.0;
    A(1, 5) = -RHOBF_DD * F61;
    A(2, 5) = -RHOBF_DD * F62;
    A(3, 5) = -RHOBF_DD * F63;
    A(4, 5) = -RHOBF_DD * F64;
    A(5, 5) = 1.0;
    A(6, 5) = 0.0;
    A(7, 5) = 0.0;
    A(8, 5) = 0.0;
    A(9, 5) = -TAUFF_DD * F1011;
    A(10, 5) = -TAUFF_DD * F1012;
    A(11, 5) = -TAUFF_DD * F1013;
    A(12, 5) = -TAUFF_DD * F1014;
    A(13, 5) = 0.0;
    A(1, 6) = -RHOBF_DD * F71;
    A(2, 6) = -RHOBF_DD * F72;
    A(3, 6) = -RHOBF_DD * F73;
    A(4, 6) = -RHOBF_DD * F74;
    A(5, 6) = 0.0;
    A(6, 6) = 1.0;
    A(7, 6) = 0.0;
    A(8, 6) = 0.0;
    A(9, 6) = -TAUFF_DD * F911;
    A(10, 6) = -TAUFF_DD * F912;
    A(11, 6) = -TAUFF_DD * F913;
    A(12, 6) = -TAUFF_DD * F914;
    A(13, 6) = Z7_BD;
    A(1, 7) = -TAUBF_DD * F71;
    A(2, 7) = -TAUBF_DD * F72;
    A(3, 7) = -TAUBF_DD * F73;
    A(4, 7) = -TAUBF_DD * F74;
    A(5, 7) = 0.0;
    A(6, 7) = 0.0;
    A(7, 7) = 1.0;
    A(8, 7) = 0.0;
    A(9, 7) = -RHOFF_DD * F911;
    A(10, 7) = -RHOFF_DD * F912;
    A(11, 7) = -RHOFF_DD * F913;
    A(12, 7) = -RHOFF_DD * F914;
    A(13, 7) = Z9_BD;
    A(1, 8) = -TAUBF_DD * F61;
    A(2, 8) = -TAUBF_DD * F62;
    A(3, 8) = -TAUBF_DD * F63;
    A(4, 8) = -TAUBF_DD * F64;
    A(5, 8) = 0.0;
    A(6, 8) = 0.0;
    A(7, 8) = 0.0;
    A(8, 8) = 1.0;
    A(9, 8) = -RHOFF_DD * F1011;
    A(10, 8) = -RHOFF_DD * F1012;
    A(11, 8) = -RHOFF_DD * F1013;
    A(12, 8) = -RHOFF_DD * F1014;
    A(13, 8) = 0.0;
    A(1, 9) = 0.0;
    A(2, 9) = 0.0;
    A(3, 9) = 0.0;
    A(4, 9) = 0.0;
    A(5, 9) = 0.0;
    A(6, 9) = 0.0;
    A(7, 9) = -RHOFF_DD * F119;
    A(8, 9) = -RHOFF_DD * F1110;
    A(9, 9) = 1.0;
    A(10, 9) = -RHOFF_DD * F1112;
    A(11, 9) = -RHOFF_DD * F1113;
    A(12, 9) = -RHOFF_DD * F1114;
    A(13, 9) = 0.0;
    A(1, 10) = -TAUBF_DD * F41;
    A(2, 10) = 0.0;
    A(3, 10) = 0.0;
    A(4, 10) = 0.0;
    A(5, 10) = -TAUBF_DD * F46;
    A(6, 10) = -TAUBF_DD * F47;
    A(7, 10) = -RHOFF_DD * F129;
    A(8, 10) = -RHOFF_DD * F1210;
    A(9, 10) = -RHOFF_DD * F1211;
    A(10, 10) = 1.0;
    A(11, 10) = 0.0;
    A(12, 10) = 0.0;
    A(13, 10) = 0.0;
    A(1, 11) = -TAUBF_DD * F31;
    A(2, 11) = 0.0;
    A(3, 11) = 0.0;
    A(4, 11) = 0.0;
    A(5, 11) = -TAUBF_DD * F36;
    A(6, 11) = -TAUBF_DD * F37;
    A(7, 11) = -RHOFF_DD * F139;
    A(8, 11) = -RHOFF_DD * F1310;
    A(9, 11) = -RHOFF_DD * F1311;
    A(10, 11) = 0.0;
    A(11, 11) = 1.0;
    A(12, 11) = 0.0;
    A(13, 11) = Z13_BD;
    A(1, 12) = -TAUBF_DD * F21;
    A(2, 12) = 0.0;
    A(3, 12) = 0.0;
    A(4, 12) = 0.0;
    A(5, 12) = -TAUBF_DD * F26;
    A(6, 12) = -TAUBF_DD * F27;
    A(7, 12) = -RHOFF_DD * F149;
    A(8, 12) = -RHOFF_DD * F1410;
    A(9, 12) = -RHOFF_DD * F1411;
    A(10, 12) = 0.0;
    A(11, 12) = 0.0;
    A(12, 12) = 1.0;
    A(13, 12) = Z14_BD;

    SOLMATS(N, A, XSOL);

    J1 = XSOL(1);
    J2 = XSOL(2);
    J3 = XSOL(3);
    J4 = XSOL(4);
    J6 = XSOL(5);
    J7 = XSOL(6);
    J9 = XSOL(7);
    J10 = XSOL(8);
    J11 = XSOL(9);
    J12 = XSOL(10);
    J13 = XSOL(11);
    J14 = XSOL(12);

    G1 = F12 * J2 + F13 * J3 + F14 * J4 + F16 * J6 + F17 * J7;
    G5 = F56 * J6 + F57 * J7 + F51 * J1 + F52 * J2 + F53 * J3 + F54 * J4;
    G8 = F89 * J9 + F810 * J10 + F811 * J11 + F812 * J12 + F813 * J13 + F814 * J14;
    G11 = F1112 * J12 + F1113 * J13 + F1114 * J14 + F119 * J9 + F1110 * J10;

    TAU_BB = 0.0;
    TAU_BD = (G5 + TAUFF_DD * G11) / 2.0;
    RHO_BD = (RHOFF_BT_PARL + TAUBF_DD * G1 + G8) / 2.0;
}

void PD_BEAM_CASE_II(Real64 const S,                        // pleat spacing (> 0)
                     Real64 const W,                        // pleat depth (>=0, same units as S)
                     [[maybe_unused]] Real64 const OMEGA_H, // horizontal profile angle, radians
                     Real64 const DE,                       // width of illumination on pleat bottom (same units as S)
                     Real64 const RHOFF_BT_PARL,
                     Real64 const TAUFF_BB_PARL,
                     Real64 const TAUFF_BD_PARL,
                     [[maybe_unused]] Real64 const RHOBF_BT_PARL,
                     [[maybe_unused]] Real64 const TAUBF_BB_PARL,
                     [[maybe_unused]] Real64 const TAUBF_BD_PARL,
                     Real64 const RHOFF_BT_PERP,
                     Real64 const TAUFF_BB_PERP,
                     Real64 const TAUFF_BD_PERP,
                     Real64 const RHOBF_BT_PERP,
                     Real64 const TAUBF_BB_PERP,
                     Real64 const TAUBF_BD_PERP,
                     Real64 const RHOBF_DD, // fabric back diffuse-diffuse reflectance
                     Real64 const RHOFF_DD, // fabric front diffuse-diffuse reflectance
                     Real64 const TAUFF_DD, // fabric front diffuse-diffuse transmittance
                     Real64 const TAUBF_DD, // fabric back diffuse-diffuse transmittance
                     Real64 &RHO_BD,        // returned: drape front beam-diffuse reflectance
                     Real64 &TAU_BD,        // returned: drape front beam-diffuse transmittance
                     Real64 &TAU_BB         // returned: drape front beam-beam transmittance
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  calculates the effective front-side solar optical properties of a drapery layer.
    // METHODOLOGY EMPLOYED:
    // TWELVE SURFACE FLAT-FABRIC MODEL WITH RECTANGULAR ENCLOSURE

    // fabric properties at current (off-normal) incidence
    //   _PARL = surface parallel to window (pleat top/bot)
    //   _PERP = surface perpendicular to window (pleat side)

    int constexpr N(10);

    Real64 TAUBF_BT_PERP;
    Real64 AB; // lengths of surfaces and diagonal strings
    Real64 GN;
    Real64 NK;
    Real64 BC;
    Real64 AN;
    Real64 AK;
    Real64 BG;
    Real64 CG;
    Real64 BK;
    Real64 CN;
    Real64 Z1_BD; // diffuse source terms due to incident beam radiation
    Real64 Z2_BD;
    Real64 Z3_BD;
    Real64 Z6_BD;
    Real64 Z8_BD;
    Real64 Z11_BD;
    Real64 Z12_BD;
    Real64 Z1_BB; // beam source terms due to incident beam radiation
    Real64 Z6_BB;
    // shape factors
    Real64 F12;
    Real64 F13;
    Real64 F15;
    Real64 F16;
    Real64 F21;
    Real64 F25;
    Real64 F26;
    Real64 F31;
    Real64 F35;
    Real64 F36;
    Real64 F41;
    Real64 F42;
    Real64 F43;
    Real64 F45;
    Real64 F46;
    Real64 F51;
    Real64 F52;
    Real64 F53;
    Real64 F54;
    Real64 F61;
    Real64 F62;
    Real64 F63;
    Real64 F78;
    Real64 F79;
    Real64 F710;
    Real64 F711;
    Real64 F712;
    Real64 F810;
    Real64 F811;
    Real64 F812;
    Real64 F910;
    Real64 F911;
    Real64 F912;
    Real64 F108;
    Real64 F109;
    Real64 F1011;
    Real64 F1012;
    Real64 F118;
    Real64 F119;
    Real64 F1110;
    Real64 F128;
    Real64 F129;
    Real64 F1210;

    Real64 J1; // radiosity, surface i
    Real64 J2;
    Real64 J3;
    Real64 J5;
    Real64 J6;
    Real64 J8;
    Real64 J9;
    Real64 J10;
    Real64 J11;
    Real64 J12;
    Real64 G1; // irradiance, surface i
    Real64 G4;
    Real64 G7;
    Real64 G10;
    Array2D<Real64> A(N + 2, N); // coefficients of the radiosity equations matrix
    Array1D<Real64> XSOL(N);     // solution vector (obtained after solving the radiosity equations matrix)

    TAUBF_BT_PERP = TAUBF_BD_PERP + TAUBF_BB_PERP;

    AB = DE;
    GN = DE;
    NK = W - DE;
    BC = NK;
    AN = std::sqrt(S * S + DE * DE);
    AK = std::sqrt(W * W + S * S);
    BG = AN;
    CG = AK;
    BK = std::sqrt(S * S + BC * BC);
    CN = std::sqrt(S * S + NK * NK);

    Z1_BB = TAUFF_BB_PARL;
    Z1_BD = TAUFF_BD_PARL;
    Z2_BD = Z1_BB * RHOBF_BT_PERP * S / GN;
    Z6_BB = TAUFF_BB_PERP * S / DE;
    Z6_BD = TAUFF_BD_PERP * S / DE;
    Z3_BD = Z6_BB * RHOBF_BT_PERP;
    Z8_BD = RHOFF_BT_PERP * S / DE;
    Z11_BD = Z6_BB * TAUBF_BT_PERP;
    Z12_BD = Z1_BB * TAUBF_BT_PERP * S / GN;

    F12 = (S + GN - AN) / (2.0 * S);
    F13 = (W + AN - (GN + AK)) / (2.0 * S);
    F15 = (W + BG - (AB + CG)) / (2.0 * S);
    F16 = (S + AB - BG) / (2.0 * S);
    F21 = (S + GN - AN) / (2.0 * GN);
    F25 = (S + CG - (BG + CN)) / (2.0 * GN);
    F26 = (AN + BG - 2.0 * S) / (2.0 * GN);
    F31 = (W + AN - (GN + AK)) / (2.0 * NK);
    F35 = (BK + CN - 2.0 * S) / (2.0 * NK);
    F36 = (S + AK - (AN + BK)) / (2.0 * NK);
    F41 = (AK + CG - 2.0 * W) / (2.0 * S);
    F42 = (W + CN - (CG + NK)) / (2.0 * S);
    F43 = (S + NK - CN) / (2.0 * S);
    F45 = (S + BC - BK) / (2.0 * S);
    F46 = (W + BK - (AK + BC)) / (2.0 * S);
    F51 = (W + BG - (AB + CG)) / (2.0 * BC);
    F52 = (S + CG - (BG + CN)) / (2.0 * BC);
    F53 = (BK + CN - 2.0 * S) / (2.0 * BC);
    F54 = (S + BC - BK) / (2.0 * BC);
    F61 = (S + AB - BG) / (2.0 * AB);
    F62 = (AN + BG - 2.0 * S) / (2.0 * AB);
    F63 = (S + AK - (AN + BK)) / (2.0 * AB);
    F78 = F12;
    F79 = F13;
    F710 = (AK + CG - 2.0 * W) / (2.0 * S);
    F711 = F15;
    F712 = F16;
    F810 = (W + CN - (CG + NK)) / (2.0 * S);
    F811 = F25;
    F812 = F26;
    F910 = (S + NK - CN) / (2.0 * NK);
    F911 = F35;
    F912 = F36;
    F108 = F42;
    F109 = F43;
    F1011 = F45;
    F1012 = F46;
    F118 = F52;
    F119 = F53;
    F1110 = (S + BC - BK) / (2.0 * NK);
    F128 = F62;
    F129 = F63;
    F1210 = (W + BK - (AK + BC)) / (2.0 * GN);

    A = 0.0;    // INITIALIZE RADIOSITY MATRIX COEFFICIENTS
    XSOL = 0.0; // INITIALIZE SOLUTION VECTOR COEFFICIENTS

    // POPULATE THE COEFFICIENTS OF THE RADIOSITY MATRIX

    A(1, 1) = 1.0;
    A(2, 1) = -RHOBF_DD * F12;
    A(3, 1) = -RHOBF_DD * F13;
    A(4, 1) = -RHOBF_DD * F15;
    A(5, 1) = -RHOBF_DD * F16;
    A(6, 1) = 0.0;
    A(7, 1) = 0.0;
    A(8, 1) = 0.0;
    A(9, 1) = 0.0;
    A(10, 1) = 0.0;
    A(11, 1) = Z1_BD;
    A(1, 2) = -RHOBF_DD * F21;
    A(2, 2) = 1.0;
    A(3, 2) = 0.0;
    A(4, 2) = -RHOBF_DD * F25;
    A(5, 2) = -RHOBF_DD * F26;
    A(6, 2) = -TAUFF_DD * F128;
    A(7, 2) = -TAUFF_DD * F129;
    A(8, 2) = -TAUFF_DD * F1210;
    A(9, 2) = 0.0;
    A(10, 2) = 0.0;
    A(11, 2) = Z2_BD;
    A(1, 3) = -RHOBF_DD * F31;
    A(2, 3) = 0.0;
    A(3, 3) = 1.0;
    A(4, 3) = -RHOBF_DD * F35;
    A(5, 3) = -RHOBF_DD * F36;
    A(6, 3) = -TAUFF_DD * F118;
    A(7, 3) = -TAUFF_DD * F119;
    A(8, 3) = -TAUFF_DD * F1110;
    A(9, 3) = 0.0;
    A(10, 3) = 0.0;
    A(11, 3) = Z3_BD;
    A(1, 4) = -RHOBF_DD * F51;
    A(2, 4) = -RHOBF_DD * F52;
    A(3, 4) = -RHOBF_DD * F53;
    A(4, 4) = 1.0;
    A(5, 4) = 0.0;
    A(6, 4) = 0.0;
    A(7, 4) = 0.0;
    A(8, 4) = -TAUFF_DD * F910;
    A(9, 4) = -TAUFF_DD * F911;
    A(10, 4) = -TAUFF_DD * F912;
    A(11, 4) = 0.0;
    A(1, 5) = -RHOBF_DD * F61;
    A(2, 5) = -RHOBF_DD * F62;
    A(3, 5) = -RHOBF_DD * F63;
    A(4, 5) = 0.0;
    A(5, 5) = 1.0;
    A(6, 5) = 0.0;
    A(7, 5) = 0.0;
    A(8, 5) = -TAUFF_DD * F810;
    A(9, 5) = -TAUFF_DD * F811;
    A(10, 5) = -TAUFF_DD * F812;
    A(11, 5) = Z6_BD;
    A(1, 6) = -TAUBF_DD * F61;
    A(2, 6) = -TAUBF_DD * F62;
    A(3, 6) = -TAUBF_DD * F63;
    A(4, 6) = 0.0;
    A(5, 6) = 0.0;
    A(6, 6) = 1.0;
    A(7, 6) = 0.0;
    A(8, 6) = -RHOFF_DD * F810;
    A(9, 6) = -RHOFF_DD * F811;
    A(10, 6) = -RHOFF_DD * F812;
    A(11, 6) = Z8_BD;
    A(1, 7) = -TAUBF_DD * F51;
    A(2, 7) = -TAUBF_DD * F52;
    A(3, 7) = -TAUBF_DD * F53;
    A(4, 7) = 0.0;
    A(5, 7) = 0.0;
    A(6, 7) = 0.0;
    A(7, 7) = 1.0;
    A(8, 7) = -RHOFF_DD * F910;
    A(9, 7) = -RHOFF_DD * F911;
    A(10, 7) = -RHOFF_DD * F912;
    A(11, 7) = 0.0;
    A(1, 8) = 0.0;
    A(2, 8) = 0.0;
    A(3, 8) = 0.0;
    A(4, 8) = 0.0;
    A(5, 8) = 0.0;
    A(6, 8) = -RHOFF_DD * F108;
    A(7, 8) = -RHOFF_DD * F109;
    A(8, 8) = 1.0;
    A(9, 8) = -RHOFF_DD * F1011;
    A(10, 8) = -RHOFF_DD * F1012;
    A(11, 8) = 0.0;
    A(1, 9) = -TAUBF_DD * F31;
    A(2, 9) = 0.0;
    A(3, 9) = 0.0;
    A(4, 9) = -TAUBF_DD * F35;
    A(5, 9) = -TAUBF_DD * F36;
    A(6, 9) = -RHOFF_DD * F118;
    A(7, 9) = -RHOFF_DD * F119;
    A(8, 9) = -RHOFF_DD * F1110;
    A(9, 9) = 1.0;
    A(10, 9) = 0.0;
    A(11, 9) = Z11_BD;
    A(1, 10) = -TAUBF_DD * F21;
    A(2, 10) = 0.0;
    A(3, 10) = 0.0;
    A(4, 10) = -TAUBF_DD * F25;
    A(5, 10) = -TAUBF_DD * F26;
    A(6, 10) = -RHOFF_DD * F128;
    A(7, 10) = -RHOFF_DD * F129;
    A(8, 10) = -RHOFF_DD * F1210;
    A(9, 10) = 0.0;
    A(10, 10) = 1.0;
    A(11, 10) = Z12_BD;

    SOLMATS(N, A, XSOL);

    J1 = XSOL(1);
    J2 = XSOL(2);
    J3 = XSOL(3);
    J5 = XSOL(4);
    J6 = XSOL(5);
    J8 = XSOL(6);
    J9 = XSOL(7);
    J10 = XSOL(8);
    J11 = XSOL(9);
    J12 = XSOL(10);

    G1 = F12 * J2 + F13 * J3 + F15 * J5 + F16 * J6;
    G4 = F41 * J1 + F42 * J2 + F43 * J3 + F45 * J5 + F46 * J6;
    G7 = F78 * J8 + F79 * J9 + F710 * J10 + F711 * J11 + F712 * J12;
    G10 = F108 * J8 + F109 * J9 + F1011 * J11 + F1012 * J12;

    TAU_BB = 0.0;
    TAU_BD = (G4 + TAUFF_DD * G10) / 2.0;
    RHO_BD = (RHOFF_BT_PARL + TAUBF_DD * G1 + G7) / 2.0;
}

void PD_BEAM_CASE_III(Real64 const S,       // pleat spacing (> 0)
                      Real64 const W,       // pleat depth (>=0, same units as S)
                      Real64 const OMEGA_H, // horizontal profile angle, radians
                      Real64 const DE,      // width of illumination on pleat bottom (same units as S)
                      Real64 const RHOFF_BT_PARL,
                      Real64 const TAUFF_BB_PARL,
                      Real64 const TAUFF_BD_PARL,
                      [[maybe_unused]] Real64 const RHOBF_BT_PARL,
                      [[maybe_unused]] Real64 const TAUBF_BB_PARL,
                      [[maybe_unused]] Real64 const TAUBF_BD_PARL,
                      Real64 const RHOFF_BT_PERP,
                      Real64 const TAUFF_BB_PERP,
                      Real64 const TAUFF_BD_PERP,
                      Real64 const RHOBF_BT_PERP,
                      Real64 const TAUBF_BB_PERP,
                      Real64 const TAUBF_BD_PERP,
                      Real64 const RHOBF_DD, // fabric back diffuse-diffuse reflectance
                      Real64 const RHOFF_DD, // fabric front diffuse-diffuse reflectance
                      Real64 const TAUFF_DD, // fabric front diffuse-diffuse transmittance
                      Real64 const TAUBF_DD, // fabric back diffuse-diffuse transmittance
                      Real64 &RHO_BD,        // returned: drape front beam-diffuse reflectance
                      Real64 &TAU_BD,        // returned: drape front beam-diffuse transmittance
                      Real64 &TAU_BB         // returned: drape front beam-beam transmittance
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  calculates the effective front-side solar optical properties of a drapery layer.
    // METHODOLOGY EMPLOYED:
    // TWELVE SURFACE FLAT-FABRIC MODEL WITH RECTANGULAR ENCLOSURE

    // fabric properties at current (off-normal) incidence
    //   _PARL = surface parallel to window (pleat top/bot)
    //   _PERP = surface perpendicular to window (pleat side)

    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr N(10);

    Real64 TAUBF_BT_PERP;
    Real64 AB; // lengths for surfaces and diagonal strings
    Real64 GN;
    Real64 NK;
    Real64 BC;
    Real64 AN;
    Real64 AK;
    Real64 BG;
    Real64 CG;
    Real64 BK;
    Real64 CN;
    Real64 Z1_BB; // beam source terms
    Real64 Z6_BB;
    Real64 Z1_BD; // diffuse source terms
    Real64 Z2_BD;
    Real64 Z6_BD;
    Real64 Z3_BD;
    Real64 Z8_BD;
    Real64 Z11_BD;
    Real64 Z12_BD;
    // shape factors
    Real64 F12;
    Real64 F13;
    Real64 F15;
    Real64 F16;
    Real64 F21;
    Real64 F25;
    Real64 F26;
    Real64 F31;
    Real64 F35;
    Real64 F36;
    Real64 F41;
    Real64 F42;
    Real64 F43;
    Real64 F45;
    Real64 F46;
    Real64 F51;
    Real64 F52;
    Real64 F53;
    Real64 F54;
    Real64 F61;
    Real64 F62;
    Real64 F63;
    Real64 F78;
    Real64 F79;
    Real64 F710;
    Real64 F711;
    Real64 F712;
    Real64 F810;
    Real64 F811;
    Real64 F812;
    Real64 F910;
    Real64 F911;
    Real64 F912;
    Real64 F108;
    Real64 F109;
    Real64 F1011;
    Real64 F1012;
    Real64 F118;
    Real64 F119;
    Real64 F1110;
    Real64 F128;
    Real64 F129;
    Real64 F1210;
    Real64 J1; // radiosity, surface i
    Real64 J2;
    Real64 J3;
    Real64 J5;
    Real64 J6;
    Real64 J8;
    Real64 J9;
    Real64 J10;
    Real64 J11;
    Real64 J12;
    Real64 G1; // irradiance, surface i
    Real64 G4;
    Real64 G7;
    Real64 G10;

    Array2D<Real64> A(N + 2, N); // coefficients of the radiosity equations matrix
    Array1D<Real64> XSOL(N);     // solution vector (obtained after solving the radiosity equations matrix)

    TAUBF_BT_PERP = TAUBF_BD_PERP + TAUBF_BB_PERP;

    AB = DE;
    GN = DE;
    NK = W - DE;
    BC = NK;
    AN = std::sqrt(S * S + DE * DE);
    AK = std::sqrt(W * W + S * S);
    BG = AN;
    CG = AK;
    BK = std::sqrt(S * S + BC * BC);
    CN = std::sqrt(S * S + NK * NK);

    Z1_BB = TAUFF_BB_PARL;
    Z1_BD = TAUFF_BD_PARL;
    Z2_BD = Z1_BB * RHOBF_BT_PERP * S / GN;
    Z6_BB = TAUFF_BB_PERP * S / DE;
    Z6_BD = TAUFF_BD_PERP * S / DE;
    Z3_BD = Z6_BB * RHOBF_BT_PERP;
    Z8_BD = RHOFF_BT_PERP * S / DE;
    Z11_BD = Z6_BB * TAUBF_BT_PERP;
    Z12_BD = Z1_BB * TAUBF_BT_PERP * S / GN;

    F12 = (S + GN - AN) / (2.0 * S);
    F13 = (W + AN - (GN + AK)) / (2.0 * S);
    F15 = (W + BG - (AB + CG)) / (2.0 * S);
    F16 = (S + AB - BG) / (2.0 * S);
    F21 = (S + GN - AN) / (2.0 * GN);
    F25 = (S + CG - (BG + CN)) / (2.0 * GN);
    F26 = (AN + BG - 2.0 * S) / (2.0 * GN);
    F31 = (W + AN - (GN + AK)) / (2.0 * NK);
    F35 = (BK + CN - 2.0 * S) / (2.0 * NK);
    F36 = (S + AK - (AN + BK)) / (2.0 * NK);
    F41 = (AK + CG - 2.0 * W) / (2.0 * S);
    F42 = (W + CN - (CG + NK)) / (2.0 * S);
    F43 = (S + NK - CN) / (2.0 * S);
    F45 = (S + BC - BK) / (2.0 * S);
    F46 = (W + BK - (AK + BC)) / (2.0 * S);
    F51 = (W + BG - (AB + CG)) / (2.0 * BC);
    F52 = (S + CG - (BG + CN)) / (2.0 * BC);
    F53 = (BK + CN - 2.0 * S) / (2.0 * BC);
    F54 = (S + BC - BK) / (2.0 * BC);
    F61 = (S + AB - BG) / (2.0 * AB);
    F62 = (AN + BG - 2.0 * S) / (2.0 * AB);
    F63 = (S + AK - (AN + BK)) / (2.0 * AB);
    F78 = F12;
    F79 = F13;
    F710 = (AK + CG - 2.0 * W) / (2.0 * S);
    F711 = F15;
    F712 = F16;
    F810 = (W + CN - (CG + NK)) / (2.0 * S);
    F811 = F25;
    F812 = F26;
    F910 = (S + NK - CN) / (2.0 * NK);
    F911 = F35;
    F912 = F36;
    F108 = F42;
    F109 = F43;
    F1011 = F45;
    F1012 = F46;
    F118 = F52;
    F119 = F53;
    F1110 = (S + BC - BK) / (2.0 * NK);
    F128 = F62;
    F129 = F63;
    F1210 = (W + BK - (AK + BC)) / (2.0 * GN);

    A = 0.0;    // INITIALIZE RADIOSITY MATRIX COEFFICIENTS
    XSOL = 0.0; // INITIALIZE SOLUTION VECTOR COEFFICIENTS

    // POPULATE THE COEFFICIENTS OF THE RADIOSITY MATRIX

    A(1, 1) = 1.0;
    A(2, 1) = -RHOBF_DD * F12;
    A(3, 1) = -RHOBF_DD * F13;
    A(4, 1) = -RHOBF_DD * F15;
    A(5, 1) = -RHOBF_DD * F16;
    A(6, 1) = 0.0;
    A(7, 1) = 0.0;
    A(8, 1) = 0.0;
    A(9, 1) = 0.0;
    A(10, 1) = 0.0;
    A(11, 1) = Z1_BD;
    A(1, 2) = -RHOBF_DD * F21;
    A(2, 2) = 1.0;
    A(3, 2) = 0.0;
    A(4, 2) = -RHOBF_DD * F25;
    A(5, 2) = -RHOBF_DD * F26;
    A(6, 2) = -TAUFF_DD * F128;
    A(7, 2) = -TAUFF_DD * F129;
    A(8, 2) = -TAUFF_DD * F1210;
    A(9, 2) = 0.0;
    A(10, 2) = 0.0;
    A(11, 2) = Z2_BD;
    A(1, 3) = -RHOBF_DD * F31;
    A(2, 3) = 0.0;
    A(3, 3) = 1.0;
    A(4, 3) = -RHOBF_DD * F35;
    A(5, 3) = -RHOBF_DD * F36;
    A(6, 3) = -TAUFF_DD * F118;
    A(7, 3) = -TAUFF_DD * F119;
    A(8, 3) = -TAUFF_DD * F1110;
    A(9, 3) = 0.0;
    A(10, 3) = 0.0;
    A(11, 3) = Z3_BD;
    A(1, 4) = -RHOBF_DD * F51;
    A(2, 4) = -RHOBF_DD * F52;
    A(3, 4) = -RHOBF_DD * F53;
    A(4, 4) = 1.0;
    A(5, 4) = 0.0;
    A(6, 4) = 0.0;
    A(7, 4) = 0.0;
    A(8, 4) = -TAUFF_DD * F910;
    A(9, 4) = -TAUFF_DD * F911;
    A(10, 4) = -TAUFF_DD * F912;
    A(11, 4) = 0.0;
    A(1, 5) = -RHOBF_DD * F61;
    A(2, 5) = -RHOBF_DD * F62;
    A(3, 5) = -RHOBF_DD * F63;
    A(4, 5) = 0.0;
    A(5, 5) = 1.0;
    A(6, 5) = 0.0;
    A(7, 5) = 0.0;
    A(8, 5) = -TAUFF_DD * F810;
    A(9, 5) = -TAUFF_DD * F811;
    A(10, 5) = -TAUFF_DD * F812;
    A(11, 5) = Z6_BD;
    A(1, 6) = -TAUBF_DD * F61;
    A(2, 6) = -TAUBF_DD * F62;
    A(3, 6) = -TAUBF_DD * F63;
    A(4, 6) = 0.0;
    A(5, 6) = 0.0;
    A(6, 6) = 1.0;
    A(7, 6) = 0.0;
    A(8, 6) = -RHOFF_DD * F810;
    A(9, 6) = -RHOFF_DD * F811;
    A(10, 6) = -RHOFF_DD * F812;
    A(11, 6) = Z8_BD;
    A(1, 7) = -TAUBF_DD * F51;
    A(2, 7) = -TAUBF_DD * F52;
    A(3, 7) = -TAUBF_DD * F53;
    A(4, 7) = 0.0;
    A(5, 7) = 0.0;
    A(6, 7) = 0.0;
    A(7, 7) = 1.0;
    A(8, 7) = -RHOFF_DD * F910;
    A(9, 7) = -RHOFF_DD * F911;
    A(10, 7) = -RHOFF_DD * F912;
    A(11, 7) = 0.0;
    A(1, 8) = 0.0;
    A(2, 8) = 0.0;
    A(3, 8) = 0.0;
    A(4, 8) = 0.0;
    A(5, 8) = 0.0;
    A(6, 8) = -RHOFF_DD * F108;
    A(7, 8) = -RHOFF_DD * F109;
    A(8, 8) = 1.0;
    A(9, 8) = -RHOFF_DD * F1011;
    A(10, 8) = -RHOFF_DD * F1012;
    A(11, 8) = 0.0;
    A(1, 9) = -TAUBF_DD * F31;
    A(2, 9) = 0.0;
    A(3, 9) = 0.0;
    A(4, 9) = -TAUBF_DD * F35;
    A(5, 9) = -TAUBF_DD * F36;
    A(6, 9) = -RHOFF_DD * F118;
    A(7, 9) = -RHOFF_DD * F119;
    A(8, 9) = -RHOFF_DD * F1110;
    A(9, 9) = 1.0;
    A(10, 9) = 0.0;
    A(11, 9) = Z11_BD;
    A(1, 10) = -TAUBF_DD * F21;
    A(2, 10) = 0.0;
    A(3, 10) = 0.0;
    A(4, 10) = -TAUBF_DD * F25;
    A(5, 10) = -TAUBF_DD * F26;
    A(6, 10) = -RHOFF_DD * F128;
    A(7, 10) = -RHOFF_DD * F129;
    A(8, 10) = -RHOFF_DD * F1210;
    A(9, 10) = 0.0;
    A(10, 10) = 1.0;
    A(11, 10) = Z12_BD;

    SOLMATS(N, A, XSOL);

    J1 = XSOL(1);
    J2 = XSOL(2);
    J3 = XSOL(3);
    J5 = XSOL(4);
    J6 = XSOL(5);
    J8 = XSOL(6);
    J9 = XSOL(7);
    J10 = XSOL(8);
    J11 = XSOL(9);
    J12 = XSOL(10);

    G1 = F12 * J2 + F13 * J3 + F15 * J5 + F16 * J6;
    G4 = F41 * J1 + F42 * J2 + F43 * J3 + F45 * J5 + F46 * J6;
    G7 = F78 * J8 + F79 * J9 + F710 * J10 + F711 * J11 + F712 * J12;
    G10 = F108 * J8 + F109 * J9 + F1011 * J11 + F1012 * J12;

    TAU_BB = (TAUFF_BB_PERP * (AB - NK) * std::abs(std::sin(OMEGA_H))) / (2.0 * S * std::abs(std::cos(OMEGA_H)));
    TAU_BD = (G4 + TAUFF_DD * G10) / 2.0;
    RHO_BD = (RHOFF_BT_PARL + TAUBF_DD * G1 + G7) / 2.0;
}

void PD_BEAM_CASE_IV(Real64 const S,                        // pleat spacing (> 0)
                     Real64 const W,                        // pleat depth (>=0, same units as S)
                     [[maybe_unused]] Real64 const OMEGA_H, // horizontal profile angle, radians
                     [[maybe_unused]] Real64 const DE,      // width of illumination on pleat bottom (same units as S)
                     Real64 const RHOFF_BT_PARL,
                     Real64 const TAUFF_BB_PARL,
                     Real64 const TAUFF_BD_PARL,
                     [[maybe_unused]] Real64 const RHOBF_BT_PARL,
                     [[maybe_unused]] Real64 const TAUBF_BB_PARL,
                     [[maybe_unused]] Real64 const TAUBF_BD_PARL,
                     Real64 const RHOFF_BT_PERP,
                     Real64 const TAUFF_BB_PERP,
                     Real64 const TAUFF_BD_PERP,
                     Real64 const RHOBF_BT_PERP,
                     Real64 const TAUBF_BB_PERP,
                     Real64 const TAUBF_BD_PERP,
                     Real64 const RHOBF_DD, // fabric back diffuse-diffuse reflectance
                     Real64 const RHOFF_DD, // fabric front diffuse-diffuse reflectance
                     Real64 const TAUFF_DD, // fabric front diffuse-diffuse transmittance
                     Real64 const TAUBF_DD, // fabric back diffuse-diffuse transmittance
                     Real64 &RHO_BD,        // returned: drape front beam-diffuse reflectance
                     Real64 &TAU_BD,        // returned: drape front beam-diffuse transmittance
                     Real64 &TAU_BB         // returned: drape front beam-beam transmittance
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  calculates the effective front-side solar optical properties of a drapery layer.
    // METHODOLOGY EMPLOYED:
    // Eight surface flat-fabric model with rectangular enclosure

    // fabric properties at current (off-normal) incidence
    //   _PARL = surface parallel to window (pleat top/bot)
    //   _PERP = surface perpendicular to window (pleat side)
    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr N(6);

    Real64 TAUBF_BT_PERP;
    Real64 AK; // length of diagonal strings
    Real64 CG;
    Real64 Z1_BB; // beam source term
    Real64 Z1_BD; // diffuse source terms
    Real64 Z2_BD;
    Real64 Z4_BD;
    Real64 Z6_BD;
    Real64 Z8_BD;
    // shape factors
    Real64 F12;
    Real64 F14;
    Real64 F21;
    Real64 F24;
    Real64 F31;
    Real64 F32;
    Real64 F34;
    Real64 F41;
    Real64 F42;
    Real64 F56;
    Real64 F57;
    Real64 F58;
    Real64 F67;
    Real64 F68;
    Real64 F76;
    Real64 F78;
    Real64 F86;
    Real64 F87;
    Real64 J1; // radiosity, surface i
    Real64 J2;
    Real64 J4;
    Real64 J6;
    Real64 J7;
    Real64 J8;
    Real64 G1; // irradiance, surface i
    Real64 G3;
    Real64 G5;
    Real64 G7;
    Array2D<Real64> A(N + 2, N); // coefficients of the radiosity equations matrix
    Array1D<Real64> XSOL(N);     // solution vector (obtained after solving the radiosity equations matrix)

    TAUBF_BT_PERP = TAUBF_BD_PERP + TAUBF_BB_PERP;

    AK = std::sqrt(W * W + S * S);
    CG = AK;

    Z1_BB = TAUFF_BB_PARL;
    Z1_BD = TAUFF_BD_PARL;
    Z2_BD = Z1_BB * RHOBF_BT_PERP * S / W;
    Z4_BD = TAUFF_BD_PERP * S / W;
    Z6_BD = RHOFF_BT_PERP * S / W;
    Z8_BD = Z1_BB * TAUBF_BT_PERP * S / W;

    F12 = (S + W - AK) / (2.0 * S);
    F14 = (S + W - CG) / (2.0 * S);
    F21 = (S + W - AK) / (2.0 * W);
    F24 = (AK + CG - 2.0 * S) / (2.0 * W);
    F31 = (AK + CG - 2.0 * W) / (2.0 * S);
    F32 = F12;
    F34 = F12;
    F41 = F21;
    F42 = F24;
    F56 = F12;
    F57 = F31;
    F58 = F14;
    F67 = F41;
    F68 = F24;
    F76 = F32;
    F78 = F34;
    F86 = F42;
    F87 = F21;

    A = 0.0;    // INITIALIZE RADIOSITY MATRIX COEFFICIENTS
    XSOL = 0.0; // INITIALIZE SOLUTION VECTOR COEFFICIENTS

    // POPULATE THE COEFFICIENTS OF THE RADIOSITY MATRIX

    A(1, 1) = 1.0;
    A(2, 1) = -RHOBF_DD * F12;
    A(3, 1) = -RHOBF_DD * F14;
    A(4, 1) = 0.0;
    A(5, 1) = 0.0;
    A(6, 1) = 0.0;
    A(7, 1) = Z1_BD;
    A(1, 2) = -RHOBF_DD * F21;
    A(2, 2) = 1.0;
    A(3, 2) = -RHOBF_DD * F24;
    A(4, 2) = -TAUFF_DD * F86;
    A(5, 2) = -TAUFF_DD * F87;
    A(6, 2) = 0.0;
    A(7, 2) = Z2_BD;
    A(1, 3) = -RHOBF_DD * F41;
    A(2, 3) = -RHOBF_DD * F42;
    A(3, 3) = 1.0;
    A(4, 3) = 0.0;
    A(5, 3) = -TAUFF_DD * F67;
    A(6, 3) = -TAUFF_DD * F68;
    A(7, 3) = Z4_BD;
    A(1, 4) = -TAUBF_DD * F41;
    A(2, 4) = -TAUBF_DD * F42;
    A(3, 4) = 0.0;
    A(4, 4) = 1.0;
    A(5, 4) = -RHOFF_DD * F67;
    A(6, 4) = -RHOFF_DD * F68;
    A(7, 4) = Z6_BD;
    A(1, 5) = 0.0;
    A(2, 5) = 0.0;
    A(3, 5) = 0.0;
    A(4, 5) = -RHOFF_DD * F76;
    A(5, 5) = 1.0;
    A(6, 5) = -RHOFF_DD * F78;
    A(7, 5) = 0.0;
    A(1, 6) = -TAUBF_DD * F21;
    A(2, 6) = 0.0;
    A(3, 6) = -TAUBF_DD * F24;
    A(4, 6) = -RHOFF_DD * F86;
    A(5, 6) = -RHOFF_DD * F87;
    A(6, 6) = 1.0;
    A(7, 6) = Z8_BD;

    SOLMATS(N, A, XSOL);

    J1 = XSOL(1);
    J2 = XSOL(2);
    J4 = XSOL(3);
    J6 = XSOL(4);
    J7 = XSOL(5);
    J8 = XSOL(6);

    G1 = F12 * J2 + F14 * J4;
    G3 = F31 * J1 + F32 * J2 + F34 * J4;
    G5 = F56 * J6 + F57 * J7 + F58 * J8;
    G7 = F76 * J6 + F78 * J8;

    TAU_BB = TAUFF_BB_PERP / 2.0;
    TAU_BD = (G3 + TAUFF_DD * G7) / 2.0;
    RHO_BD = (RHOFF_BT_PARL + TAUBF_DD * G1 + G5) / 2.0;
}

void PD_BEAM_CASE_V(Real64 const S,       // pleat spacing (> 0)
                    Real64 const W,       // pleat depth (>=0, same units as S)
                    Real64 const OMEGA_H, // horizontal profile angle, radians
                    Real64 const DE,      // width of illumination on pleat bottom (same units as S)
                    Real64 const RHOFF_BT_PARL,
                    Real64 const TAUFF_BB_PARL,
                    Real64 const TAUFF_BD_PARL,
                    [[maybe_unused]] Real64 const RHOBF_BT_PARL,
                    [[maybe_unused]] Real64 const TAUBF_BB_PARL,
                    [[maybe_unused]] Real64 const TAUBF_BD_PARL,
                    Real64 const RHOFF_BT_PERP,
                    Real64 const TAUFF_BB_PERP,
                    Real64 const TAUFF_BD_PERP,
                    Real64 const RHOBF_BT_PERP,
                    Real64 const TAUBF_BB_PERP,
                    Real64 const TAUBF_BD_PERP,
                    Real64 const RHOBF_DD, // fabric back diffuse-diffuse reflectance
                    Real64 const RHOFF_DD, // fabric front diffuse-diffuse reflectance
                    Real64 const TAUFF_DD, // fabric front diffuse-diffuse transmittance
                    Real64 const TAUBF_DD, // fabric back diffuse-diffuse transmittance
                    Real64 &RHO_BD,        // returned: drape front beam-diffuse reflectance
                    Real64 &TAU_BD,        // returned: drape front beam-diffuse transmittance
                    Real64 &TAU_BB         // returned: drape front beam-beam transmittance
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  calculates the effective front-side solar optical properties of a drapery layer.
    // METHODOLOGY EMPLOYED:
    // NINE SURFACE FLAT-FABRIC MODEL WITH RECTANGULAR ENCLOSURE

    // fabric properties at current (off-normal) incidence
    //   _PARL = surface parallel to window (pleat top/bot)
    //   _PERP = surface perpendicular to window (pleat side)
    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr N(7);

    Real64 TAUBF_BT_PERP;
    Real64 AK; // lengths of surfaces and diagonal strings
    Real64 CG;
    Real64 MK;
    Real64 DK;
    Real64 MF;
    Real64 DM;
    Real64 GM;
    Real64 GF;
    Real64 Z1_BB; // beam source term
    Real64 Z1_BD; // diffuse source terms
    Real64 Z2_BD;
    Real64 Z4_BD;
    Real64 Z6_BD;
    Real64 Z7_BD;
    Real64 Z9_BD;
    // shape factors
    Real64 F12;
    Real64 F14;
    Real64 F21;
    Real64 F24;
    Real64 F31;
    Real64 F32;
    Real64 F34;
    Real64 F41;
    Real64 F42;
    Real64 F56;
    Real64 F57;
    Real64 F58;
    Real64 F59;
    Real64 F67;
    Real64 F68;
    Real64 F69;
    Real64 F76;
    Real64 F79;
    Real64 F86;
    Real64 F89;
    Real64 F96;
    Real64 F97;
    Real64 F98;
    Real64 J1; // radiosities
    Real64 J2;
    Real64 J4;
    Real64 J6;
    Real64 J7;
    Real64 J8;
    Real64 J9;
    Real64 G1; // irradiances
    Real64 G3;
    Real64 G5;
    Real64 G7;
    Real64 G8;

    Array2D<Real64> A(N + 2, N); // coefficients of the radiosity equations matrix
    Array1D<Real64> XSOL(N);     // solution vector (obtained after solving the radiosity equations matrix)

    TAUBF_BT_PERP = TAUBF_BD_PERP + TAUBF_BB_PERP;

    AK = std::sqrt(W * W + S * S);
    CG = AK;
    Real64 const cos_OMEGA_H(std::abs(std::cos(OMEGA_H)));
    Real64 const sin_OMEGA_H(std::abs(std::sin(OMEGA_H)));
    MK = (W * sin_OMEGA_H) / cos_OMEGA_H;
    DK = AK;
    MF = S - MK;
    DM = std::sqrt(W * W + MF * MF);
    GM = std::sqrt(W * W + MK * MK);
    GF = AK;

    Z1_BB = TAUFF_BB_PARL;
    Z1_BD = TAUFF_BD_PARL;
    Z2_BD = Z1_BB * RHOBF_BT_PERP * S / DE;
    Z4_BD = TAUFF_BD_PERP * S / DE;
    Z6_BD = RHOFF_BT_PERP * S / DE;
    Z7_BD = RHOFF_BT_PARL;
    Z9_BD = Z1_BB * TAUBF_BT_PERP * S / DE;

    F12 = (S + W - AK) / (2.0 * S);
    F14 = (S + W - CG) / (2.0 * S);
    F21 = (S + W - AK) / (2.0 * W);
    F24 = (AK + CG - 2.0 * S) / (2.0 * W);
    F31 = (AK + CG - 2.0 * W) / (2.0 * S);
    F32 = F14;
    F34 = F12;
    F41 = F21;
    F42 = F24;
    F56 = F12;
    F57 = (DM + GF - (GM + W)) / (2.0 * S);
    F58 = (DK + GM - (DM + W)) / (2.0 * S);
    F59 = F14;
    F67 = (W + MF - DM) / (2.0 * W);
    F68 = (DM + S - (DK + MF)) / (2.0 * W);
    F69 = F24;
    F76 = (W + MF - DM) / (2.0 * MF);
    F79 = (GM + S - (GF + MK)) / (2.0 * MF);
    F86 = (DM + S - (DK + MF)) / (2.0 * MK);
    F89 = (W + MK - GM) / (2.0 * MK);
    F96 = F42;
    F97 = (GM + S - (GF + MK)) / (2.0 * W);
    F98 = (W + MK - GM) / (2.0 * W);

    A = 0.0;    // INITIALIZE RADIOSITY MATRIX COEFFICIENTS
    XSOL = 0.0; // INITIALIZE SOLUTION VECTOR COEFFICIENTS

    // POPULATE THE COEFFICIENTS OF THE RADIOSITY MATRIX

    A(1, 1) = 1.0;
    A(2, 1) = -RHOBF_DD * F12;
    A(3, 1) = -RHOBF_DD * F14;
    A(4, 1) = 0.0;
    A(5, 1) = 0.0;
    A(6, 1) = 0.0;
    A(7, 1) = 0.0;
    A(8, 1) = Z1_BD;
    A(1, 2) = -RHOBF_DD * F21;
    A(2, 2) = 1.0;
    A(3, 2) = -RHOBF_DD * F24;
    A(4, 2) = -TAUFF_DD * F96;
    A(5, 2) = -TAUFF_DD * F97;
    A(6, 2) = -TAUFF_DD * F98;
    A(7, 2) = 0.0;
    A(8, 2) = Z2_BD;
    A(1, 3) = -RHOBF_DD * F41;
    A(2, 3) = -RHOBF_DD * F42;
    A(3, 3) = 1.0;
    A(4, 3) = 0.0;
    A(5, 3) = -TAUFF_DD * F67;
    A(6, 3) = -TAUFF_DD * F68;
    A(7, 3) = -TAUFF_DD * F69;
    A(8, 3) = Z4_BD;
    A(1, 4) = -TAUBF_DD * F41;
    A(2, 4) = -TAUBF_DD * F42;
    A(3, 4) = 0.0;
    A(4, 4) = 1.0;
    A(5, 4) = -RHOFF_DD * F67;
    A(6, 4) = -RHOFF_DD * F68;
    A(7, 4) = -RHOFF_DD * F69;
    A(8, 4) = Z6_BD;
    A(1, 5) = 0.0;
    A(2, 5) = 0.0;
    A(3, 5) = 0.0;
    A(4, 5) = -RHOFF_DD * F76;
    A(5, 5) = 1.0;
    A(6, 5) = 0.0;
    A(7, 5) = -RHOFF_DD * F79;
    A(8, 5) = Z7_BD;
    A(1, 6) = 0.0;
    A(2, 6) = 0.0;
    A(3, 6) = 0.0;
    A(4, 6) = -RHOFF_DD * F86;
    A(5, 6) = 0.0;
    A(6, 6) = 1.0;
    A(7, 6) = -RHOFF_DD * F89;
    A(8, 6) = 0.0;
    A(1, 7) = -TAUBF_DD * F21;
    A(2, 7) = 0.0;
    A(3, 7) = -TAUBF_DD * F24;
    A(4, 7) = -RHOFF_DD * F96;
    A(5, 7) = -RHOFF_DD * F97;
    A(6, 7) = -RHOFF_DD * F98;
    A(7, 7) = 1.0;
    A(8, 7) = Z9_BD;

    SOLMATS(N, A, XSOL);

    J1 = XSOL(1);
    J2 = XSOL(2);
    J4 = XSOL(3);
    J6 = XSOL(4);
    J7 = XSOL(5);
    J8 = XSOL(6);
    J9 = XSOL(7);

    G1 = F12 * J2 + F14 * J4;
    G3 = F31 * J1 + F32 * J2 + F34 * J4;
    G5 = F56 * J6 + F57 * J7 + F58 * J8 + F59 * J9;
    G7 = F76 * J6 + F79 * J9;
    G8 = F86 * J6 + F89 * J9;

    TAU_BB = (2.0 * (DE - W) * sin_OMEGA_H * TAUFF_BB_PARL + (S * cos_OMEGA_H - (DE - W) * sin_OMEGA_H) * TAUFF_BB_PERP) / (2.0 * S * cos_OMEGA_H);
    TAU_BD = (S * G3 + TAUFF_DD * (MK * G8 + MF * G7) + MF * TAUFF_BD_PARL) / (2.0 * S);
    RHO_BD = (RHOFF_BT_PARL + TAUBF_DD * G1 + G5) / 2.0;
}

void PD_BEAM_CASE_VI(Real64 const S,                        // pleat spacing (> 0)
                     Real64 const W,                        // pleat depth (>=0, same units as S)
                     [[maybe_unused]] Real64 const OMEGA_H, // horizontal profile angle, radians
                     [[maybe_unused]] Real64 const DE,      // width of illumination on pleat bottom (same units as S)
                     Real64 const RHOFF_BT_PARL,
                     Real64 const TAUFF_BB_PARL,
                     Real64 const TAUFF_BD_PARL,
                     [[maybe_unused]] Real64 const RHOBF_BT_PARL,
                     [[maybe_unused]] Real64 const TAUBF_BB_PARL,
                     [[maybe_unused]] Real64 const TAUBF_BD_PARL,
                     [[maybe_unused]] Real64 const RHOFF_BT_PERP,
                     [[maybe_unused]] Real64 const TAUFF_BB_PERP,
                     [[maybe_unused]] Real64 const TAUFF_BD_PERP,
                     [[maybe_unused]] Real64 const RHOBF_BT_PERP,
                     [[maybe_unused]] Real64 const TAUBF_BB_PERP,
                     [[maybe_unused]] Real64 const TAUBF_BD_PERP,
                     Real64 const RHOBF_DD, // fabric back diffuse-diffuse reflectance
                     Real64 const RHOFF_DD, // fabric front diffuse-diffuse reflectance
                     Real64 const TAUFF_DD, // fabric front diffuse-diffuse transmittance
                     Real64 const TAUBF_DD, // fabric back diffuse-diffuse transmittance
                     Real64 &RHO_BD,        // returned: drape front beam-diffuse reflectance
                     Real64 &TAU_BD,        // returned: drape front beam-diffuse transmittance
                     Real64 &TAU_BB         // returned: drape front beam-beam transmittance
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  calculates the effective front-side solar optical properties of a drapery layer.
    // METHODOLOGY EMPLOYED:
    // EIGHT SURFACE FLAT-FABRIC MODEL WITH RECTANGULAR ENCLOSURE

    // fabric properties at current (off-normal) incidence
    //   _PARL = surface parallel to window (pleat top/bot)
    //   _PERP = surface perpendicular to window (pleat side)
    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr N(6);

    Real64 AK; // length of diagonal strings
    Real64 CG;
    Real64 Z1_BD; // diffuse source terms
    Real64 Z7_BD;
    // shape factors
    Real64 F12;
    Real64 F14;
    Real64 F21;
    Real64 F24;
    Real64 F31;
    Real64 F32;
    Real64 F34;
    Real64 F41;
    Real64 F42;
    Real64 F56;
    Real64 F57;
    Real64 F58;
    Real64 F67;
    Real64 F68;
    Real64 F76;
    Real64 F78;
    Real64 F86;
    Real64 F87;
    Real64 J1; // radiosity, surface i
    Real64 J2;
    Real64 J4;
    Real64 J6;
    Real64 J7;
    Real64 J8;
    Real64 G1; // irradiance, surface i
    Real64 G3;
    Real64 G5;
    Real64 G7;
    Array2D<Real64> A(N + 2, N); // coefficients of the radiosity equations matrix
    Array1D<Real64> XSOL(N);     // solution vector (obtained after solving the radiosity equations matrix)

    AK = std::sqrt(W * W + S * S);
    CG = AK;

    Z1_BD = TAUFF_BD_PARL;
    Z7_BD = RHOFF_BT_PARL;

    F12 = (S + W - AK) / (2.0 * S);
    F14 = (S + W - CG) / (2.0 * S);
    F21 = (S + W - AK) / (2.0 * W);
    F24 = (AK + CG - 2.0 * S) / (2.0 * W);
    F31 = (AK + CG - 2.0 * W) / (2.0 * S);
    F32 = F12;
    F34 = F14;
    F41 = F21;
    F42 = F24;
    F56 = F12;
    F57 = F31;
    F58 = F14;
    F67 = F41;
    F68 = F24;
    F76 = F14;
    F78 = F14;
    F86 = F42;
    F87 = F21;

    A = 0.0;
    XSOL = 0.0;

    // POPULATE THE COEFFICIENTS OF THE RADIOSITY MATRIX

    A(1, 1) = 1.0;
    A(2, 1) = -RHOBF_DD * F12;
    A(3, 1) = -RHOBF_DD * F14;
    A(4, 1) = 0.0;
    A(5, 1) = 0.0;
    A(6, 1) = 0.0;
    A(7, 1) = Z1_BD;
    A(1, 2) = -RHOBF_DD * F21;
    A(2, 2) = 1.0;
    A(3, 2) = -RHOBF_DD * F24;
    A(4, 2) = -TAUFF_DD * F86;
    A(5, 2) = -TAUFF_DD * F87;
    A(6, 2) = 0.0;
    A(7, 2) = 0.0;
    A(1, 3) = -RHOBF_DD * F41;
    A(2, 3) = -RHOBF_DD * F42;
    A(3, 3) = 1.0;
    A(4, 3) = 0.0;
    A(5, 3) = -TAUFF_DD * F67;
    A(6, 3) = -TAUFF_DD * F68;
    A(7, 3) = 0.0;
    A(1, 4) = -TAUBF_DD * F41;
    A(2, 4) = -TAUBF_DD * F42;
    A(3, 4) = 0.0;
    A(4, 4) = 1.0;
    A(5, 4) = -RHOFF_DD * F67;
    A(6, 4) = -RHOFF_DD * F68;
    A(7, 4) = 0.0;
    A(1, 5) = 0.0;
    A(2, 5) = 0.0;
    A(3, 5) = 0.0;
    A(4, 5) = -RHOFF_DD * F76;
    A(5, 5) = 1.0;
    A(6, 5) = -RHOFF_DD * F78;
    A(7, 5) = Z7_BD;
    A(1, 6) = -TAUBF_DD * F21;
    A(2, 6) = 0.0;
    A(3, 6) = -TAUBF_DD * F24;
    A(4, 6) = -RHOFF_DD * F86;
    A(5, 6) = -RHOFF_DD * F87;
    A(6, 6) = 1.0;
    A(7, 6) = 0.0;

    SOLMATS(N, A, XSOL);

    J1 = XSOL(1);
    J2 = XSOL(2);
    J4 = XSOL(3);
    J6 = XSOL(4);
    J7 = XSOL(5);
    J8 = XSOL(6);

    G1 = F12 * J2 + F14 * J4;
    G3 = F31 * J1 + F32 * J2 + F34 * J4;
    G5 = F56 * J6 + F57 * J7 + F58 * J8;
    G7 = F76 * J6 + F78 * J8;

    TAU_BB = TAUFF_BB_PARL;
    TAU_BD = (G3 + TAUFF_DD * G7 + TAUFF_BD_PARL) / 2.0;
    RHO_BD = (RHOFF_BT_PARL + TAUBF_DD * G1 + G5) / 2.0;
}

void VB_DIFF(EnergyPlusData &state,
             Real64 const S,           // slat spacing (any length units; same units as W)
             Real64 const W,           // slat tip-to-tip width (any length units; same units as S)
             Real64 const PHI,         // slat angle, radians (-PI/2 <= PHI <= PI/2)
             Real64 const RHODFS_SLAT, // reflectance of downward-facing slat surfaces (concave?)
             Real64 const RHOUFS_SLAT, // reflectance of upward-facing slat surfaces (convex?)
             Real64 const TAU_SLAT,    // diffuse transmittance of slats
             Real64 &RHOFVB,           // returned: front side effective diffuse reflectance of venetian blind
             Real64 &TAUVB             // returned: effective diffuse transmittance of venetian blind
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  Calculates the venetian blind layer effective diffuse transmittance and reflectance.
    // METHODOLOGY EMPLOYED:
    // four surface flat-slat model with slat transmittance

    // SUBROUTINE ARGUMENT DEFINITIONS:
    //   must be > 0
    //   ltyVBHOR: + = front-side slat tip below horizontal
    //   ltyVBVER: + = front-side slat tip is counter-
    //                 clockwise from normal (viewed from above)
    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view Tau_Name("VB_DIFF Tau");
    static constexpr std::string_view RhoF_Name("VB_DIFF RhoF");

    Real64 CD; // lengths of the diagonal strings used in the four-surface model
    Real64 AF;
    Real64 F13; // shape factors
    Real64 F14;
    Real64 F12;
    Real64 F31;
    Real64 F41;
    Real64 FSS;
    Real64 C3; // temporaries
    Real64 B3;
    Real64 C4;
    Real64 B4;
    Real64 K3;
    Real64 K4;
    Real64 DEN;

    Real64 const W_cos_PHI_2(pow_2(W * std::cos(PHI)));
    Real64 const W_sin_PHI(W * std::sin(PHI));
    CD = std::sqrt(W_cos_PHI_2 + pow_2(S + W_sin_PHI));
    AF = std::sqrt(W_cos_PHI_2 + pow_2(S - W_sin_PHI));

    F13 = (W + S - CD) / (2.0 * S);    // SHAPE FACTOR FRONT OPENING TO TOP SLAT
    F14 = (W + S - AF) / (2.0 * S);    // SHAPE FACTOR FRONT OPENING TO BOTTOM SLAT
    FSS = 1.0 - (S / W) * (F13 + F14); // SLAT-TO-SLAT SHAPE FACTOR
    F31 = (S / W) * F13;               // SHAPE FACTOR - TOP TO FRONT
    F41 = (S / W) * F14;               // SHAPE FACTOR - BOTTOM TO FRONT
    F12 = 1.0 - F13 - F14;             // FRONT OPENING TO BACK OPENING SHAPE FACTOR
    DEN = 1.0 - (TAU_SLAT * FSS);      // DENOMINATOR - USED FOUR TIMES
    C3 = (RHODFS_SLAT * F31 + TAU_SLAT * F41) / DEN;
    B3 = (RHODFS_SLAT * FSS) / DEN;
    C4 = (RHOUFS_SLAT * F41 + TAU_SLAT * F31) / DEN;
    B4 = (RHOUFS_SLAT * FSS) / DEN;

    K3 = (C3 + (B3 * C4)) / (1.0 - (B3 * B4));
    K4 = (C4 + (B4 * C3)) / (1.0 - (B3 * B4));
    // transmittance of VB (equal front/back)
    TAUVB = P01(state, F12 + (F14 * K3) + (F13 * K4), Tau_Name);
    // diffuse reflectance of VB front-side
    RHOFVB = P01(state, (F13 * K3) + (F14 * K4), RhoF_Name);
}

Real64 VB_SLAT_RADIUS_RATIO(Real64 const W, // slat tip-to-tip (chord) width (any units; same units as C) must be > 0
                            Real64 const C  // slat crown height (any units, same units as W) must be >= 0
)
{
    //       AUTHOR         ASHRAE 1311-RP

    // PURPOSE OF THIS FUNCTION:
    //  Returns curved slat radius ratio (W / R)

    if (C <= 0.0 || W <= 0.0) {
        // it is flat
        return 0.0;
    } else {
        Real64 CX = min(C, W / 2.001);
        return 2.0 * W * CX / (CX * CX + W * W / 4);
    }
}

void VB_SOL46_CURVE(EnergyPlusData const &state,
                    Real64 const S,           // slat spacing (any length units; same units as W)
                    Real64 const W,           // slat tip-to-tip (chord) width (any length units; same units as S)
                    Real64 const SL_WR,       // slat curvature radius ratio (= W/R)
                    Real64 const PHIx,        // slat angle, radians (-PI/2 <= PHI <= PI/2)
                    Real64 const OMEGAx,      // incident beam profile angle (radians)
                    Real64 const RHODFS_SLAT, // SW (solar) reflectance downward-facing slat surfaces (concave?)
                    Real64 const RHOUFS_SLAT, // SW (solar) reflectance upward-facing slat surfaces (convex?)
                    Real64 const TAU_SLAT,    // SW (solar) transmittance of slats
                    Real64 &RHO_BD,           // returned: effective SW (solar) beam-to-diffuse reflectance front side
                    Real64 &TAU_BB,           // returned: effective SW (solar) beam-to-beam transmittance front side
                    Real64 &TAU_BD            // returned: effective SW (solar) beam-to-diffuse transmittance front side
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  Calculates the venetian blind layer effective solar transmittance and reflectance.
    // METHODOLOGY EMPLOYED:
    // Four and six surface curve-slat model with slat transmittance. For back side
    // reflectance call this routine a second time with the same input data - except
    // negative the slat angle, PHI_DEG.

    // SUBROUTINE ARGUMENT DEFINITIONS:
    //    must be > 0
    //   must be > 0
    //   0 = flat
    //   ltyVBHOR: + = front-side slat tip below horizontal
    //   ltyVBVER: + = front-side slat tip is counter-
    //                 clockwise from normal (viewed from above)
    //   ltyVBHOR: +=above horizontal
    //   ltyVBVER: +=clockwise when viewed from above
    //   Note: All solar slat properties are incident-to-diffuse
    //         Specular effects not covered by model

    Real64 DE; // distance from front tip of any slat to shadow (caused by the adjacent slat) on
    // the plane of the same slat; DE may be greater than the slat width, W
    Real64 PHI;
    Real64 OMEGA;
    Real64 SL_RAD;
    Real64 SL_THETA;
    Real64 Slope;
    Real64 T_CORR_D;
    Real64 T_CORR_F;
    Real64 RHO_TEMP;
    Real64 TAU_TEMP;
    Real64 XA;
    Real64 XB;
    Real64 XC;
    Real64 XD;
    Real64 XE;
    Real64 XF(0);
    Real64 YA;
    Real64 YB;
    Real64 YC;
    Real64 YD;
    Real64 YE;
    Real64 YF(0);
    int CORR;

    DE = 0.0; // INITIALIZE DE
    CORR = 1;

    // limit slat angle to +/- 90 deg
    PHI = max(-Constant::DegToRad * 90.0, min(Constant::DegToRad * 90.0, PHIx));
    // limit profile angle to +/- 89.5 deg
    OMEGA = max(-Constant::DegToRad * 89.5, min(Constant::DegToRad * 89.5, OMEGAx));

    SL_RAD = W / max(SL_WR, 0.0000001);
    SL_THETA = 2.0 * std::asin(0.5 * SL_WR);

    if (CORR > 0) { // CORRECT FOR SLAT CURVATURE BY SETTING CORR = 1

        //  DETERMINE BOUNDS FOR CURVATURE CORRECTION AND APPLY CORRECTION TO BEAM-BEAM TRANSMITTANCE
        if (std::abs(PHI + OMEGA) < SL_THETA / 2.0) {
            //  CALCULATE BEAM TRANSMISSION
            XA = SL_RAD * std::sin(-SL_THETA / 2.0); // Glass-side end coordinate
            YA = SL_RAD * std::cos(-SL_THETA / 2.0);
            XB = -XA; // Indoor-side end coordinate
            YB = YA;
            YC = SL_RAD * std::cos(PHI + OMEGA); // Tangent to slat in irradiance direction
            XC = std::sqrt(pow_2(SL_RAD) - pow_2(YC));
            Slope = -XC / YC;
            if (std::abs(Slope) < state.dataWindowEquivalentLayer->SMALL_ERROR) {
                XD = 0.0;
                YD = YA;
                XE = 0.0;
                YE = YD;
                // Bug XF, YF not set but used below (XE, YE are set but NOT used)
            } else {
                if ((PHI + OMEGA) < 0.0) {
                    XC = -XC;
                    Slope = -Slope;
                    XD = (YB - Slope * XB) / (-1.0 / Slope - Slope);
                    XF = (YA - Slope * XA) / (-1.0 / Slope - Slope);
                    XE = XA + 2.0 * std::abs(XA - XF);
                } else {
                    XD = (YA - Slope * XA) / (-1.0 / Slope - Slope);
                    XF = (YB - Slope * XB) / (-1.0 / Slope - Slope);
                    XE = XB - 2.0 * std::abs(XB - XF);
                }
                YD = -XD / Slope;
                YE = -XE / Slope;
                YF = -XF / Slope;
            }

            T_CORR_D = std::sqrt(pow_2(XC - XD) + pow_2(YC - YD)); // Slat thickness perpendicular to light direction
            T_CORR_F = std::sqrt(pow_2(XC - XF) + pow_2(YC - YF));

            TAU_BB = 1.0 - T_CORR_D / (S * std::cos(OMEGA));

        } else {
            // DO NOT APPLY CURVATURE CORRECTION TO BEAM-BEAM TRANSMITTANCE
            if (std::abs(OMEGA + PHI) < 0.0001) {
                DE = S * 1000000.0;
            } else {
                DE = S * std::abs(std::cos(OMEGA) / std::sin(OMEGA + PHI));
            }
            //  CHECK TO SEE IF THERE IS DIRECT BEAM TRANSMISSION
            if ((DE / W) > (1.0 - state.dataWindowEquivalentLayer->SMALL_ERROR)) { // YES
                TAU_BB = max(0.0, (DE - W) / DE);
            } else { // NO
                TAU_BB = 0.0;
            }
        }

        // CHECK TO SEE IF CURVATURE CORRECTION INCLUDES DOUBLE BLOCKAGE
        // (TAU_BB < 0.0 AND SET TAU_BB = 0.0)
        if (TAU_BB < 0.0) { // YES, THERE IS DOUBLE BLOCKAGE

            TAU_BB = 0.0;

            // DO NOT APPLY CURVATURE CORRECTION TO RHO_BD, TAU_BD IF TAU_BB < 0.0
            if (std::abs(OMEGA + PHI) < 0.0001) {
                DE = S * 1000000.0;
            } else {
                DE = S * std::abs(std::cos(OMEGA) / std::sin(OMEGA + PHI));
            }
            if ((DE / W) > (1.0 - state.dataWindowEquivalentLayer->SMALL_ERROR)) { // YES
                VB_SOL4(state, S, W, OMEGA, DE, PHI, RHODFS_SLAT, RHOUFS_SLAT, TAU_SLAT, RHO_BD, TAU_BD);

            } else { // NO
                VB_SOL6(state, S, W, OMEGA, DE, PHI, RHODFS_SLAT, RHOUFS_SLAT, TAU_SLAT, RHO_BD, TAU_BD);
            }

        } else { // NO, THERE IS NO DOUBLE BLOCKAGE

            if (std::abs(PHI + OMEGA) < (SL_THETA / 2.0)) { // YES, APPLY CURVATURE CORRECTION

                XA = SL_RAD * std::sin(-SL_THETA / 2.0); // Glass-side end coordinate
                YA = SL_RAD * std::cos(-SL_THETA / 2.0);
                XB = -XA; // Indoor-side end coordinate
                YB = YA;
                YC = SL_RAD * std::cos(PHI + OMEGA); // Tangent to slat in irradiance direction
                XC = std::sqrt(pow_2(SL_RAD) - pow_2(YC));
                Slope = -XC / YC;
                if (std::abs(Slope) < state.dataWindowEquivalentLayer->SMALL_ERROR) {
                    XD = 0.0;
                    YD = YA;
                    XE = 0.0;
                    YE = YD;
                    // Bug XF, YF not set but used below (XE, YE are set but NOT used)
                } else {
                    if ((PHI + OMEGA) < 0.0) {
                        XC = -XC;
                        Slope = -Slope;
                        XD = (YB - Slope * XB) / (-1.0 / Slope - Slope);
                        XF = (YA - Slope * XA) / (-1.0 / Slope - Slope);
                        XE = XA + 2.0 * std::abs(XA - XF);
                    } else {
                        XD = (YA - Slope * XA) / (-1.0 / Slope - Slope);
                        XF = (YB - Slope * XB) / (-1.0 / Slope - Slope);
                        XE = XB - 2.0 * std::abs(XB - XF);
                    }
                    YD = -XD / Slope;
                    YE = -XE / Slope;
                    YF = -XF / Slope;
                }
                T_CORR_D = std::sqrt(pow_2(XC - XD) + pow_2(YC - YD)); // Slat thickness perpendicular to light direction
                T_CORR_F = std::sqrt(pow_2(XC - XF) + pow_2(YC - YF));

                if ((PHI + OMEGA) >= 0.0) { // Slat is lit from above
                    DE = XC - XA;
                    VB_SOL6(state, S, W, OMEGA, DE, PHI, RHODFS_SLAT, RHOUFS_SLAT, TAU_SLAT, RHO_BD, TAU_BD);
                    Real64 const S_cos_OMEGA_inv(1.0 / (S * std::cos(OMEGA)));
                    RHO_BD *= T_CORR_D * S_cos_OMEGA_inv;
                    TAU_BD *= T_CORR_D * S_cos_OMEGA_inv;
                } else { // Slat is lit from below
                    DE = XC - XA;
                    VB_SOL6(state, S, W, OMEGA, DE, PHI, RHODFS_SLAT, RHOUFS_SLAT, TAU_SLAT, RHO_BD, TAU_BD);
                    Real64 const S_cos_OMEGA_inv(1.0 / (S * std::cos(OMEGA)));
                    RHO_TEMP = RHO_BD * T_CORR_F * S_cos_OMEGA_inv;
                    TAU_TEMP = TAU_BD * T_CORR_F * S_cos_OMEGA_inv;
                    DE = std::abs(XB - XF);
                    VB_SOL6(state, S, W, OMEGA, DE, PHI, RHODFS_SLAT, RHOUFS_SLAT, TAU_SLAT, RHO_BD, TAU_BD);
                    RHO_BD = RHO_BD * (T_CORR_D - T_CORR_F) * S_cos_OMEGA_inv + RHO_TEMP;
                    TAU_BD = TAU_BD * (T_CORR_D - T_CORR_F) * S_cos_OMEGA_inv + TAU_TEMP;
                }

            } else { // NO, DO NOT APPLY CURVATURE CORRECTION
                if (std::abs(OMEGA + PHI) < 0.0001) {
                    DE = S * 1000000.0;
                } else {
                    DE = S * std::abs(std::cos(OMEGA) / std::sin(OMEGA + PHI));
                }
                if (DE / W > 1.0 - state.dataWindowEquivalentLayer->SMALL_ERROR) { // YES
                    VB_SOL4(state, S, W, OMEGA, DE, PHI, RHODFS_SLAT, RHOUFS_SLAT, TAU_SLAT, RHO_BD, TAU_BD);

                } else { // NO
                    VB_SOL6(state, S, W, OMEGA, DE, PHI, RHODFS_SLAT, RHOUFS_SLAT, TAU_SLAT, RHO_BD, TAU_BD);
                }
            }
        }

    } else { // DO NOT CORRECT FOR SLAT CURVATURE

        //  CHECK TO SEE IF BEAM IS ALLIGNED WITH SLATS
        if (std::abs(PHI + OMEGA) < state.dataWindowEquivalentLayer->SMALL_ERROR) { // YES!
            RHO_BD = 0.0;
            TAU_BB = 1.0;
            TAU_BD = 0.0;

        } else { // BEAM NOT ALIGNED WITH SLATS
            RHO_BD = 0.0;
            TAU_BB = 0.0;
            TAU_BD = 0.0;
            DE = S * std::abs(std::cos(OMEGA) / std::sin(OMEGA + PHI));
            //  CHECK TO SEE IF THERE IS DIRECT BEAM TRANSMISSION
            if ((DE / W) > (1.0 - state.dataWindowEquivalentLayer->SMALL_ERROR)) { // YES
                TAU_BB = (DE - W) / DE;
                if (TAU_BB < 0.0) {
                    TAU_BB = 0.0;
                }
                VB_SOL4(state, S, W, OMEGA, DE, PHI, RHODFS_SLAT, RHOUFS_SLAT, TAU_SLAT, RHO_BD, TAU_BD);
            } else { // NO
                TAU_BB = 0.0;
                VB_SOL6(state, S, W, OMEGA, DE, PHI, RHODFS_SLAT, RHOUFS_SLAT, TAU_SLAT, RHO_BD, TAU_BD);
            } //  END CHECK FOR DIRECT BEAM TRANSMISSION
        } // END CHECK TO SEE IF BEAM ALLIGNED WITH SLATS
    }
}

void VB_SOL4(EnergyPlusData const &state,
             Real64 const S,           // slat spacing (any length units; same units as W)
             Real64 const W,           // slat tip-to-tip width (any length units; same units as S)
             Real64 const OMEGA,       // incident beam profile angle (radians)
             Real64 const DE,          // distance from front tip of any slat to shadow (caused by the adjacent slat) on
             Real64 const PHI,         // slat angle, radians (-PI/2 <= PHI <= PI/2)
             Real64 const RHODFS_SLAT, // solar reflectance downward-facing slat surfaces (concave?)
             Real64 const RHOUFS_SLAT, // solar reflectance upward-facing slat surfaces (convex?)
             Real64 const TAU_SLAT,    // solar transmittance of slat
             Real64 &RHO_BD,           // returned: solar beam-to-diffuse reflectance the venetian blind (front side)
             Real64 &TAU_BD            // returned: solar beam-to-diffuse transmittance of the venetian blind (front side)
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  Calculates the venetian blind layer effective solar transmittance and reflectance.
    // METHODOLOGY EMPLOYED:
    //  Four surface Flat-Plate Model with slat transmittance

    // SUBROUTINE ARGUMENT DEFINITIONS:
    //    must be > 0
    //   must be > 0
    //   ltyVBHOR: +=above horizontal
    //   ltyVBVER: +=clockwise when viewed from above
    //    the plane of the same slat de may be greater than the slat width, w
    //   ltyVBHOR: + = front-side slat tip below horizontal
    //   ltyVBVER: + = front-side slat tip is counter-
    //                 clockwise from normal (viewed from above)
    //    Note: all solar slat properties - incident-to-diffuse

    Real64 AF; // lengths of diagonal strings used in the four-surface model
    Real64 CD;
    Real64 F13; // Shape factors
    Real64 F14;
    Real64 F23;
    Real64 F24;
    Real64 F34;
    Real64 F43;
    Real64 Z3; // diffuse source terms from surfaces 3 and 4 due to incident beam radiation
    Real64 Z4;
    Real64 J3; // radiosity, surface i
    Real64 J4;
    Real64 B3; // temporaries
    Real64 B4;
    Real64 C3;
    Real64 C4;

    Real64 const W_cos_PHI_2(pow_2(W * std::cos(PHI)));
    Real64 const W_sin_PHI(W * std::sin(PHI));
    AF = std::sqrt(W_cos_PHI_2 + pow_2(S - W_sin_PHI));
    CD = std::sqrt(W_cos_PHI_2 + pow_2(S + W_sin_PHI));
    //  CHECK TO SEE WHICH SIDE OF SLAT IS SUNLIT
    if (PHI + OMEGA >= 0.0) { // SUN SHINES ON TOP OF SLAT

        Z3 = TAU_SLAT * S / DE;
        Z4 = RHOUFS_SLAT * S / DE;
        //  PRINT *, PHI, OMEGA, DE, 'TOPLIT'

    } else { // SUN SHINES ON BOTTOM OF SLAT
        Z3 = RHODFS_SLAT * S / DE;
        Z4 = TAU_SLAT * S / DE;
        //      PRINT *, PHI, OMEGA, DE, 'BOTLIT'
    }
    //  CHECK TO SEE IF VENETIAN BLIND IS CLOSED
    if (std::abs(PHI - Constant::PiOvr2) < state.dataWindowEquivalentLayer->SMALL_ERROR) { // VENETIAN BLIND IS CLOSED

        // CHECK TO SEE IF THERE ARE GAPS IN BETWEEN SLATS WHEN THE BLIND IS CLOSED
        if (W < S) { // YES, THERE ARE GAPS IN BETWEEN SLATS
            RHO_BD = (W / S) * RHOUFS_SLAT;
            TAU_BD = (W / S) * TAU_SLAT;
        } else { // NO, THERE ARE NO GAPS IN BETWEEN SLATS
            RHO_BD = RHOUFS_SLAT;
            TAU_BD = TAU_SLAT;
        } // END OF CHECK FOR GAPS IN BETWEEN SLATS

    } else { // VENETIAN BLIND IS OPENED

        F13 = (S + W - CD) / (2.0 * S);
        F14 = (S + W - AF) / (2.0 * S);
        F23 = (S + W - AF) / (2.0 * S);
        F24 = (S + W - CD) / (2.0 * S);
        F34 = (CD + AF - 2.0 * S) / (2.0 * W);
        F43 = (CD + AF - 2.0 * S) / (2.0 * W);

        C3 = 1.0 / (1.0 - TAU_SLAT * F43);
        B3 = (RHODFS_SLAT * F34) / (1.0 - TAU_SLAT * F43);
        C4 = 1.0 / (1.0 - TAU_SLAT * F34);
        B4 = (RHOUFS_SLAT * F43) / (1.0 - TAU_SLAT * F34);
        J3 = (C3 * Z3 + B3 * C4 * Z4) / (1.0 - B3 * B4);
        J4 = (C4 * Z4 + B4 * C3 * Z3) / (1.0 - B3 * B4);

        RHO_BD = F13 * J3 + F14 * J4;
        TAU_BD = F23 * J3 + F24 * J4;

    } // END OF CHECK FOR CLOSED BLIND
}

void VB_SOL6(EnergyPlusData const &state,
             Real64 const S,           // slat spacing (any length units; same units as W)
             Real64 const W,           // slat tip-to-tip width (any length units; same units as S)
             Real64 const OMEGA,       // incident beam profile angle (radians)
             Real64 const DE,          // distance from front tip of any slat to shadow (caused by the adjacent slat) on
             Real64 const PHI,         // slat angle, radians (-PI/2 <= PHI <= PI/2)
             Real64 const RHODFS_SLAT, // solar reflectance downward-facing slat surfaces (concave)
             Real64 const RHOUFS_SLAT, // solar reflectance upward-facing slat surfaces (convex)
             Real64 const TAU_SLAT,    // solar transmittance of slat
             Real64 &RHO_BD,           // returned: solar beam-to-diffuse reflectance the venetian blind (front side)
             Real64 &TAU_BD            // returned: solar beam-to-diffuse transmittance of the venetian blind (front side)
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  Calculates the venetian blind layer effective solar transmittance and reflectance.
    // METHODOLOGY EMPLOYED:
    //  six surface flat-slat model with slat transmittance. If you want the back
    //  side reflectance call the routine a second time with the same input data
    //  except negative the slat angle, PHI_DEG

    // SUBROUTINE ARGUMENT DEFINITIONS:
    //    must be > 0
    //   must be > 0
    //   ltyVBHOR: +=above horizontal
    //   ltyVBVER: +=clockwise when viewed from above
    //    the plane of the same slat DE may be greater than the slat width, w
    //   ltyVBHOR: + = front-side slat tip below horizontal
    //   ltyVBVER: + = front-side slat tip is counter-
    //                 clockwise from normal (viewed from above)
    //    Note: all solar slat properties - incident-to-diffuse
    int constexpr N(4);

    Real64 AB; // lengths of slat segments and diagonal strings
    Real64 AE;
    Real64 AF;
    Real64 BC;
    Real64 BD;
    Real64 BF;
    Real64 CD;
    Real64 CE;
    Real64 EF;
    //  used in the six-surface model
    Real64 F13; // shape factors
    Real64 F14;
    Real64 F23;
    Real64 F24;
    Real64 F34;
    Real64 F36;
    Real64 F15;
    Real64 F16;
    Real64 F43;
    Real64 F45;
    Real64 F54;
    Real64 F56;
    Real64 F63;
    Real64 F65;
    Real64 F25;
    Real64 F26;
    Real64 Z3; // diffuse source terms from surfaces 3 and 4 due to incident beam radiation
    Real64 Z4;
    Real64 J3; // radiosity, surface i
    Real64 J4;
    Real64 J5;
    Real64 J6;
    Array2D<Real64> A(N + 2, N); // coefficients of the radiosity equations matrix
    Array1D<Real64> XSOL(N);     // solution vector (obtained after solving the radiosity equations matrix)

    //  CHECK TO SEE WHICH SIDE OF SLAT IS SUNLIT
    if ((PHI + OMEGA) >= 0.0) { // SUN SHINES ON TOP OF SLAT
        Z3 = TAU_SLAT * S / DE;
        Z4 = RHOUFS_SLAT * S / DE;
        //      PRINT *, PHI, OMEGA, DE, 'TOPLIT'

    } else { // SUN SHINES ON BOTTOM OF SLAT
        Z3 = RHODFS_SLAT * S / DE;
        Z4 = TAU_SLAT * S / DE;
        //      PRINT *, PHI, OMEGA, DE, 'BOTLIT'
    }

    //  CHECK TO SEE IF VENETIAN BLIND IS CLOSED
    if (std::abs(PHI - Constant::PiOvr2) < state.dataWindowEquivalentLayer->SMALL_ERROR) { // VENETIAN BLIND IS CLOSED

        // CHECK TO SEE IF THERE ARE GAPS IN BETWEEN SLATS WHEN THE BLIND IS CLOSED
        if (W < S) { // YES, THERE ARE GAPS IN BETWEEN SLATS
            RHO_BD = (W / S) * RHOUFS_SLAT;
            TAU_BD = (W / S) * TAU_SLAT;
        } else { // NO, THERE ARE NO GAPS IN BETWEEN SLATS
            RHO_BD = RHOUFS_SLAT;
            TAU_BD = TAU_SLAT;
        } // END OF CHECK FOR GAPS IN BETWEEN SLATS

    } else { // VENETIAN BLIND IS OPENED
        AB = DE;
        Real64 const cos_PHI(std::cos(PHI));
        Real64 const sin_PHI(std::sin(PHI));
        Real64 const W_cos_PHI_2(pow_2(W * cos_PHI));
        AF = std::sqrt(W_cos_PHI_2 + pow_2(S - W * sin_PHI));
        BC = W - AB;
        EF = BC;
        Real64 const DE_cos_PHI_2(pow_2(DE * cos_PHI));
        Real64 const EF_cos_PHI_2(pow_2(EF * cos_PHI));
        BD = std::sqrt(DE_cos_PHI_2 + pow_2(S + DE * sin_PHI));
        BF = std::sqrt(EF_cos_PHI_2 + pow_2(S - EF * sin_PHI));
        CD = std::sqrt(W_cos_PHI_2 + pow_2(S + W * sin_PHI));
        CE = std::sqrt(EF_cos_PHI_2 + pow_2(S + EF * sin_PHI));
        AE = std::sqrt(DE_cos_PHI_2 + pow_2(S - DE * sin_PHI));

        F13 = (S + AB - BD) / (2.0 * S);
        F14 = (S + DE - AE) / (2.0 * S);
        F15 = (W + BD - (AB + CD)) / (2.0 * S);
        F16 = (W + AE - (AF + DE)) / (2.0 * S);
        F23 = (W + BF - (BC + AF)) / (2.0 * S);
        F24 = (W + CE - (CD + EF)) / (2.0 * S);
        F25 = (S + BC - BF) / (2.0 * S);
        F26 = (S + EF - CE) / (2.0 * S);
        F34 = (AE + BD - 2.0 * S) / (2.0 * AB);
        F36 = (AF + S - (AE + BF)) / (2.0 * AB);
        F43 = (AE + BD - 2.0 * S) / (2.0 * DE);
        F45 = (CD + S - (BD + CE)) / (2.0 * DE);
        F54 = (CD + S - (BD + CE)) / (2.0 * BC);
        F56 = (CE + BF - 2.0 * S) / (2.0 * BC);
        F63 = (AF + S - (AE + BF)) / (2.0 * EF);
        F65 = (BF + CE - 2.0 * S) / (2.0 * EF);

        // POPULATE THE COEFFICIENTS OF THE RADIOSITY MATRIX

        A(1, 1) = 1.0 - TAU_SLAT * F43;
        A(2, 1) = -RHODFS_SLAT * F34;
        A(3, 1) = -TAU_SLAT * F45;
        A(4, 1) = -RHODFS_SLAT * F36;
        A(5, 1) = Z3;
        A(1, 2) = -RHOUFS_SLAT * F43;
        A(2, 2) = 1.0 - TAU_SLAT * F34;
        A(3, 2) = -RHOUFS_SLAT * F45;
        A(4, 2) = -TAU_SLAT * F36;
        A(5, 2) = Z4;
        A(1, 3) = -TAU_SLAT * F63;
        A(2, 3) = -RHODFS_SLAT * F54;
        A(3, 3) = 1.0 - TAU_SLAT * F65;
        A(4, 3) = -RHODFS_SLAT * F56;
        A(5, 3) = 0.0;
        A(1, 4) = -RHOUFS_SLAT * F63;
        A(2, 4) = -TAU_SLAT * F54;
        A(3, 4) = -RHOUFS_SLAT * F65;
        A(4, 4) = 1.0 - TAU_SLAT * F56;
        A(5, 4) = 0.0;

        SOLMATS(N, A, XSOL);

        J3 = XSOL(1);
        J4 = XSOL(2);
        J5 = XSOL(3);
        J6 = XSOL(4);

        RHO_BD = F13 * J3 + F14 * J4 + F15 * J5 + F16 * J6;
        TAU_BD = F23 * J3 + F24 * J4 + F25 * J5 + F26 * J6;
    } // END OF CHECK FOR CLOSED BLIND
}

void SOLMATS(int const N,          // # of active rows in A
             Array2S<Real64> A,    // matrix, minimum required dimensions: A( N, N+2)
             Array1D<Real64> &XSOL // returned: solution vector, min req dimension: XSOL( N)
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  Matrix solver.
    // METHODOLOGY EMPLOYED:
    //  Solves matrix by the elimination method supplemented by a search for the
    //  largest pivotal element at each stage

    int NM1 = N - 1;
    int NP1 = N + 1;
    int NP2 = N + 2;

    for (int I = 1; I <= N; ++I) {
        A(NP2, I) = 0.0;
        // DO 1 J=1,NP1    ! TODO ?
    }

    for (int I = 1; I <= N; ++I) {
        for (int J = 1; J <= NP1; ++J) {
            A(NP2, I) += A(J, I);
        }
    }

    for (int L = 1; L <= N - 1; ++L) {
        Real64 CMAX = A(L, L);
        int LP = L + 1;
        int NOS = L;

        for (int I = LP; I <= N; ++I) {
            if (std::abs(CMAX) < std::abs(A(L, I))) {
                CMAX = A(L, I);
                NOS = I;
            }
        }

        // Swap rows
        if (NOS != L) {
            for (int J = 1; J <= NP2; ++J) {
                Real64 TEMP = A(J, L);
                A(J, L) = A(J, NOS);
                A(J, NOS) = TEMP;
            }
        }

        for (int I = LP; I <= N; ++I) {
            Real64 C = 0.0;
            Real64 Y = -A(L, I) / A(L, L);
            for (int J = L; J <= NP2; ++J) {
                A(J, I) += Y * A(J, L);
            }
            for (int J = L; J <= NP1; ++J) {
                C += A(J, I);
            }
        }
    }

    // back-substitute
    XSOL(N) = A(NP1, N) / A(N, N);
    for (int I = 1; I <= NM1; ++I) {
        int NI = N - I;
        Real64 D = 0.0;
        for (int J = 1; J <= I; ++J) {
            int NJ = N + 1 - J;
            D += A(NJ, NI) * XSOL(NJ);
        }
        XSOL(NI) = (A(NP1, NI) - D) / A(NI, NI);
    }
}

void ASHWAT_ThermalCalc(EnergyPlusData &state,
                        CFSTY &FS,        // fenestration system
                        Real64 const TIN, // indoor / outdoor air temperature, K
                        Real64 const TOUT,
                        Real64 const HCIN, // indoor / outdoor convective heat transfer
                        Real64 const HCOUT,
                        Real64 const TRMOUT,
                        Real64 const TRMIN,           // indoor / outdoor mean radiant temp, K
                        Array1S<Real64> const SOURCE, // absorbed solar by layer, W/m2
                        Real64 const TOL,             // convergence tolerance, usually
                        Array1D<Real64> &QOCF,        // returned: heat flux to layer i from gaps i-1 and i
                        Real64 &QOCFRoom,             // returned: open channel heat gain to room, W/m2
                        Array1D<Real64> &T,           // returned: layer temperatures, 1=outside-most layer, K
                        Array1D<Real64> &Q,           // returned: heat flux at ith gap (betw layers i and i+1), W/m2
                        Array1D<Real64> &JF,          // returned: front (outside facing) radiosity of surfaces, W/m2
                        Array1D<Real64> &JB,          // returned: back (inside facing) radiosity, W/m2
                        Array1D<Real64> &HC           // returned: gap convective heat transfer coefficient, W/m2K
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT (University of Waterloo, Mechanical Engineering)
    //                      Chip Barnaby (WrightSoft)
    //       DATE WRITTEN   LATEST MODIFICATIONS, February 2008
    //       MODIFIED       Bereket Nigusse, June 2013
    //                      added standard 155099 inside convection
    //                      coefficient calculation for U-Factor

    // PURPOSE OF THIS SUBROUTINE:
    //     Subroutine to calculate the glazing temperatures of the
    //     various elements of a window/shade array while solving an energy
    //     balance which accounts for absorbed solar radiation, indoor-
    //     outdoor temperature difference, any combination of hemispherical
    //     IR optical properties of the various glazings/shading layers.
    //     Mean radiant temperatures can differ from air temperature on
    //     both the indoor and outdoor sides.
    //     It is also possible to allow air-flow between the two layers
    //     adjacent to the indoor side and/or the two layers adjacent the
    //     outdoor side. U-factor and SHGC calculations are also included (optional)

    // METHODOLOGY EMPLOYED:
    // Uses the net radiation method developed for ASHWAT fenestration
    // model by John Wright, the University of WaterLoo

    // REFERENCES:
    //  ASHRAE RP-1311

    // Argument array dimensioning
    EP_SIZE_CHECK(QOCF, FS.NL);
    EP_SIZE_CHECK(T, FS.NL);
    EP_SIZE_CHECK(JF, FS.NL + 1);
    EP_SIZE_CHECK(JB, FS.NL + 1);
    EP_SIZE_CHECK(HC, FS.NL + 1);

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:
    //   FS.NL determines # of layers modelled
    //   coefficient, W/m2K
    //   = outside direct + outside diffuse + inside diffuse
    //   0.001 (good) or 0.0001 (tight)
    //   due to open channel flow, W/m2
    //   + = heat flow indoor to outdoor
    //   JF( NL+1) = room radiosity
    //   JB[ 0] = outside environment radiosity
    //   0=outside, 1=betw layer 1-2, ..., NL=inside

    // FUNCTION PARAMETER DEFINITIONS:
    constexpr int MaxIter(100); // maximum number of iterations allowed
    static constexpr std::string_view RoutineName("ASHWAT_ThermalCalc: ");

    Real64 ALPHA;
    Real64 HCOCFout;
    Array2D<Real64> A(3 * FS.NL + 4, 3 * FS.NL + 2);
    Array1D<Real64> XSOL(3 * FS.NL + 2);
    Real64 MAXERR;
    Array1D<Real64> TNEW(FS.NL);        // latest estimate of layer temperatures, K
    Array1D<Real64> EB({0, FS.NL + 1}); // black emissive power by layer, W/m2
                                        //   EB( 0) = outdoor environment, EB( NL+1) = indoor environment
    Array1D<Real64> HHAT({0, FS.NL});   // convective heat transfer coefficient (W/m2.K4)
                                        //   based on EB, NOT temperature difference
    Real64 RHOF_ROOM;                   // effective longwave room-side properties
    Real64 TAU_ROOM;
    Real64 EPSF_ROOM;
    Real64 RHOB_OUT; // effective longwave outdoor environment properties
    Real64 TAU_OUT;
    Real64 EPSB_OUT;
    Array1D<Real64> QNET(FS.NL); // checksum - net heat flux to a layer - should be zero - not needed
    int ADIM;                    // dimension of the A matrix
    int CONVRG;
    int NL;
    int I;
    int J;
    int ITRY;
    Array1D_int ISDL({0, FS.NL + 1}); // Flag to mark diathermanous layers, 0=opaque
    Array1D<Real64> QOCF_F(FS.NL);    // heat flux to outdoor-facing surface of layer i, from gap i-1,
                                      //   due to open channel flow, W/m2
    Array1D<Real64> QOCF_B(FS.NL);    // heat flux to indoor-facing surface of layer i, from gap i,
                                      //   due to open channel flow, W/m2
    Array1D<Real64> HR({0, FS.NL});   // Radiant heat transfer coefficient [W/m2K]
    Array1D<Real64> HJR(FS.NL);       // radiative and convective jump heat transfer coefficients
    Array1D<Real64> HJC(FS.NL);
    Array1D<Real64> RHOF({0, FS.NL + 1}); // longwave reflectance, front    !  these variables help simplify
    Array1D<Real64> RHOB({0, FS.NL + 1}); // longwave reflectance, back     !  the code because it is useful to
    Array1D<Real64> EPSF({0, FS.NL + 1}); // longwave emissivity,  front    !  increase the scope of the arrays
    Array1D<Real64> EPSB({0, FS.NL + 1}); // longwave emissivity,  back     !  to include indoor and outdoor
    Array1D<Real64> TAU({0, FS.NL + 1});  // longwave transmittance         !  nodes - more general
    Array2D<Real64> HC2D(6, 6);           // convective heat transfer coefficients between layers i and j
    Array2D<Real64> HR2D(6, 6);           // radiant heat transfer coefficients between layers i and j
    Array1D<Real64> HCIout(6);            // convective and radiant heat transfer coefficients between
    Array1D<Real64> HRIout(6);
    // layer i and outdoor air or mean radiant temperature, resp.
    Array1D<Real64> HCIin(6); // convective and radiant heat transfer coefficients between
    Array1D<Real64> HRIin(6);
    // layer i and indoor air or mean radiant temperature, resp.
    //  Indoor side convection coefficients - used for Open Channel Flow on indoor side
    Real64 HFS;                          // nominal height of fen system (assumed 1 m)
    Real64 TOC_EFF;                      // effective thickness of open channel, m
    Real64 ConvF;                        // convection factor: accounts for enhanced convection
                                         //   for e.g. VB adjacent to open channel
    Real64 HC_GA;                        // convection - glass to air
    Real64 HC_SA;                        // convection - shade (both sides) to air
    Real64 HC_GS;                        // convection - glass to shade (one side)
    Array1D<Real64> SOURCEdv(FS.NL + 1); // indices of merit
    Real64 QGAIN;                        // total gain to conditioned space [[W/m2]

    NL = FS.NL; // working copy
    if (NL < 1) {
        return;
    }

    HCOCFout = HCOUT; // outdoor side

    HHAT = 0.0;
    HC = 0.0;
    HR = 0.0;
    T = 0.0;
    TNEW = 0.0;
    EB = 0.0;
    JF = 0.0;
    JB = 0.0;
    Q = 0.0;
    QOCF_F = 0.0;
    QOCF_B = 0.0;
    QOCF = 0.0;
    QOCFRoom = 0.0;
    QNET = 0.0;
    QGAIN = 0.0;
    TAU = 0.0;
    RHOF = 0.0;
    RHOB = 0.0;
    EPSF = 0.0;
    EPSB = 0.0;
    HC_GA = 0.0;
    HC_SA = 0.0;
    HC_GS = 0.0;

    ITRY = 0;

    EB(0) = Constant::StefanBoltzmann * pow_4(TOUT);
    EB(NL + 1) = Constant::StefanBoltzmann * pow_4(TIN);

    ADIM = 3 * NL + 2; // DIMENSION OF A-MATRIX

    // organize longwave radiant properties - book-keeping

    TAU_ROOM = 0.0;                         // must always be zero
    RHOF_ROOM = 0.0;                        // almost always zero
    EPSF_ROOM = 1.0 - TAU_ROOM - RHOF_ROOM; // almost always unity
    RHOF(NL + 1) = RHOF_ROOM;
    EPSF(NL + 1) = EPSF_ROOM;
    TAU(NL + 1) = TAU_ROOM;

    for (I = 1; I <= NL; ++I) {
        EPSF(I) = FS.L(I).LWP_EL.EPSLF;
        EPSB(I) = FS.L(I).LWP_EL.EPSLB;
        TAU(I) = FS.L(I).LWP_EL.TAUL;
        RHOF(I) = 1.0 - FS.L(I).LWP_EL.EPSLF - FS.L(I).LWP_EL.TAUL;
        RHOB(I) = 1.0 - FS.L(I).LWP_EL.EPSLB - FS.L(I).LWP_EL.TAUL;
    }

    TAU_OUT = 0.0;                       // must always be zero
    RHOB_OUT = 0.0;                      // DON'T CHANGE
    EPSB_OUT = 1.0 - TAU_OUT - RHOB_OUT; // should always be unity
    TAU(0) = TAU_OUT;
    EPSB(0) = EPSB_OUT;
    RHOB(0) = RHOB_OUT;

    // Later could add RHOF_ROOM to the parameter list
    // Relaxation needed to keep solver stable if OCF is present

    ALPHA = 1.0;
    if (NL >= 2) {
        if (FS.G(NL - 1).GTYPE == state.dataWindowEquivalentLayer->gtyOPENin) {
            ALPHA = 0.5;
        }
        if (FS.G(1).GTYPE == state.dataWindowEquivalentLayer->gtyOPENout) {
            ALPHA = 0.10;
        }
    }

    //   FIRST ESTIMATE OF GLAZING TEMPERATURES AND BLACK EMISSIVE POWERS
    for (I = 1; I <= NL; ++I) {
        T(I) = TOUT + double(I) / double(NL + 1) * (TIN - TOUT);
        EB(I) = Constant::StefanBoltzmann * pow_4(T(I));
    }

    CONVRG = 0;

    //  Start the solver
    //  ITERATION RE-ENTRY

    Real64 const TIN_2(pow_2(TIN));
    Real64 const TOUT_2(pow_2(TOUT));
    Real64 const TRMOUT_4(pow_4(TRMOUT));
    Real64 const TRMIN_4(pow_4(TRMIN));

    for (ITRY = 1; ITRY <= MaxIter; ++ITRY) {

        //  CALCULATE GAS LAYER CONVECTIVE HEAT TRANSFER COEFFICIENTS

        // start by assuming no open channel flow on indoor side

        HC[NL] = HCIN; //  default - HC[NL] supplied by calling routine
                       //  use this for HBX
                       // or
                       // trigger calculation of HC[NL] using ASHRAE correlation
                       //  HC[NL] = HIC_ASHRAE(1.0d0, T(NL), TIN)  ! h - flat plate
                       // Add by BAN June 2013 for standard ratings U-value and SHGC calc only
                       // if (present(HCInFlag)) {
                       //     if (HCInFlag) HC[NL] = HCInWindowStandardRatings(Height, T(NL), TIN);
                       // }
        HC[0] = HCOUT; // HC[0] supplied by calling routine as HCOUT

        // Check for open channels -  only possible with at least two layers
        if (NL >= 2) {

            int hin_scheme = 3; //  different schemes for calculating convection
                                //  coefficients glass-to-air and shade-to-air
                                //  if open channel air flow is allowed
                                //  see the corresponding subroutines for detail
                                //  = 1 gives dependence of height, spacing, delta-T
                                //  = 2 gives dependence of spacing, delta-T but
                                //    returns unrealistic values for large spacing
                                //  = 3 glass-shade spacing dependence only on HCIN
                                //  = negative, applies HCIN without adjusting for
                                //    temperature, height, spacing, slat angle
                                //  Recommended -> hin_scheme=3 for use with HBX,
                                //              simplicity, right trends wrt spacing

            for (I = 1; I <= NL - 1; ++I) { // Scan gaps between layers

                // DEAL WITH INDOOR OPEN CHANNEL FLOW HERE
                if ((I == NL - 1) && (FS.G(I).GTYPE == state.dataWindowEquivalentLayer->gtyOPENin)) {

                    // TOC_EFF = FS%G( I)%TAS_EFF / 1000.    ! effective thickness of OC gap, m
                    TOC_EFF = FS.G(I).TAS_EFF; // effective thickness of OC gap, m Modified by BAN May 9, 2013
                    HFS = 1.0;                 // nominal height of system (m)

                    // convection - glass to air
                    GLtoAMB(state, TOC_EFF, HFS, T(NL - 1), TIN, HCIN, HC_GA, hin_scheme);
                    // CALL GLtoAMB( 1.0, HFS, T( NL-1), TIN, HCIN, HC_GA, hin_scheme)
                    //   ^ VERY WIDE GAP

                    // convection - shade (both sides) to air
                    ConvF = ConvectionFactor(FS.L(I + 1));
                    HC_SA = ConvF * SLtoAMB(state, TOC_EFF, HFS, T(NL), TIN, HCIN, hin_scheme);
                    // HC_SA = ConvF * SLtoAMB( 1.0, HFS, T(NL), TIN, HCIN, hin_scheme)
                    //  ^ VERY WIDE GAP

                    // convection - glass to shade (one side)
                    SLtoGL(state, TOC_EFF, T(NL), T(NL - 1), HC_GS, 1);
                    // CALL  SLtoGL( 1.0, T(NL), T(NL-1), HC_GS, 2)   !  REMOVE LATER
                    //  ^ VERY WIDE GAP, should return near zero
                    //  Don't use hin_scheme as last parameter - set manually
                    //  1 = Conduction, 2 = slight Ra penalty
                    //  Set negative for default HC_GS=0
                    //  Recommended:  2
                    HC[NL - 1] = HC_GS;
                    HC[NL] = HCIN * ConvF;
                    QOCF_B(NL - 1) = (TIN - T(NL - 1)) * HC_GA;
                    QOCF_F(NL) = (TIN - T(NL)) * (HC_SA - HC[NL]);
                    QOCFRoom = -QOCF_B(NL - 1) - QOCF_F(NL);
                    // end of gap open to indoor side

                } else if ((I == 1) && (FS.G(I).GTYPE == state.dataWindowEquivalentLayer->gtyOPENout)) {
                    // outdoor open channel
                    QOCF_B(1) = (TOUT - T(1)) * HCOCFout;
                    QOCF_F(2) = (TOUT - T(2)) * HCOCFout;
                    HC[1] = 0.0;
                    HC[0] = HCOCFout;
                } else {
                    // normal gap
                    HC[I] = HConvGap(FS.G(I), T(I), T(I + 1));
                }
            } //  end scan through gaps

            // total OCF gain to each layer
            QOCF = QOCF_F + QOCF_B;

        } //  end IF (NL .GE. 2)

        //  CONVERT TEMPERATURE POTENTIAL CONVECTIVE COEFFICIENTS to
        //  BLACK EMISSIVE POWER POTENTIAL CONVECTIVE COEFFICIENTS

        HHAT(0) = HC[0] * (1.0 / Constant::StefanBoltzmann) / ((TOUT_2 + pow_2(T(1))) * (TOUT + T(1)));

        Real64 T_I_2(pow_2(T(1))), T_IP_2;
        for (I = 1; I <= NL - 1; ++I) { // Scan the cavities
            T_IP_2 = pow_2(T(I + 1));
            HHAT(I) = HC[I] * (1.0 / Constant::StefanBoltzmann) / ((T_I_2 + T_IP_2) * (T(I) + T(I + 1)));
            T_I_2 = T_IP_2;
        }

        HHAT(NL) = HC[NL] * (1.0 / Constant::StefanBoltzmann) / ((pow_2(T(NL)) + TIN_2) * (T(NL) + TIN));

        //  SET UP MATRIX
        XSOL = 0.0;
        A = 0.0;

        int L = 1;
        A(1, L) = 1.0;
        A(2, L) = -1.0 * RHOB(0); //  -1.0 * RHOB_OUT
        A(ADIM + 1, L) = EPSB_OUT * Constant::StefanBoltzmann * TRMOUT_4;

        for (I = 1; I <= NL; ++I) {
            L = 3 * I - 1;
            A(3 * I - 2, L) = RHOF(I);
            A(3 * I - 1, L) = -1.0;
            A(3 * I, L) = EPSF(I);    //  LWP( I)%EPSLF
            A(3 * I + 2, L) = TAU(I); //  LWP( I)%TAUL
            A(ADIM + 1, L) = 0.0;

            L = 3 * I;
            if (NL == 1) {
                A(1, L) = 1.0; // Single layer
                A(2, L) = -1.0;
                A(3, L) = -1.0 * (HHAT(0) + HHAT(1));
                A(4, L) = -1.0;
                A(5, L) = 1.0;
                A(ADIM + 1, L) = -1.0 * (HHAT(0) * EB(0) + HHAT(1) * EB(2) + SOURCE(1) + QOCF(1));
            } else if (I == 1) {
                A(1, L) = 1.0; //  Outdoor layer
                A(2, L) = -1.0;
                A(3, L) = -1.0 * (HHAT(0) + HHAT(1));
                A(4, L) = -1.0;
                A(5, L) = 1.0;
                A(6, L) = HHAT(1);
                A(ADIM + 1, L) = -1.0 * (HHAT(0) * EB(0) + SOURCE(1) + QOCF(1));
            } else if (I == NL) {
                A(3 * NL - 3, L) = HHAT(NL - 1); // Indoor layer
                A(3 * NL - 2, L) = 1.0;
                A(3 * NL - 1, L) = -1.0;
                A(3 * NL, L) = -1.0 * (HHAT(NL) + HHAT(NL - 1));
                A(3 * NL + 1, L) = -1.0;
                A(3 * NL + 2, L) = 1.0;
                A(ADIM + 1, L) = -1.0 * (HHAT(NL) * EB(NL + 1) + SOURCE(NL) + QOCF(NL));
            } else {
                A(3 * I - 3, L) = HHAT(I - 1);
                A(3 * I - 2, L) = 1.0;
                A(3 * I - 1, L) = -1.0;
                A(3 * I, L) = -1.0 * (HHAT(I) + HHAT(I - 1));
                A(3 * I + 1, L) = -1.0;
                A(3 * I + 2, L) = 1.0;
                A(3 * I + 3, L) = HHAT(I);
                A(ADIM + 1, L) = -1.0 * (SOURCE(I) + QOCF(I));
            }
            L = 3 * I + 1;
            A(3 * I - 2, L) = TAU(I); //   LWP( I)%TAUL
            A(3 * I, L) = EPSB(I);    //   LWP( I)%EPSLB
            A(3 * I + 1, L) = -1.0;
            A(3 * I + 2, L) = RHOB(I);
            A(ADIM + 1, L) = 0.0;
        }

        L = 3 * NL + 2;
        A(3 * NL + 1, L) = -1.0 * RHOF(NL + 1); //   - 1.0 * RHOF_ROOM
        A(3 * NL + 2, L) = 1.0;
        A(ADIM + 1, L) = EPSF_ROOM * Constant::StefanBoltzmann * TRMIN_4;

        //  SOLVE MATRIX
        //  Call SOLMATS for single precision matrix solution
        SOLMATS(ADIM, A, XSOL);

        //  UNPACK SOLUTION VECTOR AND RECORD LARGEST TEMPERATURE CHANGE
        JB[0] = XSOL(1);

        MAXERR = 0.0;
        for (I = 1; I <= NL; ++I) {
            J = 3 * I - 1;
            JF(I) = XSOL(J);
            ++J;
            EB(I) = max(1.0, XSOL(J)); // prevent impossible temps
            TNEW(I) = root_4(EB(I) / Constant::StefanBoltzmann);
            ++J;
            JB[I] = XSOL(J);
            MAXERR = max(MAXERR, std::abs(TNEW(I) - T(I)) / TNEW(I));
        }

        JF(NL + 1) = XSOL(ADIM);

        //  CALCULATE HEAT FLUX AT EACH GAP, Q
        for (I = 0; I <= NL; ++I) { // Loop gaps (including inside and outside
            Q(I) = JF(I + 1) - JB[I] + HHAT(I) * (EB(I + 1) - EB(I));
        }

        //  A CHECK - NET HEAT FLUX INTO ANY LAYER, AT STEADY-STATE,
        //  SHOULD BE ZERO
        for (I = 1; I <= NL; ++I) {
            QNET(I) = SOURCE(I) + QOCF(I) + Q(I) - Q(I - 1);
        }

        //  UPDATE GLAZING TEMPERATURES AND BLACK EMISSIVE POWERS
        for (I = 1; I <= NL; ++I) {
            T(I) += ALPHA * (TNEW(I) - T(I));
            EB(I) = Constant::StefanBoltzmann * pow_4(T(I));
        }

        //  CHECK FOR CONVERGENCE
        if (CONVRG > 0) {
            break;
        }
        if (MAXERR < TOL) {
            ++CONVRG;
        }

    } // main iteration

    if (CONVRG == 0) {

        if (FS.WEQLSolverErrorIndex < 1) {
            ++FS.WEQLSolverErrorIndex;
            ShowSevereError(state, format("CONSTRUCTION:WINDOWEQUIVALENTLAYER = \"{}\"", FS.Name));
            ShowContinueError(state, format("{}Net radiation analysis did not converge", RoutineName));
            ShowContinueError(state, format("...Maximum error is = {:.6T}", MAXERR));
            ShowContinueError(state, format("...Convergence tolerance is = {:.6T}", TOL));
            ShowContinueErrorTimeStamp(state, "");
        } else {
            ShowRecurringWarningErrorAtEnd(state,
                                           "CONSTRUCTION:WINDOWEQUIVALENTLAYER = \"" + FS.Name + "\"; " + std::string{RoutineName} +
                                               "Net radiation analysis did not converge error continues.",
                                           FS.WEQLSolverErrorIndex);
        }
    }

    //  NOTE:  HC_SA, HC_GA and HC_SG are only available if there is
    //         an open channel on the indoor side and the calculation of
    //         these coefficients was triggered earlier
    QGAIN = SOURCE(NL + 1) + HC[NL] * (T(NL) - TIN) + JB[NL] - JF(NL + 1);
    // Modified by BAN May 3, 2013 to avoid zero layer index
    if (NL >= 2) {
        if (FS.G(NL - 1).GTYPE == state.dataWindowEquivalentLayer->gtyOPENin) {
            QGAIN = SOURCE(NL + 1) + (HC_SA / 2.0) * (T(NL) - TIN) + JB[NL] - JF(NL + 1);
            QGAIN += HC_GA * (T(NL - 1) - TIN) + (HC_SA / 2.0) * (T(NL) - TIN);
        }
    }
}

bool ASHWAT_ThermalRatings(EnergyPlusData &state,
                           CFSTY const &FS,  // fenestration system
                           Real64 const TIN, // indoor / outdoor air temperature, K
                           Real64 const TOUT,
                           Real64 const HCIN, // indoor / outdoor convective heat transfer
                           Real64 const HCOUT,
                           Real64 const TRMOUT,
                           Real64 const TRMIN,           // indoor / outdoor mean radiant temp, K
                           Real64 const ISOL,            // total incident solar, W/m2 (values used for SOURCE derivation)
                           Array1S<Real64> const SOURCE, // absorbed solar by layer, W/m2
                           Real64 const TOL,             // convergence tolerance, usually
                           Array1D<Real64> &QOCF,        // returned: heat flux to layer i from gaps i-1 and i
                           Real64 &QOCFRoom,             // returned: open channel heat gain to room, W/m2
                           Array1D<Real64> &T,           // returned: layer temperatures, 1=outside-most layer, K
                           Array1D<Real64> &Q,           // returned: heat flux at ith gap (betw layers i and i+1), W/m2
                           Array1D<Real64> &JF,          // returned: front (outside facing) radiosity of surfaces, W/m2
                           Array1D<Real64> &JB,          // returned: back (inside facing) radiosity, W/m2
                           Array1D<Real64> &HC,          // returned: gap convective heat transfer coefficient, W/m2K
                           Real64 &UCG,                  // returned: center-glass U-factor, W/m2-K
                           Real64 &SHGC,                 // returned: center-glass SHGC (Solar Heat Gain Coefficient)
                           bool const HCInFlag           // If true uses ISO Std 150099 routine for HCIn calc
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT (University of Waterloo, Mechanical Engineering)
    //                      Chip Barnaby (WrightSoft)
    //       DATE WRITTEN   LATEST MODIFICATIONS, February 2008
    //       MODIFIED       Bereket Nigusse, June 2013
    //                      added standard 155099 inside convection
    //                      coefficient calculation for U-Factor

    // PURPOSE OF THIS SUBROUTINE:
    //     Subroutine to calculate the glazing temperatures of the
    //     various elements of a window/shade array while solving an energy
    //     balance which accounts for absorbed solar radiation, indoor-
    //     outdoor temperature difference, any combination of hemispherical
    //     IR optical properties of the various glazings/shading layers.
    //     Mean radiant temperatures can differ from air temperature on
    //     both the indoor and outdoor sides.
    //     It is also possible to allow air-flow between the two layers
    //     adjacent to the indoor side and/or the two layers adjacent the
    //     outdoor side. U-factor and SHGC calculations are also included (optional)

    // METHODOLOGY EMPLOYED:
    // Uses the net radiation method developed for ASHWAT fenestration
    // model by John Wright, the University of WaterLoo

    // REFERENCES:
    //  ASHRAE RP-1311

    bool ASHWAT_ThermalRatings;

    // Argument array dimensioning
    EP_SIZE_CHECK(QOCF, FS.NL);
    EP_SIZE_CHECK(T, FS.NL);
    EP_SIZE_CHECK(JF, FS.NL + 1);
    EP_SIZE_CHECK(JB, FS.NL + 1);
    EP_SIZE_CHECK(HC, FS.NL + 1);

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:
    //   FS.NL determines # of layers modelled
    //   coefficient, W/m2K
    //   = outside direct + outside diffuse + inside diffuse
    //   0.001 (good) or 0.0001 (tight)
    //   due to open channel flow, W/m2
    //   + = heat flow indoor to outdoor
    //   JF( NL+1) = room radiosity
    //   JB[ 0] = outside environment radiosity
    //   0=outside, 1=betw layer 1-2, ..., NL=inside

    // FUNCTION PARAMETER DEFINITIONS:
    Real64 constexpr Height(1.0); // Window height (m) for standard ratings calculation
    int constexpr MaxIter(100);   // maximum number of iterations allowed

    Real64 ALPHA;
    Real64 HCOCFout;
    Array2D<Real64> A(3 * FS.NL + 4, 3 * FS.NL + 2);
    Array1D<Real64> XSOL(3 * FS.NL + 2);
    Real64 MAXERR;
    Array1D<Real64> TNEW(FS.NL);        // latest estimate of layer temperatures, K
    Array1D<Real64> EB({0, FS.NL + 1}); // black emissive power by layer, W/m2
                                        //   EB( 0) = outdoor environment, EB( NL+1) = indoor environment
    Array1D<Real64> HHAT({0, FS.NL});   // convective heat transfer coefficient (W/m2.K4)
                                        //   based on EB, NOT temperature difference
    Real64 RHOF_ROOM;                   // effective longwave room-side properties
    Real64 TAU_ROOM;
    Real64 EPSF_ROOM;
    Real64 RHOB_OUT; // effective longwave outdoor environment properties
    Real64 TAU_OUT;
    Real64 EPSB_OUT;
    Array1D<Real64> QNET(FS.NL); // checksum - net heat flux to a layer - should be zero - not needed
    int ADIM;                    // dimension of the A matrix
    int CONVRG;
    int NL;
    int I;
    int J;
    int ITRY;
    Array1D_int ISDL({0, FS.NL + 1}); // Flag to mark diathermanous layers, 0=opaque
    int NDLIAR;                       // Number of Diathermanous Layers In A Row (i.e., consecutive)
    int IB;                           // Counter begin and end limits
    int IE;
    int IDV;                       // Integer dummy variable, general utility
    Array1D<Real64> QOCF_F(FS.NL); // heat flux to outdoor-facing surface of layer i, from gap i-1,
                                   //   due to open channel flow, W/m2
    Array1D<Real64> QOCF_B(FS.NL); // heat flux to indoor-facing surface of layer i, from gap i,
                                   //   due to open channel flow, W/m2
    Real64 Rvalue;                 // R-value in IP units [hr.ft2.F/BTU]
    Real64 TAE_IN;                 // Indoor and outdoor effective ambient temperatures [K]
    Real64 TAE_OUT;
    Array1D<Real64> HR({0, FS.NL}); // Radiant heat transfer coefficient [W/m2K]
    Array1D<Real64> HJR(FS.NL);     // radiative and convective jump heat transfer coefficients
    Array1D<Real64> HJC(FS.NL);
    Real64 FHR_OUT; // hre/(hre+hce) fraction radiant h, outdoor or indoor, used for TAE
    Real64 FHR_IN;
    Real64 Q_IN;                          // net gain to the room [W/m2], including transmitted solar
    Array1D<Real64> RHOF({0, FS.NL + 1}); // longwave reflectance, front    !  these variables help simplify
    Array1D<Real64> RHOB({0, FS.NL + 1}); // longwave reflectance, back     !  the code because it is useful to
    Array1D<Real64> EPSF({0, FS.NL + 1}); // longwave emissivity,  front    !  increase the scope of the arrays
    Array1D<Real64> EPSB({0, FS.NL + 1}); // longwave emissivity,  back     !  to include indoor and outdoor
    Array1D<Real64> TAU({0, FS.NL + 1});  // longwave transmittance         !  nodes - more general
    Real64 RTOT;                          // total resistance from TAE_OUT to TAE_IN [m2K/W]
    Array2D<Real64> HC2D(6, 6);           // convective heat transfer coefficients between layers i and j
    Array2D<Real64> HR2D(6, 6);           // radiant heat transfer coefficients between layers i and j
    Array1D<Real64> HCIout(6);            // convective and radiant heat transfer coefficients between
    Array1D<Real64> HRIout(6);
    // layer i and outdoor air or mean radiant temperature, resp.
    Array1D<Real64> HCIin(6); // convective and radiant heat transfer coefficients between
    Array1D<Real64> HRIin(6);
    // layer i and indoor air or mean radiant temperature, resp.
    Real64 HCinout; // convective and radiant heat transfer coefficients between
    Real64 HRinout;
    // indoor and outdoor air or mean radiant temperatures
    // (almost always zero)
    //  Indoor side convection coefficients - used for Open Channel Flow on indoor side
    Real64 HFS;     // nominal height of fen system (assumed 1 m)
    Real64 TOC_EFF; // effective thickness of open channel, m
    Real64 ConvF;   // convection factor: accounts for enhanced convection
                    //   for e.g. VB adjacent to open channel
    Real64 HC_GA;   // convection - glass to air
    Real64 HC_SA;   // convection - shade (both sides) to air
    Real64 HC_GS;   // convection - glass to shade (one side)
    Real64 TINdv;   // dummy variables used
    Real64 TOUTdv;
    Real64 TRMINdv; // for boundary conditions in calculating
    Real64 TRMOUTdv;
    Array1D<Real64> SOURCEdv(FS.NL + 1); // indices of merit
    Real64 QGAIN;                        // total gain to conditioned space [[W/m2]
    Real64 SaveHCNLm;                    // place to save HC[NL-1] - two resistance networks differ
    Real64 SaveHCNL;                     // place to save HC[NL]   - two resistance networks differ
                                         // in their definitions of these heat transfer coefficients

    ASHWAT_ThermalRatings = false; // init to failure
    NL = FS.NL;                    // working copy
    if (NL < 1) {
        return ASHWAT_ThermalRatings;
    }

    HCOCFout = HCOUT; // outdoor side

    HHAT = 0.0;
    HC = 0.0;
    HR = 0.0;
    T = 0.0;
    TNEW = 0.0;
    EB = 0.0;
    JF = 0.0;
    JB = 0.0;
    Q = 0.0;
    QOCF_F = 0.0;
    QOCF_B = 0.0;
    QOCF = 0.0;
    QOCFRoom = 0.0;
    QNET = 0.0;
    QGAIN = 0.0;
    TAU = 0.0;
    RHOF = 0.0;
    RHOB = 0.0;
    EPSF = 0.0;
    EPSB = 0.0;
    HC_GA = 0.0;
    HC_SA = 0.0;
    HC_GS = 0.0;

    ITRY = 0;

    EB(0) = Constant::StefanBoltzmann * pow_4(TOUT);
    EB(NL + 1) = Constant::StefanBoltzmann * pow_4(TIN);

    ADIM = 3 * NL + 2; // DIMENSION OF A-MATRIX

    // organize longwave radiant properties - book-keeping

    TAU_ROOM = 0.0;                         // must always be zero
    RHOF_ROOM = 0.0;                        // almost always zero
    EPSF_ROOM = 1.0 - TAU_ROOM - RHOF_ROOM; // almost always unity
    RHOF(NL + 1) = RHOF_ROOM;
    EPSF(NL + 1) = EPSF_ROOM;
    TAU(NL + 1) = TAU_ROOM;

    for (I = 1; I <= NL; ++I) {
        EPSF(I) = FS.L(I).LWP_EL.EPSLF;
        EPSB(I) = FS.L(I).LWP_EL.EPSLB;
        TAU(I) = FS.L(I).LWP_EL.TAUL;
        RHOF(I) = 1.0 - FS.L(I).LWP_EL.EPSLF - FS.L(I).LWP_EL.TAUL;
        RHOB(I) = 1.0 - FS.L(I).LWP_EL.EPSLB - FS.L(I).LWP_EL.TAUL;
    }

    TAU_OUT = 0.0;                       // must always be zero
    RHOB_OUT = 0.0;                      // DON'T CHANGE
    EPSB_OUT = 1.0 - TAU_OUT - RHOB_OUT; // should always be unity
    TAU(0) = TAU_OUT;
    EPSB(0) = EPSB_OUT;
    RHOB(0) = RHOB_OUT;

    // Later could add RHOF_ROOM to the parameter list
    // Relaxation needed to keep solver stable if OCF is present

    ALPHA = 1.0;
    if (NL >= 2) {
        if (FS.G(NL - 1).GTYPE == state.dataWindowEquivalentLayer->gtyOPENin) {
            ALPHA = 0.5;
        }
        if (FS.G(1).GTYPE == state.dataWindowEquivalentLayer->gtyOPENout) {
            ALPHA = 0.10;
        }
    }

    //   FIRST ESTIMATE OF GLAZING TEMPERATURES AND BLACK EMISSIVE POWERS
    for (I = 1; I <= NL; ++I) {
        T(I) = TOUT + double(I) / double(NL + 1) * (TIN - TOUT);
        EB(I) = Constant::StefanBoltzmann * pow_4(T(I));
    }

    CONVRG = 0;

    //  Start the solver
    //  ITERATION RE-ENTRY

    Real64 const TIN_2(pow_2(TIN));
    Real64 const TOUT_2(pow_2(TOUT));
    Real64 const TRMOUT_4(pow_4(TRMOUT));
    Real64 const TRMIN_4(pow_4(TRMIN));

    for (ITRY = 1; ITRY <= MaxIter; ++ITRY) {

        //  CALCULATE GAS LAYER CONVECTIVE HEAT TRANSFER COEFFICIENTS

        // start by assuming no open channel flow on indoor side

        HC[NL] = HCIN; //  default - HC[NL] supplied by calling routine
                       //  use this for HBX
                       // or
                       // trigger calculation of HC[NL] using ASHRAE correlation
                       //  HC[NL] = HIC_ASHRAE(1.0d0, T(NL), TIN)  ! h - flat plate
                       // Add by BAN June 2013 for standard ratings U-value and SHGC calc only
        if (HCInFlag) {
            HC[NL] = HCInWindowStandardRatings(state, Height, T(NL), TIN);
        }
        HC[0] = HCOUT; // HC[0] supplied by calling routine as HCOUT

        // Check for open channels -  only possible with at least two layers
        if (NL >= 2) {

            int hin_scheme = 3; //  different schemes for calculating convection
                                //  coefficients glass-to-air and shade-to-air
                                //  if open channel air flow is allowed
                                //  see the corresponding subroutines for detail
                                //  = 1 gives dependence of height, spacing, delta-T
                                //  = 2 gives dependence of spacing, delta-T but
                                //    returns unrealistic values for large spacing
                                //  = 3 glass-shade spacing dependence only on HCIN
                                //  = negative, applies HCIN without adjusting for
                                //    temperature, height, spacing, slat angle
                                //  Recommended -> hin_scheme=3 for use with HBX,
                                //              simplicity, right trends wrt spacing

            for (I = 1; I <= NL - 1; ++I) { // Scan gaps between layers

                // DEAL WITH INDOOR OPEN CHANNEL FLOW HERE
                if ((I == NL - 1) && (FS.G(I).GTYPE == state.dataWindowEquivalentLayer->gtyOPENin)) {

                    // TOC_EFF = FS%G( I)%TAS_EFF / 1000.    ! effective thickness of OC gap, m
                    TOC_EFF = FS.G(I).TAS_EFF; // effective thickness of OC gap, m Modified by BAN May 9, 2013
                    HFS = 1.0;                 // nominal height of system (m)

                    // convection - glass to air
                    GLtoAMB(state, TOC_EFF, HFS, T(NL - 1), TIN, HCIN, HC_GA, hin_scheme);
                    // CALL GLtoAMB( 1.0, HFS, T( NL-1), TIN, HCIN, HC_GA, hin_scheme)
                    //   ^ VERY WIDE GAP

                    // convection - shade (both sides) to air
                    ConvF = ConvectionFactor(FS.L(I + 1));
                    HC_SA = ConvF * SLtoAMB(state, TOC_EFF, HFS, T(NL), TIN, HCIN, hin_scheme);
                    // HC_SA = ConvF * SLtoAMB( 1.0, HFS, T(NL), TIN, HCIN, hin_scheme)
                    //  ^ VERY WIDE GAP

                    // convection - glass to shade (one side)
                    SLtoGL(state, TOC_EFF, T(NL), T(NL - 1), HC_GS, 1);
                    // CALL  SLtoGL( 1.0, T(NL), T(NL-1), HC_GS, 2)   !  REMOVE LATER
                    //  ^ VERY WIDE GAP, should return near zero
                    //  Don't use hin_scheme as last parameter - set manually
                    //  1 = Conduction, 2 = slight Ra penalty
                    //  Set negative for default HC_GS=0
                    //  Recommended:  2
                    HC[NL - 1] = HC_GS;
                    HC[NL] = HCIN * ConvF;
                    QOCF_B(NL - 1) = (TIN - T(NL - 1)) * HC_GA;
                    QOCF_F(NL) = (TIN - T(NL)) * (HC_SA - HC[NL]);
                    QOCFRoom = -QOCF_B(NL - 1) - QOCF_F(NL);
                    // end of gap open to indoor side

                } else if ((I == 1) && (FS.G(I).GTYPE == state.dataWindowEquivalentLayer->gtyOPENout)) {
                    // outdoor open channel
                    QOCF_B(1) = (TOUT - T(1)) * HCOCFout;
                    QOCF_F(2) = (TOUT - T(2)) * HCOCFout;
                    HC[1] = 0.0;
                    HC[0] = HCOCFout;
                } else {
                    // normal gap
                    HC[I] = HConvGap(FS.G(I), T(I), T(I + 1));
                }
            } //  end scan through gaps

            // total OCF gain to each layer
            QOCF = QOCF_F + QOCF_B;

        } //  end IF (NL .GE. 2)

        //  CONVERT TEMPERATURE POTENTIAL CONVECTIVE COEFFICIENTS to
        //  BLACK EMISSIVE POWER POTENTIAL CONVECTIVE COEFFICIENTS

        HHAT(0) = HC[0] * (1.0 / Constant::StefanBoltzmann) / ((TOUT_2 + pow_2(T(1))) * (TOUT + T(1)));

        Real64 T_I_2(pow_2(T(1))), T_IP_2;
        for (I = 1; I <= NL - 1; ++I) { // Scan the cavities
            T_IP_2 = pow_2(T(I + 1));
            HHAT(I) = HC[I] * (1.0 / Constant::StefanBoltzmann) / ((T_I_2 + T_IP_2) * (T(I) + T(I + 1)));
            T_I_2 = T_IP_2;
        }

        HHAT(NL) = HC[NL] * (1.0 / Constant::StefanBoltzmann) / ((pow_2(T(NL)) + TIN_2) * (T(NL) + TIN));

        //  SET UP MATRIX
        XSOL = 0.0;
        A = 0.0;

        int L = 1;
        A(1, L) = 1.0;
        A(2, L) = -1.0 * RHOB(0); //  -1.0 * RHOB_OUT
        A(ADIM + 1, L) = EPSB_OUT * Constant::StefanBoltzmann * TRMOUT_4;

        for (I = 1; I <= NL; ++I) {
            L = 3 * I - 1;
            A(3 * I - 2, L) = RHOF(I);
            A(3 * I - 1, L) = -1.0;
            A(3 * I, L) = EPSF(I);    //  LWP( I)%EPSLF
            A(3 * I + 2, L) = TAU(I); //  LWP( I)%TAUL
            A(ADIM + 1, L) = 0.0;

            L = 3 * I;
            if (NL == 1) {
                A(1, L) = 1.0; // Single layer
                A(2, L) = -1.0;
                A(3, L) = -1.0 * (HHAT(0) + HHAT(1));
                A(4, L) = -1.0;
                A(5, L) = 1.0;
                A(ADIM + 1, L) = -1.0 * (HHAT(0) * EB(0) + HHAT(1) * EB(2) + SOURCE(1) + QOCF(1));
            } else if (I == 1) {
                A(1, L) = 1.0; //  Outdoor layer
                A(2, L) = -1.0;
                A(3, L) = -1.0 * (HHAT(0) + HHAT(1));
                A(4, L) = -1.0;
                A(5, L) = 1.0;
                A(6, L) = HHAT(1);
                A(ADIM + 1, L) = -1.0 * (HHAT(0) * EB(0) + SOURCE(1) + QOCF(1));
            } else if (I == NL) {
                A(3 * NL - 3, L) = HHAT(NL - 1); // Indoor layer
                A(3 * NL - 2, L) = 1.0;
                A(3 * NL - 1, L) = -1.0;
                A(3 * NL, L) = -1.0 * (HHAT(NL) + HHAT(NL - 1));
                A(3 * NL + 1, L) = -1.0;
                A(3 * NL + 2, L) = 1.0;
                A(ADIM + 1, L) = -1.0 * (HHAT(NL) * EB(NL + 1) + SOURCE(NL) + QOCF(NL));
            } else {
                A(3 * I - 3, L) = HHAT(I - 1);
                A(3 * I - 2, L) = 1.0;
                A(3 * I - 1, L) = -1.0;
                A(3 * I, L) = -1.0 * (HHAT(I) + HHAT(I - 1));
                A(3 * I + 1, L) = -1.0;
                A(3 * I + 2, L) = 1.0;
                A(3 * I + 3, L) = HHAT(I);
                A(ADIM + 1, L) = -1.0 * (SOURCE(I) + QOCF(I));
            }
            L = 3 * I + 1;
            A(3 * I - 2, L) = TAU(I); //   LWP( I)%TAUL
            A(3 * I, L) = EPSB(I);    //   LWP( I)%EPSLB
            A(3 * I + 1, L) = -1.0;
            A(3 * I + 2, L) = RHOB(I);
            A(ADIM + 1, L) = 0.0;
        }

        L = 3 * NL + 2;
        A(3 * NL + 1, L) = -1.0 * RHOF(NL + 1); //   - 1.0 * RHOF_ROOM
        A(3 * NL + 2, L) = 1.0;
        A(ADIM + 1, L) = EPSF_ROOM * Constant::StefanBoltzmann * TRMIN_4;

        //  SOLVE MATRIX
        //  Call SOLMATS for single precision matrix solution
        SOLMATS(ADIM, A, XSOL);

        //  UNPACK SOLUTION VECTOR AND RECORD LARGEST TEMPERATURE CHANGE
        JB[0] = XSOL(1);

        MAXERR = 0.0;
        for (I = 1; I <= NL; ++I) {
            J = 3 * I - 1;
            JF(I) = XSOL(J);
            ++J;
            EB(I) = max(1.0, XSOL(J)); // prevent impossible temps
            TNEW(I) = root_4(EB(I) / Constant::StefanBoltzmann);
            ++J;
            JB[I] = XSOL(J);
            MAXERR = max(MAXERR, std::abs(TNEW(I) - T(I)) / TNEW(I));
        }

        JF(NL + 1) = XSOL(ADIM);

        //  CALCULATE HEAT FLUX AT EACH GAP, Q
        for (I = 0; I <= NL; ++I) { // Loop gaps (including inside and outside
            Q(I) = JF(I + 1) - JB[I] + HHAT(I) * (EB(I + 1) - EB(I));
        }

        //  A CHECK - NET HEAT FLUX INTO ANY LAYER, AT STEADY-STATE,
        //  SHOULD BE ZERO
        for (I = 1; I <= NL; ++I) {
            QNET(I) = SOURCE(I) + QOCF(I) + Q(I) - Q(I - 1);
        }

        //  UPDATE GLAZING TEMPERATURES AND BLACK EMISSIVE POWERS
        for (I = 1; I <= NL; ++I) {
            T(I) += ALPHA * (TNEW(I) - T(I));
            EB(I) = Constant::StefanBoltzmann * pow_4(T(I));
        }

        //  CHECK FOR CONVERGENCE
        if (CONVRG > 0) {
            break;
        }
        if (MAXERR < TOL) {
            ++CONVRG;
        }

    } // main iteration

    // if (CONVRG == 0) {

    //    if (FS.WEQLSolverErrorIndex < 1) {
    //        ++FS.WEQLSolverErrorIndex;
    //        ShowSevereError(state, format("CONSTRUCTION:WINDOWEQUIVALENTLAYER = \"{}\"", FS.Name));
    //        ShowContinueError(state, format("{}Net radiation analysis did not converge", RoutineName));
    //        ShowContinueError(state, format("...Maximum error is = {:.6T}", MAXERR));
    //        ShowContinueError(state, format("...Convergence tolerance is = {:.6T}", TOL));
    //        ShowContinueErrorTimeStamp(state, "");
    //    } else {
    //        ShowRecurringWarningErrorAtEnd(state, "CONSTRUCTION:WINDOWEQUIVALENTLAYER = \"" + FS.Name + "\"; " + std::string{RoutineName} +
    //                                           "Net radiation analysis did not converge error continues.",
    //                                       FS.WEQLSolverErrorIndex);
    //    }
    //}

    //  NOTE:  HC_SA, HC_GA and HC_SG are only available if there is
    //         an open channel on the indoor side and the calculation of
    //         these coefficients was triggered earlier
    QGAIN = SOURCE(NL + 1) + HC[NL] * (T(NL) - TIN) + JB[NL] - JF(NL + 1);
    // Modified by BAN May 3, 2013 to avoid zero layer index
    if (NL >= 2) {
        if (FS.G(NL - 1).GTYPE == state.dataWindowEquivalentLayer->gtyOPENin) {
            QGAIN = SOURCE(NL + 1) + (HC_SA / 2.0) * (T(NL) - TIN) + JB[NL] - JF(NL + 1);
            QGAIN += HC_GA * (T(NL - 1) - TIN) + (HC_SA / 2.0) * (T(NL) - TIN);
        }
    }

    ASHWAT_ThermalRatings = true;

    // New code follows from here - for calculating Ucg and SHGC
    // NOTE: This code can be bypassed if
    //       indices of merit are not needed

    //  Initialize various things
    HR = 0.0;
    HJC = 0.0;
    HJR = 0.0;
    TAE_OUT = 0.0;
    TAE_IN = 0.0;
    FHR_OUT = 0.0;
    FHR_IN = 0.0;
    Q_IN = 0.0;
    RTOT = 0.0;
    UCG = 0.0;
    SHGC = 0.0;
    Rvalue = 0.0;
    HC2D = 0.0;
    HR2D = 0.0;
    HCIout = 0.0;
    HRIout = 0.0;
    HCIin = 0.0;
    HRIin = 0.0;
    HCinout = 0.0;
    HRinout = 0.0;
    TNEW = 0.0;
    TINdv = 0.0;
    TOUTdv = 0.0;
    TRMINdv = 0.0;
    TRMOUTdv = 0.0;
    SOURCEdv = 0.0;

    //  Identify the diathermanous layers
    ISDL = 0;
    for (I = 1; I <= NL; ++I) {
        if (FS.L(I).LWP_EL.TAUL > 0.001) {
            ISDL(I) = 1; // layer is diathermanous
        }
        // of tau_lw > 0.001 (ie 0.1%)
        // Note:  ISDL(0) and ISDL(NL+1)
        //        must both be zero
    } // end loop to calculate ISDL(i)

    //  determine the largest number of consecutive diathermanous layers, NDLIAR
    //                   i.e., the number of diathermanous layers in a row
    NDLIAR = 0;
    IDV = 0;
    for (I = 1; I <= NL; ++I) {
        if (ISDL(I) == 1) {
            ++IDV;
        } else {
            IDV = 0;
        }
        if (IDV > NDLIAR) {
            NDLIAR = IDV;
        }
    } // end loop to calculate NDLIAR

    if (NDLIAR > 1) {
        return ASHWAT_ThermalRatings; // cannot handle two (or more) consecutive
    }
    // diathermanous layers, U/SHGC calculation
    // will be skipped entirely
    // CHANGE TO (NDLIAR .GT. 2) ONCE
    // SUBROUTINE DL2_RES IS AVAILABLE

    //   calculate radiant heat transfer coefficients between adjacent opaque
    //   layers
    for (I = 0; I <= NL; ++I) { // scan through all gaps - including indoor/outdoor
        if ((ISDL(I) == 0) && (ISDL(I + 1) == 0)) {
            if (I == 0) { //  outdoor side
                HR(I) = HRadPar(T(1), TRMOUT, EPSF(1), EPSB(0));
            } else if (I == NL) { //  indoor side
                HR(I) = HRadPar(T(NL), TRMIN, EPSF(NL + 1), EPSB(NL));
            } else { //  cavities
                HR(I) = HRadPar(T(I), T(I + 1), EPSF(I + 1), EPSB(I));
            }
        }
    } // end loop through gaps

    //   calculate radiant heat transfer coefficients at single diathermanous
    //   layers,three coefficients in each case

    for (I = 0; I <= NL - 1; ++I) { // scan through all layers - look for single DL
                                    // layers between two opaque layers
        if ((ISDL(I) == 0) && (ISDL(I + 1) == 1) && (ISDL(I + 2) == 0)) {
            if (I == 0) { //  outdoor layer is diathermanous
                if (NL == 1) {
                    DL_RES_r2(TRMOUT, T(1), TRMIN, RHOB(0), RHOF(1), RHOB(1), TAU(1), RHOF(2), HJR(1), HR(0), HR(1));
                } else {
                    DL_RES_r2(TRMOUT, T(1), T(2), RHOB(0), RHOF(1), RHOB(1), TAU(1), RHOF(2), HJR(1), HR(0), HR(1));
                }
            } else {               //  with IF (I .EQ. 0)   i.e., i != 0
                if (I == NL - 1) { //  indoor layer is diathermanous
                    DL_RES_r2(T(NL - 1), T(NL), TRMIN, RHOB(NL - 1), RHOF(NL), RHOB(NL), TAU(NL), RHOF(NL + 1), HJR(NL), HR(NL - 1), HR(NL));
                } else { // some intermediate layer is diathermanous
                    DL_RES_r2(T(I), T(I + 1), T(I + 2), RHOB(I), RHOF(I + 1), RHOB(I + 1), TAU(I + 1), RHOF(I + 2), HJR(I + 1), HR(I), HR(I + 1));
                } //   end of IF/ELSE (I .EQ. NL-1)
            } //  end of IF/ELSE (I .EQ. 0)
        } //  end of IF(ISDL(I) .EQ. 0) .AND. .....
    } //   end of scan through all layers

    //   calculate radiant heat transfer coefficients at double diathermanous
    //   layers,six coefficients in each case
    //   THIS SECTION NOT ACTIVE YET

    if (NL >= 2) {
        for (I = 0; I <= NL - 2; ++I) { // scan through all layers - look for double DL
                                        // layers between two opaque layers
            if ((ISDL(I) == 0) && (ISDL(I + 1) == 1) && (ISDL(I + 2) == 1) && (ISDL(I + 3) == 0)) {
                if (I == 0) {
                    //                CALL DL2_RES(TRMOUT, T(1), T(2), T(3) etc)
                } else {
                    if (I == NL - 2) {
                        //                CALL DL2_RES(T(NL-2), T(NL-1), T(NL), TRMIN, etc)
                    } else {
                        //                CALL DL2_RES(T(I), T(I+1), T(I+2), T(I+3) etc)
                    } //   end of IF/ELSE (I .EQ. NL-1)
                } //  end of IF/ELSE (I .EQ. 0)
            } //  end of IF(ISDL(I) .EQ. 0) .AND. .....
        } //   end of scan through all layers

        //  calculate convective OCF/jump heat transfer coefficients
        // no OCF unless at least two layers exist
        //  It is not possible for both of the following cases to be
        //  true for the same gap (i.e., for NL=2)

        if (FS.G(NL - 1).GTYPE == state.dataWindowEquivalentLayer->gtyOPENin) {
            SaveHCNLm = HC[NL - 1];
            SaveHCNL = HC[NL];
            HC[NL - 1] = HC_GS;
            HC[NL] = HC_SA;
            HJC(NL) = HC_GA;
        }

        HC[0] = HCOUT;
        if (FS.G(1).GTYPE == state.dataWindowEquivalentLayer->gtyOPENout) {
            HC[0] = HCOUT + HCOCFout;
            HJC(1) = HCOCFout;
        }
    }

    //  copy convective heat transfer coefficients to 2D arrays
    //  adjacent layers
    IB = 1;
    IE = NL - 1;
    if (IB <= IE) {
        for (I = IB; I <= IE; ++I) {
            HC2D(I + 1, I) = HC[I];
            HC2D(I, I + 1) = HC2D(I + 1, I);
        }
    }

    //  jumpers
    IB = 2;
    IE = NL - 1;
    if (IB <= IE) {
        for (I = IB; I <= IE; ++I) {
            HC2D(I + 1, I - 1) = HJC(I);
            HC2D(I - 1, I + 1) = HC2D(I + 1, I - 1);
        }
    }

    //  double jumpers  - NOT ACTIVE YET
    IB = 2;
    IE = NL - 2;
    if (IB <= IE) {
        for (I = IB; I <= IE; ++I) {
            //         HC2D(I-1,I+2) = H2JC(I)
            //         HC2D(I+2,I-1) = HC2D(I-1,I+2)
        }
    }

    //  outdoor side
    HCIout(1) = HC[0];
    if (NL >= 2) {
        HCIout(2) = HJC(1);
    }

    //  indoor side
    HCIin(NL) = HC[NL];
    if (NL >= 2) {
        HCIin(NL - 1) = HJC(NL);
    }

    //  special case - indoor-to-outdoor convection (?)
    HCinout = 0.0;

    //  copy radiative heat transfer coefficients to 2D arrays
    //  adjacent layers
    IB = 1;
    IE = NL - 1;
    if (IB <= IE) {
        for (I = IB; I <= IE; ++I) {
            HR2D(I + 1, I) = HR(I);
            HR2D(I, I + 1) = HR2D(I + 1, I);
        }
    }

    //  jumpers
    IB = 2;
    IE = NL - 1;
    if (IB <= IE) {
        for (I = IB; I <= IE; ++I) {
            HR2D(I + 1, I - 1) = HJR(I);
            HR2D(I - 1, I + 1) = HR2D(I + 1, I - 1);
        }
    }

    //  double jumpers
    IB = 2;
    IE = NL - 2;
    if (IB <= IE) {
        for (I = IB; I <= IE; ++I) {
            //         HR2D(I-1,I+2) = H2JR(I)
            //         HR2D(I+2,I-1) = HR2D(I-1,I+2)
        }
    }

    //  outdoor side
    HRIout(1) = HR(0);
    if (NL >= 2) {
        HRIout(2) = HJR(1);
    }

    //  indoor side
    HRIin(NL) = HR(NL);
    if (NL >= 2) {
        HRIin(NL - 1) = HJR(NL);
    }

    //  special case - indoor-to-outdoor radiation
    if (NL == 1) {
        HRinout = HJR(1);
    }
    //       IF(NL .EQ. 2)  HRinout=H2JR(1)

    //  CONFIRM VALIDITY OF CODE

    // if (1 == 0) { //  was used for debugging - successfully
    //              //  and can now be bypassed
    //              //  - code in this section generates the
    //              //  same solution of temperatures (TNEW(i))
    //              //  that was found by the net radiation
    //              //  solver above (T(i))

    //    ADIM = NL;
    //    A = 0.0;
    //    XSOL = 0.0;
    //    TOUTdv = TOUT;     // solution for TNEW should
    //    TRMOUTdv = TRMOUT; // match existing solution
    //    TINdv = TIN;       // for T
    //    TRMINdv = TRMIN;
    //    SOURCEdv = SOURCE;

    //    for (I = 1; I <= NL; ++I) {
    //        A(ADIM + 1, I) = HCIout(I) * TOUTdv + HRIout(I) * TRMOUTdv + HCIin(I) * TINdv + HRIin(I) * TRMINdv + SOURCEdv(I);
    //        A(I, I) = HCIout(I) + HRIout(I) + HCIin(I) + HRIin(I);
    //        for (J = 1; J <= NL; ++J) {
    //            if (J != I) {
    //                A(I, I) += HC2D(J, I) + HR2D(J, I);
    //                A(J, I) = -1.0 * (HC2D(J, I) + HR2D(J, I));
    //            }
    //        }
    //    }

    //    //  SOLVE MATRIX
    //    //  Call SOLMATS for single precision matrix solution
    //    SOLMATS(ADIM, A, XSOL);

    //    //  UNPACK SOLUTION VECTOR

    //    SUMERR = 0.0;
    //    for (I = 1; I <= NL; ++I) {
    //        TNEW(I) = XSOL(I);
    //        SUMERR += std::abs(TNEW(I) - T(I));
    //    }

    //} //   end (1 .EQ. 0)    code disabled

    //  calculate U-factor

    ADIM = NL;
    A = 0.0;
    XSOL = 0.0;
    TNEW = 0.0;
    TOUTdv = 1.0;
    TRMOUTdv = 1.0;
    TINdv = 0.0;
    TRMINdv = 0.0;
    SOURCEdv = 0.0;

    for (I = 1; I <= NL; ++I) {
        A(ADIM + 1, I) = HCIout(I) * TOUTdv + HRIout(I) * TRMOUTdv + HCIin(I) * TINdv + HRIin(I) * TRMINdv + SOURCEdv(I);
        A(I, I) = HCIout(I) + HRIout(I) + HCIin(I) + HRIin(I);
        for (J = 1; J <= NL; ++J) {
            if (J != I) {
                A(I, I) += HC2D(J, I) + HR2D(J, I);
                A(J, I) = -1.0 * (HC2D(J, I) + HR2D(J, I));
            }
        }
    }

    //  SOLVE MATRIX
    //  Call SOLMATS for single precision matrix solution
    SOLMATS(ADIM, A, XSOL);
    //  UNPACK SOLUTION VECTOR

    for (I = 1; I <= NL; ++I) {
        TNEW(I) = XSOL(I);
    }

    Q_IN = HCinout * (TOUTdv - TINdv) + HRinout * (TRMOUTdv - TRMINdv);
    for (I = 1; I <= NL; ++I) {
        Q_IN += HCIin(I) * (TNEW(I) - TINdv) + HRIin(I) * (TNEW(I) - TRMINdv);
    }
    Q_IN += SOURCEdv(NL + 1); // this line not needed
    UCG = Q_IN;
    Rvalue = 5.678 / UCG;

    //  calculate SHGC

    SHGC = 0.0;
    if (std::abs(ISOL) > 0.01) {
        ADIM = NL;
        A = 0.0;
        XSOL = 0.0;
        TNEW = 0.0;
        TOUTdv = 0.0;
        TRMOUTdv = 0.0;
        TINdv = 0.0;
        TRMINdv = 0.0;
        for (I = 1; I <= NL + 1; ++I) {
            SOURCEdv(I) = SOURCE(I);
        }

        for (I = 1; I <= NL; ++I) {
            A(ADIM + 1, I) = HCIout(I) * TOUTdv + HRIout(I) * TRMOUTdv + HCIin(I) * TINdv + HRIin(I) * TRMINdv + SOURCEdv(I);
            A(I, I) = HCIout(I) + HRIout(I) + HCIin(I) + HRIin(I);
            for (J = 1; J <= NL; ++J) {
                if (J != I) {
                    A(I, I) += HC2D(J, I) + HR2D(J, I);
                    A(J, I) = -1.0 * (HC2D(J, I) + HR2D(J, I));
                }
            }
        }

        //  SOLVE MATRIX
        //  Call SOLMATS for single precision matrix solution
        SOLMATS(ADIM, A, XSOL);

        //  UNPACK SOLUTION VECTOR
        for (I = 1; I <= NL; ++I) {
            TNEW(I) = XSOL(I);
        }

        Q_IN = HCinout * (TOUTdv - TINdv) + HRinout * (TRMOUTdv - TRMINdv);
        for (I = 1; I <= NL; ++I) {
            Q_IN += HCIin(I) * (TNEW(I) - TINdv) + HRIin(I) * (TNEW(I) - TRMINdv);
        }
        Q_IN += SOURCEdv(NL + 1);

        SHGC = Q_IN / ISOL; // only executed if ISOL > 0.01 [W/m2]

    } //  end if (ABS(ISOL) .GT. 0.01)

    //  calculate FHR_OUT

    ADIM = NL;
    A = 0.0;
    XSOL = 0.0;
    TNEW = 0.0;
    TOUTdv = 1.0;
    TRMOUTdv = 0.0;
    TINdv = 0.0;
    TRMINdv = 0.0;
    SOURCEdv = 0.0;

    for (I = 1; I <= NL; ++I) {
        A(ADIM + 1, I) = HCIout(I) * TOUTdv + HRIout(I) * TRMOUTdv + HCIin(I) * TINdv + HRIin(I) * TRMINdv + SOURCEdv(I);
        A(I, I) = HCIout(I) + HRIout(I) + HCIin(I) + HRIin(I);
        for (J = 1; J <= NL; ++J) {
            if (J != I) {
                A(I, I) += HC2D(J, I) + HR2D(J, I);
                A(J, I) = -1.0 * (HC2D(J, I) + HR2D(J, I));
            }
        }
    }

    //  SOLVE MATRIX
    //  Call SOLMATS for single precision matrix solution
    SOLMATS(ADIM, A, XSOL);

    //  UNPACK SOLUTION VECTOR

    for (I = 1; I <= NL; ++I) {
        TNEW(I) = XSOL(I);
    }

    Q_IN = HCinout * (TOUTdv - TINdv) + HRinout * (TRMOUTdv - TRMINdv);
    for (I = 1; I <= NL; ++I) {
        Q_IN += HCIin(I) * (TNEW(I) - TINdv) + HRIin(I) * (TNEW(I) - TRMINdv);
    }
    Q_IN += SOURCEdv(NL + 1);

    FHR_OUT = 1.0 - (Q_IN / UCG);
    TAE_OUT = FHR_OUT * TRMOUT + (1.0 - FHR_OUT) * TOUT;

    //  calculate FHR_IN

    ADIM = NL;
    A = 0.0;
    XSOL = 0.0;
    TNEW = 0.0;
    TOUTdv = 0.0;
    TRMOUTdv = 0.0;
    TINdv = 1.0;
    TRMINdv = 0.0;
    SOURCEdv = 0.0;

    for (I = 1; I <= NL; ++I) {
        A(ADIM + 1, I) = HCIout(I) * TOUTdv + HRIout(I) * TRMOUTdv + HCIin(I) * TINdv + HRIin(I) * TRMINdv + SOURCEdv(I);
        A(I, I) = HCIout(I) + HRIout(I) + HCIin(I) + HRIin(I);
        for (J = 1; J <= NL; ++J) {
            if (J != I) {
                A(I, I) += HC2D(J, I) + HR2D(J, I);
                A(J, I) = -1.0 * (HC2D(J, I) + HR2D(J, I));
            }
        }
    }

    //  SOLVE MATRIX
    //  Call SOLMATS for single precision matrix solution
    SOLMATS(ADIM, A, XSOL);

    //  UNPACK SOLUTION VECTOR

    for (I = 1; I <= NL; ++I) {
        TNEW(I) = XSOL(I);
    }

    Q_IN = HCinout * (TOUTdv - TINdv) + HRinout * (TRMOUTdv - TRMINdv);
    for (I = 1; I <= NL; ++I) {
        Q_IN += HCIin(I) * (TNEW(I) - TINdv) + HRIin(I) * (TNEW(I) - TRMINdv);
    }
    Q_IN += SOURCEdv(NL + 1);

    FHR_IN = 1.0 + (Q_IN / UCG);
    TAE_IN = FHR_IN * TRMIN + (1.0 - FHR_IN) * TIN;

    //   double check heat gain to room
    //   Q_IN calculated this way should be equal to QGAIN calculated
    //   above with raw results from the net radiation solution
    //   The difference between the two is printed below
    //   Both include the directly transmitted solar gain

    Q_IN = UCG * (TAE_OUT - TAE_IN) + SHGC * ISOL;

    // End of new code - for calculating Ucg and SHGC
    //  restore convective heat transfer coefficients if altered earlier
    //  for more general resistor network - otherwise mainline will
    //  receive faulty data
    if (NL >= 2) { // no OCF unless at least two layers exist
        if (FS.G(NL - 1).GTYPE == state.dataWindowEquivalentLayer->gtyOPENin) {
            HC[NL - 1] = SaveHCNLm;
            HC[NL] = SaveHCNL;
        }
    }

    return ASHWAT_ThermalRatings;
}

void DL_RES_r2(Real64 const Tg,    // mean glass layer temperature, {K}
               Real64 const Td,    // mean diathermanous layer temperature, {K}
               Real64 const Tm,    // mean radiant room temperature, {K}
               Real64 const rhog,  // reflectance of glass layer, {-}
               Real64 const rhodf, // front reflectance of diathermanous layer, {-}
               Real64 const rhodb, // back reflectance of diathermanous layer, {-}
               Real64 const taud,  // transmittance of diathermanous layer, {-}
               Real64 const rhom,  // reflectance of the room, {-}
               Real64 &hr_gm,      // heat transfer coefficient between left and right surface {W/m2K}
               Real64 &hr_gd,      // heat transfer coefficient between left and middle surface {W/m2K}
               Real64 &hr_md       // heat transfer coefficient between right and middle surface {W/m2K}
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  Returns the radiant heat transfer coefficients between parallel surfaces:
    // METHODOLOGY EMPLOYED:
    //  Solves radiant heat transfer coefficients between three parallel surfaces.
    //  The left and right surfaces are opaque with reflectance rhog and rhom, respectively.
    //  And the middle layer is diathermanous with transmittance taud AND reflectance rhodf
    //  and rhodb on the left and right sides, respectively.
    //  The subscripts g, d and m apply to Glass, Diathermanous layer, and mean-radiant room
    //  temperature in a configuration of a window with an indoor-side shading attachment
    //  but the analysis can be applied to any three layers in the configuration described
    //  above.

    Real64 Epsg;
    Real64 Epsdf;
    Real64 Epsdb;
    Real64 Epsm;
    Array2D<Real64> A(22, 20);
    Array1D<Real64> X(20);
    // real FSg_g, FSdf_g, FSdb_g, FSm_g
    Real64 FSg_df;
    Real64 FSm_df;
    Real64 FSg_db;
    Real64 FSm_db;
    Real64 FSg_m;

    //  Calculate 4 emissivities/absorptivities

    Epsg = 1.0 - rhog;
    Epsdf = 1.0 - rhodf - taud;
    Epsdb = 1.0 - rhodb - taud;
    Epsm = 1.0 - rhom;

    //  Calculate script F shape factors
    //  FSx_y is the portion of radiation emitted
    //  by surface x that arrives at surface y
    //  via any path - including reflections
    //  By reciprocity FSxy=FSyx

    // step 1:  unit emission from (g) only

    SETUP4x4_A(rhog, rhodf, rhodb, taud, rhom, A);
    A(5, 1) = 1.0; // unit source of radiation
    SOLMATS(4, A, X);
    FSg_df = X(1);
    //  FSg_g   = X(2)
    FSg_m = X(3);
    FSg_db = X(4);

    // step 2:  unit emission from (df) only

    //   call SETUP4x4_A(rhog,rhodf,rhodb,taud,rhom,A)
    //   A(2,5) = 1.0        ! unit source of radiation
    //   call SOLMATS(4,A,X)
    //   FSdf_df  = X(1)
    //   FSdf_g   = X(2)
    //   FSdf_m   = X(3)
    //   FSdf_db  = X(4)

    // step 3:  unit emission from (db) only

    //   call SETUP4x4_A(rhog,rhodf,rhodb,taud,rhom,A)
    //   A(3,5) = 1.0        ! unit source of radiation
    //   call SOLMATS(4,A,X)
    //   FSdb_df  = X(1)
    //   FSdb_g   = X(2)
    //   FSdb_m   = X(3)
    //   FSdb_db  = X(4)

    // step 4:  unit emission from (m) only

    SETUP4x4_A(rhog, rhodf, rhodb, taud, rhom, A);
    A(5, 4) = 1.0; // unit source of radiation
    SOLMATS(4, A, X);
    FSm_df = X(1);
    //  FSm_g   = X(2)
    //  FSm_m   = X(3)
    FSm_db = X(4);

    //  calculate heat transfer coefficients
    //  hr_xy is the heat transfer coefficient from x to y [W/m2]
    //  Note:  If the emissivity of either surface x or surface y is zero
    //         then q_xy will also be zero
    //  Note:  This code has no problem with temperatures being equal

    Real64 const Td_2(pow_2(Td));
    Real64 const Tg_2(pow_2(Tg));
    Real64 const Tm_2(pow_2(Tm));
    hr_gm = Epsg * Epsm * FSg_m * Constant::StefanBoltzmann * (Tg + Tm) * (Tg_2 + Tm_2);
    hr_gd = Epsg * Epsdf * FSg_df * Constant::StefanBoltzmann * (Td + Tg) * (Td_2 + Tg_2) +
            Epsg * Epsdb * FSg_db * Constant::StefanBoltzmann * (Td + Tg) * (Td_2 + Tg_2);
    hr_md = Epsm * Epsdf * FSm_df * Constant::StefanBoltzmann * (Td + Tm) * (Td_2 + Tm_2) +
            Epsm * Epsdb * FSm_db * Constant::StefanBoltzmann * (Td + Tm) * (Td_2 + Tm_2);
}

void SETUP4x4_A(Real64 const rhog, Real64 const rhodf, Real64 const rhodb, Real64 const taud, Real64 const rhom, Array2A<Real64> A)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  Returns the 4 X 4 matrix for DL_RES_r2 routine:
    // METHODOLOGY EMPLOYED:
    //  fills in the matrix coefficients

    // Argument array dimensioning
    A.dim(22, 20);

    A = 0.0;
    A(1, 1) = 1.0;
    A(2, 1) = -1.0 * rhog;
    A(1, 2) = -1.0 * rhodf;
    A(2, 2) = 1.0;
    A(4, 2) = -1.0 * taud;
    A(1, 3) = -1.0 * taud;
    A(3, 3) = 1.0;
    A(4, 3) = -1.0 * rhodb;
    A(3, 4) = -1.0 * rhom;
    A(4, 4) = 1.0;
}

Real64 FRA(Real64 const TM, // mean gas temp, K
           Real64 const T,  // gas layer thickness, m
           Real64 const DT, // temp difference across layer, K
           Real64 const AK, // gas conductance coeffs, K = AK + BK*TM + CK*TM*TM
           Real64 const BK,
           Real64 const CK,
           Real64 const ACP, // gas specific heat coeffs, CP = ACP + BCP*TM + CCP*TM*TM
           Real64 const BCP,
           [[maybe_unused]] Real64 const CCP,
           Real64 const AVISC, // gas viscosity coeffs, VISC = AVISC + BVISC*TM + CVISC*TM*TM
           Real64 const BVISC,
           [[maybe_unused]] Real64 const CVISC,
           Real64 const RHOGAS // gas density, kg/m3
)
{
    //       AUTHOR         (John Wright, University of WaterLoo, ASHRAE 1311-RP)
    //       MODIFIED       Bereket Nigusse, FSEC/UCF, May 2013

    // PURPOSE OF THIS FUNCTION:
    // Returns Rayleigh number given surface temperatures, and coefficients of
    // quadratic correlations as a function of temperature for gas properties

    // METHODOLOGY EMPLOYED:
    //  Ra = Gr * Pr

    // REFERENCES:
    //  ASHRAE 1311-RP

    // FUNCTION ARGUMENT DEFINITIONS:
    //   (as adjusted e.g. re VB models)

    Real64 Z = 1.0;
    Real64 K = AK + BK * TM + CK * TM * TM;
    Real64 CP = ACP + BCP * TM + BCP * TM * TM;
    Real64 VISC = AVISC + BVISC * TM + BVISC * TM * TM;

    return (Constant::Gravity * RHOGAS * RHOGAS * DT * T * T * T * CP) / (VISC * K * TM * Z * Z);
}

Real64 FNU(Real64 const RA) // Rayleigh number
{
    //       AUTHOR         (John Wright, University of WaterLoo, ASHRAE 1311-RP)
    //       MODIFIED       Bereket Nigusse, FSEC/UCF, May 2013

    // PURPOSE OF THIS FUNCTION:
    // Returns Nusselt number given Rayleigh number

    // METHODOLOGY EMPLOYED:
    // Uses empirical correlation

    // REFERENCES:
    //  ASHRAE 1311-RP

    Real64 const ARA(std::abs(RA));
    if (ARA <= 10000.0) {
        return 1.0 + 1.75967e-10 * std::pow(ARA, 2.2984755);
    } else if (ARA <= 50000.0) {
        return 0.028154 * std::pow(ARA, 0.413993);
    } else {
        return 0.0673838 * std::pow(ARA, 1.0 / 3.0);
    }
}

Real64 HConvGap(CFSGAP const &G, // gap
                Real64 const T1, // bounding surface temps (K)
                Real64 const T2)
{
    //       AUTHOR         (University of WaterLoo, ASHRAE 1311-RP)
    //       MODIFIED       Bereket Nigusse, FSEC/UCF, May 2013
    // PURPOSE OF THIS FUNCTION:
    // Returns convective coefficient for a gap separated between two surfaces at
    // temperatures T1 and T2 , W/m2-K
    // METHODOLOGY EMPLOYED:
    //  HConv = "Nusselt Number" * "Conductivity Of Gas"  / "Thickness Of Gap"
    // REFERENCES:
    //  ASHRAE 1311-RP

    Real64 TM;   // Mean temperature, K
    Real64 DT;   // temperature difference, (K)
    Real64 RA;   // Rayleigh Number, (-)
    Real64 NU;   // Nusselt Number, (-)
    Real64 KGAS; // Gas conductivity at film temp, (W/m.K)
    Real64 T;    // effective gap spacing, m

    T = G.TAS_EFF;
    TM = (T1 + T2) / 2.0;
    DT = T1 - T2;
    RA = FRA(TM, T, DT, G.FG.AK, G.FG.BK, G.FG.CK, G.FG.ACP, G.FG.BCP, G.FG.CCP, G.FG.AVISC, G.FG.BVISC, G.FG.CVISC, G.RHOGAS);
    NU = FNU(RA);

    KGAS = G.FG.AK + G.FG.BK * TM + G.FG.CK * TM * TM;
    return NU * KGAS / T;
}

Real64 HRadPar(Real64 const T1, // bounding surface temps [K]
               Real64 const T2,
               Real64 const E1, // bounding surface emissivities
               Real64 const E2)
{
    //       AUTHOR         ASHRAE 1311-RP
    // PURPOSE OF THIS FUNCTION:
    // Returns radiative coefficient between two surfaces, hr, W/m2-K
    // METHODOLOGY EMPLOYED:
    //  Radiative coefficient for parallel, opaque plates configuration and
    //  automatically reverts to small-object-in-large-enclosure if one of
    //  the emissivities is set to unity  i.e., set E1=1 and surface 2 is the
    //  small object with hr based on area A2 If one emissivity is zero then
    //  hr=0, division by zero is, avoided even if T1=T2.
    // REFERENCES:
    //  ASHRAE 1311-RP

    // Return value
    Real64 HRadPar;

    Real64 DV; // dummy variable

    HRadPar = 0.0;
    if ((E1 > 0.001) && (E2 > 0.001)) {
        DV = (1.0 / E1) + (1.0 / E2) - 1.0;
        HRadPar = (Constant::StefanBoltzmann / DV) * (T1 + T2) * (pow_2(T1) + pow_2(T2));
    }
    return HRadPar;
}

Real64 HIC_ASHRAE(Real64 const L,  // glazing height, m
                  Real64 const TG, // glazing inside surf temp, C or K
                  Real64 const TI  // inside air temp, C or K
)
{
    //       AUTHOR         ASHRAE 1311-RP
    // PURPOSE OF THIS FUNCTION:
    // Returns inside surface convective coefficient, W/m2-K

    // REFERENCES:
    //  Footnote on Table 2, p. 31.6 (Fenestration) HOF 2005

    // Return value
    Real64 HIC_ASHRAE;

    HIC_ASHRAE = 1.46 * root_4(std::abs(TG - TI) / max(L, 0.001));
    return HIC_ASHRAE;
}

void SLtoGL(EnergyPlusData const &state,
            Real64 const breal, // distance from shade to glass (m)
            Real64 const Ts,    // shade temperature (K)
            Real64 const Tg,    // glass temperature (K)
            Real64 &hsg,        // the heat transfer coefficient, shade-to-glass, {W/m2K}
            int const scheme)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  Returns the heat transfer coefficient, shade-to-glass

    Real64 b;
    Real64 Tavg;
    Real64 rho;
    Real64 beta;
    Real64 dvisc;
    Real64 Cp;
    Real64 k;
    Real64 Rabsg;
    Real64 Nubsg;

    hsg = 0.0; //  default - large spacing, b

    if (scheme == 1) { //  simple conduction layer, Nu=1

        b = breal;
        if (b < 0.00001) {
            b = 0.00001; // avoid division by zero in
        }
        // calculation of this scheme

        Tavg = (Ts + Tg) / 2.0;                                      // T for properties calculations
        k = 0.02538 + ((Tavg - 290.0) / 10.0) * (0.02614 - 0.02538); // conductivity (W/m.K)
        hsg = k / b;

    } else if (scheme == 2) { // similar to Nu=1 but small penalty at
        // larger Ra    (Collins)
        b = breal;
        if (b < 0.00001) {
            b = 0.00001; // avoid division by zero in
        }
        // calculation of this scheme

        Tavg = (Ts + Tg) / 2.0; // T for properties calculations

        // properties of AIR
        rho = state.dataWindowEquivalentLayer->PAtmSeaLevel / (287.097 * Tavg); // density (kg/m3) <- temperature in (K)
        beta = 1.0 / Tavg;                                                      // thermal expansion coeff (/K)
        dvisc = (18.05 + ((Tavg - 290.0) / 10.0) * (18.53 - 18.05)) * 1.0e-6;
        //  dynamic viscosity (kg/m.sec) or (N.sec/m2)
        Cp = 1044.66 - 0.31597 * Tavg + 0.000707908 * pow_2(Tavg) - 0.00000027034 * pow_3(Tavg);
        //  specific heat at constant pressure (J/kg.K)
        k = 0.02538 + ((Tavg - 290.0) / 10.0) * (0.02614 - 0.02538); // conductivity (W/m.K)

        Rabsg = (9.81 * beta * pow_3(b) * std::abs(Ts - Tg) * pow_2(rho) * Cp) / (dvisc * k);
        Nubsg = 1.0 + 0.2 * (1.0 - std::exp(-0.005 * Rabsg));

        hsg = Nubsg * k / b;
    } //  end of scheme .eq. 2
}

Real64 SLtoAMB(EnergyPlusData const &state,
               Real64 const b,     // distance from shade to glass (m) where air flow takes place
               Real64 const L,     // window height, m (usually taken as 1 m)
               Real64 const Ts,    // shade temperature, K
               Real64 const Tamb,  // room air temperature, K
               Real64 const hc_in, // indoor (room) convective transfer coeff, W/m2K)
               int const scheme    // flag to select model, scheme=2 has problems
)
{
    //       AUTHOR         ASHRAE 1311-RP
    // PURPOSE OF THIS FUNCTION:
    // Returns shade to room air heat transfer coefficient
    // METHODOLOGY EMPLOYED:
    // fill gas is always air, orientation is always vertical
    // hsamb should be h-flatplate at b=0 and 2*h-flatplate at b=large.  Note
    // that hsamb is the same at slat angle = 0, 90, -90 degrees but increase
    // by 20% at slat angle =45 degrees to mimic air pumping between slats
    // therefore, specify slat angle=0 or 90 or -90 is shade is other than
    // a venetian blind

    // Return value
    Real64 SLtoAMB;

    // FUNCTION ARGUMENT DEFINITIONS:
    //  scheme=3 recommended

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    // a
    Real64 Tavg;
    Real64 rho;
    Real64 beta;
    Real64 dvisc;
    Real64 Cp;
    Real64 k;
    Real64 Rabsa;
    Real64 hfp;

    SLtoAMB = 2.0 * hc_in; //    DEFAULT - convection from both sides
    //    of shading layer - large spacing, b

    if (scheme == 1) {
        // properties of AIR
        Tavg = (Ts + Tamb) / 2.0;
        rho = state.dataWindowEquivalentLayer->PAtmSeaLevel / (287.097 * Tavg); // density (kg/m3) <- temperature in (K)
        beta = 1.0 / Tavg;                                                      // thermal expansion coeff (/K)
        dvisc = (18.05 + ((Tavg - 290.0) / 10.0) * (18.53 - 18.05)) * 1.0e-6;
        //  dynamic viscosity (kg/m.sec) or (N.sec/m2)
        Cp = 1044.66 - 0.31597 * Tavg + 0.000707908 * pow_2(Tavg) - 0.00000027034 * pow_3(Tavg);
        //  specific heat at constant pressure (J/kg.K)
        k = 0.02538 + ((Tavg - 290.0) / 10.0) * (0.02614 - 0.02538); // conductivity (W/m.K)

        Rabsa = (9.81 * beta * pow_3(b) * std::abs(Ts - Tamb) * pow_2(rho) * Cp) / (dvisc * k);
        if (Rabsa <= 1.0) {
            Rabsa = 1.0;
        }

        hfp = HIC_ASHRAE(L, Ts, Tamb); // h - flat plate, influence by
        // window height and temperature
        // difference.  Note:  hfp goes to
        // zero as delta-T goes to zero

        //  now adjust for distance from window glass
        SLtoAMB = hfp * (1.0 + std::exp(-6000.0 / Rabsa));
        //  SLtoAmb goes to 2*hfp at large b and hfp at small b and small (20%)
        //  penalty is applied if slat angle is not zero or +/- 90 degrees
        //  Note:  influence of distance is lost if delta-T goes to zero
        //  Note:  as delta-T -> zero, Rabga->0, SLtoAmb -> hfp, not 2hfp,
        //        for any spacing, even large b.  This is a problem

    } else if (scheme == 2) {
        // properties of AIR
        Tavg = (Ts + Tamb) / 2.0;
        rho = state.dataWindowEquivalentLayer->PAtmSeaLevel / (287.097 * Tavg); // density (kg/m3) <- temperature in (K)
        beta = 1.0 / Tavg;                                                      // thermal expansion coeff (/K)
        dvisc = (18.05 + ((Tavg - 290.0) / 10.0) * (18.53 - 18.05)) * 1.0e-6;
        //  dynamic viscosity (kg/m.sec) or (N.sec/m2)
        Cp = 1044.66 - 0.31597 * Tavg + 0.000707908 * pow_2(Tavg) - 0.00000027034 * pow_3(Tavg);
        //  specific heat at constant pressure (J/kg.K)
        k = 0.02538 + ((Tavg - 290.0) / 10.0) * (0.02614 - 0.02538); // conductivity (W/m.K)

        Rabsa = (9.81 * beta * pow_3(b) * std::abs(Ts - Tamb) * pow_2(rho) * Cp) / (dvisc * k);
        if (Rabsa <= 1.0) {
            Rabsa = 1.0;
        }

        hfp = hc_in; // h - flat plate - from calling routine
        // Note:  using this approach, L no longer has influence on hfp

        // now adjust for distance from window glass
        SLtoAMB = hfp * (1.0 + std::exp(-6000.0 / Rabsa));
        // Note:  as delta-T -> zero, Rabga->0, SLtoAmb -> hfp, not 2hfp,
        //        for any spacing, even large b.  This is a problem

    } else if (scheme == 3) {

        hfp = hc_in; // h - flat plate - from calling routine
        // now adjust for distance from window glass
        SLtoAMB = hfp * (2.0 - std::exp(-4.6 * b / 0.1));
        // Note:  using this approach, L and temperatures no longer have
        //                               influence on result
        //  SLtoAmb = 2*hc_in when glass/shade spacing, b, is large
        //  SLtoAmb = hc_in when glass/shade spacing, b, is zero
        //  The exponential decay is 99% complete at b=4 inches = 0.1 m
        //                                               ln(0.01) = -4.6
        //  This coefficient could be fine tuned in future versions, perhaps
        //  as a function of boundary layer thickness for specific values
        //  of glass and shade temperatures
    } //  end of scheme .eq. 3
    return SLtoAMB;
}

void GLtoAMB(EnergyPlusData const &state,
             Real64 const b,     // distance from shade to glass {m}
             Real64 const L,     // window height {m}, usually taken as 1 meter
             Real64 const Tg,    // glass temperature {K}
             Real64 const Tamb,  // room air temperature, {K}
             Real64 const hc_in, // inside convection coefficient, {W/m2K}
             Real64 &hgamb,      // glass to room air heat transfer coefficient
             int const scheme)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         John L. Wright, University of Waterloo,
    //                      Mechanical Engineering, Advanced Glazing System Laboratory

    // PURPOSE OF THIS SUBROUTINE:
    //  Returns the glass to room air heat transfer coefficient
    // METHODOLOGY EMPLOYED:
    // scheme = flag to select model, scheme=2 has problems, scheme=3 recommended
    // fill gas is always air, orientation is always vertical
    // hgamb should be zero at b=0, h-flatplate at b=large

    Real64 Tavg;
    Real64 rho;
    Real64 beta;
    Real64 dvisc;
    Real64 Cp;
    Real64 k;
    Real64 Rabga;
    Real64 hfp;

    hgamb = hc_in; // default - good for large glass/shade spacing

    if (scheme == 1) { //  Collins

        Tavg = (Tg + Tamb) / 2.0; // T for properties calculations

        // properties of AIR
        rho = state.dataWindowEquivalentLayer->PAtmSeaLevel / (287.097 * Tavg); // density (kg/m3) <- temperature in (K)
        beta = 1.0 / Tavg;                                                      // thermal expansion coeff (/K)
        dvisc = (18.05 + ((Tavg - 290.0) / 10.0) * (18.53 - 18.05)) * 1.0e-6;
        //  dynamic viscosity (kg/m.sec) or (N.sec/m2)
        Cp = 1044.66 - 0.31597 * Tavg + 0.000707908 * pow_2(Tavg) - 0.00000027034 * pow_3(Tavg);
        //  specific heat at constant pressure (J/kg.K)
        k = 0.02538 + ((Tavg - 290.0) / 10.0) * (0.02614 - 0.02538); // conductivity (W/m.K)

        Rabga = (9.81 * beta * pow_3(b) * std::abs(Tg - Tamb) * pow_2(rho) * Cp) / (dvisc * k);
        if (Rabga <= 1.0) {
            Rabga = 1.0;
        }

        hfp = HIC_ASHRAE(L, Tg, Tamb); // h - flat plate
        // Note:  as delta-T goes to zero, hfp will also go to zero

        hgamb = hfp * std::exp(-50.0 / Rabga);
        // Note:  as delta-T -> zero, Rabga->0, hgamb -> zero too
        //        for any spacing, even large b.  This is a problem

    } else if (scheme == 2) {

        Tavg = (Tg + Tamb) / 2.0; // T for properties calculations

        // properties of AIR
        rho = state.dataWindowEquivalentLayer->PAtmSeaLevel / (287.097 * Tavg); // density (kg/m3) <- temperature in (K)
        beta = 1.0 / Tavg;                                                      // thermal expansion coeff (/K)
        dvisc = (18.05 + ((Tavg - 290.0) / 10.0) * (18.53 - 18.05)) * 1.0e-6;
        //  dynamic viscosity (kg/m.sec) or (N.sec/m2)
        Cp = 1044.66 - 0.31597 * Tavg + 0.000707908 * pow_2(Tavg) - 0.00000027034 * pow_3(Tavg);
        //  specific heat at constant pressure (J/kg.K)
        k = 0.02538 + ((Tavg - 290.0) / 10.0) * (0.02614 - 0.02538); // conductivity (W/m.K)

        Rabga = (9.81 * beta * pow_3(b) * std::abs(Tg - Tamb) * pow_2(rho) * Cp) / (dvisc * k);
        if (Rabga <= 1.0) {
            Rabga = 1.0;
        }

        hfp = hc_in; // h - flat plate - from calling routine
        // Note:  using this approach, L no longer has influence on result
        //       but temperature does and it will drive hgamb to zero when
        //       the temperature difference goes to zero

        hgamb = hfp * std::exp(-50.0 / Rabga);
        // Note:  as delta-T -> zero, Rabga->0, hgamb -> zero too
        //        for any spacing, even large b.  This is a problem

    } else if (scheme == 3) {

        hfp = hc_in; // h - flat plate - from calling routine
        hgamb = hfp * (1.0 - std::exp(-4.6 * b / 0.1));
        // Note:  using this approach, L and temperatures no longer have
        //                               influence on result
        //  hgamb = hc_in when glass/shade spacing, b, is large
        //  hgamb = zero  when glass/shade spacing, b, is zero
        //  The exponential decay is 99% complete at b=4 inches = 0.1 m
        //                                               ln(0.01) = -4.6
        //  This coefficient could be fine tuned in future versions, perhaps
        //  as a function of boundary layer thickness for specific values
        //  of glass and shade temperatures

    } //  end of scheme .eq. 3
}

Real64 ConvectionFactor(CFSLAYER const &L) // window layer
{
    //       AUTHOR         ASHRAE 1311-RP
    // PURPOSE OF THIS FUNCTION:
    //  Modifies convection rate per shade configuration, layer convection enhancement

    if (L.LTYPE == LayerType::VBHOR) {
        // horiz VB: enhanced convection at +/- 45 due to "pumping"
        Real64 SlatADeg = min(90.0, std::abs(L.PHI_DEG));
        return 1.0 + 0.2 * std::sin(2.0 * SlatADeg);
    } else {
        return 1.0;
    }
}

bool CFSUFactor(EnergyPlusData &state,
                CFSTY const &FS,    // fenestration system
                Real64 const TOUT,  // outdoor temperature, C (air and MRT)
                Real64 const HCOUT, // outdoor convective coefficient, W/m2-K
                Real64 const TIN,   // indoor air temperature, C
                Real64 const HCIN,  // indoor convective coefficient, W/m2-K
                Real64 &U           // returned: U factor, W/m2-K
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         unknown (University of WaterLoo, ASHRAE 1311-RP)
    //       MODIFIED       Bereket Nigusse, FSEC/UCF, June 2013

    // PURPOSE OF THIS FUNCTION:
    // ! returns .TRUE. if the U-value calculation succeeded, .FALSE. if error

    // METHODOLOGY EMPLOYED:
    //  uses net radiation method to solve for window surface temperatures and
    //  heat fluxes. Then calculates the U-value from the flux and over all
    //  temperature difference.

    // REFERENCES:
    //  ASHRAE 1311-RP

    // Return value
    bool CFSUFactor;

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:
    // for conditions specified (no incident solar)
    // FUNCTION PARAMETER DEFINITIONS:
    Real64 constexpr TOL(0.01); // 0.0001d0

    int NL;
    Real64 TOABS;
    Real64 TRMOUT;
    Real64 TIABS;
    Real64 TRMIN;
    Array1D<Real64> QOCF(FS.NL);
    Real64 QOCFRoom;
    Array1D<Real64> JB({0, FS.NL});
    Array1D<Real64> JF({1, FS.NL + 1});
    Array1D<Real64> T(FS.NL);
    Array1D<Real64> Q({0, FS.NL});
    Array1D<Real64> H({0, FS.NL + 1});
    Array1D<Real64> SOURCE(FS.NL + 1);
    Real64 ISOL;
    Real64 SHGC;

    CFSUFactor = false;
    if (std::abs(TOUT - TIN) < 0.01) {
        U = -1.0;
        return CFSUFactor;
    }

    TOABS = TOUT + Constant::Kelvin;
    TRMOUT = TOABS;
    TIABS = TIN + Constant::Kelvin;
    TRMIN = TIABS;

    NL = FS.NL;
    ISOL = 0.0; // no solar winter condition
    SOURCE = 0.0;

    CFSUFactor = ASHWAT_ThermalRatings(
        state, FS, TIABS, TOABS, HCIN, HCOUT, TRMOUT, TRMIN, ISOL, SOURCE({1, NL + 1}), TOL, QOCF, QOCFRoom, T, Q, JF, JB, H, U, SHGC, true);

    return CFSUFactor;
}

void ASHWAT_Solar(int const NL,                                 // # of layers
                  Array1S<CFSSWP> const LSWP_ON,                // layer SW (solar) properties (off-normal adjusted)
                  CFSSWP const &SWP_ROOM,                       // effective SW (solar) properties of room
                  Real64 const IBEAM,                           // incident beam insolation (W/m2 aperture)
                  Real64 const IDIFF,                           // incident diffuse insolation (W/m2 aperture)
                  Real64 const ILIGHTS,                         // incident diffuse insolation (W/m2 aperture)
                  Array1S<Real64> SOURCE,                       // returned: layer-by-layer flux of absorbed
                  ObjexxFCL::Optional<Array1S<Real64>> SourceBD // returned: layer-by-layer flux of absorbed
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT and NATHAN KOTEY,
    //       DATE WRITTEN   June, 2006
    //       MODIFIED       Bereket Nigusse, JUNE 2013
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Returns the optical properties of multi-layer fenestration system model given optical
    // properties of the layers
    // METHODOLOGY EMPLOYED:
    //    Use combination net radiation method and TDMA solver
    // REFERENCES:
    //  JOHN L. WRIGHT and NATHAN KOTEY (2006). Solar Absorption By each Element in a Glazing/Shading
    //   Layer Array, ASHRAE Transactions, Vol. 112, Pt. 2. pp. 3-12.
    //   University of Waterloo, Mechanical Engineering
    //   Advanced Glazing System Laboratory

    // SUBROUTINE ARGUMENT DEFINITIONS:
    //   1=outside .. NL=inside
    //   generally black or minimally reflective
    //     on inside surface (e.g., from lights)
    //  solar radiation (beam-beam + beam-diffuse) (W/m2)
    // SOURCE(NL+1) is the flux of solar radiation
    //  absorbed in conditioned space (W/m2 aperture area)
    //  beam-diffuse solar radiation (W/m2)
    // SOURCE_BD(NL+1) is the flux of beam-diffuse solar radiation
    //  absorbed in conditioned space (W/m2 aperture area)
    // or this beam-diffuse solar transmittance of the system

    Array1D<Real64> BPLUS({0, NL}); // beam solar fluxes flowing in outward and inward directions
    Array1D<Real64> BMINUS({0, NL});
    //   correspond to Edwards QPLUS and QMINUS (except note
    //   reverse layer numbering)
    Array1D<Real64> CPLUS({0, NL}); // diffuse solar fluxes caused by BPLUS and BMINUS;
    Array1D<Real64> CMINUS({0, NL});
    //   appear as sources in diffuse calculation
    Array1D<Real64> DPLUS({0, NL}); // diffuse solar fluxes flowing in outward and inward
    Array1D<Real64> DMINUS({0, NL});
    //   directions (W/m2)
    Array1D<Real64> AP(2 * NL);
    Array1D<Real64> AE(2 * NL);
    Array1D<Real64> AW(2 * NL);
    Array1D<Real64> BP(2 * NL);
    Array1D<Real64> X(2 * NL);
    Real64 CHKSUM;
    Array1D<Real64> BeamDiffuseAbs(NL + 1); // beam-diffuse absorbed fraction of beam radiation (W/m2)
    int N_TDMA;
    int I;
    int LINE;

    if (NL < 1) {
        return;
    }

    //  STEP ONE: THE BEAM-BEAM ANALYSIS TO FIND BPLUS AND BMINUS
    NETRAD(NL, LSWP_ON, SWP_ROOM.RHOSFBB, IBEAM, BPLUS, BMINUS);

    //  STEP TWO: CALCULATE THE DIFFUSE-CAUSED-BY-BEAM SOURCES CPLUS AND CMINUS
    CPLUS(NL) = SWP_ROOM.RHOSFBD * BMINUS(NL);
    for (I = NL; I >= 1; --I) { // March through layers, indoor to outdoor
        CPLUS(I - 1) = LSWP_ON(I).RHOSFBD * BMINUS(I - 1) + LSWP_ON(I).TAUSBBD * BPLUS(I);
        CMINUS(I) = LSWP_ON(I).RHOSBBD * BPLUS(I) + LSWP_ON(I).TAUSFBD * BMINUS(I - 1);
    }
    CMINUS(0) = 0.0;

    //  STEP THREE: DIFFUSE FLUXES, DPLUS AND DMINUS,
    //  CAUSED BY DIFFUSE INCIDENT, IDIFF ON THE OUTDOOR SIDE
    //  AND BY ILIGHTS ON THE INDOOR SIDE, AND BY
    //  DIFFUSE SOURCE (FROM BEAM) FLUXES, CPLUS AND CMINUS

    N_TDMA = 2 * NL;

    for (I = 1; I <= NL; ++I) {
        LINE = (2 * I) - 1;
        AP(LINE) = LSWP_ON(I).RHOSBDD;
        AE(LINE) = 1.0;
        if (LINE != 1) { // default
            AW(LINE) = -1.0 * LSWP_ON(I).TAUS_DD;
            BP(LINE) = -1.0 * CMINUS(I);
        } else { //  special case at west-most node
            AW(1) = 0.0;
            BP(1) = -1.0 * LSWP_ON(1).TAUS_DD * IDIFF - CMINUS(1);
        }

        LINE = (2 * I);
        AW(LINE) = 1.0;
        if (LINE != N_TDMA) { // default
            AP(LINE) = LSWP_ON(I + 1).RHOSFDD;
            AE(LINE) = -1.0 * LSWP_ON(I + 1).TAUS_DD;
            BP(LINE) = -1.0 * CPLUS(I);
        } else { //  special case at east-most node
            AP(LINE) = SWP_ROOM.RHOSFDD;
            BP(N_TDMA) = -1.0 * (CPLUS(NL) + ILIGHTS);
            AE(N_TDMA) = 0.0;
        }
    }

    AUTOTDMA(X, AP, AE, AW, BP, N_TDMA);

    //   UNPACK TDMA SOLUTION VECTOR
    for (I = 1; I <= NL; ++I) {
        LINE = (2 * I) - 1;
        DPLUS(I) = X(LINE);
        LINE = (2 * I);
        DMINUS(I) = X(LINE);
    }

    //  Finish up diffuse calculations
    DMINUS(0) = IDIFF;
    DPLUS(0) = LSWP_ON(1).RHOSFDD * DMINUS(0) + LSWP_ON(1).TAUS_DD * DPLUS(1) + CPLUS(0);

    //  STEP FOUR: ABSORBED SOLAR RADIATION AT EACH LAYER/NODE
    SOURCE = 0.0;
    SOURCE(NL + 1) = BMINUS(NL) - BPLUS(NL) + DMINUS(NL) - DPLUS(NL) + ILIGHTS; // SOLAR FLUX | TRANSMITTED TO | ROOM

    //  NOTE:  In calculating SOURCE(room) there is a trick included in the
    //         previous line:  ILIGHTS is added because it is included
    //         in DPLUS(NL) but ILIGHTS should not be included in this
    //         type of calculation of SOURCE(i).  No similar adjustment
    //         is needed for any of the other values of SOURCE(i)
    //         As an alternative get the same result using:
    //     SOURCE(NL+1) = BMINUS(NL)*(1.0 - SWP_ROOM%RHOSFBB - SWP_ROOM%RHOSFBD) +
    //    &               DMINUS(NL)*(1.0 - SWP_ROOM%RHOSFDD)
    //         Take your pick

    // Added by BAN, June 7, 2013 to extract the beam-diffuse component for use
    // in the EnergyPLus heat balance.  EnergyPlus requires the beam-beam and
    // Beam-diffuse components separately.
    BeamDiffuseAbs = 0.0;
    BeamDiffuseAbs(NL + 1) = DMINUS(NL) - DPLUS(NL); // beam-diffuse transmitted to the room
    for (I = 1; I <= NL; ++I) {
        SOURCE(I) = BPLUS(I) - BMINUS(I) - BPLUS(I - 1) + BMINUS(I - 1) + DPLUS(I) - DMINUS(I) - DPLUS(I - 1) + DMINUS(I - 1);
        // Added by BAN June 7, 2013
        BeamDiffuseAbs(I) = 0.0;
    }

    if (present(SourceBD)) {
        SourceBD = BeamDiffuseAbs;
    }
    //  CHECKSUM - ALL INCOMING SOLAR FLUX MUST GO SOMEWHERE, SHOULD EQUAL ZERO
    CHKSUM = IBEAM + IDIFF + ILIGHTS - BPLUS(0) - DPLUS(0);
    for (I = 1; I <= NL + 1; ++I) {
        CHKSUM -= SOURCE(I);
    }
}

void NETRAD(int const NL,                  // # of layers, 1=outside .. NL=inside
            Array1S<CFSSWP> const LSWP_ON, // layer SW (solar) properties (off-normal adjusted)
            Real64 const RHO_room,         // effective solar reflectance of room (at inside)
            Real64 const ISOL,             // incident flux (W/m2)
            Array1D<Real64> &QPLUS,        // returned: see Edwards paper
            Array1D<Real64> &QMINUS        // returned: see Edwards paper
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  Autodesk:F2C++ Reworked to avoid complex member array usage

    // PURPOSE OF THIS SUBROUTINE:
    // Returns the solar radiant fluxes between glazing layers
    // METHODOLOGY EMPLOYED:
    //  Net Radiation Method by LARGELY EDWARDS
    //  TED, RED, QPLUS, QMINUS correspond to variables found in "Edwards"
    //  but with reversed layers order indexing (layer 1=outside .. NL=inside)
    //  GAP I is between layer I and I+1

    if (NL < 1) {
        return;
    }

    Array1D<Real64> TED(NL + 1);
    Array1D<Real64> RED(NL + 1);

    //   Reflectance and Transmittance

    RED(NL + 1) = RHO_room;
    TED(NL + 1) = 0.0;
    for (int i = NL; i >= 1; --i) {
        CFSSWP const &LSWP_ON_i(LSWP_ON(i));
        TED(i) = LSWP_ON_i.TAUSFBB / max(0.00001, 1.0 - LSWP_ON_i.RHOSBBB * RED(i + 1));
        RED(i) = LSWP_ON_i.RHOSBBB + TED(i) * LSWP_ON_i.TAUSBBB * RED(i + 1);
    }

    //   Outward and Inward Solar Fluxes, QPLUS AND QMINUS, Respectively
    QMINUS(0) = ISOL;
    QPLUS(0) = QMINUS(0) * RED(1);
    for (int i = 1; i <= NL; ++i) {
        QMINUS(i) = QMINUS(i - 1) * TED(i);
        QPLUS(i) = QMINUS(i) * RED(i + 1);
    }
}

void TDMA_R(
    Array1D<Real64> &X, const Array1D<Real64> &AP, const Array1D<Real64> &AE, const Array1D<Real64> &AW, const Array1D<Real64> &BP, int const N)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // TDMA solver
    // METHODOLOGY EMPLOYED:
    //  1-D TDMA reverse solver. East/West sweep followed by West/East sweep

    int J;
    Array1D<Real64> ALPHA(N);
    Array1D<Real64> BETA(N);

    ALPHA(N) = AW(N) / AP(N);
    BETA(N) = BP(N) / AP(N);

    for (J = N - 1; J >= 1; --J) {
        ALPHA(J) = AW(J) / (AP(J) - (ALPHA(J + 1) * AE(J)));
        BETA(J) = ((AE(J) * BETA(J + 1)) + BP(J)) / (AP(J) - (ALPHA(J + 1) * AE(J)));
    }

    X(1) = BETA(1);
    for (J = 2; J <= N; ++J) {
        X(J) = (ALPHA(J) * X(J - 1)) + BETA(J);
    }
}

void TDMA(Array1D<Real64> &X, const Array1D<Real64> &AP, const Array1D<Real64> &AE, const Array1D<Real64> &AW, const Array1D<Real64> &BP, int const N)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Matrix solver
    // METHODOLOGY EMPLOYED:
    //  1-D TDMA solver.

    int J;
    Array1D<Real64> ALPHA(N);
    Array1D<Real64> BETA(N);
    Real64 D;

    ALPHA(1) = AE(1) / AP(1);
    BETA(1) = BP(1) / AP(1);

    for (J = 2; J <= N; ++J) {
        D = AP(J) - (ALPHA(J - 1) * AW(J));
        if (std::abs(D) < 0.0001) {
            ALPHA(J) = 0.0;
            BETA(J) = 0.0;
        } else {
            ALPHA(J) = AE(J) / D;
            BETA(J) = ((AW(J) * BETA(J - 1)) + BP(J)) / D;
        }
    }

    X(N) = BETA(N);
    for (J = N - 1; J >= 1; --J) {
        X(J) = (ALPHA(J) * X(J + 1)) + BETA(J);
    }
}

void AUTOTDMA(Array1D<Real64> &X, Array1D<Real64> &AP, const Array1D<Real64> &AE, const Array1D<Real64> &AW, const Array1D<Real64> &BP, int &N)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Matrix solver manager routine
    // METHODOLOGY EMPLOYED:
    //  1-D TDMA solver.

    //  Call TDMA for forward (i.e., west-to-east and back) calculation
    //  or TDMA_R for reverse (i.e., east-to-west and back) calculation
    //      TDMA   won't tolerate RHOSFxx(1)=0   (i.e., ap(1)=0)
    //  but TDMA_R won't tolerate RHOSBxx(N-1)=0 (i.e., ap(n)=0)
    //  where n-1 refers to the outdoor layer (glazing or shading layer)

    //  This if-statement will catch the situation where RHOSFxx(1)=0.
    //  i.e., AP(1)=0.

    if (AP(1) < AP(N)) {
        TDMA_R(X, AP, AE, AW, BP, N);
    } else {
        //  This "fix" (on the next line) is only used as a last resort
        //  The if-statement will catch the very unusual situation where both
        //  RHOSBxx(N-1)=0.   AND     RHOSFxx(1)=0.
        if (AP(1) < 0.0001) {
            AP(1) = 0.0001;
        }
        TDMA(X, AP, AE, AW, BP, N);
    }
}

void ASHWAT_OffNormalProperties(EnergyPlusData &state,
                                CFSLAYER const &L,    // layer for which to derive off-normal properties
                                Real64 const THETA,   // solar beam angle of incidence, from normal, radians
                                Real64 const OMEGA_V, // solar beam vertical profile angle, +=above horizontal, radians
                                Real64 const OMEGA_H, // solar beam horizontal profile angle, +=clockwise when viewed
                                CFSSWP &LSWP_ON       // returned: off-normal properties
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Returns off-normal properties (total solar, beam-beam and beam diffuse) given
    // direct-normal, total solar, beam-beam and beam diffuse properties of layers

    // SUBROUTINE ARGUMENT DEFINITIONS:
    //   Used: LTYPE, SWP_EL, geometry
    //   Note: not altered (return is in LSWP_ON)
    //    0 <= THETA <= PI/2
    //   = solar elevation angle for a vertical wall with
    //     wall-solar azimuth angle equal to zero
    //   from above (radians)
    //   = wall-solar azimuth angle for a vertical wall
    //     Used for PD and vertical VB

    LSWP_ON = L.SWP_EL; // init to normal properties
    //  calls below modify in place

    if (IsGlazeLayerX(L)) {
        // specular glazing
        // HBX note: ltyGZS here iff modelOption F=x; spectral cases elsewhere
        Specular_SWP(LSWP_ON, THETA);
    } else if (L.LTYPE == LayerType::VBHOR) {
        VB_SWP(state, L, LSWP_ON, OMEGA_V);
    } else if (L.LTYPE == LayerType::VBVER) {
        VB_SWP(state, L, LSWP_ON, OMEGA_H);
    } else if (L.LTYPE == LayerType::DRAPE) {
        PD_SWP(state, L, LSWP_ON, OMEGA_V, OMEGA_H);
    } else if (L.LTYPE == LayerType::ROLLB) {
        RB_SWP(state, L, LSWP_ON, THETA);
    } else if (L.LTYPE == LayerType::INSCRN) {
        IS_SWP(state, L, LSWP_ON, THETA);
    } else if (L.LTYPE == LayerType::NONE || L.LTYPE == LayerType::ROOM) {
        // none or room: do nothing
    } else {
        // placeholder for add'l non-specular layers
    }
}

bool Specular_OffNormal(Real64 const THETA, // solar beam angle of incidence, from normal radians
                        Real64 &RAT_1MR,    // returned: ratio of off-normal to normal solar (1-reflectance)
                        Real64 &RAT_TAU     // returned: ratio of off-normal to normal solar transmittance
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Returns ratio of off-normal to normal of optical properties.
    // METHODOLOGY EMPLOYED:
    //  Uses a reference glass property.
    // returns TRUE if RAT_TAU < 1 or RAT_1MR < 1 (and thus Specular_Adjust s/b called)
    //    else FALSE
    // Return value
    bool Specular_OffNormal;

    // FUNCTION ARGUMENT DEFINITIONS:
    //    0 <= THETA <= PI/2
    //   NOTE: rhoAdj = 1-(1-rho)*RAT_1MR

    Real64 TAU0;
    Real64 RHO0;
    Real64 THETA1;
    Real64 THETA2;
    Real64 TAU_ON;
    Real64 RHO_ON;
    Real64 TAU_A;
    Real64 RPERP; // interface reflectance with respect to perpendicular
    Real64 RPARL;
    // and parallel polarization components of solar radiation
    Real64 TAUPERP;
    Real64 TAUPARL;
    Real64 RHOPERP;
    Real64 RHOPARL;
    Real64 N2; // reference refractive index for generating general off-normal
    //  curves for specular glazings
    Real64 KL; // extinction coefficient - thickness product, also used as a
    //  reference value to generate off-normal curves for specular layers

    Specular_OffNormal = true;
    THETA1 = std::abs(THETA);
    if (THETA1 > Constant::PiOvr2 - Constant::DegToRad) {
        // theta > 89 deg
        RAT_TAU = 0.0;
        RAT_1MR = 0.0;
    } else if (THETA1 >= Constant::DegToRad) {
        // theta >= 1 deg
        N2 = 1.526;
        KL = 55.0 * 0.006;
        TAU_A = std::exp(-1.0 * KL);
        RPERP = pow_2((N2 - 1.0) / (N2 + 1.0));
        TAU0 = TAU_A * (1.0 - RPERP) * (1.0 - RPERP) / (1.0 - (RPERP * RPERP * TAU_A * TAU_A));
        RHO0 = RPERP * (1.0 + (TAU_A * TAU0));
        THETA2 = std::asin((std::sin(THETA1)) / N2);
        TAU_A = std::exp(-1.0 * KL / std::cos(THETA2));
        RPERP = pow_2(std::sin(THETA2 - THETA1) / std::sin(THETA2 + THETA1));
        RPARL = pow_2(std::tan(THETA2 - THETA1) / std::tan(THETA2 + THETA1));
        TAUPERP = TAU_A * (1.0 - RPERP) * (1.0 - RPERP) / (1.0 - (RPERP * RPERP * TAU_A * TAU_A));
        TAUPARL = TAU_A * (1.0 - RPARL) * (1.0 - RPARL) / (1.0 - (RPARL * RPARL * TAU_A * TAU_A));
        RHOPERP = RPERP * (1.0 + (TAU_A * TAUPERP));
        RHOPARL = RPARL * (1.0 + (TAU_A * TAUPARL));
        TAU_ON = (TAUPERP + TAUPARL) / 2.0;
        RHO_ON = (RHOPERP + RHOPARL) / 2.0;
        RAT_TAU = TAU_ON / TAU0;
        RAT_1MR = (1.0 - RHO_ON) / (1.0 - RHO0);
    } else {
        Specular_OffNormal = false;
        RAT_TAU = 1.0;
        RAT_1MR = 1.0;
    }
    return Specular_OffNormal;
}

void Specular_SWP(CFSSWP &SWP,       // short wave properties (adjusted in place)
                  Real64 const OMEGA // incident angle, radians
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Manages the off-normal solar properties calculation

    Real64 RAT_1MR; // adjustment factors, see Specular_OffNormal()
    Real64 RAT_TAU; // adjustment factors, see Specular_OffNormal()

    bool Specular_OffNormalReturn = Specular_OffNormal(OMEGA, RAT_1MR, RAT_TAU);
    if (Specular_OffNormalReturn) {
        Specular_Adjust(SWP, RAT_1MR, RAT_TAU);
    }
}

void Specular_Adjust(CFSSWP &SWP,          // short wave properties (adjusted in place)
                     Real64 const RAT_1MR, // adjustment factors, see Specular_OffNormal()
                     Real64 const RAT_TAU  // adjustment factors, see Specular_OffNormal()
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // adjusts the off-normal solar properties

    SWP.TAUSFBB *= RAT_TAU;
    SWP.TAUSBBB *= RAT_TAU;
    SWP.RHOSFBB = 1.0 - RAT_1MR * (1.0 - SWP.RHOSFBB);
    SWP.RHOSBBB = 1.0 - RAT_1MR * (1.0 - SWP.RHOSBBB);
}

void Specular_RATDiff(EnergyPlusData &state, Real64 &RAT_1MRDiff, Real64 &RAT_TAUDiff)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //  Returns property ratios for estimating diffuse properties.

    Array1D<Real64> P(state.dataWindowEquivalentLayer->hipDIM);

    if (state.dataWindowEquivalentLayer->XTAUDiff < 0.0) {
        // calculate and save on first call
        state.dataWindowEquivalentLayer->X1MRDiff = HEMINT(state, Specular_F, state.dataWindowEquivalentLayer->hipRHO, P);
        state.dataWindowEquivalentLayer->XTAUDiff = HEMINT(state, Specular_F, state.dataWindowEquivalentLayer->hipTAU, P);
    }
    RAT_TAUDiff = state.dataWindowEquivalentLayer->XTAUDiff;
    RAT_1MRDiff = state.dataWindowEquivalentLayer->X1MRDiff;
}

Real64 Specular_F(EnergyPlusData const &state,
                  Real64 const THETA,                       // incidence angle, radians
                  int const OPT,                            // options (unused)
                  [[maybe_unused]] const Array1D<Real64> &P // parameters (none defined)
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // integrand fcn for specular properties.

    // Return value
    Real64 Specular_F;

    // Argument array dimensioning
    // EP_SIZE_CHECK(P, hipDIM);

    // FUNCTION ARGUMENT DEFINITIONS:
    //   1: reflectance
    //   2: transmittance

    Real64 RAT_TAU;
    Real64 RAT_1MR;

    // Modified by BAN April 19, 2013
    Specular_OffNormal(THETA, RAT_1MR, RAT_TAU);

    if (OPT == state.dataWindowEquivalentLayer->hipRHO) {
        Specular_F = RAT_1MR;
    } else if (OPT == state.dataWindowEquivalentLayer->hipTAU) {
        Specular_F = RAT_TAU;
    } else {
        Specular_F = -1.0;
    }
    return Specular_F;
}

void Specular_EstimateDiffuseProps(EnergyPlusData &state, CFSSWP &SWP) // short wave properties
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Estimates diffuse-diffuse properties.

    Real64 RAT_TAU;
    Real64 RAT_1MR;

    // #if 1
    Specular_RATDiff(state, RAT_1MR, RAT_TAU);
    // #else
    //     ! estimate diffuse properties as 60 deg angle of incidence
    //     CALL Specular_RAT60( RAT_TAU, RAT_1MR)
    // #endif
    SWP.TAUS_DD = RAT_TAU * SWP.TAUSFBB;
    SWP.RHOSFDD = 1.0 - RAT_1MR * (1.0 - SWP.RHOSFBB);
    SWP.RHOSBDD = 1.0 - RAT_1MR * (1.0 - SWP.RHOSBBB);
}

bool RB_LWP(CFSLAYER const &L, // RB layer
            CFSLWP &LLWP       // returned: equivalent layer long wave properties
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Modifies roller blind longwave properties. If not roller blind layer
    // returns False.

    // Return value
    bool RB_LWP;

    Real64 TAULX;
    Real64 OPENNESS;

    RB_LWP = false;
    if (L.LTYPE != LayerType::ROLLB) {
        return RB_LWP;
    }

    OPENNESS = L.SWP_MAT.TAUSFBB;

    OPENNESS_LW(OPENNESS, L.LWP_MAT.EPSLF, L.LWP_MAT.TAUL, LLWP.EPSLF, LLWP.TAUL);

    OPENNESS_LW(OPENNESS, L.LWP_MAT.EPSLB, L.LWP_MAT.TAUL, LLWP.EPSLB, TAULX);

    RB_LWP = true;
    return RB_LWP;
}

bool RB_SWP(EnergyPlusData &state,
            CFSLAYER const &L, // RB layer
            CFSSWP &LSWP,      // returned: equivalent layer properties set
            const Real64 THETA // incident angle, 0 <= theta <= PI/2
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    //       DATE WRITTEN   unknown
    //       MODIFIED       Jason W. DeGraw 2023
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    //  Modifies roller blind shortwave properties. If not roller blind layer
    //  returns False.

    // FUNCTION ARGUMENT DEFINITIONS:
    //   sets ONLY RHOSFDD, RHOSBDD, TAUS_DD
    //  if missing, derive diffuse properties

    if (L.LTYPE != LayerType::ROLLB) {
        return false;
    }

    // normal beam-total properties of fabric
    Real64 RHOFF_BT0 = L.SWP_MAT.RHOSFBB + L.SWP_MAT.RHOSFBD; // front rho
    Real64 RHOBF_BT0 = L.SWP_MAT.RHOSBBB + L.SWP_MAT.RHOSBBD; // back rho

    Real64 TAUFF_BT0 = L.SWP_MAT.TAUSFBB + L.SWP_MAT.TAUSFBD; // front tau
    Real64 TAUBF_BT0 = L.SWP_MAT.TAUSBBB + L.SWP_MAT.TAUSBBD; // back tau

    RB_BEAM(state, THETA, RHOFF_BT0, TAUFF_BT0, L.SWP_MAT.TAUSFBB, LSWP.RHOSFBD, LSWP.TAUSFBB, LSWP.TAUSFBD);

    RB_BEAM(state, THETA, RHOBF_BT0, TAUBF_BT0, L.SWP_MAT.TAUSBBB, LSWP.RHOSBBD, LSWP.TAUSBBB, LSWP.TAUSBBD);

    return true;
}

bool RB_SWP(EnergyPlusData &state,
            CFSLAYER const &L, // RB layer
            CFSSWP &LSWP       // returned: equivalent layer properties set
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    //       DATE WRITTEN   unknown
    //       MODIFIED       Jason W. DeGraw 2023
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    //  Modifies roller blind shortwave properties. If not roller blind layer
    //  returns False.

    // FUNCTION ARGUMENT DEFINITIONS:
    //   sets ONLY RHOSFDD, RHOSBDD, TAUS_DD
    //  if missing, derive diffuse properties

    if (L.LTYPE != LayerType::ROLLB) {
        return false;
    }

    // normal beam-total properties of fabric
    Real64 RHOFF_BT0 = L.SWP_MAT.RHOSFBB + L.SWP_MAT.RHOSFBD; // front rho
    Real64 RHOBF_BT0 = L.SWP_MAT.RHOSBBB + L.SWP_MAT.RHOSBBD; // back rho

    Real64 TAUFF_BT0 = L.SWP_MAT.TAUSFBB + L.SWP_MAT.TAUSFBD; // front tau
    Real64 TAUBF_BT0 = L.SWP_MAT.TAUSBBB + L.SWP_MAT.TAUSBBD; // back tau

    // front
    RB_DIFF(state, RHOFF_BT0, TAUFF_BT0, L.SWP_MAT.TAUSFBB, LSWP.RHOSFDD, LSWP.TAUS_DD);
    // back
    Real64 TAUX; // This gets used as output of RB_DIFF and is then discarded
    RB_DIFF(state, RHOBF_BT0, TAUBF_BT0, L.SWP_MAT.TAUSBBB, LSWP.RHOSBDD, TAUX);

    return true;
}

bool IS_LWP(CFSLAYER const &L, // IS layer
            CFSLWP &LLWP       // returned: equivalent layer long wave properties
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    //  Modifies Insect Screen longwave properties. If not Insect Screen layer
    //  returns False.

    // Return value
    bool IS_LWP;

    Real64 OPENNESS;
    Real64 TAULX;

    IS_LWP = false;
    if (L.LTYPE != LayerType::INSCRN) {
        return IS_LWP;
    }

    OPENNESS = L.SWP_MAT.TAUSFBB;

    OPENNESS_LW(OPENNESS, L.LWP_MAT.EPSLF, L.LWP_MAT.TAUL, LLWP.EPSLF, LLWP.TAUL);

    OPENNESS_LW(OPENNESS, L.LWP_MAT.EPSLB, L.LWP_MAT.TAUL, LLWP.EPSLB, TAULX);
    IS_LWP = true;
    return IS_LWP;
}

bool IS_SWP(EnergyPlusData &state,
            CFSLAYER const &L, // PD layer
            CFSSWP &LSWP,      // returned: equivalent layer properties set
            const Real64 THETA // incident angle, 0 <= theta <= PI/2
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    //       DATE WRITTEN   unknown
    //       MODIFIED       Jason W. DeGraw 2023
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Modifies Insect Screen shortwave properties. If not Insect Screen layer
    // returns False.

    // FUNCTION ARGUMENT DEFINITIONS:
    //   sets ONLY RHOSFDD, RHOSBDD, TAUS_DD
    //  if missing, derive diffuse properties

    if (L.LTYPE != LayerType::INSCRN) {
        return false;
    }

    // normal beam-total properties
    Real64 RHOFF_BT0 = L.SWP_MAT.RHOSFBB + L.SWP_MAT.RHOSFBD; // front rho
    Real64 RHOBF_BT0 = L.SWP_MAT.RHOSBBB + L.SWP_MAT.RHOSBBD; // back rho

    Real64 TAUFF_BT0 = L.SWP_MAT.TAUSFBB + L.SWP_MAT.TAUSFBD; // front tau
    Real64 TAUBF_BT0 = L.SWP_MAT.TAUSBBB + L.SWP_MAT.TAUSBBD; // back tau

    // front
    IS_BEAM(state, THETA, RHOFF_BT0, TAUFF_BT0, L.SWP_MAT.TAUSFBB, LSWP.RHOSFBD, LSWP.TAUSFBB, LSWP.TAUSFBD);

    // back -- call with reverse material properties
    IS_BEAM(state, THETA, RHOBF_BT0, TAUBF_BT0, L.SWP_MAT.TAUSBBB, LSWP.RHOSBBD, LSWP.TAUSBBB, LSWP.TAUSBBD);

    return true;
}

bool IS_SWP(EnergyPlusData &state,
            CFSLAYER const &L, // PD layer
            CFSSWP &LSWP       // returned: equivalent layer properties set
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    //       DATE WRITTEN   unknown
    //       MODIFIED       Jason W. DeGraw 2023
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Modifies Insect Screen shortwave properties. If not Insect Screen layer
    // returns False.

    // FUNCTION ARGUMENT DEFINITIONS:
    //   sets ONLY RHOSFDD, RHOSBDD, TAUS_DD
    //  if missing, derive diffuse properties

    if (L.LTYPE != LayerType::INSCRN) {
        return false;
    }

    // normal beam-total properties
    Real64 RHOFF_BT0 = L.SWP_MAT.RHOSFBB + L.SWP_MAT.RHOSFBD; // front rho
    Real64 RHOBF_BT0 = L.SWP_MAT.RHOSBBB + L.SWP_MAT.RHOSBBD; // back rho

    Real64 TAUFF_BT0 = L.SWP_MAT.TAUSFBB + L.SWP_MAT.TAUSFBD; // front tau
    Real64 TAUBF_BT0 = L.SWP_MAT.TAUSBBB + L.SWP_MAT.TAUSBBD; // back tau

    // front
    IS_DIFF(state, RHOFF_BT0, TAUFF_BT0, L.SWP_MAT.TAUSFBB, LSWP.RHOSFDD, LSWP.TAUS_DD);
    // back
    Real64 TAUX; // This gets used as output of IS_DIFF and is then discarded
    IS_DIFF(state, RHOBF_BT0, TAUBF_BT0, L.SWP_MAT.TAUSBBB, LSWP.RHOSBDD, TAUX);

    return true;
}

void Fabric_EstimateDiffuseProps(EnergyPlusData &state, CFSSWP &SWP) // fabric short wave properties
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Estimates diffuse properties of drape fabrics.
    // sets RHOSFDD, RHOSBDD, TAUS_DD

    Real64 RHOBF_BT0; // total back reflectance
    Real64 RHOFF_BT0; // total front reflectance
    Real64 TAUBF_BT0; // total back transmittance
    Real64 TAUFF_BT0; // total front transmittance
    Real64 TAUX;

    RHOFF_BT0 = SWP.RHOSFBB + SWP.RHOSFBD; // front rho
    RHOBF_BT0 = SWP.RHOSBBB + SWP.RHOSBBD; // back rho
    TAUFF_BT0 = SWP.TAUSFBB + SWP.TAUSFBD; // front tau
    TAUBF_BT0 = SWP.TAUSBBB + SWP.TAUSBBD; // back tau
    FM_DIFF(state, RHOFF_BT0, TAUFF_BT0, SWP.TAUSFBB, SWP.RHOSFDD, SWP.TAUS_DD);
    FM_DIFF(state, RHOBF_BT0, TAUBF_BT0, SWP.TAUSBBB, SWP.RHOSBDD, TAUX);
}

bool PD_LWP(EnergyPlusData &state,
            CFSLAYER const &L, // PD layer
            CFSLWP &LLWP       // returned: equivalent layer long wave properties
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    //  Modifies Drape longwave properties for openness. If not Drape Fabric layer
    //  returns False.

    // Return value
    bool PD_LWP;

    Real64 TAULX;
    Real64 OPENNESS_FABRIC;

    PD_LWP = false;
    if (L.LTYPE != LayerType::DRAPE) {
        return PD_LWP;
    }

    OPENNESS_FABRIC = L.SWP_MAT.TAUSFBB;

    PD_LW(state, L.S, L.W, OPENNESS_FABRIC, L.LWP_MAT.EPSLF, L.LWP_MAT.EPSLB, L.LWP_MAT.TAUL, LLWP.EPSLF, LLWP.TAUL);

    PD_LW(state, L.S, L.W, OPENNESS_FABRIC, L.LWP_MAT.EPSLB, L.LWP_MAT.EPSLF, L.LWP_MAT.TAUL, LLWP.EPSLB, TAULX);

    PD_LWP = true;
    return PD_LWP;
}

bool PD_SWP(EnergyPlusData &state,
            CFSLAYER const &L,      // PD layer
            CFSSWP &LSWP,           // returned: equivalent layer properties set
            const Real64 OHM_V_RAD, // vertical VB profile angles, radians
            const Real64 OHM_H_RAD  // horizontal VB profile angles, radians
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    //       DATE WRITTEN   unknown
    //       MODIFIED       Jason W. DeGraw 2023
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Modifies drape fabric shortwave properties for openness. If not drape Fabric layer
    // returns false. If profile angles not specified diffuse properties are returned.

    if (!(L.LTYPE == LayerType::DRAPE)) {
        return false;
    }

    // normal beam-total properties of fabric
    Real64 RHOFF_BT0 = L.SWP_MAT.RHOSFBB + L.SWP_MAT.RHOSFBD; // front rho
    Real64 RHOBF_BT0 = L.SWP_MAT.RHOSBBB + L.SWP_MAT.RHOSBBD; // back rho

    // drape front properties
    PD_BEAM(state,
            L.S,
            L.W,
            OHM_V_RAD,
            OHM_H_RAD,
            RHOFF_BT0,
            L.SWP_MAT.TAUSFBB,
            L.SWP_MAT.TAUSFBD,
            L.SWP_MAT.RHOSFDD,
            L.SWP_MAT.TAUS_DD,
            RHOBF_BT0,
            L.SWP_MAT.TAUSBBB,
            L.SWP_MAT.TAUSBBD,
            L.SWP_MAT.RHOSBDD,
            L.SWP_MAT.TAUS_DD,
            LSWP.RHOSFBD,
            LSWP.TAUSFBB,
            LSWP.TAUSFBD);

    // drape back properties: call with reversed fabric properties
    PD_BEAM(state,
            L.S,
            L.W,
            OHM_V_RAD,
            OHM_H_RAD,
            RHOBF_BT0,
            L.SWP_MAT.TAUSBBB,
            L.SWP_MAT.TAUSBBD,
            L.SWP_MAT.RHOSBDD,
            L.SWP_MAT.TAUS_DD,
            RHOFF_BT0,
            L.SWP_MAT.TAUSFBB,
            L.SWP_MAT.TAUSFBD,
            L.SWP_MAT.RHOSFDD,
            L.SWP_MAT.TAUS_DD,
            LSWP.RHOSBBD,
            LSWP.TAUSBBB,
            LSWP.TAUSBBD);

    return true;
}

bool PD_SWP(EnergyPlusData &state,
            CFSLAYER const &L, // PD layer
            CFSSWP &LSWP       // returned: equivalent layer properties set
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    //       DATE WRITTEN   unknown
    //       MODIFIED       Jason W. DeGraw
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Modifies drape fabric shortwave properties for openness. If not drape Fabric layer
    // returns false. If profile angles not specified diffuse properties are returned.

    Real64 TAUX; // This gets used as output of PD_DIFF and is then discarded

    if (!(L.LTYPE == LayerType::DRAPE)) {
        return false;
    }

    PD_DIFF(state, L.S, L.W, L.SWP_MAT.RHOSFDD, L.SWP_MAT.RHOSBDD, L.SWP_MAT.TAUS_DD, LSWP.RHOSFDD, LSWP.TAUS_DD);

    PD_DIFF(state, L.S, L.W, L.SWP_MAT.RHOSBDD, L.SWP_MAT.RHOSFDD, L.SWP_MAT.TAUS_DD, LSWP.RHOSBDD, TAUX);

    return true;
}

bool VB_LWP(EnergyPlusData &state,
            CFSLAYER const &L, // VB layer
            CFSLWP &LLWP       // returned: equivalent layer long wave properties
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Return venetian blind longwave properties from slat properties and geometry.
    // If not VB layer returns False.

    // Return value
    bool VB_LWP;

    Real64 RHODFS_SLAT;
    Real64 RHOUFS_SLAT;
    Real64 RHOLF;
    Real64 RHOLB;
    Real64 TAULX;

    VB_LWP = false;
    if (!IsVBLayer(L)) {
        return VB_LWP;
    }

    // slat reflectances
    RHODFS_SLAT = 1.0 - L.LWP_MAT.EPSLB - L.LWP_MAT.TAUL; // downward surface
    RHOUFS_SLAT = 1.0 - L.LWP_MAT.EPSLF - L.LWP_MAT.TAUL; // upward surface

    // TODO: are there cases where 2 calls not needed (RHODFS_SLAT == RHOUFS_SLAT??)
    VB_DIFF(state, L.S, L.W, Constant::DegToRad * L.PHI_DEG, RHODFS_SLAT, RHOUFS_SLAT, L.LWP_MAT.TAUL, RHOLF, LLWP.TAUL);
    LLWP.EPSLF = 1.0 - RHOLF - LLWP.TAUL;

    VB_DIFF(state, L.S, L.W, -Constant::DegToRad * L.PHI_DEG, RHODFS_SLAT, RHOUFS_SLAT, L.LWP_MAT.TAUL, RHOLB, TAULX);
    LLWP.EPSLB = 1.0 - RHOLB - LLWP.TAUL;

    VB_LWP = true;
    return VB_LWP;
}

bool VB_SWP(EnergyPlusData const &state,
            CFSLAYER const &L, // VB layer
            CFSSWP &LSWP,      // returned: equivalent off-normal properties
            const Real64 OMEGA // incident profile angle (radians)
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    //       DATE WRITTEN   unknown
    //       MODIFIED       Jason W. DeGraw
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Returns venetian blind off-normal short wave properties. If not VB layer
    // returns False.

    // FUNCTION ARGUMENT DEFINITIONS:
    //   sets: RHOSFBD, TAUSFBB, TAUSFBD

    if (!IsVBLayer(L)) {
        return false;
    }

    Real64 SL_WR = VB_SLAT_RADIUS_RATIO(L.W, L.C);

    // modify angle-dependent values for actual profile angle
    VB_SOL46_CURVE(state,
                   L.S,
                   L.W,
                   SL_WR,
                   Constant::DegToRad * L.PHI_DEG,
                   OMEGA,
                   L.SWP_MAT.RHOSBDD,
                   L.SWP_MAT.RHOSFDD,
                   L.SWP_MAT.TAUS_DD,
                   LSWP.RHOSFBD,
                   LSWP.TAUSFBB,
                   LSWP.TAUSFBD);

    VB_SOL46_CURVE(state,
                   L.S,
                   L.W,
                   SL_WR,
                   -Constant::DegToRad * L.PHI_DEG,
                   OMEGA,
                   L.SWP_MAT.RHOSBDD,
                   L.SWP_MAT.RHOSFDD,
                   L.SWP_MAT.TAUS_DD,
                   LSWP.RHOSBBD,
                   LSWP.TAUSBBB,
                   LSWP.TAUSBBD);

    return true;
}

bool VB_SWP(EnergyPlusData &state,
            CFSLAYER const &L, // VB layer
            CFSSWP &LSWP       // returned: equivalent off-normal properties
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    //       DATE WRITTEN   unknown
    //       MODIFIED       Jason W. DeGraw
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Returns venetian blind off-normal short wave properties. If not VB layer
    // returns False.

    // FUNCTION ARGUMENT DEFINITIONS:
    //   sets: RHOSFBD, TAUSFBB, TAUSFBD

    Real64 SL_WR;
    Real64 TAUX; // This gets used as output of VB_DIFF and is then discarded

    if (!IsVBLayer(L)) {
        return false;
    }

    SL_WR = VB_SLAT_RADIUS_RATIO(L.W, L.C);

    VB_DIFF(state, L.S, L.W, Constant::DegToRad * L.PHI_DEG, L.SWP_MAT.RHOSBDD, L.SWP_MAT.RHOSFDD, L.SWP_MAT.TAUS_DD, LSWP.RHOSFDD, LSWP.TAUS_DD);

    VB_DIFF(state, L.S, L.W, -Constant::DegToRad * L.PHI_DEG, L.SWP_MAT.RHOSBDD, L.SWP_MAT.RHOSFDD, L.SWP_MAT.TAUS_DD, LSWP.RHOSBDD, TAUX);

    return true;
}

bool VB_ShadeControl(EnergyPlusData const &state,
                     CFSLAYER &L,           // VB layer
                     Real64 const OMEGA_DEG // incident profile angle (degrees)
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    //  Modifies slat angle if shade control is true. If not uses the fixed
    //  slate angle and returns false.

    // Return value
    bool VB_ShadeControl;

    // FUNCTION ARGUMENT DEFINITIONS:
    //   see comments elsewhere re sign convention
    //   < 0 = diffuse

    Real64 SLATA;

    SLATA = L.PHI_DEG;

    if (L.CNTRL == state.dataWindowEquivalentLayer->lscVBPROF) {
        // slatA = profA (max gain)
        if (OMEGA_DEG < 0.0) {
            SLATA = -30.0;
        } else {
            SLATA = -OMEGA_DEG;
        }
    } else if (L.CNTRL == state.dataWindowEquivalentLayer->lscVBNOBM) {
        // slatA set to just exclude beam
        if (OMEGA_DEG < 0.0) {
            SLATA = VB_CriticalSlatAngle(30.0); // assume 30 deg for diffuse
        } else {
            SLATA = VB_CriticalSlatAngle(OMEGA_DEG);
        }
    }

    VB_ShadeControl = std::abs(SLATA - L.PHI_DEG) > 0.01;
    if (VB_ShadeControl) {
        L.PHI_DEG = SLATA;
    }
    return VB_ShadeControl;
}

Real64 VB_CriticalSlatAngle(Real64 const OMEGA_DEG // incident profile angle (degrees)
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Returns slat angle that just excludes beam radiation.

    // Return value
    Real64 VB_CriticalSlatAngle;

    // TODO handle vert blind cases etc
    // the slat normal points along the profile angle to block the beam solar
    VB_CriticalSlatAngle = 90.0 - OMEGA_DEG; //

    return VB_CriticalSlatAngle;
}

bool DoShadeControl(EnergyPlusData &state,
                    CFSLAYER &L,          // layer (returned updated)
                    Real64 const THETA,   // solar beam angle of incidence, from normal, (radians)
                    Real64 const OMEGA_V, // solar beam vertical profile angle, +=above horizontal (radians)
                    Real64 const OMEGA_H  // solar beam horizontal profile angle, +=clockwise when viewed
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory
    //       DATE WRITTEN   unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Returns .TRUE. if L is modified for shade control.

    // Return value
    bool DoShadeControl;

    // FUNCTION ARGUMENT DEFINITIONS:
    // 0 <= THETA <= PI/2
    //   = solar elevation angle for a vertical wall with
    //     wall-solar azimuth angle equal to zero
    //   from above (radians)
    //   = wall-solar azimuth angle for a vertical wall
    //     Used for PD and vertical VB

    Real64 OMEGA_DEG; // controlling profile angel, degrees

    DoShadeControl = false; // default: no shade controls implemented

    // must be consistent with IsControlledShade()
    if (IsVBLayer(L) && L.CNTRL != state.dataWindowEquivalentLayer->lscNONE) {
        if (THETA < 0.0 || THETA >= Constant::PiOvr2) {
            OMEGA_DEG = -1.0; // diffuse only
        } else if (L.LTYPE == LayerType::VBHOR) {
            // horiz VB
            OMEGA_DEG = state.dataWindowEquivalentLayer->RadiansToDeg * OMEGA_V;
        } else {
            // vert VB
            OMEGA_DEG = state.dataWindowEquivalentLayer->RadiansToDeg * OMEGA_H;
        }
        if (VB_ShadeControl(state, L, OMEGA_DEG)) {
            FinalizeCFSLAYER(state, L);
            DoShadeControl = true;
        }
    }
    return DoShadeControl;
}

void FinalizeCFSLAYER(EnergyPlusData &state, CFSLAYER &L) // layer, input: LTYPE, LWP_MAT, SWP_MAT
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory
    // PURPOSE OF THIS SUBROUTINE:
    //  Sets equivalent layer properties of a construction.

    // SUBROUTINE ARGUMENT DEFINITIONS:
    //          geometry (per LTYPE)
    //   output: LWP_EL, SWP_EL

    bool LOK;
    bool DOK;
    bool BOK;
    bool CFSLAYERFlag;

    if (IsVBLayer(L)) {
        LOK = VB_LWP(state, L, L.LWP_EL);
        DOK = VB_SWP(state, L, L.SWP_EL);      // SW diffuse
        BOK = VB_SWP(state, L, L.SWP_EL, 0.0); // SW properties w/ profile ang = 0
    } else {
        L.PHI_DEG = 0.0; // phi, C, CNTRL are VB only
        L.C = 0.0;
        L.CNTRL = state.dataWindowEquivalentLayer->lscNONE;
        if (L.LTYPE == LayerType::DRAPE) {
            LOK = PD_LWP(state, L, L.LWP_EL);
            DOK = PD_SWP(state, L, L.SWP_EL);           // SW diffuse
            BOK = PD_SWP(state, L, L.SWP_EL, 0.0, 0.0); // SW properties w/ profile angs = 0
        } else if (L.LTYPE == LayerType::INSCRN) {
            LOK = IS_LWP(L, L.LWP_EL);             // LW
            DOK = IS_SWP(state, L, L.SWP_EL);      // SW diffuse
            BOK = IS_SWP(state, L, L.SWP_EL, 0.0); // SW beam w/ theta = 0
        } else {
            L.S = 0.0; // geometry mbrs unused
            L.W = 0.0;
            if (L.LTYPE == LayerType::ROLLB) {
                LOK = RB_LWP(L, L.LWP_EL);             // LW
                DOK = RB_SWP(state, L, L.SWP_EL);      // SW diffuse
                BOK = RB_SWP(state, L, L.SWP_EL, 0.0); // SW beam w/ theta = 0
                                                       // ELSE IF (ISGZSLayer( L)) THEN
                // spectral glazing. Set layer xxx_MAT from GZS file data
                //    BOK = GZSLayerInit( L) .EQ. 0
                //    L%SWP_EL = L%SWP_MAT
                //    L%LWP_EL = L%LWP_MAT
                //    LOK = .TRUE.
                //    DOK = .TRUE.
            } else {
                // glazing
                L.SWP_EL = L.SWP_MAT;
                L.LWP_EL = L.LWP_MAT;
                LOK = true;
                DOK = true;
                BOK = true;
            }
        }
    }
    CFSLAYERFlag = LOK && DOK && BOK;
}

bool IsGZSLayer(CFSLAYER const &L)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory

    // PURPOSE OF THIS FUNCTION:
    // Returns .TRUE. if Layer has glazing data from external file or returns .FALSE.

    // Return value
    bool IsGZSLayer;

    IsGZSLayer = L.LTYPE == LayerType::GZS;
    return IsGZSLayer;
}

bool IsGlazeLayerX(CFSLAYER const &L)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory

    // PURPOSE OF THIS FUNCTION:
    // Returns .TRUE. if Layer has glazing (including GZS) or returns .FALSE.

    // Return value
    bool IsGlazeLayerX;

    IsGlazeLayerX = L.LTYPE == LayerType::GLAZE || IsGZSLayer(L);
    return IsGlazeLayerX;
}

bool IsControlledShade(EnergyPlusData const &state, CFSLAYER const &L)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory

    // PURPOSE OF THIS FUNCTION:
    // Returns .TRUE. if Layer is Venetian blind and is controlled or returns .FALSE.

    return IsVBLayer(L) && L.CNTRL != state.dataWindowEquivalentLayer->lscNONE;
}

bool IsVBLayer(CFSLAYER const &L)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         JOHN L. WRIGHT, University of Waterloo, Mechanical Engineering
    //                      Advanced Glazing System Laboratory

    // PURPOSE OF THIS FUNCTION:
    // Returns .TRUE. if Layer is Venetian blind, or returns .FALSE.

    return L.LTYPE == LayerType::VBHOR || L.LTYPE == LayerType::VBVER;
}

void BuildGap(EnergyPlusData &state,
              CFSGAP &G,       // returned
              int const GType, // gap type (gtyOPENin, gtyOPENout or gtySEALED)
              Real64 &TAS      // gap thickness, m
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    //       MODIFIED       Bereket Nigusse, June 2013, Jason W. DeGraw 2023

    // PURPOSE OF THIS SUBROUTINE:
    // fills in the effective gap thickness and calculates the gas density
    // The gas density is calculated at a standard manufactured condition
    // if a different condition is not specified.

    // SUBROUTINE ARGUMENT DEFINITIONS:
    // at time of manufacture, default = 21 C / 1 ATM

    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr Real64 GapThickMin(0.0001); // Minimum gap thickness allowed, m
    static constexpr std::string_view RoutineName("BuildGap: ");

    if (TAS < GapThickMin) {
        ShowSevereError(state, format("{}{}", RoutineName, G.Name));
        ShowContinueError(state, "...specified gap thickness is < 0.0001 m.  Reset to 0.00001 m");
        TAS = GapThickMin;
    }
    G.TAS = TAS;
    G.TAS_EFF = G.TAS;
    // effective gap thickness will be adjusted later if there is in between
    // venetian blind, see AdjustVBGap() routine

    G.GTYPE = GType;
    G.RHOGAS = DensityCFSFillGas(G.FG, state.dataWindowEquivalentLayer->PAtmSeaLevel, 294.15);
}

void AdjustVBGap(CFSGAP &G,        // gap, returned updated
                 CFSLAYER const &L // adjacent layer
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP

    // PURPOSE OF THIS SUBROUTINE:
    // Adjusts thickness of adjacent gaps separated by in between slatted blind.

    // METHODOLOGY EMPLOYED:
    // Treat VB layer as if it has 70% of actual thickness

    // REFERENCES:
    //  Wright, J. L., N. Y. T. Huang, and M. R. Collins.  2008.
    //  "Thermal Resistance of a Window with an Enclosed Venetian Blind: A Simplified Model,"
    //  ASHRAE Transactions, Vol. 114, Pt. 1.

    Real64 VBTHICK;

    if (!IsVBLayer(L)) {
        return; // insurance
    }

    VBTHICK = L.W * std::cos(L.PHI_DEG); // VB layer thickness at slat angle
    G.TAS_EFF = G.TAS + (L.W - 0.7 * VBTHICK) / 2.0;
}

float DensityCFSFillGas(CFSFILLGAS const &FG, // gas properties
                        Real64 const P,       // pressure, Pa
                        Real64 const T        // temperature, K
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    // PURPOSE OF THIS FUNCTION:
    // Returns gas density at P and T, kg/m3
    // METHODOLOGY EMPLOYED:
    // Uses ideal gas relations

    return (P * FG.MHAT) / (Constant::UniversalGasConst * max(T, 1.0));
}

int CFSNGlz(CFSTY const &FS) // CFS
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    // PURPOSE OF THIS FUNCTION:
    // Returns the number of glazing layers

    int CFSNGlz = 0;
    for (int iL = 1; iL <= FS.NL; ++iL) {
        if (IsGlazeLayerX(FS.L(iL))) {
            ++CFSNGlz;
        }
    }
    return CFSNGlz;
}

int CFSHasControlledShade(EnergyPlusData const &state, CFSTY const &FS)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    // PURPOSE OF THIS FUNCTION:
    // Returns index of the controlled layer in a fenestration. If no
    // controlled layer, then returns zero.

    int CFSHasControlledShade = 0;
    for (int iL = 1; iL <= FS.NL; ++iL) {
        if (IsControlledShade(state, FS.L(iL))) {
            CFSHasControlledShade = iL;
            break;
        }
    }
    return CFSHasControlledShade;
}

void CheckAndFixCFSLayer(EnergyPlusData &state, CFSLAYER &Layer)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         ASHRAE 1311-RP
    // PURPOSE OF THIS SUBROUTINE:
    // Verify CFS layer validity, sets bad items to valid defaults if possible

    FillDefaultsSWP(state, Layer, Layer.SWP_MAT);
    FinalizeCFSLAYER(state, Layer);
}

void FillDefaultsSWP(EnergyPlusData &state,
                     CFSLAYER const &L, // CFSLayer (input properties must be set)
                     CFSSWP &SWP        // properties to fill
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         The University of WaterLoo
    //       MODIFIED       Bereket Nigusse/FSEC, June 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Fills in defaulted short wave optical properties for equivalent window layers

    // may be within L
    static constexpr std::string_view RoutineName("FillDefaultsSWP: ");

    // default back taus to front (often equal)
    if (SWP.TAUSBBB < 0.0) {
        SWP.TAUSBBB = SWP.TAUSFBB;
    }
    if (SWP.TAUSBBD < 0.0) {
        SWP.TAUSBBD = SWP.TAUSFBD;
    }

    if (L.LTYPE == LayerType::GLAZE) {
        // estimate diffuse properties if any < 0 or autocalculate
        if (min(SWP.RHOSBDD, SWP.RHOSFDD, SWP.TAUS_DD) < 0.0) {
            Specular_EstimateDiffuseProps(state, SWP);
        }
    } else if (L.LTYPE == LayerType::VBHOR || L.LTYPE == LayerType::VBVER) {

    } else if (L.LTYPE == LayerType::DRAPE) {
        // estimate diffuse properties if any < 0
        if (min(SWP.RHOSBDD, SWP.RHOSFDD, SWP.TAUS_DD) < 0.0) {
            Fabric_EstimateDiffuseProps(state, SWP);
        }
    } else if (L.LTYPE == LayerType::ROLLB) {
        // estimate diffuse properties if any < 0
        if (min(SWP.RHOSBDD, SWP.RHOSFDD, SWP.TAUS_DD) < 0.0) {
            RB_SWP(state, L, SWP); // TODO RB
        }
    } else if (L.LTYPE == LayerType::INSCRN) {
        if (SWP.TAUSFBB < 0.0) {
            SWP.TAUSFBB = IS_OPENNESS(L.S, L.W);
            if (SWP.TAUSBBB < 0.0) {
                SWP.TAUSBBB = SWP.TAUSFBB;
            }
        }
        if (min(SWP.RHOSBDD, SWP.RHOSFDD, SWP.TAUS_DD) < 0.0) {
            IS_SWP(state, L, SWP); // TODO IS
        }
    } else if (L.LTYPE == LayerType::NONE || L.LTYPE == LayerType::ROOM) {
        // none or room: do nothing
    } else {
        ShowSevereError(state, format("{}{}.", RoutineName, L.Name));
        ShowContinueError(state, "...invalid layer type specified.");
    }
}

void FinalizeCFS(EnergyPlusData &state, CFSTY &FS)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         The University of WaterLoo
    //       MODIFIED       Bereket Nigusse/FSEC, May 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Complete CFS after BuildCFS by checking the shade type and gap type

    static constexpr std::string_view RoutineName("FinalizeCFS: "); // include trailing blank space

    std::string CurrentModuleObject = "WindowConstruction:EquivalentLayer";
    bool ErrorsFound = false;

    bool LVBPREV = false; // .TRUE. if previous layer is VB

    for (int iL = 1; iL <= FS.NL; ++iL) {
        if (!IsVBLayer(FS.L(iL))) {
            LVBPREV = false;
        } else if (LVBPREV) {
            ShowSevereError(state, format("{}=\"{}\", illegal.", CurrentModuleObject, FS.Name));
            ShowContinueError(state, "...adjacent VB layers are specified.");
            ErrorsFound = true;
        } else {
            LVBPREV = true;
            if (iL > 1) {
                AdjustVBGap(FS.G(iL - 1), FS.L(iL));
            }
            if (iL < FS.NL) {
                AdjustVBGap(FS.G(iL), FS.L(iL));
            }
        }
        if (iL < FS.NL) {
            int gType = FS.G(iL).GTYPE;
            if (gType == state.dataWindowEquivalentLayer->gtyOPENout && iL != 1) {
                ShowSevereError(state, format("{}=\"{}", CurrentModuleObject, FS.Name));
                ShowContinueError(state, format("...invalid EquivalentLayer window gap type specified ={}.", FS.G(iL).Name));
                ShowContinueError(state, "...VentedOutDoor gap is not outermost.");
            }
            if (gType == state.dataWindowEquivalentLayer->gtyOPENin && iL != FS.NL - 1) {
                ShowSevereError(state, format("{}=\"{}", CurrentModuleObject, FS.Name));
                ShowContinueError(state, format("...invalid EquivalentLayer window gap type specified ={}.", FS.G(iL).Name));
                ShowContinueError(state, "...VentedIndoor gap is not innermost.");
            }
        }
    }
    if (ErrorsFound) {
        ShowFatalError(state, format("{}Program terminates for preceding reason(s).", RoutineName));
    }
}

Real64 EffectiveEPSLF(CFSTY const &FS) // Complex Fenestration
{
    // FUNCTION INFORMATION:
    //       AUTHOR         <unknown>, ASHRAE 1311-RP
    // PURPOSE OF THIS FUNCTION:
    // Returns effective outside Longwave emissivity. Handles partially transparent layers

    Real64 E;  // Effective emissivity
    Real64 TX; // correction factor
    int iL;    // layers index

    E = 0.0;
    TX = 1.0;
    for (iL = 1; iL <= FS.NL + 1; ++iL) {
        if (iL == FS.NL + 1) {
            E += 0.9 * TX;
        } else {
            E += FS.L(iL).LWP_EL.EPSLF * TX;
            if (FS.L(iL).LWP_EL.TAUL < 0.001) {
                break;
            }
            TX *= FS.L(iL).LWP_EL.TAUL;
        }
    }
    return E;
}

Real64 EffectiveEPSLB(CFSTY const &FS) // Complex Fenestration
{
    // FUNCTION INFORMATION:
    //       AUTHOR         <unknown>, ASHRAE 1311-RP
    // PURPOSE OF THIS FUNCTION:
    // Returns effective inside (room side) Longwave emissivity. Handles partially transparent layers

    Real64 E = 0.0;  // Effective emissivity
    Real64 TX = 1.0; // correction factor
    for (int iL = FS.NL; iL >= 0; --iL) {
        if (iL == 0) {
            E += 0.9 * TX;
        } else {
            E += FS.L(iL).LWP_EL.EPSLB * TX;
            if (FS.L(iL).LWP_EL.TAUL < 0.001) {
                break;
            }
            TX *= FS.L(iL).LWP_EL.TAUL;
        }
    }
    return E;
}

bool FEQX(Real64 const a, // values to compare, fractional tolerance
          Real64 const b,
          Real64 const tolF,
          const Real64 tolAbs // absolute tolerance
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         <unknown>, ASHRAE 1311-RP
    // PURPOSE OF THIS FUNCTION:
    // Returns true if the difference between two real numbers is within the
    // tolerance limit specified.

    Real64 tolAbsX = max(tolAbs, 1.e-10);

    Real64 d = std::abs(a - b);
    if (d < tolAbsX) {
        return true;
    } else {
        return (2.0 * d / (std::abs(a) + std::abs(b))) < tolF;
    }
}

Real64 TRadC(Real64 const J,    // radiosity, W/m2
             Real64 const Emiss // surface emissivity
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         <unknown>, ASHRAE 1311-RP
    // PURPOSE OF THIS FUNCTION:
    // Returns equivalent celsius scale temperature from radiosity

    return root_4(J / (Constant::StefanBoltzmann * max(Emiss, 0.001))) - Constant::Kelvin;
}

void CalcEQLOpticalProperty(EnergyPlusData &state,
                            int const SurfNum,
                            SolarArrays const BeamDIffFlag, // identifier index of diffuse and beam SW radiation
                            Array2A<Real64> CFSAbs          // absorbed beam solar radiation by layers fraction
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Bereket Nigusse
    //       DATE WRITTEN   May 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the system optical properties from the individual layers
    // properties at each time step. The values returned are the layer-by-layer
    // absorptance and system transmittance for both beam and diffuse radiation.

    // METHODOLOGY EMPLOYED:
    // Uses the net radiation method developed for ASHWAT fenestration
    // model (ASHRAE RP-1311) by John Wright, the University of WaterLoo

    // Argument array dimensioning
    CFSAbs.dim(2, CFSMAXNL + 1);

    Array2D<Real64> Abs1(2, CFSMAXNL + 1);

    auto &surf = state.dataSurface->Surface(SurfNum);

    auto &CFS = state.dataWindowEquivLayer->CFS;

    Real64 IncAng = 0.0; // incident angle degree
    CFSAbs = 0.0;
    Real64 ProfAngHor = 0.0;
    Real64 ProfAngVer = 0.0;
    int ConstrNum = surf.Construction;
    int EQLNum = state.dataConstruction->Construct(surf.Construction).EQLConsPtr;
    if (BeamDIffFlag != SolarArrays::DIFF) {
        if (state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum) <= 0.0) {
            return;
        }

        for (int Lay = 1; Lay <= CFS(EQLNum).NL; ++Lay) {
            if (IsVBLayer(CFS(EQLNum).L(Lay))) {
                if (CFS(EQLNum).L(Lay).LTYPE == LayerType::VBHOR) {
                    ProfAngVer = Dayltg::ProfileAngle(state, SurfNum, state.dataEnvrn->SOLCOS, DataWindowEquivalentLayer::Orientation::Horizontal);
                } else if (CFS(EQLNum).L(Lay).LTYPE == LayerType::VBVER) {
                    ProfAngHor = Dayltg::ProfileAngle(state, SurfNum, state.dataEnvrn->SOLCOS, DataWindowEquivalentLayer::Orientation::Vertical);
                }
            }
        }
        // Incident angle
        IncAng = std::acos(state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum));
        CalcEQLWindowOpticalProperty(state, CFS(EQLNum), BeamDIffFlag, Abs1, IncAng, ProfAngVer, ProfAngHor);
        CFSAbs(1, {1, CFSMAXNL + 1}) = Abs1(1, {1, CFSMAXNL + 1});
        CFSAbs(2, {1, CFSMAXNL + 1}) = Abs1(2, {1, CFSMAXNL + 1});
    } else {
        if (state.dataWindowEquivalentLayer->EQLDiffPropFlag(EQLNum)) {
            for (int Lay = 1; Lay <= CFS(EQLNum).NL; ++Lay) {
                if (IsVBLayer(CFS(EQLNum).L(Lay))) {
                    if (CFS(EQLNum).L(Lay).LTYPE == LayerType::VBHOR) {
                        ProfAngVer =
                            Dayltg::ProfileAngle(state, SurfNum, state.dataEnvrn->SOLCOS, DataWindowEquivalentLayer::Orientation::Horizontal);
                    } else if (CFS(EQLNum).L(Lay).LTYPE == LayerType::VBVER) {
                        ProfAngHor = Dayltg::ProfileAngle(state, SurfNum, state.dataEnvrn->SOLCOS, DataWindowEquivalentLayer::Orientation::Vertical);
                    }
                }
            }
            IncAng = std::acos(state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum));
            CalcEQLWindowOpticalProperty(state, CFS(EQLNum), BeamDIffFlag, Abs1, IncAng, ProfAngVer, ProfAngHor);
            CFSAbs(_, {1, CFSMAXNL + 1}) = Abs1(_, {1, CFSMAXNL + 1});
            state.dataWindowEquivalentLayer->CFSDiffAbsTrans(_, {1, CFSMAXNL + 1}, EQLNum) = Abs1(_, {1, CFSMAXNL + 1});
            state.dataConstruction->Construct(ConstrNum).TransDiff = Abs1(1, CFS(EQLNum).NL + 1);
            state.dataConstruction->Construct(ConstrNum).AbsDiffFrontEQL({1, CFSMAXNL}) = Abs1(1, {1, CFSMAXNL});
            state.dataConstruction->Construct(ConstrNum).AbsDiffBackEQL({1, CFSMAXNL}) = Abs1(2, {1, CFSMAXNL});
            state.dataConstruction->Construct(ConstrNum).ReflectSolDiffFront = CFS(EQLNum).L(1).SWP_EL.RHOSFDD;
            state.dataConstruction->Construct(ConstrNum).ReflectSolDiffBack = CFS(EQLNum).L(CFS(EQLNum).NL).SWP_EL.RHOSBDD;
            if (!CFS(EQLNum).ISControlled) {
                state.dataWindowEquivalentLayer->EQLDiffPropFlag(EQLNum) = false;
            }
        } else {
            CFSAbs(_, {1, CFSMAXNL + 1}) = state.dataWindowEquivalentLayer->CFSDiffAbsTrans(_, {1, CFSMAXNL + 1}, EQLNum);
            state.dataConstruction->Construct(ConstrNum).TransDiff = state.dataWindowEquivalentLayer->CFSDiffAbsTrans(1, CFS(EQLNum).NL + 1, EQLNum);
            state.dataConstruction->Construct(ConstrNum).AbsDiffFrontEQL({1, CFSMAXNL}) = CFSAbs(1, {1, CFSMAXNL});
            state.dataConstruction->Construct(ConstrNum).AbsDiffBackEQL({1, CFSMAXNL}) = CFSAbs(2, {1, CFSMAXNL});
        }
    }
    if (CFS(EQLNum).VBLayerPtr > 0) {
        auto &surfShade = state.dataSurface->surfShades(SurfNum);
        surfShade.blind.slatAngDeg = CFS(EQLNum).L(CFS(EQLNum).VBLayerPtr).PHI_DEG;
    }
}

void CalcEQLWindowStandardRatings(EnergyPlusData &state, int const ConstrNum) // construction index
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Bereket Nigusse
    //       DATE WRITTEN   May 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the U-value, SHGC and Normal Transmittance of equivalent layer fenestration.

    // METHODOLOGY EMPLOYED:
    // Uses routine developed for ASHRAE RP-1311 (ASHWAT Model)

    Real64 UValue = 0.0;
    Real64 SHGCSummer = 0.0;
    Real64 TransNormal = 0.0;

    int EQLNum = state.dataConstruction->Construct(ConstrNum).EQLConsPtr;

    // calculate fenestration air-to-air U-value
    CalcEQLWindowUvalue(state, state.dataWindowEquivLayer->CFS(EQLNum), UValue);
    state.dataHeatBal->NominalU(ConstrNum) = UValue;

    // calculate the SHGC and Normal Transmittance
    CalcEQLWindowSHGCAndTransNormal(state, state.dataWindowEquivLayer->CFS(EQLNum), SHGCSummer, TransNormal);
    state.dataConstruction->Construct(ConstrNum).SummerSHGC = SHGCSummer;
    state.dataConstruction->Construct(ConstrNum).SolTransNorm = TransNormal;
}

Real64 EQLWindowInsideEffectiveEmiss(EnergyPlusData &state, int const ConstrNum)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Bereket A Nigusse
    //       DATE WRITTEN   May 2013

    // PURPOSE OF THIS FUNCTION:
    // Given the construction number, returns the equivalent layer inside
    // face effective longwave emissivity.

    return EffectiveEPSLB(state.dataWindowEquivLayer->CFS(state.dataConstruction->Construct(ConstrNum).EQLConsPtr));
}

Real64 EQLWindowOutsideEffectiveEmiss(EnergyPlusData &state, int const ConstrNum)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Bereket A Nigusse
    //       DATE WRITTEN   May 2013

    // PURPOSE OF THIS FUNCTION:
    // Given the construction number, returns the equivalent layer outside
    // face effective longwave emissivity.

    int EQLNum = state.dataConstruction->Construct(ConstrNum).EQLConsPtr;
    return EffectiveEPSLF(state.dataWindowEquivLayer->CFS(EQLNum));
}

Real64 HCInWindowStandardRatings(EnergyPlusData &state,
                                 Real64 const Height,  // Window height, 1.0 m
                                 Real64 const TSurfIn, // Inside surface temperature
                                 Real64 const TAirIn   // Zone Air Temperature
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Bereket Nigusse
    //       DATE WRITTEN   June 2013

    // PURPOSE OF THIS FUNCTION:
    // Return the inside convection coefficient for fenestration ratings.
    // This procedure is adopted from WindowTempsForNominalCond routine.
    // METHODOLOGY EMPLOYED:
    // Uses ISO Standard 15099 method to calculate the inside surface
    // convection coefficient for fenestration ratings.

    using Psychrometrics::PsyRhoAirFnPbTdbW;

    static constexpr std::string_view RoutineName("HCInWindowStandardRatings");

    Real64 TmeanFilm;       // mean film temperature
    Real64 TmeanFilmKelvin; // mean film temperature for property evaluation
    Real64 rho;             // density of (apparently dry) air [kg/m3]
    Real64 Cp;              // specific heat of air [J/kg-K]
    Real64 lambda;          // thermal conductivity of air [W/m-K]
    Real64 mu;              // dynamic viscosity of air [kg/m-s]
    Real64 RaH;             // Rayleigh number for cavity height [ Non dim]
    Real64 TiltDeg;         // glazing tilt in degrees
    Real64 sineTilt;        // sine of glazing tilt
    Real64 Nuint;           // Nusselt number for interior surface convection

    TiltDeg = 90.0;
    sineTilt = std::sin(TiltDeg * Constant::DegToRad); // degrees as arg

    // Begin calculating for ISO 15099 method.
    // mean film temperature
    TmeanFilmKelvin = TAirIn + 0.25 * (TSurfIn - TAirIn); // eq. 133 in ISO 15099
    TmeanFilm = TmeanFilmKelvin - 273.15;
    // the following properties are constants or linear relations for "standard" type reporting
    rho = PsyRhoAirFnPbTdbW(state, 101325.0, TmeanFilm, 0.0, RoutineName); // dry air assumption

    lambda = 2.873E-3 + 7.76E-5 * TmeanFilmKelvin; // Table B.1 in ISO 15099
    mu = 3.723E-6 + 4.94E-8 * TmeanFilmKelvin;     // Table B.2 in ISO 15099
    Cp = 1002.737 + 1.2324E-2 * TmeanFilmKelvin;   // Table B.3 in ISO 15099

    RaH = (pow_2(rho) * pow_3(Height) * Constant::Gravity * Cp * std::abs(TSurfIn - TAirIn)) / (TmeanFilmKelvin * mu * lambda); // eq 132 in ISO 15099

    // eq. 135 in ISO 15099 (only need this one because tilt is 90 deg)
    Nuint = 0.56 * root_4(RaH * sineTilt);
    return Nuint * lambda / Height;
}

} // namespace EnergyPlus::WindowEquivalentLayer
