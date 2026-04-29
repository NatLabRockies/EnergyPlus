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

#include <EnergyPlus/Autosizing/CoolingWaterDesAirOutletHumRatSizing.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

Real64 CoolingWaterDesAirOutletHumRatSizer::size(EnergyPlusData &state, Real64 _originalValue, bool &errorsFound)
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
                Real64 TDpIn = Psychrometrics::PsyTdpFnWPb(state, this->dataDesInletAirHumRat, state.dataEnvrn->StdBaroPress);
                if (TDpIn <= this->dataDesInletWaterTemp) {
                    this->autoSizedValue = this->dataDesInletAirHumRat;
                } else {
                    this->autoSizedValue = min(Psychrometrics::PsyWFnTdbRhPb(state, this->dataDesOutletAirTemp, 0.9, state.dataEnvrn->StdBaroPress),
                                               this->dataDesInletAirHumRat);
                }
            } else {
                this->autoSizedValue = this->finalZoneSizing(this->curZoneEqNum).CoolDesHumRat;
            }
        }
    } else if (this->curSysNum > 0) {
        if (!this->wasAutoSized && !this->sizingDesRunThisAirSys) {
            this->autoSizedValue = _originalValue;
        } else {
            if (this->curOASysNum > 0) { // coil is in OA stream
                if (this->outsideAirSys(this->curOASysNum).AirLoopDOASNum > -1) {
                    this->autoSizedValue = this->airloopDOAS[this->outsideAirSys(this->curOASysNum).AirLoopDOASNum].PrecoolHumRat;
                } else {
                    this->autoSizedValue = this->finalSysSizing(this->curSysNum).PrecoolHumRat;
                }
            } else if (this->dataDesOutletAirHumRat > 0.0) {
                this->autoSizedValue = this->dataDesOutletAirHumRat;
            } else {
                this->autoSizedValue = this->finalSysSizing(this->curSysNum).CoolSupHumRat;
            }
        }
    }

    if (this->wasAutoSized) {
        if (this->autoSizedValue > this->dataDesInletAirHumRat &&
            (Util::SameString(this->compType, "COIL:COOLING:WATER") || Util::SameString(this->compType, "COIL:COOLING:WATER:DETAILEDGEOMETRY"))) {
            std::string msg =
                this->callingRoutine + ":" + " Coil=\"" + this->compName + "\", Cooling Coil has leaving humidity ratio > entering humidity ratio.";
            this->addErrorMessage(msg);
            ShowWarningError(state, msg);
            msg = EnergyPlus::format("    Wair,in =  {:.6R} [kgWater/kgDryAir]", this->dataDesInletAirHumRat);
            this->addErrorMessage(msg);
            ShowContinueError(state, msg);
            msg = EnergyPlus::format("    Wair,out = {:.6R} [kgWater/kgDryAir]", this->autoSizedValue);
            this->addErrorMessage(msg);
            ShowContinueError(state, msg);
            if (this->dataDesInletAirHumRat > 0.016) {
                this->autoSizedValue = 0.5 * this->dataDesInletAirHumRat;
            } else {
                this->autoSizedValue = this->dataDesInletAirHumRat;
            }
            msg = "....coil leaving humidity ratio will be reset to:";
            this->addErrorMessage(msg);
            ShowContinueError(state, msg);
            msg = EnergyPlus::format("    Wair,out = {:.6R} [kgWater/kgDryAir]", this->autoSizedValue);
            this->addErrorMessage(msg);
            ShowContinueError(state, msg);
        }
        // check for dry coil and reset outlet humrat if needed
        Real64 desSatEnthAtWaterInTemp = Psychrometrics::PsyHFnTdbW(
            this->dataDesInletWaterTemp, Psychrometrics::PsyWFnTdpPb(state, this->dataDesInletWaterTemp, state.dataEnvrn->StdBaroPress));
        Real64 desHumRatAtWaterInTemp = Psychrometrics::PsyWFnTdbH(state, this->dataDesInletWaterTemp, desSatEnthAtWaterInTemp, this->callingRoutine);
        if (this->autoSizedValue < this->dataDesInletAirHumRat && desHumRatAtWaterInTemp > this->dataDesInletAirHumRat) {
            if (this->autoSizedValue < this->dataDesInletAirHumRat &&
                (Util::SameString(this->compType, "COIL:COOLING:WATER") || Util::SameString(this->compType, "COIL:COOLING:WATER:DETAILEDGEOMETRY"))) {
                std::string msg = this->callingRoutine + ":" + " Coil=\"" + this->compName +
                                  "\", Cooling Coil is running dry for sizing and has minimum humidity ratio at saturation for inlet chilled water "
                                  "temperature > design air entering humidity ratio.";
                this->addErrorMessage(msg);
                ShowWarningError(state, msg);
                msg = EnergyPlus::format("    Wair,in =  {:.6R} [kgWater/kgDryAir]", this->dataDesInletAirHumRat);
                this->addErrorMessage(msg);
                ShowContinueError(state, msg);
                msg = EnergyPlus::format("    Wair,out = {:.6R} [kgWater/kgDryAir]", this->autoSizedValue);
                this->addErrorMessage(msg);
                ShowContinueError(state, msg);
                msg = EnergyPlus::format("    Inlet chilled water temperature = {:.3R} [C]", this->dataDesInletWaterTemp);
                this->addErrorMessage(msg);
                ShowContinueError(state, msg);
                msg = EnergyPlus::format("    Minimum humidity ratio at saturation for inlet chilled water temperature = {:.6R} [kgWater/kgDryAir]",
                                         desHumRatAtWaterInTemp);
                this->addErrorMessage(msg);
                ShowContinueError(state, msg);
                this->autoSizedValue = this->dataDesInletAirHumRat;
                msg = "....coil leaving humidity ratio will be reset to:";
                this->addErrorMessage(msg);
                ShowContinueError(state, msg);
                msg = EnergyPlus::format("    Wair,out = {:.6R} [kgWater/kgDryAir]", this->autoSizedValue);
                this->addErrorMessage(msg);
                ShowContinueError(state, msg);
            }
        }
    }
    if (this->overrideSizeString) {
        this->sizingString = "Design Outlet Air Humidity Ratio [kgWater/kgDryAir]";
    }
    this->selectSizerOutput(state, errorsFound);
    if (this->isCoilReportObject) {
        ReportCoilSelection::setCoilLvgAirHumRat(state, this->coilReportNum, this->autoSizedValue);
    }
    return this->autoSizedValue;
}

} // namespace EnergyPlus
