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

#ifndef DataDaylightingDevices_hh_INCLUDED
#define DataDaylightingDevices_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace Dayltg {

    constexpr int MaxTZones(10);   // Maximum number of transition zones
    constexpr int NumOfAngles(19); // Number of data points on transmittance vs. angle curve

    enum class RadType
    {
        Invalid = -1,
        VisibleBeam,
        SolarBeam,
        SolarAniso,
        SolarIso,
        Num
    };

    struct TDDPipeData
    {
        // Members
        // Input variables
        std::string Name;            // Name of TDD pipe
        int Dome = 0;                // Pointer to the dome object
        int Diffuser = 0;            // Pointer to the diffuser object
        int Construction = 0;        // Pointer to the construction object
        Real64 Diameter = 0.0;       // Pipe diameter
        Real64 TotLength = 0.0;      // Total length of pipe, including exterior
        Real64 Reff = 0.0;           // Effective R value between TDD:DOME and TDD:DIFFUSER
        int NumOfTZones = 0;         // Number of transition zone
        Array1D_int TZone;           // Pointers to transition zones
        Array1D<Real64> TZoneLength; // Length of pipe in each transition zone
        // Calculated variables
        Real64 AspectRatio = 0.0;         // Aspect ratio, length / diameter
        Real64 ReflectVis = 0.0;          // Visible reflectance of surface
        Real64 ReflectSol = 0.0;          // Solar reflectance of surface
        Array1D<Real64> PipeTransVisBeam; // Table of beam visible transmittance vs. cosine angle
        Array1D<Real64> PipeTransSolBeam; // Table of beam solar transmittance vs. cosine angle
        Real64 TransSolIso = 0.0;         // Diffuse isotropic solar transmittance (constant)
        Real64 TransSolHorizon = 0.0;     // Diffuse horizon solar transmittance (constant)
        Real64 ExtLength = 0.0;           // Exterior exposed length of pipe
        Array1D<Real64> TZoneHeatGain;    // convection gain to transition zones
        // Report variables
        Real64 TransmittedSolar = 0.0;  // Solar transmitted by the TDD [W]
        Real64 PipeAbsorbedSolar = 0.0; // Solar absorbed in the walls of the pipe [W]
        Real64 HeatGain = 0.0;          // Solar heat gain [W]
        Real64 HeatLoss = 0.0;          // Solar heat loss [W]
        Real64 TransVisBeam = 0.0;      // TDD visible transmittance
        Real64 TransSolBeam = 0.0;      // TDD beam solar transmittance
        Real64 TransVisDiff = 0.0;      // TDD diffuse visible transmittance
        Real64 TransSolDiff = 0.0;      // TDD diffuse solar transmittance

        // Default Constructor
        TDDPipeData() : PipeTransVisBeam(Dayltg::NumOfAngles, 0.0), PipeTransSolBeam(Dayltg::NumOfAngles, 0.0)
        {
        }
    };

    struct ShelfData
    {
        // Members
        // Input variables
        std::string Name;     // Name of daylighting shelf
        int Window = 0;       // Pointer to the window object
        int InSurf = 0;       // Pointer to the inside shelf heat transfer surface
        int OutSurf = 0;      // Pointer to the outside shelf attached shading surface
        int Construction = 0; // Pointer to the outside shelf construction object
        // Calculated variables
        Real64 OutReflectVis = 0.0; // Outside shelf visible reflectance
        Real64 OutReflectSol = 0.0; // Outside shelf solar reflectance
        Real64 ViewFactor = 0.0;    // Outside shelf view factor to window
        // Report variables
    };

} // namespace Dayltg

struct DataDaylightingDevicesData : BaseGlobalStruct
{
    Array1D<Dayltg::TDDPipeData> TDDPipe;
    Array1D<Dayltg::ShelfData> Shelf;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) DataDaylightingDevicesData();
    }
};

} // namespace EnergyPlus

#endif
