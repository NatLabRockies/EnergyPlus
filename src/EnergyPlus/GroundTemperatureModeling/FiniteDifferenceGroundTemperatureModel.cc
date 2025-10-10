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
#include <algorithm>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataReportingFlags.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GroundTemperatureModeling/FiniteDifferenceGroundTemperatureModel.hh>
#include <EnergyPlus/GroundTemperatureModeling/KusudaAchenbachGroundTemperatureModel.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

namespace GroundTemp {
    //******************************************************************************

    // Finite difference model factory
    FiniteDiffGroundTempsModel *FiniteDiffGroundTempsModel::FiniteDiffGTMFactory(EnergyPlusData &state, const std::string &objectName)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Read input and creates instance of finite difference ground temp model

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool found = false;
        int NumNums;
        int NumAlphas;
        int IOStat;

        // New shared pointer for this model object
        auto *thisModel = new FiniteDiffGroundTempsModel();

        GroundTemp::ModelType modelType = GroundTemp::ModelType::FiniteDiff;

        // Search through finite diff models here
        std::string_view const cCurrentModuleObject = GroundTemp::modelTypeNamesUC[(int)modelType];
        const int numCurrModels = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        for (int modelNum = 1; modelNum <= numCurrModels; ++modelNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     modelNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNums,
                                                                     IOStat);

            if (objectName == state.dataIPShortCut->cAlphaArgs(1)) {
                // Read input into object here

                thisModel->modelType = modelType;
                thisModel->Name = state.dataIPShortCut->cAlphaArgs(1);
                thisModel->baseConductivity = state.dataIPShortCut->rNumericArgs(1);
                thisModel->baseDensity = state.dataIPShortCut->rNumericArgs(2);
                thisModel->baseSpecificHeat = state.dataIPShortCut->rNumericArgs(3);
                thisModel->waterContent = state.dataIPShortCut->rNumericArgs(4) / 100.0;
                thisModel->saturatedWaterContent = state.dataIPShortCut->rNumericArgs(5) / 100.0;
                thisModel->evapotransCoeff = state.dataIPShortCut->rNumericArgs(6);

                found = true;
                break;
            }
        }

        if (found) {
            state.dataGrndTempModelMgr->groundTempModels.push_back(thisModel);

            // Simulate
            thisModel->initAndSim(state);

            // Return the pointer
            return thisModel;
        }

        ShowFatalError(state, fmt::format("{}--Errors getting input for ground temperature model", GroundTemp::modelTypeNames[(int)modelType]));
        return nullptr;
    }

    //******************************************************************************

    void FiniteDiffGroundTempsModel::initAndSim(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Initializes and simulates finite difference ground temps model

        getWeatherData(state);

        developMesh();

        performSimulation(state);
    }

    //******************************************************************************

    void FiniteDiffGroundTempsModel::getWeatherData(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Finds correct environment for reading all weather data. Loops over all weather data in weather file
        // and data structure containing daily average of required weather data.

        // SUBROUTINE ARGUMENT DEFINITIONS:
        // an environment is available to process

        // Save current environment so we can revert back when done
        const int Envrn_reset = state.dataWeather->Envrn;
        const Constant::KindOfSim KindOfSim_reset = state.dataGlobal->KindOfSim;
        const int TimeStep_reset = state.dataGlobal->TimeStep;
        const int HourOfDay_reset = state.dataGlobal->HourOfDay;
        const bool BeginEnvrnFlag_reset = state.dataGlobal->BeginEnvrnFlag;
        const bool EndEnvrnFlag_reset = state.dataGlobal->EndEnvrnFlag;
        const bool EndMonthFlag_reset = state.dataEnvrn->EndMonthFlag;
        const bool WarmupFlag_reset = state.dataGlobal->WarmupFlag;
        const int DayOfSim_reset = state.dataGlobal->DayOfSim;
        const std::string DayOfSimChr_reset = state.dataGlobal->DayOfSimChr;
        const int NumOfWarmupDays_reset = state.dataReportFlag->NumOfWarmupDays;
        const bool BeginDayFlag_reset = state.dataGlobal->BeginDayFlag;
        const bool EndDayFlag_reset = state.dataGlobal->EndDayFlag;
        const bool BeginHourFlag_reset = state.dataGlobal->BeginHourFlag;
        const bool EndHourFlag_reset = state.dataGlobal->EndHourFlag;

        if (!state.dataWeather->WeatherFileExists) {
            ShowSevereError(state,
                            "Site:GroundTemperature:Undisturbed:FiniteDifference -- using this model requires specification of a weather file.");
            ShowContinueError(
                state, "Either place in.epw in the working directory or specify a weather file on the command line using -w /path/to/weather.epw");
            ShowFatalError(state, "Simulation halted due to input error in ground temperature model.");
        }

        // We add a new period to force running all weather data
        int originalNumOfEnvrn = state.dataWeather->NumOfEnvrn;
        ++state.dataWeather->NumOfEnvrn;
        ++state.dataWeather->TotRunPers;
        state.dataWeather->Environment.redimension(state.dataWeather->NumOfEnvrn);
        state.dataWeather->RunPeriodInput.redimension(state.dataWeather->TotRunPers);
        state.dataWeather->Environment(state.dataWeather->NumOfEnvrn).KindOfEnvrn = Constant::KindOfSim::ReadAllWeatherData;
        state.dataWeather->RPReadAllWeatherData = true;
        state.dataGlobal->WeathSimReq = true;
        // RunPeriod is initialized to be one year of simulation
        // RunPeriodInput(TotRunPers).monWeekDay = 0; // Why do this?

        Weather::SetupEnvironmentTypes(state);

        // We reset the counter to the original number of run periods, so that GetNextEnvironment will fetch the one we added
        state.dataWeather->Envrn = originalNumOfEnvrn;
        bool Available = true;
        bool ErrorsFound = false;
        Weather::GetNextEnvironment(state, Available, ErrorsFound);
        if (ErrorsFound) {
            ShowFatalError(state, "Site:GroundTemperature:Undisturbed:FiniteDifference: error in reading weather file data");
        }

        if (state.dataGlobal->KindOfSim != Constant::KindOfSim::ReadAllWeatherData) {
            // This shouldn't happen
            ShowFatalError(state, "Site:GroundTemperature:Undisturbed:FiniteDifference: error in reading weather file data, bad KindOfSim.");
        }

        weatherDataArray.dimension(state.dataWeather->NumDaysInYear);

        state.dataGlobal->BeginEnvrnFlag = true;
        state.dataGlobal->EndEnvrnFlag = false;
        state.dataEnvrn->EndMonthFlag = false;
        state.dataGlobal->WarmupFlag = false;
        state.dataGlobal->DayOfSim = 0;
        state.dataGlobal->DayOfSimChr = "0";
        state.dataReportFlag->NumOfWarmupDays = 0;

        Real64 annualAveAirTemp_num = 0.0;

        while ((state.dataGlobal->DayOfSim < state.dataWeather->NumDaysInYear) || (state.dataGlobal->WarmupFlag)) { // Begin day loop ...

            ++state.dataGlobal->DayOfSim;

            // Reset daily values
            Real64 outDryBulbTemp_num = 0.0;
            Real64 relHum_num = 0.0;
            Real64 windSpeed_num = 0.0;
            Real64 horizSolarRad_num = 0.0;
            Real64 airDensity_num = 0.0;
            int denominator = 0;

            auto &tdwd = weatherDataArray(state.dataGlobal->DayOfSim); // "This day weather data"

            state.dataGlobal->BeginDayFlag = true;
            state.dataGlobal->EndDayFlag = false;

            for (state.dataGlobal->HourOfDay = 1; state.dataGlobal->HourOfDay <= 24; ++state.dataGlobal->HourOfDay) { // Begin hour loop ...

                state.dataGlobal->BeginHourFlag = true;
                state.dataGlobal->EndHourFlag = false;

                for (state.dataGlobal->TimeStep = 1; state.dataGlobal->TimeStep <= state.dataGlobal->TimeStepsInHour; ++state.dataGlobal->TimeStep) {

                    state.dataGlobal->BeginTimeStepFlag = true;

                    // Set the End__Flag variables to true if necessary.  Note that
                    // each flag builds on the previous level.  EndDayFlag cannot be
                    // .TRUE. unless EndHourFlag is also .TRUE., etc.  Note that the
                    // EndEnvrnFlag and the EndSimFlag cannot be set during warmup.
                    // Note also that BeginTimeStepFlag, EndTimeStepFlag, and the
                    // SubTimeStepFlags can/will be set/reset in the HVAC Manager.

                    if (state.dataGlobal->TimeStep == state.dataGlobal->TimeStepsInHour) {
                        state.dataGlobal->EndHourFlag = true;
                        if (state.dataGlobal->HourOfDay == 24) {
                            state.dataGlobal->EndDayFlag = true;
                            if (!state.dataGlobal->WarmupFlag && (state.dataGlobal->DayOfSim == state.dataGlobal->NumOfDayInEnvrn)) {
                                state.dataGlobal->EndEnvrnFlag = true;
                            }
                        }
                    }

                    Weather::ManageWeather(state);

                    outDryBulbTemp_num += state.dataEnvrn->OutDryBulbTemp;
                    airDensity_num += state.dataEnvrn->OutAirDensity;
                    relHum_num += state.dataEnvrn->OutRelHumValue;
                    windSpeed_num += state.dataEnvrn->WindSpeed;
                    horizSolarRad_num += max(state.dataEnvrn->SOLCOS(3), 0.0) * state.dataEnvrn->BeamSolarRad + state.dataEnvrn->DifSolarRad;

                    state.dataGlobal->BeginHourFlag = false;
                    state.dataGlobal->BeginDayFlag = false;
                    state.dataGlobal->BeginEnvrnFlag = false;
                    state.dataGlobal->BeginSimFlag = false;

                    ++denominator;

                } // TimeStep loop

                state.dataGlobal->PreviousHour = state.dataGlobal->HourOfDay;

            } // ... End hour loop.

            tdwd.dryBulbTemp = outDryBulbTemp_num / denominator;
            tdwd.relativeHumidity = relHum_num / denominator;
            tdwd.windSpeed = windSpeed_num / denominator;
            tdwd.horizontalRadiation = horizSolarRad_num / denominator;
            tdwd.airDensity = airDensity_num / denominator;

            // Log data for domain initialization using KA model
            annualAveAirTemp_num += tdwd.dryBulbTemp;

            if (tdwd.dryBulbTemp < minDailyAirTemp) {
                minDailyAirTemp = tdwd.dryBulbTemp;
                dayOfMinDailyAirTemp = state.dataGlobal->DayOfSim;
            }

            if (tdwd.dryBulbTemp > maxDailyAirTemp) {
                maxDailyAirTemp = tdwd.dryBulbTemp;
            }

        } // ... End day loop.

        annualAveAirTemp = annualAveAirTemp_num / state.dataWeather->NumDaysInYear; // Used for initializing domain

        // Reset Environment when done reading data
        --state.dataWeather->NumOfEnvrn; // May need better way of eliminating the extra environment that was added to read the data
        --state.dataWeather->TotRunPers;
        state.dataGlobal->KindOfSim = KindOfSim_reset;
        state.dataWeather->RPReadAllWeatherData = false;
        state.dataWeather->Environment.redimension(state.dataWeather->NumOfEnvrn);
        state.dataWeather->RunPeriodInput.redimension(state.dataWeather->TotRunPers);
        state.dataWeather->Envrn = Envrn_reset;
        state.dataGlobal->TimeStep = TimeStep_reset;
        state.dataGlobal->HourOfDay = HourOfDay_reset;
        state.dataGlobal->BeginEnvrnFlag = BeginEnvrnFlag_reset;
        state.dataGlobal->EndEnvrnFlag = EndEnvrnFlag_reset;
        state.dataEnvrn->EndMonthFlag = EndMonthFlag_reset;
        state.dataGlobal->WarmupFlag = WarmupFlag_reset;
        state.dataGlobal->DayOfSim = DayOfSim_reset;
        state.dataGlobal->DayOfSimChr = DayOfSimChr_reset;
        state.dataReportFlag->NumOfWarmupDays = NumOfWarmupDays_reset;
        state.dataGlobal->BeginDayFlag = BeginDayFlag_reset;
        state.dataGlobal->EndDayFlag = EndDayFlag_reset;
        state.dataGlobal->BeginHourFlag = BeginHourFlag_reset;
        state.dataGlobal->EndHourFlag = EndHourFlag_reset;
    }

    //******************************************************************************

    void FiniteDiffGroundTempsModel::developMesh()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Creates static mesh used for model

        // Surface layer parameters
        constexpr Real64 surfaceLayerThickness = 2.0;
        constexpr Real64 surfaceLayerCellThickness = 0.015;
        constexpr int surfaceLayerNumCells = static_cast<int>(surfaceLayerThickness / surfaceLayerCellThickness);

        // Center layer parameters
        constexpr int centerLayerNumCells = 80;

        // Deep layer parameters
        constexpr Real64 deepLayerThickness = 0.2;
        constexpr Real64 deepLayerCellThickness = surfaceLayerCellThickness;
        constexpr int deepLayerNumCells = static_cast<int>(deepLayerThickness / deepLayerCellThickness);

        // Other
        Real64 currentCellDepth = 0.0;

        totalNumCells = surfaceLayerNumCells + centerLayerNumCells + deepLayerNumCells;

        // Allocate arrays
        cellArray.allocate(totalNumCells);
        cellDepths.allocate(totalNumCells);

        for (int i = 1; i <= totalNumCells; ++i) {

            // Reference to thisCell
            auto &thisCell = cellArray(i);

            // Set the index
            thisCell.index = i;

            // Give thickness to the cells
            if (i <= surfaceLayerNumCells) {
                // Constant thickness mesh here
                thisCell.thickness = surfaceLayerCellThickness;

            } else if (i <= (centerLayerNumCells + surfaceLayerNumCells)) {
                // Geometric expansion/contraction here
                int numCenterCell = i - surfaceLayerNumCells;

                if (numCenterCell <= (centerLayerNumCells / 2)) {
                    Real64 centerLayerExpansionCoeff = 1.10879;
                    thisCell.thickness = surfaceLayerCellThickness * std::pow(centerLayerExpansionCoeff, numCenterCell);
                } else {
                    thisCell.thickness =
                        cellArray((surfaceLayerNumCells + (centerLayerNumCells / 2)) - (numCenterCell - (centerLayerNumCells / 2))).thickness;
                }
            } else {
                // Constant thickness mesh here
                thisCell.thickness = deepLayerCellThickness;
            }

            // Set minimum z value
            thisCell.minZValue = currentCellDepth;

            // Populate depth array for use later when looking up temperatures
            cellDepths(i) = currentCellDepth + thisCell.thickness / 2.0;

            // Update local counter
            currentCellDepth += thisCell.thickness;

            // Set maximum z value
            thisCell.maxZValue = currentCellDepth;

            // Set base properties
            thisCell.props.conductivity = baseConductivity;
            thisCell.props.density = baseDensity;
            thisCell.props.specificHeat = baseSpecificHeat;
            thisCell.props.diffusivity = baseConductivity / (baseDensity * baseSpecificHeat);
        }
    }

    //******************************************************************************

    void FiniteDiffGroundTempsModel::performSimulation(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Simulates model, repeating years, until steady-periodic temperatures are determined.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        timeStepInSeconds = Constant::rSecsInDay;
        bool convergedFinal = false;

        initDomain(state);

        // Loop until converged
        do {

            // loop over all days
            for (state.dataGlobal->FDsimDay = 1; state.dataGlobal->FDsimDay <= state.dataWeather->NumDaysInYear; ++state.dataGlobal->FDsimDay) {

                bool iterationConverged = false;

                doStartOfTimeStepInits();

                // Loop until iteration temperature converges
                do {

                    // For all cells
                    for (int cell = 1; cell <= totalNumCells; ++cell) {

                        if (cell == 1) {
                            updateSurfaceCellTemperature(state);
                        } else if (cell > 1 && cell < totalNumCells) {
                            updateGeneralDomainCellTemperature(cell);
                        } else if (cell == totalNumCells) {
                            updateBottomCellTemperature();
                        }
                    }

                    // Check iteration temperature convergence
                    iterationConverged = checkIterationTemperatureConvergence();

                    if (!iterationConverged) {
                        // Shift temperatures for next iteration
                        updateIterationTemperatures();
                    }

                } while (!iterationConverged);

                // Shift temperatures for next timestep
                updateTimeStepTemperatures(state);
            }

            // Check final temperature convergence
            convergedFinal = checkFinalTemperatureConvergence(state);

        } while (!convergedFinal);
    }

    //******************************************************************************

    void FiniteDiffGroundTempsModel::updateSurfaceCellTemperature(const EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Determines heat transfer to surface. Updates surface cell temperature.

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 numerator = 0.0;
        Real64 denominator = 0.0;
        Real64 resistance = 0.0;
        Real64 G_hr;
        Real64 Cd;

        Real64 constexpr rho_water = 998.0; // [kg/m3]
        // evapotranspiration parameters
        Real64 constexpr absor_Corrected = 0.77;
        constexpr Real64 convert_Wm2_To_MJhrmin = 3600.0 / 1000000.0;
        constexpr Real64 convert_MJhrmin_To_Wm2 = 1.0 / convert_Wm2_To_MJhrmin;

        const auto &thisCell = cellArray(1);
        const auto &cellBelow_thisCell = cellArray(2);
        const auto &cwd = weatherDataArray(state.dataGlobal->FDsimDay); // "Current Weather Day"

        // Add effect from previous time step
        numerator += thisCell.temperature_prevTimeStep;
        ++denominator;

        // Conduction to lower cell
        resistance = thisCell.thickness / 2.0 / (thisCell.props.conductivity * thisCell.conductionArea) +
                     cellBelow_thisCell.thickness / 2.0 / (cellBelow_thisCell.props.conductivity * cellBelow_thisCell.conductionArea);
        numerator += thisCell.beta / resistance * cellBelow_thisCell.temperature;
        denominator += thisCell.beta / resistance;

        // Convection to atmosphere
        if (cwd.windSpeed > 0.1) {
            Real64 constexpr airSpecificHeat = 1003; // '[J/kg-K]
            resistance = 208.0 / (cwd.airDensity * airSpecificHeat * cwd.windSpeed * thisCell.conductionArea);
        } else {
            // Future development should include additional natural convection effects here
        }
        numerator += thisCell.beta / resistance * cwd.dryBulbTemp;
        denominator += thisCell.beta / resistance;

        // For convenience convert to Kelvin once
        const Real64 currAirTempK = cwd.dryBulbTemp + 273.15;

        // Convert input solar radiation [w/m2] into units for ET model, [MJ/hr-min]
        // Diffuse + Direct Beam Radiation
        const Real64 incidentSolar_MJhrmin = cwd.horizontalRadiation * convert_Wm2_To_MJhrmin;

        // Absorbed solar radiation, [MJ/hr-min]
        const Real64 absorbedIncidentSolar_MJhrmin = absor_Corrected * incidentSolar_MJhrmin;

        // Calculate saturated vapor pressure, [kPa]
        const Real64 vaporPressureSaturated_kPa = 0.6108 * std::exp(17.27 * cwd.dryBulbTemp / (cwd.dryBulbTemp + 237.3));

        // Calculate actual vapor pressure, [kPa]
        const Real64 vaporPressureActual_kPa = vaporPressureSaturated_kPa * cwd.relativeHumidity;

        // Calculate another Q term, [MJ/m2-hr]
        const Real64 QRAD_NL = 2.042E-10 * pow_4(currAirTempK) * (0.34 - 0.14 * std::sqrt(vaporPressureActual_kPa));

        // Calculate another Q term, [MJ/hr]
        const Real64 netIncidentRadiation_MJhr = absorbedIncidentSolar_MJhrmin - QRAD_NL;

        // constant
        constexpr Real64 CN = 37.0;

        // Check whether there was sun
        if (netIncidentRadiation_MJhr < 0.0) {
            G_hr = 0.5 * netIncidentRadiation_MJhr;
            Cd = 0.96;
        } else {
            G_hr = 0.1 * netIncidentRadiation_MJhr;
            Cd = 0.24;
        }

        const Real64 slope_S = 2503.0 * std::exp(17.27 * cwd.dryBulbTemp / (cwd.dryBulbTemp + 237.3)) / pow_2(cwd.dryBulbTemp + 237.3);
        constexpr Real64 pressure = 98.0;
        constexpr Real64 psychrometricConstant = 0.665e-3 * pressure;

        // Evapotranspiration constant, [mm/hr]
        const Real64 evapotransFluidLoss_mmhr =
            (evapotransCoeff * slope_S * (netIncidentRadiation_MJhr - G_hr) +
             psychrometricConstant * (CN / currAirTempK) * cwd.windSpeed * (vaporPressureSaturated_kPa - vaporPressureActual_kPa)) /
            (slope_S + psychrometricConstant * (1 + Cd * cwd.windSpeed));

        // Convert units, [m/hr]
        const Real64 evapotransFluidLoss_mhr = evapotransFluidLoss_mmhr / 1000.0;

        // Calculate latent heat, [MJ/kg]
        // Full formulation is cubic: L(T) = -0.0000614342 * T**3 + 0.00158927 * T**2 - 2.36418 * T + 2500.79[5]
        // In: Cubic fit to Table 2.1,p.16, Textbook: R.R.Rogers & M.K. Yau, A Short Course in Cloud Physics, 3e,(1989), Pergamon press
        // But a linear relation should suffice;
        // note-for now using the previous time step temperature as an approximation to help ensure stability
        const Real64 latentHeatVaporization = 2.501 - 2.361e-3 * thisCell.temperature_prevTimeStep;

        // Calculate evapotranspiration heat loss, [MJ/m2-hr]
        const Real64 evapotransHeatLoss_MJhrmin = rho_water * evapotransFluidLoss_mhr * latentHeatVaporization;

        // Convert net incident solar units, [W/m2]
        const Real64 netIncidentRadiation_Wm2 = netIncidentRadiation_MJhr * convert_MJhrmin_To_Wm2;

        // Convert evapotranspiration units, [W/m2]
        const Real64 evapotransHeatLoss_Wm2 = evapotransHeatLoss_MJhrmin * convert_MJhrmin_To_Wm2;

        // Calculate overall net heat ?gain? into the cell, [W]
        const Real64 incidentHeatGain = (netIncidentRadiation_Wm2 - evapotransHeatLoss_Wm2) * thisCell.conductionArea;

        // Add any solar/evapotranspiration heat gain here
        numerator += thisCell.beta * incidentHeatGain;

        // Calculate the return temperature and leave
        cellArray(1).temperature = numerator / denominator;
    }

    //******************************************************************************

    void FiniteDiffGroundTempsModel::updateGeneralDomainCellTemperature(int const cell)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Update cell temperature based on HT from cells above and below

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 numerator = 0.0;
        Real64 denominator = 0.0;
        Real64 resistance = 0.0;

        auto &thisCell = cellArray(cell);
        const auto &cellAbove_thisCell = cellArray(cell - 1);
        const auto &cellBelow_thisCell = cellArray(cell + 1);

        // add effect from cell history
        numerator += thisCell.temperature_prevTimeStep;
        ++denominator;

        // Conduction resistance between this cell and above cell
        resistance = thisCell.thickness / 2.0 / (thisCell.conductionArea * thisCell.props.conductivity) +
                     cellAbove_thisCell.thickness / 2.0 / (cellAbove_thisCell.conductionArea * cellAbove_thisCell.props.conductivity);

        numerator += thisCell.beta / resistance * cellAbove_thisCell.temperature;
        denominator += thisCell.beta / resistance;

        // Conduction resistance between this cell and below cell
        resistance = thisCell.thickness / 2.0 / (thisCell.conductionArea * thisCell.props.conductivity) +
                     cellBelow_thisCell.thickness / 2.0 / (cellBelow_thisCell.conductionArea * cellBelow_thisCell.props.conductivity);

        numerator += thisCell.beta / resistance * cellBelow_thisCell.temperature;
        denominator += thisCell.beta / resistance;

        // now that we have passed all directions, update the temperature
        thisCell.temperature = numerator / denominator;
    }

    //******************************************************************************

    void FiniteDiffGroundTempsModel::updateBottomCellTemperature()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Updates bottom cell temperature based on earth heat flux HT from cell above

        // REFERENCES:
        // Fridleifsson, I.B., R. Bertani, E.Huenges, J.W. Lund, A. Ragnarsson, L. Rybach. 2008
        //  'The possible role and contribution of geothermal energy to the mitigation of climate change.'
        //  IPCC scoping meeting on renewable energy sources: 59-80.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 numerator = 0.0;
        Real64 denominator = 0.0;
        Real64 resistance = 0.0;
        Real64 constexpr geothermalGradient = 0.025; // C/m

        auto &thisCell = cellArray(totalNumCells);
        auto &cellAbove_thisCell = cellArray(totalNumCells - 1);

        // Initialize
        numerator += thisCell.temperature_prevTimeStep;
        ++denominator;

        // Conduction resistance between this cell and above cell
        resistance = ((thisCell.thickness / 2.0) / (thisCell.conductionArea * thisCell.props.conductivity)) +
                     ((cellAbove_thisCell.thickness / 2.0) / (cellAbove_thisCell.conductionArea * cellAbove_thisCell.props.conductivity));

        numerator += (thisCell.beta / resistance) * cellAbove_thisCell.temperature;
        denominator += thisCell.beta / resistance;

        // Geothermal gradient heat transfer
        Real64 HTBottom = geothermalGradient * thisCell.props.conductivity * thisCell.conductionArea;

        numerator += thisCell.beta * HTBottom;

        cellArray(totalNumCells).temperature = numerator / denominator;
    }

    //******************************************************************************

    bool FiniteDiffGroundTempsModel::checkFinalTemperatureConvergence(const EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Checks final temperature convergence

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool converged = true;
        Real64 constexpr finalTempConvergenceCriteria = 0.05;

        if (state.dataGlobal->FDnumIterYears == maxYearsToIterate) {
            return converged;
        }

        for (int cell = 1; cell <= totalNumCells; ++cell) {

            auto &thisCell = cellArray(cell);

            if (std::abs(thisCell.temperature - thisCell.temperature_finalConvergence) >= finalTempConvergenceCriteria) {
                converged = false;
            }

            thisCell.temperature_finalConvergence = thisCell.temperature;
        }

        ++state.dataGlobal->FDnumIterYears;

        return converged;
    }

    //******************************************************************************

    bool FiniteDiffGroundTempsModel::checkIterationTemperatureConvergence()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Checks iteration temperature convergence

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool converged = true;
        Real64 constexpr iterationTempConvergenceCriteria = 0.00001;

        for (int cell = 1; cell <= totalNumCells; ++cell) {

            if (std::abs(cellArray(cell).temperature - cellArray(cell).temperature_prevIteration) >= iterationTempConvergenceCriteria) {
                converged = false;
                break;
            }
        }

        return converged;
    }

    //******************************************************************************

    void FiniteDiffGroundTempsModel::initDomain(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Initializes model using Kusuda-Achenbach model.
        // Average ground temp initialized to average annual air temperature

        // Temporary KA model for initialization
        auto tempModel = std::make_unique<KusudaGroundTempsModel>(); // (AUTO_OK) Why does this have to be a unique_ptr?

        tempModel->Name = "KAModelForFDModel";
        tempModel->modelType = GroundTemp::ModelType::Kusuda;
        tempModel->aveGroundTemp = annualAveAirTemp;
        tempModel->aveGroundTempAmplitude =
            (maxDailyAirTemp - minDailyAirTemp) / 4.0; // Rough estimate here. Ground temps will not swing as far as the air temp.
        tempModel->phaseShiftInSecs = dayOfMinDailyAirTemp * Constant::rSecsInDay;
        tempModel->groundThermalDiffusivity = baseConductivity / (baseDensity * baseSpecificHeat);

        // Initialize temperatures and volume
        for (int cell = 1; cell <= totalNumCells; ++cell) {
            auto &thisCell = cellArray(cell);

            Real64 depth = (thisCell.maxZValue + thisCell.minZValue) / 2.0;

            // Initialize temperatures
            if (tempModel) {
                thisCell.temperature = tempModel->getGroundTempAtTimeInSeconds(state, depth, 0.0); // Initialized at first day of year
            }
            thisCell.temperature_finalConvergence = thisCell.temperature;
            thisCell.temperature_prevIteration = thisCell.temperature;
            thisCell.temperature_prevTimeStep = thisCell.temperature;

            // Set cell volume
            thisCell.volume = thisCell.thickness * thisCell.conductionArea;
        }

        // Initialize freezing calculation variables
        evaluateSoilRhoCpInit();

        // Initialize the groundTemps array
        groundTemps.dimension({1, state.dataWeather->NumDaysInYear}, {1, totalNumCells}, 0.0);

        tempModel.reset();
    }

    //******************************************************************************

    void FiniteDiffGroundTempsModel::updateIterationTemperatures()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Updates iteration temperatures for convergence checks

        for (int cell = 1; cell <= totalNumCells; ++cell) {
            cellArray(cell).temperature_prevIteration = cellArray(cell).temperature;
        }
    }

    //******************************************************************************

    void FiniteDiffGroundTempsModel::updateTimeStepTemperatures(const EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Updates timestep temperatures for convergence checks.

        for (int cell = 1; cell <= totalNumCells; ++cell) {

            auto &thisCell = cellArray(cell);

            thisCell.temperature_prevTimeStep = thisCell.temperature;

            // Log temps for later use
            groundTemps(state.dataGlobal->FDsimDay, cell) = thisCell.temperature;
        }
    }

    //******************************************************************************

    void FiniteDiffGroundTempsModel::doStartOfTimeStepInits()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Updates cell properties for each timestep

        for (int cell = 1; cell <= totalNumCells; ++cell) {

            auto &thisCell = cellArray(cell);

            evaluateSoilRhoCpCell(cell);

            thisCell.beta = (timeStepInSeconds / (thisCell.props.rhoCp * thisCell.volume));
        }
    }

    //******************************************************************************

    Real64 FiniteDiffGroundTempsModel::interpolate(Real64 const x, Real64 const x_hi, Real64 const x_low, Real64 const y_hi, Real64 const y_low)
    {
        return (x - x_low) / (x_hi - x_low) * (y_hi - y_low) + y_low;
    }

    //******************************************************************************

    Real64 FiniteDiffGroundTempsModel::getGroundTemp(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Interpolates between days and depths to find correct ground temperature

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // Interpolation variables
        int i0; // First day
        int i1; // Next day
        int j1; // Next cell index (with depth greater than y-depth
        // Real64 T_i0_j0; // Temp at int( x-day ); cell lower_bound( y-depth )
        // Real64 T_i1_j0; // Temp at int( x-day ) + 1; cell lower_bound( y-depth )
        // Real64 T_i0_j1; // Temp at int( x-day ); cell lower_bound( y-depth ) + 1
        // Real64 T_i1_j1; // Temp at int( x-day ) + 1; cell lower_bound( y-depth ) + 1
        // Real64 T_ix_j0; // Temp at x-day; cell lower_bound( y-depth )
        // Real64 T_ix_j1; // Temp at x-day; cell lower_bound( y-depth ) + 1
        // RETURNS: Final Temperature--Temp at x-day; y-depth

        if (depth < 0.0) {
            depth = 0.0;
        }

        // Get index of nearest cell with depth less than depth
        auto it = std::lower_bound(cellDepths.begin(), cellDepths.end(), depth);
        int j0 = static_cast<int>(std::distance(cellDepths.begin(), it)); // Cell index with depth less than y-depth

        // Compensate for 1-based array
        ++j0;

        // Fraction of day
        const Real64 dayFrac = simTimeInDays - static_cast<int>(simTimeInDays); // Fraction of day

        if (j0 < totalNumCells - 1) {
            // All depths within domain
            j1 = j0 + 1;

            if (simTimeInDays <= 1 || simTimeInDays >= state.dataWeather->NumDaysInYear) {
                // First day of year, last day of year, and leap day
                // Interpolate between first and last day
                i0 = state.dataWeather->NumDaysInYear;
                i1 = 1;

                // Lookup ground temps
                const Real64 T_i0_j0 = groundTemps(i0, j0);
                const Real64 T_i0_j1 = groundTemps(i0, j1);
                const Real64 T_i1_j0 = groundTemps(i1, j0);
                const Real64 T_i1_j1 = groundTemps(i1, j1);

                // Interpolate between days holding depth constant
                const Real64 T_ix_j0 = interpolate(dayFrac, 1, 0, T_i1_j0, T_i0_j0);
                const Real64 T_ix_j1 = interpolate(dayFrac, 1, 0, T_i1_j1, T_i0_j1);

                // Interpolate to correct depth now that we're at the right time
                return interpolate(depth, cellDepths(j1), cellDepths(j0), T_ix_j1, T_ix_j0);
            }

            // All other days
            i0 = static_cast<int>(simTimeInDays);
            i1 = i0 + 1;

            // Lookup ground temps
            const Real64 T_i0_j0 = groundTemps(i0, j0);
            const Real64 T_i0_j1 = groundTemps(i0, j1);
            const Real64 T_i1_j0 = groundTemps(i1, j0);
            const Real64 T_i1_j1 = groundTemps(i1, j1);

            // Interpolate between days holding depth constant
            const Real64 T_ix_j0 = interpolate(dayFrac, 1, 0, T_i1_j0, T_i0_j0);
            const Real64 T_ix_j1 = interpolate(dayFrac, 1, 0, T_i1_j1, T_i0_j1);

            // Interpolate to correct depth now that we're at the right time
            return interpolate(depth, cellDepths(j1), cellDepths(j0), T_ix_j1, T_ix_j0);
        }

        // Requesting a temperature deeper than domain. Pass deepest point in domain.
        j0 = totalNumCells;
        j1 = j0;

        if (simTimeInDays <= 1 || simTimeInDays >= state.dataWeather->NumDaysInYear) {
            // First day of year, last day of year, and leap day
            // Interpolate between first and last day
            i0 = state.dataWeather->NumDaysInYear;
            i1 = 1;

            // Lookup ground temps
            const Real64 T_i0_j1 = groundTemps(i0, j1);
            const Real64 T_i1_j1 = groundTemps(i1, j1);

            // Interpolate between days holding depth constant
            return interpolate(dayFrac, 1, 0, T_i1_j1, T_i0_j1);
        }

        // All other days
        i0 = static_cast<int>(simTimeInDays);
        i1 = i0 + 1;

        // Lookup ground temps
        const Real64 T_i0_j1 = groundTemps(i0, j1);
        const Real64 T_i1_j1 = groundTemps(i1, j1);

        // Interpolate between days holding depth constant
        return interpolate(dayFrac, 1, 0, T_i1_j1, T_i0_j1);
    }

    //******************************************************************************

    Real64 FiniteDiffGroundTempsModel::getGroundTempAtTimeInSeconds(EnergyPlusData &state, Real64 const _depth, Real64 const seconds)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell
        //       DATE WRITTEN   Summer 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Retrieves ground temperature when input time is in seconds

        depth = _depth;

        simTimeInDays = seconds / Constant::rSecsInDay;

        if (simTimeInDays > state.dataWeather->NumDaysInYear) {
            simTimeInDays = remainder(simTimeInDays, state.dataWeather->NumDaysInYear);
        }

        return getGroundTemp(state);
    }

    //******************************************************************************

    Real64 FiniteDiffGroundTempsModel::getGroundTempAtTimeInMonths(EnergyPlusData &state, Real64 const _depth, int const month)
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

        // Convert months to days. Puts time in middle of specified month
        simTimeInDays = aveDaysInMonth * ((month - 1) + 0.5);

        if (simTimeInDays > state.dataWeather->NumDaysInYear) {
            simTimeInDays = remainder(simTimeInDays, state.dataWeather->NumDaysInYear);
        }

        // Get and return ground temperature
        return getGroundTemp(state);
    }

    //******************************************************************************

    void FiniteDiffGroundTempsModel::evaluateSoilRhoCpCell(int const cell)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   Summer 2011

        // PURPOSE OF THIS SUBROUTINE:
        // Evaluates the soil properties on a single cell

        // Real64 rhoCP_soil;

        // These vary by domain now, so we must be careful to retrieve them every time

        auto &thisCell = cellArray(cell);

        // set some temperatures here for generalization -- these could be set in the input file
        // constexpr Real64 frzAllIce = -0.5;
        // constexpr Real64 frzIceTrans = -0.4;
        // constexpr Real64 frzLiqTrans = -0.1;
        // constexpr Real64 frzAllLiq = 0.0;

        // calculate this cell's new Cp value based on the cell temperature
        // if (thisCell.temperature >= frzAllLiq) {
        //     rhoCP_soil = rhoCp_soil_liq_1;
        // } else if (thisCell.temperature <= frzAllIce) {
        //     rhoCP_soil = rhoCP_soil_ice;
        // } else if (thisCell.temperature > frzLiqTrans) {
        //     rhoCP_soil = rhoCp_soil_liq_1 + (rhoCP_soil_transient - rhoCP_soil_liq) / (frzAllLiq - frzLiqTrans) * (frzAllLiq -
        //     thisCell.temperature);
        // } else if (thisCell.temperature >= frzIceTrans) {
        //     rhoCP_soil = rhoCP_soil_transient;
        // } else {
        //     rhoCP_soil = rhoCP_soil_ice + (rhoCP_soil_transient - rhoCP_soil_ice) / (frzIceTrans - frzAllIce) * (thisCell.temperature - frzAllIce);
        // }

        // TODO: The calculated rhoCP_soil is commented on this line and never used.  Curious.
        thisCell.props.rhoCp = baseDensity * baseSpecificHeat; // rhoCP_soil;

        thisCell.props.specificHeat = thisCell.props.rhoCp / thisCell.props.density;
    }

    void FiniteDiffGroundTempsModel::evaluateSoilRhoCpInit()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   Summer 2011

        // PURPOSE OF THIS SUBROUTINE:
        // Evaluates the soil properties

        // These vary by domain now, so we must be careful to retrieve them every time
        const Real64 Theta_liq = waterContent;
        const Real64 Theta_sat = saturatedWaterContent;

        // Assumption
        const Real64 Theta_ice = Theta_liq;

        //'Cp (freezing) calculations
        constexpr Real64 rho_ice = 917.0;                 //'Kg / m3
        constexpr Real64 rho_liq = 1000.0;                //'kg / m3
        rhoCp_soil_liq_1 = 1225000.0 / (1.0 - Theta_sat); // J/m3K
        // from( " An improved model for predicting soil thermal conductivity from water content at room temperature, Fig 4" )
        constexpr Real64 CP_liq = 4180.0;    //'J / KgK
        constexpr Real64 CP_ice = 2066.0;    //'J / KgK
        constexpr Real64 Lat_fus = 334000.0; //'J / Kg
        constexpr Real64 Cp_transient = Lat_fus / 0.4 + (0.5 * CP_ice - (CP_liq + CP_ice) / 2.0 * 0.1) / 0.4;
        // from( " Numerical and experimental investigation of melting and freezing processes in phase change material storage" )
        rhoCP_soil_liq = rhoCp_soil_liq_1 * (1.0 - Theta_sat) + rho_liq * CP_liq * Theta_liq;
        rhoCP_soil_transient = rhoCp_soil_liq_1 * (1.0 - Theta_sat) + ((rho_liq + rho_ice) / 2.0) * Cp_transient * Theta_ice;
        rhoCP_soil_ice = rhoCp_soil_liq_1 * (1.0 - Theta_sat) + rho_ice * CP_ice * Theta_ice; //'!J / m3K
    }

    //******************************************************************************

} // namespace GroundTemp
} // namespace EnergyPlus
