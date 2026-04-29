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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HybridModel.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace HybridModel {

    // MODULE INFORMATION:
    //       AUTHOR         Sang Hoon Lee, Tianzhen Hong, Rongpeng Zhang. LBNL
    //       DATE WRITTEN   Oct 2015

    // PURPOSE OF THIS MODULE:
    // This module manages hybrid model.

    // METHODOLOGY EMPLOYED:
    //  The model uses measured zone air temperature to calculate internal thermal mass or infiltration air flow rate.

    // USE STATEMENTS:

    // Using/Aliasing
    using namespace DataHeatBalance;

    // Functions

    void GetHybridModelZone(EnergyPlusData &state)
    {
        Array1D_bool lAlphaFieldBlanks(16, false);
        Array1D_bool lNumericFieldBlanks(4, false);
        std::string CurrentModuleObject; // to assist in getting input
        Array1D_string cAlphaArgs(16);   // Alpha input items for object
        Array1D_string cAlphaFieldNames(16);
        Array1D_string cNumericFieldNames(16);
        Array1D<Real64> rNumericArgs(4); // Numeric input items for object

        // Read hybrid model input
        CurrentModuleObject = "HybridModel:Zone";
        state.dataHybridModel->NumOfHybridModelZones = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

        if (state.dataHybridModel->NumOfHybridModelZones > 0) {
            state.dataHybridModel->hybridModelZones.allocate(state.dataGlobal->NumOfZones);
            bool ErrorsFound = false; // If errors detected in input
            int NumAlphas = 0;        // Number of Alphas for each GetobjectItem call
            int NumNumbers = 0;       // Number of Numbers for each GetobjectItem call
            int IOStatus = 0;
            for (int HybridModelNum = 1; HybridModelNum <= state.dataHybridModel->NumOfHybridModelZones; ++HybridModelNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         HybridModelNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                int ZonePtr = Util::FindItemInList(cAlphaArgs(2), state.dataHeatBal->Zone); // "Zone" is a 1D array, cAlphaArgs(2) is the zone name
                if (ZonePtr > 0) {
                    auto &hmZone = state.dataHybridModel->hybridModelZones(ZonePtr);
                    hmZone.Name = cAlphaArgs(1);                                                        // Zone HybridModel name
                    state.dataHybridModel->FlagHybridModel_TM = Util::SameString(cAlphaArgs(3), "Yes"); // Calculate thermal mass option
                    state.dataHybridModel->FlagHybridModel_AI = Util::SameString(cAlphaArgs(4), "Yes"); // Calculate infiltration rate option
                    state.dataHybridModel->FlagHybridModel_PC = Util::SameString(cAlphaArgs(5), "Yes"); // Calculate people count option

                    // Pointers used to help decide which unknown parameter to solve
                    // Zone Air Infiltration Rate and Zone Internal Thermal Mass calculations cannot be performed simultaneously
                    Sched::Schedule *temperatureSched = Sched::GetSchedule(state, cAlphaArgs(6));
                    Sched::Schedule *humidityRatioSched = Sched::GetSchedule(state, cAlphaArgs(7));
                    Sched::Schedule *CO2ConcentrationSched = Sched::GetSchedule(state, cAlphaArgs(8));

                    // Not used for now
                    Sched::Schedule *peopleActivityLevelSched = Sched::GetSchedule(state, cAlphaArgs(9));
                    Sched::Schedule *peopleSensibleFractionSched = Sched::GetSchedule(state, cAlphaArgs(10));
                    Sched::Schedule *peopleRadiantFractionSched = Sched::GetSchedule(state, cAlphaArgs(11));
                    Sched::Schedule *peopleCO2GenRateSched = Sched::GetSchedule(state, cAlphaArgs(12));

                    // Pointers used to help decide wheather to include system supply terms in the inverse algorithms
                    Sched::Schedule *supplyAirTemperatureSched = Sched::GetSchedule(state, cAlphaArgs(13));
                    Sched::Schedule *supplyAirMassFlowRateSched = Sched::GetSchedule(state, cAlphaArgs(14));
                    Sched::Schedule *supplyAirHumidityRatioSched = Sched::GetSchedule(state, cAlphaArgs(15));
                    Sched::Schedule *supplyAirCO2ConcentrationSched = Sched::GetSchedule(state, cAlphaArgs(16));

                    //  Note: Internal thermal mass can be calculated only with measured temperature.
                    //                  Air infiltration rate can be calculated with either measured temperature, humidity ratio, or CO2
                    //                  concentration. People count can be calculated with either measured temperature, humidity ratio, or CO2
                    //                  concentration.

                    // Initially set all flags to be false
                    hmZone.InternalThermalMassCalc_T = false;
                    hmZone.InfiltrationCalc_T = false;
                    hmZone.InfiltrationCalc_H = false;
                    hmZone.InfiltrationCalc_C = false;
                    hmZone.PeopleCountCalc_T = false;
                    hmZone.PeopleCountCalc_H = false;
                    hmZone.PeopleCountCalc_C = false;

                    // Scenario 1: Only one unknown parameter to solve
                    // Scenario 1-1: To solve thermal mass
                    if (state.dataHybridModel->FlagHybridModel_TM) {
                        if (state.dataHybridModel->FlagHybridModel_AI) {
                            ShowSevereError(
                                state,
                                EnergyPlus::format("Field \"{} and {}\" cannot be both set to YES.", cAlphaFieldNames(3), cAlphaFieldNames(4)));
                            ErrorsFound = true;
                        }

                        if (state.dataHybridModel->FlagHybridModel_PC) {
                            ShowSevereError(
                                state,
                                EnergyPlus::format("Field \"{} and {}\" cannot be both set to YES.", cAlphaFieldNames(3), cAlphaFieldNames(5)));
                            ErrorsFound = true;
                        }

                        if (temperatureSched == nullptr) {
                            ShowSevereError(state,
                                            EnergyPlus::format("Measured Zone Air Temperature Schedule is not defined for: {}", CurrentModuleObject));
                            ErrorsFound = true;
                        } else {
                            hmZone.InternalThermalMassCalc_T = true;
                        }
                    }

                    // Scenario 1-2: To solve infiltration rate
                    if (state.dataHybridModel->FlagHybridModel_AI) {
                        if (state.dataHybridModel->FlagHybridModel_PC) {
                            ShowSevereError(
                                state,
                                EnergyPlus::format("Field \"{}\" and \"{}\" cannot be both set to YES.", cAlphaFieldNames(4), cAlphaFieldNames(5)));
                            ErrorsFound = true;
                        }
                        if (temperatureSched == nullptr && humidityRatioSched == nullptr && CO2ConcentrationSched == nullptr) {
                            // Show fatal error if no measurement schedule is provided
                            ShowSevereError(state,
                                            EnergyPlus::format("No measured environmental parameter is provided for: {}", CurrentModuleObject));
                            ShowContinueError(
                                state,
                                EnergyPlus::format("One of the field \"{}\", \"{}\", or {}\" must be provided for the HybridModel:Zone.",
                                                   cAlphaFieldNames(6),
                                                   cAlphaFieldNames(7),
                                                   cAlphaFieldNames(8)));
                            ErrorsFound = true;
                        } else {
                            if (temperatureSched != nullptr && !state.dataHybridModel->FlagHybridModel_TM) {
                                // Temperature schedule is provided, ignore humidity ratio and CO2 concentration schedules.
                                hmZone.InfiltrationCalc_T = true;
                                if (humidityRatioSched != nullptr) {
                                    ShowWarningError(state, EnergyPlus::format("Field \"{}\" is provided.", cAlphaFieldNames(6)));
                                    ShowContinueError(state, EnergyPlus::format("Field \"{}\" will not be used.", cAlphaFieldNames(7)));
                                }
                                if (CO2ConcentrationSched != nullptr) {
                                    ShowWarningError(state, EnergyPlus::format("Field \"{}\" is provided.", cAlphaFieldNames(6)));
                                    ShowContinueError(state, EnergyPlus::format("Field \"{}\" will not be used.", cAlphaFieldNames(8)));
                                }
                            }
                            if (humidityRatioSched != nullptr && temperatureSched == nullptr) {
                                // Humidity ratio schedule is provided, ignore CO2 concentration schedule.
                                hmZone.InfiltrationCalc_H = true;
                                if (CO2ConcentrationSched != nullptr) {
                                    ShowWarningError(state, EnergyPlus::format("Field \"{}\" is provided.", cAlphaFieldNames(7)));
                                    ShowContinueError(state, EnergyPlus::format("Field \"{}\" will not be used.", cAlphaFieldNames(8)));
                                }
                            }
                            if (CO2ConcentrationSched != nullptr && temperatureSched == nullptr && humidityRatioSched == nullptr) {
                                // Only CO2 concentration schedule is provided.
                                hmZone.InfiltrationCalc_C = true;
                            }
                        }
                    }

                    // Scenario 1-3: To solve people count
                    if (state.dataHybridModel->FlagHybridModel_PC) {
                        if (temperatureSched == nullptr && humidityRatioSched == nullptr && CO2ConcentrationSched == nullptr) {
                            // Show fatal error if no measurement schedule is provided
                            ShowSevereError(state,
                                            EnergyPlus::format("No measured environmental parameter is provided for: {}", CurrentModuleObject));
                            ShowContinueError(
                                state,
                                EnergyPlus::format("One of the field \"{}\", \"{}\", or {}\" must be provided for the HybridModel:Zone.",
                                                   cAlphaFieldNames(6),
                                                   cAlphaFieldNames(7),
                                                   cAlphaFieldNames(8)));
                            ErrorsFound = true;
                        } else {
                            if (temperatureSched != nullptr && !state.dataHybridModel->FlagHybridModel_TM) {
                                // Temperature schedule is provided, ignore humidity ratio and CO2 concentration schedules.
                                hmZone.PeopleCountCalc_T = true;
                                if (humidityRatioSched != nullptr) {
                                    ShowWarningError(
                                        state,
                                        "The measured air humidity ratio schedule will not be used since measured air temperature is provided.");
                                }
                                if (CO2ConcentrationSched != nullptr) {
                                    ShowWarningError(
                                        state,
                                        "The measured air CO2 concentration schedule will not be used since measured air temperature is provided.");
                                }
                            }
                            if (humidityRatioSched != nullptr && temperatureSched == nullptr) {
                                // Humidity ratio schedule is provided, ignore CO2 concentration schedule.
                                hmZone.PeopleCountCalc_H = true;
                                if (CO2ConcentrationSched != nullptr) {
                                    ShowWarningError(state,
                                                     "The measured air CO2 concentration schedule will not be used since measured air humidity "
                                                     "ratio is provided.");
                                }
                            }
                            if (CO2ConcentrationSched != nullptr && temperatureSched == nullptr && humidityRatioSched == nullptr) {
                                // Only CO2 concentration schedule is provided.
                                hmZone.PeopleCountCalc_C = true;
                            }
                        }
                    }

                    // Decide if system supply terms are valid to be included in the inverse solution
                    if (supplyAirTemperatureSched != nullptr && supplyAirMassFlowRateSched != nullptr && supplyAirHumidityRatioSched != nullptr) {
                        if (hmZone.InfiltrationCalc_T || hmZone.PeopleCountCalc_T) {
                            hmZone.IncludeSystemSupplyParameters = true;
                        } else {
                            ShowWarningError(state,
                                             EnergyPlus::format("Field \"{}\", {}, and \"{}\" will not be used in the inverse balance equation.",
                                                                cAlphaFieldNames(13),
                                                                cAlphaFieldNames(14),
                                                                cAlphaFieldNames(15)));
                        }
                    }

                    if (supplyAirHumidityRatioSched != nullptr && supplyAirMassFlowRateSched != nullptr) {
                        if (hmZone.InfiltrationCalc_H || hmZone.PeopleCountCalc_H) {
                            hmZone.IncludeSystemSupplyParameters = true;
                        } else {
                            ShowWarningError(state,
                                             EnergyPlus::format("Field \"{}\" and \"{}\" will not be used in the inverse balance equation.",
                                                                cAlphaFieldNames(15),
                                                                cAlphaFieldNames(14)));
                        }
                    }

                    if (supplyAirCO2ConcentrationSched != nullptr && supplyAirMassFlowRateSched != nullptr) {
                        if (hmZone.InfiltrationCalc_C || hmZone.PeopleCountCalc_C) {
                            hmZone.IncludeSystemSupplyParameters = true;
                        } else {
                            ShowWarningError(state,
                                             EnergyPlus::format("Field \"{}\" and \"{}\" will not be used in the inverse balance equation.",
                                                                cAlphaFieldNames(16),
                                                                cAlphaFieldNames(14)));
                        }
                    }

                    // Flags showing Hybrid Modeling settings
                    state.dataHybridModel->FlagHybridModel = hmZone.InternalThermalMassCalc_T || hmZone.InfiltrationCalc_T ||
                                                             hmZone.InfiltrationCalc_H || hmZone.InfiltrationCalc_C || hmZone.PeopleCountCalc_T ||
                                                             hmZone.PeopleCountCalc_H || hmZone.PeopleCountCalc_C;

                    if (hmZone.InternalThermalMassCalc_T || hmZone.InfiltrationCalc_T || hmZone.PeopleCountCalc_T) {
                        hmZone.measuredTempSched = temperatureSched;
                    }

                    if (hmZone.InfiltrationCalc_H || hmZone.PeopleCountCalc_H) {
                        hmZone.measuredHumRatSched = humidityRatioSched;
                    }

                    if (hmZone.InfiltrationCalc_C || hmZone.PeopleCountCalc_C) {
                        hmZone.measuredCO2ConcSched = CO2ConcentrationSched;
                    }

                    if (hmZone.IncludeSystemSupplyParameters) {
                        hmZone.supplyAirTempSched = supplyAirTemperatureSched;
                        hmZone.supplyAirMassFlowRateSched = supplyAirMassFlowRateSched;
                        hmZone.supplyAirHumRatSched = supplyAirHumidityRatioSched;
                        hmZone.supplyAirCO2ConcSched = supplyAirCO2ConcentrationSched;
                    }

                    // Get optional people related schedules
                    if (hmZone.PeopleCountCalc_T || hmZone.PeopleCountCalc_H || hmZone.PeopleCountCalc_C) {
                        if (peopleActivityLevelSched != nullptr) {
                            hmZone.peopleActivityLevelSched = peopleActivityLevelSched;
                        } else {
                            ShowWarningError(
                                state,
                                EnergyPlus::format(
                                    "Field \"{}\": default people activity level is not provided, default value of 130W/person will be used.",
                                    cAlphaFieldNames(9)));
                        }
                        if (peopleSensibleFractionSched != nullptr) {
                            hmZone.peopleSensibleFracSched = peopleSensibleFractionSched;
                        } else {
                            ShowWarningError(
                                state,
                                EnergyPlus::format(
                                    "Field \"{}\": default people sensible heat rate is not provided, default value of 0.6 will be used.",
                                    cAlphaFieldNames(10)));
                        }
                        if (peopleRadiantFractionSched != nullptr) {
                            hmZone.peopleRadiantFracSched = peopleRadiantFractionSched;
                        } else {
                            ShowWarningError(
                                state,
                                EnergyPlus::format("Field \"{}\": default people radiant heat portion (of sensible heat) is not provided, default "
                                                   "value of 0.7 will be used.",
                                                   cAlphaFieldNames(11)));
                        }
                        if (peopleCO2GenRateSched != nullptr) {
                            hmZone.peopleCO2GenRateSched = peopleCO2GenRateSched;
                        } else {
                            ShowWarningError(
                                state,
                                EnergyPlus::format("Field \"{}\": default people CO2 generation rate is not provided, default value of 0.0000000382 "
                                                   "kg/W will be used.",
                                                   cAlphaFieldNames(12)));
                        }
                    }

                    if (state.dataHybridModel->FlagHybridModel) {
                        // prepare start and end date for Hybrid Modeling
                        hmZone.measuredTempStartMonth = rNumericArgs(1);
                        hmZone.measuredTempStartDate = rNumericArgs(2);
                        hmZone.measuredTempEndMonth = rNumericArgs(3);
                        hmZone.measuredTempEndDate = rNumericArgs(4);
                        {
                            int const HMDayArr[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

                            int HybridModelStartMonth = hmZone.measuredTempStartMonth;
                            int HybridModelStartDate = hmZone.measuredTempStartDate;
                            int HybridModelEndMonth = hmZone.measuredTempEndMonth;
                            int HybridModelEndDate = hmZone.measuredTempEndDate;

                            int HMStartDay = 0;
                            int HMEndDay = 0;
                            if (HybridModelStartMonth >= 1 && HybridModelStartMonth <= 12) {
                                HMStartDay = HMDayArr[HybridModelStartMonth - 1];
                            }

                            if (HybridModelEndMonth >= 1 && HybridModelEndMonth <= 12) {
                                HMEndDay = HMDayArr[HybridModelEndMonth - 1];
                            }

                            hmZone.HybridStartDayOfYear = HMStartDay + HybridModelStartDate;
                            hmZone.HybridEndDayOfYear = HMEndDay + HybridModelEndDate;
                        }
                    }

                    // Output variable
                    if (hmZone.InfiltrationCalc_T || hmZone.InfiltrationCalc_H || hmZone.InfiltrationCalc_C) {
                        SetupOutputVariable(state,
                                            "Zone Infiltration Hybrid Model Air Change Rate",
                                            Constant::Units::ach,
                                            state.dataHeatBal->Zone(ZonePtr).InfilOAAirChangeRateHM,
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            state.dataHeatBal->Zone(ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Infiltration Hybrid Model Mass Flow Rate",
                                            Constant::Units::kg_s,
                                            state.dataHeatBal->Zone(ZonePtr).MCPIHM,
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            state.dataHeatBal->Zone(ZonePtr).Name);
                    }
                    if (hmZone.PeopleCountCalc_T || hmZone.PeopleCountCalc_H || hmZone.PeopleCountCalc_C) {
                        SetupOutputVariable(state,
                                            "Zone Hybrid Model People Count",
                                            Constant::Units::None,
                                            state.dataHeatBal->Zone(ZonePtr).NumOccHM,
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            state.dataHeatBal->Zone(ZonePtr).Name);
                    }
                    if (hmZone.InternalThermalMassCalc_T) {
                        SetupOutputVariable(state,
                                            "Zone Hybrid Model Thermal Mass Multiplier",
                                            Constant::Units::None,
                                            state.dataHeatBal->Zone(ZonePtr).ZoneVolCapMultpSensHM,
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            state.dataHeatBal->Zone(ZonePtr).Name);
                    }

                    // ZoneAirMassFlowConservation should not be activated during the Hybrid Modeling infiltration calculations
                    if (hmZone.InfiltrationCalc_T && state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance) {
                        state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance = false;
                        ShowWarningError(state, "ZoneAirMassFlowConservation is deactivated when Hybrid Modeling is performed.");
                    }
                } else {
                    ShowSevereError(
                        state,
                        EnergyPlus::format(
                            "{}=\"{}\" invalid {}=\"{}\" not found.", CurrentModuleObject, cAlphaArgs(1), cAlphaFieldNames(2), cAlphaArgs(2)));
                    ErrorsFound = true;
                }
            }

            // RoomAirModelType should be Mixing if Hybrid Modeling is performed for the zone
            if (state.dataHybridModel->FlagHybridModel) {
                for (int ZonePtr = 1; ZonePtr <= state.dataGlobal->NumOfZones; ZonePtr++) {
                    const auto &hmZone = state.dataHybridModel->hybridModelZones(ZonePtr);
                    if ((hmZone.InternalThermalMassCalc_T || hmZone.InfiltrationCalc_T) &&
                        (state.dataRoomAir->AirModel(ZonePtr).AirModel != RoomAir::RoomAirModel::Mixing)) {
                        state.dataRoomAir->AirModel(ZonePtr).AirModel = RoomAir::RoomAirModel::Mixing;
                        ShowWarningError(state, "Room Air Model Type should be Mixing if Hybrid Modeling is performed for the zone.");
                    }
                }
                if (state.dataHeatBal->doSpaceHeatBalanceSimulation || state.dataHeatBal->doSpaceHeatBalanceSizing) {
                    ShowSevereError(state, "Hybrid Modeling is not supported with ZoneAirHeatBalanceAlgorithm Space Heat Balance.");
                    ErrorsFound = true;
                }
            }

            if (ErrorsFound) {
                ShowFatalError(state, "Errors getting Hybrid Model input data. Preceding condition(s) cause termination.");
            }
        }
    }

    // Needed for unit tests, should not be normally called.

} // namespace HybridModel

} // namespace EnergyPlus
