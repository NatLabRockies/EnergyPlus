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
#include <cassert>
#include <cmath>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <milo/dtoa.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Reference.fwd.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/ResultsFramework.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace ResultsFramework {

    using namespace OutputProcessor;

    // trim string
    std::string trim(std::string_view const s)
    {
        if (s.empty()) {
            return std::string{};
        }
        auto const first = s.find_first_not_of(' ');
        auto const last = s.find_last_not_of(' ');
        if ((first == std::string::npos) || (last == std::string::npos)) {
            return std::string{};
        } else {
            return std::string{s.substr(first, last - first + 1)};
        }
    }

    // Class SimInfo
    void SimInfo::setProgramVersion(const std::string &programVersion)
    {
        ProgramVersion = programVersion;
    }

    std::string SimInfo::getProgramVersion() const
    {
        return ProgramVersion;
    }

    void SimInfo::setSimulationEnvironment(const std::string &simulationEnvironment)
    {
        SimulationEnvironment = simulationEnvironment;
    }

    void SimInfo::setInputModelURI(const std::string &inputModelURI)
    {
        InputModelURI = inputModelURI;
    }

    void SimInfo::setStartDateTimeStamp(const std::string &startDateTimeStamp)
    {
        StartDateTimeStamp = startDateTimeStamp;
    }

    void SimInfo::setRunTime(const std::string &elapsedTime)
    {
        RunTime = elapsedTime;
    }

    void SimInfo::setNumErrorsWarmup(const std::string &numWarningsDuringWarmup, const std::string &numSevereDuringWarmup)
    {
        NumWarningsDuringWarmup = numWarningsDuringWarmup;
        NumSevereDuringWarmup = numSevereDuringWarmup;
    }

    void SimInfo::setNumErrorsSizing(const std::string &numWarningsDuringSizing, const std::string &numSevereDuringSizing)
    {
        NumWarningsDuringSizing = numWarningsDuringSizing;
        NumSevereDuringSizing = numSevereDuringSizing;
    }

    void SimInfo::setNumErrorsSummary(const std::string &numWarnings, const std::string &numSevere)
    {
        NumWarnings = numWarnings;
        NumSevere = numSevere;
    }

    json SimInfo::getJSON() const
    {
        json root = {{"ProgramVersion", ProgramVersion},
                     {"SimulationEnvironment", SimulationEnvironment},
                     {"InputModelURI", InputModelURI},
                     {"StartDateTimeStamp", StartDateTimeStamp},
                     {"RunTime", RunTime},
                     {"ErrorSummary", {{"NumWarnings", NumWarnings}, {"NumSevere", NumSevere}}},
                     {"ErrorSummaryWarmup", {{"NumWarnings", NumWarningsDuringWarmup}, {"NumSevere", NumSevereDuringWarmup}}},
                     {"ErrorSummarySizing", {{"NumWarnings", NumWarningsDuringSizing}, {"NumSevere", NumSevereDuringSizing}}}};
        return root;
    }

    // Class Variable
    Variable::Variable(const std::string &VarName,
                       const OutputProcessor::ReportFreq reportFrequency,
                       const OutputProcessor::TimeStepType timeStepType,
                       const int ReportID,
                       const Constant::Units units)
        : m_varName(VarName), m_reportFreq(reportFrequency), m_timeStepType(timeStepType), m_rptID(ReportID), m_units(units)
    {
    }

    Variable::Variable(const std::string &VarName,
                       const OutputProcessor::ReportFreq reportFrequency,
                       const OutputProcessor::TimeStepType timeStepType,
                       const int ReportID,
                       const Constant::Units units,
                       const std::string &customUnits)
        : m_varName(VarName), m_reportFreq(reportFrequency), m_timeStepType(timeStepType), m_rptID(ReportID), m_units(units),
          m_customUnits(customUnits)
    {
    }

    std::string Variable::variableName() const
    {
        return m_varName;
    }

    void Variable::setVariableName(const std::string &VarName)
    {
        m_varName = VarName;
    }

    std::string Variable::sReportFrequency() const
    {
        static constexpr std::array<std::string_view, (int)ReportFreq::Num> reportFreqStrings = {
            "Detailed", "TimeStep", "Hourly", "Daily", "Monthly", "RunPeriod", "Yearly"};

        static constexpr std::array<std::string_view, (int)TimeStepType::Num> timeStepTypeStrings = {"Detailed - Zone", "Detailed - HVAC"};

        return (m_reportFreq == ReportFreq::EachCall) ? std::string(timeStepTypeStrings[(int)m_timeStepType])
                                                      : std::string(reportFreqStrings[(int)m_reportFreq]);
    }

    OutputProcessor::ReportFreq Variable::iReportFrequency() const
    {
        return m_reportFreq;
    }

    void Variable::setReportFrequency(const OutputProcessor::ReportFreq reportFrequency)
    {
        m_reportFreq = reportFrequency;
    }

    OutputProcessor::TimeStepType Variable::timeStepType() const
    {
        return m_timeStepType;
    }

    void Variable::setTimeStepType(const OutputProcessor::TimeStepType timeStepType)
    {
        m_timeStepType = timeStepType;
    }

    int Variable::reportID() const
    {
        return m_rptID;
    }

    void Variable::setReportID(int Id)
    {
        m_rptID = Id;
    }

    Constant::Units Variable::units() const
    {
        return m_units;
    }

    void Variable::setUnits(const Constant::Units units)
    {
        m_units = units;
    }

    std::string Variable::customUnits() const
    {
        return m_customUnits;
    }

    void Variable::setCustomUnits(const std::string &customUnits)
    {
        m_customUnits = customUnits;
    }

    void Variable::pushValue(const double val)
    {
        m_values.push_back(val);
    }

    double Variable::value(size_t index) const
    {
        return m_values.at(index);
    }

    size_t Variable::numValues() const
    {
        return m_values.size();
    }

    json Variable::getJSON() const
    {
        json root;
        if (m_customUnits.empty()) {
            root = {{"Name", m_varName}, {"Units", Constant::unitNames[(int)m_units]}, {"Frequency", sReportFrequency()}};
        } else {
            root = {{"Name", m_varName}, {"Units", m_customUnits}, {"Frequency", sReportFrequency()}};
        }
        return root;
    }

    // Class OutputVariable
    OutputVariable::OutputVariable(const std::string &VarName,
                                   const OutputProcessor::ReportFreq reportFrequency,
                                   const OutputProcessor::TimeStepType timeStepType,
                                   const int ReportID,
                                   const Constant::Units units)
        : Variable(VarName, reportFrequency, timeStepType, ReportID, units)
    {
    }

    OutputVariable::OutputVariable(const std::string &VarName,
                                   const OutputProcessor::ReportFreq reportFrequency,
                                   const OutputProcessor::TimeStepType timeStepType,
                                   const int ReportID,
                                   const Constant::Units units,
                                   const std::string &customUnits)
        : Variable(VarName, reportFrequency, timeStepType, ReportID, units, customUnits)
    {
    }

    // Class MeterVariable
    MeterVariable::MeterVariable(const std::string &VarName,
                                 const OutputProcessor::ReportFreq reportFrequency,
                                 const int ReportID,
                                 const Constant::Units units,
                                 const bool MeterOnly,
                                 const bool Accumulative)
        : Variable(VarName, reportFrequency, OutputProcessor::TimeStepType::Zone, ReportID, units)
    {
        acc = Accumulative;
        meter_only = MeterOnly;
    }

    bool MeterVariable::accumulative() const
    {
        return acc;
    }

    void MeterVariable::setAccumulative(bool state)
    {
        acc = state;
    }

    bool MeterVariable::meterOnly() const
    {
        return meter_only;
    }

    void MeterVariable::setMeterOnly(bool state)
    {
        meter_only = state;
    }

    json MeterVariable::getJSON() const
    {
        json root = Variable::getJSON();
        if (acc) {
            root["Cumulative"] = true;
        }
        //        if (meter_only) {
        //            root["MeterOnly"] = true;
        //        }
        return root;
    }

    // class DataFrame
    DataFrame::DataFrame(const std::string &ReportFreq)
    {
        ReportFrequency = ReportFreq;
    }

    void DataFrame::addVariable(Variable const &var)
    {
        lastVarID = var.reportID();
        variableMap.emplace(lastVarID, var);
    }

    Variable &DataFrame::lastVariable()
    {
        return variableMap.at(lastVarID);
    }

    void DataFrame::newRow(const int month, const int dayOfMonth, int hourOfDay, int curMin, int calendarYear)
    {
        if (curMin > 0) {
            hourOfDay -= 1;
        }
        if (curMin == 60) {
            curMin = 0;
            hourOfDay += 1;
        }

        if (beginningOfInterval) {
            if (hourOfDay == 24) {
                hourOfDay = 0;
            }
            std::swap(hourOfDay, lastHour);
            std::swap(curMin, lastMinute);
        }
        // future start of ISO 8601 datetime output
        // fmt::format("YYYY-{:02d}/{:02d}T{:02d}:{:02d}:00", month, dayOfMonth, hourOfDay, curMin);
        // fmt::format("{:02d}/{:02d} {:02d}:{:02d}:00", month, dayOfMonth, hourOfDay, curMin);
        if (iso8601) {
            TS.emplace_back(fmt::format("{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:00", calendarYear, month, dayOfMonth, hourOfDay, curMin));
        } else {
            TS.emplace_back(fmt::format("{:02d}/{:02d} {:02d}:{:02d}:00", month, dayOfMonth, hourOfDay, curMin));
        }
    }

    //    void DataFrame::newRow(const std::string &ts)
    //    {
    //        TS.emplace_back(ts);
    //    }

    bool DataFrame::dataFrameEnabled() const
    {
        return DataFrameEnabled;
    }

    void DataFrame::setDataFrameEnabled(bool state)
    {
        DataFrameEnabled = state;
    }

    void DataFrame::setVariablesScanned(bool state)
    {
        VariablesScanned = state;
    }

    bool DataFrame::variablesScanned() const
    {
        return VariablesScanned;
    }

    void DataFrame::pushVariableValue(const int reportID, double value)
    {
        variableMap[reportID].pushValue(value);
    }

    json DataFrame::getVariablesJSON()
    {
        json arr = json::array();
        for (auto const &varMap : variableMap) {
            arr.push_back(varMap.second.getJSON());
        }
        return arr;
    }

    json DataFrame::getJSON() const
    {
        json root;
        json cols = json::array();
        json rows = json::array();

        for (auto const &varMap : variableMap) {
            if (varMap.second.customUnits().empty()) {
                cols.push_back({{"Variable", varMap.second.variableName()}, {"Units", Constant::unitNames[(int)varMap.second.units()]}});
            } else {
                cols.push_back({{"Variable", varMap.second.variableName()}, {"Units", varMap.second.customUnits()}});
            }
        }

        json vals = json::array();

        for (size_t row = 0; row < TS.size(); ++row) {
            vals.clear();

            for (auto const &varMap : variableMap) {
                if (row < varMap.second.numValues()) {
                    vals.push_back(varMap.second.value(row));
                } else {
                    vals.push_back(nullptr);
                }
            }
            rows.push_back({{TS.at(row), vals}});
        }
        root = {{"ReportFrequency", ReportFrequency}, {"Cols", cols}, {"Rows", rows}};
        return root;
    }

    void MeterDataFrame::addVariable(MeterVariable const &var)
    {
        lastVarID = var.reportID();
        meterMap.emplace(lastVarID, var);
    }

    void MeterDataFrame::pushVariableValue(const int reportID, double value)
    {
        meterMap[reportID].pushValue(value);
    }

    json MeterDataFrame::getJSON(bool meterOnlyCheck) const
    {
        json root;
        json cols = json::array();
        json rows = json::array();

        for (auto const &varMap : meterMap) {
            if (!(meterOnlyCheck && varMap.second.meterOnly())) {
                cols.push_back({{"Variable", varMap.second.variableName()}, {"Units", Constant::unitNames[(int)varMap.second.units()]}});
            }
        }

        if (cols.empty()) {
            return root;
        }

        json vals = json::array();

        for (size_t row = 0; row < TS.size(); ++row) {
            vals.clear();

            for (auto const &varMap : meterMap) {
                if (!(meterOnlyCheck && varMap.second.meterOnly())) {
                    if (row < varMap.second.numValues()) {
                        vals.push_back(varMap.second.value(row));
                    } else {
                        vals.push_back(nullptr);
                    }
                }
            }
            rows.push_back({{TS.at(row), vals}});
        }
        root = {{"ReportFrequency", ReportFrequency}, {"Cols", cols}, {"Rows", rows}};
        return root;
    }

    void DataFrame::writeReport(JsonOutputFilePaths &jsonOutputFilePaths, bool outputJSON, bool outputCBOR, bool outputMsgPack)
    {

        json root = getJSON();
        if (ReportFrequency == "Detailed-HVAC") {
            if (outputJSON) {
                FileSystem::writeFile<FileSystem::FileTypes::JSON>(jsonOutputFilePaths.outputTSHvacJsonFilePath, root);
            }
            if (outputCBOR) {
                FileSystem::writeFile<FileSystem::FileTypes::CBOR>(jsonOutputFilePaths.outputTSHvacCborFilePath, root);
            }
            if (outputMsgPack) {
                FileSystem::writeFile<FileSystem::FileTypes::MsgPack>(jsonOutputFilePaths.outputTSHvacMsgPackFilePath, root);
            }
        } else if (ReportFrequency == "Detailed-Zone") {
            if (outputJSON) {
                FileSystem::writeFile<FileSystem::FileTypes::JSON>(jsonOutputFilePaths.outputTSZoneJsonFilePath, root);
            }
            if (outputCBOR) {
                FileSystem::writeFile<FileSystem::FileTypes::CBOR>(jsonOutputFilePaths.outputTSZoneCborFilePath, root);
            }
            if (outputMsgPack) {
                FileSystem::writeFile<FileSystem::FileTypes::MsgPack>(jsonOutputFilePaths.outputTSZoneMsgPackFilePath, root);
            }
        } else if (ReportFrequency == "TimeStep") {
            if (outputJSON) {
                FileSystem::writeFile<FileSystem::FileTypes::JSON>(jsonOutputFilePaths.outputTSJsonFilePath, root);
            }
            if (outputCBOR) {
                FileSystem::writeFile<FileSystem::FileTypes::CBOR>(jsonOutputFilePaths.outputTSCborFilePath, root);
            }
            if (outputMsgPack) {
                FileSystem::writeFile<FileSystem::FileTypes::MsgPack>(jsonOutputFilePaths.outputTSMsgPackFilePath, root);
            }
        } else if (ReportFrequency == "Daily") {
            if (outputJSON) {
                FileSystem::writeFile<FileSystem::FileTypes::JSON>(jsonOutputFilePaths.outputDYJsonFilePath, root);
            }
            if (outputCBOR) {
                FileSystem::writeFile<FileSystem::FileTypes::CBOR>(jsonOutputFilePaths.outputDYCborFilePath, root);
            }
            if (outputMsgPack) {
                FileSystem::writeFile<FileSystem::FileTypes::MsgPack>(jsonOutputFilePaths.outputDYMsgPackFilePath, root);
            }
        } else if (ReportFrequency == "Hourly") {
            if (outputJSON) {
                FileSystem::writeFile<FileSystem::FileTypes::JSON>(jsonOutputFilePaths.outputHRJsonFilePath, root);
            }
            if (outputCBOR) {
                FileSystem::writeFile<FileSystem::FileTypes::CBOR>(jsonOutputFilePaths.outputHRCborFilePath, root);
            }
            if (outputMsgPack) {
                FileSystem::writeFile<FileSystem::FileTypes::MsgPack>(jsonOutputFilePaths.outputHRMsgPackFilePath, root);
            }
        } else if (ReportFrequency == "Monthly") {
            if (outputJSON) {
                FileSystem::writeFile<FileSystem::FileTypes::JSON>(jsonOutputFilePaths.outputMNJsonFilePath, root);
            }
            if (outputCBOR) {
                FileSystem::writeFile<FileSystem::FileTypes::CBOR>(jsonOutputFilePaths.outputMNCborFilePath, root);
            }
            if (outputMsgPack) {
                FileSystem::writeFile<FileSystem::FileTypes::MsgPack>(jsonOutputFilePaths.outputMNMsgPackFilePath, root);
            }
        } else if (ReportFrequency == "RunPeriod") {
            if (outputJSON) {
                FileSystem::writeFile<FileSystem::FileTypes::JSON>(jsonOutputFilePaths.outputSMJsonFilePath, root);
            }
            if (outputCBOR) {
                FileSystem::writeFile<FileSystem::FileTypes::CBOR>(jsonOutputFilePaths.outputSMCborFilePath, root);
            }
            if (outputMsgPack) {
                FileSystem::writeFile<FileSystem::FileTypes::MsgPack>(jsonOutputFilePaths.outputSMMsgPackFilePath, root);
            }
        } else if (ReportFrequency == "Yearly") {
            if (outputJSON) {
                FileSystem::writeFile<FileSystem::FileTypes::JSON>(jsonOutputFilePaths.outputYRJsonFilePath, root);
            }
            if (outputCBOR) {
                FileSystem::writeFile<FileSystem::FileTypes::CBOR>(jsonOutputFilePaths.outputYRCborFilePath, root);
            }
            if (outputMsgPack) {
                FileSystem::writeFile<FileSystem::FileTypes::MsgPack>(jsonOutputFilePaths.outputYRMsgPackFilePath, root);
            }
        }
    }
    // class Table

    Table::Table(Array2D_string const &body,
                 Array1D_string const &rowLabels,
                 Array1D_string const &columnLabels,
                 std::string const &tableName,
                 std::string const &footnoteText)
    {

        size_t sizeColumnLabels = columnLabels.size();
        size_t sizeRowLabels = rowLabels.size();
        TableName = tableName;
        FootnoteText = footnoteText;

        for (size_t iCol = 0, k = body.index(1, 1); iCol < sizeColumnLabels; ++iCol) {
            ColHeaders.push_back(columnLabels[iCol]);
            std::vector<std::string> col;
            for (size_t iRow = 0; iRow < sizeRowLabels; ++iRow) {
                if (iCol == 0) {
                    // do this once only
                    RowHeaders.push_back(rowLabels[iRow]);
                }
                col.push_back(trim(body[k]));
                ++k;
            }
            Data.push_back(col);
        }
    }

    json Table::getJSON() const
    {
        json root;
        json cols = json::array();
        json rows;

        for (size_t col = 0; col < ColHeaders.size(); ++col) {
            cols.push_back(ColHeaders[col]);
        }

        for (size_t row = 0; row < RowHeaders.size(); ++row) {
            json rowvec = json::array();
            for (size_t col = 0; col < ColHeaders.size(); ++col) {
                rowvec.push_back(Data[col][row]);
            }
            rows[RowHeaders[row]] = rowvec;
        }

        root = {{"TableName", TableName}, {"Cols", cols}, {"Rows", rows}};

        if (!FootnoteText.empty()) {
            root["Footnote"] = FootnoteText;
        }
        return root;
    }

    // class Report

    json Report::getJSON() const
    {

        json root = {{"ReportName", ReportName}, {"For", ReportForString}};

        json cols = json::array();

        for (auto const &table : Tables) {
            cols.push_back(table.getJSON());
        }

        root["Tables"] = cols;
        return root;
    }

    // class ReportsCollection
    ReportsCollection::ReportsCollection()
    {
    }

    void ReportsCollection::addReportTable(Array2D_string const &body,
                                           Array1D_string const &rowLabels,
                                           Array1D_string const &columnLabels,
                                           std::string const &reportName,
                                           std::string const &reportForString,
                                           std::string const &tableName)
    {
        addReportTable(body, rowLabels, columnLabels, reportName, reportForString, tableName, "");
    }

    void ReportsCollection::addReportTable(Array2D_string const &body,
                                           Array1D_string const &rowLabels,
                                           Array1D_string const &columnLabels,
                                           std::string const &reportName,
                                           std::string const &reportForString,
                                           std::string const &tableName,
                                           std::string const &footnoteText)
    {
        std::string const key = reportName + reportForString;
        // Report *r;
        Table tbl(body, rowLabels, columnLabels, tableName, footnoteText);

        auto search = reportsMap.find(key);
        if (search != reportsMap.end()) {
            // r = search->second;
            search->second.Tables.push_back(tbl);
        } else {
            // r = new Report();
            Report r;
            r.ReportName = reportName;
            r.ReportForString = reportForString;
            r.Tables.push_back(tbl);
            reportsMap.emplace(key, r);
        }

        // Table *tbl = new Table( body, rowLabels, columnLabels, tableName, footnoteText );
        // r->Tables.push_back( tbl );
    }

    json ReportsCollection::getJSON() const
    {
        json root = json::array();

        for (auto const &iter : reportsMap) {
            root.push_back(iter.second.getJSON());
        }
        return root;
    }

    void CSVWriter::parseTSOutputs(EnergyPlusData &state,
                                   json const &data,
                                   std::vector<std::string> const &outputVariables,
                                   OutputProcessor::ReportFreq reportingFrequency)
    {
        if (data.empty()) {
            return;
        }
        updateReportFreq(reportingFrequency);
        std::vector<int> indices;

        std::string reportFrequency = data.at("ReportFrequency").get<std::string>();
        if (reportFrequency == "Detailed-HVAC" || reportFrequency == "Detailed-Zone") {
            reportFrequency = "Each Call";
        }
        auto const &columns = data.at("Cols");
        for (auto const &column : columns) {
            std::string search_string =
                fmt::format("{0} [{1}]({2})", column.at("Variable").get<std::string>(), column.at("Units").get<std::string>(), reportFrequency);
            auto found = std::find(outputVariables.begin(), outputVariables.end(), search_string);
            if (found == outputVariables.end()) {
                search_string =
                    fmt::format("{0} [{1}]({2})", column.at("Variable").get<std::string>(), column.at("Units").get<std::string>(), "Each Call");
                found = std::find(outputVariables.begin(), outputVariables.end(), search_string);
            }
            if (found == outputVariables.end()) {
                ShowFatalError(state, fmt::format("Output variable ({0}) not found output variable list", search_string));
            }
            outputVariableIndices[std::distance(outputVariables.begin(), found)] = true;
            indices.emplace_back(std::distance(outputVariables.begin(), found));
        }

        auto const &rows = data.at("Rows");
        for (auto const &row : rows) {
            for (auto &el : row.items()) {
                auto found_key = outputs.find(el.key());
                if (found_key == outputs.end()) {
                    std::vector<std::string> output(outputVariables.size());
                    int i = 0;
                    for (auto const &col : el.value()) {
                        if (col.is_null()) {
                            output[indices[i]] = "";
                        } else {
                            dtoa(col.get<double>(), s);
                            output[indices[i]] = s;
                        }
                        ++i;
                    }
                    outputs[el.key()] = output;
                } else {
                    int i = 0;
                    for (auto const &col : el.value()) {
                        if (col.is_null()) {
                            found_key->second[indices[i]] = "";
                        } else {
                            dtoa(col.get<double>(), s);
                            found_key->second[indices[i]] = s;
                        }
                        ++i;
                    }
                }
            }
        }
    }

    void CSVWriter::updateReportFreq(OutputProcessor::ReportFreq reportingFrequency)
    {
        if (reportingFrequency < smallestReportFreq) {
            smallestReportFreq = reportingFrequency;
        }
    }

    std::string &CSVWriter::convertToMonth(std::string &datetime)
    {
        // if running this function, there should only ever be 12 + design days values to change
        static const std::map<std::string, std::string> months({{"01", "January"},
                                                                {"02", "February"},
                                                                {"03", "March"},
                                                                {"04", "April"},
                                                                {"05", "May"},
                                                                {"06", "June"},
                                                                {"07", "July"},
                                                                {"08", "August"},
                                                                {"09", "September"},
                                                                {"10", "October"},
                                                                {"11", "November"},
                                                                {"12", "December"}});
        // 01/01 24:00:00
        std::string const month = datetime.substr(0, 2);
        size_t const pos = datetime.find(' ');
        std::string time;
        if (pos != std::string::npos) {
            time = datetime.substr(pos);
        }
        // This assert replaces ShowFatalError(state, "Monthly output variables should occur at the end of the day.");
        assert(time == " 24:00:00" || time == " 00:00:00");

        datetime = months.find(month)->second;
        return datetime;
    }

    void CSVWriter::writeOutput(EnergyPlusData &state,
                                std::vector<std::string> const &outputVariables,
                                InputOutputFile &outputFile,
                                bool outputControl,
                                bool rewriteTimestamp)
    {
        outputFile.ensure_open(state, "OpenOutputFiles", outputControl);

        print<FormatSyntax::FMT>(outputFile, "{}", "Date/Time,");
        std::string sep;
        for (auto it = outputVariables.begin(); it != outputVariables.end(); ++it) {
            if (!outputVariableIndices[std::distance(outputVariables.begin(), it)]) {
                continue;
            }
            print<FormatSyntax::FMT>(outputFile, "{}{}", sep, *it);
            if (sep.empty()) {
                sep = ",";
            }
        }
        print<FormatSyntax::FMT>(outputFile, "{}", '\n');

        for (auto &item : outputs) {
            std::string datetime = item.first;
            if (rewriteTimestamp) {
                if (smallestReportFreq < OutputProcessor::ReportFreq::Month) {
                    datetime = datetime.replace(datetime.find(' '), 1, "  ");
                } else {
                    convertToMonth(datetime);
                }
            }
            print<FormatSyntax::FMT>(outputFile, " {},", datetime);
            item.second.erase(std::remove_if(item.second.begin(),
                                             item.second.end(),
                                             [&](const std::string &d) {
                                                 auto pos = (&d - &*item.second.begin());
                                                 return !outputVariableIndices[pos];
                                             }),
                              item.second.end());
            auto result = std::find_if(item.second.rbegin(), item.second.rend(), [](std::string const &v) { return !v.empty(); });
            auto last = item.second.end() - 1;
            if (result != item.second.rend()) {
                last = (result + 1).base();
            }

            print<FormatSyntax::FMT>(outputFile, "{},", fmt::join(item.second.begin(), last, ","));
            print<FormatSyntax::FMT>(outputFile, "{}\n", *last);
        }

        outputFile.close();
    }

    void ResultsFramework::setupOutputOptions(EnergyPlusData &state)
    {
        if (state.files.outputControl.csv) {
            tsEnabled = true;
            tsAndTabularEnabled = true;
        }

        if (!state.files.outputControl.json) {
            return;
        }

        int numberOfOutputSchemaObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Output:JSON");
        if (numberOfOutputSchemaObjects == 0) {
            return;
        }

        Array1D_string alphas(6);
        int numAlphas;
        Array1D<Real64> numbers(2);
        int numNumbers;
        int status;
        state.dataInputProcessing->inputProcessor->getObjectItem(state, "Output:JSON", 1, alphas, numAlphas, numbers, numNumbers, status);

        if (numAlphas > 0) {
            std::string option = alphas(1);
            if (Util::SameString(option, "TimeSeries")) {
                tsEnabled = true;
            } else if (Util::SameString(option, "TimeSeriesAndTabular")) {
                tsEnabled = true;
                tsAndTabularEnabled = true;
            }

            // defaults
            outputJSON = true;
            outputCBOR = false;
            outputMsgPack = false;

            if (numAlphas >= 2) {
                outputJSON = Util::SameString(alphas(2), "Yes");
            }

            if (numAlphas >= 3) {
                outputCBOR = Util::SameString(alphas(3), "Yes");
            }

            if (numAlphas >= 4) {
                outputMsgPack = Util::SameString(alphas(4), "Yes");
            }

            auto const &ort = state.dataOutRptTab;
            // Jan 2021 Note: Since here we do not know weather ort->unitsStyle_Tabular has been processed or not,
            // the value "NotFound" is used for the option "UseOutputControlTableStyles" at this point;
            // This will be updated again and got concretely assigned first thing in OutputReportTabular::WriteTabularReports().
            ort->unitsStyle_JSON = OutputReportTabular::SetUnitsStyleFromString(alphas(5));
            ort->formatReals_JSON = (getYesNoValue(alphas(6)) == BooleanSwitch::Yes);
        }
    }

    bool ResultsFramework::timeSeriesEnabled() const
    {
        return tsEnabled;
    }

    bool ResultsFramework::timeSeriesAndTabularEnabled() const
    {
        return tsAndTabularEnabled;
    }

    bool ResultsFramework::JSONEnabled() const
    {
        return outputJSON;
    }

    bool ResultsFramework::CBOREnabled() const
    {
        return outputCBOR;
    }

    bool ResultsFramework::MsgPackEnabled() const
    {
        return outputMsgPack;
    }

    void ResultsFramework::initializeTSDataFrame(const OutputProcessor::ReportFreq reportFrequency,
                                                 const std::vector<OutputProcessor::OutVar *> &Variables,
                                                 const OutputProcessor::TimeStepType timeStepType)
    {
        for (auto *var : Variables) {

            if (var->Report && var->freq == reportFrequency) {
                Variable rfvar;
                if (var->units == Constant::Units::customEMS) {
                    rfvar = Variable(var->keyColonName, reportFrequency, var->timeStepType, var->ReportID, var->units, var->unitNameCustomEMS);
                } else {
                    rfvar = Variable(var->keyColonName, reportFrequency, var->timeStepType, var->ReportID, var->units);
                }

                switch (reportFrequency) {
                case OutputProcessor::ReportFreq::EachCall: { // each time UpdatedataandReport is called
                    if (timeStepType == var->timeStepType) {
                        detailedTSData[(int)timeStepType].setDataFrameEnabled(true);
                        detailedTSData[(int)timeStepType].addVariable(rfvar);
                    }
                } break;
                case OutputProcessor::ReportFreq::Hour:       // at 'EndHourFlag'
                case OutputProcessor::ReportFreq::TimeStep:   // at 'EndTimeStepFlag'
                case OutputProcessor::ReportFreq::Day:        // at 'EndDayFlag'
                case OutputProcessor::ReportFreq::Month:      // at 'EndMonthFlag'
                case OutputProcessor::ReportFreq::Simulation: // once per environment 'EndEnvrnFlag'
                case OutputProcessor::ReportFreq::Year: {     // at end of year
                    freqTSData[(int)reportFrequency].setDataFrameEnabled(true);
                    freqTSData[(int)reportFrequency].addVariable(rfvar);
                } break;
                default: {
                    assert(false);
                } break;
                }
            }
        }
        // set the scanned variables to true or false
        switch (reportFrequency) {
        case OutputProcessor::ReportFreq::EachCall: {
            detailedTSData[(int)timeStepType].setVariablesScanned(true);
        } break;
        case OutputProcessor::ReportFreq::TimeStep:   // at 'EndTimeStepFlag'
        case OutputProcessor::ReportFreq::Hour:       // at 'EndHourFlag'
        case OutputProcessor::ReportFreq::Day:        // at 'EndDayFlag'
        case OutputProcessor::ReportFreq::Month:      // at end of month
        case OutputProcessor::ReportFreq::Simulation: // once per environment 'EndEnvrnFlag'
        case OutputProcessor::ReportFreq::Year: {     // at end of year
            detailedTSData[(int)timeStepType].setVariablesScanned(true);
        } break;
        default: {
            assert(false);
        } break;
        }
    }

    void ResultsFramework::initializeMeters(const std::vector<OutputProcessor::Meter *> &meters, const OutputProcessor::ReportFreq freq)
    {
        switch (freq) {
        case OutputProcessor::ReportFreq::EachCall: {
            // nothing to do; meters are not reported at this frequency
        } break;
        case OutputProcessor::ReportFreq::TimeStep:   // at 'TimeStep'
        case OutputProcessor::ReportFreq::Hour:       // at 'Hourly'
        case OutputProcessor::ReportFreq::Day:        // at 'Daily'
        case OutputProcessor::ReportFreq::Month:      // at 'Monthly'
        case OutputProcessor::ReportFreq::Simulation: // at 'RunPeriod'/'SM'
        case OutputProcessor::ReportFreq::Year: {     // at 'Yearly'
            for (auto const *meter : meters) {
                auto const &period = meter->periods[(int)freq];
                if (period.Rpt || period.RptFO) {
                    Meters[(int)freq].addVariable(MeterVariable(meter->Name, freq, period.RptNum, meter->units, period.RptFO));
                    Meters[(int)freq].setDataFrameEnabled(true);
                }
                if (period.accRpt || period.accRptFO) {
                    Meters[(int)freq].addVariable(MeterVariable(meter->Name, freq, period.accRptNum, meter->units, period.accRptFO));
                    Meters[(int)freq].setDataFrameEnabled(true);
                }
            }
        } break;

        default: {
            assert(false);
        } break;
        } // switch (frequency)

        // set the scanned variables to true or false
        switch (freq) {
        case OutputProcessor::ReportFreq::EachCall:
            // case should not happen in Meters
            break;
        case OutputProcessor::ReportFreq::TimeStep:   // at TimeStepFlag
        case OutputProcessor::ReportFreq::Hour:       // at Hourly
        case OutputProcessor::ReportFreq::Day:        // at Daily
        case OutputProcessor::ReportFreq::Month:      // at Monthly
        case OutputProcessor::ReportFreq::Simulation: // at RunPeriod/SM
        case OutputProcessor::ReportFreq::Year: {     // at Yearly
            Meters[(int)freq].setVariablesScanned(true);
        } break;
        default:
            assert(false);
        }
    }

    void ResultsFramework::writeOutputs(EnergyPlusData &state)
    {
        if (state.files.outputControl.csv) {
            writeCSVOutput(state);
        }

        if (timeSeriesEnabled() && (outputJSON || outputCBOR || outputMsgPack)) {
            writeTimeSeriesReports(state.files.json);
        }

        if (timeSeriesAndTabularEnabled() && (outputJSON || outputCBOR || outputMsgPack)) {
            writeReport(state.files.json);
        }
    }

    void ResultsFramework::writeCSVOutput(EnergyPlusData &state)
    {
        using OutputProcessor::ReportFreq;

        if (!hasOutputData()) {
            return;
        }
        CSVWriter csv(outputVariables.size());
        CSVWriter mtr_csv(outputVariables.size());

        for (ReportFreq freq :
             {ReportFreq::Year, ReportFreq::Simulation, ReportFreq::Month, ReportFreq::Day, ReportFreq::Hour, ReportFreq::TimeStep}) {
            // Output yearly time series data
            if (hasTSData(freq)) {
                csv.parseTSOutputs(state, freqTSData[(int)freq].getJSON(), outputVariables, freq);
            }

            if (hasMeters(freq)) {
                csv.parseTSOutputs(state, Meters[(int)freq].getJSON(true), outputVariables, freq);
                mtr_csv.parseTSOutputs(state, Meters[(int)freq].getJSON(), outputVariables, freq);
            }
        }

        for (TimeStepType timeStepType : {TimeStepType::System, TimeStepType::Zone}) {
            // Output detailed HVAC time series data
            if (hasDetailedTSData(timeStepType)) {
                csv.parseTSOutputs(state, detailedTSData[(int)timeStepType].getJSON(), outputVariables, ReportFreq::EachCall);
            }
        }

        csv.writeOutput(state, outputVariables, state.files.csv, state.files.outputControl.csv, rewriteTimestamp);
        if (hasMeterData()) {
            mtr_csv.writeOutput(state, outputVariables, state.files.mtr_csv, state.files.outputControl.csv, rewriteTimestamp);
        }
    }

    void ResultsFramework::writeTimeSeriesReports(JsonOutputFilePaths &jsonOutputFilePaths)
    {
        // Output detailed Zone & HVAC time series data
        for (TimeStepType timeStepType : {TimeStepType::Zone, TimeStepType::System}) {
            if (hasDetailedTSData(timeStepType)) {
                detailedTSData[(int)timeStepType].writeReport(jsonOutputFilePaths, outputJSON, outputCBOR, outputMsgPack);
            }
        }

        // Output timestep time series data
        for (ReportFreq freq :
             {ReportFreq::TimeStep, ReportFreq::Hour, ReportFreq::Day, ReportFreq::Month, ReportFreq::Simulation, ReportFreq::Year}) {
            if (hasFreqTSData(freq)) {
                freqTSData[(int)freq].writeReport(jsonOutputFilePaths, outputJSON, outputCBOR, outputMsgPack);
            }
        }
    }

    void ResultsFramework::writeReport(JsonOutputFilePaths &jsonOutputFilePaths)
    {
        json root, outputVars, meterVars, meterData;
        root = {{"SimulationResults", {{"Simulation", SimulationInformation.getJSON()}}}};

        // output variables

        // This could be constexpr except that json maps do not take string_view keys
        static std::array<std::string, (int)TimeStepType::Num> const timeStepStrings = {"Detailed-Zone", "Detailed-HVAC"};
        for (TimeStepType timeStep : {TimeStepType::Zone, TimeStepType::System}) {
            if (hasDetailedTSData(timeStep)) {
                outputVars[timeStepStrings[(int)timeStep]] = detailedTSData[(int)timeStep].getVariablesJSON();
            }
        }

        // Same issue here
        static std::array<std::string, (int)ReportFreq::Num> const freqStrings = {
            "Detailed", "TimeStep", "Hourly", "Daily", "Monthly", "RunPeriod", "Yearly"};
        for (ReportFreq freq :
             {ReportFreq::Year, ReportFreq::Simulation, ReportFreq::Month, ReportFreq::Day, ReportFreq::Hour, ReportFreq::TimeStep}) {
            if (hasFreqTSData(freq)) {
                outputVars[freqStrings[(int)freq]] = freqTSData[(int)freq].getVariablesJSON();
            }
        }

        // output dictionary
        outputVars["OutputDictionary"] = {{"Description", "Dictionary containing output variables that may be requested"}, {"Variables", RDD}};

        // meter variables

        // -- meter values
        for (ReportFreq freq :
             {ReportFreq::Year, ReportFreq::Simulation, ReportFreq::Month, ReportFreq::Day, ReportFreq::Hour, ReportFreq::TimeStep}) {
            if (hasMeters(freq)) {
                meterVars[freqStrings[(int)freq]] = Meters[(int)freq].getVariablesJSON();
                meterData[freqStrings[(int)freq]] = Meters[(int)freq].getJSON();
            }
        }

        // -- meter dictionary
        meterVars["MeterDictionary"] = {{"Description", "Dictionary containing meter variables that may be requested"}, {"Meters", MDD}};

        root["OutputVariables"] = outputVars;
        root["MeterVariables"] = meterVars;
        root["MeterData"] = meterData;
        root["TabularReports"] = TabularReportsCollection.getJSON();

        if (outputJSON) {
            FileSystem::writeFile<FileSystem::FileTypes::JSON>(jsonOutputFilePaths.outputJsonFilePath, root);
        }
        if (outputCBOR) {
            FileSystem::writeFile<FileSystem::FileTypes::CBOR>(jsonOutputFilePaths.outputCborFilePath, root);
        }
        if (outputMsgPack) {
            FileSystem::writeFile<FileSystem::FileTypes::MsgPack>(jsonOutputFilePaths.outputMsgPackFilePath, root);
        }
    }

    void ResultsFramework::addReportVariable(std::string_view const keyedValue,
                                             std::string_view const variableName,
                                             std::string_view const units,
                                             OutputProcessor::ReportFreq const freq)
    {
        outputVariables.emplace_back(fmt::format("{0}:{1} [{2}]({3})", keyedValue, variableName, units, reportFreqNames[(int)freq]));
    }

    void ResultsFramework::addReportMeter(std::string const &meter, std::string_view units, OutputProcessor::ReportFreq const freq)
    {
        outputVariables.emplace_back(fmt::format("{0} [{1}]({2})", meter, units, reportFreqNames[(int)freq]));
    }

} // namespace ResultsFramework

} // namespace EnergyPlus
