// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

#ifndef DataConvergParams_hh_INCLUDED
#define DataConvergParams_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace DataConvergParams {

    // Note: Unless otherwise noted, the tolerance parameters listed below were chosen
    // to represent educated guesses at what the tolerances for individual physical
    // parameters should be.
    constexpr Real64 HVACEnthalpyToler(260.0);                  // Tolerance for enthalpy comparisons (in kJ/kgK)
    constexpr Real64 HVACFlowRateToler(0.01);                   // Tolerance for mass flow rate convergence (in kg/s) [~20 CFM]
    constexpr Real64 HVACFlowRateSlopeToler(0.001);             // Slope tolerance for mass flow, kg/s/iteration
    constexpr Real64 HVACFlowRateOscillationToler(0.0000001);   // tolerance for detecting duplicate flow rate in stack
    constexpr Real64 HVACHumRatToler(0.0001);                   // Tolerance for humidity ratio comparisons (kg water/kg dryair)
    constexpr Real64 HVACHumRatSlopeToler(0.00001);             // Slope tolerance for humidity ratio, kg water/kg-dryair/iteration
    constexpr Real64 HVACHumRatOscillationToler(0.00000001);    // tolerance for detecting duplicate humidity ratio in stack
    constexpr Real64 HVACPressToler(10.0);                      // Tolerance for pressure comparisons (in Pascals)
    constexpr Real64 HVACTemperatureToler(0.01);                // Tolerance for temperature comparisons (in degrees C or K)
    constexpr Real64 HVACTemperatureSlopeToler(0.001);          // Slope tolerance for Temperature, Deg C/iteration
    constexpr Real64 HVACTemperatureOscillationToler(0.000001); // tolerance for detecting duplicate temps in stack
    constexpr Real64 HVACEnergyToler(10.0);                     // Tolerance for Energy comparisons (in Watts W)
    constexpr Real64 HVACCO2Toler(0.1);                         // Tolerance for CO2 comparisons (in ppm)
    constexpr Real64 HVACGenContamToler(0.1);                   // Tolerance for generic contaminant comparisons (in ppm)
    // to be consistent, should be 20.d0 (BG Aug 2012)

    constexpr Real64 HVACCpApprox(1004.844); // Air Cp (20C,0.0Kg/Kg) Only for energy Tolerance Calculation
    // Only used to scale the answer for a more intuitive answer for comparison

    constexpr Real64 PlantFlowRateToler(0.001); // Tolerance for mass flow rate convergence (in kg/s) [~2 CFM]
    constexpr Real64 PlantFlowRateOscillationToler(0.0000001);
    constexpr Real64 PlantFlowRateSlopeToler(0.0001); // Slope tolerance for mass flow, kg/s/iteration

    constexpr Real64 PlantPressToler(10.0);                      // Tolerance for pressure comparisons (in Pascals)
    constexpr Real64 PlantTemperatureToler(0.01);                // Tolerance for temperature comparisons (in degrees C or K)
    constexpr Real64 PlantTemperatureSlopeToler(0.001);          // Slope tolerance for Temperature, Deg C/iteration
    constexpr Real64 PlantTemperatureOscillationToler(0.000001); // tolerance for detecting duplicate temps in stack

    constexpr Real64 PlantEnergyToler(10.0); // Tolerance for Energy comparisons (in Watts W)

    // Energy Tolerance Calculation, used to scale the answer for a more intuitive answer for comparison
    constexpr Real64 PlantFlowFlowRateToler(0.01); // Tolerance for mass flow rate convergence (in kg/s)

    constexpr int ConvergLogStackDepth(10);

    enum class CalledFrom
    {
        Invalid = -1,
        AirSystemDemandSide,
        AirSystemSupplySideDeck1,
        AirSystemSupplySideDeck2,
        Num
    };

    struct HVACNodeConvergLogStruct
    {
        int NodeNum = 0;
        std::array<Real64, ConvergLogStackDepth> HumidityRatio;
        std::array<Real64, ConvergLogStackDepth> MassFlowRate;
        std::array<Real64, ConvergLogStackDepth> Temperature;
    };

    struct HVACZoneInletConvergenceStruct
    {
        std::string ZoneName;
        int NumInletNodes = 0; // number of inlet nodes for zone
        Array1D<HVACNodeConvergLogStruct> InletNode;
    };

    struct HVACAirLoopIterationConvergenceStruct
    {
        std::array<bool, 3> HVACMassFlowNotConverged = {false};                               // Flag to show mass flow convergence
        std::array<Real64, ConvergLogStackDepth> HVACFlowDemandToSupplyTolValue = {0.0};      // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACFlowSupplyDeck1ToDemandTolValue = {0.0}; // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACFlowSupplyDeck2ToDemandTolValue = {0.0}; // Queue of convergence "results"
        std::array<bool, 3> HVACHumRatNotConverged = {false};                                 // Flag to show humidity ratio convergence   or failure
        std::array<Real64, ConvergLogStackDepth> HVACHumDemandToSupplyTolValue = {0.0};       // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACHumSupplyDeck1ToDemandTolValue = {0.0};  // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACHumSupplyDeck2ToDemandTolValue = {0.0};  // Queue of convergence "results"
        std::array<bool, 3> HVACTempNotConverged = {false};                                   // Flag to show temperature convergence  or failure
        std::array<Real64, ConvergLogStackDepth> HVACTempDemandToSupplyTolValue = {0.0};      // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACTempSupplyDeck1ToDemandTolValue = {0.0}; // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACTempSupplyDeck2ToDemandTolValue = {0.0}; // Queue of convergence "results"
        std::array<bool, 3> HVACEnergyNotConverged = {false};                                 // Flag to show energy convergence   or failure
        std::array<Real64, ConvergLogStackDepth> HVACEnergyDemandToSupplyTolValue = {0.0};    // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACEnergySupplyDeck1ToDemandTolValue = {0.0};    // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACEnergySupplyDeck2ToDemandTolValue = {0.0};    // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACEnthalpyDemandToSupplyTolValue = {0.0};       // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACEnthalpySupplyDeck1ToDemandTolValue = {0.0};  // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACEnthalpySupplyDeck2ToDemandTolValue = {0.0};  // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACPressureDemandToSupplyTolValue = {0.0};       // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACPressureSupplyDeck1ToDemandTolValue = {0.0};  // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACPressueSupplyDeck2ToDemandTolValue = {0.0};   // Queue of convergence "results"
        std::array<bool, 3> HVACCO2NotConverged = {false};                                         // Flag to show mass flow convergence
        std::array<Real64, ConvergLogStackDepth> HVACCO2DemandToSupplyTolValue = {0.0};            // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACCO2SupplyDeck1ToDemandTolValue = {0.0};       // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACCO2SupplyDeck2ToDemandTolValue = {0.0};       // Queue of convergence "results"
        std::array<bool, 3> HVACGenContamNotConverged = {false};                                   // Flag to show mass flow convergence
        std::array<Real64, ConvergLogStackDepth> HVACGenContamDemandToSupplyTolValue = {0.0};      // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACGenContamSupplyDeck1ToDemandTolValue = {0.0}; // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> HVACGenContamSupplyDeck2ToDemandTolValue = {0.0}; // Queue of convergence "results"
    };

    struct PlantIterationConvergenceStruct
    {
        // Members
        bool PlantMassFlowNotConverged = false;                                           // Flag to show mass flow convergence
        std::array<Real64, ConvergLogStackDepth> PlantFlowDemandToSupplyTolValue = {0.0}; // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> PlantFlowSupplyToDemandTolValue;         // Queue of convergence "results"
        bool PlantTempNotConverged = false;                                               // Flag to show temperature convergence (0) or failure (1)
        std::array<Real64, ConvergLogStackDepth> PlantTempDemandToSupplyTolValue;         // Queue of convergence "results"
        std::array<Real64, ConvergLogStackDepth> PlantTempSupplyToDemandTolValue;         // Queue of convergence "results"
    };

} // namespace DataConvergParams

struct ConvergParamsData : BaseGlobalStruct
{

    Real64 MinTimeStepSys = (1.0 / 60.0); // =1 minute
    Real64 MinTimeStepTol = 1.0e-4;       // = min allowable for ABS(1.-TimeStepSys/(MinTimeStepSys))
    Real64 MaxZoneTempDiff = 0.3;         // 0.3 C = (1% OF 300 C) = max allowable difference between
    int MaxIter = 20;                     // maximum number of iterations allowed
    int MaxPlantSubIterations = 8;        // Iteration Max for Plant Simulation sub iterations
    int MinPlantSubIterations = 2;        // Iteration Min for Plant Simulation sub iterations

    Array1D<DataConvergParams::HVACZoneInletConvergenceStruct> ZoneInletConvergence;
    Array1D<DataConvergParams::HVACAirLoopIterationConvergenceStruct> AirLoopConvergence;
    Array1D<DataConvergParams::PlantIterationConvergenceStruct> PlantConvergence;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) ConvergParamsData();
    }
};

} // namespace EnergyPlus

#endif
