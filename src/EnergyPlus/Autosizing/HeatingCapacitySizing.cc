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

#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

Real64 HeatingCapacitySizer::size(EnergyPlusData &state, Real64 _originalValue, bool &errorsFound)
{
    if (!this->checkInitialized(state, errorsFound)) {
        return 0.0;
    }
    this->preSize(state, _originalValue);
    Real64 DesVolFlow = 0.0;
    Real64 CoilInTemp = -999.0;
    Real64 CoilInHumRat = -999.0;
    Real64 CoilOutTemp = -999.0;
    Real64 CoilOutHumRat = -999.0;

    Real64 DXFlowPerCapMinRatio = 1.0;
    Real64 DXFlowPerCapMaxRatio = 1.0;
    Real64 NominalCapacityDes = 0.0;
    Real64 DesMassFlow = 0.0;
    Real64 DesCoilLoad = 0.0;
    Real64 OutAirFrac = 0.0;
    Real64 const CpAirStd = Psychrometrics::PsyCpAirFnW(0.0);

    if (this->dataEMSOverrideON) {
        this->autoSizedValue = this->dataEMSOverride;
    } else if (this->dataConstantUsedForSizing >= 0 && this->dataFractionUsedForSizing > 0) {
        // back and forth if dataConstantUsedForSizing should be > or >= 0 to make this work for AutoCalculate
        this->autoSizedValue = this->dataConstantUsedForSizing * this->dataFractionUsedForSizing;
    } else {
        if (this->curZoneEqNum > 0) {
            if (!this->wasAutoSized && !this->sizingDesRunThisZone) {
                this->autoSizedValue = _originalValue;
            } else if (this->zoneEqSizing(this->curZoneEqNum).DesignSizeFromParent) {
                this->autoSizedValue = this->zoneEqSizing(this->curZoneEqNum).DesHeatingLoad;
            } else {
                if (this->dataCoilIsSuppHeater && this->suppHeatCap > 0.0) {
                    NominalCapacityDes = this->suppHeatCap;
                    if (this->dataFlowUsedForSizing > 0.0) {
                        DesVolFlow = this->dataFlowUsedForSizing;
                    }
                } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingCapacity) {
                    NominalCapacityDes = this->zoneEqSizing(this->curZoneEqNum).DesHeatingLoad;
                    if (this->dataFlowUsedForSizing > 0.0) {
                        DesVolFlow = this->dataFlowUsedForSizing;
                    }
                } else if (this->dataCoolCoilCap > 0.0 && this->dataFlowUsedForSizing > 0.0) {
                    NominalCapacityDes = this->dataCoolCoilCap;
                    DesVolFlow = this->dataFlowUsedForSizing;
                } else if (int(this->finalZoneSizing.size()) > 0 &&
                           this->finalZoneSizing(this->curZoneEqNum).DesHeatMassFlow >= HVAC::SmallMassFlow) {
                    if (this->dataFlowUsedForSizing > 0.0) {
                        DesVolFlow = this->dataFlowUsedForSizing;
                    }
                    if (this->termUnitPIU && (this->curTermUnitSizingNum > 0)) {
                        Real64 const MinPriFlowFrac = this->termUnitSizing(this->curTermUnitSizingNum).MinPriFlowFrac;
                        if (this->termUnitSizing(this->curTermUnitSizingNum).InducesPlenumAir) {
                            CoilInTemp = (this->termUnitFinalZoneSizing(this->curTermUnitSizingNum).DesHeatCoilInTempTU * MinPriFlowFrac) +
                                         (this->termUnitFinalZoneSizing(this->curTermUnitSizingNum).ZoneRetTempAtHeatPeak * (1.0 - MinPriFlowFrac));
                        } else {
                            CoilInTemp = (this->termUnitFinalZoneSizing(this->curTermUnitSizingNum).DesHeatCoilInTempTU * MinPriFlowFrac) +
                                         (this->termUnitFinalZoneSizing(this->curTermUnitSizingNum).ZoneTempAtHeatPeak * (1.0 - MinPriFlowFrac));
                        }
                    } else if (this->termUnitIU && (this->curTermUnitSizingNum > 0)) {
                        CoilInTemp = this->termUnitFinalZoneSizing(this->curTermUnitSizingNum).ZoneTempAtHeatPeak;
                        CoilInHumRat = this->termUnitFinalZoneSizing(this->curTermUnitSizingNum).ZoneHumRatAtHeatPeak;
                    } else if (this->termUnitSingDuct && (this->curTermUnitSizingNum > 0)) {
                        CoilInTemp = this->termUnitFinalZoneSizing(this->curTermUnitSizingNum).DesHeatCoilInTempTU;
                        CoilInHumRat = this->termUnitFinalZoneSizing(this->curTermUnitSizingNum).DesHeatCoilInHumRatTU;
                    } else {
                        // all other components calculate coil inlet condition based on actual, not design, OA fraction (e.g., FanCoil)
                        if (DesVolFlow > 0.0) {
                            DesMassFlow = DesVolFlow * state.dataEnvrn->StdRhoAir;
                        } else {
                            DesMassFlow = this->finalZoneSizing(this->curZoneEqNum).DesHeatMassFlow;
                        }
                        CoilInTemp = this->setHeatCoilInletTempForZoneEqSizing(
                            this->setOAFracForZoneEqSizing(state, DesMassFlow, zoneEqSizing(this->curZoneEqNum)),
                            zoneEqSizing(this->curZoneEqNum),
                            finalZoneSizing(this->curZoneEqNum));
                        CoilInHumRat = this->setHeatCoilInletHumRatForZoneEqSizing(
                            this->setOAFracForZoneEqSizing(state, DesMassFlow, zoneEqSizing(this->curZoneEqNum)),
                            zoneEqSizing(this->curZoneEqNum),
                            finalZoneSizing(this->curZoneEqNum));
                    }
                    if ((this->termUnitSingDuct || this->termUnitPIU) && (this->curTermUnitSizingNum > 0)) {
                        CoilOutTemp = this->termUnitFinalZoneSizing(this->curTermUnitSizingNum).HeatDesTemp;
                        CoilOutHumRat = this->termUnitFinalZoneSizing(this->curTermUnitSizingNum).HeatDesHumRat;
                        Real64 const CpAir = Psychrometrics::PsyCpAirFnW(CoilOutHumRat);
                        DesCoilLoad = CpAir * state.dataEnvrn->StdRhoAir * this->termUnitSizing(this->curTermUnitSizingNum).AirVolFlow *
                                      (CoilOutTemp - CoilInTemp);
                        DesVolFlow = this->termUnitSizing(this->curTermUnitSizingNum).AirVolFlow;
                    } else if (this->termUnitIU && (this->curTermUnitSizingNum > 0)) {
                        if (this->termUnitSizing(this->curTermUnitSizingNum).InducRat > 0.01) {
                            DesVolFlow = this->termUnitSizing(this->curTermUnitSizingNum).AirVolFlow /
                                         this->termUnitSizing(this->curTermUnitSizingNum).InducRat;
                            Real64 const CpAir = Psychrometrics::PsyCpAirFnW(this->termUnitFinalZoneSizing(this->curTermUnitSizingNum).HeatDesHumRat);
                            // the design heating coil load is the zone load minus whatever the central system does.Note that
                            // DesHeatCoilInTempTU is really the primary air inlet temperature for the unit.
                            DesCoilLoad = this->termUnitFinalZoneSizing(this->curTermUnitSizingNum).DesHeatLoad -
                                          (CpAir * state.dataEnvrn->StdRhoAir * DesVolFlow *
                                           (this->termUnitFinalZoneSizing(this->curTermUnitSizingNum).DesHeatCoilInTempTU -
                                            this->termUnitFinalZoneSizing(this->curTermUnitSizingNum).ZoneTempAtHeatPeak));
                        } else {
                            DesCoilLoad = 0.0;
                        }
                    } else {
                        CoilOutTemp = this->finalZoneSizing(this->curZoneEqNum).HeatDesTemp;
                        CoilOutHumRat = this->finalZoneSizing(this->curZoneEqNum).HeatDesHumRat;
                        Real64 const CpAir = Psychrometrics::PsyCpAirFnW(CoilOutHumRat);
                        DesCoilLoad = CpAir * this->finalZoneSizing(this->curZoneEqNum).DesHeatMassFlow * (CoilOutTemp - CoilInTemp);
                        DesVolFlow = this->finalZoneSizing(this->curZoneEqNum).DesHeatMassFlow / state.dataEnvrn->StdRhoAir;
                    }
                    NominalCapacityDes = max(0.0, DesCoilLoad);
                } else {
                    NominalCapacityDes = 0.0;
                    CoilOutTemp = -999.0;
                }
                if (this->dataCoolCoilCap > 0.0) {
                    this->autoSizedValue = NominalCapacityDes * this->dataHeatSizeRatio;
                } else {
                    this->autoSizedValue = NominalCapacityDes * this->dataHeatSizeRatio * this->dataFracOfAutosizedHeatingCapacity;
                }
                if (state.dataGlobal->DisplayExtraWarnings && this->autoSizedValue <= 0.0) {
                    ShowWarningMessage(state,
                                       this->callingRoutine + ": Potential issue with equipment sizing for " + this->compType + ' ' + this->compName);
                    ShowContinueError(state, EnergyPlus::format("...Rated Total Heating Capacity = {:.2T} [W]", this->autoSizedValue));
                    if (this->zoneEqSizing(this->curZoneEqNum).HeatingCapacity ||
                        (this->dataCoolCoilCap > 0.0 && this->dataFlowUsedForSizing > 0.0)) {
                        ShowContinueError(
                            state,
                            EnergyPlus::format("...Capacity passed by parent object to size child component = {:.2T} [W]", NominalCapacityDes));
                    } else {
                        if (CoilOutTemp > -999.0) {
                            ShowContinueError(state, EnergyPlus::format("...Air flow rate used for sizing = {:.5T} [m3/s]", DesVolFlow));
                            ShowContinueError(state, EnergyPlus::format("...Coil inlet air temperature used for sizing = {:.2T} [C]", CoilInTemp));
                            ShowContinueError(state, EnergyPlus::format("...Coil outlet air temperature used for sizing = {:.2T} [C]", CoilOutTemp));
                        } else {
                            ShowContinueError(state, "...Capacity used to size child component set to 0 [W]");
                        }
                    }
                }
            }
        } else if (this->curSysNum > 0) {
            if (!this->wasAutoSized && !this->sizingDesRunThisAirSys) {
                this->autoSizedValue = _originalValue;
            } else {
                if (this->curOASysNum > 0) {
                    if (this->oaSysEqSizing(this->curOASysNum).AirFlow) {
                        DesVolFlow = this->oaSysEqSizing(this->curOASysNum).AirVolFlow;
                    } else if (this->oaSysEqSizing(this->curOASysNum).HeatingAirFlow) {
                        DesVolFlow = this->oaSysEqSizing(this->curOASysNum).HeatingAirVolFlow;
                    } else if (this->outsideAirSys(this->curOASysNum).AirLoopDOASNum > -1) {
                        DesVolFlow =
                            this->airloopDOAS[this->outsideAirSys(this->curOASysNum).AirLoopDOASNum].SizingMassFlow / state.dataEnvrn->StdRhoAir;
                    } else {
                        DesVolFlow = this->finalSysSizing(this->curSysNum).DesOutAirVolFlow;
                    }
                } else {
                    if (this->finalSysSizing(this->curSysNum).HeatingCapMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                        this->dataFracOfAutosizedHeatingCapacity = this->finalSysSizing(this->curSysNum).FractionOfAutosizedHeatingCapacity;
                    }
                    if (this->dataFlowUsedForSizing > 0.0) {
                        DesVolFlow = this->dataFlowUsedForSizing;
                    } else if (this->unitarySysEqSizing(this->curSysNum).AirFlow) {
                        DesVolFlow = this->unitarySysEqSizing(this->curSysNum).AirVolFlow;
                    } else if (this->unitarySysEqSizing(this->curSysNum).HeatingAirFlow) {
                        DesVolFlow = this->unitarySysEqSizing(this->curSysNum).HeatingAirVolFlow;
                    } else {
                        if (this->curDuctType == HVAC::AirDuctType::Main) {
                            if (this->finalSysSizing(this->curSysNum).SysAirMinFlowRat > 0.0 && !this->dataDesicRegCoil) {
                                DesVolFlow =
                                    this->finalSysSizing(this->curSysNum).SysAirMinFlowRat * this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                            } else {
                                DesVolFlow = this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                            }
                        } else if (this->curDuctType == HVAC::AirDuctType::Cooling) {
                            if (this->finalSysSizing(this->curSysNum).SysAirMinFlowRat > 0.0 && !this->dataDesicRegCoil) {
                                DesVolFlow =
                                    this->finalSysSizing(this->curSysNum).SysAirMinFlowRat * this->finalSysSizing(this->curSysNum).DesCoolVolFlow;
                            } else {
                                DesVolFlow = this->finalSysSizing(this->curSysNum).DesCoolVolFlow;
                            }
                        } else if (this->curDuctType == HVAC::AirDuctType::Heating) {
                            DesVolFlow = this->finalSysSizing(this->curSysNum).DesHeatVolFlow;
                        } else {
                            DesVolFlow = this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                        }
                    }
                }
                DesMassFlow = state.dataEnvrn->StdRhoAir * DesVolFlow;
                // get the outside air fraction
                if (this->curOASysNum > 0) {
                    OutAirFrac = 1.0;
                } else if (this->finalSysSizing(this->curSysNum).HeatOAOption == DataSizing::OAControl::MinOA) {
                    if (DesVolFlow > 0.0) {
                        OutAirFrac = this->finalSysSizing(this->curSysNum).DesOutAirVolFlow / DesVolFlow;
                    } else {
                        OutAirFrac = 1.0;
                    }
                    OutAirFrac = std::min(1.0, std::max(0.0, OutAirFrac));
                } else {
                    OutAirFrac = 1.0;
                }
                // coil inlet temperature
                if (this->curOASysNum == 0 && this->primaryAirSystem(this->curSysNum).NumOAHeatCoils > 0) {
                    CoilInTemp = OutAirFrac * this->finalSysSizing(this->curSysNum).PreheatTemp +
                                 (1.0 - OutAirFrac) * this->finalSysSizing(this->curSysNum).HeatRetTemp;
                    CoilInHumRat = OutAirFrac * this->finalSysSizing(this->curSysNum).PreheatHumRat +
                                   (1.0 - OutAirFrac) * this->finalSysSizing(this->curSysNum).HeatRetHumRat; // include humrat for coil sizing reports
                } else if (this->curOASysNum > 0 && this->outsideAirSys(this->curOASysNum).AirLoopDOASNum > -1) {
                    CoilInTemp = this->airloopDOAS[this->outsideAirSys(this->curOASysNum).AirLoopDOASNum].HeatOutTemp;
                } else {
                    CoilInTemp = OutAirFrac * this->finalSysSizing(this->curSysNum).HeatOutTemp +
                                 (1.0 - OutAirFrac) * this->finalSysSizing(this->curSysNum).HeatRetTemp;
                    CoilInHumRat = OutAirFrac * this->finalSysSizing(this->curSysNum).HeatOutHumRat +
                                   (1.0 - OutAirFrac) * this->finalSysSizing(this->curSysNum).HeatRetHumRat; // include humrat for coil sizing reports
                }
                // coil load
                if (this->curOASysNum > 0) {
                    if (this->oaSysEqSizing(this->curOASysNum).HeatingCapacity) {
                        DesCoilLoad = this->oaSysEqSizing(this->curOASysNum).DesHeatingLoad;
                    } else if (this->dataDesicRegCoil) {
                        DesCoilLoad = CpAirStd * DesMassFlow * (this->dataDesOutletAirTemp - this->dataDesInletAirTemp);
                        CoilOutTemp = this->dataDesOutletAirTemp;
                    } else if (this->outsideAirSys(this->curOASysNum).AirLoopDOASNum > -1) {
                        DesCoilLoad = CpAirStd * DesMassFlow *
                                      (this->airloopDOAS[this->outsideAirSys(this->curOASysNum).AirLoopDOASNum].PreheatTemp - CoilInTemp);
                        CoilOutTemp = this->airloopDOAS[outsideAirSys(this->curOASysNum).AirLoopDOASNum].PreheatTemp;
                    } else {
                        DesCoilLoad = CpAirStd * DesMassFlow * (this->finalSysSizing(this->curSysNum).PreheatTemp - CoilInTemp);
                        CoilOutTemp = this->finalSysSizing(this->curSysNum).PreheatTemp;
                        CoilOutHumRat = this->finalSysSizing(this->curSysNum).PreheatHumRat;
                    }
                } else {
                    if (this->unitarySysEqSizing(this->curSysNum).HeatingCapacity) {
                        DesCoilLoad = this->unitarySysEqSizing(this->curSysNum).DesHeatingLoad;
                        CoilOutTemp = this->finalSysSizing(this->curSysNum).HeatSupTemp;
                        CoilOutHumRat = this->finalSysSizing(this->curSysNum).HeatSupHumRat;
                    } else if (this->dataDesicRegCoil) {
                        DesCoilLoad = CpAirStd * DesMassFlow * (this->dataDesOutletAirTemp - this->dataDesInletAirTemp);
                        CoilOutTemp = this->dataDesOutletAirTemp;
                    } else {
                        DesCoilLoad = CpAirStd * DesMassFlow * (this->finalSysSizing(this->curSysNum).HeatSupTemp - CoilInTemp);
                        CoilOutTemp = this->finalSysSizing(this->curSysNum).HeatSupTemp;
                        CoilOutHumRat = this->finalSysSizing(this->curSysNum).HeatSupHumRat;
                    }
                }
                if (this->curSysNum <= state.dataHVACGlobal->NumPrimaryAirSys && this->airLoopControlInfo(this->curSysNum).UnitarySys) {
                    if (this->dataCoilIsSuppHeater) {
                        NominalCapacityDes = this->suppHeatCap;
                    } else if (this->dataCoolCoilCap > 0.0) {
                        NominalCapacityDes = this->dataCoolCoilCap;
                    } else {
                        // TRUE for all air loop parent equipment except UnitarySystem where flag is reset to FALSE after simulating
                        // This method allows downstream heating coils to size individually.Probably should do this for all air loop equipment
                        // ChangoverBypass model always sets AirLoopControlInfo%UnitarySys to FALSE so heating coil can individually size
                        if (this->airLoopControlInfo(this->curSysNum).UnitarySysSimulating &&
                            !Util::SameString(this->compType, "COIL:HEATING:WATER")) {
                            NominalCapacityDes = this->unitaryHeatCap;
                        } else {
                            if (DesCoilLoad >= HVAC::SmallLoad) {
                                NominalCapacityDes = DesCoilLoad;
                            } else {
                                NominalCapacityDes = 0.0;
                            }
                        }
                    }
                    DesCoilLoad = NominalCapacityDes;
                } else if (this->curSysNum <= state.dataHVACGlobal->NumPrimaryAirSys &&
                           this->finalSysSizing(this->curSysNum).HeatingCapMethod == DataSizing::CapacityPerFloorArea) {
                    NominalCapacityDes = this->finalSysSizing(this->curSysNum).HeatingTotalCapacity;
                } else if (this->curSysNum <= state.dataHVACGlobal->NumPrimaryAirSys &&
                           this->finalSysSizing(this->curSysNum).HeatingCapMethod == DataSizing::HeatingDesignCapacity &&
                           this->finalSysSizing(this->curSysNum).HeatingTotalCapacity > 0.0) {
                    NominalCapacityDes = this->finalSysSizing(this->curSysNum).HeatingTotalCapacity;
                } else {
                    if (this->dataCoolCoilCap > 0.0) { // this line can't get executed with same logic above else
                        NominalCapacityDes = this->dataCoolCoilCap;
                    } else if (DesCoilLoad >= HVAC::SmallLoad) {
                        NominalCapacityDes = DesCoilLoad;
                    } else {
                        NominalCapacityDes = 0.0;
                    }
                }
                this->autoSizedValue = NominalCapacityDes * this->dataHeatSizeRatio;
                // apply sizing factor once; see #10290
                if (this->curOASysNum > 0) {
                    if (!this->oaSysEqSizing(this->curOASysNum).HeatingCapacity) {
                        this->autoSizedValue = this->autoSizedValue * this->dataFracOfAutosizedHeatingCapacity;
                    }
                } else {
                    if (!this->unitarySysEqSizing(this->curSysNum).HeatingCapacity) {
                        this->autoSizedValue = this->autoSizedValue * this->dataFracOfAutosizedHeatingCapacity;
                    }
                }
                if (state.dataGlobal->DisplayExtraWarnings && this->autoSizedValue <= 0.0) {
                    ShowWarningMessage(state,
                                       this->callingRoutine + ": Potential issue with equipment sizing for " + this->compType + ' ' + this->compName);
                    ShowContinueError(state, EnergyPlus::format("...Rated Total Heating Capacity = {:.2T} [W]", this->autoSizedValue));
                    if (CoilOutTemp > -999.0) {
                        ShowContinueError(state, EnergyPlus::format("...Air flow rate used for sizing = {:.5T} [m3/s]", DesVolFlow));
                        ShowContinueError(state, EnergyPlus::format("...Outdoor air fraction used for sizing = {:.2T}", OutAirFrac));
                        ShowContinueError(state, EnergyPlus::format("...Coil inlet air temperature used for sizing = {:.2T} [C]", CoilInTemp));
                        ShowContinueError(state, EnergyPlus::format("...Coil outlet air temperature used for sizing = {:.2T} [C]", CoilOutTemp));
                    } else {
                        ShowContinueError(
                            state, EnergyPlus::format("...Capacity passed by parent object to size child component = {:.2T} [W]", DesCoilLoad));
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
    }
    if (!this->hardSizeNoDesignRun || this->dataScalableSizingON || this->dataScalableCapSizingON) {
        if (this->wasAutoSized && this->dataFractionUsedForSizing == 0.0) {
            // Note: the VolFlowPerRatedTotCap check is not applicable for VRF-FluidTCtrl coil model, which implements variable flow fans and
            // determines capacity using physical calculations instead of empirical curves
            bool FlagCheckVolFlowPerRatedTotCap = true;
            if (Util::SameString(this->compType, "Coil:Cooling:DX:VariableRefrigerantFlow:FluidTemperatureControl") ||
                Util::SameString(this->compType, "Coil:Heating:DX:VariableRefrigerantFlow:FluidTemperatureControl")) {
                FlagCheckVolFlowPerRatedTotCap = false;
            }

            if (this->dataIsDXCoil && FlagCheckVolFlowPerRatedTotCap && this->autoSizedValue > 0.0) {
                Real64 RatedVolFlowPerRatedTotCap = DesVolFlow / this->autoSizedValue;
                if (RatedVolFlowPerRatedTotCap < HVAC::MinRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT]) {
                    if (!this->dataEMSOverrideON && state.dataGlobal->DisplayExtraWarnings && this->printWarningFlag) {
                        ShowWarningError(state, this->callingRoutine + ' ' + this->compType + ' ' + this->compName);
                        ShowContinueError(
                            state, "..." + this->sizingString + " will be limited by the minimum rated volume flow per rated total capacity ratio.");
                        ShowContinueError(state, EnergyPlus::format("...DX coil volume flow rate [m3/s] = {:.6T}", DesVolFlow));
                        ShowContinueError(state, EnergyPlus::format("...Requested capacity [W] = {:.3T}", this->autoSizedValue));
                        ShowContinueError(state,
                                          EnergyPlus::format("...Requested flow/capacity ratio [m3/s/W] = {:.3T}", RatedVolFlowPerRatedTotCap));
                        ShowContinueError(state,
                                          EnergyPlus::format("...Minimum flow/capacity ratio [m3/s/W] = {:.3T}",
                                                             HVAC::MinRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT]));
                    }

                    DXFlowPerCapMinRatio = (DesVolFlow / HVAC::MinRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT]) /
                                           this->autoSizedValue; // set DX Coil Capacity Increase Ratio from Too Low Flow/Capacity Ratio
                    this->autoSizedValue = DesVolFlow / HVAC::MinRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT];

                    if (!this->dataEMSOverrideON && state.dataGlobal->DisplayExtraWarnings && this->printWarningFlag) {
                        ShowContinueError(state, EnergyPlus::format("...Adjusted capacity [W] = {:.3T}", this->autoSizedValue));
                    }
                } else if (RatedVolFlowPerRatedTotCap > HVAC::MaxRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT]) {
                    if (!this->dataEMSOverrideON && state.dataGlobal->DisplayExtraWarnings && this->printWarningFlag) {
                        ShowWarningError(state, this->callingRoutine + ' ' + this->compType + ' ' + this->compName);
                        ShowContinueError(
                            state, "..." + this->sizingString + " will be limited by the maximum rated volume flow per rated total capacity ratio.");
                        ShowContinueError(state, EnergyPlus::format("...DX coil volume flow rate [m3/s] = {:.6T}", DesVolFlow));
                        ShowContinueError(state, EnergyPlus::format("...Requested capacity [W] = {:.3T}", this->autoSizedValue));
                        ShowContinueError(state,
                                          EnergyPlus::format("...Requested flow/capacity ratio [m3/s/W] = {:.3T}", RatedVolFlowPerRatedTotCap));
                        ShowContinueError(state,
                                          EnergyPlus::format("...Maximum flow/capacity ratio [m3/s/W] = {:.3T}",
                                                             HVAC::MaxRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT]));
                    }

                    DXFlowPerCapMaxRatio = DesVolFlow / HVAC::MaxRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT] /
                                           this->autoSizedValue; // set DX Coil Capacity Decrease Ratio from Too High Flow/Capacity Ratio
                    this->autoSizedValue = DesVolFlow / HVAC::MaxRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT];

                    if (!this->dataEMSOverrideON && state.dataGlobal->DisplayExtraWarnings && this->printWarningFlag) {
                        ShowContinueError(state, EnergyPlus::format("...Adjusted capacity [W] = {:.3T}", this->autoSizedValue));
                    }
                }
            }
        }
    }

    // override sizing string
    if (this->overrideSizeString) {
        this->sizingString = "Heating Capacity [W]";
    }
    if (this->dataScalableCapSizingON) {
        switch (this->zoneEqSizing(this->curZoneEqNum).SizingMethod(HVAC::HeatingCapacitySizing)) {
        case DataSizing::CapacityPerFloorArea: {
            this->sizingStringScalable = "(scaled by capacity / area) ";
        } break;
        case DataSizing::FractionOfAutosizedHeatingCapacity:
        case DataSizing::FractionOfAutosizedCoolingCapacity: {
            this->sizingStringScalable = "(scaled by fractional multiplier) ";
        } break;
        default:
            break;
        }
    }

    this->selectSizerOutput(state, errorsFound);

    if (this->isCoilReportObject) {
        if (CoilInTemp > -999.0) { // set inlet air properties used during capacity sizing if available, allow for negative winter temps
            ReportCoilSelection::setCoilEntAirTemp(state, this->coilReportNum, CoilInTemp, this->curSysNum, this->curZoneEqNum);
            ReportCoilSelection::setCoilEntAirHumRat(state, this->coilReportNum, CoilInHumRat);
        }
        if (CoilOutTemp > -999.0) { // set outlet air properties used during capacity sizing if available
            ReportCoilSelection::setCoilLvgAirTemp(state, this->coilReportNum, CoilOutTemp);
            ReportCoilSelection::setCoilLvgAirHumRat(state, this->coilReportNum, CoilOutHumRat);
        }
        ReportCoilSelection::setCoilAirFlow(state, this->coilReportNum, DesVolFlow, this->wasAutoSized);
        Real64 constexpr FanCoolLoad = 0.0;
        Real64 constexpr TotCapTempModFac = 1.0;
        ReportCoilSelection::setCoilHeatingCapacity(state,
                                                    this->coilReportNum,
                                                    this->autoSizedValue,
                                                    this->wasAutoSized,
                                                    this->curSysNum,
                                                    this->curZoneEqNum,
                                                    this->curOASysNum,
                                                    FanCoolLoad,
                                                    TotCapTempModFac,
                                                    DXFlowPerCapMinRatio,
                                                    DXFlowPerCapMaxRatio);
    }
    return this->autoSizedValue;
}

void HeatingCapacitySizer::clearState()
{
    BaseSizerWithScalableInputs::clearState();
}

} // namespace EnergyPlus
