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

#include <cassert>

// EnergyPlus headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/WindowManager.hh>

// Windows library headers
#include <WCEMultiLayerOptics.hpp>

#include "WindowManagerExteriorData.hh"
#include "WindowManagerExteriorOptical.hh"

namespace EnergyPlus {

using namespace FenestrationCommon;
using namespace SpectralAveraging;
using namespace SingleLayerOptics;

using namespace DataEnvironment;
using namespace DataSurfaces;
using namespace DataHeatBalance;
namespace Window {

    std::shared_ptr<CBSDFLayer> getBSDFLayer(EnergyPlusData &state, const Material::MaterialBase *t_Material, const WavelengthRange t_Range)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   September 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // BSDF will be created in different ways that is based on material type

        std::shared_ptr<CWCELayerFactory> aFactory = nullptr;
        if (t_Material->group == Material::Group::Glass) {
            aFactory = std::make_shared<CWCESpecularLayerFactory>(t_Material, t_Range);
        } else if (t_Material->group == Material::Group::Blind) {
            aFactory = std::make_shared<CWCEVenetianBlindLayerFactory>(t_Material, t_Range);
        } else if (t_Material->group == Material::Group::Screen) {
            aFactory = std::make_shared<CWCEScreenLayerFactory>(t_Material, t_Range);
        } else if (t_Material->group == Material::Group::Shade) {
            aFactory = std::make_shared<CWCEDiffuseShadeLayerFactory>(t_Material, t_Range);
        }
        return aFactory->getBSDFLayer(state);
    }

    CScatteringLayer getScatteringLayer(EnergyPlusData &state, const Material::MaterialBase *t_Material, const WavelengthRange t_Range)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   May 2017
        //       MODIFIED       na
        //       RE-ENGINEERED
        //          April 2021: returning CScatteringLayer instead of pointer to it

        // PURPOSE OF THIS SUBROUTINE:
        // Scattering will be created in different ways that is based on material type

        std::shared_ptr<CWCELayerFactory> aFactory = nullptr;
        if (t_Material->group == Material::Group::Glass || t_Material->group == Material::Group::GlassSimple) {
            aFactory = std::make_shared<CWCESpecularLayerFactory>(t_Material, t_Range);
        } else if (t_Material->group == Material::Group::Blind) {
            aFactory = std::make_shared<CWCEVenetianBlindLayerFactory>(t_Material, t_Range);
        } else if (t_Material->group == Material::Group::Screen) {
            aFactory = std::make_shared<CWCEScreenLayerFactory>(t_Material, t_Range);
        } else if (t_Material->group == Material::Group::Shade) {
            aFactory = std::make_shared<CWCEDiffuseShadeLayerFactory>(t_Material, t_Range);
        }
        return aFactory->getLayer(state);
    }

    // void InitWCE_BSDFOpticalData() {
    //     // SUBROUTINE INFORMATION:
    //     //       AUTHOR         Simon Vidanovic
    //     //       DATE WRITTEN   September 2016
    //     //       MODIFIED       na
    //     //       RE-ENGINEERED  na
    //
    //     // PURPOSE OF THIS SUBROUTINE:
    //     // Initialize BSDF construction layers in Solar and Visible spectrum.
    //
    //     auto aWinConstBSDF = CWindowConstructionsBSDF::instance();
    //     for ( auto ConstrNum = 1; ConstrNum <= TotConstructs; ++ConstrNum ) {
    //         auto& construction( Construct( ConstrNum ) );
    //         if ( construction.isGlazingConstruction() ) {
    //             for ( auto LayNum = 1; LayNum <= construction.TotLayers; ++LayNum ) {
    //                 auto& material( dataMaterial.Material( construction.LayerPoint( LayNum ) ) );
    //                 if ( material->group != WindowGas && material->group != WindowGasMixture &&
    //                     material->group != ComplexWindowGap && material->group != ComplexWindowShade ) {
    //                     auto aMaterial = std::make_shared< Material::MaterialBase >();
    //                     *aMaterial = material;
    //
    //                     // This is necessary because rest of EnergyPlus code relies on TransDiff property
    //                     // of construction. It will basically trigger Window optical calculations if this
    //                     // property is >0.
    //                     construction.TransDiff = 0.1;
    //
    //                     auto aRange = WavelengthRange::Solar;
    //                     auto aSolarLayer = getBSDFLayer( aMaterial, aRange );
    //                     aWinConstBSDF.pushBSDFLayer( aRange, ConstrNum, aSolarLayer );
    //
    //                     aRange = WavelengthRange::Visible;
    //                     auto aVisibleLayer = getBSDFLayer( aMaterial, aRange );
    //                     aWinConstBSDF.pushBSDFLayer( aRange, ConstrNum, aVisibleLayer );
    //                 }
    //
    //             }
    //         }
    //     }
    // }

    void InitWCE_SimplifiedOpticalData(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   May 2017
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Initialize scattering construction layers in Solar and Visible spectrum.

        // Calculate optical properties of blind-type layers entered with MATERIAL:WindowBlind
        // Calculation from this is used for IR properties. Need to make sure that properties
        // are calculated with new WCE optical engine (for both blinds and screens)
        auto &s_mat = state.dataMaterial;

        if (s_mat->NumBlinds > 0) {
            CalcWindowBlindProperties(state);
        }

        // Initialize SurfaceScreen structure
        if (s_mat->NumScreens > 0) {
            CalcWindowScreenProperties(state);
        }

        auto &aWinConstSimp = CWindowConstructionsSimplified::instance(state);
        for (int ConstrNum = 1; ConstrNum <= state.dataHeatBal->TotConstructs; ++ConstrNum) {
            auto &construction = state.dataConstruction->Construct(ConstrNum);
            if (construction.isGlazingConstruction(state)) {
                for (int LayNum = 1; LayNum <= construction.TotLayers; ++LayNum) {
                    auto const *mat = s_mat->materials(construction.LayerPoint(LayNum));
                    if (mat->group != Material::Group::Gas && mat->group != Material::Group::GasMixture &&
                        mat->group != Material::Group::ComplexWindowGap && mat->group != Material::Group::ComplexShade) {
                        // This is necessary because rest of EnergyPlus code relies on TransDiff property
                        // of construction. It will basically trigger Window optical calculations if this
                        // property is >0.
                        construction.TransDiff = 0.1;

                        WavelengthRange aRange = WavelengthRange::Solar;
                        auto aSolarLayer = getScatteringLayer(state, mat, aRange); // (AUTO_OK_OBJ)
                        aWinConstSimp.pushLayer(aRange, ConstrNum, aSolarLayer);

                        aRange = WavelengthRange::Visible;
                        auto aVisibleLayer = getScatteringLayer(state, mat, aRange); // (AUTO_OK_OBJ)
                        aWinConstSimp.pushLayer(aRange, ConstrNum, aVisibleLayer);
                    }
                }
            }
        }

        // Get effective glass and shade/blind emissivities for windows that have interior blind or
        // shade. These are used to calculate zone MRT contribution from window when
        // interior blind/shade is deployed.

        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            auto &surf = state.dataSurface->Surface(SurfNum);
            auto &surfShade = state.dataSurface->surfShades(SurfNum);

            if (!surf.HeatTransSurf) {
                continue;
            }
            if (!state.dataConstruction->Construct(surf.Construction).TypeIsWindow) {
                continue;
            }
            if (state.dataSurface->SurfWinWindowModelType(SurfNum) == WindowModel::BSDF) {
                continue; // Irrelevant for Complex Fen
            }
            if (state.dataConstruction->Construct(surf.Construction).WindowTypeEQL) {
                continue; // not required
            }

            if (surf.activeShadedConstruction == 0) {
                continue;
            }
            auto &constrSh = state.dataConstruction->Construct(surf.activeShadedConstruction);
            int TotLay = constrSh.TotLayers;
            auto const *mat = s_mat->materials(constrSh.LayerPoint(TotLay));

            if (mat->group == Material::Group::Shade) {
                auto const *matShade = dynamic_cast<Material::MaterialShade const *>(mat);
                Real64 EpsGlIR = s_mat->materials(constrSh.LayerPoint(TotLay - 1))->AbsorpThermalBack;
                Real64 RhoGlIR = 1 - EpsGlIR;
                Real64 TauShIR = matShade->TransThermal;
                Real64 EpsShIR = matShade->AbsorpThermal;
                Real64 RhoShIR = max(0.0, 1.0 - TauShIR - EpsShIR);
                surfShade.effShadeEmi = EpsShIR * (1.0 + RhoGlIR * TauShIR / (1.0 - RhoGlIR * RhoShIR));
                surfShade.effGlassEmi = EpsGlIR * TauShIR / (1.0 - RhoGlIR * RhoShIR);

            } else if (mat->group == Material::Group::Blind) {
                Real64 EpsGlIR = s_mat->materials(constrSh.LayerPoint(TotLay - 1))->AbsorpThermalBack;
                Real64 RhoGlIR = 1 - EpsGlIR;

                auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(mat);
                for (int iSlatAng = 0; iSlatAng < Material::MaxSlatAngs; ++iSlatAng) {
                    auto const &btar = matBlind->TARs[iSlatAng];
                    Real64 TauShIR = btar.IR.Ft.Tra;
                    Real64 EpsShIR = btar.IR.Ft.Emi;
                    Real64 RhoShIR = max(0.0, 1.0 - TauShIR - EpsShIR);
                    constrSh.effShadeBlindEmi[iSlatAng] = EpsShIR * (1.0 + RhoGlIR * TauShIR / (1.0 - RhoGlIR * RhoShIR));
                    constrSh.effGlassEmi[iSlatAng] = EpsGlIR * TauShIR / (1.0 - RhoGlIR * RhoShIR);
                }
                // The following surface assignments previously lived in the WindowManagerExteriorThermal file.
                // They were moved here during the Material refactor, however I found that the array index values Lo/Hi
                // were making it here still as -1, and this was causing a difficult to diagnose segfault.
                // I'm moving these back over there, but open to suggestion if they need to be here.
                // surfShade.effShadeEmi = Interp(constrSh.effShadeBlindEmi[surfShade.blind.slatAngIdxLo],
                //                                constrSh.effShadeBlindEmi[surfShade.blind.slatAngIdxHi],
                //                                surfShade.blind.slatAngInterpFac);
                // surfShade.effGlassEmi = Interp(constrSh.effGlassEmi[surfShade.blind.slatAngIdxLo],
                //                                constrSh.effGlassEmi[surfShade.blind.slatAngIdxHi],
                //                                surfShade.blind.slatAngInterpFac);
            } // End of check if interior shade or interior blind
        } // End of surface loop
    } // InitWCE_SimplifiedOpticalData()

    Real64 GetSolarTransDirectHemispherical(EnergyPlusData &state, int ConstrNum)
    {
        const auto aWinConstSimp = // (AUTO_OK_SHARED_PTR)
            CWindowConstructionsSimplified::instance(state).getEquivalentLayer(state, FenestrationCommon::WavelengthRange::Solar, ConstrNum);
        return aWinConstSimp->getPropertySimple(
            0.3, 2.5, FenestrationCommon::PropertySimple::T, FenestrationCommon::Side::Front, FenestrationCommon::Scattering::DirectHemispherical);
    }

    Real64 GetVisibleTransDirectHemispherical(EnergyPlusData &state, int ConstrNum)
    {
        const auto aWinConstSimp = // (AUTO_OK_SHARED_PTR)
            CWindowConstructionsSimplified::instance(state).getEquivalentLayer(state, FenestrationCommon::WavelengthRange::Visible, ConstrNum);
        return aWinConstSimp->getPropertySimple(
            0.38, 0.78, FenestrationCommon::PropertySimple::T, FenestrationCommon::Side::Front, FenestrationCommon::Scattering::DirectHemispherical);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCEMaterialFactory
    ///////////////////////////////////////////////////////////////////////////////
    CWCEMaterialFactory::CWCEMaterialFactory(const Material::MaterialBase *t_Material, const WavelengthRange t_Range)
        : m_MaterialProperties(t_Material), m_Range(t_Range), m_Initialized(false)
    {
    }

    std::shared_ptr<CMaterial> CWCEMaterialFactory::getMaterial(EnergyPlusData &state)
    {
        if (!m_Initialized) {
            init(state);
            m_Initialized = true;
        }
        return m_Material;
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCESpecularMaterialsFactory
    ///////////////////////////////////////////////////////////////////////////////
    CWCESpecularMaterialsFactory::CWCESpecularMaterialsFactory(const Material::MaterialBase *t_Material, const WavelengthRange t_Range)
        : CWCEMaterialFactory(t_Material, t_Range)
    {
    }

    void CWCESpecularMaterialsFactory::init(EnergyPlusData &state)
    {
        auto const *matGlass = dynamic_cast<Material::MaterialGlass const *>(m_MaterialProperties);
        assert(matGlass != nullptr);

        if (matGlass->GlassSpectralDataPtr > 0) {
            auto aSolarSpectrum = CWCESpecturmProperties::getDefaultSolarRadiationSpectrum(state); // (AUTO_OK_OBJ)
            std::shared_ptr<CSpectralSampleData> aSampleData = nullptr;
            aSampleData = CWCESpecturmProperties::getSpectralSample(state, matGlass->GlassSpectralDataPtr);

            auto aSample = std::make_shared<CSpectralSample>(aSampleData, aSolarSpectrum); // (AUTO_OK_SHARED_PTR)

            FenestrationCommon::MaterialType aType = MaterialType::Monolithic;
            CWavelengthRange aRange(m_Range);
            Real64 lowLambda = aRange.minLambda();
            Real64 highLambda = aRange.maxLambda();

            // Do not apply detector data if we do not have spectral data. This will only cause more inaccurate results at the end. (Simon)
            if (m_Range == WavelengthRange::Visible && matGlass->GlassSpectralDataPtr != 0) {
                const auto aPhotopicResponse = CWCESpecturmProperties::getDefaultVisiblePhotopicResponse(state); // (AUTO_OK_OBJ)
                aSample->setDetectorData(aPhotopicResponse);
            }

            Real64 thickness = matGlass->Thickness;
            m_Material = std::make_shared<CMaterialSample>(aSample, thickness, aType, lowLambda, highLambda);
        } else {
            if (m_Range == WavelengthRange::Solar) {
                m_Material = std::make_shared<CMaterialSingleBand>(
                    matGlass->Trans, matGlass->Trans, matGlass->ReflectSolBeamFront, matGlass->ReflectSolBeamBack, m_Range);
            }
            if (m_Range == WavelengthRange::Visible) {
                m_Material = std::make_shared<CMaterialSingleBand>(
                    matGlass->TransVis, matGlass->TransVis, matGlass->ReflectVisBeamFront, matGlass->ReflectVisBeamBack, m_Range);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCEMaterialDualBandFactory
    ///////////////////////////////////////////////////////////////////////////////
    CWCEMaterialDualBandFactory::CWCEMaterialDualBandFactory(const Material::MaterialBase *t_Material, const WavelengthRange t_Range)
        : CWCEMaterialFactory(t_Material, t_Range)
    {
    }

    void CWCEMaterialDualBandFactory::init([[maybe_unused]] EnergyPlusData &state)
    {
        if (m_Range == WavelengthRange::Visible) {
            m_Material = createVisibleRangeMaterial(state);
        } else {
            auto aVisibleRangeMaterial = createVisibleRangeMaterial(state); // (AUTO_OK_OBJ)
            auto aSolarRangeMaterial = createSolarRangeMaterial(state);     // (AUTO_OK_OBJ)
            // Ratio visible to solar range. It can be calculated from solar spectrum.
            Real64 ratio = 0.49;
            m_Material = std::make_shared<CMaterialDualBand>(aVisibleRangeMaterial, aSolarRangeMaterial, ratio);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCEVenetianBlindMaterialsFactory
    ///////////////////////////////////////////////////////////////////////////////
    CWCEVenetianBlindMaterialsFactory::CWCEVenetianBlindMaterialsFactory(const Material::MaterialBase *t_Material, const WavelengthRange t_Range)
        : CWCEMaterialDualBandFactory(t_Material, t_Range)
    {
    }

    std::shared_ptr<CMaterialSingleBand> CWCEVenetianBlindMaterialsFactory::createVisibleRangeMaterial([[maybe_unused]] EnergyPlusData &state)
    {
        auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(m_MaterialProperties);
        assert(matBlind != nullptr);

        CWavelengthRange aRange(WavelengthRange::Visible);
        Real64 lowLambda = aRange.minLambda();
        Real64 highLambda = aRange.maxLambda();

        Real64 Tf = matBlind->slatTAR.Vis.Ft.Df.Tra;
        Real64 Tb = matBlind->slatTAR.Vis.Ft.Df.Tra;
        Real64 Rf = matBlind->slatTAR.Vis.Ft.Df.Ref;
        Real64 Rb = matBlind->slatTAR.Vis.Bk.Df.Ref;

        return std::make_shared<CMaterialSingleBand>(Tf, Tb, Rf, Rb, lowLambda, highLambda);
    }

    std::shared_ptr<CMaterialSingleBand> CWCEVenetianBlindMaterialsFactory::createSolarRangeMaterial([[maybe_unused]] EnergyPlusData &state)
    {
        auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(m_MaterialProperties);
        assert(matBlind != nullptr);

        CWavelengthRange aRange(WavelengthRange::Solar);
        Real64 lowLambda = aRange.minLambda();
        Real64 highLambda = aRange.maxLambda();

        Real64 Tf = matBlind->slatTAR.Sol.Ft.Df.Tra;
        Real64 Tb = matBlind->slatTAR.Sol.Ft.Df.Tra;
        Real64 Rf = matBlind->slatTAR.Sol.Ft.Df.Ref;
        Real64 Rb = matBlind->slatTAR.Sol.Bk.Df.Ref;

        return std::make_shared<CMaterialSingleBand>(Tf, Tb, Rf, Rb, lowLambda, highLambda);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCEScreenMaterialsFactory
    ///////////////////////////////////////////////////////////////////////////////
    CWCEScreenMaterialsFactory::CWCEScreenMaterialsFactory(const Material::MaterialBase *t_Material, const WavelengthRange t_Range)
        : CWCEMaterialDualBandFactory(t_Material, t_Range)
    {
        // Current EnergyPlus model does not support material transmittance different from zero.
        // To enable that, it would be necessary to change input in IDF
    }

    std::shared_ptr<CMaterialSingleBand> CWCEScreenMaterialsFactory::createVisibleRangeMaterial([[maybe_unused]] EnergyPlusData &state)
    {
        auto const *matShade = dynamic_cast<Material::MaterialShade const *>(m_MaterialProperties);
        assert(matShade != nullptr);
        CWavelengthRange aRange(WavelengthRange::Visible);
        Real64 lowLambda = aRange.minLambda();
        Real64 highLambda = aRange.maxLambda();

        Real64 Tf = 0.0;
        Real64 Tb = 0.0;
        Real64 Rf = matShade->ReflectShadeVis;
        Real64 Rb = matShade->ReflectShadeVis;

        return std::make_shared<CMaterialSingleBand>(Tf, Tb, Rf, Rb, lowLambda, highLambda);
    }

    std::shared_ptr<CMaterialSingleBand> CWCEScreenMaterialsFactory::createSolarRangeMaterial([[maybe_unused]] EnergyPlusData &state)
    {
        auto const *matShade = dynamic_cast<Material::MaterialShade const *>(m_MaterialProperties);
        assert(matShade != nullptr);
        CWavelengthRange aRange(WavelengthRange::Solar);
        Real64 lowLambda = aRange.minLambda();
        Real64 highLambda = aRange.maxLambda();

        Real64 Tf = 0.0;
        Real64 Tb = 0.0;
        Real64 Rf = matShade->ReflectShade;
        Real64 Rb = matShade->ReflectShade;

        return std::make_shared<CMaterialSingleBand>(Tf, Tb, Rf, Rb, lowLambda, highLambda);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCEDiffuseShadeMaterialsFactory
    ///////////////////////////////////////////////////////////////////////////////
    CWCEDiffuseShadeMaterialsFactory::CWCEDiffuseShadeMaterialsFactory(const Material::MaterialBase *t_Material, const WavelengthRange t_Range)
        : CWCEMaterialDualBandFactory(t_Material, t_Range)
    {
    }

    std::shared_ptr<CMaterialSingleBand> CWCEDiffuseShadeMaterialsFactory::createVisibleRangeMaterial([[maybe_unused]] EnergyPlusData &state)
    {
        auto const *matShade = dynamic_cast<Material::MaterialShade const *>(m_MaterialProperties);
        assert(matShade != nullptr);
        CWavelengthRange aRange(WavelengthRange::Visible);
        Real64 lowLambda = aRange.minLambda();
        Real64 highLambda = aRange.maxLambda();

        Real64 Tf = matShade->TransVis;
        Real64 Tb = matShade->TransVis;
        Real64 Rf = matShade->ReflectShadeVis;
        Real64 Rb = matShade->ReflectShadeVis;

        return std::make_shared<CMaterialSingleBand>(Tf, Tb, Rf, Rb, lowLambda, highLambda);
    }

    std::shared_ptr<CMaterialSingleBand> CWCEDiffuseShadeMaterialsFactory::createSolarRangeMaterial([[maybe_unused]] EnergyPlusData &state)
    {
        auto const *matShade = dynamic_cast<Material::MaterialShade const *>(m_MaterialProperties);
        assert(matShade != nullptr);
        CWavelengthRange aRange(WavelengthRange::Solar);
        Real64 lowLambda = aRange.minLambda();
        Real64 highLambda = aRange.maxLambda();

        Real64 Tf = matShade->Trans;
        Real64 Tb = matShade->Trans;
        Real64 Rf = matShade->ReflectShade;
        Real64 Rb = matShade->ReflectShade;

        return std::make_shared<CMaterialSingleBand>(Tf, Tb, Rf, Rb, lowLambda, highLambda);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCECellFactory
    ///////////////////////////////////////////////////////////////////////////////
    IWCECellDescriptionFactory::IWCECellDescriptionFactory(const Material::MaterialBase *t_Material) : m_Material(t_Material)
    {
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCESpecularCellFactory
    ///////////////////////////////////////////////////////////////////////////////

    CWCESpecularCellFactory::CWCESpecularCellFactory(const Material::MaterialBase *t_Material) : IWCECellDescriptionFactory(t_Material)
    {
    }

    std::shared_ptr<ICellDescription> CWCESpecularCellFactory::getCellDescription([[maybe_unused]] EnergyPlusData &state)
    {
        return std::make_shared<CSpecularCellDescription>();
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCEVenetianBlindCellFactory
    ///////////////////////////////////////////////////////////////////////////////
    CWCEVenetianBlindCellFactory::CWCEVenetianBlindCellFactory(const Material::MaterialBase *t_Material) : IWCECellDescriptionFactory(t_Material)
    {
    }

    std::shared_ptr<ICellDescription> CWCEVenetianBlindCellFactory::getCellDescription([[maybe_unused]] EnergyPlusData &state)
    {
        auto *matBlind = dynamic_cast<Material::MaterialBlind const *>(m_Material);
        assert(matBlind != nullptr);

        Real64 slatWidth = matBlind->SlatWidth;
        Real64 slatSpacing = matBlind->SlatSeparation;
        Real64 slatTiltAngle = 90.0 - matBlind->SlatAngle; // Need to convert to WCE system
        Real64 curvatureRadius = 0.0;                      // No curvature radius in current IDF definition
        size_t numOfSlatSegments = 5;                      // Number of segments to use in venetian calculations
        return std::make_shared<CVenetianCellDescription>(slatWidth, slatSpacing, slatTiltAngle, curvatureRadius, numOfSlatSegments);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCEScreenCellFactory
    ///////////////////////////////////////////////////////////////////////////////
    CWCEScreenCellFactory::CWCEScreenCellFactory(const Material::MaterialBase *t_Material) : IWCECellDescriptionFactory(t_Material)
    {
    }

    std::shared_ptr<ICellDescription> CWCEScreenCellFactory::getCellDescription([[maybe_unused]] EnergyPlusData &state)
    {
        Real64 diameter = m_Material->Thickness; // Thickness in this case is diameter
        // ratio is not saved withing material but rather calculated from transmittance
        const Real64 ratio = 1.0 - sqrt(dynamic_cast<Material::MaterialScreen const *>(m_Material)->Trans);
        Real64 spacing = diameter / ratio;
        return std::make_shared<CWovenCellDescription>(diameter, spacing);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCEDiffuseShadeCellFactory
    ///////////////////////////////////////////////////////////////////////////////
    CWCEDiffuseShadeCellFactory::CWCEDiffuseShadeCellFactory(const Material::MaterialBase *t_Material) : IWCECellDescriptionFactory(t_Material)
    {
    }

    std::shared_ptr<ICellDescription> CWCEDiffuseShadeCellFactory::getCellDescription([[maybe_unused]] EnergyPlusData &state)
    {
        return std::make_shared<CFlatCellDescription>();
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCEBSDFLayerFactory
    ///////////////////////////////////////////////////////////////////////////////
    CWCELayerFactory::CWCELayerFactory(const Material::MaterialBase *t_Material, const WavelengthRange t_Range)
        : m_Material(t_Material), m_Range(t_Range), m_BSDFInitialized(false), m_SimpleInitialized(false), m_MaterialFactory(nullptr)
    {
    }

    std::pair<std::shared_ptr<CMaterial>, std::shared_ptr<ICellDescription>> CWCELayerFactory::init(EnergyPlusData &state)
    {
        createMaterialFactory();
        auto aMaterial = m_MaterialFactory->getMaterial(state); // (AUTO_OK_SHARED_PTR)
        assert(aMaterial != nullptr);
        auto aCellDescription = getCellDescription(state); // (AUTO_OK_SHARED_PTR)
        assert(aCellDescription != nullptr);

        return std::make_pair(aMaterial, aCellDescription);
    }

    std::shared_ptr<CBSDFLayer> CWCELayerFactory::getBSDFLayer(EnergyPlusData &state)
    {
        if (!m_BSDFInitialized) {
            auto res = init(state);                                      // (AUTO_OK_SHARED_PTR)
            const auto aBSDF = CBSDFHemisphere::create(BSDFBasis::Full); // (AUTO_OK_OBJ)

            CBSDFLayerMaker aMaker(res.first, aBSDF, res.second);
            m_BSDFLayer = aMaker.getLayer();
            m_BSDFInitialized = true;
        }
        return m_BSDFLayer;
    }

    CScatteringLayer CWCELayerFactory::getLayer(EnergyPlusData &state)
    {
        if (!m_SimpleInitialized) {
            auto res = init(state); // (AUTO_OK_SHARED_PTR)

            m_ScatteringLayer = CScatteringLayer(res.first, res.second);
            m_SimpleInitialized = true;
        }
        return m_ScatteringLayer;
    }

    std::shared_ptr<ICellDescription> CWCELayerFactory::getCellDescription([[maybe_unused]] EnergyPlusData &state) const
    {
        return m_CellFactory->getCellDescription(state);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCESpecularLayerFactory
    ///////////////////////////////////////////////////////////////////////////////
    CWCESpecularLayerFactory::CWCESpecularLayerFactory(const Material::MaterialBase *t_Material, const WavelengthRange t_Range)
        : CWCELayerFactory(t_Material, t_Range)
    {
        m_CellFactory = std::make_shared<CWCESpecularCellFactory>(t_Material);
    }

    void CWCESpecularLayerFactory::createMaterialFactory()
    {
        m_MaterialFactory = std::make_shared<CWCESpecularMaterialsFactory>(m_Material, m_Range);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCEVenetianBlindLayerFactory
    ///////////////////////////////////////////////////////////////////////////////
    CWCEVenetianBlindLayerFactory::CWCEVenetianBlindLayerFactory(const Material::MaterialBase *t_Material, const WavelengthRange t_Range)
        : CWCELayerFactory(t_Material, t_Range)
    {
        m_CellFactory = std::make_shared<CWCEVenetianBlindCellFactory>(t_Material);
    }

    void CWCEVenetianBlindLayerFactory::createMaterialFactory()
    {
        m_MaterialFactory = std::make_shared<CWCEVenetianBlindMaterialsFactory>(m_Material, m_Range);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCEScreenLayerFactory
    ///////////////////////////////////////////////////////////////////////////////
    CWCEScreenLayerFactory::CWCEScreenLayerFactory(const Material::MaterialBase *t_Material, const WavelengthRange t_Range)
        : CWCELayerFactory(t_Material, t_Range)
    {
        m_CellFactory = std::make_shared<CWCEScreenCellFactory>(t_Material);
    }

    void CWCEScreenLayerFactory::createMaterialFactory()
    {
        m_MaterialFactory = std::make_shared<CWCEScreenMaterialsFactory>(m_Material, m_Range);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCEDiffuseShadeLayerFactory
    ///////////////////////////////////////////////////////////////////////////////
    CWCEDiffuseShadeLayerFactory::CWCEDiffuseShadeLayerFactory(const Material::MaterialBase *t_Material, const WavelengthRange t_Range)
        : CWCELayerFactory(t_Material, t_Range)
    {
        m_CellFactory = std::make_shared<CWCEDiffuseShadeCellFactory>(t_Material);
    }

    void CWCEDiffuseShadeLayerFactory::createMaterialFactory()
    {
        m_MaterialFactory = std::make_shared<CWCEDiffuseShadeMaterialsFactory>(m_Material, m_Range);
    }

} // namespace Window
} // namespace EnergyPlus
