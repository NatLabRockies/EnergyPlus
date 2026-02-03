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

#include <EnergyPlus/Autosizing/CoolingWaterflowSizing.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Psychrometrics.hh>

namespace EnergyPlus {

Real64 CoolingWaterflowSizer::size(EnergyPlusData &state, Real64 _originalValue, bool &errorsFound)
{
    if (!this->checkInitialized(state, errorsFound)) {
        return 0.0;
    }
    this->preSize(state, _originalValue);
    Real64 CoilDesWaterDeltaT = this->dataWaterCoilSizCoolDeltaT;
    // component used AutoCalculate method to size value
    // AutoCalculate is not used for cooling coil water flow sizing
    if (this->dataFractionUsedForSizing > 0.0) {
        this->autoSizedValue = this->dataConstantUsedForSizing * this->dataFractionUsedForSizing;
    } else {
        if (this->curZoneEqNum > 0) {
            if (!this->wasAutoSized && !this->sizingDesRunThisZone) {
                this->autoSizedValue = _originalValue;
            } else {
                if (this->termUnitIU && (this->curTermUnitSizingNum > 0)) {
                    this->autoSizedValue = this->termUnitSizing(this->curTermUnitSizingNum).MaxCWVolFlow;
                } else if (this->zoneEqFanCoil || this->zoneEqUnitVent || this->zoneEqVentedSlab) {
                    this->autoSizedValue = this->zoneEqSizing(this->curZoneEqNum).MaxCWVolFlow;
                } else {
                    Real64 CoilInTemp = this->finalZoneSizing(this->curZoneEqNum).DesCoolCoilInTemp;
                    Real64 CoilOutTemp = this->finalZoneSizing(this->curZoneEqNum).CoolDesTemp;
                    Real64 CoilOutHumRat = this->finalZoneSizing(this->curZoneEqNum).CoolDesHumRat;
                    Real64 CoilInHumRat = this->finalZoneSizing(this->curZoneEqNum).DesCoolCoilInHumRat;
                    Real64 DesCoilLoad =
                        this->finalZoneSizing(this->curZoneEqNum).DesCoolMassFlow *
                        (Psychrometrics::PsyHFnTdbW(CoilInTemp, CoilInHumRat) - Psychrometrics::PsyHFnTdbW(CoilOutTemp, CoilOutHumRat));
                    Real64 DesVolFlow = this->finalZoneSizing(this->curZoneEqNum).DesCoolMassFlow / state.dataEnvrn->StdRhoAir;
                    // add fan heat to coil load
                    DesCoilLoad += BaseSizerWithFanHeatInputs::calcFanDesHeatGain(DesVolFlow);
                    if (DesCoilLoad >= HVAC::SmallLoad) {
                        if (this->dataWaterLoopNum > 0 && this->dataWaterLoopNum <= (int)state.dataPlnt->PlantLoop.size() &&
                            this->dataWaterCoilSizCoolDeltaT > 0.0) {
                            Real64 Cp = state.dataPlnt->PlantLoop(this->dataWaterLoopNum)
                                            .glycol->getSpecificHeat(state, Constant::CWInitConvTemp, this->callingRoutine);
                            Real64 rho = state.dataPlnt->PlantLoop(this->dataWaterLoopNum)
                                             .glycol->getDensity(state, Constant::CWInitConvTemp, this->callingRoutine);
                            this->autoSizedValue = DesCoilLoad / (CoilDesWaterDeltaT * Cp * rho);
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
                    }
                }
            }
        } else if (this->curSysNum > 0) {
            if (!this->wasAutoSized && !this->sizingDesRunThisAirSys) {
                this->autoSizedValue = _originalValue;
            } else {
                if (this->curOASysNum > 0) {
                    CoilDesWaterDeltaT *= 0.5;
                }
                if (this->dataCapacityUsedForSizing >= HVAC::SmallLoad) {
                    if (this->dataWaterLoopNum > 0 && this->dataWaterLoopNum <= (int)state.dataPlnt->PlantLoop.size() && CoilDesWaterDeltaT > 0.0) {
                        Real64 Cp = state.dataPlnt->PlantLoop(this->dataWaterLoopNum)
                                        .glycol->getSpecificHeat(state, Constant::CWInitConvTemp, this->callingRoutine);
                        Real64 rho = state.dataPlnt->PlantLoop(this->dataWaterLoopNum)
                                         .glycol->getDensity(state, Constant::CWInitConvTemp, this->callingRoutine);
                        this->autoSizedValue = this->dataCapacityUsedForSizing / (CoilDesWaterDeltaT * Cp * rho);
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
    // override sizing string for detailed coil model
    if (this->overrideSizeString) {
        if (this->coilType_Num == HVAC::Coil_CoolingWaterDetailed) {
            if (this->isEpJSON) {
                this->sizingString = "maximum_water_flow_rate [m3/s]";
            } else {
                this->sizingString = "Maximum Water Flow Rate [m3/s]";
            }
        } else {
            if (this->isEpJSON) {
                this->sizingString = "design_water_flow_rate [m3/s]";
            }
        }
    }
    this->selectSizerOutput(state, errorsFound);
    if (this->isCoilReportObject) {
        state.dataRptCoilSelection->coilSelectionReportObj->setCoilWaterFlowPltSizNum(
            state, this->compName, this->compType, this->autoSizedValue, this->wasAutoSized, this->dataPltSizCoolNum, this->dataWaterLoopNum);
        state.dataRptCoilSelection->coilSelectionReportObj->setCoilWaterDeltaT(state, this->compName, this->compType, CoilDesWaterDeltaT);
        if (this->dataDesInletWaterTemp > 0.0) {
            state.dataRptCoilSelection->coilSelectionReportObj->setCoilEntWaterTemp(
                state, this->compName, this->compType, this->dataDesInletWaterTemp);
            state.dataRptCoilSelection->coilSelectionReportObj->setCoilLvgWaterTemp(
                state, this->compName, this->compType, this->dataDesInletWaterTemp + CoilDesWaterDeltaT);
        } else {
            state.dataRptCoilSelection->coilSelectionReportObj->setCoilEntWaterTemp(state, this->compName, this->compType, Constant::CWInitConvTemp);
            state.dataRptCoilSelection->coilSelectionReportObj->setCoilLvgWaterTemp(
                state, this->compName, this->compType, Constant::CWInitConvTemp + CoilDesWaterDeltaT);
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

void CoolingWaterflowSizer::clearState()
{
    BaseSizerWithFanHeatInputs::clearState();
}

} // namespace EnergyPlus
