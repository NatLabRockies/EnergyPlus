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

#include <EnergyPlus/Autosizing/WaterHeatingCoilUASizing.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>

namespace EnergyPlus {

Real64 WaterHeatingCoilUASizer::size(EnergyPlusData &state, Real64 _originalValue, bool &errorsFound)
{
    if (!this->checkInitialized(state, errorsFound)) {
        return 0.0;
    }
    Real64 constexpr Acc(0.0001); // Accuracy of result
    int constexpr MaxIte(500);    // Maximum number of iterations
    int SolFla = 0;               // Flag of solver

    this->preSize(state, _originalValue);
    if (this->curZoneEqNum > 0) {
        if (!this->wasAutoSized && !this->sizingDesRunThisZone) {
            this->autoSizedValue = _originalValue;
        } else {
            if (this->dataCapacityUsedForSizing > 0.0 && this->dataWaterFlowUsedForSizing > 0.0 && this->dataFlowUsedForSizing > 0.0) {
                Real64 UA0 = 0.001 * this->dataCapacityUsedForSizing;
                Real64 UA1 = this->dataCapacityUsedForSizing;
                // Invert the simple heating coil model: given the design inlet conditions and the design load,
                // find the design UA.
                auto f = [&state, this](Real64 const UA) {
                    state.dataWaterCoils->WaterCoil(this->dataCoilNum).UACoilVariable = UA;
                    WaterCoils::CalcSimpleHeatingCoil(state, this->dataCoilNum, this->dataFanOp, 1.0, state.dataWaterCoils->SimCalc);
                    state.dataSize->DataDesignCoilCapacity = state.dataWaterCoils->WaterCoil(this->dataCoilNum).TotWaterHeatingCoilRate;
                    return (dataCapacityUsedForSizing - state.dataWaterCoils->WaterCoil(this->dataCoilNum).TotWaterHeatingCoilRate) /
                           dataCapacityUsedForSizing;
                };
                General::SolveRoot(state, Acc, MaxIte, SolFla, this->autoSizedValue, f, UA0, UA1);
                if (SolFla == -1) {
                    errorsFound = true;
                    std::string msg = "Autosizing of heating coil UA failed for Coil:Heating:Water \"" + this->compName + "\"";
                    this->addErrorMessage(msg);
                    ShowSevereError(state, msg);
                    msg = "  Iteration limit exceeded in calculating coil UA";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Lower UA estimate = {:.6T} W/m2-K (0.1% of Design Coil Load)", UA0);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Upper UA estimate = {:.6T} W/m2-K (100% of Design Coil Load)", UA1);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Final UA estimate when iterations exceeded limit = {:.6T} W/m2-K", this->autoSizedValue);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = "  Zone \"" + this->finalZoneSizing(this->curZoneEqNum).ZoneName +
                          "\" coil sizing conditions (may be different than Sizing inputs):";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Coil inlet air temperature     = {:.3T} C", this->dataDesInletAirTemp);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Coil inlet air humidity ratio  = {:.3T} kgWater/kgDryAir", this->dataDesInletAirHumRat);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Coil inlet air mass flow rate  = {:.6T} kg/s", this->dataFlowUsedForSizing);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    // TotWaterHeatingCoilRate is set in CALL to CalcSimpleHeatingCoil
                    msg = format("  Design Coil Capacity           = {:.3T} W", this->dataDesignCoilCapacity);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    if (this->dataNomCapInpMeth) {
                        msg = format("  Design Coil Load               = {:.3T} W", this->dataCapacityUsedForSizing);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = format("  Coil outlet air temperature    = {:.3T} C", this->dataDesOutletAirTemp);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = format("  Coil outlet air humidity ratio = {:.3T} kgWater/kgDryAir", this->dataDesOutletAirHumRat);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                    } else if (this->termUnitSingDuct || this->termUnitPIU || this->termUnitIU || this->zoneEqFanCoil) {
                        msg = format("  Design Coil Load               = {:.3T} W", this->dataCapacityUsedForSizing);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                    } else {
                        msg = format("  Design Coil Load               = {:.3T} W", this->dataCapacityUsedForSizing);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = format("  Coil outlet air temperature    = {:.3T} C", this->finalZoneSizing(this->curZoneEqNum).HeatDesTemp);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = format("  Coil outlet air humidity ratio = {:.3T} kgWater/kgDryAir",
                                     this->finalZoneSizing(this->curZoneEqNum).HeatDesHumRat);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                    }
                    this->dataErrorsFound = true;
                } else if (SolFla == -2) {
                    this->errorType = AutoSizingResultType::ErrorType1;
                    errorsFound = true;
                    std::string msg = "Autosizing of heating coil UA failed for Coil:Heating:Water \"" + this->compName + "\"";
                    this->addErrorMessage(msg);
                    ShowSevereError(state, msg);
                    msg = "  Bad starting values for UA";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Lower UA estimate = {:.6T} W/m2-K (0.1% of Design Coil Load)", UA0);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Upper UA estimate = {:.6T} W/m2-K (100% of Design Coil Load)", UA1);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = "  Zone \"" + this->finalZoneSizing(this->curZoneEqNum).ZoneName +
                          "\" coil sizing conditions (may be different than Sizing inputs):";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Coil inlet air temperature     = {:.3T} C", this->dataDesInletAirTemp);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Coil inlet air humidity ratio  = {:.3T} kgWater/kgDryAir", this->dataDesInletAirHumRat);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Coil inlet air mass flow rate  = {:.6T} kg/s", this->dataFlowUsedForSizing);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Design Coil Capacity           = {:.3T} W", this->dataDesignCoilCapacity);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    if (this->dataNomCapInpMeth) {
                        msg = format("  Design Coil Load               = {:.3T} W", this->dataCapacityUsedForSizing);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = format("  Coil outlet air temperature    = {:.3T} C", this->dataDesOutletAirTemp);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = format("  Coil outlet air humidity ratio = {:.3T} kgWater/kgDryAir", this->dataDesOutletAirHumRat);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                    } else if (this->termUnitSingDuct || this->termUnitPIU || this->termUnitIU || this->zoneEqFanCoil) {
                        msg = format("  Design Coil Load               = {:.3T} W", this->dataCapacityUsedForSizing);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                    } else {
                        msg = format("  Design Coil Load               = {:.3T} W", this->dataCapacityUsedForSizing);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = format("  Coil outlet air temperature    = {:.3T} C", this->finalZoneSizing(this->curZoneEqNum).HeatDesTemp);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = format("  Coil outlet air humidity ratio = {:.3T} kgWater/kgDryAir",
                                     this->finalZoneSizing(this->curZoneEqNum).HeatDesHumRat);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                    }
                    // TotWaterHeatingCoilRate is set in CALL to CalcSimpleHeatingCoil
                    if (this->dataDesignCoilCapacity < this->dataCapacityUsedForSizing) {
                        msg = "  Inadequate water side capacity: in Plant Sizing for this hot water loop";
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = "  increase design loop exit temperature and/or decrease design loop delta T";
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = "  Plant Sizing object = " + this->plantSizData(this->dataPltSizHeatNum).PlantLoopName;
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = format("  Plant design loop exit temperature = {:.3T} C", this->plantSizData(this->dataPltSizHeatNum).ExitTemp);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = format("  Plant design loop delta T          = {:.3T} C", this->dataWaterCoilSizHeatDeltaT);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                    }
                    this->dataErrorsFound = true;
                }
            } else {
                this->autoSizedValue = 1.0;
                if (this->dataWaterFlowUsedForSizing > 0.0 && this->dataCapacityUsedForSizing == 0.0) {
                    std::string msg = "The design coil load used for UA sizing is zero for Coil:Heating:Water " + this->compName;
                    this->addErrorMessage(msg);
                    ShowWarningError(state, msg);
                    msg = "An autosize value for UA cannot be calculated";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = "Input a value for UA, change the heating design day, or raise";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = "  the zone heating design supply air temperature";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = "Water coil UA is set to 1 and the simulation continues.";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                }
            }
        }
    } else if (this->curSysNum > 0) {
        if (!this->wasAutoSized && !this->sizingDesRunThisAirSys) {
            this->autoSizedValue = _originalValue;
        } else {
            if (this->dataCapacityUsedForSizing >= HVAC::SmallLoad && this->dataWaterFlowUsedForSizing > 0.0 && this->dataFlowUsedForSizing > 0.0) {
                Real64 UA0 = 0.001 * this->dataCapacityUsedForSizing;
                Real64 UA1 = this->dataCapacityUsedForSizing;
                // Invert the simple heating coil model: given the design inlet conditions and the design load,
                // find the design UA.
                auto f = [&state, this](Real64 const UA) {
                    state.dataWaterCoils->WaterCoil(this->dataCoilNum).UACoilVariable = UA;
                    WaterCoils::CalcSimpleHeatingCoil(state, this->dataCoilNum, this->dataFanOp, 1.0, state.dataWaterCoils->SimCalc);
                    state.dataSize->DataDesignCoilCapacity = state.dataWaterCoils->WaterCoil(this->dataCoilNum).TotWaterHeatingCoilRate;
                    return (dataCapacityUsedForSizing - state.dataWaterCoils->WaterCoil(this->dataCoilNum).TotWaterHeatingCoilRate) /
                           dataCapacityUsedForSizing;
                };
                General::SolveRoot(state, Acc, MaxIte, SolFla, this->autoSizedValue, f, UA0, UA1);
                if (SolFla == -1) {
                    errorsFound = true;
                    std::string msg = "Autosizing of heating coil UA failed for Coil:Heating:Water \"" + this->compName + "\"";
                    this->addErrorMessage(msg);
                    ShowSevereError(state, msg);
                    msg = "  Iteration limit exceeded in calculating coil UA";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Lower UA estimate = {:.6T} W/m2-K (1% of Design Coil Load)", UA0);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Upper UA estimate = {:.6T} W/m2-K (100% of Design Coil Load)", UA1);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Final UA estimate when iterations exceeded limit = {:.6T} W/m2-K", this->autoSizedValue);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = "  AirloopHVAC \"" + this->finalSysSizing(this->curSysNum).AirPriLoopName +
                          "\" coil sizing conditions (may be different than Sizing inputs):";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Coil inlet air temperature     = {:.3T} C", this->dataDesInletAirTemp);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Coil inlet air humidity ratio  = {:.3T} kgWater/kgDryAir", this->dataDesInletAirHumRat);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Coil inlet air mass flow rate  = {:.6T} kg/s", this->dataFlowUsedForSizing);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Design Coil Capacity           = {:.3T} W", this->dataDesignCoilCapacity);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Design Coil Load               = {:.3T} W", this->dataCapacityUsedForSizing);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    if (this->dataNomCapInpMeth) {
                        msg = format("  Coil outlet air temperature    = {:.3T} C", this->dataDesOutletAirTemp);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = format("  Coil outlet air humidity ratio = {:.3T} kgWater/kgDryAir", this->dataDesOutletAirHumRat);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                    }
                    this->dataErrorsFound = true;
                } else if (SolFla == -2) {
                    this->errorType = AutoSizingResultType::ErrorType1;
                    errorsFound = true;
                    std::string msg = "Autosizing of heating coil UA failed for Coil:Heating:Water \"" + this->compName + "\"";
                    this->addErrorMessage(msg);
                    ShowSevereError(state, msg);
                    msg = "  Bad starting values for UA";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Lower UA estimate = {:.6T} W/m2-K (1% of Design Coil Load)", UA0);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Upper UA estimate = {:.6T} W/m2-K (100% of Design Coil Load)", UA1);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = "  AirloopHVAC \"" + this->finalSysSizing(this->curSysNum).AirPriLoopName +
                          "\" coil sizing conditions (may be different than Sizing inputs):";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Coil inlet air temperature     = {:.3T} C", this->dataDesInletAirTemp);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Coil inlet air humidity ratio  = {:.3T} kgWater/kgDryAir", this->dataDesInletAirHumRat);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Coil inlet air mass flow rate  = {:.6T} kg/s", this->dataFlowUsedForSizing);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Design Coil Capacity           = {:.3T} W", this->dataDesignCoilCapacity);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("  Design Coil Load               = {:.3T} W", this->dataCapacityUsedForSizing);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    if (this->dataNomCapInpMeth) {
                        msg = format("  Coil outlet air temperature    = {:.3T} C", this->dataDesOutletAirTemp);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = format("  Coil outlet air humidity ratio = {:.3T} kgWater/kgDryAir", this->dataDesOutletAirHumRat);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                    }
                    if (this->dataDesignCoilCapacity < this->dataCapacityUsedForSizing && !this->dataNomCapInpMeth) {
                        msg = "  Inadequate water side capacity: in Plant Sizing for this hot water loop";
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = "  increase design loop exit temperature and/or decrease design loop delta T";
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = "  Plant Sizing object = " + this->plantSizData(this->dataPltSizHeatNum).PlantLoopName;
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = format("  Plant design loop exit temperature = {:.3T} C", this->plantSizData(this->dataPltSizHeatNum).ExitTemp);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                        msg = format("  Plant design loop delta T          = {:.3T} C", this->dataWaterCoilSizHeatDeltaT);
                        this->addErrorMessage(msg);
                        ShowContinueError(state, msg);
                    }
                    this->dataErrorsFound = true;
                }
            } else {
                this->autoSizedValue = 1.0;
                if (this->dataWaterFlowUsedForSizing > 0.0 && this->dataCapacityUsedForSizing < HVAC::SmallLoad) {
                    std::string msg = "The design coil load used for UA sizing is zero for Coil:Heating:Water " + this->compName;
                    this->addErrorMessage(msg);
                    ShowWarningError(state, msg);
                    msg = "An autosize value for UA cannot be calculated";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = "Input a value for UA, change the heating design day, or raise";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = "  the zone heating design supply air temperature";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = "Water coil UA is set to 1 and the simulation continues.";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                }
            }
        }
    }
    if (this->dataErrorsFound) {
        state.dataSize->DataErrorsFound = true;
    }
    if (this->overrideSizeString) {
        if (this->isEpJSON) {
            this->sizingString = "u-factor_times_area_value [W/K]";
        }
    }
    this->selectSizerOutput(state, errorsFound);
    if (this->isCoilReportObject && this->curSysNum <= state.dataHVACGlobal->NumPrimaryAirSys) {
        state.dataRptCoilSelection->coilSelectionReportObj->setCoilUA(state,
                                                                      this->compName,
                                                                      this->compType,
                                                                      this->autoSizedValue,
                                                                      this->dataCapacityUsedForSizing,
                                                                      this->wasAutoSized,
                                                                      this->curSysNum,
                                                                      this->curZoneEqNum);
    }
    return this->autoSizedValue;
}

} // namespace EnergyPlus
