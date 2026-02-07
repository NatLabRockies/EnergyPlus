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

#ifndef TARCOGMain_hh_INCLUDED
#define TARCOGMain_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array2A.hh>

// EnergyPlus Headers
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/TARCOGGassesParams.hh>
#include <EnergyPlus/TARCOGParams.hh>

namespace EnergyPlus {

namespace TARCOGMain {

    void TARCOG90(EnergyPlusData &state,
                  int nlayer,                                         // Number of layers (glass + SD)
                  int iwd,                                            // Wind direction:
                  Real64 &tout,                                       // Outdoor temperature [K]
                  Real64 &tind,                                       // Indoor temperature [K]
                  Real64 &trmin,                                      // Indoor mean radiant temperature [K]
                  Real64 wso,                                         // Outdoor wind speed [m/s]
                  Real64 wsi,                                         // Inside forced air speed [m/s]
                  Real64 dir,                                         // Direct solar radiation [W/m2]
                  Real64 outir,                                       // IR radiance of window's exterior surround [W/m2]
                  int isky,                                           // Flag for sky temperature(Tsky) and sky emittance(esky)
                  Real64 tsky,                                        // Night sky temperature [K]
                  Real64 &esky,                                       // Effective night sky emittance
                  Real64 fclr,                                        // Fraction of sky that is clear
                  Real64 VacuumPressure,                              // maximal pressure for gas to be considered as vacuum
                  Real64 &VacuumMaxGapThickness,                      // maximum allowed thickness without producing warning message
                  TARCOGParams::DeflectionCalculation CalcDeflection, // Deflection calculation flag:
                  Real64 Pa,                                          // Atmospheric (outside/inside) pressure (used onlu if CalcDeflection = 1)
                  Real64 Pini,                                        // Initial pressure at time of fabrication (used only if CalcDeflection = 1)
                  Real64 Tini,                                        // Initial temperature at time of fabrication (used only if CalcDeflection = 1)
                  Array1D<Real64> &gap,                               // Vector of gap widths [m]
                  Array1D<Real64> &GapDefMax,                         // Vector of gap widths in deflected state. It will be used as input
                  Array1D<Real64> &thick,                             // Vector of glazing layer thicknesses [m]
                  Array1D<Real64> &scon,                              // Vector of conductivities of each glazing layer  [W/mK]
                  const Array1D<Real64> &YoungsMod,                   // Youngs Modulus coefficients used in deflection calculations
                  const Array1D<Real64> &PoissonsRat,                 // Poissons Ratio coefficients used in deflection calculations
                  const Array1D<Real64> &tir,                         // Vector of IR transmittances of each surface
                  const Array1D<Real64> &emis,                        // Vector of IR emittances of each surface
                  Real64 totsol,                                      // Total solar transmittance of the IGU
                  Real64 tilt,                                        // Window tilt [degrees]
                  const Array1D<Real64> &asol,                        // Vector of Absorbed solar energy fractions for each layer
                  Real64 height,                                      // IGU cavity height
                  Real64 heightt,                                     // Window height
                  Real64 width,                                       // Window width
                  const Array1D<Real64> &presure,                     // Vector of gas pressures in gaps [N/m2]
                  Array2A_int iprop,                                  // Matrix of gas codes - see mgas definition
                  Array2A<Real64> frct,                               // Matrix of mass percentages in gap mixtures
                  Array2A<Real64> xgcon,                              // Matrix of constants for gas conductivity calc
                  Array2A<Real64> xgvis,                              // Matrix of constants for gas dynamic viscosity calc
                  Array2A<Real64> xgcp,                               // Matrix of constants for gas specific heat calc at constant pressure
                  const Array1D<Real64> &xwght,                       // Vector of Molecular weights for gasses
                  const Array1D<Real64> &gama,                        // Vector of specific heat ration for low pressure calc
                  const Array1D_int &nmix,                            // Vector of number of gasses in gas mixture of each gap
                  const Array1D_int &SupportPillar,                   // Shows whether or not gap have support pillar
                  const Array1D<Real64> &PillarSpacing,               // Pillar spacing for each gap (used in case there is support pillar)
                  const Array1D<Real64> &PillarRadius,                // Pillar radius for each gap (used in case there is support pillar)
                  Array1D<Real64> &theta,                             // Vector of average temperatures of glazing surfaces [K]
                  Array1D<Real64> &LayerDef,                          // Vector of layers deflection. [m]
                  Array1D<Real64> &q,                                 // Vector of various heat fluxes [W/m2]
                  Array1D<Real64> &qv,                                // Vector of heat fluxes to each gap by ventillation [W/m2]
                  Real64 &ufactor,                                    // Center of glass U-value [W/m2 K]
                  Real64 &sc,                                         // Shading Coefficient
                  Real64 &hflux,                                      // Net heat flux between room and window [W/m2]
                  Real64 &hcin,                                       // Indoor convective surface heat transfer coefficient  [W/m2 K]
                  Real64 &hcout,                                      // Outdoor convective surface heat transfer coefficient [W/m2 K]
                  Real64 &hrin,                                       // Indoor radiative surface heat transfer coefficient [W/m2 K]
                  Real64 &hrout,                                      // Outdoor radiative surface heat transfer coefficient [W/m2 K]
                  Real64 &hin,                                        // Indoor combined film coefficient (if non-zero) [W/m2K]
                  Real64 &hout,                                       // Outdoor combined film coefficient (if non-zero) [W/m2K]
                  Array1D<Real64> &hcgas,                             // Convective part of gap effective conductivity (including in and out)
                  Array1D<Real64> &hrgas,                             // Radiative part of gap effective conductivity (including in and out)
                  Real64 &shgc,                                       // Solar heat gain coefficient - per ISO 15099
                  int &nperr,                                         // Error code
                  std::string &ErrorMessage,                          // To store error message from tarcog execution
                  Real64 &shgct,                                      // Solar heat gain coefficient - per old procedure
                  Real64 &tamb,                                       // Outdoor environmental temperature [K]
                  Real64 &troom,                                      // Indoor environmental temperature [K]
                  const Array1D_int &ibc,                             // Vector of boundary condition flags (ibc(1) - outdoor, ibc(2) - indoor
                  const Array1D<Real64> &Atop,                        // Vector with areas of top openings - between SD layers and top of
                  const Array1D<Real64> &Abot,                        // Vector with areas of bottom openings - between SD layers and
                  const Array1D<Real64> &Al,                          // Vector with areas of left-hand side openings - between SD layers and
                  const Array1D<Real64> &Ar,                          // Vector of areas of right-hand side openings - between SD layers and
                  const Array1D<Real64> &Ah,                          // Vector of total areas of holes for each SD [m2]
                  const Array1D<Real64> &SlatThick,                   // Thickness of the slat material [m]
                  const Array1D<Real64> &SlatWidth,                   // Slat width [m]
                  const Array1D<Real64> &SlatAngle,                   // Slat tilt angle [deg]
                  const Array1D<Real64> &SlatCond,                    // Conductivity of the slat material [W/m.K]
                  const Array1D<Real64> &SlatSpacing,                 // Distance between slats [m]
                  const Array1D<Real64> &SlatCurve,                   // Curvature radius of the slat [m]
                  const Array1D<Real64> &vvent,                       // Vector of velocities for forced ventilation, for each gap, and for
                  const Array1D<Real64> &tvent,                       // Vector of temperatures of ventilation gas for forced ventilation,
                  const Array1D<TARCOGParams::TARCOGLayerType> &LayerType, // Glazing layer type flag
                  const Array1D_int &nslice,                               // Vector of numbers of slices in a laminated glazing layers
                  const Array1D<Real64> &LaminateA,                        // Left-hand side array for creating slice equations
                  const Array1D<Real64> &LaminateB,                        // Right-hand side array for creating slice equations
                  const Array1D<Real64> &sumsol,                           // Array of absorbed solar energy fractions for each laminated
                  Array1D<Real64> &hg,                                     // Gas conductance of the glazing cavity [W/m2 K]
                  Array1D<Real64> &hr,                                     // Radiation conductance of the glazing cavity [W/m2 K]
                  Array1D<Real64> &hs,                                     // Thermal conductance of the glazing cavity [W/m2 K]
                  Real64 &he,                                  // External heat transfer coefficient [W/m2 K] - EN673 and ISO 10292 procedure
                  Real64 &hi,                                  // Internal heat transfer coefficient [W/m2 K] - EN673 and ISO 10292 procedure
                  Array1D<Real64> &Ra,                         // Vector of Rayleigh numbers, for each gap
                  Array1D<Real64> &Nu,                         // Vector of Nusselt numbers, for each gap
                  TARCOGGassesParams::Stdrd standard,          // Calculation standard switch:
                  TARCOGParams::TARCOGThermalModel ThermalMod, // Thermal model:
                  int Debug_mode,                              // Switch for debug output files:
                  std::string const &Debug_dir,                // Target directory for debug files
                  std::string const &Debug_file,               // File name template for debug files
                  int win_ID,                                  // ID of window (passed by W6)
                  int igu_ID,                                  // ID of the IGU (passed by W6)
                  Real64 &ShadeEmisRatioOut,                   // Ratio of modified to glass emissivity at the outermost glazing surface
                  Real64 &ShadeEmisRatioIn,                    // Ratio of modified to glass emissivity at the innermost glazing surface
                  Real64 &ShadeHcRatioOut,                     // Ratio of modified to unshaded Hc at the outermost glazing surface
                  Real64 &ShadeHcRatioIn,                      // Ratio of modified to unshaded Hc at the innermost glazing surface
                  Real64 &HcUnshadedOut,                       // Hc value at outermost glazing surface of an unshaded subsystem [W/m2 K]
                  Real64 &HcUnshadedIn,                        // Hc value at innermost glazing surface of an unshaded subsystem [W/m2 K]
                  Array1D<Real64> &Keff,                       // Vector of keff values for gaps [W/m.K]
                  Array1D<Real64> &ShadeGapKeffConv,           // Vector of convective keff values for areas above/below
                  Real64 SDScalar,                             // Factor of Venetian SD layer contribution to convection
                  int SHGCCalc,                                // SHGC calculation switch:
                  int &NumOfIterations,                        // Number of iterations for reaching solution
                  Real64 edgeGlCorrFac                         // Edge of glass correction factor
    );
}
struct TARCOGMainData : BaseGlobalStruct
{

    Array1D<Real64> sconTemp = Array1D<Real64>(TARCOGParams::maxlay);
    Array1D<Real64> thickTemp = Array1D<Real64>(TARCOGParams::maxlay);

    // Internally used
    bool converged = false; // used for convergence check in case of deflection calculations
    Array1D<Real64> told = Array1D<Real64>(TARCOGParams::maxlay2);
    Array1D<Real64> CurGap = Array1D<Real64>(TARCOGParams::MaxGap);
    Array1D<Real64> GapDefMean = Array1D<Real64>(TARCOGParams::MaxGap);

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        sconTemp = Array1D<Real64>(TARCOGParams::maxlay);
        thickTemp = Array1D<Real64>(TARCOGParams::maxlay);
        converged = false;
        told = Array1D<Real64>(TARCOGParams::maxlay2);
        CurGap = Array1D<Real64>(TARCOGParams::MaxGap);
        GapDefMean = Array1D<Real64>(TARCOGParams::MaxGap);
    };
};
} // namespace EnergyPlus

#endif
