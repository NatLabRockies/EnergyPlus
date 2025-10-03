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
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/numeric.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataDaylighting.hh>
#include <EnergyPlus/DataDaylightingDevices.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DaylightingDevices.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace Dayltg {

    // MODULE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   May 2003
    //       MODIFIED       PGE, Aug 2003:  Added daylighting shelves.
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // Simulates daylighting devices, namely tubular daylighting devices (a.k.a. light pipes, sun pipes, or
    // tubular skylights) and daylighting shelves (a.k.a. light shelves).

    // METHODOLOGY EMPLOYED:
    // TUBULAR DAYLIGHTING DEVICE
    // A tubular daylighting device (TDD) is constructed of three components:  a dome, a pipe, and a diffuser.
    // The dome and diffuser are treated as special window surfaces to take advantage of many of the already
    // existing daylighting and heat transfer routines.  Together the dome and diffuser become "receiver"
    // and "transmitter", i.e. radiation entering the dome ends up exiting the diffuser.  The geometry and
    // construction of the pipe and the constructions of the window surfaces determine the transmittance of
    // the TDD.
    // The main task of the module is to determine the total transmittance of the TDD for several
    // types of radiation, including visible beam, solar beam, solar isotropic, and solar anisotropic sky.
    // The fundamental building block for each type of radiation is the transmittance of a beam or ray of
    // radiation (visible or solar) at a given incident angle.  This transmittance is calculated and
    // tabulated for each TDD during initialization using a numerical integral based on the analytical
    // solution derived by Swift and Smith.  Diffuse transmittances are subsequently calculated by integrating
    // discrete rays over the viewable area.
    // There are three parts to the TDD model:
    //   1. Daylighting
    //   2. Solar gain
    //   3. Thermal conductive/convective gain
    // The daylighting simulation uses the visible beam transmittance to find the amount of direct beam
    // solar illuminance that enters the zone.  The visible beam transmittance is also used for calculating
    // the contribution of each discrete ray from a differential area during a comprehensive sky/ground
    // integration.
    // The heat balance simulation handles both the solar gain and thermal conductive/convective gain.
    // Although visible and solar radiation are similar, solar gain is simulated very differently from the
    // daylighting illuminance calculations.  The gain from direct beam solar is found using the
    // solar beam transmittance.  The diffuse solar, however, is more complicated.  A sky/ground integration
    // is NOT performed.  Instead anisotropic sky view factor multipliers (SurfAnisoSkyMult) are calculated for
    // each surface.  The diffuse sky/ground transmittance of the TDD is solved using a modification of the
    // SurfAnisoSkyMult.  The ground radiation transmittance and anisotropic sky transmittance are found separately.
    // See CalcTDDTransSolIso, CalcTDDTransSolHorizon, CalcTDDTransSolAniso below.
    // For thermal conductive/convective gain, TDDs are treated as one big object with an effective R value.
    // The outside face temperature of the dome and the inside face temperature of the diffuser are calculated
    // with the outside and inside heat balances respectively.  The temperatures are then copied to the inside
    // face of the dome and the outside face of the diffuser.  Normal exterior and interior convection and IR
    // radiation exchange occurs for both surfaces.
    // Solar radiation that is not transmitted through the pipe is absorbed and distributed among the transition
    // zones that the pipe passes through between dome and diffuser.  The heat is distributed proportionate to
    // the length of the zone.  Any exterior length of pipe also receives a proportionate amount of heat, but
    // this is lost to the outside.
    // REFERENCES:
    // Ellis, P. G., and Strand, R. K.  Paper to be published.
    // Swift, P. D., and Smith, G. B.  "Cylindrical Mirror Light Pipes",
    //   Solar Energy Materials and Solar Cells 36 (1995), pp. 159-168.
    // DAYLIGHTING SHELVES
    // A daylighting shelf is constructed of up to three components: a window, an inside shelf, and an outside
    // shelf.  Both inside shelf and outside shelf are optional, but if neither is specified, nothing happens.
    // The window must be divided into two window surfaces: an upper window and a lower window.  The upper
    // window interacts with the daylighting shelf but the lower window does not, except to receive shading from
    // the outside shelf.  The inside shelf, if specified, acts to reflect all transmitted light from the
    // upper window onto the ceiling of the zone as diffuse light.  The outside shelf, if specified, changes
    // the total amount of light incident on the window.  All light reflected from the outside shelf also goes
    // onto the zone ceiling.
    // Most of the work for daylighting shelves is actually done in DaylightingManager.cc, SolarShading.cc,
    // and HeatBalanceSurfaceManager.cc.  The main task of the module is to get the input and initialize the
    // shelf.  The biggest part of initialization is calculating the window view factor to the outside shelf.
    // It is up to the user to reduce the window view factor to ground accordingly.
    // The inside shelf is modeled in both daylighting and heat balance simulations by converting all light
    // transmitted by the upper window into diffuse upgoing flux.  No beam or downgoing flux can pass the end
    // of the shelf regardless of the shelf's position or orientation.  Since it is defined as a partition,
    // the inside shelf is essentially the same as an internal mass surface.  The initialization doubles the
    // surface area so that both sides are exposed to the zone air.  All beam solar transmitted by the window
    // is absorbed in one side of the shelf, i.e. half of the doubled area.
    // The outside shelf is modeled in the daylighting simulation after the detailed sky/ground integration has
    // been completed.  Since exterior surfaces currently do not reflect or have a luminance in the Daylighting
    // Manager, the shelf just serves to block part of the ground luminance.  The luminance of the shelf itself
    // is added as a lump sum based on the view factor to the shelf, the sunlit fraction, the reflectance of the
    // shelf construction, and the sun and sky illuminance on the shelf.  All the luminance is added to the
    // diffuse upgoing flux.  The shelf view factor to sky is assumed to be 1.0 for lack of better information.
    // The outside shelf is treated similarly in the heat balance simulation, but here the shelf view factor to
    // sky is conveniently given by SurfAnisoSkyMult.  NOTE:  The solar shading code was modified to allow sunlit
    // fraction, sunlit area, SurfAnisoSkyMult, etc. to be calculated for attached shading surfaces.
    // Future shelf model improvements:
    // 1. Allow beam and downgoing flux to pass the end of the inside shelf depending on actual shelf geometry.
    // 2. Reduce outside shelf view factor to sky (for daylighting) by taking into account anisotropic sky
    //    distribution and shading, i.e. the daylighting equivalent of SurfAnisoSkyMult.
    // 3. Expand view factor to shelf calculation to handle more complicated geometry.
    // REFERENCES:
    // Mills, A. F.  Heat and Mass Transfer, 1995, p. 499.  (Shape factor for adjacent rectangles.)

    // Using/Aliasing
    using DataSurfaces::ExternalEnvironment;
    using DataSurfaces::SurfaceClass;

    void InitDaylightingDevices(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   May 2003
        //       MODIFIED       PGE, Aug 2003:  Added daylighting shelves.

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes all daylighting device:  TDD pipes and daylighting shelves.
        // This is only called once at the beginning of the simulation under the BeginSimFlag.

        // METHODOLOGY EMPLOYED:
        // Daylighting and thermal variables are calculated.  BeamTrans/COSAngle table is calculated.

        struct TDDPipeStoredData
        {
            // Members
            Real64 AspectRatio;        // Aspect ratio, length / diameter
            Real64 Reflectance;        // Reflectance of surface
            Array1D<Real64> TransBeam; // Table of beam transmittance vs. cosine angle

            // Default Constructor
            TDDPipeStoredData() : AspectRatio(0.0), Reflectance(0.0), TransBeam(NumOfAngles, 0.0)
            {
            }
        };

        // Object Data
        Array1D<TDDPipeStoredData> TDDPipeStored;

        // Initialize tubular daylighting devices (TDDs)
        GetTDDInput(state);

        if ((int)state.dataDaylightingDevicesData->TDDPipe.size() > 0) {
            DisplayString(state, "Initializing Tubular Daylighting Devices");
            // Setup COSAngle list for all TDDs
            state.dataDaylightingDevices->COSAngle(1) = 0.0;
            state.dataDaylightingDevices->COSAngle(NumOfAngles) = 1.0;

            Real64 dTheta = 90.0 * Constant::DegToRad / (NumOfAngles - 1.0);
            Real64 Theta = 90.0 * Constant::DegToRad;
            for (int AngleNum = 2; AngleNum <= NumOfAngles - 1; ++AngleNum) {
                Theta -= dTheta;
                state.dataDaylightingDevices->COSAngle(AngleNum) = std::cos(Theta);
            } // AngleNum

            TDDPipeStored.allocate((int)state.dataDaylightingDevicesData->TDDPipe.size() * 2);

            for (int PipeNum = 1; PipeNum <= (int)state.dataDaylightingDevicesData->TDDPipe.size(); ++PipeNum) {
                // Initialize optical properties
                state.dataDaylightingDevicesData->TDDPipe(PipeNum).AspectRatio =
                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TotLength / state.dataDaylightingDevicesData->TDDPipe(PipeNum).Diameter;
                state.dataDaylightingDevicesData->TDDPipe(PipeNum).ReflectVis =
                    1.0 - state.dataConstruction->Construct(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Construction).InsideAbsorpVis;
                state.dataDaylightingDevicesData->TDDPipe(PipeNum).ReflectSol =
                    1.0 - state.dataConstruction->Construct(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Construction).InsideAbsorpSolar;

                // Calculate the beam transmittance table for visible and solar spectrum
                // First time thru use the visible reflectance
                Real64 Reflectance = state.dataDaylightingDevicesData->TDDPipe(PipeNum).ReflectVis;
                int NumStored = 0; // Counter for number of pipes stored as they are calculated
                int StoredNum;     // Stored TDD pipe object number
                for (int Loop = 1; Loop <= 2; ++Loop) {
                    // For computational efficiency, search stored pipes to see if an identical pipe has already been calculated
                    bool Found = false;
                    for (StoredNum = 1; StoredNum <= NumStored; ++StoredNum) {
                        if (TDDPipeStored(StoredNum).AspectRatio != state.dataDaylightingDevicesData->TDDPipe(PipeNum).AspectRatio) {
                            continue;
                        }
                        if (TDDPipeStored(StoredNum).Reflectance == Reflectance) {
                            Found = true; // StoredNum points to the matching TDDPipeStored
                            break;
                        }
                    } // StoredNum

                    if (!Found) { // Not yet calculated

                        // Add a new pipe to TDDPipeStored
                        ++NumStored;
                        TDDPipeStored(NumStored).AspectRatio = state.dataDaylightingDevicesData->TDDPipe(PipeNum).AspectRatio;
                        TDDPipeStored(NumStored).Reflectance = Reflectance;

                        // Set beam transmittances for 0 and 90 degrees
                        TDDPipeStored(NumStored).TransBeam(1) = 0.0;
                        TDDPipeStored(NumStored).TransBeam(NumOfAngles) = 1.0;

                        // Calculate intermediate beam transmittances between 0 and 90 degrees
                        Theta = 90.0 * Constant::DegToRad;
                        for (int AngleNum = 2; AngleNum <= NumOfAngles - 1; ++AngleNum) {
                            Theta -= dTheta;
                            TDDPipeStored(NumStored).TransBeam(AngleNum) =
                                CalcPipeTransBeam(Reflectance, state.dataDaylightingDevicesData->TDDPipe(PipeNum).AspectRatio, Theta);
                        } // AngleNum

                        StoredNum = NumStored;
                    }

                    // Assign stored values to TDDPipe
                    if (Loop == 1) { // Visible
                        state.dataDaylightingDevicesData->TDDPipe(PipeNum).PipeTransVisBeam = TDDPipeStored(StoredNum).TransBeam;
                    } else { // Solar
                        state.dataDaylightingDevicesData->TDDPipe(PipeNum).PipeTransSolBeam = TDDPipeStored(StoredNum).TransBeam;
                    }

                    // Second time thru use the solar reflectance
                    Reflectance = state.dataDaylightingDevicesData->TDDPipe(PipeNum).ReflectSol;
                } // Loop

                // Calculate the solar isotropic diffuse and horizon transmittances.  These values are constant for a given TDD.
                state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransSolIso = CalcTDDTransSolIso(state, PipeNum);
                state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransSolHorizon = CalcTDDTransSolHorizon(state, PipeNum);

                // Initialize thermal properties
                Real64 SumTZoneLengths = 0.0;
                for (int TZoneNum = 1; TZoneNum <= state.dataDaylightingDevicesData->TDDPipe(PipeNum).NumOfTZones; ++TZoneNum) {
                    SumTZoneLengths += state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZoneLength(TZoneNum);

                    SetupZoneInternalGain(state,
                                          state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZone(TZoneNum),
                                          state.dataDaylightingDevicesData->TDDPipe(PipeNum).Name,
                                          DataHeatBalance::IntGainType::DaylightingDeviceTubular,
                                          &state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZoneHeatGain(TZoneNum));

                } // TZoneNum

                state.dataDaylightingDevicesData->TDDPipe(PipeNum).ExtLength =
                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TotLength - SumTZoneLengths;

                // Setup report variables: CurrentModuleObject='DaylightingDevice:Tubular'
                SetupOutputVariable(state,
                                    "Tubular Daylighting Device Transmitted Solar Radiation Rate",
                                    Constant::Units::W,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransmittedSolar,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).Name);
                SetupOutputVariable(state,
                                    "Tubular Daylighting Device Pipe Absorbed Solar Radiation Rate",
                                    Constant::Units::W,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).PipeAbsorbedSolar,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).Name);
                SetupOutputVariable(state,
                                    "Tubular Daylighting Device Heat Gain Rate",
                                    Constant::Units::W,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).HeatGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).Name);
                SetupOutputVariable(state,
                                    "Tubular Daylighting Device Heat Loss Rate",
                                    Constant::Units::W,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).HeatLoss,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).Name);

                SetupOutputVariable(state,
                                    "Tubular Daylighting Device Beam Solar Transmittance",
                                    Constant::Units::None,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransSolBeam,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).Name);
                SetupOutputVariable(state,
                                    "Tubular Daylighting Device Beam Visible Transmittance",
                                    Constant::Units::None,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransVisBeam,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).Name);
                SetupOutputVariable(state,
                                    "Tubular Daylighting Device Diffuse Solar Transmittance",
                                    Constant::Units::None,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransSolDiff,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).Name);
                SetupOutputVariable(state,
                                    "Tubular Daylighting Device Diffuse Visible Transmittance",
                                    Constant::Units::None,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransVisDiff,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).Name);

            } // PipeNum

            TDDPipeStored.deallocate();
        }

        // Initialize daylighting shelves
        GetShelfInput(state);

        if ((int)state.dataDaylightingDevicesData->Shelf.size() > 0) {
            DisplayString(state, "Initializing Light Shelf Daylighting Devices");
        }

        for (int ShelfNum = 1; ShelfNum <= (int)state.dataDaylightingDevicesData->Shelf.size(); ++ShelfNum) {
            int WinSurf = state.dataDaylightingDevicesData->Shelf(ShelfNum).Window;

            int ShelfSurf = state.dataDaylightingDevicesData->Shelf(ShelfNum).InSurf;
            if (ShelfSurf > 0) {
                // Double surface area so that both sides of the shelf are treated as internal mass
                state.dataSurface->Surface(ShelfSurf).Area *= 2.0;
            }

            ShelfSurf = state.dataDaylightingDevicesData->Shelf(ShelfNum).OutSurf;
            if (ShelfSurf > 0) {
                state.dataDaylightingDevicesData->Shelf(ShelfNum).OutReflectVis =
                    1.0 - state.dataConstruction->Construct(state.dataDaylightingDevicesData->Shelf(ShelfNum).Construction).OutsideAbsorpVis;
                state.dataDaylightingDevicesData->Shelf(ShelfNum).OutReflectSol =
                    1.0 - state.dataConstruction->Construct(state.dataDaylightingDevicesData->Shelf(ShelfNum).Construction).OutsideAbsorpSolar;

                if (state.dataDaylightingDevicesData->Shelf(ShelfNum).ViewFactor < 0) {
                    CalcViewFactorToShelf(state, ShelfNum);
                }

                adjustViewFactorsWithShelf(state,
                                           state.dataDaylightingDevicesData->Shelf(ShelfNum).ViewFactor,
                                           state.dataSurface->Surface(WinSurf).ViewFactorSky,
                                           state.dataSurface->Surface(WinSurf).ViewFactorGround,
                                           WinSurf,
                                           ShelfNum);

                // Report calculated view factor so that user knows what to make the view factor to ground
                if (!state.dataDaylightingDevices->ShelfReported) {
                    print(state.files.eio,
                          "! <Shelf Details>,Name,View Factor to Outside Shelf,Window Name,Window View Factor to Sky,Window View Factor to Ground\n");
                    state.dataDaylightingDevices->ShelfReported = true;
                }
                print(state.files.eio,
                      "Shelf Details,{},{:.2R},{},{:.2R},{:.2R}\n",
                      state.dataDaylightingDevicesData->Shelf(ShelfNum).Name,
                      state.dataDaylightingDevicesData->Shelf(ShelfNum).ViewFactor,
                      state.dataSurface->Surface(WinSurf).Name,
                      state.dataSurface->Surface(WinSurf).ViewFactorSky,
                      state.dataSurface->Surface(WinSurf).ViewFactorGround);
                //      CALL Setup Output Variable(state, 'View Factor To Outside Shelf []', &
                //        Shelf(ShelfNum)%ViewFactor,'Zone','Average',Shelf(ShelfNum)%Name)
            }
        }

        // Warning that if Calculate Solar Reflection From Exterior Surfaces = Yes in Building input, then
        // solar reflection calculated from obstructions will not be used in daylighting shelf or tubular device
        // calculation

        if (state.dataSurface->CalcSolRefl &&
            ((int)state.dataDaylightingDevicesData->TDDPipe.size() > 0 || (int)state.dataDaylightingDevicesData->Shelf.size() > 0)) {
            ShowWarningError(state, "InitDaylightingDevices: Solar Distribution Model includes Solar Reflection calculations;");
            ShowContinueError(state, "the resulting reflected solar values will not be used in the");
            ShowContinueError(state, "DaylightingDevice:Shelf or DaylightingDevice:Tubular calculations.");
        }
    }

    void GetTDDInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   May 2003

        // PURPOSE OF THIS SUBROUTINE:
        // Gets the input for TDD pipes and does some error checking.

        // METHODOLOGY EMPLOYED:
        // Standard EnergyPlus methodology.

        auto &ipsc = state.dataIPShortCut;
        auto &cCurrentModuleObject = ipsc->cCurrentModuleObject;

        cCurrentModuleObject = "DaylightingDevice:Tubular";
        int NumOfTDDPipes = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (NumOfTDDPipes > 0) {
            state.dataDaylightingDevicesData->TDDPipe.allocate(NumOfTDDPipes);
            int IOStatus;   // Used in GetObjectItem
            int NumAlphas;  // Number of Alphas for each GetObjectItem call
            int NumNumbers; // Number of Numbers for each GetObjectItem call

            for (int PipeNum = 1; PipeNum <= NumOfTDDPipes; ++PipeNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         PipeNum,
                                                                         ipsc->cAlphaArgs,
                                                                         NumAlphas,
                                                                         ipsc->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         ipsc->lNumericFieldBlanks,
                                                                         ipsc->lAlphaFieldBlanks,
                                                                         ipsc->cAlphaFieldNames,
                                                                         ipsc->cNumericFieldNames);
                // Pipe name
                state.dataDaylightingDevicesData->TDDPipe(PipeNum).Name = ipsc->cAlphaArgs(1);

                // Get TDD:DOME object
                int SurfNum = Util::FindItemInList(ipsc->cAlphaArgs(2), state.dataSurface->Surface);

                if (SurfNum == 0) {
                    ShowSevereError(state, format("{} = {}:  Dome {} not found.", cCurrentModuleObject, ipsc->cAlphaArgs(1), ipsc->cAlphaArgs(2)));
                    state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                } else {
                    if (FindTDDPipe(state, SurfNum) > 0) {
                        ShowSevereError(state,
                                        format("{} = {}:  Dome {} is referenced by more than one TDD.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(2)));
                        state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                    }

                    if (state.dataSurface->Surface(SurfNum).Class != SurfaceClass::TDD_Dome) {
                        ShowSevereError(state,
                                        format("{} = {}:  Dome {} is not of surface type TubularDaylightDome.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(2)));
                        state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                    }

                    if (state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).TotGlassLayers > 1) {
                        ShowSevereError(state,
                                        format("{} = {}:  Dome {} construction ({}) must have only 1 glass layer.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(2),
                                               state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).Name));
                        state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                    }

                    if (state.dataSurface->Surface(SurfNum).HasShadeControl) {
                        ShowSevereError(state,
                                        format("{} = {}:  Dome {} must not have a shading control.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(2)));
                        state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                    }

                    if (state.dataSurface->Surface(SurfNum).FrameDivider > 0) {
                        ShowSevereError(
                            state,
                            format(
                                "{} = {}:  Dome {} must not have a frame/divider.", cCurrentModuleObject, ipsc->cAlphaArgs(1), ipsc->cAlphaArgs(2)));
                        state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                    }

                    if (state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).WindowTypeEQL) {
                        ShowSevereError(state,
                                        format("{} = {}:  Dome {} Equivalent Layer Window is not supported.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(2)));
                        state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                    }
                    // Window multiplier is already handled in SurfaceGeometry.cc

                    if (!state.dataSurface->Surface(SurfNum).ExtSolar) {
                        ShowWarningError(state,
                                         format("{} = {}:  Dome {} is not exposed to exterior radiation.",
                                                cCurrentModuleObject,
                                                ipsc->cAlphaArgs(1),
                                                ipsc->cAlphaArgs(2)));
                    }

                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome = SurfNum;
                    state.dataSurface->SurfWinTDDPipeNum(SurfNum) = PipeNum;
                }

                // Get TDD:DIFFUSER object
                SurfNum = Util::FindItemInList(ipsc->cAlphaArgs(3), state.dataSurface->Surface);

                if (SurfNum == 0) {
                    ShowSevereError(state,
                                    format("{} = {}:  Diffuser {} not found.", cCurrentModuleObject, ipsc->cAlphaArgs(1), ipsc->cAlphaArgs(3)));
                    state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                } else {
                    if (FindTDDPipe(state, SurfNum) > 0) {
                        ShowSevereError(state,
                                        format("{} = {}:  Diffuser {} is referenced by more than one TDD.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(3)));
                        state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                    }

                    if (state.dataSurface->Surface(SurfNum).OriginalClass != SurfaceClass::TDD_Diffuser) {
                        ShowSevereError(state,
                                        format("{} = {}:  Diffuser {} is not of surface type TubularDaylightDiffuser.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(3)));
                        state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                    }

                    if (state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).TotGlassLayers > 1) {
                        ShowSevereError(state,
                                        format("{} = {}:  Diffuser {} construction ({}) must have only 1 glass layer.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(3),
                                               state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).Name));
                        state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                    }

                    if (state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).TransDiff <= 1.0e-10) {
                        ShowSevereError(state,
                                        format("{} = {}:  Diffuser {} construction ({}) invalid value.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(3),
                                               state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).Name));
                        ShowContinueError(state,
                                          format("Diffuse solar transmittance of construction [{:.4R}] too small for calculations.",
                                                 state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).TransDiff));
                        state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                    }

                    if (state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome > 0 &&
                        std::abs(state.dataSurface->Surface(SurfNum).Area -
                                 state.dataSurface->Surface(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome).Area) > 0.1) {
                        if (General::SafeDivide(std::abs(state.dataSurface->Surface(SurfNum).Area -
                                                         state.dataSurface->Surface(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome).Area),
                                                state.dataSurface->Surface(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome).Area) >
                            0.1) { // greater than 10%
                            ShowSevereError(state,
                                            format("{} = {}:  Dome and diffuser areas are significantly different (>10%).",
                                                   cCurrentModuleObject,
                                                   ipsc->cAlphaArgs(1)));
                            ShowContinueError(state,
                                              format("...Diffuser Area=[{:.4R}]; Dome Area=[{:.4R}].",
                                                     state.dataSurface->Surface(SurfNum).Area,
                                                     state.dataSurface->Surface(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome).Area));
                            state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                        } else {
                            ShowWarningError(
                                state, format("{} = {}:  Dome and diffuser areas differ by > .1 m2.", cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                            ShowContinueError(state,
                                              format("...Diffuser Area=[{:.4R}]; Dome Area=[{:.4R}].",
                                                     state.dataSurface->Surface(SurfNum).Area,
                                                     state.dataSurface->Surface(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome).Area));
                        }
                    }

                    if (state.dataSurface->Surface(SurfNum).HasShadeControl) {
                        ShowSevereError(state,
                                        format("{} = {}:  Diffuser {} must not have a shading control.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(3)));
                        state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                    }

                    if (state.dataSurface->Surface(SurfNum).FrameDivider > 0) {
                        ShowSevereError(state,
                                        format("{} = {}:  Diffuser {} must not have a frame/divider.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(3)));
                        state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                    }

                    if (state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).WindowTypeEQL) {
                        ShowSevereError(state,
                                        format("{} = {}:  Diffuser {} Equivalent Layer Window is not supported.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(2)));
                        state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                    }

                    // Window multiplier is already handled in SurfaceGeometry.cc

                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).Diffuser = SurfNum;
                    state.dataSurface->SurfWinTDDPipeNum(SurfNum) = PipeNum;
                }

                // Construction
                state.dataDaylightingDevicesData->TDDPipe(PipeNum).Construction =
                    Util::FindItemInList(ipsc->cAlphaArgs(4), state.dataConstruction->Construct);

                if (state.dataDaylightingDevicesData->TDDPipe(PipeNum).Construction == 0) {
                    ShowSevereError(
                        state, format("{} = {}:  Pipe construction {} not found.", cCurrentModuleObject, ipsc->cAlphaArgs(1), ipsc->cAlphaArgs(4)));
                    state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                } else {
                    state.dataConstruction->Construct(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Construction).IsUsed = true;
                }

                if (ipsc->rNumericArgs(1) > 0) {
                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).Diameter = ipsc->rNumericArgs(1);
                } else {
                    ShowSevereError(state, format("{} = {}:  Pipe diameter must be greater than zero.", cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                }

                Real64 PipeArea = 0.25 * Constant::Pi * pow_2(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Diameter);
                if (state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome > 0 &&
                    std::abs(PipeArea - state.dataSurface->Surface(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome).Area) > 0.1) {
                    if (General::SafeDivide(
                            std::abs(PipeArea - state.dataSurface->Surface(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome).Area),
                            state.dataSurface->Surface(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome).Area) > 0.1) { // greater than 10%
                        ShowSevereError(state,
                                        format("{} = {}:  Pipe and dome/diffuser areas are significantly different (>10%).",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1)));
                        ShowContinueError(state,
                                          format("...Pipe Area=[{:.4R}]; Dome/Diffuser Area=[{:.4R}].",
                                                 PipeArea,
                                                 state.dataSurface->Surface(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome).Area));
                        state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                    } else {
                        ShowWarningError(
                            state, format("{} = {}:  Pipe and dome/diffuser areas differ by > .1 m2.", cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                        ShowContinueError(state,
                                          format("...Pipe Area=[{:.4R}]; Dome/Diffuser Area=[{:.4R}].",
                                                 PipeArea,
                                                 state.dataSurface->Surface(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome).Area));
                    }
                }

                if (ipsc->rNumericArgs(2) > 0) {
                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TotLength = ipsc->rNumericArgs(2);
                } else {
                    ShowSevereError(state, format("{} = {}:  Pipe length must be greater than zero.", cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                }

                if (ipsc->rNumericArgs(3) > 0) {
                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).Reff = ipsc->rNumericArgs(3);
                } else {
                    ShowSevereError(state,
                                    format("{} = {}:  Effective thermal resistance (R value) must be greater than zero.",
                                           cCurrentModuleObject,
                                           ipsc->cAlphaArgs(1)));
                    state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                }

                // Transition zones
                state.dataDaylightingDevicesData->TDDPipe(PipeNum).NumOfTZones = NumAlphas - 4;

                if (state.dataDaylightingDevicesData->TDDPipe(PipeNum).NumOfTZones < 1) {
                    ShowWarningError(state,
                                     format("{} = {}:  No transition zones specified.  All pipe absorbed solar goes to exterior.",
                                            cCurrentModuleObject,
                                            ipsc->cAlphaArgs(1)));
                } else if (state.dataDaylightingDevicesData->TDDPipe(PipeNum).NumOfTZones > MaxTZones) {
                    ShowSevereError(state,
                                    format("{} = {}:  Maximum number of transition zones exceeded.", cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                } else {
                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZone.allocate(state.dataDaylightingDevicesData->TDDPipe(PipeNum).NumOfTZones);
                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZoneLength.allocate(
                        state.dataDaylightingDevicesData->TDDPipe(PipeNum).NumOfTZones);
                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZoneHeatGain.allocate(
                        state.dataDaylightingDevicesData->TDDPipe(PipeNum).NumOfTZones);

                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZone = 0;
                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZoneLength = 0.0;
                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZoneHeatGain = 0.0;

                    for (int TZoneNum = 1; TZoneNum <= state.dataDaylightingDevicesData->TDDPipe(PipeNum).NumOfTZones; ++TZoneNum) {
                        std::string const TZoneName = ipsc->cAlphaArgs(TZoneNum + 4);
                        state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZone(TZoneNum) = Util::FindItemInList(TZoneName, state.dataHeatBal->Zone);
                        if (state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZone(TZoneNum) == 0) {
                            ShowSevereError(state,
                                            format("{} = {}:  Transition zone {} not found.", cCurrentModuleObject, ipsc->cAlphaArgs(1), TZoneName));
                            state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                        }

                        state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZoneLength(TZoneNum) = ipsc->rNumericArgs(TZoneNum + 3);
                        if (state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZoneLength(TZoneNum) < 0) {
                            ShowSevereError(state,
                                            format("{} = {}:  Transition zone length for {} must be zero or greater.",
                                                   cCurrentModuleObject,
                                                   ipsc->cAlphaArgs(1),
                                                   TZoneName));
                            state.dataDaylightingDevices->GetTDDInputErrorsFound = true;
                        }
                    } // TZoneNum
                }

            } // PipeNum

            if (state.dataDaylightingDevices->GetTDDInputErrorsFound) {
                ShowFatalError(state, "Errors in DaylightingDevice:Tubular input.");
            }
            state.dataDayltg->TDDTransVisBeam.allocate(Constant::iHoursInDay, NumOfTDDPipes);
            state.dataDayltg->TDDFluxInc.allocate(Constant::iHoursInDay, NumOfTDDPipes);
            state.dataDayltg->TDDFluxTrans.allocate(Constant::iHoursInDay, NumOfTDDPipes);
            for (int hr = 1; hr <= Constant::iHoursInDay; ++hr) {
                for (int tddNum = 1; tddNum <= NumOfTDDPipes; ++tddNum) {
                    state.dataDayltg->TDDTransVisBeam(hr, tddNum) = 0.0;
                    state.dataDayltg->TDDFluxInc(hr, tddNum) = Illums();
                    state.dataDayltg->TDDFluxTrans(hr, tddNum) = Illums();
                } // for (tddNum)
            } // for (hr)
        }
    }

    void GetShelfInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   August 2003

        // PURPOSE OF THIS SUBROUTINE:
        // Gets the input for light shelves and does some error checking.

        // METHODOLOGY EMPLOYED:
        // Standard EnergyPlus methodology.

        auto &ipsc = state.dataIPShortCut;
        auto &cCurrentModuleObject = ipsc->cCurrentModuleObject;

        cCurrentModuleObject = "DaylightingDevice:Shelf";
        int NumOfShelf = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (NumOfShelf > 0) {
            state.dataDaylightingDevicesData->Shelf.allocate(NumOfShelf);
            int NumAlphas;  // Number of Alphas for each GetObjectItem call
            int NumNumbers; // Number of Numbers for each GetObjectItem call
            int IOStatus;   // Used in GetObjectItem

            for (int ShelfNum = 1; ShelfNum <= NumOfShelf; ++ShelfNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         ShelfNum,
                                                                         ipsc->cAlphaArgs,
                                                                         NumAlphas,
                                                                         ipsc->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         ipsc->lNumericFieldBlanks,
                                                                         ipsc->lAlphaFieldBlanks,
                                                                         ipsc->cAlphaFieldNames,
                                                                         ipsc->cNumericFieldNames);
                // Shelf name
                state.dataDaylightingDevicesData->Shelf(ShelfNum).Name = ipsc->cAlphaArgs(1);

                // Get window object
                int SurfNum = Util::FindItemInList(ipsc->cAlphaArgs(2), state.dataSurface->Surface);

                if (SurfNum == 0) {
                    ShowSevereError(state, format("{} = {}:  Window {} not found.", cCurrentModuleObject, ipsc->cAlphaArgs(1), ipsc->cAlphaArgs(2)));
                    state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                } else {
                    if (state.dataSurface->Surface(SurfNum).Class != SurfaceClass::Window) {
                        ShowSevereError(state,
                                        format("{} = {}:  Window {} is not of surface type WINDOW.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(2)));
                        state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                    }

                    if (state.dataSurface->SurfDaylightingShelfInd(SurfNum) > 0) {
                        ShowSevereError(state,
                                        format("{} = {}:  Window {} is referenced by more than one shelf.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(2)));
                        state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                    }

                    if (state.dataSurface->Surface(SurfNum).HasShadeControl) {
                        ShowSevereError(state,
                                        format("{} = {}:  Window {} must not have a shading control.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(2)));
                        state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                    }

                    if (state.dataSurface->Surface(SurfNum).FrameDivider > 0) {
                        ShowSevereError(state,
                                        format("{} = {}:  Window {} must not have a frame/divider.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(2)));
                        state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                    }

                    if (state.dataSurface->Surface(SurfNum).Sides != 4) {
                        ShowSevereError(
                            state, format("{} = {}:  Window {} must have 4 sides.", cCurrentModuleObject, ipsc->cAlphaArgs(1), ipsc->cAlphaArgs(2)));
                        state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                    }
                    if (state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).WindowTypeEQL) {
                        ShowSevereError(state,
                                        format("{} = {}:  Window {} Equivalent Layer Window is not supported.",
                                               cCurrentModuleObject,
                                               ipsc->cAlphaArgs(1),
                                               ipsc->cAlphaArgs(2)));
                        state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                    }

                    state.dataDaylightingDevicesData->Shelf(ShelfNum).Window = SurfNum;
                    state.dataSurface->SurfDaylightingShelfInd(SurfNum) = ShelfNum;
                }

                // Get inside shelf heat transfer surface (optional)
                if (ipsc->cAlphaArgs(3) != "") {
                    SurfNum = Util::FindItemInList(ipsc->cAlphaArgs(3), state.dataSurface->Surface);

                    if (SurfNum == 0) {
                        ShowSevereError(
                            state, format("{} = {}:  Inside shelf {} not found.", cCurrentModuleObject, ipsc->cAlphaArgs(1), ipsc->cAlphaArgs(3)));
                        state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                    } else {
                        // No error if shelf belongs to more than one window, e.g. concave corners

                        if (state.dataSurface->Surface(SurfNum).ExtBoundCond != SurfNum) {
                            ShowSevereError(state,
                                            format("{} = {}:  Inside shelf {} must be its own Outside Boundary Condition Object.",
                                                   cCurrentModuleObject,
                                                   ipsc->cAlphaArgs(1),
                                                   ipsc->cAlphaArgs(3)));
                            state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                        }

                        if (state.dataSurface->Surface(SurfNum).Sides != 4) {
                            ShowSevereError(
                                state,
                                format(
                                    "{} = {}:  Inside shelf {} must have 4 sides.", cCurrentModuleObject, ipsc->cAlphaArgs(1), ipsc->cAlphaArgs(3)));
                            state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                        }

                        state.dataDaylightingDevicesData->Shelf(ShelfNum).InSurf = SurfNum;
                    }
                }

                // Get outside shelf attached shading surface (optional)
                if (ipsc->cAlphaArgs(4) != "") {
                    SurfNum = Util::FindItemInList(ipsc->cAlphaArgs(4), state.dataSurface->Surface);

                    if (SurfNum == 0) {
                        ShowSevereError(
                            state, format("{} = {}:  Outside shelf {} not found.", cCurrentModuleObject, ipsc->cAlphaArgs(1), ipsc->cAlphaArgs(4)));
                        state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                    } else {
                        // No error if shelf belongs to more than one window, e.g. concave corners

                        if (state.dataSurface->Surface(SurfNum).Class != SurfaceClass::Shading) {
                            ShowSevereError(state,
                                            format("{} = {}:  Outside shelf {} is not a Shading:Zone:Detailed object.",
                                                   cCurrentModuleObject,
                                                   ipsc->cAlphaArgs(1),
                                                   ipsc->cAlphaArgs(4)));
                            state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                        }

                        if (state.dataSurface->Surface(SurfNum).shadowSurfSched != nullptr) {
                            ShowSevereError(state,
                                            format("{} = {}:  Outside shelf {} must not have a transmittance schedule.",
                                                   cCurrentModuleObject,
                                                   ipsc->cAlphaArgs(1),
                                                   ipsc->cAlphaArgs(4)));
                            state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                        }

                        if (state.dataSurface->Surface(SurfNum).Sides != 4) {
                            ShowSevereError(
                                state,
                                format(
                                    "{} = {}:  Outside shelf {} must have 4 sides.", cCurrentModuleObject, ipsc->cAlphaArgs(1), ipsc->cAlphaArgs(4)));
                            state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                        }

                        int ConstrNum = 0;
                        // Get outside shelf construction (required if outside shelf is specified)
                        if (ipsc->cAlphaArgs(5) != "") {
                            ConstrNum = Util::FindItemInList(ipsc->cAlphaArgs(5), state.dataConstruction->Construct);

                            if (ConstrNum == 0) {
                                ShowSevereError(state,
                                                format("{} = {}:  Outside shelf construction {} not found.",
                                                       cCurrentModuleObject,
                                                       ipsc->cAlphaArgs(1),
                                                       ipsc->cAlphaArgs(5)));
                                state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                            } else if (state.dataConstruction->Construct(ConstrNum).TypeIsWindow) {
                                ShowSevereError(state,
                                                format("{} = {}:  Outside shelf construction {} must not have WindowMaterial:Glazing.",
                                                       cCurrentModuleObject,
                                                       ipsc->cAlphaArgs(1),
                                                       ipsc->cAlphaArgs(5)));
                                state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                            } else {
                                state.dataDaylightingDevicesData->Shelf(ShelfNum).Construction = ConstrNum;
                                state.dataConstruction->Construct(ConstrNum).IsUsed = true;
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{} = {}:  Outside shelf requires an outside shelf construction to be specified.",
                                                   cCurrentModuleObject,
                                                   ipsc->cAlphaArgs(1)));
                            state.dataDaylightingDevices->GetShelfInputErrorsFound = true;
                        }

                        // Get view factor to outside shelf (optional)
                        if (NumNumbers > 0) {
                            state.dataDaylightingDevicesData->Shelf(ShelfNum).ViewFactor = ipsc->rNumericArgs(1);

                            if (ipsc->rNumericArgs(1) == 0.0) {
                                ShowWarningError(state,
                                                 format("{} = {}:  View factor to outside shelf is zero.  Shelf does not reflect on window.",
                                                        cCurrentModuleObject,
                                                        ipsc->cAlphaArgs(1)));
                            }
                        } else {
                            state.dataDaylightingDevicesData->Shelf(ShelfNum).ViewFactor =
                                -1.0; // Flag to have the view factor calculated during initialization
                        }

                        state.dataDaylightingDevicesData->Shelf(ShelfNum).OutSurf = SurfNum;

                        // Reset some properties of the SURFACE:SHADING:ATTACHED object in order to receive radiation and shading
                        // Normally this would be done during initialization, but that's not early enough for some shading calculations
                        state.dataSurface->Surface(SurfNum).BaseSurf = SurfNum;
                        state.dataSurface->Surface(SurfNum).HeatTransSurf = true;
                        state.dataSurface->AllHTSurfaceList.push_back(SurfNum);
                        // Is this needed? surfZone.ZoneHTNonWindowSurfaceList.push_back(SurfNum);
                        state.dataSurface->Surface(SurfNum).Construction = ConstrNum; // Kludge to allow shading surface to be a heat transfer surface
                        state.dataSurface->SurfActiveConstruction(SurfNum) = ConstrNum;
                        state.dataConstruction->Construct(ConstrNum).IsUsed = true;
                    }
                }

                if (state.dataDaylightingDevicesData->Shelf(ShelfNum).InSurf == 0 && state.dataDaylightingDevicesData->Shelf(ShelfNum).OutSurf == 0) {
                    ShowWarningError(state,
                                     format("{} = {}:  No inside shelf or outside shelf was specified.", cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                }

            } // ShelfNum

            if (state.dataDaylightingDevices->GetShelfInputErrorsFound) {
                ShowFatalError(state, "Errors in DaylightingDevice:Shelf input.");
            }
        }
    }

    Real64 CalcPipeTransBeam(Real64 const R,    // Reflectance of surface, constant (can be made R = f(theta) later)
                             Real64 const A,    // Aspect ratio, L / d
                             Real64 const Theta // Angle of entry in radians
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   May 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the numerical integral for the transmittance of a reflective cylinder with
        // incident collimated beam radiation as described in Swift and Smith.

        // METHODOLOGY EMPLOYED:
        // Since this integral can be slow, a table of values is calculated and stored during
        // initialization of the TDD.  Intermediate values are calculated by interpolation.
        // Transmittance of sky and ground diffuse radiation is done by other functions.

        // REFERENCES:
        // Swift, P. D., and Smith, G. B.  "Cylindrical Mirror Light Pipes",
        //   Solar Energy Materials and Solar Cells 36 (1995), pp. 159-168.

        // OTHER NOTES:
        // The execution time of this function can be reduced by adjusting parameters N and xTol below.
        // However, there is some penalty in accuracy for N < 100,000 and xTol > 150.

        // USE STATEMENTS: na

        // Return value
        Real64 CalcPipeTransBeam;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        Real64 constexpr N(100000.0); // Number of integration points
        Real64 constexpr xTol(150.0); // Tolerance factor to skip iterations where dT is approximately 0
        // Must be >= 1.0, increase this number to decrease the execution time
        Real64 const myLocalTiny(TINY(1.0));

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 i; // Integration interval between points
        Real64 s; // Entry point
        Real64 dT;
        Real64 T; // Beam transmittance for collimated solar real
        Real64 x; // Intermediate variables for speed optimization
        Real64 c1;
        Real64 c2;
        Real64 xLimit; // Limiting x value to prevent floating point underflow

        CalcPipeTransBeam = 0.0;

        T = 0.0;
        i = 1.0 / N;

        xLimit = (std::log(pow_2(N) * myLocalTiny) / std::log(R)) / xTol;

        c1 = A * std::tan(Theta);
        c2 = 4.0 / Constant::Pi;

        s = i;
        while (s < (1.0 - i)) {
            x = c1 / s;

            if (x < xLimit) {
                dT = c2 * std::pow(R, int(x)) * (1.0 - (1.0 - R) * (x - int(x))) * pow_2(s) / std::sqrt(1.0 - pow_2(s));
                T += dT;
            }

            s += i;
        }

        T /= (N - 1.0); // - 1.0, because started on i, not 0

        CalcPipeTransBeam = T;

        return CalcPipeTransBeam;
    }

    Real64 CalcTDDTransSolIso(EnergyPlusData &state, int const PipeNum) // TDD pipe object number
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   July 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the transmittance of sky isotropic radiation for use with the anisotropic sky transmittance.
        // This value is also used for all ground reflected solar radiation (which is isotropic).

        // METHODOLOGY EMPLOYED:
        // The transmittance is calculated and stored once at initialization because the value is a constant.
        // The routine numerically integrates over the entire sky.  All radiation is isotropic, but incident
        // angle varies over the hemisphere.
        // Trans = Flux Transmitted / Flux Incident
        // Not sure if shading and tilt is adequately accounted for by DifShdgRatioIsoSky later on or not...

        // REFERENCES:
        // See AnisoSkyViewFactors in SolarShading.cc.

        // USE STATEMENTS: na

        // Return value
        Real64 CalcTDDTransSolIso;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        int constexpr NPH(1000); // Number of altitude integration points

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 FluxInc = 0.0;   // Incident solar flux
        Real64 FluxTrans = 0.0; // Transmitted solar flux
        Real64 trans;           // Total beam solar transmittance of TDD
        Real64 COSI;            // Cosine of incident angle
        Real64 SINI;            // Sine of incident angle

        Real64 const dPH = 90.0 * Constant::DegToRad / NPH; // Altitude angle of sky element
        Real64 PH = 0.5 * dPH;                              // Altitude angle increment

        // Integrate from 0 to Pi/2 altitude
        for (int N = 1; N <= NPH; ++N) {
            COSI = std::cos(Constant::PiOvr2 - PH);
            SINI = std::sin(Constant::PiOvr2 - PH);

            Real64 P = COSI; // Angular distribution function: P = COS(Incident Angle) for diffuse isotropic

            // Calculate total TDD transmittance for given angle
            trans = TransTDD(state, PipeNum, COSI, RadType::SolarBeam);

            FluxInc += P * SINI * dPH;
            FluxTrans += trans * P * SINI * dPH;

            PH += dPH; // Increment the altitude angle
        } // N

        CalcTDDTransSolIso = FluxTrans / FluxInc;

        return CalcTDDTransSolIso;
    }

    Real64 CalcTDDTransSolHorizon(EnergyPlusData &state, int const PipeNum) // TDD pipe object number
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   July 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the transmittance of sky horizon radiation for use with the anisotropic sky transmittance.

        // METHODOLOGY EMPLOYED:
        // The transmittance is calculated and stored once at initialization because the value is a constant.
        // The routine numerically integrates over the horizon as an infinitesimally narrow strip of the sky.
        // Horizon radiation is isotropic, but incident angle varies over the semicircle.
        // Trans = Flux Transmitted / Flux Incident
        // Not sure if shading is adequately accounted for by DifShdgRatioHoriz later on or not...

        // REFERENCES:
        // See AnisoSkyViewFactors in SolarShading.cc.

        // Using/Aliasing
        using namespace DataSurfaces;

        // Return value
        Real64 CalcTDDTransSolHorizon;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        int constexpr NTH(18); // Number of azimuth integration points

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 FluxInc = 0.0;   // Incident solar flux
        Real64 FluxTrans = 0.0; // Transmitted solar flux
        Real64 CosPhi;          // Cosine of TDD:DOME altitude angle
        Real64 Theta;           // TDD:DOME azimuth angle

        CosPhi = std::cos(Constant::PiOvr2 -
                          state.dataSurface->Surface(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome).Tilt * Constant::DegToRad);
        Theta = state.dataSurface->Surface(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome).Azimuth * Constant::DegToRad;

        if (CosPhi > 0.01) { // Dome has a view of the horizon
            // Integrate over the semicircle
            Real64 const THMIN = Theta - Constant::PiOvr2; // Minimum azimuth integration limit
            // Real64 const THMAX = Theta + PiOvr2; // Maximum azimuth integration limit
            Real64 const dTH = 180.0 * Constant::DegToRad / NTH; // Azimuth angle increment
            Real64 TH = THMIN + 0.5 * dTH;                       // Azimuth angle of sky horizon element

            for (int N = 1; N <= NTH; ++N) {
                // Calculate incident angle between dome outward normal and horizon element
                Real64 COSI = CosPhi * std::cos(TH - Theta); // Cosine of the incident angle

                // Calculate total TDD transmittance for given angle
                Real64 trans = TransTDD(state, PipeNum, COSI, RadType::SolarBeam); // Total beam solar transmittance of TDD

                FluxInc += COSI * dTH;
                FluxTrans += trans * COSI * dTH;

                TH += dTH; // Increment the azimuth angle
            } // N

            CalcTDDTransSolHorizon = FluxTrans / FluxInc;

        } else {                          // Dome is nearly horizontal and has almost no view of the horizon
            CalcTDDTransSolHorizon = 0.0; // = TransTDD(state, PipeNum, ???, SolarBeam) ! Could change to an angle near the horizon
        }

        return CalcTDDTransSolHorizon;
    }

    Real64 CalcTDDTransSolAniso(EnergyPlusData &state,
                                int const PipeNum, // TDD pipe object number
                                Real64 const COSI  // Cosine of the incident angle
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   July 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the transmittance of the anisotropic sky.

        // METHODOLOGY EMPLOYED:
        // Similar to the Trans = FluxTrans/FluxInc integrations above, the anisotropic sky can be decomposed
        // and have a different transmittance applied to each component.
        //   FluxInc = IsoSkyRad + CircumSolarRad + HorizonRad
        //   FluxTrans = T1*IsoSkyRad + T2*CircumSolarRad + T3*HorizonRad
        // It turns out that FluxTrans/FluxInc is equivalent to AnisoSkyTDDMult/SurfAnisoSkyMult.
        // SurfAnisoSkyMult has been conveniently calculated already in AnisoSkyViewFactors in SolarShading.cc.
        // SurfAnisoSkyMult = MultIsoSky*DifShdgRatioIsoSky + MultCircumSolar*SunlitFrac + MultHorizonZenith*DifShdgRatioHoriz
        // In this routine a similar AnisoSkyTDDMult is calculated that applies the appropriate transmittance to each
        // of the components above.  The result is Trans = AnisoSkyTDDMult/SurfAnisoSkyMult.
        // Shading and orientation are already taken care of by DifShdgRatioIsoSky and DifShdgRatioHoriz.

        // REFERENCES:
        // See AnisoSkyViewFactors in SolarShading.cc.

        // Return value
        Real64 CalcTDDTransSolAniso;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int DomeSurf;           // TDD:DOME surface number
        Real64 IsoSkyRad;       // Isotropic sky radiation component
        Real64 CircumSolarRad;  // Circumsolar sky radiation component
        Real64 HorizonRad;      // Horizon sky radiation component
        Real64 AnisoSkyTDDMult; // Anisotropic sky multiplier for TDD

        DomeSurf = state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome;

        if (!state.dataSysVars->DetailedSkyDiffuseAlgorithm || !state.dataSurface->ShadingTransmittanceVaries ||
            state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::Minimal) {
            IsoSkyRad = state.dataSolarShading->SurfMultIsoSky(DomeSurf) * state.dataSolarShading->SurfDifShdgRatioIsoSky(DomeSurf);
            HorizonRad = state.dataSolarShading->SurfMultHorizonZenith(DomeSurf) * state.dataSolarShading->SurfDifShdgRatioHoriz(DomeSurf);
        } else {
            IsoSkyRad = state.dataSolarShading->SurfMultIsoSky(DomeSurf) * state.dataSolarShading->SurfCurDifShdgRatioIsoSky(DomeSurf);
            HorizonRad = state.dataSolarShading->SurfMultHorizonZenith(DomeSurf) *
                         state.dataSolarShading->SurfDifShdgRatioHorizHRTS(state.dataGlobal->TimeStep, state.dataGlobal->HourOfDay, DomeSurf);
        }
        CircumSolarRad = state.dataSolarShading->SurfMultCircumSolar(DomeSurf) *
                         state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, DomeSurf);

        AnisoSkyTDDMult = state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransSolIso * IsoSkyRad +
                          TransTDD(state, PipeNum, COSI, RadType::SolarBeam) * CircumSolarRad +
                          state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransSolHorizon * HorizonRad;

        if (state.dataSolarShading->SurfAnisoSkyMult(DomeSurf) > 0.0) {
            CalcTDDTransSolAniso = AnisoSkyTDDMult / state.dataSolarShading->SurfAnisoSkyMult(DomeSurf);
        } else {
            CalcTDDTransSolAniso = 0.0;
        }

        return CalcTDDTransSolAniso;
    }

    Real64 TransTDD(EnergyPlusData &state,
                    int const PipeNum,          // TDD pipe object number
                    Real64 const COSI,          // Cosine of the incident angle
                    RadType const RadiationType // Radiation type flag
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   May 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the total transmittance of the TDD for specified radiation type.

        // METHODOLOGY EMPLOYED:
        // The transmittances for each component (i.e. TDD:DIFFUSER, TDD:DOME, and pipe) are calculated.
        // All transmittances are multiplied to get the total for the TDD:
        //   TransTDD = transDome * transPipe * transDiff
        // Transmittance of beam radiation is calculated by interpolating the values in a
        // table created during initialization.  The table values are from Swift and Smith's
        // numerical integral for collimated beam radiation.
        // Transmittances of isotropic and anisotropic diffuse radiation are more complicated and call
        // other subroutines in this module.
        // All light reaching the TDD:DIFFUSER is assumed to be diffuse.
        // NOTE: Dome transmittance could be improved by taking into account curvature of the dome.

        // REFERENCES:
        // Swift, P. D., and Smith, G. B.  "Cylindrical Mirror Light Pipes",
        //   Solar Energy Materials and Solar Cells 36 (1995), pp. 159-168.

        // Return value
        Real64 TransTDD;

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int constDome; // Construction object number for TDD:DOME
        int constDiff; // Construction object number for TDD:DIFFUSER
        Real64 transDome;
        Real64 transPipe;
        Real64 transDiff;

        TransTDD = 0.0;

        // Get constructions of each TDD component
        constDome = state.dataSurface->Surface(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome).Construction;
        constDiff = state.dataSurface->Surface(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Diffuser).Construction;

        // Get the transmittance of each component and of total TDD
        switch (RadiationType) {
        case RadType::VisibleBeam: {
            transDome = Window::POLYF(COSI, state.dataConstruction->Construct(constDome).TransVisBeamCoef);
            transPipe = InterpolatePipeTransBeam(state, COSI, state.dataDaylightingDevicesData->TDDPipe(PipeNum).PipeTransVisBeam);
            transDiff = state.dataConstruction->Construct(constDiff).TransDiffVis; // May want to change to POLYF also!

            TransTDD = transDome * transPipe * transDiff;

        } break;
        case RadType::SolarBeam: {
            transDome = Window::POLYF(COSI, state.dataConstruction->Construct(constDome).TransSolBeamCoef);
            transPipe = InterpolatePipeTransBeam(state, COSI, state.dataDaylightingDevicesData->TDDPipe(PipeNum).PipeTransSolBeam);
            transDiff = state.dataConstruction->Construct(constDiff).TransDiff; // May want to change to POLYF also!

            TransTDD = transDome * transPipe * transDiff;

        } break;
        case RadType::SolarAniso: {
            TransTDD = CalcTDDTransSolAniso(state, PipeNum, COSI);
        } break;
        case RadType::SolarIso: {
            TransTDD = state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransSolIso;
        } break;
        default:
            break;
        }

        return TransTDD;
    }

    Real64 InterpolatePipeTransBeam(EnergyPlusData &state,
                                    Real64 const COSI,               // Cosine of the incident angle
                                    const Array1D<Real64> &transBeam // Table of beam transmittance vs. cosine angle
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   July 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Interpolates the beam transmittance vs. cosine angle table.

        // METHODOLOGY EMPLOYED: na
        // REFERENCES: na

        // Using/Aliasing
        using Fluid::FindArrayIndex; // USEd code could be copied here to eliminate dependence on FluidProperties

        // Return value
        Real64 InterpolatePipeTransBeam;

        // Argument array dimensioning
        EP_SIZE_CHECK(transBeam, NumOfAngles);

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int Lo;
        int Hi;
        Real64 m;
        Real64 b;

        InterpolatePipeTransBeam = 0.0;

        // Linearly interpolate transBeam/COSAngle table to get value at current cosine of the angle
        Lo = FindArrayIndex(COSI, state.dataDaylightingDevices->COSAngle);
        Hi = Lo + 1;

        if (Lo > 0 && Hi <= NumOfAngles) {
            m = (transBeam(Hi) - transBeam(Lo)) / (state.dataDaylightingDevices->COSAngle(Hi) - state.dataDaylightingDevices->COSAngle(Lo));
            b = transBeam(Lo) - m * state.dataDaylightingDevices->COSAngle(Lo);

            InterpolatePipeTransBeam = m * COSI + b;
        } else {
            InterpolatePipeTransBeam = 0.0;
        }

        return InterpolatePipeTransBeam;
    }

    int FindTDDPipe(EnergyPlusData &state, int const WinNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   May 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Given the TDD:DOME or TDD:DIFFUSER object number, returns TDD pipe number.

        // Return value
        int FindTDDPipe;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int PipeNum; // TDD pipe object number

        FindTDDPipe = 0;

        if ((int)state.dataDaylightingDevicesData->TDDPipe.size() <= 0) {
            ShowFatalError(state,
                           format("FindTDDPipe: Surface={}, TDD:Dome object does not reference a valid Diffuser object....needs "
                                  "DaylightingDevice:Tubular of same name as Surface.",
                                  state.dataSurface->Surface(WinNum).Name));
        }

        for (PipeNum = 1; PipeNum <= (int)state.dataDaylightingDevicesData->TDDPipe.size(); ++PipeNum) {
            if ((WinNum == state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome) ||
                (WinNum == state.dataDaylightingDevicesData->TDDPipe(PipeNum).Diffuser)) {
                FindTDDPipe = PipeNum;
                break;
            }
        } // PipeNum

        return FindTDDPipe;
    }

    void DistributeTDDAbsorbedSolar(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   July 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Sums the absorbed solar gains from TDD pipes that pass through transition zones.

        // METHODOLOGY EMPLOYED:
        // The total absorbed solar gain is a sum of the following gains:
        //   1. Inward bound solar absorbed by multiple pipe reflections (solar entering pipe - solar exiting pipe)
        //   2. Outward bound solar absorbed by multiple pipe reflections due to:
        //     a. Reflection off of diffuser surface (inside of TDD)
        //     b. Zone diffuse interior shortwave incident on the diffuser from windows, lights, etc.
        //   3. Inward absorbed solar in dome and diffuser glass
        // This subroutine is called by InitIntSolarDistribution in HeatBalanceSurfaceManager.cc.

        for (int PipeNum = 1; PipeNum <= (int)state.dataDaylightingDevicesData->TDDPipe.size(); ++PipeNum) {
            int DiffSurf = state.dataDaylightingDevicesData->TDDPipe(PipeNum).Diffuser;
            Real64 transDiff = state.dataConstruction->Construct(state.dataSurface->Surface(DiffSurf).Construction).TransDiff;

            // Calculate diffuse solar reflected back up the pipe by the inside surface of the TDD:DIFFUSER
            // All solar arriving at the diffuser is assumed to be isotropically diffuse by this point
            Real64 QRefl = (state.dataHeatBal->SurfQRadSWOutIncident(DiffSurf) - state.dataHeatBal->SurfWinQRadSWwinAbsTot(DiffSurf)) *
                               state.dataSurface->Surface(DiffSurf).Area -
                           state.dataSurface->SurfWinTransSolar(DiffSurf);

            // Add diffuse interior shortwave reflected from zone surfaces and from zone sources, lights, etc.
            QRefl += state.dataHeatBal->EnclSolQSWRad(state.dataSurface->Surface(DiffSurf).SolarEnclIndex) *
                     state.dataSurface->Surface(DiffSurf).Area * transDiff;

            Real64 TotTDDPipeGain = state.dataSurface->SurfWinTransSolar(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome) -
                                    state.dataHeatBal->SurfQRadSWOutIncident(DiffSurf) * state.dataSurface->Surface(DiffSurf).Area +
                                    QRefl * (1.0 - state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransSolIso / transDiff) +
                                    state.dataHeatBal->SurfWinQRadSWwinAbs(state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome, 1) *
                                        state.dataSurface->Surface(DiffSurf).Area / 2.0 +
                                    state.dataHeatBal->SurfWinQRadSWwinAbs(DiffSurf, 1) * state.dataSurface->Surface(DiffSurf).Area /
                                        2.0; // Solar entering pipe | Solar exiting pipe | Absorbed due to
                                             // reflections on the way out | Inward absorbed solar from dome
                                             // glass | Inward absorbed solar from diffuser glass
            state.dataDaylightingDevicesData->TDDPipe(PipeNum).PipeAbsorbedSolar = max(0.0, TotTDDPipeGain); // Report variable [W]

            for (int TZoneNum = 1; TZoneNum <= state.dataDaylightingDevicesData->TDDPipe(PipeNum).NumOfTZones; ++TZoneNum) {
                // Distribute absorbed solar gain in proportion to transition zone length
                state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZoneHeatGain(TZoneNum) =
                    TotTDDPipeGain * (state.dataDaylightingDevicesData->TDDPipe(PipeNum).TZoneLength(TZoneNum) /
                                      state.dataDaylightingDevicesData->TDDPipe(PipeNum).TotLength);
            } // TZoneNum
        }
    }

    void CalcViewFactorToShelf(EnergyPlusData &state, int const ShelfNum) // Daylighting shelf object number
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   August 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Attempts to calculate exact analytical view factor from window to outside shelf.

        // METHODOLOGY EMPLOYED:
        // Uses a standard analytical solution.  It is required that window and shelf have the same width, i.e.
        // one edge (or two vertices) shared in common.  An error or warning is issued if not true.
        // A more general routine should be implemented at some point to solve for more complicated geometries.
        // Until then, the user has the option to specify their own solution for the view factor in the input object.

        // REFERENCES:
        // Mills, A. F.  Heat and Mass Transfer, 1995, p. 499.  (Shape factor for adjacent rectangles.)

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 W; // Width, height, and length of window/shelf geometry
        Real64 H;
        Real64 L;
        Real64 M; // Intermediate variables
        Real64 N;
        Real64 E1; // Intermediate equations
        Real64 E2;
        Real64 E3;
        Real64 E4;
        int VWin; // Vertex indices
        int VShelf;
        int NumMatch; // Number of vertices matched

        W = state.dataSurface->Surface(state.dataDaylightingDevicesData->Shelf(ShelfNum).Window).Width;
        H = state.dataSurface->Surface(state.dataDaylightingDevicesData->Shelf(ShelfNum).Window).Height;

        // Find length, i.e. projection, of outside shelf
        if (state.dataSurface->Surface(state.dataDaylightingDevicesData->Shelf(ShelfNum).OutSurf).Width == W) {
            L = state.dataSurface->Surface(state.dataDaylightingDevicesData->Shelf(ShelfNum).OutSurf).Height;
        } else if (state.dataSurface->Surface(state.dataDaylightingDevicesData->Shelf(ShelfNum).OutSurf).Height == W) {
            L = state.dataSurface->Surface(state.dataDaylightingDevicesData->Shelf(ShelfNum).OutSurf).Width;
        } else {
            ShowFatalError(state,
                           format("DaylightingDevice:Shelf = {}:  Width of window and outside shelf do not match.",
                                  state.dataDaylightingDevicesData->Shelf(ShelfNum).Name));
        }

        // Error if more or less than two vertices match
        NumMatch = 0;
        for (VWin = 1; VWin <= 4; ++VWin) {
            for (VShelf = 1; VShelf <= 4; ++VShelf) {
                if (distance(state.dataSurface->Surface(state.dataDaylightingDevicesData->Shelf(ShelfNum).Window).Vertex(VWin),
                             state.dataSurface->Surface(state.dataDaylightingDevicesData->Shelf(ShelfNum).OutSurf).Vertex(VShelf)) == 0.0) {
                    ++NumMatch;
                }
            }
        }

        if (NumMatch < 2) {
            ShowWarningError(
                state,
                format("DaylightingDevice:Shelf = {}:  Window and outside shelf must share two vertices.  View factor calculation may be inaccurate.",
                       state.dataDaylightingDevicesData->Shelf(ShelfNum).Name));
        } else if (NumMatch > 2) {
            ShowFatalError(state,
                           format("DaylightingDevice:Shelf = {}:  Window and outside shelf share too many vertices.",
                                  state.dataDaylightingDevicesData->Shelf(ShelfNum).Name));
        }

        // Calculate exact analytical view factor from window to outside shelf
        M = H / W;
        N = L / W;

        E1 = M * std::atan(1.0 / M) + N * std::atan(1.0 / N) - std::sqrt(pow_2(N) + pow_2(M)) * std::atan(std::pow(pow_2(N) + pow_2(M), -0.5));
        E2 = ((1.0 + pow_2(M)) * (1.0 + pow_2(N))) / (1.0 + pow_2(M) + pow_2(N));
        E3 = std::pow(pow_2(M) * (1.0 + pow_2(M) + pow_2(N)) / ((1.0 + pow_2(M)) * (pow_2(M) + pow_2(N))), pow_2(M));
        E4 = std::pow(pow_2(N) * (1.0 + pow_2(M) + pow_2(N)) / ((1.0 + pow_2(N)) * (pow_2(M) + pow_2(N))), pow_2(N));

        state.dataDaylightingDevicesData->Shelf(ShelfNum).ViewFactor = (1.0 / (Constant::Pi * M)) * (E1 + 0.25 * std::log(E2 * E3 * E4));
    }

    void adjustViewFactorsWithShelf(
        EnergyPlusData &state, Real64 &viewFactorToShelf, Real64 &viewFactorToSky, Real64 &viewFactorToGround, int WinSurf, int ShelfNum)
    {
        // First, make sure none of the view factors are less than zero and return if there isn't a problem or if
        // view factor to shelf greater than one.  Both cases together would also eliminate if other views are zero
        // which means nothing would need to be done.
        if (viewFactorToSky <= 0.0) {
            viewFactorToSky = 0.0;
        }
        if (viewFactorToGround <= 0.0) {
            viewFactorToGround = 0.0;
        }
        if (viewFactorToShelf <= 0.0) { // No shelf impact for which to account
            ShowWarningError(state,
                             format("DaylightingDevice:Shelf = {}:  Window view factor to shelf was less than 0.  This should not happen.",
                                    state.dataDaylightingDevicesData->Shelf(ShelfNum).Name));
            ShowContinueError(state, "The view factor has been reset to zero.");
            viewFactorToShelf = 0.0;
            if ((viewFactorToGround + viewFactorToSky) > 1.0) { // This data came in incorrect, fix by proportional reduction
                viewFactorToGround = viewFactorToGround / (viewFactorToGround + viewFactorToSky);
                viewFactorToSky = 1.0 - viewFactorToGround;
                ShowWarningError(state,
                                 format("DaylightingDevice:Shelf = {}:  The sum of the window view factors to ground and sky were greater than 1.  "
                                        "This should not happen.",
                                        state.dataDaylightingDevicesData->Shelf(ShelfNum).Name));
                ShowContinueError(
                    state, "The view factors have been reset to so that they do not exceed 1.  Check/fix your input file data to avoid this issue.");
            }
            return;
        }
        if (viewFactorToShelf + viewFactorToSky + viewFactorToGround <= 1.0) {
            return; // nothing wrong here
        }
        if (viewFactorToShelf >= 1.0) { // Don't allow shelf view of greater than 1 (zero out other views)
            ShowWarningError(state,
                             format("DaylightingDevice:Shelf = {}:  Window view factor to shelf was greater than 1.  This should not happen.",
                                    state.dataDaylightingDevicesData->Shelf(ShelfNum).Name));
            ShowContinueError(state, "The view factor has been reset to 1 and the other view factors to sky and ground have been set to 0.");
            viewFactorToShelf = 1.0;
            viewFactorToGround = 0.0;
            viewFactorToSky = 0.0;
            return;
        }

        // If the flow is still here, there is something that needs to be adjusted so set the maximum shelf height and the minimum window height
        int ShelfSurf = state.dataDaylightingDevicesData->Shelf(ShelfNum).OutSurf;
        Real64 zShelfMax = state.dataSurface->Surface(ShelfSurf).Vertex(1).z;
        Real64 zShelfMin = zShelfMax;
        for (int vertex = 2; vertex <= state.dataSurface->Surface(ShelfSurf).Sides; ++vertex) {
            if (state.dataSurface->Surface(ShelfSurf).Vertex(vertex).z > zShelfMax) {
                zShelfMax = state.dataSurface->Surface(ShelfSurf).Vertex(vertex).z;
            }
            if (state.dataSurface->Surface(ShelfSurf).Vertex(vertex).z < zShelfMin) {
                zShelfMin = state.dataSurface->Surface(ShelfSurf).Vertex(vertex).z;
            }
        }
        Real64 zWinMax = state.dataSurface->Surface(WinSurf).Vertex(1).z;
        Real64 zWinMin = zWinMax;
        for (int vertex = 2; vertex <= state.dataSurface->Surface(WinSurf).Sides; ++vertex) {
            if (state.dataSurface->Surface(WinSurf).Vertex(vertex).z > zWinMax) {
                zWinMax = state.dataSurface->Surface(WinSurf).Vertex(vertex).z;
            }
            if (state.dataSurface->Surface(WinSurf).Vertex(vertex).z < zWinMin) {
                zWinMin = state.dataSurface->Surface(WinSurf).Vertex(vertex).z;
            }
        }

        Real64 leftoverViewFactor;
        // Now correct the view factors based on the location of the shelf with respect to the window
        ShowWarningError(
            state,
            format("DaylightingDevice:Shelf = {}:  Window view factor to shelf [{:.2R}] results in a sum of view factors greater than 1.",
                   state.dataDaylightingDevicesData->Shelf(ShelfNum).Name,
                   state.dataDaylightingDevicesData->Shelf(ShelfNum).ViewFactor));
        if (zWinMin >= zShelfMax) { // Shelf is fully below window, reduce view to ground first based on view to shelf
            ShowContinueError(
                state,
                "Since the light shelf is below the window to which it is associated, the view factor of the window to the ground was reduced");
            ShowContinueError(
                state, "and possibly also the view factor to the sky. Check you input and/or consider turning off autosizing of the view factors.");
            leftoverViewFactor = 1.0 - viewFactorToShelf - viewFactorToSky;
            if (leftoverViewFactor >= 0.0) {
                viewFactorToGround = leftoverViewFactor; // Other view factors okay
            } else {
                viewFactorToGround = 0.0;
                viewFactorToSky = 1.0 - viewFactorToShelf;
                if (viewFactorToSky < 0.0) {
                    viewFactorToSky = 0.0;
                    viewFactorToShelf = 1.0;
                }
            }

        } else if (zShelfMin >= zWinMax) { // Shelf is fully above window, reduce view to sky first based on view to shelf
            ShowContinueError(
                state, "Since the light shelf is above the window to which it is associated, the view factor of the window to the sky was reduced");
            ShowContinueError(
                state,
                "and possibly also the view factor to the ground. Check you input and/or consider turning off autosizing of the view factors.");
            leftoverViewFactor = 1.0 - viewFactorToShelf - viewFactorToGround;
            if (leftoverViewFactor >= 0.0) {
                viewFactorToSky = leftoverViewFactor;
            } else {
                viewFactorToSky = 0.0;
                viewFactorToGround = 1.0 - viewFactorToShelf;
                if (viewFactorToGround < 0.0) {
                    viewFactorToGround = 0.0;
                    viewFactorToShelf = 1.0;
                }
            }
        } else { // At least part of the shelf is somewhere in the middle of the window so we need to split out the view factors
            ShowContinueError(
                state,
                "Since the light shelf is neither fully above or fully below the window to which it is associated, the view factor of the window");
            ShowContinueError(
                state,
                "to the ground and sky were both potentially reduced. Check you input and/or consider turning off autosizing of the view factors.");
            Real64 zShelfAvg;
            if (((zShelfMin >= zWinMin) && (zShelfMax <= zWinMax)) || // Shelf does not go above or below the window
                ((zShelfMin < zWinMin) && (zShelfMax > zWinMax))) {   // Shelf goes both above AND below the window
                zShelfAvg = 0.5 * (zShelfMin + zShelfMax);
            } else if (zShelfMin < zWinMin) { // Shelf goes partially below the window only
                Real64 fracAbove = 0.0;
                if (zShelfMax > zShelfMin) {
                    fracAbove = (zShelfMax - zWinMin) / (zShelfMax - zShelfMin);
                    if (fracAbove > 1.0) {
                        fracAbove = 1.0;
                    }
                }
                zShelfAvg = zWinMin + fracAbove * (zShelfMax - zWinMin);
            } else { // (zShelfMax > zWinMax): Shelf goes partially above window
                Real64 fracBelow = 0.0;
                if (zShelfMax > zShelfMin) {
                    fracBelow = (zWinMax - zShelfMin) / (zShelfMax - zShelfMin);
                }
                zShelfAvg = zWinMax - fracBelow * (zWinMax - zShelfMin);
            }

            // Find height ratio based on shelf average height
            Real64 heightRatio;
            if (zWinMax > zWinMin) { // Window has a positive height
                heightRatio = (zShelfAvg - zWinMin) / (zWinMax - zWinMin);
                heightRatio = min(heightRatio, 1.0);
                heightRatio = max(heightRatio, 0.0);
            } else { // Window does not have a positive height (not realistic) so set height ratio based on shelf location
                if (zShelfAvg > zWinMax) {
                    heightRatio = 1.0;
                } else {
                    heightRatio = 0.0;
                }
            }

            // Take what is left over after the view to shelf is subtracted and then distribute/adjust that proportionally
            // for the views to ground and sky based on their original weights.  Finally, account for the location of the shelf
            // with respect to the shelf and reset the values of the actual variables used in the rest of the simulation.
            leftoverViewFactor = 1.0 - viewFactorToShelf; // By previous logic above, leftover is greater than zero and less than one
            Real64 vfGroundAdjustMax;
            Real64 vfGroundAdjustMin;
            if (viewFactorToGround > viewFactorToShelf) { // How much view to ground could be reduced potentially if shelf at bottom
                vfGroundAdjustMin = viewFactorToGround - viewFactorToShelf;
            } else {
                vfGroundAdjustMin = 0.0;
            }
            if (viewFactorToGround > leftoverViewFactor) { // How much view to ground could be reduced potentially if shelf at top
                vfGroundAdjustMax = leftoverViewFactor;
            } else {
                vfGroundAdjustMax = viewFactorToGround;
            }
            viewFactorToGround = vfGroundAdjustMin + heightRatio * (vfGroundAdjustMax - vfGroundAdjustMin);
            viewFactorToSky = leftoverViewFactor - viewFactorToGround;
        }
        ShowWarningError(state,
                         format("DaylightingDevice:Shelf = {}:  As a result of user input (see previous messages), at least one view factor but "
                                "possibly more than one was reduced.",
                                state.dataDaylightingDevicesData->Shelf(ShelfNum).Name));
        ShowContinueError(state,
                          "These include the view factors to the ground, the sky, and the exterior light shelf.  Note that views to other exterior "
                          "surfaces could further complicated this.");
        ShowContinueError(state, "Please consider manually calculating or adjusting view factors to avoid this problem.");
    }

    void FigureTDDZoneGains(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec 2011

        // PURPOSE OF THIS SUBROUTINE:
        // initialize zone gains at begin new environment

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        if ((int)state.dataDaylightingDevicesData->TDDPipe.size() == 0) {
            return;
        }

        if (state.dataGlobal->BeginEnvrnFlag && state.dataDaylightingDevices->MyEnvrnFlag) {
            for (int Loop = 1; Loop <= (int)state.dataDaylightingDevicesData->TDDPipe.size(); ++Loop) {
                state.dataDaylightingDevicesData->TDDPipe(Loop).TZoneHeatGain = 0.0;
            }
            state.dataDaylightingDevices->MyEnvrnFlag = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataDaylightingDevices->MyEnvrnFlag = true;
        }
    }

} // namespace Dayltg

} // namespace EnergyPlus
