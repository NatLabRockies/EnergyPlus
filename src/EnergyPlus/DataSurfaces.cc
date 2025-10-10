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
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <tuple>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WindowManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::DataSurfaces {

// MODULE INFORMATION:
//       AUTHOR         Linda Lawrie
//       DATE WRITTEN   May 2000
//       MODIFIED       July 2003, (CC) added a flag for reference air temperature
//                      Dec 2006, DJS (PSU) added logical ecoroof variable
//                      Dec 2008, TH added new properties to SurfaceWindowCalc for thermochromic windows
//                      Jul 2011, M.J. Witte and C.O. Pedersen, add new fields to OSC for last T, max and min

// Using/Aliasing
using namespace DataVectorTypes;
using namespace DataBSDFWindow;
using namespace DataHeatBalance;
using namespace DataZoneEquipment;
using namespace DataLoopNode;
using namespace Psychrometrics;
using namespace DataEnvironment;
using namespace Window;

Array1D_string const cExtBoundCondition({-6, 0}, {"KivaFoundation", "FCGround", "OSCM", "OSC", "OSC", "Ground", "ExternalEnvironment"});

Surface2D::Surface2D(ShapeCat const shapeCat, int const axis, Vertices const &v, Vector2D const &vl, Vector2D const &vu)
    : axis(axis), vertices(v), vl(vl), vu(vu)
{
    size_type const n(vertices.size());
    assert(n >= 3);

    // Reverse vertices order if clockwise
    // If sorting by y for slab method can detect clockwise faster by just comparing edges at bottom or top-most vertex
    Real64 area(0.0); // Actually 2x the signed area
    for (Vertices::size_type i = 0; i < n; ++i) {
        Vector2D const &v(vertices[i]);
        Vector2D const &w(vertices[(i + 1) % n]);
        area += (v.x * w.y) - (w.x * v.y);
    }
    if (area < 0.0) {
        std::reverse(vertices.begin() + 1, vertices.end()); // Vertices in clockwise order: Reverse all but first
    }

    // Set up edge vectors for ray--surface intersection tests
    edges.reserve(n);
    for (Vertices::size_type i = 0; i < n; ++i) {
        edges.push_back(vertices[(i + 1) % n] - vertices[i]);
    }
    if (shapeCat == ShapeCat::Rectangular) { // Set side length squared for ray--surface intersection tests
        assert(n == 4u);
        s1 = edges[0].magnitude_squared();
        s3 = edges[3].magnitude_squared();
    } else if ((shapeCat == ShapeCat::Nonconvex) || (n >= nVerticesBig)) { // Set up slabs
        assert(n >= 4u);
        slabYs.reserve(n);
        for (size_type i = 0; i < n; ++i) {
            slabYs.push_back(vertices[i].y);
        }
        std::sort(slabYs.begin(), slabYs.end());                     // Sort the vertex y coordinates
        auto const iClip(std::unique(slabYs.begin(), slabYs.end())); // Remove duplicate y-coordinate elements
        slabYs.erase(iClip, slabYs.end());
        slabYs.shrink_to_fit();
        for (size_type iSlab = 0, iSlab_end = slabYs.size() - 1; iSlab < iSlab_end; ++iSlab) { // Create slabs
            Real64 xl(std::numeric_limits<Real64>::max());
            Real64 xu(std::numeric_limits<Real64>::lowest());
            Real64 const yl(slabYs[iSlab]);
            Real64 const yu(slabYs[iSlab + 1]);
            slabs.push_back(Slab(yl, yu));
            Slab &slab(slabs.back());
            using CrossEdge = std::tuple<Real64, Real64, size_type>;
            using CrossEdges = std::vector<CrossEdge>;
            CrossEdges crossEdges;
            for (size_type i = 0; i < n; ++i) { // Find edges crossing slab
                Vector2D const &v(vertices[i]);
                Vector2D const &w(vertices[(i + 1) % n]);
                if (((v.y <= yl) && (yu <= w.y)) || // Crosses upward
                    ((yu <= v.y) && (w.y <= yl)))   // Crosses downward
                {
                    Edge const &e(edges[i]);
                    assert(e.y != 0.0);
                    Real64 const exy(e.x / e.y);
                    Real64 const xb(v.x + (yl - v.y) * exy); // x_bot coordinate where edge intersects yl
                    Real64 const xt(v.x + (yu - v.y) * exy); // x_top coordinate where edge intersects yu
                    xl = std::min(xl, std::min(xb, xt));
                    xu = std::max(xu, std::max(xb, xt));
                    crossEdges.push_back(std::make_tuple(xb, xt, i));
                }
            }
            slab.xl = xl;
            slab.xu = xu;
            assert(crossEdges.size() >= 2u);
            std::sort(crossEdges.begin(),
                      crossEdges.end(),
                      [](CrossEdge const &e1, CrossEdge const &e2) -> bool // Lambda to sort by x_mid
                      {
                          return std::get<0>(e1) + std::get<1>(e1) <
                                 std::get<0>(e2) + std::get<1>(e2); // Sort edges by x_mid: x_bot or x_top could have repeats with shared vertex
                      });
#ifndef NDEBUG // Check x_bot and x_top are also sorted
            Real64 xb(std::get<0>(crossEdges[0]));
            Real64 xt(std::get<1>(crossEdges[0]));
            Real64 const tol(1.0e-9 * std::max(std::abs(xl), std::abs(xu))); // EnergyPlus vertex precision is not tight so tolerance isn't either
            for (auto const &edge : crossEdges) {                            // Detect non-simple polygon with crossing edges
                Real64 const xbe(std::get<0>(edge));
                Real64 const xte(std::get<1>(edge));
                assert(xb <= xbe + tol);
                assert(xt <= xte + tol);
                xb = xbe;
                xt = xte;
            }
#endif
            assert((shapeCat == ShapeCat::Nonconvex) || (crossEdges.size() == 2u));
            for (auto const &edge : crossEdges) {
                size_type const iEdge(std::get<2>(edge));
                slab.edges.push_back(iEdge); // Add edge to slab
                Vector2D const &e(edges[iEdge]);
                assert(e.y != 0.0);                                   // Constant y edge can't be a crossing edge
                slab.edgesXY.push_back(e.y != 0.0 ? e.x / e.y : 0.0); // Edge inverse slope
            }
            assert(slab.edges.size() % 2 == 0u);
            assert(slab.edges.size() == slab.edgesXY.size());
        }
    }
}

// Set Precomputed Parameters
void SurfaceData::set_computed_geometry()
{
    if (Vertex.size() >= 3) { // Skip no-vertex "surfaces"
        shapeCat = computed_shapeCat();
        plane = computed_plane();
        surface2d = computed_surface2d();
    }
}

Real64 SurfaceData::getInsideAirTemperature(EnergyPlusData &state, const int t_SurfNum) const
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   June 2016
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Routine calculates reference air temperature for given surface (refactoring from the code)
    //
    // NOTE: This routine has been copy/pasted in the past in several different modules with slight
    //       modifications at some of those places. It is quite logical that reference air temperature
    //       for the surface is calculated as public function of SurfaceData structure (class) and is
    //       later called as needed. Note that SurfaceNum had to be passed to this routine because of
    //       access to global array SurfTempEffBulkAir. I would propose refactoring where SurfTempEffBulkAir
    //       is part of SurfaceData structure and instead of calling SurfTempEffBulkAir( SurfNum ) it should
    //       be called Surface( SurfNum ).TempEffBulkAir (Simon Vidanovic)

    Real64 RefAirTemp = 0;

    // determine reference air temperature for this surface
    auto &thisSpaceHB = state.dataZoneTempPredictorCorrector->spaceHeatBalance(this->spaceNum);
    switch (state.dataSurface->SurfTAirRef(t_SurfNum)) {
    case RefAirTemp::ZoneMeanAirTemp: {
        RefAirTemp = thisSpaceHB.MAT;
    } break;
    case RefAirTemp::AdjacentAirTemp: {
        RefAirTemp = state.dataHeatBal->SurfTempEffBulkAir(t_SurfNum);
    } break;
    case RefAirTemp::ZoneSupplyAirTemp: {
        // determine ZoneEquipConfigNum for this zone
        //            ControlledZoneAirFlag = .FALSE.
        // ZoneEquipConfigNum = ZoneNum;
        // check whether this zone is a controlled zone or not
        if (!state.dataHeatBal->Zone(Zone).IsControlled) {
            ShowFatalError(state,
                           format("Zones must be controlled for Ceiling-Diffuser Convection model. No system serves zone {}",
                                  state.dataHeatBal->Zone(Zone).Name));
            // return;
        }
        // determine supply air conditions
        Real64 SumSysMCp = 0;
        Real64 SumSysMCpT = 0;
        auto const &inletNodes = (state.dataHeatBal->doSpaceHeatBalance) ? state.dataZoneEquip->spaceEquipConfig(this->spaceNum).InletNode
                                                                         : state.dataZoneEquip->ZoneEquipConfig(Zone).InletNode;
        for (int nodeNum : inletNodes) {
            auto const &inNode = state.dataLoopNodes->Node(nodeNum);
            Real64 CpAir = PsyCpAirFnW(thisSpaceHB.airHumRat);
            SumSysMCp += inNode.MassFlowRate * CpAir;
            SumSysMCpT += inNode.MassFlowRate * CpAir * inNode.Temp;
        }
        // a weighted average of the inlet temperatures.
        if (SumSysMCp > 0.0) {
            // a weighted average of the inlet temperatures.
            RefAirTemp = SumSysMCpT / SumSysMCp;
        } else {
            RefAirTemp = thisSpaceHB.MAT;
        }
    } break;
    default: {
        // currently set to mean air temp but should add error warning here
        RefAirTemp = thisSpaceHB.MAT;
    } break;
    }

    return RefAirTemp;
}

Real64 SurfaceData::getOutsideAirTemperature(EnergyPlusData &state, const int t_SurfNum) const
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   June 2016
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Routine calculates outside air temperature for given surface.
    // Routine will return inside air temperature if it is interior surface. (refactoring from the code)
    //
    // NOTE: This routine has been copy/pasted in the past in several different modules with slight
    //       modifications at some of those places. Exterior/interior surface air temperature is tied to surface.
    Real64 temperature = 0;

    if (ExtBoundCond > 0) // Interzone window
    {
        temperature = getInsideAirTemperature(state, t_SurfNum);
    } else {
        if (ExtWind) {
            // Window is exposed to wind (and possibly rain)
            if (state.dataEnvrn->IsRain) {
                // Raining: since wind exposed, outside window surface gets wet
                temperature = state.dataSurface->SurfOutWetBulbTemp(t_SurfNum);
            } else {
                // Dry
                temperature = state.dataSurface->SurfOutDryBulbTemp(t_SurfNum);
            }
        } else {
            // Window not exposed to wind
            temperature = state.dataSurface->SurfOutDryBulbTemp(t_SurfNum);
        }
    }

    return temperature;
}

Real64 SurfaceData::getOutsideIR(EnergyPlusData &state, const int t_SurfNum) const
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   July 2016
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates outside infrared radiation
    Real64 value = 0;
    if (ExtBoundCond > 0) {
        value = state.dataSurface->SurfWinIRfromParentZone(ExtBoundCond) + state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(ExtBoundCond);
    } else {
        Real64 tout = getOutsideAirTemperature(state, t_SurfNum) + Constant::Kelvin;
        value = Constant::StefanBoltzmann * pow_4(tout);
        value =
            ViewFactorSkyIR * (state.dataSurface->SurfAirSkyRadSplit(t_SurfNum) * Constant::StefanBoltzmann * pow_4(state.dataEnvrn->SkyTempKelvin) +
                               (1.0 - state.dataSurface->SurfAirSkyRadSplit(t_SurfNum)) * value) +
            ViewFactorGroundIR * value;
    }
    return value;
}

Real64 SurfaceData::getSWIncident(EnergyPlusData &state, const int t_SurfNum)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   July 2016
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Return total short wave incident to the surface

    return state.dataHeatBal->SurfQRadSWOutIncident(t_SurfNum) +
           state.dataHeatBal->EnclSolQSWRad(state.dataSurface->Surface(t_SurfNum).SolarEnclIndex);
}

int SurfaceData::getTotLayers(EnergyPlusData &state) const
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   August 2016
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Returns total number of layer for current surface

    auto &construction(state.dataConstruction->Construct(Construction));
    return construction.TotLayers;
}

// Computed Shape Category
ShapeCat SurfaceData::computed_shapeCat() const
{
    if (Shape == SurfaceShape::Triangle) {
        return ShapeCat::Triangular;
    } else if (Shape == SurfaceShape::TriangularWindow) {
        return ShapeCat::Triangular;
    } else if (Shape == SurfaceShape::TriangularDoor) {
        return ShapeCat::Triangular;
    } else if (Shape == SurfaceShape::Rectangle) {
        return ShapeCat::Rectangular;
    } else if (Shape == SurfaceShape::RectangularDoorWindow) {
        return ShapeCat::Rectangular;
    } else if (Shape == SurfaceShape::RectangularOverhang) {
        return ShapeCat::Rectangular;
    } else if (Shape == SurfaceShape::RectangularLeftFin) {
        return ShapeCat::Rectangular;
    } else if (Shape == SurfaceShape::RectangularRightFin) {
        return ShapeCat::Rectangular;
    } else if (IsConvex) {
        return ShapeCat::Convex;
    } else {
        return ShapeCat::Nonconvex;
    }
}

// Computed Plane
SurfaceData::Plane SurfaceData::computed_plane() const
{
    Vertices::size_type const n(Vertex.size());
    assert(n >= 3);
    Vector center(0.0);                           // Center (vertex average) point (not mass centroid)
    Real64 a(0.0), b(0.0), c(0.0), d(0.0);        // Plane coefficients
    for (Vertices::size_type i = 0; i < n; ++i) { // Newell's method for robustness (not speed)
        Vector const &v(Vertex[i]);
        Vector const &w(Vertex[(i + 1) % n]);
        a += (v.y - w.y) * (v.z + w.z);
        b += (v.z - w.z) * (v.x + w.x);
        c += (v.x - w.x) * (v.y + w.y);
        center += v;
    }
    d = -(dot(center, Vector(a, b, c)) / n); // center/n is the center point
    return Plane(a, b, c, d);                // a*x + b*y + c*z + d = 0
}

// Computed axis-projected 2D surface
Surface2D SurfaceData::computed_surface2d() const
{
    // Project along axis of min surface range for 2D intersection use
    Vertices::size_type const n(Vertex.size());
    assert(n >= 3);
    assert(plane == computed_plane()); // Set plane first
    using Vertex2D = ObjexxFCL::Vector2<Real64>;
    using Vertices2D = ObjexxFCL::Array1D<Vertex2D>;

    // Select axis to project along
    Real64 const a(std::abs(plane.x));                                       // Plane normal x coordinate magnitude
    Real64 const b(std::abs(plane.y));                                       // Plane normal y coordinate magnitude
    Real64 const c(std::abs(plane.z));                                       // Plane normal z coordinate magnitude
    int const axis(a >= std::max(b, c) ? 0 : (b >= std::max(a, c) ? 1 : 2)); // Project along plane's normal's largest magnitude coordinate

    // Set up 2D surface
    Vertices2D v2d(n);
    Vector const &v0(Vertex[0]);
    if (axis == 0) {               // Use y,z for 2D surface
        Real64 yl(v0.y), yu(v0.y); // y coordinate ranges
        Real64 zl(v0.z), zu(v0.z); // z coordinate ranges
        for (Vertices::size_type i = 0; i < n; ++i) {
            Vector const &v(Vertex[i]);
            v2d[i] = Vertex2D(v.y, v.z);
            yl = std::min(yl, v.y);
            yu = std::max(yu, v.y);
            zl = std::min(zl, v.z);
            zu = std::max(zu, v.z);
        }
        return Surface2D(shapeCat, axis, v2d, Vertex2D(yl, zl), Vertex2D(yu, zu));
    } else if (axis == 1) {        // Use x,z for 2D surface
        Real64 xl(v0.x), xu(v0.x); // x coordinate ranges
        Real64 zl(v0.z), zu(v0.z); // z coordinate ranges
        for (Vertices::size_type i = 0; i < n; ++i) {
            Vector const &v(Vertex[i]);
            v2d[i] = Vertex2D(v.x, v.z);
            xl = std::min(xl, v.x);
            xu = std::max(xu, v.x);
            zl = std::min(zl, v.z);
            zu = std::max(zu, v.z);
        }
        return Surface2D(shapeCat, axis, v2d, Vertex2D(xl, zl), Vertex2D(xu, zu));
    } else {                       // Use x,y for 2D surface
        Real64 xl(v0.x), xu(v0.x); // x coordinate ranges
        Real64 yl(v0.y), yu(v0.y); // y coordinate ranges
        for (Vertices::size_type i = 0; i < n; ++i) {
            Vector const &v(Vertex[i]);
            v2d[i] = Vertex2D(v.x, v.y);
            xl = std::min(xl, v.x);
            xu = std::max(xu, v.x);
            yl = std::min(yl, v.y);
            yu = std::max(yu, v.y);
        }
        return Surface2D(shapeCat, axis, v2d, Vertex2D(xl, yl), Vertex2D(xu, yu));
    }
}

Real64 SurfaceData::get_average_height(EnergyPlusData &state) const
{
    if (std::abs(SinTilt) < Constant::SmallDistance) {
        return 0.0;
    }
    using Vertex2D = ObjexxFCL::Vector2<Real64>;
    using Vertices2D = ObjexxFCL::Array1D<Vertex2D>;
    Vertices::size_type const n(Vertex.size());
    assert(n >= 3);

    Vertices2D v2d(n);

    // project onto 2D vertical plane
    Real64 xRef = Vertex[0].x;
    Real64 yRef = Vertex[0].y;
    Real64 const &saz(SinAzim);
    Real64 const &caz(CosAzim);
    for (Vertices::size_type i = 0; i < n; ++i) {
        Vector const &v(Vertex[i]);
        v2d[i] = Vertex2D(-(v.x - xRef) * caz + (v.y - yRef) * saz, v.z);
    }

    // piecewise linear integration

    // Get total width of polygon
    Real64 minX(v2d[0].x), maxX(v2d[0].x);
    for (Vertices::size_type i = 0; i < n; ++i) {
        Vertex2D const &v(v2d[i]);
        minX = std::min(minX, v.x);
        maxX = std::max(maxX, v.x);
    }
    Real64 totalWidth = maxX - minX;

    if (totalWidth == 0.0) {
        // This should never happen, but if it does, print a somewhat meaningful fatal error
        // (instead of allowing a divide by zero).
        ShowFatalError(state, format("Calculated projected surface width is zero for surface=\"{}\"", Name));
    }

    Real64 averageHeight = 0.0;
    for (Vertices::size_type i = 0; i < n; ++i) {
        Vertex2D const &v(v2d[i]);

        Vertex2D *v2;
        if (i == n - 1) {
            v2 = &v2d[0];
        } else {
            v2 = &v2d[i + 1];
        }
        averageHeight += 0.5 * (v.y + v2->y) * (v2->x - v.x) / totalWidth;
    }
    return std::abs(averageHeight) / SinTilt;
}

void SurfaceData::make_hash_key(EnergyPlusData &state, const int SurfNum)
{
    auto &s_surf = state.dataSurface;

    calcHashKey = SurfaceCalcHashKey();
    calcHashKey.Construction = Construction;
    calcHashKey.Azimuth = round(Azimuth * 10.0) / 10.0;
    calcHashKey.Tilt = round(Tilt * 10.0) / 10.0;
    calcHashKey.Height = round(Height * 10.0) / 10.0;
    calcHashKey.Zone = Zone;
    calcHashKey.EnclIndex = SolarEnclIndex;
    calcHashKey.TAirRef = s_surf->SurfTAirRef(SurfNum);

    int extBoundCond = s_surf->Surface(SurfNum).ExtBoundCond;
    if (extBoundCond > 0) {
        calcHashKey.ExtZone = s_surf->Surface(extBoundCond).Zone;
        calcHashKey.ExtEnclIndex = s_surf->Surface(extBoundCond).SolarEnclIndex;
        calcHashKey.ExtCond = 1;
    } else {
        calcHashKey.ExtZone = 0;
        calcHashKey.ExtEnclIndex = 0;
        calcHashKey.ExtCond = extBoundCond;
    }

    calcHashKey.ExtSolar = ExtSolar;
    calcHashKey.ExtWind = ExtWind;
    calcHashKey.ViewFactorGround = round(ViewFactorGround * 10.0) / 10.0;
    calcHashKey.ViewFactorSky = round(ViewFactorSky * 10.0) / 10.0;

    calcHashKey.HeatTransferAlgorithm = HeatTransferAlgorithm;
    calcHashKey.intConvModel = s_surf->surfIntConv(SurfNum).model;
    calcHashKey.extConvModel = s_surf->surfExtConv(SurfNum).model;
    calcHashKey.intConvUserModelNum = s_surf->surfIntConv(SurfNum).userModelNum;
    calcHashKey.extConvUserModelNum = s_surf->surfExtConv(SurfNum).userModelNum;
    calcHashKey.OSCPtr = OSCPtr;
    calcHashKey.OSCMPtr = OSCMPtr;

    calcHashKey.FrameDivider = FrameDivider;
    calcHashKey.SurfWinStormWinConstr = s_surf->SurfWinStormWinConstr(SurfNum);

    calcHashKey.MaterialMovInsulExt = s_surf->extMovInsuls(SurfNum).matNum;
    calcHashKey.MaterialMovInsulInt = s_surf->intMovInsuls(SurfNum).matNum;
    calcHashKey.movInsulExtSchedNum = (s_surf->extMovInsuls(SurfNum).sched == nullptr) ? -1 : s_surf->extMovInsuls(SurfNum).sched->Num;
    calcHashKey.movInsulIntSchedNum = (s_surf->intMovInsuls(SurfNum).sched == nullptr) ? -1 : s_surf->intMovInsuls(SurfNum).sched->Num;

    calcHashKey.externalShadingSchedNum =
        (s_surf->Surface(SurfNum).surfExternalShadingSched != nullptr) ? s_surf->Surface(SurfNum).surfExternalShadingSched->Num : -1;
    calcHashKey.SurroundingSurfacesNum = s_surf->Surface(SurfNum).SurfSurroundingSurfacesNum;
    calcHashKey.LinkedOutAirNode = s_surf->Surface(SurfNum).SurfLinkedOutAirNode;
    calcHashKey.outsideHeatSourceTermSchedNum = (outsideHeatSourceTermSched != nullptr) ? outsideHeatSourceTermSched->Num : -1;
    calcHashKey.insideHeatSourceTermSchedNum = (insideHeatSourceTermSched != nullptr) ? insideHeatSourceTermSched->Num : -1;
    calcHashKey.ViewFactorSrdSurfs = s_surf->Surface(SurfNum).ViewFactorSrdSurfs;
}

void SurfaceData::set_representative_surface(EnergyPlusData &state, const int SurfNum)
{
    // Make hash key for this surface (used to determine uniqueness)
    state.dataSurface->Surface(SurfNum).make_hash_key(state, SurfNum);
    // Insert surface key into map. If key already exists, it will not be added.
    // Assign the representative surface number based on the first instance of the identical key
    state.dataSurface->Surface(SurfNum).RepresentativeCalcSurfNum =
        state.dataSurface->RepresentativeSurfaceMap.insert({state.dataSurface->Surface(SurfNum).calcHashKey, SurfNum}).first->second;

    state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).RepresentativeCalcSurfNum).ConstituentSurfaceNums.push_back(SurfNum);
}

// Functions

void SetSurfaceOutBulbTempAt(EnergyPlusData &state)
{
    if (state.dataEnvrn->SiteTempGradient == 0.0) {
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; SurfNum++) {
            state.dataSurface->SurfOutDryBulbTemp(SurfNum) = state.dataEnvrn->OutDryBulbTemp;
            state.dataSurface->SurfOutWetBulbTemp(SurfNum) = state.dataEnvrn->OutWetBulbTemp;
        }
    } else {
        Real64 const BaseDryTemp(state.dataEnvrn->OutDryBulbTemp + state.dataEnvrn->WeatherFileTempModCoeff);
        Real64 const BaseWetTemp(state.dataEnvrn->OutWetBulbTemp + state.dataEnvrn->WeatherFileTempModCoeff);
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; SurfNum++) {
            // Base temperatures at Z = 0 (C)
            Real64 const Z(state.dataSurface->Surface(SurfNum).Centroid.z); // Centroid value
            if (Z <= 0.0) {
                state.dataSurface->SurfOutDryBulbTemp(SurfNum) = BaseDryTemp;
                state.dataSurface->SurfOutWetBulbTemp(SurfNum) = BaseWetTemp;
            } else {
                Real64 GradientDividend = state.dataEnvrn->SiteTempGradient * DataEnvironment::EarthRadius * Z;
                Real64 GradientDivisor = DataEnvironment::EarthRadius + Z;
                state.dataSurface->SurfOutDryBulbTemp(SurfNum) = BaseDryTemp - GradientDividend / GradientDivisor;
                state.dataSurface->SurfOutWetBulbTemp(SurfNum) = BaseWetTemp - GradientDividend / GradientDivisor;
            }
        }
    }
}

void CheckSurfaceOutBulbTempAt(EnergyPlusData &state)
{
    // Using/Aliasing
    using DataEnvironment::SetOutBulbTempAt_error;

    Real64 minBulb = 0.0;
    for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; SurfNum++) {
        minBulb = min(minBulb, state.dataSurface->SurfOutDryBulbTemp(SurfNum), state.dataSurface->SurfOutWetBulbTemp(SurfNum));
        if (minBulb < -100.0) {
            SetOutBulbTempAt_error(state, "Surface", state.dataSurface->Surface(SurfNum).Centroid.z, state.dataSurface->Surface(SurfNum).Name);
        }
    }
}

void SetSurfaceWindSpeedAt(EnergyPlusData &state)
{
    Real64 const fac(state.dataEnvrn->WindSpeed * state.dataEnvrn->WeatherFileWindModCoeff *
                     std::pow(state.dataEnvrn->SiteWindBLHeight, -state.dataEnvrn->SiteWindExp));
    if (state.dataEnvrn->SiteWindExp == 0.0) {
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; SurfNum++) {
            state.dataSurface->SurfOutWindSpeed(SurfNum) = state.dataEnvrn->WindSpeed;
        }
    } else {

        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; SurfNum++) {
            if (!state.dataSurface->Surface(SurfNum).ExtWind) {
                continue;
            }
            Real64 const Z(state.dataSurface->Surface(SurfNum).Centroid.z); // Centroid value
            if (Z <= 0.0) {
                state.dataSurface->SurfOutWindSpeed(SurfNum) = 0.0;
            } else {
                //  [Met] - at meterological Station, Height of measurement is usually 10m above ground
                //  LocalWindSpeed = Windspeed [Met] * (Wind Boundary LayerThickness [Met]/Height [Met])**Wind Exponent[Met] &
                //                     * (Height above ground / Site Wind Boundary Layer Thickness) ** Site Wind Exponent
                state.dataSurface->SurfOutWindSpeed(SurfNum) = fac * std::pow(Z, state.dataEnvrn->SiteWindExp);
            }
        }
    }
}

void SetSurfaceWindDirAt(EnergyPlusData &state)
{
    for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; SurfNum++) {
        state.dataSurface->SurfOutWindDir(SurfNum) = state.dataEnvrn->WindDir;
    }
}

std::string cSurfaceClass(SurfaceClass const ClassNo)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function returns a string based on class number.

    // Return value
    std::string ClassName;

    switch (ClassNo) {
    case SurfaceClass::Wall: {
        ClassName = "Wall";
    } break;
    case SurfaceClass::Floor: {
        ClassName = "Floor";
    } break;
    case SurfaceClass::Roof: {
        ClassName = "Roof";
    } break;
    case SurfaceClass::Window: {
        ClassName = "Window";
    } break;
    case SurfaceClass::GlassDoor: {
        ClassName = "Glass Door";
    } break;
    case SurfaceClass::Door: {
        ClassName = "Door";
    } break;
    case SurfaceClass::TDD_Dome: {
        ClassName = "TubularDaylightDome";
    } break;
    case SurfaceClass::TDD_Diffuser: {
        ClassName = "TubularDaylightDiffuser";
    } break;
    case SurfaceClass::IntMass: {
        ClassName = "Internal Mass";
    } break;
    case SurfaceClass::Shading: {
        ClassName = "Shading";
    } break;
    case SurfaceClass::Detached_B: {
        ClassName = "Detached Shading:Building";
    } break;
    case SurfaceClass::Detached_F: {
        ClassName = "Detached Shading:Fixed";
    } break;
    default: {
        ClassName = "Invalid/Unknown";
    } break;
    }

    return ClassName;
}
Real64 AbsFrontSide(EnergyPlusData &state, int SurfNum)
{
    Real64 AbsorptanceFromExteriorFrontSide =
        (state.dataSurface->SurfWinExtBeamAbsByShade(SurfNum) + state.dataSurface->SurfWinExtDiffAbsByShade(SurfNum)) *
        state.dataSurface->SurfWinShadeAbsFacFace1(SurfNum);
    Real64 AbsorptanceFromInteriorFrontSide =
        (state.dataSurface->SurfWinIntBeamAbsByShade(SurfNum) + state.dataSurface->SurfWinIntSWAbsByShade(SurfNum)) *
        state.dataSurface->SurfWinShadeAbsFacFace2(SurfNum);
    return AbsorptanceFromExteriorFrontSide + AbsorptanceFromInteriorFrontSide;
}

Real64 AbsBackSide(EnergyPlusData &state, int SurfNum)
{
    Real64 AbsorptanceFromInteriorBackSide =
        (state.dataSurface->SurfWinIntBeamAbsByShade(SurfNum) + state.dataSurface->SurfWinIntSWAbsByShade(SurfNum)) *
        state.dataSurface->SurfWinShadeAbsFacFace1(SurfNum);
    Real64 AbsorptanceFromExteriorBackSide =
        (state.dataSurface->SurfWinExtBeamAbsByShade(SurfNum) + state.dataSurface->SurfWinExtDiffAbsByShade(SurfNum)) *
        state.dataSurface->SurfWinShadeAbsFacFace2(SurfNum);
    return AbsorptanceFromExteriorBackSide + AbsorptanceFromInteriorBackSide;
}

void GetVariableAbsorptanceSurfaceList(EnergyPlusData &state)
{
    if (!state.dataMaterial->AnyVariableAbsorptance) {
        return;
    }
    for (int surfNum : state.dataSurface->AllHTSurfaceList) {
        auto const &thisSurface = state.dataSurface->Surface(surfNum);
        auto const &thisConstruct = state.dataConstruction->Construct(thisSurface.Construction);
        if (thisConstruct.TotLayers == 0) {
            continue;
        }
        if (thisConstruct.LayerPoint(1) == 0) {
            continue; // error finding material number
        }
        auto const *mat = state.dataMaterial->materials(thisConstruct.LayerPoint(1));
        if (mat->group != Material::Group::Regular) {
            continue;
        }

        if (mat->absorpVarCtrlSignal != Material::VariableAbsCtrlSignal::Invalid) {
            // check for dynamic coating defined on interior surface
            if (thisSurface.ExtBoundCond != ExternalEnvironment) {
                ShowWarningError(state,
                                 format("MaterialProperty:VariableAbsorptance defined on an interior surface, {}. This VariableAbsorptance property "
                                        "will be ignored here",
                                        thisSurface.Name));
            } else {
                state.dataSurface->AllVaryAbsOpaqSurfaceList.push_back(surfNum);
            }
        }
    }
    // check for dynamic coating defined on the non-outside layer of a construction
    for (int ConstrNum = 1; ConstrNum <= state.dataHeatBal->TotConstructs; ++ConstrNum) {
        auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
        for (int Layer = 2; Layer <= thisConstruct.TotLayers; ++Layer) {
            auto const *mat = state.dataMaterial->materials(thisConstruct.LayerPoint(Layer));
            if (mat->group != Material::Group::Regular) {
                continue;
            }
            if (mat->absorpVarCtrlSignal != Material::VariableAbsCtrlSignal::Invalid) {
                ShowWarningError(state,
                                 format("MaterialProperty:VariableAbsorptance defined on a inside-layer materials, {}. This VariableAbsorptance "
                                        "property will be ignored here",
                                        mat->Name));
            }
        }
    }
} // GetVariableAbsorptanceSurfaceList()

Compass4 AzimuthToCompass4(Real64 azimuth)
{
    assert(azimuth >= 0.0 && azimuth < 360.0);
    for (int c4 = 0; c4 < static_cast<int>(Compass4::Num); ++c4) {
        Real64 lo = Compass4AzimuthLo[c4];
        Real64 hi = Compass4AzimuthHi[c4];
        if (lo > hi) {
            if (azimuth >= lo || azimuth < hi) {
                return static_cast<Compass4>(c4);
            }
        } else {
            if (azimuth >= lo && azimuth < hi) {
                return static_cast<Compass4>(c4);
            }
        }
    }
    assert(false);
    return Compass4::Invalid;
}

Compass8 AzimuthToCompass8(Real64 azimuth)
{
    assert(azimuth >= 0.0 && azimuth < 360.0);
    for (int c8 = 0; c8 < static_cast<int>(Compass8::Num); ++c8) {
        Real64 lo = Compass8AzimuthLo[c8];
        Real64 hi = Compass8AzimuthHi[c8];
        if (lo > hi) {
            if (azimuth >= lo || azimuth < hi) {
                return static_cast<Compass8>(c8);
            }
        } else {
            if (azimuth >= lo && azimuth < hi) {
                return static_cast<Compass8>(c8);
            }
        }
    }
    assert(false);
    return Compass8::Invalid;
}

} // namespace EnergyPlus::DataSurfaces
