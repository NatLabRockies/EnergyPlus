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

#ifndef DataSurfaces_hh_INCLUDED
#define DataSurfaces_hh_INCLUDED

// C++ Headers
#include <cstddef>
#include <unordered_map>
#include <vector>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Vector4.fwd.hh>

using ObjexxFCL::Vector4;

// EnergyPlus Headers
#include <EnergyPlus/ConvectionConstants.hh>
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataBSDFWindow.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataVectorTypes.hh>
#include <EnergyPlus/DataWindowEquivalentLayer.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/Shape.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DataSurfaces {

    // Using/Aliasing
    using DataBSDFWindow::BSDFWindowDescript;
    using DataVectorTypes::Vector;

    // Not sure this is the right module for this stuff, may move it later
    enum class Compass4
    {
        Invalid = -1,
        North,
        East,
        South,
        West,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(Compass4::Num)> compass4Names = {"North", "East", "South", "West"};

    constexpr std::array<Real64, static_cast<int>(Compass4::Num)> Compass4AzimuthLo = {315.0, 45.0, 135.0, 225.0};
    constexpr std::array<Real64, static_cast<int>(Compass4::Num)> Compass4AzimuthHi = {45.0, 135.0, 225.0, 315.0};

    Compass4 AzimuthToCompass4(Real64 azimuth);

    enum class Compass8
    {
        Invalid = -1,
        North,
        NorthEast,
        East,
        SouthEast,
        South,
        SouthWest,
        West,
        NorthWest,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(Compass8::Num)> compass8Names = {
        "North", "Northeast", "East", "Southeast", "South", "Southwest", "West", "Northwest"};

    // There is a bug here, the azimuth that divides West from
    // NorthWest is 292.5 not 287.5.  Keeping it like this temporarily
    // to minimize diffs.
    constexpr std::array<Real64, static_cast<int>(Compass8::Num)> Compass8AzimuthLo = {337.5, 22.5, 67.5, 112.5, 157.5, 202.5, 247.5, 292.5};
    constexpr std::array<Real64, static_cast<int>(Compass8::Num)> Compass8AzimuthHi = {22.5, 67.5, 112.5, 157.5, 202.5, 247.5, 292.5, 337.5};

    Compass8 AzimuthToCompass8(Real64 azimuth);

    // Parameters to indicate surface shape for use with the Surface
    // derived type (see below):
    enum class SurfaceShape : int
    {
        // TODO: enum check
        Invalid = -1,
        None,
        Triangle,
        Quadrilateral,
        Rectangle,
        RectangularDoorWindow,
        RectangularOverhang,
        RectangularLeftFin,
        RectangularRightFin,
        TriangularWindow,
        TriangularDoor,
        Polygonal,
        Num
    };

    enum class SurfaceClass : int
    {
        Invalid = -1, // If any addition classes get added to this list, add appropriate data
        None,         // to ComputeNominalUwithConvCoeffs in DataHeatBalance.cc and make sure
        Wall,         // that the data aligns with any revision to this enum.
        Floor,
        Roof,
        IntMass,
        Detached_B,
        Detached_F,
        Window,
        GlassDoor,
        Door,
        Shading,
        Overhang,
        Fin,
        TDD_Dome,
        TDD_Diffuser,
        Num // The counter representing the total number of surface class, always stays at the bottom
    };

    // A coarse grain version of SurfaceClass
    enum class FWC
    {
        Invalid = -1,
        Floor,
        Wall,
        Ceiling,
        Num
    };

    int constexpr iFWC_Floor = (int)FWC::Floor;
    int constexpr iFWC_Wall = (int)FWC::Wall;
    int constexpr iFWC_Ceiling = (int)FWC::Ceiling;

    enum class SurfaceFilter
    {
        Invalid = -1,
        AllExteriorSurfaces,
        AllExteriorWindows,
        AllExteriorWalls,
        AllExteriorRoofs,
        AllExteriorFloors,
        AllInteriorSurfaces,
        AllInteriorWindows,
        AllInteriorWalls,
        AllInteriorRoofs,
        AllInteriorCeilings,
        AllInteriorFloors,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(SurfaceFilter::Num)> SurfaceFilterNamesUC = {"ALLEXTERIORSURFACES",
                                                                                                         "ALLEXTERIORWINDOWS",
                                                                                                         "ALLEXTERIORWALLS",
                                                                                                         "ALLEXTERIORROOFS",
                                                                                                         "ALLEXTERIORFLOORS",
                                                                                                         "ALLINTERIORSURFACES",
                                                                                                         "ALLINTERIORWINDOWS",
                                                                                                         "ALLINTERIORWALLS",
                                                                                                         "ALLINTERIORROOFS",
                                                                                                         "ALLINTERIORCEILINGS",
                                                                                                         "ALLINTERIORFLOORS"};

    enum class WinCover
    {
        Invalid = -1,
        Bare,
        Shaded,
        Num
    };

    constexpr int iWinCover_Bare = (int)WinCover::Bare;
    constexpr int iWinCover_Shaded = (int)WinCover::Shaded;

    enum class WinShadingType
    {
        Invalid = -1,
        NoShade,
        ShadeOff,
        IntShade,
        SwitchableGlazing,
        ExtShade,
        ExtScreen,
        IntBlind,
        ExtBlind,
        BGShade,
        BGBlind,
        IntShadeConditionallyOff,
        GlassConditionallyLightened,
        ExtShadeConditionallyOff,
        IntBlindConditionallyOff,
        ExtBlindConditionallyOff,
        BGShadeConditionallyOff,
        BGBlindConditionallyOff,
        Num
    }; // Valid window shading types: IntShade <= Type <= BGBlind; the rest are shading status

    enum class WindowShadingControlType
    {
        Invalid = -1,
        AlwaysOn,                       // "ALWAYSON",
        AlwaysOff,                      // "ALWAYSOFF",
        OnIfScheduled,                  // "ONIFSCHEDULEALLOWS",
        HiSolar,                        // "ONIFHIGHSOLARONWINDOW",
        HiHorzSolar,                    // "ONIFHIGHHORIZONTALSOLAR",
        HiOutAirTemp,                   // "ONIFHIGHOUTDOORAIRTEMPERATURE",
        HiZoneAirTemp,                  // "ONIFHIGHZONEAIRTEMPERATURE",
        HiZoneCooling,                  // "ONIFHIGHZONECOOLING",
        HiGlare,                        // "ONIFHIGHGLARE",
        MeetDaylIlumSetp,               // "MEETDAYLIGHTILLUMINANCESETPOINT",
        OnNightLoOutTemp_OffDay,        // "ONNIGHTIFLOWOUTDOORTEMPANDOFFDAY",
        OnNightLoInTemp_OffDay,         // "ONNIGHTIFLOWINSIDETEMPANDOFFDAY",
        OnNightIfHeating_OffDay,        // "ONNIGHTIFHEATINGANDOFFDAY",
        OnNightLoOutTemp_OnDayCooling,  // "ONNIGHTIFLOWOUTDOORTEMPANDONDAYIFCOOLING",
        OnNightIfHeating_OnDayCooling,  // "ONNIGHTIFHEATINGANDONDAYIFCOOLING",
        OffNight_OnDay_HiSolarWindow,   // "OFFNIGHTANDONDAYIFCOOLINGANDHIGHSOLARONWINDOW",
        OnNight_OnDay_HiSolarWindow,    // "ONNIGHTANDONDAYIFCOOLINGANDHIGHSOLARONWINDOW",
        OnHiOutTemp_HiSolarWindow,      // "ONIFHIGHOUTDOORAIRTEMPANDHIGHSOLARONWINDOW",
        OnHiOutTemp_HiHorzSolar,        // "ONIFHIGHOUTDOORAIRTEMPANDHIGHHORIZONTALSOLAR",
        OnHiZoneTemp_HiSolarWindow,     // "ONIFHIGHZONEAIRTEMPANDHIGHSOLARONWINDOW",
        OnHiZoneTemp_HiHorzSolar,       // "ONIFHIGHZONEAIRTEMPANDHIGHHORIZONTALSOLAR",
        HiSolar_HiLumin_OffMidNight,    // "ONIFHIGHSOLARANDHIGHLUMINOFFMIDNIGHT",
        HiSolar_HiLumin_OffSunset,      // "ONIFHIGHSOLARANDHIGHLUMINOFFSUNSET",
        HiSolar_HiLumin_OffNextMorning, // "ONIFHIGHSOLARANDHIGHLUMINOFFNEXTMORNING"};
        Num
    };

    enum RefAirTemp // Parameters to indicate reference air temperatures for inside surface temperature calculations
    {
        Invalid = -1,
        ZoneMeanAirTemp,   // mean air temperature of the zone => MAT
        AdjacentAirTemp,   // air temperature adjacent to surface => TempEffBulkAir
        ZoneSupplyAirTemp, // supply air temperature of the zone
        Num
    };

    constexpr std::array<int, static_cast<int>(DataSurfaces::RefAirTemp::Num)> SurfTAirRefReportVals = {1, 2, 3};

    // Parameters to indicate exterior boundary conditions for use with
    // the Surface derived type (see below):
    // Note:  Positive values correspond to an interzone adjacent surface
    constexpr int ExternalEnvironment(0);
    constexpr int Ground(-1);
    constexpr int OtherSideCoefNoCalcExt(-2);
    constexpr int OtherSideCoefCalcExt(-3);
    constexpr int OtherSideCondModeledExt(-4);
    constexpr int GroundFCfactorMethod(-5);
    constexpr int KivaFoundation(-6);

    extern Array1D_string const cExtBoundCondition;

    // Parameters to indicate the first "corner" of a surface
    // Currently, these are used only during input of surfaces
    // They are here in order to facilitate later use in shading setup/calculations.
    constexpr int UpperLeftCorner(1);
    constexpr int LowerLeftCorner(2);
    constexpr int LowerRightCorner(3);
    constexpr int UpperRightCorner(4);

    constexpr int AltAngStepsForSolReflCalc(10); // Number of steps in altitude angle for solar reflection calc
    constexpr int AzimAngStepsForSolReflCalc(9); // Number of steps in azimuth angle of solar reflection calc

    // Parameters to indicate surface classes
    // Surface Class (FLOOR, WALL, ROOF (incl's CEILING), WINDOW, DOOR, GLASSDOOR,
    // SHADING (includes OVERHANG, WING), DETACHED, INTMASS),
    // TDD:DOME, TDD:DIFFUSER (for tubular daylighting device)
    // (Note: GLASSDOOR and TDD:DIFFUSER get overwritten as WINDOW
    // in SurfaceGeometry.cc, SurfaceWindow%OriginalClass holds the true value)
    // why aren't these sequential

    enum class HeatTransferModel
    {
        Invalid = -1,
        None, // shading surfaces
        CTF,
        EMPD,
        CondFD,
        HAMT,
        Window5,             // original detailed layer-by-layer based on window 4 and window 5
        ComplexFenestration, // BSDF
        TDD,                 // tubular daylighting device
        Kiva,                // Kiva ground calculations
        AirBoundaryNoHT,     // Construction:AirBoundary - not IRT or interior window
        Num                  // count, always the final element
    };

    constexpr std::array<std::string_view, static_cast<int>(DataSurfaces::HeatTransferModel::Num)> HeatTransAlgoStrs = {
        "None",
        "CTF - ConductionTransferFunction",
        "EMPD - MoisturePenetrationDepthConductionTransferFunction",
        "CondFD - ConductionFiniteDifference",
        "HAMT - CombinedHeatAndMoistureFiniteElement",
        "Window5 Detailed Fenestration",
        "Window7 Complex Fenestration",
        "Tubular Daylighting Device",
        "KivaFoundation - TwoDimensionalFiniteDifference",
        "Air Boundary - No Heat Transfer"};

    // Daylighting illuminance components
    enum class Lum
    {
        Invalid = -1,
        Illum,
        Back,
        Source,
        Num
    };

    // Trying out this new pattern to make code look less gnarly
    constexpr int iLum_Illum = (int)Lum::Illum;
    constexpr int iLum_Back = (int)Lum::Back;
    constexpr int iLum_Source = (int)Lum::Source;

    // IS_SHADED is the flag to indicate window has no shading device or shading device is off, and no daylight glare control
    // original expression: SHADE_FLAG == ShadeOff || SHADE_FLAG == ShadeOff
    constexpr bool NOT_SHADED(WinShadingType const ShadingFlag)
    {
        return (ShadingFlag == WinShadingType::NoShade || ShadingFlag == WinShadingType::ShadeOff);
    }

    // IS_SHADED is the flag to indicate window has shade on or temporarily off but may be triggered on later to control daylight glare
    // original expression: SHADE_FLAG > ShadeOff
    constexpr bool IS_SHADED(WinShadingType const ShadingFlag)
    {
        return !NOT_SHADED(ShadingFlag);
    }

    // IS_SHADED_NO_GLARE is the flag to indicate window has shade and no daylight glare control
    // original expression: IntShade <= SHADE_FLAG <= BGBlind
    constexpr bool IS_SHADED_NO_GLARE_CTRL(WinShadingType const ShadingFlag)
    {
        return (ShadingFlag == WinShadingType::IntShade || ShadingFlag == WinShadingType::SwitchableGlazing ||
                ShadingFlag == WinShadingType::ExtShade || ShadingFlag == WinShadingType::ExtScreen || ShadingFlag == WinShadingType::IntBlind ||
                ShadingFlag == WinShadingType::ExtBlind || ShadingFlag == WinShadingType::BGShade || ShadingFlag == WinShadingType::BGBlind);
    }

    // ANY_SHADE: if SHADE_FLAG is any of the shading types including interior, exterior or between glass shades
    constexpr bool ANY_SHADE(WinShadingType const ShadingFlag)
    {
        return (ShadingFlag == WinShadingType::IntShade || ShadingFlag == WinShadingType::ExtShade || ShadingFlag == WinShadingType::BGShade);
    }

    constexpr bool ANY_SHADE_SCREEN(WinShadingType const ShadingFlag)
    {
        return (ShadingFlag == WinShadingType::IntShade || ShadingFlag == WinShadingType::ExtShade || ShadingFlag == WinShadingType::BGShade ||
                ShadingFlag == WinShadingType::ExtScreen);
    }

    constexpr bool ANY_BLIND(WinShadingType const ShadingFlag)
    {
        return (ShadingFlag == WinShadingType::IntBlind || ShadingFlag == WinShadingType::ExtBlind || ShadingFlag == WinShadingType::BGBlind);
    }

    constexpr bool ANY_INTERIOR_SHADE_BLIND(WinShadingType const ShadingFlag)
    {
        return (ShadingFlag == WinShadingType::IntShade || ShadingFlag == WinShadingType::IntBlind);
    }

    constexpr bool ANY_EXTERIOR_SHADE_BLIND_SCREEN(WinShadingType const ShadingFlag)
    {
        return (ShadingFlag == WinShadingType::ExtShade || ShadingFlag == WinShadingType::ExtBlind || ShadingFlag == WinShadingType::ExtScreen);
    }

    constexpr bool ANY_BETWEENGLASS_SHADE_BLIND(WinShadingType const ShadingFlag)
    {
        return (ShadingFlag == WinShadingType::BGShade || ShadingFlag == WinShadingType::BGBlind);
    }

    // WindowShadingControl Slat Angle Control for Blinds
    enum class SlatAngleControl
    {
        Invalid = -1,
        Fixed,
        Scheduled,
        BlockBeamSolar,
        Num
    };

    // Parameters for air flow window source
    enum class WindowAirFlowSource
    {
        Invalid = -1,
        Indoor,
        Outdoor,
        Num
    };

    // Parameters for air flow window destination
    enum class WindowAirFlowDestination
    {
        Invalid = -1,
        Indoor,
        Outdoor,
        Return,
        Num
    };

    // Parameters for air flow window control
    enum class WindowAirFlowControlType
    {
        Invalid = -1,
        MaxFlow,
        AlwaysOff,
        Schedule,
        Num
    };

    // Parameters for window model selection
    enum class WindowModel
    {
        Invalid = -1,
        Detailed, // indicates original winkelmann window 5 implementation
        BSDF,     // indicates complex fenestration window 6 implementation
        EQL,      // indicates equivalent layer window model implementation
        Num
    };

    // Parameters for PierceSurface
    constexpr std::size_t nVerticesBig(20); // Number of convex surface vertices at which to switch to PierceSurface O( log N ) method

    // Y Slab for Surface2D for PierceSurface support of Nonconvex and Many-Vertex Surfaces
    struct Surface2DSlab
    {

    public: // Types
        using Vertex = ObjexxFCL::Vector2<Real64>;
        using Vertices = ObjexxFCL::Array1D<Vertex>;
        using Edge = Vertices::size_type; // The Surface2D vertex and edge index
        using EdgeXY = Real64;            // The edge x/y inverse slope
        using Edges = std::vector<Edge>;
        using EdgesXY = std::vector<EdgeXY>;

    public: // Creation
            // Constructor
        Surface2DSlab(Real64 const yl, Real64 const yu) : xl(0.0), xu(0.0), yl(yl), yu(yu)
        {
        }

    public:              // Data
        Real64 xl, xu;   // Lower and upper x coordinates of slab bounding box
        Real64 yl, yu;   // Lower and upper y coordinates of slab
        Edges edges;     // Left-to-right ordered edges crossing the slab
        EdgesXY edgesXY; // Edge x/y inverse slopes

    }; // Surface2DSlab

    // Projected 2D Surface Representation for Fast Computational Geometry Operations
    struct Surface2D
    {

    public: // Types
        using Vector2D = Vector2<Real64>;
        using Edge = Vector2D;
        using Vertices = Array1D<Vector2D>;
        using Vectors = Array1D<Vector2D>;
        using Edges = Vectors;
        using Slab = Surface2DSlab;
        using Slabs = std::vector<Surface2DSlab>;
        using SlabYs = std::vector<Real64>;
        using size_type = Vertices::size_type;

    public: // Creation
        // Default constructor
        Surface2D()
        {
        }

        // Constructor
        Surface2D(ShapeCat const shapeCat, int const axis, Vertices const &v, Vector2D const &vl, Vector2D const &vu);

    public: // Predicates
            // Bounding box contains a point?
        bool bb_contains(Vector2D const &v) const
        {
            return (vl.x <= v.x) && (v.x <= vu.x) && (vl.y <= v.y) && (v.y <= vu.y);
        }

    public: // Comparison
            // Equality
        friend bool operator==(Surface2D const &a, Surface2D const &b)
        {
            auto const &v1 = a.vertices;
            auto const &v2 = b.vertices;
            return eq(v1, v2);
        }

        // Inequality
        friend bool operator!=(Surface2D const &a, Surface2D const &b)
        {
            return !(a == b);
        }

    public:                                              // Data
        int axis = 0;                                    // Axis of projection (0=x, 1=y, 2=z)
        Vertices vertices;                               // Vertices
        Vector2D vl = Vector2D(0.0), vu = Vector2D(0.0); // Bounding box lower and upper corner vertices
        Vectors edges;                                   // Edge vectors around the vertices
        Real64 s1 = 0.0, s3 = 0.0;                       // Rectangle side widths squared
        SlabYs slabYs;                                   // Y coordinates of slabs
        Slabs slabs;                                     // Y slice slabs for fast nonconvex and many vertex intersections

    }; // Surface2D

    struct SurfaceCalcHashKey
    {
        // Values that must be the same in order for surfaces to use a representative calculation

        int Construction = 0;          // Pointer to the construction in the Construct derived type
        Real64 Azimuth = 0.0;          // Direction the surface outward normal faces (degrees) or FACING
        Real64 Tilt = 0.0;             // Angle (deg) between the ground outward normal and the surface outward normal
        Real64 Height = 0.0;           // Height of the surface (m)
        int Zone = 0;                  // Interior environment or zone the surface is a part of
        int EnclIndex = 0;             // Pointer to enclosure this surface belongs to
        int TAirRef = 0;               // Flag for reference air temperature
        int ExtZone = 0;               // For an "interzone" surface, this is the adjacent ZONE number (not adjacent SURFACE number).
        int ExtCond = 0;               // Exterior condition type. Same as ExtBoundCond for non-interzone surfaces. Value = 1 for interzone surfaces.
        int ExtEnclIndex = 0;          // For an "interzone" surface, this is the adjacent ENCLOSURE number
        bool ExtSolar = false;         // True if the "outside" of the surface is exposed to solar
        bool ExtWind = false;          // True if the "outside" of the surface is exposed to wind
        Real64 ViewFactorGround = 0.0; // View factor to the ground from the exterior of the surface for diffuse solar radiation
        Real64 ViewFactorSky = 0.0;    // View factor to the sky from the exterior of the surface for diffuse solar radiation
        Real64 ViewFactorSrdSurfs = 0.0; // View factor to the surrounding surfaces seen from the exterior of the surface

        // Special Properties
        HeatTransferModel HeatTransferAlgorithm = HeatTransferModel::CTF; // used for surface-specific heat transfer algorithm.
        Convect::HcInt intConvModel = Convect::HcInt::Invalid;            // Interior convection algorithm
        int intConvUserModelNum = 0;                                      // Interior convection user coefficient index
        Convect::HcExt extConvModel = Convect::HcExt::Invalid;            // Exterior convection algorithm
        int extConvUserModelNum = 0;                                      // Exterior convection user coefficient index
        int OSCPtr = 0;                                                   // Pointer to OSC data structure
        int OSCMPtr = 0;                                                  // "Pointer" to OSCM data structure (other side conditions from a model)

        // Windows
        int FrameDivider = 0;          // Pointer to frame and divider information (windows only)
        int SurfWinStormWinConstr = 0; // Construction with storm window (windows only)
        //   Airflow control                      // Not supported
        //   Shading Control                      // Not supported

        // Other special boundary conditions
        //   SolarIncidentInside                  // Not supported
        int MaterialMovInsulExt = 0;                           // Pointer to the material used for exterior movable insulation
        int MaterialMovInsulInt = 0;                           // Pointer to the material used for interior movable insulation
        int movInsulExtSchedNum = Sched::SchedNum_Invalid;     // Schedule for exterior movable insulation
        int movInsulIntSchedNum = Sched::SchedNum_Invalid;     // Schedule for interior movable insulation
        int externalShadingSchedNum = Sched::SchedNum_Invalid; // Schedule for a the external shading
        int SurroundingSurfacesNum = 0; // Index of a surrounding surfaces list (defined in SurfaceProperties::SurroundingSurfaces)
        int LinkedOutAirNode = 0;       // Index of the an OutdoorAir:Node
        int outsideHeatSourceTermSchedNum = Sched::SchedNum_Invalid; // schedule of additional source of heat flux rate applied to the outside surface
        int insideHeatSourceTermSchedNum = Sched::SchedNum_Invalid;  // schedule of additional source of heat flux rate applied to the inside surface

        // based on boost::hash_combine
        std::size_t hash_combine(std::size_t current_hash, std::size_t new_hash) const
        {
            current_hash ^= new_hash + 0x9e3779b9 + (current_hash << 6) + (current_hash >> 2);
            return current_hash;
        }

        std::vector<std::size_t> get_hash_list() const
        {
            using std::hash;

            return {hash<int>()(Construction),
                    hash<Real64>()(Azimuth),
                    hash<Real64>()(Tilt),
                    hash<Real64>()(Height),
                    hash<int>()(Zone),
                    hash<int>()(EnclIndex),
                    hash<int>()(TAirRef),
                    hash<int>()(ExtZone),
                    hash<int>()(ExtCond),
                    hash<int>()(ExtEnclIndex),
                    hash<bool>()(ExtSolar),
                    hash<bool>()(ExtWind),
                    hash<Real64>()(ViewFactorGround),
                    hash<Real64>()(ViewFactorSky),

                    hash<HeatTransferModel>()(HeatTransferAlgorithm),
                    hash<Convect::HcInt>()(intConvModel),
                    hash<Convect::HcExt>()(extConvModel),
                    hash<int>()(intConvUserModelNum),
                    hash<int>()(extConvUserModelNum),
                    hash<int>()(OSCPtr),
                    hash<int>()(OSCMPtr),

                    hash<int>()(FrameDivider),
                    hash<int>()(SurfWinStormWinConstr),

                    hash<int>()(MaterialMovInsulExt),
                    hash<int>()(MaterialMovInsulInt),
                    hash<int>()(movInsulExtSchedNum),
                    hash<int>()(movInsulIntSchedNum),
                    hash<int>()(externalShadingSchedNum),
                    hash<int>()(SurroundingSurfacesNum),
                    hash<int>()(LinkedOutAirNode),
                    hash<int>()(outsideHeatSourceTermSchedNum),
                    hash<int>()(insideHeatSourceTermSchedNum)};
        }

        std::size_t get_hash() const
        {
            auto hash_list = get_hash_list();
            std::size_t combined_hash = 0u;
            for (auto hash : hash_list) {
                combined_hash = hash_combine(combined_hash, hash);
            }
            return combined_hash;
        }

        bool operator==(const SurfaceCalcHashKey &other) const
        {
            return (Construction == other.Construction && Azimuth == other.Azimuth && Tilt == other.Tilt && Height == other.Height &&
                    Zone == other.Zone && EnclIndex == other.EnclIndex && ExtZone == other.ExtZone && ExtCond == other.ExtCond &&
                    ExtEnclIndex == other.ExtEnclIndex && ExtSolar == other.ExtSolar && ExtWind == other.ExtWind &&
                    ViewFactorGround == other.ViewFactorGround && ViewFactorSky == other.ViewFactorSky &&

                    HeatTransferAlgorithm == other.HeatTransferAlgorithm && intConvModel == other.intConvModel &&
                    intConvUserModelNum == other.intConvUserModelNum && extConvUserModelNum == other.extConvUserModelNum &&
                    extConvModel == other.extConvModel && OSCPtr == other.OSCPtr && OSCMPtr == other.OSCMPtr &&

                    FrameDivider == other.FrameDivider && SurfWinStormWinConstr == other.SurfWinStormWinConstr &&

                    MaterialMovInsulExt == other.MaterialMovInsulExt && MaterialMovInsulInt == other.MaterialMovInsulInt &&
                    movInsulExtSchedNum == other.movInsulExtSchedNum && movInsulIntSchedNum == other.movInsulIntSchedNum &&
                    externalShadingSchedNum == other.externalShadingSchedNum && SurroundingSurfacesNum == other.SurroundingSurfacesNum &&
                    LinkedOutAirNode == other.LinkedOutAirNode && outsideHeatSourceTermSchedNum == other.outsideHeatSourceTermSchedNum &&
                    insideHeatSourceTermSchedNum == other.insideHeatSourceTermSchedNum);
        }
    };

    struct SurfaceCalcHasher
    {
        std::size_t operator()(const SurfaceCalcHashKey &key) const
        {
            return key.get_hash();
        }
    };

    struct SurfaceData
    {

        // Types
        using Vertices = Array1D<Vector>;
        using Plane = Vector4<Real64>;

        // Members
        std::string Name; // User supplied name of the surface (must be unique)
        int Construction; // Pointer to the construction in the Construct derived type

        int RepresentativeCalcSurfNum; // Index of the surface that is used to calculate the heat balance for this surface. Equal to this surfaces
                                       // index when not using representative surface calculations.

        std::vector<int> ConstituentSurfaceNums; // A vector of surface numbers which reference this surface for representative calculations
        int ConstructionStoredInputValue;        // holds the original value for Construction per surface input
        SurfaceClass Class;
        SurfaceClass OriginalClass;

        // Geometry related parameters
        SurfaceShape Shape;       // Surface shape (Triangle=1,Quadrilateral=2,Rectangle=3,
                                  // Rectangular Window/Door=4,Rectangular Overhang=5,
                                  // Rectangular Left Fin=6,Rectangular Right Fin=7,
                                  // Triangular Window=8)
        int Sides;                // Number of side/vertices for this surface (based on Shape)
        Real64 Area;              // Surface area of the surface (less any subsurfaces) {m2}
        Real64 GrossArea;         // Surface area of the surface (including subsurfaces) {m2}
        Real64 NetAreaShadowCalc; // Area of a wall/floor/ceiling less subsurfaces assuming all windows, if present, have unity multiplier.
                                  // Wall/floor/ceiling/roof areas that include windows include frame (unity) areas.
                                  // Areas of Windows including divider (unity) area.
                                  // These areas are used in shadowing / sunlit area calculations.
        Real64 Perimeter;         // Perimeter length of the surface {m}
        Real64 Azimuth;           // Direction the surface outward normal faces (degrees) or FACING
        Real64 Height;            // Height of the surface (m)
        Real64 Reveal;            // Depth of the window reveal (m) if this surface is a window
        Real64 Tilt;              // Angle (deg) between the ground outward normal and the surface outward normal
        Real64 Width;             // Width of the surface (m)

        // Precomputed parameters for PierceSurface performance
        ShapeCat shapeCat;   // Shape category
        Plane plane;         // Plane
        Surface2D surface2d; // 2D projected surface for efficient intersection testing

        // Vertices
        Array1D<Vector> NewVertex;
        Vertices Vertex;          // Surface Vertices are represented by Number of Sides and Vector (type)
        Vector3<Real64> Centroid; // computed centroid (also known as center of mass or surface balance point)
        Vector3<Real64> lcsx;
        Vector3<Real64> lcsy;
        Vector3<Real64> lcsz;
        Vector3<Real64> NewellAreaVector;
        Vector3<Real64> NewellSurfaceNormalVector; // same as OutNormVec in vector notation
        Vector3<Real64> OutNormVec;                // Direction cosines (outward normal vector) for surface
        Real64 SinAzim;                            // Sine of surface azimuth angle
        Real64 CosAzim;                            // Cosine of surface azimuth angle
        Real64 SinTilt;                            // Sine of surface tilt angle
        Real64 CosTilt;                            // Cosine of surface tilt angle
        bool IsConvex;                             // true if the surface is convex.
        bool IsDegenerate;                         // true if the surface is degenerate.
        bool VerticesProcessed;                    // true if vertices have been processed (only used for base surfaces)
        Real64 XShift;                             // relative coordinate shift data - used by child subsurfaces
        Real64 YShift;                             // relative coordinate shift data - used by child subsurfaces

        // Boundary conditions and interconnections
        bool HeatTransSurf;                                    // True if surface is a heat transfer surface (light shelf can also be IsShadowing)
        Sched::Schedule *outsideHeatSourceTermSched = nullptr; // Schedule of additional source of heat flux rate applied to the outside surface
        Sched::Schedule *insideHeatSourceTermSched = nullptr;  // Schedule of additional source of heat flux rate applied to the inside surface
                                                               // False if a (detached) shadowing (sub)surface
        HeatTransferModel HeatTransferAlgorithm;               // used for surface-specific heat transfer algorithm.
        std::string BaseSurfName;                              // Name of BaseSurf
        int BaseSurf;       // "Base surface" for this surface. Applies mainly to subsurfaces in which case it points back to the base surface number.
                            // Equals 0 for detached shading. BaseSurf equals surface number for all other surfaces.
        int NumSubSurfaces; // Number of subsurfaces this surface has (doors/windows)
        std::string ZoneName;         // User supplied name of the Zone
        int Zone;                     // Interior environment or zone the surface is a part of
                                      // Note that though attached shading surfaces are part of a zone, this
                                      // value is 0 there to facilitate using them as detached surfaces (more accurate shading.
        int spaceNum;                 // Space the surface is part of
        std::string ExtBoundCondName; // Name for the Outside Environment Object
        int ExtBoundCond;             // For an "interzone" surface, this is the adjacent surface number.
                                      // for an internal/adiabatic surface this is the current surface number.
                                      // Otherwise, 0=external environment, -1=ground,
                                      // -2=other side coefficients (OSC--won't always use CTFs)
                                      // -3=other side conditions model
                                      // During input, interim values of UnreconciledZoneSurface ("Surface") and
                                      // UnenteredAdjacentZoneSurface ("Zone") are used until reconciled.
        bool ExtSolar;                // True if the "outside" of the surface is exposed to solar
        bool ExtWind;                 // True if the "outside" of the surface is exposed to wind Heat transfer coefficients
        bool hasIncSolMultiplier;     // Whether the surface has a incident solar multiplier
        Real64 IncSolMultiplier;      // Incident solar multiplier, overwritten by user input in SurfaceProperty:IncidentSolarMultiplier
        Real64 ViewFactorGround;      // View factor to the ground from the exterior of the surface for diffuse solar radiation
        Real64 ViewFactorSky;         // View factor to the sky from the exterior of the surface for diffuse solar radiation
        Real64 ViewFactorGroundIR;    // View factor to the ground and shadowing surfaces from the exterior of the surface for IR radiation
        Real64 ViewFactorSkyIR; // View factor to the sky from the exterior of the surface for IR radiation Special/optional other side coefficients
                                // (OSC)
        int OSCPtr;             // Pointer to OSC data structure
        int OSCMPtr;            // "Pointer" to OSCM data structure (other side conditions from a model)
        bool MirroredSurf;      // True if it is a mirrored surface
        bool IsShadowing;       // True if a surface is a shadowing surface (light shelf can also be HeatTransSurf)
        bool IsShadowPossibleObstruction; // True if a surface can be an exterior obstruction

        // Optional parameters specific to shadowing surfaces and subsurfaces (detached shading, overhangs, wings, etc.)
        Sched::Schedule *shadowSurfSched = nullptr; // Schedule for a shadowing (sub)surface
        bool IsTransparent;                         // True if the schedule values are always 1.0 (or the minimum is 1.0)
        Real64 SchedMinValue;                       // Schedule minimum value.

        // Window Parameters (when surface is Window)
        int activeWindowShadingControl;            // Active window shading control (windows only)
        std::vector<int> windowShadingControlList; // List of possible window shading controls
        bool HasShadeControl;                      // True if the surface is listed in a WindowShadingControl object
        int activeShadedConstruction;              // The currently active shaded construction (windows only)
        int activeShadedConstructionPrev;          // The currently active shaded construction (windows only)
        std::vector<int> shadedConstructionList;   // List of shaded constructions that correspond with window shading controls (windows only - same
                                                   // indexes as windowShadingControlList)
        std::vector<int> shadedStormWinConstructionList; // List of shaded constructions with storm window that correspond with window shading
                                                         // controls (windows only - same indexes as windowShadingControlList)
        int FrameDivider;                                // Pointer to frame and divider information (windows only)
        Real64 Multiplier;                               // Multiplies glazed area, frame area and divider area (windows only)

        // Air boundaries and spaces
        int RadEnclIndex = 0;   // Pointer to raidant enclosure this surface belongs to
        int SolarEnclIndex;     // Pointer to solar enclosure this surface belongs to
        int SolarEnclSurfIndex; //  Pointer to solar enclosure surface data, EnclSolInfo(n).SurfacePtr(SolarEnclSurfIndex) points to this surface
        bool IsAirBoundarySurf; // True if surface is an air boundary surface (Construction:AirBoundary)

        Convect::SurfOrientation convOrientation = Convect::SurfOrientation::Invalid; // Surface orientation for convection calculations

        SurfaceCalcHashKey calcHashKey;        // Hash key used for determining if this surface requires unique calculations.
        bool IsSurfPropertyGndSurfacesDefined; // true if ground surfaces properties are listed for an external surface
        int SurfPropertyGndSurfIndex;          // index to a ground surfaces list (defined in SurfaceProperties::GroundSurfaces)
        bool UseSurfPropertyGndSurfTemp;       // true if at least one ground surface temperature schedules is specified
        bool UseSurfPropertyGndSurfRefl;       // true if at least one ground surfaces reflectance schedule is specified
        Real64 GndReflSolarRad;                // ground surface reflected solar radiation on exterior surfaces
        bool SurfHasSurroundingSurfProperty;   // true if surrounding surfaces properties are listed for an external surface
        bool SurfSchedExternalShadingFrac;     // true if the external shading is scheduled or calculated externally to be imported
        int SurfSurroundingSurfacesNum;        // Index of a surrounding surfaces list (defined in SurfaceProperties::SurroundingSurfaces)
        Sched::Schedule *surfExternalShadingSched = nullptr; // Schedule for a the external shading
        int SurfLinkedOutAirNode;                            // Index of the an OutdoorAir:Node, zero if none
        Real64 AE = 0.0;                                     // Product of area and emissivity for each surface
        Real64 enclAESum = 0.0;                              // Sum of area times emissivity for all other surfaces in enclosure
        Real64 SrdSurfTemp;                                  // surrounding surfaces average temperature seen by an exterior surface (C)
        Real64 ViewFactorSrdSurfs;                           // surrounding surfaces view factor sum seen by an exterior surface(-)

        // Default Constructor
        SurfaceData()
            : Construction(0), RepresentativeCalcSurfNum(-1), ConstructionStoredInputValue(0), Class(SurfaceClass::None), OriginalClass(),
              Shape(SurfaceShape::None), Sides(0), Area(0.0), GrossArea(0.0), NetAreaShadowCalc(0.0), Perimeter(0.0), Azimuth(0.0), Height(0.0),
              Reveal(0.0), Tilt(0.0), Width(0.0), shapeCat(ShapeCat::Invalid), plane(0.0, 0.0, 0.0, 0.0), Centroid(0.0, 0.0, 0.0),
              lcsx(0.0, 0.0, 0.0), lcsy(0.0, 0.0, 0.0), lcsz(0.0, 0.0, 0.0), NewellAreaVector(0.0, 0.0, 0.0),
              NewellSurfaceNormalVector(0.0, 0.0, 0.0), OutNormVec(0.0, 0.0, 0.0), SinAzim(0.0), CosAzim(0.0), SinTilt(0.0), CosTilt(0.0),
              IsConvex(true), IsDegenerate(false), VerticesProcessed(false), XShift(0.0), YShift(0.0), HeatTransSurf(false),
              HeatTransferAlgorithm(HeatTransferModel::Invalid), BaseSurf(0), NumSubSurfaces(0), Zone(0), spaceNum(0), ExtBoundCond(0),
              ExtSolar(false), ExtWind(false), hasIncSolMultiplier(false), IncSolMultiplier(1.0), ViewFactorGround(0.0), ViewFactorSky(0.0),
              ViewFactorGroundIR(0.0), ViewFactorSkyIR(0.0), OSCPtr(0), OSCMPtr(0), MirroredSurf(false), IsShadowing(false),
              IsShadowPossibleObstruction(false), IsTransparent(false), SchedMinValue(0.0), activeWindowShadingControl(0), HasShadeControl(false),
              activeShadedConstruction(0), activeShadedConstructionPrev(0), FrameDivider(0), Multiplier(1.0), SolarEnclIndex(0),
              SolarEnclSurfIndex(0), IsAirBoundarySurf(false), IsSurfPropertyGndSurfacesDefined(false), SurfPropertyGndSurfIndex(0),
              UseSurfPropertyGndSurfTemp(false), UseSurfPropertyGndSurfRefl(false), GndReflSolarRad(0.0), SurfHasSurroundingSurfProperty(false),
              SurfSchedExternalShadingFrac(false), SurfSurroundingSurfacesNum(0), SurfLinkedOutAirNode(0), SrdSurfTemp(0.0), ViewFactorSrdSurfs(0.0)
        {
        }

    public: // Methods
            // Set Precomputed Parameters
        void set_computed_geometry();

        Real64 getInsideAirTemperature(EnergyPlusData &state, const int t_SurfNum) const;

        Real64 getOutsideAirTemperature(EnergyPlusData &state, int t_SurfNum) const;

        Real64 getOutsideIR(EnergyPlusData &state, int t_SurfNum) const;

        static Real64 getSWIncident(EnergyPlusData &state, int t_SurfNum);

        int getTotLayers(EnergyPlusData &state) const;

        Real64 get_average_height(EnergyPlusData &state) const;

        void make_hash_key(EnergyPlusData &state, const int SurfNum);

        void set_representative_surface(EnergyPlusData &state, const int SurfNum);

    private: // Methods
             // Computed Shape Category
        ShapeCat computed_shapeCat() const;

        // Computed Plane
        Plane computed_plane() const;

        // Computed axis-projected 2D surface
        Surface2D computed_surface2d() const;
    };

    struct SurfaceWindowRefPt
    {
        Real64 solidAng = 0.0;    // Solid angle subtended by window from daylit ref points 1 and 2
        Real64 solidAngWtd = 0.0; // Solid angle subtended by window from ref pts weighted by glare pos factor
        std::array<std::array<Real64, (int)WinCover::Num>, (int)Lum::Num> lums = {{{0.0, 0.0}}};
        Real64 illumFromWinRep = 0.0; // Illuminance from window at reference point N [lux]
        Real64 lumWinRep = 0.0;       // Window luminance as viewed from reference point N [cd/m2]
    };

    struct SurfaceWindowCalc // Calculated window-related values
    {
        // Members
        Array1D<SurfaceWindowRefPt> refPts;

        Vector3<Real64> WinCenter = {0.0, 0.0, 0.0}; // X,Y,Z coordinates of window center point in building coord system

        Real64 theta = 0.0;           // Azimuth of window normal (rad)
        Real64 phi = 0.0;             // Altitude of window normal (rad)
        Real64 rhoCeilingWall = 0.0;  // Average interior reflectance seen by light moving up across horizontal plane thru center of window
        Real64 rhoFloorWall = 0.0;    // Same as above, but for light moving down
        Real64 fractionUpgoing = 0.0; // Fraction light entering window that goes upward

        Real64 glazedFrac = 1.0;       // (Glazed area)/(Glazed area + divider area)
        Real64 centerGlassArea = 0.0;  // Center of glass area (m2); area of glass where 1-D conduction dominates
        Real64 edgeGlassCorrFac = 1.0; // Correction factor to center-of-glass conductance to account for 2-D glass conduction thermal bridging
                                       // effects near frame and divider

        int screenNum = 0;         // Screen material number for a window with a screen
        Real64 lightWellEff = 1.0; // Light well efficiency (multiplier on exterior window vis trans due to light well losses)

        // What is 10 here?
        std::array<Real64, 10 + 1> thetaFace = {296.15}; // Face temperatures of window layers (K)

        // Multiplier on sunlit fraction due to shadowing of glass by
        // frame and divider outside projections
        std::array<Real64, (int)Constant::iHoursInDay + 1> OutProjSLFracMult = {1.0};
        // Multiplier on sunlit fraction due to shadowing of glass by
        // frame and divider inside and outside projections
        std::array<Real64, (int)Constant::iHoursInDay + 1> InOutProjSLFracMult = {1.0};

        // for shadowing of ground by building and obstructions [W/m2]
        // Enclosure inside surface area minus this surface and its
        // subsurfaces for floor/wall/ceiling (m2)
        std::array<Real64, (int)FWC::Num> EnclAreaMinusThisSurf = {0.0, 0.0, 0.0};
        // Enclosure product of inside surface area times vis
        // reflectance minus this surface and its subsurfaces, for
        // floor/wall/ceiling (m2)
        std::array<Real64, (int)FWC::Num> EnclAreaReflProdMinusThisSurf = {0.0, 0.0, 0.0};

        BSDFWindowDescript ComplexFen; // Data for complex fenestration, see DataBSDFWindow.cc for declaration
        bool hasShade = false;
        bool hasBlind = false;
        bool hasScreen = false;
    };

    struct SurfaceShade
    {
        struct
        {
            int matNum = 0;
            bool movableSlats = false;       // True if window has a blind with movable slats
            Real64 slatAng = 0.0;            // Slat angle this time step for window with blind on (radians)
            Real64 slatAngDeg = 0.0;         // Slat angle this time step for window with blind on (deg)
            bool slatAngDegEMSon = false;    // flag that indicate EMS system is actuating SlatAngThisTSDeg
            Real64 slatAngDegEMSValue = 0.0; // value that EMS sets for slat angle in degrees
            bool slatBlockBeam = false;      // True if blind slats block incident beam solar
            int slatAngIdxLo = -1;
            int slatAngIdxHi = -1;
            Real64 slatAngInterpFac = 0.0;
            Real64 profAng = 0.0;
            int profAngIdxLo = 0;
            int profAngIdxHi = 0;
            Real64 profAngInterpFac = 0.0;
            Real64 bmBmTrans = 0.0;
            Real64 airFlowPermeability = 0.0; // Blind air-flow permeability for calculation of convective flow in gap between blind and glass

            // Properties are profile-angle dependent
            Material::BlindTraAbsRef<Material::MaxProfAngs + 1> TAR;
        } blind;

        // Save these from the glass in case we need to recalculate blind properties
        struct
        {
            Real64 epsIR = 0.0;
            Real64 rhoIR = 0.0;
        } glass;

        Real64 effShadeEmi = 0.0; // Effective emissivity of interior blind or shade
        Real64 effGlassEmi = 0.0; // Effective emissivity of glass adjacent to interior blind or shade
    };

    struct SurfaceWindowFrameDiv
    {
    };

    enum class NfrcProductOptions : int
    {
        Invalid = -1,
        CasementDouble,
        CasementSingle,
        DualAction,
        Fixed,
        Garage,
        Greenhouse,
        HingedEscape,
        HorizontalSlider,
        Jal,
        Pivoted,
        ProjectingSingle,
        ProjectingDual,
        DoorSidelite,
        Skylight,
        SlidingPatioDoor,
        CurtainWall,
        SpandrelPanel,
        SideHingedDoor,
        DoorTransom,
        TropicalAwning,
        TubularDaylightingDevice,
        VerticalSlider,
        Num
    };

    enum class NfrcVisionType : int
    {
        Invalid = -1,
        Single,
        DualVertical,
        DualHorizontal,
        Num
    };

    enum class FrameDividerType : int
    {
        Invalid = -1,
        DividedLite,
        Suspended,
        Num
    };

    // Type of control order when multiple surfaces are referenced
    enum class MultiSurfaceControl
    {
        Invalid = -1,
        Sequential,
        Group,
        Num
    };

    struct FrameDividerProperties
    {
        // Members
        std::string Name;          // Name of frame/divider
        Real64 FrameWidth;         // Average width of frame in plane of window {m}
        Real64 FrameProjectionOut; // Distance normal to window between outside face of outer pane
        //  and outside of frame {m}
        Real64 FrameProjectionIn; // Distance normal to window between inside face of inner pane
        //  and inside of frame {m}
        Real64 FrameConductance;          // Effective conductance of frame (no air films) {W/m2-K}
        Real64 FrameEdgeWidth;            // default 2.5 in ! Width of glass edge region near frame {m}
        Real64 FrEdgeToCenterGlCondRatio; // Ratio of frame edge of glass conductance (without air films) to
        // center of glass conductance (without air films)
        Real64 FrameSolAbsorp;        // Solar absorptance of frame corrected for self-shading
        Real64 FrameVisAbsorp;        // Visible absorptance of frame corrected for self-shading
        Real64 FrameEmis;             // Thermal emissivity of frame
        FrameDividerType DividerType; // Type of divider {DividedLite or Suspended (between-glass}
        Real64 DividerWidth;          // Average width of divider in plane of window {m}
        int HorDividers;              // Number of horizontal dividers
        int VertDividers;             // Number of vertical dividers
        Real64 DividerProjectionOut;  // Distance normal to window between outside face of outer pane
        //  and outside of divider {m}
        Real64 DividerProjectionIn; // Distance normal to window between inside face of inner pane
        //  and inside of divider {m}
        Real64 DividerEdgeWidth;           // default 2.5 in ! Width of glass edge region near divider
        Real64 DividerConductance;         // Effective conductance of divider (no air films) {W/m2-K}
        Real64 DivEdgeToCenterGlCondRatio; // Ratio of divider edge of glass conductance (without air films) to
        // center of glass conductance (without air films)
        Real64 DividerSolAbsorp;                                   // Solar absorptance of divider corrected for self-shading
        Real64 DividerVisAbsorp;                                   // Visible absorptance of divider corrected for self-shading
        Real64 DividerEmis;                                        // Thermal emissivity of divider
        DataWindowEquivalentLayer::Orientation MullionOrientation; // Horizontal or Vertical; used only for windows with two glazing systems
        //  divided by a mullion; obtained from Window5 data file.
        NfrcProductOptions NfrcProductType; // NFRC Product Type for Assembly Calculations
        Real64 OutsideRevealSolAbs;         // Solar absorptance of outside reveal
        Real64 InsideSillDepth;             // Inside sill depth (m)
        Real64 InsideReveal;                // Inside reveal (m)
        Real64 InsideSillSolAbs;            // Solar absorptance of inside sill
        Real64 InsideRevealSolAbs;          // Solar absorptance of inside reveal

        // Default Constructor
        FrameDividerProperties()
            : FrameWidth(0.0), FrameProjectionOut(0.0), FrameProjectionIn(0.0), FrameConductance(0.0), FrameEdgeWidth(0.06355),
              FrEdgeToCenterGlCondRatio(1.0), FrameSolAbsorp(0.0), FrameVisAbsorp(0.0), FrameEmis(0.9), DividerType(FrameDividerType::DividedLite),
              DividerWidth(0.0), HorDividers(0), VertDividers(0), DividerProjectionOut(0.0), DividerProjectionIn(0.0), DividerEdgeWidth(0.06355),
              DividerConductance(0.0), DivEdgeToCenterGlCondRatio(1.0), DividerSolAbsorp(0.0), DividerVisAbsorp(0.0), DividerEmis(0.9),
              MullionOrientation(DataWindowEquivalentLayer::Orientation::Invalid), NfrcProductType(NfrcProductOptions::CurtainWall),
              OutsideRevealSolAbs(0.0), InsideSillDepth(0.0), InsideReveal(0.0), InsideSillSolAbs(0.0), InsideRevealSolAbs(0.0)
        {
        }
    };

    struct StormWindowData
    {
        // Members
        int BaseWindowNum;       // Surface number of associated exterior window
        int StormWinMaterialNum; // Material number of storm window glass
        Real64 StormWinDistance; // Distance between storm window glass and adjacent glass (m)
        int DateOn;              // Date (julian) storm window is put on
        int MonthOn;             // Month storm window is put on
        int DayOfMonthOn;        // Day of month storm window is put on
        int DateOff;             // Date (julian) storm window is taken off
        int MonthOff;            // Month storm window is taken off
        int DayOfMonthOff;       // Day of month storm window is taken off

        // Default Constructor
        StormWindowData()
            : BaseWindowNum(0), StormWinMaterialNum(0), StormWinDistance(0.0), DateOn(0), MonthOn(0), DayOfMonthOn(0), DateOff(0), MonthOff(0),
              DayOfMonthOff(0)
        {
        }
    };

    struct WindowShadingControlData
    {
        // Members
        std::string Name;                                    // User supplied name of this set of shading control data
        int ZoneIndex{0};                                    // number of the zone referenced
        int SequenceNumber{0};                               // Shading control sequence number
        WinShadingType ShadingType{WinShadingType::NoShade}; // Shading type (InteriorShade, SwitchableGlazing,
        //  CHARACTER(len=32) :: ShadingType    = ' ' ! Shading type (InteriorShade, SwitchableGlazing,
        //  ExteriorShade,InteriorBlind,ExteriorBlind,BetweenGlassShade,
        //  BetweenGlassBlind, or ExteriorScreen)
        int getInputShadedConstruction{0}; // Pointer to the shaded construction (for ShadingType=ExteriorScreen,InteriorShade,
        //  ExteriorShade,BetweenGlassShade,InteriorBlind,ExteriorBlind,BetweenGlassBlind;
        //  this must be a window construction with a screen, shade or blind layer)
        // this is only used during GetInput and should not be used during timestep calculations
        int ShadingDevice{0}; // Pointer to the material for the shading device (for ShadingType=InteriorShade,
        //  ExteriorShade,BetweenGlassShade,InteriorBlind,ExteriorBlind,BetweenGlassBlind,
        //  ExteriorScreen;
        //  this must be a Material:WindowShade, Material:WindowScreen, or Material:WindowBlind
        WindowShadingControlType shadingControlType{
            WindowShadingControlType::Invalid}; // Takes one of the following values that specifies type of shading control
        //  CHARACTER(len=60) :: ShadingControlType =' ' ! Takes one of the following values that specifies type of shading control
        // (control is active only when schedule value = 1; if no schedule
        // specified, schedule value defaults to 1)
        //  AlwaysOn: always shaded; not affected by schedule
        //  AlwaysOff: never shaded; not affected by schedule
        //  OnIfScheduleAllows: unshaded if sch val = 0, shaded if = 1
        //  OnIfHighSolarOnWindow: shaded if incident direct + diffuse > setpoint (W/m2 of window)
        //  OnIfHighHorizontalSolar: shaded if direct + diffuse horizontal solar > setpoint
        //   (W/m2 of ground)
        //  OnIfHighOutsideAirTemp: shaded if outside drybulb > setpoint (C)
        //  OnIfHighZoneAirTemp: shaded if previous time step zone temperature > setpoint (C)
        //  OnIfHighZoneCooling: shaded if previous time step zone cooling rate > setpoint (W)
        //  OnIfHighGlare: shaded if total daylight glare index at first daylighting reference point
        //   from all exterior windows in zone > maximum glare specified in daylighting
        //   input for zone.
        //  MeetDaylightIlluminanceSetpoint: shading is adjusted to just meet illuminance setpoint
        //   at first reference point (only for ShadingType=SwitchableGlazing)
        //       The following three controls are used primarily to reduce zone heating load. They
        //       can be used with any shading type but are most appropriate for opaque interior
        //       or exterior shades with a high insulating value ("opaque movable insulation").
        //  OnNightIfLowOutsideTemp/OffDay: shaded at night if outside temp < setpoint (C)
        //  OnNightIfLowInsideTemp/OffDay: shaded at night if previous time step zone air temp < setpoint (C)
        //  OnNightIfHeating/OffDay: shaded  at night if previous time step zone heating rate > setpoint (W)
        //       The following two controls are used to reduce zone heating and cooling loads.
        //       They can be used with any shading type but are most appropriate for translucent
        //       interior or exterior shades with a high insulating value ("translucent movable insulation")
        //  OnNightIfLowOutsideTemp/OnDayIfCooling: shaded at night if outside temp < setpoint (C);
        //                                         shaded daytime if prev. time step cooling rate > 0
        //  OnNightIfHeating/OnDayIfCooling: shaded at night if prev. time step heating rate > setpoint (W);
        //                                         shaded daytime if prev. time step cooling rate > 0
        //       The following two controls are used to reduce zone cooling load. They can be used
        //       with any shading type but are most appropriate for interior or exterior blinds, interior
        //       or exterior shades with low insulating value, or switchable glazing.
        //  OffNight/OnDayIfCoolingAndHighSolarOnWindow: shading off at night; shading on daytime if
        //                                         solar on window > setpoint (W/m2 of window) and
        //                                         prev. time step cooling rate > 0
        //  OnNight/OnDayIfCoolingAndHighSolarOnWindow: shading on at night; shading on daytime if
        //                                         solar on window > setpoint (W/m2 of window) and
        //                                         prev. time step cooling rate > 0
        Sched::Schedule *sched = nullptr; // schedule of 0 and 1 values: 0 => window is not shaded;
        //  1 => window is shaded if Type=Schedule or Type = ScheduleAnd...
        // and setpoint is exceeded.
        Real64 SetPoint{0.0}; // Control setpoint (dimension depends on Trigger:
        //  W/m2 of window area for solar on window,
        //  W/m2 of ground area for horizontal solar,
        //  deg C for air temp, W for zone heating and
        //  cooling rate). Not used for Shading Control Type =
        //  MeetDaylightIlluminanceSetpoint or OnIfHighGlare.
        Real64 SetPoint2{0.0}; // Second control setpoint for control types that take two setpoints.
        //   Dimension is deg C or W/m2.
        bool ShadingControlIsScheduled{false};     // True if shading control has a schedule
        bool GlareControlIsActive{false};          // True if shading control to reduce daylight glare is active
        Sched::Schedule *slatAngleSched = nullptr; // schedule of slat angle values between 0.0 and 180.0 degrees
        SlatAngleControl slatAngleControl{
            SlatAngleControl::Invalid}; // Takes one of the following values that specifies
                                        //  CHARACTER(len=32) :: slatAngleControlForBlinds = ' ' ! Takes one of the following values that specifies
                                        //  how slat angle is controlled in a blind when ShadingType =
                                        //  InteriorBlind, ExteriorBlind or BetweenGlassBlind.
                                        //  FixedSlatAngle: the slat angle is fixed at the constant value given in the
                                        //    associated Material:WindowBlind
                                        //  ScheduledSlatAngle: the slat angle in degrees between 1 and 180 is given
                                        //    by the schedule with index SlatAngleSchedule
                                        //  BlockBeamSolar: if beam solar is incident on the window, and a blind is on the
                                        //    window, the slat angle is adjusted to just block beam solar; otherwise the
                                        //    slat angle is set to the value given in the associated Material:WindowBlind.
        std::string DaylightingControlName; // string holding the Daylighting Control Object Name string
        int DaylightControlIndex{0};        // Pointer to the array of Daylighting Controls
        MultiSurfaceControl multiSurfaceControl{
            MultiSurfaceControl::Invalid};     // True if Group, False if Sequential - type of control order when multiple surfaces are referenced
        int FenestrationCount{0};              // count of fenestration references
        Array1D<std::string> FenestrationName; // string holding list of fenestration surfaces
        Array1D_int FenestrationIndex;         // Pointers to fenestration surfaces
    };

    struct OSCData
    {
        // Members
        std::string Name;                          // Name of OSC
        Real64 ConstTemp;                          // User selected constant temperature (degrees C)
        Real64 ConstTempCoef;                      // Coefficient modifying the user selected constant temperature
        Real64 ExtDryBulbCoef;                     // Coefficient modifying the external dry bulb temperature
        Real64 GroundTempCoef;                     // Coefficient modifying the ground temperature
        Real64 SurfFilmCoef;                       // Combined convective/radiative film coefficient if >0, else use other coefficients
        Real64 WindSpeedCoef;                      // Coefficient modifying the wind speed term (s/m)
        Real64 ZoneAirTempCoef;                    // Coefficient modifying the zone air temperature part of the equation
        std::string ConstTempScheduleName;         // Schedule name for scheduled outside temp
        Sched::Schedule *constTempSched = nullptr; // Index for scheduled outside temp.
        bool SinusoidalConstTempCoef;              // If true then ConstTempCoef varies by sine wave
        Real64 SinusoidPeriod;                     // period of sine wave variation  (hr)
        Real64 TPreviousCoef;                      // Coefficient modifying the OSC temp from the previous timestep (dimensionless)
        Real64 TOutsideSurfPast;                   // Outside surface temperature from previous timestep {C}
        Real64 MinTempLimit;                       // Minimum limit on OSC temp {deg C}
        Real64 MaxTempLimit;                       // Maximum limit on OSC temp {deg C}
        bool MinLimitPresent;                      // If TRUE then apply minimum limit on calculated OSC temp
        bool MaxLimitPresent;                      // If TRUE then apply maximum limit on calculated OSC temp
        Real64 OSCTempCalc;                        // Result of calculated temperature using OSC (degrees C)

        // Default Constructor
        OSCData()
            : ConstTemp(0.0), ConstTempCoef(0.0), ExtDryBulbCoef(0.0), GroundTempCoef(0.0), SurfFilmCoef(0.0), WindSpeedCoef(0.0),
              ZoneAirTempCoef(0.0), SinusoidalConstTempCoef(false), SinusoidPeriod(0.0), TPreviousCoef(0.0), TOutsideSurfPast(0.0), MinTempLimit(0.0),
              MaxTempLimit(0.0), MinLimitPresent(false), MaxLimitPresent(false), OSCTempCalc(0.0)
        {
        }
    };

    struct OSCMData
    {
        // Members
        std::string Name;             // Name of OSCM
        std::string Class;            // type of Model for OSCM
        Real64 TConv;                 // Temperature of bulk air at other side face (degrees C)
        bool EMSOverrideOnTConv;      // if true then EMS calling for convection bulk air temp override
        Real64 EMSOverrideTConvValue; // value for convection air temp when overridden
        Real64 HConv;                 // Convection coefficient (W/m2-K)
        bool EMSOverrideOnHConv;      // if true then EMS calling for convection coef override
        Real64 EMSOverrideHConvValue; // value to use for convection coef when overridden
        Real64 TRad;                  // Effective temperature of surfaces exposed to other side face (degrees C)
        bool EMSOverrideOnTRad;       // if true then EMS calling for radiation temp override
        Real64 EMSOverrideTRadValue;  // value to use for rad temp when overridden
        Real64 HRad;                  // Linearized Radiation coefficient (W/m2-K)
        bool EMSOverrideOnHrad;       // if true then EMS calling for radiation coef override
        Real64 EMSOverrideHradValue;  // value to use for rad coef when overridden

        // Default Constructor
        OSCMData()
            : TConv(20.0), EMSOverrideOnTConv(false), EMSOverrideTConvValue(0.0), HConv(4.0), EMSOverrideOnHConv(false), EMSOverrideHConvValue(0.0),
              TRad(20.0), EMSOverrideOnTRad(false), EMSOverrideTRadValue(0.0), HRad(4.0), EMSOverrideOnHrad(false), EMSOverrideHradValue(0.0)
        {
        }
    };

    struct ConvectionCoefficient
    {
        // Members
        int WhichSurface = 0;                // Which surface number this is applied to
        std::string SurfaceName;             // Which surface (name)
        Convect::OverrideType overrideType = // Override type, 1=value, 2=schedule, 3=model, 4=user curve
            Convect::OverrideType::Invalid;
        Real64 OverrideValue = 0.0;                            // User specified value
        Sched::Schedule *sched = nullptr;                      // if type="schedule" is used
        int UserCurveIndex = 0;                                // if type=UserCurve is used
        Convect::HcInt HcIntModelEq = Convect::HcInt::Invalid; // if type is one of specific model equations
        Convect::HcExt HcExtModelEq = Convect::HcExt::Invalid;
    };

    struct ShadingVertexData
    {
        // Members
        int NVert = 0;
        Array1D<Real64> XV;
        Array1D<Real64> YV;
        Array1D<Real64> ZV;

        // Default Constructor
        ShadingVertexData()
        {
        }
    };

    struct SurfaceSolarIncident
    {
        // Members
        std::string Name;
        int SurfPtr = 0;                  // surface pointer
        int ConstrPtr = 0;                // construction pointer
        Sched::Schedule *sched = nullptr; // schedule
    };

    struct SurfaceIncidentSolarMultiplier
    {
        // Members
        std::string Name;
        int SurfaceIdx = 0;               // surface index
        Real64 Scaler = 1.0;              // the constant multiplier constant from user input
        Sched::Schedule *sched = nullptr; // multiplier schedule
    };

    struct FenestrationSolarAbsorbed
    {
        // Members
        std::string Name;
        int SurfPtr;                       // surface pointer
        int ConstrPtr;                     // construction pointer
        int NumOfSched;                    // number of scheduled layers
        Array1D<Sched::Schedule *> scheds; // pointer to schedules for each layer in construction

        // Default Constructor
        FenestrationSolarAbsorbed() : SurfPtr(0), ConstrPtr(0), NumOfSched(0)
        {
        }
    };

    struct GroundSurfacesData
    {
        // Members
        std::string Name;                     // name of a ground surface
        Real64 ViewFactor = 0.0;              // view factor to a ground surface
        Sched::Schedule *tempSched = nullptr; // pointer to a ground surface temperature schedule object
        Sched::Schedule *reflSched = nullptr; // pointer to a ground Surface reflectance schedule object
    };

    struct GroundSurfacesProperty
    {
        // Members
        std::string Name;                     // name of multiple ground surfaces object
        int NumGndSurfs = 0;                  // number of groundSurfaces
        Array1D<GroundSurfacesData> GndSurfs; // ground surfaces data
        Real64 SurfsTempAvg = 0.0;            // ground Surfaces average temperature at each time step
        Real64 SurfsReflAvg = 0.0;            // ground Surfaces average reflectance at each time step
        Real64 SurfsViewFactorSum = 0.0;      // sum of view factors of ground surfaces seen by an exterior surface
        bool IsGroundViewFactorSet = false;   // true if the ground view factor field is not blank
    };

    struct SurfaceLocalEnvironment
    {
        // Members
        std::string Name;
        int SurfPtr = 0;                            // surface pointer
        Sched::Schedule *sunlitFracSched = nullptr; // schedule
        int SurroundingSurfsPtr = 0;                // schedule pointer
        int OutdoorAirNodePtr = 0;                  // outdoor air node pointer
        int GroundSurfsPtr = 0;                     // pointer to multiple ground surfaces object
    };

    struct SurroundingSurfProperty
    {
        // Members
        std::string Name;
        Real64 ViewFactor = 0.0;              // view factor to surrounding surface
        Sched::Schedule *tempSched = nullptr; // temperature schedule
    };

    struct SurroundingSurfacesProperty
    {
        // Members
        std::string Name;
        Real64 SkyViewFactor = 0.0;                 // sky view factor
        Real64 GroundViewFactor = 0.0;              // ground view factor
        Real64 SurfsViewFactorSum = 0.0;            // surrounding surfaces view factor sum
        Sched::Schedule *skyTempSched = nullptr;    // schedule
        Sched::Schedule *groundTempSched = nullptr; // schedule
        int TotSurroundingSurface = 0;              // Total number of surrounding surfaces defined for an exterior surface
        bool IsSkyViewFactorSet = false;            // false if the sky view factor field is blank
        bool IsGroundViewFactorSet = false;         // false if the ground view factor field is blank
        Array1D<SurroundingSurfProperty> SurroundingSurfs;
    };

    struct IntMassObject
    {
        // Members
        std::string Name;
        std::string ZoneOrZoneListName;   // zone or zone list name
        int ZoneOrZoneListPtr;            // pointer to a zone list
        int NumOfZones;                   // number of zones in a zone list
        int Construction;                 // pointer to construction object
        Real64 GrossArea;                 // internal surface area, [m2]
        bool ZoneListActive;              // flag to a list
        std::string spaceOrSpaceListName; // Space or Space list name
        int spaceOrSpaceListPtr;          // pointer to a Space list
        int numOfSpaces;                  // number of Spaces in a Space list
        bool spaceListActive;             // flag to a list

        // Default Constructor
        IntMassObject()
            : ZoneOrZoneListPtr(0), NumOfZones(0), Construction(0), GrossArea(0.0), ZoneListActive(false), spaceOrSpaceListPtr(0), numOfSpaces(0),
              spaceListActive(false)
        {
        }
    };

    // Surface interior convection
    struct SurfIntConv
    {

        // convection class determined by surface orientation,
        // heating/cooling system, and temperature regime
        Convect::IntConvClass convClass = Convect::IntConvClass::Invalid;
        int convClassRpt = (int)Convect::IntConvClass::Invalid;

        Convect::HcInt model = Convect::HcInt::SetByZone; // convection model
        int userModelNum = 0;                             // user defined convection model

        Convect::HcInt hcModelEq = Convect::HcInt::Invalid; // current convection model
        int hcModelEqRpt = (int)Convect::HcInt::Invalid;
        int hcUserCurveNum = 0;

        Real64 zoneWallHeight = 0.0; // geometry parameters
        Real64 zonePerimLength = 0.0;
        Real64 zoneHorizHydrDiam = 0.0;
        Real64 windowWallRatio = 0.0;
        Convect::IntConvWinLoc windowLocation = Convect::IntConvWinLoc::NotSet; // Already has NotSet defined as 0, and uses it in reporting. :(

        bool getsRadiantHeat = false;
        bool hasActiveInIt = false;
    };

    // Surface exterior convection
    struct SurfExtConv
    {
        // current classification for outside face wind regime and convection orientation
        Convect::ExtConvClass convClass = Convect::ExtConvClass::Invalid;
        int convClassRpt = (int)Convect::ExtConvClass::Invalid;

        Convect::HcExt model = Convect::HcExt::SetByZone; // conveciton model
        int userModelNum = 0;

        Convect::HcExt hfModelEq = Convect::HcExt::Invalid; // Current forced convection model
        int hfModelEqRpt = (int)Convect::HcExt::Invalid;
        int hfUserCurveNum = 0;

        Convect::HcExt hnModelEq = Convect::HcExt::Invalid; // Current natural convection model
        int hnModelEqRpt = (int)Convect::HcExt::Invalid;
        int hnUserCurveNum = 0;

        Real64 faceArea = 0.0; // Geometry parameters
        Real64 facePerimeter = 0.0;
        Real64 faceHeight = 0.0;
    };

    // Clears the global data in DataSurfaces.
    // Needed for unit tests, should not be normally called.
    // void clear_state() override;

    void SetSurfaceOutBulbTempAt(EnergyPlusData &state);

    void CheckSurfaceOutBulbTempAt(EnergyPlusData &state);

    void SetSurfaceWindSpeedAt(EnergyPlusData &state);

    void SetSurfaceWindDirAt(EnergyPlusData &state);

    Real64 AbsFrontSide(EnergyPlusData &state, int SurfNum);

    Real64 AbsBackSide(EnergyPlusData &state, int SurfNum);

    void GetVariableAbsorptanceSurfaceList(EnergyPlusData &state);

    std::string cSurfaceClass(SurfaceClass ClassNo);

    struct MovInsul
    {
        bool present = false;
        bool presentPrevTS = false;
        Real64 H = 0.0;
        int matNum = 0; // Material number
        Sched::Schedule *sched = nullptr;
    };

} // namespace DataSurfaces

struct SurfacesData : BaseGlobalStruct
{
    int TotSurfaces = 0;             // Total number of surfaces (walls, floors, roofs, windows, shading surfaces, etc.--everything)
    int TotWindows = 0;              // Total number of windows
    int TotStormWin = 0;             // Total number of storm window blocks
    int TotWinShadingControl = 0;    // Total number of window shading control blocks
    int TotUserIntConvModels = 0;    // Total number of interior convection coefficient (overrides) // TODO: Should just be a local variable I think
    int TotUserExtConvModels = 0;    // Total number of exterior convection coefficient (overrides) // TODO: Should just be a local variable I think
    int TotOSC = 0;                  // Total number of Other Side Coefficient Blocks
    int TotOSCM = 0;                 // Total number of Other Side Conditions Model Blocks.
    int TotExtVentCav = 0;           // Total number of ExteriorNaturalVentedCavity
    int TotSurfIncSolSSG = 0;        // Total number of scheduled surface gains for incident solar radiation on surface
    int TotSurfIncSolMultiplier = 0; // Total number of surfaces with incident solar multipliers
    int TotFenLayAbsSSG = 0;         // Total number of scheduled surface gains for absorbed solar radiation in window layers
    int TotSurfLocalEnv = 0;         // Total number of surface level outdoor air node.
    int TotSurfPropGndSurfs = 0;     // Total number of surface property ground surfaces object
    int Corner = 0;                  // Which corner is specified as the first vertex
    int MaxVerticesPerSurface = 4;   // Maximum number of vertices allowed for a single surface (default -- can go higher)
    int BuildingShadingCount = 0;    // Total number of Building External Shades
    int FixedShadingCount = 0;       // Total number of Fixed External Shades
    int AttachedShadingCount = 0;    // Total number of Shades attached to Zones
    int ShadingSurfaceFirst = 0;     // Start index of shading surfaces (Building External Shades, Fixed External Shades and Shades attached to Zone)
    int ShadingSurfaceLast = -1;     // End index of shading surfaces (Building External Shades, Fixed External Shades and Shades attached to Zone)
    bool AspectTransform = false;    // Set to true when GeometryTransform object is used
    bool CalcSolRefl = false;        // Set to true when Solar Reflection Calculations object is used
    bool CCW = false;                // True if vertices will be entered in CounterClockWise Order
    bool WorldCoordSystem = false;   // True if vertices will be "World Coordinates". False means relative coordinates
    bool DaylRefWorldCoordSystem = false; // True if Daylight Reference Point vertices will be "World Coordinates". False means relative coordinates
    int MaxRecPts = 0;                    // Max number of receiving points on a surface for solar reflection calc
    int MaxReflRays = 0;                  // Max number of rays from a receiving surface for solar reflection calc
    Real64 GroundLevelZ = 0.0;            // Z value of ground level for solar refl calc (m)
    bool AirflowWindows = false;          // TRUE if one or more airflow windows
    bool ShadingTransmittanceVaries = false;           // overall, shading transmittance varies for the building
    bool UseRepresentativeSurfaceCalculations = false; // Use Representative Surfaces for Calculations
    bool AnyMovableInsulation = false;                 // True if any movable insulation presents
    bool AnyMovableSlat = false;                       // True if there are any movable slats for window blinds presented

    Array1D_int SurfAdjacentZone; // Array of adjacent zones to each surface
    Array1D<Real64> X0;           // X-component of translation vector
    Array1D<Real64> Y0;           // Y-component of translation vector
    Array1D<Real64> Z0;           // Z-component of translation vector

    std::unordered_map<DataSurfaces::SurfaceCalcHashKey, int, DataSurfaces::SurfaceCalcHasher>
        RepresentativeSurfaceMap; // A map that categorizes similar surfaces with
                                  // a single representative surface index

    std::vector<int> AllHTSurfaceList;                 // List of all heat transfer surfaces
    std::vector<int> AllExtSolarSurfaceList;           // List of all exterior solar surfaces, all are heat transfer surfaces
    std::vector<int> AllExtSolAndShadingSurfaceList;   // List of all exterior solar surfaces plus all shading surfaces
    std::vector<int> AllShadowPossObstrSurfaceList;    // List of all IsShadoPossibleObstuction surfaces
    std::vector<int> AllIZSurfaceList;                 // List of all interzone heat transfer surfaces
    std::vector<int> AllHTNonWindowSurfaceList;        // List of all non-window heat transfer surfaces
    std::vector<int> AllHTWindowSurfaceList;           // List of all window surfaces
    std::vector<int> AllExtSolWindowSurfaceList;       // List of all exterior solar window surfaces
    std::vector<int> AllExtSolWinWithFrameSurfaceList; // List of all exterior solar window surfaces with a frame and divider
    std::vector<int> AllHTKivaSurfaceList;             // List of all Kiva foundation surfaces
    std::vector<int> AllSurfaceListReportOrder;        // List of all surfaces - output reporting order
    std::vector<int> AllVaryAbsOpaqSurfaceList;        // List of all opaque exterior surfaces with dynamic coating
    std::vector<int> allInsideSourceSurfaceList;       // List of all surfaces with SurfaceProperty:HeatBalanceSourceTerm for inside face
    std::vector<int> allOutsideSourceSurfaceList;      // List of all surfaces with SurfaceProperty:HeatBalanceSourceTerm for outside face
    std::vector<int> allGetsRadiantHeatSurfaceList;    // List of all surfaces that receive radiant HVAC output
    std::vector<int> intMovInsulSurfNums;
    std::vector<int> extMovInsulSurfNums;

    std::array<std::vector<int>, static_cast<int>(DataSurfaces::SurfaceFilter::Num)> SurfaceFilterLists;

    // Surface HB arrays
    Array1D<Real64> SurfOutDryBulbTemp; // Surface outside dry bulb air temperature, for surface heat balance (C)
    Array1D<Real64> SurfOutWetBulbTemp; // Surface outside wet bulb air temperature, for surface heat balance (C)
    Array1D<Real64> SurfOutWindSpeed;   // Surface outside wind speed, for surface heat balance (m/s)
    Array1D<Real64> SurfOutWindDir;     // Surface outside wind direction, for surface heat balance and ventilation(degree)
    Array1D<Real64> SurfGenericContam;  // [ppm] Surface generic contaminant as a storage term for
    Array1D<int> SurfLowTempErrCount;
    Array1D<int> SurfHighTempErrCount;

    // Surface solar arrays
    Array1D<Real64> SurfAirSkyRadSplit;        // Fractional split between the air and the sky for radiation from the surface
                                               // Fraction of sky IR coming from sky itself; 1-SurfAirSkyRadSplit comes from the atmosphere.
    Array1D<Vector3<Real64>> SurfSunCosHourly; // Hourly values of SUNCOS (solar direction cosines)
                                               // Autodesk: Init Zero-initialization added to avoid use uninitialized
    Array1D<Real64> SurfSunlitArea;            // Sunlit area by surface number
    Array1D<Real64> SurfSunlitFrac;            // Sunlit fraction by surface number
    Array1D<Real64> SurfSkySolarInc;           // Incident diffuse solar from sky; if CalcSolRefl is true, includes reflection of sky diffuse
                                               // and beam solar from exterior obstructions [W/m2]
    Array1D<Real64> SurfGndSolarInc;           // Incident diffuse solar from ground; if CalcSolRefl is true,
                                               // accounts for shadowing of ground by building and obstructions [W/m2]
    Array1D<Real64> SurfBmToBmReflFacObs;      // Factor for incident solar from specular beam refl from obstructions (W/m2)/(W/m2)
    Array1D<Real64> SurfBmToDiffReflFacObs;    // Factor for incident solar from diffuse beam refl from obstructions (W/m2)/(W/m2)
    Array1D<Real64> SurfBmToDiffReflFacGnd;    // Factor for incident solar from diffuse beam refl from ground
    Array1D<Real64> SurfSkyDiffReflFacGnd;     // sky diffuse reflection view factors from ground
    Array1D<Real64> SurfOpaqAI;                // Time step value of factor for beam absorbed on inside of opaque surface
    Array1D<Real64> SurfOpaqAO;                // Time step value of factor for beam absorbed on outside of opaque surface
    Array1D<int> SurfPenumbraID;

    // Surface reflectance
    Array2D<Real64> SurfReflFacBmToDiffSolObs;
    Array2D<Real64> SurfReflFacBmToDiffSolGnd;
    Array2D<Real64> SurfReflFacBmToBmSolObs;
    Array1D<Real64> SurfReflFacSkySolObs;
    Array1D<Real64> SurfReflFacSkySolGnd;
    Array2D<Real64> SurfCosIncAveBmToBmSolObs;

    // Surface parameters specific to solar reflection from surfaces
    Array1D<Real64> SurfShadowDiffuseSolRefl; // Diffuse solar reflectance of opaque portion
    Array1D<Real64> SurfShadowDiffuseVisRefl; // Diffuse visible reflectance of opaque portion
    Array1D<Real64> SurfShadowGlazingFrac;    // Glazing fraction
    Array1D<int> SurfShadowGlazingConstruct;  // Glazing construction number
    Array1D<int> SurfShadowRecSurfNum;        // Receiving surface number
    Array1D<std::vector<int>>
        SurfShadowDisabledZoneList; // Array of all disabled shadowing zone number to the current surface the surface diffusion model

    // Surface EMS
    Array1D<bool> SurfEMSConstructionOverrideON;          // if true, EMS is calling to override the construction value
    Array1D<int> SurfEMSConstructionOverrideValue;        // pointer value to use for Construction when overridden
    Array1D<bool> SurfEMSOverrideIntConvCoef;             // if true, EMS is calling to override the interior convection coefficient value
    Array1D<Real64> SurfEMSValueForIntConvCoef;           // Value EMS is calling to use for interior convection coefficient [W/m2-K]
    Array1D<bool> SurfEMSOverrideExtConvCoef;             // if true, EMS is calling to override the exterior convection coefficient value
    Array1D<Real64> SurfEMSValueForExtConvCoef;           // Value EMS is calling to use for exterior convection coefficient [W/m2-K]
    Array1D<bool> SurfOutDryBulbTempEMSOverrideOn;        // if true, EMS is calling to override the surface's outdoor air temp
    Array1D<Real64> SurfOutDryBulbTempEMSOverrideValue;   // value to use for EMS override of outdoor air drybulb temp (C)
    Array1D<bool> SurfOutWetBulbTempEMSOverrideOn;        // if true, EMS is calling to override the surface's outdoor wetbulb temp
    Array1D<Real64> SurfOutWetBulbTempEMSOverrideValue;   // value to use for EMS override of outdoor air wetbulb temp (C)
    Array1D<bool> SurfWindSpeedEMSOverrideOn;             //  if true, EMS is calling to override the surface's outdoor wind speed
    Array1D<Real64> SurfWindSpeedEMSOverrideValue;        // value to use for EMS override of outdoor wind speed (m/s)
    Array1D<bool> SurfViewFactorGroundEMSOverrideOn;      // if true, EMS is calling to override the surface's view factor to ground
    Array1D<Real64> SurfViewFactorGroundEMSOverrideValue; // value to use for EMS override of surface's view factor to ground
    Array1D<bool> SurfWindDirEMSOverrideOn;               // if true, EMS is calling to override the outside wind direction
    Array1D<Real64> SurfWindDirEMSOverrideValue;          // value to use for EMS override of outside wind direction (deg)

    // Surface Properties
    Array1D<int> SurfDaylightingShelfInd;        // Pointer to daylighting shelf
    Array1D<bool> SurfExtEcoRoof;                // True if the top outside construction material is of type Eco Roof
    Array1D<bool> SurfExtCavityPresent;          // true if there is an exterior vented cavity on surface
    Array1D<int> SurfExtCavNum;                  // index for this surface in ExtVentedCavity structure (if any)
    Array1D<bool> SurfIsPV;                      // true if this is a photovoltaic surface (dxf output)
    Array1D<bool> SurfIsICS;                     // true if this is an ICS collector
    Array1D<bool> SurfIsPool;                    // true if this is a pool
    Array1D<int> SurfICSPtr;                     // Index to ICS collector
    Array1D<bool> SurfIsRadSurfOrVentSlabOrPool; // surface cannot be part of both a radiant surface & ventilated slab group

    // Surface ConvCoeff Properties
    Array1D<int> SurfTAirRef;    // Flag for reference air temperature
    Array1D<int> SurfTAirRefRpt; // Flag for reference air temperature for reporting

    EPVector<DataSurfaces::SurfIntConv> surfIntConv;
    EPVector<DataSurfaces::SurfExtConv> surfExtConv;

    // Surface Window Heat Balance
    Array1D_int SurfWinInsideGlassCondensationFlag;   // 1 if innermost glass inside surface temp < zone air dew point;  0 otherwise
    Array1D_int SurfWinInsideFrameCondensationFlag;   // 1 if frame inside surface temp < zone air dew point; 0 otherwise
    Array1D_int SurfWinInsideDividerCondensationFlag; // 1 if divider inside surface temp < zone air dew point;  0 otherwise

    Array2D<Real64> SurfWinA;           // Time step value of factor for beam absorbed in window glass layers
    Array2D<Real64> SurfWinADiffFront;  // Time step value of factor for diffuse absorbed in window layers
    Array2D<Real64> SurfWinACFOverlap;  // Time step value of factor for beam absorbed in window glass layers which comes from other windows
                                        // It happens sometimes that beam enters one window and hits back of second window.
                                        // It is used in complex fenestration only
    Array1D<Real64> SurfWinTransSolar;  // Exterior beam plus diffuse solar transmitted through window, or window plus shade/blind, into zone (W)
    Array1D<Real64> SurfWinBmSolar;     // Exterior beam solar transmitted through window, or window plus blind, into zone (W)
    Array1D<Real64> SurfWinBmBmSolar;   // Exterior beam-to-beam solar transmitted through window, or window plus blind, into zone (W)
    Array1D<Real64> SurfWinBmDifSolar;  // Exterior beam-to-diffuse solar transmitted through window, or window plus blind, into zone (W)
    Array1D<Real64> SurfWinDifSolar;    // Exterior diffuse solar transmitted through window, or window plus shade/blind, into zone (W)
    Array1D<Real64> SurfWinHeatGain;    // Total heat gain from window (W) = WinTransSolar + (IR and convection from glazing, or,
                                        // if interior shade, IR and convection from zone-side of shade plus gap air convection to zone) +
                                        // (IR convection from frame) + (IR and convection from divider if no interior shade) (W)
                                        // minus SurfWinInitialDifSolInTrans minus SurfWinLossSWZoneToOutWinRep
    Array1D<Real64> SurfWinHeatGainRep; // Equals WinHeatGain when WinHeatGain >= 0.0 (W)
    Array1D<Real64> SurfWinHeatLossRep; // Equals -WinHeatGain when WinHeatGain < 0.0 (W)
    Array1D<Real64> SurfWinGainConvGlazToZoneRep;     // component of WinHeatGain convect to zone from glazing (W)
    Array1D<Real64> SurfWinGainIRGlazToZoneRep;       // component of WinHeatGain net IR to zone from glazing (W)
    Array1D<Real64> SurfWinLossSWZoneToOutWinRep;     // component of WinHeatGain shortwave transmit back out (W)
    Array1D<Real64> SurfWinGainFrameDividerToZoneRep; // component of WinHeatGain to zone from frame/divider (W)
    Array1D<Real64> SurfWinGainConvShadeToZoneRep;    // component of WinHeatGain convect to zone from front shade (W)
    Array1D<Real64> SurfWinGainIRShadeToZoneRep;      // component of WinHeatGain net IR to zone from front shade (W)
    Array1D<Real64> SurfWinGapConvHtFlowRep;          // Convective heat flow from gap in airflow window (W)
    Array1D<Real64> SurfWinShadingAbsorbedSolar;      // Exterior beam plus diffuse solar absorbed by window shading device (W)
    Array1D<Real64> SurfWinSysSolTransmittance;       // Effective solar transmittance of window + shading device, if present
    Array1D<Real64> SurfWinSysSolReflectance;         // Effective solar reflectance of window + shading device, if present
    Array1D<Real64> SurfWinSysSolAbsorptance;         // Effective solar absorptance of window + shading device, if present

    // Surface Window Energy
    Array1D<Real64> SurfWinTransSolarEnergy;           // Energy of WinTransSolar [J]
    Array1D<Real64> SurfWinBmSolarEnergy;              // Energy of WinBmSolar [J]
    Array1D<Real64> SurfWinBmBmSolarEnergy;            // Beam-to-beam energy of WinBmSolar [J]
    Array1D<Real64> SurfWinBmDifSolarEnergy;           // Beam-to-diffuse energy of WinBmSolar [J]
    Array1D<Real64> SurfWinDifSolarEnergy;             // Energy of WinDifSolar [J]
    Array1D<Real64> SurfWinHeatGainRepEnergy;          // Energy of WinHeatGainRep [J]
    Array1D<Real64> SurfWinHeatLossRepEnergy;          // Energy of WinHeatLossRep [J]
    Array1D<Real64> SurfWinShadingAbsorbedSolarEnergy; // Energy of WinShadingAbsorbedSolar [J]
    Array1D<Real64> SurfWinGapConvHtFlowRepEnergy;     // Energy of WinGapConvHtFlowRep [J]
    Array1D<Real64> SurfWinHeatTransferRepEnergy;      // Energy of WinHeatTransfer [J]
    Array1D<Real64> SurfWinIRfromParentZone;
    Array1D<Real64> SurfWinFrameQRadOutAbs;
    Array1D<Real64> SurfWinFrameQRadInAbs;
    Array1D<Real64> SurfWinDividerQRadOutAbs;
    Array1D<Real64> SurfWinDividerQRadInAbs;
    Array1D<Real64> SurfWinExtBeamAbsByShade;       // Exterior beam solar absorbed by window shade (W/m2)
    Array1D<Real64> SurfWinExtDiffAbsByShade;       // Exterior diffuse solar absorbed by window shade (W/m2)
    Array1D<Real64> SurfWinIntBeamAbsByShade;       // Interior beam solar absorbed by window shade (W/m2)
    Array1D<Real64> SurfWinIntSWAbsByShade;         // Interior diffuse solar plus short-wave from lights absorbed by window shade (W/m2)
    Array1D<Real64> SurfWinInitialDifSolAbsByShade; // Initial diffuse solar from ext and int windows absorbed by window shade (W/m2)
    Array1D<Real64> SurfWinIntLWAbsByShade;         // Interior long-wave from zone lights and equipment absorbed by window shade (W/m2)
    Array1D<Real64> SurfWinConvHeatFlowNatural;     // Convective heat flow from gap between glass and interior shade or blind (W)
    Array1D<Real64> SurfWinConvHeatGainToZoneAir;   // Convective heat gain to zone air from window gap airflow (W)
    Array1D<Real64> SurfWinRetHeatGainToZoneAir;    // Convective heat gain to return air sent to zone [W]
    Array1D<Real64> SurfWinDividerHeatGain;
    Array1D<Real64> SurfWinBlTsolBmBm;                 // Time-step value of blind beam-beam solar transmittance (-)
    Array1D<Real64> SurfWinBlTsolBmDif;                // Time-step value of blind beam-diffuse solar transmittance (-)
    Array1D<Real64> SurfWinBlTsolDifDif;               // Time-step value of blind diffuse-diffuse solar transmittance (-)
    Array1D<Real64> SurfWinBlGlSysTsolBmBm;            // Time-step value of blind/glass system beam-beam solar transmittance (-)
    Array1D<Real64> SurfWinBlGlSysTsolDifDif;          // Time-step value of blind/glass system diffuse-diffuse solar transmittance (-)
    Array1D<Real64> SurfWinScTsolBmBm;                 // Time-step value of screen beam-beam solar transmittance (-)
    Array1D<Real64> SurfWinScTsolBmDif;                // Time-step value of screen beam-diffuse solar transmittance (-)
    Array1D<Real64> SurfWinScTsolDifDif;               // Time-step value of screen diffuse-diffuse solar transmittance (-)
    Array1D<Real64> SurfWinScGlSysTsolBmBm;            // Time-step value of screen/glass system beam-beam solar transmittance (-)
    Array1D<Real64> SurfWinScGlSysTsolDifDif;          // Time-step value of screen/glass system diffuse-diffuse solar transmittance (-)
    Array1D<Real64> SurfWinGlTsolBmBm;                 // Time-step value of glass beam-beam solar transmittance (-)
    Array1D<Real64> SurfWinGlTsolBmDif;                // Time-step value of glass beam-diffuse solar transmittance (-)
    Array1D<Real64> SurfWinGlTsolDifDif;               // Time-step value of glass diffuse-diffuse solar transmittance (-)
    Array1D<Real64> SurfWinBmSolTransThruIntWinRep;    // Beam solar transmitted through interior window [W]
    Array1D<Real64> SurfWinBmSolAbsdOutsReveal;        // Multiplied by BeamSolarRad, gives beam solar absorbed by outside reveal surfaces (m2)
    Array1D<Real64> SurfWinBmSolRefldOutsRevealReport; // Beam solar reflected by outside reveal surfaces, for reporting (m2)
    Array1D<Real64> SurfWinBmSolAbsdInsReveal;         // Multiplied by BeamSolarRad, gives beam solar absorbed by inside reveal surfaces (m2)
    Array1D<Real64> SurfWinBmSolRefldInsReveal;        // Multiplied by BeamSolarRad, gives beam solar reflected by inside reveal surfaces (m2)
    Array1D<Real64> SurfWinBmSolRefldInsRevealReport;  // Beam solar reflected by inside reveal surfaces, for reporting (W)
    Array1D<Real64> SurfWinOutsRevealDiffOntoGlazing;  // Multiplied by BeamSolarRad, gives diffuse from beam reflection from outside reveal that is
                                                       // incident on the glazing per m2 of glazing (-)
    Array1D<Real64> SurfWinInsRevealDiffOntoGlazing;   // Multiplied by BeamSolarRad, gives diffuse from beam reflection from inside reveal that is
                                                       // incident on the glazing per m2 of glazing (-)
    Array1D<Real64> SurfWinInsRevealDiffIntoZone; // Multiplied by BeamSolarRad, gives diffuse from beam reflection from inside reveal that goes into
                                                  // zone directly or reflected from glazing (m2)
    Array1D<Real64> SurfWinOutsRevealDiffOntoFrame; // Multiplied by BeamSolarRad, gives diffuse from beam reflection from outside reveal that is
                                                    // incident on the outside of the frame per m2 of frame (-)
    Array1D<Real64> SurfWinInsRevealDiffOntoFrame;  // Multiplied by BeamSolarRad, gives diffuse from beam reflection from inside reveal that is
                                                    // incident on the outside of the frame per m2 of frame (-) for debugging CR 7596. TH 5/26/2009
    Array1D<Real64> SurfWinInsRevealDiffOntoGlazingReport; // Diffuse solar from beam reflection from inside reveal that is incident
                                                           // on the glazing (W)
    Array1D<Real64> SurfWinInsRevealDiffIntoZoneReport;   // Diffuse from beam reflection from inside reveal that goes into zone directly or reflected
                                                          // from glazing (W)
    Array1D<Real64> SurfWinInsRevealDiffOntoFrameReport;  // Diffuse from beam reflection from inside reveal that is incident on the frame (W)
    Array1D<Real64> SurfWinBmSolAbsdInsRevealReport;      // Beam solar absorbed by inside reveal (W)  energy
    Array1D<Real64> SurfWinBmSolTransThruIntWinRepEnergy; // energy of BmSolTransThruIntWinRep [J]
    Array1D<Real64> SurfWinBmSolRefldOutsRevealRepEnergy; // energy of BmSolRefldOutsRevealReport [J]
    Array1D<Real64> SurfWinBmSolRefldInsRevealRepEnergy;  // energy of BmSolRefldInsRevealReport [J]
    Array1D<Real64> SurfWinProfileAngHor;                 // Horizontal beam solar profile angle (degrees)
    Array1D<Real64> SurfWinProfileAngVert;                // Vertical beam solar profile angle (degrees)

    EPVector<DataSurfaces::WinShadingType> SurfWinShadingFlag; // -1: window has no shading device
    Array1D<bool> SurfWinShadingFlagEMSOn;                     // EMS control flag, true if EMS is controlling ShadingFlag with ShadingFlagEMSValue
    Array1D<int> SurfWinShadingFlagEMSValue;                   // EMS control value for Shading Flag
    Array1D<int> SurfWinStormWinFlag;                          // -1: Storm window not applicable;
                                                               // 0: Window has storm window but it is off
                                                               // 1: Window has storm window and it is on
    Array1D<int> SurfWinStormWinFlagPrevDay;                   // Previous time step value of StormWinFlag
    Array1D<Real64> SurfWinFracTimeShadingDeviceOn;            // For a single time step, = 0.0
                                                               // if no shading device or shading device is off = 1.0 if shading device is on;
    // For time intervals longer than a time step, = fraction of time that shading device is on.
    EPVector<DataSurfaces::WinShadingType> SurfWinExtIntShadePrevTS; // 1 if exterior or interior blind or shade in place previous time step;
                                                                     // 0 otherwise
    Array1D<bool> SurfWinHasShadeOrBlindLayer;                       // mark as true if the window construction has a shade or a blind layer
    Array1D<bool> SurfWinSurfDayLightInit;                           // surface has been initialized for following 5 arrays
    Array1D<int> SurfWinDaylFacPoint;                                // Pointer to daylight factors for the window
    Array1D<Real64> SurfWinVisTransSelected;                         // Window vis trans at normal incidence selected for use in dayltg calculation
    Array1D<Real64> SurfWinSwitchingFactor;                          // Window switching factor (0.0 = unswitched; 1.0 = fully switched)
    Array1D<Real64> SurfWinVisTransRatio;                            // For windows with switchable glazing,
                                                                     // ratio of normal transmittance in switched state to that in unswitched state
    Array1D<Real64> SurfWinFrameArea;                                // Frame projected area (m2)
    Array1D<Real64> SurfWinFrameConductance;                         // Frame conductance [no air films] (W/m2-K)
    Array1D<Real64> SurfWinFrameSolAbsorp;                           // Frame solar absorptance (assumed same inside and outside)
    Array1D<Real64> SurfWinFrameVisAbsorp;                           // Frame visible absorptance (assumed same inside and outside)
    Array1D<Real64> SurfWinFrameEmis;                 // Frame thermal emissivity (thermal absorptance) (assumed same inside and outside)
    Array1D<Real64> SurfWinFrEdgeToCenterGlCondRatio; // Ratio of frame edge of glass conductance (without air films) to center of glass conductance
                                                      // (without air films)
    Array1D<Real64> SurfWinFrameEdgeArea;             // Area of glass near frame (m2)
    Array1D<Real64> SurfWinFrameTempIn;               // Frame inside surface temperature (C)
    Array1D<Real64> SurfWinFrameTempInOld;            // Previous value of frame inside surface temperature (C)
    Array1D<Real64> SurfWinFrameTempSurfOut;          // Frame outside surface temperature (C)
    Array1D<Real64> SurfWinProjCorrFrOut;             // Correction factor to absorbed radiation due to frame outside projection
    Array1D<Real64> SurfWinProjCorrFrIn;              // Correction factor to absorbed radiation due to frame inside projection
    Array1D<DataSurfaces::FrameDividerType> SurfWinDividerType; // Divider type (1=DividedLite, 2=Suspended (between-pane))
    Array1D<Real64> SurfWinDividerArea;                         // Divider projected area (m2)
    Array1D<Real64> SurfWinDividerConductance;                  // Divider conductance [no air films] (W/m2-K)
    Array1D<Real64> SurfWinDividerSolAbsorp;                    // Divider solar absorptance (assumed same inside and outside)
    Array1D<Real64> SurfWinDividerVisAbsorp;                    // Divider visible absorptance (assumed same inside and outside)
    Array1D<Real64> SurfWinDividerEmis;                         // Divider thermal emissivity (thermal absorptance) (assumed same inside and outside)
    Array1D<Real64> SurfWinDivEdgeToCenterGlCondRatio;          // Ratio of divider edge of glass conductance (without air films) to center of glass
                                                                // conductance (without air films)
    Array1D<Real64> SurfWinDividerEdgeArea;                     // Area of glass near dividers (m2)
    Array1D<Real64> SurfWinDividerTempIn;                       // Divider inside surface temperature (C)
    Array1D<Real64> SurfWinDividerTempInOld;                    // Previous value of divider inside surface temperature (C)
    Array1D<Real64> SurfWinDividerTempSurfOut;                  // Divider outside surface temperature (C)
    Array1D<Real64> SurfWinProjCorrDivOut;                      // Correction factor to absorbed radiation due to divider outside projection
    Array1D<Real64> SurfWinProjCorrDivIn;                       // Correction factor to absorbed radiation due to divider inside projection
    Array1D<Real64> SurfWinShadeAbsFacFace1; // Fraction of short-wave radiation incident that is absorbed by face 1 when total absorbed radiation is
                                             // apportioned to the two faces
    Array1D<Real64> SurfWinShadeAbsFacFace2; // Fraction of short-wave radiation incident that is absorbed by face 2 when total absorbed radiation is
                                             // apportioned to the two faces
    Array1D<Real64> SurfWinConvCoeffWithShade; // Convection coefficient from glass or shade to gap air when interior
                                               // or exterior shade is present (W/m2-K)
    Array1D<Real64> SurfWinOtherConvHeatGain;  // other convective = total conv - standard model prediction for EQL window model (W)
    Array1D<Real64> SurfWinEffInsSurfTemp; // Effective inside surface temperature for window with interior blind or shade; combination of shade/blind
                                           // and glass temperatures (C)
    Array1D<Real64> SurfWinTotGlazingThickness; // Total glazing thickness from outside of outer glass to inside of inner glass (m)
    Array1D<Real64> SurfWinTanProfileAngHor;    // Tangent of horizontal profile angle
    Array1D<Real64> SurfWinTanProfileAngVert;   // Tangent of vertical profile angle
    Array1D<Real64> SurfWinInsideSillDepth;     // Depth of inside sill (m)
    Array1D<Real64> SurfWinInsideReveal;        // Depth of inside reveal (m)
    Array1D<Real64> SurfWinInsideSillSolAbs;    // Solar absorptance of inside sill
    Array1D<Real64> SurfWinInsideRevealSolAbs;  // Solar absorptance of inside reveal
    Array1D<Real64> SurfWinOutsideRevealSolAbs; // Solar absorptance of outside reveal
    Array1D<DataSurfaces::WindowAirFlowSource> SurfWinAirflowSource;           // Source of gap airflow (INSIDEAIR, OUTSIDEAIR, etc.)
    Array1D<DataSurfaces::WindowAirFlowDestination> SurfWinAirflowDestination; // Destination of gap airflow (INSIDEAIR, OUTSIDEAIR, etc.)
    Array1D<int> SurfWinAirflowReturnNodePtr;                                  // Return node pointer for destination = ReturnAir
    Array1D<Real64> SurfWinMaxAirflow;                                         // Maximum gap airflow (m3/s per m of glazing width)
    Array1D<DataSurfaces::WindowAirFlowControlType> SurfWinAirflowControlType; // Gap airflow control type (ALWAYSONATMAXFLOW, etc.)
    Array1D<bool> SurfWinAirflowHasSchedule;                                   // True if gap airflow is scheduled
    Array1D<Sched::Schedule *> SurfWinAirflowScheds;                           // Gap airflow schedule
    Array1D<Real64> SurfWinAirflowThisTS;                                      // Gap airflow this timestep (m3/s per m of glazing width)
    Array1D<Real64> SurfWinTAirflowGapOutlet;                                  // Temperature of air leaving airflow gap between glass panes (C)
    Array1D<int> SurfWinWindowCalcIterationsRep;                               // Number of iterations in window heat balance calculation
    Array1D<Real64> SurfWinVentingOpenFactorMultRep; // Window/door opening modulation multiplier on venting open factor, for reporting
    Array1D<Real64> SurfWinInsideTempForVentingRep;  // Inside air temp used to control window/door venting, for reporting (C)
    Array1D<Real64> SurfWinVentingAvailabilityRep;   // Venting availability schedule value (0.0/1.0 = no venting allowed/not allowed)
    Array1D<Real64> SurfWinSkyGndSolarInc; // Incident diffuse solar from ground-reflected sky radiation; used for Complex Fen; if CalcSolRefl is
                                           // true, accounts for shadowing of ground by building and obstructions [W/m2]
    Array1D<Real64> SurfWinBmGndSolarInc;  // Incident diffuse solar from ground-reflected beam radiation; used for Complex Fen; if CalcSolRefl is
                                           // true, accounts for shadowing of ground by building and obstructions [W/m2]
    Array1D<bool> SurfWinSolarDiffusing;   // True if exterior window with a construction that contains a diffusing glass layer
    Array1D<Real64> SurfWinFrameHeatGain;
    Array1D<Real64> SurfWinFrameHeatLoss;
    Array1D<Real64> SurfWinDividerHeatLoss;
    Array1D<Real64> SurfWinTCLayerTemp; // The temperature of the thermochromic layer of the window
    Array1D<Real64> SurfWinSpecTemp;    // The specification temperature of the TC layer glass Added for W6 integration June 2010
    Array1D<DataSurfaces::WindowModel> SurfWinWindowModelType; // if set to WindowBSDFModel, then uses BSDF methods
    Array1D<Real64> SurfWinTDDPipeNum;                         // Tubular daylighting device pipe number for TDD domes and diffusers
    Array1D<int> SurfWinStormWinConstr;                        // Construction with storm window (windows only)
    Array1D<int> SurfActiveConstruction;                       // The currently active construction with or without storm window
    Array1D<int> SurfWinActiveShadedConstruction;              // The currently active shaded construction with or without storm window (windows only)

    Array1D<DataSurfaces::MovInsul> intMovInsuls;
    Array1D<DataSurfaces::MovInsul> extMovInsuls;

    EPVector<DataSurfaces::SurfaceData> Surface;
    EPVector<DataSurfaces::SurfaceWindowCalc> SurfaceWindow;
    EPVector<DataSurfaces::SurfaceShade> surfShades;
    Array1D<DataSurfaces::FrameDividerProperties> FrameDivider;
    EPVector<DataSurfaces::StormWindowData> StormWindow;
    EPVector<DataSurfaces::WindowShadingControlData> WindowShadingControl;
    EPVector<DataSurfaces::OSCData> OSC;
    EPVector<DataSurfaces::OSCMData> OSCM;
    EPVector<DataSurfaces::ConvectionCoefficient> userIntConvModels;
    EPVector<DataSurfaces::ConvectionCoefficient> userExtConvModels;
    EPVector<DataSurfaces::ShadingVertexData> ShadeV;
    EPVector<DataSurfaces::SurfaceSolarIncident> SurfIncSolSSG;
    EPVector<DataSurfaces::SurfaceIncidentSolarMultiplier> SurfIncSolMultiplier;
    EPVector<DataSurfaces::FenestrationSolarAbsorbed> FenLayAbsSSG;
    EPVector<DataSurfaces::SurfaceLocalEnvironment> SurfLocalEnvironment;
    EPVector<DataSurfaces::SurroundingSurfacesProperty> SurroundingSurfsProperty;
    EPVector<DataSurfaces::IntMassObject> IntMassObjects;
    EPVector<DataSurfaces::GroundSurfacesProperty> GroundSurfsProperty;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) SurfacesData();
    }
};

} // namespace EnergyPlus

#endif
