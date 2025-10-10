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

// ObjexxFCL Headers

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PollutionModule.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::Pollution {
// Module containing the pollution calculation routines

// MODULE INFORMATION:
//       AUTHOR         Richard J. Liesen (RJL)
//       DATE WRITTEN   August 2002
//       MODIFIED       January 17, 2004 - J Glazer - Added source energy support including schedules for source energy
//                      January 2008 - L Lawrie - implementing schedule fields for all emission factors.
//       RE-ENGINEERED  December 2003 RJL

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms required to
// calculate the pollution, and carbon eqiuvalent for the Energy consumed

// METHODOLOGY EMPLOYED:
// The methodology employed is to calculate the
// source pollution from building energy consumption.
//    PURPOSE:= Takes the Energy from the various sources and
//               calculates the Environmental Impact Factors.
//         STEP 1:  We begin with the output expressing the energy
//         STEP 2:  The energy used by types: must be converted back
//         to source fuel types (fossil or electricity) via User Input.
//         STEP 3:  All energy numbers have been converted to units of MJ's or 1x10^6 Joules.
//         STEP 4:  Environmental Impact Factors are calculated from Coefficients

// MODULE VARIABLE DECLARATIONS:
// Total for all of the Pollutants
// Total Carbon Equivalent Components
//  !Fuel Types
// Total Carbon Equivalent Coeffs
// Purchased Efficiencies

// Fuel Types used with the Pollution Factors
// Facility Meter Indexes
// Facility Meter Values used in Pollution Calcs

void CalculatePollution(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   August 2002
    //       MODIFIED       na
    //       RE-ENGINEERED  December 2003 RJL

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is the main driver for the pollution calculation

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger events.

    if (!state.dataPollution->PollutionReportSetup) {
        return;
    }

    //   Call the Routine to Read the Energy Values from the EnergyPlus Meters
    ReadEnergyMeters(state);

    //   Call the routine that takes the fuel data and calculates the
    //     Pollution for each fuel type.
    CalcPollution(state);
}

// Get Input Section of the Module
//******************************************************************************

void SetupPollutionCalculations(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   August 2002
    //       MODIFIED       na
    //       RE-ENGINEERED  December 2003 RJL; August 2008 LKL - more standard getinput

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is the input routines and Get routines

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger events.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumPolluteRpt;
    int NumAlphas;
    int NumNums;
    int Loop;
    int IOStat;
    auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;

    // First determine if the Pollution reporting has been triggered, and is not exit.
    cCurrentModuleObject = "Output:EnvironmentalImpactFactors";
    NumPolluteRpt = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    state.dataPollution->PollutionReportSetup = true;

    for (Loop = 1; Loop <= NumPolluteRpt; ++Loop) {

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 Loop,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 state.dataIPShortCut->lNumericFieldBlanks,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        // Call this routine in the Output Processor to setup the correct Facility energy meters that are
        //  necessary to make sure that the Meter file is opened and written to by the OP so that time stamps
        //  and the like are happening as expected.
        OutputProcessor::ReportFreq freq = OutputProcessor::ReportFreq::Simulation;

        if (!state.dataIPShortCut->lAlphaFieldBlanks(1) &&
            (freq = static_cast<OutputProcessor::ReportFreq>(
                 getEnumValue(OutputProcessor::reportFreqNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(1))))) ==
                OutputProcessor::ReportFreq::Invalid) {
            ShowSevereError(state, format("Invalid reporting frequency {}", state.dataIPShortCut->cAlphaArgs(1)));
            continue;
        }

        InitPollutionMeterReporting(state, freq);
    }
}

void GetPollutionFactorInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   August 2008

    // PURPOSE OF THIS SUBROUTINE:
    // SetupPollutionCalculation must be called after meters are initialized.  This caused a problem
    // in runs so have added this routine to allow central get for most inputs.

    constexpr std::string_view routineName = "GetPollutionFactorInput";

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumAlphas;
    int NumNums;
    int IOStat;
    bool ErrorsFound = false;

    auto &ip = state.dataInputProcessing->inputProcessor;
    auto &ipsc = state.dataIPShortCut;
    auto &pm = state.dataPollution;

    if (!pm->GetInputFlagPollution) {
        return; // Input already gotten
    }
    pm->GetInputFlagPollution = false;

    ipsc->cCurrentModuleObject = "EnvironmentalImpactFactors";
    pm->NumEnvImpactFactors = ip->getNumObjectsFound(state, ipsc->cCurrentModuleObject);

    if (pm->NumEnvImpactFactors > 0) {
        // Now find and load all of the user inputs and factors.
        ip->getObjectItem(state,
                          ipsc->cCurrentModuleObject,
                          1,
                          ipsc->cAlphaArgs,
                          NumAlphas,
                          ipsc->rNumericArgs,
                          NumNums,
                          IOStat,
                          ipsc->lNumericFieldBlanks,
                          ipsc->lAlphaFieldBlanks,
                          ipsc->cAlphaFieldNames,
                          ipsc->cNumericFieldNames);
    } else if (pm->PollutionReportSetup) {
        ShowWarningError(state, format("{}: not entered.  Values will be defaulted.", ipsc->cCurrentModuleObject));
    }

    pm->PurchHeatEffic = 0.3;
    pm->PurchCoolCOP = 3.0;
    pm->SteamConvEffic = 0.25;
    pm->CarbonEquivN2O = 0.0;
    pm->CarbonEquivCH4 = 0.0;
    pm->CarbonEquivCO2 = 0.0;

    if (pm->NumEnvImpactFactors > 0) {
        // If Heating Efficiency defined by the User is negative or zero then a default of 30% will be assigned.
        if (ipsc->rNumericArgs(1) > 0.0) {
            pm->PurchHeatEffic = ipsc->rNumericArgs(1);
        }

        // If COP defined by the User is negative or zero then a default of 3.0 will be assigned.
        if (ipsc->rNumericArgs(2) > 0.0) {
            pm->PurchCoolCOP = ipsc->rNumericArgs(2);
        }

        // If Steam Conversion Efficiency defined by the User is negative or zero then a default of 25% will be assigned.
        if (ipsc->rNumericArgs(3) > 0.0) {
            pm->SteamConvEffic = ipsc->rNumericArgs(3);
        }

        // Load the Total Carbon Equivalent Pollution Factor coefficients
        pm->CarbonEquivN2O = ipsc->rNumericArgs(4);
        pm->CarbonEquivCH4 = ipsc->rNumericArgs(5);
        pm->CarbonEquivCO2 = ipsc->rNumericArgs(6);
    }

    // Compare all of the Fuel Factors and compare to PollutionCalculationFactors List
    ipsc->cCurrentModuleObject = "FuelFactors";
    pm->NumFuelFactors = ip->getNumObjectsFound(state, ipsc->cCurrentModuleObject);

    for (int Loop = 1; Loop <= state.dataPollution->NumFuelFactors; ++Loop) {
        // Now find and load all of the user inputs and factors.
        ip->getObjectItem(state,
                          ipsc->cCurrentModuleObject,
                          Loop,
                          ipsc->cAlphaArgs,
                          NumAlphas,
                          ipsc->rNumericArgs,
                          NumNums,
                          IOStat,
                          ipsc->lNumericFieldBlanks,
                          ipsc->lAlphaFieldBlanks,
                          ipsc->cAlphaFieldNames,
                          ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)};

        PollFuel pollFuel = static_cast<PollFuel>(getEnumValue(pollFuelNamesUC, Util::makeUPPER(ipsc->cAlphaArgs(1))));
        if (pollFuel == PollFuel::Invalid) {
            ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(1), ipsc->cAlphaArgs(1));
            ErrorsFound = true;
            continue;
        }

        pm->pollFuelFactorList.push_back(pollFuel);

        auto &pollCoeff = pm->pollCoeffs[(int)pollFuel];
        Constant::eFuel fuel = pollFuel2fuel[(int)pollFuel];

        if (pollCoeff.used) {
            ShowWarningError(
                state, format("{}: {} already entered. Previous entry will be used.", ipsc->cCurrentModuleObject, Constant::eFuelNames[(int)fuel]));
            continue;
        }

        pollCoeff.used = true;

        pollCoeff.sourceCoeff = ipsc->rNumericArgs(1);
        if (ipsc->lAlphaFieldBlanks(2)) {
        } else if ((pollCoeff.sourceSched = Sched::GetSchedule(state, ipsc->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2));
            ErrorsFound = true;
        } else if (!pollCoeff.sourceSched->checkMinVal(state, Clusive::In, 0.0)) {
            Sched::ShowSevereBadMin(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2), Clusive::In, 0.0);
            ErrorsFound = true;
        }

        for (int iPollutant = 0; iPollutant < (int)Pollutant::Num; ++iPollutant) {
            pollCoeff.pollutantCoeffs[iPollutant] = ipsc->rNumericArgs(iPollutant + 2);
            if (ipsc->lAlphaFieldBlanks(iPollutant + 3)) {
            } else if ((pollCoeff.pollutantScheds[iPollutant] = Sched::GetSchedule(state, ipsc->cAlphaArgs(iPollutant + 3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(iPollutant + 3), ipsc->cAlphaArgs(iPollutant + 3));
                ErrorsFound = true;
            } else if (!pollCoeff.pollutantScheds[iPollutant]->checkMinVal(state, Clusive::In, 0.0)) {
                Sched::ShowSevereBadMin(state, eoh, ipsc->cAlphaFieldNames(iPollutant + 3), ipsc->cAlphaArgs(iPollutant + 3), Clusive::In, 0.0);
                ErrorsFound = true;
            }
        } // for (iPollutant)

    } // End of the NumEnergyTypes Do Loop

    if (pm->PollutionReportSetup) { // only do this if reporting on the pollution
        // Need to go through all of the Fuel Types and make sure a Fuel Factor was found for each type of energy being simulated
        // Check for Electricity
        if (!pm->pollCoeffs[(int)PollFuel::Electricity].used && ((pm->facilityMeterNums[(int)PollFacilityMeter::Electricity] > 0) ||
                                                                 (pm->facilityMeterNums[(int)PollFacilityMeter::ElectricityProduced] > 0) ||
                                                                 (pm->facilityMeterNums[(int)PollFacilityMeter::CoolPurchased] > 0))) {
            ShowSevereError(state,
                            format("{} Not Found or Fuel not specified For Pollution Calculation for ELECTRICITY", ipsc->cCurrentModuleObject));
            ErrorsFound = true;
        }

        // Check for Natural Gas
        if (!pm->pollCoeffs[(int)PollFuel::NaturalGas].used &&
            ((pm->facilityMeterNums[(int)PollFacilityMeter::NaturalGas] > 0) || (pm->facilityMeterNums[(int)PollFacilityMeter::HeatPurchased] > 0) ||
             (pm->facilityMeterNums[(int)PollFacilityMeter::Steam] > 0))) {
            ShowSevereError(state,
                            format("{} Not Found or Fuel not specified For Pollution Calculation for NATURAL GAS", ipsc->cCurrentModuleObject));
            ErrorsFound = true;
        }
        // Check for FuelOilNo2 (Residual Oil)
        if (!pm->pollCoeffs[(int)PollFuel::FuelOil2].used && (pm->facilityMeterNums[(int)PollFacilityMeter::FuelOil2] > 0)) {
            ShowSevereError(state,
                            format("{} Not Found or Fuel not specified For Pollution Calculation for FUEL OIL #2", ipsc->cCurrentModuleObject));
            ErrorsFound = true;
        }
        // Check for FuelOilNo1 (Distillate Oil)
        if (!pm->pollCoeffs[(int)PollFuel::FuelOil1].used && (pm->facilityMeterNums[(int)PollFacilityMeter::FuelOil1] > 0)) {
            ShowSevereError(state,
                            format("{} Not Found or Fuel not specified For Pollution Calculation for FUEL OIL #1", ipsc->cCurrentModuleObject));
            ErrorsFound = true;
        }
        // Check for Coal
        if (!pm->pollCoeffs[(int)PollFuel::Coal].used && (pm->facilityMeterNums[(int)PollFacilityMeter::Coal] > 0)) {
            ShowSevereError(state, format("{} Not Found or Fuel not specified For Pollution Calculation for COAL", ipsc->cCurrentModuleObject));
            ErrorsFound = true;
        }
        // Check for Gasoline
        if (!pm->pollCoeffs[(int)PollFuel::Gasoline].used && (pm->facilityMeterNums[(int)PollFacilityMeter::Gasoline] > 0)) {
            ShowSevereError(state, format("{} Not Found or Fuel not specified For Pollution Calculation for GASOLINE", ipsc->cCurrentModuleObject));
            ErrorsFound = true;
        }
        // Check for Propane
        if (!pm->pollCoeffs[(int)PollFuel::Propane].used && (pm->facilityMeterNums[(int)PollFacilityMeter::Propane] > 0)) {
            ShowSevereError(state, format("{} Not Found or Fuel not specified For Pollution Calculation for PROPANE", ipsc->cCurrentModuleObject));
            ErrorsFound = true;
        }
        // Check for Diesel
        if (!pm->pollCoeffs[(int)PollFuel::Diesel].used && (pm->facilityMeterNums[(int)PollFacilityMeter::Diesel] > 0)) {
            ShowSevereError(state, format("{} Not Found or Fuel not specified For Pollution Calculation for DIESEL", ipsc->cCurrentModuleObject));
            ErrorsFound = true;
        }
        // Check for OtherFuel1
        if (!pm->pollCoeffs[(int)PollFuel::OtherFuel1].used && (pm->facilityMeterNums[(int)PollFacilityMeter::OtherFuel1] > 0)) {
            ShowSevereError(state, format("{} Not Found or Fuel not specified For Pollution Calculation for OTHERFUEL1", ipsc->cCurrentModuleObject));
            ErrorsFound = true;
        }
        // Check for OtherFuel2
        if (!pm->pollCoeffs[(int)PollFuel::OtherFuel2].used && (pm->facilityMeterNums[(int)PollFacilityMeter::OtherFuel2] > 0)) {
            ShowSevereError(state, format("{} Not Found or Fuel not specified For Pollution Calculation for OTHERFUEL2", ipsc->cCurrentModuleObject));
            ErrorsFound = true;
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Errors found in getting Pollution Calculation Reporting Input");
    }
}

void SetupPollutionMeterReporting(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   August 2002
    //       MODIFIED       na
    //       RE-ENGINEERED  December 2003 RJL

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is the input routines and Get routines

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger events.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &pm = state.dataPollution;

    if (pm->GetInputFlagPollution) {
        GetPollutionFactorInput(state);
        pm->GetInputFlagPollution = false;
    }

    // We are using this list rather than the enumeration to preserve the order in which meters are created to avoid ordering diffs.
    for (PollFuel pollFuel : pm->pollFuelFactorList) {

        if (!pm->pollCoeffs[(int)pollFuel].used) {
            continue;
        }

        auto &pollComp = pm->pollComps[(int)pollFuel2pollFuelComponent[(int)pollFuel]];

        Constant::eFuel fuel = pollFuel2fuel[(int)pollFuel];

        constexpr std::array<OutputProcessor::EndUseCat, (int)Constant::eFuel::Num> fuel2sovEndUseCat = {
            OutputProcessor::EndUseCat::ElectricityEmissions,
            OutputProcessor::EndUseCat::NaturalGasEmissions,
            OutputProcessor::EndUseCat::GasolineEmissions,
            OutputProcessor::EndUseCat::DieselEmissions,
            OutputProcessor::EndUseCat::CoalEmissions,
            OutputProcessor::EndUseCat::PropaneEmissions,
            OutputProcessor::EndUseCat::FuelOilNo1Emissions,
            OutputProcessor::EndUseCat::FuelOilNo2Emissions,
            OutputProcessor::EndUseCat::OtherFuel1Emissions,
            OutputProcessor::EndUseCat::OtherFuel2Emissions,
            OutputProcessor::EndUseCat::Invalid,
            OutputProcessor::EndUseCat::Invalid,
            OutputProcessor::EndUseCat::Invalid,
            OutputProcessor::EndUseCat::Invalid,
            OutputProcessor::EndUseCat::Invalid // used for OtherEquipment object
        };

        // Need to check whether this fuel is used?
        SetupOutputVariable(state,
                            format("Environmental Impact {} Source Energy", Constant::eFuelNames[(int)fuel]),
                            Constant::Units::J,
                            pollComp.sourceVal,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            "Site",
                            Constant::eResource::Source,
                            OutputProcessor::Group::Invalid,
                            fuel2sovEndUseCat[(int)fuel]);

        for (int iPollutant = 0; iPollutant < (int)Pollutant::Num; ++iPollutant) {
            SetupOutputVariable(state,
                                format("Environmental Impact {} {}", Constant::eFuelNames[(int)fuel], poll2outVarStrs[iPollutant]),
                                pollUnits[iPollutant],
                                pollComp.pollutantVals[iPollutant],
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                "Site",
                                poll2Resource[iPollutant],
                                OutputProcessor::Group::Invalid,
                                fuel2sovEndUseCat[(int)fuel]);
        }

        if (fuel == Constant::eFuel::Electricity) {
            // Setup ElectricityPurchased and ElectricitySold variables
            // Doing this here as opposed to outside the outer loop to preserve meter order and reduce ordering diffs
            SetupOutputVariable(state,
                                "Environmental Impact Purchased Electricity Source Energy",
                                Constant::Units::J,
                                pm->pollComps[(int)PollFuelComponent::ElectricityPurchased].sourceVal,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                "Site",
                                Constant::eResource::Source,
                                OutputProcessor::Group::Invalid,
                                OutputProcessor::EndUseCat::PurchasedElectricityEmissions);
            SetupOutputVariable(state,
                                "Environmental Impact Surplus Sold Electricity Source",
                                Constant::Units::J,
                                pm->pollComps[(int)PollFuelComponent::ElectricitySurplusSold].sourceVal,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                "Site",
                                Constant::eResource::Source,
                                OutputProcessor::Group::Invalid,
                                OutputProcessor::EndUseCat::SoldElectricityEmissions);
        }

    } // End of the NumEnergyTypes Do Loop

    // And Total Carbon Equivalent variables
    SetupOutputVariable(state,
                        "Environmental Impact Total N2O Emissions Carbon Equivalent Mass",
                        Constant::Units::kg,
                        pm->TotCarbonEquivFromN2O,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        "Site",
                        Constant::eResource::CarbonEquivalent,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::CarbonEquivalentEmissions);
    SetupOutputVariable(state,
                        "Environmental Impact Total CH4 Emissions Carbon Equivalent Mass",
                        Constant::Units::kg,
                        pm->TotCarbonEquivFromCH4,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        "Site",
                        Constant::eResource::CarbonEquivalent,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::CarbonEquivalentEmissions);
    SetupOutputVariable(state,
                        "Environmental Impact Total CO2 Emissions Carbon Equivalent Mass",
                        Constant::Units::kg,
                        pm->TotCarbonEquivFromCO2,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        "Site",
                        Constant::eResource::CarbonEquivalent,
                        OutputProcessor::Group::Invalid,
                        OutputProcessor::EndUseCat::CarbonEquivalentEmissions);

    // Connect pollution meters to energy meters
    for (int iMeter = 0; iMeter < (int)PollFacilityMeter::Num; ++iMeter) {
        pm->facilityMeterNums[iMeter] = GetMeterIndex(state, Util::makeUPPER(pollFacilityMeterNames[iMeter]));
    }
}

void CheckPollutionMeterReporting(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2008

    // in progress (what is in progress?)

    auto const &pm = state.dataPollution;

    if (pm->NumFuelFactors == 0 || pm->NumEnvImpactFactors == 0) {
        if (ReportingThisVariable(state, "Environmental Impact Total N2O Emissions Carbon Equivalent Mass") ||
            ReportingThisVariable(state, "Environmental Impact Total CH4 Emissions Carbon Equivalent Mass") ||
            ReportingThisVariable(state, "Environmental Impact Total CO2 Emissions Carbon Equivalent Mass") ||
            ReportingThisVariable(state, "Carbon Equivalent:Facility") ||
            ReportingThisVariable(state, "CarbonEquivalentEmissions:Carbon Equivalent")) {
            ShowWarningError(
                state, "GetPollutionFactorInput: Requested reporting for Carbon Equivalent Pollution, but insufficient information is entered.");
            ShowContinueError(
                state, "Both \"FuelFactors\" and \"EnvironmentalImpactFactors\" must be entered or the displayed carbon pollution will all be zero.");
        }
    }
}

// End of Get Input subroutines for the Pollution Module
//******************************************************************************

void CalcPollution(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   1998
    //       MODIFIED       na
    //       RE-ENGINEERED  December 2003 RJL

    //       Then the amount of Pollution produced by each fuel type is
    //       calculated in kgs.
    //       Input units for the coefficients is not standard and needs to be converted here.
    //       Most of the units are g/MJ, however water is in L/MJ and low level nuclear water is m3/MJ
    //       so only the energy has to be converted from J to MJ.

    //     For each pollution/fuel type, Schedule values are allowed.  Thus, calculations are bundled.
    auto &pm = state.dataPollution;

    for (int iPoll = 0; iPoll < (int)Pollutant::Num; ++iPoll) {
        pm->pollutantVals[iPoll] = 0.0;

        for (int iPollFuel = 0; iPollFuel < (int)PollFuel::Num; ++iPollFuel) {
            auto &pollCoeff = pm->pollCoeffs[iPollFuel];
            PollFuelComponent pollFuelComp = pollFuel2pollFuelComponent[iPollFuel];
            auto &pollComp = pm->pollComps[(int)pollFuelComp];

            if (pollCoeff.used) {
                pollComp.pollutantVals[iPoll] = 0.0;
                Real64 pollutantVal = pollCoeff.pollutantCoeffs[iPoll];

                // Why are these two the exceptions?
                if (iPoll != (int)Pollutant::Water && iPoll != (int)Pollutant::NuclearLow) {
                    pollutantVal *= 0.001;
                }

                if (pollCoeff.pollutantScheds[iPoll] != nullptr) {
                    pollutantVal *= pollCoeff.pollutantScheds[iPoll]->getCurrentVal();
                }
                pollComp.pollutantVals[iPoll] = pm->facilityMeterFuelComponentVals[(int)pollFuelComp] * 1.0e-6 * pollutantVal;
            }

            pm->pollutantVals[iPoll] += pollComp.pollutantVals[iPoll];
        } // for (iPollFactor)
    } // for (iPoll)

    pm->TotCarbonEquivFromN2O = pm->pollutantVals[(int)Pollutant::N2O] * pm->CarbonEquivN2O;
    pm->TotCarbonEquivFromCH4 = pm->pollutantVals[(int)Pollutant::CH4] * pm->CarbonEquivCH4;
    pm->TotCarbonEquivFromCO2 = pm->pollutantVals[(int)Pollutant::CO2] * pm->CarbonEquivCO2;

    auto const &pollCoeffElec = pm->pollCoeffs[(int)PollFuel::Electricity];
    auto &pollCompElec = pm->pollComps[(int)PollFuelComponent::Electricity];
    auto &pollCompElecPurchased = pm->pollComps[(int)PollFuelComponent::ElectricityPurchased];
    auto &pollCompElecSurplusSold = pm->pollComps[(int)PollFuelComponent::ElectricitySurplusSold];

    pollCompElec.sourceVal = pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::Electricity] * pollCoeffElec.sourceCoeff;
    pollCompElecPurchased.sourceVal = pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::ElectricityPurchased] * pollCoeffElec.sourceCoeff;
    pollCompElecSurplusSold.sourceVal =
        pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::ElectricitySurplusSold] * pollCoeffElec.sourceCoeff;

    if (pollCoeffElec.sourceSched != nullptr) {
        Real64 pollCoeffElecSchedVal = pollCoeffElec.sourceSched->getCurrentVal();
        pollCompElec.sourceVal *= pollCoeffElecSchedVal;
        pollCompElecPurchased.sourceVal *= pollCoeffElecSchedVal;
        pollCompElecSurplusSold.sourceVal *= pollCoeffElecSchedVal;
    }

    // does not include district heating or steam
    auto const &pollCoeffGas = pm->pollCoeffs[(int)PollFuel::NaturalGas];
    auto &pollCompGas = pm->pollComps[(int)PollFuelComponent::NaturalGas];
    pollCompGas.sourceVal = pm->facilityMeterVals[(int)PollFacilityMeter::NaturalGas] * pollCoeffGas.sourceCoeff;
    if (pollCoeffGas.sourceSched != nullptr) {
        pollCompGas.sourceVal *= pollCoeffGas.sourceSched->getCurrentVal();
    }

    for (PollFuel pollFuel : {PollFuel::FuelOil1,
                              PollFuel::FuelOil2,
                              PollFuel::Diesel,
                              PollFuel::Gasoline,
                              PollFuel::Propane,
                              PollFuel::Coal,
                              PollFuel::OtherFuel1,
                              PollFuel::OtherFuel2}) {
        auto const &pollCoeff = pm->pollCoeffs[(int)pollFuel];
        PollFuelComponent pollFuelComponent = pollFuel2pollFuelComponent[(int)pollFuel];
        auto &pollComp = pm->pollComps[(int)pollFuelComponent];

        pollComp.sourceVal = pm->facilityMeterFuelComponentVals[(int)pollFuelComponent] * pollCoeff.sourceCoeff;
        if (pollCoeff.sourceSched != nullptr) {
            pollComp.sourceVal *= pollCoeff.sourceSched->getCurrentVal();
        }
    } // for (pollFuelComponent)
} // CalcPollution()

void ReadEnergyMeters(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   1998
    //       MODIFIED       na
    //       RE-ENGINEERED  December 2003 RJL

    // PURPOSE OF THIS SUBROUTINE:
    //       Read Energy Results from the meters
    // This routine reads the meters for the energy used

    // Using/Aliasing
    Real64 FracTimeStepZone = state.dataHVACGlobal->FracTimeStepZone;
    auto &pm = state.dataPollution;

    for (int iMeter = 0; iMeter < (int)PollFacilityMeter::Num; ++iMeter) {
        pm->facilityMeterVals[iMeter] =
            GetInstantMeterValue(state, pm->facilityMeterNums[iMeter], OutputProcessor::TimeStepType::Zone) * FracTimeStepZone +
            GetInstantMeterValue(state, pm->facilityMeterNums[iMeter], OutputProcessor::TimeStepType::System);
    }

    // Now these fuel types have to be sorted and summed into categories that we have pollution factors for.
    // The Off-Site Electricity is the total needed by the facility minus the amount generated on-site.
    // The on-site pollutants will end up being other fuel types used by the generators.
    // If the difference between the 2 electric quantities is <0.0 then it will be zero for that time step.
    // We will also add the District Cooling here with a rough conversion from Energy using the User
    // defined COP.

    pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::Electricity] =
        pm->facilityMeterVals[(int)PollFacilityMeter::Electricity] - pm->facilityMeterVals[(int)PollFacilityMeter::ElectricityProduced] +
        pm->facilityMeterVals[(int)PollFacilityMeter::CoolPurchased] / pm->PurchCoolCOP;

    if (pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::Electricity] < 0.0) {
        pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::Electricity] = 0.0;
    }

    // The Natural Gas fuel type will be summed from the meters with the District Heating using an efficiency.
    pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::NaturalGas] =
        pm->facilityMeterVals[(int)PollFacilityMeter::NaturalGas] +
        pm->facilityMeterVals[(int)PollFacilityMeter::HeatPurchased] / pm->PurchHeatEffic +
        pm->facilityMeterVals[(int)PollFacilityMeter::Steam] / pm->SteamConvEffic;

    pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::FuelOil1] = pm->facilityMeterVals[(int)PollFacilityMeter::FuelOil1];
    pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::FuelOil2] = pm->facilityMeterVals[(int)PollFacilityMeter::FuelOil2];
    pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::Gasoline] = pm->facilityMeterVals[(int)PollFacilityMeter::Gasoline];
    pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::Propane] = pm->facilityMeterVals[(int)PollFacilityMeter::Propane];
    pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::Coal] = pm->facilityMeterVals[(int)PollFacilityMeter::Coal];
    pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::Diesel] = pm->facilityMeterVals[(int)PollFacilityMeter::Diesel];
    pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::OtherFuel1] = pm->facilityMeterVals[(int)PollFacilityMeter::OtherFuel1];
    pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::OtherFuel2] = pm->facilityMeterVals[(int)PollFacilityMeter::OtherFuel2];
    pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::ElectricityPurchased] =
        pm->facilityMeterVals[(int)PollFacilityMeter::ElectricityPurchased];
    pm->facilityMeterFuelComponentVals[(int)PollFuelComponent::ElectricitySurplusSold] =
        pm->facilityMeterVals[(int)PollFacilityMeter::ElectricitySurplusSold];
}

// *****************************************************************************
// Utility Routines to allow access to data inside this module.
// *****************************************************************************

void GetFuelFactorInfo(EnergyPlusData &state,
                       Constant::eFuel fuel,         // input fuel name  (standard from Tabular reports)
                       bool &fuelFactorUsed,         // return value true if user has entered this fuel
                       Real64 &fuelSourceFactor,     // if used, the source factor
                       bool &fuelFactorScheduleUsed, // if true, schedules for this fuel are used
                       Sched::Schedule **ffSched     // if schedules for this fuel are used, return schedule index
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   July 2008

    // PURPOSE OF THIS SUBROUTINE:
    // This routine allows access to data inside this module from other modules (specifically the
    // output tabular reports.
    auto const &pm = state.dataPollution;

    if (pm->GetInputFlagPollution) {
        GetPollutionFactorInput(state);
        pm->GetInputFlagPollution = false;
    }

    fuelFactorUsed = false;
    fuelSourceFactor = 0.0;
    fuelFactorScheduleUsed = false;
    *ffSched = nullptr;

    PollFuel pollFuel = fuel2pollFuel[(int)fuel];
    auto const &pollCoeff = pm->pollCoeffs[(int)pollFuel];

    if (pollCoeff.used) {
        fuelFactorUsed = true;
        fuelSourceFactor = pollCoeff.sourceCoeff;
        if (pollCoeff.sourceSched == nullptr) {
            fuelFactorScheduleUsed = false;
        } else {
            fuelFactorScheduleUsed = true;
            *ffSched = pollCoeff.sourceSched;
        }
    } else {
        fuelSourceFactor = pollFuelFactors[(int)pollFuel];
    }

    if (fuel == Constant::eFuel::DistrictHeatingWater) {
        fuelSourceFactor /= pm->PurchHeatEffic;
    } else if (fuel == Constant::eFuel::DistrictCooling) {
        fuelSourceFactor /= pm->PurchCoolCOP;
    } else if (fuel == Constant::eFuel::DistrictHeatingSteam) {
        fuelSourceFactor = 0.3 / pm->SteamConvEffic;
    }
}

void GetEnvironmentalImpactFactorInfo(EnergyPlusData &state,
                                      Real64 &efficiencyDistrictHeatingWater,  // if entered, the efficiency of District Heating Water
                                      Real64 &efficiencyDistrictCooling,       // if entered, the efficiency of District Cooling
                                      Real64 &sourceFactorDistrictHeatingSteam // if entered, the source factor for Dictrict Heating Steam
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   August 2008

    // PURPOSE OF THIS SUBROUTINE:
    // This routine allows access to data inside this module from other modules (specifically the
    // output tabular reports.

    auto const &pm = state.dataPollution;
    if (pm->GetInputFlagPollution) {
        GetPollutionFactorInput(state);
        pm->GetInputFlagPollution = false;
    }

    if (pm->NumEnvImpactFactors > 0) {
        efficiencyDistrictHeatingWater = pm->PurchHeatEffic;
        sourceFactorDistrictHeatingSteam = pm->SteamConvEffic;
        efficiencyDistrictCooling = pm->PurchCoolCOP;
    }
}

} // namespace EnergyPlus::Pollution
