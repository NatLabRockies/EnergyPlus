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

#ifndef BaseGroundTemperatureModel_hh_INCLUDED
#define BaseGroundTemperatureModel_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace GroundTemp {

    enum class ModelType
    {
        Invalid = -1,
        Kusuda,
        FiniteDiff,
        SiteBuildingSurface,
        SiteShallow,
        SiteDeep,
        SiteFCFactorMethod,
        Xing,
        Num
    };

    constexpr std::array<std::string_view, (int)ModelType::Num> modelTypeNamesUC = {"SITE:GROUNDTEMPERATURE:UNDISTURBED:KUSUDAACHENBACH",
                                                                                    "SITE:GROUNDTEMPERATURE:UNDISTURBED:FINITEDIFFERENCE",
                                                                                    "SITE:GROUNDTEMPERATURE:BUILDINGSURFACE",
                                                                                    "SITE:GROUNDTEMPERATURE:SHALLOW",
                                                                                    "SITE:GROUNDTEMPERATURE:DEEP",
                                                                                    "SITE:GROUNDTEMPERATURE:FCFACTORMETHOD",
                                                                                    "SITE:GROUNDTEMPERATURE:UNDISTURBED:XING"};

    constexpr std::array<std::string_view, (int)ModelType::Num> modelTypeNames = {"Site:GroundTemperature:Undisturbed:KusudaAchenbach",
                                                                                  "Site:GroundTemperature:Undisturbed:FiniteDifference",
                                                                                  "Site:GroundTemperature:BuildingSurface",
                                                                                  "Site:GroundTemperature:Shallow",
                                                                                  "Site:GroundTemperature:Deep",
                                                                                  "Site:GroundTemperature:FCfactorMethod",
                                                                                  "Site:GroundTemperature:Undisturbed:Xing"};

    // Base class
    class BaseGroundTempsModel
    {
    public:
        // Public Members
        std::string Name;
        ModelType modelType = ModelType::Invalid;

        BaseGroundTempsModel() = default;
        virtual ~BaseGroundTempsModel() = default;
        BaseGroundTempsModel(const BaseGroundTempsModel &) = delete;
        BaseGroundTempsModel(BaseGroundTempsModel &&) = delete;
        BaseGroundTempsModel &operator=(const BaseGroundTempsModel &) = delete;
        BaseGroundTempsModel &operator=(BaseGroundTempsModel &&) = delete;

        // Virtual method for retrieving the ground temp
        virtual Real64 getGroundTemp(EnergyPlusData &state) = 0;

        virtual Real64 getGroundTempAtTimeInSeconds(EnergyPlusData &state, Real64, Real64) = 0; // parameter names, this isn't K&R C

        virtual Real64 getGroundTempAtTimeInMonths(EnergyPlusData &state, Real64, int) = 0; // parameter names, this isn't K&R C

    protected:
        static void write_ground_temps(InputOutputFile &os, const std::string &name, const Array1D<Real64> &data)
        {
            print<FormatSyntax::FMT>(
                os,
                "! "
                "<Site:GroundTemperature:{}>,Jan{{C}},Feb{{C}},Mar{{C}},Apr{{C}},May{{C}},Jun{{C}},Jul{{C}},Aug{{C}},Sep{{C}},Oct{{"
                "C}},Nov{{C}},Dec{{C}}\n",
                name);
            print<FormatSyntax::FMT>(os, " Site:GroundTemperature:{}, {}\n", name, fmt::format("{:6.2F}", fmt::join(data, ", ")));
        }
    };

    BaseGroundTempsModel *GetGroundTempModelAndInit(EnergyPlusData &state, ModelType modelType, std::string const &name);

} // namespace GroundTemp

struct GroundTemperatureManagerData final : BaseGlobalStruct
{
    // all ground temperature model instances are owned here
    // client component models can get pointers to the instances inside this vector, but they don't own them
    std::vector<GroundTemp::BaseGroundTempsModel *> groundTempModels;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        for (const auto &groundTempModel : groundTempModels) {
            delete groundTempModel;
        }
        new (this) GroundTemperatureManagerData();
    }

    virtual ~GroundTemperatureManagerData() = default;
};

} // namespace EnergyPlus

#endif
