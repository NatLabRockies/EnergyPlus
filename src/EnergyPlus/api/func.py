# EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University
# of Illinois, The Regents of the University of California, through Lawrence
# Berkeley National Laboratory (subject to receipt of any required approvals
# from the U.S. Dept. of Energy), Oak Ridge National Laboratory, managed by UT-
# Battelle, Alliance for Sustainable Energy, LLC, and other contributors. All
# rights reserved.
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

from ctypes import CFUNCTYPE, c_char_p, c_int, c_void_p, cdll
from types import FunctionType

from pyenergyplus.common import RealEP

# CFUNCTYPE wrapped Python callbacks need to be kept in memory explicitly, otherwise GC takes it
# This causes undefined behavior but generally segfaults and illegal access violations
# Keeping them referenced in a global array here suffices to keep GC away from it
# And while we _could_ clean up after ourselves, I would imagine users aren't making *so* many callbacks that
# We have anything to worry about here
error_callbacks = []


class Glycol:
    """
    This class provides access to the glycol property calculations inside EnergyPlus.
    For now, the only glycol name allowed is plain water.  This is because other fluids are only
    initialized when they are declared in the input file.  When calling through the API,
    there is no input file, so no other fluids are declared.  This is ripe for a refactor to enable
    additional fluids, but water will suffice for now.
    """

    def __init__(self, state: c_void_p, api: cdll, glycol_name: bytes):
        """
        Creates a new Glycol instance, should almost certainly always be called from the API's Functional class, not
        directly from user code.  To get a Glycol instance from client code, call api.functional.glycol(state, "name"),
        where state is an active EnergyPlus state returned from a call to `api.state_manager.new_state()`.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`
        :param api: An active CTYPES CDLL instance
        :param glycol_name: The name of the glycol to instantiate -- currently only "water" is supported.
        """
        self.api = api
        self.api.glycolNew.argtypes = [c_void_p, c_char_p]
        self.api.glycolNew.restype = c_void_p
        self.api.glycolDelete.argtypes = [c_void_p, c_void_p]
        self.api.glycolDelete.restype = c_void_p
        self.api.glycolSpecificHeat.argtypes = [c_void_p, c_void_p, RealEP]
        self.api.glycolSpecificHeat.restype = RealEP
        self.api.glycolDensity.argtypes = [c_void_p, c_void_p, RealEP]
        self.api.glycolDensity.restype = RealEP
        self.api.glycolConductivity.argtypes = [c_void_p, c_void_p, RealEP]
        self.api.glycolConductivity.restype = RealEP
        self.api.glycolViscosity.argtypes = [c_void_p, c_void_p, RealEP]
        self.api.glycolViscosity.restype = RealEP
        self.instance = self.api.glycolNew(state, glycol_name)

    def delete(self, state: c_void_p) -> None:
        """
        Frees the memory of the associated Glycol instance inside the EnergyPlus (C++) state.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: Nothing
        """
        self.api.glycolDelete(state, self.instance)

    def specific_heat(self, state: c_void_p, temperature: float) -> float:
        """
        Returns the specific heat of the fluid at the specified temperature.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`
        :param temperature: Fluid temperature, in degrees Celsius
        :return: The specific heat of the fluid, in J/kg-K
        """
        return self.api.glycolSpecificHeat(state, self.instance, temperature)

    def density(self, state: c_void_p, temperature: float) -> float:
        """
        Returns the density of the fluid at the specified temperature.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`
        :param temperature: Fluid temperature, in degrees Celsius
        :return: The density of the fluid, in kg/m3
        """
        return self.api.glycolDensity(state, self.instance, temperature)

    def conductivity(self, state: c_void_p, temperature: float) -> float:
        """
        Returns the conductivity of the fluid at the specified temperature.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`
        :param temperature: Fluid temperature, in degrees Celsius
        :return: The conductivity of the fluid, in W/m-K
        """
        return self.api.glycolConductivity(state, self.instance, temperature)

    def viscosity(self, state: c_void_p, temperature: float) -> float:
        """
        Returns the dynamic viscosity of the fluid at the specified temperature.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`
        :param temperature: Fluid temperature, in degrees Celsius
        :return: The dynamic viscosity of the fluid, in Pa-s (or kg/m-s)
        """
        return self.api.glycolViscosity(state, self.instance, temperature)


class Refrigerant:
    """
    This class provides access to the refrigerant property calculations inside EnergyPlus.
    For now, the only refrigerant  name allowed is steam.  This is because other refrigerants are only
    initialized when they are declared in the input file.  When calling through the API, there is no
    input file, so no other refrigerants are declared.  This should be improved through later enhancements,
    but steam will provide a suitable use case for now.
    """

    def __init__(self, state: c_void_p, api: cdll, refrigerant_name: bytes):
        """
        Creates a new Refrigerant instance, should almost certainly always be called from the API's functional class,
        not directly from user code.  To get a Refrigerant instance from client code, call
        api.functional.refrigerant(state, "name"), where state is an active EnergyPlus state returned from a call to
        `api.state_manager.new_state()`.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`
        :param api: An active CTYPES CDLL instance
        :param refrigerant_name: The name of the refrigerant to instantiate -- currently only "steam" is supported.
        """
        self.refrigerant_name = refrigerant_name
        self.api = api
        self.api.refrigerantNew.argtypes = [c_void_p, c_char_p]
        self.api.refrigerantNew.restype = c_void_p
        self.api.refrigerantDelete.argtypes = [c_void_p, c_void_p]
        self.api.refrigerantDelete.restype = c_void_p
        self.api.refrigerantSaturationPressure.argtypes = [c_void_p, c_void_p, RealEP]
        self.api.refrigerantSaturationPressure.restype = RealEP
        self.api.refrigerantSaturationTemperature.argtypes = [c_void_p, c_void_p, RealEP]
        self.api.refrigerantSaturationTemperature.restype = RealEP
        self.api.refrigerantSaturatedEnthalpy.argtypes = [c_void_p, c_void_p, RealEP, RealEP]
        self.api.refrigerantSaturatedEnthalpy.restype = RealEP
        self.api.refrigerantSaturatedDensity.argtypes = [c_void_p, c_void_p, RealEP, RealEP]
        self.api.refrigerantSaturatedDensity.restype = RealEP
        self.api.refrigerantSaturatedSpecificHeat.argtypes = [c_void_p, c_void_p, RealEP, RealEP]
        self.api.refrigerantSaturatedSpecificHeat.restype = RealEP
        self.instance = self.api.refrigerantNew(state, refrigerant_name)

    def delete(self, state: c_void_p):
        """
        Frees the memory of the associated Refrigerant instance inside the EnergyPlus (C++) state.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: Nothing
        """
        self.api.refrigerantDelete(state, self.instance)

    def saturation_pressure(self, state: c_void_p, temperature: float) -> float:
        """
        Returns the saturation pressure of the refrigerant at the specified temperature.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param temperature: Refrigerant temperature, in Celsius.
        :return: Refrigerant saturation pressure, in Pa
        """
        return self.api.refrigerantSaturationPressure(state, self.instance, temperature)

    def saturation_temperature(self, state: c_void_p, pressure: float) -> float:
        """
        Returns the saturation temperature of the refrigerant at the specified pressure.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param pressure: Refrigerant pressure, in Pa
        :return: Refrigerant saturation temperature, in Celsius
        """
        return self.api.refrigerantSaturationTemperature(state, self.instance, pressure)

    def saturated_enthalpy(self, state: c_void_p, temperature: float, quality: float) -> float:
        """
        Returns the refrigerant saturated enthalpy at the specified temperature and quality.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param temperature: Refrigerant temperature, in Celsius
        :param quality: Refrigerant quality, in fractional form from 0.0 to 1.0
        :return: Refrigerant saturated enthalpy, in J/kg
        """
        return self.api.refrigerantSaturatedEnthalpy(state, self.instance, temperature, quality)

    def saturated_density(self, state: c_void_p, temperature: float, quality: float) -> float:
        """
        Returns the refrigerant density at the specified temperature and quality.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param temperature: Refrigerant temperature, in Celsius
        :param quality: Refrigerant quality, in fractional form from 0.0 to 1.0
        :return: Refrigerant saturated density, in kg/m3
        """
        return self.api.refrigerantSaturatedDensity(state, self.instance, temperature, quality)

    def saturated_specific_heat(self, state: c_void_p, temperature: float, quality: float) -> float:
        """
        Returns the refrigerant specific heat at the specified temperature and quality.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param temperature: Refrigerant temperature, in Celsius
        :param quality: Refrigerant quality, in fractional form from 0.0 to 1.0
        :return: Refrigerant saturated specific heat, in J/kg-K
        """
        return self.api.refrigerantSaturatedSpecificHeat(state, self.instance, temperature, quality)


class Psychrometrics:
    """
    This class provides access to the psychrometric functions within EnergyPlus.  Some property calculations are
    available as functions of different independent variable combinations, leading to suffixed function names, such as
    `vapor_density_b` and `relative_humidity_c`.
    """

    def __init__(self, api: cdll):
        """
        Creates a new Psychrometrics instance, should almost certainly always be called from the API's functional class,
        not directly from user code.  To get a Psychrometrics instance from client code, call
        api.functional.psychrometrics(state), where state is an active EnergyPlus state returned from a call to
        `api.state_manager.new_state()`.

        :param api: An active CTYPES CDLL instance
        """
        self.api = api
        self.api.psyRhoFnPbTdbW.argtypes = [c_void_p, RealEP, RealEP, RealEP]
        self.api.psyRhoFnPbTdbW.restype = RealEP
        self.api.psyHfgAirFnWTdb.argtypes = [c_void_p, RealEP]
        self.api.psyHfgAirFnWTdb.restype = RealEP
        self.api.psyHgAirFnWTdb.argtypes = [c_void_p, RealEP]
        self.api.psyHgAirFnWTdb.restype = RealEP
        self.api.psyHFnTdbW.argtypes = [c_void_p, RealEP, RealEP]
        self.api.psyHFnTdbW.restype = RealEP
        self.api.psyCpAirFnW.argtypes = [c_void_p, RealEP]
        self.api.psyCpAirFnW.restype = RealEP
        self.api.psyTdbFnHW.argtypes = [c_void_p, RealEP, RealEP]
        self.api.psyTdbFnHW.restype = RealEP
        self.api.psyRhovFnTdbWPb.argtypes = [c_void_p, RealEP, RealEP, RealEP]
        self.api.psyRhovFnTdbWPb.restype = RealEP
        self.api.psyTwbFnTdbWPb.argtypes = [c_void_p, RealEP, RealEP, RealEP]
        self.api.psyTwbFnTdbWPb.restype = RealEP
        self.api.psyVFnTdbWPb.argtypes = [c_void_p, RealEP, RealEP, RealEP]
        self.api.psyVFnTdbWPb.restype = RealEP
        self.api.psyWFnTdbH.argtypes = [c_void_p, RealEP, RealEP]
        self.api.psyWFnTdbH.restype = RealEP
        self.api.psyPsatFnTemp.argtypes = [c_void_p, RealEP]
        self.api.psyPsatFnTemp.restype = RealEP
        self.api.psyTsatFnHPb.argtypes = [c_void_p, RealEP, RealEP]
        self.api.psyTsatFnHPb.restype = RealEP
        self.api.psyRhovFnTdbRh.argtypes = [c_void_p, RealEP, RealEP]
        self.api.psyRhovFnTdbRh.restype = RealEP
        self.api.psyRhFnTdbRhov.argtypes = [c_void_p, RealEP, RealEP]
        self.api.psyRhFnTdbRhov.restype = RealEP
        self.api.psyRhFnTdbWPb.argtypes = [c_void_p, RealEP, RealEP, RealEP]
        self.api.psyRhFnTdbWPb.restype = RealEP
        self.api.psyWFnTdpPb.argtypes = [c_void_p, RealEP, RealEP]
        self.api.psyWFnTdpPb.restype = RealEP
        self.api.psyWFnTdbRhPb.argtypes = [c_void_p, RealEP, RealEP, RealEP]
        self.api.psyWFnTdbRhPb.restype = RealEP
        self.api.psyWFnTdbTwbPb.argtypes = [c_void_p, RealEP, RealEP, RealEP]
        self.api.psyWFnTdbTwbPb.restype = RealEP
        self.api.psyHFnTdbRhPb.argtypes = [c_void_p, RealEP, RealEP, RealEP]
        self.api.psyHFnTdbRhPb.restype = RealEP
        self.api.psyTdpFnWPb.argtypes = [c_void_p, RealEP, RealEP]
        self.api.psyTdpFnWPb.restype = RealEP
        self.api.psyTdpFnTdbTwbPb.argtypes = [c_void_p, RealEP, RealEP, RealEP]
        self.api.psyTdpFnTdbTwbPb.restype = RealEP

    def density(
        self, state: c_void_p, barometric_pressure: float, dry_bulb_temp: float, humidity_ratio: float
    ) -> float:
        """
        Returns the psychrometric density at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param barometric_pressure: Barometric pressure, in Pa
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :param humidity_ratio: Humidity ratio, in kgWater/kgDryAir
        :return:
        """
        return self.api.psyRhoFnPbTdbW(state, barometric_pressure, dry_bulb_temp, humidity_ratio)

    def latent_energy_of_air(self, state: c_void_p, dry_bulb_temp: float) -> float:
        """
        Returns the psychrometric latent energy of air at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :return:
        """
        return self.api.psyHfgAirFnWTdb(state, dry_bulb_temp)

    def latent_energy_of_moisture_in_air(self, state: c_void_p, dry_bulb_temp: float) -> float:
        """
        Returns the psychrometric latent energy of the moisture in air at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :return:
        """
        return self.api.psyHgAirFnWTdb(state, dry_bulb_temp)

    def enthalpy(self, state: c_void_p, dry_bulb_temp: float, humidity_ratio: float) -> float:
        """
        Returns the psychrometric enthalpy at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :param humidity_ratio: Humidity ratio, in kgWater/kgDryAir
        :return:
        """
        return self.api.psyHFnTdbW(state, dry_bulb_temp, humidity_ratio)

    def enthalpy_b(
        self, state: c_void_p, dry_bulb_temp: float, relative_humidity_fraction: float, barometric_pressure: float
    ) -> float:
        """
        Returns the psychrometric enthalpy at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :param relative_humidity_fraction: Psychrometric relative humidity, as a fraction from 0.0 to 1.0.
        :param barometric_pressure: Barometric pressure, in Pa
        :return:
        """
        return self.api.psyHFnTdbRhPb(state, dry_bulb_temp, relative_humidity_fraction, barometric_pressure)

    def specific_heat(self, state: c_void_p, humidity_ratio: float) -> float:
        """
        Returns the psychrometric specific heat at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param humidity_ratio: Humidity ratio, in kgWater/kgDryAir
        :return:
        """
        return self.api.psyCpAirFnW(state, humidity_ratio)

    def dry_bulb(self, state: c_void_p, enthalpy: float, humidity_ratio: float) -> float:
        """
        Returns the psychrometric dry bulb temperature at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param enthalpy: Psychrometric enthalpy, in J/kg
        :param humidity_ratio: Humidity ratio, in kgWater/kgDryAir
        :return:
        """
        return self.api.psyTdbFnHW(state, enthalpy, humidity_ratio)

    def vapor_density(
        self, state: c_void_p, dry_bulb_temp: float, humidity_ratio: float, barometric_pressure: float
    ) -> float:
        """
        Returns the psychrometric vapor density at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :param humidity_ratio: Humidity ratio, in kgWater/kgDryAir
        :param barometric_pressure: Barometric pressure, in Pa
        :return:
        """
        return self.api.psyRhovFnTdbWPb(state, dry_bulb_temp, humidity_ratio, barometric_pressure)

    def relative_humidity(self, state: c_void_p, dry_bulb_temp: float, vapor_density: float) -> float:
        """
        Returns the psychrometric relative humidity at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :param vapor_density: Psychrometric vapor density, in kg/m3
        :return:
        """
        return self.api.psyRhFnTdbRhov(state, dry_bulb_temp, vapor_density)

    def relative_humidity_b(
        self, state: c_void_p, dry_bulb_temp: float, humidity_ratio: float, barometric_pressure: float
    ) -> float:
        """
        Returns the psychrometric relative humidity at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :param humidity_ratio: Humidity ratio, in kgWater/kgDryAir
        :param barometric_pressure: Barometric pressure, in Pa
        :return:
        """
        return self.api.psyRhFnTdbWPb(state, dry_bulb_temp, humidity_ratio, barometric_pressure)

    def wet_bulb(
        self, state: c_void_p, dry_bulb_temp: float, humidity_ratio: float, barometric_pressure: float
    ) -> float:
        """
        Returns the psychrometric wet bulb temperature at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :param humidity_ratio: Humidity ratio, in kgWater/kgDryAir
        :param barometric_pressure: Barometric pressure, in Pa
        :return:
        """
        return self.api.psyTwbFnTdbWPb(state, dry_bulb_temp, humidity_ratio, barometric_pressure)

    def specific_volume(
        self, state: c_void_p, dry_bulb_temp: float, humidity_ratio: float, barometric_pressure: float
    ) -> float:
        """
        Returns the psychrometric specific volume at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :param humidity_ratio: Humidity ratio, in kgWater/kgDryAir
        :param barometric_pressure: Barometric pressure, in Pa
        :return:
        """
        return self.api.psyVFnTdbWPb(state, dry_bulb_temp, humidity_ratio, barometric_pressure)

    def saturation_pressure(self, state: c_void_p, dry_bulb_temp: float) -> float:
        """
        Returns the psychrometric saturation pressure at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :return:
        """
        return self.api.psyPsatFnTemp(state, dry_bulb_temp)

    def saturation_temperature(self, state: c_void_p, enthalpy: float, barometric_pressure: float) -> float:
        """
        Returns the psychrometric saturation temperature at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param enthalpy: Psychrometric enthalpy, in J/kg
        :param barometric_pressure: Barometric pressure, in Pa
        :return:
        """
        return self.api.psyTsatFnHPb(state, enthalpy, barometric_pressure)

    def vapor_density_b(self, state: c_void_p, dry_bulb_temp: float, relative_humidity_fraction: float) -> float:
        """
        Returns the psychrometric vapor density at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :param relative_humidity_fraction: Psychrometric relative humidity, as a fraction from 0.0 to 1.0.
        :return:
        """
        return self.api.psyRhovFnTdbRh(state, dry_bulb_temp, relative_humidity_fraction)

    def humidity_ratio(self, state: c_void_p, dry_bulb_temp: float, enthalpy: float) -> float:
        """
        Returns the psychrometric humidity ratio at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :param enthalpy: Psychrometric enthalpy, in J/kg
        :return:
        """
        return self.api.psyWFnTdbH(state, dry_bulb_temp, enthalpy)

    def humidity_ratio_b(self, state: c_void_p, dew_point_temp: float, barometric_pressure: float) -> float:
        """
        Returns the psychrometric humidity ratio at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dew_point_temp: Psychrometric dew point temperature, in Celsius
        :param barometric_pressure: Barometric pressure, in Pa
        :return:
        """
        return self.api.psyWFnTdpPb(state, dew_point_temp, barometric_pressure)

    def humidity_ratio_c(
        self, state: c_void_p, dry_bulb_temp: float, relative_humidity_fraction: float, barometric_pressure: float
    ) -> float:
        """
        Returns the psychrometric humidity ratio at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :param relative_humidity_fraction: Psychrometric relative humidity, as a fraction from 0.0 to 1.0.
        :param barometric_pressure: Barometric pressure, in Pa
        :return:
        """
        return self.api.psyWFnTdbRhPb(state, dry_bulb_temp, relative_humidity_fraction, barometric_pressure)

    def humidity_ratio_d(
        self, state: c_void_p, dry_bulb_temp: float, wet_bulb_temp: float, barometric_pressure: float
    ) -> float:
        """
        Returns the psychrometric humidity ratio at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :param wet_bulb_temp: Psychrometric wet bulb temperature, in C
        :param barometric_pressure: Barometric pressure, in Pa
        :return:
        """
        return self.api.psyWFnTdbTwbPb(state, dry_bulb_temp, wet_bulb_temp, barometric_pressure)

    def dew_point(self, state: c_void_p, humidity_ratio: float, barometric_pressure: float) -> float:
        """
        Returns the psychrometric dew point temperature at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param humidity_ratio: Humidity ratio, in kgWater/kgDryAir
        :param barometric_pressure: Barometric pressure, in Pa
        :return:
        """
        return self.api.psyTdpFnWPb(state, humidity_ratio, barometric_pressure)

    def dew_point_b(
        self, state: c_void_p, dry_bulb_temp: float, wet_bulb_temp: float, barometric_pressure: float
    ) -> float:
        """
        Returns the psychrometric dew point temperature at the specified conditions.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param dry_bulb_temp: Psychrometric dry bulb temperature, in C
        :param wet_bulb_temp: Psychrometric wet bulb temperature, in C
        :param barometric_pressure: Barometric pressure, in Pa
        :return:
        """
        return self.api.psyTdpFnTdbTwbPb(state, dry_bulb_temp, wet_bulb_temp, barometric_pressure)


class EnergyPlusVersion:
    """
    This is the EnergyPlus version.  Could also call into the DLL but it's the same effect.
    """

    def __init__(self):
        self.ep_version_major = int("${CMAKE_VERSION_MAJOR}")
        self.ep_version_minor = int("${CMAKE_VERSION_MINOR}")
        self.ep_version_patch = int("${CMAKE_VERSION_PATCH}")
        self.ep_version_build = str("${CMAKE_VERSION_BUILD}")

    def __str__(self) -> str:
        """
        Returns a string representation of this EnergyPlus version.
        :return: EnergyPlus version, as major.minor.patch-build
        """
        return "%s.%s.%s-%s" % (
            self.ep_version_major,
            self.ep_version_minor,
            self.ep_version_patch,
            self.ep_version_build,
        )


class Functional:
    """
    This API class enables accessing structures and functionality inside EnergyPlus from an outside client.
    This functional API will be extended over time, but initial targeted functionality includes fluid and refrigerant
    property methods, and surface and geometry classes and methods.

    The Functional API class itself is really just an organizational class that provides access to nested functional
    classes through member functions.  The functional API class is instantiated by the higher level EnergyPlusAPI class,
    and clients should *never* attempt to create an instance manually.  Instead, create an EnergyPlusAPI instance, and
    use the `functional` member variable to access a Functional class instance.  For Python Plugin workflows, the
    EnergyPlusPlugin base class also provides an instance of the Functional base class through the `self.api.functional`
    member variable.  Clients should use that directly when needing to make functional calls into the library.
    """

    def __init__(self, api: cdll, running_as_python_plugin: bool = False):
        self.api = api
        self.api.initializeFunctionalAPI.argtypes = [c_void_p]
        self.api.initializeFunctionalAPI.restype = c_void_p
        self.initialized = False
        self.plugin_mode = running_as_python_plugin
        self.py_error_callback_type = CFUNCTYPE(c_void_p, c_int, c_char_p)
        self.api.registerErrorCallback.argtypes = [c_void_p, self.py_error_callback_type]
        self.api.registerErrorCallback.restype = c_void_p
        self.api.energyPlusVersion.argtypes = []
        self.api.energyPlusVersion.restype = c_char_p

    def initialize(self, state: c_void_p) -> None:
        if not self.initialized and not self.plugin_mode:
            self.api.initializeFunctionalAPI(state)
            self.initialized = True

    def glycol(self, state: c_void_p, glycol_name: str) -> Glycol:
        """
        Returns a Glycol instance, which allows calculation of glycol properties.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param glycol_name: Name of the Glycol, for now only water is allowed
        :return: An instantiated Glycol structure
        """
        self.initialize(state)
        if isinstance(glycol_name, str):
            glycol_name = glycol_name.encode("utf-8")
        return Glycol(state, self.api, glycol_name)

    def refrigerant(self, state: c_void_p, refrigerant_name: str) -> Refrigerant:
        """
        Returns a Refrigerant instance, which allows calculation of refrigerant properties.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param refrigerant_name: Name of the Refrigerant, for now only steam is allowed
        :return: An instantiated Refrigerant structure
        """
        self.initialize(state)
        if isinstance(refrigerant_name, str):
            refrigerant_name = refrigerant_name.encode("utf-8")
        return Refrigerant(state, self.api, refrigerant_name)

    def psychrometrics(self, state: c_void_p) -> Psychrometrics:
        """
        Returns a Psychrometric instance, which allows calculation of psychrometric properties.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :return: An instantiated Psychrometric structure
        """
        self.initialize(state)
        return Psychrometrics(self.api)

    @staticmethod
    def ep_version() -> EnergyPlusVersion:
        return EnergyPlusVersion()

    # TODO: Move this to the runtime API section?
    def callback_error(self, state, f: FunctionType) -> None:
        """
        This function allows a client to register a function to be called back by EnergyPlus when an error message
        is added to the error file.  The user can then detect specific error messages or whatever.

        :param state: An active EnergyPlus "state" that is returned from a call to `api.state_manager.new_state()`.
        :param f: A python function which takes an integer severity and a string (bytes) argument and returns nothing
        :return: Nothing
        """
        cb_ptr = self.py_error_callback_type(f)
        error_callbacks.append(cb_ptr)
        self.api.registerErrorCallback(state, cb_ptr)

    @staticmethod
    def clear_callbacks() -> None:
        """
        This function is only used if you are running this script continually making many calls into the E+ library in
        one thread, each with many new and different error handler callbacks, and you need to clean up.

        Note this will affect all current instances in this thread, so use carefully!

        :return: Nothing
        """
        error_callbacks.clear()
