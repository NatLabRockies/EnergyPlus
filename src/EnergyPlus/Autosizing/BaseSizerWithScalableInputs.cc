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

// C++ Headers
#include <string>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/BaseSizerWithFanHeatInputs.hh>
#include <EnergyPlus/Autosizing/BaseSizerWithScalableInputs.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/Fans.hh>

namespace EnergyPlus {

void BaseSizerWithScalableInputs::initializeWithinEP(EnergyPlusData &state,
                                                     std::string_view const _compType,
                                                     std::string_view const _compName,
                                                     bool _printWarningFlag,
                                                     std::string_view const _callingRoutine)
{
    BaseSizerWithFanHeatInputs::initializeWithinEP(state, _compType, _compName, _printWarningFlag, _callingRoutine);

    this->dataScalableSizingON = state.dataSize->DataScalableSizingON;
    this->dataScalableCapSizingON = state.dataSize->DataScalableCapSizingON;
    this->dataHRFlowSizingFlag = state.dataSize->HRFlowSizingFlag;
    this->zoneCoolingOnlyFan = state.dataSize->ZoneCoolingOnlyFan;
    this->zoneHeatingOnlyFan = state.dataSize->ZoneHeatingOnlyFan;
    this->dataFracOfAutosizedCoolingAirflow = state.dataSize->DataFracOfAutosizedCoolingAirflow;
    this->dataFracOfAutosizedHeatingAirflow = state.dataSize->DataFracOfAutosizedHeatingAirflow;
    this->dataFlowPerCoolingCapacity = state.dataSize->DataFlowPerCoolingCapacity;
    this->dataAutosizedCoolingCapacity = state.dataSize->DataAutosizedCoolingCapacity;
    this->dataFlowPerHeatingCapacity = state.dataSize->DataFlowPerHeatingCapacity;
    this->dataAutosizedHeatingCapacity = state.dataSize->DataAutosizedHeatingCapacity;

    this->dataCoilSizingAirInTemp = state.dataSize->DataCoilSizingAirInTemp;
    this->dataCoilSizingAirInHumRat = state.dataSize->DataCoilSizingAirInHumRat;
    this->dataCoilSizingAirOutTemp = state.dataSize->DataCoilSizingAirOutTemp;
    this->dataCoilSizingAirOutHumRat = state.dataSize->DataCoilSizingAirOutHumRat;
    this->dataCoilSizingFanCoolLoad = state.dataSize->DataCoilSizingFanCoolLoad;
    this->dataCoilSizingCapFT = state.dataSize->DataCoilSizingCapFT;
    this->dataTotCapCurveIndex = state.dataSize->DataTotCapCurveIndex;
    this->dataTotCapCurveValue = state.dataSize->DataTotCapCurveValue;
    this->dataFracOfAutosizedCoolingCapacity = state.dataSize->DataFracOfAutosizedCoolingCapacity;
    this->dataFracOfAutosizedHeatingCapacity = state.dataSize->DataFracOfAutosizedHeatingCapacity;
    this->dataCoolCoilCap = state.dataSize->DataCoolCoilCap;
    this->dataCoilIsSuppHeater = state.dataSize->DataCoilIsSuppHeater;
    this->suppHeatCap = state.dataSize->SuppHeatCap;
    this->unitaryHeatCap = state.dataSize->UnitaryHeatCap;
    this->dataCoolCoilType = state.dataSize->DataCoolCoilType;
    this->dataCoolCoilIndex = state.dataSize->DataCoolCoilIndex;

    this->zoneHVACSizing = state.dataSize->ZoneHVACSizing;

    // set supply air fan properties
    if (this->isCoilReportObject && this->curSysNum > 0 && int(this->primaryAirSystem.size()) > 0 &&
        this->curSysNum <= state.dataHVACGlobal->NumPrimaryAirSys) {

        // int RetFanNum = this->primaryAirSystem(this->curSysNum).RetFanNum;

        // This should work for both fan types
        if (this->primaryAirSystem(this->curSysNum).supFanNum > 0) {
            ReportCoilSelection::setCoilSupplyFanInfo(state,
                                                      this->coilReportNum,
                                                      state.dataFans->fans(this->primaryAirSystem(this->curSysNum).supFanNum)->Name,
                                                      state.dataFans->fans(this->primaryAirSystem(this->curSysNum).supFanNum)->type,
                                                      this->primaryAirSystem(this->curSysNum).supFanNum);
        }
    }

    if (this->curZoneEqNum != 0) {
        if (this->zoneHVACSizingIndex > 0) {
            int coolingSAFMethod = this->zoneHVACSizing(this->zoneHVACSizingIndex).CoolingSAFMethod;
            this->zoneAirFlowSizMethod = coolingSAFMethod;
            this->dataFractionUsedForSizing = 1.0;
            this->dataConstantUsedForSizing = this->zoneHVACSizing(this->zoneHVACSizingIndex).MaxCoolAirVolFlow;
            if (coolingSAFMethod == DataSizing::FlowPerFloorArea) {
                state.dataSize->DataScalableSizingON = true;
                this->dataConstantUsedForSizing = this->zoneHVACSizing(this->zoneHVACSizingIndex).MaxCoolAirVolFlow *
                                                  state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
            } else if (coolingSAFMethod == DataSizing::FractionOfAutosizedCoolingAirflow) {
                state.dataSize->DataFracOfAutosizedCoolingAirflow = this->zoneHVACSizing(this->zoneHVACSizingIndex).MaxCoolAirVolFlow;
                state.dataSize->DataScalableSizingON = true;
            }
        } else {
            if (int(this->zoneEqSizing.size()) > 0 && int(this->zoneEqSizing(this->curZoneEqNum).SizingMethod.size()) > 0) {
                this->zoneAirFlowSizMethod = this->zoneEqSizing(this->curZoneEqNum).SizingMethod(int(this->sizingType));
            } else {
                this->zoneAirFlowSizMethod = 0;
            }
            if (this->zoneAirFlowSizMethod == 0) {
                // do nothing, sizing method not set
            } else if (this->zoneAirFlowSizMethod == DataSizing::SupplyAirFlowRate || this->zoneAirFlowSizMethod == DataSizing::None) {
            }
        }
    }
}

void BaseSizerWithScalableInputs::setHVACSizingIndexData(int const index)
{
    this->zoneHVACSizingIndex = index;
}

} // namespace EnergyPlus
