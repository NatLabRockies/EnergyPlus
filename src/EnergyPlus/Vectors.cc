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

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/Vectors.hh>

namespace EnergyPlus::Vectors {
// Module containing the routines dealing with Vector operations

// MODULE INFORMATION:
//       AUTHOR         Linda Lawrie
//       DATE WRITTEN   April 2000

// PURPOSE OF THIS MODULE:
// This module uses a global "vector" data structure and defines
// operations (using the F90 "operator" features) and other
// manipulations used with Vectors.

// Vectors should allow for more modern interpretations of the
// calculations used in the shadowing and other surface manipulations.

// METHODOLOGY EMPLOYED:
// Uses the "Vector" derived type variables to allow for more readable
// calculations.

// REFERENCES: Original idea from F90 for Scientists and
// Engineers, S. J. Chapman, 1996.
// The module defines 8 operations which can be performed on vectors:
//               Operation                     Operator
//               =========                     ========
//   1.  Creation from a real array               =
//   2.  Conversion to real array                 =
//   3.  Vector addition                          +
//   4.  Vector subtraction                       -
//   5.  Vector-scalar multiplication (4 cases)   *
//   6.  Vector-scalar division (2 cases)         /
//   7.  Dot product                            .dot.
//   8.  Cross product                            *
//   9.  2d dot product                         .twoddot.
//  10.  2d Cross product                       .twodcross.
// It contains a total of 12 procedures to implement those
// operations:  array_to_vector, vector_to_array, vector_add,
// vector_subtract, vector_times_real, real_times_vector,
// vector_times_int, int_times_vector, vector_div_real,
// vector_div_int, dot_product, and cross_product.

// Using/Aliasing
using namespace DataVectorTypes;

// MODULE PARAMETER DEFINITIONS

// Object Data
Vector const XUnit(1.0, 0.0, 0.0);
Vector const YUnit(0.0, 1.0, 0.0);
Vector const ZUnit(0.0, 0.0, 1.0);

Real64 AreaPolygon(int const n, Array1D<Vector> &p)
{

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the area of a polygon defined by the input vectors.

    // REFERENCE:
    // Graphic Gems.

    // Argument array dimensioning
    EP_SIZE_CHECK(p, n);

    Vector edge0 = p[1] - p[0];
    Vector edge1 = p[2] - p[0];

    Vector edgex = cross(edge0, edge1);
    Vector nor = VecNormalize(edgex);

    //  Initialize csum
    Vector csum;
    csum = 0.0;

    for (int i = 0; i <= n - 2; ++i) {
        csum += cross(p[i], p[i + 1]);
    }
    csum += cross(p[n - 1], p[0]);

    Real64 areap = 0.5 * std::abs(dot(nor, csum));

    return areap;
}

Real64 VecSquaredLength(Vector const &vec)
{

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the squared length of the input vector.

    // REFERENCE:
    // Graphic Gems.

    Real64 vecsqlen = (vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);

    return vecsqlen;
}

Real64 VecLength(Vector const &vec)
{

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the length of the input vector.

    // REFERENCE:
    // Graphic Gems.

    Real64 veclen = std::sqrt(VecSquaredLength(vec));

    return veclen;
}

Vector VecNegate(Vector const &vec)
{

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine negates the input vector and returns that vector.

    // REFERENCE:
    // Graphic Gems.

    Vector VecNegate;

    VecNegate.x = -vec.x;
    VecNegate.y = -vec.y;
    VecNegate.z = -vec.z;

    return VecNegate;
}

Vector VecNormalize(Vector const &vec)
{

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine normalizes the input vector and returns the normalized vector

    // REFERENCE:
    // Graphic Gems.

    // Return value
    Vector VecNormalize;

    Real64 veclen = VecLength(vec);
    if (veclen != 0.0) {
        VecNormalize.x = vec.x / veclen;
        VecNormalize.y = vec.y / veclen;
        VecNormalize.z = vec.z / veclen;
    } else {
        VecNormalize.x = 0.0;
        VecNormalize.y = 0.0;
        VecNormalize.z = 0.0;
    }

    return VecNormalize;
}

void VecRound(Vector &vec, Real64 const roundto)
{

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine rounds the input vector to a specified "rounding" value.

    // REFERENCE:
    // Graphic Gems.

    vec.x = nint64(vec.x * roundto) / roundto;
    vec.y = nint64(vec.y * roundto) / roundto;
    vec.z = nint64(vec.z * roundto) / roundto;
}

void DetermineAzimuthAndTilt(Array1D<Vector> const &Surf, // Surface Definition
                             Real64 &Azimuth,             // Outward Normal Azimuth Angle
                             Real64 &Tilt,                // Tilt angle of surface
                             Vector &lcsx,
                             Vector &lcsy,
                             Vector &lcsz,
                             Vector const &NewellSurfaceNormalVector)
{

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine determines the Azimuth (outward normal) angle,
    // Tilt angle of a given surface defined by the set of input vectors.

    // REFERENCE:
    // Discussions and examples from Bill Carroll, LBNL.

    lcsx = VecNormalize(Surf(3) - Surf(2));
    lcsz = NewellSurfaceNormalVector;
    lcsy = cross(lcsz, lcsx);

    Real64 costheta = dot(lcsz, ZUnit);

    //    if ( fabs(costheta) < 1.0d0) { // normal cases
    Real64 constexpr epsilon = 1.12e-16;
    Real64 rotang_0 = 0.0;
    if (std::abs(costheta) < 1.0 - epsilon) { // Autodesk Added - 1.12e-16 to treat 1 bit from 1.0 as 1.0 to correct different behavior seen in
                                              // release vs debug build due to slight precision differences: May want larger epsilon here
        // azimuth
        Vector x2 = cross(ZUnit, lcsz);
        rotang_0 = std::atan2(dot(x2, YUnit), dot(x2, XUnit));
    } else {
        // azimuth
        rotang_0 = std::atan2(dot(lcsx, YUnit), dot(lcsx, XUnit));
    }

    Real64 tlt = std::acos(NewellSurfaceNormalVector.z);
    tlt /= Constant::DegToRad;

    Real64 az = rotang_0;

    az /= Constant::DegToRad;
    az = mod(450.0 - az, 360.0);
    az += 90.0;
    if (az < 0.0) {
        az += 360.0;
    }
    az = mod(az, 360.0);

    // Clean up angle precision
    if (std::abs(az - 360.0) < Constant::OneThousandth) { // Bring small angles to zero
        az = 0.0;
    } else if (std::abs(az - 180.0) <
               Constant::OneMillionth) { // Bring angles near 180 to 180 //Autodesk Added to clean up debug--release discrepancies
        az = 180.0;
    }
    if (std::abs(tlt - 180.0) < Constant::OneMillionth) { // Bring angles near 180 to 180 //Autodesk Added to clean up debug--release discrepancies
        tlt = 180.0;
    }

    Azimuth = az;
    Tilt = tlt;
}

void PlaneEquation(Array1D<Vector> &verts, // Structure of the surface
                   int const nverts,       // Number of vertices in the surface
                   PlaneEq &plane,         // Equation of plane from inputs
                   bool &error             // returns true for degenerate surface
)
{

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the plane equation for a given surface (which should be planar).

    // REFERENCE:
    // Graphic Gems

    // Argument array dimensioning
    EP_SIZE_CHECK(verts, nverts);

    Vector normal = Vector(0.0, 0.0, 0.0);
    Vector refpt = Vector(0.0, 0.0, 0.0);
    for (int i = 0; i <= nverts - 1; ++i) {
        Vector const &u(verts[i]);
        Vector const &v(i < nverts - 1 ? verts[i + 1] : verts[0]);
        normal.x += (u.y - v.y) * (u.z + v.z);
        normal.y += (u.z - v.z) * (u.x + v.x);
        normal.z += (u.x - v.x) * (u.y + v.y);
        refpt += u;
    }
    // normalize the polygon normal to obtain the first
    //  three coefficients of the plane equation
    Real64 lenvec = VecLength(normal);
    error = false;
    if (lenvec != 0.0) { // should this be >0
        plane.x = normal.x / lenvec;
        plane.y = normal.y / lenvec;
        plane.z = normal.z / lenvec;
        // compute the last coefficient of the plane equation
        lenvec *= nverts;
        plane.w = -dot(refpt, normal) / lenvec;
    } else {
        error = true;
    }
}

Real64 Pt2Plane(Vector const &pt,   // Point for determining the distance
                PlaneEq const &pleq // Equation of the plane
)
{

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the distance from a point to the plane (of a surface).  Used to determine the reveal of a heat transfer subsurface.

    // REFERENCE:
    // Graphic Gems

    Real64 PtDist; // Distance of the point to the plane

    PtDist = (pleq.x * pt.x) + (pleq.y * pt.y) + (pleq.z * pt.z) + pleq.w;

    return PtDist;
}

void CreateNewellAreaVector(Array1D<Vector> const &VList, int const NSides, Vector &OutNewellAreaVector)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2004

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine creates a "Newell" vector from the vector list for a surface face.  Also the Newell Area vector.

    // REFERENCES:
    // Collaboration with Bill Carroll, LBNL.

    OutNewellAreaVector = 0.0;

    Vector V1 = VList(2) - VList(1);
    for (int Vert = 3; Vert <= NSides; ++Vert) {
        Vector V2 = VList(Vert) - VList(1);
        OutNewellAreaVector += cross(V1, V2);
        V1 = V2;
    }

    OutNewellAreaVector /= 2.0;
}

void CreateNewellSurfaceNormalVector(Array1D<Vector> const &VList, int const NSides, Vector &OutNewellSurfaceNormalVector)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   Jan 2011

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine creates a "Newell" surface normal vector from the vector list for a surface face.

    // REFERENCES:
    // September 2010: from OpenGL.org
    // Begin Function CalculateSurfaceNormal (Input Polygon) Returns Vector
    //    Set Vertex Normal to (0, 0, 0)
    //    Begin Cycle for Index in [0, Polygon.vertexNumber)
    //       Set Vertex Current to Polygon.verts[Index]
    //       Set Vertex Next    to Polygon.verts[(Index plus 1) mod Polygon.vertexNumber]
    //       Set Normal.x to Sum of Normal.x and (multiply (Current.y minus Next.y) by (Current.z plus Next.z)
    //       Set Normal.y to Sum of Normal.y and (multiply (Current.z minus Next.z) by (Current.x plus Next.x)
    //       Set Normal.z to Sum of Normal.z and (multiply (Current.x minus Next.x) by (Current.y plus Next.y)
    //    End Cycle
    //    Returning Normalize(Normal)
    // End Function

    OutNewellSurfaceNormalVector = 0.0;
    Real64 xvalue = 0.0;
    Real64 yvalue = 0.0;
    Real64 zvalue = 0.0;

    //     IF (NSides > 3) THEN
    for (int Side = 1; Side <= NSides; ++Side) {
        int curVert = Side;
        int nextVert = Side + 1;
        if (nextVert > NSides) {
            nextVert = 1;
        }
        xvalue += (VList(curVert).y - VList(nextVert).y) * (VList(curVert).z + VList(nextVert).z);
        yvalue += (VList(curVert).z - VList(nextVert).z) * (VList(curVert).x + VList(nextVert).x);
        zvalue += (VList(curVert).x - VList(nextVert).x) * (VList(curVert).y + VList(nextVert).y);
    }

    OutNewellSurfaceNormalVector.x = xvalue;
    OutNewellSurfaceNormalVector.y = yvalue;
    OutNewellSurfaceNormalVector.z = zvalue;
    OutNewellSurfaceNormalVector = VecNormalize(OutNewellSurfaceNormalVector);
}

void CompareTwoVectors(Vector const &vector1, // standard vector
                       Vector const &vector2, // standard vector
                       bool &areSame,         // true if the two vectors are the same within specified tolerance
                       Real64 const tolerance // specified tolerance
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   February 2012

    // PURPOSE OF THIS SUBROUTINE:
    // This routine will provide the ability to compare two vectors (e.g. surface normals) to be the same within a specified tolerance.

    // METHODOLOGY EMPLOYED:
    // compare each element (x,y,z)

    areSame = true;
    if (std::abs(vector1.x - vector2.x) > tolerance) {
        areSame = false;
    }
    if (std::abs(vector1.y - vector2.y) > tolerance) {
        areSame = false;
    }
    if (std::abs(vector1.z - vector2.z) > tolerance) {
        areSame = false;
    }
}

void CalcCoPlanarNess(Array1D<Vector> &Surf, int const NSides, bool &IsCoPlanar, Real64 &MaxDist, int &ErrorVertex)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   June 2004

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine provides the calculation to determine if the surface is planar or not.

    // REFERENCES:
    // Eric W. Weisstein. "Coplanar." From MathWorld--A Wolfram Web Resource.
    //   http://mathworld.wolfram.com/Coplanar.html

    // Argument array dimensioning
    EP_SIZE_CHECK(Surf, NSides);

    bool plerror;
    PlaneEq NewellPlane;

    IsCoPlanar = true;
    MaxDist = 0.0;
    ErrorVertex = 0;

    // Use first three to determine plane
    PlaneEquation(Surf, NSides, NewellPlane, plerror);

    for (int vert = 1; vert <= NSides; ++vert) {
        Real64 dist = Pt2Plane(Surf(vert), NewellPlane);
        if (std::abs(dist) > MaxDist) {
            MaxDist = std::abs(dist);
            ErrorVertex = vert;
        }
    }

    if (std::abs(MaxDist) > Constant::SmallDistance) {
        IsCoPlanar = false;
    }
}

std::vector<int>
PointsInPlane(Array1D<Vector> &BaseSurf, int const BaseSides, Array1D<Vector> const &QuerySurf, int const QuerySides, bool &ErrorFound)
{
    std::vector<int> pointIndices;

    PlaneEq NewellPlane;
    PlaneEquation(BaseSurf, BaseSides, NewellPlane, ErrorFound);

    for (int vert = 1; vert <= QuerySides; ++vert) {
        Real64 dist = Pt2Plane(QuerySurf(vert), NewellPlane);
        if (std::abs(dist) < Constant::SmallDistance) { // point on query surface is co-planar with base surface
            pointIndices.push_back(vert);
        }
    }
    return pointIndices;
}

Real64 CalcPolyhedronVolume(EnergyPlusData const &state, Polyhedron const &Poly)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   June 2004

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine provides the volume calculation for a polyhedron (i.e. Zone).

    // REFERENCES:
    // Conversations with Bill Carroll, LBNL.

    // Object Data
    Vector p3FaceOrigin;

    Real64 Volume = 0.0;

    for (int NFace = 1; NFace <= Poly.NumSurfaceFaces; ++NFace) {
        p3FaceOrigin = Poly.SurfaceFace(NFace).FacePoints(2);
        Real64 PyramidVolume = dot(Poly.SurfaceFace(NFace).NewellAreaVector, (p3FaceOrigin - state.dataVectors->p0));
        Volume += PyramidVolume / 3.0;
    }
    return Volume;
}

} // namespace EnergyPlus::Vectors
