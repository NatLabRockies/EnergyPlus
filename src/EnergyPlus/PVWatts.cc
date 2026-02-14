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

// C++ Headers
#include <math.h>
#include <stdexcept>

// ObjexxFCL Headers

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PVWatts.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

// SAM Headers
// #include <../third_party/ssc/shared/lib_irradproc.h>
// #include <../third_party/ssc/shared/lib_pvwatts.h>
// #include <../third_party/ssc/shared/lib_pvshade.h>
// #include <../third_party/ssc/shared/lib_pv_incidence_modifier.h>
#include <../third_party/ssc/ssc/sscapi.h>

namespace EnergyPlus {

namespace PVWatts {

    PVWattsGenerator::PVWattsGenerator(EnergyPlusData &state,
                                       const std::string &name,
                                       const Real64 dcSystemCapacity,
                                       ModuleType moduleType,
                                       ArrayType arrayType,
                                       Real64 systemLosses,
                                       GeometryType geometryType,
                                       Real64 tilt,
                                       Real64 azimuth,
                                       size_t surfaceNum,
                                       Real64 groundCoverageRatio)
        : moduleType_(moduleType), arrayType_(arrayType), geometryType_(geometryType), DCtoACRatio_(1.1), inverterEfficiency_(0.96),
          outputDCPower_(1000.0), outputDCEnergy_(0.0), outputACPower_(0.0), outputACEnergy_(0.0), cellTemperature_(-9999),
          planeOfArrayIrradiance_(-9999), shadedPercent_(0.0), pvwattsModule_(ssc_module_create("pvwattsv5_1ts")), pvwattsData_(ssc_data_create()),
          NumTimeStepsToday_(0.0)

    {

        assert(pvwattsModule_ != nullptr);

        bool errorsFound(false);

        if (name.empty()) {
            ShowSevereError(state, "PVWatts: name cannot be blank.");
            errorsFound = true;
        }
        name_ = name;

        if (dcSystemCapacity <= 0) {
            ShowSevereError(state, "PVWatts: DC system capacity must be greater than zero.");
            errorsFound = true;
        }
        dcSystemCapacity_ = dcSystemCapacity;

        if (systemLosses > 1.0 || systemLosses < 0.0) {
            ShowSevereError(state, format("PVWatts: Invalid system loss value {:.2R}", systemLosses));
            errorsFound = true;
        }
        systemLosses_ = systemLosses;

        if (geometryType_ == GeometryType::TILT_AZIMUTH) {
            if (tilt < 0 || tilt > 90) {
                ShowSevereError(state, format("PVWatts: Invalid tilt: {:.2R}", tilt));
                errorsFound = true;
            }
            tilt_ = tilt;
            if (azimuth < 0 || azimuth >= 360) {
                ShowSevereError(state, format("PVWatts: Invalid azimuth: {:.2R}", azimuth));
            }
            azimuth_ = azimuth;
        } else if (geometryType_ == GeometryType::SURFACE) {
            if (surfaceNum == 0 || surfaceNum > state.dataSurface->Surface.size()) {
                ShowSevereError(state, format("PVWatts: SurfaceNum not in Surfaces: {}", surfaceNum));
                errorsFound = true;
            } else {
                surfaceNum_ = surfaceNum;
                tilt_ = getSurface(state).Tilt;
                azimuth_ = getSurface(state).Azimuth;
                // TODO: Do some bounds checking on Tilt and Azimuth.
            }
        } else {
            assert(false);
        }

        if (groundCoverageRatio > 1.0 || groundCoverageRatio < 0.0) {
            ShowSevereError(state, format("PVWatts: Invalid ground coverage ratio: {:.2R}", groundCoverageRatio));
            errorsFound = true;
        }
        groundCoverageRatio_ = groundCoverageRatio;

        if (errorsFound) {
            ShowFatalError(state, "Errors found in getting PVWatts input");
        }

        // Initialize m_pvwattsData
        // Location
        ssc_data_set_number(pvwattsData_, "lat", state.dataWeather->WeatherFileLatitude);
        ssc_data_set_number(pvwattsData_, "lon", state.dataWeather->WeatherFileLongitude);
        ssc_data_set_number(pvwattsData_, "tz", state.dataWeather->WeatherFileTimeZone);
        // System Properties
        ssc_data_set_number(pvwattsData_, "time_step", state.dataGlobal->TimeStepZone);
        ssc_data_set_number(pvwattsData_, "system_capacity", dcSystemCapacity_ * 0.001);
        ssc_data_set_number(pvwattsData_, "module_type", static_cast<int>(moduleType_));
        ssc_data_set_number(pvwattsData_, "dc_ac_ratio", DCtoACRatio_);
        ssc_data_set_number(pvwattsData_, "inv_eff", inverterEfficiency_ * 100.0);
        ssc_data_set_number(pvwattsData_, "losses", systemLosses_ * 100.0);
        ssc_data_set_number(pvwattsData_, "array_type", static_cast<int>(arrayType_));
        ssc_data_set_number(pvwattsData_, "tilt", tilt_);
        ssc_data_set_number(pvwattsData_, "azimuth", azimuth_);
        ssc_data_set_number(pvwattsData_, "gcr", groundCoverageRatio_);
        // Initialize shaded percent
        ssc_data_set_number(pvwattsData_, "shaded_percent", shadedPercent_);
    }

    void PVWattsGenerator::setupOutputVariables(EnergyPlusData &state)
    {
        // Set up output variables
        SetupOutputVariable(state,
                            "Generator Produced DC Electricity Rate",
                            Constant::Units::W,
                            outputDCPower_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Generator Produced DC Electricity Energy",
                            Constant::Units::J,
                            outputDCEnergy_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_,
                            Constant::eResource::ElectricityProduced,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::Photovoltaic);
        SetupOutputVariable(state,
                            "Generator PV Cell Temperature",
                            Constant::Units::C,
                            cellTemperature_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Plane of Array Irradiance",
                            Constant::Units::W_m2,
                            planeOfArrayIrradiance_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Shaded Percent",
                            Constant::Units::Perc,
                            shadedPercent_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
    }

    std::unique_ptr<PVWattsGenerator> PVWattsGenerator::createFromIdfObj(EnergyPlusData &state, int objNum)
    {
        Array1D_string cAlphaFieldNames;
        Array1D_string cNumericFieldNames;
        Array1D_bool lNumericFieldBlanks;
        Array1D_bool lAlphaFieldBlanks;
        Array1D_string cAlphaArgs;
        Array1D<Real64> rNumericArgs;
        constexpr int maxAlphas = 6;  // from idd
        constexpr int maxNumeric = 5; // from idd
        cAlphaFieldNames.allocate(maxAlphas);
        cNumericFieldNames.allocate(maxNumeric);
        lNumericFieldBlanks.allocate(maxNumeric);
        lAlphaFieldBlanks.allocate(maxAlphas);
        cAlphaArgs.allocate(maxAlphas);
        rNumericArgs.allocate(maxNumeric);
        int NumAlphas;
        int NumNums;
        int IOStat;
        bool errorsFound = false;

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 "Generator:PVWatts",
                                                                 objNum,
                                                                 cAlphaArgs,
                                                                 NumAlphas,
                                                                 rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericFieldBlanks,
                                                                 lAlphaFieldBlanks,
                                                                 cAlphaFieldNames,
                                                                 cNumericFieldNames);

        const std::string name(cAlphaArgs(AlphaFields::NAME));
        const Real64 dcSystemCapacity(rNumericArgs(NumFields::DC_SYSTEM_CAPACITY));
        const std::map<std::string, ModuleType> moduleTypeMap = {
            {"STANDARD", ModuleType::STANDARD}, {"PREMIUM", ModuleType::PREMIUM}, {"THINFILM", ModuleType::THIN_FILM}};
        ModuleType moduleType;
        auto moduleTypeIt = moduleTypeMap.find(cAlphaArgs(AlphaFields::MODULE_TYPE));
        if (moduleTypeIt == moduleTypeMap.end()) {
            ShowSevereError(state, format("PVWatts: Invalid Module Type: {}", cAlphaArgs(AlphaFields::MODULE_TYPE)));
            errorsFound = true;
        } else {
            moduleType = moduleTypeIt->second;
        }

        const std::map<std::string, ArrayType> arrayTypeMap = {{"FIXEDOPENRACK", ArrayType::FIXED_OPEN_RACK},
                                                               {"FIXEDROOFMOUNTED", ArrayType::FIXED_ROOF_MOUNTED},
                                                               {"ONEAXIS", ArrayType::ONE_AXIS},
                                                               {"ONEAXISBACKTRACKING", ArrayType::ONE_AXIS_BACKTRACKING},
                                                               {"TWOAXIS", ArrayType::TWO_AXIS}};
        ArrayType arrayType;
        auto arrayTypeIt = arrayTypeMap.find(cAlphaArgs(AlphaFields::ARRAY_TYPE));
        if (arrayTypeIt == arrayTypeMap.end()) {
            ShowSevereError(state, format("PVWatts: Invalid Array Type: {}", cAlphaArgs(AlphaFields::ARRAY_TYPE)));
            errorsFound = true;
        } else {
            arrayType = arrayTypeIt->second;
        }

        const Real64 systemLosses(rNumericArgs(NumFields::SYSTEM_LOSSES));
        const std::map<std::string, GeometryType> geometryTypeMap{{"TILTAZIMUTH", GeometryType::TILT_AZIMUTH}, {"SURFACE", GeometryType::SURFACE}};
        GeometryType geometryType;
        auto geometryTypeIt = geometryTypeMap.find(cAlphaArgs(AlphaFields::GEOMETRY_TYPE));
        if (geometryTypeIt == geometryTypeMap.end()) {
            ShowSevereError(state, format("PVWatts: Invalid Geometry Type: {}", cAlphaArgs(AlphaFields::GEOMETRY_TYPE)));
            errorsFound = true;
        } else {
            geometryType = geometryTypeIt->second;
        }

        const Real64 tilt(rNumericArgs(NumFields::TILT_ANGLE));
        const Real64 azimuth(rNumericArgs(NumFields::AZIMUTH_ANGLE));
        int surfaceNum;
        if (lAlphaFieldBlanks(AlphaFields::SURFACE_NAME)) {
            surfaceNum = 0;
        } else {
            surfaceNum = Util::FindItemInList(cAlphaArgs(AlphaFields::SURFACE_NAME), state.dataSurface->Surface);
        }

        if (errorsFound) {
            ShowFatalError(state, "Errors found in getting PVWatts input");
        }

        if (NumNums < NumFields::GROUND_COVERAGE_RATIO) {
            return std::make_unique<PVWattsGenerator>(
                state, name, dcSystemCapacity, moduleType, arrayType, systemLosses, geometryType, tilt, azimuth, surfaceNum, 0.4);
        }
        const Real64 groundCoverageRatio(rNumericArgs(NumFields::GROUND_COVERAGE_RATIO));

        return std::make_unique<PVWattsGenerator>(
            state, name, dcSystemCapacity, moduleType, arrayType, systemLosses, geometryType, tilt, azimuth, surfaceNum, groundCoverageRatio);
    }

    Real64 PVWattsGenerator::getDCSystemCapacity()
    {
        return dcSystemCapacity_;
    }

    ModuleType PVWattsGenerator::getModuleType()
    {
        return moduleType_;
    }

    ArrayType PVWattsGenerator::getArrayType()
    {
        return arrayType_;
    }

    Real64 PVWattsGenerator::getSystemLosses()
    {
        return systemLosses_;
    }

    GeometryType PVWattsGenerator::getGeometryType()
    {
        return geometryType_;
    }

    Real64 PVWattsGenerator::getTilt()
    {
        return tilt_;
    }

    Real64 PVWattsGenerator::getAzimuth()
    {
        return azimuth_;
    }

    DataSurfaces::SurfaceData &PVWattsGenerator::getSurface(EnergyPlusData &state)
    {
        return state.dataSurface->Surface(surfaceNum_);
    }

    Real64 PVWattsGenerator::getGroundCoverageRatio()
    {
        return groundCoverageRatio_;
    }

    Real64 PVWattsGenerator::getCellTemperature()
    {
        return cellTemperature_;
    }

    void PVWattsGenerator::setCellTemperature(Real64 cellTemp)
    {
        cellTemperature_ = cellTemp;
    }

    Real64 PVWattsGenerator::getPlaneOfArrayIrradiance()
    {
        return planeOfArrayIrradiance_;
    }

    void PVWattsGenerator::setPlaneOfArrayIrradiance(Real64 poa)
    {
        planeOfArrayIrradiance_ = poa;
    }

    void PVWattsGenerator::setDCtoACRatio(Real64 const dc2ac)
    {
        DCtoACRatio_ = dc2ac;
    }

    void PVWattsGenerator::setInverterEfficiency(Real64 const inverterEfficiency)
    {
        inverterEfficiency_ = inverterEfficiency;
    }

    void PVWattsGenerator::calc(EnergyPlusData &state)
    {
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        // We only run this once for each zone time step.
        const int NumTimeStepsToday_loc = state.dataGlobal->HourOfDay * state.dataGlobal->TimeStepsInHour + state.dataGlobal->TimeStep;
        if (NumTimeStepsToday_ != NumTimeStepsToday_loc) {
            NumTimeStepsToday_ = NumTimeStepsToday_loc;
        } else {
            outputDCEnergy_ = outputDCPower_ * TimeStepSysSec;
            outputACEnergy_ = outputACPower_ * TimeStepSysSec;
            return;
        }
        // SSC Inputs
        // Time
        ssc_data_set_number(pvwattsData_, "year", state.dataEnvrn->Year);
        ssc_data_set_number(pvwattsData_, "month", state.dataEnvrn->Month);
        ssc_data_set_number(pvwattsData_, "day", state.dataEnvrn->DayOfMonth);
        ssc_data_set_number(pvwattsData_, "hour", state.dataGlobal->HourOfDay - 1);
        ssc_data_set_number(pvwattsData_, "minute", (state.dataGlobal->TimeStep - 0.5) * state.dataGlobal->MinutesInTimeStep);

        // Weather Conditions
        ssc_data_set_number(pvwattsData_, "beam", state.dataEnvrn->BeamSolarRad);
        ssc_data_set_number(pvwattsData_, "diffuse", state.dataEnvrn->DifSolarRad);
        ssc_data_set_number(pvwattsData_, "tamb", state.dataEnvrn->OutDryBulbTemp);
        ssc_data_set_number(pvwattsData_, "wspd", state.dataEnvrn->WindSpeed);
        Real64 albedo = state.dataWeather->wvarsHrTsToday(state.dataGlobal->TimeStep, state.dataGlobal->HourOfDay).Albedo;
        if (!(std::isfinite(albedo) && albedo > 0.0 && albedo < 1)) {
            albedo = 0.2;
        }
        ssc_data_set_number(pvwattsData_, "alb", albedo);

        // In/Out Properties
        ssc_data_set_number(pvwattsData_, "tcell", cellTemperature_);
        ssc_data_set_number(pvwattsData_, "poa", planeOfArrayIrradiance_);

        // Get the shading from the geometry, if applicable
        if (geometryType_ == GeometryType::SURFACE) {
            shadedPercent_ = (1.0 - state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, surfaceNum_)) * 100.0;
            ssc_data_set_number(pvwattsData_, "shaded_percent", shadedPercent_);
        }

        if (ssc_module_exec(pvwattsModule_, pvwattsData_) == 0) {
            // Error
            const char *errtext;
            int sscErrType;
            float time;
            int i = 0;
            while ((errtext = ssc_module_log(pvwattsModule_, i++, &sscErrType, &time)) != nullptr) {
                std::string err("PVWatts: ");
                switch (sscErrType) {
                case SSC_WARNING:
                    err.append(errtext);
                    ShowWarningMessage(state, err);
                    break;
                case SSC_ERROR:
                    err.append(errtext);
                    ShowErrorMessage(state, err);
                    break;
                default:
                    break;
                }
            }
        } else {
            // Report Out
            ssc_data_get_number(pvwattsData_, "dc", &outputDCPower_);
            outputDCEnergy_ = outputDCPower_ * TimeStepSysSec;
            ssc_data_get_number(pvwattsData_, "ac", &outputACPower_);
            outputACEnergy_ = outputACPower_ * TimeStepSysSec;
            ssc_data_get_number(pvwattsData_, "tcell", &cellTemperature_);
            ssc_data_get_number(pvwattsData_, "poa", &planeOfArrayIrradiance_);
        }
    }

    void PVWattsGenerator::getResults(Real64 &GeneratorPower, Real64 &GeneratorEnergy, Real64 &ThermalPower, Real64 &ThermalEnergy)
    {
        GeneratorPower = outputDCPower_;
        GeneratorEnergy = outputDCEnergy_;
        ThermalPower = 0.0;
        ThermalEnergy = 0.0;
    }

} // namespace PVWatts

} // namespace EnergyPlus
