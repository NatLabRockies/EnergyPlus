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

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/CoolingWaterDesAirInletTempSizing.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/Psychrometrics.hh>

namespace EnergyPlus {

Real64 CoolingWaterDesAirInletTempSizer::size(EnergyPlusData &state, Real64 _originalValue, bool &errorsFound)
{
    if (!this->checkInitialized(state, errorsFound)) {
        return 0.0;
    }
    this->preSize(state, _originalValue);
    if (this->curZoneEqNum > 0) {
        if (!this->wasAutoSized && !this->sizingDesRunThisZone) {
            this->autoSizedValue = _originalValue;
        } else {
            if (this->termUnitIU) {
                this->autoSizedValue = this->finalZoneSizing(this->curZoneEqNum).ZoneTempAtCoolPeak;
            } else if (this->zoneEqFanCoil) {
                Real64 DesMassFlow = this->finalZoneSizing(this->curZoneEqNum).DesCoolMassFlow;
                this->autoSizedValue = this->setCoolCoilInletTempForZoneEqSizing(
                    this->setOAFracForZoneEqSizing(state, DesMassFlow, this->zoneEqSizing(this->curZoneEqNum)),
                    this->zoneEqSizing(this->curZoneEqNum),
                    this->finalZoneSizing(this->curZoneEqNum));
            } else {
                this->autoSizedValue = this->finalZoneSizing(this->curZoneEqNum).DesCoolCoilInTemp;
            }
            Real64 fanDeltaT = 0.0;
            if (this->dataFanPlacement == HVAC::FanPlace::BlowThru) {
                // calculate fan heat to get fan air-side delta T
                Real64 FanCoolLoad = this->calcFanDesHeatGain(this->dataAirFlowUsedForSizing);
                if (this->dataDesInletAirHumRat > 0.0 && this->dataAirFlowUsedForSizing > 0.0) {
                    Real64 CpAir = Psychrometrics::PsyCpAirFnW(this->dataDesInletAirHumRat);
                    fanDeltaT = FanCoolLoad / (CpAir * state.dataEnvrn->StdRhoAir * this->dataAirFlowUsedForSizing);
                }
            }
            this->autoSizedValue += fanDeltaT;
        }
    } else if (this->curSysNum > 0) {
        if (!this->wasAutoSized && !this->sizingDesRunThisAirSys) {
            this->autoSizedValue = _originalValue;
        } else {
            if (this->curOASysNum > 0) { // coil is in OA stream
                if (this->outsideAirSys(this->curOASysNum).AirLoopDOASNum > -1) {
                    this->autoSizedValue = this->airloopDOAS[this->outsideAirSys(this->curOASysNum).AirLoopDOASNum].SizingCoolOATemp;
                } else {
                    this->autoSizedValue = this->finalSysSizing(this->curSysNum).OutTempAtCoolPeak;
                }
            } else {                                                               // coil is in main air loop
                if (this->primaryAirSystem(this->curSysNum).NumOACoolCoils == 0) { // there is no precooling of the OA stream
                    this->autoSizedValue = this->finalSysSizing(this->curSysNum).MixTempAtCoolPeak;
                } else if (this->dataDesInletAirTemp > 0.0) {
                    this->autoSizedValue = this->dataDesInletAirTemp;
                } else { // there is precooling of the OA stream
                    Real64 OutAirFrac = 1.0;
                    if (this->dataFlowUsedForSizing > 0.0) {
                        OutAirFrac = this->finalSysSizing(this->curSysNum).DesOutAirVolFlow / this->dataFlowUsedForSizing;
                    }
                    OutAirFrac = min(1.0, max(0.0, OutAirFrac));
                    this->autoSizedValue = OutAirFrac * this->finalSysSizing(this->curSysNum).PrecoolTemp +
                                           (1.0 - OutAirFrac) * this->finalSysSizing(this->curSysNum).RetTempAtCoolPeak;
                }
                Real64 fanDeltaT = 0.0;
                if (this->primaryAirSystem(this->curSysNum).supFanPlace == HVAC::FanPlace::BlowThru) {
                    // calculate fan heat to get fan air-side delta T
                    Real64 FanCoolLoad = this->calcFanDesHeatGain(this->dataAirFlowUsedForSizing);
                    if (this->dataDesInletAirHumRat > 0.0 && this->dataAirFlowUsedForSizing > 0.0) {
                        Real64 CpAir = Psychrometrics::PsyCpAirFnW(this->dataDesInletAirHumRat);
                        fanDeltaT = FanCoolLoad / (CpAir * state.dataEnvrn->StdRhoAir * this->dataAirFlowUsedForSizing);
                        this->setDataDesAccountForFanHeat(state,
                                                          false); // used in CoolingCapacitySizing calculations to avoid double counting fan heat
                    }
                }
                this->autoSizedValue += fanDeltaT;
            }
        }
    }
    // override sizing string
    if (this->overrideSizeString) {
        this->sizingString = "Design Inlet Air Temperature [C]";
    }
    this->selectSizerOutput(state, errorsFound);
    if (this->isCoilReportObject) {
        if (this->curSysNum <= this->numPrimaryAirSys) {
            ReportCoilSelection::setCoilEntAirTemp(state, this->coilReportNum, this->autoSizedValue, this->curSysNum, this->curZoneEqNum);
        }
    }
    return this->autoSizedValue;
}

void CoolingWaterDesAirInletTempSizer::clearState()
{
    BaseSizerWithFanHeatInputs::clearState();
}

} // namespace EnergyPlus
