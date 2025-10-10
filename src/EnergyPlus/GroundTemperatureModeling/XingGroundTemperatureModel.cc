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

// C++ Headers
#include <memory>

// EnergyPlus headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/GroundTemperatureModeling/XingGroundTemperatureModel.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

//******************************************************************************
namespace GroundTemp {
    // Xing model factory
    XingGroundTempsModel *XingGroundTempsModel::XingGTMFactory(EnergyPlusData &state, const std::string &objectName)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Reads input and creates instance of Xing ground temps model

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool found = false;
        int NumNums;
        int NumAlphas;
        int IOStat;

        // New shared pointer for this model object
        auto *thisModel = new XingGroundTempsModel();

        ModelType modelType = ModelType::Xing;

        std::string_view const cCurrentModuleObject = GroundTemp::modelTypeNamesUC[(int)modelType];
        const int numCurrModels = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        thisModel->modelType = modelType;
        thisModel->Name = objectName;

        for (int modelNum = 1; modelNum <= numCurrModels; ++modelNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     modelNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNums,
                                                                     IOStat);

            if (thisModel->Name == state.dataIPShortCut->cAlphaArgs(1)) {
                // Read remaining input into object here
                thisModel->groundThermalDiffusivity = state.dataIPShortCut->rNumericArgs(1) /
                                                      (state.dataIPShortCut->rNumericArgs(2) * state.dataIPShortCut->rNumericArgs(3)) *
                                                      Constant::rSecsInDay;
                thisModel->aveGroundTemp = state.dataIPShortCut->rNumericArgs(4);
                thisModel->surfTempAmplitude_1 = state.dataIPShortCut->rNumericArgs(5);
                thisModel->surfTempAmplitude_2 = state.dataIPShortCut->rNumericArgs(6);
                thisModel->phaseShift_1 = state.dataIPShortCut->rNumericArgs(7);
                thisModel->phaseShift_2 = state.dataIPShortCut->rNumericArgs(8);

                found = true;
                break;
            }
        }

        if (found) {
            state.dataGrndTempModelMgr->groundTempModels.push_back(thisModel);
            return thisModel;
        }

        ShowFatalError(state, fmt::format("{}--Errors getting input for ground temperature model", GroundTemp::modelTypeNames[(int)modelType]));
        return nullptr;
    }

    //******************************************************************************

    Real64 XingGroundTempsModel::getGroundTemp(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Returns the ground temperature for the Site:GroundTemperature:Undisturbed:Xing

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        const Real64 tp = state.dataWeather->NumDaysInYear; // Period of soil temperature cycle

        // Inits
        const Real64 Ts_1 = surfTempAmplitude_1; // Amplitude of surface temperature
        const Real64 PL_1 = phaseShift_1;        // Phase shift of surface temperature
        const Real64 Ts_2 = surfTempAmplitude_2; // Amplitude of surface temperature
        const Real64 PL_2 = phaseShift_2;        // Phase shift of surface temperature

        constexpr int n1 = 1;
        const Real64 gamma1 = std::sqrt((n1 * Constant::Pi) / (groundThermalDiffusivity * tp));
        const Real64 exp1 = -depth * gamma1;
        const Real64 cos1 = 2 * Constant::Pi * n1 / tp * (simTimeInDays - PL_1) - depth * gamma1;

        constexpr int n2 = 2;
        const Real64 gamma2 = std::sqrt((n2 * Constant::Pi) / (groundThermalDiffusivity * tp));
        const Real64 exp2 = -depth * gamma2;
        const Real64 cos2 = 2 * Constant::Pi * n2 / tp * (simTimeInDays - PL_2) - depth * gamma2;

        const Real64 summation = std::exp(exp1) * Ts_1 * std::cos(cos1) + std::exp(exp2) * Ts_2 * std::cos(cos2);

        return aveGroundTemp - summation;
    }

    //******************************************************************************

    Real64 XingGroundTempsModel::getGroundTempAtTimeInMonths(EnergyPlusData &state, const Real64 _depth, const int _month)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Returns ground temperature when input time is in months

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // TODO: Fixing this to be floating point 12.0 causes diffs and failed tests
        Real64 const aveDaysInMonth = state.dataWeather->NumDaysInYear / 12;

        depth = _depth;

        // Set month
        if (_month >= 1 && _month <= 12) {
            simTimeInDays = aveDaysInMonth * (_month - 1 + 0.5);
        } else {
            const int monthIndex = _month % 12;
            simTimeInDays = aveDaysInMonth * (monthIndex - 1 + 0.5);
        }

        // Get and return ground temp
        return getGroundTemp(state);
    }

    //******************************************************************************

    Real64 XingGroundTempsModel::getGroundTempAtTimeInSeconds(EnergyPlusData &state, const Real64 _depth, const Real64 seconds)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Returns ground temperature when time is in seconds

        depth = _depth;

        simTimeInDays = seconds / Constant::rSecsInDay;

        if (simTimeInDays > state.dataWeather->NumDaysInYear) {
            simTimeInDays = remainder(simTimeInDays, state.dataWeather->NumDaysInYear);
        }

        return getGroundTemp(state);
    }

    //******************************************************************************
} // namespace GroundTemp
} // namespace EnergyPlus
