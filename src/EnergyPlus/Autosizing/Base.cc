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

#include <EnergyPlus/Autosizing/All_Simple_Sizing.hh>
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/SQLiteProcedures.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

void BaseSizer::initializeWithinEP(EnergyPlusData &state,
                                   std::string_view const _compType,
                                   std::string_view const _compName,
                                   bool _printWarningFlag,
                                   std::string_view const _callingRoutine)
{
    this->initialized = true;
    this->compType = _compType;
    this->compName = _compName;
    this->isEpJSON = state.dataGlobal->isEpJSON;
    this->printWarningFlag = _printWarningFlag;
    this->callingRoutine = _callingRoutine;
    this->stdRhoAir = state.dataEnvrn->StdRhoAir;
    this->sysSizingRunDone = state.dataSize->SysSizingRunDone;
    this->zoneSizingRunDone = state.dataSize->ZoneSizingRunDone;
    this->curSysNum = state.dataSize->CurSysNum;
    this->curOASysNum = state.dataSize->CurOASysNum;
    this->curZoneEqNum = state.dataSize->CurZoneEqNum;
    this->curDuctType = state.dataSize->CurDuctType;
    this->numPrimaryAirSys = state.dataHVACGlobal->NumPrimaryAirSys;
    this->numSysSizInput = state.dataSize->NumSysSizInput;
    this->doSystemSizing = state.dataGlobal->DoSystemSizing;
    this->numZoneSizingInput = state.dataSize->NumZoneSizingInput;
    this->doZoneSizing = state.dataGlobal->DoZoneSizing;
    this->curTermUnitSizingNum = state.dataSize->CurTermUnitSizingNum;
    this->termUnitSingDuct = state.dataSize->TermUnitSingDuct;
    this->termUnitPIU = state.dataSize->TermUnitPIU;
    this->termUnitIU = state.dataSize->TermUnitIU;
    this->zoneEqFanCoil = state.dataSize->ZoneEqFanCoil;
    this->otherEqType = !(this->termUnitSingDuct || this->termUnitPIU || this->termUnitIU || this->zoneEqFanCoil);
    this->zoneEqUnitHeater = state.dataSize->ZoneEqUnitHeater;
    this->zoneEqUnitVent = state.dataSize->ZoneEqUnitVent;
    this->zoneEqVentedSlab = state.dataSize->ZoneEqVentedSlab;
    this->zoneSizingInput = state.dataSize->ZoneSizingInput;
    this->unitarySysEqSizing = state.dataSize->UnitarySysEqSizing;
    this->oaSysEqSizing = state.dataSize->OASysEqSizing;
    this->outsideAirSys = state.dataAirLoop->OutsideAirSys;
    this->termUnitSizing = state.dataSize->TermUnitSizing;
    this->finalZoneSizing = state.dataSize->FinalZoneSizing;
    this->termUnitFinalZoneSizing = state.dataSize->TermUnitFinalZoneSizing;
    this->zoneEqSizing = state.dataSize->ZoneEqSizing;
    this->sysSizingInputData = state.dataSize->SysSizInput;
    this->finalSysSizing = state.dataSize->FinalSysSizing;
    this->plantSizData = state.dataSize->PlantSizData;
    this->primaryAirSystem = state.dataAirSystemsData->PrimaryAirSystems;
    this->airLoopControlInfo = state.dataAirLoop->AirLoopControlInfo;
    this->airloopDOAS = state.dataAirLoopHVACDOAS->airloopDOAS;
    if (EnergyPlus::BaseSizer::isValidCoilType(this->compType)) { // coil reports fail if compType is not one of HVAC::cAllCoilTypes
        this->isCoilReportObject = true;
    }
    if (EnergyPlus::BaseSizer::isValidFanType(this->compType)) { // fan reports fail if compType is not a valid fan type
        this->isFanReportObject = true;
    }

    // global sizing data
    dataEMSOverrideON = state.dataSize->DataEMSOverrideON;
    dataEMSOverride = state.dataSize->DataEMSOverride;
    this->dataAutosizable = state.dataSize->DataAutosizable;
    this->minOA = DataSizing::OAControl::MinOA;
    this->dataConstantUsedForSizing = state.dataSize->DataConstantUsedForSizing;
    this->dataFractionUsedForSizing = state.dataSize->DataFractionUsedForSizing;
    state.dataSize->DataConstantUsedForSizing = 0.0; // reset here instead of in component model?
    state.dataSize->DataFractionUsedForSizing = 0.0;

    this->dataFanIndex = state.dataSize->DataFanIndex;
    this->dataFanType = state.dataSize->DataFanType;

    // global Data* sizing constants
    this->dataPltSizHeatNum = state.dataSize->DataPltSizHeatNum;
    this->dataWaterLoopNum = state.dataSize->DataWaterLoopNum;
    this->dataPltSizCoolNum = state.dataSize->DataPltSizCoolNum;
    this->dataWaterCoilSizHeatDeltaT = state.dataSize->DataWaterCoilSizHeatDeltaT;
    this->dataWaterCoilSizCoolDeltaT = state.dataSize->DataWaterCoilSizCoolDeltaT;
    this->dataCapacityUsedForSizing = state.dataSize->DataCapacityUsedForSizing;
    this->dataHeatSizeRatio = state.dataSize->DataHeatSizeRatio;

    this->dataAirFlowUsedForSizing = state.dataSize->DataAirFlowUsedForSizing;
    this->dataDesInletAirTemp = state.dataSize->DataDesInletAirTemp;
    this->dataDesAccountForFanHeat = state.dataSize->DataDesAccountForFanHeat;
    this->dataFanPlacement = state.dataSize->DataFanPlacement;
    this->dataDesInletAirHumRat = state.dataSize->DataDesInletAirHumRat;
    this->dataDesOutletAirHumRat = state.dataSize->DataDesOutletAirHumRat;
    this->dataDesOutletAirTemp = state.dataSize->DataDesOutletAirTemp;
    this->dataDesInletWaterTemp = state.dataSize->DataDesInletWaterTemp;
    this->dataFlowUsedForSizing = state.dataSize->DataFlowUsedForSizing;
    this->dataWaterFlowUsedForSizing = state.dataSize->DataWaterFlowUsedForSizing;

    this->dataSizingFraction = state.dataSize->DataSizingFraction;
    this->dataDXSpeedNum = state.dataSize->DataDXSpeedNum;
    this->dataDesicRegCoil = state.dataSize->DataDesicRegCoil;
    this->dataZoneUsedForSizing = state.dataSize->DataZoneUsedForSizing;
    this->dataDesicDehumNum = state.dataSize->DataDesicDehumNum;

    this->dataNomCapInpMeth = state.dataSize->DataNomCapInpMeth;
    this->dataCoilNum = state.dataSize->DataCoilNum;
    this->dataFanOp = state.dataSize->DataFanOp;
    this->dataDesignCoilCapacity = state.dataSize->DataDesignCoilCapacity;
    this->dataErrorsFound = state.dataSize->DataErrorsFound;
    this->dataBypassFrac = state.dataSize->DataBypassFrac;
    this->dataIsDXCoil = state.dataSize->DataIsDXCoil;
    this->dataNonZoneNonAirloopValue = state.dataSize->DataNonZoneNonAirloopValue;
    this->dataDXCoolsLowSpeedsAutozize = state.dataSize->DataDXCoolsLowSpeedsAutozize;
}

void BaseSizer::initializeFromAPI(EnergyPlusData &state, Real64 const elevation)
{
    this->clearState();
    this->initialized = true;
    this->compType = "API_component_type";
    this->compName = "API_component_name";
    this->printWarningFlag = false;
    this->callingRoutine = "called_from_API";
    Real64 barometricPressure = DataEnvironment::StdPressureSeaLevel * std::pow(1.0 - 2.25577e-05 * elevation, 5.2559);
    this->stdRhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(state, barometricPressure, 20.0, 0.0);
    this->isCoilReportObject = false;
}

void BaseSizer::addErrorMessage(std::string const &s)
{
    this->lastErrorMessages.append(s).append("\n");
}

std::string BaseSizer::getLastErrorMessages()
{
    std::string s(this->lastErrorMessages);
    this->lastErrorMessages = "";
    return s;
}

void BaseSizer::preSize(EnergyPlusData &state, Real64 const _originalValue)
{
    if (this->sizingType == AutoSizingType::Invalid) {
        std::string msg = "Sizing Library Base Class: preSize, SizingType not defined.";
        this->addErrorMessage(msg);
        ShowSevereError(state, msg);
        ShowFatalError(state, "Sizing type causes fatal error.");
    }
    this->originalValue = _originalValue;
    this->autoCalculate = false;
    this->errorType = EnergyPlus::AutoSizingResultType::NoError;
    this->initialized = false; // force use of Init then Size in subsequent calls
    this->hardSizeNoDesignRun = !(this->sysSizingRunDone || this->zoneSizingRunDone);
    this->sizingDesRunThisZone = false;
    this->sizingDesRunThisAirSys = false;

    if (this->dataFractionUsedForSizing == 0.0 && this->dataConstantUsedForSizing > 0.0) {
        this->errorType = AutoSizingResultType::ErrorType1;
        this->autoCalculate = true;
        this->hardSizeNoDesignRun = false;
        if (this->wasAutoSized) {
            this->autoSizedValue = 0.0;
            std::string msg = "Sizing Library: DataConstantUsedForSizing and DataFractionUsedForSizing used for autocalculating " +
                              this->sizingString + " must both be greater than 0.";
            this->addErrorMessage(msg);
            ShowSevereError(state, msg);
        }
    } else if (this->dataFractionUsedForSizing > 0.0) {
        this->autoCalculate = true;
        this->hardSizeNoDesignRun = false;
    } else if (this->sizingType == AutoSizingType::AutoCalculateSizing) {
        this->autoCalculate = true;
        if (this->originalValue == DataSizing::AutoSize && !this->dataEMSOverrideON) {
            this->errorType = AutoSizingResultType::ErrorType1;
            std::string msg = "Sizing Library: DataConstantUsedForSizing and DataFractionUsedForSizing used for autocalculating " +
                              this->sizingString + " must both be greater than 0.";
            this->addErrorMessage(msg);
            ShowSevereError(state, msg);
        }
    }

    if (this->curSysNum > 0 && this->curSysNum <= this->numPrimaryAirSys) {
        if (this->sysSizingRunDone) {
            int sysNum = this->curSysNum;
            this->sizingDesRunThisAirSys = std::any_of(
                this->sysSizingInputData.begin(), this->sysSizingInputData.end(), [sysNum](auto const &ssid) { return ssid.AirLoopNum == sysNum; });
        }
        if (allocated(this->unitarySysEqSizing)) {
            this->airLoopSysFlag =
                this->unitarySysEqSizing(this->curSysNum).CoolingCapacity || this->unitarySysEqSizing(this->curSysNum).HeatingCapacity;
        }
        if (this->curOASysNum > 0) {
            this->oaSysFlag = this->oaSysEqSizing(this->curOASysNum).CoolingCapacity || this->oaSysEqSizing(this->curOASysNum).HeatingCapacity;
        }
    }

    if (this->curZoneEqNum > 0) {
        if (allocated(this->zoneEqSizing)) {
            this->sizingDesValueFromParent = this->zoneEqSizing(this->curZoneEqNum).DesignSizeFromParent;
        }
        if (this->zoneSizingRunDone) {
            int zoneNum = this->curZoneEqNum;
            this->sizingDesRunThisZone = std::any_of(
                this->zoneSizingInput.begin(), this->zoneSizingInput.end(), [zoneNum](auto const &zsi) { return zsi.ZoneNum == zoneNum; });
        }
        this->hardSizeNoDesignRun = false;
    }

    if (this->originalValue == DataSizing::AutoSize) {
        this->wasAutoSized = true;
        this->hardSizeNoDesignRun = false;
        if (!this->sizingDesRunThisAirSys && this->curSysNum > 0 && !this->autoCalculate) {
            if (!this->sysSizingRunDone) {
                std::string msg = "For autosizing of " + this->compType + ' ' + this->compName + ", a system sizing run must be done.";
                this->addErrorMessage(msg);
                ShowSevereError(state, msg);
                if (this->numSysSizInput == 0) {
                    std::string msg2 = "No \"Sizing:System\" objects were entered.";
                    this->addErrorMessage(msg2);
                    ShowContinueError(state, msg2);
                }
                if (!this->doSystemSizing) {
                    std::string msg2 = R"(The "SimulationControl" object did not have the field "Do System Sizing Calculation" set to Yes.)";
                    this->addErrorMessage(msg2);
                    ShowContinueError(state, msg2);
                }
                ShowFatalError(state, "Program terminates due to previously shown condition(s).");
            }
        }
        if (!this->sizingDesRunThisZone && this->curZoneEqNum > 0 && !this->sizingDesValueFromParent && !this->autoCalculate) {
            if (!this->zoneSizingRunDone) {
                std::string msg = "For autosizing of " + this->compType + ' ' + this->compName + ", a zone sizing run must be done.";
                this->addErrorMessage(msg);
                ShowSevereError(state, msg);
                if (this->numZoneSizingInput == 0) {
                    std::string msg2 = "No \"Sizing:Zone\" objects were entered.";
                    this->addErrorMessage(msg2);
                    ShowContinueError(state, msg2);
                }
                if (!this->doZoneSizing) {
                    std::string msg2 = R"(The "SimulationControl" object did not have the field "Do Zone Sizing Calculation" set to Yes.)";
                    this->addErrorMessage(msg2);
                    ShowContinueError(state, msg2);
                }
                ShowFatalError(state, "Program terminates due to previously shown condition(s).");
            }
        }
    } else {
        this->wasAutoSized = false;
    }
}

void BaseSizer::reportSizerOutput(EnergyPlusData &state,
                                  std::string_view CompType,
                                  std::string_view CompName,
                                  std::string_view VarDesc,
                                  Real64 const VarValue,
                                  ObjexxFCL::Optional_string_const UsrDesc,
                                  ObjexxFCL::Optional<Real64 const> UsrValue)
{

    static constexpr std::string_view Format_990(
        "! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n");
    static constexpr std::string_view Format_991(" Component Sizing Information, {}, {}, {}, {:.5R}\n");

    // to do, make this a parameter. Unfortunately this function is used in MANY
    // places so it involves touching most of E+
    if (state.dataEnvrn->oneTimeCompRptHeaderFlag) {
        print(state.files.eio, Format_990);
        state.dataEnvrn->oneTimeCompRptHeaderFlag = false;
    }

    print(state.files.eio, Format_991, CompType, CompName, VarDesc, VarValue);
    // add to tabular output reports
    OutputReportPredefined::AddCompSizeTableEntry(state, CompType, CompName, VarDesc, VarValue);

    if (present(UsrDesc) && present(UsrValue)) {
        print(state.files.eio, Format_991, CompType, CompName, UsrDesc(), UsrValue());
        OutputReportPredefined::AddCompSizeTableEntry(state, CompType, CompName, UsrDesc(), UsrValue);
    } else if (present(UsrDesc) || present(UsrValue)) {
        ShowFatalError(state, "ReportSizingOutput: (Developer Error) - called with user-specified description or value but not both.");
    }

    // add to SQL output
    if (state.dataSQLiteProcedures->sqlite) {
        state.dataSQLiteProcedures->sqlite->addSQLiteComponentSizingRecord(CompType, CompName, VarDesc, VarValue);
    }
    if (present(UsrDesc) && present(UsrValue)) {
        if (state.dataSQLiteProcedures->sqlite) {
            state.dataSQLiteProcedures->sqlite->addSQLiteComponentSizingRecord(CompType, CompName, UsrDesc(), UsrValue);
        }
    }
}

void BaseSizer::reportSizerStrOutput(
    EnergyPlusData &state, std::string_view CompType, std::string_view CompName, std::string_view VarDesc, std::string_view VarValue)
{

    static constexpr std::string_view Format_990(
        "! <Component Sizing Information>, Component Type, Component Name, Input Field Description, Value\n");
    static constexpr std::string_view Format_991(" Component Sizing Information, {}, {}, {}, {}\n");

    // to do, make this a parameter. Unfortunately this function is used in MANY
    // places so it involves touching most of E+
    if (state.dataEnvrn->oneTimeCompRptHeaderFlag) {
        print(state.files.eio, Format_990);
        state.dataEnvrn->oneTimeCompRptHeaderFlag = false;
    }

    print(state.files.eio, Format_991, CompType, CompName, VarDesc, VarValue);
    // add to tabular output reports
    OutputReportPredefined::AddCompSizeTableStrEntry(state, CompType, CompName, VarDesc, VarValue);

    // add to SQL output
    if (state.dataSQLiteProcedures->sqlite) {
        state.dataSQLiteProcedures->sqlite->addSQLiteComponentSizingStrRecord(CompType, CompName, VarDesc, VarValue);
    }
}

void BaseSizer::selectSizerOutput(EnergyPlusData &state, bool &errorsFound)
{
    if (this->printWarningFlag) {
        if (this->dataEMSOverrideON) { // EMS overrides value
            this->autoSizedValue = this->dataEMSOverride;
            this->reportSizerOutput(
                state, this->compType, this->compName, "User-Specified " + this->sizingStringScalable + this->sizingString, this->autoSizedValue);
        } else if (this->hardSizeNoDesignRun && !this->wasAutoSized && Util::SameString(this->compType, "Fan:ZoneExhaust")) {
            this->autoSizedValue = this->originalValue;
        } else if (this->wasAutoSized && this->dataFractionUsedForSizing > 0.0 && this->dataConstantUsedForSizing > 0.0) {
            this->autoSizedValue = this->dataFractionUsedForSizing * this->dataConstantUsedForSizing;
            this->reportSizerOutput(
                state, this->compType, this->compName, "Design Size " + this->sizingStringScalable + this->sizingString, this->autoSizedValue);
        } else if (!this->wasAutoSized &&
                   (this->autoSizedValue == this->originalValue || this->autoSizedValue == 0.0)) { // no sizing run done or autosizes to 0
            this->autoSizedValue = this->originalValue;
            if (this->dataAutosizable || (!this->sizingDesRunThisZone && Util::SameString(this->compType, "Fan:ZoneExhaust"))) {
                this->reportSizerOutput(
                    state, this->compType, this->compName, "User-Specified " + this->sizingStringScalable + this->sizingString, this->autoSizedValue);
            }
        } else if (!this->wasAutoSized && this->autoSizedValue >= 0.0 && this->originalValue == 0.0) { // input was blank or zero
            this->autoSizedValue = this->originalValue;
            this->reportSizerOutput(
                state, this->compType, this->compName, "User-Specified " + this->sizingStringScalable + this->sizingString, this->autoSizedValue);
        } else if (this->wasAutoSized && this->autoSizedValue >= 0.0 &&
                   this->originalValue <= 0.0) { // autosized to 0 or greater and input is 0 or autosize
            // might need more logic here to catch everything correctly
            if (this->dataScalableSizingON && int(this->zoneAirFlowSizMethod) > 0) {
                this->reportSizerOutput(
                    state, this->compType, this->compName, "User-Specified " + this->sizingStringScalable + this->sizingString, this->autoSizedValue);
            } else {
                this->reportSizerOutput(state, this->compType, this->compName, "Design Size " + this->sizingString, this->autoSizedValue);
            }
        } else if (this->autoSizedValue >= 0.0 && this->originalValue > 0.0) {
            if ((std::abs(this->autoSizedValue - this->originalValue) / this->originalValue) > state.dataSize->AutoVsHardSizingThreshold) {
                if (this->dataAutosizable) {
                    this->reportSizerOutput(state,
                                            this->compType,
                                            this->compName,
                                            "Design Size " + this->sizingString,
                                            this->autoSizedValue,
                                            "User-Specified " + this->sizingStringScalable + this->sizingString,
                                            this->originalValue);
                }
            } else {
                if (this->dataAutosizable) {
                    this->reportSizerOutput(state,
                                            this->compType,
                                            this->compName,
                                            "User-Specified " + this->sizingStringScalable + this->sizingString,
                                            this->originalValue);
                }
            }
            if (state.dataGlobal->DisplayExtraWarnings && this->dataAutosizable) {
                if ((std::abs(this->autoSizedValue - this->originalValue) / this->originalValue) > state.dataSize->AutoVsHardSizingThreshold) {
                    std::string msg = this->callingRoutine + ": Potential issue with equipment sizing for " + this->compType + ' ' + this->compName;
                    this->addErrorMessage(msg);
                    ShowMessage(state, msg);
                    msg = format("User-Specified {}{} = {:.5R}", this->sizingStringScalable, this->sizingString, this->originalValue);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("differs from Design Size {} = {:.5R}", this->sizingString, this->autoSizedValue);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = "This may, or may not, indicate mismatched component sizes.";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = "Verify that the value entered is intended and is consistent with other components.";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                }
            }
            if (!this->wasAutoSized) {
                this->autoSizedValue = this->originalValue;
            }
        } else if (this->wasAutoSized && this->autoSizedValue != DataSizing::AutoSize) {
            this->reportSizerOutput(
                state, this->compType, this->compName, "Design Size " + this->sizingStringScalable + this->sizingString, this->autoSizedValue);
        } else {
            std::string msg = this->callingRoutine + ' ' + this->compType + ' ' + this->compName + ", Developer Error: Component sizing incomplete.";
            this->addErrorMessage(msg);
            ShowSevereError(state, msg);
            msg = format("SizingString = {}, SizingResult = {:.1T}", this->sizingString, this->originalValue);
            this->addErrorMessage(msg);
            ShowContinueError(state, msg);
            this->errorType = AutoSizingResultType::ErrorType1;
        }
    } else if (!this->wasAutoSized && !this->autoCalculate) {
        this->autoSizedValue = this->originalValue;
    }
    this->overrideSizeString = true; // reset for next sizer
    if (this->errorType != AutoSizingResultType::NoError) {
        std::string msg = "Developer Error: sizing of " + this->sizingString + " failed.";
        this->addErrorMessage(msg);
        ShowSevereError(state, msg);
        msg = "Occurs in " + this->compType + " " + this->compName;
        this->addErrorMessage(msg);
        ShowContinueError(state, msg);
        errorsFound = true;
    }
}

void BaseSizer::select2StgDXHumCtrlSizerOutput(EnergyPlusData &state, bool &errorsFound)
{
    if (this->printWarningFlag) {
        if (this->dataEMSOverrideON) { // EMS overrides value
            this->reportSizerOutput(
                state, this->compType, this->compName, "User-Specified " + this->sizingStringScalable + this->sizingString, this->autoSizedValue);
            if (Util::SameString(this->compType, "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE")) {
                this->autoSizedValue *= (1 - this->dataBypassFrac); // now reapply for second message and remianing simulation calcs
                this->reportSizerOutput(state,
                                        this->compType,
                                        this->compName,
                                        "User-Specified " + this->sizingStringScalable + this->sizingString + " ( non-bypassed )",
                                        this->autoSizedValue);
            }
        } else if (!this->wasAutoSized && (this->autoSizedValue == this->originalValue || this->autoSizedValue == 0.0)) { // no sizing run done
            this->autoSizedValue = this->originalValue;
            this->reportSizerOutput(
                state, this->compType, this->compName, "User-Specified " + this->sizingStringScalable + this->sizingString, this->autoSizedValue);
            if (Util::SameString(this->compType, "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE")) {
                this->autoSizedValue *= (1 - this->dataBypassFrac); // now reapply for second message and remianing simulation calcs
                this->reportSizerOutput(state,
                                        this->compType,
                                        this->compName,
                                        "User-Specified " + this->sizingStringScalable + this->sizingString + " ( non-bypassed )",
                                        this->autoSizedValue);
            }
        } else if (!this->wasAutoSized && this->autoSizedValue >= 0.0 && this->originalValue == 0.0) { // input was blank or zero
            this->autoSizedValue = this->originalValue;
            this->reportSizerOutput(
                state, this->compType, this->compName, "User-Specified " + this->sizingStringScalable + this->sizingString, this->autoSizedValue);
            if (Util::SameString(this->compType, "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE")) {
                this->autoSizedValue *= (1 - this->dataBypassFrac); // now reapply for second message and remianing simulation calcs
                this->reportSizerOutput(state,
                                        this->compType,
                                        this->compName,
                                        "User-Specified " + this->sizingStringScalable + this->sizingString + " ( non-bypassed )",
                                        this->autoSizedValue);
            }
        } else if (this->wasAutoSized && this->autoSizedValue >= 0.0 &&
                   this->originalValue <= 0.0) { // autosized to 0 or greater and input is 0 or autosize
                                                 // might need more logic here to catch everything correctly
            if (this->dataScalableSizingON && int(this->zoneAirFlowSizMethod) > 0) {
                this->reportSizerOutput(
                    state, this->compType, this->compName, "User-Specified " + this->sizingStringScalable + this->sizingString, this->autoSizedValue);
            } else {
                this->reportSizerOutput(state, this->compType, this->compName, "Design Size " + this->sizingString, this->autoSizedValue);
            }
            if (Util::SameString(this->compType, "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE")) {
                this->autoSizedValue *= (1 - this->dataBypassFrac); // now reapply for second message and remianing simulation calcs
                this->reportSizerOutput(
                    state, this->compType, this->compName, "Design Size " + this->sizingString + " ( non-bypassed )", this->autoSizedValue);
            }
        } else if (this->autoSizedValue >= 0.0 && this->originalValue > 0.0) {
            if ((std::abs(this->autoSizedValue - this->originalValue) / this->originalValue) > state.dataSize->AutoVsHardSizingThreshold) {
                this->reportSizerOutput(state,
                                        this->compType,
                                        this->compName,
                                        "Design Size " + this->sizingString,
                                        this->autoSizedValue,
                                        "User-Specified " + this->sizingStringScalable + this->sizingString,
                                        this->originalValue);
                if (Util::SameString(this->compType, "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE")) {
                    this->autoSizedValue *= (1 - this->dataBypassFrac); // now reapply for second message and remianing simulation calcs
                    this->originalValue *= (1 - this->dataBypassFrac);  // now reapply for second message and remianing simulation calcs
                    this->reportSizerOutput(state,
                                            this->compType,
                                            this->compName,
                                            "Design Size " + this->sizingString + " ( non-bypassed )",
                                            this->autoSizedValue,
                                            "User-Specified " + this->sizingStringScalable + this->sizingString + " ( non-bypassed )",
                                            this->originalValue);
                }
            } else {
                if (Util::SameString(this->compType, "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE")) {
                    this->autoSizedValue /= (1 - this->dataBypassFrac); // back out bypass fraction applied in GetInput
                }
                this->reportSizerOutput(
                    state, this->compType, this->compName, "User-Specified " + this->sizingStringScalable + this->sizingString, this->originalValue);
                if (Util::SameString(this->compType, "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE")) {
                    this->autoSizedValue *= (1 - this->dataBypassFrac); // now reapply for second message and remianing simulation calcs
                    this->reportSizerOutput(state,
                                            this->compType,
                                            this->compName,
                                            "User-Specified " + this->sizingStringScalable + this->sizingString + " ( non-bypassed )",
                                            this->autoSizedValue);
                }
            }
            if (state.dataGlobal->DisplayExtraWarnings) {
                if ((std::abs(this->autoSizedValue - this->originalValue) / this->originalValue) > state.dataSize->AutoVsHardSizingThreshold) {
                    std::string msg = this->callingRoutine + ": Potential issue with equipment sizing for " + this->compType + ' ' + this->compName;
                    this->addErrorMessage(msg);
                    ShowMessage(state, msg);
                    msg = format("User-Specified {}{} = {:.5R}", this->sizingStringScalable, this->sizingString, this->originalValue);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = format("differs from Design Size {} = {:.5R}", this->sizingString, this->autoSizedValue);
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = "This may, or may not, indicate mismatched component sizes.";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                    msg = "Verify that the value entered is intended and is consistent with other components.";
                    this->addErrorMessage(msg);
                    ShowContinueError(state, msg);
                }
            }
            if (!this->wasAutoSized) {
                this->autoSizedValue = this->originalValue;
            }
        } else if (this->wasAutoSized && this->autoSizedValue != DataSizing::AutoSize) {
            this->reportSizerOutput(
                state, this->compType, this->compName, "Design Size " + this->sizingStringScalable + this->sizingString, this->autoSizedValue);
        } else {
            std::string msg = this->callingRoutine + ' ' + this->compType + ' ' + this->compName + ", Developer Error: Component sizing incomplete.";
            this->addErrorMessage(msg);
            ShowSevereError(state, msg);
            msg = format("SizingString = {}, SizingResult = {:.1T}", this->sizingString, this->originalValue);
            this->addErrorMessage(msg);
            ShowContinueError(state, msg);
            this->errorType = AutoSizingResultType::ErrorType1;
        }
    } else if (!this->wasAutoSized && !this->autoCalculate) {
        this->autoSizedValue = this->originalValue;
    }
    this->overrideSizeString = true; // reset for next sizer
    if (this->errorType != AutoSizingResultType::NoError) {
        std::string msg = "Developer Error: sizing of " + this->sizingString + " failed.";
        this->addErrorMessage(msg);
        ShowSevereError(state, msg);
        msg = "Occurs in " + this->compType + " " + this->compName;
        this->addErrorMessage(msg);
        ShowContinueError(state, msg);
        errorsFound = true;
    }
}

bool BaseSizer::isValidCoilType(std::string const &_compType)
{
    int coilNum = 0;
    for (auto const &coilType : HVAC::cAllCoilTypes) {
        coilNum += 1;
        if (Util::SameString(_compType, coilType)) {
            this->coilType_Num = coilNum;
            return true;
        }
    }
    this->coilType_Num = 0;
    return false;
}

bool BaseSizer::isValidFanType(std::string const &_compType)
{
    // if compType name is one of the fan objects, then return true
    if (Util::SameString(_compType, "Fan:SystemModel")) {
        return true;
    } else if (Util::SameString(_compType, "Fan:ComponentModel")) {
        return true;
    } else if (Util::SameString(_compType, "Fan:OnOff")) {
        return true;
    } else if (Util::SameString(_compType, "Fan:ConstantVolume")) {
        return true;
    } else if (Util::SameString(_compType, "Fan:VariableVolume")) {
        return true;
    } else {
        return false;
    }
}

bool BaseSizer::checkInitialized(EnergyPlusData &state, bool &errorsFound)
{
    if (!this->initialized) {
        errorsFound = true;
        this->errorType = AutoSizingResultType::ErrorType2;
        this->autoSizedValue = 0.0;
        std::string msg = "Developer Error: uninitialized sizing of " + this->sizingString + ".";
        this->addErrorMessage(msg);
        ShowSevereError(state, msg);
        msg = "Occurs in " + this->compType + " " + this->compName;
        this->addErrorMessage(msg);
        ShowContinueError(state, msg);
        return false;
    }
    return true;
}

void BaseSizer::overrideSizingString(std::string_view const string)
{
    this->sizingString = string;
    this->overrideSizeString = false;
}

Real64 BaseSizer::setOAFracForZoneEqSizing(const EnergyPlusData &state, Real64 const desMassFlow, DataSizing::ZoneEqSizingData const &zoneEqSizing)
{
    Real64 outAirFrac = 0.0;
    if (desMassFlow <= 0.0) {
        return outAirFrac;
    }

    if (zoneEqSizing.ATMixerVolFlow > 0.0) {
        // set central DOAS AT mixer OA fraction
        outAirFrac = min(state.dataEnvrn->StdRhoAir * zoneEqSizing.ATMixerVolFlow / desMassFlow, 1.0);
    } else if (zoneEqSizing.OAVolFlow > 0.0) { // set zone equipment OA fraction
        outAirFrac = min(state.dataEnvrn->StdRhoAir * zoneEqSizing.OAVolFlow / desMassFlow, 1.0);
    }
    return outAirFrac;
}

Real64 BaseSizer::setHeatCoilInletTempForZoneEqSizing(Real64 const outAirFrac,
                                                      DataSizing::ZoneEqSizingData const &zoneEqSizing,
                                                      DataSizing::ZoneSizingData const &finalZoneSizing)
{
    Real64 coilInTemp = 0.0;
    if (zoneEqSizing.ATMixerVolFlow > 0.0) {
        // adjust for central DOAS AT mixer mixed inlet temp
        coilInTemp = (1.0 - outAirFrac) * finalZoneSizing.ZoneRetTempAtHeatPeak + outAirFrac * zoneEqSizing.ATMixerHeatPriDryBulb;
    } else if (zoneEqSizing.OAVolFlow > 0.0) {
        // adjust for raw OA mixed inlet temp
        coilInTemp = (1.0 - outAirFrac) * finalZoneSizing.ZoneTempAtHeatPeak + outAirFrac * finalZoneSizing.OutTempAtHeatPeak;
    } else {
        // use zone condition for sizing zone equipment
        coilInTemp = finalZoneSizing.ZoneTempAtHeatPeak;
    }
    return coilInTemp;
}

Real64 BaseSizer::setHeatCoilInletHumRatForZoneEqSizing(Real64 const outAirFrac,
                                                        DataSizing::ZoneEqSizingData const &zoneEqSizing,
                                                        DataSizing::ZoneSizingData const &finalZoneSizing)
{
    Real64 coilInHumRat = 0.0;
    if (zoneEqSizing.ATMixerVolFlow > 0.0) {
        // adjust for central DOAS AT mixer mixed inlet humrat
        coilInHumRat = (1.0 - outAirFrac) * finalZoneSizing.ZoneHumRatAtHeatPeak + outAirFrac * zoneEqSizing.ATMixerHeatPriHumRat;
    } else if (zoneEqSizing.OAVolFlow > 0.0) { // adjust for raw OA mixed inlet humrat
        coilInHumRat = (1.0 - outAirFrac) * finalZoneSizing.ZoneHumRatAtHeatPeak + outAirFrac * finalZoneSizing.OutHumRatAtHeatPeak;
    } else {
        coilInHumRat = finalZoneSizing.ZoneHumRatAtHeatPeak;
    }
    return coilInHumRat;
}

Real64 BaseSizer::setCoolCoilInletTempForZoneEqSizing(Real64 const outAirFrac,
                                                      DataSizing::ZoneEqSizingData const &zoneEqSizing,
                                                      DataSizing::ZoneSizingData const &finalZoneSizing)
{
    Real64 coilInTemp = 0.0;
    if (zoneEqSizing.ATMixerVolFlow > 0.0) {
        // adjust for central DOAS AT mixer mixed inlet temp
        coilInTemp = (1.0 - outAirFrac) * finalZoneSizing.ZoneRetTempAtCoolPeak + outAirFrac * zoneEqSizing.ATMixerCoolPriDryBulb;
    } else if (zoneEqSizing.OAVolFlow > 0.0) {
        // adjust for raw OA mixed inlet temp
        coilInTemp = (1.0 - outAirFrac) * finalZoneSizing.ZoneTempAtCoolPeak + outAirFrac * finalZoneSizing.OutTempAtCoolPeak;
    } else {
        // use zone condition for sizing zone equipment
        coilInTemp = finalZoneSizing.ZoneTempAtCoolPeak;
    }
    return coilInTemp;
}

Real64 BaseSizer::setCoolCoilInletHumRatForZoneEqSizing(Real64 const outAirFrac,
                                                        DataSizing::ZoneEqSizingData const &zoneEqSizing,
                                                        DataSizing::ZoneSizingData const &finalZoneSizing)
{
    Real64 coilInHumRat = 0.0;
    if (zoneEqSizing.ATMixerVolFlow > 0.0) {
        // adjust for central DOAS AT mixer mixed inlet humrat
        coilInHumRat = (1.0 - outAirFrac) * finalZoneSizing.ZoneHumRatAtCoolPeak + outAirFrac * zoneEqSizing.ATMixerCoolPriHumRat;
    } else if (zoneEqSizing.OAVolFlow > 0.0) { // adjust for raw OA mixed inlet humrat
        coilInHumRat = (1.0 - outAirFrac) * finalZoneSizing.ZoneHumRatAtCoolPeak + outAirFrac * finalZoneSizing.OutHumRatAtCoolPeak;
    } else {
        coilInHumRat = finalZoneSizing.ZoneHumRatAtCoolPeak;
    }
    return coilInHumRat;
}

void BaseSizer::clearState()
{
    stdRhoAir = 0.0;

    zoneAirFlowSizMethod = 0;
    dataScalableSizingON = false;
    dataScalableCapSizingON = false;
    isCoilReportObject = false; // provides access to coil reporting
    isFanReportObject = false;  // provides access to fan reporting
    initialized = false;        // indicates initializeWithinEP was called
    errorType = AutoSizingResultType::NoError;
    sizingString = "";
    sizingStringScalable = "";
    overrideSizeString = true;
    originalValue = 0.0;
    autoSizedValue = 0.0;
    wasAutoSized = false;
    hardSizeNoDesignRun = false;
    sizingDesRunThisAirSys = false;
    sizingDesRunThisZone = false;
    sizingDesValueFromParent = false;
    airLoopSysFlag = false;
    oaSysFlag = false;
    coilType_Num = 0;
    compType = "";
    compName = "";
    isEpJSON = false;

    sysSizingRunDone = false;
    zoneSizingRunDone = false;
    curSysNum = 0;
    curOASysNum = 0;
    curZoneEqNum = 0;
    curDuctType = HVAC::AirDuctType::Invalid;
    curTermUnitSizingNum = 0; // index in zone equipment vector - for single duct, IU, and PIU
    numPrimaryAirSys = 0;
    numSysSizInput = 0;
    doSystemSizing = false;
    numZoneSizingInput = 0;
    doZoneSizing = false;
    autoCalculate = false; // indicator that AutoCalculate is used

    // terminal units
    termUnitSingDuct = false; // single duct terminal unit
    termUnitPIU = false;      // powered induction unit
    termUnitIU = false;       // induction terminal unit
    zoneEqFanCoil = false;    // fan coil zone equipment
    otherEqType = false;      // this covers the ELSE type switch
    zoneEqUnitHeater = false; // unit heater zone equipment
    zoneEqUnitVent = false;   // unit ventilator zone equipment
    zoneEqVentedSlab = false; // ventilated slab zone equipment

    // error message handling
    getLastErrorMessages();

    // global sizing data
    minOA = DataSizing::OAControl::Invalid;

    // global Data* sizing constants
    dataEMSOverrideON = false;
    dataEMSOverride = 0.0;
    dataAutosizable = false;
    dataConstantUsedForSizing = 0.0;
    dataFractionUsedForSizing = 0.0;
    dataDXCoolsLowSpeedsAutozize = false;
    dataPltSizHeatNum = 0;
    dataWaterLoopNum = 0;
    dataFanIndex = -1;
    dataFanType = HVAC::FanType::Invalid;
    dataWaterCoilSizCoolDeltaT = 0.0;
    dataWaterCoilSizHeatDeltaT = 0.0;
    dataCapacityUsedForSizing = 0.0;
    dataPltSizCoolNum = 0;
    dataDesInletAirHumRat = 0.0;
    dataFlowUsedForSizing = 0.0;
    dataDesOutletAirHumRat = 0.0;
    dataDesInletWaterTemp = 0.0;
    dataDesOutletAirTemp = 0.0;
    dataWaterFlowUsedForSizing = 0.0;
    dataSizingFraction = 1.0;
    dataDXSpeedNum = 0.0;
    dataAirFlowUsedForSizing = 0.0;
    dataDesInletAirTemp = 0.0;
    dataDesAccountForFanHeat = false;
    dataFanPlacement = HVAC::FanPlace::Invalid;
    dataDesicRegCoil = false;
    dataHeatSizeRatio = 0.0;
    dataZoneUsedForSizing = 0;
    dataDesicDehumNum = 0;
    dataNomCapInpMeth = false;
    dataCoilNum = 0;
    dataFanOp = HVAC::FanOp::Invalid;
    dataDesignCoilCapacity = 0.0;
    dataErrorsFound = false;
    dataBypassFrac = 0.0;
    dataIsDXCoil = false;

    dataNonZoneNonAirloopValue = 0.0;

    printWarningFlag = false;
    callingRoutine = "";
    sysSizingInputData.clear();
    zoneSizingInput.clear();
    unitarySysEqSizing.clear();
    oaSysEqSizing.clear();
    zoneEqSizing.clear();
    outsideAirSys.clear();
    termUnitSizing.clear();
    termUnitFinalZoneSizing.clear();
    finalZoneSizing.clear();
    finalSysSizing.clear();
    plantSizData.clear();
    primaryAirSystem.clear();
    airloopDOAS.clear();
}

} // namespace EnergyPlus
