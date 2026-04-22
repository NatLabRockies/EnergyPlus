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

#include <EnergyPlus/Autosizing/SystemAirFlowSizing.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

Real64 SystemAirFlowSizer::size(EnergyPlusData &state, Real64 _originalValue, bool &errorsFound)
{

    if (!this->checkInitialized(state, errorsFound)) {
        return 0.0;
    }
    this->preSize(state, _originalValue);
    std::string DDNameFanPeak;
    std::string dateTimeFanPeak;

    if (this->dataEMSOverrideON) {
        this->autoSizedValue = this->dataEMSOverride;
    } else if (this->dataConstantUsedForSizing > 0.0 && this->dataFractionUsedForSizing > 0.0) {
        this->autoSizedValue = this->dataConstantUsedForSizing * this->dataFractionUsedForSizing;
    } else {
        if (this->curZoneEqNum > 0) {
            if (!this->wasAutoSized && !this->sizingDesRunThisZone) {
                this->autoSizedValue = _originalValue;
            } else if (this->zoneEqSizing(this->curZoneEqNum).DesignSizeFromParent) {
                this->autoSizedValue = this->zoneEqSizing(this->curZoneEqNum).AirVolFlow;
            } else {
                if ((this->zoneAirFlowSizMethod == DataSizing::SupplyAirFlowRate) || (this->zoneAirFlowSizMethod == DataSizing::None)) {

                    if (this->zoneEqSizing(this->curZoneEqNum).SystemAirFlow) {
                        this->autoSizedValue = max(this->zoneEqSizing(this->curZoneEqNum).AirVolFlow,
                                                   this->finalZoneSizing(this->curZoneEqNum).DesCoolVolFlow,
                                                   this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow);
                        if (this->autoSizedValue == this->finalZoneSizing(this->curZoneEqNum).DesCoolVolFlow) {
                            if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                            }
                        } else if (this->autoSizedValue == this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow) {
                            if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                            }
                        } else if (this->autoSizedValue == this->zoneEqSizing(this->curZoneEqNum).AirVolFlow) {
                            DDNameFanPeak = "Unknown";
                        }
                    } else {
                        if (this->zoneCoolingOnlyFan) {
                            this->autoSizedValue = this->finalZoneSizing(this->curZoneEqNum).DesCoolVolFlow;
                            if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                            }
                        } else if (this->zoneHeatingOnlyFan) {
                            this->autoSizedValue = this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow;
                            if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                            }
                        } else if (this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow && !this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow) {
                            this->autoSizedValue = this->zoneEqSizing(this->curZoneEqNum).CoolingAirVolFlow;
                            if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                            }
                        } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow && !this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow) {
                            this->autoSizedValue = this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow;
                            if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                            }
                        } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow && this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow) {
                            this->autoSizedValue = max(this->zoneEqSizing(this->curZoneEqNum).CoolingAirVolFlow,
                                                       this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow);
                            if (this->autoSizedValue == this->zoneEqSizing(this->curZoneEqNum).CoolingAirVolFlow) {
                                if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                    this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                    DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                    dateTimeFanPeak = EnergyPlus::format(
                                        "{}/{} {}",
                                        state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                        state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                        ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                                }
                            } else if (this->autoSizedValue == this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow) {
                                if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                    this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                    DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                    dateTimeFanPeak = EnergyPlus::format(
                                        "{}/{} {}",
                                        state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                        state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                        ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                                }
                            }
                        } else {
                            this->autoSizedValue = max(this->finalZoneSizing(this->curZoneEqNum).DesCoolVolFlow,
                                                       this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow);
                            if (this->autoSizedValue == this->finalZoneSizing(this->curZoneEqNum).DesCoolVolFlow) {
                                if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                    this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                    DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                    dateTimeFanPeak = EnergyPlus::format(
                                        "{}/{} {}",
                                        state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                        state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                        ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                                }
                            } else if (this->autoSizedValue == this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow) {
                                if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                    this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                    DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                    dateTimeFanPeak = EnergyPlus::format(
                                        "{}/{} {}",
                                        state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                        state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                        ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                                }
                            }
                        }
                    }
                } else if (this->zoneAirFlowSizMethod == DataSizing::FractionOfAutosizedCoolingAirflow) {
                    if (this->zoneCoolingOnlyFan) {
                        this->autoSizedValue = this->dataFracOfAutosizedCoolingAirflow * this->finalZoneSizing(this->curZoneEqNum).DesCoolVolFlow;
                        if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                        }
                    } else if (this->zoneHeatingOnlyFan) {
                        this->autoSizedValue = this->dataFracOfAutosizedHeatingAirflow * this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow;
                        if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                        }
                    } else if (this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow && !this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow) {
                        this->autoSizedValue = this->dataFracOfAutosizedCoolingAirflow * this->zoneEqSizing(this->curZoneEqNum).CoolingAirVolFlow;
                        if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                        }
                    } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow && !this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow) {
                        this->autoSizedValue = this->dataFracOfAutosizedHeatingAirflow * this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow;
                        if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                        }
                    } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow && this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow) {
                        this->autoSizedValue =
                            max(this->dataFracOfAutosizedCoolingAirflow * this->zoneEqSizing(this->curZoneEqNum).CoolingAirVolFlow,
                                this->dataFracOfAutosizedHeatingAirflow * this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow);
                        if (this->autoSizedValue ==
                            this->dataFracOfAutosizedCoolingAirflow * this->zoneEqSizing(this->curZoneEqNum).CoolingAirVolFlow) {
                            if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                            }
                        } else if (this->autoSizedValue ==
                                   this->dataFracOfAutosizedHeatingAirflow * this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow) {
                            if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                            }
                        }
                    } else {
                        this->autoSizedValue =
                            max(this->dataFracOfAutosizedCoolingAirflow * this->finalZoneSizing(this->curZoneEqNum).DesCoolVolFlow,
                                this->dataFracOfAutosizedHeatingAirflow * this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow);
                        if (this->autoSizedValue ==
                            this->dataFracOfAutosizedCoolingAirflow * this->finalZoneSizing(this->curZoneEqNum).DesCoolVolFlow) {
                            if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                            }
                        } else if (this->autoSizedValue ==
                                   this->dataFracOfAutosizedHeatingAirflow * this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow) {
                            if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                            }
                        }
                    }
                } else if (this->zoneAirFlowSizMethod == DataSizing::FractionOfAutosizedHeatingAirflow) {
                    if (this->zoneCoolingOnlyFan) {
                        this->autoSizedValue = this->dataFracOfAutosizedCoolingAirflow * this->finalZoneSizing(this->curZoneEqNum).DesCoolVolFlow;
                        if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                        }
                    } else if (this->zoneHeatingOnlyFan) {
                        this->autoSizedValue = this->dataFracOfAutosizedHeatingAirflow * this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow;
                        if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                        }
                    } else if (this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow && !this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow) {
                        this->autoSizedValue = this->dataFracOfAutosizedCoolingAirflow * this->zoneEqSizing(this->curZoneEqNum).CoolingAirVolFlow;
                        if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                        }
                    } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow && !this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow) {
                        this->autoSizedValue = this->dataFracOfAutosizedHeatingAirflow * this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow;
                        if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                        }
                    } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow && this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow) {
                        this->autoSizedValue =
                            max(this->dataFracOfAutosizedCoolingAirflow * this->zoneEqSizing(this->curZoneEqNum).CoolingAirVolFlow,
                                this->dataFracOfAutosizedHeatingAirflow * this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow);
                        if (this->autoSizedValue ==
                            this->dataFracOfAutosizedCoolingAirflow * this->zoneEqSizing(this->curZoneEqNum).CoolingAirVolFlow) {
                            if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                            }
                        } else if (this->autoSizedValue ==
                                   this->dataFracOfAutosizedHeatingAirflow * this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow) {
                            if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                            }
                        }
                    } else {
                        this->autoSizedValue =
                            max(this->dataFracOfAutosizedCoolingAirflow * this->finalZoneSizing(this->curZoneEqNum).DesCoolVolFlow,
                                this->dataFracOfAutosizedHeatingAirflow * this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow);
                        if (this->autoSizedValue ==
                            this->dataFracOfAutosizedCoolingAirflow * this->finalZoneSizing(this->curZoneEqNum).DesCoolVolFlow) {
                            if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                            }
                        } else if (this->autoSizedValue ==
                                   this->dataFracOfAutosizedHeatingAirflow * this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow) {
                            if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                            }
                        }
                    }
                } else if (this->zoneAirFlowSizMethod == DataSizing::FlowPerCoolingCapacity) {
                    if (this->zoneCoolingOnlyFan) {
                        this->autoSizedValue = this->dataFlowPerCoolingCapacity * this->dataAutosizedCoolingCapacity;
                        if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                        }
                    } else if (this->zoneHeatingOnlyFan) {
                        this->autoSizedValue = this->dataFlowPerHeatingCapacity * this->dataAutosizedHeatingCapacity;
                        if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                        }
                    } else if (this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow && !this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow) {
                        this->autoSizedValue = this->dataFlowPerCoolingCapacity * this->dataAutosizedCoolingCapacity;
                        if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                        }
                    } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow && !this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow) {
                        this->autoSizedValue = this->dataFlowPerHeatingCapacity * this->dataAutosizedHeatingCapacity;
                        if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                        }
                    } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow && this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow) {
                        this->autoSizedValue = max(this->dataFlowPerCoolingCapacity * this->dataAutosizedCoolingCapacity,
                                                   this->dataFlowPerHeatingCapacity * this->dataAutosizedHeatingCapacity);
                        if (this->autoSizedValue == this->dataFlowPerCoolingCapacity * this->dataAutosizedCoolingCapacity) {
                            if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                            }
                        } else if (this->autoSizedValue == this->dataFlowPerHeatingCapacity * this->dataAutosizedHeatingCapacity) {
                            if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                            }
                        }
                    } else {
                        this->autoSizedValue = max(this->dataFlowPerCoolingCapacity * this->dataAutosizedCoolingCapacity,
                                                   this->dataFlowPerHeatingCapacity * this->dataAutosizedHeatingCapacity);
                        if (this->autoSizedValue == this->dataFlowPerCoolingCapacity * this->dataAutosizedCoolingCapacity) {
                            if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                            }
                        } else if (this->autoSizedValue == this->dataFlowPerHeatingCapacity * this->dataAutosizedHeatingCapacity) {
                            if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                            }
                        }
                    }
                } else if (this->zoneAirFlowSizMethod == DataSizing::FlowPerHeatingCapacity) {
                    if (this->zoneCoolingOnlyFan) {
                        this->autoSizedValue = this->dataFlowPerCoolingCapacity * this->dataAutosizedCoolingCapacity;
                        if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                        }
                    } else if (this->zoneHeatingOnlyFan) {
                        this->autoSizedValue = this->dataFlowPerHeatingCapacity * this->dataAutosizedHeatingCapacity;
                        if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                        }
                    } else if (this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow && !this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow) {
                        this->autoSizedValue = this->dataFlowPerCoolingCapacity * this->dataAutosizedCoolingCapacity;
                        if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                        }
                    } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow && !this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow) {
                        this->autoSizedValue = this->dataFlowPerHeatingCapacity * this->dataAutosizedHeatingCapacity;
                        if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                        }
                    } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow && this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow) {
                        this->autoSizedValue = max(this->dataFlowPerCoolingCapacity * this->dataAutosizedCoolingCapacity,
                                                   this->dataFlowPerHeatingCapacity * this->dataAutosizedHeatingCapacity);
                        if (this->autoSizedValue == this->dataFlowPerCoolingCapacity * this->dataAutosizedCoolingCapacity) {
                            if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                            }
                        } else if (this->autoSizedValue == this->dataFlowPerHeatingCapacity * this->dataAutosizedHeatingCapacity) {
                            if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                            }
                        }
                    } else {
                        this->autoSizedValue = max(this->dataFlowPerCoolingCapacity * this->dataAutosizedCoolingCapacity,
                                                   this->dataFlowPerHeatingCapacity * this->dataAutosizedHeatingCapacity);
                        if (this->autoSizedValue == this->dataFlowPerCoolingCapacity * this->dataAutosizedCoolingCapacity) {
                            if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                            }
                        } else if (this->autoSizedValue == this->dataFlowPerHeatingCapacity * this->dataAutosizedHeatingCapacity) {
                            if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                            }
                        }
                    }
                } else {
                    if (this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow && !this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow) {
                        this->autoSizedValue = this->zoneEqSizing(this->curZoneEqNum).CoolingAirVolFlow;
                        if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                        }
                    } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow && !this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow) {
                        this->autoSizedValue = this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow;
                        if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                            this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                            DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                            dateTimeFanPeak = EnergyPlus::format(
                                "{}/{} {}",
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                        }
                    } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow && this->zoneEqSizing(this->curZoneEqNum).CoolingAirFlow) {
                        this->autoSizedValue =
                            max(this->zoneEqSizing(this->curZoneEqNum).CoolingAirVolFlow, this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow);
                        if (this->autoSizedValue == this->zoneEqSizing(this->curZoneEqNum).CoolingAirVolFlow) {
                            if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                            }
                        } else if (this->autoSizedValue == this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow) {
                            if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                            }
                        }
                    } else {
                        if (this->zoneEqSizing(this->curZoneEqNum).SystemAirFlow) {
                            this->autoSizedValue = max(this->zoneEqSizing(this->curZoneEqNum).AirVolFlow,
                                                       this->zoneEqSizing(this->curZoneEqNum).CoolingAirVolFlow,
                                                       this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow);
                        } else if (this->zoneCoolingOnlyFan) {
                            this->autoSizedValue = this->finalZoneSizing(this->curZoneEqNum).DesCoolVolFlow;
                            if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                            }
                        } else if (this->zoneHeatingOnlyFan) {
                            this->autoSizedValue = this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow;
                            if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                dateTimeFanPeak = EnergyPlus::format(
                                    "{}/{} {}",
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                    state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                    ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                            }
                        } else {
                            this->autoSizedValue = max(this->finalZoneSizing(this->curZoneEqNum).DesCoolVolFlow,
                                                       this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow);
                            if (this->autoSizedValue == this->finalZoneSizing(this->curZoneEqNum).DesCoolVolFlow) {
                                if (this->finalZoneSizing(this->curZoneEqNum).CoolDDNum > 0 &&
                                    this->finalZoneSizing(this->curZoneEqNum).CoolDDNum <= state.dataEnvrn->TotDesDays) {
                                    DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Title;
                                    dateTimeFanPeak = EnergyPlus::format(
                                        "{}/{} {}",
                                        state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).Month,
                                        state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).CoolDDNum).DayOfMonth,
                                        ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtCoolMax));
                                }
                            } else if (this->autoSizedValue == this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow) {
                                if (this->finalZoneSizing(this->curZoneEqNum).HeatDDNum > 0 &&
                                    this->finalZoneSizing(this->curZoneEqNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                    DDNameFanPeak = state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Title;
                                    dateTimeFanPeak = EnergyPlus::format(
                                        "{}/{} {}",
                                        state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).Month,
                                        state.dataWeather->DesDayInput(this->finalZoneSizing(this->curZoneEqNum).HeatDDNum).DayOfMonth,
                                        ReportCoilSelection::getTimeText(state, this->finalZoneSizing(this->curZoneEqNum).TimeStepNumAtHeatMax));
                                }
                            }
                        }
                    }
                    if (this->dataFractionUsedForSizing > 0.0) {
                        this->autoSizedValue = this->autoSizedValue * this->dataFractionUsedForSizing;
                    }
                }
            }
        } else if (this->curSysNum > 0) {
            if (!this->wasAutoSized && !this->sizingDesRunThisAirSys) {
                this->autoSizedValue = _originalValue;
            } else {
                if (this->dataHRFlowSizingFlag) { // HX sizing
                    if (this->curOASysNum != 0) {
                        // size to supply air duct flow rate
                        if (this->outsideAirSys(this->curOASysNum).AirLoopDOASNum > -1) {
                            auto const &thisAirloopDOAS = this->airloopDOAS[this->outsideAirSys(this->curOASysNum).AirLoopDOASNum];
                            this->autoSizedValue = thisAirloopDOAS.SizingMassFlow / state.dataEnvrn->StdRhoAir;
                        } else if (this->finalSysSizing(this->curSysNum).DesOutAirVolFlow > 0.0) {
                            this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesOutAirVolFlow;
                        } else {
                            switch (this->curDuctType) {
                            case HVAC::AirDuctType::Main: {
                                this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                            } break;
                            case HVAC::AirDuctType::Cooling: {
                                this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesCoolVolFlow;
                            } break;
                            case HVAC::AirDuctType::Heating: {
                                this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesHeatVolFlow;
                            } break;
                            case HVAC::AirDuctType::Other: {
                                this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                            } break;
                            default: {
                                this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                            } break;
                            }
                        }
                    } else {
                        switch (this->curDuctType) {
                        case HVAC::AirDuctType::Main: {
                            this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                        } break;
                        case HVAC::AirDuctType::Cooling: {
                            this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesCoolVolFlow;
                        } break;
                        case HVAC::AirDuctType::Heating: {
                            this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesHeatVolFlow;
                        } break;

                        case HVAC::AirDuctType::Other: {
                            this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                        } break;
                        default: {
                            this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                        } break;
                        }
                    }

                } else {
                    if (this->curOASysNum > 0) {
                        // should a parent object ever override DOAS or OASystem flow rate (e.g., using oaSysEqSizing().AirVolFlow)?
                        if (outsideAirSys(this->curOASysNum).AirLoopDOASNum > -1) {
                            this->autoSizedValue =
                                this->airloopDOAS[outsideAirSys(this->curOASysNum).AirLoopDOASNum].SizingMassFlow / state.dataEnvrn->StdRhoAir;
                        } else {
                            switch (this->curDuctType) {
                            case HVAC::AirDuctType::Cooling: {
                                this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesCoolVolFlow;
                            } break;
                            case HVAC::AirDuctType::Heating: {
                                this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesHeatVolFlow;
                            } break;
                            case HVAC::AirDuctType::Main:
                            case HVAC::AirDuctType::Other:
                            default: {
                                this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                            } break;
                            }
                            if (this->autoSizedValue == this->finalSysSizing(this->curSysNum).DesHeatVolFlow) {
                                if (this->finalSysSizing(this->curSysNum).HeatDDNum > 0 &&
                                    this->finalSysSizing(this->curSysNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                    auto &desDayInput = state.dataWeather->DesDayInput(this->finalSysSizing(this->curSysNum).HeatDDNum);
                                    DDNameFanPeak = desDayInput.Title;
                                    dateTimeFanPeak = EnergyPlus::format(
                                        "{}/{} {}",
                                        desDayInput.Month,
                                        desDayInput.DayOfMonth,
                                        ReportCoilSelection::getTimeText(state, this->finalSysSizing(this->curSysNum).SysHeatAirTimeStepPk));
                                }
                            } else if (this->autoSizedValue == this->finalSysSizing(this->curSysNum).DesCoolVolFlow) {
                                auto &sysSizPeakDDNum = state.dataSize->SysSizPeakDDNum(this->curSysNum);
                                if (sysSizPeakDDNum.CoolFlowPeakDD > 0 && sysSizPeakDDNum.CoolFlowPeakDD <= state.dataEnvrn->TotDesDays) {
                                    auto &desDayInput = state.dataWeather->DesDayInput(sysSizPeakDDNum.CoolFlowPeakDD);
                                    DDNameFanPeak = desDayInput.Title;
                                    dateTimeFanPeak =
                                        EnergyPlus::format("{}/{} {}",
                                                           desDayInput.Month,
                                                           desDayInput.DayOfMonth,
                                                           ReportCoilSelection::getTimeText(
                                                               state, sysSizPeakDDNum.TimeStepAtCoolFlowPk(sysSizPeakDDNum.CoolFlowPeakDD)));
                                }
                            }
                        }
                    } else {
                        auto &sysSizPeakDDNum = state.dataSize->SysSizPeakDDNum(this->curSysNum);

                        if (this->airLoopSysFlag) {
                            auto const &unitarysysEqSizing = this->unitarySysEqSizing(this->curSysNum);
                            if (unitarysysEqSizing.CoolingAirFlow && unitarysysEqSizing.HeatingAirFlow) {
                                this->autoSizedValue = std::max(unitarysysEqSizing.CoolingAirVolFlow, unitarysysEqSizing.HeatingAirVolFlow);
                                if (this->autoSizedValue == unitarysysEqSizing.CoolingAirVolFlow) {
                                    if (sysSizPeakDDNum.CoolFlowPeakDD > 0 && sysSizPeakDDNum.CoolFlowPeakDD <= state.dataEnvrn->TotDesDays) {
                                        auto &desDayInput = state.dataWeather->DesDayInput(sysSizPeakDDNum.CoolFlowPeakDD);
                                        DDNameFanPeak = desDayInput.Title;
                                        dateTimeFanPeak =
                                            EnergyPlus::format("{}/{} {}",
                                                               desDayInput.Month,
                                                               desDayInput.DayOfMonth,
                                                               ReportCoilSelection::getTimeText(
                                                                   state, sysSizPeakDDNum.TimeStepAtCoolFlowPk(sysSizPeakDDNum.CoolFlowPeakDD)));
                                    }

                                } else if (this->autoSizedValue == unitarysysEqSizing.HeatingAirVolFlow) {
                                    if (this->finalSysSizing(this->curSysNum).HeatDDNum > 0 &&
                                        this->finalSysSizing(this->curSysNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                        auto &desDayInput = state.dataWeather->DesDayInput(this->finalSysSizing(this->curSysNum).HeatDDNum);
                                        DDNameFanPeak = desDayInput.Title;
                                        dateTimeFanPeak = EnergyPlus::format(
                                            "{}/{} {}",
                                            desDayInput.Month,
                                            desDayInput.DayOfMonth,
                                            ReportCoilSelection::getTimeText(state, this->finalSysSizing(this->curSysNum).SysHeatAirTimeStepPk));
                                    }
                                }
                            } else if (unitarysysEqSizing.CoolingAirFlow) {
                                this->autoSizedValue = unitarysysEqSizing.CoolingAirVolFlow;
                                if (sysSizPeakDDNum.CoolFlowPeakDD > 0 && sysSizPeakDDNum.CoolFlowPeakDD <= state.dataEnvrn->TotDesDays) {
                                    auto &desDayInput = state.dataWeather->DesDayInput(sysSizPeakDDNum.CoolFlowPeakDD);
                                    DDNameFanPeak = state.dataWeather->DesDayInput(sysSizPeakDDNum.CoolFlowPeakDD).Title;
                                    dateTimeFanPeak =
                                        EnergyPlus::format("{}/{} {}",
                                                           desDayInput.Month,
                                                           desDayInput.DayOfMonth,
                                                           ReportCoilSelection::getTimeText(
                                                               state, sysSizPeakDDNum.TimeStepAtCoolFlowPk(sysSizPeakDDNum.CoolFlowPeakDD)));
                                }
                            } else if (unitarysysEqSizing.HeatingAirFlow) {
                                this->autoSizedValue = unitarysysEqSizing.HeatingAirVolFlow;
                                if (this->finalSysSizing(this->curSysNum).HeatDDNum > 0 &&
                                    this->finalSysSizing(this->curSysNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                    auto &desDayInput = state.dataWeather->DesDayInput(this->finalSysSizing(this->curSysNum).HeatDDNum);
                                    DDNameFanPeak = desDayInput.Title;
                                    dateTimeFanPeak = EnergyPlus::format(
                                        "{}/{} {}",
                                        desDayInput.Month,
                                        desDayInput.DayOfMonth,
                                        ReportCoilSelection::getTimeText(state, this->finalSysSizing(this->curSysNum).SysHeatAirTimeStepPk));
                                }

                            } else {
                                this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                                if (this->autoSizedValue == this->finalSysSizing(this->curSysNum).DesHeatVolFlow) {
                                    if (this->finalSysSizing(this->curSysNum).HeatDDNum > 0 &&
                                        this->finalSysSizing(this->curSysNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                        auto &desDayInput = state.dataWeather->DesDayInput(this->finalSysSizing(this->curSysNum).HeatDDNum);
                                        DDNameFanPeak = desDayInput.Title;
                                        dateTimeFanPeak = EnergyPlus::format(
                                            "{}/{} {}",
                                            desDayInput.Month,
                                            desDayInput.DayOfMonth,
                                            ReportCoilSelection::getTimeText(state, this->finalSysSizing(this->curSysNum).SysHeatAirTimeStepPk));
                                    }
                                } else if (this->autoSizedValue == this->finalSysSizing(this->curSysNum).DesCoolVolFlow) {
                                    if (sysSizPeakDDNum.CoolFlowPeakDD > 0 && sysSizPeakDDNum.CoolFlowPeakDD <= state.dataEnvrn->TotDesDays) {
                                        auto &desDayInput = state.dataWeather->DesDayInput(sysSizPeakDDNum.CoolFlowPeakDD);
                                        DDNameFanPeak = desDayInput.Title;
                                        dateTimeFanPeak =
                                            EnergyPlus::format("{}/{} {}",
                                                               desDayInput.Month,
                                                               desDayInput.DayOfMonth,
                                                               ReportCoilSelection::getTimeText(
                                                                   state, sysSizPeakDDNum.TimeStepAtCoolFlowPk(sysSizPeakDDNum.CoolFlowPeakDD)));
                                    }
                                }
                            }
                        } else {
                            this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                            if (this->autoSizedValue == this->finalSysSizing(this->curSysNum).DesHeatVolFlow) {
                                if (this->finalSysSizing(this->curSysNum).HeatDDNum > 0 &&
                                    this->finalSysSizing(this->curSysNum).HeatDDNum <= state.dataEnvrn->TotDesDays) {
                                    auto &desDayInput = state.dataWeather->DesDayInput(this->finalSysSizing(this->curSysNum).HeatDDNum);
                                    DDNameFanPeak = desDayInput.Title;
                                    dateTimeFanPeak = EnergyPlus::format(
                                        "{}/{} {}",
                                        desDayInput.Month,
                                        desDayInput.DayOfMonth,
                                        ReportCoilSelection::getTimeText(state, this->finalSysSizing(this->curSysNum).SysHeatAirTimeStepPk));
                                }
                            } else if (this->autoSizedValue == this->finalSysSizing(this->curSysNum).DesCoolVolFlow) {
                                if (sysSizPeakDDNum.CoolFlowPeakDD > 0 && sysSizPeakDDNum.CoolFlowPeakDD <= state.dataEnvrn->TotDesDays) {
                                    auto &desDayInput = state.dataWeather->DesDayInput(sysSizPeakDDNum.CoolFlowPeakDD);
                                    DDNameFanPeak = desDayInput.Title;
                                    dateTimeFanPeak =
                                        EnergyPlus::format("{}/{} {}",
                                                           desDayInput.Month,
                                                           desDayInput.DayOfMonth,
                                                           ReportCoilSelection::getTimeText(
                                                               state, sysSizPeakDDNum.TimeStepAtCoolFlowPk(sysSizPeakDDNum.CoolFlowPeakDD)));
                                }
                            }
                        }
                        if (this->dataFractionUsedForSizing > 0.0) {
                            this->autoSizedValue = this->autoSizedValue * this->dataFractionUsedForSizing;
                        }
                    }
                }
            }
        } else if (this->dataNonZoneNonAirloopValue > 0) {
            this->autoSizedValue = this->dataNonZoneNonAirloopValue;
        } else if (!this->wasAutoSized) {
            this->autoSizedValue = this->originalValue;
        } else {
            std::string msg = this->callingRoutine + ' ' + this->compType + ' ' + this->compName + ", Developer Error: Component sizing incomplete.";
            ShowSevereError(state, msg);
            this->addErrorMessage(msg);
            msg = EnergyPlus::format("SizingString = {}, SizingResult = {:.1T}", this->sizingString, this->autoSizedValue);
            ShowContinueError(state, msg);
            this->addErrorMessage(msg);
            errorsFound = true;
        }

        if (this->dataScalableSizingON) {
            if (this->zoneAirFlowSizMethod == DataSizing::SupplyAirFlowRate || this->zoneAirFlowSizMethod == DataSizing::None) {
                this->sizingStringScalable = "(scaled by flow / zone) ";
            } else if (this->zoneAirFlowSizMethod == DataSizing::FlowPerFloorArea) {
                this->sizingStringScalable = "(scaled by flow / area) ";
            } else if (this->zoneAirFlowSizMethod == DataSizing::FractionOfAutosizedCoolingAirflow ||
                       this->zoneAirFlowSizMethod == DataSizing::FractionOfAutosizedHeatingAirflow) {
                this->sizingStringScalable = "(scaled by fractional multiplier) ";
            } else if (this->zoneAirFlowSizMethod == DataSizing::FlowPerCoolingCapacity ||
                       this->zoneAirFlowSizMethod == DataSizing::FlowPerHeatingCapacity) {
                this->sizingStringScalable = "(scaled by flow / capacity) ";
            }
        }
    }

    // override sizing string
    if (this->overrideSizeString) {
        if (Util::SameString(this->compType, "ZoneHVAC:FourPipeFanCoil")) {
            this->sizingString = "Maximum Supply Air Flow Rate [m3/s]";
            if (this->isEpJSON) {
                this->sizingString = "maximum_supply_air_flow_rate [m3/s]";
            }
        } else if (Util::SameString(this->compType, "ZoneHVAC:UnitVentilator")) {
            this->sizingString = "Maximum Supply Air Flow Rate [m3/s]";
            if (this->isEpJSON) {
                this->sizingString = "maximum_supply_air_flow_rate [m3/s]";
            }
        } else if (Util::SameString(this->compType, "Fan:SystemModel")) {
            this->sizingString = "Design Maximum Air Flow Rate [m3/s]";
            // if (this->isEpJSON) this->sizingString = "design_maximum_air_flow_rate [m3/s]";
        } else {
            if (this->isEpJSON) {
                this->sizingString = "supply_air_maximum_flow_rate [m3/s]";
            }
        }
    }

    this->selectSizerOutput(state, errorsFound);
    if (this->isFanReportObject) {
        //  fill fan peak day and time here
        if (ReportCoilSelection::isCompTypeFan(this->compType)) {
            if (this->dataScalableSizingON) {
                DDNameFanPeak = "Scaled size, not from any peak";
                dateTimeFanPeak = "Scaled size, not from any peak";
            }
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanDesDay, this->compName, DDNameFanPeak);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanPkTime, this->compName, dateTimeFanPeak);
        }
    }
    return this->autoSizedValue;
}

void SystemAirFlowSizer::clearState()
{
    BaseSizerWithScalableInputs::clearState();
}

} // namespace EnergyPlus
