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

// EnergyPlus headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WindowManager.hh>

// Windows library headers
#include <WCEMultiLayerOptics.hpp>
#include <WCETarcog.hpp>

// EnergyPlus headers
#include <EnergyPlus/WindowManagerExteriorThermal.hh>

namespace EnergyPlus {

using namespace DataEnvironment;
using namespace DataSurfaces;
using namespace DataHeatBalance;
using namespace General;

namespace Window {

    /////////////////////////////////////////////////////////////////////////////////////////
    void CalcWindowHeatBalanceExternalRoutines(EnergyPlusData &state,
                                               int const SurfNum,          // Surface number
                                               Real64 const HextConvCoeff, // Outside air film conductance coefficient
                                               Real64 &SurfInsideTemp,     // Inside window surface temperature
                                               Real64 &SurfOutsideTemp     // Outside surface temperature (C)
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   July 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Main wrapper routine to pick-up data from EnergyPlus and then call Windows-CalcEngine routines
        // to obtain results

        auto &surf = state.dataSurface->Surface(SurfNum);
        auto &surfWin = state.dataSurface->SurfaceWindow(SurfNum);
        int ConstrNum = surf.Construction;
        auto &construction = state.dataConstruction->Construct(ConstrNum);

        constexpr Real64 solutionTolerance = 0.02;

        // Tarcog thermal system for solving heat transfer through the window
        int activeConstrNum = CWCEHeatTransferFactory::getActiveConstructionNumber(state, surf, SurfNum);
        auto aFactory = CWCEHeatTransferFactory(state, surf, SurfNum, activeConstrNum); // (AUTO_OK)
        auto aSystem = aFactory.getTarcogSystem(state, HextConvCoeff);                  // (AUTO_OK_SPTR)
        aSystem->setTolerance(solutionTolerance);

        // get previous timestep temperatures solution for faster iterations
        std::vector<Real64> Guess;
        int totSolidLayers = construction.TotSolidLayers;

        // Interior and exterior shading layers have gas between them and IGU but that gas
        // was not part of construction so it needs to be increased by one
        if (ANY_SHADE_SCREEN(state.dataSurface->SurfWinShadingFlag(SurfNum)) || ANY_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum))) {
            ++totSolidLayers;
        }

        for (int k = 1; k <= 2 * totSolidLayers; ++k) {
            Guess.push_back(state.dataSurface->SurfaceWindow(SurfNum).thetaFace[k]);
        }

        try {
            aSystem->setInitialGuess(Guess);
            aSystem->solve();
        } catch (const std::exception &ex) {
            ShowSevereError(state, "Error in Windows Calculation Engine Exterior Module.");
            ShowContinueError(state, ex.what());
        }

        auto aLayers = aSystem->getSolidLayers(); // (AUTO_OK_OBJ)
        int i = 1;
        for (const auto &aLayer : aLayers) { // (AUTO_OK_SPTR)
            Real64 aTemp = 0;
            for (auto aSide : FenestrationCommon::EnumSide()) { // (AUTO_OK) I don't understand what this construct is
                aTemp = aLayer->getTemperature(aSide);
                state.dataWindowManager->thetas[i - 1] = aTemp;
                if (i == 1) {
                    SurfOutsideTemp = aTemp - Constant::Kelvin;
                }
                ++i;
            }
            SurfInsideTemp = aTemp - Constant::Kelvin;
            if (ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum))) {
                auto &surfShade = state.dataSurface->surfShades(SurfNum);
                Real64 EffShBlEmiss = surfShade.effShadeEmi;
                Real64 EffGlEmiss = surfShade.effGlassEmi;
                if (surfShade.blind.movableSlats) {
                    surfShade.effShadeEmi = Interp(construction.effShadeBlindEmi[surfShade.blind.slatAngIdxLo],
                                                   construction.effShadeBlindEmi[surfShade.blind.slatAngIdxHi],
                                                   surfShade.blind.slatAngInterpFac);
                    surfShade.effGlassEmi = Interp(construction.effGlassEmi[surfShade.blind.slatAngIdxLo],
                                                   construction.effGlassEmi[surfShade.blind.slatAngIdxHi],
                                                   surfShade.blind.slatAngInterpFac);
                }
                state.dataSurface->SurfWinEffInsSurfTemp(SurfNum) =
                    (EffShBlEmiss * SurfInsideTemp + EffGlEmiss * (state.dataWindowManager->thetas[2 * totSolidLayers - 3] - Constant::Kelvin)) /
                    (EffShBlEmiss + EffGlEmiss);
            }
        }

        state.dataHeatBalSurf->SurfHConvInt(SurfNum) = aSystem->getHc(Tarcog::ISO15099::Environment::Indoor);
        if (ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum)) || aFactory.isInteriorShade()) {
            auto &surfShade = state.dataSurface->surfShades(SurfNum);
            // It is not clear why EnergyPlus keeps this interior calculations separately for interior shade. This does create different
            // solution from heat transfer from tarcog itself. Need to confirm with LBNL team about this approach. Note that heat flow
            // through shade (consider case when openings are zero) is different from heat flow obtained by these equations. Will keep
            // these calculations just to confirm that current exterior engine is giving close results to what is in here. (Simon)
            int totLayers = aLayers.size();
            state.dataWindowManager->nglface = 2 * totLayers - 2;
            state.dataWindowManager->nglfacep = state.dataWindowManager->nglface + 2;
            auto aShadeLayer = aLayers[totLayers - 1]; // (AUTO_OK_SPTR)
            auto aGlassLayer = aLayers[totLayers - 2]; // (AUTO_OK_SPTR)
            Real64 ShadeArea = state.dataSurface->Surface(SurfNum).Area + state.dataSurface->SurfWinDividerArea(SurfNum);
            auto frontSurface = aShadeLayer->getSurface(FenestrationCommon::Side::Front); // (AUTO_OK_SPTR)
            auto backSurface = aShadeLayer->getSurface(FenestrationCommon::Side::Back);   // (AUTO_OK_SPTR)
            Real64 EpsShIR1 = frontSurface->getEmissivity();
            Real64 EpsShIR2 = backSurface->getEmissivity();
            Real64 TauShIR = frontSurface->getTransmittance();
            Real64 RhoShIR1 = max(0.0, 1.0 - TauShIR - EpsShIR1);
            Real64 RhoShIR2 = max(0.0, 1.0 - TauShIR - EpsShIR2);
            Real64 glassEmiss = aGlassLayer->getSurface(FenestrationCommon::Side::Back)->getEmissivity();
            Real64 RhoGlIR2 = 1.0 - glassEmiss;
            Real64 ShGlReflFacIR = 1.0 - RhoGlIR2 * RhoShIR1;
            Real64 rmir = state.dataSurface->SurfWinIRfromParentZone(SurfNum) + state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(SurfNum);
            Real64 NetIRHeatGainShade =
                ShadeArea * EpsShIR2 *
                    (Constant::StefanBoltzmann * pow(state.dataWindowManager->thetas[state.dataWindowManager->nglfacep - 1], 4) - rmir) +
                EpsShIR1 * (Constant::StefanBoltzmann * pow(state.dataWindowManager->thetas[state.dataWindowManager->nglfacep - 2], 4) - rmir) *
                    RhoGlIR2 * TauShIR / ShGlReflFacIR;
            Real64 NetIRHeatGainGlass =
                ShadeArea * (glassEmiss * TauShIR / ShGlReflFacIR) *
                (Constant::StefanBoltzmann * pow(state.dataWindowManager->thetas[state.dataWindowManager->nglface - 1], 4) - rmir);
            Real64 tind = surf.getInsideAirTemperature(state, SurfNum) + Constant::Kelvin;
            Real64 ConvHeatGainFrZoneSideOfShade = ShadeArea * state.dataHeatBalSurf->SurfHConvInt(SurfNum) *
                                                   (state.dataWindowManager->thetas[state.dataWindowManager->nglfacep - 1] - tind);
            state.dataSurface->SurfWinHeatGain(SurfNum) =
                state.dataSurface->SurfWinTransSolar(SurfNum) + ConvHeatGainFrZoneSideOfShade + NetIRHeatGainGlass + NetIRHeatGainShade;

            state.dataSurface->SurfWinGainIRGlazToZoneRep(SurfNum) = NetIRHeatGainGlass;

            // Effective shade and glass emissivities that are used later for energy calculations.
            // This needs to be checked as well. (Simon)
            surfShade.effShadeEmi = EpsShIR1 * (1.0 + RhoGlIR2 * TauShIR / (1.0 - RhoGlIR2 * RhoShIR2));
            surfShade.effGlassEmi = glassEmiss * TauShIR / (1.0 - RhoGlIR2 * RhoShIR2);

            Real64 glassTemperature = aGlassLayer->getSurface(FenestrationCommon::Side::Back)->getTemperature();
            state.dataSurface->SurfWinEffInsSurfTemp(SurfNum) =
                (surfShade.effShadeEmi * SurfInsideTemp + surfShade.effGlassEmi * (glassTemperature - Constant::Kelvin)) /
                (surfShade.effShadeEmi + surfShade.effGlassEmi);

        } else {
            // Another adoptation to old source that looks suspicious. Check if heat flow through
            // window is actually matching these values. (Simon)

            //
            auto &surfShade = state.dataSurface->surfShades(SurfNum);
            int totLayers = aLayers.size();
            auto aGlassLayer = aLayers[totLayers - 1];                                  // (AUTO_OK_SPTR)
            auto backSurface = aGlassLayer->getSurface(FenestrationCommon::Side::Back); // (AUTO_OK_SPTR)

            Real64 h_cin = aSystem->getHc(Tarcog::ISO15099::Environment::Indoor);
            Real64 ConvHeatGainFrZoneSideOfGlass =
                surf.Area * h_cin * (backSurface->getTemperature() - aSystem->getAirTemperature(Tarcog::ISO15099::Environment::Indoor));

            Real64 rmir = state.dataSurface->SurfWinIRfromParentZone(SurfNum) + state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(SurfNum);
            Real64 NetIRHeatGainGlass =
                surf.Area * backSurface->getEmissivity() * (Constant::StefanBoltzmann * pow(backSurface->getTemperature(), 4) - rmir);

            state.dataSurface->SurfWinEffInsSurfTemp(SurfNum) =
                aLayers[totLayers - 1]->getTemperature(FenestrationCommon::Side::Back) - Constant::Kelvin;
            surfShade.effGlassEmi = aLayers[totLayers - 1]->getSurface(FenestrationCommon::Side::Back)->getEmissivity();

            state.dataSurface->SurfWinHeatGain(SurfNum) =
                state.dataSurface->SurfWinTransSolar(SurfNum) + ConvHeatGainFrZoneSideOfGlass + NetIRHeatGainGlass;
            state.dataSurface->SurfWinGainConvGlazToZoneRep(SurfNum) = ConvHeatGainFrZoneSideOfGlass;
            state.dataSurface->SurfWinGainIRGlazToZoneRep(SurfNum) = NetIRHeatGainGlass;
        }

        state.dataSurface->SurfWinLossSWZoneToOutWinRep(SurfNum) =
            state.dataHeatBal->EnclSolQSWRad(state.dataSurface->Surface(SurfNum).SolarEnclIndex) * surf.Area * (1 - construction.ReflectSolDiffBack) +
            state.dataHeatBalSurf->SurfWinInitialBeamSolInTrans(SurfNum);
        state.dataSurface->SurfWinHeatGain(SurfNum) -=
            (state.dataSurface->SurfWinLossSWZoneToOutWinRep(SurfNum) + state.dataHeatBalSurf->SurfWinInitialDifSolInTrans(SurfNum) * surf.Area);

        for (int k = 1; k <= surf.getTotLayers(state); ++k) {
            surfWin.thetaFace[2 * k - 1] = state.dataWindowManager->thetas[2 * k - 2];
            surfWin.thetaFace[2 * k] = state.dataWindowManager->thetas[2 * k - 1];

            // temperatures for reporting
            state.dataHeatBal->SurfWinFenLaySurfTempFront(SurfNum, k) = state.dataWindowManager->thetas[2 * k - 2] - Constant::Kelvin;
            state.dataHeatBal->SurfWinFenLaySurfTempBack(SurfNum, k) = state.dataWindowManager->thetas[2 * k - 1] - Constant::Kelvin;
        }
    }

    Real64
    GetIGUUValueForNFRCReport(EnergyPlusData &state, const int surfNum, const int constrNum, const Real64 windowWidth, const Real64 windowHeight)
    {
        Real64 tilt = 90.0;

        auto &surface = state.dataSurface->Surface(surfNum);
        auto aFactory = CWCEHeatTransferFactory(state, surface, surfNum, constrNum); // (AUTO_OK)

        const auto winterGlassUnit = aFactory.getTarcogSystemForReporting(state, false, windowWidth, windowHeight, tilt); // (AUTO_OK_SPTR)

        return winterGlassUnit->getUValue();
    }

    Real64 GetSHGCValueForNFRCReporting(EnergyPlusData &state, int surfNum, int constrNum, Real64 windowWidth, Real64 windowHeight)
    {
        Real64 tilt = 90.0;

        auto &surface = state.dataSurface->Surface(surfNum);
        auto aFactory = CWCEHeatTransferFactory(state, surface, surfNum, constrNum); // (AUTO_OK)

        const auto summerGlassUnit = aFactory.getTarcogSystemForReporting(state, true, windowWidth, windowHeight, tilt); // (AUTO_OK_SPTR)
        return summerGlassUnit->getSHGC(state.dataConstruction->Construct(surface.Construction).SolTransNorm);
    }

    void GetWindowAssemblyNfrcForReport(EnergyPlusData &state,
                                        int const surfNum,
                                        int constrNum,
                                        Real64 windowWidth,
                                        Real64 windowHeight,
                                        EnergyPlus::DataSurfaces::NfrcVisionType vision,
                                        Real64 &uvalue,
                                        Real64 &shgc,
                                        Real64 &vt)
    {
        auto &surface = state.dataSurface->Surface(surfNum);
        auto &frameDivider = state.dataSurface->FrameDivider(surface.FrameDivider);

        auto aFactory = CWCEHeatTransferFactory(state, surface, surfNum, constrNum); // (AUTO_OK)

        for (bool isSummer : {false, true}) {
            constexpr Real64 framehExtConvCoeff = 30.0;
            constexpr Real64 framehIntConvCoeff = 8.0;
            constexpr Real64 tilt = 90.0;

            auto insulGlassUnit = aFactory.getTarcogSystemForReporting(state, isSummer, windowWidth, windowHeight, tilt); // (AUTO_OK_SPTR)

            const double centerOfGlassUvalue = insulGlassUnit->getUValue();

            auto winterGlassUnit = aFactory.getTarcogSystemForReporting(state, false, windowWidth, windowHeight, tilt); // (AUTO_OK_SPTR)

            const double frameUvalue = aFactory.overallUfactorFromFilmsAndCond(frameDivider.FrameConductance, framehIntConvCoeff, framehExtConvCoeff);
            const double frameEdgeUValue = winterGlassUnit->getUValue() * frameDivider.FrEdgeToCenterGlCondRatio; // not sure about this
            const double frameProjectedDimension = frameDivider.FrameWidth;
            const double frameWettedLength = frameProjectedDimension + frameDivider.FrameProjectionIn;
            const double frameAbsorptance = frameDivider.FrameSolAbsorp;

            Tarcog::ISO15099::FrameData frameData{frameUvalue, frameEdgeUValue, frameProjectedDimension, frameWettedLength, frameAbsorptance};

            const double dividerUvalue =
                aFactory.overallUfactorFromFilmsAndCond(frameDivider.DividerConductance, framehIntConvCoeff, framehExtConvCoeff);
            const double dividerEdgeUValue = centerOfGlassUvalue * frameDivider.DivEdgeToCenterGlCondRatio; // not sure about this
            const double dividerProjectedDimension = frameDivider.DividerWidth;
            const double dividerWettedLength = dividerProjectedDimension + frameDivider.DividerProjectionIn;
            const double dividerAbsorptance = frameDivider.DividerSolAbsorp;
            const int numHorizDividers = frameDivider.HorDividers;
            const int numVertDividers = frameDivider.VertDividers;

            Tarcog::ISO15099::FrameData dividerData{
                dividerUvalue, dividerEdgeUValue, dividerProjectedDimension, dividerWettedLength, dividerAbsorptance};

            const Real64 tVis = state.dataConstruction->Construct(constrNum).VisTransNorm;
            const Real64 tSol = state.dataConstruction->Construct(constrNum).SolTransNorm;

            if (vision == DataSurfaces::NfrcVisionType::Single) {
                Tarcog::ISO15099::WindowSingleVision window(windowWidth, windowHeight, tVis, tSol, insulGlassUnit);
                window.setFrameTop(frameData);
                window.setFrameBottom(frameData);
                window.setFrameLeft(frameData);
                window.setFrameRight(frameData);
                window.setDividers(dividerData, numHorizDividers, numVertDividers);

                if (isSummer) {
                    vt = window.vt();
                    shgc = window.shgc();
                } else {
                    uvalue = window.uValue();
                }
            } else if (vision == EnergyPlus::DataSurfaces::NfrcVisionType::DualHorizontal) {
                Tarcog::ISO15099::DualVisionHorizontal window(windowWidth, windowHeight, tVis, tSol, insulGlassUnit, tVis, tSol, insulGlassUnit);
                window.setFrameLeft(frameData);
                window.setFrameRight(frameData);
                window.setFrameBottomLeft(frameData);
                window.setFrameBottomRight(frameData);
                window.setFrameTopLeft(frameData);
                window.setFrameTopRight(frameData);
                window.setFrameMeetingRail(frameData);
                window.setDividers(dividerData, numHorizDividers, numVertDividers);

                if (isSummer) {
                    vt = window.vt();
                    shgc = window.shgc();
                } else {
                    uvalue = window.uValue();
                }
            } else if (vision == EnergyPlus::DataSurfaces::NfrcVisionType::DualVertical) {
                Tarcog::ISO15099::DualVisionVertical window(windowWidth, windowHeight, tVis, tSol, insulGlassUnit, tVis, tSol, insulGlassUnit);
                window.setFrameTop(frameData);
                window.setFrameBottom(frameData);
                window.setFrameTopLeft(frameData);
                window.setFrameTopRight(frameData);
                window.setFrameBottomLeft(frameData);
                window.setFrameBottomRight(frameData);
                window.setFrameMeetingRail(frameData);
                window.setDividers(dividerData, numHorizDividers, numVertDividers);

                if (isSummer) {
                    vt = window.vt();
                    shgc = window.shgc();
                } else {
                    uvalue = window.uValue();
                }
            } else {
                Tarcog::ISO15099::WindowSingleVision window(windowWidth, windowHeight, tVis, tSol, insulGlassUnit);
                window.setFrameTop(frameData);
                window.setFrameBottom(frameData);
                window.setFrameLeft(frameData);
                window.setFrameRight(frameData);
                window.setDividers(dividerData, numHorizDividers, numVertDividers);

                if (isSummer) {
                    vt = window.vt();
                    shgc = window.shgc();
                } else {
                    uvalue = window.uValue();
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    //  CWCEHeatTransferFactory
    /////////////////////////////////////////////////////////////////////////////////////////

    CWCEHeatTransferFactory::CWCEHeatTransferFactory(EnergyPlusData &state, SurfaceData const &surface, int const t_SurfNum, int const t_ConstrNum)
        : m_Surface(surface), m_Window(state.dataSurface->SurfaceWindow(t_SurfNum)), m_ShadePosition(ShadePosition::NoShade), m_SurfNum(t_SurfNum),
          m_SolidLayerIndex(0), m_ConstructionNumber(t_ConstrNum), m_TotLay(getNumOfLayers(state)), m_InteriorBSDFShade(false), m_ExteriorShade(false)
    {
        if (!state.dataConstruction->Construct(m_ConstructionNumber).WindowTypeBSDF &&
            state.dataSurface->SurfWinShadingFlag.size() >= static_cast<size_t>(m_SurfNum)) {
            if (ANY_SHADE_SCREEN(state.dataSurface->SurfWinShadingFlag(m_SurfNum)) || ANY_BLIND(state.dataSurface->SurfWinShadingFlag(m_SurfNum))) {
                m_ConstructionNumber = state.dataSurface->SurfWinActiveShadedConstruction(m_SurfNum);
                m_TotLay = getNumOfLayers(state);
            }
        }
        const WinShadingType ShadeFlag = getShadeType(state, m_ConstructionNumber);

        if (ANY_INTERIOR_SHADE_BLIND(ShadeFlag)) {
            m_ShadePosition = ShadePosition::Interior;
        }

        else if (ANY_EXTERIOR_SHADE_BLIND_SCREEN(ShadeFlag)) {
            m_ShadePosition = ShadePosition::Exterior;
        }

        else if (ANY_BETWEENGLASS_SHADE_BLIND(ShadeFlag)) {
            m_ShadePosition = ShadePosition::Between;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<Tarcog::ISO15099::CSingleSystem> CWCEHeatTransferFactory::getTarcogSystem(EnergyPlusData &state, Real64 const t_HextConvCoeff)
    {
        auto Indoor = getIndoor(state);                    // (AUTO_OK_SPTR)
        auto Outdoor = getOutdoor(state, t_HextConvCoeff); // (AUTO_OK_SPTR)
        auto aIGU = getIGU();                              // (AUTO_OK_OBJ)

        // pick-up all layers and put them in IGU (this includes gap layers as well)
        for (int i = 0; i < m_TotLay; ++i) {
            auto aLayer = getIGULayer(state, i + 1); // (AUTO_OK_SPTR)
            assert(aLayer != nullptr);
            // IDF for "standard" windows do not insert gas between glass and shade. Tarcog needs that gas
            // and it will be created here
            if (m_ShadePosition == ShadePosition::Interior && i == m_TotLay - 1) {
                auto aAirLayer = getShadeToGlassLayer(state, i + 1); // (AUTO_OK_SPTR)
                aIGU.addLayer(aAirLayer);
            }
            aIGU.addLayer(aLayer);
            if (m_ShadePosition == ShadePosition::Exterior && i == 0) {
                auto aAirLayer = getShadeToGlassLayer(state, i + 1); // (AUTO_OK_SPTR)
                aIGU.addLayer(aAirLayer);
            }
        }

        return std::make_shared<Tarcog::ISO15099::CSingleSystem>(aIGU, Indoor, Outdoor);
    }

    std::shared_ptr<Tarcog::ISO15099::IIGUSystem> CWCEHeatTransferFactory::getTarcogSystemForReporting(
        EnergyPlusData &state, bool const useSummerConditions, const Real64 width, const Real64 height, const Real64 tilt)
    {
        auto Indoor = getIndoorNfrc(useSummerConditions);   // (AUTO_OK_SPTR)
        auto Outdoor = getOutdoorNfrc(useSummerConditions); // (AUTO_OK_SPTR)
        auto aIGU = getIGU(width, height, tilt);            // (AUTO_OK_OBJ)

        m_SolidLayerIndex = 0;
        // pick-up all layers and put them in IGU (this includes gap layers as well)
        for (int i = 0; i < m_TotLay; ++i) {
            auto aLayer = getIGULayer(state, i + 1); // (AUTO_OK_SPTR)
            assert(aLayer != nullptr);
            // IDF for "standard" windows do not insert gas between glass and shade. Tarcog needs that gas
            // and it will be created here
            if (m_ShadePosition == ShadePosition::Interior && i == m_TotLay - 1) {
                auto aAirLayer = getShadeToGlassLayer(state, i + 1); // (AUTO_OK_SPTR)
                aIGU.addLayer(aAirLayer);
            }
            aIGU.addLayer(aLayer);
            if (m_ShadePosition == ShadePosition::Exterior && i == 0) {
                auto aAirLayer = getShadeToGlassLayer(state, i + 1); // (AUTO_OK_SPTR)
                aIGU.addLayer(aAirLayer);
            }
        }

        return std::make_shared<Tarcog::ISO15099::CSystem>(aIGU, Indoor, Outdoor);
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    Material::MaterialBase *CWCEHeatTransferFactory::getLayerMaterial(EnergyPlusData &state, int const t_Index) const
    {
        int ConstrNum = m_ConstructionNumber;

        // BSDF window do not have special shading flag
        if (!state.dataConstruction->Construct(ConstrNum).WindowTypeBSDF &&
            state.dataSurface->SurfWinShadingFlag.size() >= static_cast<size_t>(m_SurfNum)) {
            if (ANY_SHADE_SCREEN(state.dataSurface->SurfWinShadingFlag(m_SurfNum)) || ANY_BLIND(state.dataSurface->SurfWinShadingFlag(m_SurfNum))) {
                ConstrNum = state.dataSurface->SurfWinActiveShadedConstruction(m_SurfNum);
            }
        }

        auto &construction = state.dataConstruction->Construct(ConstrNum);
        const int LayPtr = construction.LayerPoint(t_Index);
        return state.dataMaterial->materials(LayPtr);
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<Tarcog::ISO15099::CBaseIGULayer> CWCEHeatTransferFactory::getIGULayer(EnergyPlusData &state, int const t_Index)
    {
        std::shared_ptr<Tarcog::ISO15099::CBaseIGULayer> aLayer = nullptr;

        auto *material = getLayerMaterial(state, t_Index);

        Material::Group matGroup = material->group;

        if ((matGroup == Material::Group::Glass) || (matGroup == Material::Group::GlassSimple) || (matGroup == Material::Group::Blind) ||
            (matGroup == Material::Group::Shade) || (matGroup == Material::Group::Screen) || (matGroup == Material::Group::ComplexShade)) {
            ++m_SolidLayerIndex;
            aLayer = getSolidLayer(state, material, m_SolidLayerIndex);
        } else if (matGroup == Material::Group::Gas || matGroup == Material::Group::GasMixture) {
            aLayer = getGapLayer(material);
        } else if (matGroup == Material::Group::ComplexWindowGap) {
            aLayer = getComplexGapLayer(state, material);
        }

        return aLayer;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    int CWCEHeatTransferFactory::getNumOfLayers(EnergyPlusData &state) const
    {
        return state.dataConstruction->Construct(m_ConstructionNumber).TotLayers;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<Tarcog::ISO15099::CBaseIGULayer>
    CWCEHeatTransferFactory::getSolidLayer(EnergyPlusData &state, Material::MaterialBase const *mat, int const t_Index)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   July 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Creates solid layer object from material properties in EnergyPlus
        Real64 emissFront = 0.0;
        Real64 emissBack = 0.0;
        Real64 transThermalFront = 0.0;
        Real64 transThermalBack = 0.0;
        Real64 thickness = 0.0;
        Real64 conductivity = 0.0;
        Real64 createOpenness = false;
        Real64 Atop = 0.0;
        Real64 Abot = 0.0;
        Real64 Aleft = 0.0;
        Real64 Aright = 0.0;
        Real64 Afront = 0.0;

        if (mat->group == Material::Group::Glass || mat->group == Material::Group::GlassSimple) {
            auto const *matGlass = dynamic_cast<Material::MaterialGlass const *>(mat);
            assert(matGlass != nullptr);

            emissFront = matGlass->AbsorpThermalFront;
            emissBack = matGlass->AbsorpThermalBack;
            transThermalFront = matGlass->TransThermal;
            transThermalBack = matGlass->TransThermal;
            thickness = matGlass->Thickness;
            conductivity = matGlass->Conductivity;

        } else if (mat->group == Material::Group::Blind) {
            // auto const &surfShade = state.dataSurface->surfShades(m_SurfNum);
            auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(mat);
            assert(matBlind != nullptr);
            thickness = matBlind->SlatThickness;
            conductivity = matBlind->SlatConductivity;
            Atop = matBlind->topOpeningMult;
            Abot = matBlind->bottomOpeningMult;
            Aleft = matBlind->leftOpeningMult;
            Aright = matBlind->rightOpeningMult;

            Real64 slatAng = matBlind->SlatAngle * Constant::DegToRad;
            Real64 PermA = std::sin(slatAng) - matBlind->SlatThickness / matBlind->SlatSeparation;
            Real64 PermB =
                1.0 - (std::abs(matBlind->SlatWidth * std::cos(slatAng)) + matBlind->SlatThickness * std::sin(slatAng)) / matBlind->SlatSeparation;
            Afront = min(1.0, max(0.0, PermA, PermB));

            int iSlatLo, iSlatHi;
            Real64 interpFac;

            Material::GetSlatIndicesInterpFac(slatAng, iSlatLo, iSlatHi, interpFac);

            emissFront = Interp(matBlind->TARs[iSlatLo].IR.Ft.Emi, matBlind->TARs[iSlatHi].IR.Ft.Emi, interpFac);
            emissBack = Interp(matBlind->TARs[iSlatLo].IR.Bk.Emi, matBlind->TARs[iSlatHi].IR.Bk.Emi, interpFac);
            transThermalFront = Interp(matBlind->TARs[iSlatLo].IR.Ft.Tra, matBlind->TARs[iSlatHi].IR.Ft.Tra, interpFac);
            transThermalBack = Interp(matBlind->TARs[iSlatLo].IR.Bk.Tra, matBlind->TARs[iSlatHi].IR.Bk.Tra, interpFac);

            if (t_Index == 1) {
                m_ExteriorShade = true;
            }

        } else if (mat->group == Material::Group::Shade) {
            auto const *matShade = dynamic_cast<Material::MaterialShade const *>(mat);
            assert(matShade != nullptr);

            emissFront = matShade->AbsorpThermal;
            emissBack = matShade->AbsorpThermal;
            transThermalFront = matShade->TransThermal;
            transThermalBack = matShade->TransThermal;
            thickness = matShade->Thickness;
            conductivity = matShade->Conductivity;

            Atop = matShade->topOpeningMult;
            Abot = matShade->bottomOpeningMult;
            Aleft = matShade->leftOpeningMult;
            Aright = matShade->rightOpeningMult;
            Afront = matShade->airFlowPermeability;
            if (t_Index == 1) {
                m_ExteriorShade = true;
            }

        } else if (mat->group == Material::Group::Screen) {
            auto const *matScreen = dynamic_cast<Material::MaterialScreen const *>(mat);
            assert(matScreen != nullptr);

            // Simon: Existing code already takes into account geometry of Woven and scales down
            // emissivity for opening area.
            emissFront = matScreen->AbsorpThermal;
            emissBack = matScreen->AbsorpThermal;
            transThermalFront = matScreen->TransThermal;
            transThermalBack = matScreen->TransThermal;
            thickness = matScreen->Thickness;
            conductivity = matScreen->Conductivity;

            Atop = matScreen->topOpeningMult;
            Abot = matScreen->bottomOpeningMult;
            Aleft = matScreen->leftOpeningMult;
            Aright = matScreen->rightOpeningMult;
            Afront = matScreen->airFlowPermeability;
            if (t_Index == 1) {
                m_ExteriorShade = true;
            }

        } else if (mat->group == Material::Group::ComplexShade) {
            auto const *matShade = dynamic_cast<Material::MaterialComplexShade const *>(mat);
            assert(matShade != nullptr);

            thickness = matShade->Thickness;
            conductivity = matShade->Conductivity;
            emissFront = matShade->FrontEmissivity;
            emissBack = matShade->BackEmissivity;
            transThermalFront = matShade->TransThermal;
            transThermalBack = matShade->TransThermal;
            Afront = matShade->frontOpeningMult;
            Atop = matShade->topOpeningMult;
            Abot = matShade->bottomOpeningMult;
            Aleft = matShade->leftOpeningMult;
            Aright = matShade->rightOpeningMult;
            createOpenness = true;
            m_InteriorBSDFShade = ((2 * t_Index - 1) == m_TotLay);
        }

        std::shared_ptr<Tarcog::ISO15099::ISurface> frontSurface = std::make_shared<Tarcog::ISO15099::CSurface>(emissFront, transThermalFront);
        std::shared_ptr<Tarcog::ISO15099::ISurface> backSurface = std::make_shared<Tarcog::ISO15099::CSurface>(emissBack, transThermalBack);
        auto aSolidLayer = // (AUTO_OK_SPTR)
            std::make_shared<Tarcog::ISO15099::CIGUSolidLayer>(thickness, conductivity, frontSurface, backSurface);
        if (createOpenness) {
            auto aOpenings = std::make_shared<Tarcog::ISO15099::CShadeOpenings>(Atop, Abot, Aleft, Aright, Afront, Afront); // (AUTO_OK_SPTR)
            aSolidLayer = std::make_shared<Tarcog::ISO15099::CIGUShadeLayer>(aSolidLayer, aOpenings);
        }
        static constexpr double standardizedRadiationIntensity = 783.0;
        if (state.dataWindowManager->inExtWindowModel->isExternalLibraryModel()) {
            auto &surface(state.dataSurface->Surface(m_SurfNum));
            const int ConstrNum = getActiveConstructionNumber(state, surface, m_SurfNum);
            std::shared_ptr<MultiLayerOptics::CMultiLayerScattered> aLayer =
                CWindowConstructionsSimplified::instance(state).getEquivalentLayer(state, FenestrationCommon::WavelengthRange::Solar, ConstrNum);

            // Report is done for normal incidence
            constexpr Real64 Theta = 0.0;
            constexpr Real64 Phi = 0.0;
            const Real64 absCoeff =
                aLayer->getAbsorptanceLayer(t_Index, FenestrationCommon::Side::Front, FenestrationCommon::ScatteringSimple::Diffuse, Theta, Phi);
            aSolidLayer->setSolarAbsorptance(absCoeff, standardizedRadiationIntensity);
        } else {
            const Real64 absCoeff{state.dataConstruction->Construct(state.dataSurface->Surface(m_SurfNum).Construction).AbsDiff(t_Index)};
            aSolidLayer->setSolarAbsorptance(absCoeff, standardizedRadiationIntensity);
        }
        return aSolidLayer;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<Tarcog::ISO15099::CBaseIGULayer> CWCEHeatTransferFactory::getGapLayer(Material::MaterialBase const *material) const
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   July 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Creates gap layer object from material properties in EnergyPlus
        Real64 constexpr pres = 1e5; // Old code uses this constant pressure
        Real64 thickness = material->Thickness;
        auto aGas = getGas(material); // (AUTO_OK_OBJ)
        std::shared_ptr<Tarcog::ISO15099::CBaseIGULayer> aLayer = std::make_shared<Tarcog::ISO15099::CIGUGapLayer>(thickness, pres, aGas);
        return aLayer;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<Tarcog::ISO15099::CBaseIGULayer> CWCEHeatTransferFactory::getShadeToGlassLayer(EnergyPlusData &state, int const t_Index) const
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   August 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Creates gap layer object from material properties in EnergyPlus
        Real64 constexpr pres = 1e5; // Old code uses this constant pressure
        auto aGas = getAir();        // (AUTO_OK_OBJ)
        Real64 thickness = 0.0;

        auto &s_mat = state.dataMaterial;
        auto &surfWin = state.dataSurface->SurfaceWindow(m_SurfNum);
        auto const &surfShade = state.dataSurface->surfShades(m_SurfNum);

        const WinShadingType ShadeFlag = getShadeType(state, m_ConstructionNumber);

        if (ShadeFlag == WinShadingType::IntBlind || ShadeFlag == WinShadingType::ExtBlind) {
            thickness = dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShade.blind.matNum))->toGlassDist;
        } else if (ShadeFlag == WinShadingType::ExtScreen) {
            thickness = dynamic_cast<Material::MaterialScreen const *>(s_mat->materials(surfWin.screenNum))->toGlassDist;
        } else if (ShadeFlag == WinShadingType::IntShade || ShadeFlag == WinShadingType::ExtShade) {
            const auto *material = dynamic_cast<Material::MaterialShade *>(getLayerMaterial(state, t_Index));
            assert(material != nullptr);
            thickness = material->toGlassDist;
        }
        std::shared_ptr<Tarcog::ISO15099::CBaseIGULayer> aLayer = std::make_shared<Tarcog::ISO15099::CIGUGapLayer>(thickness, pres, aGas);
        return aLayer;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<Tarcog::ISO15099::CBaseIGULayer> CWCEHeatTransferFactory::getComplexGapLayer([[maybe_unused]] EnergyPlusData &state,
                                                                                                 Material::MaterialBase const *materialBase) const
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   July 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Creates gap layer object from material properties in EnergyPlus
        Real64 constexpr pres = 1e5; // Old code uses this constant pressure
        auto const *mat = dynamic_cast<Material::MaterialComplexWindowGap const *>(materialBase);
        assert(mat != nullptr);
        Real64 thickness = mat->Thickness;
        auto aGas = getGas(mat); // (AUTO_OK_OBJ)
        return std::make_shared<Tarcog::ISO15099::CIGUGapLayer>(thickness, pres, aGas);
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    Gases::CGas CWCEHeatTransferFactory::getGas(Material::MaterialBase const *materialBase) const
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   July 2016
        //       MODIFIED       na
        //       RE-ENGINEERED
        //          April 2021: April 2021: Return of CGas instead of pointer to it

        // PURPOSE OF THIS SUBROUTINE:
        // Creates gap layer object from material properties in EnergyPlus
        auto const *matGas = dynamic_cast<Material::MaterialGasMix const *>(materialBase);
        assert(matGas != nullptr);
        const int numGases = matGas->numGases;
        double constexpr vacuumCoeff = 1.4; // Load vacuum coefficient once it is implemented (Simon).
        std::string const &gasName = matGas->Name;
        Gases::CGas aGas;
        for (int i = 0; i < numGases; ++i) {
            auto const &gas = matGas->gases[i];
            Real64 wght = gas.wght;
            Real64 fract = matGas->gasFracts[i];
            Gases::CIntCoeff aCon(gas.con.c0, gas.con.c1, gas.con.c2);
            Gases::CIntCoeff aCp(gas.cp.c0, gas.cp.c1, gas.cp.c2);
            Gases::CIntCoeff aVis(gas.vis.c0, gas.vis.c1, gas.vis.c2);
            Gases::CGasData aData(gasName, wght, vacuumCoeff, aCp, aCon, aVis);
            aGas.addGasItem(fract, aData);
        }
        return aGas;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    Gases::CGas CWCEHeatTransferFactory::getAir()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   August 2016
        //       MODIFIED       na
        //       RE-ENGINEERED
        //          April 2021: CGas return from the function instead of pointer to it

        // PURPOSE OF THIS SUBROUTINE:
        // Creates air gas layer for tarcog routines
        return {};
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<Tarcog::ISO15099::CEnvironment> CWCEHeatTransferFactory::getIndoor(EnergyPlusData &state) const
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   July 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Creates indoor environment object from surface properties in EnergyPlus
        Real64 tin = m_Surface.getInsideAirTemperature(state, m_SurfNum) + Constant::Kelvin;
        Real64 hcin = state.dataHeatBalSurf->SurfHConvInt(m_SurfNum);

        Real64 IR = state.dataSurface->SurfWinIRfromParentZone(m_SurfNum) + state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(m_SurfNum);

        std::shared_ptr<Tarcog::ISO15099::CEnvironment> Indoor =
            std::make_shared<Tarcog::ISO15099::CIndoorEnvironment>(tin, state.dataEnvrn->OutBaroPress);
        Indoor->setHCoeffModel(Tarcog::ISO15099::BoundaryConditionsCoeffModel::CalculateH, hcin);
        Indoor->setEnvironmentIR(IR);
        return Indoor;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<Tarcog::ISO15099::CEnvironment> CWCEHeatTransferFactory::getOutdoor(EnergyPlusData &state, const Real64 t_Hext) const
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   July 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Creates outdoor environment object from surface properties in EnergyPlus
        double tout = m_Surface.getOutsideAirTemperature(state, m_SurfNum) + Constant::Kelvin;
        double IR = m_Surface.getOutsideIR(state, m_SurfNum);
        // double dirSolRad = SurfQRadSWOutIncident( t_SurfNum ) + QS( Surface( t_SurfNum ).Zone );
        double swRadiation = m_Surface.getSWIncident(state, m_SurfNum);
        double tSky = state.dataEnvrn->SkyTempKelvin;
        double airSpeed = 0.0;
        if (m_Surface.ExtWind) {
            airSpeed = state.dataSurface->SurfOutWindSpeed(m_SurfNum);
        }
        double fclr = 1 - state.dataEnvrn->CloudFraction;
        Tarcog::ISO15099::AirHorizontalDirection airDirection = Tarcog::ISO15099::AirHorizontalDirection::Windward;
        std::shared_ptr<Tarcog::ISO15099::CEnvironment> Outdoor = std::make_shared<Tarcog::ISO15099::COutdoorEnvironment>(
            tout, airSpeed, swRadiation, airDirection, tSky, Tarcog::ISO15099::SkyModel::AllSpecified, state.dataEnvrn->OutBaroPress, fclr);
        Outdoor->setHCoeffModel(Tarcog::ISO15099::BoundaryConditionsCoeffModel::HcPrescribed, t_Hext);
        Outdoor->setEnvironmentIR(IR);
        return Outdoor;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    Tarcog::ISO15099::CIGU CWCEHeatTransferFactory::getIGU() const
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   July 2016
        //       MODIFIED       na
        //       RE-ENGINEERED
        //          April 2021: Return CIGU object rather than pointer to it

        // PURPOSE OF THIS SUBROUTINE:
        // Creates IGU object from surface properties in EnergyPlus

        return {m_Surface.Width, m_Surface.Height, m_Surface.Tilt};
    }

    Tarcog::ISO15099::CIGU CWCEHeatTransferFactory::getIGU(double width, double height, double tilt)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   November 2021
        //       MODIFIED       na
        //       RE-ENGINEERED
        //          April 2021: Return CIGU object rather than pointer to it

        // PURPOSE OF THIS SUBROUTINE:
        // Creates IGU object for given width, height and tilt

        return {width, height, tilt};
    }

    int
    CWCEHeatTransferFactory::getActiveConstructionNumber(EnergyPlusData &state, EnergyPlus::DataSurfaces::SurfaceData const &surface, int t_SurfNum)
    {
        int result = surface.Construction;
        const WinShadingType ShadeFlag = state.dataSurface->SurfWinShadingFlag(t_SurfNum);

        if (ANY_SHADE_SCREEN(ShadeFlag) || ANY_BLIND(ShadeFlag)) {
            result = state.dataSurface->SurfWinActiveShadedConstruction(t_SurfNum);
        }

        return result;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    bool CWCEHeatTransferFactory::isInteriorShade() const
    {
        return m_InteriorBSDFShade;
    }

    std::shared_ptr<Tarcog::ISO15099::CEnvironment> CWCEHeatTransferFactory::getOutdoorNfrc(bool const useSummerConditions)
    {
        // NFRC 100 Section 4.3.1
        Real64 airTemperature = -18.0 + Constant::Kelvin; // Kelvins
        Real64 airSpeed = 5.5;                            // meters per second
        Real64 tSky = -18.0 + Constant::Kelvin;           // Kelvins
        Real64 solarRadiation = 0.;                       // W/m2
        if (useSummerConditions) {
            // NFRC 200 Section 4.3.1
            airTemperature = 32.0 + Constant::Kelvin;
            airSpeed = 2.75;
            tSky = 32.0 + Constant::Kelvin;
            solarRadiation = 783.;
        }
        auto Outdoor = // (AUTO_OK_SPTR)
            Tarcog::ISO15099::Environments::outdoor(airTemperature, airSpeed, solarRadiation, tSky, Tarcog::ISO15099::SkyModel::AllSpecified);
        Outdoor->setHCoeffModel(Tarcog::ISO15099::BoundaryConditionsCoeffModel::CalculateH);
        return Outdoor;
    }

    std::shared_ptr<Tarcog::ISO15099::CEnvironment> CWCEHeatTransferFactory::getIndoorNfrc(bool const useSummerConditions)
    {
        // NFRC 100 Section 4.3.1
        Real64 roomTemperature = 21. + Constant::Kelvin;
        if (useSummerConditions) {
            // NFRC 200 Section 4.3.1
            roomTemperature = 24. + Constant::Kelvin;
        }
        return Tarcog::ISO15099::Environments::indoor(roomTemperature);
    }

    WinShadingType CWCEHeatTransferFactory::getShadeType(EnergyPlusData &state, int ConstrNum)
    {
        auto &s_mat = state.dataMaterial;
        WinShadingType ShadeFlag = WinShadingType::NoShade;

        const int TotLay = state.dataConstruction->Construct(ConstrNum).TotLayers;
        const int TotGlassLay = state.dataConstruction->Construct(ConstrNum).TotGlassLayers;
        const int matOutNum = state.dataConstruction->Construct(ConstrNum).LayerPoint(1);
        const int matInNum = state.dataConstruction->Construct(ConstrNum).LayerPoint(TotLay);

        auto const *matOut = s_mat->materials(matOutNum);
        auto const *matIn = s_mat->materials(matInNum);

        if (matOut->group == Material::Group::Shade) { // Exterior shade present
            ShadeFlag = WinShadingType::ExtShade;
        } else if (matOut->group == Material::Group::Screen) { // Exterior screen present
            ShadeFlag = WinShadingType::ExtScreen;
        } else if (matOut->group == Material::Group::Blind) { // Exterior blind present
            ShadeFlag = WinShadingType::ExtBlind;
        } else if (matIn->group == Material::Group::Shade) { // Interior shade present
            ShadeFlag = WinShadingType::IntShade;
        } else if (matIn->group == Material::Group::Blind) { // Interior blind present
            ShadeFlag = WinShadingType::IntBlind;
        } else if (TotGlassLay == 2) {
            auto const *mat3 = s_mat->materials(state.dataConstruction->Construct(ConstrNum).LayerPoint(3));
            if (mat3->group == Material::Group::Shade) {
                ShadeFlag = WinShadingType::BGShade;
            } else if (mat3->group == Material::Group::Blind) {
                ShadeFlag = WinShadingType::BGBlind;
            }
        } else if (TotGlassLay == 3) {
            auto const *mat5 = s_mat->materials(state.dataConstruction->Construct(ConstrNum).LayerPoint(5));
            if (mat5->group == Material::Group::Shade) {
                ShadeFlag = WinShadingType::BGShade;
            } else if (mat5->group == Material::Group::Blind) {
                ShadeFlag = WinShadingType::BGBlind;
            }
        }

        return ShadeFlag;
    }

    double CWCEHeatTransferFactory::overallUfactorFromFilmsAndCond(double conductance, double insideFilm, double outsideFilm)
    {
        double rOverall(0.);
        double uFactor(0.);
        if (insideFilm != 0 && outsideFilm != 0. && conductance != 0.) {
            rOverall = 1 / insideFilm + 1 / conductance + 1 / outsideFilm;
        }
        if (rOverall != 0.) {
            uFactor = 1 / rOverall;
        }
        return uFactor;
    }

} // namespace Window
} // namespace EnergyPlus
