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

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/GroundTemperatureModeling/SiteShallowGroundTemperatures.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

//******************************************************************************
namespace GroundTemp {
    // Site:GroundTemperature:Shallow factory
    SiteShallowGroundTemps *SiteShallowGroundTemps::ShallowGTMFactory(EnergyPlusData &state, const std::string &objectName)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Reads input and creates instance of Site:GroundDomain:Shallow object

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool errorsFound = false;

        // New shared pointer for this model object
        auto *thisModel = new SiteShallowGroundTemps();

        ModelType modelType = ModelType::SiteShallow;

        std::string_view const cCurrentModuleObject = GroundTemp::modelTypeNamesUC[(int)modelType];
        const int numCurrObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        thisModel->modelType = modelType;
        thisModel->Name = objectName;

        if (numCurrObjects == 1) {

            int NumNums;
            int NumAlphas;
            int IOStat;

            // Get the object names for each construction from the input processor
            state.dataInputProcessing->inputProcessor->getObjectItem(
                state, cCurrentModuleObject, 1, state.dataIPShortCut->cAlphaArgs, NumAlphas, state.dataIPShortCut->rNumericArgs, NumNums, IOStat);

            if (NumNums < 12) {
                ShowSevereError(state, fmt::format("{}: Less than 12 values entered.", GroundTemp::modelTypeNames[(int)modelType]));
                errorsFound = true;
            }

            // Assign the ground temps to the variable
            for (int i = 1; i <= 12; ++i) {
                thisModel->surfaceGroundTemps[i - 1] = state.dataIPShortCut->rNumericArgs(i);
            }

            state.dataEnvrn->GroundTempInputs[static_cast<int>(DataEnvironment::GroundTempType::Shallow)] = true;

        } else if (numCurrObjects > 1) {
            ShowSevereError(state, fmt::format("{}: Too many objects entered. Only one allowed.", GroundTemp::modelTypeNames[(int)modelType]));
            errorsFound = true;
        } else {
            std::fill(thisModel->surfaceGroundTemps.begin(), thisModel->surfaceGroundTemps.end(), 13.0);
        }

        // Write Final Ground Temp Information to the initialization output file
        write_ground_temps(state.files.eio, "Shallow", thisModel->surfaceGroundTemps);

        if (!errorsFound) {
            state.dataGrndTempModelMgr->groundTempModels.push_back(thisModel);
            return thisModel;
        }

        ShowFatalError(state, fmt::format("{}--Errors getting input for ground temperature model", GroundTemp::modelTypeNames[(int)modelType]));
        return nullptr;
    }

    //******************************************************************************

    Real64 SiteShallowGroundTemps::getGroundTemp([[maybe_unused]] EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Return the ground temperature from Site:GroundTemperature:Shallow

        return surfaceGroundTemps[timeOfSimInMonths - 1];
    }

    //******************************************************************************

    Real64 SiteShallowGroundTemps::getGroundTempAtTimeInSeconds(EnergyPlusData &state, [[maybe_unused]] Real64 const _depth, Real64 const _seconds)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Returns the ground temperature when input time is in seconds

        // USE STATEMENTS:
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 secPerMonth = state.dataWeather->NumDaysInYear * Constant::rSecsInDay / 12;

        // Convert secs to months
        int month = ceil(_seconds / secPerMonth);

        if (month >= 1 && month <= 12) {
            timeOfSimInMonths = month;
        } else {
            timeOfSimInMonths = month % 12;
        }

        // Get and return ground temp
        return getGroundTemp(state);
    }

    //******************************************************************************

    Real64 SiteShallowGroundTemps::getGroundTempAtTimeInMonths(EnergyPlusData &state, [[maybe_unused]] Real64 const _depth, int const _month)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Returns the ground temperature when input time is in months

        // Set month
        if (_month >= 1 && _month <= 12) {
            timeOfSimInMonths = _month;
        } else {
            timeOfSimInMonths = _month % 12;
        }

        // Get and return ground temp
        return getGroundTemp(state);
    }

    //******************************************************************************
} // namespace GroundTemp
} // namespace EnergyPlus
