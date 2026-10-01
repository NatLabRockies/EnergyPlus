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

#include <EnergyPlus/api/TypeDefs.h>
#include <EnergyPlus/api/func.h>
#include <EnergyPlus/api/state.h>
#include <stdio.h>

int errorsOccurred = 0;
void errorHandler(int errortype, const char *message)
{
    (void)errortype;
    (void)message;
    errorsOccurred++;
}

int main()
{
    EnergyPlusState state = stateNew();
    initializeFunctionalAPI(state);

    // GLYCOLS
    Glycol glycol = NULL;
    glycol = glycolNew(state, "WatEr");
    for (int temp = 5; temp < 35; temp += 10) {
        Real64 thisTemp = (float)temp;
        Real64 specificHeat = glycolSpecificHeat(state, glycol, thisTemp);
        Real64 density = glycolDensity(state, glycol, thisTemp);
        Real64 conductivity = glycolConductivity(state, glycol, thisTemp);
        Real64 viscosity = glycolViscosity(state, glycol, thisTemp);
        printf("C API Test: Calculated props at T=%4.1f: %8.4f, %8.4f, %8.4f, %8.4f \n", thisTemp, specificHeat, density, conductivity, viscosity);
    }
    glycolDelete(state, glycol);

    // REFRIGERANTS
    Refrigerant refrig = NULL;
    refrig = refrigerantNew(state, "SteaM");
    Real64 temperature = 100.0;
    Real64 satPress = refrigerantSaturationPressure(state, refrig, temperature); // expecting about 100,000 Pa
    Real64 thisPress = 100000;
    Real64 satTemp = refrigerantSaturationTemperature(state, refrig, thisPress); // expecting about 100 degC
    printf("C API Test: Saturated Properties: At 100C, Psat=%8.4f; at 100000Pa, Tsat=%8.4f\n", satPress, satTemp);
    Real64 satLiqDens = refrigerantSaturatedDensity(state, refrig, temperature, 0.0);    // liq = 958 kg/m3
    Real64 satLiqCp = refrigerantSaturatedSpecificHeat(state, refrig, temperature, 0.0); // liq = 4,216 J/kgK
    Real64 satLiqEnth = refrigerantSaturatedEnthalpy(state, refrig, temperature, 0.0);
    printf("C API Test: Sat Liq at 100C: rho=%8.4f, Cp=%8.4f, h=%8.4f\n", satLiqDens, satLiqCp, satLiqEnth);
    Real64 satVapDens = refrigerantSaturatedDensity(state, refrig, temperature, 1.0);    // vap = 1/1.6718 ~~ 0.59 kg/m3
    Real64 satVapCp = refrigerantSaturatedSpecificHeat(state, refrig, temperature, 1.0); // vap = 2,080 J/kgK
    Real64 satVapEnth = refrigerantSaturatedEnthalpy(state, refrig, temperature, 1.0);
    printf("C API Test: Sat Vap at 100C: rho=%8.4f, Cp=%8.4f, h=%8.4f\n", satVapDens, satVapCp, satVapEnth);
    // Real64 enthDifference = satVapEnth - satLiqEnth; // vap-liq = 2,675,570-419,170 ~ 2,256,400 J/kg

    // superheated properties aren't working, I think there is a bug in the FluidProperties module
    //    temperature = 150;
    //    Real64 supEnth = refrigerantSuperHeatedEnthalpy(refrig, temperature, thisPress);
    //    Real64 thisEnth = 303;
    //    Real64 supPress = refrigerantSuperHeatedPressure(refrig, temperature, thisEnth);
    //    Real64 supDensity = refrigerantSuperHeatedDensity(refrig, temperature, thisPress);
    refrigerantDelete(state, refrig);

    // PSYCHROMETRICS
    // test point is:
    //   Barometric Pressure: 101325 Pa
    //   Dry Bulb Temp: 24 C
    //   Relative Humidity: 0.5
    //   Humidity Ratio: ~0.009 kg/kg
    //   Saturation Temp: ~17 C
    //   Saturation Pressure: 2985 Pa
    //   Enthalpy: ~48000 J/kg
    //   Specific Volume: 0.855 m3/kg
    //   Density: ~1.17
    //   Wet Bulb: ~17 C
    //   Dew Point: ~13 C
    //   Vapor Density: 0.0107 kg/m3
    //   Specific Heat: ~1007 J/kgK
    printf("C API Test: Psych props, test point is about 101325Pa, 24C, 50%% humidity:\n");
    Real64 db = psyTdbFnHW(state, 48000, 0.009);
    printf("C API Test: Expected DB ~ 24 C; Calculated: %8.4f\n", db);
    Real64 rh = psyRhFnTdbRhov(state, 24, 0.0107);
    Real64 rh2 = psyRhFnTdbWPb(state, 24, 0.009, 101325);
    printf("C API Test: Expected RH ~ 0.5; Calculated: %8.4f, %8.4f\n", rh, rh2);
    Real64 hr = psyWFnTdbH(state, 24, 48000);
    Real64 hr2 = psyWFnTdpPb(state, 13, 101325);
    Real64 hr3 = psyWFnTdbRhPb(state, 24, 0.5, 101325);
    Real64 hr4 = psyWFnTdbTwbPb(state, 24, 17, 101325);
    printf("C API Test: Expected HumRat ~ 0.009; Calculated: %8.4f, %8.4f, %8.4f, %8.4f\n", hr, hr2, hr3, hr4);
    Real64 tSat = psyTsatFnHPb(state, 48000, 101325);
    printf("C API Test: Expected Tsat ~ 17 C; Calculated: %8.4f\n", tSat);
    Real64 pSat = psyPsatFnTemp(state, 24);
    printf("C API Test: Expected Psat ~ 2985 Pa; Calculated: %8.4f\n", pSat);
    Real64 h = psyHFnTdbW(state, 24, 0.009);
    Real64 h2 = psyHFnTdbRhPb(state, 24, 0.5, 101325);
    printf("C API Test: Expected Enth ~ 48000 J/kg; Calculated: %8.4f, %8.4f\n", h, h2);
    Real64 volume = psyVFnTdbWPb(state, 24, 0.009, 101325);
    printf("C API Test: Expected v ~ 0.855 m3/kg; Calculated: %8.4f\n", volume);
    Real64 density = psyRhoFnPbTdbW(state, 101325, 24, 0.009);
    printf("C API Test: Expected rho ~ 1.17 kg/m3; Calculated: %8.4f\n", density);
    Real64 wb = psyTwbFnTdbWPb(state, 24, 0.009, 101325);
    printf("C API Test: Expected WB ~ 17 C; Calculated: %8.4f\n", wb);
    Real64 dp = psyTdpFnWPb(state, 0.009, 101325);
    Real64 dp2 = psyTdpFnTdbTwbPb(state, 24, 17, 101325);
    printf("C API Test: Expected DP ~ 13 C; Calculated: %8.4f, %8.4f\n", dp, dp2);
    Real64 vaporDensity = psyRhovFnTdbWPb(state, 24, 0.009, 101325);
    Real64 vaporDensity_2 = psyRhovFnTdbRh(state, 24, 0.5);
    printf("C API Test: Expected VapDensity ~ 0.0107 kg/m3; Calculated: %8.4f, %8.4f\n", vaporDensity, vaporDensity_2);
    Real64 cp = psyCpAirFnW(state, 0.009);
    printf("C API Test: Expected Cp ~ 1007 J/kgK; Calculated: %8.4f\n", cp);
    Real64 energy = psyHfgAirFnWTdb(state, 24);
    printf("C API Test: Calculated energy: %8.4f\n", energy);
    Real64 moisture_energy = psyHgAirFnWTdb(state, 24);
    printf("C API Test: Calculated energy of moisture: %8.4f\n", moisture_energy);

    // check that we get error messages back:
    registerErrorCallback(state, errorHandler);
    Real64 dpErroneous = psyTdpFnTdbTwbPb(state, 16, 17, 101325);
    printf("C API Test: Got back erroneous value of dew point: %8.4f\n", dpErroneous);
    if (errorsOccurred > 0) {
        printf("C API Test: Errors were caught during dew point calculation, good!\n");
    } else {
#ifdef EP_psych_errors
        printf("C API Test: Errors were NOT caught during dew point calculation, bad!\n");
        return 1;
#else
        printf("C API Test: Psychrometric errors are disabled; no dew point calculation errors expected.\n");
#endif
    }

    printf("EnergyPlus Version: %s", energyPlusVersion());

    return 0;
}
