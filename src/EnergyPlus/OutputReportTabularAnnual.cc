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
#include <EnergyPlus/CostEstimateManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportData.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/OutputReportTabularAnnual.hh>
#include <EnergyPlus/SQLiteProcedures.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::OutputReportTabularAnnual {

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

    int jAlpha;
    int numParams;            // Number of elements combined
    int numAlphas;            // Number of elements in the alpha array
    int numNums;              // Number of elements in the numeric array
    Array1D_string alphArray; // character string data
    Array1D<Real64> numArray; // numeric data
    int IOStat;               // IO Status when calling get input subroutine
    // static bool ErrorsFound( false );
    int objCount(0);
    int curNumDgts;
    AnnualFieldSet::AggregationKind curAgg(AnnualFieldSet::AggregationKind::sumOrAvg);

    auto &annualTables = state.dataOutputReportTabularAnnual->annualTables;

    objCount = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, currentModuleObject);
    if (objCount > 0) {

        state.dataOutRptTab->WriteTabularFiles = true;

        // if not a run period using weather do not create reports
        if (!state.dataGlobal->DoWeathSim) {
            ShowWarningError(
                state,
                format("{} requested with SimulationControl Run Simulation for Weather File Run Periods set to No so {} will not be generated",
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
            annualTables.push_back(AnnualTable(state, alphArray(1), alphArray(2), alphArray(3)));
            // the remaining fields are repeating in groups of three and need to be added to the data structure
            for (jAlpha = 4; jAlpha <= numAlphas; jAlpha += 2) {
                std::string curVarMtr = alphArray(jAlpha);
                if (curVarMtr.empty()) {
                    ShowWarningError(state,
                                     format("{}: Blank column specified in '{}', need to provide a variable or meter or EMS variable name ",
                                            currentModuleObject,
                                            alphArray(1)));
                }
                if (jAlpha <= numAlphas) {
                    std::string aggregationString = alphArray(jAlpha + 1);
                    curAgg = stringToAggKind(state, aggregationString);
                } else {
                    curAgg = AnnualFieldSet::AggregationKind::sumOrAvg; // if missing aggregation type use SumOrAverage
                }
                int indexNums = 1 + (jAlpha - 3) / 2; // compute the corresponding field index in the numArray
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
            ShowSevereError(state, format("{}: Must enter at least the first six fields.", currentModuleObject));
        }
    }
}

void AnnualTable::addFieldSet(std::string varName, AnnualFieldSet::AggregationKind aggKind, int dgts)
// Jason Glazer, August 2015
// This method is used along with the constructor to convert the GetInput for REPORT:TABLE:ANNUAL
// into the class data.
{
    m_annualFields.push_back(AnnualFieldSet(varName, aggKind, dgts));
    m_annualFields.back().m_colHead = varName; // use the variable name for the column heading
}

void AnnualTable::addFieldSet(std::string varName, std::string colName, AnnualFieldSet::AggregationKind aggKind, int dgts)
// Jason Glazer, August 2015
// This overloaded method allows for a specific column name to be different than the output variable or meter name
{
    m_annualFields.push_back(AnnualFieldSet(varName, aggKind, dgts));
    m_annualFields.back().m_colHead = colName; // use the user supplied column heading instead of just the variable name
}

void AnnualTable::setupGathering(EnergyPlusData &state)
// Jason Glazer, August 2015
// This method is used after GetInput for REPORT:TABLE:ANNUAL to set up how output variables, meters,
// input fields, and ems variables are gathered.
{
    OutputProcessor::VariableType typeVar = OutputProcessor::VariableType::Invalid;
    OutputProcessor::StoreType avgSumVar;
    OutputProcessor::TimeStepType stepTypeVar;
    Constant::Units unitsVar = Constant::Units::None;
    Array1D_string namesOfKeys;   // keyNames
    Array1D_int indexesForKeyVar; // keyVarIndexes
    std::list<std::string> allKeys;

    std::string filterFieldUpper = m_filter;
    std::transform(filterFieldUpper.begin(), filterFieldUpper.end(), filterFieldUpper.begin(), ::toupper);
    bool useFilter = (m_filter.size() != 0);

    std::vector<AnnualFieldSet>::iterator fldStIt;
    for (fldStIt = m_annualFields.begin(); fldStIt != m_annualFields.end(); ++fldStIt) {
        int keyCount = fldStIt->getVariableKeyCountandTypeFromFldSt(state, typeVar, avgSumVar, stepTypeVar, unitsVar);
        fldStIt->getVariableKeysFromFldSt(state, typeVar, keyCount, fldStIt->m_namesOfKeys, fldStIt->m_indexesForKeyVar);
        for (std::string nm : fldStIt->m_namesOfKeys) {
            std::string nmUpper = nm;
            std::transform(nmUpper.begin(), nmUpper.end(), nmUpper.begin(), ::toupper);
            if (!useFilter || nmUpper.find(filterFieldUpper) != std::string::npos) {
                allKeys.push_back(nm); // create list of all items
            }
        }
        fldStIt->m_typeOfVar = typeVar;
        fldStIt->m_varAvgSum = avgSumVar;
        fldStIt->m_varStepType = stepTypeVar;
        fldStIt->m_varUnits = unitsVar;
        fldStIt->m_keyCount = keyCount;
    }
    allKeys.sort();
    allKeys.unique();                                                        // will now just have a list of the unique keys that is sorted
    std::copy(allKeys.begin(), allKeys.end(), back_inserter(m_objectNames)); // copy list to the object names
    // size all columns list of cells to be the size of the
    for (fldStIt = m_annualFields.begin(); fldStIt != m_annualFields.end(); ++fldStIt) {
        fldStIt->m_cell.resize(m_objectNames.size());
    }
    // for each column (field set) set the rows cell to the output variable index (for variables)
    int foundKeyIndex;
    int tableRowIndex = 0;
    for (std::vector<std::string>::iterator objNmIt = m_objectNames.begin(); objNmIt != m_objectNames.end(); ++objNmIt) {
        for (fldStIt = m_annualFields.begin(); fldStIt != m_annualFields.end(); ++fldStIt) {
            foundKeyIndex = -1;
            for (std::string::size_type i = 0; i < fldStIt->m_namesOfKeys.size(); i++) {
                if (fldStIt->m_namesOfKeys[i] == *objNmIt) {
                    foundKeyIndex = i;
                    break;
                }
            }
            if (foundKeyIndex > -1) {
                fldStIt->m_cell[tableRowIndex].indexesForKeyVar = fldStIt->m_indexesForKeyVar[foundKeyIndex];
            } else {
                fldStIt->m_cell[tableRowIndex].indexesForKeyVar = -1; // flag value that cell is not gathered
            }
            if (fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::maximum ||
                fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::maximumDuringHoursShown) {
                fldStIt->m_cell[tableRowIndex].result = -9.9e99;
            } else if (fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::minimum ||
                       fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::minimumDuringHoursShown) {
                fldStIt->m_cell[tableRowIndex].result = 9.9e99;
            } else {
                fldStIt->m_cell[tableRowIndex].result = 0.0;
            }
            fldStIt->m_cell[tableRowIndex].duration = 0.0;
            fldStIt->m_cell[tableRowIndex].timeStamp = 0;
        }
        tableRowIndex++;
    }
}

void checkAggregationOrderForAnnual(EnergyPlusData &state)
{
    std::vector<AnnualTable>::iterator annualTableIt;
    bool invalidAggregationOrderFound = false;
    auto &annualTables = state.dataOutputReportTabularAnnual->annualTables;
    if (!state.dataGlobal->DoWeathSim) { // if no weather simulation than no reading of MonthlyInput array
        return;
    }
    for (annualTableIt = annualTables.begin(); annualTableIt != annualTables.end(); ++annualTableIt) {
        if (annualTableIt->invalidAggregationOrder(state)) {
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
    std::vector<AnnualFieldSet>::iterator fldStIt;
    bool foundMinOrMax = false;
    bool foundHourAgg = false;
    bool missingMaxOrMinError = false;
    bool missingHourAggError = false;
    for (fldStIt = m_annualFields.begin(); fldStIt != m_annualFields.end(); ++fldStIt) {
        if ((fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::maximum) ||
            (fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::minimum)) {
            foundMinOrMax = true;
        } else if ((fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::hoursNonZero) ||
                   (fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::hoursZero) ||
                   (fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::hoursPositive) ||
                   (fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::hoursNonPositive) ||
                   (fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::hoursNegative) ||
                   (fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::hoursNonNegative)) {
            foundHourAgg = true;
        } else if (fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::valueWhenMaxMin) {
            if (!foundMinOrMax) {
                missingMaxOrMinError = true;
            }
        } else if ((fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::sumOrAverageHoursShown) ||
                   (fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::maximumDuringHoursShown) ||
                   (fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::minimumDuringHoursShown)) {
            if (!foundHourAgg) {
                missingHourAggError = true;
            }
        }
    }
    if (missingMaxOrMinError) {
        ShowSevereError(state,
                        format("The Output:Table:Annual report named=\"{}\" has a valueWhenMaxMin aggregation type for a column without a previous "
                               "column that uses either the minimum or maximum aggregation types. The report will not be generated.",
                               m_name));
    }
    if (missingHourAggError) {
        ShowSevereError(state,
                        format("The Output:Table:Annual report named=\"{}\" has a --DuringHoursShown aggregation type for a column without a "
                               "previous field that uses one of the Hour-- aggregation types. The report will not be generated.",
                               m_name));
    }
    return (missingHourAggError || missingMaxOrMinError);
}

void GatherAnnualResultsForTimeStep(EnergyPlusData &state, OutputProcessor::TimeStepType kindOfTimeStep)
{
    // Jason Glazer, August 2015
    // This function is not part of the class but acts as an interface between procedural code and the class by
    // gathering data for each of the AnnualTable objects
    std::vector<AnnualTable>::iterator annualTableIt;
    auto &annualTables = state.dataOutputReportTabularAnnual->annualTables;
    for (annualTableIt = annualTables.begin(); annualTableIt != annualTables.end(); ++annualTableIt) {
        annualTableIt->gatherForTimestep(state, kindOfTimeStep);
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
                    // int oldTimeStamp = fldStIt->m_cell[row].timeStamp;
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
                        if (fldStIt->m_varAvgSum == OutputProcessor::StoreType::Sum) {                     // if it is a summed variable
                            fldStIt->m_cell[row].deferredResults.push_back(curValue /= secondsInTimeStep); // divide by time just like max and min
                        } else {
                            fldStIt->m_cell[row].deferredResults.push_back(curValue);
                        }
                        fldStIt->m_cell[row].deferredElapsed.push_back(elapsedTime); // save the amount of time for this particular value
                        newDuration = oldDuration + elapsedTime;
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
                            } else if (fldStRemainIt->m_aggregate == AnnualFieldSet::AggregationKind::valueWhenMaxMin) {
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
                            // int scanStepType = fldStRemainIt->m_varStepType;
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
                                } else if (fldStRemainIt->m_aggregate == AnnualFieldSet::AggregationKind::sumOrAverageHoursShown) {
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
                            activeHoursShown = false; // fixed CR8317
                        }
                    }
                }
            }
        }
    }
}

void ResetAnnualGathering(EnergyPlusData &state)
{
    // Jason Glazer, October 2015
    // This function is not part of the class but acts as an interface between procedural code and the class by
    // resetting data for each of the AnnualTable objects
    std::vector<AnnualTable>::iterator annualTableIt;
    auto &annualTables = state.dataOutputReportTabularAnnual->annualTables;
    for (annualTableIt = annualTables.begin(); annualTableIt != annualTables.end(); ++annualTableIt) {
        annualTableIt->resetGathering();
    }
}

void AnnualTable::resetGathering()
{
    std::vector<AnnualFieldSet>::iterator fldStIt;
    for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
        for (fldStIt = m_annualFields.begin(); fldStIt != m_annualFields.end(); ++fldStIt) {
            if (fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::maximum ||
                fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::maximumDuringHoursShown) {
                fldStIt->m_cell[row].result = -9.9e99;
            } else if (fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::minimum ||
                       fldStIt->m_aggregate == AnnualFieldSet::AggregationKind::minimumDuringHoursShown) {
                fldStIt->m_cell[row].result = 9.9e99;
            } else {
                fldStIt->m_cell[row].result = 0.0;
            }
            fldStIt->m_cell[row].duration = 0.0;
            fldStIt->m_cell[row].timeStamp = 0;
            // if any defered results
            fldStIt->m_cell[row].deferredResults.clear();
            fldStIt->m_cell[row].deferredElapsed.clear();
        }
    }
}

Real64 AnnualTable::getElapsedTime(EnergyPlusData &state, OutputProcessor::TimeStepType kindOfTimeStep)
{
    Real64 elapsedTime;
    if (kindOfTimeStep == OutputProcessor::TimeStepType::Zone) {
        elapsedTime = state.dataHVACGlobal->TimeStepSys;
    } else {
        elapsedTime = state.dataGlobal->TimeStepZone;
    }
    return elapsedTime;
}

Real64 AnnualTable::getSecondsInTimeStep(EnergyPlusData &state, OutputProcessor::TimeStepType kindOfTimeStep)
{
    Real64 secondsInTimeStep;
    if (kindOfTimeStep == OutputProcessor::TimeStepType::Zone) {
        secondsInTimeStep = state.dataHVACGlobal->TimeStepSysSec;
    } else {
        secondsInTimeStep = state.dataGlobal->TimeStepZoneSec;
    }
    return secondsInTimeStep;
}

void WriteAnnualTables(EnergyPlusData &state)
{
    auto &annualTables = state.dataOutputReportTabularAnnual->annualTables;
    for (int iUnitSystem = 0; iUnitSystem <= 1; iUnitSystem++) {
        OutputReportTabular::UnitsStyle unitsStyle_cur = state.dataOutRptTab->unitsStyle;
        bool produceTabular = true;
        bool produceSQLite = false;
        if (produceDualUnitsFlags(iUnitSystem,
                                  state.dataOutRptTab->unitsStyle,
                                  state.dataOutRptTab->unitsStyle_SQLite,
                                  unitsStyle_cur,
                                  produceTabular,
                                  produceSQLite)) {
            break;
        }

        // Jason Glazer, August 2015
        // This function is not part of the class but acts as an interface between procedural code and the class by
        // invoking the writeTable member function for each of the AnnualTable objects
        std::vector<AnnualTable>::iterator annualTableIt;
        for (annualTableIt = annualTables.begin(); annualTableIt != annualTables.end(); ++annualTableIt) {
            annualTableIt->writeTable(state, unitsStyle_cur, produceTabular, produceSQLite);
        }
    }
}

void AnnualTable::writeTable(EnergyPlusData &state, OutputReportTabular::UnitsStyle unitsStyle, bool produceTabular_para, bool produceSQLite_para)
{
    Array1D_string columnHead;
    Array1D_int columnWidth;
    Array1D_string rowHead;
    Array2D_string tableBody;
    Real64 veryLarge = 1.0E280;
    Real64 verySmall = -1.0E280;
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

    static Real64 const storedMaxVal(std::numeric_limits<Real64>::max());
    static Real64 const storedMinVal(std::numeric_limits<Real64>::lowest());

    aggString = setupAggString();
    Real64 energyUnitsConversionFactor = AnnualTable::setEnergyUnitStringAndFactor(unitsStyle, energyUnitsString);

    // Compute the columns related to the binning schemes
    computeBinColumns(state, unitsStyle);

    // Use title case names of variables if available for column headers
    columnHeadersToTitleCase(state);

    // first loop through and count how many 'columns' are defined
    // since max and min actually define two columns (the value
    // and the timestamp).
    int columnCount = 0;
    std::vector<AnnualFieldSet>::iterator fldStIt;
    for (fldStIt = m_annualFields.begin(); fldStIt != m_annualFields.end(); ++fldStIt) {
        columnCount += columnCountForAggregation(fldStIt->m_aggregate);
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
    for (fldStIt = m_annualFields.begin(); fldStIt != m_annualFields.end(); ++fldStIt) {
        std::string curAggString = aggString[(int)fldStIt->m_aggregate];
        if (curAggString.size() > 0) {
            curAggString = " {" + trim(curAggString) + '}';
        }
        // do the unit conversions
        if (unitsStyle == OutputReportTabular::UnitsStyle::InchPound || unitsStyle == OutputReportTabular::UnitsStyle::InchPoundExceptElectricity) {
            varNameWithUnits = format("{} [{}]", fldStIt->m_variMeter, Constant::unitNames[(int)fldStIt->m_varUnits]);
            OutputReportTabular::LookupSItoIP(state, varNameWithUnits, indexUnitConv, curUnits);
            OutputReportTabular::GetUnitConversion(state, indexUnitConv, curConversionFactor, curConversionOffset, curUnits);
        } else { // just do the Joule conversion
            // if units is in Joules, convert if specified
            if (fldStIt->m_varUnits == Constant::Units::J) {
                curUnits = energyUnitsString;
                curConversionFactor = energyUnitsConversionFactor;
                curConversionOffset = 0.0;
            } else { // if not joules don't perform conversion
                curUnits = Constant::unitNames[(int)fldStIt->m_varUnits];
                curConversionFactor = 1.0;
                curConversionOffset = 0.0;
            }
        }
        int curAgg = fldStIt->m_aggregate;
        columnRecount += columnCountForAggregation(fldStIt->m_aggregate);
        if ((curAgg == AnnualFieldSet::AggregationKind::sumOrAvg) || (curAgg == AnnualFieldSet::AggregationKind::sumOrAverageHoursShown)) {
            // put in the name of the variable for the column
            columnHead(columnRecount) = fldStIt->m_colHead + curAggString + " [" + curUnits + ']';
            sumVal = 0.0;
            sumDuration = 0.0;
            minVal = storedMaxVal;
            maxVal = storedMinVal;

            for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
                if (fldStIt->m_cell[row].indexesForKeyVar >= 0) {
                    if (fldStIt->m_varAvgSum == OutputProcessor::StoreType::Average) { // if it is a average variable divide by duration
                        if (fldStIt->m_cell[row].duration != 0.0) {
                            curVal = ((fldStIt->m_cell[row].result / fldStIt->m_cell[row].duration) * curConversionFactor) + curConversionOffset;
                        } else {
                            curVal = 0.0;
                        }
                        sumVal += (fldStIt->m_cell[row].result * curConversionFactor) + curConversionOffset;
                        sumDuration += fldStIt->m_cell[row].duration;
                    } else {
                        curVal = (fldStIt->m_cell[row].result * curConversionFactor) + curConversionOffset;
                        sumVal += curVal;
                    }
                    tableBody(columnRecount, row + 1) = OutputReportTabular::RealToStr(curVal, fldStIt->m_showDigits);
                    if (curVal > maxVal) {
                        maxVal = curVal;
                    }
                    if (curVal < minVal) {
                        minVal = curVal;
                    }
                } else {
                    tableBody(columnRecount, row + 1) = "-";
                }

            } // row
            // add the summary to bottom
            if (fldStIt->m_varAvgSum == OutputProcessor::StoreType::Average) { // if it is a average variable divide by duration
                if (sumDuration > 0) {
                    tableBody(columnRecount, rowSumAvg) = OutputReportTabular::RealToStr(sumVal / sumDuration, fldStIt->m_showDigits);
                } else {
                    tableBody(columnRecount, rowSumAvg) = "";
                }
            } else {
                tableBody(columnRecount, rowSumAvg) = OutputReportTabular::RealToStr(sumVal, fldStIt->m_showDigits);
            }
            if (minVal != storedMaxVal) {
                tableBody(columnRecount, rowMax) = OutputReportTabular::RealToStr(minVal, fldStIt->m_showDigits);
            }
            if (maxVal != storedMinVal) {
                tableBody(columnRecount, rowMin) = OutputReportTabular::RealToStr(maxVal, fldStIt->m_showDigits);
            }
        } else if ((curAgg == AnnualFieldSet::AggregationKind::hoursZero) || (curAgg == AnnualFieldSet::AggregationKind::hoursNonZero) ||
                   (curAgg == AnnualFieldSet::AggregationKind::hoursPositive) || (curAgg == AnnualFieldSet::AggregationKind::hoursNonPositive) ||
                   (curAgg == AnnualFieldSet::AggregationKind::hoursNegative) || (curAgg == AnnualFieldSet::AggregationKind::hoursNonNegative)) {
            // put in the name of the variable for the column
            columnHead(columnRecount) = fldStIt->m_colHead + curAggString + " [HOURS]";
            sumVal = 0.0;
            minVal = storedMaxVal;
            maxVal = storedMinVal;
            for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
                curVal = fldStIt->m_cell[row].result;
                curVal = curVal * curConversionFactor + curConversionOffset;
                tableBody(columnRecount, row + 1) = OutputReportTabular::RealToStr(curVal, fldStIt->m_showDigits);
                sumVal += curVal;
                if (curVal > maxVal) {
                    maxVal = curVal;
                }
                if (curVal < minVal) {
                    minVal = curVal;
                }
            } // row
            // add the summary to bottom
            tableBody(columnRecount, rowSumAvg) = OutputReportTabular::RealToStr(sumVal, fldStIt->m_showDigits);
            if (minVal != storedMaxVal) {
                tableBody(columnRecount, rowMax) = OutputReportTabular::RealToStr(minVal, fldStIt->m_showDigits);
            }
            if (maxVal != storedMinVal) {
                tableBody(columnRecount, rowMin) = OutputReportTabular::RealToStr(maxVal, fldStIt->m_showDigits);
            }
        } else if (curAgg == AnnualFieldSet::AggregationKind::valueWhenMaxMin) {
            if (fldStIt->m_varAvgSum == OutputProcessor::StoreType::Sum) {
                curUnits += "/s";
            }
            fixUnitsPerSecond(curUnits, curConversionFactor);
            columnHead(columnRecount) = fldStIt->m_colHead + curAggString + " [" + curUnits + ']';
            minVal = storedMaxVal;
            maxVal = storedMinVal;
            for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
                curVal = fldStIt->m_cell[row].result;
                curVal = curVal * curConversionFactor + curConversionOffset;
                tableBody(columnRecount, row + 1) = OutputReportTabular::RealToStr(curVal, fldStIt->m_showDigits);
                if (curVal > maxVal) {
                    maxVal = curVal;
                }
                if (curVal < minVal) {
                    minVal = curVal;
                }
            } // row
            // add the summary to bottom
            if (minVal != storedMaxVal) {
                tableBody(columnRecount, rowMin) = OutputReportTabular::RealToStr(minVal, fldStIt->m_showDigits);
            }
            if (maxVal != storedMinVal) {
                tableBody(columnRecount, rowMax) = OutputReportTabular::RealToStr(maxVal, fldStIt->m_showDigits);
            }
        } else if ((curAgg == AnnualFieldSet::AggregationKind::maximum) || (curAgg == AnnualFieldSet::AggregationKind::minimum) ||
                   (curAgg == AnnualFieldSet::AggregationKind::maximumDuringHoursShown) ||
                   (curAgg == AnnualFieldSet::AggregationKind::minimumDuringHoursShown)) {
            // put in the name of the variable for the column
            if (fldStIt->m_varAvgSum == OutputProcessor::StoreType::Sum) { // if it is a summed variable
                curUnits += "/s";
            }
            fixUnitsPerSecond(curUnits, curConversionFactor);
            columnHead(columnRecount - 1) = fldStIt->m_colHead + curAggString + " [" + curUnits + ']';
            columnHead(columnRecount) = fldStIt->m_colHead + " {TIMESTAMP}";
            minVal = storedMaxVal;
            maxVal = storedMinVal;
            for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
                curVal = fldStIt->m_cell[row].result;
                // CR7788 the conversion factors were causing an overflow for the InchPound case since the
                // value was very small
                // restructured the following lines to hide showing HUGE and -HUGE values in output table CR8154 Glazer
                if ((curVal < veryLarge) && (curVal > verySmall)) {
                    curVal = curVal * curConversionFactor + curConversionOffset;
                    if (curVal > maxVal) {
                        maxVal = curVal;
                    }
                    if (curVal < minVal) {
                        minVal = curVal;
                    }
                    if (curVal < veryLarge && curVal > verySmall) {
                        tableBody(columnRecount - 1, row + 1) = OutputReportTabular::RealToStr(curVal, fldStIt->m_showDigits);
                    } else {
                        tableBody(columnRecount - 1, row + 1) = "-";
                    }
                    tableBody(columnRecount, row + 1) = OutputReportTabular::DateToString(fldStIt->m_cell[row].timeStamp);
                } else {
                    tableBody(columnRecount - 1, row + 1) = "-";
                    tableBody(columnRecount, row + 1) = "-";
                }
            } // row
            // add the summary to bottom
            // Don't include if the original min and max values are still present
            if (minVal < veryLarge) {
                tableBody(columnRecount - 1, rowMin) = OutputReportTabular::RealToStr(minVal, fldStIt->m_showDigits);
            } else {
                tableBody(columnRecount - 1, rowMin) = "-";
            }
            if (maxVal > verySmall) {
                tableBody(columnRecount - 1, rowMax) = OutputReportTabular::RealToStr(maxVal, fldStIt->m_showDigits);
            } else {
                tableBody(columnRecount - 1, rowMax) = "-";
            }
        } else if (curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsMinToMax) {
            // put in the name of the variable for the column
            if (fldStIt->m_varAvgSum == OutputProcessor::StoreType::Sum) { // if it is a summed variable
                curUnits += "/s";
            }
            fixUnitsPerSecond(curUnits, curConversionFactor);
            for (int iBin = 0; iBin != 10; iBin++) {
                char binIndicator = iBin + 65;
                columnHead(columnRecount - 9 + iBin) = fldStIt->m_colHead + curAggString + " BIN " + binIndicator;
                for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
                    tableBody(columnRecount - 9 + iBin, row + 1) =
                        OutputReportTabular::RealToStr(fldStIt->m_cell[row].m_timeInBin[iBin], fldStIt->m_showDigits);
                }
                tableBody(columnRecount - 9 + iBin, rowSumAvg) =
                    OutputReportTabular::RealToStr(fldStIt->m_timeInBinTotal[iBin], fldStIt->m_showDigits);
            }
            createBinRangeTable = true;
        } else if (curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsZeroToMax) {
            // put in the name of the variable for the column
            if (fldStIt->m_varAvgSum == OutputProcessor::StoreType::Sum) { // if it is a summed variable
                curUnits += "/s";
            }
            fixUnitsPerSecond(curUnits, curConversionFactor);
            for (int iBin = 0; iBin != 10; iBin++) {
                char binIndicator = iBin + 65;
                columnHead(columnRecount - 9 + iBin) = fldStIt->m_colHead + curAggString + " BIN " + binIndicator;
                for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
                    tableBody(columnRecount - 9 + iBin, row + 1) =
                        OutputReportTabular::RealToStr(fldStIt->m_cell[row].m_timeInBin[iBin], fldStIt->m_showDigits);
                }
                tableBody(columnRecount - 9 + iBin, rowSumAvg) =
                    OutputReportTabular::RealToStr(fldStIt->m_timeInBinTotal[iBin], fldStIt->m_showDigits);
            }
            columnHead(columnRecount - 10) = fldStIt->m_colHead + curAggString + " LESS THAN BIN A";
            for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
                tableBody(columnRecount - 10, row + 1) =
                    OutputReportTabular::RealToStr(fldStIt->m_cell[row].m_timeBelowBottomBin, fldStIt->m_showDigits);
            }
            tableBody(columnRecount - 10, rowSumAvg) = OutputReportTabular::RealToStr(fldStIt->m_timeBelowBottomBinTotal, fldStIt->m_showDigits);
            createBinRangeTable = true;
        } else if (curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsMinToZero) {
            // put in the name of the variable for the column
            if (fldStIt->m_varAvgSum == OutputProcessor::StoreType::Sum) { // if it is a summed variable
                curUnits += "/s";
            }
            fixUnitsPerSecond(curUnits, curConversionFactor);
            for (int iBin = 0; iBin != 10; iBin++) {
                char binIndicator = iBin + 65;
                columnHead(columnRecount - 10 + iBin) = fldStIt->m_colHead + curAggString + " BIN " + binIndicator;
                for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
                    tableBody(columnRecount - 10 + iBin, row + 1) =
                        OutputReportTabular::RealToStr(fldStIt->m_cell[row].m_timeInBin[iBin], fldStIt->m_showDigits);
                }
                tableBody(columnRecount - 10 + iBin, rowSumAvg) =
                    OutputReportTabular::RealToStr(fldStIt->m_timeInBinTotal[iBin], fldStIt->m_showDigits);
            }
            columnHead(columnRecount) = fldStIt->m_colHead + curAggString + " MORE THAN BIN J";
            for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
                tableBody(columnRecount, row + 1) = OutputReportTabular::RealToStr(fldStIt->m_cell[row].m_timeAboveTopBin, fldStIt->m_showDigits);
            }
            tableBody(columnRecount, rowSumAvg) = OutputReportTabular::RealToStr(fldStIt->m_timeAboveTopBinTotal, fldStIt->m_showDigits);
            createBinRangeTable = true;
        } else if (curAgg == AnnualFieldSet::AggregationKind::hoursInTenPercentBins ||
                   curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusTwoStdDev ||
                   curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusThreeStdDev) {
        }
    } // fldStIt
    if (produceTabular_para) {
        OutputReportTabular::WriteReportHeaders(state, m_name, "Entire Facility", OutputProcessor::StoreType::Average);
        OutputReportTabular::WriteSubtitle(state, "Custom Annual Report");
        OutputReportTabular::WriteTable(state, tableBody, rowHead, columnHead, columnWidth, true); // transpose annual XML tables.
    }
    if (produceSQLite_para) {
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
        for (fldStIt = m_annualFields.begin(); fldStIt != m_annualFields.end(); ++fldStIt) {
            int curAgg = fldStIt->m_aggregate;
            if ((curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsMinToMax) ||
                (curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsZeroToMax) ||
                (curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsMinToZero)) {
                tableBodyRange = ""; // set entire table to blank as default
                Real64 binBottom = fldStIt->m_bottomBinValue;
                Real64 binTop = fldStIt->m_topBinValue;
                Real64 numBins = 10.;
                Real64 intervalSize = (binTop - binBottom) / numBins;

                // could not get the following to work using
                colHeadRange(1) = "BIN A";
                colHeadRange(2) = "BIN B";
                colHeadRange(3) = "BIN C";
                colHeadRange(4) = "BIN D";
                colHeadRange(5) = "BIN E";
                colHeadRange(6) = "BIN F";
                colHeadRange(7) = "BIN G";
                colHeadRange(8) = "BIN H";
                colHeadRange(9) = "BIN I";
                colHeadRange(10) = "BIN J";
                for (int iBin = 0; iBin != 10; iBin++) {
                    // colHeadRange( iBin + 1 ) = "BIN " + ( char )( iBin + 65 ); // not sure why this does not work
                    tableBodyRange(iBin + 1, 1) = OutputReportTabular::RealToStr(binBottom + float(iBin) * intervalSize, fldStIt->m_showDigits);
                    tableBodyRange(iBin + 1, 2) = OutputReportTabular::RealToStr(binBottom + float(iBin + 1) * intervalSize, fldStIt->m_showDigits);
                }
                if (produceTabular_para) {
                    OutputReportTabular::WriteSubtitle(state, "Bin Sizes for: " + fldStIt->m_colHead);
                    OutputReportTabular::WriteTable(
                        state, tableBodyRange, rowHeadRange, colHeadRange, colWidthRange, true); // transpose annual XML tables.
                }
                if (produceSQLite_para) {
                    if (state.dataSQLiteProcedures->sqlite) {
                        state.dataSQLiteProcedures->sqlite->createSQLiteTabularDataRecords(
                            tableBodyRange, rowHeadRange, colHeadRange, m_name, "Entire Facility", "Bin Sizes");
                    }
                }
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
    Real64 convFactor;
    // set the unit conversion
    if (unitsStyle == OutputReportTabular::UnitsStyle::None) {
        unitString = "J";
        convFactor = 1.0;
    } else if (unitsStyle == OutputReportTabular::UnitsStyle::JtoKWH) {
        unitString = "kWh";
        convFactor = 1.0 / 3600000.0;
    } else if (unitsStyle == OutputReportTabular::UnitsStyle::JtoMJ) {
        unitString = "MJ";
        convFactor = 1.0 / 1000000.0;
    } else if (unitsStyle == OutputReportTabular::UnitsStyle::JtoGJ) {
        unitString = "GJ";
        convFactor = 1.0 / 1000000000.0;
    } else { // Should never happen but assures compilers of initialization
        unitString = "J";
        convFactor = 1.0;
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
        ShowWarningError(state, format("Invalid aggregation type=\"{}\"  Defaulting to SumOrAverage.", inString));
    }
    return outAggType;
}

int AnnualTable::columnCountForAggregation(AnnualFieldSet::AggregationKind curAgg)
{
    int returnCount = 0;
    if (curAgg == AnnualFieldSet::AggregationKind::sumOrAvg || curAgg == AnnualFieldSet::AggregationKind::valueWhenMaxMin ||
        curAgg == AnnualFieldSet::AggregationKind::hoursZero || curAgg == AnnualFieldSet::AggregationKind::hoursNonZero ||
        curAgg == AnnualFieldSet::AggregationKind::hoursPositive || curAgg == AnnualFieldSet::AggregationKind::hoursNonPositive ||
        curAgg == AnnualFieldSet::AggregationKind::hoursNegative || curAgg == AnnualFieldSet::AggregationKind::hoursNonNegative ||
        curAgg == AnnualFieldSet::AggregationKind::sumOrAverageHoursShown || curAgg == AnnualFieldSet::AggregationKind::noAggregation) {
        returnCount = 1;
    } else if (curAgg == AnnualFieldSet::AggregationKind::maximum || curAgg == AnnualFieldSet::AggregationKind::minimum ||
               curAgg == AnnualFieldSet::AggregationKind::maximumDuringHoursShown ||
               curAgg == AnnualFieldSet::AggregationKind::minimumDuringHoursShown) {
        returnCount = 2;
    } else if (curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsMinToMax) {
        returnCount = 10;
    } else if (curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsZeroToMax ||
               curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsMinToZero) {
        returnCount = 11;
    } else if (curAgg == AnnualFieldSet::AggregationKind::hoursInTenPercentBins ||
               curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusTwoStdDev ||
               curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusThreeStdDev) {
        returnCount = 12;
    }
    return returnCount;
}

std::string AnnualTable::trim(const std::string &str)
{
    std::string whitespace = " \t";
    const size_t strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) {
        return ""; // no content
    }

    const size_t strEnd = str.find_last_not_of(whitespace);
    const size_t strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

void AddAnnualTableOfContents(EnergyPlusData &state, std::ostream &nameOfStream)
{
    // Jason Glazer, August 2015
    // This function is not part of the class but acts as an interface between procedural code and the class by
    // invoking the writeTable member function for each of the AnnualTable objects
    std::vector<AnnualTable>::iterator annualTableIt;
    auto &annualTables = state.dataOutputReportTabularAnnual->annualTables;
    for (annualTableIt = annualTables.begin(); annualTableIt != annualTables.end(); ++annualTableIt) {
        annualTableIt->addTableOfContents(nameOfStream);
    }
}

void AnnualTable::addTableOfContents(std::ostream &nameOfStream)
{
    nameOfStream << "<p><b>" << m_name << "</b></p> |\n";
    nameOfStream << "<a href=\"#" << OutputReportTabular::MakeAnchorName(m_name, "Entire Facility") << "\">" << "Entire Facility" << "</a>    |   \n";
}

void AnnualTable::computeBinColumns(EnergyPlusData &state, OutputReportTabular::UnitsStyle const unitsStyle_para)
{
    std::vector<AnnualFieldSet>::iterator fldStIt;
    Real64 constexpr veryLarge = 1.0E280;
    Real64 constexpr verySmall = -1.0E280;
    for (fldStIt = m_annualFields.begin(); fldStIt != m_annualFields.end(); ++fldStIt) {
        int curAgg = fldStIt->m_aggregate;
        // for columns with binning aggregation types compute the statistics
        if (curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsMinToMax || curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsZeroToMax ||
            curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsMinToZero || curAgg == AnnualFieldSet::AggregationKind::hoursInTenPercentBins ||
            curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusTwoStdDev ||
            curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusThreeStdDev) {
            // the size the deferred vectors should be same for all rows
            if (allRowsSameSizeDefferedVectors(fldStIt)) {
                convertUnitForDeferredResults(state, fldStIt, unitsStyle_para);
                std::vector<Real64> deferredTotalForColumn;
                Real64 minVal = veryLarge;
                Real64 maxVal = verySmall;
                Real64 sum = 0;
                Real64 curVal = 0.0;
                for (unsigned int jDefRes = 0; jDefRes != fldStIt->m_cell[0].deferredResults.size(); jDefRes++) {
                    sum = 0;
                    for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
                        curVal = fldStIt->m_cell[row].deferredResults[jDefRes];
                        sum += curVal;
                        if (curVal > maxVal) {
                            maxVal = curVal;
                        }
                        if (curVal < minVal) {
                            minVal = curVal;
                        }
                    }
                    deferredTotalForColumn.push_back(sum / float(m_objectNames.size())); // put average value into the total row
                }
                if (curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsMinToMax) {
                    fldStIt->m_topBinValue = maxVal;
                    fldStIt->m_bottomBinValue = minVal;
                } else if (curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsZeroToMax) {
                    fldStIt->m_topBinValue = maxVal;
                    fldStIt->m_bottomBinValue = 0.0;
                } else if (curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsMinToZero) {
                    fldStIt->m_topBinValue = 0.0;
                    fldStIt->m_bottomBinValue = minVal;
                } else if (curAgg == AnnualFieldSet::AggregationKind::hoursInTenPercentBins) {
                    fldStIt->m_topBinValue = 1.0;
                    fldStIt->m_bottomBinValue = 0.0;
                } else if (curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusTwoStdDev) {
                } else if (curAgg == AnnualFieldSet::AggregationKind::hoursInTenBinsPlusMinusThreeStdDev) {
                }
                // compute the actual amount of time spent in each bin and above and below
                for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
                    fldStIt->m_cell[row].m_timeInBin = calculateBins(10,
                                                                     fldStIt->m_cell[row].deferredResults,
                                                                     fldStIt->m_cell[row].deferredElapsed,
                                                                     fldStIt->m_topBinValue,
                                                                     fldStIt->m_bottomBinValue,
                                                                     fldStIt->m_cell[row].m_timeAboveTopBin,
                                                                     fldStIt->m_cell[row].m_timeBelowBottomBin);
                }
                // do the total row binning
                fldStIt->m_timeInBinTotal = calculateBins(10,
                                                          deferredTotalForColumn,
                                                          fldStIt->m_cell[0].deferredElapsed,
                                                          fldStIt->m_topBinValue,
                                                          fldStIt->m_bottomBinValue,
                                                          fldStIt->m_timeAboveTopBinTotal,
                                                          fldStIt->m_timeBelowBottomBinTotal);
            }
        }
    }
}

bool AnnualTable::allRowsSameSizeDefferedVectors(std::vector<AnnualFieldSet>::iterator fldStIt)
{
    bool returnFlag = true;
    unsigned int sizeOfDeferred = 0;
    for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
        if (sizeOfDeferred == 0) {
            sizeOfDeferred = fldStIt->m_cell[row].deferredResults.size();
        } else {
            if (fldStIt->m_cell[row].deferredResults.size() != sizeOfDeferred) {
                returnFlag = false;
                return returnFlag;
            }
        }
    }
    return returnFlag;
}

void AnnualTable::convertUnitForDeferredResults(EnergyPlusData &state,
                                                std::vector<AnnualFieldSet>::iterator fldStIt,
                                                OutputReportTabular::UnitsStyle const unitsStyle)
{
    Real64 curConversionFactor;
    Real64 curConversionOffset;
    std::string curUnits;
    std::string energyUnitsString;
    Real64 curSI;
    Real64 curIP;
    Real64 energyUnitsConversionFactor = AnnualTable::setEnergyUnitStringAndFactor(unitsStyle, energyUnitsString);
    // do the unit conversions
    if (unitsStyle == OutputReportTabular::UnitsStyle::InchPound || unitsStyle == OutputReportTabular::UnitsStyle::InchPoundExceptElectricity) {
        int indexUnitConv;
        std::string varNameWithUnits = format("{} [{}]", fldStIt->m_variMeter, Constant::unitNames[(int)fldStIt->m_varUnits]);
        OutputReportTabular::LookupSItoIP(state, varNameWithUnits, indexUnitConv, curUnits);
        OutputReportTabular::GetUnitConversion(state, indexUnitConv, curConversionFactor, curConversionOffset, curUnits);
    } else { // just do the Joule conversion
        // if units is in Joules, convert if specified
        if (fldStIt->m_varUnits == Constant::Units::J) {
            curUnits = energyUnitsString;
            curConversionFactor = energyUnitsConversionFactor;
            curConversionOffset = 0.0;
        } else { // if not joules don't perform conversion
            curUnits = Constant::unitNames[(int)fldStIt->m_varUnits];
            curConversionFactor = 1.0;
            curConversionOffset = 0.0;
        }
    }
    if (fldStIt->m_varAvgSum == OutputProcessor::StoreType::Sum) {
        curUnits += "/s";
    }
    fixUnitsPerSecond(curUnits, curConversionFactor);
    if (curConversionFactor != 1.0 || curConversionOffset != 0.0) {
        for (unsigned int row = 0; row != m_objectNames.size(); row++) { // loop through by row.
            for (unsigned int jDefRes = 0; jDefRes != fldStIt->m_cell[0].deferredResults.size(); jDefRes++) {
                curSI = fldStIt->m_cell[row].deferredResults[jDefRes];
                curIP = curSI * curConversionFactor + curConversionOffset;
                fldStIt->m_cell[row].deferredResults[jDefRes] = curIP;
            }
        }
    }
}

std::vector<Real64> AnnualTable::calculateBins(int const numberOfBins,
                                               std::vector<Real64> valuesToBin,
                                               std::vector<Real64> corrElapsedTime,
                                               Real64 const topOfBins,
                                               Real64 const bottomOfBins,
                                               Real64 &timeAboveTopBin,
                                               Real64 &timeBelowBottomBin)
{
    std::vector<Real64> returnBins(0.0);
    returnBins.resize(numberOfBins);
    Real64 intervalSize = (topOfBins - bottomOfBins) / float(numberOfBins);
    timeAboveTopBin = 0.0;
    timeBelowBottomBin = 0.0;
    std::vector<Real64>::iterator elapsedTimeIt;
    elapsedTimeIt = corrElapsedTime.begin();
    std::vector<Real64>::iterator valueIt;
    for (valueIt = valuesToBin.begin(); valueIt != valuesToBin.end(); ++valueIt) {
        if (*valueIt < bottomOfBins) {
            timeBelowBottomBin += *elapsedTimeIt;
        } else if (*valueIt >= topOfBins) {
            timeAboveTopBin += *elapsedTimeIt;
        } else {
            // determine which bin the results are in
            int binNum = int((*valueIt - bottomOfBins) / intervalSize);
            if (binNum < numberOfBins && binNum >= 0) {
                returnBins[binNum] += *elapsedTimeIt;
            }
        }
        ++elapsedTimeIt;
    }
    return returnBins;
}

void AnnualTable::columnHeadersToTitleCase(EnergyPlusData &state)
{
    std::vector<AnnualFieldSet>::iterator fldStIt;
    for (fldStIt = m_annualFields.begin(); fldStIt != m_annualFields.end(); ++fldStIt) {
        if (fldStIt->m_variMeter == fldStIt->m_colHead) {
            if (fldStIt->m_indexesForKeyVar.size() > 0) {
                int varNum = fldStIt->m_indexesForKeyVar[0];
                if (fldStIt->m_typeOfVar == OutputProcessor::VariableType::Real) {
                    fldStIt->m_colHead = state.dataOutputProcessor->outVars[varNum]->name;
                } else if (fldStIt->m_typeOfVar == OutputProcessor::VariableType::Meter) {
                    fldStIt->m_colHead = state.dataOutputProcessor->meters[varNum]->Name;
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

std::vector<std::string> AnnualTable::inspectTable()
{
    // added function just to inspect the main private AnnualTable members because no other
    // interface to the AnnualTable class is output oriented except writeTable and that is very complex.
    std::vector<std::string> ret;
    ret.push_back(m_name);
    ret.push_back(m_filter);
    ret.push_back(m_sched->Name);
    return ret;
}

std::vector<std::string> AnnualTable::inspectTableFieldSets(int fldIndex)
{
    // added function just to inspect the private field set members of AnnualTable because no other
    // interface to the AnnualTable class is output oriented except writeTable and that is very complex.
    std::vector<std::string> ret;
    AnnualFieldSet fldSt;
    fldSt = m_annualFields[fldIndex];
    ret.push_back(fldSt.m_colHead);
    ret.push_back(fldSt.m_variMeter);
    ret.push_back(std::string(Constant::unitNames[(int)fldSt.m_varUnits]));
    std::string outStr = std::to_string(fldSt.m_showDigits);
    // ints
    ret.push_back(outStr);
    outStr = std::to_string((int)fldSt.m_typeOfVar);
    ret.push_back(outStr);
    outStr = std::to_string(fldSt.m_keyCount);
    ret.push_back(outStr);
    outStr = std::to_string((int)fldSt.m_varAvgSum);
    ret.push_back(outStr);
    outStr = std::to_string((int)fldSt.m_varStepType);
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
    if (fldSt.m_cell.size() > 0) {
        outStr = std::to_string(fldSt.m_cell[0].result);
        ret.push_back(outStr);
    }
    return ret;
}

} // namespace EnergyPlus::OutputReportTabularAnnual
