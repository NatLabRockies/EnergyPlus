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

#pragma once

#include <EnergyPlus/GroundHeatExchangers/Base.hh>

namespace EnergyPlus {

struct EnergyPlusData;

namespace GroundHeatExchangers {

    struct GLHEVert : GLHEBase // LCOV_EXCL_LINE
    {

        static constexpr auto moduleName = "GroundHeatExchanger:System";
        Real64 bhDiameter = 0.0;  // Diameter of borehole {m}
        Real64 bhRadius = 0.0;    // Radius of borehole {m}
        Real64 bhLength = 0.0;    // Length of borehole {m}
        Real64 bhUTubeDist = 0.0; // Distance between u-tube legs {m}
        GFuncCalcMethod gFuncCalcMethod = GFuncCalcMethod::Invalid;

        // Parameters for the multipole method
        Real64 theta_1 = 0.0;
        Real64 theta_2 = 0.0;
        Real64 theta_3 = 0.0;
        Real64 sigma = 0.0;

        std::vector<Real64> GFNC_shortTimestep;
        std::vector<Real64> LNTTS_shortTimestep;

        GLHEVert() = default;
        GLHEVert(EnergyPlusData &state, std::string const &objName, nlohmann::json const &j);
        ~GLHEVert() override = default;

        static std::vector<Real64> distances(MyCartesian const &point_i, MyCartesian const &point_j);

        Real64 calcResponse(std::vector<Real64> const &dists, Real64 currTime) const;

        Real64 integral(MyCartesian const &point_i, std::shared_ptr<GLHEVertSingle> const &bh_j, Real64 currTime) const;

        Real64 doubleIntegral(std::shared_ptr<GLHEVertSingle> const &bh_i, std::shared_ptr<GLHEVertSingle> const &bh_j, Real64 currTime) const;

        void calcShortTimestepGFunctions(EnergyPlusData &state);

        void calcLongTimestepGFunctions(EnergyPlusData &state) const;

        void calcGFunctions(EnergyPlusData &state) override;

        void calcUniformHeatFluxGFunctions(EnergyPlusData &state) const;

        void calcUniformBHWallTempGFunctionsWithGHEDesigner(EnergyPlusData &state) const;

        Real64 calcHXResistance(EnergyPlusData &state) override;

        void initGLHESimVars(EnergyPlusData &state) override;

        void getAnnualTimeConstant() override;

        Real64 getGFunc(Real64 time) override;

        Real64 calcBHAverageResistance(EnergyPlusData &state);

        Real64 calcBHTotalInternalResistance(EnergyPlusData &state);

        Real64 calcBHGroutResistance(EnergyPlusData &state);

        Real64 calcPipeConductionResistance() const;

        Real64 calcPipeConvectionResistance(EnergyPlusData &state);

        static Real64 frictionFactor(Real64 reynoldsNum);

        Real64 calcPipeResistance(EnergyPlusData &state);

        void combineShortAndLongTimestepGFunctions() const;

        void initEnvironment(EnergyPlusData &state, [[maybe_unused]] Real64 CurTime) override;

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;

        void setupTimeVectors() const;
    };

} // namespace GroundHeatExchangers

} // namespace EnergyPlus
