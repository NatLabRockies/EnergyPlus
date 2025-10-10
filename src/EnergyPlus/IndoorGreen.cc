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

#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataDaylighting.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/IndoorGreen.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>
#include <EnergyPlus/api/datatransfer.h>

namespace EnergyPlus {

namespace IndoorGreen {
    // Module containing the routines dealing with the Indoor Living Walls
    static constexpr std::array<std::string_view, static_cast<int>(ETCalculationMethod::Num)> etCalculationMethodsUC = {"PENMAN-MONTEITH",
                                                                                                                        "STANGHELLINI"};
    static constexpr std::array<std::string_view, static_cast<int>(LightingMethod::Num)> lightingMethodsUC = {"LED", "DAYLIGHT", "LED-DAYLIGHT"};

    void SimIndoorGreen(EnergyPlusData &state)
    {
        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the thermal performance of indoor living walls including the grow lights.
        // This subroutine interacts with inside surface heat balance, zone air heat balance and zone air moisture balance in EnergyPlus.
        auto const &lw = state.dataIndoorGreen;
        if (lw->getInputFlag) {
            bool ErrorsFound(false);
            const char *RoutineName("IndoorLivingWall: "); // include trailing blank space
            GetIndoorGreenInput(state, ErrorsFound);
            if (ErrorsFound) {
                ShowFatalError(state, format("{}Errors found in input.  Program terminates.", RoutineName));
            }
            SetIndoorGreenOutput(state);
            lw->getInputFlag = false;
        }
        if (lw->NumIndoorGreen > 0) {
            InitIndoorGreen(state);
            // Simulate evapotranspiration from indoor living walls
            ETModel(state);
        }
    }

    void GetIndoorGreenInput(EnergyPlusData &state, bool &ErrorsFound)
    {
        // PURPOSE OF THIS SUBROUTINE:
        // Get the input for the indoor living wall objects and store the input data in the indoorGreens array.

        auto &s_lw = state.dataIndoorGreen;
        auto &s_ip = state.dataInputProcessing->inputProcessor;
        auto &s_ipsc = state.dataIPShortCut;

        static constexpr std::string_view RoutineName("GetIndoorLivingWallInput: ");
        std::string_view cCurrentModuleObject = "IndoorLivingWall"; // match the idd
        int NumNums;                                                // Number of real numbers returned by GetObjectItem
        int NumAlphas;                                              // Number of alphanumerics returned by GetObjectItem
        int IOStat;                                                 // Status flag from GetObjectItem

        s_lw->NumIndoorGreen = s_ip->getNumObjectsFound(state, cCurrentModuleObject);
        if (s_lw->NumIndoorGreen > 0) {
            s_lw->indoorGreens.allocate(s_lw->NumIndoorGreen); // Allocate the IndoorGreen input data array
        }
        for (int IndoorGreenNum = 1; IndoorGreenNum <= s_lw->NumIndoorGreen; ++IndoorGreenNum) {
            auto &ig = s_lw->indoorGreens(IndoorGreenNum);
            s_ip->getObjectItem(state,
                                cCurrentModuleObject,
                                IndoorGreenNum,
                                s_ipsc->cAlphaArgs,
                                NumAlphas,
                                s_ipsc->rNumericArgs,
                                NumNums,
                                IOStat,
                                s_ipsc->lNumericFieldBlanks,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);
            ErrorObjectHeader eoh{RoutineName, cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};
            Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
            ig.Name = s_ipsc->cAlphaArgs(1);
            ig.SurfName = s_ipsc->cAlphaArgs(2);
            ig.SurfPtr = Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataSurface->Surface);
            if (ig.SurfPtr <= 0) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                ErrorsFound = true;
            } else {
                if (state.dataSurface->Surface(ig.SurfPtr).insideHeatSourceTermSched != nullptr) {
                    ShowSevereError(state,
                                    format("The indoor green surface {} has an Inside Face Heat Source Term Schedule defined. This surface cannot "
                                           "also be used for indoor green.",
                                           s_ipsc->cAlphaArgs(2)));
                    ErrorsFound = true;
                }
                ig.ZonePtr = state.dataSurface->Surface(ig.SurfPtr).Zone;
                ig.SpacePtr = state.dataSurface->Surface(ig.SurfPtr).spaceNum;

                if (ig.ZonePtr <= 0 || ig.SpacePtr <= 0) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {} entered={}, {} is not assoicated with a thermal zone or space",
                                           RoutineName,
                                           s_ipsc->cAlphaArgs(1),
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ErrorsFound = true;
                } else if (state.dataSurface->Surface(ig.SurfPtr).ExtBoundCond < 0 ||
                           state.dataSurface->Surface(ig.SurfPtr).HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::CTF) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {} entered={}, not a valid surface for indoor green module",
                                           RoutineName,
                                           s_ipsc->cAlphaArgs(1),
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ErrorsFound = true;
                }
            }

            if ((ig.sched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            } else if (!ig.sched->checkMinVal(state, Clusive::In, 0.0)) {
                Sched::ShowSevereBadMin(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), Clusive::In, 0.0);
                ErrorsFound = true;
            }

            ig.etCalculationMethod = ETCalculationMethod::PenmanMonteith; // default
            ig.etCalculationMethod = static_cast<ETCalculationMethod>(getEnumValue(etCalculationMethodsUC, s_ipsc->cAlphaArgs(4)));
            ig.lightingMethod = LightingMethod::LED; // default
            ig.lightingMethod = static_cast<LightingMethod>(getEnumValue(lightingMethodsUC, s_ipsc->cAlphaArgs(5)));

            switch (ig.lightingMethod) {
            case LightingMethod::LED: {
                if ((ig.ledSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(6))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6));
                    ErrorsFound = true;
                } else if (!ig.ledSched->checkMinVal(state, Clusive::In, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6), Clusive::In, 0.0);
                    ErrorsFound = true;
                }
            } break;
            case LightingMethod::Daylighting: {
                ig.LightRefPtr = Util::FindItemInList(s_ipsc->cAlphaArgs(7),
                                                      state.dataDayltg->DaylRefPt,
                                                      &EnergyPlus::Dayltg::RefPointData::Name); // Field: Daylighting Reference Point Name
                ig.LightControlPtr = Util::FindItemInList(s_ipsc->cAlphaArgs(7),
                                                          state.dataDayltg->daylightControl,
                                                          &EnergyPlus::Dayltg::DaylightingControl::Name); // Field: Daylighting Control Name
                if (ig.LightControlPtr == 0) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(7), s_ipsc->cAlphaArgs(7));
                    ErrorsFound = true;
                    continue;
                }
            } break;
            case LightingMethod::LEDDaylighting: {
                ig.LightRefPtr = Util::FindItemInList(s_ipsc->cAlphaArgs(7),
                                                      state.dataDayltg->DaylRefPt,
                                                      &EnergyPlus::Dayltg::RefPointData::Name); // Field: Daylighting Reference Point Name
                ig.LightControlPtr = Util::FindItemInList(s_ipsc->cAlphaArgs(7),
                                                          state.dataDayltg->daylightControl,
                                                          &EnergyPlus::Dayltg::DaylightingControl::Name); // Field: Daylighting Control Name
                if (ig.LightControlPtr == 0) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(7), s_ipsc->cAlphaArgs(7));
                    ErrorsFound = true;
                    continue;
                }

                if ((ig.ledDaylightTargetSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(8))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(8), s_ipsc->cAlphaArgs(8));
                    ErrorsFound = true;
                } else if (!ig.ledDaylightTargetSched->checkMinVal(state, Clusive::In, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, s_ipsc->cAlphaFieldNames(8), s_ipsc->cAlphaArgs(8), Clusive::In, 0.0);
                    ErrorsFound = true;
                }
            } break;

            default:
                break;
            }

            ig.LeafArea = s_ipsc->rNumericArgs(1);
            if (ig.LeafArea < 0) {
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {} entered={}",
                                       RoutineName,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cNumericFieldNames(1),
                                       s_ipsc->rNumericArgs(1)));
                ErrorsFound = true;
            }
            ig.LEDNominalPPFD = s_ipsc->rNumericArgs(2);
            if (ig.LEDNominalPPFD < 0) {
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {} entered={}",
                                       RoutineName,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cNumericFieldNames(2),
                                       s_ipsc->rNumericArgs(2)));
                ErrorsFound = true;
            }
            ig.LEDNominalEleP = s_ipsc->rNumericArgs(3);
            if (ig.LEDNominalEleP < 0) {
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {} entered={}",
                                       RoutineName,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cNumericFieldNames(3),
                                       s_ipsc->rNumericArgs(3)));
                ErrorsFound = true;
            }
            ig.LEDRadFraction = s_ipsc->rNumericArgs(4);
            if (ig.LEDRadFraction < 0 || ig.LEDRadFraction > 1.0) {
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {} entered={}",
                                       RoutineName,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cNumericFieldNames(4),
                                       s_ipsc->rNumericArgs(4)));
                ErrorsFound = true;
            }
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state, "IndoorLivingWall", ig.Name, "Evapotranspiration Rate", "[kg_m2s]", ig.EMSETCalOverrideOn, ig.EMSET);
            } // EMS and API
        }
    }

    void SetIndoorGreenOutput(EnergyPlusData &state)
    {
        // Set up output variables
        auto &lw = state.dataIndoorGreen;
        for (int IndoorGreenNum = 1; IndoorGreenNum <= lw->NumIndoorGreen; ++IndoorGreenNum) {
            auto &ig = lw->indoorGreens(IndoorGreenNum);
            SetupZoneInternalGain(state,
                                  ig.ZonePtr,
                                  ig.Name,
                                  DataHeatBalance::IntGainType::IndoorGreen,
                                  &ig.SensibleRate,
                                  nullptr,
                                  nullptr,
                                  &ig.LatentRate,
                                  nullptr,
                                  nullptr,
                                  nullptr);

            SetupOutputVariable(state,
                                "Indoor Living Wall Plant Surface Temperature",
                                Constant::Units::C,
                                state.dataHeatBalSurf->SurfTempIn(ig.SurfPtr),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                ig.Name);
            SetupOutputVariable(state,
                                "Indoor Living Wall Sensible Heat Gain Rate",
                                Constant::Units::W,
                                ig.SensibleRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                ig.Name);
            SetupOutputVariable(state,
                                "Indoor Living Wall Latent Heat Gain Rate",
                                Constant::Units::W,
                                ig.LatentRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                ig.Name);
            SetupOutputVariable(state,
                                "Indoor Living Wall Evapotranspiration Rate",
                                Constant::Units::kg_m2s,
                                ig.ETRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                ig.Name);
            SetupOutputVariable(state,
                                "Indoor Living Wall Energy Rate Required For Evapotranspiration Per Unit Area",
                                Constant::Units::W_m2,
                                ig.LambdaET,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                ig.Name);
            SetupOutputVariable(state,
                                "Indoor Living Wall LED Operational PPFD",
                                Constant::Units::umol_m2s,
                                ig.LEDActualPPFD,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                ig.Name);
            SetupOutputVariable(state,
                                "Indoor Living Wall PPFD",
                                Constant::Units::umol_m2s,
                                ig.ZPPFD,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                ig.Name);
            SetupOutputVariable(state,
                                "Indoor Living Wall Vapor Pressure Deficit",
                                Constant::Units::Pa,
                                ig.ZVPD,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                ig.Name);
            SetupOutputVariable(state,
                                "Indoor Living Wall LED Sensible Heat Gain Rate",
                                Constant::Units::W,
                                ig.SensibleRateLED,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                ig.Name);
            SetupOutputVariable(state,
                                "Indoor Living Wall LED Operational Power",
                                Constant::Units::W,
                                ig.LEDActualEleP,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                ig.Name);
            SetupOutputVariable(state,
                                "Indoor Living Wall LED Electricity Energy",
                                Constant::Units::J,
                                ig.LEDActualEleCon,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                ig.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Building,
                                OutputProcessor::EndUseCat::InteriorLights,
                                "IndoorLivingWall", // End Use subcategory
                                state.dataHeatBal->Zone(ig.ZonePtr).Name,
                                state.dataHeatBal->Zone(ig.ZonePtr).Multiplier,
                                state.dataHeatBal->Zone(ig.ZonePtr).ListMultiplier,
                                state.dataHeatBal->space(ig.SpacePtr).spaceType);
        }
    }

    void InitIndoorGreen(EnergyPlusData const &state)
    {
        // Set the reporting variables to zero at each timestep.
        for (auto &ig : state.dataIndoorGreen->indoorGreens) {
            ig.SensibleRate = 0.0;
            ig.SensibleRateLED = 0.0;
            ig.LatentRate = 0.0;
            ig.ZCO2 = 400;
            ig.ZPPFD = 0;
        }
    }

    void ETModel(EnergyPlusData &state)
    {
        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for the calculation of evapotranspiration effects from the Indoor Greenery System objects.
        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("ETModel: ");
        auto &lw = state.dataIndoorGreen;
        Real64 ZonePreTemp; // Indoor air temprature (C)
        Real64 ZonePreHum;  // Indoor humidity ratio (kg moisture / kg dry air)
        Real64 ZoneNewTemp; // Indoor air temprature (C) after ET
        Real64 ZoneNewHum;  // Indoor humidity ratio (kg moisture / kg dry air) after ET
        Real64 ZoneSatHum;  // Saturated humidity ratio
        Real64 ZoneCO2;     // Indoor zone co2 concentration (ppm)
        Real64 ZonePPFD;    // Indoor net radiation (PPFD)
        Real64 ZoneVPD;     // vapor pressure deficit (kpa); local variable
        Real64 Timestep;    // s
        Real64 ETTotal;     // kg
        Real64 rhoair;      // kg/m3
        Real64 Tdp;         // dew point temperature
        Real64 Twb;         // wet bulb temperature
        Real64 HCons;       // enthalpy (J/kg)
        Real64 HMid;        // enthalpy 3rd point (J/kg)
        Real64 ZoneAirVol;  // zone air volume (m3)
        Real64 LAI;         // leaf area index, the ratio of one-side leaf area per unit plant growing area, maximum LAI =2 if LAI_cal>2.0
        Real64 LAI_Cal;     // calculated leaf area index based on users's input on total leaf area
        Real64 OutPb;       // outdoor pressure (kPa)
        Real64 vp;          // actual vapor pressure of the air (kpa)
        Real64 vpSat;       // saturated vapor pressure at air temperature (kpa)
        Timestep = state.dataHVACGlobal->TimeStepSysSec; // unit s
        for (int IndoorGreenNum = 1; IndoorGreenNum <= lw->NumIndoorGreen; ++IndoorGreenNum) {
            auto &ig = lw->indoorGreens(IndoorGreenNum);
            ZonePreTemp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ig.ZonePtr).ZT;
            ZonePreHum = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ig.ZonePtr).airHumRat;
            ZoneCO2 = 400;
            OutPb = state.dataEnvrn->OutBaroPress / 1000;
            Tdp = Psychrometrics::PsyTdpFnWPb(state, ZonePreHum, OutPb * 1000);
            vp = Psychrometrics::PsyPsatFnTemp(state, Tdp, RoutineName) / 1000;
            vpSat = Psychrometrics::PsyPsatFnTemp(state, ZonePreTemp, RoutineName) / 1000;
            ig.ZVPD = (vpSat - vp) * 1000; // Pa
            LAI_Cal = ig.LeafArea / state.dataSurface->Surface(ig.SurfPtr).Area;
            LAI = LAI_Cal;
            if (LAI_Cal > 2.0) {
                LAI = 2.0; // maximum LAI=2.0 in the surface heat balance
                ShowSevereError(state, format("Maximum indoor living wall leaf area index (LAI) =2.0 is used,calculated LAI is {}", LAI_Cal));
            }
            switch (ig.lightingMethod) {
            case LightingMethod::LED: {
                ig.ZPPFD = ig.ledSched->getCurrentVal() * ig.LEDNominalPPFD; // PPFD
                ig.LEDActualPPFD = ig.ZPPFD;
                ig.LEDActualEleP = ig.ledSched->getCurrentVal() * ig.LEDNominalEleP;
                ig.LEDActualEleCon = ig.LEDActualEleP * Timestep;
            } break;
            case LightingMethod::Daylighting: {
                ig.ZPPFD = 0;
                ig.LEDActualPPFD = 0;
                ig.LEDActualEleP = 0;
                ig.LEDActualEleCon = 0;
                if (!state.dataDayltg->CalcDayltghCoefficients_firstTime && state.dataEnvrn->SunIsUp) {
                    ig.ZPPFD = state.dataDayltg->daylightControl(ig.LightControlPtr).refPts(1).lums[DataSurfaces::iLum_Illum] /
                               77; // To be updated currently only take one reference point; 77 conversion factor from Lux to PPFD
                }
            } break;

            case LightingMethod::LEDDaylighting: {
                Real64 a = ig.ledDaylightTargetSched->getCurrentVal();
                Real64 b = 0;
                if (!state.dataDayltg->CalcDayltghCoefficients_firstTime && state.dataEnvrn->SunIsUp) {
                    b = state.dataDayltg->daylightControl(ig.LightControlPtr).refPts(1).lums[DataSurfaces::iLum_Illum] /
                        77; // To be updated currently only take one reference point; 77 conversion factor from Lux to PPFD
                }
                ig.LEDActualPPFD = max((a - b), 0.0);
                if (ig.LEDActualPPFD >= ig.LEDNominalPPFD) {
                    ig.ZPPFD = ig.LEDNominalPPFD + b; // LED Nominal + Daylight
                    ig.LEDActualEleP = ig.LEDNominalEleP;
                    ig.LEDActualEleCon = ig.LEDNominalEleP * Timestep;
                } else {
                    ig.ZPPFD = a; // Targeted PPFD
                    ig.LEDActualEleP = ig.LEDNominalEleP * ig.LEDActualPPFD / ig.LEDNominalPPFD;
                    ig.LEDActualEleCon = ig.LEDActualEleP * Timestep;
                }
            } break;
            default:
                break;
            }
            ZonePPFD = ig.ZPPFD;
            ZoneVPD = ig.ZVPD / 1000; // kPa
            // ET Calculation
            if (ig.EMSETCalOverrideOn) {
                ig.ETRate = ig.EMSET;
            } else {
                Real64 SwitchF = ig.etCalculationMethod == ETCalculationMethod::PenmanMonteith ? 1.0 : 2 * LAI;
                ig.ETRate = ETBaseFunction(state, ZonePreTemp, ZonePreHum, ZonePPFD, ZoneVPD, LAI, SwitchF);
            }
            Real64 effectivearea = std::min(ig.LeafArea, LAI * state.dataSurface->Surface(ig.SurfPtr).Area);
            ETTotal = ig.ETRate * Timestep * effectivearea *
                      ig.sched->getCurrentVal(); // kg; this unit area should be surface area instead of total leaf area
            Real64 hfg = Psychrometrics::PsyHfgAirFnWTdb(ZonePreHum, ZonePreTemp) / std::pow(10, 6); // Latent heat of vaporization (MJ/kg)
            ig.LambdaET = ETTotal * hfg * std::pow(10, 6) / state.dataSurface->Surface(ig.SurfPtr).Area / Timestep; // (W/m2))
            rhoair = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, ZonePreTemp, ZonePreHum);
            ZoneAirVol = state.dataHeatBal->Zone(ig.ZonePtr).Volume;
            ZoneNewHum = ZonePreHum + ETTotal / (rhoair * ZoneAirVol);
            Twb = Psychrometrics::PsyTwbFnTdbWPb(state, ZonePreTemp, ZonePreHum, state.dataEnvrn->OutBaroPress);
            ZoneSatHum = Psychrometrics::PsyWFnTdbRhPb(state, Twb, 1.0, state.dataEnvrn->OutBaroPress); // saturated humidity ratio
            HCons = Psychrometrics::PsyHFnTdbW(ZonePreTemp, ZonePreHum);
            if (ZoneNewHum <= ZoneSatHum) {
                ZoneNewTemp = Psychrometrics::PsyTdbFnHW(HCons, ZoneNewHum);
            } else {
                ZoneNewTemp = Twb;
                ZoneNewHum = ZoneSatHum;
            }
            HMid = Psychrometrics::PsyHFnTdbW(ZoneNewTemp, ZonePreHum);
            ig.LatentRate = ZoneAirVol * rhoair * (HCons - HMid) / Timestep; // unit W
            ig.SensibleRateLED = (1 - ig.LEDRadFraction) * ig.LEDActualEleP; // convective heat gain from LED lights when LED is on;
            ig.SensibleRate = -1.0 * ig.LatentRate + ig.SensibleRateLED;
            state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(ig.SurfPtr) =
                ig.LEDRadFraction * 0.9 * ig.LEDActualEleP /
                state.dataSurface->Surface(ig.SurfPtr).Area; // assume the energy from radiation for photosynthesis is only 10%.
        }
    }

    Real64 ETBaseFunction(EnergyPlusData &state, Real64 ZonePreTemp, Real64 ZonePreHum, Real64 ZonePPFD, Real64 ZoneVPD, Real64 LAI, Real64 SwitchF)
    {
        // This subroutine provides calculation for Penman-Monteith model and Stanghellini models to predict evapotranspiration rates of plants.
        // Reference: Monteith, J.L. Evaporation and environment. in Symposia of the society for experimental biology. 1965. Cambridge University
        // Press (CUP) Cambridge
        // Reference: Stanghellini, C., Transpiration of greenhouse crops: an aid to climate management, 1987, Institute of Agricultural Engineering,
        // Wageningen, The Netherlands

        Real64 hfg = Psychrometrics::PsyHfgAirFnWTdb(ZonePreHum, ZonePreTemp) / std::pow(10, 6); // Latent heat of vaporization (MJ/kg)
        // Slope of the saturation vapor pressure-temperature curve (kPa/°C)
        Real64 slopepat = 0.200 * std::pow((0.00738 * ZonePreTemp + 0.8072), 7) - 0.000116;
        Real64 CpAir = Psychrometrics::PsyCpAirFnW(ZonePreHum) / std::pow(10, 6); // specific heat of air at constant pressure (MJ kg−1 °C−1)
        Real64 OutPb = state.dataEnvrn->OutBaroPress / 1000;                      // outdoor pressure (kPa)
        Real64 constexpr mw(0.622);                                               // ratio molecular weight of water vapor / dry air = 0.622.
        Real64 psyconst = CpAir * OutPb / (hfg * mw);                             // Psychrometric constant (kPa/°C)
        Real64 In = ZonePPFD * 0.327 / std::pow(10, 6);                           // net radiation MW/m2
        Real64 G = 0.0;                                                           // soil heat flux (MJ/(m2s))
        Real64 rhoair = Psychrometrics::PsyRhoAirFnPbTdbW(state, OutPb * 1000, ZonePreTemp, ZonePreHum); // kg/m3
        Real64 ETRate;                                                                                   // mm/s; kg/(m2s)
        Real64 rs = 60 * (1500 + ZonePPFD) / (200 + ZonePPFD);                                           // stomatal resistance s/m
        Real64 ra = 350 * std::pow((0.1 / 0.1), 0.5) * (1 / (LAI + 1e-10));                              // aerodynamic resistance s/m
        ETRate = (1 / hfg) * (slopepat * (In - G) + (SwitchF * rhoair * CpAir * ZoneVPD) / ra) /
                 (slopepat + psyconst * (1 + rs / ra)); // Penman-Monteith ET model
        return ETRate;                                  // mm/s; kg/(m2s)
    }

} // namespace IndoorGreen

} // namespace EnergyPlus
