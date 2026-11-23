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
#include <array>
#include <format>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/GroundTemperatureModeling/KusudaAchenbachGroundTemperatureModel.hh>
#include <EnergyPlus/GroundTemperatureModeling/SiteShallowGroundTemperatures.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

//******************************************************************************
namespace GroundTemp {

    // Kusuda model factory
    KusudaGroundTempsModel *KusudaGroundTempsModel::KusudaGTMFactory(EnergyPlusData &state, const std::string &objectName)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Reads input and creates instance of Kusuda ground temps model

        // Locals
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool found = false;
        // New shared pointer for this model object
        auto *thisModel = new KusudaGroundTempsModel();

        // There was some **spooky** behavior here. One of the calling sites for this factory was passing in a reference
        // to a shared input buffer item as the objectName argument. Taking a local copy ensures the sought name persists.
        const std::string lookingForName = objectName; // NOLINT(*-unnecessary-copy-initialization)

        ModelType modelType = ModelType::Kusuda;

        std::string_view const cCurrentModuleObject = GroundTemp::modelTypeNames[(int)modelType];
        std::string const currentModuleObject(cCurrentModuleObject);
        auto *inputProcessor = state.dataInputProcessing->inputProcessor.get();
        auto const modelInstances = inputProcessor->epJSON.find(currentModuleObject);
        if (modelInstances == inputProcessor->epJSON.end()) {
            ShowFatalError(state, std::format("{}--Errors getting input for ground temperature model", GroundTemp::modelTypeNames[(int)modelType]));
        }
        auto const &modelSchemaProps = inputProcessor->getObjectSchemaProps(state, currentModuleObject);

        for (auto const &modelInstance : modelInstances.value().items()) {
            auto const modelName = Util::makeUPPER(modelInstance.key());
            auto const &modelFields = modelInstance.value();

            if (lookingForName == modelName) {
                inputProcessor->markObjectAsUsed(currentModuleObject, modelInstance.key());

                // Read input into object here
                thisModel->Name = modelName;
                thisModel->modelType = modelType;
                thisModel->groundThermalDiffusivity = inputProcessor->getRealFieldValue(modelFields, modelSchemaProps, "soil_thermal_conductivity") /
                                                      (inputProcessor->getRealFieldValue(modelFields, modelSchemaProps, "soil_density") *
                                                       inputProcessor->getRealFieldValue(modelFields, modelSchemaProps, "soil_specific_heat"));

                std::array<Real64, 3> flags = {
                    inputProcessor->getRealFieldValue(modelFields, modelSchemaProps, "average_soil_surface_temperature"),
                    inputProcessor->getRealFieldValue(modelFields, modelSchemaProps, "average_amplitude_of_surface_temperature"),
                    inputProcessor->getRealFieldValue(modelFields, modelSchemaProps, "phase_shift_of_minimum_surface_temperature")};
                const bool useGroundTempDataForKusuda =
                    std::any_of(flags.begin(), flags.end(), [](Real64 const flag) { return static_cast<bool>(flag); });

                if (useGroundTempDataForKusuda) {
                    // Use Kusuda Parameters
                    thisModel->aveGroundTemp = inputProcessor->getRealFieldValue(modelFields, modelSchemaProps, "average_soil_surface_temperature");
                    thisModel->aveGroundTempAmplitude =
                        inputProcessor->getRealFieldValue(modelFields, modelSchemaProps, "average_amplitude_of_surface_temperature");
                    thisModel->phaseShiftInSecs =
                        inputProcessor->getRealFieldValue(modelFields, modelSchemaProps, "phase_shift_of_minimum_surface_temperature") *
                        Constant::rSecsInDay;
                } else {
                    // Use data from Site:GroundTemperature:Shallow to generate parameters

                    constexpr int monthsInYear(12);
                    constexpr int avgDaysInMonth(30);
                    int monthOfMinSurfTemp(0);
                    Real64 averageGroundTemp(0);
                    Real64 amplitudeOfGroundTemp(0);
                    Real64 phaseShiftOfMinGroundTempDays(0);
                    Real64 minSurfTemp(100);  // Set high; month 1 temp will be lower than that and actually get updated
                    Real64 maxSurfTemp(-100); // Set low initially but will get updated

                    // get a non-owning pointer to the shallow ground temperature object, whether user-input or defaults
                    BaseGroundTempsModel *shallowObj = SiteShallowGroundTemps::ShallowGTMFactory(state, "");

                    for (int monthIndex = 1; monthIndex <= 12; ++monthIndex) {
                        const Real64 currMonthTemp = shallowObj->getGroundTempAtTimeInMonths(state, 0.0, monthIndex);

                        // Calculate Average Ground Temperature for all 12 months of the year:
                        averageGroundTemp += currMonthTemp;

                        // Need max temp, min temp, and month of min surf temp to set amplitude and month of min surf temp
                        if (currMonthTemp <= minSurfTemp) {
                            monthOfMinSurfTemp = monthIndex;
                            minSurfTemp = currMonthTemp;
                        }

                        if (currMonthTemp >= maxSurfTemp) {
                            maxSurfTemp = currMonthTemp;
                        }
                    }

                    averageGroundTemp /= monthsInYear;

                    amplitudeOfGroundTemp = (maxSurfTemp - minSurfTemp) / 2.0;

                    phaseShiftOfMinGroundTempDays = monthOfMinSurfTemp * avgDaysInMonth;

                    // Assign to KA Model
                    thisModel->aveGroundTemp = averageGroundTemp;
                    thisModel->aveGroundTempAmplitude = amplitudeOfGroundTemp;
                    thisModel->phaseShiftInSecs = phaseShiftOfMinGroundTempDays * Constant::rSecsInDay;
                }

                found = true;
                break;
            }
        }

        if (found) {
            state.dataGrndTempModelMgr->groundTempModels.push_back(thisModel);
            return thisModel;
        }

        ShowFatalError(state, std::format("{}--Errors getting input for ground temperature model", GroundTemp::modelTypeNames[(int)modelType]));
        return nullptr;
    }

    //******************************************************************************

    Real64 KusudaGroundTempsModel::getGroundTemp(EnergyPlusData &state)
    {
        // AUTHOR         Matt Mitchell
        // DATE WRITTEN   June 2015

        // PURPOSE OF THIS FUNCTION:
        // Returns a ground temperature

        // METHODOLOGY EMPLOYED:
        // Kusuda and Achenbach correlation is used

        Real64 const secsInYear = Constant::rSecsInDay * state.dataWeather->NumDaysInYear;

        const Real64 term1 = -depth * std::sqrt(Constant::Pi / (secsInYear * groundThermalDiffusivity));
        const Real64 term2 = (2 * Constant::Pi / secsInYear) *
                             (simTimeInSeconds - phaseShiftInSecs - (depth / 2) * std::sqrt(secsInYear / (Constant::Pi * groundThermalDiffusivity)));

        return aveGroundTemp - aveGroundTempAmplitude * std::exp(term1) * std::cos(term2);
    }

    //******************************************************************************

    Real64 KusudaGroundTempsModel::getGroundTempAtTimeInSeconds(EnergyPlusData &state, Real64 const _depth, Real64 const _seconds)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Returns the ground temperature when input time is in seconds

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 const secondsInYear = state.dataWeather->NumDaysInYear * Constant::rSecsInDay;

        depth = _depth;

        simTimeInSeconds = _seconds;

        if (simTimeInSeconds > secondsInYear) {
            simTimeInSeconds = remainder(simTimeInSeconds, secondsInYear);
        }

        // Get and return ground temperature
        return getGroundTemp(state);
    }

    //******************************************************************************

    Real64 KusudaGroundTempsModel::getGroundTempAtTimeInMonths(EnergyPlusData &state, Real64 const _depth, int const _month)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Returns the ground temperature when input time is in months

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 const aveSecondsInMonth = (state.dataWeather->NumDaysInYear / 12) * Constant::rSecsInDay;
        Real64 const secondsPerYear = state.dataWeather->NumDaysInYear * Constant::rSecsInDay;

        depth = _depth;

        simTimeInSeconds = aveSecondsInMonth * (_month - 1 + 0.5);

        if (simTimeInSeconds > secondsPerYear) {
            simTimeInSeconds = remainder(simTimeInSeconds, secondsPerYear);
        }

        // Get and return ground temperature
        return getGroundTemp(state);
    }

    //******************************************************************************

} // namespace GroundTemp
} // namespace EnergyPlus
