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
#include <cstdint>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/ArrayS.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataShadowingCombinations.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/PierceSurface.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/TARCOGGassesParams.hh>
#include <EnergyPlus/TARCOGMain.hh>
#include <EnergyPlus/TARCOGParams.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/Vectors.hh>
#include <EnergyPlus/WindowComplexManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace WindowComplexManager {

    // Module containing the routines dealing with complex fenestration

    // MODULE INFORMATION:
    //       AUTHOR         Joe Klems
    //       DATE WRITTEN   ???
    //       MODIFIED       November 2011, Simon Vidanovic
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    //  Initialize data for solar and thermal calculations and also performs thermal calculations for BSDF window

    using namespace DataVectorTypes;
    using namespace DataBSDFWindow;
    using namespace DataSurfaces; // , ONLY: TotSurfaces,TotWindows,Surface,SurfaceWindow   !update this later
    using namespace DataHeatBalance;
    using namespace DataShadowingCombinations;
    using namespace Vectors;

    void InitBSDFWindows(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Joe Klems
        //       DATE WRITTEN   August 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set up the overall optical geometry for a BSDF window

        using namespace Vectors;

        int BaseSurf;           // base surface number (used in finding back surface)
        int NumStates;          // Local variable for the number of states
        Array1D<Real64> Thetas; // temp array holding theta values
        Array1D_int NPhis;      // temp array holding number of phis for a given theta
        Array1D<Real64> V(3);   // vector array
        Real64 VLen;            // Length of vector array
        int NHold;              // No. values in the Temporary array

        struct TempBasisIdx
        {
            // Members
            int Basis; // Basis no in basis table
            int State; // State in which basis first occurs

            // Default Constructor
            TempBasisIdx() : Basis(0), State(0)
            {
            }
        };

        // Object Data
        Array1D<TempBasisIdx> IHold; // Temporary array

        if (state.dataBSDFWindow->TotComplexFenStates <= 0) {
            return; // Nothing to do if no complex fenestration states
        }
        // Construct Basis List
        state.dataWindowComplexManager->BasisList.allocate(state.dataBSDFWindow->TotComplexFenStates);

        // Note:  Construction of the basis list contains the assumption of identical incoming and outgoing bases in
        //            that the complex fenestration state definition contains only one basis description, hence
        //            assumes square property matrices.  If this assumption were relaxed through change of the
        //            definition or additional definition of a state type with non-square matrices, then the loop
        //            below should be modified to enter both of the bases into the basis list.

        for (int IConst = state.dataBSDFWindow->FirstBSDF; IConst <= state.dataBSDFWindow->FirstBSDF + state.dataBSDFWindow->TotComplexFenStates - 1;
             ++IConst) {
            state.dataWindowComplexManager->MatrixNo = state.dataConstruction->Construct(IConst).BSDFInput.BasisMatIndex;
            if (state.dataWindowComplexManager->NumBasis == 0) {
                state.dataWindowComplexManager->NumBasis = 1;
                ConstructBasis(state, IConst, state.dataWindowComplexManager->BasisList(1));
            } else {
                for (int IBasis = 1; IBasis <= state.dataWindowComplexManager->NumBasis; ++IBasis) {
                    if (state.dataWindowComplexManager->MatrixNo == state.dataWindowComplexManager->BasisList(IBasis).BasisMatIndex) {
                        goto BsLoop_loop;
                    }
                }
                ++state.dataWindowComplexManager->NumBasis;
                ConstructBasis(state, IConst, state.dataWindowComplexManager->BasisList(state.dataWindowComplexManager->NumBasis));
            }
        BsLoop_loop:;
        }
        state.dataWindowComplexManager->BasisList.redimension(state.dataWindowComplexManager->NumBasis);
        //  Proceed to set up geometry for complex fenestration states
        state.dataBSDFWindow->ComplexWind.allocate(state.dataSurface->TotSurfaces); // Set up companion array to SurfaceWindow to hold window
        //     geometry for each state.  This is an allocatable array of
        //     geometries for the window states but only the complex
        //     fenestration surfaces will have the arrays allocated
        //  Search Thru Surfaces for Complex Fenestration State references
        //  This will define the first complex fenestration state for that window, others will follow if there are
        //     control specifications
        state.dataWindowComplexManager->WindowList.allocate(state.dataSurface->TotSurfaces); // Temporary allocation
        state.dataWindowComplexManager->WindowStateList.allocate(state.dataBSDFWindow->TotComplexFenStates,
                                                                 state.dataSurface->TotSurfaces); // Temporary allocation
        for (int ISurf = 1; ISurf <= state.dataSurface->TotSurfaces; ++ISurf) {
            int IConst = state.dataSurface->Surface(ISurf).Construction;
            if (IConst == 0) {
                continue; // This is true for overhangs (Shading:Zone:Detailed)
            }
            if (!(state.dataConstruction->Construct(IConst).TypeIsWindow && (state.dataConstruction->Construct(IConst).WindowTypeBSDF))) {
                continue; // Only BSDF windows
            }
            // Simon Check: Thermal construction removed
            // ThConst = Construct(IConst)%BSDFInput%ThermalConstruction
            state.dataSurface->SurfWinWindowModelType(ISurf) = WindowModel::BSDF;
            state.dataHeatBal->AnyBSDF = true;
            ++state.dataWindowComplexManager->NumComplexWind;
            NumStates = 1;
            state.dataWindowComplexManager->WindowList(state.dataWindowComplexManager->NumComplexWind).NumStates =
                1; // Having found the construction reference in
            // the Surface array defines the first state for this window
            state.dataWindowComplexManager->WindowList(state.dataWindowComplexManager->NumComplexWind).SurfNo = ISurf;
            // WindowList(  NumComplexWind ).Azimuth = DegToRadians * Surface( ISurf ).Azimuth;
            // WindowList(  NumComplexWind ).Tilt = DegToRadians * Surface( ISurf ).Tilt;
            state.dataWindowComplexManager->WindowStateList(NumStates, state.dataWindowComplexManager->NumComplexWind).InitInc =
                state.dataWindowComplexManager->Calculate_Geometry;
            state.dataWindowComplexManager->WindowStateList(NumStates, state.dataWindowComplexManager->NumComplexWind).InitTrn =
                state.dataWindowComplexManager->Calculate_Geometry;
            state.dataWindowComplexManager->WindowStateList(NumStates, state.dataWindowComplexManager->NumComplexWind).CopyIncState = 0;
            state.dataWindowComplexManager->WindowStateList(NumStates, state.dataWindowComplexManager->NumComplexWind).CopyTrnState = 0;
            state.dataWindowComplexManager->WindowStateList(NumStates, state.dataWindowComplexManager->NumComplexWind).Konst = IConst;
            // Simon Check: ThermalConstruction assigned to current construction
            // WindowStateList(NumComplexWind, NumStates)%ThermConst = ThConst
            for (int I = 1; I <= state.dataWindowComplexManager->NumBasis; ++I) { // Find basis in Basis List
                if (state.dataConstruction->Construct(IConst).BSDFInput.BasisMatIndex == state.dataWindowComplexManager->BasisList(I).BasisMatIndex) {
                    state.dataWindowComplexManager->WindowStateList(NumStates, state.dataWindowComplexManager->NumComplexWind).IncBasisIndx =
                        I; // Note: square property matrices
                    state.dataWindowComplexManager->WindowStateList(NumStates, state.dataWindowComplexManager->NumComplexWind).TrnBasisIndx =
                        I; //   assumption
                }
            }
            if (state.dataWindowComplexManager->WindowStateList(NumStates, state.dataWindowComplexManager->NumComplexWind).IncBasisIndx <= 0) {
                ShowFatalError(state, "Complex Window Init: Window Basis not in BasisList.");
            }
        }
        //  Should now have a WindowList with dataWindowComplexManager. NumComplexWind entries containing all the complex fenestrations
        //    with a first state defined for each.
        //  *  *  *
        //  Here a search should be made for control specifications, which will give additional states for
        //    controlled complex fenestrations.  These should be added to the dataWindowComplexManager.WindowStateList, and
        //     WindowList( )%NumStates incremented for each window for which states are added.
        //      Added states should have WindowStateList ( , )%InitInc set to Calculate_Geometry
        //  *  *  *

        // At this point, we have a complete WindowList and WindowStateList, with dataWindowComplexManager. NumComplexWind
        //   defined, and NumStates for each complex window defined
        // Now sort through the window list to see that geometry will only be done once for each
        //  window, basis combination
        // Note:  code below assumes identical incoming and outgoing bases; following code will
        //   need revision if this assumption relaxed

        for (int IWind = 1; IWind <= state.dataWindowComplexManager->NumComplexWind; ++IWind) { // Search window list for repeated bases
            if (state.dataWindowComplexManager->WindowList(IWind).NumStates > 1) {
                IHold.allocate(state.dataWindowComplexManager->WindowList(IWind).NumStates);
                NHold = 1;
                IHold(1).State = 1;
                IHold(1).Basis = state.dataWindowComplexManager->WindowStateList(1, IWind).IncBasisIndx;
                // If the Mth new basis found is basis B in the basis list, and it
                // first occurs in the WindowStateList  in state N, then IHold(M)%Basis=B
                // and IHold(M)%State=N
                for (int K = 1; K <= state.dataWindowComplexManager->NumBasis; ++K) {
                    if (K > NHold) {
                        break;
                    }
                    int KBasis = IHold(K).Basis;
                    int J = IHold(K).State;
                    state.dataWindowComplexManager->InitBSDFWindowsOnce = true;
                    for (int I = J + 1; I <= state.dataWindowComplexManager->WindowList(IWind).NumStates;
                         ++I) { // See if subsequent states have the same basis
                        if ((state.dataWindowComplexManager->WindowStateList(I, state.dataWindowComplexManager->NumComplexWind).InitInc ==
                             state.dataWindowComplexManager->Calculate_Geometry) &&
                            (state.dataWindowComplexManager->WindowStateList(I, state.dataWindowComplexManager->NumComplexWind).IncBasisIndx ==
                             KBasis)) {
                            // Note:  square property matrices (same inc & trn bases) assumption
                            // If same incident and outgoing basis assumption removed, following code will need to
                            //  be extended to treat the two bases separately
                            state.dataWindowComplexManager->WindowStateList(I, state.dataWindowComplexManager->NumComplexWind).InitInc =
                                state.dataWindowComplexManager->Copy_Geometry;
                            state.dataWindowComplexManager->WindowStateList(I, state.dataWindowComplexManager->NumComplexWind).InitTrn =
                                state.dataWindowComplexManager->Copy_Geometry;
                            state.dataWindowComplexManager->WindowStateList(I, state.dataWindowComplexManager->NumComplexWind).CopyIncState = J;
                            state.dataWindowComplexManager->WindowStateList(I, state.dataWindowComplexManager->NumComplexWind).CopyTrnState = J;
                        } else if (state.dataWindowComplexManager->InitBSDFWindowsOnce) {
                            state.dataWindowComplexManager->InitBSDFWindowsOnce = false; // First occurrence of a different basis
                            ++NHold;
                            IHold(NHold).State = I;
                            IHold(NHold).Basis = state.dataWindowComplexManager->WindowStateList(I, IWind).IncBasisIndx;
                            state.dataWindowComplexManager->WindowStateList(I, state.dataWindowComplexManager->NumComplexWind).InitTrn =
                                state.dataWindowComplexManager->Calculate_Geometry;
                            state.dataWindowComplexManager->WindowStateList(I, state.dataWindowComplexManager->NumComplexWind).CopyIncState = 0;
                            state.dataWindowComplexManager->WindowStateList(I, state.dataWindowComplexManager->NumComplexWind).CopyTrnState = 0;
                        }
                    }
                }
                IHold.deallocate();
            }
        }

        //  Now go through window list and window state list and calculate or copy the
        //   geometry information for each window, state
        for (int IWind = 1; IWind <= state.dataWindowComplexManager->NumComplexWind; ++IWind) {
            int ISurf = state.dataWindowComplexManager->WindowList(IWind).SurfNo;
            NumStates = state.dataWindowComplexManager->WindowList(IWind).NumStates;
            // ALLOCATE(SurfaceWindow( ISurf )%ComplexFen)    !activate the BSDF window description
            //  for this surface
            state.dataSurface->SurfaceWindow(ISurf).ComplexFen.NumStates = NumStates;
            state.dataSurface->SurfaceWindow(ISurf).ComplexFen.State.allocate(NumStates); // Allocate space for the states
            state.dataBSDFWindow->ComplexWind(ISurf).NumStates = NumStates;
            state.dataBSDFWindow->ComplexWind(ISurf).Geom.allocate(NumStates); // Allocate space for the geometries
            // Azimuth = WindowList( IWind ).Azimuth;
            // Tilt = WindowList( IWind ).Tilt;
            // Get the number of back surfaces for this window
            BaseSurf = state.dataSurface->Surface(ISurf).BaseSurf; // ShadowComb is organized by base surface
            int NBkSurf = state.dataShadowComb->ShadowComb(BaseSurf).NumBackSurf;
            state.dataBSDFWindow->ComplexWind(ISurf).NBkSurf = NBkSurf;
            // Define the back surface directions
            state.dataBSDFWindow->ComplexWind(ISurf).sWinSurf.allocate(NBkSurf);
            state.dataBSDFWindow->ComplexWind(ISurf).sdotN.allocate(NBkSurf);
            // Define the unit vectors pointing from the window center to the back surface centers
            for (int KBkSurf = 1; KBkSurf <= NBkSurf; ++KBkSurf) {
                BaseSurf = state.dataSurface->Surface(ISurf).BaseSurf;                    // ShadowComb is organized by base surface
                int JSurf = state.dataShadowComb->ShadowComb(BaseSurf).BackSurf(KBkSurf); // these are all proper back surfaces
                V = state.dataSurface->Surface(JSurf).Centroid - state.dataSurface->Surface(ISurf).Centroid;
                VLen = magnitude(V);
                // Define the unit vector from the window center to the back
                state.dataBSDFWindow->ComplexWind(ISurf).sWinSurf(KBkSurf) = V / VLen;
                // surface center
                // Define the back surface cosine(incident angle)
                state.dataBSDFWindow->ComplexWind(ISurf).sdotN(KBkSurf) = dot(V, state.dataSurface->Surface(JSurf).OutNormVec) / VLen;
            }
            for (int IState = 1; IState <= NumStates; ++IState) {
                // The following assumes identical incoming and outgoing bases.  The logic will need to be
                //  redesigned if this assumption is relaxed
                int IConst = state.dataWindowComplexManager->WindowStateList(IState, IWind).Konst;
                // ThConst = WindowStateList ( IWind , IState )%ThermConst
                state.dataSurface->SurfaceWindow(ISurf).ComplexFen.State(IState).Konst = IConst;
                // SurfaceWindow(ISurf)%ComplexFen%State(IState)%ThermConst = ThConst
                if (state.dataWindowComplexManager->WindowStateList(IState, IWind).InitInc == state.dataWindowComplexManager->Calculate_Geometry) {
                    state.dataBSDFWindow->ComplexWind(ISurf).Geom(IState).Inc = state.dataWindowComplexManager->BasisList(
                        state.dataWindowComplexManager->WindowStateList(IState, IWind).IncBasisIndx); // Put in the basis structure from the BasisList
                    state.dataBSDFWindow->ComplexWind(ISurf).Geom(IState).Trn =
                        state.dataWindowComplexManager->BasisList(state.dataWindowComplexManager->WindowStateList(IState, IWind).TrnBasisIndx);

                    SetupComplexWindowStateGeometry(state,
                                                    ISurf,
                                                    IState,
                                                    IConst,
                                                    state.dataBSDFWindow->ComplexWind(ISurf),
                                                    state.dataBSDFWindow->ComplexWind(ISurf).Geom(IState),
                                                    state.dataSurface->SurfaceWindow(ISurf).ComplexFen.State(IState));
                    // Note--setting up the state geometry will include constructing outgoing basis/surface
                    //  maps and those incoming maps that will not depend on shading.
                } else {
                    state.dataSurface->SurfaceWindow(ISurf).ComplexFen.State(IState) = state.dataSurface->SurfaceWindow(ISurf).ComplexFen.State(
                        state.dataWindowComplexManager->WindowStateList(IState, IWind).CopyIncState); // Note this overwrites Konst
                    state.dataSurface->SurfaceWindow(ISurf).ComplexFen.State(IState).Konst = IConst;  //  so it has to be put back
                    // SurfaceWindow (ISurf )%ComplexFen%State(IState)%ThermConst = ThConst  !same for ThermConst
                    state.dataBSDFWindow->ComplexWind(ISurf).Geom(IState) =
                        state.dataBSDFWindow->ComplexWind(ISurf).Geom(state.dataWindowComplexManager->WindowStateList(IState, IWind).CopyIncState);
                }

            } // State loop
        } // Complex Window loop
        //  Allocate all beam-dependent complex fenestration quantities
        for (int IWind = 1; IWind <= state.dataWindowComplexManager->NumComplexWind; ++IWind) {
            int ISurf = state.dataWindowComplexManager->WindowList(IWind).SurfNo;
            NumStates = state.dataWindowComplexManager->WindowList(IWind).NumStates;
            for (int IState = 1; IState <= NumStates; ++IState) {
                AllocateCFSStateHourlyData(state, ISurf, IState);
            } // State loop
        } // Complex Window loop
    }

    void AllocateCFSStateHourlyData(EnergyPlusData &state,
                                    int const iSurf, // Surface number
                                    int const iState // Complex fenestration state number
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   May 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Allocate hourly data arrays for complex fenestration state

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NLayers; // Number of complex fenestration layers
        int NBkSurf; // Number of back surfaces
        int KBkSurf; // Back surfaces counter

        NLayers = state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(iState).NLayers;
        NBkSurf = state.dataBSDFWindow->ComplexWind(iSurf).NBkSurf;

        state.dataBSDFWindow->ComplexWind(iSurf).Geom(iState).SolBmGndWt.allocate(
            24, state.dataGlobal->TimeStepsInHour, state.dataBSDFWindow->ComplexWind(iSurf).Geom(iState).NGnd);
        state.dataBSDFWindow->ComplexWind(iSurf).Geom(iState).SolBmIndex.allocate(24, state.dataGlobal->TimeStepsInHour);
        state.dataBSDFWindow->ComplexWind(iSurf).Geom(iState).ThetaBm.allocate(24, state.dataGlobal->TimeStepsInHour);
        state.dataBSDFWindow->ComplexWind(iSurf).Geom(iState).PhiBm.allocate(24, state.dataGlobal->TimeStepsInHour);
        state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(iState).WinDirHemiTrans.allocate(24, state.dataGlobal->TimeStepsInHour);
        state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(iState).WinDirSpecTrans.allocate(24, state.dataGlobal->TimeStepsInHour);
        state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(iState).WinBmGndTrans.allocate(24, state.dataGlobal->TimeStepsInHour);
        state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(iState).WinBmFtAbs.allocate(24, state.dataGlobal->TimeStepsInHour, NLayers);
        state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(iState).WinBmGndAbs.allocate(24, state.dataGlobal->TimeStepsInHour, NLayers);
        state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(iState).WinToSurfBmTrans.allocate(24, state.dataGlobal->TimeStepsInHour, NBkSurf);
        state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(iState).BkSurf.allocate(NBkSurf);
        for (KBkSurf = 1; KBkSurf <= NBkSurf; ++KBkSurf) {
            state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(iState).BkSurf(KBkSurf).WinDHBkRefl.allocate(24,
                                                                                                                  state.dataGlobal->TimeStepsInHour);
            state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(iState).BkSurf(KBkSurf).WinDirBkAbs.allocate(
                24, state.dataGlobal->TimeStepsInHour, NLayers);
        }
    }

    void ExpandComplexState(EnergyPlusData &state,
                            int const iSurf, // Surface number
                            int const iConst // Construction number
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   May 2013
        //       MODIFIED       Simon Vidanovic (July 2013)
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // When complex fenestration is controlled by EMS, program does not know in advance how many states are assigned to
        // ceratin surface. This information can be obtain only at runtime. Purpose of this routine is to extend number of states
        // used by complex fenestration in case that is necessary.

        // Expands states by one
        int NumOfStates = state.dataSurface->SurfaceWindow(iSurf).ComplexFen.NumStates;

        state.dataBSDFWindow->ComplexWind(iSurf).Geom.redimension(NumOfStates + 1);
        state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State.redimension(NumOfStates + 1);

        // Do daylighting geometry only in case it is initialized. If daylighting is not used then no need to expand state for that
        if (state.dataBSDFWindow->ComplexWind(iSurf).DaylightingInitialized) {
            state.dataBSDFWindow->ComplexWind(iSurf).DaylghtGeom.redimension(NumOfStates + 1);
            state.dataBSDFWindow->ComplexWind(iSurf).DaylightingInitialized = false;
        } else {
            state.dataBSDFWindow->ComplexWind(iSurf).DaylghtGeom.allocate(NumOfStates + 1);
        }

        // Increase number of states and insert new state
        ++NumOfStates;
        state.dataSurface->SurfaceWindow(iSurf).ComplexFen.NumStates = NumOfStates;
        state.dataBSDFWindow->ComplexWind(iSurf).NumStates = NumOfStates;

        state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(NumOfStates).Konst = iConst;

        // load basis and setup window state geometry
        ConstructBasis(state, iConst, state.dataBSDFWindow->ComplexWind(iSurf).Geom(NumOfStates).Inc);
        ConstructBasis(state, iConst, state.dataBSDFWindow->ComplexWind(iSurf).Geom(NumOfStates).Trn);

        SetupComplexWindowStateGeometry(state,
                                        iSurf,
                                        NumOfStates,
                                        iConst,
                                        state.dataBSDFWindow->ComplexWind(iSurf),
                                        state.dataBSDFWindow->ComplexWind(iSurf).Geom(NumOfStates),
                                        state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(NumOfStates));

        // allocation of memory for hourly data can be performed only after window state geometry has been setup
        AllocateCFSStateHourlyData(state, iSurf, NumOfStates);

        // calculate static properties for complex fenestration
        CalcWindowStaticProperties(state,
                                   iSurf,
                                   NumOfStates,
                                   state.dataBSDFWindow->ComplexWind(iSurf),
                                   state.dataBSDFWindow->ComplexWind(iSurf).Geom(NumOfStates),
                                   state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(NumOfStates));

        // calculate hourly data from complex fenestration
        CFSShadeAndBeamInitialization(state, iSurf, NumOfStates);
    }

    void CheckCFSStates(EnergyPlusData &state, int const iSurf) // Surface number
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   May 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Check if there are new states available for complex fenestration and performs proper initialization

        int NumOfStates; // number of states for current surface
        bool StateFound; // variable to indicate if state has been found
        int i;           // Local counter
        int CurrentCFSState;

        StateFound = false;
        CurrentCFSState = state.dataSurface->SurfaceWindow(iSurf).ComplexFen.CurrentState;

        // Check if EMS changed construction number
        if (state.dataSurface->Surface(iSurf).Construction != state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(CurrentCFSState).Konst) {

            // If construction number changed then take new state
            // First search for existing states. Maybe state is already added in previous timestep
            NumOfStates = state.dataSurface->SurfaceWindow(iSurf).ComplexFen.NumStates;
            for (i = 1; i <= NumOfStates; ++i) {
                if (state.dataSurface->Surface(iSurf).Construction == state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(i).Konst) {
                    StateFound = true;
                    CurrentCFSState = i;
                    state.dataSurface->SurfaceWindow(iSurf).ComplexFen.CurrentState = i;
                }
            }
        } else {
            StateFound = true;
        }

        // If new state is not found in the list of current states, then create new one, initialize and make it active
        if (!StateFound) {
            ExpandComplexState(state, iSurf, state.dataSurface->Surface(iSurf).Construction);
            CurrentCFSState = state.dataSurface->SurfaceWindow(iSurf).ComplexFen.NumStates;
            state.dataSurface->SurfaceWindow(iSurf).ComplexFen.CurrentState = CurrentCFSState;
        }
    }

    void InitComplexWindows(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   November 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Extract simple init for Complex Windows

        // One-time initialization
        if (state.dataWindowComplexManager->InitComplexWindowsOnce) {
            state.dataWindowComplexManager->InitComplexWindowsOnce = false;
            InitBSDFWindows(state);
            CalcStaticProperties(state);
        }
    }

    void UpdateComplexWindows(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Joe Klems
        //       DATE WRITTEN   August 2011
        //       MODIFIED       B. Griffith, Nov. 2012 revised for detailed timestep integration mode
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Performs the shading-dependent initialization of the Complex Fenestration data;
        // On first call, calls the one-time initializition

        // LOGICAL,SAVE    ::  Once  =.TRUE.  !Flag for insuring things happen once
        int NumStates; // Number of states for a given complex fen
        int ISurf;     // Index for sorting thru Surface array
        int IState;    // Index identifying the window state for a particular window
        int IWind;     // Index identifying a window in the WindowList

        if (state.dataWindowComplexManager->NumComplexWind == 0) {
            return;
        }

        if (state.dataGlobal->KickOffSizing || state.dataGlobal->KickOffSimulation) {
            return;
        }

        // Shading-dependent initialization; performed once for each shading period

        // Initialize the geometric quantities

        for (IWind = 1; IWind <= state.dataWindowComplexManager->NumComplexWind; ++IWind) {
            ISurf = state.dataWindowComplexManager->WindowList(IWind).SurfNo;
            NumStates = state.dataBSDFWindow->ComplexWind(ISurf).NumStates;
            for (IState = 1; IState <= NumStates; ++IState) {
                CFSShadeAndBeamInitialization(state, ISurf, IState);
            } // State loop
        } // window loop
    }

    void CFSShadeAndBeamInitialization(EnergyPlusData &state,
                                       int const iSurf, // Window surface number
                                       int const iState // Window state number
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   May 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates shading properties of complex fenestration
        // Refactoring from Klems code

        using namespace Vectors;

        Vector SunDir(0.0, 0.0, 1.0); // unit vector pointing toward sun (world CS)
        Vector HitPt(0.0, 0.0, 1.0);  // vector location of ray intersection with a surface

        if (state.dataGlobal->KickOffSizing || state.dataGlobal->KickOffSimulation) {
            return;
        }

        int IncRay;   // Index of incident ray corresponding to beam direction
        Real64 Theta; // Theta angle of incident ray corresponding to beam direction
        Real64 Phi;   // Phi angle of incident ray corresponding to beam direction
        bool hit;     // hit flag
        int TotHits;  // hit counter
        auto &complexWindow(state.dataBSDFWindow->ComplexWind(iSurf));
        auto &complexWindowGeom(complexWindow.Geom(iState));
        auto &surfaceWindowState(state.dataSurface->SurfaceWindow(iSurf).ComplexFen.State(iState));

        if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
            std::size_t lHT(0);  // Linear index for ( Hour, TS )
            std::size_t lHTI(0); // Linear index for ( Hour, TS, I )
            for (int Hour = 1; Hour <= 24; ++Hour) {
                for (int TS = 1; TS <= state.dataGlobal->TimeStepsInHour; ++TS, ++lHT) { // [ lHT ] == ( Hour, TS )
                    SunDir = state.dataBSDFWindow->SUNCOSTS(TS, Hour);
                    Theta = 0.0;
                    Phi = 0.0;
                    if (state.dataBSDFWindow->SUNCOSTS(TS, Hour)(3) > DataEnvironment::SunIsUpValue) {
                        IncRay = FindInBasis(state, SunDir, RayIdentificationType::Front_Incident, iSurf, iState, complexWindowGeom.Inc, Theta, Phi);
                        complexWindowGeom.ThetaBm[lHT] = Theta;
                        complexWindowGeom.PhiBm[lHT] = Phi;
                    } else {
                        complexWindowGeom.ThetaBm[lHT] = 0.0;
                        complexWindowGeom.PhiBm[lHT] = 0.0;
                        IncRay = 0; // sundown can't have ray incident on window
                    }
                    if (IncRay > 0) { // Sun may be incident on the window
                        complexWindowGeom.SolBmIndex[lHT] = IncRay;
                    } else { // Window can't be sunlit, set front incidence ray index to zero
                        complexWindowGeom.SolBmIndex[lHT] = 0;
                    }
                    for (int I = 1, nGnd = complexWindowGeom.NGnd; I <= nGnd; ++I, ++lHTI) { // Gnd pt loop
                        TotHits = 0;
                        Vector const gndPt(complexWindowGeom.GndPt(I));
                        for (int JSurf = 1, eSurf = state.dataSurface->TotSurfaces; JSurf <= eSurf; ++JSurf) {
                            // the following test will cycle on anything except exterior surfaces and shading surfaces
                            if (state.dataSurface->Surface(JSurf).HeatTransSurf &&
                                state.dataSurface->Surface(JSurf).ExtBoundCond != ExternalEnvironment) {
                                continue;
                            }
                            // skip surfaces that face away from the ground point
                            if (dot(SunDir, state.dataSurface->Surface(JSurf).NewellSurfaceNormalVector) >= 0.0) {
                                continue;
                            }
                            // Looking for surfaces between GndPt and sun
                            hit = PierceSurface(state, JSurf, gndPt, SunDir, HitPt);
                            if (hit) {
                                // Are not going into the details of whether a hit surface is transparent
                                // Since this is ultimately simply weighting the transmittance, so great
                                // detail is not warranted
                                ++TotHits;
                                break;
                            }
                        }
                        if (TotHits > 0) {
                            complexWindowGeom.SolBmGndWt[lHTI] = 0.0; // [ lHTI ] == ( Hour, TS, I )
                        } else {
                            complexWindowGeom.SolBmGndWt[lHTI] = 1.0; // [ lHTI ] == ( Hour, TS, I )
                        }
                    } // Gnd pt loop

                    // update window beam properties
                    CalculateWindowBeamProperties(state, iSurf, iState, complexWindow, complexWindowGeom, surfaceWindowState, Hour, TS);
                } // Timestep loop
            } // Hour loop
        } else { // detailed timestep integration
            std::size_t const lHT(
                complexWindowGeom.ThetaBm.index(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep)); // [ lHT ] == ( HourOfDay, TimeStep )
            SunDir = state.dataBSDFWindow->SUNCOSTS(state.dataGlobal->TimeStep, state.dataGlobal->HourOfDay);
            Theta = 0.0;
            Phi = 0.0;
            if (state.dataBSDFWindow->SUNCOSTS(state.dataGlobal->TimeStep, state.dataGlobal->HourOfDay)(3) > DataEnvironment::SunIsUpValue) {
                IncRay = FindInBasis(state, SunDir, RayIdentificationType::Front_Incident, iSurf, iState, complexWindowGeom.Inc, Theta, Phi);
                complexWindowGeom.ThetaBm[lHT] = Theta;
                complexWindowGeom.PhiBm[lHT] = Phi;
            } else {
                complexWindowGeom.ThetaBm[lHT] = 0.0;
                complexWindowGeom.PhiBm[lHT] = 0.0;
                IncRay = 0; // sundown can't have ray incident on window
            }

            if (IncRay > 0) { // Sun may be incident on the window
                complexWindowGeom.SolBmIndex[lHT] = IncRay;
            } else { // Window can't be sunlit, set front incidence ray index to zero
                complexWindowGeom.SolBmIndex[lHT] = 0.0;
            }
            std::size_t lHTI(complexWindowGeom.SolBmGndWt.index(
                state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, 1));        // Linear index for ( HourOfDay, TimeStep, I )
            for (int I = 1, nGnd = complexWindowGeom.NGnd; I <= nGnd; ++I, ++lHTI) { // Gnd pt loop
                TotHits = 0;
                Vector const gndPt(complexWindowGeom.GndPt(I));
                for (int JSurf = 1; JSurf <= state.dataSurface->TotSurfaces; ++JSurf) {
                    // the following test will cycle on anything except exterior surfaces and shading surfaces
                    if (state.dataSurface->Surface(JSurf).HeatTransSurf && state.dataSurface->Surface(JSurf).ExtBoundCond != ExternalEnvironment) {
                        continue;
                    }
                    // skip surfaces that face away from the ground point
                    if (dot(SunDir, state.dataSurface->Surface(JSurf).NewellSurfaceNormalVector) >= 0.0) {
                        continue;
                    }
                    // Looking for surfaces between GndPt and sun
                    hit = PierceSurface(state, JSurf, gndPt, SunDir, HitPt);
                    if (hit) {
                        // Are not going into the details of whether a hit surface is transparent
                        // Since this is ultimately simply weighting the transmittance, so great
                        // detail is not warranted
                        ++TotHits;
                        break;
                    }
                }
                if (TotHits > 0) {
                    complexWindowGeom.SolBmGndWt[lHTI] = 0.0; // [ lHTI ] == ( HourOfDay, TimeStep, I )
                } else {
                    complexWindowGeom.SolBmGndWt[lHTI] = 1.0; // [ lHTI ] == ( HourOfDay, TimeStep, I )
                }
            } // Gnd pt loop

            // Update window beam properties
            CalculateWindowBeamProperties(
                state, iSurf, iState, complexWindow, complexWindowGeom, surfaceWindowState, state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep);
        } // solar calculation mode, average over days or detailed
    }

    void CalculateWindowBeamProperties(EnergyPlusData &state,
                                       int const ISurf,                   // Window surface number
                                       int const IState,                  // Window state number
                                       BSDFWindowGeomDescr const &Window, // Window Geometry
                                       BSDFGeomDescr const &Geom,         // State Geometry
                                       BSDFStateDescr &State,             // State Description
                                       int const Hour,                    // Hour number
                                       int const TS                       // Timestep number
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Joe Klems
        //       DATE WRITTEN   August 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates those optical properties of all the Complex Fenestrations that
        //  depend on the beam direction (hence, on hour and time step)

        // METHODOLOGY EMPLOYED:
        // Locate the bidirectional property matrices in the BSDFInput structure
        // and use them to calculate the desired average properties.

        using namespace Vectors;

        int IConst; // State construction number
        int I;      // general purpose index--Back surface
        int J;      // general purpose index--ray
        int JRay;   // ray index number
        Real64 Theta;
        Real64 Phi;
        int JSurf;               // gen purpose surface no
        int BaseSurf;            // base surface no
        int M;                   // general purpose index--ray
        int L;                   // general purpose index--layer
        int KBkSurf;             // general purpose index--back surface
        Real64 Sum1;             // general purpose sum
        Real64 Sum2;             // general purpose sum
        int IBm;                 // index of beam ray in incoming basis
        int BkIncRay;            // index of sun dir in back incidence basis
        bool RegWindFnd;         // flag for regular exterior back surf window
        Array1D_int RegWinIndex; // bk surf nos of reg windows
        int NRegWin(0);          // no reg windows found as back surfaces
        int KRegWin(0);          // index of reg window as back surface
        Real64 Refl;             // temporary reflectance
        Array1D<Real64> Absorb;  // temporary layer absorptance

        // Object Data
        Vector SunDir; // current sun direction

        IConst = state.dataSurface->SurfaceWindow(ISurf).ComplexFen.State(IState).Konst;

        //  Begin calculation
        //  Calculate the Transmittance from a given beam direction to a given zone surface

        IBm = Geom.SolBmIndex(Hour, TS);
        if (IBm <= 0.0) { // Beam cannot be incident on window for this Hour, TS
            State.WinToSurfBmTrans(Hour, TS, {1, Window.NBkSurf}) = 0.0;
            State.WinDirHemiTrans(Hour, TS) = 0.0;
            State.WinDirSpecTrans(Hour, TS) = 0.0;
            State.WinBmFtAbs(Hour, TS, {1, State.NLayers}) = 0.0;
        } else {
            for (I = 1; I <= Window.NBkSurf; ++I) { // Back surface loop
                Sum1 = 0.0;
                for (J = 1; J <= Geom.NSurfInt(I); ++J) { // Ray loop
                    Sum1 +=
                        Geom.Trn.Lamda(Geom.SurfInt(J, I)) * state.dataConstruction->Construct(IConst).BSDFInput.SolFrtTrans(IBm, Geom.SurfInt(J, I));
                } // Ray loop
                State.WinToSurfBmTrans(Hour, TS, I) = Sum1;
            } // Back surface loop
            // Calculate the directional-hemispherical transmittance
            Sum1 = 0.0;
            for (J = 1; J <= Geom.Trn.NBasis; ++J) {
                Sum1 += Geom.Trn.Lamda(J) * state.dataConstruction->Construct(IConst).BSDFInput.SolFrtTrans(IBm, J);
            }
            State.WinDirHemiTrans(Hour, TS) = Sum1;
            // Calculate the directional specular transmittance
            // Note:  again using assumption that Inc and Trn basis have same structure
            State.WinDirSpecTrans(Hour, TS) = Geom.Trn.Lamda(IBm) * state.dataConstruction->Construct(IConst).BSDFInput.SolFrtTrans(IBm, IBm);
            // Calculate the layer front absorptance for beam radiation
            for (L = 1; L <= State.NLayers; ++L) {
                State.WinBmFtAbs(Hour, TS, L) = state.dataConstruction->Construct(IConst).BSDFInput.Layer(L).FrtAbs(IBm, 1);
            }
        }
        // Calculate,  for a given beam direction, the transmittance into the zone
        // for ground-reflected radiation (transmitted radiation assumed uniformly diffuse)

        Sum1 = 0.0;
        Sum2 = 0.0;
        for (J = 1; J <= Geom.NGnd; ++J) { // Incident ray loop
            JRay = Geom.GndIndex(J);
            if (Geom.SolBmGndWt(Hour, TS, J) > 0.0) {
                Sum2 += Geom.SolBmGndWt(Hour, TS, J) * Geom.Inc.Lamda(JRay);
                for (M = 1; M <= Geom.Trn.NBasis; ++M) { // Outgoing ray loop
                    Sum1 += Geom.SolBmGndWt(Hour, TS, J) * Geom.Inc.Lamda(JRay) * Geom.Trn.Lamda(M) *
                            state.dataConstruction->Construct(IConst).BSDFInput.SolFrtTrans(JRay, M);
                } // Outgoing ray loop
            }
        } // Indcident ray loop
        if (Sum2 > 0.0) {
            State.WinBmGndTrans(Hour, TS) = Sum1 / Sum2;
        } else {
            State.WinBmGndTrans(Hour, TS) = 0.0; // No unshaded ground => no transmittance
        }

        // Calculate,  for a given beam direction, the layer front absorptance
        // for ground-reflected radiation

        for (L = 1; L <= State.NLayers; ++L) { // layer loop
            Sum1 = 0.0;
            Sum2 = 0.0;
            for (J = 1; J <= Geom.NGnd; ++J) { // Incident ray loop
                JRay = Geom.GndIndex(J);
                if (Geom.SolBmGndWt(Hour, TS, J) > 0.0) {
                    Sum2 += Geom.SolBmGndWt(Hour, TS, J) * Geom.Inc.Lamda(JRay);
                    Sum1 += Geom.SolBmGndWt(Hour, TS, J) * Geom.Inc.Lamda(JRay) *
                            state.dataConstruction->Construct(IConst).BSDFInput.Layer(L).FrtAbs(JRay, 1);
                }
            } // Incident ray loop
            if (Sum2 > 0.0) {
                State.WinBmGndAbs(Hour, TS, L) = Sum1 / Sum2;
            } else {
                State.WinBmGndAbs(Hour, TS, L) = 0.0; // No unshaded ground => no absorptance
            }
        } // layer loop

        // Check the back surfaces for exterior windows
        RegWindFnd = false;
        NRegWin = 0.0;
        RegWinIndex.allocate(Window.NBkSurf);
        for (KBkSurf = 1; KBkSurf <= Window.NBkSurf; ++KBkSurf) {
            BaseSurf = state.dataSurface->Surface(ISurf).BaseSurf; // ShadowComb is organized by base surface
            JSurf = state.dataShadowComb->ShadowComb(BaseSurf).BackSurf(KBkSurf);
            if (state.dataSurface->SurfWinWindowModelType(JSurf) == WindowModel::BSDF) {
                continue;
            }
            if (!(state.dataSurface->Surface(JSurf).Class == SurfaceClass::Window ||
                  state.dataSurface->Surface(JSurf).Class == SurfaceClass::GlassDoor)) {
                continue;
            }
            if (!(state.dataSurface->Surface(JSurf).HeatTransSurf && state.dataSurface->Surface(JSurf).ExtBoundCond == ExternalEnvironment &&
                  state.dataSurface->Surface(JSurf).ExtSolar)) {
                continue;
            }
            // Back surface is an exterior window or door
            RegWindFnd = true;
            ++NRegWin;
            RegWinIndex(NRegWin) = KBkSurf;
        }
        if (RegWindFnd) {
            Absorb.allocate(State.NLayers);
            SunDir = state.dataBSDFWindow->SUNCOSTS(TS, Hour);
            BkIncRay = FindInBasis(state,
                                   SunDir,
                                   RayIdentificationType::Back_Incident,
                                   ISurf,
                                   IState,
                                   state.dataBSDFWindow->ComplexWind(ISurf).Geom(IState).Trn,
                                   Theta,
                                   Phi);
            if (BkIncRay > 0) {
                // Here calculate the back incidence properties for the solar ray
                // this does not say whether or not the ray can pass through the
                // back surface window and hit this one!
                Sum1 = 0.0;
                for (J = 1; J <= Geom.Trn.NBasis; ++J) {
                    Sum1 += Geom.Trn.Lamda(J) * state.dataConstruction->Construct(IConst).BSDFInput.SolBkRefl(BkIncRay, J);
                }
                Refl = Sum1;
                for (L = 1; L <= State.NLayers; ++L) {
                    Absorb(L) = state.dataConstruction->Construct(IConst).BSDFInput.Layer(L).BkAbs(BkIncRay, 1);
                }
            } else {
                // solar ray can't be incident on back, so set properties equal to zero
                Refl = 0.0;
                for (L = 1; L <= State.NLayers; ++L) {
                    Absorb(L) = 0.0;
                }
            }
            for (KRegWin = 1; KRegWin <= NRegWin; ++KRegWin) {
                KBkSurf = RegWinIndex(KRegWin);
                State.BkSurf(KBkSurf).WinDHBkRefl(Hour, TS) = Refl;
                for (L = 1; L <= State.NLayers; ++L) {
                    State.BkSurf(KBkSurf).WinDirBkAbs(Hour, TS, L) = Absorb(L);
                }
            }
        }
        if (allocated(Absorb)) {
            Absorb.deallocate();
        }
        RegWinIndex.deallocate();
    }

    void CalcStaticProperties(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Joe Klems
        //       DATE WRITTEN   <date_written>
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates those optical properties of all the Complex Fenestrations that
        // do not depend on the beam direction (hence, on hour and time step)

        using namespace Vectors;

        //       ISurf(0);     // Index for sorting thru Surface array
        //                     //        static int IConst( 0 ); // Index for accessing Construct array
        //       IState(0);    // Index identifying the window state for a particular window
        //       IWind(0);     // Index identifying a window in the WindowList
        //       NumStates(0); // local copy of no of states

        for (int IWind = 1; IWind <= state.dataWindowComplexManager->NumComplexWind; ++IWind) {
            int ISurf = state.dataWindowComplexManager->WindowList(IWind).SurfNo;
            int NumStates = state.dataWindowComplexManager->WindowList(IWind).NumStates;
            for (int IState = 1; IState <= NumStates; ++IState) {
                // IConst = WindowStateList ( IWind , IState )%Konst
                state.dataSurface->SurfaceWindow(ISurf).ComplexFen.State(IState).Konst =
                    state.dataWindowComplexManager->WindowStateList(IState, IWind).Konst;
                CalcWindowStaticProperties(state,
                                           ISurf,
                                           IState,
                                           state.dataBSDFWindow->ComplexWind(ISurf),
                                           state.dataBSDFWindow->ComplexWind(ISurf).Geom(IState),
                                           state.dataSurface->SurfaceWindow(ISurf).ComplexFen.State(IState));
            }
        }
    }

    void CalculateBasisLength(EnergyPlusData &state,
                              BSDFWindowInputStruct const &Input, // BSDF data input struct for this construction
                              int const IConst,                   // Construction number of input
                              int &NBasis                         // Calculated Basis length
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Joe Klems
        //       DATE WRITTEN   August 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the basis length for a Window6 Non-Symmetric or Axisymmetric basis
        // from the input basis matrix

        if (Input.BasisMatNcols == 1) {
            // Axisymmetric basis, No. rows is no. of thetas = basis length
            NBasis = Input.BasisMatNrows;
            return;
        }
        NBasis = 1;
        for (int I = 2; I <= Input.BasisMatNrows; ++I) {
            NBasis += std::floor(state.dataConstruction->Construct(IConst).BSDFInput.BasisMat(2, I) + 0.001);
        }
    }

    void ConstructBasis(EnergyPlusData &state,
                        int const IConst, // Index for accessing Construct array
                        BasisStruct &Basis)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Joe Klems
        //       DATE WRITTEN  June 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set up a basis from the matrix information pointed to in Construction by ICons

        int I(0);               // general purpose index
        int J(0);               // general purpose index
        int NThetas(0);         // Current number of theta values
        int NumElem(0);         // Number of elements in current basis
        int ElemNo(0);          // Current basis element number
        int MaxNPhis;           // Max no of NPhis for any theta
        Real64 Theta(0.0);      // Current theta value
        Real64 Phi(0.0);        // Current phi value
        Real64 DTheta(0.0);     // Increment for theta value (Window6 type input)
        Real64 DPhi(0.0);       // Increment for phi value (Window6 type input)
        Real64 HalfDTheta(0.0); // Half-width of all theta bins except first and last (W6 input)
        Real64 Lamda(0.0);      // Current 'Lamda' value (element weight)
        Real64 SolAng(0.0);     // Current element solid angle
        Real64 NextTheta(0.0);  // Next theta in the W6 basis after current
        Real64 LastTheta(0.0);  // Previous theta in the W6 basis before current
        Real64 LowerTheta(0.0); // Lower theta boundary of the element
        Real64 UpperTheta(0.0); // Upper theta boundary of the element
        Array1D<Real64> Thetas; // temp array holding theta values
        Array1D_int NPhis;      // temp array holding number of phis for a given theta

        NThetas = state.dataConstruction->Construct(IConst).BSDFInput.BasisMatNrows; // Note here assuming row by row input
        Basis.NThetas = NThetas;
        Basis.BasisMatIndex = state.dataConstruction->Construct(IConst).BSDFInput.BasisMatIndex;
        Basis.NBasis = state.dataConstruction->Construct(IConst).BSDFInput.NBasis;
        Basis.Grid.allocate(Basis.NBasis);
        Thetas.allocate(NThetas + 1); // Temp array
        // By convention the Thetas array contains a final point at Pi/2 which is not a basis element
        NPhis.allocate(NThetas + 1); // Temp array
        Basis.Thetas.allocate(NThetas + 1);
        Basis.NPhis.allocate(NThetas + 1);

        Basis.Lamda.allocate(state.dataConstruction->Construct(IConst).BSDFInput.NBasis);
        Basis.SolAng.allocate(state.dataConstruction->Construct(IConst).BSDFInput.NBasis);
        if (state.dataConstruction->Construct(IConst).BSDFInput.BasisType == DataBSDFWindow::Basis::WINDOW) {
            //   WINDOW6 Basis
            Basis.BasisType = DataBSDFWindow::Basis::WINDOW;
            if (state.dataConstruction->Construct(IConst).BSDFInput.BasisSymmetryType == DataBSDFWindow::BasisSymmetry::None) {
                // No basis symmetry
                Basis.BasisSymmetryType = DataBSDFWindow::BasisSymmetry::None;
                Thetas(1) = 0.0;                          // By convention, the first basis point is at the center (theta=0,phi=0)
                Thetas(NThetas + 1) = 0.5 * Constant::Pi; // and there is an N+1st point (not a basis element) at Pi/2
                NPhis(1) = 1;
                NumElem = 1;
                for (I = 2; I <= NThetas; ++I) {
                    Thetas(I) = state.dataConstruction->Construct(IConst).BSDFInput.BasisMat(1, I) * Constant::DegToRad;
                    NPhis(I) = std::floor(state.dataConstruction->Construct(IConst).BSDFInput.BasisMat(2, I) + 0.001);
                    if (NPhis(I) <= 0) {
                        ShowFatalError(state, "WindowComplexManager: incorrect input, no. phis must be positive.");
                    }
                    NumElem += NPhis(I);
                }
                MaxNPhis = maxval(NPhis({1, NThetas}));
                Basis.Phis.allocate(NThetas + 1, MaxNPhis + 1); // N+1st Phi point (not basis element) at 2Pi
                Basis.BasisIndex.allocate(MaxNPhis, NThetas + 1);
                Basis.Phis = 0.0;                                                            // Initialize so undefined elements will contain zero
                Basis.BasisIndex = 0;                                                        // Initialize so undefined elements will contain zero
                if (NumElem != state.dataConstruction->Construct(IConst).BSDFInput.NBasis) { // Constructed Basis must match property matrices
                    ShowFatalError(state, "WindowComplexManager: Constructed basis length does not match property matrices.");
                }
                Basis.Thetas = Thetas;
                Basis.NPhis = NPhis;
                ElemNo = 0;
                for (I = 1; I <= NThetas; ++I) {
                    Theta = Thetas(I);
                    if (I == 1) { // First theta value must always be zero
                        HalfDTheta = 0.5 * Thetas(I + 1);
                        LastTheta = 0.0;
                        NextTheta = Thetas(I + 1);
                        LowerTheta = 0.0;
                        UpperTheta = HalfDTheta;
                    } else if (I > 1 && I < NThetas) {
                        LastTheta = Thetas(I - 1);
                        NextTheta = Thetas(I + 1);
                        LowerTheta = UpperTheta;
                        HalfDTheta = Theta - LowerTheta;
                        UpperTheta = Theta + HalfDTheta;
                    } else if (I == NThetas) {
                        LastTheta = Thetas(I - 1);
                        NextTheta = 0.5 * Constant::Pi;
                        LowerTheta = UpperTheta; // It is assumed that Thetas(N) is the mean between the previous
                        // UpperTheta and pi/2.
                        UpperTheta = 0.5 * Constant::Pi;
                    }
                    DPhi = 2.0 * Constant::Pi / NPhis(I);
                    if (I == 1) {
                        Lamda = Constant::Pi * pow_2(std::sin(UpperTheta));
                        SolAng = 2.0 * Constant::Pi * (1.0 - std::cos(UpperTheta));
                    } else {
                        Lamda = 0.5 * DPhi * (pow_2(std::sin(UpperTheta)) - pow_2(std::sin(LowerTheta))); // For W6 basis, lamda is funct of Theta and
                        // NPhis, not individual Phi
                        SolAng = DPhi * (std::cos(LowerTheta) - std::cos(UpperTheta));
                    }
                    DTheta = UpperTheta - LowerTheta;
                    Basis.Phis(I, NPhis(I) + 1) = 2.0 * Constant::Pi; // Non-basis-element Phi point for table searching in Phi
                    for (J = 1; J <= NPhis(I); ++J) {
                        ++ElemNo;
                        Basis.BasisIndex(J, I) = ElemNo;
                        Phi = (J - 1) * DPhi;
                        Basis.Phis(I, J) = Phi; // Note: this ordering of I & J are necessary to allow Phis(Theta) to
                        //  be searched as a one-dimensional table
                        FillBasisElement(state,
                                         Theta,
                                         Phi,
                                         ElemNo,
                                         Basis.Grid(ElemNo),
                                         LowerTheta,
                                         UpperTheta,
                                         DPhi,
                                         DataBSDFWindow::Basis::WINDOW); // This gets all the simple grid characteristics
                        Basis.Lamda(ElemNo) = Lamda;
                        Basis.SolAng(ElemNo) = SolAng;
                    }
                }
            } else { // BST
                //  Axisymmetric basis symmetry (Note this only useful specular systems, where it allows shorter data input)
                Basis.BasisSymmetryType = DataBSDFWindow::BasisSymmetry::Axisymmetric;
                Thetas(1) = 0.0;                          // By convention, the first basis point is at the center (theta=0,phi=0)
                Thetas(NThetas + 1) = 0.5 * Constant::Pi; // and there is an N+1st point (not a basis element) at Pi/2
                NPhis = 1;                                // As insurance, define one phi for each theta
                NumElem = 1;
                for (I = 2; I <= NThetas; ++I) {
                    Thetas(I) = state.dataConstruction->Construct(IConst).BSDFInput.BasisMat(1, I) * Constant::DegToRad;
                    ++NumElem;
                }
                Basis.Phis.allocate(1, NThetas);
                Basis.BasisIndex.allocate(1, NThetas);
                Basis.Phis = 0.0;                                                            // Initialize so undefined elements will contain zero
                Basis.BasisIndex = 0;                                                        // Initialize so undefined elements will contain zero
                if (NumElem != state.dataConstruction->Construct(IConst).BSDFInput.NBasis) { // Constructed Basis must match property matrices
                    ShowFatalError(state, "WindowComplexManager: Constructed basis length does not match property matrices.");
                }
                Basis.Thetas = Thetas;
                Basis.NPhis = NPhis;
                ElemNo = 0;
                DPhi = 2.0 * Constant::Pi;
                for (I = 1; I <= NThetas; ++I) {
                    Theta = Thetas(I);
                    if (I == 1) { // First theta value must always be zero
                        HalfDTheta = 0.5 * Thetas(I + 1);
                        LastTheta = 0.0;
                        NextTheta = Thetas(I + 1);
                        LowerTheta = 0.0;
                        UpperTheta = HalfDTheta;
                    } else if (I > 1 && I < NThetas) {
                        LastTheta = Thetas(I - 1);
                        NextTheta = Thetas(I + 1);
                        LowerTheta = UpperTheta;
                        HalfDTheta = Theta - LowerTheta;
                        UpperTheta = Theta + HalfDTheta;
                    } else if (I == NThetas) {
                        LastTheta = Thetas(I - 1);
                        NextTheta = 0.5 * Constant::Pi;
                        LowerTheta = UpperTheta; // It is assumed that Thetas(N) is the mean between the previous
                        // UpperTheta and pi/2.
                        UpperTheta = 0.5 * Constant::Pi;
                    }
                    if (I == 1) {
                        Lamda = Constant::Pi * pow_2(std::sin(UpperTheta));
                        SolAng = 2.0 * Constant::Pi * (1.0 - std::cos(UpperTheta));
                    } else {
                        Lamda = 0.5 * DPhi * (pow_2(std::sin(UpperTheta)) - pow_2(std::sin(LowerTheta))); // For W6 basis, lamda is funct of Theta and
                        // NPhis, not individual Phi
                        SolAng = DPhi * (std::cos(LowerTheta) - std::cos(UpperTheta));
                    }
                    DTheta = UpperTheta - LowerTheta;
                    ++ElemNo;
                    Basis.BasisIndex(1, I) = ElemNo;
                    Phi = 0.0;
                    Basis.Phis(I, 1) = Phi; // Note: this ordering of I & J are necessary to allow Phis(Theta) to
                    //  be searched as a one-dimensional table
                    FillBasisElement(state,
                                     Theta,
                                     Phi,
                                     ElemNo,
                                     Basis.Grid(ElemNo),
                                     LowerTheta,
                                     UpperTheta,
                                     DPhi,
                                     DataBSDFWindow::Basis::WINDOW); // This gets all the simple grid characteristics
                    Basis.Lamda(ElemNo) = Lamda;
                    Basis.SolAng(ElemNo) = SolAng;
                }
            } // BST
        } else { // BTW
            ShowFatalError(state, "WindowComplexManager: Non-Window6 basis type not yet implemented.");
        } // BTW
        Thetas.deallocate();
        NPhis.deallocate();
    }

    void FillBasisElement(EnergyPlusData &state,
                          Real64 const Theta, // Central polar angle of element
                          Real64 const Phi,   // Central azimuthal angle of element
                          int const Elem,     // Index number of element in basis
                          BasisElemDescr &BasisElem,
                          Real64 const LowerTheta,              // Lower edge of element (polar angle)
                          Real64 const UpperTheta,              // Upper edge of element (polar angle)
                          Real64 const DPhi,                    // Width of element (azimuthal angle)
                          DataBSDFWindow::Basis const InputType // Basis type
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Joe Klems
        //       DATE WRITTEN   August 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // fill in values for all the components of a basis element

        if (InputType == DataBSDFWindow::Basis::WINDOW) {
            // WINDOW6 Type BASIS
            if (Elem == 1) {
                // first element, theta=0, is special case
                BasisElem.Theta = Theta;
                BasisElem.Phi = 0.0;
                BasisElem.dPhi = 2.0 * Constant::Pi;
                BasisElem.UpprTheta = UpperTheta;
                BasisElem.dTheta = BasisElem.UpprTheta - Theta;
                BasisElem.LwrTheta = Theta;
                BasisElem.LwrPhi = 0.0;
                BasisElem.UpprPhi = 2.0 * Constant::Pi;
            } else {
                BasisElem.Theta = Theta;
                BasisElem.Phi = Phi;
                BasisElem.dPhi = DPhi;
                BasisElem.LwrPhi = Phi - DPhi / 2.0;
                BasisElem.UpprPhi = Phi + DPhi / 2.0;
                BasisElem.LwrTheta = LowerTheta;
                BasisElem.UpprTheta = UpperTheta;
                BasisElem.dTheta = BasisElem.UpprTheta - BasisElem.LwrTheta;
            }
        } else {
            // Non-WINDOW6 Type Basis
            // Currently not implemented
            ShowFatalError(state, "WindowComplexManager: Custom basis type not yet implemented.");
        }
    }

    void SetupComplexWindowStateGeometry(EnergyPlusData &state,
                                         int const ISurf,                       // Surface number of the complex fenestration
                                         int const IState,                      // State number of the complex fenestration state
                                         int const IConst,                      // Pointer to construction for this state
                                         BSDFWindowGeomDescr &Window,           // Window Geometry
                                         BSDFGeomDescr &Geom,                   // State Geometry
                                         [[maybe_unused]] BSDFStateDescr &State // State Description
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         J. Klems
        //       DATE WRITTEN   June 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Define all the geometric quantites for a complex fenestration state

        using namespace Vectors;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // INTEGER, INTENT(IN)      ::  IWind            !Complex fenestration number (in window list)

        Real64 Azimuth; // Complex fenestration azimuth
        Real64 Tilt;    // Complex fenestration tilt
        int ElemNo;     // Grid index variable
        bool hit;       // Surface intersection flag
        int I;          // Temp Indices
        int J;
        int IRay;                    // Ray index variable
        int IZone;                   // Zone containing the complex window
        int JSurf;                   // Secondary Surface index
        int BaseSurf;                // base surface index
        int KBkSurf;                 // Back surface index
        int MaxHits;                 // Max no of hits found
        int MaxInt;                  // Max no of intersections found
        int NSky;                    // No of sky rays
        int NGnd;                    // No of gnd rays
        int NReflSurf;               // No of rays striking ext surfaces
        int NBkSurf;                 // No of back surfaces
        int TotHits;                 // Current number of surface intersections
        Real64 Theta;                // Basis theta angle
        Real64 Phi;                  // Basis phi angle
        Real64 HitDsq;               // Squared distance to current hit pt
        Real64 LeastHitDsq;          // Squared distance to closest hit pt
        Array1D<Real64> V(3);        // vector array
        Array1D_int TmpRfSfInd;      // Temporary RefSurfIndex
        Array1D_int TmpRfRyNH;       // Temporary RefRayNHits
        Array2D_int TmpHSurfNo;      // Temporary HitSurfNo
        Array2D<Real64> TmpHSurfDSq; // Temporary HitSurfDSq
        Array1D_int TmpSkyInd;       // Temporary sky index list
        Array1D_int TmpGndInd;       // Temporary gnd index list
        Array2D_int TmpSurfInt;      // Temporary index of ray intersecing back surf
        Array2D<Real64> TmpSjdotN;   // Temporary dot prod of ray angle w bk surf norm
        Array1D_int ITemp1D;         // Temporary INT 1D array
        Array2D<Real64> Temp2D;      // Temporary real 2D array
        Real64 TransRSurf;           // Norminal transmittance of shading surface
        Real64 WtSum;                // Sum for normalizing various weights
        Real64 DotProd;              // Temporary variable for manipulating dot product .dot.

        struct BackHitList
        {
            // Members
            int KBkSurf;   // Back surface index of the hit surface
            int HitSurf;   // Surface number of the hit surface
            Vector HitPt;  // coords of hit pt (world syst)
            Real64 HitDsq; // Squared distance to the current hit pt

            // Default Constructor
            BackHitList() : KBkSurf(0), HitSurf(0), HitDsq(0.0)
            {
            }
        };

        // Object Data
        Vector HitPt;             // coords of hit pt (world syst)
        Vector X;                 // position vector
        Vector VecNorm;           // outer normal vector
        Array1D<Vector> TmpGndPt; // Temporary ground intersection list
        Array2D<Vector> TempV2D;  // Temporary vector 2D array
        Array2D<Vector> TmpHitPt; // Temporary HitPt
        BackHitList BSHit;        // Temp list of back surface hit quantities for a ray

        // This routine primarily fills in the BSDFGeomDescr type for a given window and state
        // Note that on call the incoming and outgoing basis structure types have already been filled in
        //  Define the central ray directions (in world coordinate system)

        state.dataSurface->SurfaceWindow(ISurf).ComplexFen.State(IState).NLayers = state.dataConstruction->Construct(IConst).BSDFInput.NumLayers;
        Azimuth = Constant::DegToRad * state.dataSurface->Surface(ISurf).Azimuth;
        Tilt = Constant::DegToRad * state.dataSurface->Surface(ISurf).Tilt;

        // For incoming grid

        Geom.sInc.allocate(Geom.Inc.NBasis);
        Geom.sInc = Vector(0.0, 0.0, 0.0);
        Geom.pInc.allocate(Geom.Inc.NBasis);
        Geom.CosInc.allocate(Geom.Inc.NBasis);
        Geom.DAInc.allocate(Geom.Inc.NBasis);
        Geom.pInc = BSDFDaylghtPosition(0.0, 0.0);
        for (ElemNo = 1; ElemNo <= Geom.Inc.NBasis; ++ElemNo) {
            Theta = Geom.Inc.Grid(ElemNo).Theta;
            Phi = Geom.Inc.Grid(ElemNo).Phi;
            // The following puts in the vectors depending on
            // window orientation
            Geom.sInc(ElemNo) = WorldVectFromW6(state, Theta, Phi, RayIdentificationType::Front_Incident, Tilt, Azimuth);
            Geom.pInc(ElemNo) = DaylghtAltAndAzimuth(Geom.sInc(ElemNo));

            Geom.CosInc(ElemNo) = std::cos(Geom.Inc.Grid(ElemNo).Theta);
            // Geom%DAInc(ElemNo) = COS(Geom%pInc(ElemNo)%Altitude) * Geom%Inc%Grid(ElemNo)%dTheta * Geom%Inc%Grid(ElemNo)%dPhi
            // Geom%DAInc(ElemNo) = Geom%Inc%Grid(ElemNo)%dTheta * Geom%Inc%Grid(ElemNo)%dPhi
            Geom.DAInc(ElemNo) = std::cos(Geom.Inc.Grid(ElemNo).Theta) * Geom.Inc.Grid(ElemNo).dTheta * Geom.Inc.Grid(ElemNo).dPhi;
        }
        //  For outgoing grid
        Geom.sTrn.allocate(Geom.Trn.NBasis);
        Geom.sTrn = Vector(0.0, 0.0, 0.0);
        Geom.pTrn.allocate(Geom.Trn.NBasis);
        Geom.pTrn = BSDFDaylghtPosition(0.0, 0.0);
        for (ElemNo = 1; ElemNo <= Geom.Trn.NBasis; ++ElemNo) {
            Theta = Geom.Trn.Grid(ElemNo).Theta;
            Phi = Geom.Trn.Grid(ElemNo).Phi;
            // The following puts in the vectors depending on
            // window orientation
            Geom.sTrn(ElemNo) = WorldVectFromW6(state, Theta, Phi, RayIdentificationType::Front_Transmitted, Tilt, Azimuth);
            Geom.pTrn(ElemNo) = DaylghtAltAndAzimuth(Geom.sTrn(ElemNo));
        }
        //  Incident Basis:
        //  Construct sky and ground ray index maps, and list of rays intersecting exterior surfaces
        // Sky, and ground ray index maps, and rays that are potentially beam radiation reflected from exterior surfaces
        TmpRfSfInd.allocate(Geom.Inc.NBasis);
        TmpRfRyNH.allocate(Geom.Inc.NBasis);
        TmpHSurfNo.allocate(state.dataSurface->TotSurfaces, Geom.Inc.NBasis);
        TmpHSurfDSq.allocate(state.dataSurface->TotSurfaces, Geom.Inc.NBasis);
        TmpHitPt.allocate(state.dataSurface->TotSurfaces, Geom.Inc.NBasis);
        TmpSkyInd.allocate(Geom.Inc.NBasis);
        TmpGndInd.allocate(Geom.Inc.NBasis);
        TmpGndPt.allocate(Geom.Inc.NBasis);
        NSky = 0;
        NGnd = 0;
        NReflSurf = 0;
        TmpRfRyNH = 0;
        Geom.NSkyUnobs = 0;
        Geom.NGndUnobs = 0;
        //  Note--this loop could be repeated for different positions in the window plane (as for detailed reflection
        //  calculations, varying the origin in the call to PierceSurface.  Essentially, have set NsubV =1.
        for (IRay = 1; IRay <= Geom.Inc.NBasis; ++IRay) {
            if (Geom.sInc(IRay).z < 0.0) {
                // A ground ray
                ++Geom.NGndUnobs;
            } else {
                // A sky ray
                ++Geom.NSkyUnobs;
            }
            // Exterior reveal shadowing/reflection treatment should be inserted here
            TotHits = 0;
            for (JSurf = 1; JSurf <= state.dataSurface->TotSurfaces; ++JSurf) {
                // the following test will cycle on anything except exterior surfaces and shading surfaces
                if (state.dataSurface->Surface(JSurf).HeatTransSurf && state.dataSurface->Surface(JSurf).ExtBoundCond != ExternalEnvironment) {
                    continue;
                }
                //  skip the base surface containing the window and any other subsurfaces of that surface
                if (JSurf == state.dataSurface->Surface(ISurf).BaseSurf ||
                    state.dataSurface->Surface(JSurf).BaseSurf == state.dataSurface->Surface(ISurf).BaseSurf) {
                    continue;
                }
                //  skip surfaces that face away from the window
                DotProd = dot(Geom.sInc(IRay), state.dataSurface->Surface(JSurf).NewellSurfaceNormalVector);
                if (DotProd >= 0.0) {
                    continue;
                }
                hit = PierceSurface(state, JSurf, state.dataSurface->Surface(ISurf).Centroid, Geom.sInc(IRay), HitPt);
                if (!hit) {
                    continue; // Miss: Try next surface
                }
                if (TotHits == 0) {
                    //  First hit for this ray
                    TotHits = 1;
                    ++NReflSurf;
                    TmpRfSfInd(NReflSurf) = IRay;
                    TmpRfRyNH(NReflSurf) = 1;
                    TmpHSurfNo(1, NReflSurf) = JSurf;
                    TmpHitPt(1, NReflSurf) = HitPt;
                    V = HitPt - state.dataSurface->Surface(ISurf).Centroid; // vector array from window ctr to hit pt
                    LeastHitDsq = magnitude_squared(V);                     // dist^2 window ctr to hit pt
                    TmpHSurfDSq(1, NReflSurf) = LeastHitDsq;
                    if (!state.dataSurface->Surface(JSurf).HeatTransSurf && state.dataSurface->Surface(JSurf).shadowSurfSched != nullptr) {
                        TransRSurf = 1.0; // If a shadowing surface may have a scheduled transmittance,
                        //   treat it here as completely transparent
                    } else {
                        TransRSurf = 0.0;
                    }
                } else {
                    V = HitPt - state.dataSurface->Surface(ISurf).Centroid;
                    HitDsq = magnitude_squared(V);
                    if (HitDsq >= LeastHitDsq) {
                        if (TransRSurf > 0.0) { // forget the new hit if the closer hit is opaque
                            J = TotHits + 1;
                            if (TotHits > 1) {
                                for (I = 2; I <= TotHits; ++I) {
                                    if (HitDsq < TmpHSurfDSq(I, NReflSurf)) {
                                        J = I;
                                        break;
                                    }
                                }
                                if (!state.dataSurface->Surface(JSurf).HeatTransSurf &&
                                    state.dataSurface->Surface(JSurf).shadowSurfSched == nullptr) {
                                    //  The new hit is opaque, so we can drop all the hits further away
                                    TmpHSurfNo(J, NReflSurf) = JSurf;
                                    TmpHitPt(J, NReflSurf) = HitPt;
                                    TmpHSurfDSq(J, NReflSurf) = HitDsq;
                                    TotHits = J;
                                } else {
                                    //  The new hit is scheduled (presumed transparent), so keep the more distant hits
                                    //     Note that all the hists in the list will be transparent except the last,
                                    //       which may be either transparent or opaque
                                    if (TotHits >= J) {
                                        for (I = TotHits; I >= J; --I) {
                                            TmpHSurfNo(I + 1, NReflSurf) = TmpHSurfNo(I, NReflSurf);
                                            TmpHitPt(I + 1, NReflSurf) = TmpHitPt(I, NReflSurf);
                                            TmpHSurfDSq(I + 1, NReflSurf) = TmpHSurfDSq(I, NReflSurf);
                                        }
                                        TmpHSurfNo(J, NReflSurf) = JSurf;
                                        TmpHitPt(J, NReflSurf) = HitPt;
                                        TmpHSurfDSq(J, NReflSurf) = HitDsq;
                                        ++TotHits;
                                    }
                                }
                            }
                        }
                    } else {
                        //  A new closest hit.  If it is opaque, drop the current hit list,
                        //    otherwise add it at the front
                        LeastHitDsq = HitDsq;
                        if (!state.dataSurface->Surface(JSurf).HeatTransSurf && state.dataSurface->Surface(JSurf).shadowSurfSched != nullptr) {
                            TransRSurf = 1.0; // New closest hit is transparent, keep the existing hit list
                            for (I = TotHits; I >= 1; --I) {
                                TmpHSurfNo(I + 1, NReflSurf) = TmpHSurfNo(I, NReflSurf);
                                TmpHitPt(I + 1, NReflSurf) = TmpHitPt(I, NReflSurf);
                                TmpHSurfDSq(I + 1, NReflSurf) = TmpHSurfDSq(I, NReflSurf);
                                ++TotHits;
                            }
                        } else {
                            TransRSurf = 0.0; // New closest hit is opaque, drop the existing hit list
                            TotHits = 1;
                        }
                        TmpHSurfNo(1, NReflSurf) = JSurf; // In either case the new hit is put in position 1
                        TmpHitPt(1, NReflSurf) = HitPt;
                        TmpHSurfDSq(1, NReflSurf) = LeastHitDsq;
                    }
                }
            } // End of loop over surfaces
            if (TotHits <= 0) {
                // This ray reached the sky or ground unobstructed
                if (Geom.sInc(IRay).z < 0.0) {
                    // A ground ray
                    ++NGnd;
                    TmpGndInd(NGnd) = IRay;
                    TmpGndPt(NGnd).x = state.dataSurface->Surface(ISurf).Centroid.x -
                                       (Geom.sInc(IRay).x / Geom.sInc(IRay).z) * state.dataSurface->Surface(ISurf).Centroid.z;
                    TmpGndPt(NGnd).y = state.dataSurface->Surface(ISurf).Centroid.y -
                                       (Geom.sInc(IRay).y / Geom.sInc(IRay).z) * state.dataSurface->Surface(ISurf).Centroid.z;
                    TmpGndPt(NGnd).z = 0.0;
                } else {
                    // A sky ray
                    ++NSky;
                    TmpSkyInd(NSky) = IRay;
                }
            } else {
                // Save the number of hits for this ray
                TmpRfRyNH(NReflSurf) = TotHits;
            }
        } // End of loop over basis rays
        // Store results of indexing the incident basis for this window
        Geom.NSky = NSky;
        Geom.NGnd = NGnd;
        Geom.NReflSurf = NReflSurf;
        Geom.SkyIndex.allocate(NSky);
        Geom.SkyIndex = TmpSkyInd({1, NSky});
        TmpSkyInd.deallocate();
        Geom.GndIndex.allocate(NGnd);
        Geom.GndPt.allocate(NGnd);
        Geom.GndIndex = TmpGndInd({1, NGnd});
        Geom.GndPt = TmpGndPt({1, NGnd});
        TmpGndInd.deallocate();
        TmpGndPt.deallocate();
        MaxHits = maxval(TmpRfRyNH);
        Geom.RefSurfIndex.allocate(NReflSurf);
        Geom.RefRayNHits.allocate(NReflSurf);
        Geom.HitSurfNo.allocate(MaxHits, NReflSurf);
        Geom.HitSurfDSq.allocate(MaxHits, NReflSurf);
        Geom.HitPt.allocate(MaxHits, NReflSurf);
        Geom.RefSurfIndex = TmpRfSfInd({1, NReflSurf});
        Geom.RefRayNHits = TmpRfRyNH({1, NReflSurf});
        Geom.HitSurfNo = 0;
        Geom.HitSurfDSq = 0.0;
        Geom.HitPt = Vector(0.0, 0.0, 0.0);
        for (I = 1; I <= NReflSurf; ++I) {
            TotHits = TmpRfRyNH(I);
            Geom.HitSurfNo({1, TotHits}, I) = TmpHSurfNo({1, TotHits}, I);
            Geom.HitSurfDSq({1, TotHits}, I) = TmpHSurfDSq({1, TotHits}, I);
            Geom.HitPt({1, TotHits}, I) = TmpHitPt({1, TotHits}, I);
        }
        TmpRfRyNH.deallocate();
        TmpRfSfInd.deallocate();
        TmpHSurfNo.deallocate();
        TmpHSurfDSq.deallocate();
        TmpHitPt.deallocate();
        // In above scheme sky and ground rays are those that intesect no exterior surfaces.
        //  The list of hit points is compiled for later (future?) calculation
        //  of reflections from these surfaces.  The hit list for each ray includes all
        //   surfaces with schedulable transmittance intersected by the ray,
        //   in order of increasing distance, up to the first opaque surface.
        //  Rays that intesect one or more schedulable transmittance but no opaque
        //  surfaces (therefore may reach the sky or ground) are left out of the sky/ground
        //  calcuation.  A correction for these rays could/should be made after the
        //  shading calculation.
        // Now calculate weights for averaging the transmittance matrix
        // Sky Weights
        Geom.SolSkyWt.allocate(NSky);
        for (I = 1; I <= NSky; ++I) {
            J = Geom.SkyIndex(I);
            Geom.SolSkyWt(I) = SkyWeight(Geom.sInc(J));
        }
        WtSum = sum(Geom.SolSkyWt({1, NSky}));
        if (WtSum > Constant::rTinyValue) {
            Geom.SolSkyWt({1, NSky}) /= WtSum;
        } else {
            Geom.SolSkyWt({1, NSky}) = 0.0;
        }
        // SkyGround Weights
        Geom.SolSkyGndWt.allocate(NGnd);
        for (I = 1; I <= NGnd; ++I) {
            Geom.SolSkyGndWt(I) = SkyGndWeight(Geom.GndPt(I));
        }
        WtSum = sum(Geom.SolSkyGndWt({1, NGnd}));
        if (WtSum > Constant::rTinyValue) {
            Geom.SolSkyGndWt({1, NGnd}) /= WtSum;
        } else {
            Geom.SolSkyGndWt({1, NGnd}) = 0.0;
        }
        //  Weights for beam reflected from ground are calculated after shading
        //  interval is determined
        // Transmitted Basis:
        //  Construct back surface intersection maps
        IZone = state.dataSurface->Surface(ISurf).Zone;
        NBkSurf = Window.NBkSurf;
        Geom.NSurfInt.allocate(NBkSurf);
        Geom.NSurfInt = 0; // Initialize the number of intersections to zero
        TmpSurfInt.allocate(Geom.Trn.NBasis, NBkSurf);
        TmpSjdotN.allocate(Geom.Trn.NBasis, NBkSurf);
        // Find the intersections of the basis rays with the back surfaces
        for (IRay = 1; IRay <= Geom.Trn.NBasis; ++IRay) { // ray loop
            TotHits = 0;
            //  Insert treatment of intersection & reflection from interior reveals here
            for (KBkSurf = 1; KBkSurf <= NBkSurf; ++KBkSurf) {                        // back surf loop
                BaseSurf = state.dataSurface->Surface(ISurf).BaseSurf;                // ShadowComb is organized by base surface
                JSurf = state.dataShadowComb->ShadowComb(BaseSurf).BackSurf(KBkSurf); // these are all proper back surfaces
                hit = PierceSurface(state, JSurf, state.dataSurface->Surface(ISurf).Centroid, Geom.sTrn(IRay), HitPt);
                if (!hit) {
                    continue; // Miss: Try next surface
                }
                if (TotHits == 0) {
                    //  First hit for this ray
                    TotHits = 1;
                    BSHit.KBkSurf = KBkSurf;
                    BSHit.HitSurf = JSurf;
                    BSHit.HitPt = HitPt;
                    V = HitPt - state.dataSurface->Surface(ISurf).Centroid;
                    BSHit.HitDsq = magnitude_squared(V);
                } else if (BSHit.HitSurf == state.dataSurface->Surface(JSurf).BaseSurf) {
                    //  another hit, check whether this is a subsurface of a previously hit base surface
                    //  (which would be listed first in the Surface array)
                    //  if so, replace the previous hit with this one
                    ++TotHits;
                    BSHit.KBkSurf = KBkSurf;
                    BSHit.HitSurf = JSurf;
                    BSHit.HitPt = HitPt;
                    V = HitPt - state.dataSurface->Surface(ISurf).Centroid;
                    BSHit.HitDsq = magnitude_squared(V);
                } else {
                    ++TotHits;
                    // is the new hit closer than the previous one (i.e., zone not strictly convex)?
                    // if so, take the closer hit
                    V = HitPt - state.dataSurface->Surface(ISurf).Centroid;
                    HitDsq = magnitude_squared(V);
                    if (HitDsq < BSHit.HitDsq) {
                        BSHit.KBkSurf = KBkSurf;
                        BSHit.HitSurf = JSurf;
                        BSHit.HitPt = HitPt;
                        BSHit.HitDsq = HitDsq;
                    }
                }
            } // back surf loop
            if (TotHits == 0) {
                // this should not happen--means a ray has gotten lost
                // ShowWarningError(state, "BSDF--Zone surfaces do not completely enclose zone--transmitted ray lost");
            } else {
                KBkSurf = BSHit.KBkSurf;
                JSurf = BSHit.HitSurf;
                ++Geom.NSurfInt(KBkSurf);
                TmpSurfInt(Geom.NSurfInt(KBkSurf), KBkSurf) = IRay;
                VecNorm = state.dataSurface->Surface(JSurf).OutNormVec;
                TmpSjdotN(Geom.NSurfInt(KBkSurf), KBkSurf) = dot(Geom.sTrn(IRay), VecNorm);
            }
        } // ray loop
        //  All rays traced, now put away the results in the temporary arrays
        MaxInt = maxval(Geom.NSurfInt);
        Geom.SurfInt.allocate(MaxInt, Window.NBkSurf);
        Geom.SjdotN.allocate(MaxInt, Window.NBkSurf);
        Geom.SurfInt = 0;
        for (I = 1; I <= Window.NBkSurf; ++I) {
            Geom.SurfInt({1, Geom.NSurfInt(I)}, I) = TmpSurfInt({1, Geom.NSurfInt(I)}, I);
            Geom.SjdotN({1, Geom.NSurfInt(I)}, I) = TmpSjdotN({1, Geom.NSurfInt(I)}, I);
        }

        TmpSurfInt.deallocate();
        TmpSjdotN.deallocate();
    }

    void CalcWindowStaticProperties(EnergyPlusData &state,
                                    int const ISurf,             // Surface number of the complex fenestration
                                    int const IState,            // State number of the complex fenestration state
                                    BSDFWindowGeomDescr &Window, // Window Geometry
                                    BSDFGeomDescr &Geom,         // State Geometry
                                    BSDFStateDescr &State        // State Description
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Joe Klems
        //       DATE WRITTEN   <date_written>
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates those optical properties of all the Complex Fenestrations that
        // do not depend on the beam direction (hence, on hour and time step)

        using namespace Vectors;

        int IConst;   // Pointer to construction for this fenestration
        int I(0);     // general purpose index
        int J(0);     // general purpose index
        int JJ(0);    // general purpose index--ray
        int L(0);     // general purpose index--layer
        int M(0);     // general purpose index--ray
        int KBkSurf;  // back surface index
        int JSurf;    // surface number (used for back surface)
        int BaseSurf; // base surface number (used for finding back surface)
        Real64 Sum1;  // general purpose temporary sum
        Real64 Sum2;  // general purpose temporary sum
        Real64 Sum3;  // general purpose temporary sum
        Real64 Hold;  // temp variable

        IConst = state.dataSurface->SurfaceWindow(ISurf).ComplexFen.State(IState).Konst;

        // Calculate the hemispherical-hemispherical transmittance

        Sum1 = 0.0;
        Sum2 = 0.0;
        for (J = 1; J <= Geom.Inc.NBasis; ++J) { // Incident ray loop
            Sum2 += Geom.Inc.Lamda(J);
            for (M = 1; M <= Geom.Trn.NBasis; ++M) { // Outgoing ray loop
                Sum1 += Geom.Inc.Lamda(J) * Geom.Trn.Lamda(M) * state.dataConstruction->Construct(IConst).BSDFInput.SolFrtTrans(M, J);
            } // Outgoing ray loop
        } // Incident ray loop
        if (Sum2 > 0) {
            State.WinDiffTrans = Sum1 / Sum2;
        } else {
            State.WinDiffTrans = 0.0;
            ShowWarningError(state, "BSDF--Inc basis has zero projected solid angle");
        }

        // Calculate the hemispherical-hemispherical transmittance for visible spetrum

        Sum1 = 0.0;
        Sum2 = 0.0;
        for (J = 1; J <= Geom.Inc.NBasis; ++J) { // Incident ray loop
            Sum2 += Geom.Inc.Lamda(J);
            for (M = 1; M <= Geom.Trn.NBasis; ++M) { // Outgoing ray loop
                Sum1 += Geom.Inc.Lamda(J) * Geom.Trn.Lamda(M) * state.dataConstruction->Construct(IConst).BSDFInput.VisFrtTrans(M, J);
            } // Outgoing ray loop
        } // Incident ray loop
        if (Sum2 > 0.0) {
            State.WinDiffVisTrans = Sum1 / Sum2;
        } else {
            State.WinDiffVisTrans = 0.0;
            ShowWarningError(state, "BSDF--Inc basis has zero projected solid angle");
        }

        // Set the nominal diffuse transmittance so the surface isn't mistaken as opaque
        // Simon: Commented this out. We are not using TransDiff and it is already set to 0.1 in input routines.
        // Construct( IConst ).TransDiff = SurfaceWindow( ISurf ).ComplexFen.State( IState ).WinDiffTrans;
        // Calculate Window Sky Transmittance (transmitted radiation assumed diffuse)
        // and Sky Absorptance (by layer)
        Sum1 = 0.0;
        Sum2 = 0.0;
        Sum3 = 0.0;
        for (JJ = 1; JJ <= Geom.NSky; ++JJ) {
            for (M = 1; M <= Geom.Trn.NBasis; ++M) {
                J = Geom.SkyIndex(JJ);
                Sum1 +=
                    Geom.SolSkyWt(JJ) * state.dataConstruction->Construct(IConst).BSDFInput.SolFrtTrans(M, J) * Geom.Inc.Lamda(J) * Geom.Trn.Lamda(M);
            }
        }
        for (JJ = 1; JJ <= Geom.NSky; ++JJ) {
            J = Geom.SkyIndex(JJ);
            Sum2 += Geom.SolSkyWt(JJ) * Geom.Inc.Lamda(J);
        }

        if (Sum2 != 0.0) {
            State.WinSkyTrans = Sum1 / Sum2;
        } else {
            State.WinSkyTrans = 0.0;
        }

        State.WinSkyFtAbs.allocate(State.NLayers);
        // Also allocate the beam quantities for this state
        for (L = 1; L <= State.NLayers; ++L) {
            Sum3 = 0.0;
            for (JJ = 1; JJ <= Geom.NSky; ++JJ) {
                J = Geom.SkyIndex(JJ);
                Sum3 += Geom.SolSkyWt(JJ) * Geom.Inc.Lamda(J) * state.dataConstruction->Construct(IConst).BSDFInput.Layer(L).FrtAbs(J, 1);
            }

            if (Sum2 != 0.0) {
                State.WinSkyFtAbs(L) = Sum3 / Sum2;
            } else {
                State.WinSkyFtAbs(L) = 0.0;
            }
        }

        // Calculate Window Sky/Ground Transmittance
        //(applies to ground-reflected sky radiation, transmitted radiation assumed diffuse)
        // This is the same calculation as the sky transmittance, except that the set of incident
        // rays and the ray weights are different
        // Also calculate Window Sky/Ground Absorptance (by layer)
        Sum1 = 0.0;
        Sum2 = 0.0;
        Sum3 = 0.0;

        for (JJ = 1; JJ <= Geom.NGnd; ++JJ) {
            for (M = 1; M <= Geom.Trn.NBasis; ++M) {
                J = Geom.GndIndex(JJ);
                Sum1 += Geom.SolSkyGndWt(JJ) * state.dataConstruction->Construct(IConst).BSDFInput.SolFrtTrans(M, J) * Geom.Inc.Lamda(J) *
                        Geom.Trn.Lamda(M);
            }
        }

        for (JJ = 1; JJ <= Geom.NGnd; ++JJ) {
            J = Geom.GndIndex(JJ);
            Sum2 += Geom.SolSkyGndWt(JJ) * Geom.Inc.Lamda(J);
        }

        if (Sum2 != 0.0) {
            State.WinSkyGndTrans = Sum1 / Sum2;
        } else {
            State.WinSkyGndTrans = 0.0;
        }

        State.WinSkyGndAbs.allocate(State.NLayers);
        for (L = 1; L <= State.NLayers; ++L) {
            Sum3 = 0.0;
            for (JJ = 1; JJ <= Geom.NGnd; ++JJ) {
                J = Geom.GndIndex(JJ);
                Sum3 += Geom.SolSkyGndWt(JJ) * Geom.Inc.Lamda(J) * state.dataConstruction->Construct(IConst).BSDFInput.Layer(L).FrtAbs(J, 1);
            }

            if (Sum2 != 0.0) {
                State.WinSkyGndAbs(L) = Sum3 / Sum2;
            } else {
                State.WinSkyGndAbs(L) = 0.0;
            }
        }

        // Calculate Window Back Hemispherical Reflectance and Layer Back Hemispherical Absorptance
        Sum1 = 0.0;
        Sum2 = 0.0;
        Sum3 = 0.0;
        // Note this again assumes the equivalence Inc basis = transmission basis for back incidence and
        // Trn basis = incident basis for back incidence
        for (J = 1; J <= Geom.Trn.NBasis; ++J) {
            for (M = 1; M <= Geom.Inc.NBasis; ++M) {
                Sum1 += state.dataConstruction->Construct(IConst).BSDFInput.SolBkRefl(M, J) * Geom.Trn.Lamda(J) * Geom.Inc.Lamda(M);
            }
        }
        for (J = 1; J <= Geom.Trn.NBasis; ++J) {
            Sum2 += Geom.Trn.Lamda(J);
        }

        if (Sum2 != 0.0) {
            State.WinBkHemRefl = Sum1 / Sum2;
        } else {
            State.WinBkHemRefl = 0.0;
        }

        state.dataConstruction->Construct(IConst).ReflectSolDiffBack = State.WinBkHemRefl;

        State.WinBkHemAbs.allocate(State.NLayers);
        for (L = 1; L <= State.NLayers; ++L) {
            for (J = 1; J <= Geom.Trn.NBasis; ++J) {
                Sum3 += Geom.Trn.Lamda(J) * state.dataConstruction->Construct(IConst).BSDFInput.Layer(L).BkAbs(J, 1);
            }

            if (Sum2 != 0.0) {
                State.WinBkHemAbs(L) = Sum3 / Sum2;
            } else {
                State.WinBkHemAbs(L) = 0.0;
            }

            // Put this into the construction for use in non-detailed optical calculations
            state.dataConstruction->Construct(IConst).AbsDiffBack(L) = State.WinBkHemAbs(L);
        }

        // Calculate Window Layer Front Hemispherical Absorptance
        Sum1 = 0.0;
        Sum2 = 0.0;
        for (J = 1; J <= Geom.Inc.NBasis; ++J) {
            Sum2 += Geom.Inc.Lamda(J);
        }
        State.WinFtHemAbs.allocate(State.NLayers);
        for (L = 1; L <= State.NLayers; ++L) {
            Sum1 = 0.0;
            for (J = 1; J <= Geom.Inc.NBasis; ++J) {
                Sum1 += Geom.Inc.Lamda(J) * state.dataConstruction->Construct(IConst).BSDFInput.Layer(L).FrtAbs(J, 1);
            }

            if (Sum2 != 0.0) {
                State.WinFtHemAbs(L) = Sum1 / Sum2;
            } else {
                State.WinFtHemAbs(L) = 0.0;
            }

            // Put this into the construction for use in non-detailed optical calculations
            state.dataConstruction->Construct(IConst).AbsDiff(L) = State.WinFtHemAbs(L);
        }

        // Calculate Window Back Hemispherical Visible Reflectance
        Sum1 = 0.0;
        Sum2 = 0.0;
        // Note this again assumes the equivalence Inc basis = transmission basis for back incidence and
        // Trn basis = incident basis for back incidence
        for (J = 1; J <= Geom.Trn.NBasis; ++J) {
            for (M = 1; M <= Geom.Inc.NBasis; ++M) {
                Sum1 += state.dataConstruction->Construct(IConst).BSDFInput.VisBkRefl(M, J) * Geom.Trn.Lamda(J) * Geom.Inc.Lamda(M);
            }
        }
        for (J = 1; J <= Geom.Trn.NBasis; ++J) {
            Sum2 += Geom.Trn.Lamda(J);
        }

        if (Sum2 != 0.0) {
            State.WinBkHemVisRefl = Sum1 / Sum2;
        } else {
            State.WinBkHemVisRefl = 0.0;
        }

        state.dataConstruction->Construct(IConst).ReflectVisDiffBack = State.WinBkHemVisRefl;

        //     *     *     *     *
        // Note potential problem if one relaxes the assumption that Inc and Trn basis have same structure:
        //  The following calculations are made for the set of ray numbers defined in the Trn basis that
        //   were determined to connect the center of the window to a particular back surface.
        //   Here it is assumed that one can reverse these rays and get an equivalent set in the Trn
        //   basis for back-incidence quantities: back transmittance and back layer absorptance
        //   This assumption may fail if the Inc and Trn bases are allowed to have different structure.
        //   Note also that in this case one would need to rethink the relationship of the basis
        //   definitions to back-incidence quantities:  possibly this would
        //   also require that the basis for back incident quantities be
        //   different from the Trn basis, and similarly the basis for backward outgoing rays
        //   be different from the Inc basis.

        //     *     *     *     *
        //  Note that we are assuming that for back incidence the layer numberings are the same
        //  as for front incidence, i.e., from outside to inside when incidence is from inside
        //     *     *     *     *
        // For back surfaces that are complex fenestrations, calculate the directional-hemispherical back
        //  reflectance and the directional back absorptance by layer for this fenestration receiving
        //  radiation via the back surface
        //  Make this calculation only for cases where the back surface is a Complex Fenestration
        // First allocate the back surface section of the state properties
        if (!allocated(State.BkSurf)) {
            State.BkSurf.allocate(Window.NBkSurf);
        }
        for (KBkSurf = 1; KBkSurf <= Window.NBkSurf; ++KBkSurf) {  // back surface loop
            BaseSurf = state.dataSurface->Surface(ISurf).BaseSurf; // ShadowComb is organized by base surface
            JSurf = state.dataShadowComb->ShadowComb(BaseSurf).BackSurf(KBkSurf);
            if (state.dataSurface->SurfWinWindowModelType(JSurf) != WindowModel::BSDF) {
                continue;
            }

            //  Directional-hemispherical back reflectance
            Sum1 = 0.0;
            Sum2 = 0.0;
            for (J = 1; J <= Geom.NSurfInt(KBkSurf); ++J) { // Inc Ray loop
                Sum2 += Geom.Trn.Lamda(Geom.SurfInt(J, KBkSurf));
                for (M = 1; M <= Geom.Inc.NBasis; ++M) { // Outgoing Ray loop
                    Sum1 += Geom.Trn.Lamda(Geom.SurfInt(J, KBkSurf)) * Geom.Inc.Lamda(M) *
                            state.dataConstruction->Construct(IConst).BSDFInput.SolBkRefl(Geom.SurfInt(J, KBkSurf), M);
                } // Outgoing Ray loop
            } // Inc Ray loop
            if (Sum2 > 0.0) {
                Hold = Sum1 / Sum2;
                for (I = 1; I <= 24; ++I) {
                    for (J = 1; J <= state.dataGlobal->TimeStepsInHour; ++J) {
                        State.BkSurf(KBkSurf).WinDHBkRefl(I, J) = Hold;
                    }
                }
            } else {
                for (I = 1; I <= 24; ++I) {
                    for (J = 1; J <= state.dataGlobal->TimeStepsInHour; ++J) {
                        State.BkSurf(KBkSurf).WinDHBkRefl(I, J) = 0.0;
                    }
                }
            }

            //  Directional layer  back absorption
            for (L = 1; L <= State.NLayers; ++L) { // layer loop
                Sum1 = 0.0;
                Sum2 = 0.0;
                for (J = 1; J <= Geom.NSurfInt(KBkSurf); ++J) { // Inc Ray loop
                    Sum2 += Geom.Trn.Lamda(Geom.SurfInt(J, KBkSurf));
                    Sum1 += Geom.Trn.Lamda(Geom.SurfInt(J, KBkSurf)) *
                            state.dataConstruction->Construct(IConst).BSDFInput.Layer(L).BkAbs(Geom.SurfInt(J, KBkSurf), 1);
                } // Inc Ray loop
                if (Sum2 > 0.0) {
                    Hold = Sum1 / Sum2;
                    for (I = 1; I <= 24; ++I) {
                        for (J = 1; J <= state.dataGlobal->TimeStepsInHour; ++J) {
                            State.BkSurf(KBkSurf).WinDirBkAbs(I, J, L) = Hold;
                        }
                    }
                } else {
                    for (I = 1; I <= 24; ++I) {
                        for (J = 1; J <= state.dataGlobal->TimeStepsInHour; ++J) {
                            State.BkSurf(KBkSurf).WinDirBkAbs(I, J, L) = 0.0;
                        }
                    }
                }

            } // layer loop
        } // back surface loop

        // ********************************************************************************
        // Allocation and calculation of integrated values for front of window surface
        // ********************************************************************************

        // Sum of front absorptances for each incident direction (integration of absorptances)
        if (!allocated(State.IntegratedFtAbs)) {
            State.IntegratedFtAbs.allocate(Geom.Inc.NBasis);
        }
        for (J = 1; J <= Geom.Inc.NBasis; ++J) {
            Sum1 = 0.0;
            for (L = 1; L <= State.NLayers; ++L) { // layer loop
                Sum1 += state.dataConstruction->Construct(IConst).BSDFInput.Layer(L).FrtAbs(J, 1);
            }
            State.IntegratedFtAbs(J) = Sum1;
        }

        // Integrating front transmittance
        if (!allocated(State.IntegratedFtTrans)) {
            State.IntegratedFtTrans.allocate(Geom.Inc.NBasis);
        }
        for (J = 1; J <= Geom.Inc.NBasis; ++J) { // Incident ray loop
            Sum1 = 0.0;
            for (M = 1; M <= Geom.Trn.NBasis; ++M) { // Outgoing ray loop
                Sum1 += Geom.Trn.Lamda(J) * state.dataConstruction->Construct(IConst).BSDFInput.SolFrtTrans(J, M);
            } // Outgoing ray loop
            State.IntegratedFtTrans(J) = Sum1;
        } // Incident ray loop

        if (!allocated(State.IntegratedFtRefl)) {
            State.IntegratedFtRefl.allocate(Geom.Inc.NBasis);
        }
        // Integrating front reflectance
        for (J = 1; J <= Geom.Inc.NBasis; ++J) { // Incoming ray loop
            State.IntegratedFtRefl(J) = 1 - State.IntegratedFtTrans(J) - State.IntegratedFtAbs(J);
        } // Incoming ray loop

        // ********************************************************************************
        // Allocation and calculation of integrated values for back of window surface
        // ********************************************************************************

        // Sum of back absorptances for each incident direction (integration of absorptances)
        if (!allocated(State.IntegratedBkAbs)) {
            State.IntegratedBkAbs.allocate(Geom.Trn.NBasis);
        }
        for (J = 1; J <= Geom.Trn.NBasis; ++J) {
            Sum1 = 0.0;
            for (L = 1; L <= State.NLayers; ++L) { // layer loop
                Sum1 += state.dataConstruction->Construct(IConst).BSDFInput.Layer(L).BkAbs(J, 1);
            }
            State.IntegratedBkAbs(J) = Sum1;
        }

        // Integrating back reflectance
        if (!allocated(State.IntegratedBkRefl)) {
            State.IntegratedBkRefl.allocate(Geom.Trn.NBasis);
        }
        for (J = 1; J <= Geom.Trn.NBasis; ++J) { // Outgoing ray loop
            Sum1 = 0.0;
            for (M = 1; M <= Geom.Inc.NBasis; ++M) { // Incident ray loop
                Sum1 += Geom.Inc.Lamda(J) * state.dataConstruction->Construct(IConst).BSDFInput.SolBkRefl(J, M);
            } // Incident ray loop
            State.IntegratedBkRefl(J) = Sum1;
        } // Outgoing ray loop

        if (!allocated(State.IntegratedBkTrans)) {
            State.IntegratedBkTrans.allocate(Geom.Trn.NBasis);
        }
        // Integrating back transmittance
        for (J = 1; J <= Geom.Trn.NBasis; ++J) { // Outgoing ray loop
            State.IntegratedBkTrans(J) = 1 - State.IntegratedBkRefl(J) - State.IntegratedBkAbs(J);
        } // Outgoing ray loop
    }

    Real64 SkyWeight([[maybe_unused]] Vector const &DirVec) // Direction of the element to be weighted
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Joe Klems
        //       DATE WRITTEN   June 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Search a one-dimensional array for a given value, returning the index of the element equal to the value, if
        //   found, or zero

        using namespace Vectors;

        // Return value
        Real64 Wt; // Weight

        Wt = 1.0;

        // To do:  figure out how to weight sky elements to reproduce the current E+ assumptions
        //  Possibly one will need to calculated average DH transmittance for isotropic sky and
        //  horizon separately and then later average according to sky conditions.  Then a completely
        //  different scheme for daylight.  For now: rays that reach sky equally weighted in calculating
        //  transmittance, rays passing through surfaces with scheduled transmittance are neglected.

        return Wt;
    }

    Real64 SkyGndWeight([[maybe_unused]] Vector const &PosVec) // x,y,z(=0) of ground intersection pt
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Joe Klems
        //       DATE WRITTEN   June 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Search a one-dimensional array for a given value, returning the index of the element equal to the value, if
        //   found, or zero

        using namespace Vectors;

        // Return value
        Real64 Wt; // Weight

        Wt = 1.0;

        //  At present, equally weights all ground rays for calculation of the complex window transmittance for
        //  sky radiation reflected from ground.  This does not take into account shading of the ground.
        //  The correct procedure would be to generate a set of rays to the sky and see which do not intersect
        //  surfaces, as is done in the reflection manager.  However, this would increase computational load.
        //  Given that equal weighting, by averaging the transmittance only over rays that come from the ground,
        //  already produces a more accurate ground transmittance than the existing method, it is at least questionable
        //  whether the more detailed procedure would produce enough improvement in accuracy to make up for
        //  the additional calculation time.  Therefore a more detailed treatment is deferred until there is some
        //  experience with the new method to determine whether further detail is warranted.

        return Wt;
    }

    BSDFDaylghtPosition DaylghtAltAndAzimuth(Vector const &UnitVect) // vector which needs to be converted
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   April 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Transform unit vector (given in world coordinates) into altitude and azimuth.  Azimuth is measured from positive x-axe.
        // Altitude range is from -pi/2 to pi/2. Vector laying in horizontal plane will have altitude equal to zero and vector
        // pointing upward will have altitude equal to pi/2. Range for azimuth is calculated from -pi to +pi.

        using namespace DataBSDFWindow;
        using namespace Vectors;
        // Return value
        BSDFDaylghtPosition DayPos; // altitude and azimuth in world coordinates

        if (UnitVect.x != 0.0) {
            if (UnitVect.x >= 0.0) {
                DayPos.Azimuth = std::atan(UnitVect.y / UnitVect.x);
            } else {
                if (UnitVect.y >= 0.0) {
                    DayPos.Azimuth = Constant::Pi + std::atan(UnitVect.y / UnitVect.x);
                } else {
                    DayPos.Azimuth = -Constant::Pi + std::atan(UnitVect.y / UnitVect.x);
                }
            }
        } else {
            if (UnitVect.y >= 0.0) {
                DayPos.Azimuth = Constant::PiOvr2;
            } else {
                DayPos.Azimuth = -Constant::PiOvr2;
            }
        }

        DayPos.Altitude = std::asin(UnitVect.z);

        return DayPos;
    }

    Vector WorldVectFromW6([[maybe_unused]] EnergyPlusData &state,
                           Real64 const Theta,                  // Polar angle in W6 Coords
                           Real64 const Phi,                    // Azimuthal angle in W6 Coords
                           const RayIdentificationType RadType, // Type of radiation: Front_Incident, etc.
                           Real64 const Gamma,                  // Surface tilt angle, radians, world coordinate system
                           Real64 const Alpha                   // Surface azimuth, radians, world coordinate system
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Joe Klems
        //       DATE WRITTEN   Aug 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Transform angular coordinates in the WINDOW6 coordinate system for
        // a given surface into a unit vector in the world coordinate system,
        // pointing to the radiation source (for incident radiation) or in
        // the direction of propagation (for outgoing radiation)

        using namespace Vectors;

        // Return value
        Vector UnitVect; // unit vector direction in world CS

        // Error tolerance is used to make small numbers equal to zero.  Due to precision of pi constant used in E+, performing
        // trigonometric operations on those constant will not cause absolutely accurate results
        Real64 constexpr ErrorTolerance(1.e-10);

        UnitVect = Vector(0.0, 0.0, 0.0);

        Real64 const sin_Phi = std::sin(Phi);
        Real64 const cos_Phi = std::cos(Phi);

        Real64 const sin_Gamma = std::sin(Gamma);
        Real64 const cos_Gamma = std::cos(Gamma);

        Real64 const sin_Alpha = std::sin(Alpha);
        Real64 const cos_Alpha = std::cos(Alpha);

        Real64 const sin_Theta = std::sin(Theta);
        Real64 const cos_Theta = std::cos(Theta);

        switch (RadType) {
        case RayIdentificationType::Front_Incident: { // W6 vector will point in direction of propagation, must reverse to get world vector
            //  after the W6 vector has been rotated into the world CS
            UnitVect.x = sin_Theta * sin_Phi * cos_Gamma * sin_Alpha - sin_Theta * cos_Phi * cos_Alpha + cos_Theta * sin_Gamma * sin_Alpha;
            UnitVect.y = sin_Theta * cos_Phi * sin_Alpha + sin_Theta * sin_Phi * cos_Gamma * cos_Alpha + cos_Theta * sin_Gamma * cos_Alpha;
            UnitVect.z = -(sin_Theta * sin_Phi * sin_Gamma - cos_Theta * cos_Gamma);
            break;
        }
        case RayIdentificationType::Front_Transmitted: {
            UnitVect.x = sin_Theta * cos_Phi * cos_Alpha - sin_Theta * sin_Phi * cos_Gamma * sin_Alpha - cos_Theta * sin_Gamma * sin_Alpha;
            UnitVect.y = -(sin_Theta * cos_Phi * sin_Alpha + sin_Theta * sin_Phi * cos_Gamma * cos_Alpha + cos_Theta * sin_Gamma * cos_Alpha);
            UnitVect.z = sin_Theta * sin_Phi * sin_Gamma - cos_Theta * cos_Gamma;
            break;
        }
        case RayIdentificationType::Front_Reflected: {
            UnitVect.x = sin_Theta * cos_Phi * cos_Alpha - sin_Theta * sin_Phi * cos_Gamma * sin_Alpha + cos_Theta * sin_Gamma * sin_Alpha;
            UnitVect.y = cos_Theta * sin_Gamma * cos_Alpha - sin_Theta * cos_Phi * sin_Alpha - sin_Theta * sin_Phi * cos_Gamma * cos_Alpha;
            UnitVect.z = sin_Theta * sin_Phi * sin_Gamma + cos_Theta * cos_Gamma;
            break;
        }
        case RayIdentificationType::Back_Incident: {
            UnitVect.x = sin_Theta * sin_Phi * cos_Gamma * sin_Alpha - sin_Theta * cos_Phi * cos_Alpha - cos_Theta * sin_Gamma * sin_Alpha;
            UnitVect.y = sin_Theta * cos_Phi * sin_Alpha + sin_Theta * sin_Phi * cos_Gamma * cos_Alpha - cos_Theta * sin_Gamma * cos_Alpha;
            UnitVect.z = -cos_Theta * cos_Gamma - sin_Theta * sin_Phi * sin_Gamma;
            break;
        }
        case RayIdentificationType::Back_Transmitted: { // This is same as front reflected
            UnitVect.x = sin_Theta * cos_Phi * cos_Alpha - sin_Theta * sin_Phi * cos_Gamma * sin_Alpha + cos_Theta * sin_Gamma * sin_Alpha;
            UnitVect.y = cos_Theta * sin_Gamma * cos_Alpha - sin_Theta * cos_Phi * sin_Alpha - sin_Theta * sin_Phi * cos_Gamma * cos_Alpha;
            UnitVect.z = sin_Theta * sin_Phi * sin_Gamma + cos_Theta * cos_Gamma;
            break;
        }
        case RayIdentificationType::Back_Reflected: { // This is same as front transmitted
            UnitVect.x = sin_Theta * cos_Phi * cos_Alpha - sin_Theta * sin_Phi * cos_Gamma * cos_Alpha - cos_Theta * sin_Gamma * sin_Alpha;
            UnitVect.y = -(sin_Theta * cos_Phi * sin_Alpha + sin_Theta * sin_Phi * cos_Gamma * cos_Alpha + cos_Theta * sin_Gamma * cos_Alpha);
            UnitVect.z = sin_Theta * sin_Phi * sin_Gamma - cos_Theta * cos_Gamma;
            break;
        }
        default:
            break;
        }

        // Remove small numbers from evaluation (due to limited decimal points for pi)
        if (std::abs(UnitVect.x) <= ErrorTolerance) {
            UnitVect.x = 0.0;
        }
        if (std::abs(UnitVect.y) <= ErrorTolerance) {
            UnitVect.y = 0.0;
        }
        if (std::abs(UnitVect.z) <= ErrorTolerance) {
            UnitVect.z = 0.0;
        }

        return UnitVect;
    }

    int FindInBasis(EnergyPlusData &state,
                    Vector const &RayToFind,             // Ray vector direction in world CS
                    const RayIdentificationType RadType, // Type of radiation: Front_Incident, etc.
                    int const ISurf,                     // Window Surface number
                    [[maybe_unused]] int const IState,   // Complex Fenestration state number
                    BasisStruct const &Basis,            // Complex Fenestration basis root
                    Real64 &Theta,                       // Theta value for ray
                    Real64 &Phi                          // Phi value for ray
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Joe Klems
        //       DATE WRITTEN August 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        using namespace Vectors;

        // Return value
        int RayIndex; // Index of ray in basis, zero if ray not in hemisphere

        // INTEGER, INTENT(IN)      ::  IWind  !window index in window list

        int ITheta;   // Table index of Theta
        int IPhi;     // Table index of Phi, given ITheta
        int IThDn;    // Theta lower table index
        int IThUp;    // Theta upper table index
        int IPhDn;    // Phi lower table index
        int IPhUp;    // Phi upper table index
        Real64 Gamma; // Gamma (tilt) angle of window
        Real64 Alpha; // Alpha (azimuth) angle of window
        Real64 DotProd;

        Theta = 0.0;
        Phi = 0.0;

        // Check if surface and vector are pointing in different directions
        DotProd = dot(RayToFind, state.dataSurface->Surface(ISurf).NewellSurfaceNormalVector);
        if (DotProd <= 0.0) {
            RayIndex = 0;
            return RayIndex;
        }

        // get window tilt and azimuth
        Gamma = Constant::DegToRad * state.dataSurface->Surface(ISurf).Tilt;
        Alpha = Constant::DegToRad * state.dataSurface->Surface(ISurf).Azimuth;
        // get the corresponding local Theta, Phi for ray
        W6CoordsFromWorldVect(state, RayToFind, RadType, Gamma, Alpha, Theta, Phi);

        if (Theta >= 0.5 * Constant::Pi) { // Ray was in not in correct hemisphere
            RayIndex = 0;
            return RayIndex;
        }
        if (Basis.BasisSymmetryType == DataBSDFWindow::BasisSymmetry::None) {
            // Search the basis thetas
            if (Theta <= 0.0) {
                // Special case, Theta = 0.; this is always the first basis element
                RayIndex = 1;
                return RayIndex;
            }
            // So here Theta > 0
            // Note the table searches always go to the limit point, which is not itself a basis element
            IThUp = SearchAscTable(Theta, Basis.NThetas + 1, Basis.Thetas);
            IThDn = IThUp - 1;
            // Determine which of the theta basis points is closer to the Theta value
            if (Theta <= Basis.Grid(Basis.BasisIndex(1, IThDn)).UpprTheta) {
                // Note this will take care of both the special cases IThUp=2 and IThUp=NThetas +1
                ITheta = IThDn;
            } else {
                ITheta = IThUp;
            }
            // Now determine the Phi index
            if (Basis.NPhis(ITheta) == 1) {
                // Note that for W6 basis this can only happen for the first basis element
                // If later bases are introduced this logic may have to be redesigned
                RayIndex = Basis.BasisIndex(1, ITheta);
                return RayIndex;
            }
            IPhUp = SearchAscTable(Phi, Basis.NPhis(ITheta) + 1, Basis.Phis(ITheta, _));
            IPhDn = IPhUp - 1;
            if (Phi <= Basis.Grid(Basis.BasisIndex(IPhDn, ITheta)).UpprPhi) {
                IPhi = IPhDn;
            } else {
                if (IPhUp == Basis.NPhis(ITheta) + 1) {
                    // Phi is above upper limit for highest Phi basis element, meaning it is closer to 2Pi,
                    // i.e., the first element
                    IPhi = 1;
                } else {
                    IPhi = IPhUp;
                }
            }
            RayIndex = Basis.BasisIndex(IPhi, ITheta);
            return RayIndex;
        } else if (Basis.BasisSymmetryType == DataBSDFWindow::BasisSymmetry::Axisymmetric) {
            // Search the basis thetas
            if (Theta <= 0.0) {
                // Special case, Theta = 0.; this is always the first basis element
                RayIndex = 1;
                return RayIndex;
            }
            // So here Theta > 0
            // Note the table searches always go to the limit point, which is not itself a basis element
            IThUp = SearchAscTable(Theta, Basis.NThetas + 1, Basis.Thetas);
            IThDn = IThUp - 1;
            // Determine which of the theta basis points is closer to the Theta value
            if (Theta <= Basis.Grid(Basis.BasisIndex(1, IThDn)).UpprTheta) {
                // Note this will take care of both the special cases IThUp=2 and IThUp=NThetas +1
                ITheta = IThDn;
            } else {
                ITheta = IThUp;
            }
            RayIndex = Basis.BasisIndex(1, ITheta);
            return RayIndex;
        }
        // No other type is implemented
        RayIndex = 0;

        return RayIndex;
    }

    void W6CoordsFromWorldVect([[maybe_unused]] EnergyPlusData &state,
                               Vector const &RayVect,               // Ray vector direction in world CS
                               const RayIdentificationType RadType, // Type of radiation: Front_Incident, etc.
                               Real64 const Gamma,                  // Surface tilt angle, world coordinate system
                               Real64 const Alpha,                  // Surface azimuth, world coordinate system
                               Real64 &Theta,                       // Polar angle in W6 Coords
                               Real64 &Phi                          // Azimuthal angle in W6 Coords
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Joe Klems
        //       DATE WRITTEN   August 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Invert the transformation from W6 to world coordinates to
        // calculate the theta, phi corresponding to a given ray direction
        // in the world coordinate system, for a window with a
        // given rotation and tilt (Gamma and Alpha)
        //  (needed for locating the sun direction in the local coordinate system)

        using namespace Vectors;

        Real64 Cost(0.0); // Temp for cos theta
        Real64 Sint;      // Temp for sin theta
        Real64 Psi;       // Temp for phi before rotation adjustment
        Real64 RdotX;     // Temp variable for manipulating .dot. produt
        Real64 RdotY;     // Temp variable for manipulating .dot. produt
        Real64 RdotZ;     // Temp variable for manipulating .dot. produt

        // Object Data
        Vector W6x; // W6 x coordinate unit vector
        Vector W6y; // W6 y coordinate unit vector
        Vector W6z; // W6 z coordinate unit vector

        // define the local W6 coordinate vectors
        W6x.x = std::cos(Alpha);
        W6x.y = -std::sin(Alpha);
        W6x.z = 0.0;
        W6y.x = -std::cos(Gamma) * std::sin(Alpha);
        W6y.y = -std::cos(Gamma) * std::cos(Alpha);
        W6y.z = std::sin(Gamma);
        W6z.x = -std::sin(Gamma) * std::sin(Alpha);
        W6z.y = -std::sin(Gamma) * std::cos(Alpha);
        W6z.z = -std::cos(Gamma);

        switch (RadType) {
        case RayIdentificationType::Front_Incident: {
            RdotZ = dot(W6z, RayVect);
            Cost = -RdotZ;
            Sint = std::sqrt(1.0 - pow_2(Cost));
            Theta = std::acos(Cost);
            RdotY = dot(W6y, RayVect);
            RdotX = dot(W6x, RayVect);
            Psi = std::atan2(-RdotY / Sint, -RdotX / Sint);
            if (Psi < 0.0) {
                Phi = 2.0 * Constant::Pi + Psi;
            } else {
                Phi = Psi;
            }
            break;
        }
        case RayIdentificationType::Front_Transmitted: {
            Cost = dot(W6z, RayVect);
            Sint = std::sqrt(1.0 - pow_2(Cost));
            Theta = std::acos(Cost);
            RdotY = dot(W6y, RayVect);
            RdotX = dot(W6x, RayVect);
            Psi = std::atan2(RdotY / Sint, RdotX / Sint);
            if (Psi < 0.0) {
                Phi = 2.0 * Constant::Pi + Psi;
            } else {
                Phi = Psi;
            }
            break;
        }
        case RayIdentificationType::Front_Reflected: {
            RdotZ = dot(W6z, RayVect);
            Cost = -RdotZ;
            Sint = std::sqrt(1.0 - pow_2(Cost));
            Theta = std::acos(Cost);
            RdotY = dot(W6y, RayVect);
            RdotX = dot(W6x, RayVect);
            Psi = std::atan2(RdotY / Sint, RdotX / Sint);
            if (Psi < 0.0) {
                Phi = 2.0 * Constant::Pi + Psi;
            } else {
                Phi = Psi;
            }
            break;
        }
        case RayIdentificationType::Back_Incident: {
            Cost = dot(W6z, RayVect);
            Sint = std::sqrt(1.0 - pow_2(Cost));
            Theta = std::acos(Cost);
            RdotY = dot(W6y, RayVect);
            RdotX = dot(W6x, RayVect);
            Psi = std::atan2(-RdotY / Sint, -RdotX / Sint);
            if (Psi < 0.0) {
                Phi = 2 * Constant::Pi + Psi;
            } else {
                Phi = Psi;
            }
            break;
        }
        case RayIdentificationType::Back_Transmitted: { // This is same as front reflected
            RdotZ = dot(W6z, RayVect);
            Cost = -RdotZ;
            Sint = std::sqrt(1.0 - pow_2(Cost));
            Theta = std::acos(Cost);
            RdotY = dot(W6y, RayVect);
            RdotX = dot(W6x, RayVect);
            Psi = std::atan2(RdotY / Sint, RdotX / Sint);
            if (Psi < 0.0) {
                Phi = 2.0 * Constant::Pi + Psi;
            } else {
                Phi = Psi;
            }
            break;
        }
        case RayIdentificationType::Back_Reflected: { // This is same as front transmitted
            Cost = dot(W6z, RayVect);
            Sint = std::sqrt(1.0 - pow_2(Cost));
            Theta = std::acos(Cost);
            RdotY = dot(W6y, RayVect);
            RdotX = dot(W6x, RayVect);
            Psi = std::atan2(RdotY / Sint, RdotX / Sint);
            if (Psi < 0.0) {
                Phi = 2.0 * Constant::Pi + Psi;
            } else {
                Phi = Psi;
            }
            break;
        }
        default:
            assert(false);
            break;
        }
        if (std::abs(Cost) < Constant::rTinyValue) {
            Cost = 0.0;
        }
        if (Cost < 0.0) {
            Theta = Constant::Pi - Theta; // This signals ray out of hemisphere
        }
    }

    void CalcComplexWindowThermal(EnergyPlusData &state,
                                  int const SurfNum,          // Surface number
                                  int &ConstrNum,             // Construction number
                                  Real64 const HextConvCoeff, // Outside air film conductance coefficient
                                  Real64 &SurfInsideTemp,     // Inside window surface temperature
                                  Real64 &SurfOutsideTemp,    // Outside surface temperature (C)
                                  Real64 &SurfOutsideEmiss,
                                  DataBSDFWindow::Condition const CalcCondition // Calucation condition (summer, winter or no condition)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   October 2009
        //       MODIFIED       Simon Vidanovic
        //       RE-ENGINEERED  September 2011

        // PURPOSE OF THIS SUBROUTINE:
        // wrapper between E+ and TARCOG

        // METHODOLOGY EMPLOYED:
        // draft out an attempt for proof-of-concept, to reuse native TARCOG implementation
        // based off of 1-26-2009 version of WinCOG/TARCOG solution from Carli, Inc.

        using namespace DataBSDFWindow;
        using Psychrometrics::PsyCpAirFnW;
        using Psychrometrics::PsyTdpFnWPb;
        using TARCOGGassesParams::maxgas;
        using TARCOGMain::TARCOG90;
        using TARCOGParams::maxlay;
        using TARCOGParams::maxlay1;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // (temperature of innermost face) [C]
        // INTEGER, INTENT(IN)        :: CurrentThermalModelNumber

        // SUBROUTINE PARAMETER DEFINITIONS:
        // INTEGER,  PARAMETER :: maxlay = 100 ! maximum number of layers (including laminates)
        // INTEGER,  PARAMETER :: maxgas = 10  ! maximum number of individual gasses
        // INTEGER, PARAMETER :: maxlay1  = maxlay+1     ! maximum number of 'gaps', including in and out (maxlay+1)
        // REAL(r64), PARAMETER :: StefanBoltzmannConst = 5.6697d-8   ! Stefan-Boltzmann constant in W/(m2*K4)

        // TARCOG Inputs:
        int nlayer(0);     // Number of glazing layers
        int iwd(0);        // Wind direction:  0 - windward, 1 - leeward
        Real64 tout(0.0);  // Outdoor temperature [K]
        Real64 tind(0.0);  // Indoor temperature [K]
        Real64 trmin(0.0); // Indoor mean radiant temperature [K]
        Real64 wso(0.0);   // Outdoor wind speed [m/s]
        Real64 wsi(0.0);   // Inside forced air speed [m/s]
        Real64 dir(0.0);   // Direct solar radiation [W/m^2]
        int isky(0);       // Flag for sky temperature (Tsky) and sky emittance (esky)
        //                      0 - both tsky and esky are specified
        //                      1 - tsky specified, esky = 1
        //                      2 - Swinbank model for effective sky emittance
        Real64 tsky(0.0);             // Night sky temperature [K]
        Real64 esky(0.0);             // Effective night sky emittance
        Real64 fclr(0.0);             // Fraction of sky that is clear
        Real64 VacuumPressure;        // maximal pressure for gas to be considered as vacuum [Pa]
        Real64 VacuumMaxGapThickness; // maximal gap thickness for which vacuum calculation will work without issuing
        // warning message

        auto &gap = state.dataWindowComplexManager->gap;
        auto &thick = state.dataWindowComplexManager->thick;
        auto &scon = state.dataWindowComplexManager->scon;
        auto &tir = state.dataWindowComplexManager->tir;
        auto &emis = state.dataWindowComplexManager->emis;
        auto &SupportPlr = state.dataWindowComplexManager->SupportPlr;
        auto &PillarSpacing = state.dataWindowComplexManager->PillarSpacing;
        auto &PillarRadius = state.dataWindowComplexManager->PillarRadius;
        auto &asol = state.dataWindowComplexManager->asol;
        auto &presure = state.dataWindowComplexManager->presure;
        auto &GapDefMax = state.dataWindowComplexManager->GapDefMax;
        auto &YoungsMod = state.dataWindowComplexManager->YoungsMod;
        auto &PoissonsRat = state.dataWindowComplexManager->PoissonsRat;
        auto &LayerDef = state.dataWindowComplexManager->LayerDef;
        auto &iprop = state.dataWindowComplexManager->iprop;
        auto &frct = state.dataWindowComplexManager->frct;
        auto &gcon = state.dataWindowComplexManager->gcon;
        auto &gvis = state.dataWindowComplexManager->gvis;
        auto &gcp = state.dataWindowComplexManager->gcp;
        auto &wght = state.dataWindowComplexManager->wght;
        auto &gama = state.dataWindowComplexManager->gama;
        auto &nmix = state.dataWindowComplexManager->nmix;
        auto &ibc = state.dataWindowComplexManager->ibc;
        auto &Atop = state.dataWindowComplexManager->Atop;
        auto &Abot = state.dataWindowComplexManager->Abot;
        auto &Al = state.dataWindowComplexManager->Al;
        auto &Ar = state.dataWindowComplexManager->Ar;
        auto &Ah = state.dataWindowComplexManager->Ah;
        auto &SlatThick = state.dataWindowComplexManager->SlatThick;
        auto &SlatWidth = state.dataWindowComplexManager->SlatWidth;
        auto &SlatAngle = state.dataWindowComplexManager->SlatAngle;
        auto &SlatCond = state.dataWindowComplexManager->SlatCond;
        auto &SlatSpacing = state.dataWindowComplexManager->SlatSpacing;
        auto &SlatCurve = state.dataWindowComplexManager->SlatCurve;
        auto &vvent = state.dataWindowComplexManager->vvent;
        auto &tvent = state.dataWindowComplexManager->tvent;
        auto &LayerType = state.dataWindowComplexManager->LayerType;
        auto &nslice = state.dataWindowComplexManager->nslice;
        auto &LaminateA = state.dataWindowComplexManager->LaminateA;
        auto &LaminateB = state.dataWindowComplexManager->LaminateB;
        auto &sumsol = state.dataWindowComplexManager->sumsol;
        auto &theta = state.dataWindowComplexManager->theta;
        auto &q = state.dataWindowComplexManager->q;
        auto &qv = state.dataWindowComplexManager->qv;
        auto &hcgap = state.dataWindowComplexManager->hcgap;
        auto &hrgap = state.dataWindowComplexManager->hrgap;
        auto &hg = state.dataWindowComplexManager->hg;
        auto &hr = state.dataWindowComplexManager->hr;
        auto &hs = state.dataWindowComplexManager->hs;
        auto &Ra = state.dataWindowComplexManager->Ra;
        auto &Nu = state.dataWindowComplexManager->Nu;
        auto &Keff = state.dataWindowComplexManager->Keff;
        auto &ShadeGapKeffConv = state.dataWindowComplexManager->ShadeGapKeffConv;

        Real64 totsol(0.0);  // Total solar transmittance of the IGU
        Real64 tilt(0.0);    // Window tilt [degrees]
        Real64 height(0.0);  // IGU cavity height [m]
        Real64 heightt(0.0); // Total window height [m]
        Real64 width(0.0);   // Window width [m]

        // Deflection
        // Tarcog requires deflection as input parameters.  Deflection is NOT used in EnergyPlus simulations
        TARCOGParams::DeflectionCalculation CalcDeflection; // Deflection calculation flag:
        //    0 - no deflection calculations
        //    1 - perform deflection calculation (input is Pressure/Temp)
        //    2 - perform deflection calculation (input is measured deflection)
        Real64 Pa;        // Atmospheric (outside/inside) pressure (used onlu if CalcDeflection = 1)
        Real64 Pini;      // Initial presssure at time of fabrication (used only if CalcDeflection = 1)
        Real64 Tini;      // Initial temperature at time of fabrication (used only if CalcDeflection = 1)
        Real64 hin(0.0);  // Indoor combined film coefficient (if non-zero) [W/m^2.K]
        Real64 hout(0.0); // Outdoor combined film coefficient (if non-zero) [W/m^2.K]
        TARCOGGassesParams::Stdrd standard(TARCOGGassesParams::Stdrd::ISO15099); // Calculation standard switch:
        //                 1 - ISO 15099,
        //                 2 - EN673 / ISO 10292 Declared,
        //                 3 - EN673 / ISO 10292 Design.
        TARCOGParams::TARCOGThermalModel ThermalMod = TARCOGParams::TARCOGThermalModel::ISO15099; // Thermal model:
        //                 0 - ISO15099
        //                 1 - Scaled Cavity Width (SCW)
        //                 2 - Convective Scalar Model (CSM)
        std::string Debug_dir;          // Target directory for debug files (pointer to a character array)
        std::string Debug_file("Test"); // Template file name used to create debug output files
        std::int32_t Window_ID(-1);     // ID of the window (long integer value, passed by W6)
        std::int32_t IGU_ID(-1);        // ID of the IGU (long integer value, passed by W6)
        Real64 SDScalar(0.0);           // SD convection factor (value between 0 and 1)
        //                 0.0 - No SD layer
        //                 1.0 - Closed SD
        //               Notes:   * vvent, tvent, Atop, Abot, Al, Ar and Ah are considered for SD layers only.
        //                       ** SlatThick, SlatWidth, SlatAngle, SlatCond, SlatSpacing, SlatCurve
        //                          are used for Venetian blind layers only.
        //                      *** For vvent & tvent: vvent(1) - exterior, vvent(nlayer+1) - interior.
        //                     **** Forced ventilation calculation is not active at this time.
        // TARCOG Output:

        Real64 ufactor(0.0);           // Center of glass U-value [W/m^2.K]
        Real64 sc(0.0);                // Shading Coefficient
        Real64 hflux(0.0);             // Net heat flux between room and window [W/m^2]
        Real64 hcin(0.0);              // Indoor convective surface heat transfer coefficient  [W/m^2.K]
        Real64 hcout(0.0);             // Outdoor convective surface heat transfer coefficient [W/m^2.K]
        Real64 hrin(0.0);              // Indoor radiative surface heat transfer coefficient [W/m^2.K]
        Real64 hrout(0.0);             // Outdoor radiative surface heat transfer coefficient [W/m^2.K]
        Real64 shgc(0.0);              // Solar heat gain coefficient - per ISO 15099
        Real64 shgct(0.0);             // Solar heat gain coefficient - per old procedure
        Real64 tamb(0.0);              // Outdoor environmental temperature [K]
        Real64 troom(0.0);             // Indoor environmental temperature [K]
        Real64 he(0.0);                // External heat transfer coefficient [W/m^2.K] - EN673 and ISO 10292 procedure
        Real64 hi(0.0);                // Internal heat transfer coefficient [W/m^2.K] - EN673 and ISO 10292 procedure
        int nperr(0);                  // Error code
        Real64 ShadeEmisRatioOut(0.0); // Ratio of modified to glass emissivity at the outermost glazing surface
        Real64 ShadeEmisRatioIn(0.0);  // Ratio of modified to glass emissivity at the innermost glazing surface
        Real64 ShadeHcRatioOut(0.0);   // Ratio of modified to unshaded Hc at the outermost glazing surface
        Real64 ShadeHcRatioIn(0.0);    // Ratio of modified to unshaded Hc at the innermost glazing surface
        Real64 HcUnshadedOut(0.0);     // Hc value at outdoor surface of an unshaded subsystem [W/m^2.K]
        Real64 HcUnshadedIn(0.0);      // Hc value at indoor surface of an unshaded subsystem [W/m^2.K]

        int ZoneNum; // Zone number corresponding to SurfNum

        int TotLay; // Total number of layers in a construction
        //   (sum of solid layers and gap layers)
        int Lay;                  // Layer number
        int LayPtr;               // Material number for a layer
        int IGlass;               // glass layer number (1,2,3,...)
        int IGap;                 // Gap layer number (1,2,...)
        int TotGlassLay;          // Total number of glass layers in a construction
        int k;                    // Layer counter
        int SurfNumAdj;           // An interzone surface's number in the adjacent zone
        WinShadingType ShadeFlag; // Flag indicating whether shade or blind is on, and shade/blind position
        int IMix;

        Real64 IncidentSolar;       // Solar incident on outside of window (W)
        Real64 ConvHeatFlowNatural; // Convective heat flow from gap between glass and interior shade or blind (W)
        Real64 ShadeArea;           // shade/blind area (m2)
        Real64 sconsh;              // shade/blind conductance (W/m2-K)
        Real64 CondHeatGainShade;   // Conduction through shade/blind, outside to inside (W)

        Real64 ShGlReflFacIR; // Factor for long-wave inter-reflection between shade/blind and adjacent glass
                              //        Real64 RhoGlIR1; // Long-wave reflectance of glass surface facing shade/blind; 1=exterior shade/blind,
        Real64 RhoGlIR2;
        //  2=interior shade/blind
        Real64 RhoShIR1; // Long-wave reflectance of shade/blind surface facing glass; 1=interior shade/blind,
        Real64 RhoShIR2;
        //  2=exterior shade/blind
        Real64 EpsShIR1; // Long-wave emissivity of shade/blind surface facing glass; 1=interior shade/blind,
        Real64 EpsShIR2;
        //  2=exterior shade/blind
        Real64 TauShIR;            // Long-wave transmittance of isolated shade/blind
        Real64 NetIRHeatGainShade; // Net IR heat gain to zone from interior shade/blind (W)
        Real64 NetIRHeatGainGlass; // Net IR heat gain to zone from shade/blind side of glass when interior
        //  shade/blind is present. Zero if shade/blind has zero IR transmittance (W)
        Real64 ConvHeatGainFrZoneSideOfShade; // Convective heat gain to zone from side of interior shade facing zone (W)
        Real64 ConvHeatGainFrZoneSideOfGlass; // Convective heat gain to zone from side of glass facing zone when
        //  no interior shade/blind is present (W)
        Real64 CondHeatGainGlass;     // Conduction through inner glass layer, outside to inside (W)
        Real64 TotAirflowGap;         // Total volumetric airflow through window gap (m3/s)
        Real64 TAirflowGapOutlet;     // Temperature of air leaving airflow gap between glass panes (K)
        Real64 TAirflowGapOutletC;    // Temperature of air leaving airflow gap between glass panes (C)
        Real64 ConvHeatFlowForced;    // Convective heat flow from forced airflow gap (W)
        Real64 InletAirHumRat;        // Humidity ratio of air from window gap entering fan
        Real64 ZoneTemp;              // Zone air temperature (C)
        Real64 CpAirOutlet;           // Heat capacity of air from window gap (J/kg-K)
        Real64 CpAirZone;             // Heat capacity of zone air (J/kg-K)
        Real64 ConvHeatGainToZoneAir; // Convective heat gain to zone air from window gap airflow (W)
                                      //        int ConstrNumSh; // Construction number with shading device
        int CalcSHGC(0);              // SHGC calculations are not necessary for E+ run
        int NumOfIterations(0);

        std::string tarcogErrorMessage; // store error text from tarcog

        // Simon: locally used variables
        int ngllayer;
        int nglface;
        int nglfacep;
        int ThermalModelNum;
        Real64 rmir; // IR radiance of window's interior surround (W/m2)
        Real64 outir;
        Real64 Ebout;
        Real64 dominantGapWidth; // store value for dominant gap width.  Used for airflow calculations
        Real64 edgeGlCorrFac;

        Real64 SrdSurfTempAbs; // Absolute temperature of a surrounding surface
        Real64 OutSrdIR;

        auto &s_mat = state.dataMaterial;
        // fill local vars

        CalcDeflection = TARCOGParams::DeflectionCalculation::NONE;
        CalcSHGC = 0;

        if (CalcCondition == DataBSDFWindow::Condition::Invalid) {
            ConstrNum = state.dataSurface->Surface(SurfNum).Construction;
            SurfNumAdj = state.dataSurface->Surface(SurfNum).ExtBoundCond;
            ShadeFlag = state.dataSurface->SurfWinShadingFlag(SurfNum);
        }

        TotGlassLay = state.dataConstruction->Construct(ConstrNum).TotGlassLayers;
        ngllayer = state.dataConstruction->Construct(ConstrNum).TotGlassLayers;
        nglface = 2 * ngllayer;
        nglfacep = nglface;
        hrin = 0.0;
        hcin = 0.0;
        hrout = 0.0;
        hcout = 0.0;

        Pa = state.dataEnvrn->OutBaroPress;

        ThermalModelNum = state.dataConstruction->Construct(ConstrNum).BSDFInput.ThermalModel;
        standard = state.dataMaterial->WindowThermalModel(ThermalModelNum).CalculationStandard;
        ThermalMod = state.dataMaterial->WindowThermalModel(ThermalModelNum).ThermalModel;
        CalcDeflection = state.dataMaterial->WindowThermalModel(ThermalModelNum).DeflectionModel;
        SDScalar = state.dataMaterial->WindowThermalModel(ThermalModelNum).SDScalar;
        VacuumPressure = state.dataMaterial->WindowThermalModel(ThermalModelNum).VacuumPressureLimit;
        Tini = state.dataMaterial->WindowThermalModel(ThermalModelNum).InitialTemperature - Constant::Kelvin;
        Pini = state.dataMaterial->WindowThermalModel(ThermalModelNum).InitialPressure;

        nlayer = state.dataConstruction->Construct(ConstrNum).TotSolidLayers;
        isky = 3; // IR radiation is provided from external source
        iwd = 0;  // assume windward for now.  TODO compare surface normal with wind direction

        if (CalcCondition == DataBSDFWindow::Condition::Invalid) {
            ZoneNum = state.dataSurface->Surface(SurfNum).Zone;
            Real64 RefAirTemp = state.dataSurface->Surface(SurfNum).getInsideAirTemperature(state, SurfNum);
            tind = RefAirTemp + Constant::Kelvin; // Inside air temperature

            // now get "outside" air temperature
            if (SurfNumAdj > 0) { // Interzone window

                int enclNumAdj = state.dataSurface->Surface(SurfNumAdj).RadEnclIndex;
                RefAirTemp = state.dataSurface->Surface(SurfNumAdj).getInsideAirTemperature(state, SurfNumAdj);
                tout = RefAirTemp + Constant::Kelvin; // outside air temperature

                tsky = state.dataViewFactor->EnclRadInfo(enclNumAdj).MRT +
                       Constant::Kelvin; // TODO this misses IR from sources such as high temp radiant and baseboards

                //  ! Add long-wave radiation from adjacent zone absorbed by glass layer closest to the adjacent zone.
                //  AbsRadGlassFace(1) = AbsRadGlassFace(1) + SurfQRadThermInAbs(SurfNumAdj)
                //  ! The IR radiance of this window's "exterior" surround is the IR radiance
                //  ! from surfaces and high-temp radiant sources in the adjacent zone
                outir = state.dataSurface->SurfWinIRfromParentZone(SurfNumAdj) + state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(SurfNumAdj);

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
                        tout = state.dataSurface->SurfOutWetBulbTemp(SurfNum) + Constant::Kelvin;
                    } else { // Dry
                        tout = state.dataSurface->SurfOutDryBulbTemp(SurfNum) + Constant::Kelvin;
                    }
                } else { // Window not exposed to wind
                    tout = state.dataSurface->SurfOutDryBulbTemp(SurfNum) + Constant::Kelvin;
                }
                // tsky = SkyTemp + TKelvin
                tsky = state.dataEnvrn->SkyTempKelvin;
                Ebout = state.dataWindowComplexManager->sigma * pow_4(tout);
                outir = state.dataSurface->Surface(SurfNum).ViewFactorSkyIR *
                            (state.dataSurface->SurfAirSkyRadSplit(SurfNum) * state.dataWindowComplexManager->sigma * pow_4(tsky) +
                             (1.0 - state.dataSurface->SurfAirSkyRadSplit(SurfNum)) * Ebout) +
                        state.dataSurface->Surface(SurfNum).ViewFactorGroundIR * Ebout + OutSrdIR;
            }

            hin = state.dataHeatBalSurf->SurfHConvInt(SurfNum); // Room-side surface convective film conductance
            ibc(2) = 0; // convective coefficient on indoor side will be recalculated (like in Winkelmann routines)

            // hcout=HextConvCoeff  ! Exterior convection coefficient is passed in from outer routine
            hout = HextConvCoeff; // Exterior convection coefficient is passed in from outer routine
            ibc(1) = 2;           // prescribed convective film coeff on outdoor side
            tilt = state.dataSurface->Surface(SurfNum).Tilt;
            height = state.dataSurface->Surface(SurfNum).Height;
            heightt = height; // for now put same window and glazing pocket hights
            width = state.dataSurface->Surface(SurfNum).Width;

            // indoor mean radiant temperature.
            // IR incident on window from zone surfaces and high-temp radiant sources
            rmir = state.dataSurface->SurfWinIRfromParentZone(SurfNum) + state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(SurfNum);
            trmin = root_4(rmir / Constant::StefanBoltzmann); // TODO check model equation.

            // outdoor wind speed
            if (!state.dataSurface->Surface(SurfNum).ExtWind) {
                wso = 0.0; // No wind exposure
                           // ELSE IF (Surface(SurfNum)%Class == SurfaceClass::Window .AND. SurfaceWindow(SurfNum)%ShadingFlag ==
                           // WinShadingType::ExtShade) THEN
                //  wso =  0.0  ! Assume zero wind speed at outside glass surface of window with exterior shade
            } else {
                wso = state.dataSurface->SurfOutWindSpeed(SurfNum);
            }

            // indoor wind speed
            wsi = 0.0; // assumuption (TODO, what to use for inside air velocity?)

            fclr = 1.0 - state.dataEnvrn->CloudFraction;
        }

        TotLay = state.dataConstruction->Construct(ConstrNum).TotLayers;
        IGap = 0;

        //****************************************************************************************************
        // Inside and outside gas coefficients
        //****************************************************************************************************
        iprop(1, 1) = 1;  // air on outdoor side
        frct(1, 1) = 1.0; // pure air on outdoor side
        nmix(1) = 1;      // pure air on outdoor side

        iprop(1, nlayer + 1) = 1;  // air on indoor side
        frct(1, nlayer + 1) = 1.0; // pure air on indoor side
        nmix(nlayer + 1) = 1;      // pure air on indoor side

        // Simon: feed gas coefficients with air.  This is necessary for tarcog because it is used on indoor and outdoor sides
        auto const &gasAir = Material::gases[(int)Material::GasType::Air];
        wght(iprop(1, 1)) = gasAir.wght;
        gama(iprop(1, 1)) = gasAir.specHeatRatio;

        gcon(1, iprop(1, 1)) = gasAir.con.c0;
        gcon(2, iprop(1, 1)) = gasAir.con.c1;
        gcon(3, iprop(1, 1)) = gasAir.con.c2;
        gvis(1, iprop(1, 1)) = gasAir.vis.c0;
        gvis(2, iprop(1, 1)) = gasAir.vis.c1;
        gvis(3, iprop(1, 1)) = gasAir.vis.c2;
        gcp(1, iprop(1, 1)) = gasAir.cp.c0;
        gcp(2, iprop(1, 1)) = gasAir.cp.c1;
        gcp(3, iprop(1, 1)) = gasAir.cp.c2;

        // Fill window layer properties needed for window layer heat balance calculation
        IGlass = 0;
        IGap = 0;
        for (Lay = 1; Lay <= TotLay; ++Lay) {
            LayPtr = state.dataConstruction->Construct(ConstrNum).LayerPoint(Lay);
            auto const *mat = s_mat->materials(LayPtr);

            if ((mat->group == Material::Group::Glass) || (mat->group == Material::Group::GlassSimple)) {
                auto const *matGlass = dynamic_cast<Material::MaterialGlass const *>(mat);
                assert(matGlass != nullptr);

                ++IGlass;
                LayerType(IGlass) = TARCOGParams::TARCOGLayerType::SPECULAR; // this marks specular layer type
                thick(IGlass) = matGlass->Thickness;
                scon(IGlass) = matGlass->Conductivity;
                emis(2 * IGlass - 1) = matGlass->AbsorpThermalFront;
                emis(2 * IGlass) = matGlass->AbsorpThermalBack;
                tir(2 * IGlass - 1) = matGlass->TransThermal;
                tir(2 * IGlass) = matGlass->TransThermal;
                YoungsMod(IGlass) = matGlass->YoungModulus;
                PoissonsRat(IGlass) = matGlass->PoissonsRatio;

            } else if (mat->group == Material::Group::ComplexShade) {
                auto const *matShade = dynamic_cast<Material::MaterialComplexShade const *>(mat);
                ++IGlass;
                LayerType(IGlass) = matShade->LayerType;

                thick(IGlass) = matShade->Thickness;
                scon(IGlass) = matShade->Conductivity;
                emis(2 * IGlass - 1) = matShade->FrontEmissivity;
                emis(2 * IGlass) = matShade->BackEmissivity;
                tir(2 * IGlass - 1) = matShade->TransThermal;
                tir(2 * IGlass) = matShade->TransThermal;

                // This needs to be converted into correct areas. That can be done only after loading complete window data
                Atop(IGlass) = matShade->topOpeningMult;
                Abot(IGlass) = matShade->bottomOpeningMult;
                Al(IGlass) = matShade->leftOpeningMult;
                Ar(IGlass) = matShade->rightOpeningMult;
                Ah(IGlass) = matShade->frontOpeningMult;

                SlatThick(IGlass) = matShade->SlatThickness;
                SlatWidth(IGlass) = matShade->SlatWidth;
                SlatAngle(IGlass) = matShade->SlatAngle;
                SlatCond(IGlass) = matShade->SlatConductivity;
                SlatSpacing(IGlass) = matShade->SlatSpacing;
                SlatCurve(IGlass) = matShade->SlatCurve;

            } else if (mat->group == Material::Group::ComplexWindowGap) {
                auto const *matGap = dynamic_cast<Material::MaterialComplexWindowGap const *>(mat);
                ++IGap;
                gap(IGap) = matGap->Thickness;
                presure(IGap) = matGap->Pressure;

                GapDefMax(IGap) = matGap->deflectedThickness;

                PillarSpacing(IGap) = matGap->pillarSpacing;
                PillarRadius(IGap) = matGap->pillarRadius;
                SupportPlr(IGap) = matGap->pillarSpacing != 0.0 && matGap->pillarRadius != 0.0;

                nmix(IGap + 1) = matGap->numGases;
                for (IMix = 1; IMix <= nmix(IGap + 1); ++IMix) {
                    auto const &gas = matGap->gases[IMix - 1];
                    frct(IMix, IGap + 1) = matGap->gasFracts[IMix - 1];

                    // Now has to build-up gas coefficients arrays. All used gasses should be stored into these arrays and
                    // to be correctly referenced by gap arrays

                    // First check if gas coefficients are already part of array.  Duplicates are not necessary
                    bool feedData(false);
                    CheckGasCoefs(gas.wght, iprop(IMix, IGap + 1), wght, feedData);
                    if (feedData) {
                        wght(iprop(IMix, IGap + 1)) = gas.wght;
                        gama(iprop(IMix, IGap + 1)) = gas.specHeatRatio;

                        gcon(1, iprop(IMix, IGap + 1)) = gas.con.c0;
                        gcon(2, iprop(IMix, IGap + 1)) = gas.con.c1;
                        gcon(3, iprop(IMix, IGap + 1)) = gas.con.c2;

                        gvis(1, iprop(IMix, IGap + 1)) = gas.vis.c0;
                        gvis(2, iprop(IMix, IGap + 1)) = gas.vis.c1;
                        gvis(3, iprop(IMix, IGap + 1)) = gas.vis.c2;

                        gcp(1, iprop(IMix, IGap + 1)) = gas.cp.c0;
                        gcp(2, iprop(IMix, IGap + 1)) = gas.cp.c1;
                        gcp(3, iprop(IMix, IGap + 1)) = gas.cp.c2;
                    } // IF feedData THEN
                }
            } else {
                ShowContinueError(state, "Illegal layer type in Construction:ComplexFenestrationState.");
                ShowContinueError(state, "Allowed object are:");
                ShowContinueError(state, "   - WindowMaterial:Glazing");
                ShowContinueError(state, "   - WindowMaterial:ComplexShade");
                ShowContinueError(state, "   - WindowMaterial:Gap");
                ShowFatalError(state, "halting because of error in layer definition for Construction:ComplexFenestrationState");
            }

        } // End of loop over glass, gap and blind/shade layers in a window construction

        if (CalcCondition == DataBSDFWindow::Condition::Invalid) {
            // now calculate correct areas for multipliers
            for (Lay = 1; Lay <= nlayer; ++Lay) {
                if (LayerType(Lay) != TARCOGParams::TARCOGLayerType::SPECULAR) { // Layer is shading
                    // before changing multipliers, need to determine which one is dominant gap width
                    if (Lay == 1) { // Exterior shading device
                        dominantGapWidth = gap(Lay);
                    } else if (Lay == nlayer) { // Interior shading device
                        dominantGapWidth = gap(Lay - 1);
                    } else { // In-between shading device
                        dominantGapWidth = min(gap(Lay - 1), gap(Lay));
                    }
                    Atop(Lay) *= dominantGapWidth * width;
                    Abot(Lay) *= dominantGapWidth * width;
                    Al(Lay) *= dominantGapWidth * height;
                    Ar(Lay) *= dominantGapWidth * height;
                    Ah(Lay) *= width * height;
                }
            }
        }

        // ThermalMod = 0
        CalcSHGC = 0;

        Window_ID = ConstrNum;

        // vector of absorbed solar energy fractions for each layer.
        asol = 0.0;
        // direct solar radiation
        if (CalcCondition == DataBSDFWindow::Condition::Invalid) {
            ShadeFlag = state.dataSurface->SurfWinShadingFlag(SurfNum);
            dir = state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) +
                  state.dataHeatBal->EnclSolQSWRad(state.dataSurface->Surface(SurfNum).SolarEnclIndex); // TODO, check , !
            //                  currently using Exterior beam plus diffuse solar incident on surface
            //                  plus zone short wave.  CHECK
            // if (dir.ne.0.0d0) then
            for (IGlass = 1; IGlass <= nlayer; ++IGlass) {
                // IF (dir > 0.0D0 ) THEN
                asol(IGlass) = state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, IGlass);
                // ELSE
                //  asol(IGLASS) = 0.0D0
                // ENDIF
            }
            // end if

            // Add contribution of IR from zone internal gains (lights, equipment and people). This is absorbed in zone-side layer and it
            // is assumed that nothing is transmitted through
            asol(nlayer) += state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum);

            presure = state.dataEnvrn->OutBaroPress;

            // Instead of doing temperature guess get solution from previous iteration.  That should be much better than guess
            for (k = 1; k <= 2 * nlayer; ++k) {
                theta(k) = state.dataSurface->SurfaceWindow(SurfNum).thetaFace[k];
            }
        }

        // Standard conditions run (winter and summer)
        if (CalcCondition == DataBSDFWindow::Condition::Winter) {
            tind = 294.15;
            tout = 255.15;
            hcout = 26.0;
            wso = 5.5;
            dir = 0.0;
        } else if (CalcCondition == DataBSDFWindow::Condition::Summer) {
            tind = 297.15;
            tout = 305.15;
            hcout = 15.0;
            wso = 2.75;
            dir = 783.0;
            CalcSHGC = 1;
        }

        // Common condition data
        if (CalcCondition != DataBSDFWindow::Condition::Invalid) {
            trmin = tind;
            outir = 0.0;
            tsky = tout;
            wsi = 0.0;
            fclr = 1.0;
            ibc(1) = 0;
            ibc(2) = 0;
            presure = 101325.0;
            iwd = 0; // Windward wind direction
            isky = 0;
            esky = 1.0;
            height = 1.0;
            heightt = 1.0;
            width = 1.0;
            tilt = 90.0;
            // Just to make initial quess different from absolute zero
            theta = 273.15;
        }

        edgeGlCorrFac = (SurfNum != 0) ? state.dataSurface->SurfaceWindow(SurfNum).edgeGlassCorrFac : 1;

        //  call TARCOG
        int constexpr Debug_mode = 0;
        TARCOG90(state,
                 nlayer,
                 iwd,
                 tout,
                 tind,
                 trmin,
                 wso,
                 wsi,
                 dir,
                 outir,
                 isky,
                 tsky,
                 esky,
                 fclr,
                 VacuumPressure,
                 VacuumMaxGapThickness,
                 CalcDeflection,
                 Pa,
                 Pini,
                 Tini,
                 gap,
                 GapDefMax,
                 thick,
                 scon,
                 YoungsMod,
                 PoissonsRat,
                 tir,
                 emis,
                 totsol,
                 tilt,
                 asol,
                 height,
                 heightt,
                 width,
                 presure,
                 iprop,
                 frct,
                 gcon,
                 gvis,
                 gcp,
                 wght,
                 gama,
                 nmix,
                 SupportPlr,
                 PillarSpacing,
                 PillarRadius,
                 theta,
                 LayerDef,
                 q,
                 qv,
                 ufactor,
                 sc,
                 hflux,
                 hcin,
                 hcout,
                 hrin,
                 hrout,
                 hin,
                 hout,
                 hcgap,
                 hrgap,
                 shgc,
                 nperr,
                 tarcogErrorMessage,
                 shgct,
                 tamb,
                 troom,
                 ibc,
                 Atop,
                 Abot,
                 Al,
                 Ar,
                 Ah,
                 SlatThick,
                 SlatWidth,
                 SlatAngle,
                 SlatCond,
                 SlatSpacing,
                 SlatCurve,
                 vvent,
                 tvent,
                 LayerType,
                 nslice,
                 LaminateA,
                 LaminateB,
                 sumsol,
                 hg,
                 hr,
                 hs,
                 he,
                 hi,
                 Ra,
                 Nu,
                 standard,
                 ThermalMod,
                 Debug_mode,
                 Debug_dir,
                 Debug_file,
                 Window_ID,
                 IGU_ID,
                 ShadeEmisRatioOut,
                 ShadeEmisRatioIn,
                 ShadeHcRatioOut,
                 ShadeHcRatioIn,
                 HcUnshadedOut,
                 HcUnshadedIn,
                 Keff,
                 ShadeGapKeffConv,
                 SDScalar,
                 CalcSHGC,
                 NumOfIterations,
                 edgeGlCorrFac);

        // process results from TARCOG
        if ((nperr > 0) && (nperr < 1000)) { // process error signal from tarcog

            ShowSevereError(state, "Window tarcog returned an error");
            tarcogErrorMessage = "message = \"" + tarcogErrorMessage + "\"";
            ShowContinueErrorTimeStamp(state, tarcogErrorMessage);
            if (CalcCondition == DataBSDFWindow::Condition::Invalid) {
                ShowContinueError(state, format("surface name = {}", state.dataSurface->Surface(SurfNum).Name));
            }
            ShowContinueError(state, format("construction name = {}", state.dataConstruction->Construct(ConstrNum).Name));
            ShowFatalError(state, "halting because of error in tarcog");
        } else if (CalcCondition == DataBSDFWindow::Condition::Winter) {
            state.dataHeatBal->NominalU(ConstrNum) = ufactor;
        } else if (CalcCondition == DataBSDFWindow::Condition::Summer) {
            // tempInt = SurfaceWindow(SurfNum)%ComplexFen%CurrentState
            // tempReal = SurfaceWindow(SurfNum)%ComplexFen%State(tempInt)%WinDiffTrans

            // Sum1 = 0.0d0
            // Sum2 = 0.0d0
            // do  j = 1 , ComplexWind(SurfNum)%Geom%Inc%NBasis     !Incident ray loop
            //  Sum2 = Sum2 + ComplexWind(SurfNum)%Geom%Inc%Lamda (j)
            //  do  m = 1 , ComplexWind(SurfNum)%Geom%Trn%NBasis     !Outgoing ray loop
            //    Sum1 =Sum1 + ComplexWind(SurfNum)%Geom%Inc%Lamda(j) * ComplexWind(SurfNum)%Geom%Trn%Lamda(m) * &
            //      & Construct(ConstrNum)%BSDFInput%SolFrtTrans ( j , m )
            //  end do      !Outgoing ray loop
            // end do      !Incident ray loop
            // if (Sum2 > 0.0d0) THEN
            //  tempReal = Sum1/Sum2
            // else
            //  tempReal = 0.0d0
            //  CALL ShowWarningError(state, 'BSDF--Inc basis has zero projected solid angle')
            // endif

            state.dataConstruction->Construct(ConstrNum).SummerSHGC = shgc;

            // Construct(SurfNum)%VisTransNorm = SurfaceWindow(SurfNum)%ComplexFen%State(tempInt)%WinDiffVisTrans
        } else if (CalcCondition == DataBSDFWindow::Condition::Invalid) { // expect converged results...
            // Window heat balance solution has converged.

            state.dataSurface->SurfWinWindowCalcIterationsRep(SurfNum) = NumOfIterations;
            state.dataHeatBalSurf->SurfHConvInt(SurfNum) = hcin;

            // For interior shade, add convective gain from glass/shade gap air flow to zone convective gain;
            // For all cases, get total window heat gain for reporting. See CalcWinFrameAndDividerTemps for
            // contribution of frame and divider.

            SurfInsideTemp = theta(2 * nlayer) - Constant::Kelvin;
            state.dataSurface->SurfWinEffInsSurfTemp(SurfNum) = SurfInsideTemp;
            SurfOutsideTemp = theta(1) - Constant::Kelvin;
            SurfOutsideEmiss = emis(1);

            IncidentSolar = state.dataSurface->Surface(SurfNum).Area * state.dataHeatBal->SurfQRadSWOutIncident(SurfNum);
            if (ANY_INTERIOR_SHADE_BLIND(ShadeFlag)) {
                // Interior shade or blind
                ConvHeatFlowNatural = -qv(nlayer) * height * width;

                state.dataSurface->SurfWinConvHeatFlowNatural(SurfNum) = ConvHeatFlowNatural;
                state.dataSurface->SurfWinGapConvHtFlowRep(SurfNum) = ConvHeatFlowNatural;
                state.dataSurface->SurfWinGapConvHtFlowRepEnergy(SurfNum) =
                    state.dataSurface->SurfWinGapConvHtFlowRep(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                // Window heat gain from glazing and shade/blind to zone. Consists of transmitted solar, convection
                //   from air exiting gap, convection from zone-side of shade/blind, net IR to zone from shade and net IR to
                //   zone from the glass adjacent to the shade/blind (zero if shade/blind IR transmittance is zero).
                // Following assumes glazed area = window area (i.e., dividers ignored) in calculating
                //   IR to zone from glass when interior shade/blind is present.
                ShadeArea = state.dataSurface->Surface(SurfNum).Area + state.dataSurface->SurfWinDividerArea(SurfNum);
                sconsh = scon(ngllayer + 1) / thick(ngllayer + 1);
                nglfacep = nglface + 2;
                CondHeatGainShade = ShadeArea * sconsh * (theta(nglfacep - 1) - theta(nglfacep));
                EpsShIR1 = emis(nglface + 1);
                EpsShIR2 = emis(nglface + 2);
                TauShIR = tir(nglface + 1);
                RhoShIR1 = max(0.0, 1.0 - TauShIR - EpsShIR1);
                RhoShIR2 = max(0.0, 1.0 - TauShIR - EpsShIR2);
                RhoGlIR2 = 1.0 - emis(2 * ngllayer);
                ShGlReflFacIR = 1.0 - RhoGlIR2 * RhoShIR1;
                NetIRHeatGainShade =
                    ShadeArea * EpsShIR2 * (state.dataWindowComplexManager->sigma * pow_4(theta(nglfacep)) - rmir) +
                    EpsShIR1 * (state.dataWindowComplexManager->sigma * pow_4(theta(nglfacep - 1)) - rmir) * RhoGlIR2 * TauShIR / ShGlReflFacIR;
                NetIRHeatGainGlass = ShadeArea * (emis(2 * ngllayer) * TauShIR / ShGlReflFacIR) *
                                     (state.dataWindowComplexManager->sigma * pow_4(theta(2 * ngllayer)) - rmir);
                ConvHeatGainFrZoneSideOfShade = ShadeArea * hcin * (theta(nglfacep) - tind);
                state.dataSurface->SurfWinHeatGain(SurfNum) = state.dataSurface->SurfWinTransSolar(SurfNum) + ConvHeatFlowNatural +
                                                              ConvHeatGainFrZoneSideOfShade + NetIRHeatGainGlass + NetIRHeatGainShade;
                // store components for reporting
                state.dataSurface->SurfWinGainConvShadeToZoneRep(SurfNum) = ConvHeatGainFrZoneSideOfShade;
                state.dataSurface->SurfWinGainIRGlazToZoneRep(SurfNum) = NetIRHeatGainGlass;
                state.dataSurface->SurfWinGainIRShadeToZoneRep(SurfNum) = NetIRHeatGainShade;
            } else {
                // Interior shade or blind not present; innermost layer is glass
                CondHeatGainGlass =
                    state.dataSurface->Surface(SurfNum).Area * scon(nlayer) / thick(nlayer) * (theta(2 * nlayer - 1) - theta(2 * nlayer));
                NetIRHeatGainGlass = state.dataSurface->Surface(SurfNum).Area * emis(2 * nlayer) *
                                     (state.dataWindowComplexManager->sigma * pow_4(theta(2 * nlayer)) - rmir);
                ConvHeatGainFrZoneSideOfGlass = state.dataSurface->Surface(SurfNum).Area * hcin * (theta(2 * nlayer) - tind);
                state.dataSurface->SurfWinHeatGain(SurfNum) =
                    state.dataSurface->SurfWinTransSolar(SurfNum) + ConvHeatGainFrZoneSideOfGlass + NetIRHeatGainGlass;
                // store components for reporting
                state.dataSurface->SurfWinGainConvGlazToZoneRep(SurfNum) = ConvHeatGainFrZoneSideOfGlass;
                state.dataSurface->SurfWinGainIRGlazToZoneRep(SurfNum) = NetIRHeatGainGlass;
                // need to report convective heat flow from the gap in case of exterior shade
                if (ShadeFlag == WinShadingType::ExtShade) {
                    ConvHeatFlowNatural = -qv(2) * height * width; // qv(1) is exterior environment

                    state.dataSurface->SurfWinGapConvHtFlowRep(SurfNum) = ConvHeatFlowNatural;
                    state.dataSurface->SurfWinGapConvHtFlowRepEnergy(SurfNum) =
                        state.dataSurface->SurfWinGapConvHtFlowRep(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                }
            }

            // Add convective heat gain from airflow window
            // Note: effect of fan heat on gap outlet temperature is neglected since fan power (based
            // on pressure drop through the gap) is extremely small

            // WinGapConvHtFlowRep(SurfNum) = 0.0d0
            // WinGapConvHtFlowRepEnergy(SurfNum) = 0.0d0
            TotAirflowGap = state.dataSurface->SurfWinAirflowThisTS(SurfNum) * state.dataSurface->Surface(SurfNum).Width;
            TAirflowGapOutlet = Constant::Kelvin; // TODO Need to calculate this
            TAirflowGapOutletC = TAirflowGapOutlet - Constant::Kelvin;
            state.dataSurface->SurfWinTAirflowGapOutlet(SurfNum) = TAirflowGapOutletC;
            if (state.dataSurface->SurfWinAirflowThisTS(SurfNum) > 0.0) {
                ConvHeatFlowForced = sum(qv); // TODO.  figure forced ventilation heat flow in Watts

                state.dataSurface->SurfWinGapConvHtFlowRep(SurfNum) = ConvHeatFlowForced;
                state.dataSurface->SurfWinGapConvHtFlowRepEnergy(SurfNum) =
                    state.dataSurface->SurfWinGapConvHtFlowRep(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                // Add heat from gap airflow to zone air if destination is inside air; save the heat gain to return
                // air in case it needs to be sent to the zone (due to no return air determined in HVAC simulation)
                if (state.dataSurface->SurfWinAirflowDestination(SurfNum) == WindowAirFlowDestination::Indoor ||
                    state.dataSurface->SurfWinAirflowDestination(SurfNum) == WindowAirFlowDestination::Return) {
                    auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
                    if (state.dataSurface->SurfWinAirflowSource(SurfNum) == WindowAirFlowSource::Indoor) {
                        InletAirHumRat = thisZoneHB.airHumRat;
                    } else { // AirflowSource = outside air
                        InletAirHumRat = state.dataEnvrn->OutHumRat;
                    }
                    ZoneTemp = thisZoneHB.MAT; // this should be Tin (account for different reference temps)
                    CpAirOutlet = PsyCpAirFnW(InletAirHumRat);
                    CpAirZone = PsyCpAirFnW(thisZoneHB.airHumRat);
                    ConvHeatGainToZoneAir = TotAirflowGap * (CpAirOutlet * (TAirflowGapOutletC)-CpAirZone * ZoneTemp);
                    if (state.dataSurface->SurfWinAirflowDestination(SurfNum) == WindowAirFlowDestination::Indoor) {
                        state.dataSurface->SurfWinConvHeatGainToZoneAir(SurfNum) = ConvHeatGainToZoneAir;
                        state.dataSurface->SurfWinHeatGain(SurfNum) += ConvHeatGainToZoneAir;
                    } else {
                        state.dataSurface->SurfWinRetHeatGainToZoneAir(SurfNum) = ConvHeatGainToZoneAir;
                    }
                }
                // For AirflowDestination = ReturnAir in a controlled (i.e., conditioned) zone with return air, see CalcZoneLeavingConditions
                // for calculation of modification of return-air temperature due to airflow from window gaps into return air.
            }

            // Correct WinHeatGain for interior diffuse shortwave (solar and shortwave from lights) transmitted
            // back out window
            ConstrNum = state.dataSurface->Surface(SurfNum).Construction;
            // ConstrNumSh = Surface(SurfNum)%ShadedConstruction
            // IF(SurfaceWindow(SurfNum)%StormWinFlag==1) THEN
            //  ConstrNum = Surface(SurfNum)%StormWinConstruction
            //  ConstrNumSh = Surface(SurfNum)%StormWinShadedConstruction
            // END IF
            // IF(ShadeFlag <= 0) THEN
            // TransDiff = Construct(ConstrNum).TransDiff;
            int IState = state.dataSurface->SurfaceWindow(SurfNum).ComplexFen.NumStates;
            Real64 ReflDiff = state.dataSurface->SurfaceWindow(SurfNum).ComplexFen.State(IState).WinBkHemRefl;
            // ELSE IF(ShadeFlag==WinShadingType::IntShade .OR. ShadeFlag==WinShadingType::ExtShade) THEN
            //  TransDiff = Construct(ConstrNum)%TransDiff
            // ELSE IF(ShadeFlag==WinShadingType::IntBlind .OR. ShadeFlag==WinShadingType::ExtBlind .OR.ShadeFlag==WinShadingType::BGBlind) THEN
            //  TransDiff = InterpSlatAng(SurfaceWindow(SurfNum)%SlatAngThisTS,SurfaceWindow(SurfNum)%MovableSlats, &
            //                             Construct(ConstrNumSh)%BlTransDiff)
            // ELSE IF(ShadeFlag == SwitchableGlazing) THEN
            //  TransDiff = InterpSW(SurfaceWindow(SurfNum)%SwitchingFactor,Construct(ConstrNum)%TransDiff, &
            //                             Construct(ConstrNumSh)%TransDiff)
            // END IF
            state.dataSurface->SurfWinLossSWZoneToOutWinRep(SurfNum) =
                state.dataHeatBal->EnclSolQSWRad(state.dataSurface->Surface(SurfNum).SolarEnclIndex) * state.dataSurface->Surface(SurfNum).Area *
                    (1 - ReflDiff) +
                state.dataHeatBalSurf->SurfWinInitialBeamSolInTrans(SurfNum);
            state.dataSurface->SurfWinHeatGain(SurfNum) -=
                (state.dataSurface->SurfWinLossSWZoneToOutWinRep(SurfNum) +
                 state.dataHeatBalSurf->SurfWinInitialDifSolInTrans(SurfNum) * state.dataSurface->Surface(SurfNum).Area);

            if (ShadeFlag == WinShadingType::IntShade || ShadeFlag == WinShadingType::ExtShade) {
                state.dataSurface->SurfWinShadingAbsorbedSolar(SurfNum) =
                    (state.dataSurface->SurfWinExtBeamAbsByShade(SurfNum) + state.dataSurface->SurfWinExtDiffAbsByShade(SurfNum)) *
                    (state.dataSurface->Surface(SurfNum).Area + state.dataSurface->SurfWinDividerArea(SurfNum));
                state.dataSurface->SurfWinShadingAbsorbedSolarEnergy(SurfNum) =
                    state.dataSurface->SurfWinShadingAbsorbedSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;
            }
            if (state.dataEnvrn->SunIsUp) {
                state.dataSurface->SurfWinSysSolTransmittance(SurfNum) =
                    state.dataSurface->SurfWinTransSolar(SurfNum) /
                    (state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) *
                         (state.dataSurface->Surface(SurfNum).Area + state.dataSurface->SurfWinDividerArea(SurfNum)) +
                     0.0001);
                state.dataSurface->SurfWinSysSolAbsorptance(SurfNum) =
                    (state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) + state.dataSurface->SurfWinShadingAbsorbedSolar(SurfNum)) /
                    (state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) *
                         (state.dataSurface->Surface(SurfNum).Area + state.dataSurface->SurfWinDividerArea(SurfNum)) +
                     0.0001);
                state.dataSurface->SurfWinSysSolReflectance(SurfNum) =
                    1.0 - state.dataSurface->SurfWinSysSolTransmittance(SurfNum) - state.dataSurface->SurfWinSysSolAbsorptance(SurfNum);
            } else {
                state.dataSurface->SurfWinSysSolTransmittance(SurfNum) = 0.0;
                state.dataSurface->SurfWinSysSolAbsorptance(SurfNum) = 0.0;
                state.dataSurface->SurfWinSysSolReflectance(SurfNum) = 0.0;
            }

            // Save hcv for use in divider calc with interior or exterior shade (see CalcWinFrameAndDividerTemps)
            if (ShadeFlag == WinShadingType::IntShade) {
                state.dataSurface->SurfWinConvCoeffWithShade(SurfNum) = 0.0;
            }

            if (ShadeFlag == WinShadingType::IntShade) {
                auto const &surfShade = state.dataSurface->surfShades(SurfNum);
                SurfInsideTemp = theta(2 * ngllayer + 2) - Constant::Kelvin;

                // // Get properties of inside shading layer

                Real64 EffShEmiss = surfShade.effShadeEmi;
                Real64 EffGlEmiss = surfShade.effGlassEmi;
                state.dataSurface->SurfWinEffInsSurfTemp(SurfNum) =
                    (EffShEmiss * SurfInsideTemp + EffGlEmiss * (theta(2 * ngllayer) - Constant::Kelvin)) / (EffShEmiss + EffGlEmiss);

            } else {
                SurfOutsideTemp = theta(1) - Constant::Kelvin;
            }

            for (k = 1; k <= nlayer; ++k) {
                state.dataSurface->SurfaceWindow(SurfNum).thetaFace[2 * k - 1] = theta(2 * k - 1);
                state.dataSurface->SurfaceWindow(SurfNum).thetaFace[2 * k] = theta(2 * k);

                // temperatures for reporting
                state.dataHeatBal->SurfWinFenLaySurfTempFront(SurfNum, k) = theta(2 * k - 1) - Constant::Kelvin;
                state.dataHeatBal->SurfWinFenLaySurfTempBack(SurfNum, k) = theta(2 * k) - Constant::Kelvin;
                // thetas(k) = theta(k)
            }
        }
    }

    // This function check if gas with molecular weight has already been feed into coefficients and
    // feed arrays

    void CheckGasCoefs(Real64 const currentWeight, int &indexNumber, Array1D<Real64> &wght, bool &feedData)
    {

        // Using/Aliasing
        using TARCOGGassesParams::maxgas;

        // Argument array dimensioning
        EP_SIZE_CHECK(wght, maxgas);

        feedData = false;
        bool coeffFound = false;
        int counter = 1;
        while ((counter <= maxgas) && (wght(counter) != 0) && (!coeffFound)) {
            if (std::abs(currentWeight - wght(counter)) < 1.0e-5) {
                coeffFound = true;
            } else {
                ++counter;
            }
        } // DO WHILE((counter.LE.maxgas).AND.(wght(couner).NE.0).AND.(.NOT.coeffFound))

        // In case coefficient is not found data needs to be stored in gas coefficients arrays
        if ((!coeffFound) && (counter < maxgas)) {
            feedData = true;
        }

        indexNumber = counter;
    }

    int SearchAscTable(Real64 const y,            // Value to be found in the table
                       int const n,               // Number of values in the table
                       Array1S<Real64> const ytab // Table of values, monotonic, ascending order
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Joe Klems
        //       DATE WRITTEN   Feb 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Given an ascending monotonic table with n entries, find  an index i
        // such that ytab(i-1) < y <= ytab(i)

        // METHODOLOGY EMPLOYED:
        // binary search

        // Return value
        int SearchAscTable;

        int Ih;    // Intex for upper end of interval
        int Il;    // Index for lower end of interval
        int Im;    // Index for midpoint of interval
        Real64 Yh; // Table value for upper end of interval
        Real64 Yl; // Table value for lower end of interval
        Real64 Ym; // Table value for midpoint of interval

        Yh = ytab(n);
        Yl = ytab(1);
        Ih = n;
        Il = 1;
        if (y < Yl) {
            SearchAscTable = 1;
            return SearchAscTable;
        } else if (y > Yh) {
            SearchAscTable = n;
            return SearchAscTable;
        }
        while (true) {
            if (Ih - Il <= 1) {
                break;
            }
            Im = (Ih + Il) / 2;
            Ym = ytab(Im);
            if (y <= Ym) {
                Yh = Ym;
                Ih = Im;
            } else {
                Yl = Ym;
                Il = Im;
            }
        }

        SearchAscTable = Ih;

        return SearchAscTable;
    }

} // namespace WindowComplexManager

} // namespace EnergyPlus
