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

#ifndef SAM_SIMULATION_CORE_LIB_BATTERY_LIFETIME_H
#define SAM_SIMULATION_CORE_LIB_BATTERY_LIFETIME_H

#include <memory>


struct calendar_cycle_params;

struct lifetime_params {
    double dt_hr;

    enum MODEL_CHOICE {
        CALCYC,
        NMCNREL         // K. Smith: Life Prediction model coefficients
    } model_choice;

    std::shared_ptr<calendar_cycle_params> cal_cyc;

    lifetime_params();

    lifetime_params &operator=(const lifetime_params &rhs);

    friend std::ostream &operator<<(std::ostream &os, const lifetime_params &p);
};

struct cycle_state;
struct calendar_state;
struct lifetime_nmc_state;

struct lifetime_state {
    double q_relative;                      // total lifetime relative capacity %
    int n_cycles;
    double range;
    double average_range;
    double day_age_of_battery;

    // CALCYC model state
    std::shared_ptr<calendar_state> calendar;
    std::shared_ptr<cycle_state> cycle;

    // NREL NMC model state
    std::shared_ptr<lifetime_nmc_state> nmc_li_neg;

    lifetime_state();

    lifetime_state(const lifetime_state &rhs);

    lifetime_state(const std::shared_ptr<cycle_state>& cyc, const std::shared_ptr<calendar_state>& cal);

    explicit lifetime_state(const std::shared_ptr<lifetime_nmc_state>& nmc);

    lifetime_state &operator=(const lifetime_state &rhs);

    friend std::ostream &operator<<(std::ostream &os, const lifetime_state &p);
};

class lifetime_t {
public:
    lifetime_t() = default;

    lifetime_t(const lifetime_t &rhs);

    lifetime_t &operator=(const lifetime_t &rhs);

    virtual lifetime_t *clone() = 0;

    virtual ~lifetime_t() = default;

    /// Execute the lifetime models given the current lifetime run index, capacity model, and temperature
    virtual void runLifetimeModels(size_t lifetimeIndex, bool charge_changed, double prev_DOD, double DOD, double T_battery) = 0;

    /// Return the relative capacity percentage of nominal (%)
    double capacity_percent();

    virtual double estimateCycleDamage() = 0;

    virtual void replaceBattery(double percent_to_replace) = 0;

    lifetime_params get_params();

    lifetime_state get_state();

protected:

    std::shared_ptr<lifetime_state> state;
    std::shared_ptr<lifetime_params> params;

    friend class battery_t;
};


#endif //SAM_SIMULATION_CORE_LIB_BATTERY_LIFETIME_H
