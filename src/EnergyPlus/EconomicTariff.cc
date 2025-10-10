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
#include <cassert>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/numeric.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/CostEstimateManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/EconomicTariff.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/ResultsFramework.hh>
#include <EnergyPlus/SQLiteProcedures.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::EconomicTariff {

// MODULE INFORMATION:
//    AUTHOR         Jason Glazer of GARD Analytics, Inc.
//    DATE WRITTEN   May 2004

//    Compute utility bills for a building based on energy
//    use estimate.

constexpr std::array<std::string_view, (int)EconConv::Num> convEnergyStrings = {
    "", "kWh", "Therm", "MMBtu", "MJ", "kBTU", "MCF", "CCF", "m3", "gal", "kgal"};
constexpr std::array<std::string_view, (int)EconConv::Num> convDemandStrings = {
    "", "kW", "Therm", "MMBtu", "MJ", "kBTU", "MCF", "CCF", "m3", "gal", "kgal"};
constexpr std::array<std::string_view, (int)EconConv::Num> econConvNamesUC = {
    "USERDEFINED", "KWH", "THERM", "MMBTU", "MJ", "KBTU", "MCF", "CCF", "M3", "GAL", "KGAL"};

constexpr std::array<std::string_view, (int)DemandWindow::Num> demandWindowStrings = {"/Hr", "/Hr", "/Hr", "/Day", "/Wk"};

constexpr std::array<std::string_view, (int)BuySell::Num> buySellNames = {"BuyFromUtility", "SellToUtility", "NetMetering"};
constexpr std::array<std::string_view, (int)BuySell::Num> buySellNamesUC = {"BUYFROMUTILITY", "SELLTOUTILITY", "NETMETERING"};

constexpr std::array<std::string_view, (int)Season::Num> seasonNames = {"Unused", "Winter", "Spring", "Summer", "Fall", "Annual", "Monthly"};
constexpr std::array<std::string_view, (int)Season::Num> seasonNamesUC = {"Unused", "WINTER", "SPRING", "SUMMER", "FALL", "ANNUAL", "MONTHLY"};

constexpr std::array<std::string_view, (int)Op::Num> opNamesUC = {"SUM",
                                                                  "MULTIPLY",
                                                                  "SUBTRACT",
                                                                  "DIVIDE",
                                                                  "ABSOLUTE",
                                                                  "INTEGER",
                                                                  "SIGN",
                                                                  "ROUND",
                                                                  "MAXIMUM",
                                                                  "MINIMUM",
                                                                  "EXCEEDS",
                                                                  "ANNUALMINIMUM",
                                                                  "ANNUALMAXIMUM",
                                                                  "ANNUALSUM",
                                                                  "ANNUALAVERAGE",
                                                                  "ANNUALOR",
                                                                  "ANNUALAND",
                                                                  "ANNUALMAXIMUMZERO",
                                                                  "ANNUALMINIMUMZERO",
                                                                  "IF",
                                                                  "GREATERTHAN",
                                                                  "GREATEREQUAL",
                                                                  "LESSTHAN",
                                                                  "LESSEQUAL",
                                                                  "EQUAL",
                                                                  "NOTEQUAL",
                                                                  "AND",
                                                                  "OR",
                                                                  "NOT",
                                                                  "ADD",
                                                                  "FROM"};

constexpr std::array<std::string_view, (int)Op::Num> opNames2UC = {
    "SUM",   "MULT",   "SUBT",   "DIV", "ABS", "INT", "SIGN", "ROUND", "MAX", "MIN", "EXCEEDS", "ANMIN", "ANMAX", "ANSUM", "ANAVG", "ANOR",
    "ANAND", "ANMAXZ", "ANMINZ", "IF",  "GT",  "GE",  "LT",   "LE",    "EQ",  "NE",  "AND",     "OR",    "NOT",   "ADD",   "NOOP"};

constexpr std::array<std::string_view, (int)Cat::Num> catNames = {
    "EnergyCharges", "DemandCharges", "ServiceCharges", "Basis", "Adjustment", "Surcharge", "Subtotal", "Taxes", "Total", "NotIncluded"};
constexpr std::array<std::string_view, (int)Cat::Num> catNamesUC = {
    "ENERGYCHARGES", "DEMANDCHARGES", "SERVICECHARGES", "BASIS", "ADJUSTMENT", "SURCHARGE", "SUBTOTAL", "TAXES", "TOTAL", "NOTINCLUDED"};

constexpr std::array<std::string_view, (int)Native::Num> nativeNames = {"TotalEnergy",
                                                                        "TotalDemand",
                                                                        "PeakEnergy",
                                                                        "PeakDemand",
                                                                        "ShoulderEnergy",
                                                                        "ShoulderDemand",
                                                                        "OffPeakEnergy",
                                                                        "OffPeakDemand",
                                                                        "MidPeakEnergy",
                                                                        "MidPeakDemand",
                                                                        "PeakExceedsOffPeak",
                                                                        "OffPeakExceedsPeak",
                                                                        "PeakExceedsMidPeak",
                                                                        "MidPeakExceedsPeak",
                                                                        "PeakExceedsShoulder",
                                                                        "ShoulderExceedsPeak",
                                                                        "IsWinter",
                                                                        "IsNotWinter",
                                                                        "IsSpring",
                                                                        "IsNotSpring",
                                                                        "IsSummer",
                                                                        "IsNotSummer",
                                                                        "IsAutumn",
                                                                        "IsNotAutumn",
                                                                        "PeakAndShoulderEnergy",
                                                                        "PeakAndShoulderDemand",
                                                                        "PeakAndMidPeakEnergy",
                                                                        "PeakAndMidPeakDemand",
                                                                        "ShoulderAndOffPeakEnergy",
                                                                        "ShoulderAndOffPeakDemand",
                                                                        "PeakAndOffPeakEnergy",
                                                                        "PeakAndOffPeakDemand",
                                                                        "RealTimePriceCosts",
                                                                        "AboveCustomerBaseCosts",
                                                                        "BelowCustomerBaseCosts",
                                                                        "AboveCustomerBaseEnergy",
                                                                        "BelowCustomerBaseEnergy"};
constexpr std::array<std::string_view, (int)Native::Num> nativeNamesUC = {"TOTALENERGY",
                                                                          "TOTALDEMAND",
                                                                          "PEAKENERGY",
                                                                          "PEAKDEMAND",
                                                                          "SHOULDERENERGY",
                                                                          "SHOULDERDEMAND",
                                                                          "OFFPEAKENERGY",
                                                                          "OFFPEAKDEMAND",
                                                                          "MIDPEAKENERGY",
                                                                          "MIDPEAKDEMAND",
                                                                          "PEAKEXCEEDSOFFPEAK",
                                                                          "OFFPEAKEXCEEDSPEAK",
                                                                          "PEAKEXCEEDSMIDPEAK",
                                                                          "MIDPEAKEXCEEDSPEAK",
                                                                          "PEAKEXCEEDSSHOULDER",
                                                                          "SHOULDEREXCEEDSPEAK",
                                                                          "ISWINTER",
                                                                          "ISNOTWINTER",
                                                                          "ISSPRING",
                                                                          "ISNOTSPRING",
                                                                          "ISSUMMER",
                                                                          "ISNOTSUMMER",
                                                                          "ISAUTUMN",
                                                                          "ISNOTAUTUMN",
                                                                          "PEAKANDSHOULDERENERGY",
                                                                          "PEAKANDSHOULDERDEMAND",
                                                                          "PEAKANDMIDPEAKENERGY",
                                                                          "PEAKANDMIDPEAKDEMAND",
                                                                          "SHOULDERANDOFFPEAKENERGY",
                                                                          "SHOULDERANDOFFPEAKDEMAND",
                                                                          "PEAKANDOFFPEAKENERGY",
                                                                          "PEAKANDOFFPEAKDEMAND",
                                                                          "REALTIMEPRICECOSTS",
                                                                          "ABOVECUSTOMERBASECOSTS",
                                                                          "BELOWCUSTOMERBASECOSTS",
                                                                          "ABOVECUSTOMERBASEENERGY",
                                                                          "BELOWCUSTOMERBASEENERGY"};

constexpr std::array<std::string_view, (int)VarUnitType::Num> varUnitTypeNames = {"Energy", "Demand", "Dimensionless", "Currency"};
constexpr std::array<std::string_view, (int)VarUnitType::Num> varUnitTypeNamesUC = {"ENERGY", "DEMAND", "DIMENSIONLESS", "CURRENCY"};

void UpdateUtilityBills(EnergyPlusData &state)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   September 2003

    //    Single routine used to call all get input
    //    routines for economics.

    auto &s_econ = state.dataEconTariff;

    if (s_econ->Update_GetInput) {
        bool ErrorsFound = false;

        GetInputEconomicsTariff(state, ErrorsFound);
        // do rest of GetInput only if at least one tariff is defined.
        GetInputEconomicsCurrencyType(state, ErrorsFound);
        if (s_econ->numTariff >= 1) {
            if (!ErrorsFound && state.dataOutRptTab->displayEconomicResultSummary) {
                OutputReportTabular::AddTOCEntry(state, "Economics Results Summary Report", "Entire Facility");
            }
            CreateCategoryNativeVariables(state);
            GetInputEconomicsQualify(state, ErrorsFound);
            GetInputEconomicsChargeSimple(state, ErrorsFound);
            GetInputEconomicsChargeBlock(state, ErrorsFound);
            GetInputEconomicsRatchet(state, ErrorsFound);
            GetInputEconomicsVariable(state, ErrorsFound);
            GetInputEconomicsComputation(state, ErrorsFound);
            CreateDefaultComputation(state);
        }
        s_econ->Update_GetInput = false;
        if (ErrorsFound) {
            ShowFatalError(state, "UpdateUtilityBills: Preceding errors cause termination.");
        }
    }
    if (state.dataGlobal->DoOutputReporting && (state.dataGlobal->KindOfSim == Constant::KindOfSim::RunPeriodWeather)) {
        GatherForEconomics(state);
    }
}

//======================================================================================================================
//======================================================================================================================

//    GET INPUT ROUTINES

//======================================================================================================================
//======================================================================================================================

void GetInputEconomicsTariff(EnergyPlusData &state, bool &ErrorsFound) // true if errors found during getting input objects.
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //       DATE WRITTEN   May 2004
    //       MODIFIED       Aug. 2017, Julien Marrec of EffiBEM. Handled conversions factor based on meter resources
    //
    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine reads the input file for "UtilityCost:Tariff" objects
    // It will be the right conversion factors based on the associated meter resource type
    // meaning if "CCF" is picked, the conversion factor isn't the same whether it's a water meter or a fuel meter.

    static constexpr std::string_view RoutineName("GetInputEconomicsTariff: ");
    static constexpr std::string_view routineName = "GetInputEconomicsTariff";

    int NumAlphas; // Number of elements in the alpha array
    int NumNums;   // Number of elements in the numeric array
    int IOStat;    // IO Status when calling get input subroutine
    bool isNotNumeric;
    // variables for getting report variable/meter index
    int KeyCount;
    OutputProcessor::VariableType TypeVar;
    OutputProcessor::StoreType AvgSumVar;
    OutputProcessor::TimeStepType StepTypeVar;
    Constant::Units UnitsVar = Constant::Units::None; // Units sting, may be blank
    Array1D_string NamesOfKeys;                       // Specific key name
    Array1D_int IndexesForKeyVar;                     // Array index

    auto &s_ipsc = state.dataIPShortCut;
    auto &s_econ = state.dataEconTariff;

    std::string_view CurrentModuleObject = "UtilityCost:Tariff";
    s_econ->numTariff = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    s_econ->tariff.allocate(s_econ->numTariff);
    for (int iInObj = 1; iInObj <= s_econ->numTariff; ++iInObj) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 iInObj,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};
        auto &tariff = s_econ->tariff(iInObj);

        // check to make sure none of the values are another economic object
        for (int jFld = 1; jFld <= NumAlphas; ++jFld) {
            //  args are always turned to upper case but this is okay...
            if (hasi(s_ipsc->cAlphaArgs(jFld), "UtilityCost:")) {
                ShowWarningError(state, format("{}{}=\"{}\".", RoutineName, CurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "... a field was found containing UtilityCost: which may indicate a missing comma.");
            }
        }
        // name of the tariff
        tariff.tariffName = s_ipsc->cAlphaArgs(1);
        // check if tariff name is unique
        int found = 0;
        for (int jObj = 1; jObj <= iInObj - 1; ++jObj) {
            if (tariff.tariffName == s_econ->tariff(jObj).tariffName) {
                found = jObj;
                break;
            }
        }
        if (found > 0) {
            ShowSevereDuplicateName(state, eoh);
            ErrorsFound = true;
        }
        // name of the report meter
        tariff.reportMeter = s_ipsc->cAlphaArgs(2);
        // call the key count function but only need count during this pass
        GetVariableKeyCountandType(state, tariff.reportMeter, KeyCount, TypeVar, AvgSumVar, StepTypeVar, UnitsVar);
        // if no meters found for that name
        if (KeyCount == 0) {
            ShowWarningError(state, format("{}{}=\"{}\" missing meter", RoutineName, CurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(
                state,
                format("Meter referenced is not present due to a lack of equipment that uses that energy source/meter:\"{}\".", tariff.reportMeter));
            tariff.reportMeterIndx = -1;
        } else {
            NamesOfKeys.allocate(KeyCount);
            IndexesForKeyVar.allocate(KeyCount);
            GetVariableKeys(state, tariff.reportMeter, TypeVar, NamesOfKeys, IndexesForKeyVar);
            // although this retrieves all keys for a variable, we only need one so the first one is chosen
            if (KeyCount > 1) {
                ShowWarningError(state, format("{}{}=\"{}\" multiple keys", RoutineName, CurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "... Multiple keys for variable select. First key will be used.");
            }
            // assign the index
            tariff.reportMeterIndx = IndexesForKeyVar(1);
            // get rid of the arrays used to get the variable number
            NamesOfKeys.deallocate();
            IndexesForKeyVar.deallocate();
        }

        // Start by checking what type of meter we do have, some units can be used for several resources with different conversion factors
        // Explicitly assume it's not a water meter nor an electric meter nor a gas meter (was already done in constructor though)
        tariff.kindMtr = MeterType::Other;

        // Determine whether this meter is related to electricity, or water, or gas
        if (tariff.reportMeterIndx != -1) {
            switch (state.dataOutputProcessor->meters[tariff.reportMeterIndx]->resource) {
            // Various types of electricity meters
            case Constant::eResource::Electricity: {
                tariff.kindMtr = MeterType::ElecSimple;
            } break;
            case Constant::eResource::ElectricityProduced: {
                tariff.kindMtr = MeterType::ElecProduced;
            } break;
            case Constant::eResource::ElectricityPurchased: {
                tariff.kindMtr = MeterType::ElecPurchased;
            } break;
            case Constant::eResource::ElectricitySurplusSold: {
                tariff.kindMtr = MeterType::ElecSurplusSold;
            } break;
            case Constant::eResource::ElectricityNet: {
                tariff.kindMtr = MeterType::ElecNet;
            } break;
            // Handle the case where its a water meter
            case Constant::eResource::Water:
            case Constant::eResource::OnSiteWater:
            case Constant::eResource::MainsWater:
            case Constant::eResource::RainWater:
            case Constant::eResource::WellWater:
            case Constant::eResource::Condensate: {
                tariff.kindMtr = MeterType::Water;
            } break;
            // Or a Natural Gas meter
            case Constant::eResource::NaturalGas: {
                tariff.kindMtr = MeterType::Gas;
            } break;
            default: {
                tariff.kindMtr = MeterType::Other; // Do or assert something here?
            } break;
            } // switch
        }

        // Assign the right conversion factors based on the resource type

        // If it's a water meter
        // We set demandConv to something analogous to m3/h
        switch (tariff.kindMtr) {
        case MeterType::Water: {
            // conversion factor
            tariff.convChoice = static_cast<EconConv>(getEnumValue(econConvNamesUC, s_ipsc->cAlphaArgs(3)));
            switch (tariff.convChoice) {
            case EconConv::USERDEF: {
                tariff.energyConv = s_ipsc->rNumericArgs(1); // energy conversion factor
                tariff.demandConv = s_ipsc->rNumericArgs(2); // demand conversion factor
            } break;

            case EconConv::M3: {
                tariff.energyConv = 1.0;
                tariff.demandConv = 3600.0;
            } break;

            case EconConv::CCF: {
                tariff.energyConv = 0.35314666721488586;
                tariff.demandConv = 0.35314666721488586 * 3600;
            } break;

            case EconConv::GAL: {
                tariff.energyConv = 264.1720523602524;
                tariff.demandConv = 264.1720523602524 * 3600;
            } break;

            case EconConv::KGAL: {
                tariff.energyConv = 0.2641720523602524;
                tariff.demandConv = 0.2641720523602524 * 3600;
            } break;

            default: { // ERROR: not a valid conversion, default to M3
                tariff.convChoice = EconConv::M3;
                tariff.energyConv = 1.0;
                tariff.demandConv = 3600.0;
                ShowWarningInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), "M3");
            } break;
            } // switch (tariff.convChoice)

        } break;

            // If it's an electric meter
            // Volumetric units such as MCF or CCF doesn't make sense IMHO (JM)
            // THERM is strange for an electric meter but currently I accept but issue a warning
        case MeterType::ElecSimple:
        case MeterType::ElecProduced:
        case MeterType::ElecPurchased:
        case MeterType::ElecSurplusSold:
        case MeterType::ElecNet: {
            tariff.convChoice = static_cast<EconConv>(getEnumValue(econConvNamesUC, s_ipsc->cAlphaArgs(3)));

            switch (tariff.convChoice) {
            case EconConv::USERDEF: {
                tariff.energyConv = s_ipsc->rNumericArgs(1); // energy conversion factor
                tariff.demandConv = s_ipsc->rNumericArgs(2); // demand conversion factor
            } break;

            case EconConv::KWH: {
                tariff.energyConv = 0.0000002778;
                tariff.demandConv = 0.001;
            } break;

            case EconConv::MJ: {
                tariff.energyConv = 0.000001;
                tariff.demandConv = 0.0036;
            } break;

            case EconConv::MMBTU: {
                tariff.energyConv = 9.4781712e-10;
                tariff.demandConv = 0.000003412;
            } break;

            case EconConv::KBTU: {
                tariff.energyConv = 9.4781712e-7;
                tariff.demandConv = 0.003412;
            } break;

                // We accept the following choices, but issue a warning
            case EconConv::THERM: {
                tariff.energyConv = 9.4781712e-9;
                tariff.demandConv = 0.00003412;
                ShowWarningCustom(
                    state,
                    eoh,
                    format("{}=\"{}\", Therm is an unusual choice for an electric resource.", s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3)));
            } break;

                // Otherwise, default to kWh
            default: {
                tariff.convChoice = EconConv::KWH;
                tariff.energyConv = 0.0000002778;
                tariff.demandConv = 0.001;
                ShowWarningInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), "KWH");
            } break;
            } // switch (tariff.convChoice)
        } break;

            // If it's a gas meter
        case MeterType::Gas: {
            tariff.convChoice = static_cast<EconConv>(getEnumValue(econConvNamesUC, s_ipsc->cAlphaArgs(3)));
            switch (tariff.convChoice) {
            case EconConv::USERDEF: {
                tariff.energyConv = s_ipsc->rNumericArgs(1); // energy conversion factor
                tariff.demandConv = s_ipsc->rNumericArgs(2); // demand conversion factor
            } break;

            case EconConv::KWH: {
                tariff.energyConv = 0.0000002778;
                tariff.demandConv = 0.001;
            } break;

            case EconConv::THERM: {
                tariff.energyConv = 9.4781712e-9;
                tariff.demandConv = 0.00003412;
            } break;

            case EconConv::MMBTU: {
                tariff.energyConv = 9.4781712e-10;
                tariff.demandConv = 0.000003412;
            } break;

            case EconConv::MJ: {
                tariff.energyConv = 0.000001;
                tariff.demandConv = 0.0036;
            } break;

            case EconConv::KBTU: {
                tariff.energyConv = 9.4781712e-7;
                tariff.demandConv = 0.003412;
            } break;
                // Volumetric units for natural gas
                // Actually assuming 1 therm = 1 CCF (= 100 ft^3)
            case EconConv::MCF: {
                tariff.energyConv = 9.4781712e-10;
                tariff.demandConv = 0.000003412;
            } break;

            case EconConv::CCF: {
                tariff.energyConv = 9.4781712e-9;
                tariff.demandConv = 0.00003412;
            } break;

                // Obtained from converting CCF above to m^3 so the same heat content of natural gas is used (1 therm = 1 CCF)
            case EconConv::M3: {
                tariff.energyConv = 2.6839192e-10;
                tariff.demandConv = 9.6617081E-05;
            } break;

                // Otherwise, default to kWh
            default: {
                tariff.convChoice = EconConv::KWH;
                tariff.energyConv = 0.0000002778;
                tariff.demandConv = 0.001;
                ShowWarningInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), "KWH");
            } break;
            } // witch (tariff.convChoice)
        } break;

            // It it's neither an electric, water or gas meter, we cannot accept volumetric units
            // because we cannot infer the heat content
        case MeterType::Other: {
            tariff.convChoice = static_cast<EconConv>(getEnumValue(econConvNamesUC, s_ipsc->cAlphaArgs(3)));
            switch (tariff.convChoice) {
            case EconConv::USERDEF: {
                tariff.energyConv = s_ipsc->rNumericArgs(1); // energy conversion factor
                tariff.demandConv = s_ipsc->rNumericArgs(2); // demand conversion factor
            } break;

            case EconConv::KWH: {
                tariff.energyConv = 0.0000002778;
                tariff.demandConv = 0.001;
            } break;

            case EconConv::THERM: {
                tariff.energyConv = 9.4781712e-9;
                tariff.demandConv = 0.00003412;
            } break;

            case EconConv::MMBTU: {
                tariff.energyConv = 9.4781712e-10;
                tariff.demandConv = 0.000003412;
            } break;

            case EconConv::MJ: {
                tariff.energyConv = 0.000001;
                tariff.demandConv = 0.0036;
            } break;

            case EconConv::KBTU: {
                tariff.energyConv = 9.4781712e-7;
                tariff.demandConv = 0.003412;
            } break;

                // Otherwise, default to kWh
            default: {
                tariff.convChoice = EconConv::KWH;
                tariff.energyConv = 0.0000002778;
                tariff.demandConv = 0.001;
                ShowWarningInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), "KWH");
            } break;
            } // switch (tariff.convChoice)
        } break;

        default: {
        } break;
        } // Default conversion factors have been applied from here on

        // schedules
        // period schedule
        if (len(s_ipsc->cAlphaArgs(4)) > 0) {
            if ((tariff.periodSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
                ErrorsFound = true;
            }
        }

        // season schedule
        if (len(s_ipsc->cAlphaArgs(5)) > 0) {
            if ((tariff.seasonSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(5))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5));
                ErrorsFound = true;
            }
        }

        // month schedule
        if (len(s_ipsc->cAlphaArgs(6)) > 0) {
            if ((tariff.monthSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(6))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6));
                ErrorsFound = true;
            }
        }
        // type of demand window
        if (Util::SameString(s_ipsc->cAlphaArgs(7), "QuarterHour")) {
            // check to make sure that the demand window and the TIMESTEP IN HOUR are consistent.
            { // Why is this a nested scope?
                switch (state.dataGlobal->TimeStepsInHour) {
                case 1:
                case 3:
                case 5:
                case 15: {
                    tariff.demandWindow = DemandWindow::Hour;
                    tariff.demWinTime = 1.00;
                    ShowWarningError(state, format("{}{}=\"{}\" invalid data", RoutineName, CurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state,
                                      format("Demand window of QuarterHour is not consistent with number of timesteps per hour [{}].",
                                             state.dataGlobal->TimeStepsInHour));
                    ShowContinueError(state, "Demand window will be set to FullHour, and the simulation continues.");
                } break;
                case 2:
                case 6:
                case 10:
                case 30: {
                    tariff.demandWindow = DemandWindow::Half;
                    tariff.demWinTime = 0.50;
                    ShowWarningError(state, format("{}{}=\"{}\" invalid data", RoutineName, CurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state,
                                      format("Demand window of QuarterHour is not consistent with number of timesteps per hour [{}].",
                                             state.dataGlobal->TimeStepsInHour));
                    ShowContinueError(state, "Demand window will be set to HalfHour, and the simulation continues.");
                } break;
                case 4:
                case 12:
                case 20:
                case 60: {
                    tariff.demandWindow = DemandWindow::Quarter;
                    tariff.demWinTime = 0.25;
                } break;
                default: {
                    assert(false);
                } break;
                }
            }
        } else if (Util::SameString(s_ipsc->cAlphaArgs(7), "HalfHour")) {
            {
                switch (state.dataGlobal->TimeStepsInHour) {
                case 1:
                case 3:
                case 5:
                case 15: {
                    tariff.demandWindow = DemandWindow::Hour;
                    tariff.demWinTime = 1.00;
                    ShowWarningError(state, format("{}{}=\"{}\" invalid data", RoutineName, CurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state,
                                      format("Demand window of HalfHour is not consistent with number of timesteps per hour [{}].",
                                             state.dataGlobal->TimeStepsInHour));
                    ShowContinueError(state, "Demand window will be set to FullHour, and the simulation continues.");
                } break;
                case 2:
                case 4:
                case 6:
                case 10:
                case 12:
                case 20:
                case 30:
                case 60: {
                    tariff.demandWindow = DemandWindow::Half;
                    tariff.demWinTime = 0.50;
                } break;
                default: {
                    // assert(false); // EconomicTariff unit test gets here with NumOfTimeStepInHour == 0
                } break;
                }
            }
        } else if (Util::SameString(s_ipsc->cAlphaArgs(7), "FullHour")) {
            tariff.demandWindow = DemandWindow::Hour;
            tariff.demWinTime = 1.00;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(7), "Day")) {
            tariff.demandWindow = DemandWindow::Day;
            tariff.demWinTime = 24.00;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(7), "Week")) {
            tariff.demandWindow = DemandWindow::Week;
            tariff.demWinTime = 24.0 * 7.0;
        } else {
            // if not entered default to the same logic as quarter of an hour
            {
                switch (state.dataGlobal->TimeStepsInHour) {
                case 1:
                case 3:
                case 5:
                case 15: {
                    tariff.demandWindow = DemandWindow::Hour;
                    tariff.demWinTime = 1.00;
                } break;
                case 2:
                case 6:
                case 10:
                case 30: {
                    tariff.demandWindow = DemandWindow::Half;
                    tariff.demWinTime = 0.50;
                } break;
                case 4:
                case 12:
                case 20:
                case 60: {
                    tariff.demandWindow = DemandWindow::Quarter;
                    tariff.demWinTime = 0.25;
                } break;
                default: {
                    // assert(false); // EconomicTariff unit test got here with NumOfTimeStepInHour == 0
                } break;
                }
            }
        }
        // monthly charge
        tariff.monthChgVal = Util::ProcessNumber(s_ipsc->cAlphaArgs(8), isNotNumeric);
        tariff.monthChgPt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(8), isNotNumeric, varIsArgument, varNotYetDefined, ObjType::Invalid, 0, iInObj);
        // minimum monthly charge
        if (len(s_ipsc->cAlphaArgs(9)) > 0) {
            tariff.minMonthChgVal = Util::ProcessNumber(s_ipsc->cAlphaArgs(9), isNotNumeric);
        } else {
            tariff.minMonthChgVal = -HUGE_(-1.0); // set to a very negative value
        }
        tariff.minMonthChgPt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(9), isNotNumeric, varIsArgument, varNotYetDefined, ObjType::Invalid, 0, iInObj);
        // real time pricing
        tariff.chargeSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(10));
        tariff.baseUseSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(11));
        // group name for separate distribution and transmission rates
        tariff.groupName = s_ipsc->cAlphaArgs(12);
        // buy or sell option

        if (s_ipsc->lAlphaFieldBlanks(13)) {
            tariff.buyOrSell = BuySell::BuyFromUtility;
        } else if ((tariff.buyOrSell = static_cast<BuySell>(getEnumValue(buySellNamesUC, s_ipsc->cAlphaArgs(13)))) == BuySell::Invalid) {
            ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(13), s_ipsc->cAlphaArgs(13));
            ErrorsFound = true;
        }

        // check if meter is consistent with buy or sell option
        if (tariff.buyOrSell == BuySell::SellToUtility) {
            if (!Util::SameString(tariff.reportMeter, "ELECTRICITYSURPLUSSOLD:FACILITY")) {
                ShowWarningError(state, format("{}{}=\"{}\" atypical meter", RoutineName, CurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, format("The meter chosen \"{}\" is not typically used with the sellToUtility option.", tariff.reportMeter));
                ShowContinueError(state, "Usually the ElectricitySurplusSold:Facility meter is selected when the sellToUtility option is used.");
            }
        } else if (tariff.buyOrSell == BuySell::NetMetering) {
            if (!Util::SameString(tariff.reportMeter, "ELECTRICITYNET:FACILITY")) {
                ShowWarningError(state, format("{}{}=\"{}\" atypical meter", RoutineName, CurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, format("The meter chosen \"{}\" is not typically used with the netMetering option.", tariff.reportMeter));
                ShowContinueError(state, "Usually the ElectricityNet:Facility meter is selected when the netMetering option is used.");
            }
        } else if (tariff.buyOrSell == BuySell::BuyFromUtility) {
            if (hasi(tariff.reportMeter, "Elec")) { // test if electric meter
                if (!(Util::SameString(tariff.reportMeter, "Electricity:Facility") ||
                      Util::SameString(tariff.reportMeter, "ElectricityPurchased:Facility"))) {
                    ShowWarningError(state, format("{}{}=\"{}\" atypical meter", RoutineName, CurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state,
                                      format("The meter chosen \"{}\" is not typically used with the buyFromUtility option.", tariff.reportMeter));
                    ShowContinueError(state,
                                      "Usually the Electricity:Facility meter or the ElectricityPurchased:Facility is selected when the "
                                      "buyFromUtility option is used.");
                }
            }
        }

        // initialize gathering arrays
        for (int kMonth = 1; kMonth <= NumMonths; ++kMonth) {
            tariff.seasonForMonth(kMonth) = Season::Invalid;
            for (int lPeriod = 0; lPeriod < (int)Period::Num; ++lPeriod) {
                tariff.gatherEnergy(kMonth)[lPeriod] = 0.0;
                tariff.gatherDemand(kMonth)[lPeriod] = 0.0;
            }
        }

        // assume that the tariff is qualified
        tariff.isQualified = true;
        tariff.ptDisqualifier = 0;
        // assume that the tariff is not selected
        tariff.isSelected = false;
        tariff.totalAnnualCost = 0.0;
        // now create the Table Of Contents entries for an HTML file
        if (state.dataOutRptTab->displayTariffReport) {
            OutputReportTabular::AddTOCEntry(state, "Tariff Report", tariff.tariffName);
        }
        // associate the resource number with each tariff
        if (tariff.reportMeterIndx != -1) {
            tariff.resource = state.dataOutputProcessor->meters[tariff.reportMeterIndx]->resource;
        }
    }
}

void GetInputEconomicsQualify(EnergyPlusData &state, bool &ErrorsFound) // true if errors found during getting input objects.
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   May 2004

    //    Read the input file for "Economics:Qualify" objects.

    static constexpr std::string_view RoutineName("GetInputEconomicsQualify: ");
    static constexpr std::string_view routineName = "GetInputEconomicsQualify";

    int iInObj;    // loop index variable for reading in objects
    int NumAlphas; // Number of elements in the alpha array
    int NumNums;   // Number of elements in the numeric array
    int IOStat;    // IO Status when calling get input subroutine
    bool isNotNumeric;
    int jFld;

    auto &s_econ = state.dataEconTariff;
    auto &s_ipsc = state.dataIPShortCut;

    s_ipsc->cCurrentModuleObject = "UtilityCost:Qualify";
    s_econ->numQualify = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    s_econ->qualify.allocate(s_econ->numQualify);

    for (iInObj = 1; iInObj <= s_econ->numQualify; ++iInObj) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 iInObj,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        auto &qualify = s_econ->qualify(iInObj);

        // check to make sure none of the values are another economic object
        for (jFld = 1; jFld <= NumAlphas; ++jFld) {
            if (hasi(s_ipsc->cAlphaArgs(jFld), "UtilityCost:")) {
                ShowWarningError(state, format("{}{}=\"{}\".", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "... a field was found containing UtilityCost: which may indicate a missing comma.");
            }
        }
        // index of the tariff name in the tariff array
        qualify.tariffIndx = FindTariffIndex(state, s_ipsc->cAlphaArgs(2), s_ipsc->cAlphaArgs(1), ErrorsFound, s_ipsc->cCurrentModuleObject);
        warnIfNativeVarname(state, s_ipsc->cAlphaArgs(1), qualify.tariffIndx, ErrorsFound, s_ipsc->cCurrentModuleObject);
        qualify.namePt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(1), true, varIsAssigned, varNotYetDefined, ObjType::Qualify, iInObj, qualify.tariffIndx);
        // index of the variable in the variable array
        qualify.sourcePt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(3), true, varIsArgument, varNotYetDefined, ObjType::Invalid, 0, qualify.tariffIndx);
        // indicator if maximum test otherwise minimum
        if (s_ipsc->cAlphaArgs(4) == "MINIMUM") {
            qualify.isMaximum = false;
        } else if (s_ipsc->cAlphaArgs(4) == "MAXIMUM") {
            qualify.isMaximum = true;
        } else {
            ShowWarningInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4), "Maximum");
            qualify.isMaximum = true;
        }
        // value of the threshold
        qualify.thresholdVal = Util::ProcessNumber(s_ipsc->cAlphaArgs(5), isNotNumeric);
        qualify.thresholdPt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(5), isNotNumeric, varIsArgument, varNotYetDefined, ObjType::Invalid, 0, qualify.tariffIndx);
        // enumerated list of the kind of season
        if ((qualify.season = static_cast<Season>(getEnumValue(seasonNamesUC, s_ipsc->cAlphaArgs(6)))) == Season::Invalid) {
            ShowWarningInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6), "Annual");
            qualify.season = Season::Annual;
        }

        // indicator if consecutive months otherwise count
        if (s_ipsc->cAlphaArgs(7) == "COUNT") {
            qualify.isConsecutive = false;
        } else if (s_ipsc->cAlphaArgs(7) == "CONSECUTIVE") {
            qualify.isConsecutive = true;
        } else {
            ShowWarningInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5), "Consecutive");
            qualify.isConsecutive = true;
        }
        // number of months the test must be good for
        qualify.numberOfMonths = s_ipsc->rNumericArgs(1);
    }
}

void GetInputEconomicsChargeSimple(EnergyPlusData &state, bool &ErrorsFound) // true if errors found during getting input objects.
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   May 2004

    //    Read the input file for "Economics:Charge:Simple" objects.

    static constexpr std::string_view RoutineName("GetInputEconomicsChargeSimple: ");
    static constexpr std::string_view routineName = "GetInputEconomicsChargeSimple";
    int NumAlphas; // Number of elements in the alpha array
    int NumNums;   // Number of elements in the numeric array
    int IOStat;    // IO Status when calling get input subroutine
    bool isNotNumeric;

    auto &s_econ = state.dataEconTariff;
    auto &s_ipsc = state.dataIPShortCut;
    s_ipsc->cCurrentModuleObject = "UtilityCost:Charge:Simple";

    s_econ->numChargeSimple = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    s_econ->chargeSimple.allocate(s_econ->numChargeSimple);
    for (int iInObj = 1; iInObj <= s_econ->numChargeSimple; ++iInObj) {
        auto &chargeSimple = s_econ->chargeSimple(iInObj);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 iInObj,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        // check to make sure none of the values are another economic object
        for (int jFld = 1; jFld <= NumAlphas; ++jFld) {
            if (hasi(s_ipsc->cAlphaArgs(jFld), "UtilityCost:")) {
                ShowWarningError(state, format("{}{}=\"{}\".", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "... a field was found containing UtilityCost: which may indicate a missing comma.");
            }
        }
        // index of the tariff name in the tariff array
        chargeSimple.tariffIndx = FindTariffIndex(state, s_ipsc->cAlphaArgs(2), s_ipsc->cAlphaArgs(1), ErrorsFound, s_ipsc->cCurrentModuleObject);
        warnIfNativeVarname(state, s_ipsc->cAlphaArgs(1), chargeSimple.tariffIndx, ErrorsFound, s_ipsc->cCurrentModuleObject);
        chargeSimple.namePt = AssignVariablePt(
            state, s_ipsc->cAlphaArgs(1), true, varIsAssigned, varNotYetDefined, ObjType::ChargeSimple, iInObj, chargeSimple.tariffIndx);
        // index of the variable in the variable array
        chargeSimple.sourcePt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(3), true, varIsArgument, varNotYetDefined, ObjType::Invalid, 0, chargeSimple.tariffIndx);
        // enumerated list of the kind of season
        chargeSimple.season = static_cast<Season>(getEnumValue(seasonNamesUC, s_ipsc->cAlphaArgs(4)));
        if (chargeSimple.season == Season::Invalid) {
            ShowWarningInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4), "Annual");
            chargeSimple.season = Season::Annual;
        }

        // check to make sure a seasonal schedule is specified if the season is not annual
        if (chargeSimple.season != Season::Annual) {
            if (chargeSimple.tariffIndx != 0) {
                if (s_econ->tariff(chargeSimple.tariffIndx).seasonSched == nullptr) {
                    ShowWarningError(state, format("{}{}=\"{}\" invalid data", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("{}=\"{}\".", s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4)));
                    ShowContinueError(state,
                                      " a Season other than Annual is used but no Season Schedule Name is specified in the UtilityCost:Tariff.");
                }
            }
        }
        // index of the category in the variable array
        chargeSimple.categoryPt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(5), true, varIsAssigned, varNotYetDefined, ObjType::Category, iInObj, chargeSimple.tariffIndx);
        // cost per unit value or variable
        chargeSimple.costPerVal = Util::ProcessNumber(s_ipsc->cAlphaArgs(6), isNotNumeric);
        chargeSimple.costPerPt = AssignVariablePt(
            state, s_ipsc->cAlphaArgs(6), isNotNumeric, varIsArgument, varNotYetDefined, ObjType::Invalid, 0, chargeSimple.tariffIndx);
    }
}

void GetInputEconomicsChargeBlock(EnergyPlusData &state, bool &ErrorsFound) // true if errors found during getting input objects.
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   May 2004

    //    Read the input file for "Economics:Charge:Block" objects.

    static constexpr std::string_view RoutineName("GetInputEconomicsChargeBlock: ");
    static constexpr std::string_view routineName = "GetInputEconomicsChargeBlock";

    int NumAlphas; // Number of elements in the alpha array
    int NumNums;   // Number of elements in the numeric array
    int IOStat;    // IO Status when calling get input subroutine
    bool isNotNumeric;
    int alphaOffset;        // offset used in blocks for alpha array
    Real64 hugeNumber(0.0); // Autodesk Value not used but suppresses warning about HUGE_() call

    auto &s_econ = state.dataEconTariff;
    auto &s_ipsc = state.dataIPShortCut;
    s_ipsc->cCurrentModuleObject = "UtilityCost:Charge:Block";

    hugeNumber = HUGE_(hugeNumber);
    s_econ->numChargeBlock = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    s_econ->chargeBlock.allocate(s_econ->numChargeBlock);
    for (int iInObj = 1; iInObj <= s_econ->numChargeBlock; ++iInObj) {
        auto &chargeBlock = s_econ->chargeBlock(iInObj);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 iInObj,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        // check to make sure none of the values are another economic object
        for (int jFld = 1; jFld <= NumAlphas; ++jFld) {
            if (hasi(s_ipsc->cAlphaArgs(jFld), "UtilityCost:")) {
                ShowWarningError(state, format("{}{}=\"{}\".", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "... a field was found containing UtilityCost: which may indicate a missing comma.");
            }
        }
        // index of the tariff name in the tariff array
        chargeBlock.tariffIndx = FindTariffIndex(state, s_ipsc->cAlphaArgs(2), s_ipsc->cAlphaArgs(1), ErrorsFound, s_ipsc->cCurrentModuleObject);
        warnIfNativeVarname(state, s_ipsc->cAlphaArgs(1), chargeBlock.tariffIndx, ErrorsFound, s_ipsc->cCurrentModuleObject);
        chargeBlock.namePt = AssignVariablePt(
            state, s_ipsc->cAlphaArgs(1), true, varIsAssigned, varNotYetDefined, ObjType::ChargeBlock, iInObj, chargeBlock.tariffIndx);
        // index of the variable in the variable array
        chargeBlock.sourcePt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(3), true, varIsArgument, varNotYetDefined, ObjType::Invalid, 0, chargeBlock.tariffIndx);
        // enumerated list of the kind of season
        chargeBlock.season = static_cast<Season>(getEnumValue(seasonNamesUC, s_ipsc->cAlphaArgs(4)));
        if (chargeBlock.season == Season::Invalid) {
            ShowWarningInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4), "Annual");
            chargeBlock.season = Season::Annual;
        }

        // check to make sure a seasonal schedule is specified if the season is not annual
        if (chargeBlock.season != Season::Annual) {
            if (chargeBlock.tariffIndx != 0) {
                if (s_econ->tariff(chargeBlock.tariffIndx).seasonSched == nullptr) {
                    ShowWarningError(state, format("{}{}=\"{}\" invalid data", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("{}=\"{}\".", s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4)));
                    ShowContinueError(state,
                                      " a Season other than Annual is used but no Season Schedule Name is specified in the UtilityCost:Tariff.");
                }
            }
        }
        // index of the category in the variable array
        chargeBlock.categoryPt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(5), true, varIsAssigned, varNotYetDefined, ObjType::Category, iInObj, chargeBlock.tariffIndx);
        // index of the remaining into variable in the variable array
        chargeBlock.remainingPt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(6), true, varIsAssigned, varNotYetDefined, ObjType::Category, iInObj, chargeBlock.tariffIndx);
        // block size multiplier
        if (len(s_ipsc->cAlphaArgs(7)) == 0) { // if blank
            chargeBlock.blkSzMultVal = 1.0;    // default is 1 if left blank
            chargeBlock.blkSzMultPt = 0;
        } else {
            chargeBlock.blkSzMultVal = Util::ProcessNumber(s_ipsc->cAlphaArgs(7), isNotNumeric);
            chargeBlock.blkSzMultPt = AssignVariablePt(
                state, s_ipsc->cAlphaArgs(7), isNotNumeric, varIsArgument, varNotYetDefined, ObjType::Invalid, 0, chargeBlock.tariffIndx);
        }
        // number of blocks used
        chargeBlock.numBlk = (NumAlphas - 7) / 2;
        for (int jBlk = 1; jBlk <= chargeBlock.numBlk; ++jBlk) {
            alphaOffset = 7 + (jBlk - 1) * 2;
            // catch the "remaining" code word for the block size
            if (Util::SameString(s_ipsc->cAlphaArgs(alphaOffset + 1), "REMAINING")) {
                chargeBlock.blkSzVal(jBlk) = hugeNumber / 1000000; // using small portion of largest possible value to prevent overflow
                chargeBlock.blkSzPt(jBlk) = 0;
            } else {
                // array of block size
                chargeBlock.blkSzVal(jBlk) = Util::ProcessNumber(s_ipsc->cAlphaArgs(alphaOffset + 1), isNotNumeric);

                chargeBlock.blkSzPt(jBlk) = AssignVariablePt(state,
                                                             s_ipsc->cAlphaArgs(alphaOffset + 1),
                                                             isNotNumeric,
                                                             varIsArgument,
                                                             varNotYetDefined,
                                                             ObjType::Invalid,
                                                             0,
                                                             chargeBlock.tariffIndx);
            }
            // array of block cost
            chargeBlock.blkCostVal(jBlk) = Util::ProcessNumber(s_ipsc->cAlphaArgs(alphaOffset + 2), isNotNumeric);
            chargeBlock.blkCostPt(jBlk) = AssignVariablePt(state,
                                                           s_ipsc->cAlphaArgs(alphaOffset + 2),
                                                           isNotNumeric,
                                                           varIsArgument,
                                                           varNotYetDefined,
                                                           ObjType::Invalid,
                                                           0,
                                                           chargeBlock.tariffIndx);
        }
    }
}

void GetInputEconomicsRatchet(EnergyPlusData &state, bool &ErrorsFound) // true if errors found during getting input objects.
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   May 2004

    //    Read the input file for "Economics:Ratchet" objects.

    static constexpr std::string_view RoutineName("GetInputEconomicsRatchet: ");
    static constexpr std::string_view routineName = "GetInputEconomicsRatchet";

    int NumAlphas; // Number of elements in the alpha array
    int NumNums;   // Number of elements in the numeric array
    int IOStat;    // IO Status when calling get input subroutine
    bool isNotNumeric;

    auto &s_econ = state.dataEconTariff;
    auto &s_ipsc = state.dataIPShortCut;
    s_ipsc->cCurrentModuleObject = "UtilityCost:Ratchet";

    s_econ->numRatchet = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    s_econ->ratchet.allocate(s_econ->numRatchet);
    for (int iInObj = 1; iInObj <= s_econ->numRatchet; ++iInObj) {
        auto &ratchet = s_econ->ratchet(iInObj);

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 iInObj,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        // check to make sure none of the values are another economic object
        for (int jFld = 1; jFld <= NumAlphas; ++jFld) {
            if (hasi(s_ipsc->cAlphaArgs(jFld), "UtilityCost:")) {
                ShowWarningError(state, format("{}{}=\"{}\".", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "... a field was found containing UtilityCost: which may indicate a missing comma.");
            }
        }
        // index of the tariff name in the tariff array
        ratchet.tariffIndx = FindTariffIndex(state, s_ipsc->cAlphaArgs(2), s_ipsc->cAlphaArgs(1), ErrorsFound, s_ipsc->cCurrentModuleObject);
        warnIfNativeVarname(state, s_ipsc->cAlphaArgs(1), ratchet.tariffIndx, ErrorsFound, s_ipsc->cCurrentModuleObject);
        ratchet.namePt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(1), true, varIsAssigned, varNotYetDefined, ObjType::Ratchet, iInObj, ratchet.tariffIndx);
        // index of the variable in the variable array
        ratchet.baselinePt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(3), true, varIsArgument, varNotYetDefined, ObjType::Ratchet, iInObj, ratchet.tariffIndx);
        // index of the variable in the variable array
        ratchet.adjustmentPt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(4), true, varIsArgument, varNotYetDefined, ObjType::Ratchet, iInObj, ratchet.tariffIndx);
        // seasons to and from
        ratchet.seasonFrom = static_cast<Season>(getEnumValue(seasonNamesUC, s_ipsc->cAlphaArgs(5)));
        if (ratchet.seasonFrom == Season::Invalid) {
            ShowWarningInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5), "Annual");
            ratchet.seasonFrom = Season::Annual;
        }
        ratchet.seasonTo = static_cast<Season>(getEnumValue(seasonNamesUC, s_ipsc->cAlphaArgs(6)));
        if (ratchet.seasonTo == Season::Invalid) {
            ShowWarningInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(56), "Annual");
            ratchet.seasonTo = Season::Annual;
        }

        // ratchet multiplier
        ratchet.multiplierVal = Util::ProcessNumber(s_ipsc->cAlphaArgs(7), isNotNumeric);
        ratchet.multiplierPt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(7), isNotNumeric, varIsArgument, varNotYetDefined, ObjType::Invalid, 0, ratchet.tariffIndx);
        // ratchet offset
        ratchet.offsetVal = Util::ProcessNumber(s_ipsc->cAlphaArgs(8), isNotNumeric);
        ratchet.offsetPt =
            AssignVariablePt(state, s_ipsc->cAlphaArgs(8), isNotNumeric, varIsArgument, varNotYetDefined, ObjType::Invalid, 0, ratchet.tariffIndx);
    }
}

void GetInputEconomicsVariable(EnergyPlusData &state, bool &ErrorsFound) // true if errors found during getting input objects.
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   May 2004

    //    Read the input file for "Economics:Variable" objects.

    static constexpr std::string_view RoutineName("GetInputEconomicsVariable: ");
    static constexpr std::string_view routineName = "GetInputEconomicsVariable";

    int NumAlphas; // Number of elements in the alpha array
    int NumNums;   // Number of elements in the numeric array
    int IOStat;    // IO Status when calling get input subroutine

    auto &s_econ = state.dataEconTariff;
    auto &s_ipsc = state.dataIPShortCut;

    s_ipsc->cCurrentModuleObject = "UtilityCost:Variable";
    int numEconVarObj = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    for (int iInObj = 1; iInObj <= numEconVarObj; ++iInObj) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 iInObj,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        // check to make sure none of the values are another economic object
        for (int jFld = 1; jFld <= NumAlphas; ++jFld) {
            if (hasi(s_ipsc->cAlphaArgs(jFld), "UtilityCost:")) {
                ShowWarningError(state, format("{}{}=\"{}\".", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "... a field was found containing UtilityCost: which may indicate a missing comma.");
            }
        }
        int tariffPt = FindTariffIndex(state, s_ipsc->cAlphaArgs(2), s_ipsc->cAlphaArgs(1), ErrorsFound, s_ipsc->cCurrentModuleObject);
        int variablePt = AssignVariablePt(state, s_ipsc->cAlphaArgs(1), true, varIsArgument, varUserDefined, ObjType::Variable, iInObj, tariffPt);
        warnIfNativeVarname(state, s_ipsc->cAlphaArgs(1), tariffPt, ErrorsFound, s_ipsc->cCurrentModuleObject);
        auto &econVar = s_econ->econVar(variablePt);

        // validate the kind of variable - not used internally except for validation
        econVar.varUnitType = static_cast<VarUnitType>(getEnumValue(varUnitTypeNamesUC, s_ipsc->cAlphaArgs(3)));
        if (econVar.varUnitType == VarUnitType::Invalid) {
            ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
            ErrorsFound = true;
            econVar.varUnitType = VarUnitType::Dimensionless;
        }

        // move number inputs into econVar
        for (int jVal = 1; jVal <= NumNums; ++jVal) {
            econVar.values(jVal) = s_ipsc->rNumericArgs(jVal);
        }
        // fill the rest of the array with the last value entered
        if (NumNums < NumMonths) {
            for (int jVal = NumNums + 1; jVal <= NumMonths; ++jVal) {
                econVar.values(jVal) = s_ipsc->rNumericArgs(NumNums);
            }
        }
    }
}

void GetInputEconomicsComputation(EnergyPlusData &state, bool &ErrorsFound) // true if errors found during getting input objects.
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   May 2004

    //    Read the input file for "Economics:Computation" objects.
    //    This object is only used for very complex rates.

    static constexpr std::string_view RoutineName("GetInputEconomicsComputation: ");

    int NumAlphas; // Number of elements in the alpha array
    int NumNums;   // Number of elements in the numeric array
    int IOStat;    // IO Status when calling get input subroutine

    auto &s_econ = state.dataEconTariff;
    auto &s_ipsc = state.dataIPShortCut;

    s_ipsc->cCurrentModuleObject = "UtilityCost:Computation";
    s_econ->numComputation = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    s_econ->computation.allocate(s_econ->numTariff); // not the number of Computations but the number of tariffs
    // set default values for computation
    for (auto &e : s_econ->computation) {
        e.computeName.clear();
        e.firstStep = 0;
        e.lastStep = -1;
        e.isUserDef = false;
    }
    for (int iInObj = 1; iInObj <= s_econ->numComputation; ++iInObj) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 iInObj,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);
        // check to make sure none of the values are another economic object
        for (int jFld = 1; jFld <= NumAlphas; ++jFld) {
            if (hasi(s_ipsc->cAlphaArgs(jFld), "UtilityCost:")) {
                ShowWarningError(state, format("{}{}=\"{}\".", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "... a field was found containing UtilityCost: which may indicate a missing comma.");
            }
        }
        int tariffPt = FindTariffIndex(state, s_ipsc->cAlphaArgs(2), s_ipsc->cAlphaArgs(1), ErrorsFound, s_ipsc->cCurrentModuleObject);
        warnIfNativeVarname(state, s_ipsc->cAlphaArgs(1), tariffPt, ErrorsFound, s_ipsc->cCurrentModuleObject);
        // tariff and computation share the same index, the tariff index
        // so all references are to the tariffPt
        auto &computation = s_econ->computation(tariffPt);

        if (isWithinRange(state, tariffPt, 1, s_econ->numTariff)) {
            computation.computeName = s_ipsc->cAlphaArgs(1);
            computation.firstStep = s_econ->numSteps + 1;
            for (int jLine = 3; jLine <= NumAlphas; ++jLine) {
                parseComputeLine(state, s_ipsc->cAlphaArgs(jLine), tariffPt);
            }
            computation.lastStep = s_econ->numSteps;
            // check to make sure that some steps were defined
            if (computation.firstStep >= computation.lastStep) {
                computation.firstStep = 0;
                computation.lastStep = -1;
                computation.isUserDef = false;
                ShowSevereError(state, format("{}{}=\"{}\" invalid data.", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "... No lines in the computation can be interpreted ");
                ErrorsFound = true;
            } else {
                computation.isUserDef = true;
            }
        } else {
            ShowSevereError(state, format("{}{}=\"{}\" invalid data.", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("... not found {}=\"{}\".", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
            ErrorsFound = true;
        }
    }
}

void GetInputEconomicsCurrencyType(EnergyPlusData &state, bool &ErrorsFound) // true if errors found during getting input objects.
{
    //       AUTHOR         Jason Glazer
    //       DATE WRITTEN   August 2008

    //   Sets the type of currency (U.S. Dollar, Euro, Yen, etc.. )
    //   This is a "unique" object.

    static constexpr std::string_view RoutineName("GetInputEconomicsCurrencyType: ");

    int NumCurrencyType;
    int NumAlphas; // Number of elements in the alpha array
    int NumNums;   // Number of elements in the numeric array
    int IOStat;    // IO Status when calling get input subroutine
    int i;

    auto &s_ipsc = state.dataIPShortCut;
    s_ipsc->cCurrentModuleObject = "CurrencyType";

    initializeMonetaryUnit(state);
    NumCurrencyType = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    state.dataCostEstimateManager->selectedMonetaryUnit = 0; // invalid
    if (NumCurrencyType == 0) {
        state.dataCostEstimateManager->selectedMonetaryUnit = 1; // USD - U.S. Dollar
    } else if (NumCurrencyType == 1) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 1,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);
        // Monetary Unit
        for (i = 1; i <= (int)state.dataCostEstimateManager->monetaryUnit.size(); ++i) {
            if (Util::SameString(s_ipsc->cAlphaArgs(1), state.dataCostEstimateManager->monetaryUnit(i).code)) {
                state.dataCostEstimateManager->selectedMonetaryUnit = i;
                break;
            }
        }
        if (state.dataCostEstimateManager->selectedMonetaryUnit == 0) {
            ShowSevereError(state, format("{}{}=\"{}\" invalid data.", RoutineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("... invalid {}.", s_ipsc->cAlphaFieldNames(1)));
            ErrorsFound = true;
        }
    } else if (NumCurrencyType > 1) {
        ShowWarningError(state,
                         format("{}{} Only one instance of this object is allowed. USD will be used.", RoutineName, s_ipsc->cCurrentModuleObject));
        state.dataCostEstimateManager->selectedMonetaryUnit = 1; // USD - U.S. Dollar
    }
}

void parseComputeLine(EnergyPlusData &state, std::string const &lineOfCompute, int const fromTariff)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   June 2004

    //   Converts a single line in the ECONOMICS:COMPUTE
    //   command into tokens for computation

    //   Scan the line from the end of the line to the front of the
    //   line and search for operators and variables. All items
    //   are put into the step array.

    auto &s_econ = state.dataEconTariff;

    std::string word;

    size_t endOfWord = len(lineOfCompute) - 1;
    while (endOfWord != std::string::npos) {
        // get a single word (text string delimited by spaces)
        GetLastWord(lineOfCompute, endOfWord, word);
        // first see if word is an operator
        Op op = static_cast<Op>(getEnumValue(opNamesUC, word));
        if (op == Op::Invalid) {
            static_cast<Op>(getEnumValue(opNames2UC, word));
        }

        // if not an operator then look for
        if (op != Op::Invalid) {
            incrementSteps(state);
            s_econ->steps(s_econ->numSteps).type = StepType::Op;
            s_econ->steps(s_econ->numSteps).op = op;

        } else {
            int varNum;
            // see if argument or assignment (assignment will be first string on line)
            if (endOfWord != std::string::npos) {
                varNum = AssignVariablePt(state, word, true, varIsArgument, varNotYetDefined, ObjType::Invalid, 0, fromTariff);
            } else {
                varNum = AssignVariablePt(state, word, true, varIsAssigned, varNotYetDefined, ObjType::AssignCompute, 0, fromTariff);
            }

            // if a token is found then put it into step array
            if (varNum == 0) {
                ShowWarningError(state, format("In UtilityCost:Computation line: {}", lineOfCompute));
                ShowContinueError(state, format("  Do not recognize: {} Will skip.", word));
            } else {
                incrementSteps(state);
                s_econ->steps(s_econ->numSteps).type = StepType::Var;
                s_econ->steps(s_econ->numSteps).varNum = varNum;
            }
        }
    }

    incrementSteps(state);
    s_econ->steps(s_econ->numSteps).type = StepType::EOL; // at the end of the line show a zero to clear the stack
}

void GetLastWord(std::string const &lineOfText, std::string::size_type &endOfScan, std::string &aWord)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   June 2004

    //   Returns the last substring of the line of text to the
    //   left of the endOfSubStrg pointer. A substring is
    //   delimited by spaces.  Quotes are not significant
    //   (they are treated just like any other non-space character)

    //   Scan the string from the end.

    size_t curEndOfScan = endOfScan;
    if (curEndOfScan != std::string::npos) {
        if (curEndOfScan >= len(lineOfText)) {
            curEndOfScan = len(lineOfText) - 1;
        }
        // check if currently on a space or not
        bool isInWord;
        size_t beginOfWord;
        size_t endOfWord;
        if (lineOfText[curEndOfScan] == ' ') {
            isInWord = false;
            beginOfWord = 0;
            endOfWord = 0;
        } else {
            isInWord = true;
            beginOfWord = curEndOfScan;
            endOfWord = curEndOfScan;
        }
        // scan backwards from
        bool isSpace;
        for (size_t iString = curEndOfScan; iString <= curEndOfScan; --iString) { // Unsigned will wrap to npos after 0
            if (lineOfText[iString] == ' ') {
                isSpace = true;
            } else {
                isSpace = false;
            }
            // all logical conditions of isSpace and isInWord
            if (isSpace) {
                if (isInWord) {
                    // found the space in front of the word
                    break;
                } else {
                    // still have not found the back of the word
                    // do nothing
                }
            } else {
                if (isInWord) {
                    // still have not found the space in front of the word
                    beginOfWord = iString;
                } else {
                    // found the last character of the word
                    endOfWord = iString;
                    beginOfWord = iString;
                    isInWord = true;
                }
            }
        }
        aWord = lineOfText.substr(beginOfWord, endOfWord - beginOfWord + 1);
        endOfScan = beginOfWord - 1;
    } else {
        endOfScan = std::string::npos;
        aWord = "";
    }
}

void initializeMonetaryUnit(EnergyPlusData &state)
{
    //       AUTHOR         Jason Glazer
    //       DATE WRITTEN   August 2008

    //   Sets the type of monetary unit array.

    //   Uses get input structure similar to other objects
    //   The monetaryUnitSymbols.xls spreadsheet helps create the code for this routine

    //   www.xe.com/symbols.php

    int numMonetaryUnit = 111;
    state.dataCostEstimateManager->monetaryUnit.allocate(numMonetaryUnit);
    state.dataCostEstimateManager->monetaryUnit(1).code = "USD";
    state.dataCostEstimateManager->monetaryUnit(2).code = "AFN";
    state.dataCostEstimateManager->monetaryUnit(3).code = "ALL";
    state.dataCostEstimateManager->monetaryUnit(4).code = "ANG";
    state.dataCostEstimateManager->monetaryUnit(5).code = "ARS";
    state.dataCostEstimateManager->monetaryUnit(6).code = "AUD";
    state.dataCostEstimateManager->monetaryUnit(7).code = "AWG";
    state.dataCostEstimateManager->monetaryUnit(8).code = "AZN";
    state.dataCostEstimateManager->monetaryUnit(9).code = "BAM";
    state.dataCostEstimateManager->monetaryUnit(10).code = "BBD";
    state.dataCostEstimateManager->monetaryUnit(11).code = "BGN";
    state.dataCostEstimateManager->monetaryUnit(12).code = "BMD";
    state.dataCostEstimateManager->monetaryUnit(13).code = "BND";
    state.dataCostEstimateManager->monetaryUnit(14).code = "BOB";
    state.dataCostEstimateManager->monetaryUnit(15).code = "BRL";
    state.dataCostEstimateManager->monetaryUnit(16).code = "BSD";
    state.dataCostEstimateManager->monetaryUnit(17).code = "BWP";
    state.dataCostEstimateManager->monetaryUnit(18).code = "BYR";
    state.dataCostEstimateManager->monetaryUnit(19).code = "BZD";
    state.dataCostEstimateManager->monetaryUnit(20).code = "CAD";
    state.dataCostEstimateManager->monetaryUnit(21).code = "CHF";
    state.dataCostEstimateManager->monetaryUnit(22).code = "CLP";
    state.dataCostEstimateManager->monetaryUnit(23).code = "CNY";
    state.dataCostEstimateManager->monetaryUnit(24).code = "COP";
    state.dataCostEstimateManager->monetaryUnit(25).code = "CRC";
    state.dataCostEstimateManager->monetaryUnit(26).code = "CUP";
    state.dataCostEstimateManager->monetaryUnit(27).code = "CZK";
    state.dataCostEstimateManager->monetaryUnit(28).code = "DKK";
    state.dataCostEstimateManager->monetaryUnit(29).code = "DOP";
    state.dataCostEstimateManager->monetaryUnit(30).code = "EEK";
    state.dataCostEstimateManager->monetaryUnit(31).code = "EGP";
    state.dataCostEstimateManager->monetaryUnit(32).code = "EUR";
    state.dataCostEstimateManager->monetaryUnit(33).code = "FJD";
    state.dataCostEstimateManager->monetaryUnit(34).code = "GBP";
    state.dataCostEstimateManager->monetaryUnit(35).code = "GHC";
    state.dataCostEstimateManager->monetaryUnit(36).code = "GIP";
    state.dataCostEstimateManager->monetaryUnit(37).code = "GTQ";
    state.dataCostEstimateManager->monetaryUnit(38).code = "GYD";
    state.dataCostEstimateManager->monetaryUnit(39).code = "HKD";
    state.dataCostEstimateManager->monetaryUnit(40).code = "HNL";
    state.dataCostEstimateManager->monetaryUnit(41).code = "HRK";
    state.dataCostEstimateManager->monetaryUnit(42).code = "HUF";
    state.dataCostEstimateManager->monetaryUnit(43).code = "IDR";
    state.dataCostEstimateManager->monetaryUnit(44).code = "ILS";
    state.dataCostEstimateManager->monetaryUnit(45).code = "IMP";
    state.dataCostEstimateManager->monetaryUnit(46).code = "INR";
    state.dataCostEstimateManager->monetaryUnit(47).code = "IRR";
    state.dataCostEstimateManager->monetaryUnit(48).code = "ISK";
    state.dataCostEstimateManager->monetaryUnit(49).code = "JEP";
    state.dataCostEstimateManager->monetaryUnit(50).code = "JMD";
    state.dataCostEstimateManager->monetaryUnit(51).code = "JPY";
    state.dataCostEstimateManager->monetaryUnit(52).code = "KGS";
    state.dataCostEstimateManager->monetaryUnit(53).code = "KHR";
    state.dataCostEstimateManager->monetaryUnit(54).code = "KPW";
    state.dataCostEstimateManager->monetaryUnit(55).code = "KRW";
    state.dataCostEstimateManager->monetaryUnit(56).code = "KYD";
    state.dataCostEstimateManager->monetaryUnit(57).code = "KZT";
    state.dataCostEstimateManager->monetaryUnit(58).code = "LAK";
    state.dataCostEstimateManager->monetaryUnit(59).code = "LBP";
    state.dataCostEstimateManager->monetaryUnit(60).code = "LKR";
    state.dataCostEstimateManager->monetaryUnit(61).code = "LRD";
    state.dataCostEstimateManager->monetaryUnit(62).code = "LTL";
    state.dataCostEstimateManager->monetaryUnit(63).code = "LVL";
    state.dataCostEstimateManager->monetaryUnit(64).code = "MKD";
    state.dataCostEstimateManager->monetaryUnit(65).code = "MNT";
    state.dataCostEstimateManager->monetaryUnit(66).code = "MUR";
    state.dataCostEstimateManager->monetaryUnit(67).code = "MXN";
    state.dataCostEstimateManager->monetaryUnit(68).code = "MYR";
    state.dataCostEstimateManager->monetaryUnit(69).code = "MZN";
    state.dataCostEstimateManager->monetaryUnit(70).code = "NAD";
    state.dataCostEstimateManager->monetaryUnit(71).code = "NGN";
    state.dataCostEstimateManager->monetaryUnit(72).code = "NIO";
    state.dataCostEstimateManager->monetaryUnit(73).code = "NOK";
    state.dataCostEstimateManager->monetaryUnit(74).code = "NPR";
    state.dataCostEstimateManager->monetaryUnit(75).code = "NZD";
    state.dataCostEstimateManager->monetaryUnit(76).code = "OMR";
    state.dataCostEstimateManager->monetaryUnit(77).code = "PAB";
    state.dataCostEstimateManager->monetaryUnit(78).code = "PEN";
    state.dataCostEstimateManager->monetaryUnit(79).code = "PHP";
    state.dataCostEstimateManager->monetaryUnit(80).code = "PKR";
    state.dataCostEstimateManager->monetaryUnit(81).code = "PLN";
    state.dataCostEstimateManager->monetaryUnit(82).code = "PYG";
    state.dataCostEstimateManager->monetaryUnit(83).code = "QAR";
    state.dataCostEstimateManager->monetaryUnit(84).code = "RON";
    state.dataCostEstimateManager->monetaryUnit(85).code = "RSD";
    state.dataCostEstimateManager->monetaryUnit(86).code = "RUB";
    state.dataCostEstimateManager->monetaryUnit(87).code = "SAR";
    state.dataCostEstimateManager->monetaryUnit(88).code = "SBD";
    state.dataCostEstimateManager->monetaryUnit(89).code = "SCR";
    state.dataCostEstimateManager->monetaryUnit(90).code = "SEK";
    state.dataCostEstimateManager->monetaryUnit(91).code = "SGD";
    state.dataCostEstimateManager->monetaryUnit(92).code = "SHP";
    state.dataCostEstimateManager->monetaryUnit(93).code = "SOS";
    state.dataCostEstimateManager->monetaryUnit(94).code = "SRD";
    state.dataCostEstimateManager->monetaryUnit(95).code = "SVC";
    state.dataCostEstimateManager->monetaryUnit(96).code = "SYP";
    state.dataCostEstimateManager->monetaryUnit(97).code = "THB";
    state.dataCostEstimateManager->monetaryUnit(98).code = "TRL";
    state.dataCostEstimateManager->monetaryUnit(99).code = "TRY";
    state.dataCostEstimateManager->monetaryUnit(100).code = "TTD";
    state.dataCostEstimateManager->monetaryUnit(101).code = "TVD";
    state.dataCostEstimateManager->monetaryUnit(102).code = "TWD";
    state.dataCostEstimateManager->monetaryUnit(103).code = "UAH";
    state.dataCostEstimateManager->monetaryUnit(104).code = "UYU";
    state.dataCostEstimateManager->monetaryUnit(105).code = "UZS";
    state.dataCostEstimateManager->monetaryUnit(106).code = "VEF";
    state.dataCostEstimateManager->monetaryUnit(107).code = "VND";
    state.dataCostEstimateManager->monetaryUnit(108).code = "XCD";
    state.dataCostEstimateManager->monetaryUnit(109).code = "YER";
    state.dataCostEstimateManager->monetaryUnit(110).code = "ZAR";
    state.dataCostEstimateManager->monetaryUnit(111).code = "ZWD";

    state.dataCostEstimateManager->monetaryUnit(1).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(2).txt = "AFN";
    state.dataCostEstimateManager->monetaryUnit(3).txt = "Lek";
    state.dataCostEstimateManager->monetaryUnit(4).txt = "ANG";
    state.dataCostEstimateManager->monetaryUnit(5).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(6).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(7).txt = "AWG";
    state.dataCostEstimateManager->monetaryUnit(8).txt = "AZN";
    state.dataCostEstimateManager->monetaryUnit(9).txt = "KM";
    state.dataCostEstimateManager->monetaryUnit(10).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(11).txt = "BGN";
    state.dataCostEstimateManager->monetaryUnit(12).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(13).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(14).txt = "$b";
    state.dataCostEstimateManager->monetaryUnit(15).txt = "R$";
    state.dataCostEstimateManager->monetaryUnit(16).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(17).txt = "P";
    state.dataCostEstimateManager->monetaryUnit(18).txt = "p.";
    state.dataCostEstimateManager->monetaryUnit(19).txt = "BZ$";
    state.dataCostEstimateManager->monetaryUnit(20).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(21).txt = "CHF";
    state.dataCostEstimateManager->monetaryUnit(22).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(23).txt = "CNY";
    state.dataCostEstimateManager->monetaryUnit(24).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(25).txt = "CRC";
    state.dataCostEstimateManager->monetaryUnit(26).txt = "CUP";
    state.dataCostEstimateManager->monetaryUnit(27).txt = "CZK";
    state.dataCostEstimateManager->monetaryUnit(28).txt = "kr";
    state.dataCostEstimateManager->monetaryUnit(29).txt = "RD$";
    state.dataCostEstimateManager->monetaryUnit(30).txt = "kr";
    state.dataCostEstimateManager->monetaryUnit(31).txt = "£";
    state.dataCostEstimateManager->monetaryUnit(32).txt = "EUR";
    state.dataCostEstimateManager->monetaryUnit(33).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(34).txt = "£";
    state.dataCostEstimateManager->monetaryUnit(35).txt = "¢";
    state.dataCostEstimateManager->monetaryUnit(36).txt = "£";
    state.dataCostEstimateManager->monetaryUnit(37).txt = "Q";
    state.dataCostEstimateManager->monetaryUnit(38).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(39).txt = "HK$";
    state.dataCostEstimateManager->monetaryUnit(40).txt = "L";
    state.dataCostEstimateManager->monetaryUnit(41).txt = "kn";
    state.dataCostEstimateManager->monetaryUnit(42).txt = "Ft";
    state.dataCostEstimateManager->monetaryUnit(43).txt = "Rp";
    state.dataCostEstimateManager->monetaryUnit(44).txt = "ILS";
    state.dataCostEstimateManager->monetaryUnit(45).txt = "£";
    state.dataCostEstimateManager->monetaryUnit(46).txt = "INR";
    state.dataCostEstimateManager->monetaryUnit(47).txt = "IRR";
    state.dataCostEstimateManager->monetaryUnit(48).txt = "kr";
    state.dataCostEstimateManager->monetaryUnit(49).txt = "£";
    state.dataCostEstimateManager->monetaryUnit(50).txt = "J$";
    state.dataCostEstimateManager->monetaryUnit(51).txt = "¥";
    state.dataCostEstimateManager->monetaryUnit(52).txt = "KGS";
    state.dataCostEstimateManager->monetaryUnit(53).txt = "KHR";
    state.dataCostEstimateManager->monetaryUnit(54).txt = "KPW";
    state.dataCostEstimateManager->monetaryUnit(55).txt = "KRW";
    state.dataCostEstimateManager->monetaryUnit(56).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(57).txt = "KZT";
    state.dataCostEstimateManager->monetaryUnit(58).txt = "LAK";
    state.dataCostEstimateManager->monetaryUnit(59).txt = "£";
    state.dataCostEstimateManager->monetaryUnit(60).txt = "LKR";
    state.dataCostEstimateManager->monetaryUnit(61).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(62).txt = "Lt";
    state.dataCostEstimateManager->monetaryUnit(63).txt = "Ls";
    state.dataCostEstimateManager->monetaryUnit(64).txt = "MKD";
    state.dataCostEstimateManager->monetaryUnit(65).txt = "MNT";
    state.dataCostEstimateManager->monetaryUnit(66).txt = "MUR";
    state.dataCostEstimateManager->monetaryUnit(67).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(68).txt = "RM";
    state.dataCostEstimateManager->monetaryUnit(69).txt = "MT";
    state.dataCostEstimateManager->monetaryUnit(70).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(71).txt = "NGN";
    state.dataCostEstimateManager->monetaryUnit(72).txt = "C$";
    state.dataCostEstimateManager->monetaryUnit(73).txt = "kr";
    state.dataCostEstimateManager->monetaryUnit(74).txt = "NPR";
    state.dataCostEstimateManager->monetaryUnit(75).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(76).txt = "OMR";
    state.dataCostEstimateManager->monetaryUnit(77).txt = "B/.";
    state.dataCostEstimateManager->monetaryUnit(78).txt = "S/.";
    state.dataCostEstimateManager->monetaryUnit(79).txt = "Php";
    state.dataCostEstimateManager->monetaryUnit(80).txt = "PKR";
    state.dataCostEstimateManager->monetaryUnit(81).txt = "PLN";
    state.dataCostEstimateManager->monetaryUnit(82).txt = "Gs";
    state.dataCostEstimateManager->monetaryUnit(83).txt = "QAR";
    state.dataCostEstimateManager->monetaryUnit(84).txt = "lei";
    state.dataCostEstimateManager->monetaryUnit(85).txt = "RSD";
    state.dataCostEstimateManager->monetaryUnit(86).txt = "RUB";
    state.dataCostEstimateManager->monetaryUnit(87).txt = "SAR";
    state.dataCostEstimateManager->monetaryUnit(88).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(89).txt = "SCR";
    state.dataCostEstimateManager->monetaryUnit(90).txt = "kr";
    state.dataCostEstimateManager->monetaryUnit(91).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(92).txt = "£";
    state.dataCostEstimateManager->monetaryUnit(93).txt = "S";
    state.dataCostEstimateManager->monetaryUnit(94).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(95).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(96).txt = "£";
    state.dataCostEstimateManager->monetaryUnit(97).txt = "THB";
    state.dataCostEstimateManager->monetaryUnit(98).txt = "TRL";
    state.dataCostEstimateManager->monetaryUnit(99).txt = "YTL";
    state.dataCostEstimateManager->monetaryUnit(100).txt = "TT$";
    state.dataCostEstimateManager->monetaryUnit(101).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(102).txt = "NT$";
    state.dataCostEstimateManager->monetaryUnit(103).txt = "UAH";
    state.dataCostEstimateManager->monetaryUnit(104).txt = "$U";
    state.dataCostEstimateManager->monetaryUnit(105).txt = "UZS";
    state.dataCostEstimateManager->monetaryUnit(106).txt = "Bs";
    state.dataCostEstimateManager->monetaryUnit(107).txt = "VND";
    state.dataCostEstimateManager->monetaryUnit(108).txt = "$";
    state.dataCostEstimateManager->monetaryUnit(109).txt = "YER";
    state.dataCostEstimateManager->monetaryUnit(110).txt = "R";
    state.dataCostEstimateManager->monetaryUnit(111).txt = "Z$";

    state.dataCostEstimateManager->monetaryUnit(1).html = "$";
    state.dataCostEstimateManager->monetaryUnit(2).html = "&#x060b;";
    state.dataCostEstimateManager->monetaryUnit(3).html = "Lek";
    state.dataCostEstimateManager->monetaryUnit(4).html = "&#x0192;";
    state.dataCostEstimateManager->monetaryUnit(5).html = "$";
    state.dataCostEstimateManager->monetaryUnit(6).html = "$";
    state.dataCostEstimateManager->monetaryUnit(7).html = "&#x0192;";
    state.dataCostEstimateManager->monetaryUnit(8).html = "&#x043c;&#x0430;&#x043d;";
    state.dataCostEstimateManager->monetaryUnit(9).html = "KM";
    state.dataCostEstimateManager->monetaryUnit(10).html = "$";
    state.dataCostEstimateManager->monetaryUnit(11).html = "&#x043b;&#x0432;";
    state.dataCostEstimateManager->monetaryUnit(12).html = "$";
    state.dataCostEstimateManager->monetaryUnit(13).html = "$";
    state.dataCostEstimateManager->monetaryUnit(14).html = "$b";
    state.dataCostEstimateManager->monetaryUnit(15).html = "R$";
    state.dataCostEstimateManager->monetaryUnit(16).html = "$";
    state.dataCostEstimateManager->monetaryUnit(17).html = "P";
    state.dataCostEstimateManager->monetaryUnit(18).html = "p.";
    state.dataCostEstimateManager->monetaryUnit(19).html = "BZ$";
    state.dataCostEstimateManager->monetaryUnit(20).html = "$";
    state.dataCostEstimateManager->monetaryUnit(21).html = "CHF";
    state.dataCostEstimateManager->monetaryUnit(22).html = "$";
    state.dataCostEstimateManager->monetaryUnit(23).html = "&#x5143;";
    state.dataCostEstimateManager->monetaryUnit(24).html = "$";
    state.dataCostEstimateManager->monetaryUnit(25).html = "&#x20a1;";
    state.dataCostEstimateManager->monetaryUnit(26).html = "&#x20b1;";
    state.dataCostEstimateManager->monetaryUnit(27).html = "&#x004b;&#x010d;";
    state.dataCostEstimateManager->monetaryUnit(28).html = "kr";
    state.dataCostEstimateManager->monetaryUnit(29).html = "RD$";
    state.dataCostEstimateManager->monetaryUnit(30).html = "kr";
    state.dataCostEstimateManager->monetaryUnit(31).html = "£";
    state.dataCostEstimateManager->monetaryUnit(32).html = "&#x20ac;";
    state.dataCostEstimateManager->monetaryUnit(33).html = "$";
    state.dataCostEstimateManager->monetaryUnit(34).html = "£";
    state.dataCostEstimateManager->monetaryUnit(35).html = "¢";
    state.dataCostEstimateManager->monetaryUnit(36).html = "£";
    state.dataCostEstimateManager->monetaryUnit(37).html = "Q";
    state.dataCostEstimateManager->monetaryUnit(38).html = "$";
    state.dataCostEstimateManager->monetaryUnit(39).html = "HK$";
    state.dataCostEstimateManager->monetaryUnit(40).html = "L";
    state.dataCostEstimateManager->monetaryUnit(41).html = "kn";
    state.dataCostEstimateManager->monetaryUnit(42).html = "Ft";
    state.dataCostEstimateManager->monetaryUnit(43).html = "Rp";
    state.dataCostEstimateManager->monetaryUnit(44).html = "&#x20aa;";
    state.dataCostEstimateManager->monetaryUnit(45).html = "£";
    state.dataCostEstimateManager->monetaryUnit(46).html = "&#x20a8;";
    state.dataCostEstimateManager->monetaryUnit(47).html = "&#xfdfc;";
    state.dataCostEstimateManager->monetaryUnit(48).html = "kr";
    state.dataCostEstimateManager->monetaryUnit(49).html = "£";
    state.dataCostEstimateManager->monetaryUnit(50).html = "J$";
    state.dataCostEstimateManager->monetaryUnit(51).html = "¥";
    state.dataCostEstimateManager->monetaryUnit(52).html = "&#x043b;&#x0432;";
    state.dataCostEstimateManager->monetaryUnit(53).html = "&#x17db;";
    state.dataCostEstimateManager->monetaryUnit(54).html = "&#x20a9;";
    state.dataCostEstimateManager->monetaryUnit(55).html = "&#x20a9;";
    state.dataCostEstimateManager->monetaryUnit(56).html = "$";
    state.dataCostEstimateManager->monetaryUnit(57).html = "&#x043b;&#x0432;";
    state.dataCostEstimateManager->monetaryUnit(58).html = "&#x20ad;";
    state.dataCostEstimateManager->monetaryUnit(59).html = "£";
    state.dataCostEstimateManager->monetaryUnit(60).html = "&#x20a8;";
    state.dataCostEstimateManager->monetaryUnit(61).html = "$";
    state.dataCostEstimateManager->monetaryUnit(62).html = "Lt";
    state.dataCostEstimateManager->monetaryUnit(63).html = "Ls";
    state.dataCostEstimateManager->monetaryUnit(64).html = "&#x0434;&#x0435;&#x043d;";
    state.dataCostEstimateManager->monetaryUnit(65).html = "&#x20ae;";
    state.dataCostEstimateManager->monetaryUnit(66).html = "&#x20a8;";
    state.dataCostEstimateManager->monetaryUnit(67).html = "$";
    state.dataCostEstimateManager->monetaryUnit(68).html = "RM";
    state.dataCostEstimateManager->monetaryUnit(69).html = "MT";
    state.dataCostEstimateManager->monetaryUnit(70).html = "$";
    state.dataCostEstimateManager->monetaryUnit(71).html = "&#x20a6;";
    state.dataCostEstimateManager->monetaryUnit(72).html = "C$";
    state.dataCostEstimateManager->monetaryUnit(73).html = "kr";
    state.dataCostEstimateManager->monetaryUnit(74).html = "&#x20a8;";
    state.dataCostEstimateManager->monetaryUnit(75).html = "$";
    state.dataCostEstimateManager->monetaryUnit(76).html = "&#xfdfc;";
    state.dataCostEstimateManager->monetaryUnit(77).html = "B/.";
    state.dataCostEstimateManager->monetaryUnit(78).html = "S/.";
    state.dataCostEstimateManager->monetaryUnit(79).html = "Php";
    state.dataCostEstimateManager->monetaryUnit(80).html = "&#x20a8;";
    state.dataCostEstimateManager->monetaryUnit(81).html = "&#x007a;&#x0142;";
    state.dataCostEstimateManager->monetaryUnit(82).html = "Gs";
    state.dataCostEstimateManager->monetaryUnit(83).html = "&#xfdfc;";
    state.dataCostEstimateManager->monetaryUnit(84).html = "lei";
    state.dataCostEstimateManager->monetaryUnit(85).html = "&#x0414;&#x0438;&#x043d;&#x002e;";
    state.dataCostEstimateManager->monetaryUnit(86).html = "&#x0440;&#x0443;&#x0431;";
    state.dataCostEstimateManager->monetaryUnit(87).html = "&#xfdfc;";
    state.dataCostEstimateManager->monetaryUnit(88).html = "$";
    state.dataCostEstimateManager->monetaryUnit(89).html = "&#x20a8;";
    state.dataCostEstimateManager->monetaryUnit(90).html = "kr";
    state.dataCostEstimateManager->monetaryUnit(91).html = "$";
    state.dataCostEstimateManager->monetaryUnit(92).html = "£";
    state.dataCostEstimateManager->monetaryUnit(93).html = "S";
    state.dataCostEstimateManager->monetaryUnit(94).html = "$";
    state.dataCostEstimateManager->monetaryUnit(95).html = "$";
    state.dataCostEstimateManager->monetaryUnit(96).html = "£";
    state.dataCostEstimateManager->monetaryUnit(97).html = "&#x0e3f;";
    state.dataCostEstimateManager->monetaryUnit(98).html = "&#x20a4;";
    state.dataCostEstimateManager->monetaryUnit(99).html = "YTL";
    state.dataCostEstimateManager->monetaryUnit(100).html = "TT$";
    state.dataCostEstimateManager->monetaryUnit(101).html = "$";
    state.dataCostEstimateManager->monetaryUnit(102).html = "NT$";
    state.dataCostEstimateManager->monetaryUnit(103).html = "&#x20b4;";
    state.dataCostEstimateManager->monetaryUnit(104).html = "$U";
    state.dataCostEstimateManager->monetaryUnit(105).html = "&#x043b;&#x0432;";
    state.dataCostEstimateManager->monetaryUnit(106).html = "Bs";
    state.dataCostEstimateManager->monetaryUnit(107).html = "&#x20ab;";
    state.dataCostEstimateManager->monetaryUnit(108).html = "$";
    state.dataCostEstimateManager->monetaryUnit(109).html = "&#xfdfc;";
    state.dataCostEstimateManager->monetaryUnit(110).html = "R";
    state.dataCostEstimateManager->monetaryUnit(111).html = "Z$";
}

int FindTariffIndex(
    EnergyPlusData &state, std::string const &nameOfTariff, std::string const &nameOfReferingObj, bool &ErrorsFound, std::string const &nameOfCurObj)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   May 2004

    //    Find the index for the tariff string provided or else
    //    raise a warning.

    auto &s_econ = state.dataEconTariff;

    int FindTariffIndex;
    int found = 0;

    for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
        if (Util::SameString(nameOfTariff, s_econ->tariff(iTariff).tariffName)) {
            found = iTariff;
            break;
        }
    }
    if (found > 0) {
        FindTariffIndex = found;
    } else {
        ShowSevereError(state, format("{}=\"{}\" invalid tariff referenced", nameOfCurObj, nameOfReferingObj));
        ShowContinueError(state, format("not found UtilityCost:Tariff=\"{}\".", nameOfTariff));
        ErrorsFound = true;
        FindTariffIndex = 0;
    }
    return FindTariffIndex;
}

void warnIfNativeVarname(
    EnergyPlusData &state, std::string const &objName, int const curTariffIndex, bool &ErrorsFound, std::string const &curobjName)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   March 2007

    //   Issue a warning if the variable name (usually the object name) is
    //   one of the names of native variables
    auto &s_econ = state.dataEconTariff;

    bool throwError = false;
    if (getEnumValue(nativeNamesUC, objName) != -1) {
        throwError = true;
    } else if (getEnumValue(catNamesUC, objName) != -1) {
        throwError = true;
    }

    if (throwError) {
        ErrorsFound = true;
        if (curTariffIndex >= 1 && curTariffIndex <= s_econ->numTariff) {
            ShowSevereError(state, format("UtilityCost:Tariff=\"{}\" invalid referenced name", s_econ->tariff(curTariffIndex).tariffName));
            ShowContinueError(state, format("{}=\"{}\" You cannot name an object using the same name as a native variable.", curobjName, objName));
        } else {
            ShowSevereError(state, format("{}=\"{}\" You cannot name an object using the same name as a native variable.", curobjName, objName));
        }
    }
}

int AssignVariablePt(EnergyPlusData &state,
                     std::string_view const stringIn,
                     bool const flagIfNotNumeric,
                     int const useOfVar,
                     int const varSpecific,
                     ObjType const econObjKind,
                     int const objIndex,
                     int const tariffPt)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   May 2004

    //   If the string is not numeric, check if it is a valid string to use as
    //   a variable name. Check if name has been used before and if not create
    //   the variable using the string as its name.
    //   Return the index of the variable.

    auto &s_econ = state.dataEconTariff;
    int AssignVariablePt;

    if (flagIfNotNumeric && (len(stringIn) >= 1)) {
        std::string inNoSpaces = RemoveSpaces(state, stringIn);
        int found = 0;
        if (allocated(s_econ->econVar)) {
            for (int iVar = 1; iVar <= s_econ->numEconVar; ++iVar) {
                if (s_econ->econVar(iVar).tariffIndx == tariffPt) {
                    if (Util::SameString(s_econ->econVar(iVar).name, inNoSpaces)) {
                        found = iVar;
                        break;
                    }
                }
            }
        }
        if (found > 0) {
            AssignVariablePt = found;
            if (s_econ->econVar(found).kindOfObj == ObjType::Invalid) {
                s_econ->econVar(found).kindOfObj = econObjKind;
                if (s_econ->econVar(found).index == 0) {
                    s_econ->econVar(found).index = objIndex;
                }
            }
        } else {
            incrementEconVar(state);
            s_econ->econVar(s_econ->numEconVar).name = inNoSpaces;
            s_econ->econVar(s_econ->numEconVar).kindOfObj = econObjKind;
            s_econ->econVar(s_econ->numEconVar).index = objIndex;
            AssignVariablePt = s_econ->numEconVar;
        }
        // now set the flag for the type of usage the variable has
        if (useOfVar == varIsArgument) {
            s_econ->econVar(AssignVariablePt).isArgument = true;
        } else if (useOfVar == varIsAssigned) {
            s_econ->econVar(AssignVariablePt).isAssigned = true;
        }
        s_econ->econVar(AssignVariablePt).tariffIndx = tariffPt;
        // if the user defines the UtilityCost:Computation then this is called when reading the
        // UtilityCost:Tariff with varNotYetDefined but they are already defined because
        // the subroutine CreateCategoryNativeVariables has already been called.

        //        if (!((varSpecific == varNotYetDefined) && (econVar(AssignVariablePt).specific >= catEnergyCharges))) {
        if ((varSpecific != varNotYetDefined) ||
            (s_econ->econVar(AssignVariablePt).kindOfObj != ObjType::Category && s_econ->econVar(AssignVariablePt).kindOfObj != ObjType::Native)) {
            s_econ->econVar(AssignVariablePt).specific = varSpecific;
        }
    } else { // if the string was numeric return a zero
        AssignVariablePt = 0;
    }
    return AssignVariablePt;
}

void incrementEconVar(EnergyPlusData &state)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   May 2004

    //   Increment the Increase the size of the

    int constexpr sizeIncrement(100);
    auto &s_econ = state.dataEconTariff;

    if (!allocated(s_econ->econVar)) {
        s_econ->econVar.allocate(sizeIncrement);
        s_econ->sizeEconVar = sizeIncrement;
        s_econ->numEconVar = 1;
    } else {
        ++s_econ->numEconVar;
        // if larger than current size grow the array
        if (s_econ->numEconVar > s_econ->sizeEconVar) {
            s_econ->econVar.redimension(s_econ->sizeEconVar += sizeIncrement);
        }
    }
    auto &econVar = s_econ->econVar(s_econ->numEconVar);

    // initialize new record) //Autodesk Most of these match default initialization so not needed
    econVar.name = "";
    econVar.tariffIndx = 0;
    econVar.kindOfObj = ObjType::Invalid;
    econVar.index = 0;
    std::fill(econVar.values.begin(), econVar.values.end(), 0.0);
    econVar.isArgument = false;
    econVar.isAssigned = false;
    econVar.specific = varNotYetDefined;
    //        econVar( numEconVar ).values = 0.0; //Autodesk Already initialized above
    // Autodesk Don't initialize cntMeDependOn
    econVar.Operator = Op::Invalid;
    econVar.firstOperand = 1; // Autodesk Default initialization sets this to 0
    econVar.lastOperand = 0;
    econVar.activeNow = false;
    econVar.isEvaluated = false;
    // Autodesk Don't initialize isReported
    // Autodesk Don't initialize varUnitType
}

void incrementSteps(EnergyPlusData &state)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   June 2004

    //   Increment the step array counter and if
    //   necessary increase the size of the array.

    auto &s_econ = state.dataEconTariff;
    int constexpr sizeIncrement(100);

    if (!allocated(s_econ->steps)) {
        s_econ->steps.allocate(sizeIncrement);
        s_econ->sizeSteps = sizeIncrement;
        s_econ->numSteps = 1;
    } else {
        ++s_econ->numSteps;
        // if larger than current size grow the array
        if (s_econ->numSteps > s_econ->sizeSteps) {
            s_econ->steps.redimension(s_econ->sizeSteps += sizeIncrement);
        }
    }
    // initialize new record
    s_econ->steps(s_econ->numSteps).type = StepType::EOL;
}

std::string RemoveSpaces(EnergyPlusData &state, std::string_view const StringIn)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   May 2004

    //   Return the string with all spaces removed.

    std::string StringOut;
    bool foundSpaces = false;
    for (std::string::size_type iString = 0; iString < len(StringIn); ++iString) {
        if (StringIn[iString] != ' ') {
            StringOut += StringIn[iString];
        } else {
            foundSpaces = true;
        }
    }
    if (foundSpaces) {
        ShowWarningError(state, format("UtilityCost: Spaces were removed from the variable=\"{}\".", StringIn));
        ShowContinueError(state, format("...Resultant variable=\"{}\".", StringOut));
    }
    return StringOut;
}

void CreateCategoryNativeVariables(EnergyPlusData &state)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   May 2004

    //    For each tariff create variables that are used for the
    //    categories (i.e., EnergyCharges).

    auto &s_econ = state.dataEconTariff;
    for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
        auto &tariff = s_econ->tariff(iTariff);

        // category variables first
        for (int iCat = 0; iCat < (int)Cat::Num; ++iCat) {
            tariff.cats[iCat] = AssignVariablePt(state, catNames[iCat], true, varIsAssigned, iCat, ObjType::Category, 0, iTariff);
        }

        tariff.firstCategory = tariff.cats[(int)Cat::EnergyCharges];
        tariff.lastCategory = tariff.cats[(int)Cat::NotIncluded];

        // category variables first

        for (int iNative = 0; iNative < (int)Native::Num; ++iNative) {
            tariff.natives[iNative] = AssignVariablePt(state, nativeNames[iNative], true, varIsArgument, iNative, ObjType::Native, 0, iTariff);
        }
        tariff.firstNative = tariff.natives[(int)Native::TotalEnergy];
        tariff.lastNative = tariff.natives[(int)Native::BelowCustomerBaseEnergy];
    }
}

//======================================================================================================================
//======================================================================================================================

//    DEFAULT COMPUTATION RELATED ROUTINES

//======================================================================================================================
//======================================================================================================================

void CreateDefaultComputation(EnergyPlusData &state)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   June 2004

    // PURPOSE OF THIS SUBROUTINE:
    //    For most tariffs defined in EnergyPlus no specific
    //    ECONOMICS:COMPUTATION will be entered. In that case,
    //    a default sequence of computation steps needs to be
    //    created.  This routine creates the default
    //    computation steps.
    //    Object           Fields         Depend On Fields
    //    Qualify          namePt         sourcePt
    //                                    thresholdPt
    //    Charge:Simple    namePt         sourcePt
    //                     categoryPt     costPerPt
    //    Charge:Block     namePt         sourcePt
    //                     categoryPt     blkSzMultPt
    //                     remainingPt    blkSzPt
    //                                    blkCostPt
    //    Ratchet          namePt         baselinePt
    //                                    adjustmentPt
    //                                    multiplierPt
    //                                    offsetPt
    //    These will be formed into expressions that look like
    //      namePt NOOP sourcePt thresholdPt
    //    The different Charges are combined using the SUM operation
    //    into categories.
    //      category SUM chg1Name chg2Name chg3Name
    //    Since the dependency array has one target and multiple
    //    parameters, remainingPt is shown as a separate equation that
    //    depends on namePt for Charge:Block. The equation will not be
    //    displayed or processed except in the sort.
    //      remainingPt NOOP namePt
    //    Many lines of the computation will include just the name of
    //    a single variable which triggers the calculation for that
    //    charge, ratchet or qualify.
    //      chg1Name
    //    It is also possible that two variables referenced within one
    //    object could include a dependency relationship also. For
    //    example, the blkSzPt could be calculated using the same sourePt
    //    in Charge:Block.

    // METHODOLOGY EMPLOYED:
    //    Since some ECONOMCIS:* objects depend on other variables
    //    first must create the order of when to perform the
    //    computations. First a dependency table is created that
    //    indicates what variables are dependent on other variables.
    //    A directed acyclic graph (DAG) describes the general
    //    problem which is usually solved using a topological
    //    sorting algorithm.
    //    Each line/step is generated and put into the depend
    //    array. Also in the array are counts of how many items it
    //    depends on and a list of entries that are dependent on that
    //    line.

    auto &s_econ = state.dataEconTariff;

    // for each tariff that does not have a UtilityCost:Computation object go through the variables
    for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
        auto &tariff = s_econ->tariff(iTariff);
        auto &computation = s_econ->computation(iTariff);
        if (!computation.isUserDef) {
            // clear all variables so that they are not active
            for (int jVar = 1; jVar <= s_econ->numEconVar; ++jVar) {
                s_econ->econVar(jVar).activeNow = false;
            }
            // make all native variables active
            for (int jVar = tariff.firstNative; jVar <= tariff.lastNative; ++jVar) {
                s_econ->econVar(jVar).activeNow = true;
            }
            //"clear" the dependOn array
            s_econ->numOperand = 0;
            // Define the preset equations (category summation)
            int curTotal = tariff.cats[(int)Cat::Total];
            int curSubtotal = tariff.cats[(int)Cat::Subtotal];
            int curBasis = tariff.cats[(int)Cat::Basis];
            // total SUM subtotal taxes
            s_econ->econVar(curTotal).Operator = Op::SUM;
            s_econ->econVar(curTotal).activeNow = true;
            addOperand(state, curTotal, curSubtotal);
            addOperand(state, curTotal, tariff.cats[(int)Cat::Taxes]);
            // subtotal SUM basis adjustments surcharges
            s_econ->econVar(curSubtotal).Operator = Op::SUM;
            s_econ->econVar(curSubtotal).activeNow = true;
            addOperand(state, curSubtotal, curBasis);
            addOperand(state, curSubtotal, tariff.cats[(int)Cat::Adjustment]);
            addOperand(state, curSubtotal, tariff.cats[(int)Cat::Surcharge]);
            // basis SUM EnergyCharges DemandCharges ServiceCharges
            s_econ->econVar(curBasis).Operator = Op::SUM;
            s_econ->econVar(curBasis).activeNow = true;
            addOperand(state, curBasis, tariff.cats[(int)Cat::EnergyCharges]);
            addOperand(state, curBasis, tariff.cats[(int)Cat::DemandCharges]);
            addOperand(state, curBasis, tariff.cats[(int)Cat::ServiceCharges]);
            // set up the equations for other objects
            addChargesToOperand(state, iTariff, tariff.cats[(int)Cat::EnergyCharges]);
            addChargesToOperand(state, iTariff, tariff.cats[(int)Cat::DemandCharges]);
            addChargesToOperand(state, iTariff, tariff.cats[(int)Cat::ServiceCharges]);
            addChargesToOperand(state, iTariff, tariff.cats[(int)Cat::Adjustment]);
            addChargesToOperand(state, iTariff, tariff.cats[(int)Cat::Surcharge]);
            addChargesToOperand(state, iTariff, tariff.cats[(int)Cat::Taxes]);
            // add the real time pricing to the energy charges
            if (tariff.chargeSched != nullptr) {
                addOperand(state, tariff.cats[(int)Cat::EnergyCharges], tariff.natives[(int)Native::RealTimePriceCosts]);
            }
            // now add equations with NOOP to represent each object with its
            // dependencies
            // Qualify
            for (int kObj = 1; kObj <= s_econ->numQualify; ++kObj) {
                auto const &qualify = s_econ->qualify(kObj);
                if (qualify.tariffIndx == iTariff) {
                    int curObject = qualify.namePt;
                    s_econ->econVar(curObject).Operator = Op::NOOP;
                    s_econ->econVar(curObject).activeNow = true;
                    addOperand(state, curObject, qualify.sourcePt);
                    addOperand(state, curObject, qualify.thresholdPt);
                }
            }
            // Ratchet
            for (int kObj = 1; kObj <= s_econ->numRatchet; ++kObj) {
                auto const &ratchet = s_econ->ratchet(kObj);
                if (ratchet.tariffIndx == iTariff) {
                    int curObject = ratchet.namePt;
                    s_econ->econVar(curObject).Operator = Op::NOOP;
                    s_econ->econVar(curObject).activeNow = true;
                    addOperand(state, curObject, ratchet.baselinePt);
                    addOperand(state, curObject, ratchet.adjustmentPt);
                    addOperand(state, curObject, ratchet.multiplierPt);
                    addOperand(state, curObject, ratchet.offsetPt);
                }
            }
            // ChargeSimple
            for (int kObj = 1; kObj <= s_econ->numChargeSimple; ++kObj) {
                auto const &chargeSimple = s_econ->chargeSimple(kObj);
                if (chargeSimple.tariffIndx == iTariff) {
                    int curObject = chargeSimple.namePt;
                    s_econ->econVar(curObject).Operator = Op::NOOP;
                    s_econ->econVar(curObject).activeNow = true;
                    addOperand(state, curObject, chargeSimple.sourcePt);
                    addOperand(state, curObject, chargeSimple.costPerPt);
                }
            }
            // ChargeBlock
            for (int kObj = 1; kObj <= s_econ->numChargeBlock; ++kObj) {
                auto const &chargeBlock = s_econ->chargeBlock(kObj);
                if (chargeBlock.tariffIndx == iTariff) {
                    int curObject = chargeBlock.namePt;
                    s_econ->econVar(curObject).Operator = Op::NOOP;
                    s_econ->econVar(curObject).activeNow = true;
                    addOperand(state, curObject, chargeBlock.sourcePt);
                    addOperand(state, curObject, chargeBlock.blkSzMultPt);
                    for (int mBlock = 1; mBlock <= chargeBlock.numBlk; ++mBlock) {
                        addOperand(state, curObject, chargeBlock.blkSzPt(mBlock));
                        addOperand(state, curObject, chargeBlock.blkCostPt(mBlock));
                    }
                    // now add a new "equation" for dependency of remainingPt on namePt
                    int remainPt = chargeBlock.remainingPt;
                    if (remainPt > 0) {
                        s_econ->econVar(remainPt).Operator = Op::NOOP;
                        s_econ->econVar(remainPt).activeNow = true;
                        addOperand(state, remainPt, curObject);
                    }
                }
            }
            // Economic:Variable
            // make all of the user defined variables as active
            for (int iVar = 1; iVar <= s_econ->numEconVar; ++iVar) {
                if (s_econ->econVar(iVar).tariffIndx == iTariff) {
                    if (s_econ->econVar(iVar).kindOfObj == ObjType::Variable) {
                        s_econ->econVar(iVar).activeNow = true;
                    }
                }
            }
            // make sure no computation is already user defined
            if (computation.firstStep != 0) {
                ShowWarningError(state, format("In UtilityCost:Tariff: Overwriting user defined tariff {}", tariff.tariffName));
            }
            // initialize the computation
            computation.computeName = "Autogenerated - " + tariff.tariffName;
            computation.firstStep = s_econ->numSteps + 1;
            computation.lastStep = -1; // this will be incremented by addStep
            computation.isUserDef = false;
            // now all "equations" are defined, treat the variables with the list
            // of dependencies as a directed acyclic graph and use "count down" algorithm
            // to do a topological sort of the variables into the order for computation
            // First, clear the counters
            for (int jVar = 1; jVar <= s_econ->numEconVar; ++jVar) {
                s_econ->econVar(jVar).cntMeDependOn = 0;
            }
            // Second, add up the number of dependencies on each variable
            for (int iVar = 1; iVar <= s_econ->numEconVar; ++iVar) {
                if (s_econ->econVar(iVar).activeNow) {
                    if (s_econ->econVar(iVar).lastOperand >= s_econ->econVar(iVar).firstOperand) {
                        s_econ->econVar(iVar).cntMeDependOn = 1 + s_econ->econVar(iVar).lastOperand - s_econ->econVar(iVar).firstOperand;
                    }
                }
            }
            // Third, start removing items with zero connections and decrease each
            //   counter.
            int numNoDepend = -1;
            int loopCount = 0;
            while ((numNoDepend != 0) || (loopCount > 100000)) {
                numNoDepend = 0;
                for (int iVar = 1; iVar <= s_econ->numEconVar; ++iVar) {
                    if (s_econ->econVar(iVar).activeNow) {
                        // find a variable that has no more dangling dependencies
                        if (s_econ->econVar(iVar).cntMeDependOn == 0) {
                            // If the variable is a native variable then
                            // IF (econVar(iVar)%kindOfObj .NE. iEconVarObjType::Native) THEN
                            if ((s_econ->econVar(iVar).kindOfObj != ObjType::Native) && (s_econ->econVar(iVar).kindOfObj != ObjType::Variable)) {
                                if (s_econ->econVar(iVar).lastOperand >= s_econ->econVar(iVar).firstOperand) {
                                    // transfer variables and operator to the computation and list of steps
                                    // go through the operands backwards (end of line is evaluated first)
                                    for (int kOperand = s_econ->econVar(iVar).lastOperand; kOperand >= s_econ->econVar(iVar).firstOperand;
                                         --kOperand) {
                                        incrementSteps(state);
                                        s_econ->steps(s_econ->numSteps) = s_econ->operands(kOperand);
                                    }
                                    // append the operator (either SUM or NOOP)
                                    incrementSteps(state);
                                    s_econ->steps(s_econ->numSteps).type = StepType::Op;
                                    s_econ->steps(s_econ->numSteps).op = s_econ->econVar(iVar).Operator;
                                    // append the variable itself
                                    incrementSteps(state);
                                    s_econ->steps(s_econ->numSteps).type = StepType::Var;
                                    s_econ->steps(s_econ->numSteps).varNum = iVar;
                                    // at the end of the line show a zero to clear the stack
                                    incrementSteps(state);
                                    s_econ->steps(s_econ->numSteps).type = StepType::EOL;
                                }
                            }
                            // go through each other variable looking for places where this variable is used
                            // and decrement their counters.
                            for (int jVar = 1; jVar <= s_econ->numEconVar; ++jVar) {
                                if (s_econ->econVar(jVar).activeNow) {
                                    for (int kOperand = s_econ->econVar(jVar).firstOperand; kOperand <= s_econ->econVar(jVar).lastOperand;
                                         ++kOperand) {
                                        int referVar = s_econ->operands(kOperand).varNum;
                                        if (iVar == referVar) {
                                            --s_econ->econVar(jVar).cntMeDependOn;
                                            // for each variable that has been decremented to zero increment the counter
                                            if (s_econ->econVar(jVar).cntMeDependOn <= 0) {
                                                ++numNoDepend;
                                            }
                                        }
                                    }
                                }
                            }
                            // make the variable inactive
                            s_econ->econVar(iVar).activeNow = false;
                        }
                    }
                }
                ++loopCount;
            }
            if (loopCount > 100000) {
                ShowWarningError(state,
                                 format("UtilityCost:Tariff: Loop count exceeded when counting dependencies in tariff: {}", tariff.tariffName));
            }
            // make sure that all variables associated with the tariff are included
            bool remainingVarFlag = false;
            for (int iVar = 1; iVar <= s_econ->numEconVar; ++iVar) {
                if (s_econ->econVar(iVar).activeNow) {
                    remainingVarFlag = true;
                }
            }
            if (remainingVarFlag) {
                ShowWarningError(state,
                                 format("CreateDefaultComputation: In UtilityCost:Computation: Circular or invalid dependencies found in tariff: {}",
                                        tariff.tariffName));
                ShowContinueError(state, "  UtilityCost variables that may have invalid dependencies and the variables they are dependent on.");
                for (int iVar = 1; iVar <= s_econ->numEconVar; ++iVar) {
                    if (s_econ->econVar(iVar).tariffIndx == iTariff) {
                        if (s_econ->econVar(iVar).activeNow) {
                            ShowContinueError(state, format("     {}", s_econ->econVar(iVar).name));
                            for (int kOperand = s_econ->econVar(iVar).firstOperand; kOperand <= s_econ->econVar(iVar).lastOperand; ++kOperand) {
                                ShowContinueError(state, format("        ->  {}", s_econ->econVar(s_econ->operands(kOperand).varNum).name));
                            }
                        }
                    }
                }
            }
            // set the end of the computations
            computation.lastStep = s_econ->numSteps;
            if (computation.firstStep >= computation.lastStep) {
                computation.firstStep = 0;
                computation.lastStep = -1;
                ShowWarningError(state,
                                 format("CreateDefaultComputation: In UtilityCost:Computation: No lines in the auto-generated computation can be "
                                        "interpreted in tariff: {}",
                                        tariff.tariffName));
            }
        }
    }
}

void addOperand(EnergyPlusData &state, int const varMe, int const varOperand)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    //   Used by CreateDefaultComputation to create the dependency
    //   relationship in the EconVar array

    int constexpr sizeIncrement(100);
    auto &s_econ = state.dataEconTariff;

    if (varOperand != 0) {
        // increment the numOperand and allocate/reallocate the array
        // if necessary
        if (!allocated(s_econ->operands)) {
            s_econ->operands.allocate(sizeIncrement);
            s_econ->sizeOperand = sizeIncrement;
            s_econ->numOperand = 1;
        } else {
            ++s_econ->numOperand;
            // if larger than current size grow the array
            if (s_econ->numOperand > s_econ->sizeOperand) {
                s_econ->operands.redimension(s_econ->sizeOperand += sizeIncrement);
            }
        }
        auto &econVar = s_econ->econVar(varMe);

        // now add the dependency relationship
        s_econ->operands(s_econ->numOperand).type = StepType::Var;
        s_econ->operands(s_econ->numOperand).varNum = varOperand;
        econVar.lastOperand = s_econ->numOperand;
        // if it is the first time addOperand was called with the varMe value
        // then set the first pointer as well
        if (varMe != s_econ->addOperand_prevVarMe) {
            econVar.firstOperand = s_econ->numOperand;
            s_econ->addOperand_prevVarMe = varMe;
        }
    }
}

void addChargesToOperand(EnergyPlusData &state, int const curTariff, int const curPointer)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    //   Used by CreateDefaultComputation to create the "equation"
    //   for the categories that are summations of ECONOMICS:CHARGES:BLOCK
    //   and ECONOMICS:CHARGES:SIMPLE

    auto const &s_econ = state.dataEconTariff;
    auto const &chargeSimple = s_econ->chargeSimple;
    auto const &chargeBlock = s_econ->chargeBlock;

    s_econ->econVar(curPointer).Operator = Op::SUM;
    s_econ->econVar(curPointer).activeNow = true;
    for (int kObj = 1; kObj <= s_econ->numChargeSimple; ++kObj) {
        if (chargeSimple(kObj).tariffIndx == curTariff) {
            if (chargeSimple(kObj).categoryPt == curPointer) {
                addOperand(state, curPointer, chargeSimple(kObj).namePt);
            }
        }
    }
    for (int kObj = 1; kObj <= s_econ->numChargeBlock; ++kObj) {
        if (chargeBlock(kObj).tariffIndx == curTariff) {
            if (chargeBlock(kObj).categoryPt == curPointer) {
                addOperand(state, curPointer, chargeBlock(kObj).namePt);
            }
        }
    }
}

//======================================================================================================================
//======================================================================================================================

//    GATHER TIMESTEP VALUES ROUTINE

//======================================================================================================================
//======================================================================================================================

void GatherForEconomics(EnergyPlusData &state)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   June 2004

    //   Gathers the data each timestep and updates the arrays
    //   holding the data that will be used by the tariff
    //   calculation.

    Real64 curInstantValue;
    Real64 curDemand;
    Real64 curEnergy;
    Real64 curRTPprice;    // real time price
    Real64 curRTPbaseline; // real time price customer baseline load
    Real64 curRTPenergy;   // energy applied to real time price
    Real64 curRTPcost;     // cost for energy for current time

    auto &s_econ = state.dataEconTariff;

    if (s_econ->numTariff >= 1) {
        for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
            auto &tariff = s_econ->tariff(iTariff);
            // if the meter is defined get the value
            if (tariff.reportMeterIndx != -1) {
                curInstantValue = GetCurrentMeterValue(state, tariff.reportMeterIndx);
            } else {
                curInstantValue = 0.0;
            }
            // remember the demand is still energy over a period of time divided by the
            // length of time. This gathers the energy also.
            tariff.collectEnergy += curInstantValue;
            tariff.collectTime += state.dataGlobal->TimeStepZoneSec;
            // added *SecInHour when adding RTP support August 2008
            if (tariff.collectTime >= tariff.demWinTime * Constant::rSecsInHour) {
                // get current value that has been converted into desired units
                curDemand = tariff.demandConv * tariff.collectEnergy / tariff.collectTime;
                curEnergy = tariff.energyConv * tariff.collectEnergy;
                // get the schedule values
                // remember no confirmation of schedule values occurs prior to now
                // The season schedule
                Season curSeason = (tariff.seasonSched != nullptr) ? static_cast<Season>(tariff.seasonSched->getCurrentVal()) : Season::Winter;
                int curPeriod = (tariff.periodSched != nullptr) ? tariff.periodSched->getCurrentVal() : 1;

                int curMonth =
                    (tariff.monthSched != nullptr) ? tariff.monthSched->getCurrentVal() :
                                                   // #7814 - Have to be careful with DST. tariff::seasonForMonth is overwritten at each timestep, and
                                                   // only the last value is retained, so make sure to capture the right one
                        (((state.dataGlobal->HourOfDay + state.dataEnvrn->DSTIndicator) <= Constant::iHoursInDay) ? state.dataEnvrn->Month
                                                                                                                  : state.dataEnvrn->MonthTomorrow);

                bool isGood = false;
                if (curSeason == Season::Winter || curSeason == Season::Spring || curSeason == Season::Summer || curSeason == Season::Fall ||
                    curSeason == Season::Annual) {
                    if (isWithinRange(state, curPeriod, 1, 4)) {
                        if (isWithinRange(state, curMonth, 1, 12)) {
                            isGood = true;
                        }
                    }
                }
                if (isGood) {
                    tariff.seasonForMonth(curMonth) = curSeason;
                    tariff.gatherEnergy(curMonth)[(int)curPeriod] += curEnergy;
                    if (tariff.gatherDemand(curMonth)[(int)curPeriod] < curDemand) {
                        tariff.gatherDemand(curMonth)[(int)curPeriod] = curDemand;
                    }
                } else {
                    ShowWarningError(state, format("UtilityCost:Tariff: While gathering for: {}", tariff.tariffName));
                    ShowContinueError(state, "Invalid schedule values - outside of range");
                }
                // Real Time Pricing
                if (tariff.chargeSched != nullptr) {
                    curRTPprice = tariff.chargeSched->getCurrentVal();
                    // if customer baseline load schedule is used, subtract that off of the
                    // current energy
                    if (tariff.baseUseSched != nullptr) {
                        curRTPbaseline = tariff.baseUseSched->getCurrentVal();
                        curRTPenergy = curEnergy - curRTPbaseline;
                    } else {
                        curRTPenergy = curEnergy;
                    }
                    // calculate the real time cost for current times energy
                    curRTPcost = curRTPenergy * curRTPprice;
                    tariff.RTPcost(curMonth) += curRTPcost;
                    if (curRTPcost > 0) {
                        tariff.RTPaboveBaseCost(curMonth) += curRTPcost;
                    } else {
                        tariff.RTPbelowBaseCost(curMonth) += curRTPcost;
                    }
                    if (curRTPenergy > 0) {
                        tariff.RTPaboveBaseEnergy(curMonth) += curRTPenergy;
                    } else {
                        tariff.RTPbelowBaseEnergy(curMonth) += curRTPenergy;
                    }
                }
                // reset the counters
                tariff.collectEnergy = 0.0;
                tariff.collectTime = 0.0;
            }
        }
    }
}

bool isWithinRange(EnergyPlusData &state, int const testVal, int const minThreshold, int const maxThreshold)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    //   Simple function to check if an integer is equal to or between
    //   two other values.

    bool isWithinRange;

    if (maxThreshold < minThreshold) {
        ShowWarningError(state, "UtilityCost: Invalid thresholds in IsWithinRange routine.");
    }
    if ((testVal <= maxThreshold) && (testVal >= minThreshold)) {
        isWithinRange = true;
    } else {
        isWithinRange = false;
    }
    return isWithinRange;
}

//======================================================================================================================
//======================================================================================================================

//    COMPUTE THE UTILITY BILLS AND CREATE REPORTS

//======================================================================================================================
//======================================================================================================================

void ComputeTariff(EnergyPlusData &state)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    //    Perform the calculation steps to compute the monthly
    //    utility bills for the user entered tariffs.
    //    The list of steps for the tariff computation are in order
    //    for stack based computation (reverse polish notation)

    auto &s_econ = state.dataEconTariff;

    // values used in specific operations
    Array1D<Real64> a(NumMonths);
    Array1D<Real64> b(NumMonths);
    Array1D<Real64> c(NumMonths);
    Array1D<Real64> d(NumMonths);

    int constexpr noVar(0);

    Real64 annualAggregate;

    if (!state.files.outputControl.writeTabular(state)) {
        state.dataOutRptTab->WriteTabularFiles = false;
        return;
    }

    Real64 hugeValue = HUGE_(Real64());
    //  Clear the isEvaluated flags for all economics variables.
    for (int nVar = 1; nVar <= s_econ->numEconVar; ++nVar) {
        s_econ->econVar(nVar).isEvaluated = false;
    }

    if (s_econ->numTariff == 0) {
        return;
    }

    state.dataOutRptTab->WriteTabularFiles = true;
    setNativeVariables(state);
    int aPt;
    int bPt;
    int cPt;
    for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
        for (int jStep = s_econ->computation(iTariff).firstStep; jStep <= s_econ->computation(iTariff).lastStep; ++jStep) {
            auto const &step = s_econ->steps(jStep);
            int annualCnt = 0;

            // end of line - assign variable and clear stack
            // if the stack still has two items on it then assign the values to the
            // pointer otherwise if it follows a NOOP line it will only have one item
            // that has already been assigned and no further action is required.
            if (step.type == StepType::EOL) {
                if (s_econ->topOfStack >= 2) {
                    popStack(state, b, bPt); // pop the variable pointer
                    popStack(state, a, aPt); // pop the values
                    if (isWithinRange(state, bPt, 1, s_econ->numEconVar)) {
                        s_econ->econVar(bPt).values = a;
                    }
                }
                s_econ->topOfStack = 0;

                // Variable
            } else if (step.type == StepType::Var) {
                pushStack(state, s_econ->econVar(step.varNum).values, step.varNum);

                // Operator
            } else if (step.type == StepType::Op) {
                switch (step.op) {

                case Op::SUM: {
                    a = 0.0;
                    for (int kStack = 1, kStack_end = s_econ->topOfStack; kStack <= kStack_end; ++kStack) { // popStack modifies topOfStack
                        popStack(state, b, bPt);
                        a += b;
                    }
                    pushStack(state, a, noVar);
                } break;

                case Op::MULTIPLY: {
                    popStack(state, b, bPt);
                    popStack(state, a, aPt);
                    pushStack(state, a * b, noVar);
                } break;

                case Op::SUBTRACT: {
                    popStack(state, b, bPt);
                    popStack(state, a, aPt);
                    pushStack(state, b - a, noVar);
                } break;

                case Op::DIVIDE: {
                    popStack(state, a, aPt);
                    popStack(state, b, bPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        c(lMonth) = (b(lMonth) != 0.0) ? (a(lMonth) / b(lMonth)) : 0.0;
                    }
                    pushStack(state, c, noVar);
                } break;

                case Op::ABSOLUTEVALUE: {
                    popStack(state, a, aPt);
                    pushStack(state, ObjexxFCL::abs(a), noVar);
                } break;

                case Op::INTEGER: {
                    popStack(state, a, aPt);
                    pushStack(state, Array1D_double(Array1D_int(a)), noVar);
                } break;

                case Op::SIGN: {
                    popStack(state, a, aPt);
                    pushStack(state, sign(1.0, a), noVar);
                    //        CASE (opROUND)
                    //          CALL popStack(b,bPt)
                    //          CALL popStack(a,aPt)
                    //          DO lMonth = 1,MaxNumMonths
                    //            IF ((b(lMonth) .LE. 5) .AND. (b(lMonth) .GE. -5)) THEN
                    //              c(lMonth) = FLOAT(INT(a(lMonth) / (10 ** b(lMonth))) * (10 ** b(lMonth)))
                    //            END IF
                    //          END DO
                    //          CALL pushStack(c,noVar)
                } break;

                case Op::MAXIMUM: {
                    a = -hugeValue;
                    for (int kStack = 1, kStack_end = s_econ->topOfStack; kStack <= kStack_end; ++kStack) { // popStack modifies topOfStack
                        popStack(state, b, bPt);
                        for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                            if (b(lMonth) > a(lMonth)) {
                                a(lMonth) = b(lMonth);
                            }
                        }
                    }
                    pushStack(state, a, noVar);
                } break;

                case Op::MINIMUM: {
                    a = hugeValue;
                    for (int kStack = 1, kStack_end = s_econ->topOfStack; kStack <= kStack_end; ++kStack) { // popStack modifies topOfStack
                        popStack(state, b, bPt);
                        for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                            if (b(lMonth) < a(lMonth)) {
                                a(lMonth) = b(lMonth);
                            }
                        }
                    }
                    pushStack(state, a, noVar);
                } break;

                case Op::EXCEEDS: {
                    popStack(state, b, bPt);
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        if (a(lMonth) > b(lMonth)) {
                            c(lMonth) = a(lMonth) - b(lMonth);
                        } else {
                            c(lMonth) = 0.0;
                        }
                    }
                    pushStack(state, c, noVar);
                } break;

                case Op::ANNUALMINIMUM: {
                    // takes the minimum but ignores zeros
                    annualAggregate = hugeValue;
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        if (a(lMonth) != 0) {
                            if (a(lMonth) < annualAggregate) {
                                annualAggregate = a(lMonth);
                            }
                        }
                    }
                    // if all months are zero then hugeValue still in annual but should be zero
                    if (annualAggregate == hugeValue) {
                        annualAggregate = 0.0;
                    }
                    c = annualAggregate;
                    pushStack(state, c, noVar);
                } break;

                case Op::ANNUALMAXIMUM: {
                    // takes the maximum but ignores zeros
                    annualAggregate = -hugeValue;
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        if (a(lMonth) != 0) {
                            if (a(lMonth) > annualAggregate) {
                                annualAggregate = a(lMonth);
                            }
                        }
                    }
                    // if all months are zero then hugeValue still in annual but should be zero
                    if (annualAggregate == -hugeValue) {
                        annualAggregate = 0.0;
                    }
                    c = annualAggregate;
                    pushStack(state, c, noVar);
                } break;

                case Op::ANNUALSUM: {
                    // takes the maximum but ignores zeros
                    annualAggregate = 0.0;
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        annualAggregate += a(lMonth);
                    }
                    c = annualAggregate;
                    pushStack(state, c, noVar);
                } break;

                case Op::ANNUALAVERAGE: {
                    // takes the annual sum but ignores zeros
                    annualAggregate = 0.0;
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        if (a(lMonth) != 0) {
                            annualAggregate += a(lMonth);
                            ++annualCnt;
                        }
                    }
                    // if all months are zero then return zero
                    c = (annualCnt != 0) ? (annualAggregate / annualCnt) : 0.0;
                    pushStack(state, c, noVar);
                } break;

                case Op::ANNUALOR: {
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        if (a(lMonth) != 0) {
                            ++annualCnt;
                        }
                    }
                    // if any months is not zero then "true"
                    c = (annualCnt >= 1) ? 1.0 : 0.0;
                    pushStack(state, c, noVar);
                } break;

                case Op::ANNUALAND: {
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        if (a(lMonth) != 0) {
                            ++annualCnt;
                        }
                    }
                    // if all months are not zero then "true"
                    c = (annualCnt == NumMonths) ? 1.0 : 0.0;
                    pushStack(state, c, noVar);
                } break;

                case Op::ANNUALMAXIMUMZERO: {
                    // takes the maximum including zeros
                    annualAggregate = -hugeValue;
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        if (a(lMonth) > annualAggregate) {
                            annualAggregate = a(lMonth);
                        }
                    }
                    c = annualAggregate;
                    pushStack(state, c, noVar);
                } break;

                case Op::ANNUALMINIMUMZERO: {
                    // takes the maximum including zeros
                    annualAggregate = hugeValue;
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        if (a(lMonth) < annualAggregate) {
                            annualAggregate = a(lMonth);
                        }
                    }
                    c = annualAggregate;
                    pushStack(state, c, noVar);
                } break;

                case Op::IF: {
                    popStack(state, c, cPt);
                    popStack(state, b, bPt);
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        d(lMonth) = (a(lMonth) != 0) ? b(lMonth) : c(lMonth);
                    }
                    pushStack(state, d, noVar);
                } break;

                case Op::GREATERTHAN: {
                    popStack(state, b, bPt);
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        c(lMonth) = (a(lMonth) > b(lMonth)) ? 1.0 : 0.0;
                    }
                    pushStack(state, c, noVar);
                } break;

                case Op::GREATEREQUAL: {
                    popStack(state, b, bPt);
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        if (a(lMonth) >= b(lMonth)) {
                            c(lMonth) = 1.0;
                        } else {
                            c(lMonth) = 0.0;
                        }
                    }
                    pushStack(state, c, noVar);
                } break;

                case Op::LESSTHAN: {
                    popStack(state, b, bPt);
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        c(lMonth) = (a(lMonth) < b(lMonth)) ? 1.0 : 0.0;
                    }
                    pushStack(state, c, noVar);
                } break;

                case Op::LESSEQUAL: {
                    popStack(state, b, bPt);
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        c(lMonth) = (a(lMonth) <= b(lMonth)) ? 1.0 : 0.0;
                    }
                    pushStack(state, c, noVar);
                } break;

                case Op::EQUAL: {
                    popStack(state, b, bPt);
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        c(lMonth) = (a(lMonth) == b(lMonth)) ? 1.0 : 0.0;
                    }
                    pushStack(state, c, noVar);
                } break;

                case Op::NOTEQUAL: {
                    popStack(state, b, bPt);
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        c(lMonth) = (a(lMonth) != b(lMonth)) ? 1.0 : 0.0;
                    }
                    pushStack(state, c, noVar);
                } break;

                case Op::AND: {
                    popStack(state, b, bPt);
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        c(lMonth) = ((a(lMonth) != 0) && (b(lMonth) != 0)) ? 1.0 : 0.0;
                    }
                    pushStack(state, c, noVar);
                } break;

                case Op::OR: {
                    popStack(state, b, bPt);
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        c(lMonth) = ((a(lMonth) != 0) || (b(lMonth) != 0)) ? 1.0 : 0.0;
                    }
                    pushStack(state, c, noVar);
                } break;

                case Op::NOT: {
                    popStack(state, a, aPt);
                    for (int lMonth = 1; lMonth <= NumMonths; ++lMonth) {
                        c(lMonth) = (a(lMonth) == 0) ? 1.0 : 0.0;
                    }
                    pushStack(state, c, noVar);
                } break;

                case Op::ADD: {
                    popStack(state, b, bPt);
                    popStack(state, a, aPt);
                    pushStack(state, a + b, noVar);
                } break;

                case Op::NOOP: {
                    // do nothing but clear the stack
                    s_econ->topOfStack = 0;
                    // No longer pushing a zero to fix bug
                    // and push zero
                    // a = 0
                } break;

                default: {
                } break;
                }
            }
        }
        checkMinimumMonthlyCharge(state, iTariff);
    }
    selectTariff(state);
    LEEDtariffReporting(state);
}

void pushStack(EnergyPlusData &state, Array1A<Real64> const monthlyArray, int const variablePointer)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    //    A stack is used in the evaluation of the tariff since
    //    the variables and operators are in a reverse polish
    //    notation order. The stack operates on a last-in
    //    first out basis. The stack consists of both a pointer
    //    to the variable and the twelve monthly values.
    //    This routine puts an item on the top of the stack.

    monthlyArray.dim(NumMonths);

    Array1D<Real64> curMonthlyArray(NumMonths);
    int constexpr sizeIncrement(50);

    auto &s_econ = state.dataEconTariff;
    auto &stack = s_econ->stack;
    auto const &econVar = s_econ->econVar;
    auto const &tariff = s_econ->tariff;

    curMonthlyArray = monthlyArray;
    if (!allocated(stack)) {
        stack.allocate(sizeIncrement);
        s_econ->sizeStack = sizeIncrement;
        s_econ->topOfStack = 1;
    } else {
        ++s_econ->topOfStack;
        // if larger than current size grow the array
        if (s_econ->topOfStack > s_econ->sizeStack) {
            stack.redimension(s_econ->sizeStack += sizeIncrement);
        }
    }
    // now push the values on to the stack
    stack(s_econ->topOfStack).varPt = variablePointer;
    // check if variable has been evaluated if it is CHARGE:SIMPLE, CHARGE:BLOCK, RATCHET, or QUALIFY
    // if it has not overwrite the values for monthlyArray with the evaluated values
    if (variablePointer != 0) {
        if (!econVar(variablePointer).isEvaluated) {

            switch (econVar(variablePointer).kindOfObj) {
            case ObjType::ChargeSimple:
                evaluateChargeSimple(state, variablePointer);
                break;
            case ObjType::ChargeBlock:
                evaluateChargeBlock(state, variablePointer);
                break;
            case ObjType::Ratchet:
                evaluateRatchet(state, variablePointer);
                break;
            case ObjType::Qualify:
                evaluateQualify(state, variablePointer);
                break;
            case ObjType::Invalid:
                ShowWarningError(state, format("UtilityCost variable not defined: {}", econVar(variablePointer).name));
                ShowContinueError(state, format("   In tariff: {}", tariff(econVar(variablePointer).tariffIndx).tariffName));
                ShowContinueError(state, "   This may be the result of a misspelled variable name in the UtilityCost:Computation object.");
                ShowContinueError(state, "   All zero values will be assumed for this variable.");
                break;
            case ObjType::Variable:
            case ObjType::Category:
            case ObjType::Native:
            case ObjType::AssignCompute:
            case ObjType::Tariff:
            case ObjType::Computation:
                // do nothing
                break;
            default:
                ShowWarningError(state,
                                 format("UtilityCost Debugging issue. Invalid kind of variable used (pushStack). {} in tariff: {}",
                                        econVar(variablePointer).kindOfObj,
                                        tariff(econVar(variablePointer).tariffIndx).tariffName));
            }
            // if the serviceCharges are being evaluated add in the monthly charges
            if (econVar(variablePointer).kindOfObj == ObjType::Category && econVar(variablePointer).specific == (int)Cat::ServiceCharges) {
                addMonthlyCharge(state, variablePointer);
            }
            // get the results of performing the evaluation - should have been
            // put into the econVar values
            curMonthlyArray = econVar(variablePointer).values;
        }
    }
    // now assign
    stack(s_econ->topOfStack).values = curMonthlyArray;
}

void popStack(EnergyPlusData &state, Array1A<Real64> monthlyArray, int &variablePointer)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    //    A stack is used in the evaluation of the tariff since
    //    the variables and operators are in a reverse polish
    //    notation order. The stack operates on a last-in
    //    first out basis. The stack consists of both a pointer
    //    to the variable and the twelve monthly values.
    //    This routine returns the item on the top of the stack
    //    and removes it from the stack.

    monthlyArray.dim(NumMonths);

    auto &s_econ = state.dataEconTariff;
    auto const &stack = s_econ->stack;

    if (s_econ->topOfStack >= 1) {
        variablePointer = stack(s_econ->topOfStack).varPt;
        monthlyArray = stack(s_econ->topOfStack).values;
    } else {
        ShowWarningError(
            state,
            format("UtilityCost:Tariff: stack underflow in calculation of utility bills. On variable: {}", s_econ->econVar(variablePointer).name));
        variablePointer = 0;
        monthlyArray = {0.0};
        s_econ->topOfStack = 0;
    }
    --s_econ->topOfStack;
}

void evaluateChargeSimple(EnergyPlusData &state, int const usingVariable)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    Array1D<Real64> sourceVals(NumMonths);
    Array1D<Real64> costPer(NumMonths);
    Array1D<Real64> resultChg(NumMonths);
    Array1D<Real64> seasonMask(NumMonths);

    auto &s_econ = state.dataEconTariff;
    int curTariff = s_econ->econVar(usingVariable).tariffIndx;
    auto const &tariff = s_econ->tariff(curTariff);
    int indexInChg = s_econ->econVar(usingVariable).index;
    auto const &chargeSimple = s_econ->chargeSimple(indexInChg);

    // check the tariff - make sure they match
    if (chargeSimple.namePt != usingVariable) {
        ShowWarningError(state, "UtilityCost:Tariff Debugging issue. ChargeSimple index does not match variable pointer.");
        ShowContinueError(state, format("   Between: {}", s_econ->econVar(usingVariable).name));
        ShowContinueError(state, format("       And: {}", s_econ->econVar(chargeSimple.namePt).name));
    }
    if (chargeSimple.tariffIndx != curTariff) {
        ShowWarningError(state, "UtilityCost:Tariff Debugging issue. ChargeSimple index does not match tariff index.");
        ShowContinueError(state, format("   Between: {}", tariff.tariffName));
        ShowContinueError(state, format("       And: {}", s_econ->tariff(chargeSimple.tariffIndx).tariffName));
    }
    // data from the Charge:Simple
    sourceVals = s_econ->econVar(chargeSimple.sourcePt).values;
    // determine if costPer should be based on variable or value
    if (chargeSimple.costPerPt != 0) {
        costPer = s_econ->econVar(chargeSimple.costPerPt).values;
    } else {
        costPer = chargeSimple.costPerVal;
    }
    // find proper season mask
    if (chargeSimple.season == Season::Summer) {
        seasonMask = s_econ->econVar(tariff.natives[(int)Native::IsSummer]).values;
    } else if (chargeSimple.season == Season::Winter) {
        seasonMask = s_econ->econVar(tariff.natives[(int)Native::IsWinter]).values;
    } else if (chargeSimple.season == Season::Spring) {
        seasonMask = s_econ->econVar(tariff.natives[(int)Native::IsSpring]).values;
    } else if (chargeSimple.season == Season::Fall) {
        seasonMask = s_econ->econVar(tariff.natives[(int)Native::IsAutumn]).values;
    } else if (chargeSimple.season == Season::Annual) {
        seasonMask = 1.0; // all months are 1
    }

    // finally perform calculations
    resultChg = sourceVals * costPer * seasonMask;
    // store the cost in the name of the variable
    s_econ->econVar(usingVariable).values = resultChg;
    // set the flag that it has been evaluated so it won't be evaluated multiple times
    s_econ->econVar(usingVariable).isEvaluated = true;
}

void evaluateChargeBlock(EnergyPlusData &state, int const usingVariable)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    Array1D<Real64> sourceVals(NumMonths);
    Array1D<Real64> blkSzMult(NumMonths);
    Array1D<Real64> remainVals(NumMonths);
    Array1D<Real64> resultChg(NumMonths);
    Array1D<Real64> amountForBlk(NumMonths);
    Array1D<Real64> curBlkSz(NumMonths);
    Array1D<Real64> curBlkCost(NumMonths);
    Array1D<Real64> seasonMask(NumMonths);

    auto &s_econ = state.dataEconTariff;
    int curTariff = s_econ->econVar(usingVariable).tariffIndx;
    auto const &tariff = s_econ->tariff(curTariff);
    int indexInChg = s_econ->econVar(usingVariable).index;
    auto const &chargeBlock = s_econ->chargeBlock(indexInChg);

    // check the tariff - make sure they match
    if (chargeBlock.namePt != usingVariable) {
        ShowWarningError(state, "UtilityCost:Tariff Debugging issue. chargeBlock index does not match variable pointer.");
        ShowContinueError(state, format("   Between: {}", s_econ->econVar(usingVariable).name));
        ShowContinueError(state, format("       And: {}", s_econ->econVar(chargeBlock.namePt).name));
    }
    if (chargeBlock.tariffIndx != curTariff) {
        ShowWarningError(state, "UtilityCost:Tariff Debugging issue. chargeBlock index does not match tariff index.");
        ShowContinueError(state, format("   Between: {}", tariff.tariffName));
        ShowContinueError(state, format("       And: {}", s_econ->tariff(chargeBlock.tariffIndx).tariffName));
    }
    // data from the chargeBlock
    sourceVals = s_econ->econVar(chargeBlock.sourcePt).values;
    // find proper season mask

    if (chargeBlock.season == Season::Summer) {
        seasonMask = s_econ->econVar(tariff.natives[(int)Native::IsSummer]).values;
    } else if (chargeBlock.season == Season::Winter) {
        seasonMask = s_econ->econVar(tariff.natives[(int)Native::IsWinter]).values;
    } else if (chargeBlock.season == Season::Spring) {
        seasonMask = s_econ->econVar(tariff.natives[(int)Native::IsSpring]).values;
    } else if (chargeBlock.season == Season::Fall) {
        seasonMask = s_econ->econVar(tariff.natives[(int)Native::IsAutumn]).values;
    } else if (chargeBlock.season == Season::Annual) {
        seasonMask = 1.0; // all months are 1
    }
    // get block size multiplier
    if (chargeBlock.blkSzMultPt != 0) {
        blkSzMult = s_econ->econVar(chargeBlock.blkSzMultPt).values;
    } else {
        blkSzMult = chargeBlock.blkSzMultVal;
    }
    // initially set the remaining energy or demand to the source
    remainVals = sourceVals;
    // initially set the result (cost) to zero
    resultChg = 0.0;
    // loop through the blocks performing calculations
    for (int iBlk = 1; iBlk <= chargeBlock.numBlk; ++iBlk) {
        if (chargeBlock.blkSzPt(iBlk) != 0) {
            curBlkSz = s_econ->econVar(chargeBlock.blkSzPt(iBlk)).values;
        } else {
            curBlkSz = chargeBlock.blkSzVal(iBlk);
        }
        if (chargeBlock.blkCostPt(iBlk) != 0) {
            curBlkCost = s_econ->econVar(chargeBlock.blkCostPt(iBlk)).values;
        } else {
            curBlkCost = chargeBlock.blkCostVal(iBlk);
        }
        // loop through the months
        for (int jMonth = 1; jMonth <= NumMonths; ++jMonth) {
            if (seasonMask(jMonth) == 1) {
                // IF ((curBlkSz(jMonth) * blkSzMult(jMonth)) .GT. remainVals(jMonth)) THEN - CR 6547
                if (blkSzMult(jMonth) != 0) {
                    if (curBlkSz(jMonth) > (remainVals(jMonth) / blkSzMult(jMonth))) {
                        amountForBlk(jMonth) = remainVals(jMonth);
                    } else {
                        amountForBlk(jMonth) = curBlkSz(jMonth) * blkSzMult(jMonth);
                    }
                } else {
                    amountForBlk(jMonth) = 0.0;
                }
                resultChg(jMonth) += amountForBlk(jMonth) * curBlkCost(jMonth);
                remainVals(jMonth) -= amountForBlk(jMonth);
            }
        }
    }
    // store the amount remaining if a variable is specified
    if (chargeBlock.remainingPt != 0) {
        s_econ->econVar(chargeBlock.remainingPt).values = remainVals;
    } else {
        bool flagAllZero = true;
        for (int jMonth = 1; jMonth <= NumMonths; ++jMonth) {
            if (seasonMask(jMonth) == 1) {
                if (remainVals(jMonth) != 0) {
                    flagAllZero = false;
                }
            }
        }
        if (!flagAllZero) {
            ShowWarningError(
                state,
                format("UtilityCost:Tariff Not all energy or demand was assigned in the block charge: {}", s_econ->econVar(usingVariable).name));
        }
    }
    // store the cost in the name of the variable
    s_econ->econVar(usingVariable).values = resultChg;
    // set the flag that it has been evaluated so it won't be evaluated multiple times
    s_econ->econVar(usingVariable).isEvaluated = true;
}

void evaluateRatchet(EnergyPlusData &state, int const usingVariable)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    Array1D<Real64> baselineVals(NumMonths);
    Array1D<Real64> adjustmentVals(NumMonths);
    Array1D<Real64> multiplierVals(NumMonths);
    Array1D<Real64> offsetVals(NumMonths);
    Array1D<Real64> seasonFromMask(NumMonths);
    Array1D<Real64> seasonToMask(NumMonths);
    Array1D<Real64> adjSeasonal(NumMonths);
    Array1D<Real64> adjPeak(NumMonths);
    Array1D<Real64> maxAdjBase(NumMonths);
    Array1D<Real64> finalResult(NumMonths);

    auto &s_econ = state.dataEconTariff;
    int curTariff = s_econ->econVar(usingVariable).tariffIndx;
    auto const &tariff = s_econ->tariff(curTariff);
    int indexInChg = s_econ->econVar(usingVariable).index;
    auto const &ratchet = s_econ->ratchet(indexInChg);
    bool isMonthly = false;

    // check the tariff - make sure they match
    if (ratchet.namePt != usingVariable) {
        ShowWarningError(state, "UtilityCost:Tariff Debugging issue. Ratchet index does not match variable pointer.");
        ShowContinueError(state, format("   Between: {}", s_econ->econVar(usingVariable).name));
        ShowContinueError(state, format("       And: {}", s_econ->econVar(ratchet.namePt).name));
    }
    if (ratchet.tariffIndx != curTariff) {
        ShowWarningError(state, "UtilityCost:Tariff Debugging issue. Ratchet index does not match tariff index.");
        ShowContinueError(state, format("   Between: {}", tariff.tariffName));
        ShowContinueError(state, format("       And: {}", s_econ->tariff(ratchet.tariffIndx).tariffName));
    }
    // data from the Ratchet
    baselineVals = s_econ->econVar(ratchet.baselinePt).values;
    adjustmentVals = s_econ->econVar(ratchet.adjustmentPt).values;
    // determine if multiplier should be based on variable or value
    if (ratchet.multiplierPt != 0) {
        multiplierVals = s_econ->econVar(ratchet.multiplierPt).values;
    } else {
        multiplierVals = ratchet.multiplierVal;
    }
    // determine if offset should be based on variable or value
    if (ratchet.offsetPt != 0) {
        offsetVals = s_econ->econVar(ratchet.offsetPt).values;
    } else {
        offsetVals = ratchet.offsetVal;
    }
    // find proper season from mask
    if (ratchet.seasonFrom == Season::Summer) {
        seasonFromMask = s_econ->econVar(tariff.natives[(int)Native::IsSummer]).values;
        isMonthly = false;
    } else if (ratchet.seasonFrom == Season::Winter) {
        seasonFromMask = s_econ->econVar(tariff.natives[(int)Native::IsWinter]).values;
        isMonthly = false;
    } else if (ratchet.seasonFrom == Season::Spring) {
        seasonFromMask = s_econ->econVar(tariff.natives[(int)Native::IsSpring]).values;
        isMonthly = false;
    } else if (ratchet.seasonFrom == Season::Fall) {
        seasonFromMask = s_econ->econVar(tariff.natives[(int)Native::IsAutumn]).values;
        isMonthly = false;
    } else if (ratchet.seasonFrom == Season::Annual) {
        seasonFromMask = 1.0; // all months are 1
        isMonthly = false;
    } else if (ratchet.seasonFrom == Season::Monthly) {
        seasonFromMask = 1.0; // all months are 1
        isMonthly = true;
    } else {
        assert(false);
    }

    // find proper season to mask
    if (ratchet.seasonTo == Season::Summer) {
        seasonToMask = s_econ->econVar(tariff.natives[(int)Native::IsSummer]).values;
    } else if (ratchet.seasonTo == Season::Winter) {
        seasonToMask = s_econ->econVar(tariff.natives[(int)Native::IsWinter]).values;
    } else if (ratchet.seasonTo == Season::Spring) {
        seasonToMask = s_econ->econVar(tariff.natives[(int)Native::IsSpring]).values;
    } else if (ratchet.seasonTo == Season::Fall) {
        seasonToMask = s_econ->econVar(tariff.natives[(int)Native::IsAutumn]).values;
    } else if (ratchet.seasonTo == Season::Annual) {
        seasonToMask = 1.0; // all months are 1
    }
    // finally perform calculations
    if (isMonthly) {
        adjSeasonal = adjustmentVals;
    } else {
        Real64 maximumVal = -HUGE_(Real64());
        for (int iMonth = 1; iMonth <= NumMonths; ++iMonth) {
            if (seasonFromMask(iMonth) == 1) {
                if (adjustmentVals(iMonth) > maximumVal) {
                    maximumVal = adjustmentVals(iMonth);
                }
            }
        }
        adjSeasonal = maximumVal;
    }
    for (int iMonth = 1; iMonth <= NumMonths; ++iMonth) {
        // calculate adjusted peak value after offset and multiplier
        adjPeak(iMonth) = (adjSeasonal(iMonth) + offsetVals(iMonth)) * multiplierVals(iMonth);
        // the maximum of the adjustment and the baseline
        if (adjPeak(iMonth) > baselineVals(iMonth)) {
            maxAdjBase(iMonth) = adjPeak(iMonth);
        } else {
            maxAdjBase(iMonth) = baselineVals(iMonth);
        }
    }
    for (int iMonth = 1; iMonth <= NumMonths; ++iMonth) {
        if (seasonToMask(iMonth) == 1) {
            finalResult(iMonth) = maxAdjBase(iMonth);
        } else {
            finalResult(iMonth) = baselineVals(iMonth);
        }
    }
    // store the cost in the name of the variable
    s_econ->econVar(usingVariable).values = finalResult;
    // set the flag that it has been evaluated so it won't be evaluated multiple times
    s_econ->econVar(usingVariable).isEvaluated = true;
}

void evaluateQualify(EnergyPlusData &state, int const usingVariable)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    Array1D<Real64> sourceVals(NumMonths);
    Array1D<Real64> thresholdVals(NumMonths);
    Array1D_int monthsQualify(NumMonths);
    Array1D<Real64> seasonMask(NumMonths);
    int adjNumberOfMonths;
    bool isQualified;

    auto &s_econ = state.dataEconTariff;
    auto &econVar = s_econ->econVar(usingVariable);

    int curTariff = econVar.tariffIndx;
    auto &tariff = s_econ->tariff(curTariff);
    int indexInQual = econVar.index;
    auto const &qualify = s_econ->qualify(indexInQual);
    // check the tariff - make sure they match
    if (qualify.namePt != usingVariable) {
        ShowWarningError(state, "UtilityCost:Tariff Debugging issue. Qualify index does not match variable pointer.");
        ShowContinueError(state, format("   Between: {}", econVar.name));
        ShowContinueError(state, format("       And: {}", s_econ->econVar(qualify.namePt).name));
    }
    if (qualify.tariffIndx != curTariff) {
        ShowWarningError(state, "UtilityCost:Tariff Debugging issue. Qualify index does not match tariff index.");
        ShowContinueError(state, format("   Between: {}", tariff.tariffName));
        ShowContinueError(state, format("       And: {}", s_econ->tariff(qualify.tariffIndx).tariffName));
    }
    // data from the Qualify
    sourceVals = s_econ->econVar(qualify.sourcePt).values;
    bool curIsMaximum = qualify.isMaximum;
    bool curIsConsecutive = qualify.isConsecutive;
    int curNumberOfMonths = qualify.numberOfMonths;
    // determine if threshold should be based on variable or value
    if (qualify.thresholdPt != 0) {
        thresholdVals = s_econ->econVar(qualify.thresholdPt).values;
    } else {
        thresholdVals = qualify.thresholdVal;
    }
    // find proper season mask

    if (qualify.season == Season::Summer) {
        seasonMask = s_econ->econVar(tariff.natives[(int)Native::IsSummer]).values;
    } else if (qualify.season == Season::Winter) {
        seasonMask = s_econ->econVar(tariff.natives[(int)Native::IsWinter]).values;
    } else if (qualify.season == Season::Spring) {
        seasonMask = s_econ->econVar(tariff.natives[(int)Native::IsSpring]).values;
    } else if (qualify.season == Season::Fall) {
        seasonMask = s_econ->econVar(tariff.natives[(int)Native::IsAutumn]).values;
    } else if (qualify.season == Season::Annual) {
        seasonMask = 1.0; // all months are 1
    }

    // any months with no energy use are excluded from the qualification process
    for (int iMonth = 1; iMonth <= NumMonths; ++iMonth) {
        if (s_econ->econVar(tariff.natives[(int)Native::TotalEnergy]).values(iMonth) == 0) {
            seasonMask(iMonth) = 0.0;
        }
    }
    // finally perform calculations
    // loop through the months
    int monthsInSeason = 0;
    for (int iMonth = 1; iMonth <= NumMonths; ++iMonth) {
        if (seasonMask(iMonth) == 1) {
            ++monthsInSeason;
            // use threshold as maximum or minimum
            if (curIsMaximum) {
                if (sourceVals(iMonth) > thresholdVals(iMonth)) {
                    monthsQualify(iMonth) = 0; // greater than maximum threshold so it is not qualified
                } else {
                    monthsQualify(iMonth) = 1; // less than maximum threshold so it is qualified
                }
            } else {
                if (sourceVals(iMonth) < thresholdVals(iMonth)) {
                    monthsQualify(iMonth) = 0; // less than minimum threshold so it is not qualified
                } else {
                    monthsQualify(iMonth) = 1; // greater than minimum threshold so it is qualified
                }
            }
        } else {
            monthsQualify(iMonth) = -1; // flag that indicates not part of the season
        }
    }
    // see if the number of months is longer then the number of months and adjust
    if (curNumberOfMonths > monthsInSeason) {
        adjNumberOfMonths = monthsInSeason;
    } else {
        adjNumberOfMonths = curNumberOfMonths;
    }
    // now that each month is qualified or not, depending on the type of test see if the entire qualify pass or not
    int cntAllQualMonths = 0;
    int cntConsecQualMonths = 0;
    int maxConsecQualMonths = 0;
    for (int iMonth = 1; iMonth <= NumMonths; ++iMonth) {
        {
            int const SELECT_CASE_var(monthsQualify(iMonth));
            if (SELECT_CASE_var == 1) { // qualified
                ++cntAllQualMonths;
                ++cntConsecQualMonths;
                // see if the count is greater then the previous count and if it is make it the new count
                if (cntConsecQualMonths > maxConsecQualMonths) {
                    maxConsecQualMonths = cntConsecQualMonths;
                }
            } else if (SELECT_CASE_var == 0) { // not qualified
                // reset the counter on consecutive months
                cntConsecQualMonths = 0;
            }
        }
    }
    // if test is for consecutive months
    if (curIsConsecutive) {
        if (maxConsecQualMonths >= adjNumberOfMonths) {
            isQualified = true;
        } else {
            isQualified = false;
        }
    } else { // count not consecutive
        if (cntAllQualMonths >= adjNumberOfMonths) {
            isQualified = true;
        } else {
            isQualified = false;
        }
    }
    // now update the tariff level qualifier - only update if the tariff is still qualified
    // and the current qualifier fails.
    if (tariff.isQualified) {
        if (!isQualified) {
            tariff.isQualified = false;
            tariff.ptDisqualifier = usingVariable;
        }
    }
    // store the cost in the name of the variable
    econVar.values = monthsQualify;
    // set the flag that it has been evaluated so it won't be evaluated multiple times
    econVar.isEvaluated = true;
}

void addMonthlyCharge(EnergyPlusData &state, int const usingVariable)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    //    Include the monthly charges in the calculations

    auto &s_econ = state.dataEconTariff;
    int curTariff = s_econ->econVar(usingVariable).tariffIndx;
    auto const &tariff = s_econ->tariff(curTariff);
    // check the tariff - make sure they match
    if (tariff.cats[(int)Cat::ServiceCharges] != usingVariable) {
        ShowWarningError(state, "UtilityCost:Tariff Debugging issue. Tariff index for service charge does not match variable pointer.");
        ShowContinueError(state, format("   Between: {}", tariff.tariffName));
        ShowContinueError(state, format("       And: {}", s_econ->tariff(tariff.cats[(int)Cat::ServiceCharges]).tariffName));
    }
    if (tariff.monthChgPt != 0) {
        s_econ->econVar(usingVariable).values += s_econ->econVar(tariff.monthChgPt).values;
    } else {
        s_econ->econVar(usingVariable).values += tariff.monthChgVal;
    }
    // zero out months with no energy consumption
    // curTotalEnergy = tariff.nativeTotalEnergy
    // DO iMonth = 1, NumMonths
    //  IF (s_econ->econVar(curTotalEnergy)%values(iMonth) .EQ. 0) THEN
    //    s_econ->econVar(usingVariable)%values(iMonth) = 0
    //  END IF
    // END DO
}

void checkMinimumMonthlyCharge(EnergyPlusData &state, int const curTariff)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   August 2008

    //    Check if the total is as big as the minimum monthly charge

    auto &s_econ = state.dataEconTariff;
    auto const &tariff = s_econ->tariff(curTariff);

    int totalVar = tariff.cats[(int)Cat::Total];
    int minMonVar = tariff.minMonthChgPt;
    // if a variable is defined use that
    if (minMonVar != 0) {
        for (int iMonth = 1; iMonth <= NumMonths; ++iMonth) {
            if (s_econ->econVar(totalVar).values(iMonth) < s_econ->econVar(minMonVar).values(iMonth)) {
                s_econ->econVar(totalVar).values(iMonth) = s_econ->econVar(minMonVar).values(iMonth);
            }
        }
    } else { // use the constant value
        for (int iMonth = 1; iMonth <= NumMonths; ++iMonth) {
            if (s_econ->econVar(totalVar).values(iMonth) < tariff.minMonthChgVal) {
                s_econ->econVar(totalVar).values(iMonth) = tariff.minMonthChgVal;
            }
        }
    }
}

void setNativeVariables(EnergyPlusData &state)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    //    Set up the "built in" i.e. native variables that hold
    //    the energy and demand from the simulation.

    auto &s_econ = state.dataEconTariff;

    Array1D<Real64> monthVal(NumMonths);
    Real64 bigNumber(0.0); // Autodesk Value not used but suppresses warning about HUGE_() call

    bigNumber = HUGE_(bigNumber);
    for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
        auto &tariff = s_econ->tariff(iTariff);
        // nativeTotalEnergy
        monthVal = 0.0;
        for (int kMonth = 1; kMonth <= NumMonths; ++kMonth) {
            for (int jPeriod = 0; jPeriod < (int)Period::Num; ++jPeriod) {
                monthVal(kMonth) += tariff.gatherEnergy(kMonth)[jPeriod];
            }
        }
        s_econ->econVar(tariff.natives[(int)Native::TotalEnergy]).values = monthVal;
        // nativeTotalDemand
        monthVal = -bigNumber;
        for (int kMonth = 1; kMonth <= NumMonths; ++kMonth) {
            for (int jPeriod = 0; jPeriod < (int)Period::Num; ++jPeriod) {
                if (tariff.gatherDemand(kMonth)[jPeriod] > monthVal(kMonth)) {
                    monthVal(kMonth) = tariff.gatherDemand(kMonth)[jPeriod];
                }
            }
        }
        // if no maximum was set just set to zero
        for (int kMonth = 1; kMonth <= NumMonths; ++kMonth) {
            if (monthVal(kMonth) == -bigNumber) {
                monthVal(kMonth) = 0.0;
            }
        }
        s_econ->econVar(tariff.natives[(int)Native::TotalDemand]).values = monthVal;
        for (int kMonth = 1; kMonth <= NumMonths; ++kMonth) {
            // nativePeakEnergy
            s_econ->econVar(tariff.natives[(int)Native::PeakEnergy]).values(kMonth) = tariff.gatherEnergy(kMonth)[(int)Period::Peak];
            // nativePeakDemand
            s_econ->econVar(tariff.natives[(int)Native::PeakDemand]).values(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::Peak];
            // nativeShoulderEnergy
            s_econ->econVar(tariff.natives[(int)Native::ShoulderEnergy]).values(kMonth) = tariff.gatherEnergy(kMonth)[(int)Period::Shoulder];
            // nativeShoulderDemand
            s_econ->econVar(tariff.natives[(int)Native::ShoulderDemand]).values(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::Shoulder];
            // nativeOffPeakEnergy
            s_econ->econVar(tariff.natives[(int)Native::OffPeakEnergy]).values(kMonth) = tariff.gatherEnergy(kMonth)[(int)Period::OffPeak];
            // nativeOffPeakDemand
            s_econ->econVar(tariff.natives[(int)Native::OffPeakDemand]).values(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::OffPeak];
            // nativeMidPeakEnergy
            s_econ->econVar(tariff.natives[(int)Native::MidPeakEnergy]).values(kMonth) = tariff.gatherEnergy(kMonth)[(int)Period::MidPeak];
            // nativeMidPeakDemand
            s_econ->econVar(tariff.natives[(int)Native::MidPeakDemand]).values(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::MidPeak];
            // nativePeakExceedsOffPeak
            monthVal(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::Peak] - tariff.gatherDemand(kMonth)[(int)Period::OffPeak];
            if (monthVal(kMonth) > 0) {
                s_econ->econVar(tariff.natives[(int)Native::PeakExceedsOffPeak]).values(kMonth) = monthVal(kMonth);
            } else {
                s_econ->econVar(tariff.natives[(int)Native::PeakExceedsOffPeak]).values(kMonth) = 0.0;
            }
            // nativeOffPeakExceedsPeak
            monthVal(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::OffPeak] - tariff.gatherDemand(kMonth)[(int)Period::Peak];
            if (monthVal(kMonth) > 0) {
                s_econ->econVar(tariff.natives[(int)Native::OffPeakExceedsPeak]).values(kMonth) = monthVal(kMonth);
            } else {
                s_econ->econVar(tariff.natives[(int)Native::OffPeakExceedsPeak]).values(kMonth) = 0.0;
            }
            // nativePeakExceedsMidPeak
            monthVal(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::Peak] - tariff.gatherDemand(kMonth)[(int)Period::MidPeak];
            if (monthVal(kMonth) > 0) {
                s_econ->econVar(tariff.natives[(int)Native::PeakExceedsMidPeak]).values(kMonth) = monthVal(kMonth);
            } else {
                s_econ->econVar(tariff.natives[(int)Native::PeakExceedsOffPeak]).values(kMonth) = 0.0;
            }
            // nativeMidPeakExceedsPeak
            monthVal(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::MidPeak] - tariff.gatherDemand(kMonth)[(int)Period::Peak];
            if (monthVal(kMonth) > 0) {
                s_econ->econVar(tariff.natives[(int)Native::MidPeakExceedsPeak]).values(kMonth) = monthVal(kMonth);
            } else {
                s_econ->econVar(tariff.natives[(int)Native::MidPeakExceedsPeak]).values(kMonth) = 0.0;
            }
            // nativePeakExceedsShoulder
            monthVal(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::Peak] - tariff.gatherDemand(kMonth)[(int)Period::Shoulder];
            if (monthVal(kMonth) > 0) {
                s_econ->econVar(tariff.natives[(int)Native::PeakExceedsShoulder]).values(kMonth) = monthVal(kMonth);
            } else {
                s_econ->econVar(tariff.natives[(int)Native::PeakExceedsShoulder]).values(kMonth) = 0.0;
            }
            // nativeShoulderExceedsPeak
            monthVal(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::Shoulder] - tariff.gatherDemand(kMonth)[(int)Period::Peak];
            if (monthVal(kMonth) > 0) {
                s_econ->econVar(tariff.natives[(int)Native::ShoulderExceedsPeak]).values(kMonth) = monthVal(kMonth);
            } else {
                s_econ->econVar(tariff.natives[(int)Native::ShoulderExceedsPeak]).values(kMonth) = 0.0;
            }
            // nativeIsWinter
            // nativeIsNotWinter
            if (tariff.seasonForMonth(kMonth) == Season::Winter) {
                s_econ->econVar(tariff.natives[(int)Native::IsWinter]).values(kMonth) = 1.0;
                s_econ->econVar(tariff.natives[(int)Native::IsNotWinter]).values(kMonth) = 0.0;
            } else {
                s_econ->econVar(tariff.natives[(int)Native::IsWinter]).values(kMonth) = 0.0;
                s_econ->econVar(tariff.natives[(int)Native::IsNotWinter]).values(kMonth) = 1.0;
            }
            // nativeIsSpring
            // nativeIsNotSpring
            if (tariff.seasonForMonth(kMonth) == Season::Spring) {
                s_econ->econVar(tariff.natives[(int)Native::IsSpring]).values(kMonth) = 1.0;
                s_econ->econVar(tariff.natives[(int)Native::IsNotSpring]).values(kMonth) = 0.0;
            } else {
                s_econ->econVar(tariff.natives[(int)Native::IsSpring]).values(kMonth) = 0.0;
                s_econ->econVar(tariff.natives[(int)Native::IsNotSpring]).values(kMonth) = 1.0;
            }
            // nativeIsSummer
            // nativeIsNotSummer
            if (tariff.seasonForMonth(kMonth) == Season::Summer) {
                s_econ->econVar(tariff.natives[(int)Native::IsSummer]).values(kMonth) = 1.0;
                s_econ->econVar(tariff.natives[(int)Native::IsNotSummer]).values(kMonth) = 0.0;
            } else {
                s_econ->econVar(tariff.natives[(int)Native::IsSummer]).values(kMonth) = 0.0;
                s_econ->econVar(tariff.natives[(int)Native::IsNotSummer]).values(kMonth) = 1.0;
            }
            // nativeIsAutumn
            // nativeIsNotAutumn
            if (tariff.seasonForMonth(kMonth) == Season::Fall) {
                s_econ->econVar(tariff.natives[(int)Native::IsAutumn]).values(kMonth) = 1.0;
                s_econ->econVar(tariff.natives[(int)Native::IsNotAutumn]).values(kMonth) = 0.0;
            } else {
                s_econ->econVar(tariff.natives[(int)Native::IsAutumn]).values(kMonth) = 0.0;
                s_econ->econVar(tariff.natives[(int)Native::IsNotAutumn]).values(kMonth) = 1.0;
            }
            // nativePeakAndShoulderEnergy
            s_econ->econVar(tariff.natives[(int)Native::PeakAndShoulderEnergy]).values(kMonth) =
                tariff.gatherEnergy(kMonth)[(int)Period::Peak] + tariff.gatherEnergy(kMonth)[(int)Period::Shoulder];
            // nativePeakAndShoulderDemand
            if (tariff.gatherDemand(kMonth)[(int)Period::Peak] > tariff.gatherDemand(kMonth)[(int)Period::Shoulder]) {
                s_econ->econVar(tariff.natives[(int)Native::PeakAndShoulderDemand]).values(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::Peak];
            } else {
                s_econ->econVar(tariff.natives[(int)Native::PeakAndShoulderDemand]).values(kMonth) =
                    tariff.gatherDemand(kMonth)[(int)Period::Shoulder];
            }
            // nativePeakAndMidPeakEnergy
            s_econ->econVar(tariff.natives[(int)Native::PeakAndMidPeakEnergy]).values(kMonth) =
                tariff.gatherEnergy(kMonth)[(int)Period::Peak] + tariff.gatherEnergy(kMonth)[(int)Period::MidPeak];
            // nativePeakAndMidPeakDemand
            if (tariff.gatherDemand(kMonth)[(int)Period::Peak] > tariff.gatherDemand(kMonth)[(int)Period::MidPeak]) {
                s_econ->econVar(tariff.natives[(int)Native::PeakAndMidPeakDemand]).values(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::Peak];
            } else {
                s_econ->econVar(tariff.natives[(int)Native::PeakAndMidPeakDemand]).values(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::MidPeak];
            }
            // nativeShoulderAndOffPeakEnergy
            s_econ->econVar(tariff.natives[(int)Native::ShoulderAndOffPeakEnergy]).values(kMonth) =
                tariff.gatherEnergy(kMonth)[(int)Period::Shoulder] + tariff.gatherEnergy(kMonth)[(int)Period::OffPeak];
            // nativeShoulderAndOffPeakDemand
            if (tariff.gatherDemand(kMonth)[(int)Period::Shoulder] > tariff.gatherDemand(kMonth)[(int)Period::OffPeak]) {
                s_econ->econVar(tariff.natives[(int)Native::ShoulderAndOffPeakDemand]).values(kMonth) =
                    tariff.gatherDemand(kMonth)[(int)Period::Shoulder];
            } else {
                s_econ->econVar(tariff.natives[(int)Native::ShoulderAndOffPeakDemand]).values(kMonth) =
                    tariff.gatherDemand(kMonth)[(int)Period::OffPeak];
            }
            // nativePeakAndOffPeakEnergy
            s_econ->econVar(tariff.natives[(int)Native::PeakAndOffPeakEnergy]).values(kMonth) =
                tariff.gatherEnergy(kMonth)[(int)Period::Peak] + tariff.gatherEnergy(kMonth)[(int)Period::OffPeak];
            // nativePeakAndOffPeakDemand
            if (tariff.gatherDemand(kMonth)[(int)Period::Peak] > tariff.gatherDemand(kMonth)[(int)Period::OffPeak]) {
                s_econ->econVar(tariff.natives[(int)Native::PeakAndOffPeakDemand]).values(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::Peak];
            } else {
                s_econ->econVar(tariff.natives[(int)Native::PeakAndOffPeakDemand]).values(kMonth) = tariff.gatherDemand(kMonth)[(int)Period::OffPeak];
            }
            // nativeRealTimePriceCosts
            s_econ->econVar(tariff.natives[(int)Native::RealTimePriceCosts]).values(kMonth) = tariff.RTPcost(kMonth);
            // nativeAboveCustomerBaseCosts
            s_econ->econVar(tariff.natives[(int)Native::AboveCustomerBaseCosts]).values(kMonth) = tariff.RTPaboveBaseCost(kMonth);
            // nativeBelowCustomerBaseCosts
            s_econ->econVar(tariff.natives[(int)Native::BelowCustomerBaseCosts]).values(kMonth) = tariff.RTPbelowBaseCost(kMonth);
            // nativeAboveCustomerBaseEnergy
            s_econ->econVar(tariff.natives[(int)Native::AboveCustomerBaseEnergy]).values(kMonth) = tariff.RTPaboveBaseEnergy(kMonth);
            // nativeBelowCustomerBaseEnergy
            s_econ->econVar(tariff.natives[(int)Native::BelowCustomerBaseEnergy]).values(kMonth) = tariff.RTPbelowBaseEnergy(kMonth);
        }
    }
}

void LEEDtariffReporting(EnergyPlusData &state)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   October 2012

    //    Write the economic results for LEED reporting

    Real64 elecTotalEne;
    Real64 gasTotalEne;
    Real64 distCoolTotalEne;
    Real64 distHeatWaterTotalEne;
    Real64 distHeatSteamTotalEne;
    Real64 otherTotalEne;
    Real64 elecTotalCost;
    Real64 gasTotalCost;
    Real64 otherTotalCost;
    Real64 distCoolTotalCost;
    Real64 distHeatWaterTotalCost;
    Real64 distHeatSteamTotalCost;
    Real64 allTotalCost;
    EconConv elecUnits;
    EconConv gasUnits;
    EconConv distCoolUnits;
    EconConv distHeatWaterUnits;
    EconConv distHeatSteamUnits;
    EconConv othrUnits;
    DemandWindow gasDemWindowUnits;
    DemandWindow distCoolDemWindowUnits;
    DemandWindow distHeatWaterDemWindowUnits;
    DemandWindow distHeatSteamDemWindowUnits;
    DemandWindow othrDemWindowUnits;

    auto &s_orp = state.dataOutRptPredefined;

    auto &s_econ = state.dataEconTariff;

    if (s_econ->numTariff > 0) {
        int distCoolFacilMeter = GetMeterIndex(state, "DISTRICTCOOLING:FACILITY");
        int distHeatWaterFacilMeter = GetMeterIndex(state, "DISTRICTHEATINGWATER:FACILITY");
        int distHeatSteamFacilMeter = GetMeterIndex(state, "DISTRICTHEATINGSTEAM:FACILITY");
        elecTotalEne = 0.0;
        gasTotalEne = 0.0;
        distCoolTotalEne = 0.0;
        distHeatWaterTotalEne = 0.0;
        distHeatSteamTotalEne = 0.0;
        otherTotalEne = 0.0;
        elecTotalCost = 0.0;
        gasTotalCost = 0.0;
        distCoolTotalCost = 0.0;
        distHeatWaterTotalCost = 0.0;
        distHeatSteamTotalCost = 0.0;
        otherTotalCost = 0.0;
        allTotalCost = 0.0;
        elecUnits = EconConv::USERDEF;
        gasUnits = EconConv::USERDEF;
        distCoolUnits = EconConv::USERDEF;
        distHeatWaterUnits = EconConv::USERDEF;
        distHeatSteamUnits = EconConv::USERDEF;
        othrUnits = EconConv::USERDEF;
        gasDemWindowUnits = DemandWindow::Invalid;
        othrDemWindowUnits = DemandWindow::Invalid;
        std::string elecTariffNames = "";
        std::string gasTariffNames = "";
        std::string distCoolTariffNames = "";
        std::string distHeatWaterTariffNames = "";
        std::string distHeatSteamTariffNames = "";
        std::string othrTariffNames = "";
        for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
            auto &tariff = s_econ->tariff(iTariff);
            if (tariff.isSelected) {
                allTotalCost += tariff.totalAnnualCost;
                if (tariff.kindMtr == MeterType::ElecSimple || tariff.kindMtr == MeterType::ElecProduced ||
                    tariff.kindMtr == MeterType::ElecPurchased || tariff.kindMtr == MeterType::ElecSurplusSold ||
                    tariff.kindMtr == MeterType::ElecNet) {
                    if (tariff.totalAnnualEnergy > elecTotalEne) {
                        elecTotalEne = tariff.totalAnnualEnergy;
                    }
                    elecTotalCost += tariff.totalAnnualCost;
                    elecTariffNames += ' ' + tariff.tariffName;
                    elecUnits = tariff.convChoice;
                } else if (tariff.kindMtr == MeterType::Gas) {
                    if (tariff.totalAnnualEnergy > gasTotalEne) {
                        gasTotalEne = tariff.totalAnnualEnergy;
                    }
                    gasTotalCost += tariff.totalAnnualCost;
                    gasTariffNames += ' ' + tariff.tariffName;
                    gasUnits = tariff.convChoice;
                    gasDemWindowUnits = tariff.demandWindow;
                } else if (tariff.reportMeterIndx == distCoolFacilMeter) {
                    if (tariff.totalAnnualEnergy > distCoolTotalEne) {
                        distCoolTotalEne = tariff.totalAnnualEnergy;
                    }
                    distCoolTotalCost += tariff.totalAnnualCost;
                    distCoolTariffNames += ' ' + tariff.tariffName;
                    distCoolUnits = tariff.convChoice;
                    distCoolDemWindowUnits = tariff.demandWindow;
                } else if (tariff.reportMeterIndx == distHeatWaterFacilMeter) {
                    if (tariff.totalAnnualEnergy > distHeatWaterTotalEne) {
                        distHeatWaterTotalEne = tariff.totalAnnualEnergy;
                    }
                    distHeatWaterTotalCost += tariff.totalAnnualCost;
                    distHeatWaterTariffNames += ' ' + tariff.tariffName;
                    distHeatWaterUnits = tariff.convChoice;
                    distHeatWaterDemWindowUnits = tariff.demandWindow;
                } else if (tariff.reportMeterIndx == distHeatSteamFacilMeter) {
                    if (tariff.totalAnnualEnergy > distHeatSteamTotalEne) {
                        distHeatSteamTotalEne = tariff.totalAnnualEnergy;
                    }
                    distHeatSteamTotalCost += tariff.totalAnnualCost;
                    distHeatSteamTariffNames += ' ' + tariff.tariffName;
                    distHeatSteamUnits = tariff.convChoice;
                    distHeatSteamDemWindowUnits = tariff.demandWindow;
                } else if (tariff.kindMtr != MeterType::Water) {
                    if (tariff.totalAnnualEnergy > otherTotalEne) {
                        otherTotalEne = tariff.totalAnnualEnergy;
                    }
                    otherTotalCost += tariff.totalAnnualCost;
                    othrTariffNames += ' ' + tariff.tariffName;
                    othrUnits = tariff.convChoice;
                    othrDemWindowUnits = tariff.demandWindow;
                } else {
                }
            }
        }
        // names of the rates
        OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEtsRtNm, "Electricity", elecTariffNames);
        OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEtsRtNm, "Natural Gas", gasTariffNames);
        if (distCoolTotalEne != 0) {
            OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEtsRtNm, "District Cooling", distCoolTariffNames);
        }
        if (distHeatWaterTotalEne != 0) {
            OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEtsRtNm, "District Heating Water", distHeatWaterTariffNames);
        }
        if (distHeatSteamTotalEne != 0) {
            OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEtsRtNm, "District Heating Steam", distHeatSteamTariffNames);
        }
        OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEtsRtNm, "Other", othrTariffNames);
        // virtual rate
        if (elecTotalEne != 0) {
            OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEtsVirt, "Electricity", elecTotalCost / elecTotalEne, 3);
        }
        if (gasTotalEne != 0) {
            OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEtsVirt, "Natural Gas", gasTotalCost / gasTotalEne, 3);
        }
        if (otherTotalEne != 0) {
            OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEtsVirt, "Other", otherTotalCost / otherTotalEne, 3);
        }
        // units
        OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEtsEneUnt, "Electricity", format("{}", convEnergyStrings[(int)elecUnits]));
        OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEtsEneUnt, "Natural Gas", format("{}", convEnergyStrings[(int)gasUnits]));
        OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEtsEneUnt, "Other", format("{}", convEnergyStrings[(int)othrUnits]));
        OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEtsDemUnt, "Electricity", format("{}", convDemandStrings[(int)elecUnits]));
        OutputReportPredefined::PreDefTableEntry(
            state,
            s_orp->pdchLeedEtsDemUnt,
            "Natural Gas",
            format("{}{}",
                   convDemandStrings[(int)gasUnits],
                   (gasDemWindowUnits == DemandWindow::Invalid) ? "" : demandWindowStrings[(int)gasDemWindowUnits]));
        OutputReportPredefined::PreDefTableEntry(
            state,
            s_orp->pdchLeedEtsDemUnt,
            "Other",
            format("{}{}",
                   convDemandStrings[(int)othrUnits],
                   (othrDemWindowUnits == DemandWindow::Invalid) ? "" : demandWindowStrings[(int)othrDemWindowUnits]));

        // total cost
        OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEcsTotal, "Electricity", elecTotalCost, 2);
        OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEcsTotal, "Natural Gas", gasTotalCost, 2);
        OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEcsTotal, "Other", otherTotalCost, 2);
        // show district energy if used
        if (distCoolTotalEne != 0) {
            OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEtsVirt, "District Cooling", distCoolTotalCost / distCoolTotalEne, 3);
            OutputReportPredefined::PreDefTableEntry(
                state, s_orp->pdchLeedEtsEneUnt, "District Cooling", format("{}", convEnergyStrings[(int)distCoolUnits]));
            OutputReportPredefined::PreDefTableEntry(
                state,
                s_orp->pdchLeedEtsDemUnt,
                "District Cooling",
                format("{}{}",
                       convDemandStrings[(int)distCoolUnits],
                       (distCoolDemWindowUnits == DemandWindow::Invalid) ? "" : demandWindowStrings[(int)distCoolDemWindowUnits]));
            OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEcsTotal, "District Cooling", distCoolTotalCost, 2);
        }
        if (distHeatWaterTotalEne != 0) {
            OutputReportPredefined::PreDefTableEntry(
                state, s_orp->pdchLeedEtsVirt, "District Heating Water", distHeatWaterTotalCost / distHeatWaterTotalEne, 3);
            OutputReportPredefined::PreDefTableEntry(
                state, s_orp->pdchLeedEtsEneUnt, "District Heating Water", format("{}", convEnergyStrings[(int)distHeatWaterUnits]));
            OutputReportPredefined::PreDefTableEntry(
                state,
                s_orp->pdchLeedEtsDemUnt,
                "District Heating Water",
                format("{}{}",
                       convDemandStrings[(int)distHeatWaterUnits],
                       (distHeatWaterDemWindowUnits == DemandWindow::Invalid) ? "" : demandWindowStrings[(int)distHeatWaterDemWindowUnits]));
            OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEcsTotal, "District Heating Water", distHeatWaterTotalCost, 2);
        }
        if (distHeatSteamTotalEne != 0) {
            OutputReportPredefined::PreDefTableEntry(
                state, s_orp->pdchLeedEtsVirt, "District Heating Steam", distHeatSteamTotalCost / distHeatSteamTotalEne, 3);
            OutputReportPredefined::PreDefTableEntry(
                state, s_orp->pdchLeedEtsEneUnt, "District Heating Steam", format("{}", convEnergyStrings[(int)distHeatSteamUnits]));
            OutputReportPredefined::PreDefTableEntry(
                state,
                s_orp->pdchLeedEtsDemUnt,
                "District Heating Steam",
                format("{}{}",
                       convDemandStrings[(int)distHeatSteamUnits],
                       (distHeatSteamDemWindowUnits == DemandWindow::Invalid) ? "" : demandWindowStrings[(int)distHeatSteamDemWindowUnits]));
            OutputReportPredefined::PreDefTableEntry(state, s_orp->pdchLeedEcsTotal, "District Heating Steam", distHeatSteamTotalCost, 2);
        }
        // save the total costs for later to compute process fraction
        s_orp->LEEDelecCostTotal = elecTotalCost;
        s_orp->LEEDgasCostTotal = gasTotalCost;
        s_orp->LEEDothrCostTotal = distCoolTotalCost + distHeatWaterTotalCost + distHeatSteamTotalCost + otherTotalCost;
        OutputReportPredefined::PreDefTableEntry(state,
                                                 s_orp->pdchLeedEcsTotal,
                                                 "Total",
                                                 elecTotalCost + gasTotalCost + distCoolTotalCost + distHeatWaterTotalCost + distHeatSteamTotalCost +
                                                     otherTotalCost,
                                                 2);
    }
}

void WriteTabularTariffReports(EnergyPlusData &state)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004
    //    MODIFIED       January 2010, Kyle Benne
    //                   Added SQLite output

    // all arrays are in the format: (row, column)
    Array1D_string columnHead;
    Array1D_int columnWidth;
    Array1D_string rowHead;
    Array2D_string tableBody;
    // other local variables
    Real64 elecTotalCost;
    Real64 gasTotalCost;
    Real64 otherTotalCost;
    Real64 allTotalCost;
    Real64 perAreaUnitConv(0.0);

    auto &s_econ = state.dataEconTariff;

    // Here to it is ready to assign ort->unitStyle_SQLite (not in SQLiteProcedures.cc)
    // when ort->unitsStyle inputs should have been concretely processed and assigned.
    // Included this here to make sure the units specifications are correctly updated.
    if (state.dataOutRptTab->unitsStyle_SQLite == OutputReportTabular::UnitsStyle::NotFound) {
        state.dataOutRptTab->unitsStyle_SQLite = state.dataOutRptTab->unitsStyle; // This is the default UseOutputControlTableStyles
    }

    // compute floor area if no ABUPS
    if (state.dataOutRptTab->buildingConditionedFloorArea == 0.0) {
        OutputReportTabular::DetermineBuildingFloorArea(state);
    }

    if (s_econ->numTariff > 0) {
        auto &econVar = s_econ->econVar;

        if (state.dataOutRptTab->displayEconomicResultSummary) {
            DisplayString(state, "Writing Tariff Reports");
            for (auto &e : econVar) {
                e.isReported = false;
            }
            showWarningsBasedOnTotal(state);
            //---------------------------------
            // Economics Results Summary Report
            //---------------------------------
            OutputReportTabular::WriteReportHeaders(
                state, "Economics Results Summary Report", "Entire Facility", OutputProcessor::StoreType::Average);

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

                // do unit conversions if necessary
                std::string perAreaUnitName;
                if ((unitsStyle_cur == OutputReportTabular::UnitsStyle::InchPound) ||
                    (unitsStyle_cur == OutputReportTabular::UnitsStyle::InchPoundExceptElectricity)) {
                    int unitConvIndex = 0;
                    std::string SIunit = "[~~$~~/m2]";
                    OutputReportTabular::LookupSItoIP(state, SIunit, unitConvIndex, perAreaUnitName);
                    perAreaUnitConv = OutputReportTabular::ConvertIP(state, unitConvIndex, 1.0);
                } else {
                    perAreaUnitName = "[~~$~~/m2]";
                    perAreaUnitConv = 1.0;
                }

                //---- Annual Summary
                rowHead.allocate(3);
                columnHead.allocate(4);
                columnWidth.allocate(4);
                tableBody.allocate(4, 3);
                tableBody = "";
                columnHead(1) = "Electricity";
                columnHead(2) = "Natural Gas";
                columnHead(3) = "Other";
                columnHead(4) = "Total";
                rowHead(1) = "Cost [~~$~~]";
                rowHead(2) = "Cost per Total Building Area " + perAreaUnitName;
                rowHead(3) = "Cost per Net Conditioned Building Area " + perAreaUnitName;
                elecTotalCost = 0.0;
                gasTotalCost = 0.0;
                otherTotalCost = 0.0;
                allTotalCost = 0.0;
                for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
                    auto const &tariff = s_econ->tariff(iTariff);
                    if (tariff.isSelected) {
                        allTotalCost += tariff.totalAnnualCost;
                        if (tariff.kindMtr == MeterType::ElecSimple || tariff.kindMtr == MeterType::ElecProduced ||
                            tariff.kindMtr == MeterType::ElecPurchased || tariff.kindMtr == MeterType::ElecSurplusSold ||
                            tariff.kindMtr == MeterType::ElecNet) {
                            elecTotalCost += tariff.totalAnnualCost;
                        } else if (tariff.kindMtr == MeterType::Gas) {
                            gasTotalCost += tariff.totalAnnualCost;
                        } else if (tariff.kindMtr != MeterType::Water) {
                            otherTotalCost += tariff.totalAnnualCost;
                            // removed because this was confusing        columnHead(3) = tariff.reportMeter
                        }
                    }
                }
                tableBody(1, 1) = OutputReportTabular::RealToStr(elecTotalCost, 2);
                tableBody(2, 1) = OutputReportTabular::RealToStr(gasTotalCost, 2);
                tableBody(3, 1) = OutputReportTabular::RealToStr(otherTotalCost, 2);
                tableBody(4, 1) = OutputReportTabular::RealToStr(allTotalCost, 2);
                if (state.dataOutRptTab->buildingGrossFloorArea > 0.0) {
                    tableBody(1, 2) =
                        OutputReportTabular::RealToStr((elecTotalCost / state.dataOutRptTab->buildingGrossFloorArea) * perAreaUnitConv, 2);
                    tableBody(2, 2) =
                        OutputReportTabular::RealToStr((gasTotalCost / state.dataOutRptTab->buildingGrossFloorArea) * perAreaUnitConv, 2);
                    tableBody(3, 2) =
                        OutputReportTabular::RealToStr((otherTotalCost / state.dataOutRptTab->buildingGrossFloorArea) * perAreaUnitConv, 2);
                    tableBody(4, 2) =
                        OutputReportTabular::RealToStr((allTotalCost / state.dataOutRptTab->buildingGrossFloorArea) * perAreaUnitConv, 2);
                }
                if (state.dataOutRptTab->buildingConditionedFloorArea > 0.0) {
                    tableBody(1, 3) =
                        OutputReportTabular::RealToStr((elecTotalCost / state.dataOutRptTab->buildingConditionedFloorArea) * perAreaUnitConv, 2);
                    tableBody(2, 3) =
                        OutputReportTabular::RealToStr((gasTotalCost / state.dataOutRptTab->buildingConditionedFloorArea) * perAreaUnitConv, 2);
                    tableBody(3, 3) =
                        OutputReportTabular::RealToStr((otherTotalCost / state.dataOutRptTab->buildingConditionedFloorArea) * perAreaUnitConv, 2);
                    tableBody(4, 3) =
                        OutputReportTabular::RealToStr((allTotalCost / state.dataOutRptTab->buildingConditionedFloorArea) * perAreaUnitConv, 2);
                }
                columnWidth = 14; // array assignment - same for all columns
                if (produceTabular) {
                    OutputReportTabular::WriteSubtitle(state, "Annual Cost");
                    OutputReportTabular::WriteTable(state, tableBody, rowHead, columnHead, columnWidth);
                }
                if (produceSQLite) {
                    if (state.dataSQLiteProcedures->sqlite) {
                        state.dataSQLiteProcedures->sqlite->createSQLiteTabularDataRecords(
                            tableBody, rowHead, columnHead, "Economics Results Summary Report", "Entire Facility", "Annual Cost");
                    }
                }
                if (produceTabular) {
                    if (state.dataResultsFramework->resultsFramework->timeSeriesAndTabularEnabled()) {
                        state.dataResultsFramework->resultsFramework->TabularReportsCollection.addReportTable(
                            tableBody, rowHead, columnHead, "Economics Results Summary Report", "Entire Facility", "Annual Cost");
                    }
                }
                columnHead.deallocate();
                rowHead.deallocate();
                columnWidth.deallocate();
                tableBody.deallocate();
            }
            //---- Tariff Summary
            rowHead.allocate(s_econ->numTariff);
            columnHead.allocate(6);
            columnWidth.allocate(6);
            tableBody.allocate(6, s_econ->numTariff);
            tableBody = "";
            columnHead(1) = "Selected";
            columnHead(2) = "Qualified";
            columnHead(3) = "Meter";
            columnHead(4) = "Buy or Sell";
            columnHead(5) = "Group";
            columnHead(6) = "Annual Cost (~~$~~)";
            for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
                auto const &tariff = s_econ->tariff(iTariff);
                rowHead(iTariff) = tariff.tariffName;
                tableBody(1, iTariff) = yesNoNames[(int)tariff.isSelected];
                tableBody(2, iTariff) = yesNoNames[(int)tariff.isQualified];

                tableBody(3, iTariff) = tariff.reportMeter;

                if (tariff.buyOrSell == BuySell::BuyFromUtility) {
                    tableBody(4, iTariff) = "Buy";
                } else if (tariff.buyOrSell == BuySell::SellToUtility) {
                    tableBody(4, iTariff) = "Sell";
                } else if (tariff.buyOrSell == BuySell::NetMetering) {
                    tableBody(4, iTariff) = "Net";
                }

                if (tariff.groupName == "") {
                    tableBody(5, iTariff) = "(none)";
                } else {
                    tableBody(5, iTariff) = tariff.groupName;
                }
                tableBody(6, iTariff) = OutputReportTabular::RealToStr(tariff.totalAnnualCost, 2);
            }
            columnWidth = 14; // array assignment - same for all columns
            OutputReportTabular::WriteSubtitle(state, "Tariff Summary");
            OutputReportTabular::WriteTable(state, tableBody, rowHead, columnHead, columnWidth);
            if (state.dataSQLiteProcedures->sqlite) {
                state.dataSQLiteProcedures->sqlite->createSQLiteTabularDataRecords(
                    tableBody, rowHead, columnHead, "Economics Results Summary Report", "Entire Facility", "Tariff Summary");
            }
            if (state.dataResultsFramework->resultsFramework->timeSeriesAndTabularEnabled()) {
                state.dataResultsFramework->resultsFramework->TabularReportsCollection.addReportTable(
                    tableBody, rowHead, columnHead, "Economics Results Summary Report", "Entire Facility", "Tariff Summary");
            }
            columnHead.deallocate();
            rowHead.deallocate();
            columnWidth.deallocate();
            tableBody.deallocate();
        }
        //---------------------------------
        // Tariff Report
        //---------------------------------
        if (state.dataOutRptTab->displayTariffReport) {
            for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
                auto const &tariff = s_econ->tariff(iTariff);
                auto const &computation = s_econ->computation(iTariff);
                OutputReportTabular::WriteReportHeaders(state, "Tariff Report", tariff.tariffName, OutputProcessor::StoreType::Average);
                rowHead.allocate(7);
                columnHead.allocate(1);
                columnWidth.allocate(1);
                tableBody.allocate(1, 7);
                tableBody = "";
                columnHead(1) = "Parameter";
                rowHead(1) = "Meter";
                rowHead(2) = "Selected";
                rowHead(3) = "Group";
                rowHead(4) = "Qualified";
                rowHead(5) = "Disqualifier";
                rowHead(6) = "Computation";
                rowHead(7) = "Units";
                tableBody(1, 1) = tariff.reportMeter;
                if (tariff.isSelected) {
                    tableBody(1, 2) = "Yes";
                } else {
                    tableBody(1, 2) = "No";
                }
                if (tariff.groupName == "") {
                    tableBody(1, 3) = "(none)";
                } else {
                    tableBody(1, 3) = tariff.groupName;
                }
                if (tariff.isQualified) {
                    tableBody(1, 4) = "Yes";
                } else {
                    tableBody(1, 4) = "No";
                }
                if (tariff.isQualified) {
                    tableBody(1, 5) = "n/a";
                } else {
                    tableBody(1, 5) = econVar(tariff.ptDisqualifier).name;
                }
                if (computation.isUserDef) {
                    tableBody(1, 6) = computation.computeName;
                } else {
                    tableBody(1, 6) = "automatic";
                }
                switch (tariff.convChoice) {
                case EconConv::USERDEF: {
                    tableBody(1, 7) = "User Defined";
                } break;
                case EconConv::KWH: {
                    tableBody(1, 7) = "kWh";
                } break;
                case EconConv::THERM: {
                    tableBody(1, 7) = "Therm";
                } break;
                case EconConv::MMBTU: {
                    tableBody(1, 7) = "MMBtu";
                } break;
                case EconConv::MJ: {
                    tableBody(1, 7) = "MJ";
                } break;
                case EconConv::KBTU: {
                    tableBody(1, 7) = "kBtu";
                } break;
                case EconConv::MCF: {
                    tableBody(1, 7) = "MCF";
                } break;
                case EconConv::CCF: {
                    tableBody(1, 7) = "CCF";
                } break;
                default:
                    break;
                }
                columnWidth = 14; // array assignment - same for all columns
                OutputReportTabular::WriteSubtitle(state, "General");
                OutputReportTabular::WriteTable(state, tableBody, rowHead, columnHead, columnWidth);
                if (state.dataSQLiteProcedures->sqlite) {
                    state.dataSQLiteProcedures->sqlite->createSQLiteTabularDataRecords(
                        tableBody, rowHead, columnHead, "Tariff Report", tariff.tariffName, "General");
                }
                if (state.dataResultsFramework->resultsFramework->timeSeriesAndTabularEnabled()) {
                    state.dataResultsFramework->resultsFramework->TabularReportsCollection.addReportTable(
                        tableBody, rowHead, columnHead, "Tariff Report", tariff.tariffName, "General");
                }
                columnHead.deallocate();
                rowHead.deallocate();
                columnWidth.deallocate();
                tableBody.deallocate();
                //---- Categories
                for (auto &e : econVar) {
                    e.activeNow = false;
                }
                econVar(tariff.cats[(int)Cat::EnergyCharges]).activeNow = true;
                econVar(tariff.cats[(int)Cat::DemandCharges]).activeNow = true;
                econVar(tariff.cats[(int)Cat::ServiceCharges]).activeNow = true;
                econVar(tariff.cats[(int)Cat::Basis]).activeNow = true;
                econVar(tariff.cats[(int)Cat::Adjustment]).activeNow = true;
                econVar(tariff.cats[(int)Cat::Surcharge]).activeNow = true;
                econVar(tariff.cats[(int)Cat::Subtotal]).activeNow = true;
                econVar(tariff.cats[(int)Cat::Taxes]).activeNow = true;
                econVar(tariff.cats[(int)Cat::Total]).activeNow = true;
                ReportEconomicVariable(state, "Categories", false, true, tariff.tariffName);
                //---- Charges
                for (auto &e : econVar) {
                    e.activeNow = false;
                }
                for (int kVar = 1; kVar <= s_econ->numEconVar; ++kVar) {
                    if (econVar(kVar).tariffIndx == iTariff) {
                        if ((econVar(kVar).kindOfObj == ObjType::ChargeSimple) || (econVar(kVar).kindOfObj == ObjType::ChargeBlock)) {
                            econVar(kVar).activeNow = true;
                        }
                    }
                }
                ReportEconomicVariable(state, "Charges", true, true, tariff.tariffName);
                //---- Sources for Charges
                for (auto &e : econVar) {
                    e.activeNow = false;
                }
                for (int kVar = 1; kVar <= s_econ->numEconVar; ++kVar) {
                    if (econVar(kVar).tariffIndx == iTariff) {
                        int indexInChg = econVar(kVar).index;
                        if (econVar(kVar).kindOfObj == ObjType::ChargeSimple) {
                            auto &chargeSimple = s_econ->chargeSimple(indexInChg);
                            if (chargeSimple.sourcePt > 0) {
                                econVar(chargeSimple.sourcePt).activeNow = true;
                            }
                        } else if (econVar(kVar).kindOfObj == ObjType::ChargeBlock) {
                            auto &chargeBlock = s_econ->chargeBlock(indexInChg);
                            if (chargeBlock.sourcePt > 0) {
                                econVar(chargeBlock.sourcePt).activeNow = true;
                            }
                        }
                    }
                }
                ReportEconomicVariable(state, "Corresponding Sources for Charges", false, false, tariff.tariffName);
                //---- Rachets
                for (auto &e : econVar) {
                    e.activeNow = false;
                }
                for (int kVar = 1; kVar <= s_econ->numEconVar; ++kVar) {
                    if (econVar(kVar).tariffIndx == iTariff) {
                        if (econVar(kVar).kindOfObj == ObjType::Ratchet) {
                            econVar(kVar).activeNow = true;
                        }
                    }
                }
                ReportEconomicVariable(state, "Ratchets", false, false, tariff.tariffName);
                //---- Qualifies
                for (auto &e : econVar) {
                    e.activeNow = false;
                }
                for (int kVar = 1; kVar <= s_econ->numEconVar; ++kVar) {
                    if (econVar(kVar).tariffIndx == iTariff) {
                        if (econVar(kVar).kindOfObj == ObjType::Qualify) {
                            econVar(kVar).activeNow = true;
                        }
                    }
                }
                ReportEconomicVariable(state, "Qualifies", false, false, tariff.tariffName);
                //---- Native Variables
                for (auto &e : econVar) {
                    e.activeNow = false;
                }
                for (int kVar = tariff.firstNative; kVar <= tariff.lastNative; ++kVar) {
                    econVar(kVar).activeNow = true;
                }
                ReportEconomicVariable(state, "Native Variables", false, false, tariff.tariffName);
                //---- Other Variables
                for (auto &e : econVar) {
                    e.activeNow = false;
                }
                for (int kVar = 1; kVar <= s_econ->numEconVar; ++kVar) {
                    if (econVar(kVar).tariffIndx == iTariff) {
                        if (!econVar(kVar).isReported) {
                            econVar(kVar).activeNow = true;
                        }
                    }
                }
                ReportEconomicVariable(state, "Other Variables", false, false, tariff.tariffName);
                //---- Computation
                if (computation.isUserDef) {
                    OutputReportTabular::WriteTextLine(state, "Computation -  User Defined", true);
                } else {
                    OutputReportTabular::WriteTextLine(state, "Computation -  Automatic", true);
                }
                std::string outString = "";
                for (int lStep = computation.firstStep; lStep <= computation.lastStep; ++lStep) {
                    auto &step = s_econ->steps(lStep);

                    if (step.type == StepType::EOL) {
                        OutputReportTabular::WriteTextLine(state, rstrip(outString));
                        outString = "";
                    } else if (step.type == StepType::Var) {
                        outString = econVar(step.varNum).name + ' ' + outString;
                    } else if (step.type == StepType::Op) {
                        outString = format("{} {}", opNamesUC[(int)step.op], outString);
                    }
                }
            }
        }
    }
}

void showWarningsBasedOnTotal(EnergyPlusData &state)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    //    Get the annual maximum and sum for the econVariable.

    auto &s_econ = state.dataEconTariff;
    for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
        auto const &tariff = s_econ->tariff(iTariff);
        if (tariff.buyOrSell == BuySell::BuyFromUtility) {
            if (tariff.totalAnnualCost < 0) {
                ShowWarningError(state, "UtilityCost:Tariff: A negative annual total cost when buying electricity from a utility is unusual. ");
                ShowContinueError(state, format("  In UtilityCost:Tariff named {}", tariff.tariffName));
            }
        } else if (tariff.buyOrSell == BuySell::SellToUtility) {
            if (tariff.totalAnnualCost > 0) {
                ShowWarningError(state, "UtilityCost:Tariff: A positive annual total cost when selling electricity to a utility is unusual. ");
                ShowContinueError(state, format("  In UtilityCost:Tariff named {}", tariff.tariffName));
            }
        }
    }
}

void getMaxAndSum(EnergyPlusData &state, int const varPointer, Real64 &sumResult, Real64 &maxResult)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    //    Get the annual maximum and sum for the econVariable.

    Real64 maximumVal(0.0); // Autodesk Value not used but suppresses warning about HUGE_() call

    auto const &s_econ = state.dataEconTariff;
    auto const &econVar = s_econ->econVar(varPointer);

    Real64 sumVal = 0.0;
    maximumVal = -HUGE_(maximumVal);
    for (int jMonth = 1; jMonth <= 12; ++jMonth) { // note not all months get printed out if more than 12 are used.- need to fix this later
        Real64 curVal = econVar.values(jMonth);
        sumVal += curVal;
        if (curVal > maximumVal) {
            maximumVal = curVal;
        }
    }
    sumResult = sumVal;
    maxResult = maximumVal;
}

void ReportEconomicVariable(
    EnergyPlusData &state, std::string const &titleString, bool const includeCategory, bool const showCurrencySymbol, std::string const &forString)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004
    //    MODIFIED       January 2010, Kyle Benne
    //                   Added sqlite output

    //    Report all econVar that show as activeNow

    // all arrays are in the format: (row, column)
    Array1D_string columnHead;
    Array1D_int columnWidth;
    Array1D_string rowHead;
    Array2D_string tableBody;
    Real64 sumVal;
    Real64 maximumVal;
    Real64 curVal;
    int curIndex;
    int curCatPt;
    int curCategory;

    int iVar;
    int jMonth;
    int cntOfVar;
    int nCntOfVar;

    auto const &s_econ = state.dataEconTariff;
    auto const &econVar = s_econ->econVar;
    auto const &chargeBlock = s_econ->chargeBlock;
    auto const &chargeSimple = s_econ->chargeSimple;

    cntOfVar = 0;
    for (iVar = 1; iVar <= s_econ->numEconVar; ++iVar) {
        if (econVar(iVar).activeNow) {
            ++cntOfVar;
        }
    }
    if (includeCategory) {
        rowHead.allocate(cntOfVar);
        columnHead.allocate(15);
        columnWidth.allocate(15);
        tableBody.allocate(15, cntOfVar);
    } else {
        rowHead.allocate(cntOfVar);
        columnHead.allocate(14);
        columnWidth.allocate(14);
        tableBody.allocate(14, cntOfVar);
    }
    // column names
    columnHead(1) = "Jan";
    columnHead(2) = "Feb";
    columnHead(3) = "Mar";
    columnHead(4) = "Apr";
    columnHead(5) = "May";
    columnHead(6) = "Jun";
    columnHead(7) = "Jul";
    columnHead(8) = "Aug";
    columnHead(9) = "Sep";
    columnHead(10) = "Oct";
    columnHead(11) = "Nov";
    columnHead(12) = "Dec";
    columnHead(13) = "Sum";
    columnHead(14) = "Max";
    if (includeCategory) {
        columnHead(15) = "Category";
    }
    nCntOfVar = 0;
    // row names
    for (iVar = 1; iVar <= s_econ->numEconVar; ++iVar) {
        if (econVar(iVar).activeNow) {
            ++nCntOfVar;
            if (showCurrencySymbol) {
                rowHead(nCntOfVar) = econVar(iVar).name + " (~~$~~)";
            } else {
                rowHead(nCntOfVar) = econVar(iVar).name;
            }
        }
    }
    // fill the body
    nCntOfVar = 0;
    for (iVar = 1; iVar <= s_econ->numEconVar; ++iVar) {
        if (econVar(iVar).activeNow) {
            ++nCntOfVar;
            for (jMonth = 1; jMonth <= 12; ++jMonth) { // note not all months get printed out if more than 12 are used.- need to fix this later
                curVal = econVar(iVar).values(jMonth);
                if ((curVal > 0) && (curVal < 1)) {
                    tableBody(jMonth, nCntOfVar) = OutputReportTabular::RealToStr(curVal, 4);
                } else {
                    tableBody(jMonth, nCntOfVar) = OutputReportTabular::RealToStr(curVal, 2);
                }
            }
            getMaxAndSum(state, iVar, sumVal, maximumVal);
            tableBody(13, nCntOfVar) = OutputReportTabular::RealToStr(sumVal, 2);
            tableBody(14, nCntOfVar) = OutputReportTabular::RealToStr(maximumVal, 2);
            if (includeCategory) {
                // first find category
                curCategory = 0;
                curIndex = econVar(iVar).index;

                switch (econVar(iVar).kindOfObj) {
                case ObjType::ChargeSimple:
                    if ((curIndex >= 1) && (curIndex <= s_econ->numChargeSimple)) {
                        curCatPt = chargeSimple(curIndex).categoryPt;
                    }
                    break;
                case ObjType::ChargeBlock:
                    if ((curIndex >= 1) && (curIndex <= s_econ->numChargeBlock)) {
                        curCatPt = chargeBlock(curIndex).categoryPt;
                    }
                    break;
                default:
                    break;
                }

                if ((curCatPt >= 1) && (curCatPt <= s_econ->numEconVar)) {
                    curCategory = econVar(curCatPt).specific;
                }

                // In this specific table, "NotIncluded" is written as "none" so need a special case for that
                tableBody(15, nCntOfVar) =
                    (econVar(curCatPt).kindOfObj == ObjType::Category)
                        ? ((econVar(curCatPt).specific == (int)Cat::NotIncluded) ? "none" : catNames[econVar(curCatPt).specific])
                        : "none";
            }
            s_econ->econVar(iVar).isReported = true;
        }
    }
    columnWidth = 14; // array assignment - same for all columns
    OutputReportTabular::WriteSubtitle(state, titleString);
    OutputReportTabular::WriteTable(state, tableBody, rowHead, columnHead, columnWidth);
    if (state.dataSQLiteProcedures->sqlite) {
        state.dataSQLiteProcedures->sqlite->createSQLiteTabularDataRecords(tableBody, rowHead, columnHead, "Tariff Report", forString, titleString);
    }
    if (state.dataResultsFramework->resultsFramework->timeSeriesAndTabularEnabled()) {
        state.dataResultsFramework->resultsFramework->TabularReportsCollection.addReportTable(
            tableBody, rowHead, columnHead, "Tariff Report", forString, titleString);
    }
    columnHead.deallocate();
    rowHead.deallocate();
    columnWidth.deallocate();
    tableBody.deallocate();
}

void selectTariff(EnergyPlusData &state)
{
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   July 2004

    //    To select tariffs for each combination of meter and
    //    group.  If multiple tariffs have the same meter and
    //    group, then select the one with the lowest cost.
    //    For electric tariffs, since they may have buy, sell, or
    //    netmetering, they need to be combined more carefully.
    //    Multiple meters are used but buy + sell might be more or
    //    less expensive than netmeter.

    Array1D_int groupIndex;     // index number (in tariff) for the group name
    Array1D_int MinTariffIndex; // tariff index for the Minimum value
    int curMinTariffIndex;

    auto const &s_econ = state.dataEconTariff;
    auto const &econVar(s_econ->econVar);

    groupIndex.dimension(s_econ->numTariff, 0);
    int groupCount = 0;
    int numMins = 0;
    MinTariffIndex.dimension(s_econ->numTariff, 0);
    for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
        auto &tariff = s_econ->tariff(iTariff);
        // compute the total annual cost of each tariff
        int totalVarPt = tariff.cats[(int)Cat::Total];
        int totEneVarPt = tariff.natives[(int)Native::TotalEnergy];
        Real64 annualTotal = 0.0;
        Real64 annEneTotal = 0.0;
        for (int jMonth = 1; jMonth <= NumMonths; ++jMonth) {
            annualTotal += econVar(totalVarPt).values(jMonth);
            annEneTotal += econVar(totEneVarPt).values(jMonth);
        }
        tariff.totalAnnualCost = annualTotal;
        tariff.totalAnnualEnergy = annEneTotal;
        // Set the groupIndex
        if (groupIndex(iTariff) == 0) {
            // set the current item to the tariff index
            ++groupCount;
            groupIndex(iTariff) = groupCount;
            // set all remaining matching items to the same index
            for (int kTariff = iTariff + 1; kTariff <= s_econ->numTariff; ++kTariff) {
                if (Util::SameString(s_econ->tariff(kTariff).groupName, tariff.groupName)) {
                    groupIndex(kTariff) = groupCount;
                }
            }
        }
    }
    // First process the all tariff and identify the lowest cost for each type of meter and group.
    for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
        auto &tariff = s_econ->tariff(iTariff);
        if (tariff.isQualified) {
            bool isFound = false;
            for (int lMin = 1; lMin <= numMins; ++lMin) {
                curMinTariffIndex = MinTariffIndex(lMin);
                // find matching meter and group
                if (tariff.reportMeterIndx == s_econ->tariff(curMinTariffIndex).reportMeterIndx) {
                    if (groupIndex(iTariff) == groupIndex(curMinTariffIndex)) {
                        isFound = true;
                        // found the matching mater and group now test if smaller Min is current tariff
                        if (tariff.totalAnnualCost < s_econ->tariff(curMinTariffIndex).totalAnnualCost) {
                            MinTariffIndex(lMin) = iTariff;
                            // select the new Minimum tariff and deselect the one that was just exceeded
                            s_econ->tariff(curMinTariffIndex).isSelected = false;
                            tariff.isSelected = true;
                        }
                    }
                }
            }
            if (!isFound) {
                ++numMins;
                if (numMins > s_econ->numTariff) {
                    ShowWarningError(state, "UtilityCost:Tariff Debugging error numMins greater than numTariff.");
                }
                MinTariffIndex(numMins) = iTariff;
                // tariff(numMins)%isSelected = .TRUE.  !original
                tariff.isSelected = true; // BTG changed 2/7/2005     CR6573
            }
        }
    }
    // Now select for the electric meters. If electric buying and selling and netmetering all are going
    // on, need to determine which combination should be selected. Within each group select just one set
    // of electric results.  The electric results can be either the buy rate only, the buy rate plus the
    // sell rate, or the netmetering rate, whichever of these three is the lowest combination.
    // (The kindElectricMtr was assigned in GetInputEconomicsTariff)
    for (int mGroup = 1; mGroup <= groupCount; ++mGroup) {
        int lowestSimpleTariff = 0;
        int lowestPurchaseTariff = 0;
        int lowestSurplusSoldTariff = 0;
        int lowestNetMeterTariff = 0;
        for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
            auto &tariff = s_econ->tariff(iTariff);
            if (tariff.isQualified) {
                if (tariff.isSelected) {
                    if (groupIndex(iTariff) == mGroup) {
                        if (tariff.kindMtr == MeterType::ElecSimple) {
                            lowestSimpleTariff = iTariff;
                        } else if (tariff.kindMtr == MeterType::ElecProduced) {
                            // don't show electric produced rates as ever selected since surplus sold is more relevant
                            tariff.isSelected = false;
                        } else if (tariff.kindMtr == MeterType::ElecPurchased) {
                            lowestPurchaseTariff = iTariff;
                        } else if (tariff.kindMtr == MeterType::ElecSurplusSold) {
                            lowestSurplusSoldTariff = iTariff;
                        } else if (tariff.kindMtr == MeterType::ElecNet) {
                            lowestNetMeterTariff = iTariff;
                        }
                    }
                }
            }
        }
        // compare the simple and purchased metered tariffs
        if ((lowestSimpleTariff > 0) && (lowestPurchaseTariff > 0)) {
            if (s_econ->tariff(lowestSimpleTariff).totalAnnualCost < s_econ->tariff(lowestPurchaseTariff).totalAnnualCost) {
                s_econ->tariff(lowestPurchaseTariff).isSelected = false;
                lowestPurchaseTariff = 0;
            } else {
                s_econ->tariff(lowestSimpleTariff).isSelected = false;
                lowestSimpleTariff = 0;
            }
        }
        // if surplus sold is negative use it otherwise don't
        if (lowestSurplusSoldTariff > 0) {
            if (s_econ->tariff(lowestSurplusSoldTariff).totalAnnualCost > 0) {
                s_econ->tariff(lowestSurplusSoldTariff).isSelected = false;
                lowestSurplusSoldTariff = 0;
            }
        }
        // if netmetering is used compare it to simple plus surplus
        if (((lowestNetMeterTariff > 0) && (lowestSurplusSoldTariff > 0)) && (lowestSimpleTariff > 0)) {
            if (s_econ->tariff(lowestNetMeterTariff).totalAnnualCost <
                (s_econ->tariff(lowestSimpleTariff).totalAnnualCost + s_econ->tariff(lowestSurplusSoldTariff).totalAnnualCost)) {
                s_econ->tariff(lowestSimpleTariff).isSelected = false;
                lowestSimpleTariff = 0;
                s_econ->tariff(lowestSurplusSoldTariff).isSelected = false;
                lowestSurplusSoldTariff = 0;
            } else {
                s_econ->tariff(lowestNetMeterTariff).isSelected = false;
                lowestNetMeterTariff = 0;
            }
        }
        // if netmetering is used compare it to purchased plus surplus
        if (((lowestNetMeterTariff > 0) && (lowestSurplusSoldTariff > 0)) && (lowestPurchaseTariff > 0)) {
            if (s_econ->tariff(lowestNetMeterTariff).totalAnnualCost <
                (s_econ->tariff(lowestPurchaseTariff).totalAnnualCost + s_econ->tariff(lowestSurplusSoldTariff).totalAnnualCost)) {
                s_econ->tariff(lowestPurchaseTariff).isSelected = false;
                lowestPurchaseTariff = 0;
                s_econ->tariff(lowestSurplusSoldTariff).isSelected = false;
                // lowestSurplusSoldTariff = 0; // not used after this point
            } else {
                s_econ->tariff(lowestNetMeterTariff).isSelected = false;
                lowestNetMeterTariff = 0;
            }
        }
        // if netmetering is used compare it to simple only
        if ((lowestNetMeterTariff > 0) && (lowestSimpleTariff > 0)) {
            if (s_econ->tariff(lowestNetMeterTariff).totalAnnualCost < s_econ->tariff(lowestSimpleTariff).totalAnnualCost) {
                s_econ->tariff(lowestSimpleTariff).isSelected = false;
                // lowestSimpleTariff = 0; // not used after this point
            } else {
                s_econ->tariff(lowestNetMeterTariff).isSelected = false;
                lowestNetMeterTariff = 0;
            }
        }
        // if netmetering is used compare it to purchased only
        if ((lowestNetMeterTariff > 0) && (lowestPurchaseTariff > 0)) {
            if (s_econ->tariff(lowestNetMeterTariff).totalAnnualCost < s_econ->tariff(lowestPurchaseTariff).totalAnnualCost) {
                s_econ->tariff(lowestPurchaseTariff).isSelected = false;
                // lowestPurchaseTariff = 0; // not used after this point
            } else {
                s_econ->tariff(lowestNetMeterTariff).isSelected = false;
                // lowestNetMeterTariff = 0; // not used after this point
            }
        }
    }
    groupIndex.deallocate();
    MinTariffIndex.deallocate();
}

void GetMonthlyCostForResource(EnergyPlusData &state, Constant::eResource const inResourceNumber, Array1A<Real64> outMonthlyCosts)
{
    //       AUTHOR         Jason Glazer
    //       DATE WRITTEN   May 2010

    //  Return the total annual cost for a given resource number.

    // Argument array dimensioning
    auto const &s_econ = state.dataEconTariff;

    outMonthlyCosts.dim(12);

    outMonthlyCosts = 0.0;
    for (int iTariff = 1; iTariff <= s_econ->numTariff; ++iTariff) {
        auto const &tariff = s_econ->tariff(iTariff);
        if (tariff.isSelected) {
            if (tariff.resource == inResourceNumber) {
                auto const &econVar = s_econ->econVar(tariff.cats[(int)Cat::Total]);
                for (int jMonth = 1; jMonth <= 12; ++jMonth) { // use 12 because LCC assume 12 months
                    outMonthlyCosts(jMonth) += econVar.values(jMonth);
                }
            }
        }
    }
}

} // namespace EnergyPlus::EconomicTariff
