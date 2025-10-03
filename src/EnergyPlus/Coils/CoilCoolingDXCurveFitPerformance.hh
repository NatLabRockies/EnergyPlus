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

#ifndef ENERGYPLUS_COILS_COILCOOLINGDXCURVEFITPERFORMANCE
#define ENERGYPLUS_COILS_COILCOOLINGDXCURVEFITPERFORMANCE

#include <string>
#include <vector>

#include <EnergyPlus/Coils/CoilCoolingDXCurveFitOperatingMode.hh>
#include <EnergyPlus/Coils/CoilCoolingDXPerformanceBase.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/StandardRatings.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

struct CoilCoolingDXCurveFitPerformanceInputSpecification
{
    std::string name;
    Real64 crankcase_heater_capacity;
    Real64 minimum_outdoor_dry_bulb_temperature_for_compressor_operation;
    Real64 maximum_outdoor_dry_bulb_temperature_for_crankcase_heater_operation;
    Real64 unit_internal_static_air_pressure;
    Real64 basin_heater_capacity;
    Real64 basin_heater_setpoint_temperature;
    std::string basin_heater_operating_schedule_name;
    std::string compressor_fuel_type;
    std::string base_operating_mode_name;
    std::string alternate_operating_mode_name;
    std::string alternate_operating_mode2_name;
    std::string outdoor_temperature_dependent_crankcase_heater_capacity_curve_name;
    std::string capacity_control;
};

struct CoilCoolingDXCurveFitPerformance : public CoilCoolingDXPerformanceBase
{
    static constexpr std::string_view object_name = "Coil:Cooling:DX:CurveFit:Performance";
    std::string parentName;

    void instantiateFromInputSpec(EnergyPlusData &state, const CoilCoolingDXCurveFitPerformanceInputSpecification &input_data);

    void simulate(EnergyPlusData &state,
                  const DataLoopNode::NodeData &inletNode,
                  DataLoopNode::NodeData &outletNode,
                  HVAC::CoilMode currentCoilMode,
                  int speedNum,
                  Real64 speedRatio,
                  HVAC::FanOp const fanOp,
                  DataLoopNode::NodeData &condInletNode,
                  DataLoopNode::NodeData &condOutletNode,
                  bool const singleMode,
                  Real64 LoadSHR = 0.0) override;

    void calculate(EnergyPlusData &state,
                   CoilCoolingDXCurveFitOperatingMode &currentMode,
                   const DataLoopNode::NodeData &inletNode,
                   DataLoopNode::NodeData &outletNode,
                   int speedNum,
                   Real64 speedRatio,
                   HVAC::FanOp const fanOp,
                   DataLoopNode::NodeData &condInletNode,
                   DataLoopNode::NodeData &condOutletNode,
                   bool const singleMode);

    void calcStandardRatings210240(EnergyPlusData &state) override;

    CoilCoolingDXCurveFitPerformanceInputSpecification original_input_specs;

    CoilCoolingDXCurveFitPerformance() = default;

    explicit CoilCoolingDXCurveFitPerformance(EnergyPlusData &state, const std::string &name);

    void size(EnergyPlusData &state) override;

    void setOperMode(EnergyPlusData &state, CoilCoolingDXCurveFitOperatingMode &currentMode, int const mode);

    void oneTimeMinOATSetup();

    Real64 ratedCBF(EnergyPlusData &) override
    {
        return normalMode.speeds[normalMode.nominalSpeedIndex].RatedCBF;
    }

    Real64 grossRatedSHR(EnergyPlusData &) override
    {
        return normalMode.speeds[normalMode.nominalSpeedIndex].grossRatedSHR;
    }

    Real64 grossRatedCoolingCOPAtMaxSpeed(EnergyPlusData &) override
    {
        return normalMode.speeds.back().original_input_specs.gross_rated_cooling_COP;
    }

    const std::string_view nameAtSpeed(int speed) override
    {
        return normalMode.speeds[speed].name;
    }

    Real64 ratedAirMassFlowRateMaxSpeed(EnergyPlusData &, HVAC::CoilMode const mode) override
    {
        if (mode != HVAC::CoilMode::Normal) {
            return alternateMode.speeds.back().RatedAirMassFlowRate;
        } else {
            return normalMode.speeds.back().RatedAirMassFlowRate;
        }
    }

    Real64 ratedAirMassFlowRateMinSpeed(EnergyPlusData &, HVAC::CoilMode const mode) override
    {
        if (mode != HVAC::CoilMode::Normal) {
            return alternateMode.speeds.front().RatedAirMassFlowRate;
        } else {
            return normalMode.speeds.front().RatedAirMassFlowRate;
        }
    }

    Real64 ratedCondAirMassFlowRateNomSpeed(EnergyPlusData &, HVAC::CoilMode const mode) override
    {
        if (mode != HVAC::CoilMode::Normal) {
            return alternateMode.speeds[alternateMode.nominalSpeedIndex].RatedCondAirMassFlowRate;
        } else {
            return normalMode.speeds[normalMode.nominalSpeedIndex].RatedCondAirMassFlowRate;
        }
    }

    Real64 ratedEvapAirMassFlowRate(EnergyPlusData &) override
    {
        return normalMode.ratedEvapAirMassFlowRate;
    }

    Real64 ratedEvapAirFlowRate(EnergyPlusData &) override
    {
        return normalMode.ratedEvapAirFlowRate;
    }

    Real64 ratedGrossTotalCap() override
    {
        return normalMode.ratedGrossTotalCap;
    }

    int indexCapFT(HVAC::CoilMode const mode) override
    {
        if (mode != HVAC::CoilMode::Normal) {
            return alternateMode.speeds[alternateMode.nominalSpeedIndex].indexCapFT;
        } else {
            return normalMode.speeds[normalMode.nominalSpeedIndex].indexCapFT;
        }
    }

    bool subcoolReheatFlag() override
    {
        return (!original_input_specs.base_operating_mode_name.empty() && !original_input_specs.alternate_operating_mode_name.empty() &&
                !original_input_specs.alternate_operating_mode2_name.empty());
    }

    int numSpeeds() override
    {
        return static_cast<int>(normalMode.speeds.size());
    }

    virtual void setToHundredPercentDOAS() override
    {
        for (auto &speed : normalMode.speeds) {
            speed.minRatedVolFlowPerRatedTotCap = HVAC::MinRatedVolFlowPerRatedTotCap2;
            speed.maxRatedVolFlowPerRatedTotCap = HVAC::MaxRatedVolFlowPerRatedTotCap2;
        }
        if (maxAvailCoilMode != HVAC::CoilMode::Normal) {
            for (auto &speed : alternateMode.speeds) {
                speed.minRatedVolFlowPerRatedTotCap = HVAC::MinRatedVolFlowPerRatedTotCap2;
                speed.maxRatedVolFlowPerRatedTotCap = HVAC::MaxRatedVolFlowPerRatedTotCap2;
            }
        }
    }

    Real64 evapAirFlowRateAtSpeedIndex(EnergyPlusData &, int index) override
    {
        return normalMode.speeds[index].evap_air_flow_rate;
    }

    Real64 ratedTotalCapacityAtSpeedIndex(EnergyPlusData &, int index) override
    {
        return normalMode.speeds[index].rated_total_capacity;
    }

    Real64 currentEvapCondPumpPowerAtSpeed(EnergyPlusData &, int speed) override
    {
        return normalMode.getCurrentEvapCondPumpPower(speed);
    }

    Real64 evapCondenserEffectivenessAtSpeedIndex(EnergyPlusData &, int index) override
    {
        return normalMode.speeds[index].evap_condenser_effectiveness;
    }

    Real64 evapAirFlowFraction(EnergyPlusData &) override
    {
        return normalMode.speeds.front().original_input_specs.evaporator_air_flow_fraction;
    }

    Real64 maxOutdoorDrybulbForBasin = 0.0;
    bool mySizeFlag = true;

    Real64 evapCondBasinHeatSetpoint = 0.0;
    Sched::Schedule *evapCondBasinHeatSched = nullptr;
    bool oneTimeEIOHeaderWrite = true;
    Real64 wasteHeatRate = 0.0;

    CoilCoolingDXCurveFitOperatingMode normalMode;
    CoilCoolingDXCurveFitOperatingMode alternateMode;  // enhanced dehumidifcation or Subcool mode
    CoilCoolingDXCurveFitOperatingMode alternateMode2; // Reheat mode
    bool myOneTimeMinOATFlag = true;
};

} // namespace EnergyPlus

#endif // ENERGYPLUS_COILS_COILCOOLINGDXCURVEFITPERFORMANCE
