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

#ifndef DataBranchAirLoopPlant_hh_INCLUDED
#define DataBranchAirLoopPlant_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace DataBranchAirLoopPlant {

    // Parameters for tolerance
    constexpr Real64 MassFlowTolerance(0.000000001); // minimum significant mass flow rate (kg/s)

    // Pressure Curve Type: None, pressure, or generic curve (if generic it will be a positive value which is the curve manager index)
    enum class PressureCurveType
    {
        Invalid = -1,
        None,
        Pressure,
        Generic,
        Num
    };

    // Parameters for flow Control Types for branch flow resolution inside splitter/mixers
    enum class ControlType
    {
        Invalid = -1,
        Active,
        Passive,
        SeriesActive,
        Bypass,
        Num
    };

    // Types

    struct PlantPressureCurveData
    {
        // Members
        std::string Name;
        Real64 EquivDiameter = 0.0;         // - An effective diameter for calculation of Re & e/D [m]
        Real64 MinorLossCoeff = 0.0;        // - K factor                                          [-]
        Real64 EquivLength = 0.0;           // - An effective length to apply friction calculation [m]
        Real64 EquivRoughness = 0.0;        // - An effective roughness (e) to calculate e/D       [m]
        bool ConstantFPresent = false;      // - Signal for if a constant value of f was entered
        Real64 ConstantF = 0.0;             // - Constant value of f (if applicable)               [-]
        bool EMSOverrideOn = false;         // if TRUE, then EMS is calling to override curve value
        Real64 EMSOverrideCurveValue = 0.0; // Value of curve result EMS is directing to use

        //  report variables.
        Real64 CurveOutput = 0.0;
        Real64 CurveInput1 = 0.0; // - MassFlow                                         [kg/s]
        Real64 CurveInput2 = 0.0; // - Density                                          [kg/m3]
        Real64 CurveInput3 = 0.0; // - Velocity                                         [m/s]
    };
} // namespace DataBranchAirLoopPlant

struct DataBranchAirLoopPlantData : BaseGlobalStruct
{
    Array1D<DataBranchAirLoopPlant::PlantPressureCurveData> PressureCurve;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) DataBranchAirLoopPlantData();
    }
};

} // namespace EnergyPlus

#endif
