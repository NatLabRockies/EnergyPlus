# EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the
# University of Illinois, The Regents of the University of California, through
# Lawrence Berkeley National Laboratory (subject to receipt of any required
# approvals from the U.S. Dept. of Energy), Oak Ridge National Laboratory,
# managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
# contributors. All rights reserved.
#
# NOTICE: This Software was developed under funding from the U.S. Department of
# Energy and the U.S. Government consequently retains certain rights. As such,
# the U.S. Government has been granted for itself and others acting on its
# behalf a paid-up, nonexclusive, irrevocable, worldwide license in the
# Software to reproduce, distribute copies to the public, prepare derivative
# works, and perform publicly and display publicly, and to permit others to do
# so.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# (1) Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#
# (2) Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#
# (3) Neither the name of the University of California, Lawrence Berkeley
#     National Laboratory, the University of Illinois, U.S. Dept. of Energy nor
#     the names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
# (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in
#     stand-alone form without changes from the version obtained under this
#     License, or (ii) Licensee makes a reference solely to the software
#     portion of its product, Licensee must refer to the software as
#     "EnergyPlus version X" software, where "X" is the version number Licensee
#     obtained under this License and may not use a different name for the
#     software. Except as specifically required in this Section (4), Licensee
#     shall not use in a company name, a product name, in advertising,
#     publicity, or other promotional activities any name, trade name,
#     trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or
#     confusingly similar designation, without the U.S. Department of Energy's
#     prior written consent.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import os
import sys

from pyenergyplus.api import EnergyPlusAPI

api = EnergyPlusAPI()

state = api.state_manager.new_state()

# GLYCOL TESTS
glycol = api.functional.glycol(state, "water")
for t in [5.0, 15.0, 25.0]:
    cp = glycol.specific_heat(state, t)
    rho = glycol.density(state, t)
    k = glycol.conductivity(state, t)
    viscosity = glycol.viscosity(state, t)
    print("Python API Test: Calculated properties at T=%s: %f, %f, %f, %f" % (t, cp, rho, k, viscosity))
glycol.delete(state)

# REFRIGERANT TESTS
refrigerant = api.functional.refrigerant(state, "steam")
temperature = 100.0
satPress = refrigerant.saturation_pressure(state, temperature)  # expecting about 101325 Pa
thisPress = 100000
satTemp = refrigerant.saturation_temperature(state, thisPress)  # expecting about 100 degC
print("Python API Test: Saturated Properties: At 100C, Psat=%8.4f; at 100000Pa, Tsat=%8.4f" % (satPress, satTemp))
satLiqDens = refrigerant.saturated_density(state, temperature, 0.0)  # // liq = 958 kg/m3
satLiqCp = refrigerant.saturated_specific_heat(state, temperature, 0.0)  # liq = 4,216 J/kgK
satLiqEnth = refrigerant.saturated_enthalpy(state, temperature, 0.0)
print("C API Test: Sat Liq at 100C: rho=%8.4f, Cp=%8.4f, h=%8.4f" % (satLiqDens, satLiqCp, satLiqEnth))
satVapDens = refrigerant.saturated_density(state, temperature, 1.0)  # vap = 1/1.6718 ~~ 0.59 kg/m3
satVapCp = refrigerant.saturated_specific_heat(state, temperature, 1.0)  # vap = 2,080 J/kgK
satVapEnth = refrigerant.saturated_enthalpy(state, temperature, 1.0)
print("C API Test: Sat Vap at 100C: rho=%8.4f, Cp=%8.4f, h=%8.4f" % (satVapDens, satVapCp, satVapEnth))
enthDifference = satVapEnth - satLiqEnth  # vap-liq = 2,675,570-419,170 ~ 2,256,400 J/kg
refrigerant.delete(state)

# PSYCHROMETRIC TESTS
psychrometrics = api.functional.psychrometrics(state)
# // PSYCHROMETRICS
# // test point is:
# //   Barometric Pressure: 101325 Pa
# //   Dry Bulb Temp: 24 C
# //   Relative Humidity: 0.5
# //   Humidity Ratio: ~0.009 kg/kg
# //   Saturation Temp: ~17 C
# //   Saturation Pressure: 2985 Pa
# //   Enthalpy: ~48000 J/kg
# //   Specific Volume: 0.855 m3/kg
# //   Density: ~1.17
# //   Wet Bulb: ~17 C
# //   Dew Point: ~13 C
# //   Vapor Density: 0.0107 kg/m3
# //   Specific Heat: ~1007 J/kgK
print("Python API Test: Psych props, test point is about 101325Pa, 24C, 50%% humidity:")
db = psychrometrics.dry_bulb(state, 48000, 0.009)
print("Python API Test: Expected DB ~ 24 C Calculated: %8.4f" % db)
rh = psychrometrics.relative_humidity(state, 24, 0.0107)
rh2 = psychrometrics.relative_humidity_b(state, 24, 0.009, 101325)
print("Python API Test: Expected RH ~ 0.5 Calculated: %8.4f, %8.4f" % (rh, rh2))
hr = psychrometrics.humidity_ratio(state, 24, 48000)
hr2 = psychrometrics.humidity_ratio_b(state, 13, 101325)
hr3 = psychrometrics.humidity_ratio_c(state, 24, 0.5, 101325)
hr4 = psychrometrics.humidity_ratio_d(state, 24, 17, 101325)
print("Python API Test: Expected HumRat ~ 0.009 Calculated: %8.4f, %8.4f, %8.4f, %8.4f" % (hr, hr2, hr3, hr4))
tSat = psychrometrics.saturation_temperature(state, 48000, 101325)
print("Python API Test: Expected Tsat ~ 17 C Calculated: %8.4f" % tSat)
pSat = psychrometrics.saturation_pressure(state, 24)
print("Python API Test: Expected Psat ~ 2985 Pa Calculated: %8.4f" % pSat)
h = psychrometrics.enthalpy(state, 24, 0.009)
h2 = psychrometrics.enthalpy_b(state, 24, 0.5, 101325)
print("Python API Test: Expected Enth ~ 0.48000 J/kg Calculated: %8.4f, %8.4f" % (h, h2))
volume = psychrometrics.specific_volume(state, 24, 0.009, 101325)
print("Python API Test: Expected v ~ 0.855 m3/kg Calculated: %8.4f" % volume)
density = psychrometrics.density(state, 101325, 24, 0.009)
print("Python API Test: Expected rho ~ 1.17 kg/m3 Calculated: %8.4f" % density)
wb = psychrometrics.wet_bulb(state, 24, 0.009, 101325)
print("Python API Test: Expected WB ~ 17 C Calculated: %8.4f" % wb)
dp = psychrometrics.dew_point(state, 0.009, 101325)
dp2 = psychrometrics.dew_point_b(state, 24, 17, 101325)
print("Python API Test: Expected DP ~ 13 C Calculated: %8.4f, %8.4f" % (dp, dp2))
vaporDensity = psychrometrics.vapor_density(state, 24, 0.009, 101325)
vaporDensity_2 = psychrometrics.vapor_density_b(state, 24, 0.5)
print("Python API Test: Expected VapDensity ~ 0.0107 kg/m3 Calculated: %8.4f, %8.4f" % (vaporDensity, vaporDensity_2))
cp = psychrometrics.specific_heat(state, 0.009)
print("Python API Test: Expected Cp ~ 1007 J/kgK Calculated: %8.4f" % cp)
energy = psychrometrics.latent_energy_of_air(state, 24)
print("Python API Test: Calculated energy?: %8.4f" % energy)
moisture_energy = psychrometrics.latent_energy_of_moisture_in_air(state, 24)
print("Python API Test: Calculated energy of moisture: %8.4f" % moisture_energy)

# check that we get error messages back:
error_count = 0


def error_handler(severity: int, message: bytes) -> None:
    global error_count
    error_count += 1


api.functional.callback_error(state, error_handler)
erroneous_dew_point = psychrometrics.dew_point_b(state, 16, 17, 101325)
print("Python API Test: Got back erroneous value of dew point: %8.4f\n" % erroneous_dew_point)
if error_count > 0:
    print("Python API Test: Errors were caught during dew point calculation, good!")
else:
    if os.environ.get("ENERGYPLUS_USE_PSYCH_ERRORS") == "1":
        print("Python API Test: Errors were NOT caught during dew point calculation, bad!")
        sys.exit(1)
    print("Python API Test: Psychrometric errors are disabled; no dew point calculation errors expected.")

print("EnergyPlus Version: " + str(api.functional.ep_version()))
