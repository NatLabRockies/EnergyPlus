// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

#ifndef DataHeatBalSurface_hh_INCLUDED
#define DataHeatBalSurface_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace DataHeatBalSurface {

    // Data
    // MODULE PARAMETER DEFINITIONS
    Real64 constexpr MinSurfaceTempLimit(-100.0);            // Lowest inside surface temperature allowed in Celsius
    Real64 constexpr MinSurfaceTempLimitBeforeFatal(-250.0); // 2.5 times MinSurfaceTempLimit
    Real64 constexpr DefaultSurfaceTempLimit(200.0);         // Highest inside surface temperature allowed in Celsius
    Real64 constexpr IterDampConst(5.0);                     // Damping constant for inside surface temperature iterations
    int constexpr ItersReevalConvCoeff(30);                  // Number of iterations between inside convection coefficient reevaluations
    int constexpr MaxIterations(500);                        // Maximum number of iterations allowed for inside surface temps
    Real64 constexpr PoolIsOperatingLimit(0.0001);           // Limit to determine if swimming pool is operating or not
    int constexpr MinEMPDIterations(4);                      // Minimum number of iterations required for EMPD solution
    int constexpr IterationsForCondFDRelaxChange(5);         // number of iterations for inside temps that triggers a change

} // namespace DataHeatBalSurface

struct HeatBalSurfData : BaseGlobalStruct
{

    std::vector<bool> Zone_has_mixed_HT_models; // True if any surfaces in zone use CondFD, HAMT, or Kiva

    // Integer Variables for the Heat Balance Simulation
    Array1D_int SurfCurrNumHist; // From Old Bldctf.inc

    // Surface heat balance limits and convergence parameters
    Real64 MaxSurfaceTempLimit = 200.0;            // Highest inside surface temperature allowed in Celsius
    Real64 MaxSurfaceTempLimitBeforeFatal = 500.0; // 2.5 times MaxSurfaceTempLimit
    int MinIterations = 1;                         // Minimum number of iterations for surface heat balance
    bool InterZoneWindow = false;                  // True if there is an interzone window
    Real64 SumSurfaceHeatEmission = 0.0;           // Heat emission from all surfaces

    // Variables Dimensioned to Max Number of Heat Transfer Surfaces (maxhts)
    Array1D<Real64> SurfCTFConstInPart;  // Constant Inside Portion of the CTF calculation
    Array1D<Real64> SurfCTFConstOutPart; // Constant Outside Portion of the CTF calculation
    // This group of arrays (soon to be vectors) added to facilitate vectorizable loops in CalcHeatBalanceInsideSurf2CTFOnly
    Array1D<Real64> SurfCTFCross0;       // Construct.CTFCross(0)
    Array1D<Real64> SurfCTFInside0;      // Construct.CTFInside(0)
    Array1D<Real64> SurfCTFSourceIn0;    // Construct.CTFSourceIn(0)
    Array1D<Real64> SurfTempOutHist;     // TH(1,1,SurfNum)
    Array1D<Real64> SurfQSourceSinkHist; // SurfQsrcHist(SurfNum, 1)

    Array1D_int SurfIsAdiabatic;     // 0 not adiabatic, 1 is adiabatic
    Array1D_int SurfIsSourceOrSink;  // 0 no internal source/sink, 1 has internal source/sing
    Array1D_int SurfIsOperatingPool; // 0 not pool, 1 is pool
    Array1D<Real64> SurfTempTerm;    // TempTerm for heatbalance equation
    Array1D<Real64> SurfTempDiv;     // Divisor for heatbalance equation
    // end group added to support CalcHeatBalanceInsideSurf2CTFOnly
    Array1D<Real64> SurfTempIn;           // Temperature of the Inside Surface for each heat transfer surface
    Array1D<Real64> SurfTempInsOld;       // SurfTempIn from previous iteration for convergence check
    Array1D<Real64> SurfTempInTmp;        // Inside Surface Temperature Of Each Heat Transfer Surface
    Array1D<Real64> SurfHConvExt;         // Outside Convection Coefficient
    Array1D<Real64> SurfWinCoeffAdjRatio; // Convective Coefficient Adjustment Ratio assuming highly conductive frames
                                          // Only applicable for exterior window surfaces
    Array1D<Real64> SurfHAirExt;          // Outside Radiation Coefficient to Air
    Array1D<Real64> SurfHSkyExt;          // Outside Radiation Coefficient to Sky
    Array1D<Real64> SurfHGrdExt;          // Outside Radiation Coefficient to Ground
    Array1D<Real64> SurfHConvInt;         // INSIDE CONVECTION COEFFICIENT
    Array1D<Real64> SurfTempSource;       // Temperature at the source location for each heat transfer surface
    Array1D<Real64> SurfTempUserLoc;      // Temperature at the user specified location for each heat transfer surface
    Array1D<Real64> SurfTempInMovInsRep;  // Temperature of interior movable insulation on the side facing the zone
    Array1D<Real64> SurfHSrdSurfExt;      // Outside Radiation Coefficient to Surrounding Surfaces

    Array1D<Real64> SurfQConvInRep;        // Surface convection heat gain at inside face [J]
    Array1D<Real64> SurfQdotConvInRep;     // Surface convection heat transfer rate at inside face surface [W] (report)
    Array1D<Real64> SurfQdotConvInPerArea; // Surface conv heat transfer rate per m2 at inside face surf (report){w/m2]

    // these next three all are for net IR thermal radiation exchange with other surfaces in the model.
    Array1D<Real64> SurfQRadNetSurfInRep;    // Surface thermal radiation heat gain at Inside face [J]
    Array1D<Real64> SurfQdotRadNetSurfInRep; // Surface thermal radiation heat transfer inside face surface [W]
    // these next three all are for solar radiation gains on inside face
    Array1D<Real64> SurfQRadSolarInRep;           // Surface thermal radiation heat gain at Inside face [J]
    Array1D<Real64> SurfQdotRadSolarInRep;        // Surface thermal radiation heat transfer inside face surface [W]
    Array1D<Real64> SurfQdotRadSolarInRepPerArea; // [W/m2]Surface thermal radiation heat transfer rate per m2 at Inside face surf
    // these next two all are for Lights visible radiation gains on inside face
    Array1D<Real64> SurfQRadLightsInRep;    // Surface thermal radiation heat gain at Inside face [J]
    Array1D<Real64> SurfQdotRadLightsInRep; // Surface thermal radiation heat transfer inside face surface [W]
    // these next two all are for Internal Gains sources of radiation gains on inside face
    Array1D<Real64> SurfQRadIntGainsInRep;    // Surface thermal radiation heat gain at Inside face [J]
    Array1D<Real64> SurfQdotRadIntGainsInRep; // Surface thermal radiation heat transfer inside face surface [W]
    // these next four all are for Radiative HVAC sources of radiation gains on inside face
    Array1D<Real64> SurfQRadHVACInRep;        // Surface thermal radiation heat gain at Inside face [J]
    Array1D<Real64> SurfQdotRadHVACInRep;     // Surface thermal radiation heat transfer inside face surface [W]
    Array1D<Real64> SurfQdotRadHVACInPerArea; // [W/m2]Surface thermal radiation heat transfer rate per m2 at Inside face surf

    Array1D<Real64> SurfQConvOutReport;     // Surface convection heat gain at Outside face [J]
    Array1D<Real64> SurfQdotConvOutRep;     // Surface convection heat transfer rate at Outside face surface [W]
    Array1D<Real64> SurfQdotConvOutPerArea; // Surface conv heat transfer rate per m2 at Outside face surf (report){w/m2]

    Array1D<Real64> SurfQRadOutReport;        // Surface thermal radiation heat gain at Outside face [J]
    Array1D<Real64> SurfQdotRadOutRep;        // Surface thermal radiation heat transfer outside face surface [W]
    Array1D<Real64> SurfQdotRadOutRepPerArea; // [W/m2]Surface thermal radiation heat transfer rate per m2 at Outside face surf
    Array1D<Real64> SurfQAirExtReport;        // Surface Outside Face Thermal Radiation to Air Heat Transfer Rate [W]
    Array1D<Real64> SurfQHeatEmiReport;       // Surface Outside Face Heat Emission to Air Rate [W]

    Array1D<Real64> SurfOpaqInsFaceCondGainRep; // Opaq Surf Ins Face Cond when Opaq Surf Ins Face Cond >= 0
    Array1D<Real64> SurfOpaqInsFaceCondLossRep; // Opaq Surf Ins Face Cond when Opaq Surf Ins Face Cond  < 0
    Array1D<Real64> SurfOpaqInsFaceCond;        // Opaque surface inside face heat conduction flow (W) from inside of opaque surfaces,
                                                // for reporting (W)
    Array1D<Real64> SurfOpaqInsFaceCondFlux;    // Opaque surface inside face heat conduction flux (W/m2) from inside of opaque surfaces,
                                                // for reporting (W/m2)
    Array1D<Real64> SurfOpaqInsFaceCondEnergy;  // Opaque surface inside face heat conduction flow (J) from inside of opaque surfaces,
                                                // for reporting (J)

    Array1D<Real64> SurfOpaqExtFaceCondGainRep; // Opaq Surf Ext Face Cond when Opaq Surf Ext Face Cond >= 0
    Array1D<Real64> SurfOpaqExtFaceCondLossRep; // Opaq Surf Ext Face Cond when Opaq Surf Ext Face Cond  < 0
    Array1D<Real64> SurfOpaqOutFaceCond;     // Opaque surface outside face heat conduction flow (W) from inside of opaque surfaces, for reporting (W)
    Array1D<Real64> SurfOpaqOutFaceCondFlux; // Opaque surface outside face heat conduct flux (W/m2) from outside of opaque surfaces,
                                             // for reporting (W/m2)
    Array1D<Real64> SurfOpaqOutFaceCondEnergy; // Opaque surface outside face heat conduction flow (J) from inside of opaque surfaces,
                                               // for reporting (J)

    Array1D<Real64> SurfOpaqAvgFaceCondGainRep; // Opaq Surf average Face Cond when Opaq Surf average Face Cond >= 0
    Array1D<Real64> SurfOpaqAvgFaceCondLossRep; // Opaq Surf average Face Cond when Opaq Surf average Face Cond  < 0
    Array1D<Real64> SurfOpaqAvgFaceCond;     // Opaque surface average heat conduction flow (W) net conduction from outside environ toward inside zone
                                             // from inside of opaque surfaces, for reporting (W)
    Array1D<Real64> SurfOpaqAvgFaceCondFlux; // Opaque surface average face heat conduction flux (W/m2) net conduction from outside environ to inside
                                             // zone from inside of opaque surfaces, for reporting (W/m2)
    Array1D<Real64> SurfOpaqAvgFaceCondEnergy; // Opaque surface average heat conduction flow (J) net conduction from outside environ toward inside
                                               // zone from inside of opaque surfaces, for reporting (J)

    Array1D<Real64> SurfOpaqStorageCondGainRep; // Opaque surface stored heat conduction flow when Opaque surface stored heat conduction flow  >= 0
    Array1D<Real64> SurfOpaqStorageCondLossRep; // Opaque surface stored heat conduction flow when Opaque surface stored heat conduction flow   < 0
    Array1D<Real64> SurfOpaqStorageCond;        // Opaque surface stored heat conduction flow (W) storage of heat inside surface,
                                                // positive is increasing in surf
    Array1D<Real64> SurfOpaqStorageCondFlux;    // Opaque surface stored heat conduction flux (W/m2) storage of heat inside surface,
                                                // positive is increasing in surf
    Array1D<Real64> SurfOpaqStorageCondEnergy;  // Opaque surface stored heat conduction flow (J) storage of heat inside surface,
                                                // positive is increasing in surf

    Array1D<Real64> SurfOpaqInsFaceBeamSolAbsorbed; // Opaque surface inside face absorbed beam solar, for reporting (W)
    Array1D<Real64> SurfTempOut; // Temperature of the Outside Surface for each heat transfer surface used for reporting purposes only. Ref: TH(x,1,1)
    Array1D<Real64> SurfQRadSWOutMvIns; // Short wave radiation absorbed on outside of movable insulation

    Array1D<Real64> SurfQdotRadNetLWInPerArea;  // Net interior long wavelength radiation to a surface from other surfaces
    Array1D<Real64> SurfQdotRadLightsInPerArea; // Short wave from Lights radiation absorbed on inside of opaque surface
    // Variables that are used in both the Surface Heat Balance and the Moisture Balance
    Array1D<Real64> SurfOpaqQRadSWOutAbs;  // Short wave radiation absorbed on outside of opaque surface
    Array1D<Real64> SurfOpaqQRadSWInAbs;   // Short wave radiation absorbed on inside of opaque surface
    Array1D<Real64> SurfQRadLWOutSrdSurfs; // Long wave radiation absorbed on outside of exterior surface

    Array1D<Real64> SurfQAdditionalHeatSourceOutside; // Additional heat source term on boundary conditions at outside surface
    Array1D<Real64> SurfQAdditionalHeatSourceInside;  // Additional heat source term on boundary conditions at inside surface

    Array1D<Real64> SurfOpaqInitialDifSolInAbs;   // Initial diffuse solar absorbed on inside of opaque surface [W/m2]
    Array1D<Real64> SurfWinInitialDifSolInTrans;  // Initial diffuse solar transmitted out through window inside face [W/m2]
    Array1D<Real64> SurfWinInitialBeamSolInTrans; // Interior beam solar transmitted out through window inside face [W]

    // REAL(r64) variables from BLDCTF.inc and only used in the Heat Balance
    // Hist Term (1 = Current Time, 2-MaxCTFTerms = previous times)
    Array1D<Array1D<Real64>> SurfInsideTempHist;  // Temperature history - inside (Hist Term, SurfNum)
    Array1D<Array1D<Real64>> SurfOutsideTempHist; // Temperature history - outside (Hist Term, SurfNum)
    Array1D<Array1D<Real64>>
        SurfInsideTempHistMaster; // Master temperature history (on the time step for the construct) - inside (Hist Term, SurfNum)
    Array1D<Array1D<Real64>>
        SurfOutsideTempHistMaster;                // Master temperature history (on the time step for the construct) - outside (Hist Term, SurfNum)
    Array1D<Array1D<Real64>> SurfInsideFluxHist;  // Flux history - inside (Hist Term, SurfNum)
    Array1D<Array1D<Real64>> SurfOutsideFluxHist; // Flux history - outside (Hist Term, SurfNum)
    Array1D<Array1D<Real64>> SurfInsideFluxHistMaster;  // Master flux history (on the time step for the construct) - inside (Hist Term, SurfNum)
    Array1D<Array1D<Real64>> SurfOutsideFluxHistMaster; // Master flux history (on the time step for the construct) - outside (Hist Term, SurfNum)

    Array2D<Real64> SurfTsrcHist;   // Temperature history at the source location (SurfNum,Term)
    Array2D<Real64> SurfTuserHist;  // Temperature history at the user specified location (SurfNum,Term)
    Array2D<Real64> SurfQsrcHist;   // Heat source/sink history for the surface (SurfNum,Term)
    Array2D<Real64> SurfTsrcHistM;  // Master temperature history at the source location (SurfNum,Term)
    Array2D<Real64> SurfTuserHistM; // Master temperature history at the user specified location (SurfNum,Term)
    Array2D<Real64> SurfQsrcHistM;  // Master heat source/sink history for the surface (SurfNum,Term)

    Array2D<Real64> ZoneFractDifShortZtoZ; // Fraction of diffuse short radiation in Zone 2 transmitted to Zone 1
    Array1D_bool EnclSolRecDifShortFromZ;  // True if Zone gets short radiation from another

    // Surface Heat Balance
    Array1D<Real64> SurfAbsSolarExt;                      // Solar Absorptivity of surface outside face or interior movable insulation if present
    Array1D<Real64> SurfAbsThermalExt;                    // Thermal Absorptivity of surface outside face or interior movable insulation if present
    Array1D<Real64> SurfAbsSolarInt;                      // Solar absorptivity of surface inside face or exterior movable insulation if present
    Array1D<Material::SurfaceRoughness> SurfRoughnessExt; // Roughness of surface inside face or exterior movable insulation if present
    Array1D<Real64> SurfAbsThermalInt;                    // Thermal absorptivity of surface inside face or exterior movable insulation if present

    std::vector<int> SurfMovSlatsIndexList;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) HeatBalSurfData();
    }
};

} // namespace EnergyPlus

#endif
