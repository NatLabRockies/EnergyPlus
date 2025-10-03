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

#ifndef DataMoistureBalance_hh_INCLUDED
#define DataMoistureBalance_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Array3D.hh>
#include <ObjexxFCL/Array4D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

struct MoistureBalanceData : BaseGlobalStruct
{
    // This is more or less the traditional value from BLAST.
    // static constexpr Real64 Lam = 2500000.0; // heat of adsorption for building materials
    // Public Variables that will also be used in the Moisture Surface Balance
    Array3D<Real64> FluxH; // transfer function coeff for calculating the CPF Flux history term
    Array4D<Real64> Icoef; // transfer function coeff for calculating the CPF history term
    Array2D<Real64> DiffC; // Thermal Diffusivity in combined potential formulation (CPF)
    // for each equation
    Array2D<Real64> mtinc; // # of Moisture transfer function time increment for each equation
    Array1D<Real64> S1;    // Thermal Diffusivity in combined potential formulation (CPF)
    // for each equation
    Array1D<Real64> R2; // Thermal Diffusivity in combined potential formulation (CPF)
    // for each equation
    Array1D<Real64> TempOutsideAirFD; // Temperature outside air for the FD surface

    Array2D_int mhstry; // # of FD History terms for each equation
    Array1D_int CMTF;   // Type of material layer
    Array2D_int Nmrf;   // # of Moisture Response Factors for CPF Solution

    // variables used for MTF moisture implementation
    Array1D<Real64> RhoVaporAirOut; // Vapor Density outside surface
    Array1D<Real64> RhoVaporAirIn;  // Vapor Density inside surface
    Array1D<Real64> HConvExtFD;     // thermal convection coefficient outside surface
    Array1D<Real64> HMassConvExtFD; // mass convection coefficient outside surface
    Array1D<Real64> HConvInFD;      // thermal convection coefficient inside surface
    Array1D<Real64> HMassConvInFD;  // mass convection coefficient inside surface
    Array1D<Real64> RhoVaporSurfIn; // Vapor Density inside surface
    Array1D<Real64> HSkyFD;         // Sky Convection Coefficient
    Array1D<Real64> HGrndFD;        // Ground Convection Coefficient
    Array1D<Real64> HAirFD;         // Air Convection Coefficient
    Array1D<Real64> HSurrFD;        // Surrounding Surfaces LWR Exchange Coefficient

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) MoistureBalanceData();
    }
};

} // namespace EnergyPlus

#endif
