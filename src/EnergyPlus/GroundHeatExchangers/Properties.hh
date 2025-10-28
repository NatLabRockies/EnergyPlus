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

namespace EnergyPlus {

struct EnergyPlusData;

namespace GroundHeatExchangers {

    static constexpr Real64 hrsPerMonth = 730.0; // Number of hours in month
    static constexpr Real64 maxTSinHr = 60.0;    // Max number of time step in an hour
    static constexpr std::array<std::string_view, 2> GFuncCalcMethodsStrs = {"UHFCALC", "UBHWTCALC"};

    enum class GFuncCalcMethod
    {
        Invalid = -1,
        UniformHeatFlux,
        UniformBoreholeWallTemp,
        Num
    };

    struct ThermophysicalProps // LCOV_EXCL_LINE
    {
        Real64 k = 0.0;           // Thermal conductivity [W/m-K]
        Real64 rho = 0.0;         // Density [kg/m3]
        Real64 cp = 0.0;          // Specific heat [J/kg-K]
        Real64 rhoCp = 0.0;       // Specific heat capacity [J/kg-K]
        Real64 diffusivity = 0.0; // Thermal diffusivity [m2/s]
    };

    struct PipeProps : ThermophysicalProps // LCOV_EXCL_LINE
    {
        Real64 outDia = 0.0;      // Outer diameter of the pipe [m]
        Real64 innerDia = 0.0;    // Inner diameter of the pipe [m]
        Real64 outRadius = 0.0;   // Outer radius of the pipe [m]
        Real64 innerRadius = 0.0; // Inner radius of the pipe [m]
        Real64 thickness = 0.0;   // Thickness of the pipe wall [m]
    };

    struct GLHEVertProps
    {
        static constexpr auto moduleName = "GroundHeatExchanger:Vertical:Properties";
        std::string name;          // Name
        Real64 bhTopDepth = 0.0;   // Depth of top of borehole {m}
        Real64 bhLength = 0.0;     // Length of borehole from top of borehole {m}
        Real64 bhDiameter = 0.0;   // Diameter of borehole {m}
        ThermophysicalProps grout; // Grout properties
        PipeProps pipe;            // Pipe properties
        Real64 bhUTubeDist = 0.0;  // U-tube, shank-to-shank spacing {m}

        GLHEVertProps() = default;
        GLHEVertProps(EnergyPlusData &state, std::string const &objName, nlohmann::json const &j);

        static std::shared_ptr<GLHEVertProps> GetVertProps(EnergyPlusData &state, std::string const &objectName);
    };

} // namespace GroundHeatExchangers

} // namespace EnergyPlus
