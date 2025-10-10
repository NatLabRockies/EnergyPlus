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
#include <memory>
#include <string>
#include <unordered_set>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include "re2/re2.h"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataOutputs.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/ResultsFramework.hh>
#include <EnergyPlus/SQLiteProcedures.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

#include <fmt/ostream.h>
#include <milo/dtoa.h>

namespace EnergyPlus {

namespace OutputProcessor {

    // MODULE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   December 1998
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // This module contains the major Output Processor routines.
    // In addition, in this file are several routines which can be called
    // without using the OutputProcessor Module

    // METHODOLOGY EMPLOYED:
    // Lots of pointers and other fancy data stuff.  (I didn't see a single pointer here)

    // Routines tagged on the end of this module:
    //  AddDDOutVar
    //  GenOutputVariablesAuditReport
    //  GetCurrentMeterValue
    //  GetInstantMeterValue
    //  GetInternalVariableValue
    //  GetInternalVariableValueExternalInterface
    //  GetMeteredVariables
    //  GetMeterIndex
    //  GetMeterResourceType
    //  GetNumMeteredVariables
    //  GetVariableKeyCountandType
    //  GetVariableKeys
    //  InitPollutionMeterReporting
    //  ProduceRDDMDD
    //  ReportingThisVariable
    //  SetInitialMeterReportingAndOutputNames
    //  SetupOutputVariable
    //  UpdateDataandReport
    //  UpdateMeterReporting

    // Functions

    int DetermineMinuteForReporting(EnergyPlusData const &state)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // When reporting peaks, minutes are used but not necessarily easily calculated.

        Real64 constexpr FracToMin(60.0);
        return ((state.dataGlobal->CurrentTime + state.dataHVACGlobal->SysTimeElapsed) - int(state.dataGlobal->CurrentTime)) * FracToMin;
    }

    void InitializeOutput(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes the OutputProcessor data structures.

        auto &op = state.dataOutputProcessor;

        // Initialize end use category names - the indices must match up with endUseNames in OutputReportTabular
        op->EndUseCategory.allocate((int)Constant::EndUse::Num);
        op->EndUseCategory((int)Constant::EndUse::Heating + 1).Name = "Heating";
        op->EndUseCategory((int)Constant::EndUse::Cooling + 1).Name = "Cooling";
        op->EndUseCategory((int)Constant::EndUse::InteriorLights + 1).Name = "InteriorLights";
        op->EndUseCategory((int)Constant::EndUse::ExteriorLights + 1).Name = "ExteriorLights";
        op->EndUseCategory((int)Constant::EndUse::InteriorEquipment + 1).Name = "InteriorEquipment";
        op->EndUseCategory((int)Constant::EndUse::ExteriorEquipment + 1).Name = "ExteriorEquipment";
        op->EndUseCategory((int)Constant::EndUse::Fans + 1).Name = "Fans";
        op->EndUseCategory((int)Constant::EndUse::Pumps + 1).Name = "Pumps";
        op->EndUseCategory((int)Constant::EndUse::HeatRejection + 1).Name = "HeatRejection";
        op->EndUseCategory((int)Constant::EndUse::Humidification + 1).Name = "Humidifier";
        op->EndUseCategory((int)Constant::EndUse::HeatRecovery + 1).Name = "HeatRecovery";
        op->EndUseCategory((int)Constant::EndUse::WaterSystem + 1).Name = "WaterSystems";
        op->EndUseCategory((int)Constant::EndUse::Refrigeration + 1).Name = "Refrigeration";
        op->EndUseCategory((int)Constant::EndUse::Cogeneration + 1).Name = "Cogeneration";

        // Initialize display names for output table - this could go away if end use key names are changed to match
        op->EndUseCategory((int)Constant::EndUse::Heating + 1).DisplayName = "Heating";
        op->EndUseCategory((int)Constant::EndUse::Cooling + 1).DisplayName = "Cooling";
        op->EndUseCategory((int)Constant::EndUse::InteriorLights + 1).DisplayName = "Interior Lighting";
        op->EndUseCategory((int)Constant::EndUse::ExteriorLights + 1).DisplayName = "Exterior Lighting";
        op->EndUseCategory((int)Constant::EndUse::InteriorEquipment + 1).DisplayName = "Interior Equipment";
        op->EndUseCategory((int)Constant::EndUse::ExteriorEquipment + 1).DisplayName = "Exterior Equipment";
        op->EndUseCategory((int)Constant::EndUse::Fans + 1).DisplayName = "Fans";
        op->EndUseCategory((int)Constant::EndUse::Pumps + 1).DisplayName = "Pumps";
        op->EndUseCategory((int)Constant::EndUse::HeatRejection + 1).DisplayName = "Heat Rejection";
        op->EndUseCategory((int)Constant::EndUse::Humidification + 1).DisplayName = "Humidification";
        op->EndUseCategory((int)Constant::EndUse::HeatRecovery + 1).DisplayName = "Heat Recovery";
        op->EndUseCategory((int)Constant::EndUse::WaterSystem + 1).DisplayName = "Water Systems";
        op->EndUseCategory((int)Constant::EndUse::Refrigeration + 1).DisplayName = "Refrigeration";
        op->EndUseCategory((int)Constant::EndUse::Cogeneration + 1).DisplayName = "Generators";

        op->OutputInitialized = true;

        op->TimeStepZoneSec = double(state.dataGlobal->MinutesInTimeStep) * 60.0;

        state.files.mtd.ensure_open(state, "InitializeMeters", state.files.outputControl.mtd);
    } // InitializeOutput()

    void addEndUseSubcategory(EnergyPlusData &state, EndUseCat endUseCat, std::string_view const endUseSubName)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   February 2006

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages the list of subcategories for each end-use category.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &op = state.dataOutputProcessor;

        Constant::EndUse endUse = endUseCat2endUse[(int)endUseCat];
        if (endUse == Constant::EndUse::Invalid) {
            ShowSevereError(state, format("Nonexistent end use passed to addEndUseSpaceType={}", endUseCatNames[(int)endUseCat]));
            return;
        }

        auto &endUseCategory = op->EndUseCategory((int)endUse + 1);

        for (int EndUseSubNum = 1; EndUseSubNum <= endUseCategory.NumSubcategories; ++EndUseSubNum) {
            if (Util::SameString(endUseCategory.SubcategoryName(EndUseSubNum), endUseSubName)) {
                return; // Subcategory already exists, no further action required
            }
        }

        // Add the subcategory by reallocating the array
        endUseCategory.SubcategoryName.redimension(++endUseCategory.NumSubcategories);
        endUseCategory.SubcategoryName(endUseCategory.NumSubcategories) = endUseSubName;

        if (endUseCategory.NumSubcategories > op->MaxNumSubcategories) {
            op->MaxNumSubcategories = endUseCategory.NumSubcategories;
        }
    } // addEndUseSubcategory()

    void addEndUseSpaceType(EnergyPlusData &state, OutputProcessor::EndUseCat sovEndUseCat, std::string_view const EndUseSpaceTypeName)
    {
        auto &op = state.dataOutputProcessor;

        Constant::EndUse endUse = endUseCat2endUse[(int)sovEndUseCat];

        if (endUse == Constant::EndUse::Invalid) {
            ShowSevereError(state, format("Nonexistent end use passed to addEndUseSpaceType={}", endUseCatNames[(int)sovEndUseCat]));
            return;
        }

        auto &endUseCat = op->EndUseCategory((int)endUse + 1);

        for (int endUseSpTypeNum = 1; endUseSpTypeNum <= endUseCat.numSpaceTypes; ++endUseSpTypeNum) {
            if (Util::SameString(endUseCat.spaceTypeName(endUseSpTypeNum), EndUseSpaceTypeName)) {
                return; // SpaceType already exists, no further action required
            }
        }

        // Add the space type by reallocating the array
        endUseCat.spaceTypeName.redimension(++endUseCat.numSpaceTypes);
        endUseCat.spaceTypeName(endUseCat.numSpaceTypes) = EndUseSpaceTypeName;

        if (endUseCat.numSpaceTypes > op->maxNumEndUseSpaceTypes) {
            op->maxNumEndUseSpaceTypes = endUseCat.numSpaceTypes;
        }
    } // addEndUseSpaceType()

    void SetupTimePointers(EnergyPlusData &state,
                           TimeStepType const timeStep, // Which timestep is being set up, 'Zone'=1, 'HVAC'=2
                           Real64 &TimeStep             // The timestep variable.  Used to get the address
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine sets up the derived type for the output processor that
        // contains pointers to the TimeStep values used in the simulation.

        // METHODOLOGY EMPLOYED:
        // Indicate that the TimeStep passed in is a target for the pointer
        // attributes in the derived types.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // ValidateTimeStepType will throw a Fatal if not valid
        if (state.dataOutputProcessor->TimeValue[(int)timeStep].TimeStep != nullptr) {
            ShowFatalError(state, format("SetupTimePointers was already called for {}", timeStepTypeNames[(int)timeStep]));
        }
        state.dataOutputProcessor->TimeValue[(int)timeStep].TimeStep = &TimeStep;
    }

    void CheckReportVariable(EnergyPlusData &state, std::string_view const Name, std::string const &Key, std::vector<int> &reqVarList)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine will get the report variable information from input and
        // determine if this variable (KeyedValue and VariableName) should be reported
        // and, if so, what frequency to report.

        // This routine is called when SetupOutputVariable is called with no "optional"
        // Reporting Frequency.  It is expected that SetupOutputVariable would only be
        // called once for each keyed variable to be triggered for output (from the input
        // requests).  The optional report frequency would only be used for debugging
        // purposes.  Therefore, this routine will collect all occasions where this
        // passed variablename would be reported from the requested input.  It builds
        // a list of these requests (ReportList) so that the calling routine can propagate
        // the requests into the correct data structure.

        // METHODOLOGY EMPLOYED:
        // This instance being requested will always have a key associated with it.  Matching
        // instances (from input) may or may not have keys, but only one instance of a reporting
        // frequency per variable is allowed.  ReportList will be populated with ReqRepVars indices
        // of those extra things from input that satisfy this condition.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        // Make sure that input has been read
        GetReportVariableInput(state);

        auto const &op = state.dataOutputProcessor;

        for (int iReqVar = 0; iReqVar < (int)op->reqVars.size(); ++iReqVar) {
            auto *reqVar = op->reqVars[iReqVar];

            if (!Util::SameString(reqVar->name, Name)) {
                continue;
            }

            if (!reqVar->key.empty() && !(reqVar->is_simple_string && Util::SameString(reqVar->key, Key)) &&
                !(!reqVar->is_simple_string && RE2::FullMatch(std::string{Key}, *(reqVar->case_insensitive_pattern)))) {
                continue;
            }

            // A match. Make sure doesn't duplicate
            reqVar->Used = true;
            bool Dup = false;
            // op->ReportList is allocated to a large value, so we can't use a std::find_if on it (why not?)
            for (int iReqVar2 : reqVarList) {
                if (op->reqVars[iReqVar2]->freq == reqVar->freq && op->reqVars[iReqVar2]->sched == reqVar->sched) {
                    Dup = true;
                    break;
                }
            }

            if (!Dup) {
                reqVarList.push_back(iReqVar);
            }
        }
    }

    constexpr std::array<std::string_view, (int)ReportFreq::Num> reportingFrequencyNoticeStrings = {
        " !Each Call",                                                             // EachCall
        " !TimeStep",                                                              // TimeStep
        " !Hourly",                                                                // Hourly
        " !Daily [Value,Min,Hour,Minute,Max,Hour,Minute]",                         // Daily
        " !Monthly [Value,Min,Day,Hour,Minute,Max,Day,Hour,Minute]",               // Monthly
        " !RunPeriod [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]", // Simulation
        " !Annual [Value,Min,Month,Day,Hour,Minute,Max,Month,Day,Hour,Minute]"     // Yearly
    };

    ReportFreq determineFrequency(EnergyPlusData &state, const std::string_view FreqString)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998
        //       MODIFIED       December 2017; Jason DeGraw

        // REFERENCES:
        //       \field Reporting Frequency
        //       \type choice
        //       \key Detailed
        //       \note Detailed lists every instance (i.e. HVAC variable timesteps)
        //       \key Timestep
        //       \note Timestep refers to the zone Timestep/Number of Timesteps in hour value
        //       \note RunPeriod, Environment, and Annual are the same
        //       \key Hourly
        //       \key Daily
        //       \key Monthly
        //       \key RunPeriod
        //       \key Environment
        //       \key Annual
        //       \default Hourly
        //       \note RunPeriod and Environment are synonymous

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::array<std::string_view, (int)ReportFreq::Num + 1> PossibleFreqs = {
            "DETA", "TIME", "HOUR", "DAIL", "MONT", "RUNP", "ENVI", "ANNU"};
        //=(/'detail','Timestep','Hourly','Daily','Monthly','RunPeriod','Environment','Annual'/)
        static constexpr std::array<std::string_view, (int)ReportFreq::Num + 1> ExactFreqStrings = {
            "Detailed", "Timestep", "Hourly", "Daily", "Monthly", "RunPeriod", "Environment", "Annual"};
        static constexpr std::array<std::string_view, (int)ReportFreq::Num + 1> ExactFreqStringsUC = {
            "DETAILED", "TIMESTEP", "HOURLY", "DAILY", "MONTHLY", "RUNPERIOD", "ENVIRONMENT", "ANNUAL"};
        // Vector of the result, was { -1, 0, 1, 2, 3, 4, 4, 4 } before the addition of Yearly;
        static constexpr std::array<ReportFreq, (int)ReportFreq::Num + 1> FreqValues = {ReportFreq::EachCall,
                                                                                        ReportFreq::TimeStep,
                                                                                        ReportFreq::Hour,
                                                                                        ReportFreq::Day,
                                                                                        ReportFreq::Month,
                                                                                        ReportFreq::Simulation,
                                                                                        ReportFreq::Simulation,
                                                                                        ReportFreq::Year};

        ReportFreq freq = ReportFreq::Hour; // Default
        // TODO: I think it's supposed to be upper case already, but tests aren't doing that at least...
        const std::string FreqStringUpper = Util::makeUPPER(FreqString);
        std::string::size_type const LenString = min(len(FreqString), static_cast<std::string::size_type>(4u));

        if (LenString < 4u) {
            return freq;
        }

        std::string const FreqStringTrim(FreqStringUpper.substr(0, LenString));
        for (unsigned Loop = 0; Loop < FreqValues.size(); ++Loop) {
            if (FreqStringTrim == PossibleFreqs[Loop]) {
                if (FreqStringUpper != ExactFreqStringsUC[Loop]) {
                    ShowWarningError(state, format("DetermineFrequency: Entered frequency=\"{}\" is not an exact match to key strings.", FreqString));
                    ShowContinueError(state, format("Frequency={} will be used.", ExactFreqStrings[Loop]));
                }
                freq = std::max(FreqValues[Loop], state.dataOutputProcessor->minimumReportFreq);
                break;
            }
        }
        return freq;
    }

    void GetReportVariableInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998
        //       MODIFIED       December 2017; Jason DeGraw

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the requested report variables from
        // the input file.
        // Report Variable,
        //        \memo each Report Variable command picks variables to be put onto the standard output file (.eso)
        //        \memo some variables may not be reported for every simulation
        //   A1 , \field Key_Value
        //        \note use '*' (without quotes) to apply this variable to all keys
        //   A2 , \field Variable_Name
        //   A3 , \field Reporting_Frequency
        //        \type choice
        //        \key detailed
        //        \key timestep
        //        \key hourly
        //        \key daily
        //        \key monthly
        //        \key runperiod
        //   A4 ; \field Schedule_Name
        //        \type object-list
        //        \object-list ScheduleNames

        constexpr std::string_view routineName = "GetReportVariableInput";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlpha;
        int NumNumbers;
        int IOStat;
        bool ErrorsFound(false); // If errors detected in input
        std::string cCurrentModuleObject;
        Array1D_string cAlphaArgs(4);
        Array1D_string cAlphaFieldNames(4);
        Array1D_bool lAlphaBlanks(4);
        Array1D<Real64> rNumericArgs(1);
        Array1D_string cNumericFieldNames(1);
        Array1D_bool lNumericBlanks(1);
        auto &op = state.dataOutputProcessor;

        // Bail out if the input has already been read in
        if (!op->GetOutputInputFlag) {
            return;
        }
        op->GetOutputInputFlag = false;

        // First check environment variable to see of possible override for minimum reporting frequency
        if (!state.dataSysVars->MinReportFrequency.empty()) {
            // Formats
            static constexpr std::string_view Format_800("! <Minimum Reporting Frequency (overriding input value)>, Value, Input Value\n");
            static constexpr std::string_view Format_801(" Minimum Reporting Frequency, {},{}\n");
            op->minimumReportFreq = determineFrequency(state, state.dataSysVars->MinReportFrequency);
            print(state.files.eio, Format_800);
            print(state.files.eio, Format_801, reportingFrequencyNoticeStrings[(int)op->minimumReportFreq], state.dataSysVars->MinReportFrequency);
        }

        cCurrentModuleObject = "Output:Variable";
        int numReqVariables = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        for (int Loop = 1; Loop <= numReqVariables; ++Loop) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     Loop,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

            // Check for duplicates?
            ReqVar *reqVar = new ReqVar();
            op->reqVars.push_back(reqVar);

            reqVar->key = cAlphaArgs(1);
            if (reqVar->key == "*") {
                reqVar->key = std::string();
            }

            bool is_simple_string = !DataOutputs::isKeyRegexLike(reqVar->key);
            reqVar->is_simple_string = is_simple_string;
            if (!is_simple_string) {
                reqVar->case_insensitive_pattern = std::make_shared<RE2>("(?i)" + reqVar->key);
            }

            std::string::size_type const lbpos = index(cAlphaArgs(2), '['); // Remove Units designation if user put it in
            if (lbpos != std::string::npos) {
                cAlphaArgs(2).erase(lbpos);
                // right trim
                cAlphaArgs(2) = cAlphaArgs(2).substr(0, std::min(cAlphaArgs(2).find_last_not_of(" \f\n\r\t\v") + 1, cAlphaArgs(2).size()));
            }
            reqVar->name = cAlphaArgs(2);

            reqVar->freq = determineFrequency(state, Util::makeUPPER(cAlphaArgs(3)));
            if (reqVar->freq == ReportFreq::Invalid) {
                ShowSevereInvalidKey(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                ErrorsFound = true;
            }

            // Schedule information
            if (lAlphaBlanks(4)) {
                reqVar->sched = nullptr;
            } else if ((reqVar->sched = Sched::GetSchedule(state, cAlphaArgs(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(4), cAlphaArgs(4));
                ErrorsFound = true;
            }

            reqVar->Used = false;
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("GetReportVariableInput:{}: errors in input.", cCurrentModuleObject));
        }
    }

    std::string produceDateString(int const date,       // Date of min/max
                                  ReportFreq const freq // Reporting Frequency
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   December 1998

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine produces the appropriate min/max string depending
        // on the reporting frequency.

        // METHODOLOGY EMPLOYED:
        // Prior to calling this routine, the basic value string will be
        // produced, but DecodeMonDayHrMin will not have been called.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Mon;
        int Day;
        int Hour;
        int Minute;

        General::DecodeMonDayHrMin(date, Mon, Day, Hour, Minute);

        switch (freq) {
        case ReportFreq::Day:
            return format("{:2},{:2}", Hour, Minute);
        case ReportFreq::Month:
            return format("{:2},{:2},{:2}", Day, Hour, Minute);
        case ReportFreq::Year:
        case ReportFreq::Simulation:
            return format("{:2},{:2},{:2},{:2}", Mon, Day, Hour, Minute);
        default:
            return std::string();
        }
    }

    // *****************************************************************************
    // The following routines implement Energy Meters in EnergyPlus.
    // *****************************************************************************

    void GetCustomMeterInput(EnergyPlusData &state, bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2006

        // PURPOSE OF THIS SUBROUTINE:
        // This routine will help implement "custom"/user defined meters.  However, it must be called after all
        // the other meters are set up and all report variables are established.

        // REFERENCES:
        // Processes the objects:
        // Meter:Custom,
        //    \extensible:2 - repeat last two fields, remembering to remove ; from "inner" fields.
        //    \memo Used to allow users to combine specific variables and/or meters into
        //    \memo "custom" meter configurations.
        //    A1,  \field Name
        //         \required-field
        //         \reference CustomMeterNames
        //    A2,  \field Fuel Type
        //         \type choice
        //         \key Electricity
        //         \key NaturalGas
        //         \key PropaneGas
        //         \key FuelOilNo1
        //         \key FuelOilNo2
        //         \key Coal
        //         \key Diesel
        //         \key Gasoline
        //         \key Water
        //         \key Generic
        //         \key OtherFuel1
        //         \key OtherFuel2
        //    A3,  \field Key Name 1
        //         \required-field
        //         \begin-extensible
        //    A4,  \field Report Variable or Meter Name 1
        //         \required-field
        // <etc>
        // AND
        // Meter:CustomDecrement,
        //    \extensible:2 - repeat last two fields, remembering to remove ; from "inner" fields.
        //    \memo Used to allow users to combine specific variables and/or meters into
        //    \memo "custom" meter configurations.
        //    A1,  \field Name
        //         \required-field
        //         \reference CustomMeterNames
        //    A2,  \field Fuel Type
        //         \type choice
        //         \key Electricity
        //         \key NaturalGas
        //         \key PropaneGas
        //         \key FuelOilNo1
        //         \key FuelOilNo2
        //         \key Coal
        //         \key Diesel
        //         \key Gasoline
        //         \key Water
        //         \key Generic
        //         \key OtherFuel1
        //         \key OtherFuel2
        //    A3,  \field Source Meter Name
        //         \required-field
        //    A4,  \field Key Name 1
        //         \required-field
        //         \begin-extensible
        //    A5,  \field Report Variable or Meter Name 1
        //         \required-field
        // <etc>

        constexpr std::string_view routineName = "GetCustomMeterInput";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &op = state.dataOutputProcessor;
        auto &ip = state.dataInputProcessing->inputProcessor;
        auto &ipsc = state.dataIPShortCut;

        int NumAlpha;
        int NumNumbers;
        int IOStat;
        Array1D_string NamesOfKeys;   // Specific key name
        Array1D_int IndexesForKeyVar; // Array index

        Array1D_int VarsOnSourceMeter;

        bool BigErrorsFound = false;

        int numCustomMeters = 0, numCustomDecMeters = 0;
        std::vector<std::string> customMeterNames;
        std::vector<std::string> customDecMeterNames;
        if (auto const found = ip->epJSON.find("Meter:Custom"); found != ip->epJSON.end()) {
            for (auto meterInstance = found.value().begin(); meterInstance != found.value().end(); ++meterInstance, ++numCustomMeters) {
                customMeterNames.push_back(Util::makeUPPER(meterInstance.key()));
            }
        }

        if (auto const found = ip->epJSON.find("Meter:CustomDecrement"); found != ip->epJSON.end()) {
            for (auto meterInstance = found.value().begin(); meterInstance != found.value().end(); ++meterInstance, ++numCustomDecMeters) {
                customDecMeterNames.push_back(Util::makeUPPER(meterInstance.key()));
            }
        }

        ipsc->cCurrentModuleObject = "Meter:Custom";
        for (int Loop = 1; Loop <= numCustomMeters; ++Loop) {
            ip->getObjectItem(state,
                              ipsc->cCurrentModuleObject,
                              Loop,
                              ipsc->cAlphaArgs,
                              NumAlpha,
                              ipsc->rNumericArgs,
                              NumNumbers,
                              IOStat,
                              ipsc->lNumericFieldBlanks,
                              ipsc->lAlphaFieldBlanks,
                              ipsc->cAlphaFieldNames,
                              ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)};

            std::string meterName = ipsc->cAlphaArgs(1);
            std::string::size_type lbrackPos = index(meterName, '[');
            if (lbrackPos != std::string::npos) {
                meterName.erase(lbrackPos);
            }

            std::string meterNameUC = Util::makeUPPER(meterName);

            // Check for duplicate name
            if (op->meterMap.find(meterNameUC) != op->meterMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
                continue;
            }

            // Check for invalid resource
            Constant::eResource resource =
                static_cast<Constant::eResource>(getEnumValue(Constant::eResourceNamesUC, Util::makeUPPER(ipsc->cAlphaArgs(2))));
            if (resource == Constant::eResource::Invalid) {
                ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2));
                ErrorsFound = true;
                continue;
            }

            Constant::Units units = Constant::Units::Invalid;

            // We essentially have to do this loop twice, once to
            // check for errors and once to construct the meter.  The
            // reason is that meters are cross-linked with source
            // meters and variables and those back-links will be
            // tricky to undo later.
            bool foundBadSrc = false;
            bool itemsAssigned = false;

            for (int fldIndex = 3; fldIndex <= NumAlpha; fldIndex += 2) {
                if (ipsc->lAlphaFieldBlanks(fldIndex + 1)) {
                    ShowSevereEmptyField(state, eoh, ipsc->cAlphaFieldNames(fldIndex + 1));
                    foundBadSrc = true;
                    break;
                }

                std::string meterOrVarNameUC = Util::makeUPPER(ipsc->cAlphaArgs(fldIndex + 1));
                lbrackPos = index(meterOrVarNameUC, '[');
                if (lbrackPos != std::string::npos) {
                    meterOrVarNameUC.erase(lbrackPos);
                }

                // A custom meter cannot reference another custom meter
                if (std::find(customMeterNames.begin(), customMeterNames.end(), meterOrVarNameUC) != customMeterNames.end()) {
                    ShowWarningError(state,
                                     format(R"(Meter:Custom="{}", contains a reference to another Meter:Custom in field: {}="{}".)",
                                            ipsc->cAlphaArgs(1),
                                            ipsc->cAlphaFieldNames(fldIndex + 1),
                                            ipsc->cAlphaArgs(fldIndex + 1)));
                    foundBadSrc = true;
                    break;
                }

                // A custom meter cannot reference another customDec meter
                if (std::find(customDecMeterNames.begin(), customDecMeterNames.end(), meterOrVarNameUC) != customDecMeterNames.end()) {
                    ShowWarningError(state,
                                     format(R"(Meter:Custom="{}", contains a reference to another Meter:CustomDecrement in field: {}="{}".)",
                                            ipsc->cAlphaArgs(1),
                                            ipsc->cAlphaFieldNames(fldIndex + 1),
                                            ipsc->cAlphaArgs(fldIndex + 1)));
                    foundBadSrc = true;
                    break;
                }

                if (auto foundSrcMeter = op->meterMap.find(meterOrVarNameUC); foundSrcMeter != op->meterMap.end()) {
                    int srcMeterNum = foundSrcMeter->second;
                    auto *srcMeter = op->meters[srcMeterNum];
                    assert(srcMeter->type == MeterType::Normal);

                    // If it's the first meter, it gets to set the units
                    if (units == Constant::Units::Invalid) {
                        units = srcMeter->units;
                        itemsAssigned = true;
                    } else if (units != srcMeter->units) {
                        ShowWarningCustom(state,
                                          eoh,
                                          format(R"(Meter:Custom="{}", differing units in {}="{}".)",
                                                 ipsc->cAlphaArgs(1),
                                                 ipsc->cAlphaFieldNames(fldIndex + 1),
                                                 meterOrVarNameUC));
                        ShowContinueError(state,
                                          format("...will not be shown with the Meter results; units for meter={}, units for this variable={}.",
                                                 Constant::unitNames[(int)units],
                                                 Constant::unitNames[(int)srcMeter->units]));
                        foundBadSrc = true;
                        break;
                    }

                    // It's a variable
                } else if (auto foundSrcDDVar = op->ddOutVarMap.find(meterOrVarNameUC); foundSrcDDVar != op->ddOutVarMap.end()) {
                    int srcDDVarNum = foundSrcDDVar->second;
                    auto *srcDDVar = op->ddOutVars[srcDDVarNum];

                    // Has to be a summed variable
                    if (srcDDVar->storeType != StoreType::Sum) {
                        ShowWarningCustom(state,
                                          eoh,
                                          format(R"(Meter:Custom="{}", variable not summed variable {}="{}".)",
                                                 ipsc->cAlphaArgs(1),
                                                 ipsc->cAlphaFieldNames(fldIndex + 1),
                                                 meterOrVarNameUC));
                        ShowContinueError(state,
                                          format("...will not be shown with the Meter results; units for meter={}, units for this variable={}.",
                                                 Constant::unitNames[(int)units],
                                                 Constant::unitNames[(int)srcDDVar->units]));
                        foundBadSrc = true;
                        break;
                    }

                    // If it's the first variable, it gets to set the units
                    if (units == Constant::Units::Invalid) {
                        units = srcDDVar->units;
                        // Otherwise it has to match the existing units
                    } else if (units != srcDDVar->units) {
                        ShowWarningCustom(
                            state, eoh, format("differing units in {}=\"{}\".", ipsc->cAlphaFieldNames(fldIndex + 1), meterOrVarNameUC));
                        ShowContinueError(state,
                                          format("...will not be shown with the Meter results; units for meter={}, units for this variable={}.",
                                                 Constant::unitNames[(int)units],
                                                 Constant::unitNames[(int)srcDDVar->units]));
                        foundBadSrc = true;
                        break;
                    }

                    bool KeyIsStar = (ipsc->cAlphaArgs(fldIndex) == "*" || ipsc->lAlphaFieldBlanks(fldIndex));
                    // Have already checked for mismatching units between meter and source variable and assigned units
                    if (KeyIsStar) {
                        if (srcDDVar->keyOutVarNums.empty()) {
                            ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(fldIndex + 1), meterOrVarNameUC);
                            foundBadSrc = true;
                            break;
                        }

                        itemsAssigned = true;
                    } else { // Key is not "*"
                        bool foundKey = false;
                        for (int keyOutVarNum : srcDDVar->keyOutVarNums) {
                            if (op->outVars[keyOutVarNum]->keyUC == ipsc->cAlphaArgs(fldIndex)) {
                                foundKey = true;
                                itemsAssigned = true;
                                break;
                            }
                        }
                        if (!foundKey) {
                            ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(fldIndex + 1), meterOrVarNameUC);
                            foundBadSrc = true;
                            break;
                        }
                    } // if (keyIsStar)

                    // Not a meter or a variable
                } else {
                    // Cannot use ShowWarningItemNotFound because this string appears in a unit test
                    ShowWarningError(state,
                                     format(R"(Meter:Custom="{}", invalid {}="{}".)",
                                            ipsc->cAlphaArgs(1),
                                            ipsc->cAlphaFieldNames(fldIndex + 1),
                                            ipsc->cAlphaArgs(fldIndex + 1)));
                    ShowContinueError(state, "...will not be shown with the Meter results.");
                    // Not setting the foundBadSrc flag here.
                }

            } // for (fldIndex)

            // Somehow, this meter is not linked to any variables either directly or via another meter
            if (!itemsAssigned) {
                ShowWarningError(state, format("Meter:Custom=\"{}\", no items assigned ", ipsc->cAlphaArgs(1)));
                ShowContinueError(
                    state, "...will not be shown with the Meter results. This may be caused by a Meter:Custom be assigned to another Meter:Custom.");
                continue;
            }

            // One of the sources is bad
            if (foundBadSrc) {
                continue;
            }

            auto *meter = new Meter(meterName);
            meter->type = MeterType::Custom;
            meter->resource = resource;
            meter->units = units;
            bool errFlag = false;
            meter->RT_forIPUnits = GetResourceIPUnits(state, meter->resource, meter->units, errFlag);
            if (errFlag) {
                ShowContinueError(state, format("..on {}=\"{}\".", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "..requests for IP units from this meter will be ignored.");
            }

            // This meter is good
            int meterNum = op->meters.size();
            op->meters.push_back(meter);
            op->meterMap.insert_or_assign(meterNameUC, meterNum);

            for (ReportFreq reportFreq :
                 {ReportFreq::TimeStep, ReportFreq::Hour, ReportFreq::Day, ReportFreq::Month, ReportFreq::Year, ReportFreq::Simulation}) {
                meter->periods[(int)reportFreq].RptNum = ++op->ReportNumberCounter;
            }

            for (ReportFreq reportFreq :
                 {ReportFreq::TimeStep, ReportFreq::Hour, ReportFreq::Day, ReportFreq::Month, ReportFreq::Year, ReportFreq::Simulation}) {
                meter->periods[(int)reportFreq].accRptNum = ++op->ReportNumberCounter;
            }

            // Do the loop again, this time without error checking
            for (int fldIndex = 3; fldIndex <= NumAlpha; fldIndex += 2) {
                // No need to check for empty fields
                std::string meterOrVarNameUC = Util::makeUPPER(ipsc->cAlphaArgs(fldIndex + 1));
                lbrackPos = index(meterOrVarNameUC, '[');
                if (lbrackPos != std::string::npos) {
                    meterOrVarNameUC.erase(lbrackPos);
                }

                // No need to check for custom source meters
                if (auto foundSrcMeter = op->meterMap.find(meterOrVarNameUC); foundSrcMeter != op->meterMap.end()) {
                    int srcMeterNum = foundSrcMeter->second;
                    auto *srcMeter = op->meters[srcMeterNum];
                    assert(srcMeter->type == MeterType::Normal);

                    // No need to check for units
                    // No need to check for duplicates

                    // Check for duplicates
                    if (std::find(meter->srcMeterNums.begin(), meter->srcMeterNums.end(), srcMeterNum) != meter->srcMeterNums.end()) {
                        ShowWarningCustom(state,
                                          eoh,
                                          format("{}=\"{}\" referenced multiple times, only first instance will be used",
                                                 ipsc->cAlphaFieldNames(fldIndex + 1),
                                                 meterOrVarNameUC));
                        continue;
                    }

                    // Link meter to src meter and var and vice versa
                    meter->srcMeterNums.push_back(srcMeterNum);
                    srcMeter->dstMeterNums.push_back(meterNum);

                    for (int srcVarNum : srcMeter->srcVarNums) {
                        if (std::find(meter->srcVarNums.begin(), meter->srcVarNums.end(), srcVarNum) == meter->srcVarNums.end()) {
                            meter->srcVarNums.push_back(srcVarNum);
                            op->outVars[srcVarNum]->meterNums.push_back(meterNum);
                        }
                    }

                    // It's a variable
                } else if (auto foundSrcDDVar = op->ddOutVarMap.find(meterOrVarNameUC); foundSrcDDVar != op->ddOutVarMap.end()) {
                    int srcDDVarNum = foundSrcDDVar->second;
                    auto *srcDDVar = op->ddOutVars[srcDDVarNum];

                    // No need to check for a summed variable
                    // No need to check for units match or to assign units

                    bool KeyIsStar = (ipsc->cAlphaArgs(fldIndex) == "*" || ipsc->lAlphaFieldBlanks(fldIndex));
                    // Have already checked for mismatching units between meter and source variable and assigned units
                    if (KeyIsStar) {
                        // No need to check for empty keys
                        for (int keyOutVarNum : srcDDVar->keyOutVarNums) {
                            if (std::find(meter->srcVarNums.begin(), meter->srcVarNums.end(), keyOutVarNum) != meter->srcVarNums.end()) {
                                ShowWarningCustom(state,
                                                  eoh,
                                                  format("Output variable \"{}\" referenced multiple times (directly or via meter)",
                                                         op->outVars[keyOutVarNum]->keyColonNameUC));

                            } else {
                                meter->srcVarNums.push_back(keyOutVarNum);
                                op->outVars[keyOutVarNum]->meterNums.push_back(meterNum);
                            }
                        }
                    } else { // Key is not "*"
                        for (int keyOutVarNum : srcDDVar->keyOutVarNums) {
                            if (op->outVars[keyOutVarNum]->keyUC == ipsc->cAlphaArgs(fldIndex)) {
                                if (std::find(meter->srcVarNums.begin(), meter->srcVarNums.end(), keyOutVarNum) != meter->srcVarNums.end()) {
                                    ShowWarningCustom(state,
                                                      eoh,
                                                      format("Output variable \"{}\" referenced multiple times (directly or via meter)",
                                                             op->outVars[keyOutVarNum]->keyColonNameUC));
                                } else {
                                    meter->srcVarNums.push_back(keyOutVarNum);
                                    op->outVars[keyOutVarNum]->meterNums.push_back(meterNum);
                                }
                                break;
                            }
                        }
                    } // if (keyIsStar)
                } // if (meter or variable)

            } // for (fldIndex)
        } // for (Loop)

        ipsc->cCurrentModuleObject = "Meter:CustomDecrement";
        for (int Loop = 1; Loop <= numCustomDecMeters; ++Loop) {
            ip->getObjectItem(state,
                              ipsc->cCurrentModuleObject,
                              Loop,
                              ipsc->cAlphaArgs,
                              NumAlpha,
                              ipsc->rNumericArgs,
                              NumNumbers,
                              IOStat,
                              ipsc->lNumericFieldBlanks,
                              ipsc->lAlphaFieldBlanks,
                              ipsc->cAlphaFieldNames,
                              ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)};

            std::string meterName = ipsc->cAlphaArgs(1);
            std::string::size_type lbrackPos = index(meterName, '[');
            if (lbrackPos != std::string::npos) {
                meterName.erase(lbrackPos);
            }
            std::string meterNameUC = Util::makeUPPER(meterName);

            // Search for duplicate name
            if (op->meterMap.find(meterNameUC) != op->meterMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
                continue;
            }

            // Can't use resource type in AddMeter cause it will confuse it with other meters.  So, now:
            Constant::eResource resource =
                static_cast<Constant::eResource>(getEnumValue(Constant::eResourceNamesUC, Util::makeUPPER(ipsc->cAlphaArgs(2))));
            if (resource == Constant::eResource::Invalid) {
                ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2));
                ErrorsFound = true;
                continue;
            }

            bool itemsAssigned = false;

            std::string decMeterName = ipsc->cAlphaArgs(3);
            lbrackPos = index(decMeterName, '[');
            if (lbrackPos != std::string::npos) {
                decMeterName.erase(lbrackPos);
            }
            std::string decMeterNameUC = Util::makeUPPER(decMeterName);

            // DecMeter cannot be a Meter:Custom
            if (std::find(customDecMeterNames.begin(), customDecMeterNames.end(), decMeterNameUC) != customDecMeterNames.end()) {
                ShowWarningError(state,
                                 format(R"(Meter:CustomDec="{}", contains a reference to another Meter:CustomDecrement in field: {}="{}".)",
                                        ipsc->cAlphaArgs(1),
                                        ipsc->cAlphaFieldNames(3),
                                        ipsc->cAlphaArgs(3)));
                ErrorsFound = true;
                continue;
            }

            auto foundDecMeter = op->meterMap.find(decMeterName);
            if (foundDecMeter == op->meterMap.end()) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(3), decMeterName);
                ErrorsFound = true;
                continue;
            }

            int decMeterNum = foundDecMeter->second;
            auto *decMeter = op->meters[decMeterNum];
            assert(decMeter->type == MeterType::Normal);

            Constant::Units units = decMeter->units;

            itemsAssigned = true;

            // We essentially have to do this loop twice, once to
            // check for errors and once to construct the meter.  The
            // reason is that meters are cross-linked with source
            // meters and variables and those back-links will be
            // tricky to undo later.
            bool foundBadSrc = false;

            for (int fldIndex = 4; fldIndex <= NumAlpha; fldIndex += 2) {
                if (ipsc->lAlphaFieldBlanks(fldIndex + 1)) {
                    ShowSevereEmptyField(state, eoh, ipsc->cAlphaFieldNames(fldIndex + 1));
                    foundBadSrc = true;
                    break;
                }

                std::string meterOrVarNameUC = Util::makeUPPER(ipsc->cAlphaArgs(fldIndex + 1));
                lbrackPos = index(meterOrVarNameUC, '[');
                if (lbrackPos != std::string::npos) {
                    meterOrVarNameUC.erase(lbrackPos);
                }

                // A custom meter cannot reference another custom meter
                if (std::find(customDecMeterNames.begin(), customDecMeterNames.end(), meterOrVarNameUC) != customDecMeterNames.end()) {
                    ShowWarningError(state,
                                     format(R"(Meter:Custom="{}", contains a reference to another Meter:CustomDecrement in field: {}="{}".)",
                                            ipsc->cAlphaArgs(1),
                                            ipsc->cAlphaFieldNames(fldIndex + 1),
                                            ipsc->cAlphaArgs(fldIndex + 1)));
                    foundBadSrc = true;
                    break;
                }

                if (auto foundSrcMeter = op->meterMap.find(meterOrVarNameUC); foundSrcMeter != op->meterMap.end()) {
                    int srcMeterNum = foundSrcMeter->second;
                    auto *srcMeter = op->meters[srcMeterNum];
                    assert(srcMeter->type == MeterType::Normal || srcMeter->type == MeterType::Custom);

                    // If it's the first meter, it gets to set the units
                    if (units == Constant::Units::Invalid) {
                        units = srcMeter->units;
                        itemsAssigned = true;
                    } else if (units != srcMeter->units) {
                        ShowWarningCustom(state,
                                          eoh,
                                          format(R"(Meter:Custom="{}", differing units in {}="{}".)",
                                                 ipsc->cAlphaArgs(1),
                                                 ipsc->cAlphaFieldNames(fldIndex + 1),
                                                 meterOrVarNameUC));
                        ShowContinueError(state,
                                          format("...will not be shown with the Meter results; units for meter={}, units for this variable={}.",
                                                 Constant::unitNames[(int)units],
                                                 Constant::unitNames[(int)srcMeter->units]));
                        foundBadSrc = true;
                        break;
                    }

                    // It's a variable
                } else if (auto foundSrcDDVar = op->ddOutVarMap.find(meterOrVarNameUC); foundSrcDDVar != op->ddOutVarMap.end()) {
                    int srcDDVarNum = foundSrcDDVar->second;
                    auto *srcDDVar = op->ddOutVars[srcDDVarNum];

                    // Has to be a summed variable
                    if (srcDDVar->storeType != StoreType::Sum) {
                        ShowWarningCustom(state,
                                          eoh,
                                          format(R"(Meter:Custom="{}", variable not summed variable {}="{}".)",
                                                 ipsc->cAlphaArgs(1),
                                                 ipsc->cAlphaFieldNames(fldIndex + 1),
                                                 meterOrVarNameUC));
                        ShowContinueError(state,
                                          format("...will not be shown with the Meter results; units for meter={}, units for this variable={}.",
                                                 Constant::unitNames[(int)units],
                                                 Constant::unitNames[(int)srcDDVar->units]));
                        foundBadSrc = true;
                        break;
                    }

                    // If it's the first variable, it gets to set the units
                    if (units == Constant::Units::Invalid) {
                        units = srcDDVar->units;
                        // Otherwise it has to match the existing units
                    } else if (units != srcDDVar->units) {
                        ShowWarningCustom(
                            state, eoh, format("differing units in {}=\"{}\".", ipsc->cAlphaFieldNames(fldIndex + 1), meterOrVarNameUC));
                        ShowContinueError(state,
                                          format("...will not be shown with the Meter results; units for meter={}, units for this variable={}.",
                                                 Constant::unitNames[(int)units],
                                                 Constant::unitNames[(int)srcDDVar->units]));
                        foundBadSrc = true;
                        break;
                    }

                    bool KeyIsStar = (ipsc->cAlphaArgs(fldIndex) == "*" || ipsc->lAlphaFieldBlanks(fldIndex));
                    // Have already checked for mismatching units between meter and source variable and assigned units
                    if (KeyIsStar) {
                        if (srcDDVar->keyOutVarNums.empty()) {
                            ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(fldIndex + 1), meterOrVarNameUC);
                            foundBadSrc = true;
                            break;
                        }

                        itemsAssigned = true;
                    } else { // Key is not "*"
                        bool foundKey = false;
                        for (int keyOutVarNum : srcDDVar->keyOutVarNums) {
                            if (op->outVars[keyOutVarNum]->keyUC == ipsc->cAlphaArgs(fldIndex)) {
                                foundKey = true;
                                itemsAssigned = true;
                                break;
                            }
                        }
                        if (!foundKey) {
                            ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(fldIndex + 1), meterOrVarNameUC);
                            foundBadSrc = true;
                            break;
                        }
                    } // if (keyIsStar)

                    // Not a meter or a variable
                } else {
                    // Cannot use ShowWarningItemNotFound because this string appears in a unit test
                    ShowWarningError(state,
                                     format(R"(Meter:Custom="{}", invalid {}="{}".)",
                                            ipsc->cAlphaArgs(1),
                                            ipsc->cAlphaFieldNames(fldIndex + 1),
                                            ipsc->cAlphaArgs(fldIndex + 1)));
                    ShowContinueError(state, "...will not be shown with the Meter results.");
                    foundBadSrc = true;
                    break;
                }

            } // for (fldIndex)

            // Somehow, this meter is not linked to any variables either directly or via another meter
            if (!itemsAssigned) {
                ShowWarningError(state, format("Meter:Custom=\"{}\", no items assigned ", ipsc->cAlphaArgs(1)));
                ShowContinueError(
                    state, "...will not be shown with the Meter results. This may be caused by a Meter:Custom be assigned to another Meter:Custom.");
                continue;
            }

            // One of the sources is bad
            if (foundBadSrc) {
                continue;
            }

            auto *meter = new Meter(meterName);
            meter->type = MeterType::CustomDec;
            meter->resource = resource;
            meter->units = units;
            bool errFlag = false;
            meter->RT_forIPUnits = GetResourceIPUnits(state, meter->resource, meter->units, errFlag);
            if (errFlag) {
                ShowContinueError(state, format("..on {}=\"{}\".", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "..requests for IP units from this meter will be ignored.");
            }

            meter->decMeterNum = decMeterNum;

            // This meter is good
            int meterNum = op->meters.size();
            op->meters.push_back(meter);
            op->meterMap.insert_or_assign(meterNameUC, meterNum);

            for (ReportFreq reportFreq :
                 {ReportFreq::TimeStep, ReportFreq::Hour, ReportFreq::Day, ReportFreq::Month, ReportFreq::Year, ReportFreq::Simulation}) {
                meter->periods[(int)reportFreq].RptNum = ++op->ReportNumberCounter;
            }

            for (ReportFreq reportFreq :
                 {ReportFreq::TimeStep, ReportFreq::Hour, ReportFreq::Day, ReportFreq::Month, ReportFreq::Year, ReportFreq::Simulation}) {
                meter->periods[(int)reportFreq].accRptNum = ++op->ReportNumberCounter;
            }

            //  Links meter to dec meter and its output variable and vice versa
            meter->srcMeterNums.push_back(meter->decMeterNum);
            decMeter->dstMeterNums.push_back(meterNum);

            // Not linking decMeter vars to this meter and vice versa
            // for (int srcVarNum : decMeter->srcVarNums) {
            //    if (std::find(meter->srcVarNums.begin(), meter->srcVarNums.end(), srcVarNum) != meter->srcVarNums.end()) continue; // Already linked
            //    meter->srcVarNums.push_back(srcVarNum);
            //    op->outVars[srcVarNum]->meterNums.push_back(meterNum);
            // }

            // Do the loop again, this time without error checking
            for (int fldIndex = 4; fldIndex <= NumAlpha; fldIndex += 2) {
                // No need to check for empty fields
                std::string meterOrVarNameUC = Util::makeUPPER(ipsc->cAlphaArgs(fldIndex + 1));
                lbrackPos = index(meterOrVarNameUC, '[');
                if (lbrackPos != std::string::npos) {
                    meterOrVarNameUC.erase(lbrackPos);
                }

                // No need to check for custom source meters
                if (auto foundSrcMeter = op->meterMap.find(meterOrVarNameUC); foundSrcMeter != op->meterMap.end()) {
                    int srcMeterNum = foundSrcMeter->second;
                    auto *srcMeter = op->meters[srcMeterNum];
                    assert(srcMeter->type == MeterType::Normal || srcMeter->type == MeterType::Custom);

                    // No need to check for units
                    // No need to check for duplicates

                    // Check for duplicates
                    if (std::find(meter->srcMeterNums.begin(), meter->srcMeterNums.end(), srcMeterNum) != meter->srcMeterNums.end()) {
                        ShowWarningCustom(state,
                                          eoh,
                                          format("{}=\"{}\" referenced multiple times, only first instance will be used",
                                                 ipsc->cAlphaFieldNames(fldIndex + 1),
                                                 meterOrVarNameUC));
                        continue;
                    }

                    // Link meter to src meter and var and vice versa
                    meter->srcMeterNums.push_back(srcMeterNum);
                    srcMeter->dstMeterNums.push_back(meterNum);

                    for (int srcVarNum : srcMeter->srcVarNums) {
                        if (std::find(meter->srcVarNums.begin(), meter->srcVarNums.end(), srcVarNum) == meter->srcVarNums.end()) {
                            meter->srcVarNums.push_back(srcVarNum);
                            op->outVars[srcVarNum]->meterNums.push_back(meterNum);
                        }
                    }

                    // It's a variable
                } else if (auto foundSrcDDVar = op->ddOutVarMap.find(meterOrVarNameUC); foundSrcDDVar != op->ddOutVarMap.end()) {
                    int srcDDVarNum = foundSrcDDVar->second;
                    auto *srcDDVar = op->ddOutVars[srcDDVarNum];

                    // No need to check for a summed variable
                    // No need to check for units match or to assign units

                    bool KeyIsStar = (ipsc->cAlphaArgs(fldIndex) == "*" || ipsc->lAlphaFieldBlanks(fldIndex));
                    // Have already checked for mismatching units between meter and source variable and assigned units
                    if (KeyIsStar) {
                        // No need to check for empty keys
                        for (int keyOutVarNum : srcDDVar->keyOutVarNums) {
                            if (std::find(meter->srcVarNums.begin(), meter->srcVarNums.end(), keyOutVarNum) != meter->srcVarNums.end()) {
                                ShowWarningCustom(state,
                                                  eoh,
                                                  format("Output variable \"{}\" referenced multiple times (directly or via meter)",
                                                         op->outVars[keyOutVarNum]->keyColonNameUC));

                            } else {
                                meter->srcVarNums.push_back(keyOutVarNum);
                                op->outVars[keyOutVarNum]->meterNums.push_back(meterNum);
                            }
                        }
                    } else { // Key is not "*"
                        for (int keyOutVarNum : srcDDVar->keyOutVarNums) {
                            if (op->outVars[keyOutVarNum]->keyUC == ipsc->cAlphaArgs(fldIndex)) {
                                if (std::find(meter->srcVarNums.begin(), meter->srcVarNums.end(), keyOutVarNum) != meter->srcVarNums.end()) {
                                    ShowWarningCustom(state,
                                                      eoh,
                                                      format("Output variable \"{}\" referenced multiple times (directly or via meter)",
                                                             op->outVars[keyOutVarNum]->keyColonNameUC));
                                } else {
                                    meter->srcVarNums.push_back(keyOutVarNum);
                                    op->outVars[keyOutVarNum]->meterNums.push_back(meterNum);
                                }
                                break;
                            }
                        }
                    } // if (keyIsStar)
                } // if (meter or variable)

            } // for (fldIndex)
        }

        if (BigErrorsFound) {
            ErrorsFound = true;
        }
    }

    int AddMeter(EnergyPlusData &state,
                 std::string const &Name,          // Name for the meter
                 Constant::Units const units,      // Units for the meter
                 Constant::eResource resource,     // ResourceType for the meter
                 EndUseCat endUseCat,              // EndUse for the meter
                 std::string_view const EndUseSub, // EndUse subcategory for the meter
                 Group group,
                 int outVarNum) // Variable index
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine adds a meter to the current definition set of meters.  If the maximum has
        // already been reached, a reallocation procedure begins.  This action needs to be done at the
        // start of the simulation, primarily before any output is stored.

        // Make sure this isn't already in the list of meter names
        auto &op = state.dataOutputProcessor;

        int meterNum = -1;
        Meter *meter = nullptr;

        std::string nameUC = Util::makeUPPER(Name);

        if (auto found = op->meterMap.find(nameUC); found != op->meterMap.end()) {
            meterNum = found->second;
            meter = op->meters[meterNum];
        } else {

            meterNum = op->meters.size();
            meter = new Meter(Name);
            op->meters.push_back(meter);
            op->meterMap.insert_or_assign(nameUC, meterNum);

            meter->type = MeterType::Normal;
            meter->resource = resource;
            meter->endUseCat = endUseCat;
            meter->EndUseSub = EndUseSub;
            meter->group = group;
            meter->units = units;
            meter->CurTSValue = 0.0;

            for (ReportFreq reportFreq :
                 {ReportFreq::TimeStep, ReportFreq::Hour, ReportFreq::Day, ReportFreq::Month, ReportFreq::Year, ReportFreq::Simulation}) {
                meter->periods[(int)reportFreq].RptNum = ++op->ReportNumberCounter;
            }

            for (ReportFreq reportFreq :
                 {ReportFreq::TimeStep, ReportFreq::Hour, ReportFreq::Day, ReportFreq::Month, ReportFreq::Year, ReportFreq::Simulation}) {
                meter->periods[(int)reportFreq].accRptNum = ++op->ReportNumberCounter;
            }

            if (meter->resource != Constant::eResource::Invalid) {
                bool errFlag = false;
                meter->RT_forIPUnits = GetResourceIPUnits(state, meter->resource, units, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("..on Meter=\"{}\".", Name));
                    ShowContinueError(state, "..requests for IP units from this meter will be ignored.");
                }
            }
        }

        // outVarNum == -1 is only true in unit tests
        if (outVarNum != -1) {
            OutVar *var = op->outVars[outVarNum];
            var->meterNums.push_back(meterNum);
            meter->srcVarNums.push_back(outVarNum);
        }

        return meterNum;
    }

    void AttachMeters(EnergyPlusData &state,
                      Constant::Units const units,      // Units for this meter
                      Constant::eResource resource,     // Electricity, Gas, etc.
                      EndUseCat endUseCat,              // End-use category (Lights, Heating, etc.)
                      std::string_view const EndUseSub, // End-use subcategory (user-defined, e.g., General Lights, Task Lights, etc.)
                      Group group,                      // Group key (Facility, Zone, Building, etc.)
                      std::string const &ZoneName,      // Zone key only applicable for Building group
                      std::string const &SpaceType,     // Space Type key only applicable for Building group
                      int const outVarNum               // Number of this report variable
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine determines which meters this variable will be on (if any),
        // creates those meters and links the variable to them (and vice versa).

        std::string_view resourceName = Constant::eResourceNames[(int)resource];

        std::string endUseSub = standardizeEndUseSub(endUseCat, EndUseSub);

        if (!endUseSub.empty()) {
            addEndUseSubcategory(state, endUseCat, endUseSub);
        }

        if (!SpaceType.empty()) {
            addEndUseSpaceType(state, endUseCat, SpaceType);
        }

        std::string meterName = format("{}:Facility", resourceName);
        AddMeter(state, meterName, units, resource, EndUseCat::Invalid, "", Group::Invalid, outVarNum);

        if (group != Group::Invalid) {
            std::string groupMeterName = format("{}:{}", resourceName, groupNames[(int)group]);
            AddMeter(state, groupMeterName, units, resource, EndUseCat::Invalid, "", group, outVarNum);

            if (group == Group::Building) {
                if (!ZoneName.empty()) {
                    std::string zoneMeterName = format("{}:Zone:{}", resourceName, ZoneName);
                    AddMeter(state, zoneMeterName, units, resource, EndUseCat::Invalid, "", Group::Zone, outVarNum);
                }
                if (!SpaceType.empty()) {
                    std::string spaceMeterName = format("{}:SpaceType:{}", resourceName, SpaceType);
                    AddMeter(state, spaceMeterName, units, resource, EndUseCat::Invalid, "", Group::SpaceType, outVarNum);
                }
            } // if (Group == "Building")
        }

        //!! Following if we do EndUse by ResourceType
        if (endUseCat != EndUseCat::Invalid) {
            std::string_view endUseCatName = endUseCatNames[(int)endUseCat];
            std::string enduseMeterName = format("{}:{}", endUseCatName, resourceName);
            AddMeter(state, enduseMeterName, units, resource, endUseCat, "", Group::Invalid, outVarNum);

            if (group == Group::Building) { // Match to Zone and Space
                if (!ZoneName.empty()) {
                    std::string enduseZoneMeterName = format("{}:{}:Zone:{}", endUseCatName, resourceName, ZoneName);
                    AddMeter(state, enduseZoneMeterName, units, resource, endUseCat, "", Group::Zone, outVarNum);
                }
                if (!SpaceType.empty()) {
                    std::string enduseSpaceMeterName = format("{}:{}:SpaceType:{}", endUseCatName, resourceName, SpaceType);
                    AddMeter(state, enduseSpaceMeterName, units, resource, endUseCat, "", Group::SpaceType, outVarNum);
                }
            }

            // End-Use Subcategories
            if (!endUseSub.empty()) {
                std::string subEnduseMeterName = format("{}:{}:{}", endUseSub, endUseCatNames[(int)endUseCat], resourceName);
                AddMeter(state, subEnduseMeterName, units, resource, endUseCat, endUseSub, Group::Invalid, outVarNum);

                if (group == Group::Building) { // Match to Zone and Space
                    if (!ZoneName.empty()) {
                        std::string subEnduseZoneMeterName = format("{}:{}:{}:Zone:{}", endUseSub, endUseCatName, resourceName, ZoneName);
                        AddMeter(state, subEnduseZoneMeterName, units, resource, endUseCat, endUseSub, Group::Zone, outVarNum);
                    }
                    if (!SpaceType.empty()) {
                        std::string subEnduseSpaceMeterName = format("{}:{}:{}:SpaceType:{}", endUseSub, endUseCatName, resourceName, SpaceType);
                        AddMeter(state, subEnduseSpaceMeterName, units, resource, endUseCat, endUseSub, Group::SpaceType, outVarNum);
                    }
                } // if (sovGroup == Building)
            } // if (!endUseSub.empty())
        } // if (sovEndUseCat != Invalid)
    } // AttachMeters()

    std::string standardizeEndUseSub(EndUseCat endUseCat, std::string_view endUseSubName)
    {
        if (!endUseSubName.empty()) {
            return std::string(endUseSubName);
        } else if (endUseCat == EndUseCat::Invalid) {
            return "";
        } else if (endUseCat2endUse[(int)endUseCat] != Constant::EndUse::Invalid) {
            return "General";
        } else {
            return "";
        }
    }

    OutputProcessor::RT_IPUnits GetResourceIPUnits(EnergyPlusData &state,
                                                   Constant::eResource resource, // Resource Type
                                                   Constant::Units const units,  // Meter units
                                                   bool &ErrorsFound             // true if errors found during subroutine
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2012
        //       MODIFIED       September 2012; made into subroutine
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // In order to set up tabular reports for IP units, need to search on same strings
        // that tabular reports does for IP conversion.

        // REFERENCES:
        // OutputReportTabular looks for:
        // CONSUMP - not used in meters
        // ELEC - Electricity (kWH)
        // GAS - Gas (therm)
        // COOL - Cooling (ton)
        // and we need to add WATER (for m3/gal, etc)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        RT_IPUnits IPUnits;

        // Commented this out to avoid accidentally clearing an error condition by calling this function
        // ErrorsFound = false;

        switch (resource) {
        case Constant::eResource::Electricity:
        case Constant::eResource::ElectricityProduced:
        case Constant::eResource::ElectricityPurchased:
        case Constant::eResource::ElectricitySurplusSold:
        case Constant::eResource::ElectricityNet: {
            IPUnits = RT_IPUnits::Electricity;
        } break;
        case Constant::eResource::NaturalGas: {
            IPUnits = RT_IPUnits::Gas;
        } break;
        case Constant::eResource::Water:
        case Constant::eResource::MainsWater:
        case Constant::eResource::RainWater:
        case Constant::eResource::WellWater:
        case Constant::eResource::OnSiteWater: {
            IPUnits = RT_IPUnits::Water;
        } break;
        case Constant::eResource::DistrictCooling:
        case Constant::eResource::PlantLoopCoolingDemand: {
            IPUnits = RT_IPUnits::Cooling;
        } break;
        default: {
            if (units == Constant::Units::m3) {
                IPUnits = RT_IPUnits::OtherM3;
            } else if (units == Constant::Units::kg) {
                IPUnits = RT_IPUnits::OtherKG;
            } else if (units == Constant::Units::L) {
                IPUnits = RT_IPUnits::OtherL;
            } else {
                IPUnits = RT_IPUnits::OtherJ;
            }
        } break;
        } // switch

        //  write(outputfiledebug,*) 'resourcetype=',TRIM(resourcetype)
        //  write(outputfiledebug,*) 'ipunits type=',CodeForIPUnits
        if (units != Constant::Units::kg && units != Constant::Units::J && units != Constant::Units::m3 && units != Constant::Units::L) {
            ShowWarningMessage(
                state, format("DetermineMeterIPUnits: Meter units not recognized for IP Units conversion=[{}].", Constant::unitNames[(int)units]));
            ErrorsFound = true;
        }
        return IPUnits;
    }

    void UpdateMeters(EnergyPlusData &state, int const TimeStamp) // Current TimeStamp (for max/min)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   April 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the meters with the current time step value
        // for each meter.  Also, sets min/max values for hourly...run period reporting.

        if (state.dataGlobal->WarmupFlag) {
            return;
        }

        auto &op = state.dataOutputProcessor;

        if (op->meters.size() == 0 || op->meterValues.size() == 0) {
            return;
        }

        for (int iMeter = 0; iMeter < (int)op->meters.size(); ++iMeter) {
            auto *meter = op->meters[iMeter];
            if (meter->type != MeterType::CustomDec && meter->type != MeterType::CustomDiff) {
                meter->periods[(int)ReportFreq::TimeStep].Value += op->meterValues[iMeter];
                // Is this correct? What is going on here?
            } else {
                meter->periods[(int)ReportFreq::TimeStep].Value += op->meterValues[iMeter];
                //                meter->periods[(int)ReportFreq::TimeStep].Value =
                //        op->meters[meter->decMeterNum]->periods[(int)ReportFreq::TimeStep].Value - op->meterValues[iMeter];
            }

            Real64 TSValue = meter->periods[(int)ReportFreq::TimeStep].Value;
            meter->periods[(int)ReportFreq::Hour].Value += TSValue;
            meter->periods[(int)ReportFreq::Day].Value += TSValue;
            meter->periods[(int)ReportFreq::Month].Value += TSValue;
            meter->periods[(int)ReportFreq::Year].Value += TSValue;
            meter->periods[(int)ReportFreq::Simulation].Value += TSValue;
            meter->periodFinYrSM.Value += TSValue;
        } // for (iMeter)

        // Set Max
        for (auto *meter : op->meters) {
            Real64 TSValue = meter->periods[(int)ReportFreq::TimeStep].Value;
            Real64 TSValueComp = TSValue; //  - 0.00001;

            // Todo - HRMinVal, HRMaxVal not used
            auto &periodDY = meter->periods[(int)ReportFreq::Day];
            if (TSValueComp <= periodDY.MaxVal) {
                continue;
            }
            periodDY.MaxVal = TSValue;
            periodDY.MaxValDate = TimeStamp;

            auto &periodMN = meter->periods[(int)ReportFreq::Month];
            if (TSValueComp <= periodMN.MaxVal) {
                continue;
            }
            periodMN.MaxVal = TSValue;
            periodMN.MaxValDate = TimeStamp;

            auto &periodYR = meter->periods[(int)ReportFreq::Year];
            if (TSValueComp > periodYR.MaxVal) {
                periodYR.MaxVal = TSValue;
                periodYR.MaxValDate = TimeStamp;
            }

            auto &periodSM = meter->periods[(int)ReportFreq::Simulation];
            if (TSValueComp > periodSM.MaxVal) {
                periodSM.MaxVal = TSValue;
                periodSM.MaxValDate = TimeStamp;
            }

            if (TSValueComp > meter->periodFinYrSM.MaxVal) {
                meter->periodFinYrSM.MaxVal = TSValue;
                meter->periodFinYrSM.MaxValDate = TimeStamp;
            }
        } // for (meter)

        // Set Min
        for (auto *meter : op->meters) {
            Real64 TSValue = meter->periods[(int)ReportFreq::TimeStep].Value;
            Real64 TSValueComp = TSValue; // + 0.00001;

            auto &periodDY = meter->periods[(int)ReportFreq::Day];
            if (TSValueComp >= periodDY.MinVal) {
                continue;
            }

            periodDY.MinVal = TSValue;
            periodDY.MinValDate = TimeStamp;

            auto &periodMN = meter->periods[(int)ReportFreq::Month];
            if (TSValueComp >= periodMN.MinVal) {
                continue;
            }

            periodMN.MinVal = TSValue;
            periodMN.MinValDate = TimeStamp;

            auto &periodYR = meter->periods[(int)ReportFreq::Year];
            if (TSValueComp < periodYR.MinVal) {
                periodYR.MinVal = TSValue;
                periodYR.MinValDate = TimeStamp;
            }

            auto &periodSM = meter->periods[(int)ReportFreq::Simulation];
            if (TSValueComp < periodSM.MinVal) {
                periodSM.MinVal = TSValue;
                periodSM.MinValDate = TimeStamp;
            }

            if (TSValueComp < meter->periodFinYrSM.MinVal) {
                meter->periodFinYrSM.MinVal = TSValue;
                meter->periodFinYrSM.MinValDate = TimeStamp;
            }
        } // for (meter)

        for (int iMeter = 0; iMeter < (int)op->meters.size(); ++iMeter) {
            op->meterValues[iMeter] = 0.0; // Ready for next update
        }
    } // UpdateMeters()

    void ResetAccumulationWhenWarmupComplete(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   June 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Resets the accumulating meter values. Needed after warmup period is over to
        // reset the totals on meters so that they are not accumulated over the warmup period

        auto const &op = state.dataOutputProcessor;

        for (auto *meter : op->meters) {
            for (int iPeriod = (int)ReportFreq::Hour; iPeriod < (int)ReportFreq::Num; ++iPeriod) {
                meter->periods[iPeriod].resetVals();
            }
            meter->periodFinYrSM.resetVals();
        }

        for (auto *var : op->outVars) {
            if (var->freq == ReportFreq::Month || var->freq == ReportFreq::Year || var->freq == ReportFreq::Simulation) {
                var->StoreValue = 0.0;
                var->NumStored = 0;
            }
        }
    } // ResetAccumulationWhenWarmupComplete()

    void ReportTSMeters(EnergyPlusData &state,
                        Real64 const StartMinute, // Start Minute for TimeStep
                        Real64 const EndMinute,   // End Minute for TimeStep
                        bool &PrintESOTimeStamp,  // True if the ESO Time Stamp also needs to be printed
                        bool PrintTimeStampToSQL  // Print Time Stamp to SQL file
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reports on the meters that have been requested for
        // reporting on each time step.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool PrintTimeStamp;
        int CurDayType;
        auto &op = state.dataOutputProcessor;
        auto &rf = state.dataResultsFramework->resultsFramework;
        auto &rfMetersTS = rf->Meters[(int)ReportFreq::TimeStep];

        if (!rfMetersTS.dataFrameEnabled()) {
            rf->initializeMeters(op->meters, ReportFreq::TimeStep);
        }

        PrintTimeStamp = true;
        for (int Loop = 0; Loop < (int)op->meters.size(); ++Loop) {
            auto *meter = op->meters[Loop];
            auto &periodTS = meter->periods[(int)ReportFreq::TimeStep];
            meter->CurTSValue = periodTS.Value;
            if (!periodTS.Rpt && !periodTS.accRpt) {
                continue;
            }
            if (PrintTimeStamp) {
                CurDayType = state.dataEnvrn->DayOfWeek;
                if (state.dataEnvrn->HolidayIndex > 0) {
                    CurDayType = state.dataEnvrn->HolidayIndex;
                }
                WriteTimeStampFormatData(state,
                                         state.files.mtr,
                                         ReportFreq::EachCall,
                                         op->freqStampReportNums[(int)ReportFreq::TimeStep],
                                         state.dataGlobal->DayOfSimChr,
                                         PrintTimeStamp && PrintTimeStampToSQL,
                                         state.dataEnvrn->Month,
                                         state.dataEnvrn->DayOfMonth,
                                         state.dataGlobal->HourOfDay,
                                         EndMinute,
                                         StartMinute,
                                         state.dataEnvrn->DSTIndicator,
                                         Sched::dayTypeNames[CurDayType]);
                if (rfMetersTS.dataFrameEnabled()) {
                    rfMetersTS.newRow(
                        state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth, state.dataGlobal->HourOfDay, EndMinute, state.dataGlobal->CalendarYear);
                }
                PrintTimeStamp = false;
                PrintTimeStampToSQL = false;
            }

            if (PrintESOTimeStamp && !periodTS.RptFO && !periodTS.accRptFO) {
                CurDayType = (state.dataEnvrn->HolidayIndex > 0) ? state.dataEnvrn->HolidayIndex : state.dataEnvrn->DayOfWeek;
                WriteTimeStampFormatData(state,
                                         state.files.eso,
                                         ReportFreq::EachCall,
                                         op->freqStampReportNums[(int)ReportFreq::TimeStep],
                                         state.dataGlobal->DayOfSimChr,
                                         PrintTimeStamp && PrintESOTimeStamp && PrintTimeStampToSQL,
                                         state.dataEnvrn->Month,
                                         state.dataEnvrn->DayOfMonth,
                                         state.dataGlobal->HourOfDay,
                                         EndMinute,
                                         StartMinute,
                                         state.dataEnvrn->DSTIndicator,
                                         Sched::dayTypeNames[CurDayType]);
                PrintESOTimeStamp = false;
            }

            if (periodTS.Rpt) {
                periodTS.WriteReportData(state, ReportFreq::TimeStep);
                rfMetersTS.pushVariableValue(periodTS.RptNum, periodTS.Value);
            }

            if (periodTS.accRpt) {
                WriteCumulativeReportMeterData(state, periodTS.accRptNum, periodTS.Value, periodTS.accRptFO);
                rfMetersTS.pushVariableValue(periodTS.accRptNum, periodTS.Value);
            }
        }

        for (auto *meter : op->meters) {
            meter->periods[(int)ReportFreq::TimeStep].Value = 0.0;
        }
    } // ReportTSMeters()

    void ReportMeters(EnergyPlusData &state,
                      ReportFreq freq,
                      bool PrintTimeStampToSQL // Print Time Stamp to SQL file
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reports on the meters that have been requested for
        // reporting on each hour.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool PrintTimeStamp;
        int CurDayType;
        auto &op = state.dataOutputProcessor;
        auto &rf = state.dataResultsFramework->resultsFramework;
        auto &rfMeters = rf->Meters[(int)freq];

        assert(freq == ReportFreq::Hour || freq == ReportFreq::Day || freq == ReportFreq::Month || freq == ReportFreq::Year ||
               freq == ReportFreq::Simulation);

        if (!rfMeters.dataFrameEnabled()) {
            rf->initializeMeters(op->meters, freq);
        }

        PrintTimeStamp = true;
        for (auto *meter : op->meters) {
            auto &period = meter->periods[(int)freq];

            if (freq == ReportFreq::Simulation) {
                meter->periodLastSM.Value = period.Value;
                meter->periodLastSM.MinVal = period.MinVal;
                meter->periodLastSM.MinValDate = period.MinValDate;
                meter->periodLastSM.MaxVal = period.MaxVal;
                meter->periodLastSM.MaxValDate = period.MaxValDate;
            }

            if (!period.Rpt && !period.accRpt) {
                continue;
            }
            if (PrintTimeStamp) {
                CurDayType = (state.dataEnvrn->HolidayIndex > 0) ? state.dataEnvrn->HolidayIndex : state.dataEnvrn->DayOfWeek;

                switch (freq) {

                case ReportFreq::Hour: {
                    WriteTimeStampFormatData(state,
                                             state.files.mtr,
                                             freq,
                                             op->freqStampReportNums[(int)freq],
                                             state.dataGlobal->DayOfSimChr,
                                             PrintTimeStamp && PrintTimeStampToSQL,
                                             state.dataEnvrn->Month,
                                             state.dataEnvrn->DayOfMonth,
                                             state.dataGlobal->HourOfDay,
                                             -1, // EndMinute
                                             -1, // StartMinute
                                             state.dataEnvrn->DSTIndicator,
                                             Sched::dayTypeNames[CurDayType]);
                } break;

                case ReportFreq::Day: {
                    WriteTimeStampFormatData(state,
                                             state.files.mtr,
                                             freq,
                                             op->freqStampReportNums[(int)freq],
                                             state.dataGlobal->DayOfSimChr,
                                             PrintTimeStamp && PrintTimeStampToSQL,
                                             state.dataEnvrn->Month,
                                             state.dataEnvrn->DayOfMonth,
                                             -1, // Hour
                                             -1, // EndMinute
                                             -1, // StartMinute
                                             state.dataEnvrn->DSTIndicator,
                                             Sched::dayTypeNames[CurDayType]);
                } break;

                case ReportFreq::Month: {
                    WriteTimeStampFormatData(state,
                                             state.files.mtr,
                                             freq,
                                             op->freqStampReportNums[(int)freq],
                                             state.dataGlobal->DayOfSimChr,
                                             PrintTimeStamp && PrintTimeStampToSQL,
                                             state.dataEnvrn->Month);
                } break;

                case ReportFreq::Year: {
                    WriteYearlyTimeStamp(state,
                                         state.files.mtr,
                                         op->freqStampReportNums[(int)freq],
                                         state.dataGlobal->CalendarYearChr,
                                         PrintTimeStamp && PrintTimeStampToSQL);
                } break;

                case ReportFreq::Simulation: {
                    WriteTimeStampFormatData(state,
                                             state.files.mtr,
                                             freq,
                                             op->freqStampReportNums[(int)freq],
                                             state.dataGlobal->DayOfSimChr,
                                             PrintTimeStamp && PrintTimeStampToSQL);
                } break;

                default: {
                } break;
                } // switch (freq)

                if (rfMeters.dataFrameEnabled()) {
                    rfMeters.newRow(
                        state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth, state.dataGlobal->HourOfDay, 0, state.dataGlobal->CalendarYear);
                }
                PrintTimeStamp = false;
                PrintTimeStampToSQL = false;
            }

            if (period.Rpt) {
                period.WriteReportData(state, freq);
                rfMeters.pushVariableValue(period.RptNum, period.Value);
                period.Value = 0.0;

                if (freq != ReportFreq::Hour) {
                    period.MinVal = MinSetValue;
                    period.MaxVal = MaxSetValue;
                }
            }

            if (period.accRpt) {
                WriteCumulativeReportMeterData(state, period.accRptNum, meter->periods[(int)ReportFreq::Simulation].Value, period.accRptFO);
                rfMeters.pushVariableValue(period.accRptNum, meter->periods[(int)ReportFreq::Simulation].Value);
            }
        } // for (meter)
    } // ReportMeters()

    void ReportForTabularReports(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   August 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is called after all the simulation is done and before
        // tabular reports in order to reduce the number of calls to the predefined routine
        // for SM (Simulation period) meters, the value of the last calculation is stored
        // in the data structure.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto const &op = state.dataOutputProcessor;

        for (auto *meter : op->meters) {
            auto &period = meter->periodFinYrSM;

            switch (meter->RT_forIPUnits) {
            case RT_IPUnits::Electricity: {
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMelecannual, meter->Name, period.Value * Constant::convertJtoGJ);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMelecminvalue, meter->Name, period.MinVal / state.dataGlobal->TimeStepZoneSec);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMelecminvaluetime, meter->Name, DateToStringWithMonth(period.MinValDate));
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMelecmaxvalue, meter->Name, period.MaxVal / state.dataGlobal->TimeStepZoneSec);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMelecmaxvaluetime, meter->Name, DateToStringWithMonth(period.MaxValDate));
            } break;

            case RT_IPUnits::Gas: {
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMgasannual, meter->Name, period.Value * Constant::convertJtoGJ);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMgasminvalue, meter->Name, period.MinVal / state.dataGlobal->TimeStepZoneSec);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMgasminvaluetime, meter->Name, DateToStringWithMonth(period.MinValDate));
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMgasmaxvalue, meter->Name, period.MaxVal / state.dataGlobal->TimeStepZoneSec);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMgasmaxvaluetime, meter->Name, DateToStringWithMonth(period.MaxValDate));
            } break;

            case RT_IPUnits::Cooling: {
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMcoolannual, meter->Name, period.Value * Constant::convertJtoGJ);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMcoolminvalue, meter->Name, period.MinVal / state.dataGlobal->TimeStepZoneSec);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMcoolminvaluetime, meter->Name, DateToStringWithMonth(period.MinValDate));
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMcoolmaxvalue, meter->Name, period.MaxVal / state.dataGlobal->TimeStepZoneSec);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMcoolmaxvaluetime, meter->Name, DateToStringWithMonth(period.MaxValDate));
            } break;

            case RT_IPUnits::Water: {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEMwaterannual, meter->Name, period.Value);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMwaterminvalue, meter->Name, period.MinVal / state.dataGlobal->TimeStepZoneSec);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMwaterminvaluetime, meter->Name, DateToStringWithMonth(period.MinValDate));
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMwatermaxvalue, meter->Name, period.MaxVal / state.dataGlobal->TimeStepZoneSec);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMwatermaxvaluetime, meter->Name, DateToStringWithMonth(period.MaxValDate));
            } break;

            case RT_IPUnits::OtherKG: {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEMotherKGannual, meter->Name, period.Value);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherKGminvalue, meter->Name, period.MinVal / state.dataGlobal->TimeStepZoneSec, 3);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherKGminvaluetime, meter->Name, DateToStringWithMonth(period.MinValDate));
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherKGmaxvalue, meter->Name, period.MaxVal / state.dataGlobal->TimeStepZoneSec, 3);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherKGmaxvaluetime, meter->Name, DateToStringWithMonth(period.MaxValDate));
            } break;

            case RT_IPUnits::OtherM3: {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEMotherM3annual, meter->Name, period.Value, 3);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherM3minvalue, meter->Name, period.MinVal / state.dataGlobal->TimeStepZoneSec, 3);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherM3minvaluetime, meter->Name, DateToStringWithMonth(period.MinValDate));
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherM3maxvalue, meter->Name, period.MaxVal / state.dataGlobal->TimeStepZoneSec, 3);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherM3maxvaluetime, meter->Name, DateToStringWithMonth(period.MaxValDate));
            } break;

            case RT_IPUnits::OtherL: {
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchEMotherLannual, meter->Name, period.Value, 3);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherLminvalue, meter->Name, period.MinVal / state.dataGlobal->TimeStepZoneSec, 3);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherLminvaluetime, meter->Name, DateToStringWithMonth(period.MinValDate));
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherLmaxvalue, meter->Name, period.MaxVal / state.dataGlobal->TimeStepZoneSec, 3);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherLmaxvaluetime, meter->Name, DateToStringWithMonth(period.MaxValDate));
            } break;

            default: {
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherJannual, meter->Name, period.Value * Constant::convertJtoGJ);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherJminvalue, meter->Name, period.MinVal / state.dataGlobal->TimeStepZoneSec);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherJminvaluetime, meter->Name, DateToStringWithMonth(period.MinValDate));
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherJmaxvalue, meter->Name, period.MaxVal / state.dataGlobal->TimeStepZoneSec);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchEMotherJmaxvaluetime, meter->Name, DateToStringWithMonth(period.MaxValDate));
            } break;
            } // switch
        } // for (meter)
    } // ReportForTabularReports()

    std::string DateToStringWithMonth(int const codedDate) // word containing encoded month, day, hour, minute
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   August 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Convert the coded date format into a usable
        //   string

        if (codedDate == 0) {
            return "-";
        }

        static constexpr std::string_view DateFmt("{:02}-{:3}-{:02}:{:02}");

        // ((month*100 + day)*100 + hour)*100 + minute
        int Month;  // month in integer format (1-12)
        int Day;    // day in integer format (1-31)
        int Hour;   // hour in integer format (1-24)
        int Minute; // minute in integer format (0:59)

        General::DecodeMonDayHrMin(codedDate, Month, Day, Hour, Minute);

        if (Month < 1 || Month > 12) {
            return "-";
        }
        if (Day < 1 || Day > 31) {
            return "-";
        }
        if (Hour < 1 || Hour > 24) {
            return "-";
        }
        if (Minute < 0 || Minute > 60) {
            return "-";
        }

        --Hour;
        if (Minute == 60) {
            ++Hour;
            Minute = 0;
        }

        std::string monthName;
        switch (Month) {
        case 1:
            monthName = "JAN";
            break;
        case 2:
            monthName = "FEB";
            break;
        case 3:
            monthName = "MAR";
            break;
        case 4:
            monthName = "APR";
            break;
        case 5:
            monthName = "MAY";
            break;
        case 6:
            monthName = "JUN";
            break;
        case 7:
            monthName = "JUL";
            break;
        case 8:
            monthName = "AUG";
            break;
        case 9:
            monthName = "SEP";
            break;
        case 10:
            monthName = "OCT";
            break;
        case 11:
            monthName = "NOV";
            break;
        case 12:
            monthName = "DEC";
            break;
        default:
            assert(false);
        }

        return format(DateFmt, Day, monthName, Hour, Minute);
    }

    std::string OutVar::multiplierString() const
    {
        return (ZoneMult == 1 && ZoneListMult == 1)
                   ? ""
                   : format(" * {}  (Zone Multiplier = {}, Zone List Multiplier = {})", ZoneMult * ZoneListMult, ZoneMult, ZoneListMult);
    }

    void ReportMeterDetails(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   January 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Writes the meter details report.  This shows which variables are on
        // meters as well as the meter contents.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &op = state.dataOutputProcessor;

        for (auto const *var : op->outVars) {

            if (var->meterNums.empty()) {
                continue;
            }

            print(state.files.mtd,
                  "\n Meters for {},{} [{}]{}\n",
                  var->ReportID,
                  var->keyColonName,
                  Constant::unitNames[(int)var->units],
                  var->multiplierString());

            for (int const meterNum : var->meterNums) {
                auto const *meter = op->meters[meterNum];

                print(state.files.mtd,
                      "  On{}Meter={} [{}]\n",
                      (meter->type == MeterType::Normal) ? "" : "Custom",
                      meter->Name,
                      Constant::unitNames[(int)meter->units]);
            }
        } // for (var)

        for (auto const *meter : op->meters) {

            print(state.files.mtd, "\n For Meter={} [{}]", meter->Name, Constant::unitNames[(int)meter->units]);
            if (meter->resource != Constant::eResource::Invalid) {
                print(state.files.mtd, ", ResourceType={}", Constant::eResourceNames[(int)meter->resource]);
            }
            if (meter->endUseCat != EndUseCat::Invalid) {
                print(state.files.mtd, ", EndUse={}", endUseCatNames[(int)meter->endUseCat]);
            }
            if (meter->group != Group::Invalid) {
                print(state.files.mtd, ", Group={}", groupNames[(int)meter->group]);
            }
            print(state.files.mtd, ", contents are:\n");

            if (meter->type == MeterType::Normal) {
                for (int srcVarNum : meter->srcVarNums) {
                    auto const *var = op->outVars[srcVarNum];
                    print(state.files.mtd, "  {}{}\n", var->keyColonName, var->multiplierString());
                }

            } else if (meter->type == MeterType::Custom) {
                for (int srcVarNum : meter->srcVarNums) {
                    auto const *var = op->outVars[srcVarNum];
                    print(state.files.mtd, "  {}{}\n", var->keyColonName, var->multiplierString());
                }

            } else if (meter->type == MeterType::CustomDec) {
                print(state.files.mtd,
                      " Values for this meter will be Source Meter={}; but will be decremented by:\n",
                      op->meters[meter->decMeterNum]->Name);
                for (int srcVarNum : meter->srcVarNums) {
                    auto const *var = op->outVars[srcVarNum];
                    print(state.files.mtd, "  {}{}\n", var->keyColonName, var->multiplierString());
                }
            }
        } // for (meter)
    } // ReportMeterDetails()

    // *****************************************************************************
    // End of routines for Energy Meters implementation in EnergyPlus.
    // *****************************************************************************

    void WriteTimeStampFormatData(
        EnergyPlusData &state,
        InputOutputFile &outputFile,
        ReportFreq const reportingInterval, // See Module Parameter Definitions for ReportEach, ReportTimeStep, ReportHourly, etc.
        int const reportID,                 // The ID of the time stamp
        std::string const &DayOfSimChr,     // the number of days simulated so far
        bool writeToSQL,
        int const Month,               // the month of the reporting interval
        int const DayOfMonth,          // The day of the reporting interval
        int const Hour,                // The hour of the reporting interval
        Real64 const EndMinute,        // The last minute in the reporting interval
        Real64 const StartMinute,      // The starting minute of the reporting interval
        int const DST,                 // A flag indicating whether daylight savings time is observed
        std::string_view const DayType // The day tied for the data (e.g., Monday)
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   July 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function reports the timestamp data for the output processor
        // Much of the code in this function was embedded in earlier versions of EnergyPlus
        // and was moved to this location to simplify maintenance and to allow for data output
        // to the SQL database

        std::string reportStr = (reportID == -1) ? "" : std::to_string(reportID);

        assert(reportStr.length() + DayOfSimChr.length() + (DayType.length()) + 26 < N_WriteTimeStampFormatData); // Check will fit in stamp size

        if (!outputFile.good()) {
            return;
        }

        auto &sql = state.dataSQLiteProcedures->sqlite;

        switch (reportingInterval) {
        case ReportFreq::EachCall:
        case ReportFreq::TimeStep: {
            assert(Month != -1 && DayOfMonth != -1 && Hour != -1 && StartMinute != -1 && EndMinute != -1 && DST != -1 && !DayType.empty());
            print<FormatSyntax::FMT>(outputFile,
                                     "{},{},{:2d},{:2d},{:2d},{:2d},{:5.2f},{:5.2f},{}\n",
                                     reportStr,
                                     DayOfSimChr,
                                     Month,
                                     DayOfMonth,
                                     DST,
                                     Hour,
                                     StartMinute,
                                     EndMinute,
                                     DayType);

            if (writeToSQL && sql) {
                sql->createSQLiteTimeIndexRecord(reportingInterval,
                                                 reportID,
                                                 state.dataGlobal->DayOfSim,
                                                 state.dataEnvrn->CurEnvirNum,
                                                 state.dataGlobal->CalendarYear,
                                                 state.dataEnvrn->CurrentYearIsLeapYear,
                                                 Month,
                                                 DayOfMonth,
                                                 Hour,
                                                 EndMinute,
                                                 StartMinute,
                                                 DST,
                                                 DayType,
                                                 state.dataGlobal->WarmupFlag);
            }
        } break;

        case ReportFreq::Hour: {
            assert(Month != -1 && DayOfMonth != -1 && Hour != -1 && DST != -1 && !DayType.empty());
            print<FormatSyntax::FMT>(outputFile,
                                     "{},{},{:2d},{:2d},{:2d},{:2d},{:5.2f},{:5.2f},{}\n",
                                     reportStr,
                                     DayOfSimChr,
                                     Month,
                                     DayOfMonth,
                                     DST,
                                     Hour,
                                     0.0,
                                     60.0,
                                     DayType);
            if (writeToSQL && sql) {
                sql->createSQLiteTimeIndexRecord(reportingInterval,
                                                 reportID,
                                                 state.dataGlobal->DayOfSim,
                                                 state.dataEnvrn->CurEnvirNum,
                                                 state.dataGlobal->CalendarYear,
                                                 state.dataEnvrn->CurrentYearIsLeapYear,
                                                 Month,
                                                 DayOfMonth,
                                                 Hour,
                                                 -1, // EndMinute
                                                 -1, // StartMinute
                                                 DST,
                                                 DayType,
                                                 state.dataGlobal->WarmupFlag);
            }
        } break;
        case ReportFreq::Day: {
            assert(Month != -1 && DayOfMonth != -1 && DST != -1 && !DayType.empty());
            print<FormatSyntax::FMT>(outputFile, "{},{},{:2d},{:2d},{:2d},{}\n", reportStr, DayOfSimChr, Month, DayOfMonth, DST, DayType);
            if (writeToSQL && sql) {
                sql->createSQLiteTimeIndexRecord(reportingInterval,
                                                 reportID,
                                                 state.dataGlobal->DayOfSim,
                                                 state.dataEnvrn->CurEnvirNum,
                                                 state.dataGlobal->CalendarYear,
                                                 state.dataEnvrn->CurrentYearIsLeapYear,
                                                 Month,
                                                 DayOfMonth,
                                                 -1, // Hour
                                                 -1, // EndMinute
                                                 -1, // StartMinute
                                                 DST,
                                                 DayType,
                                                 state.dataGlobal->WarmupFlag);
            }
        } break;

        case ReportFreq::Month: {
            assert(Month != -1);
            print<FormatSyntax::FMT>(outputFile, "{},{},{:2d}\n", reportStr, DayOfSimChr, Month);
            if (writeToSQL && sql) {
                sql->createSQLiteTimeIndexRecord(reportingInterval,
                                                 reportID,
                                                 state.dataGlobal->DayOfSim,
                                                 state.dataEnvrn->CurEnvirNum,
                                                 state.dataGlobal->CalendarYear,
                                                 state.dataEnvrn->CurrentYearIsLeapYear,
                                                 Month);
            }
        } break;

        case ReportFreq::Simulation: {
            print<FormatSyntax::FMT>(outputFile, "{},{}\n", reportStr, DayOfSimChr);
            if (writeToSQL && sql) {
                sql->createSQLiteTimeIndexRecord(reportingInterval,
                                                 reportID,
                                                 state.dataGlobal->DayOfSim,
                                                 state.dataEnvrn->CurEnvirNum,
                                                 state.dataGlobal->CalendarYear,
                                                 state.dataEnvrn->CurrentYearIsLeapYear);
            }
        } break;
        default: {
            if (sql) {
                sql->sqliteWriteMessage(
                    format<FormatSyntax::FMT>("Illegal reportingInterval passed to WriteTimeStampFormatData: {}", (int)reportingInterval));
            }
        } break;
        } // switch (reportFreq)
    } // WriteTimeStampFormatData()

    void WriteYearlyTimeStamp(EnergyPlusData &state,
                              InputOutputFile &outputFile,
                              int reportID,                    // The ID of the time stamp
                              std::string const &yearOfSimChr, // the year of the simulation
                              bool writeToSQL)
    {
        print(outputFile, "{},{}\n", reportID, yearOfSimChr);
        auto &sql = state.dataSQLiteProcedures->sqlite;
        if (writeToSQL && sql) {
            sql->createYearlyTimeIndexRecord(state.dataGlobal->CalendarYear, state.dataEnvrn->CurEnvirNum);
        }
    } // WriteYearlyTimeStamp()

    void OutVar::writeReportDictionaryItem(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   August 2008
        //       MODIFIED       April 2011; Linda Lawrie
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes the ESO data dictionary information to the output files
        // and the SQL database

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &rf = state.dataResultsFramework->resultsFramework;
        auto &sql = state.dataSQLiteProcedures->sqlite;

        std::string_view unitsString = (units == Constant::Units::customEMS && !unitNameCustomEMS.empty())
                                           ? unitNameCustomEMS
                                           : ((units == Constant::Units::Invalid) ? "" : Constant::unitNames[(int)units]);

        std::string schedString = (sched != nullptr) ? sched->Name : "";

        if (state.files.eso.good()) {
            print(state.files.eso,
                  "{},{},{},{} [{}]{}{}{}\n",
                  ReportID,
                  reportFreqArbitraryInts[(int)freq],
                  key,
                  name,
                  unitsString,
                  reportingFrequencyNoticeStrings[(int)freq],
                  !schedString.empty() ? "," : "",
                  schedString);
        }

        if (freq == ReportFreq::Hour || freq == ReportFreq::Day || freq == ReportFreq::Month || freq == ReportFreq::Year ||
            freq == ReportFreq::Simulation) {
            state.dataOutputProcessor->freqTrackingVariables[(int)freq] = true;
        }

        if (sql) {
            sql->createSQLiteReportDictionaryRecord(ReportID, storeType, indexGroup, key, name, timeStepType, unitsString, freq, false, schedString);
        }

        // add to ResultsFramework for output variable list, need to check RVI/MVI later
        rf->addReportVariable(key, name, unitsString, freq);

    } // OutVar::WriteReportDictionaryItem()

    void WriteMeterDictionaryItem(EnergyPlusData &state,
                                  ReportFreq const freq, // The reporting interval (e.g., hourly, daily)
                                  StoreType const storeType,
                                  int const reportID,             // The reporting ID in for the variable
                                  std::string const &indexGroup,  // The reporting group for the variable
                                  std::string const &meterName,   // The variable's meter name
                                  Constant::Units const units,    // The variables units
                                  bool const cumulativeMeterFlag, // A flag indicating cumulative data
                                  bool const meterFileOnlyFlag    // A flag indicating whether the data is to be written to standard output
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   August 2008
        //       MODIFIED       April 2011; Linda Lawrie
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // The subroutine writes meter data dictionary information to the output files
        // and the SQL database. Much of the code here was embedded in other subroutines
        // and was moved here for the purposes of ease of maintenance and to allow easy
        // data reporting to the SQL database

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &rf = state.dataResultsFramework->resultsFramework;
        auto &sql = state.dataSQLiteProcedures->sqlite;

        std::string FreqString = std::string(reportingFrequencyNoticeStrings[(int)freq]);
        std::string FreqString2 = FreqString.substr(0, index(FreqString, '['));

        const auto print_meter = [&](EnergyPlusData &state, const int frequency) {
            const auto out = [&](InputOutputFile &of) {
                if (of.good()) {
                    if (cumulativeMeterFlag) {
                        static constexpr std::string_view fmt = "{},{},Cumulative {} [{}]{}\n";
                        print(of, fmt, reportID, 1, meterName, Constant::unitNames[(int)units], FreqString2);
                    } else {
                        static constexpr std::string_view fmt = "{},{},{} [{}]{}\n";
                        print(of, fmt, reportID, frequency, meterName, Constant::unitNames[(int)units], FreqString);
                    }
                }
            };

            out(state.files.mtr);
            if (!meterFileOnlyFlag) {
                out(state.files.eso);
            }
        };

        print_meter(state, reportFreqArbitraryInts[(int)freq]);

        static constexpr std::string_view keyedValueStringCum("Cumulative ");
        static constexpr std::string_view keyedValueStringNon;
        std::string_view const keyedValueString(cumulativeMeterFlag ? keyedValueStringCum : keyedValueStringNon);

        if (sql) {
            sql->createSQLiteReportDictionaryRecord(
                reportID, storeType, indexGroup, keyedValueString, meterName, TimeStepType::Zone, Constant::unitNames[(int)units], freq, true);
        }

        // add to ResultsFramework for output variable list, need to check RVI/MVI later
        rf->addReportMeter(meterName, Constant::unitNames[(int)units], freq);

    } // WriteMeterDictionaryItem()

    void OutVar::writeOutput(EnergyPlusData &state,
                             ReportFreq const reportFreq // The report type or interval (e.g., hourly)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   August 2008
        //       MODIFIED       April 2011; Linda Lawrie, December 2017; Jason DeGraw
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes real report variable data to the output file and
        // SQL database. Much of the code here was an included in earlier versions
        // of the UpdateDataandReport subroutine. The code was moved to facilitate
        // easier maintenance and writing of data to the SQL database.

        if (state.dataSysVars->UpdateDataDuringWarmupExternalInterface && !state.dataSysVars->ReportDuringWarmup) {
            return;
        }

        if (!Report || freq != reportFreq || !Stored) {
            return;
        }

        if (NumStored > 0.0) {
            writeReportData(state);
            ++state.dataGlobal->StdOutputRecordCount;
        }

        StoreValue = 0.0;
        NumStored = 0.0;
        MinValue = MinSetValue;
        MaxValue = MaxSetValue;
        Stored = false;
    } // OutVar::WriteOutput()

    void WriteCumulativeReportMeterData(EnergyPlusData &state,
                                        int const reportID,      // The variable's report ID
                                        Real64 const repValue,   // The variable's value
                                        bool const meterOnlyFlag // A flag that indicates if the data should be written to standard output
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   July 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes the cumulative meter data to the output files and
        // SQL database.

        std::string NumberOut; // Character for producing "number out"
        auto &sql = state.dataSQLiteProcedures->sqlite;

        if (repValue == 0.0) {
            NumberOut = "0.0";
        } else {
            char meterData[129];
            dtoa(repValue, meterData);
            NumberOut = std::string(meterData);
        }

        if (sql) {
            sql->createSQLiteReportDataRecord(reportID, repValue);
        }

        if (state.files.mtr.good()) {
            print(state.files.mtr, "{},{}\n", reportID, NumberOut);
        }
        ++state.dataGlobal->StdMeterRecordCount;

        if (!meterOnlyFlag) {
            if (state.files.eso.good()) {
                print(state.files.eso, "{},{}\n", reportID, NumberOut);
            }
            ++state.dataGlobal->StdOutputRecordCount;
        }
    } // WriteCumulativeReportMeterData()

    void MeterPeriod::WriteReportData(EnergyPlusData &state, ReportFreq const freq)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   July 2008

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes for the non-cumulative meter data to the output files and
        // SQL database.

        auto &sql = state.dataSQLiteProcedures->sqlite;

        std::string NumberOut;

        if (Value == 0.0) {
            NumberOut = "0.0";
        } else {
            char tmp[128];
            dtoa(Value, tmp);
            NumberOut = std::string(tmp);
        }

        if (sql) {
            sql->createSQLiteReportDataRecord(RptNum, Value, freq, MinVal, MinValDate, MaxVal, MaxValDate, state.dataGlobal->MinutesInTimeStep);
        }

        if ((freq == ReportFreq::EachCall) || (freq == ReportFreq::TimeStep) || (freq == ReportFreq::Hour)) { // -1, 0, 1
            if (state.files.mtr.good()) {
                print(state.files.mtr, "{},{}\n", RptNum, NumberOut);
            }
            ++state.dataGlobal->StdMeterRecordCount;
            if (state.files.eso.good() && !RptFO) {
                print(state.files.eso, "{},{}\n", RptNum, NumberOut);
                ++state.dataGlobal->StdOutputRecordCount;
            }
        } else { // if ( ( reportingInterval == ReportDaily ) || ( reportingInterval == ReportMonthly ) || ( reportingInterval == ReportSim ) ) {
                 // // 2, 3, 4
            // Append the min and max strings with date information
            char minValString[128], maxValString[128];
            dtoa(MinVal, minValString);
            dtoa(MaxVal, maxValString);

            std::string minDateString = produceDateString(MinValDate, freq);
            std::string maxDateString = produceDateString(MaxValDate, freq);

            if (state.files.mtr.good()) {
                print(state.files.mtr, "{},{},{},{},{},{}\n", RptNum, NumberOut, minValString, minDateString, maxValString, maxDateString);
            }

            ++state.dataGlobal->StdMeterRecordCount;
            if (state.files.eso.good() && !RptFO) {
                print(state.files.eso, "{},{},{},{},{},{}\n", RptNum, NumberOut, minValString, minDateString, maxValString, maxDateString);
                ++state.dataGlobal->StdOutputRecordCount;
            }
        }
    } // MeterPeriod::WriteReportData()

    void WriteNumericData(EnergyPlusData &state,
                          int const reportID,   // The variable's reporting ID
                          Real64 const repValue // The variable's value
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Mark Adams
        //       DATE WRITTEN   May 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE:
        // This subroutine writes real data to the output files and
        // SQL database.
        // This is a refactor of WriteRealData.
        //
        // Much of the code here was an included in earlier versions
        // of the UpdateDataandReport subroutine. The code was moved to facilitate
        // easier maintenance and writing of data to the SQL database.
        auto &sql = state.dataSQLiteProcedures->sqlite;

        if (state.dataSysVars->UpdateDataDuringWarmupExternalInterface && !state.dataSysVars->ReportDuringWarmup) {
            return;
        }

        if (sql) {
            sql->createSQLiteReportDataRecord(reportID, repValue);
        }

        if (state.files.eso.good()) {
            char numericData[129];
            dtoa(repValue, numericData);
            print<FormatSyntax::FMT>(state.files.eso, "{},{}\n", reportID, numericData);
        }
    } // WriteNumericData()

    void WriteNumericData(EnergyPlusData &state,
                          int const reportID,    // The variable's reporting ID
                          int32_t const repValue // The variable's value
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Mark Adams
        //       DATE WRITTEN   May 2016
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE:
        // This subroutine writes real data to the output files and
        // SQL database.
        // This is a refactor of WriteIntegerData.
        //
        // Much of the code here was an included in earlier versions
        // of the UpdateDataandReport subroutine. The code was moved to facilitate
        // easier maintenance and writing of data to the SQL database.

        //        i32toa(repValue, state.dataOutputProcessor->s_WriteNumericData);
        auto &sql = state.dataSQLiteProcedures->sqlite;

        if (sql) {
            sql->createSQLiteReportDataRecord(reportID, repValue);
        }

        if (state.files.eso.good()) {
            print<FormatSyntax::FMT>(state.files.eso, "{},{}\n", reportID, fmt::format_int(repValue).c_str());
        }
    } // WriteNumericData()

    void OutVar::writeReportData(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes averaged integer data to the output files and
        // SQL database. It supports the WriteIntegerVariableOutput subroutine.
        // Much of the code here was an included in earlier versions
        // of the UpdateDataandReport subroutine. The code was moved to facilitate
        // easier maintenance and writing of data to the SQL database.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &rf = state.dataResultsFramework->resultsFramework;
        auto &sql = state.dataSQLiteProcedures->sqlite;

        Real64 repVal = (storeType == StoreType::Average) ? (StoreValue / NumStored) : StoreValue;

        // Append the min and max strings with date information
        if (rf->timeSeriesEnabled() &&
            (freq == ReportFreq::Day || freq == ReportFreq::Month || freq == ReportFreq::Year || freq == ReportFreq::Simulation)) {
            // add to daily TS data store
            rf->freqTSData[(int)freq].pushVariableValue(ReportID, repVal);
        }

        if (sql) {
            sql->createSQLiteReportDataRecord(ReportID, repVal, freq, MinValue, minValueDate, MaxValue, maxValueDate);
        }

        if (state.files.eso.good()) {
            std::string NumberOut;
            if (varType == VariableType::Real) {
                char tmp[128];
                dtoa(repVal, tmp);
                NumberOut = std::string(tmp);
            } else {
                // Can someone explain why we are printing integers as
                // floats and why we are doing it differently than
                // floats?
                NumberOut = (repVal == 0.0) ? "0.0" : format("{:f}", repVal);
            }

            if ((freq == ReportFreq::EachCall) || (freq == ReportFreq::TimeStep) || (freq == ReportFreq::Hour)) { // -1, 0, 1
                print(state.files.eso, "{},{}\n", ReportID, NumberOut);
            } else {
                char minValString[128], maxValString[128];
                dtoa(MinValue, minValString);
                dtoa(MaxValue, maxValString);

                std::string minDateString = produceDateString(minValueDate, freq);
                std::string maxDateString = produceDateString(maxValueDate, freq);

                print(state.files.eso, "{},{},{},{},{},{}\n", ReportID, NumberOut, minValString, minDateString, maxValString, maxDateString);
            }
        }
    } // OutVar::WriteReportData()

    int DetermineIndexGroupKeyFromMeterName([[maybe_unused]] EnergyPlusData &state, std::string const &meterName) // the meter name
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   May 2009

        // PURPOSE OF THIS FUNCTION:
        // This function attemps to guess determine how a meter variable should be
        // grouped.  It does this by parsing the meter name and then assigns a
        // indexGroupKey based on the name

        // Facility indices are in the 100s
        if (has(meterName, "Electricity:Facility")) {
            return 100;
        } else if (has(meterName, "NaturalGas:Facility")) {
            return 101;
        } else if (has(meterName, "DistricHeatingWater:Facility")) {
            return 102;
        } else if (has(meterName, "DistricCooling:Facility")) {
            return 103;
        } else if (has(meterName, "ElectricityNet:Facility")) {
            return 104;

            // Building indices are in the 200s
        } else if (has(meterName, "Electricity:Building")) {
            return 201;
        } else if (has(meterName, "NaturalGas:Building")) {
            return 202;

            // HVAC indices are in the 300s
        } else if (has(meterName, "Electricity:HVAC")) {
            return 301;

            // InteriorLights:Electricity:Zone indices are in the 500s
        } else if (has(meterName, "InteriorLights:Electricity:Zone")) {
            return 501;

            // InteriorLights:Electricity indices are in the 400s
        } else if (has(meterName, "InteriorLights:Electricity")) {
            return 401;

            // Unknown items have negative indices
        } else {
            return -11;
        }

        return -1;
    } // DetermineIndexGroupKeyFromMeterName()

    std::string DetermineIndexGroupFromMeterGroup(Meter const *meter) // the meter
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Greg Stark
        //       DATE WRITTEN   May 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function attemps to determine how a meter variable should be
        // grouped.  It does this by parsing the meter group

        // Return value
        std::string indexGroup;

        if (meter->group != Group::Invalid) {
            indexGroup = groupNames[(int)meter->group];
        } else {
            indexGroup = "Facility";
        }

        if (meter->resource != Constant::eResource::Invalid) {
            indexGroup += format(":{}", Constant::eResourceNames[(int)meter->resource]);
        }

        if (meter->endUseCat != EndUseCat::Invalid) {
            indexGroup += format(":{}", endUseCatNames[(int)meter->endUseCat]);
        }

        if (len(meter->EndUseSub) > 0) {
            indexGroup += ":" + meter->EndUseSub;
        }

        return indexGroup;
    } // DetermineIndexGroupFromMeterGroup()

    void SetInternalVariableValue(EnergyPlusData &state,
                                  OutputProcessor::VariableType const varType, // 1=integer, 2=real, 3=meter
                                  int const keyVarIndex,                       // Array index
                                  Real64 const SetRealVal,                     // real value to set, if type is real or meter
                                  int const SetIntVal                          // integer value to set if type is integer
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   August 2012

        // PURPOSE OF THIS SUBROUTINE:
        // This is a simple set routine for output pointers
        // It is intended for special use to reinitializations those pointers used for EMS sensors

        // METHODOLOGY EMPLOYED:
        // given a variable type and variable index,
        // assign the pointers the values passed in.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &op = state.dataOutputProcessor;

        if (varType == VariableType::Integer) {
            OutVarInt *varInt = dynamic_cast<OutVarInt *>(op->outVars[keyVarIndex]);
            assert(varInt != nullptr);
            *varInt->Which = SetIntVal;
        } else if (varType == VariableType::Real) {
            OutVarReal *varReal = dynamic_cast<OutVarReal *>(op->outVars[keyVarIndex]);
            assert(varReal != nullptr);
            *varReal->Which = SetRealVal;
        } else if (varType == VariableType::Meter) {
            op->meters[keyVarIndex]->CurTSValue = SetRealVal;
        }
    } // SetInternalVariableValue()

    // returns the unit string for a DDVariableTypes item and custom string when customEMS is used
    std::string unitStringFromDDitem(EnergyPlusData &state, int const ddNum // index provided for DDVariableTypes
    )
    {
        // This function is here just for unit test purposes
        DDOutVar *ddVar = state.dataOutputProcessor->ddOutVars[ddNum];
        Constant::Units units = ddVar->units;
        return format(" [{}]", units == Constant::Units::customEMS ? ddVar->unitNameCustomEMS : Constant::unitNames[(int)units]);
    } // unitStringFromDDitem()

} // namespace OutputProcessor

// TODO: Probably move these to a different location

void SetupOutputVariable(EnergyPlusData &state,
                         std::string_view const name,            // String Name of variable (with units)
                         Constant::Units const units,            // Actual units corresponding to the actual variable
                         Real64 &ActualVariable,                 // Actual Variable, used to set up pointer
                         OutputProcessor::TimeStepType timeStep, // Zone, HeatBalance=1, HVAC, System, Plant=2
                         OutputProcessor::StoreType store,       // State, Average=1, NonState, Sum=2
                         std::string const &key,                 // Associated Key for this variable
                         Constant::eResource resource,           // Meter Resource Type (Electricity, Gas, etc)
                         OutputProcessor::Group group,           // Meter Super Group Key (Building, System, Plant)
                         OutputProcessor::EndUseCat endUseCat,   // Meter End Use Key (Lights, Heating, Cooling, etc)
                         std::string_view const EndUseSub,       // Meter End Use Sub Key (General Lights, Task Lights, etc)
                         std::string const &zone,                // Meter Zone Key (zone name)
                         int const ZoneMult,                     // Zone Multiplier, defaults to 1
                         int const ZoneListMult,                 // Zone List Multiplier, defaults to 1
                         std::string const &spaceType,           // Space type (applicable for Building group only)
                         int const indexGroupKey,                // Group identifier for SQL output
                         std::string_view const customUnitName,  // the custom name for the units from EMS definition of units
                         OutputProcessor::ReportFreq freq        // Internal use -- causes reporting at this frequency
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   December 1998
    //       MODIFIED       January 2001; Implement Meters
    //                      August 2008; Implement SQL output
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets up the variable data structure that will be used
    // to track values of the output variables of EnergyPlus.

    // METHODOLOGY EMPLOYED:
    // Pointers (as pointers), pointers (as indices), and lots of other KEWL data stuff.

    using namespace OutputProcessor;

    auto &op = state.dataOutputProcessor;

    if (!op->OutputInitialized) {
        InitializeOutput(state);
    }

    std::vector<int> reqVarNums;

    // Determine whether to Report or not
    CheckReportVariable(state, name, key, reqVarNums);
    if (reqVarNums.empty()) {
        reqVarNums.push_back(-1);
    }

    // Is this redundant with CheckReportVariable?
    bool const ThisOneOnTheList = DataOutputs::FindItemInVariableList(state, key, name);

    bool OnMeter = (resource != Constant::eResource::Invalid) || (endUseCat != EndUseCat::Invalid) || (!EndUseSub.empty()) ||
                   (group != Group::Invalid) || (!zone.empty()) || (!spaceType.empty());

    if (OnMeter && store == StoreType::Average) {
        ShowSevereError(state, "Meters can only be \"Summed\" variables");
        ShowContinueError(state, fmt::format("..reference variable={}:{}", key, name));
        OnMeter = false;
    }

    int ddOutVarNum = AddDDOutVar(state, name, timeStep, store, VariableType::Real, units, customUnitName);
    auto *ddOutVar = op->ddOutVars[ddOutVarNum];

    ++op->NumOfRVariable_Setup;

    // If we add any output variables here at all, the first one will be at this index
    int firstAddedOutVarNum = (int)op->outVars.size();

    op->NumTotalRVariable += reqVarNums.size();

    if (!OnMeter && !ThisOneOnTheList) {
        return;
    }

    if (store == StoreType::Sum) {
        ++op->NumOfRVariable_Sum;
    }
    if (OnMeter) {
        ++op->NumOfRVariable_Meter;
    }

    for (int reqVarNum : reqVarNums) {

        ++op->NumOfRVariable;

        OutVarReal *var = new OutVarReal;
        op->outVars.push_back(var);

        // Link this keyed variable to the dictionary entry
        ddOutVar->keyOutVarNums.push_back(op->outVars.size() - 1);
        var->ddVarNum = ddOutVarNum;

        var->varType = VariableType::Real;
        var->timeStepType = timeStep;
        var->storeType = store;
        var->name = name;
        var->nameUC = Util::makeUPPER(var->name);
        var->key = key;
        var->keyUC = Util::makeUPPER(key);
        var->keyColonName = fmt::format("{}:{}", key, name);
        var->keyColonNameUC = Util::makeUPPER(var->keyColonName);
        var->units = units;
        if (units == Constant::Units::customEMS) {
            var->unitNameCustomEMS = customUnitName;
        }
        var->freq = freq;
        var->sched = nullptr;
        var->ReportID = ++op->ReportNumberCounter;
        var->Which = &ActualVariable;
        var->ZoneMult = ZoneMult;
        var->ZoneListMult = ZoneListMult;
        var->indexGroupKey = indexGroupKey;
        var->indexGroup = timeStepTypeNames[(int)var->timeStepType];

        // This is only done for the first variable in the list.  It
        // could be moved out of this loop entirely but then some
        // numberings in unit tests would not line up
        if (OnMeter) {
            AttachMeters(state, units, resource, endUseCat, EndUseSub, group, zone, spaceType, firstAddedOutVarNum);
            OnMeter = false;
        }

        // This is a dummy variable that is not being reported, it is only being used to feed a particular meter.
        if (reqVarNum == -1) {
            continue;
        }

        var->Report = true;

        // freq != ReportFreq::Hour
        if (freq == ReportFreq::Hour) {
            var->freq = op->reqVars[reqVarNum]->freq;
            var->sched = op->reqVars[reqVarNum]->sched;
        }

        var->writeReportDictionaryItem(state);
    }

} // SetupOutputVariable()

void SetupOutputVariable(EnergyPlusData &state,
                         std::string_view const name,                // String Name of variable
                         Constant::Units const units,                // Actual units corresponding to the actual variable
                         int &ActualVariable,                        // Actual Variable, used to set up pointer
                         OutputProcessor::TimeStepType timeStepType, // Zone, HeatBalance=1, HVAC, System, Plant=2
                         OutputProcessor::StoreType storeType,       // State, Average=1, NonState, Sum=2
                         std::string const &key,                     // Associated Key for this variable
                         [[maybe_unused]] int const indexGroupKey,   // Group identifier for SQL output
                         OutputProcessor::ReportFreq freq            // Internal use -- causes reporting at this freqency
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   December 1998
    //       MODIFIED       August 2008; Added SQL output capability
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets up the variable data structure that will be used
    // to track values of the output variables of EnergyPlus.

    // METHODOLOGY EMPLOYED:
    // Pointers (as pointers), pointers (as indices), and lots of other KEWL data stuff <-- LOL

    // Using/Aliasing
    using namespace OutputProcessor;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &op = state.dataOutputProcessor;

    if (!op->OutputInitialized) {
        InitializeOutput(state);
    }

    // Determine whether to Report or not
    std::vector<int> reqVarNums;
    CheckReportVariable(state, name, key, reqVarNums);
    if (reqVarNums.empty()) {
        reqVarNums.push_back(-1);
    }

    // DataOutputs::OutputVariablesForSimulation is case-insentitive
    int ddOutVarNum = AddDDOutVar(state, name, timeStepType, storeType, VariableType::Integer, units);
    auto *ddOutVar = op->ddOutVars[ddOutVarNum];

    ++op->NumOfIVariable_Setup;

    op->NumTotalIVariable += (!reqVarNums.empty()) ? reqVarNums.size() : 1;
    bool ThisOneOnTheList = DataOutputs::FindItemInVariableList(state, key, name);
    if (!ThisOneOnTheList) {
        return;
    }

    if (storeType == StoreType::Sum) {
        ++op->NumOfIVariable_Sum;
    }

    for (int reqVarNum : reqVarNums) {

        ++op->NumOfIVariable;

        OutVarInt *var = new OutVarInt;
        op->outVars.push_back(var);
        // Add to ddVar key list
        ddOutVar->keyOutVarNums.push_back(op->outVars.size() - 1);

        var->varType = VariableType::Integer;
        var->timeStepType = timeStepType;
        var->storeType = storeType;
        var->name = name;
        var->nameUC = Util::makeUPPER(var->name);
        var->key = key;
        var->keyUC = Util::makeUPPER(key);
        var->keyColonName = fmt::format("{}:{}", key, name);
        var->keyColonNameUC = Util::makeUPPER(var->keyColonName);
        var->units = units;
        var->ReportID = ++op->ReportNumberCounter;
        var->Which = &ActualVariable;
        var->indexGroupKey = -1;

        if (reqVarNum == -1) {
            continue;
        }

        var->Report = true;

        if (freq != ReportFreq::Hour) {
            var->freq = freq;
            var->sched = nullptr;
        } else {
            var->freq = op->reqVars[reqVarNum]->freq;
            var->sched = op->reqVars[reqVarNum]->sched;
        }

        var->writeReportDictionaryItem(state);
    }
} // SetOutputVariable()

void UpdateDataandReport(EnergyPlusData &state, OutputProcessor::TimeStepType const t_TimeStepTypeKey) // What kind of data to update (Zone, HVAC)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   December 1998
    //       MODIFIED       January 2001; Resolution integrated at the Zone TimeStep intervals
    //       MODIFIED       August 2008; Added SQL output capability
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine writes the actual report variable (for user requested
    // Report Variables) strings to the standard output file.

    // Using/Aliasing
    using namespace OutputProcessor;
    using General::EncodeMonDayHrMin;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool TimePrint(true);        // True if the time needs to be printed
    bool EndTimeStepFlag(false); // True when it's the end of the Zone Time Step
    auto &op = state.dataOutputProcessor;
    auto &rf = state.dataResultsFramework->resultsFramework;

    if (t_TimeStepTypeKey != TimeStepType::Zone && t_TimeStepTypeKey != TimeStepType::System) {
        ShowFatalError(state, "Invalid reporting requested -- UpdateDataAndReport");
    }

    // Basic record keeping and report out if "detailed"
    Real64 StartMinute = op->TimeValue[(int)t_TimeStepTypeKey].CurMinute; // StartMinute for UpdateData call
    op->TimeValue[(int)t_TimeStepTypeKey].CurMinute += (*op->TimeValue[(int)t_TimeStepTypeKey].TimeStep) * 60.0;
    if (t_TimeStepTypeKey == TimeStepType::System &&
        (op->TimeValue[(int)TimeStepType::System].CurMinute == op->TimeValue[(int)TimeStepType::Zone].CurMinute)) {
        EndTimeStepFlag = true;
    } else if (t_TimeStepTypeKey == TimeStepType::Zone) {
        EndTimeStepFlag = true;
    } else {
        EndTimeStepFlag = false;
    }
    Real64 MinuteNow = op->TimeValue[(int)t_TimeStepTypeKey].CurMinute; // What minute it is now

    int MDHM; // Month,Day,Hour,Minute
    EncodeMonDayHrMin(MDHM, state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth, state.dataGlobal->HourOfDay, int(MinuteNow));
    TimePrint = true;

    Real64 rxTime = (MinuteNow - StartMinute) /
                    double(state.dataGlobal->MinutesInTimeStep); // (MinuteNow-StartMinute)/REAL(MinutesPerTimeStep,r64) - for execution time

    if (rf->timeSeriesEnabled()) {
        // R and I data frames for TimeStepType::TimeStepZone
        if (!rf->detailedTSData[(int)t_TimeStepTypeKey].variablesScanned()) {
            rf->initializeTSDataFrame(ReportFreq::EachCall, op->outVars, t_TimeStepTypeKey);
        }
    }

    if (rf->timeSeriesEnabled()) {
        rf->detailedTSData[(int)t_TimeStepTypeKey].newRow(state.dataEnvrn->Month,
                                                          state.dataEnvrn->DayOfMonth,
                                                          state.dataGlobal->HourOfDay,
                                                          op->TimeValue[(int)t_TimeStepTypeKey].CurMinute,
                                                          state.dataGlobal->CalendarYear);
    }

    // Main "Record Keeping" Loops for R and I variables
    for (auto *var : op->outVars) {
        if (var->timeStepType != t_TimeStepTypeKey) {
            continue;
        }

        Real64 value = (var->varType == VariableType::Real) ? *(dynamic_cast<OutVarReal *>(var))->Which : *(dynamic_cast<OutVarInt *>(var))->Which;

        var->Stored = true;

        if (var->storeType == StoreType::Average) {
            Real64 CurVal = value * rxTime;
            // TODO: Is this correct? Integer logic is different
            if (var->varType == VariableType::Real) {
                if (value > var->MaxValue) {
                    var->MaxValue = value;
                    var->maxValueDate = MDHM;
                }
                if (value < var->MinValue) {
                    var->MinValue = value;
                    var->minValueDate = MDHM;
                }
            } else { // var->varType == VariableType::Integer
                if (CurVal > var->MaxValue) {
                    var->MaxValue = CurVal;
                    var->maxValueDate = MDHM;
                }
                if (CurVal < var->MinValue) {
                    var->MinValue = CurVal;
                    var->minValueDate = MDHM;
                }
            }
            var->TSValue += CurVal;
            var->EITSValue = var->TSValue; // CR - 8481 fix - 09/06/2011
        } else {
            if (value > var->MaxValue) {
                var->MaxValue = value;
                var->maxValueDate = MDHM;
            }
            if (value < var->MinValue) {
                var->MinValue = value;
                var->minValueDate = MDHM;
            }
            var->TSValue += value;
            var->EITSValue = var->TSValue; // CR - 8481 fix - 09/06/2011
        }

        // End of "record keeping"  Report if applicable
        if (!var->Report) {
            continue;
        }

        if (var->sched != nullptr && var->sched->getCurrentVal() == 0.0) {
            continue;
        }

        var->tsStored = true;
        if (!var->thisTSStored) {
            ++var->thisTSCount;
            var->thisTSStored = true;
        }

        if (var->freq != ReportFreq::EachCall) {
            continue;
        }

        if (TimePrint) {
            if (op->LHourP != state.dataGlobal->HourOfDay || std::abs(op->LStartMin - StartMinute) > 0.001 ||
                std::abs(op->LEndMin - op->TimeValue[(int)t_TimeStepTypeKey].CurMinute) > 0.001) {
                int CurDayType = state.dataEnvrn->DayOfWeek;
                if (state.dataEnvrn->HolidayIndex > 0) {
                    CurDayType = state.dataEnvrn->HolidayIndex;
                }
                WriteTimeStampFormatData(state,
                                         state.files.eso,
                                         ReportFreq::EachCall,
                                         op->freqStampReportNums[(int)ReportFreq::TimeStep],
                                         state.dataGlobal->DayOfSimChr,
                                         true,
                                         state.dataEnvrn->Month,
                                         state.dataEnvrn->DayOfMonth,
                                         state.dataGlobal->HourOfDay,
                                         op->TimeValue[(int)t_TimeStepTypeKey].CurMinute,
                                         StartMinute,
                                         state.dataEnvrn->DSTIndicator,
                                         Sched::dayTypeNames[CurDayType]);
                op->LHourP = state.dataGlobal->HourOfDay;
                op->LStartMin = StartMinute;
                op->LEndMin = op->TimeValue[(int)t_TimeStepTypeKey].CurMinute;
            }
            TimePrint = false;
        }

        WriteNumericData(state, var->ReportID, value);
        ++state.dataGlobal->StdOutputRecordCount;

        if (rf->timeSeriesEnabled()) {
            rf->detailedTSData[(int)t_TimeStepTypeKey].pushVariableValue(var->ReportID, value);
        }
    } // for (var)

    if (t_TimeStepTypeKey == TimeStepType::System) {
        return; // All other stuff happens at the "zone" time step call to this routine.
    }

    // TimeStep Block (Report on Zone TimeStep)

    if (EndTimeStepFlag) {
        if (rf->timeSeriesEnabled()) {
            if (!rf->freqTSData[(int)ReportFreq::TimeStep].variablesScanned()) {
                rf->initializeTSDataFrame(ReportFreq::TimeStep, op->outVars);
            }
            rf->freqTSData[(int)ReportFreq::TimeStep].newRow(state.dataEnvrn->Month,
                                                             state.dataEnvrn->DayOfMonth,
                                                             state.dataGlobal->HourOfDay,
                                                             op->TimeValue[(int)TimeStepType::Zone].CurMinute,
                                                             state.dataGlobal->CalendarYear);
        }

        // Update meters on the TimeStep  (Zone)
        if (op->meterValues.capacity() > 0) {
            for (int iMeter = 0; iMeter < (int)op->meters.size(); ++iMeter) {
                auto *meter = op->meters[iMeter];
                if (meter->type == MeterType::Normal || meter->type == MeterType::Custom) {
                    for (int srcVarNum : meter->srcVarNums) {
                        auto *var = op->outVars[srcVarNum];
                        // Separate the Zone variables from the HVAC variables using TimeStepType
                        if (var->timeStepType != TimeStepType::Zone && var->timeStepType != TimeStepType::System) {
                            continue;
                        }
                        // Add to the total all of the appropriate variables, make sure to use var->TSValue and not *var->Which
                        op->meterValues[iMeter] += var->TSValue * var->ZoneMult * var->ZoneListMult;
                    }
                } else if (meter->type == MeterType::CustomDec) {
                    auto *decMeter = op->meters[meter->decMeterNum];
                    for (int srcVarNum : decMeter->srcVarNums) {
                        auto *var = op->outVars[srcVarNum];
                        if (var->timeStepType != TimeStepType::Zone && var->timeStepType != TimeStepType::System) {
                            continue;
                        }
                        op->meterValues[iMeter] += var->TSValue * var->ZoneMult * var->ZoneListMult;
                    }
                    for (int srcVarNum : meter->srcVarNums) {
                        auto *var = op->outVars[srcVarNum];
                        if (var->timeStepType != TimeStepType::Zone && var->timeStepType != TimeStepType::System) {
                            continue;
                        }
                        op->meterValues[iMeter] -= var->TSValue * var->ZoneMult * var->ZoneListMult;
                    }
                } else {
                    assert(false);
                }
            } // for (iMeter)
        } // if (op->meterValues.capacity() > 0)

        for (auto *var : op->outVars) {
            if (var->timeStepType != TimeStepType::Zone && var->timeStepType != TimeStepType::System) {
                continue;
            }

            bool ReportNow = true;
            if (var->sched != nullptr) {
                ReportNow = (var->sched->getCurrentVal() != 0.0); // SetReportNow(RVar%SchedPtr)
            }
            if (!ReportNow || !var->Report) {
                var->TSValue = 0.0;
            }
            //        IF (RVar%StoreType == AveragedVar) THEN
            //          RVar%Value=RVar%Value+RVar%TSValue/NumOfTimeStepInHour
            //        ELSE
            var->Value += var->TSValue;
            //        ENDIF

            if (!ReportNow || !var->Report) {
                continue;
            }

            if (var->freq == ReportFreq::TimeStep) {
                if (TimePrint) {
                    if (op->LHourP != state.dataGlobal->HourOfDay || std::abs(op->LStartMin - StartMinute) > 0.001 ||
                        std::abs(op->LEndMin - op->TimeValue[(int)var->timeStepType].CurMinute) > 0.001) {
                        int CurDayType = state.dataEnvrn->DayOfWeek;
                        if (state.dataEnvrn->HolidayIndex > 0) {
                            CurDayType = state.dataEnvrn->HolidayIndex;
                        }
                        WriteTimeStampFormatData(state,
                                                 state.files.eso,
                                                 ReportFreq::EachCall,
                                                 op->freqStampReportNums[(int)ReportFreq::TimeStep],
                                                 state.dataGlobal->DayOfSimChr,
                                                 true,
                                                 state.dataEnvrn->Month,
                                                 state.dataEnvrn->DayOfMonth,
                                                 state.dataGlobal->HourOfDay,
                                                 op->TimeValue[(int)var->timeStepType].CurMinute,
                                                 StartMinute,
                                                 state.dataEnvrn->DSTIndicator,
                                                 Sched::dayTypeNames[CurDayType]);
                        op->LHourP = state.dataGlobal->HourOfDay;
                        op->LStartMin = StartMinute;
                        op->LEndMin = op->TimeValue[(int)var->timeStepType].CurMinute;
                    }
                    TimePrint = false;
                } // if (TimePrint)

                WriteNumericData(state, var->ReportID, var->TSValue);
                ++state.dataGlobal->StdOutputRecordCount;

                if (rf->timeSeriesEnabled()) {
                    rf->freqTSData[(int)ReportFreq::TimeStep].pushVariableValue(var->ReportID, var->TSValue);
                }
            }
            var->TSValue = 0.0;
            var->thisTSStored = false;
        } // for (var)

        UpdateMeters(state, MDHM);

        ReportTSMeters(state, StartMinute, op->TimeValue[(int)TimeStepType::Zone].CurMinute, TimePrint, TimePrint);

    } // TimeStep Block

    // Hour Block
    if (state.dataGlobal->EndHourFlag) {
        if (op->freqTrackingVariables[(int)ReportFreq::Hour]) {
            int CurDayType = state.dataEnvrn->DayOfWeek;
            if (state.dataEnvrn->HolidayIndex > 0) {
                CurDayType = state.dataEnvrn->HolidayIndex;
            }
            WriteTimeStampFormatData(state,
                                     state.files.eso,
                                     ReportFreq::Hour,
                                     op->freqStampReportNums[(int)ReportFreq::TimeStep],
                                     state.dataGlobal->DayOfSimChr,
                                     true,
                                     state.dataEnvrn->Month,
                                     state.dataEnvrn->DayOfMonth,
                                     state.dataGlobal->HourOfDay,
                                     -1, // EndMinute
                                     -1, // startMinute
                                     state.dataEnvrn->DSTIndicator,
                                     Sched::dayTypeNames[CurDayType]);
            TimePrint = false;
        }

        if (rf->timeSeriesEnabled()) {
            if (!rf->freqTSData[(int)ReportFreq::Hour].variablesScanned()) {
                rf->initializeTSDataFrame(ReportFreq::Hour, op->outVars);
            }
            rf->freqTSData[(int)ReportFreq::Hour].newRow(
                state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth, state.dataGlobal->HourOfDay, 0, state.dataGlobal->CalendarYear);
        }

        op->TimeValue[(int)TimeStepType::Zone].CurMinute = 0.0;
        op->TimeValue[(int)TimeStepType::System].CurMinute = 0.0;

        for (auto *var : op->outVars) {

            if (var->timeStepType != TimeStepType::Zone && var->timeStepType != TimeStepType::System) {
                continue;
            }

            //        ReportNow=.TRUE.
            //        IF (RVar%SchedPtr > 0) &
            //          ReportNow=(GetCurrentScheduleValue(state, RVar%SchedPtr) /= 0.0)  !SetReportNow(RVar%SchedPtr)

            //        IF (ReportNow) THEN
            if (var->tsStored) {
                if (var->storeType == StoreType::Average) {
                    var->Value /= double(var->thisTSCount);
                }
                if (var->Report && var->freq == ReportFreq::Hour && var->Stored) {
                    WriteNumericData(state, var->ReportID, var->Value);
                    ++state.dataGlobal->StdOutputRecordCount;
                    var->Stored = false;
                    // add time series value for hourly to data store
                    if (rf->timeSeriesEnabled()) {
                        rf->freqTSData[(int)ReportFreq::Hour].pushVariableValue(var->ReportID, var->Value);
                    }
                }
                var->StoreValue += var->Value;
                ++var->NumStored;
            }
            var->tsStored = false;
            var->thisTSStored = false;
            var->thisTSCount = 0;
            var->Value = 0.0;
        } // for (var)

        ReportMeters(state, ReportFreq::Hour, TimePrint);
    } // Hour Block

    if (!state.dataGlobal->EndHourFlag) {
        return;
    }

    // Day Block
    if (state.dataGlobal->EndDayFlag) {
        if (op->freqTrackingVariables[(int)ReportFreq::Day]) {
            int CurDayType = state.dataEnvrn->DayOfWeek;
            if (state.dataEnvrn->HolidayIndex > 0) {
                CurDayType = state.dataEnvrn->HolidayIndex;
            }
            WriteTimeStampFormatData(state,
                                     state.files.eso,
                                     ReportFreq::Day,
                                     op->freqStampReportNums[(int)ReportFreq::Day],
                                     state.dataGlobal->DayOfSimChr,
                                     true,
                                     state.dataEnvrn->Month,
                                     state.dataEnvrn->DayOfMonth,
                                     -1, // Hour
                                     -1, // EndMinute
                                     -1, // StartMinute
                                     state.dataEnvrn->DSTIndicator,
                                     Sched::dayTypeNames[CurDayType]);
            TimePrint = false;
        }
        if (rf->timeSeriesEnabled()) {
            if (!rf->freqTSData[(int)ReportFreq::Day].variablesScanned()) {
                rf->initializeTSDataFrame(ReportFreq::Day, op->outVars);
            }
            rf->freqTSData[(int)ReportFreq::Day].newRow(
                state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth, state.dataGlobal->HourOfDay, 0, state.dataGlobal->CalendarYear);
        }

        op->NumHoursInMonth += 24;
        for (auto *var : op->outVars) {
            if (var->timeStepType != TimeStepType::Zone && var->timeStepType != TimeStepType::System) {
                continue;
            }
            var->writeOutput(state, ReportFreq::Day);
        }

        ReportMeters(state, ReportFreq::Day, TimePrint);

    } // Day Block

    // Only continue if EndDayFlag is set
    if (!state.dataGlobal->EndDayFlag) {
        return;
    }

    // Month Block
    if (state.dataEnvrn->EndMonthFlag || state.dataGlobal->EndEnvrnFlag) {
        if (op->freqTrackingVariables[(int)ReportFreq::Month]) {
            WriteTimeStampFormatData(state,
                                     state.files.eso,
                                     ReportFreq::Month,
                                     op->freqStampReportNums[(int)ReportFreq::Month],
                                     state.dataGlobal->DayOfSimChr,
                                     true,
                                     state.dataEnvrn->Month);
            TimePrint = false;
        }

        if (rf->timeSeriesEnabled()) {
            if (!rf->freqTSData[(int)ReportFreq::Month].variablesScanned()) {
                rf->initializeTSDataFrame(ReportFreq::Month, op->outVars);
            }
            rf->freqTSData[(int)ReportFreq::Month].newRow(
                state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth, state.dataGlobal->HourOfDay, 0, state.dataGlobal->CalendarYear);
        }

        op->NumHoursInSim += op->NumHoursInMonth;
        state.dataEnvrn->EndMonthFlag = false;
        for (auto *var : op->outVars) {
            if (var->timeStepType != TimeStepType::Zone && var->timeStepType != TimeStepType::System) {
                continue;
            }
            var->writeOutput(state, ReportFreq::Month);
        }

        ReportMeters(state, ReportFreq::Month, TimePrint);

        op->NumHoursInMonth = 0;
    } // Month Block

    // Sim/Environment Block
    if (state.dataGlobal->EndEnvrnFlag) {
        if (op->freqTrackingVariables[(int)ReportFreq::Simulation]) {
            WriteTimeStampFormatData(state,
                                     state.files.eso,
                                     ReportFreq::Simulation,
                                     op->freqStampReportNums[(int)ReportFreq::Simulation],
                                     state.dataGlobal->DayOfSimChr,
                                     true);
            TimePrint = false;
        }

        if (rf->timeSeriesEnabled()) {
            if (!rf->freqTSData[(int)ReportFreq::Simulation].variablesScanned()) {
                rf->initializeTSDataFrame(ReportFreq::Simulation, op->outVars);
            }
            rf->freqTSData[(int)ReportFreq::Simulation].newRow(
                state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth, state.dataGlobal->HourOfDay, 0, state.dataGlobal->CalendarYear);
        }

        for (auto *var : op->outVars) {
            if (var->timeStepType != TimeStepType::Zone && var->timeStepType != TimeStepType::System) {
                continue;
            }
            var->writeOutput(state, ReportFreq::Simulation);
        }

        ReportMeters(state, ReportFreq::Simulation, TimePrint);

        op->NumHoursInSim = 0;
    }

    // Yearly Block
    if (state.dataEnvrn->EndYearFlag) {
        if (op->freqTrackingVariables[(int)ReportFreq::Year]) {
            WriteYearlyTimeStamp(state, state.files.eso, op->freqStampReportNums[(int)ReportFreq::Year], state.dataGlobal->CalendarYearChr, true);
            TimePrint = false;
        }
        if (rf->timeSeriesEnabled()) {
            if (!rf->freqTSData[(int)ReportFreq::Year].variablesScanned()) {
                rf->initializeTSDataFrame(ReportFreq::Year, op->outVars);
            }
            rf->freqTSData[(int)ReportFreq::Year].newRow(
                state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth, state.dataGlobal->HourOfDay, 0, state.dataGlobal->CalendarYear);
        }

        for (auto *var : op->outVars) {
            if (var->timeStepType != TimeStepType::Zone && var->timeStepType != TimeStepType::System) {
                continue;
            }
            var->writeOutput(state, ReportFreq::Year);
        }

        ReportMeters(state, ReportFreq::Year, TimePrint);

        state.dataGlobal->CalendarYear += 1;
        state.dataGlobal->CalendarYearChr = fmt::to_string(state.dataGlobal->CalendarYear);
    }
} // UpdateDataandReport()

void GenOutputVariablesAuditReport(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   February 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine reports (to the .err file) any report variables
    // which were requested but not "setup" during the run.  These will
    // either be items that were not used in the IDF file or misspellings
    // of report variable names.

    // METHODOLOGY EMPLOYED:
    // Use flagged data structure in OutputProcessor.

    // Using/Aliasing
    using namespace OutputProcessor;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &op = state.dataOutputProcessor;

    static constexpr std::array<std::string_view, (int)ReportFreq::Num> localReportFreqNames = {
        "Detailed",  // EachCall  // For some reason, this is "Detailed" here and "Each Call" in other places
        "TimeStep",  // TimeStep
        "Hourly",    // Hourly
        "Daily",     // Daily
        "Monthly",   // Monthly
        "RunPeriod", // Simulation
        "Annual"     // Yearly
    };

    for (auto *reqVar : op->reqVars) {
        if (reqVar->Used) {
            continue;
        }
        if (reqVar->key.empty()) {
            reqVar->key = "*";
        }
        if (has(reqVar->name, "OPAQUE SURFACE INSIDE FACE CONDUCTION") && !state.dataGlobal->DisplayAdvancedReportVariables &&
            !state.dataOutputProcessor->OpaqSurfWarned) {
            ShowWarningMessage(state, R"(Variables containing "Opaque Surface Inside Face Conduction" are now "advanced" variables.)");
            ShowContinueError(state, "You must enter the \"Output:Diagnostics,DisplayAdvancedReportVariables;\" statement to view.");
            ShowContinueError(state, "First, though, read cautionary statements in the \"InputOutputReference\" document.");
            state.dataOutputProcessor->OpaqSurfWarned = true;
        }
        if (!state.dataOutputProcessor->Rept) {
            ShowWarningMessage(state, "The following Report Variables were requested but not generated -- check.rdd file");
            ShowContinueError(state, "Either the IDF did not contain these elements, the variable name is misspelled,");
            ShowContinueError(state,
                              "or the requested variable is an advanced output which requires Output : Diagnostics, DisplayAdvancedReportVariables;");
            state.dataOutputProcessor->Rept = true;
        }
        ShowMessage(state, format("Key={}, VarName={}, Frequency={}", reqVar->key, reqVar->name, localReportFreqNames[(int)reqVar->freq]));
    }
} // GenOutputVariablesAuditReport()

void UpdateMeterReporting(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   January 2001
    //       MODIFIED       February 2007 -- add cumulative meter reporting
    //                      January 2012 -- add predefined tabular meter reporting
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is called at the end of the first HVAC iteration and
    // sets up the reporting for the Energy Meters.  It also may show a fatal error
    // if errors occurred during initial SetupOutputVariable processing.  It "gets"
    // the Report Meter input:
    // Report Meter,
    //        \memo Meters requested here show up on eplusout.eso and eplusout.mtr
    //   A1 , \field Meter_Name
    //        \required-field
    //        \note Form is EnergyUseType:..., e.g. Electricity:* for all Electricity meters
    //        \note or EndUse:..., e.g. InteriorLights:* for all interior lights
    //        \note Report MeterFileOnly puts results on the eplusout.mtr file only
    //   A2 ; \field Reporting_Frequency
    //        \type choice
    //        \key timestep
    //        \note timestep refers to the zone timestep/timestep in hour value
    //        \note runperiod, environment, and annual are the same
    //        \key hourly
    //        \key daily
    //        \key monthly
    //        \key runperiod
    //        \key environment
    //        \key annual
    //        \note runperiod, environment, and annual are synonymous
    // Report MeterFileOnly,
    //        \memo same reporting as Report Meter -- goes to eplusout.mtr only
    //   A1 , \field Meter_Name
    //        \required-field
    //        \note Form is EnergyUseType:..., e.g. Electricity:* for all Electricity meters
    //        \note or EndUse:..., e.g. InteriorLights:* for all interior lights
    //        \note Report MeterFileOnly puts results on the eplusout.mtr file only
    //   A2 ; \field Reporting_Frequency
    //        \type choice
    //        \key timestep
    //        \note timestep refers to the zone timestep/timestep in hour value
    //        \note runperiod, environment, and annual are the same
    //        \key hourly
    //        \key daily
    //        \key monthly
    //        \key runperiod
    //        \key environment
    //        \key annual
    //        \note runperiod, environment, and annual are synonymous

    // Using/Aliasing
    using namespace OutputProcessor;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int Loop;
    Array1D_string Alphas(2);
    Array1D<Real64> Numbers(1);
    int NumAlpha;
    int NumNumbers;
    int IOStat;
    int NumReqMeters;
    int NumReqMeterFOs;

    bool ErrorsFound(false); // If errors detected in input
    auto const &op = state.dataOutputProcessor;
    auto &ipsc = state.dataIPShortCut;

    GetCustomMeterInput(state, ErrorsFound);
    if (ErrorsFound) {
        op->ErrorsLogged = true;
    }

    // Helper lambda to locate a meter index from its name. Returns a negative value if not found
    auto setupMeterFromMeterName = // (AUTO_OK_LAMBDA)
        [&state](std::string &name, ReportFreq freq, bool MeterFileOnlyIndicator, bool CumulativeIndicator) -> bool {
        bool result = false;

        size_t varnameLen = index(name, '[');
        if (varnameLen != std::string::npos) {
            name.erase(varnameLen);
        }

        auto const &op = state.dataOutputProcessor;

        std::string::size_type wildCardPosition = index(name, '*');

        if (wildCardPosition == std::string::npos) {
            if (auto found = op->meterMap.find(name); found != op->meterMap.end()) {
                SetInitialMeterReportingAndOutputNames(state, found->second, MeterFileOnlyIndicator, freq, CumulativeIndicator);
                result = true;
            }
        } else { // Wildcard input
            std::string nameSubstr = name.substr(0, wildCardPosition);
            for (int iMeter = 0; iMeter < (int)op->meters.size(); ++iMeter) {
                if (Util::SameString(op->meters[iMeter]->Name.substr(0, wildCardPosition), nameSubstr)) {
                    SetInitialMeterReportingAndOutputNames(state, iMeter, MeterFileOnlyIndicator, freq, CumulativeIndicator);
                    result = true;
                }
            }
        }

        return result;
    };

    ipsc->cCurrentModuleObject = "Output:Meter";
    NumReqMeters = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ipsc->cCurrentModuleObject);

    for (Loop = 1; Loop <= NumReqMeters; ++Loop) {

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 ipsc->cCurrentModuleObject,
                                                                 Loop,
                                                                 Alphas,
                                                                 NumAlpha,
                                                                 Numbers,
                                                                 NumNumbers,
                                                                 IOStat,
                                                                 ipsc->lNumericFieldBlanks,
                                                                 ipsc->lAlphaFieldBlanks,
                                                                 ipsc->cAlphaFieldNames,
                                                                 ipsc->cNumericFieldNames);

        bool meterFileOnlyIndicator = false;
        bool cumulativeIndicator = false;
        ReportFreq freq = determineFrequency(state, Util::makeUPPER(Alphas(2)));

        if (!setupMeterFromMeterName(Alphas(1), freq, meterFileOnlyIndicator, cumulativeIndicator)) {
            ShowWarningError(state, format("{}: invalid {}=\"{}\" - not found.", ipsc->cCurrentModuleObject, ipsc->cAlphaFieldNames(1), Alphas(1)));
        }
    }

    ipsc->cCurrentModuleObject = "Output:Meter:MeterFileOnly";
    NumReqMeterFOs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ipsc->cCurrentModuleObject);
    for (Loop = 1; Loop <= NumReqMeterFOs; ++Loop) {

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 ipsc->cCurrentModuleObject,
                                                                 Loop,
                                                                 Alphas,
                                                                 NumAlpha,
                                                                 Numbers,
                                                                 NumNumbers,
                                                                 IOStat,
                                                                 ipsc->lNumericFieldBlanks,
                                                                 ipsc->lAlphaFieldBlanks,
                                                                 ipsc->cAlphaFieldNames,
                                                                 ipsc->cNumericFieldNames);

        bool meterFileOnlyIndicator = true;
        bool cumulativeIndicator = false;
        ReportFreq freq = determineFrequency(state, Util::makeUPPER(Alphas(2)));
        if (!setupMeterFromMeterName(Alphas(1), freq, meterFileOnlyIndicator, cumulativeIndicator)) {
            ShowWarningError(state, format("{}: invalid {}=\"{}\" - not found.", ipsc->cCurrentModuleObject, ipsc->cAlphaFieldNames(1), Alphas(1)));
        }
    }

    ipsc->cCurrentModuleObject = "Output:Meter:Cumulative";
    NumReqMeters = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ipsc->cCurrentModuleObject);

    for (Loop = 1; Loop <= NumReqMeters; ++Loop) {

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 ipsc->cCurrentModuleObject,
                                                                 Loop,
                                                                 Alphas,
                                                                 NumAlpha,
                                                                 Numbers,
                                                                 NumNumbers,
                                                                 IOStat,
                                                                 ipsc->lNumericFieldBlanks,
                                                                 ipsc->lAlphaFieldBlanks,
                                                                 ipsc->cAlphaFieldNames,
                                                                 ipsc->cNumericFieldNames);

        bool meterFileOnlyIndicator = false;
        bool cumulativeIndicator = true;
        ReportFreq freq = determineFrequency(state, Util::makeUPPER(Alphas(2)));
        if (!setupMeterFromMeterName(Alphas(1), freq, meterFileOnlyIndicator, cumulativeIndicator)) {
            ShowWarningError(state, format("{}: invalid {}=\"{}\" - not found.", ipsc->cCurrentModuleObject, ipsc->cAlphaFieldNames(1), Alphas(1)));
        }
    }

    ipsc->cCurrentModuleObject = "Output:Meter:Cumulative:MeterFileOnly";
    NumReqMeterFOs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ipsc->cCurrentModuleObject);
    for (Loop = 1; Loop <= NumReqMeterFOs; ++Loop) {

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 ipsc->cCurrentModuleObject,
                                                                 Loop,
                                                                 Alphas,
                                                                 NumAlpha,
                                                                 Numbers,
                                                                 NumNumbers,
                                                                 IOStat,
                                                                 ipsc->lNumericFieldBlanks,
                                                                 ipsc->lAlphaFieldBlanks,
                                                                 ipsc->cAlphaFieldNames,
                                                                 ipsc->cNumericFieldNames);

        bool meterFileOnlyIndicator = true;
        bool cumulativeIndicator = true;
        ReportFreq freq = determineFrequency(state, Util::makeUPPER(Alphas(2)));
        if (!setupMeterFromMeterName(Alphas(1), freq, meterFileOnlyIndicator, cumulativeIndicator)) {
            ShowWarningError(state, format("{}: invalid {}=\"{}\" - not found.", ipsc->cCurrentModuleObject, ipsc->cAlphaFieldNames(1), Alphas(1)));
        }
    }

    ReportMeterDetails(state);

    if (op->ErrorsLogged) {
        ShowFatalError(state, "UpdateMeterReporting: Previous Meter Specification errors cause program termination.");
    }

    op->meterValues.resize(op->meters.size(), 0.0);
    std::fill(op->meterValues.begin(), op->meterValues.end(), 0.0);
} // UpdateMeterReporting()

void SetInitialMeterReportingAndOutputNames(EnergyPlusData &state,
                                            int const WhichMeter,              // Which meter number
                                            bool const MeterFileOnlyIndicator, // true if this is a meter file only reporting
                                            OutputProcessor::ReportFreq freq,  // at what frequency is the meter reported
                                            bool const CumulativeIndicator     // true if this is a Cumulative meter reporting
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   February 2007

    // PURPOSE OF THIS SUBROUTINE:
    // Set values and output initial names to output files.

    // Using/Aliasing
    using namespace OutputProcessor;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &op = state.dataOutputProcessor;

    auto *meter = op->meters[WhichMeter];
    auto &period = meter->periods[(freq == ReportFreq::EachCall) ? (int)ReportFreq::TimeStep : (int)freq];
    if (!CumulativeIndicator) {
        if (MeterFileOnlyIndicator && period.Rpt) {
            ShowWarningError(state,
                             format(R"(Output:Meter:MeterFileOnly requested for "{}" ({}), already on "Output:Meter". Will report to both {} and {})",
                                    meter->Name,
                                    reportFreqNames[(freq == ReportFreq::EachCall) ? (int)ReportFreq::TimeStep : (int)freq],
                                    state.files.eso.filePath.filename(),
                                    state.files.mtr.filePath.filename()));
        }
        if (!period.Rpt) {
            period.Rpt = true;
            if (MeterFileOnlyIndicator) {
                period.RptFO = true;
            } else {
                op->freqTrackingVariables[(int)freq] = true;
            }
            // int indexGroupKey = DetermineIndexGroupKeyFromMeterName(state, meter->Name);
            meter->indexGroup = DetermineIndexGroupFromMeterGroup(meter);
            WriteMeterDictionaryItem(
                state, freq, StoreType::Sum, period.RptNum, meter->indexGroup, meter->Name, meter->units, false, MeterFileOnlyIndicator);
        }
    } else { // !CumulativeIndicator
        if (MeterFileOnlyIndicator && period.accRpt) {
            ShowWarningError(state,
                             format("Output:Meter:MeterFileOnly requested for \"Cumulative {}\" (TimeStep), already on \"Output:Meter\". "
                                    "Will report to both {} and {}",
                                    meter->Name,
                                    state.files.eso.filePath.filename(),
                                    state.files.mtr.filePath.filename()));
        }

        if (!period.accRpt) {
            period.accRpt = true;
            if (MeterFileOnlyIndicator) {
                period.accRptFO = true;
            }
            // int indexGroupKey = DetermineIndexGroupKeyFromMeterName(state, meter->Name);
            meter->indexGroup = DetermineIndexGroupFromMeterGroup(op->meters[WhichMeter]);
            WriteMeterDictionaryItem(
                state, freq, StoreType::Sum, period.accRptNum, meter->indexGroup, meter->Name, meter->units, true, MeterFileOnlyIndicator);
        }
    } // if (CumulativeIndicator)
} // SetInitialMeterReportingAndOutputNames()

int GetMeterIndex(EnergyPlusData const &state, std::string const &name)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   August 2002

    // PURPOSE OF THIS FUNCTION:
    // This function returns a index to the meter "number" (aka assigned report number)
    // for the meter name.  If none active for this run, a zero is returned.  This is used later to
    // obtain a meter "value".

    auto const &op = state.dataOutputProcessor;

    auto found = op->meterMap.find(name);
    return (found != op->meterMap.end()) ? found->second : -1;
} // GetMeterIndex()

Constant::eResource GetMeterResourceType(EnergyPlusData const &state, int const MeterNumber) // Which Meter Number (from GetMeterIndex)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   August 2002

    // Using/Aliasing
    using namespace OutputProcessor;

    return (MeterNumber != -1) ? state.dataOutputProcessor->meters[MeterNumber]->resource : Constant::eResource::Invalid;
} // GetMeterResourceType()

Real64 GetCurrentMeterValue(EnergyPlusData const &state, int const MeterNumber) // Which Meter Number (from GetMeterIndex)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   August 2002

    // PURPOSE OF THIS FUNCTION:
    // This function returns the current meter value (timestep) for the meter number indicated.

    return (MeterNumber != -1) ? state.dataOutputProcessor->meters[MeterNumber]->CurTSValue : 0.0;
} // GetCurrentMeterValue()

Real64 GetInstantMeterValue(EnergyPlusData &state,
                            int const meterNum,                              // Which Meter Number (from GetMeterIndex)
                            OutputProcessor::TimeStepType const timeStepType // Whether this is zone of HVAC
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   February 2003

    // PURPOSE OF THIS FUNCTION:
    // This function returns the Instantaneous meter value (timestep) for the meter number indicated
    //  using TimeStepType to differentiate between Zone and HVAC values.

    // Using/Aliasing
    using namespace OutputProcessor;

    Real64 InstantMeterValue = 0.0;

    if (meterNum == -1) {
        return InstantMeterValue;
    }

    auto const &op = state.dataOutputProcessor;
    auto *meter = op->meters[meterNum];

    if (meter->type == MeterType::Normal || meter->type == MeterType::Custom) {
        for (int srcVarNum : meter->srcVarNums) {
            auto *var = op->outVars[srcVarNum];
            // Separate the Zone variables from the HVAC variables using TimeStepType
            if (var->timeStepType != timeStepType) {
                continue;
            }

            auto *rVar = dynamic_cast<OutVarReal *>(var);
            assert(rVar != nullptr);
            // Add to the total all of the appropriate variables
            InstantMeterValue += (*rVar->Which) * rVar->ZoneMult * rVar->ZoneListMult;
        }

    } else if (meter->type == MeterType::CustomDec) {
        auto *decMeter = op->meters[meter->decMeterNum];
        for (int srcVarNum : decMeter->srcVarNums) {
            auto *var = op->outVars[srcVarNum];
            if (var->timeStepType != timeStepType) {
                continue;
            }
            auto *rVar = dynamic_cast<OutVarReal *>(var);
            assert(rVar != nullptr);
            InstantMeterValue += (*rVar->Which) * rVar->ZoneMult * rVar->ZoneListMult;
        }
        for (int srcVarNum : meter->srcVarNums) {
            auto *var = op->outVars[srcVarNum];
            if (var->timeStepType != timeStepType) {
                continue;
            }
            auto *rVar = dynamic_cast<OutVarReal *>(var);
            assert(rVar != nullptr);
            InstantMeterValue -= (*rVar->Which) * rVar->ZoneMult * rVar->ZoneListMult;
        }
    } else {
        assert(false);
    }

    return InstantMeterValue;
} // GetInstantMeterValue()

Real64 GetInternalVariableValue(EnergyPlusData &state,
                                OutputProcessor::VariableType const varType, // 1=integer, 2=real, 3=meter
                                int const keyVarIndex                        // Array index
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   December 2000
    //       MODIFIED       August 2003, M. J. Witte

    // PURPOSE OF THIS FUNCTION:
    // This function returns the current value of the Internal Variable assigned to
    // the varType and keyVarIndex.  Values may be accessed for REAL(r64) and integer
    // report variables and meter variables.  The variable type (varType) may be
    // determined by calling subroutine and GetVariableKeyCountandType.  The
    // index (keyVarIndex) may be determined by calling subroutine GetVariableKeys.

    // METHODOLOGY EMPLOYED:
    // Uses Internal OutputProcessor data structure to return value.

    // Using/Aliasing
    using namespace OutputProcessor;

    // Return value
    Real64 resultVal; // value returned

    auto const &op = state.dataOutputProcessor;

    // Select based on variable type:  integer, real, or meter
    if (varType == VariableType::Invalid) { // Variable not a found variable
        resultVal = 0.0;
    } else if (varType == VariableType::Integer || varType == VariableType::Real) {
        if (keyVarIndex < 0 || keyVarIndex >= (int)op->outVars.size()) {
            ShowFatalError(state, "GetInternalVariableValue: passed variable index beyond range of array.");
            ShowContinueError(state, format("Index = {} Number of variables = {}", keyVarIndex, op->outVars.size()));
        }

        // must use %Which, %Value is always zero if variable is not a requested report variable
        resultVal = (varType == VariableType::Integer) ? (double)*(dynamic_cast<OutVarInt *>(op->outVars[keyVarIndex]))->Which
                                                       : (double)*(dynamic_cast<OutVarReal *>(op->outVars[keyVarIndex]))->Which;
    } else if (varType == VariableType::Meter) {
        resultVal = GetCurrentMeterValue(state, keyVarIndex);
    } else if (varType == VariableType::Schedule) {
        resultVal = state.dataSched->schedules[keyVarIndex]->getCurrentVal();
    } else {
        resultVal = 0.0;
    }

    return resultVal;
} // GetInternalVariableValue()

Real64 GetInternalVariableValueExternalInterface(EnergyPlusData &state,
                                                 OutputProcessor::VariableType const varType, // 1=integer, 2=REAL(r64), 3=meter
                                                 int const keyVarIndex                        // Array index
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Thierry S. Nouidui
    //       DATE WRITTEN   August 2011

    // PURPOSE OF THIS FUNCTION:
    // This function returns the last zone-timestep value of the Internal Variable assigned to
    // the varType and keyVarIndex.  Values may be accessed for REAL(r64) and integer
    // report variables and meter variables.  The variable type (varType) may be
    // determined by calling subroutine and GetVariableKeyCountandType.  The
    // index (keyVarIndex) may be determined by calling subroutine GetVariableKeys.

    // METHODOLOGY EMPLOYED:
    // Uses Internal OutputProcessor data structure to return value.

    // Using/Aliasing
    using namespace OutputProcessor;

    // Return value
    Real64 resultVal; // value returned

    auto const &op = state.dataOutputProcessor;

    // Select based on variable type:  integer, REAL(r64), or meter
    if (varType == VariableType::Invalid) { // Variable not a found variable
        resultVal = 0.0;
    } else if (varType == VariableType::Integer || varType == VariableType::Real) {
        if (keyVarIndex < 0 || keyVarIndex >= (int)op->outVars.size()) {
            ShowFatalError(state, "GetInternalVariableValueExternalInterface: passed index beyond range of array.");
        }

        resultVal = (double)op->outVars[keyVarIndex]->EITSValue;
    } else if (varType == VariableType::Meter) {
        resultVal = GetCurrentMeterValue(state, keyVarIndex);
    } else if (varType == VariableType::Schedule) {
        resultVal = state.dataSched->schedules[keyVarIndex]->getCurrentVal();
    } else {
        resultVal = 0.0;
    }

    return resultVal;
} // GetInternalVariableValueExternalInterface()

int GetNumMeteredVariables(EnergyPlusData const &state,
                           [[maybe_unused]] std::string const &ComponentType, // Given Component Type
                           std::string const &ComponentName                   // Given Component Name (user defined)
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2005

    // PURPOSE OF THIS FUNCTION:
    // This function counts the number of metered variables associated with the
    // given ComponentType/Name.   This resultant number would then be used to
    // allocate arrays for a call the GetMeteredVariables routine.

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    int NumVariables = 0;
    auto const &op = state.dataOutputProcessor;

    for (auto *var : op->outVars) {
        //    Pos=INDEX(RVariableTypes(Loop)%VarName,':')
        //    IF (ComponentName /= RVariableTypes(Loop)%VarNameUC(1:Pos-1)) CYCLE
        if (var->varType != OutputProcessor::VariableType::Real) {
            continue;
        }
        if (ComponentName != var->keyUC) {
            continue;
        }
        if (!var->meterNums.empty()) {
            ++NumVariables;
        }
    }
    return NumVariables;
} // GetNumMeteredVariables()

int GetMeteredVariables(EnergyPlusData &state,
                        std::string const &ComponentName,                 // Given Component Name (user defined)
                        Array1D<OutputProcessor::MeteredVar> &meteredVars // Variable Types (1=integer, 2=real, 3=meter)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2005
    //       MODIFIED       Jason DeGraw 2/12/2020, de-optionalized
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This routine gets the variable names and other associated information
    // for metered variables associated with the given ComponentType/Name.

    // Using/Aliasing
    using namespace OutputProcessor;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumVariables;
    auto &op = state.dataOutputProcessor;

    NumVariables = 0;

    for (int iVar = 0; iVar < (int)op->outVars.size(); ++iVar) {
        //    Pos=INDEX(RVariableTypes(Loop)%VarName,':')
        //    IF (ComponentName /= RVariableTypes(Loop)%VarNameUC(1:Pos-1)) CYCLE

        auto *var = op->outVars[iVar];
        if (var->varType != VariableType::Real) {
            continue;
        }
        if (ComponentName != var->keyUC) {
            continue;
        }
        if (var->meterNums.empty()) {
            continue;
        }

        auto &meteredVar = meteredVars(++NumVariables);

        meteredVar.num = iVar;
        meteredVar.varType = VariableType::Real;
        meteredVar.timeStepType = var->timeStepType;
        meteredVar.units = var->units;

        meteredVar.resource = op->meters[var->meterNums[0]]->resource;
        meteredVar.name = var->keyColonNameUC;

        bool foundEndUse = false;
        bool foundGroup = false;
        for (int meterNum : var->meterNums) {
            auto *meter = op->meters[meterNum];
            if (!foundEndUse && meter->endUseCat != EndUseCat::Invalid) {
                meteredVar.endUseCat = meter->endUseCat;
                foundEndUse = true;
            }

            if (!foundGroup && meter->group != Group::Invalid) {
                meteredVar.group = meter->group;
                foundGroup = true;
            }

            if (foundEndUse && foundGroup) {
                break;
            }
        }

        meteredVar.rptNum = var->ReportID;
    }
    return NumVariables;
} // GetMeteredVariables()

void GetVariableKeyCountandType(EnergyPlusData &state,
                                std::string const &name, // Standard variable name
                                int &numKeys,            // Number of keys found
                                OutputProcessor::VariableType &varType,
                                OutputProcessor::StoreType &storeType,       // Variable  is Averaged=1 or Summed=2
                                OutputProcessor::TimeStepType &timeStepType, // Variable time step is Zone=1 or HVAC=2
                                Constant::Units &units                       // Units enumeration
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Michael J. Witte
    //       DATE WRITTEN   August 2003

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine returns the variable TYPE (Real, integer, meter, schedule, etc.)
    // (varType) whether it is an averaged or summed variable (varAvgSum),
    // whether it is a zone or HVAC time step (varStepType),
    // and the number of keynames for a given report variable or report meter name
    // (varName).  The variable type (varType) and number of keys (numKeys) are
    // used when calling subroutine GetVariableKeys to obtain a list of the
    // keynames for a particular variable and a corresponding list of indexes.

    // METHODOLOGY EMPLOYED:
    // Uses Internal OutputProcessor data structure to search for varName
    // in each of the three output data arrays:
    //       RVariableTypes - real report variables
    //       IVariableTypes - integer report variables
    //       EnergyMeters   - report meters (via GetMeterIndex function)
    //       Schedules      - specific schedule values
    // When the variable is found, the variable type (varType) is set and the
    // number of associated keys is counted.

    using namespace OutputProcessor;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto const &op = state.dataOutputProcessor;

    varType = VariableType::Invalid;
    numKeys = 0;
    storeType = StoreType::Average;
    timeStepType = TimeStepType::Zone;
    units = Constant::Units::None; // Why is this None and not Invalid?

    std::string nameUC = Util::makeUPPER(name);

    // Search Variable List First
    if (auto found = op->ddOutVarMap.find(nameUC); found != op->ddOutVarMap.end()) {
        auto const *ddOutVar = op->ddOutVars[found->second];
        varType = ddOutVar->variableType;
        storeType = ddOutVar->storeType;
        timeStepType = ddOutVar->timeStepType;
        units = ddOutVar->units;
        numKeys = ddOutVar->keyOutVarNums.size();

    } else if (auto found2 = op->meterMap.find(nameUC); found2 != op->meterMap.end()) {
        // Search Meters if not found in integers or reals
        // Use the GetMeterIndex function
        // Meters do not have keys, so only one will be found
        int meterNum = found2->second;
        numKeys = 1;
        varType = VariableType::Meter;
        units = op->meters[meterNum]->units;
        storeType = StoreType::Sum;
        timeStepType = TimeStepType::Zone;

    } else {
        // Search schedules if not found in integers, reals, or meters
        // Schedules do not have keys, so only one will be found
        auto *sched = Sched::GetSchedule(state, nameUC);
        if (sched != nullptr) {
            numKeys = 1;
            varType = VariableType::Schedule;
            if (sched->schedTypeNum != Sched::SchedNum_Invalid) {
                std::string const &schedTypeName = state.dataSched->scheduleTypes[sched->schedTypeNum]->Name;
                units = static_cast<Constant::Units>(getEnumValue(Constant::unitNamesUC, schedTypeName));
            }
            storeType = StoreType::Average;
            timeStepType = TimeStepType::Zone;
        }
    }
} // GetVariableKeyCountandType()

void GetVariableKeys(EnergyPlusData &state,
                     std::string const &varName,                  // Standard variable name
                     OutputProcessor::VariableType const varType, // 1=integer, 2=real, 3=meter
                     Array1D_string &keyNames,
                     Array1D_int &keyOutVarNums // Array index for

)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Michael J. Witte
    //       DATE WRITTEN   August 2003

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine returns a list of keynames and indexes associated
    // with a particular report variable or report meter name (varName).
    // This routine assumes that the variable TYPE (Real, integer, meter, etc.)
    // may be determined by calling GetVariableKeyCountandType.  The variable type
    // and index can then be used with function GetInternalVariableValue to
    // to retrieve the current value of a particular variable/keyname combination.

    // METHODOLOGY EMPLOYED:
    // Uses Internal OutputProcessor data structure to search for varName
    // and build list of keynames and indexes.  The indexes are the array index
    // in the data array for the

    // Using/Aliasing
    using namespace OutputProcessor;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    std::string nameUC = Util::makeUPPER(varName);

    // Select based on variable type:  integer, real, or meter
    if (varType == VariableType::Integer || varType == VariableType::Real) {
        auto const &op = state.dataOutputProcessor;
        auto found = op->ddOutVarMap.find(nameUC);
        if (found == op->ddOutVarMap.end()) {
            return;
        }

        auto const *ddOutVar = op->ddOutVars[found->second];

        if (ddOutVar->keyOutVarNums.size() > size(keyOutVarNums)) {
            ShowFatalError(state, "Invalid array size in GetVariableKeys");
        }

        int iKey = 0;
        for (int keyOutVarNum : ddOutVar->keyOutVarNums) {
            ++iKey;
            keyOutVarNums(iKey) = keyOutVarNum;
            keyNames(iKey) = op->outVars[keyOutVarNum]->keyUC;
        }

    } else if (varType == VariableType::Meter) { // Meter

        if (size(keyOutVarNums) == 0) {
            ShowFatalError(state, "Invalid array size in GetVariableKeys");
        }
        keyOutVarNums(1) = GetMeterIndex(state, varName);
        keyNames(1) = "Meter";

    } else if (varType == VariableType::Schedule) { // Schedule

        if (size(keyOutVarNums) == 0) {
            ShowFatalError(state, "Invalid array size in GetVariableKeys");
        }
        keyOutVarNums(1) = Sched::GetScheduleNum(state, varName);
        keyNames(1) = "Environment";
    } else {
        // do nothing
    }
} // GetVariableKeys()

bool ReportingThisVariable(EnergyPlusData &state, std::string const &RepVarName)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2008

    // PURPOSE OF THIS FUNCTION:
    // This function scans the report variables and reports back
    // if user has requested this variable be reported.
    using namespace OutputProcessor;

    auto const &op = state.dataOutputProcessor;

    std::string name = Util::makeUPPER(RepVarName);

    for (int iReqVar = 0; iReqVar < (int)op->reqVars.size(); ++iReqVar) {
        if (op->reqVars[iReqVar]->name == name) {
            return true;
        }
    }

    if (auto found = op->meterMap.find(name); found != op->meterMap.end()) {
        auto const *meter = op->meters[found->second];
        for (int iFreq = (int)ReportFreq::TimeStep; iFreq < (int)ReportFreq::Num; ++iFreq) {
            if (iFreq == (int)ReportFreq::Year) {
                continue;
            }
            auto const &period = meter->periods[iFreq];
            if (period.Rpt || period.RptFO || period.accRpt || period.accRptFO) {
                return true;
            }
        }
    }

    return false;
} // ReportingThisVariable()

void InitPollutionMeterReporting(EnergyPlusData &state, OutputProcessor::ReportFreq freq)
{

    // SUBROUTINE INFORMATION:Richard Liesen
    //       DATE WRITTEN   July 2002

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is called at the end of the first HVAC iteration and
    // sets up the reporting for the Pollution Meters.
    // ReportPollutionOutput,
    //   A1 ; \field Reporting_Frequency
    //        \type choice
    //        \key timestep
    //        \key hourly
    //        \key daily
    //        \key monthly
    //        \key runperiod
    // METHODOLOGY EMPLOYED:
    // The program tries to setup all of the following meters if the Pollution Report is initiated.
    //       Electricity:Facility [J]
    //       Diesel:Facility [J]
    //       DistrictCooling:Facility [J]
    //       DistrictHeating:Facility [J]
    //       Gas:Facility [J]
    //       GASOLINE:Facility [J]
    //       COAL:Facility [J]
    //       FuelOilNo1:Facility [J]
    //       FuelOilNo2:Facility [J]
    //       Propane:Facility [J]
    //       ElectricityProduced:Facility [J]
    //       Pollutant:CO2
    //       Pollutant:CO
    //       Pollutant:CH4
    //       Pollutant:NOx
    //       Pollutant:N2O
    //       Pollutant:SO2
    //       Pollutant:PM
    //       Pollutant:PM10
    //       Pollutant:PM2.5
    //       Pollutant:NH3
    //       Pollutant:NMVOC
    //       Pollutant:Hg
    //       Pollutant:Pb
    //       Pollutant:WaterEnvironmentalFactors
    //       Pollutant:Nuclear High
    //       Pollutant:Nuclear Low
    //       Pollutant:Carbon Equivalent

    // Using/Aliasing
    using namespace OutputProcessor;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    auto &op = state.dataOutputProcessor;

    for (int iResource = 0; iResource < (int)Constant::eResource::Num; ++iResource) {
        std::string meterName = format("{}:Facility", Constant::eResourceNames[iResource]);
        std::string meterNameUC = Util::makeUPPER(meterName);

        auto found = op->meterMap.find(meterNameUC);
        if (found == op->meterMap.end()) {
            continue;
        }

        auto *meter = op->meters[found->second];
        auto &period = meter->periods[(int)freq];

        // int indexGroupKey = DetermineIndexGroupKeyFromMeterName(state, meter->Name);
        meter->indexGroup = DetermineIndexGroupFromMeterGroup(meter);

        // All of the specified meters are checked and the headers printed to the meter file if this
        //  has not been done previously
        if (!period.Rpt) {
            period.Rpt = true;
            WriteMeterDictionaryItem(state, freq, StoreType::Sum, period.RptNum, meter->indexGroup, meter->Name, meter->units, false, false);
            op->freqTrackingVariables[(int)freq] = true;
        }
    }
} // InitPollutionMeterReporting()

void ProduceRDDMDD(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2009

    // PURPOSE OF THIS SUBROUTINE:
    // provide a single call for writing out the Report Data Dictionary and Meter Data Dictionary.

    // Using/Aliasing
    using namespace OutputProcessor;
    using General::ScanForReports;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    std::string VarOption1;
    std::string VarOption2;
    bool DoReport;
    bool SortByName;

    auto const &op = state.dataOutputProcessor;
    auto const &rf = state.dataResultsFramework->resultsFramework;

    //  See if Report Variables should be turned on
    SortByName = false;
    ScanForReports(state, "VariableDictionary", DoReport, _, VarOption1, VarOption2);
    //  IF (.not. DoReport) RETURN

    if (DoReport) {
        op->ProduceReportVDD = ReportVDD::Yes;
        if (VarOption1 == std::string("IDF")) {
            op->ProduceReportVDD = ReportVDD::IDF;
        }
        if (!VarOption2.empty()) {
            if (Util::SameString(VarOption2, "Name") || Util::SameString(VarOption2, "AscendingName")) {
                SortByName = true;
            }
        }
    }

    state.files.rdd.ensure_open(state, "ProduceRDDMDD", state.files.outputControl.rdd);
    state.files.mdd.ensure_open(state, "ProduceRDDMDD", state.files.outputControl.mdd);
    if (op->ProduceReportVDD == ReportVDD::Yes) {
        print(state.files.rdd, "Program Version,{},{}{}", state.dataStrGlobals->VerStringVar, state.dataStrGlobals->IDDVerString, '\n');
        print(state.files.rdd, "Var Type (reported time step),Var Report Type,Variable Name [Units]{}", '\n');

        print(state.files.mdd, "Program Version,{},{}{}", state.dataStrGlobals->VerStringVar, state.dataStrGlobals->IDDVerString, '\n');
        print(state.files.mdd, "Var Type (reported time step),Var Report Type,Variable Name [Units]{}", '\n');
    } else if (op->ProduceReportVDD == ReportVDD::IDF) {
        print(state.files.rdd, "! Program Version,{},{}{}", state.dataStrGlobals->VerStringVar, state.dataStrGlobals->IDDVerString, '\n');
        print(state.files.rdd, "! Output:Variable Objects (applicable to this run){}", '\n');

        print(state.files.mdd, "! Program Version,{},{}{}", state.dataStrGlobals->VerStringVar, state.dataStrGlobals->IDDVerString, '\n');
        print(state.files.mdd, "! Output:Meter Objects (applicable to this run){}", '\n');
    }

    if (op->ProduceReportVDD == ReportVDD::Yes || op->ProduceReportVDD == ReportVDD::IDF) {

        auto miVar = op->ddOutVarMap.begin();
        int aiVar = 0;
        for (;;) {
            int iVar = -1;
            // Too complicated to do this logic in the for loop header
            if (SortByName) {
                if (miVar == op->ddOutVarMap.end()) {
                    break;
                }
                iVar = miVar->second;
                ++miVar;
            } else {
                if (aiVar == (int)op->ddOutVars.size()) {
                    break;
                }
                iVar = aiVar;
                ++aiVar;
            }

            auto *ddVar = op->ddOutVars[iVar];

            if (ddVar->ReportedOnDDFile) {
                continue;
            }

            static constexpr std::array<std::string_view, (int)TimeStepType::Num> timeStepNamesLocal = {"Zone", "HVAC"};
            std::string_view timeStepName = timeStepNamesLocal[(int)ddVar->timeStepType];
            std::string_view storeTypeName = storeTypeNames[(int)ddVar->storeType];
            std::string_view varName = ddVar->name;
            std::string_view unitName =
                (ddVar->units == Constant::Units::customEMS) ? ddVar->unitNameCustomEMS : Constant::unitNames[(int)ddVar->units];
            if (op->ProduceReportVDD == ReportVDD::Yes) {
                print(state.files.rdd, "{},{},{} [{}]\n", timeStepName, storeTypeName, varName, unitName);
                rf->RDD.push_back(format("{},{},{} [{}]", timeStepName, storeTypeName, varName, unitName));
            } else {
                print(state.files.rdd, "Output:Variable,*,{},hourly; !- {} {} [{}]\n", varName, timeStepName, storeTypeName, unitName);
                rf->RDD.push_back(format("{},{},{} [{}]", timeStepName, storeTypeName, varName, unitName));
            }

            ddVar->ReportedOnDDFile = true;
            if (SortByName) {
                while (ddVar->Next != -1) {
                    ddVar = op->ddOutVars[ddVar->Next];

                    timeStepName = timeStepTypeNames[(int)ddVar->timeStepType];
                    storeTypeName = storeTypeNames[(int)ddVar->storeType];
                    varName = ddVar->name;

                    if (op->ProduceReportVDD == ReportVDD::Yes) {
                        print(state.files.rdd, "{},{},{} [{}]\n", timeStepName, storeTypeName, varName, unitName);
                        rf->RDD.push_back(format("{},{},{} [{}]", timeStepName, storeTypeName, varName, unitName));
                    } else {
                        print(state.files.rdd, "Output:Variable,*,{},hourly; !- {} {} [{}]\n", varName, timeStepName, storeTypeName, unitName);
                        rf->RDD.push_back(format("{},{},{} [{}]", timeStepName, storeTypeName, varName, unitName));
                    }
                    ddVar->ReportedOnDDFile = true;
                } // while (ddVar->Next != 0)
            } // if (SortByName)
        } // for (aiVar, miVar)
    } // if (produceReportVDD)
    state.files.rdd.close();

    auto miMeter = op->meterMap.begin();
    for (int aiMeter = 0; aiMeter < (int)op->meters.size() && miMeter != op->meterMap.end(); ++aiMeter, ++miMeter) {
        int iMeter = (SortByName) ? miMeter->second : aiMeter;
        auto *meter = op->meters[iMeter];
        std::string_view unitName = Constant::unitNames[(int)meter->units];
        if (op->ProduceReportVDD == ReportVDD::Yes) {
            print(state.files.mdd, "Zone,Meter,{} [{}]\n", meter->Name, unitName);
            rf->MDD.push_back(format("Zone,Meter,{} [{}]", meter->Name, unitName));
        } else if (op->ProduceReportVDD == ReportVDD::IDF) {
            print(state.files.mdd, "Output:Meter,{},hourly; !- [{}]\n", meter->Name, unitName);
            rf->MDD.push_back(format("Output:Meter,{} [{}]", meter->Name, unitName));
            print(state.files.mdd, "Output:Meter:Cumulative,{},hourly; !- [{}]\n", meter->Name, unitName);
            rf->MDD.push_back(format("Output:Meter:Cumulative,{} [{}]", meter->Name, unitName));
        }
    }
    state.files.mdd.close();
} // ProduceRDDMDD()

int AddDDOutVar(EnergyPlusData const &state,
                std::string_view const name, // Variable Name
                OutputProcessor::TimeStepType const timeStepType,
                OutputProcessor::StoreType const storeType,
                OutputProcessor::VariableType const variableType,
                Constant::Units const units,
                std::string_view const customUnitName // the custom name for the units from EMS definition of units
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   August 2010

    // PURPOSE OF THIS SUBROUTINE:
    // This routine maintains a unique list of Output Variables for the
    // Variable Dictionary output.

    // Using/Aliasing
    using namespace OutputProcessor;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto const &op = state.dataOutputProcessor;

    std::string nameUC = Util::makeUPPER(name);

    auto found = op->ddOutVarMap.find(nameUC);
    if (found == op->ddOutVarMap.end()) {
        auto *ddVar = new DDOutVar();
        op->ddOutVars.push_back(ddVar);
        // Add to map
        op->ddOutVarMap.insert_or_assign(nameUC, op->ddOutVars.size() - 1);

        ddVar->timeStepType = timeStepType;
        ddVar->storeType = storeType;
        ddVar->variableType = variableType;
        ddVar->name = name;
        ddVar->units = units;
        if (!customUnitName.empty() && units == Constant::Units::customEMS) {
            ddVar->unitNameCustomEMS = customUnitName;
        }
        return op->ddOutVars.size() - 1;

    } else if (units == op->ddOutVars[found->second]->units) {
        return found->second;

    } else {           // not the same as first units
        int dup2 = -1; // for duplicate variable name
        auto *ddVarDup = op->ddOutVars[found->second];
        while (ddVarDup->Next != -1) {
            if (units != op->ddOutVars[ddVarDup->Next]->units) {
                ddVarDup = op->ddOutVars[ddVarDup->Next];
                continue;
            }
            dup2 = ddVarDup->Next;
            break;
        }
        if (dup2 == -1) {
            DDOutVar *ddVar2 = new DDOutVar();
            op->ddOutVars.push_back(ddVar2);
            // Don't add this one to the map.  Leave the map pointing to the first one
            ddVar2->timeStepType = timeStepType;
            ddVar2->storeType = storeType;
            ddVar2->variableType = variableType;
            ddVar2->name = name;
            ddVar2->units = units;
            if (!customUnitName.empty() && units == Constant::Units::customEMS) {
                ddVar2->unitNameCustomEMS = customUnitName;
            }
            ddVarDup->Next = op->ddOutVars.size() - 1;

            return op->ddOutVars.size() - 1;
        } else {
            return dup2;
        } // if (dup2 == 0)
    } // if (unitsForVar)
} // AddDDOutVar()

int initErrorFile(EnergyPlusData &state)
{
    state.files.err_stream = std::make_unique<std::ofstream>(state.files.outputErrFilePath);
    if (state.files.err_stream->bad()) {
        DisplayString(state, fmt::format("ERROR: Could not open file {} for output (write).", state.files.outputErrFilePath));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
} // initErrorFile()

} // namespace EnergyPlus
