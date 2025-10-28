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

#ifndef ENERGYPLUS_COILS_COIL_COOLING_DX_PERFORMANCE_BASE
#define ENERGYPLUS_COILS_COIL_COOLING_DX_PERFORMANCE_BASE

#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <optional>
#include <string>
#include <vector>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

struct CoilCoolingDXPerformanceBase
{
    CoilCoolingDXPerformanceBase() = default;

    std::string name;
    std::string parentName;

    // standard rating stuff -- for now just 210/240
    Real64 standardRatingCoolingCapacity = 0.0; // net cooling capacity of single speed DX cooling coil
    Real64 standardRatingSEER = 0.0;            // seasonal energy efficiency ratio of single speed DX cooling coil
    Real64 standardRatingSEER_Standard = 0.0;   // seasonal energy efficiency ratio
    Real64 standardRatingEER = 0.0;             // energy efficiency ratio of single speed DX cooling coil
    Real64 standardRatingIEER = 0.0;            // Integrated energy efficiency ratio of single speed DX cooling coil

    // standard rating stuff -- for now just 210/240 2023
    Real64 standardRatingCoolingCapacity2023 = 0.0; // net cooling capacity of single speed DX cooling coil
    Real64 standardRatingSEER2_User = 0.0;          // seasonal energy efficiency ratio of single speed DX cooling coil
    Real64 standardRatingSEER2_Standard = 0.0;
    Real64 standardRatingEER2 = 0.0;  // energy efficiency ratio of single speed DX cooling coil
    Real64 standardRatingIEER2 = 0.0; // Integrated energy efficiency ratio of singgle speed DX cooling coil | AHRI Std.340/360-2022(IP)

    Real64 powerUse = 0.0;
    Real64 RTF = 0.0;
    Real64 wasteHeatRate = 0.0;
    Real64 recoveredEnergyRate = 0.0;

    Real64 crankcaseHeaterCap = 0.0;
    Real64 crankcaseHeaterPower = 0.0;
    int crankcaseHeaterCapacityCurveIndex = 0;
    Real64 crankcaseHeaterElectricityConsumption = 0.0;
    Constant::eFuel compressorFuelType = Constant::eFuel::Invalid;
    std::string compressorFuelTypeForOutput;
    Real64 compressorFuelRate = 0.0;
    Real64 compressorFuelConsumption = 0.0;
    Real64 electricityConsumption = 0.0;
    Real64 evapCondBasinHeatCap = 0.0;
    Real64 basinHeaterPower = 0.0;
    Real64 basinHeaterElectricityConsumption = 0.0;
    Real64 minOutdoorDrybulb = 0.0;
    // int hasAlternateMode = 0; // 0 Normal, 1 Enhanced, 2 SubcoolReheat
    int OperatingMode = 0;
    Real64 ModeRatio = 0.0;
    Real64 NormalSHR = 0.0;
    HVAC::CoilMode maxAvailCoilMode = HVAC::CoilMode::Normal; // max available coil mode, 0 Normal, 1 Enhanced, 2 SubcoolReheat
    Sched::Schedule *coilCoolingDXAvailSched = nullptr;

    enum CapControlMethod
    {
        CONTINUOUS,
        DISCRETE
    };
    CapControlMethod capControlMethod = CapControlMethod::DISCRETE;

    virtual void size(EnergyPlusData &)
    {
    }

    virtual void simulate(EnergyPlusData &,
                          const DataLoopNode::NodeData &,
                          DataLoopNode::NodeData &,
                          HVAC::CoilMode,
                          // Real64 &,
                          int,
                          Real64,
                          HVAC::FanOp const,
                          DataLoopNode::NodeData &,
                          DataLoopNode::NodeData &,
                          bool const,
                          Real64 = 0.0)
    {
    }

    virtual Real64 ratedCBF(EnergyPlusData &state) = 0;

    virtual Real64 grossRatedSHR(EnergyPlusData &state) = 0;

    virtual Real64 ratedEvapAirMassFlowRate(EnergyPlusData &state) = 0;

    virtual Real64 ratedEvapAirFlowRate(EnergyPlusData &state) = 0;

    virtual Real64 ratedGrossTotalCap() = 0;

    virtual int numSpeeds() = 0;

    virtual Real64 evapAirFlowRateAtSpeedIndex(EnergyPlusData &state, int) = 0;

    virtual Real64 ratedTotalCapacityAtSpeedIndex(EnergyPlusData &, int) = 0;

    virtual Real64 grossRatedCoolingCOPAtMaxSpeed(EnergyPlusData &state)
    {
        ShowFatalError(state, "Gross Rated Cooling COP is not an available parameter for this coil type.");
        return 0.0;
    }

    virtual const std::string_view nameAtSpeed(int)
    {
        return "";
    }

    virtual Real64 ratedAirMassFlowRateMaxSpeed(EnergyPlusData &state, [[maybe_unused]] HVAC::CoilMode const mode = HVAC::CoilMode::Normal)
    {
        ShowFatalError(state, "Rated Air Mass Flow Rate is not an available parameter for this coil type.");
        return 0.0;
    }

    virtual Real64 ratedAirMassFlowRateMinSpeed(EnergyPlusData &state, [[maybe_unused]] HVAC::CoilMode const mode = HVAC::CoilMode::Normal)
    {
        ShowFatalError(state, "Rated Air Mass Flow Rate is not an available parameter for this coil type.");
        return 0.0;
    }

    virtual Real64 ratedCondAirMassFlowRateNomSpeed([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] HVAC::CoilMode const mode)
    {
        return 0.0;
    }

    virtual int indexCapFT([[maybe_unused]] HVAC::CoilMode const mode)
    {
        return 0;
    }

    virtual bool subcoolReheatFlag()
    {
        return false;
    }

    virtual void calcStandardRatings210240(EnergyPlusData &)
    {
    }

    virtual void setToHundredPercentDOAS()
    {
    }

    virtual Real64 currentEvapCondPumpPowerAtSpeed(EnergyPlusData &state, int)
    {
        ShowFatalError(state, "Current Evaporator Condenser Pump Power is not an available parameter for this coil type.");
        return 0.0;
    }

    virtual Real64 evapCondenserEffectivenessAtSpeedIndex(EnergyPlusData &state, int)
    {
        ShowFatalError(state, "Evaporator Condenser Effectiveness is not an available parameter for this coil type.");
        return 0.0;
    }

    virtual Real64 evapAirFlowFraction(EnergyPlusData &state)
    {
        ShowFatalError(state, "Evaporator Air Flow Fraction is not an available parameter for this coil type.");
        return 0.0;
    }
};

} // namespace EnergyPlus
#endif // ENERGYPLUS_COILS_COIL_COOLING_DX_PERFORMANCE_BASE
