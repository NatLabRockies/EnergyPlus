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

#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/GroundHeatExchangers/Base.hh>

namespace EnergyPlus {

struct EnergyPlusData;

namespace GroundHeatExchangers {

    struct GLHESlinky : GLHEBase
    {
        static constexpr auto moduleName = "GroundHeatExchanger:Slinky";
        bool verticalConfig = false; // HX Configuration Flag
        Real64 coilDiameter = 0.0;   // Diameter of the slinky coils [m]
        Real64 coilPitch = 0.0;      // Center-to-center slinky coil spacing [m]
        Real64 coilDepth = 0.0;      // Average depth of the coil [m]
        Real64 trenchDepth = 0.0;    // Trench depth from ground surface to trench bottom [m]
        Real64 trenchLength = 0.0;   // Length of single trench [m]
        int numTrenches = 0;         // Number of parallel trenches [m]
        Real64 trenchSpacing = 0.0;  // Spacing between parallel trenches [m]
        int numCoils = 0;            // Number of coils
        int monthOfMinSurfTemp = 0;
        Real64 maxSimYears = 0.0;
        Real64 minSurfTemp = 0.0;
        Array1D<Real64> X0;
        Array1D<Real64> Y0;
        Real64 Z0 = 0.0;

        GLHESlinky() = default;
        GLHESlinky(EnergyPlusData &state, std::string const &objName, nlohmann::json const &j);

        Real64 calcHXResistance(EnergyPlusData &state) override;

        void calcGFunctions(EnergyPlusData &state) override;

        void initGLHESimVars(EnergyPlusData &state) override;

        void getAnnualTimeConstant() override;

        Real64 doubleIntegral(int m, int n, int m1, int n1, Real64 t, int I0, int J0);

        Real64 integral(int m, int n, int m1, int n1, Real64 t, Real64 eta, int J0);

        Real64 distance(int m, int n, int m1, int n1, Real64 eta, Real64 theta);

        Real64 distanceToFictRing(int m, int n, int m1, int n1, Real64 eta, Real64 theta);

        Real64 distToCenter(int m, int n, int m1, int n1);

        Real64 nearFieldResponseFunction(int m, int n, int m1, int n1, Real64 eta, Real64 theta, Real64 t);

        Real64 midFieldResponseFunction(int m, int n, int m1, int n1, Real64 t);

        Real64 getGFunc(Real64 time) override;

        void initEnvironment(EnergyPlusData &state, Real64 CurTime) override;

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;
    };

} // namespace GroundHeatExchangers

} // namespace EnergyPlus
