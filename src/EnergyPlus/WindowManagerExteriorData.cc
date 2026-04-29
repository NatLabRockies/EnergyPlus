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

#include <cassert>

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBSDFWindow.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WindowComplexManager.hh>
#include <EnergyPlus/WindowManager.hh>
#include <EnergyPlus/WindowManagerExteriorData.hh>

#include <WCEMultiLayerOptics.hpp>

namespace EnergyPlus {

using namespace DataSurfaces;
using namespace DataHeatBalance;
using namespace WindowComplexManager;

using namespace SingleLayerOptics;
using namespace FenestrationCommon;
using namespace SpectralAveraging;
using namespace MultiLayerOptics;

namespace Window {

    bool isSurfaceHit(EnergyPlusData &state, const int t_SurfNum, const Vector &t_Ray)
    {
        Real64 DotProd = dot(t_Ray, state.dataSurface->Surface(t_SurfNum).NewellSurfaceNormalVector);
        return (DotProd > 0);
    }

    std::pair<Real64, Real64> getWCECoordinates(EnergyPlusData &state, int const t_SurfNum, Vector const &t_Ray, const BSDFDirection t_Direction)
    {
        Real64 Theta = 0;
        Real64 Phi = 0;

        // get window tilt and azimuth
        Real64 Gamma = Constant::DegToRad * state.dataSurface->Surface(t_SurfNum).Tilt;
        Real64 Alpha = Constant::DegToRad * state.dataSurface->Surface(t_SurfNum).Azimuth;

        RayIdentificationType RadType = RayIdentificationType::Front_Incident;

        if (t_Direction == BSDFDirection::Outgoing) {
            RadType = RayIdentificationType::Back_Incident;
        }

        // get the corresponding local Theta, Phi for ray
        W6CoordsFromWorldVect(state, t_Ray, RadType, Gamma, Alpha, Theta, Phi);

        Theta = 180 / Constant::Pi * Theta;
        Phi = 180 / Constant::Pi * Phi;

        return std::make_pair(Theta, Phi);
    }

    std::pair<Real64, Real64> getSunWCEAngles(EnergyPlusData &state, const int t_SurfNum, const BSDFDirection t_Direction)
    {
        return getWCECoordinates(
            state, t_SurfNum, state.dataBSDFWindow->SUNCOSTS(state.dataGlobal->TimeStep, state.dataGlobal->HourOfDay), t_Direction);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //       CWCESpecturmProperties
    ///////////////////////////////////////////////////////////////////////////////
    CSeries CWCESpecturmProperties::getDefaultSolarRadiationSpectrum(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   September 2016
        //       MODIFIED       na
        //       RE-ENGINEERED
        //          April 2021: returning CSeries instead of pointer to CSeries

        // PURPOSE OF THIS SUBROUTINE:
        // Handles solar radiation spetrum from default location or IDF
        CSeries solarRadiation;

        for (int i = 1; i <= nume; ++i) {
            solarRadiation.addProperty(state.dataWindowManager->wle[i - 1], state.dataWindowManager->e[i - 1]);
        }

        return solarRadiation;
    }

    ///////////////////////////////////////////////////////////////////////////////
    CSeries CWCESpecturmProperties::getDefaultVisiblePhotopicResponse(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   September 2016
        //       MODIFIED       na
        //       RE-ENGINEERED
        //          April 2021: Function now returns CSeries instead of pointer to CSeries

        // PURPOSE OF THIS SUBROUTINE:
        // Handles solar radiation spetrum from default location or IDF
        CSeries visibleResponse;

        for (int i = 1; i <= numt3; ++i) {
            visibleResponse.addProperty(state.dataWindowManager->wlt3[i - 1], state.dataWindowManager->y30[i - 1]);
        }

        return visibleResponse;
    }

    ///////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<CSpectralSampleData> CWCESpecturmProperties::getSpectralSample(EnergyPlusData &state, int const t_SampleDataPtr)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Simon Vidanovic
        //       DATE WRITTEN   September 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Reads spectral data value

        auto &s_mat = state.dataMaterial;

        assert(t_SampleDataPtr != 0); // It must not be called for zero value
        std::shared_ptr<CSpectralSampleData> aSampleData = std::make_shared<CSpectralSampleData>();
        auto spectralData = s_mat->SpectralData(t_SampleDataPtr); // (AUTO_OK_OBJ)
        int numOfWl = spectralData.NumOfWavelengths;
        for (int i = 1; i <= numOfWl; ++i) {
            Real64 wl = spectralData.WaveLength(i);
            Real64 T = spectralData.Trans(i);
            Real64 Rf = spectralData.ReflFront(i);
            Real64 Rb = spectralData.ReflBack(i);
            aSampleData->addRecord(wl, T, Rf, Rb);
        }

        return aSampleData;
    }

    ///////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<CSpectralSampleData> CWCESpecturmProperties::getSpectralSample(Material::MaterialGlass const &t_MaterialProperties)
    {
        Real64 Tsol = t_MaterialProperties.Trans;
        Real64 Rfsol = t_MaterialProperties.ReflectSolBeamFront;
        Real64 Rbsol = t_MaterialProperties.ReflectSolBeamBack;
        std::shared_ptr<CMaterial> aSolMat = std::make_shared<CMaterialSingleBand>(Tsol, Tsol, Rfsol, Rbsol, 0.3, 2.5);

        Real64 Tvis = t_MaterialProperties.TransVis;
        Real64 Rfvis = t_MaterialProperties.ReflectVisBeamFront;
        Real64 Rbvis = t_MaterialProperties.ReflectVisBeamBack;
        std::shared_ptr<CMaterial> aVisMat = std::make_shared<CMaterialSingleBand>(Tvis, Tvis, Rfvis, Rbvis, 0.38, 0.78);

        CMaterialDualBand aMat = CMaterialDualBand(aVisMat, aSolMat, 0.49);
        std::vector<Real64> aWl = aMat.getBandWavelengths();
        std::vector<Real64> aTf = aMat.getBandProperties(Property::T, Side::Front);
        std::vector<Real64> aRf = aMat.getBandProperties(Property::R, Side::Front);
        std::vector<Real64> aRb = aMat.getBandProperties(Property::R, Side::Back);
        std::shared_ptr<CSpectralSampleData> aSampleData = std::make_shared<CSpectralSampleData>();
        for (size_t i = 0; i < aWl.size(); ++i) {
            aSampleData->addRecord(aWl[i], aTf[i], aRf[i], aRb[i]);
        }

        return aSampleData;
    }

    CWindowConstructionsSimplified &CWindowConstructionsSimplified::instance(EnergyPlusData &state)
    {
        if (state.dataWindowManagerExterior->p_inst == nullptr) {
            state.dataWindowManagerExterior->p_inst = std::unique_ptr<CWindowConstructionsSimplified>(new CWindowConstructionsSimplified());
        }
        return *state.dataWindowManagerExterior->p_inst;
    }

    CWindowConstructionsSimplified::CWindowConstructionsSimplified()
    {
        m_Layers[WavelengthRange::Solar] = Layers_Map();
        m_Layers[WavelengthRange::Visible] = Layers_Map();
    }

    void CWindowConstructionsSimplified::pushLayer(WavelengthRange const t_Range, int const t_ConstrNum, const CScatteringLayer &t_Layer)
    {
        Layers_Map &aMap = m_Layers.at(t_Range);
        const auto it = aMap.find(t_ConstrNum);
        if (it == aMap.end()) {
            aMap[t_ConstrNum] = IGU_Layers();
        }
        aMap.at(t_ConstrNum).push_back(t_Layer);
    }

    std::shared_ptr<CMultiLayerScattered>
    CWindowConstructionsSimplified::getEquivalentLayer(EnergyPlusData &state, WavelengthRange const t_Range, int const t_ConstrNum)
    {
        auto it = m_Equivalent.find(std::make_pair(t_Range, t_ConstrNum));
        if (it == m_Equivalent.end()) {
            // Layer was not requested before. Need to create it now.
            // shared_ptr< vector< double > > commonWl = getCommonWavelengths( t_Range, t_ConstrNum );
            IGU_Layers iguLayers = getLayers(state, t_Range, t_ConstrNum);
            std::shared_ptr<CMultiLayerScattered> aEqLayer = std::make_shared<CMultiLayerScattered>(iguLayers[0]);
            for (size_t i = 1u; i < iguLayers.size(); ++i) {
                aEqLayer->addLayer(iguLayers[i]);
            }

            auto aSolarSpectrum = CWCESpecturmProperties::getDefaultSolarRadiationSpectrum(state); // (AUTO_OK_OBJ)
            aEqLayer->setSourceData(aSolarSpectrum);
            m_Equivalent[std::make_pair(t_Range, t_ConstrNum)] = aEqLayer;
        }

        return m_Equivalent.at(std::make_pair(t_Range, t_ConstrNum));
    }

    void CWindowConstructionsSimplified::clearState()
    {
    }

    IGU_Layers CWindowConstructionsSimplified::getLayers(EnergyPlusData &state, WavelengthRange const t_Range, int const t_ConstrNum) const
    {
        Layers_Map aMap = m_Layers.at(t_Range); // Do you mean to make a copy of the entire map here?
        auto it = aMap.find(t_ConstrNum);
        if (it == aMap.end()) {
            ShowFatalError(state, "Incorrect construction selection.");
            // throw std::runtime_error("Incorrect construction selection.");
        }
        return it->second;
    }

} // namespace Window

} // namespace EnergyPlus
