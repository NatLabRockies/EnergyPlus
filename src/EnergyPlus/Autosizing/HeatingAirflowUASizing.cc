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

#include <EnergyPlus/Autosizing/HeatingAirflowUASizing.hh>
#include <EnergyPlus/DataHVACGlobals.hh>

namespace EnergyPlus {

void HeatingAirflowUASizer::initializeForSingleDuctZoneTerminal(EnergyPlusData &state, Real64 const elevation, Real64 mainFlowRate)
{
    this->initializeFromAPI(state, elevation);
    this->zoneSizingRunDone = true;
    this->curZoneEqNum = 1;
    this->termUnitSingDuct = true;
    this->curTermUnitSizingNum = 1;
    this->termUnitSizing.allocate(1);
    this->termUnitSizing(1).AirVolFlow = mainFlowRate;
}

void HeatingAirflowUASizer::initializeForZoneInductionUnit(EnergyPlusData &state,
                                                           Real64 const elevation,
                                                           Real64 mainFlowRate,
                                                           Real64 reheatMultiplier)
{
    this->initializeFromAPI(state, elevation);
    this->zoneSizingRunDone = true;
    this->curZoneEqNum = 1;
    this->termUnitIU = true;
    this->termUnitPIU = true; // probably don't need to set both, but whatever
    this->curTermUnitSizingNum = 1;
    this->termUnitSizing.allocate(1);
    this->termUnitSizing(1).AirVolFlow = mainFlowRate;
    this->termUnitSizing(1).ReheatAirFlowMult = reheatMultiplier;
}

void HeatingAirflowUASizer::initializeForZoneFanCoil(EnergyPlusData &state, Real64 const elevation, Real64 designHeatVolumeFlowRate)
{
    this->initializeFromAPI(state, elevation);
    this->zoneSizingRunDone = true;
    this->zoneEqFanCoil = true;
    this->curZoneEqNum = 1;
    this->finalZoneSizing.allocate(1);
    this->finalZoneSizing(1).DesHeatVolFlow = designHeatVolumeFlowRate;
}

// TODO: What would the zone otherTypeEq include?

void HeatingAirflowUASizer::initializeForSystemOutdoorAir(EnergyPlusData &state, Real64 const elevation, Real64 overallSystemMassFlowRate, bool DOAS)
{
    this->initializeFromAPI(state, elevation);
    this->curSysNum = 1;
    this->curOASysNum = 1;
    if (DOAS) {
        this->outsideAirSys(1).AirLoopDOASNum = 0; // the DOAS structure is a zero-based vector, w00t!
        this->airloopDOAS.emplace_back();
        this->airloopDOAS[0].SizingMassFlow = overallSystemMassFlowRate;
    } else {
        this->finalSysSizing.allocate(1);
        this->finalSysSizing(1).DesOutAirVolFlow = 0.0; // TODO: what do I do here?
        this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesOutAirVolFlow;
    }
}

void HeatingAirflowUASizer::initializeForSystemMainDuct(EnergyPlusData &state,
                                                        Real64 const elevation,
                                                        Real64 overallSystemVolFlow,
                                                        Real64 minFlowRateRatio)
{
    this->initializeFromAPI(state, elevation);
    this->curSysNum = 1;
    this->curDuctType = HVAC::AirDuctType::Main;
    this->finalSysSizing.allocate(1);
    this->finalSysSizing(1).SysAirMinFlowRat = minFlowRateRatio;
    this->finalSysSizing(1).DesMainVolFlow = overallSystemVolFlow;
}

void HeatingAirflowUASizer::initializeForSystemCoolingDuct(EnergyPlusData &state, Real64 const elevation)
{
    this->initializeFromAPI(state, elevation);
    this->curSysNum = 1;
}

void HeatingAirflowUASizer::initializeForSystemHeatingDuct(EnergyPlusData &state, Real64 const elevation)
{
    this->initializeFromAPI(state, elevation);
    this->curSysNum = 1;
}

void HeatingAirflowUASizer::initializeForSystemOtherDuct(EnergyPlusData &state, Real64 const elevation)
{
    this->initializeFromAPI(state, elevation);
    this->curSysNum = 1;
}

Real64 HeatingAirflowUASizer::size(EnergyPlusData &state, Real64 _originalValue, bool &errorsFound)
{
    if (!this->checkInitialized(state, errorsFound)) {
        return 0.0;
    }
    this->preSize(state, _originalValue);
    if (this->curZoneEqNum > 0) {
        if (!this->wasAutoSized && !this->sizingDesRunThisZone) {
            this->autoSizedValue = _originalValue;
        } else {
            if (this->termUnitSingDuct && (this->curTermUnitSizingNum > 0)) {
                this->autoSizedValue = this->stdRhoAir * this->termUnitSizing(this->curTermUnitSizingNum).AirVolFlow;
            } else if ((this->termUnitPIU || this->termUnitIU) && (this->curTermUnitSizingNum > 0)) {
                this->autoSizedValue = this->stdRhoAir * this->termUnitSizing(this->curTermUnitSizingNum).AirVolFlow *
                                       this->termUnitSizing(this->curTermUnitSizingNum).ReheatAirFlowMult;
            } else if (this->zoneEqFanCoil) {
                this->autoSizedValue = this->stdRhoAir * this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow;
            } else if (this->otherEqType) {
                if (this->zoneEqSizing(this->curZoneEqNum).SystemAirFlow) {
                    this->autoSizedValue = this->zoneEqSizing(this->curZoneEqNum).AirVolFlow * this->stdRhoAir;
                } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow) {
                    this->autoSizedValue = this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow * this->stdRhoAir;
                } else {
                    this->autoSizedValue = this->finalZoneSizing(this->curZoneEqNum).DesHeatMassFlow;
                }
            } else {
                this->errorType = AutoSizingResultType::ErrorType1;
                errorsFound = true;
            }
        }
    } else if (this->curSysNum > 0) {
        if (!this->wasAutoSized && !this->sizingDesRunThisAirSys) {
            this->autoSizedValue = _originalValue;
        } else {
            if (this->curOASysNum > 0) {
                if (this->outsideAirSys(this->curOASysNum).AirLoopDOASNum > -1) {
                    this->autoSizedValue = this->airloopDOAS[this->outsideAirSys(this->curOASysNum).AirLoopDOASNum].SizingMassFlow / this->stdRhoAir;
                } else {
                    this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesOutAirVolFlow;
                }
            } else {
                if (this->curDuctType == HVAC::AirDuctType::Main) {
                    if (this->finalSysSizing(this->curSysNum).SysAirMinFlowRat > 0.0) {
                        this->autoSizedValue =
                            this->finalSysSizing(this->curSysNum).SysAirMinFlowRat * this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                    } else {
                        this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                    }
                } else if (this->curDuctType == HVAC::AirDuctType::Cooling) {
                    if (this->finalSysSizing(this->curSysNum).SysAirMinFlowRat > 0.0) {
                        this->autoSizedValue =
                            this->finalSysSizing(this->curSysNum).SysAirMinFlowRat * this->finalSysSizing(this->curSysNum).DesCoolVolFlow;
                    } else {
                        this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesCoolVolFlow;
                    }
                } else if (this->curDuctType == HVAC::AirDuctType::Heating) {
                    this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesHeatVolFlow;
                } else {
                    this->autoSizedValue = this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                }
            }
            this->autoSizedValue *= this->stdRhoAir;
        }
    }
    if (this->autoSizedValue < HVAC::SmallAirVolFlow) {
        this->addErrorMessage("Autosized value was zero or less than zero");
        this->autoSizedValue = 0.0;
    }
    if (this->overrideSizeString) {
        if (this->isEpJSON) {
            this->sizingString = "heating_coil_airflow_for_ua";
        }
    }
    this->selectSizerOutput(state, errorsFound);
    return this->autoSizedValue;
}

} // namespace EnergyPlus
