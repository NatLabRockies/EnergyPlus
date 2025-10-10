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

#ifndef ResultsSchema_hh_INCLUDED
#define ResultsSchema_hh_INCLUDED

#include <memory>
#include <unordered_map>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

#include <nlohmann/json.hpp>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/OutputProcessor.hh>

namespace EnergyPlus {

// Forward declarations
class EnergyPlusFixture;
class ResultsFrameworkFixture;
struct EnergyPlusData;
struct JsonOutputFilePaths;

namespace ResultsFramework {

    using json = nlohmann::json;

    using OutputProcessor::ReportFreq;
    using OutputProcessor::TimeStepType;

    // trim string
    std::string trim(std::string_view const s);

    // base result object
    class BaseResultObject
    {
    public:
        BaseResultObject() = default;
    };

    class SimInfo : public BaseResultObject
    {
    public:
        void setProgramVersion(const std::string &programVersion);
        std::string getProgramVersion() const;
        void setSimulationEnvironment(const std::string &simulationEnvironment);
        void setInputModelURI(const std::string &inputModelURI);
        void setStartDateTimeStamp(const std::string &startDateTimeStamp);
        void setRunTime(const std::string &elapsedTime);
        void setNumErrorsWarmup(const std::string &numWarningsDuringWarmup, const std::string &numSevereDuringWarmup);
        void setNumErrorsSizing(const std::string &numWarningsDuringSizing, const std::string &numSevereDuringSizing);
        void setNumErrorsSummary(const std::string &numWarnings, const std::string &numSevere);
        json getJSON() const;

    protected:
        std::string ProgramVersion;
        std::string SimulationEnvironment;
        std::string InputModelURI;
        std::string StartDateTimeStamp;
        std::string RunTime;
        std::string NumWarningsDuringWarmup, NumSevereDuringWarmup, NumWarningsDuringSizing, NumSevereDuringSizing, NumWarnings, NumSevere;
    };

    class Variable : public BaseResultObject
    {
    public:
        Variable() = default;
        Variable(const std::string &VarName,
                 const ReportFreq reportFrequency,
                 const OutputProcessor::TimeStepType timeStepType,
                 const int ReportID,
                 Constant::Units units);
        Variable(const std::string &VarName,
                 const ReportFreq reportFrequency,
                 const OutputProcessor::TimeStepType timeStepType,
                 const int ReportID,
                 Constant::Units units,
                 const std::string &customUnits);

        std::string variableName() const;
        void setVariableName(const std::string &VarName);

        std::string sReportFrequency() const;
        ReportFreq iReportFrequency() const;
        void setReportFrequency(const ReportFreq reportFrequency);

        OutputProcessor::TimeStepType timeStepType() const;
        void setTimeStepType(const OutputProcessor::TimeStepType timeStepType);

        int reportID() const;
        void setReportID(const int Id);

        Constant::Units units() const;
        void setUnits(Constant::Units units);

        std::string customUnits() const;
        void setCustomUnits(const std::string &customUnits);

        void pushValue(const double val);
        double value(size_t index) const;
        size_t numValues() const;

        virtual json getJSON() const;

    protected:
        std::string m_varName;
        ReportFreq m_reportFreq = ReportFreq::EachCall;
        OutputProcessor::TimeStepType m_timeStepType = OutputProcessor::TimeStepType::Zone;
        int m_rptID = -1;
        Constant::Units m_units;
        std::string m_customUnits;
        std::vector<double> m_values;
    };

    class OutputVariable : public Variable
    {
    public:
        OutputVariable(const std::string &VarName,
                       const ReportFreq reportFrequency,
                       const OutputProcessor::TimeStepType timeStepType,
                       const int ReportID,
                       Constant::Units units);

        OutputVariable(const std::string &VarName,
                       const ReportFreq reportFrequency,
                       const OutputProcessor::TimeStepType timeStepType,
                       const int ReportID,
                       Constant::Units units,
                       const std::string &customUnits);
    };

    class MeterVariable : public Variable
    {
    public:
        MeterVariable() = default;
        MeterVariable(const std::string &VarName,
                      const ReportFreq reportFrequency,
                      const int ReportID,
                      Constant::Units units,
                      const bool MeterOnly,
                      const bool Acculumative = false);

        bool accumulative() const;
        void setAccumulative(bool state);
        bool meterOnly() const;
        void setMeterOnly(bool state);

        json getJSON() const override;

    protected:
        bool acc = false;
        bool meter_only = true;
    };

    class DataFrame : public BaseResultObject
    {
    public:
        typedef std::pair<int, Variable> VarPtrPair;

        explicit DataFrame(const std::string &ReportFreq);
        virtual ~DataFrame() = default;

        void addVariable(Variable const &var);

        void setDataFrameEnabled(bool state);

        bool dataFrameEnabled() const;

        void setVariablesScanned(bool state);

        bool variablesScanned() const;

        void newRow(const int month, const int dayOfMonth, int hourOfDay, int curMin, int calendarYear);
        //        void newRow(const std::string &ts);
        virtual void pushVariableValue(const int reportID, double value);

        Variable &lastVariable();

        json getVariablesJSON();
        json getJSON() const;

        void writeReport(JsonOutputFilePaths &jsonOutputFilePaths, bool outputJSON, bool outputCBOR, bool outputMsgPack);

        // Need to find a way to protect these, they can't be changed on the fly
        bool iso8601 = false;
        bool beginningOfInterval = false;

    protected:
        bool DataFrameEnabled = false;
        bool VariablesScanned = false;
        int lastHour = 0;
        int lastMinute = 0;
        std::string ReportFrequency;
        std::vector<std::string> TS;
        std::map<int, Variable> variableMap;
        int lastVarID = -1;
    };

    class MeterDataFrame : public DataFrame
    {
    public:
        // clang-format off
        explicit MeterDataFrame(const std::string &ReportFreq) : DataFrame(ReportFreq) {};
        // clang-format on
        virtual ~MeterDataFrame() = default;

        void addVariable(MeterVariable const &var);

        void pushVariableValue(const int reportID, double value) override;

        json getJSON(bool meterOnlyCheck = false) const;

    protected:
        std::map<int, MeterVariable> meterMap;
    };

    class Table : public BaseResultObject
    {
    public:
        std::string TableName;
        std::string FootnoteText;
        std::vector<std::string> ColHeaders;
        std::vector<std::string> RowHeaders;
        std::vector<std::vector<std::string>> Data;

        Table(Array2D_string const &body,
              Array1D_string const &rowLabels,
              Array1D_string const &columnLabels,
              std::string const &tableName,
              std::string const &footnoteText);

        json getJSON() const;
    };

    class Report : public BaseResultObject
    {
    public:
        std::string ReportName;
        std::string ReportForString;
        std::vector<Table> Tables;

        json getJSON() const;
    };

    class ReportsCollection : public BaseResultObject
    {
    public:
        typedef std::pair<std::string, Report> RptPtrPair;

        ReportsCollection();

        void addReportTable(Array2D_string const &body,
                            Array1D_string const &rowLabels,
                            Array1D_string const &columnLabels,
                            std::string const &reportName,
                            std::string const &reportForString,
                            std::string const &tableName);

        void addReportTable(Array2D_string const &body,
                            Array1D_string const &rowLabels,
                            Array1D_string const &columnLabels,
                            std::string const &reportName,
                            std::string const &reportForString,
                            std::string const &tableName,
                            std::string const &footnoteText);

        json getJSON() const;

    protected:
        std::unordered_map<std::string, Report> reportsMap;
        Report rpt;
    };

    class CSVWriter : public BaseResultObject
    {
    public:
        CSVWriter() = default;
        explicit CSVWriter(std::size_t num_output_variables) : outputVariableIndices(std::vector<bool>(num_output_variables, false))
        {
        }

        void writeOutput(EnergyPlusData &state,
                         std::vector<std::string> const &outputVariables,
                         InputOutputFile &outputFile,
                         bool outputControl,
                         bool rewriteTimestamp);
        void parseTSOutputs(EnergyPlusData &state, json const &data, std::vector<std::string> const &outputVariables, ReportFreq reportingFrequency);

    private:
        friend class EnergyPlus::EnergyPlusFixture;
        friend class EnergyPlus::ResultsFrameworkFixture;

        char s[129] = {0};
        ReportFreq smallestReportFreq = ReportFreq::Year;
        std::map<std::string, std::vector<std::string>> outputs;
        std::vector<bool> outputVariableIndices;

        static std::string &convertToMonth(std::string &datetime);
        void updateReportFreq(ReportFreq reportingFrequency);
        // void readRVI();
        // void readMVI();
    };

    class ResultsFramework : public BaseResultObject
    {
    public:
        ResultsFramework() = default;

        virtual ~ResultsFramework() = default;

        void setupOutputOptions(EnergyPlusData &state);

        bool timeSeriesEnabled() const;

        bool timeSeriesAndTabularEnabled() const;

        bool JSONEnabled() const;

        bool CBOREnabled() const;

        bool MsgPackEnabled() const;

        void initializeTSDataFrame(const ReportFreq reportFrequency,
                                   const std::vector<OutputProcessor::OutVar *> &Variables,
                                   const OutputProcessor::TimeStepType timeStepType = OutputProcessor::TimeStepType::Zone);

        void initializeMeters(const std::vector<OutputProcessor::Meter *> &EnergyMeters, const ReportFreq reportFrequency);

        std::array<DataFrame, (int)TimeStepType::Num> detailedTSData = {// DataFrame("Dummy"),
                                                                        DataFrame("Detailed-Zone"),
                                                                        DataFrame("Detailed-HVAC")};

        std::array<DataFrame, (int)ReportFreq::Num> freqTSData = {DataFrame("Each Call"),
                                                                  DataFrame("TimeStep"),
                                                                  DataFrame("Hourly"),
                                                                  DataFrame("Daily"),
                                                                  DataFrame("Monthly"),
                                                                  DataFrame("RunPeriod"),
                                                                  DataFrame("Yearly")};

        std::array<MeterDataFrame, (int)ReportFreq::Num> Meters = {MeterDataFrame("Each Call"),
                                                                   MeterDataFrame("TimeStep"),
                                                                   MeterDataFrame("Hourly"),
                                                                   MeterDataFrame("Daily"),
                                                                   MeterDataFrame("Monthly"),
                                                                   MeterDataFrame("RunPeriod"),
                                                                   MeterDataFrame("Yearly")};

        void setISO8601(const bool value)
        {
            rewriteTimestamp = !value;
            for (int iTimeStep = (int)TimeStepType::Zone; iTimeStep < (int)TimeStepType::Num; ++iTimeStep) {
                detailedTSData[iTimeStep].iso8601 = value;
            }

            for (int iFreq = (int)ReportFreq::TimeStep; iFreq < (int)ReportFreq::Num; ++iFreq) {
                freqTSData[iFreq].iso8601 = Meters[iFreq].iso8601 = value;
            }
        }

        void setBeginningOfInterval(const bool value)
        {
            for (int iTimeStep = 0; iTimeStep < (int)TimeStepType::Num; ++iTimeStep) {
                detailedTSData[iTimeStep].beginningOfInterval = value;
            }

            for (int iFreq = 0; iFreq < (int)ReportFreq::Num; ++iFreq) {
                freqTSData[iFreq].beginningOfInterval = Meters[iFreq].beginningOfInterval = value;
            }
        }

        void writeOutputs(EnergyPlusData &state);

        void addReportVariable(std::string_view const keyedValue,
                               std::string_view const variableName,
                               std::string_view const units,
                               ReportFreq const reportingInterval);

        void addReportMeter(std::string const &meter, std::string_view const units, ReportFreq const reportingInterval);

        SimInfo SimulationInformation;

        std::vector<std::string> MDD;
        std::vector<std::string> RDD;
        ReportsCollection TabularReportsCollection;

    protected:
        bool tsEnabled = false;
        bool tsAndTabularEnabled = false;
        bool outputJSON = false;
        bool outputCBOR = false;
        bool outputMsgPack = false;
        bool rewriteTimestamp = true; // Convert monthly data timestamp to month name
        std::vector<std::string> outputVariables;

        void writeTimeSeriesReports(JsonOutputFilePaths &jsonOutputFilePaths);

        void writeReport(JsonOutputFilePaths &jsonOutputFilePaths);

        void writeCSVOutput(EnergyPlusData &state);

    private:
        friend class EnergyPlus::EnergyPlusFixture;
        friend class EnergyPlus::ResultsFrameworkFixture;

    protected:
        inline bool hasDetailedTSData(TimeStepType timeStepType) const
        {
            return detailedTSData[(int)timeStepType].dataFrameEnabled();
        }

        inline bool hasFreqTSData(ReportFreq freq) const
        {
            return freqTSData[(int)freq].dataFrameEnabled();
        }

#ifdef GET_OUT
        inline bool hasRIDetailedZoneTSData() const
        {
            return detailedTSData[(int)TimeStepType::Zone].iDataFrameEnabled() || detailedTSData[(int)TimeStepType::Zone].rDataFrameEnabled();
        };

        inline bool hasRIDetailedHVACTSData() const
        {
            return detailedTSData[(int)TimeStepType::System].iDataFrameEnabled() || detailedTSData[(int)TimeStepType::System].rDataFrameEnabled();
        };

        // This API can be condensed in an obvious way
        inline bool hasRITimestepTSData() const
        {
            return freqTSData[(int)ReportFreq::TimeStep].iDataFrameEnabled() || freqTSData[(int)ReportFreq::TimeStep].rDataFrameEnabled();
        };

        inline bool hasRIHourlyTSData() const
        {
            return freqTSData[(int)ReportFreq::Hour].iDataFrameEnabled() || freqTSData[(int)ReportFreq::Hour].rDataFrameEnabled();
        };

        inline bool hasRIDailyTSData() const
        {
            return freqTSData[(int)ReportFreq::Day].iDataFrameEnabled() || freqTSData[(int)ReportFreq::Day].rDataFrameEnabled();
        };

        inline bool hasRIMonthlyTSData() const
        {
            return freqTSData[(int)ReportFreq::Month].iDataFrameEnabled() || freqTSData[(int)ReportFreq::Month].rDataFrameEnabled();
        };

        inline bool hasRIRunPeriodTSData() const
        {
            return freqTSData[(int)ReportFreq::Simulation].iDataFrameEnabled() || freqTSData[(int)ReportFreq::Simulation].rDataFrameEnabled();
        };

        inline bool hasRIYearlyTSData() const
        {
            return freqTSData[(int)ReportFreq::Year].iDataFrameEnabled() || freqTSData[(int)ReportFreq::Year].rDataFrameEnabled();
        };

        inline bool hasTSMeters() const
        {
            return Meters[(int)ReportFreq::TimeStep].dataFrameEnabled();
        };

        inline bool hasHRMeters() const
        {
            return Meters[(int)ReportFreq::Hour].dataFrameEnabled();
        };

        inline bool hasDYMeters() const
        {
            return Meters[(int)ReportFreq::Day].dataFrameEnabled();
        };

        inline bool hasMNMeters() const
        {
            return Meters[(int)ReportFreq::Month].dataFrameEnabled();
        };

        inline bool hasSMMeters() const
        {
            return Meters[(int)ReportFreq::Simulation].dataFrameEnabled();
        };

        inline bool hasYRMeters() const
        {
            return Meters[(int)ReportFreq::Year].dataFrameEnabled();
        };

#endif //
        inline bool hasMeters(ReportFreq freq) const
        {
            return Meters[(int)freq].dataFrameEnabled();
        }

        inline bool hasMeterData() const
        {
            return hasMeters(ReportFreq::TimeStep) || hasMeters(ReportFreq::Hour) || hasMeters(ReportFreq::Day) || hasMeters(ReportFreq::Month) ||
                   hasMeters(ReportFreq::Simulation) || hasMeters(ReportFreq::Year);
        };

        inline bool hasTSData(ReportFreq freq, TimeStepType timeStepType = TimeStepType::Invalid) const
        {
            assert(freq != ReportFreq::Invalid && (freq != ReportFreq::EachCall || timeStepType != TimeStepType::Invalid));
            return (freq == ReportFreq::EachCall) ? detailedTSData[(int)timeStepType].dataFrameEnabled() : freqTSData[(int)freq].dataFrameEnabled();
        };

        inline bool hasAnyTSData() const
        {
            for (int iTimeStep = 0; iTimeStep < (int)TimeStepType::Num; ++iTimeStep) {
                if (detailedTSData[iTimeStep].dataFrameEnabled()) {
                    return true;
                }
            }
            for (int iFreq = (int)ReportFreq::TimeStep; iFreq < (int)ReportFreq::Num; ++iFreq) {
                if (freqTSData[iFreq].dataFrameEnabled()) {
                    return true;
                }
            }
            return false;
        };

        inline bool hasOutputData() const
        {
            return hasAnyTSData() || hasMeterData();
        };
    };

} // namespace ResultsFramework

struct ResultsFrameworkData : BaseGlobalStruct
{

    std::unique_ptr<ResultsFramework::ResultsFramework> resultsFramework = std::make_unique<ResultsFramework::ResultsFramework>();

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        using OutputProcessor::ReportFreq;
        for (int iFreq = (int)ReportFreq::TimeStep; iFreq < (int)ReportFreq::Num; ++iFreq) {
            auto &meters = this->resultsFramework->Meters[iFreq];
            meters.setDataFrameEnabled(false);
            meters.setVariablesScanned(false);
        }
    }
};

} // namespace EnergyPlus

#endif
