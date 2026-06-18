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
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/Autosizing/BaseSizerWithFanHeatInputs.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/Fans.hh>

namespace EnergyPlus {

void BaseSizerWithFanHeatInputs::initializeWithinEP(EnergyPlusData &state,
                                                    std::string_view const _compType,
                                                    std::string_view const _compName,
                                                    bool _printWarningFlag,
                                                    std::string_view const _callingRoutine)
{
    BaseSizer::initializeWithinEP(state, _compType, _compName, _printWarningFlag, _callingRoutine);
    this->dataDesAccountForFanHeat = state.dataSize->DataDesAccountForFanHeat;
    // water coils on main branch have no parent object to set DataFan* variables
    if (!(this->primaryAirSystem.empty()) && this->curSysNum > 0 && this->curOASysNum == 0) {
        if (this->primaryAirSystem(this->curSysNum).supFanType != HVAC::FanType::Invalid) {
            this->dataFanType = this->primaryAirSystem(this->curSysNum).supFanType;
            this->dataFanIndex = this->primaryAirSystem(this->curSysNum).supFanNum;
        }
    }
    this->getFanInputsForDesHeatGain(state,
                                     this->dataFanIndex,
                                     this->deltaP,
                                     this->motEff,
                                     this->totEff,
                                     this->motInAirFrac,
                                     this->fanShaftPow,
                                     this->motInPower,
                                     this->fanCompModel);
}

Real64 BaseSizerWithFanHeatInputs::calcFanDesHeatGain(Real64 const &airVolFlow)
{
    if (this->dataFanType == HVAC::FanType::Invalid || this->dataFanIndex == 0) {
        return 0.0;
    }
    if (this->fanCompModel) {
        return this->fanShaftPow + (this->motInPower - this->fanShaftPow) * this->motInAirFrac;
    }
    Real64 const fanPowerTot = (airVolFlow * this->deltaP) / this->totEff;
    return this->motEff * fanPowerTot + (fanPowerTot - this->motEff * fanPowerTot) * this->motInAirFrac;
}

void BaseSizerWithFanHeatInputs::getFanInputsForDesHeatGain(EnergyPlusData &state,
                                                            int fanIndex,
                                                            Real64 &deltaP,
                                                            Real64 &motEff,
                                                            Real64 &totEff,
                                                            Real64 &motInAirFrac,
                                                            Real64 &fanShaftPow,
                                                            Real64 &motInPower,
                                                            bool &fanCompModel)
{
    // if fan unknown or air flow sizing (recursive call to size fan) then return
    if (fanIndex <= 0 || this->isFanReportObject) {
        return;
    }

    // This should work for all fan types now
    state.dataFans->fans(fanIndex)->getInputsForDesignHeatGain(state, deltaP, motEff, totEff, motInAirFrac, fanShaftPow, motInPower, fanCompModel);
}

void BaseSizerWithFanHeatInputs::setDataDesAccountForFanHeat(EnergyPlusData &state, bool flag)
{
    state.dataSize->DataDesAccountForFanHeat = flag;
}

} // namespace EnergyPlus
