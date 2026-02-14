// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

#ifndef DXFEarClipping_hh_INCLUDED
#define DXFEarClipping_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataVectorTypes.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DXFEarClipping {

    // Using/Aliasing
    using DataVectorTypes::dTriangle;
    using DataVectorTypes::Vector;
    using DataVectorTypes::Vector_2d;

    // Data

    // Derived type definitions:
    // na

    // Module variable declarations:
    // na
    // Subroutine specifications for module <module_name>:

    // Functions

    bool InPolygon(Vector const &point, Array1D<Vector> &poly, int const nsides);

    Real64 Modulus(Vector const &point);

    int Triangulate(EnergyPlusData &state,
                    int const nsides, // number of sides to polygon
                    Array1D<Vector> &polygon,
                    Array1D<dTriangle> &outtriangles,
                    Real64 const surfazimuth,            // surface azimuth angle (outward facing normal)
                    Real64 const surftilt,               // surface tilt angle
                    std::string const &surfname,         // surface name (for error messages)
                    DataSurfaces::SurfaceClass surfclass // surface class
    );

    Real64 angle_2dvector(Real64 const xa, // vertex coordinate
                          Real64 const ya, // vertex coordinate
                          Real64 const xb, // vertex coordinate
                          Real64 const yb, // vertex coordinate
                          Real64 const xc, // vertex coordinate
                          Real64 const yc  // vertex coordinate
    );

    bool polygon_contains_point_2d(int const nsides,            // number of sides (vertices)
                                   Array1D<Vector_2d> &polygon, // points of polygon
                                   Vector_2d const &point       // point to be tested
    );

    void generate_ears(EnergyPlusData &state,
                       int const nvert, // number of vertices in polygon
                       Array1D<Vector_2d> &vertex,
                       Array1D_int &ears,       // number of ears possible (dimensioned to nvert)
                       int &nears,              // number of ears found
                       Array1D_int &r_vertices, // number of reflex vertices (>180) possible
                       int &nrverts,            // number of reflex vertices found (>=180)
                       Array1D_int &c_vertices, // number of convex vertices
                       int &ncverts,            // number of convex vertices found (< 180)
                       Array1D_bool &removed,   // array that shows if a vertex has been removed (calling routine)
                       Array1D_int &earvert,    // vertex indicators for first ear
                       Array1D<Real64> &rangles);

    void CalcWallCoordinateTransformation(int const nsides,
                                          Array1D<Vector> &polygon,
                                          Real64 const surfazimuth,
                                          Real64 const surftilt, // unused1208
                                          Array1D<Real64> &xvt,
                                          Array1D<Real64> &yvt,
                                          Array1D<Real64> &zvt);

    void CalcRfFlrCoordinateTransformation(int const nsides,
                                           Array1D<Vector> &polygon,
                                           Real64 const surfazimuth, // unused1208
                                           Real64 const surftilt,
                                           Array1D<Real64> &xvt,
                                           Array1D<Real64> &yvt,
                                           Array1D<Real64> &zvt);

    // void reorder(int &nvert); // unused1208

} // namespace DXFEarClipping

struct DXFEarClippingData : BaseGlobalStruct
{

    bool trackit = false;
    int errcount = 0;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->trackit = false;
        this->errcount = 0;
    }
};

} // namespace EnergyPlus

#endif
