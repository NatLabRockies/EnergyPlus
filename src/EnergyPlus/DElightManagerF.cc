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
#include <cmath>
#include <fstream>
#include <ostream>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/DElightManagerF.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataDElight.hh>
// #include <EnergyPlus/DataDaylighting.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>

extern "C" {
#include <DElight/DElightManagerC.h>
}

namespace EnergyPlus {

namespace DElightManagerF {

    // MODULE INFORMATION
    //       AUTHOR         Robert J. Hitchcock
    //       DATE WRITTEN   August 2003
    //       MODIFIED       January 2004
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:

    // Defines INTERFACE Statements for Fortran calls to the DElightManagerC.cpp module.
    // The DElightManager.cpp module in turn defines the C/C++ calls to the DElight DLL.
    // Also, contains subroutines for performing associated operations.

    // METHODOLOGY EMPLOYED:

    // C Language Implementation of DOE2.1d and Superlite 3.0
    // Daylighting Algorithms with new Complex Fenestration System
    // analysis algorithms.
    // The original DOE2 daylighting algorithms and implementation
    // in FORTRAN were developed by F.C. Winkelmann at the
    // Lawrence Berkeley National Laboratory.
    // The original Superlite algorithms and implementation in FORTRAN
    // were developed by Michael Modest and Jong-Jin Kim
    // under contract with Lawrence Berkeley National Laboratory.
    // REFERENCES:

    // "Daylighting Calculation in DOE-2," F.C.Winkelmann, LBL-11353, May 1983
    // "Daylighting Simulation in the DOE-2 Building Energy Analysis Program,"
    // F.C. Winkelmann and S. Selkowitz, Energy and Buildings 8(1985)271-286

    // USE STATEMENTS:
    using namespace DataDElight;

    void DElightInputGenerator(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Robert J. Hitchcock
        //       DATE WRITTEN   August 2003
        //       MODIFIED       February 2004 - Changes to accommodate mods in DElight IDD
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine creates a DElight input file from EnergyPlus processed input.

        // USE STATEMENTS:
        using namespace DataHeatBalance;   // Gives access to Building, Zone and Lights data
        using namespace DataEnvironment;   // Gives access to Site data
        using namespace DataSurfaces;      // Gives access to Surface data
        using namespace DataStringGlobals; // Gives access to Program Path and Current Time/Date
        using InternalHeatGains::CheckLightsReplaceableMinMaxForZone;
        using InternalHeatGains::GetDesignLightingLevelForZone;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int iNumDElightZones;   // Counter for Thermal Zones with hosted Daylighting:DElight objects
        int iNumOpaqueSurfs;    // Counter for opaque surfaces in each zone
        int iNumWindows;        // Counter for windows hosted in each surface
        int iconstruct;         // Index for construction type of surfaces
        int iMatlLayer;         // Index for the outside (i.e., 1st) Material Layer for a Construction
        Real64 rExtVisRefl;     // Exterior visible reflectance of a material
        Real64 rLightLevel;     // installed lighting level for current zone
        Real64 CosBldgRelNorth; // Cosine of Building rotation
        Real64 SinBldgRelNorth; // Sine of Building rotation
        Real64 CosZoneRelNorth; // Cosine of Zone rotation
        Real64 SinZoneRelNorth; // Sine of Zone rotation
        Real64 Xb;              // temp var for transformation calc
        Real64 Yb;              // temp var for transformation calc
        Vector3<Real64> RefPt_WCS_Coord;
        Array1D_int iWndoConstIndexes(100);
        bool lWndoConstFound;      // Flag for non-unique window const index
        std::string cNameWOBlanks; // Name without blanks
        bool ErrorsFound;
        int iHostedCFS;
        bool lWndoIsDoppelganger; // Flag for doppelganger window test
        int iDoppelganger;
        bool ldoTransform;
        Real64 roldAspectRatio;
        Real64 rnewAspectRatio;
        Real64 Xo;
        Real64 XnoRot;
        Real64 Xtrans;
        Real64 Yo;
        Real64 YnoRot;
        Real64 Ytrans;

        // Formats
        static constexpr std::string_view Format_901("Version EPlus : DElight input generated from EnergyPlus processed input {}\n");
        static constexpr std::string_view Format_902(
            "\nBuilding_Name {}\nSite_Latitude  {:12.4F}\nSite_Longitude {:12.4F}\nSite_Altitude  {:12.4F}\nBldg_Azimuth   {:12.4F}\nSite_Time_Zone "
            "{:12.4F}\nAtm_Moisture  0.07 0.07 0.07 0.07 0.07 0.07 0.07 0.07 0.07 0.07 0.07 0.07\nAtm_Turbidity 0.12 0.12 0.12 0.12 0.12 0.12 0.12 "
            "0.12 0.12 0.12 0.12 0.12\n");
        static constexpr std::string_view Format_903("\nZONES\nN_Zones {:4}\n");
        static constexpr std::string_view Format_904(
            "\nZONE DATA\nZone {}\nBldgSystem_Zone_Origin {:12.4F}{:12.4F}{:12.4F}\nZone_Azimuth    {:12.4F}\nZone_Multiplier {:5}\nZone_Floor_Area "
            "{:12.4F}\nZone_Volume     {:12.4F}\nZone_Installed_Lighting {:12.4F}\nMin_Input_Power    {:12.4F}\nMin_Light_Fraction "
            "{:12.4F}\nLight_Ctrl_Steps   {:3}\nLight_Ctrl_Prob    {:12.4F}\nView_Azimuth  0.0\nMax_Grid_Node_Area {:12.4F}\n");
        static constexpr std::string_view Format_905("\nZONE LIGHTING SCHEDULES\nN_Lt_Scheds 0\n");
        static constexpr std::string_view Format_906("\nZONE SURFACES\nN_Surfaces {:4}\n");
        static constexpr std::string_view Format_907("\nZONE SURFACE DATA\nSurface {}\nWCS_Azimuth {:12.4F}\nWCS_Tilt    {:12.4F}\nVis_Refl    "
                                                     "{:12.4F}\nExt_Refl    {:12.4F}\nGnd_Refl     0.2\nN_WCS_Vertices {:6}\n");
        static constexpr std::string_view Format_908("Vertex {:12.4F}{:12.4F}{:12.4F}\n");
        static constexpr std::string_view Format_909("\nSURFACE WINDOWS\nN_Windows {:6}\n");
        static constexpr std::string_view Format_910(
            "\nSURFACE WINDOW DATA\nWindow     {}\nGlass_Type {:8}\nShade_Flag   0\nOverhang_Fin_Depth    0.0 0.0 "
            "0.0\nOverhang_Fin_Distance 0.0 0.0 0.0\nN_WCS_Vertices {:4}\n");
        static constexpr std::string_view Format_911("\nSURFACE CFS\nN_CFS {:6}\n");
        static constexpr std::string_view Format_915(
            "\nCOMPLEX FENESTRATION DATA\nCFS_Name   {}\nCFS_Type   {}\nFenestration_Rotation {:12.4F}\nN_WCS_Vertices {:4}\n");
        static constexpr std::string_view Format_912("\nZONE REFERENCE POINTS\nN_Ref_Pts {:4}\n");
        static constexpr std::string_view Format_913(
            "\nZONE REFERENCE POINT DATA\nReference_Point {}\nRefPt_WCS_Coords {:12.4F}{:12.4F}{:12.4F}\nZone_Fraction "
            "{:12.4F}\nLight_Set_Pt {:12.4F}\nLight_Ctrl_Type {:4}\n");
        static constexpr std::string_view Format_914("\nBUILDING SHADES\nN_BShades 0\n");
        static constexpr std::string_view Format_920("\nLIBRARY DATA\nGLASS TYPES\nN_Glass_Types {:4}\n");
        static constexpr std::string_view Format_921(
            "\nGLASS TYPE DATA\nName {:6}\nEPlusDiffuse_Transmittance   {:12.4F}\nEPlusDiffuse_Int_Reflectance "
            "{:12.4F}\nEPlus_Vis_Trans_Coeff_1 {:17.9F}\nEPlus_Vis_Trans_Coeff_2 {:17.9F}\nEPlus_Vis_Trans_Coeff_3 "
            "{:17.9F}\nEPlus_Vis_Trans_Coeff_4 {:17.9F}\nEPlus_Vis_Trans_Coeff_5 {:17.9F}\nEPlus_Vis_Trans_Coeff_6 {:17.9F}\n");

        // Init the ErrorsFound flag
        ErrorsFound = false;

        GetInputDElightComplexFenestration(state, ErrorsFound);

        CheckForGeometricTransform(state, ldoTransform, roldAspectRatio, rnewAspectRatio);

        // Init the counter for Thermal Zones with hosted Daylighting:DElight objects
        iNumDElightZones = 0;

        // Init the counter for Window Construction types for writing to Library Data section of DElight input file
        int iNumWndoConsts = 0;

        // Open a file for writing DElight input from EnergyPlus data
        auto delightInFile = state.files.delightIn.open(state, "DElightInputGenerator", state.files.outputControl.delightin); // (THIS_AUTO_OK)

        // Start of DElight input file
        print(delightInFile, Format_901, state.dataStrGlobals->CurrentDateTime);

        // Building Data Section retrieved from DataHeatBalance and DataEnvironment modules
        // Remove any blanks from the Building Name for ease of input to DElight
        cNameWOBlanks = ReplaceBlanksWithUnderscores(state.dataHeatBal->BuildingName);
        print(delightInFile,
              Format_902,
              cNameWOBlanks,
              state.dataEnvrn->Latitude,
              state.dataEnvrn->Longitude,
              state.dataEnvrn->Elevation * M2FT,
              state.dataHeatBal->BuildingAzimuth,
              state.dataEnvrn->TimeZoneNumber);

        // Calc cos and sin of Building Relative North values for later use in transforming Reference Point coordinates
        CosBldgRelNorth = std::cos(-state.dataHeatBal->BuildingAzimuth * Constant::DegToRad);
        SinBldgRelNorth = std::sin(-state.dataHeatBal->BuildingAzimuth * Constant::DegToRad);

        // Loop through the Daylighting:Controls objects that use DElight checking for a host Zone
        for (auto &znDayl : state.dataDayltg->daylightControl) {
            if (znDayl.DaylightMethod == Dayltg::DaylightingMethod::DElight) {

                // Register Error if 0 DElight RefPts have been input for valid DElight object
                if (znDayl.TotalDaylRefPoints == 0) {
                    ShowSevereError(state, format("No Reference Points input for daylighting zone using DElight ={}", znDayl.Name));
                    ErrorsFound = true;
                }

                // Register Warning if more than 100 DElight RefPts have been input for valid DElight object
                if (znDayl.TotalDaylRefPoints > 100) {
                    // Restrict to 100 Ref Pt maximum
                    znDayl.TotalDaylRefPoints = 100;
                    ShowWarningError(state, format("Maximum of 100 Reference Points exceeded for daylighting zone using DElight ={}", znDayl.Name));
                    ShowWarningError(state, "  Only first 100 Reference Points included in DElight analysis");
                }

                // Should already be allocated
                assert((int)znDayl.refPts.size() == znDayl.TotalDaylRefPoints);
                for (auto &refPt : znDayl.refPts) {
                    refPt.absCoords = {0.0, 0.0, 0.0};
                    refPt.lums[(int)Lum::Illum] = 0.0;
                    refPt.glareIndex = 0.0;
                }

                // RJH 2008-03-07: Allocate and Init DaylIllumAtRefPt array for this DElight zone
                // znDayl.DaylIllumAtRefPt.allocate(znDayl.TotalDaylRefPoints);
                // znDayl.DaylIllumAtRefPt = 0.0;
                // The following not used in DElight but allocated for convenience
                // znDayl.GlareIndexAtRefPt.allocate(znDayl.TotalDaylRefPoints);
                // znDayl.GlareIndexAtRefPt = 0.0;

                // Increment counter of Thermal Zones with valid hosted DElight object
                ++iNumDElightZones;
            }
        } // traverse ZoneDaylight array

        // Zone Data Section
        print(delightInFile, Format_903, iNumDElightZones);

        // Loop through the Daylighting:DElight objects searching for a match to the current Zone

        for (auto &znDayl : state.dataDayltg->daylightControl) {
            if (znDayl.DaylightMethod == Dayltg::DaylightingMethod::DElight) {
                int const izone = Util::FindItemInList(znDayl.ZoneName, state.dataHeatBal->Zone);
                if (izone != 0) {

                    rLightLevel = GetDesignLightingLevelForZone(state, izone);
                    CheckLightsReplaceableMinMaxForZone(state, izone);
                    auto &zn(state.dataHeatBal->Zone(izone));

                    // Write this Zone to the DElight input file
                    // Remove any blanks from the Zone Name for ease of input to DElight
                    cNameWOBlanks = ReplaceBlanksWithUnderscores(zn.Name);
                    print(delightInFile,
                          Format_904,
                          cNameWOBlanks,
                          zn.OriginX * M2FT,
                          zn.OriginY * M2FT,
                          zn.OriginZ * M2FT,
                          zn.RelNorth,
                          zn.Multiplier * zn.ListMultiplier,
                          zn.FloorArea * M22FT2,
                          zn.Volume * M32FT3,
                          rLightLevel / (zn.FloorArea * M22FT2 + 0.00001),
                          znDayl.MinPowerFraction,
                          znDayl.MinLightFraction,
                          znDayl.LightControlSteps,
                          znDayl.LightControlProbability,
                          znDayl.DElightGriddingResolution * M22FT2);

                    // Calc cos and sin of Zone Relative North values for later use in transforming Reference Point coordinates
                    CosZoneRelNorth = std::cos(-zn.RelNorth * Constant::DegToRad);
                    SinZoneRelNorth = std::sin(-zn.RelNorth * Constant::DegToRad);

                    // Zone Lighting Schedule Data Section
                    // NOTE: Schedules are not required since hourly values are retrieved from EnergyPlus as needed
                    print(delightInFile, Format_905);

                    // Zone Surface Data Section
                    // Count the number of opaque surfaces bounding the current zone
                    iNumOpaqueSurfs = 0;
                    for (int spaceNum : zn.spaceIndexes) {
                        auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                        for (int isurf = thisSpace.HTSurfaceFirst; isurf <= thisSpace.HTSurfaceLast; ++isurf) {
                            auto const &surf(state.dataSurface->Surface(isurf));
                            if (surf.Class == SurfaceClass::Wall) {
                                ++iNumOpaqueSurfs;
                            }
                            if (surf.Class == SurfaceClass::Roof) {
                                ++iNumOpaqueSurfs;
                            }
                            if (surf.Class == SurfaceClass::Floor) {
                                ++iNumOpaqueSurfs;
                            }
                        }
                    } // Zone Opaque Surface loop

                    print(delightInFile, Format_906, iNumOpaqueSurfs);

                    // Write each opaque bounding Surface to the DElight input file
                    for (int spaceNum : zn.spaceIndexes) {
                        auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                        int const iSurfaceFirst = thisSpace.HTSurfaceFirst;
                        int const iSurfaceLast = thisSpace.HTSurfaceLast;
                        for (int isurf = iSurfaceFirst; isurf <= iSurfaceLast; ++isurf) {

                            auto &surf(state.dataSurface->Surface(isurf));

                            // Only process "opaque bounding" surface types
                            if ((surf.Class == SurfaceClass::Wall) || (surf.Class == SurfaceClass::Roof) || (surf.Class == SurfaceClass::Floor)) {

                                // Get the Construction index for this Surface
                                iconstruct = surf.Construction;

                                // Is this Surface exposed to the exterior?
                                if (surf.ExtSolar) {
                                    // Get the index for the outside (i.e., 1st) Material Layer for this Construction
                                    iMatlLayer = state.dataConstruction->Construct(iconstruct).LayerPoint(1);
                                    // Get the outside visible reflectance of this material layer
                                    // (since Construct(iconstruct)%ReflectVisDiffFront always appears to == 0.0)
                                    rExtVisRefl = 1.0 - state.dataMaterial->materials(iMatlLayer)->AbsorpVisible;
                                } else {
                                    rExtVisRefl = 0.0;
                                }

                                // Remove any blanks from the Surface Name for ease of input to DElight
                                cNameWOBlanks = ReplaceBlanksWithUnderscores(surf.Name);
                                print(delightInFile,
                                      Format_907,
                                      cNameWOBlanks,
                                      surf.Azimuth,
                                      surf.Tilt,
                                      state.dataConstruction->Construct(iconstruct).ReflectVisDiffBack,
                                      rExtVisRefl,
                                      surf.Sides);

                                // Write out the vertex coordinates for each vertex
                                for (int ivert = 1; ivert <= surf.Sides; ++ivert) {
                                    print(delightInFile,
                                          Format_908,
                                          surf.Vertex(ivert).x * M2FT,
                                          surf.Vertex(ivert).y * M2FT,
                                          surf.Vertex(ivert).z * M2FT);
                                }

                                // Count each Window hosted by the current opaque bounding Surface
                                iNumWindows = 0;
                                for (int iwndo = iSurfaceFirst; iwndo <= iSurfaceLast; ++iwndo) {
                                    if (state.dataSurface->Surface(iwndo).Class == SurfaceClass::Window) {
                                        auto &wndo(state.dataSurface->Surface(iwndo));
                                        if (wndo.BaseSurfName == surf.Name) {

                                            // Error if window has multiplier > 1 since this causes incorrect illuminance calc
                                            if (wndo.Multiplier > 1.0) {
                                                ShowSevereError(
                                                    state,
                                                    format(
                                                        "Multiplier > 1.0 for window {} not allowed since it is in a zone with DElight daylighting.",
                                                        wndo.Name));
                                                ErrorsFound = true;
                                            }

                                            // Error if window has a shading device (blind/shade/screen) since
                                            // DElight cannot perform dynamic shading device deployment
                                            if (wndo.HasShadeControl) {
                                                ShowSevereError(state,
                                                                format("Shading Device on window {} dynamic control is not supported in a zone with "
                                                                       "DElight daylighting.",
                                                                       wndo.Name));
                                                ErrorsFound = true;
                                            }

                                            // Loop through all Doppelganger Surface Names to ignore these Windows
                                            lWndoIsDoppelganger = false;
                                            for (auto const &cfs : state.dataDayltg->DElightComplexFene) {

                                                // Is the current Window Surface a Doppelganger?
                                                if (wndo.Name == cfs.wndwName) {
                                                    // Ignore this Doppelganger Window
                                                    lWndoIsDoppelganger = true;
                                                }

                                            } // CFS object loop A

                                            if (!lWndoIsDoppelganger) {
                                                ++iNumWindows;
                                            }

                                        } // Surface hosts Window test
                                    } // Window test
                                } // Window loop

                                print(delightInFile, Format_909, iNumWindows);

                                // If the current opaque bounding Surface hosts Windows,
                                // then write each hosted Window to the DElight input file
                                // and track the Window Construction type for later writing
                                if (iNumWindows > 0) {
                                    for (int iwndo2 = iSurfaceFirst; iwndo2 <= iSurfaceLast; ++iwndo2) {
                                        if (state.dataSurface->Surface(iwndo2).Class == SurfaceClass::Window) {

                                            auto &wndo2(state.dataSurface->Surface(iwndo2));

                                            if (wndo2.BaseSurfName == surf.Name) {

                                                // Loop through all Doppelganger Surface Names to ignore these Windows
                                                lWndoIsDoppelganger = false;

                                                for (auto const &cfs : state.dataDayltg->DElightComplexFene) {

                                                    // Is the current Window Surface a Doppelganger?
                                                    if (wndo2.Name == cfs.wndwName) {
                                                        // Ignore this Doppelganger Window
                                                        lWndoIsDoppelganger = true;
                                                    }

                                                } // CFS object loop A

                                                if (!lWndoIsDoppelganger) {

                                                    // Track unique window construction types here for later writing to
                                                    // the library section of DElight input file

                                                    // Get the Construction index for this Window Surface
                                                    iconstruct = wndo2.Construction;

                                                    // Has the current Construction index been encountered before?
                                                    lWndoConstFound = false;
                                                    for (int iconst = 1; iconst <= iNumWndoConsts; ++iconst) {
                                                        if (iconstruct == iWndoConstIndexes(iconst)) {
                                                            lWndoConstFound = true;
                                                        }
                                                    }
                                                    if (!lWndoConstFound) {
                                                        ++iNumWndoConsts;
                                                        iWndoConstIndexes(iNumWndoConsts) = iconstruct;
                                                    }

                                                    // Write this Window to the DElight input file
                                                    // Remove any blanks from the Window Surface Name for ease of input to DElight
                                                    cNameWOBlanks = ReplaceBlanksWithUnderscores(wndo2.Name);
                                                    print(delightInFile, Format_910, cNameWOBlanks, iconstruct + 10000, wndo2.Sides);
                                                    // Use WndoConstIndex + 10000 as the Glass Type Name
                                                    // to differentiate EPlus glass types within DElight

                                                    // Write out the vertex coordinates for each vertex
                                                    for (int ivert = 1; ivert <= wndo2.Sides; ++ivert) {
                                                        print(delightInFile,
                                                              Format_908,
                                                              wndo2.Vertex(ivert).x * M2FT,
                                                              wndo2.Vertex(ivert).y * M2FT,
                                                              wndo2.Vertex(ivert).z * M2FT);
                                                    }
                                                } //! lWndoIsDoppelganger
                                            } // Surface hosts Window2 test
                                        } // Window2 Class test
                                    } // Window2 loop
                                } // Hosted Windows test

                                // Write the number of CFS hosted by the current Opaque Bounding Surface
                                iHostedCFS = 0;

                                // Loop through the input CFS objects searching for a match to the current Opaque Bounding Surface
                                for (auto const &cfs : state.dataDayltg->DElightComplexFene) {

                                    // Does the current Opaque Bounding Surface host the current CFS object?
                                    if (surf.Name == cfs.surfName) {
                                        // Count this hosted CFS
                                        ++iHostedCFS;
                                    }
                                } // CFS object loop 1

                                print(delightInFile, Format_911, iHostedCFS);

                                // Now write each of the hosted CFS data
                                // Loop through the input CFS objects searching for a match to the current Opaque Bounding Surface
                                for (auto const &cfs : state.dataDayltg->DElightComplexFene) {

                                    // Does the current Opaque Bounding Surface host the current CFS object?
                                    if (surf.Name == cfs.surfName) {

                                        // Get the Doppelganger surface for this CFS
                                        iDoppelganger = 0;
                                        for (int iwndo3 = iSurfaceFirst; iwndo3 <= iSurfaceLast; ++iwndo3) {

                                            auto const &wndo3(state.dataSurface->Surface(iwndo3));

                                            if (wndo3.Class == SurfaceClass::Window) {

                                                // Is the current Window Surface the Doppelganger for the current CFS?
                                                if (wndo3.Name == cfs.wndwName) {
                                                    // Store the window surface index for future reference
                                                    iDoppelganger = iwndo3;
                                                }
                                            }
                                        }

                                        // Make sure that a valid Doppelganger surface exists
                                        if (iDoppelganger > 0) {

                                            // Write the data for this hosted CFS
                                            auto &doppelgangerSurf(state.dataSurface->Surface(iDoppelganger));

                                            // Remove any blanks from the CFS Name for ease of input to DElight
                                            cNameWOBlanks = ReplaceBlanksWithUnderscores(cfs.Name);
                                            print(
                                                delightInFile, Format_915, cNameWOBlanks, cfs.ComplexFeneType, cfs.feneRota, doppelgangerSurf.Sides);

                                            // Write out the vertex coordinates for each vertex
                                            for (int ivert = 1; ivert <= doppelgangerSurf.Sides; ++ivert) {
                                                print(delightInFile,
                                                      Format_908,
                                                      doppelgangerSurf.Vertex(ivert).x * M2FT,
                                                      doppelgangerSurf.Vertex(ivert).y * M2FT,
                                                      doppelgangerSurf.Vertex(ivert).z * M2FT);
                                            }
                                        }
                                        // Register Error if there is no valid Doppelganger for current Complex Fenestration
                                        if (iDoppelganger == 0) {
                                            ShowSevereError(state,
                                                            format("No Doppelganger Window Surface found for Complex Fenestration ={}", cfs.Name));
                                            ErrorsFound = true;
                                        }
                                    } // The current Opaque Bounding Surface hosts the current CFS object?
                                } // CFS object loop 2
                            } // Opaque Bounding Surface test
                        }
                    } // Zone Surface loop

                    // Write ZONE REFERENCE POINTS
                    print(delightInFile, Format_912, znDayl.TotalDaylRefPoints);

                    // Loop through the Daylighting:DElight:Reference Point objects checking for the current DElight Zone host
                    for (auto &refPt : state.dataDayltg->DaylRefPt) {

                        // Is this RefPt hosted by current DElight Zone?
                        if (izone == refPt.ZoneNum) {
                            auto &thisZone = state.dataHeatBal->Zone(izone);

                            // Limit to maximum of 100 RefPts
                            if (znDayl.TotalDaylRefPoints <= 100) {

                                if (state.dataSurface->DaylRefWorldCoordSystem) {
                                    RefPt_WCS_Coord = refPt.coords;
                                } else {
                                    // Transform reference point coordinates into building coordinate system
                                    Xb = refPt.coords.x * CosZoneRelNorth - refPt.coords.y * SinZoneRelNorth + thisZone.OriginX;
                                    Yb = refPt.coords.x * SinZoneRelNorth + refPt.coords.y * CosZoneRelNorth + thisZone.OriginY;
                                    // Transform into World Coordinate System
                                    RefPt_WCS_Coord.x = Xb * CosBldgRelNorth - Yb * SinBldgRelNorth;
                                    RefPt_WCS_Coord.y = Xb * SinBldgRelNorth + Yb * CosBldgRelNorth;
                                    RefPt_WCS_Coord.z = refPt.coords.z + thisZone.OriginZ;
                                    if (ldoTransform) {         // Geometry transform
                                        Xo = RefPt_WCS_Coord.x; // world coordinates.... shifted by relative north angle...
                                        Yo = RefPt_WCS_Coord.y;
                                        // next derotate the building
                                        XnoRot = Xo * CosBldgRelNorth + Yo * SinBldgRelNorth;
                                        YnoRot = Yo * CosBldgRelNorth - Xo * SinBldgRelNorth;
                                        // translate
                                        Xtrans = XnoRot * std::sqrt(rnewAspectRatio / roldAspectRatio);
                                        Ytrans = YnoRot * std::sqrt(roldAspectRatio / rnewAspectRatio);
                                        // rerotate
                                        RefPt_WCS_Coord.x = Xtrans * CosBldgRelNorth - Ytrans * SinBldgRelNorth;

                                        RefPt_WCS_Coord.y = Xtrans * SinBldgRelNorth + Ytrans * CosBldgRelNorth;
                                    }
                                }
                                znDayl.refPts(refPt.indexToFracAndIllum).absCoords = RefPt_WCS_Coord;

                                // Validate that Reference Point coordinates are within the host Zone
                                if (RefPt_WCS_Coord.x < thisZone.MinimumX || RefPt_WCS_Coord.x > thisZone.MaximumX) {
                                    ShowSevereError(state,
                                                    format("DElightInputGenerator:Reference point X Value outside Zone Min/Max X, Zone={}", zn.Name));
                                    ShowContinueError(state,
                                                      format("...X Reference Point= {:.2R}, Zone Minimum X= {:.2R}, Zone Maximum X= {:.2R}",
                                                             thisZone.MinimumX,
                                                             RefPt_WCS_Coord.x,
                                                             thisZone.MaximumX));
                                    ErrorsFound = true;
                                }
                                if (RefPt_WCS_Coord.y < thisZone.MinimumY || RefPt_WCS_Coord.y > thisZone.MaximumY) {
                                    ShowSevereError(state,
                                                    format("DElightInputGenerator:Reference point Y Value outside Zone Min/Max Y, Zone={}", zn.Name));
                                    ShowContinueError(state,
                                                      format("...Y Reference Point= {:.2R}, Zone Minimum Y= {:.2R}, Zone Maximum Y= {:.2R}",
                                                             thisZone.MinimumY,
                                                             RefPt_WCS_Coord.y,
                                                             thisZone.MaximumY));
                                    ErrorsFound = true;
                                }
                                if (RefPt_WCS_Coord.z < state.dataHeatBal->Zone(izone).MinimumZ || RefPt_WCS_Coord.z > thisZone.MaximumZ) {
                                    ShowSevereError(
                                        state,
                                        format("DElightInputGenerator:Reference point Z Value outside Zone Min/Max Z, Zone={}", thisZone.Name));
                                    ShowContinueError(state,
                                                      format("...Z Reference Point= {:.2R}, Zone Minimum Z= {:.2R}, Zone Maximum Z= {:.2R}",
                                                             thisZone.MinimumZ,
                                                             RefPt_WCS_Coord.z,
                                                             thisZone.MaximumZ));
                                    ErrorsFound = true;
                                }

                                // Write this RefPt to the DElight input file

                                // Remove any blanks from the RefPt Name for ease of input to DElight
                                cNameWOBlanks = ReplaceBlanksWithUnderscores(refPt.Name);
                                if (refPt.indexToFracAndIllum != 0) {
                                    print(delightInFile,
                                          Format_913,
                                          cNameWOBlanks,
                                          RefPt_WCS_Coord.x * M2FT,
                                          RefPt_WCS_Coord.y * M2FT,
                                          RefPt_WCS_Coord.z * M2FT,
                                          znDayl.refPts(refPt.indexToFracAndIllum).fracZoneDaylit,
                                          znDayl.refPts(refPt.indexToFracAndIllum).illumSetPoint * LUX2FC,
                                          znDayl.LightControlType);
                                    // RJH 2008-03-07: Set up DaylIllumAtRefPt for output for this DElight zone RefPt
                                    SetupOutputVariable(state,
                                                        "Daylighting Reference Point Illuminance",
                                                        Constant::Units::lux,
                                                        znDayl.refPts(refPt.indexToFracAndIllum).lums[(int)Lum::Illum],
                                                        OutputProcessor::TimeStepType::Zone,
                                                        OutputProcessor::StoreType::Average,
                                                        refPt.Name);
                                } else {
                                    print(delightInFile,
                                          Format_913,
                                          cNameWOBlanks,
                                          RefPt_WCS_Coord.x * M2FT,
                                          RefPt_WCS_Coord.y * M2FT,
                                          RefPt_WCS_Coord.z * M2FT,
                                          0.0,
                                          0.0 * LUX2FC,
                                          znDayl.LightControlType); // should never happen but just in case send zero fraction and illuminance
                                }
                            } // Max 100 RefPt test
                        } // RefPt in current DElight Zone test
                    } // traverse reference points loop
                } // if in a zone
            } // Zone hosts DElight object test
        } // traverse ZoneDayLight object loop

        // Write BUILDING SHADES
        print(delightInFile, Format_914);

        // Write LIBRARY DATA
        print(delightInFile, Format_920, iNumWndoConsts);

        // Write GLASS TYPES
        // VisBeamCoeffs are processed in EPlus by POLYF() function
        // Use WndoConstIndex + 10000 as the Glass Type Name to differentiate EPlus glass types within DElight
        for (int iconst = 1; iconst <= iNumWndoConsts; ++iconst) {
            print(delightInFile,
                  Format_921,
                  iWndoConstIndexes(iconst) + 10000,
                  state.dataConstruction->Construct(iWndoConstIndexes(iconst)).TransDiffVis,
                  state.dataConstruction->Construct(iWndoConstIndexes(iconst)).ReflectVisDiffBack,
                  state.dataConstruction->Construct(iWndoConstIndexes(iconst)).TransVisBeamCoef[0],
                  state.dataConstruction->Construct(iWndoConstIndexes(iconst)).TransVisBeamCoef[1],
                  state.dataConstruction->Construct(iWndoConstIndexes(iconst)).TransVisBeamCoef[2],
                  state.dataConstruction->Construct(iWndoConstIndexes(iconst)).TransVisBeamCoef[3],
                  state.dataConstruction->Construct(iWndoConstIndexes(iconst)).TransVisBeamCoef[4],
                  state.dataConstruction->Construct(iWndoConstIndexes(iconst)).TransVisBeamCoef[5]);

        } // Glass Type loop

        if (ErrorsFound) {
            ShowFatalError(state, "Problems with Daylighting:DElight input, see previous error messages");
        }
    }

    void GenerateDElightDaylightCoefficients(Real64 &dLatitude, int &iErrorFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   September 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // The purpose of this subroutine is to provide an envelop to the DElightDaylightCoefficients routine

        delightdaylightcoefficients(dLatitude, &iErrorFlag);
    }

    void GetInputDElightComplexFenestration(EnergyPlusData &state, bool &ErrorsFound)
    {
        // Perform GetInput function for the Daylighting:DELight:ComplexFenestration object
        // Glazer - July 2016

        int NumAlpha;
        int NumNumber;
        int IOStat;
        int CFSNum = 0;

        constexpr std::string_view cCurrentModuleObject("Daylighting:DELight:ComplexFenestration");

        int TotDElightCFS = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        state.dataDayltg->DElightComplexFene.allocate(TotDElightCFS);
        for (auto &cfs : state.dataDayltg->DElightComplexFene) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     ++CFSNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlpha,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            cfs.Name = state.dataIPShortCut->cAlphaArgs(1);
            cfs.ComplexFeneType = state.dataIPShortCut->cAlphaArgs(2);
            cfs.surfName = state.dataIPShortCut->cAlphaArgs(3);
            if (Util::FindItemInList(cfs.surfName, state.dataSurface->Surface) == 0) {
                ShowSevereError(state,
                                format("{}{}",
                                       cCurrentModuleObject,
                                       ": " + cfs.Name + ", invalid " + state.dataIPShortCut->cAlphaFieldNames(3) + "=\"" + cfs.surfName + "\"."));
                ErrorsFound = true;
            }
            cfs.wndwName = state.dataIPShortCut->cAlphaArgs(4);
            if (Util::FindItemInList(cfs.surfName, state.dataSurface->Surface) == 0) {
                ShowSevereError(state,
                                format("{}{}",
                                       cCurrentModuleObject,
                                       ": " + cfs.Name + ", invalid " + state.dataIPShortCut->cAlphaFieldNames(4) + "=\"" + cfs.wndwName + "\"."));
                ErrorsFound = true;
            }
            cfs.feneRota = state.dataIPShortCut->rNumericArgs(1);
            if (cfs.feneRota < 0. || cfs.feneRota > 360.) {
                ShowSevereError(state,
                                format("{}{}",
                                       cCurrentModuleObject,
                                       ": " + cfs.Name + ", invalid " + state.dataIPShortCut->cNumericFieldNames(1) + " outside of range 0 to 360."));
                ErrorsFound = true;
            }
        }
    }

    void CheckForGeometricTransform(EnergyPlusData &state, bool &doTransform, Real64 &OldAspectRatio, Real64 &NewAspectRatio)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // check for geometrytransform in the daylighting access for reference points

        // METHODOLOGY EMPLOYED:
        // once reference points  have been converted to WCS,
        //  change them to reflect a different aspect
        // ratio for the entire building based on user input.

        // SUBROUTINE PARAMETER DEFINITIONS:
        constexpr std::string_view CurrentModuleObject("GeometryTransform");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D_string cAlphas(1);
        Array1D<Real64> rNumerics(2);

        // begin execution
        // get user input...
        doTransform = false;
        OldAspectRatio = 1.0;
        NewAspectRatio = 1.0;

        if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject) == 1) {
            int NAlphas;
            int NNum;
            int IOStat;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     1,
                                                                     cAlphas,
                                                                     NAlphas,
                                                                     rNumerics,
                                                                     NNum,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            OldAspectRatio = rNumerics(1);
            NewAspectRatio = rNumerics(2);
            if (cAlphas(1) != "XY") {
                ShowWarningError(
                    state,
                    format("{}{}", CurrentModuleObject, ": invalid " + state.dataIPShortCut->cAlphaFieldNames(1) + "=" + cAlphas(1) + "...ignored."));
            }
            doTransform = true;
            state.dataSurface->AspectTransform = true;
        }
        if (state.dataSurface->WorldCoordSystem) {
            doTransform = false;
            state.dataSurface->AspectTransform = false;
        }
    }

    std::string ReplaceBlanksWithUnderscores(std::string const &InputString) // Input String
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Robert J. Hitchcock
        //       DATE WRITTEN   August 2003

        // PURPOSE OF THIS SUBROUTINE:
        // This function returns a representation of the InputString with blanks replaced with underscores.

        // METHODOLOGY EMPLOYED:
        // Uses the std::replace function from the C++ library

        // FUNCTION LOCAL VARIABLE DECLARATIONS:

        std::string ResultString(trimmed(InputString));
        std::replace(ResultString.begin(), ResultString.end(), ' ', '_');
        return ResultString;
    }

    void DElightElecLtgCtrl(int iNameLength,
                            std::string const &cZoneName,
                            Real64 dBldgLat,
                            Real64 dHISKF,
                            Real64 dHISUNF,
                            Real64 dCloudFraction,
                            Real64 dSOLCOSX,
                            Real64 dSOLCOSY,
                            Real64 dSOLCOSZ,
                            Real64 &pdPowerReducFac,
                            int piErrorFlag)
    {
        std::vector<char> zoneNameArr(getCharArrayFromString(cZoneName));
        delightelecltgctrl(
            iNameLength, &zoneNameArr[0], dBldgLat, dHISKF, dHISUNF, dCloudFraction, dSOLCOSX, dSOLCOSY, dSOLCOSZ, &pdPowerReducFac, &piErrorFlag);
    }

    std::vector<char> getCharArrayFromString(std::string const &originalString)
    {
        std::vector<char> returnVal(originalString.begin(), originalString.end());
        returnVal.push_back('\0'); // get null terminated string of chars
        return returnVal;
    }

    std::string getStringFromCharArray(std::vector<char> const &originalCharArray)
    {
        return std::string(originalCharArray.begin(), originalCharArray.end());
    }

} // namespace DElightManagerF

} // namespace EnergyPlus
