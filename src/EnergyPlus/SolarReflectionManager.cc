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

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DataVectorTypes.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/PierceSurface.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SolarReflectionManager.hh>
#include <EnergyPlus/SolarShading.hh>

namespace EnergyPlus {

namespace SolarReflectionManager {

    // MODULE INFORMATION
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   September 2003
    //       MODIFIED       May 2004, FCW: modify calculation of receiving point location on a
    //                        receiving surface so can handle surface of any number of vertices
    //                        (previously restricted to 3- or 4-sided surfaces).
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // Manages the calculation of factors for solar reflected from obstructions and ground.

    // METHODOLOGY EMPLOYED:
    // REFERENCES: na

    // OTHER NOTES: na

    // Using/Aliasing
    using namespace DataHeatBalance;
    using namespace DataSurfaces;
    using namespace DataEnvironment;

    using namespace DataVectorTypes;

    void InitSolReflRecSurf(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   September 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Initializes the derived type SolReflRecSurf, which contains information
        // needed to calculate factors for solar reflection from obstructions and ground.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int SurfNum;            // Surface number
        int RecSurfNum;         // Receiving surface number
        int loop;               // DO loop indices
        int loop1;              // DO loop indices
        int loopA;              // DO loop indices
        int loopB;              // DO loop indices
        int ObsSurfNum;         // Surface number of an obstruction
        bool ObsBehindRec;      // True if an obstruction is entirely behind a receiving surface
        bool ObsHasView;        // True if view between receiving surface and heat trans surf obstruction
        Vector3<Real64> RecVec; // First vertex of a receiving surface (m)
        Vector3<Real64> ObsVec; // A vertex of a candidate obstructing surface (m)
        Vector3<Real64> VecAB;  // Vector from receiving surface vertex to obstruction surface vertex (m)
        Vector3<Real64> HitPt;  // Hit point (m)
        Real64 DotProd;         // Dot product of vectors (m2)
        int RecPtNum;           // Receiving point number
        // unused  REAL(r64)         :: SumX                 ! Sum of X (or Y or Z) coordinate values of a surface
        // unused  REAL(r64)         :: SumY                 ! Sum of X (or Y or Z) coordinate values of a surface
        // unused  REAL(r64)         :: SumZ                 ! Sum of X (or Y or Z) coordinate values of a surface
        Real64 PhiSurf;   // Altitude of normal to receiving surface (radians)
        Real64 ThetaSurf; // Azimuth of normal to receiving surface (radians)
        Real64 PhiMin;    // Minimum and maximum values of ray altitude angle (radians)
        Real64 PhiMax;    // Minimum and maximum values of ray altitude angle (radians)
        Real64 ThetaMin;  // Minimum and maximum values of ray azimuth angle (radians)
        Real64 ThetaMax;  // Minimum and maximum values of ray azimuth angle (radians)
        Real64 Phi;       // Ray altitude angle, increment, sine, and cosine
        Real64 DPhi;      // Ray altitude angle, increment, sine, and cosine
        Real64 SPhi;      // Ray altitude angle, increment, sine, and cosine
        Real64 CPhi;      // Ray altitude angle, increment, sine, and cosine
        Real64 Theta;     // Ray azimuth angle and increment
        Real64 DTheta;    // Ray azimuth angle and increment
        int IPhi;         // Ray altitude angle and azimuth angle indices
        int ITheta;       // Ray altitude angle and azimuth angle indices
        // unused  REAL(r64)         :: APhi                 ! Intermediate variable
        int RayNum;                   // Ray number
        Vector3<Real64> URay;         // Unit vector along ray pointing away from receiving surface
        Real64 CosIncAngRay;          // Cosine of angle of incidence of ray on receiving surface
        Real64 dOmega;                // Solid angle associated with a ray
        bool hit;                     // True iff obstruction is hit
        int TotObstructionsHit;       // Number of obstructions hit by a ray
        Real64 HitDistance;           // Distance from receiving point to hit point for a ray (m)
        int NearestHitSurfNum;        // Surface number of nearest obstruction hit by a ray
        Vector3<Real64> NearestHitPt; // Nearest hit pit for a ray (m)
        Real64 NearestHitDistance;    // Distance from receiving point to nearest hit point for a ray (m)
        int ObsSurfNumToSkip;         // Surface number of obstruction to be ignored
        Vector3<Real64> RecPt;        // Receiving point (m)
        Vector3<Real64> RayVec;       // Unit vector along ray
        Vector3<Real64> Vec1;         // Vectors between hit surface vertices (m)
        Vector3<Real64> Vec2;         // Vectors between hit surface vertices (m)
        Vector3<Real64> VNorm;        // For a hit surface, unit normal vector pointing into the hemisphere
        // containing the receiving point
        int ObsConstrNum; // Construction number of obstruction; = 0 if a shading surface
        Real64 Alfa;      // Direction angles for ray heading towards the ground (radians)
        Real64 Beta;
        Real64 HorDis;               // Distance between ground hit point and proj'n of receiving pt onto ground (m)
        Vector3<Real64> GroundHitPt; // Coordinates of ground hit point
        // unused  REAL(r64)         :: ArgASin
        Real64 ACosTanTan;
        int J;           // DO loop indices
        int K;           // DO loop indices
        int NumRecPts;   // Number of surface receiving points for reflected solar radiation
        Real64 VertexWt; // Vertex weighting factor for calculating receiving points

        static Vector3<Real64> const unit_z(0.0, 0.0, 1.0);
        static Vector3<Real64> const zero3(0.0);

        // Find number of surfaces that are sun-exposed exterior building heat transfer surfaces.
        // These are candidates for receiving solar reflected from obstructions and ground.
        // CR 7640.  12/3/2008 BG simplified logic to allow for Other Side Conditions Modeled boundary condition.
        //           and solar collectors on shading surfaces that need this.

        // shading surfaces have ExtSolar = False, so they are not included in TotSolReflRecSurf
        state.dataSolarReflectionManager->TotSolReflRecSurf = 0;
        for (SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            if (state.dataSurface->Surface(SurfNum).ExtSolar) {
                ++state.dataSolarReflectionManager->TotSolReflRecSurf;
            }
        }

        // TH 3/29/2010. ShadowSurfPossibleReflector is not used!
        // Set flag that determines whether a surface can be an exterior reflector
        // DO SurfNum = 1,TotSurfaces
        //  Surface(SurfNum)%ShadowSurfPossibleReflector = .FALSE.
        // Exclude non-exterior heat transfer surfaces (but not OtherSideCondModeledExt = -4 CR7640)
        //  IF(Surface(SurfNum)%HeatTransSurf .AND. Surface(SurfNum)%ExtBoundCond > 0 ) CYCLE
        //  IF(Surface(SurfNum)%HeatTransSurf .AND. Surface(SurfNum)%ExtBoundCond == Ground) CYCLE
        //  IF(Surface(SurfNum)%HeatTransSurf .AND. Surface(SurfNum)%ExtBoundCond == OtherSideCoefNoCalcExt) CYCLE
        //  IF(Surface(SurfNum)%HeatTransSurf .AND. Surface(SurfNum)%ExtBoundCond == OtherSideCoefCalcExt) CYCLE

        // Exclude daylighting shelves. A separate solar reflection calculation is done for these.
        //  IF(Surface(SurfNum)%Shelf > 0) CYCLE

        // Exclude duplicate shading surfaces
        // TH 3/24/2010. Why? a mirror shading surface can reflect solar (either beam or diffuse)
        //  can use a flag like Surface(SurfNum)%Mirrored (True or False) to avoid string comparison
        //   and to allow surface names starting with 'Mir'
        // IF(Surface(SurfNum)%Name(1:3) == 'Mir') CYCLE
        //  IF(Surface(SurfNum)%MirroredSurf) CYCLE

        //  Surface(SurfNum)%ShadowSurfPossibleReflector = .TRUE.
        // END DO

        if (state.dataSolarReflectionManager->TotSolReflRecSurf == 0) {
            ShowWarningError(state, "Calculation of solar reflected from obstructions has been requested but there");
            ShowContinueError(state, "are no building surfaces that can receive reflected solar. Calculation will not be done.");
            state.dataSurface->CalcSolRefl = false;
            return;
        }

        // Should this be moved up front?
        if (state.dataEnvrn->IgnoreSolarRadiation) {
            state.dataSolarReflectionManager->TotSolReflRecSurf = 0;
            state.dataSurface->CalcSolRefl = false;
            return;
        }

        state.dataSolarReflectionManager->SolReflRecSurf.allocate(state.dataSolarReflectionManager->TotSolReflRecSurf);

        state.dataSurface->SurfReflFacBmToDiffSolObs.dimension(24, state.dataSurface->TotSurfaces, 0.0);
        state.dataSurface->SurfReflFacBmToDiffSolGnd.dimension(24, state.dataSurface->TotSurfaces, 0.0);
        state.dataSurface->SurfReflFacBmToBmSolObs.dimension(24, state.dataSurface->TotSurfaces, 0.0);
        state.dataSurface->SurfReflFacSkySolObs.dimension(state.dataSurface->TotSurfaces, 0.0);
        state.dataSurface->SurfReflFacSkySolGnd.dimension(state.dataSurface->TotSurfaces, 0.0);
        state.dataSurface->SurfCosIncAveBmToBmSolObs.dimension(24, state.dataSurface->TotSurfaces, 0.0);

        // Only surfaces with sun exposure can receive solar reflection from ground or onstructions
        //  Shading surfaces are always not exposed to solar (ExtSolar = False)
        RecSurfNum = 0;
        for (SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            state.dataSurface->SurfShadowRecSurfNum(SurfNum) = 0;
            if (state.dataSurface->Surface(SurfNum).ExtSolar) {
                ++RecSurfNum;
                state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).SurfNum = SurfNum;
                state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).SurfName = state.dataSurface->Surface(SurfNum).Name;
                state.dataSurface->SurfShadowRecSurfNum(SurfNum) = RecSurfNum;

                // Warning if any receiving surface vertex is below ground level, taken to be at Z = 0 in absolute coords
                for (loop = 1; loop <= state.dataSurface->Surface(SurfNum).Sides; ++loop) {
                    if (state.dataSurface->Surface(SurfNum).Vertex(loop).z < state.dataSurface->GroundLevelZ) {
                        ShowWarningError(
                            state,
                            format("Calculation of reflected solar onto surface={} may be inaccurate", state.dataSurface->Surface(SurfNum).Name));
                        ShowContinueError(state, "because it has one or more vertices below ground level.");
                        break;
                    }
                }
            }
        }

        // Get MaxRecPts for allocating SolReflRecSurf arrays that depend on number of receiving points
        state.dataSurface->MaxRecPts = 1;
        for (RecSurfNum = 1; RecSurfNum <= state.dataSolarReflectionManager->TotSolReflRecSurf; ++RecSurfNum) {
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumRecPts =
                state.dataSurface->Surface(state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).SurfNum).Sides;
            if (state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumRecPts > state.dataSurface->MaxRecPts) {
                state.dataSurface->MaxRecPts = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumRecPts;
            }
        }

        state.dataSurface->MaxReflRays = AltAngStepsForSolReflCalc * AzimAngStepsForSolReflCalc;
        for (RecSurfNum = 1; RecSurfNum <= state.dataSolarReflectionManager->TotSolReflRecSurf; ++RecSurfNum) {
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NormVec = 0.0;
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).RecPt.dimension(state.dataSurface->MaxRecPts, zero3);
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).RayVec.dimension(state.dataSurface->MaxReflRays, zero3);
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).CosIncAngRay.dimension(state.dataSurface->MaxReflRays, 0.0);
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).dOmegaRay.dimension(state.dataSurface->MaxReflRays, 0.0);
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum)
                .HitPt.dimension(state.dataSurface->MaxReflRays, state.dataSurface->MaxRecPts, zero3);
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum)
                .HitPtSurfNum.dimension(state.dataSurface->MaxReflRays, state.dataSurface->MaxRecPts, 0.0);
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum)
                .HitPtSolRefl.dimension(state.dataSurface->MaxReflRays, state.dataSurface->MaxRecPts, 0.0);
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum)
                .RecPtHitPtDis.dimension(state.dataSurface->MaxReflRays, state.dataSurface->MaxRecPts, 0.0);
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum)
                .HitPtNormVec.dimension(state.dataSurface->MaxReflRays, state.dataSurface->MaxRecPts, zero3);
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).PossibleObsSurfNums.dimension(state.dataSurface->TotSurfaces, 0);
        }

        for (RecSurfNum = 1; RecSurfNum <= state.dataSolarReflectionManager->TotSolReflRecSurf; ++RecSurfNum) {
            SurfNum = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).SurfNum;
            // Outward norm to receiving surface
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NormVec = state.dataSurface->Surface(SurfNum).OutNormVec;
            RecVec = state.dataSurface->Surface(SurfNum).Vertex(1);
            // Loop over all surfaces and find those that can be obstructing surfaces for this receiving surf
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumPossibleObs = 0;
            for (ObsSurfNum = 1; ObsSurfNum <= state.dataSurface->TotSurfaces; ++ObsSurfNum) {
                // Exclude the receiving surface itself and its base surface (if it has one)
                if (ObsSurfNum == SurfNum || ObsSurfNum == state.dataSurface->Surface(SurfNum).BaseSurf) {
                    continue;
                }
                // Exclude non-exterior heat transfer surfaces
                if (state.dataSurface->Surface(ObsSurfNum).HeatTransSurf && state.dataSurface->Surface(ObsSurfNum).ExtBoundCond != 0) {
                    continue;
                }
                // Exclude duplicate shading surfaces
                // IF(Surface(ObsSurfNum)%Name(1:3) == 'Mir') CYCLE
                // TH2 CR8959
                // IF(Surface(ObsSurfNum)%MirroredSurf) CYCLE

                // Exclude surfaces that are entirely behind the receiving surface.This is true if dot products of the
                // rec. surface outward normal and vector from first vertex of rec. surface and each vertex of
                // obstructing surface are all negative.
                ObsBehindRec = true;
                for (loop = 1; loop <= state.dataSurface->Surface(ObsSurfNum).Sides; ++loop) {
                    ObsVec = state.dataSurface->Surface(ObsSurfNum).Vertex(loop);
                    DotProd = dot(state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NormVec, ObsVec - RecVec);
                    // CR8251      IF(DotProd > 0.01d0) THEN  ! This obstructing-surface vertex is not behind receiving surface
                    if (DotProd > Constant::OneMillionth) { // This obstructing-surface vertex is not behind receiving surface
                        ObsBehindRec = false;
                        break;
                    }
                }
                if (ObsBehindRec) {
                    continue;
                }

                // Exclude heat transfer surfaces that have no view with the receiving surface.
                // There is view if: for at least one vector VecAB from a receiving surface vertex to
                // a vertex of a potential obstructing surface that satisfies VecAB.nA > 0.0 and VecAB.nB < 0.0,
                // where nA and nB are the outward normal to the receiving and obstructing surface, resp.
                if (state.dataSurface->Surface(ObsSurfNum).HeatTransSurf) {
                    ObsHasView = false;
                    for (loopA = 1; loopA <= state.dataSurface->Surface(SurfNum).Sides; ++loopA) {
                        for (loopB = 1; loopB <= state.dataSurface->Surface(ObsSurfNum).Sides; ++loopB) {
                            VecAB = (state.dataSurface->Surface(ObsSurfNum).Vertex(loopB) - state.dataSurface->Surface(SurfNum).Vertex(loopA));
                            if (dot(VecAB, state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NormVec) > 0.0 &&
                                dot(VecAB, state.dataSurface->Surface(ObsSurfNum).OutNormVec) < 0.0) {
                                ObsHasView = true;
                                break;
                            }
                        }
                        if (ObsHasView) {
                            break;
                        }
                    }
                    if (!ObsHasView) {
                        continue;
                    }
                }

                // This is a possible obstructing surface for this receiving surface
                ++state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumPossibleObs;
                state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum)
                    .PossibleObsSurfNums(state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumPossibleObs) = ObsSurfNum;
            }

            // Get coordinates of receiving points on this receiving surface. The number of receiving points
            // is equal to the number of surface vertices (3 or higher).

            NumRecPts = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumRecPts;
            for (J = 1; J <= NumRecPts; ++J) {
                state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).RecPt(J) = 0.0;
                for (K = 1; K <= NumRecPts; ++K) {
                    if (NumRecPts == 3) { // Receiving surface is a triangle
                        VertexWt = 0.2;
                        if (K == J) {
                            VertexWt = 0.6;
                        }
                    } else { // Receiving surface has 4 or more vertices
                        VertexWt = 1.0 / (2.0 * NumRecPts);
                        if (K == J) {
                            VertexWt = (NumRecPts + 1.0) / (2.0 * NumRecPts);
                        }
                    }
                    state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).RecPt(J).x +=
                        VertexWt * state.dataSurface->Surface(SurfNum).Vertex(K).x;
                    state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).RecPt(J).y +=
                        VertexWt * state.dataSurface->Surface(SurfNum).Vertex(K).y;
                    state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).RecPt(J).z +=
                        VertexWt * state.dataSurface->Surface(SurfNum).Vertex(K).z;
                }
            }

            // Create rays going outward from receiving surface. The same rays will be used at each receiving point.
            // The rays are used in calculating diffusely reflected solar incident on receiving surface.

            // Divide hemisphere around receiving surface into elements of altitude Phi and
            // azimuth Theta and create ray unit vector at each Phi,Theta pair in front of the surface.
            // Phi = 0 at the horizon; Phi = Pi/2 at the zenith

            PhiSurf = std::asin(state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NormVec.z);
            Real64 const tan_PhiSurf = std::tan(PhiSurf);
            Real64 const sin_PhiSurf = std::sin(PhiSurf);
            Real64 const cos_PhiSurf = std::cos(PhiSurf);

            if (std::abs(state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NormVec.x) > 1.0e-5 ||
                std::abs(state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NormVec.y) > 1.0e-5) {
                ThetaSurf = std::atan2(state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NormVec.y,
                                       state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NormVec.x);
            } else {
                ThetaSurf = 0.0;
            }
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).PhiNormVec = PhiSurf;
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).ThetaNormVec = ThetaSurf;
            PhiMin = max(-Constant::PiOvr2, PhiSurf - Constant::PiOvr2);
            PhiMax = min(Constant::PiOvr2, PhiSurf + Constant::PiOvr2);
            DPhi = (PhiMax - PhiMin) / AltAngStepsForSolReflCalc;
            RayNum = 0;

            // Altitude loop
            for (IPhi = 1; IPhi <= AltAngStepsForSolReflCalc; ++IPhi) {
                Phi = PhiMin + (IPhi - 0.5) * DPhi;
                SPhi = std::sin(Phi);
                CPhi = std::cos(Phi);
                // Third component of ray unit vector in (Theta,Phi) direction
                URay(3) = SPhi;

                if (PhiSurf >= 0.0) {
                    if (Phi >= Constant::PiOvr2 - PhiSurf) {
                        ThetaMin = -Constant::Pi;
                        ThetaMax = Constant::Pi;
                    } else {
                        ACosTanTan = std::acos(-std::tan(Phi) * tan_PhiSurf);
                        ThetaMin = ThetaSurf - std::abs(ACosTanTan);
                        ThetaMax = ThetaSurf + std::abs(ACosTanTan);
                    }

                } else { // PhiSurf < 0.0
                    if (Phi <= -PhiSurf - Constant::PiOvr2) {
                        ThetaMin = -Constant::Pi;
                        ThetaMax = Constant::Pi;
                    } else {
                        ACosTanTan = std::acos(-std::tan(Phi) * tan_PhiSurf);
                        ThetaMin = ThetaSurf - std::abs(ACosTanTan);
                        ThetaMax = ThetaSurf + std::abs(ACosTanTan);
                    }
                }

                DTheta = (ThetaMax - ThetaMin) / AzimAngStepsForSolReflCalc;
                dOmega = CPhi * DTheta * DPhi;

                // Azimuth loop
                for (ITheta = 1; ITheta <= AzimAngStepsForSolReflCalc; ++ITheta) {
                    Theta = ThetaMin + (ITheta - 0.5) * DTheta;
                    URay.x = CPhi * std::cos(Theta);
                    URay.y = CPhi * std::sin(Theta);
                    // Cosine of angle of incidence of ray on receiving surface
                    CosIncAngRay = SPhi * sin_PhiSurf + CPhi * cos_PhiSurf * std::cos(Theta - ThetaSurf);
                    if (CosIncAngRay < 0.0) {
                        continue; // Ray is behind receiving surface (although there shouldn't be any)
                    }
                    ++RayNum;
                    state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).RayVec(RayNum) = URay;
                    state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).CosIncAngRay(RayNum) = CosIncAngRay;
                    state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).dOmegaRay(RayNum) = dOmega;
                } // End of azimuth loop

            } // End of altitude loop
            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumReflRays = RayNum;

        } // End of loop over receiving surfaces

        // Loop again over receiving surfaces and, for each ray, get hit point and info associated with that point
        // (hit point = point that ray intersects nearest obstruction, or, if ray is downgoing and hits no
        // obstructions, point that ray intersects ground plane).

        for (RecSurfNum = 1; RecSurfNum <= state.dataSolarReflectionManager->TotSolReflRecSurf; ++RecSurfNum) {
            SurfNum = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).SurfNum;
            for (RecPtNum = 1; RecPtNum <= state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumRecPts; ++RecPtNum) {
                RecPt = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).RecPt(RecPtNum);
                for (RayNum = 1; RayNum <= state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumReflRays; ++RayNum) {
                    // Loop over possible obstructions. If ray hits one or more obstructions get hit point on closest obstruction.
                    // If ray hits no obstructions and is going upward set HitPointSurfNum = 0.
                    // If ray hits no obstructions and is going downward set HitPointSurfNum = -1 and get hit point on ground.
                    TotObstructionsHit = 0;
                    NearestHitSurfNum = 0;
                    NearestHitDistance = 1.0e+8;
                    ObsSurfNumToSkip = 0;
                    RayVec = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).RayVec(RayNum);
                    for (loop1 = 1; loop1 <= state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumPossibleObs; ++loop1) {
                        // Surface number of this obstruction
                        ObsSurfNum = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).PossibleObsSurfNums(loop1);
                        // If a window was hit previously (see below), ObsSurfNumToSkip was set to the window's base surface in order
                        // to remove that surface from consideration as a hit surface for this ray
                        if (ObsSurfNum == ObsSurfNumToSkip) {
                            continue;
                        }
                        // Determine if this ray hits ObsSurfNum (in which case hit is true) and, if so, what the
                        // distance from the receiving point to the hit point is
                        hit = PierceSurface(state, ObsSurfNum, RecPt, RayVec, HitPt);
                        if (hit) {
                            // added TH 3/29/2010 to set ObsSurfNumToSkip
                            if (state.dataSurface->Surface(ObsSurfNum).Class == SurfaceClass::Window) {
                                ObsSurfNumToSkip = state.dataSurface->Surface(ObsSurfNum).BaseSurf;
                            }

                            // If obstruction is a window and its base surface is the nearest obstruction hit so far,
                            // set NearestHitSurfNum to this window. Note that in this case NearestHitDistance has already
                            // been calculated, so does not have to be recalculated.
                            if (state.dataSurface->Surface(ObsSurfNum).Class == SurfaceClass::Window &&
                                state.dataSurface->Surface(ObsSurfNum).BaseSurf == NearestHitSurfNum) {
                                NearestHitSurfNum = ObsSurfNum;
                            } else {
                                ++TotObstructionsHit;
                                // Distance from receiving point to hit point
                                HitDistance = distance(HitPt, RecPt);
                                // Reset NearestHitSurfNum and NearestHitDistance if this hit point is closer than previous closest
                                if (HitDistance < NearestHitDistance) {
                                    NearestHitDistance = HitDistance;
                                    NearestHitSurfNum = ObsSurfNum;
                                    NearestHitPt = HitPt;
                                } else if (HitDistance == NearestHitDistance) { // TH2 CR8959
                                    // Ray hits mirrored surfaces. Choose the surface facing the ray.
                                    if (dot(state.dataSurface->Surface(ObsSurfNum).OutNormVec, RayVec) <= 0.0) {
                                        NearestHitSurfNum = ObsSurfNum;
                                    }
                                }
                            }
                        } // End of check if obstruction was hit
                    } // End of loop over possible obstructions for this ray

                    if (TotObstructionsHit > 0) {
                        // One or more obstructions were hit by this ray
                        state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).HitPtSurfNum(RayNum, RecPtNum) = NearestHitSurfNum;
                        state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).RecPtHitPtDis(RayNum, RecPtNum) = NearestHitDistance;
                        state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).HitPt(RayNum, RecPtNum) = NearestHitPt;
                        // For hit surface, calculate unit normal vector pointing into the hemisphere
                        // containing the receiving point
                        Vec1 = (state.dataSurface->Surface(NearestHitSurfNum).Vertex(1) - state.dataSurface->Surface(NearestHitSurfNum).Vertex(3));
                        Vec2 = (state.dataSurface->Surface(NearestHitSurfNum).Vertex(2) - state.dataSurface->Surface(NearestHitSurfNum).Vertex(3));
                        VNorm = cross(Vec1, Vec2);
                        VNorm.normalize(); // Do Handle magnitude==0
                        if (dot(VNorm, -RayVec) < 0.0) {
                            VNorm = -VNorm;
                        }
                        state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).HitPtNormVec(RayNum, RecPtNum) = VNorm;
                        // Get solar and visible beam-to-diffuse reflectance at nearest hit point
                        ObsConstrNum = state.dataSurface->Surface(NearestHitSurfNum).Construction;
                        if (ObsConstrNum > 0) {
                            // Exterior building surface is nearest hit
                            if (!state.dataConstruction->Construct(ObsConstrNum).TypeIsWindow) {
                                // Obstruction is not a window, i.e., is an opaque surface
                                state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).HitPtSolRefl(RayNum, RecPtNum) =
                                    1.0 - state.dataConstruction->Construct(ObsConstrNum).OutsideAbsorpSolar;
                            } else {
                                // Obstruction is a window. Assume it is bare so that there is no beam-to-diffuse reflection
                                // (beam-to-beam reflection is calculated in subroutine CalcBeamSolSpecularReflFactors).
                                state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).HitPtSolRefl(RayNum, RecPtNum) = 0.0;
                            }
                        } else {
                            // Shading surface is nearest hit
                            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).HitPtSolRefl(RayNum, RecPtNum) =
                                state.dataSurface->SurfShadowDiffuseSolRefl(NearestHitSurfNum);
                        }
                    } else {
                        // No obstructions were hit by this ray
                        state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).HitPtSurfNum(RayNum, RecPtNum) = 0;
                        // If ray is going downward find the hit point on the ground plane if the receiving point
                        // is above ground level; note that GroundLevelZ is <= 0.0
                        if (RayVec(3) < 0.0 &&
                            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).RecPt(RecPtNum).z > state.dataSurface->GroundLevelZ) {
                            // Ray hits ground
                            Alfa = std::acos(-RayVec.z);
                            Beta = std::atan2(RayVec.y, RayVec.x);
                            HorDis = (RecPt.z - state.dataSurface->GroundLevelZ) * std::tan(Alfa);
                            GroundHitPt.z = state.dataSurface->GroundLevelZ;
                            GroundHitPt.x = RecPt.x + HorDis * std::cos(Beta);
                            GroundHitPt.y = RecPt.y + HorDis * std::sin(Beta);
                            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).HitPt(RayNum, RecPtNum) = GroundHitPt;
                            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).HitPtSurfNum(RayNum, RecPtNum) = -1;
                            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).RecPtHitPtDis(RayNum, RecPtNum) =
                                (RecPt(3) - state.dataSurface->GroundLevelZ) / (-RayVec(3));
                            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).HitPtSolRefl(RayNum, RecPtNum) =
                                state.dataEnvrn->GndReflectance;
                            state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).HitPtNormVec(RayNum, RecPtNum) = unit_z;
                        } // End of check if ray hits ground
                    } // End of check if obstruction hit
                } // End of RayNum loop
            } // End of receiving point loop
        } // End of receiving surface loop
    }

    //=====================================================================================================

    void CalcBeamSolDiffuseReflFactors(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   September 2003
        //       MODIFIED       TH 4/6/2010, fixed CR 7872
        //       RE-ENGINEERED  B. Griffith, October 2012, for timestep integrated solar.

        // PURPOSE OF THIS SUBROUTINE:
        // manage calculations for factors for irradiance on exterior heat transfer surfaces due to
        // beam-to-diffuse solar reflection from obstructions and ground.

        // METHODOLOGY EMPLOYED: call worker routine depending on solar calculation method

        if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
            if (state.dataGlobal->BeginSimFlag) {
                DisplayString(state, "Calculating Beam-to-Diffuse Exterior Solar Reflection Factors");
            } else {
                DisplayString(state, "Updating Beam-to-Diffuse Exterior Solar Reflection Factors");
            }
            state.dataSurface->SurfReflFacBmToDiffSolObs = 0.0;
            state.dataSurface->SurfReflFacBmToDiffSolGnd = 0.0;
            for (state.dataSolarReflectionManager->IHr = 1; state.dataSolarReflectionManager->IHr <= 24; ++state.dataSolarReflectionManager->IHr) {
                FigureBeamSolDiffuseReflFactors(state, state.dataSolarReflectionManager->IHr);
            } // End of IHr loop
        } else { // timestep integrated solar, use current hour of day
            state.dataSurface->SurfReflFacBmToDiffSolObs(state.dataGlobal->HourOfDay, {1, state.dataSurface->TotSurfaces}) = 0.0;
            state.dataSurface->SurfReflFacBmToDiffSolGnd(state.dataGlobal->HourOfDay, {1, state.dataSurface->TotSurfaces}) = 0.0;
            FigureBeamSolDiffuseReflFactors(state, state.dataGlobal->HourOfDay);
        }
    }

    void FigureBeamSolDiffuseReflFactors(EnergyPlusData &state, int const iHour)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann, derived from original CalcBeamSolDiffuseReflFactors
        //       DATE WRITTEN   September 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  B. Griffith, October 2012, revised for timestep integrated solar

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates factors for irradiance on exterior heat transfer surfaces due to
        // beam-to-diffuse solar reflection from obstructions and ground.

        Array1D<Real64> ReflBmToDiffSolObs(state.dataSurface->MaxRecPts); // Irradiance at a receiving point for
        // beam solar diffusely reflected from obstructions, divided by
        // beam normal irradiance
        Array1D<Real64> ReflBmToDiffSolGnd(state.dataSurface->MaxRecPts); // Irradiance at a receiving point for
        // beam solar diffusely reflected from the ground, divided by
        // beam normal irradiance
        bool hit; // True iff obstruction is hit
        ReflBmToDiffSolObs = 0.0;
        ReflBmToDiffSolGnd = 0.0;

        // Unit vector to sun
        state.dataSolarReflectionManager->SunVec = state.dataSurface->SurfSunCosHourly(iHour);

        // loop through each surface that can receive beam solar reflected as diffuse solar from other surfaces
        for (state.dataSolarReflectionManager->RecSurfNum = 1;
             state.dataSolarReflectionManager->RecSurfNum <= state.dataSolarReflectionManager->TotSolReflRecSurf;
             ++state.dataSolarReflectionManager->RecSurfNum) {
            state.dataSolarReflectionManager->SurfNum =
                state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->RecSurfNum).SurfNum;

            for (state.dataSolarReflectionManager->RecPtNum = 1;
                 state.dataSolarReflectionManager->RecPtNum <=
                 state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->RecSurfNum).NumRecPts;
                 ++state.dataSolarReflectionManager->RecPtNum) {
                ReflBmToDiffSolObs(state.dataSolarReflectionManager->RecPtNum) = 0.0;
                ReflBmToDiffSolGnd(state.dataSolarReflectionManager->RecPtNum) = 0.0;

                for (state.dataSolarReflectionManager->RayNum = 1;
                     state.dataSolarReflectionManager->RayNum <=
                     state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->RecSurfNum).NumReflRays;
                     ++state.dataSolarReflectionManager->RayNum) {
                    state.dataSolarReflectionManager->HitPtSurfNum =
                        state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->RecSurfNum)
                            .HitPtSurfNum(state.dataSolarReflectionManager->RayNum, state.dataSolarReflectionManager->RecPtNum);

                    // Skip rays that do not hit an obstruction or ground.
                    // (Note that if a downgoing ray does not hit an obstruction it will have HitPtSurfNum = 0
                    // if the receiving point is below ground level (see subr. InitSolReflRecSurf); this means
                    // that a below-ground-level receiving point receives no ground-reflected radiation although
                    // it is allowed to receive obstruction-reflected solar radiation and direct (unreflected)
                    // beam and sky solar radiation. As far as reflected solar is concerned, the program does
                    // not handle a sloped ground plane or a horizontal ground plane whose level is different
                    // from one side of the building to another.)
                    if (state.dataSolarReflectionManager->HitPtSurfNum == 0) {
                        continue; // Ray hits sky or obstruction with receiving pt. below ground level
                    }

                    if (state.dataSolarReflectionManager->HitPtSurfNum > 0) {
                        // Skip rays that hit a daylighting shelf, from which solar reflection is calculated separately.
                        if (state.dataSurface->SurfDaylightingShelfInd(state.dataSolarReflectionManager->HitPtSurfNum) > 0) {
                            continue;
                        }

                        // Skip rays that hit a window
                        // If hit point's surface is a window or glass door go to next ray since it is assumed for now
                        // that windows have only beam-to-beam, not beam-to-diffuse, reflection
                        // TH 3/29/2010. Code modified and moved
                        if (state.dataSurface->Surface(state.dataSolarReflectionManager->HitPtSurfNum).Class == SurfaceClass::Window ||
                            state.dataSurface->Surface(state.dataSolarReflectionManager->HitPtSurfNum).Class == SurfaceClass::GlassDoor) {
                            continue;
                        }

                        // Skip rays that hit non-sunlit surface. Assume first time step of the hour.
                        state.dataSolarReflectionManager->SunLitFract =
                            state.dataHeatBal->SurfSunlitFrac(iHour, 1, state.dataSolarReflectionManager->HitPtSurfNum);

                        // If hit point's surface is not sunlit go to next ray
                        // TH 3/25/2010. why limit to HeatTransSurf? shading surfaces should also apply
                        // IF(Surface(HitPtSurfNum)%HeatTransSurf .AND. SunLitFract < 0.01d0) CYCLE
                        if (state.dataSolarReflectionManager->SunLitFract < 0.01) {
                            continue;
                        }

                        // TH 3/26/2010. If the hit point falls into the shadow even though SunLitFract > 0, can Cycle.
                        //  This cannot be done now, therefore there are follow-up checks of blocking sun ray
                        //   from the hit point.

                        // TH 3/29/2010. Code modified and moved up
                        // If hit point's surface is a window go to next ray since it is assumed for now
                        // that windows have only beam-to-beam, not beam-to-diffuse, reflection
                        // IF(Surface(HitPtSurfNum)%Construction > 0) THEN
                        //  IF(Construct(Surface(HitPtSurfNum)%Construction)%TypeIsWindow) CYCLE
                        // END IF
                    }

                    // Does an obstruction block the vector from this ray's hit point to the sun?
                    state.dataSolarReflectionManager->OriginThisRay =
                        state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->RecSurfNum)
                            .HitPt(state.dataSolarReflectionManager->RayNum, state.dataSolarReflectionManager->RecPtNum);

                    // Note: if sun is in back of hit surface relative to receiving point, CosIncBmAtHitPt will be < 0
                    state.dataSolarReflectionManager->CosIncBmAtHitPt =
                        dot(state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->RecSurfNum)
                                .HitPtNormVec(state.dataSolarReflectionManager->RayNum, state.dataSolarReflectionManager->RecPtNum),
                            state.dataSolarReflectionManager->SunVec);
                    if (state.dataSolarReflectionManager->CosIncBmAtHitPt <= 0.0) {
                        continue;
                    }

                    // CR 7872 - TH 4/6/2010. The shading surfaces should point to the receiveing heat transfer surface
                    //  according to the the right hand rule. If user inputs do not follow the rule, use the following
                    //  code to check the mirrored shading surface
                    if (state.dataSolarReflectionManager->HitPtSurfNum > 0) {
                        if (state.dataSurface->Surface(state.dataSolarReflectionManager->HitPtSurfNum).IsShadowing) {
                            if (state.dataSolarReflectionManager->HitPtSurfNum + 1 < state.dataSurface->TotSurfaces) {
                                if (state.dataSurface->Surface(state.dataSolarReflectionManager->HitPtSurfNum + 1).IsShadowing &&
                                    state.dataSurface->Surface(state.dataSolarReflectionManager->HitPtSurfNum + 1).MirroredSurf) {
                                    // Check whether the sun is behind the mirrored shading surface
                                    state.dataSolarReflectionManager->CosIncBmAtHitPt2 =
                                        dot(state.dataSurface->Surface(state.dataSolarReflectionManager->HitPtSurfNum + 1).OutNormVec,
                                            state.dataSolarReflectionManager->SunVec);
                                    if (state.dataSolarReflectionManager->CosIncBmAtHitPt2 >= 0.0) {
                                        continue;
                                    }
                                }
                            }
                        }
                    }

                    // TH 3/25/2010. CR 7872. Seems should loop over all possible obstructions for the HitPtSurfNum
                    //  rather than RecSurfNum, because if the HitPtSurfNum is a shading surface,
                    //  it does not belong to SolReflRecSurf which only contain heat transfer surfaces
                    //  that can receive reflected solar (ExtSolar = True)!

                    // To speed up, ideally should store all possible shading surfaces for the HitPtSurfNum
                    //  obstruction surface in the SolReflSurf(HitPtSurfNum)%PossibleObsSurfNums(loop) array as well
                    hit = false;
                    for (state.dataSolarReflectionManager->ObsSurfNum = 1;
                         state.dataSolarReflectionManager->ObsSurfNum <= state.dataSurface->TotSurfaces;
                         ++state.dataSolarReflectionManager->ObsSurfNum) {
                        //        DO loop = 1,SolReflRecSurf(RecSurfNum)%NumPossibleObs
                        //          ObsSurfNum = SolReflRecSurf(RecSurfNum)%PossibleObsSurfNums(loop)

                        // CR 8959 -- The other side of a mirrored surface cannot obstruct the mirrored surface
                        if (state.dataSolarReflectionManager->HitPtSurfNum > 0) {
                            if (state.dataSurface->Surface(state.dataSolarReflectionManager->HitPtSurfNum).MirroredSurf) {
                                if (state.dataSolarReflectionManager->ObsSurfNum == state.dataSolarReflectionManager->HitPtSurfNum - 1) {
                                    continue;
                                }
                            }
                        }

                        // skip the hit surface
                        if (state.dataSolarReflectionManager->ObsSurfNum == state.dataSolarReflectionManager->HitPtSurfNum) {
                            continue;
                        }

                        // skip mirrored surfaces
                        if (state.dataSurface->Surface(state.dataSolarReflectionManager->ObsSurfNum).MirroredSurf) {
                            continue;
                        }
                        // IF(Surface(ObsSurfNum)%ShadowingSurf .AND. Surface(ObsSurfNum)%Name(1:3) == 'Mir') THEN
                        //  CYCLE
                        // ENDIF

                        // skip interior surfaces
                        if (state.dataSurface->Surface(state.dataSolarReflectionManager->ObsSurfNum).ExtBoundCond >= 1) {
                            continue;
                        }

                        // For now it is assumed that obstructions that are shading surfaces are opaque.
                        // An improvement here would be to allow these to have transmittance.
                        hit = PierceSurface(state,
                                            state.dataSolarReflectionManager->ObsSurfNum,
                                            state.dataSolarReflectionManager->OriginThisRay,
                                            state.dataSolarReflectionManager->SunVec,
                                            state.dataSolarReflectionManager->ObsHitPt);
                        if (hit) {
                            break; // An obstruction was hit
                        }
                    }
                    if (hit) {
                        continue; // Sun does not reach this ray's hit point
                    }

                    // Sun reaches this ray's hit point; get beam-reflected diffuse radiance at hit point for
                    // unit beam normal solar

                    // CosIncBmAtHitPt = DOT_PRODUCT(SolReflRecSurf(RecSurfNum)%HitPtNormVec(RecPtNum,RayNum),SunVec)
                    // Note: if sun is in back of hit surface relative to receiving point, CosIncBmAtHitPt will be < 0
                    // and use of MAX in following gives zero beam solar reflecting at hit point.
                    // BmReflSolRadiance = MAX(0.0d0,CosIncBmAtHitPt)*SolReflRecSurf(RecSurfNum)%HitPtSolRefl(RecPtNum,RayNum)

                    state.dataSolarReflectionManager->BmReflSolRadiance =
                        state.dataSolarReflectionManager->CosIncBmAtHitPt *
                        state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->RecSurfNum)
                            .HitPtSolRefl(state.dataSolarReflectionManager->RayNum, state.dataSolarReflectionManager->RecPtNum);

                    if (state.dataSolarReflectionManager->BmReflSolRadiance > 0.0) {
                        // Contribution to reflection factor from this hit point
                        if (state.dataSolarReflectionManager->HitPtSurfNum > 0) {
                            // Ray hits an obstruction
                            state.dataSolarReflectionManager->dReflBeamToDiffSol =
                                state.dataSolarReflectionManager->BmReflSolRadiance *
                                state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->RecSurfNum)
                                    .dOmegaRay(state.dataSolarReflectionManager->RayNum) *
                                state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->RecSurfNum)
                                    .CosIncAngRay(state.dataSolarReflectionManager->RayNum) /
                                Constant::Pi;
                            ReflBmToDiffSolObs(state.dataSolarReflectionManager->RecPtNum) += state.dataSolarReflectionManager->dReflBeamToDiffSol;
                        } else {
                            // Ray hits ground (in this case we do not multiply by BmReflSolRadiance since
                            // ground reflectance and cos of incidence angle of sun on
                            // ground is taken into account later when SurfReflFacBmToDiffSolGnd is used)
                            state.dataSolarReflectionManager->dReflBeamToDiffSol =
                                state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->RecSurfNum)
                                    .dOmegaRay(state.dataSolarReflectionManager->RayNum) *
                                state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->RecSurfNum)
                                    .CosIncAngRay(state.dataSolarReflectionManager->RayNum) /
                                Constant::Pi;
                            ReflBmToDiffSolGnd(state.dataSolarReflectionManager->RecPtNum) += state.dataSolarReflectionManager->dReflBeamToDiffSol;
                        }
                    }
                } // End of loop over rays from receiving point
            } // End of loop over receiving points

            // Average over receiving points
            state.dataSurface->SurfReflFacBmToDiffSolObs(iHour, state.dataSolarReflectionManager->SurfNum) = 0.0;
            state.dataSurface->SurfReflFacBmToDiffSolGnd(iHour, state.dataSolarReflectionManager->SurfNum) = 0.0;
            state.dataSolarReflectionManager->NumRecPts =
                state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->RecSurfNum).NumRecPts;
            for (state.dataSolarReflectionManager->RecPtNum = 1;
                 state.dataSolarReflectionManager->RecPtNum <= state.dataSolarReflectionManager->NumRecPts;
                 ++state.dataSolarReflectionManager->RecPtNum) {
                state.dataSurface->SurfReflFacBmToDiffSolObs(iHour, state.dataSolarReflectionManager->SurfNum) +=
                    ReflBmToDiffSolObs(state.dataSolarReflectionManager->RecPtNum);
                state.dataSurface->SurfReflFacBmToDiffSolGnd(iHour, state.dataSolarReflectionManager->SurfNum) +=
                    ReflBmToDiffSolGnd(state.dataSolarReflectionManager->RecPtNum);
            }
            state.dataSurface->SurfReflFacBmToDiffSolObs(iHour, state.dataSolarReflectionManager->SurfNum) /=
                state.dataSolarReflectionManager->NumRecPts;
            state.dataSurface->SurfReflFacBmToDiffSolGnd(iHour, state.dataSolarReflectionManager->SurfNum) /=
                state.dataSolarReflectionManager->NumRecPts;

            // Do not allow SurfReflFacBmToDiffSolGnd to exceed the surface's unobstructed ground view factor
            state.dataSurface->SurfReflFacBmToDiffSolGnd(iHour, state.dataSolarReflectionManager->SurfNum) =
                min(0.5 * (1.0 - state.dataSurface->Surface(state.dataSolarReflectionManager->SurfNum).CosTilt),
                    state.dataSurface->SurfReflFacBmToDiffSolGnd(iHour, state.dataSolarReflectionManager->SurfNum));
            // Note: the above factors are dimensionless; they are equal to
            // (W/m2 reflected solar incident on SurfNum)/(W/m2 beam normal solar)
        } // End of loop over receiving surfaces
    }

    //=================================================================================================

    void CalcBeamSolSpecularReflFactors(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   September 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  B. Griffith, October 2012, for timestep integrated solar

        // PURPOSE OF THIS SUBROUTINE:
        // Manage calculation of factors for beam solar irradiance on exterior heat transfer surfaces due to
        // specular (beam-to-beam) reflection from obstructions such as a highly-glazed neighboring
        // building.

        // METHODOLOGY EMPLOYED:
        // call worker routine as appropriate

        if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
            if (state.dataGlobal->BeginSimFlag) {
                DisplayString(state, "Calculating Beam-to-Beam Exterior Solar Reflection Factors");
            } else {
                DisplayString(state, "Updating Beam-to-Beam Exterior Solar Reflection Factors");
            }
            state.dataSurface->SurfReflFacBmToBmSolObs = 0.0;
            state.dataSurface->SurfCosIncAveBmToBmSolObs = 0.0;
            for (state.dataSolarReflectionManager->NumHr = 1; state.dataSolarReflectionManager->NumHr <= 24;
                 ++state.dataSolarReflectionManager->NumHr) {
                FigureBeamSolSpecularReflFactors(state, state.dataSolarReflectionManager->NumHr);
            } // End of NumHr loop
        } else { // timestep integrated solar, use current hour of day
            state.dataSurface->SurfReflFacBmToBmSolObs(state.dataGlobal->HourOfDay, {1, state.dataSurface->TotSurfaces}) = 0.0;
            state.dataSurface->SurfCosIncAveBmToBmSolObs(state.dataGlobal->HourOfDay, {1, state.dataSurface->TotSurfaces}) = 0.0;
            FigureBeamSolSpecularReflFactors(state, state.dataGlobal->HourOfDay);
        }
    }

    void FigureBeamSolSpecularReflFactors(EnergyPlusData &state, int const iHour)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   September 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  B. Griffith, October 2012, for timestep integrated solar

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates factors for beam solar irradiance on exterior heat transfer surfaces due to
        // specular (beam-to-beam) reflection from obstructions such as a highly-glazed neighboring
        // building. Specular reflection can occur from shading surfaces with non-zero specular
        // reflectance and from exterior windows of the building (in calculating reflection from
        // these windows, they are assumed to have no shades or blinds).
        // Reflection from the ground and opaque building surfaces is assumed to be totally diffuse,
        // i.e. these surfaces has no specular reflection component.

        // METHODOLOGY EMPLOYED:
        // <description>

        // REFERENCES:
        // na

        // Locals
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D<Real64> ReflBmToDiffSolObs(state.dataSurface->MaxRecPts); // Irradiance at a receiving point for
        // beam solar diffusely reflected from obstructions, divided by
        // beam normal irradiance
        // unused  INTEGER           :: RayNum               =0   ! Ray number
        bool hitRefl;                                                   // True iff reflecting surface is hit
        bool hitObs;                                                    // True iff obstruction is hit
        bool hitObsRefl;                                                // True iff obstruction hit between rec. pt. and reflection point
        Array1D<Real64> ReflBmToBmSolObs(state.dataSurface->MaxRecPts); // Irradiance at a receiving point for
        // beam solar specularly reflected from obstructions, divided by
        // beam normal irradiance
        Real64 ReflDistanceSq; // Distance squared from receiving point to hit point on a reflecting surface (m)
        Real64 ReflDistance;   // Distance from receiving point to hit point on a reflecting surface (m)
        Array1D<Real64> ReflFacTimesCosIncSum(state.dataSurface->MaxRecPts); // Sum of ReflFac times CosIncAngRefl

        ReflBmToDiffSolObs = 0.0;
        ReflFacTimesCosIncSum = 0.0;

        if (state.dataSurface->SurfSunCosHourly(iHour)(3) < DataEnvironment::SunIsUpValue) {
            return; // Skip if sun is below horizon
        }

        // Unit vector to sun
        state.dataSolarReflectionManager->SunVect = state.dataSurface->SurfSunCosHourly(iHour);

        for (int RecSurfNum = 1; RecSurfNum <= state.dataSolarReflectionManager->TotSolReflRecSurf; ++RecSurfNum) {
            int const SurfNum =
                state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).SurfNum; // Heat transfer surface number corresponding to RecSurfNum
            if (state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumPossibleObs > 0) {
                ReflBmToBmSolObs = 0.0;
                ReflFacTimesCosIncSum = 0.0;
                int const NumRecPts = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumRecPts;
                // Find possible reflecting surfaces for this receiving surface
                for (int loop = 1, loop_end = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumPossibleObs; loop <= loop_end; ++loop) {
                    int const ReflSurfNum =
                        state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).PossibleObsSurfNums(loop); // Reflecting surface number
                    // Keep windows; keep shading surfaces with specular reflectance
                    if ((state.dataSurface->Surface(ReflSurfNum).Class == SurfaceClass::Window && state.dataSurface->Surface(ReflSurfNum).ExtSolar) ||
                        (state.dataSurface->SurfShadowGlazingFrac(ReflSurfNum) > 0.0 && state.dataSurface->Surface(ReflSurfNum).IsShadowing)) {
                        // Skip if window and not sunlit
                        if (state.dataSurface->Surface(ReflSurfNum).Class == SurfaceClass::Window &&
                            state.dataHeatBal->SurfSunlitFrac(iHour, 1, ReflSurfNum) < 0.01) {
                            continue;
                        }
                        // Check if sun is in front of this reflecting surface.
                        state.dataSolarReflectionManager->ReflNorm = state.dataSurface->Surface(ReflSurfNum).OutNormVec;
                        state.dataSolarReflectionManager->CosIncAngRefl =
                            dot(state.dataSolarReflectionManager->SunVect, state.dataSolarReflectionManager->ReflNorm);
                        if (state.dataSolarReflectionManager->CosIncAngRefl < 0.0) {
                            continue;
                        }

                        // Get sun position unit vector for mirror image of sun in reflecting surface
                        state.dataSolarReflectionManager->SunVecMir =
                            state.dataSolarReflectionManager->SunVect -
                            2.0 * dot(state.dataSolarReflectionManager->SunVect, state.dataSolarReflectionManager->ReflNorm) *
                                state.dataSolarReflectionManager->ReflNorm;
                        // Angle of incidence of reflected beam on receiving surface
                        state.dataSolarReflectionManager->CosIncAngRec =
                            dot(state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NormVec, state.dataSolarReflectionManager->SunVecMir);
                        if (state.dataSolarReflectionManager->CosIncAngRec <= 0.0) {
                            continue;
                        }
                        for (int RecPtNum = 1; RecPtNum <= NumRecPts; ++RecPtNum) {
                            // See if ray from receiving point to mirrored sun hits the reflecting surface
                            state.dataSolarReflectionManager->RecPt = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).RecPt(RecPtNum);
                            hitRefl = PierceSurface(state,
                                                    ReflSurfNum,
                                                    state.dataSolarReflectionManager->RecPt,
                                                    state.dataSolarReflectionManager->SunVecMir,
                                                    state.dataSolarReflectionManager->HitPtRefl);
                            if (hitRefl) { // Reflecting surface was hit
                                ReflDistanceSq =
                                    distance_squared(state.dataSolarReflectionManager->HitPtRefl, state.dataSolarReflectionManager->RecPt);
                                ReflDistance = std::sqrt(ReflDistanceSq);
                                // Determine if ray from receiving point to hit point is obstructed
                                hitObsRefl = false;
                                for (int loop2 = 1, loop2_end = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumPossibleObs;
                                     loop2 <= loop2_end;
                                     ++loop2) {
                                    int const ObsSurfNum = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).PossibleObsSurfNums(loop2);
                                    if (ObsSurfNum == ReflSurfNum || ObsSurfNum == state.dataSurface->Surface(ReflSurfNum).BaseSurf) {
                                        continue;
                                    }
                                    hitObs = PierceSurface(state,
                                                           ObsSurfNum,
                                                           state.dataSolarReflectionManager->RecPt,
                                                           state.dataSolarReflectionManager->SunVecMir,
                                                           ReflDistance,
                                                           state.dataSolarReflectionManager->HitPtObs); // ReflDistance cutoff added
                                    if (hitObs) { // => Could skip distance check (unless < vs <= ReflDistance really matters)
                                        if (distance_squared(state.dataSolarReflectionManager->HitPtObs, state.dataSolarReflectionManager->RecPt) <
                                            ReflDistanceSq) {
                                            hitObsRefl = true;
                                            break;
                                        }
                                    }
                                }
                                if (hitObsRefl) {
                                    continue; // Obstruction closer than reflection pt. was hit; go to next rec. pt.
                                }
                                // There is no obstruction for this ray between rec. pt. and hit point on reflecting surface.
                                // See if ray from hit pt. on reflecting surface to original (unmirrored) sun position is obstructed
                                hitObs = false;
                                if (state.dataSurface->Surface(ReflSurfNum).Class == SurfaceClass::Window) { // Reflecting surface is a window
                                    // Receiving surface number for this window
                                    int const ReflSurfRecNum = state.dataSurface->SurfShadowRecSurfNum(
                                        ReflSurfNum); // Receiving surface number corresponding to a reflecting surface number
                                    if (ReflSurfRecNum > 0) {
                                        // Loop over possible obstructions for this window
                                        for (int loop2 = 1,
                                                 loop2_end = state.dataSolarReflectionManager->SolReflRecSurf(ReflSurfRecNum).NumPossibleObs;
                                             loop2 <= loop2_end;
                                             ++loop2) {
                                            int const ObsSurfNum =
                                                state.dataSolarReflectionManager->SolReflRecSurf(ReflSurfRecNum).PossibleObsSurfNums(loop2);
                                            hitObs = PierceSurface(state,
                                                                   ObsSurfNum,
                                                                   state.dataSolarReflectionManager->HitPtRefl,
                                                                   state.dataSolarReflectionManager->SunVect,
                                                                   state.dataSolarReflectionManager->HitPtObs);
                                            if (hitObs) {
                                                break;
                                            }
                                        }
                                    }
                                } else { // Reflecting surface is a building shade
                                    for (int ObsSurfNum : state.dataSurface->AllShadowPossObstrSurfaceList) {
                                        if (ObsSurfNum == ReflSurfNum) {
                                            continue;
                                        }

                                        // TH2 CR8959 -- Skip mirrored surfaces
                                        if (state.dataSurface->Surface(ObsSurfNum).MirroredSurf) {
                                            continue;
                                        }
                                        // TH2 CR8959 -- The other side of a mirrored surface cannot obstruct the mirrored surface
                                        if (state.dataSurface->Surface(ReflSurfNum).MirroredSurf) {
                                            if (ObsSurfNum == ReflSurfNum - 1) {
                                                continue;
                                            }
                                        }

                                        hitObs = PierceSurface(state,
                                                               ObsSurfNum,
                                                               state.dataSolarReflectionManager->HitPtRefl,
                                                               state.dataSolarReflectionManager->SunVect,
                                                               state.dataSolarReflectionManager->HitPtObs);
                                        if (hitObs) {
                                            break;
                                        }
                                    }
                                }

                                if (hitObs) {
                                    continue; // Obstruction hit between reflection hit point and sun; go to next receiving pt.
                                }

                                // No obstructions. Calculate reflected beam irradiance at receiving pt. from this reflecting surface.
                                state.dataSolarReflectionManager->SpecReflectance = 0.0;
                                if (state.dataSurface->Surface(ReflSurfNum).Class == SurfaceClass::Window) {
                                    state.dataSolarReflectionManager->ConstrNumRefl = state.dataSurface->Surface(ReflSurfNum).Construction;
                                    state.dataSolarReflectionManager->SpecReflectance = Window::POLYF(
                                        std::abs(state.dataSolarReflectionManager->CosIncAngRefl),
                                        state.dataConstruction->Construct(state.dataSolarReflectionManager->ConstrNumRefl).ReflSolBeamFrontCoef);
                                }
                                if (state.dataSurface->Surface(ReflSurfNum).IsShadowing &&
                                    state.dataSurface->SurfShadowGlazingConstruct(ReflSurfNum) > 0) {
                                    state.dataSolarReflectionManager->ConstrNumRefl = state.dataSurface->SurfShadowGlazingConstruct(ReflSurfNum);
                                    state.dataSolarReflectionManager->SpecReflectance =
                                        state.dataSurface->SurfShadowGlazingFrac(ReflSurfNum) *
                                        Window::POLYF(
                                            std::abs(state.dataSolarReflectionManager->CosIncAngRefl),
                                            state.dataConstruction->Construct(state.dataSolarReflectionManager->ConstrNumRefl).ReflSolBeamFrontCoef);
                                }
                                // Angle of incidence of reflected beam on receiving surface
                                state.dataSolarReflectionManager->CosIncAngRec =
                                    dot(state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NormVec,
                                        state.dataSolarReflectionManager->SunVecMir);
                                state.dataSolarReflectionManager->ReflFac =
                                    state.dataSolarReflectionManager->SpecReflectance * state.dataSolarReflectionManager->CosIncAngRec;
                                // Contribution to specular reflection factor
                                ReflBmToBmSolObs(RecPtNum) += state.dataSolarReflectionManager->ReflFac;
                                ReflFacTimesCosIncSum(RecPtNum) +=
                                    state.dataSolarReflectionManager->ReflFac * state.dataSolarReflectionManager->CosIncAngRec;
                            } // End of check if reflecting surface was hit
                        } // End of loop over receiving points
                    } // End of check if valid reflecting surface
                } // End of loop over obstructing surfaces
                // Average over receiving points

                for (int RecPtNum = 1; RecPtNum <= NumRecPts; ++RecPtNum) {
                    if (ReflBmToBmSolObs(RecPtNum) != 0.0) {
                        state.dataSolarReflectionManager->CosIncWeighted = ReflFacTimesCosIncSum(RecPtNum) / ReflBmToBmSolObs(RecPtNum);
                    } else {
                        state.dataSolarReflectionManager->CosIncWeighted = 0.0;
                    }
                    state.dataSurface->SurfCosIncAveBmToBmSolObs(iHour, SurfNum) += state.dataSolarReflectionManager->CosIncWeighted;
                    state.dataSurface->SurfReflFacBmToBmSolObs(iHour, SurfNum) += ReflBmToBmSolObs(RecPtNum);
                }
                state.dataSurface->SurfReflFacBmToBmSolObs(iHour, SurfNum) /= double(NumRecPts);
                state.dataSurface->SurfCosIncAveBmToBmSolObs(iHour, SurfNum) /= double(NumRecPts);
            } // End of check if number of possible obstructions > 0
        } // End of loop over receiving surfaces
    }

    //=================================================================================================

    void CalcSkySolDiffuseReflFactors(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   October 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates factors for irradiance on exterior heat transfer surfaces due to
        // reflection of sky diffuse solar radiation from obstructions and ground.

        // Using/Aliasing

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D<Real64> ReflSkySolObs(state.dataSurface->MaxRecPts); // Irradiance at a receiving point for sky diffuse solar
        // reflected from obstructions, divided by unobstructed
        // sky diffuse horizontal irradiance
        Array1D<Real64> ReflSkySolGnd(state.dataSurface->MaxRecPts); // Irradiance at a receiving point for sky diffuse solar
        // reflected from ground, divided by unobstructed
        // sky diffuse horizontal irradiance
        bool hitObs; // True iff obstruction is hit

        Real64 const DPhi(Constant::PiOvr2 / (AltAngStepsForSolReflCalc / 2.0));      // Altitude angle and increment (radians)
        Real64 const DTheta(2.0 * Constant::Pi / (2.0 * AzimAngStepsForSolReflCalc)); // Azimuth increment (radians)

        // Pre-compute these constants
        // Initialize the 0 index with dummy value so the iterators line up below
        std::vector<Real64> sin_Phi({-1});
        std::vector<Real64> cos_Phi({-1});
        for (int IPhi = 1; IPhi <= (AltAngStepsForSolReflCalc / 2); ++IPhi) {
            Real64 Phi((IPhi - 0.5) * DPhi); // Altitude angle and increment (radians)
            sin_Phi.push_back(std::sin(Phi));
            cos_Phi.push_back(std::cos(Phi));
        }

        std::vector<Real64> sin_Theta({-1});
        std::vector<Real64> cos_Theta({-1});
        for (int ITheta = 1; ITheta <= 2 * AzimAngStepsForSolReflCalc; ++ITheta) {
            Real64 Theta = (ITheta - 0.5) * DTheta; // Azimuth angle (radians)
            sin_Theta.push_back(std::sin(Theta));
            cos_Theta.push_back(std::cos(Theta));
        }

        DisplayString(state, "Calculating Sky Diffuse Exterior Solar Reflection Factors");
        ReflSkySolObs = 0.0;
        ReflSkySolGnd = 0.0;

        for (state.dataSolarReflectionManager->iRecSurfNum = 1;
             state.dataSolarReflectionManager->iRecSurfNum <= state.dataSolarReflectionManager->TotSolReflRecSurf;
             ++state.dataSolarReflectionManager->iRecSurfNum) {
            state.dataSolarReflectionManager->iSurfNum =
                state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->iRecSurfNum).SurfNum;
            for (state.dataSolarReflectionManager->iRecPtNum = 1;
                 state.dataSolarReflectionManager->iRecPtNum <=
                 state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->iRecSurfNum).NumRecPts;
                 ++state.dataSolarReflectionManager->iRecPtNum) {
                ReflSkySolObs(state.dataSolarReflectionManager->iRecPtNum) = 0.0;
                ReflSkySolGnd(state.dataSolarReflectionManager->iRecPtNum) = 0.0;
                for (state.dataSolarReflectionManager->iRayNum = 1;
                     state.dataSolarReflectionManager->iRayNum <=
                     state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->iRecSurfNum).NumReflRays;
                     ++state.dataSolarReflectionManager->iRayNum) {
                    state.dataSolarReflectionManager->HitPntSurfNum =
                        state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->iRecSurfNum)
                            .HitPtSurfNum(state.dataSolarReflectionManager->iRayNum, state.dataSolarReflectionManager->iRecPtNum);
                    // Skip rays that do not hit an obstruction or ground.
                    // (Note that if a downgoing ray does not hit an obstruction it will have HitPtSurfNum = 0
                    // if the receiving point is below ground level (see subr. InitSolReflRecSurf); this means
                    // that a below-ground-level receiving point receives no ground-reflected radiation although
                    // it is allowed to receive obstruction-reflected solar radiation and direct (unreflected)
                    // beam and sky solar radiation. As far as reflected solar is concerned, the program does
                    // not handle a sloped ground plane or a horizontal ground plane whose level is different
                    // from one side of the building to another.)
                    if (state.dataSolarReflectionManager->HitPntSurfNum == 0) {
                        continue; // Ray hits sky or obstruction with receiving pt. below ground level
                    }
                    state.dataSolarReflectionManager->HitPntRefl =
                        state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->iRecSurfNum)
                            .HitPt(state.dataSolarReflectionManager->iRayNum, state.dataSolarReflectionManager->iRecPtNum);
                    if (state.dataSolarReflectionManager->HitPntSurfNum > 0) {
                        // Ray hits an obstruction
                        // Skip hit points on daylighting shelves, from which solar reflection is separately calculated
                        if (state.dataSurface->SurfDaylightingShelfInd(state.dataSolarReflectionManager->HitPntSurfNum) > 0) {
                            continue;
                        }
                        // Reflected radiance at hit point divided by unobstructed sky diffuse horizontal irradiance
                        state.dataSolarReflectionManager->HitPtSurfNumX = state.dataSolarReflectionManager->HitPntSurfNum;
                        // Each shading surface has a "mirror" duplicate surface facing in the opposite direction.
                        // The following gets the correct side of a shading surface in order to get the right value
                        // of DifShdgRatioIsoSky (the two sides can have different sky shadowing).
                        if (state.dataSurface->Surface(state.dataSolarReflectionManager->HitPntSurfNum).IsShadowing) {
                            if (dot(state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->iRecSurfNum)
                                        .RayVec(state.dataSolarReflectionManager->iRayNum),
                                    state.dataSurface->Surface(state.dataSolarReflectionManager->HitPntSurfNum).OutNormVec) > 0.0) {
                                if (state.dataSolarReflectionManager->HitPntSurfNum + 1 < state.dataSurface->TotSurfaces) {
                                    state.dataSolarReflectionManager->HitPtSurfNumX = state.dataSolarReflectionManager->HitPntSurfNum + 1;
                                }
                                if (state.dataSurface->SurfDaylightingShelfInd(state.dataSolarReflectionManager->HitPtSurfNumX) > 0) {
                                    continue;
                                }
                            }
                        }

                        if (!state.dataSysVars->DetailedSkyDiffuseAlgorithm || !state.dataSurface->ShadingTransmittanceVaries ||
                            state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::Minimal) {
                            state.dataSolarReflectionManager->SkyReflSolRadiance =
                                state.dataSurface->Surface(state.dataSolarReflectionManager->HitPtSurfNumX).ViewFactorSky *
                                state.dataSolarShading->SurfDifShdgRatioIsoSky(state.dataSolarReflectionManager->HitPtSurfNumX) *
                                state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->iRecSurfNum)
                                    .HitPtSolRefl(state.dataSolarReflectionManager->iRayNum, state.dataSolarReflectionManager->iRecPtNum);
                        } else {
                            state.dataSolarReflectionManager->SkyReflSolRadiance =
                                state.dataSurface->Surface(state.dataSolarReflectionManager->HitPtSurfNumX).ViewFactorSky *
                                state.dataSolarShading->SurfDifShdgRatioIsoSkyHRTS(1, 1, state.dataSolarReflectionManager->HitPtSurfNumX) *
                                state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->iRecSurfNum)
                                    .HitPtSolRefl(state.dataSolarReflectionManager->iRayNum, state.dataSolarReflectionManager->iRecPtNum);
                        }
                        state.dataSolarReflectionManager->dReflSkySol =
                            state.dataSolarReflectionManager->SkyReflSolRadiance *
                            state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->iRecSurfNum)
                                .dOmegaRay(state.dataSolarReflectionManager->iRayNum) *
                            state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->iRecSurfNum)
                                .CosIncAngRay(state.dataSolarReflectionManager->iRayNum) /
                            Constant::Pi;
                        ReflSkySolObs(state.dataSolarReflectionManager->iRecPtNum) += state.dataSolarReflectionManager->dReflSkySol;
                    } else {
                        // Ray hits ground;
                        // Find radiance at hit point due to reflection of sky diffuse reaching
                        // ground directly, i.e., without reflecting from obstructions.
                        // Send rays upward from hit point and see which ones are unobstructed and so go to sky.
                        // Divide hemisphere centered at ground hit point into elements of altitude Phi and
                        // azimuth Theta and create upward-going ray unit vector at each Phi,Theta pair.
                        // Phi = 0 at the horizon; Phi = Pi/2 at the zenith.
                        Real64 dReflSkyGnd = 0.0; // Factor for ground radiance due to direct sky diffuse reflection
                        // Altitude loop
                        for (int IPhi = 1; IPhi <= (AltAngStepsForSolReflCalc / 2); ++IPhi) {
                            // Third component of ray unit vector in (Theta,Phi) direction
                            state.dataSolarReflectionManager->URay(3) = sin_Phi[IPhi];
                            Real64 dOmega = cos_Phi[IPhi] * DTheta * DPhi; // Solid angle increment (steradians)
                            // Cosine of angle of incidence of ray on ground
                            Real64 CosIncAngRayToSky = sin_Phi[IPhi]; // Cosine of incidence angle on ground of ray to sky
                            // Azimuth loop
                            for (int ITheta = 1; ITheta <= 2 * AzimAngStepsForSolReflCalc; ++ITheta) {
                                state.dataSolarReflectionManager->URay.x = cos_Phi[IPhi] * cos_Theta[ITheta];
                                state.dataSolarReflectionManager->URay.y = cos_Phi[IPhi] * sin_Theta[ITheta];
                                // Does this ray hit an obstruction?
                                hitObs = false;
                                for (int ObsSurfNum : state.dataSurface->AllShadowPossObstrSurfaceList) {
                                    state.dataSolarReflectionManager->iObsSurfNum = ObsSurfNum;
                                    // Horizontal roof surfaces cannot be obstructions for rays from ground
                                    if (state.dataSurface->Surface(state.dataSolarReflectionManager->iObsSurfNum).Tilt < 5.0) {
                                        continue;
                                    }
                                    if (!state.dataSurface->Surface(state.dataSolarReflectionManager->iObsSurfNum).IsShadowing) {
                                        if (dot(state.dataSolarReflectionManager->URay,
                                                state.dataSurface->Surface(state.dataSolarReflectionManager->iObsSurfNum).OutNormVec) >= 0.0) {
                                            continue;
                                        }
                                        // Special test for vertical surfaces with URay dot OutNormVec < 0; excludes
                                        // case where ground hit point is in back of ObsSurfNum
                                        if (state.dataSurface->Surface(state.dataSolarReflectionManager->iObsSurfNum).Tilt > 89.0 &&
                                            state.dataSurface->Surface(state.dataSolarReflectionManager->iObsSurfNum).Tilt < 91.0) {
                                            state.dataSolarReflectionManager->SurfVert =
                                                state.dataSurface->Surface(state.dataSolarReflectionManager->iObsSurfNum).Vertex(2);
                                            state.dataSolarReflectionManager->SurfVertToGndPt =
                                                state.dataSolarReflectionManager->HitPntRefl - state.dataSolarReflectionManager->SurfVert;
                                            if (dot(state.dataSolarReflectionManager->SurfVertToGndPt,
                                                    state.dataSurface->Surface(state.dataSolarReflectionManager->iObsSurfNum).OutNormVec) < 0.0) {
                                                continue;
                                            }
                                        }
                                    }
                                    hitObs = PierceSurface(state,
                                                           state.dataSolarReflectionManager->iObsSurfNum,
                                                           state.dataSolarReflectionManager->HitPntRefl,
                                                           state.dataSolarReflectionManager->URay,
                                                           state.dataSolarReflectionManager->HitPntObs);
                                    if (hitObs) {
                                        break;
                                    }
                                }
                                if (hitObs) {
                                    continue; // Obstruction hit
                                }
                                // Sky is hit
                                dReflSkyGnd += CosIncAngRayToSky * dOmega / Constant::Pi;
                            } // End of azimuth loop
                        } // End of altitude loop
                        ReflSkySolGnd(state.dataSolarReflectionManager->iRecPtNum) +=
                            dReflSkyGnd *
                            state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->iRecSurfNum)
                                .dOmegaRay(state.dataSolarReflectionManager->iRayNum) *
                            state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->iRecSurfNum)
                                .CosIncAngRay(state.dataSolarReflectionManager->iRayNum) /
                            Constant::Pi;
                    } // End of check if ray from receiving point hits obstruction or ground
                } // End of loop over rays from receiving point
            } // End of loop over receiving points

            // Average over receiving points
            state.dataSurface->SurfReflFacSkySolObs(state.dataSolarReflectionManager->iSurfNum) = 0.0;
            state.dataSurface->SurfReflFacSkySolGnd(state.dataSolarReflectionManager->iSurfNum) = 0.0;
            state.dataSolarReflectionManager->iNumRecPts =
                state.dataSolarReflectionManager->SolReflRecSurf(state.dataSolarReflectionManager->iRecSurfNum).NumRecPts;
            for (state.dataSolarReflectionManager->iRecPtNum = 1;
                 state.dataSolarReflectionManager->iRecPtNum <= state.dataSolarReflectionManager->iNumRecPts;
                 ++state.dataSolarReflectionManager->iRecPtNum) {
                state.dataSurface->SurfReflFacSkySolObs(state.dataSolarReflectionManager->iSurfNum) +=
                    ReflSkySolObs(state.dataSolarReflectionManager->iRecPtNum);
                state.dataSurface->SurfReflFacSkySolGnd(state.dataSolarReflectionManager->iSurfNum) +=
                    ReflSkySolGnd(state.dataSolarReflectionManager->iRecPtNum);
            }
            state.dataSurface->SurfReflFacSkySolObs(state.dataSolarReflectionManager->iSurfNum) /= state.dataSolarReflectionManager->iNumRecPts;
            state.dataSurface->SurfReflFacSkySolGnd(state.dataSolarReflectionManager->iSurfNum) /= state.dataSolarReflectionManager->iNumRecPts;
            // Do not allow SurfReflFacBmToDiffSolGnd to exceed the surface's unobstructed ground view factor
            state.dataSurface->SurfReflFacSkySolGnd(state.dataSolarReflectionManager->iSurfNum) =
                min(0.5 * (1.0 - state.dataSurface->Surface(state.dataSolarReflectionManager->iSurfNum).CosTilt),
                    state.dataSurface->SurfReflFacSkySolGnd(state.dataSolarReflectionManager->iSurfNum));
            // Note: the above factors are dimensionless; they are equal to
            // (W/m2 reflected solar incident on SurfNum)/(W/m2 unobstructed horizontal sky diffuse irradiance)
        } // End of loop over receiving surfaces
    }

} // namespace SolarReflectionManager

} // namespace EnergyPlus
