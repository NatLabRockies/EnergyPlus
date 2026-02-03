// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

#include <EnergyPlus/Autosizing/CoolingWaterDesAirOutletTempSizing.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/Psychrometrics.hh>

namespace EnergyPlus {

Real64 CoolingWaterDesAirOutletTempSizer::size(EnergyPlusData &state, Real64 _originalValue, bool &errorsFound)
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
                Real64 Cp =
                    state.dataPlnt->PlantLoop(this->dataWaterLoopNum).glycol->getSpecificHeat(state, Constant::CWInitConvTemp, this->callingRoutine);
                Real64 rho =
                    state.dataPlnt->PlantLoop(this->dataWaterLoopNum).glycol->getDensity(state, Constant::CWInitConvTemp, this->callingRoutine);
                Real64 DesCoilLoad = this->dataWaterFlowUsedForSizing * this->dataWaterCoilSizCoolDeltaT * Cp * rho;
                Real64 T1Out =
                    this->dataDesInletAirTemp - DesCoilLoad / (state.dataEnvrn->StdRhoAir * Psychrometrics::PsyCpAirFnW(this->dataDesInletAirHumRat) *
                                                               this->dataAirFlowUsedForSizing);
                Real64 T2Out = this->plantSizData(this->dataPltSizCoolNum).ExitTemp + 2.0;
                this->autoSizedValue = max(T1Out, T2Out);
            } else {
                this->autoSizedValue = this->finalZoneSizing(this->curZoneEqNum).CoolDesTemp;
            }
            Real64 fanDeltaT = 0.0;
            if (this->dataFanPlacement == HVAC::FanPlace::DrawThru) {
                // calculate fan heat to get fan air-side delta T
                Real64 FanCoolLoad = this->calcFanDesHeatGain(this->dataAirFlowUsedForSizing);
                if (this->dataDesInletAirHumRat > 0.0 && this->dataAirFlowUsedForSizing > 0.0) {
                    Real64 CpAir = Psychrometrics::PsyCpAirFnW(this->dataDesInletAirHumRat);
                    fanDeltaT = FanCoolLoad / (CpAir * state.dataEnvrn->StdRhoAir * this->dataAirFlowUsedForSizing);
                    this->setDataDesAccountForFanHeat(state, false); // used in CoolingCapacitySizing calculations to avoid double counting fan heat
                }
            }
            this->autoSizedValue -= fanDeltaT;

            if (this->autoSizedValue < this->dataDesInletWaterTemp && this->dataWaterFlowUsedForSizing > 0.0) { // flow here is water vol flow rate
                std::string msg = this->callingRoutine + ":" + " Coil=\"" + this->compName +
                                  "\", Cooling Coil has leaving air temperature < entering water temperature.";
                this->addErrorMessage(msg);
                ShowWarningError(state, msg);
                msg = format("    Tair,out  =  {:.3R}", this->autoSizedValue);
                this->addErrorMessage(msg);
                ShowContinueError(state, msg);
                msg = format("    Twater,in = {:.3R}", this->dataDesInletWaterTemp);
                this->addErrorMessage(msg);
                ShowContinueError(state, msg);
                this->autoSizedValue = this->dataDesInletWaterTemp + 0.5;
                msg = "....coil leaving air temperature will be reset to:";
                this->addErrorMessage(msg);
                ShowContinueError(state, msg);
                msg = format("    Tair,out = {:.3R}", this->autoSizedValue);
                this->addErrorMessage(msg);
                ShowContinueError(state, msg);
            }
        }
    } else if (this->curSysNum > 0) {
        if (!this->wasAutoSized && !this->sizingDesRunThisAirSys) {
            this->autoSizedValue = _originalValue;
        } else {
            if (this->curOASysNum > 0) {
                if (this->outsideAirSys(this->curOASysNum).AirLoopDOASNum > -1) {
                    this->autoSizedValue = this->airloopDOAS[this->outsideAirSys(this->curOASysNum).AirLoopDOASNum].PrecoolTemp;
                } else {
                    this->autoSizedValue = this->finalSysSizing(this->curSysNum).PrecoolTemp;
                }
            } else if (this->dataDesOutletAirTemp > 0.0) {
                this->autoSizedValue = this->dataDesOutletAirTemp;
                Real64 fanDeltaT = 0.0;
                if (this->primaryAirSystem(this->curSysNum).supFanPlace == HVAC::FanPlace::DrawThru) {
                    // calculate fan heat to get fan air-side delta T
                    Real64 FanCoolLoad = this->calcFanDesHeatGain(this->dataAirFlowUsedForSizing);
                    if (this->dataDesInletAirHumRat > 0.0 && this->dataAirFlowUsedForSizing > 0.0) {
                        Real64 CpAir = Psychrometrics::PsyCpAirFnW(this->dataDesInletAirHumRat);
                        fanDeltaT = FanCoolLoad / (CpAir * state.dataEnvrn->StdRhoAir * this->dataAirFlowUsedForSizing);
                        this->setDataDesAccountForFanHeat(state,
                                                          false); // used in CoolingCapacitySizing calculations to avoid double counting fan heat
                    }
                }
                this->autoSizedValue -= fanDeltaT;
            } else {
                this->autoSizedValue = this->finalSysSizing(this->curSysNum).CoolSupTemp;
                Real64 fanDeltaT = 0.0;
                if (this->primaryAirSystem(this->curSysNum).supFanPlace == HVAC::FanPlace::DrawThru) {
                    // calculate fan heat to get fan air-side delta T
                    Real64 FanCoolLoad = this->calcFanDesHeatGain(this->dataAirFlowUsedForSizing);
                    if (this->dataDesInletAirHumRat > 0.0 && this->dataAirFlowUsedForSizing > 0.0) {
                        Real64 CpAir = Psychrometrics::PsyCpAirFnW(this->dataDesInletAirHumRat);
                        fanDeltaT = FanCoolLoad / (CpAir * state.dataEnvrn->StdRhoAir * this->dataAirFlowUsedForSizing);
                        this->setDataDesAccountForFanHeat(state,
                                                          false); // used in CoolingCapacitySizing calculations to avoid double counting fan heat
                    }
                }
                this->autoSizedValue -= fanDeltaT;
            }
            if (this->autoSizedValue < this->dataDesInletWaterTemp && this->dataWaterFlowUsedForSizing > 0.0) {
                std::string msg = this->callingRoutine + ":" + " Coil=\"" + this->compName +
                                  "\", Cooling Coil has leaving air temperature < entering water temperature.";
                this->addErrorMessage(msg);
                ShowWarningError(state, msg);
                msg = format("    Tair,out  =  {:.3R}", this->autoSizedValue);
                ShowContinueError(state, msg);
                msg = format("    Twater,in = {:.3R}", this->dataDesInletWaterTemp);
                ShowContinueError(state, msg);
                this->autoSizedValue = this->dataDesInletWaterTemp + 0.5;
                msg = "....coil leaving air temperature will be reset to:";
                ShowContinueError(state, msg);
                msg = format("    Tair,out = {:.3R}", this->autoSizedValue);
                ShowContinueError(state, msg);
            }
        }
    }
    // override sizing string
    if (this->overrideSizeString) {
        if (this->isEpJSON) {
            this->sizingString = "design_outlet_air_temperature [C]";
        }
    }
    this->selectSizerOutput(state, errorsFound);
    if (this->isCoilReportObject) {
        state.dataRptCoilSelection->coilSelectionReportObj->setCoilLvgAirTemp(state, this->compName, this->compType, this->autoSizedValue);
    }
    return this->autoSizedValue;
}

void CoolingWaterDesAirOutletTempSizer::clearState()
{
    BaseSizerWithFanHeatInputs::clearState();
}

} // namespace EnergyPlus
