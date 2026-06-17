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

#ifndef DataViewFactorInformation_hh_INCLUDED
#define DataViewFactorInformation_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace DataViewFactorInformation {

    struct EnclosureViewFactorInformation
    {
        // Members
        std::string Name;                    // Enclosure name
        std::vector<std::string> spaceNames; // Space names which are part of this enclosure
        std::vector<int> spaceNums;          // Spaces which are part of this enclosure
        int NumOfSurfaces = 0;               // Number of surfaces in the enclosure
        Array2D<Real64> F;                   // View Factors: F(i,j) = from surface j to surface i (counter-intuitive, but optimized for performance)
        Array2D<Real64> ScriptF;             // Hottel's Script F //Tuned Transposed
        Array1D<Real64> Area;                // Surface area
        Array1D<Real64> Emissivity;          // Surface emissivity
        Array1D<Real64> Azimuth;             // Azimuth angle of the surface (in degrees)
        Array1D<Real64> Tilt;                // Tilt angle of the surface (in degrees)
        Array1D<Real64> FMRT;                // Mean Radiant Temperature "View Factor" used in Carroll method
        Array1D<Real64> Fp;                  // F' (Oppenheim surface resistance used in Carroll method)
        Array1D_int SurfacePtr;              // Surface number for surfaces in this enclosure
        std::vector<int> SurfaceReportNums;  // Enclosure surface numbers for reporting view factors in old order (sub-surfaces follow base surfaces)
        Real64 FloorArea = 0.0;              // Floor area of zone(s) in enclosure
        Real64 ExtWindowArea = 0.0;          // Exterior window area
        Real64 TotalSurfArea = 0.0;          // Total surface area
        Array1D<Real64> SolAbsorptance;      // Surface solar absorptance
        int TotalEnclosureDaylRefPoints = 0; // Total number of daylighting reference points in enclosure
        bool HasInterZoneWindow = false;     // Interzone Window(s) present in this zone
        Real64 solVMULT = 0.0;               // 1/(Sum Of Inside Surfaces Area*Absorptance)
        Real64 radQThermalRad = 0.0;         // TOTAL THERMAL RADIATION ADDED TO Radiant Enclosure
        Real64 radThermAbsMult = 0.0;        // EnclRadThermAbsMult  - MULTIPLIER TO COMPUTE 'ITABSF'
        bool solAbsFirstCalc = true;         // for error message
        bool radReCalc = false; // Enclosure solar or thermal radiation properties need recalc due to window/shading status change or envelope EMS
                                // actuators present
        Real64 sumAE = 0.0;     // Sum of surface area * emissivity for all surfaces in the enclosure
        Real64 sumAET = 0.0;    // Sum of surface area * emissivity * temperature for all surfaces in the enclosure
        Real64 MRT = 0.0;       // Mean radiant temperature of the enclosure
        bool reCalcMRT = false; // True when MRT needs to be recalculated
    };

} // namespace DataViewFactorInformation

struct ViewFactorInfoData : BaseGlobalStruct
{

    int NumOfRadiantEnclosures = 0; // Number of radiant enclosures
    int NumOfSolarEnclosures = 0;   // Number of solar enclosures
    EPVector<DataViewFactorInformation::EnclosureViewFactorInformation> EnclRadInfo;
    EPVector<DataViewFactorInformation::EnclosureViewFactorInformation> EnclSolInfo;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        NumOfRadiantEnclosures = 0;
        NumOfSolarEnclosures = 0;
        EnclRadInfo.clear();
        EnclSolInfo.clear();
    }
};

} // namespace EnergyPlus

#endif
