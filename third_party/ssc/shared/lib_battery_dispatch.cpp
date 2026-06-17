/**
BSD-3-Clause
Copyright 2019 Alliance for Sustainable Energy, LLC
Redistribution and use in source and binary forms, with or without modification, are permitted provided
that the following conditions are met :
1.	Redistributions of source code must retain the above copyright notice, this list of conditions
and the following disclaimer.
2.	Redistributions in binary form must reproduce the above copyright notice, this list of conditions
and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.	Neither the name of the copyright holder nor the names of its contributors may be used to endorse
or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER, CONTRIBUTORS, UNITED STATES GOVERNMENT OR UNITED STATES
DEPARTMENT OF ENERGY, NOR ANY OF THEIR EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "lib_battery_dispatch.h"
#include "lib_battery_powerflow.h"
#include "lib_shared_inverter.h"

#include <math.h>
#include <algorithm>
#include <cstdint>

/*
Dispatch base class
*/
dispatch_t::dispatch_t(battery_t* Battery, double dt_hour, double SOC_min, double SOC_max, int current_choice, double Ic_max, double Id_max,
    double Pc_max_kwdc, double Pd_max_kwdc, double Pc_max_kwac, double Pd_max_kwac,
    double t_min, int mode, int battMeterPosition)
{
    // initialize battery power flow
    std::unique_ptr<BatteryPowerFlow> tmp(new BatteryPowerFlow(dt_hour));
    m_batteryPowerFlow = std::move(tmp);
    m_batteryPower = m_batteryPowerFlow->getBatteryPower();
    m_batteryPower->currentChargeMax = Ic_max;
    m_batteryPower->currentDischargeMax = Id_max;
    m_batteryPower->stateOfChargeMax = SOC_max;
    m_batteryPower->stateOfChargeMin = SOC_min;
    m_batteryPower->depthOfDischargeMax = SOC_max - SOC_min;
    m_batteryPower->powerBatteryChargeMaxDC = Pc_max_kwdc;
    m_batteryPower->powerBatteryDischargeMaxDC = Pd_max_kwdc;
    m_batteryPower->powerBatteryChargeMaxAC = Pc_max_kwac;
    m_batteryPower->powerBatteryDischargeMaxAC = Pd_max_kwac;
    m_batteryPower->meterPosition = battMeterPosition;

    // initalize Battery and a copy of the Battery for iteration
    _Battery = Battery;
    _Battery_initial = new battery_t(*_Battery);

    // Call the dispatch init method
    init(_Battery, dt_hour, current_choice, t_min, mode);
}

void dispatch_t::init(battery_t* Battery, double dt_hour, int current_choice, double t_min, int mode)
{
    _dt_hour = dt_hour;
    _current_choice = current_choice;
    _t_min = t_min;
    _mode = mode;

    // limit the switch from charging to discharge so that doesn't flip-flop subhourly
    _t_at_mode = 1000;
    _prev_charging = false;
    _charging = false;
    _e_max = Battery->V() * Battery->charge_maximum_lifetime() * util::watt_to_kilowatt * 0.01 * (m_batteryPower->stateOfChargeMax - m_batteryPower->stateOfChargeMin);
    _grid_recharge = false;

	// initialize powerflow model
	m_batteryPower->canClipCharge = false;
	m_batteryPower->canSystemCharge = false;
	m_batteryPower->canGridCharge = false;
	m_batteryPower->canDischarge = false;
}

// deep copy
dispatch_t::dispatch_t(const dispatch_t& dispatch)
{
    std::unique_ptr<BatteryPowerFlow> tmp(new BatteryPowerFlow(*dispatch.m_batteryPowerFlow));
    m_batteryPowerFlow = std::move(tmp);
    m_batteryPower = m_batteryPowerFlow->getBatteryPower();

    _Battery = new battery_t(*dispatch._Battery);
    _Battery_initial = new battery_t(*dispatch._Battery_initial);
    init(_Battery, dispatch._dt_hour, dispatch._current_choice, dispatch._t_min, dispatch._mode);
}

// shallow copy from dispatch to this
void dispatch_t::copy(const dispatch_t* dispatch)
{
    _Battery->set_state(dispatch->_Battery->get_state());
    _Battery_initial->set_state(dispatch->_Battery_initial->get_state());
    init(_Battery, dispatch->_dt_hour, dispatch->_current_choice, dispatch->_t_min, dispatch->_mode);

    // can't create shallow copy of unique ptr
    std::unique_ptr<BatteryPowerFlow> tmp(new BatteryPowerFlow(*dispatch->m_batteryPowerFlow));
    m_batteryPowerFlow = std::move(tmp);
    m_batteryPower = m_batteryPowerFlow->getBatteryPower();
}
void dispatch_t::delete_clone()
{
    // need to delete both, since allocated memory for both in deep copy
    if (_Battery) {
        delete _Battery;
    }
    if (_Battery_initial) {
        delete _Battery_initial;
        _Battery_initial = nullptr;
    }
}
dispatch_t::~dispatch_t()
{
    // original _Battery doesn't need deleted, since was a pointer passed in
    delete _Battery_initial;
}
void dispatch_t::finalize(size_t idx, double& I)
{
	_Battery->set_state(_Battery_initial->get_state());
	m_batteryPower->powerBatteryDC = 0;
	m_batteryPower->powerBatteryAC = 0;
	m_batteryPower->powerGridToBattery = 0;
	m_batteryPower->powerBatteryToGrid = 0;
	m_batteryPower->powerSystemToGrid = 0;
	_Battery->run(idx, I);
}

bool dispatch_t::check_constraints(double& I, size_t count)
{
    bool iterate;
    double I_initial = I;
    bool current_iterate = false;
    bool power_iterate = false;

    // don't allow any changes to violate current limits
    if (restrict_current(I))
    {
        current_iterate = true;
    }
    // don't allow violations of power limits
    else if (restrict_power(I))
    {
        power_iterate = true;
    }
    // decrease the current draw if took too much
    if (I > 0 && _Battery->SOC() < m_batteryPower->stateOfChargeMin - tolerance)
    {
        m_batteryPower->powerBatteryTarget = _Battery_initial->calculate_max_discharge_kw(&I);
    }
    // decrease the current charging if charged too much
    else if (I < 0 && _Battery->SOC() > m_batteryPower->stateOfChargeMax + tolerance)
    {
        m_batteryPower->powerBatteryTarget = _Battery_initial->calculate_max_charge_kw(&I);
    }
    // Don't allow grid charging unless explicitly allowed (reduce charging)
    if (!m_batteryPower->canGridCharge && I < 0 && m_batteryPower->powerGridToBattery > tolerance)
    {
        m_batteryPower->powerBatteryTarget += m_batteryPower->powerGridToBattery;
        I = _Battery->calculate_current_for_power_kw(m_batteryPower->powerBatteryTarget);
        m_batteryPower->powerGridToBattery = 0;
	}
	// Don't allow grid charging if producing PV
	else if (m_batteryPower->connectionMode == dispatch_t::DC_CONNECTED &&
		m_batteryPower->powerGridToBattery > 0 &&
		(m_batteryPower->powerSystemToGrid > 0 || m_batteryPower->powerSystemToLoad > 0))
	{
        m_batteryPower->powerBatteryTarget += m_batteryPower->powerGridToBattery;
        I = _Battery->calculate_current_for_power_kw(m_batteryPower->powerBatteryTarget);
    }
    // Error checking for battery charging
    double power_to_batt = m_batteryPower->powerBatteryDC;
	if (m_batteryPower->connectionMode == dispatch_t::DC_CONNECTED){
	    power_to_batt = -(m_batteryPower->powerSystemToBattery + m_batteryPower->powerFuelCellToBattery);
	    if (m_batteryPower->sharedInverter->powerDC_kW < 0)
	        power_to_batt += m_batteryPower->sharedInverter->powerDC_kW;    // charging from grid
	    power_to_batt *= m_batteryPower->singlePointEfficiencyDCToDC;
	    // if error is from from numerical solution, may not need to adjust battery
	}
	else {
	    power_to_batt = -(m_batteryPower->powerSystemToBattery + m_batteryPower->powerGridToBattery + m_batteryPower->powerFuelCellToBattery);
	    power_to_batt *= m_batteryPower->singlePointEfficiencyACToDC;
    }

    if (m_batteryPower->powerBatteryTarget < 0 && abs(power_to_batt - m_batteryPower->powerBatteryTarget) > 0.005 * fabs(power_to_batt)) {
        m_batteryPower->powerBatteryTarget = power_to_batt;
        m_batteryPower->powerBatteryDC = m_batteryPower->powerBatteryTarget;
        I = _Battery_initial->calculate_current_for_power_kw(m_batteryPower->powerBatteryTarget);
    }
    // Don't allow battery to discharge if it gets wasted due to inverter efficiency limitations
    // Typically, this would be due to low power flow, so just cut off battery.
    if (m_batteryPower->connectionMode == dispatch_t::DC_CONNECTED && m_batteryPower->sharedInverter->efficiencyAC < m_batteryPower->inverterEfficiencyCutoff)
    {
        // The requested DC power
        double powerBatterykWdc = _Battery->I() * _Battery->V() * util::watt_to_kilowatt;

		// if battery discharging, see if can back off to get higher efficiency
		if (m_batteryPower->powerBatteryDC > 0) {
			if (powerBatterykWdc + m_batteryPower->powerSystem > m_batteryPower->sharedInverter->getACNameplateCapacitykW()) {
				powerBatterykWdc = m_batteryPower->sharedInverter->getACNameplateCapacitykW() - m_batteryPower->powerSystem;
				powerBatterykWdc = fmax(powerBatterykWdc, 0);
                m_batteryPower->powerBatteryTarget = powerBatterykWdc;
                I = _Battery->calculate_current_for_power_kw(m_batteryPower->powerBatteryTarget);
            }
        }
        // if charging, this will also be due to low powerflow from grid-charging, just cut off that component
        else if (m_batteryPower->powerBatteryDC < 0 && m_batteryPower->powerGridToBattery > 0) {
            I *= fmax(1.0 - fabs(m_batteryPower->powerGridToBattery * m_batteryPower->sharedInverter->efficiencyAC * 0.01 / m_batteryPower->powerBatteryDC), 0);
            m_batteryPower->powerBatteryTarget = _Battery->calculate_voltage_for_current(I) * I * util::watt_to_kilowatt;
        }
    }

    iterate = abs(I_initial - I) > tolerance;

    // update constraints for current, power, if they are now violated
    if (!current_iterate) {
        current_iterate = restrict_current(I);
    }
    if (!power_iterate) {
        power_iterate = restrict_power(I);
    }

    // iterate if any of the conditions are met
    if (iterate || current_iterate || power_iterate)
        iterate = true;

    // stop iterating after n tries
    if (count > battery_dispatch::constraintCount)
        iterate = false;

    // don't allow battery to flip from charging to discharging or vice versa
    if (fabs(I) > tolerance && (I_initial / I) < 0) {
        I = 0;
        iterate = false;
    }


    // reset
    if (iterate)
    {
        _Battery->set_state(_Battery_initial->get_state());
        m_batteryPowerFlow->calculate();
    }

    return iterate;
}
void dispatch_t::SOC_controller()
{
    _charging = _prev_charging;

    // Implement minimum SOC cut-off
    if (m_batteryPower->powerBatteryDC > 0)
    {
        if (_Battery->SOC() <= m_batteryPower->stateOfChargeMin + tolerance) {
            m_batteryPower->powerBatteryDC = 0;
        }
        else {
            _charging = false;
        }
    }
    // Maximum SOC cut-off
    else if (m_batteryPower->powerBatteryDC < 0)
    {
        if (_Battery->SOC() >= m_batteryPower->stateOfChargeMax - tolerance) {
            m_batteryPower->powerBatteryDC = 0;
        }
        else {
            _charging = true;
        }
    }
}

void dispatch_t::switch_controller()
{
    // Implement rapid switching check
    if (_charging != _prev_charging)
    {
        if (_t_at_mode <= _t_min)
        {
            m_batteryPower->powerBatteryDC = 0.;
            _charging = _prev_charging;
        }
        else
            _t_at_mode = 0;
    }
    _t_at_mode += (int)(round(_dt_hour * util::hour_to_min));
}
double dispatch_t::current_controller(double power_kw)
{
    double I = _Battery->calculate_current_for_power_kw(power_kw);
    restrict_current(I);
    return I;
}
bool dispatch_t::restrict_current(double& I)
{
    bool iterate = false;
    if (_current_choice == RESTRICT_CURRENT || _current_choice == RESTRICT_BOTH)
    {
        if (I < 0)
        {
            if (fabs(I) > m_batteryPower->currentChargeMax)
            {
                I = -m_batteryPower->currentChargeMax;
                iterate = true;
            }
        }
        else
        {
            if (I > m_batteryPower->currentDischargeMax)
            {
                I = m_batteryPower->currentDischargeMax;
                iterate = true;
            }
        }
    }
    return iterate;
}
bool dispatch_t::restrict_power(double& I)
{
    bool iterate = false;
    if (_current_choice == RESTRICT_POWER || _current_choice == RESTRICT_BOTH)
    {
        double powerBattery = I * _Battery->V() * util::watt_to_kilowatt;
        double powerBatteryAC = powerBattery;
        if (powerBattery < 0)
            powerBatteryAC = powerBattery / m_batteryPower->singlePointEfficiencyACToDC;
        else if (powerBattery > 0)
            powerBatteryAC = powerBattery * m_batteryPower->singlePointEfficiencyDCToAC;

        double dP = 0.;

        // charging
        if (powerBattery < 0)
        {
            if (fabs(powerBattery) > m_batteryPower->powerBatteryChargeMaxDC * (1 + low_tolerance))
            {
                dP = fabs(m_batteryPower->powerBatteryChargeMaxDC - fabs(powerBattery));

                // increase (reduce) charging magnitude by percentage
                I -= (dP / fabs(powerBattery)) * I;
                iterate = true;
            }
            else if (m_batteryPower->connectionMode == m_batteryPower->AC_CONNECTED &&
                fabs(powerBatteryAC) > m_batteryPower->powerBatteryChargeMaxAC * (1 + low_tolerance))
            {
                dP = fabs(m_batteryPower->powerBatteryChargeMaxAC - fabs(powerBatteryAC));

                // increase (reduce) charging magnitude by percentage
                I -= (dP / fabs(powerBattery)) * I;
                iterate = true;
            }
            // This could just be grid power since that's technically the only AC component.  But, limit all to this
            else if (m_batteryPower->connectionMode == m_batteryPower->DC_CONNECTED &&
                fabs(powerBatteryAC) > m_batteryPower->powerBatteryChargeMaxAC * (1 + low_tolerance))
            {
                dP = fabs(m_batteryPower->powerBatteryChargeMaxAC - fabs(powerBatteryAC));

                // increase (reduce) charging magnitude by percentage
                I -= (dP / fabs(powerBattery)) * I;
                iterate = true;
            }
        }
        else
        {
            if (fabs(powerBattery) > m_batteryPower->powerBatteryDischargeMaxDC * (1 + low_tolerance))
            {
                dP = fabs(m_batteryPower->powerBatteryDischargeMaxDC - powerBattery);

                // decrease discharging magnitude
                I -= (dP / fabs(powerBattery)) * I;
                iterate = true;
            }
            else if (fabs(powerBatteryAC) > m_batteryPower->powerBatteryDischargeMaxAC * (1 + low_tolerance))
            {
                dP = fabs(m_batteryPower->powerBatteryDischargeMaxAC - powerBatteryAC);

                // decrease discharging magnitude
                I -= (dP / fabs(powerBattery)) * I;
                iterate = true;
            }
        }
    }
    return iterate;
}

void dispatch_t::runDispatch(size_t year, size_t hour_of_year, size_t step)
{
    // Ensure the battery operates within the state-of-charge limits
    SOC_controller();

    // Ensure the battery isn't switching rapidly between charging and dischaging
    switch_controller();

    // Calculate current, and ensure the battery falls within the current limits
    double I = current_controller(m_batteryPower->powerBatteryDC);

    // Setup battery iteration
    _Battery_initial->set_state(_Battery->get_state());

    bool iterate = true;
    size_t count = 0;
    size_t lifetimeIndex = util::lifetimeIndex(year, hour_of_year, step, static_cast<size_t>(1 / _dt_hour));

    do {

        // Run Battery Model to update charge based on charge/discharge
        m_batteryPower->powerBatteryDC = _Battery->run(lifetimeIndex, I);
        m_batteryPower->powerSystemLoss = _Battery->getLoss();

        // Update power flow calculations, calculate AC power, and check the constraints
        m_batteryPowerFlow->calculate();
        iterate = check_constraints(I, count);

        // If current changed during last iteration of constraints checker, recalculate internal battery state
        if (!iterate) {
            finalize(lifetimeIndex, I);
            m_batteryPower->powerBatteryDC = I * _Battery->V() * util::watt_to_kilowatt;
        }
        else {
            _Battery->set_state(_Battery_initial->get_state());
        }
        count++;

    } while (iterate);

    // finalize AC power flow calculation and update for next step
    m_batteryPowerFlow->calculate();
    _prev_charging = _charging;
}

double dispatch_t::power_tofrom_battery() { return m_batteryPower->powerBatteryAC; }
double dispatch_t::power_tofrom_grid() { return m_batteryPower->powerGrid; }
double dispatch_t::power_gen() { return m_batteryPower->powerGeneratedBySystem; }
double dispatch_t::power_pv_to_load() { return m_batteryPower->powerSystemToLoad; }
double dispatch_t::power_battery_to_load() { return m_batteryPower->powerBatteryToLoad; }
double dispatch_t::power_grid_to_load() { return m_batteryPower->powerGridToLoad; }
double dispatch_t::power_fuelcell_to_load() { return m_batteryPower->powerFuelCellToLoad; }
double dispatch_t::power_pv_to_batt() { return m_batteryPower->powerSystemToBattery; }
double dispatch_t::power_grid_to_batt() { return m_batteryPower->powerGridToBattery; }
double dispatch_t::power_fuelcell_to_batt() { return m_batteryPower->powerFuelCellToBattery; }
double dispatch_t::power_pv_to_grid() { return m_batteryPower->powerSystemToGrid; }
double dispatch_t::power_battery_to_grid() { return m_batteryPower->powerBatteryToGrid; }
double dispatch_t::power_fuelcell_to_grid() { return m_batteryPower->powerFuelCellToGrid; }
double dispatch_t::power_conversion_loss() { return m_batteryPower->powerConversionLoss; }
double dispatch_t::power_system_loss() { return m_batteryPower->powerSystemLoss; }
double dispatch_t::battery_power_to_fill() { return _Battery->power_to_fill(m_batteryPower->stateOfChargeMax); }
double dispatch_t::battery_soc() { return _Battery->SOC(); }
BatteryPowerFlow * dispatch_t::getBatteryPowerFlow() { return m_batteryPowerFlow.get(); }
BatteryPower * dispatch_t::getBatteryPower() { return m_batteryPower; }

dispatch_automatic_t::dispatch_automatic_t(
	battery_t * Battery,
	double dt_hour,
	double SOC_min,
	double SOC_max,
	int current_choice,
	double Ic_max,
	double Id_max,
	double Pc_max_kwdc,
	double Pd_max_kwdc,
	double Pc_max_kwac,
	double Pd_max_kwac,
	double t_min,
	int dispatch_mode,
	int pv_dispatch,
	size_t nyears,
	size_t look_ahead_hours,
	double dispatch_update_frequency_hours,
	bool can_charge,
	bool can_clip_charge,
	bool can_grid_charge,
	bool can_fuelcell_charge,
    std::vector<double> battReplacementCostPerkWh,
    int battCycleCostChoice,
    std::vector<double> battCycleCost
	) : dispatch_t(Battery, dt_hour, SOC_min, SOC_max, current_choice, Ic_max, Id_max, Pc_max_kwdc, Pd_max_kwdc, Pc_max_kwac, Pd_max_kwac,

    t_min, dispatch_mode, pv_dispatch)
{

    _dt_hour = dt_hour;
    _dt_hour_update = dispatch_update_frequency_hours;

    _hour_last_updated = SIZE_MAX;

    _forecast_hours = look_ahead_hours;
    _steps_per_hour = (size_t)(1. / dt_hour);
    _num_steps = 24 * _steps_per_hour;

	_day_index = 0;
	_month = 1;
	_nyears = nyears;
    curr_year = 0;

    _mode = dispatch_mode;
    _safety_factor = 0.03;

	m_batteryPower->canClipCharge = can_clip_charge;
	m_batteryPower->canSystemCharge = can_charge;
	m_batteryPower->canGridCharge = can_grid_charge;
	m_batteryPower->canFuelCellCharge = can_fuelcell_charge;
	m_batteryPower->canDischarge = true;
    m_battReplacementCostPerKWH = battReplacementCostPerkWh;
    m_battCycleCostChoice = battCycleCostChoice;
    cycle_costs_by_year = battCycleCost;
}

void dispatch_automatic_t::init_with_pointer(const dispatch_automatic_t* tmp)
{
	_day_index = tmp->_day_index;
	_month = tmp->_month;
	_num_steps = tmp->_num_steps;
	_hour_last_updated = tmp->_hour_last_updated;
	_dt_hour = tmp->_dt_hour;
	_dt_hour_update = tmp->_dt_hour_update;
	_steps_per_hour = tmp->_steps_per_hour;
	_nyears = tmp->_nyears;
    curr_year = tmp->curr_year;
	_mode = tmp->_mode;
	_safety_factor = tmp->_safety_factor;
	_forecast_hours = tmp->_forecast_hours;
    m_battReplacementCostPerKWH = tmp->m_battReplacementCostPerKWH;
    m_battCycleCostChoice = tmp->m_battCycleCostChoice;
    m_cycleCost = tmp->m_cycleCost;
    cycle_costs_by_year = tmp->cycle_costs_by_year;
}

// deep copy from dispatch to this
dispatch_automatic_t::dispatch_automatic_t(const dispatch_t& dispatch) :
    dispatch_t(dispatch)
{
    const dispatch_automatic_t* tmp = dynamic_cast<const dispatch_automatic_t*>(&dispatch);
    init_with_pointer(tmp);
}

// shallow copy from dispatch to this
void dispatch_automatic_t::copy(const dispatch_t* dispatch)
{
    dispatch_t::copy(dispatch);
    const dispatch_automatic_t* tmp = dynamic_cast<const dispatch_automatic_t*>(dispatch);
    init_with_pointer(tmp);
}

void dispatch_automatic_t::update_pv_data(std::vector<double> P_pv_ac){ _P_pv_ac = P_pv_ac;}

void dispatch_automatic_t::update_cliploss_data(double_vec P_cliploss)
{
    _P_cliploss_dc = P_cliploss;

    // append to end to allow for look-ahead
    for (size_t i = 0; i != _forecast_hours * _steps_per_hour; i++)
        _P_cliploss_dc.push_back(P_cliploss[i]);
}
void dispatch_automatic_t::set_custom_dispatch(std::vector<double> P_batt_dc) { _P_battery_use = P_batt_dc; }
int dispatch_automatic_t::get_mode() { return _mode; }
double dispatch_automatic_t::power_batt_target() { return m_batteryPower->powerBatteryTarget; }

void dispatch_automatic_t::dispatch(size_t year,
    size_t hour_of_year,
    size_t step)
{
    runDispatch(year, hour_of_year, step);
}


bool dispatch_automatic_t::check_constraints(double& I, size_t count)
{
    // check common constraints before checking automatic dispatch specific ones
    bool iterate = dispatch_t::check_constraints(I, count);

    if (!iterate)
    {
        double I_initial = I;
        double P_battery = I * _Battery->V() * util::watt_to_kilowatt;
        double P_target = m_batteryPower->powerBatteryTarget;

        // Common to automated behind the meter and front of meter
        iterate = true;


		// Don't respect target if bidirectional inverter efficiency is low while charging
		if (m_batteryPower->connectionMode == dispatch_t::DC_CONNECTED &&
			m_batteryPower->sharedInverter->efficiencyAC <= m_batteryPower->inverterEfficiencyCutoff &&
			P_target < 0)
		{
			iterate = false;
			// Power adjustments were handled in dispatch_t::check_constraints, don't iterate, move along
		}

        // Try and force controller to meet target or custom dispatch
        else if (P_battery > P_target + tolerance || P_battery < P_target - tolerance)
        {
            // Difference between the dispatch and the desired dispatch
            double dP = P_battery - m_batteryPower->powerBatteryTarget;
            double SOC = _Battery->SOC();

            // Case 1: Charging, need to increase charging to meet target (P_battery < 0, dP > 0)
            if (P_battery <= 0 && dP > 0) {
                // Don't charge more if can't grid charge (assumes PV and fuel cell have met max possible)
                if (!m_batteryPower->canGridCharge) {
                    iterate = false;
                }
                // Don't charge more if battery is already close to full
                if (SOC > m_batteryPower->stateOfChargeMax - tolerance) {
                    iterate = false;
                }
                // Don't charge more if would violate current or power charge limits
                if (I > m_batteryPower->currentChargeMax - tolerance ||
                    fabs(P_battery) > m_batteryPower->powerBatteryChargeMaxDC - tolerance ||
                    fabs(m_batteryPower->powerBatteryAC) > m_batteryPower->powerBatteryChargeMaxAC - tolerance) {
                    iterate = false;
                }
                // restrict based on power limits
                else {
                    double dP_max = fmin(fmin(dP, m_batteryPower->powerBatteryChargeMaxDC - fabs(P_battery)),
                        m_batteryPower->powerBatteryChargeMaxAC - fabs(m_batteryPower->powerBatteryAC));
                    dP = fmax(dP_max, 0);
                }
            }
            // Case 2: Discharging, need to increase discharge to meet target (P_battery > 0, dP < 0)
            else if (P_battery > 0 && dP < 0) {
                // Don't discharge more if already near min SOC
                if (SOC < m_batteryPower->stateOfChargeMin + tolerance) {
                    iterate = false;
                }
                // Don't discharge more if would violate current or power discharge limits
                if (I > m_batteryPower->currentDischargeMax - tolerance ||
                    P_battery > m_batteryPower->powerBatteryDischargeMaxDC - tolerance ||
                    m_batteryPower->powerBatteryAC > m_batteryPower->powerBatteryDischargeMaxAC - tolerance) {
                    iterate = false;
                }
                // restrict based on power limits
                else {
                    double dP_max = fmax(fmax(dP, P_battery - m_batteryPower->powerBatteryDischargeMaxDC),
                        m_batteryPower->powerBatteryAC - m_batteryPower->powerBatteryChargeMaxAC);
                    dP = fmin(dP_max, 0);
                }
            }
            // Otherwise safe, (decreasing charging, decreasing discharging)
            double dQ = dP * _dt_hour * util::kilowatt_to_watt / _Battery->V();
            double dSOC = 100 * dQ / _Battery->charge_maximum_lifetime();

            if (iterate) {

				double dI = dP * util::kilowatt_to_watt / _Battery->V();
				if (SOC + dSOC > m_batteryPower->stateOfChargeMax + tolerance) {
					double dSOC_use = (m_batteryPower->stateOfChargeMax - SOC);
					double dQ_use = dSOC_use * 0.01 * _Battery->charge_maximum_lifetime();
					dI = dQ_use / _dt_hour;
				}
				else if (SOC + dSOC < m_batteryPower->stateOfChargeMin - tolerance) {
					double dSOC_use = (m_batteryPower->stateOfChargeMin - SOC);
					double dQ_use = dSOC_use * 0.01 * _Battery->charge_maximum_lifetime();
					dI = dQ_use / _dt_hour;
				}
				I -= dI;
			}
		}

		// Behind the meter
		if (m_batteryPower->meterPosition == dispatch_t::BEHIND)
		{
			// Don't let PV export to grid if can still charge battery (increase charging) (unless following custom dispatch)
			if (_mode != dispatch_t::CUSTOM_DISPATCH && m_batteryPower->powerSystemToGrid  > tolerance && m_batteryPower->canSystemCharge &&
                    _Battery->SOC() < m_batteryPower->stateOfChargeMax - tolerance && fabs(I) < fabs(m_batteryPower->currentChargeMax))
			{
				if (fabs(m_batteryPower->powerBatteryAC) < tolerance)
					I -= (m_batteryPower->powerSystemToGrid  * util::kilowatt_to_watt / _Battery->V());
				else
					I -= (m_batteryPower->powerSystemToGrid  / fabs(m_batteryPower->powerBatteryAC)) *fabs(I);
			}
			// Don't let battery export to the grid if behind the meter
			else if (m_batteryPower->powerBatteryToGrid > tolerance)
			{
                if (fabs(m_batteryPower->powerBatteryAC) < tolerance) {
                    I -= (m_batteryPower->powerBatteryToGrid * util::kilowatt_to_watt / _Battery->V());
                }
                else {
                    I -= (m_batteryPower->powerBatteryToGrid / fabs(m_batteryPower->powerBatteryAC)) * fabs(I);
                }
                m_batteryPower->powerBatteryTarget -= m_batteryPower->powerBatteryToGrid;
                m_batteryPower->powerBatteryAC -= m_batteryPower->powerBatteryToGrid; // Target was too large given PV, reduce
			}
			else
				iterate = false;
		}
		else
			iterate = false;

        // don't allow any changes to violate current limits
        bool current_iterate = restrict_current(I);

        // don't allow any changes to violate power limites
        bool power_iterate = restrict_power(I);

        // iterate if any of the conditions are met
        if (iterate || current_iterate || power_iterate)
            iterate = true;

        // stop iterating after n tries
        if (count > battery_dispatch::constraintCount)
            iterate = false;

        // don't allow battery to flip from charging to discharging or vice versa
        if ((I_initial / I) < 0)
            I = 0;

        // reset
        if (iterate)
        {
            _Battery->set_state(_Battery_initial->get_state());
            //			m_batteryPower->powerBatteryAC = 0;
            //			m_batteryPower->powerGridToBattery = 0;
            //			m_batteryPower->powerBatteryToGrid = 0;
            //			m_batteryPower->powerPVToGrid  = 0;
        }
    }
    return iterate;
}

battery_metrics_t::battery_metrics_t(double dt_hour)
{
    _dt_hour = dt_hour;

    // single value metrics
    _e_charge_accumulated = 0;
    _e_charge_from_pv = 0.;
    _e_charge_from_grid = _e_charge_accumulated; // assumes initial charge from grid
    _e_discharge_accumulated = 0.;
    _e_loss_system = 0.;
    _average_efficiency = 100.;
    _average_roundtrip_efficiency = 100.;
    _pv_charge_percent = 0.;

    // annual metrics
    _e_charge_from_pv_annual = 0.;
    _e_charge_from_grid_annual = _e_charge_from_grid;
    _e_charge_annual = _e_charge_accumulated;
    _e_discharge_annual = 0.;
    _e_loss_system_annual = _e_loss_system;
    _e_grid_import_annual = 0.;
    _e_grid_export_annual = 0.;
    _e_loss_annual = 0.;
}
double battery_metrics_t::average_battery_conversion_efficiency() { return _average_efficiency; }
double battery_metrics_t::average_battery_roundtrip_efficiency() { return _average_roundtrip_efficiency; }
double battery_metrics_t::pv_charge_percent() { return _pv_charge_percent; }
double battery_metrics_t::energy_pv_charge_annual() { return _e_charge_from_pv_annual; }
double battery_metrics_t::energy_grid_charge_annual() { return _e_charge_from_grid_annual; }
double battery_metrics_t::energy_charge_annual() { return _e_charge_annual; }
double battery_metrics_t::energy_discharge_annual() { return _e_discharge_annual; }
double battery_metrics_t::energy_grid_import_annual() { return _e_grid_import_annual; }
double battery_metrics_t::energy_grid_export_annual() { return _e_grid_export_annual; }
double battery_metrics_t::energy_loss_annual() { return _e_loss_annual; }
double battery_metrics_t::energy_system_loss_annual() { return _e_loss_system_annual; };

void battery_metrics_t::compute_metrics_ac(const BatteryPower* batteryPower)
{
	accumulate_grid_annual(batteryPower->powerGrid);
	accumulate_battery_charge_components(batteryPower->powerBatteryAC, batteryPower->powerSystemToBattery, batteryPower->powerGridToBattery);
	accumulate_energy_charge(batteryPower->powerBatteryAC);
	accumulate_energy_discharge(batteryPower->powerBatteryAC);
	accumulate_energy_system_loss(batteryPower->powerSystemLoss);
	compute_annual_loss();
}
void battery_metrics_t::compute_annual_loss()
{
    double e_conversion_loss = 0.;
    if (_e_charge_annual > _e_discharge_annual)
        e_conversion_loss = _e_charge_annual - _e_discharge_annual;
    _e_loss_annual = e_conversion_loss + _e_loss_system_annual;
}
void battery_metrics_t::accumulate_energy_charge(double P_tofrom_batt)
{
    if (P_tofrom_batt < 0.)
    {
        _e_charge_accumulated += (-P_tofrom_batt) * _dt_hour;
        _e_charge_annual += (-P_tofrom_batt) * _dt_hour;
    }
}
void battery_metrics_t::accumulate_energy_discharge(double P_tofrom_batt)
{
    if (P_tofrom_batt > 0.)
    {
        _e_discharge_accumulated += P_tofrom_batt * _dt_hour;
        _e_discharge_annual += P_tofrom_batt * _dt_hour;
    }
}
void battery_metrics_t::accumulate_energy_system_loss(double P_system_loss)
{
    _e_loss_system += P_system_loss * _dt_hour;
    _e_loss_system_annual += P_system_loss * _dt_hour;
}
void battery_metrics_t::accumulate_battery_charge_components(double P_tofrom_batt, double P_pv_to_batt, double P_grid_to_batt)
{
    if (P_tofrom_batt < 0.)
    {
        _e_charge_from_pv += P_pv_to_batt * _dt_hour;
        _e_charge_from_pv_annual += P_pv_to_batt * _dt_hour;
        _e_charge_from_grid += P_grid_to_batt * _dt_hour;
        _e_charge_from_grid_annual += P_grid_to_batt * _dt_hour;
    }
    _average_efficiency = 100. * (_e_discharge_accumulated / _e_charge_accumulated);
    _average_roundtrip_efficiency = 100. * (_e_discharge_accumulated / (_e_charge_accumulated + _e_loss_system));
    _pv_charge_percent = 100. * (_e_charge_from_pv / _e_charge_accumulated);
}
void battery_metrics_t::accumulate_grid_annual(double P_tofrom_grid)
{
    // e_grid > 0 (export to grid)
    // e_grid < 0 (import from grid)

    if (P_tofrom_grid > 0)
        _e_grid_export_annual += P_tofrom_grid * _dt_hour;
    else
        _e_grid_import_annual += (-P_tofrom_grid) * _dt_hour;
}

void battery_metrics_t::new_year()
{
    _e_charge_from_pv_annual = 0.;
    _e_charge_from_grid_annual = 0;
    _e_charge_annual = 0.;
    _e_discharge_annual = 0.;
    _e_grid_import_annual = 0.;
    _e_grid_export_annual = 0.;
    _e_loss_system_annual = 0.;
}

bool byGrid:: operator()(grid_point const& a, grid_point const& b)
{
    return a.Grid() > b.Grid();
}

bool byCost::operator() (grid_point const& a, grid_point const& b)
{
    if (a.Cost() == b.Cost())
    {
        return a.Grid() > b.Grid();
    }
    return a.Cost() > b.Cost();
}

bool byLowestMarginalCost::operator() (grid_point const& a, grid_point const& b)
{

    if (fabs(a.MarginalCost() - b.MarginalCost()) < 1e-7)
    {
        if (fabs(a.Grid()) < 1e-7 || fabs(b.Grid()) < 1e-7)
        {
            return a.Grid() < b.Grid();
        }
        else if (fabs((a.Cost() / a.Grid()) - (b.Cost() / b.Grid())) < 1e-7)
        {
            return a.Grid() < b.Grid();
        }
        return (a.Cost() / a.Grid()) < (b.Cost() / b.Grid());
    }

    return a.MarginalCost() < b.MarginalCost();

}
