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

#ifndef battery_h
#define battery_h

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <cstdio>
#include <algorithm>

#include "lib_util.h"
#include "lib_battery_capacity.h"
#include "lib_battery_voltage.h"
#include "lib_battery_lifetime_calendar_cycle.h"
#include "lib_battery_lifetime_nmc.h"

/**
* \class thermal_t
*
* \brief
*
*  The Thermal class
*/

struct thermal_state {
    double q_relative_thermal;   //[%]
    double T_batt;               // C
    double T_room;
    double heat_dissipated;      // W
    double T_batt_prev;

    friend std::ostream &operator<<(std::ostream &os, const thermal_state &p);
};

struct thermal_params {
    double dt_hr;
    double mass;                 // [kg]
    double surface_area;         // [m2] - exposed surface area
    double Cp;                   // [J/KgK] - battery specific heat capacity
    double h;                    // [W/m2/K] - general heat transfer coefficient
    double resistance;                    // [Ohm] - internal resistance

    bool en_cap_vs_temp;       // if true, no capacity degradation from temp and do not use cap_vs_temp
    util::matrix_t<double> cap_vs_temp;

    enum OPTIONS {
        VALUE, SCHEDULE
    };
    int option;
    double T_room_init;                    // starting temperature C
    std::vector<double> T_room_schedule;   // can be year one hourly data or a single value constant throughout year

    friend std::ostream &operator<<(std::ostream &os, const thermal_params &p);
};

class thermal_t {
public:
    // constructors for capacity as an entry from a cap_vs_temp table
    thermal_t(double dt_hour, double mass, double surface_area, double R, double Cp, double h,
              const util::matrix_t<double> &c_vs_t, std::vector<double> T_room_C);

    thermal_t(double dt_hour, double mass, double surface_area, double R, double Cp, double h,
              const util::matrix_t<double> &c_vs_t, double T_room_C);

    // constructors for capacity as an analytical function
    thermal_t(double dt_hour, double mass, double surface_area, double R, double Cp, double h,
        double T_room_C);

    thermal_t(double dt_hour, double mass, double surface_area, double R, double Cp, double h,
         std::vector<double> T_room_C);

    explicit thermal_t(std::shared_ptr<thermal_params> p);

    thermal_t(const thermal_t &rhs);

    thermal_t &operator=(const thermal_t &rhs);

    thermal_t *clone();

    void updateTemperature(double I, size_t lifetimeIndex);

    void replace_battery(size_t lifetimeIndex);

    // outputs
    double T_battery();

    double capacity_percent();

    thermal_state get_state();

    thermal_params get_params();

protected:
    double dt_sec;              // [sec] - timestep
    void calc_capacity();

    std::shared_ptr<thermal_params> params;
    std::shared_ptr<thermal_state> state;

private:

    void initialize();

    friend class battery_t;
};


/**
* \class losses_t
*
* \brief
*
*  The Battery losses class takes generic losses which occur during charging, discharge, or idle operation modes:
*  The model also accepts a time-series vector of losses defined for every time step of the first year of simulation
*  which may be used in lieu of the losses for operational mode.
*/

struct losses_state {
    double loss_kw;

    friend std::ostream &operator<<(std::ostream &os, const losses_state &p);
};

struct losses_params {
    enum OPTIONS {
        MONTHLY, SCHEDULE
    };
    int loss_choice;

    std::vector<double> monthly_charge_loss;
    std::vector<double> monthly_discharge_loss;
    std::vector<double> monthly_idle_loss;
    std::vector<double> schedule_loss;

    friend std::ostream &operator<<(std::ostream &os, const losses_params &p);
};

class losses_t {
public:

    /**
    * \function losses_t
    *
    * Construct the losses object for monthly losses
    *
    * \param[in] monthly_charge vector (size 1 for annual or 12 for monthly) containing battery system losses when charging (kW) (applied to PV or grid)
    * \param[in] monthly_discharge vector (size 1 for annual or 12 for monthly) containing battery system losses when discharge (kW) (applied to battery power)
    * \param[in] monthly_idle vector (size 1 for annual or 12 for monthly) containing battery system losses when idle (kW) (applied to PV or grid)
    */
    losses_t(const std::vector<double>& monthly_charge, const std::vector<double>& monthly_discharge, const std::vector<double>& monthly_idle);

    /**
    * \function losses_t
    *
    * Construct the losses object for schedule of timeseries losses
    *
    * \param[in] schedule_loss vector (size 0 for constant or per timestep) containing battery system losses
    */
    explicit losses_t(const std::vector<double>& schedule_loss = std::vector<double>(1, 0));

    explicit losses_t(std::shared_ptr<losses_params> p);

    losses_t(const losses_t& rhs);

    losses_t &operator=(const losses_t& rhs);

    /// Run the losses model at the present simulation index (for year 1 only)
    void run_losses(size_t lifetimeIndex, double dt_hour, double charge_operation);

    /// Get the loss at the specified simulation index (year 1)
    double getLoss();

    losses_state get_state();

    losses_params get_params();

protected:
    std::shared_ptr<losses_state> state;
    std::shared_ptr<losses_params> params;

private:
    void initialize();

    friend class battery_t;
};

/*
Class which encapsulates a battery and all its models
*/

struct replacement_state {
    int n_replacements;                                 // number of replacements this year
    std::vector<int> indices_replaced;               // lifetime indices at which replacements occurred

    friend std::ostream &operator<<(std::ostream &os, const replacement_state &p);
};

struct replacement_params {
    enum OPTIONS {
        NONE, CAPACITY_PERCENT, SCHEDULE
    };
    int replacement_option;

    /// Maximum capacity relative to nameplate at which to replace battery back to 100%
    double replacement_capacity;

    std::vector<double> replacement_schedule_percent;    // (0 - 100%)

    friend std::ostream &operator<<(std::ostream &os, const replacement_params &p);
};

struct battery_state {
    size_t last_idx;

    // Values for battery as a whole
    double V;
    double Q;
    double Q_max;
    double I;
    double I_dischargeable;
    double I_chargeable;
    double P;
    double P_dischargeable;
    double P_chargeable;

    // Values for battery cell
    std::shared_ptr<capacity_state> capacity;
    std::shared_ptr<voltage_state> voltage;
    std::shared_ptr<thermal_state> thermal;
    std::shared_ptr<lifetime_state> lifetime;
    std::shared_ptr<losses_state> losses;
    std::shared_ptr<replacement_state> replacement;

    // create with new subclass states
    battery_state();

    // create with given subclass states
    battery_state(const std::shared_ptr<capacity_state> &cap, const std::shared_ptr<voltage_state> &vol,
                  const std::shared_ptr<thermal_state> &therm, const std::shared_ptr<lifetime_state> &life,
                  const std::shared_ptr<losses_state> &loss);

    battery_state(const battery_state& rhs);

    battery_state &operator=(const battery_state &rhs);

    friend std::ostream &operator<<(std::ostream &os, const battery_state &p);
};

struct battery_params {
    enum CHEM {
        LEAD_ACID, LITHIUM_ION, VANADIUM_REDOX, IRON_FLOW
    };
    int chem;
    double dt_hr;
    double nominal_energy;
    double nominal_voltage;
    std::shared_ptr<capacity_params> capacity;
    std::shared_ptr<voltage_params> voltage;
    std::shared_ptr<thermal_params> thermal;
    std::shared_ptr<lifetime_params> lifetime;
    std::shared_ptr<losses_params> losses;
    std::shared_ptr<replacement_params> replacement;

    battery_params();

    battery_params(const std::shared_ptr<capacity_params> &cap, const std::shared_ptr<voltage_params> &vol,
                  const std::shared_ptr<thermal_params> &therm, const std::shared_ptr<lifetime_params> &life,
                  const std::shared_ptr<losses_params> &loss);

    battery_params(const battery_params& rhs);

    battery_params &operator=(const battery_params &rhs);

    friend std::ostream &operator<<(std::ostream &os, const battery_params &p);
};

class battery_t {
public:
    battery_t(double dt_hr, int chem,
              capacity_t* capacity_model,
              voltage_t* voltage_model,
              lifetime_t* lifetime_model,
              thermal_t* thermal_model,
              losses_t* losses_model);

    explicit battery_t(std::shared_ptr<battery_params> p);

    battery_t(const battery_t &battery);

    // replace by capacity
    void setupReplacements(double capacity);

    // replace by schedule
    void setupReplacements(std::vector<double> replacement_percents);

    void runReplacement(size_t year, size_t hour, size_t step);

    void resetReplacement();

    double getNumReplacementYear();

    // Returns the % replacement if on a capacity schedule. Returns 0 for "none" or "calendar"
    double getReplacementPercent();

    // Change the timestep of the battery and its component models
    void ChangeTimestep(double dt_hr);

    // Run all for single time step, updating all component model states and return the dispatched power [kW]
    double run(size_t lifetimeIndex, double &I);

    // Run for a single time step, using a control current A and the time step found in battery state
    void runCurrent(double I);

    // Run for a single time step, using a control power kW and the time step found in battery state
    void runPower(double P);

    double calculate_voltage_for_current(double I);

    // Return the max charge or discharge power achievable in the next time step, and the required current [A]
    double calculate_max_charge_kw(double *max_current_A = nullptr);

    double calculate_max_discharge_kw(double *max_current_A = nullptr);

    // Returns current [A] required to dispatch input power [kW], or the max power (to which P_kw is set)
    double calculate_current_for_power_kw(double &P_kw);

    double estimateCycleDamage();

    // Run a component level model
    void runCapacityModel(double &I);

    void runVoltageModel();

    void runThermalModel(double I, size_t lifetimeIndex);

    void runLifetimeModel(size_t lifetimeIndex);

    void runLossesModel(size_t lifetimeIndex);

    void changeSOCLimits(double min, double max);

    // Get capacity quantities
    double charge_needed(double SOC_max);

    double charge_total();

    double charge_maximum();

    double charge_maximum_lifetime();

    double charge_maximum_thermal();

    double energy_nominal();

    // Get the maximum energy in one full charge-dischage cycle, based on dispatch limits
    double energy_max(double SOC_max, double SOC_min);

    // Get the energy available between the current SOC and SOC_min
    double energy_available(double SOC_min);

    double energy_to_fill(double SOC_max);

    double power_to_fill(double SOC_max);

    double SOC();

    double V(); // the actual battery voltage

    double V_nominal(); // the nominal battery voltage

    double I();

    // Get estimated losses
    double calculate_loss(double power, size_t lifetimeIndex);

    // Get the losses at the current step
    double getLoss();

    battery_state get_state();

    battery_params get_params();

    void set_state(const battery_state& state, double dt_hr=0.0);

private:
    std::unique_ptr<capacity_t> capacity;
    std::unique_ptr<thermal_t> thermal;
    std::unique_ptr<lifetime_t> lifetime;
    std::unique_ptr<voltage_t> voltage;
    std::unique_ptr<losses_t> losses;

    std::shared_ptr<battery_state> state;
    std::shared_ptr<battery_params> params;

    void initialize();

    void update_state(double I);
};

#endif
