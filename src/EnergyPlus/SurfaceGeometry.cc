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
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/member.functions.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/ConvectionConstants.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataReportingFlags.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/DataWindowEquivalentLayer.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/Vectors.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/WindowManager.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>

namespace EnergyPlus {

namespace SurfaceGeometry {

    // Module containing the routines dealing with the Surface Geometry

    // MODULE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   June 2000
    //       MODIFIED       DJS (PSU Dec 2006) to add ecoroof

    // PURPOSE OF THIS MODULE:
    // This module performs the functions required of the surface geometry.

    static std::string const BlankString;

    int constexpr unenteredAdjacentSpaceSurface = -997; // allows users to enter one zone surface ("Space")
                                                        // referencing another in adjacent space
    int constexpr unenteredAdjacentZoneSurface = -998;  // allows users to enter one zone surface ("Zone")
                                                        // referencing another in adjacent zone
    int constexpr unreconciledZoneSurface = -999;       // interim value between entering surfaces ("Surface") and reconciling

    void AllocateSurfaceWindows(EnergyPlusData &state, int NumSurfaces)
    {
        state.dataSurface->SurfWinA.dimension(state.dataSurface->TotSurfaces, DataWindowEquivalentLayer::CFSMAXNL + 1, 0.0);
        state.dataSurface->SurfWinADiffFront.dimension(state.dataSurface->TotSurfaces, DataWindowEquivalentLayer::CFSMAXNL + 1, 0.0);
        state.dataSurface->SurfWinACFOverlap.dimension(state.dataSurface->TotSurfaces, state.dataHeatBal->MaxSolidWinLayers, 0.0);

        state.dataSurface->SurfWinFrameQRadOutAbs.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinFrameQRadInAbs.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinDividerQRadOutAbs.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinDividerQRadInAbs.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinExtBeamAbsByShade.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinExtDiffAbsByShade.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinIntBeamAbsByShade.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinIntSWAbsByShade.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinInitialDifSolAbsByShade.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinIntLWAbsByShade.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinConvHeatFlowNatural.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinConvHeatGainToZoneAir.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinRetHeatGainToZoneAir.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinDividerHeatGain.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBlTsolBmBm.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBlTsolBmDif.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBlTsolDifDif.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBlGlSysTsolBmBm.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBlGlSysTsolDifDif.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinScTsolBmBm.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinScTsolBmDif.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinScTsolDifDif.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinScGlSysTsolBmBm.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinScGlSysTsolDifDif.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinGlTsolBmBm.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinGlTsolBmDif.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinGlTsolDifDif.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBmSolTransThruIntWinRep.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBmSolAbsdOutsReveal.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBmSolRefldOutsRevealReport.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBmSolAbsdInsReveal.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBmSolRefldInsReveal.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBmSolRefldInsRevealReport.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinOutsRevealDiffOntoGlazing.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinInsRevealDiffOntoGlazing.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinInsRevealDiffIntoZone.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinOutsRevealDiffOntoFrame.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinInsRevealDiffOntoFrame.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinInsRevealDiffOntoGlazingReport.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinInsRevealDiffIntoZoneReport.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinInsRevealDiffOntoFrameReport.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBmSolAbsdInsRevealReport.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBmSolTransThruIntWinRepEnergy.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBmSolRefldOutsRevealRepEnergy.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBmSolRefldInsRevealRepEnergy.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinProfileAngHor.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinProfileAngVert.dimension(NumSurfaces, 0);

        state.dataSurface->SurfWinShadingFlag.dimension(NumSurfaces, DataSurfaces::WinShadingType::ShadeOff);
        state.dataSurface->SurfWinShadingFlagEMSOn.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinShadingFlagEMSValue.dimension(NumSurfaces, 0.0);
        state.dataSurface->SurfWinStormWinFlag.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinStormWinFlagPrevDay.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinFracTimeShadingDeviceOn.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinExtIntShadePrevTS.dimension(NumSurfaces, DataSurfaces::WinShadingType::ShadeOff);
        state.dataSurface->SurfWinHasShadeOrBlindLayer.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinSurfDayLightInit.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinDaylFacPoint.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinVisTransSelected.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinSwitchingFactor.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinVisTransRatio.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinIRfromParentZone.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinFrameArea.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinFrameConductance.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinFrameSolAbsorp.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinFrameVisAbsorp.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinFrameEmis.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinFrEdgeToCenterGlCondRatio.dimension(NumSurfaces, 1.0);
        state.dataSurface->SurfWinFrameEdgeArea.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinFrameTempIn.dimension(NumSurfaces, 23.0);
        state.dataSurface->SurfWinFrameTempInOld.dimension(NumSurfaces, 23.0);
        state.dataSurface->SurfWinFrameTempSurfOut.dimension(NumSurfaces, 23.0);
        state.dataSurface->SurfWinProjCorrFrOut.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinProjCorrFrIn.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinDividerType.dimension(NumSurfaces, DataSurfaces::FrameDividerType::DividedLite);
        state.dataSurface->SurfWinDividerArea.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinDividerConductance.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinDividerSolAbsorp.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinDividerVisAbsorp.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinDividerEmis.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinDivEdgeToCenterGlCondRatio.dimension(NumSurfaces, 1);
        state.dataSurface->SurfWinDividerEdgeArea.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinDividerTempIn.dimension(NumSurfaces, 23.0);
        state.dataSurface->SurfWinDividerTempInOld.dimension(NumSurfaces, 23.0);
        state.dataSurface->SurfWinDividerTempSurfOut.dimension(NumSurfaces, 23.0);
        state.dataSurface->SurfWinProjCorrDivOut.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinProjCorrDivIn.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinShadeAbsFacFace1.dimension(NumSurfaces, 0.5);
        state.dataSurface->SurfWinShadeAbsFacFace2.dimension(NumSurfaces, 0.5);
        state.dataSurface->SurfWinConvCoeffWithShade.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinOtherConvHeatGain.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinEffInsSurfTemp.dimension(NumSurfaces, 23.0);
        state.dataSurface->SurfWinTotGlazingThickness.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinTanProfileAngHor.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinTanProfileAngVert.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinInsideSillDepth.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinInsideReveal.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinInsideSillSolAbs.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinInsideRevealSolAbs.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinOutsideRevealSolAbs.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinAirflowSource.dimension(NumSurfaces, DataSurfaces::WindowAirFlowSource::Invalid);
        state.dataSurface->SurfWinAirflowDestination.dimension(NumSurfaces, DataSurfaces::WindowAirFlowDestination::Invalid);
        state.dataSurface->SurfWinAirflowReturnNodePtr.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinMaxAirflow.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinAirflowControlType.dimension(NumSurfaces, DataSurfaces::WindowAirFlowControlType::Invalid);
        state.dataSurface->SurfWinAirflowHasSchedule.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinAirflowScheds.dimension(NumSurfaces, nullptr);
        state.dataSurface->SurfWinAirflowThisTS.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinTAirflowGapOutlet.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinWindowCalcIterationsRep.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinVentingOpenFactorMultRep.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinInsideTempForVentingRep.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinVentingAvailabilityRep.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinSkyGndSolarInc.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinBmGndSolarInc.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinSolarDiffusing.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinFrameHeatGain.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinFrameHeatLoss.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinDividerHeatLoss.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinTCLayerTemp.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinSpecTemp.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinWindowModelType.dimension(NumSurfaces, DataSurfaces::WindowModel::Detailed);
        state.dataSurface->SurfWinTDDPipeNum.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinStormWinConstr.dimension(NumSurfaces, 0);
        state.dataSurface->SurfActiveConstruction.dimension(NumSurfaces, 0);
        state.dataSurface->SurfWinActiveShadedConstruction.dimension(NumSurfaces, 0);
    }

    void SetupZoneGeometry(EnergyPlusData &state, bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   September 1977
        //       MODIFIED       April 2002 (FCW): add warning for Solar Distribution
        //                      = FullInteriorExterior when window has reveal
        //                      Add fatal error when triangular window has reveal
        //                      May 2002(FCW): Allow triangular windows to have reveal (subr SHDRVL
        //                      in SolarShading). Remove above warning and fatal error.
        //       RE-ENGINEERED  November 1997 (RKS,LKL)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine controls the processing of detached shadowing and zone surfaces for computing their vertices.

        static constexpr std::string_view RoutineName("SetUpZoneGeometry: ");

        // Zones must have been "gotten" before this call
        // The RelNorth variables are used if "relative" coordinates are input as well as setting up DaylightingCoords

        // these include building north axis and Building Rotation for Appendix G
        state.dataSurfaceGeometry->CosBldgRelNorth =
            std::cos(-(state.dataHeatBal->BuildingAzimuth + state.dataHeatBal->BuildingRotationAppendixG) * Constant::DegToRad);
        state.dataSurfaceGeometry->SinBldgRelNorth =
            std::sin(-(state.dataHeatBal->BuildingAzimuth + state.dataHeatBal->BuildingRotationAppendixG) * Constant::DegToRad);

        // these are only for Building Rotation for Appendix G when using world coordinate system
        state.dataSurfaceGeometry->CosBldgRotAppGonly = std::cos(-state.dataHeatBal->BuildingRotationAppendixG * Constant::DegToRad);
        state.dataSurfaceGeometry->SinBldgRotAppGonly = std::sin(-state.dataHeatBal->BuildingRotationAppendixG * Constant::DegToRad);

        state.dataSurfaceGeometry->CosZoneRelNorth.allocate(state.dataGlobal->NumOfZones);
        state.dataSurfaceGeometry->SinZoneRelNorth.allocate(state.dataGlobal->NumOfZones);

        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {

            state.dataSurfaceGeometry->CosZoneRelNorth(ZoneNum) = std::cos(-state.dataHeatBal->Zone(ZoneNum).RelNorth * Constant::DegToRad);
            state.dataSurfaceGeometry->SinZoneRelNorth(ZoneNum) = std::sin(-state.dataHeatBal->Zone(ZoneNum).RelNorth * Constant::DegToRad);
        }
        GetSurfaceData(state, ErrorsFound);

        if (ErrorsFound) {
            state.dataSurfaceGeometry->CosZoneRelNorth.deallocate();
            state.dataSurfaceGeometry->SinZoneRelNorth.deallocate();
            return;
        }

        ZoneEquipmentManager::GetZoneEquipment(state); // Necessary to get this before window air gap code

        GetWindowGapAirflowControlData(state, ErrorsFound);

        GetStormWindowData(state, ErrorsFound);

        if (!ErrorsFound && state.dataSurface->TotStormWin > 0) {
            CreateStormWindowConstructions(state);
        }

        DataHeatBalance::SetFlagForWindowConstructionWithShadeOrBlindLayer(state);

        state.dataSurfaceGeometry->CosZoneRelNorth.deallocate();
        state.dataSurfaceGeometry->SinZoneRelNorth.deallocate();

        state.dataHeatBal->CalcWindowRevealReflection = false; // Set to True in ProcessSurfaceVertices if beam solar reflection from window reveals
        // is requested for one or more exterior windows.
        state.dataSurface->BuildingShadingCount = 0;
        state.dataSurface->FixedShadingCount = 0;
        state.dataSurface->AttachedShadingCount = 0;
        state.dataSurface->ShadingSurfaceFirst = 0;
        state.dataSurface->ShadingSurfaceLast = -1;

        // Reserve space to avoid excess allocations
        state.dataSurface->AllExtSolAndShadingSurfaceList.reserve(state.dataSurface->TotSurfaces);

        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) { // Loop through all surfaces...
            auto &thisSurface = state.dataSurface->Surface(SurfNum);

            state.dataSurface->SurfAirSkyRadSplit(SurfNum) = std::sqrt(0.5 * (1.0 + thisSurface.CosTilt));

            // Set flag that determines whether a surface is a shadowing surface
            thisSurface.IsShadowing = false;
            if (thisSurface.Class == SurfaceClass::Shading || thisSurface.Class == SurfaceClass::Detached_F ||
                thisSurface.Class == SurfaceClass::Detached_B) {
                thisSurface.IsShadowing = true;
                if (state.dataSurface->ShadingSurfaceFirst == 0) {
                    state.dataSurface->ShadingSurfaceFirst = SurfNum;
                }
                state.dataSurface->ShadingSurfaceLast = SurfNum;
            }
            if ((thisSurface.HeatTransSurf && thisSurface.ExtSolar) || thisSurface.IsShadowing) {
                // Some attached shading surfaces may be true for both
                state.dataSurface->AllExtSolAndShadingSurfaceList.push_back(SurfNum);
            }
            if (thisSurface.Class == SurfaceClass::Shading) {
                ++state.dataSurface->AttachedShadingCount;
            }
            if (thisSurface.Class == SurfaceClass::Detached_F) {
                ++state.dataSurface->FixedShadingCount;
            }
            if (thisSurface.Class == SurfaceClass::Detached_B) {
                ++state.dataSurface->BuildingShadingCount;
            }

            if (thisSurface.Class != SurfaceClass::IntMass) {
                ProcessSurfaceVertices(state, SurfNum, ErrorsFound);
            }
        }

        for (auto &e : state.dataHeatBal->Zone) {
            e.ExtWindowArea = 0.0;
            e.HasWindow = false;
            e.ExtGrossWallArea = 0.0;
            e.ExtNetWallArea = 0.0;
            e.TotalSurfArea = 0.0;
        }

        for (auto &s : state.dataHeatBal->space) {
            s.extWindowArea = 0.0;
            s.totalSurfArea = 0.0;
        }
        bool DetailedWWR = (state.dataInputProcessing->inputProcessor->getNumSectionsFound("DETAILEDWWR_DEBUG") > 0);
        if (DetailedWWR) {
            print(state.files.debug, "{}", "=======User Entered Classification =================");
            print(state.files.debug, "{}", "Surface,Class,Area,Tilt");
        }

        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) { // Loop through all surfaces to find windows...
            auto &thisSurface = state.dataSurface->Surface(SurfNum);

            if (!thisSurface.HeatTransSurf && !thisSurface.IsAirBoundarySurf) {
                continue; // Skip shadowing (sub)surfaces
            }
            auto &thisZone = state.dataHeatBal->Zone(thisSurface.Zone);
            auto &thisSpace = state.dataHeatBal->space(thisSurface.spaceNum);

            thisZone.TotalSurfArea += thisSurface.Area;
            thisSpace.totalSurfArea += thisSurface.Area;
            if (thisSurface.Class == SurfaceClass::Roof) {
                thisZone.geometricCeilingArea += thisSurface.GrossArea;
            } else if (thisSurface.Class == SurfaceClass::Floor) {
                thisZone.geometricFloorArea += thisSurface.GrossArea;
            }
            if (state.dataConstruction->Construct(thisSurface.Construction).TypeIsWindow) {
                thisZone.TotalSurfArea += state.dataSurface->SurfWinFrameArea(SurfNum);
                thisZone.HasWindow = true;
                thisSpace.totalSurfArea += state.dataSurface->SurfWinFrameArea(SurfNum);
                if (((thisSurface.ExtBoundCond == DataSurfaces::ExternalEnvironment) ||
                     (thisSurface.ExtBoundCond == DataSurfaces::OtherSideCondModeledExt)) &&
                    (thisSurface.Class != SurfaceClass::TDD_Dome)) {
                    thisZone.ExtWindowArea += thisSurface.GrossArea;
                    thisSpace.extWindowArea += thisSurface.GrossArea;
                    thisZone.ExtWindowArea_Multiplied =
                        thisZone.ExtWindowArea + thisSurface.GrossArea * thisSurface.Multiplier * thisZone.Multiplier * thisZone.ListMultiplier;
                    if (DetailedWWR) {
                        print(state.files.debug,
                              "{},Window,{:.2R},{:.1R}\n",
                              thisSurface.Name,
                              thisSurface.GrossArea * thisSurface.Multiplier * thisZone.Multiplier * thisZone.ListMultiplier,
                              thisSurface.Tilt);
                    }
                }
            } else {
                if (thisSurface.ExtBoundCond == DataSurfaces::ExternalEnvironment ||
                    thisSurface.ExtBoundCond == DataSurfaces::OtherSideCondModeledExt) {
                    thisZone.ExteriorTotalSurfArea += thisSurface.GrossArea;
                    thisSpace.ExteriorTotalSurfArea += thisSurface.GrossArea;
                    if (thisSurface.Class == SurfaceClass::Wall) {
                        thisZone.ExtNetWallArea += thisSurface.Area;
                        thisZone.ExtGrossWallArea += thisSurface.GrossArea;
                        thisSpace.ExtGrossWallArea += thisSurface.GrossArea;
                        thisZone.ExtGrossWallArea_Multiplied += thisSurface.GrossArea * thisZone.Multiplier * thisZone.ListMultiplier;
                        thisZone.extPerimeter += thisSurface.Width;
                        thisSpace.extPerimeter += thisSurface.Width;
                        if (DetailedWWR) {
                            print(state.files.debug,
                                  "{},Wall,{:.2R},{:.1R}\n",
                                  thisSurface.Name,
                                  thisSurface.GrossArea * thisZone.Multiplier * thisZone.ListMultiplier,
                                  thisSurface.Tilt);
                        }
                    }
                } else if (thisSurface.ExtBoundCond == DataSurfaces::Ground || thisSurface.ExtBoundCond == DataSurfaces::GroundFCfactorMethod ||
                           thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
                    thisZone.ExteriorTotalGroundSurfArea += thisSurface.GrossArea;
                    if (thisSurface.Class == SurfaceClass::Wall) {
                        thisZone.ExtGrossGroundWallArea += thisSurface.GrossArea;
                        thisZone.ExtGrossGroundWallArea_Multiplied += thisSurface.GrossArea * thisZone.Multiplier * thisZone.ListMultiplier;
                        if (DetailedWWR) {
                            print(state.files.debug,
                                  "{},Wall-GroundContact,{:.2R},{:.1R}\n",
                                  thisSurface.Name,
                                  thisSurface.GrossArea * thisZone.Multiplier * thisZone.ListMultiplier,
                                  thisSurface.Tilt);
                        }
                    }
                }
            }

        } // ...end of surfaces windows DO loop

        if (DetailedWWR) {
            print(state.files.debug, "{}\n", "========================");
            print(state.files.debug, "{}\n", "Zone,ExtWallArea,ExtWindowArea");
        }

        for (auto &thisZone : state.dataHeatBal->Zone) {
            int CeilCount = 0;
            int FloorCount = 0;
            int WallCount = 0;
            Real64 AverageHeight = 0.0; // Used to keep track of average height of a surface/zone
            Real64 ZMax = -99999.0;     // Maximum Z of a surface (detailed outside coefficient calculation)
            Real64 ZMin = 99999.0;      // Minimum Z of a surface (detailed outside coefficient calculation)
            Real64 ZCeilAvg = 0.0;
            Real64 ZFlrAvg = 0.0;
            if (DetailedWWR) {
                print(state.files.debug, "{},{:.2R},{:.2R}\n", thisZone.Name, thisZone.ExtGrossWallArea, thisZone.ExtWindowArea);
            }
            for (int spaceNum : thisZone.spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                // Use AllSurfaceFirst which includes air boundaries
                for (int SurfNum = thisSpace.AllSurfaceFirst; SurfNum <= thisSpace.AllSurfaceLast; ++SurfNum) {
                    auto &thisSurface = state.dataSurface->Surface(SurfNum);

                    if (thisSurface.Class == SurfaceClass::Roof) {
                        // Use Average Z for surface, more important for roofs than floors...
                        ++CeilCount;
                        Real64 Z1 = minval(thisSurface.Vertex, &Vector::z);
                        Real64 Z2 = maxval(thisSurface.Vertex, &Vector::z);
                        //        ZCeilAvg=ZCeilAvg+(Z1+Z2)/2.d0
                        ZCeilAvg += ((Z1 + Z2) / 2.0) * (thisSurface.GrossArea / thisZone.geometricCeilingArea);
                    }
                    if (thisSurface.Class == SurfaceClass::Floor) {
                        // Use Average Z for surface, more important for roofs than floors...
                        ++FloorCount;
                        Real64 Z1 = minval(thisSurface.Vertex, &Vector::z);
                        Real64 Z2 = maxval(thisSurface.Vertex, &Vector::z);
                        //        ZFlrAvg=ZFlrAvg+(Z1+Z2)/2.d0
                        ZFlrAvg += ((Z1 + Z2) / 2.0) * (thisSurface.GrossArea / thisZone.geometricFloorArea);
                    }
                    if (thisSurface.Class == SurfaceClass::Wall) {
                        // Use Wall calculation in case no roof & floor in zone
                        ++WallCount;
                        if (WallCount == 1) {
                            ZMax = thisSurface.Vertex(1).z;
                            ZMin = ZMax;
                        }
                        ZMax = max(ZMax, maxval(thisSurface.Vertex, &Vector::z));
                        ZMin = min(ZMin, minval(thisSurface.Vertex, &Vector::z));
                    }
                }
            }
            if (CeilCount > 0 && FloorCount > 0) {
                AverageHeight = ZCeilAvg - ZFlrAvg;
            } else {
                AverageHeight = (ZMax - ZMin);
            }
            if (AverageHeight <= 0.0) {
                AverageHeight = (ZMax - ZMin);
            }

            if (thisZone.CeilingHeight > 0.0) {
                thisZone.ceilingHeightEntered = true;
                if (AverageHeight > 0.0) {
                    if (std::abs(AverageHeight - thisZone.CeilingHeight) / thisZone.CeilingHeight > 0.05) {
                        if (state.dataSurfaceGeometry->ErrCount == 1 && !state.dataGlobal->DisplayExtraWarnings) {
                            ShowWarningError(
                                state,
                                format("{}Entered Ceiling Height for some zone(s) significantly different from calculated Ceiling Height",
                                       RoutineName));
                            ShowContinueError(state,
                                              "...use Output:Diagnostics,DisplayExtraWarnings; to show more details on each max iteration exceeded.");
                        }
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            ShowWarningError(state,
                                             format("{}Entered Ceiling Height for Zone=\"{}\" significantly different from calculated Ceiling Height",
                                                    RoutineName,
                                                    thisZone.Name));
                            static constexpr std::string_view ValFmt("{:.2F}");
                            std::string String1 = format(ValFmt, thisZone.CeilingHeight);
                            std::string String2 = format(ValFmt, AverageHeight);
                            ShowContinueError(
                                state,
                                format("{}Entered Ceiling Height={}, Calculated Ceiling Height={}, entered height will be used in calculations.",
                                       RoutineName,
                                       String1,
                                       String2));
                        }
                    }
                }
            }
            if ((thisZone.CeilingHeight <= 0.0) && (AverageHeight > 0.0)) {
                thisZone.CeilingHeight = AverageHeight;
            }
            // Need to add check here - don't touch if already user-specified
        }

        CalculateZoneVolume(state); // Calculate Zone Volumes

        // Calculate zone centroid (and min/max x,y,z for zone)
        // Use AllSurfaceFirst which includes air boundaries
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            auto &thisZone = state.dataHeatBal->Zone(ZoneNum);
            bool nonInternalMassSurfacesPresent = false;
            bool internalMassSurfacesPresent = false;
            Real64 TotSurfArea = 0.0;
            thisZone.Centroid = Vector(0.0, 0.0, 0.0);
            if ((thisZone.AllSurfaceFirst > 0) && (state.dataSurface->Surface(thisZone.AllSurfaceFirst).Sides > 0)) {
                thisZone.MinimumX = state.dataSurface->Surface(thisZone.AllSurfaceFirst).Vertex(1).x;
                thisZone.MaximumX = state.dataSurface->Surface(thisZone.AllSurfaceFirst).Vertex(1).x;
                thisZone.MinimumY = state.dataSurface->Surface(thisZone.AllSurfaceFirst).Vertex(1).y;
                thisZone.MaximumY = state.dataSurface->Surface(thisZone.AllSurfaceFirst).Vertex(1).y;
                thisZone.MinimumZ = state.dataSurface->Surface(thisZone.AllSurfaceFirst).Vertex(1).z;
                thisZone.MaximumZ = state.dataSurface->Surface(thisZone.AllSurfaceFirst).Vertex(1).z;
            }
            for (int spaceNum : thisZone.spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);

                for (int SurfNum = thisSpace.AllSurfaceFirst; SurfNum <= thisSpace.AllSurfaceLast; ++SurfNum) {
                    auto &thisSurface = state.dataSurface->Surface(SurfNum);
                    if (thisSurface.Class == SurfaceClass::IntMass) {
                        internalMassSurfacesPresent = true;
                        continue;
                    }
                    if (!thisSurface.IsAirBoundarySurf) {
                        nonInternalMassSurfacesPresent = true;
                    }
                    if (thisSurface.Class == SurfaceClass::Wall || (thisSurface.Class == SurfaceClass::Roof) ||
                        (thisSurface.Class == SurfaceClass::Floor)) {

                        thisZone.Centroid.x += thisSurface.Centroid.x * thisSurface.GrossArea;
                        thisZone.Centroid.y += thisSurface.Centroid.y * thisSurface.GrossArea;
                        thisZone.Centroid.z += thisSurface.Centroid.z * thisSurface.GrossArea;
                        TotSurfArea += thisSurface.GrossArea;
                    }
                    thisZone.MinimumX = min(thisZone.MinimumX, minval(thisSurface.Vertex, &Vector::x));
                    thisZone.MaximumX = max(thisZone.MaximumX, maxval(thisSurface.Vertex, &Vector::x));
                    thisZone.MinimumY = min(thisZone.MinimumY, minval(thisSurface.Vertex, &Vector::y));
                    thisZone.MaximumY = max(thisZone.MaximumY, maxval(thisSurface.Vertex, &Vector::y));
                    thisZone.MinimumZ = min(thisZone.MinimumZ, minval(thisSurface.Vertex, &Vector::z));
                    thisZone.MaximumZ = max(thisZone.MaximumZ, maxval(thisSurface.Vertex, &Vector::z));
                }
            }
            if (TotSurfArea > 0.0) {
                thisZone.Centroid.x /= TotSurfArea;
                thisZone.Centroid.y /= TotSurfArea;
                thisZone.Centroid.z /= TotSurfArea;
            }
            if (internalMassSurfacesPresent && !nonInternalMassSurfacesPresent) {
                ShowSevereError(
                    state, format("{}Zone=\"{}\" has only internal mass surfaces.  Need at least one other surface.", RoutineName, thisZone.Name));
                ErrorsFound = true;
            }
        }

        state.dataSurface->SurfAdjacentZone.dimension(state.dataSurface->TotSurfaces, 0);
        // note -- adiabatic surfaces will show same zone as surface
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            if (state.dataSurface->Surface(SurfNum).ExtBoundCond <= 0) {
                continue;
            }
            state.dataSurface->SurfAdjacentZone(SurfNum) = state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).ExtBoundCond).Zone;
        }

        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                auto const &thisSurface = state.dataSurface->Surface(SurfNum);
                if (!thisSurface.HeatTransSurf && thisSurface.ZoneName == state.dataHeatBal->Zone(ZoneNum).Name) {
                    ++state.dataHeatBal->Zone(ZoneNum).NumShadingSurfaces;
                }

                if (thisSurface.Zone != ZoneNum) {
                    continue;
                }

                if (thisSurface.HeatTransSurf && (thisSurface.Class == SurfaceClass::Wall || thisSurface.Class == SurfaceClass::Roof ||
                                                  thisSurface.Class == SurfaceClass::Floor)) {
                    ++state.dataHeatBal->Zone(ZoneNum).NumSurfaces;
                }

                if (thisSurface.HeatTransSurf && (thisSurface.Class == SurfaceClass::Window || thisSurface.Class == SurfaceClass::GlassDoor ||
                                                  thisSurface.Class == SurfaceClass::Door || thisSurface.Class == SurfaceClass::TDD_Dome ||
                                                  thisSurface.Class == SurfaceClass::TDD_Diffuser)) {
                    ++state.dataHeatBal->Zone(ZoneNum).NumSubSurfaces;
                }

            } // surfaces
        } // zones

        for (int const SurfNum : state.dataSurface->AllSurfaceListReportOrder) {
            auto const &thisSurface = state.dataSurface->Surface(SurfNum);
            Real64 NominalUwithConvCoeffs = 0.0;
            if (thisSurface.Construction > 0 && thisSurface.Construction <= state.dataHeatBal->TotConstructs) {
                bool isWithConvCoefValid = false;
                NominalUwithConvCoeffs = DataHeatBalance::ComputeNominalUwithConvCoeffs(state, SurfNum, isWithConvCoefValid);
            }

            // populate the predefined report related to u-values with films
            // only exterior surfaces including underground
            DataSurfaces::SurfaceClass const SurfaceClass(thisSurface.Class);
            if ((thisSurface.ExtBoundCond == DataSurfaces::ExternalEnvironment) || (thisSurface.ExtBoundCond == DataSurfaces::Ground) ||
                (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) || (thisSurface.ExtBoundCond == DataSurfaces::GroundFCfactorMethod)) {
                if ((SurfaceClass == SurfaceClass::Wall) || (SurfaceClass == SurfaceClass::Floor) || (SurfaceClass == SurfaceClass::Roof)) {
                    OutputReportPredefined::PreDefTableEntry(
                        state, state.dataOutRptPredefined->pdchOpUfactFilm, thisSurface.Name, NominalUwithConvCoeffs, 3);
                } else if (SurfaceClass == SurfaceClass::Door) {
                    OutputReportPredefined::PreDefTableEntry(
                        state, state.dataOutRptPredefined->pdchDrUfactFilm, thisSurface.Name, NominalUwithConvCoeffs, 3);
                }
            } else {
                if ((SurfaceClass == SurfaceClass::Wall) || (SurfaceClass == SurfaceClass::Floor) || (SurfaceClass == SurfaceClass::Roof)) {
                    OutputReportPredefined::PreDefTableEntry(
                        state, state.dataOutRptPredefined->pdchIntOpUfactFilm, thisSurface.Name, NominalUwithConvCoeffs, 3);
                } else if (SurfaceClass == SurfaceClass::Door) {
                    OutputReportPredefined::PreDefTableEntry(
                        state, state.dataOutRptPredefined->pdchIntDrUfactFilm, thisSurface.Name, NominalUwithConvCoeffs, 3);
                }
            }
        } // surfaces

        // Write number of shadings to initialization output file
        print(state.files.eio,
              "! <Shading Summary>, Number of Fixed Detached Shades, Number of Building Detached Shades, Number of Attached Shades\n");

        print(state.files.eio,
              " Shading Summary,{},{},{}\n",
              state.dataSurface->FixedShadingCount,
              state.dataSurface->BuildingShadingCount,
              state.dataSurface->AttachedShadingCount);

        // Write number of zones header to initialization output file
        print(state.files.eio, "! <Zone Summary>, Number of Zones, Number of Zone Surfaces, Number of SubSurfaces\n");

        print(state.files.eio,
              " Zone Summary,{},{},{}\n",
              state.dataGlobal->NumOfZones,
              state.dataSurface->TotSurfaces - state.dataSurface->FixedShadingCount - state.dataSurface->BuildingShadingCount -
                  state.dataSurface->AttachedShadingCount,
              sum(state.dataHeatBal->Zone, &DataHeatBalance::ZoneData::NumSubSurfaces));

        // Write Zone Information header to the initialization output file
        static constexpr std::string_view Format_721(
            "! <Zone Information>,Zone Name,North Axis {deg},Origin X-Coordinate {m},Origin Y-Coordinate {m},Origin Z-Coordinate "
            "{m},Centroid X-Coordinate {m},Centroid Y-Coordinate {m},Centroid Z-Coordinate {m},Type,Zone Multiplier,Zone List "
            "Multiplier,Minimum X {m},Maximum X {m},Minimum Y {m},Maximum Y {m},Minimum Z {m},Maximum Z {m},Ceiling Height {m},Volume "
            "{m3},Zone Inside Convection Algorithm {Simple-Detailed-CeilingDiffuser-TrombeWall},Zone Outside Convection Algorithm "
            "{Simple-Detailed-Tarp-MoWitt-DOE-2-BLAST}, Floor Area {m2},Exterior Gross Wall Area {m2},Exterior Net Wall Area {m2},Exterior "
            "Window "
            "Area {m2}, Number of Surfaces, Number of SubSurfaces, Number of Shading SubSurfaces,  Part of Total Building Area");
        print(state.files.eio, "{}\n", Format_721);

        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            // Write Zone Information to the initialization output file
            std::string String1;
            std::string String2;
            std::string String3;

            switch (state.dataHeatBal->Zone(ZoneNum).IntConvAlgo) {
            case Convect::HcInt::ASHRAESimple: {
                String1 = "Simple";
            } break;
            case Convect::HcInt::ASHRAETARP: {
                String1 = "TARP";
            } break;
            case Convect::HcInt::CeilingDiffuser: {
                String1 = "CeilingDiffuser";
            } break;
            case Convect::HcInt::TrombeWall: {
                String1 = "TrombeWall";
            } break;
            case Convect::HcInt::AdaptiveConvectionAlgorithm: {
                String1 = "AdaptiveConvectionAlgorithm";
            } break;
            case Convect::HcInt::ASTMC1340: {
                String1 = "ASTMC1340";
            } break;
            default:
                break;
            }

            switch (state.dataHeatBal->Zone(ZoneNum).ExtConvAlgo) {
            case Convect::HcExt::ASHRAESimple: {
                String2 = "Simple";
            } break;
            case Convect::HcExt::ASHRAETARP: {
                String2 = "TARP";
            } break;
            case Convect::HcExt::TarpHcOutside: {
                String2 = "TARP";
            } break;
            case Convect::HcExt::MoWiTTHcOutside: {
                String2 = "MoWitt";
            } break;
            case Convect::HcExt::DOE2HcOutside: {
                String2 = "DOE-2";
            } break;
            case Convect::HcExt::AdaptiveConvectionAlgorithm: {
                String2 = "AdaptiveConvectionAlgorithm";
            } break;
            default:
                break;
            }

            String3 = (state.dataHeatBal->Zone(ZoneNum).isPartOfTotalArea) ? "Yes" : "No";

            static constexpr std::string_view Format_720(
                " Zone Information, "
                "{},{:.1R},{:.2R},{:.2R},{:.2R},{:.2R},{:.2R},{:.2R},{},{},{},{:.2R},{:.2R},{:.2R},{:.2R},{:.2R},{:.2R},"
                "{:.2R},{:.2R},{},{},{:.2R},{:.2R},{:.2R},{:.2R},{},{},{},{}\n");

            print(state.files.eio,
                  Format_720,
                  state.dataHeatBal->Zone(ZoneNum).Name,
                  state.dataHeatBal->Zone(ZoneNum).RelNorth,
                  state.dataHeatBal->Zone(ZoneNum).OriginX,
                  state.dataHeatBal->Zone(ZoneNum).OriginY,
                  state.dataHeatBal->Zone(ZoneNum).OriginZ,
                  state.dataHeatBal->Zone(ZoneNum).Centroid.x,
                  state.dataHeatBal->Zone(ZoneNum).Centroid.y,
                  state.dataHeatBal->Zone(ZoneNum).Centroid.z,
                  state.dataHeatBal->Zone(ZoneNum).OfType,
                  state.dataHeatBal->Zone(ZoneNum).Multiplier,
                  state.dataHeatBal->Zone(ZoneNum).ListMultiplier,
                  state.dataHeatBal->Zone(ZoneNum).MinimumX,
                  state.dataHeatBal->Zone(ZoneNum).MaximumX,
                  state.dataHeatBal->Zone(ZoneNum).MinimumY,
                  state.dataHeatBal->Zone(ZoneNum).MaximumY,
                  state.dataHeatBal->Zone(ZoneNum).MinimumZ,
                  state.dataHeatBal->Zone(ZoneNum).MaximumZ,
                  state.dataHeatBal->Zone(ZoneNum).CeilingHeight,
                  state.dataHeatBal->Zone(ZoneNum).Volume,
                  String1,
                  String2,
                  state.dataHeatBal->Zone(ZoneNum).FloorArea,
                  state.dataHeatBal->Zone(ZoneNum).ExtGrossWallArea,
                  state.dataHeatBal->Zone(ZoneNum).ExtNetWallArea,
                  state.dataHeatBal->Zone(ZoneNum).ExtWindowArea,
                  state.dataHeatBal->Zone(ZoneNum).NumSurfaces,
                  state.dataHeatBal->Zone(ZoneNum).NumSubSurfaces,
                  state.dataHeatBal->Zone(ZoneNum).NumShadingSurfaces,
                  String3);

        } // ZoneNum

        // Set up solar distribution enclosures allowing for any air boundaries
        SetupEnclosuresAndAirBoundaries(state, state.dataViewFactor->EnclSolInfo, SurfaceGeometry::enclosureType::SolarEnclosures, ErrorsFound);

        // Do the Stratosphere check
        DataHeatBalance::SetZoneOutBulbTempAt(state);
        DataHeatBalance::CheckZoneOutBulbTempAt(state);
    }

    void AllocateSurfaceArrays(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 1998

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine allocates all of the arrays at the module level which require allocation.

        // METHODOLOGY EMPLOYED:
        // Allocation is dependent on the user input file.

        state.dataSurface->ShadeV.allocate(state.dataSurface->TotSurfaces);
        for (auto &e : state.dataSurface->ShadeV) {
            e.NVert = 0;
        }
        // Individual components (XV,YV,ZV) allocated in routine ProcessSurfaceVertices
        state.dataSurface->X0.dimension(state.dataSurface->TotSurfaces, 0.0);
        state.dataSurface->Y0.dimension(state.dataSurface->TotSurfaces, 0.0);
        state.dataSurface->Z0.dimension(state.dataSurface->TotSurfaces, 0.0);

        // Surface EMS arrays
        state.dataSurface->SurfEMSConstructionOverrideON.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfEMSConstructionOverrideValue.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfEMSOverrideIntConvCoef.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfEMSValueForIntConvCoef.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfEMSOverrideExtConvCoef.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfEMSValueForExtConvCoef.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfOutDryBulbTempEMSOverrideOn.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfOutDryBulbTempEMSOverrideValue.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfOutWetBulbTempEMSOverrideOn.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfOutWetBulbTempEMSOverrideValue.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfWindSpeedEMSOverrideOn.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfWindSpeedEMSOverrideValue.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfViewFactorGroundEMSOverrideOn.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfViewFactorGroundEMSOverrideValue.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfWindDirEMSOverrideOn.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfWindDirEMSOverrideValue.allocate(state.dataSurface->TotSurfaces);
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            state.dataSurface->SurfEMSConstructionOverrideON(SurfNum) = false;
            state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum) = 0.0;
            state.dataSurface->SurfEMSOverrideIntConvCoef(SurfNum) = false;
            state.dataSurface->SurfEMSValueForIntConvCoef(SurfNum) = 0.0;
            state.dataSurface->SurfEMSOverrideExtConvCoef(SurfNum) = false;
            state.dataSurface->SurfEMSValueForExtConvCoef(SurfNum) = 0.0;
            state.dataSurface->SurfOutDryBulbTempEMSOverrideOn(SurfNum) = false;
            state.dataSurface->SurfOutDryBulbTempEMSOverrideValue(SurfNum) = 0.0;
            state.dataSurface->SurfOutWetBulbTempEMSOverrideOn(SurfNum) = false;
            state.dataSurface->SurfOutWetBulbTempEMSOverrideValue(SurfNum) = 0.0;
            state.dataSurface->SurfWindSpeedEMSOverrideOn(SurfNum) = false;
            state.dataSurface->SurfWindSpeedEMSOverrideValue(SurfNum) = 0.0;
            state.dataSurface->SurfViewFactorGroundEMSOverrideOn(SurfNum) = false;
            state.dataSurface->SurfViewFactorGroundEMSOverrideValue(SurfNum) = 0.0;
            state.dataSurface->SurfWindDirEMSOverrideOn(SurfNum) = false;
            state.dataSurface->SurfWindDirEMSOverrideValue(SurfNum) = 0.0;
        }
        // Following are surface hb arrays
        state.dataSurface->SurfOutDryBulbTemp.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfOutWetBulbTemp.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfOutWindSpeed.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfOutWindDir.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfGenericContam.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfPenumbraID.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfAirSkyRadSplit.allocate(state.dataSurface->TotSurfaces);
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            state.dataSurface->SurfOutDryBulbTemp(SurfNum) = 0.0;
            state.dataSurface->SurfOutWetBulbTemp(SurfNum) = 0.0;
            state.dataSurface->SurfOutWindSpeed(SurfNum) = 0.0;
            state.dataSurface->SurfOutWindDir(SurfNum) = 0.0;
            state.dataSurface->SurfGenericContam(SurfNum) = 0.0;
            state.dataSurface->SurfPenumbraID(SurfNum) = -1;
            state.dataSurface->SurfAirSkyRadSplit(SurfNum) = 0.0;
        }
        // Following are surface property arrays used in SurfaceGeometry
        state.dataSurface->SurfShadowRecSurfNum.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfShadowDisabledZoneList.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfShadowDiffuseSolRefl.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfShadowDiffuseVisRefl.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfShadowGlazingFrac.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfShadowGlazingConstruct.allocate(state.dataSurface->TotSurfaces);
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            state.dataSurface->SurfShadowRecSurfNum(SurfNum) = 0;
            state.dataSurface->SurfShadowDiffuseSolRefl(SurfNum) = 0.0;
            state.dataSurface->SurfShadowDiffuseVisRefl(SurfNum) = 0.0;
            state.dataSurface->SurfShadowGlazingFrac(SurfNum) = 0.0;
            state.dataSurface->SurfShadowGlazingConstruct(SurfNum) = 0;
        }
        state.dataSurface->SurfExtEcoRoof.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfExtCavityPresent.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfExtCavNum.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfIsPV.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfIsICS.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfIsPool.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfICSPtr.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfIsRadSurfOrVentSlabOrPool.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfDaylightingShelfInd.allocate(state.dataSurface->TotSurfaces);
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            state.dataSurface->SurfExtEcoRoof(SurfNum) = false;
            state.dataSurface->SurfExtCavityPresent(SurfNum) = false;
            state.dataSurface->SurfExtCavNum(SurfNum) = 0;
            state.dataSurface->SurfIsPV(SurfNum) = false;
            state.dataSurface->SurfIsICS(SurfNum) = false;
            state.dataSurface->SurfIsPool(SurfNum) = false;
            state.dataSurface->SurfICSPtr(SurfNum) = 0;
            state.dataSurface->SurfIsRadSurfOrVentSlabOrPool(SurfNum) = false;
            state.dataSurface->SurfDaylightingShelfInd(SurfNum) = 0;
        }
        state.dataSurface->SurfLowTempErrCount.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfHighTempErrCount.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->surfIntConv.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfTAirRef.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->SurfTAirRefRpt.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->surfExtConv.allocate(state.dataSurface->TotSurfaces);
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            state.dataSurface->SurfLowTempErrCount(SurfNum) = 0;
            state.dataSurface->SurfHighTempErrCount(SurfNum) = 0;
            state.dataSurface->surfIntConv(SurfNum) = DataSurfaces::SurfIntConv();
            state.dataSurface->surfExtConv(SurfNum) = DataSurfaces::SurfExtConv();
            state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::Invalid;
            state.dataSurface->SurfTAirRefRpt(SurfNum) = static_cast<int>(DataSurfaces::RefAirTemp::Invalid);
        }

        state.dataSurface->intMovInsuls.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->extMovInsuls.allocate(state.dataSurface->TotSurfaces);
    }

    void GetSurfaceData(EnergyPlusData &state, bool &ErrorsFound) // If errors found in input
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   November 1997
        //       MODIFIED       April 1999, Linda Lawrie
        //                      Dec. 2000, FW (add "one-wall zone" checks)
        //       RE-ENGINEERED  May 2000, Linda Lawrie (breakout surface type gets)

        // PURPOSE OF THIS SUBROUTINE:
        // The purpose of this subroutine is to read in the surface information
        // from the input data file and interpret and put in the derived type

        // METHODOLOGY EMPLOYED:
        // The order of surfaces does not matter and the surfaces are resorted into
        // the hierarchical order:
        //  All Shading Surfaces
        //  Airwalls for space x1
        //  Base Surfaces for space x1
        //  Opaque Subsurfaces for space x1
        //  Window Subsurfaces for space x1
        //  TDD Dome Surfaces for space x1
        //  Airwalls for space x2
        //  Base Surfaces for space x2
        //  etc
        //  Pointers are set in the spaces (AllSurfaceFirst/Last, HTSurfaceFirst/Last, OpaqOrIntMassSurfaceFirst/Last, WindowSurfaceFirst/Last,
        //  OpaqOrWinSurfaceFirst/Last, TDDDomeFirst/Last)

        // REFERENCES:
        //   This routine manages getting the input for the following Objects:
        // SurfaceGeometry
        // Surface:Shading:Detached
        // Surface:HeatTransfer
        // Surface:HeatTransfer:Sub
        // Surface:Shading:Attached
        // Surface:InternalMass

        // Vertex input:
        //  N3 , \field Number of Surface Vertices -- Number of (X,Y,Z) groups in this surface
        //       \note currently limited 3 or 4, later?
        //       \min 3
        //       \max 4
        //       \memo vertices are given in SurfaceGeometry coordinates -- if relative, all surface coordinates
        //       \memo are "relative" to the Zone Origin.  if WCS, then building and zone origins are used
        //       \memo for some internal calculations, but all coordinates are given in an "absolute" system.
        //  N4,  \field Vertex 1 X-coordinate
        //       \units m
        //       \type real
        //  N5 , \field Vertex 1 Y-coordinate
        //       \units m
        //       \type real
        //  N6 , \field Vertex 1 Z-coordinate
        //       \units m
        //       \type real
        //  N7,  \field Vertex 2 X-coordinate
        //       \units m
        //       \type real
        //  N8,  \field Vertex 2 Y-coordinate
        //       \units m
        //       \type real
        //  N9,  \field Vertex 2 Z-coordinate
        //       \units m
        //       \type real
        //  N10, \field Vertex 3 X-coordinate
        //       \units m
        //       \type real
        //  N11, \field Vertex 3 Y-coordinate
        //       \units m
        //       \type real
        //  N12, \field Vertex 3 Z-coordinate
        //       \units m
        //       \type real
        //  N13, \field Vertex 4 X-coordinate
        //       \units m
        //       \type real
        //  N14, \field Vertex 4 Y-coordinate
        //       \type real
        //       \units m
        //  N15; \field Vertex 4 Z-coordinate
        //       \units m
        //       \type real

        // The vertices are stored in the surface derived type.
        //      +(1)-------------------------(4)+
        //      |                               |
        //      |                               |
        //      |                               |
        //      +(2)-------------------------(3)+
        //  The above diagram shows the actual coordinate points of a typical wall
        //  (you're on the outside looking toward the wall) as stored into
        //  Surface%Vertex(1:<number-of-sides>)

        static constexpr std::string_view RoutineName = "GetSurfaceData: ";
        using namespace Vectors;
        using namespace DataErrorTracking;

        int ConstrNumFound;    // Construction number of matching interzone surface
        bool NonMatch(false);  // Error for non-matching interzone surfaces
        int MovedSurfs;        // Number of Moved Surfaces (when sorting into hierarchical structure)
        bool SurfError(false); // General Surface Error, causes fatal error at end of routine
        int TotLay;            // Total layers in a construction
        int TotLayFound;       // Total layers in the construction of a matching interzone surface
        // Simple Surfaces (Rectangular)
        int LayNumOutside; // Outside material numbers for a shaded construction
        // entries with two glazing systems
        int NeedToAddSurfaces;    // Surfaces that will be added due to "unentered" other zone surface
        int NeedToAddSubSurfaces; // SubSurfaces that will be added due to "unentered" other zone surface
        int CurNewSurf = 0;
        Real64 SurfWorldAz;
        Real64 SurfTilt;

        int MultFound;
        int MultSurfNum;
        bool SubSurfaceSevereDisplayed;
        bool subSurfaceError(false);
        bool errFlag;

        bool izConstDiff;    // differences in construction for IZ surfaces
        bool izConstDiffMsg; // display message about hb diffs only once.

        // Get the total number of surfaces to allocate derived type and for surface loops

        if (state.dataSurfaceGeometry->GetSurfaceDataOneTimeFlag) {
            return;
        } else {
            state.dataSurfaceGeometry->GetSurfaceDataOneTimeFlag = true;
        }

        GetGeometryParameters(state, ErrorsFound);

        if (state.dataSurface->WorldCoordSystem) {
            bool RelWarning = false;
            if (state.dataHeatBal->BuildingAzimuth != 0.0) {
                RelWarning = true;
            }
            for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
                if (state.dataHeatBal->Zone(ZoneNum).RelNorth != 0.0) {
                    RelWarning = true;
                }
            }
            if (RelWarning && !state.dataSurfaceGeometry->WarningDisplayed) {
                ShowWarningError(
                    state,
                    format("{}World Coordinate System selected.  Any non-zero Building/Zone North Axes or non-zero Zone Origins are ignored.",
                           RoutineName));
                ShowContinueError(state,
                                  "These may be used in daylighting reference point coordinate calculations but not in normal geometry inputs.");
                state.dataSurfaceGeometry->WarningDisplayed = true;
            }
            RelWarning = false;
            for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
                if (state.dataHeatBal->Zone(ZoneNum).OriginX != 0.0) {
                    RelWarning = true;
                }
                if (state.dataHeatBal->Zone(ZoneNum).OriginY != 0.0) {
                    RelWarning = true;
                }
                if (state.dataHeatBal->Zone(ZoneNum).OriginZ != 0.0) {
                    RelWarning = true;
                }
            }
            if (RelWarning && !state.dataSurfaceGeometry->WarningDisplayed) {
                ShowWarningError(
                    state,
                    format("{}World Coordinate System selected.  Any non-zero Building/Zone North Axes or non-zero Zone Origins are ignored.",
                           RoutineName));
                ShowContinueError(state,
                                  "These may be used in daylighting reference point coordinate calculations but not in normal geometry inputs.");
                state.dataSurfaceGeometry->WarningDisplayed = true;
            }
        }

        int TotDetachedFixed = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Shading:Site:Detailed");
        int TotDetachedBldg = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Shading:Building:Detailed");
        int TotRectDetachedFixed = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Shading:Site");
        int TotRectDetachedBldg = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Shading:Building");
        int TotHTSurfs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "BuildingSurface:Detailed");
        int TotDetailedWalls = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Wall:Detailed");
        int TotDetailedRoofs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "RoofCeiling:Detailed");
        int TotDetailedFloors = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Floor:Detailed");
        int TotHTSubs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "FenestrationSurface:Detailed");
        int TotShdSubs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Shading:Zone:Detailed");
        int TotOverhangs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Shading:Overhang");
        int TotOverhangsProjection = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Shading:Overhang:Projection");
        int TotFins = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Shading:Fin");
        int TotFinsProjection = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Shading:Fin:Projection");
        int TotRectWindows = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Window");
        int TotRectDoors = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Door");
        int TotRectGlazedDoors = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "GlazedDoor");
        int TotRectIZWindows = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Window:Interzone");
        int TotRectIZDoors = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Door:Interzone");
        int TotRectIZGlazedDoors = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "GlazedDoor:Interzone");
        int TotRectExtWalls = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Wall:Exterior");
        int TotRectIntWalls = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Wall:Adiabatic");
        int TotRectIZWalls = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Wall:Interzone");
        int TotRectUGWalls = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Wall:Underground");
        int TotRectRoofs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Roof");
        int TotRectCeilings = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Ceiling:Adiabatic");
        int TotRectIZCeilings = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Ceiling:Interzone");
        int TotRectGCFloors = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Floor:GroundContact");
        int TotRectIntFloors = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Floor:Adiabatic");
        int TotRectIZFloors = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Floor:Interzone");

        state.dataSurface->TotOSC = 0;

        int TotIntMassSurfaces = GetNumIntMassSurfaces(state);

        state.dataSurface->TotSurfaces = (TotDetachedFixed + TotDetachedBldg + TotRectDetachedFixed + TotRectDetachedBldg) * 2 + TotHTSurfs +
                                         TotHTSubs + TotShdSubs * 2 + TotIntMassSurfaces + TotOverhangs * 2 + TotOverhangsProjection * 2 +
                                         TotFins * 4 + TotFinsProjection * 4 + TotDetailedWalls + TotDetailedRoofs + TotDetailedFloors +
                                         TotRectWindows + TotRectDoors + TotRectGlazedDoors + TotRectIZWindows + TotRectIZDoors +
                                         TotRectIZGlazedDoors + TotRectExtWalls + TotRectIntWalls + TotRectIZWalls + TotRectUGWalls + TotRectRoofs +
                                         TotRectCeilings + TotRectIZCeilings + TotRectGCFloors + TotRectIntFloors + TotRectIZFloors;

        state.dataSurfaceGeometry->SurfaceTmp.allocate(state.dataSurface->TotSurfaces); // Allocate the Surface derived type appropriately
        state.dataSurfaceGeometry->UniqueSurfaceNames.reserve(state.dataSurface->TotSurfaces);
        // SurfaceTmp structure is allocated via derived type initialization.

        int NumSurfs = 0;
        int AddedSubSurfaces = 0;
        state.dataErrTracking->AskForSurfacesReport = true;

        GetDetShdSurfaceData(state, ErrorsFound, NumSurfs, TotDetachedFixed, TotDetachedBldg);

        GetRectDetShdSurfaceData(state, ErrorsFound, NumSurfs, TotRectDetachedFixed, TotRectDetachedBldg);

        GetHTSurfaceData(state,
                         ErrorsFound,
                         NumSurfs,
                         TotHTSurfs,
                         TotDetailedWalls,
                         TotDetailedRoofs,
                         TotDetailedFloors,
                         state.dataSurfaceGeometry->BaseSurfCls,
                         state.dataSurfaceGeometry->BaseSurfIDs,
                         NeedToAddSurfaces);

        GetRectSurfaces(state,
                        ErrorsFound,
                        NumSurfs,
                        TotRectExtWalls,
                        TotRectIntWalls,
                        TotRectIZWalls,
                        TotRectUGWalls,
                        TotRectRoofs,
                        TotRectCeilings,
                        TotRectIZCeilings,
                        TotRectGCFloors,
                        TotRectIntFloors,
                        TotRectIZFloors,
                        state.dataSurfaceGeometry->BaseSurfIDs,
                        NeedToAddSurfaces);

        GetHTSubSurfaceData(state,
                            ErrorsFound,
                            NumSurfs,
                            TotHTSubs,
                            state.dataSurfaceGeometry->SubSurfCls,
                            state.dataSurfaceGeometry->SubSurfIDs,
                            AddedSubSurfaces,
                            NeedToAddSubSurfaces);

        GetRectSubSurfaces(state,
                           ErrorsFound,
                           NumSurfs,
                           TotRectWindows,
                           TotRectDoors,
                           TotRectGlazedDoors,
                           TotRectIZWindows,
                           TotRectIZDoors,
                           TotRectIZGlazedDoors,
                           state.dataSurfaceGeometry->SubSurfIDs,
                           AddedSubSurfaces,
                           NeedToAddSubSurfaces);

        GetAttShdSurfaceData(state, ErrorsFound, NumSurfs, TotShdSubs);

        GetSimpleShdSurfaceData(state, ErrorsFound, NumSurfs, TotOverhangs, TotOverhangsProjection, TotFins, TotFinsProjection);

        GetIntMassSurfaceData(state, ErrorsFound, NumSurfs);

        state.dataSurface->TotSurfaces = NumSurfs + AddedSubSurfaces + NeedToAddSurfaces + NeedToAddSubSurfaces;

        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors discovered, program terminates.", RoutineName));
        }

        state.dataSurface->Surface.allocate(state.dataSurface->TotSurfaces); // Allocate the Surface derived type appropriately
        state.dataSurface->SurfaceWindow.allocate(state.dataSurface->TotSurfaces);
        state.dataSurface->surfShades.allocate(state.dataSurface->TotSurfaces);
        AllocateSurfaceArrays(state);
        AllocateSurfaceWindows(state, state.dataSurface->TotSurfaces);

        // Have to make room for added surfaces, if needed
        int FirstTotalSurfaces = NumSurfs + AddedSubSurfaces;
        if (NeedToAddSurfaces + NeedToAddSubSurfaces > 0) {
            state.dataSurfaceGeometry->SurfaceTmp.redimension(state.dataSurface->TotSurfaces);
            CurNewSurf = FirstTotalSurfaces;
        }

        // add the "need to add" surfaces
        // Debug    write(outputfiledebug,*) ' need to add ',NeedtoAddSurfaces+NeedToAddSubSurfaces
        for (int SurfNum = 1; SurfNum <= FirstTotalSurfaces; ++SurfNum) {
            auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
            if ((surfTemp.ExtBoundCond != unenteredAdjacentZoneSurface) && (surfTemp.ExtBoundCond != unenteredAdjacentSpaceSurface)) {
                continue;
            }
            // Need to add surface
            ++CurNewSurf;
            // Debug    write(outputfiledebug,*) ' adding surface=',curnewsurf
            auto &newSurf = state.dataSurfaceGeometry->SurfaceTmp(CurNewSurf);
            newSurf = surfTemp;
            //  Basic parameters are the same for both surfaces.
            if (surfTemp.ExtBoundCond == unenteredAdjacentZoneSurface) {
                int Found = Util::FindItemInList(surfTemp.ExtBoundCondName, state.dataHeatBal->Zone, state.dataGlobal->NumOfZones);
                if (Found == 0) {
                    continue;
                }
                newSurf.Zone = Found;
                auto &newZone = state.dataHeatBal->Zone(Found);
                newSurf.ZoneName = newZone.Name;
                assert(newZone.spaceIndexes.size() >= 1);
                newSurf.spaceNum = 0; // clear this here and set later
            } else if (surfTemp.ExtBoundCond == unenteredAdjacentSpaceSurface) {
                int Found = Util::FindItemInList(surfTemp.ExtBoundCondName, state.dataHeatBal->space, state.dataGlobal->numSpaces);
                if (Found == 0) {
                    continue;
                }
                newSurf.spaceNum = Found;
                int zoneNum = state.dataHeatBal->space(Found).zoneNum;
                newSurf.Zone = zoneNum;
                newSurf.ZoneName = state.dataHeatBal->Zone(zoneNum).Name;
            }
            // Reverse Construction
            newSurf.Construction = DataHeatBalance::AssignReverseConstructionNumber(state, surfTemp.Construction, SurfError);
            newSurf.ConstructionStoredInputValue = newSurf.Construction;
            // Reverse Vertices
            int NVert = surfTemp.Sides;
            for (int Vert = 1; Vert <= surfTemp.Sides; ++Vert) {
                newSurf.Vertex(Vert) = surfTemp.Vertex(NVert);
                --NVert;
            }
            if (newSurf.Sides > 2) {
                Vectors::CreateNewellAreaVector(newSurf.Vertex, newSurf.Sides, newSurf.NewellAreaVector);
                newSurf.GrossArea = Vectors::VecLength(newSurf.NewellAreaVector);
                newSurf.Area = newSurf.GrossArea;
                newSurf.NetAreaShadowCalc = newSurf.Area;
                Vectors::CreateNewellSurfaceNormalVector(newSurf.Vertex, newSurf.Sides, newSurf.NewellSurfaceNormalVector);
                Vectors::DetermineAzimuthAndTilt(
                    newSurf.Vertex, SurfWorldAz, SurfTilt, newSurf.lcsx, newSurf.lcsy, newSurf.lcsz, newSurf.NewellSurfaceNormalVector);
                newSurf.Azimuth = SurfWorldAz;
                newSurf.Tilt = SurfTilt;
                newSurf.convOrientation = Convect::GetSurfConvOrientation(newSurf.Tilt);

                // Sine and cosine of azimuth and tilt
                newSurf.SinAzim = std::sin(SurfWorldAz * Constant::DegToRad);
                newSurf.CosAzim = std::cos(SurfWorldAz * Constant::DegToRad);
                newSurf.SinTilt = std::sin(SurfTilt * Constant::DegToRad);
                newSurf.CosTilt = std::cos(SurfTilt * Constant::DegToRad);
                // Outward normal unit vector (pointing away from room)
                newSurf.OutNormVec = newSurf.NewellSurfaceNormalVector;
                for (int n = 1; n <= 3; ++n) {
                    if (std::abs(newSurf.OutNormVec(n) - 1.0) < 1.e-06) {
                        newSurf.OutNormVec(n) = +1.0;
                    }
                    if (std::abs(newSurf.OutNormVec(n) + 1.0) < 1.e-06) {
                        newSurf.OutNormVec(n) = -1.0;
                    }
                    if (std::abs(newSurf.OutNormVec(n)) < 1.e-06) {
                        newSurf.OutNormVec(n) = 0.0;
                    }
                }

                // Can perform tests on this surface here
                newSurf.ViewFactorSky = 0.5 * (1.0 + newSurf.CosTilt);
                newSurf.ViewFactorGround = 0.5 * (1.0 - newSurf.CosTilt);

                // The following IR view factors are modified in subr. SkyDifSolarShading if there are shadowing
                // surfaces
                newSurf.ViewFactorSkyIR = newSurf.ViewFactorSky;
                newSurf.ViewFactorGroundIR = 0.5 * (1.0 - newSurf.CosTilt);
            }

            // Change Name
            newSurf.Name = "iz-" + surfTemp.Name;
            // Debug   write(outputfiledebug,*) ' new surf name=',TRIM(SurfaceTmp(CurNewSurf)%Name)
            // Debug   write(outputfiledebug,*) ' new surf in zone=',TRIM(surfacetmp(curnewsurf)%zoneName)
            newSurf.ExtBoundCond = unreconciledZoneSurface;
            surfTemp.ExtBoundCond = unreconciledZoneSurface;
            newSurf.ExtBoundCondName = surfTemp.Name;
            surfTemp.ExtBoundCondName = newSurf.Name;
            if (newSurf.Class == SurfaceClass::Roof || newSurf.Class == SurfaceClass::Wall || newSurf.Class == SurfaceClass::Floor) {
                // base surface
                if (surfTemp.Class == SurfaceClass::Roof) {
                    newSurf.Class = SurfaceClass::Floor;
                    // Debug          write(outputfiledebug,*) ' new surfaces is a floor'
                } else if (surfTemp.Class == SurfaceClass::Floor) {
                    newSurf.Class = SurfaceClass::Roof;
                    // Debug          write(outputfiledebug,*) ' new surfaces is a roof'
                }
                newSurf.BaseSurf = CurNewSurf;
                newSurf.BaseSurfName = newSurf.Name;
                // Debug        write(outputfiledebug,*) ' basesurf, extboundcondname=',TRIM(SurfaceTmp(CurNewSurf)%ExtBoundCondName)
            } else {
                // subsurface
                int Found =
                    Util::FindItemInList("iz-" + surfTemp.BaseSurfName, state.dataSurfaceGeometry->SurfaceTmp, FirstTotalSurfaces + CurNewSurf - 1);
                if (Found > 0) {
                    newSurf.BaseSurfName = "iz-" + surfTemp.BaseSurfName;
                    newSurf.BaseSurf = Found;
                    auto &foundBaseSurf = state.dataSurfaceGeometry->SurfaceTmp(Found);
                    foundBaseSurf.Area -= newSurf.Area;
                    if (newSurf.Class == SurfaceClass::Window || newSurf.Class == SurfaceClass::GlassDoor) {
                        foundBaseSurf.NetAreaShadowCalc -= newSurf.Area / newSurf.Multiplier;
                    } else { // Door, TDD:Diffuser, TDD:DOME
                        foundBaseSurf.NetAreaShadowCalc -= newSurf.Area;
                    }
                    newSurf.ExtBoundCond = foundBaseSurf.ExtBoundCond;
                    newSurf.ExtBoundCondName = surfTemp.Name;
                    newSurf.ExtSolar = foundBaseSurf.ExtSolar;
                    newSurf.ExtWind = foundBaseSurf.ExtWind;
                    newSurf.Zone = foundBaseSurf.Zone;
                    newSurf.ZoneName = foundBaseSurf.ZoneName;
                    newSurf.spaceNum = foundBaseSurf.spaceNum;
                    newSurf.OSCPtr = foundBaseSurf.OSCPtr;
                    // Debug        write(outputfiledebug,*) ' subsurf, extboundcondname=',TRIM(SurfaceTmp(CurNewSurf)%ExtBoundCondName)
                    // Debug        write(outputfiledebug,*) ' subsurf, basesurf=',TRIM('iz-'//SurfaceTmp(SurfNum)%BaseSurfName)
                } else {
                    ShowSevereError(state,
                                    format("{}Adding unentered subsurface, could not find base surface=iz-{}", RoutineName, surfTemp.BaseSurfName));
                    SurfError = true;
                }
            }
        }
        //**********************************************************************************
        // After all of the surfaces have been defined then the base surfaces for the
        // sub-surfaces can be defined.  Loop through surfaces and match with the sub-surface
        // names.
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
            if (!surfTemp.HeatTransSurf) {
                continue;
            }

            int Found = 0;
            // why are we doing this again?  this should have already been done.
            if (Util::SameString(surfTemp.BaseSurfName, surfTemp.Name)) {
                Found = SurfNum;
            } else {
                Found = Util::FindItemInList(surfTemp.BaseSurfName, state.dataSurfaceGeometry->SurfaceTmp, state.dataSurface->TotSurfaces);
            }
            if (Found > 0) {
                surfTemp.BaseSurf = Found;
                if (SurfNum != Found) { // for subsurfaces
                    if (surfTemp.HeatTransSurf) {
                        ++state.dataSurfaceGeometry->SurfaceTmp(Found).NumSubSurfaces;
                    }
                    if (surfTemp.Class < SurfaceClass::Window || surfTemp.Class > SurfaceClass::TDD_Diffuser) {
                        if (surfTemp.Class == SurfaceClass::None) {
                            ShowSevereError(state, format("{}Invalid SubSurface detected, Surface={}", RoutineName, surfTemp.Name));
                        } else {
                            ShowSevereError(state,
                                            format("{}Invalid SubSurface detected, Surface={}, class={} invalid class for subsurface",
                                                   RoutineName,
                                                   surfTemp.Name,
                                                   state.dataSurfaceGeometry->BaseSurfCls(int(surfTemp.Class))));
                            SurfError = true;
                        }
                    }
                }
            }

        } // ...end of the Surface DO loop for finding BaseSurf
        //**********************************************************************************
        // The surfaces need to be hierarchical by space.  Input is allowed to be in any order.  In
        // this section the surfaces are reordered into:
        //    All shadowing surfaces (if mirrored, Mir- surface follows immediately after original)
        //      Shading:Site
        //      Shading:Building
        //      Shading:space (and variants)
        //    For each space:
        //      Walls
        //      Floors
        //      Roofs/Ceilings
        //      Internal Mass
        //      Non-Window subsurfaces (including doors)
        //      Window subsurfaces (including TubularDaylightingDiffusers)
        //      TubularDaylightingDomes
        //    After reordering, MovedSurfs should equal TotSurfaces

        // For reporting purposes, the legacy surface order is also saved in DataSurfaces::AllSurfaceListReportOrder:
        //    All shadowing surfaces (if mirrored, Mir- surface follows immediately after original)
        //      Shading:Site
        //      Shading:Building
        //      Shading:Zone (and variants)
        //    For each zone:
        //      Walls
        //        subsurfaces for each wall (windows, doors, in input order, not sorted) follow the base surface
        //      Floors
        //        subsurfaces for each floor (windows, doors, in input order, not sorted) follow the base surface
        //      Roofs/Ceilings
        //        subsurfaces for each roof/ceiling (windows, doors, in input order, not sorted) follow the base surface
        //      Internal Mass
        //    After reordering, MovedSurfs should equal TotSurfaces

        MovedSurfs = 0;
        Array1D<bool> SurfaceTmpClassMoved; // Tmp class is moved
        SurfaceTmpClassMoved.dimension(state.dataSurface->TotSurfaces, false);
        state.dataSurface->AllSurfaceListReportOrder.reserve(state.dataSurface->TotSurfaces);

        CreateMissingSpaces(state, state.dataSurfaceGeometry->SurfaceTmp);

        // Old SurfNum to New SurfNum
        // Old = order in state.dataSurfaceGeometry->SurfaceTmp
        // New = order in state.dataSurface->Surface
        EPVector<int> oldToNewSurfNums;
        oldToNewSurfNums.resize(state.dataSurface->TotSurfaces, -1);

        // Move all shading Surfaces to Front
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
            if (surfTemp.Class != SurfaceClass::Detached_F && surfTemp.Class != SurfaceClass::Detached_B && surfTemp.Class != SurfaceClass::Shading) {
                continue;
            }

            //  A shading surface
            ++MovedSurfs;
            // Store list of moved surface numbers in reporting order
            state.dataSurface->Surface(MovedSurfs) = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
            SurfaceTmpClassMoved(SurfNum) = true; //'Moved'
            state.dataSurface->AllSurfaceListReportOrder.push_back(SurfNum);
            oldToNewSurfNums(SurfNum) = MovedSurfs;
        }

        //  For each zone

        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
                // Group air boundary surfaces first within each space
                for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                    auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
                    if (SurfaceTmpClassMoved(SurfNum)) {
                        continue;
                    }
                    if (surfTemp.spaceNum != spaceNum) {
                        continue;
                    }
                    int constNum = surfTemp.Construction;
                    if (constNum == 0) {
                        continue;
                    }
                    if (!state.dataConstruction->Construct(constNum).TypeIsAirBoundary) {
                        continue;
                    }

                    //  An air boundary surface
                    surfTemp.IsAirBoundarySurf = true;
                    ++MovedSurfs;
                    state.dataSurface->Surface(MovedSurfs) = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
                    //  If base Surface Type (Wall, Floor, Roof/Ceiling)
                    if ((surfTemp.Class == state.dataSurfaceGeometry->BaseSurfIDs(1)) ||
                        (surfTemp.Class == state.dataSurfaceGeometry->BaseSurfIDs(2)) ||
                        (surfTemp.Class == state.dataSurfaceGeometry->BaseSurfIDs(3))) {
                        // Store list of moved surface numbers in reporting order. We use the old position, we'll reconcile later
                        // We don't do it for Air Door/Air Windows yet, we want them listed below each base surf they belong to
                        state.dataSurface->AllSurfaceListReportOrder.push_back(SurfNum);
                    }
                    oldToNewSurfNums(SurfNum) = MovedSurfs;
                    SurfaceTmpClassMoved(SurfNum) = true; //'Moved'
                }

                //  For each Base Surface Type (Wall, Floor, Roof/Ceiling) - put these first

                for (const DataSurfaces::SurfaceClass Loop : state.dataSurfaceGeometry->BaseSurfIDs) {

                    for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                        auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);

                        if (SurfaceTmpClassMoved(SurfNum)) {
                            continue;
                        }
                        if (surfTemp.Zone == 0) {
                            continue;
                        }

                        if (surfTemp.spaceNum != spaceNum) {
                            continue;
                        }
                        if (surfTemp.Class != Loop) {
                            continue;
                        }

                        ++MovedSurfs;
                        state.dataSurface->Surface(MovedSurfs) = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
                        oldToNewSurfNums(SurfNum) = MovedSurfs;
                        SurfaceTmpClassMoved(SurfNum) = true; // 'Moved'
                        // Store list of moved surface numbers in order reporting order (subsurfaces follow their base surface)
                        state.dataSurface->AllSurfaceListReportOrder.push_back(SurfNum);

                        //  Find all subsurfaces to this surface - just to update Report them in order
                        for (int SubSurfNum = 1; SubSurfNum <= state.dataSurface->TotSurfaces; ++SubSurfNum) {
                            // Gotta avoid pushing myself again!
                            if (SubSurfNum == SurfNum) {
                                continue;
                            }
                            // We don't check if already moved, because we didn't add them to AllSurfaceListReportOrder above!
                            if (state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).Zone == 0) {
                                continue;
                            }
                            if (state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).BaseSurf != SurfNum) {
                                continue;
                            }
                            // Add original sub-surface numbers as placeholders in surface list for reporting
                            state.dataSurface->AllSurfaceListReportOrder.push_back(SubSurfNum);
                        }
                    }
                }

                // Internal mass goes next
                for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                    if (SurfaceTmpClassMoved(SurfNum)) {
                        continue;
                    }

                    auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
                    if (surfTemp.spaceNum != spaceNum) {
                        continue;
                    }
                    if (surfTemp.Class != SurfaceClass::IntMass) {
                        continue;
                    }
                    ++MovedSurfs;
                    state.dataSurface->Surface(MovedSurfs) = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
                    oldToNewSurfNums(SurfNum) = MovedSurfs;
                    SurfaceTmpClassMoved(SurfNum) = true; // 'Moved'
                    // Store list of moved surface numbers in reporting order
                    state.dataSurface->AllSurfaceListReportOrder.push_back(SurfNum);
                }

                // Opaque door goes next
                for (int SubSurfNum = 1; SubSurfNum <= state.dataSurface->TotSurfaces; ++SubSurfNum) {

                    if (SurfaceTmpClassMoved(SubSurfNum)) {
                        continue;
                    }
                    if (state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).spaceNum != spaceNum) {
                        continue;
                    }
                    if (state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).Class != SurfaceClass::Door) {
                        continue;
                    }

                    ++MovedSurfs;
                    state.dataSurface->Surface(MovedSurfs) = state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum);
                    oldToNewSurfNums(SubSurfNum) = MovedSurfs;
                    SurfaceTmpClassMoved(SubSurfNum) = true; // 'Moved'
                }

                // The exterior window subsurfaces (includes SurfaceClass::Window and SurfaceClass::GlassDoor) goes next
                for (int SubSurfNum = 1; SubSurfNum <= state.dataSurface->TotSurfaces; ++SubSurfNum) {

                    if (SurfaceTmpClassMoved(SubSurfNum)) {
                        continue;
                    }
                    if (state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).spaceNum != spaceNum) {
                        continue;
                    }
                    if (state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).ExtBoundCond > 0) {
                        continue; // Exterior window
                    }
                    if ((state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).Class != SurfaceClass::Window) &&
                        (state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).Class != SurfaceClass::GlassDoor)) {
                        continue;
                    }

                    ++MovedSurfs;
                    state.dataSurface->Surface(MovedSurfs) = state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum);
                    oldToNewSurfNums(SubSurfNum) = MovedSurfs;
                    SurfaceTmpClassMoved(SubSurfNum) = true; // 'Moved'
                }

                // The interior window subsurfaces (includes SurfaceClass::Window and SurfaceClass::GlassDoor) goes next
                for (int SubSurfNum = 1; SubSurfNum <= state.dataSurface->TotSurfaces; ++SubSurfNum) {

                    if (SurfaceTmpClassMoved(SubSurfNum)) {
                        continue;
                    }
                    if (state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).spaceNum != spaceNum) {
                        continue;
                    }
                    if (state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).ExtBoundCond <= 0) {
                        continue;
                    }
                    if ((state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).Class != SurfaceClass::Window) &&
                        (state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).Class != SurfaceClass::GlassDoor)) {
                        continue;
                    }

                    ++MovedSurfs;
                    state.dataSurface->Surface(MovedSurfs) = state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum);
                    oldToNewSurfNums(SubSurfNum) = MovedSurfs;
                    SurfaceTmpClassMoved(SubSurfNum) = true; // 'Moved'
                }

                // The SurfaceClass::TDD_Diffuser (OriginalClass = Window) goes next
                for (int SubSurfNum = 1; SubSurfNum <= state.dataSurface->TotSurfaces; ++SubSurfNum) {

                    if (SurfaceTmpClassMoved(SubSurfNum)) {
                        continue;
                    }
                    if (state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).spaceNum != spaceNum) {
                        continue;
                    }
                    if (state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).Class != SurfaceClass::TDD_Diffuser) {
                        continue;
                    }

                    ++MovedSurfs;
                    state.dataSurface->Surface(MovedSurfs) = state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum);
                    oldToNewSurfNums(SubSurfNum) = MovedSurfs;
                    SurfaceTmpClassMoved(SubSurfNum) = true; // 'Moved'
                }

                // Last but not least, SurfaceClass::TDD_Dome
                for (int SubSurfNum = 1; SubSurfNum <= state.dataSurface->TotSurfaces; ++SubSurfNum) {

                    if (SurfaceTmpClassMoved(SubSurfNum)) {
                        continue;
                    }
                    if (state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).spaceNum != spaceNum) {
                        continue;
                    }
                    if (state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum).Class != SurfaceClass::TDD_Dome) {
                        continue;
                    }

                    ++MovedSurfs;
                    state.dataSurface->Surface(MovedSurfs) = state.dataSurfaceGeometry->SurfaceTmp(SubSurfNum);
                    oldToNewSurfNums(SubSurfNum) = MovedSurfs;
                    SurfaceTmpClassMoved(SubSurfNum) = true; // 'Moved'
                }
            }
        }

        // Validity checking
        assert(state.dataSurface->TotSurfaces == MovedSurfs);
        assert(state.dataSurface->TotSurfaces == static_cast<int>(state.dataSurface->AllSurfaceListReportOrder.size()));
        assert(state.dataSurface->TotSurfaces == static_cast<int>(oldToNewSurfNums.size()));

        // Assert validity of indices
        assert(std::find_if(state.dataSurface->AllSurfaceListReportOrder.cbegin(), state.dataSurface->AllSurfaceListReportOrder.cend(), [](int i) {
                   return i < 1;
               }) == state.dataSurface->AllSurfaceListReportOrder.cend());

        assert(std::find_if(oldToNewSurfNums.cbegin(), oldToNewSurfNums.cend(), [](int i) { return i < 1; }) == oldToNewSurfNums.cend());

        if (MovedSurfs != state.dataSurface->TotSurfaces) {
            ShowSevereError(
                state,
                format("{}Reordered # of Surfaces ({}) not = Total # of Surfaces ({})", RoutineName, MovedSurfs, state.dataSurface->TotSurfaces));
            SurfError = true;
            for (int Loop = 1; Loop <= state.dataSurface->TotSurfaces; ++Loop) {
                if (!SurfaceTmpClassMoved(Loop) && state.dataSurfaceGeometry->SurfaceTmp(Loop).Class == SurfaceClass::Invalid) {
                    ShowSevereError(state,
                                    format("{}Error in Surface= \"{} indicated Zone=\"{}\"",
                                           RoutineName,
                                           state.dataSurfaceGeometry->SurfaceTmp(Loop).Name,
                                           state.dataSurfaceGeometry->SurfaceTmp(Loop).ZoneName));
                }
            }
            ShowWarningError(
                state, format("{}Remaining surface checks will use \"reordered number of surfaces\", not number of original surfaces", RoutineName));
        }

        // Realign the relationship: surface to base surface
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            auto &movedSurf = state.dataSurface->Surface(SurfNum);
            if (movedSurf.BaseSurf > 0) {
                int newBaseSurfNum = oldToNewSurfNums(movedSurf.BaseSurf);
                movedSurf.BaseSurf = newBaseSurfNum;

                if (newBaseSurfNum < 1) {
                    ShowFatalError(
                        state,
                        format("{}Couldn't find the new Surface Number for surface index {} named '{}'. Looking for BaseSurf old index of {}",
                               RoutineName,
                               SurfNum,
                               movedSurf.Name,
                               movedSurf.BaseSurf));
                }
            }
            auto &reportOrderNum = state.dataSurface->AllSurfaceListReportOrder[SurfNum - 1];
            if (reportOrderNum > 0) {
                int newReportOrderNum = oldToNewSurfNums(reportOrderNum);
                reportOrderNum = newReportOrderNum;
            }
        }

        state.dataSurfaceGeometry->SurfaceTmp.deallocate(); // DeAllocate the Temp Surface derived type

        createSpaceSurfaceLists(state);

        //  For each Base Surface Type (Wall, Floor, Roof)

        for (const DataSurfaces::SurfaceClass Loop : state.dataSurfaceGeometry->BaseSurfIDs) {
            for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {

                if (state.dataSurface->Surface(SurfNum).Zone == 0) {
                    continue;
                }

                if (state.dataSurface->Surface(SurfNum).Class != Loop) {
                    continue;
                }

                //  Find all subsurfaces to this surface
                for (int SubSurfNum = 1; SubSurfNum <= state.dataSurface->TotSurfaces; ++SubSurfNum) {

                    if (SurfNum == SubSurfNum) {
                        continue;
                    }
                    if (state.dataSurface->Surface(SubSurfNum).Zone == 0) {
                        continue;
                    }
                    if (state.dataSurface->Surface(SubSurfNum).BaseSurf != SurfNum) {
                        continue;
                    }

                    // Check facing angle of Sub compared to base
                    checkSubSurfAzTiltNorm(state, state.dataSurface->Surface(SurfNum), state.dataSurface->Surface(SubSurfNum), subSurfaceError);
                    if (subSurfaceError) {
                        SurfError = true;
                    }
                }
            }
        }

        //**********************************************************************************
        // Now, match up interzone surfaces
        NonMatch = false;
        izConstDiffMsg = false;
        for (int SurfNum = 1; SurfNum <= MovedSurfs; ++SurfNum) { // TotSurfaces
            //  Clean up Shading Surfaces, make sure they don't go through here.
            if (!state.dataSurface->Surface(SurfNum).HeatTransSurf) {
                continue;
            }
            //   If other surface, match it up
            //  Both interzone and "internal" surfaces have this pointer set
            //  Internal surfaces point to themselves, Interzone to another
            if (state.dataSurface->Surface(SurfNum).ExtBoundCond == unreconciledZoneSurface) {
                if (not_blank(state.dataSurface->Surface(SurfNum).ExtBoundCondName)) {
                    int Found = 0;
                    if (state.dataSurface->Surface(SurfNum).ExtBoundCondName == state.dataSurface->Surface(SurfNum).Name) {
                        Found = SurfNum;
                    } else {
                        Found = Util::FindItemInList(state.dataSurface->Surface(SurfNum).ExtBoundCondName, state.dataSurface->Surface, MovedSurfs);
                    }
                    if (Found != 0) {
                        state.dataSurface->Surface(SurfNum).ExtBoundCond = Found;
                        // Check that matching surface is also "OtherZoneSurface"
                        if (state.dataSurface->Surface(Found).ExtBoundCond <= 0 &&
                            state.dataSurface->Surface(Found).ExtBoundCond != unreconciledZoneSurface) {
                            ShowSevereError(state, format("{}Potential \"OtherZoneSurface\" is not matched correctly:", RoutineName));

                            ShowContinueError(state,
                                              format("Surface={}, Zone={}",
                                                     state.dataSurface->Surface(SurfNum).Name,
                                                     state.dataSurface->Surface(SurfNum).ZoneName));
                            ShowContinueError(state,
                                              format("Nonmatched Other/InterZone Surface={}, Zone={}",
                                                     state.dataSurface->Surface(Found).Name,
                                                     state.dataSurface->Surface(Found).ZoneName));
                            SurfError = true;
                        }
                        // Check that matching interzone surface has construction with reversed layers
                        if (Found != SurfNum) { // Interzone surface
                            // Make sure different zones too (CR 4110)
                            if (state.dataSurface->Surface(SurfNum).spaceNum == state.dataSurface->Surface(Found).spaceNum) {
                                ++state.dataSurfaceGeometry->ErrCount2;
                                if (state.dataSurfaceGeometry->ErrCount2 == 1 && !state.dataGlobal->DisplayExtraWarnings) {
                                    ShowWarningError(state,
                                                     format("{}CAUTION -- Interspace surfaces are occurring in the same space(s).", RoutineName));
                                    ShowContinueError(
                                        state, "...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual occurrences.");
                                }
                                if (state.dataGlobal->DisplayExtraWarnings) {
                                    ShowWarningError(state, format("{}CAUTION -- Interspace surfaces are usually in different spaces", RoutineName));
                                    ShowContinueError(state,
                                                      format("Surface={}, Space={}, Zone={}",
                                                             state.dataSurface->Surface(SurfNum).Name,
                                                             state.dataHeatBal->space(state.dataSurface->Surface(SurfNum).spaceNum).Name,
                                                             state.dataSurface->Surface(SurfNum).ZoneName));
                                    ShowContinueError(state,
                                                      format("Surface={}, Space={}, Zone={}",
                                                             state.dataSurface->Surface(Found).Name,
                                                             state.dataHeatBal->space(state.dataSurface->Surface(Found).spaceNum).Name,
                                                             state.dataSurface->Surface(Found).ZoneName));
                                }
                            }
                            int ConstrNum = state.dataSurface->Surface(SurfNum).Construction;
                            int ConstrNumFound = state.dataSurface->Surface(Found).Construction;
                            if (ConstrNum <= 0 || ConstrNumFound <= 0) {
                                continue;
                            }
                            if (state.dataConstruction->Construct(ConstrNum).ReverseConstructionNumLayersWarning &&
                                state.dataConstruction->Construct(ConstrNumFound).ReverseConstructionNumLayersWarning) {
                                continue;
                            }
                            if (state.dataConstruction->Construct(ConstrNum).ReverseConstructionLayersOrderWarning &&
                                state.dataConstruction->Construct(ConstrNumFound).ReverseConstructionLayersOrderWarning) {
                                continue;
                            }
                            TotLay = state.dataConstruction->Construct(ConstrNum).TotLayers;
                            TotLayFound = state.dataConstruction->Construct(ConstrNumFound).TotLayers;
                            if (TotLay != TotLayFound) { // Different number of layers
                                // match on like Uvalues (nominal)
                                if (std::abs(state.dataHeatBal->NominalU(ConstrNum) - state.dataHeatBal->NominalU(ConstrNumFound)) > 0.001) {
                                    ShowSevereError(state,
                                                    format("{}Construction {} of interzone surface {} does not have the same number of layers as the "
                                                           "construction {} of adjacent surface {}",
                                                           RoutineName,
                                                           state.dataConstruction->Construct(ConstrNum).Name,
                                                           state.dataSurface->Surface(SurfNum).Name,
                                                           state.dataConstruction->Construct(ConstrNumFound).Name,
                                                           state.dataSurface->Surface(Found).Name));
                                    if (!state.dataConstruction->Construct(ConstrNum).ReverseConstructionNumLayersWarning ||
                                        !state.dataConstruction->Construct(ConstrNumFound).ReverseConstructionNumLayersWarning) {
                                        ShowContinueError(state, "...this problem for this pair will not be reported again.");
                                        state.dataConstruction->Construct(ConstrNum).ReverseConstructionNumLayersWarning = true;
                                        state.dataConstruction->Construct(ConstrNumFound).ReverseConstructionNumLayersWarning = true;
                                    }
                                    SurfError = true;
                                }
                            } else { // Same number of layers; check for reverse layers
                                // check layers as number of layers is the same
                                izConstDiff = false;
                                // ok if same nominal U
                                CheckForReversedLayers(state, izConstDiff, ConstrNum, ConstrNumFound, TotLay);
                                if (izConstDiff &&
                                    std::abs(state.dataHeatBal->NominalU(ConstrNum) - state.dataHeatBal->NominalU(ConstrNumFound)) > 0.001) {
                                    ShowSevereError(state,
                                                    format("{}Construction {} of interzone surface {} does not have the same materials in the "
                                                           "reverse order as the construction {} of adjacent surface {}",
                                                           RoutineName,
                                                           state.dataConstruction->Construct(ConstrNum).Name,
                                                           state.dataSurface->Surface(SurfNum).Name,
                                                           state.dataConstruction->Construct(ConstrNumFound).Name,
                                                           state.dataSurface->Surface(Found).Name));
                                    ShowContinueError(state,
                                                      "or the properties of the reversed layers are not correct due to differing layer front and "
                                                      "back side values");
                                    if (!state.dataConstruction->Construct(ConstrNum).ReverseConstructionLayersOrderWarning ||
                                        !state.dataConstruction->Construct(ConstrNumFound).ReverseConstructionLayersOrderWarning) {
                                        ShowContinueError(state, "...this problem for this pair will not be reported again.");
                                        state.dataConstruction->Construct(ConstrNum).ReverseConstructionLayersOrderWarning = true;
                                        state.dataConstruction->Construct(ConstrNumFound).ReverseConstructionLayersOrderWarning = true;
                                    }
                                    SurfError = true;
                                } else if (izConstDiff) {
                                    ShowWarningError(state,
                                                     format("{}Construction {} of interzone surface {} does not have the same materials in the "
                                                            "reverse order as the construction {} of adjacent surface {}",
                                                            RoutineName,
                                                            state.dataConstruction->Construct(ConstrNum).Name,
                                                            state.dataSurface->Surface(SurfNum).Name,
                                                            state.dataConstruction->Construct(ConstrNumFound).Name,
                                                            state.dataSurface->Surface(Found).Name));
                                    ShowContinueError(state,
                                                      "or the properties of the reversed layers are not correct due to differing layer front and "
                                                      "back side values");
                                    ShowContinueError(
                                        state,
                                        format("...but Nominal U values are similar, diff=[{:.4R}] ... simulation proceeds.",
                                               std::abs(state.dataHeatBal->NominalU(ConstrNum) - state.dataHeatBal->NominalU(ConstrNumFound))));
                                    if (!izConstDiffMsg) {
                                        ShowContinueError(state,
                                                          "...if the two zones are expected to have significantly different temperatures, the proper "
                                                          "\"reverse\" construction should be created.");
                                        izConstDiffMsg = true;
                                    }
                                    if (!state.dataConstruction->Construct(ConstrNum).ReverseConstructionLayersOrderWarning ||
                                        !state.dataConstruction->Construct(ConstrNumFound).ReverseConstructionLayersOrderWarning) {
                                        ShowContinueError(state, "...this problem for this pair will not be reported again.");
                                        state.dataConstruction->Construct(ConstrNum).ReverseConstructionLayersOrderWarning = true;
                                        state.dataConstruction->Construct(ConstrNumFound).ReverseConstructionLayersOrderWarning = true;
                                    }
                                }
                            }

                            // If significantly different areas -- this would not be good
                            MultFound = state.dataHeatBal->Zone(state.dataSurface->Surface(Found).Zone).Multiplier *
                                        state.dataHeatBal->Zone(state.dataSurface->Surface(Found).Zone).ListMultiplier;
                            MultSurfNum = state.dataHeatBal->Zone(state.dataSurface->Surface(SurfNum).Zone).Multiplier *
                                          state.dataHeatBal->Zone(state.dataSurface->Surface(SurfNum).Zone).ListMultiplier;
                            if (state.dataSurface->Surface(Found).Area > 0.0) {
                                if (std::abs((state.dataSurface->Surface(Found).Area * MultFound -
                                              state.dataSurface->Surface(SurfNum).Area * MultSurfNum) /
                                             state.dataSurface->Surface(Found).Area * MultFound) > 0.02) { // 2% difference in areas
                                    ++state.dataSurfaceGeometry->ErrCount4;
                                    if (state.dataSurfaceGeometry->ErrCount4 == 1 && !state.dataGlobal->DisplayExtraWarnings) {
                                        ShowWarningError(
                                            state,
                                            format("{}InterZone Surface Areas do not match as expected and might not satisfy conservation of energy:",
                                                   RoutineName));
                                        ShowContinueError(
                                            state, "...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual mismatches.");
                                    }
                                    if (state.dataGlobal->DisplayExtraWarnings) {
                                        ShowWarningError(
                                            state,
                                            format("{}InterZone Surface Areas do not match as expected and might not satisfy conservation of energy:",
                                                   RoutineName));

                                        if (MultFound == 1 && MultSurfNum == 1) {
                                            ShowContinueError(state,
                                                              format("  Area={:.1T} in Surface={}, Zone={}",
                                                                     state.dataSurface->Surface(SurfNum).Area,
                                                                     state.dataSurface->Surface(SurfNum).Name,
                                                                     state.dataSurface->Surface(SurfNum).ZoneName));
                                            ShowContinueError(state,
                                                              format("  Area={:.1T} in Surface={}, Zone={}",
                                                                     state.dataSurface->Surface(Found).Area,
                                                                     state.dataSurface->Surface(Found).Name,
                                                                     state.dataSurface->Surface(Found).ZoneName));
                                        } else { // Show multiplier info
                                            ShowContinueError(state,
                                                              format("  Area={:.1T}, Multipliers={}, Total Area={:.1T} in Surface={} Zone={}",
                                                                     state.dataSurface->Surface(SurfNum).Area,
                                                                     MultSurfNum,
                                                                     state.dataSurface->Surface(SurfNum).Area * MultSurfNum,
                                                                     state.dataSurface->Surface(SurfNum).Name,
                                                                     state.dataSurface->Surface(SurfNum).ZoneName));

                                            ShowContinueError(state,
                                                              format("  Area={:.1T}, Multipliers={}, Total Area={:.1T} in Surface={} Zone={}",
                                                                     state.dataSurface->Surface(Found).Area,
                                                                     MultFound,
                                                                     state.dataSurface->Surface(Found).Area * MultFound,
                                                                     state.dataSurface->Surface(Found).Name,
                                                                     state.dataSurface->Surface(Found).ZoneName));
                                        }
                                    }
                                }
                            }
                            // Check opposites Azimuth and Tilt
                            // Tilt
                            if (std::abs(std::abs(state.dataSurface->Surface(Found).Tilt + state.dataSurface->Surface(SurfNum).Tilt) - 180.0) > 1.0) {
                                ShowWarningError(state, format("{}InterZone Surface Tilts do not match as expected.", RoutineName));
                                ShowContinueError(state,
                                                  format("  Tilt={:.1T} in Surface={}, Zone={}",
                                                         state.dataSurface->Surface(SurfNum).Tilt,
                                                         state.dataSurface->Surface(SurfNum).Name,
                                                         state.dataSurface->Surface(SurfNum).ZoneName));
                                ShowContinueError(state,
                                                  format("  Tilt={:.1T} in Surface={}, Zone={}",
                                                         state.dataSurface->Surface(Found).Tilt,
                                                         state.dataSurface->Surface(Found).Name,
                                                         state.dataSurface->Surface(Found).ZoneName));
                            }
                            // check surface class match.  interzone surface.

                            if ((state.dataSurface->Surface(SurfNum).Class == SurfaceClass::Wall &&
                                 state.dataSurface->Surface(Found).Class != SurfaceClass::Wall) ||
                                (state.dataSurface->Surface(SurfNum).Class != SurfaceClass::Wall &&
                                 state.dataSurface->Surface(Found).Class == SurfaceClass::Wall)) {
                                ShowWarningError(state, format("{}InterZone Surface Classes do not match as expected.", RoutineName));
                                ShowContinueError(state,
                                                  format("Surface=\"{}\", surface class={}",
                                                         state.dataSurface->Surface(SurfNum).Name,
                                                         cSurfaceClass(state.dataSurface->Surface(SurfNum).Class)));
                                ShowContinueError(state,
                                                  format("Adjacent Surface=\"{}\", surface class={}",
                                                         state.dataSurface->Surface(Found).Name,
                                                         cSurfaceClass(state.dataSurface->Surface(Found).Class)));
                                ShowContinueError(state, "Other errors/warnings may follow about these surfaces.");
                            }
                            if ((state.dataSurface->Surface(SurfNum).Class == SurfaceClass::Roof &&
                                 state.dataSurface->Surface(Found).Class != SurfaceClass::Floor) ||
                                (state.dataSurface->Surface(SurfNum).Class != SurfaceClass::Roof &&
                                 state.dataSurface->Surface(Found).Class == SurfaceClass::Floor)) {
                                ShowWarningError(state, format("{}InterZone Surface Classes do not match as expected.", RoutineName));
                                ShowContinueError(state,
                                                  format("Surface=\"{}\", surface class={}",
                                                         state.dataSurface->Surface(SurfNum).Name,
                                                         cSurfaceClass(state.dataSurface->Surface(SurfNum).Class)));
                                ShowContinueError(state,
                                                  format("Adjacent Surface=\"{}\", surface class={}",
                                                         state.dataSurface->Surface(Found).Name,
                                                         cSurfaceClass(state.dataSurface->Surface(Found).Class)));
                                ShowContinueError(state, "Other errors/warnings may follow about these surfaces.");
                            }
                            if (state.dataSurface->Surface(SurfNum).Class != SurfaceClass::Roof &&
                                state.dataSurface->Surface(SurfNum).Class != SurfaceClass::Floor) {
                                // Walls, Windows, Doors, Glass Doors
                                if (state.dataSurface->Surface(SurfNum).Class != SurfaceClass::Wall) {
                                    // Surface is a Door, Window or Glass Door
                                    if (state.dataSurface->Surface(SurfNum).BaseSurf == 0) {
                                        continue; // error detected elsewhere
                                    }
                                    if (state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).Class == SurfaceClass::Roof ||
                                        state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).Class == SurfaceClass::Floor) {
                                        continue;
                                    }
                                }
                                if (std::abs(std::abs(state.dataSurface->Surface(SurfNum).Azimuth - state.dataSurface->Surface(Found).Azimuth) -
                                             180.0) > 1.0) {
                                    if (std::abs(state.dataSurface->Surface(SurfNum).SinTilt) > 0.5 || state.dataGlobal->DisplayExtraWarnings) {
                                        // if horizontal surfaces, then these are windows/doors/etc in those items.
                                        ShowWarningError(state, format("{}InterZone Surface Azimuths do not match as expected.", RoutineName));
                                        ShowContinueError(state,
                                                          format("  Azimuth={:.1T}, Tilt={:.1T}, in Surface={}, Zone={}",
                                                                 state.dataSurface->Surface(SurfNum).Azimuth,
                                                                 state.dataSurface->Surface(SurfNum).Tilt,
                                                                 state.dataSurface->Surface(SurfNum).Name,
                                                                 state.dataSurface->Surface(SurfNum).ZoneName));
                                        ShowContinueError(state,
                                                          format("  Azimuth={:.1T}, Tilt={:.1T}, in Surface={}, Zone={}",
                                                                 state.dataSurface->Surface(Found).Azimuth,
                                                                 state.dataSurface->Surface(Found).Tilt,
                                                                 state.dataSurface->Surface(Found).Name,
                                                                 state.dataSurface->Surface(Found).ZoneName));
                                        ShowContinueError(
                                            state,
                                            format("..surface class of first surface={}", cSurfaceClass(state.dataSurface->Surface(SurfNum).Class)));
                                        ShowContinueError(
                                            state,
                                            format("..surface class of second surface={}", cSurfaceClass(state.dataSurface->Surface(Found).Class)));
                                    }
                                }
                            }

                            // Make sure exposures (Sun, Wind) are the same.....and are "not"
                            if (state.dataSurface->Surface(SurfNum).ExtSolar || state.dataSurface->Surface(Found).ExtSolar) {
                                ShowWarningError(state, format("{}Interzone surfaces cannot be \"SunExposed\" -- removing SunExposed", RoutineName));
                                ShowContinueError(state,
                                                  format("  Surface={}, Zone={}",
                                                         state.dataSurface->Surface(SurfNum).Name,
                                                         state.dataSurface->Surface(SurfNum).ZoneName));
                                ShowContinueError(state,
                                                  format("  Surface={}, Zone={}",
                                                         state.dataSurface->Surface(Found).Name,
                                                         state.dataSurface->Surface(Found).ZoneName));
                                state.dataSurface->Surface(SurfNum).ExtSolar = false;
                                state.dataSurface->Surface(Found).ExtSolar = false;
                            }
                            if (state.dataSurface->Surface(SurfNum).ExtWind || state.dataSurface->Surface(Found).ExtWind) {
                                ShowWarningError(state,
                                                 format("{}Interzone surfaces cannot be \"WindExposed\" -- removing WindExposed", RoutineName));
                                ShowContinueError(state,
                                                  format("  Surface={}, Zone={}",
                                                         state.dataSurface->Surface(SurfNum).Name,
                                                         state.dataSurface->Surface(SurfNum).ZoneName));
                                ShowContinueError(state,
                                                  format("  Surface={}, Zone={}",
                                                         state.dataSurface->Surface(Found).Name,
                                                         state.dataSurface->Surface(Found).ZoneName));
                                state.dataSurface->Surface(SurfNum).ExtWind = false;
                                state.dataSurface->Surface(Found).ExtWind = false;
                            }
                        }
                        // Set opposing surface back to this one (regardless of error)
                        state.dataSurface->Surface(Found).ExtBoundCond = SurfNum;
                        // Check subsurfaces...  make sure base surface is also an interzone surface
                        if (state.dataSurface->Surface(SurfNum).BaseSurf != SurfNum) { // Subsurface
                            if ((state.dataSurface->Surface(SurfNum).ExtBoundCond != SurfNum) &&
                                not_blank(state.dataSurface->Surface(SurfNum).ExtBoundCondName)) {
                                // if not internal subsurface
                                if (state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).ExtBoundCond ==
                                    state.dataSurface->Surface(SurfNum).BaseSurf) {
                                    // base surface is not interzone surface
                                    ShowSevereError(state,
                                                    format("{}SubSurface=\"{}\" is an interzone subsurface.",
                                                           RoutineName,
                                                           state.dataSurface->Surface(SurfNum).Name));
                                    ShowContinueError(state,
                                                      format("..but the Base Surface is not an interzone surface, Surface=\"{}\".",
                                                             state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).Name));
                                    SurfError = true;
                                }
                            }
                        }
                    } else {
                        //  Seems unlikely that an internal surface would be missing itself, so this message
                        //  only indicates for adjacent (interzone) surfaces.
                        ShowSevereError(state,
                                        format("{}Adjacent Surface not found: {} adjacent to surface {}",
                                               RoutineName,
                                               state.dataSurface->Surface(SurfNum).ExtBoundCondName,
                                               state.dataSurface->Surface(SurfNum).Name));
                        NonMatch = true;
                        SurfError = true;
                    }
                } else if (state.dataSurface->Surface(SurfNum).BaseSurf != SurfNum) { // Subsurface
                    if (state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).ExtBoundCond > 0 &&
                        state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).ExtBoundCond !=
                            state.dataSurface->Surface(SurfNum).BaseSurf) { // If Interzone surface, subsurface must be also.
                        ShowSevereError(state, format("{}SubSurface on Interzone Surface must be an Interzone SubSurface.", RoutineName));
                        ShowContinueError(state,
                                          format("...OutsideFaceEnvironment is blank, in Surface={}", state.dataSurface->Surface(SurfNum).Name));
                        SurfError = true;
                    } else {
                        ++state.dataSurfaceGeometry->ErrCount3;
                        if (state.dataSurfaceGeometry->ErrCount3 == 1 && !state.dataGlobal->DisplayExtraWarnings) {
                            ShowWarningError(state, format("{}Blank name for Outside Boundary Condition Objects.", RoutineName));
                            ShowContinueError(state, "...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual surfaces.");
                        }
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            ShowWarningError(state,
                                             format("{}Blank name for Outside Boundary Condition Object, in surface={}",
                                                    RoutineName,
                                                    state.dataSurface->Surface(SurfNum).Name));
                            ShowContinueError(state,
                                              format("Resetting this surface to be an internal zone surface, zone={}",
                                                     state.dataSurface->Surface(SurfNum).ZoneName));
                        }
                        state.dataSurface->Surface(SurfNum).ExtBoundCondName = state.dataSurface->Surface(SurfNum).Name;
                        state.dataSurface->Surface(SurfNum).ExtBoundCond = SurfNum;
                    }
                } else {
                    ++state.dataSurfaceGeometry->ErrCount3;
                    if (state.dataSurfaceGeometry->ErrCount3 == 1 && !state.dataGlobal->DisplayExtraWarnings) {
                        ShowSevereError(state, format("{}Blank name for Outside Boundary Condition Objects.", RoutineName));
                        ShowContinueError(state, "...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual surfaces.");
                    }
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        ShowWarningError(state,
                                         format("{}Blank name for Outside Boundary Condition Object, in surface={}",
                                                RoutineName,
                                                state.dataSurface->Surface(SurfNum).Name));
                        ShowContinueError(state,
                                          format("Resetting this surface to be an internal zone (adiabatic) surface, zone={}",
                                                 state.dataSurface->Surface(SurfNum).ZoneName));
                    }
                    state.dataSurface->Surface(SurfNum).ExtBoundCondName = state.dataSurface->Surface(SurfNum).Name;
                    state.dataSurface->Surface(SurfNum).ExtBoundCond = SurfNum;
                    SurfError = true;
                }
            }

        } // ...end of the Surface DO loop for finding BaseSurf
        if (NonMatch) {
            ShowSevereError(state, format("{}Non matching interzone surfaces found", RoutineName));
        }

        //**********************************************************************************
        // Warn about interzone surfaces that have adiabatic windows/vice versa
        SubSurfaceSevereDisplayed = false;
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            if (!state.dataSurface->Surface(SurfNum).HeatTransSurf) {
                continue;
            }
            if (state.dataSurface->Surface(SurfNum).BaseSurf == SurfNum) {
                continue; // base surface
            }
            // not base surface.  Check it.
            if (state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).ExtBoundCond <= 0) { // exterior or other base surface
                if (state.dataSurface->Surface(SurfNum).ExtBoundCond !=
                    state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).ExtBoundCond) { // should match base surface
                    if (state.dataSurface->Surface(SurfNum).ExtBoundCond == SurfNum) {
                        ShowSevereError(
                            state,
                            format("{}Subsurface=\"{}\" exterior condition [adiabatic surface] in a base surface=\"{}\" with exterior condition [{}]",
                                   RoutineName,
                                   state.dataSurface->Surface(SurfNum).Name,
                                   state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).Name,
                                   DataSurfaces::cExtBoundCondition(
                                       state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).ExtBoundCond)));
                        SurfError = true;
                    } else if (state.dataSurface->Surface(SurfNum).ExtBoundCond > 0) {
                        ShowSevereError(
                            state,
                            format("{}Subsurface=\"{}\" exterior condition [interzone surface] in a base surface=\"{}\" with exterior condition [{}]",
                                   RoutineName,
                                   state.dataSurface->Surface(SurfNum).Name,
                                   state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).Name,
                                   DataSurfaces::cExtBoundCondition(
                                       state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).ExtBoundCond)));
                        SurfError = true;
                    } else if (state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).ExtBoundCond ==
                               DataSurfaces::OtherSideCondModeledExt) {
                        ShowWarningError(state,
                                         format("{}Subsurface=\"{}\" exterior condition [{}] in a base surface=\"{}\" with exterior condition [{}]",
                                                RoutineName,
                                                state.dataSurface->Surface(SurfNum).Name,
                                                DataSurfaces::cExtBoundCondition(state.dataSurface->Surface(SurfNum).ExtBoundCond),
                                                state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).Name,
                                                DataSurfaces::cExtBoundCondition(
                                                    state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).ExtBoundCond)));
                        ShowContinueError(state, "...SubSurface will not use the exterior condition model of the base surface.");
                    } else {
                        ShowSevereError(state,
                                        format("{}Subsurface=\"{}\" exterior condition [{}] in a base surface=\"{}\" with exterior condition [{}]",
                                               RoutineName,
                                               state.dataSurface->Surface(SurfNum).Name,
                                               DataSurfaces::cExtBoundCondition(state.dataSurface->Surface(SurfNum).ExtBoundCond),
                                               state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).Name,
                                               DataSurfaces::cExtBoundCondition(
                                                   state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).ExtBoundCond)));
                        SurfError = true;
                    }
                    if (!SubSurfaceSevereDisplayed && SurfError) {
                        ShowContinueError(state, "...calculations for heat balance would be compromised.");
                        SubSurfaceSevereDisplayed = true;
                    }
                }
            } else if (state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).BaseSurf ==
                       state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).ExtBoundCond) {
                // adiabatic surface. make sure subsurfaces match
                if (state.dataSurface->Surface(SurfNum).ExtBoundCond != SurfNum) { // not adiabatic surface
                    if (state.dataSurface->Surface(SurfNum).ExtBoundCond > 0) {
                        ShowSevereError(state,
                                        format("{}Subsurface=\"{}\" exterior condition [interzone surface] in a base surface=\"{}\" with exterior "
                                               "condition [adiabatic surface]",
                                               RoutineName,
                                               state.dataSurface->Surface(SurfNum).Name,
                                               state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).Name));
                    } else {
                        ShowSevereError(
                            state,
                            format("{}Subsurface=\"{}\" exterior condition [{}] in a base surface=\"{}\" with exterior condition [adiabatic surface]",
                                   RoutineName,
                                   state.dataSurface->Surface(SurfNum).Name,
                                   DataSurfaces::cExtBoundCondition(state.dataSurface->Surface(SurfNum).ExtBoundCond),
                                   state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).Name));
                    }
                    if (!SubSurfaceSevereDisplayed) {
                        ShowContinueError(state, "...calculations for heat balance would be compromised.");
                        SubSurfaceSevereDisplayed = true;
                    }
                    SurfError = true;
                }
            } else if (state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).ExtBoundCond > 0) { // interzone surface
                if (state.dataSurface->Surface(SurfNum).ExtBoundCond == SurfNum) {
                    ShowSevereError(state,
                                    format("{}Subsurface=\"{}\" is an adiabatic surface in an Interzone base surface=\"{}\"",
                                           RoutineName,
                                           state.dataSurface->Surface(SurfNum).Name,
                                           state.dataSurface->Surface(state.dataSurface->Surface(SurfNum).BaseSurf).Name));
                    if (!SubSurfaceSevereDisplayed) {
                        ShowContinueError(state, "...calculations for heat balance would be compromised.");
                        SubSurfaceSevereDisplayed = true;
                    }
                    //        SurfError=.TRUE.
                }
            }
        }

        setSurfaceFirstLast(state);

        // Set up Floor Areas for Zones and Spaces
        Real64 constexpr floorAreaTolerance(0.05);
        Real64 constexpr floorAreaPercentTolerance(floorAreaTolerance * 100.0);
        if (!SurfError) {
            int ErrCount = 0;
            for (auto &thisSpace : state.dataHeatBal->space) {
                auto &thisZone = state.dataHeatBal->Zone(thisSpace.zoneNum);
                Real64 calcFloorArea = 0.0; // Calculated floor area used for this space
                for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
                    auto const &thisSurf = state.dataSurface->Surface(SurfNum);
                    if (thisSurf.Class == SurfaceClass::Floor) {
                        thisZone.HasFloor = true;
                        thisSpace.hasFloor = true;
                        calcFloorArea += thisSurf.Area;
                    }
                    if (thisSurf.Class == SurfaceClass::Roof) {
                        thisZone.CeilingArea += thisSurf.Area;
                        thisZone.HasRoof = true;
                    }
                }
                if (thisSpace.userEnteredFloorArea != Constant::AutoCalculate) {
                    // Check entered vs calculated
                    if (thisSpace.userEnteredFloorArea > 0.0) { // User entered Space floor area,
                        // produce message if not near calculated
                        if (calcFloorArea > 0.0) {
                            Real64 diffp = std::abs(calcFloorArea - thisSpace.userEnteredFloorArea) / thisSpace.userEnteredFloorArea;
                            if (diffp > floorAreaTolerance) {
                                ++ErrCount;
                                if (ErrCount == 1 && !state.dataGlobal->DisplayExtraWarnings) {
                                    ShowWarningError(
                                        state,
                                        format("{}Entered Space Floor Area(s) differ more than {:.0R}% from calculated Space Floor Area(s).",
                                               std::string(RoutineName),
                                               floorAreaPercentTolerance));
                                    ShowContinueError(state,
                                                      "...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual Spaces.");
                                }
                                if (state.dataGlobal->DisplayExtraWarnings) {
                                    // Warn user of using specified Space Floor Area
                                    ShowWarningError(
                                        state,
                                        format("{}Entered Floor Area for Space=\"{}\" is {:.1R}% different from the calculated Floor Area.",
                                               std::string(RoutineName),
                                               thisSpace.Name,
                                               diffp * 100.0));
                                    ShowContinueError(state,
                                                      format("Entered Space Floor Area={:.2R}, Calculated Space Floor Area={:.2R}, entered "
                                                             "Floor Area will be used.",
                                                             thisSpace.userEnteredFloorArea,
                                                             calcFloorArea));
                                }
                            }
                        }
                        thisSpace.FloorArea = thisSpace.userEnteredFloorArea;
                        thisSpace.hasFloor = true;
                    }
                } else {
                    thisSpace.FloorArea = calcFloorArea;
                }
            }
            ErrCount = 0;
            for (auto &thisZone : state.dataHeatBal->Zone) {
                // Calculate zone floor area as sum of space floor areas
                Real64 zoneCalcFloorArea = 0.0; // Calculated floor area excluding air boundary surfaces
                for (int spaceNum : thisZone.spaceIndexes) {
                    zoneCalcFloorArea += state.dataHeatBal->space(spaceNum).FloorArea;
                    thisZone.HasFloor |= state.dataHeatBal->space(spaceNum).hasFloor;
                }
                if (thisZone.UserEnteredFloorArea != Constant::AutoCalculate) {
                    // Check entered vs calculated
                    if (thisZone.UserEnteredFloorArea > 0.0) { // User entered zone floor area,
                        // produce message if not near calculated
                        if (zoneCalcFloorArea > 0.0) {
                            Real64 diffp = std::abs(zoneCalcFloorArea - thisZone.UserEnteredFloorArea) / thisZone.UserEnteredFloorArea;
                            if (diffp > 0.05) {
                                ++ErrCount;
                                if (ErrCount == 1 && !state.dataGlobal->DisplayExtraWarnings) {
                                    ShowWarningError(
                                        state,
                                        format("{}Entered Zone Floor Area(s) differ more than {:.0R}% from the sum of the Space Floor Area(s).",
                                               std::string(RoutineName),
                                               floorAreaPercentTolerance));
                                    ShowContinueError(state,
                                                      "...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual zones.");
                                }
                                if (state.dataGlobal->DisplayExtraWarnings) {
                                    // Warn user of using specified Zone Floor Area
                                    ShowWarningError(state,
                                                     format("{}Entered Floor Area for Zone=\"{}\" is {:.1R}% different from the sum of the "
                                                            "Space Floor Area(s).",
                                                            std::string(RoutineName),
                                                            thisZone.Name,
                                                            diffp * 100.0));
                                    ShowContinueError(state,
                                                      format("Entered Zone Floor Area={:.2R}, Sum of Space Floor Area(s)={:.2R}",
                                                             thisZone.UserEnteredFloorArea,
                                                             zoneCalcFloorArea));
                                    ShowContinueError(
                                        state, "Entered Zone Floor Area will be used and Space Floor Area(s) will be adjusted proportionately.");
                                }
                            }
                        }
                        thisZone.FloorArea = thisZone.UserEnteredFloorArea;
                        thisZone.HasFloor = true;

                        // Adjust space floor areas to match zone floor area
                        if (thisZone.numSpaces == 1) {
                            // If the zone contains only one space, then set the Space area to the Zone area
                            int spaceNum = thisZone.spaceIndexes(1);
                            state.dataHeatBal->space(spaceNum).FloorArea = thisZone.FloorArea;
                        } else if (zoneCalcFloorArea > 0.0) {
                            // Adjust space areas proportionately
                            Real64 areaRatio = thisZone.FloorArea / zoneCalcFloorArea;
                            for (int spaceNum : thisZone.spaceIndexes) {
                                state.dataHeatBal->space(spaceNum).FloorArea *= areaRatio;
                            }
                        } else {
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                // Warn if calculated floor area was zero and there is more than one Space
                                ShowWarningError(
                                    state,
                                    format("{}Entered Floor Area entered for Zone=\"{}\" significantly different from sum of Space Floor Areas",
                                           RoutineName,
                                           thisZone.Name));
                                ShowContinueError(state,
                                                  "But the sum of the Space Floor Areas is zero and there is more than one Space in the zone."
                                                  "Unable to apportion the zone floor area. Space Floor Areas are zero.");
                            }
                        }
                    } else {
                        if (zoneCalcFloorArea > 0.0) {
                            thisZone.FloorArea = zoneCalcFloorArea;
                        }
                    }
                } else {
                    thisZone.FloorArea = zoneCalcFloorArea;
                }
                Real64 totSpacesFloorArea = 0.0;
                for (int spaceNum : thisZone.spaceIndexes) {
                    totSpacesFloorArea += state.dataHeatBal->space(spaceNum).FloorArea;
                }
                if (totSpacesFloorArea > 0.0) {
                    for (int spaceNum : thisZone.spaceIndexes) {
                        state.dataHeatBal->space(spaceNum).fracZoneFloorArea = state.dataHeatBal->space(spaceNum).FloorArea / totSpacesFloorArea;
                    }
                } // else leave fractions at zero
            }
        }

        for (int SurfNum = 1; SurfNum <= MovedSurfs; ++SurfNum) { // TotSurfaces
            if (state.dataSurface->Surface(SurfNum).Area < 1.e-06) {
                ShowSevereError(state,
                                format("{}Zero or negative surface area[{:.5R}], Surface={}",
                                       RoutineName,
                                       state.dataSurface->Surface(SurfNum).Area,
                                       state.dataSurface->Surface(SurfNum).Name));
                SurfError = true;
            }
            if (state.dataSurface->Surface(SurfNum).Area >= 1.e-06 && state.dataSurface->Surface(SurfNum).Area < 0.001) {
                ShowWarningError(state,
                                 format("{}Very small surface area[{:.5R}], Surface={}",
                                        RoutineName,
                                        state.dataSurface->Surface(SurfNum).Area,
                                        state.dataSurface->Surface(SurfNum).Name));
            }
        }

        for (int SurfNum = 1; SurfNum <= MovedSurfs; ++SurfNum) { // TotSurfaces
            auto &surf = state.dataSurface->Surface(SurfNum);
            // GLASSDOORs and TDD:DIFFUSERs will be treated as windows in the subsequent heat transfer and daylighting
            // calculations. Reset class to 'Window' after saving the original designation in SurfaceWindow.

            surf.OriginalClass = surf.Class;

            if (surf.Class == SurfaceClass::GlassDoor || surf.Class == SurfaceClass::TDD_Diffuser) {
                surf.Class = SurfaceClass::Window;
            }

            if (surf.Class == SurfaceClass::TDD_Dome) {
                // Reset the TDD:DOME subsurface to act as a base surface that can shade and be shaded
                // NOTE: This must be set early so that subsequent shading calculations are done correctly
                surf.BaseSurf = SurfNum;
            }
        }

        auto &s_mat = state.dataMaterial;

        // I don't think this entire loop matters
        errFlag = false;
        if (!SurfError) {
            for (int SurfNum = 1; SurfNum <= MovedSurfs; ++SurfNum) { // TotSurfaces
                auto &surf = state.dataSurface->Surface(SurfNum);
                if (!surf.HasShadeControl) {
                    continue;
                }

                int ConstrNumSh = surf.activeShadedConstruction;
                if (ConstrNumSh <= 0) {
                    continue;
                }

                auto &winShadeCtrl = state.dataSurface->WindowShadingControl(surf.activeWindowShadingControl);
                if (!ANY_BLIND(winShadeCtrl.ShadingType)) {
                    continue;
                }
                // use first item since others should be identical

                auto &surfShade = state.dataSurface->surfShades(SurfNum);
                // TH 1/7/2010. CR 7930
                // The old code did not consider between-glass blind. Also there should not be two blinds - both interior and exterior
                // Use the new generic code (assuming only one blind) as follows
                for (int iMatNum = 1; iMatNum <= state.dataConstruction->Construct(ConstrNumSh).TotLayers; ++iMatNum) {
                    auto *mat = s_mat->materials(state.dataConstruction->Construct(ConstrNumSh).LayerPoint(iMatNum));

                    if (mat->group != Material::Group::Blind) {
                        continue;
                    }

                    auto *matBlind = dynamic_cast<Material::MaterialBlind *>(mat);
                    assert(matBlind != nullptr);

                    surfShade.blind.matNum = mat->Num;
                    break;
                }

                if (errFlag) {
                    ErrorsFound = true;
                    ShowContinueError(state, format("WindowShadingControl {} has errors, program will terminate.", winShadeCtrl.Name));
                }

                if (winShadeCtrl.slatAngleControl != DataSurfaces::SlatAngleControl::Fixed) {
                    surfShade.blind.movableSlats = true;
                    state.dataSurface->AnyMovableSlat = true;
                    state.dataHeatBalSurf->SurfMovSlatsIndexList.push_back(SurfNum);
                }
            } // End of surface loop

            // final associate fenestration surfaces referenced in WindowShadingControl
            FinalAssociateWindowShadingControlFenestration(state, ErrorsFound);
            CheckWindowShadingControlSimilarForWindow(state, ErrorsFound);
        }

        // Check for zones with not enough surfaces
        for (auto &thisZone : state.dataHeatBal->Zone) {
            int OpaqueHTSurfs = 0;        // Number of floors, walls and roofs in a zone
            int OpaqueHTSurfsWithWin = 0; // Number of floors, walls and roofs with windows in a zone
            int InternalMassSurfs = 0;    // Number of internal mass surfaces in a zone
            int priorBaseSurfNum = 0;

            for (int spaceNum : thisZone.spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                if (thisSpace.HTSurfaceFirst == 0) {
                    continue; // Zone with no surfaces
                }
                for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
                    auto const &thisSurf = state.dataSurface->Surface(SurfNum);
                    if (thisSurf.Class == SurfaceClass::Floor || thisSurf.Class == SurfaceClass::Wall || thisSurf.Class == SurfaceClass::Roof) {
                        ++OpaqueHTSurfs;
                    }
                    if (thisSurf.Class == SurfaceClass::IntMass) {
                        ++InternalMassSurfs;
                    }
                    if (thisSurf.Class == SurfaceClass::Window) {
                        // Count base surface only once for multiple windows on a wall
                        int thisBaseSurfNum = thisSurf.BaseSurf;
                        if (thisBaseSurfNum != priorBaseSurfNum) {
                            ++OpaqueHTSurfsWithWin;
                            priorBaseSurfNum = thisBaseSurfNum;
                        }
                    }
                }
            }
            if (OpaqueHTSurfsWithWin == 1 && OpaqueHTSurfs == 1 && InternalMassSurfs == 0) {
                SurfError = true;
                ShowSevereError(state,
                                format("{}Zone {} has only one floor, wall or roof, and this surface has a window.", RoutineName, thisZone.Name));
                ShowContinueError(state, "Add more floors, walls or roofs, or an internal mass surface.");
            }
        }

        // set up vertex of centroid for each surface.
        CalcSurfaceCentroid(state);

        SetupShadeSurfacesForSolarCalcs(state); // if shading surfaces are solar collectors or PV, then we need full solar calc.

        GetMovableInsulationData(state, ErrorsFound);

        if (state.dataSurface->CalcSolRefl) {
            GetShadingSurfReflectanceData(state, ErrorsFound);
        }

        LayNumOutside = 0;

        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            auto &surf = state.dataSurface->Surface(SurfNum);
            // Check for EcoRoof and only 1 allowed to be used.
            if (surf.Construction > 0) {
                state.dataSurface->SurfExtEcoRoof(SurfNum) = state.dataConstruction->Construct(surf.Construction).TypeIsEcoRoof;
            }
            if (!state.dataSurface->SurfExtEcoRoof(SurfNum)) {
                continue;
            }
            if (LayNumOutside == 0) {
                LayNumOutside = state.dataConstruction->Construct(surf.Construction).LayerPoint(1);
                continue;
            }
            if (LayNumOutside != state.dataConstruction->Construct(surf.Construction).LayerPoint(1)) {
                ShowSevereError(state, format("{}Only one EcoRoof Material is currently allowed for all constructions.", RoutineName));
                ShowContinueError(state, format("... first material={}", s_mat->materials(LayNumOutside)->Name));
                ShowContinueError(state,
                                  format("... conflicting Construction={} uses material={}",
                                         state.dataConstruction->Construct(surf.Construction).Name,
                                         s_mat->materials(state.dataConstruction->Construct(surf.Construction).LayerPoint(1))->Name));
                ErrorsFound = true;
            }
        }

        // Reserve space to avoid excess allocations
        state.dataSurface->AllHTSurfaceList.reserve(state.dataSurface->TotSurfaces);
        state.dataSurface->AllExtSolarSurfaceList.reserve(state.dataSurface->TotSurfaces);
        state.dataSurface->AllShadowPossObstrSurfaceList.reserve(state.dataSurface->TotSurfaces);
        state.dataSurface->AllIZSurfaceList.reserve(state.dataSurface->TotSurfaces);
        state.dataSurface->AllHTNonWindowSurfaceList.reserve(state.dataSurface->TotSurfaces - state.dataSurface->TotWindows);
        state.dataSurface->AllHTWindowSurfaceList.reserve(state.dataSurface->TotWindows);
        state.dataSurface->AllExtSolWindowSurfaceList.reserve(state.dataSurface->TotWindows);
        state.dataSurface->AllExtSolWinWithFrameSurfaceList.reserve(state.dataSurface->TotWindows);
        state.dataSurface->AllHTKivaSurfaceList.reserve(state.dataSurface->TotSurfaces);

        // Set flag that determines whether a surface can be an exterior obstruction
        // Also set associated surfaces for Kiva foundations and build heat transfer surface lists
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            auto &surf = state.dataSurface->Surface(SurfNum);
            surf.IsShadowPossibleObstruction = false;
            if (surf.ExtSolar) {
                // This may include some attached shading surfaces
                state.dataSurface->AllExtSolarSurfaceList.push_back(SurfNum);
            }
            if (surf.HeatTransSurf) {
                // Outside light shelves get tagged later as HeatTransSurf=true but they haven't been processed yet
                state.dataSurface->AllHTSurfaceList.push_back(SurfNum);
                int const zoneNum(surf.Zone);
                auto &surfZone(state.dataHeatBal->Zone(zoneNum));
                surfZone.ZoneHTSurfaceList.push_back(SurfNum);
                // Sort window vs non-window surfaces
                if (surf.Class == DataSurfaces::SurfaceClass::Window) {
                    state.dataSurface->AllHTWindowSurfaceList.push_back(SurfNum);
                    surfZone.ZoneHTWindowSurfaceList.push_back(SurfNum);
                    if (surf.ExtSolar) {
                        state.dataSurface->AllExtSolWindowSurfaceList.push_back(SurfNum);
                        if (surf.FrameDivider > 0) {
                            state.dataSurface->AllExtSolWinWithFrameSurfaceList.push_back(SurfNum);
                        }
                    }
                } else {
                    state.dataSurface->AllHTNonWindowSurfaceList.push_back(SurfNum);
                    surfZone.ZoneHTNonWindowSurfaceList.push_back(SurfNum);
                }
                int const surfExtBoundCond(surf.ExtBoundCond);
                // Build zone and interzone surface lists
                if ((surfExtBoundCond > 0) && (surfExtBoundCond != SurfNum)) {
                    state.dataSurface->AllIZSurfaceList.push_back(SurfNum);
                    surfZone.ZoneIZSurfaceList.push_back(SurfNum);
                    auto &adjZone(state.dataHeatBal->Zone(state.dataSurface->Surface(surfExtBoundCond).Zone));
                    adjZone.ZoneHTSurfaceList.push_back(SurfNum);
                    adjZone.ZoneIZSurfaceList.push_back(SurfNum);
                    // Sort window vs non-window surfaces
                    if (surf.Class == DataSurfaces::SurfaceClass::Window) {
                        adjZone.ZoneHTWindowSurfaceList.push_back(SurfNum);
                    } else {
                        adjZone.ZoneHTNonWindowSurfaceList.push_back(SurfNum);
                    }
                }
            }

            // Exclude non-exterior heat transfer surfaces (but not OtherSideCondModeledExt = -4 CR7640)
            if (surf.HeatTransSurf && surf.ExtBoundCond > 0) {
                continue;
            }
            if (surf.HeatTransSurf && surf.ExtBoundCond == DataSurfaces::Ground) {
                continue;
            }
            if (surf.HeatTransSurf && surf.ExtBoundCond == DataSurfaces::KivaFoundation) {
                state.dataSurface->AllHTKivaSurfaceList.push_back(SurfNum);
                if (!ErrorsFound) {
                    state.dataSurfaceGeometry->kivaManager.foundationInputs[surf.OSCPtr].surfaces.push_back(SurfNum);
                }
                continue;
            }
            if (surf.HeatTransSurf && surf.ExtBoundCond == DataSurfaces::OtherSideCoefNoCalcExt) {
                continue;
            }
            if (surf.HeatTransSurf && surf.ExtBoundCond == DataSurfaces::OtherSideCoefCalcExt) {
                continue;
            }
            // Exclude windows and doors, i.e., consider only their base surfaces as possible obstructions
            if (surf.Class == SurfaceClass::Window || surf.Class == SurfaceClass::Door) {
                continue;
            }
            // Exclude duplicate shading surfaces
            if (surf.MirroredSurf) {
                continue;
            }
            // Exclude air boundary surfaces
            if (surf.IsAirBoundarySurf) {
                continue;
            }

            surf.IsShadowPossibleObstruction = true;
            state.dataSurface->AllShadowPossObstrSurfaceList.push_back(SurfNum);
        } // for (SurfNum)

        // Check for IRT surfaces in invalid places.
        if (std::any_of(state.dataConstruction->Construct.begin(),
                        state.dataConstruction->Construct.end(),
                        [](Construction::ConstructionProps const &e) { return e.TypeIsIRT; })) {
            int iTmp1 = 0;
            for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                auto &surf = state.dataSurface->Surface(SurfNum);
                if (!surf.HeatTransSurf) {
                    continue; // ignore shading surfaces
                }
                if (surf.ExtBoundCond > 0 && surf.ExtBoundCond != SurfNum) {
                    continue; // interzone, not adiabatic surface
                }
                if (!state.dataConstruction->Construct(surf.Construction).TypeIsIRT) {
                    continue;
                }
                if (!state.dataGlobal->DisplayExtraWarnings) {
                    ++iTmp1;
                } else {
                    ShowWarningError(state,
                                     format("{}Surface=\"{}\" uses InfraredTransparent construction in a non-interzone surface. (illegal use)",
                                            RoutineName,
                                            surf.Name));
                }
            }
            if (iTmp1 > 0) {
                ShowWarningError(
                    state,
                    format("{}Surfaces use InfraredTransparent constructions {} in non-interzone surfaces. (illegal use)", RoutineName, iTmp1));
                ShowContinueError(state, "For explicit details on each use, use Output:Diagnostics,DisplayExtraWarnings;");
            }
        }

        // Populate SurfaceFilter lists
        for (int iSurfaceFilter = 1; iSurfaceFilter < static_cast<int>(DataSurfaces::SurfaceFilter::Num); ++iSurfaceFilter) {
            state.dataSurface->SurfaceFilterLists[iSurfaceFilter].reserve(state.dataSurface->TotSurfaces);
        }

        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            auto const &surf = state.dataSurface->Surface(SurfNum);
            if (!surf.HeatTransSurf) {
                continue;
            }
            if (surf.ExtBoundCond > 0) {
                state.dataSurface->SurfaceFilterLists[static_cast<int>(DataSurfaces::SurfaceFilter::AllInteriorSurfaces)].push_back(SurfNum);
                if (state.dataConstruction->Construct(surf.Construction).TypeIsWindow) {
                    state.dataSurface->SurfaceFilterLists[static_cast<int>(DataSurfaces::SurfaceFilter::AllInteriorWindows)].push_back(SurfNum);
                } else if (surf.Class == SurfaceClass::Wall) {
                    state.dataSurface->SurfaceFilterLists[static_cast<int>(DataSurfaces::SurfaceFilter::AllInteriorWalls)].push_back(SurfNum);
                } else if (surf.Class == SurfaceClass::Floor) {
                    state.dataSurface->SurfaceFilterLists[static_cast<int>(DataSurfaces::SurfaceFilter::AllInteriorFloors)].push_back(SurfNum);
                } else if (surf.Class == SurfaceClass::Roof) {
                    state.dataSurface->SurfaceFilterLists[static_cast<int>(DataSurfaces::SurfaceFilter::AllInteriorRoofs)].push_back(SurfNum);
                    state.dataSurface->SurfaceFilterLists[static_cast<int>(DataSurfaces::SurfaceFilter::AllInteriorCeilings)].push_back(SurfNum);
                }
            } else {
                state.dataSurface->SurfaceFilterLists[static_cast<int>(DataSurfaces::SurfaceFilter::AllExteriorSurfaces)].push_back(SurfNum);
                if (state.dataConstruction->Construct(surf.Construction).TypeIsWindow) {
                    state.dataSurface->SurfaceFilterLists[static_cast<int>(DataSurfaces::SurfaceFilter::AllExteriorWindows)].push_back(SurfNum);
                } else if (surf.Class == SurfaceClass::Wall) {
                    state.dataSurface->SurfaceFilterLists[static_cast<int>(DataSurfaces::SurfaceFilter::AllExteriorWalls)].push_back(SurfNum);
                } else if (surf.Class == SurfaceClass::Floor) {
                    state.dataSurface->SurfaceFilterLists[static_cast<int>(DataSurfaces::SurfaceFilter::AllExteriorFloors)].push_back(SurfNum);
                } else if (surf.Class == SurfaceClass::Roof) {
                    state.dataSurface->SurfaceFilterLists[static_cast<int>(DataSurfaces::SurfaceFilter::AllExteriorRoofs)].push_back(SurfNum);
                    state.dataSurface->SurfaceFilterLists[static_cast<int>(DataSurfaces::SurfaceFilter::AllInteriorCeilings)].push_back(SurfNum);
                }
            }
        } // for (SurfNum)

        // Note, could do same for Window Area and detecting if Interzone Surface in Zone

        if (state.dataSurfaceGeometry->Warning1Count > 0) {
            ShowWarningMessage(state,
                               format("{}Window dimensions differ from Window 5/6 data file dimensions, {} times.",
                                      RoutineName,
                                      state.dataSurfaceGeometry->Warning1Count));
            ShowContinueError(state, "This will affect the frame heat transfer calculation if the frame in the Data File entry");
            ShowContinueError(state, "is not uniform, i.e., has sections with different geometry and/or thermal properties.");
            ShowContinueError(state, "For explicit details on each window, use Output:Diagnostics,DisplayExtraWarnings;");
        }
        if (state.dataSurfaceGeometry->Warning2Count > 0) {
            ShowWarningMessage(state,
                               format("{}Exterior Windows have been replaced with Window 5/6 two glazing systems, {} times.",
                                      RoutineName,
                                      state.dataSurfaceGeometry->Warning2Count));
            ShowContinueError(state, "Note that originally entered dimensions are overridden.");
            ShowContinueError(state, "For explicit details on each window, use Output:Diagnostics,DisplayExtraWarnings;");
        }
        if (state.dataSurfaceGeometry->Warning3Count > 0) {
            ShowWarningMessage(state,
                               format("{}Interior Windows have been replaced with Window 5/6 two glazing systems, {} times.",
                                      RoutineName,
                                      state.dataSurfaceGeometry->Warning3Count));
            ShowContinueError(state, "Note that originally entered dimensions are overridden.");
            ShowContinueError(state, "For explicit details on each window, use Output:Diagnostics,DisplayExtraWarnings;");
        }

        if (state.dataErrTracking->TotalMultipliedWindows > 0) {
            ShowWarningMessage(state,
                               format("{}There are {} window/glass door(s) that may cause inaccurate shadowing due to Solar Distribution.",
                                      RoutineName,
                                      state.dataErrTracking->TotalMultipliedWindows));
            ShowContinueError(state, "For explicit details on each window, use Output:Diagnostics,DisplayExtraWarnings;");
            state.dataErrTracking->TotalWarningErrors += state.dataErrTracking->TotalMultipliedWindows;
        }
        if (state.dataErrTracking->TotalCoincidentVertices > 0) {
            ShowWarningMessage(state,
                               format("{}There are {} coincident/collinear vertices; These have been deleted unless the deletion would bring the "
                                      "number of surface sides < 3.",
                                      RoutineName,
                                      state.dataErrTracking->TotalCoincidentVertices));
            ShowContinueError(state, "For explicit details on each problem surface, use Output:Diagnostics,DisplayExtraWarnings;");
            state.dataErrTracking->TotalWarningErrors += state.dataErrTracking->TotalCoincidentVertices;
        }
        if (state.dataErrTracking->TotalDegenerateSurfaces > 0) {
            ShowSevereMessage(state,
                              format("{}There are {} degenerate surfaces; Degenerate surfaces are those with number of sides < 3.",
                                     RoutineName,
                                     state.dataErrTracking->TotalDegenerateSurfaces));
            ShowContinueError(state, "These surfaces should be deleted.");
            ShowContinueError(state, "For explicit details on each problem surface, use Output:Diagnostics,DisplayExtraWarnings;");
            state.dataErrTracking->TotalSevereErrors += state.dataErrTracking->TotalDegenerateSurfaces;
        }

        GetHTSurfExtVentedCavityData(state, ErrorsFound);

        state.dataSurfaceGeometry->exposedFoundationPerimeter.getData(state, ErrorsFound);

        GetSurfaceHeatTransferAlgorithmOverrides(state, ErrorsFound);

        // Set up enclosures, process Air Boundaries if any
        SetupEnclosuresAndAirBoundaries(state, state.dataViewFactor->EnclRadInfo, SurfaceGeometry::enclosureType::RadiantEnclosures, ErrorsFound);

        GetSurfaceGroundSurfsData(state, ErrorsFound);

        GetSurfaceSrdSurfsData(state, ErrorsFound);

        GetSurfaceLocalEnvData(state, ErrorsFound);

        if (SurfError || ErrorsFound) {
            ErrorsFound = true;
            ShowFatalError(state, format("{}Errors discovered, program terminates.", RoutineName));
        }

        int TotShadSurf = TotDetachedFixed + TotDetachedBldg + TotRectDetachedFixed + TotRectDetachedBldg + TotShdSubs + TotOverhangs +
                          TotOverhangsProjection + TotFins + TotFinsProjection;
        int NumDElightCmplxFen = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Daylighting:DElight:ComplexFenestration");
        if (TotShadSurf > 0 && (NumDElightCmplxFen > 0 || Dayltg::doesDayLightingUseDElight(state))) {
            ShowWarningError(state, format("{}When using DElight daylighting the presence of exterior shading surfaces is ignored.", RoutineName));
        }

        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; SurfNum++) {
            auto &surf = state.dataSurface->Surface(SurfNum);
            // Initialize run time surface arrays
            state.dataSurface->SurfActiveConstruction(SurfNum) = surf.Construction;
            surf.RepresentativeCalcSurfNum = SurfNum;
        }

        // Representative surface calculations: Assign representative heat transfer surfaces
        if (state.dataSurface->UseRepresentativeSurfaceCalculations &&
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneProperty:UserViewFactors:BySurfaceName") == 0) {
            for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
                for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                    auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                    for (int surfNum = thisSpace.HTSurfaceFirst; surfNum <= thisSpace.HTSurfaceLast; surfNum++) {
                        auto &surface(state.dataSurface->Surface(surfNum));
                        // Conditions where surface always needs to be unique
                        bool forceUniqueSurface = surface.HasShadeControl ||
                                                  state.dataSurface->SurfWinAirflowSource(surfNum) != DataSurfaces::WindowAirFlowSource::Invalid ||
                                                  state.dataConstruction->Construct(surface.Construction).SourceSinkPresent ||
                                                  surface.Class == SurfaceClass::TDD_Dome ||
                                                  (surface.Class == SurfaceClass::Window &&
                                                   (surface.OriginalClass == SurfaceClass::TDD_Diffuser ||
                                                    state.dataSurface->SurfWinWindowModelType(surfNum) != DataSurfaces::WindowModel::Detailed ||
                                                    state.dataWindowManager->inExtWindowModel->isExternalLibraryModel() ||
                                                    state.dataConstruction->Construct(surface.Construction).isTCWindow));
                        if (!forceUniqueSurface) {
                            state.dataSurface->Surface(surfNum).set_representative_surface(state, surfNum);
                        }
                    }
                }
            }
        }

        if (SurfError || ErrorsFound) {
            ErrorsFound = true;
            ShowFatalError(state, format("{}Errors discovered, program terminates.", RoutineName));
        }
    }

    void CreateMissingSpaces(EnergyPlusData &state, Array1D<SurfaceGeometry::SurfaceData> &Surfaces)
    {
        static constexpr std::string_view RoutineName = "CreateMissingSpaces: ";
        // Scan surfaces to see if Space was assigned in input
        EPVector<bool> anySurfacesWithSpace;    // True if any surfaces in a zone do not have a space assigned in input
        EPVector<bool> anySurfacesWithoutSpace; // True if any surfaces in a zone have a space assigned in input
        anySurfacesWithSpace.resize(state.dataGlobal->NumOfZones, false);
        anySurfacesWithoutSpace.resize(state.dataGlobal->NumOfZones, false);

        for (int surfNum = 1; surfNum <= state.dataSurface->TotSurfaces; ++surfNum) {
            auto &thisSurf = Surfaces(surfNum);
            if (!thisSurf.HeatTransSurf) {
                continue; // ignore shading surfaces
            }
            if (thisSurf.Class == DataSurfaces::SurfaceClass::IntMass) {
                continue; // skip internal mass surfaces for this check
            }
            if (thisSurf.BaseSurf != surfNum) {
                // Set space for subsurfaces
                thisSurf.spaceNum = Surfaces(thisSurf.BaseSurf).spaceNum;
            }
            if (thisSurf.spaceNum > 0) {
                anySurfacesWithSpace(thisSurf.Zone) = true;
            } else if (thisSurf.ExtBoundCond != unreconciledZoneSurface) {
                anySurfacesWithoutSpace(thisSurf.Zone) = true;
            } else if (thisSurf.Name.substr(0, 3) != "iz-") {
                // Only trigger a new space if the spaceless surface is not an autogenerated interzone surface
                anySurfacesWithoutSpace(thisSurf.Zone) = true;
            }
        }

        // Create any missing Spaces
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            auto &thisZone = state.dataHeatBal->Zone(zoneNum);
            if (anySurfacesWithoutSpace(zoneNum)) {
                // If any surfaces in the zone are not assigned to a space, may need to create a new space
                // Every zone has at least one space, created in HeatBalanceManager::GetSpaceData
                // If no surfaces have a space assigned, then the default space will be used, otherwise, create a new space
                if (anySurfacesWithSpace(zoneNum)) {
                    // Add new space
                    ++state.dataGlobal->numSpaces;
                    assert(state.dataHeatBal->space.size() >= state.dataGlobal->numSpaces);
                    state.dataHeatBal->space(state.dataGlobal->numSpaces).zoneNum = zoneNum;
                    // Add to zone's list of spaces
                    thisZone.spaceIndexes.emplace_back(state.dataGlobal->numSpaces);
                    ++state.dataHeatBal->Zone(zoneNum).numSpaces;
                    assert(state.dataHeatBal->Zone(zoneNum).numSpaces == int(state.dataHeatBal->Zone(zoneNum).spaceIndexes.size()));
                    // If some surfaces in the zone are assigned to a space, the new space is the remainder of the zone
                    state.dataHeatBal->space(state.dataGlobal->numSpaces).Name =
                        thisZone.Name + "-REMAINDER"; // Make UPPERcase so it can be referenced in input
                    state.dataHeatBal->space(state.dataGlobal->numSpaces).isRemainderSpace = true;
                    state.dataHeatBal->space(state.dataGlobal->numSpaces).spaceType = "GENERAL";
                    state.dataHeatBal->space(state.dataGlobal->numSpaces).spaceTypeNum = HeatBalanceManager::GetGeneralSpaceTypeNum(state);
                }
            }
        }
        // Right-size space vector
        state.dataHeatBal->space.resize(state.dataGlobal->numSpaces);

        // Assign Spaces to surfaces without one
        for (int surfNum = 1; surfNum <= state.dataSurface->TotSurfaces; ++surfNum) {
            auto &thisSurf = Surfaces(surfNum);
            if (!thisSurf.HeatTransSurf) {
                continue; // ignore shading surfaces
            }
            if (thisSurf.spaceNum == 0) {
                int const numSpaces = state.dataHeatBal->Zone(thisSurf.Zone).numSpaces;
                int const lastSpaceForZone = state.dataHeatBal->Zone(thisSurf.Zone).spaceIndexes(numSpaces);
                thisSurf.spaceNum = lastSpaceForZone;
                if ((thisSurf.ExtBoundCond == unreconciledZoneSurface) && (thisSurf.Name.substr(0, 3) == "iz-")) {
                    if (state.dataHeatBal->Zone(thisSurf.Zone).numSpaces > 1) {
                        // Only trigger warning if the spaceless surface is an autogenerated interzone surface
                        ShowWarningError(state,
                                         format("{}Surface=\"{}\" has Outside Boundary Condition=Zone, but Zone=\"{}\" has more than 1 Space.",
                                                RoutineName,
                                                thisSurf.Name.substr(3),
                                                thisSurf.ZoneName));
                        ShowContinueError(state,
                                          format("Auto-generated surface=\"{}\" will be assigned to Space=\"{}\"",
                                                 thisSurf.Name,
                                                 state.dataHeatBal->space(thisSurf.spaceNum).Name));
                        ShowContinueError(state, "Use Outside Boundary Condition = Space to specify the exact Space for the outside boundary.");
                    }
                }
            }
        }
    }

    void createSpaceSurfaceLists(EnergyPlusData &state)
    {
        static constexpr std::string_view RoutineName("createSpaceSurfaceLists: ");
        // Build Space surface lists now that all of the surface sorting is complete
        for (int surfNum = 1; surfNum <= state.dataSurface->TotSurfaces; ++surfNum) {
            auto &thisSurf = state.dataSurface->Surface(surfNum);
            if (!thisSurf.HeatTransSurf) {
                continue; // ignore shading surfaces
            }
            // Add to Space's list of surfaces
            state.dataHeatBal->space(thisSurf.spaceNum).surfaces.emplace_back(surfNum);
        }
        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
            if (int(state.dataHeatBal->space(spaceNum).surfaces.size()) == 0) {
                ShowWarningError(state, format("{}Space={} has no surfaces.", RoutineName, state.dataHeatBal->space(spaceNum).Name));
            }
        }
    }

    void setSurfaceFirstLast(EnergyPlusData &state)
    {
        // Set Zone and Space Surface First/Last Pointers
        // Space surface lists have been built earlier in createSpaceSurfaceLists
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
                auto &thisSpace = state.dataHeatBal->space(spaceNum);
                for (int SurfNum : thisSpace.surfaces) {
                    auto &surf = state.dataSurface->Surface(SurfNum);
                    if (thisSpace.AllSurfaceFirst == 0) {
                        thisSpace.AllSurfaceFirst = SurfNum;
                    }
                    thisSpace.AllSurfaceLast = SurfNum;

                    if (surf.IsAirBoundarySurf) {
                        surf.HeatTransSurf = false;
                        continue;
                    }
                    // Non window surfaces are grouped next within each space
                    if (thisSpace.HTSurfaceFirst == 0) {
                        thisSpace.HTSurfaceFirst = SurfNum;
                        thisSpace.OpaqOrIntMassSurfaceFirst = SurfNum;
                        thisSpace.OpaqOrWinSurfaceFirst = SurfNum;
                    }
                    thisSpace.HTSurfaceLast = SurfNum;

                    // Window surfaces are grouped next within each space
                    if ((surf.Class == DataSurfaces::SurfaceClass::Window) || (surf.Class == DataSurfaces::SurfaceClass::GlassDoor) ||
                        (surf.Class == DataSurfaces::SurfaceClass::TDD_Diffuser)) {
                        if (thisSpace.WindowSurfaceFirst == 0) {
                            thisSpace.WindowSurfaceFirst = SurfNum;
                        }
                        thisSpace.WindowSurfaceLast = SurfNum;
                    } else if (surf.Class != DataSurfaces::SurfaceClass::TDD_Dome) {
                        thisSpace.OpaqOrIntMassSurfaceLast = SurfNum;
                    }

                    // TDDDome surfaces are grouped last within each space
                    if (surf.Class == DataSurfaces::SurfaceClass::TDD_Dome) {
                        if (thisSpace.TDDDomeFirst == 0) {
                            thisSpace.TDDDomeFirst = SurfNum;
                        }
                        thisSpace.TDDDomeLast = SurfNum;
                    } else {
                        thisSpace.OpaqOrWinSurfaceLast = SurfNum;
                    }
                }
                state.dataHeatBal->Zone(ZoneNum).AllSurfaceLast = thisSpace.AllSurfaceLast;
            }
            int firstSpaceNum = state.dataHeatBal->Zone(ZoneNum).spaceIndexes(1);
            state.dataHeatBal->Zone(ZoneNum).AllSurfaceFirst = state.dataHeatBal->space(firstSpaceNum).AllSurfaceFirst;
        }
    }

    void checkSubSurfAzTiltNorm(EnergyPlusData &state,
                                SurfaceData &baseSurface, // Base surface data (in)
                                SurfaceData &subSurface,  // Subsurface data (in)
                                bool &surfaceError        // True if surface azimuths or tilts differ by more than error tolerance
    )
    {
        bool sameSurfNormal = false; // True if surface has the same surface normal within tolerance
        Real64 constexpr warningTolerance = 30.0;
        Real64 constexpr errorTolerance = 90.0;

        surfaceError = false; // True if surface has the same surface normal within tolerance

        // Check if base surface and subsurface have the same normal
        Vectors::CompareTwoVectors(baseSurface.NewellSurfaceNormalVector, subSurface.NewellSurfaceNormalVector, sameSurfNormal, 0.001);
        if (sameSurfNormal) { // copy lcs vectors
                              // Prior logic tested for azimuth difference < 30 and then skipped this - this caused large diffs in
                              // CmplxGlz_MeasuredDeflectionAndShading Restoring that check here but will require further investigation (MJW Dec 2015)
                              // if (std::abs(baseSurface.Azimuth - subSurface.Azimuth) > warningTolerance) {
            subSurface.lcsx = baseSurface.lcsx;
            subSurface.lcsy = baseSurface.lcsy;
            subSurface.lcsz = baseSurface.lcsz;
            // }
        } else {
            bool baseSurfHoriz = false; // True if base surface is near horizontal
            // // Not sure what this does, but keeping for now (MJW Dec 2015)
            // if (std::abs(subSurface.Azimuth - 360.0) < 0.01) {
            //     subSurface.Azimuth = 360.0 - subSurface.Azimuth;
            // }
            // if (std::abs(baseSurface.Azimuth - 360.0) < 0.01) {
            //     baseSurface.Azimuth = 360.0 - baseSurface.Azimuth;
            // }

            // Is base surface horizontal? If so, ignore azimuth differences
            if (std::abs(baseSurface.Tilt) <= 1.0e-5 || std::abs(baseSurface.Tilt - 180.0) <= 1.0e-5) {
                baseSurfHoriz = true;
            }

            if (((General::rotAzmDiffDeg(baseSurface.Azimuth, subSurface.Azimuth) > errorTolerance) && !baseSurfHoriz) ||
                (std::abs(baseSurface.Tilt - subSurface.Tilt) > errorTolerance)) {
                surfaceError = true;
                ShowSevereError(
                    state,
                    format("checkSubSurfAzTiltNorm: Outward facing angle of subsurface differs more than {:.1R} degrees from base surface.",
                           errorTolerance));
                ShowContinueError(state,
                                  format("Subsurface=\"{}\" Tilt = {:.1R}  Azimuth = {:.1R}", subSurface.Name, subSurface.Tilt, subSurface.Azimuth));
                ShowContinueError(
                    state, format("Base surface=\"{}\" Tilt = {:.1R}  Azimuth = {:.1R}", baseSurface.Name, baseSurface.Tilt, baseSurface.Azimuth));
            } else if (((General::rotAzmDiffDeg(baseSurface.Azimuth, subSurface.Azimuth) > warningTolerance) && !baseSurfHoriz) ||
                       (std::abs(baseSurface.Tilt - subSurface.Tilt) > warningTolerance)) {
                ++state.dataSurfaceGeometry->checkSubSurfAzTiltNormErrCount;
                if (state.dataSurfaceGeometry->checkSubSurfAzTiltNormErrCount == 1 && !state.dataGlobal->DisplayExtraWarnings) {
                    ShowWarningError(state,
                                     format("checkSubSurfAzTiltNorm: Some Outward Facing angles of subsurfaces differ more than {:.1R} "
                                            "degrees from base surface.",
                                            warningTolerance));
                    ShowContinueError(state, "...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual surfaces.");
                }
                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowWarningError(
                        state,
                        format("checkSubSurfAzTiltNorm: Outward facing angle of subsurface differs more than {:.1R} degrees from base surface.",
                               warningTolerance));
                    ShowContinueError(
                        state, format("Subsurface=\"{}\" Tilt = {:.1R}  Azimuth = {:.1R}", subSurface.Name, subSurface.Tilt, subSurface.Azimuth));
                    ShowContinueError(
                        state,
                        format("Base surface=\"{}\" Tilt = {:.1R}  Azimuth = {:.1R}", baseSurface.Name, baseSurface.Tilt, baseSurface.Azimuth));
                }
            }
        }
    }

    void GetGeometryParameters(EnergyPlusData &state, bool &ErrorsFound) // set to true if errors found during input
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reads in the "Surface Geometry" parameters, verifies them,
        // and sets "global" variables that will tell other routines how the surface
        // vertices are expected in input.

        // REFERENCES:
        // GlobalGeometryRules Definition
        // GlobalGeometryRules,
        //      \required-object
        //      \unique-object
        //  A1, \field Starting Vertex Position
        //      \required-field
        //      \note Specified as entry for a 4 sided surface/rectangle
        //      \note Surfaces are specified as viewed from outside the surface
        //      \note Shading surfaces as viewed from behind.  (towards what they are shading)
        //      \type choice
        //      \key UpperLeftCorner
        //      \key LowerLeftCorner
        //      \key UpperRightCorner
        //      \key LowerRightCorner
        //  A2, \field Vertex Entry Direction
        //      \required-field
        //      \type choice
        //      \key Counterclockwise
        //      \key Clockwise
        //  A3, \field Coordinate System
        //      \required-field
        //      \note relative -- coordinates are entered relative to zone origin
        //      \note world -- all coordinates entered are "absolute" for this facility
        //      \note absolute -- same as world
        //      \type choice
        //      \key Relative
        //      \key World
        //      \key Absolute
        //  A4, \field Daylighting Reference Point Coordinate System
        //      \type choice
        //      \key Relative
        //      \default Relative
        //      \note Relative -- coordinates are entered relative to zone origin
        //      \key World
        //      \note World -- all coordinates entered are "absolute" for this facility
        //      \key Absolute
        //      \note absolute -- same as world
        //  A5; \field Rectangular Surface Coordinate System
        //      \type choice
        //      \key Relative
        //      \default Relative
        //      \note Relative -- Starting corner is entered relative to zone origin
        //      \key World
        //      \note World -- Starting corner is entered in "absolute"
        //      \key Absolute
        //      \note absolute -- same as world

        // SUBROUTINE PARAMETER DEFINITIONS:
        static Array1D_string const FlCorners(4, {"UpperLeftCorner", "LowerLeftCorner", "LowerRightCorner", "UpperRightCorner"});

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D_string GAlphas(5);
        Array1D<Real64> GNum(1);

        auto &s_ipsc = state.dataIPShortCut;

        s_ipsc->cCurrentModuleObject = "GlobalGeometryRules";
        int NumStmt = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        std::string OutMsg = " Surface Geometry,";

        {
            int const SELECT_CASE_var = NumStmt;

            if (SELECT_CASE_var == 1) {
                int NNum;
                int IOStat;
                int NAlphas;
                // This is the valid case
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         s_ipsc->cCurrentModuleObject,
                                                                         1,
                                                                         GAlphas,
                                                                         NAlphas,
                                                                         GNum,
                                                                         NNum,
                                                                         IOStat,
                                                                         s_ipsc->lNumericFieldBlanks,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);

                // Even though these will be validated, set defaults in case error here -- wont
                // cause aborts in later surface gets (hopefully)
                state.dataSurface->Corner = DataSurfaces::UpperLeftCorner;
                state.dataSurface->WorldCoordSystem = true;
                state.dataSurface->CCW = true;

                int Found = Util::FindItem(GAlphas(1), FlCorners, 4);
                if (Found == 0) {
                    ShowSevereError(state, format("{}: Invalid {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(1), GAlphas(1)));
                    ErrorsFound = true;
                } else {
                    state.dataSurface->Corner = Found;
                    OutMsg += FlCorners(state.dataSurface->Corner) + ',';
                }

                bool OK = false;
                if (Util::SameString(GAlphas(2), "CCW") || Util::SameString(GAlphas(2), "Counterclockwise")) {
                    state.dataSurface->CCW = true;
                    OutMsg += "Counterclockwise,";
                    OK = true;
                }
                if (Util::SameString(GAlphas(2), "CW") || Util::SameString(GAlphas(2), "Clockwise")) {
                    state.dataSurface->CCW = false;
                    OutMsg += "Clockwise,";
                    OK = true;
                }
                if (!OK) {
                    ShowSevereError(state, format("{}: Invalid {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(2), GAlphas(2)));
                    ErrorsFound = true;
                }

                OK = false;
                if (Util::SameString(GAlphas(3), "World") || Util::SameString(GAlphas(3), "Absolute")) {
                    state.dataSurface->WorldCoordSystem = true;
                    OutMsg += "WorldCoordinateSystem,";
                    OK = true;
                }
                if (Util::SameString(GAlphas(3), "Relative")) {
                    state.dataSurface->WorldCoordSystem = false;
                    OutMsg += "RelativeCoordinateSystem,";
                    OK = true;
                }
                if (!OK) {
                    ShowWarningError(state, format("{}: Invalid {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(3), GAlphas(3)));
                    ShowContinueError(state, format("{} defaults to \"WorldCoordinateSystem\"", s_ipsc->cAlphaFieldNames(3)));
                    state.dataSurface->WorldCoordSystem = true;
                    OutMsg += "WorldCoordinateSystem,";
                }

                OK = false;
                if (Util::SameString(GAlphas(4), "World") || Util::SameString(GAlphas(4), "Absolute")) {
                    state.dataSurface->DaylRefWorldCoordSystem = true;
                    OutMsg += "WorldCoordinateSystem,";
                    OK = true;
                }
                if (Util::SameString(GAlphas(4), "Relative") || GAlphas(4).empty()) {
                    state.dataSurface->DaylRefWorldCoordSystem = false;
                    OutMsg += "RelativeCoordinateSystem,";
                    OK = true;
                }
                if (!OK) {
                    ShowWarningError(state, format("{}: Invalid {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(4), GAlphas(4)));
                    ShowContinueError(state, format("{} defaults to \"RelativeToZoneOrigin\"", s_ipsc->cAlphaFieldNames(4)));
                    state.dataSurface->DaylRefWorldCoordSystem = false;
                    OutMsg += "RelativeToZoneOrigin,";
                }

                OK = false;
                if (Util::SameString(GAlphas(5), "World") || Util::SameString(GAlphas(5), "Absolute")) {
                    state.dataSurfaceGeometry->RectSurfRefWorldCoordSystem = true;
                    OutMsg += "WorldCoordinateSystem";
                    OK = true;
                }
                if (Util::SameString(GAlphas(5), "Relative") || GAlphas(5).empty()) {
                    state.dataSurfaceGeometry->RectSurfRefWorldCoordSystem = false;
                    OutMsg += "RelativeToZoneOrigin";
                    OK = true;
                }
                if (!OK) {
                    ShowWarningError(state, format("{}: Invalid {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(5), GAlphas(5)));
                    ShowContinueError(state, format("{} defaults to \"RelativeToZoneOrigin\"", s_ipsc->cAlphaFieldNames(5)));
                    state.dataSurfaceGeometry->RectSurfRefWorldCoordSystem = false;
                    OutMsg += "RelativeToZoneOrigin";
                }

            } else if (SELECT_CASE_var == 0) {

                ShowSevereError(state, format("{}: Required object not found.", s_ipsc->cCurrentModuleObject));
                OutMsg += "None found in input";
                ErrorsFound = true;

            } else {

                ShowSevereError(state, format("{}: Too many objects entered.  Only one allowed.", s_ipsc->cCurrentModuleObject));
                ErrorsFound = true;
            }
        }

        if (!state.dataSurface->WorldCoordSystem) {
            if (state.dataSurface->DaylRefWorldCoordSystem) {
                ShowWarningError(state, format("{}: Potential mismatch of coordinate specifications.", s_ipsc->cCurrentModuleObject));
                ShowContinueError(state, format("{}=\"{}\"; while ", s_ipsc->cAlphaFieldNames(3), GAlphas(3)));
                ShowContinueError(state, format("{}=\"{}\".", s_ipsc->cAlphaFieldNames(4), GAlphas(4)));
            }
            if (state.dataSurfaceGeometry->RectSurfRefWorldCoordSystem) {
                ShowWarningError(state, format("{}: Potential mismatch of coordinate specifications.", s_ipsc->cCurrentModuleObject));
                ShowContinueError(state, format("{}=\"{}\"; while ", s_ipsc->cAlphaFieldNames(3), GAlphas(3)));
                ShowContinueError(state, format("{}=\"{}\".", s_ipsc->cAlphaFieldNames(5), GAlphas(5)));
            }
        } else {
            bool RelWarning = false;
            for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
                if (state.dataHeatBal->Zone(ZoneNum).OriginX != 0.0) {
                    RelWarning = true;
                }
                if (state.dataHeatBal->Zone(ZoneNum).OriginY != 0.0) {
                    RelWarning = true;
                }
                if (state.dataHeatBal->Zone(ZoneNum).OriginZ != 0.0) {
                    RelWarning = true;
                }
            }
            if (RelWarning && !state.dataSurfaceGeometry->RectSurfRefWorldCoordSystem) {
                ShowWarningError(state,
                                 format("{}: Potential mismatch of coordinate specifications. Note that the rectangular surfaces are relying on the "
                                        "default SurfaceGeometry for 'Relative to zone' coordinate.",
                                        s_ipsc->cCurrentModuleObject));
                ShowContinueError(state, format("{}=\"{}\"; while ", s_ipsc->cAlphaFieldNames(3), GAlphas(3)));
                if (GAlphas(5) == "RELATIVE") {
                    ShowContinueError(state, format("{}=\"{}\".", s_ipsc->cAlphaFieldNames(5), GAlphas(5)));
                } else if (GAlphas(5) != "ABSOLUTE") {
                    ShowContinueError(state, format("{}=\"defaults to RELATIVE\".", s_ipsc->cAlphaFieldNames(5)));
                }
            }
        }

        print(state.files.eio,
              "! <Surface Geometry>,Starting Corner,Vertex Input Direction,Coordinate System,Daylight Reference "
              "Point Coordinate System,Rectangular (Simple) Surface Coordinate System\n");
        print(state.files.eio, "{}\n", OutMsg);
    }

    void GetDetShdSurfaceData(EnergyPlusData &state,
                              bool &ErrorsFound,          // Error flag indicator (true if errors found)
                              int &SurfNum,               // Count of Current SurfaceNumber
                              int const TotDetachedFixed, // Number of Fixed Detached Shading Surfaces to obtain
                              int const TotDetachedBldg   // Number of Building Detached Shading Surfaces to obtain
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the Detached Shading Surface Data,
        // checks it for errors, etc.

        static constexpr std::string_view routineName = "GetDetShdSurfaceData";
        // SUBROUTINE PARAMETER DEFINITIONS:
        static Array1D_string const cModuleObjects(2, {"Shading:Site:Detailed", "Shading:Building:Detailed"});

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int IOStat;     // IO Status when calling get input subroutine
        int NumAlphas;  // Number of material alpha names being passed
        int NumNumbers; // Number of material properties being passed
        int Loop;
        int ItemsToGet;
        SurfaceClass ClassItem;
        int numSides;
        Real64 SchedMinValue;
        Real64 SchedMaxValue;

        auto &s_ipsc = state.dataIPShortCut;

        if ((TotDetachedFixed + TotDetachedBldg) > 0 && state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::Minimal) {
            ShowWarningError(state, "Detached shading effects are ignored when Solar Distribution = MinimalShadowing");
        }

        if ((TotDetachedFixed + TotDetachedBldg) == 0) {
            return;
        }

        for (int Item = 1; Item <= 2; ++Item) {

            s_ipsc->cCurrentModuleObject = cModuleObjects(Item);
            if (Item == 1) {
                ItemsToGet = TotDetachedFixed;
                ClassItem = SurfaceClass::Detached_F;
            } else { // IF (Item == 2) THEN
                ItemsToGet = TotDetachedBldg;
                ClassItem = SurfaceClass::Detached_B;
            }

            state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, s_ipsc->cCurrentModuleObject, Loop, NumAlphas, NumNumbers);
            if (NumAlphas != 2) {
                ShowSevereError(
                    state,
                    format("{}: Object Definition indicates not = 2 Alpha Objects, Number Indicated={}", s_ipsc->cCurrentModuleObject, NumAlphas));
                ErrorsFound = true;
            }

            for (Loop = 1; Loop <= ItemsToGet; ++Loop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         s_ipsc->cCurrentModuleObject,
                                                                         Loop,
                                                                         s_ipsc->cAlphaArgs,
                                                                         NumAlphas,
                                                                         s_ipsc->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStat,
                                                                         s_ipsc->lNumericFieldBlanks,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

                if (GlobalNames::VerifyUniqueInterObjectName(state,
                                                             state.dataSurfaceGeometry->UniqueSurfaceNames,
                                                             s_ipsc->cAlphaArgs(1),
                                                             s_ipsc->cCurrentModuleObject,
                                                             s_ipsc->cAlphaFieldNames(1),
                                                             ErrorsFound)) {
                    continue;
                }

                ++SurfNum;

                auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
                surfTemp.Name = s_ipsc->cAlphaArgs(1); // Set the Surface Name in the Derived Type
                surfTemp.Class = ClassItem;
                surfTemp.HeatTransSurf = false;
                surfTemp.ExtSolar = true;
                // Base transmittance of a shadowing (sub)surface

                if (s_ipsc->lAlphaFieldBlanks(2)) {
                    surfTemp.shadowSurfSched =
                        nullptr; // Leaving this as nullptr rather than making AlwaysOff because the uses and tests are too varied
                } else if ((surfTemp.shadowSurfSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                    ErrorsFound = true;
                } else if (!surfTemp.shadowSurfSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                    Sched::ShowSevereBadMinMax(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2), Clusive::In, 0.0, Clusive::In, 1.0);
                    ErrorsFound = true;
                } else {
                    SchedMinValue = surfTemp.shadowSurfSched->getMinVal(state);
                    surfTemp.SchedMinValue = SchedMinValue;
                    SchedMaxValue = surfTemp.shadowSurfSched->getCurrentVal();
                    if (SchedMinValue == 1.0) {
                        // Set transparent for now, check for EMS actuators later in SolarShading::resetShadingSurfaceTransparency
                        surfTemp.IsTransparent = true;
                    }
                    if (SchedMinValue < 0.0) {
                        ShowSevereError(state,
                                        format("{}=\"{}\", {}=\"{}\", has schedule values < 0.",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               s_ipsc->cAlphaFieldNames(2),
                                               s_ipsc->cAlphaArgs(2)));
                        ShowContinueError(state, "...Schedule values < 0 have no meaning for shading elements.");
                    }
                    if (SchedMaxValue > 0.0) {
                        state.dataSolarShading->anyScheduledShadingSurface = true;
                    }
                    if (SchedMaxValue > 1.0) {
                        ShowSevereError(state,
                                        format("{}=\"{}\", {}=\"{}\", has schedule values > 1.",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               s_ipsc->cAlphaFieldNames(2),
                                               s_ipsc->cAlphaArgs(2)));
                        ShowContinueError(state, "...Schedule values > 1 have no meaning for shading elements.");
                    }
                    if (std::abs(SchedMinValue - SchedMaxValue) > Constant::OneMillionth) {
                        state.dataSurface->ShadingTransmittanceVaries = true;
                    }
                }
                if (s_ipsc->lNumericFieldBlanks(1) || s_ipsc->rNumericArgs(1) == Constant::AutoCalculate) {
                    numSides = (NumNumbers - 1) / 3;
                    surfTemp.Sides = numSides;
                    if (mod(NumNumbers - 1, 3) != 0) {
                        ShowWarningError(state,
                                         format("{}=\"{}\", {}",
                                                s_ipsc->cCurrentModuleObject,
                                                surfTemp.Name,
                                                format("{} not even multiple of 3. Will read in {}", s_ipsc->cNumericFieldNames(1), surfTemp.Sides)));
                    }
                    if (numSides < 3) {
                        ShowSevereError(state,
                                        format("{}=\"{}\", {} (autocalculate) must be >= 3. Only {} provided.",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               s_ipsc->cNumericFieldNames(1),
                                               surfTemp.Sides));
                        ErrorsFound = true;
                        continue;
                    }
                } else {
                    numSides = (NumNumbers - 1) / 3;
                    surfTemp.Sides = s_ipsc->rNumericArgs(1);
                    if (numSides > surfTemp.Sides) {
                        ShowWarningError(state,
                                         format("{}=\"{}\", field {}={}",
                                                s_ipsc->cCurrentModuleObject,
                                                surfTemp.Name,
                                                s_ipsc->cNumericFieldNames(1),
                                                fmt::to_string(surfTemp.Sides)));
                        ShowContinueError(
                            state, format("...but {} were entered. Only the indicated {} will be used.", numSides, s_ipsc->cNumericFieldNames(1)));
                    }
                }
                surfTemp.Vertex.allocate(surfTemp.Sides);
                GetVertices(state, SurfNum, surfTemp.Sides, s_ipsc->rNumericArgs({2, _}));
                CheckConvexity(state, SurfNum, surfTemp.Sides);
                if (state.dataReportFlag->MakeMirroredDetachedShading) {
                    MakeMirrorSurface(state, SurfNum);
                }
            }

        } // Item Loop
    }

    void GetRectDetShdSurfaceData(EnergyPlusData &state,
                                  bool &ErrorsFound,              // Error flag indicator (true if errors found)
                                  int &SurfNum,                   // Count of Current SurfaceNumber
                                  int const TotRectDetachedFixed, // Number of Fixed Detached Shading Surfaces to obtain
                                  int const TotRectDetachedBldg   // Number of Building Detached Shading Surfaces to obtain
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2009

        // PURPOSE OF THIS SUBROUTINE:
        // Gets the simple, rectangular detached surfaces.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static Array1D_string const cModuleObjects(2, {"Shading:Site", "Shading:Building"});

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int IOStat;     // IO Status when calling get input subroutine
        int NumAlphas;  // Number of material alpha names being passed
        int NumNumbers; // Number of material properties being passed
        int Loop;
        int ItemsToGet;
        SurfaceClass ClassItem;

        auto &s_ipsc = state.dataIPShortCut;

        if ((TotRectDetachedFixed + TotRectDetachedBldg) > 0 && state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::Minimal) {
            ShowWarningError(state, "Detached shading effects are ignored when Solar Distribution = MinimalShadowing");
        }

        if (TotRectDetachedFixed + TotRectDetachedBldg == 0) {
            return;
        }
        for (int Item = 1; Item <= 2; ++Item) {

            s_ipsc->cCurrentModuleObject = cModuleObjects(Item);
            if (Item == 1) {
                ItemsToGet = TotRectDetachedFixed;
                ClassItem = SurfaceClass::Detached_F;
            } else { // IF (Item == 2) THEN
                ItemsToGet = TotRectDetachedBldg;
                ClassItem = SurfaceClass::Detached_B;
            }

            state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, s_ipsc->cCurrentModuleObject, Loop, NumAlphas, NumNumbers);
            if (NumAlphas != 1) {
                ShowSevereError(
                    state,
                    format("{}: Object Definition indicates not = 1 Alpha Objects, Number Indicated={}", s_ipsc->cCurrentModuleObject, NumAlphas));
                ErrorsFound = true;
            }

            for (Loop = 1; Loop <= ItemsToGet; ++Loop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         s_ipsc->cCurrentModuleObject,
                                                                         Loop,
                                                                         s_ipsc->cAlphaArgs,
                                                                         NumAlphas,
                                                                         s_ipsc->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStat,
                                                                         s_ipsc->lNumericFieldBlanks,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);

                if (GlobalNames::VerifyUniqueInterObjectName(state,
                                                             state.dataSurfaceGeometry->UniqueSurfaceNames,
                                                             s_ipsc->cAlphaArgs(1),
                                                             s_ipsc->cCurrentModuleObject,
                                                             s_ipsc->cAlphaFieldNames(1),
                                                             ErrorsFound)) {
                    continue;
                }

                ++SurfNum;

                auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
                surfTemp.Name = s_ipsc->cAlphaArgs(1); // Set the Surface Name in the Derived Type
                surfTemp.Class = ClassItem;
                surfTemp.HeatTransSurf = false;
                surfTemp.ExtSolar = true;

                surfTemp.Azimuth = s_ipsc->rNumericArgs(1);
                if (surfTemp.Class == SurfaceClass::Detached_B && !state.dataSurface->WorldCoordSystem) {
                    surfTemp.Azimuth += state.dataHeatBal->BuildingAzimuth;
                }
                if (surfTemp.Class == SurfaceClass::Detached_B) {
                    surfTemp.Azimuth += state.dataHeatBal->BuildingRotationAppendixG;
                }
                surfTemp.Tilt = s_ipsc->rNumericArgs(2);
                surfTemp.convOrientation = Convect::GetSurfConvOrientation(surfTemp.Tilt);

                surfTemp.Sides = 4;
                surfTemp.Vertex.allocate(surfTemp.Sides);

                MakeRectangularVertices(state,
                                        SurfNum,
                                        s_ipsc->rNumericArgs(3),
                                        s_ipsc->rNumericArgs(4),
                                        s_ipsc->rNumericArgs(5),
                                        s_ipsc->rNumericArgs(6),
                                        s_ipsc->rNumericArgs(7),
                                        state.dataSurfaceGeometry->RectSurfRefWorldCoordSystem);

                if (surfTemp.Area <= 0.0) {
                    ShowSevereError(
                        state,
                        format("{}=\"{}\", Surface Area <= 0.0; Entered Area={:.2T}", s_ipsc->cCurrentModuleObject, surfTemp.Name, surfTemp.Area));
                    ErrorsFound = true;
                }

                if (state.dataReportFlag->MakeMirroredDetachedShading) {
                    MakeMirrorSurface(state, SurfNum);
                }
            }

        } // Item Loop
    }

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
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the HeatTransfer Surface Data, checks it for errors, etc.

        // REFERENCES:
        // Heat Transfer Surface Definition
        // BuildingSurface:Detailed,
        //  \extensible:3 -- duplicate last set of x,y,z coordinates (last 3 fields), remembering to remove ; from "inner" fields.
        //  \format vertices
        //  A1 , \field Name
        //       \required-field
        //       \type alpha
        //       \reference SurfaceNames
        //       \reference SurfAndSubSurfNames
        //       \reference AllHeatTranSurfNames
        //       \reference HeatTranBaseSurfNames
        //       \reference OutFaceEnvNames
        //       \reference AllHeatTranAngFacNames
        //       \reference RadGroupAndSurfNames
        //       \reference SurfGroupAndHTSurfNames
        //       \reference AllShadingAndHTSurfNames
        //  A2 , \field Surface Type
        //       \required-field
        //       \type choice
        //       \key Floor
        //       \key Wall
        //       \key Ceiling
        //       \key Roof
        //  A3 , \field Construction Name
        //       \required-field
        //       \note To be matched with a construction in this input file
        //       \type object-list
        //       \object-list ConstructionNames
        //  A4 , \field Zone Name
        //       \required-field
        //       \note Zone the surface is a part of
        //       \type object-list
        //       \object-list ZoneNames
        //  A5 , \field Outside Boundary Condition
        //       \required-field
        //       \type choice
        //       \key Adiabatic
        //       \key Surface
        //       \key Zone
        //       \key Outdoors
        //       \key Ground
        //       \key GroundFCfactorMethod
        //       \key OtherSideCoefficients
        //       \key OtherSideConditionsModel
        //       \key GroundSlabPreprocessorAverage
        //       \key GroundSlabPreprocessorCore
        //       \key GroundSlabPreprocessorPerimeter
        //       \key GroundBasementPreprocessorAverageWall
        //       \key GroundBasementPreprocessorAverageFloor
        //       \key GroundBasementPreprocessorUpperWall
        //       \key GroundBasementPreprocessorLowerWall
        //  A6,  \field Outside Boundary Condition Object
        //       \type object-list
        //       \object-list OutFaceEnvNames
        //       \note Non-blank only if the field Outside Boundary Condition is Surface,
        //       \note Zone, OtherSideCoefficients or OtherSideConditionsModel
        //       \note If Surface, specify name of corresponding surface in adjacent zone or
        //       \note specify current surface name for internal partition separating like zones
        //       \note If Zone, specify the name of the corresponding zone and
        //       \note the program will generate the corresponding interzone surface
        //       \note If OtherSideCoefficients, specify name of SurfaceProperty:OtherSideCoefficients
        //       \note If OtherSideConditionsModel, specify name of SurfaceProperty:OtherSideConditionsModel
        //  A7 , \field Sun Exposure
        //       \required-field
        //       \type choice
        //       \key SunExposed
        //       \key NoSun
        //       \default SunExposed
        //  A8,  \field Wind Exposure
        //       \required-field
        //       \type choice
        //       \key WindExposed
        //       \key NoWind
        //       \default WindExposed
        //  N1,  \field View Factor to Ground
        //       \type real
        //       \note From the exterior of the surface
        //       \note Unused if one uses the "reflections" options in Solar Distribution in Building input
        //       \note unless a DaylightingDevice:Shelf or DaylightingDevice:Tubular object has been specified.
        //       \note autocalculate will automatically calculate this value from the tilt of the surface
        //       \autocalculatable
        //       \minimum 0.0
        //       \maximum 1.0
        //       \default autocalculate
        //  N2 , \field Number of Vertices
        //       \note shown with 120 vertex coordinates -- extensible object
        //       \note  "extensible" -- duplicate last set of x,y,z coordinates (last 3 fields),
        //       \note remembering to remove ; from "inner" fields.
        //       \note for clarity in any error messages, renumber the fields as well.
        //       \note (and changing z terminator to a comma "," for all but last one which needs a semi-colon ";")
        //       \autocalculatable
        //       \minimum 3
        //       \default autocalculate
        //       \note vertices are given in GlobalGeometryRules coordinates -- if relative, all surface coordinates
        //       \note are "relative" to the Zone Origin.  If world, then building and zone origins are used
        //       \note for some internal calculations, but all coordinates are given in an "absolute" system.
        //  N3-xx as indicated by the N3 value

        // Using/Aliasing

        // SUBROUTINE PARAMETER DEFINITIONS:
        static Array1D_string const cModuleObjects(4, {"BuildingSurface:Detailed", "Wall:Detailed", "Floor:Detailed", "RoofCeiling:Detailed"});

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int IOStat;          // IO Status when calling get input subroutine
        int SurfaceNumAlpha; // Number of material alpha names being passed
        int SurfaceNumProp;  // Number of material properties being passed
        int ZoneNum;         // DO loop counter (zones)
        int Found;           // For matching interzone surfaces
        int Loop;
        int ItemsToGet;
        int ClassItem;
        int ArgPointer;
        int numSides;

        auto &s_ipsc = state.dataIPShortCut;

        GetOSCData(state, ErrorsFound);
        GetOSCMData(state, ErrorsFound);
        GetFoundationData(state, ErrorsFound);

        NeedToAddSurfaces = 0;
        for (int Item = 1; Item <= 4; ++Item) {

            s_ipsc->cCurrentModuleObject = cModuleObjects(Item);
            if (Item == 1) {
                ItemsToGet = TotHTSurfs;
                ClassItem = 0;
            } else if (Item == 2) {
                ItemsToGet = TotDetailedWalls;
                ClassItem = 1;
            } else if (Item == 3) {
                ItemsToGet = TotDetailedFloors;
                ClassItem = 2;
            } else { // IF (Item == 4) THEN
                ItemsToGet = TotDetailedRoofs;
                ClassItem = 3;
            }

            state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
                state, s_ipsc->cCurrentModuleObject, Loop, SurfaceNumAlpha, SurfaceNumProp);
            if (Item == 1) {
                if (SurfaceNumAlpha != 9) {
                    ShowSevereError(state,
                                    format("{}: Object Definition indicates not = 9 Alpha Objects, Number Indicated={}",
                                           s_ipsc->cCurrentModuleObject,
                                           SurfaceNumAlpha));
                    ErrorsFound = true;
                }
            } else {
                if (SurfaceNumAlpha != 8) {
                    ShowSevereError(state,
                                    format("{}: Object Definition indicates not = 8 Alpha Objects, Number Indicated={}",
                                           s_ipsc->cCurrentModuleObject,
                                           SurfaceNumAlpha));
                    ErrorsFound = true;
                }
            }

            for (Loop = 1; Loop <= ItemsToGet; ++Loop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         s_ipsc->cCurrentModuleObject,
                                                                         Loop,
                                                                         s_ipsc->cAlphaArgs,
                                                                         SurfaceNumAlpha,
                                                                         s_ipsc->rNumericArgs,
                                                                         SurfaceNumProp,
                                                                         IOStat,
                                                                         s_ipsc->lNumericFieldBlanks,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);

                if (GlobalNames::VerifyUniqueInterObjectName(state,
                                                             state.dataSurfaceGeometry->UniqueSurfaceNames,
                                                             s_ipsc->cAlphaArgs(1),
                                                             s_ipsc->cCurrentModuleObject,
                                                             s_ipsc->cAlphaFieldNames(1),
                                                             ErrorsFound)) {
                    continue;
                }

                ++SurfNum;

                auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
                surfTemp.Name = s_ipsc->cAlphaArgs(1); // Set the Surface Name in the Derived Type
                ArgPointer = 2;
                if (Item == 1) {
                    if (s_ipsc->cAlphaArgs(2) == "CEILING") {
                        s_ipsc->cAlphaArgs(2) = "ROOF";
                    }
                    ClassItem = Util::FindItemInList(s_ipsc->cAlphaArgs(2), BaseSurfCls, 3);
                    if (ClassItem == 0) {
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               s_ipsc->cAlphaFieldNames(2),
                                               s_ipsc->cAlphaArgs(2)));
                        ErrorsFound = true;
                    } else {
                        surfTemp.Class = BaseSurfIDs(ClassItem);
                    }
                    ++ArgPointer;
                } else {
                    surfTemp.Class = BaseSurfIDs(ClassItem);
                }

                surfTemp.Construction =
                    Util::FindItemInList(s_ipsc->cAlphaArgs(ArgPointer), state.dataConstruction->Construct, state.dataHeatBal->TotConstructs);

                if (surfTemp.Construction == 0) {
                    ErrorsFound = true;
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\".",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(ArgPointer),
                                           s_ipsc->cAlphaArgs(ArgPointer)));
                } else if (state.dataConstruction->Construct(surfTemp.Construction).TypeIsWindow) {
                    ErrorsFound = true;
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\" - has Window materials.",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(ArgPointer),
                                           s_ipsc->cAlphaArgs(ArgPointer)));
                    if (Item == 1) {
                        ShowContinueError(state, format("...because {}={}", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
                    } else {
                        ShowContinueError(state, format("...because Surface Type={}", BaseSurfCls(ClassItem)));
                    }
                } else {
                    state.dataConstruction->Construct(surfTemp.Construction).IsUsed = true;
                    surfTemp.ConstructionStoredInputValue = surfTemp.Construction;
                }
                surfTemp.HeatTransSurf = true;
                surfTemp.BaseSurf = SurfNum;
                surfTemp.BaseSurfName = surfTemp.Name;

                ++ArgPointer;
                surfTemp.ZoneName = s_ipsc->cAlphaArgs(ArgPointer);
                ZoneNum = Util::FindItemInList(surfTemp.ZoneName, state.dataHeatBal->Zone, state.dataGlobal->NumOfZones);

                if (ZoneNum != 0) {
                    surfTemp.Zone = ZoneNum;
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\".",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(ArgPointer),
                                           s_ipsc->cAlphaArgs(ArgPointer)));
                    surfTemp.Class = SurfaceClass::Invalid;
                    surfTemp.ZoneName = "Unknown Zone";
                    ErrorsFound = true;
                }

                ++ArgPointer;
                if (!s_ipsc->lAlphaFieldBlanks(ArgPointer)) {
                    int spaceNum = Util::FindItemInList(s_ipsc->cAlphaArgs(ArgPointer), state.dataHeatBal->space);

                    if (spaceNum != 0) {
                        surfTemp.spaceNum = spaceNum;
                        if (surfTemp.Zone != state.dataHeatBal->space(spaceNum).zoneNum) {
                            ShowSevereError(state,
                                            format("{}=\"{}\", invalid {}=\"{}\" is not in the same zone as the surface.",
                                                   s_ipsc->cCurrentModuleObject,
                                                   surfTemp.Name,
                                                   s_ipsc->cAlphaFieldNames(ArgPointer),
                                                   s_ipsc->cAlphaArgs(ArgPointer)));
                            surfTemp.Class = SurfaceClass::Invalid;
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}\" not found.",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               s_ipsc->cAlphaFieldNames(ArgPointer),
                                               s_ipsc->cAlphaArgs(ArgPointer)));
                        surfTemp.Class = SurfaceClass::Invalid;
                        ErrorsFound = true;
                    }
                }
                // Get the ExteriorBoundaryCondition flag from input There are 4 conditions that
                // can take place. The conditions are set with a 0, -1, or -2, or all of the
                // zone names have to be looked at and generate the interzone array number
                ++ArgPointer;
                surfTemp.ExtBoundCondName = s_ipsc->cAlphaArgs(ArgPointer + 1);

                if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "Outdoors")) {
                    surfTemp.ExtBoundCond = DataSurfaces::ExternalEnvironment;
                } else if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "Adiabatic")) {
                    surfTemp.ExtBoundCond = unreconciledZoneSurface;
                    surfTemp.ExtBoundCondName = surfTemp.Name;

                } else if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "Ground")) {
                    surfTemp.ExtBoundCond = DataSurfaces::Ground;
                    if (state.dataSurfaceGeometry->NoGroundTempObjWarning) {
                        if (!state.dataEnvrn->GroundTempInputs[(int)DataEnvironment::GroundTempType::BuildingSurface]) {
                            ShowWarningError(state,
                                             "GetHTSurfaceData: Surfaces with interface to Ground found but no \"Ground Temperatures\" were input.");
                            ShowContinueError(state, format("Found first in surface={}", s_ipsc->cAlphaArgs(1)));
                            ShowContinueError(state,
                                              format("Defaults, constant throughout the year of ({:.1R}) will be used.",
                                                     state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface]));
                        }
                        state.dataSurfaceGeometry->NoGroundTempObjWarning = false;
                    }

                    // Added for FCfactor method
                } else if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "GroundFCfactorMethod")) {
                    surfTemp.ExtBoundCond = DataSurfaces::GroundFCfactorMethod;
                    if (state.dataSurfaceGeometry->NoFCGroundTempObjWarning) {
                        if (!state.dataEnvrn->GroundTempInputs[(int)DataEnvironment::GroundTempType::FCFactorMethod]) {
                            ShowSevereError(state,
                                            "GetHTSurfaceData: Surfaces with interface to GroundFCfactorMethod found but no \"FC Ground "
                                            "Temperatures\" were input.");
                            ShowContinueError(state, format("Found first in surface={}", s_ipsc->cAlphaArgs(1)));
                            ShowContinueError(state,
                                              "Either add a \"Site:GroundTemperature:FCfactorMethod\" object or use a weather file with "
                                              "Ground Temperatures.");
                            ErrorsFound = true;
                            state.dataSurfaceGeometry->NoFCGroundTempObjWarning = false;
                        }
                    }
                    if (surfTemp.Construction > 0) {
                        if (surfTemp.Class == SurfaceClass::Wall && !state.dataConstruction->Construct(surfTemp.Construction).TypeIsCfactorWall) {
                            ShowSevereError(
                                state,
                                format("{}=\"{}\", invalid {}", s_ipsc->cCurrentModuleObject, surfTemp.Name, s_ipsc->cAlphaFieldNames(ArgPointer)));
                            ShowContinueError(state,
                                              format("Construction=\"{}\" is not type Construction:CfactorUndergroundWall.",
                                                     state.dataConstruction->Construct(surfTemp.Construction).Name));
                            ErrorsFound = true;
                        }
                        if (surfTemp.Class == SurfaceClass::Floor && !state.dataConstruction->Construct(surfTemp.Construction).TypeIsFfactorFloor) {
                            ShowSevereError(
                                state,
                                format("{}=\"{}\", invalid {}", s_ipsc->cCurrentModuleObject, surfTemp.Name, s_ipsc->cAlphaFieldNames(ArgPointer)));
                            ShowContinueError(state,
                                              format("Construction=\"{}\" is not type Construction:FfactorGroundFloor.",
                                                     state.dataConstruction->Construct(surfTemp.Construction).Name));
                            ErrorsFound = true;
                        }
                    }

                } else if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "OtherSideCoefficients")) {
                    Found = Util::FindItemInList(surfTemp.ExtBoundCondName, state.dataSurface->OSC, state.dataSurface->TotOSC);
                    if (Found == 0) {
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}\".",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               s_ipsc->cAlphaFieldNames(ArgPointer + 1),
                                               s_ipsc->cAlphaArgs(ArgPointer + 1)));
                        ShowContinueError(state, " no OtherSideCoefficients of that name.");
                        ErrorsFound = true;
                    } else {
                        surfTemp.OSCPtr = Found;
                        if (state.dataSurface->OSC(Found).SurfFilmCoef > 0.0) {
                            surfTemp.ExtBoundCond = DataSurfaces::OtherSideCoefCalcExt;
                        } else {
                            surfTemp.ExtBoundCond = DataSurfaces::OtherSideCoefNoCalcExt;
                        }
                    }

                } else if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "Surface")) {
                    // it has to be another surface which needs to be found
                    // this will be found on the second pass through the surface input
                    // for flagging, set the value to UnreconciledZoneSurface
                    // name (ExtBoundCondName) will be validated later.
                    surfTemp.ExtBoundCond = unreconciledZoneSurface;
                    if (s_ipsc->lAlphaFieldBlanks(ArgPointer + 1)) {
                        surfTemp.ExtBoundCondName = surfTemp.Name;
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=<blank>.",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               s_ipsc->cAlphaFieldNames(ArgPointer + 1)));
                        ShowContinueError(state, format("..{}=\"Surface\" must be non-blank.", s_ipsc->cAlphaFieldNames(ArgPointer)));
                        ShowContinueError(state, "..This surface will become an adiabatic surface - no doors/windows allowed.");
                    }

                } else if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "Zone")) {
                    // This is the code for an unmatched "other surface"
                    // will be set up later.
                    surfTemp.ExtBoundCond = unenteredAdjacentZoneSurface;
                    // check OutsideFaceEnvironment for legal zone
                    Found = Util::FindItemInList(surfTemp.ExtBoundCondName, state.dataHeatBal->Zone, state.dataGlobal->NumOfZones);
                    ++NeedToAddSurfaces;

                    if (Found == 0) {
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}\".",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               s_ipsc->cAlphaFieldNames(ArgPointer),
                                               s_ipsc->cAlphaArgs(ArgPointer)));
                        ShowContinueError(state, "..Referenced as Zone for this surface.");
                        ErrorsFound = true;
                    }

                } else if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "Space")) {
                    // This is the code for an unmatched "other surface"
                    // will be set up later.
                    state.dataSurfaceGeometry->SurfaceTmp(SurfNum).ExtBoundCond = unenteredAdjacentSpaceSurface;
                    // check OutsideFaceEnvironment for legal zone
                    Found = Util::FindItemInList(
                        state.dataSurfaceGeometry->SurfaceTmp(SurfNum).ExtBoundCondName, state.dataHeatBal->space, state.dataGlobal->numSpaces);
                    ++NeedToAddSurfaces;

                    if (Found == 0) {
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}\".",
                                               s_ipsc->cCurrentModuleObject,
                                               state.dataSurfaceGeometry->SurfaceTmp(SurfNum).Name,
                                               s_ipsc->cAlphaFieldNames(ArgPointer),
                                               s_ipsc->cAlphaArgs(ArgPointer)));
                        ShowContinueError(state, "..Referenced as Space for this surface.");
                        ErrorsFound = true;
                    }

                } else if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "Foundation")) {

                    if (!state.dataWeather->WeatherFileExists) {
                        ShowSevereError(
                            state,
                            format("{}=\"{}\", using \"Foundation\" type Outside Boundary Condition requires specification of a weather file",
                                   s_ipsc->cCurrentModuleObject,
                                   surfTemp.Name));
                        ShowContinueError(state,
                                          "Either place in.epw in the working directory or specify a weather file on the command line using -w "
                                          "/path/to/weather.epw");
                        ErrorsFound = true;
                    }

                    // Find foundation object, if blank use default
                    if (s_ipsc->lAlphaFieldBlanks(ArgPointer + 1)) {

                        if (!state.dataSurfaceGeometry->kivaManager.defaultAdded) {
                            // Add default foundation if no other foundation object specified
                            state.dataSurfaceGeometry->kivaManager.addDefaultFoundation();
                        }
                        surfTemp.OSCPtr =
                            state.dataSurfaceGeometry->kivaManager.defaultIndex; // Reuse OSC pointer...shouldn't be used for non OSC surfaces anyway.
                    } else {
                        Found = state.dataSurfaceGeometry->kivaManager.findFoundation(surfTemp.ExtBoundCondName);
                        if (Found != (int)state.dataSurfaceGeometry->kivaManager.foundationInputs.size()) {
                            surfTemp.OSCPtr = Found;
                        } else {
                            ShowSevereError(state,
                                            format("{}=\"{}\", invalid {}=\"{}\".",
                                                   s_ipsc->cCurrentModuleObject,
                                                   surfTemp.Name,
                                                   s_ipsc->cAlphaFieldNames(ArgPointer + 1),
                                                   s_ipsc->cAlphaArgs(ArgPointer + 1)));
                            ErrorsFound = true;
                        }
                    }

                    if (state.dataConstruction->Construct(surfTemp.Construction).SourceSinkPresent) {
                        ShowSevereError(
                            state,
                            format("{}=\"{}\", construction may not have an internal source/sink", s_ipsc->cCurrentModuleObject, surfTemp.Name));
                        ErrorsFound = true;
                    }
                    surfTemp.ExtBoundCond = DataSurfaces::KivaFoundation;
                } else if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "OtherSideConditionsModel")) {
                    Found = Util::FindItemInList(surfTemp.ExtBoundCondName, state.dataSurface->OSCM, state.dataSurface->TotOSCM);
                    if (Found == 0) {
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}\".",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               s_ipsc->cAlphaFieldNames(ArgPointer + 1),
                                               s_ipsc->cAlphaArgs(ArgPointer + 1)));
                        ErrorsFound = true;
                    }
                    surfTemp.OSCMPtr = Found;
                    surfTemp.ExtBoundCond = DataSurfaces::OtherSideCondModeledExt;

                } else if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "GroundSlabPreprocessorAverage") ||
                           Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "GroundSlabPreprocessorCore") ||
                           Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "GroundSlabPreprocessorPerimeter") ||
                           Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "GroundBasementPreprocessorAverageFloor") ||
                           Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "GroundBasementPreprocessorAverageWall") ||
                           Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "GroundBasementPreprocessorUpperWall") ||
                           Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "GroundBasementPreprocessorLowerWall")) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\".",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(ArgPointer),
                                           s_ipsc->cAlphaArgs(ArgPointer)));
                    ShowContinueError(state, "The ExpandObjects program has not been run or is not in your EnergyPlus.exe folder.");
                    ErrorsFound = true;

                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\".",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(ArgPointer),
                                           s_ipsc->cAlphaArgs(ArgPointer)));
                    ShowContinueError(state,
                                      "Should be one of \"Outdoors\", \"Adiabatic\", Ground\", \"Surface\", \"OtherSideCoefficients\", "
                                      "\"OtherSideConditionsModel\" or \"Zone\"");
                    ErrorsFound = true;
                } // ... End of the ExtBoundCond logical IF Block

                ArgPointer += 2;
                // Set the logical flag for the exterior solar
                if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "SunExposed")) {
                    if ((surfTemp.ExtBoundCond != DataSurfaces::ExternalEnvironment) &&
                        (surfTemp.ExtBoundCond != DataSurfaces::OtherSideCondModeledExt)) {
                        ShowWarningError(state,
                                         format("{}=\"{}\", {}=\"{}\".",
                                                s_ipsc->cCurrentModuleObject,
                                                surfTemp.Name,
                                                s_ipsc->cAlphaFieldNames(ArgPointer),
                                                s_ipsc->cAlphaArgs(ArgPointer)));
                        ShowContinueError(state, "..This surface is not exposed to External Environment.  Sun exposure has no effect.");
                    } else {
                        surfTemp.ExtSolar = true;
                    }
                } else if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "NoSun")) {
                    surfTemp.ExtSolar = false;
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\".",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(ArgPointer),
                                           s_ipsc->cAlphaArgs(ArgPointer)));
                    ErrorsFound = true;
                }

                ++ArgPointer;
                // Set the logical flag for the exterior wind
                if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "WindExposed")) {
                    surfTemp.ExtWind = true;
                } else if (Util::SameString(s_ipsc->cAlphaArgs(ArgPointer), "NoWind")) {
                    surfTemp.ExtWind = false;
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\".",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(ArgPointer),
                                           s_ipsc->cAlphaArgs(ArgPointer)));
                    ErrorsFound = true;
                }

                // Set the logical flag for the EcoRoof presented, this is only based on the flag in the construction type
                //                if (surfTemp.Construction > 0)
                //                    surfTemp.ExtEcoRoof =
                //                        state.dataConstruction->Construct(surfTemp.Construction).TypeIsEcoRoof;

                surfTemp.ViewFactorGround = s_ipsc->rNumericArgs(1);
                if (s_ipsc->lNumericFieldBlanks(1)) {
                    surfTemp.ViewFactorGround = Constant::AutoCalculate;
                }
                if (s_ipsc->lNumericFieldBlanks(2) || s_ipsc->rNumericArgs(2) == Constant::AutoCalculate) {
                    numSides = (SurfaceNumProp - 2) / 3;
                    surfTemp.Sides = numSides;
                    if (mod(SurfaceNumProp - 2, 3) != 0) {
                        ShowWarningError(state,
                                         format("{}=\"{}\", {}",
                                                s_ipsc->cCurrentModuleObject,
                                                surfTemp.Name,
                                                format("{} not even multiple of 3. Will read in {}", s_ipsc->cNumericFieldNames(2), surfTemp.Sides)));
                    }
                    if (numSides < 3) {
                        ShowSevereError(state,
                                        format("{}=\"{}\", {} (autocalculate) must be >= 3. Only {} provided.",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               s_ipsc->cNumericFieldNames(2),
                                               surfTemp.Sides));
                        ErrorsFound = true;
                        continue;
                    }
                } else {
                    numSides = (SurfaceNumProp - 2) / 3;
                    surfTemp.Sides = s_ipsc->rNumericArgs(2);
                    if (numSides > surfTemp.Sides) {
                        ShowWarningError(state,
                                         format("{}=\"{}\", field {}={}",
                                                s_ipsc->cCurrentModuleObject,
                                                surfTemp.Name,
                                                s_ipsc->cNumericFieldNames(2),
                                                fmt::to_string(surfTemp.Sides)));
                        ShowContinueError(
                            state, format("...but {} were entered. Only the indicated {} will be used.", numSides, s_ipsc->cNumericFieldNames(2)));
                    }
                }
                surfTemp.Vertex.allocate(surfTemp.Sides);
                surfTemp.NewVertex.allocate(surfTemp.Sides);
                GetVertices(state, SurfNum, surfTemp.Sides, s_ipsc->rNumericArgs({3, _}));
                if (surfTemp.Area <= 0.0) {
                    ShowSevereError(
                        state,
                        format("{}=\"{}\", Surface Area <= 0.0; Entered Area={:.2T}", s_ipsc->cCurrentModuleObject, surfTemp.Name, surfTemp.Area));
                    ErrorsFound = true;
                }

                CheckConvexity(state, SurfNum, surfTemp.Sides);
                if (Util::SameString(s_ipsc->cAlphaArgs(5), "Surface")) {
                    if (surfTemp.Sides != static_cast<int>(surfTemp.Vertex.size())) {
                        ShowSevereError(state,
                                        format("{}=\"{}\", After CheckConvexity, mismatch between Sides ({}) and size of Vertex ({}).",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               surfTemp.Sides,
                                               surfTemp.Vertex.size()));
                        ShowContinueError(state, "CheckConvexity is used to verify the convexity of a surface and detect collinear points.");
                        ErrorsFound = true;
                    }
                }
                if (surfTemp.Construction > 0) {
                    // Check wall height for the CFactor walls

                    if (surfTemp.Class == SurfaceClass::Wall && state.dataConstruction->Construct(surfTemp.Construction).TypeIsCfactorWall) {
                        if (std::abs(surfTemp.Height - state.dataConstruction->Construct(surfTemp.Construction).Height) > 0.05) {
                            ShowWarningError(
                                state,
                                format("{}=\"{}\", underground Wall Height = {:.2T}", s_ipsc->cCurrentModuleObject, surfTemp.Name, surfTemp.Height));
                            ShowContinueError(state, "..which does not match its construction height.");
                        }
                    }

                    // Check area and perimeter for the FFactor floors
                    if (surfTemp.Class == SurfaceClass::Floor && state.dataConstruction->Construct(surfTemp.Construction).TypeIsFfactorFloor) {
                        if (std::abs(surfTemp.Area - state.dataConstruction->Construct(surfTemp.Construction).Area) > 0.1) {
                            ShowWarningError(
                                state,
                                format("{}=\"{}\", underground Floor Area = {:.2T}", s_ipsc->cCurrentModuleObject, surfTemp.Name, surfTemp.Area));
                            ShowContinueError(state, "..which does not match its construction area.");
                        }
                        if (surfTemp.Perimeter < state.dataConstruction->Construct(surfTemp.Construction).PerimeterExposed - 0.1) {
                            ShowWarningError(state,
                                             format("{}=\"{}\", underground Floor Perimeter = {:.2T}",
                                                    s_ipsc->cCurrentModuleObject,
                                                    surfTemp.Name,
                                                    surfTemp.Perimeter));
                            ShowContinueError(state, "..which is less than its construction exposed perimeter.");
                        }
                    }
                }
                // Not sure if it's better to add this or guard in SolarShading.cc
                // surfTemp.shadowSurfSched = nullptr
            }
        } // Item Looop
        // Check number of Vertex between base surface and Outside Boundary surface
        int ExtSurfNum;
        for (int i = 1; i <= SurfNum; i++) {
            if (state.dataSurfaceGeometry->SurfaceTmp(i).ExtBoundCond == unreconciledZoneSurface &&
                state.dataSurfaceGeometry->SurfaceTmp(i).ExtBoundCondName != "") {
                ExtSurfNum = Util::FindItemInList(state.dataSurfaceGeometry->SurfaceTmp(i).ExtBoundCondName, state.dataSurfaceGeometry->SurfaceTmp);
                // If we cannot find the referenced surface
                if (ExtSurfNum == 0) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" references an outside boundary surface that cannot be found:{}",
                                           s_ipsc->cCurrentModuleObject,
                                           state.dataSurfaceGeometry->SurfaceTmp(i).Name,
                                           state.dataSurfaceGeometry->SurfaceTmp(i).ExtBoundCondName));
                    ErrorsFound = true;
                    // If vertex size mismatch
                } else if (state.dataSurfaceGeometry->SurfaceTmp(i).Vertex.size() !=
                           state.dataSurfaceGeometry->SurfaceTmp(ExtSurfNum).Vertex.size()) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", Vertex size mismatch between base surface :{} and outside boundary surface: {}",
                                           s_ipsc->cCurrentModuleObject,
                                           state.dataSurfaceGeometry->SurfaceTmp(i).Name,
                                           state.dataSurfaceGeometry->SurfaceTmp(i).Name,
                                           state.dataSurfaceGeometry->SurfaceTmp(ExtSurfNum).Name));
                    ShowContinueError(state,
                                      format("The vertex sizes are {} for base surface and {} for outside boundary surface. Please check inputs.",
                                             state.dataSurfaceGeometry->SurfaceTmp(i).Vertex.size(),
                                             state.dataSurfaceGeometry->SurfaceTmp(ExtSurfNum).Vertex.size()));
                    ErrorsFound = true;
                }
            }
        }
    }

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
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   December 2008

        // PURPOSE OF THIS SUBROUTINE:
        // Get simple (rectangular, LLC corner specified) walls

        // SUBROUTINE PARAMETER DEFINITIONS:
        static Array1D_string const cModuleObjects(10,
                                                   {"Wall:Exterior",
                                                    "Wall:Adiabatic",
                                                    "Wall:Interzone",
                                                    "Wall:Underground",
                                                    "Roof",
                                                    "Ceiling:Adiabatic",
                                                    "Ceiling:Interzone",
                                                    "Floor:GroundContact",
                                                    "Floor:Adiabatic",
                                                    "Floor:Interzone"});

        int ItemsToGet;
        int NumAlphas;
        int NumNumbers;
        int IOStat; // IO Status when calling get input subroutine
        int Found;  // For matching base surfaces
        bool GettingIZSurfaces;
        int OtherSurfaceField;
        int ExtBoundCondition;
        int ClassItem;
        int ZoneNum;

        auto &s_ipsc = state.dataIPShortCut;

        for (int Item = 1; Item <= 10; ++Item) {

            s_ipsc->cCurrentModuleObject = cModuleObjects(Item);
            if (Item == 1) {
                ItemsToGet = TotRectExtWalls;
                GettingIZSurfaces = false;
                OtherSurfaceField = 0;
                ExtBoundCondition = DataSurfaces::ExternalEnvironment;
                ClassItem = 1;
            } else if (Item == 2) {
                ItemsToGet = TotRectIntWalls;
                GettingIZSurfaces = false;
                OtherSurfaceField = 0;
                ExtBoundCondition = unreconciledZoneSurface;
                ClassItem = 1;
            } else if (Item == 3) {
                ItemsToGet = TotRectIZWalls;
                GettingIZSurfaces = true;
                OtherSurfaceField = 5;
                ExtBoundCondition = unreconciledZoneSurface;
                ClassItem = 1;
            } else if (Item == 4) {
                ItemsToGet = TotRectUGWalls;
                GettingIZSurfaces = false;
                OtherSurfaceField = 0;
                ExtBoundCondition = DataSurfaces::Ground;
                ClassItem = 1;
            } else if (Item == 5) {
                ItemsToGet = TotRectRoofs;
                GettingIZSurfaces = false;
                OtherSurfaceField = 0;
                ExtBoundCondition = DataSurfaces::ExternalEnvironment;
                ClassItem = 3;
            } else if (Item == 6) {
                ItemsToGet = TotRectCeilings;
                GettingIZSurfaces = false;
                OtherSurfaceField = 0;
                ExtBoundCondition = unreconciledZoneSurface;
                ClassItem = 3;
            } else if (Item == 7) {
                ItemsToGet = TotRectIZCeilings;
                GettingIZSurfaces = false;
                OtherSurfaceField = 5;
                ExtBoundCondition = unreconciledZoneSurface;
                ClassItem = 3;
            } else if (Item == 8) {
                ItemsToGet = TotRectGCFloors;
                GettingIZSurfaces = false;
                OtherSurfaceField = 0;
                ExtBoundCondition = DataSurfaces::Ground;
                ClassItem = 2;
            } else if (Item == 9) {
                ItemsToGet = TotRectIntFloors;
                GettingIZSurfaces = false;
                OtherSurfaceField = 0;
                ExtBoundCondition = unreconciledZoneSurface;
                ClassItem = 2;
            } else { // IF (Item == 10) THEN
                ItemsToGet = TotRectIZFloors;
                GettingIZSurfaces = true;
                OtherSurfaceField = 5;
                ExtBoundCondition = unreconciledZoneSurface;
                ClassItem = 2;
            }

            for (int Loop = 1; Loop <= ItemsToGet; ++Loop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         s_ipsc->cCurrentModuleObject,
                                                                         Loop,
                                                                         s_ipsc->cAlphaArgs,
                                                                         NumAlphas,
                                                                         s_ipsc->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStat,
                                                                         s_ipsc->lNumericFieldBlanks,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);

                if (GlobalNames::VerifyUniqueInterObjectName(state,
                                                             state.dataSurfaceGeometry->UniqueSurfaceNames,
                                                             s_ipsc->cAlphaArgs(1),
                                                             s_ipsc->cCurrentModuleObject,
                                                             s_ipsc->cAlphaFieldNames(1),
                                                             ErrorsFound)) {
                    continue;
                }

                if (NumNumbers < 7) {
                    ShowSevereError(
                        state,
                        format("{}=\"{}\", Too few number of numeric args=[{}].", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), NumNumbers));
                    ErrorsFound = true;
                }

                ++SurfNum;

                auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
                surfTemp.Name = s_ipsc->cAlphaArgs(1);   // Set the Surface Name in the Derived Type
                surfTemp.Class = BaseSurfIDs(ClassItem); // Set class number

                surfTemp.Construction =
                    Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataConstruction->Construct, state.dataHeatBal->TotConstructs);

                if (surfTemp.Construction == 0) {
                    ErrorsFound = true;
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\".",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                } else if (state.dataConstruction->Construct(surfTemp.Construction).TypeIsWindow) {
                    ErrorsFound = true;
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\" - has Window materials.",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(3),
                                           s_ipsc->cAlphaArgs(2)));
                    ShowContinueError(state, format("...because {}={}", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
                } else {
                    state.dataConstruction->Construct(surfTemp.Construction).IsUsed = true;
                    surfTemp.ConstructionStoredInputValue = surfTemp.Construction;
                }
                surfTemp.HeatTransSurf = true;
                surfTemp.BaseSurf = SurfNum;
                surfTemp.BaseSurfName = surfTemp.Name;

                surfTemp.ZoneName = s_ipsc->cAlphaArgs(3);
                ZoneNum = Util::FindItemInList(surfTemp.ZoneName, state.dataHeatBal->Zone, state.dataGlobal->NumOfZones);

                if (ZoneNum != 0) {
                    surfTemp.Zone = ZoneNum;
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\".",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(3),
                                           s_ipsc->cAlphaArgs(3)));
                    surfTemp.Class = SurfaceClass::Invalid;
                    surfTemp.ZoneName = "Unknown Zone";
                    ErrorsFound = true;
                }

                if (!s_ipsc->lAlphaFieldBlanks(4)) {
                    int spaceNum = Util::FindItemInList(s_ipsc->cAlphaArgs(4), state.dataHeatBal->space);

                    if (spaceNum != 0) {
                        surfTemp.spaceNum = spaceNum;
                    } else {
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}\".",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               s_ipsc->cAlphaFieldNames(4),
                                               s_ipsc->cAlphaArgs(4)));
                        surfTemp.Class = SurfaceClass::Invalid;
                        ErrorsFound = true;
                    }
                }

                surfTemp.ExtBoundCond = ExtBoundCondition;
                if (surfTemp.Construction > 0) {
                    if (surfTemp.Class == SurfaceClass::Wall && state.dataConstruction->Construct(surfTemp.Construction).TypeIsCfactorWall &&
                        surfTemp.ExtBoundCond == DataSurfaces::Ground) {
                        surfTemp.ExtBoundCond = DataSurfaces::GroundFCfactorMethod;
                    } else if (state.dataConstruction->Construct(surfTemp.Construction).TypeIsCfactorWall) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", Construction type is \"Construction:CfactorUndergroundWall\" but invalid for this object.",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name));
                    }

                    if (surfTemp.Class == SurfaceClass::Floor && state.dataConstruction->Construct(surfTemp.Construction).TypeIsFfactorFloor &&
                        surfTemp.ExtBoundCond == DataSurfaces::Ground) {
                        surfTemp.ExtBoundCond = DataSurfaces::GroundFCfactorMethod;
                    } else if (state.dataConstruction->Construct(surfTemp.Construction).TypeIsFfactorFloor) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", Construction type is \"Construction:FfactorGroundFloor\" but invalid for this object.",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name));
                    }
                }
                surfTemp.ExtSolar = false;
                surfTemp.ExtWind = false;
                surfTemp.ViewFactorGround = Constant::AutoCalculate;

                if (surfTemp.ExtBoundCond == DataSurfaces::ExternalEnvironment) {
                    surfTemp.ExtSolar = true;
                    surfTemp.ExtWind = true;

                    // Set the logical flag for the EcoRoof presented, this is only based on the flag in the construction type
                    //                    if (surfTemp.Construction > 0)
                    //                        surfTemp.ExtEcoRoof =
                    //                            state.dataConstruction->Construct(surfTemp.Construction).TypeIsEcoRoof;

                } else if (surfTemp.ExtBoundCond == unreconciledZoneSurface) {
                    if (GettingIZSurfaces) {
                        surfTemp.ExtBoundCondName = s_ipsc->cAlphaArgs(OtherSurfaceField);
                        Found = Util::FindItemInList(surfTemp.ExtBoundCondName, state.dataHeatBal->Zone, state.dataGlobal->NumOfZones);
                        // see if match to zone, then it's an unentered other surface, else reconciled later
                        if (Found > 0) {
                            ++NeedToAddSurfaces;
                            surfTemp.ExtBoundCond = unenteredAdjacentZoneSurface;
                        }
                    } else {
                        surfTemp.ExtBoundCondName = surfTemp.Name;
                    }

                } else if (surfTemp.ExtBoundCond == DataSurfaces::Ground) {
                    if (state.dataSurfaceGeometry->NoGroundTempObjWarning) {
                        if (!state.dataEnvrn->GroundTempInputs[(int)DataEnvironment::GroundTempType::BuildingSurface]) {
                            ShowWarningError(state,
                                             "GetRectSurfaces: Surfaces with interface to Ground found but no \"Ground Temperatures\" were input.");
                            ShowContinueError(state, format("Found first in surface={}", s_ipsc->cAlphaArgs(1)));
                            ShowContinueError(state,
                                              format("Defaults, constant throughout the year of ({:.1R}) will be used.",
                                                     state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface]));
                        }
                        state.dataSurfaceGeometry->NoGroundTempObjWarning = false;
                    }

                } else if (surfTemp.ExtBoundCond == DataSurfaces::GroundFCfactorMethod) {
                    if (state.dataSurfaceGeometry->NoFCGroundTempObjWarning) {
                        if (!state.dataEnvrn->GroundTempInputs[(int)DataEnvironment::GroundTempType::FCFactorMethod]) {
                            ShowSevereError(state,
                                            "GetRectSurfaces: Surfaces with interface to GroundFCfactorMethod found but no \"FC Ground "
                                            "Temperatures\" were input.");
                            ShowContinueError(state, format("Found first in surface={}", s_ipsc->cAlphaArgs(1)));
                            ShowContinueError(state,
                                              "Either add a \"Site:GroundTemperature:FCfactorMethod\" object or use a weather file with "
                                              "Ground Temperatures.");
                            ErrorsFound = true;
                            state.dataSurfaceGeometry->NoFCGroundTempObjWarning = false;
                        }
                    }

                } // ... End of the ExtBoundCond logical IF Block

                surfTemp.Azimuth = s_ipsc->rNumericArgs(1);
                surfTemp.Tilt = s_ipsc->rNumericArgs(2);
                surfTemp.convOrientation = Convect::GetSurfConvOrientation(surfTemp.Tilt);
                if (!state.dataSurface->WorldCoordSystem) {
                    if (ZoneNum != 0) {
                        surfTemp.Azimuth += state.dataHeatBal->BuildingAzimuth + state.dataHeatBal->Zone(ZoneNum).RelNorth;
                    }
                }
                if (ZoneNum != 0) {
                    surfTemp.Azimuth += state.dataHeatBal->BuildingRotationAppendixG;
                }

                surfTemp.Sides = 4;
                surfTemp.Vertex.allocate(surfTemp.Sides);

                MakeRectangularVertices(state,
                                        SurfNum,
                                        s_ipsc->rNumericArgs(3),
                                        s_ipsc->rNumericArgs(4),
                                        s_ipsc->rNumericArgs(5),
                                        s_ipsc->rNumericArgs(6),
                                        s_ipsc->rNumericArgs(7),
                                        state.dataSurfaceGeometry->RectSurfRefWorldCoordSystem);

                if (surfTemp.Area <= 0.0) {
                    ShowSevereError(
                        state,
                        format("{}=\"{}\", Surface Area <= 0.0; Entered Area={:.2T}", s_ipsc->cCurrentModuleObject, surfTemp.Name, surfTemp.Area));
                    ErrorsFound = true;
                }

                // Check wall height for the CFactor walls
                if (surfTemp.Class == SurfaceClass::Wall && surfTemp.ExtBoundCond == DataSurfaces::GroundFCfactorMethod) {
                    if (std::abs(surfTemp.Height - state.dataConstruction->Construct(surfTemp.Construction).Height) > 0.05) {
                        ShowWarningError(
                            state,
                            format("{}=\"{}\", underground Wall Height = {:.2T}", s_ipsc->cCurrentModuleObject, surfTemp.Name, surfTemp.Height));
                        ShowContinueError(state, "..which deos not match its construction height.");
                    }
                }

                // Check area and perimeter for the FFactor floors
                if (surfTemp.Class == SurfaceClass::Floor && surfTemp.ExtBoundCond == DataSurfaces::GroundFCfactorMethod) {
                    if (std::abs(surfTemp.Area - state.dataConstruction->Construct(surfTemp.Construction).Area) > 0.1) {
                        ShowWarningError(
                            state, format("{}=\"{}\", underground Floor Area = {:.2T}", s_ipsc->cCurrentModuleObject, surfTemp.Name, surfTemp.Area));
                        ShowContinueError(state, "..which does not match its construction area.");
                    }
                    if (surfTemp.Perimeter < state.dataConstruction->Construct(surfTemp.Construction).PerimeterExposed - 0.1) {
                        ShowWarningError(
                            state,
                            format(
                                "{}=\"{}\", underground Floor Perimeter = {:.2T}", s_ipsc->cCurrentModuleObject, surfTemp.Name, surfTemp.Perimeter));
                        ShowContinueError(state, "..which is less than its construction exposed perimeter.");
                    }
                }
            } // Getting Items
        }
    }

    void MakeRectangularVertices(EnergyPlusData &state,
                                 int const SurfNum,
                                 Real64 const XCoord,
                                 Real64 const YCoord,
                                 Real64 const ZCoord,
                                 Real64 const Length,
                                 Real64 const Height,
                                 bool const SurfWorldCoordSystem)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   December 2008

        // PURPOSE OF THIS SUBROUTINE:
        // This routine creates world/3d coordinates for rectangular surfaces using azimuth, tilt, LLC (X,Y,Z), length & height.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 XLLC;
        Real64 YLLC;
        Real64 ZLLC;
        Array1D<Real64> XX(4);
        Array1D<Real64> YY(4);
        Real64 Xb;
        Real64 Yb;
        Real64 Perimeter;
        int Vrt;

        auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);

        if (surfTemp.Zone == 0 && (surfTemp.Class != SurfaceClass::Detached_F && surfTemp.Class != SurfaceClass::Detached_B)) {
            return;
        }

        surfTemp.Height = Height;
        surfTemp.Width = Length;

        Real64 SurfAzimuth = surfTemp.Azimuth;
        Real64 SurfTilt = surfTemp.Tilt;
        Real64 CosSurfAzimuth = std::cos(SurfAzimuth * Constant::DegToRad);
        Real64 SinSurfAzimuth = std::sin(SurfAzimuth * Constant::DegToRad);
        Real64 CosSurfTilt = std::cos(SurfTilt * Constant::DegToRad);
        Real64 SinSurfTilt = std::sin(SurfTilt * Constant::DegToRad);

        if (!SurfWorldCoordSystem) {
            if (surfTemp.Zone > 0) {
                Xb = XCoord * state.dataSurfaceGeometry->CosZoneRelNorth(surfTemp.Zone) -
                     YCoord * state.dataSurfaceGeometry->SinZoneRelNorth(surfTemp.Zone) + state.dataHeatBal->Zone(surfTemp.Zone).OriginX;
                Yb = XCoord * state.dataSurfaceGeometry->SinZoneRelNorth(surfTemp.Zone) +
                     YCoord * state.dataSurfaceGeometry->CosZoneRelNorth(surfTemp.Zone) + state.dataHeatBal->Zone(surfTemp.Zone).OriginY;
                XLLC = Xb * state.dataSurfaceGeometry->CosBldgRelNorth - Yb * state.dataSurfaceGeometry->SinBldgRelNorth;
                YLLC = Xb * state.dataSurfaceGeometry->SinBldgRelNorth + Yb * state.dataSurfaceGeometry->CosBldgRelNorth;
                ZLLC = ZCoord + state.dataHeatBal->Zone(surfTemp.Zone).OriginZ;
            } else {
                if (surfTemp.Class == SurfaceClass::Detached_B) {
                    Xb = XCoord;
                    Yb = YCoord;
                    XLLC = Xb * state.dataSurfaceGeometry->CosBldgRelNorth - Yb * state.dataSurfaceGeometry->SinBldgRelNorth;
                    YLLC = Xb * state.dataSurfaceGeometry->SinBldgRelNorth + Yb * state.dataSurfaceGeometry->CosBldgRelNorth;
                    ZLLC = ZCoord;
                } else {
                    XLLC = XCoord;
                    YLLC = YCoord;
                    ZLLC = ZCoord;
                }
            }
        } else {
            // for world coordinates, only rotate for appendix G
            Xb = XCoord;
            Yb = YCoord;
            ZLLC = ZCoord;
            if (surfTemp.Class != SurfaceClass::Detached_F) {
                XLLC = Xb * state.dataSurfaceGeometry->CosBldgRotAppGonly - Yb * state.dataSurfaceGeometry->SinBldgRotAppGonly;
                YLLC = Xb * state.dataSurfaceGeometry->SinBldgRotAppGonly + Yb * state.dataSurfaceGeometry->CosBldgRotAppGonly;
            } else {
                XLLC = Xb;
                YLLC = Yb;
            }
        }

        XX(1) = 0.0;
        XX(2) = 0.0;
        XX(3) = Length;
        XX(4) = Length;
        YY(1) = Height;
        YY(4) = Height;
        YY(3) = 0.0;
        YY(2) = 0.0;

        for (int n = 1; n <= surfTemp.Sides; ++n) {
            Vrt = n;
            surfTemp.Vertex(Vrt).x = XLLC - XX(n) * CosSurfAzimuth - YY(n) * CosSurfTilt * SinSurfAzimuth;
            surfTemp.Vertex(Vrt).y = YLLC + XX(n) * SinSurfAzimuth - YY(n) * CosSurfTilt * CosSurfAzimuth;
            surfTemp.Vertex(Vrt).z = ZLLC + YY(n) * SinSurfTilt;
        }

        Vectors::CreateNewellAreaVector(surfTemp.Vertex, surfTemp.Sides, surfTemp.NewellAreaVector);
        surfTemp.GrossArea = Vectors::VecLength(surfTemp.NewellAreaVector);
        surfTemp.Area = surfTemp.GrossArea;
        surfTemp.NetAreaShadowCalc = surfTemp.Area;
        Vectors::CreateNewellSurfaceNormalVector(surfTemp.Vertex, surfTemp.Sides, surfTemp.NewellSurfaceNormalVector);
        Vectors::DetermineAzimuthAndTilt(
            surfTemp.Vertex, SurfAzimuth, SurfTilt, surfTemp.lcsx, surfTemp.lcsy, surfTemp.lcsz, surfTemp.NewellSurfaceNormalVector);
        surfTemp.Azimuth = SurfAzimuth;
        surfTemp.Tilt = SurfTilt;
        surfTemp.convOrientation = Convect::GetSurfConvOrientation(surfTemp.Tilt);
        // Sine and cosine of azimuth and tilt
        surfTemp.SinAzim = SinSurfAzimuth;
        surfTemp.CosAzim = CosSurfAzimuth;
        surfTemp.SinTilt = SinSurfTilt;
        surfTemp.CosTilt = CosSurfTilt;
        surfTemp.ViewFactorGround = 0.5 * (1.0 - surfTemp.CosTilt);
        // Outward normal unit vector (pointing away from room)

        surfTemp.OutNormVec = surfTemp.NewellSurfaceNormalVector;
        for (int n = 1; n <= 3; ++n) {
            if (std::abs(surfTemp.OutNormVec(n) - 1.0) < 1.e-06) {
                surfTemp.OutNormVec(n) = +1.0;
            }
            if (std::abs(surfTemp.OutNormVec(n) + 1.0) < 1.e-06) {
                surfTemp.OutNormVec(n) = -1.0;
            }
            if (std::abs(surfTemp.OutNormVec(n)) < 1.e-06) {
                surfTemp.OutNormVec(n) = 0.0;
            }
        }

        // Can perform tests on this surface here
        surfTemp.ViewFactorSky = 0.5 * (1.0 + surfTemp.CosTilt);
        // The following IR view factors are modified in subr. SkyDifSolarShading if there are shadowing
        // surfaces
        surfTemp.ViewFactorSkyIR = surfTemp.ViewFactorSky;
        surfTemp.ViewFactorGroundIR = 0.5 * (1.0 - surfTemp.CosTilt);

        Perimeter = distance(surfTemp.Vertex(surfTemp.Sides), surfTemp.Vertex(1));
        for (Vrt = 2; Vrt <= surfTemp.Sides; ++Vrt) {
            Perimeter += distance(surfTemp.Vertex(Vrt), surfTemp.Vertex(Vrt - 1));
        }
        surfTemp.Perimeter = Perimeter;

        // Call to transform vertices

        TransformVertsByAspect(state, SurfNum, surfTemp.Sides);
    }

    void GetHTSubSurfaceData(EnergyPlusData &state,
                             bool &ErrorsFound,                       // Error flag indicator (true if errors found)
                             int &SurfNum,                            // Count of Current SurfaceNumber
                             int const TotHTSubs,                     // Number of Heat Transfer SubSurfaces to obtain
                             const Array1D_string &SubSurfCls,        // Valid Classes for Sub Surfaces
                             const Array1D<SurfaceClass> &SubSurfIDs, // ID Assignments for valid sub surface classes
                             int &AddedSubSurfaces,                   // Subsurfaces added when windows reference Window5
                             int &NeedToAddSurfaces                   // Number of surfaces to add, based on unentered IZ surfaces
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   May 2000
        //       MODIFIED       August 2012 - line up subsurfaces with base surface types

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the HeatTransfer Sub Surface Data, checks it for errors, etc.

        // REFERENCES:
        // Heat Transfer Subsurface Definition
        // FenestrationSurface:Detailed,
        //        \min-fields 19
        //        \memo Used for windows, doors, glass doors, tubular daylighting devices
        //        \format vertices
        //   A1 , \field Name
        //        \required-field
        //        \type alpha
        //   A2 , \field Surface Type
        //        \required-field
        //        \type choice
        //        \key Window
        //        \key Door
        //        \key GlassDoor
        //        \key TubularDaylightDome
        //        \key TubularDaylightDiffuser
        //   A3 , \field Construction Name
        //        \required-field
        //        \note To be matched with a construction in this input file
        //        \type object-list
        //        \object-list ConstructionNames
        //   A4 , \field Building Surface Name
        //        \required-field
        //        \type object-list
        //        \object-list SurfaceNames
        //   A5,  \field Outside Boundary Condition Object
        //        \type object-list
        //        \object-list OutFaceEnvNames
        //        \note Non-blank only if base surface field Outside Boundary Condition is
        //        \note Surface or OtherSideCoefficients
        //        \note If Base Surface's Surface, specify name of corresponding subsurface in adjacent zone or
        //        \note specify current subsurface name for internal partition separating like zones
        //        \note If OtherSideCoefficients, specify name of SurfaceProperty:OtherSideCoefficients
        //        \note  or leave blank to inherit Base Surface's OtherSide Coefficients
        //   N1, \field View Factor to Ground
        //        \type real
        //        \note From the exterior of the surface
        //        \note Unused if one uses the "reflections" options in Solar Distribution in Building input
        //        \note unless a DaylightingDevice:Shelf or DaylightingDevice:Tubular object has been specified.
        //        \note autocalculate will automatically calculate this value from the tilt of the surface
        //        \autocalculatable
        //        \minimum 0.0
        //        \maximum 1.0
        //        \default autocalculate
        //   A6, \field Frame and Divider Name
        //        \note Enter the name of a WindowProperty:FrameAndDivider object
        //        \type object-list
        //        \object-list WindowFrameAndDividerNames
        //        \note Used only for exterior windows (rectangular) and glass doors.
        //        \note Unused for triangular windows.
        //        \note If not specified (blank), window or glass door has no frame or divider
        //        \note and no beam solar reflection from reveal surfaces.
        //   N2 , \field Multiplier
        //        \note Used only for Surface Type = WINDOW, GLASSDOOR or DOOR
        //        \note Non-integer values will be truncated to integer
        //        \default 1.0
        //        \minimum 1.0
        //   N3 , \field Number of Vertices
        //        \minimum 3
        //        \maximum 4
        //        \autocalculatable
        //        \default autocalculate
        //        \note vertices are given in GlobalGeometryRules coordinates -- if relative, all surface coordinates
        //        \note are "relative" to the Zone Origin.  If world, then building and zone origins are used
        //        \note for some internal calculations, but all coordinates are given in an "absolute" system.
        //  N4-15 as indicated by the N3 value

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int IOStat;          // IO Status when calling get input subroutine
        int SurfaceNumAlpha; // Number of material alpha names being passed
        int SurfaceNumProp;  // Number of material properties being passed
        int Found;           // For matching interzone surfaces
        int Loop;
        int ValidChk;
        int numSides;

        auto &s_ipsc = state.dataIPShortCut;

        GetWindowShadingControlData(state, ErrorsFound);
        s_ipsc->cCurrentModuleObject = "FenestrationSurface:Detailed";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, s_ipsc->cCurrentModuleObject, Loop, SurfaceNumAlpha, SurfaceNumProp);

        if (SurfaceNumAlpha != 6) {
            ShowSevereError(
                state,
                format("{}: Object Definition indicates not = 6 Alpha Objects, Number Indicated={}", s_ipsc->cCurrentModuleObject, SurfaceNumAlpha));
            ErrorsFound = true;
        }

        if (SurfaceNumProp != 15) {
            ShowSevereError(
                state,
                format("{}: Object Definition indicates > 15 Numeric Objects, Number Indicated={}", s_ipsc->cCurrentModuleObject, SurfaceNumAlpha));
            ErrorsFound = true;
        }
        NeedToAddSurfaces = 0;

        for (Loop = 1; Loop <= TotHTSubs; ++Loop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     s_ipsc->cAlphaArgs,
                                                                     SurfaceNumAlpha,
                                                                     s_ipsc->rNumericArgs,
                                                                     SurfaceNumProp,
                                                                     IOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            if (GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataSurfaceGeometry->UniqueSurfaceNames,
                                                         s_ipsc->cAlphaArgs(1),
                                                         s_ipsc->cCurrentModuleObject,
                                                         s_ipsc->cAlphaFieldNames(1),
                                                         ErrorsFound)) {
                continue;
            }

            ++SurfNum;

            auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);

            if (SurfaceNumProp < 12) {
                ShowSevereError(
                    state, format("{}=\"{}\", Too few number of numeric args=[{}].", s_ipsc->cCurrentModuleObject, surfTemp.Name, SurfaceNumProp));
                ErrorsFound = true;
            }

            surfTemp.Name = s_ipsc->cAlphaArgs(1); // Set the Surface Name in the Derived Type
            ValidChk = Util::FindItemInList(s_ipsc->cAlphaArgs(2), SubSurfCls, 6);
            if (ValidChk == 0) {
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {}=\"{}",
                                       s_ipsc->cCurrentModuleObject,
                                       surfTemp.Name,
                                       s_ipsc->cAlphaFieldNames(2),
                                       s_ipsc->cAlphaArgs(2)));
                ErrorsFound = true;
            } else {
                surfTemp.Class = SubSurfIDs(ValidChk); // Set class number
            }

            surfTemp.Construction = Util::FindItemInList(s_ipsc->cAlphaArgs(3), state.dataConstruction->Construct, state.dataHeatBal->TotConstructs);

            if (surfTemp.Construction == 0) {
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {}=\"{}\".",
                                       s_ipsc->cCurrentModuleObject,
                                       surfTemp.Name,
                                       s_ipsc->cAlphaFieldNames(3),
                                       s_ipsc->cAlphaArgs(3)));
                ErrorsFound = true;
                continue;
            } else {
                state.dataConstruction->Construct(surfTemp.Construction).IsUsed = true;
                surfTemp.ConstructionStoredInputValue = surfTemp.Construction;
            }

            if (surfTemp.Class == SurfaceClass::Window || surfTemp.Class == SurfaceClass::GlassDoor || surfTemp.Class == SurfaceClass::TDD_Diffuser ||
                surfTemp.Class == SurfaceClass::TDD_Dome) {

                if (surfTemp.Construction != 0) {
                    auto const &construction = state.dataConstruction->Construct(surfTemp.Construction);
                    if (!construction.TypeIsWindow && !construction.TypeIsAirBoundary) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\" has an opaque surface construction; it should have a window construction.",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name));
                    }
                    if (state.dataConstruction->Construct(surfTemp.Construction).SourceSinkPresent) {
                        ErrorsFound = true;
                        ShowSevereError(
                            state,
                            format("{}=\"{}\": Windows are not allowed to have embedded sources/sinks", s_ipsc->cCurrentModuleObject, surfTemp.Name));
                    }
                }

            } else if (surfTemp.Construction != 0) {
                if (state.dataConstruction->Construct(surfTemp.Construction).TypeIsWindow) {
                    ErrorsFound = true;
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\" - has Window materials.",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(3),
                                           s_ipsc->cAlphaArgs(3)));
                    ShowContinueError(state, format("...because {}={}", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
                }
            }

            surfTemp.HeatTransSurf = true;

            surfTemp.BaseSurfName = s_ipsc->cAlphaArgs(4);
            //  The subsurface inherits properties from the base surface
            //  Exterior conditions, Zone, etc.
            //  We can figure out the base surface though, because they've all been entered
            Found = Util::FindItemInList(surfTemp.BaseSurfName, state.dataSurfaceGeometry->SurfaceTmp, state.dataSurface->TotSurfaces);
            if (Found > 0) {
                surfTemp.BaseSurf = Found;
                surfTemp.ExtBoundCond = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtBoundCond;
                surfTemp.ExtBoundCondName = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtBoundCondName;
                surfTemp.ExtSolar = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtSolar;
                surfTemp.ExtWind = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtWind;
                surfTemp.Zone = state.dataSurfaceGeometry->SurfaceTmp(Found).Zone;
                surfTemp.ZoneName = state.dataSurfaceGeometry->SurfaceTmp(Found).ZoneName;
                surfTemp.OSCPtr = state.dataSurfaceGeometry->SurfaceTmp(Found).OSCPtr;
                if (state.dataSurfaceGeometry->SurfaceTmp(Found).ExtBoundCond == unreconciledZoneSurface &&
                    state.dataSurfaceGeometry->SurfaceTmp(Found).ExtBoundCondName ==
                        state.dataSurfaceGeometry->SurfaceTmp(Found).Name) { // Adiabatic surface, no windows or doors allowed
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\".",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(4),
                                           s_ipsc->cAlphaArgs(4)));
                    ShowContinueError(state, "... adiabatic surfaces cannot have windows or doors.");
                    ShowContinueError(state,
                                      "... no solar transmission will result for these windows or doors. You must have interior windows or doors on "
                                      "Interzone surfaces for transmission to result.");
                }
            } else {
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {}=\"{}",
                                       s_ipsc->cCurrentModuleObject,
                                       surfTemp.Name,
                                       s_ipsc->cAlphaFieldNames(4),
                                       s_ipsc->cAlphaArgs(4)));
                surfTemp.ZoneName = "Unknown Zone";
                ErrorsFound = true;
            }
            if (surfTemp.Class == SurfaceClass::TDD_Dome || surfTemp.Class == SurfaceClass::TDD_Diffuser) {
                surfTemp.ExtBoundCond = DataSurfaces::ExternalEnvironment;
            }

            if (surfTemp.ExtBoundCond == DataSurfaces::ExternalEnvironment) {
                if (!s_ipsc->lAlphaFieldBlanks(5)) {
                    ShowWarningError(state,
                                     format("{}=\"{}\", invalid field {}", s_ipsc->cCurrentModuleObject, surfTemp.Name, s_ipsc->cAlphaFieldNames(5)));
                    ShowContinueError(
                        state,
                        format("...when Base surface uses \"Outdoors\" as {}, subsurfaces need to be blank to inherit the outdoor characteristics.",
                               s_ipsc->cAlphaFieldNames(5)));
                    ShowContinueError(state, "...Surface external characteristics changed to reflect base surface.");
                }
            }

            if (surfTemp.ExtBoundCond == unreconciledZoneSurface) { // "Surface" Base Surface
                if (!s_ipsc->lAlphaFieldBlanks(5)) {
                    surfTemp.ExtBoundCondName = s_ipsc->cAlphaArgs(5);
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid blank {}", s_ipsc->cCurrentModuleObject, surfTemp.Name, s_ipsc->cAlphaFieldNames(5)));
                    ShowContinueError(
                        state,
                        format("...when Base surface uses \"Surface\" as {}, subsurfaces must also specify specific surfaces in the adjacent zone.",
                               s_ipsc->cAlphaFieldNames(5)));
                    surfTemp.ExtBoundCondName = s_ipsc->cAlphaArgs(5); // putting it as blank will not confuse things later.
                    ErrorsFound = true;
                }
            }

            if ((surfTemp.ExtBoundCond == unenteredAdjacentZoneSurface) ||
                (surfTemp.ExtBoundCond == unenteredAdjacentSpaceSurface)) { // "Zone" - unmatched interior surface
                ++NeedToAddSurfaces;
                // ignoring window5datafiles for now -- will need to add.
            }

            if (surfTemp.ExtBoundCond == DataSurfaces::OtherSideCoefNoCalcExt || surfTemp.ExtBoundCond == DataSurfaces::OtherSideCoefCalcExt) {
                if (!s_ipsc->lAlphaFieldBlanks(5)) { // Otherside Coef special Name
                    Found = Util::FindItemInList(s_ipsc->cAlphaArgs(5), state.dataSurface->OSC, state.dataSurface->TotOSC);
                    if (Found == 0) {
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}\".",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               s_ipsc->cAlphaFieldNames(5),
                                               s_ipsc->cAlphaArgs(5)));
                        ShowContinueError(state, "...base surface requires that this subsurface have OtherSideCoefficients -- not found.");
                        ErrorsFound = true;
                    } else { // found
                        // The following allows for a subsurface that has different characteristics than
                        // the base surface with OtherSide Coeff -- do we want that or is it an error?
                        surfTemp.OSCPtr = Found;
                        surfTemp.ExtBoundCondName = s_ipsc->cAlphaArgs(5);
                        if (state.dataSurface->OSC(Found).SurfFilmCoef > 0.0) {
                            surfTemp.ExtBoundCond = DataSurfaces::OtherSideCoefCalcExt;
                        } else {
                            surfTemp.ExtBoundCond = DataSurfaces::OtherSideCoefNoCalcExt;
                        }
                    }
                }
            }

            if (surfTemp.ExtBoundCond == DataSurfaces::OtherSideCondModeledExt) {
                surfTemp.ExtBoundCond = DataSurfaces::ExternalEnvironment;
            }

            if (surfTemp.ExtBoundCondName == BlankString) {
                surfTemp.ExtBoundCondName = surfTemp.Name;
            }
            surfTemp.ViewFactorGround = s_ipsc->rNumericArgs(1);
            if (s_ipsc->lNumericFieldBlanks(1)) {
                surfTemp.ViewFactorGround = Constant::AutoCalculate;
            }

            if (s_ipsc->lNumericFieldBlanks(3) || s_ipsc->rNumericArgs(3) == Constant::AutoCalculate) {
                s_ipsc->rNumericArgs(3) = (SurfaceNumProp - 3) / 3;
                surfTemp.Sides = s_ipsc->rNumericArgs(3);
                if (mod(SurfaceNumProp - 3, 3) != 0) {
                    ShowWarningError(state,
                                     format("{}=\"{}\", {}",
                                            s_ipsc->cCurrentModuleObject,
                                            surfTemp.Name,
                                            format("{} not even multiple of 3. Will read in {}", s_ipsc->cNumericFieldNames(3), surfTemp.Sides)));
                }
                if (s_ipsc->rNumericArgs(3) < 3) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", {} (autocalculate) must be >= 3. Only {} provided.",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cNumericFieldNames(3),
                                           surfTemp.Sides));
                    ErrorsFound = true;
                    continue;
                }
            } else {
                numSides = (SurfaceNumProp - 2) / 3;
                surfTemp.Sides = s_ipsc->rNumericArgs(3);
                if (numSides > surfTemp.Sides) {
                    ShowWarningError(state,
                                     format("{}=\"{}\", field {}={}",
                                            s_ipsc->cCurrentModuleObject,
                                            surfTemp.Name,
                                            s_ipsc->cNumericFieldNames(3),
                                            fmt::to_string(surfTemp.Sides)));
                    ShowContinueError(state,
                                      format("...but {} were entered. Only the indicated {} will be used.", numSides, s_ipsc->cNumericFieldNames(3)));
                }
            }
            surfTemp.Vertex.allocate(surfTemp.Sides);
            if (surfTemp.Class == SurfaceClass::Window || surfTemp.Class == SurfaceClass::GlassDoor || surfTemp.Class == SurfaceClass::Door) {
                surfTemp.Multiplier = int(s_ipsc->rNumericArgs(2));
            }
            // Only windows, glass doors and doors can have Multiplier > 1:
            if ((surfTemp.Class != SurfaceClass::Window && surfTemp.Class != SurfaceClass::GlassDoor && surfTemp.Class != SurfaceClass::Door) &&
                s_ipsc->rNumericArgs(2) > 1.0) {
                ShowWarningError(state,
                                 format("{}=\"{}\", invalid {}=[{:.1T}].",
                                        s_ipsc->cCurrentModuleObject,
                                        surfTemp.Name,
                                        s_ipsc->cNumericFieldNames(2),
                                        s_ipsc->rNumericArgs(2)));
                ShowContinueError(state,
                                  format("...because {}={} multiplier will be set to 1.0.", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
                surfTemp.Multiplier = 1.0;
            }

            GetVertices(state, SurfNum, surfTemp.Sides, s_ipsc->rNumericArgs({4, _}));

            CheckConvexity(state, SurfNum, surfTemp.Sides);
            surfTemp.windowShadingControlList.clear();
            surfTemp.activeWindowShadingControl = 0;
            surfTemp.HasShadeControl = false;

            surfTemp.shadedConstructionList.clear();
            surfTemp.activeShadedConstruction = 0;
            surfTemp.shadedStormWinConstructionList.clear();

            if (surfTemp.Class == SurfaceClass::Window || surfTemp.Class == SurfaceClass::GlassDoor || surfTemp.Class == SurfaceClass::TDD_Diffuser ||
                surfTemp.Class == SurfaceClass::TDD_Dome) {

                if (surfTemp.ExtBoundCond == DataSurfaces::OtherSideCoefNoCalcExt || surfTemp.ExtBoundCond == DataSurfaces::OtherSideCoefCalcExt) {
                    ShowSevereError(
                        state,
                        format("{}=\"{}\", Other side coefficients are not allowed with windows.", s_ipsc->cCurrentModuleObject, surfTemp.Name));
                    ErrorsFound = true;
                }

                if (surfTemp.ExtBoundCond == DataSurfaces::Ground) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", Exterior boundary condition = Ground is not allowed with windows.",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name));
                    ErrorsFound = true;
                }

                if (surfTemp.ExtBoundCond == DataSurfaces::KivaFoundation) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", Exterior boundary condition = Foundation is not allowed with windows.",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name));
                    ErrorsFound = true;
                }

                InitialAssociateWindowShadingControlFenestration(state, ErrorsFound, SurfNum);

                CheckWindowShadingControlFrameDivider(state, "GetHTSubSurfaceData", ErrorsFound, SurfNum, 6);

                if (surfTemp.Sides == 3) { // Triangular window
                    if (!s_ipsc->cAlphaArgs(6).empty()) {
                        ShowWarningError(state,
                                         format("{}=\"{}\", invalid {}=\"{}\".",
                                                s_ipsc->cCurrentModuleObject,
                                                surfTemp.Name,
                                                s_ipsc->cAlphaFieldNames(6),
                                                s_ipsc->cAlphaArgs(6)));
                        ShowContinueError(state, ".. because it is a triangular window and cannot have a frame or divider or reveal reflection.");
                        ShowContinueError(state, "Frame, divider and reveal reflection will be ignored for this window.");
                    }
                    surfTemp.FrameDivider = 0;
                } // End of check if window is triangular or rectangular

            } // check on non-opaquedoor subsurfaces

            CheckSubSurfaceMiscellaneous(
                state, "GetHTSubSurfaceData", ErrorsFound, SurfNum, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(3), AddedSubSurfaces);

        } // End of main loop over subsurfaces
    }

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
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   December 2008

        // PURPOSE OF THIS SUBROUTINE:
        // Get simple (rectangular, relative origin to base surface) windows, doors, glazed doors.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static Array1D_string const cModuleObjects(6, {"Window", "Door", "GlazedDoor", "Window:Interzone", "Door:Interzone", "GlazedDoor:Interzone"});

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ItemsToGet;
        int Loop;
        int NumAlphas;
        int NumNumbers;
        int IOStat; // IO Status when calling get input subroutine
        int Found;  // For matching base surfaces
        bool GettingIZSurfaces;
        int FrameField;
        int OtherSurfaceField;
        int ClassItem;
        int IZFound;

        auto &s_ipsc = state.dataIPShortCut;
        for (int Item = 1; Item <= 6; ++Item) {

            s_ipsc->cCurrentModuleObject = cModuleObjects(Item);
            if (Item == 1) {
                ItemsToGet = TotWindows;
                GettingIZSurfaces = false;
                FrameField = 5;
                OtherSurfaceField = 0;
                ClassItem = 1;
            } else if (Item == 2) {
                ItemsToGet = TotDoors;
                GettingIZSurfaces = false;
                FrameField = 0;
                OtherSurfaceField = 0;
                ClassItem = 2;
            } else if (Item == 3) {
                ItemsToGet = TotGlazedDoors;
                GettingIZSurfaces = false;
                FrameField = 5;
                OtherSurfaceField = 0;
                ClassItem = 3;
            } else if (Item == 4) {
                ItemsToGet = TotIZWindows;
                GettingIZSurfaces = true;
                FrameField = 0;
                OtherSurfaceField = 4;
                ClassItem = 1;
            } else if (Item == 5) {
                ItemsToGet = TotIZDoors;
                GettingIZSurfaces = true;
                FrameField = 0;
                OtherSurfaceField = 4;
                ClassItem = 2;
            } else { // Item = 6
                ItemsToGet = TotIZGlazedDoors;
                GettingIZSurfaces = true;
                FrameField = 0;
                OtherSurfaceField = 4;
                ClassItem = 3;
            }

            for (Loop = 1; Loop <= ItemsToGet; ++Loop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         s_ipsc->cCurrentModuleObject,
                                                                         Loop,
                                                                         s_ipsc->cAlphaArgs,
                                                                         NumAlphas,
                                                                         s_ipsc->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStat,
                                                                         s_ipsc->lNumericFieldBlanks,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);

                if (GlobalNames::VerifyUniqueInterObjectName(state,
                                                             state.dataSurfaceGeometry->UniqueSurfaceNames,
                                                             s_ipsc->cAlphaArgs(1),
                                                             s_ipsc->cCurrentModuleObject,
                                                             s_ipsc->cAlphaFieldNames(1),
                                                             ErrorsFound)) {
                    continue;
                }

                if (NumNumbers < 5) {
                    ShowSevereError(
                        state,
                        format("{}=\"{}\", Too few number of numeric args=[{}].", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), NumNumbers));
                    ErrorsFound = true;
                }

                ++SurfNum;
                auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);

                surfTemp.Name = s_ipsc->cAlphaArgs(1);  // Set the Surface Name in the Derived Type
                surfTemp.Class = SubSurfIDs(ClassItem); // Set class number

                surfTemp.Construction =
                    Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataConstruction->Construct, state.dataHeatBal->TotConstructs);

                if (surfTemp.Construction == 0) {
                    ErrorsFound = true;
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\".",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                } else {
                    state.dataConstruction->Construct(surfTemp.Construction).IsUsed = true;
                    surfTemp.ConstructionStoredInputValue = surfTemp.Construction;
                }

                if (surfTemp.Class == SurfaceClass::Window || surfTemp.Class == SurfaceClass::GlassDoor) {

                    if (surfTemp.Construction != 0) {
                        auto const &construction = state.dataConstruction->Construct(surfTemp.Construction);

                        if (!construction.TypeIsWindow && !construction.TypeIsAirBoundary) {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format("{}=\"{}\" has an opaque surface construction; it should have a window construction.",
                                                   s_ipsc->cCurrentModuleObject,
                                                   surfTemp.Name));
                        }
                        if (state.dataConstruction->Construct(surfTemp.Construction).SourceSinkPresent) {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format("{}=\"{}\": Windows are not allowed to have embedded sources/sinks",
                                                   s_ipsc->cCurrentModuleObject,
                                                   surfTemp.Name));
                        }
                    }

                } else if (surfTemp.Construction != 0) {
                    if (state.dataConstruction->Construct(surfTemp.Construction).TypeIsWindow) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}\" - has Window materials.",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               s_ipsc->cAlphaFieldNames(2),
                                               s_ipsc->cAlphaArgs(2)));
                    }
                }

                surfTemp.HeatTransSurf = true;

                surfTemp.BaseSurfName = s_ipsc->cAlphaArgs(3);
                //  The subsurface inherits properties from the base surface
                //  Exterior conditions, Zone, etc.
                //  We can figure out the base surface though, because they've all been entered
                Found = Util::FindItemInList(surfTemp.BaseSurfName, state.dataSurfaceGeometry->SurfaceTmp, state.dataSurface->TotSurfaces);
                if (Found > 0) {
                    surfTemp.BaseSurf = Found;
                    surfTemp.ExtBoundCond = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtBoundCond;
                    surfTemp.ExtBoundCondName = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtBoundCondName;
                    surfTemp.ExtSolar = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtSolar;
                    surfTemp.ExtWind = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtWind;
                    surfTemp.Tilt = state.dataSurfaceGeometry->SurfaceTmp(Found).Tilt;
                    surfTemp.convOrientation = Convect::GetSurfConvOrientation(surfTemp.Tilt);
                    surfTemp.Azimuth = state.dataSurfaceGeometry->SurfaceTmp(Found).Azimuth;
                    surfTemp.Zone = state.dataSurfaceGeometry->SurfaceTmp(Found).Zone;
                    surfTemp.ZoneName = state.dataSurfaceGeometry->SurfaceTmp(Found).ZoneName;
                    surfTemp.OSCPtr = state.dataSurfaceGeometry->SurfaceTmp(Found).OSCPtr;
                    surfTemp.ViewFactorGround = state.dataSurfaceGeometry->SurfaceTmp(Found).ViewFactorGround;
                    surfTemp.ViewFactorSky = state.dataSurfaceGeometry->SurfaceTmp(Found).ViewFactorSky;
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(3),
                                           s_ipsc->cAlphaArgs(3)));
                    surfTemp.ZoneName = "Unknown Zone";
                    ErrorsFound = true;
                    continue;
                }
                if (state.dataSurfaceGeometry->SurfaceTmp(Found).ExtBoundCond == unreconciledZoneSurface &&
                    state.dataSurfaceGeometry->SurfaceTmp(Found).ExtBoundCondName ==
                        state.dataSurfaceGeometry->SurfaceTmp(Found).Name) { // Adiabatic surface, no windows or doors allowed
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\".",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(3),
                                           s_ipsc->cAlphaArgs(3)));
                    ShowContinueError(state, "... adiabatic surfaces cannot have windows or doors.");
                    ShowContinueError(state,
                                      "... no solar transmission will result for these windows or doors. You must have interior windows or doors on "
                                      "Interzone surfaces for transmission to result.");
                }

                if (surfTemp.ExtBoundCond == unreconciledZoneSurface) { // "Surface" Base Surface
                    if (!GettingIZSurfaces) {
                        ShowSevereError(state, format("{}=\"{}\", invalid use of object", s_ipsc->cCurrentModuleObject, surfTemp.Name));
                        ShowContinueError(
                            state,
                            format(
                                "...when Base surface uses \"Surface\" as {}, subsurfaces must also specify specific surfaces in the adjacent zone.",
                                s_ipsc->cAlphaFieldNames(5)));
                        ShowContinueError(state, format("...Please use {}:Interzone to enter this surface.", s_ipsc->cCurrentModuleObject));
                        surfTemp.ExtBoundCondName = BlankString; // putting it as blank will not confuse things later.
                        ErrorsFound = true;
                    }
                }

                if (surfTemp.ExtBoundCond == unreconciledZoneSurface) { // "Surface" Base Surface
                    if (GettingIZSurfaces) {
                        surfTemp.ExtBoundCondName = s_ipsc->cAlphaArgs(OtherSurfaceField);
                        IZFound = Util::FindItemInList(surfTemp.ExtBoundCondName, state.dataHeatBal->Zone, state.dataGlobal->NumOfZones);
                        if (IZFound > 0) {
                            surfTemp.ExtBoundCond = unenteredAdjacentZoneSurface;
                        }
                    } else { // Interior Window
                        surfTemp.ExtBoundCondName = surfTemp.Name;
                    }
                }

                // This is the parent's property:
                if (surfTemp.ExtBoundCond == unenteredAdjacentZoneSurface) { // OtherZone - unmatched interior surface
                    if (GettingIZSurfaces) {
                        ++NeedToAddSubSurfaces;
                    } else { // Interior Window
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid Interzone Surface, specify {}:InterZone",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name,
                                               s_ipsc->cCurrentModuleObject));
                        ShowContinueError(state, "...when base surface is an interzone surface, subsurface must also be an interzone surface.");
                        ++NeedToAddSubSurfaces;
                        ErrorsFound = true;
                    }
                }

                if (GettingIZSurfaces) {
                    if (s_ipsc->lAlphaFieldBlanks(OtherSurfaceField)) {
                        // blank -- set it up for unentered adjacent zone
                        if (surfTemp.ExtBoundCond == unenteredAdjacentZoneSurface) {                                   // already set but need Zone
                            surfTemp.ExtBoundCondName = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtBoundCondName; // base surface has it
                        } else if (surfTemp.ExtBoundCond == unreconciledZoneSurface) {
                            surfTemp.ExtBoundCondName = state.dataSurfaceGeometry->SurfaceTmp(Found).ZoneName; // base surface has it
                            surfTemp.ExtBoundCond = unenteredAdjacentZoneSurface;
                        } else { // not correct boundary condition for interzone subsurface
                            ShowSevereError(
                                state,
                                format("{}=\"{}\", invalid Base Surface type for Interzone Surface", s_ipsc->cCurrentModuleObject, surfTemp.Name));
                            ShowContinueError(state,
                                              "...when base surface is not an interzone surface, subsurface must also not be an interzone surface.");
                            ErrorsFound = true;
                        }
                    }
                }

                if (surfTemp.ExtBoundCond == DataSurfaces::OtherSideCondModeledExt) {
                    surfTemp.ExtBoundCond = DataSurfaces::ExternalEnvironment;
                }

                //      SurfaceTmp(SurfNum)%ViewFactorGround = AutoCalculate

                surfTemp.Sides = 4;
                surfTemp.Vertex.allocate(surfTemp.Sides);
                if (surfTemp.Class == SurfaceClass::Window || surfTemp.Class == SurfaceClass::GlassDoor || surfTemp.Class == SurfaceClass::Door) {
                    surfTemp.Multiplier = int(s_ipsc->rNumericArgs(1));
                } else if (s_ipsc->rNumericArgs(1) > 1.0) {
                    ShowWarningError(state,
                                     format("{}=\"{}\", invalid {}=[{:.1T}].",
                                            s_ipsc->cCurrentModuleObject,
                                            surfTemp.Name,
                                            s_ipsc->cNumericFieldNames(1),
                                            s_ipsc->rNumericArgs(1)));
                    ShowContinueError(state,
                                      format("...because {}={} multiplier will be set to 1.0.", s_ipsc->cAlphaFieldNames(1), s_ipsc->cAlphaArgs(1)));
                    surfTemp.Multiplier = 1.0;
                }

                MakeRelativeRectangularVertices(state,
                                                surfTemp.BaseSurf,
                                                SurfNum,
                                                s_ipsc->rNumericArgs(2),
                                                s_ipsc->rNumericArgs(3),
                                                s_ipsc->rNumericArgs(4),
                                                s_ipsc->rNumericArgs(5));

                if (surfTemp.Area <= 0.0) {
                    ShowSevereError(
                        state,
                        format("{}=\"{}\", Surface Area <= 0.0; Entered Area={:.2T}", s_ipsc->cCurrentModuleObject, surfTemp.Name, surfTemp.Area));
                    ErrorsFound = true;
                }

                surfTemp.windowShadingControlList.clear();
                surfTemp.activeWindowShadingControl = 0;
                surfTemp.HasShadeControl = false;

                surfTemp.shadedConstructionList.clear();
                surfTemp.activeShadedConstruction = 0;
                surfTemp.shadedStormWinConstructionList.clear();

                InitialAssociateWindowShadingControlFenestration(state, ErrorsFound, SurfNum);

                if (!GettingIZSurfaces && (surfTemp.Class == SurfaceClass::Window || surfTemp.Class == SurfaceClass::GlassDoor)) {

                    if (surfTemp.ExtBoundCond == DataSurfaces::OtherSideCoefNoCalcExt ||
                        surfTemp.ExtBoundCond == DataSurfaces::OtherSideCoefCalcExt) {
                        ShowSevereError(
                            state,
                            format("{}=\"{}\", Other side coefficients are not allowed with windows.", s_ipsc->cCurrentModuleObject, surfTemp.Name));
                        ErrorsFound = true;
                    }

                    if (surfTemp.ExtBoundCond == DataSurfaces::Ground) {
                        ShowSevereError(state,
                                        format("{}=\"{}\", Exterior boundary condition = Ground is not allowed with windows.",
                                               s_ipsc->cCurrentModuleObject,
                                               surfTemp.Name));
                        ErrorsFound = true;
                    }

                    CheckWindowShadingControlFrameDivider(state, "GetRectSubSurfaces", ErrorsFound, SurfNum, FrameField);

                } // check on non-opaquedoor subsurfaces

                CheckSubSurfaceMiscellaneous(
                    state, "GetRectSubSurfaces", ErrorsFound, SurfNum, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(2), AddedSubSurfaces);

            } // Getting Items
        }
    }

    void CheckWindowShadingControlFrameDivider(EnergyPlusData &state,
                                               std::string_view const cRoutineName, // routine name calling this one (for error messages)
                                               bool &ErrorsFound,                   // true if errors have been found or are found here
                                               int const SurfNum,                   // current surface number
                                               int const FrameField                 // field number for frame/divider
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   December 2008

        // PURPOSE OF THIS SUBROUTINE:
        // This routine performs checks on WindowShadingControl settings and Frame/Divider Settings.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ConstrNumSh;    // Construction number with Shade
        int ShDevNum;       // Shading Device number
        int Lay;            // Layer number
        int TotGlassLayers; // Number of glass layers in window construction
        int TotLayers;      // Number of layers in unshaded construction
        int TotShLayers;    // Number of layers in shaded construction
        int MatGap;         // Gap material number
        int MatGap1;        // Material number of gap to left (outer side) of between-glass shade/blind
        int MatGap2;        // Material number of gap to right (inner side) of between-glass shade/blind
        int MatSh;          // Between-glass shade/blind material number
        Real64 MatGapCalc;  // Calculated MatGap diff for shaded vs non-shaded constructions

        // If WindowShadingControl has been specified for this window --
        // Set shaded construction number if shaded construction was specified in WindowShadingControl.
        // Otherwise, create shaded construction if WindowShadingControl for this window has
        // interior or exterior shade/blind (but not between-glass shade/blind) specified.

        auto &s_ipsc = state.dataIPShortCut;
        auto &s_mat = state.dataMaterial;

        auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);

        for (std::size_t shadeControlIndex = 0; shadeControlIndex < surfTemp.windowShadingControlList.size(); ++shadeControlIndex) {
            int WSCPtr = surfTemp.windowShadingControlList[shadeControlIndex];
            ConstrNumSh = 0;
            if (!ErrorsFound && surfTemp.HasShadeControl) {
                ConstrNumSh = surfTemp.shadedConstructionList[shadeControlIndex];
                if (ConstrNumSh > 0) {
                    surfTemp.activeShadedConstruction = ConstrNumSh;
                } else if (ANY_INTERIOR_SHADE_BLIND(state.dataSurface->WindowShadingControl(WSCPtr).ShadingType) ||
                           ANY_EXTERIOR_SHADE_BLIND_SCREEN(state.dataSurface->WindowShadingControl(WSCPtr).ShadingType)) {
                    ShDevNum = state.dataSurface->WindowShadingControl(WSCPtr).ShadingDevice;
                    if (ShDevNum > 0) {
                        CreateShadedWindowConstruction(state, SurfNum, WSCPtr, ShDevNum, shadeControlIndex);
                        ConstrNumSh = surfTemp.activeShadedConstruction;
                    }
                }
            }

            // Error checks for shades and blinds

            int ConstrNum = surfTemp.Construction;
            if (!ErrorsFound && WSCPtr > 0 && ConstrNum > 0 && ConstrNumSh > 0) {

                if (ANY_INTERIOR_SHADE_BLIND(state.dataSurface->WindowShadingControl(WSCPtr).ShadingType)) {
                    TotLayers = state.dataConstruction->Construct(ConstrNum).TotLayers;
                    TotShLayers = state.dataConstruction->Construct(ConstrNumSh).TotLayers;
                    if (TotShLayers - 1 != TotLayers) {
                        ShowWarningError(
                            state,
                            "WindowShadingControl: Interior shade or blind: Potential problem in match of unshaded/shaded constructions, "
                            "shaded should have 1 more layers than unshaded.");
                        ShowContinueError(state, format("Unshaded construction={}", state.dataConstruction->Construct(ConstrNum).Name));
                        ShowContinueError(state, format("Shaded construction={}", state.dataConstruction->Construct(ConstrNumSh).Name));
                        ShowContinueError(state,
                                          "If preceding two constructions are same name, you have likely specified a WindowShadingControl (Field #3) "
                                          "with the Window Construction rather than a shaded construction.");
                    }
                    for (Lay = 1; Lay <= state.dataConstruction->Construct(ConstrNum).TotLayers; ++Lay) {
                        if (state.dataConstruction->Construct(ConstrNum).LayerPoint(Lay) !=
                            state.dataConstruction->Construct(ConstrNumSh).LayerPoint(Lay)) {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format(" The glass and gas layers in the shaded and unshaded constructions do not match for window={}",
                                                   surfTemp.Name));
                            ShowContinueError(state, format("Unshaded construction={}", state.dataConstruction->Construct(ConstrNum).Name));
                            ShowContinueError(state, format("Shaded construction={}", state.dataConstruction->Construct(ConstrNumSh).Name));
                            break;
                        }
                    }
                }

                if (ANY_EXTERIOR_SHADE_BLIND_SCREEN(state.dataSurface->WindowShadingControl(WSCPtr).ShadingType)) {
                    TotLayers = state.dataConstruction->Construct(ConstrNum).TotLayers;
                    TotShLayers = state.dataConstruction->Construct(ConstrNumSh).TotLayers;
                    if (TotShLayers - 1 != TotLayers) {
                        ShowWarningError(state,
                                         "WindowShadingControl: Exterior shade, screen or blind: Potential problem in match of unshaded/shaded "
                                         "constructions, shaded should have 1 more layer than unshaded.");
                        ShowContinueError(state, format("Unshaded construction={}", state.dataConstruction->Construct(ConstrNum).Name));
                        ShowContinueError(state, format("Shaded construction={}", state.dataConstruction->Construct(ConstrNumSh).Name));
                        ShowContinueError(
                            state,
                            "If preceding two constructions have the same name, you have likely specified a WindowShadingControl (Field "
                            "#3) with the Window Construction rather than a shaded construction.");
                    }
                    for (Lay = 1; Lay <= state.dataConstruction->Construct(ConstrNum).TotLayers; ++Lay) {
                        if (state.dataConstruction->Construct(ConstrNum).LayerPoint(Lay) !=
                            state.dataConstruction->Construct(ConstrNumSh).LayerPoint(Lay + 1)) {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format(" The glass and gas layers in the shaded and unshaded constructions do not match for window={}",
                                                   surfTemp.Name));
                            ShowContinueError(state, format("Unshaded construction={}", state.dataConstruction->Construct(ConstrNum).Name));
                            ShowContinueError(state, format("Shaded construction={}", state.dataConstruction->Construct(ConstrNumSh).Name));
                            break;
                        }
                    }
                }

                if (ANY_BETWEENGLASS_SHADE_BLIND(state.dataSurface->WindowShadingControl(WSCPtr).ShadingType)) {
                    // Divider not allowed with between-glass shade or blind
                    if (surfTemp.FrameDivider > 0) {
                        if (state.dataSurface->FrameDivider(surfTemp.FrameDivider).DividerWidth > 0.0) {
                            ShowWarningError(state, format("A divider cannot be specified for window {}", surfTemp.Name));
                            ShowContinueError(state, ", which has a between-glass shade or blind.");
                            ShowContinueError(state, "Calculation will proceed without the divider for this window.");
                            state.dataSurface->FrameDivider(surfTemp.FrameDivider).DividerWidth = 0.0;
                        }
                    }
                    // Check consistency of gap widths between unshaded and shaded constructions
                    TotGlassLayers = state.dataConstruction->Construct(ConstrNum).TotGlassLayers;
                    TotLayers = state.dataConstruction->Construct(ConstrNum).TotLayers;
                    TotShLayers = state.dataConstruction->Construct(ConstrNumSh).TotLayers;
                    if (TotShLayers - 2 != TotLayers) {
                        ShowWarningError(
                            state,
                            "WindowShadingControl: Between Glass Shade/Blind: Potential problem in match of unshaded/shaded constructions, "
                            "shaded should have 2 more layers than unshaded.");
                        ShowContinueError(state, format("Unshaded construction={}", state.dataConstruction->Construct(ConstrNum).Name));
                        ShowContinueError(state, format("Shaded construction={}", state.dataConstruction->Construct(ConstrNumSh).Name));
                        ShowContinueError(state,
                                          "If preceding two constructions are same name, you have likely specified a WindowShadingControl (Field #3) "
                                          "with the Window Construction rather than a shaded construction.");
                    }
                    if (state.dataConstruction->Construct(ConstrNum).LayerPoint(TotLayers) !=
                        state.dataConstruction->Construct(ConstrNumSh).LayerPoint(TotShLayers)) {
                        ShowSevereError(state, format("{}: Mis-match in unshaded/shaded inside layer materials.  These should match.", cRoutineName));
                        ShowContinueError(state,
                                          format("Unshaded construction={}, Material={}",
                                                 state.dataConstruction->Construct(ConstrNum).Name,
                                                 s_mat->materials(state.dataConstruction->Construct(ConstrNum).LayerPoint(TotLayers))->Name));
                        ShowContinueError(state,
                                          format("Shaded construction={}, Material={}",
                                                 state.dataConstruction->Construct(ConstrNumSh).Name,
                                                 s_mat->materials(state.dataConstruction->Construct(ConstrNumSh).LayerPoint(TotShLayers))->Name));
                        ErrorsFound = true;
                    }
                    if (state.dataConstruction->Construct(ConstrNum).LayerPoint(1) != state.dataConstruction->Construct(ConstrNumSh).LayerPoint(1)) {
                        ShowSevereError(state, format("{}: Mis-match in unshaded/shaded inside layer materials.  These should match.", cRoutineName));
                        ShowContinueError(state,
                                          format("Unshaded construction={}, Material={}",
                                                 state.dataConstruction->Construct(ConstrNum).Name,
                                                 s_mat->materials(state.dataConstruction->Construct(ConstrNum).LayerPoint(1))->Name));
                        ShowContinueError(state,
                                          format("Shaded construction={}, Material={}",
                                                 state.dataConstruction->Construct(ConstrNumSh).Name,
                                                 s_mat->materials(state.dataConstruction->Construct(ConstrNumSh).LayerPoint(1))->Name));
                        ErrorsFound = true;
                    }
                    if (TotGlassLayers == 2 || TotGlassLayers == 3) {
                        MatGap = state.dataConstruction->Construct(ConstrNum).LayerPoint(2 * TotGlassLayers - 2);
                        MatGap1 = state.dataConstruction->Construct(ConstrNumSh).LayerPoint(2 * TotGlassLayers - 2);
                        MatGap2 = state.dataConstruction->Construct(ConstrNumSh).LayerPoint(2 * TotGlassLayers);
                        MatSh = state.dataConstruction->Construct(ConstrNumSh).LayerPoint(2 * TotGlassLayers - 1);
                        if (state.dataSurface->WindowShadingControl(WSCPtr).ShadingType == DataSurfaces::WinShadingType::BGBlind) {
                            MatGapCalc = std::abs(s_mat->materials(MatGap)->Thickness -
                                                  (s_mat->materials(MatGap1)->Thickness + s_mat->materials(MatGap2)->Thickness));
                            if (MatGapCalc > 0.001) {
                                ShowSevereError(state,
                                                format("{}: The gap width(s) for the unshaded window construction {}",
                                                       cRoutineName,
                                                       state.dataConstruction->Construct(ConstrNum).Name));
                                ShowContinueError(state,
                                                  "are inconsistent with the gap widths for shaded window construction " +
                                                      state.dataConstruction->Construct(ConstrNumSh).Name);
                                ShowContinueError(state, "for window " + surfTemp.Name + ", which has a between-glass blind.");
                                ShowContinueError(
                                    state,
                                    format("..Material={} thickness={:.3R} -", s_mat->materials(MatGap)->Name, s_mat->materials(MatGap)->Thickness));
                                ShowContinueError(state,
                                                  format("..( Material={} thickness={:.3R} +",
                                                         s_mat->materials(MatGap1)->Name,
                                                         s_mat->materials(MatGap1)->Thickness));
                                ShowContinueError(state,
                                                  format("..Material={} thickness={:.3R} )=[{:.3R}] >.001",
                                                         s_mat->materials(MatGap2)->Name,
                                                         s_mat->materials(MatGap2)->Thickness,
                                                         MatGapCalc));
                                ErrorsFound = true;
                            }
                        } else { // Between-glass shade
                            MatGapCalc = std::abs(
                                s_mat->materials(MatGap)->Thickness -
                                (s_mat->materials(MatGap1)->Thickness + s_mat->materials(MatGap2)->Thickness + s_mat->materials(MatSh)->Thickness));
                            if (MatGapCalc > 0.001) {
                                ShowSevereError(state,
                                                format("{}: The gap width(s) for the unshaded window construction {}",
                                                       cRoutineName,
                                                       state.dataConstruction->Construct(ConstrNum).Name));
                                ShowContinueError(state,
                                                  "are inconsistent with the gap widths for shaded window construction " +
                                                      state.dataConstruction->Construct(ConstrNumSh).Name);
                                ShowContinueError(state, "for window " + surfTemp.Name + ", which has a between-glass shade.");
                                ShowContinueError(
                                    state,
                                    format("..Material={} thickness={:.3R} -", s_mat->materials(MatGap)->Name, s_mat->materials(MatGap)->Thickness));
                                ShowContinueError(state,
                                                  format("...( Material={} thickness={:.3R} +",
                                                         s_mat->materials(MatGap1)->Name,
                                                         s_mat->materials(MatGap1)->Thickness));
                                ShowContinueError(state,
                                                  format("..Material={} thickness={:.3R} +",
                                                         s_mat->materials(MatGap2)->Name,
                                                         s_mat->materials(MatGap2)->Thickness));
                                ShowContinueError(state,
                                                  format("..Material={} thickness={:.3R} )=[{:.3R}] >.001",
                                                         s_mat->materials(MatSh)->Name,
                                                         s_mat->materials(MatSh)->Thickness,
                                                         MatGapCalc));
                                ErrorsFound = true;
                            }
                        }
                    }
                }
            }
        }

        if (surfTemp.Sides != 3) { // Rectangular Window
            // Initialize the FrameDivider number for this window. W5FrameDivider will be positive if
            // this window's construction came from the Window5 data file and that construction had an
            // associated frame or divider. It will be zero if the window's construction is not from the
            // Window5 data file, or the construction is from the data file, but the construction has no
            // associated frame or divider. Note that if there is a FrameDivider candidate for this
            // window from the Window5 data file it is used instead of the window's input FrameDivider.

            if (surfTemp.Construction != 0) {
                surfTemp.FrameDivider = state.dataConstruction->Construct(surfTemp.Construction).W5FrameDivider;

                // Warning if FrameAndDivider for this window is over-ridden by one from Window5 Data File
                if (surfTemp.FrameDivider > 0 && !s_ipsc->lAlphaFieldBlanks(FrameField)) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", {}=\"{}\"",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(FrameField),
                                           s_ipsc->cAlphaArgs(FrameField)));
                    ShowContinueError(state,
                                      format("will be replaced with FrameAndDivider from Window5 Data File entry {}",
                                             state.dataConstruction->Construct(surfTemp.Construction).Name));
                }

                if (!s_ipsc->lAlphaFieldBlanks(FrameField) && surfTemp.FrameDivider == 0) {
                    surfTemp.FrameDivider = Util::FindItemInList(s_ipsc->cAlphaArgs(FrameField), state.dataSurface->FrameDivider);
                    if (surfTemp.FrameDivider == 0) {
                        if (!state.dataConstruction->Construct(surfTemp.Construction).WindowTypeEQL) {
                            ShowSevereError(state,
                                            format("{}=\"{}\", invalid {}=\"{}\"",
                                                   s_ipsc->cCurrentModuleObject,
                                                   surfTemp.Name,
                                                   s_ipsc->cAlphaFieldNames(FrameField),
                                                   s_ipsc->cAlphaArgs(FrameField)));
                            ErrorsFound = true;
                        } else {
                            ShowSevereError(state,
                                            format("{}=\"{}\", invalid {}=\"{}\"",
                                                   s_ipsc->cCurrentModuleObject,
                                                   surfTemp.Name,
                                                   s_ipsc->cAlphaFieldNames(FrameField),
                                                   s_ipsc->cAlphaArgs(FrameField)));
                            ShowContinueError(state, "...Frame/Divider is not supported in Equivalent Layer Window model.");
                        }
                    }
                    // Divider not allowed with between-glass shade or blind
                    for (int WSCPtr : surfTemp.windowShadingControlList) {
                        if (!ErrorsFound && WSCPtr > 0 && ConstrNumSh > 0) {
                            if (ANY_BETWEENGLASS_SHADE_BLIND(state.dataSurface->WindowShadingControl(WSCPtr).ShadingType)) {
                                if (surfTemp.FrameDivider > 0) {
                                    if (state.dataSurface->FrameDivider(surfTemp.FrameDivider).DividerWidth > 0.0) {
                                        ShowSevereError(state,
                                                        format("{}=\"{}\", invalid {}=\"{}\"",
                                                               s_ipsc->cCurrentModuleObject,
                                                               surfTemp.Name,
                                                               s_ipsc->cAlphaFieldNames(FrameField),
                                                               s_ipsc->cAlphaArgs(FrameField)));
                                        ShowContinueError(state,
                                                          "Divider cannot be specified because the construction has a between-glass shade or blind.");
                                        ShowContinueError(state, "Calculation will proceed without the divider for this window.");
                                        ShowContinueError(
                                            state,
                                            format("Divider width = [{:.2R}].", state.dataSurface->FrameDivider(surfTemp.FrameDivider).DividerWidth));
                                        state.dataSurface->FrameDivider(surfTemp.FrameDivider).DividerWidth = 0.0;
                                    }
                                } // End of check if window has divider
                            } // End of check if window has a between-glass shade or blind
                        } // End of check if window has a shaded construction
                    } // end of looping through window shading controls of window
                } // End of check if window has an associated FrameAndDivider
            } // End of check if window has a construction
        }

        if (state.dataConstruction->Construct(surfTemp.Construction).WindowTypeEQL) {
            if (surfTemp.FrameDivider > 0) {
                // Equivalent Layer window does not have frame/divider model
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {}=\"{}\"",
                                       s_ipsc->cCurrentModuleObject,
                                       surfTemp.Name,
                                       s_ipsc->cAlphaFieldNames(FrameField),
                                       s_ipsc->cAlphaArgs(FrameField)));
                ShowContinueError(state, "Frame/Divider is not supported in Equivalent Layer Window model.");
                surfTemp.FrameDivider = 0;
            }
        }
    }

    void CheckSubSurfaceMiscellaneous(EnergyPlusData &state,
                                      std::string_view const cRoutineName,       // routine name calling this one (for error messages)
                                      bool &ErrorsFound,                         // true if errors have been found or are found here
                                      int const SurfNum,                         // current surface number
                                      std::string const &SubSurfaceName,         // name of the surface
                                      std::string const &SubSurfaceConstruction, // name of the construction
                                      int &AddedSubSurfaces)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   December 2008

        // PURPOSE OF THIS SUBROUTINE:
        // This routine performs miscellaneous checks on subsurfaces: Windows, GlassDoors, Doors, Tubular Devices.

        auto &s_mat = state.dataMaterial;
        // Warning if window has multiplier > 1 and SolarDistribution = FullExterior or FullInteriorExterior

        auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
        if ((surfTemp.Class == SurfaceClass::Window || surfTemp.Class == SurfaceClass::GlassDoor) &&
            static_cast<int>(state.dataHeatBal->SolarDistribution) > static_cast<int>(DataHeatBalance::Shadowing::Minimal) &&
            surfTemp.Multiplier > 1.0) {
            if (state.dataGlobal->DisplayExtraWarnings) {
                ShowWarningError(state, format("{}: A Multiplier > 1.0 for window/glass door {}", cRoutineName, surfTemp.Name));
                ShowContinueError(state, "in conjunction with SolarDistribution = FullExterior or FullInteriorExterior");
                ShowContinueError(state, "can cause inaccurate shadowing on the window and/or");
                ShowContinueError(state, "inaccurate interior solar distribution from the window.");
            }
            ++state.dataErrTracking->TotalMultipliedWindows;
        }

        //  Require that a construction referenced by a surface that is a window
        //  NOT have a shading device layer; use WindowShadingControl to specify a shading device.

        int ConstrNum = surfTemp.Construction;
        if (ConstrNum > 0) {
            int NumShades = 0;
            for (int Lay = 1; Lay <= state.dataConstruction->Construct(ConstrNum).TotLayers; ++Lay) {
                int LayerPtr = state.dataConstruction->Construct(ConstrNum).LayerPoint(Lay);
                if (LayerPtr == 0) {
                    continue; // Error is caught already, will terminate later
                }
                if (s_mat->materials(LayerPtr)->group == Material::Group::Shade || s_mat->materials(LayerPtr)->group == Material::Group::Blind ||
                    s_mat->materials(LayerPtr)->group == Material::Group::Screen) {
                    ++NumShades;
                }
            }
            if (NumShades != 0) {
                ShowSevereError(state, format("{}: Window \"{}\" must not directly reference", cRoutineName, SubSurfaceName));
                ShowContinueError(state, format("a Construction (i.e, \"{}\") with a shading device.", SubSurfaceConstruction));
                ShowContinueError(state, "Use WindowShadingControl to specify a shading device for a window.");
                ErrorsFound = true;
            }
        }

        // Disallow glass transmittance dirt factor for interior windows and glass doors

        if (surfTemp.ExtBoundCond != DataSurfaces::ExternalEnvironment &&
            (surfTemp.Class == SurfaceClass::Window || surfTemp.Class == SurfaceClass::GlassDoor)) {
            ConstrNum = surfTemp.Construction;
            if (ConstrNum > 0) {
                for (int Lay = 1; Lay <= state.dataConstruction->Construct(ConstrNum).TotLayers; ++Lay) {
                    int LayerPtr = state.dataConstruction->Construct(ConstrNum).LayerPoint(Lay);
                    auto const *mat = s_mat->materials(LayerPtr);
                    if (mat->group != Material::Group::Glass) {
                        continue;
                    }

                    if (dynamic_cast<Material::MaterialGlass const *>(mat)->GlassTransDirtFactor < 1.0) {
                        ShowSevereError(state, format("{}: Interior Window or GlassDoor {} has a glass layer with", cRoutineName, SubSurfaceName));
                        ShowContinueError(state, "Dirt Correction Factor for Solar and Visible Transmittance < 1.0");
                        ShowContinueError(state, "A value less than 1.0 for this factor is only allowed for exterior windows and glass doors.");
                        ErrorsFound = true;
                    }
                }
            }
        }

        // If this is a window with a construction from the Window5DataFile, call routine that will
        // (1) if one glazing system on Data File, give warning message if window height or width
        //     differ by more than 10% from those of the glazing system on the Data File;
        // (2) if two glazing systems (separated by a mullion) on Data File, create a second window
        //     and adjust the dimensions of the original and second windows to those on the Data File

        if (surfTemp.Construction != 0) {

            if (state.dataConstruction->Construct(surfTemp.Construction).FromWindow5DataFile) {

                ModifyWindow(state, SurfNum, ErrorsFound, AddedSubSurfaces);

            } else {
                // Calculate net area for base surface (note that ModifyWindow, above, adjusts net area of
                // base surface for case where window construction is from Window5 Data File
                // In case there is in error in this window's base surface (i.e. none)..
                if (surfTemp.BaseSurf > 0) {
                    state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).Area -= surfTemp.Area;

                    // Subtract TDD:DIFFUSER area from other side interzone surface
                    if ((surfTemp.Class == SurfaceClass::TDD_Diffuser) &&
                        not_blank(
                            state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).ExtBoundCondName)) { // Base surface is an interzone surface
                        // Lookup interzone surface of the base surface
                        // (Interzone surfaces have not been assigned yet, but all base surfaces should already be loaded.)
                        int Found = Util::FindItemInList(state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).ExtBoundCondName,
                                                         state.dataSurfaceGeometry->SurfaceTmp,
                                                         SurfNum);
                        if (Found != 0) {
                            state.dataSurfaceGeometry->SurfaceTmp(Found).Area -= surfTemp.Area;
                        }
                    }
                    if (state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).Area <= 0.0) {
                        ShowSevereError(state,
                                        format("{}: Surface Openings have too much area for base surface={}",
                                               cRoutineName,
                                               state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).Name));
                        ShowContinueError(state, format("Opening Surface creating error={}", surfTemp.Name));
                        ErrorsFound = true;
                    }
                    // Net area of base surface with unity window multipliers (used in shadowing checks)
                    // For Windows, Glass Doors and Doors, just one area is subtracted.  For the rest, should be
                    // full area.
                    if (surfTemp.Class == SurfaceClass::Window || surfTemp.Class == SurfaceClass::GlassDoor) {
                        state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).NetAreaShadowCalc -= surfTemp.Area / surfTemp.Multiplier;
                    } else if (surfTemp.Class == SurfaceClass::Door) { // Door, TDD:Diffuser, TDD:DOME
                        state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).NetAreaShadowCalc -= surfTemp.Area / surfTemp.Multiplier;
                    } else {
                        state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).NetAreaShadowCalc -= surfTemp.Area;
                    }
                }
            }
        }
    }

    void MakeRelativeRectangularVertices(EnergyPlusData &state,
                                         int const BaseSurfNum, // Base surface
                                         int const SurfNum,
                                         Real64 const XCoord,
                                         Real64 const ZCoord,
                                         Real64 const Length,
                                         Real64 const Height)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   December 2008

        // PURPOSE OF THIS SUBROUTINE:
        // This routine creates world/3d coordinates for rectangular surfaces using relative X and Z, length & height.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D<Real64> XX(4);
        Array1D<Real64> YY(4);
        Real64 Perimeter;
        int Vrt;

        if (BaseSurfNum == 0) {
            return; // invalid base surface, don't bother
        }

        // Tilt and Facing (Azimuth) will be same as the Base Surface

        auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
        surfTemp.Height = Height;
        surfTemp.Width = Length;

        Real64 SurfAzimuth = surfTemp.Azimuth;
        Real64 SurfTilt = surfTemp.Tilt;
        Real64 CosSurfAzimuth = std::cos(SurfAzimuth * Constant::DegToRad);
        Real64 SinSurfAzimuth = std::sin(SurfAzimuth * Constant::DegToRad);
        Real64 CosSurfTilt = std::cos(SurfTilt * Constant::DegToRad);
        Real64 SinSurfTilt = std::sin(SurfTilt * Constant::DegToRad);
        Real64 BaseCosSurfAzimuth = state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).CosAzim;
        Real64 BaseSinSurfAzimuth = state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).SinAzim;
        Real64 BaseCosSurfTilt = state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).CosTilt;
        Real64 BaseSinSurfTilt = state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).SinTilt;

        Real64 XLLC = state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).Vertex(2).x - XCoord * BaseCosSurfAzimuth -
                      ZCoord * BaseCosSurfTilt * BaseSinSurfAzimuth;
        Real64 YLLC = state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).Vertex(2).y + XCoord * BaseSinSurfAzimuth -
                      ZCoord * BaseCosSurfTilt * BaseCosSurfAzimuth;
        Real64 ZLLC = state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).Vertex(2).z + ZCoord * BaseSinSurfTilt;

        XX(1) = 0.0;
        XX(2) = 0.0;
        XX(3) = Length;
        XX(4) = Length;
        YY(1) = Height;
        YY(4) = Height;
        YY(3) = 0.0;
        YY(2) = 0.0;

        for (int n = 1; n <= surfTemp.Sides; ++n) {
            Vrt = n;
            surfTemp.Vertex(Vrt).x = XLLC - XX(n) * CosSurfAzimuth - YY(n) * CosSurfTilt * SinSurfAzimuth;
            surfTemp.Vertex(Vrt).y = YLLC + XX(n) * SinSurfAzimuth - YY(n) * CosSurfTilt * CosSurfAzimuth;
            surfTemp.Vertex(Vrt).z = ZLLC + YY(n) * SinSurfTilt;
        }

        Vectors::CreateNewellAreaVector(surfTemp.Vertex, surfTemp.Sides, surfTemp.NewellAreaVector);
        surfTemp.GrossArea = Vectors::VecLength(surfTemp.NewellAreaVector);
        surfTemp.Area = surfTemp.GrossArea;
        surfTemp.NetAreaShadowCalc = surfTemp.Area;
        Vectors::CreateNewellSurfaceNormalVector(surfTemp.Vertex, surfTemp.Sides, surfTemp.NewellSurfaceNormalVector);
        Vectors::DetermineAzimuthAndTilt(
            surfTemp.Vertex, SurfAzimuth, SurfTilt, surfTemp.lcsx, surfTemp.lcsy, surfTemp.lcsz, surfTemp.NewellSurfaceNormalVector);
        surfTemp.Azimuth = SurfAzimuth;
        surfTemp.Tilt = SurfTilt;
        surfTemp.convOrientation = Convect::GetSurfConvOrientation(surfTemp.Tilt);
        // Sine and cosine of azimuth and tilt
        surfTemp.SinAzim = SinSurfAzimuth;
        surfTemp.CosAzim = CosSurfAzimuth;
        surfTemp.SinTilt = SinSurfTilt;
        surfTemp.CosTilt = CosSurfTilt;
        if (surfTemp.Class != SurfaceClass::Window && surfTemp.Class != SurfaceClass::GlassDoor && surfTemp.Class != SurfaceClass::Door) {
            surfTemp.ViewFactorGround = 0.5 * (1.0 - surfTemp.CosTilt);
        }
        // Outward normal unit vector (pointing away from room)
        surfTemp.OutNormVec = surfTemp.NewellSurfaceNormalVector;
        for (int n = 1; n <= 3; ++n) {
            if (std::abs(surfTemp.OutNormVec(n) - 1.0) < 1.e-06) {
                surfTemp.OutNormVec(n) = +1.0;
            }
            if (std::abs(surfTemp.OutNormVec(n) + 1.0) < 1.e-06) {
                surfTemp.OutNormVec(n) = -1.0;
            }
            if (std::abs(surfTemp.OutNormVec(n)) < 1.e-06) {
                surfTemp.OutNormVec(n) = 0.0;
            }
        }

        //  IF (SurfaceTmp(SurfNum)%Class == SurfaceClass::Roof .and. SurfTilt > 80.) THEN
        //    WRITE(TiltString,'(F5.1)') SurfTilt
        //    TiltString=ADJUSTL(TiltString)
        //    CALL ShowWarningError(state, format("Roof/Ceiling Tilt={}{}{}{}{}{}{}{}{}{} for Surface={}{}{}, in
        //    Zone={}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}", //TRIM(TiltString)//',,
        //    much, greater, than, expected, tilt, of, 0,'//, &, //, //TRIM(SurfaceTmp(SurfNum)%Name)//, &, //, //TRIM(SurfaceTmp(SurfNum)%ZoneName)),
        //    //, ENDIF, //, IF, (SurfaceTmp(SurfNum)%Class, ==, SurfaceClass::Floor, .and., SurfTilt, <, 170.), THEN, //, WRITE(TiltString,'(F5.1)'),
        //    SurfTilt, //, TiltString=ADJUSTL(TiltString), //, CALL, ShowWarningError(state, 'Floor Tilt='//TRIM(TiltString)//', much less than
        //    expected tilt of 180,'//   &
        //                          ' for Surface='//TRIM(SurfaceTmp(SurfNum)%Name)//  &
        //                          ', in Zone='//TRIM(SurfaceTmp(SurfNum)%ZoneName)), //, ENDIF, if,
        //                          (surfTemp.Class, ==, SurfaceClass::Window, ||,
        //                          surfTemp.Class, ==, SurfaceClass::GlassDoor, ||,
        //                          surfTemp.Class, ==, SurfaceClass::Door),
        //                          surfTemp.Area, *=,
        //                          surfTemp.Multiplier;, //, Can, perform, tests, on, this, surface, here,
        //                          surfTemp.ViewFactorSky, =, 0.5, *, (1.0,
        //                          surfTemp.CosTilt));
        // The following IR view factors are modified in subr. SkyDifSolarShading if there are shadowing
        // surfaces
        surfTemp.ViewFactorSkyIR = surfTemp.ViewFactorSky;
        surfTemp.ViewFactorGroundIR = 0.5 * (1.0 - surfTemp.CosTilt);

        Perimeter = distance(surfTemp.Vertex(surfTemp.Sides), surfTemp.Vertex(1));
        for (Vrt = 2; Vrt <= surfTemp.Sides; ++Vrt) {
            Perimeter += distance(surfTemp.Vertex(Vrt), surfTemp.Vertex(Vrt - 1));
        }
        surfTemp.Perimeter = Perimeter;

        // Call to transform vertices

        TransformVertsByAspect(state, SurfNum, surfTemp.Sides);
    }

    void GetAttShdSurfaceData(EnergyPlusData &state,
                              bool &ErrorsFound,   // Error flag indicator (true if errors found)
                              int &SurfNum,        // Count of Current SurfaceNumber
                              int const TotShdSubs // Number of Attached Shading SubSurfaces to obtain
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the HeatTransfer Surface Data,
        // checks it for errors, etc.

        static constexpr std::string_view routineName = "GetAttShdSurfaceData";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int IOStat;     // IO Status when calling get input subroutine
        int NumAlphas;  // Number of alpha names being passed
        int NumNumbers; // Number of properties being passed
        int Found;      // For matching interzone surfaces
        int Loop;
        Real64 SchedMinValue;
        Real64 SchedMaxValue;

        auto &s_ipsc = state.dataIPShortCut;

        if (TotShdSubs > 0 && state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::Minimal) {
            ShowWarningError(state, "Shading effects of Fins and Overhangs are ignored when Solar Distribution = MinimalShadowing");
        }
        s_ipsc->cCurrentModuleObject = "Shading:Zone:Detailed";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, s_ipsc->cCurrentModuleObject, Loop, NumAlphas, NumNumbers);
        if (NumAlphas != 3) {
            ShowSevereError(
                state, format("{}: Object Definition indicates not = 3 Alpha Objects, Number Indicated={}", s_ipsc->cCurrentModuleObject, NumAlphas));
            ErrorsFound = true;
        }

        for (Loop = 1; Loop <= TotShdSubs; ++Loop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            if (GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataSurfaceGeometry->UniqueSurfaceNames,
                                                         s_ipsc->cAlphaArgs(1),
                                                         s_ipsc->cCurrentModuleObject,
                                                         s_ipsc->cAlphaFieldNames(1),
                                                         ErrorsFound)) {
                continue;
            }

            ++SurfNum;
            auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);

            surfTemp.Name = s_ipsc->cAlphaArgs(1); // Set the Surface Name in the Derived Type
            surfTemp.Class = SurfaceClass::Shading;
            surfTemp.HeatTransSurf = false;
            surfTemp.BaseSurfName = s_ipsc->cAlphaArgs(2);
            //  The subsurface inherits properties from the base surface
            //  Exterior conditions, Zone, etc.
            //  We can figure out the base surface though, because they've all been entered
            Found = Util::FindItemInList(surfTemp.BaseSurfName, state.dataSurfaceGeometry->SurfaceTmp, state.dataSurface->TotSurfaces);
            if (Found > 0) {
                // SurfaceTmp(SurfNum)%BaseSurf=Found
                surfTemp.ExtBoundCond = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtBoundCond;
                surfTemp.ExtSolar = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtSolar;
                surfTemp.ExtWind = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtWind;
                surfTemp.Zone = state.dataSurfaceGeometry->SurfaceTmp(Found).Zone; // Necessary to do relative coordinates in GetVertices below
                surfTemp.ZoneName = state.dataSurfaceGeometry->SurfaceTmp(Found).ZoneName; // Necessary to have surface drawn in OutputReports
            } else {
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {}=\"{}",
                                       s_ipsc->cCurrentModuleObject,
                                       surfTemp.Name,
                                       s_ipsc->cAlphaFieldNames(2),
                                       s_ipsc->cAlphaArgs(2)));
                ErrorsFound = true;
            }
            if (surfTemp.ExtBoundCond == unenteredAdjacentZoneSurface) {
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {}=\"{}",
                                       s_ipsc->cCurrentModuleObject,
                                       surfTemp.Name,
                                       s_ipsc->cAlphaFieldNames(2),
                                       s_ipsc->cAlphaArgs(2)));
                ShowContinueError(state, "...trying to attach a shading device to an interzone surface.");
                ErrorsFound = true;

                surfTemp.ExtBoundCond = DataSurfaces::ExternalEnvironment; // reset so program won't crash during "add surfaces"
            }
            if (surfTemp.ExtBoundCond == unreconciledZoneSurface) {
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {}=\"{}",
                                       s_ipsc->cCurrentModuleObject,
                                       surfTemp.Name,
                                       s_ipsc->cAlphaFieldNames(2),
                                       s_ipsc->cAlphaArgs(2)));
                ShowContinueError(state, "...trying to attach a shading device to an interior surface.");
                ErrorsFound = true;

                surfTemp.ExtBoundCond = DataSurfaces::ExternalEnvironment; // reset so program won't crash during "add surfaces"
            }

            if (s_ipsc->lAlphaFieldBlanks(3)) {
                // Defaults to constant-0.0, but leave this as nullptr for now
            } else if ((surfTemp.shadowSurfSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            } else if (!surfTemp.shadowSurfSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                Sched::ShowSevereBadMinMax(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), Clusive::In, 0.0, Clusive::In, 1.0);
                ErrorsFound = true;
            } else {

                SchedMinValue = surfTemp.shadowSurfSched->getMinVal(state);
                surfTemp.SchedMinValue = SchedMinValue;
                SchedMaxValue = surfTemp.shadowSurfSched->getMaxVal(state);
                if (SchedMinValue == 1.0) {
                    // Set transparent for now, check for EMS actuators later in SolarShading::resetShadingSurfaceTransparency
                    surfTemp.IsTransparent = true;
                }
                if (SchedMaxValue > 0.0) {
                    state.dataSolarShading->anyScheduledShadingSurface = true;
                }
                if (SchedMaxValue - SchedMinValue > Constant::OneMillionth) {
                    state.dataSurface->ShadingTransmittanceVaries = true;
                }
            }

            if (s_ipsc->lNumericFieldBlanks(1) || s_ipsc->rNumericArgs(1) == Constant::AutoCalculate) {
                s_ipsc->rNumericArgs(1) = (NumNumbers - 1) / 3;
                surfTemp.Sides = s_ipsc->rNumericArgs(1);
                if (mod(NumNumbers - 1, 3) != 0) {
                    ShowWarningError(state,
                                     format("{}=\"{}\", {}",
                                            s_ipsc->cCurrentModuleObject,
                                            surfTemp.Name,
                                            format("{} not even multiple of 3. Will read in {}", s_ipsc->cNumericFieldNames(1), surfTemp.Sides)));
                }
                if (s_ipsc->rNumericArgs(1) < 3) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", {} (autocalculate) must be >= 3. Only {} provided.",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cNumericFieldNames(1),
                                           surfTemp.Sides));
                    ErrorsFound = true;
                    continue;
                }
            } else {
                surfTemp.Sides = s_ipsc->rNumericArgs(1);
            }
            surfTemp.Vertex.allocate(surfTemp.Sides);
            GetVertices(state, SurfNum, surfTemp.Sides, s_ipsc->rNumericArgs({2, _}));
            CheckConvexity(state, SurfNum, surfTemp.Sides);
            //    IF (SurfaceTmp(SurfNum)%Sides == 3) THEN
            //      CALL ShowWarningError(state, TRIM(s_ipsc->cCurrentModuleObject)//'="'//TRIM(SurfaceTmp(SurfNum)%Name)//  &
            //                        ' should not be triangular.')
            //      CALL ShowContinueError(state, '...Check results carefully.')
            //      ErrorsFound=.TRUE.
            //    ENDIF
            // Reset surface to be "detached"
            surfTemp.BaseSurf = 0;
            //    SurfaceTmp(SurfNum)%BaseSurfName='  '
            surfTemp.Zone = 0;
            // SurfaceTmp(SurfNum)%ZoneName='  '
            if (state.dataReportFlag->MakeMirroredAttachedShading) {
                MakeMirrorSurface(state, SurfNum);
            }
        }
    }

    void GetSimpleShdSurfaceData(EnergyPlusData &state,
                                 bool &ErrorsFound,                // Error flag indicator (true if errors found)
                                 int &SurfNum,                     // Count of Current SurfaceNumber
                                 int const TotOverhangs,           // Number of Overhangs to obtain
                                 int const TotOverhangsProjection, // Number of Overhangs (projection) to obtain
                                 int const TotFins,                // Number of Fins to obtain
                                 int const TotFinsProjection       // Number of Fins (projection) to obtain
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2009

        // PURPOSE OF THIS SUBROUTINE:
        // Get simple overhang and fin descriptions.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static Array1D_string const cModuleObjects(4, {"Shading:Overhang", "Shading:Overhang:Projection", "Shading:Fin", "Shading:Fin:Projection"});

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ItemsToGet;
        int NumAlphas;
        int NumNumbers;
        int IOStat; // IO Status when calling get input subroutine
        Real64 Depth;
        Real64 Length;
        Real64 Xp;
        Real64 Yp;
        Real64 Zp;
        Real64 XLLC;
        Real64 YLLC;
        int BaseSurfNum;
        Real64 TiltAngle;
        bool MakeFin;

        auto &s_ipsc = state.dataIPShortCut;

        if ((TotOverhangs + TotOverhangsProjection + TotFins + TotFinsProjection) > 0 &&
            state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::Minimal) {
            ShowWarningError(state, "Shading effects of Fins and Overhangs are ignored when Solar Distribution = MinimalShadowing");
        }
        for (int Item = 1; Item <= 4; ++Item) {

            s_ipsc->cCurrentModuleObject = cModuleObjects(Item);
            if (Item == 1) {
                ItemsToGet = TotOverhangs;
            } else if (Item == 2) {
                ItemsToGet = TotOverhangsProjection;
            } else if (Item == 3) {
                ItemsToGet = TotFins;
            } else { // ! (Item == 4) THEN
                ItemsToGet = TotFinsProjection;
            }

            for (int Loop = 1; Loop <= ItemsToGet; ++Loop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         s_ipsc->cCurrentModuleObject,
                                                                         Loop,
                                                                         s_ipsc->cAlphaArgs,
                                                                         NumAlphas,
                                                                         s_ipsc->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStat,
                                                                         s_ipsc->lNumericFieldBlanks,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);

                if (GlobalNames::VerifyUniqueInterObjectName(state,
                                                             state.dataSurfaceGeometry->UniqueSurfaceNames,
                                                             s_ipsc->cAlphaArgs(1),
                                                             s_ipsc->cCurrentModuleObject,
                                                             s_ipsc->cAlphaFieldNames(1),
                                                             ErrorsFound)) {
                    continue;
                }

                ++SurfNum;
                auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);

                surfTemp.Name = s_ipsc->cAlphaArgs(1); // Set the Surface Name in the Derived Type
                surfTemp.Class = SurfaceClass::Shading;
                surfTemp.HeatTransSurf = false;
                // this object references a window or door....
                int Found = Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataSurfaceGeometry->SurfaceTmp, state.dataSurface->TotSurfaces);
                if (Found > 0) {
                    BaseSurfNum = state.dataSurfaceGeometry->SurfaceTmp(Found).BaseSurf;
                    surfTemp.BaseSurfName = state.dataSurfaceGeometry->SurfaceTmp(Found).BaseSurfName;
                    surfTemp.ExtBoundCond = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtBoundCond;
                    surfTemp.ExtSolar = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtSolar;
                    surfTemp.ExtWind = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtWind;
                    surfTemp.Zone = state.dataSurfaceGeometry->SurfaceTmp(Found).Zone; // Necessary to do relative coordinates in GetVertices below
                    surfTemp.ZoneName = state.dataSurfaceGeometry->SurfaceTmp(Found).ZoneName; // Necessary to have surface drawn in OutputReports
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ErrorsFound = true;
                    continue;
                }
                if (surfTemp.ExtBoundCond == unenteredAdjacentZoneSurface) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ShowContinueError(state, "...trying to attach a shading device to an interzone surface.");
                    ErrorsFound = true;
                    surfTemp.ExtBoundCond = DataSurfaces::ExternalEnvironment; // reset so program won't crash during "add surfaces"
                }
                if (surfTemp.ExtBoundCond == unreconciledZoneSurface) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}",
                                           s_ipsc->cCurrentModuleObject,
                                           surfTemp.Name,
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ShowContinueError(state, "...trying to attach a shading device to an interior surface.");
                    ErrorsFound = true;
                    surfTemp.ExtBoundCond = DataSurfaces::ExternalEnvironment; // reset so program won't crash during "add surfaces"
                }

                surfTemp.shadowSurfSched = nullptr;

                //===== Overhang =====

                if (Item < 3) {
                    //  Found is the surface window or door.
                    //   N1,  \field Height above Window or Door
                    //        \units m
                    //   N2,  \field Tilt Angle from Window/Door
                    //        \units deg
                    //        \default 90
                    //        \minimum 0
                    //        \maximum 180
                    //   N3,  \field Left extension from Window/Door Width
                    //        \units m
                    //   N4,  \field Right extension from Window/Door Width
                    //        \note N3 + N4 + Window/Door Width is Overhang Length
                    //        \units m
                    //   N5;  \field Depth
                    //        \units m
                    // for projection option:
                    //   N5;  \field Depth as Fraction of Window/Door Height
                    //        \units m
                    Length = s_ipsc->rNumericArgs(3) + s_ipsc->rNumericArgs(4) + state.dataSurfaceGeometry->SurfaceTmp(Found).Width;
                    if (Item == 1) {
                        Depth = s_ipsc->rNumericArgs(5);
                    } else if (Item == 2) {
                        Depth = s_ipsc->rNumericArgs(5) * state.dataSurfaceGeometry->SurfaceTmp(Found).Height;
                    }

                    if (Length * Depth <= 0.0) {
                        ShowSevereError(state,
                                        format("{}=\"{}\", illegal surface area=[{:.2R}]. Surface will NOT be entered.",
                                               s_ipsc->cCurrentModuleObject,
                                               s_ipsc->cAlphaArgs(1),
                                               Length * Depth));
                        continue;
                    }

                    TiltAngle = state.dataSurfaceGeometry->SurfaceTmp(Found).Tilt + s_ipsc->rNumericArgs(2);
                    surfTemp.Tilt = TiltAngle;
                    surfTemp.convOrientation = Convect::GetSurfConvOrientation(surfTemp.Tilt);
                    surfTemp.Azimuth = state.dataSurfaceGeometry->SurfaceTmp(Found).Azimuth;

                    // Make it relative to surface origin.....
                    Xp = state.dataSurfaceGeometry->SurfaceTmp(Found).Vertex(2).x - state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).Vertex(2).x;
                    Yp = state.dataSurfaceGeometry->SurfaceTmp(Found).Vertex(2).y - state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).Vertex(2).y;
                    Zp = state.dataSurfaceGeometry->SurfaceTmp(Found).Vertex(2).z - state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).Vertex(2).z;

                    XLLC = -Xp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).CosAzim +
                           Yp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).SinAzim;

                    YLLC =
                        -Xp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).SinAzim *
                            state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).CosTilt -
                        Yp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).CosAzim * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).CosTilt +
                        Zp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).SinTilt;

                    surfTemp.Sides = 4;
                    surfTemp.Vertex.allocate(surfTemp.Sides);

                    MakeRelativeRectangularVertices(state,
                                                    BaseSurfNum,
                                                    SurfNum,
                                                    XLLC - s_ipsc->rNumericArgs(3),
                                                    YLLC + state.dataSurfaceGeometry->SurfaceTmp(Found).Height + s_ipsc->rNumericArgs(1),
                                                    Length,
                                                    Depth);

                    // Reset surface to be "detached"
                    //    SurfaceTmp(SurfNum)%BaseSurfName='  '
                    //    SurfaceTmp(SurfNum)%ZoneName='  '

                    surfTemp.BaseSurf = 0;
                    surfTemp.Zone = 0;

                    // and mirror
                    if (state.dataReportFlag->MakeMirroredAttachedShading) {
                        MakeMirrorSurface(state, SurfNum);
                    }

                } else { // Fins

                    //===== Fins =====

                    //===== Left Fin =====

                    //   N1,  \field Left Extension from Window/Door
                    //        \units m
                    //   N2,  \field Left Distance Above Top of Window
                    //        \units m
                    //   N3,  \field Left Distance Below Bottom of Window
                    //        \units m
                    //        \note N2 + N3 + height of Window/Door is height of Fin
                    //   N4,  \field Left Tilt Angle from Window/Door
                    //        \units deg
                    //        \default 90
                    //        \minimum 0
                    //        \maximum 180
                    //   N5,  \field Left Depth
                    //        \units m
                    // for projection option:
                    //   N5,  \field Left Depth as Fraction of Window/Door Width
                    //        \units m
                    surfTemp.Name = surfTemp.Name + " Left";
                    Length = s_ipsc->rNumericArgs(2) + s_ipsc->rNumericArgs(3) + state.dataSurfaceGeometry->SurfaceTmp(Found).Height;
                    if (Item == 3) {
                        Depth = s_ipsc->rNumericArgs(5);
                    } else if (Item == 4) {
                        Depth = s_ipsc->rNumericArgs(5) * state.dataSurfaceGeometry->SurfaceTmp(Found).Width;
                    }

                    MakeFin = true;
                    if (Length * Depth <= 0.0) {
                        ShowWarningError(state,
                                         format("{}=Left Fin of \"{}\", illegal surface area=[{:.2R}]. Surface will NOT be entered.",
                                                s_ipsc->cCurrentModuleObject,
                                                s_ipsc->cAlphaArgs(1),
                                                Length * Depth));
                        MakeFin = false;
                    }

                    if (MakeFin) {
                        TiltAngle = state.dataSurfaceGeometry->SurfaceTmp(Found).Tilt;
                        surfTemp.Tilt = TiltAngle;
                        surfTemp.convOrientation = Convect::GetSurfConvOrientation(surfTemp.Tilt);
                        surfTemp.Azimuth = state.dataSurfaceGeometry->SurfaceTmp(Found).Azimuth - (180.0 - s_ipsc->rNumericArgs(4));

                        // Make it relative to surface origin.....

                        Xp =
                            state.dataSurfaceGeometry->SurfaceTmp(Found).Vertex(2).x - state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).Vertex(2).x;
                        Yp =
                            state.dataSurfaceGeometry->SurfaceTmp(Found).Vertex(2).y - state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).Vertex(2).y;
                        Zp =
                            state.dataSurfaceGeometry->SurfaceTmp(Found).Vertex(2).z - state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).Vertex(2).z;

                        XLLC = -Xp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).CosAzim +
                               Yp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).SinAzim;

                        YLLC = -Xp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).SinAzim *
                                   state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).CosTilt -
                               Yp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).CosAzim *
                                   state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).CosTilt +
                               Zp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).SinTilt;

                        surfTemp.CosAzim = std::cos(surfTemp.Azimuth * Constant::DegToRad);
                        surfTemp.SinAzim = std::sin(surfTemp.Azimuth * Constant::DegToRad);
                        surfTemp.CosTilt = std::cos(surfTemp.Tilt * Constant::DegToRad);
                        surfTemp.SinTilt = std::sin(surfTemp.Tilt * Constant::DegToRad);

                        surfTemp.Sides = 4;
                        surfTemp.Vertex.allocate(surfTemp.Sides);

                        MakeRelativeRectangularVertices(
                            state, BaseSurfNum, SurfNum, XLLC - s_ipsc->rNumericArgs(1), YLLC - s_ipsc->rNumericArgs(3), -Depth, Length);

                        // Reset surface to be "detached"
                        //    SurfaceTmp(SurfNum)%BaseSurfName='  '
                        //    SurfaceTmp(SurfNum)%ZoneName='  '

                        surfTemp.BaseSurf = 0;
                        surfTemp.Zone = 0;

                        // and mirror
                        if (state.dataReportFlag->MakeMirroredAttachedShading) {
                            MakeMirrorSurface(state, SurfNum);
                        }
                    } else {
                        --SurfNum;
                    }

                    //===== Right Fin =====

                    //   N6,  \field Right Extension from Window/Door
                    //        \units m
                    //   N7,  \field Right Distance Above Top of Window
                    //        \units m
                    //   N8,  \field Right Distance Below Bottom of Window
                    //        \note N7 + N8 + height of Window/Door is height of Fin
                    //        \units m
                    //   N9,  \field Right Tilt Angle from Window/Door
                    //        \units deg
                    //        \default 90
                    //        \minimum 0
                    //        \maximum 180
                    //   N10; \field Right Depth
                    //        \units m
                    // for projection option:
                    //   N10; \field Right Depth as Fraction of Window/Door Width
                    //        \units m

                    ++SurfNum;
                    auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
                    surfTemp.Name = s_ipsc->cAlphaArgs(1) + " Right"; // Set the Surface Name in the Derived Type
                    surfTemp.Class = SurfaceClass::Shading;
                    surfTemp.HeatTransSurf = false;
                    BaseSurfNum = state.dataSurfaceGeometry->SurfaceTmp(Found).BaseSurf;
                    surfTemp.BaseSurfName = state.dataSurfaceGeometry->SurfaceTmp(Found).BaseSurfName;
                    surfTemp.ExtBoundCond = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtBoundCond;
                    surfTemp.ExtSolar = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtSolar;
                    surfTemp.ExtWind = state.dataSurfaceGeometry->SurfaceTmp(Found).ExtWind;
                    surfTemp.Zone = state.dataSurfaceGeometry->SurfaceTmp(Found).Zone; // Necessary to do relative coordinates in GetVertices below
                    surfTemp.ZoneName = state.dataSurfaceGeometry->SurfaceTmp(Found).ZoneName; // Necessary to have surface drawn in OutputReports

                    surfTemp.shadowSurfSched = nullptr;
                    Length = s_ipsc->rNumericArgs(7) + s_ipsc->rNumericArgs(8) + state.dataSurfaceGeometry->SurfaceTmp(Found).Height;
                    if (Item == 3) {
                        Depth = s_ipsc->rNumericArgs(10);
                    } else if (Item == 4) {
                        Depth = s_ipsc->rNumericArgs(10) * state.dataSurfaceGeometry->SurfaceTmp(Found).Width;
                    }

                    MakeFin = true;
                    if (Length * Depth <= 0.0) {
                        ShowWarningError(state,
                                         format("{}=Right Fin of \"{}\", illegal surface area=[{:.2R}]. Surface will NOT be entered.",
                                                s_ipsc->cCurrentModuleObject,
                                                s_ipsc->cAlphaArgs(1),
                                                Length * Depth));
                        MakeFin = false;
                    }

                    if (MakeFin) {
                        // Make it relative to surface origin.....

                        Xp =
                            state.dataSurfaceGeometry->SurfaceTmp(Found).Vertex(2).x - state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).Vertex(2).x;
                        Yp =
                            state.dataSurfaceGeometry->SurfaceTmp(Found).Vertex(2).y - state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).Vertex(2).y;
                        Zp =
                            state.dataSurfaceGeometry->SurfaceTmp(Found).Vertex(2).z - state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).Vertex(2).z;

                        XLLC = -Xp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).CosAzim +
                               Yp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).SinAzim;

                        YLLC = -Xp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).SinAzim *
                                   state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).CosTilt -
                               Yp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).CosAzim *
                                   state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).CosTilt +
                               Zp * state.dataSurfaceGeometry->SurfaceTmp(BaseSurfNum).SinTilt;

                        TiltAngle = state.dataSurfaceGeometry->SurfaceTmp(Found).Tilt;
                        surfTemp.Tilt = TiltAngle;
                        surfTemp.convOrientation = Convect::GetSurfConvOrientation(surfTemp.Tilt);
                        surfTemp.Azimuth = state.dataSurfaceGeometry->SurfaceTmp(Found).Azimuth - (180.0 - s_ipsc->rNumericArgs(9));
                        surfTemp.CosAzim = std::cos(surfTemp.Azimuth * Constant::DegToRad);
                        surfTemp.SinAzim = std::sin(surfTemp.Azimuth * Constant::DegToRad);
                        surfTemp.CosTilt = std::cos(surfTemp.Tilt * Constant::DegToRad);
                        surfTemp.SinTilt = std::sin(surfTemp.Tilt * Constant::DegToRad);

                        surfTemp.Sides = 4;
                        surfTemp.Vertex.allocate(surfTemp.Sides);

                        MakeRelativeRectangularVertices(state,
                                                        BaseSurfNum,
                                                        SurfNum,
                                                        XLLC + state.dataSurfaceGeometry->SurfaceTmp(Found).Width + s_ipsc->rNumericArgs(6),
                                                        YLLC - s_ipsc->rNumericArgs(8),
                                                        -Depth,
                                                        Length);

                        // Reset surface to be "detached"
                        //    SurfaceTmp(SurfNum)%BaseSurfName='  '
                        //    SurfaceTmp(SurfNum)%ZoneName='  '

                        surfTemp.BaseSurf = 0;
                        surfTemp.Zone = 0;

                        // and mirror
                        if (state.dataReportFlag->MakeMirroredAttachedShading) {
                            MakeMirrorSurface(state, SurfNum);
                        }
                    } else {
                        --SurfNum;
                    }
                }
            }
        }
    }

    void GetIntMassSurfaceData(EnergyPlusData &state,
                               bool &ErrorsFound, // Error flag indicator (true if errors found)
                               int &SurfNum       // Count of Current SurfaceNumber
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the Internal Surface Data,
        // checks it for errors, etc.

        // REFERENCES:
        // Internal Mass Surface Definition
        // Surface:HeatTransfer:InternalMass,
        //       \note used to describe internal zone surface area that does not need to be part of geometric representation
        //  A1 , \field User Supplied Surface Name
        //       \type alpha
        //       \reference SurfaceNames
        //  A2 , \field Construction Name of the Surface
        //       \note To be matched with a construction in this input file
        //       \type object-list
        //       \object-list ConstructionNames
        //  A3 , \field Interior Environment
        //       \note Zone the surface is a part of
        //       \type object-list
        //       \object-list ZoneNames
        //  N1,  \field View factor to Person (to people?)
        //       \type real
        //       \note from the interior of the surface
        //  N2 ; \field Surface area
        //       \units m2

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetIntMassSurfaceData: ");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int IOStat;          // IO Status when calling get input subroutine
        int SurfaceNumAlpha; // Number of material alpha names being passed
        int SurfaceNumArg;   // Number of material properties being passed

        auto &s_ipsc = state.dataIPShortCut;
        s_ipsc->cCurrentModuleObject = "InternalMass";
        int TotIntMass = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        if (TotIntMass == 0) {
            return;
        }

        state.dataSurface->IntMassObjects.allocate(TotIntMass);

        // scan for use of Zone lists in InternalMass objects
        bool errFlag = false;
        int NumIntMassSurfaces = 0;
        for (int Item = 1; Item <= TotIntMass; ++Item) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Item,
                                                                     s_ipsc->cAlphaArgs,
                                                                     SurfaceNumAlpha,
                                                                     s_ipsc->rNumericArgs,
                                                                     SurfaceNumArg,
                                                                     IOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            if (GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataSurfaceGeometry->UniqueSurfaceNames,
                                                         s_ipsc->cAlphaArgs(1),
                                                         s_ipsc->cCurrentModuleObject,
                                                         s_ipsc->cAlphaFieldNames(1),
                                                         ErrorsFound)) {
                continue;
            }

            state.dataSurface->IntMassObjects(Item).Name = s_ipsc->cAlphaArgs(1);
            state.dataSurface->IntMassObjects(Item).GrossArea = s_ipsc->rNumericArgs(1);
            state.dataSurface->IntMassObjects(Item).Construction =
                Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataConstruction->Construct, state.dataHeatBal->TotConstructs);
            state.dataSurface->IntMassObjects(Item).ZoneOrZoneListName = s_ipsc->cAlphaArgs(3);
            int Item1 = Util::FindItemInList(s_ipsc->cAlphaArgs(3), state.dataHeatBal->Zone, state.dataGlobal->NumOfZones);
            int ZLItem = 0;
            if (Item1 == 0 && state.dataHeatBal->NumOfZoneLists > 0) {
                ZLItem = Util::FindItemInList(s_ipsc->cAlphaArgs(3), state.dataHeatBal->ZoneList);
            }
            if (Item1 > 0) {
                if (s_ipsc->lAlphaFieldBlanks(4)) {
                    ++NumIntMassSurfaces;
                }
                state.dataSurface->IntMassObjects(Item).NumOfZones = 1;
                state.dataSurface->IntMassObjects(Item).ZoneListActive = false;
                state.dataSurface->IntMassObjects(Item).ZoneOrZoneListPtr = Item1;
            } else if (ZLItem > 0) {
                NumIntMassSurfaces += state.dataHeatBal->ZoneList(ZLItem).NumOfZones;
                state.dataSurface->IntMassObjects(Item).NumOfZones = state.dataHeatBal->ZoneList(ZLItem).NumOfZones;
                state.dataSurface->IntMassObjects(Item).ZoneListActive = true;
                state.dataSurface->IntMassObjects(Item).ZoneOrZoneListPtr = ZLItem;
            } else if (state.dataIPShortCut->lAlphaFieldBlanks(4)) {
                // If Space or SpaceList Name is blank, then throw error.
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cAlphaFieldNames(3),
                                       s_ipsc->cAlphaArgs(3)));
                ++SurfNum;
                auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
                surfTemp.Class = SurfaceClass::Invalid;
                surfTemp.ZoneName = "Unknown Zone";
                ErrorsFound = true;
                errFlag = true;
            }

            if (!s_ipsc->lAlphaFieldBlanks(4)) {
                state.dataSurface->IntMassObjects(Item).spaceOrSpaceListName = s_ipsc->cAlphaArgs(4);
                Item1 = Util::FindItemInList(s_ipsc->cAlphaArgs(4), state.dataHeatBal->space);
                int SLItem = 0;
                if (Item1 == 0 && int(state.dataHeatBal->spaceList.size()) > 0) {
                    SLItem = Util::FindItemInList(s_ipsc->cAlphaArgs(4), state.dataHeatBal->spaceList);
                }
                if (Item1 > 0) {
                    ++NumIntMassSurfaces;
                    state.dataSurface->IntMassObjects(Item).numOfSpaces = 1;
                    state.dataSurface->IntMassObjects(Item).spaceListActive = false;
                    state.dataSurface->IntMassObjects(Item).spaceOrSpaceListPtr = Item1;
                    state.dataSurface->IntMassObjects(Item).NumOfZones = 1;
                    state.dataSurface->IntMassObjects(Item).ZoneListActive = false;
                    state.dataSurface->IntMassObjects(Item).ZoneOrZoneListPtr = state.dataHeatBal->space(Item1).zoneNum;
                } else if (SLItem > 0) {
                    int numOfSpaces = int(state.dataHeatBal->spaceList(SLItem).numListSpaces);
                    NumIntMassSurfaces += numOfSpaces;
                    state.dataSurface->IntMassObjects(Item).numOfSpaces = numOfSpaces;
                    state.dataSurface->IntMassObjects(Item).spaceListActive = true;
                    state.dataSurface->IntMassObjects(Item).spaceOrSpaceListPtr = SLItem;
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                           s_ipsc->cCurrentModuleObject,
                                           s_ipsc->cAlphaArgs(1),
                                           s_ipsc->cAlphaFieldNames(4),
                                           s_ipsc->cAlphaArgs(4)));
                    ++SurfNum;
                    auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
                    surfTemp.Class = SurfaceClass::Invalid;
                    ErrorsFound = true;
                    errFlag = true;
                }
            }

            if (errFlag) {
                ShowSevereError(state, format("{}Errors with invalid names in {} objects.", RoutineName, s_ipsc->cCurrentModuleObject));
                ShowContinueError(state, "...These will not be read in.  Other errors may occur.");
                NumIntMassSurfaces = 0;
            }

            if (state.dataSurface->IntMassObjects(Item).Construction == 0) {
                ErrorsFound = true;
                ShowSevereError(state,
                                format("{}=\"{}\", {} not found={}",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cAlphaFieldNames(2),
                                       s_ipsc->cAlphaArgs(2)));
            } else if (state.dataConstruction->Construct(state.dataSurface->IntMassObjects(Item).Construction).TypeIsWindow) {
                ErrorsFound = true;
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {}=\"{}\" - has Window materials.",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cAlphaFieldNames(2),
                                       s_ipsc->cAlphaArgs(2)));
            } else {
                state.dataConstruction->Construct(state.dataSurface->IntMassObjects(Item).Construction).IsUsed = true;
            }
        }

        if (NumIntMassSurfaces > 0) {
            for (int Loop = 1; Loop <= TotIntMass; ++Loop) {
                int numberOfZonesOrSpaces = 1;
                if (state.dataSurface->IntMassObjects(Loop).ZoneListActive) {
                    numberOfZonesOrSpaces = state.dataSurface->IntMassObjects(Loop).NumOfZones;
                } else if (state.dataSurface->IntMassObjects(Loop).spaceListActive) {
                    numberOfZonesOrSpaces = state.dataSurface->IntMassObjects(Loop).numOfSpaces;
                }

                for (int Item1 = 1; Item1 <= numberOfZonesOrSpaces; ++Item1) {

                    ++SurfNum;

                    auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
                    surfTemp.Construction = state.dataSurface->IntMassObjects(Loop).Construction;
                    if (!state.dataSurface->IntMassObjects(Loop).ZoneListActive && !state.dataSurface->IntMassObjects(Loop).spaceListActive) {
                        surfTemp.Zone = state.dataSurface->IntMassObjects(Loop).ZoneOrZoneListPtr;
                        surfTemp.spaceNum = state.dataSurface->IntMassObjects(Loop).spaceOrSpaceListPtr;
                        surfTemp.Name = state.dataSurface->IntMassObjects(Loop).Name;
                        surfTemp.Class = SurfaceClass::IntMass;
                        surfTemp.ZoneName = state.dataSurface->IntMassObjects(Loop).ZoneOrZoneListName;
                        surfTemp.HeatTransSurf = true;
                    } else {
                        int ZoneNum = 0; // index to a zone
                        if (state.dataSurface->IntMassObjects(Loop).ZoneListActive) {
                            General::CheckCreatedZoneItemName(
                                state,
                                RoutineName,
                                s_ipsc->cCurrentModuleObject,
                                state.dataHeatBal
                                    ->Zone(state.dataHeatBal->ZoneList(state.dataSurface->IntMassObjects(Loop).ZoneOrZoneListPtr).Zone(Item1))
                                    .Name,
                                state.dataHeatBal->ZoneList(state.dataSurface->IntMassObjects(Loop).ZoneOrZoneListPtr).MaxZoneNameLength,
                                state.dataSurface->IntMassObjects(Loop).Name,
                                state.dataSurfaceGeometry->SurfaceTmp,
                                SurfNum - 1,
                                surfTemp.Name,
                                errFlag);

                            ZoneNum = state.dataHeatBal->ZoneList(state.dataSurface->IntMassObjects(Loop).ZoneOrZoneListPtr).Zone(Item1);
                        } else if (state.dataSurface->IntMassObjects(Loop).spaceListActive) {
                            int spaceNum = state.dataHeatBal->spaceList(state.dataSurface->IntMassObjects(Loop).spaceOrSpaceListPtr).spaces(Item1);
                            ZoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                            const std::string spaceName = state.dataHeatBal->space(spaceNum).Name;
                            surfTemp.Name = spaceName + ' ' + state.dataSurface->IntMassObjects(Loop).Name;
                            surfTemp.spaceNum = spaceNum;
                        }
                        surfTemp.Zone = ZoneNum;
                        surfTemp.Class = SurfaceClass::IntMass;
                        surfTemp.ZoneName = state.dataHeatBal->Zone(ZoneNum).Name;
                        surfTemp.HeatTransSurf = true;
                        if (errFlag) {
                            ErrorsFound = true;
                        }
                    }

                    if (state.dataSurface->IntMassObjects(Loop).Construction > 0) {
                        if (state.dataConstruction->Construct(state.dataSurface->IntMassObjects(Loop).Construction).IsUsed) {
                            surfTemp.ConstructionStoredInputValue = state.dataSurface->IntMassObjects(Loop).Construction;
                        }
                    }
                    surfTemp.GrossArea = state.dataSurface->IntMassObjects(Loop).GrossArea;
                    surfTemp.Area = surfTemp.GrossArea;
                    surfTemp.NetAreaShadowCalc = surfTemp.Area;
                    surfTemp.Width = surfTemp.Area;
                    surfTemp.Height = 1.0;
                    surfTemp.Tilt = 90.0;
                    surfTemp.convOrientation = Convect::GetSurfConvOrientation(surfTemp.Tilt);
                    surfTemp.CosTilt = 0.0; // Tuned Was std::cos( 90.0 * DegToRadians )
                    surfTemp.SinTilt = 1.0; // Tuned Was std::sin( 90.0 * DegToRadians )
                    surfTemp.Azimuth = 0.0;
                    surfTemp.CosAzim = 1.0; // Tuned Was std::cos( 0.0 )
                    surfTemp.SinAzim = 0.0; // Tuned Was std::sin( 0.0 )
                    // Outward normal unit vector (pointing away from room)
                    surfTemp.OutNormVec = surfTemp.lcsz;
                    surfTemp.ViewFactorSky = 0.5;
                    surfTemp.ExtSolar = false;
                    surfTemp.ExtWind = false;
                    surfTemp.BaseSurf = SurfNum;
                    surfTemp.BaseSurfName = surfTemp.Name;
                    surfTemp.ExtBoundCondName = surfTemp.Name;
                    surfTemp.ExtBoundCond = unreconciledZoneSurface;
                }
            }
        }
    }

    int GetNumIntMassSurfaces(EnergyPlusData &state) // Number of Internal Mass Surfaces to obtain

    {
        // Counts internal mass surfaces applied to zones and zone lists

        int IOStat;          // IO Status when calling get input subroutine
        int SurfaceNumAlpha; // Number of material alpha names being passed
        int SurfaceNumArg;   // Number of material properties being passed

        auto &s_ipsc = state.dataIPShortCut;
        s_ipsc->cCurrentModuleObject = "InternalMass";

        int NumIntMassSurf = 0;
        int TotIntMass = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "InternalMass");

        if (TotIntMass == 0) {
            return NumIntMassSurf;
        }
        // scan for zones and zone lists in InternalMass objects
        for (int Item = 1; Item <= TotIntMass; ++Item) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Item,
                                                                     s_ipsc->cAlphaArgs,
                                                                     SurfaceNumAlpha,
                                                                     s_ipsc->rNumericArgs,
                                                                     SurfaceNumArg,
                                                                     IOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            int Item1 = Util::FindItemInList(s_ipsc->cAlphaArgs(3), state.dataHeatBal->Zone, state.dataGlobal->NumOfZones);
            int ZLItem = 0;
            if (Item1 == 0 && state.dataHeatBal->NumOfZoneLists > 0) {
                ZLItem = Util::FindItemInList(s_ipsc->cAlphaArgs(3), state.dataHeatBal->ZoneList);
            }
            if (Item1 > 0) {
                if (s_ipsc->lAlphaFieldBlanks(4)) {
                    ++NumIntMassSurf;
                }
            } else if (ZLItem > 0) {
                NumIntMassSurf += state.dataHeatBal->ZoneList(ZLItem).NumOfZones;
            }

            if (!s_ipsc->lAlphaFieldBlanks(4)) {
                Item1 = Util::FindItemInList(s_ipsc->cAlphaArgs(4), state.dataHeatBal->space);
                int SLItem = 0;
                if (Item1 == 0 && int(state.dataHeatBal->spaceList.size()) > 0) {
                    SLItem = Util::FindItemInList(s_ipsc->cAlphaArgs(4), state.dataHeatBal->spaceList);
                }
                if (Item1 > 0) {
                    ++NumIntMassSurf;
                } else if (SLItem > 0) {
                    int numOfSpaces = int(state.dataHeatBal->spaceList(SLItem).numListSpaces);
                    NumIntMassSurf += numOfSpaces;
                }
            }
        }
        NumIntMassSurf = max(NumIntMassSurf, TotIntMass);
        return NumIntMassSurf;
    }

    void GetShadingSurfReflectanceData(EnergyPlusData &state, bool &ErrorsFound) // If errors found in input
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   Sept 2003

        // PURPOSE OF THIS SUBROUTINE:
        // Gets data for a Shading Surface Reflectance object.  This is only called when the
        // Solar Distribution is to be calculated for reflectances.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int IOStat;                       // IO Status when calling get input subroutine
        int NumAlpha;                     // Number of alpha names being passed
        int NumProp;                      // Number of properties being passed
        int TotShadingSurfaceReflectance; // Total Shading Surface Reflectance statements
        int Loop;                         // DO loop index
        int SurfNum;                      // Surface number
        int GlConstrNum;                  // Glazing construction number
        bool WrongSurfaceType;

        auto &s_ipsc = state.dataIPShortCut;
        // For shading surfaces, initialize value of reflectance values to default values. These values
        // may be overridden below for shading surfaces with an associated Shading Surface Reflectance object.
        for (SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            auto const &surf = state.dataSurface->Surface(SurfNum);
            if (!(surf.Class == SurfaceClass::Shading || surf.Class == SurfaceClass::Detached_F || surf.Class == SurfaceClass::Detached_B ||
                  surf.Class == SurfaceClass::Overhang || surf.Class == SurfaceClass::Fin)) {
                continue;
            }
            state.dataSurface->SurfShadowDiffuseSolRefl(SurfNum) = 0.2;
            state.dataSurface->SurfShadowDiffuseVisRefl(SurfNum) = 0.2;
            state.dataSurface->SurfShadowGlazingFrac(SurfNum) = 0.0;
            state.dataSurface->SurfShadowGlazingConstruct(SurfNum) = 0;
        }

        // Get the total number of Shading Surface Reflectance objects
        s_ipsc->cCurrentModuleObject = "ShadingProperty:Reflectance";
        TotShadingSurfaceReflectance = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        //  IF(TotShadingSurfaceReflectance.EQ.0) RETURN

        for (Loop = 1; Loop <= TotShadingSurfaceReflectance; ++Loop) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlpha,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumProp,
                                                                     IOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);
            SurfNum = Util::FindItemInList(s_ipsc->cAlphaArgs(1), state.dataSurface->Surface, state.dataSurface->TotSurfaces);
            if (SurfNum == 0) {
                ShowWarningError(state, format("{}=\"{}\", invalid specification", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, format(".. not found {}=\"{}\".", s_ipsc->cAlphaFieldNames(1), s_ipsc->cAlphaArgs(1)));
                //      ErrorsFound =.TRUE.
                continue;
            }

            // Check that associated surface is a shading surface
            WrongSurfaceType = false;
            auto const &surf = state.dataSurface->Surface(SurfNum);
            if (!(surf.Class == SurfaceClass::Shading || surf.Class == SurfaceClass::Detached_F || surf.Class == SurfaceClass::Detached_B ||
                  surf.Class == SurfaceClass::Overhang || surf.Class == SurfaceClass::Fin)) {
                WrongSurfaceType = true;
            }
            if (WrongSurfaceType) {
                ShowSevereError(
                    state,
                    format("GetShadingSurfReflectanceData: {}=\"{}\", surface is not a shading surface.", s_ipsc->cCurrentModuleObject, surf.Name));
                ErrorsFound = true;
                continue;
            }

            // If associated surface is a shading surface, set reflectance values
            state.dataSurface->SurfShadowGlazingFrac(SurfNum) = s_ipsc->rNumericArgs(3);
            state.dataSurface->SurfShadowDiffuseSolRefl(SurfNum) = (1.0 - s_ipsc->rNumericArgs(3)) * s_ipsc->rNumericArgs(1);
            state.dataSurface->SurfShadowDiffuseVisRefl(SurfNum) = (1.0 - s_ipsc->rNumericArgs(3)) * s_ipsc->rNumericArgs(2);
            if (s_ipsc->rNumericArgs(3) > 0.0) {
                GlConstrNum = Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataConstruction->Construct, state.dataHeatBal->TotConstructs);
                if (GlConstrNum == 0) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", {} not found={}",
                                           s_ipsc->cCurrentModuleObject,
                                           state.dataSurface->Surface(SurfNum).Name,
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ErrorsFound = true;
                } else {
                    state.dataConstruction->Construct(GlConstrNum).IsUsed = true;
                }
                state.dataSurface->SurfShadowGlazingConstruct(SurfNum) = GlConstrNum;
            }
            SurfNum = Util::FindItemInList("Mir-" + s_ipsc->cAlphaArgs(1), state.dataSurface->Surface, state.dataSurface->TotSurfaces);
            if (SurfNum == 0) {
                continue;
            }
            state.dataSurface->SurfShadowGlazingFrac(SurfNum) = s_ipsc->rNumericArgs(3);
            state.dataSurface->SurfShadowDiffuseSolRefl(SurfNum) = (1.0 - s_ipsc->rNumericArgs(3)) * s_ipsc->rNumericArgs(1);
            state.dataSurface->SurfShadowDiffuseVisRefl(SurfNum) = (1.0 - s_ipsc->rNumericArgs(3)) * s_ipsc->rNumericArgs(2);
            if (s_ipsc->rNumericArgs(3) > 0.0) {
                GlConstrNum = Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataConstruction->Construct, state.dataHeatBal->TotConstructs);
                if (GlConstrNum != 0) {
                    state.dataConstruction->Construct(GlConstrNum).IsUsed = true;
                }
                state.dataSurface->SurfShadowGlazingConstruct(SurfNum) = GlConstrNum;
            }

        } // End of loop over Shading Surface Reflectance objects

        // Write reflectance values to .eio file.
        print(state.files.eio,
              "! <ShadingProperty Reflectance>,Shading Surface Name,Shading Type,Diffuse Solar Reflectance, Diffuse "
              "Visible Reflectance,Surface Glazing Fraction,Surface Glazing Construction\n");

        for (SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            auto const &surf = state.dataSurface->Surface(SurfNum);
            if (!(surf.Class == SurfaceClass::Shading || surf.Class == SurfaceClass::Detached_F || surf.Class == SurfaceClass::Detached_B ||
                  surf.Class == SurfaceClass::Overhang || surf.Class == SurfaceClass::Fin)) {
                continue;
            }

            constexpr std::string_view fmt = "ShadingProperty Reflectance,{},{},{:.2R},{:.2R},{:.2R}, {}\n";
            if (state.dataSurface->SurfShadowGlazingConstruct(SurfNum) != 0) {
                print(state.files.eio,
                      fmt,
                      surf.Name,
                      cSurfaceClass(surf.Class),
                      state.dataSurface->SurfShadowDiffuseSolRefl(SurfNum),
                      state.dataSurface->SurfShadowDiffuseVisRefl(SurfNum),
                      state.dataSurface->SurfShadowGlazingFrac(SurfNum),
                      state.dataConstruction->Construct(state.dataSurface->SurfShadowGlazingConstruct(SurfNum)).Name);
            } else {
                print(state.files.eio,
                      fmt,
                      surf.Name,
                      cSurfaceClass(surf.Class),
                      state.dataSurface->SurfShadowDiffuseSolRefl(SurfNum),
                      state.dataSurface->SurfShadowDiffuseVisRefl(SurfNum),
                      state.dataSurface->SurfShadowGlazingFrac(SurfNum),
                      "N/A");
            }
        }
    }

    void GetHTSurfExtVentedCavityData(EnergyPlusData &state, bool &ErrorsFound) // Error flag indicator (true if errors found)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         BGriffith
        //       DATE WRITTEN   January 2005

        // PURPOSE OF THIS SUBROUTINE:
        // load input data for Exterior Vented Cavity Special case for heat transfer surfaces

        // METHODOLOGY EMPLOYED:
        // usual E+ input processes

        // REFERENCES:
        // derived from SUBROUTINE GetTranspiredCollectorInput

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Item;          // Item to be "gotten"
        int NumAlphas;     // Number of Alphas for each GetObjectItem call
        int NumNumbers;    // Number of Numbers for each GetObjectItem call
        int MaxNumAlphas;  // argument for call to GetObjectDefMaxArgs
        int MaxNumNumbers; // argument for call to GetObjectDefMaxArgs
        int Dummy;         // argument for call to GetObjectDefMaxArgs
        int IOStatus;      // Used in GetObjectItem
        int Found;
        int AlphaOffset; // local temp var
        std::string Roughness;
        int ThisSurf;                   // do loop counter
        Real64 AvgAzimuth;              // temp for error checking
        Real64 AvgTilt;                 // temp for error checking
        constexpr Real64 AZITOL = 15.0; // Degree Azimuth Angle Tolerance
        constexpr Real64 TILTOL = 10.0; // Degree Tilt Angle Tolerance
        int SurfID;                     // local surface "pointer"

        auto &s_ipsc = state.dataIPShortCut;
        s_ipsc->cCurrentModuleObject = "SurfaceProperty:ExteriorNaturalVentedCavity";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, s_ipsc->cCurrentModuleObject, Dummy, MaxNumAlphas, MaxNumNumbers);

        if (MaxNumNumbers != 8) {
            ShowSevereError(
                state,
                format("{}: Object Definition indicates not = 8 Number Objects, Number Indicated={}", s_ipsc->cCurrentModuleObject, MaxNumNumbers));
            ErrorsFound = true;
        }

        state.dataSurface->TotExtVentCav = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        state.dataHeatBal->ExtVentedCavity.allocate(state.dataSurface->TotExtVentCav);

        for (Item = 1; Item <= state.dataSurface->TotExtVentCav; ++Item) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Item,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);
            // first handle cAlphaArgs
            bool ErrorInName = false;
            bool IsBlank = false;

            Util::VerifyName(state,
                             s_ipsc->cAlphaArgs(1),
                             state.dataHeatBal->ExtVentedCavity,
                             Item - 1,
                             ErrorInName,
                             IsBlank,
                             s_ipsc->cCurrentModuleObject + " Name");
            if (ErrorInName) {
                ShowContinueError(state, "...cannot not duplicate other names");
                ErrorsFound = true;
                continue;
            }
            state.dataHeatBal->ExtVentedCavity(Item).Name = s_ipsc->cAlphaArgs(1);

            state.dataHeatBal->ExtVentedCavity(Item).OSCMName = s_ipsc->cAlphaArgs(2);
            if (!s_ipsc->lAlphaFieldBlanks(2)) {
                Found = Util::FindItemInList(state.dataHeatBal->ExtVentedCavity(Item).OSCMName, state.dataSurface->OSCM, state.dataSurface->TotOSCM);
                if (Found == 0) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\".",
                                           s_ipsc->cCurrentModuleObject,
                                           state.dataHeatBal->ExtVentedCavity(Item).Name,
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ErrorsFound = true;
                }
            } else {
                Found = 0;
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {} cannot be blank.",
                                       s_ipsc->cCurrentModuleObject,
                                       state.dataHeatBal->ExtVentedCavity(Item).Name,
                                       s_ipsc->cAlphaFieldNames(2)));
                ErrorsFound = true;
            }
            state.dataHeatBal->ExtVentedCavity(Item).OSCMPtr = Found;

            Roughness = s_ipsc->cAlphaArgs(3);
            // Select the correct Number for the associated ascii name for the roughness type
            if (Util::SameString(Roughness, "VerySmooth")) {
                state.dataHeatBal->ExtVentedCavity(Item).BaffleRoughness = Material::SurfaceRoughness::VerySmooth;
            } else if (Util::SameString(Roughness, "Smooth")) {
                state.dataHeatBal->ExtVentedCavity(Item).BaffleRoughness = Material::SurfaceRoughness::Smooth;
            } else if (Util::SameString(Roughness, "MediumSmooth")) {
                state.dataHeatBal->ExtVentedCavity(Item).BaffleRoughness = Material::SurfaceRoughness::MediumSmooth;
            } else if (Util::SameString(Roughness, "MediumRough")) {
                state.dataHeatBal->ExtVentedCavity(Item).BaffleRoughness = Material::SurfaceRoughness::MediumRough;
            } else if (Util::SameString(Roughness, "Rough")) {
                state.dataHeatBal->ExtVentedCavity(Item).BaffleRoughness = Material::SurfaceRoughness::Rough;
            } else if (Util::SameString(Roughness, "VeryRough")) {
                state.dataHeatBal->ExtVentedCavity(Item).BaffleRoughness = Material::SurfaceRoughness::VeryRough;
            } // TODO: fix this after creating FindEnumeratedValueIndex()

            // Was it set?
            if (state.dataHeatBal->ExtVentedCavity(Item).BaffleRoughness == Material::SurfaceRoughness::Invalid) {
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {}=\"{}",
                                       s_ipsc->cCurrentModuleObject,
                                       state.dataHeatBal->ExtVentedCavity(Item).Name,
                                       s_ipsc->cAlphaFieldNames(3),
                                       s_ipsc->cAlphaArgs(3)));
                ErrorsFound = true;
            }

            AlphaOffset = 3;
            state.dataHeatBal->ExtVentedCavity(Item).NumSurfs = NumAlphas - AlphaOffset;
            if (state.dataHeatBal->ExtVentedCavity(Item).NumSurfs == 0) {
                ShowSevereError(state,
                                format("{}=\"{}\", no underlying surfaces specified. Must have at least one.",
                                       s_ipsc->cCurrentModuleObject,
                                       state.dataHeatBal->ExtVentedCavity(Item).Name));
                ErrorsFound = true;
                continue;
            }
            state.dataHeatBal->ExtVentedCavity(Item).SurfPtrs.allocate(state.dataHeatBal->ExtVentedCavity(Item).NumSurfs);
            state.dataHeatBal->ExtVentedCavity(Item).SurfPtrs = 0;
            for (ThisSurf = 1; ThisSurf <= state.dataHeatBal->ExtVentedCavity(Item).NumSurfs; ++ThisSurf) {
                Found = Util::FindItemInList(s_ipsc->cAlphaArgs(ThisSurf + AlphaOffset), state.dataSurface->Surface, state.dataSurface->TotSurfaces);
                if (Found == 0) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}",
                                           s_ipsc->cCurrentModuleObject,
                                           state.dataHeatBal->ExtVentedCavity(Item).Name,
                                           s_ipsc->cAlphaFieldNames(ThisSurf + AlphaOffset),
                                           s_ipsc->cAlphaArgs(ThisSurf + AlphaOffset)));
                    ErrorsFound = true;
                    continue;
                }
                // check that surface is appropriate, Heat transfer, Sun, Wind,
                if (!state.dataSurface->Surface(Found).HeatTransSurf) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}",
                                           s_ipsc->cCurrentModuleObject,
                                           state.dataHeatBal->ExtVentedCavity(Item).Name,
                                           s_ipsc->cAlphaFieldNames(ThisSurf + AlphaOffset),
                                           s_ipsc->cAlphaArgs(ThisSurf + AlphaOffset)));
                    ShowContinueError(state, "...because it is not a Heat Transfer Surface.");
                    ErrorsFound = true;
                    continue;
                }
                if (!state.dataSurface->Surface(Found).ExtSolar) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}",
                                           s_ipsc->cCurrentModuleObject,
                                           state.dataHeatBal->ExtVentedCavity(Item).Name,
                                           s_ipsc->cAlphaFieldNames(ThisSurf + AlphaOffset),
                                           s_ipsc->cAlphaArgs(ThisSurf + AlphaOffset)));
                    ShowContinueError(state, "...because it is not exposed to Sun.");
                    ErrorsFound = true;
                    continue;
                }
                if (!state.dataSurface->Surface(Found).ExtWind) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}",
                                           s_ipsc->cCurrentModuleObject,
                                           state.dataHeatBal->ExtVentedCavity(Item).Name,
                                           s_ipsc->cAlphaFieldNames(ThisSurf + AlphaOffset),
                                           s_ipsc->cAlphaArgs(ThisSurf + AlphaOffset)));
                    ShowContinueError(state, "...because it is not exposed to Wind.");
                    ErrorsFound = true;
                    continue;
                }
                if (state.dataSurface->Surface(Found).ExtBoundCond != DataSurfaces::OtherSideCondModeledExt) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", is invalid", s_ipsc->cCurrentModuleObject, state.dataHeatBal->ExtVentedCavity(Item).Name));
                    ShowContinueError(state,
                                      format("...because {}=\"{}\".",
                                             s_ipsc->cAlphaFieldNames(ThisSurf + AlphaOffset),
                                             s_ipsc->cAlphaArgs(ThisSurf + AlphaOffset)));
                    ShowContinueError(state, "...is not an OtherSideConditionedModel surface.");
                    ErrorsFound = true;
                    continue;
                }
                state.dataHeatBal->ExtVentedCavity(Item).SurfPtrs(ThisSurf) = Found;

                // now set info in Surface structure
                state.dataSurface->SurfExtCavNum(Found) = Item;
                state.dataSurface->SurfExtCavityPresent(Found) = true;
            }

            if (ErrorsFound) {
                continue; // previous inner do loop may have detected problems that need to be cycle'd again to avoid crash
            }

            // now that we should have all the surfaces, do some preparations and checks.

            // are they all similar tilt and azimuth? Issue warnings so people can do it if they really want
            Real64 const surfaceArea(sum_sub(state.dataSurface->Surface, &SurfaceData::Area, state.dataHeatBal->ExtVentedCavity(Item).SurfPtrs));
            //            AvgAzimuth = sum( Surface( ExtVentedCavity( Item ).SurfPtrs ).Azimuth * Surface( ExtVentedCavity( Item ).SurfPtrs
            //).Area
            //)
            ///  sum(  Surface( ExtVentedCavity( Item ).SurfPtrs ).Area ); //Autodesk:F2C++ Array subscript usage: Replaced by below
            AvgAzimuth = sum_product_sub(state.dataSurface->Surface,
                                         &SurfaceData::Azimuth,
                                         &SurfaceData::Area,
                                         state.dataHeatBal->ExtVentedCavity(Item).SurfPtrs) /
                         surfaceArea; // Autodesk:F2C++ Functions handle array subscript usage
            //            AvgTilt = sum( Surface( ExtVentedCavity( Item ).SurfPtrs ).Tilt * Surface( ExtVentedCavity( Item ).SurfPtrs ).Area )
            //            /
            // sum(  Surface( ExtVentedCavity( Item ).SurfPtrs ).Area ); //Autodesk:F2C++ Array subscript usage: Replaced by below
            AvgTilt = sum_product_sub(
                          state.dataSurface->Surface, &SurfaceData::Tilt, &SurfaceData::Area, state.dataHeatBal->ExtVentedCavity(Item).SurfPtrs) /
                      surfaceArea; // Autodesk:F2C++ Functions handle array subscript usage
            for (ThisSurf = 1; ThisSurf <= state.dataHeatBal->ExtVentedCavity(Item).NumSurfs; ++ThisSurf) {
                SurfID = state.dataHeatBal->ExtVentedCavity(Item).SurfPtrs(ThisSurf);
                if (General::rotAzmDiffDeg(state.dataSurface->Surface(SurfID).Azimuth, AvgAzimuth) > AZITOL) {
                    ShowWarningError(state,
                                     format("{}=\"{}, Surface {} has Azimuth different from others in the associated group.",
                                            s_ipsc->cCurrentModuleObject,
                                            state.dataHeatBal->ExtVentedCavity(Item).Name,
                                            state.dataSurface->Surface(SurfID).Name));
                }
                if (std::abs(state.dataSurface->Surface(SurfID).Tilt - AvgTilt) > TILTOL) {
                    ShowWarningError(state,
                                     format("{}=\"{}, Surface {} has Tilt different from others in the associated group.",
                                            s_ipsc->cCurrentModuleObject,
                                            state.dataHeatBal->ExtVentedCavity(Item).Name,
                                            state.dataSurface->Surface(SurfID).Name));
                }

                // test that there are no windows.  Now allow windows
                // If (Surface(SurfID)%GrossArea >  Surface(SurfID)%Area) Then
                //      Call ShowWarningError(state, 'Surface '//TRIM(Surface(SurfID)%name)//' has a subsurface whose area is not being ' &
                //         //'subtracted in the group of surfaces associated with '//TRIM(ExtVentedCavity(Item)%Name))
                // endif
            }
            state.dataHeatBal->ExtVentedCavity(Item).Tilt = AvgTilt;
            state.dataHeatBal->ExtVentedCavity(Item).Azimuth = AvgAzimuth;

            // find area weighted centroid.
            //            ExtVentedCavity( Item ).Centroid.z = sum( Surface( ExtVentedCavity( Item ).SurfPtrs ).Centroid.z * Surface(
            // ExtVentedCavity(  Item
            //).SurfPtrs ).Area ) / sum( Surface( ExtVentedCavity( Item ).SurfPtrs ).Area ); //Autodesk:F2C++ Array subscript usage: Replaced
            // by below
            state.dataHeatBal->ExtVentedCavity(Item).Centroid.z = sum_product_sub(state.dataSurface->Surface,
                                                                                  &SurfaceData::Centroid,
                                                                                  &Vector::z,
                                                                                  state.dataSurface->Surface,
                                                                                  &SurfaceData::Area,
                                                                                  state.dataHeatBal->ExtVentedCavity(Item).SurfPtrs) /
                                                                  surfaceArea; // Autodesk:F2C++ Functions handle array subscript usage

            // now handle rNumericArgs from input object
            state.dataHeatBal->ExtVentedCavity(Item).Porosity = s_ipsc->rNumericArgs(1);
            state.dataHeatBal->ExtVentedCavity(Item).LWEmitt = s_ipsc->rNumericArgs(2);
            state.dataHeatBal->ExtVentedCavity(Item).SolAbsorp = s_ipsc->rNumericArgs(3);
            state.dataHeatBal->ExtVentedCavity(Item).HdeltaNPL = s_ipsc->rNumericArgs(4);
            state.dataHeatBal->ExtVentedCavity(Item).PlenGapThick = s_ipsc->rNumericArgs(5);
            if (state.dataHeatBal->ExtVentedCavity(Item).PlenGapThick <= 0.0) {
                ShowSevereError(state, format("{}=\"{}\", invalid .", s_ipsc->cCurrentModuleObject, state.dataHeatBal->ExtVentedCavity(Item).Name));
                ErrorsFound = true;
                ShowContinueError(
                    state,
                    format("...because field \"{}\" must be greater than Zero=[{:.2T}].", s_ipsc->cNumericFieldNames(5), s_ipsc->rNumericArgs(5)));
                continue;
            }
            state.dataHeatBal->ExtVentedCavity(Item).AreaRatio = s_ipsc->rNumericArgs(6);
            state.dataHeatBal->ExtVentedCavity(Item).Cv = s_ipsc->rNumericArgs(7);
            state.dataHeatBal->ExtVentedCavity(Item).Cd = s_ipsc->rNumericArgs(8);

            // Fill out data we now know
            // sum areas of HT surface areas
            //            ExtVentedCavity( Item ).ProjArea = sum( Surface( ExtVentedCavity( Item ).SurfPtrs ).Area ); //Autodesk:F2C++ Array
            // subscript  usage: Replaced by below
            state.dataHeatBal->ExtVentedCavity(Item).ProjArea = surfaceArea;
            if (state.dataHeatBal->ExtVentedCavity(Item).ProjArea <= 0.0) {
                ShowSevereError(state, format("{}=\"{}\", invalid .", s_ipsc->cCurrentModuleObject, state.dataHeatBal->ExtVentedCavity(Item).Name));
                ErrorsFound = true;
                ShowContinueError(state,
                                  format("...because gross area of underlying surfaces must be greater than Zero=[{:.2T}].",
                                         state.dataHeatBal->ExtVentedCavity(Item).ProjArea));
                continue;
            }
            state.dataHeatBal->ExtVentedCavity(Item).ActualArea =
                state.dataHeatBal->ExtVentedCavity(Item).ProjArea * state.dataHeatBal->ExtVentedCavity(Item).AreaRatio;

            SetupOutputVariable(state,
                                "Surface Exterior Cavity Baffle Surface Temperature",
                                Constant::Units::C,
                                state.dataHeatBal->ExtVentedCavity(Item).Tbaffle,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ExtVentedCavity(Item).Name);
            SetupOutputVariable(state,
                                "Surface Exterior Cavity Air Drybulb Temperature",
                                Constant::Units::C,
                                state.dataHeatBal->ExtVentedCavity(Item).TAirCav,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ExtVentedCavity(Item).Name);
            SetupOutputVariable(state,
                                "Surface Exterior Cavity Total Natural Ventilation Air Change Rate",
                                Constant::Units::ach,
                                state.dataHeatBal->ExtVentedCavity(Item).PassiveACH,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ExtVentedCavity(Item).Name);
            SetupOutputVariable(state,
                                "Surface Exterior Cavity Total Natural Ventilation Mass Flow Rate",
                                Constant::Units::kg_s,
                                state.dataHeatBal->ExtVentedCavity(Item).PassiveMdotVent,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ExtVentedCavity(Item).Name);
            SetupOutputVariable(state,
                                "Surface Exterior Cavity Natural Ventilation from Wind Mass Flow Rate",
                                Constant::Units::kg_s,
                                state.dataHeatBal->ExtVentedCavity(Item).PassiveMdotWind,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ExtVentedCavity(Item).Name);
            SetupOutputVariable(state,
                                "Surface Exterior Cavity Natural Ventilation from Buoyancy Mass Flow Rate",
                                Constant::Units::kg_s,
                                state.dataHeatBal->ExtVentedCavity(Item).PassiveMdotTherm,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ExtVentedCavity(Item).Name);
        }
    }

    void ExposedFoundationPerimeter::getData(EnergyPlusData &state, bool &ErrorsFound)
    {

        static constexpr std::string_view routineName = "ExposedFoundationPerimeter::getData";
        int IOStatus; // Used in GetObjectItem
        int NumAlphas;
        int NumNumbers;

        Real64 constexpr tolerance = 1e-6;

        auto &s_ipsc = state.dataIPShortCut;

        s_ipsc->cCurrentModuleObject = "SurfaceProperty:ExposedFoundationPerimeter";
        int numObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        for (int obj = 1; obj <= numObjects; ++obj) {
            int alpF = 1;
            int numF = 1;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     obj,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(alpF)};

            int Found = Util::FindItemInList(s_ipsc->cAlphaArgs(alpF), state.dataSurface->Surface, state.dataSurface->TotSurfaces);
            if (Found == 0) {
                ShowSevereError(state, format("{}=\"{}\", did not find matching surface", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            }
            alpF++;
            if (state.dataSurface->Surface(Found).Class != SurfaceClass::Floor) {
                ShowWarningError(
                    state, format("{}: {}, surface is not a floor surface", s_ipsc->cCurrentModuleObject, state.dataSurface->Surface(Found).Name));
                ShowContinueError(state, format("{} will not be used", s_ipsc->cCurrentModuleObject));
                continue;
            }

            // Choose calculation method

            enum class CalculationMethod
            {
                Invalid = -1,
                TotalExposedPerimeter,
                ExposedPerimeterFraction,
                Bysegment,
                Num
            };

            constexpr std::array<std::string_view, static_cast<int>(CalculationMethod::Num)> CalculationMethodUC = {
                "TOTALEXPOSEDPERIMETER", "EXPOSEDPERIMETERFRACTION", "BYSEGMENT"};
            CalculationMethod calculationMethod = static_cast<CalculationMethod>(getEnumValue(CalculationMethodUC, s_ipsc->cAlphaArgs(alpF)));
            if (calculationMethod != CalculationMethod::TotalExposedPerimeter && calculationMethod != CalculationMethod::ExposedPerimeterFraction &&
                calculationMethod != CalculationMethod::Bysegment) {
                ShowSevereError(state,
                                format("{}=\"{}\", {} is not a valid choice for {}",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1),
                                       calculationMethod,
                                       s_ipsc->cAlphaFieldNames(alpF)));
                ErrorsFound = true;
            }
            alpF++;

            Data data;
            data.useDetailedExposedPerimeter = true;

            if (!s_ipsc->lNumericFieldBlanks(numF)) {
                if (calculationMethod == CalculationMethod::TotalExposedPerimeter) {
                    data.exposedFraction = s_ipsc->rNumericArgs(numF) / state.dataSurface->Surface(Found).Perimeter;
                    if (data.exposedFraction > 1 + tolerance) {
                        ShowWarningError(state,
                                         format("{}: {}, {} is greater than the perimeter of {}",
                                                s_ipsc->cCurrentModuleObject,
                                                state.dataSurface->Surface(Found).Name,
                                                s_ipsc->cNumericFieldNames(numF),
                                                state.dataSurface->Surface(Found).Name));
                        ShowContinueError(state,
                                          format("{} perimeter = {}, {} exposed perimeter = {}",
                                                 state.dataSurface->Surface(Found).Name,
                                                 state.dataSurface->Surface(Found).Perimeter,
                                                 s_ipsc->cCurrentModuleObject,
                                                 s_ipsc->rNumericArgs(numF)));
                        ShowContinueError(
                            state,
                            format("{} will be set equal to {} perimeter", s_ipsc->cNumericFieldNames(numF), state.dataSurface->Surface(Found).Name));
                        data.exposedFraction = 1.0;
                    }

                    data.useDetailedExposedPerimeter = false;
                } else {
                    ShowWarningError(state,
                                     format("{}: {}, {} set as calculation method, but a value has been set for {}. This value will be ignored.",
                                            s_ipsc->cCurrentModuleObject,
                                            state.dataSurface->Surface(Found).Name,
                                            calculationMethod,
                                            s_ipsc->cNumericFieldNames(numF)));
                }
            } else {
                if (calculationMethod == CalculationMethod::TotalExposedPerimeter) {
                    ShowSevereError(state,
                                    format("{}: {}, {} set as calculation method, but no value has been set for {}",
                                           s_ipsc->cCurrentModuleObject,
                                           state.dataSurface->Surface(Found).Name,
                                           calculationMethod,
                                           s_ipsc->cNumericFieldNames(numF)));
                    ErrorsFound = true;
                }
            }
            numF++;

            if (!s_ipsc->lNumericFieldBlanks(numF)) {
                if (calculationMethod == CalculationMethod::ExposedPerimeterFraction) {
                    data.exposedFraction = s_ipsc->rNumericArgs(numF);
                    data.useDetailedExposedPerimeter = false;
                } else {
                    ShowWarningError(state,
                                     format("{}: {}, {} set as calculation method, but a value has been set for {}. This value will be ignored.",
                                            s_ipsc->cCurrentModuleObject,
                                            state.dataSurface->Surface(Found).Name,
                                            calculationMethod,
                                            s_ipsc->cNumericFieldNames(numF)));
                }
            } else {
                if (calculationMethod == CalculationMethod::ExposedPerimeterFraction) {
                    ShowSevereError(state,
                                    format("{}: {}, {} set as calculation method, but no value has been set for {}",
                                           s_ipsc->cCurrentModuleObject,
                                           state.dataSurface->Surface(Found).Name,
                                           calculationMethod,
                                           s_ipsc->cNumericFieldNames(numF)));
                    ErrorsFound = true;
                }
            }
            numF++;

            int numRemainingFields = NumAlphas - (alpF - 1) + NumNumbers - (numF - 1);
            if (numRemainingFields > 0) {
                if (calculationMethod == CalculationMethod::Bysegment) {
                    if (numRemainingFields != (int)state.dataSurface->Surface(Found).Vertex.size()) {
                        ShowSevereError(state,
                                        format("{}: {}, must have equal number of segments as the floor has vertices.{}\" and \"{}\"",
                                               s_ipsc->cCurrentModuleObject,
                                               state.dataSurface->Surface(Found).Name,
                                               s_ipsc->cAlphaFieldNames(alpF),
                                               s_ipsc->cNumericFieldNames(numF - 1)));
                        ShowContinueError(state,
                                          format("{} number of vertices = {}, {} number of segments = {}",
                                                 state.dataSurface->Surface(Found).Name,
                                                 state.dataSurface->Surface(Found).Vertex.size(),
                                                 s_ipsc->cCurrentModuleObject,
                                                 numRemainingFields));
                        ErrorsFound = true;
                    }
                    for (int segNum = 0; segNum < numRemainingFields; segNum++) {
                        if (s_ipsc->lAlphaFieldBlanks(alpF)) {
                            ShowSevereEmptyField(
                                state, eoh, s_ipsc->cAlphaFieldNames(alpF), "Calculation Method", CalculationMethodUC[(int)calculationMethod]);
                            ErrorsFound = true;
                        } else if (BooleanSwitch bs = getYesNoValue(s_ipsc->cAlphaArgs(alpF)); bs != BooleanSwitch::Invalid) {
                            data.isExposedPerimeter.push_back(static_cast<bool>(bs));
                        } else {
                            ShowSevereInvalidBool(state, eoh, s_ipsc->cAlphaFieldNames(alpF), s_ipsc->cAlphaArgs(alpF));
                            ErrorsFound = true;
                        }
                        alpF++;
                    }
                }
            } else {
                if (calculationMethod == CalculationMethod::Bysegment) {
                    ShowSevereError(state,
                                    format("{}: {}, {} set as calculation method, but no values have been set for Surface Segments Exposed",
                                           s_ipsc->cCurrentModuleObject,
                                           state.dataSurface->Surface(Found).Name,
                                           calculationMethod));
                    ErrorsFound = true;
                }
            }
            surfaceMap[Found] = data;
        }
    }

    void GetSurfaceLocalEnvData(EnergyPlusData &state, bool &ErrorsFound) // Error flag indicator (true if errors found)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         X LUO
        //       DATE WRITTEN   July 2017

        // PURPOSE OF THIS SUBROUTINE:
        // load input data for Outdoor Air Node for exterior surfaces

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetSurfaceLocalEnvData: ");
        static constexpr std::string_view routineName = "GetSurfaceLocalEnvData";

        auto &s_ipsc = state.dataIPShortCut;

        s_ipsc->cCurrentModuleObject = "SurfaceProperty:LocalEnvironment";
        state.dataSurface->TotSurfLocalEnv = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        if (state.dataSurface->TotSurfLocalEnv > 0) {
            int NumAlpha;
            int NumNumeric;
            int IOStat;

            state.dataGlobal->AnyLocalEnvironmentsInModel = true;

            if (!allocated(state.dataSurface->SurfLocalEnvironment)) {
                state.dataSurface->SurfLocalEnvironment.allocate(state.dataSurface->TotSurfLocalEnv);
            }

            for (int Loop = 1; Loop <= state.dataSurface->TotSurfLocalEnv; ++Loop) {

                auto &SurfLocalEnv = state.dataSurface->SurfLocalEnvironment(Loop);

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         s_ipsc->cCurrentModuleObject,
                                                                         Loop,
                                                                         s_ipsc->cAlphaArgs,
                                                                         NumAlpha,
                                                                         s_ipsc->rNumericArgs,
                                                                         NumNumeric,
                                                                         IOStat,
                                                                         s_ipsc->lNumericFieldBlanks,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

                Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);

                SurfLocalEnv.Name = s_ipsc->cAlphaArgs(1);

                // Assign surface number
                int SurfNum = Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataSurface->Surface);
                if (SurfNum == 0) {
                    ShowSevereError(state,
                                    format("{} {} = \"{}\", object. Illegal value for \"{}\" has been found.",
                                           RoutineName,
                                           s_ipsc->cCurrentModuleObject,
                                           SurfLocalEnv.Name,
                                           s_ipsc->cAlphaFieldNames(2)));
                    ShowContinueError(state,
                                      format("{} entered value = \"{}\", no corresponding surface (ref BuildingSurface:Detailed) has been "
                                             "found in the input file.",
                                             s_ipsc->cAlphaFieldNames(2),
                                             s_ipsc->cAlphaArgs(2)));
                    ErrorsFound = true;
                } else {
                    SurfLocalEnv.SurfPtr = SurfNum;
                }

                // Assign Sunlit Fraction Schedule number
                if (s_ipsc->lAlphaFieldBlanks(3)) {
                } else if ((SurfLocalEnv.sunlitFracSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                    ErrorsFound = true;
                }

                // Assign surrounding surfaces object number;
                if (!s_ipsc->lAlphaFieldBlanks(4)) {
                    int SurroundingSurfsNum = Util::FindItemInList(s_ipsc->cAlphaArgs(4), state.dataSurface->SurroundingSurfsProperty);
                    if (SurroundingSurfsNum == 0) {
                        ShowSevereError(state,
                                        format("{} {} = \"{}\", object. Illegal value for \"{}\" has been found.",
                                               RoutineName,
                                               s_ipsc->cCurrentModuleObject,
                                               SurfLocalEnv.Name,
                                               s_ipsc->cAlphaFieldNames(4)));
                        ShowContinueError(state,
                                          format("{} entered value = \"{}\", no corresponding surrounding surfaces properties has been found "
                                                 "in the input file.",
                                                 s_ipsc->cAlphaFieldNames(4),
                                                 s_ipsc->cAlphaArgs(4)));
                        ErrorsFound = true;
                    } else {
                        SurfLocalEnv.SurroundingSurfsPtr = SurroundingSurfsNum;
                    }
                }

                // Assign outdoor air node number;
                if (!s_ipsc->lAlphaFieldBlanks(5)) {
                    int NodeNum = GetOnlySingleNode(state,
                                                    s_ipsc->cAlphaArgs(5),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::SurfacePropertyLocalEnvironment,
                                                    SurfLocalEnv.Name,
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);
                    if (NodeNum == 0 && OutAirNodeManager::CheckOutAirNodeNumber(state, NodeNum)) {
                        ShowSevereError(state,
                                        format("{} {} = \"{}\", object. Illegal value for \"{}\" has been found.",
                                               RoutineName,
                                               s_ipsc->cCurrentModuleObject,
                                               SurfLocalEnv.Name,
                                               s_ipsc->cAlphaFieldNames(5)));
                        ShowContinueError(state,
                                          format("{} entered value = \"{}\", no corresponding outdoor air node has been found in the input file.",
                                                 s_ipsc->cAlphaFieldNames(5),
                                                 s_ipsc->cAlphaArgs(5)));
                        ErrorsFound = true;
                    } else {
                        SurfLocalEnv.OutdoorAirNodePtr = NodeNum;
                    }
                }

                // get ground surfaces object number;
                if (!s_ipsc->lAlphaFieldBlanks(6)) {
                    int GndSurfsNum = Util::FindItemInList(s_ipsc->cAlphaArgs(6), state.dataSurface->GroundSurfsProperty);
                    if (GndSurfsNum == 0) {
                        ShowSevereError(state,
                                        format("{} {} = \"{}\", object. Illegal value for \"{}\" has been found.",
                                               RoutineName,
                                               s_ipsc->cCurrentModuleObject,
                                               SurfLocalEnv.Name,
                                               s_ipsc->cAlphaFieldNames(6)));
                        ShowContinueError(
                            state,
                            format("{} entered value = \"{}\", no corresponding ground surfaces object has been found in the input file.",
                                   s_ipsc->cAlphaFieldNames(6),
                                   s_ipsc->cAlphaArgs(6)));
                        ErrorsFound = true;
                    } else {
                        SurfLocalEnv.GroundSurfsPtr = GndSurfsNum;
                    }
                }
            }
        }
        // Link surface properties to surface object
        for (int SurfLoop = 1; SurfLoop <= state.dataSurface->TotSurfaces; ++SurfLoop) {
            for (int Loop = 1; Loop <= state.dataSurface->TotSurfLocalEnv; ++Loop) {
                auto const &SurfLocalEnv = state.dataSurface->SurfLocalEnvironment(Loop);
                if (SurfLocalEnv.SurfPtr == SurfLoop) {
                    auto &surface = state.dataSurface->Surface(SurfLoop);
                    if (SurfLocalEnv.OutdoorAirNodePtr != 0) {
                        surface.SurfLinkedOutAirNode = SurfLocalEnv.OutdoorAirNodePtr;
                    }
                    if (SurfLocalEnv.sunlitFracSched != nullptr) {
                        surface.SurfSchedExternalShadingFrac = true;
                        surface.surfExternalShadingSched = SurfLocalEnv.sunlitFracSched;
                    }
                    if (SurfLocalEnv.SurroundingSurfsPtr != 0) {
                        surface.SurfHasSurroundingSurfProperty = true;
                        surface.SurfSurroundingSurfacesNum = SurfLocalEnv.SurroundingSurfsPtr;
                        surface.ViewFactorSrdSurfs =
                            state.dataSurface->SurroundingSurfsProperty(surface.SurfSurroundingSurfacesNum).SurfsViewFactorSum;
                        if (surface.ViewFactorSrdSurfs == 0.0) {
                            surface.SurfHasSurroundingSurfProperty = false;
                        }
                    }
                    if (SurfLocalEnv.GroundSurfsPtr != 0) {
                        surface.IsSurfPropertyGndSurfacesDefined = true;
                        surface.UseSurfPropertyGndSurfTemp = true;
                        surface.UseSurfPropertyGndSurfRefl = true;
                        surface.SurfPropertyGndSurfIndex = SurfLocalEnv.GroundSurfsPtr;
                    }
                }
            }
        }
    }

    void GetSurfaceSrdSurfsData(EnergyPlusData &state, bool &ErrorsFound) // Error flag indicator (true if errors found)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         X LUO
        //       DATE WRITTEN   July 2017

        // PURPOSE OF THIS SUBROUTINE:
        // load input data for surrounding surfaces properties for exterior surfaces
        constexpr std::string_view routineName = "GetSurfaceSrdSurfsData";

        auto &s_ipsc = state.dataIPShortCut;

        s_ipsc->cCurrentModuleObject = "SurfaceProperty:SurroundingSurfaces";
        int TotSrdSurfProperties = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        if (TotSrdSurfProperties > 0) {
            int NumAlpha;
            int NumNumeric;
            int IOStat;

            if (!allocated(state.dataSurface->SurroundingSurfsProperty)) {
                state.dataSurface->SurroundingSurfsProperty.allocate(TotSrdSurfProperties);
            }

            for (int Loop = 1; Loop <= TotSrdSurfProperties; ++Loop) {

                auto &SrdSurfsProp = state.dataSurface->SurroundingSurfsProperty(Loop);

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         s_ipsc->cCurrentModuleObject,
                                                                         Loop,
                                                                         s_ipsc->cAlphaArgs,
                                                                         NumAlpha,
                                                                         s_ipsc->rNumericArgs,
                                                                         NumNumeric,
                                                                         IOStat,
                                                                         s_ipsc->lNumericFieldBlanks,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

                Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);

                // A1: Name
                SrdSurfsProp.Name = s_ipsc->cAlphaArgs(1);

                // N1: sky view factor
                if (!s_ipsc->lNumericFieldBlanks(1)) {
                    SrdSurfsProp.SkyViewFactor = s_ipsc->rNumericArgs(1);
                    SrdSurfsProp.IsSkyViewFactorSet = true;
                }

                // A2: sky temp sch name
                if (s_ipsc->lAlphaFieldBlanks(2)) {
                } else if ((SrdSurfsProp.skyTempSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
                    ShowWarningItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                }

                // N2: ground view factor
                if (!s_ipsc->lNumericFieldBlanks(2)) {
                    SrdSurfsProp.GroundViewFactor = s_ipsc->rNumericArgs(2);
                    SrdSurfsProp.IsGroundViewFactorSet = true;
                }

                // A3: ground temp sch name
                if (s_ipsc->lAlphaFieldBlanks(3)) {
                } else if ((SrdSurfsProp.groundTempSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
                    ShowWarningItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                }

                // The object requires at least one srd surface input, each surface requires a set of 3 fields (2 Alpha fields Name and Temp
                // Sch Name and 1 Num fields View Factor)
                if (NumAlpha < 5) {
                    ShowSevereError(state, format("{} = \"{}\" is not defined correctly.", s_ipsc->cCurrentModuleObject, SrdSurfsProp.Name));
                    ShowContinueError(state, "At lease one set of surrounding surface properties should be defined.");
                    ErrorsFound = true;
                    continue;
                }
                if ((NumAlpha - 3) / 2 != (NumNumeric - 2)) {
                    ShowSevereError(state, format("{} = \"{}\" is not defined correctly.", s_ipsc->cCurrentModuleObject, SrdSurfsProp.Name));
                    ShowContinueError(state, "Check number of input fields for each surrounding surface.");
                    ErrorsFound = true;
                    continue;
                }
                // Read surrounding surfaces properties
                SrdSurfsProp.TotSurroundingSurface = NumNumeric - 2;
                SrdSurfsProp.SurroundingSurfs.allocate(SrdSurfsProp.TotSurroundingSurface);
                for (int SurfLoop = 1; SurfLoop <= SrdSurfsProp.TotSurroundingSurface; ++SurfLoop) {
                    auto &surroundSurf = SrdSurfsProp.SurroundingSurfs(SurfLoop);
                    surroundSurf.Name = s_ipsc->cAlphaArgs(SurfLoop * 2 + 2);
                    surroundSurf.ViewFactor = s_ipsc->rNumericArgs(SurfLoop + 2);

                    // Added checking
                    if (s_ipsc->lAlphaFieldBlanks(SurfLoop * 2 + 3)) {
                        ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(SurfLoop * 2 + 3));
                        ErrorsFound = true;
                    } else if ((surroundSurf.tempSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(SurfLoop * 2 + 3))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(SurfLoop * 2 + 3), s_ipsc->cAlphaArgs(SurfLoop * 2 + 3));
                        ErrorsFound = true;
                    }
                    SrdSurfsProp.SurfsViewFactorSum += SrdSurfsProp.SurroundingSurfs(SurfLoop).ViewFactor;
                }
            }
        }
    }

    void GetSurfaceGroundSurfsData(EnergyPlusData &state, bool &ErrorsFound)
    {
        static constexpr std::string_view routineName = "GetSurfaceGroundSurfsData";

        auto &s_ipsc = state.dataIPShortCut;
        s_ipsc->cCurrentModuleObject = "SurfaceProperty:GroundSurfaces";
        state.dataSurface->TotSurfPropGndSurfs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        auto const instances = state.dataInputProcessing->inputProcessor->epJSON.find(s_ipsc->cCurrentModuleObject);
        if (instances == state.dataInputProcessing->inputProcessor->epJSON.end()) {
            if (state.dataSurface->TotSurfPropGndSurfs > 0) {
                ErrorsFound = true;
            }
            return;
        }

        auto &instancesValue = instances.value();
        for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
            auto const &fields = instance.value();
            std::string const &thisObjectName = instance.key();

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, thisObjectName};

            DataSurfaces::GroundSurfacesProperty thisGndSurfsObj;
            thisGndSurfsObj.Name = Util::makeUPPER(thisObjectName);
            state.dataInputProcessing->inputProcessor->markObjectAsUsed(s_ipsc->cCurrentModuleObject, thisObjectName);
            auto groundSurfaces = fields.find("ground_surfaces");
            if (groundSurfaces != fields.end()) {
                auto &groundSurfacesArray = groundSurfaces.value();
                thisGndSurfsObj.NumGndSurfs = groundSurfacesArray.size();
                for (auto &groundSurface : groundSurfacesArray) {
                    DataSurfaces::GroundSurfacesData thisGndSurf;
                    auto GndSurfName = groundSurface.find("ground_surface_name");
                    if (GndSurfName != groundSurface.end()) {
                        std::string ground_surf_name = GndSurfName.value().get<std::string>();
                        if (!ground_surf_name.empty()) {
                            thisGndSurf.Name = Util::makeUPPER(ground_surf_name);
                        }
                    }
                    auto groundSurfViewFactor = groundSurface.find("ground_surface_view_factor");
                    if (groundSurfViewFactor != groundSurface.end()) {
                        thisGndSurf.ViewFactor = groundSurfViewFactor.value().get<Real64>();
                        thisGndSurfsObj.IsGroundViewFactorSet = true;
                    }
                    auto TempSchName = groundSurface.find("ground_surface_temperature_schedule_name");
                    if (TempSchName != groundSurface.end()) {
                        std::string schedName = TempSchName.value().get<std::string>();
                        if (schedName.empty()) {
                        } else if ((thisGndSurf.tempSched = Sched::GetSchedule(state, Util::makeUPPER(schedName))) == nullptr) {
                            ShowSevereItemNotFound(state, eoh, "Ground Surface Temperature Schedule Name", schedName);
                            ErrorsFound = true;
                        }
                    }

                    auto ReflSchName = groundSurface.find("ground_surface_reflectance_schedule_name");
                    if (ReflSchName != groundSurface.end()) {
                        std::string schedName = ReflSchName.value().get<std::string>();
                        if (schedName.empty()) {
                        } else if ((thisGndSurf.reflSched = Sched::GetSchedule(state, Util::makeUPPER(schedName))) == nullptr) {
                            ShowSevereItemNotFound(state, eoh, "Ground Surface Reflectance Schedule Name", schedName);
                            ErrorsFound = true;
                        }
                    }
                    thisGndSurfsObj.GndSurfs.push_back(thisGndSurf);
                }
            }
            for (int gSurfNum = 1; gSurfNum <= thisGndSurfsObj.NumGndSurfs; gSurfNum++) {
                thisGndSurfsObj.SurfsViewFactorSum += thisGndSurfsObj.GndSurfs(gSurfNum).ViewFactor;
            }
            state.dataSurface->GroundSurfsProperty.push_back(thisGndSurfsObj);
        } // for (instance)

        // set report variables
        if (state.dataSurface->TotSurfPropGndSurfs > 0) {
            for (int Loop = 1; Loop <= state.dataSurface->TotSurfPropGndSurfs; Loop++) {
                bool SetTempSchReportVar = true;
                bool SetReflSchReportVar = true;
                auto &thisGndSurfsObj = state.dataSurface->GroundSurfsProperty(Loop);
                for (int gSurfNum = 1; gSurfNum <= thisGndSurfsObj.NumGndSurfs; gSurfNum++) {
                    if (thisGndSurfsObj.GndSurfs(gSurfNum).tempSched != nullptr && SetTempSchReportVar) {
                        SetupOutputVariable(state,
                                            "Surfaces Property Ground Surfaces Average Temperature",
                                            Constant::Units::C,
                                            thisGndSurfsObj.SurfsTempAvg,
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            thisGndSurfsObj.Name);
                        SetTempSchReportVar = false;
                    }
                    if (thisGndSurfsObj.GndSurfs(gSurfNum).reflSched != nullptr && SetReflSchReportVar) {
                        SetupOutputVariable(state,
                                            "Surfaces Property Ground Surfaces Average Reflectance",
                                            Constant::Units::None,
                                            thisGndSurfsObj.SurfsReflAvg,
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            thisGndSurfsObj.Name);
                        SetReflSchReportVar = false;
                    }
                }
            }
        }
    }

    void GetSurfaceHeatTransferAlgorithmOverrides(EnergyPlusData &state, bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith, portions from ApplyConvectionValue by Linda Lawrie
        //       DATE WRITTEN   July 2012
        static constexpr std::string_view routineName = "GetSurfaceHeatTransferAlgorithmOverrides";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int CountHTAlgoObjectsSingleSurf;
        int CountHTAlgoObjectsMultiSurf;
        int CountHTAlgoObjectsSurfList;
        int IOStatus; // Used in GetObjectItem
        DataSurfaces::HeatTransferModel tmpAlgoInput;
        int Item;
        int Item1;
        int NumAlphas;
        int NumNumbers;
        int Found;
        bool SurfacesOfType;
        int SurfNum;
        //  INTEGER :: Index
        int NumEMPDMat;
        int NumPCMat;
        int NumVTCMat;
        int NumHAMTMat1;
        int NumHAMTMat2;
        int NumHAMTMat3;
        int NumHAMTMat4;
        int NumHAMTMat5;
        int NumHAMTMat6;
        int SumHAMTMat;
        bool msgneeded;

        auto &s_ipsc = state.dataIPShortCut;
        s_ipsc->cCurrentModuleObject = "SurfaceProperty:HeatBalanceSourceTerm";
        int CountAddHeatSourceSurf = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        for (Item = 1; Item <= CountAddHeatSourceSurf; ++Item) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Item,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};
            Found = Util::FindItemInList(s_ipsc->cAlphaArgs(1), state.dataSurface->Surface, state.dataSurface->TotSurfaces);

            if (Found == 0) {
                ShowSevereError(state, format("{}=\"{}\", did not find matching surface.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            } else if (state.dataSurface->Surface(Found).insideHeatSourceTermSched != nullptr ||
                       state.dataSurface->Surface(Found).outsideHeatSourceTermSched != nullptr) {
                ShowSevereError(state,
                                format("{}=\"{}\", multiple SurfaceProperty:HeatBalanceSourceTerm objects applied to the same surface.",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            }

            if (s_ipsc->lAlphaFieldBlanks(2)) {
            } else if ((state.dataSurface->Surface(Found).insideHeatSourceTermSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                ErrorsFound = true;
            } else {
                state.dataSurface->allInsideSourceSurfaceList.emplace_back(Found);
            }

            if (s_ipsc->lAlphaFieldBlanks(3)) {
            } else if ((state.dataSurface->Surface(Found).outsideHeatSourceTermSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            } else if (state.dataSurface->Surface(Found).OSCPtr > 0) {
                ShowSevereError(state,
                                format("{}=\"SurfaceProperty:HeatBalanceSourceTerm\", cannot be specified for OtherSideCoefficient Surface={}",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            } else {
                state.dataSurface->allOutsideSourceSurfaceList.emplace_back(Found);
            }

            if (state.dataSurface->Surface(Found).outsideHeatSourceTermSched == nullptr &&
                state.dataSurface->Surface(Found).insideHeatSourceTermSched == nullptr) {
                ShowSevereError(
                    state, format("{}=\"{}\", no schedule defined for additional heat source.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            }
        }

        // first initialize each heat transfer surface with the overall model type, array assignment
        for (auto &e : state.dataSurface->Surface) {
            e.HeatTransferAlgorithm = state.dataHeatBal->OverallHeatTransferSolutionAlgo;
        }

        s_ipsc->cCurrentModuleObject = "SurfaceProperty:HeatTransferAlgorithm";
        CountHTAlgoObjectsSingleSurf = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        s_ipsc->cCurrentModuleObject = "SurfaceProperty:HeatTransferAlgorithm";
        for (Item = 1; Item <= CountHTAlgoObjectsSingleSurf; ++Item) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Item,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);
            bool ErrorsFoundSingleSurf = false;
            Found = Util::FindItemInList(s_ipsc->cAlphaArgs(1), state.dataSurface->Surface, state.dataSurface->TotSurfaces);

            if (Found == 0) {
                ShowSevereError(state, format("{}=\"{}\", did not find matching surface.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ErrorsFoundSingleSurf = true;
            }

            {
                std::string const &SELECT_CASE_var = s_ipsc->cAlphaArgs(2);

                if (SELECT_CASE_var == "CONDUCTIONTRANSFERFUNCTION") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::CTF;
                    state.dataHeatBal->AnyCTF = true;
                } else if (SELECT_CASE_var == "MOISTUREPENETRATIONDEPTHCONDUCTIONTRANSFERFUNCTION") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::EMPD;
                    state.dataHeatBal->AnyEMPD = true;
                } else if (SELECT_CASE_var == "COMBINEDHEATANDMOISTUREFINITEELEMENT") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::HAMT;
                    state.dataHeatBal->AnyHAMT = true;
                } else if (SELECT_CASE_var == "CONDUCTIONFINITEDIFFERENCE") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::CondFD;
                    state.dataHeatBal->AnyCondFD = true;
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}",
                                           s_ipsc->cCurrentModuleObject,
                                           s_ipsc->cAlphaArgs(1),
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ErrorsFoundSingleSurf = true;
                }
            }

            if (!ErrorsFoundSingleSurf) {
                state.dataSurface->Surface(Found).HeatTransferAlgorithm = tmpAlgoInput;
            } else {
                ErrorsFound = true;
            }
        } // single surface heat transfer algorithm override

        s_ipsc->cCurrentModuleObject = "SurfaceProperty:HeatTransferAlgorithm:MultipleSurface";
        CountHTAlgoObjectsMultiSurf = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        for (Item = 1; Item <= CountHTAlgoObjectsMultiSurf; ++Item) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Item,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);
            bool ErrorsFoundMultiSurf = false;
            {
                std::string const &SELECT_CASE_var = s_ipsc->cAlphaArgs(3);

                if (SELECT_CASE_var == "CONDUCTIONTRANSFERFUNCTION") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::CTF;
                    state.dataHeatBal->AnyCTF = true;
                } else if (SELECT_CASE_var == "MOISTUREPENETRATIONDEPTHCONDUCTIONTRANSFERFUNCTION") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::EMPD;
                    state.dataHeatBal->AnyEMPD = true;
                } else if (SELECT_CASE_var == "COMBINEDHEATANDMOISTUREFINITEELEMENT") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::HAMT;
                    state.dataHeatBal->AnyHAMT = true;
                } else if (SELECT_CASE_var == "CONDUCTIONFINITEDIFFERENCE") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::CondFD;
                    state.dataHeatBal->AnyCondFD = true;
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}",
                                           s_ipsc->cCurrentModuleObject,
                                           s_ipsc->cAlphaArgs(1),
                                           s_ipsc->cAlphaFieldNames(3),
                                           s_ipsc->cAlphaArgs(3)));
                    ErrorsFoundMultiSurf = true;
                }
            }

            {
                std::string const &SELECT_CASE_var = s_ipsc->cAlphaArgs(2);

                if (SELECT_CASE_var == "ALLEXTERIORSURFACES") {
                    SurfacesOfType = false;
                    for (SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                        auto &surf = state.dataSurface->Surface(SurfNum);
                        if (!surf.HeatTransSurf) {
                            continue;
                        }
                        if (surf.ExtBoundCond > 0) {
                            continue; // Interior surfaces
                        }
                        if (state.dataConstruction->Construct(surf.Construction).TypeIsWindow) {
                            continue;
                        }
                        SurfacesOfType = true;
                        surf.HeatTransferAlgorithm = tmpAlgoInput;
                    }

                } else if (SELECT_CASE_var == "ALLEXTERIORWALLS") {
                    SurfacesOfType = false;
                    for (SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                        auto &surf = state.dataSurface->Surface(SurfNum);
                        if (!surf.HeatTransSurf) {
                            continue;
                        }
                        if (surf.ExtBoundCond > 0) {
                            continue; // Interior surfaces
                        }

                        if (surf.Class != SurfaceClass::Wall) {
                            continue;
                        }
                        if (state.dataConstruction->Construct(surf.Construction).TypeIsWindow) {
                            continue;
                        }
                        SurfacesOfType = true;
                        surf.HeatTransferAlgorithm = tmpAlgoInput;
                    }

                } else if (SELECT_CASE_var == "ALLEXTERIORROOFS") {
                    SurfacesOfType = false;
                    for (SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                        auto &surf = state.dataSurface->Surface(SurfNum);
                        if (!surf.HeatTransSurf) {
                            continue;
                        }
                        if (surf.ExtBoundCond > 0) {
                            continue; // Interior surfaces
                        }
                        if (surf.Class != SurfaceClass::Roof) {
                            continue;
                        }
                        if (state.dataConstruction->Construct(surf.Construction).TypeIsWindow) {
                            continue;
                        }
                        SurfacesOfType = true;
                        surf.HeatTransferAlgorithm = tmpAlgoInput;
                    }

                } else if (SELECT_CASE_var == "ALLEXTERIORFLOORS") {
                    SurfacesOfType = false;
                    for (SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                        auto &surf = state.dataSurface->Surface(SurfNum);
                        if (!surf.HeatTransSurf) {
                            continue;
                        }
                        if (surf.ExtBoundCond > 0) {
                            continue; // Interior surfaces
                        }
                        if (surf.Class != SurfaceClass::Floor) {
                            continue;
                        }
                        if (state.dataConstruction->Construct(surf.Construction).TypeIsWindow) {
                            continue;
                        }
                        SurfacesOfType = true;
                        surf.HeatTransferAlgorithm = tmpAlgoInput;
                    }

                } else if (SELECT_CASE_var == "ALLGROUNDCONTACTSURFACES") {
                    SurfacesOfType = false;
                    for (SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                        auto &surf = state.dataSurface->Surface(SurfNum);
                        if (!surf.HeatTransSurf) {
                            continue;
                        }
                        if (surf.ExtBoundCond != DataSurfaces::Ground) {
                            continue; // ground BC
                        }
                        if (state.dataConstruction->Construct(surf.Construction).TypeIsWindow) {
                            continue;
                        }
                        SurfacesOfType = true;
                        surf.HeatTransferAlgorithm = tmpAlgoInput;
                    }
                } else if (SELECT_CASE_var == "ALLINTERIORSURFACES") {
                    SurfacesOfType = false;
                    for (SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                        auto &surf = state.dataSurface->Surface(SurfNum);
                        if (!surf.HeatTransSurf) {
                            continue;
                        }
                        if (surf.ExtBoundCond <= 0) {
                            continue; // Exterior surfaces
                        }
                        if (state.dataConstruction->Construct(surf.Construction).TypeIsWindow) {
                            continue;
                        }
                        SurfacesOfType = true;
                        surf.HeatTransferAlgorithm = tmpAlgoInput;
                    }

                } else if (SELECT_CASE_var == "ALLINTERIORWALLS") {
                    SurfacesOfType = false;
                    for (SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                        auto &surf = state.dataSurface->Surface(SurfNum);
                        if (!surf.HeatTransSurf) {
                            continue;
                        }
                        if (surf.ExtBoundCond <= 0) {
                            continue; // Exterior surfaces
                        }
                        if (surf.Class != SurfaceClass::Wall) {
                            continue;
                        }
                        if (state.dataConstruction->Construct(surf.Construction).TypeIsWindow) {
                            continue;
                        }
                        SurfacesOfType = true;
                        surf.HeatTransferAlgorithm = tmpAlgoInput;
                    }

                } else if ((SELECT_CASE_var == "ALLINTERIORROOFS") || (SELECT_CASE_var == "ALLINTERIORCEILINGS")) {
                    SurfacesOfType = false;
                    for (SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                        auto &surf = state.dataSurface->Surface(SurfNum);
                        if (!surf.HeatTransSurf) {
                            continue;
                        }
                        if (surf.ExtBoundCond <= 0) {
                            continue; // Exterior surfaces
                        }
                        if (surf.Class != SurfaceClass::Roof) {
                            continue;
                        }
                        if (state.dataConstruction->Construct(surf.Construction).TypeIsWindow) {
                            continue;
                        }
                        SurfacesOfType = true;
                        surf.HeatTransferAlgorithm = tmpAlgoInput;
                    }

                } else if (SELECT_CASE_var == "ALLINTERIORFLOORS") {
                    SurfacesOfType = false;
                    for (SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                        auto &surf = state.dataSurface->Surface(SurfNum);
                        if (!surf.HeatTransSurf) {
                            continue;
                        }
                        if (surf.ExtBoundCond <= 0) {
                            continue; // Exterior surfaces
                        }
                        if (surf.Class != SurfaceClass::Floor) {
                            continue;
                        }
                        if (state.dataConstruction->Construct(surf.Construction).TypeIsWindow) {
                            continue;
                        }
                        SurfacesOfType = true;
                        surf.HeatTransferAlgorithm = tmpAlgoInput;
                    }
                } else {
                    SurfacesOfType = false;
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}",
                                           s_ipsc->cCurrentModuleObject,
                                           s_ipsc->cAlphaArgs(1),
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ErrorsFoundMultiSurf = true;
                }
            }

            if (!SurfacesOfType) {
                ShowWarningError(
                    state,
                    format("In {}=\"{}\", for Multiple Surface Assignment=\"{}\", there were no surfaces of that type found for assignment.",
                           s_ipsc->cCurrentModuleObject,
                           s_ipsc->cAlphaArgs(1),
                           s_ipsc->cAlphaArgs(2)));
            }
            if (ErrorsFoundMultiSurf) {
                ErrorsFound = true;
            }

        } // multi surface heat transfer algo override

        s_ipsc->cCurrentModuleObject = "SurfaceProperty:HeatTransferAlgorithm:SurfaceList";
        CountHTAlgoObjectsSurfList = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        for (Item = 1; Item <= CountHTAlgoObjectsSurfList; ++Item) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Item,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);
            bool ErrorsFoundSurfList = false;
            {
                std::string const &SELECT_CASE_var = s_ipsc->cAlphaArgs(2);

                if (SELECT_CASE_var == "CONDUCTIONTRANSFERFUNCTION") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::CTF;
                    state.dataHeatBal->AnyCTF = true;
                } else if (SELECT_CASE_var == "MOISTUREPENETRATIONDEPTHCONDUCTIONTRANSFERFUNCTION") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::EMPD;
                    state.dataHeatBal->AnyEMPD = true;
                } else if (SELECT_CASE_var == "COMBINEDHEATANDMOISTUREFINITEELEMENT") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::HAMT;
                    state.dataHeatBal->AnyHAMT = true;
                } else if (SELECT_CASE_var == "CONDUCTIONFINITEDIFFERENCE") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::CondFD;
                    state.dataHeatBal->AnyCondFD = true;
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}",
                                           s_ipsc->cCurrentModuleObject,
                                           s_ipsc->cAlphaArgs(1),
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ErrorsFoundSurfList = true;
                }
            }

            for (Item1 = 3; Item1 <= NumAlphas; ++Item1) {

                Found = Util::FindItemInList(s_ipsc->cAlphaArgs(Item1), state.dataSurface->Surface, state.dataSurface->TotSurfaces);

                if (Found == 0) {
                    ShowSevereError(state, format("{}=\"{}\", did not find matching surface.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("Name of surface not found = \"{}\"", s_ipsc->cAlphaArgs(Item1)));
                    ErrorsFoundSurfList = true;
                }

                if (!ErrorsFoundSurfList) {
                    state.dataSurface->Surface(Found).HeatTransferAlgorithm = tmpAlgoInput;
                } else {
                    ErrorsFound = true;
                }
            }
        }

        s_ipsc->cCurrentModuleObject = "SurfaceProperty:HeatTransferAlgorithm:Construction";
        CountHTAlgoObjectsSurfList = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        for (Item = 1; Item <= CountHTAlgoObjectsSurfList; ++Item) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Item,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);
            bool ErrorsFoundByConstruct = false;
            {
                std::string const &SELECT_CASE_var = s_ipsc->cAlphaArgs(2);

                if (SELECT_CASE_var == "CONDUCTIONTRANSFERFUNCTION") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::CTF;
                    state.dataHeatBal->AnyCTF = true;
                } else if (SELECT_CASE_var == "MOISTUREPENETRATIONDEPTHCONDUCTIONTRANSFERFUNCTION") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::EMPD;
                    state.dataHeatBal->AnyEMPD = true;
                } else if (SELECT_CASE_var == "COMBINEDHEATANDMOISTUREFINITEELEMENT") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::HAMT;
                    state.dataHeatBal->AnyHAMT = true;
                } else if (SELECT_CASE_var == "CONDUCTIONFINITEDIFFERENCE") {
                    tmpAlgoInput = DataSurfaces::HeatTransferModel::CondFD;
                    state.dataHeatBal->AnyCondFD = true;
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}",
                                           s_ipsc->cCurrentModuleObject,
                                           s_ipsc->cAlphaArgs(1),
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ErrorsFoundByConstruct = true;
                }
            }

            Found = Util::FindItemInList(s_ipsc->cAlphaArgs(3), state.dataConstruction->Construct, state.dataHeatBal->TotConstructs);
            if (Found == 0) {
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {}=\"{}",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cAlphaFieldNames(3),
                                       s_ipsc->cAlphaArgs(3)));
                ErrorsFoundByConstruct = true;
            }

            if (!ErrorsFoundByConstruct) {
                for (Item1 = 1; Item1 <= state.dataSurface->TotSurfaces; ++Item1) {
                    if (state.dataSurface->Surface(Item1).Construction == Found) {
                        state.dataSurface->Surface(Item1).HeatTransferAlgorithm = tmpAlgoInput;
                    }
                }
            }
        }

        // Change algorithm for Kiva and air boundary foundation surfaces
        for (auto &surf : state.dataSurface->Surface) {
            if (surf.ExtBoundCond == DataSurfaces::KivaFoundation) {
                surf.HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::Kiva;
                state.dataHeatBal->AnyKiva = true;
            }
        }

        // test for missing materials for algorithms selected
        NumEMPDMat = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "MaterialProperty:MoisturePenetrationDepth:Settings");
        NumPCMat = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "MaterialProperty:PhaseChange") +
                   state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "MaterialProperty:PhaseChangeHysteresis");
        NumVTCMat = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "MaterialProperty:VariableThermalConductivity");
        NumHAMTMat1 = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "MaterialProperty:HeatAndMoistureTransfer:Settings");
        NumHAMTMat2 =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "MaterialProperty:HeatAndMoistureTransfer:SorptionIsotherm");
        NumHAMTMat3 = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "MaterialProperty:HeatAndMoistureTransfer:Suction");
        NumHAMTMat4 = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "MaterialProperty:HeatAndMoistureTransfer:Redistribution");
        NumHAMTMat5 = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "MaterialProperty:HeatAndMoistureTransfer:Diffusion");
        NumHAMTMat6 =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "MaterialProperty:HeatAndMoistureTransfer:ThermalConductivity");
        SumHAMTMat = NumHAMTMat1 + NumHAMTMat2 + NumHAMTMat3 + NumHAMTMat4 + NumHAMTMat5 + NumHAMTMat6;
        msgneeded = false;

        if (NumEMPDMat > 0 && !state.dataHeatBal->AnyEMPD) {
            ShowWarningError(state,
                             format("The input file includes {} MaterialProperty:MoisturePenetrationDepth:Settings objects but the moisture "
                                    "penetration depth algorithm is not used anywhere.",
                                    NumEMPDMat));
            msgneeded = true;
        }
        if (NumPCMat > 0 && !state.dataHeatBal->AnyCondFD) {
            ShowWarningError(state,
                             format("The input file includes {} MaterialProperty:PhaseChange objects but the conduction finite difference algorithm "
                                    "is not used anywhere.",
                                    NumPCMat));
            msgneeded = true;
        }
        if (NumVTCMat > 0 && !state.dataHeatBal->AnyCondFD) {
            ShowWarningError(state,
                             format("The input file includes {} MaterialProperty:VariableThermalConductivity objects but the conduction finite "
                                    "difference algorithm is not used anywhere.",
                                    NumVTCMat));
            msgneeded = true;
        }
        if (SumHAMTMat > 0 && !state.dataHeatBal->AnyHAMT) {
            ShowWarningError(state,
                             format("The input file includes {} MaterialProperty:HeatAndMoistureTransfer:* objects but the combined heat and "
                                    "moisture finite difference algorithm is not used anywhere.",
                                    SumHAMTMat));
            msgneeded = true;
        }
        if (msgneeded) {
            ShowContinueError(state, "Previous materials will be ignored due to HeatBalanceAlgorithm choice.");
        }
        msgneeded = false;
        if (NumEMPDMat == 0 && state.dataHeatBal->AnyEMPD) {
            ShowWarningError(state,
                             "The moisture penetration depth conduction transfer function algorithm is used but the input file includes no "
                             "MaterialProperty:MoisturePenetrationDepth:Settings objects.");
            msgneeded = true;
        }
        if (SumHAMTMat == 0 && state.dataHeatBal->AnyHAMT) {
            ShowWarningError(state,
                             "The combined heat and moisture finite element algorithm is used but the input file includes no "
                             "MaterialProperty:HeatAndMoistureTransfer:* objects.");
            msgneeded = true;
        }
        if (msgneeded) {
            ShowContinueError(state,
                              "Certain materials objects are necessary to achieve proper results with the heat transfer algorithm(s) selected.");
        }

        // Write Solution Algorithm to the initialization output file for User Verification
        print(state.files.eio,
              "{}\n",
              "! <Surface Heat Transfer Algorithm>, Value {CTF - ConductionTransferFunction | EMPD - "
              "MoisturePenetrationDepthConductionTransferFunction | CondFD - ConductionFiniteDifference | HAMT - "
              "CombinedHeatAndMoistureFiniteElement} - Description,Inside Surface Max Temperature Limit{C}, Surface "
              "Convection Coefficient Lower Limit {W/m2-K}, Surface Convection Coefficient Upper Limit {W/m2-K}");

        int numberOfHeatTransferAlgosUsed = 0;
        // Formats
        static constexpr std::string_view Format_725("Surface Heat Transfer Algorithm, {},{:.0R},{:.2R},{:.1R}\n");

        if (state.dataHeatBal->AnyCTF) {
            constexpr std::string_view AlgoName = "CTF - ConductionTransferFunction";
            ++numberOfHeatTransferAlgosUsed;
            print(state.files.eio,
                  Format_725,
                  AlgoName,
                  state.dataHeatBalSurf->MaxSurfaceTempLimit,
                  state.dataHeatBal->LowHConvLimit,
                  state.dataHeatBal->HighHConvLimit);
        }
        if (state.dataHeatBal->AnyEMPD) {
            state.dataHeatBal->AllCTF = false;
            constexpr std::string_view AlgoName = "EMPD - MoisturePenetrationDepthConductionTransferFunction";
            ++numberOfHeatTransferAlgosUsed;
            print(state.files.eio,
                  Format_725,
                  AlgoName,
                  state.dataHeatBalSurf->MaxSurfaceTempLimit,
                  state.dataHeatBal->LowHConvLimit,
                  state.dataHeatBal->HighHConvLimit);
            if (state.dataHeatBal->doSpaceHeatBalanceSimulation || state.dataHeatBal->doSpaceHeatBalanceSizing) {
                ShowSevereError(
                    state,
                    "MoisturePenetrationDepthConductionTransferFunction is not supported with ZoneAirHeatBalanceAlgorithm Space Heat Balance.");
                ErrorsFound = true;
            }
        }
        if (state.dataHeatBal->AnyCondFD) {
            state.dataHeatBal->AllCTF = false;
            constexpr std::string_view AlgoName = "CondFD - ConductionFiniteDifference";
            ++numberOfHeatTransferAlgosUsed;
            print(state.files.eio,
                  Format_725,
                  AlgoName,
                  state.dataHeatBalSurf->MaxSurfaceTempLimit,
                  state.dataHeatBal->LowHConvLimit,
                  state.dataHeatBal->HighHConvLimit);
        }
        if (state.dataHeatBal->AnyHAMT) {
            state.dataHeatBal->AllCTF = false;
            constexpr std::string_view AlgoName = "HAMT - CombinedHeatAndMoistureFiniteElement";
            ++numberOfHeatTransferAlgosUsed;
            print(state.files.eio,
                  Format_725,
                  AlgoName,
                  state.dataHeatBalSurf->MaxSurfaceTempLimit,
                  state.dataHeatBal->LowHConvLimit,
                  state.dataHeatBal->HighHConvLimit);
            if (state.dataHeatBal->doSpaceHeatBalanceSimulation || state.dataHeatBal->doSpaceHeatBalanceSizing) {
                ShowSevereError(state, "CombinedHeatAndMoistureFiniteElement is not supported with ZoneAirHeatBalanceAlgorithm Space Heat Balance.");
                ErrorsFound = true;
            }
        }
        if (state.dataHeatBal->AnyKiva) {
            state.dataHeatBal->AllCTF = false;
            constexpr std::string_view AlgoName = "KivaFoundation - TwoDimensionalFiniteDifference";
            ++numberOfHeatTransferAlgosUsed;
            print(state.files.eio,
                  Format_725,
                  AlgoName,
                  state.dataHeatBalSurf->MaxSurfaceTempLimit,
                  state.dataHeatBal->LowHConvLimit,
                  state.dataHeatBal->HighHConvLimit);
        }

        // Check HeatTransferAlgorithm for interior surfaces
        if (numberOfHeatTransferAlgosUsed > 1) {
            int ExtSurfNum;
            for (Item = 1; Item <= state.dataSurface->TotSurfaces; ++Item) {
                auto &surf = state.dataSurface->Surface(Item);
                if (surf.ExtBoundCond > 0) {
                    if ((surf.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::Invalid) ||
                        (surf.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::None)) {
                        continue;
                    }
                    ExtSurfNum = surf.ExtBoundCond;
                    auto &extSurf = state.dataSurface->Surface(ExtSurfNum);
                    if (surf.HeatTransferAlgorithm != extSurf.HeatTransferAlgorithm) {
                        ShowWarningError(state,
                                         "An interior surface is defined as two surfaces with reverse constructions. The HeatTransferAlgorithm in "
                                         "both constructions should be same.");
                        ShowContinueError(state,
                                          format("The HeatTransferAlgorithm of Surface: {}, is {}",
                                                 surf.Name,
                                                 DataSurfaces::HeatTransAlgoStrs[static_cast<int>(surf.HeatTransferAlgorithm)]));
                        ShowContinueError(state,
                                          format("The HeatTransferAlgorithm of Surface: {}, is {}",
                                                 extSurf.Name,
                                                 DataSurfaces::HeatTransAlgoStrs[static_cast<int>(extSurf.HeatTransferAlgorithm)]));
                        if (surf.HeatTransferAlgorithm > extSurf.HeatTransferAlgorithm) {
                            ShowContinueError(state,
                                              format("The HeatTransferAlgorithm of Surface: {}, is assigned to {}. Simulation continues.",
                                                     extSurf.Name,
                                                     DataSurfaces::HeatTransAlgoStrs[static_cast<int>(surf.HeatTransferAlgorithm)]));
                            extSurf.HeatTransferAlgorithm = surf.HeatTransferAlgorithm;
                        } else {
                            ShowContinueError(state,
                                              format("The HeatTransferAlgorithm of Surface: {}, is assigned to {}. Simulation continues.",
                                                     surf.Name,
                                                     DataSurfaces::HeatTransAlgoStrs[static_cast<int>(extSurf.HeatTransferAlgorithm)]));
                            surf.HeatTransferAlgorithm = extSurf.HeatTransferAlgorithm;
                        }
                    }
                }
            }
        }

        // Assign model type to windows, shading surfaces, and TDDs
        for (Item = 1; Item <= state.dataSurface->TotSurfaces; ++Item) {
            if (state.dataSurface->Surface(Item).Class == SurfaceClass::Window || state.dataSurface->Surface(Item).Class == SurfaceClass::GlassDoor) {
                // todo, add complex fenestration switch  HeatTransferModel_ComplexFenestration
                if (state.dataSurface->SurfWinWindowModelType(Item) == DataSurfaces::WindowModel::BSDF) {
                    state.dataSurface->Surface(Item).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::ComplexFenestration;
                } else {
                    state.dataSurface->Surface(Item).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::Window5;
                }
            }
            if (state.dataSurface->Surface(Item).Class == SurfaceClass::Detached_B ||
                state.dataSurface->Surface(Item).Class == SurfaceClass::Detached_F ||
                state.dataSurface->Surface(Item).Class == SurfaceClass::Shading || state.dataSurface->Surface(Item).Class == SurfaceClass::Overhang ||
                state.dataSurface->Surface(Item).Class == SurfaceClass::Fin) {
                state.dataSurface->Surface(Item).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::None;
            }
            if (state.dataSurface->Surface(Item).Class == SurfaceClass::TDD_Diffuser ||
                state.dataSurface->Surface(Item).Class == SurfaceClass::TDD_Dome) {
                state.dataSurface->Surface(Item).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::TDD;
            }

            if (state.dataSurface->Surface(Item).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CTF ||
                state.dataSurface->Surface(Item).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD) {
                state.dataConstruction->Construct(state.dataSurface->Surface(Item).Construction).IsUsedCTF = true;
            }
        }
    }

    struct PopCoincidentVertexReturn
    {
        double perimeter;
        int poppedVertexPos = -1; // This is a STL vector position, 0-indexed
        int keptVertexPos = -1;
    };

    PopCoincidentVertexReturn checkPopCoincidentVertex(const Array1D<Vector> &vertices)
    {

        size_t const nSides = vertices.size();

        // Pass one: Vector of distance from this vertex to the next one
        std::vector<Real64> distances(nSides);
        size_t index = 0;
        double min_distance = std::numeric_limits<Real64>::max();
        double perimeter = 0.0;
        for (auto it = vertices.begin(); it != vertices.end(); ++it) {
            auto itnext = std::next(it);
            if (itnext == std::end(vertices)) {
                itnext = std::begin(vertices);
            }
            const auto dist = distance(*it, *itnext);
            distances[index++] = dist;
            min_distance = std::min(min_distance, dist);
            perimeter += dist;
        }
        // Return early if nothing to be popped
        if (min_distance >= Constant::OneCentimeter) {
            return {perimeter};
        }

        // Pass two: figure out the vertex that is coincident with its previous and/or next vertex and
        // that minimizes the (distanceThisToNext + distanceThisToPrev).
        Real64 min_weight = std::numeric_limits<Real64>::max();
        int poppedVertexPos = -1;
        int keptVertexPos = -1;

        for (index = 0; index < nSides; ++index) {
            size_t const prevIndex = (index == 0) ? nSides - 1 : index - 1;
            Real64 const &distanceThisToNext = distances[index];
            Real64 const &distanceThisToPrev = distances[prevIndex];
            if ((distanceThisToNext >= Constant::OneCentimeter) && (distanceThisToPrev >= Constant::OneCentimeter)) {
                continue;
            }
            Real64 const weight = distanceThisToNext + distanceThisToPrev;
            if (weight < min_weight) {
                min_weight = weight;
                poppedVertexPos = static_cast<int>(index);
                if (distanceThisToPrev < distanceThisToNext) {
                    keptVertexPos = prevIndex;
                } else {
                    keptVertexPos = static_cast<int>((index == nSides - 1) ? 0 : index + 1);
                }
            }
        }

        // Return the keptVertexPos (which can be the previous or the next), so we can print the displayExtraWarning correctly
        return {perimeter, poppedVertexPos, keptVertexPos};
    }

    void GetVertices(EnergyPlusData &state,
                     int const SurfNum,             // Current surface number
                     int const NSides,              // Number of sides to figure
                     Array1S<Real64> const Vertices // Vertices, in specified order
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the surface vertices from the arrays
        // passed by the calling routine.  These had previously been obtained
        // from the InputProcessor (GetObjectItem).  This routine will provide
        // a standard place for determining various properties of the surface
        // from the vertices.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetVertices: ");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int n;    // Loop counter
        int NSrc; // Used for CW -> CCW transformation
        int NTar; // Used for CW -> CCW transformation
        Real64 SurfWorldAz;
        Real64 SurfTilt;
        Real64 Perimeter; // Perimeter length of the surface
        Real64 Xb;        // Intermediate calculation
        Real64 Yb;        // Intermediate calculation
        int ZoneNum;
        int ThisCorner;
        Real64 ThisWidth;
        Real64 ThisHeight;
        // unused    REAL(r64) :: ccwtest
        // unused    LOGICAL   :: SurfaceCCW
        Real64 dotp;

        // Object Data
        Vector const TestVector(0.0, 0.0, 1.0);
        Vector temp;

        auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);

        if (NSides > state.dataSurface->MaxVerticesPerSurface) {
            state.dataSurface->MaxVerticesPerSurface = NSides;
        }
        int Ptr = 1;
        for (n = 1; n <= NSides; ++n) {
            surfTemp.Vertex(n).x = Vertices(Ptr);
            ++Ptr;
            surfTemp.Vertex(n).y = Vertices(Ptr);
            ++Ptr;
            surfTemp.Vertex(n).z = Vertices(Ptr);
            ++Ptr;
        }

        // Address changing vertices if they were put in in CW order rather than CCW
        if (!state.dataSurface->CCW) {
            // If even number of sides, this will transfer appropriately
            // If odd number, will leave the "odd" one, which is what you want.
            NSrc = NSides;
            NTar = 2;
            for (n = 1; n <= (NSides - 1) / 2; ++n) {
                temp = surfTemp.Vertex(NSrc);
                surfTemp.Vertex(NSrc) = surfTemp.Vertex(NTar);
                surfTemp.Vertex(NTar) = temp;
                --NSrc;
                ++NTar;
            }
        }
        // Now address which "Corner" has been put in first.  Note: the azimuth and tilt and area
        // calculations do not care which corner is put in first.
        // 2/2011 - don't think the shading calculations have a corner preference.  Will keep this for
        // consistency (for now)
        ThisCorner = state.dataSurface->Corner;
        while (ThisCorner != DataSurfaces::UpperLeftCorner) {
            if (NSides < 4) {
                if (ThisCorner == DataSurfaces::UpperRightCorner) {
                    break;
                }
            }
            NTar = ThisCorner;
            NSrc = ThisCorner + 1;
            if (NSrc > NSides) {
                NSrc = 1;
            }
            for (n = 1; n <= NSides - 1; ++n) {
                temp = surfTemp.Vertex(NTar);
                surfTemp.Vertex(NTar) = surfTemp.Vertex(NSrc);
                surfTemp.Vertex(NSrc) = temp;
                ++NTar;
                ++NSrc;
                if (NTar > NSides) {
                    NTar = 1;
                }
                if (NSrc > NSides) {
                    NSrc = 1;
                }
            }
            ++ThisCorner;
            if (ThisCorner > NSides) {
                ThisCorner = 1;
            }
        } // Corners
        if (!state.dataSurface->WorldCoordSystem) {
            // Input in "relative" coordinates, use Building and Zone North Axes and Origins
            //                                  to translate each point (including rotation for Appendix G)
            ZoneNum = surfTemp.Zone;
            if (ZoneNum > 0) {
                for (n = 1; n <= NSides; ++n) {
                    Xb = surfTemp.Vertex(n).x * state.dataSurfaceGeometry->CosZoneRelNorth(ZoneNum) -
                         surfTemp.Vertex(n).y * state.dataSurfaceGeometry->SinZoneRelNorth(ZoneNum) + state.dataHeatBal->Zone(ZoneNum).OriginX;
                    Yb = surfTemp.Vertex(n).x * state.dataSurfaceGeometry->SinZoneRelNorth(ZoneNum) +
                         surfTemp.Vertex(n).y * state.dataSurfaceGeometry->CosZoneRelNorth(ZoneNum) + state.dataHeatBal->Zone(ZoneNum).OriginY;
                    surfTemp.Vertex(n).x = Xb * state.dataSurfaceGeometry->CosBldgRelNorth - Yb * state.dataSurfaceGeometry->SinBldgRelNorth;
                    surfTemp.Vertex(n).y = Xb * state.dataSurfaceGeometry->SinBldgRelNorth + Yb * state.dataSurfaceGeometry->CosBldgRelNorth;
                    surfTemp.Vertex(n).z += state.dataHeatBal->Zone(ZoneNum).OriginZ;
                }
            } else if (surfTemp.Class == SurfaceClass::Detached_B) {
                for (n = 1; n <= NSides; ++n) {
                    Xb = surfTemp.Vertex(n).x;
                    Yb = surfTemp.Vertex(n).y;
                    surfTemp.Vertex(n).x = Xb * state.dataSurfaceGeometry->CosBldgRelNorth - Yb * state.dataSurfaceGeometry->SinBldgRelNorth;
                    surfTemp.Vertex(n).y = Xb * state.dataSurfaceGeometry->SinBldgRelNorth + Yb * state.dataSurfaceGeometry->CosBldgRelNorth;
                }
            }
        } else {
            // if world coordinate only need to rotate for Appendix G
            ZoneNum = surfTemp.Zone;
            if (ZoneNum > 0) {
                for (n = 1; n <= NSides; ++n) {
                    Xb = surfTemp.Vertex(n).x;
                    Yb = surfTemp.Vertex(n).y;
                    surfTemp.Vertex(n).x = Xb * state.dataSurfaceGeometry->CosBldgRotAppGonly - Yb * state.dataSurfaceGeometry->SinBldgRotAppGonly;
                    surfTemp.Vertex(n).y = Xb * state.dataSurfaceGeometry->SinBldgRotAppGonly + Yb * state.dataSurfaceGeometry->CosBldgRotAppGonly;
                }
            } else if (surfTemp.Class == SurfaceClass::Detached_B) {
                for (n = 1; n <= NSides; ++n) {
                    Xb = surfTemp.Vertex(n).x;
                    Yb = surfTemp.Vertex(n).y;
                    surfTemp.Vertex(n).x = Xb * state.dataSurfaceGeometry->CosBldgRotAppGonly - Yb * state.dataSurfaceGeometry->SinBldgRotAppGonly;
                    surfTemp.Vertex(n).y = Xb * state.dataSurfaceGeometry->SinBldgRotAppGonly + Yb * state.dataSurfaceGeometry->CosBldgRotAppGonly;
                }
            }
        }

        if (NSides > 2) {
            auto &surface = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
            auto &vertices = surface.Vertex;
            auto &nSides = surface.Sides;
            std::string TiltString;

            while (true) {
                PopCoincidentVertexReturn const popResult = checkPopCoincidentVertex(vertices);
                Perimeter = popResult.perimeter;
                if (popResult.poppedVertexPos < 0) {
                    // No pop needed, we're done
                    break;
                }

                // Grab the popped one, and the kept one (regardless of whether it's previous or next)
                auto it = vertices.begin();
                std::advance(it, popResult.poppedVertexPos);
                int const poppedVertexIndex = popResult.poppedVertexPos + 1;

                auto itKept = vertices.begin();
                std::advance(itKept, popResult.keptVertexPos);
                int const keptVertexIndex = popResult.keptVertexPos + 1;

                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowWarningError(state,
                                     format("{}Distance between two vertices < .01, possibly coincident. for Surface={}, in Zone={}",
                                            RoutineName,
                                            surfTemp.Name,
                                            surfTemp.ZoneName));

                    bool const printPoppedFirst = (poppedVertexIndex < keptVertexIndex) ? !(poppedVertexIndex == 1 && keptVertexIndex == nSides)
                                                                                        : (poppedVertexIndex == nSides && keptVertexIndex == 1);

                    if (printPoppedFirst) {
                        ShowContinueError(state, format("Vertex [{}]=({:.2R},{:.2R},{:.2R})", poppedVertexIndex, it->x, it->y, it->z));
                        ShowContinueError(state, format("Vertex [{}]=({:.2R},{:.2R},{:.2R})", keptVertexIndex, itKept->x, itKept->y, itKept->z));
                    } else {
                        ShowContinueError(state, format("Vertex [{}]=({:.2R},{:.2R},{:.2R})", keptVertexIndex, itKept->x, itKept->y, itKept->z));
                        ShowContinueError(state, format("Vertex [{}]=({:.2R},{:.2R},{:.2R})", poppedVertexIndex, it->x, it->y, it->z));
                    }
                }
                ++state.dataErrTracking->TotalCoincidentVertices;
                if (nSides <= 3) {
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        ShowContinueError(state,
                                          format("Cannot Drop Vertex [{}]; Number of Surface Sides at minimum. This surface is now a "
                                                 "degenerate surface.",
                                                 poppedVertexIndex));
                    }
                    ++state.dataErrTracking->TotalDegenerateSurfaces;
                    // If degenerate, we won't be able to pop now nor later, so exit
                    // mark degenerate surface?
                    break;
                }

                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowContinueError(state, format("Dropping Vertex [{}].", poppedVertexIndex));
                }
                --nSides;
                vertices.erase(it);
                // No need to recompute perimeter, because it'll be done in the next iteration, until no popping or degenerate happens
            }

            surfTemp.Perimeter = Perimeter;

            Vectors::CreateNewellSurfaceNormalVector(surfTemp.Vertex, surfTemp.Sides, surfTemp.NewellSurfaceNormalVector);
            Vectors::CreateNewellAreaVector(surfTemp.Vertex, surfTemp.Sides, surfTemp.NewellAreaVector);
            // For surfaces with subsurfaces, the following two areas are turned into net areas later by
            // subtracting subsurface areas
            surfTemp.GrossArea = Vectors::VecLength(surfTemp.NewellAreaVector);
            surfTemp.Area = surfTemp.GrossArea;
            surfTemp.NetAreaShadowCalc = surfTemp.Area;
            Vectors::DetermineAzimuthAndTilt(
                surfTemp.Vertex, SurfWorldAz, SurfTilt, surfTemp.lcsx, surfTemp.lcsy, surfTemp.lcsz, surfTemp.NewellSurfaceNormalVector);
            dotp = dot(surfTemp.NewellSurfaceNormalVector, TestVector);
            if (surfTemp.Class == SurfaceClass::Roof && dotp < -0.000001) {
                TiltString = format("{:.1R}", SurfTilt);
                ShowWarningError(state,
                                 format("{}Roof/Ceiling is upside down! Tilt angle=[{}], should be near 0, Surface=\"{}\", in Zone=\"{}\".",
                                        RoutineName,
                                        TiltString,
                                        surfTemp.Name,
                                        surfTemp.ZoneName));
                ShowContinueError(state, "Automatic fix is attempted.");
                ReverseAndRecalculate(state, SurfNum, surfTemp.Sides, SurfWorldAz, SurfTilt);
            } else if (surfTemp.Class == SurfaceClass::Roof && SurfTilt > 80.0) {
                TiltString = format("{:.1R}", SurfTilt);
                ShowWarningError(
                    state,
                    format("{}Roof/Ceiling is not oriented correctly! Tilt angle=[{}], should be near 0, Surface=\"{}\", in Zone=\"{}\".",
                           RoutineName,
                           TiltString,
                           surfTemp.Name,
                           surfTemp.ZoneName));
            }
            if (surfTemp.Class == SurfaceClass::Floor && dotp > 0.000001) {
                TiltString = format("{:.1R}", SurfTilt);
                ShowWarningError(state,
                                 format("{}Floor is upside down! Tilt angle=[{}], should be near 180, Surface=\"{}\", in Zone=\"{}\".",
                                        RoutineName,
                                        TiltString,
                                        surfTemp.Name,
                                        surfTemp.ZoneName));
                ShowContinueError(state, "Automatic fix is attempted.");
                ReverseAndRecalculate(state, SurfNum, surfTemp.Sides, SurfWorldAz, SurfTilt);
            } else if (surfTemp.Class == SurfaceClass::Floor && SurfTilt < 158.2) { // slope/grade = 40%!
                TiltString = format("{:.1R}", SurfTilt);
                ShowWarningError(state,
                                 format("{}Floor is not oriented correctly! Tilt angle=[{}], should be near 180, Surface=\"{}\", in Zone=\"{}\".",
                                        RoutineName,
                                        TiltString,
                                        surfTemp.Name,
                                        surfTemp.ZoneName));
            }
            surfTemp.Azimuth = SurfWorldAz;
            surfTemp.Tilt = SurfTilt;
            surfTemp.convOrientation = Convect::GetSurfConvOrientation(surfTemp.Tilt);

            // Sine and cosine of azimuth and tilt
            surfTemp.SinAzim = std::sin(SurfWorldAz * Constant::DegToRad);
            surfTemp.CosAzim = std::cos(SurfWorldAz * Constant::DegToRad);
            surfTemp.SinTilt = std::sin(SurfTilt * Constant::DegToRad);
            surfTemp.CosTilt = std::cos(SurfTilt * Constant::DegToRad);
            if (surfTemp.ViewFactorGround == Constant::AutoCalculate) {
                surfTemp.ViewFactorGround = 0.5 * (1.0 - surfTemp.CosTilt);
            }
            // Outward normal unit vector (pointing away from room)
            surfTemp.OutNormVec = surfTemp.NewellSurfaceNormalVector;
            for (n = 1; n <= 3; ++n) {
                if (std::abs(surfTemp.OutNormVec(n) - 1.0) < 1.e-06) {
                    surfTemp.OutNormVec(n) = +1.0;
                }
                if (std::abs(surfTemp.OutNormVec(n) + 1.0) < 1.e-06) {
                    surfTemp.OutNormVec(n) = -1.0;
                }
                if (std::abs(surfTemp.OutNormVec(n)) < 1.e-06) {
                    surfTemp.OutNormVec(n) = 0.0;
                }
            }

            if (surfTemp.Class == SurfaceClass::Window || surfTemp.Class == SurfaceClass::GlassDoor || surfTemp.Class == SurfaceClass::Door) {
                surfTemp.Area *= surfTemp.Multiplier;
            }
            // Can perform tests on this surface here
            surfTemp.ViewFactorSky = 0.5 * (1.0 + surfTemp.CosTilt);
            // The following IR view factors are modified in subr. SkyDifSolarShading if there are shadowing
            // surfaces
            surfTemp.ViewFactorSkyIR = surfTemp.ViewFactorSky;
            surfTemp.ViewFactorGroundIR = 0.5 * (1.0 - surfTemp.CosTilt);

            // Call to transform vertices

            TransformVertsByAspect(state, SurfNum, surfTemp.Sides);

        } else {
            ShowFatalError(state, format("{}Called with less than 2 sides, Surface={}", RoutineName, surfTemp.Name));
        }

        // Preliminary Height/Width
        temp = surfTemp.Vertex(3) - surfTemp.Vertex(2);
        ThisWidth = Vectors::VecLength(temp);
        temp = surfTemp.Vertex(2) - surfTemp.Vertex(1);
        ThisHeight = Vectors::VecLength(temp);
        surfTemp.Height = ThisHeight;
        surfTemp.Width = ThisWidth;
    }

    void ReverseAndRecalculate(EnergyPlusData &state,
                               int const SurfNum,   // Surface number for the surface
                               int const NSides,    // number of sides to surface
                               Real64 &SurfAzimuth, // Surface Facing angle (will be 0 for roofs/floors)
                               Real64 &SurfTilt     // Surface tilt (
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2011

        // PURPOSE OF THIS SUBROUTINE:
        // This routine reverses the vertices for a surface (needed when roof/floor is upside down)
        // and recalculates the azimuth, etc for the surface.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("ReverseAndRecalculate: ");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int n;      // Loop Control
        int RevPtr; // pointer for reversing vertices
        std::string TiltString;

        // Object Data
        Array1D<Vector> Vertices(NSides); // Vertices, in specified order

        auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);

        for (n = 1; n <= NSides; ++n) {
            Vertices(n) = surfTemp.Vertex(n);
        }
        RevPtr = NSides;
        for (n = 1; n <= NSides; ++n) {
            surfTemp.Vertex(n) = Vertices(RevPtr);
            --RevPtr;
        }

        print(state.files.debug, "Reversing Surface Name={}\n", surfTemp.Name);
        for (n = 1; n <= NSides; ++n) {
            print(state.files.debug,
                  "side={:5} abs coord vertex= {:18.13F} {:18.13F} {:18.13F}\n",
                  n,
                  surfTemp.Vertex(n).x,
                  surfTemp.Vertex(n).y,
                  surfTemp.Vertex(n).z);
        }

        Vectors::CreateNewellSurfaceNormalVector(surfTemp.Vertex, surfTemp.Sides, surfTemp.NewellSurfaceNormalVector);
        Vectors::DetermineAzimuthAndTilt(
            surfTemp.Vertex, SurfAzimuth, SurfTilt, surfTemp.lcsx, surfTemp.lcsy, surfTemp.lcsz, surfTemp.NewellSurfaceNormalVector);
        if (surfTemp.Class == SurfaceClass::Roof && SurfTilt > 80.0) {
            TiltString = format("{:.1R}", SurfTilt);
            ShowWarningError(
                state,
                format("{}Roof/Ceiling is still upside down! Tilt angle=[{}], should be near 0, please fix manually.", RoutineName, TiltString));
        }
        if (surfTemp.Class == SurfaceClass::Floor && SurfTilt < 158.2) { // 40% grade!
            ShowWarningError(
                state, format("{}Floor is still upside down! Tilt angle=[{}], should be near 180, please fix manually.", RoutineName, TiltString));
        }
    }

    void MakeMirrorSurface(EnergyPlusData &state, int &SurfNum) // In=>Surface to Mirror, Out=>new Surface index // This is not good
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   June 2002

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine creates a "mirror" surface using the indicated surface.
        // This is the simple approach for bi-directional shading devices.  If, perchance,
        // the user has already taken care of this (e.g. fins in middle of wall), there will
        // be extra shading devices shown.

        auto &origSurface = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
        auto &newSurface = state.dataSurfaceGeometry->SurfaceTmp(SurfNum + 1);
        newSurface = origSurface;

        auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);

        int nVert = origSurface.Sides;
        // Reverse the vertices in the original surface.  Add "MIR-" to name.
        for (int Vert = 1; Vert <= surfTemp.Sides; ++Vert) {
            newSurface.Vertex(Vert) = origSurface.Vertex(nVert);
            --nVert;
        }
        newSurface.Name = "Mir-" + origSurface.Name;
        newSurface.MirroredSurf = true;

        if (newSurface.Sides > 2) {
            Vectors::CreateNewellAreaVector(newSurface.Vertex, newSurface.Sides, newSurface.NewellAreaVector);
            newSurface.GrossArea = Vectors::VecLength(newSurface.NewellAreaVector);
            newSurface.Area = newSurface.GrossArea;
            newSurface.NetAreaShadowCalc = newSurface.Area;
            Vectors::CreateNewellSurfaceNormalVector(newSurface.Vertex, newSurface.Sides, newSurface.NewellSurfaceNormalVector);
            Real64 SurfWorldAz = 0.0;
            Real64 SurfTilt = 0.0;
            Vectors::DetermineAzimuthAndTilt(
                newSurface.Vertex, SurfWorldAz, SurfTilt, newSurface.lcsx, newSurface.lcsy, newSurface.lcsz, newSurface.NewellSurfaceNormalVector);
            newSurface.Azimuth = SurfWorldAz;
            newSurface.Tilt = SurfTilt;
            newSurface.convOrientation = Convect::GetSurfConvOrientation(newSurface.Tilt);

            // Sine and cosine of azimuth and tilt
            newSurface.SinAzim = std::sin(SurfWorldAz * Constant::DegToRad);
            newSurface.CosAzim = std::cos(SurfWorldAz * Constant::DegToRad);
            newSurface.SinTilt = std::sin(SurfTilt * Constant::DegToRad);
            newSurface.CosTilt = std::cos(SurfTilt * Constant::DegToRad);
            // Outward normal unit vector (pointing away from room)
            newSurface.OutNormVec = newSurface.NewellSurfaceNormalVector;
            for (int n = 1; n <= 3; ++n) {
                if (std::abs(newSurface.OutNormVec(n) - 1.0) < 1.e-06) {
                    newSurface.OutNormVec(n) = +1.0;
                }
                if (std::abs(newSurface.OutNormVec(n) + 1.0) < 1.e-06) {
                    newSurface.OutNormVec(n) = -1.0;
                }
                if (std::abs(newSurface.OutNormVec(n)) < 1.e-06) {
                    newSurface.OutNormVec(n) = 0.0;
                }
            }

            // Can perform tests on this surface here
            newSurface.ViewFactorSky = 0.5 * (1.0 + newSurface.CosTilt);
            // The following IR view factors are modified in subr. SkyDifSolarShading if there are shadowing surfaces
            newSurface.ViewFactorSkyIR = newSurface.ViewFactorSky;
            newSurface.ViewFactorGroundIR = 0.5 * (1.0 - newSurface.CosTilt);
            ++SurfNum; // Calling function expects incremented argument on return
        }
    }

    void GetWindowShadingControlData(EnergyPlusData &state, bool &ErrorsFound) // If errors found in input
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   November 1998
        //       MODIFIED       Aug 2001 (FW): add handling of new ShadingControlIsScheduled
        //                      and GlareControlIsActive fields
        //                      Nov 2001 (FW): add ShadingDevice as alternative to ShadedConstruction
        //                      Dec 2001 (FW): add slat angle controls for blinds
        //                      Aug 2002 (FW): add Setpoint2; check that specified control type is legal
        //                      Feb 2003 (FW): add error if Material Name of Shading Device is used with
        //                        Shading Type = BetweenGlassShade or BetweenGlassBlind
        //                      Dec 2003 (FW): improve BetweenGlassBlind error messages
        //                      Feb 2009 (BG): improve error checking for OnIfScheduleAllows

        // PURPOSE OF THIS SUBROUTINE:
        // Reads in the window shading control information
        // from the input data file, interprets it and puts it in the derived type

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view routineName = "GetWindowShadingControlData";

        int constexpr NumValidShadingTypes(9);
        static Array1D_string const cValidShadingTypes(NumValidShadingTypes,
                                                       {
                                                           "SHADEOFF",          // 1
                                                           "INTERIORSHADE",     // 2
                                                           "SWITCHABLEGLAZING", // 3
                                                           "EXTERIORSHADE",     // 4
                                                           "EXTERIORSCREEN",    // 5
                                                           "INTERIORBLIND",     // 6
                                                           "EXTERIORBLIND",     // 7
                                                           "BETWEENGLASSSHADE", // 8
                                                           "BETWEENGLASSBLIND"  // 9
                                                       });

        constexpr std::array<std::string_view, static_cast<int>(DataSurfaces::WindowShadingControlType::Num)> WindowShadingControlTypeNamesUC{
            "ALWAYSON",
            "ALWAYSOFF",
            "ONIFSCHEDULEALLOWS",
            "ONIFHIGHSOLARONWINDOW",
            "ONIFHIGHHORIZONTALSOLAR",
            "ONIFHIGHOUTDOORAIRTEMPERATURE",
            "ONIFHIGHZONEAIRTEMPERATURE",
            "ONIFHIGHZONECOOLING",
            "ONIFHIGHGLARE",
            "MEETDAYLIGHTILLUMINANCESETPOINT",
            "ONNIGHTIFLOWOUTDOORTEMPANDOFFDAY",
            "ONNIGHTIFLOWINSIDETEMPANDOFFDAY",
            "ONNIGHTIFHEATINGANDOFFDAY",
            "ONNIGHTIFLOWOUTDOORTEMPANDONDAYIFCOOLING",
            "ONNIGHTIFHEATINGANDONDAYIFCOOLING",
            "OFFNIGHTANDONDAYIFCOOLINGANDHIGHSOLARONWINDOW",
            "ONNIGHTANDONDAYIFCOOLINGANDHIGHSOLARONWINDOW",
            "ONIFHIGHOUTDOORAIRTEMPANDHIGHSOLARONWINDOW",
            "ONIFHIGHOUTDOORAIRTEMPANDHIGHHORIZONTALSOLAR",
            "ONIFHIGHZONEAIRTEMPANDHIGHSOLARONWINDOW",
            "ONIFHIGHZONEAIRTEMPANDHIGHHORIZONTALSOLAR",
            "ONIFHIGHSOLARORHIGHLUMINANCETILLMIDNIGHT",
            "ONIFHIGHSOLARORHIGHLUMINANCETILLSUNSET",
            "ONIFHIGHSOLARORHIGHLUMINANCETILLNEXTMORNING"};

        constexpr std::array<std::string_view, static_cast<int>(DataSurfaces::SlatAngleControl::Num)> SlatAngleNamesUC{
            "FIXEDSLATANGLE", "SCHEDULEDSLATANGLE", "BLOCKBEAMSOLAR"};

        constexpr std::array<std::string_view, static_cast<int>(DataSurfaces::MultiSurfaceControl::Num)> MultiSurfaceControlNamesUC = {"SEQUENTIAL",
                                                                                                                                       "GROUP"};

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int IOStat;          // IO Status when calling get input subroutine
        int ControlNumAlpha; // Number of control alpha names being passed
        int ControlNumProp;  // Number of control properties being passed
        int ControlNum;      // DO loop counter/index for window shading control number
        int IShadedConst;    // Construction number of shaded construction
        int IShadingDevice;  // Material number of shading device
        int NLayers;         // Layers in shaded construction
        int Loop;
        bool BGShadeBlindError; // True if problem with construction that is supposed to have between-glass
        // shade or blind
        int Found;

        auto &s_mat = state.dataMaterial;
        auto &s_ipsc = state.dataIPShortCut;
        // Get the total number of window shading control blocks
        s_ipsc->cCurrentModuleObject = "WindowShadingControl";
        state.dataSurface->TotWinShadingControl = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        if (state.dataSurface->TotWinShadingControl == 0) {
            return;
        }

        state.dataSurface->WindowShadingControl.allocate(state.dataSurface->TotWinShadingControl);

        ControlNum = 0;
        for (Loop = 1; Loop <= state.dataSurface->TotWinShadingControl; ++Loop) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     s_ipsc->cAlphaArgs,
                                                                     ControlNumAlpha,
                                                                     s_ipsc->rNumericArgs,
                                                                     ControlNumProp,
                                                                     IOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            bool ErrorInName = false;
            bool IsBlank = false;

            Util::VerifyName(state,
                             s_ipsc->cAlphaArgs(1),
                             state.dataSurface->WindowShadingControl,
                             ControlNum,
                             ErrorInName,
                             IsBlank,
                             s_ipsc->cCurrentModuleObject + " Name");
            if (ErrorInName) {
                ErrorsFound = true;
                continue;
            }

            ++ControlNum;

            auto &windowShadingControl = state.dataSurface->WindowShadingControl(ControlNum);

            windowShadingControl.Name = s_ipsc->cAlphaArgs(1); // Set the Control Name in the Derived Type

            windowShadingControl.ZoneIndex = Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataHeatBal->Zone);
            if (windowShadingControl.ZoneIndex == 0) {
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cAlphaFieldNames(2),
                                       s_ipsc->cAlphaArgs(2)));
                ErrorsFound = true;
            }

            windowShadingControl.SequenceNumber = int(s_ipsc->rNumericArgs(1));

            // For upward compatibility change old "noninsulating" and "insulating" shade types to
            // INTERIORSHADE or EXTERIORSHADE
            if (s_ipsc->cAlphaArgs(3) == "INTERIORNONINSULATINGSHADE" || s_ipsc->cAlphaArgs(3) == "INTERIORINSULATINGSHADE") {
                ShowWarningError(state,
                                 format("{}=\"{}\" is using obsolete {}=\"{}\", changing to \"InteriorShade\"",
                                        s_ipsc->cCurrentModuleObject,
                                        windowShadingControl.Name,
                                        s_ipsc->cAlphaFieldNames(3),
                                        s_ipsc->cAlphaArgs(3)));
                windowShadingControl.ShadingType = DataSurfaces::WinShadingType::IntShade;
                s_ipsc->cAlphaArgs(3) = "INTERIORSHADE";
            }
            if (s_ipsc->cAlphaArgs(3) == "EXTERIORNONINSULATINGSHADE" || s_ipsc->cAlphaArgs(3) == "EXTERIORINSULATINGSHADE") {
                ShowWarningError(state,
                                 format("{}=\"{}\" is using obsolete {}=\"{}\", changing to \"ExteriorShade\"",
                                        s_ipsc->cCurrentModuleObject,
                                        windowShadingControl.Name,
                                        s_ipsc->cAlphaFieldNames(3),
                                        s_ipsc->cAlphaArgs(3)));
                windowShadingControl.ShadingType = DataSurfaces::WinShadingType::ExtShade;
                s_ipsc->cAlphaArgs(3) = "EXTERIORSHADE";
            }

            // Check for illegal shading type name
            Found = Util::FindItemInList(s_ipsc->cAlphaArgs(3), cValidShadingTypes, NumValidShadingTypes);
            if (Found <= 1) {
                ErrorsFound = true;
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\".",
                                       s_ipsc->cCurrentModuleObject,
                                       windowShadingControl.Name,
                                       s_ipsc->cAlphaFieldNames(3),
                                       s_ipsc->cAlphaArgs(3)));
            } else {
                windowShadingControl.ShadingType = DataSurfaces::WinShadingType(Found);
            }

            // WindowShadingControl().getInputShadedConstruction is only used during GetInput process and is ultimately stored in
            // Surface().shadedConstructionList
            windowShadingControl.getInputShadedConstruction =
                Util::FindItemInList(s_ipsc->cAlphaArgs(4), state.dataConstruction->Construct, state.dataHeatBal->TotConstructs);
            windowShadingControl.ShadingDevice = Material::GetMaterialNum(state, s_ipsc->cAlphaArgs(9));

            if (s_ipsc->lAlphaFieldBlanks(6)) {
                windowShadingControl.sched = Sched::GetScheduleAlwaysOn(state); // Not an availability schedule, but the default is constant-1.0
            } else if ((windowShadingControl.sched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(6))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6));
                ErrorsFound = true;
            }

            windowShadingControl.SetPoint = s_ipsc->rNumericArgs(2);
            windowShadingControl.SetPoint2 = s_ipsc->rNumericArgs(3);
            windowShadingControl.ShadingControlIsScheduled = getYesNoValue(s_ipsc->cAlphaArgs(7)) == BooleanSwitch::Yes;
            windowShadingControl.GlareControlIsActive = getYesNoValue(s_ipsc->cAlphaArgs(8)) == BooleanSwitch::Yes;

            if (windowShadingControl.ShadingType != DataSurfaces::WinShadingType::IntBlind &&
                windowShadingControl.ShadingType != DataSurfaces::WinShadingType::ExtBlind &&
                windowShadingControl.ShadingType != DataSurfaces::WinShadingType::BGBlind) {
            } else if (s_ipsc->cAlphaArgs(10).empty()) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(10), s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            } else if ((windowShadingControl.slatAngleControl = static_cast<DataSurfaces::SlatAngleControl>(
                            getEnumValue(SlatAngleNamesUC, s_ipsc->cAlphaArgs(10)))) == DataSurfaces::SlatAngleControl::Invalid) {
                ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(10), s_ipsc->cAlphaArgs(10));
                ErrorsFound = true;
            } else if (windowShadingControl.slatAngleControl != DataSurfaces::SlatAngleControl::Scheduled) {
            } else if (s_ipsc->lAlphaFieldBlanks(11)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(11), s_ipsc->cAlphaFieldNames(10), s_ipsc->cAlphaArgs(10));
                ErrorsFound = true;
            } else if ((windowShadingControl.slatAngleSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(11))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(11), s_ipsc->cAlphaArgs(11));
                ErrorsFound = true;
            }

            // store the string for now and associate it after daylighting control objects are read
            windowShadingControl.DaylightingControlName = s_ipsc->cAlphaArgs(12);

            windowShadingControl.multiSurfaceControl =
                static_cast<DataSurfaces::MultiSurfaceControl>(getEnumValue(MultiSurfaceControlNamesUC, s_ipsc->cAlphaArgs(13)));

            if (windowShadingControl.multiSurfaceControl == DataSurfaces::MultiSurfaceControl::Invalid) {
                windowShadingControl.multiSurfaceControl = DataSurfaces::MultiSurfaceControl::Sequential;
                ShowWarningError(state,
                                 format("{}=\"{}\" should be either SEQUENTIAL or GROUP {}=\"{}\", defaulting to \"SEQUENTIAL\"",
                                        s_ipsc->cCurrentModuleObject,
                                        windowShadingControl.Name,
                                        s_ipsc->cAlphaFieldNames(13),
                                        s_ipsc->cAlphaArgs(13)));
            }

            if (ControlNumAlpha >= 14) {
                windowShadingControl.FenestrationCount = ControlNumAlpha - 13;
                windowShadingControl.FenestrationName.allocate(windowShadingControl.FenestrationCount);
                windowShadingControl.FenestrationIndex.allocate(windowShadingControl.FenestrationCount);
                for (int i = 1; i <= windowShadingControl.FenestrationCount; i++) {
                    windowShadingControl.FenestrationName(i) = s_ipsc->cAlphaArgs(i + 13);
                }
            } else {
                ShowSevereError(state,
                                format("{}=\"{}\" invalid. Must reference at least one Fenestration Surface object name.",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1)));
            }

            windowShadingControl.shadingControlType = static_cast<DataSurfaces::WindowShadingControlType>(
                getEnumValue(WindowShadingControlTypeNamesUC, Util::makeUPPER(s_ipsc->cAlphaArgs(5))));

            if (windowShadingControl.ShadingDevice > 0) {
                if (s_mat->materials(windowShadingControl.ShadingDevice)->group == Material::Group::Screen &&
                    !(windowShadingControl.shadingControlType == DataSurfaces::WindowShadingControlType::AlwaysOn ||
                      windowShadingControl.shadingControlType == DataSurfaces::WindowShadingControlType::AlwaysOff ||
                      windowShadingControl.shadingControlType == DataSurfaces::WindowShadingControlType::OnIfScheduled)) {
                    ErrorsFound = true;
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=\"{}\" for exterior screens.",
                                           s_ipsc->cCurrentModuleObject,
                                           windowShadingControl.Name,
                                           s_ipsc->cAlphaFieldNames(5),
                                           s_ipsc->cAlphaArgs(5)));
                    ShowContinueError(state,
                                      "Valid shading control types for exterior window screens are ALWAYSON, ALWAYSOFF, or ONIFSCHEDULEALLOWS.");
                }
            } else {
                if (windowShadingControl.getInputShadedConstruction > 0) {
                    state.dataConstruction->Construct(windowShadingControl.getInputShadedConstruction).IsUsed = true;
                    if (s_mat->materials(state.dataConstruction->Construct(windowShadingControl.getInputShadedConstruction).LayerPoint(1))->group ==
                            Material::Group::Screen &&
                        !(windowShadingControl.shadingControlType == DataSurfaces::WindowShadingControlType::AlwaysOn ||
                          windowShadingControl.shadingControlType == DataSurfaces::WindowShadingControlType::AlwaysOff ||
                          windowShadingControl.shadingControlType == DataSurfaces::WindowShadingControlType::OnIfScheduled)) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\" invalid {}=\"{}\" for exterior screens.",
                                               s_ipsc->cCurrentModuleObject,
                                               windowShadingControl.Name,
                                               s_ipsc->cAlphaFieldNames(5),
                                               s_ipsc->cAlphaArgs(5)));
                        ShowContinueError(state,
                                          "Valid shading control types for exterior window screens are ALWAYSON, ALWAYSOFF, or ONIFSCHEDULEALLOWS.");
                    }
                } else if (s_ipsc->lAlphaFieldBlanks(4)) {
                    ShowSevereError(
                        state,
                        format("{}=\"{}\", {} is blank.", s_ipsc->cCurrentModuleObject, windowShadingControl.Name, s_ipsc->cAlphaFieldNames(4)));
                    ShowContinueError(state, "A valid construction is required.");
                    ErrorsFound = true;
                } else {
                    ShowSevereError(
                        state,
                        format("{}=\"{}\", {} is invalid.", s_ipsc->cCurrentModuleObject, windowShadingControl.Name, s_ipsc->cAlphaFieldNames(4)));
                    ShowContinueError(state, format("Construction=\"{}\" was used. A valid construction is required.", s_ipsc->cAlphaArgs(4)));
                    ErrorsFound = true;
                }
            }

            // Warning if setpoint is unintentionally zero
            if (windowShadingControl.SetPoint == 0 && windowShadingControl.shadingControlType != DataSurfaces::WindowShadingControlType::AlwaysOn &&
                windowShadingControl.shadingControlType != DataSurfaces::WindowShadingControlType::AlwaysOff &&
                windowShadingControl.shadingControlType != DataSurfaces::WindowShadingControlType::OnIfScheduled &&
                windowShadingControl.shadingControlType != DataSurfaces::WindowShadingControlType::HiGlare) {
                ShowWarningError(state, format("{}=\"{}\", The first SetPoint is zero.", s_ipsc->cCurrentModuleObject, windowShadingControl.Name));
                ShowContinueError(state, "..You may have forgotten to specify that setpoint.");
            }

            // Error checks
            if (BooleanSwitch bs = getYesNoValue(s_ipsc->cAlphaArgs(7)); bs == BooleanSwitch::Invalid) { // Shading Control is Schedule field
                ShowSevereInvalidBool(state, eoh, s_ipsc->cAlphaFieldNames(7), s_ipsc->cAlphaArgs(7));
                ErrorsFound = true;
            }

            if (BooleanSwitch bs = getYesNoValue(s_ipsc->cAlphaArgs(8)); bs == BooleanSwitch::Invalid) { // Shading Control is Schedule field
                ShowSevereInvalidBool(state, eoh, s_ipsc->cAlphaFieldNames(8), s_ipsc->cAlphaArgs(8));
                ErrorsFound = true;
            }

            if ((windowShadingControl.shadingControlType == DataSurfaces::WindowShadingControlType::OnIfScheduled) &&
                (!windowShadingControl.ShadingControlIsScheduled)) { // CR 7709 BG
                ErrorsFound = true;
                ShowSevereError(state,
                                format("{} = \"{}\" invalid, {} must be set to \"Yes\" for {} = OnIfScheduleAllows",
                                       s_ipsc->cCurrentModuleObject,
                                       windowShadingControl.Name,
                                       s_ipsc->cAlphaFieldNames(7),
                                       s_ipsc->cAlphaFieldNames(5)));
            }

            if (windowShadingControl.shadingControlType == DataSurfaces::WindowShadingControlType::MeetDaylIlumSetp &&
                windowShadingControl.ShadingType != DataSurfaces::WinShadingType::SwitchableGlazing) {
                ErrorsFound = true;
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\".",
                                       s_ipsc->cCurrentModuleObject,
                                       windowShadingControl.Name,
                                       s_ipsc->cAlphaFieldNames(3),
                                       s_ipsc->cAlphaArgs(3)));
                ShowContinueError(state,
                                  format("...{} must be SwitchableGlazing for this control, but entered type=\"{}\".",
                                         s_ipsc->cAlphaFieldNames(3),
                                         s_ipsc->cAlphaArgs(3)));
            }

            DataSurfaces::WinShadingType ShTyp = windowShadingControl.ShadingType;
            IShadedConst = windowShadingControl.getInputShadedConstruction;
            IShadingDevice = windowShadingControl.ShadingDevice;

            if (IShadedConst == 0 && IShadingDevice == 0) {
                ShowSevereError(state,
                                format("{}=\"{}\" has no matching shaded construction or shading device.",
                                       s_ipsc->cCurrentModuleObject,
                                       windowShadingControl.Name));
                ErrorsFound = true;
            } else if (IShadedConst == 0 && IShadingDevice > 0) {
                if (ShTyp == DataSurfaces::WinShadingType::SwitchableGlazing) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" has {}= SwitchableGlazing but no matching shaded construction",
                                           s_ipsc->cCurrentModuleObject,
                                           windowShadingControl.Name,
                                           s_ipsc->cAlphaArgs(3)));
                    ErrorsFound = true;
                }
                if ((ShTyp == DataSurfaces::WinShadingType::IntShade || ShTyp == DataSurfaces::WinShadingType::ExtShade) &&
                    s_mat->materials(IShadingDevice)->group != Material::Group::Shade) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" has {}= InteriorShade or ExteriorShade but matching shading device is not a window shade",
                                           s_ipsc->cCurrentModuleObject,
                                           windowShadingControl.Name,
                                           s_ipsc->cAlphaArgs(3)));
                    ShowContinueError(state, format("{} in error=\"{}\".", s_ipsc->cAlphaFieldNames(8), s_mat->materials(IShadingDevice)->Name));
                    ErrorsFound = true;
                }
                if ((ShTyp == DataSurfaces::WinShadingType::ExtScreen) && s_mat->materials(IShadingDevice)->group != Material::Group::Screen) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" has {}= ExteriorScreen but matching shading device is not a window screen",
                                           s_ipsc->cCurrentModuleObject,
                                           windowShadingControl.Name,
                                           s_ipsc->cAlphaArgs(3)));
                    ShowContinueError(state, format("{} in error=\"{}\".", s_ipsc->cAlphaFieldNames(8), s_mat->materials(IShadingDevice)->Name));
                    ErrorsFound = true;
                }
                if ((ShTyp == DataSurfaces::WinShadingType::IntBlind || ShTyp == DataSurfaces::WinShadingType::ExtBlind) &&
                    s_mat->materials(IShadingDevice)->group != Material::Group::Blind) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" has {}= InteriorBlind or ExteriorBlind but matching shading device is not a window blind",
                                           s_ipsc->cCurrentModuleObject,
                                           windowShadingControl.Name,
                                           s_ipsc->cAlphaArgs(3)));
                    ShowContinueError(state, format("{} in error=\"{}\".", s_ipsc->cAlphaFieldNames(8), s_mat->materials(IShadingDevice)->Name));
                    ErrorsFound = true;
                }
                if (ShTyp == DataSurfaces::WinShadingType::BGShade || ShTyp == DataSurfaces::WinShadingType::BGBlind) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" has {}= BetweenGlassShade or BetweenGlassBlind and",
                                           s_ipsc->cCurrentModuleObject,
                                           windowShadingControl.Name,
                                           s_ipsc->cAlphaArgs(3)));
                    ShowContinueError(state,
                                      format("{} is specified. This is illegal. Specify shaded construction instead.", s_ipsc->cAlphaFieldNames(8)));
                    ErrorsFound = true;
                }
            } else if (IShadedConst > 0 && IShadingDevice > 0) {
                IShadingDevice = 0;
                ShowWarningError(state,
                                 format("{}=\"{}\" Both {} and {} are specified.",
                                        s_ipsc->cCurrentModuleObject,
                                        windowShadingControl.Name,
                                        s_ipsc->cAlphaFieldNames(4),
                                        s_ipsc->cAlphaFieldNames(9)));
                ShowContinueError(
                    state, format("The {}=\"{}\" will be used.", s_ipsc->cAlphaFieldNames(4), state.dataConstruction->Construct(IShadedConst).Name));
            }

            // If type = interior or exterior shade or blind require that the shaded construction
            // have a shade layer in the correct position
            if (IShadedConst != 0) {

                NLayers = state.dataConstruction->Construct(IShadedConst).TotLayers;
                BGShadeBlindError = false;
                IShadingDevice = 0;
                if (state.dataConstruction->Construct(IShadedConst).LayerPoint(NLayers) != 0) {
                    if (windowShadingControl.ShadingType == DataSurfaces::WinShadingType::IntShade) {
                        IShadingDevice = state.dataConstruction->Construct(IShadedConst).LayerPoint(NLayers);
                        if (s_mat->materials(state.dataConstruction->Construct(IShadedConst).LayerPoint(NLayers))->group != Material::Group::Shade) {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format("{}=\"{}\" the {}=\"{}\"",
                                                   s_ipsc->cCurrentModuleObject,
                                                   windowShadingControl.Name,
                                                   s_ipsc->cAlphaFieldNames(4),
                                                   s_ipsc->cAlphaArgs(4)));
                            ShowContinueError(state,
                                              format("of {}=\"{}\" should have a shade layer on the inside of the window.",
                                                     s_ipsc->cAlphaFieldNames(3),
                                                     s_ipsc->cAlphaArgs(3)));
                        }
                    } else if (windowShadingControl.ShadingType == DataSurfaces::WinShadingType::ExtShade) {
                        IShadingDevice = state.dataConstruction->Construct(IShadedConst).LayerPoint(1);
                        if (s_mat->materials(state.dataConstruction->Construct(IShadedConst).LayerPoint(1))->group != Material::Group::Shade) {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format("{}=\"{}\" the {}=\"{}\"",
                                                   s_ipsc->cCurrentModuleObject,
                                                   windowShadingControl.Name,
                                                   s_ipsc->cAlphaFieldNames(43),
                                                   s_ipsc->cAlphaArgs(4)));
                            ShowContinueError(state,
                                              format("of {}=\"{}\" should have a shade layer on the outside of the window.",
                                                     s_ipsc->cAlphaFieldNames(3),
                                                     s_ipsc->cAlphaArgs(3)));
                        }
                    } else if (windowShadingControl.ShadingType == DataSurfaces::WinShadingType::ExtScreen) {
                        IShadingDevice = state.dataConstruction->Construct(IShadedConst).LayerPoint(1);
                        if (s_mat->materials(state.dataConstruction->Construct(IShadedConst).LayerPoint(1))->group != Material::Group::Screen) {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format("{}=\"{}\" the {}=\"{}\"",
                                                   s_ipsc->cCurrentModuleObject,
                                                   windowShadingControl.Name,
                                                   s_ipsc->cAlphaFieldNames(4),
                                                   s_ipsc->cAlphaArgs(4)));
                            ShowContinueError(state,
                                              format("of {}=\"{}\" should have a screen layer on the outside of the window.",
                                                     s_ipsc->cAlphaFieldNames(3),
                                                     s_ipsc->cAlphaArgs(3)));
                        }
                    } else if (windowShadingControl.ShadingType == DataSurfaces::WinShadingType::IntBlind) {
                        IShadingDevice = state.dataConstruction->Construct(IShadedConst).LayerPoint(NLayers);
                        if (s_mat->materials(state.dataConstruction->Construct(IShadedConst).LayerPoint(NLayers))->group != Material::Group::Blind) {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format("{}=\"{}\" the {}=\"{}\"",
                                                   s_ipsc->cCurrentModuleObject,
                                                   windowShadingControl.Name,
                                                   s_ipsc->cAlphaFieldNames(4),
                                                   s_ipsc->cAlphaArgs(4)));
                            ShowContinueError(state,
                                              format("of {}=\"{}\" should have a blind layer on the inside of the window.",
                                                     s_ipsc->cAlphaFieldNames(3),
                                                     s_ipsc->cAlphaArgs(3)));
                        }
                    } else if (windowShadingControl.ShadingType == DataSurfaces::WinShadingType::ExtBlind) {
                        IShadingDevice = state.dataConstruction->Construct(IShadedConst).LayerPoint(1);
                        if (s_mat->materials(state.dataConstruction->Construct(IShadedConst).LayerPoint(1))->group != Material::Group::Blind) {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format("{}=\"{}\" the {}=\"{}\"",
                                                   s_ipsc->cCurrentModuleObject,
                                                   windowShadingControl.Name,
                                                   s_ipsc->cAlphaFieldNames(4),
                                                   s_ipsc->cAlphaArgs(4)));
                            ShowContinueError(state,
                                              format("of {}=\"{}\" should have a blind layer on the outside of the window.",
                                                     s_ipsc->cAlphaFieldNames(3),
                                                     s_ipsc->cAlphaArgs(3)));
                        }
                    } else if (windowShadingControl.ShadingType == DataSurfaces::WinShadingType::BGShade) {
                        if (NLayers != 5 && NLayers != 7) {
                            BGShadeBlindError = true;
                        }
                        if (NLayers == 5) {
                            if (s_mat->materials(state.dataConstruction->Construct(IShadedConst).LayerPoint(3))->group != Material::Group::Shade) {
                                BGShadeBlindError = true;
                            }
                        }
                        if (NLayers == 7) {
                            if (s_mat->materials(state.dataConstruction->Construct(IShadedConst).LayerPoint(5))->group != Material::Group::Shade) {
                                BGShadeBlindError = true;
                            }
                        }
                        if (BGShadeBlindError) {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format("{}=\"{}\" the {}=\"{}\"",
                                                   s_ipsc->cCurrentModuleObject,
                                                   windowShadingControl.Name,
                                                   s_ipsc->cAlphaFieldNames(4),
                                                   s_ipsc->cAlphaArgs(4)));
                            ShowContinueError(state,
                                              format("of {}=\"{}\" should have two or three glass layers and a",
                                                     s_ipsc->cAlphaFieldNames(3),
                                                     s_ipsc->cAlphaArgs(3)));
                            ShowContinueError(state, "between-glass shade layer with a gas layer on each side.");
                        }
                    } else if (windowShadingControl.ShadingType == DataSurfaces::WinShadingType::BGBlind) {
                        if (NLayers != 5 && NLayers != 7) {
                            BGShadeBlindError = true;
                        }
                        if (NLayers == 5) {
                            if (s_mat->materials(state.dataConstruction->Construct(IShadedConst).LayerPoint(3))->group != Material::Group::Blind) {
                                BGShadeBlindError = true;
                            }
                        }
                        if (NLayers == 7) {
                            if (s_mat->materials(state.dataConstruction->Construct(IShadedConst).LayerPoint(5))->group != Material::Group::Blind) {
                                BGShadeBlindError = true;
                            }
                        }
                        if (BGShadeBlindError) {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format("{}=\"{}\" the {}=\"{}\"",
                                                   s_ipsc->cCurrentModuleObject,
                                                   windowShadingControl.Name,
                                                   s_ipsc->cAlphaFieldNames(4),
                                                   s_ipsc->cAlphaArgs(4)));
                            ShowContinueError(state,
                                              format("of {}=\"{}\" should have two or three glass layers and a",
                                                     s_ipsc->cAlphaFieldNames(3),
                                                     s_ipsc->cAlphaArgs(3)));
                            ShowContinueError(state, "between-glass blind layer with a gas layer on each side.");
                        }
                    }
                }
                if (IShadingDevice > 0) {
                    if ((ShTyp == DataSurfaces::WinShadingType::IntShade || ShTyp == DataSurfaces::WinShadingType::ExtShade) &&
                        s_mat->materials(IShadingDevice)->group != Material::Group::Shade) {
                        ShowSevereError(state,
                                        format("{}=\"{}\" has {}= InteriorShade or ExteriorShade but matching shading device is not a window shade",
                                               s_ipsc->cCurrentModuleObject,
                                               windowShadingControl.Name,
                                               s_ipsc->cAlphaFieldNames(3)));
                        ShowContinueError(state, format("Shading Device in error=\"{}\".", s_mat->materials(IShadingDevice)->Name));
                        ErrorsFound = true;
                    }
                    if ((ShTyp == DataSurfaces::WinShadingType::ExtScreen) && s_mat->materials(IShadingDevice)->group != Material::Group::Screen) {
                        ShowSevereError(state,
                                        format("{}=\"{}\" has {}= ExteriorScreen but matching shading device is not an exterior window screen.",
                                               s_ipsc->cCurrentModuleObject,
                                               windowShadingControl.Name,
                                               s_ipsc->cAlphaFieldNames(3)));
                        ShowContinueError(state, format("Shading Device in error=\"{}\".", s_mat->materials(IShadingDevice)->Name));
                        ErrorsFound = true;
                    }
                    if ((ShTyp == DataSurfaces::WinShadingType::IntBlind || ShTyp == DataSurfaces::WinShadingType::ExtBlind) &&
                        s_mat->materials(IShadingDevice)->group != Material::Group::Blind) {
                        ShowSevereError(state,
                                        format("{}=\"{}\" has {}= InteriorBlind or ExteriorBlind but matching shading device is not a window blind.",
                                               s_ipsc->cCurrentModuleObject,
                                               windowShadingControl.Name,
                                               s_ipsc->cAlphaFieldNames(3)));
                        ShowContinueError(state, format("Shading Device in error=\"{}\".", s_mat->materials(IShadingDevice)->Name));
                        ErrorsFound = true;
                    }
                }
            }
        } // End of loop over window shading controls
    }

    void InitialAssociateWindowShadingControlFenestration(EnergyPlusData &state, bool &ErrorsFound, int SurfNum)
    {
        auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
        // J.Glazer 2018 - operates on SurfaceTmp array before final indices are known for windows and sets the activeWindowShadingControl
        for (int iShadeCtrl = 1; iShadeCtrl <= state.dataSurface->TotWinShadingControl; ++iShadeCtrl) {
            int curShadedConstruction = state.dataSurface->WindowShadingControl(iShadeCtrl).getInputShadedConstruction;
            for (int jFeneRef = 1; jFeneRef <= state.dataSurface->WindowShadingControl(iShadeCtrl).FenestrationCount; ++jFeneRef) {
                if (Util::SameString(state.dataSurface->WindowShadingControl(iShadeCtrl).FenestrationName(jFeneRef), surfTemp.Name)) {
                    state.dataGlobal->AndShadingControlInModel = true;
                    surfTemp.HasShadeControl = true;
                    surfTemp.windowShadingControlList.push_back(iShadeCtrl);
                    surfTemp.activeWindowShadingControl = iShadeCtrl;
                    surfTemp.shadedConstructionList.push_back(curShadedConstruction);
                    surfTemp.activeShadedConstruction = curShadedConstruction;

                    // check to make the window refenced is an exterior window
                    if (surfTemp.ExtBoundCond != DataSurfaces::ExternalEnvironment) {
                        ErrorsFound = true;
                        ShowSevereError(
                            state,
                            format("InitialAssociateWindowShadingControlFenestration: \"{}\", invalid  because it is not an exterior window.",
                                   surfTemp.Name));
                        ShowContinueError(
                            state,
                            format(".. It appears on WindowShadingControl object: \"{}", state.dataSurface->WindowShadingControl(iShadeCtrl).Name));
                    }
                    // check to make sure the window is not using equivalent layer window construction
                    if (state.dataConstruction->Construct(surfTemp.Construction).WindowTypeEQL) {
                        ErrorsFound = true;
                        ShowSevereError(state, format("InitialAssociateWindowShadingControlFenestration: =\"{}\", invalid \".", surfTemp.Name));
                        ShowContinueError(state, ".. equivalent layer window model does not use shading control object.");
                        ShowContinueError(state, ".. Shading control is set to none or zero, and simulation continues.");
                        ShowContinueError(
                            state,
                            format(".. It appears on WindowShadingControl object: \"{}", state.dataSurface->WindowShadingControl(iShadeCtrl).Name));
                        surfTemp.activeWindowShadingControl = 0;
                    }
                }
            }
        }
    }

    void FinalAssociateWindowShadingControlFenestration(EnergyPlusData &state, bool &ErrorsFound)
    {
        // J.Glazer 2018 - operates on Surface array after final indices are known for windows and checks to make sure it is correct
        for (int iShadeCtrl = 1; iShadeCtrl <= state.dataSurface->TotWinShadingControl; ++iShadeCtrl) {
            for (int jFeneRef = 1; jFeneRef <= state.dataSurface->WindowShadingControl(iShadeCtrl).FenestrationCount; ++jFeneRef) {
                int fenestrationIndex = Util::FindItemInList(state.dataSurface->WindowShadingControl(iShadeCtrl).FenestrationName(jFeneRef),
                                                             state.dataSurface->Surface,
                                                             state.dataSurface->TotSurfaces);
                if (std::find(state.dataSurface->Surface(fenestrationIndex).windowShadingControlList.begin(),
                              state.dataSurface->Surface(fenestrationIndex).windowShadingControlList.end(),
                              iShadeCtrl) != state.dataSurface->Surface(fenestrationIndex).windowShadingControlList.end()) {
                    state.dataSurface->WindowShadingControl(iShadeCtrl).FenestrationIndex(jFeneRef) = fenestrationIndex;
                } else {
                    // this error condition should not occur since the rearrangement of Surface() from SurfureTmp() is reliable.
                    ErrorsFound = true;
                    ShowSevereError(state,
                                    format("FinalAssociateWindowShadingControlFenestration: Fenestration surface named \"{}\" has "
                                           "WindowShadingContol index that does not match the initial index assigned.",
                                           state.dataSurface->Surface(fenestrationIndex).Name));
                    ShowContinueError(state,
                                      format("This occurs while WindowShadingControl object: \"{}\" is being evaluated. ",
                                             state.dataSurface->WindowShadingControl(iShadeCtrl).Name));
                }
            }
        }
    }

    void CheckWindowShadingControlSimilarForWindow(EnergyPlusData &state, bool &ErrorsFound)
    {
        // For each window check if all window shading controls on list are the same except for name, schedule name, construction, and
        // material
        for (auto &theSurf : state.dataSurface->Surface) {
            if (theSurf.HasShadeControl) {
                if (theSurf.windowShadingControlList.size() > 1) {
                    int firstWindowShadingControl = theSurf.windowShadingControlList.front();
                    for (auto wsc = std::next(theSurf.windowShadingControlList.begin()); wsc != theSurf.windowShadingControlList.end(); ++wsc) {
                        if (!isWindowShadingControlSimilar(state, firstWindowShadingControl, *wsc)) {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format("CheckWindowShadingControlSimilarForWindow: Fenestration surface named \"{}\" has multiple "
                                                   "WindowShadingContols that are not similar.",
                                                   theSurf.Name));
                            ShowContinueError(state,
                                              format("for: \"{} and: {}",
                                                     state.dataSurface->WindowShadingControl(firstWindowShadingControl).Name,
                                                     state.dataSurface->WindowShadingControl(*wsc).Name));
                        }
                    }
                }
            }
        }
    }

    bool isWindowShadingControlSimilar(EnergyPlusData &state, int a, int b)
    {
        // Compares two window shading controls are the same except for the name, schedule name, construction, and material
        auto const &WindowShadingControlA = state.dataSurface->WindowShadingControl(a);
        auto const &WindowShadingControlB = state.dataSurface->WindowShadingControl(b);
        return (WindowShadingControlA.ZoneIndex == WindowShadingControlB.ZoneIndex &&
                WindowShadingControlA.ShadingType == WindowShadingControlB.ShadingType &&
                WindowShadingControlA.shadingControlType == WindowShadingControlB.shadingControlType &&
                WindowShadingControlA.SetPoint == WindowShadingControlB.SetPoint &&
                WindowShadingControlA.ShadingControlIsScheduled == WindowShadingControlB.ShadingControlIsScheduled &&
                WindowShadingControlA.GlareControlIsActive == WindowShadingControlB.GlareControlIsActive &&
                WindowShadingControlA.slatAngleControl == WindowShadingControlB.slatAngleControl &&
                WindowShadingControlA.SetPoint2 == WindowShadingControlB.SetPoint2 &&
                WindowShadingControlA.DaylightingControlName == WindowShadingControlB.DaylightingControlName &&
                WindowShadingControlA.DaylightControlIndex == WindowShadingControlB.DaylightControlIndex &&
                WindowShadingControlA.multiSurfaceControl == WindowShadingControlB.multiSurfaceControl);
    }

    void GetStormWindowData(EnergyPlusData &state, bool &ErrorsFound) // If errors found in input
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   December 2003

        // PURPOSE OF THIS SUBROUTINE:
        // Reads in the storm window data from the input file, interprets it and puts it in the derived type

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int IOStat;           // IO Status when calling get input subroutine
        int StormWinNumAlpha; // Number of alpha names being passed
        int StormWinNumProp;  // Number of properties being passed
        int loop;             // Do loop counter

        auto &s_ipsc = state.dataIPShortCut;
        auto &s_mat = state.dataMaterial;

        // Get the total number of storm window input objects
        s_ipsc->cCurrentModuleObject = "WindowProperty:StormWindow";
        state.dataSurface->TotStormWin = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        if (state.dataSurface->TotStormWin == 0) {
            return;
        }

        state.dataSurface->StormWindow.allocate(state.dataSurface->TotStormWin);

        int StormWinNum = 0;
        for (loop = 1; loop <= state.dataSurface->TotStormWin; ++loop) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     loop,
                                                                     s_ipsc->cAlphaArgs,
                                                                     StormWinNumAlpha,
                                                                     s_ipsc->rNumericArgs,
                                                                     StormWinNumProp,
                                                                     IOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);
            ++StormWinNum;
            state.dataSurface->StormWindow(StormWinNum).BaseWindowNum =
                Util::FindItemInList(s_ipsc->cAlphaArgs(1), state.dataSurface->Surface, state.dataSurface->TotSurfaces);
            state.dataSurface->StormWindow(StormWinNum).StormWinMaterialNum = Material::GetMaterialNum(state, s_ipsc->cAlphaArgs(2));
            state.dataSurface->StormWindow(StormWinNum).StormWinDistance = s_ipsc->rNumericArgs(1);
            state.dataSurface->StormWindow(StormWinNum).MonthOn = s_ipsc->rNumericArgs(2);
            state.dataSurface->StormWindow(StormWinNum).DayOfMonthOn = s_ipsc->rNumericArgs(3);
            state.dataSurface->StormWindow(StormWinNum).DateOn =
                General::OrdinalDay(state.dataSurface->StormWindow(StormWinNum).MonthOn, state.dataSurface->StormWindow(StormWinNum).DayOfMonthOn, 1);
            state.dataSurface->StormWindow(StormWinNum).MonthOff = s_ipsc->rNumericArgs(4);
            state.dataSurface->StormWindow(StormWinNum).DayOfMonthOff = s_ipsc->rNumericArgs(5);
            state.dataSurface->StormWindow(StormWinNum).DateOff = General::OrdinalDay(
                state.dataSurface->StormWindow(StormWinNum).MonthOff, state.dataSurface->StormWindow(StormWinNum).DayOfMonthOff, 1);

            if (state.dataSurface->StormWindow(StormWinNum).DateOn == state.dataSurface->StormWindow(StormWinNum).DateOff) {
                ShowSevereError(state,
                                format("{}: Date On = Date Off -- not allowed, occurred in WindowProperty:StormWindow Input #{}",
                                       s_ipsc->cCurrentModuleObject,
                                       StormWinNum));
                ErrorsFound = true;
            }

            enum Month
            {
                January = 1,
                February,
                March,
                April,
                May,
                June,
                July,
                August,
                September,
                October,
                November,
                December
            };
            constexpr std::array<int, 13> oneBasedDaysInMonth = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

            int const monthOn = state.dataSurface->StormWindow(StormWinNum).MonthOn;
            if (monthOn >= January && monthOn <= December) {
                if (state.dataSurface->StormWindow(StormWinNum).DayOfMonthOn >
                    oneBasedDaysInMonth[state.dataSurface->StormWindow(StormWinNum).MonthOn]) {
                    ShowSevereError(state,
                                    format("{}: Date On (Day of Month) [{}], invalid for WindowProperty:StormWindow Input #{}",
                                           s_ipsc->cCurrentModuleObject,
                                           state.dataSurface->StormWindow(StormWinNum).DayOfMonthOn,
                                           StormWinNum));
                    ErrorsFound = true;
                }
                break;
            } else {
                ShowSevereError(state,
                                format("{}: Date On Month [{}], invalid for WindowProperty:StormWindow Input #{}",
                                       s_ipsc->cCurrentModuleObject,
                                       state.dataSurface->StormWindow(StormWinNum).MonthOn,
                                       StormWinNum));
                ErrorsFound = true;
            }

            int const monthOff = state.dataSurface->StormWindow(StormWinNum).MonthOff;
            if (monthOff >= January && monthOff <= December) {
                if (state.dataSurface->StormWindow(StormWinNum).DayOfMonthOff >
                    oneBasedDaysInMonth[state.dataSurface->StormWindow(StormWinNum).MonthOff]) {
                    ShowSevereError(state,
                                    format("{}: Date Off (Day of Month) [{}], invalid for WindowProperty:StormWindow Input #{}",
                                           s_ipsc->cCurrentModuleObject,
                                           state.dataSurface->StormWindow(StormWinNum).DayOfMonthOff,
                                           StormWinNum));
                    ErrorsFound = true;
                }
                break;
            } else {
                ShowSevereError(state,
                                format("{}: Date Off Month [{}], invalid for WindowProperty:StormWindow Input #{}",
                                       s_ipsc->cCurrentModuleObject,
                                       state.dataSurface->StormWindow(StormWinNum).MonthOff,
                                       StormWinNum));
                ErrorsFound = true;
            }
        }

        // Error checks

        for (StormWinNum = 1; StormWinNum <= state.dataSurface->TotStormWin; ++StormWinNum) {
            // Require BaseWindowNum be that of an exterior window
            int SurfNum = state.dataSurface->StormWindow(StormWinNum).BaseWindowNum;
            if (SurfNum == 0) {
                ShowSevereError(state, format("{}=\"{}\" invalid.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            } else {
                auto const &surf = state.dataSurface->Surface(SurfNum);
                if (surf.Class != SurfaceClass::Window || surf.ExtBoundCond != 0) {
                    ShowSevereError(state, format("{}=\"{}\"", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowSevereError(state, format("cannot be used with surface={}", surf.Name));
                    ShowContinueError(state, "because that surface is not an exterior window.");
                    ErrorsFound = true;
                }
            }

            // Require that storm window material be glass
            int MatNum = state.dataSurface->StormWindow(StormWinNum).StormWinMaterialNum;
            if (SurfNum > 0) {
                if (MatNum == 0) {
                    ShowSevereError(state, format("{}=\"{}\"", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state,
                                      format("{}=\"{}\" not found as storm window layer.", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
                    ErrorsFound = true;
                } else {
                    if (s_mat->materials(MatNum)->group != Material::Group::Glass) {
                        ShowSevereError(state, format("{}=\"{}\"", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                        ShowContinueError(state,
                                          format("{}=\"{}must be a WindowMaterial:Glazing or WindowMaterial:Glazing:RefractionExtinctionMethod",
                                                 s_ipsc->cAlphaFieldNames(2),
                                                 s_ipsc->cAlphaArgs(2)));
                        ErrorsFound = true;
                    }
                }
                // Error if base window has airflow control
                if (state.dataSurface->SurfWinAirflowControlType(SurfNum) != DataSurfaces::WindowAirFlowControlType::Invalid) {
                    ShowSevereError(
                        state,
                        format("{}=\"{} cannot be used because it is an airflow window (i.e., has WindowProperty:AirflowControl specified)",
                               s_ipsc->cCurrentModuleObject,
                               s_ipsc->cAlphaArgs(1)));
                    ErrorsFound = true;
                }
            }

            // Check for reversal of on and off times
            if (SurfNum > 0) {
                if ((state.dataEnvrn->Latitude > 0.0 &&
                     (state.dataSurface->StormWindow(StormWinNum).MonthOn < state.dataSurface->StormWindow(StormWinNum).MonthOff)) ||
                    (state.dataEnvrn->Latitude <= 0.0 &&
                     (state.dataSurface->StormWindow(StormWinNum).MonthOn > state.dataSurface->StormWindow(StormWinNum).MonthOff))) {
                    ShowWarningError(state, format("{}=\"{}\" check times that storm window", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state,
                                      format("is put on (month={}, day={}) and taken off (month={}, day={});",
                                             state.dataSurface->StormWindow(StormWinNum).MonthOn,
                                             state.dataSurface->StormWindow(StormWinNum).DayOfMonthOn,
                                             state.dataSurface->StormWindow(StormWinNum).MonthOff,
                                             state.dataSurface->StormWindow(StormWinNum).DayOfMonthOff));
                    ShowContinueError(state, format("these times may be reversed for your building latitude={:.2R} deg.", state.dataEnvrn->Latitude));
                }
            }
        }
    }

    void GetWindowGapAirflowControlData(EnergyPlusData &state, bool &ErrorsFound) // If errors found in input
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   Feb 2003
        //       MODIFIED       June 2003, FCW: add destination = return air;
        //                        more error messages

        // PURPOSE OF THIS SUBROUTINE:
        // Reads in the window airflow control information from the input data file,
        // interprets it and puts it in the SurfaceWindow derived type

        static constexpr std::string_view routineName = "GetWindowGapAirflowControlData";

        int IOStat;               // IO Status when calling get input subroutine
        int ControlNumAlpha;      // Number of control alpha names being passed
        int ControlNumProp;       // Number of control properties being passed
        int TotWinAirflowControl; // Total window airflow control statements
        int Loop;
        int ConstrNum(0); // Construction number
        int ConstrNumSh;  // Shaded Construction number
        int MatGapFlow;   // Material number of gas in airflow gap of window's construction
        int MatGapFlow1;  // Material number of gas on either side of a between-glass shade/blind
        int MatGapFlow2;

        constexpr std::array<std::string_view, static_cast<int>(DataSurfaces::WindowAirFlowSource::Num)> WindowAirFlowSourceNamesUC{"INDOORAIR",
                                                                                                                                    "OUTDOORAIR"};
        constexpr std::array<std::string_view, static_cast<int>(DataSurfaces::WindowAirFlowDestination::Num)> WindowAirFlowDestinationNamesUC{
            "INDOORAIR", "OUTDOORAIR", "RETURNAIR"};

        // of the shaded construction of airflow window
        auto &s_ipsc = state.dataIPShortCut;
        auto &s_mat = state.dataMaterial;

        // Get the total number of window airflow control statements
        s_ipsc->cCurrentModuleObject = "WindowProperty:AirflowControl";
        TotWinAirflowControl = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        if (TotWinAirflowControl == 0) {
            return;
        }

        for (Loop = 1; Loop <= TotWinAirflowControl; ++Loop) { // Loop through all surfaces in the input...

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     s_ipsc->cAlphaArgs,
                                                                     ControlNumAlpha,
                                                                     s_ipsc->rNumericArgs,
                                                                     ControlNumProp,
                                                                     IOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            int SurfNum = Util::FindItemInList(s_ipsc->cAlphaArgs(1), state.dataSurface->Surface, state.dataSurface->TotSurfaces);
            if (SurfNum == 0) {
                ShowSevereError(state, format("{}=\"{}\" not found.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            }
            // Check that associated surface is a 2- or 3-pane exterior window
            if (SurfNum != 0) {
                bool WrongSurfaceType = false;
                auto const &surf = state.dataSurface->Surface(SurfNum);
                if (surf.Class != SurfaceClass::Window) {
                    WrongSurfaceType = true;
                }
                if (surf.Class == SurfaceClass::Window) {
                    ConstrNum = surf.Construction;
                    if (state.dataConstruction->Construct(ConstrNum).TotGlassLayers != 2 &&
                        state.dataConstruction->Construct(ConstrNum).TotGlassLayers != 3) {
                        WrongSurfaceType = true;
                    }
                    if (surf.ExtBoundCond != DataSurfaces::ExternalEnvironment) {
                        WrongSurfaceType = true;
                    }
                }
                if (WrongSurfaceType) {
                    ShowSevereError(
                        state,
                        format("{}=\"{}\" is not an exterior window with 2 or 3 glass layers.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ErrorsFound = true;
                }
            }

            // Error if illegal airflow source
            if (s_ipsc->cAlphaArgs(2) != "INDOORAIR" && s_ipsc->cAlphaArgs(2) != "OUTDOORAIR") {
                ErrorsFound = true;
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\"",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cAlphaFieldNames(2),
                                       s_ipsc->cAlphaArgs(2)));
            }

            // Error if illegal airflow destination
            if (s_ipsc->cAlphaArgs(3) != "INDOORAIR" && s_ipsc->cAlphaArgs(3) != "OUTDOORAIR" && s_ipsc->cAlphaArgs(3) != "RETURNAIR") {
                ErrorsFound = true;
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\"",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cAlphaFieldNames(3),
                                       s_ipsc->cAlphaArgs(3)));
            }

            // Error if source = OutsideAir and destination = ReturnAir
            if (s_ipsc->cAlphaArgs(2) == "OUTDOORAIR" && s_ipsc->cAlphaArgs(3) == "RETURNAIR") {
                ErrorsFound = true;
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\"",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cAlphaFieldNames(2),
                                       s_ipsc->cAlphaArgs(2)));
                ShowContinueError(state, format("..when {}=\"{}\"", s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3)));
            }

            // Error if illegal airflow control type
            if (s_ipsc->cAlphaArgs(4) != "ALWAYSONATMAXIMUMFLOW" && s_ipsc->cAlphaArgs(4) != "ALWAYSOFF" &&
                s_ipsc->cAlphaArgs(4) != "SCHEDULEDONLY") {
                ErrorsFound = true;
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\"",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cAlphaFieldNames(4),
                                       s_ipsc->cAlphaArgs(4)));
            }

            // Error if illegal value for Airflow Has Multiplier Schedule
            if (s_ipsc->cAlphaArgs(5) != "YES" && s_ipsc->cAlphaArgs(5) != "NO") {
                ErrorsFound = true;
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\"",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cAlphaFieldNames(5),
                                       s_ipsc->cAlphaArgs(5)));
            }

            // Error if Airflow Control Type = ScheduledOnly and Airflow Has Multiplier Schedule = No
            if (s_ipsc->cAlphaArgs(4) == "SCHEDULEDONLY" && s_ipsc->cAlphaArgs(5) == "NO") {
                ErrorsFound = true;
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\"",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cAlphaFieldNames(4),
                                       s_ipsc->cAlphaArgs(4)));
                ShowContinueError(state, format("..when {}=\"{}\"", s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5)));
            }

            // Warning if Airflow Control Type = AlwaysOnAtMaxFlow and Airflow Has Multiplier Schedule = Yes
            if (s_ipsc->cAlphaArgs(4) == "ALWAYSONATMAXIMUMFLOW" && s_ipsc->cAlphaArgs(5) == "YES") {
                ShowWarningError(state,
                                 format("{}=\"{}has {}=\"{}\"",
                                        s_ipsc->cCurrentModuleObject,
                                        s_ipsc->cAlphaArgs(1),
                                        s_ipsc->cAlphaFieldNames(4),
                                        s_ipsc->cAlphaArgs(4)));
                ShowContinueError(state,
                                  format("..but {}=\"{}If specified, the {} will be ignored.",
                                         s_ipsc->cAlphaFieldNames(5),
                                         s_ipsc->cAlphaArgs(5),
                                         s_ipsc->cAlphaFieldNames(5)));
            }

            // Warning if Airflow Control Type = AlwaysOff and Airflow Has Multiplier Schedule = Yes
            if (s_ipsc->cAlphaArgs(4) == "ALWAYSOFF" && s_ipsc->cAlphaArgs(5) == "YES") {
                ShowWarningError(state,
                                 format("{}=\"{}has {}=\"{}\"",
                                        s_ipsc->cCurrentModuleObject,
                                        s_ipsc->cAlphaArgs(1),
                                        s_ipsc->cAlphaFieldNames(4),
                                        s_ipsc->cAlphaArgs(4)));
                ShowContinueError(state,
                                  format("..but {}=\"{}\". If specified, the {} will be ignored.",
                                         s_ipsc->cAlphaFieldNames(5),
                                         s_ipsc->cAlphaArgs(5),
                                         s_ipsc->cAlphaFieldNames(5)));
            }

            if (SurfNum > 0) {
                auto const &surf = state.dataSurface->Surface(SurfNum);
                state.dataSurface->AirflowWindows = true;
                state.dataSurface->SurfWinAirflowSource(SurfNum) =
                    static_cast<DataSurfaces::WindowAirFlowSource>(getEnumValue(WindowAirFlowSourceNamesUC, s_ipsc->cAlphaArgs(2)));

                state.dataSurface->SurfWinAirflowDestination(SurfNum) =
                    static_cast<DataSurfaces::WindowAirFlowDestination>(getEnumValue(WindowAirFlowDestinationNamesUC, s_ipsc->cAlphaArgs(3)));

                if (state.dataSurface->SurfWinAirflowDestination(SurfNum) == DataSurfaces::WindowAirFlowDestination::Return) {
                    int controlledZoneNum = DataZoneEquipment::GetControlledZoneIndex(state, surf.ZoneName);
                    if (controlledZoneNum > 0) {
                        state.dataHeatBal->Zone(surf.Zone).HasAirFlowWindowReturn = true;
                    }

                    // Set return air node number
                    state.dataSurface->SurfWinAirflowReturnNodePtr(SurfNum) = 0;
                    std::string retNodeName = "";
                    if (!s_ipsc->lAlphaFieldBlanks(7)) {
                        retNodeName = s_ipsc->cAlphaArgs(7);
                    }
                    std::string callDescription = s_ipsc->cCurrentModuleObject + "=" + surf.Name;
                    state.dataSurface->SurfWinAirflowReturnNodePtr(SurfNum) =
                        DataZoneEquipment::GetReturnAirNodeForZone(state, surf.Zone, retNodeName, callDescription);
                    if (state.dataSurface->SurfWinAirflowReturnNodePtr(SurfNum) == 0) {
                        ShowSevereError(state,
                                        format("{}{}=\"{}\", airflow window return air node not found for {} = {}",
                                               routineName,
                                               s_ipsc->cCurrentModuleObject,
                                               surf.Name,
                                               s_ipsc->cAlphaFieldNames(3),
                                               s_ipsc->cAlphaArgs(3)));
                        if (!s_ipsc->lAlphaFieldBlanks(7)) {
                            ShowContinueError(
                                state,
                                format("{}=\"{}\" did not find a matching return air node.", s_ipsc->cAlphaFieldNames(7), s_ipsc->cAlphaArgs(7)));
                        }
                        ShowContinueError(state,
                                          "..Airflow windows with Airflow Destination = ReturnAir must reference a controlled Zone (appear in a "
                                          "ZoneHVAC:EquipmentConnections object) with at least one return air node.");
                        ErrorsFound = true;
                    }
                }
                if (Util::SameString(s_ipsc->cAlphaArgs(4), "AlwaysOnAtMaximumFlow")) {
                    state.dataSurface->SurfWinAirflowControlType(SurfNum) = DataSurfaces::WindowAirFlowControlType::MaxFlow;
                } else if (Util::SameString(s_ipsc->cAlphaArgs(4), "AlwaysOff")) {
                    state.dataSurface->SurfWinAirflowControlType(SurfNum) = DataSurfaces::WindowAirFlowControlType::AlwaysOff;
                } else if (Util::SameString(s_ipsc->cAlphaArgs(4), "ScheduledOnly")) {
                    state.dataSurface->SurfWinAirflowControlType(SurfNum) = DataSurfaces::WindowAirFlowControlType::Schedule;
                }
                state.dataSurface->SurfWinMaxAirflow(SurfNum) = s_ipsc->rNumericArgs(1);
                if (s_ipsc->cAlphaArgs(4) == "SCHEDULEDONLY" && s_ipsc->cAlphaArgs(5) == "YES") {
                    if (s_ipsc->lAlphaFieldBlanks(6)) {
                        ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
                        ErrorsFound = true;
                    } else if ((state.dataSurface->SurfWinAirflowScheds(SurfNum) = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(6))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6));
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", has {}=\"{}\"",
                                               s_ipsc->cCurrentModuleObject,
                                               s_ipsc->cAlphaArgs(1),
                                               s_ipsc->cAlphaFieldNames(4),
                                               s_ipsc->cAlphaArgs(4)));
                        ShowContinueError(state,
                                          format("..and {}=\"{}\", but no {} specified.",
                                                 s_ipsc->cAlphaFieldNames(5),
                                                 s_ipsc->cAlphaArgs(5),
                                                 s_ipsc->cAlphaFieldNames(6)));
                    } else {
                        state.dataSurface->SurfWinAirflowHasSchedule(SurfNum) = true;
                        if ((state.dataSurface->SurfWinAirflowScheds(SurfNum) = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(6))) == nullptr) {
                            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6));
                            ErrorsFound = true;
                        }
                    }
                }
                // Warning if associated window is an interior window
                if (surf.ExtBoundCond != DataSurfaces::ExternalEnvironment && !ErrorsFound) {
                    ShowWarningError(state,
                                     format("{}=\"{}\", is an Interior window; cannot be an airflow window.",
                                            s_ipsc->cCurrentModuleObject,
                                            s_ipsc->cAlphaArgs(1)));
                }
                if (!ErrorsFound) {
                    // Require that gas in airflow gap has type = air
                    MatGapFlow = state.dataConstruction->Construct(ConstrNum).LayerPoint(2);
                    if (state.dataConstruction->Construct(ConstrNum).TotGlassLayers == 3) {
                        MatGapFlow = state.dataConstruction->Construct(ConstrNum).LayerPoint(4);
                    }
                    if (dynamic_cast<Material::MaterialGasMix const *>(s_mat->materials(MatGapFlow))->gases[0].type != Material::GasType::Air) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", Gas type not air in airflow gap of construction {}",
                                               s_ipsc->cCurrentModuleObject,
                                               s_ipsc->cAlphaArgs(1),
                                               state.dataConstruction->Construct(ConstrNum).Name));
                    }
                    // Require that gas be air in airflow gaps on either side of a between glass shade/blind
                    if (surf.HasShadeControl) {
                        for (std::size_t listIndex = 0; listIndex < surf.windowShadingControlList.size(); ++listIndex) {
                            int WSCPtr = surf.windowShadingControlList[listIndex];
                            if (ANY_BETWEENGLASS_SHADE_BLIND(state.dataSurface->WindowShadingControl(WSCPtr).ShadingType)) {
                                ConstrNumSh = surf.shadedConstructionList[listIndex];
                                if (state.dataConstruction->Construct(ConstrNum).TotGlassLayers == 2) {
                                    MatGapFlow1 = state.dataConstruction->Construct(ConstrNumSh).LayerPoint(2);
                                    MatGapFlow2 = state.dataConstruction->Construct(ConstrNumSh).LayerPoint(4);
                                } else {
                                    MatGapFlow1 = state.dataConstruction->Construct(ConstrNumSh).LayerPoint(4);
                                    MatGapFlow2 = state.dataConstruction->Construct(ConstrNumSh).LayerPoint(6);
                                }
                                if (dynamic_cast<Material::MaterialGasMix const *>(s_mat->materials(MatGapFlow1))->gases[0].type !=
                                        Material::GasType::Air ||
                                    dynamic_cast<Material::MaterialGasMix const *>(s_mat->materials(MatGapFlow2))->gases[0].type !=
                                        Material::GasType::Air) {
                                    ErrorsFound = true;
                                    ShowSevereError(state,
                                                    format("{}=\"{}\", gas type must be air on either side of the shade/blind",
                                                           s_ipsc->cCurrentModuleObject,
                                                           s_ipsc->cAlphaArgs(1)));
                                }
                                break; // only need the first window shading control since they should be the same
                            }
                        }
                    }
                }
            }

        } // End of loop over window airflow controls
    }

    void GetFoundationData(EnergyPlusData &state, bool &ErrorsFound)
    {

        int NumAlphas;
        int NumProps;
        int IOStat;

        static constexpr std::string_view routineName = "GetFoundationData";

        auto &s_ipsc = state.dataIPShortCut;

        // Read Kiva Settings
        s_ipsc->cCurrentModuleObject = "Foundation:Kiva:Settings";
        int TotKivaStgs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        if (TotKivaStgs > 1) {
            ErrorsFound = true;
            ShowSevereError(state, format("Multiple {} objects found. Only one is allowed.", s_ipsc->cCurrentModuleObject));
        }

        if (TotKivaStgs == 1) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     1,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumProps,
                                                                     IOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            int numF = 1;
            int alpF = 1;

            if (!s_ipsc->lNumericFieldBlanks(numF)) {
                state.dataSurfaceGeometry->kivaManager.settings.soilK = s_ipsc->rNumericArgs(numF);
            }
            numF++;
            if (!s_ipsc->lNumericFieldBlanks(numF)) {
                state.dataSurfaceGeometry->kivaManager.settings.soilRho = s_ipsc->rNumericArgs(numF);
            }
            numF++;
            if (!s_ipsc->lNumericFieldBlanks(numF)) {
                state.dataSurfaceGeometry->kivaManager.settings.soilCp = s_ipsc->rNumericArgs(numF);
            }
            numF++;
            if (!s_ipsc->lNumericFieldBlanks(numF)) {
                state.dataSurfaceGeometry->kivaManager.settings.groundSolarAbs = s_ipsc->rNumericArgs(numF);
            }
            numF++;
            if (!s_ipsc->lNumericFieldBlanks(numF)) {
                state.dataSurfaceGeometry->kivaManager.settings.groundThermalAbs = s_ipsc->rNumericArgs(numF);
            }
            numF++;
            if (!s_ipsc->lNumericFieldBlanks(numF)) {
                state.dataSurfaceGeometry->kivaManager.settings.groundRoughness = s_ipsc->rNumericArgs(numF);
            }
            numF++;
            if (!s_ipsc->lNumericFieldBlanks(numF)) {
                state.dataSurfaceGeometry->kivaManager.settings.farFieldWidth = s_ipsc->rNumericArgs(numF);
            }
            numF++;

            if (!s_ipsc->lAlphaFieldBlanks(alpF)) {
                if (Util::SameString(s_ipsc->cAlphaArgs(alpF), "ZeroFlux")) {
                    state.dataSurfaceGeometry->kivaManager.settings.deepGroundBoundary = HeatBalanceKivaManager::KivaManager::Settings::ZERO_FLUX;
                } else if (Util::SameString(s_ipsc->cAlphaArgs(alpF), "GroundWater")) {
                    state.dataSurfaceGeometry->kivaManager.settings.deepGroundBoundary = HeatBalanceKivaManager::KivaManager::Settings::GROUNDWATER;
                } else if (Util::SameString(s_ipsc->cAlphaArgs(alpF), "Autoselect")) {
                    state.dataSurfaceGeometry->kivaManager.settings.deepGroundBoundary = HeatBalanceKivaManager::KivaManager::Settings::AUTO;
                } else {
                    ErrorsFound = true;
                    ShowSevereError(state,
                                    format("{}, {} is not a valid choice for {}",
                                           s_ipsc->cCurrentModuleObject,
                                           s_ipsc->cAlphaArgs(alpF),
                                           s_ipsc->cAlphaFieldNames(alpF)));
                }
            }
            alpF++;

            if (s_ipsc->lNumericFieldBlanks(numF) || s_ipsc->rNumericArgs(numF) == Constant::AutoCalculate) {
                // Autocalculate deep-ground depth (see KivaManager::defineDefaultFoundation() for actual calculation)
                state.dataSurfaceGeometry->kivaManager.settings.deepGroundDepth = 40.0;
                state.dataSurfaceGeometry->kivaManager.settings.autocalculateDeepGroundDepth = true;
                if (state.dataSurfaceGeometry->kivaManager.settings.deepGroundBoundary != HeatBalanceKivaManager::KivaManager::Settings::AUTO) {
                    ErrorsFound = true;
                    ShowSevereError(state,
                                    format("{}, {} should not be set to Autocalculate unless {} is set to Autoselect",
                                           s_ipsc->cCurrentModuleObject,
                                           s_ipsc->cNumericFieldNames(numF),
                                           s_ipsc->cAlphaFieldNames(alpF - 1)));
                }
            } else {
                state.dataSurfaceGeometry->kivaManager.settings.deepGroundDepth = s_ipsc->rNumericArgs(numF);
                state.dataSurfaceGeometry->kivaManager.settings.autocalculateDeepGroundDepth = false;
            }
            numF++;
            if (!s_ipsc->lNumericFieldBlanks(numF)) {
                state.dataSurfaceGeometry->kivaManager.settings.minCellDim = s_ipsc->rNumericArgs(numF);
            }
            numF++;
            if (!s_ipsc->lNumericFieldBlanks(numF)) {
                state.dataSurfaceGeometry->kivaManager.settings.maxGrowthCoeff = s_ipsc->rNumericArgs(numF);
            }
            numF++;

            if (!s_ipsc->lAlphaFieldBlanks(alpF)) {
                if (Util::SameString(s_ipsc->cAlphaArgs(alpF), "Hourly")) {
                    state.dataSurfaceGeometry->kivaManager.settings.timestepType = HeatBalanceKivaManager::KivaManager::Settings::HOURLY;
                    state.dataSurfaceGeometry->kivaManager.timestep = 3600.; // seconds
                } else {                                                     // if (Util::SameString(s_ipsc->cAlphaArgs( alpF ), "Timestep"))
                    state.dataSurfaceGeometry->kivaManager.settings.timestepType = HeatBalanceKivaManager::KivaManager::Settings::TIMESTEP;
                    state.dataSurfaceGeometry->kivaManager.timestep = state.dataGlobal->MinutesInTimeStep * 60.;
                }
            }
            alpF++;
        }

        // Set default foundation (probably doesn't need to be called if there are no Kiva
        // surfaces, but we don't know that yet). We call this here so that the default
        // foundation is available for 1) the starting copy for user-defined Foundation:Kiva
        // object default inputs, and 2) the actual default Foundation object if a
        // user-defined Foundation:Kiva name is not referenced by a surface.
        state.dataSurfaceGeometry->kivaManager.defineDefaultFoundation(state);

        // Read Foundation objects
        s_ipsc->cCurrentModuleObject = "Foundation:Kiva";
        int TotKivaFnds = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        if (TotKivaFnds > 0) {
            auto &s_mat = state.dataMaterial;
            for (int Loop = 1; Loop <= TotKivaFnds; ++Loop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         s_ipsc->cCurrentModuleObject,
                                                                         Loop,
                                                                         s_ipsc->cAlphaArgs,
                                                                         NumAlphas,
                                                                         s_ipsc->rNumericArgs,
                                                                         NumProps,
                                                                         IOStat,
                                                                         s_ipsc->lNumericFieldBlanks,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

                int numF = 1;
                int alpF = 1;

                bool ErrorInName = false;

                HeatBalanceKivaManager::FoundationKiva fndInput;

                fndInput.name = s_ipsc->cAlphaArgs(alpF);
                alpF++;
                Util::IsNameEmpty(state, fndInput.name, s_ipsc->cCurrentModuleObject, ErrorInName);
                if (ErrorInName) {
                    ErrorsFound = true;
                    continue;
                }

                // Start with copy of default
                auto &fnd = fndInput.foundation;
                fnd = state.dataSurfaceGeometry->kivaManager.defaultFoundation.foundation;

                // Indoor temperature
                if (!s_ipsc->lNumericFieldBlanks(numF)) {
                    fndInput.assumedIndoorTemperature = s_ipsc->rNumericArgs(numF);
                } else {
                    fndInput.assumedIndoorTemperature = -9999;
                }
                numF++;

                // Interior horizontal insulation
                if (!s_ipsc->lAlphaFieldBlanks(alpF)) {
                    int index = Material::GetMaterialNum(state, s_ipsc->cAlphaArgs(alpF));
                    if (index == 0) {
                        ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(alpF), s_ipsc->cAlphaArgs(alpF));
                        ErrorsFound = true;
                        continue;
                    }
                    auto *m = s_mat->materials(index);
                    if (m->group != Material::Group::Regular || m->ROnly) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF),
                                               s_ipsc->cAlphaArgs(alpF)));
                        ShowContinueError(state, "Must be of type \"Material\"");
                        continue;
                    }
                    fndInput.intHIns.x = 0.0;
                    fndInput.intHIns.material = Kiva::Material(m->Conductivity, m->Density, m->SpecHeat);
                    fndInput.intHIns.depth = m->Thickness;
                }
                alpF++;

                if (!s_ipsc->lAlphaFieldBlanks(alpF - 1)) {
                    if (s_ipsc->lNumericFieldBlanks(numF)) {
                        fndInput.intHIns.z = 0.0;
                    } else {
                        fndInput.intHIns.z = s_ipsc->rNumericArgs(numF);
                    }
                    numF++;
                    if (s_ipsc->lNumericFieldBlanks(numF)) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", {} defined, but no {}provided",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF - 1),
                                               s_ipsc->cNumericFieldNames(numF)));
                        continue;
                    } else {
                        fndInput.intHIns.width = -s_ipsc->rNumericArgs(numF);
                    }
                    numF++;
                } else {
                    if (!s_ipsc->lNumericFieldBlanks(numF)) {
                        ShowWarningError(
                            state,
                            format("{}=\"{}\", no {} defined", s_ipsc->cCurrentModuleObject, fndInput.name, s_ipsc->cAlphaFieldNames(alpF - 1)));
                        ShowContinueError(state, format("{} will not be used.", s_ipsc->cNumericFieldNames(numF)));
                    }
                    numF++;
                    if (!s_ipsc->lNumericFieldBlanks(numF)) {
                        ShowWarningError(
                            state,
                            format("{}=\"{}\", no {} defined", s_ipsc->cCurrentModuleObject, fndInput.name, s_ipsc->cAlphaFieldNames(alpF - 1)));
                        ShowContinueError(state, format("{} will not be used.", s_ipsc->cNumericFieldNames(numF)));
                    }
                    numF++;
                }

                // Interior vertical insulation
                if (!s_ipsc->lAlphaFieldBlanks(alpF)) {
                    int index = Material::GetMaterialNum(state, s_ipsc->cAlphaArgs(alpF));
                    if (index == 0) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("Did not find matching material for {}=\"{}\", {}, missing material = {}",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF),
                                               s_ipsc->cAlphaArgs(alpF)));
                        continue;
                    }
                    auto *m = s_mat->materials(index);
                    if (m->group != Material::Group::Regular || m->ROnly) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF),
                                               s_ipsc->cAlphaArgs(alpF)));
                        ShowContinueError(state, "Must be of type \"Material\"");
                        continue;
                    }
                    fndInput.intVIns.material = Kiva::Material(m->Conductivity, m->Density, m->SpecHeat);
                    fndInput.intVIns.width = -m->Thickness;
                    fndInput.intVIns.x = 0.0;
                    fndInput.intVIns.z = 0.0;
                }
                alpF++;

                if (!s_ipsc->lAlphaFieldBlanks(alpF - 1)) {
                    if (s_ipsc->lNumericFieldBlanks(numF)) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", {} defined, but no {}provided",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF - 1),
                                               s_ipsc->cNumericFieldNames(numF)));
                        continue;
                    } else {
                        fndInput.intVIns.depth = s_ipsc->rNumericArgs(numF);
                    }
                    numF++;
                } else {
                    if (!s_ipsc->lNumericFieldBlanks(numF)) {
                        ShowWarningError(
                            state,
                            format("{}=\"{}\", no {} defined", s_ipsc->cCurrentModuleObject, fndInput.name, s_ipsc->cAlphaFieldNames(alpF - 1)));
                        ShowContinueError(state, format("{} will not be used.", s_ipsc->cNumericFieldNames(numF)));
                    }
                    numF++;
                }

                // Exterior horizontal insulation
                if (!s_ipsc->lAlphaFieldBlanks(alpF)) {
                    int index = Material::GetMaterialNum(state, s_ipsc->cAlphaArgs(alpF));
                    if (index == 0) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("Did not find matching material for {}=\"{}\", {}, missing material = {}",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF),
                                               s_ipsc->cAlphaArgs(alpF)));
                        continue;
                    }
                    auto *m = s_mat->materials(index);
                    if (m->group != Material::Group::Regular || m->ROnly) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF),
                                               s_ipsc->cAlphaArgs(alpF)));
                        ShowContinueError(state, "Must be of type \"Material\"");
                        continue;
                    }
                    fndInput.extHIns.x = 0.0;
                    fndInput.extHIns.material = Kiva::Material(m->Conductivity, m->Density, m->SpecHeat);
                    fndInput.extHIns.depth = m->Thickness;
                }
                alpF++;

                if (!s_ipsc->lAlphaFieldBlanks(alpF - 1)) {
                    if (s_ipsc->lNumericFieldBlanks(numF)) {
                        fndInput.extHIns.z = 0.0;
                    } else {
                        fndInput.extHIns.z = s_ipsc->rNumericArgs(numF);
                    }
                    numF++;
                    if (s_ipsc->lNumericFieldBlanks(numF)) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", {} defined, but no {}provided",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF - 1),
                                               s_ipsc->cNumericFieldNames(numF)));
                        continue;
                    } else {
                        fndInput.extHIns.width = s_ipsc->rNumericArgs(numF);
                    }
                    numF++;
                } else {
                    if (!s_ipsc->lNumericFieldBlanks(numF)) {
                        ShowWarningError(
                            state,
                            format("{}=\"{}\", no {} defined", s_ipsc->cCurrentModuleObject, fndInput.name, s_ipsc->cAlphaFieldNames(alpF - 1)));
                        ShowContinueError(state, format("{} will not be used.", s_ipsc->cNumericFieldNames(numF)));
                    }
                    numF++;
                    if (!s_ipsc->lNumericFieldBlanks(numF)) {
                        ShowWarningError(
                            state,
                            format("{}=\"{}\", no {} defined", s_ipsc->cCurrentModuleObject, fndInput.name, s_ipsc->cAlphaFieldNames(alpF - 1)));
                        ShowContinueError(state, format("{} will not be used.", s_ipsc->cNumericFieldNames(numF)));
                    }
                    numF++;
                }

                // Exterior vertical insulation
                if (!s_ipsc->lAlphaFieldBlanks(alpF)) {
                    int index = Material::GetMaterialNum(state, s_ipsc->cAlphaArgs(alpF));
                    if (index == 0) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("Did not find matching material for {}=\"{}\", {}, missing material = {}",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF),
                                               s_ipsc->cAlphaArgs(alpF)));
                        continue;
                    }
                    auto *m = s_mat->materials(index);
                    if (m->group != Material::Group::Regular || m->ROnly) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF),
                                               s_ipsc->cAlphaArgs(alpF)));
                        ShowContinueError(state, "Must be of type \"Material\"");
                        continue;
                    }
                    fndInput.extVIns.material = Kiva::Material(m->Conductivity, m->Density, m->SpecHeat);
                    fndInput.extVIns.width = m->Thickness;
                    fndInput.extVIns.x = 0.0;
                    fndInput.extVIns.z = 0.0;
                }
                alpF++;

                if (!s_ipsc->lAlphaFieldBlanks(alpF - 1)) {
                    if (s_ipsc->lNumericFieldBlanks(numF)) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", {} defined, but no {}provided",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF - 1),
                                               s_ipsc->cNumericFieldNames(numF)));
                        continue;
                    } else {
                        fndInput.extVIns.depth = s_ipsc->rNumericArgs(numF);
                    }
                    numF++;
                } else {
                    if (!s_ipsc->lNumericFieldBlanks(numF)) {
                        ShowWarningError(
                            state,
                            format("{}=\"{}\", no {} defined", s_ipsc->cCurrentModuleObject, fndInput.name, s_ipsc->cAlphaFieldNames(alpF - 1)));
                        ShowContinueError(state, format("{} will not be used.", s_ipsc->cNumericFieldNames(numF)));
                    }
                    numF++;
                }

                // Foundation wall
                if (!s_ipsc->lNumericFieldBlanks(numF)) {
                    fnd.wall.heightAboveGrade = s_ipsc->rNumericArgs(numF);
                }
                numF++;

                if (!s_ipsc->lNumericFieldBlanks(numF)) {
                    fnd.wall.depthBelowSlab = s_ipsc->rNumericArgs(numF);
                }
                numF++;

                if (!s_ipsc->lAlphaFieldBlanks(alpF)) {
                    fndInput.wallConstructionIndex = Util::FindItemInList(s_ipsc->cAlphaArgs(alpF), state.dataConstruction->Construct);
                    if (fndInput.wallConstructionIndex == 0) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("Did not find matching construction for {}=\"{}\", {}, missing construction = {}",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF),
                                               s_ipsc->cAlphaArgs(alpF)));
                        continue;
                    }
                    auto &c = state.dataConstruction->Construct(fndInput.wallConstructionIndex);
                    c.IsUsed = true;
                    if (c.TypeIsWindow) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF),
                                               s_ipsc->cAlphaArgs(alpF)));
                        ShowContinueError(state, "Cannot be a window construction");
                        continue;
                    }
                } else {
                    fndInput.wallConstructionIndex = 0; // Use default wall construction
                }
                alpF++;

                // Footing
                if (!s_ipsc->lAlphaFieldBlanks(alpF)) {
                    int index = Material::GetMaterialNum(state, s_ipsc->cAlphaArgs(alpF));
                    if (index == 0) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("Did not find matching material for {}=\"{}\", {}, missing material = {}",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF),
                                               s_ipsc->cAlphaArgs(alpF)));
                        continue;
                    }
                    auto *m = s_mat->materials(index);
                    if (m->group != Material::Group::Regular || m->ROnly) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", invalid {}=\"{}",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF),
                                               s_ipsc->cAlphaArgs(alpF)));
                        ShowContinueError(state, "Must be of type \"Material\"");
                        continue;
                    }
                    fndInput.footing.material = Kiva::Material(m->Conductivity, m->Density, m->SpecHeat);
                    fndInput.footing.width = m->Thickness;
                    fndInput.footing.x = 0.0;
                    fndInput.footing.z = 0.0;
                }
                alpF++;

                if (!s_ipsc->lAlphaFieldBlanks(alpF - 1)) {
                    if (s_ipsc->lNumericFieldBlanks(numF)) {
                        ErrorsFound = true;
                        ShowSevereError(state,
                                        format("{}=\"{}\", {} defined, but no {}provided",
                                               s_ipsc->cCurrentModuleObject,
                                               fndInput.name,
                                               s_ipsc->cAlphaFieldNames(alpF - 1),
                                               s_ipsc->cNumericFieldNames(numF)));
                        continue;
                    } else {
                        fndInput.footing.depth = s_ipsc->rNumericArgs(numF);
                    }
                    numF++;
                } else {
                    if (!s_ipsc->lNumericFieldBlanks(numF)) {
                        ShowWarningError(
                            state,
                            format("{}=\"{}\", no {} defined", s_ipsc->cCurrentModuleObject, fndInput.name, s_ipsc->cAlphaFieldNames(alpF - 1)));
                        ShowContinueError(state, format("{} will not be used.", s_ipsc->cNumericFieldNames(numF)));
                    }
                    numF++;
                }

                // General Blocks
                int numRemainingFields = NumAlphas - (alpF - 1) + NumProps - (numF - 1);
                if (numRemainingFields > 0) {
                    int numBlocks = numRemainingFields / 4;
                    if (mod(numRemainingFields, 4) != 0) {
                        ShowWarningError(state,
                                         format("{}=\"{}\", number of Block fields not even multiple of 4. Will read in {}",
                                                s_ipsc->cCurrentModuleObject,
                                                fndInput.name,
                                                numBlocks));
                    }
                    for (int blockNum = 0; blockNum < numBlocks; blockNum++) {
                        Kiva::InputBlock block;
                        if (!s_ipsc->lAlphaFieldBlanks(alpF)) {
                            int index = Material::GetMaterialNum(state, s_ipsc->cAlphaArgs(alpF));
                            if (index == 0) {
                                ErrorsFound = true;
                                ShowSevereError(state,
                                                format("Did not find matching material for {}=\"{}\", {}, missing material = {}",
                                                       s_ipsc->cCurrentModuleObject,
                                                       fndInput.name,
                                                       s_ipsc->cAlphaFieldNames(alpF),
                                                       s_ipsc->cAlphaArgs(alpF)));
                                continue;
                            }
                            auto *m = s_mat->materials(index);
                            if (m->group != Material::Group::Regular || m->ROnly) {
                                ErrorsFound = true;
                                ShowSevereError(state,
                                                format("{}=\"{}\", invalid {}=\"{}",
                                                       s_ipsc->cCurrentModuleObject,
                                                       fndInput.name,
                                                       s_ipsc->cAlphaFieldNames(alpF),
                                                       s_ipsc->cAlphaArgs(alpF)));
                                ShowContinueError(state, "Must be of type \"Material\"");
                                continue;
                            }
                            block.material = Kiva::Material(m->Conductivity, m->Density, m->SpecHeat);
                            block.width = m->Thickness;
                        } else {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format("{}=\"{}\", {} is required and not given.",
                                                   s_ipsc->cCurrentModuleObject,
                                                   fndInput.name,
                                                   s_ipsc->cAlphaFieldNames(alpF)));
                            continue;
                        }
                        alpF++;

                        if (s_ipsc->lNumericFieldBlanks(numF)) {
                            block.depth = 0.0; // Temporary indicator to default to foundation depth
                        } else {
                            block.depth = s_ipsc->rNumericArgs(numF);
                        }
                        numF++;

                        if (s_ipsc->lNumericFieldBlanks(numF)) {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format("{}=\"{}\", {} defined, but no {}provided",
                                                   s_ipsc->cCurrentModuleObject,
                                                   fndInput.name,
                                                   s_ipsc->cAlphaFieldNames(alpF - 1),
                                                   s_ipsc->cNumericFieldNames(numF)));
                            continue;
                        } else {
                            block.x = s_ipsc->rNumericArgs(numF);
                        }
                        numF++;

                        if (s_ipsc->lNumericFieldBlanks(numF)) {
                            block.z = 0.0;
                        } else {
                            block.z = s_ipsc->rNumericArgs(numF);
                        }
                        numF++;

                        fnd.inputBlocks.push_back(block);
                    }
                }

                state.dataSurfaceGeometry->kivaManager.foundationInputs.push_back(fndInput);
            }
        }
    }

    void GetOSCData(EnergyPlusData &state, bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Jul 2011, M.J. Witte and C.O. Pedersen, add new fields to OSC for last T, max and min

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the OtherSideCoefficient data.

        // REFERENCES:
        // Other Side Coefficient Definition
        // OtherSideCoefficients,
        //       \memo This object sets the other side conditions for a surface in a variety of ways.
        //   A1, \field OtherSideCoeff Name
        //       \required-field
        //       \reference OSCNames
        //       \reference OutFaceEnvNames
        //   N1, \field Combined convective/radiative film coefficient
        //       \required-field
        //       \type real
        //       \note if>0, N1 becomes exterior convective/radiative film coefficient and other fields
        //       \note are used to calc outside air temp then exterior surface temp based on outside air
        //       \note and specified coefficient
        //       \note if<=0, then remaining fields calculate the outside surface temperature(?)
        //       \note following fields are used in the equation:
        //       \note SurfTemp=N7*TempZone + N4*OutsideDryBulb + N2*N3 + GroundTemp*N5 + WindSpeed*N6*OutsideDryBulb
        //   N2, \field User selected Constant Temperature
        //       \units C
        //       \type real
        //       \note This parameter will be overwritten by the values from the schedule(A2 below) if one is present
        //   N3, \field Coefficient modifying the user selected constant temperature
        //       \note This coefficient is used even with a schedule.  It should normally be 1.0 in that case
        //   N4, \field Coefficient modifying the external dry bulb temperature
        //       \type real
        //   N5, \field Coefficient modifying the ground temperature
        //       \type real
        //   N6, \field Coefficient modifying the wind speed term (s/m)
        //       \type real
        //   N7, \field Coefficient modifying the zone air temperature part of the equation
        //       \type real
        //   A2, \field ScheduleName for constant temperature
        //       \note Name of Schedule for values of "const" temperature.
        //       \note Schedule values replace N2 - User selected constant temperature.
        //       \type object-list
        //       \object-list ScheduleNames
        //   A3, \field Sinusoidal Variation of Constant Temperature Coefficient
        //       \note Optionally used to vary Constant Temperature Coefficient with unitary sine wave
        //       \type choice
        //       \key Yes
        //       \key No
        //       \default No
        //   N8; \field Period of Sinusoidal Variation
        //       \note Use with sinusoidal variation to define the time period
        //       \type real
        //       \units hr
        //       \default 24
        //  N9, \field Previous Other Side Temperature Coefficient
        //      \note This coefficient multiplies the other side temperature result from the
        //      \note previous zone timestep
        //      \type real
        //      \default 0
        // N10, \field Minimum Other Side Temperature
        //      \type real
        //      \units C
        //      \default -100
        // N11; \field Maximum Other Side Temperature
        //      \type real
        //      \units C
        //      \default 200

        static constexpr std::string_view routineName = "GetOSCData";
        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;
        int NumProps;
        int Loop;
        int IOStat;
        std::string cOSCLimitsString;

        auto &s_ipsc = state.dataIPShortCut;

        s_ipsc->cCurrentModuleObject = "SurfaceProperty:OtherSideCoefficients";
        state.dataSurface->TotOSC = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        state.dataSurface->OSC.allocate(state.dataSurface->TotOSC);

        int OSCNum = 0;
        for (Loop = 1; Loop <= state.dataSurface->TotOSC; ++Loop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumProps,
                                                                     IOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};
            bool ErrorInName = false;
            bool IsBlank = false;
            Util::VerifyName(
                state, s_ipsc->cAlphaArgs(1), state.dataSurface->OSC, OSCNum, ErrorInName, IsBlank, s_ipsc->cCurrentModuleObject + " Name");
            if (ErrorInName) {
                ErrorsFound = true;
                continue;
            }

            ++OSCNum;
            state.dataSurface->OSC(OSCNum).Name = s_ipsc->cAlphaArgs(1);
            state.dataSurface->OSC(OSCNum).SurfFilmCoef = s_ipsc->rNumericArgs(1);
            state.dataSurface->OSC(OSCNum).ConstTemp = s_ipsc->rNumericArgs(2); //  This will be replaced if  schedule is used
            state.dataSurface->OSC(OSCNum).ConstTempCoef =
                s_ipsc->rNumericArgs(3); //  This multiplier is used (even with schedule).  It should normally be 1.0
            state.dataSurface->OSC(OSCNum).ExtDryBulbCoef = s_ipsc->rNumericArgs(4);
            state.dataSurface->OSC(OSCNum).GroundTempCoef = s_ipsc->rNumericArgs(5);
            state.dataSurface->OSC(OSCNum).WindSpeedCoef = s_ipsc->rNumericArgs(6);
            state.dataSurface->OSC(OSCNum).ZoneAirTempCoef = s_ipsc->rNumericArgs(7);
            state.dataSurface->OSC(OSCNum).SinusoidPeriod = s_ipsc->rNumericArgs(8);

            if ((NumAlphas == 1) || s_ipsc->lAlphaFieldBlanks(2)) { //  Const temp will come from schedule specified below.
            } else if ((state.dataSurface->OSC(OSCNum).constTempSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                ErrorsFound = true;
            }

            if (!s_ipsc->lAlphaFieldBlanks(3)) {
                if (BooleanSwitch bs = getYesNoValue(s_ipsc->cAlphaArgs(3)); bs != BooleanSwitch::Invalid) {
                    state.dataSurface->OSC(OSCNum).SinusoidalConstTempCoef = static_cast<bool>(bs);
                } else {
                    ShowSevereInvalidBool(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                    ErrorsFound = true;
                }
            }

            if (s_ipsc->rNumericArgs(1) > 0.0 && !any_ne(s_ipsc->rNumericArgs({3, 7}), 0.0) &&
                (!state.dataSurface->OSC(OSCNum).SinusoidalConstTempCoef)) {
                ShowSevereError(state, format("{}=\"{}\" has zeros for all coefficients.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "...The outdoor air temperature for surfaces using this OtherSideCoefficients object will always be 0C.");
            }

            if (s_ipsc->rNumericArgs(1) <= 0.0 && !any_ne(s_ipsc->rNumericArgs({3, 7}), 0.0) &&
                (!state.dataSurface->OSC(OSCNum).SinusoidalConstTempCoef)) {
                ShowSevereError(state, format("{}=\"{}\" has zeros for all coefficients.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state,
                                  "...The outside surface temperature for surfaces using this OtherSideCoefficients object will always be 0C.");
            }

            state.dataSurface->OSC(OSCNum).TPreviousCoef = s_ipsc->rNumericArgs(9);

            if (!s_ipsc->lNumericFieldBlanks(10)) {
                state.dataSurface->OSC(OSCNum).MinLimitPresent = true;
                state.dataSurface->OSC(OSCNum).MinTempLimit = s_ipsc->rNumericArgs(10);
                cOSCLimitsString = format("{:.3R}", s_ipsc->rNumericArgs(10));
            } else {
                cOSCLimitsString = "N/A";
            }
            if (!s_ipsc->lNumericFieldBlanks(11)) {
                state.dataSurface->OSC(OSCNum).MaxLimitPresent = true;
                state.dataSurface->OSC(OSCNum).MaxTempLimit = s_ipsc->rNumericArgs(11);
                cOSCLimitsString += format(",{:.3R}", s_ipsc->rNumericArgs(10));
            } else {
                cOSCLimitsString += ",N/A";
            }
        }

        for (Loop = 1; Loop <= state.dataSurface->TotOSC; ++Loop) {
            if (Loop == 1) {
                static constexpr std::string_view OSCFormat1(
                    "! <Other Side Coefficients>,Name,Combined convective/radiative film coefficient {W/m2-K},User selected "
                    "Constant Temperature {C},Coefficient modifying the constant temperature term,Coefficient modifying the external "
                    "dry bulb temperature term,Coefficient modifying the ground temperature term,Coefficient modifying the wind speed "
                    "term {s/m},Coefficient modifying the zone air temperature term,Constant Temperature Schedule Name,Sinusoidal "
                    "Variation,Period of Sinusoidal Variation,Previous Other Side Temperature Coefficient,Minimum Other Side "
                    "Temperature {C},Maximum Other Side Temperature {C}");
                print(state.files.eio, "{}\n", OSCFormat1);
            }
            if (state.dataSurface->OSC(Loop).SurfFilmCoef > 0.0) {
                s_ipsc->cAlphaArgs(1) = format("{:.3R}", state.dataSurface->OSC(Loop).SurfFilmCoef);
                SetupOutputVariable(state,
                                    "Surface Other Side Coefficients Exterior Air Drybulb Temperature",
                                    Constant::Units::C,
                                    state.dataSurface->OSC(Loop).OSCTempCalc,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataSurface->OSC(Loop).Name);
            } else {
                s_ipsc->cAlphaArgs(1) = "N/A";
            }

            print(state.files.eio,
                  "Other Side Coefficients,{},{},{},{:.3R},{:.3R},{:.3R},{:.3R},{:.3R},{},{},{:.3R},{:.3R},{}\n",
                  state.dataSurface->OSC(Loop).Name,
                  s_ipsc->cAlphaArgs(1),
                  (state.dataSurface->OSC(Loop).constTempSched != nullptr) ? "N/A" : format("{:.2R}", state.dataSurface->OSC(Loop).ConstTemp),
                  state.dataSurface->OSC(Loop).ConstTempCoef,
                  state.dataSurface->OSC(Loop).ExtDryBulbCoef,
                  state.dataSurface->OSC(Loop).GroundTempCoef,
                  state.dataSurface->OSC(Loop).WindSpeedCoef,
                  state.dataSurface->OSC(Loop).ZoneAirTempCoef,
                  (state.dataSurface->OSC(Loop).constTempSched == nullptr) ? "N/A" : state.dataSurface->OSC(Loop).constTempSched->Name,
                  s_ipsc->cAlphaArgs(3),
                  state.dataSurface->OSC(Loop).SinusoidPeriod,
                  state.dataSurface->OSC(Loop).TPreviousCoef,
                  cOSCLimitsString);
        }
    }

    void GetOSCMData(EnergyPlusData &state, bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   November 2004

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the OtherSideConditionsModel data.

        // REFERENCES:
        // derived from GetOSCData subroutine by Linda Lawrie

        //  OtherSideConditionsModel,
        //      \memo This object sets up modifying the other side conditions for a surface from other model results.
        //  A1, \field OtherSideConditionsModel Name
        //      \required-field
        //      \reference OSCMNames
        //      \reference OutFaceEnvNames
        //  A2; \field Type of Model to determine Boundary Conditions
        //      \type choice
        //      \key Transpired Collector
        //      \key Vented PV Cavity
        //      \key Hybrid PV Transpired Collector

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;
        int NumProps;
        int IOStat;

        auto &s_ipsc = state.dataIPShortCut;
        s_ipsc->cCurrentModuleObject = "SurfaceProperty:OtherSideConditionsModel";
        state.dataSurface->TotOSCM = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        state.dataSurface->OSCM.allocate(state.dataSurface->TotOSCM);
        // OSCM is already initialized in derived type defn.

        int OSCMNum = 0;
        for (int Loop = 1; Loop <= state.dataSurface->TotOSCM; ++Loop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(
                state, s_ipsc->cCurrentModuleObject, Loop, s_ipsc->cAlphaArgs, NumAlphas, s_ipsc->rNumericArgs, NumProps, IOStat);
            bool ErrorInName = false;
            bool IsBlank = false;
            Util::VerifyName(
                state, s_ipsc->cAlphaArgs(1), state.dataSurface->OSCM, OSCMNum, ErrorInName, IsBlank, s_ipsc->cCurrentModuleObject + " Name");
            if (ErrorInName) {
                ErrorsFound = true;
                continue;
            }

            ++OSCMNum;
            state.dataSurface->OSCM(OSCMNum).Name = s_ipsc->cAlphaArgs(1);
            // Note no validation of the below at this time:
            state.dataSurface->OSCM(OSCMNum).Class = s_ipsc->cAlphaArgs(2);
            // setup output vars for modeled coefficients
            SetupOutputVariable(state,
                                "Surface Other Side Conditions Modeled Convection Air Temperature",
                                Constant::Units::C,
                                state.dataSurface->OSCM(OSCMNum).TConv,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataSurface->OSCM(OSCMNum).Name);
            SetupOutputVariable(state,
                                "Surface Other Side Conditions Modeled Convection Heat Transfer Coefficient",
                                Constant::Units::W_m2K,
                                state.dataSurface->OSCM(OSCMNum).HConv,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataSurface->OSCM(OSCMNum).Name);
            SetupOutputVariable(state,
                                "Surface Other Side Conditions Modeled Radiation Temperature",
                                Constant::Units::C,
                                state.dataSurface->OSCM(OSCMNum).TRad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataSurface->OSCM(OSCMNum).Name);
            SetupOutputVariable(state,
                                "Surface Other Side Conditions Modeled Radiation Heat Transfer Coefficient",
                                Constant::Units::W_m2K,
                                state.dataSurface->OSCM(OSCMNum).HRad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataSurface->OSCM(OSCMNum).Name);

            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "Other Side Boundary Conditions",
                                 state.dataSurface->OSCM(OSCMNum).Name,
                                 "Convection Bulk Air Temperature",
                                 "[C]",
                                 state.dataSurface->OSCM(OSCMNum).EMSOverrideOnTConv,
                                 state.dataSurface->OSCM(OSCMNum).EMSOverrideTConvValue);
                SetupEMSActuator(state,
                                 "Other Side Boundary Conditions",
                                 state.dataSurface->OSCM(OSCMNum).Name,
                                 "Convection Heat Transfer Coefficient",
                                 "[W/m2-K]",
                                 state.dataSurface->OSCM(OSCMNum).EMSOverrideOnHConv,
                                 state.dataSurface->OSCM(OSCMNum).EMSOverrideHConvValue);
                SetupEMSActuator(state,
                                 "Other Side Boundary Conditions",
                                 state.dataSurface->OSCM(OSCMNum).Name,
                                 "Radiation Effective Temperature",
                                 "[C]",
                                 state.dataSurface->OSCM(OSCMNum).EMSOverrideOnTRad,
                                 state.dataSurface->OSCM(OSCMNum).EMSOverrideTRadValue);
                SetupEMSActuator(state,
                                 "Other Side Boundary Conditions",
                                 state.dataSurface->OSCM(OSCMNum).Name,
                                 "Radiation Linear Heat Transfer Coefficient",
                                 "[W/m2-K]",
                                 state.dataSurface->OSCM(OSCMNum).EMSOverrideOnHrad,
                                 state.dataSurface->OSCM(OSCMNum).EMSOverrideHradValue);
            }
        }

        for (int Loop = 1; Loop <= state.dataSurface->TotOSCM; ++Loop) {
            if (Loop == 1) {
                static constexpr std::string_view OSCMFormat1("! <Other Side Conditions Model>,Name,Class\n");
                print(state.files.eio, OSCMFormat1);
            }
            print(state.files.eio, "Other Side Conditions Model,{},{}\n", state.dataSurface->OSCM(Loop).Name, state.dataSurface->OSCM(Loop).Class);
        }
    }

    void GetMovableInsulationData(EnergyPlusData &state, bool &ErrorsFound) // If errors found in input
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the movable insulation data that can be associated with a surface.

        // REFERENCES:
        // Movable Insulation Definition
        // SurfaceControl:MovableInsulation,
        //       \memo Exterior or Interior Insulation on opaque surfaces
        //   A1, \field Insulation Type
        //       \required-field
        //       \type choice
        //       \key Outside
        //       \key Inside
        //   A2, \field Surface Name
        //       \required-field
        //       \type object-list
        //       \object-list SurfaceNames
        //   A3, \field Material Name
        //       \required-field
        //       \object-list MaterialName
        //   A4; \field Schedule Name
        //        \required-field
        //        \type object-list
        //        \object-list ScheduleNames

        static constexpr std::string_view routineName = "GetMovableInsulationInput";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NAlphas;
        int NNums;
        int IOStat;

        enum class InsulationType
        {
            Invalid = -1,
            Outside,
            Inside,
            Num
        };
        constexpr std::array<std::string_view, static_cast<int>(InsulationType::Num)> insulationTypeNamesUC = {"OUTSIDE", "INSIDE"};

        auto &s_ipsc = state.dataIPShortCut;
        auto &s_mat = state.dataMaterial;

        s_ipsc->cCurrentModuleObject = "SurfaceControl:MovableInsulation";
        int NMatInsul = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        for (int Loop = 1; Loop <= NMatInsul; ++Loop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NNums,
                                                                     IOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            InsulationType insulationType = static_cast<InsulationType>(getEnumValue(insulationTypeNamesUC, s_ipsc->cAlphaArgs(1)));
            if (insulationType == InsulationType::Invalid) {
                ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(1), s_ipsc->cAlphaArgs(1));
                ErrorsFound = true;
                continue;
            }

            int SurfNum;
            if ((SurfNum = Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataSurface->Surface, state.dataSurface->TotSurfaces)) == 0) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                ErrorsFound = true;
                continue;
            }

            int MaterNum;
            if ((MaterNum = Material::GetMaterialNum(state, s_ipsc->cAlphaArgs(3))) == 0) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                ErrorsFound = true;
                continue;
            }

            Sched::Schedule *sched = nullptr;
            if (s_ipsc->lAlphaFieldBlanks(4)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(4));
                ErrorsFound = true;
                continue;
            } else if ((sched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
                ErrorsFound = true;
                continue;
            }

            auto *thisMaterial = s_mat->materials(MaterNum);

            Array1D_string const cMaterialGroupType({-1, 18},
                                                    {"invalid",
                                                     "Material/Material:NoMass",
                                                     "Material:AirGap",
                                                     "WindowMaterial:Shade",
                                                     "WindowMaterial:Glazing*",
                                                     "WindowMaterial:Gas",
                                                     "WindowMaterial:Blind",
                                                     "WindowMaterial:GasMixture",
                                                     "WindowMaterial:Screen",
                                                     "Material:RoofVegetation",
                                                     "Material:InfraredTransparent",
                                                     "WindowMaterial:SimpleGlazingSystem",
                                                     "WindowMaterial:ComplexShade",
                                                     "WindowMaterial:Gap",
                                                     "WindowMaterial:Glazing:EquivalentLayer",
                                                     "WindowMaterial:Shade:EquivalentLayer",
                                                     "WindowMaterial:Drape:EquivalentLayer",
                                                     "WindowMaterial:Blind:EquivalentLayer",
                                                     "WindowMaterial:Screen:EquivalentLayer",
                                                     "WindowMaterial:Gap:EquivalentLayer"});

            Material::Group const MaterialLayerGroup = thisMaterial->group;
            if ((MaterialLayerGroup == Material::Group::GlassSimple) || (MaterialLayerGroup == Material::Group::ShadeEQL) ||
                (MaterialLayerGroup == Material::Group::DrapeEQL) || (MaterialLayerGroup == Material::Group::BlindEQL) ||
                (MaterialLayerGroup == Material::Group::ScreenEQL) || (MaterialLayerGroup == Material::Group::WindowGapEQL)) {
                ShowSevereError(state, format("Invalid movable insulation material for {}:", s_ipsc->cCurrentModuleObject));
                ShowSevereError(
                    state, format("...Movable insulation material type specified = {}", cMaterialGroupType(static_cast<int>(MaterialLayerGroup))));
                ShowSevereError(state, format("...Movable insulation material name specified = {}", s_ipsc->cAlphaArgs(3)));
                ErrorsFound = true;
            }

            switch (insulationType) {
            case InsulationType::Outside: {
                auto &movInsul = state.dataSurface->extMovInsuls(SurfNum);
                if (movInsul.matNum > 0) {
                    ShowSevereDuplicateAssignment(
                        state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2), s_mat->materials(movInsul.matNum)->Name);
                    ErrorsFound = true;
                }

                movInsul.matNum = MaterNum;
                movInsul.sched = sched;
                state.dataSurface->AnyMovableInsulation = true;
                state.dataSurface->extMovInsulSurfNums.push_back(SurfNum);

                if (thisMaterial->Resistance <= 0.0) {
                    if (thisMaterial->Conductivity <= 0.0 || thisMaterial->Thickness <= 0.0) {
                        ShowSevereError(state,
                                        format("{}, {}=\"{}\", invalid material.",
                                               s_ipsc->cCurrentModuleObject,
                                               s_ipsc->cAlphaFieldNames(2),
                                               s_ipsc->cAlphaArgs(2)));
                        ShowContinueError(state, "\"Outside\", invalid material for movable insulation.");
                        ShowContinueError(state,
                                          format("Material=\"{}\",Resistance=[{:.3R}], must be > 0 for use in Movable Insulation.",
                                                 thisMaterial->Name,
                                                 thisMaterial->Resistance));
                        ErrorsFound = true;
                    } else if (thisMaterial->Conductivity > 0.0) {
                        thisMaterial->Resistance = thisMaterial->Thickness / thisMaterial->Conductivity;
                    }
                }

                if (thisMaterial->Conductivity <= 0.0) {
                    if (thisMaterial->Resistance <= 0.0) {
                        ShowSevereError(state,
                                        format("{}, {}=\"{}\", invalid material.",
                                               s_ipsc->cCurrentModuleObject,
                                               s_ipsc->cAlphaFieldNames(2),
                                               s_ipsc->cAlphaArgs(2)));
                        ShowContinueError(state, "\"Outside\", invalid material for movable insulation.");
                        ShowContinueError(state,
                                          format("Material=\"{}\",Conductivity=[{:.3R}], must be > 0 for use in Movable Insulation.",
                                                 thisMaterial->Name,
                                                 thisMaterial->Conductivity));
                        ErrorsFound = true;
                    }
                }
            } break;

            case InsulationType::Inside: {
                auto &movInsul = state.dataSurface->intMovInsuls(SurfNum);
                if (movInsul.matNum > 0) {
                    ShowSevereDuplicateAssignment(
                        state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2), s_mat->materials(movInsul.matNum)->Name);
                    ErrorsFound = true;
                }

                movInsul.matNum = MaterNum;
                movInsul.sched = sched;
                state.dataSurface->AnyMovableInsulation = true;
                state.dataSurface->intMovInsulSurfNums.push_back(SurfNum);
                if (thisMaterial->Resistance <= 0.0) {
                    if (thisMaterial->Conductivity <= 0.0 || thisMaterial->Thickness <= 0.0) {
                        ShowSevereError(state,
                                        format("{}, {}=\"{}\", invalid material.",
                                               s_ipsc->cCurrentModuleObject,
                                               s_ipsc->cAlphaFieldNames(2),
                                               s_ipsc->cAlphaArgs(2)));
                        ShowContinueError(state, "\"Inside\", invalid material for movable insulation.");
                        ShowContinueError(state,
                                          format("Material=\"{}\",Resistance=[{:.3R}], must be > 0 for use in Movable Insulation.",
                                                 thisMaterial->Name,
                                                 thisMaterial->Resistance));
                        ErrorsFound = true;
                    } else if (thisMaterial->Conductivity > 0.0) {
                        thisMaterial->Resistance = thisMaterial->Thickness / thisMaterial->Conductivity;
                    }
                }
            } break;
            default: {
                assert(false);
            } break;
            } // switch (inulationType)

            if (state.dataSurface->Surface(SurfNum).Class == SurfaceClass::Window) {
                ShowSevereError(state, format("{}, {}=\"{}\"", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
                ShowContinueError(state, "invalid use on a Window. Use WindowShadingControl instead.");
                ErrorsFound = true;
            }
        } // for (Loop)
    } // GetMovableInsulationData()

    // Calculates the volume (m3) of a zone using the surfaces as possible.
    void CalculateZoneVolume(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Legacy Code
        //       DATE WRITTEN   1992-1994
        //       MODIFIED       Sep 2007, Mar 2017

        // METHODOLOGY EMPLOYED:
        // Uses surface area information for calculations.  Modified to use the
        // user-entered ceiling height (x floor area, if applicable) instead of using
        // the calculated volume when the user enters the ceiling height.

        // REFERENCES:
        // Legacy Code (IBLAST)

        Vectors::Polyhedron ZoneStruct;
        bool initmsg = true;
        bool ShowZoneSurfaces = (state.dataInputProcessing->inputProcessor->getNumSectionsFound("SHOWZONESURFACES_DEBUG") > 0);
        EPVector<int> surfacenotused;

        enum class ZoneVolumeCalcMethod
        {
            Invalid = -1,
            Enclosed,
            FloorAreaTimesHeight1,
            FloorAreaTimesHeight2,
            CeilingAreaTimesHeight,
            OpWallAreaTimesDistance,
            UserProvided,
            Num
        };

        int countNotFullyEnclosedZones = 0;
        for (auto &thisZone : state.dataHeatBal->Zone) {
            if (!thisZone.HasFloor) {
                ShowWarningError(state,
                                 format("No floor exists in Zone=\"{}\", zone floor area is zero. All values for this zone that are entered per "
                                        "floor area will be zero.",
                                        thisZone.Name));
            }

            Real64 SumAreas = 0.0;
            Real64 CalcVolume = 0.0;
            // Use AllSurfaceFirst which includes air boundaries
            int NFaces = thisZone.AllSurfaceLast - thisZone.AllSurfaceFirst + 1;
            int notused = 0;
            ZoneStruct.NumSurfaceFaces = NFaces;
            ZoneStruct.SurfaceFace.allocate(NFaces);
            int NActFaces = 0;
            surfacenotused.dimension(NFaces, 0);

            for (int SurfNum = thisZone.AllSurfaceFirst; SurfNum <= thisZone.AllSurfaceLast; ++SurfNum) {
                assert(SurfNum > 0);
                auto &thisSurface = state.dataSurface->Surface(SurfNum);
                // Only include Base Surfaces in Calc.

                if (thisSurface.Class != SurfaceClass::Wall && thisSurface.Class != SurfaceClass::Floor && thisSurface.Class != SurfaceClass::Roof) {
                    ++notused;
                    surfacenotused(notused) = SurfNum;
                    continue;
                }

                ++NActFaces;
                auto &thisFace = ZoneStruct.SurfaceFace(NActFaces);
                thisFace.FacePoints.allocate(thisSurface.Sides);
                thisFace.NSides = thisSurface.Sides;
                thisFace.SurfNum = SurfNum;
                thisFace.FacePoints({1, thisSurface.Sides}) = thisSurface.Vertex({1, thisSurface.Sides});
                Vectors::CreateNewellAreaVector(thisFace.FacePoints, thisFace.NSides, thisFace.NewellAreaVector);
                SumAreas += Vectors::VecLength(thisFace.NewellAreaVector);
            }
            ZoneStruct.NumSurfaceFaces = NActFaces;

            bool isFloorHorizontal = false;
            bool isCeilingHorizontal = false;
            bool areWallsVertical = false;
            std::tie(isFloorHorizontal, isCeilingHorizontal, areWallsVertical) = areSurfaceHorizAndVert(state, ZoneStruct);
            Real64 oppositeWallArea = 0.0;
            Real64 distanceBetweenOppositeWalls = 0.0;

            bool areWallsSameHeight = areWallHeightSame(state, ZoneStruct);

            std::vector<EdgeOfSurf> listOfedgeNotUsedTwice;
            bool isZoneEnclosed = isEnclosedVolume(ZoneStruct, listOfedgeNotUsedTwice);
            ZoneVolumeCalcMethod volCalcMethod;

            Real64 floorAreaForVolume = (thisZone.FloorArea > 0.0) ? thisZone.FloorArea : thisZone.geometricFloorArea;
            Real64 ceilingAreaForVolume = (thisZone.CeilingArea > 0.0) ? thisZone.CeilingArea : thisZone.geometricCeilingArea;

            if (isZoneEnclosed) {
                CalcVolume = Vectors::CalcPolyhedronVolume(state, ZoneStruct);
                volCalcMethod = ZoneVolumeCalcMethod::Enclosed;
            } else if (floorAreaForVolume > 0.0 && thisZone.CeilingHeight > 0.0 && areFloorAndCeilingSame(state, ZoneStruct)) {
                CalcVolume = floorAreaForVolume * thisZone.CeilingHeight;
                volCalcMethod = ZoneVolumeCalcMethod::FloorAreaTimesHeight1;
            } else if (isFloorHorizontal && areWallsVertical && areWallsSameHeight && floorAreaForVolume > 0.0 && thisZone.CeilingHeight > 0.0) {
                CalcVolume = floorAreaForVolume * thisZone.CeilingHeight;
                volCalcMethod = ZoneVolumeCalcMethod::FloorAreaTimesHeight2;
            } else if (isCeilingHorizontal && areWallsVertical && areWallsSameHeight && ceilingAreaForVolume > 0.0 && thisZone.CeilingHeight > 0.0) {
                CalcVolume = ceilingAreaForVolume * thisZone.CeilingHeight;
                volCalcMethod = ZoneVolumeCalcMethod::CeilingAreaTimesHeight;
            } else if (areOppositeWallsSame(state, ZoneStruct, oppositeWallArea, distanceBetweenOppositeWalls)) {
                CalcVolume = oppositeWallArea * distanceBetweenOppositeWalls;
                volCalcMethod = ZoneVolumeCalcMethod::OpWallAreaTimesDistance;
            } else if (thisZone.Volume == Constant::AutoCalculate) { // no user entered zone volume
                ShowSevereError(state,
                                format("For zone: {} it is not possible to calculate the volume from the surrounding surfaces so either provide the "
                                       "volume value or define all the surfaces to fully enclose the zone.",
                                       thisZone.Name));
                CalcVolume = 0.;
                volCalcMethod = ZoneVolumeCalcMethod::Invalid;
            } else {
                CalcVolume = 0.;
                volCalcMethod = ZoneVolumeCalcMethod::UserProvided;
            }
            if (!isZoneEnclosed) {
                ++countNotFullyEnclosedZones;
                if (state.dataGlobal->DisplayExtraWarnings) { // report missing
                    ShowWarningError(state,
                                     format("CalculateZoneVolume: The Zone=\"{}\" is not fully enclosed. To be fully enclosed, each edge of a "
                                            "surface must also be an edge on one other surface.",
                                            thisZone.Name));
                    switch (volCalcMethod) {
                    case ZoneVolumeCalcMethod::FloorAreaTimesHeight1:
                        ShowContinueError(state,
                                          "  The zone volume was calculated using the floor area times ceiling height method where the floor and "
                                          "ceiling are the same except for the z-coordinates.");
                        break;
                    case ZoneVolumeCalcMethod::FloorAreaTimesHeight2:
                        ShowContinueError(state,
                                          "  The zone volume was calculated using the floor area times ceiling height method where the floor is "
                                          "horizontal, the walls are vertical, and the wall heights are all the same.");
                        break;
                    case ZoneVolumeCalcMethod::CeilingAreaTimesHeight:
                        ShowContinueError(state,
                                          "  The zone volume was calculated using the ceiling area times ceiling height method where the ceiling is "
                                          "horizontal, the walls are vertical, and the wall heights are all the same.");
                        break;
                    case ZoneVolumeCalcMethod::OpWallAreaTimesDistance:
                        ShowContinueError(state,
                                          "  The zone volume was calculated using the opposite wall area times the distance between them method ");
                        break;
                    case ZoneVolumeCalcMethod::UserProvided:
                        ShowContinueError(state, "  The zone volume was provided as an input to the ZONE object ");
                        break;
                    case ZoneVolumeCalcMethod::Invalid:
                        ShowContinueError(state, "  The zone volume was not calculated and an error exists. ");
                        break;
                    case ZoneVolumeCalcMethod::Enclosed: // should not be called but completes enumeration
                        ShowContinueError(state, "  The zone volume was calculated using multiple pyramids and was fully enclosed. ");
                        break;
                    default:
                        assert(false);
                    }
                    for (auto &edge : listOfedgeNotUsedTwice) {
                        if (edge.count < 2) {
                            ShowContinueError(
                                state,
                                fmt::format("  The surface \"{}\" has an edge that was used only once: it is not an edge on another surface",
                                            state.dataSurface->Surface(edge.surfNum).Name));

                        } else {
                            ShowContinueError(
                                state,
                                fmt::format("  The surface \"{}\" has an edge that was used {} times: it is an edge on three or more surfaces: ",
                                            state.dataSurface->Surface(edge.surfNum).Name,
                                            edge.count));
                            std::string surfaceNames = "    It was found on the following Surfaces: ";
                            for (int surfNum : edge.otherSurfNums) {
                                surfaceNames += fmt::format("'{}' ", state.dataSurface->Surface(surfNum).Name);
                            }
                            ShowContinueError(state, surfaceNames);
                        }
                        ShowContinueError(state, format("    Vertex start {{ {:.4R}, {:.4R}, {:.4R}}}", edge.start.x, edge.start.y, edge.start.z));
                        ShowContinueError(state, format("    Vertex end   {{ {:.4R}, {:.4R}, {:.4R}}}", edge.end.x, edge.end.y, edge.end.z));
                    }
                }
            }
            if (thisZone.Volume > 0.0) { // User entered zone volume, produce message if not near calculated
                if (CalcVolume > 0.0) {
                    if (std::abs(CalcVolume - thisZone.Volume) / thisZone.Volume > 0.05) {
                        ++state.dataSurfaceGeometry->ErrCount5;
                        if (state.dataSurfaceGeometry->ErrCount5 == 1 && !state.dataGlobal->DisplayExtraWarnings) {
                            if (initmsg) {
                                ShowMessage(state,
                                            "Note that the following warning(s) may/will occur if you have not enclosed your zone completely.");
                                initmsg = false;
                            }
                            ShowWarningError(state, "Entered Zone Volumes differ from calculated zone volume(s).");
                            ShowContinueError(state, "...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual zones.");
                        }
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if (initmsg) {
                                ShowMessage(state,
                                            "Note that the following warning(s) may/will occur if you have not enclosed your zone completely.");
                                initmsg = false;
                            }
                            // Warn user of using specified Zone Volume
                            ShowWarningError(
                                state,
                                format("Entered Volume entered for Zone=\"{}\" significantly different from calculated Volume", thisZone.Name));
                            ShowContinueError(state,
                                              format("Entered Zone Volume value={:.2R}, Calculated Zone Volume value={:.2R}, entered volume will be "
                                                     "used in calculations.",
                                                     thisZone.Volume,
                                                     CalcVolume));
                        }
                    }
                }
            } else if (thisZone.ceilingHeightEntered) { // User did not enter zone volume, but entered ceiling height
                if (floorAreaForVolume > 0.0) {
                    thisZone.Volume = floorAreaForVolume * thisZone.CeilingHeight;
                } else { // ceiling height entered but floor area zero
                    thisZone.Volume = CalcVolume;
                }
            } else { // Neither ceiling height nor volume entered
                thisZone.Volume = CalcVolume;
            }

            if (thisZone.Volume <= 0.0) {
                ShowWarningError(state, format("Indicated Zone Volume <= 0.0 for Zone={}", thisZone.Name));
                ShowContinueError(state, format("The calculated Zone Volume was={:.2R}", thisZone.Volume));
                ShowContinueError(state, "The simulation will continue with the Zone Volume set to 10.0 m3. ");
                ShowContinueError(state, "...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual zones.");
                thisZone.Volume = 10.;
            }
            // For now - pro-rate space volumes by floor area, if not entered
            for (int spaceNum : thisZone.spaceIndexes) {
                auto &thisSpace = state.dataHeatBal->space(spaceNum);
                // don't touch if already user-specified
                if (thisSpace.Volume > 0.0) {
                    continue;
                }
                if (thisZone.numSpaces == 1) {
                    thisSpace.Volume = thisZone.Volume;
                } else if (thisZone.geometricFloorArea > 0.0) {
                    thisSpace.Volume = thisZone.Volume * thisSpace.FloorArea / thisZone.geometricFloorArea;
                }
            }
            Real64 totSpacesVolume = 0.0;
            for (int spaceNum : thisZone.spaceIndexes) {
                totSpacesVolume += state.dataHeatBal->space(spaceNum).Volume;
            }
            if (totSpacesVolume > 0.0) {
                for (int spaceNum : thisZone.spaceIndexes) {
                    state.dataHeatBal->space(spaceNum).fracZoneVolume = state.dataHeatBal->space(spaceNum).Volume / totSpacesVolume;
                }
            } // else leave fractions at zero

            if (ShowZoneSurfaces) {
                if (state.dataSurfaceGeometry->ShowZoneSurfaceHeaders) {
                    print(state.files.debug, "{}\n", "===================================");
                    print(state.files.debug, "{}\n", "showing zone surfaces used and not used in volume calculation");
                    print(state.files.debug, "{}\n", "for volume calculation, only floors, walls and roofs/ceilings are used");
                    print(state.files.debug, "{}\n", "surface class, 1=wall, 2=floor, 3=roof/ceiling");
                    print(state.files.debug, "{}\n", "unused surface class(es), 5=internal mass, 11=window, 12=glass door");
                    print(state.files.debug, "{}\n", "                          13=door, 14=shading, 15=overhang, 16=fin");
                    print(state.files.debug, "{}\n", "                          17=TDD Dome, 18=TDD Diffuser");
                    state.dataSurfaceGeometry->ShowZoneSurfaceHeaders = false;
                }
                print(state.files.debug, "{}\n", "===================================");
                print(state.files.debug, "zone={} calc volume={}\n", thisZone.Name, CalcVolume);
                print(state.files.debug, " nsurfaces={} nactual={}\n", NFaces, NActFaces);
            }
            for (int faceNum = 1; faceNum <= ZoneStruct.NumSurfaceFaces; ++faceNum) {
                auto &thisFace = ZoneStruct.SurfaceFace(faceNum);
                if (ShowZoneSurfaces) {
                    if (faceNum <= NActFaces) {
                        auto &thisSurface = state.dataSurface->Surface(thisFace.SurfNum);
                        print(state.files.debug, "surface={} nsides={}\n", thisFace.SurfNum, thisFace.NSides);
                        print(state.files.debug, "surface name={} class={}\n", thisSurface.Name, thisSurface.Class);
                        print(state.files.debug, "area={}\n", thisSurface.GrossArea);
                        for (int iside = 1; iside <= thisFace.NSides; ++iside) {
                            auto const &FacePoint(thisFace.FacePoints(iside));
                            print(state.files.debug, "{} {} {}\n", FacePoint.x, FacePoint.y, FacePoint.z);
                        }
                    }
                }
                thisFace.FacePoints.deallocate();
            }
            if (ShowZoneSurfaces) {
                for (int SurfNum = 1; SurfNum <= notused; ++SurfNum) {
                    print(state.files.debug,
                          "notused:surface={} name={} class={}\n",
                          surfacenotused(SurfNum),
                          state.dataSurface->Surface(surfacenotused(SurfNum)).Name,
                          state.dataSurface->Surface(surfacenotused(SurfNum)).Class);
                }
            }

            ZoneStruct.SurfaceFace.deallocate();
            surfacenotused.deallocate();

        } // zone loop
        if (!state.dataGlobal->DisplayExtraWarnings) {
            if (countNotFullyEnclosedZones == 1) {
                ShowWarningError(
                    state, "CalculateZoneVolume: 1 zone is not fully enclosed. For more details use:  Output:Diagnostics,DisplayExtrawarnings; ");
            } else if (countNotFullyEnclosedZones > 1) {
                ShowWarningError(state,
                                 format("CalculateZoneVolume: {} zones are not fully enclosed. For more details use:  "
                                        "Output:Diagnostics,DisplayExtrawarnings; ",
                                        countNotFullyEnclosedZones));
            }
        }
    }

    // test if the volume described by the polyhedron if full enclosed (would not leak)
    bool isEnclosedVolume(DataVectorTypes::Polyhedron const &zonePoly, std::vector<EdgeOfSurf> &edgeNot2)
    {
        // J. Glazer - March 2017

        std::vector<Vector> uniqueVertices = makeListOfUniqueVertices(zonePoly);

        std::vector<EdgeOfSurf> edgeNot2orig = edgesNotTwoForEnclosedVolumeTest(zonePoly, uniqueVertices);
        // if all edges had two counts then it is fully enclosed
        if (edgeNot2orig.empty()) {
            edgeNot2 = edgeNot2orig;
            return true;
        } else { // if the count is three or greater it is likely that a vertex that is colinear was counted on the faces on one edge and not
                 // on the "other side" of the edge Go through all the points looking for the number that are colinear and see if that is
                 // consistent with the number of edges found that didn't have a count of two
            DataVectorTypes::Polyhedron updatedZonePoly = updateZonePolygonsForMissingColinearPoints(
                zonePoly, uniqueVertices); // this is done after initial test since it is computationally intensive.
            std::vector<EdgeOfSurf> edgeNot2again = edgesNotTwoForEnclosedVolumeTest(updatedZonePoly, uniqueVertices);
            if (edgeNot2again.empty()) {
                return true;
            } else {
                edgeNot2 = edgesInBoth(edgeNot2orig,
                                       edgeNot2again); // only return a list of those edges that appear in both the original edge and the
                                                       // revised edges this eliminates added edges that will confuse users and edges that
                                                       // were caught by the updateZonePoly routine
                return false;
            }
        }
    }

    // returns a vector of edges that are in both vectors
    std::vector<EdgeOfSurf> edgesInBoth(std::vector<EdgeOfSurf> const &edges1, std::vector<EdgeOfSurf> const &edges2)
    {
        // J. Glazer - June 2017
        // this is not optimized but the number of edges for a typical polyhedron is 12 and is probably rarely bigger than 20.

        std::vector<EdgeOfSurf> inBoth;
        for (const auto &e1 : edges1) {
            for (const auto &e2 : edges2) {
                if (edgesEqualOnSameSurface(e1, e2)) {
                    inBoth.push_back(e1);
                    break;
                }
            }
        }
        return inBoth;
    }

    // returns true if the edges match - including the surface number
    bool edgesEqualOnSameSurface(EdgeOfSurf a, EdgeOfSurf b)
    {
        if (a.surfNum != b.surfNum) {
            return false;
        }

        // vertex comparison (we compare indices, so absolute equal)
        return ((a.start == b.start && a.end == b.end) || (a.start == b.end && a.end == b.start));
    }

    // returns the number of times the edges of the polyhedron of the zone are not used twice by the sides
    std::vector<EdgeOfSurf> edgesNotTwoForEnclosedVolumeTest(DataVectorTypes::Polyhedron const &zonePoly, std::vector<Vector> const &uniqueVertices)
    {
        // J. Glazer - March 2017

        struct EdgeByPts
        {
            int start;
            int end;
            int count;
            int firstSurfNum;
            std::vector<int> otherSurfNums;
            EdgeByPts() : start(0), end(0), count(0), firstSurfNum(0)
            {
            }
        };
        std::vector<EdgeByPts> uniqueEdges;
        uniqueEdges.reserve(zonePoly.NumSurfaceFaces * 6);

        // construct list of unique edges
        Vector curVertex;
        int curVertexIndex;
        for (int iFace = 1; iFace <= zonePoly.NumSurfaceFaces; ++iFace) {
            Vector prevVertex;
            int prevVertexIndex;
            for (int jVertex = 1; jVertex <= zonePoly.SurfaceFace(iFace).NSides; ++jVertex) {
                if (jVertex == 1) {
                    prevVertex = zonePoly.SurfaceFace(iFace).FacePoints(zonePoly.SurfaceFace(iFace).NSides); // the last point
                    prevVertexIndex = findIndexOfVertex(prevVertex, uniqueVertices);
                } else {
                    prevVertex = curVertex;
                    prevVertexIndex = curVertexIndex;
                }
                curVertex = zonePoly.SurfaceFace(iFace).FacePoints(jVertex);
                curVertexIndex = findIndexOfVertex(curVertex, uniqueVertices); // uses isAlmostEqual3dPt
                auto it = std::find_if(uniqueEdges.begin(), uniqueEdges.end(), [&curVertexIndex, &prevVertexIndex](const auto &edge) {
                    return ((edge.start == curVertexIndex && edge.end == prevVertexIndex) ||
                            (edge.start == prevVertexIndex && edge.end == curVertexIndex));
                });
                if (it == uniqueEdges.end()) {
                    EdgeByPts curEdge;
                    curEdge.start = prevVertexIndex;
                    curEdge.end = curVertexIndex;
                    curEdge.count = 1;
                    curEdge.firstSurfNum = zonePoly.SurfaceFace(iFace).SurfNum;
                    uniqueEdges.emplace_back(curEdge);
                } else {
                    ++(it->count);
                    it->otherSurfNums.push_back(zonePoly.SurfaceFace(iFace).SurfNum);
                }
            }
        }
        // All edges for an enclosed polyhedron should be shared by two (and only two) sides.
        // So if the count is not two for all edges, the polyhedron is not enclosed
        std::vector<EdgeOfSurf> edgesNotTwoCount;
        for (const auto &anEdge : uniqueEdges) {
            if (anEdge.count != 2) {
                EdgeOfSurf curEdgeOne;
                curEdgeOne.surfNum = anEdge.firstSurfNum;
                curEdgeOne.start = uniqueVertices[anEdge.start];
                curEdgeOne.end = uniqueVertices[anEdge.end];
                curEdgeOne.count = anEdge.count;
                curEdgeOne.otherSurfNums = anEdge.otherSurfNums;
                edgesNotTwoCount.push_back(curEdgeOne);
            }
        }
        return edgesNotTwoCount;
    }

    // create a list of unique vertices given the polyhedron describing the zone
    std::vector<Vector> makeListOfUniqueVertices(DataVectorTypes::Polyhedron const &zonePoly)
    {
        // J. Glazer - March 2017

        std::vector<Vector> uniqVertices;
        uniqVertices.reserve(zonePoly.NumSurfaceFaces * 6);

        for (int iFace = 1; iFace <= zonePoly.NumSurfaceFaces; ++iFace) {
            for (int jVertex = 1; jVertex <= zonePoly.SurfaceFace(iFace).NSides; ++jVertex) {
                Vector curVertex = zonePoly.SurfaceFace(iFace).FacePoints(jVertex);
                if (uniqVertices.size() == 0) {
                    uniqVertices.emplace_back(curVertex);
                } else {
                    bool found = false;
                    for (const auto &unqV : uniqVertices) {
                        if (isAlmostEqual3dPt(curVertex, unqV)) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        uniqVertices.emplace_back(curVertex);
                    }
                }
            }
        }
        return uniqVertices;
    }

    // updates the polyhedron used to describe a zone to include points on an edge that are between and collinear to points already describing
    // the edge
    DataVectorTypes::Polyhedron updateZonePolygonsForMissingColinearPoints(DataVectorTypes::Polyhedron const &zonePoly,
                                                                           std::vector<Vector> const &uniqVertices)
    {
        // J. Glazer - March 2017

        DataVectorTypes::Polyhedron updZonePoly = zonePoly; // set the return value to the original polyhedron describing the zone

        for (auto &updFace : updZonePoly.SurfaceFace) {
            bool insertedVertext = true;
            while (insertedVertext) {
                insertedVertext = false;
                auto &vertices = updFace.FacePoints;
                for (auto it = vertices.begin(); it != vertices.end(); ++it) {

                    auto itnext = std::next(it);
                    if (itnext == std::end(vertices)) {
                        itnext = std::begin(vertices);
                    }

                    auto curVertex = *it;      // (AUTO_OK_OBJ) can't tell if a copy is the intended behavior here
                    auto nextVertex = *itnext; // (AUTO_OK_OBJ)

                    // now go through all the vertices and see if they are colinear with start and end vertices
                    for (const auto &testVertex : uniqVertices) {
                        if (!isAlmostEqual3dPt(curVertex, testVertex) && !isAlmostEqual3dPt(nextVertex, testVertex)) {
                            if (isPointOnLineBetweenPoints(curVertex, nextVertex, testVertex)) {
                                vertices.insert(itnext, testVertex);
                                ++updFace.NSides;
                                insertedVertext = true;
                                break;
                            }
                        }
                    }
                    // Break out of the loop on vertices of the surface too, and start again at the while
                    if (insertedVertext) {
                        break;
                    }
                }
            }
        }
        return updZonePoly;
    }

    // test if the ceiling and floor are the same except for their height difference by looking at the corners
    bool areFloorAndCeilingSame(EnergyPlusData &state, DataVectorTypes::Polyhedron const &zonePoly)
    {
        // J. Glazer - March 2017

        // check if the floor and ceiling are the same
        // this is almost equivalent to saying, if you ignore the z-coordinate, are the vertices the same
        // so if you could all the unique vertices of the floor and ceiling, ignoring the z-coordinate, they
        // should always be even (they would be two but you might define multiple surfaces that meet in a corner)

        std::vector<DataVectorTypes::Vector2dCount> floorCeilingXY;
        floorCeilingXY.reserve(zonePoly.NumSurfaceFaces * 6);

        // make list of x and y coordinates for all faces that are on the floor or ceiling
        for (int iFace = 1; iFace <= zonePoly.NumSurfaceFaces; ++iFace) {
            int curSurfNum = zonePoly.SurfaceFace(iFace).SurfNum;
            if (state.dataSurface->Surface(curSurfNum).Class == SurfaceClass::Floor ||
                state.dataSurface->Surface(curSurfNum).Class == SurfaceClass::Roof) {
                for (int jVertex = 1; jVertex <= zonePoly.SurfaceFace(iFace).NSides; ++jVertex) {
                    Vector curVertex = zonePoly.SurfaceFace(iFace).FacePoints(jVertex);
                    DataVectorTypes::Vector2dCount curXYc;
                    curXYc.x = curVertex.x;
                    curXYc.y = curVertex.y;
                    curXYc.count = 1;
                    bool found = false;
                    for (DataVectorTypes::Vector2dCount &curFloorCeiling : floorCeilingXY) { // can't use just "auto" because updating floorCeilingXY
                        if (isAlmostEqual2dPt(curXYc, curFloorCeiling)) {                    // count ignored in comparison
                            ++curFloorCeiling.count;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        floorCeilingXY.emplace_back(curXYc);
                    }
                }
            }
        }
        // now make sure every point has been counted and even number of times (usually twice)
        // if they are then the ceiling and floor are (almost certainly) the same x and y coordinates.
        bool areFlrAndClgSame = true;
        if (floorCeilingXY.size() > 0) {
            for (auto const &curFloorCeiling : floorCeilingXY) {
                if (curFloorCeiling.count % 2 != 0) {
                    areFlrAndClgSame = false;
                    break;
                }
            }
        } else {
            areFlrAndClgSame = false;
        }
        return areFlrAndClgSame;
    }

    // test if the walls of a zone are all the same height using the polyhedron describing the zone geometry
    bool areWallHeightSame(EnergyPlusData &state, DataVectorTypes::Polyhedron const &zonePoly)
    {
        // J. Glazer - March 2017

        // test if all the wall heights are the same (all walls have the same maximum z-coordinate

        bool areWlHgtSame = true;
        Real64 wallHeightZ = -Constant::BigNumber;
        bool foundWallHeight = false;
        for (int iFace = 1; iFace <= zonePoly.NumSurfaceFaces; ++iFace) {
            int curSurfNum = zonePoly.SurfaceFace(iFace).SurfNum;
            if (state.dataSurface->Surface(curSurfNum).Class == SurfaceClass::Wall) {
                Real64 maxZ = -Constant::BigNumber;
                for (int jVertex = 1; jVertex <= zonePoly.SurfaceFace(iFace).NSides; ++jVertex) {
                    Vector curVertex = zonePoly.SurfaceFace(iFace).FacePoints(jVertex);
                    if (maxZ < curVertex.z) {
                        maxZ = curVertex.z;
                    }
                }
                if (foundWallHeight) {
                    if (std::abs(maxZ - wallHeightZ) > Constant::TwoCentimeters) {
                        areWlHgtSame = false;
                        break;
                    }
                } else {
                    wallHeightZ = maxZ;
                    foundWallHeight = true;
                }
            }
        }
        return areWlHgtSame;
    }

    // tests if the floor is horizontal, ceiling is horizontal, and walls are vertical and returns all three as a tuple of booleans
    std::tuple<bool, bool, bool> areSurfaceHorizAndVert(EnergyPlusData &state, DataVectorTypes::Polyhedron const &zonePoly)
    {
        // J. Glazer - March 2017

        // check if floors and ceilings are horizontal and walls are vertical
        bool isFlrHoriz = true;
        bool isClgHoriz = true;
        bool areWlVert = true;
        for (int iFace = 1; iFace <= zonePoly.NumSurfaceFaces; ++iFace) {
            int curSurfNum = zonePoly.SurfaceFace(iFace).SurfNum;
            if (state.dataSurface->Surface(curSurfNum).Class == SurfaceClass::Floor) {
                if (std::abs(state.dataSurface->Surface(curSurfNum).Tilt - 180.) > 1.) { // with 1 degree angle
                    isFlrHoriz = false;
                }
            } else if (state.dataSurface->Surface(curSurfNum).Class == SurfaceClass::Roof) { // includes ceilings
                if (std::abs(state.dataSurface->Surface(curSurfNum).Tilt) > 1.) {            // with 1 degree angle of
                    isClgHoriz = false;
                }
            } else if (state.dataSurface->Surface(curSurfNum).Class == SurfaceClass::Wall) {
                if (std::abs(state.dataSurface->Surface(curSurfNum).Tilt - 90) > 1.) { // with 1 degree angle
                    areWlVert = false;
                }
            }
        }
        return std::make_tuple(isFlrHoriz, isClgHoriz, areWlVert);
    }

    // tests whether a pair of walls in the zone are the same except offset from one another and facing the opposite direction and also
    // returns the wall area and distance between
    bool areOppositeWallsSame(EnergyPlusData &state,
                              DataVectorTypes::Polyhedron const &zonePoly,
                              Real64 &oppositeWallArea,            // return the area of the wall that has an opposite wall
                              Real64 &distanceBetweenOppositeWalls // returns distance
    )
    {
        // J. Glazer - March 2017

        // approach: if opposite surfaces have opposite azimuth and same area, then check the distance between the
        // vertices( one counting backwards ) and if it is the same distance than assume that it is the same.
        bool foundOppEqual = false;
        for (int iFace = 1; iFace <= zonePoly.NumSurfaceFaces; ++iFace) {
            int curSurfNum = zonePoly.SurfaceFace(iFace).SurfNum;
            if (state.dataSurface->Surface(curSurfNum).Class == SurfaceClass::Wall) {
                std::vector<int> facesAtAz = listOfFacesFacingAzimuth(state, zonePoly, state.dataSurface->Surface(curSurfNum).Azimuth);
                bool allFacesEquidistant = true;
                oppositeWallArea = 0.;
                for (int curFace : facesAtAz) {
                    int possOppFace = findPossibleOppositeFace(state, zonePoly, curFace);
                    if (possOppFace > 0) { // an opposite fact was found
                        oppositeWallArea += state.dataSurface->Surface(zonePoly.SurfaceFace(curFace).SurfNum).Area;
                        if (!areCornersEquidistant(zonePoly, curFace, possOppFace, distanceBetweenOppositeWalls)) {
                            allFacesEquidistant = false;
                            break;
                        }
                    } else {
                        allFacesEquidistant = false;
                        break;
                    }
                }
                if (allFacesEquidistant) {
                    foundOppEqual = true;
                    break; // only need to find the first case where opposite walls are the same
                }
            }
        }
        return foundOppEqual;
    }

    // provides a list of indices of polyhedron faces that are facing a specific azimuth
    std::vector<int> listOfFacesFacingAzimuth(EnergyPlusData &state, DataVectorTypes::Polyhedron const &zonePoly, Real64 const azimuth)
    {
        // J. Glazer - March 2017

        std::vector<int> facingAzimuth;
        facingAzimuth.reserve(zonePoly.NumSurfaceFaces);

        for (int iFace = 1; iFace <= zonePoly.NumSurfaceFaces; ++iFace) {
            int curSurfNum = zonePoly.SurfaceFace(iFace).SurfNum;
            if (General::rotAzmDiffDeg(state.dataSurface->Surface(curSurfNum).Azimuth, azimuth) < 1.) {
                facingAzimuth.emplace_back(iFace);
            }
        }
        return facingAzimuth;
    }

    // returns the index of the face of a polyhedron that is probably opposite of the face index provided
    int findPossibleOppositeFace(EnergyPlusData &state, DataVectorTypes::Polyhedron const &zonePoly, int const faceIndex)
    {
        // J. Glazer - March 2017

        int selectedSurNum = zonePoly.SurfaceFace(faceIndex).SurfNum;
        Real64 selectedAzimuth = state.dataSurface->Surface(selectedSurNum).Azimuth;
        Real64 oppositeAzimuth = fmod(selectedAzimuth + 180., 360.);
        Real64 selectedArea = state.dataSurface->Surface(selectedSurNum).Area;
        int selectedNumCorners = zonePoly.SurfaceFace(faceIndex).NSides;
        int found = -1;

        for (int iFace = 1; iFace <= zonePoly.NumSurfaceFaces; ++iFace) {
            int curSurfNum = zonePoly.SurfaceFace(iFace).SurfNum;
            if ((zonePoly.SurfaceFace(iFace).NSides == selectedNumCorners) &&
                (std::abs(state.dataSurface->Surface(curSurfNum).Area - selectedArea) < 0.01) &&
                (std::abs(state.dataSurface->Surface(curSurfNum).Azimuth - oppositeAzimuth) < 1.)) {
                found = iFace;
                break;
            }
        }
        return found;
    }

    // tests if the corners of one face of the polyhedron are the same distance from corners of another face
    bool areCornersEquidistant(DataVectorTypes::Polyhedron const &zonePoly, int const faceIndex, int const opFaceIndex, Real64 &distanceBetween)
    {
        // J. Glazer - March 2017

        bool allAreEquidistant = true;
        Real64 firstDistance = -99.;
        if (zonePoly.SurfaceFace(faceIndex).NSides == zonePoly.SurfaceFace(opFaceIndex).NSides) { // double check that the number of sides match
            for (int iVertex = 1; iVertex <= zonePoly.SurfaceFace(faceIndex).NSides; ++iVertex) {
                int iVertexOpp = 1 + zonePoly.SurfaceFace(faceIndex).NSides - iVertex; // count backwards for opposite face
                Real64 curDistBetwCorners =
                    distance(zonePoly.SurfaceFace(faceIndex).FacePoints(iVertex), zonePoly.SurfaceFace(opFaceIndex).FacePoints(iVertexOpp));
                if (iVertex == 1) {
                    firstDistance = curDistBetwCorners;
                } else {
                    if (std::abs(curDistBetwCorners - firstDistance) > Constant::OneCentimeter) {
                        allAreEquidistant = false;
                        break;
                    }
                }
            }
        } else {
            allAreEquidistant = false;
        }
        if (allAreEquidistant) {
            distanceBetween = firstDistance;
        }
        return allAreEquidistant;
    }

    // test if two points in space are in the same position based on a small tolerance
    bool isAlmostEqual3dPt(DataVectorTypes::Vector v1, DataVectorTypes::Vector v2)
    {
        // J. Glazer - March 2017

        return ((std::abs(v1.x - v2.x) < Constant::OneCentimeter) && (std::abs(v1.y - v2.y) < Constant::OneCentimeter) &&
                (std::abs(v1.z - v2.z) < Constant::OneCentimeter));
    }

    // test if two points on a plane are in the same position based on a small tolerance
    bool isAlmostEqual2dPt(DataVectorTypes::Vector_2d v1, DataVectorTypes::Vector_2d v2)
    {
        // J. Glazer - March 2017

        return ((std::abs(v1.x - v2.x) < Constant::OneCentimeter) && (std::abs(v1.y - v2.y) < Constant::OneCentimeter));
    }

    // test if two points on a plane are in the same position based on a small tolerance (based on Vector2dCount comparison)
    bool isAlmostEqual2dPt(DataVectorTypes::Vector2dCount v1, DataVectorTypes::Vector2dCount v2)
    {
        // J. Glazer - March 2017

        return ((std::abs(v1.x - v2.x) < Constant::OneCentimeter) && (std::abs(v1.y - v2.y) < Constant::OneCentimeter));
    }

    // returns the index of vertex in a list that is in the same position in space as the given vertex
    int findIndexOfVertex(DataVectorTypes::Vector vertexToFind, std::vector<DataVectorTypes::Vector> const &listOfVertices)
    {
        // J. Glazer - March 2017

        for (std::size_t i = 0; i < listOfVertices.size(); i++) {
            if (isAlmostEqual3dPt(listOfVertices[i], vertexToFind)) {
                return i;
            }
        }
        return -1;
    }

    // returns the distance between two points in space
    Real64 distance(DataVectorTypes::Vector v1, DataVectorTypes::Vector v2)
    {
        // J. Glazer - March 2017

        return sqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
    }

    Real64 distanceFromPointToLine(DataVectorTypes::Vector start, DataVectorTypes::Vector end, DataVectorTypes::Vector test)
    {
        // np.linalg.norm(np.cross(e-s,p-s)/np.linalg.norm(e-s))
        DataVectorTypes::Vector t = end - start;
        t.normalize(); // Unit vector of start to end

        DataVectorTypes::Vector other = test - start;

        DataVectorTypes::Vector projection = DataVectorTypes::cross(t, other); // normal unit vector, that's the distance component
        return projection.length();
    }

    // tests if a point in space lies on the line segment defined by two other points
    bool isPointOnLineBetweenPoints(DataVectorTypes::Vector start, DataVectorTypes::Vector end, DataVectorTypes::Vector test)
    {
        // J. Glazer - March 2017
        // The tolerance has to be low enough. Take for eg a plenum that has an edge that's 30meters long, you risk adding point from the
        // floor to the roof, cf #7383 compute the shortest distance from the point to the line first to avoid false positive
        if (distanceFromPointToLine(start, end, test) <
            Constant::OneCentimeter) { // distanceFromPointToLine always positive, it's calculated as norml_L2
            return (std::abs((distance(start, end) - (distance(start, test) + distance(test, end)))) < Constant::OneCentimeter);
        }
        return false;
    }

    bool EdgeOfSurf::operator==(const EdgeOfSurf &other) const
    {
        return ((isAlmostEqual3dPt(this->start, other.start) && isAlmostEqual3dPt(this->end, other.end)) ||
                (isAlmostEqual3dPt(this->start, other.end) && isAlmostEqual3dPt(this->end, other.start)));
    }

    bool EdgeOfSurf::operator!=(const EdgeOfSurf &other) const
    {
        return !(*this == other);
    }

    bool EdgeOfSurf::containsPoints(const Vector &vertex) const
    {
        return (!isAlmostEqual3dPt(this->start, vertex) && !isAlmostEqual3dPt(this->end, vertex) &&
                isPointOnLineBetweenPoints(this->start, this->end, vertex));
    }

    double EdgeOfSurf::length() const
    {
        return distance(this->start, this->end);
    }

    void ProcessSurfaceVertices(EnergyPlusData &state, int const ThisSurf, bool &ErrorsFound)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Legacy Code (Walton)
        //       DATE WRITTEN   1976
        //       MODIFIED        FW, Mar 2002: Add triangular windows
        //                       FW, May 2002: modify test for 4-sided but non-rectangular subsurfaces
        //                       FW, Sep 2002: add shape for base surfaces (walls and detached shading surfaces)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine processes each surface into the vertex representation used
        // by the shading procedures.

        // METHODOLOGY EMPLOYED:
        // Detached Shading, Base Surfaces, Attached Shading surfaces are represented in the
        // same manner as original.  Subsurfaces (windows, doors) are a "relative coordinate".

        static constexpr std::string_view RoutineName("ProcessSurfaceVertices: ");

        Real64 X1;           // Intermediate Result
        Real64 Y1;           // Intermediate Result
        Real64 Z1;           // Intermediate Result
        Real64 XLLC;         // X-coordinate of lower left corner
        Real64 YLLC;         // Y-coordinate of lower left corner
        Real64 ZLLC;         // Z-coordinate of lower left corner
        int n;               // Vertex Number in Loop
        int ThisBaseSurface; // Current base surface
        Real64 Xp;
        Real64 Yp;
        Real64 Zp;
        Real64 SurfWorldAz; // Surface Azimuth (facing)
        Real64 SurfTilt;    // Surface Tilt
        DataSurfaces::SurfaceShape ThisShape(DataSurfaces::SurfaceShape::None);
        bool BaseSurface; // True if a base surface or a detached shading surface
        Real64 ThisSurfAz;
        Real64 ThisSurfTilt;
        Real64 ThisReveal;
        Real64 ThisWidth;
        Real64 ThisHeight;
        Real64 FrWidth;      // Frame width for exterior windows (m)
        Real64 FrArea;       // Frame area for exterior windows(m2)
        Real64 DivWidth;     // Divider width for exterior windows (m)
        Real64 DivArea;      // Divider area for exterior windows (m2)
        Real64 DivFrac;      // Fraction of divider area without overlaps
        bool ErrorInSurface; // false/true, depending on pass through routine
        bool HeatTransSurf;
        Real64 OutOfLine;

        // Object Data
        Vectors::PlaneEq BasePlane;
        Vector TVect;
        Vector CoordinateTransVector;

        if (state.dataSurface->Surface(ThisSurf).VerticesProcessed) {
            return;
        }

        ErrorInSurface = false;

        if (state.dataSurfaceGeometry->ProcessSurfaceVerticesOneTimeFlag) {
            state.dataSurfaceGeometry->Xpsv.allocate(state.dataSurface->MaxVerticesPerSurface);
            state.dataSurfaceGeometry->Ypsv.allocate(state.dataSurface->MaxVerticesPerSurface);
            state.dataSurfaceGeometry->Zpsv.allocate(state.dataSurface->MaxVerticesPerSurface);
            state.dataSurfaceGeometry->Xpsv = 0.0;
            state.dataSurfaceGeometry->Ypsv = 0.0;
            state.dataSurfaceGeometry->Zpsv = 0.0;
            state.dataSurfaceGeometry->ProcessSurfaceVerticesOneTimeFlag = false;
        }

        // Categorize this surface
        auto &surf = state.dataSurface->Surface(ThisSurf);
        BaseSurface = (surf.BaseSurf == 0 || surf.BaseSurf == ThisSurf);

        ThisBaseSurface = surf.BaseSurf; // Dont know if this is still needed or not
        HeatTransSurf = surf.HeatTransSurf;

        // Kludge for daylighting shelves
        if (surf.IsShadowing) {
            ThisBaseSurface = ThisSurf;
            HeatTransSurf = true;
        }

        // IF (Surface(ThisSurf)%Name(1:3) /= 'Mir') THEN
        if (!surf.MirroredSurf) {
            bool IsCoPlanar;
            int LastVertexInError;
            Vectors::CalcCoPlanarNess(surf.Vertex, surf.Sides, IsCoPlanar, OutOfLine, LastVertexInError);
            if (!IsCoPlanar) {
                if (OutOfLine > 0.01) {
                    ShowSevereError(state,
                                    format("{}Suspected non-planar surface:\"{}\", Max \"out of line\"={:.5T} at Vertex # {}",
                                           RoutineName,
                                           surf.Name,
                                           OutOfLine,
                                           LastVertexInError));
                } else {
                    ShowWarningError(state,
                                     format("{}Possible non-planar surface:\"{}\", Max \"out of line\"={:.5T} at Vertex # {}",
                                            RoutineName,
                                            surf.Name,
                                            OutOfLine,
                                            LastVertexInError));
                }
                //       ErrorInSurface=.TRUE.
            }
        }

        if (BaseSurface) {
            SurfWorldAz = surf.Azimuth;
            SurfTilt = surf.Tilt;
            for (n = 1; n <= surf.Sides; ++n) {
                state.dataSurfaceGeometry->Xpsv(n) = surf.Vertex(n).x;
                state.dataSurfaceGeometry->Ypsv(n) = surf.Vertex(n).y;
                state.dataSurfaceGeometry->Zpsv(n) = surf.Vertex(n).z;
            }
            TVect = surf.Vertex(3) - surf.Vertex(2);
            ThisWidth = Vectors::VecLength(TVect);
            TVect = surf.Vertex(2) - surf.Vertex(1);
            ThisHeight = Vectors::VecLength(TVect);
            surf.Width = ThisWidth;
            surf.Height = ThisHeight; // For a horizontal surface this is actually length!
            if (surf.Sides == 3) {
                surf.Shape = DataSurfaces::SurfaceShape::Triangle;
            } else if (surf.Sides == 4) {
                // Test for rectangularity
                if (isRectangle(state, ThisSurf)) {
                    surf.Shape = DataSurfaces::SurfaceShape::Rectangle;
                } else {
                    surf.Shape = DataSurfaces::SurfaceShape::Quadrilateral;
                }
            } else { // Surface( ThisSurf ).Sides > 4
                surf.Shape = DataSurfaces::SurfaceShape::Polygonal;
                if (std::abs(ThisHeight * ThisWidth - surf.GrossArea) > 0.001) {
                    surf.Width = std::sqrt(surf.GrossArea);
                    surf.Height = surf.Width;
                    ThisWidth = surf.Width;
                    ThisHeight = surf.Height;
                }
            }

        } else { // It's a subsurface to previous basesurface in this set of calls

            ThisSurfAz = surf.Azimuth;
            ThisSurfTilt = surf.Tilt;

            // Retrieve base surface info
            Real64 const baseSurfWorldAz = state.dataSurface->Surface(ThisBaseSurface).Azimuth;
            Real64 const baseSurfTilt = state.dataSurface->Surface(ThisBaseSurface).Tilt;
            Real64 const BaseCosAzimuth = std::cos(baseSurfWorldAz * Constant::DegToRad);
            Real64 const BaseSinAzimuth = std::sin(baseSurfWorldAz * Constant::DegToRad);
            Real64 const BaseCosTilt = std::cos(baseSurfTilt * Constant::DegToRad);
            Real64 const BaseSinTilt = std::sin(baseSurfTilt * Constant::DegToRad);
            Real64 const BaseXLLC = state.dataSurface->Surface(ThisBaseSurface).Vertex(2).x;
            Real64 const BaseYLLC = state.dataSurface->Surface(ThisBaseSurface).Vertex(2).y;
            Real64 const BaseZLLC = state.dataSurface->Surface(ThisBaseSurface).Vertex(2).z;

            if (HeatTransSurf) {

                if (surf.Sides == 4) {
                    ThisShape = DataSurfaces::SurfaceShape::RectangularDoorWindow;
                } else if (surf.Sides == 3 && surf.Class == SurfaceClass::Window) {
                    ThisShape = DataSurfaces::SurfaceShape::TriangularWindow;
                } else if (surf.Sides == 3 && surf.Class == SurfaceClass::Door) {
                    ThisShape = DataSurfaces::SurfaceShape::TriangularDoor;
                } else {
                    assert(false);
                }

            } else { //  this is a shadowing subsurface

                if (std::abs(state.dataSurface->Surface(surf.BaseSurf).Tilt - ThisSurfTilt) <= 0.01) {
                    // left or right fin
                    if (ThisSurfAz < 0.0) {
                        ThisSurfAz += 360.0;
                    }
                    if (ThisSurfAz > state.dataSurface->Surface(surf.BaseSurf).Azimuth) {
                        ThisShape = DataSurfaces::SurfaceShape::RectangularLeftFin;
                    } else {
                        ThisShape = DataSurfaces::SurfaceShape::RectangularRightFin;
                    }
                } else {
                    ThisShape = DataSurfaces::SurfaceShape::RectangularOverhang;
                }
            }

            // Setting relative coordinates for shadowing calculations for subsurfaces
            bool SError; // Bool used for return value of calls to PlaneEquation
            switch (ThisShape) {
            case DataSurfaces::SurfaceShape::RectangularDoorWindow: { // Rectangular heat transfer subsurface
                Vectors::PlaneEquation(
                    state.dataSurface->Surface(surf.BaseSurf).Vertex, state.dataSurface->Surface(surf.BaseSurf).Sides, BasePlane, SError);
                if (SError) {
                    ShowSevereError(state, format("{}Degenerate surface (likely two vertices equal):\"{}\".", RoutineName, surf.Name));
                    ErrorInSurface = true;
                }
                ThisReveal = -Vectors::Pt2Plane(surf.Vertex(2), BasePlane);
                if (std::abs(ThisReveal) < 0.0002) {
                    ThisReveal = 0.0;
                }
                surf.Reveal = ThisReveal;
                Xp = surf.Vertex(2).x - BaseXLLC;
                Yp = surf.Vertex(2).y - BaseYLLC;
                Zp = surf.Vertex(2).z - BaseZLLC;
                XLLC = -Xp * BaseCosAzimuth + Yp * BaseSinAzimuth;
                YLLC = -Xp * BaseSinAzimuth * BaseCosTilt - Yp * BaseCosAzimuth * BaseCosTilt + Zp * BaseSinTilt;
                ZLLC = Xp * BaseSinAzimuth * BaseSinTilt + Yp * BaseCosAzimuth * BaseSinTilt + Zp * BaseCosTilt;
                TVect = surf.Vertex(3) - surf.Vertex(2);
                ThisWidth = Vectors::VecLength(TVect);
                TVect = surf.Vertex(2) - surf.Vertex(1);
                ThisHeight = Vectors::VecLength(TVect);
                surf.Width = ThisWidth;
                surf.Height = ThisHeight;

                // Processing of 4-sided but non-rectangular Window, Door or GlassDoor, for use in calc of convective air flow.
                if (!isRectangle(state, ThisSurf)) {

                    // Transform the surface into an equivalent rectangular surface with the same area and aspect ratio.
                    MakeEquivalentRectangle(state, ThisSurf, ErrorsFound);

                    if (state.dataGlobal->DisplayExtraWarnings) {
                        ShowWarningError(state, format("{}Suspected 4-sided but non-rectangular Window, Door or GlassDoor:", RoutineName));
                        ShowContinueError(
                            state,
                            format("Surface={} is transformed into an equivalent rectangular surface with the same area and aspect ratio. ",
                                   surf.Name));
                    }
                }

                state.dataSurfaceGeometry->Xpsv(1) = XLLC;
                state.dataSurfaceGeometry->Xpsv(2) = XLLC;
                state.dataSurfaceGeometry->Xpsv(3) = XLLC + surf.Width;
                state.dataSurfaceGeometry->Xpsv(4) = XLLC + surf.Width;
                state.dataSurfaceGeometry->Ypsv(1) = YLLC + surf.Height;
                state.dataSurfaceGeometry->Ypsv(4) = YLLC + surf.Height;
                state.dataSurfaceGeometry->Ypsv(2) = YLLC;
                state.dataSurfaceGeometry->Ypsv(3) = YLLC;
                state.dataSurfaceGeometry->Zpsv(1) = ZLLC;
                state.dataSurfaceGeometry->Zpsv(2) = ZLLC;
                state.dataSurfaceGeometry->Zpsv(3) = ZLLC;
                state.dataSurfaceGeometry->Zpsv(4) = ZLLC;

                if (surf.Class == SurfaceClass::Window && surf.ExtBoundCond == DataSurfaces::ExternalEnvironment && surf.FrameDivider > 0) {
                    int FrDivNum = surf.FrameDivider;
                    // Set flag for calculating beam solar reflection from outside and/or inside window reveal
                    if ((surf.Reveal > 0.0 && state.dataSurface->FrameDivider(FrDivNum).OutsideRevealSolAbs > 0.0) ||
                        (state.dataSurface->FrameDivider(FrDivNum).InsideSillDepth > 0.0 &&
                         state.dataSurface->FrameDivider(FrDivNum).InsideSillSolAbs > 0.0) ||
                        (state.dataSurface->FrameDivider(FrDivNum).InsideReveal > 0.0 &&
                         state.dataSurface->FrameDivider(FrDivNum).InsideRevealSolAbs > 0.0)) {
                        state.dataHeatBal->CalcWindowRevealReflection = true;
                    }

                    // For exterior window with frame, subtract frame area from base surface
                    // (only rectangular windows are allowed to have a frame and/or divider;
                    // Surface(ThisSurf)%FrameDivider will be 0 for triangular windows)
                    FrWidth = state.dataSurface->FrameDivider(FrDivNum).FrameWidth;
                    if (FrWidth > 0.0) {
                        FrArea = (surf.Height + 2.0 * FrWidth) * (surf.Width + 2.0 * FrWidth) - surf.Area / surf.Multiplier;
                        state.dataSurface->SurfWinFrameArea(ThisSurf) = FrArea * surf.Multiplier;
                        if ((state.dataSurface->Surface(surf.BaseSurf).Area - state.dataSurface->SurfWinFrameArea(ThisSurf)) <= 0.0) {
                            ShowSevereError(state, format("{}Base Surface=\"{}\", ", RoutineName, state.dataSurface->Surface(surf.BaseSurf).Name));
                            ShowContinueError(state,
                                              format("Window Surface=\"{}\" area (with frame) is too large to fit on the surface.", surf.Name));
                            ShowContinueError(state,
                                              format("Base surface area (-windows and doors)=[{:.2T}] m2, frame area=[{:.2T}] m2.",
                                                     state.dataSurface->Surface(surf.BaseSurf).Area,
                                                     state.dataSurface->SurfWinFrameArea(ThisSurf)));
                            ErrorInSurface = true;
                        }
                        state.dataSurface->Surface(surf.BaseSurf).Area -= state.dataSurface->SurfWinFrameArea(ThisSurf);
                    }
                    // If exterior window has divider, subtract divider area to get glazed area
                    DivWidth = state.dataSurface->FrameDivider(surf.FrameDivider).DividerWidth;
                    if (DivWidth > 0.0 && !ErrorInSurface) {
                        DivArea = DivWidth * (state.dataSurface->FrameDivider(FrDivNum).HorDividers * surf.Width +
                                              state.dataSurface->FrameDivider(FrDivNum).VertDividers * surf.Height -
                                              state.dataSurface->FrameDivider(FrDivNum).HorDividers *
                                                  state.dataSurface->FrameDivider(FrDivNum).VertDividers * DivWidth);
                        state.dataSurface->SurfWinDividerArea(ThisSurf) = DivArea * surf.Multiplier;
                        if ((surf.Area - state.dataSurface->SurfWinDividerArea(ThisSurf)) <= 0.0) {
                            ShowSevereError(state, format("{}Divider area exceeds glazed opening for window {}", RoutineName, surf.Name));
                            ShowContinueError(state,
                                              format("Window surface area=[{:.2T}] m2, divider area=[{:.2T}] m2.",
                                                     surf.Area,
                                                     state.dataSurface->SurfWinDividerArea(ThisSurf)));
                            ErrorInSurface = true;
                        }
                        surf.Area -= state.dataSurface->SurfWinDividerArea(ThisSurf); // Glazed area
                        if (DivArea <= 0.0) {
                            ShowWarningError(state, format("{}Calculated Divider Area <= 0.0 for Window={}", RoutineName, surf.Name));
                            if (state.dataSurface->FrameDivider(FrDivNum).HorDividers == 0) {
                                ShowContinueError(state, "..Number of Horizontal Dividers = 0.");
                            }
                            if (state.dataSurface->FrameDivider(FrDivNum).VertDividers == 0) {
                                ShowContinueError(state, "..Number of Vertical Dividers = 0.");
                            }
                        } else {
                            auto &surfWin = state.dataSurface->SurfaceWindow(ThisSurf);
                            surfWin.glazedFrac = surf.Area / (surf.Area + state.dataSurface->SurfWinDividerArea(ThisSurf));
                            // Correction factor for portion of divider subject to divider projection correction
                            DivFrac = (1.0 - state.dataSurface->FrameDivider(FrDivNum).HorDividers *
                                                 state.dataSurface->FrameDivider(FrDivNum).VertDividers * pow_2(DivWidth) / DivArea);
                            state.dataSurface->SurfWinProjCorrDivOut(ThisSurf) =
                                DivFrac * state.dataSurface->FrameDivider(FrDivNum).DividerProjectionOut / DivWidth;
                            state.dataSurface->SurfWinProjCorrDivIn(ThisSurf) =
                                DivFrac * state.dataSurface->FrameDivider(FrDivNum).DividerProjectionIn / DivWidth;
                            // Correction factor for portion of frame subject to frame projection correction
                            if (FrWidth > 0.0) {
                                state.dataSurface->SurfWinProjCorrFrOut(ThisSurf) =
                                    (state.dataSurface->FrameDivider(FrDivNum).FrameProjectionOut / FrWidth) *
                                    (ThisHeight + ThisWidth -
                                     (state.dataSurface->FrameDivider(FrDivNum).HorDividers +
                                      state.dataSurface->FrameDivider(FrDivNum).VertDividers) *
                                         DivWidth) /
                                    (ThisHeight + ThisWidth + 2 * FrWidth);
                                state.dataSurface->SurfWinProjCorrFrIn(ThisSurf) =
                                    (state.dataSurface->FrameDivider(FrDivNum).FrameProjectionIn / FrWidth) *
                                    (ThisHeight + ThisWidth -
                                     (state.dataSurface->FrameDivider(FrDivNum).HorDividers +
                                      state.dataSurface->FrameDivider(FrDivNum).VertDividers) *
                                         DivWidth) /
                                    (ThisHeight + ThisWidth + 2 * FrWidth);
                            }
                        }
                    }
                }
            } break;
            case DataSurfaces::SurfaceShape::TriangularWindow:
            case DataSurfaces::SurfaceShape::TriangularDoor: {
                Vectors::PlaneEquation(
                    state.dataSurface->Surface(surf.BaseSurf).Vertex, state.dataSurface->Surface(surf.BaseSurf).Sides, BasePlane, SError);
                if (SError) {
                    ShowSevereError(state, format("{}Degenerate surface (likely two vertices equal):\"{}\".", RoutineName, surf.Name));
                    ErrorInSurface = true;
                }
                ThisReveal = -Vectors::Pt2Plane(surf.Vertex(2), BasePlane);
                if (std::abs(ThisReveal) < 0.0002) {
                    ThisReveal = 0.0;
                }
                surf.Reveal = ThisReveal;
                Xp = surf.Vertex(2).x - BaseXLLC;
                Yp = surf.Vertex(2).y - BaseYLLC;
                Zp = surf.Vertex(2).z - BaseZLLC;
                state.dataSurfaceGeometry->Xpsv(2) = -Xp * BaseCosAzimuth + Yp * BaseSinAzimuth;
                state.dataSurfaceGeometry->Ypsv(2) = -Xp * BaseSinAzimuth * BaseCosTilt - Yp * BaseCosAzimuth * BaseCosTilt + Zp * BaseSinTilt;
                state.dataSurfaceGeometry->Zpsv(2) = Xp * BaseSinAzimuth * BaseSinTilt + Yp * BaseCosAzimuth * BaseSinTilt + Zp * BaseCosTilt;
                TVect = surf.Vertex(3) - surf.Vertex(2);
                ThisWidth = Vectors::VecLength(TVect);
                TVect = surf.Vertex(2) - surf.Vertex(1);
                ThisHeight = Vectors::VecLength(TVect);
                surf.Width = ThisWidth;
                // Effective height and width of a triangular window for use in calc of convective air flow
                // in gap between glass and shading device when shading device is present
                surf.Height = 4.0 * surf.Area / (3.0 * surf.Width);
                surf.Width *= 0.75;

                Xp = surf.Vertex(1).x - BaseXLLC;
                Yp = surf.Vertex(1).y - BaseYLLC;
                Zp = surf.Vertex(1).z - BaseZLLC;
                state.dataSurfaceGeometry->Xpsv(1) = -Xp * BaseCosAzimuth + Yp * BaseSinAzimuth;
                state.dataSurfaceGeometry->Ypsv(1) = -Xp * BaseSinAzimuth * BaseCosTilt - Yp * BaseCosAzimuth * BaseCosTilt + Zp * BaseSinTilt;
                state.dataSurfaceGeometry->Zpsv(1) = Xp * BaseSinAzimuth * BaseSinTilt + Yp * BaseCosAzimuth * BaseSinTilt + Zp * BaseCosTilt;

                Xp = surf.Vertex(3).x - BaseXLLC;
                Yp = surf.Vertex(3).y - BaseYLLC;
                Zp = surf.Vertex(3).z - BaseZLLC;
                state.dataSurfaceGeometry->Xpsv(3) = -Xp * BaseCosAzimuth + Yp * BaseSinAzimuth;
                state.dataSurfaceGeometry->Ypsv(3) = -Xp * BaseSinAzimuth * BaseCosTilt - Yp * BaseCosAzimuth * BaseCosTilt + Zp * BaseSinTilt;
                state.dataSurfaceGeometry->Zpsv(3) = Xp * BaseSinAzimuth * BaseSinTilt + Yp * BaseCosAzimuth * BaseSinTilt + Zp * BaseCosTilt;
            } break;
            case DataSurfaces::SurfaceShape::RectangularOverhang: {
                Xp = surf.Vertex(2).x - BaseXLLC;
                Yp = surf.Vertex(2).y - BaseYLLC;
                Zp = surf.Vertex(2).z - BaseZLLC;
                XLLC = -Xp * BaseCosAzimuth + Yp * BaseSinAzimuth;
                YLLC = -Xp * BaseSinAzimuth * BaseCosTilt - Yp * BaseCosAzimuth * BaseCosTilt + Zp * BaseSinTilt;
                ZLLC = Xp * BaseSinAzimuth * BaseSinTilt + Yp * BaseCosAzimuth * BaseSinTilt + Zp * BaseCosTilt;
                TVect = surf.Vertex(3) - surf.Vertex(2);
                ThisWidth = Vectors::VecLength(TVect);
                TVect = surf.Vertex(2) - surf.Vertex(1);
                ThisHeight = Vectors::VecLength(TVect);
                surf.Width = ThisWidth;
                surf.Height = ThisHeight;
                state.dataSurfaceGeometry->Xpsv(1) = XLLC;
                state.dataSurfaceGeometry->Xpsv(2) = XLLC;
                state.dataSurfaceGeometry->Xpsv(3) = XLLC + surf.Width;
                state.dataSurfaceGeometry->Xpsv(4) = XLLC + surf.Width;
                state.dataSurfaceGeometry->Ypsv(1) = YLLC;
                state.dataSurfaceGeometry->Ypsv(2) = YLLC;
                state.dataSurfaceGeometry->Ypsv(3) = YLLC;
                state.dataSurfaceGeometry->Ypsv(4) = YLLC;
                state.dataSurfaceGeometry->Zpsv(1) = surf.Height;
                state.dataSurfaceGeometry->Zpsv(4) = surf.Height;
                state.dataSurfaceGeometry->Zpsv(2) = 0.0;
                state.dataSurfaceGeometry->Zpsv(3) = 0.0;
            } break;
            case DataSurfaces::SurfaceShape::RectangularLeftFin: {
                Xp = surf.Vertex(2).x - BaseXLLC;
                Yp = surf.Vertex(2).y - BaseYLLC;
                Zp = surf.Vertex(2).z - BaseZLLC;
                XLLC = -Xp * BaseCosAzimuth + Yp * BaseSinAzimuth;
                YLLC = -Xp * BaseSinAzimuth * BaseCosTilt - Yp * BaseCosAzimuth * BaseCosTilt + Zp * BaseSinTilt;
                ZLLC = Xp * BaseSinAzimuth * BaseSinTilt + Yp * BaseCosAzimuth * BaseSinTilt + Zp * BaseCosTilt;
                TVect = surf.Vertex(3) - surf.Vertex(2);
                ThisWidth = Vectors::VecLength(TVect);
                TVect = surf.Vertex(2) - surf.Vertex(1);
                ThisHeight = Vectors::VecLength(TVect);
                surf.Width = ThisWidth;
                surf.Height = ThisHeight;
                state.dataSurfaceGeometry->Xpsv(1) = XLLC;
                state.dataSurfaceGeometry->Xpsv(2) = XLLC;
                state.dataSurfaceGeometry->Xpsv(3) = XLLC;
                state.dataSurfaceGeometry->Xpsv(4) = XLLC;
                state.dataSurfaceGeometry->Ypsv(1) = YLLC;
                state.dataSurfaceGeometry->Ypsv(2) = YLLC;
                state.dataSurfaceGeometry->Ypsv(3) = YLLC + surf.Width;
                state.dataSurfaceGeometry->Ypsv(4) = YLLC + surf.Width;
                state.dataSurfaceGeometry->Zpsv(1) = surf.Height;
                state.dataSurfaceGeometry->Zpsv(4) = surf.Height;
                state.dataSurfaceGeometry->Zpsv(2) = 0.0;
                state.dataSurfaceGeometry->Zpsv(3) = 0.0;
            } break;
            case DataSurfaces::SurfaceShape::RectangularRightFin: {
                Xp = surf.Vertex(2).x - BaseXLLC;
                Yp = surf.Vertex(2).y - BaseYLLC;
                Zp = surf.Vertex(2).z - BaseZLLC;
                XLLC = -Xp * BaseCosAzimuth + Yp * BaseSinAzimuth;
                YLLC = -Xp * BaseSinAzimuth * BaseCosTilt - Yp * BaseCosAzimuth * BaseCosTilt + Zp * BaseSinTilt;
                ZLLC = Xp * BaseSinAzimuth * BaseSinTilt + Yp * BaseCosAzimuth * BaseSinTilt + Zp * BaseCosTilt;
                TVect = surf.Vertex(3) - surf.Vertex(2);
                ThisWidth = Vectors::VecLength(TVect);
                TVect = surf.Vertex(2) - surf.Vertex(1);
                ThisHeight = Vectors::VecLength(TVect);
                surf.Width = ThisWidth;
                surf.Height = ThisHeight;
                state.dataSurfaceGeometry->Xpsv(1) = XLLC;
                state.dataSurfaceGeometry->Xpsv(2) = XLLC;
                state.dataSurfaceGeometry->Xpsv(3) = XLLC;
                state.dataSurfaceGeometry->Xpsv(4) = XLLC;
                state.dataSurfaceGeometry->Ypsv(1) = YLLC + surf.Width;
                state.dataSurfaceGeometry->Ypsv(2) = YLLC + surf.Width;
                state.dataSurfaceGeometry->Ypsv(3) = YLLC;
                state.dataSurfaceGeometry->Ypsv(4) = YLLC;
                state.dataSurfaceGeometry->Zpsv(1) = surf.Height;
                state.dataSurfaceGeometry->Zpsv(4) = surf.Height;
                state.dataSurfaceGeometry->Zpsv(2) = 0.0;
                state.dataSurfaceGeometry->Zpsv(3) = 0.0;
            } break;
            default: {
                // Error Condition
                ShowSevereError(state, format("{}Incorrect surface shape number.", RoutineName), OptionalOutputFileRef{state.files.eso});
                ShowContinueError(state, "Please notify EnergyPlus support of this error and send input file.");
                ErrorInSurface = true;
            } break;
            }

            for (n = 1; n <= surf.Sides; ++n) {
                // if less than 1/10 inch
                state.dataSurfaceGeometry->Xpsv(n) = nint64(10000.0 * state.dataSurfaceGeometry->Xpsv(n)) / 10000.0;
                if (std::abs(state.dataSurfaceGeometry->Xpsv(n)) < 0.0025) {
                    state.dataSurfaceGeometry->Xpsv(n) = 0.0;
                }
                state.dataSurfaceGeometry->Ypsv(n) = nint64(10000.0 * state.dataSurfaceGeometry->Ypsv(n)) / 10000.0;
                if (std::abs(state.dataSurfaceGeometry->Ypsv(n)) < 0.0025) {
                    state.dataSurfaceGeometry->Ypsv(n) = 0.0;
                }
                state.dataSurfaceGeometry->Zpsv(n) = nint64(10000.0 * state.dataSurfaceGeometry->Zpsv(n)) / 10000.0;
                if (std::abs(state.dataSurfaceGeometry->Zpsv(n)) < 0.0025) {
                    state.dataSurfaceGeometry->Zpsv(n) = 0.0;
                }
            }

            surf.Shape = ThisShape;

        } // End of check if ThisSurf is a base surface

        if (ErrorInSurface) {
            ErrorsFound = true;
            return;
        }

        // Transfer to XV,YV,ZV arrays

        state.dataSurface->ShadeV(ThisSurf).NVert = surf.Sides;
        state.dataSurface->ShadeV(ThisSurf).XV.allocate(surf.Sides);
        state.dataSurface->ShadeV(ThisSurf).YV.allocate(surf.Sides);
        state.dataSurface->ShadeV(ThisSurf).ZV.allocate(surf.Sides);

        for (n = 1; n <= surf.Sides; ++n) {
            // if less than 1/10 inch
            state.dataSurface->ShadeV(ThisSurf).XV(n) = state.dataSurfaceGeometry->Xpsv(n);
            state.dataSurface->ShadeV(ThisSurf).YV(n) = state.dataSurfaceGeometry->Ypsv(n);
            state.dataSurface->ShadeV(ThisSurf).ZV(n) = state.dataSurfaceGeometry->Zpsv(n);
        }

        // Process Surfaces According to Type of Coordinate Origin.
        if (BaseSurface) {

            // General Surfaces:
            CalcCoordinateTransformation(state, ThisSurf, CoordinateTransVector); // X00,Y00,Z00,X,Y,Z,A)    ! Compute Coordinate Transformation

            // RECORD DIRECTION COSINES.
            if (HeatTransSurf) { // This is a general surface but not detached shading surface

                // RECORD COORDINATE TRANSFORMATION FOR BASE SURFACES.
                state.dataSurface->X0(ThisBaseSurface) = CoordinateTransVector.x;
                state.dataSurface->Y0(ThisBaseSurface) = CoordinateTransVector.y;
                state.dataSurface->Z0(ThisBaseSurface) = CoordinateTransVector.z;

                // COMPUTE INVERSE TRANSFORMATION.
                X1 = state.dataSurfaceGeometry->Xpsv(2) - CoordinateTransVector.x;
                Y1 = state.dataSurfaceGeometry->Ypsv(2) - CoordinateTransVector.y;
                Z1 = state.dataSurfaceGeometry->Zpsv(2) - CoordinateTransVector.z;
                // Store the relative coordinate shift values for later use by any subsurfaces
                state.dataSurface->Surface(ThisBaseSurface).XShift = state.dataSurface->Surface(ThisBaseSurface).lcsx.x * X1 +
                                                                     state.dataSurface->Surface(ThisBaseSurface).lcsx.y * Y1 +
                                                                     state.dataSurface->Surface(ThisBaseSurface).lcsx.z * Z1;
                state.dataSurface->Surface(ThisBaseSurface).YShift = state.dataSurface->Surface(ThisBaseSurface).lcsy.x * X1 +
                                                                     state.dataSurface->Surface(ThisBaseSurface).lcsy.y * Y1 +
                                                                     state.dataSurface->Surface(ThisBaseSurface).lcsy.z * Z1;
                state.dataSurface->Surface(ThisBaseSurface).VerticesProcessed = true;
            }

            // SUBSURFACES: (Surface(ThisSurf)%BaseSurf /= ThisSurf)
        } else {
            // WINDOWS OR DOORS:

            // SHIFT RELATIVE COORDINATES FROM LOWER LEFT CORNER TO ORIGIN DEFINED
            // BY CTRAN AND SET DIRECTION COSINES SAME AS BASE SURFACE.
            if (!state.dataSurface->Surface(ThisBaseSurface).VerticesProcessed) {

                if (surf.IsAirBoundarySurf) {
                    ProcessSurfaceVertices(state, ThisBaseSurface, ErrorsFound);
                } else {

                    ShowSevereError(state, format("{}Developer error for Subsurface={}", RoutineName, surf.Name));
                    ShowContinueError(state,
                                      format("Base surface={} vertices must be processed before any subsurfaces.",
                                             state.dataSurface->Surface(ThisBaseSurface).Name));
                    ShowFatalError(state, std::string{RoutineName});
                }
            }

            for (n = 1; n <= surf.Sides; ++n) {
                state.dataSurface->ShadeV(ThisSurf).XV(n) += state.dataSurface->Surface(ThisBaseSurface).XShift;
                state.dataSurface->ShadeV(ThisSurf).YV(n) += state.dataSurface->Surface(ThisBaseSurface).YShift;
            }
        }
    }

    void CalcCoordinateTransformation(EnergyPlusData &state,
                                      int const SurfNum,            // Surface Number
                                      Vector &CompCoordTranslVector // Coordinate Translation Vector
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton, BLAST
        //       DATE WRITTEN   August 1976
        //       MODIFIED       LKL, May 2004 -- >4 sided polygons
        //       RE-ENGINEERED  Yes

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine develops a coordinate transformation such that the X-axis goes
        // through points 2 and 3 and the Y-axis goes through point 1
        // of a plane figure in 3-d space.

        // REFERENCES:
        // 'NECAP' - NASA'S Energy-Cost Analysis Program

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 Gamma; // Intermediate Result
        Real64 DotSelfX23;

        // Object Data
        Vector x21;
        Vector x23;

        // Determine Components of the Coordinate Translation Vector.
        auto const &surf = state.dataSurface->Surface(SurfNum);

        x21 = surf.Vertex(2) - surf.Vertex(1);
        x23 = surf.Vertex(2) - surf.Vertex(3);

        DotSelfX23 = magnitude_squared(x23);

        if (DotSelfX23 <= Constant::OneMillionth) {
            ShowSevereError(state, format("CalcCoordinateTransformation: Invalid dot product, surface=\"{}\":", surf.Name));
            for (int I = 1; I <= surf.Sides; ++I) {
                auto const &point = surf.Vertex(I);
                ShowContinueError(state, format(" ({:8.3F},{:8.3F},{:8.3F})", point.x, point.y, point.z));
            }
            ShowFatalError(
                state, "CalcCoordinateTransformation: Program terminates due to preceding condition.", OptionalOutputFileRef{state.files.eso});
            return;
        }

        Gamma = dot(x21, x23) / magnitude_squared(x23);

        CompCoordTranslVector = surf.Vertex(2) + Gamma * (surf.Vertex(3) - surf.Vertex(2));
    }

    void CreateShadedWindowConstruction(EnergyPlusData &state,
                                        int const SurfNum,          // Surface number
                                        int const WSCPtr,           // Pointer to WindowShadingControl for SurfNum
                                        int const ShDevNum,         // Shading device material number for WSCptr
                                        int const shadeControlIndex // index to the Surface().windowShadingControlList,
                                                                    // Surface().shadedConstructionList, and Surface().shadedStormWinConstructionList
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   Nov 2001

        // PURPOSE OF THIS SUBROUTINE:
        // Creates a shaded window construction for windows whose WindowShadingControl
        // has a shading device specified instead of a shaded construction

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ConstrNewSh;          // Number of shaded construction that is created
        std::string ConstrNameSh; // Shaded construction name

        auto &s_mat = state.dataMaterial;
        auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);

        std::string const &ShDevName = s_mat->materials(ShDevNum)->Name;
        int ConstrNum = surfTemp.Construction;
        std::string const &ConstrName = state.dataConstruction->Construct(ConstrNum).Name;

        if (ANY_INTERIOR_SHADE_BLIND(state.dataSurface->WindowShadingControl(WSCPtr).ShadingType)) {
            ConstrNameSh = ConstrName + ':' + ShDevName + ":INT";
        } else {
            ConstrNameSh = ConstrName + ':' + ShDevName + ":EXT";
        }

        // If this construction name already exists, set the surface's shaded construction number to it

        ConstrNewSh = Util::FindItemInList(ConstrNameSh, state.dataConstruction->Construct);

        if (ConstrNewSh > 0) {
            surfTemp.shadedConstructionList[shadeControlIndex] = ConstrNewSh;
            surfTemp.activeShadedConstruction = ConstrNewSh; // set the active to the current for now
        } else {

            // Create new construction

            ConstrNewSh = state.dataHeatBal->TotConstructs + 1;
            surfTemp.shadedConstructionList[shadeControlIndex] = ConstrNewSh;
            surfTemp.activeShadedConstruction = ConstrNewSh; // set the active to the current for now
            state.dataHeatBal->TotConstructs = ConstrNewSh;
            state.dataConstruction->Construct.redimension(state.dataHeatBal->TotConstructs);
            state.dataHeatBal->NominalRforNominalUCalculation.redimension(state.dataHeatBal->TotConstructs);
            state.dataHeatBal->NominalRforNominalUCalculation(state.dataHeatBal->TotConstructs) = 0.0;
            state.dataHeatBal->NominalU.redimension(state.dataHeatBal->TotConstructs);
            state.dataHeatBal->NominalU(state.dataHeatBal->TotConstructs) = 0.0;
            state.dataHeatBal->NominalUBeforeAdjusted.redimension(state.dataHeatBal->TotConstructs);
            state.dataHeatBal->CoeffAdjRatio.redimension(state.dataHeatBal->TotConstructs) = 1.0;

            state.dataConstruction->Construct(state.dataHeatBal->TotConstructs).setArraysBasedOnMaxSolidWinLayers(state);

            int TotLayersOld = state.dataConstruction->Construct(ConstrNum).TotLayers;
            int TotLayersNew = TotLayersOld + 1;

            state.dataConstruction->Construct(ConstrNewSh).LayerPoint = 0;

            auto const *thisMaterialSh = s_mat->materials(ShDevNum);
            auto &thisConstructNewSh = state.dataConstruction->Construct(ConstrNewSh);
            if (state.dataSurface->WindowShadingControl(WSCPtr).ShadingType == DataSurfaces::WinShadingType::IntShade ||
                state.dataSurface->WindowShadingControl(WSCPtr).ShadingType == DataSurfaces::WinShadingType::IntBlind) {
                // Interior shading device
                thisConstructNewSh.LayerPoint({1, TotLayersOld}) = state.dataConstruction->Construct(ConstrNum).LayerPoint({1, TotLayersOld});
                thisConstructNewSh.LayerPoint(TotLayersNew) = ShDevNum;
                thisConstructNewSh.InsideAbsorpSolar = thisMaterialSh->AbsorpSolar;
                auto const *thisMaterialShLayer1 = s_mat->materials(state.dataConstruction->Construct(ConstrNewSh).LayerPoint(1));
                thisConstructNewSh.OutsideAbsorpSolar = thisMaterialShLayer1->AbsorpSolar;
                thisConstructNewSh.OutsideAbsorpThermal = thisMaterialShLayer1->AbsorpThermalFront;
            } else {
                // Exterior shading device
                thisConstructNewSh.LayerPoint(1) = ShDevNum;
                thisConstructNewSh.LayerPoint({2, TotLayersNew}) = state.dataConstruction->Construct(ConstrNum).LayerPoint({1, TotLayersOld});
                auto const *thisMaterialShInside = s_mat->materials(state.dataConstruction->Construct(ConstrNewSh).LayerPoint(TotLayersNew));
                thisConstructNewSh.InsideAbsorpSolar = thisMaterialShInside->AbsorpSolar;
                thisConstructNewSh.OutsideAbsorpSolar = thisMaterialSh->AbsorpSolar;
                thisConstructNewSh.OutsideAbsorpThermal = thisMaterialSh->AbsorpThermalFront;
            }
            // The following InsideAbsorpThermal applies only to inside glass; it is corrected
            //  later in InitGlassOpticalCalculations if construction has inside shade or blind.
            thisConstructNewSh.InsideAbsorpThermal =
                s_mat->materials(state.dataConstruction->Construct(ConstrNum).LayerPoint(TotLayersOld))->AbsorpThermalBack;
            thisConstructNewSh.OutsideRoughness = Material::SurfaceRoughness::VerySmooth;
            thisConstructNewSh.DayltPropPtr = 0;
            thisConstructNewSh.CTFCross.fill(0.0);
            thisConstructNewSh.CTFFlux.fill(0.0);
            thisConstructNewSh.CTFInside.fill(0.0);
            thisConstructNewSh.CTFOutside.fill(0.0);
            thisConstructNewSh.CTFSourceIn.fill(0.0);
            thisConstructNewSh.CTFSourceOut.fill(0.0);
            thisConstructNewSh.CTFTimeStep = 0.0;
            thisConstructNewSh.CTFTSourceOut.fill(0.0);
            thisConstructNewSh.CTFTSourceIn.fill(0.0);
            thisConstructNewSh.CTFTSourceQ.fill(0.0);
            thisConstructNewSh.CTFTUserOut.fill(0.0);
            thisConstructNewSh.CTFTUserIn.fill(0.0);
            thisConstructNewSh.CTFTUserSource.fill(0.0);
            thisConstructNewSh.NumHistories = 0;
            thisConstructNewSh.NumCTFTerms = 0;
            thisConstructNewSh.UValue = 0.0;
            thisConstructNewSh.SourceSinkPresent = false;
            thisConstructNewSh.SolutionDimensions = 0;
            thisConstructNewSh.SourceAfterLayer = 0;
            thisConstructNewSh.TempAfterLayer = 0;
            thisConstructNewSh.ThicknessPerpend = 0.0;
            thisConstructNewSh.AbsDiff = 0.0;
            thisConstructNewSh.AbsDiffBack = 0.0;
            thisConstructNewSh.AbsDiffShade = 0.0;
            thisConstructNewSh.AbsDiffBackShade = 0.0;
            thisConstructNewSh.ShadeAbsorpThermal = 0.0;
            std::fill(thisConstructNewSh.AbsBeamShadeCoef.begin(), thisConstructNewSh.AbsBeamShadeCoef.end(), 0.0);
            thisConstructNewSh.TransDiff = 0.0;
            thisConstructNewSh.TransDiffVis = 0.0;
            thisConstructNewSh.ReflectSolDiffBack = 0.0;
            thisConstructNewSh.ReflectSolDiffFront = 0.0;
            thisConstructNewSh.ReflectVisDiffBack = 0.0;
            thisConstructNewSh.ReflectVisDiffFront = 0.0;
            std::fill(thisConstructNewSh.TransSolBeamCoef.begin(), thisConstructNewSh.TransSolBeamCoef.end(), 0.0);
            std::fill(thisConstructNewSh.TransVisBeamCoef.begin(), thisConstructNewSh.TransVisBeamCoef.end(), 0.0);
            std::fill(thisConstructNewSh.ReflSolBeamFrontCoef.begin(), thisConstructNewSh.ReflSolBeamFrontCoef.end(), 0.0);
            std::fill(thisConstructNewSh.ReflSolBeamBackCoef.begin(), thisConstructNewSh.ReflSolBeamBackCoef.end(), 0.0);
            thisConstructNewSh.W5FrameDivider = 0;
            thisConstructNewSh.FromWindow5DataFile = false;

            thisConstructNewSh.Name = ConstrNameSh;
            thisConstructNewSh.TotLayers = TotLayersNew;
            thisConstructNewSh.TotSolidLayers = state.dataConstruction->Construct(ConstrNum).TotSolidLayers + 1;
            thisConstructNewSh.TotGlassLayers = state.dataConstruction->Construct(ConstrNum).TotGlassLayers;
            thisConstructNewSh.TypeIsWindow = true;
            thisConstructNewSh.IsUsed = true;

            for (int Layer = 1; Layer <= state.dataHeatBal->MaxSolidWinLayers; ++Layer) {
                std::fill(thisConstructNewSh.AbsBeamCoef(Layer).begin(), thisConstructNewSh.AbsBeamCoef(Layer).end(), 0.0);
                std::fill(thisConstructNewSh.AbsBeamBackCoef(Layer).begin(), thisConstructNewSh.AbsBeamBackCoef(Layer).end(), 0.0);
            }
        }
    }

    void CreateStormWindowConstructions(EnergyPlusData &state)
    {
        // For windows with an associated StormWindow object, creates a construction
        // consisting of the base construction plus a storm window and air gap on the outside.
        // If the window has an interior or between-glass shade/blind, also creates a
        // construction consisting of the storm window added to the shaded construction.
        DisplayString(state, "Creating Storm Window Constructions");

        auto &s_mat = state.dataMaterial;

        for (int StormWinNum = 1; StormWinNum <= state.dataSurface->TotStormWin; ++StormWinNum) {
            int SurfNum = state.dataSurface->StormWindow(StormWinNum).BaseWindowNum; // Surface number
            auto &surf = state.dataSurface->Surface(SurfNum);
            int ConstrNum = surf.Construction; // Number of unshaded construction
            // Fatal error if base construction has more than three glass layers
            if (state.dataConstruction->Construct(ConstrNum).TotGlassLayers > 3) {
                ShowFatalError(state, format("Window={} has more than 3 glass layers; a storm window cannot be applied.", surf.Name));
            }

            // create unshaded construction with storm window
            const std::string ChrNum = fmt::to_string(StormWinNum);
            std::string ConstrNameSt = "BARECONSTRUCTIONWITHSTORMWIN:" + ChrNum; // Name of unshaded construction with storm window
            // If this construction name already exists, set the surface's storm window construction number to it
            int ConstrNewSt = Util::FindItemInList(ConstrNameSt,
                                                   state.dataConstruction->Construct,
                                                   state.dataHeatBal->TotConstructs); // Number of unshaded storm window construction that is created
            // If necessary, create new material corresponding to the air layer between the storm window and the rest of the window
            int MatNewStAir = createAirMaterialFromDistance(state, state.dataSurface->StormWindow(StormWinNum).StormWinDistance, "AIR:STORMWIN:");
            if (ConstrNewSt == 0) {
                ConstrNewSt = createConstructionWithStorm(
                    state, ConstrNum, ConstrNameSt, state.dataSurface->StormWindow(StormWinNum).StormWinMaterialNum, MatNewStAir);
            }
            state.dataSurface->SurfWinStormWinConstr(SurfNum) = ConstrNewSt;

            // create shaded constructions with storm window
            surf.shadedStormWinConstructionList.resize(surf.shadedConstructionList.size(),
                                                       0); // make the shaded storm window size the same size as the number of shaded constructions
            for (std::size_t iConstruction = 0; iConstruction < surf.shadedConstructionList.size(); ++iConstruction) {
                int curConstruction = surf.shadedConstructionList[iConstruction];
                // Set ShAndSt, which is true if the window has a shaded construction to which a storm window
                // can be added. (A storm window can be added if there is an interior shade or blind and up to three
                // glass layers, or there is a between-glass shade or blind and two glass layers.)
                bool ShAndSt = false; // True if unshaded and shaded window can have a storm window
                int TotLayers = state.dataConstruction->Construct(curConstruction).TotLayers;            // Total layers in a construction
                int MatIntSh = state.dataConstruction->Construct(curConstruction).LayerPoint(TotLayers); // Material number of interior shade or blind
                int MatBetweenGlassSh = 0; // Material number of between-glass shade or blind
                if (TotLayers == 5) {
                    MatBetweenGlassSh = state.dataConstruction->Construct(curConstruction).LayerPoint(3);
                }
                if (state.dataConstruction->Construct(curConstruction).TotGlassLayers <= 3 &&
                    (s_mat->materials(MatIntSh)->group == Material::Group::Shade || s_mat->materials(MatIntSh)->group == Material::Group::Blind)) {
                    ShAndSt = true;
                }
                if (MatBetweenGlassSh > 0) {
                    if (s_mat->materials(MatBetweenGlassSh)->group == Material::Group::Shade ||
                        s_mat->materials(MatBetweenGlassSh)->group == Material::Group::Blind) {
                        ShAndSt = true;
                    } else {
                        ShowContinueError(state, format("Window={} has a shaded construction to which a storm window cannot be applied.", surf.Name));
                        ShowContinueError(state, "Storm windows can only be applied to shaded constructions that:");
                        ShowContinueError(state, "have an interior shade or blind and up to three glass layers, or");
                        ShowContinueError(state, "have a between-glass shade or blind and two glass layers.");
                        ShowFatalError(state, "EnergyPlus is exiting due to reason stated above.");
                    }
                }
                if (ShAndSt) {
                    std::string ConstrNameStSh = "SHADEDCONSTRUCTIONWITHSTORMWIN:" + state.dataConstruction->Construct(iConstruction).Name + ":" +
                                                 ChrNum; // Name of shaded construction with storm window
                    int ConstrNewStSh = createConstructionWithStorm(
                        state, ConstrNum, ConstrNameStSh, state.dataSurface->StormWindow(StormWinNum).StormWinMaterialNum, MatNewStAir);
                    surf.shadedStormWinConstructionList[iConstruction] = ConstrNewStSh; // put in same index as the shaded construction
                }
            } // end of loop for shaded constructions
        } // end of loop over storm window objects
    }

    int createAirMaterialFromDistance(EnergyPlusData &state, Real64 distance, std::string_view namePrefix)
    {
        auto const &s_mat = state.dataMaterial;

        int mmDistance = int(1000 * distance); // Thickness of air gap in mm (usually between storm window and rest of window)
        std::string MatNameStAir = format("{}{}MM", namePrefix, mmDistance); // Name of created air layer material
        int matNum = Material::GetMaterialNum(state, MatNameStAir);
        if (matNum != 0) {
            return matNum;
        }

        // Create new material
        auto *mat = new Material::MaterialGasMix;
        mat->Name = MatNameStAir;
        mat->group = Material::Group::Gas;

        s_mat->materials.push_back(mat);
        mat->Num = s_mat->materials.isize();
        s_mat->materialMap.insert_or_assign(Util::makeUPPER(mat->Name), mat->Num);

        mat->Roughness = Material::SurfaceRoughness::MediumRough;
        mat->Conductivity = 0.0;
        mat->Density = 0.0;
        mat->Resistance = 0.0;
        mat->SpecHeat = 0.0;
        mat->Thickness = distance;
        mat->numGases = 1;
        mat->gases[0] = Material::gases[(int)Material::GasType::Air];
        mat->gasFracts[0] = 1.0;
        mat->AbsorpSolar = 0.0;
        mat->AbsorpThermal = 0.0;
        mat->AbsorpVisible = 0.0;
        return mat->Num;
    }

    // create a new construction with storm based on an old construction and storm and gap materials
    int createConstructionWithStorm(EnergyPlusData &state, int oldConstruction, std::string name, int stormMaterial, int gapMaterial)
    {
        int newConstruct = Util::FindItemInList(name,
                                                state.dataConstruction->Construct,
                                                state.dataHeatBal->TotConstructs); // Number of shaded storm window construction that is created
        if (newConstruct == 0) {
            auto &s_mat = state.dataMaterial;
            state.dataHeatBal->TotConstructs = state.dataHeatBal->TotConstructs + 1;
            newConstruct = state.dataHeatBal->TotConstructs;
            state.dataConstruction->Construct.redimension(state.dataHeatBal->TotConstructs);
            state.dataHeatBal->NominalRforNominalUCalculation.redimension(state.dataHeatBal->TotConstructs);
            state.dataHeatBal->NominalU.redimension(state.dataHeatBal->TotConstructs);
            state.dataHeatBal->NominalUBeforeAdjusted.redimension(state.dataHeatBal->TotConstructs);
            state.dataHeatBal->CoeffAdjRatio.redimension(state.dataHeatBal->TotConstructs) = 1.0;

            auto &thisConstruct = state.dataConstruction->Construct(state.dataHeatBal->TotConstructs);
            // these Construct arrays dimensioned based on MaxSolidWinLayers
            thisConstruct.setArraysBasedOnMaxSolidWinLayers(state);

            int TotLayersOld = state.dataConstruction->Construct(oldConstruction).TotLayers;
            thisConstruct.LayerPoint({1, Construction::MaxLayersInConstruct}) = 0;
            thisConstruct.LayerPoint(1) = stormMaterial;
            thisConstruct.LayerPoint(2) = gapMaterial;
            thisConstruct.LayerPoint({3, TotLayersOld + 2}) = state.dataConstruction->Construct(oldConstruction).LayerPoint({1, TotLayersOld});
            thisConstruct.Name = name;
            thisConstruct.TotLayers = TotLayersOld + 2;
            thisConstruct.TotSolidLayers = state.dataConstruction->Construct(oldConstruction).TotSolidLayers + 1;
            thisConstruct.TotGlassLayers = state.dataConstruction->Construct(oldConstruction).TotGlassLayers + 1;
            thisConstruct.TypeIsWindow = true;
            thisConstruct.InsideAbsorpVis = 0.0;
            thisConstruct.OutsideAbsorpVis = 0.0;
            thisConstruct.InsideAbsorpSolar = 0.0;
            thisConstruct.OutsideAbsorpSolar = 0.0;
            thisConstruct.InsideAbsorpThermal = state.dataConstruction->Construct(oldConstruction).InsideAbsorpThermal;
            thisConstruct.OutsideAbsorpThermal = s_mat->materials(stormMaterial)->AbsorpThermalFront;
            thisConstruct.OutsideRoughness = Material::SurfaceRoughness::VerySmooth;
            thisConstruct.DayltPropPtr = 0;
            thisConstruct.CTFCross.fill(0.0);
            thisConstruct.CTFFlux.fill(0.0);
            thisConstruct.CTFInside.fill(0.0);
            thisConstruct.CTFOutside.fill(0.0);
            thisConstruct.CTFSourceIn.fill(0.0);
            thisConstruct.CTFSourceOut.fill(0.0);
            thisConstruct.CTFTimeStep = 0.0;
            thisConstruct.CTFTSourceOut.fill(0.0);
            thisConstruct.CTFTSourceIn.fill(0.0);
            thisConstruct.CTFTSourceQ.fill(0.0);
            thisConstruct.CTFTUserOut.fill(0.0);
            thisConstruct.CTFTUserIn.fill(0.0);
            thisConstruct.CTFTUserSource.fill(0.0);
            thisConstruct.NumHistories = 0;
            thisConstruct.NumCTFTerms = 0;
            thisConstruct.UValue = 0.0;
            thisConstruct.SourceSinkPresent = false;
            thisConstruct.SolutionDimensions = 0;
            thisConstruct.SourceAfterLayer = 0;
            thisConstruct.TempAfterLayer = 0;
            thisConstruct.ThicknessPerpend = 0.0;
            thisConstruct.AbsDiffIn = 0.0;
            thisConstruct.AbsDiffOut = 0.0;
            thisConstruct.AbsDiff = 0.0;
            thisConstruct.AbsDiffBack = 0.0;
            thisConstruct.AbsDiffShade = 0.0;
            thisConstruct.AbsDiffBackShade = 0.0;
            thisConstruct.ShadeAbsorpThermal = 0.0;
            std::fill(thisConstruct.AbsBeamShadeCoef.begin(), thisConstruct.AbsBeamShadeCoef.end(), 0.0);
            thisConstruct.TransDiff = 0.0;
            thisConstruct.TransDiffVis = 0.0;
            thisConstruct.ReflectSolDiffBack = 0.0;
            thisConstruct.ReflectSolDiffFront = 0.0;
            thisConstruct.ReflectVisDiffBack = 0.0;
            thisConstruct.ReflectVisDiffFront = 0.0;
            std::fill(thisConstruct.TransSolBeamCoef.begin(), thisConstruct.TransSolBeamCoef.end(), 0.0);
            std::fill(thisConstruct.TransVisBeamCoef.begin(), thisConstruct.TransVisBeamCoef.end(), 0.0);
            std::fill(thisConstruct.ReflSolBeamFrontCoef.begin(), thisConstruct.ReflSolBeamFrontCoef.end(), 0.0);
            std::fill(thisConstruct.ReflSolBeamBackCoef.begin(), thisConstruct.ReflSolBeamBackCoef.end(), 0.0);
            thisConstruct.W5FrameDivider = 0;
            thisConstruct.FromWindow5DataFile = false;
            thisConstruct.W5FileMullionWidth = 0.0;
            thisConstruct.W5FileMullionOrientation = DataWindowEquivalentLayer::Orientation::Invalid;
            thisConstruct.W5FileGlazingSysWidth = 0.0;
            thisConstruct.W5FileGlazingSysHeight = 0.0;
            for (int Layer = 1; Layer <= state.dataHeatBal->MaxSolidWinLayers; ++Layer) {
                std::fill(thisConstruct.AbsBeamCoef(Layer).begin(), thisConstruct.AbsBeamCoef(Layer).end(), 0.0);
                std::fill(thisConstruct.AbsBeamBackCoef(Layer).begin(), thisConstruct.AbsBeamBackCoef(Layer).end(), 0.0);
            }
        }
        return (newConstruct);
    }

    void ModifyWindow(EnergyPlusData &state,
                      int const SurfNum,    // SurfNum has construction of glazing system from Window5 Data File;
                      bool &ErrorsFound,    // Set to true if errors found
                      int &AddedSubSurfaces // Subsurfaces added when window references a
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   Feb 2002
        //       MODIFIED       June 2004, FCW: SinAzim, CosAzim, SinTilt, CosTilt, OutNormVec, GrossArea
        //                       and Perimeter weren't being set for created window for case when
        //                       window from Window5DataFile had two different glazing systems. Also,
        //                       GrossArea and Perimeter of original window were not being recalculated.
        //                      October 2007, LKL: Net area for shading calculations was not being
        //                       recalculated.

        // PURPOSE OF THIS SUBROUTINE:
        // If a window from the Window5DataFile has one glazing system, modify the
        // vertex coordinates of the original window to correspond to the dimensions
        // of the glazing system on the Data File.
        // If a window from the Window5DataFile has two different glazing systems, split
        // the window into two separate windows with different properties and adjust the
        // vertices of these windows taking into account the dimensions of the glazing systems
        // on the Data File and the width and orientation of the mullion that separates
        // the glazing systems.

        // SUBROUTINE ARGUMENT DEFINITIONS:
        // If there is a second glazing system on the Data File, SurfNum+1
        // has the construction of the second glazing system.

        // 2-glazing system Window5 data file entry

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 H; // Height and width of original window (m)
        Real64 W;
        // unused1208  REAL(r64)    :: MulWidth                        ! Mullion width (m)
        Real64 h1; // height and width of first glazing system (m)
        Real64 w1;
        // unused1208  REAL(r64)    :: h2,w2                           ! height and width of second glazing system (m)
        // unused1208  type (rectangularwindow) :: NewCoord
        int IConst;             // Construction number of first glazing system
        int IConst2;            // Construction number of second glazing system
        std::string Const2Name; // Name of construction of second glazing system
        // unused1208  REAL(r64)    :: AreaNew                         ! Sum of areas of the two glazing systems (m2)

        auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);

        struct rectangularwindow
        {
            // Members
            Array1D<Vector> Vertex;

            // Default Constructor
            rectangularwindow() : Vertex(4)
            {
            }
        };

        // Object Data
        Vector TVect;
        rectangularwindow OriginalCoord;

        IConst = surfTemp.Construction;

        // Height and width of original window
        TVect = surfTemp.Vertex(3) - surfTemp.Vertex(2);
        W = Vectors::VecLength(TVect); // SQRT((X(3)-X(2))**2 + (Y(3)-Y(2))**2 + (Z(3)-Z(2))**2)
        TVect = surfTemp.Vertex(2) - surfTemp.Vertex(1);
        H = Vectors::VecLength(TVect); // SQRT((X(1)-X(2))**2 + (Y(1)-Y(2))**2 + (Z(1)-Z(2))**2)

        // Save coordinates of original window in case Window 5 data overwrites.
        OriginalCoord.Vertex({1, surfTemp.Sides}) = surfTemp.Vertex({1, surfTemp.Sides});

        // Height and width of first glazing system
        h1 = state.dataConstruction->Construct(IConst).W5FileGlazingSysHeight;
        w1 = state.dataConstruction->Construct(IConst).W5FileGlazingSysWidth;

        Const2Name = state.dataConstruction->Construct(IConst).Name + ":2";
        IConst2 = Util::FindItemInList(Const2Name, state.dataConstruction->Construct);

        if (IConst2 == 0) { // Only one glazing system on Window5 Data File for this window.

            // So... original dimensions and area of window are used (entered in IDF)
            // Warning if dimensions of original window differ from those on Data File by more than 10%

            if (std::abs((H - h1) / H) > 0.10 || std::abs((W - w1) / W) > 0.10) {

                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowWarningError(state,
                                     format("SurfaceGeometry: ModifyWindow: Window {} uses the Window5 Data File Construction {}",
                                            surfTemp.Name,
                                            state.dataConstruction->Construct(IConst).Name));
                    ShowContinueError(state, format("The height {:.3R}(m) or width  (m) of this window differs by more than 10%{:.3R}", H, W));
                    ShowContinueError(state,
                                      format("from the corresponding height {:.3R} (m) or width  (m) on the Window5 Data file.{:.3R}", h1, w1));
                    ShowContinueError(state, "This will affect the frame heat transfer calculation if the frame in the Data File entry");
                    ShowContinueError(state, "is not uniform, i.e., has sections with different geometry and/or thermal properties.");
                } else {
                    ++state.dataSurfaceGeometry->Warning1Count;
                }
            }

            // Calculate net area for base surface
            state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).Area -= surfTemp.Area;
            if (state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).Area <= 0.0) {
                ShowSevereError(
                    state,
                    format("Subsurfaces have too much area for base surface={}", state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).Name));
                ShowContinueError(state, format("Subsurface creating error={}", surfTemp.Name));
                ErrorsFound = true;
            }

            // Net area of base surface with unity window multipliers (used in shadowing checks)
            state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).NetAreaShadowCalc -= surfTemp.Area / surfTemp.Multiplier;

        } else { // Two glazing systems on Window5 data file for this window

            // if exterior window, okay.

            if (surfTemp.ExtBoundCond == DataSurfaces::ExternalEnvironment) {
                // There are two glazing systems (separated by a vertical or horizontal mullion) on the Window5 Data File.
                // Fill in geometry data for the second window (corresponding to the second glazing system on the data file.
                // The first glazing system is assumed to be at left for vertical mullion, at bottom for horizontal mullion.
                // The second glazing system is assumed to be at right for vertical mullion, at top for horizontal mullion.
                // The lower left-hand corner of the original window (its vertex #2) is assumed to coincide with
                // vertex #2 of the first glazing system.

                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowMessage(state,
                                format("SurfaceGeometry: ModifyWindow: Window {} has been replaced with the Window 5/6 two glazing system=\"{}\".",
                                       surfTemp.Name,
                                       state.dataConstruction->Construct(IConst).Name));
                    ShowContinueError(state, "Note that originally entered dimensions are overridden.");
                } else {
                    ++state.dataSurfaceGeometry->Warning2Count;
                }

                // Allocate another window
                AddWindow(state, SurfNum, ErrorsFound, AddedSubSurfaces);

            } else if (surfTemp.ExtBoundCond > 0) { // Interior window, specified  ! not external environment

                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowWarningError(
                        state,
                        format("SurfaceGeometry: ModifyWindow: Interior Window {} has been replaced with the Window 5/6 two glazing system=\"{}\".",
                               surfTemp.Name,
                               state.dataConstruction->Construct(IConst).Name));
                    ShowContinueError(
                        state, "Please check to make sure interior window is correct. Note that originally entered dimensions are overridden.");
                } else {
                    ++state.dataSurfaceGeometry->Warning3Count;
                }

                AddWindow(state, SurfNum, ErrorsFound, AddedSubSurfaces);

            } else { // Interior window, specified not entered

                ShowSevereError(state, format("SurfaceGeometry: ModifyWindow: Interior Window {} is a window in an adjacent zone.", surfTemp.Name));
                ShowContinueError(
                    state,
                    format("Attempted to add/reverse Window 5/6 multiple glazing system=\"{}\".", state.dataConstruction->Construct(IConst).Name));
                ShowContinueError(state, "Cannot use these Window 5/6 constructs for these Interior Windows. Program will terminate.");
                ErrorsFound = true;
            }

        } // End of check if one or two glazing systems are on the Window5 Data File
    }

    void AddWindow(EnergyPlusData &state,
                   int const SurfNum,    // SurfNum has construction of glazing system from Window5 Data File;
                   bool &ErrorsFound,    // Set to true if errors found
                   int &AddedSubSurfaces // Subsurfaces added when window references a
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   Nov 2008

        // PURPOSE OF THIS SUBROUTINE:
        // This routine is called from ModifyWindow to add a window.  Allows it to be
        // called in more than one place in the calling routine so as to be able to have
        // specific warnings or errors issued.

        // SUBROUTINE ARGUMENT DEFINITIONS:
        // If there is a second glazing system on the Data File, SurfNum+1
        // has the construction of the second glazing system.

        // 2-glazing system Window5 data file entry

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int loop; // DO loop index
        Real64 H; // Height and width of original window (m)
        Real64 W;
        Real64 MulWidth; // Mullion width (m)
        Real64 h1;       // height and width of first glazing system (m)
        Real64 w1;
        Real64 h2; // height and width of second glazing system (m)
        Real64 w2;
        Real64 xa; // Vertex intermediate variables (m)
        Real64 ya;
        Real64 za;
        Real64 xb;
        Real64 yb;
        Real64 zb;
        Real64 dx; // Vertex displacements from original window (m)
        Real64 dy;
        int IConst;             // Construction number of first glazing system
        int IConst2;            // Construction number of second glazing system
        std::string Const2Name; // Name of construction of second glazing system
        Real64 AreaNew;         // Sum of areas of the two glazing systems (m2)

        struct rectangularwindow
        {
            // Members
            Array1D<Vector> Vertex;

            // Default Constructor
            rectangularwindow() : Vertex(4)
            {
            }
        };

        // Object Data
        Vector TVect;
        rectangularwindow NewCoord;
        rectangularwindow OriginalCoord;

        auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);

        IConst = surfTemp.Construction;

        // Height and width of original window
        TVect = surfTemp.Vertex(3) - surfTemp.Vertex(2);
        W = Vectors::VecLength(TVect); // SQRT((X(3)-X(2))**2 + (Y(3)-Y(2))**2 + (Z(3)-Z(2))**2)
        TVect = surfTemp.Vertex(2) - surfTemp.Vertex(1);
        H = Vectors::VecLength(TVect); // SQRT((X(1)-X(2))**2 + (Y(1)-Y(2))**2 + (Z(1)-Z(2))**2)

        // Save coordinates of original window in case Window 5 data overwrites.
        OriginalCoord.Vertex({1, surfTemp.Sides}) = surfTemp.Vertex({1, surfTemp.Sides});

        // Height and width of first glazing system
        h1 = state.dataConstruction->Construct(IConst).W5FileGlazingSysHeight;
        w1 = state.dataConstruction->Construct(IConst).W5FileGlazingSysWidth;

        Const2Name = state.dataConstruction->Construct(IConst).Name + ":2";
        IConst2 = Util::FindItemInList(Const2Name, state.dataConstruction->Construct);

        ++AddedSubSurfaces;
        state.dataSurfaceGeometry->SurfaceTmp.redimension(++state.dataSurface->TotSurfaces);

        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Vertex.allocate(4);

        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Name = surfTemp.Name + ":2";
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Construction = IConst2;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).ConstructionStoredInputValue = IConst2;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Class = surfTemp.Class;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Azimuth = surfTemp.Azimuth;
        // Sine and cosine of azimuth and tilt
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).SinAzim = surfTemp.SinAzim;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).CosAzim = surfTemp.CosAzim;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).SinTilt = surfTemp.SinTilt;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).CosTilt = surfTemp.CosTilt;
        // Outward normal unit vector (pointing away from room)
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Centroid = surfTemp.Centroid;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).lcsx = surfTemp.lcsx;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).lcsy = surfTemp.lcsy;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).lcsz = surfTemp.lcsz;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).NewellAreaVector = surfTemp.NewellAreaVector;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).OutNormVec = surfTemp.OutNormVec;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Reveal = surfTemp.Reveal;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Shape = surfTemp.Shape;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Sides = surfTemp.Sides;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Tilt = surfTemp.Tilt;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).convOrientation = Convect::GetSurfConvOrientation(surfTemp.Tilt);
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).HeatTransSurf = surfTemp.HeatTransSurf;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).BaseSurfName = surfTemp.BaseSurfName;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).BaseSurf = surfTemp.BaseSurf;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).ZoneName = surfTemp.ZoneName;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Zone = surfTemp.Zone;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).ExtBoundCondName = surfTemp.ExtBoundCondName;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).ExtBoundCond = surfTemp.ExtBoundCond;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).ExtSolar = surfTemp.ExtSolar;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).ExtWind = surfTemp.ExtWind;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).ViewFactorGround = surfTemp.ViewFactorGround;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).ViewFactorSky = surfTemp.ViewFactorSky;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).ViewFactorGroundIR = surfTemp.ViewFactorGroundIR;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).ViewFactorSkyIR = surfTemp.ViewFactorSkyIR;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).OSCPtr = surfTemp.OSCPtr;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).shadowSurfSched = surfTemp.shadowSurfSched;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).activeWindowShadingControl = surfTemp.activeWindowShadingControl;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).windowShadingControlList = surfTemp.windowShadingControlList;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).HasShadeControl = surfTemp.HasShadeControl;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).activeShadedConstruction = surfTemp.activeShadedConstruction;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).windowShadingControlList = surfTemp.windowShadingControlList;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).shadedStormWinConstructionList =
            surfTemp.shadedStormWinConstructionList;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).FrameDivider = surfTemp.FrameDivider;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Multiplier = surfTemp.Multiplier;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).NetAreaShadowCalc = surfTemp.NetAreaShadowCalc;

        MulWidth = state.dataConstruction->Construct(IConst).W5FileMullionWidth;
        w2 = state.dataConstruction->Construct(IConst2).W5FileGlazingSysWidth;
        h2 = state.dataConstruction->Construct(IConst2).W5FileGlazingSysHeight;

        // Correction to net area of base surface. Add back in the original glazing area and subtract the
        // area of the two glazing systems. Note that for Surface(SurfNum)%Class = 'Window' the effect
        // of a window multiplier is included in the glazing area. Note that frame areas are subtracted later.

        AreaNew = surfTemp.Multiplier * (h1 * w1 + h2 * w2); // both glazing systems
        // Adjust net area for base surface
        state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).Area -= AreaNew;

        // Net area of base surface with unity window multipliers (used in shadowing checks)
        state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).NetAreaShadowCalc -= AreaNew / surfTemp.Multiplier;

        // Reset area, etc. of original window
        surfTemp.Area = surfTemp.Multiplier * (h1 * w1);
        surfTemp.GrossArea = surfTemp.Area;
        surfTemp.NetAreaShadowCalc = h1 * w1;
        surfTemp.Perimeter = 2 * (h1 + w1);
        surfTemp.Height = h1;
        surfTemp.Width = w1;
        // Set area, etc. of new window
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Area =
            state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Multiplier * (h2 * w2);
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).GrossArea =
            state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Area;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).NetAreaShadowCalc = h2 * w2;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Perimeter = 2 * (h2 + w2);
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Height = h2;
        state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Width = w2;

        if (state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).Area <= 0.0) {
            ShowSevereError(state,
                            format("SurfaceGeometry: ModifyWindow: Subsurfaces have too much area for base surface={}",
                                   state.dataSurfaceGeometry->SurfaceTmp(surfTemp.BaseSurf).Name));
            ShowContinueError(state, format("Subsurface (window) creating error={}", surfTemp.Name));
            ShowContinueError(state,
                              format("This window has been replaced by two windows from the Window5 Data File of total area {:.2R} m2", AreaNew));
            ErrorsFound = true;
        }

        // Assign vertices to the new window; modify vertices of original window.
        // In the following, vertices are numbered counter-clockwise with vertex #1 at the upper left.

        if (state.dataConstruction->Construct(IConst).W5FileMullionOrientation == DataWindowEquivalentLayer::Orientation::Vertical) {

            // VERTICAL MULLION: original window is modified to become left-hand glazing (system #1);
            // new window is created to become right-hand glazing (system #2)

            // Left-hand glazing

            // Vertex 1
            dx = 0.0;
            dy = H - h1;
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(1).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(1).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(1).z = za + (dx / W) * (zb - za);

            // Vertex 2
            dx = 0.0;
            dy = H;
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(2).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(2).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(2).z = za + (dx / W) * (zb - za);

            // Vertex 3
            dx = w1;
            dy = H;
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(3).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(3).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(3).z = za + (dx / W) * (zb - za);

            // Vertex 4
            dx = w1;
            dy = H - h1;
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(4).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(4).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(4).z = za + (dx / W) * (zb - za);

            for (loop = 1; loop <= surfTemp.Sides; ++loop) {
                surfTemp.Vertex(loop) = NewCoord.Vertex(loop);
            }

            // Right-hand glazing

            // Vertex 1
            dx = w1 + MulWidth;
            dy = H - (h1 + h2) / 2.0;
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(1).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(1).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(1).z = za + (dx / W) * (zb - za);

            // Vertex 2
            dx = w1 + MulWidth;
            dy = H + (h2 - h1) / 2.0;
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(2).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(2).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(2).z = za + (dx / W) * (zb - za);

            // Vertex 3
            dx = w1 + MulWidth + w2;
            dy = H + (h2 - h1) / 2.0;
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(3).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(3).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(3).z = za + (dx / W) * (zb - za);

            // Vertex 4
            dx = w1 + MulWidth + w2;
            dy = H - (h1 + h2) / 2.0;
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(4).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(4).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(4).z = za + (dx / W) * (zb - za);

            for (loop = 1; loop <= state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Sides; ++loop) {
                state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Vertex(loop) = NewCoord.Vertex(loop);
            }

        } else { // Horizontal mullion

            // HORIZONTAL MULLION: original window is modified to become bottom glazing (system #1);
            // new window is created to become top glazing (system #2)

            // Bottom glazing

            // Vertex 1
            dx = 0.0;
            dy = H - h1;
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(1).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(1).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(1).z = za + (dx / W) * (zb - za);

            // Vertex 2
            dx = 0.0;
            dy = H;
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(2).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(2).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(2).z = za + (dx / W) * (zb - za);

            // Vertex 3
            dx = w1;
            dy = H;
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(3).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(3).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(3).z = za + (dx / W) * (zb - za);

            // Vertex 4
            dx = w1;
            dy = H - h1;
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(4).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(4).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(4).z = za + (dx / W) * (zb - za);

            for (loop = 1; loop <= surfTemp.Sides; ++loop) {
                surfTemp.Vertex(loop) = NewCoord.Vertex(loop);
            }

            // Top glazing

            // Vertex 1
            dx = (w1 - w2) / 2.0;
            dy = H - (h1 + h2 + MulWidth);
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(1).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(1).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(1).z = za + (dx / W) * (zb - za);

            // Vertex 2
            dx = (w1 - w2) / 2.0;
            dy = H - (h1 + MulWidth);
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(2).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(2).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(2).z = za + (dx / W) * (zb - za);

            // Vertex 3
            dx = (w1 + w2) / 2.0;
            dy = H - (h1 + MulWidth);
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(3).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(3).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(3).z = za + (dx / W) * (zb - za);

            // Vertex 4
            dx = (w1 + w2) / 2.0;
            dy = H - (h1 + h2 + MulWidth);
            xa = OriginalCoord.Vertex(1).x + (dy / H) * (OriginalCoord.Vertex(2).x - OriginalCoord.Vertex(1).x);
            ya = OriginalCoord.Vertex(1).y + (dy / H) * (OriginalCoord.Vertex(2).y - OriginalCoord.Vertex(1).y);
            za = OriginalCoord.Vertex(1).z + (dy / H) * (OriginalCoord.Vertex(2).z - OriginalCoord.Vertex(1).z);
            xb = OriginalCoord.Vertex(4).x + (dy / H) * (OriginalCoord.Vertex(3).x - OriginalCoord.Vertex(4).x);
            yb = OriginalCoord.Vertex(4).y + (dy / H) * (OriginalCoord.Vertex(3).y - OriginalCoord.Vertex(4).y);
            zb = OriginalCoord.Vertex(4).z + (dy / H) * (OriginalCoord.Vertex(3).z - OriginalCoord.Vertex(4).z);
            NewCoord.Vertex(4).x = xa + (dx / W) * (xb - xa);
            NewCoord.Vertex(4).y = ya + (dx / W) * (yb - ya);
            NewCoord.Vertex(4).z = za + (dx / W) * (zb - za);

            for (loop = 1; loop <= state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Sides; ++loop) {
                state.dataSurfaceGeometry->SurfaceTmp(state.dataSurface->TotSurfaces).Vertex(loop) = NewCoord.Vertex(loop);
            }

        } // End of check if vertical or horizontal mullion
    }

    void TransformVertsByAspect(EnergyPlusData &state,
                                int const SurfNum, // Current surface number
                                int const NSides   // Number of sides to figure
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent T Griffith
        //       DATE WRITTEN   April 2003

        // PURPOSE OF THIS SUBROUTINE:
        // Alter input for surface geometry
        // Optimizing building design for energy can involve
        //  altering building geometry.  Rather than assemble routines to transform
        //  geometry through pre-processing on input, it may be simpler to change
        //  vertices within EnergyPlus since it already reads the data from the input
        //  file and there would no longer be a need to rewrite the text data.
        //  This is essentially a crude hack to allow adjusting geometry with
        //  a single parameter...

        // METHODOLOGY EMPLOYED:
        // once vertices have been converted to WCS, change them to reflect a different aspect
        // ratio for the entire building based on user input.
        // This routine is called once for each surface by subroutine GetVertices

        static std::string const CurrentModuleObject("GeometryTransform");

        Array1D_string cAlphas(1);
        Array1D<Real64> rNumerics(2);
        auto &OldAspectRatio = state.dataSurfaceGeometry->OldAspectRatio;
        auto &NewAspectRatio = state.dataSurfaceGeometry->NewAspectRatio;
        int n;
        Real64 Xo;
        Real64 XnoRot;
        Real64 Xtrans;
        Real64 Yo;
        Real64 YnoRot;
        Real64 Ytrans;
        // begin execution
        // get user input...

        auto &s_ipsc = state.dataIPShortCut;
        auto &surfTemp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);

        if (state.dataSurfaceGeometry->firstTime) {
            if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject) == 1) {
                int NAlphas;
                int NNum;
                int IOStat;
                auto &s_ipsc = state.dataIPShortCut;
                auto &transformPlane = state.dataSurfaceGeometry->transformPlane;
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         1,
                                                                         cAlphas,
                                                                         NAlphas,
                                                                         rNumerics,
                                                                         NNum,
                                                                         IOStat,
                                                                         s_ipsc->lNumericFieldBlanks,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);
                OldAspectRatio = rNumerics(1);
                NewAspectRatio = rNumerics(2);
                transformPlane = cAlphas(1);
                if (transformPlane != "XY") {
                    ShowWarningError(state, format("{}: invalid {}=\"{}...ignored.", CurrentModuleObject, s_ipsc->cAlphaFieldNames(1), cAlphas(1)));
                }
                state.dataSurfaceGeometry->firstTime = false;
                state.dataSurfaceGeometry->noTransform = false;
                state.dataSurface->AspectTransform = true;
                if (state.dataSurface->WorldCoordSystem) {
                    ShowWarningError(state, format("{}: must use Relative Coordinate System.  Transform request ignored.", CurrentModuleObject));
                    state.dataSurfaceGeometry->noTransform = true;
                    state.dataSurface->AspectTransform = false;
                }
            } else {
                state.dataSurfaceGeometry->firstTime = false;
            }
        }
        if (state.dataSurfaceGeometry->noTransform) {
            return;
        }

        // check surface type.
        if (!surfTemp.HeatTransSurf) {
            // Site Shading do not get transformed.
            if (surfTemp.Class == SurfaceClass::Detached_F) {
                return;
            }
        }

        // testing method of transforming  x and y coordinates as follows

        // this works if not rotated wrt north axis ... but if it is, then trouble
        // try to first derotate it , transform by aspect and then rotate back.

        for (n = 1; n <= NSides; ++n) {
            Xo = surfTemp.Vertex(n).x; // world coordinates.... shifted by relative north angle...
            Yo = surfTemp.Vertex(n).y;
            // next derotate the building
            XnoRot = Xo * state.dataSurfaceGeometry->CosBldgRelNorth + Yo * state.dataSurfaceGeometry->SinBldgRelNorth;
            YnoRot = Yo * state.dataSurfaceGeometry->CosBldgRelNorth - Xo * state.dataSurfaceGeometry->SinBldgRelNorth;
            // translate
            Xtrans = XnoRot * std::sqrt(NewAspectRatio / OldAspectRatio);
            Ytrans = YnoRot * std::sqrt(OldAspectRatio / NewAspectRatio);
            // rerotate
            surfTemp.Vertex(n).x = Xtrans * state.dataSurfaceGeometry->CosBldgRelNorth - Ytrans * state.dataSurfaceGeometry->SinBldgRelNorth;

            surfTemp.Vertex(n).y = Xtrans * state.dataSurfaceGeometry->SinBldgRelNorth + Ytrans * state.dataSurfaceGeometry->CosBldgRelNorth;
        }
    }

    void CalcSurfaceCentroid(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Feb. 2004

        // PURPOSE OF THIS SUBROUTINE:
        // compute centroid of all the surfaces in the main
        // surface structure. Store the vertex coordinates of
        // the centroid in the 'SURFACE' derived type.

        // METHODOLOGY EMPLOYED:
        // The centroid of triangle is easily computed by averaging the vertices
        // The centroid of a quadrilateral is computed by area weighting the centroids
        // of two triangles.
        // (Algorithm would need to be changed for higher order
        // polygons with more than four sides).

        auto &Triangle1 = state.dataSurfaceGeometry->Triangle1;
        auto &Triangle2 = state.dataSurfaceGeometry->Triangle2;
        static Vector const zero_vector(0.0);
        Vector centroid;

        int negZcount(0); // for warning error in surface centroids

        // loop through all the surfaces
        for (int ThisSurf = 1; ThisSurf <= state.dataSurface->TotSurfaces; ++ThisSurf) {
            auto &surface = state.dataSurface->Surface(ThisSurf);

            if (surface.Class == SurfaceClass::IntMass) {
                continue;
            }

            auto const &vertex = surface.Vertex;

            if (surface.Sides == 3) { // 3-sided polygon

                centroid = cen(vertex(1), vertex(2), vertex(3));

            } else if (surface.Sides == 4) { // 4-sided polygon

                // split into 2 3-sided polygons (Triangle 1 and Triangle 2)
                Triangle1(1) = vertex(1);
                Triangle1(2) = vertex(2);
                Triangle1(3) = vertex(3);
                Triangle2(1) = vertex(1);
                Triangle2(2) = vertex(3);
                Triangle2(3) = vertex(4);

                // get total Area of quad.
                Real64 TotalArea(surface.GrossArea);
                if (TotalArea <= 0.0) {
                    // catch a problem....
                    ShowWarningError(state, format("CalcSurfaceCentroid: zero area surface, for surface={}", surface.Name));
                    continue;
                }

                // get area fraction of triangles.
                Real64 Tri1Area(Vectors::AreaPolygon(3, Triangle1) / TotalArea);
                Real64 Tri2Area(Vectors::AreaPolygon(3, Triangle2) / TotalArea);

                // check if sum of fractions are slightly greater than 1.0 which is a symptom of the triangles for a non-convex
                // quadrilateral using the wrong two triangles
                if ((Tri1Area + Tri2Area) > 1.05) {

                    // if so repeat the process with the other two possible triangles (notice the vertices are in a different order this
                    // time) split into 2 3-sided polygons (Triangle 1 and Triangle 2)
                    Triangle1(1) = vertex(1);
                    Triangle1(2) = vertex(2);
                    Triangle1(3) = vertex(4);
                    Triangle2(1) = vertex(2);
                    Triangle2(2) = vertex(3);
                    Triangle2(3) = vertex(4);

                    // get area fraction of triangles.
                    Real64 AreaTriangle1 = Vectors::AreaPolygon(3, Triangle1);
                    Real64 AreaTriangle2 = Vectors::AreaPolygon(3, Triangle2);
                    TotalArea = AreaTriangle1 + AreaTriangle2;
                    Tri1Area = AreaTriangle1 / TotalArea;
                    Tri2Area = AreaTriangle2 / TotalArea;
                }

                // get centroid of Triangle 1
                Vector cen1(cen(Triangle1(1), Triangle1(2), Triangle1(3)));

                // get centroid of Triangle 2
                Vector cen2(cen(Triangle2(1), Triangle2(2), Triangle2(3)));

                // find area weighted combination of the two centroids (coded to avoid temporary Vectors)
                cen1 *= Tri1Area;
                cen2 *= Tri2Area;
                centroid = cen1;
                centroid += cen2;

            } else if (surface.Sides >= 5) { // multi-sided polygon
                // (Maybe triangulate?  For now, use old "z" average method")
                // and X and Y -- straight average

                //        X1=MINVAL(Surface(ThisSurf)%Vertex(1:Surface(ThisSurf)%Sides)%x)
                //        X2=MAXVAL(Surface(ThisSurf)%Vertex(1:Surface(ThisSurf)%Sides)%x)
                //        Y1=MINVAL(Surface(ThisSurf)%Vertex(1:Surface(ThisSurf)%Sides)%y)
                //        Y2=MAXVAL(Surface(ThisSurf)%Vertex(1:Surface(ThisSurf)%Sides)%y)
                //        Z1=MINVAL(Surface(ThisSurf)%Vertex(1:Surface(ThisSurf)%Sides)%z)
                //        Z2=MAXVAL(Surface(ThisSurf)%Vertex(1:Surface(ThisSurf)%Sides)%z)
                //        Xcm=(X1+X2)/2.0d0
                //        Ycm=(Y1+Y2)/2.0d0
                //        Zcm=(Z1+Z2)/2.0d0

                // Calc centroid as average of surfaces
                centroid = 0.0;
                for (int vert = 1; vert <= surface.Sides; ++vert) {
                    centroid += vertex(vert);
                }
                centroid /= double(surface.Sides);

            } else {

                if (!surface.Name.empty()) {
                    ShowWarningError(state, format("CalcSurfaceCentroid: caught problem with # of sides, for surface={}", surface.Name));
                    ShowContinueError(state, format("... number of sides must be >= 3, this surface # sides={}", surface.Sides));
                } else {
                    ShowWarningError(state, format("CalcSurfaceCentroid: caught problem with # of sides, for surface=#{}", ThisSurf));
                    ShowContinueError(state,
                                      "...surface name is blank. Examine surfaces -- this may be a problem with ill-formed interzone surfaces.");
                    ShowContinueError(state, format("... number of sides must be >= 3, this surface # sides={}", surface.Sides));
                }
                centroid = 0.0;
            }

            // store result in the surface structure in DataSurfaces
            surface.Centroid = centroid;

            if (centroid.z < 0.0) {
                if (surface.ExtWind || surface.ExtBoundCond == DataSurfaces::ExternalEnvironment) {
                    ++negZcount;
                }
            }

        } // loop through surfaces

        if (negZcount > 0) {
            ShowWarningError(state, format("CalcSurfaceCentroid: {} Surfaces have the Z coordinate < 0.", negZcount));
            ShowContinueError(state, "...in any calculations, Wind Speed will be 0.0 for these surfaces.");
            ShowContinueError(state,
                              format("...in any calculations, Outside temperatures will be the outside temperature + {:.3R} for these surfaces.",
                                     state.dataEnvrn->WeatherFileTempModCoeff));
            ShowContinueError(state, "...that is, these surfaces will have conditions as though at ground level.");
        }
    }

    void SetupShadeSurfacesForSolarCalcs(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec. 2008

        // PURPOSE OF THIS SUBROUTINE:
        // determine if Shading surfaces need full solar calcs because they
        // are also used to define geometry of an active solar component.
        // Normally, a shading surface is not included in calculations of incident solar, only shading

        // METHODOLOGY EMPLOYED:
        // Mine solar renewables input and collect surface names.
        // find shading surfaces with names that match those in solar objects.
        // setup flags for shading surfaces so that the solar renewables can reuse incident solar calcs
        // new solar component models that use shading surfaces will have to extend the code here.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D_string TmpCandidateSurfaceNames;
        Array1D_string TmpCandidateICSSurfaceNames;
        Array1D_string TmpCandidateICSBCTypeNames;
        int NumAlphas;  // Number of alpha names being passed
        int NumNumbers; // Number of numeric parameters being passed
        int IOStatus;

        auto &s_ipsc = state.dataIPShortCut;
        // First collect names of surfaces referenced by active solar components
        s_ipsc->cCurrentModuleObject = "SolarCollector:FlatPlate:Water";
        int NumOfFlatPlateUnits = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        s_ipsc->cCurrentModuleObject = "SolarCollector:FlatPlate:PhotovoltaicThermal";
        int NumPVTs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        s_ipsc->cCurrentModuleObject = "Generator:Photovoltaic";
        int NumPVs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
        s_ipsc->cCurrentModuleObject = "SolarCollector:IntegralCollectorStorage";
        int NumOfICSUnits = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        int NumCandidateNames = NumOfFlatPlateUnits + NumPVTs + NumPVs + NumOfICSUnits;
        int NumOfCollectors = NumOfFlatPlateUnits + NumOfICSUnits;

        TmpCandidateSurfaceNames.allocate(NumCandidateNames);
        TmpCandidateICSSurfaceNames.allocate(NumOfCollectors);
        TmpCandidateICSBCTypeNames.allocate(NumOfCollectors);

        if (NumOfCollectors > 0) {
            s_ipsc->cCurrentModuleObject = "SolarCollector:FlatPlate:Water";
            for (int CollectorNum = 1; CollectorNum <= NumOfFlatPlateUnits; ++CollectorNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(
                    state, s_ipsc->cCurrentModuleObject, CollectorNum, s_ipsc->cAlphaArgs, NumAlphas, s_ipsc->rNumericArgs, NumNumbers, IOStatus);

                TmpCandidateSurfaceNames(CollectorNum) = s_ipsc->cAlphaArgs(3);
                TmpCandidateICSBCTypeNames(CollectorNum) = "";
            }
        }

        if (NumPVTs > 0) {
            s_ipsc->cCurrentModuleObject = "SolarCollector:FlatPlate:PhotovoltaicThermal";
            for (int PVTnum = 1; PVTnum <= NumPVTs; ++PVTnum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(
                    state, s_ipsc->cCurrentModuleObject, PVTnum, s_ipsc->cAlphaArgs, NumAlphas, s_ipsc->rNumericArgs, NumNumbers, IOStatus);

                TmpCandidateSurfaceNames(NumOfFlatPlateUnits + PVTnum) = s_ipsc->cAlphaArgs(2);
            }
        }

        if (NumPVs > 0) {
            s_ipsc->cCurrentModuleObject = "Generator:Photovoltaic";
            for (int PVnum = 1; PVnum <= NumPVs; ++PVnum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(
                    state, s_ipsc->cCurrentModuleObject, PVnum, s_ipsc->cAlphaArgs, NumAlphas, s_ipsc->rNumericArgs, NumNumbers, IOStatus);
                TmpCandidateSurfaceNames(NumOfFlatPlateUnits + NumPVTs + PVnum) = s_ipsc->cAlphaArgs(2);
            }
        }

        if (NumOfICSUnits > 0) {
            s_ipsc->cCurrentModuleObject = "SolarCollector:IntegralCollectorStorage";
            for (int CollectorNum = 1; CollectorNum <= NumOfICSUnits; ++CollectorNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(
                    state, s_ipsc->cCurrentModuleObject, CollectorNum, s_ipsc->cAlphaArgs, NumAlphas, s_ipsc->rNumericArgs, NumNumbers, IOStatus);
                TmpCandidateSurfaceNames(NumOfFlatPlateUnits + NumPVTs + NumPVs + CollectorNum) = s_ipsc->cAlphaArgs(3);
                TmpCandidateICSSurfaceNames(NumOfFlatPlateUnits + CollectorNum) = s_ipsc->cAlphaArgs(3);
                TmpCandidateICSBCTypeNames(NumOfFlatPlateUnits + CollectorNum) = s_ipsc->cAlphaArgs(4);
            }
        }

        // loop through all the surfaces
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            auto &surf = state.dataSurface->Surface(SurfNum);
            int Found = Util::FindItemInList(surf.Name, TmpCandidateSurfaceNames, NumCandidateNames);
            if (Found > 0) {
                if (!surf.HeatTransSurf) { // not BIPV, must be a shading surf with solar device
                    // Setup missing values to allow shading surfaces to model incident solar and wind
                    surf.ExtSolar = true;
                    surf.ExtWind = true;
                    surf.ViewFactorGround = 0.5 * (1.0 - surf.CosTilt);
                }
                // check if this surface is used for ICS collector mounting and has OthersideCondictionsModel as its
                // boundary condition
                if (NumOfICSUnits > 0) {
                    for (int CollectorNum = 1; CollectorNum <= NumOfCollectors; ++CollectorNum) {
                        if (Util::SameString(surf.Name, TmpCandidateICSSurfaceNames(CollectorNum)) &&
                            Util::SameString(TmpCandidateICSBCTypeNames(CollectorNum), "OTHERSIDECONDITIONSMODEL")) {
                            state.dataSurface->SurfIsICS(SurfNum) = true;
                            state.dataSurface->SurfICSPtr(SurfNum) = CollectorNum;
                        }
                    }
                }

            } // end of IF (Found > 0) Then
        }
    }

    void
    SetupEnclosuresAndAirBoundaries(EnergyPlusData &state,
                                    EPVector<DataViewFactorInformation::EnclosureViewFactorInformation> &Enclosures, // Radiant or Solar Enclosures
                                    SurfaceGeometry::enclosureType const EnclosureType,                              // Radiant or Solar
                                    bool &ErrorsFound)                                                               // Set to true if errors found
    {
        static constexpr std::string_view RoutineName = "SetupEnclosuresAndAirBoundaries";
        bool anyGroupedSpaces = false;
        bool radiantSetup = false;
        bool solarSetup = false;
        std::string RadiantOrSolar = "";
        int enclosureNum = 0;
        if (EnclosureType == RadiantEnclosures) {
            radiantSetup = true;
            RadiantOrSolar = "Radiant";
            state.dataViewFactor->EnclRadInfo.allocate(state.dataGlobal->numSpaces);
        } else if (EnclosureType == SolarEnclosures) {
            solarSetup = true;
            RadiantOrSolar = "Solar";
            state.dataViewFactor->EnclSolInfo.allocate(state.dataGlobal->numSpaces);
        } else {
            ShowFatalError(
                state, format("{}: Illegal call to this function. Second argument must be 'RadiantEnclosures' or 'SolarEnclosures'", RoutineName));
        }
        if (std::any_of(state.dataConstruction->Construct.begin(),
                        state.dataConstruction->Construct.end(),
                        [](Construction::ConstructionProps const &e) { return e.TypeIsAirBoundary; })) {
            int errorCount = 0;
            for (int surfNum = 1; surfNum <= state.dataSurface->TotSurfaces; ++surfNum) {
                auto &surf = state.dataSurface->Surface(surfNum);
                if (surf.Construction == 0) {
                    continue;
                }
                auto &constr = state.dataConstruction->Construct(surf.Construction);
                if (!constr.TypeIsAirBoundary) {
                    continue;
                }
                surf.IsAirBoundarySurf = true;

                // Check for invalid air boundary surfaces - valid only on non-adiabatic interzone surfaces
                // Only check this once during radiant setup, skip for solar setup
                if (radiantSetup && (surf.ExtBoundCond <= 0 || surf.ExtBoundCond == surfNum)) {
                    ErrorsFound = true;
                    if (!state.dataGlobal->DisplayExtraWarnings) {
                        ++errorCount;
                    } else {
                        ShowSevereError(
                            state, format("{}: Surface=\"{}\" uses Construction:AirBoundary in a non-interzone surface.", RoutineName, surf.Name));
                    }
                } else {
                    // Process air boundary - set surface properties and set up enclosures Radiant exchange, Boundary is grouped - assign enclosure
                    state.dataHeatBal->AnyAirBoundary = true;
                    int thisSideEnclosureNum = 0;
                    int otherSideEnclosureNum = 0;
                    if (radiantSetup) {
                        // Radiant enclosure setup
                        constr.IsUsedCTF = false;
                        surf.HeatTransSurf = false;
                        surf.HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::AirBoundaryNoHT;
                        thisSideEnclosureNum = state.dataHeatBal->space(surf.spaceNum).radiantEnclosureNum;
                        otherSideEnclosureNum = state.dataHeatBal->space(state.dataSurface->Surface(surf.ExtBoundCond).spaceNum).radiantEnclosureNum;
                    } else {
                        // Solar enclosure setup
                        thisSideEnclosureNum = state.dataHeatBal->space(surf.spaceNum).solarEnclosureNum;
                        otherSideEnclosureNum = state.dataHeatBal->space(state.dataSurface->Surface(surf.ExtBoundCond).spaceNum).solarEnclosureNum;
                    }
                    anyGroupedSpaces = true;
                    if ((thisSideEnclosureNum == 0) && (otherSideEnclosureNum == 0)) {
                        // Neither zone is assigned to an enclosure, so increment the counter and assign to both
                        ++enclosureNum;
                        auto &thisEnclosure = Enclosures(enclosureNum);
                        thisSideEnclosureNum = enclosureNum;
                        thisEnclosure.Name = format("{} Enclosure {}", RadiantOrSolar, enclosureNum);
                        thisEnclosure.spaceNames.push_back(state.dataHeatBal->space(surf.spaceNum).Name);
                        thisEnclosure.spaceNums.push_back(surf.spaceNum);
                        thisEnclosure.FloorArea += state.dataHeatBal->space(surf.spaceNum).FloorArea;
                        otherSideEnclosureNum = enclosureNum;
                        int otherSideSpaceNum = state.dataSurface->Surface(surf.ExtBoundCond).spaceNum;
                        if (otherSideSpaceNum != surf.spaceNum) {
                            thisEnclosure.spaceNames.push_back(state.dataHeatBal->space(otherSideSpaceNum).Name);
                            thisEnclosure.spaceNums.push_back(otherSideSpaceNum);
                            thisEnclosure.FloorArea += state.dataHeatBal->space(otherSideSpaceNum).FloorArea;
                        }
                        if (radiantSetup) {
                            state.dataHeatBal->space(surf.spaceNum).radiantEnclosureNum = thisSideEnclosureNum;
                            state.dataHeatBal->space(state.dataSurface->Surface(surf.ExtBoundCond).spaceNum).radiantEnclosureNum =
                                otherSideEnclosureNum;
                        } else {
                            thisEnclosure.ExtWindowArea += state.dataHeatBal->space(surf.spaceNum).extWindowArea;
                            thisEnclosure.TotalSurfArea += state.dataHeatBal->space(surf.spaceNum).totalSurfArea;
                            if (otherSideSpaceNum != surf.spaceNum) {
                                thisEnclosure.ExtWindowArea += state.dataHeatBal->space(otherSideSpaceNum).extWindowArea;
                                thisEnclosure.TotalSurfArea += state.dataHeatBal->space(otherSideSpaceNum).totalSurfArea;
                            }
                            state.dataHeatBal->space(surf.spaceNum).solarEnclosureNum = thisSideEnclosureNum;
                            state.dataHeatBal->space(state.dataSurface->Surface(surf.ExtBoundCond).spaceNum).solarEnclosureNum =
                                otherSideEnclosureNum;
                        }
                    } else if (thisSideEnclosureNum == 0) {
                        // Other side is assigned, so use that one for both
                        thisSideEnclosureNum = otherSideEnclosureNum;
                        auto &thisEnclosure = Enclosures(thisSideEnclosureNum);
                        thisEnclosure.spaceNames.push_back(state.dataHeatBal->space(surf.spaceNum).Name);
                        thisEnclosure.spaceNums.push_back(surf.spaceNum);
                        thisEnclosure.FloorArea += state.dataHeatBal->space(surf.spaceNum).FloorArea;
                        if (radiantSetup) {
                            state.dataHeatBal->space(surf.spaceNum).radiantEnclosureNum = thisSideEnclosureNum;
                        } else {
                            thisEnclosure.ExtWindowArea += state.dataHeatBal->space(surf.spaceNum).extWindowArea;
                            thisEnclosure.TotalSurfArea += state.dataHeatBal->space(surf.spaceNum).totalSurfArea;
                            state.dataHeatBal->space(surf.spaceNum).solarEnclosureNum = thisSideEnclosureNum;
                        }
                    } else if (otherSideEnclosureNum == 0) {
                        // This side is assigned, so use that one for both
                        otherSideEnclosureNum = thisSideEnclosureNum;
                        auto &thisEnclosure = Enclosures(thisSideEnclosureNum);
                        thisEnclosure.spaceNames.push_back(state.dataHeatBal->space(state.dataSurface->Surface(surf.ExtBoundCond).spaceNum).Name);
                        thisEnclosure.spaceNums.push_back(state.dataSurface->Surface(surf.ExtBoundCond).spaceNum);
                        thisEnclosure.FloorArea += state.dataHeatBal->space(state.dataSurface->Surface(surf.ExtBoundCond).spaceNum).FloorArea;
                        if (radiantSetup) {
                            state.dataHeatBal->space(state.dataSurface->Surface(surf.ExtBoundCond).spaceNum).radiantEnclosureNum =
                                otherSideEnclosureNum;
                        } else {
                            thisEnclosure.ExtWindowArea +=
                                state.dataHeatBal->space(state.dataSurface->Surface(surf.ExtBoundCond).spaceNum).extWindowArea;
                            thisEnclosure.TotalSurfArea +=
                                state.dataHeatBal->space(state.dataSurface->Surface(surf.ExtBoundCond).spaceNum).totalSurfArea;
                            state.dataHeatBal->space(state.dataSurface->Surface(surf.ExtBoundCond).spaceNum).solarEnclosureNum =
                                otherSideEnclosureNum;
                        }
                    } else if (thisSideEnclosureNum != otherSideEnclosureNum) {
                        // If both sides are already assigned to an enclosure, then merge the two enclosures
                        auto const &thisEnclosure = Enclosures(thisSideEnclosureNum);
                        auto &otherEnclosure = Enclosures(otherSideEnclosureNum);
                        for (const auto &zName : thisEnclosure.spaceNames) {
                            otherEnclosure.spaceNames.push_back(zName);
                        }
                        for (int zNum : thisEnclosure.spaceNums) {
                            otherEnclosure.spaceNums.push_back(zNum);
                            if (radiantSetup) {
                                state.dataHeatBal->space(zNum).radiantEnclosureNum = otherSideEnclosureNum;
                            } else {
                                state.dataHeatBal->space(zNum).solarEnclosureNum = otherSideEnclosureNum;
                            }
                        }
                        otherEnclosure.FloorArea += thisEnclosure.FloorArea;
                        otherEnclosure.ExtWindowArea += thisEnclosure.ExtWindowArea;
                        otherEnclosure.TotalSurfArea += thisEnclosure.TotalSurfArea;
                        // Move any enclosures beyond thisEnclosure down one slot - at this point all enclosures are named "Radiant
                        // Enclosure N"
                        for (int enclNum = thisSideEnclosureNum; enclNum < enclosureNum; ++enclNum) {
                            std::string saveName = Enclosures(enclNum).Name;
                            Enclosures(enclNum) = Enclosures(enclNum + 1);
                            Enclosures(enclNum).Name = saveName;
                            for (int sNum : Enclosures(enclNum).spaceNums) {
                                if (radiantSetup) {
                                    state.dataHeatBal->space(sNum).radiantEnclosureNum = enclNum;
                                } else {
                                    state.dataHeatBal->space(sNum).solarEnclosureNum = enclNum;
                                }
                            }
                        }
                        // Clear the last rad enclosure and reduce the total number of enclosures by 1
                        Enclosures(enclosureNum).Name.clear();
                        Enclosures(enclosureNum).spaceNames.clear();
                        Enclosures(enclosureNum).spaceNums.clear();
                        Enclosures(enclosureNum).FloorArea = 0;
                        Enclosures(enclosureNum).ExtWindowArea = 0;
                        Enclosures(enclosureNum).TotalSurfArea = 0;
                        enclosureNum -= 1;
                    }
                    if (solarSetup && constr.TypeIsAirBoundaryMixing) {
                        // Set up mixing air boundaries only once, during solar setup
                        int spaceNum1 = min(surf.spaceNum, state.dataSurface->Surface(surf.ExtBoundCond).spaceNum);
                        int spaceNum2 = max(surf.spaceNum, state.dataSurface->Surface(surf.ExtBoundCond).spaceNum);
                        // This pair already saved?
                        bool found = false;
                        for (auto const &thisAirBoundaryMixing : state.dataHeatBal->airBoundaryMixing) {
                            if ((spaceNum1 == thisAirBoundaryMixing.space1) && (spaceNum2 == thisAirBoundaryMixing.space2)) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            // Store the space pairs, schedule, and flow rate to use later to create cross mixing objects
                            DataHeatBalance::AirBoundaryMixingSpecs newAirBoundaryMixing;
                            newAirBoundaryMixing.space1 = spaceNum1;
                            newAirBoundaryMixing.space2 = spaceNum2;
                            newAirBoundaryMixing.sched = state.dataConstruction->Construct(surf.Construction).airBoundaryMixingSched;
                            Real64 mixingVolume = state.dataConstruction->Construct(surf.Construction).AirBoundaryACH *
                                                  min(state.dataHeatBal->space(spaceNum1).Volume, state.dataHeatBal->space(spaceNum2).Volume) /
                                                  Constant::rSecsInHour;
                            newAirBoundaryMixing.mixingVolumeFlowRate = mixingVolume;
                            state.dataHeatBal->airBoundaryMixing.push_back(newAirBoundaryMixing);
                        }
                    }
                }
            }
            if (errorCount > 0) {
                ShowSevereError(state, format("{}: {} surfaces use Construction:AirBoundary in non-interzone surfaces.", RoutineName, errorCount));
                ShowContinueError(state, "For explicit details on each use, use Output:Diagnostics,DisplayExtraWarnings;");
            }
        }
        // Check for any spaces defined only by floor surface(s) and group them
        for (auto const &zone : state.dataHeatBal->Zone) {
            int newEnclosureNum = 0;
            for (int const spaceNum : zone.spaceIndexes) {
                int spaceEnclosureNum = 0;
                bool spaceHasOnlyFloors = false;
                if (radiantSetup) {
                    spaceEnclosureNum = state.dataHeatBal->space(spaceNum).radiantEnclosureNum;
                } else {
                    spaceEnclosureNum = state.dataHeatBal->space(spaceNum).solarEnclosureNum;
                }
                if (spaceEnclosureNum == 0) {
                    spaceHasOnlyFloors = true;
                    for (int const surfNum : state.dataHeatBal->space(spaceNum).surfaces) {
                        if (state.dataSurface->Surface(surfNum).Class == SurfaceClass::IntMass) {
                            continue;
                        }
                        if (state.dataSurface->Surface(surfNum).Class != SurfaceClass::Floor) {
                            spaceHasOnlyFloors = false;
                            break;
                        }
                    }
                }
                if (spaceEnclosureNum == 0 && spaceHasOnlyFloors) {
                    anyGroupedSpaces = true;
                    if (newEnclosureNum == 0) {
                        // Assign one new enclosure for all loose floors in this zone
                        ++enclosureNum;
                        newEnclosureNum = enclosureNum;
                    }
                    if (radiantSetup) {
                        state.dataHeatBal->space(spaceNum).radiantEnclosureNum = enclosureNum;
                    } else {
                        state.dataHeatBal->space(spaceNum).solarEnclosureNum = enclosureNum;
                    }
                    auto &thisEnclosure = Enclosures(enclosureNum);
                    // Give this enclosure the zone name and assign this to the zone-remainder space if it exists
                    thisEnclosure.Name = zone.Name;
                    thisEnclosure.spaceNames.push_back(state.dataHeatBal->space(spaceNum).Name);
                    thisEnclosure.spaceNums.push_back(spaceNum);
                    thisEnclosure.FloorArea = state.dataHeatBal->space(spaceNum).FloorArea;
                    thisEnclosure.ExtWindowArea = state.dataHeatBal->space(spaceNum).extWindowArea;
                    thisEnclosure.TotalSurfArea = state.dataHeatBal->space(spaceNum).totalSurfArea;
                }
            }
        }

        if (anyGroupedSpaces) {
            // All grouped spaces have been assigned to an enclosure, now assign remaining spaces
            for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
                auto &curSpace = state.dataHeatBal->space(spaceNum);
                int spaceEnclosureNum = 0;
                if (radiantSetup) {
                    spaceEnclosureNum = curSpace.radiantEnclosureNum;
                } else {
                    spaceEnclosureNum = curSpace.solarEnclosureNum;
                }
                if (spaceEnclosureNum == 0) {
                    if (Util::SameString(curSpace.Name, state.dataHeatBal->Zone(curSpace.zoneNum).Name + "-REMAINDER")) {
                        // Search for existing enclosure with same name as the zone
                        spaceEnclosureNum = Util::FindItemInList(state.dataHeatBal->Zone(curSpace.zoneNum).Name, Enclosures);
                    }
                    if (spaceEnclosureNum == 0) {
                        // Otherwise add a new one named for the space
                        ++enclosureNum;
                        spaceEnclosureNum = enclosureNum;
                        Enclosures(spaceEnclosureNum).Name = curSpace.Name;
                    }
                    if (radiantSetup) {
                        curSpace.radiantEnclosureNum = spaceEnclosureNum;
                    } else {
                        curSpace.solarEnclosureNum = spaceEnclosureNum;
                    }
                    auto &thisEnclosure = Enclosures(spaceEnclosureNum);
                    thisEnclosure.spaceNames.push_back(curSpace.Name);
                    thisEnclosure.spaceNums.push_back(spaceNum);
                    thisEnclosure.FloorArea += curSpace.FloorArea;
                    thisEnclosure.ExtWindowArea += curSpace.extWindowArea;
                    thisEnclosure.TotalSurfArea += curSpace.totalSurfArea;
                }
            }
            if (radiantSetup) {
                state.dataViewFactor->NumOfRadiantEnclosures = enclosureNum;
            } else {
                state.dataViewFactor->NumOfSolarEnclosures = enclosureNum;
            }
        } else {
            // There are no grouped radiant air boundaries, assign each space to it's own radiant enclosure
            for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
                auto &thisEnclosure = Enclosures(spaceNum);
                thisEnclosure.Name = state.dataHeatBal->space(spaceNum).Name;
                thisEnclosure.spaceNames.push_back(state.dataHeatBal->space(spaceNum).Name);
                thisEnclosure.spaceNums.push_back(spaceNum);
                thisEnclosure.FloorArea = state.dataHeatBal->space(spaceNum).FloorArea;
                if (radiantSetup) {
                    state.dataHeatBal->space(spaceNum).radiantEnclosureNum = spaceNum;
                } else {
                    state.dataHeatBal->space(spaceNum).solarEnclosureNum = spaceNum;
                    thisEnclosure.ExtWindowArea = state.dataHeatBal->space(spaceNum).extWindowArea;
                    thisEnclosure.TotalSurfArea = state.dataHeatBal->space(spaceNum).totalSurfArea;
                }
            }
            if (radiantSetup) {
                state.dataViewFactor->NumOfRadiantEnclosures = state.dataGlobal->numSpaces;
            } else {
                state.dataViewFactor->NumOfSolarEnclosures = state.dataGlobal->numSpaces;
            }
        }
        if (radiantSetup) {
            assert(state.dataViewFactor->NumOfRadiantEnclosures <= int(Enclosures.size()));
            Enclosures.resize(state.dataViewFactor->NumOfRadiantEnclosures);
        } else {
            assert(state.dataViewFactor->NumOfSolarEnclosures <= int(Enclosures.size()));
            Enclosures.resize(state.dataViewFactor->NumOfSolarEnclosures);
        }

        for (auto &thisEnclosure : state.dataViewFactor->EnclRadInfo) {
            SetupOutputVariable(state,
                                "Enclosure Mean Radiant Temperature",
                                Constant::Units::C,
                                thisEnclosure.MRT,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                thisEnclosure.Name);
        }

        // TODO MJW: For now, set the max and min enclosure numbers for each zone to be used in CalcInteriorRadExchange with ZoneToResimulate
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                if (state.dataHeatBal->Zone(zoneNum).zoneRadEnclosureFirst == -1) { // initial value
                    state.dataHeatBal->Zone(zoneNum).zoneRadEnclosureFirst = state.dataHeatBal->space(spaceNum).radiantEnclosureNum;
                } else {
                    state.dataHeatBal->Zone(zoneNum).zoneRadEnclosureFirst =
                        min(state.dataHeatBal->Zone(zoneNum).zoneRadEnclosureFirst, state.dataHeatBal->space(spaceNum).radiantEnclosureNum);
                }
                state.dataHeatBal->Zone(zoneNum).zoneRadEnclosureLast =
                    max(state.dataHeatBal->Zone(zoneNum).zoneRadEnclosureLast, state.dataHeatBal->space(spaceNum).radiantEnclosureNum);
            }
            assert(state.dataHeatBal->Zone(zoneNum).zoneRadEnclosureFirst != -1);
            assert(state.dataHeatBal->Zone(zoneNum).zoneRadEnclosureLast != -1);
            assert(state.dataHeatBal->Zone(zoneNum).zoneRadEnclosureFirst <= state.dataHeatBal->Zone(zoneNum).zoneRadEnclosureLast);
        }
    }

    void CheckConvexity(EnergyPlusData &state,
                        int const SurfNum, // Current surface number
                        int const NSides   // Number of sides to figure
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Tyler Hoyt
        //       DATE WRITTEN   December 2010
        //       MODIFIED       CR8752 - incorrect note of non-convex polygons

        // PURPOSE OF THIS SUBROUTINE: This subroutine verifies the convexity of a
        // surface that is exposed to the sun in the case that full shading calculations
        // are required. The calculation conveniently detects collinear points as well,
        // and returns a list of indices that are collinear within the plane of the surface.

        // METHODOLOGY EMPLOYED: First the surface is determined to have dimension 2 in
        // either the xy, yz, or xz plane. That plane is selected to do the testing.
        // Vectors representing the edges of the polygon and the perpendicular dot product
        // between adjacent edges are computed. This allows the turning angle to be determined.
        // If the turning angle is greater than pi/2, it turns to the right, and if it is
        // less than pi/2, it turns left. The direction of the turn is stored, and if it
        // changes as the edges are iterated the surface is not convex. Meanwhile it stores
        // the indices of vertices that are collinear and are later removed.

        // REFERENCES:
        // http://mathworld.wolfram.com/ConvexPolygon.html

        constexpr Real64 TurnThreshold = 0.000001; // Sensitivity of convexity test, in radians

        Real64 LastTheta = 0.0;                 // Angle between edge vectors
        bool SignFlag;                          // Direction of edge turn : true is right, false is left
        bool PrevSignFlag(false);               // Container for the sign of the previous iteration's edge turn
        bool PrevSignFlagInitialized(false);    // Whether we picked a PrevSignFlag already or not
        auto &X = state.dataSurfaceGeometry->X; // containers for x,y,z vertices of the surface
        auto &Y = state.dataSurfaceGeometry->Y;
        auto &Z = state.dataSurfaceGeometry->Z;
        auto &A = state.dataSurfaceGeometry->A; // containers for convexity test
        auto &B = state.dataSurfaceGeometry->B;
        auto &VertSize = state.dataSurfaceGeometry->VertSize; // size of X,Y,Z,A,B arrays

        std::vector<int> surfCollinearVerts; // index of vertices to remove, 1-indexed
        surfCollinearVerts.reserve(NSides + 2);

        if (state.dataSurfaceGeometry->CheckConvexityFirstTime) {
            X.allocate(state.dataSurface->MaxVerticesPerSurface + 2);
            Y.allocate(state.dataSurface->MaxVerticesPerSurface + 2);
            Z.allocate(state.dataSurface->MaxVerticesPerSurface + 2);
            A.allocate(state.dataSurface->MaxVerticesPerSurface + 2);
            B.allocate(state.dataSurface->MaxVerticesPerSurface + 2);
            VertSize = state.dataSurface->MaxVerticesPerSurface;
            state.dataSurfaceGeometry->CheckConvexityFirstTime = false;
        }

        if (NSides > VertSize) {
            X.deallocate();
            Y.deallocate();
            Z.deallocate();
            A.deallocate();
            B.deallocate();
            X.allocate(state.dataSurface->MaxVerticesPerSurface + 2);
            Y.allocate(state.dataSurface->MaxVerticesPerSurface + 2);
            Z.allocate(state.dataSurface->MaxVerticesPerSurface + 2);
            A.allocate(state.dataSurface->MaxVerticesPerSurface + 2);
            B.allocate(state.dataSurface->MaxVerticesPerSurface + 2);
            VertSize = state.dataSurface->MaxVerticesPerSurface;
        }

        auto &surfaceTmp = state.dataSurfaceGeometry->SurfaceTmp(SurfNum);
        auto &vertices = surfaceTmp.Vertex;

        for (int n = 1; n <= NSides; ++n) {
            X(n) = vertices(n).x;
            Y(n) = vertices(n).y;
            Z(n) = vertices(n).z;
        }
        X(NSides + 1) = vertices(1).x;
        Y(NSides + 1) = vertices(1).y;
        Z(NSides + 1) = vertices(1).z;
        X(NSides + 2) = vertices(2).x;
        Y(NSides + 2) = vertices(2).y;
        Z(NSides + 2) = vertices(2).z;

        // Determine a suitable plane in which to do the tests
        Real64 Det = 0.0;
        for (int n = 1; n <= NSides; ++n) {
            Det += X(n) * Y(n + 1) - X(n + 1) * Y(n);
        }
        if (std::abs(Det) > Constant::SmallDistance) {
            A = X;
            B = Y;
        } else {
            Det = 0.0;
            for (int n = 1; n <= NSides; ++n) {
                Det += X(n) * Z(n + 1) - X(n + 1) * Z(n);
            }
            if (std::abs(Det) > Constant::SmallDistance) {
                A = X;
                B = Z;
            } else {
                Det = 0.0;
                for (int n = 1; n <= NSides; ++n) {
                    Det += Y(n) * Z(n + 1) - Y(n + 1) * Z(n);
                }
                if (std::abs(Det) > Constant::SmallDistance) {
                    A = Y;
                    B = Z;
                } else {
                    // This condition should not be reached if the surfaces are guaranteed to be planar already
                    ShowSevereError(state, format("CheckConvexity: Surface=\"{}\" is non-planar.", surfaceTmp.Name));
                    ShowContinueError(state, "Coincident Vertices will be removed as possible.");
                    for (int n = 1; n <= surfaceTmp.Sides; ++n) {
                        auto const &point = vertices(n);
                        static constexpr std::string_view ErrFmt = " ({:8.3F},{:8.3F},{:8.3F})";
                        ShowContinueError(state, format(ErrFmt, point.x, point.y, point.z));
                    }
                }
            }
        }

        for (int n = 1; n <= NSides; ++n) { // perform convexity test in the plane determined above.

            DataVectorTypes::Vector_2d pt0(A(n), B(n));
            DataVectorTypes::Vector_2d pt1(A(n + 1), B(n + 1));
            DataVectorTypes::Vector_2d pt2(A(n + 2), B(n + 2));

            DataVectorTypes::Vector_2d V1 = pt1 - pt0;
            DataVectorTypes::Vector_2d V2 = pt2 - pt1;

            Real64 V1len = V1.length(); // = norm_L2()
            Real64 V2len = V2.length();
            if (V1len <= 1.e-8 || V2len <= 1.e-8) {
                // At least two points are coincident. Should this happen? GetVertices is supposed to pop these vertices
                continue;
            }
            Real64 CrossProd = V1.cross(V2);
            Real64 sinarg = CrossProd / (V1len * V2len);
            if (sinarg < -1.0) {
                sinarg = -1.0;
            } else if (sinarg > 1.0) {
                sinarg = 1.0;
            }
            Real64 Theta = std::asin(sinarg);
            if (Theta > TurnThreshold) {
                SignFlag = true;
            } else if (Theta < -TurnThreshold) {
                SignFlag = false;
            } else { // std::abs(Theta) < TurnThreshold
                // Store the index of the collinear vertex for removal
                int colinearIndex = n + 1;
                if (colinearIndex > NSides) {
                    colinearIndex -= NSides;
                }
                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowWarningError(
                        state,
                        format("CheckConvexity: Surface=\"{}\", vertex {} is colinear with previous and next.", surfaceTmp.Name, colinearIndex));
                }
                ++state.dataErrTracking->TotalCoincidentVertices;
                surfCollinearVerts.push_back(colinearIndex);
                continue;
            }

            if (!PrevSignFlagInitialized) {
                PrevSignFlag = SignFlag;
                LastTheta = Theta;
                PrevSignFlagInitialized = true;
                continue;
            }

            if (SignFlag != PrevSignFlag) {
                if (state.dataGlobal->DisplayExtraWarnings && surfaceTmp.ExtSolar &&
                    (state.dataHeatBal->SolarDistribution != DataHeatBalance::Shadowing::Minimal) && surfaceTmp.IsConvex &&
                    !state.dataSysVars->SutherlandHodgman &&
                    (state.dataSysVars->shadingMethod == DataSystemVariables::ShadingMethod::PolygonClipping)) {
                    ShowWarningError(state,
                                     format("CheckConvexity: Zone=\"{}\", Surface=\"{}\" is non-convex.",
                                            state.dataHeatBal->Zone(surfaceTmp.Zone).Name,
                                            surfaceTmp.Name));
                    int Np1 = n + 1;
                    if (Np1 > NSides) {
                        Np1 -= NSides;
                    }
                    int Np2 = n + 2;
                    if (Np2 > NSides) {
                        Np2 -= NSides;
                    }
                    ShowContinueError(state, format("...vertex {} to vertex {} to vertex {}", n, Np1, Np2));
                    ShowContinueError(state, format("...vertex {}=[{:.2R},{:.2R},{:.2R}]", n, X(n), Y(n), Z(n)));
                    ShowContinueError(state, format("...vertex {}=[{:.2R},{:.2R},{:.2R}]", Np1, X(n + 1), Y(n + 1), Z(n + 1)));
                    ShowContinueError(state, format("...vertex {}=[{:.2R},{:.2R},{:.2R}]", Np2, X(n + 2), Y(n + 2), Z(n + 2)));
                    // ShowContinueError(state, format("...theta angle=[{:.6R}]", Theta));
                    // ShowContinueError(state, format("...last theta angle=[{:.6R}]", LastTheta));
                }
                surfaceTmp.IsConvex = false;
                // #10103 - We do not want to break early, because we do want to consistently remove colinear vertices
                // to avoid potential vertex size mismatch fatal errors
                // break;
            }
            PrevSignFlag = SignFlag;
            LastTheta = Theta;
        }

        // must check to make sure don't remove NSides below 3
        int M = surfCollinearVerts.size();
        if (M > 0) { // Remove the collinear points determined above
            if (NSides - M >= 3) {
                surfaceTmp.Sides = NSides - M;
                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowWarningError(state,
                                     format("CheckConvexity: Surface=\"{}\" has [{}] collinear points that have been removed.", surfaceTmp.Name, M));
                }
            } else { // too many
                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowWarningError(state, format("CheckConvexity: Surface=\"{}\" has [{}] collinear points.", surfaceTmp.Name, M));
                    ShowContinueError(state, "...too many to remove all.  Will leave the surface with 3 sides. But this is now a degenerate surface");
                }
                ++state.dataErrTracking->TotalDegenerateSurfaces;
                surfaceTmp.Sides = 3; // max(NSides - M, 3) = 3 since NSide - M is < 3;
                surfCollinearVerts.resize(NSides - 3);
            }

            // remove duplicated points: For that we construct a new array of vertices, only copying indices that aren't in SurfCollinearVerts
            // Then we move that array into the original one
            Array1D<Vector> newVertices;
            newVertices.allocate(surfaceTmp.Sides);

            int n = 0;
            for (int i = 1; i <= NSides; ++i) {
                if (std::find(surfCollinearVerts.cbegin(), surfCollinearVerts.cend(), i) == surfCollinearVerts.cend()) {
                    newVertices(++n) = vertices(i);
                }
            }
            vertices = std::move(newVertices);

            if (state.dataGlobal->DisplayExtraWarnings) {
                ShowWarningError(state,
                                 format("CheckConvexity: Surface=\"{}\": The vertex points has been reprocessed as Sides = {}",
                                        surfaceTmp.Name,
                                        surfaceTmp.Sides));
            }
        }
    }

    bool isRectangle(EnergyPlusData &state, int const ThisSurf // Surface number
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         M.J. Witte
        //       DATE WRITTEN   October 2015

        // PURPOSE: Check if a 4-sided surface is a rectangle

        Real64 Diagonal1;                                            // Length of diagonal of 4-sided figure from vertex 1 to vertex 3 (m)
        Real64 Diagonal2;                                            // Length of diagonal of 4-sided figure from vertex 2 to vertex 4 (m)
        Real64 DotProd;                                              // Dot product of two adjacent sides - to test for right angle
        Real64 const cos89deg = std::cos(89.0 * Constant::DegToRad); // tolerance for right angle
        Vector Vect32;                                               // normalized vector from vertex 3 to vertex 2
        Vector Vect21;                                               // normalized vector from vertex 2 to vertex 1

        auto &surf = state.dataSurface->Surface(ThisSurf);
        Diagonal1 = Vectors::VecLength(surf.Vertex(1) - surf.Vertex(3));
        Diagonal2 = Vectors::VecLength(surf.Vertex(2) - surf.Vertex(4));
        // Test for rectangularity
        if (std::abs(Diagonal1 - Diagonal2) < 0.020) { // This tolerance based on coincident vertex tolerance of 0.01
            Vect32 = Vectors::VecNormalize(surf.Vertex(3) - surf.Vertex(2));
            Vect21 = Vectors::VecNormalize(surf.Vertex(2) - surf.Vertex(1));
            DotProd = dot(Vect32, Vect21);
            if (std::abs(DotProd) <= cos89deg) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    void MakeEquivalentRectangle(EnergyPlusData &state,
                                 int const SurfNum, // Surface number
                                 bool &ErrorsFound  // Error flag indicator (true if errors found)
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         R. Zhang, LBNL
        //       DATE WRITTEN   September 2016

        // PURPOSE OF THIS SUBROUTINE:
        // Processing of 4-sided but non-rectangular Window, Door or GlassDoor.
        // Calculate the effective height and width of the surface.
        //
        // METHODOLOGY EMPLOYED:
        // Transform the surface into an equivalent rectangular surface with the same area and aspect ratio.

        Real64 AspectRatio; // Aspect ratio
        Real64 WidthEff;    // Effective width of the surface
        Real64 WidthMax;    // X difference between the vertex on the most left and the one on the most right
        Real64 HeightEff;   // Effective height of the surface
        Real64 HeightMax;   // Y difference between the lowest and highest vertices
        Real64 Xp;
        Real64 Yp;
        Real64 Zp;
        Real64 XLLC;
        Real64 YLLC;
        Real64 ZLLC;

        if (SurfNum == 0) {
            // invalid surface
            ErrorsFound = true;
            return;
        }

        auto &surf = state.dataSurface->Surface(SurfNum);
        if (surf.Sides != 4) {
            // the method is designed for 4-sided surface
            return;
        } else if (isRectangle(state, SurfNum)) {
            // no need to transform
            return;
        }

        Real64 SurfWorldAz = surf.Azimuth;
        Real64 SurfTilt = surf.Tilt;
        Real64 BaseCosAzimuth = std::cos(SurfWorldAz * Constant::DegToRad);
        Real64 BaseSinAzimuth = std::sin(SurfWorldAz * Constant::DegToRad);
        Real64 BaseCosTilt = std::cos(SurfTilt * Constant::DegToRad);
        Real64 BaseSinTilt = std::sin(SurfTilt * Constant::DegToRad);
        int NumSurfSides = surf.Sides;

        // Calculate WidthMax and HeightMax
        WidthMax = 0.0;
        HeightMax = 0.0;
        for (int i = 1; i < NumSurfSides; ++i) {
            for (int j = i + 1; j <= NumSurfSides; ++j) {

                Xp = surf.Vertex(j).x - surf.Vertex(i).x;
                Yp = surf.Vertex(j).y - surf.Vertex(i).y;
                Zp = surf.Vertex(j).z - surf.Vertex(i).z;

                XLLC = -Xp * BaseCosAzimuth + Yp * BaseSinAzimuth;
                YLLC = -Xp * BaseSinAzimuth * BaseCosTilt - Yp * BaseCosAzimuth * BaseCosTilt + Zp * BaseSinTilt;
                ZLLC = Xp * BaseSinAzimuth * BaseSinTilt + Yp * BaseCosAzimuth * BaseSinTilt + Zp * BaseCosTilt;

                if (std::abs(XLLC) > WidthMax) {
                    WidthMax = std::abs(XLLC);
                }
                if (std::abs(YLLC) > WidthMax) {
                    HeightMax = std::abs(YLLC);
                }
            }
        }

        // Perform transformation by calculating WidthEff and HeightEff
        if ((WidthMax > 0) && (HeightMax > 0)) {
            AspectRatio = WidthMax / HeightMax;
        } else {
            AspectRatio = 1;
        }
        WidthEff = std::sqrt(surf.Area * AspectRatio);
        HeightEff = std::sqrt(surf.Area / AspectRatio);

        // Assign the effective width and length to the surface
        surf.Width = WidthEff;
        surf.Height = HeightEff;
    }

    void CheckForReversedLayers(EnergyPlusData &state,
                                bool &RevLayerDiffs,    // true when differences are discovered in interzone constructions
                                int const ConstrNum,    // construction index
                                int const ConstrNumRev, // construction index for reversed construction
                                int const TotalLayers   // total layers for construction definition
    )
    {
        auto &s_mat = state.dataMaterial;
        RevLayerDiffs = false;

        for (int LayerNo = 1; LayerNo <= TotalLayers; ++LayerNo) {

            int thisConstLayer = state.dataConstruction->Construct(ConstrNum).LayerPoint(LayerNo);
            int revConstLayer = state.dataConstruction->Construct(ConstrNumRev).LayerPoint(TotalLayers - LayerNo + 1);
            if (thisConstLayer == revConstLayer) {
                continue;
            }

            auto const *mat = s_mat->materials(thisConstLayer);
            auto const *matRev = s_mat->materials(revConstLayer);

            // If not point to the same layer, check to see if this is window glass which might need to have
            // front and back material properties reversed.
            Real64 constexpr SmallDiff = 0.0001;
            if ((mat->group == Material::Group::Glass) && (matRev->group == Material::Group::Glass)) {
                auto const *matGlass = dynamic_cast<Material::MaterialGlass const *>(mat);
                auto const *matGlassRev = dynamic_cast<Material::MaterialGlass const *>(matRev);
                assert(matGlass != nullptr);
                assert(matGlassRev != nullptr);

                // Both layers are window glass, so need to check to see if the properties are reversed
                if ((abs(matGlass->Thickness - matGlassRev->Thickness) > SmallDiff) ||
                    (abs(matGlass->ReflectSolBeamBack - matGlassRev->ReflectSolBeamFront) > SmallDiff) ||
                    (abs(matGlass->ReflectSolBeamFront - matGlassRev->ReflectSolBeamBack) > SmallDiff) ||
                    (abs(matGlass->TransVis - matGlassRev->TransVis) > SmallDiff) ||
                    (abs(matGlass->ReflectVisBeamBack - matGlassRev->ReflectVisBeamFront) > SmallDiff) ||
                    (abs(matGlass->ReflectVisBeamFront - matGlassRev->ReflectVisBeamBack) > SmallDiff) ||
                    (abs(matGlass->TransThermal - matGlassRev->TransThermal) > SmallDiff) ||
                    (abs(matGlass->AbsorpThermalBack - matGlassRev->AbsorpThermalFront) > SmallDiff) ||
                    (abs(matGlass->AbsorpThermalFront - matGlassRev->AbsorpThermalBack) > SmallDiff) ||
                    (abs(matGlass->Conductivity - matGlassRev->Conductivity) > SmallDiff) ||
                    (abs(matGlass->GlassTransDirtFactor - matGlassRev->GlassTransDirtFactor) > SmallDiff) ||
                    (matGlass->SolarDiffusing != matGlassRev->SolarDiffusing) ||
                    (abs(matGlass->YoungModulus - matGlassRev->YoungModulus) > SmallDiff) ||
                    (abs(matGlass->PoissonsRatio - matGlassRev->PoissonsRatio) > SmallDiff)) {
                    RevLayerDiffs = true;
                    break; // exit when diff
                } // If none of the above conditions is met, then these should be the same layers in reverse (RevLayersDiffs = false)
            } else if ((mat->group == Material::Group::GlassEQL) && (matRev->group == Material::Group::GlassEQL)) {
                auto const *matGlass = dynamic_cast<Material::MaterialGlassEQL const *>(mat);
                auto const *matGlassRev = dynamic_cast<Material::MaterialGlassEQL const *>(matRev);

                if ((abs(matGlass->TAR.Sol.Bk.Bm[0].BmTra - matGlassRev->TAR.Sol.Ft.Bm[0].BmTra) > SmallDiff) ||
                    (abs(matGlass->TAR.Sol.Ft.Bm[0].BmTra - matGlassRev->TAR.Sol.Bk.Bm[0].BmTra) > SmallDiff) ||
                    (abs(matGlass->TAR.Sol.Bk.Bm[0].BmRef - matGlassRev->TAR.Sol.Ft.Bm[0].BmRef) > SmallDiff) ||
                    (abs(matGlass->TAR.Sol.Ft.Bm[0].BmRef - matGlassRev->TAR.Sol.Bk.Bm[0].BmRef) > SmallDiff) ||
                    (abs(matGlass->TAR.Vis.Bk.Bm[0].BmTra - matGlassRev->TAR.Vis.Ft.Bm[0].BmTra) > SmallDiff) ||
                    (abs(matGlass->TAR.Vis.Ft.Bm[0].BmTra - matGlassRev->TAR.Vis.Bk.Bm[0].BmTra) > SmallDiff) ||
                    (abs(matGlass->TAR.Vis.Bk.Bm[0].BmRef - matGlassRev->TAR.Vis.Ft.Bm[0].BmRef) > SmallDiff) ||
                    (abs(matGlass->TAR.Vis.Ft.Bm[0].BmRef - matGlassRev->TAR.Vis.Bk.Bm[0].BmRef) > SmallDiff) ||
                    (abs(matGlass->TAR.Sol.Bk.Bm[0].DfTra - matGlassRev->TAR.Sol.Ft.Bm[0].DfTra) > SmallDiff) ||
                    (abs(matGlass->TAR.Sol.Ft.Bm[0].DfTra - matGlassRev->TAR.Sol.Bk.Bm[0].DfTra) > SmallDiff) ||
                    (abs(matGlass->TAR.Sol.Bk.Bm[0].DfRef - matGlassRev->TAR.Sol.Ft.Bm[0].DfRef) > SmallDiff) ||
                    (abs(matGlass->TAR.Sol.Ft.Bm[0].DfRef - matGlassRev->TAR.Sol.Bk.Bm[0].DfRef) > SmallDiff) ||
                    (abs(matGlass->TAR.Vis.Bk.Bm[0].DfTra - matGlassRev->TAR.Vis.Ft.Bm[0].DfTra) > SmallDiff) ||
                    (abs(matGlass->TAR.Vis.Ft.Bm[0].DfTra - matGlassRev->TAR.Vis.Bk.Bm[0].DfTra) > SmallDiff) ||
                    (abs(matGlass->TAR.Vis.Bk.Bm[0].DfRef - matGlassRev->TAR.Vis.Ft.Bm[0].DfRef) > SmallDiff) ||
                    (abs(matGlass->TAR.Vis.Ft.Bm[0].DfRef - matGlassRev->TAR.Vis.Bk.Bm[0].DfRef) > SmallDiff) ||
                    (abs(matGlass->TAR.Sol.Ft.Df.Tra - matGlassRev->TAR.Sol.Ft.Df.Tra) > SmallDiff) ||
                    (abs(matGlass->TAR.Sol.Bk.Df.Ref - matGlassRev->TAR.Sol.Ft.Df.Ref) > SmallDiff) ||
                    (abs(matGlass->TAR.Sol.Ft.Df.Ref - matGlassRev->TAR.Sol.Bk.Df.Ref) > SmallDiff) ||
                    (abs(matGlass->TAR.Vis.Ft.Df.Tra - matGlassRev->TAR.Vis.Ft.Df.Tra) > SmallDiff) ||
                    (abs(matGlass->TAR.Vis.Bk.Df.Ref - matGlassRev->TAR.Vis.Ft.Df.Ref) > SmallDiff) ||
                    (abs(matGlass->TAR.Vis.Ft.Df.Ref - matGlassRev->TAR.Vis.Bk.Df.Ref) > SmallDiff) ||
                    (abs(matGlass->TAR.IR.Ft.Tra - matGlassRev->TAR.IR.Ft.Tra) > SmallDiff) ||
                    (abs(matGlass->TAR.IR.Bk.Emi - matGlassRev->TAR.IR.Ft.Emi) > SmallDiff) ||
                    (abs(matGlass->TAR.IR.Ft.Emi - matGlassRev->TAR.IR.Bk.Emi) > SmallDiff) ||
                    (abs(matGlass->Resistance - matGlassRev->Resistance) > SmallDiff)) {
                    RevLayerDiffs = true;
                    break; // exit when diff
                } // If none of the above conditions is met, then these should be the same layers in reverse (RevLayersDiffs = false)

            } else {
                // Other material types do not have reversed constructions so if they are not the same layer there is a problem
                // (RevLayersDiffs = true)
                RevLayerDiffs = true;
                break; // exit when diff
            } // End check of whether or not these are WindowGlass
        }
    }

    void GetGeoSummaryRoof(EnergyPlusData const &state, GeoSummary &geoSummaryRoof)
    {
        std::vector<Vector> uniqueRoofVertices;
        std::vector<SurfaceGeometry::EdgeOfSurf> uniqEdgeOfSurfs; // I'm only partially using this
        for (const auto &surface : state.dataSurface->Surface) {

            if (surface.ExtBoundCond != DataSurfaces::ExternalEnvironment) {
                continue;
            }
            if (!surface.HeatTransSurf) {
                continue;
            }

            if (surface.Tilt > 45.0) { // TODO Double check tilt wrt outside vs inside?
                continue;
            }

            Real64 const z_min(minval(surface.Vertex, &Vector::z));
            Real64 const z_max(maxval(surface.Vertex, &Vector::z));
            Real64 const verticalHeight = z_max - z_min;
            geoSummaryRoof.Height += verticalHeight * surface.Area;
            geoSummaryRoof.Tilt += surface.Tilt * surface.Area;
            geoSummaryRoof.Azimuth += surface.Azimuth * surface.Area;
            geoSummaryRoof.Area += surface.Area;

            for (auto it = surface.Vertex.begin(); it != surface.Vertex.end(); ++it) {

                auto itnext = std::next(it);
                if (itnext == std::end(surface.Vertex)) {
                    itnext = std::begin(surface.Vertex);
                }

                auto &curVertex = *it;
                auto &nextVertex = *itnext;
                auto it2 = std::find_if(uniqueRoofVertices.begin(), uniqueRoofVertices.end(), [&curVertex](const auto &unqV) {
                    return SurfaceGeometry::isAlmostEqual3dPt(curVertex, unqV);
                });
                if (it2 == std::end(uniqueRoofVertices)) {
                    uniqueRoofVertices.emplace_back(curVertex);
                }

                SurfaceGeometry::EdgeOfSurf thisEdge;
                thisEdge.start = std::move(curVertex);
                thisEdge.end = std::move(nextVertex);
                thisEdge.count = 1;

                // Uses the custom operator== that uses isAlmostEqual3dPt internally and doesn't care about order of the start/end
                auto itEdge = std::find(uniqEdgeOfSurfs.begin(), uniqEdgeOfSurfs.end(), thisEdge);
                if (itEdge == uniqEdgeOfSurfs.end()) {
                    uniqEdgeOfSurfs.emplace_back(std::move(thisEdge));
                } else {
                    ++(itEdge->count);
                }
            }
        }

        if (geoSummaryRoof.Area > 0) {
            geoSummaryRoof.Height /= geoSummaryRoof.Area;
            geoSummaryRoof.Tilt /= geoSummaryRoof.Area;
            geoSummaryRoof.Azimuth /= geoSummaryRoof.Area;
        } else {
            geoSummaryRoof.Height = 0.0;
            geoSummaryRoof.Tilt = 0.0;
            geoSummaryRoof.Azimuth = 0.0;
        }

        // Remove the ones that are already used twice
        uniqEdgeOfSurfs.erase(
            std::remove_if(uniqEdgeOfSurfs.begin(), uniqEdgeOfSurfs.end(), [](const auto &edge) -> bool { return edge.count == 2; }),
            uniqEdgeOfSurfs.end());

        // Intersect with unique vertices as much as needed
        bool insertedVertext = true;
        while (insertedVertext) {
            insertedVertext = false;

            for (auto &edge : uniqEdgeOfSurfs) {

                // now go through all the vertices and see if they are colinear with start and end vertices
                for (const auto &testVertex : uniqueRoofVertices) {
                    if (edge.containsPoints(testVertex)) {
                        SurfaceGeometry::EdgeOfSurf newEdgeOfSurface;
                        newEdgeOfSurface.start = testVertex;
                        newEdgeOfSurface.end = edge.end;
                        edge.end = testVertex;
                        uniqEdgeOfSurfs.emplace_back(std::move(newEdgeOfSurface));
                        insertedVertext = true;
                        break;
                    }
                }
                // Break out of the loop on edges, and start again at the while
                if (insertedVertext) {
                    break;
                }
            }
        }

        // recount
        for (auto &edge : uniqEdgeOfSurfs) {
            edge.count = std::count(uniqEdgeOfSurfs.begin(), uniqEdgeOfSurfs.end(), edge);
        }

        uniqEdgeOfSurfs.erase(
            std::remove_if(uniqEdgeOfSurfs.begin(), uniqEdgeOfSurfs.end(), [](const auto &edge) -> bool { return edge.count == 2; }),
            uniqEdgeOfSurfs.end());

        geoSummaryRoof.Perimeter =
            std::accumulate(uniqEdgeOfSurfs.cbegin(), uniqEdgeOfSurfs.cend(), 0.0, [](const double &sum, const SurfaceGeometry::EdgeOfSurf &edge) {
                return sum + edge.length();
            });
    }

} // namespace SurfaceGeometry

} // namespace EnergyPlus
