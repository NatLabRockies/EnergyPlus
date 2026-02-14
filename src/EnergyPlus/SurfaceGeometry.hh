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

#ifndef SurfaceGeometry_hh_INCLUDED
#define SurfaceGeometry_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1S.hh>

// EnergyPlus Headers
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataVectorTypes.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/HeatBalanceKivaManager.hh>
#include <EnergyPlus/Vectors.hh>

// C++ Headers
#include <map>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace SurfaceGeometry {

    // Using/Aliasing
    using DataSurfaces::SurfaceClass;
    using DataSurfaces::SurfaceData;
    using DataVectorTypes::Vector;

    enum enclosureType
    {
        RadiantEnclosures,
        SolarEnclosures
    };

    void SetupZoneGeometry(EnergyPlusData &state, bool &ErrorsFound);

    void AllocateSurfaceArrays(EnergyPlusData &state);

    void AllocateSurfaceWindows(EnergyPlusData &state, int NumSurfaces);

    void GetSurfaceData(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    void CreateMissingSpaces(EnergyPlusData &state, Array1D<SurfaceGeometry::SurfaceData> &Surfaces);

    void createSpaceSurfaceLists(EnergyPlusData &state);

    void setSurfaceFirstLast(EnergyPlusData &state);

    void checkSubSurfAzTiltNorm(EnergyPlusData &state,
                                SurfaceData &baseSurface, // Base surface data (in)
                                SurfaceData &subSurface,  // Subsurface data (in)
                                bool &surfaceError        // True if there is subsurface error that requires a fatal
    );

    void GetGeometryParameters(EnergyPlusData &state, bool &ErrorsFound); // set to true if errors found during input

    void GetDetShdSurfaceData(EnergyPlusData &state,
                              bool &ErrorsFound,          // Error flag indicator (true if errors found)
                              int &SurfNum,               // Count of Current SurfaceNumber
                              int const TotDetachedFixed, // Number of Fixed Detached Shading Surfaces to obtain
                              int const TotDetachedBldg   // Number of Building Detached Shading Surfaces to obtain
    );

    void GetRectDetShdSurfaceData(EnergyPlusData &state,
                                  bool &ErrorsFound,              // Error flag indicator (true if errors found)
                                  int &SurfNum,                   // Count of Current SurfaceNumber
                                  int const TotRectDetachedFixed, // Number of Fixed Detached Shading Surfaces to obtain
                                  int const TotRectDetachedBldg   // Number of Building Detached Shading Surfaces to obtain
    );

    void GetHTSurfaceData(EnergyPlusData &state,
                          bool &ErrorsFound,                 // Error flag indicator (true if errors found)
                          int &SurfNum,                      // Count of Current SurfaceNumber
                          int const TotHTSurfs,              // Number of Heat Transfer Base Surfaces to obtain
                          int const TotDetailedWalls,        // Number of Wall:Detailed items to obtain
                          int const TotDetailedRoofs,        // Number of RoofCeiling:Detailed items to obtain
                          int const TotDetailedFloors,       // Number of Floor:Detailed items to obtain
                          const Array1D_string &BaseSurfCls, // Valid Classes for Base Surfaces
                          const Array1D<SurfaceClass> &BaseSurfIDs,
                          int &NeedToAddSurfaces // Number of surfaces to add, based on unentered IZ surfaces
    );

    void GetRectSurfaces(EnergyPlusData &state,
                         bool &ErrorsFound,                        // Error flag indicator (true if errors found)
                         int &SurfNum,                             // Count of Current SurfaceNumber
                         int const TotRectExtWalls,                // Number of Exterior Walls to obtain
                         int const TotRectIntWalls,                // Number of Adiabatic Walls to obtain
                         int const TotRectIZWalls,                 // Number of Interzone Walls to obtain
                         int const TotRectUGWalls,                 // Number of Underground to obtain
                         int const TotRectRoofs,                   // Number of Roofs to obtain
                         int const TotRectCeilings,                // Number of Adiabatic Ceilings to obtain
                         int const TotRectIZCeilings,              // Number of Interzone Ceilings to obtain
                         int const TotRectGCFloors,                // Number of Floors with Ground Contact to obtain
                         int const TotRectIntFloors,               // Number of Adiabatic Walls to obtain
                         int const TotRectIZFloors,                // Number of Interzone Floors to obtain
                         const Array1D<SurfaceClass> &BaseSurfIDs, // ID Assignments for valid surface classes
                         int &NeedToAddSurfaces                    // Number of surfaces to add, based on unentered IZ surfaces
    );

    void MakeRectangularVertices(EnergyPlusData &state,
                                 int const SurfNum,
                                 Real64 const XCoord,
                                 Real64 const YCoord,
                                 Real64 const ZCoord,
                                 Real64 const Length,
                                 Real64 const Height,
                                 bool const SurfWorldCoordSystem);

    void GetHTSubSurfaceData(EnergyPlusData &state,
                             bool &ErrorsFound,                       // Error flag indicator (true if errors found)
                             int &SurfNum,                            // Count of Current SurfaceNumber
                             int const TotHTSubs,                     // Number of Heat Transfer SubSurfaces to obtain
                             const Array1D_string &SubSurfCls,        // Valid Classes for Sub Surfaces
                             const Array1D<SurfaceClass> &SubSurfIDs, // ID Assignments for valid sub surface classes
                             int &AddedSubSurfaces,                   // Subsurfaces added when windows reference Window5
                             int &NeedToAddSurfaces                   // Number of surfaces to add, based on unentered IZ surfaces
    );

    void GetRectSubSurfaces(EnergyPlusData &state,
                            bool &ErrorsFound,                       // Error flag indicator (true if errors found)
                            int &SurfNum,                            // Count of Current SurfaceNumber
                            int const TotWindows,                    // Number of Window SubSurfaces to obtain
                            int const TotDoors,                      // Number of Door SubSurfaces to obtain
                            int const TotGlazedDoors,                // Number of Glass Door SubSurfaces to obtain
                            int const TotIZWindows,                  // Number of Interzone Window SubSurfaces to obtain
                            int const TotIZDoors,                    // Number of Interzone Door SubSurfaces to obtain
                            int const TotIZGlazedDoors,              // Number of Interzone Glass Door SubSurfaces to obtain
                            const Array1D<SurfaceClass> &SubSurfIDs, // ID Assignments for valid sub surface classes
                            int &AddedSubSurfaces,                   // Subsurfaces added when windows reference Window5
                            int &NeedToAddSubSurfaces                // Number of surfaces to add, based on unentered IZ surfaces
    );

    void CheckWindowShadingControlFrameDivider(EnergyPlusData &state,
                                               std::string_view const cRoutineName, // routine name calling this one (for error messages)
                                               bool &ErrorsFound,                   // true if errors have been found or are found here
                                               int const SurfNum,                   // current surface number
                                               int const FrameField                 // field number for frame/divider
    );

    void CheckSubSurfaceMiscellaneous(EnergyPlusData &state,
                                      std::string_view const cRoutineName,       // routine name calling this one (for error messages)
                                      bool &ErrorsFound,                         // true if errors have been found or are found here
                                      int const SurfNum,                         // current surface number
                                      std::string const &SubSurfaceName,         // name of the surface
                                      std::string const &SubSurfaceConstruction, // name of the construction
                                      int &AddedSubSurfaces);

    void MakeRelativeRectangularVertices(EnergyPlusData &state,
                                         int const BaseSurfNum, // Base surface
                                         int const SurfNum,
                                         Real64 const XCoord,
                                         Real64 const ZCoord,
                                         Real64 const Length,
                                         Real64 const Height);

    void MakeEquivalentRectangle(EnergyPlusData &state,
                                 int const SurfNum, // Surface number
                                 bool &ErrorsFound  // Error flag indicator (true if errors found)
    );

    void GetAttShdSurfaceData(EnergyPlusData &state,
                              bool &ErrorsFound,   // Error flag indicator (true if errors found)
                              int &SurfNum,        // Count of Current SurfaceNumber
                              int const TotShdSubs // Number of Attached Shading SubSurfaces to obtain
    );

    void GetSimpleShdSurfaceData(EnergyPlusData &state,
                                 bool &ErrorsFound,                // Error flag indicator (true if errors found)
                                 int &SurfNum,                     // Count of Current SurfaceNumber
                                 int const TotOverhangs,           // Number of Overhangs to obtain
                                 int const TotOverhangsProjection, // Number of Overhangs (projection) to obtain
                                 int const TotFins,                // Number of Fins to obtain
                                 int const TotFinsProjection       // Number of Fins (projection) to obtain
    );

    void GetIntMassSurfaceData(EnergyPlusData &state,
                               bool &ErrorsFound, // Error flag indicator (true if errors found)
                               int &SurfNum       // Count of Current SurfaceNumber
    );

    int GetNumIntMassSurfaces(EnergyPlusData &state); // Number of Internal Mass Surfaces to obtain

    void GetShadingSurfReflectanceData(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    void GetSurfaceSrdSurfsData(EnergyPlusData &state, bool &ErrorsFound); // Error flag indicator (true if errors found)

    void GetSurfaceLocalEnvData(EnergyPlusData &state, bool &ErrorsFound); // Error flag indicator (true if errors found)

    void GetHTSurfExtVentedCavityData(EnergyPlusData &state, bool &ErrorsFound); // Error flag indicator (true if errors found)

    void GetSurfaceHeatTransferAlgorithmOverrides(EnergyPlusData &state, bool &ErrorsFound);

    void GetSurfaceGroundSurfsData(EnergyPlusData &state, bool &ErrorsFound); // Error flag indicator (true if errors found)

    // Roof perimeter, Area, weighted-by-area average height azimuth
    struct GeoSummary
    {
        // Members
        Real64 Area = 0.0;      // Sum of all roof surface areas
        Real64 Perimeter = 0.0; // Actual perimeter of all roof surfaces, after removing all edges that are used twice (and inserting vertices
                                // to split surfaces as needed)
        Real64 Height = 0.0;    // Weighted average mean vertical height: for each surface, take max - Zmin value,
                                // then do a weighted average by surface area
        Real64 Azimuth = 0.0;   // Weighted average azimuth
        Real64 Tilt = 0.0;      // Weighted average tilt

        Real64 Zmax = 0;
        Real64 Zmin = 0;
        Real64 Ymax = 0;
        Real64 Ymin = 0;
        Real64 Xmax = 0;
        Real64 Xmin = 0;
    };

    void GetGeoSummaryRoof(EnergyPlusData const &state, GeoSummary &geoSumRoof);

    class ExposedFoundationPerimeter
    {
    public:
        void getData(EnergyPlusData &state, bool &ErrorsFound);
        struct Data
        {
            double exposedFraction = -1; // hush up cppcheck
            std::vector<bool> isExposedPerimeter;
            bool useDetailedExposedPerimeter;
        };
        std::map<int, Data> surfaceMap;
    };

    void GetVertices(EnergyPlusData &state,
                     int const SurfNum,             // Current surface number
                     int const NSides,              // Number of sides to figure
                     Array1S<Real64> const Vertices // Vertices, in specified order
    );

    void ReverseAndRecalculate(EnergyPlusData &state,
                               int const SurfNum,   // Surface number for the surface
                               int const NSides,    // number of sides to surface
                               Real64 &SurfAzimuth, // Surface Facing angle (will be 0 for roofs/floors)
                               Real64 &SurfTilt     // Surface tilt (
    );

    void MakeMirrorSurface(EnergyPlusData &state, int &SurfNum); // In=>Surface to Mirror, Out=>new Surface index

    void GetWindowShadingControlData(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    void InitialAssociateWindowShadingControlFenestration(EnergyPlusData &state, bool &ErrorsFound, int SurfNum);

    void FinalAssociateWindowShadingControlFenestration(EnergyPlusData &state, bool &ErrorsFound);

    void CheckWindowShadingControlSimilarForWindow(EnergyPlusData &state, bool &ErrorsFound);

    bool isWindowShadingControlSimilar(EnergyPlusData &state, int a, int b);

    void GetStormWindowData(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    void GetWindowGapAirflowControlData(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    void GetOSCData(EnergyPlusData &state, bool &ErrorsFound);

    void GetOSCMData(EnergyPlusData &state, bool &ErrorsFound);

    void GetFoundationData(EnergyPlusData &state, bool &ErrorsFound);

    void GetMovableInsulationData(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    void CalculateZoneVolume(EnergyPlusData &state);

    struct EdgeOfSurf
    {
        int surfNum = 0;
        Vector start;
        Vector end;
        std::vector<int> otherSurfNums;
        int count = 0;
        EdgeOfSurf() : start(Vector(0., 0., 0.)), end(Vector(0., 0., 0.))
        {
        }

        bool operator==(const EdgeOfSurf &other) const;
        bool operator!=(const EdgeOfSurf &other) const;
        bool containsPoints(const Vector &vertex) const;
        double length() const;
    };

    bool isEnclosedVolume(DataVectorTypes::Polyhedron const &zonePoly, std::vector<EdgeOfSurf> &edgeNot2);

    std::vector<EdgeOfSurf> edgesInBoth(std::vector<EdgeOfSurf> const &edges1, std::vector<EdgeOfSurf> const &edges2);

    bool edgesEqualOnSameSurface(EdgeOfSurf a, EdgeOfSurf b);

    std::vector<EdgeOfSurf> edgesNotTwoForEnclosedVolumeTest(DataVectorTypes::Polyhedron const &zonePoly, std::vector<Vector> const &uniqueVertices);

    std::vector<Vector> makeListOfUniqueVertices(DataVectorTypes::Polyhedron const &zonePoly);

    DataVectorTypes::Polyhedron updateZonePolygonsForMissingColinearPoints(DataVectorTypes::Polyhedron const &zonePoly,
                                                                           std::vector<Vector> const &uniqVertices);

    bool areFloorAndCeilingSame(EnergyPlusData &state, DataVectorTypes::Polyhedron const &zonePoly);

    bool areWallHeightSame(EnergyPlusData &state, DataVectorTypes::Polyhedron const &zonePoly);

    std::tuple<bool, bool, bool> areSurfaceHorizAndVert(EnergyPlusData &state, DataVectorTypes::Polyhedron const &zonePoly);

    bool areOppositeWallsSame(EnergyPlusData &state,
                              DataVectorTypes::Polyhedron const &zonePoly,
                              Real64 &oppositeWallArea,
                              Real64 &distanceBetweenOppositeWalls);

    std::vector<int> listOfFacesFacingAzimuth(EnergyPlusData &state, DataVectorTypes::Polyhedron const &zonePoly, Real64 azimuth);

    int findPossibleOppositeFace(EnergyPlusData &state, DataVectorTypes::Polyhedron const &zonePoly, int faceIndex);

    bool areCornersEquidistant(DataVectorTypes::Polyhedron const &zonePoly, int faceIndex, int opFaceIndex, Real64 &distanceBetween);

    bool isAlmostEqual3dPt(DataVectorTypes::Vector v1, DataVectorTypes::Vector v2);

    bool isAlmostEqual2dPt(DataVectorTypes::Vector_2d v1, DataVectorTypes::Vector_2d v2);

    int findIndexOfVertex(DataVectorTypes::Vector vertexToFind, std::vector<DataVectorTypes::Vector> const &listOfVertices);

    Real64 distance(DataVectorTypes::Vector v1, DataVectorTypes::Vector v2);

    Real64 distanceFromPointToLine(DataVectorTypes::Vector start, DataVectorTypes::Vector end, DataVectorTypes::Vector test);

    bool isPointOnLineBetweenPoints(DataVectorTypes::Vector start, DataVectorTypes::Vector end, DataVectorTypes::Vector test);

    void ProcessSurfaceVertices(EnergyPlusData &state, int const ThisSurf, bool &ErrorsFound);

    void CalcCoordinateTransformation(EnergyPlusData &state,
                                      int const SurfNum,            // Surface Number
                                      Vector &CompCoordTranslVector // Coordinate Translation Vector
    );

    void CreateShadedWindowConstruction(EnergyPlusData &state,
                                        int const SurfNum,          // Surface number
                                        int const WSCPtr,           // Pointer to WindowShadingControl for SurfNum
                                        int const ShDevNum,         // Shading device material number for WSCptr
                                        int const shadeControlIndex // index to the Surface().windowShadingControlList,
                                                                    // Surface().shadedConstructionList, and Surface().shadedStormWinConstructionList
    );

    void CreateStormWindowConstructions(EnergyPlusData &state);

    int createAirMaterialFromDistance(EnergyPlusData &state, Real64 distance, std::string_view namePrefix); // return new material number

    // create a new construction with storm based on an old construction and storm and gap materials
    int createConstructionWithStorm(EnergyPlusData &state, int oldConstruction, std::string name, int stormMaterial, int gapMaterial);

    void ModifyWindow(EnergyPlusData &state,
                      int const SurfNum,    // SurfNum has construction of glazing system from Window5 Data File;
                      bool &ErrorsFound,    // Set to true if errors found
                      int &AddedSubSurfaces // Subsurfaces added when window references a
    );

    void AddWindow(EnergyPlusData &state,
                   int const SurfNum,    // SurfNum has construction of glazing system from Window5 Data File;
                   bool &ErrorsFound,    // Set to true if errors found
                   int &AddedSubSurfaces // Subsurfaces added when window references a
    );

    void TransformVertsByAspect(EnergyPlusData &state,
                                int const SurfNum, // Current surface number
                                int const NSides   // Number of sides to figure
    );

    void CalcSurfaceCentroid(EnergyPlusData &state);

    void SetupShadeSurfacesForSolarCalcs(EnergyPlusData &state);

    void
    SetupEnclosuresAndAirBoundaries(EnergyPlusData &state,
                                    EPVector<DataViewFactorInformation::EnclosureViewFactorInformation> &Enclosures, // Radiant or Solar Enclosures
                                    SurfaceGeometry::enclosureType EnclosureType,                                    // Radiant or Solar
                                    bool &ErrorsFound);                                                              // Set to true if errors found

    void CheckConvexity(EnergyPlusData &state,
                        int const SurfNum, // Current surface number
                        int const NSides   // Number of sides to figure
    );

    bool isRectangle(EnergyPlusData &state, int const ThisSurf // Current surface number
    );

    void CheckForReversedLayers(EnergyPlusData &state,
                                bool &RevLayerDiffs,    // true when differences are discovered in interzone constructions
                                int const ConstrNum,    // construction index
                                int const ConstrNumRev, // construction index for reversed construction
                                int const TotalLayers   // total layers for construction definition
    );

} // namespace SurfaceGeometry

struct SurfaceGeometryData : BaseGlobalStruct
{

    Real64 CosBldgRelNorth = 0.0;    // Cosine of the building rotation (relative north) (includes appendix G rotation)
    Real64 SinBldgRelNorth = 0.0;    // Sine of the building rotation (relative north)   (includes appendix G rotation)
    Real64 CosBldgRotAppGonly = 0.0; // Cosine of the building rotation for appendix G only(relative north)
    Real64 SinBldgRotAppGonly = 0.0; // Sine of the building rotation for appendix G only (relative north)
    Array1D<Real64> CosZoneRelNorth; // Cosine of the zone rotation (relative north)
    Array1D<Real64> SinZoneRelNorth; // Sine of the zone rotation (relative north)

    bool NoGroundTempObjWarning = true;   // This will cause a warning to be issued if surfaces with "Ground"
                                          // outside environment are used but no ground temperature object was input.
    bool NoFCGroundTempObjWarning = true; // This will cause a warning to be issued if surfaces with "GroundFCfactorMethod"
                                          // outside environment are used but no FC ground temperatures was input.
    bool RectSurfRefWorldCoordSystem =
        false;             // GlobalGeometryRules:Field Rectangular Surface Coordinate System (A5) = World (true) or Relative (false)
    int Warning1Count = 0; // counts of Modify Window 5/6 windows
    int Warning2Count = 0; // counts of overriding exterior windows with Window 5/6 glazing systems
    int Warning3Count = 0; // counts of overriding interior windows with Window 5/6 glazing systems

    bool ProcessSurfaceVerticesOneTimeFlag = true;
    int checkSubSurfAzTiltNormErrCount = 0;
    Array1D<Real64> Xpsv;
    Array1D<Real64> Ypsv;
    Array1D<Real64> Zpsv;

    bool GetSurfaceDataOneTimeFlag = false;
    std::unordered_map<std::string, std::string> UniqueSurfaceNames;
    bool firstTime = true;
    bool noTransform = true;
    bool CheckConvexityFirstTime = true;

    Array1D_string const BaseSurfCls;
    Array1D_string const SubSurfCls;
    Array1D<DataSurfaces::SurfaceClass> const BaseSurfIDs;
    Array1D<DataSurfaces::SurfaceClass> const SubSurfIDs;
    Array1D<SurfaceGeometry::SurfaceData> SurfaceTmp; // Allocated/Deallocated during input processing
    HeatBalanceKivaManager::KivaManager kivaManager;
    SurfaceGeometry::ExposedFoundationPerimeter exposedFoundationPerimeter;

    int ErrCount = 0;
    bool WarningDisplayed = false;
    int ErrCount2 = 0;
    int ErrCount3 = 0;
    int ErrCount4 = 0; // counts of interzone area mismatches.
    bool ShowZoneSurfaceHeaders = true;
    int ErrCount5 = 0;
    Real64 OldAspectRatio = 0.0;
    Real64 NewAspectRatio = 0.0;
    std::string transformPlane;
    Array1D<Vectors::Vector> Triangle1 = Array1D<Vectors::Vector>(3); // working struct for a 3-sided surface
    Array1D<Vectors::Vector> Triangle2 = Array1D<Vectors::Vector>(3); // working struct for a 3-sided surface
    Array1D<Real64> X;                                                // containers for x,y,z vertices of the surface
    Array1D<Real64> Y;
    Array1D<Real64> Z;
    Array1D<Real64> A; // containers for convexity test
    Array1D<Real64> B;
    int VertSize = 0; // size of X,Y,Z,A,B arrays

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        ProcessSurfaceVerticesOneTimeFlag = true;
        checkSubSurfAzTiltNormErrCount = 0;
        Xpsv.deallocate();
        Ypsv.deallocate();
        Zpsv.deallocate();
        // Following are used only during getting vertices, so are module variables here.
        CosBldgRelNorth = 0.0;
        SinBldgRelNorth = 0.0;
        CosBldgRotAppGonly = 0.0;
        SinBldgRotAppGonly = 0.0;
        CosZoneRelNorth.deallocate();
        SinZoneRelNorth.deallocate();
        NoGroundTempObjWarning = true;
        NoFCGroundTempObjWarning = true;
        RectSurfRefWorldCoordSystem = false;
        Warning1Count = 0;
        Warning2Count = 0;
        Warning3Count = 0;
        SurfaceTmp.deallocate();
        GetSurfaceDataOneTimeFlag = false;
        UniqueSurfaceNames.clear();
        kivaManager = HeatBalanceKivaManager::KivaManager();
        firstTime = true;
        noTransform = true;
        CheckConvexityFirstTime = true;
        ErrCount = 0;
        WarningDisplayed = false;
        ErrCount2 = 0;
        ErrCount3 = 0;
        ErrCount4 = 0;
        ErrCount5 = 0;
        ShowZoneSurfaceHeaders = true;
    }

    // Default Constructor
    SurfaceGeometryData()
        : BaseSurfCls(3, {"WALL", "FLOOR", "ROOF"}),
          SubSurfCls(6, {"WINDOW", "DOOR", "GLASSDOOR", "SHADING", "TUBULARDAYLIGHTDOME", "TUBULARDAYLIGHTDIFFUSER"}),
          BaseSurfIDs(3, {DataSurfaces::SurfaceClass::Wall, DataSurfaces::SurfaceClass::Floor, DataSurfaces::SurfaceClass::Roof}),
          SubSurfIDs(6,
                     {DataSurfaces::SurfaceClass::Window,
                      DataSurfaces::SurfaceClass::Door,
                      DataSurfaces::SurfaceClass::GlassDoor,
                      DataSurfaces::SurfaceClass::Shading,
                      DataSurfaces::SurfaceClass::TDD_Dome,
                      DataSurfaces::SurfaceClass::TDD_Diffuser})
    {
    }
};
} // namespace EnergyPlus

#endif
