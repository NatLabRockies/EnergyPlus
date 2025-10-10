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

#ifndef DaylightingManager_hh_INCLUDED
#define DaylightingManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1A.hh>
#include <ObjexxFCL/Array2A.hh>
#include <ObjexxFCL/Array2S.hh>
#include <ObjexxFCL/Vector3.fwd.hh>

// EnergyPlus Headers
#include <EnergyPlus/DataBSDFWindow.hh>
#include <EnergyPlus/DataDaylighting.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Dayltg {

    // Surface count crossover for using octree algorithm
    // The octree gives lower computational complexity for much higher performance
    //  as the surface count increases but has some overhead such that the direct
    //  algorithm can be more efficient at small surface counts.
    // Testing to date shows that the octree performance is close to that of the
    //  direct algorithm even with small surface counts and that there is no single
    //  crossover that is ideal for all models: some cases with 10-30 surfaces were
    //  faster with the octree but another with 80 surfaces was faster with the
    //  direct algorithm.
    // A reasonable, conservative crossover is selected but may be refined as more
    //  experience is gained.
    constexpr int octreeCrossover(100); // Octree surface count crossover
    constexpr int NTH(18);              // Number of azimuth steps for sky integration
    constexpr int NPH(8);               // Number of altitude steps for sky integration

    // It's crazy having both NPH and NPHMAX
    constexpr int NPHMAX(10); // Number of sky/ground integration steps in altitude
    constexpr int NTHMAX(16); // Number of sky/ground integration steps in azimuth

    struct SunAngles
    {
        Real64 phi = 0.0; // Solar altitude (radians)
        Real64 sinPhi = 0.0;
        Real64 cosPhi = 0.0;
        Real64 theta = 0.0; // Solar azimuth (rad) in Absolute Coordinate System (azimuth=0 along east)
    };

    void DayltgAveInteriorReflectance(EnergyPlusData &state, int const enclNum); // Enclosure number

    void CalcDayltgCoefficients(EnergyPlusData &state);

    void CalcDayltgCoeffsRefMapPoints(EnergyPlusData &state);

    void CalcDayltgCoeffsRefPoints(EnergyPlusData &state, int const daylightCtrlNum);

    void CalcDayltgCoeffsMapPoints(EnergyPlusData &state, int const mapNum);

    void FigureDayltgCoeffsAtPointsSetupForWindow(EnergyPlusData &state,
                                                  int const daylightCtrlNum, // zero if called for map points
                                                  int const iRefPoint,
                                                  int const loopwin,
                                                  CalledFor const CalledFrom,    // indicate  which type of routine called this routine
                                                  Vector3<Real64> const &RREF,   // Location of a reference point in absolute coordinate system
                                                  Vector3<Real64> const &VIEWVC, // View vector in absolute coordinate system
                                                  int &IWin,
                                                  int &IWin2,
                                                  int &NWX,
                                                  int &NWY,
                                                  Vector3<Real64> &W2,  // Second vertex of window
                                                  Vector3<Real64> &W3,  // Third vertex of window
                                                  Vector3<Real64> &W21, // Vector from window vertex 2 to window vertex 1
                                                  Vector3<Real64> &W23, // Vector from window vertex 2 to window vertex 3
                                                  int &LSHCAL,      // Interior shade calculation flag:  0=not yet calculated, 1=already calculated
                                                  int &InShelfSurf, // Inside daylighting shelf surface number
                                                  int &ICtrl,       // Window control counter
                                                  DataSurfaces::WinShadingType &ShType, // Window shading type
                                                  int &BlNum,                           // Window blind number
                                                  Vector3<Real64> &WNORM2,              // Unit vector normal to window
                                                  ExtWinType &ExtWinType, // Exterior window type (InZoneExtWin, AdjZoneExtWin, NotInOrAdjZoneExtWin)
                                                  int &IConst,            // Construction counter
                                                  Vector3<Real64> &RREF2, // Location of virtual reference point in absolute coordinate system
                                                  Real64 &DWX,            // Horizontal dimension of window element (m)
                                                  Real64 &DWY,            // Vertical dimension of window element (m)
                                                  Real64 &DAXY,           // Area of window element
                                                  Vector3<Real64> &U2,    // Second vertex of window for TDD:DOME (if exists)
                                                  Vector3<Real64> &U23,   // Vector from window vertex 2 to window vertex 3 for TDD:DOME (if exists)
                                                  Vector3<Real64> &U21,   // Vector from window vertex 2 to window vertex 1 for TDD:DOME (if exists)
                                                  Vector3<Real64> &VIEWVC2, // Virtual view vector in absolute coordinate system
                                                  bool &Rectangle,          // True if window is rectangular
                                                  bool &Triangle,           // True if window is triangular
                                                  int const MapNum = 0);

    void FigureDayltgCoeffsAtPointsForWindowElements(
        EnergyPlusData &state,
        int const daylightCtrlNum, // Current daylighting control number (only used when called from RefPoint)
        int const iRefPoint,
        int const loopwin,
        CalledFor const CalledFrom, // indicate  which type of routine called this routine
        int const WinEl,            // Current window element number
        int const IWin,
        int const IWin2,
        int const iXelement,
        int const iYelement,
        Real64 &SkyObstructionMult,
        Vector3<Real64> const &W2,      // Second vertex of window
        Vector3<Real64> const &W21,     // Vector from window vertex 2 to window vertex 1
        Vector3<Real64> const &W23,     // Vector from window vertex 2 to window vertex 3
        Vector3<Real64> const &RREF,    // Location of a reference point in absolute coordinate system
        int const NWYlim,               // For triangle, largest NWY for a given IX
        Vector3<Real64> const &VIEWVC2, // Virtual view vector in absolute coordinate system
        Real64 const DWX,               // Horizontal dimension of window element (m)
        Real64 const DWY,               // Vertical dimension of window element (m)
        Real64 const DAXY,              // Area of window element
        Vector3<Real64> const &U2,      // Second vertex of window for TDD:DOME (if exists)
        Vector3<Real64> const &U23,     // Vector from window vertex 2 to window vertex 3 for TDD:DOME (if exists)
        Vector3<Real64> const &U21,     // Vector from window vertex 2 to window vertex 1 for TDD:DOME (if exists)
        Vector3<Real64> &RWIN,          // Center of a window element for TDD:DOME (if exists) in abs coord sys
        Vector3<Real64> &RWIN2,         // Center of a window element for TDD:DOME (if exists) in abs coord sys
        Vector3<Real64> &Ray,           // Unit vector along ray from reference point to window element
        Real64 &PHRAY,                  // Altitude of ray from reference point to window element (radians)
        int &LSHCAL,                    // Interior shade calculation flag:  0=not yet calculated, 1=already calculated
        Real64 &COSB,                   // Cosine of angle between window outward normal and ray from reference point to window element
        Real64 &ObTrans,                // Product of solar transmittances of exterior obstructions hit by ray
        Real64 &TVISB,                  // Visible transmittance of window for COSB angle of incidence (times light well
        Real64 &DOMEGA,                 // Solid angle subtended by window element wrt reference point (steradians)
        Real64 &THRAY,                  // Azimuth of ray from reference point to window element (radians)
        bool &hitIntObs,                // True iff interior obstruction hit
        bool &hitExtObs,                // True iff ray from ref pt to ext win hits an exterior obstruction
        Vector3<Real64> const &WNORM2,  // Unit vector normal to window
        ExtWinType const ExtWinType,    // Exterior window type (InZoneExtWin, AdjZoneExtWin, NotInOrAdjZoneExtWin)
        int const IConst,               // Construction counter
        Vector3<Real64> const &RREF2,   // Location of virtual reference point in absolute coordinate system
        bool const Triangle,
        Real64 &TVISIntWin,     // Visible transmittance of int win at COSBIntWin for light from ext win
        Real64 &TVISIntWinDisk, // Visible transmittance of int win at COSBIntWin for sun
        int const MapNum = 0);

    void InitializeCFSDaylighting(EnergyPlusData &state,
                                  int const daylightCtrlNum,       // Current daylighting control number
                                  int const IWin,                  // Complex fenestration number
                                  int const NWX,                   // Number of horizontal divisions
                                  int const NWY,                   // Number of vertical divisions
                                  Vector3<Real64> const &RefPoint, // reference point coordinates
                                  int const NRefPts,               // Number of reference points
                                  int const iRefPoint,             // Reference points counter
                                  CalledFor const CalledFrom,
                                  int const MapNum = 0);

    void InitializeCFSStateData(EnergyPlusData &state,
                                DataBSDFWindow::BSDFRefPoints &StateRefPoint,
                                DataBSDFWindow::BSDFRefPointsGeomDescr &DaylghtGeomDescr,
                                int const daylightCtrlNum, // Current daylighting control number
                                int const iWin,
                                Vector3<Real64> const &RefPoint, // reference point
                                int const CurFenState,
                                int const NBasis,
                                int const NTrnBasis,
                                Real64 const AZVIEW,
                                int const NWX,
                                int const NWY,
                                Vector3<Real64> const &W2,
                                Vector3<Real64> const &W21,
                                Vector3<Real64> const &W23,
                                Real64 const DWX,
                                Real64 const DWY,
                                Vector3<Real64> const &WNorm, // unit vector from window (point towards outside)
                                Real64 const WinElArea);

    void AllocateForCFSRefPointsState(
        EnergyPlusData &state, DataBSDFWindow::BSDFRefPoints &StateRefPoint, int const NumOfWinEl, int const NBasis, int const NTrnBasis);

    void AllocateForCFSRefPointsGeometry(DataBSDFWindow::BSDFRefPointsGeomDescr &RefPointsGeomDescr, int const NumOfWinEl);

    void CFSRefPointSolidAngle(EnergyPlusData &state,
                               Vector3<Real64> const &RefPoint,
                               Vector3<Real64> const &RWin,
                               Vector3<Real64> const &WNorm,
                               DataBSDFWindow::BSDFRefPoints &RefPointMap,
                               DataBSDFWindow::BSDFRefPointsGeomDescr &RefPointGeomMap,
                               int const iWin,
                               int const CurFenState,
                               int const NTrnBasis,
                               int const curWinEl,
                               Real64 const WinElArea);

    void CFSRefPointPosFactor(EnergyPlusData &state,
                              Vector3<Real64> const &RefPoint,
                              DataBSDFWindow::BSDFRefPoints &RefPointMap,
                              int const iWin,
                              int const CurFenState,
                              int const NTrnBasis,
                              Real64 const AZVIEW);

    Real64 CalcObstrMultiplier(EnergyPlusData &state,
                               Vector3<Real64> const &GroundHitPt, // Coordinates of point that ray hits ground (m)
                               int const AltSteps,                 // Number of steps in altitude angle for solar reflection calc
                               int const AzimSteps                 // Number of steps in azimuth angle of solar reflection calc
    );

    void FigureDayltgCoeffsAtPointsForSunPosition(
        EnergyPlusData &state,
        int const daylightCtrlNum, // Current daylighting control number
        int const iRefPoint,
        int const iXelement,
        int const NWX, // Number of window elements in x direction for dayltg calc
        int const iYelement,
        int const NWY,   // Number of window elements in y direction for dayltg calc
        int const WinEl, // Current window element counter
        int const IWin,
        int const IWin2,
        int const iHour,
        int &ISunPos,
        Real64 const SkyObstructionMult,
        Vector3<Real64> const &RWIN2, // Center of a window element for TDD:DOME (if exists) in abs coord sys
        Vector3<Real64> const &Ray,   // Unit vector along ray from reference point to window element
        Real64 const PHRAY,           // Altitude of ray from reference point to window element (radians)
        int const LSHCAL,             // Interior shade calculation flag:  0=not yet calculated, 1=already calculated
        int const InShelfSurf,        // Inside daylighting shelf surface number
        Real64 const COSB,            // Cosine of angle between window outward normal and ray from reference point to window element
        Real64 const ObTrans, // Product of solar transmittances of exterior obstructions hit by ray from reference point through a window element
        Real64 const TVISB,   // Visible transmittance of window for COSB angle of incidence (times light well efficiency, if appropriate)
        Real64 const DOMEGA,  // Solid angle subtended by window element wrt reference point (steradians)
        int const ICtrl,      // Window control counter
        DataSurfaces::WinShadingType const ShType, // Window shading type
        int const BlNum,                           // Window blind number
        Real64 const THRAY,                        // Azimuth of ray from reference point to window element (radians)
        Vector3<Real64> const &WNORM2,             // Unit vector normal to window
        ExtWinType const ExtWinType,               // Exterior window type (InZoneExtWin, AdjZoneExtWin, NotInOrAdjZoneExtWin)
        int const IConst,                          // Construction counter
        Real64 const AZVIEW,                       // Azimuth of view vector in absolute coord system for glare calculation (radians)
        Vector3<Real64> const &RREF2,              // Location of virtual reference point in absolute coordinate system
        bool const hitIntObs,                      // True iff interior obstruction hit
        bool const hitExtObs,                      // True iff ray from ref pt to ext win hits an exterior obstruction
        CalledFor const CalledFrom,                // indicate  which type of routine called this routine
        Real64 TVISIntWin,                         // Visible transmittance of int win at COSBIntWin for light from ext win
        Real64 &TVISIntWinDisk,                    // Visible transmittance of int win at COSBIntWin for sun
        int const MapNum = 0);

    void FigureRefPointDayltgFactorsToAddIllums(EnergyPlusData &state,
                                                int const daylightCtrlNum, // Current daylighting control number
                                                int const iRefPoint,
                                                int const iHour,
                                                int &ISunPos,
                                                int const IWin,
                                                int const loopwin,
                                                int const NWX,  // Number of window elements in x direction for dayltg calc
                                                int const NWY,  // Number of window elements in y direction for dayltg calc
                                                int const ICtrl // Window control counter
    );

    void FigureMapPointDayltgFactorsToAddIllums(EnergyPlusData &state,
                                                int const MapNum,
                                                int const iMapPoint,
                                                int const iHour,
                                                int const IWin,
                                                int const loopwin,
                                                int const ICtrl // Window control counter
    );

    void GetDaylightingParametersInput(EnergyPlusData &state);

    void GetInputIlluminanceMap(EnergyPlusData &state, bool &ErrorsFound);

    void GetDaylightingControls(EnergyPlusData &state, bool &ErrorsFound);

    void GeometryTransformForDaylighting(EnergyPlusData &state);

    void GetInputDayliteRefPt(EnergyPlusData &state, bool &ErrorsFound);

    bool doesDayLightingUseDElight(EnergyPlusData const &state);

    void CheckTDDsAndLightShelvesInDaylitZones(EnergyPlusData &state);

    void AssociateWindowShadingControlWithDaylighting(EnergyPlusData &state);

    void GetLightWellData(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    int findWinShadingStatus(int const IWin);

    Real64 DayltgGlare(EnergyPlusData &state,
                       int IL,                   // Reference point index: 1=first ref pt, 2=second ref pt
                       Real64 BLUM,              // Window background (surround) luminance (cd/m2)
                       int const daylightCtrlNum // Current daylighting control number
    );

    void DayltgGlareWithIntWins(EnergyPlusData &state,
                                int const daylightCtrlNum // Current daylighting control number
    );

    void DayltgExtHorizIllum(EnergyPlusData &state,
                             Illums &HI // Horizontal illuminance from sky for different sky types
    );

    // Product of solar transmittances of exterior obstructions
    Real64 DayltgHitObstruction(EnergyPlusData &state,
                                int const IHOUR,           // Hour number
                                int const IWin,            // Window index
                                Vector3<Real64> const &R1, // Origin of ray (m)
                                Vector3<Real64> const &RN  // Destination of ray (m)
    );

    bool DayltgHitInteriorObstruction(EnergyPlusData &state,
                                      int const IWin,            // Window index
                                      Vector3<Real64> const &R1, // Origin of ray (m)
                                      Vector3<Real64> const &R2  // Destination of ray (m)
    );

    bool DayltgHitBetWinObstruction(EnergyPlusData &state,
                                    int const IWin1,           // Surface number of origin window
                                    int const IWin2,           // Surface number of destination window
                                    Vector3<Real64> const &R1, // Origin of ray (on IWin1) (m)
                                    Vector3<Real64> const &R2  // Destination of ray (on IWin2) (m)
    );

    void initDaylighting(EnergyPlusData &state, bool const initSurfaceHeatBalancefirstTime);

    void manageDaylighting(EnergyPlusData &state);

    void DayltgInteriorIllum(EnergyPlusData &state, int const daylightCtrlNum); // Daylighting:Controls number

    void DayltgInteriorTDDIllum(EnergyPlusData &state);

    void DayltgElecLightingControl(EnergyPlusData &state);

    Real64 DayltgGlarePositionFactor(Real64 X, // Lateral and vertical distance of luminous window element from
                                     Real64 Y);

    void DayltgInterReflectedIllum(EnergyPlusData &state,
                                   int const ISunPos, // Sun position counter; used to avoid calculating various
                                   int const IHR,     // Hour of day
                                   int const enclNum, // Daylighting enclosure index
                                   int const IWin     // Window index
    );

    void ComplexFenestrationLuminances(EnergyPlusData &state,
                                       int const IWin,
                                       int const WinEl,
                                       int const NBasis,
                                       int const IHR,
                                       int const iRefPoint,
                                       Array1D<Illums> &ElementLuminanceSky, // sky related luminance at window element (exterior side)
                                       CalledFor const CalledFrom,
                                       int const MapNum = 0);

    void DayltgInterReflectedIllumComplexFenestration(EnergyPlusData &state,
                                                      int const IWin,            // Window index
                                                      int const WinEl,           // Current window element counter
                                                      int const IHR,             // Hour of day
                                                      int const daylightCtrlNum, // Daylighting control number
                                                      int const iRefPoint,       // reference point counter
                                                      CalledFor const CalledFrom,
                                                      int const MapNum = 0);

    void DayltgDirectIllumComplexFenestration(EnergyPlusData &state,
                                              int const IWin,      // Window index
                                              int const WinEl,     // Current window element counter
                                              int const IHR,       // Hour of day
                                              int const iRefPoint, // reference point index
                                              CalledFor const CalledFrom,
                                              int const MapNum = 0);

    void DayltgDirectSunDiskComplexFenestration(EnergyPlusData &state,
                                                int const iWin,  // Window index
                                                int const iHour, // Hour of day
                                                int const iRefPoint,
                                                int const NumEl,            // Total number of window elements
                                                Real64 const AZVIEW,        // Azimuth of view vector in absolute coord system for
                                                CalledFor const CalledFrom, // indicate  which type of routine called this routine
                                                int const MapNum = 0);

    Real64 DayltgSkyLuminance(EnergyPlusData const &state,
                              SkyType sky,        // Sky type: 1=clear, 2=clear turbid, 3=intermediate, 4=overcast
                              Real64 const THSKY, // Azimuth and altitude of sky element (radians)
                              Real64 const PHSKY);

    // Solar profile angle (radians).
    Real64 ProfileAngle(EnergyPlusData &state,
                        int const SurfNum,                                     // Surface number
                        Vector3<Real64> const &CosDirSun,                      // Solar direction cosines
                        DataWindowEquivalentLayer::Orientation const HorOrVert // If HORIZONTAL, calculates ProfileAngHor
    );

    void DayltgClosestObstruction(EnergyPlusData &state,
                                  Vector3<Real64> const &RecPt,  // Point on window from which ray emanates (m)
                                  Vector3<Real64> const &RayVec, // Unit vector along ray pointing away from window (m)
                                  int &NearestHitSurfNum,        // Surface number of nearest obstruction that is hit by ray;
                                  Vector3<Real64> &NearestHitPt  // Ray's hit point on nearest obstruction (m)
    );

    // Luminance at ReflHitPt from beam solar reflection for unit
    Real64 DayltgSurfaceLumFromSun(EnergyPlusData &state,
                                   int const IHR,                   // Hour number
                                   Vector3<Real64> const &Ray,      // Ray from window to reflecting surface (m)
                                   int const ReflSurfNum,           // Number of surface for which luminance is being calculated
                                   Vector3<Real64> const &ReflHitPt // Point on ReflSurfNum for luminance calculation (m)
    );

    void DayltgInteriorMapIllum(EnergyPlusData &state);

    void ReportIllumMap(EnergyPlusData &state, int const MapNum);

    void CloseReportIllumMaps(EnergyPlusData &state);

    void CloseDFSFile(EnergyPlusData &state);

    void DayltgSetupAdjZoneListsAndPointers(EnergyPlusData &state);

    void CreateShadeDeploymentOrder(EnergyPlusData &state, int const enclNum);

    void MapShadeDeploymentOrderToLoopNumber(EnergyPlusData &state, int const enclNum);

    void DayltgInterReflIllFrIntWins(EnergyPlusData &state, int const enclNum);

    void CalcMinIntWinSolidAngs(EnergyPlusData &state);

    void CheckForGeometricTransform(EnergyPlusData &state, bool &doTransform, Real64 &OldAspectRatio, Real64 &NewAspectRatio);

    void WriteDaylightMapTitle(EnergyPlusData &state,
                               int const mapNum,
                               InputOutputFile &mapFile,
                               std::string const &mapName,
                               std::string const &environmentName,
                               int const ZoneNum,
                               std::string const &refPts,
                               Real64 const zcoord);

} // namespace Dayltg

struct DaylightingData : BaseGlobalStruct
{
    int maxControlRefPoints = 0;
    int maxShadeDeployOrderExtWins = 0;
    int maxDayltgExtWins = 0;
    int maxEnclSubSurfaces = 0;

    bool mapResultsToReport = false; // used when only partial hour has "sun up"
    bool mapResultsReported = false; // when no map results are ever reported this will still be false
    char MapColSep;                  // Character for separating map columns (tab, space, comma)
    bool DFSReportSizingDays = false;
    bool DFSReportAllShadowCalculationDays = false;

    Array1D<Dayltg::EnclDaylightCalc> enclDaylight;
    Array1D<Dayltg::ZoneDaylightCalc> ZoneDaylight;
    Array1D<Dayltg::DaylightingControl> daylightControl;
    Array1D<Dayltg::IllumMap> illumMaps;
    Array1D<Dayltg::RefPointData> DaylRefPt;
    Array1D<Dayltg::DElightComplexFeneData> DElightComplexFene;
    Array1D<Real64> spacePowerReductionFactor; // Average electric power reduction factor for space due to daylighting

    bool CalcDayltghCoefficients_firstTime = true;
    bool getDaylightingParametersInputFlag = true;
    bool refFirstTime = true;
    bool DayltgInteriorIllum_firstTime = true; // true first time routine is called
    bool FirstTimeDaylFacCalc = true;
    bool VeryFirstTime = true;
    bool mapFirstTime = true;
    bool CheckTDDs_firstTime = true;
    bool DayltgExtHorizIllum_firstTime = true; // flag for first time thru to initialize
    bool DayltgInteriorMapIllum_FirstTimeFlag = true;
    bool ReportIllumMap_firstTime = true;
    bool SQFirstTime = true;
    bool doSkyReporting = true;
    bool CreateDFSReportFile = true;

    int TotWindowsWithDayl = 0;       // Total number of exterior windows in all daylit zones
    Array1D<Real64> DaylIllum;        // Daylight illuminance at reference points (lux)
    int maxNumRefPtInAnyDaylCtrl = 0; // The most number of reference points that any single daylighting control has
    int maxNumRefPtInAnyEncl = 0;     // The most number of reference points that any single enclosure has

    Dayltg::SunAngles sunAngles = Dayltg::SunAngles();
    std::array<Dayltg::SunAngles, (int)Constant::iHoursInDay + 1> sunAnglesHr = {Dayltg::SunAngles()};

    // In the following I,J,K arrays:
    // I = 1 for clear sky, 2 for clear turbid, 3 for intermediate, 4 for overcast;
    // J = 1 for bare window, 2 - 12 for shaded;
    // K = sun position index.
    std::array<Dayltg::Illums, (int)Constant::iHoursInDay + 1> horIllum = {
        Dayltg::Illums()}; // Horizontal illuminance from sky, by sky type, for each hour of the day
    Array1D<std::array<Dayltg::Illums, (int)DataSurfaces::WinCover::Num>> dirIllum;  // Sky-related component of direct illuminance
    Array1D<std::array<Dayltg::Illums, (int)DataSurfaces::WinCover::Num>> reflIllum; // Sky-related portion of internally reflected illuminance
    Array1D<std::array<Dayltg::Illums, (int)DataSurfaces::WinCover::Num>> winLum;    // Sky-related window luminance

    Array1D<std::array<Dayltg::Illums, (int)DataSurfaces::WinCover::Num>> avgWinLum; // Sky-related average window luminance
    // Allocatable daylight factor arrays  -- are in the ZoneDaylight Structure

    Array2D<Real64> TDDTransVisBeam;
    Array2D<Dayltg::Illums> TDDFluxInc;
    Array2D<Dayltg::Illums> TDDFluxTrans;

    Array2D_int MapErrIndex;
    Array2D_int RefErrIndex;

    bool MySunIsUpFlag = false;
    bool CalcDayltgCoeffsMapPointsMySunIsUpFlag = false;
    int AltSteps_last = 0;
    std::array<Real64, DataSurfaces::AltAngStepsForSolReflCalc / 2 + 1> cos_Phi = {0.0}; // cos( Phi ) table
    std::array<Real64, DataSurfaces::AltAngStepsForSolReflCalc / 2 + 1> sin_Phi = {0.0}; // sin( Phi ) table
    int AzimSteps_last = 0;
    std::array<Real64, 2 * DataSurfaces::AzimAngStepsForSolReflCalc + 1> cos_Theta = {0.0}; // cos( Theta ) table
    std::array<Real64, 2 * DataSurfaces::AzimAngStepsForSolReflCalc + 1> sin_Theta = {0.0}; // sin( Theta ) table

    std::array<Real64, Dayltg::NPH + 1> PH; // Altitude of sky element (radians)

    // Ratio of obstructed to unobstructed sky diffuse at a ground point for each (TH,PH) direction
    std::array<Real64, Dayltg::NTH + 1> TH;     // Azimuth of sky element (radians)
    std::array<Real64, Dayltg::NPH + 1> SPHCPH; // Sine times cosine of altitude of sky element

    // Not sure why these need to be state variables and can't local
    // variable of DayltgInterReflectIllum(), but some EMS tests break
    // if they are made local
    std::array<std::array<Real64, Dayltg::NTHMAX + 1>, Dayltg::NPHMAX + 1> SkyObstructionMult;
    std::array<std::array<Real64, Dayltg::NTHMAX + 1>, Dayltg::NPHMAX + 1> ObTransM; // ObTrans value for each (TH,PH) direction

    Array1D_bool FirstTimeMaps;
    Array1D_bool EnvrnPrint;
    Array1D_string SavedMnDy;
    Array1D<Real64> XValue;
    Array1D<Real64> YValue;
    Array2D<Real64> IllumValue;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->CalcDayltghCoefficients_firstTime = true;
        this->getDaylightingParametersInputFlag = true;
        this->refFirstTime = true;
        this->DayltgInteriorIllum_firstTime = true;
        this->FirstTimeDaylFacCalc = true;
        this->VeryFirstTime = true;
        this->mapFirstTime = true;
        this->CheckTDDs_firstTime = true;
        this->DayltgExtHorizIllum_firstTime = true;
        this->DayltgInteriorMapIllum_FirstTimeFlag = true;
        this->ReportIllumMap_firstTime = true;
        this->SQFirstTime = true;
        this->doSkyReporting = true;
        this->CreateDFSReportFile = true;
        this->TotWindowsWithDayl = 0;
        this->DaylIllum.deallocate();
        this->maxNumRefPtInAnyDaylCtrl = 0;
        this->maxNumRefPtInAnyEncl = 0;
        this->sunAngles = Dayltg::SunAngles();
        this->sunAnglesHr = {Dayltg::SunAngles()};

        this->horIllum = {Dayltg::Illums()};
        this->dirIllum.deallocate();
        this->reflIllum.deallocate();
        this->winLum.deallocate();
        this->avgWinLum.deallocate();

        this->TDDTransVisBeam.deallocate();
        this->TDDFluxInc.deallocate();
        this->TDDFluxTrans.deallocate();
        this->MapErrIndex.deallocate();
        this->RefErrIndex.deallocate();
        this->MySunIsUpFlag = false;
        this->CalcDayltgCoeffsMapPointsMySunIsUpFlag = false;
        this->AltSteps_last = 0;
        this->AzimSteps_last = 0;
        this->cos_Phi = {0.0};
        this->sin_Phi = {0.0};
        this->cos_Theta = {0.0};
        this->sin_Theta = {0.0};
        this->FirstTimeMaps.clear();
        this->EnvrnPrint.clear();
        this->SavedMnDy.clear();
        this->XValue.clear();
        this->YValue.clear();
        this->IllumValue.clear();
    }
};

} // namespace EnergyPlus

#endif
