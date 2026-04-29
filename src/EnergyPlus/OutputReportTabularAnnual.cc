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
#include <list>
#include <ostream>
#include <string>
#include <vector>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Array2S.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportData.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/OutputReportTabularAnnual.hh>
#include <EnergyPlus/ResultsFramework.hh>
#include <EnergyPlus/SQLiteProcedures.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::OutputReportTabularAnnual {

constexpr Real64 veryLarge = std::numeric_limits<Real64>::max();
constexpr Real64 verySmall = std::numeric_limits<Real64>::lowest();

void GetInputTabularAnnual(EnergyPlusData &state)
{
    // Jason Glazer, August 2015
    // The function assigns the input information for
    // REPORT:TABLE:ANNUAL also known as row per object
    // reports that are defined by the user. The input
    // information is assigned to a data structure that
    // is used for both user defined monthly reports and
    // predefined monthly reports.

    static std::string const currentModuleObject("Output:Table:Annual");

    int numParams;            // Number of elements combined
    int numAlphas;            // Number of elements in the alpha array
    int numNums;              // Number of elements in the numeric array
    Array1D_string alphArray; // character string data
    Array1D<Real64> numArray; // numeric data
    int IOStat;               // IO Status when calling get input subroutine
    int objCount(0);
    AnnualFieldSet::AggregationKind curAgg(AnnualFieldSet::AggregationKind::sumOrAvg);

    auto &annualTables = state.dataOutputReportTabularAnnual->annualTables;

    objCount = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, currentModuleObject);
    if (objCount > 0) {

        state.dataOutRptTab->WriteTabularFiles = true;

        // if not a run period using weather do not create reports
        if (!state.dataGlobal->DoWeathSim) {
            ShowWarningError(
                state,
                EnergyPlus::format(
                    "{} requested with SimulationControl Run Simulation for Weather File Run Periods set to No so {} will not be generated",
                    currentModuleObject,
                    currentModuleObject));
            return;
        }
    }
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, currentModuleObject, numParams, numAlphas, numNums);
    alphArray.allocate(numAlphas);
    numArray.dimension(numNums, 0.0);
    for (int tabNum = 1; tabNum <= objCount; ++tabNum) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state, currentModuleObject, tabNum, alphArray, numAlphas, numArray, numNums, IOStat);
        if (numAlphas >= 5) {
            annualTables.emplace_back(state, alphArray(1), alphArray(2), alphArray(3));
            // the remaining fields are repeating in groups of three and need to be added to the data structure
            for (int jAlpha = 4; jAlpha <= numAlphas; jAlpha += 2) {
                std::string curVarMtr = alphArray(jAlpha);
                if (curVarMtr.empty()) {
                    ShowWarningError(
                        state,
                        EnergyPlus::format("{}: Blank column specified in '{}', need to provide a variable or meter or EMS variable name ",
                                           currentModuleObject,
                                           alphArray(1)));
                }
                if (jAlpha <= numAlphas) {
                    const std::string &aggregationString = alphArray(jAlpha + 1);
                    curAgg = stringToAggKind(state, aggregationString);
                } else {
                    curAgg = AnnualFieldSet::AggregationKind::sumOrAvg; // if missing aggregation type use SumOrAverage
                }
                const int indexNums = 1 + (jAlpha - 3) / 2; // compute the corresponding field index in the numArray
                int curNumDgts;
                if (indexNums <= numNums) {
                    curNumDgts = numArray(indexNums);
                } else {
                    curNumDgts = 2;
                }
                if (!curVarMtr.empty()) {
                    annualTables.back().addFieldSet(curVarMtr, curAgg, curNumDgts);
                }
            }
            annualTables.back().setupGathering(state);
        } else {
            ShowSevereError(state, EnergyPlus::format("{}: Must enter at least the first six fields.", currentModuleObject));
        }
    }
}

void AnnualTable::addFieldSet(const std::string &varName, AnnualFieldSet::AggregationKind aggKind, int dgts)
// Jason Glazer, August 2015
// This method is used along with the constructor to convert the GetInput for REPORT:TABLE:ANNUAL
// into the class data.
{
    m_annualFields.emplace_back(varName, aggKind, dgts);
    m_annualFields.back().m_colHead = varName; // use the variable name for the column heading
}

void AnnualTable::addFieldSet(const std::string &varName, const std::string &colName, AnnualFieldSet::AggregationKind aggKind, int dgts)
// Jason Glazer, August 2015
// This overloaded method allows for a specific column name to be different than the output variable or meter name
{
    m_annualFields.emplace_back(varName, aggKind, dgts);
    m_annualFields.back().m_colHead = colName; // use the user supplied column heading instead of just the variable name
}

void AnnualTable::setupGathering(EnergyPlusData &state)
{
    // Used after GetInput for REPORT:TABLE:ANNUAL to set up how output variables, meters,
    // input fields, and ems variables are gathered.

    OutputProcessor::VariableType typeVar = OutputProcessor::VariableType::Invalid;
    OutputProcessor::StoreType avgSumVar;
    OutputProcessor::TimeStepType stepTypeVar;
    Constant::Units unitsVar = Constant::Units::None;
    std::list<std::string> allKeys;

    std::string filterFieldUpper = m_filter;
    std::transform(filterFieldUpper.begin(), filterFieldUpper.end(), filterFieldUpper.begin(), ::toupper);
    const bool useFilter = !m_filter.empty();

    for (auto &fldSt : m_annualFields) {
        const int keyCount = fldSt.getVariableKeyCountandTypeFromFldSt(state, typeVar, avgSumVar, stepTypeVar, unitsVar);
        fldSt.getVariableKeysFromFldSt(state, typeVar, keyCount, fldSt.m_namesOfKeys, fldSt.m_indexesForKeyVar);
        for (const auto &nm : fldSt.m_namesOfKeys) {
            std::string nmUpper = nm;
            std::transform(nmUpper.begin(), nmUpper.end(), nmUpper.begin(), ::toupper);
            if (!useFilter || nmUpper.find(filterFieldUpper) != std::string::npos) {
                allKeys.push_back(nm);
            }
        }
        fldSt.m_typeOfVar = typeVar;
        fldSt.m_varAvgSum = avgSumVar;
        fldSt.m_varStepType = stepTypeVar;
        fldSt.m_varUnits = unitsVar;
        fldSt.m_keyCount = keyCount;
    }
    allKeys.sort();
    allKeys.unique();
    m_objectNames.clear();
    std::copy(allKeys.begin(), allKeys.end(), std::back_inserter(m_objectNames));
    // Size each field set's cell array for the number of object names (rows)
    for (auto &fldSt : m_annualFields) {
        fldSt.m_cell.resize(m_objectNames.size());
    }
    // Populate per-row/per-field cell metadata
    int tableRowIndex = 0;
    for (const auto &objName : m_objectNames) {
        for (auto &fldSt : m_annualFields) {
            int foundKeyIndex = -1;
            for (std::size_t i = 0; i < fldSt.m_namesOfKeys.size(); ++i) {
                if (fldSt.m_namesOfKeys[i] == objName) {
                    foundKeyIndex = static_cast<int>(i);
                    break;
                }
            }
            fldSt.m_cell[tableRowIndex].indexesForKeyVar = (foundKeyIndex >= 0) ? fldSt.m_indexesForKeyVar[foundKeyIndex] : -1;
            // Initialize result based on aggregation kind
            switch (fldSt.m_aggregate) {
            case AnnualFieldSet::AggregationKind::maximum:
            case AnnualFieldSet::AggregationKind::maximumDuringHoursShown:
                fldSt.m_cell[tableRowIndex].result = verySmall;
                break;
            case AnnualFieldSet::AggregationKind::minimum:
            case AnnualFieldSet::AggregationKind::minimumDuringHoursShown:
                fldSt.m_cell[tableRowIndex].result = veryLarge;
                break;
            default:
                fldSt.m_cell[tableRowIndex].result = 0.0;
                break;
            }
            fldSt.m_cell[tableRowIndex].duration = 0.0;
            fldSt.m_cell[tableRowIndex].timeStamp = 0;
        }
        ++tableRowIndex;
    }
}

void checkAggregationOrderForAnnual(EnergyPlusData &state)
{
    bool invalidAggregationOrderFound = false;
    auto &annualTables = state.dataOutputReportTabularAnnual->annualTables;
    if (!state.dataGlobal->DoWeathSim) { // if no weather simulation than no reading of MonthlyInput array
        return;
    }
    for (auto &annualTable : annualTables) {
        if (annualTable.invalidAggregationOrder(state)) {
            invalidAggregationOrderFound = true;
        }
    }
    if (invalidAggregationOrderFound) {
        ShowFatalError(state, "OutputReportTabularAnnual: Invalid aggregations detected, no simulation performed.");
    }
}

// Generate an error message if an advanced aggregation kind columns don't follow the appropriate column - Glazer 2017
bool AnnualTable::invalidAggregationOrder(EnergyPlusData &state)
{
    bool foundMinOrMax = false;
    bool foundHourAgg = false;
    bool missingMaxOrMinError = false;
    bool missingHourAggError = false;
    for (auto const &fldSt : m_annualFields) {
        switch (fldSt.m_aggregate) {
        case AnnualFieldSet::AggregationKind::maximum:
        case AnnualFieldSet::AggregationKind::minimum:
            foundMinOrMax = true;
            break;
        case AnnualFieldSet::AggregationKind::hoursNonZero:
        case AnnualFieldSet::AggregationKind::hoursZero:
        case AnnualFieldSet::AggregationKind::hoursPositive:
        case AnnualFieldSet::AggregationKind::hoursNonPositive:
        case AnnualFieldSet::AggregationKind::hoursNegative:
        case AnnualFieldSet::AggregationKind::hoursNonNegative:
            foundHourAgg = true;
            break;
        case AnnualFieldSet::AggregationKind::valueWhenMaxMin:
            if (!foundMinOrMax) {
                missingMaxOrMinError = true;
            }
            break;
        case AnnualFieldSet::AggregationKind::sumOrAverageHoursShown:
        case AnnualFieldSet::AggregationKind::maximumDuringHoursShown:
        case AnnualFieldSet::AggregationKind::minimumDuringHoursShown:
            if (!foundHourAgg) {
                missingHourAggError = true;
            }
            break;
        default:
            break;
        }
    }
    if (missingMaxOrMinError) {
        ShowSevereError(
            state,
            EnergyPlus::format("The Output:Table:Annual report named=\"{}\" has a valueWhenMaxMin aggregation type for a column without a previous "
                               "column that uses either the minimum or maximum aggregation types. The report will not be generated.",
                               m_name));
    }
    if (missingHourAggError) {
        ShowSevereError(
            state,
            EnergyPlus::format("The Output:Table:Annual report named=\"{}\" has a --DuringHoursShown aggregation type for a column without a "
                               "previous field that uses one of the Hour-- aggregation types. The report will not be generated.",
                               m_name));
    }
    return (missingHourAggError || missingMaxOrMinError);
}

void GatherAnnualResultsForTimeStep(EnergyPlusData &state, const OutputProcessor::TimeStepType kindOfTimeStep)
{
    // Jason Glazer, August 2015
    // This function is not part of the class but acts as an interface between procedural code and the class by
    // gathering data for each of the AnnualTable objects
    auto &annualTables = state.dataOutputReportTabularAnnual->annualTables;
    for (auto &annualTable : annualTables) {
        annualTable.gatherForTimestep(state, kindOfTimeStep);
    }
}

void AnnualTable::gatherForTimestep(EnergyPlusData &state, OutputProcessor::TimeStepType kindOfTimeStep)
{
    // Jason Glazer, August 2015
    // For each cell of the table, gather the value as indicated by the type of aggregation

    int timestepTimeStamp;
    Real64 elapsedTime = AnnualTable::getElapsedTime(state, kindOfTimeStep);
    Real64 secondsInTimeStep = AnnualTable::getSecondsInTimeStep(state, kindOfTimeStep);
    bool activeMinMax = false;
    bool activeHoursShown = false;
    // if schedule is used and the current value is zero, don't gather values
    if (m_sched != nullptr && m_sched->getCurrentVal() == 0.0) {
        return;
    }
    // loop through the fields
    std::vector<AnnualFieldSet>::iterator fldStIt;
    std::vector<AnnualFieldSet>::iterator fldStRemainIt;
    for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
        for (fldStIt = m_annualFields.begin(); fldStIt != m_annualFields.end(); ++fldStIt) {
            OutputProcessor::VariableType curTypeOfVar = fldStIt->m_typeOfVar;
            OutputProcessor::TimeStepType curStepType = fldStIt->m_varStepType;
            if (curStepType == kindOfTimeStep) // this is a much simpler conditional than the code in monthly gathering
            {
                int curVarNum = fldStIt->m_cell[row].indexesForKeyVar;
                if (curVarNum > -1) {
                    Real64 curValue = GetInternalVariableValue(state, curTypeOfVar, curVarNum);
                    // Get the value from the result array
                    Real64 oldResultValue = fldStIt->m_cell[row].result;
                    Real64 oldDuration = fldStIt->m_cell[row].duration;
                    // Zero the revised values (as default if not set later)
                    Real64 newResultValue = 0.0;
                    int newTimeStamp = 0;
                    Real64 newDuration = 0.0;
                    bool activeNewValue = false;
                    // the current timestamp
                    int minuteCalculated = OutputProcessor::DetermineMinuteForReporting(state);
                    General::EncodeMonDayHrMin(
                        timestepTimeStamp, state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth, state.dataGlobal->HourOfDay, minuteCalculated);
                    // perform the selected aggregation type
                    // the following types of aggregations are not gathered at this point:
                    // noAggregation, valueWhenMaxMin, sumOrAverageHoursShown,     maximumDuringHoursShown, minimumDuringHoursShown:
                    switch (fldStIt->m_aggregate) {
                    case AnnualFieldSet::AggregationKind::sumOrAvg:
                        if (fldStIt->m_varAvgSum == OutputProcessor::StoreType::Sum) { // if it is a summed variable
                            newResultValue = oldResultValue + curValue;
                        } else {
                            newResultValue = oldResultValue + curValue * elapsedTime; // for averaging - weight by elapsed time
                        }
                        newDuration = oldDuration + elapsedTime;
                        activeNewValue = true;
                        break;
                    case AnnualFieldSet::AggregationKind::maximum:
                        // per MJW when a summed variable is used divide it by the length of the time step
                        if (fldStIt->m_varAvgSum == OutputProcessor::StoreType::Sum) { // if it is a summed variable
                            curValue /= secondsInTimeStep;
                        }
                        if (curValue > oldResultValue) {
                            newResultValue = curValue;
                            newTimeStamp = timestepTimeStamp;
                            activeMinMax = true;
                            activeNewValue = true;
                        } else {
                            activeMinMax = false; // reset this
                        }
                        break;
                    case AnnualFieldSet::AggregationKind::minimum:
                        // per MJW when a summed variable is used divide it by the length of the time step
                        if (fldStIt->m_varAvgSum == OutputProcessor::StoreType::Sum) { // if it is a summed variable
                            curValue /= secondsInTimeStep;
                        }
                        if (curValue < oldResultValue) {
                            newResultValue = curValue;
                            newTimeStamp = timestepTimeStamp;
                            activeMinMax = true;
                            activeNewValue = true;
                        } else {
                            activeMinMax = false; // reset this
                        }
                        break;
                    case AnnualFieldSet::AggregationKind::hoursNonZero:
                        if (curValue != 0) {
                            newResultValue = oldResultValue + elapsedTime;
                            activeHoursShown = true;
                            activeNewValue = true;
                        } else {
                            activeHoursShown = false;
                        }
                        break;
                    case AnnualFieldSet::AggregationKind::hoursZero:
                        if (curValue == 0) {
                            newResultValue = oldResultValue + elapsedTime;
                            activeHoursShown = true;
                            activeNewValue = true;
                        } else {
                            activeHoursShown = false;
                        }
                        break;
                    case AnnualFieldSet::AggregationKind::hoursPositive:
                        if (curValue > 0) {
                            newResultValue = oldResultValue + elapsedTime;
                            activeHoursShown = true;
                            activeNewValue = true;
                        } else {
                            activeHoursShown = false;
                        }
                        break;
                    case AnnualFieldSet::AggregationKind::hoursNonPositive:
                        if (curValue <= 0) {
                            newResultValue = oldResultValue + elapsedTime;
                            activeHoursShown = true;
                            activeNewValue = true;
                        } else {
                            activeHoursShown = false;
                        }
                        break;
                    case AnnualFieldSet::AggregationKind::hoursNegative:
                        if (curValue < 0) {
                            newResultValue = oldResultValue + elapsedTime;
                            activeHoursShown = true;
                            activeNewValue = true;
                        } else {
                            activeHoursShown = false;
                        }
                        break;
                    case AnnualFieldSet::AggregationKind::hoursNonNegative:
                        if (curValue >= 0) {
                            newResultValue = oldResultValue + elapsedTime;
                            activeHoursShown = true;
                            activeNewValue = true;
                        } else {
                            activeHoursShown = false;
                        }
                        break;
                    case AnnualFieldSet::AggregationKind::hoursInTenPercentBins:
                    case AnnualFieldSet::AggregationKind::hoursInTenBinsMinToMax:
                    case AnnualFieldSet::AggregationKind::hoursInTenBinsZeroToMax:
                    case AnnualFieldSet::AggregationKind::hoursInTenBinsMinToZero:
                    case AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusTwoStdDev:
                    case AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusThreeStdDev:
                        //  for all of the binning options add the value to the deferred
                        if (fldStIt->m_varAvgSum == OutputProcessor::StoreType::Sum) { // if it is a summed variable
                            const Real64 curValueRate = curValue / secondsInTimeStep;  // divide by time just like max and min
                            fldStIt->m_cell[row].deferredResults.push_back(curValueRate);
                        } else {
                            fldStIt->m_cell[row].deferredResults.push_back(curValue);
                        }
                        fldStIt->m_cell[row].deferredElapsed.push_back(elapsedTime); // save the amount of time for this particular value
                        // newDuration = oldDuration + elapsedTime;
                        break;
                    case AnnualFieldSet::AggregationKind::noAggregation:
                    case AnnualFieldSet::AggregationKind::valueWhenMaxMin:
                    case AnnualFieldSet::AggregationKind::sumOrAverageHoursShown:
                    case AnnualFieldSet::AggregationKind::maximumDuringHoursShown:
                    case AnnualFieldSet::AggregationKind::minimumDuringHoursShown:
                        // do nothing
                        break;
                    } // end switch fldStIt->m_aggregate

                    // if the new value has been set then set the monthly values to the
                    // new columns. This skips the aggregation types that don't even get
                    // triggered now such as valueWhenMinMax and all the agg*HoursShown
                    if (activeNewValue) {
                        fldStIt->m_cell[row].result = newResultValue;
                        fldStIt->m_cell[row].timeStamp = newTimeStamp;
                        fldStIt->m_cell[row].duration = newDuration;
                    }
                    // if a minimum or maximum value was set this timeStep then
                    // scan the remaining columns of the table looking for values
                    // that are aggregation type "ValueWhenMaxMin" and set their values
                    // if another minimum or maximum column is found then end
                    // the scan (it will be taken care of when that column is done)
                    if (activeMinMax) {
                        for (fldStRemainIt = fldStIt + 1; fldStRemainIt != m_annualFields.end(); ++fldStRemainIt) {
                            if (fldStRemainIt->m_aggregate == AnnualFieldSet::AggregationKind::maximum ||
                                fldStRemainIt->m_aggregate == AnnualFieldSet::AggregationKind::minimum) {
                                // end scanning since these might reset
                                break; // for fldStRemainIt
                            }
                            if (fldStRemainIt->m_aggregate == AnnualFieldSet::AggregationKind::valueWhenMaxMin) {
                                // this case is when the value should be set
                                OutputProcessor::VariableType scanTypeOfVar = fldStRemainIt->m_typeOfVar;
                                // int scanStepType = fldStRemainIt->m_varStepType;
                                int scanVarNum = fldStRemainIt->m_cell[row].indexesForKeyVar;
                                if (scanVarNum > -1) {
                                    Real64 scanValue = GetInternalVariableValue(state, scanTypeOfVar, scanVarNum);
                                    // When a summed variable is used divide it by the length of the time step
                                    if (fldStRemainIt->m_varAvgSum == OutputProcessor::StoreType::Sum) { // if it is a summed variable
                                        scanValue /= secondsInTimeStep;
                                    }
                                    fldStRemainIt->m_cell[row].result = scanValue;
                                }
                            } else {
                                // do nothing
                            }
                        }
                    }
                    // If the hours variable is active then scan through the rest of the variables
                    // and accumulate
                    if (activeHoursShown) {
                        for (fldStRemainIt = fldStIt + 1; fldStRemainIt != m_annualFields.end(); ++fldStRemainIt) {
                            OutputProcessor::VariableType scanTypeOfVar = fldStRemainIt->m_typeOfVar;
                            int scanVarNum = fldStRemainIt->m_cell[row].indexesForKeyVar;
                            Real64 oldScanValue = fldStRemainIt->m_cell[row].result;
                            if (scanVarNum > -1) {
                                Real64 scanValue = GetInternalVariableValue(state, scanTypeOfVar, scanVarNum);
                                if (fldStRemainIt->m_aggregate == AnnualFieldSet::AggregationKind::hoursZero ||
                                    fldStRemainIt->m_aggregate == AnnualFieldSet::AggregationKind::hoursNonZero ||
                                    fldStRemainIt->m_aggregate == AnnualFieldSet::AggregationKind::hoursPositive ||
                                    fldStRemainIt->m_aggregate == AnnualFieldSet::AggregationKind::hoursNonPositive ||
                                    fldStRemainIt->m_aggregate == AnnualFieldSet::AggregationKind::hoursNegative ||
                                    fldStRemainIt->m_aggregate == AnnualFieldSet::AggregationKind::hoursNonNegative) {
                                    // end scanning since these might reset
                                    break; // for fldStRemainIt
                                }
                                if (fldStRemainIt->m_aggregate == AnnualFieldSet::AggregationKind::sumOrAverageHoursShown) {
                                    if (fldStIt->m_varAvgSum == OutputProcessor::StoreType::Sum) { // if it is a summed variable
                                        fldStRemainIt->m_cell[row].result = oldScanValue + scanValue;
                                    } else {
                                        fldStRemainIt->m_cell[row].result =
                                            oldScanValue + scanValue * elapsedTime; // for averaging - weight by elapsed time
                                    }
                                    fldStRemainIt->m_cell[row].duration += elapsedTime;
                                } else if (fldStRemainIt->m_aggregate == AnnualFieldSet::AggregationKind::minimumDuringHoursShown) {
                                    if (fldStRemainIt->m_varAvgSum == OutputProcessor::StoreType::Sum) { // if it is a summed variable
                                        scanValue /= secondsInTimeStep;
                                    }
                                    if (scanValue < oldScanValue) {
                                        fldStRemainIt->m_cell[row].result = scanValue;
                                        fldStRemainIt->m_cell[row].timeStamp = timestepTimeStamp;
                                    }
                                } else if (fldStRemainIt->m_aggregate == AnnualFieldSet::AggregationKind::maximumDuringHoursShown) {
                                    if (fldStRemainIt->m_varAvgSum == OutputProcessor::StoreType::Sum) { // if it is a summed variable
                                        scanValue /= secondsInTimeStep;
                                    }
                                    if (scanValue > oldScanValue) {
                                        fldStRemainIt->m_cell[row].result = scanValue;
                                        fldStRemainIt->m_cell[row].timeStamp = timestepTimeStamp;
                                    }
                                } else {
                                    // do nothing
                                }
                            }
                            activeHoursShown = false;
                        }
                    }
                }
            }
        }
    }
}

void ResetAnnualGathering(const EnergyPlusData &state)
{
    // Jason Glazer, October 2015
    // This function is not part of the class but acts as an interface between procedural code and the class by
    // resetting data for each of the AnnualTable objects
    auto &annualTables = state.dataOutputReportTabularAnnual->annualTables;
    for (auto &annualTable : annualTables) {
        annualTable.resetGathering();
    }
}

void AnnualTable::resetGathering()
{
    for (std::size_t row = 0; row < m_objectNames.size(); ++row) {
        for (auto &fldSt : m_annualFields) {
            auto &cell = fldSt.m_cell[row];
            switch (fldSt.m_aggregate) {
            case AnnualFieldSet::AggregationKind::maximum:
            case AnnualFieldSet::AggregationKind::maximumDuringHoursShown:
                cell.result = verySmall;
                break;
            case AnnualFieldSet::AggregationKind::minimum:
            case AnnualFieldSet::AggregationKind::minimumDuringHoursShown:
                cell.result = veryLarge;
                break;
            default:
                cell.result = 0.0;
                break;
            }
            cell.duration = 0.0;
            cell.timeStamp = 0;

            // Clear deferred results
            cell.deferredResults.clear();
            cell.deferredElapsed.clear();
        }
    }
}

Real64 AnnualTable::getElapsedTime(const EnergyPlusData &state, const OutputProcessor::TimeStepType kindOfTimeStep)
{
    Real64 elapsedTime;
    if (kindOfTimeStep == OutputProcessor::TimeStepType::Zone) {
        elapsedTime = state.dataGlobal->TimeStepZone;
    } else {
        elapsedTime = state.dataHVACGlobal->TimeStepSys;
    }
    return elapsedTime;
}

Real64 AnnualTable::getSecondsInTimeStep(const EnergyPlusData &state, const OutputProcessor::TimeStepType kindOfTimeStep)
{
    Real64 secondsInTimeStep;
    if (kindOfTimeStep == OutputProcessor::TimeStepType::Zone) {
        secondsInTimeStep = state.dataGlobal->TimeStepZoneSec;
    } else {
        secondsInTimeStep = state.dataHVACGlobal->TimeStepSysSec;
    }
    return secondsInTimeStep;
}

void WriteAnnualTables(EnergyPlusData &state)
{
    auto &annualTables = state.dataOutputReportTabularAnnual->annualTables;
    for (auto const &currentStyle : state.dataOutRptTab->tabularReportPasses) {

        // Jason Glazer, August 2015
        // This function is not part of the class but acts as an interface between procedural code and the class by
        // invoking the writeTable member function for each of the AnnualTable objects
        for (auto &annualTable : annualTables) {
            annualTable.writeTable(state, currentStyle);
        }
    }
}

void AnnualTable::writeTable(EnergyPlusData &state, OutputReportTabular::tabularReportStyle const style)
{
    Array1D_string columnHead;
    Array1D_int columnWidth;
    Array1D_string rowHead;
    Array2D_string tableBody;
    std::vector<std::string> aggString;
    std::string energyUnitsString;
    std::string varNameWithUnits;
    int indexUnitConv;
    Real64 curVal;
    std::string curUnits;
    Real64 curConversionFactor;
    Real64 curConversionOffset;
    Real64 minVal;
    Real64 maxVal;
    Real64 sumVal;
    Real64 sumDuration;
    bool createBinRangeTable = false;

    aggString = setupAggString();
    Real64 energyUnitsConversionFactor = AnnualTable::setEnergyUnitStringAndFactor(style.unitsStyle, energyUnitsString);

    // Compute the columns related to the binning schemes
    computeBinColumns(state, style.unitsStyle);

    // Use title case names of variables if available for column headers
    columnHeadersToTitleCase(state);

    // first loop through and count how many 'columns' are defined
    // since max and min actually define two columns (the value
    // and the timestamp).
    int columnCount = 0;
    for (auto &fldStIt : m_annualFields) {
        columnCount += columnCountForAggregation(fldStIt.m_aggregate);
    }
    columnHead.allocate(columnCount);
    columnWidth.dimension(columnCount);
    columnWidth = 14;                        // array assignment - same for all columns
    int rowCount = m_objectNames.size() + 4; // add blank, sum/avg, min, max rows.
    int rowSumAvg = m_objectNames.size() + 2;
    int rowMin = m_objectNames.size() + 3;
    int rowMax = m_objectNames.size() + 4;

    rowHead.allocate(rowCount);
    for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
        rowHead(row + 1) = m_objectNames[row];
    }
    rowHead(rowSumAvg) = "Annual Sum or Average";
    rowHead(rowMin) = "Minimum of Rows";
    rowHead(rowMax) = "Maximum of Rows";

    tableBody.allocate(columnCount, rowCount);
    tableBody = ""; // set entire table to blank as default
    int columnRecount = 0;
    for (auto &fldSt : m_annualFields) {
        std::string curAggString = aggString[static_cast<int>(fldSt.m_aggregate)];
        if (!curAggString.empty()) {
            curAggString = " {" + trim(curAggString) + '}';
        }
        // do the unit conversions
        switch (style.unitsStyle) {
        case OutputReportTabular::UnitsStyle::InchPound:
        case OutputReportTabular::UnitsStyle::InchPoundExceptElectricity: {
            varNameWithUnits = EnergyPlus::format("{} [{}]", fldSt.m_variMeter, Constant::unitNames[static_cast<int>(fldSt.m_varUnits)]);
            OutputReportTabular::LookupSItoIP(state, varNameWithUnits, indexUnitConv, curUnits);
            OutputReportTabular::GetUnitConversion(state, indexUnitConv, curConversionFactor, curConversionOffset, curUnits);
            break;
        }
        default: {
            // Just do the Joule conversion
            switch (fldSt.m_varUnits) {
            case Constant::Units::J:
                curUnits = energyUnitsString;
                curConversionFactor = energyUnitsConversionFactor;
                curConversionOffset = 0.0;
                break;
            default:
                curUnits = Constant::unitNames[static_cast<int>(fldSt.m_varUnits)];
                curConversionFactor = 1.0;
                curConversionOffset = 0.0;
                break;
            }
            break;
        }
        }
        int curAgg = fldSt.m_aggregate;
        columnRecount += columnCountForAggregation(fldSt.m_aggregate);
        switch (curAgg) {
        case AnnualFieldSet::AggregationKind::sumOrAvg:
        case AnnualFieldSet::AggregationKind::sumOrAverageHoursShown: {
            columnHead(columnRecount) = fldSt.m_colHead + curAggString + " [" + curUnits + ']';
            sumVal = 0.0;
            sumDuration = 0.0;
            minVal = veryLarge;
            maxVal = verySmall;
            for (unsigned int row = 0; row != m_objectNames.size(); ++row) {
                if (fldSt.m_cell[row].indexesForKeyVar >= 0) {
                    if (fldSt.m_varAvgSum == OutputProcessor::StoreType::Average) {
                        if (fldSt.m_cell[row].duration != 0.0) {
                            curVal = ((fldSt.m_cell[row].result / fldSt.m_cell[row].duration) * curConversionFactor) + curConversionOffset;
                        } else {
                            curVal = 0.0;
                        }
                        sumVal += (fldSt.m_cell[row].result * curConversionFactor) + curConversionOffset;
                        sumDuration += fldSt.m_cell[row].duration;
                    } else {
                        curVal = (fldSt.m_cell[row].result * curConversionFactor) + curConversionOffset;
                        sumVal += curVal;
                    }
                    tableBody(columnRecount, row + 1) = OutputReportTabular::RealToStr(style.formatReals, curVal, fldSt.m_showDigits);
                    if (curVal > maxVal) {
                        maxVal = curVal;
                    }
                    if (curVal < minVal) {
                        minVal = curVal;
                    }
                } else {
                    tableBody(columnRecount, row + 1) = "-";
                }
            }
            if (fldSt.m_varAvgSum == OutputProcessor::StoreType::Average) {
                if (sumDuration > 0) {
                    tableBody(columnRecount, rowSumAvg) = OutputReportTabular::RealToStr(style.formatReals, sumVal / sumDuration, fldSt.m_showDigits);
                } else {
                    tableBody(columnRecount, rowSumAvg) = "";
                }
            } else {
                tableBody(columnRecount, rowSumAvg) = OutputReportTabular::RealToStr(style.formatReals, sumVal, fldSt.m_showDigits);
            }
            if (minVal != veryLarge) {
                tableBody(columnRecount, rowMax) = OutputReportTabular::RealToStr(style.formatReals, minVal, fldSt.m_showDigits);
            }
            if (maxVal != verySmall) {
                tableBody(columnRecount, rowMin) = OutputReportTabular::RealToStr(style.formatReals, maxVal, fldSt.m_showDigits);
            }
            break;
        }
        case AnnualFieldSet::AggregationKind::hoursZero:
        case AnnualFieldSet::AggregationKind::hoursNonZero:
        case AnnualFieldSet::AggregationKind::hoursPositive:
        case AnnualFieldSet::AggregationKind::hoursNonPositive:
        case AnnualFieldSet::AggregationKind::hoursNegative:
        case AnnualFieldSet::AggregationKind::hoursNonNegative: {
            columnHead(columnRecount) = fldSt.m_colHead + curAggString + " [HOURS]";
            sumVal = 0.0;
            minVal = veryLarge;
            maxVal = verySmall;
            for (unsigned int row = 0; row != m_objectNames.size(); ++row) {
                curVal = fldSt.m_cell[row].result;
                curVal = curVal * curConversionFactor + curConversionOffset;
                tableBody(columnRecount, row + 1) = OutputReportTabular::RealToStr(style.formatReals, curVal, fldSt.m_showDigits);
                sumVal += curVal;
                if (curVal > maxVal) {
                    maxVal = curVal;
                }
                if (curVal < minVal) {
                    minVal = curVal;
                }
            }
            tableBody(columnRecount, rowSumAvg) = OutputReportTabular::RealToStr(style.formatReals, sumVal, fldSt.m_showDigits);
            if (minVal != veryLarge) {
                tableBody(columnRecount, rowMax) = OutputReportTabular::RealToStr(style.formatReals, minVal, fldSt.m_showDigits);
            }
            if (maxVal != verySmall) {
                tableBody(columnRecount, rowMin) = OutputReportTabular::RealToStr(style.formatReals, maxVal, fldSt.m_showDigits);
            }
            break;
        }
        case AnnualFieldSet::AggregationKind::valueWhenMaxMin: {
            if (fldSt.m_varAvgSum == OutputProcessor::StoreType::Sum) {
                curUnits += "/s";
            }
            fixUnitsPerSecond(curUnits, curConversionFactor);
            columnHead(columnRecount) = fldSt.m_colHead + curAggString + " [" + curUnits + ']';
            minVal = veryLarge;
            maxVal = verySmall;
            for (unsigned int row = 0; row != m_objectNames.size(); ++row) {
                curVal = fldSt.m_cell[row].result;
                curVal = curVal * curConversionFactor + curConversionOffset;
                tableBody(columnRecount, row + 1) = OutputReportTabular::RealToStr(style.formatReals, curVal, fldSt.m_showDigits);
                if (curVal > maxVal) {
                    maxVal = curVal;
                }
                if (curVal < minVal) {
                    minVal = curVal;
                }
            }
            if (minVal != veryLarge) {
                tableBody(columnRecount, rowMin) = OutputReportTabular::RealToStr(style.formatReals, minVal, fldSt.m_showDigits);
            }
            if (maxVal != verySmall) {
                tableBody(columnRecount, rowMax) = OutputReportTabular::RealToStr(style.formatReals, maxVal, fldSt.m_showDigits);
            }
            break;
        }
        case AnnualFieldSet::AggregationKind::maximum:
        case AnnualFieldSet::AggregationKind::minimum:
        case AnnualFieldSet::AggregationKind::maximumDuringHoursShown:
        case AnnualFieldSet::AggregationKind::minimumDuringHoursShown: {
            if (fldSt.m_varAvgSum == OutputProcessor::StoreType::Sum) {
                curUnits += "/s";
            }
            fixUnitsPerSecond(curUnits, curConversionFactor);
            columnHead(columnRecount - 1) = fldSt.m_colHead + curAggString + " [" + curUnits + ']';
            columnHead(columnRecount) = fldSt.m_colHead + " {TIMESTAMP}";
            minVal = veryLarge;
            maxVal = verySmall;
            for (unsigned int row = 0; row != m_objectNames.size(); ++row) {
                curVal = fldSt.m_cell[row].result;
                if ((curVal < veryLarge) && (curVal > verySmall)) {
                    curVal = curVal * curConversionFactor + curConversionOffset;
                    if (curVal > maxVal) {
                        maxVal = curVal;
                    }
                    if (curVal < minVal) {
                        minVal = curVal;
                    }
                    if (curVal < veryLarge && curVal > verySmall) {
                        tableBody(columnRecount - 1, row + 1) = OutputReportTabular::RealToStr(style.formatReals, curVal, fldSt.m_showDigits);
                    } else {
                        tableBody(columnRecount - 1, row + 1) = "-";
                    }
                    tableBody(columnRecount, row + 1) = OutputReportTabular::DateToString(fldSt.m_cell[row].timeStamp);
                } else {
                    tableBody(columnRecount - 1, row + 1) = "-";
                    tableBody(columnRecount, row + 1) = "-";
                }
            }
            if (minVal < veryLarge) {
                tableBody(columnRecount - 1, rowMin) = OutputReportTabular::RealToStr(style.formatReals, minVal, fldSt.m_showDigits);
            } else {
                tableBody(columnRecount - 1, rowMin) = "-";
            }
            if (maxVal > verySmall) {
                tableBody(columnRecount - 1, rowMax) = OutputReportTabular::RealToStr(style.formatReals, maxVal, fldSt.m_showDigits);
            } else {
                tableBody(columnRecount - 1, rowMax) = "-";
            }
            break;
        }
        case AnnualFieldSet::AggregationKind::hoursInTenBinsMinToMax: {
            if (fldSt.m_varAvgSum == OutputProcessor::StoreType::Sum) {
                curUnits += "/s";
            }
            fixUnitsPerSecond(curUnits, curConversionFactor);
            for (int iBin = 0; iBin != 10; ++iBin) {
                char binIndicator = static_cast<char>(iBin + 65);
                columnHead(columnRecount - 9 + iBin) = fldSt.m_colHead + curAggString + " BIN " + binIndicator;
                for (unsigned int row = 0; row != m_objectNames.size(); ++row) {
                    tableBody(columnRecount - 9 + iBin, row + 1) =
                        OutputReportTabular::RealToStr(style.formatReals, fldSt.m_cell[row].m_timeInBin[iBin], fldSt.m_showDigits);
                }

                tableBody(columnRecount - 9 + iBin, rowSumAvg) =
                    OutputReportTabular::RealToStr(style.formatReals, fldSt.m_timeInBinTotal[iBin], fldSt.m_showDigits);
            }
            createBinRangeTable = true;
            break;
        }
        case AnnualFieldSet::AggregationKind::hoursInTenBinsZeroToMax: {
            if (fldSt.m_varAvgSum == OutputProcessor::StoreType::Sum) {
                curUnits += "/s";
            }
            fixUnitsPerSecond(curUnits, curConversionFactor);
            for (int iBin = 0; iBin != 10; ++iBin) {
                char binIndicator = static_cast<char>(iBin + 65);
                columnHead(columnRecount - 9 + iBin) = fldSt.m_colHead + curAggString + " BIN " + binIndicator;
                for (unsigned int row = 0; row != m_objectNames.size(); ++row) {
                    tableBody(columnRecount - 9 + iBin, row + 1) =
                        OutputReportTabular::RealToStr(style.formatReals, fldSt.m_cell[row].m_timeInBin[iBin], fldSt.m_showDigits);
                }
                tableBody(columnRecount - 9 + iBin, rowSumAvg) =
                    OutputReportTabular::RealToStr(style.formatReals, fldSt.m_timeInBinTotal[iBin], fldSt.m_showDigits);
            }
            columnHead(columnRecount - 10) = fldSt.m_colHead + curAggString + " LESS THAN BIN A";
            for (unsigned int row = 0; row != m_objectNames.size(); ++row) {
                tableBody(columnRecount - 10, row + 1) =
                    OutputReportTabular::RealToStr(style.formatReals, fldSt.m_cell[row].m_timeBelowBottomBin, fldSt.m_showDigits);
            }
            tableBody(columnRecount - 10, rowSumAvg) =
                OutputReportTabular::RealToStr(style.formatReals, fldSt.m_timeBelowBottomBinTotal, fldSt.m_showDigits);
            createBinRangeTable = true;
            break;
        }
        case AnnualFieldSet::AggregationKind::hoursInTenBinsMinToZero: {
            if (fldSt.m_varAvgSum == OutputProcessor::StoreType::Sum) {
                curUnits += "/s";
            }
            fixUnitsPerSecond(curUnits, curConversionFactor);
            for (int iBin = 0; iBin != 10; ++iBin) {
                char binIndicator = static_cast<char>(iBin + 65);
                columnHead(columnRecount - 10 + iBin) = fldSt.m_colHead + curAggString + " BIN " + binIndicator;
                for (unsigned int row = 0; row != m_objectNames.size(); ++row) {
                    tableBody(columnRecount - 10 + iBin, row + 1) =
                        OutputReportTabular::RealToStr(style.formatReals, fldSt.m_cell[row].m_timeInBin[iBin], fldSt.m_showDigits);
                }
                tableBody(columnRecount - 10 + iBin, rowSumAvg) =
                    OutputReportTabular::RealToStr(style.formatReals, fldSt.m_timeInBinTotal[iBin], fldSt.m_showDigits);
            }
            columnHead(columnRecount) = fldSt.m_colHead + curAggString + " MORE THAN BIN J";
            for (unsigned int row = 0; row != m_objectNames.size(); ++row) {
                tableBody(columnRecount, row + 1) =
                    OutputReportTabular::RealToStr(style.formatReals, fldSt.m_cell[row].m_timeAboveTopBin, fldSt.m_showDigits);
            }
            tableBody(columnRecount, rowSumAvg) = OutputReportTabular::RealToStr(style.formatReals, fldSt.m_timeAboveTopBinTotal, fldSt.m_showDigits);
            createBinRangeTable = true;
            break;
        }
        case AnnualFieldSet::AggregationKind::hoursInTenPercentBins:
        case AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusTwoStdDev:
        case AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusThreeStdDev:
            break;

        default:
            break;
        }

    } // fldStIt
    if (style.produceTabular) {
        OutputReportTabular::WriteReportHeaders(state, m_name, "Entire Facility", OutputProcessor::StoreType::Average);
        OutputReportTabular::WriteSubtitle(state, "Custom Annual Report");
        OutputReportTabular::WriteTable(state, tableBody, rowHead, columnHead, columnWidth, true); // transpose annual XML tables.
    }
    if (style.produceJSON) {
        if (state.dataResultsFramework->resultsFramework->timeSeriesAndTabularEnabled()) {
            state.dataResultsFramework->resultsFramework->TabularReportsCollection.addReportTable(
                tableBody, rowHead, columnHead, m_name, "Entire Facility", "Custom Annual Report");
        }
    }
    if (style.produceSQLite) {
        if (state.dataSQLiteProcedures->sqlite) {
            state.dataSQLiteProcedures->sqlite->createSQLiteTabularDataRecords(
                tableBody, rowHead, columnHead, m_name, "Entire Facility", "Custom Annual Report");
        }
    }
    // for the new binning aggregation types create a second table of the bin ranges
    if (createBinRangeTable) {
        Array1D_string colHeadRange;
        Array1D_int colWidthRange;
        Array1D_string rowHeadRange;
        Array2D_string tableBodyRange;
        colHeadRange.allocate(10);
        colWidthRange.allocate(10);
        colWidthRange = 14; // array assignment - same for all columns
        rowHeadRange.allocate(2);
        rowHeadRange(1) = ">=";
        rowHeadRange(2) = "<";
        tableBodyRange.allocate(10, 2);
        for (auto &fldStIt : m_annualFields) {
            switch (fldStIt.m_aggregate) {
            case AnnualFieldSet::AggregationKind::hoursInTenBinsMinToMax:
            case AnnualFieldSet::AggregationKind::hoursInTenBinsZeroToMax:
            case AnnualFieldSet::AggregationKind::hoursInTenBinsMinToZero: {
                const Real64 binBottom = fldStIt.m_bottomBinValue;
                const Real64 binTop = fldStIt.m_topBinValue;
                constexpr int numBins = 10;
                const Real64 intervalSize =
                    ((binBottom == veryLarge) && (binTop == verySmall)) ? 0 : ((binTop - binBottom) / static_cast<Real64>(numBins));

                // Column headers
                for (int iBin = 0; iBin < numBins; ++iBin) {
                    const char binLetter = static_cast<char>('A' + iBin);
                    colHeadRange(iBin + 1) = std::string("BIN ") + binLetter;
                }
                // Bin bounds
                for (int iBin = 0; iBin < numBins; ++iBin) {
                    const Real64 lo = binBottom + static_cast<Real64>(iBin) * intervalSize;
                    const Real64 hi = binBottom + static_cast<Real64>(iBin + 1) * intervalSize;
                    tableBodyRange(iBin + 1, 1) = OutputReportTabular::RealToStr(style.formatReals, lo, fldStIt.m_showDigits);
                    tableBodyRange(iBin + 1, 2) = OutputReportTabular::RealToStr(style.formatReals, hi, fldStIt.m_showDigits);
                }
                if (style.produceTabular) {
                    OutputReportTabular::WriteSubtitle(state, "Bin Sizes for: " + fldStIt.m_colHead);
                    OutputReportTabular::WriteTable(state,
                                                    tableBodyRange,
                                                    rowHeadRange,
                                                    colHeadRange,
                                                    colWidthRange,
                                                    true); // transpose annual XML tables
                }
                if (style.produceSQLite) {
                    if (state.dataSQLiteProcedures->sqlite) {
                        state.dataSQLiteProcedures->sqlite->createSQLiteTabularDataRecords(
                            tableBodyRange, rowHeadRange, colHeadRange, m_name, "Entire Facility", "Bin Sizes");
                    }
                }
                if (style.produceJSON) {
                    if (state.dataResultsFramework->resultsFramework->timeSeriesAndTabularEnabled()) {
                        state.dataResultsFramework->resultsFramework->TabularReportsCollection.addReportTable(
                            tableBodyRange, rowHeadRange, colHeadRange, m_name, "Entire Facility", "Bin Sizes");
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

std::vector<std::string> AnnualTable::setupAggString()
{
    std::vector<std::string> retStringVec;
    retStringVec.resize(20);
    retStringVec[AnnualFieldSet::AggregationKind::sumOrAvg] = "";
    retStringVec[AnnualFieldSet::AggregationKind::maximum] = " MAXIMUM ";
    retStringVec[AnnualFieldSet::AggregationKind::minimum] = " MINIMUM ";
    retStringVec[AnnualFieldSet::AggregationKind::valueWhenMaxMin] = " AT MAX/MIN ";
    retStringVec[AnnualFieldSet::AggregationKind::hoursZero] = " HOURS ZERO ";
    retStringVec[AnnualFieldSet::AggregationKind::hoursNonZero] = " HOURS NON-ZERO ";
    retStringVec[AnnualFieldSet::AggregationKind::hoursPositive] = " HOURS POSITIVE ";
    retStringVec[AnnualFieldSet::AggregationKind::hoursNonPositive] = " HOURS NON-POSITIVE ";
    retStringVec[AnnualFieldSet::AggregationKind::hoursNegative] = " HOURS NEGATIVE ";
    retStringVec[AnnualFieldSet::AggregationKind::hoursNonNegative] = " HOURS NON-NEGATIVE ";
    retStringVec[AnnualFieldSet::AggregationKind::hoursInTenPercentBins] = " HOURS IN";            // " HOURS IN TEN PERCENT BINS ";
    retStringVec[AnnualFieldSet::AggregationKind::hoursInTenBinsMinToMax] = " HOURS IN";           // " HOURS IN TEN BINS MIN TO MAX ";
    retStringVec[AnnualFieldSet::AggregationKind::hoursInTenBinsZeroToMax] = " HOURS IN";          // " HOURS IN TEN BINS ZERO TO MAX ";
    retStringVec[AnnualFieldSet::AggregationKind::hoursInTenBinsMinToZero] = " HOURS IN";          // " HOURS IN TEN BINS MIN TO ZERO ";
    retStringVec[AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusTwoStdDev] = " HOURS IN"; // " HOURS IN TEN BINS PLUS OR MINUS TWO STD DEV ";
    retStringVec[AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusThreeStdDev] =
        " HOURS IN"; // " HOURS IN TEN BINS PLUS OR MINUS THREE STD DEV ";
    retStringVec[AnnualFieldSet::AggregationKind::noAggregation] = " NO AGGREGATION ";
    retStringVec[AnnualFieldSet::AggregationKind::sumOrAverageHoursShown] = " FOR HOURS SHOWN ";
    retStringVec[AnnualFieldSet::AggregationKind::maximumDuringHoursShown] = " MAX FOR HOURS SHOWN ";
    retStringVec[AnnualFieldSet::AggregationKind::minimumDuringHoursShown] = " MIN FOR HOURS SHOWN ";
    return retStringVec;
}

Real64 AnnualTable::setEnergyUnitStringAndFactor(OutputReportTabular::UnitsStyle const unitsStyle, std::string &unitString)
{
    Real64 convFactor = 1.0;
    unitString = "J";
    switch (unitsStyle) {
    case OutputReportTabular::UnitsStyle::JtoKWH:
        unitString = "kWh";
        convFactor = 1.0 / 3600000.0;
        break;
    case OutputReportTabular::UnitsStyle::JtoMJ:
        unitString = "MJ";
        convFactor = 1.0 / 1000000.0;
        break;
    case OutputReportTabular::UnitsStyle::JtoGJ:
        unitString = "GJ";
        convFactor = 1.0 / 1000000000.0;
        break;
    case OutputReportTabular::UnitsStyle::None:
    default:
        break;
    }

    return convFactor;
}

void AnnualTable::fixUnitsPerSecond(std::string &unitString, Real64 &conversionFactor)
{
    if (unitString == "J/s") {
        unitString = "W";
    } else if (unitString == "kWh/s") {
        unitString = "W";
        conversionFactor *= 3600000.0;
    } else if (unitString == "GJ/s") {
        unitString = "kW";
        conversionFactor *= 1000000.0;
    } else if (unitString == "MJ/s") {
        unitString = "kW";
        conversionFactor *= 1000.0;
    } else if (unitString == "therm/s") {
        unitString = "kBtu/h";
        conversionFactor *= 360000.0;
    } else if (unitString == "kBtu/s") {
        unitString = "kBtu/h";
        conversionFactor *= 3600.0;
    } else if (unitString == "ton-hrs/s") {
        unitString = "ton";
        conversionFactor *= 3600.0;
    }
}

AnnualFieldSet::AggregationKind stringToAggKind(EnergyPlusData &state, std::string inString)
// Jason Glazer, August 2015
// The function converts a string into an enumeration that describes the type of aggregation
// used in REPORT:TABLE:ANNUAL.
{
    AnnualFieldSet::AggregationKind outAggType;

    if (Util::SameString(inString, "SumOrAverage")) {
        outAggType = AnnualFieldSet::AggregationKind::sumOrAvg;
    } else if (Util::SameString(inString, "Maximum")) {
        outAggType = AnnualFieldSet::AggregationKind::maximum;
    } else if (Util::SameString(inString, "Minimum")) {
        outAggType = AnnualFieldSet::AggregationKind::minimum;
    } else if (Util::SameString(inString, "ValueWhenMaximumOrMinimum")) {
        outAggType = AnnualFieldSet::AggregationKind::valueWhenMaxMin;
    } else if (Util::SameString(inString, "HoursZero")) {
        outAggType = AnnualFieldSet::AggregationKind::hoursZero;
    } else if (Util::SameString(inString, "HoursNonzero")) {
        outAggType = AnnualFieldSet::AggregationKind::hoursNonZero;
    } else if (Util::SameString(inString, "HoursPositive")) {
        outAggType = AnnualFieldSet::AggregationKind::hoursPositive;
    } else if (Util::SameString(inString, "HoursNonpositive")) {
        outAggType = AnnualFieldSet::AggregationKind::hoursNonPositive;
    } else if (Util::SameString(inString, "HoursNegative")) {
        outAggType = AnnualFieldSet::AggregationKind::hoursNegative;
    } else if (Util::SameString(inString, "HoursNonNegative")) {
        outAggType = AnnualFieldSet::AggregationKind::hoursNonNegative;
    } else if (Util::SameString(inString, "HoursInTenPercentBins")) {
        outAggType = AnnualFieldSet::AggregationKind::hoursInTenPercentBins;
    } else if (Util::SameString(inString, "HourInTenBinsMinToMax")) {
        outAggType = AnnualFieldSet::AggregationKind::hoursInTenBinsMinToMax;
    } else if (Util::SameString(inString, "HourInTenBinsZeroToMax")) {
        outAggType = AnnualFieldSet::AggregationKind::hoursInTenBinsZeroToMax;
    } else if (Util::SameString(inString, "HourInTenBinsMinToZero")) {
        outAggType = AnnualFieldSet::AggregationKind::hoursInTenBinsMinToZero;
    } else if (Util::SameString(inString, "HoursInTenBinsPlusMinusTwoStdDev")) {
        outAggType = AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusTwoStdDev;
    } else if (Util::SameString(inString, "HoursInTenBinsPlusMinusThreeStdDev")) {
        outAggType = AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusThreeStdDev;
    } else if (Util::SameString(inString, "NoAggregation")) {
        outAggType = AnnualFieldSet::AggregationKind::noAggregation;
    } else if (Util::SameString(inString, "SumOrAverageDuringHoursShown")) {
        outAggType = AnnualFieldSet::AggregationKind::sumOrAverageHoursShown;
    } else if (Util::SameString(inString, "MaximumDuringHoursShown")) {
        outAggType = AnnualFieldSet::AggregationKind::maximumDuringHoursShown;
    } else if (Util::SameString(inString, "MinimumDuringHoursShown")) {
        outAggType = AnnualFieldSet::AggregationKind::minimumDuringHoursShown;
    } else {
        outAggType = AnnualFieldSet::AggregationKind::sumOrAvg;
        ShowWarningError(state, EnergyPlus::format("Invalid aggregation type=\"{}\"  Defaulting to SumOrAverage.", inString));
    }
    return outAggType;
}

int AnnualTable::columnCountForAggregation(const AnnualFieldSet::AggregationKind curAgg)
{
    switch (curAgg) {
    case AnnualFieldSet::AggregationKind::sumOrAvg:
    case AnnualFieldSet::AggregationKind::valueWhenMaxMin:
    case AnnualFieldSet::AggregationKind::hoursZero:
    case AnnualFieldSet::AggregationKind::hoursNonZero:
    case AnnualFieldSet::AggregationKind::hoursPositive:
    case AnnualFieldSet::AggregationKind::hoursNonPositive:
    case AnnualFieldSet::AggregationKind::hoursNegative:
    case AnnualFieldSet::AggregationKind::hoursNonNegative:
    case AnnualFieldSet::AggregationKind::sumOrAverageHoursShown:
    case AnnualFieldSet::AggregationKind::noAggregation:
        return 1;
    case AnnualFieldSet::AggregationKind::maximum:
    case AnnualFieldSet::AggregationKind::minimum:
    case AnnualFieldSet::AggregationKind::maximumDuringHoursShown:
    case AnnualFieldSet::AggregationKind::minimumDuringHoursShown:
        return 2;
    case AnnualFieldSet::AggregationKind::hoursInTenBinsMinToMax:
        return 10;
    case AnnualFieldSet::AggregationKind::hoursInTenBinsZeroToMax:
    case AnnualFieldSet::AggregationKind::hoursInTenBinsMinToZero:
        return 11;
    case AnnualFieldSet::AggregationKind::hoursInTenPercentBins:
    case AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusTwoStdDev:
    case AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusThreeStdDev:
        return 12;
    default:
        return 0;
    }
}

std::string AnnualTable::trim(const std::string &str)
{
    const std::string whitespace = " \t";
    const size_t strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) {
        return ""; // no content
    }

    const size_t strEnd = str.find_last_not_of(whitespace);
    const size_t strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

void AddAnnualTableOfContents(const EnergyPlusData &state, std::ostream &nameOfStream)
{
    // Jason Glazer, August 2015
    // This function is not part of the class but acts as an interface between procedural code and the class by
    // invoking the writeTable member function for each of the AnnualTable objects
    auto &annualTables = state.dataOutputReportTabularAnnual->annualTables;
    for (auto &annualTable : annualTables) {
        annualTable.addTableOfContents(nameOfStream);
    }
}

void AnnualTable::addTableOfContents(std::ostream &nameOfStream) const
{
    nameOfStream << "<p><b>" << m_name << "</b></p> |\n";
    nameOfStream << "<a href=\"#" << OutputReportTabular::MakeAnchorName(m_name, "Entire Facility") << "\">" << "Entire Facility" << "</a>    |   \n";
}

void AnnualTable::computeBinColumns(EnergyPlusData &state, OutputReportTabular::UnitsStyle const unitsStyle_para)
{
    for (auto &fldStIt : m_annualFields) {
        // for columns with binning aggregation types compute the statistics
        switch (fldStIt.m_aggregate) {
        case AnnualFieldSet::AggregationKind::hoursInTenBinsMinToMax:
        case AnnualFieldSet::AggregationKind::hoursInTenBinsZeroToMax:
        case AnnualFieldSet::AggregationKind::hoursInTenBinsMinToZero:
        case AnnualFieldSet::AggregationKind::hoursInTenPercentBins:
        case AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusTwoStdDev:
        case AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusThreeStdDev: {
            // the size the deferred vectors should be same for all rows
            if (!allRowsSameSizeDeferredVectors(fldStIt)) {
                break;
            }

            convertUnitForDeferredResults(state, fldStIt, unitsStyle_para);

            std::vector<Real64> deferredTotalForColumn;
            Real64 minVal = veryLarge;
            Real64 maxVal = verySmall;
            Real64 sum = 0;
            Real64 curVal = 0.0;

            for (unsigned int jDefRes = 0; jDefRes != fldStIt.m_cell[0].deferredResults.size(); jDefRes++) {
                sum = 0;
                for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
                    curVal = fldStIt.m_cell[row].deferredResults[jDefRes];
                    sum += curVal;
                    if (curVal > maxVal) {
                        maxVal = curVal;
                    }
                    if (curVal < minVal) {
                        minVal = curVal;
                    }
                }
                deferredTotalForColumn.push_back(sum / static_cast<float>(m_objectNames.size())); // put average value into the total row
            }

            // Decide bin range endpoints based on aggregation kind
            switch (fldStIt.m_aggregate) {
            case AnnualFieldSet::AggregationKind::hoursInTenBinsMinToMax:
                fldStIt.m_topBinValue = maxVal;
                fldStIt.m_bottomBinValue = minVal;
                break;
            case AnnualFieldSet::AggregationKind::hoursInTenBinsZeroToMax:
                fldStIt.m_topBinValue = maxVal;
                fldStIt.m_bottomBinValue = 0.0;
                break;
            case AnnualFieldSet::AggregationKind::hoursInTenBinsMinToZero:
                fldStIt.m_topBinValue = 0.0;
                fldStIt.m_bottomBinValue = minVal;
                break;
            case AnnualFieldSet::AggregationKind::hoursInTenPercentBins:
                fldStIt.m_topBinValue = 1.0;
                fldStIt.m_bottomBinValue = 0.0;
                break;
            case AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusTwoStdDev:
            case AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusThreeStdDev:
                break;
            default:
                break;
            }

            // compute the actual amount of time spent in each bin and above and below
            for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
                fldStIt.m_cell[row].m_timeInBin = calculateBins(10,
                                                                fldStIt.m_cell[row].deferredResults,
                                                                fldStIt.m_cell[row].deferredElapsed,
                                                                fldStIt.m_topBinValue,
                                                                fldStIt.m_bottomBinValue,
                                                                fldStIt.m_cell[row].m_timeAboveTopBin,
                                                                fldStIt.m_cell[row].m_timeBelowBottomBin);
            }
            // do the total row binning
            fldStIt.m_timeInBinTotal = calculateBins(10,
                                                     deferredTotalForColumn,
                                                     fldStIt.m_cell[0].deferredElapsed,
                                                     fldStIt.m_topBinValue,
                                                     fldStIt.m_bottomBinValue,
                                                     fldStIt.m_timeAboveTopBinTotal,
                                                     fldStIt.m_timeBelowBottomBinTotal);
            break;
        }
        default:
            break;
        }
    }
}

bool AnnualTable::allRowsSameSizeDeferredVectors(const AnnualFieldSet &fldSt) const
{
    bool returnFlag = true;
    unsigned int sizeOfDeferred = 0;
    for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
        if (sizeOfDeferred == 0) {
            sizeOfDeferred = fldSt.m_cell[row].deferredResults.size();
        } else {
            if (fldSt.m_cell[row].deferredResults.size() != sizeOfDeferred) {
                returnFlag = false;
                return returnFlag;
            }
        }
    }
    return returnFlag;
}

void AnnualTable::convertUnitForDeferredResults(EnergyPlusData &state, AnnualFieldSet &fldSt, OutputReportTabular::UnitsStyle const unitsStyle) const
{
    Real64 curConversionFactor;
    Real64 curConversionOffset;
    std::string curUnits;
    std::string energyUnitsString;
    const Real64 energyUnitsConversionFactor = AnnualTable::setEnergyUnitStringAndFactor(unitsStyle, energyUnitsString);

    // do the unit conversions
    switch (unitsStyle) {
    case OutputReportTabular::UnitsStyle::InchPound:
    case OutputReportTabular::UnitsStyle::InchPoundExceptElectricity: {
        int indexUnitConv;
        const std::string varNameWithUnits =
            EnergyPlus::format("{} [{}]", fldSt.m_variMeter, Constant::unitNames[static_cast<int>(fldSt.m_varUnits)]);
        OutputReportTabular::LookupSItoIP(state, varNameWithUnits, indexUnitConv, curUnits);
        OutputReportTabular::GetUnitConversion(state, indexUnitConv, curConversionFactor, curConversionOffset, curUnits);
        break;
    }
    default: { // SI (and "Jto*" styles): just do the Joule conversion if needed
        switch (fldSt.m_varUnits) {
        case Constant::Units::J:
            curUnits = energyUnitsString;
            curConversionFactor = energyUnitsConversionFactor;
            curConversionOffset = 0.0;
            break;
        default:
            curUnits = Constant::unitNames[static_cast<int>(fldSt.m_varUnits)];
            curConversionFactor = 1.0;
            curConversionOffset = 0.0;
            break;
        }
        break;
    }
    }

    if (fldSt.m_varAvgSum == OutputProcessor::StoreType::Sum) {
        curUnits += "/s";
    }
    fixUnitsPerSecond(curUnits, curConversionFactor);

    if (curConversionFactor != 1.0 || curConversionOffset != 0.0) {
        for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
            for (unsigned int jDefRes = 0; jDefRes != fldSt.m_cell[0].deferredResults.size(); jDefRes++) {
                const Real64 curSI = fldSt.m_cell[row].deferredResults[jDefRes];
                const Real64 curIP = curSI * curConversionFactor + curConversionOffset;
                fldSt.m_cell[row].deferredResults[jDefRes] = curIP;
            }
        }
    }
}

std::vector<Real64> AnnualTable::calculateBins(int const numberOfBins,
                                               const std::vector<Real64> &valuesToBin,
                                               std::vector<Real64> corrElapsedTime,
                                               Real64 const topOfBins,
                                               Real64 const bottomOfBins,
                                               Real64 &timeAboveTopBin,
                                               Real64 &timeBelowBottomBin)
{
    if (numberOfBins <= 0) {
        return {};
    }
    std::vector returnBins(numberOfBins, 0.0);
    timeAboveTopBin = 0.0;
    timeBelowBottomBin = 0.0;
    if (valuesToBin.empty()) {
        return returnBins;
    }
    const Real64 intervalSize = (topOfBins - bottomOfBins) / static_cast<Real64>(numberOfBins);
    std::vector<Real64>::iterator elapsedTimeIt = corrElapsedTime.begin();
    for (auto const &valueIt : valuesToBin) {
        if (valueIt < bottomOfBins) {
            timeBelowBottomBin += *elapsedTimeIt;
        } else if (valueIt >= topOfBins) {
            timeAboveTopBin += *elapsedTimeIt;
        } else {
            // determine which bin the results are in
            const int binNum = static_cast<int>((valueIt - bottomOfBins) / intervalSize);
            if (binNum < numberOfBins && binNum >= 0) {
                returnBins[binNum] += *elapsedTimeIt;
            }
        }
        ++elapsedTimeIt;
    }
    return returnBins;
}

void AnnualTable::columnHeadersToTitleCase(EnergyPlusData const &state)
{
    for (auto &fldSt : m_annualFields) {
        if (fldSt.m_variMeter == fldSt.m_colHead) {
            if (!fldSt.m_indexesForKeyVar.empty()) {
                const int varNum = fldSt.m_indexesForKeyVar[0];
                if (fldSt.m_typeOfVar == OutputProcessor::VariableType::Real) {
                    fldSt.m_colHead = state.dataOutputProcessor->outVars[varNum]->name;
                } else if (fldSt.m_typeOfVar == OutputProcessor::VariableType::Meter) {
                    fldSt.m_colHead = state.dataOutputProcessor->meters[varNum]->Name;
                }
            }
        }
    }
}

void AnnualTable::clearTable()
{
    m_name = "";
    m_filter = "";
    m_sched = nullptr;
    m_objectNames.clear();
    m_annualFields.clear();
}

std::vector<std::string> AnnualTable::inspectTable() const
{
    // added function just to inspect the main private AnnualTable members because no other
    // interface to the AnnualTable class is output oriented except writeTable and that is very complex.
    std::vector<std::string> ret;
    ret.push_back(m_name);
    ret.push_back(m_filter);
    ret.push_back(m_sched->Name);
    return ret;
}

std::vector<std::string> AnnualTable::inspectTableFieldSets(int const fldIndex) const
{
    // added function just to inspect the private field set members of AnnualTable because no other
    // interface to the AnnualTable class is output oriented except writeTable and that is very complex.
    AnnualFieldSet const &fldSt = m_annualFields[fldIndex];
    std::vector<std::string> ret;
    bool const hasCell = !fldSt.m_cell.empty();
    ret.reserve(hasCell ? 14 : 13);
    ret.push_back(fldSt.m_colHead);
    ret.push_back(fldSt.m_variMeter);
    ret.emplace_back(Constant::unitNames[static_cast<int>(fldSt.m_varUnits)]);
    std::string outStr = std::to_string(fldSt.m_showDigits);
    // ints
    ret.push_back(outStr);
    outStr = std::to_string(static_cast<int>(fldSt.m_typeOfVar));
    ret.push_back(outStr);
    outStr = std::to_string(fldSt.m_keyCount);
    ret.push_back(outStr);
    outStr = std::to_string(static_cast<int>(fldSt.m_varAvgSum));
    ret.push_back(outStr);
    outStr = std::to_string(static_cast<int>(fldSt.m_varStepType));
    ret.push_back(outStr);
    outStr = std::to_string(fldSt.m_aggregate);
    ret.push_back(outStr);
    // floats
    outStr = std::to_string(fldSt.m_bottomBinValue);
    ret.push_back(outStr);
    outStr = std::to_string(fldSt.m_topBinValue);
    ret.push_back(outStr);
    outStr = std::to_string(fldSt.m_timeAboveTopBinTotal);
    ret.push_back(outStr);
    outStr = std::to_string(fldSt.m_timeBelowBottomBinTotal);
    ret.push_back(outStr);
    // cell value
    if (hasCell) {
        outStr = std::to_string(fldSt.m_cell[0].result);
        ret.push_back(outStr);
    }
    return ret;
}

} // namespace EnergyPlus::OutputReportTabularAnnual
