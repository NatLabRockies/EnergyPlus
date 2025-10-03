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

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/InputProcessing/IdfParser.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InputProcessing/InputValidation.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/api/func.h>
#include <EnergyPlus/api/state.h>

void initializeFunctionalAPI(EnergyPlusState state)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    if (!thisState->dataInputProcessing->inputProcessor) {
        thisState->dataInputProcessing->inputProcessor = EnergyPlus::InputProcessor::factory();
    }
    thisState->init_constant_state(*thisState);
    thisState->init_state(*thisState);
}

const char *apiVersionFromEPlus(EnergyPlusState)
{
    return EnergyPlus::DataStringGlobals::PythonAPIVersion.c_str();
}
const char *energyPlusVersion()
{
    return EnergyPlus::DataStringGlobals::VerString.c_str();
}

void registerErrorCallback(EnergyPlusState state, std::function<void(EnergyPlus::Error, const std::string &)> f)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    thisState->dataGlobal->errorCallback = f;
}

void registerErrorCallback(EnergyPlusState state, void (*f)(int, const char *))
{
    const auto stdf = [f](EnergyPlus::Error e, const std::string &message) { f(static_cast<int>(e), message.c_str()); };
    registerErrorCallback(state, stdf);
}

Glycol glycolNew(EnergyPlusState state, const char *glycolName)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    auto *glycol = EnergyPlus::Fluid::GetGlycol(*thisState, EnergyPlus::Util::makeUPPER(glycolName));
    return reinterpret_cast<Glycol>(glycol);
}
void glycolDelete(EnergyPlusState, Glycol)
{
}

Real64 glycolSpecificHeat(EnergyPlusState state, Glycol glycol, Real64 temperature)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return reinterpret_cast<EnergyPlus::Fluid::GlycolProps *>(glycol)->getSpecificHeat(*thisState, temperature, "C-API");
}
Real64 glycolDensity(EnergyPlusState state, Glycol glycol, Real64 temperature)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return reinterpret_cast<EnergyPlus::Fluid::GlycolProps *>(glycol)->getDensity(*thisState, temperature, "C-API");
}
Real64 glycolConductivity(EnergyPlusState state, Glycol glycol, Real64 temperature)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return reinterpret_cast<EnergyPlus::Fluid::GlycolProps *>(glycol)->getConductivity(*thisState, temperature, "C-API");
}
Real64 glycolViscosity(EnergyPlusState state, Glycol glycol, Real64 temperature)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return reinterpret_cast<EnergyPlus::Fluid::GlycolProps *>(glycol)->getViscosity(*thisState, temperature, "C-API");
}

Refrigerant refrigerantNew(EnergyPlusState state, const char *refrigerantName)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    auto *refrigerant = EnergyPlus::Fluid::GetRefrig(*thisState, EnergyPlus::Util::makeUPPER(refrigerantName));
    return reinterpret_cast<Refrigerant>(refrigerant);
}
void refrigerantDelete(EnergyPlusState, Refrigerant)
{
}

Real64 refrigerantSaturationPressure(EnergyPlusState state, Refrigerant refrigerant, Real64 temperature)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return reinterpret_cast<EnergyPlus::Fluid::RefrigProps *>(refrigerant)->getSatPressure(*thisState, temperature, "C-API");
}
Real64 refrigerantSaturationTemperature(EnergyPlusState state, Refrigerant refrigerant, Real64 pressure)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return reinterpret_cast<EnergyPlus::Fluid::RefrigProps *>(refrigerant)->getSatTemperature(*thisState, pressure, "C-API");
}
Real64 refrigerantSaturatedEnthalpy(EnergyPlusState state, Refrigerant refrigerant, Real64 temperature, Real64 quality)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return reinterpret_cast<EnergyPlus::Fluid::RefrigProps *>(refrigerant)->getSatEnthalpy(*thisState, temperature, quality, "C-API");
}
Real64 refrigerantSaturatedDensity(EnergyPlusState state, Refrigerant refrigerant, Real64 temperature, Real64 quality)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return reinterpret_cast<EnergyPlus::Fluid::RefrigProps *>(refrigerant)->getSatDensity(*thisState, temperature, quality, "C-API");
}
Real64 refrigerantSaturatedSpecificHeat(EnergyPlusState state, Refrigerant refrigerant, Real64 temperature, Real64 quality)
{
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return reinterpret_cast<EnergyPlus::Fluid::RefrigProps *>(refrigerant)->getSatSpecificHeat(*thisState, temperature, quality, "C-API");
}
// Real64 refrigerantSuperHeatedEnthalpy(EnergyPlusState, Refrigerant refrigerant, Real64 temperature, Real64 pressure) {
//    return reinterpret_cast<EnergyPlus::FluidProperties::RefrigerantAPI *>(refrigerant)->superHeatedEnthalpy(temperature, pressure);
//}
// Real64 refrigerantSuperHeatedPressure(EnergyPlusState, Refrigerant refrigerant, Real64 temperature, Real64 enthalpy) {
//    return reinterpret_cast<EnergyPlus::FluidProperties::RefrigerantAPI *>(refrigerant)->superHeatedPressure(temperature, enthalpy);
//}
// Real64 refrigerantSuperHeatedDensity(EnergyPlusState, Refrigerant refrigerant, Real64 temperature, Real64 pressure) {
//    return reinterpret_cast<EnergyPlus::FluidProperties::RefrigerantAPI *>(refrigerant)->superHeatedDensity(temperature, pressure);
//}

Real64 psyRhoFnPbTdbW(EnergyPlusState state, Real64 const pb, Real64 const tdb, Real64 const dw)
{
    // barometric pressure (Pascals)
    // dry bulb temperature (Celsius)
    // humidity ratio (kgWater/kgDryAir)
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyRhoAirFnPbTdbW_fast(*thisState, pb, tdb, dw);
}
Real64 psyHfgAirFnWTdb(EnergyPlusState, Real64 const T)
{
    // input temperature {Celsius}
    return EnergyPlus::Psychrometrics::PsyHfgAirFnWTdb(0.0, T); // humidity ratio is not used
}
Real64 psyHgAirFnWTdb(EnergyPlusState, Real64 const T)
{
    // input temperature {Celsius}
    return EnergyPlus::Psychrometrics::PsyHgAirFnWTdb(0.0, T); // humidity ratio is not used
}
Real64 psyHFnTdbW(EnergyPlusState, Real64 const TDB, Real64 const dW)
{
    // dry-bulb temperature {C}
    // humidity ratio
    return EnergyPlus::Psychrometrics::PsyHFnTdbW_fast(TDB, dW);
}
Real64 psyCpAirFnW(EnergyPlusState, Real64 const dw)
{
    // humidity ratio {kgWater/kgDryAir}
    // input temperature {Celsius}
    return EnergyPlus::Psychrometrics::PsyCpAirFnW(dw);
}
Real64 psyTdbFnHW(EnergyPlusState, Real64 const H, Real64 const dW)
{
    // enthalpy {J/kg}
    // humidity ratio
    return EnergyPlus::Psychrometrics::PsyTdbFnHW(H, dW);
}
Real64 psyRhovFnTdbWPb(EnergyPlusState, Real64 const Tdb, Real64 const dW, Real64 const PB)
{
    // dry-bulb temperature {C}
    // humidity ratio
    // Barometric Pressure {Pascals}
    return EnergyPlus::Psychrometrics::PsyRhovFnTdbWPb_fast(Tdb, dW, PB);
}
Real64 psyTwbFnTdbWPb(EnergyPlusState state, Real64 const Tdb, Real64 const W, Real64 const Pb)
{
    // dry-bulb temperature {C}
    // humidity ratio
    // barometric pressure {Pascals}
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyTwbFnTdbWPb(*thisState, Tdb, W, Pb);
}
Real64 psyVFnTdbWPb(EnergyPlusState state, Real64 const TDB, Real64 const dW, Real64 const PB)
{
    // dry-bulb temperature {C}
    // humidity ratio
    // barometric pressure {Pascals}
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyVFnTdbWPb(*thisState, TDB, dW, PB);
}
Real64 psyWFnTdbH(EnergyPlusState state, Real64 const TDB, Real64 const H)
{
    // dry-bulb temperature {C}
    // enthalpy {J/kg}
    std::string dummyString;
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyWFnTdbH(*thisState, TDB, H, dummyString, true);
}
Real64 psyPsatFnTemp(EnergyPlusState state, Real64 const T)
{
    // dry-bulb temperature {C}
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyPsatFnTemp(*thisState, T);
}
Real64 psyTsatFnHPb(EnergyPlusState state, Real64 const H, Real64 const Pb)
{
    // enthalpy {J/kg}
    // barometric pressure {Pascals}
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyTsatFnHPb(*thisState, H, Pb);
}
Real64 psyRhovFnTdbRh(EnergyPlusState state, Real64 const Tdb, Real64 const RH)
{
    // dry-bulb temperature {C}
    // relative humidity value (0.0-1.0)
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyRhovFnTdbRh(*thisState, Tdb, RH);
}
Real64 psyRhFnTdbRhov(EnergyPlusState state, Real64 const Tdb, Real64 const Rhovapor)
{
    // dry-bulb temperature {C}
    // vapor density in air {kg/m3}
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyRhFnTdbRhov(*thisState, Tdb, Rhovapor);
}
Real64 psyRhFnTdbWPb(EnergyPlusState state, Real64 const TDB, Real64 const dW, Real64 const PB)
{
    // dry-bulb temperature {C}
    // humidity ratio
    // barometric pressure {Pascals}
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyRhFnTdbWPb(*thisState, TDB, dW, PB);
}
Real64 psyWFnTdpPb(EnergyPlusState state, Real64 const TDP, Real64 const PB)
{
    // dew-point temperature {C}
    // barometric pressure {Pascals}
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyWFnTdpPb(*thisState, TDP, PB);
}
Real64 psyWFnTdbRhPb(EnergyPlusState state, Real64 const TDB, Real64 const RH, Real64 const PB)
{
    // dry-bulb temperature {C}
    // relative humidity value (0.0-1.0)
    // barometric pressure {Pascals}
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyWFnTdbRhPb(*thisState, TDB, RH, PB);
}
Real64 psyWFnTdbTwbPb(EnergyPlusState state, Real64 const TDB, Real64 const TWBin, Real64 const PB)
{
    // dry-bulb temperature {C}
    // wet-bulb temperature {C}
    // barometric pressure {Pascals}
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyWFnTdbTwbPb(*thisState, TDB, TWBin, PB);
}
Real64 psyHFnTdbRhPb(EnergyPlusState state, Real64 const TDB, Real64 const RH, Real64 const PB)
{
    // dry-bulb temperature {C}
    // relative humidity value (0.0 - 1.0)
    // barometric pressure (N/M**2) {Pascals}
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyHFnTdbRhPb(*thisState, TDB, RH, PB);
}
Real64 psyTdpFnWPb(EnergyPlusState state, Real64 const W, Real64 const PB)
{
    // humidity ratio
    // barometric pressure (N/M**2) {Pascals}
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyTdpFnWPb(*thisState, W, PB);
}
Real64 psyTdpFnTdbTwbPb(EnergyPlusState state, Real64 const TDB, Real64 const TWB, Real64 const PB)
{
    // dry-bulb temperature {C}
    // wet-bulb temperature {C}
    // barometric pressure (N/M**2) {Pascals}
    auto *thisState = reinterpret_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::Psychrometrics::PsyTdpFnTdbTwbPb(*thisState, TDB, TWB, PB);
}
