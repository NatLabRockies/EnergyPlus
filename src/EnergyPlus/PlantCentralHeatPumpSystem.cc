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

// C++ Headers
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <format>
#include <limits>
#include <string>
#include <unordered_set>
#include <vector>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Formatters.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantCentralHeatPumpSystem.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::PlantCentralHeatPumpSystem {

// MODULE INFORMATION:
//       AUTHOR         PNNL
//       DATE WRITTEN   Feb 2013
//       MODIFIED       na
//       RE-ENGINEERED  na
// PURPOSE OF THIS MODULE:
// This module simulates the performance of the CentralHeatPumpSystem objects
// It currently includes one object: ChillerHeaterPerformance:Electric:EIR.
// The other object available for this CentralHeatPumpSystem object such as
// HeatPumpPerformance:WaterToWater:EIR will be implemented later.

// METHODOLOGY EMPLOYED:
//  Once the PlantLoopManager determines that the CentralHeatPumpSystem
//  is available to meet a loop cooling and heating demands, it calls simulate
//  which in turn calls the electric PlantCentralHeatPumpSystem model. The
//  PlantCentralHeatPumpSystem model is based on polynomial fits of chiller/heater or heat
//  pump performance data.

namespace {

    void getPartLoadCurveMinMax(EnergyPlusData &state, int const curveIndex, Real64 &minimumPartLoadRatio, Real64 &maximumPartLoadRatio)
    {
        auto const *curve = state.dataCurveManager->curves(curveIndex);
        if (curve->numDims == 1) {
            Curve::GetCurveMinMaxValues(state, curveIndex, minimumPartLoadRatio, maximumPartLoadRatio);
        } else {
            Real64 condenserTempMinimum = 0.0;
            Real64 condenserTempMaximum = 0.0;
            Curve::GetCurveMinMaxValues(state, curveIndex, condenserTempMinimum, condenserTempMaximum, minimumPartLoadRatio, maximumPartLoadRatio);
        }
    }

    Real64 evaluatePartLoadCurve(EnergyPlusData &state, int const curveIndex, Real64 const condenserTemp, Real64 const partLoadRatio)
    {
        auto const *curve = state.dataCurveManager->curves(curveIndex);
        return curve->numDims == 1 ? Curve::CurveValue(state, curveIndex, partLoadRatio)
                                   : Curve::CurveValue(state, curveIndex, condenserTemp, partLoadRatio);
    }

    struct PartLoadOperatingPoint
    {
        Real64 evaporatorLoad = 0.0;
        Real64 condenserLoad = 0.0;
        Real64 compressorPower = 0.0;
        Real64 falseLoadRate = 0.0;
        Real64 partLoadRatio = 0.0;
        Real64 cyclingRatio = 0.0;
        Real64 eirPartLoadModifier = 0.0;
    };

    // A unit-width PLR bracket reaches the 1.0e-12 tolerance in 40 subdivisions; 50 leaves conservative headroom.
    constexpr int maxOuterSolverIterations = 100;
    constexpr int maxPartLoadSolverIterations = 50;
    constexpr Real64 temperatureConvergenceTolerance = 1.0e-8;
    constexpr Real64 partLoadAbsoluteTolerance = 1.0e-12;
    constexpr Real64 loadAbsoluteTolerance = 1.0e-7;
    constexpr Real64 loadRelativeTolerance = 1.0e-12;

    struct BisectionResult
    {
        Real64 value = 0.0;
        Real64 bracketWidth = 0.0;
        Real64 loadResidual = 0.0;
        int iterations = 0;
        SolverConvergenceStatus status = SolverConvergenceStatus::NotRequired;
    };

    template <typename LoadAtValue>
    BisectionResult solveBisection(
        Real64 lowerValue, Real64 upperValue, Real64 const targetLoad, Real64 const valueTolerance, Real64 const loadScale, LoadAtValue loadAtValue)
    {
        BisectionResult result;
        Real64 previousValue = std::numeric_limits<Real64>::quiet_NaN();
        Real64 const loadTolerance = max(loadAbsoluteTolerance, loadRelativeTolerance * loadScale);

        for (int iteration = 0; iteration < maxPartLoadSolverIterations; ++iteration) {
            Real64 const candidateValue = 0.5 * (lowerValue + upperValue);
            Real64 const candidateLoad = loadAtValue(candidateValue);
            result.value = candidateValue;
            result.loadResidual = std::abs(candidateLoad - targetLoad);
            result.iterations = iteration + 1;

            if (!std::isfinite(candidateValue) || !std::isfinite(candidateLoad) || !std::isfinite(result.loadResidual)) {
                result.status = SolverConvergenceStatus::Invalid;
                break;
            }

            bool const stagnated = candidateValue == lowerValue || candidateValue == upperValue || candidateValue == previousValue;
            if (candidateLoad < targetLoad) {
                lowerValue = candidateValue;
            } else {
                upperValue = candidateValue;
            }
            result.bracketWidth = upperValue - lowerValue;

            if (result.bracketWidth <= valueTolerance && result.loadResidual <= loadTolerance) {
                result.status = SolverConvergenceStatus::Converged;
                break;
            }
            if (stagnated) {
                result.status = SolverConvergenceStatus::Stagnated;
                break;
            }
            previousValue = candidateValue;
        }

        if (result.status == SolverConvergenceStatus::NotRequired) {
            result.status = SolverConvergenceStatus::IterationLimit;
        }
        return result;
    }

    std::string_view solverStatusName(SolverConvergenceStatus const status)
    {
        switch (status) {
        case SolverConvergenceStatus::NotRequired:
            return "not required";
        case SolverConvergenceStatus::Converged:
            return "converged";
        case SolverConvergenceStatus::Stagnated:
            return "floating-point stagnation";
        case SolverConvergenceStatus::IterationLimit:
            return "iteration limit";
        case SolverConvergenceStatus::Invalid:
            return "invalid numerical value";
        default:
            assert(false);
            return "unknown";
        }
    }

    bool solverFailed(SolverConvergenceStatus const status)
    {
        return status != SolverConvergenceStatus::NotRequired && status != SolverConvergenceStatus::Converged;
    }

    void reportSolverFailure(EnergyPlusData &state,
                             std::string const &systemName,
                             std::string const &moduleName,
                             int const moduleNum,
                             std::string_view const operatingMode,
                             std::string_view const iterationType,
                             SolverConvergenceStatus const status,
                             std::string const &requestedLoads,
                             int const iterations,
                             Real64 const bracketOrTemperatureResidual,
                             Real64 const loadResidual,
                             Real64 const finalOperatingPoint,
                             std::string_view const finalOperatingPointUnits,
                             SolverWarningData &warning)
    {
        ++warning.count;
        if (warning.count == 1) {
            ShowWarningError(state,
                             std::format(R"(CentralHeatPumpSystem "{}" module {} ("{}") {} {} failed to converge ({}).)",
                                         systemName,
                                         moduleNum,
                                         moduleName,
                                         operatingMode,
                                         iterationType,
                                         solverStatusName(status)));
            ShowContinueError(state,
                              std::format("Requested {}; iterations={}; bracket/temperature residual={:.6g}; load residual={:.6g} W; "
                                          "final operating point={:.6g} {}.",
                                          requestedLoads,
                                          iterations,
                                          bracketOrTemperatureResidual,
                                          loadResidual,
                                          finalOperatingPoint,
                                          finalOperatingPointUnits));
            return;
        }

        ShowRecurringWarningErrorAtEnd(state,
                                       std::format(R"(CentralHeatPumpSystem "{}" module {} ("{}") {} {} convergence failure continues.)",
                                                   systemName,
                                                   moduleNum,
                                                   moduleName,
                                                   operatingMode,
                                                   iterationType),
                                       warning.recurringIndex,
                                       bracketOrTemperatureResidual,
                                       std::string(finalOperatingPointUnits));
    }

} // namespace

void Module::initialize(PerformanceData const &performanceData, Sched::Schedule *const schedule)
{
    this->performance = &performanceData;
    this->availabilitySchedule = schedule;
    this->variableFlow = performanceData.variableFlow;
    this->sizing = ModuleSizingData();
    this->sizing.referenceCoolingCapacity = performanceData.referenceCoolingCapacity;
    this->sizing.referenceHeatingCapacity = performanceData.referenceHeatingCapacity;
    this->sizing.referenceHeatingCOP = performanceData.referenceHeatingCOP;
    this->sizing.referenceHeatingPower = performanceData.referenceHeatingPower;
    this->sizing.designEvaporatorVolFlowRate = performanceData.designEvaporatorVolFlowRate;
    this->sizing.designCondenserVolFlowRate = performanceData.designCondenserVolFlowRate;
    this->minimumEvaporatorOutletTemp = 0.0;
    this->capacityCurveErrorCount = 0;
    this->capacityCurveErrorIndex = 0;
    this->coolingSolverWarning = SolverWarningData();
    this->heatingSolverWarning = SolverWarningData();
    this->heatingPartLoadSolverWarning = SolverWarningData();
    this->simultaneousSolverWarning = SolverWarningData();
    this->simultaneousPartLoadSolverWarning = SolverWarningData();
    this->result = ModuleResult();
}

PerformanceData const &Module::performanceData() const
{
    assert(this->performance != nullptr);
    return *this->performance;
}

std::string const &Module::name() const
{
    return this->performanceData().Name;
}

bool Module::isAvailable() const
{
    return this->availabilitySchedule == nullptr || this->availabilitySchedule->getCurrentVal() > 0.0;
}

ModePerformanceData Module::coolingModePerformance() const
{
    auto const &definition = this->performanceData();
    ModePerformanceData mode;
    mode.condenserMode = definition.coolingCondenserTemperatureMode;
    mode.capacityTemperatureCurveIndex = definition.coolingCapacityTemperatureCurveIndex;
    mode.eirTemperatureCurveIndex = definition.coolingEIRTemperatureCurveIndex;
    mode.eirPartLoadCurveIndex = definition.coolingEIRPartLoadCurveIndex;
    mode.referenceEvaporatorCapacity = this->sizing.referenceCoolingCapacity;
    mode.referenceCOP = definition.referenceCoolingCOP;
    mode.referenceEvaporatorLeavingTemp = definition.coolingReferenceEvaporatorOutletTemp;
    mode.referenceCondenserEnteringTemp = definition.coolingReferenceCondenserInletTemp;
    mode.referenceCondenserLeavingTemp = definition.coolingReferenceCondenserOutletTemp;
    mode.minimumPartLoadRatio = definition.coolingMinimumPartLoadRatio;
    mode.maximumPartLoadRatio = definition.coolingMaximumPartLoadRatio;
    mode.optimumPartLoadRatio = definition.coolingOptimumPartLoadRatio;
    return mode;
}

ModePerformanceData Module::heatingModePerformance() const
{
    auto const &definition = this->performanceData();
    ModePerformanceData mode;
    mode.condenserMode = definition.heatingCondenserTemperatureMode;
    mode.capacityTemperatureCurveIndex = definition.heatingCapacityTemperatureCurveIndex;
    mode.eirTemperatureCurveIndex = definition.heatingEIRTemperatureCurveIndex;
    mode.eirPartLoadCurveIndex = definition.heatingEIRPartLoadCurveIndex;
    mode.referenceEvaporatorCapacity = this->sizing.referenceHeatingCapacity;
    mode.referenceCOP = this->sizing.referenceHeatingCOP;
    mode.referenceEvaporatorLeavingTemp = definition.heatingReferenceEvaporatorOutletTemp;
    mode.referenceCondenserEnteringTemp = definition.heatingReferenceCondenserInletTemp;
    mode.referenceCondenserLeavingTemp = definition.heatingReferenceCondenserOutletTemp;
    mode.minimumPartLoadRatio = definition.heatingMinimumPartLoadRatio;
    mode.maximumPartLoadRatio = definition.heatingMaximumPartLoadRatio;
    mode.optimumPartLoadRatio = definition.heatingOptimumPartLoadRatio;
    return mode;
}

void Module::mapResultToPlantConnections()
{
    auto &moduleResult = this->result;

    moduleResult.coolingInletTemp = 0.0;
    moduleResult.coolingOutletTemp = 0.0;
    moduleResult.coolingMassFlowRate = 0.0;
    moduleResult.heatingInletTemp = 0.0;
    moduleResult.heatingOutletTemp = 0.0;
    moduleResult.heatingMassFlowRate = 0.0;
    moduleResult.sourceInletTemp = 0.0;
    moduleResult.sourceOutletTemp = 0.0;
    moduleResult.sourceMassFlowRate = 0.0;
    moduleResult.coolingDelivered = 0.0;
    moduleResult.heatingDelivered = 0.0;
    moduleResult.heatRecovered = 0.0;
    moduleResult.sourceHeatTransfer = 0.0;

    switch (moduleResult.currentMode) {
    case CurrentMode::CoolingOnly:
    case CurrentMode::CoolingDominant:
        moduleResult.coolingInletTemp = moduleResult.evaporatorInletTemp;
        moduleResult.coolingOutletTemp = moduleResult.evaporatorOutletTemp;
        moduleResult.coolingMassFlowRate = moduleResult.evaporatorMassFlowRate;
        moduleResult.sourceInletTemp = moduleResult.condenserInletTemp;
        moduleResult.sourceOutletTemp = moduleResult.condenserOutletTemp;
        moduleResult.sourceMassFlowRate = moduleResult.condenserMassFlowRate;
        moduleResult.coolingDelivered = moduleResult.qEvaporator;
        moduleResult.sourceHeatTransfer = moduleResult.qCondenser;
        break;
    case CurrentMode::HeatingOnly:
    case CurrentMode::HeatingDominant:
        moduleResult.heatingInletTemp = moduleResult.condenserInletTemp;
        moduleResult.heatingOutletTemp = moduleResult.condenserOutletTemp;
        moduleResult.heatingMassFlowRate = moduleResult.condenserMassFlowRate;
        moduleResult.sourceInletTemp = moduleResult.evaporatorInletTemp;
        moduleResult.sourceOutletTemp = moduleResult.evaporatorOutletTemp;
        moduleResult.sourceMassFlowRate = moduleResult.evaporatorMassFlowRate;
        moduleResult.heatingDelivered = moduleResult.qCondenser;
        moduleResult.sourceHeatTransfer = -moduleResult.qEvaporator;
        break;
    case CurrentMode::HeatRecovery:
        moduleResult.coolingInletTemp = moduleResult.evaporatorInletTemp;
        moduleResult.coolingOutletTemp = moduleResult.evaporatorOutletTemp;
        moduleResult.coolingMassFlowRate = moduleResult.evaporatorMassFlowRate;
        moduleResult.heatingInletTemp = moduleResult.condenserInletTemp;
        moduleResult.heatingOutletTemp = moduleResult.condenserOutletTemp;
        moduleResult.heatingMassFlowRate = moduleResult.condenserMassFlowRate;
        moduleResult.coolingDelivered = moduleResult.qEvaporator;
        moduleResult.heatingDelivered = moduleResult.qCondenser;
        moduleResult.heatRecovered = moduleResult.qCondenser;
        break;
    case CurrentMode::Off:
    case CurrentMode::Invalid:
    case CurrentMode::Num:
        break;
    }

    moduleResult.updatePowerAccounting(this->performanceData().compressorMotorEfficiency);
}

void Module::updateResultEnergies(Real64 const secondsInTimeStep)
{
    this->result.updateEnergies(secondsInTimeStep);
}

void Module::resetResult(Real64 const evaporatorInletTemp, Real64 const condenserInletTemp)
{
    this->result = ModuleResult();
    this->result.evaporatorInletTemp = evaporatorInletTemp;
    this->result.evaporatorOutletTemp = evaporatorInletTemp;
    this->result.condenserInletTemp = condenserInletTemp;
    this->result.condenserOutletTemp = condenserInletTemp;
    this->mapResultToPlantConnections();
}

PlantComponent *CentralHeatPumpSystem::factory(EnergyPlusData &state, std::string const &objectName)
{
    // Process the input data
    if (state.dataPlantCentralHeatPumpSystem->getSystemInputFlag) {
        getCentralHeatPumpSystemInput(state);
        state.dataPlantCentralHeatPumpSystem->getSystemInputFlag = false;
    }

    auto &systems = state.dataPlantCentralHeatPumpSystem->systems;
    if (auto const system = std::ranges::find_if(systems, [&objectName](auto const &candidate) { return candidate.Name == objectName; });
        system != systems.end()) {
        return &*system;
    }

    ShowFatalError(state,
                   std::format("CentralHeatPumpSystem::factory: Error "
                               "getting inputs for object named: {}",
                               objectName)); // LCOV_EXCL_LINE
}

void CentralHeatPumpSystem::onInitLoopEquip(EnergyPlusData &state, const PlantLocation &calledFromLocation)
{
    this->initialize(state, 0.0, calledFromLocation.loopNum, false);
    this->size(state);
}

void CentralHeatPumpSystem::getDesignCapacities(
    [[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &maxLoad, Real64 &minLoad, Real64 &optimalLoad)
{
    minLoad = 0.0;
    maxLoad = 0.0;
    optimalLoad = 0.0;
    Real64 minimumStageLoad = std::numeric_limits<Real64>::max();
    auto accumulateCapacity = [&minimumStageLoad](ModePerformanceData const &mode, Real64 &maximumLoad, Real64 &optimumLoad) {
        if (mode.referenceEvaporatorCapacity <= 0.0) {
            return;
        }
        maximumLoad += mode.referenceEvaporatorCapacity * max(0.0, mode.maximumPartLoadRatio);
        optimumLoad += mode.referenceEvaporatorCapacity * max(0.0, mode.optimumPartLoadRatio);
        minimumStageLoad = min(minimumStageLoad, mode.referenceEvaporatorCapacity * max(0.0, mode.minimumPartLoadRatio));
    };

    if (calledFromLocation.loopNum == this->coolingPlantLoc.loopNum) {
        for (auto const &module : this->modules) {
            accumulateCapacity(module.coolingModePerformance(), maxLoad, optimalLoad);
        }
    } else if (calledFromLocation.loopNum == this->heatingPlantLoc.loopNum) {
        for (auto const &module : this->modules) {
            auto heatingMode = module.heatingModePerformance();
            heatingMode.referenceEvaporatorCapacity +=
                max(0.0, module.sizing.referenceHeatingPower) * module.performanceData().compressorMotorEfficiency;
            accumulateCapacity(heatingMode, maxLoad, optimalLoad);
        }
    } else if (calledFromLocation.loopNum == this->sourcePlantLoc.loopNum) {
        Real64 maximumSourceRejection = 0.0;
        Real64 optimumSourceRejection = 0.0;
        Real64 maximumSourceExtraction = 0.0;
        Real64 optimumSourceExtraction = 0.0;
        for (auto const &module : this->modules) {
            auto coolingMode = module.coolingModePerformance();
            Real64 const referenceCoolingPower =
                coolingMode.referenceCOP > 0.0 ? coolingMode.referenceEvaporatorCapacity / coolingMode.referenceCOP : 0.0;
            coolingMode.referenceEvaporatorCapacity += referenceCoolingPower * module.performanceData().compressorMotorEfficiency;
            accumulateCapacity(coolingMode, maximumSourceRejection, optimumSourceRejection);
            accumulateCapacity(module.heatingModePerformance(), maximumSourceExtraction, optimumSourceExtraction);
        }
        maxLoad = max(maximumSourceRejection, maximumSourceExtraction);
        optimalLoad = max(optimumSourceRejection, optimumSourceExtraction);
    }

    if (minimumStageLoad < std::numeric_limits<Real64>::max()) {
        minLoad = minimumStageLoad;
    }
}

void CentralHeatPumpSystem::getSizingFactor(Real64 &sizingFactor)
{
    sizingFactor = 1.0;
}

void CentralHeatPumpSystem::simulate(
    EnergyPlusData &state, const PlantLocation &calledFromLocation, bool firstHVACIteration, Real64 &currentLoad, bool const runFlag)
{
    if (calledFromLocation.loopNum != this->sourcePlantLoc.loopNum) {

        this->initialize(state, currentLoad, calledFromLocation.loopNum, runFlag);
        if (!runFlag) {
            currentLoad = 0.0;
            if (calledFromLocation.loopNum == this->coolingPlantLoc.loopNum) {
                this->requestedCoolingLoad = 0.0;
            } else if (calledFromLocation.loopNum == this->heatingPlantLoc.loopNum) {
                this->requestedHeatingLoad = 0.0;
            }
            if (this->requestedCoolingLoad <= HVAC::SmallLoad && this->requestedHeatingLoad <= HVAC::SmallLoad) {
                this->resetOffState(state);
            }
            return;
        }
        this->calculate(state, currentLoad, calledFromLocation.loopNum);

    } else if (calledFromLocation.loopNum == this->sourcePlantLoc.loopNum) {
        // Useful-load callbacks own the system run state. The source callback only
        // publishes the authoritative load-side result and must not erase an active
        // cooling or heating request.
        PlantUtilities::UpdateChillerComponentCondenserSide(state,
                                                            calledFromLocation.loopNum,
                                                            this->sourcePlantLoc.loopSideNum,
                                                            DataPlant::PlantEquipmentType::CentralHeatPumpSystem,
                                                            this->sourceInletNodeNum,
                                                            this->sourceOutletNodeNum,
                                                            this->report.sourceHeatTransferRate,
                                                            this->report.sourceInletTemp,
                                                            this->report.sourceOutletTemp,
                                                            this->report.sourceMassFlowRate,
                                                            firstHVACIteration);
    }
}

void CentralHeatPumpSystem::size(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Yunzhi Huang, PNNL
    //       DATE WRITTEN   Feb 2013
    //       MODIFIED       November 2013 Daeho Kang, add component sizing table
    //       entries RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //  This subroutine is for sizing all the components under each
    //  'CentralHeatPumpSystem' object, for which capacities and flow rates have
    //  not been specified in the input.

    // METHODOLOGY EMPLOYED:
    //  Obtains evaporator flow rate from the plant sizing array. Calculates
    //  reference capacity from the evaporator (or load side) flow rate and the
    //  chilled water loop design delta T. The condenser flow (or source side)
    //  rate is calculated from the reference capacity, the COP, and the condenser
    //  loop design delta T.

    static constexpr std::string_view routineName("CentralHeatPumpSystem::size");

    // auto-size the chiller heater components

    for (auto &module : this->modules) {
        bool errorsFound = false;

        // find the appropriate Plant Sizing objects
        int coolingPlantSizingIndex = this->coolingPlantLoc.loop->PlantSizNum;
        int sourcePlantSizingIndex = this->sourcePlantLoc.loop->PlantSizNum;

        auto const &performance = module.performanceData();
        auto &sizing = module.sizing;

        Real64 sizingNominalCapacity = sizing.referenceCoolingCapacity;
        Real64 sizingEvaporatorVolFlowRate = sizing.designEvaporatorVolFlowRate;
        Real64 sizingCondenserVolFlowRate = sizing.designCondenserVolFlowRate;

        // auto-size the Evaporator Flow Rate
        if (coolingPlantSizingIndex > 0) {
            Real64 designEvaporatorVolFlowRate = 0.0;
            if (state.dataSize->PlantSizData(coolingPlantSizingIndex).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
                designEvaporatorVolFlowRate = state.dataSize->PlantSizData(coolingPlantSizingIndex).DesVolFlowRate * performance.sizingFactor;
            }
            sizingEvaporatorVolFlowRate =
                performance.designEvaporatorVolFlowRateWasAutoSized ? designEvaporatorVolFlowRate : sizing.designEvaporatorVolFlowRate;
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (performance.designEvaporatorVolFlowRateWasAutoSized) {
                    sizing.designEvaporatorVolFlowRate = designEvaporatorVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport && !this->mySizesReported) {
                        BaseSizer::reportSizerOutput(state,
                                                     "ChillerHeaterPerformance:Electric:EIR",
                                                     module.name(),
                                                     "Design Size Reference Chilled Water Flow Rate [m3/s]",
                                                     designEvaporatorVolFlowRate);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "ChillerHeaterPerformance:Electric:EIR",
                                                     module.name(),
                                                     "Initial Design Size Reference Chilled Water Flow Rate [m3/s]",
                                                     designEvaporatorVolFlowRate);
                    }
                } else if (sizing.designEvaporatorVolFlowRate > 0.0 && designEvaporatorVolFlowRate > 0.0 &&
                           state.dataPlnt->PlantFinalSizesOkayToReport && !this->mySizesReported) {
                    Real64 const userEvaporatorVolFlowRate = sizing.designEvaporatorVolFlowRate;
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeaterPerformance:Electric:EIR",
                                                 module.name(),
                                                 "Design Size Reference Chilled Water Flow Rate [m3/s]",
                                                 designEvaporatorVolFlowRate,
                                                 "User-Specified Reference Chilled Water Flow Rate [m3/s]",
                                                 userEvaporatorVolFlowRate);
                    if (state.dataGlobal->DisplayExtraWarnings && (std::abs(designEvaporatorVolFlowRate - userEvaporatorVolFlowRate) /
                                                                   userEvaporatorVolFlowRate) > state.dataSize->AutoVsHardSizingThreshold) {
                        ShowMessage(state,
                                    std::format("CentralHeatPumpSystem::size: "
                                                "Potential issue with equipment sizing for {}",
                                                module.name()));
                        ShowContinueError(state,
                                          std::format("User-Specified Reference Chilled "
                                                      "Water Flow Rate of {:.5f} [m3/s]",
                                                      userEvaporatorVolFlowRate));
                        ShowContinueError(state,
                                          std::format("differs from Design Size Reference Chilled "
                                                      "Water Flow Rate of {:.5f} [m3/s]",
                                                      designEvaporatorVolFlowRate));
                        ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                        ShowContinueError(state,
                                          "Verify that the value entered is intended and "
                                          "is consistent with other components.");
                    }
                }
            }
        } else {
            if (performance.designEvaporatorVolFlowRateWasAutoSized) {
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    ShowSevereError(state,
                                    "Autosizing of ChillerHeaterPerformance:Electric:EIR evaporator flow "
                                    "rate requires a loop Sizing:Plant object");
                    ShowContinueError(state, std::format("Occurs in ChillerHeaterPerformance:Electric:EIR object={}", module.name()));
                    errorsFound = true;
                }
            } else {
                if (sizing.designEvaporatorVolFlowRate > 0.0 && state.dataPlnt->PlantFinalSizesOkayToReport && !this->mySizesReported) {
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeaterPerformance:Electric:EIR",
                                                 module.name(),
                                                 "User-Specified Reference Chilled Water Flow Rate [m3/s]",
                                                 sizing.designEvaporatorVolFlowRate);
                }
            }
        }
        sizing.temporaryEvaporatorVolFlowRate = max(0.0, sizingEvaporatorVolFlowRate);

        // auto-size the Reference Cooling Capacity
        // each individual chiller heater module is sized to be capable of
        // supporting the total load on the system
        if (coolingPlantSizingIndex > 0) {
            Real64 designNominalCapacity = 0.0;
            if (state.dataSize->PlantSizData(coolingPlantSizingIndex).DesVolFlowRate >= HVAC::SmallWaterVolFlow &&
                sizingEvaporatorVolFlowRate > 0.0) {
                Real64 const cp = this->coolingPlantLoc.loop->glycol->getSpecificHeat(state, Constant::CWInitConvTemp, routineName);
                Real64 const rho = this->coolingPlantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, routineName);
                designNominalCapacity = cp * rho * state.dataSize->PlantSizData(coolingPlantSizingIndex).DeltaT * sizingEvaporatorVolFlowRate;
            }
            sizingNominalCapacity = performance.referenceCoolingCapacityWasAutoSized ? designNominalCapacity : sizing.referenceCoolingCapacity;
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (performance.referenceCoolingCapacityWasAutoSized) {
                    sizing.referenceCoolingCapacity = designNominalCapacity;

                    // Now that we have the Reference Cooling Capacity, we need to also
                    // initialize the Heating side given the ratios
                    sizing.referenceHeatingCapacity = sizing.referenceCoolingCapacity * performance.heatingToCoolingCapacityRatio;

                    sizing.referenceHeatingPower =
                        (sizing.referenceCoolingCapacity / performance.referenceCoolingCOP) * performance.heatingToCoolingPowerRatio;

                    sizing.referenceHeatingCOP = sizing.referenceHeatingCapacity / sizing.referenceHeatingPower;

                    if (state.dataPlnt->PlantFinalSizesOkayToReport && !this->mySizesReported) {
                        BaseSizer::reportSizerOutput(state,
                                                     "ChillerHeaterPerformance:Electric:EIR",
                                                     module.name(),
                                                     "Design Size Reference Capacity [W]",
                                                     designNominalCapacity);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "ChillerHeaterPerformance:Electric:EIR",
                                                     module.name(),
                                                     "Initial Design Size Reference Capacity [W]",
                                                     designNominalCapacity);
                    }
                } else if (sizing.referenceCoolingCapacity > 0.0 && designNominalCapacity > 0.0 && state.dataPlnt->PlantFinalSizesOkayToReport &&
                           !this->mySizesReported) {
                    Real64 const userNominalCapacity = sizing.referenceCoolingCapacity;
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeaterPerformance:Electric:EIR",
                                                 module.name(),
                                                 "Design Size Reference Capacity [W]",
                                                 designNominalCapacity,
                                                 "User-Specified Reference Capacity [W]",
                                                 userNominalCapacity);
                    if (state.dataGlobal->DisplayExtraWarnings &&
                        (std::abs(designNominalCapacity - userNominalCapacity) / userNominalCapacity) > state.dataSize->AutoVsHardSizingThreshold) {
                        ShowMessage(state,
                                    std::format("CentralHeatPumpSystem::size: "
                                                "Potential issue with equipment sizing for {}",
                                                module.name()));
                        ShowContinueError(state, std::format("User-Specified Reference Capacity of {:.2f} [W]", userNominalCapacity));
                        ShowContinueError(state, std::format("differs from Design Size Reference Capacity of {:.2f} [W]", designNominalCapacity));
                        ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                        ShowContinueError(state,
                                          "Verify that the value entered is intended and "
                                          "is consistent with other components.");
                    }
                }
            }
        } else {
            if (performance.referenceCoolingCapacityWasAutoSized) {
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    ShowSevereError(state,
                                    std::format("Size ChillerHeaterPerformance:Electric:EIR=\"{}\", "
                                                "autosize error.",
                                                module.name()));
                    ShowContinueError(state, "Autosizing of ChillerHeaterPerformance:Electric:EIR reference capacity requires");
                    ShowContinueError(state, "a cooling loop Sizing:Plant object.");
                    errorsFound = true;
                }
            } else {
                if (sizing.referenceCoolingCapacity > 0.0 && state.dataPlnt->PlantFinalSizesOkayToReport && !this->mySizesReported) {
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeaterPerformance:Electric:EIR",
                                                 module.name(),
                                                 "User-Specified Reference Capacity [W]",
                                                 sizing.referenceCoolingCapacity);
                }
            }
        }

        // auto-size the condenser volume flow rate
        // each individual chiller heater module is sized to be capable of
        // supporting the total load on the system
        if (sourcePlantSizingIndex > 0) {
            auto const &sourceSizing = state.dataSize->PlantSizData(sourcePlantSizingIndex);
            Real64 designCondenserVolFlowRate = 0.0;
            if (sourceSizing.DesVolFlowRate >= HVAC::SmallWaterVolFlow && sizingNominalCapacity > 0.0 && sourceSizing.DeltaT > 0.0 &&
                performance.referenceCoolingCOP > 0.0) {
                Real64 const rho = this->sourcePlantLoc.loop->glycol->getDensity(state, performance.coolingReferenceCondenserInletTemp, routineName);
                Real64 const cp =
                    this->sourcePlantLoc.loop->glycol->getSpecificHeat(state, performance.coolingReferenceCondenserInletTemp, routineName);
                designCondenserVolFlowRate = sizingNominalCapacity * (1.0 + performance.compressorMotorEfficiency / performance.referenceCoolingCOP) /
                                             (sourceSizing.DeltaT * cp * rho);
            }
            sizingCondenserVolFlowRate =
                performance.designCondenserVolFlowRateWasAutoSized ? designCondenserVolFlowRate : sizing.designCondenserVolFlowRate;
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (performance.designCondenserVolFlowRateWasAutoSized) {
                    sizing.designCondenserVolFlowRate = designCondenserVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport && !this->mySizesReported) {
                        BaseSizer::reportSizerOutput(state,
                                                     "ChillerHeaterPerformance:Electric:EIR",
                                                     module.name(),
                                                     "Design Size Reference Condenser Water Flow Rate [m3/s]",
                                                     designCondenserVolFlowRate);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "ChillerHeaterPerformance:Electric:EIR",
                                                     module.name(),
                                                     "Initial Design Size Reference Condenser Water Flow Rate "
                                                     "[m3/s]",
                                                     designCondenserVolFlowRate);
                    }
                } else if (sizing.designCondenserVolFlowRate > 0.0 && designCondenserVolFlowRate > 0.0 &&
                           state.dataPlnt->PlantFinalSizesOkayToReport && !this->mySizesReported) {
                    Real64 const userCondenserVolFlowRate = sizing.designCondenserVolFlowRate;
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeaterPerformance:Electric:EIR",
                                                 module.name(),
                                                 "Design Size Reference Condenser Water Flow Rate [m3/s]",
                                                 designCondenserVolFlowRate,
                                                 "User-Specified Reference Condenser Water Flow Rate [m3/s]",
                                                 userCondenserVolFlowRate);
                    if (state.dataGlobal->DisplayExtraWarnings && (std::abs(designCondenserVolFlowRate - userCondenserVolFlowRate) /
                                                                   userCondenserVolFlowRate) > state.dataSize->AutoVsHardSizingThreshold) {
                        ShowMessage(state,
                                    std::format("CentralHeatPumpSystem::size: "
                                                "Potential issue with equipment sizing for {}",
                                                module.name()));
                        ShowContinueError(state,
                                          std::format("User-Specified Reference Condenser "
                                                      "Water Flow Rate of {:.5f} [m3/s]",
                                                      userCondenserVolFlowRate));
                        ShowContinueError(state,
                                          std::format("differs from Design Size Reference "
                                                      "Condenser Water Flow Rate of {:.5f} [m3/s]",
                                                      designCondenserVolFlowRate));
                        ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                        ShowContinueError(state,
                                          "Verify that the value entered is intended and "
                                          "is consistent with other components.");
                    }
                }
            }
        } else {
            if (performance.designCondenserVolFlowRateWasAutoSized) {
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    ShowSevereError(state,
                                    std::format("Size ChillerHeaterPerformance:Electric:EIR=\"{}\", "
                                                "autosize error.",
                                                module.name()));
                    ShowContinueError(state,
                                      "Autosizing of ChillerHeaterPerformance:Electric:EIR "
                                      "condenser flow rate requires");
                    ShowContinueError(state, "a condenser loop Sizing:Plant object.");
                    errorsFound = true;
                }
            } else {
                if (sizing.designCondenserVolFlowRate > 0.0 && state.dataPlnt->PlantFinalSizesOkayToReport && !this->mySizesReported) {
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeaterPerformance:Electric:EIR",
                                                 module.name(),
                                                 "User-Specified Reference Condenser Water Flow Rate [m3/s]",
                                                 sizing.designCondenserVolFlowRate);
                }
            }
        }
        sizing.temporaryCondenserVolFlowRate = max(0.0, sizingCondenserVolFlowRate);

        if (state.dataPlnt->PlantFinalSizesOkayToReport && !this->mySizesReported) {
            // create predefined report
            const std::string &equipmentName = module.name();
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchMechType, equipmentName, "ChillerHeaterPerformance:Electric:EIR");
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchMechNomEff, equipmentName, performance.referenceCoolingCOP);
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchMechNomCap, equipmentName, sizing.referenceCoolingCapacity);
        }

        if (errorsFound) {
            ShowFatalError(state, "Preceding sizing errors cause program termination");
        }
    }

    // sum individual volume flows and register system inlets
    Real64 totalEvaporatorVolFlowRate = 0.0;
    Real64 totalSourceVolFlowRate = 0.0;
    Real64 totalHeatingVolFlowRate = 0.0;
    for (auto const &module : this->modules) {
        auto const &performance = module.performanceData();
        auto const &sizing = module.sizing;
        totalEvaporatorVolFlowRate += sizing.temporaryEvaporatorVolFlowRate;
        totalSourceVolFlowRate += max(sizing.temporaryEvaporatorVolFlowRate, sizing.temporaryCondenserVolFlowRate);
        totalHeatingVolFlowRate += performance.designHeatingVolFlowRate;
    }

    PlantUtilities::RegisterPlantCompDesignFlow(state, this->coolingInletNodeNum, totalEvaporatorVolFlowRate);
    PlantUtilities::RegisterPlantCompDesignFlow(state, this->heatingInletNodeNum, totalHeatingVolFlowRate);
    // Source extraction uses evaporator flow and source rejection uses condenser
    // flow; only one direction is active per module.
    PlantUtilities::RegisterPlantCompDesignFlow(state, this->sourceInletNodeNum, totalSourceVolFlowRate);

    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
        this->mySizesReported = true;
    }
}

void CentralHeatPumpSystem::resolveFlowMode(EnergyPlusData &state)
{
    bool hasConstantFlow = false;
    bool hasVariableFlow = false;
    for (auto const &module : this->modules) {
        auto const &performance = module.performanceData();
        hasConstantFlow = hasConstantFlow || performance.constantFlow || !performance.variableFlow;
        hasVariableFlow = hasVariableFlow || performance.variableFlow;
    }

    bool const mixedFlowModes = hasConstantFlow && hasVariableFlow;
    this->allModulesVariableFlow = hasVariableFlow && !hasConstantFlow;
    if (mixedFlowModes) {
        ShowWarningError(state,
                         std::format("CentralHeatPumpSystem={} contains both "
                                     "constant-flow and variable-flow performance objects.",
                                     this->Name));
        ShowContinueError(state, "All modules in this CentralHeatPumpSystem will use constant-flow control.");
    }

    for (auto &module : this->modules) {
        module.variableFlow = !mixedFlowModes && this->allModulesVariableFlow;
    }
}

void getCentralHeatPumpSystemInput(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Yunzhi Huang and Daeho Kang, PNNL
    //       DATE WRITTEN:    Feb 2013

    // PURPOSE OF THIS SUBROUTINE:
    //  This routine will get the input required by the CentralHeatPumpSystem model.

    static constexpr std::string_view routineName = "getCentralHeatPumpSystemInput";
    static constexpr char objectType[] = "CentralHeatPumpSystem";
    static constexpr char performanceObjectType[] = "ChillerHeaterPerformance:Electric:EIR";

    static constexpr char coolingInletNodeKey[] = "cooling_loop_inlet_node_name";
    static constexpr char coolingOutletNodeKey[] = "cooling_loop_outlet_node_name";
    static constexpr char sourceInletNodeKey[] = "source_loop_inlet_node_name";
    static constexpr char sourceOutletNodeKey[] = "source_loop_outlet_node_name";
    static constexpr char heatingInletNodeKey[] = "heating_loop_inlet_node_name";
    static constexpr char heatingOutletNodeKey[] = "heating_loop_outlet_node_name";
    static constexpr char ancillaryPowerKey[] = "ancillary_power";
    static constexpr char ancillaryScheduleKey[] = "ancillary_operation_schedule_name";

    struct ResolvedModuleGroup
    {
        std::size_t performanceIndex = 0;
        int moduleCount = 0;
        Sched::Schedule *availabilitySchedule = nullptr;
    };

    struct ModuleGroupFieldKeys
    {
        std::string performanceObjectType;
        std::string performanceName;
        std::string controlScheduleName;
        std::string numberOfModules;
    };

    auto moduleGroupFieldKeys = [](int const groupNumber) {
        std::string const suffix = std::to_string(groupNumber);
        ModuleGroupFieldKeys keys{
            "chiller_heater_modules_performance_component_object_type_" + suffix,
            "chiller_heater_modules_performance_component_name_" + suffix,
            "chiller_heater_modules_control_schedule_name_" + suffix,
            "number_of_chiller_heater_modules_" + suffix,
        };

        // These schema keys reflect the original IDD field spellings.
        if (groupNumber == 3) {
            keys.performanceObjectType = "chiller_heater_performance_component_object_type_3";
            keys.performanceName = "chiller_heater_performance_component_name_3";
        } else if (groupNumber == 5) {
            keys.performanceName = "chiller_heater_models_performance_component_name_5";
        } else if (groupNumber == 11) {
            keys.controlScheduleName = "chiller_heater_module_control_schedule_name_11";
        } else if (groupNumber == 18) {
            keys.controlScheduleName = "chiller_heater_modules_control_control_schedule_name_18";
        }
        return keys;
    };

    bool errorsFound = false;
    auto &inputProcessor = state.dataInputProcessing->inputProcessor;
    auto &systems = state.dataPlantCentralHeatPumpSystem->systems;
    state.dataIPShortCut->cCurrentModuleObject = objectType;
    state.dataPlantCentralHeatPumpSystem->numPerformanceReferences = 0;

    auto const instances = inputProcessor->epJSON.find(objectType);
    if (instances == inputProcessor->epJSON.end() || instances->empty()) {
        ShowSevereError(state, std::format("No {} equipment specified in input file", objectType));
        return;
    }

    auto const &instancesValue = instances.value();
    systems.clear();
    systems.resize(instancesValue.size());

    // Performance definitions are independent named objects. Parse and retain them before resolving system references.
    getPerformanceInput(state);

    auto const &objectSchemaProps = inputProcessor->getObjectSchemaProps(state, objectType);
    assert(instancesValue.size() == systems.size());

    std::unordered_set<std::string> systemNames;
    std::size_t systemIndex = 0;
    for (auto const &systemObject : instancesValue.items()) {
        auto const &key = systemObject.key();
        auto const &objectFields = systemObject.value();
        inputProcessor->markObjectAsUsed(objectType, key);

        ErrorObjectHeader const eoh{routineName, objectType, key};
        auto &system = systems.at(systemIndex++);
        system.Name = Util::makeUPPER(key);
        if (!systemNames.emplace(system.Name).second) {
            ShowSevereDuplicateName(state, eoh);
            errorsFound = true;
            continue;
        }

        std::string const coolingInletNodeName = inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, coolingInletNodeKey);
        std::string const coolingOutletNodeName = inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, coolingOutletNodeKey);
        std::string const sourceInletNodeName = inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, sourceInletNodeKey);
        std::string const sourceOutletNodeName = inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, sourceOutletNodeKey);
        std::string const heatingInletNodeName = inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, heatingInletNodeKey);
        std::string const heatingOutletNodeName = inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, heatingOutletNodeKey);

        system.coolingInletNodeNum = Node::GetOnlySingleNode(state,
                                                             coolingInletNodeName,
                                                             errorsFound,
                                                             Node::ConnectionObjectType::CentralHeatPumpSystem,
                                                             system.Name,
                                                             Node::FluidType::Water,
                                                             Node::ConnectionType::Inlet,
                                                             Node::CompFluidStream::Primary,
                                                             Node::ObjectIsNotParent);
        system.coolingOutletNodeNum = Node::GetOnlySingleNode(state,
                                                              coolingOutletNodeName,
                                                              errorsFound,
                                                              Node::ConnectionObjectType::CentralHeatPumpSystem,
                                                              system.Name,
                                                              Node::FluidType::Water,
                                                              Node::ConnectionType::Outlet,
                                                              Node::CompFluidStream::Primary,
                                                              Node::ObjectIsNotParent);
        system.coolingSetpointNodeNum = system.coolingOutletNodeNum;
        Node::TestCompSet(state, objectType, system.Name, coolingInletNodeName, coolingOutletNodeName, "Chilled Water Nodes");

        system.sourceInletNodeNum = Node::GetOnlySingleNode(state,
                                                            sourceInletNodeName,
                                                            errorsFound,
                                                            Node::ConnectionObjectType::CentralHeatPumpSystem,
                                                            system.Name,
                                                            Node::FluidType::Water,
                                                            Node::ConnectionType::Inlet,
                                                            Node::CompFluidStream::Secondary,
                                                            Node::ObjectIsNotParent);
        system.sourceOutletNodeNum = Node::GetOnlySingleNode(state,
                                                             sourceOutletNodeName,
                                                             errorsFound,
                                                             Node::ConnectionObjectType::CentralHeatPumpSystem,
                                                             system.Name,
                                                             Node::FluidType::Water,
                                                             Node::ConnectionType::Outlet,
                                                             Node::CompFluidStream::Secondary,
                                                             Node::ObjectIsNotParent);
        Node::TestCompSet(state, objectType, system.Name, sourceInletNodeName, sourceOutletNodeName, "Source Nodes");

        system.heatingInletNodeNum = Node::GetOnlySingleNode(state,
                                                             heatingInletNodeName,
                                                             errorsFound,
                                                             Node::ConnectionObjectType::CentralHeatPumpSystem,
                                                             system.Name,
                                                             Node::FluidType::Water,
                                                             Node::ConnectionType::Inlet,
                                                             Node::CompFluidStream::Tertiary,
                                                             Node::ObjectIsNotParent);
        system.heatingOutletNodeNum = Node::GetOnlySingleNode(state,
                                                              heatingOutletNodeName,
                                                              errorsFound,
                                                              Node::ConnectionObjectType::CentralHeatPumpSystem,
                                                              system.Name,
                                                              Node::FluidType::Water,
                                                              Node::ConnectionType::Outlet,
                                                              Node::CompFluidStream::Tertiary,
                                                              Node::ObjectIsNotParent);
        system.heatingSetpointNodeNum = system.heatingOutletNodeNum;
        Node::TestCompSet(state, objectType, system.Name, heatingInletNodeName, heatingOutletNodeName, "Hot water Nodes");

        system.ancillaryPower = inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, ancillaryPowerKey);
        std::string const ancillaryScheduleName = inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, ancillaryScheduleKey);
        if (ancillaryScheduleName.empty()) {
            system.ancillaryPowerSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((system.ancillaryPowerSched = Sched::GetSchedule(state, ancillaryScheduleName)) == nullptr) {
            ShowSevereItemNotFound(state, eoh, ancillaryScheduleKey, ancillaryScheduleName);
            errorsFound = true;
        }

        std::vector<ResolvedModuleGroup> resolvedGroups;
        resolvedGroups.reserve(20);
        std::size_t totalModuleCount = 0;
        for (int groupNumber = 1; groupNumber <= 20; ++groupNumber) {
            auto const keys = moduleGroupFieldKeys(groupNumber);
            if (!objectFields.contains(keys.performanceObjectType)) {
                continue;
            }

            std::string const enteredPerformanceObjectType =
                inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, keys.performanceObjectType);
            std::string const performanceName = inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, keys.performanceName);
            if (!Util::SameString(enteredPerformanceObjectType, performanceObjectType)) {
                ShowSevereError(state,
                                std::format("{}: {}={} is not a supported performance object type for {}={}",
                                            routineName,
                                            keys.performanceObjectType,
                                            enteredPerformanceObjectType,
                                            objectType,
                                            system.Name));
                errorsFound = true;
                continue;
            }

            auto const &performanceDefinitions = state.dataPlantCentralHeatPumpSystem->performanceDefinitions;
            auto const performance = std::ranges::find_if(
                performanceDefinitions, [&performanceName](auto const &candidate) { return Util::SameString(performanceName, candidate.Name); });
            if (performance == performanceDefinitions.end()) {
                ShowSevereItemNotFound(state, eoh, keys.performanceName, performanceName);
                ShowContinueError(state, "Select the name of a ChillerHeaterPerformance:Electric:EIR object.");
                errorsFound = true;
                continue;
            }
            auto const performanceIndex = static_cast<std::size_t>(std::distance(performanceDefinitions.begin(), performance));

            int const moduleCount = inputProcessor->getIntFieldValue(objectFields, objectSchemaProps, keys.numberOfModules);
            if (moduleCount < 1) {
                ShowSevereError(state,
                                std::format("{}: {} must be at least 1 for {}={}", routineName, keys.numberOfModules, objectType, system.Name));
                errorsFound = true;
                continue;
            }

            std::string const scheduleName = inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, keys.controlScheduleName);
            Sched::Schedule *availabilitySchedule = Sched::GetScheduleAlwaysOn(state);
            if (!scheduleName.empty() && (availabilitySchedule = Sched::GetSchedule(state, scheduleName)) == nullptr) {
                availabilitySchedule = Sched::GetScheduleAlwaysOn(state);
                ShowWarningItemNotFound(state, eoh, keys.controlScheduleName, scheduleName, "the AlwaysOn schedule");
            }

            resolvedGroups.push_back({performanceIndex, moduleCount, availabilitySchedule});
            totalModuleCount += static_cast<std::size_t>(moduleCount);
            ++state.dataPlantCentralHeatPumpSystem->numPerformanceReferences;
        }

        if (resolvedGroups.empty()) {
            ShowSevereError(state, std::format("{}: No valid module groups specified for {}={}", routineName, objectType, system.Name));
            errorsFound = true;
            continue;
        }

        system.modules.resize(totalModuleCount);
        std::size_t moduleIndex = 0;
        for (auto const &group : resolvedGroups) {
            for (int identicalModuleNum = 1; identicalModuleNum <= group.moduleCount; ++identicalModuleNum) {
                system.modules.at(moduleIndex++)
                    .initialize(state.dataPlantCentralHeatPumpSystem->performanceDefinitions.at(group.performanceIndex), group.availabilitySchedule);
            }
        }
        assert(moduleIndex == system.modules.size());
        system.resolveFlowMode(state);
    }

    if (errorsFound) {
        ShowFatalError(state, std::format("getCentralHeatPumpSystemInput: Invalid {} Input, preceding condition(s) cause termination.", objectType));
    }
}

void CentralHeatPumpSystem::setupOutputVars(EnergyPlusData &state)
{
    SetupOutputVariable(state,
                        "Chiller Heater System Cooling Electricity Energy",
                        Constant::Units::J,
                        this->report.coolingElectricEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Cooling);

    SetupOutputVariable(state,
                        "Chiller Heater System Heating Electricity Energy",
                        Constant::Units::J,
                        this->report.heatingElectricEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Heating);

    SetupOutputVariable(state,
                        "Chiller Heater System Cooling Electricity Rate",
                        Constant::Units::W,
                        this->report.coolingElectricPower,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater System Heating Electricity Rate",
                        Constant::Units::W,
                        this->report.heatingElectricPower,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater System Cooling Energy",
                        Constant::Units::J,
                        this->report.coolingHeatTransferEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Chillers);

    SetupOutputVariable(state,
                        "Chiller Heater System Heating Energy",
                        Constant::Units::J,
                        this->report.heatingHeatTransferEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Boilers);

    SetupOutputVariable(state,
                        "Chiller Heater System Source Heat Transfer Energy",
                        Constant::Units::J,
                        this->report.sourceHeatTransferEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::HeatRejection);

    SetupOutputVariable(state,
                        "Chiller Heater System Cooling Rate",
                        Constant::Units::W,
                        this->report.coolingHeatTransferRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater System Heating Rate",
                        Constant::Units::W,
                        this->report.heatingHeatTransferRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater System Source Heat Transfer Rate",
                        Constant::Units::W,
                        this->report.sourceHeatTransferRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater System Cooling Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->report.coolingMassFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater System Heating Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->report.heatingMassFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater System Source Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->report.sourceMassFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater System Cooling Inlet Temperature",
                        Constant::Units::C,
                        this->report.coolingInletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater System Heating Inlet Temperature",
                        Constant::Units::C,
                        this->report.heatingInletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater System Source Inlet Temperature",
                        Constant::Units::C,
                        this->report.sourceInletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater System Cooling Outlet Temperature",
                        Constant::Units::C,
                        this->report.coolingOutletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater System Heating Outlet Temperature",
                        Constant::Units::C,
                        this->report.heatingOutletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater System Source Outlet Temperature",
                        Constant::Units::C,
                        this->report.sourceOutletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    if (!this->modules.empty()) {
        for (std::size_t moduleIndex = 0; moduleIndex < this->modules.size(); ++moduleIndex) {
            int const moduleNum = static_cast<int>(moduleIndex) + 1;
            auto &module = this->modules[moduleIndex];
            SetupOutputVariable(state,
                                std::format("Chiller Heater Operation Mode Unit {}", moduleNum),
                                Constant::Units::None,
                                module.result.currentMode,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Part Load Ratio Unit {}", moduleNum),
                                Constant::Units::None,
                                module.result.partLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Cycling Ratio Unit {}", moduleNum),
                                Constant::Units::None,
                                module.result.cyclingRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Cooling Electricity Rate Unit {}", moduleNum),
                                Constant::Units::W,
                                module.result.coolingPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Heating Electricity Rate Unit {}", moduleNum),
                                Constant::Units::W,
                                module.result.heatingPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Cooling Electricity Energy Unit {}", moduleNum),
                                Constant::Units::J,
                                module.result.coolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Heating Electricity Energy Unit {}", moduleNum),
                                Constant::Units::J,
                                module.result.heatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Cooling Rate Unit {}", moduleNum),
                                Constant::Units::W,
                                module.result.qEvaporator,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Cooling Energy Unit {}", moduleNum),
                                Constant::Units::J,
                                module.result.evaporatorEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater False Load Heat Transfer Rate Unit {}", moduleNum),
                                Constant::Units::W,
                                module.result.falseLoadRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater False Load Heat Transfer Energy Unit {}", moduleNum),
                                Constant::Units::J,
                                module.result.falseLoadEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Evaporator Inlet Temperature Unit {}", moduleNum),
                                Constant::Units::C,
                                module.result.evaporatorInletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Evaporator Outlet Temperature Unit {}", moduleNum),
                                Constant::Units::C,
                                module.result.evaporatorOutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Evaporator Mass Flow Rate Unit {}", moduleNum),
                                Constant::Units::kg_s,
                                module.result.evaporatorMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Condenser Heat Transfer Rate Unit {}", moduleNum),
                                Constant::Units::W,
                                module.result.qCondenser,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Condenser Heat Transfer Energy Unit {}", moduleNum),
                                Constant::Units::J,
                                module.result.condenserEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater COP Unit {}", moduleNum),
                                Constant::Units::W_W,
                                module.result.actualCOP,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Capacity Temperature Modifier Multiplier Unit {}", moduleNum),
                                Constant::Units::None,
                                module.result.capacityTemperatureModifier,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater EIR Temperature Modifier Multiplier Unit {}", moduleNum),
                                Constant::Units::None,
                                module.result.eirTemperatureModifier,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater EIR Part Load Modifier Multiplier Unit {}", moduleNum),
                                Constant::Units::None,
                                module.result.eirPartLoadModifier,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Condenser Inlet Temperature Unit {}", moduleNum),
                                Constant::Units::C,
                                module.result.condenserInletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Condenser Outlet Temperature Unit {}", moduleNum),
                                Constant::Units::C,
                                module.result.condenserOutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());

            SetupOutputVariable(state,
                                std::format("Chiller Heater Condenser Mass Flow Rate Unit {}", moduleNum),
                                Constant::Units::kg_s,
                                module.result.condenserMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                module.name());
        } // End of individual chiller heater count for current system

    } // End of individual chiller heater output
}

void getPerformanceInput(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Kyung Tae Yun, Mississippi State University
    //       DATE WRITTEN:    Feb 2013

    // PURPOSE OF THIS SUBROUTINE:
    //  This routine will get the input required by the
    //  ChillerHeaterPerformance:Electric:EIR model.

    static constexpr std::string_view routineName = "getPerformanceInput";
    static constexpr char objectType[] = "ChillerHeaterPerformance:Electric:EIR";

    static constexpr char referenceCoolingCapacityKey[] = "reference_cooling_mode_evaporator_capacity";
    static constexpr char referenceCoolingCOPKey[] = "reference_cooling_mode_cop";
    static constexpr char referenceCoolingLeavingCoolingTempKey[] = "reference_cooling_mode_leaving_chilled_water_temperature";
    static constexpr char referenceCoolingEnteringCondenserTempKey[] = "reference_cooling_mode_entering_condenser_fluid_temperature";
    static constexpr char referenceCoolingLeavingCondenserTempKey[] = "reference_cooling_mode_leaving_condenser_water_temperature";
    static constexpr char referenceHeatingCapacityRatioKey[] = "reference_heating_mode_cooling_capacity_ratio";
    static constexpr char referenceHeatingPowerRatioKey[] = "reference_heating_mode_cooling_power_input_ratio";
    static constexpr char referenceHeatingLeavingCoolingTempKey[] = "reference_heating_mode_leaving_chilled_water_temperature";
    static constexpr char referenceHeatingLeavingCondenserTempKey[] = "reference_heating_mode_leaving_condenser_water_temperature";
    static constexpr char referenceHeatingEnteringCondenserTempKey[] = "reference_heating_mode_entering_condenser_fluid_temperature";
    static constexpr char heatingEnteringCoolingLowLimitKey[] = "heating_mode_entering_chilled_water_temperature_low_limit";
    static constexpr char coolingFlowModeKey[] = "chilled_water_flow_mode_type";
    static constexpr char designCoolingFlowRateKey[] = "design_chilled_water_flow_rate";
    static constexpr char designCondenserWaterFlowRateKey[] = "design_condenser_water_flow_rate";
    static constexpr char designHeatingFlowRateKey[] = "design_hot_water_flow_rate";
    static constexpr char compressorMotorEfficiencyKey[] = "compressor_motor_efficiency";
    static constexpr char coolingCondenserVariableKey[] = "cooling_mode_temperature_curve_condenser_water_independent_variable";
    static constexpr char coolingCapacityCurveKey[] = "cooling_mode_cooling_capacity_function_of_temperature_curve_name";
    static constexpr char coolingEIRTemperatureCurveKey[] = "cooling_mode_electric_input_to_cooling_output_ratio_function_of_"
                                                            "temperature_curve_name";
    static constexpr char coolingEIRPartLoadCurveKey[] = "cooling_mode_electric_input_to_cooling_output_ratio_function_of_part_"
                                                         "load_ratio_curve_name";
    static constexpr char coolingOptimumPartLoadRatioKey[] = "cooling_mode_cooling_capacity_optimum_part_load_ratio";
    static constexpr char heatingCondenserVariableKey[] = "heating_mode_temperature_curve_condenser_water_independent_variable";
    static constexpr char heatingCapacityCurveKey[] = "heating_mode_cooling_capacity_function_of_temperature_curve_name";
    static constexpr char heatingEIRTemperatureCurveKey[] = "heating_mode_electric_input_to_cooling_output_ratio_function_of_"
                                                            "temperature_curve_name";
    static constexpr char heatingEIRPartLoadCurveKey[] = "heating_mode_electric_input_to_cooling_output_ratio_function_of_part_"
                                                         "load_ratio_curve_name";
    static constexpr char heatingOptimumPartLoadRatioKey[] = "heating_mode_cooling_capacity_optimum_part_load_ratio";
    static constexpr char sizingFactorKey[] = "sizing_factor";

    static constexpr char referenceCoolingCapacityField[] = "Reference Cooling Mode Evaporator Capacity";
    static constexpr char referenceCoolingCOPField[] = "Reference Cooling Mode COP";
    static constexpr char referenceHeatingCapacityRatioField[] = "Reference Heating Mode Cooling Capacity Ratio";
    static constexpr char referenceHeatingPowerRatioField[] = "Reference Heating Mode Cooling Power Input Ratio";
    static constexpr char compressorMotorEfficiencyField[] = "Compressor Motor Efficiency";
    static constexpr char coolingCapacityCurveField[] = "Cooling Mode Cooling Capacity Function of Temperature Curve Name";
    static constexpr char coolingEIRTemperatureCurveField[] = "Cooling Mode Electric Input to Cooling Output Ratio Function of "
                                                              "Temperature Curve Name";
    static constexpr char coolingEIRPartLoadCurveField[] = "Cooling Mode Electric Input to Cooling Output Ratio Function of Part "
                                                           "Load Ratio Curve Name";
    static constexpr char coolingOptimumPartLoadRatioField[] = "Cooling Mode Cooling Capacity Optimum Part Load Ratio";
    static constexpr char heatingCapacityCurveField[] = "Heating Mode Cooling Capacity Function of Temperature Curve Name";
    static constexpr char heatingEIRTemperatureCurveField[] = "Heating Mode Electric Input to Cooling Output Ratio Function of "
                                                              "Temperature Curve Name";
    static constexpr char heatingEIRPartLoadCurveField[] = "Heating Mode Electric Input to Cooling Output Ratio Function of Part "
                                                           "Load Ratio Curve Name";
    static constexpr char heatingOptimumPartLoadRatioField[] = "Heating Mode Cooling Capacity Optimum Part Load Ratio";

    bool errorsFound = false; // True when input errors are found

    auto &inputProcessor = state.dataInputProcessing->inputProcessor;
    auto &performanceDefinitions = state.dataPlantCentralHeatPumpSystem->performanceDefinitions;
    state.dataIPShortCut->cCurrentModuleObject = objectType;
    auto const instances = inputProcessor->epJSON.find(objectType);

    performanceDefinitions.clear();
    if (instances == inputProcessor->epJSON.end() || instances->empty()) {
        ShowSevereError(state, std::format("No {} equipment specified in input file", objectType));
        errorsFound = true;
    } else {
        performanceDefinitions.resize(instances->size());
    }

    if (instances != inputProcessor->epJSON.end()) {
        auto const &objectSchemaProps = inputProcessor->getObjectSchemaProps(state, objectType);
        auto const &instancesValue = instances.value();
        assert(instancesValue.size() == performanceDefinitions.size());

        std::unordered_set<std::string> performanceNames;
        std::size_t performanceIndex = 0;
        for (auto const &performanceObject : instancesValue.items()) {
            auto const &key = performanceObject.key();
            auto const &objectFields = performanceObject.value();

            inputProcessor->markObjectAsUsed(objectType, key);
            ErrorObjectHeader const eoh{routineName, objectType, key};

            auto &performanceDefinition = performanceDefinitions.at(performanceIndex++);
            performanceDefinition.Name = Util::makeUPPER(key);
            if (!performanceNames.emplace(performanceDefinition.Name).second) {
                ShowSevereDuplicateName(state, eoh);
                errorsFound = true;
                continue;
            }

            std::string const coolingCondenserVariable =
                inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, coolingCondenserVariableKey);
            performanceDefinition.coolingCondenserTemperatureMode = Util::SameString(coolingCondenserVariable, "LEAVINGCONDENSER")
                                                                        ? CondenserTemperatureMode::LeavingCondenser
                                                                        : CondenserTemperatureMode::EnteringCondenser;

            std::string const coolingCapacityCurveName = inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, coolingCapacityCurveKey);
            performanceDefinition.coolingCapacityTemperatureCurveIndex = Curve::GetCurveIndex(state, coolingCapacityCurveName);
            if (performanceDefinition.coolingCapacityTemperatureCurveIndex == 0) {
                ShowSevereError(state, std::format("Invalid {}={}", objectType, performanceDefinition.Name));
                ShowContinueError(state, std::format("Entered in {}={}", coolingCapacityCurveField, coolingCapacityCurveName));
                errorsFound = true;
            }

            std::string const coolingEIRTemperatureCurveName =
                inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, coolingEIRTemperatureCurveKey);
            performanceDefinition.coolingEIRTemperatureCurveIndex = Curve::GetCurveIndex(state, coolingEIRTemperatureCurveName);
            if (performanceDefinition.coolingEIRTemperatureCurveIndex == 0) {
                ShowSevereError(state, std::format("Invalid {}={}", objectType, performanceDefinition.Name));
                ShowContinueError(state, std::format("Entered in {}={}", coolingEIRTemperatureCurveField, coolingEIRTemperatureCurveName));
                errorsFound = true;
            }

            std::string const coolingEIRPartLoadCurveName =
                inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, coolingEIRPartLoadCurveKey);
            performanceDefinition.coolingEIRPartLoadCurveIndex = Curve::GetCurveIndex(state, coolingEIRPartLoadCurveName);
            if (performanceDefinition.coolingEIRPartLoadCurveIndex == 0) {
                ShowSevereError(state, std::format("Invalid {}={}", objectType, performanceDefinition.Name));
                ShowContinueError(state, std::format("Entered in {}={}", coolingEIRPartLoadCurveField, coolingEIRPartLoadCurveName));
                errorsFound = true;
            }

            std::string const heatingCondenserVariable =
                inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, heatingCondenserVariableKey);
            performanceDefinition.heatingCondenserTemperatureMode = Util::SameString(heatingCondenserVariable, "LEAVINGCONDENSER")
                                                                        ? CondenserTemperatureMode::LeavingCondenser
                                                                        : CondenserTemperatureMode::EnteringCondenser;

            std::string const heatingCapacityCurveName = inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, heatingCapacityCurveKey);
            performanceDefinition.heatingCapacityTemperatureCurveIndex = Curve::GetCurveIndex(state, heatingCapacityCurveName);
            if (performanceDefinition.heatingCapacityTemperatureCurveIndex == 0) {
                ShowSevereError(state, std::format("Invalid {}={}", objectType, performanceDefinition.Name));
                ShowContinueError(state, std::format("Entered in {}={}", heatingCapacityCurveField, heatingCapacityCurveName));
                errorsFound = true;
            }

            std::string const heatingEIRTemperatureCurveName =
                inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, heatingEIRTemperatureCurveKey);
            performanceDefinition.heatingEIRTemperatureCurveIndex = Curve::GetCurveIndex(state, heatingEIRTemperatureCurveName);
            if (performanceDefinition.heatingEIRTemperatureCurveIndex == 0) {
                ShowSevereError(state, std::format("Invalid {}={}", objectType, performanceDefinition.Name));
                ShowContinueError(state, std::format("Entered in {}={}", heatingEIRTemperatureCurveField, heatingEIRTemperatureCurveName));
                errorsFound = true;
            }

            std::string const heatingEIRPartLoadCurveName =
                inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, heatingEIRPartLoadCurveKey);
            performanceDefinition.heatingEIRPartLoadCurveIndex = Curve::GetCurveIndex(state, heatingEIRPartLoadCurveName);
            if (performanceDefinition.heatingEIRPartLoadCurveIndex == 0) {
                ShowSevereError(state, std::format("Invalid {}={}", objectType, performanceDefinition.Name));
                ShowContinueError(state, std::format("Entered in {}={}", heatingEIRPartLoadCurveField, heatingEIRPartLoadCurveName));
                errorsFound = true;
            }

            std::string const coolingFlowMode = inputProcessor->getAlphaFieldValue(objectFields, objectSchemaProps, coolingFlowModeKey);
            performanceDefinition.constantFlow = coolingFlowMode == "CONSTANTFLOW";
            performanceDefinition.variableFlow = coolingFlowMode == "VARIABLEFLOW";

            // Chiller rated performance data
            performanceDefinition.referenceCoolingCapacity =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, referenceCoolingCapacityKey);
            performanceDefinition.referenceCoolingCapacityWasAutoSized = performanceDefinition.referenceCoolingCapacity == DataSizing::AutoSize;
            if (!performanceDefinition.referenceCoolingCapacityWasAutoSized && performanceDefinition.referenceCoolingCapacity <= 0.0) {
                ShowSevereError(state, std::format("Invalid {}={}", objectType, performanceDefinition.Name));
                ShowContinueError(state,
                                  std::format("Entered in {}={:.2f}", referenceCoolingCapacityField, performanceDefinition.referenceCoolingCapacity));
                errorsFound = true;
            }

            performanceDefinition.referenceCoolingCOP = inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, referenceCoolingCOPKey);
            if (performanceDefinition.referenceCoolingCOP <= 0.0) {
                ShowSevereError(state, std::format("Invalid {}={}", objectType, performanceDefinition.Name));
                ShowContinueError(state, std::format("Entered in {}={:.2f}", referenceCoolingCOPField, performanceDefinition.referenceCoolingCOP));
                errorsFound = true;
            }

            performanceDefinition.coolingReferenceEvaporatorOutletTemp =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, referenceCoolingLeavingCoolingTempKey);
            performanceDefinition.coolingReferenceCondenserInletTemp =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, referenceCoolingEnteringCondenserTempKey);
            performanceDefinition.coolingReferenceCondenserOutletTemp =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, referenceCoolingLeavingCondenserTempKey);

            performanceDefinition.heatingToCoolingCapacityRatio =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, referenceHeatingCapacityRatioKey);
            if (performanceDefinition.heatingToCoolingCapacityRatio <= 0.0) {
                ShowSevereError(state, std::format("Invalid {}={}", objectType, performanceDefinition.Name));
                ShowContinueError(
                    state,
                    std::format("Entered in {}={:.2f}", referenceHeatingCapacityRatioField, performanceDefinition.heatingToCoolingCapacityRatio));
                errorsFound = true;
            }

            performanceDefinition.heatingToCoolingPowerRatio =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, referenceHeatingPowerRatioKey);
            if (performanceDefinition.heatingToCoolingPowerRatio <= 0.0) {
                ShowSevereError(state, std::format("Invalid {}={}", objectType, performanceDefinition.Name));
                ShowContinueError(
                    state, std::format("Entered in {}={:.2f}", referenceHeatingPowerRatioField, performanceDefinition.heatingToCoolingPowerRatio));
                errorsFound = true;
            }

            if (!performanceDefinition.referenceCoolingCapacityWasAutoSized && performanceDefinition.referenceCoolingCapacity > 0.0 &&
                performanceDefinition.referenceCoolingCOP > 0.0 && performanceDefinition.heatingToCoolingCapacityRatio > 0.0 &&
                performanceDefinition.heatingToCoolingPowerRatio > 0.0) {
                performanceDefinition.referenceHeatingCapacity =
                    performanceDefinition.heatingToCoolingCapacityRatio * performanceDefinition.referenceCoolingCapacity;
                performanceDefinition.referenceHeatingPower =
                    (performanceDefinition.referenceCoolingCapacity / performanceDefinition.referenceCoolingCOP) *
                    performanceDefinition.heatingToCoolingPowerRatio;
                performanceDefinition.referenceHeatingCOP =
                    performanceDefinition.referenceHeatingCapacity / performanceDefinition.referenceHeatingPower;
            }

            performanceDefinition.heatingReferenceEvaporatorOutletTemp =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, referenceHeatingLeavingCoolingTempKey);
            performanceDefinition.heatingReferenceCondenserOutletTemp =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, referenceHeatingLeavingCondenserTempKey);
            performanceDefinition.heatingReferenceCondenserInletTemp =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, referenceHeatingEnteringCondenserTempKey);
            performanceDefinition.minimumEvaporatorOutletTemp =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, heatingEnteringCoolingLowLimitKey);

            performanceDefinition.designEvaporatorVolFlowRate =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, designCoolingFlowRateKey);
            performanceDefinition.designEvaporatorVolFlowRateWasAutoSized = performanceDefinition.designEvaporatorVolFlowRate == DataSizing::AutoSize;
            performanceDefinition.designCondenserVolFlowRate =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, designCondenserWaterFlowRateKey);
            performanceDefinition.designCondenserVolFlowRateWasAutoSized = performanceDefinition.designCondenserVolFlowRate == DataSizing::AutoSize;
            performanceDefinition.designHeatingVolFlowRate =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, designHeatingFlowRateKey);
            performanceDefinition.compressorMotorEfficiency =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, compressorMotorEfficiencyKey);
            performanceDefinition.coolingOptimumPartLoadRatio =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, coolingOptimumPartLoadRatioKey);
            performanceDefinition.heatingOptimumPartLoadRatio =
                inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, heatingOptimumPartLoadRatioKey);
            performanceDefinition.sizingFactor = inputProcessor->getRealFieldValue(objectFields, objectSchemaProps, sizingFactorKey);

            if (performanceDefinition.sizingFactor <= 0.0) {
                performanceDefinition.sizingFactor = 1.0;
            }

            if (performanceDefinition.compressorMotorEfficiency < 0.0 || performanceDefinition.compressorMotorEfficiency > 1.0) {
                ShowSevereError(state, std::format("getPerformanceInput: For {}: {}", objectType, performanceDefinition.Name));
                ShowContinueError(state, std::format("{} = {:.3f}", compressorMotorEfficiencyField, performanceDefinition.compressorMotorEfficiency));
                ShowContinueError(state, std::format("{} must be greater than or equal to zero", compressorMotorEfficiencyField));
                ShowContinueError(state, std::format("{} must be less than or equal to one", compressorMotorEfficiencyField));
                errorsFound = true;
            }

            Real64 const coolingReferenceCondenserTemp =
                performanceDefinition.coolingCondenserTemperatureMode == CondenserTemperatureMode::LeavingCondenser
                    ? performanceDefinition.coolingReferenceCondenserOutletTemp
                    : performanceDefinition.coolingReferenceCondenserInletTemp;
            Real64 const heatingReferenceCondenserTemp =
                performanceDefinition.heatingCondenserTemperatureMode == CondenserTemperatureMode::LeavingCondenser
                    ? performanceDefinition.heatingReferenceCondenserOutletTemp
                    : performanceDefinition.heatingReferenceCondenserInletTemp;

            // Check the CAP-FT, EIR-FT, and PLR curves and warn user if different
            // from 1.0 by more than +-10%
            if (performanceDefinition.coolingCapacityTemperatureCurveIndex > 0) {
                Real64 curveValue = Curve::CurveValue(state,
                                                      performanceDefinition.coolingCapacityTemperatureCurveIndex,
                                                      performanceDefinition.coolingReferenceEvaporatorOutletTemp,
                                                      coolingReferenceCondenserTemp);
                if (curveValue > 1.10 || curveValue < 0.90) {
                    ShowWarningError(state,
                                     "Capacity ratio as a function of temperature "
                                     "curve output is not equal to 1.0");
                    ShowContinueError(state, std::format("(+ or - 10%) at reference conditions for {}= {}", objectType, performanceDefinition.Name));
                    ShowContinueError(state, std::format("Curve output at reference conditions = {:.3f}", curveValue));
                }
            }

            if (performanceDefinition.coolingEIRTemperatureCurveIndex > 0) {
                Real64 curveValue = Curve::CurveValue(state,
                                                      performanceDefinition.coolingEIRTemperatureCurveIndex,
                                                      performanceDefinition.coolingReferenceEvaporatorOutletTemp,
                                                      coolingReferenceCondenserTemp);
                if (curveValue > 1.10 || curveValue < 0.90) {
                    ShowWarningError(state,
                                     "Energy input ratio as a function of temperature "
                                     "curve output is not equal to 1.0");
                    ShowContinueError(state, std::format("(+ or - 10%) at reference conditions for {}= {}", objectType, performanceDefinition.Name));
                    ShowContinueError(state, std::format("Curve output at reference conditions = {:.3f}", curveValue));
                }
            }

            if (performanceDefinition.coolingEIRPartLoadCurveIndex > 0) {
                Real64 curveValue =
                    evaluatePartLoadCurve(state, performanceDefinition.coolingEIRPartLoadCurveIndex, coolingReferenceCondenserTemp, 1.0);

                if (curveValue > 1.10 || curveValue < 0.90) {
                    ShowWarningError(state,
                                     "Energy input ratio as a function of part-load "
                                     "ratio curve output is not equal to 1.0");
                    ShowContinueError(state, std::format("(+ or - 10%) at reference conditions for {}= {}", objectType, performanceDefinition.Name));
                    ShowContinueError(state, std::format("Curve output at reference conditions = {:.3f}", curveValue));
                }
            }

            if (performanceDefinition.coolingEIRPartLoadCurveIndex > 0) {
                bool foundNegativeValue = false;
                std::array<Real64, 11> curveValues{}; // Used to evaluate PLFFPLR curve objects
                for (int curvePointIndex = 0; curvePointIndex <= 10; ++curvePointIndex) {
                    Real64 curveValue = evaluatePartLoadCurve(
                        state, performanceDefinition.coolingEIRPartLoadCurveIndex, coolingReferenceCondenserTemp, (curvePointIndex / 10.0));
                    if (curveValue < 0.0) {
                        foundNegativeValue = true;
                    }
                    curveValues[static_cast<std::size_t>(curvePointIndex)] = int(curveValue * 100.0 / 100.0);
                }
                if (foundNegativeValue) {
                    ShowWarningError(state,
                                     "Energy input ratio as a function of part-load "
                                     "ratio curve shows negative values ");
                    ShowContinueError(state, std::format("for {}= {}", objectType, performanceDefinition.Name));
                    ShowContinueError(state,
                                      "EIR as a function of PLR curve output at "
                                      "various part-load ratios shown below:");
                    ShowContinueError(state,
                                      "PLR   =  0.00   0.10   0.20   0.30   0.40  "
                                      " 0.50   0.60   0.70   0.80   0.90   1.00");

                    ShowContinueError(state, std::format("Curve Output = {:7.2f}", EnergyPlus::join(curveValues, ",")));

                    errorsFound = true;
                }
            }

            if (performanceDefinition.heatingCapacityTemperatureCurveIndex > 0) {
                Real64 curveValue = Curve::CurveValue(state,
                                                      performanceDefinition.heatingCapacityTemperatureCurveIndex,
                                                      performanceDefinition.heatingReferenceEvaporatorOutletTemp,
                                                      heatingReferenceCondenserTemp);
                if (curveValue > 1.10 || curveValue < 0.90) {
                    ShowWarningError(state,
                                     "Capacity ratio as a function of temperature "
                                     "curve output is not equal to 1.0");
                    ShowContinueError(state, std::format("(+ or - 10%) at reference conditions for {}= {}", objectType, performanceDefinition.Name));
                    ShowContinueError(state, std::format("Curve output at reference conditions = {:.3f}", curveValue));
                }
            }

            if (performanceDefinition.heatingEIRTemperatureCurveIndex > 0) {
                Real64 curveValue = Curve::CurveValue(state,
                                                      performanceDefinition.heatingEIRTemperatureCurveIndex,
                                                      performanceDefinition.heatingReferenceEvaporatorOutletTemp,
                                                      heatingReferenceCondenserTemp);
                if (curveValue > 1.10 || curveValue < 0.90) {
                    ShowWarningError(state,
                                     "Energy input ratio as a function of temperature "
                                     "curve output is not equal to 1.0");
                    ShowContinueError(state, std::format("(+ or - 10%) at reference conditions for {}= {}", objectType, performanceDefinition.Name));
                    ShowContinueError(state, std::format("Curve output at reference conditions = {:.3f}", curveValue));
                }
            }

            if (performanceDefinition.heatingEIRPartLoadCurveIndex > 0) {
                Real64 curveValue =
                    evaluatePartLoadCurve(state, performanceDefinition.heatingEIRPartLoadCurveIndex, heatingReferenceCondenserTemp, 1.0);

                if (curveValue > 1.10 || curveValue < 0.90) {
                    ShowWarningError(state,
                                     "Energy input ratio as a function of part-load "
                                     "ratio curve output is not equal to 1.0");
                    ShowContinueError(state, std::format("(+ or - 10%) at reference conditions for {}= {}", objectType, performanceDefinition.Name));
                    ShowContinueError(state, std::format("Curve output at reference conditions = {:.3f}", curveValue));
                }
            }

            if (performanceDefinition.heatingEIRPartLoadCurveIndex > 0) {
                bool foundNegativeValue = false;
                std::array<Real64, 11> curveValues{}; // Used to evaluate PLFFPLR curve objects
                for (int curvePointIndex = 0; curvePointIndex <= 10; ++curvePointIndex) {
                    Real64 curveValue = evaluatePartLoadCurve(
                        state, performanceDefinition.heatingEIRPartLoadCurveIndex, heatingReferenceCondenserTemp, (curvePointIndex / 10.0));
                    if (curveValue < 0.0) {
                        foundNegativeValue = true;
                    }
                    curveValues[static_cast<std::size_t>(curvePointIndex)] = int(curveValue * 100.0) / 100.0;
                }
                if (foundNegativeValue) {
                    ShowWarningError(state,
                                     "Energy input ratio as a function of part-load "
                                     "ratio curve shows negative values ");
                    ShowContinueError(state, std::format("for {}= {}", objectType, performanceDefinition.Name));
                    ShowContinueError(state,
                                      "EIR as a function of PLR curve output at "
                                      "various part-load ratios shown below:");
                    ShowContinueError(state,
                                      "PLR          =    0.00   0.10   0.20   0.30   "
                                      "0.40   0.50   0.60   0.70   0.80   0.90   1.00");

                    ShowContinueError(state, std::format("Curve Output = {:7.2f}", EnergyPlus::join(curveValues, ",")));

                    errorsFound = true;
                }
            }

            auto validatePartLoadDomain = [&](int const curveIndex,
                                              Real64 const optimumPartLoadRatio,
                                              Real64 &minimumPartLoadRatio,
                                              Real64 &maximumPartLoadRatio,
                                              std::string_view const optimumFieldName,
                                              std::string_view const curveFieldName,
                                              std::string const &curveName,
                                              std::string_view const mode) {
                if (curveIndex <= 0) {
                    return;
                }
                getPartLoadCurveMinMax(state, curveIndex, minimumPartLoadRatio, maximumPartLoadRatio);
                if (minimumPartLoadRatio < 0.0 || minimumPartLoadRatio > 1.0 || maximumPartLoadRatio < 1.0 ||
                    maximumPartLoadRatio < minimumPartLoadRatio) {
                    ShowSevereError(state, std::format("Invalid {} part-load curve domain for {}={}", mode, objectType, performanceDefinition.Name));
                    ShowContinueError(state, std::format("Entered in {}={}", curveFieldName, curveName));
                    ShowContinueError(state,
                                      std::format("Part-load ratio limits [{:.3f}, {:.3f}] must "
                                                  "include 1.0 and have a minimum no less than zero.",
                                                  minimumPartLoadRatio,
                                                  maximumPartLoadRatio));
                    errorsFound = true;
                }
                if (optimumPartLoadRatio < minimumPartLoadRatio || optimumPartLoadRatio > maximumPartLoadRatio) {
                    ShowSevereError(state, std::format("Invalid {}={}", objectType, performanceDefinition.Name));
                    ShowContinueError(state, std::format("Entered in {}={:.3f}", optimumFieldName, optimumPartLoadRatio));
                    ShowContinueError(state,
                                      std::format("{} must be within the associated part-load "
                                                  "curve limits [{:.3f}, {:.3f}].",
                                                  optimumFieldName,
                                                  minimumPartLoadRatio,
                                                  maximumPartLoadRatio));
                    errorsFound = true;
                }
            };

            validatePartLoadDomain(performanceDefinition.coolingEIRPartLoadCurveIndex,
                                   performanceDefinition.coolingOptimumPartLoadRatio,
                                   performanceDefinition.coolingMinimumPartLoadRatio,
                                   performanceDefinition.coolingMaximumPartLoadRatio,
                                   coolingOptimumPartLoadRatioField,
                                   coolingEIRPartLoadCurveField,
                                   coolingEIRPartLoadCurveName,
                                   "cooling");
            validatePartLoadDomain(performanceDefinition.heatingEIRPartLoadCurveIndex,
                                   performanceDefinition.heatingOptimumPartLoadRatio,
                                   performanceDefinition.heatingMinimumPartLoadRatio,
                                   performanceDefinition.heatingMaximumPartLoadRatio,
                                   heatingOptimumPartLoadRatioField,
                                   heatingEIRPartLoadCurveField,
                                   heatingEIRPartLoadCurveName,
                                   "heating");
        }
        assert(performanceIndex == performanceDefinitions.size());
    }

    if (errorsFound) {
        ShowFatalError(state, std::format("Errors found in processing input for {}", objectType));
    }
}

void CentralHeatPumpSystem::initializeDesignFlowLimits(EnergyPlusData &state)
{
    static constexpr std::string_view routineName("CentralHeatPumpSystem::initialize");

    this->coolingVolFlowRate = 0.0;
    this->heatingVolFlowRate = 0.0;
    this->sourceVolFlowRate = 0.0;

    for (auto const &module : this->modules) {
        auto const &performance = module.performanceData();
        auto const &sizing = module.sizing;
        this->coolingVolFlowRate += sizing.designEvaporatorVolFlowRate;
        this->heatingVolFlowRate += performance.designHeatingVolFlowRate;
        this->sourceVolFlowRate += max(sizing.designEvaporatorVolFlowRate, sizing.designCondenserVolFlowRate);
    }

    Real64 const coolingDensity = this->coolingPlantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, routineName);
    Real64 const heatingDensity = this->heatingPlantLoc.loop->glycol->getDensity(state, Constant::HWInitConvTemp, routineName);
    Real64 const sourceDensity = this->sourcePlantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, routineName);

    this->coolingMassFlowRateMax = this->coolingVolFlowRate * coolingDensity;
    this->heatingMassFlowRateMax = this->heatingVolFlowRate * heatingDensity;
    this->sourceMassFlowRateMax = this->sourceVolFlowRate * sourceDensity;

    PlantUtilities::InitComponentNodes(state, 0.0, this->coolingMassFlowRateMax, this->coolingInletNodeNum, this->coolingOutletNodeNum);
    PlantUtilities::InitComponentNodes(state, 0.0, this->heatingMassFlowRateMax, this->heatingInletNodeNum, this->heatingOutletNodeNum);
    PlantUtilities::InitComponentNodes(state, 0.0, this->sourceMassFlowRateMax, this->sourceInletNodeNum, this->sourceOutletNodeNum);

    for (auto &module : this->modules) {
        auto const &performance = module.performanceData();
        auto &sizing = module.sizing;
        sizing.maximumCoolingMassFlowRate = coolingDensity * sizing.designEvaporatorVolFlowRate;
        sizing.maximumHeatingMassFlowRate = heatingDensity * performance.designHeatingVolFlowRate;
        sizing.maximumSourceEvaporatorMassFlowRate = sourceDensity * sizing.designEvaporatorVolFlowRate;
        sizing.maximumSourceCondenserMassFlowRate = sourceDensity * sizing.designCondenserVolFlowRate;
        sizing.maximumEvaporatorMassFlowRate = max(sizing.maximumCoolingMassFlowRate, sizing.maximumSourceEvaporatorMassFlowRate);
        sizing.maximumCondenserMassFlowRate = max(sizing.maximumHeatingMassFlowRate, sizing.maximumSourceCondenserMassFlowRate);
    }
}

void CentralHeatPumpSystem::resetOffState(EnergyPlusData &state, bool const releasePlantFlows)
{
    Real64 const coolingInletTemp = state.dataLoopNodes->Node(this->coolingInletNodeNum).Temp;
    Real64 const heatingInletTemp = state.dataLoopNodes->Node(this->heatingInletNodeNum).Temp;
    Real64 const sourceInletTemp = state.dataLoopNodes->Node(this->sourceInletNodeNum).Temp;

    this->requestedCoolingLoad = 0.0;
    this->requestedHeatingLoad = 0.0;
    this->isCoolingDominant = false;
    this->isHeatingDominant = false;

    for (auto &module : this->modules) {
        module.resetResult(coolingInletTemp, sourceInletTemp);
    }

    this->report = SystemReportData();
    this->report.coolingInletTemp = coolingInletTemp;
    this->report.coolingOutletTemp = coolingInletTemp;
    this->report.heatingInletTemp = heatingInletTemp;
    this->report.heatingOutletTemp = heatingInletTemp;
    this->report.sourceInletTemp = sourceInletTemp;
    this->report.sourceOutletTemp = sourceInletTemp;

    state.dataLoopNodes->Node(this->coolingOutletNodeNum).Temp = coolingInletTemp;
    state.dataLoopNodes->Node(this->heatingOutletNodeNum).Temp = heatingInletTemp;
    state.dataLoopNodes->Node(this->sourceOutletNodeNum).Temp = sourceInletTemp;

    auto releaseFlow = [&](PlantLocation const &plantLoc, int const inletNodeNum, int const outletNodeNum) {
        state.dataLoopNodes->Node(inletNodeNum).MassFlowRateRequest = 0.0;
        if (releasePlantFlows && plantLoc.comp != nullptr) {
            Real64 requestedMassFlowRate = 0.0;
            PlantUtilities::SetComponentFlowRate(state, requestedMassFlowRate, inletNodeNum, outletNodeNum, plantLoc);
        }
    };
    releaseFlow(this->coolingPlantLoc, this->coolingInletNodeNum, this->coolingOutletNodeNum);
    releaseFlow(this->heatingPlantLoc, this->heatingInletNodeNum, this->heatingOutletNodeNum);
    releaseFlow(this->sourcePlantLoc, this->sourceInletNodeNum, this->sourceOutletNodeNum);
}

void CentralHeatPumpSystem::initialize(EnergyPlusData &state,
                                       Real64 load, // Demand Load
                                       int loopNum, // Loop Number Index
                                       bool const runFlag)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Daeho Kang, PNNL
    //       DATE WRITTEN   Feb 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //  This subroutine is for initializations of the CentralHeatPumpSystem
    //  variables

    // METHODOLOGY EMPLOYED:
    //  Uses the status flags to trigger initializations.

    if (this->setupOutputVarsFlag) {
        this->setupOutputVars(state);
        this->setupOutputVarsFlag = false;
    }

    if (this->plantScanPending) {
        // Locate the chillers on the plant loops for later usage
        bool errFlag = false;
        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::CentralHeatPumpSystem,
                                                this->coolingPlantLoc,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->coolingInletNodeNum,
                                                _);

        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::CentralHeatPumpSystem,
                                                this->heatingPlantLoc,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->heatingInletNodeNum,
                                                _);

        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::CentralHeatPumpSystem,
                                                this->sourcePlantLoc,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->sourceInletNodeNum,
                                                _);

        if (errFlag) {
            ShowFatalError(state,
                           std::format("CentralHeatPumpSystem::initialize: CentralHeatPumpSystem={} could not "
                                       "be located on all three connected plant loops.",
                                       this->Name));
        }

        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->coolingPlantLoc, this->sourcePlantLoc, DataPlant::PlantEquipmentType::CentralHeatPumpSystem, true);

        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->heatingPlantLoc, this->sourcePlantLoc, DataPlant::PlantEquipmentType::CentralHeatPumpSystem, true);

        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->coolingPlantLoc, this->heatingPlantLoc, DataPlant::PlantEquipmentType::CentralHeatPumpSystem, true);

        if (this->allModulesVariableFlow) { // why do this only for VS chiller heaters?
                                            // constant flow also uses set points.
            // Reset flow priority
            if (loopNum == this->coolingPlantLoc.loopNum) {
                DataPlant::CompData::getPlantComponent(state, this->coolingPlantLoc).FlowPriority = DataPlant::LoopFlowStatus::NeedyIfLoopOn;
            } else if (loopNum == this->heatingPlantLoc.loopNum) {
                DataPlant::CompData::getPlantComponent(state, this->heatingPlantLoc).FlowPriority = DataPlant::LoopFlowStatus::NeedyIfLoopOn;
            }
        } // moved up from below next 2 set point checks for #5808

        // check if setpoint on outlet node - chilled water loop
        if (state.dataLoopNodes->Node(this->coolingOutletNodeNum).TempSetPoint == Node::SensedNodeFlagValue) {
            if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                if (!this->coolingSetpointErrorIssued) {
                    ShowWarningError(state,
                                     std::format("Missing temperature setpoint on cooling side "
                                                 "for CentralHeatPumpSystem named {}",
                                                 this->Name));
                    ShowContinueError(state,
                                      "  A temperature setpoint is needed at the outlet node of "
                                      "a CentralHeatPumpSystem, use a SetpointManager");
                    ShowContinueError(state,
                                      "  The overall loop setpoint will be assumed for "
                                      "CentralHeatPumpSystem. The simulation continues ... ");
                    this->coolingSetpointErrorIssued = true;
                }
            } else {
                // need call to EMS to check node
                bool FatalError = false; // but not really fatal yet, but should be.
                EMSManager::CheckIfNodeSetPointManagedByEMS(state, this->coolingOutletNodeNum, HVAC::CtrlVarType::Temp, FatalError);
                state.dataLoopNodes->NodeSetpointCheck(this->coolingOutletNodeNum).needsSetpointChecking = false;
                if (FatalError) {
                    if (!this->coolingSetpointErrorIssued) {
                        ShowWarningError(state,
                                         std::format("Missing temperature setpoint on cooling "
                                                     "side for CentralHeatPumpSystem named {}",
                                                     this->Name));
                        ShowContinueError(state,
                                          "A temperature setpoint is needed at the "
                                          "outlet node of a CentralHeatPumpSystem ");
                        ShowContinueError(state,
                                          "use a Setpoint Manager to establish a setpoint "
                                          "at the chiller side outlet node ");
                        ShowContinueError(state,
                                          "or use an EMS actuator to establish a "
                                          "setpoint at the outlet node ");
                        ShowContinueError(state,
                                          "The overall loop setpoint will be assumed for "
                                          "chiller side. The simulation continues ... ");
                        this->coolingSetpointErrorIssued = true;
                    }
                }
            }
            this->coolingSetpointNodeNum = this->coolingPlantLoc.loop->TempSetPointNodeNum;
        }

        if (state.dataLoopNodes->Node(this->heatingOutletNodeNum).TempSetPoint == Node::SensedNodeFlagValue) {
            if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                if (!this->heatingSetpointErrorIssued) {
                    ShowWarningError(state,
                                     std::format("Missing temperature setpoint on heating side "
                                                 "for CentralHeatPumpSystem named {}",
                                                 this->Name));
                    ShowContinueError(state,
                                      "  A temperature setpoint is needed at the outlet node of "
                                      "a CentralHeatPumpSystem, use a SetpointManager");
                    ShowContinueError(state,
                                      "  The overall loop setpoint will be assumed for "
                                      "CentralHeatPumpSystem. The simulation continues ... ");
                    this->heatingSetpointErrorIssued = true;
                }
            } else {
                // need call to EMS to check node
                bool FatalError = false; // but not really fatal yet, but should be.
                EMSManager::CheckIfNodeSetPointManagedByEMS(state, this->heatingOutletNodeNum, HVAC::CtrlVarType::Temp, FatalError);
                state.dataLoopNodes->NodeSetpointCheck(this->heatingOutletNodeNum).needsSetpointChecking = false;
                if (FatalError) {
                    if (!this->heatingSetpointErrorIssued) {
                        ShowWarningError(state,
                                         std::format("Missing temperature setpoint on heating "
                                                     "side for CentralHeatPumpSystem named {}",
                                                     this->Name));
                        ShowContinueError(state,
                                          "A temperature setpoint is needed at the "
                                          "outlet node of a CentralHeatPumpSystem ");
                        ShowContinueError(state,
                                          "use a Setpoint Manager to establish a setpoint "
                                          "at the chiller side outlet node ");
                        ShowContinueError(state,
                                          "or use an EMS actuator to establish a "
                                          "setpoint at the outlet node ");
                        ShowContinueError(state,
                                          "The overall loop setpoint will be assumed for "
                                          "chiller side. The simulation continues ... ");
                        this->heatingSetpointErrorIssued = true;
                    }
                }
            }
            this->heatingSetpointNodeNum = this->heatingPlantLoc.loop->TempSetPointNodeNum;
        }
        this->plantScanPending = false;
    }

    if (this->environmentInitPending && state.dataGlobal->BeginEnvrnFlag && (state.dataPlnt->PlantFirstSizesOkayToFinalize)) {
        this->initializeDesignFlowLimits(state);
        this->resetOffState(state, false);
        this->environmentInitPending = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->environmentInitPending = true;
    }

    Real64 coolingMassFlowRate; // Chilled water mass flow rate
    Real64 heatingMassFlowRate; // Hot water mass flow rate
    Real64 sourceMassFlowRate;  // Condenser water mass flow rate

    if (!runFlag) {
        if (loopNum == this->coolingPlantLoc.loopNum) {
            coolingMassFlowRate = 0.0;
            PlantUtilities::SetComponentFlowRate(
                state, coolingMassFlowRate, this->coolingInletNodeNum, this->coolingOutletNodeNum, this->coolingPlantLoc);
        } else if (loopNum == this->heatingPlantLoc.loopNum) {
            heatingMassFlowRate = 0.0;
            PlantUtilities::SetComponentFlowRate(
                state, heatingMassFlowRate, this->heatingInletNodeNum, this->heatingOutletNodeNum, this->heatingPlantLoc);
        } else if (loopNum == this->sourcePlantLoc.loopNum) {
            sourceMassFlowRate = 0.0;
            PlantUtilities::SetComponentFlowRate(
                state, sourceMassFlowRate, this->sourceInletNodeNum, this->sourceOutletNodeNum, this->sourcePlantLoc);
        }
        return;
    }

    // Switch over the mass flow rate to the condenser loop, i.e., ground heat
    // exchanger
    if (loopNum == this->coolingPlantLoc.loopNum) { // called for on cooling loop
        if (load < -1.0) {                          // calling for cooling
            coolingMassFlowRate = state.dataLoopNodes->Node(this->coolingInletNodeNum).MassFlowRateMax;
        } else {
            coolingMassFlowRate = 0.0;
        }
        if (this->requestedHeatingLoad > 1.0) {
            heatingMassFlowRate = state.dataLoopNodes->Node(this->heatingInletNodeNum).MassFlowRateMax;
        } else {
            heatingMassFlowRate = 0.0;
        }
        if ((load < -1.0) || (this->requestedHeatingLoad > 1.0)) {
            sourceMassFlowRate = state.dataLoopNodes->Node(this->sourceInletNodeNum).MassFlowRateMax;
        } else {
            sourceMassFlowRate = 0.0;
        }

    } else if (loopNum == this->heatingPlantLoc.loopNum) {
        if (load > 1.0) {
            heatingMassFlowRate = state.dataLoopNodes->Node(this->heatingInletNodeNum).MassFlowRateMax;
        } else {
            heatingMassFlowRate = 0.0;
        }
        if (this->requestedCoolingLoad > 1.0) {
            coolingMassFlowRate = state.dataLoopNodes->Node(this->coolingInletNodeNum).MassFlowRateMax;
        } else {
            coolingMassFlowRate = 0.0;
        }
        if ((load > 1.0) || (this->requestedCoolingLoad > 1.0)) {
            sourceMassFlowRate = state.dataLoopNodes->Node(this->sourceInletNodeNum).MassFlowRateMax;
        } else {
            sourceMassFlowRate = 0.0;
        }

    } else if (loopNum == this->sourcePlantLoc.loopNum) {
        if (this->requestedCoolingLoad > 1.0) {
            coolingMassFlowRate = state.dataLoopNodes->Node(this->coolingInletNodeNum).MassFlowRateMax;
        } else {
            coolingMassFlowRate = 0.0;
        }
        if (this->requestedHeatingLoad > 1.0) {
            heatingMassFlowRate = state.dataLoopNodes->Node(this->heatingInletNodeNum).MassFlowRateMax;
        } else {
            heatingMassFlowRate = 0.0;
        }
        if ((this->requestedHeatingLoad > 1.0) || (this->requestedCoolingLoad > 1.0)) {
            sourceMassFlowRate = state.dataLoopNodes->Node(this->sourceInletNodeNum).MassFlowRateMax;
        } else {
            sourceMassFlowRate = 0.0;
        }
    }

    PlantUtilities::SetComponentFlowRate(state, coolingMassFlowRate, this->coolingInletNodeNum, this->coolingOutletNodeNum, this->coolingPlantLoc);

    PlantUtilities::SetComponentFlowRate(state, heatingMassFlowRate, this->heatingInletNodeNum, this->heatingOutletNodeNum, this->heatingPlantLoc);

    PlantUtilities::SetComponentFlowRate(state, sourceMassFlowRate, this->sourceInletNodeNum, this->sourceOutletNodeNum, this->sourcePlantLoc);
}

ModuleResult CentralHeatPumpSystem::solveCoolingOnly(EnergyPlusData &state,
                                                     std::size_t const moduleIndex,
                                                     Real64 const coolingLoad,
                                                     Real64 const evaporatorMassFlowRateMax,
                                                     Real64 const condenserMassFlowRate,
                                                     Real64 const evaporatorInletTemp,
                                                     Real64 const condenserInletTemp)
{
    static constexpr std::string_view routineName("CentralHeatPumpSystem cooling-only solver");

    auto &module = this->modules[moduleIndex];
    int const moduleNum = static_cast<int>(moduleIndex) + 1;
    auto const &performance = module.performanceData();
    ModePerformanceData const modePerformance = module.coolingModePerformance();
    ModuleResult result;
    result.requestedCoolingLoad = max(0.0, coolingLoad);
    result.evaporatorInletTemp = evaporatorInletTemp;
    result.evaporatorOutletTemp = evaporatorInletTemp;
    result.condenserInletTemp = condenserInletTemp;
    result.condenserOutletTemp = condenserInletTemp;
    result.unmetCoolingLoad = result.requestedCoolingLoad;

    if (result.requestedCoolingLoad <= HVAC::SmallLoad || evaporatorMassFlowRateMax <= DataBranchAirLoopPlant::MassFlowTolerance ||
        condenserMassFlowRate <= DataBranchAirLoopPlant::MassFlowTolerance || modePerformance.referenceEvaporatorCapacity <= 0.0 ||
        modePerformance.referenceCOP <= 0.0) {
        result.updatePowerAccounting(performance.compressorMotorEfficiency);
        return result;
    }

    Real64 const minPartLoadRatio = modePerformance.minimumPartLoadRatio;
    Real64 const maxPartLoadRatio = modePerformance.maximumPartLoadRatio;

    Real64 const evaporatorCp = this->coolingPlantLoc.loop->glycol->getSpecificHeat(state, evaporatorInletTemp, routineName);
    Real64 const condenserCp = this->sourcePlantLoc.loop->glycol->getSpecificHeat(state, condenserInletTemp, routineName);
    Real64 evaporatorOutletTarget = state.dataLoopNodes->Node(this->coolingSetpointNodeNum).TempSetPoint;
    if (evaporatorOutletTarget == Node::SensedNodeFlagValue) {
        evaporatorOutletTarget = module.minimumEvaporatorOutletTemp;
    }
    evaporatorOutletTarget = max(evaporatorOutletTarget, module.minimumEvaporatorOutletTemp);
    Real64 const evaporatorDeltaTempTarget = max(0.0, evaporatorInletTemp - evaporatorOutletTarget);
    Real64 const flowLimitedCooling = evaporatorMassFlowRateMax * evaporatorCp * evaporatorDeltaTempTarget;

    if (flowLimitedCooling <= HVAC::SmallLoad) {
        result.updatePowerAccounting(performance.compressorMotorEfficiency);
        return result;
    }

    Real64 evaporatorOutletGuess = evaporatorOutletTarget;
    Real64 condenserOutletGuess = condenserInletTemp;
    Real64 availableEvaporatorCapacity = 0.0;
    Real64 qEvaporator = 0.0;
    Real64 qCondenser = 0.0;
    Real64 compressorPower = 0.0;
    Real64 falseLoadRate = 0.0;
    Real64 partLoadRatio = 0.0;
    Real64 cyclingRatio = 0.0;
    Real64 evaporatorMassFlowRate = evaporatorMassFlowRateMax;
    Real64 evaporatorOutletTemp = evaporatorInletTemp;
    Real64 condenserOutletTemp = condenserInletTemp;
    Real64 capacityModifier = 0.0;
    Real64 eirTemperatureModifier = 0.0;
    Real64 eirPartLoadModifier = 0.0;

    for (int iteration = 0; iteration < maxOuterSolverIterations; ++iteration) {
        result.solver.outerIterations = iteration + 1;
        Real64 const condenserCurveTemp = selectCondenserCurveTemperature(modePerformance, condenserInletTemp, condenserOutletGuess);
        ++result.solver.curveEvaluations;
        capacityModifier = evaluateCapacityTemperatureModifier(state, module, modePerformance, evaporatorOutletGuess, condenserCurveTemp);
        availableEvaporatorCapacity = modePerformance.referenceEvaporatorCapacity * capacityModifier;
        qEvaporator = std::min({result.requestedCoolingLoad, availableEvaporatorCapacity * maxPartLoadRatio, flowLimitedCooling});

        if (qEvaporator <= HVAC::SmallLoad || availableEvaporatorCapacity <= 0.0) {
            qEvaporator = 0.0;
            break;
        }

        if (this->allModulesVariableFlow) {
            evaporatorMassFlowRate = min(evaporatorMassFlowRateMax, qEvaporator / (evaporatorCp * evaporatorDeltaTempTarget));
        } else {
            evaporatorMassFlowRate = evaporatorMassFlowRateMax;
        }
        evaporatorOutletTemp = evaporatorInletTemp - qEvaporator / (evaporatorMassFlowRate * evaporatorCp);

        Real64 const requestedPartLoadRatio = qEvaporator / availableEvaporatorCapacity;
        partLoadRatio = min(maxPartLoadRatio, max(requestedPartLoadRatio, minPartLoadRatio));
        cyclingRatio = minPartLoadRatio > 0.0 ? min(1.0, requestedPartLoadRatio / minPartLoadRatio) : 1.0;
        falseLoadRate = max(0.0, availableEvaporatorCapacity * partLoadRatio * cyclingRatio - qEvaporator);

        ++result.solver.curveEvaluations;
        eirTemperatureModifier =
            max(0.0, Curve::CurveValue(state, modePerformance.eirTemperatureCurveIndex, evaporatorOutletTemp, condenserCurveTemp));
        ++result.solver.curveEvaluations;
        eirPartLoadModifier = max(0.0, evaluatePartLoadCurve(state, modePerformance.eirPartLoadCurveIndex, condenserCurveTemp, partLoadRatio));
        compressorPower = (availableEvaporatorCapacity / modePerformance.referenceCOP) * eirTemperatureModifier * eirPartLoadModifier * cyclingRatio;
        qCondenser = qEvaporator + falseLoadRate + compressorPower * performance.compressorMotorEfficiency;
        condenserOutletTemp = condenserInletTemp + qCondenser / (condenserMassFlowRate * condenserCp);

        Real64 const residual = max(std::abs(evaporatorOutletTemp - evaporatorOutletGuess), std::abs(condenserOutletTemp - condenserOutletGuess));
        result.solver.temperatureResidual = residual;
        if (!std::isfinite(residual)) {
            result.solver.outerStatus = SolverConvergenceStatus::Invalid;
            break;
        }
        if (residual <= temperatureConvergenceTolerance) {
            result.solver.outerStatus = SolverConvergenceStatus::Converged;
            break;
        }
        Real64 const nextEvaporatorOutletGuess = 0.5 * (evaporatorOutletGuess + evaporatorOutletTemp);
        Real64 const nextCondenserOutletGuess = 0.5 * (condenserOutletGuess + condenserOutletTemp);
        if (nextEvaporatorOutletGuess == evaporatorOutletGuess && nextCondenserOutletGuess == condenserOutletGuess) {
            result.solver.outerStatus = SolverConvergenceStatus::Stagnated;
            break;
        }
        evaporatorOutletGuess = nextEvaporatorOutletGuess;
        condenserOutletGuess = nextCondenserOutletGuess;
    }

    if (result.solver.outerStatus == SolverConvergenceStatus::NotRequired && result.solver.outerIterations == maxOuterSolverIterations) {
        result.solver.outerStatus = SolverConvergenceStatus::IterationLimit;
    }
    if (!state.dataGlobal->WarmupFlag && solverFailed(result.solver.outerStatus)) {
        reportSolverFailure(state,
                            this->Name,
                            module.name(),
                            moduleNum,
                            "cooling-only",
                            "temperature iteration",
                            result.solver.outerStatus,
                            std::format("cooling load={:.6g} W", result.requestedCoolingLoad),
                            result.solver.outerIterations,
                            result.solver.temperatureResidual,
                            0.0,
                            condenserOutletTemp,
                            "C",
                            module.coolingSolverWarning);
    }

    if (qEvaporator <= HVAC::SmallLoad) {
        result.updatePowerAccounting(performance.compressorMotorEfficiency);
        return result;
    }

    Real64 const condenserCurveTemp = selectCondenserCurveTemperature(modePerformance, condenserInletTemp, condenserOutletTemp);
    ++result.solver.curveEvaluations;
    capacityModifier = evaluateCapacityTemperatureModifier(state, module, modePerformance, evaporatorOutletTemp, condenserCurveTemp);
    availableEvaporatorCapacity = modePerformance.referenceEvaporatorCapacity * capacityModifier;
    ++result.solver.curveEvaluations;
    eirTemperatureModifier = max(0.0, Curve::CurveValue(state, modePerformance.eirTemperatureCurveIndex, evaporatorOutletTemp, condenserCurveTemp));
    ++result.solver.curveEvaluations;
    eirPartLoadModifier = max(0.0, evaluatePartLoadCurve(state, modePerformance.eirPartLoadCurveIndex, condenserCurveTemp, partLoadRatio));
    compressorPower = (availableEvaporatorCapacity / modePerformance.referenceCOP) * eirTemperatureModifier * eirPartLoadModifier * cyclingRatio;
    falseLoadRate = max(0.0, availableEvaporatorCapacity * partLoadRatio * cyclingRatio - qEvaporator);
    qCondenser = qEvaporator + falseLoadRate + compressorPower * performance.compressorMotorEfficiency;
    condenserOutletTemp = condenserInletTemp + qCondenser / (condenserMassFlowRate * condenserCp);

    ++result.solver.curveEvaluations;
    Real64 const availableEIRPartLoadModifier =
        max(0.0, evaluatePartLoadCurve(state, modePerformance.eirPartLoadCurveIndex, condenserCurveTemp, maxPartLoadRatio));
    Real64 const availablePower =
        (availableEvaporatorCapacity / modePerformance.referenceCOP) * eirTemperatureModifier * availableEIRPartLoadModifier;

    result.currentMode = CurrentMode::CoolingOnly;
    result.availableEvaporatorCapacity = availableEvaporatorCapacity;
    result.availableCondenserCapacity = availableEvaporatorCapacity * maxPartLoadRatio + availablePower * performance.compressorMotorEfficiency;
    result.qEvaporator = qEvaporator;
    result.qCondenser = qCondenser;
    result.coolingPower = compressorPower;
    result.falseLoadRate = falseLoadRate;
    result.partLoadRatio = partLoadRatio;
    result.cyclingRatio = cyclingRatio;
    result.unloadingRatio = partLoadRatio;
    result.capacityTemperatureModifier = capacityModifier;
    result.eirTemperatureModifier = eirTemperatureModifier;
    result.eirPartLoadModifier = eirPartLoadModifier;
    result.capacityCurveEvaporatorTemp = evaporatorOutletTemp;
    result.capacityCurveCondenserTemp = condenserCurveTemp;
    result.eirCurveEvaporatorTemp = evaporatorOutletTemp;
    result.eirCurveCondenserTemp = condenserCurveTemp;
    result.eirPartLoadCurvePLR = partLoadRatio;
    result.eirPartLoadCurveCondenserTemp = condenserCurveTemp;
    result.actualCOP = compressorPower > 0.0 ? (qEvaporator + falseLoadRate) / compressorPower : 0.0;
    result.evaporatorOutletTemp = evaporatorOutletTemp;
    result.evaporatorMassFlowRate = evaporatorMassFlowRate;
    result.condenserOutletTemp = condenserOutletTemp;
    result.condenserMassFlowRate = condenserMassFlowRate;
    result.unmetCoolingLoad = max(0.0, result.requestedCoolingLoad - qEvaporator);
    result.updatePowerAccounting(performance.compressorMotorEfficiency);
    return result;
}

ModuleResult CentralHeatPumpSystem::solveHeatingOnly(EnergyPlusData &state,
                                                     std::size_t const moduleIndex,
                                                     Real64 const heatingLoad,
                                                     Real64 const evaporatorMassFlowRate,
                                                     Real64 const condenserMassFlowRateMax,
                                                     Real64 const evaporatorInletTemp,
                                                     Real64 const condenserInletTemp)
{
    static constexpr std::string_view routineName("CentralHeatPumpSystem heating-only solver");

    auto &module = this->modules[moduleIndex];
    int const moduleNum = static_cast<int>(moduleIndex) + 1;
    auto const &performance = module.performanceData();
    ModePerformanceData const modePerformance = module.heatingModePerformance();
    ModuleResult result;
    result.requestedHeatingLoad = max(0.0, heatingLoad);
    result.evaporatorInletTemp = evaporatorInletTemp;
    result.evaporatorOutletTemp = evaporatorInletTemp;
    result.condenserInletTemp = condenserInletTemp;
    result.condenserOutletTemp = condenserInletTemp;
    result.unmetHeatingLoad = result.requestedHeatingLoad;

    if (result.requestedHeatingLoad <= HVAC::SmallLoad || evaporatorMassFlowRate <= DataBranchAirLoopPlant::MassFlowTolerance ||
        condenserMassFlowRateMax <= DataBranchAirLoopPlant::MassFlowTolerance || modePerformance.referenceEvaporatorCapacity <= 0.0 ||
        modePerformance.referenceCOP <= 0.0) {
        result.updatePowerAccounting(performance.compressorMotorEfficiency);
        return result;
    }

    Real64 const minPartLoadRatio = modePerformance.minimumPartLoadRatio;
    Real64 const maxPartLoadRatio = modePerformance.maximumPartLoadRatio;

    Real64 const evaporatorCp = this->sourcePlantLoc.loop->glycol->getSpecificHeat(state, evaporatorInletTemp, routineName);
    Real64 const condenserCp = this->heatingPlantLoc.loop->glycol->getSpecificHeat(state, condenserInletTemp, routineName);
    Real64 const evaporatorOutletLowLimit = max(performance.minimumEvaporatorOutletTemp, module.minimumEvaporatorOutletTemp);
    Real64 const sourceLimitedEvaporatorHeat = max(0.0, evaporatorMassFlowRate * evaporatorCp * (evaporatorInletTemp - evaporatorOutletLowLimit));

    bool hasCondenserOutletLimit = false;
    Real64 condenserOutletLimit = 0.0;
    Real64 const plantHeatingSetPoint = state.dataLoopNodes->Node(this->heatingSetpointNodeNum).TempSetPoint;
    if (plantHeatingSetPoint != Node::SensedNodeFlagValue) {
        condenserOutletLimit = plantHeatingSetPoint;
        hasCondenserOutletLimit = true;
    }
    Real64 const heatingLimitedCondenserHeat = hasCondenserOutletLimit
                                                   ? max(0.0, condenserMassFlowRateMax * condenserCp * (condenserOutletLimit - condenserInletTemp))
                                                   : std::numeric_limits<Real64>::max();

    if (sourceLimitedEvaporatorHeat <= HVAC::SmallLoad || heatingLimitedCondenserHeat <= HVAC::SmallLoad) {
        result.updatePowerAccounting(performance.compressorMotorEfficiency);
        return result;
    }

    Real64 evaporatorOutletGuess =
        max(evaporatorOutletLowLimit, evaporatorInletTemp - sourceLimitedEvaporatorHeat / (evaporatorMassFlowRate * evaporatorCp));
    Real64 condenserOutletGuess =
        hasCondenserOutletLimit
            ? min(condenserOutletLimit, condenserInletTemp + result.requestedHeatingLoad / (condenserMassFlowRateMax * condenserCp))
            : condenserInletTemp;
    Real64 availableEvaporatorCapacity = 0.0;
    Real64 availableCondenserCapacity = 0.0;
    Real64 qEvaporator = 0.0;
    Real64 qCondenser = 0.0;
    Real64 compressorPower = 0.0;
    Real64 partLoadRatio = 0.0;
    Real64 cyclingRatio = 0.0;
    Real64 condenserMassFlowRate = condenserMassFlowRateMax;
    Real64 evaporatorOutletTemp = evaporatorInletTemp;
    Real64 condenserOutletTemp = condenserInletTemp;
    Real64 capacityModifier = 0.0;
    Real64 eirTemperatureModifier = 0.0;
    Real64 eirPartLoadModifier = 0.0;

    for (int iteration = 0; iteration < maxOuterSolverIterations; ++iteration) {
        result.solver.outerIterations = iteration + 1;
        result.solver.partLoadStatus = SolverConvergenceStatus::NotRequired;
        result.solver.partLoadBracketWidth = 0.0;
        result.solver.loadResidual = 0.0;
        Real64 const condenserCurveTemp = selectCondenserCurveTemperature(modePerformance, condenserInletTemp, condenserOutletGuess);
        ++result.solver.curveEvaluations;
        capacityModifier = evaluateCapacityTemperatureModifier(state, module, modePerformance, evaporatorOutletGuess, condenserCurveTemp);
        availableEvaporatorCapacity = modePerformance.referenceEvaporatorCapacity * capacityModifier;
        ++result.solver.curveEvaluations;
        eirTemperatureModifier =
            max(0.0, Curve::CurveValue(state, modePerformance.eirTemperatureCurveIndex, evaporatorOutletGuess, condenserCurveTemp));

        if (availableEvaporatorCapacity <= 0.0) {
            break;
        }

        auto operatingPointAtPLR = [&](Real64 const plr) {
            PartLoadOperatingPoint point;
            point.partLoadRatio = plr;
            ++result.solver.curveEvaluations;
            point.eirPartLoadModifier = max(0.0, evaluatePartLoadCurve(state, modePerformance.eirPartLoadCurveIndex, condenserCurveTemp, plr));
            point.evaporatorLoad = availableEvaporatorCapacity * plr;
            point.compressorPower = (availableEvaporatorCapacity / modePerformance.referenceCOP) * eirTemperatureModifier * point.eirPartLoadModifier;
            point.condenserLoad = point.evaporatorLoad + point.compressorPower * performance.compressorMotorEfficiency;
            return point;
        };

        Real64 maximumAllowedPLR = min(maxPartLoadRatio, sourceLimitedEvaporatorHeat / availableEvaporatorCapacity);
        Real64 maximumCyclingRatio = 1.0;
        if (maximumAllowedPLR < minPartLoadRatio) {
            maximumCyclingRatio = minPartLoadRatio > 0.0 ? max(0.0, maximumAllowedPLR / minPartLoadRatio) : 0.0;
            maximumAllowedPLR = minPartLoadRatio;
        }
        auto const maximumPoint = operatingPointAtPLR(maximumAllowedPLR);
        availableCondenserCapacity = maximumPoint.condenserLoad * maximumCyclingRatio;
        availableCondenserCapacity = min(availableCondenserCapacity, heatingLimitedCondenserHeat);
        Real64 const targetCondenserHeat = min(result.requestedHeatingLoad, availableCondenserCapacity);
        if (targetCondenserHeat <= HVAC::SmallLoad) {
            break;
        }

        auto const minimumPoint = operatingPointAtPLR(minPartLoadRatio);
        if (targetCondenserHeat < minimumPoint.condenserLoad) {
            partLoadRatio = minPartLoadRatio;
            cyclingRatio = min(maximumCyclingRatio, targetCondenserHeat / minimumPoint.condenserLoad);
        } else {
            cyclingRatio = 1.0;
            Real64 const loadScale = max({targetCondenserHeat, availableCondenserCapacity, 1.0});
            const auto [value, bracketWidth, loadResidual, iterations, status] =
                solveBisection(minPartLoadRatio, maximumAllowedPLR, targetCondenserHeat, partLoadAbsoluteTolerance, loadScale, [&](Real64 const plr) {
                    return operatingPointAtPLR(plr).condenserLoad;
                });
            result.solver.partLoadIterations += iterations;
            result.solver.partLoadStatus = status;
            result.solver.partLoadBracketWidth = bracketWidth;
            result.solver.loadResidual = loadResidual;
            partLoadRatio = value;
        }

        auto const operatingPoint = operatingPointAtPLR(partLoadRatio);
        qEvaporator = availableEvaporatorCapacity * partLoadRatio * cyclingRatio;
        compressorPower = operatingPoint.compressorPower * cyclingRatio;
        eirPartLoadModifier = operatingPoint.eirPartLoadModifier;
        qCondenser = qEvaporator + compressorPower * performance.compressorMotorEfficiency;

        if (this->allModulesVariableFlow && hasCondenserOutletLimit && condenserOutletLimit > condenserInletTemp) {
            condenserMassFlowRate = min(condenserMassFlowRateMax, qCondenser / (condenserCp * (condenserOutletLimit - condenserInletTemp)));
        } else {
            condenserMassFlowRate = condenserMassFlowRateMax;
        }
        evaporatorOutletTemp = evaporatorInletTemp - qEvaporator / (evaporatorMassFlowRate * evaporatorCp);
        condenserOutletTemp = condenserInletTemp + qCondenser / (condenserMassFlowRate * condenserCp);

        Real64 const residual = max(std::abs(evaporatorOutletTemp - evaporatorOutletGuess), std::abs(condenserOutletTemp - condenserOutletGuess));
        result.solver.temperatureResidual = residual;
        if (!std::isfinite(residual)) {
            result.solver.outerStatus = SolverConvergenceStatus::Invalid;
            break;
        }
        if (residual <= temperatureConvergenceTolerance) {
            result.solver.outerStatus = SolverConvergenceStatus::Converged;
            break;
        }
        Real64 const nextEvaporatorOutletGuess = 0.5 * (evaporatorOutletGuess + evaporatorOutletTemp);
        Real64 const nextCondenserOutletGuess = 0.5 * (condenserOutletGuess + condenserOutletTemp);
        if (nextEvaporatorOutletGuess == evaporatorOutletGuess && nextCondenserOutletGuess == condenserOutletGuess) {
            result.solver.outerStatus = SolverConvergenceStatus::Stagnated;
            break;
        }
        evaporatorOutletGuess = nextEvaporatorOutletGuess;
        condenserOutletGuess = nextCondenserOutletGuess;
    }

    if (result.solver.outerStatus == SolverConvergenceStatus::NotRequired && result.solver.outerIterations == maxOuterSolverIterations) {
        result.solver.outerStatus = SolverConvergenceStatus::IterationLimit;
    }
    if (!state.dataGlobal->WarmupFlag && solverFailed(result.solver.partLoadStatus)) {
        reportSolverFailure(state,
                            this->Name,
                            module.name(),
                            moduleNum,
                            "heating-only",
                            "part-load iteration",
                            result.solver.partLoadStatus,
                            std::format("heating load={:.6g} W", result.requestedHeatingLoad),
                            result.solver.partLoadIterations,
                            result.solver.partLoadBracketWidth,
                            result.solver.loadResidual,
                            partLoadRatio,
                            "PLR",
                            module.heatingPartLoadSolverWarning);
    }
    if (!state.dataGlobal->WarmupFlag && solverFailed(result.solver.outerStatus)) {
        reportSolverFailure(state,
                            this->Name,
                            module.name(),
                            moduleNum,
                            "heating-only",
                            "temperature iteration",
                            result.solver.outerStatus,
                            std::format("heating load={:.6g} W", result.requestedHeatingLoad),
                            result.solver.outerIterations,
                            result.solver.temperatureResidual,
                            result.solver.loadResidual,
                            condenserOutletTemp,
                            "C",
                            module.heatingSolverWarning);
    }

    if (qCondenser <= HVAC::SmallLoad) {
        result.updatePowerAccounting(performance.compressorMotorEfficiency);
        return result;
    }

    Real64 const condenserCurveTemp = selectCondenserCurveTemperature(modePerformance, condenserInletTemp, condenserOutletTemp);
    ++result.solver.curveEvaluations;
    capacityModifier = evaluateCapacityTemperatureModifier(state, module, modePerformance, evaporatorOutletTemp, condenserCurveTemp);
    availableEvaporatorCapacity = modePerformance.referenceEvaporatorCapacity * capacityModifier;
    ++result.solver.curveEvaluations;
    eirTemperatureModifier = max(0.0, Curve::CurveValue(state, modePerformance.eirTemperatureCurveIndex, evaporatorOutletTemp, condenserCurveTemp));
    ++result.solver.curveEvaluations;
    eirPartLoadModifier = max(0.0, evaluatePartLoadCurve(state, modePerformance.eirPartLoadCurveIndex, condenserCurveTemp, partLoadRatio));
    compressorPower = (availableEvaporatorCapacity / modePerformance.referenceCOP) * eirTemperatureModifier * eirPartLoadModifier * cyclingRatio;
    qEvaporator = availableEvaporatorCapacity * partLoadRatio * cyclingRatio;
    qCondenser = qEvaporator + compressorPower * performance.compressorMotorEfficiency;
    evaporatorOutletTemp = evaporatorInletTemp - qEvaporator / (evaporatorMassFlowRate * evaporatorCp);
    condenserOutletTemp = condenserInletTemp + qCondenser / (condenserMassFlowRate * condenserCp);

    result.currentMode = CurrentMode::HeatingOnly;
    result.availableEvaporatorCapacity = availableEvaporatorCapacity;
    result.availableCondenserCapacity = availableCondenserCapacity;
    result.qEvaporator = qEvaporator;
    result.qCondenser = qCondenser;
    result.heatingPower = compressorPower;
    result.partLoadRatio = partLoadRatio;
    result.cyclingRatio = cyclingRatio;
    result.unloadingRatio = partLoadRatio;
    result.capacityTemperatureModifier = capacityModifier;
    result.eirTemperatureModifier = eirTemperatureModifier;
    result.eirPartLoadModifier = eirPartLoadModifier;
    result.capacityCurveEvaporatorTemp = evaporatorOutletTemp;
    result.capacityCurveCondenserTemp = condenserCurveTemp;
    result.eirCurveEvaporatorTemp = evaporatorOutletTemp;
    result.eirCurveCondenserTemp = condenserCurveTemp;
    result.eirPartLoadCurvePLR = partLoadRatio;
    result.eirPartLoadCurveCondenserTemp = condenserCurveTemp;
    result.actualCOP = compressorPower > 0.0 ? qCondenser / compressorPower : 0.0;
    result.evaporatorOutletTemp = evaporatorOutletTemp;
    result.evaporatorMassFlowRate = evaporatorMassFlowRate;
    result.condenserOutletTemp = condenserOutletTemp;
    result.condenserMassFlowRate = condenserMassFlowRate;
    result.unmetHeatingLoad = max(0.0, result.requestedHeatingLoad - qCondenser);
    result.updatePowerAccounting(performance.compressorMotorEfficiency);
    return result;
}

ModuleResult CentralHeatPumpSystem::solveSimultaneous(EnergyPlusData &state,
                                                      std::size_t const moduleIndex,
                                                      Real64 const coolingLoad,
                                                      Real64 const heatingLoad,
                                                      Real64 const maximumCoolingMassFlowRate,
                                                      Real64 const maximumHeatingMassFlowRate,
                                                      Real64 const maximumSourceMassFlowRate,
                                                      Real64 const coolingInletTemp,
                                                      Real64 const heatingInletTemp,
                                                      Real64 const sourceInletTemp)
{
    static constexpr std::string_view routineName("CentralHeatPumpSystem simultaneous solver");

    auto &module = this->modules[moduleIndex];
    int const moduleNum = static_cast<int>(moduleIndex) + 1;
    auto const &performance = module.performanceData();
    auto const &sizing = module.sizing;
    ModePerformanceData const modePerformance = module.heatingModePerformance();
    ModuleResult result;
    result.requestedCoolingLoad = max(0.0, coolingLoad);
    result.requestedHeatingLoad = max(0.0, heatingLoad);
    result.unmetCoolingLoad = result.requestedCoolingLoad;
    result.unmetHeatingLoad = result.requestedHeatingLoad;
    result.coolingInletTemp = coolingInletTemp;
    result.coolingOutletTemp = coolingInletTemp;
    result.heatingInletTemp = heatingInletTemp;
    result.heatingOutletTemp = heatingInletTemp;
    result.sourceInletTemp = sourceInletTemp;
    result.sourceOutletTemp = sourceInletTemp;

    if (result.requestedCoolingLoad <= HVAC::SmallLoad || result.requestedHeatingLoad <= HVAC::SmallLoad ||
        maximumCoolingMassFlowRate <= DataBranchAirLoopPlant::MassFlowTolerance ||
        maximumHeatingMassFlowRate <= DataBranchAirLoopPlant::MassFlowTolerance || modePerformance.referenceEvaporatorCapacity <= 0.0 ||
        modePerformance.referenceCOP <= 0.0) {
        result.updatePowerAccounting(performance.compressorMotorEfficiency);
        return result;
    }

    Real64 const minPartLoadRatio = modePerformance.minimumPartLoadRatio;
    Real64 const maxPartLoadRatio = modePerformance.maximumPartLoadRatio;

    Real64 const coolingCp = this->coolingPlantLoc.loop->glycol->getSpecificHeat(state, coolingInletTemp, routineName);
    Real64 const heatingCp = this->heatingPlantLoc.loop->glycol->getSpecificHeat(state, heatingInletTemp, routineName);
    Real64 const sourceCp = this->sourcePlantLoc.loop->glycol->getSpecificHeat(state, sourceInletTemp, routineName);

    Real64 coolingOutletTarget = state.dataLoopNodes->Node(this->coolingSetpointNodeNum).TempSetPoint;
    if (coolingOutletTarget == Node::SensedNodeFlagValue) {
        coolingOutletTarget = module.minimumEvaporatorOutletTemp;
    }
    coolingOutletTarget = max(coolingOutletTarget, module.minimumEvaporatorOutletTemp);
    Real64 const coolingDeltaTempTarget = max(0.0, coolingInletTemp - coolingOutletTarget);
    Real64 const coolingTarget = min(result.requestedCoolingLoad, maximumCoolingMassFlowRate * coolingCp * coolingDeltaTempTarget);

    bool hasHeatingOutletLimit = false;
    Real64 heatingOutletLimit = 0.0;
    Real64 const plantHeatingSetPoint = state.dataLoopNodes->Node(this->heatingSetpointNodeNum).TempSetPoint;
    if (plantHeatingSetPoint != Node::SensedNodeFlagValue) {
        heatingOutletLimit = plantHeatingSetPoint;
        hasHeatingOutletLimit = true;
    }
    Real64 const heatingDeltaTempTarget = hasHeatingOutletLimit ? max(0.0, heatingOutletLimit - heatingInletTemp) : 0.0;
    Real64 const heatingLimitedHeating =
        hasHeatingOutletLimit ? maximumHeatingMassFlowRate * heatingCp * heatingDeltaTempTarget : std::numeric_limits<Real64>::max();
    Real64 const heatingTarget = min(result.requestedHeatingLoad, heatingLimitedHeating);

    auto sourceFlowLimit = [maximumSourceMassFlowRate](Real64 const nodeFlowLimit, Real64 const designFlowLimit) {
        Real64 flowLimit = nodeFlowLimit;
        if (flowLimit <= DataBranchAirLoopPlant::MassFlowTolerance) {
            flowLimit = designFlowLimit;
        }
        if (flowLimit <= DataBranchAirLoopPlant::MassFlowTolerance) {
            flowLimit = maximumSourceMassFlowRate;
        }
        return min(maximumSourceMassFlowRate, max(0.0, flowLimit));
    };
    Real64 const maximumSourceEvaporatorMassFlowRate =
        sourceFlowLimit(sizing.maximumSourceEvaporatorMassFlowRate, sizing.maximumEvaporatorMassFlowRate);
    Real64 const maximumSourceCondenserMassFlowRate = sourceFlowLimit(sizing.maximumSourceCondenserMassFlowRate, sizing.maximumCondenserMassFlowRate);
    Real64 const sourceOutletLowLimit = max(performance.minimumEvaporatorOutletTemp, module.minimumEvaporatorOutletTemp);
    Real64 const sourceExtractionCapacity = maximumSourceEvaporatorMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance
                                                ? max(0.0, maximumSourceEvaporatorMassFlowRate * sourceCp * (sourceInletTemp - sourceOutletLowLimit))
                                                : 0.0;
    bool const canRejectToSource = maximumSourceCondenserMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance;

    if (coolingTarget <= HVAC::SmallLoad || heatingTarget <= HVAC::SmallLoad) {
        result.updatePowerAccounting(performance.compressorMotorEfficiency);
        return result;
    }

    Real64 evaporatorCurveTempGuess = coolingOutletTarget;
    Real64 condenserEnteringTempGuess = heatingInletTemp;
    Real64 condenserLeavingTempGuess = hasHeatingOutletLimit
                                           ? min(heatingOutletLimit, heatingInletTemp + heatingTarget / (maximumHeatingMassFlowRate * heatingCp))
                                           : heatingInletTemp;

    Real64 availableEvaporatorCapacity = 0.0;
    Real64 availableCondenserCapacity = 0.0;
    Real64 qEvaporator = 0.0;
    Real64 qCondenser = 0.0;
    Real64 compressorPower = 0.0;
    Real64 falseLoadRate = 0.0;
    Real64 partLoadRatio = 0.0;
    Real64 cyclingRatio = 0.0;
    Real64 capacityModifier = 0.0;
    Real64 eirTemperatureModifier = 0.0;
    Real64 eirPartLoadModifier = 0.0;

    Real64 coolingDelivered = 0.0;
    Real64 heatingDelivered = 0.0;
    Real64 sourceExtraction = 0.0;
    Real64 sourceRejection = 0.0;
    Real64 coolingMassFlowRate = 0.0;
    Real64 heatingMassFlowRate = 0.0;
    Real64 sourceMassFlowRate = 0.0;
    Real64 coolingOutletTemp = coolingInletTemp;
    Real64 heatingOutletTemp = heatingInletTemp;
    Real64 sourceOutletTemp = sourceInletTemp;
    Real64 evaporatorInletTemp = coolingInletTemp;
    Real64 evaporatorOutletTemp = coolingInletTemp;
    Real64 evaporatorMassFlowRate = 0.0;
    Real64 condenserInletTemp = heatingInletTemp;
    Real64 condenserOutletTemp = heatingInletTemp;
    Real64 condenserMassFlowRate = 0.0;
    Real64 temperatureRelaxation = 0.5;
    std::array<Real64, 3> previousTemperatureResiduals{};
    bool havePreviousTemperatureResiduals = false;

    for (int iteration = 0; iteration < maxOuterSolverIterations; ++iteration) {
        result.solver.outerIterations = iteration + 1;
        result.solver.partLoadStatus = SolverConvergenceStatus::NotRequired;
        result.solver.partLoadBracketWidth = 0.0;
        result.solver.loadResidual = 0.0;
        Real64 const condenserCurveTemp = selectCondenserCurveTemperature(modePerformance, condenserEnteringTempGuess, condenserLeavingTempGuess);
        ++result.solver.curveEvaluations;
        capacityModifier = evaluateCapacityTemperatureModifier(state, module, modePerformance, evaporatorCurveTempGuess, condenserCurveTemp);
        availableEvaporatorCapacity = modePerformance.referenceEvaporatorCapacity * capacityModifier;
        ++result.solver.curveEvaluations;
        eirTemperatureModifier =
            max(0.0, Curve::CurveValue(state, modePerformance.eirTemperatureCurveIndex, evaporatorCurveTempGuess, condenserCurveTemp));
        if (availableEvaporatorCapacity <= 0.0) {
            break;
        }

        auto operatingPointAtEvaporatorLoad = [&](Real64 const evaporatorLoad) {
            PartLoadOperatingPoint point;
            if (evaporatorLoad <= HVAC::SmallLoad) {
                return point;
            }
            Real64 const requestedPartLoadRatio = evaporatorLoad / availableEvaporatorCapacity;
            point.partLoadRatio = min(maxPartLoadRatio, max(requestedPartLoadRatio, minPartLoadRatio));
            point.cyclingRatio =
                requestedPartLoadRatio < minPartLoadRatio && minPartLoadRatio > 0.0 ? requestedPartLoadRatio / minPartLoadRatio : 1.0;
            ++result.solver.curveEvaluations;
            point.eirPartLoadModifier =
                max(0.0, evaluatePartLoadCurve(state, modePerformance.eirPartLoadCurveIndex, condenserCurveTemp, point.partLoadRatio));
            point.evaporatorLoad = evaporatorLoad;
            point.compressorPower = (availableEvaporatorCapacity / modePerformance.referenceCOP) * eirTemperatureModifier *
                                    point.eirPartLoadModifier * point.cyclingRatio;
            point.falseLoadRate = max(0.0, availableEvaporatorCapacity * point.partLoadRatio * point.cyclingRatio - point.evaporatorLoad);
            point.condenserLoad = point.evaporatorLoad + point.falseLoadRate + point.compressorPower * performance.compressorMotorEfficiency;
            return point;
        };

        Real64 const maximumEvaporatorLoad = min(availableEvaporatorCapacity * maxPartLoadRatio, coolingTarget + sourceExtractionCapacity);
        auto coolingPoint = operatingPointAtEvaporatorLoad(min(coolingTarget, maximumEvaporatorLoad));
        auto maximumPoint = operatingPointAtEvaporatorLoad(maximumEvaporatorLoad);
        availableCondenserCapacity = maximumPoint.condenserLoad;
        auto selectedPoint = coolingPoint;

        bool const heatingDrivesCycle = coolingPoint.condenserLoad + HVAC::SmallLoad < heatingTarget;
        bool const excessHeatCannotBeRejected = !canRejectToSource && coolingPoint.condenserLoad > heatingTarget + HVAC::SmallLoad;
        if (heatingDrivesCycle || excessHeatCannotBeRejected) {
            Real64 const lowerEvaporatorLoad = heatingDrivesCycle ? coolingPoint.evaporatorLoad : 0.0;
            Real64 const upperEvaporatorLoad = heatingDrivesCycle ? maximumEvaporatorLoad : coolingPoint.evaporatorLoad;
            Real64 const boundedHeatingTarget = min(heatingTarget, operatingPointAtEvaporatorLoad(upperEvaporatorLoad).condenserLoad);
            Real64 const evaporatorCapacityScale = max(availableEvaporatorCapacity, 1.0);
            Real64 const loadScale = max({boundedHeatingTarget, availableCondenserCapacity, 1.0});
            auto const bisection =
                solveBisection(lowerEvaporatorLoad,
                               upperEvaporatorLoad,
                               boundedHeatingTarget,
                               partLoadAbsoluteTolerance * evaporatorCapacityScale,
                               loadScale,
                               [&](Real64 const evaporatorLoad) { return operatingPointAtEvaporatorLoad(evaporatorLoad).condenserLoad; });
            result.solver.partLoadIterations += bisection.iterations;
            result.solver.partLoadStatus = bisection.status;
            result.solver.partLoadBracketWidth = bisection.bracketWidth / evaporatorCapacityScale;
            result.solver.loadResidual = bisection.loadResidual;
            selectedPoint = operatingPointAtEvaporatorLoad(bisection.value);
        }

        qEvaporator = selectedPoint.evaporatorLoad;
        qCondenser = selectedPoint.condenserLoad;
        compressorPower = selectedPoint.compressorPower;
        falseLoadRate = selectedPoint.falseLoadRate;
        partLoadRatio = selectedPoint.partLoadRatio;
        cyclingRatio = selectedPoint.cyclingRatio;
        eirPartLoadModifier = selectedPoint.eirPartLoadModifier;

        coolingDelivered = min(coolingTarget, qEvaporator);
        sourceExtraction = max(0.0, qEvaporator - coolingDelivered);
        heatingDelivered = min(heatingTarget, qCondenser);
        sourceRejection = max(0.0, qCondenser - heatingDelivered);

        if (this->allModulesVariableFlow) {
            coolingMassFlowRate = coolingDelivered > HVAC::SmallLoad && coolingDeltaTempTarget > 0.0
                                      ? min(maximumCoolingMassFlowRate, coolingDelivered / (coolingCp * coolingDeltaTempTarget))
                                      : 0.0;
            heatingMassFlowRate = heatingDelivered > HVAC::SmallLoad && heatingDeltaTempTarget > 0.0
                                      ? min(maximumHeatingMassFlowRate, heatingDelivered / (heatingCp * heatingDeltaTempTarget))
                                      : maximumHeatingMassFlowRate;
        } else {
            coolingMassFlowRate = coolingDelivered > HVAC::SmallLoad ? maximumCoolingMassFlowRate : 0.0;
            heatingMassFlowRate = heatingDelivered > HVAC::SmallLoad ? maximumHeatingMassFlowRate : 0.0;
        }
        // Keep variable source flow continuous with residual source load so the blended curve temperatures do not jump at heat recovery.
        if (sourceExtraction > HVAC::SmallLoad) {
            if (this->allModulesVariableFlow && sourceExtractionCapacity > HVAC::SmallLoad) {
                sourceMassFlowRate = maximumSourceEvaporatorMassFlowRate * min(1.0, sourceExtraction / sourceExtractionCapacity);
            } else {
                sourceMassFlowRate = maximumSourceEvaporatorMassFlowRate;
            }
        } else if (sourceRejection > HVAC::SmallLoad) {
            if (this->allModulesVariableFlow && qCondenser > HVAC::SmallLoad) {
                sourceMassFlowRate = maximumSourceCondenserMassFlowRate * min(1.0, sourceRejection / qCondenser);
            } else {
                sourceMassFlowRate = maximumSourceCondenserMassFlowRate;
            }
        } else {
            sourceMassFlowRate = 0.0;
        }

        coolingOutletTemp = coolingMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance
                                ? coolingInletTemp - coolingDelivered / (coolingMassFlowRate * coolingCp)
                                : coolingInletTemp;
        heatingOutletTemp = heatingMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance
                                ? heatingInletTemp + heatingDelivered / (heatingMassFlowRate * heatingCp)
                                : heatingInletTemp;
        sourceOutletTemp = sourceMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance
                               ? sourceInletTemp + (sourceRejection - sourceExtraction) / (sourceMassFlowRate * sourceCp)
                               : sourceInletTemp;

        Real64 const evaporatorSourceMassFlowRate = sourceExtraction > HVAC::SmallLoad ? sourceMassFlowRate : 0.0;
        evaporatorMassFlowRate = coolingMassFlowRate + evaporatorSourceMassFlowRate;
        if (evaporatorMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
            evaporatorInletTemp = (coolingMassFlowRate * coolingInletTemp + evaporatorSourceMassFlowRate * sourceInletTemp) / evaporatorMassFlowRate;
            evaporatorOutletTemp =
                (coolingMassFlowRate * coolingOutletTemp + evaporatorSourceMassFlowRate * sourceOutletTemp) / evaporatorMassFlowRate;
        }

        Real64 const condenserSourceMassFlowRate = sourceRejection > HVAC::SmallLoad ? sourceMassFlowRate : 0.0;
        condenserMassFlowRate = heatingMassFlowRate + condenserSourceMassFlowRate;
        if (condenserMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
            condenserInletTemp = (heatingMassFlowRate * heatingInletTemp + condenserSourceMassFlowRate * sourceInletTemp) / condenserMassFlowRate;
            condenserOutletTemp = (heatingMassFlowRate * heatingOutletTemp + condenserSourceMassFlowRate * sourceOutletTemp) / condenserMassFlowRate;
        }

        std::array<Real64, 3> const temperatureResiduals = {evaporatorOutletTemp - evaporatorCurveTempGuess,
                                                            condenserInletTemp - condenserEnteringTempGuess,
                                                            condenserOutletTemp - condenserLeavingTempGuess};
        Real64 const temperatureResidual =
            max({std::abs(temperatureResiduals[0]), std::abs(temperatureResiduals[1]), std::abs(temperatureResiduals[2])});
        result.solver.temperatureResidual = temperatureResidual;
        if (!std::isfinite(temperatureResidual)) {
            result.solver.outerStatus = SolverConvergenceStatus::Invalid;
            break;
        }
        if (temperatureResidual <= temperatureConvergenceTolerance) {
            result.solver.outerStatus = SolverConvergenceStatus::Converged;
            break;
        }
        // Vector Aitken relaxation accelerates slow coupling and damps oscillatory temperature updates.
        if (havePreviousTemperatureResiduals) {
            std::array<Real64, 3> const residualChange = {temperatureResiduals[0] - previousTemperatureResiduals[0],
                                                          temperatureResiduals[1] - previousTemperatureResiduals[1],
                                                          temperatureResiduals[2] - previousTemperatureResiduals[2]};
            Real64 const residualChangeMagnitudeSquared =
                residualChange[0] * residualChange[0] + residualChange[1] * residualChange[1] + residualChange[2] * residualChange[2];
            if (residualChangeMagnitudeSquared > 0.0) {
                Real64 const previousResidualProjection = previousTemperatureResiduals[0] * residualChange[0] +
                                                          previousTemperatureResiduals[1] * residualChange[1] +
                                                          previousTemperatureResiduals[2] * residualChange[2];
                Real64 const acceleratedRelaxation = -temperatureRelaxation * previousResidualProjection / residualChangeMagnitudeSquared;
                if (std::isfinite(acceleratedRelaxation) && acceleratedRelaxation > 0.0) {
                    temperatureRelaxation = std::clamp(acceleratedRelaxation, 0.01, 1.0);
                } else {
                    temperatureRelaxation = max(0.01, 0.5 * temperatureRelaxation);
                }
            }
        }
        Real64 const nextEvaporatorCurveTempGuess = evaporatorCurveTempGuess + temperatureRelaxation * temperatureResiduals[0];
        Real64 const nextCondenserEnteringTempGuess = condenserEnteringTempGuess + temperatureRelaxation * temperatureResiduals[1];
        Real64 const nextCondenserLeavingTempGuess = condenserLeavingTempGuess + temperatureRelaxation * temperatureResiduals[2];
        if (nextEvaporatorCurveTempGuess == evaporatorCurveTempGuess && nextCondenserEnteringTempGuess == condenserEnteringTempGuess &&
            nextCondenserLeavingTempGuess == condenserLeavingTempGuess) {
            result.solver.outerStatus = SolverConvergenceStatus::Stagnated;
            break;
        }
        evaporatorCurveTempGuess = nextEvaporatorCurveTempGuess;
        condenserEnteringTempGuess = nextCondenserEnteringTempGuess;
        condenserLeavingTempGuess = nextCondenserLeavingTempGuess;
        previousTemperatureResiduals = temperatureResiduals;
        havePreviousTemperatureResiduals = true;
    }

    if (result.solver.outerStatus == SolverConvergenceStatus::NotRequired && result.solver.outerIterations == maxOuterSolverIterations) {
        result.solver.outerStatus = SolverConvergenceStatus::IterationLimit;
    }
    if (!state.dataGlobal->WarmupFlag && solverFailed(result.solver.partLoadStatus)) {
        reportSolverFailure(state,
                            this->Name,
                            module.name(),
                            moduleNum,
                            "simultaneous",
                            "part-load iteration",
                            result.solver.partLoadStatus,
                            std::format("cooling/heating loads={:.6g}/{:.6g} W", result.requestedCoolingLoad, result.requestedHeatingLoad),
                            result.solver.partLoadIterations,
                            result.solver.partLoadBracketWidth,
                            result.solver.loadResidual,
                            partLoadRatio,
                            "PLR",
                            module.simultaneousPartLoadSolverWarning);
    }
    if (!state.dataGlobal->WarmupFlag && solverFailed(result.solver.outerStatus)) {
        reportSolverFailure(state,
                            this->Name,
                            module.name(),
                            moduleNum,
                            "simultaneous",
                            "temperature iteration",
                            result.solver.outerStatus,
                            std::format("cooling/heating loads={:.6g}/{:.6g} W", result.requestedCoolingLoad, result.requestedHeatingLoad),
                            result.solver.outerIterations,
                            result.solver.temperatureResidual,
                            result.solver.loadResidual,
                            condenserOutletTemp,
                            "C",
                            module.simultaneousSolverWarning);
    }

    if (qEvaporator <= HVAC::SmallLoad || qCondenser <= HVAC::SmallLoad) {
        result.updatePowerAccounting(performance.compressorMotorEfficiency);
        return result;
    }

    Real64 const sourceHeatTransfer = sourceRejection - sourceExtraction;
    Real64 const modeTolerance = max(HVAC::SmallLoad, 1.0e-8 * max(qEvaporator, qCondenser));
    if (sourceHeatTransfer > modeTolerance) {
        result.currentMode = CurrentMode::CoolingDominant;
        result.coolingPower = compressorPower;
    } else if (sourceHeatTransfer < -modeTolerance) {
        result.currentMode = CurrentMode::HeatingDominant;
        result.heatingPower = compressorPower;
    } else {
        result.currentMode = CurrentMode::HeatRecovery;
        result.coolingPower = compressorPower;
    }

    result.availableEvaporatorCapacity = availableEvaporatorCapacity;
    result.availableCondenserCapacity = availableCondenserCapacity;
    result.qEvaporator = qEvaporator;
    result.qCondenser = qCondenser;
    result.falseLoadRate = falseLoadRate;
    result.partLoadRatio = partLoadRatio;
    result.cyclingRatio = cyclingRatio;
    result.unloadingRatio = partLoadRatio;
    result.capacityTemperatureModifier = capacityModifier;
    result.eirTemperatureModifier = eirTemperatureModifier;
    result.eirPartLoadModifier = eirPartLoadModifier;
    result.capacityCurveEvaporatorTemp = evaporatorOutletTemp;
    result.capacityCurveCondenserTemp = selectCondenserCurveTemperature(modePerformance, condenserInletTemp, condenserOutletTemp);
    result.eirCurveEvaporatorTemp = evaporatorOutletTemp;
    result.eirCurveCondenserTemp = result.capacityCurveCondenserTemp;
    result.eirPartLoadCurvePLR = partLoadRatio;
    result.eirPartLoadCurveCondenserTemp = result.capacityCurveCondenserTemp;
    result.actualCOP = compressorPower > 0.0 ? qCondenser / compressorPower : 0.0;
    result.evaporatorInletTemp = evaporatorInletTemp;
    result.evaporatorOutletTemp = evaporatorOutletTemp;
    result.evaporatorMassFlowRate = evaporatorMassFlowRate;
    result.condenserInletTemp = condenserInletTemp;
    result.condenserOutletTemp = condenserOutletTemp;
    result.condenserMassFlowRate = condenserMassFlowRate;
    result.coolingOutletTemp = coolingOutletTemp;
    result.coolingMassFlowRate = coolingMassFlowRate;
    result.heatingOutletTemp = heatingOutletTemp;
    result.heatingMassFlowRate = heatingMassFlowRate;
    result.sourceOutletTemp = sourceOutletTemp;
    result.sourceMassFlowRate = sourceMassFlowRate;
    result.coolingDelivered = coolingDelivered;
    result.heatingDelivered = heatingDelivered;
    result.heatRecovered = heatingDelivered;
    result.sourceHeatTransfer = sourceHeatTransfer;
    result.unmetCoolingLoad = max(0.0, result.requestedCoolingLoad - coolingDelivered);
    result.unmetHeatingLoad = max(0.0, result.requestedHeatingLoad - heatingDelivered);
    result.updatePowerAccounting(performance.compressorMotorEfficiency);
    return result;
}

Real64 CentralHeatPumpSystem::selectCondenserCurveTemperature(ModePerformanceData const &modePerformance,
                                                              Real64 const condenserEnteringTemp,
                                                              Real64 const condenserLeavingTemp)
{
    return modePerformance.condenserMode == CondenserTemperatureMode::EnteringCondenser ? condenserEnteringTemp : condenserLeavingTemp;
}

Real64 CentralHeatPumpSystem::evaluateCapacityTemperatureModifier(
    EnergyPlusData &state, Module &module, ModePerformanceData const &modePerformance, Real64 const evaporatorOutletTemp, Real64 const condenserTemp)
{
    Real64 capacityTemperatureModifier = Curve::CurveValue(state, modePerformance.capacityTemperatureCurveIndex, evaporatorOutletTemp, condenserTemp);

    if (capacityTemperatureModifier < 0.0) {
        if (module.capacityCurveErrorCount < 1 && !state.dataGlobal->WarmupFlag) {
            ++module.capacityCurveErrorCount;
            ShowWarningError(state, std::format("ChillerHeaterPerformance:Electric:EIR \"{}\":", module.name()));
            ShowContinueError(state,
                              std::format(" modules Capacity as a Function of "
                                          "Temperature curve output is negative ({:.3f}).",
                                          capacityTemperatureModifier));
            ShowContinueError(state,
                              std::format(" Negative value occurs using an Evaporator Outlet Temp "
                                          "of {:.1f} and a Condenser Inlet Temp of {:.1f}.",
                                          evaporatorOutletTemp,
                                          condenserTemp));
            ShowContinueErrorTimeStamp(state, " Resetting curve output to zero and continuing simulation.");
        } else if (!state.dataGlobal->WarmupFlag) {
            ++module.capacityCurveErrorCount;
            ShowRecurringWarningErrorAtEnd(state,
                                           "ChillerHeaterPerformance:Electric:EIR \"" + module.name() +
                                               "\": modules Capacity as a Function of Temperature curve "
                                               "output is negative warning continues...",
                                           module.capacityCurveErrorIndex,
                                           capacityTemperatureModifier,
                                           capacityTemperatureModifier);
        }
        capacityTemperatureModifier = 0.0;
    }
    return capacityTemperatureModifier;
}

void CentralHeatPumpSystem::calculateCoolingOnly(EnergyPlusData &state,
                                                 Real64 const coolingMassFlowRate,
                                                 Real64 const sourceMassFlowRate,
                                                 Real64 const coolingInletTemp,
                                                 Real64 const sourceInletTemp)
{
    Real64 remainingCoolingLoad = this->requestedCoolingLoad;
    Real64 remainingCoolingMassFlowRate = max(0.0, coolingMassFlowRate);
    Real64 remainingSourceMassFlowRate = max(0.0, sourceMassFlowRate);

    auto allocateConnectionFlow = [](Real64 const remainingFlow, Real64 const connectionDesignFlow, Real64 const legacyDesignFlow) {
        Real64 designFlow = connectionDesignFlow;
        if (designFlow <= DataBranchAirLoopPlant::MassFlowTolerance) {
            designFlow = legacyDesignFlow;
        }
        if (designFlow <= DataBranchAirLoopPlant::MassFlowTolerance) {
            designFlow = remainingFlow;
        }
        return min(max(0.0, remainingFlow), max(0.0, designFlow));
    };

    for (std::size_t moduleIndex = 0; moduleIndex < this->modules.size(); ++moduleIndex) {
        auto &module = this->modules[moduleIndex];
        auto const &sizing = module.sizing;
        bool const moduleIsAvailable = module.isAvailable();

        Real64 const moduleCoolingMassFlowRate =
            allocateConnectionFlow(remainingCoolingMassFlowRate, sizing.maximumCoolingMassFlowRate, sizing.maximumEvaporatorMassFlowRate);
        Real64 const moduleSourceMassFlowRate =
            allocateConnectionFlow(remainingSourceMassFlowRate, sizing.maximumSourceCondenserMassFlowRate, sizing.maximumCondenserMassFlowRate);
        ModuleResult result;
        if (moduleIsAvailable && remainingCoolingLoad > HVAC::SmallLoad && moduleCoolingMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance &&
            moduleSourceMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
            result = this->solveCoolingOnly(
                state, moduleIndex, remainingCoolingLoad, moduleCoolingMassFlowRate, moduleSourceMassFlowRate, coolingInletTemp, sourceInletTemp);
            module.result = result;
            module.mapResultToPlantConnections();
            result = module.result;
        } else {
            result.requestedCoolingLoad = remainingCoolingLoad;
            result.unmetCoolingLoad = remainingCoolingLoad;
            result.evaporatorInletTemp = coolingInletTemp;
            result.evaporatorOutletTemp = coolingInletTemp;
            result.condenserInletTemp = sourceInletTemp;
            result.condenserOutletTemp = sourceInletTemp;
            result.coolingInletTemp = coolingInletTemp;
            result.coolingOutletTemp = coolingInletTemp;
            result.sourceInletTemp = sourceInletTemp;
            result.sourceOutletTemp = sourceInletTemp;
        }

        result.isAvailable = moduleIsAvailable;
        remainingCoolingLoad = max(0.0, remainingCoolingLoad - result.coolingDelivered);
        result.unmetCoolingLoad = remainingCoolingLoad;
        remainingCoolingMassFlowRate = max(0.0, remainingCoolingMassFlowRate - result.coolingMassFlowRate);
        remainingSourceMassFlowRate = max(0.0, remainingSourceMassFlowRate - result.sourceMassFlowRate);
        module.result = result;
    }

    this->isCoolingDominant = false;
    this->isHeatingDominant = false;
    this->updateReportingAndNodes(state,
                                  coolingMassFlowRate,
                                  0.0,
                                  sourceMassFlowRate,
                                  coolingInletTemp,
                                  state.dataLoopNodes->Node(this->heatingInletNodeNum).Temp,
                                  sourceInletTemp);
}

void CentralHeatPumpSystem::calculateHeatingOnly(EnergyPlusData &state,
                                                 Real64 const heatingMassFlowRate,
                                                 Real64 const sourceMassFlowRate,
                                                 Real64 const heatingInletTemp,
                                                 Real64 const sourceInletTemp)
{
    Real64 remainingHeatingLoad = this->requestedHeatingLoad;
    Real64 remainingHeatingMassFlowRate = max(0.0, heatingMassFlowRate);
    Real64 remainingSourceMassFlowRate = max(0.0, sourceMassFlowRate);

    auto allocateConnectionFlow = [](Real64 const remainingFlow, Real64 const connectionDesignFlow, Real64 const legacyDesignFlow) {
        Real64 designFlow = connectionDesignFlow;
        if (designFlow <= DataBranchAirLoopPlant::MassFlowTolerance) {
            designFlow = legacyDesignFlow;
        }
        if (designFlow <= DataBranchAirLoopPlant::MassFlowTolerance) {
            designFlow = remainingFlow;
        }
        return min(max(0.0, remainingFlow), max(0.0, designFlow));
    };

    for (std::size_t moduleIndex = 0; moduleIndex < this->modules.size(); ++moduleIndex) {
        auto &module = this->modules[moduleIndex];
        auto const &sizing = module.sizing;
        bool const moduleIsAvailable = module.isAvailable();

        Real64 const moduleHeatingMassFlowRate =
            allocateConnectionFlow(remainingHeatingMassFlowRate, sizing.maximumHeatingMassFlowRate, sizing.maximumCondenserMassFlowRate);
        Real64 const moduleSourceMassFlowRate =
            allocateConnectionFlow(remainingSourceMassFlowRate, sizing.maximumSourceEvaporatorMassFlowRate, sizing.maximumEvaporatorMassFlowRate);
        ModuleResult result;
        if (moduleIsAvailable && remainingHeatingLoad > HVAC::SmallLoad && moduleHeatingMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance &&
            moduleSourceMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
            result = this->solveHeatingOnly(
                state, moduleIndex, remainingHeatingLoad, moduleSourceMassFlowRate, moduleHeatingMassFlowRate, sourceInletTemp, heatingInletTemp);
            module.result = result;
            module.mapResultToPlantConnections();
            result = module.result;
        } else {
            result.requestedHeatingLoad = remainingHeatingLoad;
            result.unmetHeatingLoad = remainingHeatingLoad;
            result.evaporatorInletTemp = sourceInletTemp;
            result.evaporatorOutletTemp = sourceInletTemp;
            result.condenserInletTemp = heatingInletTemp;
            result.condenserOutletTemp = heatingInletTemp;
            result.heatingInletTemp = heatingInletTemp;
            result.heatingOutletTemp = heatingInletTemp;
            result.sourceInletTemp = sourceInletTemp;
            result.sourceOutletTemp = sourceInletTemp;
        }

        result.isAvailable = moduleIsAvailable;
        remainingHeatingLoad = max(0.0, remainingHeatingLoad - result.heatingDelivered);
        result.unmetHeatingLoad = remainingHeatingLoad;
        remainingHeatingMassFlowRate = max(0.0, remainingHeatingMassFlowRate - result.heatingMassFlowRate);
        remainingSourceMassFlowRate = max(0.0, remainingSourceMassFlowRate - result.sourceMassFlowRate);
        module.result = result;
    }

    this->isCoolingDominant = false;
    this->isHeatingDominant = false;
    this->updateReportingAndNodes(state,
                                  0.0,
                                  heatingMassFlowRate,
                                  sourceMassFlowRate,
                                  state.dataLoopNodes->Node(this->coolingInletNodeNum).Temp,
                                  heatingInletTemp,
                                  sourceInletTemp);
}

void CentralHeatPumpSystem::updateReportingAndNodes(EnergyPlusData const &state,
                                                    Real64 const coolingMassFlowRate,
                                                    Real64 const heatingMassFlowRate,
                                                    Real64 const sourceMassFlowRate,
                                                    Real64 const coolingInletTemp,
                                                    Real64 const heatingInletTemp,
                                                    Real64 const sourceInletTemp)
{
    Real64 const secondsInTimeStep = state.dataHVACGlobal->TimeStepSysSec;
    Real64 totalCoolingHeatTransferRate = 0.0;
    Real64 totalHeatingHeatTransferRate = 0.0;
    Real64 totalSourceHeatTransfer = 0.0;
    Real64 totalCoolingPower = 0.0;
    Real64 totalHeatingPower = 0.0;
    Real64 usedCoolingMassFlowRate = 0.0;
    Real64 usedHeatingMassFlowRate = 0.0;
    Real64 usedSourceMassFlowRate = 0.0;
    Real64 coolingOutletTemperatureSum = 0.0;
    Real64 heatingOutletTemperatureSum = 0.0;
    Real64 sourceOutletTemperatureSum = 0.0;

    for (auto &module : this->modules) {
        module.updateResultEnergies(secondsInTimeStep);
        auto const &result = module.result;
        totalCoolingHeatTransferRate += result.coolingDelivered;
        totalHeatingHeatTransferRate += result.heatingDelivered;
        totalSourceHeatTransfer += result.sourceHeatTransfer;
        totalCoolingPower += result.coolingPower;
        totalHeatingPower += result.heatingPower;
        usedCoolingMassFlowRate += result.coolingMassFlowRate;
        usedHeatingMassFlowRate += result.heatingMassFlowRate;
        usedSourceMassFlowRate += result.sourceMassFlowRate;
        coolingOutletTemperatureSum += result.coolingOutletTemp * result.coolingMassFlowRate;
        heatingOutletTemperatureSum += result.heatingOutletTemp * result.heatingMassFlowRate;
        sourceOutletTemperatureSum += result.sourceOutletTemp * result.sourceMassFlowRate;
    }

    auto mixConnection =
        [](Real64 const totalMassFlowRate, Real64 const usedMassFlowRate, Real64 const outletTemperatureSum, Real64 const inletTemp) {
            if (totalMassFlowRate <= DataBranchAirLoopPlant::MassFlowTolerance || usedMassFlowRate <= DataBranchAirLoopPlant::MassFlowTolerance) {
                return inletTemp;
            }
            if (usedMassFlowRate > totalMassFlowRate) {
                return outletTemperatureSum / usedMassFlowRate;
            }
            Real64 const bypassMassFlowRate = totalMassFlowRate - usedMassFlowRate;
            return (outletTemperatureSum + bypassMassFlowRate * inletTemp) / totalMassFlowRate;
        };

    Real64 const coolingOutletTemp = mixConnection(coolingMassFlowRate, usedCoolingMassFlowRate, coolingOutletTemperatureSum, coolingInletTemp);
    Real64 const heatingOutletTemp = mixConnection(heatingMassFlowRate, usedHeatingMassFlowRate, heatingOutletTemperatureSum, heatingInletTemp);
    Real64 const sourceOutletTemp = mixConnection(sourceMassFlowRate, usedSourceMassFlowRate, sourceOutletTemperatureSum, sourceInletTemp);

    if (this->ancillaryPower > 0.0) {
        Real64 const scheduleValue = this->ancillaryPowerSched != nullptr ? this->ancillaryPowerSched->getCurrentVal() : 1.0;
        Real64 const availableAncillaryPower = this->ancillaryPower * scheduleValue;
        if ((totalHeatingHeatTransferRate > HVAC::SmallLoad && totalCoolingHeatTransferRate <= HVAC::SmallLoad) || this->isHeatingDominant) {
            totalHeatingPower += availableAncillaryPower;
        } else {
            totalCoolingPower += availableAncillaryPower;
        }
    }

    this->report.coolingInletTemp = coolingInletTemp;
    this->report.coolingOutletTemp = coolingOutletTemp;
    this->report.heatingInletTemp = heatingInletTemp;
    this->report.heatingOutletTemp = heatingOutletTemp;
    this->report.sourceInletTemp = sourceInletTemp;
    this->report.sourceOutletTemp = sourceOutletTemp;
    this->report.coolingMassFlowRate = coolingMassFlowRate;
    this->report.heatingMassFlowRate = heatingMassFlowRate;
    this->report.sourceMassFlowRate = sourceMassFlowRate;
    this->report.coolingElectricPower = totalCoolingPower;
    this->report.heatingElectricPower = totalHeatingPower;
    this->report.coolingHeatTransferRate = totalCoolingHeatTransferRate;
    this->report.heatingHeatTransferRate = totalHeatingHeatTransferRate;
    this->report.sourceHeatTransferRate = totalSourceHeatTransfer;
    this->report.coolingElectricEnergy = totalCoolingPower * secondsInTimeStep;
    this->report.heatingElectricEnergy = totalHeatingPower * secondsInTimeStep;
    this->report.coolingHeatTransferEnergy = totalCoolingHeatTransferRate * secondsInTimeStep;
    this->report.heatingHeatTransferEnergy = totalHeatingHeatTransferRate * secondsInTimeStep;
    this->report.sourceHeatTransferEnergy = totalSourceHeatTransfer * secondsInTimeStep;

    state.dataLoopNodes->Node(this->coolingOutletNodeNum).Temp = coolingOutletTemp;
    state.dataLoopNodes->Node(this->heatingOutletNodeNum).Temp = heatingOutletTemp;
    state.dataLoopNodes->Node(this->sourceOutletNodeNum).Temp = sourceOutletTemp;
}

void CentralHeatPumpSystem::calculateSimultaneous(EnergyPlusData &state,
                                                  Real64 const coolingMassFlowRate,
                                                  Real64 const heatingMassFlowRate,
                                                  Real64 const sourceMassFlowRate,
                                                  Real64 const coolingInletTemp,
                                                  Real64 const heatingInletTemp,
                                                  Real64 const sourceInletTemp)
{
    Real64 remainingCoolingLoad = this->requestedCoolingLoad;
    Real64 remainingHeatingLoad = this->requestedHeatingLoad;
    Real64 remainingCoolingMassFlowRate = max(0.0, coolingMassFlowRate);
    Real64 remainingHeatingMassFlowRate = max(0.0, heatingMassFlowRate);
    Real64 remainingSourceMassFlowRate = max(0.0, sourceMassFlowRate);

    for (std::size_t moduleIndex = 0; moduleIndex < this->modules.size(); ++moduleIndex) {
        auto &module = this->modules[moduleIndex];
        auto const &sizing = module.sizing;
        bool const moduleIsAvailable = module.isAvailable();

        auto moduleFlowLimit = [](Real64 const remainingFlow, Real64 const connectionDesignFlow, Real64 const legacyDesignFlow) {
            Real64 flowLimit = connectionDesignFlow;
            if (flowLimit <= DataBranchAirLoopPlant::MassFlowTolerance) {
                flowLimit = legacyDesignFlow;
            }
            if (flowLimit <= DataBranchAirLoopPlant::MassFlowTolerance) {
                flowLimit = remainingFlow;
            }
            return min(remainingFlow, max(0.0, flowLimit));
        };

        Real64 const moduleCoolingMassFlowRate =
            moduleFlowLimit(remainingCoolingMassFlowRate, sizing.maximumCoolingMassFlowRate, sizing.maximumEvaporatorMassFlowRate);
        Real64 const moduleHeatingMassFlowRate =
            moduleFlowLimit(remainingHeatingMassFlowRate, sizing.maximumHeatingMassFlowRate, sizing.maximumCondenserMassFlowRate);
        Real64 moduleSourceMassFlowRate = 0.0;
        ModuleResult result;

        if (moduleIsAvailable && remainingCoolingLoad > HVAC::SmallLoad && remainingHeatingLoad > HVAC::SmallLoad &&
            moduleCoolingMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance &&
            moduleHeatingMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
            moduleSourceMassFlowRate = moduleFlowLimit(remainingSourceMassFlowRate,
                                                       max(sizing.maximumSourceEvaporatorMassFlowRate, sizing.maximumSourceCondenserMassFlowRate),
                                                       max(sizing.maximumEvaporatorMassFlowRate, sizing.maximumCondenserMassFlowRate));
            result = this->solveSimultaneous(state,
                                             moduleIndex,
                                             remainingCoolingLoad,
                                             remainingHeatingLoad,
                                             moduleCoolingMassFlowRate,
                                             moduleHeatingMassFlowRate,
                                             moduleSourceMassFlowRate,
                                             coolingInletTemp,
                                             heatingInletTemp,
                                             sourceInletTemp);
        } else if (moduleIsAvailable && remainingCoolingLoad > HVAC::SmallLoad &&
                   moduleCoolingMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
            moduleSourceMassFlowRate =
                moduleFlowLimit(remainingSourceMassFlowRate, sizing.maximumSourceCondenserMassFlowRate, sizing.maximumCondenserMassFlowRate);
            result = this->solveCoolingOnly(
                state, moduleIndex, remainingCoolingLoad, moduleCoolingMassFlowRate, moduleSourceMassFlowRate, coolingInletTemp, sourceInletTemp);
            module.result = result;
            module.mapResultToPlantConnections();
            result = module.result;
            if (result.currentMode == CurrentMode::CoolingOnly) {
                result.currentMode = CurrentMode::CoolingDominant;
            }
            result.requestedHeatingLoad = remainingHeatingLoad;
            result.unmetHeatingLoad = remainingHeatingLoad;
        } else if (moduleIsAvailable && remainingHeatingLoad > HVAC::SmallLoad &&
                   moduleHeatingMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
            moduleSourceMassFlowRate =
                moduleFlowLimit(remainingSourceMassFlowRate, sizing.maximumSourceEvaporatorMassFlowRate, sizing.maximumEvaporatorMassFlowRate);
            result = this->solveHeatingOnly(
                state, moduleIndex, remainingHeatingLoad, moduleSourceMassFlowRate, moduleHeatingMassFlowRate, sourceInletTemp, heatingInletTemp);
            module.result = result;
            module.mapResultToPlantConnections();
            result = module.result;
            if (result.currentMode == CurrentMode::HeatingOnly) {
                result.currentMode = CurrentMode::HeatingDominant;
            }
            result.requestedCoolingLoad = remainingCoolingLoad;
            result.unmetCoolingLoad = remainingCoolingLoad;
        } else {
            result.requestedCoolingLoad = remainingCoolingLoad;
            result.requestedHeatingLoad = remainingHeatingLoad;
            result.unmetCoolingLoad = remainingCoolingLoad;
            result.unmetHeatingLoad = remainingHeatingLoad;
            result.coolingInletTemp = coolingInletTemp;
            result.coolingOutletTemp = coolingInletTemp;
            result.heatingInletTemp = heatingInletTemp;
            result.heatingOutletTemp = heatingInletTemp;
            result.sourceInletTemp = sourceInletTemp;
            result.sourceOutletTemp = sourceInletTemp;
        }

        result.isAvailable = moduleIsAvailable;
        remainingCoolingLoad = max(0.0, remainingCoolingLoad - result.coolingDelivered);
        remainingHeatingLoad = max(0.0, remainingHeatingLoad - result.heatingDelivered);
        result.unmetCoolingLoad = remainingCoolingLoad;
        result.unmetHeatingLoad = remainingHeatingLoad;
        remainingCoolingMassFlowRate = max(0.0, remainingCoolingMassFlowRate - result.coolingMassFlowRate);
        remainingHeatingMassFlowRate = max(0.0, remainingHeatingMassFlowRate - result.heatingMassFlowRate);
        remainingSourceMassFlowRate = max(0.0, remainingSourceMassFlowRate - result.sourceMassFlowRate);

        module.result = result;
    }

    Real64 totalCoolingHeatTransferRate = 0.0;
    Real64 totalHeatingHeatTransferRate = 0.0;
    Real64 totalSourceHeatTransfer = 0.0;
    for (auto const &module : this->modules) {
        totalCoolingHeatTransferRate += module.result.coolingDelivered;
        totalHeatingHeatTransferRate += module.result.heatingDelivered;
        totalSourceHeatTransfer += module.result.sourceHeatTransfer;
    }
    Real64 const sourceModeTolerance =
        max(HVAC::SmallLoad, 1.0e-8 * max({totalCoolingHeatTransferRate, totalHeatingHeatTransferRate, std::abs(totalSourceHeatTransfer)}));
    this->isCoolingDominant = totalSourceHeatTransfer > sourceModeTolerance;
    this->isHeatingDominant = totalSourceHeatTransfer < -sourceModeTolerance;
    this->updateReportingAndNodes(
        state, coolingMassFlowRate, heatingMassFlowRate, sourceMassFlowRate, coolingInletTemp, heatingInletTemp, sourceInletTemp);
}

void CentralHeatPumpSystem::calculate(EnergyPlusData &state, Real64 &load, int const loopNum)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Daeho Kang, PNNL
    //       DATE WRITTEN   Feb 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //  Calculate node information connected to plant & condenser loop

    // METHODOLOGY EMPLOYED:
    //  Use empirical curve fits to model performance at off-reference conditions

    Real64 currentHeatingLoad = 0.0; // Total heating load chiller heater bank (system) meets

    // Chiller heater bank chilled water inlet mass flow rate
    Real64 coolingInletMassFlowRate = 0.0;

    Real64 heatingInletMassFlowRate = 0.0;
    Real64 sourceInletMassFlowRate = 0.0;
    Real64 coolingInletTemp = state.dataLoopNodes->Node(this->coolingInletNodeNum).Temp;

    // Chiller heater bank hot water inlet temperature
    Real64 heatingInletTemp = state.dataLoopNodes->Node(this->heatingInletNodeNum).Temp;

    // Chiller heater bank condenser loop inlet temperature
    Real64 sourceInletTemp = state.dataLoopNodes->Node(this->sourceInletNodeNum).Temp;

    Real64 currentCoolingLoad = 0.0; // Total cooling load chiller heater bank (system) meets

    // Initiate loads and inlet temperatures each loop
    if (loopNum == this->coolingPlantLoc.loopNum) {
        coolingInletMassFlowRate = state.dataLoopNodes->Node(this->coolingInletNodeNum).MassFlowRate;
        heatingInletMassFlowRate = state.dataLoopNodes->Node(this->heatingInletNodeNum).MassFlowRate;
        sourceInletMassFlowRate = state.dataLoopNodes->Node(this->sourceInletNodeNum).MassFlowRate;
        this->requestedCoolingLoad = 0.0;
        currentCoolingLoad = std::abs(load);
        this->requestedCoolingLoad = currentCoolingLoad;
        if (coolingInletMassFlowRate == 0.0) {
            sourceInletMassFlowRate = 0.0;
        }

    } else if (loopNum == this->heatingPlantLoc.loopNum) {
        coolingInletMassFlowRate = state.dataLoopNodes->Node(this->coolingInletNodeNum).MassFlowRate;
        heatingInletMassFlowRate = state.dataLoopNodes->Node(this->heatingInletNodeNum).MassFlowRate;
        sourceInletMassFlowRate = state.dataLoopNodes->Node(this->sourceInletNodeNum).MassFlowRate;
        this->requestedHeatingLoad = 0.0;
        currentHeatingLoad = load;
        this->requestedHeatingLoad = currentHeatingLoad;
        if (heatingInletMassFlowRate == 0.0) {
            sourceInletMassFlowRate = 0.0;
        }
    }

    if (this->requestedCoolingLoad > HVAC::SmallLoad && this->requestedHeatingLoad > HVAC::SmallLoad) {
        coolingInletMassFlowRate = state.dataLoopNodes->Node(this->coolingInletNodeNum).MassFlowRate;
        heatingInletMassFlowRate = state.dataLoopNodes->Node(this->heatingInletNodeNum).MassFlowRate;
        sourceInletMassFlowRate = state.dataLoopNodes->Node(this->sourceInletNodeNum).MassFlowRate;

        this->calculateSimultaneous(
            state, coolingInletMassFlowRate, heatingInletMassFlowRate, sourceInletMassFlowRate, coolingInletTemp, heatingInletTemp, sourceInletTemp);

        PlantUtilities::SetComponentFlowRate(
            state, coolingInletMassFlowRate, this->coolingInletNodeNum, this->coolingOutletNodeNum, this->coolingPlantLoc);
        PlantUtilities::SetComponentFlowRate(
            state, heatingInletMassFlowRate, this->heatingInletNodeNum, this->heatingOutletNodeNum, this->heatingPlantLoc);
        PlantUtilities::SetComponentFlowRate(
            state, sourceInletMassFlowRate, this->sourceInletNodeNum, this->sourceOutletNodeNum, this->sourcePlantLoc);

        load = loopNum == this->coolingPlantLoc.loopNum ? -this->report.coolingHeatTransferRate : this->report.heatingHeatTransferRate;
        return;
    }

    this->isCoolingDominant = false;
    this->isHeatingDominant = false;

    if (loopNum == this->coolingPlantLoc.loopNum && currentCoolingLoad > HVAC::SmallLoad) {
        this->calculateCoolingOnly(state, coolingInletMassFlowRate, sourceInletMassFlowRate, coolingInletTemp, sourceInletTemp);
        PlantUtilities::SetComponentFlowRate(
            state, coolingInletMassFlowRate, this->coolingInletNodeNum, this->coolingOutletNodeNum, this->coolingPlantLoc);
        PlantUtilities::SetComponentFlowRate(
            state, heatingInletMassFlowRate, this->heatingInletNodeNum, this->heatingOutletNodeNum, this->heatingPlantLoc);
        PlantUtilities::SetComponentFlowRate(
            state, sourceInletMassFlowRate, this->sourceInletNodeNum, this->sourceOutletNodeNum, this->sourcePlantLoc);
        load = -this->report.coolingHeatTransferRate;
        return;
    }

    if (loopNum == this->heatingPlantLoc.loopNum && currentHeatingLoad > HVAC::SmallLoad) {
        this->calculateHeatingOnly(state, heatingInletMassFlowRate, sourceInletMassFlowRate, heatingInletTemp, sourceInletTemp);
        PlantUtilities::SetComponentFlowRate(
            state, coolingInletMassFlowRate, this->coolingInletNodeNum, this->coolingOutletNodeNum, this->coolingPlantLoc);
        PlantUtilities::SetComponentFlowRate(
            state, heatingInletMassFlowRate, this->heatingInletNodeNum, this->heatingOutletNodeNum, this->heatingPlantLoc);
        PlantUtilities::SetComponentFlowRate(
            state, sourceInletMassFlowRate, this->sourceInletNodeNum, this->sourceOutletNodeNum, this->sourcePlantLoc);
        load = this->report.heatingHeatTransferRate;
        return;
    }

    this->resetOffState(state);
    load = 0.0;
}

void CentralHeatPumpSystem::oneTimeInit([[maybe_unused]] EnergyPlusData &state)
{
}

} // namespace EnergyPlus::PlantCentralHeatPumpSystem
