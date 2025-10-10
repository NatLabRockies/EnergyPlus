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

#ifndef FluidProperties_hh_INCLUDED
#define FluidProperties_hh_INCLUDED

// C++ Headers
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Array2S.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Psychrometrics.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Fluid {

    constexpr int GlycolNum_Water = 1;
    constexpr int RefrigNum_Steam = 1;

#undef PERFORMANCE_OPT

#ifdef EP_cache_GlycolSpecificHeat
    int constexpr t_sh_cache_size = 1024 * 1024;
    int constexpr t_sh_precision_bits = 24;
    std::uint64_t constexpr t_sh_cache_mask = (t_sh_cache_size - 1);
#endif

    enum class RefrigError
    {
        Invalid = -1,
        SatTemp,
        SatPress,
        SatTempDensity,
        SatSupEnthalpy,
        SatSupEnthalpyTemp,
        SatSupEnthalpyPress,
        SatSupPress,
        SatSupPressTemp,
        SatSupPressEnthalpy,
        SatSupDensity,
        SatSupDensityTemp,
        SatSupDensityPress,
        Num
    };

    struct RefrigProps
    {
        // Members
        std::string Name; // Name of the refrigerant
        int Num = 0;
        bool used = false;

        std::string satTempArrayName; // Array of saturated temperature points, must be same for all properties
        std::string supTempArrayName; // Array of superheated temperature points, must be same for all properties

        int NumPsPoints = 0;          // Number of saturation pressure
        Real64 PsLowTempValue = 0.0;  // Low Temperature Value for Ps (>0.0)
        Real64 PsHighTempValue = 0.0; // High Temperature Value for Ps (max in tables)
        int PsLowTempIndex = 0;       // Low Temperature Min Index for Ps (>0.0)
        int PsHighTempIndex = 0;      // High Temperature Max Index for Ps (>0.0)
        Real64 PsLowPresValue = 0.0;  // Low Pressure Value for Ps (>0.0)
        Real64 PsHighPresValue = 0.0; // High Pressure Value for Ps (max in tables)
        int PsLowPresIndex = 0;       // Low Pressure Min Index for Ps (>0.0)
        int PsHighPresIndex = 0;      // High Pressure Max Index for Ps (>0.0)
        Array1D<Real64> PsTemps;      // Temperatures for saturation pressures
        Array1D<Real64> PsValues;     // Saturation pressures at PsTemps
#ifdef PERFORMANCE_OPT
        Array1D<Real64> PsTempRatios; // PsTempRatios(i) = (PsValues(i+1) - PsValues(i)) / (PsTemps(i+1) - PsTemps(i)).  Speed optimization.
#endif                                // PERFORMANCE_OPT

        int NumHPoints = 0;            // Number of enthalpy points
        Real64 HfLowTempValue = 0.0;   // Low Temperature Value for Hf (>0.0)
        Real64 HfHighTempValue = 0.0;  // High Temperature Value for Hf (max in tables)
        int HfLowTempIndex = 0;        // Low Temperature Min Index for Hf (>0.0)
        int HfHighTempIndex = 0;       // High Temperature Max Index for Hf (>0.0)
        Real64 HfgLowTempValue = 0.0;  // Low Temperature Value for Hfg (>0.0)
        Real64 HfgHighTempValue = 0.0; // High Temperature Value for Hfg (max in tables)
        int HfgLowTempIndex = 0;       // Low Temperature Min Index for Hfg (>0.0)
        int HfgHighTempIndex = 0;      // High Temperature Max Index for Hfg (>0.0)
        Array1D<Real64> HTemps;        // Temperatures for enthalpy points
        Array1D<Real64> HfValues;      // Enthalpy of saturated fluid at HTemps
        Array1D<Real64> HfgValues;     // Enthalpy of saturated fluid/gas at HTemps
#ifdef PERFORMANCE_OPT
        Array1D<Real64> HfTempRatios;
        Array1D<Real64> HfgTempRatios;
#endif // PERFORMANCE_OPT

        int NumCpPoints = 0;            // Number of specific heat of fluid points
        Real64 CpfLowTempValue = 0.0;   // Low Temperature Value for Cpf (>0.0)
        Real64 CpfHighTempValue = 0.0;  // High Temperature Value for Cpf (max in tables)
        int CpfLowTempIndex = 0;        // Low Temperature Min Index for Cpf (>0.0)
        int CpfHighTempIndex = 0;       // High Temperature Max Index for Cpf (>0.0)
        Real64 CpfgLowTempValue = 0.0;  // Low Temperature Value for Cpfg (>0.0)
        Real64 CpfgHighTempValue = 0.0; // High Temperature Value for Cpfg (max in tables)
        int CpfgLowTempIndex = 0;       // Low Temperature Min Index for Cpfg (>0.0)
        int CpfgHighTempIndex = 0;      // High Temperature Max Index for Cpfg (>0.0)
        Array1D<Real64> CpTemps;        // Temperatures for specific heat points
        Array1D<Real64> CpfValues;      // Specific heat of saturated fluid at CpTemps
        Array1D<Real64> CpfgValues;     // Specific heat of saturated fluid/gas at CpTemps
#ifdef PERFORMANCE_OPT
        Array1D<Real64> CpfTempRatios;
        Array1D<Real64> CpfgTempRatios;
#endif // PERFORMANCE_OPT

        int NumRhoPoints = 0;            // Number of density of fluid points
        Real64 RhofLowTempValue = 0.0;   // Low Temperature Value for Rhof (>0.0)
        Real64 RhofHighTempValue = 0.0;  // High Temperature Value for Rhof (max in tables)
        int RhofLowTempIndex = 0;        // Low Temperature Min Index for Rhof (>0.0)
        int RhofHighTempIndex = 0;       // High Temperature Max Index for Rhof (>0.0)
        Real64 RhofgLowTempValue = 0.0;  // Low Temperature Value for Rhofg (>0.0)
        Real64 RhofgHighTempValue = 0.0; // High Temperature Value for Rhofg (max in tables)
        int RhofgLowTempIndex = 0;       // Low Temperature Min Index for Rhofg (>0.0)
        int RhofgHighTempIndex = 0;      // High Temperature Max Index for Rhofg (>0.0)
        Array1D<Real64> RhoTemps;        // Temperatures for density of fluid points
        Array1D<Real64> RhofValues;      // Density of saturated fluid at RhoTemps
        Array1D<Real64> RhofgValues;     // Density of saturated fluid/gas at RhoTemps
#ifdef PERFORMANCE_OPT
        Array1D<Real64> RhofTempRatios;
        Array1D<Real64> RhofgTempRatios;
#endif // PERFORMANCE_OPT

        int NumSupTempPoints = 0;    // Number of temperature points for superheated enthalpy
        int NumSupPressPoints = 0;   // Number of pressure points for superheated enthalpy
        Array1D<Real64> SupTemps;    // Temperatures for superheated gas
        Array1D<Real64> SupPress;    // Pressures for superheated gas
        Array2D<Real64> HshValues;   // Enthalpy of superheated gas at HshTemps, HshPress
        Array2D<Real64> RhoshValues; // Density of superheated gas at HshTemps, HshPress

        std::array<ErrorCountIndex, (int)RefrigError::Num> errors;

        Real64 getQuality(EnergyPlusData &state,
                          Real64 Temperature,           // actual temperature given as input
                          Real64 Enthalpy,              // actual enthalpy given as input
                          std::string_view CalledFrom); // routine this function was called from (error messages)

        Real64 getSatPressure(EnergyPlusData &state,
                              Real64 Temperature,           // actual temperature given as input
                              std::string_view CalledFrom); // routine this function was called from (error messages)

        Real64 getSatTemperature(EnergyPlusData &state,
                                 Real64 Pressure,              // actual temperature given as input
                                 std::string_view CalledFrom); // routine this function was called from (error messages)

        Real64 getSatEnthalpy(EnergyPlusData &state,
                              Real64 Temperature,           // actual temperature given as input
                              Real64 Quality,               // actual quality given as input
                              std::string_view CalledFrom); // routine this function was called from (error messages)

        Real64 getSatDensity(EnergyPlusData &state,
                             Real64 Temperature,           // actual temperature given as input
                             Real64 Quality,               // actual quality given as input
                             std::string_view CalledFrom); // routine this function was called from (error messages)

        Real64 getSatSpecificHeat(EnergyPlusData &state,
                                  Real64 Temperature,           // actual temperature given as input
                                  Real64 Quality,               // actual quality given as input
                                  std::string_view CalledFrom); // routine this function was called from (error messages)

        Real64 getSupHeatEnthalpy(EnergyPlusData &state,
                                  Real64 Temperature,           // actual temperature given as input
                                  Real64 Pressure,              // actual pressure given as input
                                  std::string_view CalledFrom); // routine this function was called from (error messages)

        Real64 getSupHeatPressure(EnergyPlusData &state,
                                  Real64 Temperature,           // actual temperature given as input
                                  Real64 Enthalpy,              // actual enthalpy given as input
                                  std::string_view CalledFrom); // routine this function was called from (error messages)

        Real64 getSupHeatTemp(EnergyPlusData &state,
                              Real64 Pressure,              // actual pressure given as input
                              Real64 Enthalpy,              // actual enthalpy given as input
                              Real64 TempLow,               // lower bound of temperature in the iteration
                              Real64 TempUp,                // upper bound of temperature in the iteration
                              std::string_view CalledFrom); // routine this function was called from (error messages)

        Real64 getSupHeatDensity(EnergyPlusData &state,
                                 Real64 Temperature,           // actual temperature given as input
                                 Real64 Pressure,              // actual pressure given as input
                                 std::string_view CalledFrom); // routine this function was called from (error messages)

        void setTempLimits(EnergyPlusData &state, bool &ErrorsFound);
    };

    enum class GlycolError
    {
        Invalid = -1,
        SpecHeatLow,
        SpecHeatHigh,
        DensityLow,
        DensityHigh,
        ConductivityLow,
        ConductivityHigh,
        ViscosityLow,
        ViscosityHigh,
        Num
    };

    struct GlycolRawProps
    {
        // Members
        std::string Name; // Name of the glycol
        int Num = 0;

        std::string CpTempArrayName;
        bool CpDataPresent = false; // Flag set when specific heat data is available
        int NumCpTempPoints = 0;    // Number of temperature points for specific heat
        int NumCpConcPoints = 0;    // Number of concentration points for specific heat
        Array1D<Real64> CpTemps;    // Temperatures for specific heat of glycol
        Array1D<Real64> CpConcs;    // Concentration for specific heat of glycol
        Array2D<Real64> CpValues;   // Specific heat data values

        std::string RhoTempArrayName;
        bool RhoDataPresent = false; // Flag set when density data is available
        int NumRhoTempPoints = 0;    // Number of temperature points for density
        int NumRhoConcPoints = 0;    // Number of concentration points for density
        Array1D<Real64> RhoTemps;    // Temperatures for density of glycol
        Array1D<Real64> RhoConcs;    // Concentration for density of glycol
        Array2D<Real64> RhoValues;   // Density data values

        std::string CondTempArrayName;
        bool CondDataPresent = false; // Flag set when conductivity data is available
        int NumCondTempPoints = 0;    // Number of temperature points for conductivity
        int NumCondConcPoints = 0;    // Number of concentration points for conductivity
        Array1D<Real64> CondTemps;    // Temperatures for conductivity of glycol
        Array1D<Real64> CondConcs;    // Concentration for conductivity of glycol
        Array2D<Real64> CondValues;   // conductivity values

        std::string ViscTempArrayName;
        bool ViscDataPresent = false; // Flag set when viscosity data is available
        int NumViscTempPoints = 0;    // Number of temperature points for viscosity
        int NumViscConcPoints = 0;    // Number of concentration points for viscosity
        Array1D<Real64> ViscTemps;    // Temperatures for viscosity of glycol
        Array1D<Real64> ViscConcs;    // Concentration for viscosity of glycol
        Array2D<Real64> ViscValues;   // viscosity values
    };

    struct GlycolProps
    {
        // Members
        std::string Name; // Name of the glycol mixture (used by other parts of code)
        int Num = 0;
        bool used = false;

        std::string GlycolName; // Name of non-water fluid that is part of this mixture
        // (refers to ethylene glycol, propylene glycol, or user fluid)
        int BaseGlycolIndex = 0; // Index in user defined glycol data (>0 = index in raw data,
        // -1=propylene glycol, -2=ethylene glycol)
        Real64 Concentration = 0.0; // Concentration (if applicable)

        bool CpDataPresent = false;   // Flag set when specific heat data is available
        Real64 CpLowTempValue = 0.0;  // Low Temperature Value for Cp (>0.0)
        Real64 CpHighTempValue = 0.0; // High Temperature Value for Cp (max in tables)
        int CpLowTempIndex = 0;       // Low Temperature Min Index for Cp (>0.0)
        int CpHighTempIndex = 0;      // High Temperature Max Index for Cp (>0.0)
        int NumCpTempPoints = 0;      // Number of temperature points for specific heat
        Array1D<Real64> CpTemps;      // Temperatures for specific heat of glycol
        Array1D<Real64> CpValues;     // Specific heat data values (J/kg-K)
#ifdef PERFORMANCE_OPT
        int LoCpTempIdxLast = 1;
        Array1D<Real64> CpTempRatios; // Speed optimization
#endif                                // PERFORMANCE_OPT

        bool RhoDataPresent = false;   // Flag set when density data is available
        int NumRhoTempPoints = 0.0;    // Number of temperature points for density
        Real64 RhoLowTempValue = 0.0;  // Low Temperature Value for Rho (>0.0)
        Real64 RhoHighTempValue = 0.0; // High Temperature Value for Rho (max in tables)
        int RhoLowTempIndex = 0;       // Low Temperature Min Index for Rho (>0.0)
        int RhoHighTempIndex = 0;      // High Temperature Max Index for Rho (>0.0)
        Array1D<Real64> RhoTemps;      // Temperatures for density of glycol
        Array1D<Real64> RhoValues;     // Density data values (kg/m3)
#ifdef PERFORMANCE_OPT
        int LoRhoTempIdxLast = 1;
        Array1D<Real64> RhoTempRatios; // Speed optimization
#endif                                 // PERFORMANCE_OPT

        bool CondDataPresent = false;   // Flag set when conductivity data is available
        int NumCondTempPoints = 0;      // Number of temperature points for conductivity
        Real64 CondLowTempValue = 0.0;  // Low Temperature Value for Cond (>0.0)
        Real64 CondHighTempValue = 0.0; // High Temperature Value for Cond (max in tables)
        int CondLowTempIndex = 0;       // Low Temperature Min Index for Cond (>0.0)
        int CondHighTempIndex = 0;      // High Temperature Max Index for Cond (>0.0)
        Array1D<Real64> CondTemps;      // Temperatures for conductivity of glycol
        Array1D<Real64> CondValues;     // conductivity values (W/m-K)
#ifdef PERFORMANCE_OPT
        int LoCondTempIdxLast = 1;
        Array1D<Real64> CondTempRatios; // Speed optimization
#endif                                  // PERFORMANCE_OPT

        bool ViscDataPresent = false;   // Flag set when viscosity data is available
        int NumViscTempPoints = 0;      // Number of temperature points for viscosity
        Real64 ViscLowTempValue = 0.0;  // Low Temperature Value for Visc (>0.0)
        Real64 ViscHighTempValue = 0.0; // High Temperature Value for Visc (max in tables)
        int ViscLowTempIndex = 0;       // Low Temperature Min Index for Visc (>0.0)
        int ViscHighTempIndex = 0;      // High Temperature Max Index for Visc (>0.0)
        Array1D<Real64> ViscTemps;      // Temperatures for viscosity of glycol
        Array1D<Real64> ViscValues;     // viscosity values (mPa-s)
#ifdef PERFORMANCE_OPT
        int LoViscTempIdxLast = 1;
        Array1D<Real64> ViscTempRatios;
#endif // PERFORMANCE_OPT

        std::array<ErrorCountIndex, (int)GlycolError::Num> errors;

#ifdef EP_cache_GlycolSpecificHeat
        Real64 getSpecificHeat_raw(EnergyPlusData &state,
                                   Real64 Temperature,         // actual temperature given as input
                                   std::string_view CalledFrom // routine this function was called from (error messages)
        );
#endif
        Real64 getSpecificHeat(EnergyPlusData &state,
                               Real64 Temperature,           // actual temperature given as input
                               std::string_view CalledFrom); // routine this function was called from (error messages)

        Real64 getDensity(EnergyPlusData &state,
                          Real64 Temperature,           // actual temperature given as input
                          std::string_view CalledFrom); // routine this function was called from (error messages)

        Real64 getConductivity(EnergyPlusData &state,
                               Real64 Temperature,           // actual temperature given as input
                               std::string_view CalledFrom); // routine this function was called from (error messages)

        Real64 getViscosity(EnergyPlusData &state,
                            Real64 Temperature,           // actual temperature given as input
                            std::string_view CalledFrom); // routine this function was called from (error messages)

        void setTempLimits(EnergyPlusData &state, bool &ErrorsFound);

        void getDensityTemperatureLimits(EnergyPlusData &state, Real64 &MinTempLimit, Real64 &MaxTempLimit);

        void getSpecificHeatTemperatureLimits(EnergyPlusData &state, Real64 &MinTempLimit, Real64 &MaxTempLimit);
    };

    struct cached_tsh
    {
        // Members
        std::uint64_t iT = 1000;
        Real64 sh = 0.0;
    };

    void GetFluidPropertiesData(EnergyPlusData &state);

    void InitConstantFluidPropertiesData(EnergyPlusData &state);

    template <size_t NumOfTemps, size_t NumOfConcs>
    void InterpDefValuesForGlycolConc(
        EnergyPlusData &state,
        const std::array<Real64, NumOfConcs> &RawConcData,                         // concentrations for raw data
        const std::array<std::array<Real64, NumOfTemps>, NumOfConcs> &RawPropData, // raw property data (concentration, temperature)
        Real64 Concentration,                                                      // concentration of actual fluid mix
        Array1D<Real64> &InterpData                                                // interpolated output data at proper concentration
    );

    void InterpValuesForGlycolConc(EnergyPlusData &state,
                                   int NumOfConcs,                     // number of concentrations (dimension of raw data)
                                   int NumOfTemps,                     // number of temperatures (dimension of raw data)
                                   const Array1D<Real64> &RawConcData, // concentrations for raw data
                                   Array2S<Real64> RawPropData,        // raw property data (temperature,concentration)
                                   Real64 Concentration,               // concentration of actual fluid mix
                                   Array1D<Real64> &InterpData         // interpolated output data at proper concentration
    );

    void ReportAndTestGlycols(EnergyPlusData &state);

    void ReportAndTestRefrigerants(EnergyPlusData &state);
#ifdef GET_OUT
    Real64 GetQualityRefrig(EnergyPlusData &state,
                            std::string const &Refrigerant, // carries in substance name
                            Real64 Temperature,             // actual temperature given as input
                            Real64 Enthalpy,                // actual enthalpy given as input
                            int &RefrigIndex,               // Index to Refrigerant Properties
                            std::string_view CalledFrom     // routine this function was called from (error messages)
    );

    Real64 GetSatPressureRefrig(EnergyPlusData &state,
                                std::string_view Refrigerant, // carries in substance name
                                Real64 Temperature,           // actual temperature given as input
                                int &RefrigIndex,             // Index to Refrigerant Properties
                                std::string_view CalledFrom   // routine this function was called from (error messages)
    );

    Real64 GetSatTemperatureRefrig(EnergyPlusData &state,
                                   std::string_view Refrigerant, // carries in substance name
                                   Real64 Pressure,              // actual temperature given as input
                                   int &RefrigIndex,             // Index to Refrigerant Properties
                                   std::string_view CalledFrom   // routine this function was called from (error messages)
    );

    Real64 GetSatEnthalpyRefrig(EnergyPlusData &state,
                                std::string_view Refrigerant, // carries in substance name
                                Real64 Temperature,           // actual temperature given as input
                                Real64 Quality,               // actual quality given as input
                                int &RefrigIndex,             // Index to Refrigerant Properties
                                std::string_view CalledFrom   // routine this function was called from (error messages)
    );

    Real64 GetSatDensityRefrig(EnergyPlusData &state,
                               std::string_view Refrigerant, // carries in substance name
                               Real64 Temperature,           // actual temperature given as input
                               Real64 Quality,               // actual quality given as input
                               int &RefrigIndex,             // Index to Refrigerant Properties
                               std::string_view CalledFrom   // routine this function was called from (error messages)
    );

    Real64 GetSatSpecificHeatRefrig(EnergyPlusData &state,
                                    std::string_view Refrigerant, // carries in substance name
                                    Real64 Temperature,           // actual temperature given as input
                                    Real64 Quality,               // actual quality given as input
                                    int &RefrigIndex,             // Index to Refrigerant Properties
                                    std::string_view CalledFrom   // routine this function was called from (error messages)
    );

    Real64 GetSupHeatEnthalpyRefrig(EnergyPlusData &state,
                                    std::string_view Refrigerant, // carries in substance name
                                    Real64 Temperature,           // actual temperature given as input
                                    Real64 Pressure,              // actual pressure given as input
                                    int &RefrigIndex,             // Index to Refrigerant Properties
                                    std::string_view CalledFrom   // routine this function was called from (error messages)
    );

    Real64 GetSupHeatPressureRefrig(EnergyPlusData &state,
                                    std::string_view Refrigerant, // carries in substance name
                                    Real64 Temperature,           // actual temperature given as input
                                    Real64 Enthalpy,              // actual enthalpy given as input
                                    int &RefrigIndex,             // Index to Refrigerant Properties
                                    std::string_view CalledFrom   // routine this function was called from (error messages)
    );

    Real64 GetSupHeatTempRefrig(EnergyPlusData &state,
                                std::string_view Refrigerant, // carries in substance name
                                Real64 Pressure,              // actual pressure given as input
                                Real64 Enthalpy,              // actual enthalpy given as input
                                Real64 TempLow,               // lower bound of temperature in the iteration
                                Real64 TempUp,                // upper bound of temperature in the iteration
                                int &RefrigIndex,             // Index to Refrigerant Properties
                                std::string_view CalledFrom   // routine this function was called from (error messages)
    );

    Real64 GetSupHeatDensityRefrig(EnergyPlusData &state,
                                   std::string_view Refrigerant, // carries in substance name
                                   Real64 Temperature,           // actual temperature given as input
                                   Real64 Pressure,              // actual pressure given as input
                                   int &RefrigIndex,             // Index to Refrigerant Properties
                                   std::string_view CalledFrom   // routine this function was called from (error messages)
    );

    Real64 GetSpecificHeatGlycol(EnergyPlusData &state,
                                 std::string_view Glycol,    // carries in substance name
                                 Real64 Temperature,         // actual temperature given as input
                                 int &GlycolIndex,           // Index to Glycol Properties
                                 std::string_view CalledFrom // routine this function was called from (error messages)
    );

    Real64 GetDensityGlycol(EnergyPlusData &state,
                            std::string_view Glycol,    // carries in substance name
                            Real64 Temperature,         // actual temperature given as input
                            int &GlycolIndex,           // Index to Glycol Properties
                            std::string_view CalledFrom // routine this function was called from (error messages)
    );

    Real64 GetConductivityGlycol(EnergyPlusData &state,
                                 std::string_view Glycol,    // carries in substance name
                                 Real64 Temperature,         // actual temperature given as input
                                 int &GlycolIndex,           // Index to Glycol Properties
                                 std::string_view CalledFrom // routine this function was called from (error messages)
    );

    Real64 GetViscosityGlycol(EnergyPlusData &state,
                              std::string_view Glycol,    // carries in substance name
                              Real64 Temperature,         // actual temperature given as input
                              int &GlycolIndex,           // Index to Glycol Properties
                              std::string_view CalledFrom // routine this function was called from (error messages)
    );
#endif // GET_OUT

    inline Real64 GetInterpValue(Real64 const Tact, // actual temperature at which we want the property of interest
                                 Real64 const Tlo,  // temperature below Tact for which we have property data
                                 Real64 const Thi,  // temperature above Tact for which we have property data
                                 Real64 const Xlo,  // value of property at Tlo
                                 Real64 const Xhi   // value of property at Thi
    )
    {
        return Xhi - (((Thi - Tact) / (Thi - Tlo)) * (Xhi - Xlo));
    }

    int GetRefrigNum(EnergyPlusData &state, std::string_view name);
    RefrigProps *GetRefrig(EnergyPlusData &state, std::string_view name);
    RefrigProps *GetSteam(EnergyPlusData &state);

    int GetGlycolRawNum(EnergyPlusData &state, std::string_view name);
    GlycolRawProps *GetGlycolRaw(EnergyPlusData &state, std::string_view name);

    int GetGlycolNum(EnergyPlusData &state, std::string_view name);
    GlycolProps *GetGlycol(EnergyPlusData &state, std::string_view name);
    GlycolProps *GetWater(EnergyPlusData &state);

    std::string GetGlycolNameByIndex(EnergyPlusData &state, int Idx); // carries in substance index

    int FindArrayIndex(Real64 Value,                 // Value to be placed/found within the array of values
                       Array1D<Real64> const &Array, // Array of values in ascending order
                       int LowBound,                 // Valid values lower bound (set by calling program)
                       int UpperBound                // Valid values upper bound (set by calling program)
    );

    int FindArrayIndex(Real64 Value,                // Value to be placed/found within the array of values
                       Array1D<Real64> const &Array // Array of values in ascending order
    );

    Real64 GetInterpolatedSatProp(EnergyPlusData &state,
                                  Real64 Temperature,               // Saturation Temp.
                                  Array1D<Real64> const &PropTemps, // Array of temperature at which props are available
                                  Array1D<Real64> const &LiqProp,   // Array of saturated liquid properties
                                  Array1D<Real64> const &VapProp,   // Array of saturatedvapour properties
                                  Real64 Quality,                   // Quality
                                  std::string_view CalledFrom,      // routine this function was called from (error messages)
                                  int LowBound,                     // Valid values lower bound (set by calling program)
                                  int UpperBound                    // Valid values upper bound (set by calling program)
    );

    void ReportOrphanFluids(EnergyPlusData &state);
} // namespace Fluid

struct FluidData : BaseGlobalStruct
{
    bool DebugReportGlycols = false;
    bool DebugReportRefrigerants = false;
    int GlycolErrorLimitTest = 1; // how many times error is printed with details before recurring called
    int RefrigErrorLimitTest = 1; // how many times error is printed with details before recurring called

    Array1D<Fluid::RefrigProps *> refrigs;
    Array1D<Fluid::GlycolRawProps *> glycolsRaw;
    Array1D<Fluid::GlycolProps *> glycols;

    std::array<int, (int)Fluid::GlycolError::Num> glycolErrorLimits = {0, 0, 0, 0, 0, 0, 0, 0};

    int SatErrCountGetSupHeatEnthalpyRefrig = 0;
    int SatErrCountGetSupHeatDensityRefrig = 0;
    int TempLoRangeErrIndexGetQualityRefrig = 0;
    int TempHiRangeErrIndexGetQualityRefrig = 0;
    int TempRangeErrCountGetInterpolatedSatProp = 0;
    int TempRangeErrIndexGetInterpolatedSatProp = 0;

#ifdef EP_cache_GlycolSpecificHeat
    std::array<Fluid::cached_tsh, Fluid::t_sh_cache_size> cached_t_sh;
#endif

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
        Fluid::InitConstantFluidPropertiesData(state);
    }

    void init_state(EnergyPlusData &state) override
    {
        Fluid::GetFluidPropertiesData(state);
    }

    void clear_state() override
    {

        for (int i = 1; i <= refrigs.isize(); ++i) {
            refrigs(i)->HshValues.deallocate();
            refrigs(i)->RhoshValues.deallocate();
            delete refrigs(i);
        }
        for (int i = 1; i <= glycolsRaw.isize(); ++i) {
            delete glycolsRaw(i);
        }
        for (int i = 1; i <= glycols.isize(); ++i) {
            delete glycols(i);
        }

        new (this) FluidData();
    }
};

} // namespace EnergyPlus

#endif
