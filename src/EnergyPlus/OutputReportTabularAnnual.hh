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

#ifndef OutputReportTabularAnnual_hh_INCLUDED
#define OutputReportTabularAnnual_hh_INCLUDED

// C++ Headers
#include <ostream>
#include <string>
#include <vector>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/OutputReportData.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace OutputReportTabularAnnual {

    // these functions are not in the class and act as an interface between procedural code and object oriented

    void GetInputTabularAnnual(EnergyPlusData &state);

    void checkAggregationOrderForAnnual(EnergyPlusData &state);

    void GatherAnnualResultsForTimeStep(EnergyPlusData &state, OutputProcessor::TimeStepType kindOfTimeStep);

    void ResetAnnualGathering(const EnergyPlusData &state);

    void WriteAnnualTables(EnergyPlusData &state);

    void AddAnnualTableOfContents(const EnergyPlusData &state, std::ostream &);

    AnnualFieldSet::AggregationKind stringToAggKind(EnergyPlusData &state, std::string inString);

    class AnnualTable
    {
    public:
        // Default Constructor
        AnnualTable() = default;

        // Member Constructor
        AnnualTable(EnergyPlusData &state, const std::string &name, const std::string &filter, const std::string &schedName)
            : m_name(name), m_filter(filter)
        {
            if (!schedName.empty()) {
                m_sched = Sched::GetSchedule(state, schedName); // index to the period schedule
            } else {
                m_sched = nullptr;
            }
        };

        void addFieldSet(const std::string &, AnnualFieldSet::AggregationKind, int);

        void addFieldSet(const std::string &, const std::string &, AnnualFieldSet::AggregationKind, int);

        void setupGathering(EnergyPlusData &state);

        bool invalidAggregationOrder(EnergyPlusData &state);

        void gatherForTimestep(EnergyPlusData &state, OutputProcessor::TimeStepType kindOfTimeStep);

        void resetGathering();

        void writeTable(EnergyPlusData &state, OutputReportTabular::tabularReportStyle const style);

        void addTableOfContents(std::ostream &) const;

        std::vector<std::string> inspectTable() const;

        std::vector<std::string> inspectTableFieldSets(int) const;

        void clearTable();

        // this could be private but was made public for unit testing only
        void columnHeadersToTitleCase(EnergyPlusData const &state);

    private:
        // Members

        std::string m_name; // identifier
        std::string m_filter;
        Sched::Schedule *m_sched = nullptr;
        std::vector<std::string> m_objectNames;     // for each row of annual table
        std::vector<AnnualFieldSet> m_annualFields; // for each column

        static Real64 getElapsedTime(const EnergyPlusData &state, OutputProcessor::TimeStepType kindOfTimeStep);

        static Real64 getSecondsInTimeStep(const EnergyPlusData &state, OutputProcessor::TimeStepType kindOfTimeStep);

        void computeBinColumns(EnergyPlusData &state, OutputReportTabular::UnitsStyle unitsStyle_para);

        static std::vector<std::string> setupAggString();

        static Real64 setEnergyUnitStringAndFactor(OutputReportTabular::UnitsStyle unitsStyle, std::string &unitString);

        static int columnCountForAggregation(AnnualFieldSet::AggregationKind curAgg);

        static std::string trim(const std::string &str);

        static void fixUnitsPerSecond(std::string &unitString, Real64 &conversionFactor);

        bool allRowsSameSizeDeferredVectors(const AnnualFieldSet &fldSt) const;

        void convertUnitForDeferredResults(EnergyPlusData &state, AnnualFieldSet &fldSt, OutputReportTabular::UnitsStyle unitsStyle) const;

        static std::vector<Real64> calculateBins(int numberOfBins,
                                                 std::vector<Real64> const &valuesToBin,
                                                 std::vector<Real64> const corrElapsedTime,
                                                 Real64 const topOfBins,
                                                 Real64 const bottomOfBins,
                                                 Real64 &timeAboveTopBin,
                                                 Real64 &timeBelowBottomBin);

    }; // class AnnualTable

} // namespace OutputReportTabularAnnual

struct OutputReportTabularAnnualData : BaseGlobalStruct
{

    std::vector<OutputReportTabularAnnual::AnnualTable> annualTables;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->annualTables.clear();
    }
};

} // namespace EnergyPlus

#endif
