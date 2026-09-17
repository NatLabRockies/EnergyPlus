// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
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

#ifndef PlantCentralHeatPumpSystem_hh_INCLUDED
#define PlantCentralHeatPumpSystem_hh_INCLUDED

// C++ Headers
#include <vector>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PlantCentralHeatPumpSystem {

    enum class CurrentMode
    {
        Invalid = -1,
        Off,
        CoolingOnly,
        HeatingOnly,
        HeatRecovery,
        CoolingDominant,
        HeatingDominant,
        Num
    };

    enum class CondenserTemperatureMode
    {
        Invalid = -1,
        EnteringCondenser,
        LeavingCondenser,
        Num
    };

    enum class SolverConvergenceStatus
    {
        Invalid = -1,
        NotRequired,
        Converged,
        Stagnated,
        IterationLimit,
        Num
    };

    struct SolverStatistics
    {
        SolverConvergenceStatus outerStatus = SolverConvergenceStatus::NotRequired;
        SolverConvergenceStatus partLoadStatus = SolverConvergenceStatus::NotRequired;
        int outerIterations = 0;
        int partLoadIterations = 0;
        int curveEvaluations = 0;
        Real64 temperatureResidual = 0.0;
        Real64 partLoadBracketWidth = 0.0;
        Real64 loadResidual = 0.0;
    };

    struct SolverWarningData
    {
        int count = 0;
        int recurringIndex = 0;
    };

    struct ModuleResult
    {
        CurrentMode currentMode = CurrentMode::Off;
        bool isAvailable = false;
        bool isRunning = false;

        Real64 requestedCoolingLoad = 0.0;        // Cooling load presented to this module [W]
        Real64 requestedHeatingLoad = 0.0;        // Heating load presented to this module [W]
        Real64 availableEvaporatorCapacity = 0.0; // Available evaporator capacity at the operating temperatures [W]
        Real64 availableCondenserCapacity = 0.0;  // Available condenser capacity at the operating temperatures [W]
        Real64 qEvaporator = 0.0;                 // Refrigerant evaporator heat transfer [W]
        Real64 qCondenser = 0.0;                  // Refrigerant condenser heat transfer [W]

        Real64 compressorPower = 0.0;        // Compressor electric power counted once [W]
        Real64 coolingPower = 0.0;           // Compressor power assigned to cooling reporting [W]
        Real64 heatingPower = 0.0;           // Compressor power assigned to heating reporting [W]
        Real64 motorHeatToRefrigerant = 0.0; // Compressor power delivered to the refrigerant [W]
        Real64 motorHeatLoss = 0.0;          // Compressor power rejected outside the refrigerant cycle [W]
        Real64 falseLoadRate = 0.0;          // False loading above useful water-side load [W]

        Real64 partLoadRatio = 0.0;
        Real64 cyclingRatio = 0.0;
        Real64 unloadingRatio = 0.0;
        Real64 capacityTemperatureModifier = 0.0;
        Real64 eirTemperatureModifier = 0.0;
        Real64 eirPartLoadModifier = 0.0;
        Real64 capacityCurveEvaporatorTemp = 0.0;   // Evaporator temperature used by the capacity curve [C]
        Real64 capacityCurveCondenserTemp = 0.0;    // Condenser temperature used by the capacity curve [C]
        Real64 eirCurveEvaporatorTemp = 0.0;        // Evaporator temperature used by the EIR temperature curve [C]
        Real64 eirCurveCondenserTemp = 0.0;         // Condenser temperature used by the EIR temperature curve [C]
        Real64 eirPartLoadCurvePLR = 0.0;           // PLR used to evaluate the EIR part-load curve
        Real64 eirPartLoadCurveCondenserTemp = 0.0; // Condenser temperature used by a bivariate EIR part-load curve [C]
        Real64 actualCOP = 0.0;

        Real64 evaporatorInletTemp = 0.0;
        Real64 evaporatorOutletTemp = 0.0;
        Real64 evaporatorMassFlowRate = 0.0;
        Real64 condenserInletTemp = 0.0;
        Real64 condenserOutletTemp = 0.0;
        Real64 condenserMassFlowRate = 0.0;

        Real64 coolingInletTemp = 0.0;
        Real64 coolingOutletTemp = 0.0;
        Real64 coolingMassFlowRate = 0.0;
        Real64 heatingInletTemp = 0.0;
        Real64 heatingOutletTemp = 0.0;
        Real64 heatingMassFlowRate = 0.0;
        Real64 sourceInletTemp = 0.0;
        Real64 sourceOutletTemp = 0.0;
        Real64 sourceMassFlowRate = 0.0;

        Real64 coolingDelivered = 0.0;   // Useful chilled-water cooling [W]
        Real64 heatingDelivered = 0.0;   // Useful hot-water heating [W]
        Real64 heatRecovered = 0.0;      // Condenser heat routed to heat recovery [W]
        Real64 sourceHeatTransfer = 0.0; // Positive rejects to source; negative extracts from source [W]
        Real64 unmetCoolingLoad = 0.0;   // Remaining system cooling load after this module [W]
        Real64 unmetHeatingLoad = 0.0;   // Remaining system heating load after this module [W]

        SolverStatistics solver;

        Real64 falseLoadEnergy = 0.0;
        Real64 coolingEnergy = 0.0;
        Real64 heatingEnergy = 0.0;
        Real64 evaporatorEnergy = 0.0;
        Real64 condenserEnergy = 0.0;

        void updateEnergies(Real64 secondsInTimeStep)
        {
            this->falseLoadEnergy = this->falseLoadRate * secondsInTimeStep;
            this->coolingEnergy = this->coolingPower * secondsInTimeStep;
            this->heatingEnergy = this->heatingPower * secondsInTimeStep;
            this->evaporatorEnergy = this->qEvaporator * secondsInTimeStep;
            this->condenserEnergy = this->qCondenser * secondsInTimeStep;
        }

        void updatePowerAccounting(Real64 openMotorEfficiency)
        {
            this->compressorPower = this->coolingPower + this->heatingPower;
            this->motorHeatToRefrigerant = this->compressorPower * openMotorEfficiency;
            this->motorHeatLoss = this->compressorPower - this->motorHeatToRefrigerant;
            this->isRunning = this->currentMode == CurrentMode::CoolingOnly || this->currentMode == CurrentMode::HeatingOnly ||
                              this->currentMode == CurrentMode::HeatRecovery || this->currentMode == CurrentMode::CoolingDominant ||
                              this->currentMode == CurrentMode::HeatingDominant;
        }

        [[nodiscard]] Real64 moduleEnergyBalanceResidual() const
        {
            return this->qCondenser - this->qEvaporator - this->motorHeatToRefrigerant - this->falseLoadRate;
        }

        [[nodiscard]] Real64 routingEnergyBalanceResidual() const
        {
            return this->heatingDelivered + this->sourceHeatTransfer - this->coolingDelivered - this->motorHeatToRefrigerant - this->falseLoadRate;
        }
    };

    struct PerformanceData
    {
        std::string Name;
        CondenserTemperatureMode coolingCondenserTemperatureMode = CondenserTemperatureMode::Invalid;
        CondenserTemperatureMode heatingCondenserTemperatureMode = CondenserTemperatureMode::Invalid;
        bool constantFlow = false;
        bool variableFlow = false;
        int coolingCapacityTemperatureCurveIndex = 0;
        int coolingEIRTemperatureCurveIndex = 0;
        int coolingEIRPartLoadCurveIndex = 0;
        int heatingCapacityTemperatureCurveIndex = 0;
        int heatingEIRTemperatureCurveIndex = 0;
        int heatingEIRPartLoadCurveIndex = 0;
        Real64 referenceCoolingCapacity = 0.0;
        bool referenceCoolingCapacityWasAutoSized = false;
        Real64 referenceCoolingCOP = 0.0;
        Real64 coolingReferenceEvaporatorOutletTemp = 0.0;
        Real64 coolingReferenceCondenserInletTemp = 0.0;
        Real64 coolingReferenceCondenserOutletTemp = 0.0;
        Real64 coolingMaximumPartLoadRatio = 0.0;
        Real64 coolingOptimumPartLoadRatio = 0.0;
        Real64 coolingMinimumPartLoadRatio = 0.0;
        Real64 heatingToCoolingCapacityRatio = 0.0;
        Real64 heatingToCoolingPowerRatio = 0.0;
        Real64 referenceHeatingCapacity = 0.0;
        Real64 referenceHeatingCOP = 0.0;
        Real64 referenceHeatingPower = 0.0;
        Real64 heatingReferenceEvaporatorOutletTemp = 0.0;
        Real64 heatingReferenceCondenserInletTemp = 0.0;
        Real64 heatingReferenceCondenserOutletTemp = 0.0;
        Real64 minimumEvaporatorOutletTemp = 0.0;
        Real64 heatingMaximumPartLoadRatio = 0.0;
        Real64 heatingOptimumPartLoadRatio = 0.0;
        Real64 heatingMinimumPartLoadRatio = 0.0;
        Real64 designEvaporatorVolFlowRate = 0.0;
        bool designEvaporatorVolFlowRateWasAutoSized = false;
        Real64 designCondenserVolFlowRate = 0.0;
        bool designCondenserVolFlowRateWasAutoSized = false;
        Real64 designHeatingVolFlowRate = 0.0;
        Real64 compressorMotorEfficiency = 0.0;
        Real64 sizingFactor = 1.0;
    };

    struct ModuleSizingData
    {
        Real64 referenceCoolingCapacity = 0.0;
        Real64 referenceHeatingCapacity = 0.0;
        Real64 referenceHeatingCOP = 0.0;
        Real64 referenceHeatingPower = 0.0;
        Real64 designEvaporatorVolFlowRate = 0.0;
        Real64 temporaryEvaporatorVolFlowRate = 0.0;
        Real64 designCondenserVolFlowRate = 0.0;
        Real64 temporaryCondenserVolFlowRate = 0.0;
        Real64 maximumCondenserMassFlowRate = 0.0;
        Real64 maximumEvaporatorMassFlowRate = 0.0;
        Real64 maximumCoolingMassFlowRate = 0.0;
        Real64 maximumHeatingMassFlowRate = 0.0;
        Real64 maximumSourceEvaporatorMassFlowRate = 0.0;
        Real64 maximumSourceCondenserMassFlowRate = 0.0;
    };

    struct ModePerformanceData
    {
        CondenserTemperatureMode condenserMode = CondenserTemperatureMode::Invalid;
        int capacityTemperatureCurveIndex = 0;
        int eirTemperatureCurveIndex = 0;
        int eirPartLoadCurveIndex = 0;
        Real64 referenceEvaporatorCapacity = 0.0;
        Real64 referenceCOP = 0.0;
        Real64 referenceEvaporatorLeavingTemp = 0.0;
        Real64 referenceCondenserEnteringTemp = 0.0;
        Real64 referenceCondenserLeavingTemp = 0.0;
        Real64 minimumPartLoadRatio = 0.0;
        Real64 maximumPartLoadRatio = 0.0;
        Real64 optimumPartLoadRatio = 0.0;
    };

    struct Module
    {
        PerformanceData const *performance = nullptr;
        Sched::Schedule *availabilitySchedule = nullptr;
        bool variableFlow = false;
        ModuleSizingData sizing;
        Real64 minimumEvaporatorOutletTemp = 0.0;
        int capacityCurveErrorCount = 0;
        int capacityCurveErrorIndex = 0;
        SolverWarningData coolingSolverWarning;
        SolverWarningData heatingSolverWarning;
        SolverWarningData heatingPartLoadSolverWarning;
        SolverWarningData simultaneousSolverWarning;
        SolverWarningData simultaneousPartLoadSolverWarning;
        ModuleResult result;

        void initialize(PerformanceData const &performanceData, Sched::Schedule *schedule);
        [[nodiscard]] PerformanceData const &performanceData() const;
        [[nodiscard]] std::string const &name() const;
        [[nodiscard]] bool isAvailable() const;
        [[nodiscard]] ModePerformanceData coolingModePerformance() const;
        [[nodiscard]] ModePerformanceData heatingModePerformance() const;
        void mapResultToPlantConnections();
        void updateResultEnergies(Real64 secondsInTimeStep);
        void resetResult(Real64 evaporatorInletTemp, Real64 condenserInletTemp);
    };

    struct SystemReportData
    {
        Real64 coolingElectricEnergy = 0.0;     // System cooling electric consumption [J]
        Real64 heatingElectricEnergy = 0.0;     // System heating electric consumption [J]
        Real64 coolingHeatTransferEnergy = 0.0; // Chilled water heat transfer energy [J]
        Real64 heatingHeatTransferEnergy = 0.0; // Hot water heat transfer energy [J]
        Real64 sourceHeatTransferEnergy = 0.0;  // Source loop heat transfer energy [J]
        Real64 coolingElectricPower = 0.0;      // System cooling electric consumption rate [W]
        Real64 heatingElectricPower = 0.0;      // System heating electric consumption rate [W]
        Real64 coolingHeatTransferRate = 0.0;   // Chilled water heat transfer rate [W]
        Real64 heatingHeatTransferRate = 0.0;   // Hot water heat transfer rate [W]
        Real64 sourceHeatTransferRate = 0.0;    // Source loop heat transfer rate [W]
        Real64 coolingInletTemp = 0.0;          // Chilled water inlet temperature [C]
        Real64 heatingInletTemp = 0.0;          // Hot water inlet temperature [C]
        Real64 sourceInletTemp = 0.0;           // Source loop inlet temperature [C]
        Real64 coolingOutletTemp = 0.0;         // Chilled water Outlet temperature [C]
        Real64 heatingOutletTemp = 0.0;         // Hot water Outlet temperature [C]
        Real64 sourceOutletTemp = 0.0;          // Source loop Outlet temperature [C]
        Real64 coolingMassFlowRate = 0.0;       // Cooling loop mass flow rate [kg/s]
        Real64 heatingMassFlowRate = 0.0;       // Heating loop mass flow rate [kg/s]
        Real64 sourceMassFlowRate = 0.0;        // Source loop mass flow rate [kg/s]
    };

    struct CentralHeatPumpSystem : PlantComponent
    {
        std::string Name;                               // User identifier
        bool allModulesVariableFlow = false;            // True when every module uses variable-flow control
        Sched::Schedule *ancillaryPowerSched = nullptr; // Schedule value for ancillary power control
        int coolingInletNodeNum = 0;                    // Node number on the inlet side of the plant (Chilled Water side)
        int coolingOutletNodeNum = 0;                   // Node number on the outlet side of the plant (Chilled Water side)
        int heatingInletNodeNum = 0;                    // Node number on the inlet side of the plant (Hot water side)
        int heatingOutletNodeNum = 0;                   // Node number on the outlet side of the plant (Hot water side)
        int sourceInletNodeNum = 0;                     // Node number on the inlet side of the plant (source side)
        int sourceOutletNodeNum = 0;                    // Node number on the outlet side of the plant (source side)
        int coolingSetpointNodeNum = 0;                 // Node number of the cooling setpoint temperature node
        int heatingSetpointNodeNum = 0;                 // Node number of the heating setpoint temperature node
        bool checkMinimumEvaporatorOutletTemp = true;   // True if minimum evaporator outlet temperature warning is enabled
        Real64 coolingMassFlowRateMax = 0.0;            // Maximum chilled water mass flow rate
        Real64 heatingMassFlowRateMax = 0.0;            // Maximum hot water mass flow rate
        Real64 sourceMassFlowRateMax = 0.0;             // Maximum Source loop mass flow rate
        Real64 requestedCoolingLoad = 0.0;              // Cooling demand for the central heat pump system
        Real64 requestedHeatingLoad = 0.0;              // Heating demand for the central heat pump system
        Real64 ancillaryPower = 0.0;                    // System ancillary power
        std::vector<Module> modules;                    // Expanded runtime modules
        bool coolingSetpointErrorIssued = false;        // true if setpoint warning issued
        bool heatingSetpointErrorIssued = false;        // true if setpoint warning issued
        PlantLocation coolingPlantLoc = {};             // Chilled water plant loop component index
        PlantLocation heatingPlantLoc = {};             // Hot water plant loop component index
        PlantLocation sourcePlantLoc = {};              // Source plant loop component location
        Real64 coolingVolFlowRate = 0.0;                // Cooling loop volume flow rate [m3/s]
        Real64 heatingVolFlowRate = 0.0;                // Heating loop volume flow rate [m3/s]
        Real64 sourceVolFlowRate = 0.0;                 // Source loop volume flow rate [m3/s]
        bool plantScanPending = true;
        bool environmentInitPending = true;
        bool isCoolingDominant = false;
        bool isHeatingDominant = false;
        SystemReportData report;
        bool setupOutputVarsFlag = true;
        bool mySizesReported = false;

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void getSizingFactor(Real64 &sizingFactor) override;

        void getDesignCapacities(
            EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &maxLoad, Real64 &minLoad, Real64 &optimalLoad) override;

        void setupOutputVars(EnergyPlusData &state);

        void initialize(EnergyPlusData &state,
                        Real64 load, // Demand Load
                        int loopNum, // Loop Number Index
                        bool runFlag = true);

        void initializeDesignFlowLimits(EnergyPlusData &state);

        void resolveFlowMode(EnergyPlusData &state);

        void resetOffState(EnergyPlusData &state, bool releasePlantFlows = true);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool firstHVACIteration,
                      Real64 &currentLoad,
                      bool runFlag) override;

        void size(EnergyPlusData &state);

        void calculate(EnergyPlusData &state, Real64 &load, int loopNum);

        void calculateCoolingOnly(
            EnergyPlusData &state, Real64 coolingMassFlowRate, Real64 sourceMassFlowRate, Real64 coolingInletTemp, Real64 sourceInletTemp);

        void calculateHeatingOnly(
            EnergyPlusData &state, Real64 heatingMassFlowRate, Real64 sourceMassFlowRate, Real64 heatingInletTemp, Real64 sourceInletTemp);

        ModuleResult solveCoolingOnly(EnergyPlusData &state,
                                      std::size_t moduleIndex,
                                      Real64 coolingLoad,
                                      Real64 evaporatorMassFlowRateMax,
                                      Real64 condenserMassFlowRate,
                                      Real64 evaporatorInletTemp,
                                      Real64 condenserInletTemp);

        ModuleResult solveHeatingOnly(EnergyPlusData &state,
                                      std::size_t moduleIndex,
                                      Real64 heatingLoad,
                                      Real64 evaporatorMassFlowRate,
                                      Real64 condenserMassFlowRateMax,
                                      Real64 evaporatorInletTemp,
                                      Real64 condenserInletTemp);

        ModuleResult solveSimultaneous(EnergyPlusData &state,
                                       std::size_t moduleIndex,
                                       Real64 coolingLoad,
                                       Real64 heatingLoad,
                                       Real64 maximumCoolingMassFlowRate,
                                       Real64 maximumHeatingMassFlowRate,
                                       Real64 maximumSourceMassFlowRate,
                                       Real64 coolingInletTemp,
                                       Real64 heatingInletTemp,
                                       Real64 sourceInletTemp);

        void calculateSimultaneous(EnergyPlusData &state,
                                   Real64 coolingMassFlowRate,
                                   Real64 heatingMassFlowRate,
                                   Real64 sourceMassFlowRate,
                                   Real64 coolingInletTemp,
                                   Real64 heatingInletTemp,
                                   Real64 sourceInletTemp);

        void updateReportingAndNodes(EnergyPlusData const &state,
                                     Real64 coolingMassFlowRate,
                                     Real64 heatingMassFlowRate,
                                     Real64 sourceMassFlowRate,
                                     Real64 coolingInletTemp,
                                     Real64 heatingInletTemp,
                                     Real64 sourceInletTemp);

        static Real64
        selectCondenserCurveTemperature(ModePerformanceData const &modePerformance, Real64 condenserEnteringTemp, Real64 condenserLeavingTemp);

        static Real64 evaluateCapacityTemperatureModifier(
            EnergyPlusData &state, Module &module, ModePerformanceData const &modePerformance, Real64 evaporatorOutletTemp, Real64 condenserTemp);

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation) override;

        void oneTimeInit(EnergyPlusData &state) override;
    };

    void getCentralHeatPumpSystemInput(EnergyPlusData &state);

    void getPerformanceInput(EnergyPlusData &state);

} // namespace PlantCentralHeatPumpSystem

struct PlantCentralHeatPumpSystemData : BaseGlobalStruct
{

    bool getSystemInputFlag = true;   // When TRUE, calls subroutine to read input file.
    int numPerformanceReferences = 0; // Number of system performance references
    std::vector<PlantCentralHeatPumpSystem::CentralHeatPumpSystem> systems;
    std::vector<PlantCentralHeatPumpSystem::PerformanceData> performanceDefinitions;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) PlantCentralHeatPumpSystemData();
    }
};

} // namespace EnergyPlus

#endif
