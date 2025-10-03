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

#include <nlohmann/json.hpp>

#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/GroundHeatExchangers/Properties.hh>

namespace EnergyPlus {

struct EnergyPlusData;

namespace GroundHeatExchangers {

    struct MyCartesian
    {
        Real64 x = 0.0;
        Real64 y = 0.0;
        Real64 z = 0.0;
    };

    struct GLHEVertSingle
    {
        static constexpr auto moduleName = "GroundHeatExchanger:Vertical:Single";
        std::string name;                     // Name
        Real64 xLoc = 0.0;                    // X-direction location {m}
        Real64 yLoc = 0.0;                    // Y-direction location {m}
        Real64 dl_i = 0.0;                    // Discretized bh length between points
        Real64 dl_ii = 0.0;                   // Discretized bh length between points
        Real64 dl_j = 0.0;                    // Discretized bh length between points
        std::shared_ptr<GLHEVertProps> props; // Properties
        std::vector<MyCartesian>
            pointLocations_i; // Discretized point locations for when computing temperature response of other boreholes on this bh
        std::vector<MyCartesian> pointLocations_ii; // Discretized point locations for when computing temperature response of this bh on itself
        std::vector<MyCartesian>
            pointLocations_j; // Discretized point locations for when other bh are computing the temperature response of this bh on themselves

        GLHEVertSingle() = default;
        GLHEVertSingle(EnergyPlusData &state, std::string const &objName, nlohmann::json const &j);
        static std::shared_ptr<GLHEVertSingle> GetSingleBH(EnergyPlusData &state, std::string const &objectName);
    };

} // namespace GroundHeatExchangers

} // namespace EnergyPlus
