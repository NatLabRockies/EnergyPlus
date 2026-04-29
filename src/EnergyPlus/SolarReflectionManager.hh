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

#ifndef SolarReflectionManager_hh_INCLUDED
#define SolarReflectionManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Vector3.hh>

// EnergyPlus Headers
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace SolarReflectionManager {

    struct SolReflRecSurfData
    {
        // Members
        int SurfNum;                     // Number of heat transfer surface
        std::string SurfName;            // Name of heat transfer surface
        int NumRecPts;                   // Number of receiving points
        Array1D<Vector3<Real64>> RecPt;  // Coordinates of receiving point on receiving surface in global CS (m)
        Vector3<Real64> NormVec;         // Unit outward normal to receiving surface
        Real64 ThetaNormVec;             // Azimuth of surface normal (radians)
        Real64 PhiNormVec;               // Altitude of surface normal (radians)
        int NumReflRays;                 // Number of rays from this receiving surface
        Array1D<Vector3<Real64>> RayVec; // Unit vector in direction of ray from receiving surface
        Array1D<Real64> CosIncAngRay;    // Cosine of angle between ray and receiving surface outward normal
        Array1D<Real64> dOmegaRay;       // Delta solid angle associated with ray
        Array2D<Vector3<Real64>> HitPt;  // For each receiving point and ray, coords of hit point on obstruction
        // that is closest to receiving point (m)
        Array2D_int HitPtSurfNum; // Number of surface containing the hit point for a ray, except:
        //  0 => ray does not hit an obstruction, but hits sky
        //  -1 => ray does not hit an obstruction, but hits ground
        Array2D<Real64> HitPtSolRefl;          // Beam-to-diffuse solar reflectance at hit point
        Array2D<Real64> RecPtHitPtDis;         // Distance from receiving point to hit point (m)
        Array2D<Vector3<Real64>> HitPtNormVec; // Hit point's surface normal unit vector pointing into hemisphere
        //  containing the receiving point
        Array1D_int PossibleObsSurfNums; // Surface numbers of possible obstructions for a receiving surf
        int NumPossibleObs;              // Number of possible obstructions for a receiving surface

        // Default Constructor
        SolReflRecSurfData() : SurfNum(0), NumRecPts(0), NormVec(0.0), ThetaNormVec(0.0), PhiNormVec(0.0), NumReflRays(0), NumPossibleObs(0)
        {
        }
    };

    void InitSolReflRecSurf(EnergyPlusData &state);

    //=====================================================================================================

    void CalcBeamSolDiffuseReflFactors(EnergyPlusData &state);

    void FigureBeamSolDiffuseReflFactors(EnergyPlusData &state, int const iHour);

    //=================================================================================================

    void CalcBeamSolSpecularReflFactors(EnergyPlusData &state);

    void FigureBeamSolSpecularReflFactors(EnergyPlusData &state, int const iHour);

    //=================================================================================================

    void CalcSkySolDiffuseReflFactors(EnergyPlusData &state);

} // namespace SolarReflectionManager

struct SolarReflectionManagerData : BaseGlobalStruct
{

    int TotSolReflRecSurf = 0; // Total number of exterior surfaces that can receive reflected solar
    int TotPhiReflRays = 0;    // Number of rays in altitude angle (-90 to 90 deg) for diffuse refl calc
    int TotThetaReflRays = 0;  // Number of rays in azimuth angle (0 to 180 deg) for diffuse refl calc

    Array1D<SolarReflectionManager::SolReflRecSurfData> SolReflRecSurf;

    // static variables extracted from functions
    int IHr = 0;            // Hour number
    Vector3<Real64> SunVec; // Unit vector to sun
    int RecSurfNum = 0;     // Receiving surface number
    int SurfNum = 0;        // Heat transfer surface number corresponding to RecSurfNum
    int RecPtNum = 0;       // Receiving point number
    int NumRecPts = 0;      // Number of receiving points on a receiving surface
    int HitPtSurfNum = 0;   // Surface number of hit point: -1 = ground,
    // 0 = sky or obstruction with receiving point below ground level,
    // >0 = obstruction with receiving point above ground level
    int RayNum = 0;                  // Ray number
    Vector3<Real64> OriginThisRay;   // Origin point of a ray (m)
    Vector3<Real64> ObsHitPt;        // Hit point on obstruction (m)
    int ObsSurfNum = 0;              // Obstruction surface number
    Real64 CosIncBmAtHitPt = 0.0;    // Cosine of incidence angle of beam solar at hit point
    Real64 CosIncBmAtHitPt2 = 0.0;   // Cosine of incidence angle of beam solar at hit point, the mirrored shading surface
    Real64 BmReflSolRadiance = 0.0;  // Solar radiance at hit point due to incident beam, divided by beam normal irradiance
    Real64 dReflBeamToDiffSol = 0.0; // Contribution to reflection factor at a receiving point from beam solar reflected from a hit point
    Real64 SunLitFract = 0.0;        // Sunlit fraction
    int NumHr = 0;                   // Hour number
    Vector3<Real64> SunVect;         // Unit vector to sun
    Vector3<Real64> SunVecMir;       // Unit vector to sun mirrored by a reflecting surface
    Vector3<Real64> RecPt;           // Receiving point (m)
    Vector3<Real64> HitPtRefl;       // Hit point on a reflecting surface (m)
    Vector3<Real64> HitPtObs;        // Hit point on obstruction (m)
    Vector3<Real64> ReflNorm;        // Unit normal to reflecting surface
    Real64 SpecReflectance = 0.0;    // Specular reflectance of a reflecting surface
    int ConstrNumRefl = 0;           // Construction number of a reflecting surface
    Real64 CosIncAngRefl = 0.0;      // Cosine of incidence angle of beam on reflecting surface
    Real64 CosIncAngRec = 0.0;       // Angle of incidence of reflected beam on receiving surface
    Real64 ReflFac = 0.0;            // Contribution to specular reflection factor
    Real64 CosIncWeighted = 0.0;     // Cosine of incidence angle on receiving surf weighted by reflection factor
    int iRecSurfNum = 0;             // Receiving surface number
    int iSurfNum = 0;                // Heat transfer surface number corresponding to RecSurfNum
    int iObsSurfNum = 0;             // Obstruction surface number
    int iRecPtNum = 0;               // Receiving point number
    int iNumRecPts = 0;              // Number of receiving points on a receiving surface
    int HitPntSurfNum = 0;           // Surface number of hit point: -1 = ground,
    // 0 = sky or obstruction with receiving point below ground level,
    // >0 = obstruction with receiving point above ground level
    int HitPtSurfNumX = 0;           // For a shading surface, HitPtSurfNum for original surface, HitPitSurfNum + 1 for mirror surface
    int iRayNum = 0;                 // Ray number
    Vector3<Real64> HitPntRefl;      // Coordinates of hit point on obstruction or ground (m)
    Vector3<Real64> HitPntObs;       // Hit point on an obstruction (m)
    Real64 SkyReflSolRadiance = 0.0; // Reflected radiance at hit point divided by unobstructed sky diffuse horizontal irradiance
    Real64 dReflSkySol = 0.0;        // Contribution to reflection factor at a receiving point from sky solar reflected from a hit point
    Vector3<Real64> URay;            // Unit vector along ray from ground hit point
    Vector3<Real64> SurfVertToGndPt; // Vector from a vertex of possible obstructing surface to ground hit point (m)
    Vector3<Real64> SurfVert;        // Surface vertex (m)

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->IHr = 0;
        this->SunVec = 0.0;
        this->RecSurfNum = 0;
        this->SurfNum = 0;
        this->RecPtNum = 0;
        this->NumRecPts = 0;
        this->HitPtSurfNum = 0;
        this->RayNum = 0;
        this->OriginThisRay = 0.0;
        this->ObsHitPt = 0.0;
        this->ObsSurfNum = 0;
        this->CosIncBmAtHitPt = 0.0;
        this->CosIncBmAtHitPt2 = 0.0;
        this->BmReflSolRadiance = 0.0;
        this->dReflBeamToDiffSol = 0.0;
        this->SunLitFract = 0.0;
        this->NumHr = 0;
        this->SunVect = 0.0;
        this->SunVecMir = 0.0;
        this->RecPt = 0.0;
        this->HitPtRefl = 0.0;
        this->HitPtObs = 0.0;
        this->ReflNorm = 0.0;
        this->SpecReflectance = 0.0;
        this->ConstrNumRefl = 0;
        this->CosIncAngRefl = 0.0;
        this->CosIncAngRec = 0.0;
        this->ReflFac = 0.0;
        this->CosIncWeighted = 0.0;
        this->iRecSurfNum = 0;
        this->iSurfNum = 0;
        this->iObsSurfNum = 0;
        this->iRecPtNum = 0;
        this->iNumRecPts = 0;
        this->HitPntSurfNum = 0;
        this->HitPtSurfNumX = 0;
        this->iRayNum = 0;
        this->HitPntRefl = 0.0;
        this->HitPntObs = 0.0;
        this->SkyReflSolRadiance = 0.0;
        this->dReflSkySol = 0.0;
        this->URay = 0.0;
        this->SurfVertToGndPt = 0.0;
        this->SurfVert = 0.0;
    }

    // Default Constructor
    SolarReflectionManagerData() = default;
};
} // namespace EnergyPlus

#endif
