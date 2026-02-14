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

#include <ObjexxFCL/Array1D.hh>

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/PhaseChangeModeling/HysteresisModel.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace Material {

    Real64 MaterialPhaseChange::getEnthalpy(Real64 T, Real64 Tc, Real64 tau1, Real64 tau2) const
    {
        // Looks up the enthalpy on the characteristic curve defined by the parameters Tc, tau1, and tau2,
        // and the position on that curve defined by T.
        Real64 eta1 = (this->totalLatentHeat / 2) * exp(-2 * std::abs(T - Tc) / tau1);
        Real64 eta2 = (this->totalLatentHeat / 2) * exp(-2 * std::abs(T - Tc) / tau2);
        if (T <= Tc) {
            return (this->specificHeatSolid * T) + eta1;
        }
        return (this->specificHeatSolid * Tc) + this->totalLatentHeat + this->specificHeatLiquid * (T - Tc) - eta2;
    }

    Real64 MaterialPhaseChange::getCurrentSpecificHeat(
        Real64 prevTempTD, Real64 updatedTempTDT, Real64 phaseChangeTempReverse, Phase prevPhaseChangeState, Phase &phaseChangeState)
    {
        // Main public facing function; returns the current specific heat based on input properties, and current and previous conditions.
        // In a future version, this could be compartmentalized to track all states and histories, but it would require some further modification to
        // the HBFDManager
        Real64 TempLowPCM = this->peakTempMelting - this->deltaTempMeltingLow;
        Real64 TempHighPCM = this->peakTempMelting + this->deltaTempMeltingHigh;
        Real64 Tc;   // assigned later
        Real64 Tau1; // assigned later
        Real64 Tau2; // assigned later
        Real64 TempLowPCF = this->peakTempFreezing - this->deltaTempFreezingLow;
        Real64 TempHighPCF = this->peakTempFreezing + this->deltaTempFreezingHigh;
        Real64 Cp;
        Real64 phaseChangeDeltaT = prevTempTD - updatedTempTDT;

        // determine phase change state and curve characteristics based on delta T direction, updated temp, and previous state
        if (phaseChangeDeltaT <= 0) {
            Tc = this->peakTempMelting;
            Tau1 = this->deltaTempMeltingLow;
            Tau2 = this->deltaTempMeltingHigh;
            if (updatedTempTDT < TempLowPCM) {
                phaseChangeState = Phase::Crystallized;
            } else if (updatedTempTDT <= TempHighPCM) {
                phaseChangeState = Phase::Melting;
                if (prevPhaseChangeState == Phase::Freezing || prevPhaseChangeState == Phase::Transition) {
                    phaseChangeState = Phase::Transition;
                }
            } else {
                phaseChangeState = Phase::Liquid;
            }
        } else { // phaseChangeDeltaT > 0
            Tc = this->peakTempFreezing;
            Tau1 = this->deltaTempFreezingLow;
            Tau2 = this->deltaTempFreezingHigh;
            if (updatedTempTDT < TempLowPCF) {
                phaseChangeState = Phase::Crystallized;
            } else if (updatedTempTDT <= TempHighPCF) {
                phaseChangeState = Phase::Freezing;
                if (prevPhaseChangeState == Phase::Melting || prevPhaseChangeState == Phase::Transition) {
                    phaseChangeState = Phase::Transition;
                }
            } else {
                phaseChangeState = Phase::Liquid;
            }
        }

        // Why is phaseChangeTransition a state variable of the material and not the surface?
        // determine if we are transitioning or not
        if (prevPhaseChangeState == Phase::Transition && phaseChangeState == Phase::Crystallized) {
            this->phaseChangeTransition = true;
        } else if (prevPhaseChangeState == Phase::Transition && phaseChangeState == Phase::Freezing) {
            this->phaseChangeTransition = true;
            // this->phaseChangeState = 0; ?????
        } else if (prevPhaseChangeState == Phase::Freezing && phaseChangeState == Phase::Transition) {
            this->phaseChangeTransition = true;
        } else if (prevPhaseChangeState == Phase::Crystallized && phaseChangeState == Phase::Transition) {
            this->phaseChangeTransition = true;
        } else {
            this->phaseChangeTransition = false;
        }

        // now calculate the enthalpy appropriately
        if (!this->phaseChangeTransition) {
            this->enthOld = this->getEnthalpy(prevTempTD, Tc, Tau1, Tau2);
            this->enthNew = this->getEnthalpy(updatedTempTDT, Tc, Tau1, Tau2);
        } else {
            if (prevPhaseChangeState == Phase::Freezing && phaseChangeState == Phase::Transition) {
                this->enthRev =
                    this->getEnthalpy(phaseChangeTempReverse, this->peakTempFreezing, this->deltaTempFreezingLow, this->deltaTempFreezingHigh);
                this->enthNew = (this->specHeatTransition * updatedTempTDT) + (this->enthOld - (this->specHeatTransition * prevTempTD));
                this->enthalpyM = this->getEnthalpy(updatedTempTDT, this->peakTempMelting, this->deltaTempMeltingLow, this->deltaTempMeltingHigh);
                this->enthalpyF = this->getEnthalpy(updatedTempTDT, this->peakTempFreezing, this->deltaTempFreezingLow, this->deltaTempFreezingHigh);
                if (this->enthNew < this->enthRev && this->enthNew >= this->enthalpyF && updatedTempTDT <= prevTempTD) {
                    phaseChangeState = Phase::Freezing;
                    this->enthNew =
                        this->getEnthalpy(updatedTempTDT, this->peakTempFreezing, this->deltaTempFreezingLow, this->deltaTempFreezingHigh);
                } else if ((this->enthNew < this->enthalpyF) && (this->enthNew > this->enthalpyM)) {
                    phaseChangeState = Phase::Transition;
                    this->enthNew = (this->specHeatTransition * updatedTempTDT) + (this->enthOld - (this->specHeatTransition * prevTempTD));
                } else if ((this->enthNew < this->enthalpyF) && (updatedTempTDT > phaseChangeTempReverse)) {
                    phaseChangeState = Phase::Transition;
                    this->enthNew =
                        (this->specHeatTransition * updatedTempTDT) + (this->enthRev - (this->specHeatTransition * phaseChangeTempReverse));
                } else if ((this->enthNew <= this->enthalpyM) && (updatedTempTDT <= phaseChangeTempReverse)) {
                    phaseChangeState = Phase::Transition;
                    this->enthNew =
                        (this->specHeatTransition * updatedTempTDT) + (this->enthRev - (this->specHeatTransition * phaseChangeTempReverse));
                }
            } else if (prevPhaseChangeState == Phase::Transition && phaseChangeState == Phase::Transition) {
                if (updatedTempTDT < phaseChangeTempReverse) {
                    Tc = this->peakTempMelting;
                    Tau1 = this->deltaTempMeltingLow;
                    Tau2 = this->deltaTempMeltingHigh;
                } else if (updatedTempTDT > phaseChangeTempReverse) {
                    Tc = this->peakTempFreezing;
                    Tau1 = this->deltaTempFreezingLow;
                    Tau2 = this->deltaTempFreezingHigh;
                }
                this->enthRev = this->getEnthalpy(phaseChangeTempReverse, Tc, this->deltaTempMeltingLow, this->deltaTempMeltingHigh);
                this->enthNew = (this->specHeatTransition * updatedTempTDT) + (this->enthOld - (this->specHeatTransition * prevTempTD));
                this->enthalpyM = this->getEnthalpy(updatedTempTDT, this->peakTempMelting, this->deltaTempMeltingLow, this->deltaTempMeltingHigh);
                this->enthalpyF = this->getEnthalpy(updatedTempTDT, this->peakTempMelting, this->deltaTempMeltingLow, this->deltaTempMeltingHigh);
                if ((updatedTempTDT < phaseChangeTempReverse) && (this->enthNew > this->enthalpyF)) {
                    phaseChangeState = Phase::Freezing;
                    this->enthNew =
                        this->getEnthalpy(updatedTempTDT, this->peakTempFreezing, this->deltaTempFreezingLow, this->deltaTempFreezingHigh);
                } else if ((this->enthNew < this->enthalpyF) && (this->enthNew > this->enthalpyM) &&
                           (updatedTempTDT < prevTempTD || updatedTempTDT > prevTempTD)) {
                    phaseChangeState = Phase::Transition;
                    this->enthNew =
                        (this->specHeatTransition * updatedTempTDT) + (this->enthRev - (this->specHeatTransition * phaseChangeTempReverse));
                } else if (this->enthNew <= this->enthalpyM && updatedTempTDT >= prevTempTD && this->enthNew > this->enthOld) {
                    phaseChangeState = Phase::Melting;
                    this->enthNew =
                        (this->specHeatTransition * updatedTempTDT) + (this->enthRev - (this->specHeatTransition * phaseChangeTempReverse));
                }
            } else if (prevPhaseChangeState == Phase::Transition && phaseChangeState == Phase::Crystallized) {
                this->enthRev =
                    this->getEnthalpy(phaseChangeTempReverse, this->peakTempFreezing, this->deltaTempFreezingLow, this->deltaTempFreezingHigh);
                this->enthNew = (this->specHeatTransition * updatedTempTDT) + (this->enthRev - (this->specHeatTransition * phaseChangeTempReverse));
                this->enthalpyM = this->getEnthalpy(updatedTempTDT, this->peakTempMelting, this->deltaTempMeltingLow, this->deltaTempMeltingHigh);
                this->enthalpyF = this->getEnthalpy(updatedTempTDT, this->peakTempFreezing, this->deltaTempFreezingLow, this->deltaTempFreezingHigh);
                if ((this->enthNew < this->enthalpyF) && (this->enthNew > this->enthalpyM)) {
                    phaseChangeState = Phase::Transition;
                    this->enthNew =
                        (this->specHeatTransition * updatedTempTDT) + (this->enthRev - (this->specHeatTransition * phaseChangeTempReverse));
                } else if (this->enthNew <= this->enthalpyM && updatedTempTDT >= prevTempTD) {
                    phaseChangeState = Phase::Melting;
                    this->enthNew = this->getEnthalpy(updatedTempTDT, this->peakTempMelting, this->deltaTempMeltingLow, this->deltaTempMeltingHigh);
                }
            } else if (prevPhaseChangeState == Phase::Melting && phaseChangeState == Phase::Transition) {
                this->enthNew = (this->specHeatTransition * updatedTempTDT) + (this->enthOld - (this->specHeatTransition * prevTempTD));
                this->enthalpyM = this->getEnthalpy(updatedTempTDT, this->peakTempMelting, this->deltaTempMeltingLow, this->deltaTempMeltingHigh);
                this->enthalpyF = this->getEnthalpy(updatedTempTDT, this->peakTempFreezing, this->deltaTempFreezingLow, this->deltaTempFreezingHigh);
                if ((this->enthNew < this->enthOld) && (updatedTempTDT < prevTempTD)) {
                    phaseChangeState = Phase::Transition;
                    this->enthNew = (this->specHeatTransition * updatedTempTDT) + (this->enthOld - (this->specHeatTransition * prevTempTD));
                } else if ((this->enthNew < this->enthalpyF) && (this->enthNew > this->enthalpyM) && (updatedTempTDT < prevTempTD)) {
                    phaseChangeState = Phase::Transition;
                    this->enthNew =
                        (this->specHeatTransition * updatedTempTDT) + (this->enthRev - (this->specHeatTransition * phaseChangeTempReverse));
                } else if ((this->enthNew >= this->enthalpyF) && (updatedTempTDT <= phaseChangeTempReverse)) {
                    phaseChangeState = Phase::Transition;
                    this->enthNew =
                        (this->specHeatTransition * updatedTempTDT) + (this->enthRev - (this->specHeatTransition * phaseChangeTempReverse));
                }
            } else if (prevPhaseChangeState == Phase::Transition && phaseChangeState == Phase::Freezing) {
                this->enthalpyM = this->getEnthalpy(updatedTempTDT, this->peakTempMelting, this->deltaTempMeltingLow, this->deltaTempMeltingHigh);
                this->enthalpyF = this->getEnthalpy(updatedTempTDT, this->peakTempFreezing, this->deltaTempFreezingLow, this->deltaTempFreezingHigh);
                this->enthRev =
                    this->getEnthalpy(phaseChangeTempReverse, this->peakTempFreezing, this->deltaTempFreezingLow, this->deltaTempFreezingHigh);
                this->enthNew = (this->specHeatTransition * updatedTempTDT) + (this->enthRev - (this->specHeatTransition * phaseChangeTempReverse));
            }
        }

        // then calculate the specific heat and return it
        if (!this->phaseChangeTransition) {
            if (this->enthNew == this->enthOld) {
                Cp = this->CpOld;
            } else {
                Cp = this->specHeat(prevTempTD, updatedTempTDT, Tc, Tau1, Tau2, this->enthOld, this->enthNew);
            }
        } else {
            Cp = this->specHeatTransition;
        }
        this->CpOld = Cp;
        return Cp;
    }

    Real64 MaterialPhaseChange::specHeat(Real64 temperaturePrev,
                                         Real64 temperatureCurrent,
                                         Real64 criticalTemperature,
                                         Real64 tau1,
                                         Real64 tau2,
                                         Real64 EnthalpyOld,
                                         Real64 EnthalpyNew) const
    {

        //    Tc                  ! Critical (Melting/Freezing) Temperature of PCM
        //    Tau1                ! Width of Melting Zone low
        //    Tau2                ! Width of Melting Zone high
        //    EnthalpyOld         ! Previous Timestep Nodal Enthalpy
        //    EnthalpyNew         ! Current Timestep Nodal Enthalpy

        Real64 T = temperatureCurrent;

        if (T < criticalTemperature) {
            Real64 DEta1 = -(this->totalLatentHeat * (T - criticalTemperature) * exp(-2 * std::abs(T - criticalTemperature) / tau1)) /
                           (tau1 * std::abs(T - criticalTemperature));
            Real64 Cp1 = this->specificHeatSolid;
            return (Cp1 + DEta1);
        }
        if (T == criticalTemperature) {
            return (EnthalpyNew - EnthalpyOld) / (temperatureCurrent - temperaturePrev);
        }
        Real64 DEta2 = (this->totalLatentHeat * (T - criticalTemperature) * exp(-2 * std::abs(T - criticalTemperature) / tau2)) /
                       (tau2 * std::abs(T - criticalTemperature));
        Real64 Cp2 = this->specificHeatLiquid;
        return Cp2 + DEta2;
    }

    Real64 MaterialPhaseChange::getConductivity(Real64 T) const
    {
        if (T < this->peakTempMelting) {
            return this->fullySolidThermalConductivity;
        }
        if (T > this->peakTempFreezing) {
            return this->fullyLiquidThermalConductivity;
        }
        return (this->fullySolidThermalConductivity + this->fullyLiquidThermalConductivity) / 2.0;
    }

    Real64 MaterialPhaseChange::getDensity(Real64 T) const
    {
        if (T < this->peakTempMelting) {
            return this->fullySolidDensity;
        }
        if (T > this->peakTempFreezing) {
            return this->fullyLiquidDensity;
        }
        return (this->fullySolidDensity + this->fullyLiquidDensity) / 2.0;
    }

    void GetHysteresisData(EnergyPlusData &state, bool &ErrorsFound)
    {
        static constexpr std::string_view routineName = "GetHysteresisData";

        auto &s_ipsc = state.dataIPShortCut;
        auto &s_ip = state.dataInputProcessing->inputProcessor;
        auto &s_mat = state.dataMaterial;

        // convenience variables
        s_ipsc->cCurrentModuleObject = "MaterialProperty:PhaseChangeHysteresis";
        int numPhaseChangeModels = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        // loop over all hysteresis input instances, if zero, this will simply not do anything
        for (int hmNum = 1; hmNum <= numPhaseChangeModels; ++hmNum) {

            // just a few vars to pass in and out to GetObjectItem
            int ioStatus;
            int numAlphas;
            int numNumbers;

            // get the input data and store it in the Shortcuts structures
            s_ip->getObjectItem(state,
                                s_ipsc->cCurrentModuleObject,
                                hmNum,
                                s_ipsc->cAlphaArgs,
                                numAlphas,
                                s_ipsc->rNumericArgs,
                                numNumbers,
                                ioStatus,
                                s_ipsc->lNumericFieldBlanks,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};
            // the input processor validates the numeric inputs based on the IDD definition
            // still validate the name to make sure there aren't any duplicates or blanks
            // blanks are easy: fatal if blank

            if (s_ipsc->lAlphaFieldBlanks(1)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(1), s_ipsc->cAlphaArgs(1));
                ErrorsFound = true;
                continue;
            }

            int matNum = GetMaterialNum(state, s_ipsc->cAlphaArgs(1));
            if (matNum == 0) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(1), s_ipsc->cAlphaArgs(1));
                ErrorsFound = true;
                continue;
            }

            auto *mat = s_mat->materials(matNum);
            if (mat->group != Group::Regular) {
                ShowSevereCustom(state, eoh, format("Material {} is not a Regular material.", mat->Name));
                ErrorsFound = true;
                continue;
            }

            if (mat->hasPCM) {
                ShowSevereCustom(state, eoh, format("Material {} already has {} properties defined.", mat->Name, s_ipsc->cCurrentModuleObject));
                ErrorsFound = true;
                continue;
            }

            if (mat->hasEMPD) {
                ShowSevereCustom(state, eoh, format("Material {} already has EMPD properties defined.", mat->Name));
                ErrorsFound = true;
                continue;
            }

            if (mat->hasHAMT) {
                ShowSevereCustom(state, eoh, format("Material {} already has HAMT properties defined.", mat->Name));
                ErrorsFound = true;
                continue;
            }

            // Need to upgrade this object to MaterialPhaseChange
            auto *matPC = new MaterialPhaseChange;
            matPC->MaterialBase::operator=(*mat); // Deep copy the parent object

            delete mat;
            s_mat->materials(matNum) = matPC;

            // now build out a new hysteresis instance and add it to the vector
            matPC->totalLatentHeat = s_ipsc->rNumericArgs(1);
            matPC->fullyLiquidThermalConductivity = s_ipsc->rNumericArgs(2);
            matPC->fullyLiquidDensity = s_ipsc->rNumericArgs(3);
            matPC->specificHeatLiquid = s_ipsc->rNumericArgs(4);
            matPC->deltaTempMeltingHigh = s_ipsc->rNumericArgs(5);
            matPC->peakTempMelting = s_ipsc->rNumericArgs(6);
            matPC->deltaTempMeltingLow = s_ipsc->rNumericArgs(7);
            matPC->fullySolidThermalConductivity = s_ipsc->rNumericArgs(8);
            matPC->fullySolidDensity = s_ipsc->rNumericArgs(9);
            matPC->specificHeatSolid = s_ipsc->rNumericArgs(10);
            matPC->deltaTempFreezingHigh = s_ipsc->rNumericArgs(11);
            matPC->peakTempFreezing = s_ipsc->rNumericArgs(12);
            matPC->deltaTempFreezingLow = s_ipsc->rNumericArgs(13);
            matPC->specHeatTransition = (matPC->specificHeatSolid + matPC->specificHeatLiquid) / 2.0;
            matPC->CpOld = matPC->specificHeatSolid;
            matPC->hasPCM = true;
        }
    }

} // namespace Material

} // namespace EnergyPlus
