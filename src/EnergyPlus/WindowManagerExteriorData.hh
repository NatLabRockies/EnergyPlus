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

#ifndef WindowManagerExteriorData_hh_INCLUDED
#define WindowManagerExteriorData_hh_INCLUDED

// C++ Headers
#include <map>
#include <memory>
#include <vector>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/Vectors.hh>
#include <WCEMultiLayerOptics.hpp>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DataHeatBalance {
    struct MaterialBase;
    struct MaterialChild;
} // namespace DataHeatBalance

} // namespace EnergyPlus

namespace FenestrationCommon {

enum class WavelengthRange;
class CSeries;

} // namespace FenestrationCommon

namespace SpectralAveraging {

class CSpectralSampleData;
}

namespace SingleLayerOptics {

class CBSDFLayer;
class CScatteringLayer;
enum class BSDFDirection;

} // namespace SingleLayerOptics

namespace MultiLayerOptics {

class CMultiPaneBSDF;
class CMultiLayerScattered;

} // namespace MultiLayerOptics

namespace EnergyPlus {

namespace Window {

    // using IGU_BSDFLayers = std::vector< std::shared_ptr< SingleLayerOptics::CBSDFLayer > >;
    using IGU_Layers = std::vector<SingleLayerOptics::CScatteringLayer>;
    // Construction numbers in EnergyPlus are not stored in orders and it can contain wall numbers
    // in between. So we will just use map to store layers so that we get optimized search.
    // using LayersBSDF_Map = std::map< int, std::shared_ptr< IGU_BSDFLayers > >;
    using Layers_Map = std::map<int, IGU_Layers>;

    // Test if surface is hit by beam defined with vector
    bool isSurfaceHit(EnergyPlusData &state, const int t_SurfNum, const DataVectorTypes::Vector &t_Ray);

    // Converts world coordinates (E+) into local surface coordinates that suites better for
    // WCE operations. Return values are angles Theta and Phi that are used to define WCE direction
    std::pair<Real64, Real64> getWCECoordinates(EnergyPlusData &state,
                                                const int t_SurfNum,
                                                const DataVectorTypes::Vector &t_Ray,
                                                const SingleLayerOptics::BSDFDirection t_Direction);

    // Returns Theta and Phi coordinates of surface BSDF for current Sun position
    std::pair<Real64, Real64> getSunWCEAngles(EnergyPlusData &state, const int t_SurfNum, const SingleLayerOptics::BSDFDirection t_Direction);

    ///////////////////////////////////////////////////////////////////////////////
    //   CWCESpecturmProperties
    ///////////////////////////////////////////////////////////////////////////////
    class CWCESpecturmProperties
    {
    public:
        static std::shared_ptr<SpectralAveraging::CSpectralSampleData> getSpectralSample(EnergyPlusData &state, int const t_SampleDataPtr);
        static std::shared_ptr<SpectralAveraging::CSpectralSampleData> getSpectralSample(Material::MaterialGlass const &t_MaterialProperties);
        static FenestrationCommon::CSeries getDefaultSolarRadiationSpectrum(EnergyPlusData &state);
        static FenestrationCommon::CSeries getDefaultVisiblePhotopicResponse(EnergyPlusData &state);
    };

    ///////////////////////////////////////////////////////////////////////////////
    //   CWindowConstructionsSimplified
    ///////////////////////////////////////////////////////////////////////////////
    // Singleton to keep window constructions in simplified (scattering) format.
    class CWindowConstructionsSimplified
    {
    public:
        static CWindowConstructionsSimplified &instance(EnergyPlusData &state);

        void pushLayer(FenestrationCommon::WavelengthRange const t_Range, int t_ConstrNum, const SingleLayerOptics::CScatteringLayer &t_Layer);

        std::shared_ptr<MultiLayerOptics::CMultiLayerScattered>
        getEquivalentLayer(EnergyPlusData &state, FenestrationCommon::WavelengthRange t_Range, int t_ConstrNum);

        static void clearState();
        CWindowConstructionsSimplified();

    private:
        IGU_Layers getLayers(EnergyPlusData &state, FenestrationCommon::WavelengthRange t_Range, int t_ConstrNum) const;

        // Need separate layer properties for Solar and Visible range
        std::map<FenestrationCommon::WavelengthRange, Layers_Map> m_Layers;
        std::map<std::pair<FenestrationCommon::WavelengthRange, int>, std::shared_ptr<MultiLayerOptics::CMultiLayerScattered>> m_Equivalent;
    };

} // namespace Window

struct WindowManagerExteriorData : BaseGlobalStruct
{
    std::unique_ptr<Window::CWindowConstructionsSimplified> p_inst;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) WindowManagerExteriorData();
    }
};

} // namespace EnergyPlus

#endif
