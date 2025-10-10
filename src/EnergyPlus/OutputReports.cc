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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/DXFEarClipping.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSurfaceColors.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/OutputReports.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

void ReportSurfaces(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   February 1999
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calls several optional routines to report
    // the surfaces to output formats that can render the data
    // into a descriptive picture.

    // METHODOLOGY EMPLOYED:
    // Use a REPORT command to determine if there should be
    // a file created.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    state.dataErrTracking->AskForSurfacesReport = false;

    int SurfDetails = 0;
    bool SurfVert = false;
    bool SurfDet = false;
    bool DXFDone = false;
    bool VRMLDone = false;
    std::string Option1;
    std::string Option2;
    bool DoReport;

    General::ScanForReports(state, "Surfaces", DoReport, "Lines", Option1);
    if (DoReport) {
        LinesOut(state, Option1);
    }

    General::ScanForReports(state, "Surfaces", DoReport, "Vertices");
    if (DoReport) {
        if (!SurfVert) {
            ++SurfDetails;
            SurfVert = true;
        }
    }

    General::ScanForReports(state, "Surfaces", DoReport, "Details");
    if (DoReport) {
        if (!SurfDet) {
            SurfDetails += 10;
            SurfDet = true;
        }
    }

    General::ScanForReports(state, "Surfaces", DoReport, "DetailsWithVertices");
    if (DoReport) {
        if (!SurfDet) {
            SurfDetails += 10;
            SurfDet = true;
        }
        if (!SurfVert) {
            ++SurfDetails;
            SurfVert = true;
        }
    }

    General::ScanForReports(state, "Surfaces", DoReport, "DXF", Option1, Option2);
    if (DoReport) {
        if (!DXFDone) {
            if (!Option2.empty()) {
                DataSurfaceColors::SetUpSchemeColors(state, Option2, "DXF");
            }
            DXFOut(state, Option1, Option2);
            DXFDone = true;
        } else {
            ShowWarningError(state, format("ReportSurfaces: DXF output already generated.  DXF with option=[{}] will not be generated.", Option1));
        }
    }

    General::ScanForReports(state, "Surfaces", DoReport, "DXF:WireFrame", Option1, Option2);
    if (DoReport) {
        if (!DXFDone) {
            if (!Option2.empty()) {
                DataSurfaceColors::SetUpSchemeColors(state, Option2, "DXF");
            }
            DXFOutWireFrame(state, Option2);
            DXFDone = true;
        } else {
            ShowWarningError(state, "ReportSurfaces: DXF output already generated.  DXF:WireFrame will not be generated.");
        }
    }

    General::ScanForReports(state, "Surfaces", DoReport, "VRML", Option1, Option2);
    if (DoReport) {
        if (!VRMLDone) {
            VRMLOut(state, Option1, Option2);
            VRMLDone = true;
        } else {
            ShowWarningError(state, format("ReportSurfaces: VRML output already generated.  VRML with option=[{}] will not be generated.", Option1));
        }
    }

    General::ScanForReports(state, "Surfaces", DoReport, "CostInfo");
    if (DoReport) {
        CostInfoOut(state);
    }

    if (SurfDet || SurfVert) {
        DetailsForSurfaces(state, SurfDetails);
    }
}

void LinesOut(EnergyPlusData &state, std::string const &option)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   March 1999
    //       MODIFIED       March 2006 -- add option for "IDF segments out"
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine produces a file of lines in the surfaces.

    // METHODOLOGY EMPLOYED:
    // Use the surface absolute coordinate information to produce
    // lines.

    static constexpr std::string_view vertexstring("X,Y,Z ==> Vertex");

    if (state.dataSurface->TotSurfaces > 0 && !allocated(state.dataSurface->Surface)) {
        // no error needed, probably in end processing, just return
        return;
    }

    if (state.dataOutputReports->optiondone) {
        ShowWarningError(state,
                         format("Report of Surfaces/Lines Option has already been completed with option={}", state.dataOutputReports->lastoption));
        ShowContinueError(state, format("..option=\"{}\" will not be done this time.", option));
        return;
    }

    state.dataOutputReports->lastoption = option;
    state.dataOutputReports->optiondone = true;

    auto slnfile = state.files.sln.open(state, "LinesOut", state.files.outputControl.sln);

    if (option != "IDF") {
        for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
            auto &thisSurface = state.dataSurface->Surface(surf);
            if (thisSurface.Class == DataSurfaces::SurfaceClass::IntMass) {
                continue;
            }
            if (thisSurface.Sides == 0) {
                continue;
            }
            print<FormatSyntax::FMT>(slnfile, "{}:{}\n", thisSurface.ZoneName, thisSurface.Name);
            for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                static constexpr std::string_view fmt700("{:10.2F},{:10.2F},{:10.2F},{:10.2F},{:10.2F},{:10.2F}\n");

                if (vert != thisSurface.Sides) {
                    print<check_syntax(fmt700)>(slnfile,
                                                fmt700,
                                                thisSurface.Vertex(vert).x,
                                                thisSurface.Vertex(vert).y,
                                                thisSurface.Vertex(vert).z,
                                                thisSurface.Vertex(vert + 1).x,
                                                thisSurface.Vertex(vert + 1).y,
                                                thisSurface.Vertex(vert + 1).z);
                } else {
                    print<check_syntax(fmt700)>(slnfile,
                                                fmt700,
                                                thisSurface.Vertex(vert).x,
                                                thisSurface.Vertex(vert).y,
                                                thisSurface.Vertex(vert).z,
                                                thisSurface.Vertex(1).x,
                                                thisSurface.Vertex(1).y,
                                                thisSurface.Vertex(1).z);
                }
            }
        }
    } else {
        print<FormatSyntax::FMT>(slnfile, "{}\n", " Building North Axis = 0");
        print<FormatSyntax::FMT>(slnfile, "{}\n", "GlobalGeometryRules,UpperLeftCorner,CounterClockwise,WorldCoordinates;");
        for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
            auto &thisSurface = state.dataSurface->Surface(surf);
            if (thisSurface.Class == DataSurfaces::SurfaceClass::IntMass) {
                continue;
            }
            if (thisSurface.Sides == 0) {
                continue;
            }
            // process heat transfer surfaces
            print(slnfile, " Surface={}, Name={}, Azimuth={:.1R}\n", cSurfaceClass(thisSurface.Class), thisSurface.Name, thisSurface.Azimuth);
            print<FormatSyntax::FMT>(slnfile, "  {},  !- Number of (X,Y,Z) groups in this surface\n", thisSurface.Sides);
            for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                std::string optcommasemi = ",";
                if (vert == thisSurface.Sides) {
                    optcommasemi = ";";
                }
                static constexpr std::string_view fmtcoord("  {:10.2F},{:10.2F},{:10.2F}{}  !- {} {}\n");
                print<check_syntax(fmtcoord)>(slnfile,
                                              fmtcoord,
                                              thisSurface.Vertex(vert).x,
                                              thisSurface.Vertex(vert).y,
                                              thisSurface.Vertex(vert).z,
                                              optcommasemi,
                                              vertexstring,
                                              vert);
            }
        }
    }
}

static std::string normalizeName(std::string name)
{
    std::replace(begin(name), end(name), ' ', '_');
    std::replace(begin(name), end(name), ':', '_');
    return name;
}

static void WriteDXFCommon(EnergyPlusData &state, InputOutputFile &of, const std::string &ColorScheme)
{
    static constexpr std::string_view Format_800(
        "  0\nTEXT\n  8\n1\n  6\nContinuous\n 62\n{:3}\n 10\n{:15.5F}\n 20\n{:15.5F}\n 30\n{:15.5F}\n 40\n .25\n  "
        "1\nTrue North\n 41\n 0.0\n  7\nMONOTXT\n210\n0.0\n220\n0.0\n230\n1.0\n");
    static constexpr std::string_view Format_801(
        "  0\nTEXT\n  8\n1\n  6\nContinuous\n 62\n{:3}\n 10\n{:15.5F}\n 20\n{:15.5F}\n 30\n{:15.5F}\n 40\n .4\n  "
        "1\n{}\n 41\n 0.0\n  7\nMONOTXT\n210\n0.0\n220\n0.0\n230\n1.0\n");

    static constexpr std::string_view Format_703_0("  0\n3DFACE\n  8\n1\n 62\n{:3}\n");
    static constexpr std::string_view Format_703_1(" 10\n{:15.5F}\n 20\n{:15.5F}\n 30\n{:15.5F}\n");
    static constexpr std::string_view Format_703_2(" 11\n{:15.5F}\n 21\n{:15.5F}\n 31\n{:15.5F}\n");
    static constexpr std::string_view Format_703_3(" 12\n{:15.5F}\n 22\n{:15.5F}\n 32\n{:15.5F}\n");
    static constexpr std::string_view Format_703_4(" 13\n{:15.5F}\n 23\n{:15.5F}\n 33\n{:15.5F}\n");

    static constexpr std::string_view Format_708{"999\n{}{}{}\n"};
    static constexpr std::string_view Format_710{"999\n{}\n"};

    constexpr int lenArr = 4;
    std::array<Real64, lenArr> StemX = {-10.0, -10.0, -10.0, -10.0};
    std::array<Real64, lenArr> StemY = {3.0, 3.0, 0.0, 0.0};
    std::array<Real64, lenArr> StemZ = {0.1, 0.0, 0.0, 0.1};
    std::array<Real64, lenArr> Head1X = {-10.0, -10.0, -10.5, -10.5};
    std::array<Real64, lenArr> Head1Y = {3.0, 3.0, 2.133975, 2.133975};
    std::array<Real64, lenArr> Head1Z = {0.1, 0.0, 0.0, 0.1};
    std::array<Real64, lenArr> Head2X = {-10.0, -10.0, -9.5, -9.5};
    std::array<Real64, lenArr> Head2Y = {3.0, 3.0, 2.133975, 2.133975};
    std::array<Real64, lenArr> Head2Z = {0.1, 0.0, 0.0, 0.1};
    std::array<Real64, lenArr> NSide1X = {-10.5, -10.5, -10.5, -10.5};
    std::array<Real64, lenArr> NSide1Y = {4.5, 4.5, 3.5, 3.5};
    std::array<Real64, lenArr> NSide1Z = {0.1, 0.0, 0.0, 0.1};
    std::array<Real64, lenArr> NSide2X = {-10.5, -10.5, -9.5, -9.5};
    std::array<Real64, lenArr> NSide2Y = {4.5, 4.5, 3.5, 3.5};
    std::array<Real64, lenArr> NSide2Z = {0.1, 0.0, 0.0, 0.1};
    std::array<Real64, lenArr> NSide3X = {-9.5, -9.5, -9.5, -9.5};
    std::array<Real64, lenArr> NSide3Y = {4.5, 4.5, 3.5, 3.5};
    std::array<Real64, lenArr> NSide3Z = {0.1, 0.0, 0.0, 0.1};

    if (ColorScheme.empty()) {
        print(of, Format_708, "Color Scheme", ",", "Default");
    } else {
        print(of, Format_708, "Color Scheme", ",", ColorScheme);
    }

    Real64 minx = 99999.0;
    Real64 miny = 99999.0;
    for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
        auto &thisSurface = state.dataSurface->Surface(surf);
        if (thisSurface.Class == DataSurfaces::SurfaceClass::IntMass) {
            continue;
        }
        for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
            minx = min(minx, thisSurface.Vertex(vert).x);
            miny = min(miny, thisSurface.Vertex(vert).y);
        }
    }

    for (int vert = 0; vert < lenArr; ++vert) {
        StemX[vert] += minx;
        StemY[vert] += miny;
        Head1X[vert] += minx;
        Head1Y[vert] += miny;
        Head2X[vert] += minx;
        Head2Y[vert] += miny;
        NSide1X[vert] += minx;
        NSide1Y[vert] += miny;
        NSide2X[vert] += minx;
        NSide2Y[vert] += miny;
        NSide3X[vert] += minx;
        NSide3Y[vert] += miny;
    }

    auto &DXFcolorno = state.dataSurfColor->DXFcolorno;

    // This writes "True North" above the Arrow Head
    print(of, Format_710, "Text - True North");
    print<check_syntax(Format_800)>(
        of, Format_800, DXFcolorno[static_cast<int>(DataSurfaceColors::ColorNo::Text)], StemX[0] - 1.0, StemY[0], StemZ[0]);

    print(of, Format_710, "Text - Building Title");
    print<check_syntax(Format_801)>(of,
                                    Format_801,
                                    DXFcolorno[static_cast<int>(DataSurfaceColors::ColorNo::Text)],
                                    StemX[0] - 4.0,
                                    StemY[0] - 4.0,
                                    StemZ[0],
                                    "Building - " + state.dataHeatBal->BuildingName);

    // We want to point the north arrow to true north
    print(of, Format_710, "North Arrow Stem");
    print(of, Format_703_0, DXFcolorno[static_cast<int>(DataSurfaceColors::ColorNo::Text)]);
    print(of, Format_703_1, StemX[0], StemY[0], StemZ[0]);
    print(of, Format_703_2, StemX[1], StemY[1], StemZ[1]);
    print(of, Format_703_3, StemX[2], StemY[2], StemZ[2]);
    print(of, Format_703_4, StemX[3], StemY[3], StemZ[3]);

    print(of, Format_710, "North Arrow Head 1");
    print(of, Format_703_0, DXFcolorno[static_cast<int>(DataSurfaceColors::ColorNo::Text)]);
    print(of, Format_703_1, Head1X[0], Head1Y[0], Head1Z[0]);
    print(of, Format_703_2, Head1X[1], Head1Y[1], Head1Z[1]);
    print(of, Format_703_3, Head1X[2], Head1Y[2], Head1Z[2]);
    print(of, Format_703_4, Head1X[3], Head1Y[3], Head1Z[3]);

    print(of, Format_710, "North Arrow Head 2");
    print(of, Format_703_0, DXFcolorno[static_cast<int>(DataSurfaceColors::ColorNo::Text)]);
    print(of, Format_703_1, Head2X[0], Head2Y[0], Head2Z[0]);
    print(of, Format_703_2, Head2X[1], Head2Y[1], Head2Z[1]);
    print(of, Format_703_3, Head2X[2], Head2Y[2], Head2Z[2]);
    print(of, Format_703_4, Head2X[3], Head2Y[3], Head2Z[3]);

    print(of, Format_710, "North Arrow Side 1");
    print(of, Format_703_0, DXFcolorno[static_cast<int>(DataSurfaceColors::ColorNo::Text)]);
    print(of, Format_703_1, NSide1X[0], NSide1Y[0], NSide1Z[0]);
    print(of, Format_703_2, NSide1X[1], NSide1Y[1], NSide1Z[1]);
    print(of, Format_703_3, NSide1X[2], NSide1Y[2], NSide1Z[2]);
    print(of, Format_703_4, NSide1X[3], NSide1Y[3], NSide1Z[3]);

    print(of, Format_710, "North Arrow Side 2");
    print(of, Format_703_0, DXFcolorno[static_cast<int>(DataSurfaceColors::ColorNo::Text)]);
    print(of, Format_703_1, NSide2X[0], NSide2Y[0], NSide2Z[0]);
    print(of, Format_703_2, NSide2X[1], NSide2Y[1], NSide2Z[1]);
    print(of, Format_703_3, NSide2X[2], NSide2Y[2], NSide2Z[2]);
    print(of, Format_703_4, NSide2X[3], NSide2Y[3], NSide2Z[3]);

    print(of, Format_710, "North Arrow Side 3");
    print(of, Format_703_0, DXFcolorno[static_cast<int>(DataSurfaceColors::ColorNo::Text)]);
    print(of, Format_703_1, NSide3X[0], NSide3Y[0], NSide3Z[0]);
    print(of, Format_703_2, NSide3X[1], NSide3Y[1], NSide3Z[1]);
    print(of, Format_703_3, NSide3X[2], NSide3Y[2], NSide3Z[2]);
    print(of, Format_703_4, NSide3X[3], NSide3Y[3], NSide3Z[3]);

    print(of, Format_710, "Zone Names");

    for (int zones = 1; zones <= state.dataGlobal->NumOfZones; ++zones) {
        print<check_syntax(Format_710)>(of, Format_710, fmt::format("Zone={}:{}", zones, normalizeName(state.dataHeatBal->Zone(zones).Name)));
    }
}

static void DXFDaylightingReferencePoints(EnergyPlusData &state, InputOutputFile &of)
{

    static constexpr std::string_view Format_709("  0\nCIRCLE\n  8\n{}\n 62\n{:3}\n 10\n{:15.5F}\n 20\n{:15.5F}\n 30\n{:15.5F}\n 40\n{:15.5F}\n");

    // Do any daylighting reference points on layer for zone
    if ((int)state.dataDayltg->DaylRefPt.size() > 0) {
        for (int daylightCtrlNum = 1; daylightCtrlNum <= (int)state.dataDayltg->daylightControl.size(); ++daylightCtrlNum) {
            auto &thisDaylightControl = state.dataDayltg->daylightControl(daylightCtrlNum);
            DataSurfaceColors::ColorNo curcolorno = DataSurfaceColors::ColorNo::DaylSensor1;
            std::string refPtType;
            if (thisDaylightControl.DaylightMethod == Dayltg::DaylightingMethod::DElight) {
                refPtType = "DEDayRefPt";
            } else if (thisDaylightControl.DaylightMethod == Dayltg::DaylightingMethod::SplitFlux) {
                refPtType = "DayRefPt";
            }

            for (auto const &refPt : thisDaylightControl.refPts) {
                print<FormatSyntax::FMT>(of, "999\n{}:{}:{}\n", thisDaylightControl.ZoneName, refPtType, state.dataDayltg->DaylRefPt(refPt.num).Name);
                print<check_syntax(Format_709)>(of,
                                                Format_709,
                                                normalizeName(thisDaylightControl.ZoneName),
                                                state.dataSurfColor->DXFcolorno[static_cast<int>(curcolorno)],
                                                refPt.absCoords.x,
                                                refPt.absCoords.y,
                                                refPt.absCoords.z,
                                                0.2);
                curcolorno = DataSurfaceColors::ColorNo::DaylSensor2; // ref pts 2 and later are this color
            }
        }
    }
}

void DXFOut(EnergyPlusData &state,
            std::string const &PolygonAction,
            std::string const &ColorScheme // Name from user for color scheme or blank
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   March 1999
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine produces a file of DXF objects for the surfaces.

    // METHODOLOGY EMPLOYED:
    // Use the surface absolute coordinate information to produce
    // lines.

    bool ThickPolyline(false);
    bool RegularPolyline(false);
    std::string PolylineWidth(" 0.55");
    bool TriangulateFace(false);

    // Formats
    constexpr std::string_view Format_702("  0\nSECTION\n  2\nENTITIES\n");
    constexpr std::string_view Format_707("999\nDXF created from EnergyPlus\n");
    constexpr std::string_view Format_708("999\n{}{}{}\n");

    constexpr std::string_view Format_715(
        "  0\nPOLYLINE\n  8\n{}\n 62\n{:3}\n 66\n  1\n 10\n 0.0\n 20\n 0.0\n 30\n{:15.5F}\n 70\n   9\n 40\n{}\n 41\n{}\n");
    constexpr std::string_view Format_716("  0\nVERTEX\n  8\n{}\n 10\n{:15.5F}\n 20\n{:15.5F}\n 30\n{:15.5F}\n");
    constexpr std::string_view Format_717("  0\nSEQEND\n  8\n{}\n");
    constexpr std::string_view Format_704(
        "  0\n3DFACE\n  8\n{}\n 62\n{:3}\n 10\n{:15.5F}\n 20\n{:15.5F}\n 30\n{:15.5F}\n 11\n{:15.5F}\n 21\n{:15.5F}\n "
        "31\n{:15.5F}\n 12\n{:15.5F}\n 22\n{:15.5F}\n 32\n{:15.5F}\n");
    constexpr std::string_view Format_704_0("  0\n3DFACE\n  8\n{}\n 62\n{:3}\n");
    constexpr std::string_view Format_704_1(" 10\n{:15.5F}\n 20\n{:15.5F}\n 30\n{:15.5F}\n");
    constexpr std::string_view Format_704_2(" 11\n{:15.5F}\n 21\n{:15.5F}\n 31\n{:15.5F}\n");
    constexpr std::string_view Format_704_3(" 12\n{:15.5F}\n 22\n{:15.5F}\n 32\n{:15.5F}\n");
    constexpr std::string_view Format_705(" 13\n{:15.5F}\n 23\n{:15.5F}\n 33\n{:15.5F}\n");
    constexpr std::string_view Format_706("  0\nENDSEC\n  0\nEOF\n");
    constexpr std::string_view Format_709("  0\nCIRCLE\n  8\n{}\n 62\n{:3}\n 10\n{:15.5F}\n 20\n{:15.5F}\n 30\n{:15.5F}\n 40\n{:15.5F}\n");
    constexpr std::string_view Format_710("999\n{}\n");

    if (PolygonAction == "TRIANGULATE3DFACE" || PolygonAction == "TRIANGULATE" || PolygonAction.empty()) {
        TriangulateFace = true;
        RegularPolyline = false;
        ThickPolyline = false;
    } else if (PolygonAction == "THICKPOLYLINE") {
        ThickPolyline = true;
        RegularPolyline = false;
        TriangulateFace = false;
    } else if (PolygonAction == "REGULARPOLYLINE") {
        RegularPolyline = true;
        TriangulateFace = false;
        ThickPolyline = false;
        PolylineWidth = " 0";
    } else {
        ShowWarningError(state, format("DXFOut: Illegal key specified for Surfaces with > 4 sides={}", PolygonAction));
        ShowContinueError(state, R"(...Valid keys are: "ThickPolyline", "RegularPolyline", "Triangulate3DFace".)");
        ShowContinueError(state, "\"Triangulate3DFace\" will be used for any surfaces with > 4 sides.");
        TriangulateFace = true;
        RegularPolyline = false;
        ThickPolyline = false;
    }

    if (state.dataSurface->TotSurfaces > 0 && !allocated(state.dataSurface->Surface)) {
        // no error needed, probably in end processing, just return
        return;
    }

    auto dxffile = state.files.dxf.open(state, "DXFOut", state.files.outputControl.dxf);

    print(dxffile, Format_702); // Start of Entities section

    print(dxffile, Format_707); // Comment

    print(dxffile, Format_708, "Program Version", ",", state.dataStrGlobals->VerStringVar);

    if (PolygonAction.empty()) {
        print(dxffile, Format_708, "Polygon Action", ",", "Triangulate3DFace");
    } else {
        print(dxffile, Format_708, "Polygon Action", ",", PolygonAction);
    }

    WriteDXFCommon(state, dxffile, ColorScheme);
    auto &DXFcolorno = state.dataSurfColor->DXFcolorno;
    DataSurfaceColors::ColorNo colorindex = DataSurfaceColors::ColorNo::ShdDetFix;
    //  Do all detached shading surfaces first
    for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
        std::string ShadeType;
        auto &thisSurface = state.dataSurface->Surface(surf);

        if (thisSurface.HeatTransSurf) {
            continue;
        }
        if (thisSurface.Class == DataSurfaces::SurfaceClass::Shading) {
            continue;
        }
        if (thisSurface.Sides == 0) {
            continue;
        }
        if (thisSurface.Class == DataSurfaces::SurfaceClass::Detached_F) {
            colorindex = DataSurfaceColors::ColorNo::ShdDetFix;
        }
        if (thisSurface.Class == DataSurfaces::SurfaceClass::Detached_B) {
            colorindex = DataSurfaceColors::ColorNo::ShdDetBldg;
        }
        if (state.dataSurface->SurfIsPV(surf)) {
            colorindex = DataSurfaceColors::ColorNo::PV;
        }
        if (thisSurface.Class == DataSurfaces::SurfaceClass::Detached_F) {
            ShadeType = "Fixed Shading";
            print(dxffile, Format_710, "Fixed Shading:" + thisSurface.Name);
        } else if (thisSurface.Class == DataSurfaces::SurfaceClass::Detached_B) {
            ShadeType = "Building Shading";
            print(dxffile, Format_710, "Building Shading:" + thisSurface.Name);
        }
        if (thisSurface.Sides <= 4) {
            print(dxffile, Format_704_0, ShadeType, DXFcolorno[static_cast<int>(colorindex)]);
            print(dxffile, Format_704_1, thisSurface.Vertex(1).x, thisSurface.Vertex(1).y, thisSurface.Vertex(1).z);
            print(dxffile, Format_704_2, thisSurface.Vertex(2).x, thisSurface.Vertex(2).y, thisSurface.Vertex(2).z);
            print(dxffile, Format_704_3, thisSurface.Vertex(3).x, thisSurface.Vertex(3).y, thisSurface.Vertex(3).z);
            if (thisSurface.Sides == 3) {
                print(dxffile, Format_705, thisSurface.Vertex(3).x, thisSurface.Vertex(3).y, thisSurface.Vertex(3).z);
            } else {
                print(dxffile, Format_705, thisSurface.Vertex(4).x, thisSurface.Vertex(4).y, thisSurface.Vertex(4).z);
            }
        } else { // polygon
            if (!TriangulateFace) {
                Real64 minz = 99999.0;
                for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                    minz = min(minz, thisSurface.Vertex(vert).z);
                }
                print(dxffile, Format_715, ShadeType, DXFcolorno[static_cast<int>(colorindex)], minz, PolylineWidth, PolylineWidth);
                for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                    print(dxffile, Format_716, ShadeType, thisSurface.Vertex(vert).x, thisSurface.Vertex(vert).y, thisSurface.Vertex(vert).z);
                }
                print(dxffile, Format_717, ShadeType);
            } else {
                Array1D<DataVectorTypes::dTriangle> mytriangles;

                const int ntri = DXFEarClipping::Triangulate(state,
                                                             thisSurface.Sides,
                                                             thisSurface.Vertex,
                                                             mytriangles,
                                                             thisSurface.Azimuth,
                                                             thisSurface.Tilt,
                                                             thisSurface.Name,
                                                             thisSurface.Class);
                for (int svert = 1; svert <= ntri; ++svert) {
                    const int vv0 = mytriangles(svert).vv0;
                    const int vv1 = mytriangles(svert).vv1;
                    const int vv2 = mytriangles(svert).vv2;
                    print(dxffile,
                          Format_704,
                          ShadeType,
                          DXFcolorno[static_cast<int>(colorindex)],
                          thisSurface.Vertex(vv0).x,
                          thisSurface.Vertex(vv0).y,
                          thisSurface.Vertex(vv0).z,
                          thisSurface.Vertex(vv1).x,
                          thisSurface.Vertex(vv1).y,
                          thisSurface.Vertex(vv1).z,
                          thisSurface.Vertex(vv2).x,
                          thisSurface.Vertex(vv2).y,
                          thisSurface.Vertex(vv2).z);
                    print(dxffile, Format_705, thisSurface.Vertex(vv2).x, thisSurface.Vertex(vv2).y, thisSurface.Vertex(vv2).z);
                }
                mytriangles.deallocate();
            }
        }
    }

    // now do zone surfaces, by zone
    for (int zones = 1; zones <= state.dataGlobal->NumOfZones; ++zones) {
        const std::string TempZoneName = normalizeName(state.dataHeatBal->Zone(zones).Name);

        for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
            auto &thisSurface = state.dataSurface->Surface(surf);
            if (thisSurface.Zone != zones) {
                continue;
            }
            if (thisSurface.Sides == 0) {
                continue;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::IntMass) {
                continue;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Wall) {
                colorindex = DataSurfaceColors::ColorNo::Wall;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Roof) {
                colorindex = DataSurfaceColors::ColorNo::Roof;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Floor) {
                colorindex = DataSurfaceColors::ColorNo::Floor;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Door) {
                colorindex = DataSurfaceColors::ColorNo::Door;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Window) {
                if (thisSurface.OriginalClass == DataSurfaces::SurfaceClass::Window) {
                    colorindex = DataSurfaceColors::ColorNo::Window;
                }
                if (thisSurface.OriginalClass == DataSurfaces::SurfaceClass::GlassDoor) {
                    colorindex = DataSurfaceColors::ColorNo::GlassDoor;
                }
                if (thisSurface.OriginalClass == DataSurfaces::SurfaceClass::TDD_Dome) {
                    colorindex = DataSurfaceColors::ColorNo::TDDDome;
                }
                if (thisSurface.OriginalClass == DataSurfaces::SurfaceClass::TDD_Diffuser) {
                    colorindex = DataSurfaceColors::ColorNo::TDDDiffuser;
                }
            }
            if (state.dataSurface->SurfIsPV(surf)) {
                colorindex = DataSurfaceColors::ColorNo::PV;
            }

            print(dxffile, Format_710, thisSurface.ZoneName + ':' + thisSurface.Name);
            if (thisSurface.Sides <= 4) {
                print(dxffile, Format_704_0, TempZoneName, DXFcolorno[static_cast<int>(colorindex)]);
                print(dxffile, Format_704_1, thisSurface.Vertex(1).x, thisSurface.Vertex(1).y, thisSurface.Vertex(1).z);
                print(dxffile, Format_704_2, thisSurface.Vertex(2).x, thisSurface.Vertex(2).y, thisSurface.Vertex(2).z);
                print(dxffile, Format_704_3, thisSurface.Vertex(3).x, thisSurface.Vertex(3).y, thisSurface.Vertex(3).z);
                if (thisSurface.Sides == 3) {
                    print(dxffile, Format_705, thisSurface.Vertex(3).x, thisSurface.Vertex(3).y, thisSurface.Vertex(3).z);
                } else {
                    print(dxffile, Format_705, thisSurface.Vertex(4).x, thisSurface.Vertex(4).y, thisSurface.Vertex(4).z);
                }
            } else { // polygon surface
                if (!TriangulateFace) {
                    Real64 minz = 99999.0;
                    for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                        minz = min(minz, thisSurface.Vertex(vert).z);
                    }
                    print(dxffile, Format_715, TempZoneName, DXFcolorno[static_cast<int>(colorindex)], minz, PolylineWidth, PolylineWidth);
                    for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                        print(dxffile, Format_716, TempZoneName, thisSurface.Vertex(vert).x, thisSurface.Vertex(vert).y, thisSurface.Vertex(vert).z);
                    }
                    print(dxffile, Format_717, TempZoneName);
                } else {
                    Array1D<DataVectorTypes::dTriangle> mytriangles;

                    const int ntri = DXFEarClipping::Triangulate(state,
                                                                 thisSurface.Sides,
                                                                 thisSurface.Vertex,
                                                                 mytriangles,
                                                                 thisSurface.Azimuth,
                                                                 thisSurface.Tilt,
                                                                 thisSurface.Name,
                                                                 thisSurface.Class);
                    for (int svert = 1; svert <= ntri; ++svert) {
                        const int vv0 = mytriangles(svert).vv0;
                        const int vv1 = mytriangles(svert).vv1;
                        const int vv2 = mytriangles(svert).vv2;
                        print(dxffile,
                              Format_704,
                              TempZoneName,
                              DXFcolorno[static_cast<int>(colorindex)],
                              thisSurface.Vertex(vv0).x,
                              thisSurface.Vertex(vv0).y,
                              thisSurface.Vertex(vv0).z,
                              thisSurface.Vertex(vv1).x,
                              thisSurface.Vertex(vv1).y,
                              thisSurface.Vertex(vv1).z,
                              thisSurface.Vertex(vv2).x,
                              thisSurface.Vertex(vv2).y,
                              thisSurface.Vertex(vv2).z);
                        print(dxffile, Format_705, thisSurface.Vertex(vv2).x, thisSurface.Vertex(vv2).y, thisSurface.Vertex(vv2).z);
                    }
                    mytriangles.deallocate();
                }
            }
        }
        // still have to do shading surfaces for zone
        for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
            auto &thisSurface = state.dataSurface->Surface(surf);
            // if (surface(surf)%heattranssurf) CYCLE ! Shading with a construction is allowed to be HT surf for daylighting shelves
            if (thisSurface.Class != DataSurfaces::SurfaceClass::Shading) {
                continue;
            }
            if (thisSurface.ZoneName != state.dataHeatBal->Zone(zones).Name) {
                continue;
            }
            if (thisSurface.Sides == 0) {
                continue;
            }
            colorindex = DataSurfaceColors::ColorNo::ShdAtt;
            if (state.dataSurface->SurfIsPV(surf)) {
                colorindex = DataSurfaceColors::ColorNo::PV;
            }
            print(dxffile, Format_710, thisSurface.ZoneName + ':' + thisSurface.Name);
            if (thisSurface.Sides <= 4) {
                print(dxffile, Format_704_0, TempZoneName, DXFcolorno[static_cast<int>(colorindex)]);
                print(dxffile, Format_704_1, thisSurface.Vertex(1).x, thisSurface.Vertex(1).y, thisSurface.Vertex(1).z);
                print(dxffile, Format_704_2, thisSurface.Vertex(2).x, thisSurface.Vertex(2).y, thisSurface.Vertex(2).z);
                print(dxffile, Format_704_3, thisSurface.Vertex(3).x, thisSurface.Vertex(3).y, thisSurface.Vertex(3).z);
                if (thisSurface.Sides == 3) {
                    print(dxffile, Format_705, thisSurface.Vertex(3).x, thisSurface.Vertex(3).y, thisSurface.Vertex(3).z);
                } else {
                    print(dxffile, Format_705, thisSurface.Vertex(4).x, thisSurface.Vertex(4).y, thisSurface.Vertex(4).z);
                }
            } else { // polygon attached shading
                if (!TriangulateFace) {
                    Real64 minz = 99999.0;
                    for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                        minz = min(minz, thisSurface.Vertex(vert).z);
                    }
                    print(dxffile, Format_715, TempZoneName, DXFcolorno[static_cast<int>(colorindex)], minz, PolylineWidth, PolylineWidth);
                    for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                        print(dxffile, Format_716, TempZoneName, thisSurface.Vertex(vert).x, thisSurface.Vertex(vert).y, thisSurface.Vertex(vert).z);
                    }
                    print(dxffile, Format_717, TempZoneName);
                } else {
                    Array1D<DataVectorTypes::dTriangle> mytriangles;
                    int ntri = 0;
                    if (thisSurface.Shape == DataSurfaces::SurfaceShape::RectangularOverhang) {
                        ntri = DXFEarClipping::Triangulate(state,
                                                           thisSurface.Sides,
                                                           thisSurface.Vertex,
                                                           mytriangles,
                                                           thisSurface.Azimuth,
                                                           thisSurface.Tilt,
                                                           thisSurface.Name,
                                                           DataSurfaces::SurfaceClass::Overhang);
                    } else {
                        ntri = DXFEarClipping::Triangulate(state,
                                                           thisSurface.Sides,
                                                           thisSurface.Vertex,
                                                           mytriangles,
                                                           thisSurface.Azimuth,
                                                           thisSurface.Tilt,
                                                           thisSurface.Name,
                                                           DataSurfaces::SurfaceClass::Fin);
                    }
                    for (int svert = 1; svert <= ntri; ++svert) {
                        const int vv0 = mytriangles(svert).vv0;
                        const int vv1 = mytriangles(svert).vv1;
                        const int vv2 = mytriangles(svert).vv2;
                        print(dxffile,
                              Format_704,
                              TempZoneName,
                              DXFcolorno[static_cast<int>(colorindex)],
                              thisSurface.Vertex(vv0).x,
                              thisSurface.Vertex(vv0).y,
                              thisSurface.Vertex(vv0).z,
                              thisSurface.Vertex(vv1).x,
                              thisSurface.Vertex(vv1).y,
                              thisSurface.Vertex(vv1).z,
                              thisSurface.Vertex(vv2).x,
                              thisSurface.Vertex(vv2).y,
                              thisSurface.Vertex(vv2).z);
                        print(dxffile, Format_705, thisSurface.Vertex(vv2).x, thisSurface.Vertex(vv2).y, thisSurface.Vertex(vv2).z);
                    }
                    mytriangles.deallocate();
                }
            }
        }
    }

    DXFDaylightingReferencePoints(state, dxffile);

    for (int zones = 1; zones <= state.dataGlobal->NumOfZones; ++zones) {
        const DataSurfaceColors::ColorNo curcolorno = DataSurfaceColors::ColorNo::DaylSensor1;

        for (auto const &illumMap : state.dataDayltg->illumMaps) {
            if (illumMap.zoneIndex != zones) {
                continue;
            }
            int numRefPt = 0;
            for (auto const &refPt : illumMap.refPts) {
                print(dxffile, Format_710, format("{}:MapRefPt:{}", state.dataHeatBal->Zone(zones).Name, ++numRefPt));
                print(dxffile,
                      Format_709,
                      normalizeName(state.dataHeatBal->Zone(zones).Name),
                      DXFcolorno[static_cast<int>(curcolorno)],
                      refPt.absCoords.x,
                      refPt.absCoords.y,
                      refPt.absCoords.z,
                      0.05);
            }
        }
    }

    print(dxffile, Format_706);
}

void DXFOutWireFrame(EnergyPlusData &state, std::string const &ColorScheme)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   August 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine produces a file of DXF objects for the surfaces (all lines -- wireframe).

    // METHODOLOGY EMPLOYED:
    // Use the surface absolute coordinate information to produce
    // lines.

    constexpr std::string_view PolylineWidth = " 0.55";

    constexpr std::string_view Format_702("  0\nSECTION\n  2\nENTITIES\n");
    constexpr std::string_view Format_707("999\nDXF created from EnergyPlus\n");
    constexpr std::string_view Format_708("999\n{}{}{}\n");

    constexpr std::string_view Format_715(
        "  0\nPOLYLINE\n  8\n{}\n 62\n{:3}\n 66\n  1\n 10\n 0.0\n 20\n 0.0\n 30\n{:15.5F}\n 70\n   9\n 40\n{}\n 41\n{}\n");
    constexpr std::string_view Format_716("  0\nVERTEX\n  8\n{}\n 10\n{:15.5F}\n 20\n{:15.5F}\n 30\n{:15.5F}\n");
    constexpr std::string_view Format_717("  0\nSEQEND\n  8\n{}\n");
    constexpr std::string_view Format_706("  0\nENDSEC\n  0\nEOF\n");
    constexpr std::string_view Format_710("999\n{}\n");

    if (state.dataSurface->TotSurfaces > 0 && !allocated(state.dataSurface->Surface)) {
        // no error needed, probably in end processing, just return
        return;
    }

    auto dxffile = state.files.dxf.open(state, "DXFOutWireFrame", state.files.outputControl.dxf);

    print(dxffile, Format_702); // Start of Entities section

    print(dxffile, Format_707); // Comment

    print(dxffile, Format_708, "Program Version", ",", state.dataStrGlobals->VerStringVar);
    print(dxffile, Format_708, "DXF using Wireframe", ' ', ' ');

    WriteDXFCommon(state, dxffile, ColorScheme);

    //  Do all detached shading surfaces first
    int surfcount = 0;
    DataSurfaceColors::ColorNo colorindex = DataSurfaceColors::ColorNo::Invalid;
    for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
        std::string ShadeType;
        auto &thisSurface = state.dataSurface->Surface(surf);
        if (thisSurface.HeatTransSurf) {
            continue;
        }
        if (thisSurface.Class == DataSurfaces::SurfaceClass::Shading) {
            continue;
        }
        if (thisSurface.Class == DataSurfaces::SurfaceClass::Detached_F) {
            colorindex = DataSurfaceColors::ColorNo::ShdDetFix;
        }
        if (thisSurface.Class == DataSurfaces::SurfaceClass::Detached_B) {
            colorindex = DataSurfaceColors::ColorNo::ShdDetBldg;
        }
        if (state.dataSurface->SurfIsPV(surf)) {
            colorindex = DataSurfaceColors::ColorNo::PV;
        }
        if (thisSurface.Class == DataSurfaces::SurfaceClass::Detached_F) {
            ShadeType = "Fixed Shading";
            print(dxffile, Format_710, "Fixed Shading:" + thisSurface.Name);
        } else if (thisSurface.Class == DataSurfaces::SurfaceClass::Detached_B) {
            ShadeType = "Building Shading";
            print(dxffile, Format_710, "Building Shading:" + thisSurface.Name);
        }
        ++surfcount;
        ShadeType += format("_{}", surfcount);
        Real64 minz = 99999.0;
        for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
            minz = min(minz, thisSurface.Vertex(vert).z);
        }

        print(dxffile, Format_715, ShadeType, state.dataSurfColor->DXFcolorno[static_cast<int>(colorindex)], minz, PolylineWidth, PolylineWidth);
        for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
            print(dxffile, Format_716, ShadeType, thisSurface.Vertex(vert).x, thisSurface.Vertex(vert).y, thisSurface.Vertex(vert).z);
        }
        print(dxffile, Format_717, ShadeType);
    }

    // now do zone surfaces, by zone
    for (int zones = 1; zones <= state.dataGlobal->NumOfZones; ++zones) {
        std::string const SaveZoneName = normalizeName(state.dataHeatBal->Zone(zones).Name);

        surfcount = 0;
        for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
            auto &thisSurface = state.dataSurface->Surface(surf);
            if (thisSurface.Zone != zones) {
                continue;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::IntMass) {
                continue;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Wall) {
                colorindex = DataSurfaceColors::ColorNo::Wall;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Roof) {
                colorindex = DataSurfaceColors::ColorNo::Roof;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Floor) {
                colorindex = DataSurfaceColors::ColorNo::Floor;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Door) {
                colorindex = DataSurfaceColors::ColorNo::Door;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Window) {
                if (thisSurface.OriginalClass == DataSurfaces::SurfaceClass::Window) {
                    colorindex = DataSurfaceColors::ColorNo::Window;
                }
                if (thisSurface.OriginalClass == DataSurfaces::SurfaceClass::GlassDoor) {
                    colorindex = DataSurfaceColors::ColorNo::GlassDoor;
                }
                if (thisSurface.OriginalClass == DataSurfaces::SurfaceClass::TDD_Dome) {
                    colorindex = DataSurfaceColors::ColorNo::TDDDome;
                }
                if (thisSurface.OriginalClass == DataSurfaces::SurfaceClass::TDD_Diffuser) {
                    colorindex = DataSurfaceColors::ColorNo::TDDDiffuser;
                }
            }
            if (state.dataSurface->SurfIsPV(surf)) {
                colorindex = DataSurfaceColors::ColorNo::PV;
            }
            ++surfcount;

            print(dxffile, Format_710, thisSurface.ZoneName + ':' + thisSurface.Name);
            std::string const TempZoneName = format("{}_{}", SaveZoneName, surfcount);
            Real64 minz = 99999.0;
            for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                minz = min(minz, thisSurface.Vertex(vert).z);
            }

            print(
                dxffile, Format_715, TempZoneName, state.dataSurfColor->DXFcolorno[static_cast<int>(colorindex)], minz, PolylineWidth, PolylineWidth);
            for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                print(dxffile, Format_716, TempZoneName, thisSurface.Vertex(vert).x, thisSurface.Vertex(vert).y, thisSurface.Vertex(vert).z);
            }
            print(dxffile, Format_717, TempZoneName);
        }
        // still have to do shading surfaces for zone
        surfcount = 0;
        for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
            auto &thisSurface = state.dataSurface->Surface(surf);
            if (thisSurface.Class != DataSurfaces::SurfaceClass::Shading) {
                continue;
            }
            if (thisSurface.ZoneName != state.dataHeatBal->Zone(zones).Name) {
                continue;
            }
            colorindex = DataSurfaceColors::ColorNo::ShdAtt;
            if (state.dataSurface->SurfIsPV(surf)) {
                colorindex = DataSurfaceColors::ColorNo::PV;
            }
            ++surfcount;

            print(dxffile, Format_710, thisSurface.ZoneName + ':' + thisSurface.Name);
            std::string const TempZoneName = format("{}_{}", SaveZoneName, surfcount);
            Real64 minz = 99999.0;
            for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                minz = min(minz, thisSurface.Vertex(vert).z);
            }

            print(
                dxffile, Format_715, TempZoneName, state.dataSurfColor->DXFcolorno[static_cast<int>(colorindex)], minz, PolylineWidth, PolylineWidth);
            for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                print(dxffile, Format_716, TempZoneName, thisSurface.Vertex(vert).x, thisSurface.Vertex(vert).y, thisSurface.Vertex(vert).z);
            }
            print(dxffile, Format_717, TempZoneName);
        }
    }

    DXFDaylightingReferencePoints(state, dxffile);

    print(dxffile, Format_706);
}

void DetailsForSurfaces(EnergyPlusData &state, int const RptType) // (1=Vertices only, 10=Details only, 11=Details with vertices)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   February 2002
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine provides an optional detailed surface report
    // for each surface in the input file.

    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr static std::array<std::string_view, 9> ConvCoeffCalcs = {
        "ASHRAESimple", "ASHRAETARP", "CeilingDiffuser", "TrombeWall", "TARP", "MoWitt", "DOE-2", "BLAST", "AdaptiveConvectionAlgorithm"};

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    std::string BaseSurfName;
    std::string ConstructionName;
    std::string IntConvCoeffCalc;
    std::string ExtConvCoeffCalc;
    Real64 NominalUwithConvCoeffs;
    std::string cNominalU;
    std::string cNominalUwithConvCoeffs;
    std::string SolarDiffusing;
    std::string AlgoName;

    if (state.dataSurface->TotSurfaces > 0 && !allocated(state.dataSurface->Surface)) {
        // no error needed, probably in end processing, just return
        return;
    }

    std::stringstream ss;
    auto *eiostream = &ss;
    //!!!    Write Header lines for report
    if (RptType == 10) {                                                                                          // Details only
        *eiostream << "! <Zone Surfaces>,Zone Name,# Surfaces\n";                                                 // Format_700
        *eiostream << "! <Shading Surfaces>,Number of Shading Surfaces,# Surfaces\n";                             // Format_700b
        *eiostream << "! <HeatTransfer Surface>,Surface Name,Surface Class,Base Surface,Heat Transfer Algorithm"; // Format_701
        *eiostream << ",Construction,Nominal U (w/o film coefs) {W/m2-K},Nominal U (with film coefs) {W/m2-K},Solar Diffusing,Area (Net) {m2},Area "
                      "(Gross) {m2},Area (Sunlit Calc) {m2},Azimuth {deg},Tilt {deg},~Width {m},~Height {m},Reveal "
                      "{m},ExtBoundCondition,ExtConvCoeffCalc,IntConvCoeffCalc,SunExposure,WindExposure,ViewFactorToGround,ViewFactorToSky,"
                      "ViewFactorToGround-IR,ViewFactorToSky-IR,#Sides\n";                                   // Format_7011
        *eiostream << "! <Shading Surface>,Surface Name,Surface Class,Base Surface,Heat Transfer Algorithm"; // Format_701b
        *eiostream << ",Transmittance Schedule,Min Schedule Value,Max Schedule Value,Solar Diffusing,Area (Net) {m2},Area (Gross) {m2},Area (Sunlit "
                      "Calc) {m2},Azimuth {deg},Tilt {deg},~Width {m},~Height {m},Reveal "
                      "{m},ExtBoundCondition,ExtConvCoeffCalc,IntConvCoeffCalc,SunExposure,WindExposure,ViewFactorToGround,ViewFactorToSky,"
                      "ViewFactorToGround-IR,ViewFactorToSky-IR,#Sides\n";                                         // Format_7011b
        *eiostream << "! <Frame/Divider Surface>,Surface Name,Surface Class,Base Surface,Heat Transfer Algorithm"; // Format_701c
        *eiostream << ",Construction,Nominal U (w/o film coefs) {W/m2-K},Nominal U (with film coefs) {W/m2-K},Solar Diffusing,Area (Net) {m2},Area "
                      "(Gross) {m2},Area (Sunlit Calc) {m2},Azimuth {deg},Tilt {deg},~Width {m},~Height {m},Reveal {m}\n"; // Format_7011c
    } else if (RptType == 11) {                                                                                            // Details with Vertices
        *eiostream << "! <Zone Surfaces>,Zone Name,# Surfaces";                                                            // Format_700
        *eiostream << ", Vertices are shown starting at Upper-Left-Corner => Counter-Clockwise => World Coordinates\n";    // Format_710
        *eiostream << "! <Shading Surfaces>,Number of Shading Surfaces,# Surfaces";                                        // Format_700b
        *eiostream << ", Vertices are shown starting at Upper-Left-Corner => Counter-Clockwise => World Coordinates\n";    // Format_710
        *eiostream << "! <HeatTransfer Surface>,Surface Name,Surface Class,Base Surface,Heat Transfer Algorithm";          // Format_701
        *eiostream << ",Construction,Nominal U (w/o film coefs) {W/m2-K},Nominal U (with film coefs) {W/m2-K},Solar Diffusing,Area (Net) {m2},Area "
                      "(Gross) {m2},Area (Sunlit Calc) {m2},Azimuth {deg},Tilt {deg},~Width {m},~Height {m},Reveal "
                      "{m},ExtBoundCondition,ExtConvCoeffCalc,IntConvCoeffCalc,SunExposure,WindExposure,ViewFactorToGround,ViewFactorToSky,"
                      "ViewFactorToGround-IR,ViewFactorToSky-IR,#Sides"; // Format_7011
        *eiostream << ",Vertex 1 X {m},Vertex 1 Y {m},Vertex 1 Z {m},Vertex 2 X {m},Vertex 2 Y {m},Vertex 2 Z {m},Vertex 3 X {m},Vertex 3 Y "
                      "{m},Vertex 3 Z {m},Vertex 4 X {m},Vertex 4 Z {m},Vertex 4 Z {m},{etc}\n";             // Format_707
        *eiostream << "! <Shading Surface>,Surface Name,Surface Class,Base Surface,Heat Transfer Algorithm"; // Format_701b
        *eiostream << ",Transmittance Schedule,Min Schedule Value,Max Schedule Value,Solar Diffusing,Area (Net) {m2},Area (Gross) {m2},Area (Sunlit "
                      "Calc) {m2},Azimuth {deg},Tilt {deg},~Width {m},~Height {m},Reveal "
                      "{m},ExtBoundCondition,ExtConvCoeffCalc,IntConvCoeffCalc,SunExposure,WindExposure,ViewFactorToGround,ViewFactorToSky,"
                      "ViewFactorToGround-IR,ViewFactorToSky-IR,#Sides"; // Format_7011b
        *eiostream << ",Vertex 1 X {m},Vertex 1 Y {m},Vertex 1 Z {m},Vertex 2 X {m},Vertex 2 Y {m},Vertex 2 Z {m},Vertex 3 X {m},Vertex 3 Y "
                      "{m},Vertex 3 Z {m},Vertex 4 X {m},Vertex 4 Z {m},Vertex 4 Z {m},{etc}\n";                   // Format_707
        *eiostream << "! <Frame/Divider Surface>,Surface Name,Surface Class,Base Surface,Heat Transfer Algorithm"; // Format_701c
        // Vertices are not applicable for window frame and divider, so skip 707
        *eiostream << ",Construction,Nominal U (w/o film coefs) {W/m2-K},Nominal U (with film coefs) {W/m2-K},Solar Diffusing,Area (Net) {m2},Area "
                      "(Gross) {m2},Area (Sunlit Calc) {m2},Azimuth {deg},Tilt {deg},~Width {m},~Height {m},Reveal {m}\n"; // Format_7011c
    } else {                                                                                                               // Vertices only
        *eiostream << "! <Zone Surfaces>,Zone Name,# Surfaces";                                                            // Format_700
        *eiostream << ", Vertices are shown starting at Upper-Left-Corner => Counter-Clockwise => World Coordinates\n";    // Format_710
        *eiostream << "! <Shading Surfaces>,Number of Shading Surfaces,# Surfaces";                                        // Format_700b
        *eiostream << ", Vertices are shown starting at Upper-Left-Corner => Counter-Clockwise => World Coordinates\n";    // Format_710
        *eiostream << "! <HeatTransfer Surface>,Surface Name,Surface Class,Base Surface,Heat Transfer Algorithm";          // Format_701
        *eiostream << ",#Sides";                                                                                           // Format_7012
        *eiostream << ",Vertex 1 X {m},Vertex 1 Y {m},Vertex 1 Z {m},Vertex 2 X {m},Vertex 2 Y {m},Vertex 2 Z {m},Vertex 3 X {m},Vertex 3 Y "
                      "{m},Vertex 3 Z {m},Vertex 4 X {m},Vertex 4 Z {m},Vertex 4 Z {m},{etc}\n";             // Format_707
        *eiostream << "! <Shading Surface>,Surface Name,Surface Class,Base Surface,Heat Transfer Algorithm"; // Format_701b
        *eiostream << ",#Sides";                                                                             // Format_7012
        *eiostream << ",Vertex 1 X {m},Vertex 1 Y {m},Vertex 1 Z {m},Vertex 2 X {m},Vertex 2 Y {m},Vertex 2 Z {m},Vertex 3 X {m},Vertex 3 Y "
                      "{m},Vertex 3 Z {m},Vertex 4 X {m},Vertex 4 Z {m},Vertex 4 Z {m},{etc}\n"; // Format_707
        // Vertices are not applicable for window frame and divider, so skip 701c here
    }

    // Do just "detached" shading first
    int surf2 = 0;
    for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
        surf2 = surf;
        auto const &thisSurface = state.dataSurface->Surface(surf);
        if (thisSurface.Zone != 0) {
            break;
        }
    }
    if ((surf2 - 1) > 0) {
        std::string ScheduleName;
        std::string cSchedMin;
        std::string cSchedMax;
        *eiostream << "Shading Surfaces," << "Number of Shading Surfaces," << surf2 - 1 << '\n';
        for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
            auto &thisSurface = state.dataSurface->Surface(surf);
            if (thisSurface.Zone != 0) {
                break;
            }
            AlgoName = "None";
            *eiostream << "Shading Surface," << thisSurface.Name << "," << cSurfaceClass(thisSurface.Class) << "," << thisSurface.BaseSurfName << ","
                       << AlgoName << ",";
            if (RptType == 10) {
                if (thisSurface.shadowSurfSched != nullptr) {
                    ScheduleName = thisSurface.shadowSurfSched->Name;
                    cSchedMin = format("{:.2R}", thisSurface.shadowSurfSched->getMinVal(state));
                    cSchedMax = format("{:.2R}", thisSurface.shadowSurfSched->getMinVal(state));
                } else {
                    ScheduleName = "";
                    cSchedMin = "0.0";
                    cSchedMax = "0.0";
                }
                *eiostream << ScheduleName << "," << cSchedMin << "," << cSchedMax << "," << ' ' << "," << format("{:.2R}", thisSurface.Area) << ","
                           << format("{:.2R}", thisSurface.GrossArea) << "," << format("{:.2R}", thisSurface.NetAreaShadowCalc) << ","
                           << format("{:.2R}", thisSurface.Azimuth) << "," << format("{:.2R}", thisSurface.Tilt) << ","
                           << format("{:.2R}", thisSurface.Width) << "," << format("{:.2R}", thisSurface.Height) << ",";
                *eiostream << ",,,,,,,,,," << fmt::to_string(thisSurface.Sides) << '\n';
            } else if (RptType == 1) {
                *eiostream << fmt::to_string(thisSurface.Sides) << ",";
            } else {
                if (thisSurface.shadowSurfSched != nullptr) {
                    ScheduleName = thisSurface.shadowSurfSched->Name;
                    cSchedMin = format("{:.2R}", thisSurface.shadowSurfSched->getMinVal(state));
                    cSchedMax = format("{:.2R}", thisSurface.shadowSurfSched->getMinVal(state));
                } else {
                    ScheduleName = "";
                    cSchedMin = "0.0";
                    cSchedMax = "0.0";
                }
                *eiostream << ScheduleName << "," << cSchedMin << "," << cSchedMax << "," << ' ' << "," << format("{:.2R}", thisSurface.Area) << ","
                           << format("{:.2R}", thisSurface.GrossArea) << "," << format("{:.2R}", thisSurface.NetAreaShadowCalc) << ","
                           << format("{:.2R}", thisSurface.Azimuth) << "," << format("{:.2R}", thisSurface.Tilt) << ","
                           << format("{:.2R}", thisSurface.Width) << "," << format("{:.2R}", thisSurface.Height) << ",";
                *eiostream << ",,,,,,,,,," << fmt::to_string(thisSurface.Sides) << ",";
            }
            if (RptType == 10) {
                continue;
            }
            for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                if (vert != thisSurface.Sides) {
                    *eiostream << format("{:.2R}", thisSurface.Vertex(vert).x) << "," << format("{:.2R}", thisSurface.Vertex(vert).y) << ","
                               << format("{:.2R}", thisSurface.Vertex(vert).z) << ",";
                } else {
                    *eiostream << format("{:.2R}", thisSurface.Vertex(vert).x) << "," << format("{:.2R}", thisSurface.Vertex(vert).y) << ","
                               << format("{:.2R}", thisSurface.Vertex(vert).z) << '\n';
                }
            }
            //  This shouldn't happen with shading surface -- always have vertices
            if (thisSurface.Sides == 0) {
                *eiostream << '\n';
            }
        }
    }

    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        *eiostream << "Zone Surfaces," << state.dataHeatBal->Zone(ZoneNum).Name << ","
                   << (state.dataHeatBal->Zone(ZoneNum).AllSurfaceLast - state.dataHeatBal->Zone(ZoneNum).AllSurfaceFirst + 1) << '\n';
        for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
            auto &thisSurface = state.dataSurface->Surface(surf);
            if (thisSurface.Zone != ZoneNum) {
                continue;
            }
            SolarDiffusing = "";
            if (RptType == 10 || RptType == 11) { // Details and Details with Vertices
                if (thisSurface.BaseSurf == surf) {
                    BaseSurfName = "";
                } else {
                    BaseSurfName = thisSurface.BaseSurfName;
                }

                AlgoName = DataSurfaces::HeatTransAlgoStrs[(int)thisSurface.HeatTransferAlgorithm];

                // Default Convection Coefficient Calculation Algorithms
                // This doulbe lookup is a screwed up way to do this, but ...
                IntConvCoeffCalc = ConvCoeffCalcs[Convect::HcIntReportVals[static_cast<int>(state.dataHeatBal->Zone(ZoneNum).IntConvAlgo)] - 1];
                ExtConvCoeffCalc = ConvCoeffCalcs[Convect::HcExtReportVals[static_cast<int>(state.dataHeatBal->Zone(ZoneNum).ExtConvAlgo)] - 1];

                *eiostream << "HeatTransfer Surface," << thisSurface.Name << "," << cSurfaceClass(thisSurface.Class) << "," << BaseSurfName << ","
                           << AlgoName << ",";

                // NOTE - THIS CODE IS REPEATED IN SurfaceGeometry.cc IN SetupZoneGeometry
                // Calculate Nominal U-value with convection/film coefficients for reporting by adding on
                // prescribed R-values for interior and exterior convection coefficients as found in ASHRAE 90.1-2004, Appendix A
                if (thisSurface.Construction > 0 && thisSurface.Construction <= state.dataHeatBal->TotConstructs) {
                    cNominalUwithConvCoeffs = "";
                    ConstructionName = state.dataConstruction->Construct(thisSurface.Construction).Name;
                    switch (thisSurface.Class) {
                    case DataSurfaces::SurfaceClass::Wall: {
                        // Interior:  vertical, still air, Rcin = 0.68 ft2-F-hr/BTU
                        // Exterior:  vertical, exterior wind exposure, Rcout = 0.17 ft2-F-hr/BTU
                        if (state.dataHeatBal->NominalU(thisSurface.Construction) > 0.0) {
                            NominalUwithConvCoeffs = 1.0 / (0.1197548 + (1.0 / state.dataHeatBal->NominalU(thisSurface.Construction)) + 0.0299387);
                        } else {
                            cNominalUwithConvCoeffs = "[invalid]";
                        }
                    } break;
                    case DataSurfaces::SurfaceClass::Floor: {
                        // Interior:  horizontal, still air, heat flow downward, Rcin = 0.92 ft2-F-hr/BTU
                        // Exterior:  horizontal, semi-exterior (crawlspace), Rcout = 0.46 ft2-F-hr/BTU
                        if (state.dataHeatBal->NominalU(thisSurface.Construction) > 0.0) {
                            NominalUwithConvCoeffs = 1.0 / (0.1620212 + (1.0 / state.dataHeatBal->NominalU(thisSurface.Construction)) + 0.0810106);
                        } else {
                            cNominalUwithConvCoeffs = "[invalid]";
                        }
                    } break;
                    case DataSurfaces::SurfaceClass::Roof: {
                        // Interior:  horizontal, still air, heat flow upward, Rcin = 0.61 ft2-F-hr/BTU
                        // Exterior:  horizontal, semi-exterior (attic), Rcout = 0.46 ft2-F-hr/BTU
                        if (state.dataHeatBal->NominalU(thisSurface.Construction) > 0.0) {
                            NominalUwithConvCoeffs = 1.0 / (0.1074271 + (1.0 / state.dataHeatBal->NominalU(thisSurface.Construction)) + 0.0810106);
                        } else {
                            cNominalUwithConvCoeffs = "[invalid]";
                        }
                    } break;
                    default: {
                        if (state.dataHeatBal->NominalU(thisSurface.Construction) > 0.0) {
                            NominalUwithConvCoeffs = state.dataHeatBal->NominalU(thisSurface.Construction);
                        } else {
                            cNominalUwithConvCoeffs = "[invalid]";
                        }
                    } break;
                    }
                    if (cNominalUwithConvCoeffs.empty()) {
                        cNominalUwithConvCoeffs = format("{:.3R}", NominalUwithConvCoeffs);
                    } else {
                        cNominalUwithConvCoeffs = "[invalid]";
                    }
                    if ((thisSurface.Class == DataSurfaces::SurfaceClass::Window) || (thisSurface.Class == DataSurfaces::SurfaceClass::TDD_Dome)) {
                        // DataSurfaces::SurfaceClass::Window also covers glass doors and TDD:Diffusers
                        cNominalU = "N/A";
                        if (state.dataSurface->SurfWinSolarDiffusing(surf)) {
                            SolarDiffusing = "Yes";
                        } else {
                            SolarDiffusing = "No";
                        }
                    } else {
                        cNominalU = format("{:.3R}", state.dataHeatBal->NominalU(thisSurface.Construction));
                    }
                } else {
                    cNominalUwithConvCoeffs = "**";
                    cNominalU = "**";
                    ConstructionName = "**invalid**";
                }

                *eiostream << ConstructionName << "," << cNominalU << "," << cNominalUwithConvCoeffs << "," << SolarDiffusing << ","
                           << format("{:.2R}", thisSurface.Area) << "," << format("{:.2R}", thisSurface.GrossArea) << ","
                           << format("{:.2R}", thisSurface.NetAreaShadowCalc) << "," << format("{:.2R}", thisSurface.Azimuth) << ","
                           << format("{:.2R}", thisSurface.Tilt) << "," << format("{:.2R}", thisSurface.Width) << ","
                           << format("{:.2R}", thisSurface.Height) << "," << format("{:.2R}", thisSurface.Reveal) << ",";

                static constexpr std::array<std::string_view, (int)Convect::OverrideType::Num> overrideTypeStrs = {
                    "User Supplied Value", "User Supplied Schedule", "User Supplied Curve", "User Specified Model"};

                auto const &surfIntConv = state.dataSurface->surfIntConv(surf);
                auto const &surfExtConv = state.dataSurface->surfExtConv(surf);
                if (surfIntConv.userModelNum != 0) {
                    IntConvCoeffCalc = overrideTypeStrs[(int)state.dataSurface->userIntConvModels(surfIntConv.userModelNum).overrideType];
                } else {
                    Convect::HcInt hcInt = surfIntConv.model;
                    if (hcInt == Convect::HcInt::SetByZone) {
                        hcInt = state.dataHeatBal->Zone(ZoneNum).IntConvAlgo;
                    }
                    IntConvCoeffCalc = ConvCoeffCalcs[Convect::HcIntReportVals[(int)hcInt] - 1];
                }
                if (surfExtConv.userModelNum != 0) {
                    ExtConvCoeffCalc = overrideTypeStrs[(int)state.dataSurface->userExtConvModels(surfExtConv.userModelNum).overrideType];
                } else {
                    Convect::HcExt hcExt = surfExtConv.model;
                    if (hcExt == Convect::HcExt::SetByZone) {
                        hcExt = state.dataHeatBal->Zone(ZoneNum).ExtConvAlgo;
                    }
                    ExtConvCoeffCalc = ConvCoeffCalcs[Convect::HcExtReportVals[static_cast<int>(hcExt)] - 1];
                }
                if (thisSurface.ExtBoundCond == DataSurfaces::ExternalEnvironment) {
                    *eiostream << "ExternalEnvironment" << "," << ExtConvCoeffCalc << "," << IntConvCoeffCalc << ",";
                } else if (thisSurface.ExtBoundCond == DataSurfaces::Ground) {
                    *eiostream << "Ground" << "," << "N/A-Ground" << "," << IntConvCoeffCalc << ",";
                } else if (thisSurface.ExtBoundCond == DataSurfaces::GroundFCfactorMethod) {
                    *eiostream << "FCGround" << "," << "N/A-FCGround" << "," << IntConvCoeffCalc << ",";
                } else if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
                    *eiostream << "Foundation" << "," << "N/A-Foundation" << "," << IntConvCoeffCalc << ",";
                } else if (thisSurface.ExtBoundCond == DataSurfaces::OtherSideCoefNoCalcExt ||
                           thisSurface.ExtBoundCond == DataSurfaces::OtherSideCoefCalcExt) {
                    *eiostream << state.dataSurface->OSC(thisSurface.OSCPtr).Name << "," << "N/A-OSC" << "," << IntConvCoeffCalc << ",";
                } else if (thisSurface.ExtBoundCond == DataSurfaces::OtherSideCondModeledExt) {
                    *eiostream << state.dataSurface->OSCM(thisSurface.OSCMPtr).Name << "," << "N/A-OSCM" << "," << IntConvCoeffCalc << ",";
                } else {
                    *eiostream << thisSurface.ExtBoundCondName << "," << "Other/Same Surface Int Conv" << "," << IntConvCoeffCalc << ",";
                }
                if (thisSurface.ExtSolar) {
                    *eiostream << "SunExposed" << ",";
                } else {
                    *eiostream << "NoSun" << ",";
                }
                if (thisSurface.ExtWind) {
                    *eiostream << "WindExposed" << ",";
                } else {
                    *eiostream << "NoWind" << ",";
                }
                if (RptType == 10) {
                    *eiostream << format("{:.2R}", thisSurface.ViewFactorGround) << "," << format("{:.2R}", thisSurface.ViewFactorSky) << ","
                               << format("{:.2R}", thisSurface.ViewFactorGroundIR) << "," << format("{:.2R}", thisSurface.ViewFactorSkyIR) << ","
                               << fmt::to_string(thisSurface.Sides) << '\n';
                } else {
                    *eiostream << format("{:.2R}", thisSurface.ViewFactorGround) << "," << format("{:.2R}", thisSurface.ViewFactorSky) << ","
                               << format("{:.2R}", thisSurface.ViewFactorGroundIR) << "," << format("{:.2R}", thisSurface.ViewFactorSkyIR) << ","
                               << fmt::to_string(thisSurface.Sides) << ",";
                    for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                        if (vert != thisSurface.Sides) {
                            *eiostream << format("{:.2R}", thisSurface.Vertex(vert).x) << "," << format("{:.2R}", thisSurface.Vertex(vert).y) << ","
                                       << format("{:.2R}", thisSurface.Vertex(vert).z) << ",";
                        } else {
                            *eiostream << format("{:.2R}", thisSurface.Vertex(vert).x) << "," << format("{:.2R}", thisSurface.Vertex(vert).y) << ","
                                       << format("{:.2R}", thisSurface.Vertex(vert).z) << '\n';
                        }
                    }
                    if (thisSurface.Sides == 0) {
                        *eiostream << '\n';
                    }
                }
                // if window, report frame/divider as appropriate
                if (thisSurface.FrameDivider > 0) {
                    int fd = thisSurface.FrameDivider;
                    if (state.dataSurface->FrameDivider(fd).FrameWidth > 0.0) {
                        AlgoName = DataSurfaces::HeatTransAlgoStrs[(int)thisSurface.HeatTransferAlgorithm];
                        *eiostream << "Frame/Divider Surface," << state.dataSurface->FrameDivider(fd).Name << "," << "Frame," << thisSurface.Name
                                   << "," << AlgoName << ",";
                        *eiostream << ",N/A,N/A,," << format("{:.2R}", state.dataSurface->SurfWinFrameArea(surf)) << ","
                                   << format("{:.2R}", state.dataSurface->SurfWinFrameArea(surf) / thisSurface.Multiplier) << ",*" << ",N/A"
                                   << ",N/A," << format("{:.2R}", state.dataSurface->FrameDivider(fd).FrameWidth) << ",N/A" << '\n';
                    }
                    if (state.dataSurface->FrameDivider(fd).DividerWidth > 0.0) {
                        if (state.dataSurface->FrameDivider(fd).DividerType == DataSurfaces::FrameDividerType::DividedLite) {
                            *eiostream << "Frame/Divider Surface," << state.dataSurface->FrameDivider(fd).Name << "," << "Divider:DividedLite,"
                                       << thisSurface.Name << ",,";
                        } else {
                            *eiostream << "Frame/Divider Surface," << state.dataSurface->FrameDivider(fd).Name << "," << "Divider:Suspended,"
                                       << thisSurface.Name << ",,";
                        }
                        *eiostream << ",N/A,N/A,," << format("{:.2R}", state.dataSurface->SurfWinDividerArea(surf)) << ","
                                   << format("{:.2R}", state.dataSurface->SurfWinDividerArea(surf) / thisSurface.Multiplier) << ",*" << ",N/A"
                                   << ",N/A," << format("{:.2R}", state.dataSurface->FrameDivider(fd).DividerWidth) << ",N/A" << '\n';
                    }
                }
            } else { // RptType=1  Vertices only
                if (thisSurface.BaseSurf == surf) {
                    BaseSurfName = "";
                } else {
                    BaseSurfName = thisSurface.BaseSurfName;
                }

                AlgoName = DataSurfaces::HeatTransAlgoStrs[(int)thisSurface.HeatTransferAlgorithm];

                *eiostream << "HeatTransfer Surface," << thisSurface.Name << "," << cSurfaceClass(thisSurface.Class) << "," << BaseSurfName << ","
                           << AlgoName << ",";
                *eiostream << fmt::to_string(thisSurface.Sides) << ",";
                for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                    if (vert != thisSurface.Sides) {
                        *eiostream << format("{:.2R}", thisSurface.Vertex(vert).x) << "," << format("{:.2R}", thisSurface.Vertex(vert).y) << ","
                                   << format("{:.2R}", thisSurface.Vertex(vert).z) << ",";
                    } else {
                        *eiostream << format("{:.2R}", thisSurface.Vertex(vert).x) << "," << format("{:.2R}", thisSurface.Vertex(vert).y) << ","
                                   << format("{:.2R}", thisSurface.Vertex(vert).z) << '\n';
                    }
                }
                if (thisSurface.Sides == 0) {
                    *eiostream << '\n';
                }
            }
        } // surfaces
    } // zones

    print(state.files.eio, "{}", eiostream->str());
}

void CostInfoOut(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   April 2003
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine produces a file with information about surfaces.
    // for the purpose of producing first cost estimates to include in objective value functions
    // for design optimization

    // METHODOLOGY EMPLOYED:
    // Access data in DataSurfaces and report

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Array1D_bool uniqueSurf;

    if (state.dataSurface->TotSurfaces > 0 && !allocated(state.dataSurface->Surface)) {
        // no error needed, probably in end processing, just return
        return;
    }

    // need to determine unique surfaces... some surfaces are shared by zones and hence doubled
    uniqueSurf.dimension(state.dataSurface->TotSurfaces, true);

    for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
        auto const &thisSurface = state.dataSurface->Surface(surf);
        if (thisSurface.ExtBoundCond > 0) {
            if (thisSurface.ExtBoundCond < surf) { // already cycled through
                uniqueSurf(surf) = false;
            }
        }
        if (thisSurface.Construction == 0) { // throw out others for now
            uniqueSurf(surf) = false;
        }
    }

    auto scifile = state.files.sci.open(state, "CostInfoOut", state.files.outputControl.sci);

    print(scifile, "{:12}{:12}\n", state.dataSurface->TotSurfaces, count(uniqueSurf));
    print(scifile, "{}\n", " data for surfaces useful for cost information");
    print(scifile, "{}\n", " Number, Name, Construction, class, area, grossarea");

    for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
        // if (surface(surf)%class .eq. DataSurfaces::SurfaceClass::IntMass) CYCLE
        if (!uniqueSurf(surf)) {
            continue;
        }
        // why the heck are constructions == 0 ?
        auto &thisSurface = state.dataSurface->Surface(surf);
        if (thisSurface.Construction != 0) {
            // Formats
            static constexpr std::string_view Format_801("{:5},{},{},{},{:14.5F},{:14.5F}\n");
            print<check_syntax(Format_801)>(scifile,
                                            Format_801,
                                            surf,
                                            thisSurface.Name,
                                            state.dataConstruction->Construct(thisSurface.Construction).Name,
                                            cSurfaceClass(thisSurface.Class),
                                            thisSurface.Area,
                                            thisSurface.GrossArea);
        }
    }

    uniqueSurf.deallocate();
}

void VRMLOut(EnergyPlusData &state, const std::string &PolygonAction, const std::string &ColorScheme)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   August 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine produces a file of VRML output for the surfaces.

    // METHODOLOGY EMPLOYED:
    // Use the surface absolute coordinate information to produce
    // lines.

    // SUBROUTINE PARAMETER DEFINITIONS:
    enum class Color
    {
        Invalid = -1,
        Wall,
        Window,
        FixedShade,
        SubShade,
        Roof,
        Floor,
        BldgShade,
        Num
    };

    constexpr static std::array<std::string_view, static_cast<int>(Color::Num)> colorstring = {
        "WALL", "WINDOW", "FIXEDSHADE", "SUBSHADE", "ROOF", "FLOOR", "BLDGSHADE"};

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool ThickPolyline(false);
    bool RegularPolyline(false);
    std::string PolylineWidth(" 0.55");
    bool TriangulateFace(false);

    // Formats
    static constexpr std::string_view Format_702("#VRML V2.0 utf8\n");
    static constexpr std::string_view Format_707(
        "WorldInfo {{\n   title \"Building - {}\"\n   info [\"EnergyPlus Program Version {}\"]\n   info [\"Surface Color Scheme {}\"]\n}}\n");
    static constexpr std::string_view Format_800("Shape {{\nappearance DEF {} Appearance {{\nmaterial Material {{ diffuseColor {} }}\n}}\n}}\n");
    static constexpr std::string_view Format_801(
        "Shape {{\nappearance USE {}\ngeometry IndexedFaceSet {{\nsolid TRUE\ncoord DEF {}{} Coordinate {{\npoint [\n");
    static constexpr std::string_view Format_802("{:15.5F} {:15.5F} {:15.5F},\n");
    static constexpr std::string_view Format_803("]\n}}\ncoordIndex [\n");
    static constexpr std::string_view Format_805("]\nccw TRUE\nsolid TRUE\n}}\n}}\n");

    if (PolygonAction == "TRIANGULATE3DFACE" || PolygonAction == "TRIANGULATE") {
        TriangulateFace = true;
    } else if (PolygonAction == "THICKPOLYLINE" || PolygonAction.empty()) {
        ThickPolyline = true;
    } else if (PolygonAction == "REGULARPOLYLINE") {
        RegularPolyline = true;
        PolylineWidth = " 0";
    } else {
        ShowWarningError(state, format("VRMLOut: Illegal key specified for Surfaces with > 4 sides={}", PolygonAction));
        ShowContinueError(state, "\"TRIANGULATE 3DFACE\" will be used for any surfaces with > 4 sides.");
        TriangulateFace = true;
    }

    if (state.dataSurface->TotSurfaces > 0 && !allocated(state.dataSurface->Surface)) {
        // no error needed, probably in end processing, just return
        return;
    }

    auto wrlfile = state.files.wrl.open(state, "VRMLOut", state.files.outputControl.wrl);

    print(wrlfile, Format_702);

    if (ColorScheme.empty()) {
        print<check_syntax(Format_707)>(
            wrlfile, Format_707, state.dataHeatBal->BuildingName, state.dataStrGlobals->VerStringVar, "Default"); // World Info
    } else {
        print<check_syntax(Format_707)>(
            wrlfile, Format_707, state.dataHeatBal->BuildingName, state.dataStrGlobals->VerStringVar, ColorScheme); // World Info
    }

    print(wrlfile, "# Zone Names\n");
    for (int zones = 1; zones <= state.dataGlobal->NumOfZones; ++zones) {
        print(wrlfile, "# Zone={}:{}\n", zones, normalizeName(state.dataHeatBal->Zone(zones).Name));
    }

    // Define the colors:

    print<check_syntax(Format_800)>(wrlfile, Format_800, "FLOOR", "0.502 0.502 0.502");
    print<check_syntax(Format_800)>(wrlfile, Format_800, "ROOF", "1 1 0");
    print<check_syntax(Format_800)>(wrlfile, Format_800, "WALL", "0 1 0");
    print<check_syntax(Format_800)>(wrlfile, Format_800, "WINDOW", "0 1 1");
    print<check_syntax(Format_800)>(wrlfile, Format_800, "DOOR", "0 1 1");
    print<check_syntax(Format_800)>(wrlfile, Format_800, "GLASSDOOR", "0 1 1");
    print<check_syntax(Format_800)>(wrlfile, Format_800, "FIXEDSHADE", "1 0 1");
    print<check_syntax(Format_800)>(wrlfile, Format_800, "BLDGSHADE", "0 0 1");
    print<check_syntax(Format_800)>(wrlfile, Format_800, "SUBSHADE", "1 0 1");
    print<check_syntax(Format_800)>(wrlfile, Format_800, "BACKCOLOR", "0.502 0.502 0.784");

    Color colorindex = Color::Invalid;

    //  Do all detached shading surfaces first
    for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
        auto &thisSurface = state.dataSurface->Surface(surf);
        if (thisSurface.HeatTransSurf) {
            continue;
        }
        if (thisSurface.IsAirBoundarySurf) {
            continue;
        }
        if (thisSurface.Class == DataSurfaces::SurfaceClass::Shading) {
            continue;
        }
        if (thisSurface.Sides == 0) {
            continue;
        }
        if (thisSurface.Class == DataSurfaces::SurfaceClass::Detached_F) {
            colorindex = Color::FixedShade;
        }
        if (thisSurface.Class == DataSurfaces::SurfaceClass::Detached_B) {
            colorindex = Color::BldgShade;
        }
        if (thisSurface.Class == DataSurfaces::SurfaceClass::Detached_F) {
            print(wrlfile, "# Fixed Shading:{}\n", thisSurface.Name);
        } else if (thisSurface.Class == DataSurfaces::SurfaceClass::Detached_B) {
            print(wrlfile, "# Building Shading:{}", thisSurface.Name);
        }
        print<check_syntax(Format_801)>(wrlfile, Format_801, colorstring[static_cast<int>(colorindex)], "Surf", surf);
        for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
            print<check_syntax(Format_802)>(wrlfile, Format_802, thisSurface.Vertex(vert).x, thisSurface.Vertex(vert).y, thisSurface.Vertex(vert).z);
        }
        print<check_syntax(Format_803)>(wrlfile, Format_803);
        if (thisSurface.Sides <= 4 || !TriangulateFace) {
            for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                print<FormatSyntax::FMT>(wrlfile, " {}", vert - 1);
                if (vert == thisSurface.Sides) {
                    print(wrlfile, " -1\n");
                }
            }
            print<check_syntax(Format_805)>(wrlfile, Format_805);
        } else { // will be >4 sided polygon with triangulate option
            Array1D<DataVectorTypes::dTriangle> mytriangles;
            const int ntri = DXFEarClipping::Triangulate(state,
                                                         thisSurface.Sides,
                                                         thisSurface.Vertex,
                                                         mytriangles,
                                                         thisSurface.Azimuth,
                                                         thisSurface.Tilt,
                                                         thisSurface.Name,
                                                         thisSurface.Class);
            for (int svert = 1; svert <= ntri; ++svert) {
                const int vv0 = mytriangles(svert).vv0;
                const int vv1 = mytriangles(svert).vv1;
                const int vv2 = mytriangles(svert).vv2;
                print(wrlfile, " {} {} {} -1\n", vv0 - 1, vv1 - 1, vv2 - 1);
            }
            print(wrlfile, Format_805);
            mytriangles.deallocate();
        }
    }
    //  ! now do zone surfaces, by zone
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        int oldSurfNum = 0;
        for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
            auto &thisSurface = state.dataSurface->Surface(surf);
            ++oldSurfNum;
            if (thisSurface.Zone != zoneNum) {
                continue;
            }
            if (thisSurface.Sides == 0) {
                continue;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::IntMass) {
                continue;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Wall) {
                colorindex = Color::Wall;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Roof) {
                colorindex = Color::Roof;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::TDD_Dome) {
                colorindex = Color::Window;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Floor) {
                colorindex = Color::Floor;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Window) {
                colorindex = Color::Window;
            }
            if (thisSurface.Class == DataSurfaces::SurfaceClass::Door) {
                colorindex = Color::Window;
            }

            print(wrlfile, "# {}:{}\n", thisSurface.ZoneName, thisSurface.Name);
            print<check_syntax(Format_801)>(wrlfile, Format_801, colorstring[static_cast<int>(colorindex)], "Surf", oldSurfNum);
            for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                print(wrlfile, Format_802, thisSurface.Vertex(vert).x, thisSurface.Vertex(vert).y, thisSurface.Vertex(vert).z);
            }
            print<check_syntax(Format_803)>(wrlfile, Format_803);
            if (thisSurface.Sides <= 4 || !TriangulateFace) {
                for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                    print(wrlfile, " {}", vert - 1);
                    if (vert == thisSurface.Sides) {
                        print(wrlfile, " -1\n");
                    }
                }
                print<check_syntax(Format_805)>(wrlfile, Format_805);
            } else { // will be >4 sided polygon with triangulate option
                Array1D<DataVectorTypes::dTriangle> mytriangles;
                const int ntri = DXFEarClipping::Triangulate(state,
                                                             thisSurface.Sides,
                                                             thisSurface.Vertex,
                                                             mytriangles,
                                                             thisSurface.Azimuth,
                                                             thisSurface.Tilt,
                                                             thisSurface.Name,
                                                             thisSurface.Class);
                for (int svert = 1; svert <= ntri; ++svert) {
                    const int vv0 = mytriangles(svert).vv0;
                    const int vv1 = mytriangles(svert).vv1;
                    const int vv2 = mytriangles(svert).vv2;
                    print(wrlfile, " {} {} {} -1\n", vv0 - 1, vv1 - 1, vv2 - 1);
                }
                print(wrlfile, Format_805);
                mytriangles.deallocate();
            }
        }
        // still have to do shading surfaces for zone
        colorindex = Color::SubShade;
        for (int surf : state.dataSurface->AllSurfaceListReportOrder) {
            auto &thisSurface = state.dataSurface->Surface(surf);
            //      !if (surface(surf)%heattranssurf) CYCLE ! Shading with a construction is allowed to be HT surf for daylighting shelves
            if (thisSurface.Class != DataSurfaces::SurfaceClass::Shading) {
                continue;
            }
            if (thisSurface.ZoneName != state.dataHeatBal->Zone(zoneNum).Name) {
                continue;
            }
            if (thisSurface.Sides == 0) {
                continue;
            }
            print(wrlfile, "# {}:{}\n", thisSurface.ZoneName, thisSurface.Name);
            print<check_syntax(Format_801)>(wrlfile, Format_801, colorstring[static_cast<int>(colorindex)], "Surf", surf);
            for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                print(wrlfile, Format_802, thisSurface.Vertex(vert).x, thisSurface.Vertex(vert).y, thisSurface.Vertex(vert).z);
            }
            print(wrlfile, Format_803);
            if (thisSurface.Sides <= 4 || !TriangulateFace) {
                for (int vert = 1; vert <= thisSurface.Sides; ++vert) {
                    print(wrlfile, " {}", vert - 1);
                    if (vert == thisSurface.Sides) {
                        print(wrlfile, " -1\n");
                    }
                }
                print(wrlfile, Format_805);
            } else { // will be >4 sided polygon with triangulate option
                Array1D<DataVectorTypes::dTriangle> mytriangles;
                const int ntri = DXFEarClipping::Triangulate(state,
                                                             thisSurface.Sides,
                                                             thisSurface.Vertex,
                                                             mytriangles,
                                                             thisSurface.Azimuth,
                                                             thisSurface.Tilt,
                                                             thisSurface.Name,
                                                             thisSurface.Class);
                for (int svert = 1; svert <= ntri; ++svert) {
                    const int vv0 = mytriangles(svert).vv0;
                    const int vv1 = mytriangles(svert).vv1;
                    const int vv2 = mytriangles(svert).vv2;
                    print(wrlfile, " {} {} {} -1\n", vv0 - 1, vv1 - 1, vv2 - 1);
                }
                print(wrlfile, Format_805);
                mytriangles.deallocate();
            }
        }
    }

    // vrml does not have daylighting reference points included
}

} // namespace EnergyPlus
