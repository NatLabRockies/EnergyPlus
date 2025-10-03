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

#ifndef ENERGYPLUS_COILS_COILCOOLINGDXCURVEFITOPERATINGMODE
#define ENERGYPLUS_COILS_COILCOOLINGDXCURVEFITOPERATINGMODE

#include <string>
#include <vector>

#include <EnergyPlus/Coils/CoilCoolingDXCurveFitSpeed.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

struct CoilCoolingDXCurveFitOperatingModeInputSpecification
{
    std::string name;
    Real64 gross_rated_total_cooling_capacity = 0.0;
    Real64 rated_evaporator_air_flow_rate = 0.0;
    Real64 rated_condenser_air_flow_rate = 0.0;
    Real64 maximum_cycling_rate = 0.0;
    Real64 ratio_of_initial_moisture_evaporation_rate_and_steady_state_latent_capacity = 0.0;
    Real64 latent_capacity_time_constant = 0.0;
    Real64 nominal_time_for_condensate_removal_to_begin = 0.0;
    std::string apply_latent_degradation_to_speeds_greater_than_1;
    std::string condenser_type;
    Real64 nominal_evap_condenser_pump_power = 0.0;
    Real64 nominal_speed_number = 0.0;
    std::vector<std::string> speed_data_names;
};

struct CoilCoolingDXCurveFitOperatingMode
{
    std::string object_name = "Coil:Cooling:DX:CurveFit:OperatingMode";
    std::string parentName;

    void instantiateFromInputSpec(EnergyPlusData &state, CoilCoolingDXCurveFitOperatingModeInputSpecification input_data);
    void size(EnergyPlusData &state);
    void oneTimeInit(EnergyPlusData &state);
    CoilCoolingDXCurveFitOperatingModeInputSpecification original_input_specs;
    CoilCoolingDXCurveFitOperatingMode() = default;
    explicit CoilCoolingDXCurveFitOperatingMode(EnergyPlusData &state, const std::string &name_to_find);
    Real64 getCurrentEvapCondPumpPower(int speedNum);
    void CalcOperatingMode(EnergyPlusData &state,
                           const DataLoopNode::NodeData &inletNode,
                           DataLoopNode::NodeData &outletNode,
                           int speedNum,
                           Real64 speedRatio,
                           HVAC::FanOp const fanOp,
                           DataLoopNode::NodeData &condInletNode,
                           DataLoopNode::NodeData &condOutletNode,
                           bool singleMode);

    std::string name;
    Real64 ratedGrossTotalCap = 0.0;       // [W]
    Real64 ratedEvapAirFlowRate = 0.0;     // [m3/s]
    Real64 ratedCondAirFlowRate = 0.0;     // [m3/s]
    Real64 ratedEvapAirMassFlowRate = 0.0; // [kg/s]
    bool ratedGrossTotalCapIsAutosized = false;
    bool ratedEvapAirFlowRateIsAutosized = false;

    // Latent degradation model
    Real64 timeForCondensateRemoval = 0.0;
    Real64 evapRateRatio = 0.0;
    Real64 maxCyclingRate = 0.0;
    Real64 latentTimeConst = 0.0;
    bool latentDegradationActive = false;
    bool applyLatentDegradationAllSpeeds = false;

    // results from coil model at speed
    //    Real64 OpModeOutletTemp = 0.0;
    //    Real64 OpModeOutletHumRat = 0.0;
    //    Real64 OpModeOutletEnth = 0.0;
    Real64 OpModePower = 0.0;
    Real64 OpModeRTF = 0.0;
    Real64 OpModeWasteHeat = 0.0;

    Real64 nominalEvaporativePumpPower = 0.0;
    int nominalSpeedIndex = 0;

    // each mode can now have EMS overridden evap air flow and total cap
    bool ratedAirVolFlowEMSOverrideON = false;
    Real64 ratedAirVolFlowEMSOverrideValue = 0.0;
    bool ratedTotCapFlowEMSOverrideON = false;
    Real64 ratedTotCapFlowEMSOverrideValue = 0.0;
    Real64 minOutdoorDrybulb = -25.0;

    enum class CondenserType
    {
        Invalid = -1,
        AIRCOOLED,
        EVAPCOOLED,
        Num
    };

    CondenserType condenserType = CondenserType::AIRCOOLED;

    Real64 condInletTemp = 0.0; // condenser inlet node temp or outdoor temp if no condenser node {C}

    std::vector<CoilCoolingDXCurveFitSpeed> speeds;
};

} // namespace EnergyPlus

#endif // ENERGYPLUS_COILS_COILCOOLINGDXCURVEFITOPERATINGMODE
