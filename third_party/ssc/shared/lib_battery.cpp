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

#include <cmath>
#include <functional>

#include "lib_battery.h"

/*
Define Thermal Model
*/

void thermal_t::initialize() {
    if (params->en_cap_vs_temp && (params->cap_vs_temp.nrows() < 2 || params->cap_vs_temp.ncols() != 2)) {
        throw std::runtime_error("thermal_t: capacity vs temperature matrix must have two columns and at least two rows");
    }

    if (params->en_cap_vs_temp) {
        size_t n = params->cap_vs_temp.nrows();
        for (int i = 0; i < (int) n; i++) {
            params->cap_vs_temp(i, 0);
        }
    }

    state = std::make_shared<thermal_state>();
    if (params->option == thermal_params::SCHEDULE)
        state->T_room = params->T_room_schedule[0];
    else
        state->T_room = params->T_room_init;
    state->T_batt = state->T_room;

    state->T_batt_prev = state->T_room;
    state->heat_dissipated = 0;
    state->q_relative_thermal = 100;
    dt_sec = params->dt_hr * 3600;
}

thermal_t::thermal_t(double dt_hour, double mass, double surface_area, double R, double Cp, double h,
                     const util::matrix_t<double> &c_vs_t, std::vector<double> T_room_C) {
    params = std::shared_ptr<thermal_params>(new thermal_params({dt_hour, mass, surface_area, Cp, h, R, true, c_vs_t}));
    params->option = thermal_params::SCHEDULE;
    params->T_room_schedule = std::move(T_room_C);
    initialize();
    state->T_room = params->T_room_schedule[0];
}

thermal_t::thermal_t(double dt_hour, double mass, double surface_area, double R, double Cp, double h,
                     const util::matrix_t<double> &c_vs_t, double T_room_C) {
    params = std::shared_ptr<thermal_params>(new thermal_params({dt_hour, mass, surface_area, Cp, h, R, true, c_vs_t}));
    params->option = thermal_params::VALUE;
    params->T_room_init = T_room_C;
    initialize();
}

thermal_t::thermal_t(double dt_hour, double mass, double surface_area, double R, double Cp, double h,
                     double T_room_C) {
    params = std::shared_ptr<thermal_params>(new thermal_params({dt_hour, mass, surface_area, Cp, h, R, false, util::matrix_t<double>()}));
    params->option = thermal_params::VALUE;
    params->T_room_init = T_room_C;
    initialize();
}

thermal_t::thermal_t(double dt_hour, double mass, double surface_area, double R, double Cp, double h,
                     std::vector<double> T_room_C) {
    params = std::shared_ptr<thermal_params>(new thermal_params({ dt_hour, mass, surface_area, Cp, h, R, false, util::matrix_t<double>()}));
    params->option = thermal_params::SCHEDULE;
    params->T_room_schedule = std::move(T_room_C);
    initialize();
    state->T_room = params->T_room_schedule[0];
}

thermal_t::thermal_t(std::shared_ptr<thermal_params> p) {
    params = std::move(p);
    initialize();
}

thermal_t::thermal_t(const thermal_t &rhs) {
    params = std::make_shared<thermal_params>(*rhs.params);
    state = std::make_shared<thermal_state>(*rhs.state);
    dt_sec = rhs.dt_sec;
}

thermal_t &thermal_t::operator=(const thermal_t &rhs) {
    if (this != &rhs) {
        *params = *rhs.params;
        dt_sec = rhs.dt_sec;
        *state = *rhs.state;
    }
    return *this;
}

thermal_t *thermal_t::clone() { return new thermal_t(*this); }

void thermal_t::replace_battery(size_t lifetimeIndex) {
    if (params->option == thermal_params::VALUE)
        state->T_batt = params->T_room_schedule[lifetimeIndex % params->T_room_schedule.size()];
    else
        state->T_batt = state->T_room;
    state->heat_dissipated = 0;
    state->T_batt_prev = state->T_room;
    state->q_relative_thermal = 100.;
}

void thermal_t::calc_capacity() {
    double percent = 100;

    // if using en_cap_vs_temp, it is done in the life model
    if (params->en_cap_vs_temp) {
        percent = util::linterp_col(params->cap_vs_temp, 0, state->T_batt, 1);
    }

    if (std::isnan(percent) || percent < 0 || percent > 100) {
        percent = 100;
//        log.add("Unable to determine capacity adjustment for temperature, ignoring");
    }
    state->q_relative_thermal = percent;
}

// battery temperature is the average temp during the time step
void thermal_t::updateTemperature(double I, size_t lifetimeIndex) {
    if (params->option == thermal_params::SCHEDULE) {
        state->T_room = params->T_room_schedule[lifetimeIndex % params->T_room_schedule.size()];
    }

    // the battery temp is the average temp over that step, starting with temp from end of last timestep
    double T_steady_state = I * I * params->resistance / (params->surface_area * params->h) + state->T_room;
    double diffusion = exp(-params->surface_area * params->h * dt_sec / params->mass / params->Cp);
    double coeff_avg = params->mass * params->Cp / params->surface_area / params->h / dt_sec;
    state->T_batt = (state->T_batt_prev - T_steady_state) * coeff_avg * (1 - diffusion) + T_steady_state;
    state->heat_dissipated = (state->T_batt - state->T_room) * params->surface_area * params->h / 1000.;

    // update temp for use in next timestep
    state->T_batt_prev = (state->T_batt_prev - T_steady_state) * diffusion + T_steady_state;

    calc_capacity();
}

double thermal_t::capacity_percent() { return state->q_relative_thermal; }

double thermal_t::T_battery() { return state->T_batt; }

thermal_state thermal_t::get_state() { return *state; }

thermal_params thermal_t::get_params() { return *params; }

/*
Define Losses
*/
void losses_t::initialize() {
    state = std::make_shared<losses_state>();
    state->loss_kw = 0;
    if (params->loss_choice == losses_params::MONTHLY) {
        if (params->monthly_charge_loss.size() == 1) {
            params->monthly_charge_loss = std::vector<double>(12, params->monthly_charge_loss[0]);
        }
        else if (params->monthly_charge_loss.size() != 12){
            goto fail;
        }
        if (params->monthly_discharge_loss.size() == 1) {
            params->monthly_discharge_loss = std::vector<double>(12, params->monthly_discharge_loss[0]);
        }
        else if (params->monthly_discharge_loss.size() != 12) {
            goto fail;
        }
        if (params->monthly_idle_loss.size() == 1) {
            params->monthly_idle_loss = std::vector<double>(12, params->monthly_idle_loss[0]);
        }
        else if (params->monthly_idle_loss.size() != 12) {
            goto fail;
        }
        return;
        fail:
        throw std::runtime_error("losses_t error: loss arrays length must be 1 or 12 for monthly input mode");
    }
    else {
        if (params->schedule_loss.empty()) {
            throw std::runtime_error("losses_t error: loss length must be greater than 0 for schedule mode");
        }
    }
}

losses_t::losses_t(const std::vector<double>& monthly_charge, const std::vector<double>& monthly_discharge, const std::vector<double>& monthly_idle) {
    params = std::make_shared<losses_params>();
    params->loss_choice = losses_params::MONTHLY;
    params->monthly_charge_loss = monthly_charge;
    params->monthly_discharge_loss = monthly_discharge;
    params->monthly_idle_loss = monthly_idle;
    initialize();
}

losses_t::losses_t(const std::vector<double>& schedule_loss) {
    params = std::make_shared<losses_params>();
    params->loss_choice = losses_params::SCHEDULE;
    params->schedule_loss = schedule_loss;
    initialize();
}

losses_t::losses_t(std::shared_ptr<losses_params> p) {
    params = std::move(p);
    initialize();
}

losses_t::losses_t(const losses_t& rhs) {
    params = std::make_shared<losses_params>(*rhs.params);
    state = std::make_shared<losses_state>(*rhs.state);
}

losses_t &losses_t::operator=(const losses_t& rhs) {
    if (this != &rhs) {
        *params = *rhs.params;
        *state = *rhs.state;
    }
    return *this;
}

void losses_t::run_losses(size_t lifetimeIndex, double dtHour, double charge_operation) {
    size_t indexYearOne = util::yearOneIndex(dtHour, lifetimeIndex);
    auto hourOfYear = (size_t) std::floor(indexYearOne * dtHour);
    size_t monthIndex = util::month_of((double) (hourOfYear)) - 1;

    // update system losses depending on user input
    if (params->loss_choice == losses_params::MONTHLY) {
        if (charge_operation == capacity_state::CHARGE)
            state->loss_kw = params->monthly_charge_loss[monthIndex];
        if (charge_operation == capacity_state::DISCHARGE)
            state->loss_kw = params->monthly_discharge_loss[monthIndex];
        if (charge_operation == capacity_state::NO_CHARGE)
            state->loss_kw = params->monthly_idle_loss[monthIndex];
    }
    else if (params->loss_choice == losses_params::SCHEDULE)  {
        state->loss_kw = params->schedule_loss[lifetimeIndex % params->schedule_loss.size()];
    }
}

double losses_t::getLoss() { return state->loss_kw; }

losses_state losses_t::get_state() { return *state; }

losses_params losses_t::get_params() {return *params; }

/*
Define Battery
*/
battery_state::battery_state():
        battery_state(std::make_shared<capacity_state>(),
                      std::make_shared<voltage_state>(),
                      std::make_shared<thermal_state>(),
                      std::make_shared<lifetime_state>(),
                      std::make_shared<losses_state>()) {
}

battery_state::battery_state(const std::shared_ptr<capacity_state> &cap, const std::shared_ptr<voltage_state> &vol,
                             const std::shared_ptr<thermal_state> &therm, const std::shared_ptr<lifetime_state> &life,
                             const std::shared_ptr<losses_state> &loss) {
    last_idx = 0;
    V = 0;
    P = 0;
    Q = 0;
    Q_max = 0;
    I = 0;
    I_dischargeable = 0;
    I_chargeable = 0;
    P_dischargeable = 0;
    P_chargeable = 0;
    capacity = cap;
    voltage = vol;
    thermal = therm;
    lifetime = life;
    losses = loss;
    replacement = std::make_shared<replacement_state>();
}

battery_state::battery_state(const battery_state& rhs) {
    operator=(rhs);
}

battery_state &battery_state::operator=(const battery_state &rhs) {
    if (this != &rhs) {
        last_idx = rhs.last_idx;
        V = rhs.V;
        P = rhs.P;
        Q = rhs.Q;
        Q_max = rhs.Q_max;
        I = rhs.I;
        I_dischargeable = rhs.I_dischargeable;
        I_chargeable = rhs.I_chargeable;
        P_dischargeable = rhs.P_dischargeable;
        P_chargeable = rhs.P_chargeable;
        if (capacity)
            *capacity = *rhs.capacity;
        else
            capacity = std::make_shared<capacity_state>(*rhs.capacity);
        if (voltage)
            *voltage = *rhs.voltage;
        else
            voltage = std::make_shared<voltage_state>(*rhs.voltage);
        if (thermal)
            *thermal = *rhs.thermal;
        else
            thermal = std::make_shared<thermal_state>(*rhs.thermal);
        if (lifetime)
            *lifetime = *rhs.lifetime;
        else
            lifetime = std::make_shared<lifetime_state>(*rhs.lifetime);
        if (losses)
            *losses = *rhs.losses;
        else
            losses = std::make_shared<losses_state>(*rhs.losses);
        if (replacement)
            *replacement = *rhs.replacement;
        else
            replacement = std::make_shared<replacement_state>(*rhs.replacement);
    }
    return *this;
}

battery_params::battery_params():
        battery_params(std::make_shared<capacity_params>(),
                       std::make_shared<voltage_params>(),
                       std::make_shared<thermal_params>(),
                       std::make_shared<lifetime_params>(),
                       std::make_shared<losses_params>()) {
}

battery_params::battery_params(const std::shared_ptr<capacity_params> &cap, const std::shared_ptr<voltage_params> &vol,
                               const std::shared_ptr<thermal_params> &therm,
                               const std::shared_ptr<lifetime_params> &life,
                               const std::shared_ptr<losses_params> &loss) {
    chem = -1;
    dt_hr = 0.;
    nominal_energy = 0;
    nominal_voltage = 0;
    capacity = cap;
    voltage = vol;
    thermal = therm;
    lifetime = life;
    losses = loss;
    replacement = std::make_shared<replacement_params>();
}

battery_params::battery_params(const battery_params& rhs) {
    operator=(rhs);
}

battery_params &battery_params::operator=(const battery_params &rhs) {
    if (this != &rhs) {
        chem = rhs.chem;
        dt_hr = rhs.dt_hr;
        nominal_voltage = rhs.nominal_voltage;
        nominal_energy = rhs.nominal_energy;
        if (capacity)
            *capacity = *rhs.capacity;
        else
            capacity = std::make_shared<capacity_params>(*rhs.capacity);
        if (voltage)
            *voltage = *rhs.voltage;
        else
            voltage = std::make_shared<voltage_params>(*rhs.voltage);
        if (thermal)
            *thermal = *rhs.thermal;
        else
            thermal = std::make_shared<thermal_params>(*rhs.thermal);
        if (lifetime)
            *lifetime = *rhs.lifetime;
        else
            lifetime = std::make_shared<lifetime_params>(*rhs.lifetime);
        if (losses)
            *losses = *rhs.losses;
        else
            losses = std::make_shared<losses_params>(*rhs.losses);
        if (replacement)
            *replacement = *rhs.replacement;
        else
            replacement = std::make_shared<replacement_params>(*rhs.replacement);
    }
    return *this;
}

void battery_t::initialize() {
    // capacity
    if (params->chem == battery_params::LEAD_ACID) {
        capacity = std::unique_ptr<capacity_t>(new capacity_kibam_t(params->capacity));
    } else {
        capacity = std::unique_ptr<capacity_t>(new capacity_lithium_ion_t(params->capacity));
    }

    // voltage
    if (params->voltage->voltage_choice == voltage_params::TABLE || params->chem == battery_params::IRON_FLOW) {
        voltage = std::unique_ptr<voltage_t>(new voltage_table_t(params->voltage));
    }
    else if (params->chem == battery_params::LEAD_ACID  || params->chem == battery_params::LITHIUM_ION) {
        voltage = std::unique_ptr<voltage_t>(new voltage_dynamic_t(params->voltage));
    }
    else if (params->chem == battery_params::VANADIUM_REDOX) {
        voltage = std::unique_ptr<voltage_t>(new voltage_vanadium_redox_t(params->voltage));
    }
    voltage->set_initial_SOC(capacity->state->SOC);

    // lifetime
    if (params->lifetime->model_choice == lifetime_params::CALCYC)
        lifetime = std::unique_ptr<lifetime_calendar_cycle_t>(new lifetime_calendar_cycle_t(params->lifetime));
    else
        lifetime = std::unique_ptr<lifetime_nmc_t>(new lifetime_nmc_t(params->lifetime));

    // thermal
    thermal = std::unique_ptr<thermal_t>(new thermal_t(params->thermal));

    // losses
    losses = std::unique_ptr<losses_t>(new losses_t(params->losses));

    state = std::make_shared<battery_state>(capacity->state, voltage->state, thermal->state, lifetime->state, losses->state);
}

battery_t::battery_t(double dt_hr, int chem, capacity_t *capacity_model, voltage_t *voltage_model,
                     lifetime_t *lifetime_model, thermal_t *thermal_model, losses_t *losses_model) {
    capacity = std::unique_ptr<capacity_t>(capacity_model);
    voltage = std::unique_ptr<voltage_t>(voltage_model);
    lifetime = std::unique_ptr<lifetime_t>(lifetime_model);
    thermal = std::unique_ptr<thermal_t>(thermal_model);
    if (losses_model == nullptr) {
        losses = std::unique_ptr<losses_t>(new losses_t());
    } else {
        losses = std::unique_ptr<losses_t>(losses_model);
    }

    state = std::make_shared<battery_state>(capacity->state, voltage->state, thermal->state, lifetime->state, losses->state);
    params = std::make_shared<battery_params>(capacity->params, voltage->params, thermal->params, lifetime->params, losses->params);
    params->dt_hr = dt_hr;
    params->chem = chem;
    params->nominal_voltage = params->voltage->Vnom_default * params->voltage->num_cells_series;
    params->nominal_energy = params->nominal_voltage * params->voltage->num_strings * params->voltage->dynamic.Qfull * 1e-3;

    // initial conditions
    voltage->set_initial_SOC(capacity->state->SOC);
}

battery_t::battery_t(std::shared_ptr<battery_params> p):
        params(std::move(p)) {
    initialize();
}

battery_t::battery_t(const battery_t &rhs) {
    params = std::make_shared<battery_params>();
    *params = *rhs.params;
    initialize();
    *state = *rhs.state;
}

void battery_t::setupReplacements(double capacity_percent) {
    params->replacement = std::make_shared<replacement_params>();
    params->replacement->replacement_option = replacement_params::CAPACITY_PERCENT;
    params->replacement->replacement_capacity = capacity_percent;
}

void battery_t::setupReplacements(std::vector<double> replacement_percents) {
    params->replacement = std::make_shared<replacement_params>();
    params->replacement->replacement_option = replacement_params::SCHEDULE;
    params->replacement->replacement_schedule_percent = std::move(replacement_percents);
}

double battery_t::calculate_current_for_power_kw(double &P_kw) {
    if (P_kw == 0.)
        return 0.;
    double current;
    if (P_kw < 0) {
        double max_P = calculate_max_charge_kw(&current);
        if (max_P > P_kw) {
            P_kw = max_P;
            return current;
        }
    } else {
        double max_P = calculate_max_discharge_kw(&current);
        if (max_P < P_kw) {
            P_kw = max_P;
            return current;
        }
    }
    return voltage->calculate_current_for_target_w(P_kw * 1000., capacity->q0(),
                                                   fmin(capacity->qmax(), capacity->qmax_thermal()),
                                                   thermal->T_battery());
}

double battery_t::calculate_voltage_for_current(double I) {
    // TODO: add looping when this function will actually be used... doesn't work that well atm
    double qmax = fmin(capacity->qmax(), capacity->qmax_thermal());
    return voltage->calculate_voltage_for_current(I, charge_total(), qmax, thermal->T_battery());
}

double battery_t::calculate_max_charge_kw(double *max_current_A) {
    thermal_state thermal_initial = thermal->get_state();
    double q = capacity->q0();
    double SOC_ratio = capacity->params->maximum_SOC * 0.01;
    double qmax = charge_maximum() * SOC_ratio;
    double power_W = 0;
    double current = 0;
    size_t its = 0;
    while (fabs(power_W - voltage->calculate_max_charge_w(q, qmax, thermal->T_battery(), &current)) > tolerance
           && its++ < 10) {
        power_W = voltage->calculate_max_charge_w(q, qmax, thermal->T_battery(), &current);
        thermal->updateTemperature(current, state->last_idx + 1);
        qmax = capacity->qmax() * thermal->capacity_percent() * 0.01 * SOC_ratio;
    }
    if (max_current_A)
        *max_current_A = current;
    *thermal->state = thermal_initial;
    return power_W / 1000.;
}

double battery_t::calculate_max_discharge_kw(double *max_current_A) {
    thermal_state thermal_initial = thermal->get_state();
    double q = capacity->q0();
    double SOC_ratio = (1. - capacity->params->minimum_SOC * 0.01);
    double qmax = charge_maximum() * SOC_ratio;
    double power_W = 0;
    double current = 0;
    size_t its = 0;
    while (fabs(power_W - voltage->calculate_max_discharge_w(q, qmax, thermal->T_battery(), &current)) > tolerance
           && its++ < 5) {
        power_W = voltage->calculate_max_discharge_w(q, qmax, thermal->T_battery(), &current);
        thermal->updateTemperature(current, state->last_idx + 1);
        qmax = capacity->qmax() * thermal->capacity_percent()  * 0.01 * SOC_ratio;
    }
    if (max_current_A)
        *max_current_A = current;
    *thermal->state = thermal_initial;
    return power_W / 1000.;
}

void battery_t::ChangeTimestep(double dt_hr) {
    if (dt_hr <= 0)
        throw std::runtime_error("battery_t timestep must be greater than 0 hour");
    if (dt_hr > 1)
        throw std::runtime_error("battery_t timestep must be less than or equal to 1 hour");

    auto old_hr = (double)state->last_idx * params->dt_hr;
    state->last_idx = (size_t)(old_hr / dt_hr);
    /*
    if (fabs(old_hr / dt_hr - state->last_idx) > 1e-7)
        throw std::runtime_error("battery_t dt_hr step size can only be changed to a higher step size when the current time step"
                                 " is at a time step common to both the previous and new step size. For instance, if running"
                                 " 30-min steps, step size can only be increased to 60-min step at the hour.");
    */
    params->dt_hr = dt_hr;
    params->capacity->dt_hr = dt_hr;
    params->voltage->dt_hr = dt_hr;
    params->thermal->dt_hr = dt_hr;
    thermal->dt_sec = dt_hr * 3600;
    params->lifetime->dt_hr = dt_hr;
}

double battery_t::run(size_t lifetimeIndex, double &I) {
    // Temperature affects capacity, but capacity model can reduce current, which reduces temperature, need to iterate
    double I_initial = I;
    size_t iterate_count = 0;
    capacity_state capacity_initial = capacity->get_state();
    thermal_state thermal_initial = thermal->get_state();

    while (iterate_count < 5) {
        runThermalModel(I, lifetimeIndex);
        runCapacityModel(I);

        double numerator = fabs(I - I_initial);
        if ((numerator > 0.0) && (numerator / fabs(I_initial) > tolerance)) {
            *thermal->state = thermal_initial;
            *capacity->state = capacity_initial;
            I_initial = I;
            iterate_count++;
        } else {
            break;
        }

    }
    runVoltageModel();
    runLifetimeModel(lifetimeIndex);
    runLossesModel(lifetimeIndex);

    update_state(I);
    return state->P;
}

void battery_t::runCurrent(double I) {
    run(++state->last_idx, I);
        }

void battery_t::runPower(double P) {
    double I = calculate_current_for_power_kw(P);
    run(++state->last_idx, I);
}

void battery_t::runThermalModel(double I, size_t lifetimeIndex) {
    thermal->updateTemperature(I, lifetimeIndex);
}

double battery_t::estimateCycleDamage() {
    return lifetime->estimateCycleDamage();
}

void battery_t::runCapacityModel(double &I) {
    // Don't update max capacity if the battery is idle
    if (fabs(I) > tolerance) {
        // Need to first update capacity model to ensure temperature accounted for
        capacity->updateCapacityForThermal(thermal->capacity_percent());
    }
    capacity->updateCapacity(I, params->dt_hr);
}

void battery_t::runVoltageModel() {
    voltage->updateVoltage(capacity->q0(), capacity->qmax(), capacity->I(), thermal->T_battery(), params->dt_hr);
}

void battery_t::runLifetimeModel(size_t lifetimeIndex) {
    lifetime->runLifetimeModels(lifetimeIndex,
                                capacity->chargeChanged(), 100. - capacity->SOC_prev(), 100. - capacity->SOC(),
                                thermal->T_battery());
    capacity->updateCapacityForLifetime(lifetime->capacity_percent());
}

void battery_t::runLossesModel(size_t idx) {
    if (idx > state->last_idx || idx == 0) {
        losses->run_losses(idx, params->dt_hr, capacity->charge_operation());
        state->last_idx = idx;
    }
}

void battery_t::runReplacement(size_t year, size_t hour, size_t step) {
    if (year == 0 && hour == 0)
        return;

    if (params->replacement->replacement_option == replacement_params::OPTIONS::NONE)
        return;

    bool replace = false;
    double percent = 0;
    if (params->replacement->replacement_option == replacement_params::OPTIONS::SCHEDULE) {
        if (year < params->replacement->replacement_schedule_percent.size()) {
            percent = params->replacement->replacement_schedule_percent[year];
            if (percent > 0 && hour == 0 && step == 0) {
                replace = true;
            }
        }
    } else if (params->replacement->replacement_option == replacement_params::OPTIONS::CAPACITY_PERCENT) {
        if ((lifetime->capacity_percent() - tolerance) <= params->replacement->replacement_capacity) {
            replace = true;
            percent = 100.;
        }
    }

    if (replace) {
        state->replacement->n_replacements++;
        state->replacement->indices_replaced.push_back(util::lifetimeIndex(year, hour, step, (size_t) (1 / params->dt_hr)));
        lifetime->replaceBattery(percent);
        capacity->replace_battery(percent);
        thermal->replace_battery(year);
    }
}

void battery_t::resetReplacement() {
    state->replacement->n_replacements = 0;
}

double battery_t::getNumReplacementYear() {
    return state->replacement->n_replacements;
}

double battery_t::getReplacementPercent()
{
    if (params->replacement->replacement_option == params->replacement->CAPACITY_PERCENT)
    {
        return (params->replacement->replacement_capacity / 100.0);
    }

    return 0.0;
}

void battery_t::changeSOCLimits(double min, double max) {
    capacity->change_SOC_limits(min, max);
}

double battery_t::charge_needed(double SOC_max) {
    double charge_needed = capacity->qmax_thermal() * SOC_max * 0.01 - capacity->q0();
    if (charge_needed > 0)
        return charge_needed;
    else
        return 0.;
}

double battery_t::energy_to_fill(double SOC_max) {
    double battery_voltage = this->V_nominal(); // [V]
    double charge_needed_to_fill = this->charge_needed(SOC_max); // [Ah] - qmax - q0
    return (charge_needed_to_fill * battery_voltage) * util::watt_to_kilowatt;  // [kWh]
}

double battery_t::energy_nominal() {
    return V_nominal() * capacity->qmax() * util::watt_to_kilowatt;
}

double battery_t::energy_max(double SOC_max, double SOC_min) {
   return V()* charge_maximum_lifetime() * (SOC_max - SOC_min) * 0.01 * util::watt_to_kilowatt;
}

double battery_t::energy_available(double SOC_min) {
    return V() * charge_maximum_lifetime() * (SOC() - SOC_min) * 0.01 * util::watt_to_kilowatt;
}

double battery_t::power_to_fill(double SOC_max) {
    // in one time step
    return (this->energy_to_fill(SOC_max) / params->dt_hr);
}

double battery_t::charge_total() { return capacity->q0(); }

double battery_t::charge_maximum() { return fmin(capacity->qmax(), capacity->qmax_thermal()); }

double battery_t::charge_maximum_lifetime() { return capacity->qmax(); }

double battery_t::charge_maximum_thermal() { return capacity->qmax_thermal(); }

double battery_t::V() { return voltage->battery_voltage(); }

double battery_t::V_nominal() { return voltage->battery_voltage_nominal(); }

double battery_t::SOC() { return capacity->SOC(); }

double battery_t::I() { return capacity->I(); }

double battery_t::calculate_loss(double power, size_t lifetimeIndex) {
    size_t indexYearOne = util::yearOneIndex(params->dt_hr, lifetimeIndex);
    auto hourOfYear = (size_t)std::floor(indexYearOne * params->dt_hr);
    size_t monthIndex = (size_t) util::month_of((double)(hourOfYear)) - 1;

    if (params->losses->loss_choice == losses_params::MONTHLY) {
        if (power > 0) {
            return params->losses->monthly_discharge_loss[monthIndex];
        }
        else if (power < 0) {
            return params->losses->monthly_charge_loss[monthIndex];
        }
        else {
            return params->losses->monthly_idle_loss[monthIndex];
        }

    }
    else {
        return params->losses->schedule_loss[lifetimeIndex % params->losses->schedule_loss.size()];
    }
}

double battery_t::getLoss() {
    return losses->getLoss();
}

battery_state battery_t::get_state() { return *state; }

battery_params battery_t::get_params() { return *params; }

void battery_t::set_state(const battery_state& tmp_state, double dt_hr) {
    *state = tmp_state;
    if (dt_hr > 0 && dt_hr <= 1)
        params->dt_hr = dt_hr;
}

void battery_t::update_state(double I) {
    state->I = I;
    state->Q = capacity->q0();
    state->Q_max = capacity->qmax();
    state->V = voltage->battery_voltage();
    state->P_dischargeable = calculate_max_discharge_kw(&state->I_dischargeable);
    state->P_chargeable = calculate_max_charge_kw(&state->I_chargeable);
    state->P = I * voltage->battery_voltage() * util::watt_to_kilowatt;
}
