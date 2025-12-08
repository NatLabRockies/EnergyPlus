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

#include <EnergyPlus/Autosizing/HeatingWaterflowSizing.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Psychrometrics.hh>

namespace EnergyPlus {

Real64 HeatingWaterflowSizer::size(EnergyPlusData &state, Real64 _originalValue, bool &errorsFound)
{
    if (!this->checkInitialized(state, errorsFound)) {
        return 0.0;
    }
    this->preSize(state, _originalValue);
    // component used AutoCalculate method to size value
    if (this->dataFractionUsedForSizing > 0.0) {
        this->autoSizedValue = this->dataConstantUsedForSizing * this->dataFractionUsedForSizing;
    } else {
        if (this->curZoneEqNum > 0) {
            if (!this->wasAutoSized && !this->sizingDesRunThisZone) {
                this->autoSizedValue = _originalValue;
            } else {
                if ((this->termUnitSingDuct || this->termUnitPIU || this->termUnitIU) && (this->curTermUnitSizingNum > 0)) {
                    this->autoSizedValue = this->termUnitSizing(this->curTermUnitSizingNum).MaxHWVolFlow;
                } else if (this->zoneEqFanCoil) {
                    this->autoSizedValue = this->zoneEqSizing(this->curZoneEqNum).MaxHWVolFlow;
                } else if (this->zoneEqUnitHeater || this->zoneEqVentedSlab) {
                    // for unit ventilator the cp value is calculated at 5.05(InitConvTemp) for the
                    // child and 60.0C for the unit ventilator //|| ZoneEqUnitVent
                    this->autoSizedValue = this->zoneEqSizing(this->curZoneEqNum).MaxHWVolFlow;
                } else {
                    Real64 DesMassFlow = this->finalZoneSizing(this->curZoneEqNum).DesHeatMassFlow;
                    if (this->zoneEqSizing(this->curZoneEqNum).SystemAirFlow) {
                        DesMassFlow = this->zoneEqSizing(this->curZoneEqNum).AirVolFlow * state.dataEnvrn->StdRhoAir;
                    } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow) {
                        DesMassFlow = this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow * state.dataEnvrn->StdRhoAir;
                    }
                    Real64 CoilInTemp = this->setHeatCoilInletTempForZoneEqSizing(
                        this->setOAFracForZoneEqSizing(state, DesMassFlow, this->zoneEqSizing(this->curZoneEqNum)),
                        this->zoneEqSizing(this->curZoneEqNum),
                        this->finalZoneSizing(this->curZoneEqNum));
                    Real64 CoilOutTemp = this->finalZoneSizing(this->curZoneEqNum).HeatDesTemp;
                    Real64 CoilOutHumRat = this->finalZoneSizing(this->curZoneEqNum).HeatDesHumRat;
                    Real64 DesCoilLoad = Psychrometrics::PsyCpAirFnW(CoilOutHumRat) * DesMassFlow * (CoilOutTemp - CoilInTemp);
                    if (DesCoilLoad >= HVAC::SmallLoad) {
                        if (this->dataWaterLoopNum > 0 && this->dataWaterLoopNum <= (int)state.dataPlnt->PlantLoop.size() &&
                            this->dataWaterCoilSizHeatDeltaT > 0.0) {
                            Real64 Cp = state.dataPlnt->PlantLoop(this->dataWaterLoopNum)
                                            .glycol->getSpecificHeat(state, Constant::HWInitConvTemp, this->callingRoutine);
                            Real64 rho = state.dataPlnt->PlantLoop(this->dataWaterLoopNum)
                                             .glycol->getDensity(state, Constant::HWInitConvTemp, this->callingRoutine);
                            this->autoSizedValue = DesCoilLoad / (this->dataWaterCoilSizHeatDeltaT * Cp * rho);
                        } else {
                            std::string msg = "Developer Error: For autosizing of " + this->compType + ' ' + this->compName +
                                              ", certain inputs are required. Add PlantLoop, Plant loop number and/or Water Coil water delta T.";
                            this->errorType = AutoSizingResultType::ErrorType1;
                            this->addErrorMessage(msg);
                            ShowSevereError(state, msg);
                        }
                    } else {
                        this->autoSizedValue = 0.0;
                    }
                }
            }
        } else if (this->curSysNum > 0) {
            if (!this->wasAutoSized && !this->sizingDesRunThisAirSys) {
                this->autoSizedValue = _originalValue;
            } else {
                if (this->dataCapacityUsedForSizing >= HVAC::SmallLoad) {
                    if (this->dataWaterLoopNum > 0 && this->dataWaterLoopNum <= (int)state.dataPlnt->PlantLoop.size() &&
                        this->dataWaterCoilSizHeatDeltaT > 0.0) {
                        Real64 Cp = state.dataPlnt->PlantLoop(this->dataWaterLoopNum)
                                        .glycol->getSpecificHeat(state, Constant::HWInitConvTemp, this->callingRoutine);
                        Real64 rho = state.dataPlnt->PlantLoop(this->dataWaterLoopNum)
                                         .glycol->getDensity(state, Constant::HWInitConvTemp, this->callingRoutine);
                        this->autoSizedValue = this->dataCapacityUsedForSizing / (this->dataWaterCoilSizHeatDeltaT * Cp * rho);
                    } else {
                        this->autoSizedValue = 0.0;
                        std::string msg =
                            "Developer Error: For autosizing of " + this->compType + ' ' + this->compName +
                            ", certain inputs are required. Add PlantLoop, Plant loop number, coil capacity and/or Water Coil water delta T.";
                        this->errorType = AutoSizingResultType::ErrorType1;
                        this->addErrorMessage(msg);
                        ShowSevereError(state, msg);
                    }
                } else {
                    this->autoSizedValue = 0.0;
                    // Warning about zero design coil load is issued elsewhere.
                }
            }
        }
    }
    if (this->overrideSizeString) {
        if (this->isEpJSON) {
            this->sizingString = "maximum_water_flow_rate [m3/s]";
        }
    }
    this->selectSizerOutput(state, errorsFound);
    if (this->isCoilReportObject) {
        state.dataRptCoilSelection->coilSelectionReportObj->setCoilWaterFlowPltSizNum(
            state, this->compName, this->compType, this->autoSizedValue, this->wasAutoSized, this->dataPltSizHeatNum, this->dataWaterLoopNum);
        state.dataRptCoilSelection->coilSelectionReportObj->setCoilEntWaterTemp(state, this->compName, this->compType, Constant::HWInitConvTemp);
        if (!this->plantSizData.empty() && this->dataPltSizHeatNum > 0) {
            state.dataRptCoilSelection->coilSelectionReportObj->setCoilWaterDeltaT(
                state, this->compName, this->compType, this->plantSizData(this->dataPltSizHeatNum).DeltaT);
            state.dataRptCoilSelection->coilSelectionReportObj->setCoilLvgWaterTemp(
                state, this->compName, this->compType, Constant::HWInitConvTemp - this->plantSizData(this->dataPltSizHeatNum).DeltaT);
        }
        this->calcCoilWaterFlowRates(state,
                                     this->compName,
                                     this->compType,
                                     this->autoSizedValue,
                                     this->dataWaterLoopNum,
                                     this->curZoneEqNum,
                                     this->curSysNum,
                                     this->curOASysNum,
                                     this->finalZoneSizing,
                                     this->finalSysSizing);
    }
    return this->autoSizedValue;
}

} // namespace EnergyPlus
