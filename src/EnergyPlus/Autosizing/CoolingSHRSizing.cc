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

#include <EnergyPlus/Autosizing/CoolingSHRSizing.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>

namespace EnergyPlus {

Real64 CoolingSHRSizer::size(EnergyPlusData &state, Real64 _originalValue, bool &errorsFound)
{
    Real64 constexpr RatedInletAirTemp(26.6667);     // 26.6667C or 80F
    Real64 constexpr RatedInletAirHumRat(0.0111847); // Humidity ratio corresponding to 80F dry bulb/67F wet bulb

    if (!this->checkInitialized(state, errorsFound)) {
        return 0.0;
    }
    this->preSize(state, _originalValue);

    if (this->dataFractionUsedForSizing > 0.0) {
        this->autoSizedValue = this->dataConstantUsedForSizing * this->dataFractionUsedForSizing;
    } else {
        if (this->dataEMSOverrideON) {
            this->autoSizedValue = this->dataEMSOverride;
        } else {
            if (!this->wasAutoSized &&
                ((this->curZoneEqNum > 0 && !this->sizingDesRunThisZone) || (this->curSysNum > 0 && !this->sizingDesRunThisAirSys))) {
                this->autoSizedValue = _originalValue;
            } else {
                if (this->dataFlowUsedForSizing >= HVAC::SmallAirVolFlow && this->dataCapacityUsedForSizing > 0.0) {
                    // For autosizing the rated SHR, we set a minimum SHR of 0.676 and a maximum of 0.798. The min SHR occurs occurs at the
                    // minimum flow / capacity ratio = MinRatedVolFlowPerRatedTotCap = 0.00004027 [m3/s / W] = 300 [cfm/ton].
                    // The max SHR occurs at maximum flow / capacity ratio = MaxRatedVolFlowPerRatedTotCap = 0.00006041 [m3/s / W] = 450
                    // [cfm/ton]. For flow / capacity ratios between the min and max we linearly interpolate between min and max SHR. Thus
                    // rated SHR is a linear function of the rated flow / capacity ratio. This linear function (see below) is the result of a
                    // regression of flow/capacity ratio vs SHR for several actual coils.
                    Real64 RatedVolFlowPerRatedTotCap = this->dataFlowUsedForSizing / this->dataCapacityUsedForSizing;
                    if (state.dataHVACGlobal->DXCT == HVAC::DXCoilType::Regular) {
                        if (RatedVolFlowPerRatedTotCap > HVAC::MaxRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT]) {
                            this->autoSizedValue = 0.431 + 6086.0 * HVAC::MaxRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT];
                        } else if (RatedVolFlowPerRatedTotCap < HVAC::MinRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT]) {
                            this->autoSizedValue = 0.431 + 6086.0 * HVAC::MinRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT];
                        } else {
                            this->autoSizedValue = 0.431 + 6086.0 * RatedVolFlowPerRatedTotCap;
                        }
                    } else { // DOASDXCoil, or DXCT = 2
                        if (RatedVolFlowPerRatedTotCap > HVAC::MaxRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT]) {
                            this->autoSizedValue = 0.389 + 7684.0 * HVAC::MaxRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT];
                        } else if (RatedVolFlowPerRatedTotCap < HVAC::MinRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT]) {
                            this->autoSizedValue = 0.389 + 7684.0 * HVAC::MinRatedVolFlowPerRatedTotCap[(int)state.dataHVACGlobal->DXCT];
                        } else {
                            this->autoSizedValue = 0.389 + 7684.0 * RatedVolFlowPerRatedTotCap;
                        }
                    }

                    // check that the autosized SHR corresponds to a valid apperatus dew point (ADP) temperature
                    this->autoSizedValue = DXCoils::ValidateADP(state,
                                                                this->compType,
                                                                this->compName,
                                                                RatedInletAirTemp,
                                                                RatedInletAirHumRat,
                                                                this->dataCapacityUsedForSizing,
                                                                this->dataFlowUsedForSizing,
                                                                this->autoSizedValue,
                                                                this->callingRoutine);
                    if (this->dataSizingFraction < 1.0) {
                        this->autoSizedValue *= this->dataSizingFraction;
                    }
                } else {
                    if (this->wasAutoSized) {
                        this->autoSizedValue = 1.0;
                        std::string msg = "Developer Error: For autosizing of " + this->compType + ' ' + this->compName +
                                          ", DataFlowUsedForSizing and DataCapacityUsedForSizing " + this->sizingString +
                                          " must both be greater than 0.";
                        this->errorType = AutoSizingResultType::ErrorType1;
                        this->addErrorMessage(msg);
                    }
                }
            }
        }
    }
    this->updateSizingString(state);
    this->selectSizerOutput(state, errorsFound);
    return this->autoSizedValue;
}

void CoolingSHRSizer::updateSizingString(EnergyPlusData &state)
{
    if (!overrideSizeString) {
        return;
    }
    // override sizingString to match existing text
    if (this->coilType_Num == HVAC::CoilDX_CoolingTwoSpeed) {
        if (this->dataDXSpeedNum == 1) { // mode 1 is high speed in DXCoils loop
            if (this->isEpJSON) {
                this->sizingString = "high_speed_rated_sensible_heat_ratio";
            } else {
                this->sizingString = "High Speed Rated Sensible Heat Ratio";
            }
        } else if (this->dataDXSpeedNum == 2) {
            if (this->isEpJSON) {
                this->sizingString = "low_speed_gross_rated_sensible_heat_ratio";
            } else {
                this->sizingString = "Low Speed Gross Rated Sensible Heat Ratio";
            }
        }
    } else if (this->coilType_Num == HVAC::CoilDX_MultiSpeedCooling) {
        if (this->isEpJSON) {
            this->sizingString = fmt::format("speed_{}_rated_sensible_heat_ratio", state.dataSize->DataDXSpeedNum);
        } else {
            this->sizingString = fmt::format("Speed {} Rated Sensible Heat Ratio", state.dataSize->DataDXSpeedNum);
        }
    } else if (this->coilType_Num == HVAC::CoilVRF_FluidTCtrl_Cooling) {
        if (this->isEpJSON) {
            this->sizingString = "rated_sensible_heat_ratio";
        } else {
            this->sizingString = "Rated Sensible Heat Ratio";
        }
    } else if (this->coilType_Num == HVAC::CoilDX_CurveFit_Speed) {
        if (this->isEpJSON) {
            this->sizingString = "gross_sensible_heat_ratio";
        } else {
            this->sizingString = "Gross Sensible Heat Ratio";
        }
    } else {
        if (this->isEpJSON) {
            this->sizingString = "gross_rated_sensible_heat_ratio";
        }
    }
}

} // namespace EnergyPlus
