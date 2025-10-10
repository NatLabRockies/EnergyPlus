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

// Kiva Headers
#include <libkiva/Errors.hpp>
#ifdef GROUND_PLOT
#    include <EnergyPlus/DataStringGlobals.hh>
#    include <libgroundplot/GroundPlot.hpp>
#endif

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/HeatBalanceKivaManager.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/ThermalComfort.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/Vectors.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::HeatBalanceKivaManager {

void kivaErrorCallback(const int messageType, const std::string message, void *contextPtr)
{
    if (!contextPtr) {
        throw FatalError(format("Unhandled Kiva Error: {}", message));
    }
    std::string fullMessage;
    std::pair<EnergyPlusData *, std::string> contextPair = *(std::pair<EnergyPlusData *, std::string> *)contextPtr;
    if (contextPair.second.size() > 0) {
        fullMessage = format("{}: {}", contextPair.second, message);
    } else {
        fullMessage = format("Kiva: {}", message);
    }
    if (messageType == Kiva::MSG_INFO) {
        ShowMessage(*contextPair.first, fullMessage);
    } else if (messageType == Kiva::MSG_WARN) {
        ShowWarningError(*contextPair.first, fullMessage);
    } else { // if (messageType == Kiva::MSG_ERR)
        ShowSevereError(*contextPair.first, fullMessage);
        ShowFatalError(*contextPair.first, "Kiva: Errors discovered, program terminates.");
    }
}

KivaInstanceMap::KivaInstanceMap(EnergyPlusData &state,
                                 Kiva::Foundation &foundation,
                                 int floorSurface,
                                 std::vector<int> wallSurfaces,
                                 int zoneNum,
                                 Real64 zoneAssumedTemperature,
                                 Real64 floorWeight,
                                 int constructionNum,
                                 KivaManager *kmPtr)
    : instance(foundation), floorSurface(floorSurface), wallSurfaces(wallSurfaces), zoneNum(zoneNum), zoneControlType(KIVAZONE_UNCONTROLLED),
      zoneControlNum(0), zoneAssumedTemperature(zoneAssumedTemperature), floorWeight(floorWeight), constructionNum(constructionNum), kmPtr(kmPtr)
{

    for (int i = 1; i <= state.dataZoneCtrls->NumTempControlledZones; ++i) {
        if (state.dataZoneCtrls->TempControlledZone(i).ActualZoneNum == zoneNum) {
            zoneControlType = KIVAZONE_TEMPCONTROL;
            zoneControlNum = i;
            break;
        }
    }
    for (int i = 1; i <= state.dataZoneCtrls->NumComfortControlledZones; ++i) {
        if (state.dataZoneCtrls->ComfortControlledZone(i).ActualZoneNum == zoneNum) {
            zoneControlType = KIVAZONE_COMFORTCONTROL;
            zoneControlNum = i;
            break;
        }
    }
    for (size_t i = 1; i <= state.dataZoneCtrls->StageControlledZone.size(); ++i) {
        if (state.dataZoneCtrls->StageControlledZone(i).ActualZoneNum == zoneNum) {
            zoneControlType = KIVAZONE_STAGEDCONTROL;
            zoneControlNum = i;
            break;
        }
    }
}

void KivaInstanceMap::initGround(EnergyPlusData &state, const KivaWeatherData &kivaWeather)
{

#ifdef GROUND_PLOT
    std::string constructionName;
    if (constructionNum == 0) {
        constructionName = "Default Footing Wall Construction";
    } else {
        constructionName = state.dataConstruction->Construct(constructionNum).Name;
    }

    ss.dir = format("{}/{} {:.2R} {}",
                    FileSystem::getAbsolutePath(state.dataStrGlobals->outDirPath),
                    state.dataSurface->Surface(floorSurface).Name,
                    instance.ground->foundation.foundationDepth,
                    constructionName);

    debugDir = ss.dir;
    plotNum = 0;
    double &l = instance.ground->foundation.reductionLength2;
    constexpr double width = 0.0;
    const double depth = instance.ground->foundation.foundationDepth + width / 2.0;
    const double range = max(width, depth);
    ss.xRange = {l - range / 2.0, l + range / 2.0};
    ss.yRange = {0.5, 0.5};
    ss.zRange = {-range, instance.ground->foundation.wall.heightAboveGrade};
    ss.grid = true;
    ss.contours = false;
    ss.size = 1600;
    // ss.plotType = Kiva::SnapshotSettings::P_FLUX;

    if (ss.plotType == Kiva::SnapshotSettings::P_FLUX) {
        ss.minValue = 0.0;
        ss.maxValue = 100.0;
    }

    gp = Kiva::GroundPlot(ss, instance.ground->domain, instance.ground->foundation);
#endif

    int numAccelaratedTimesteps = 3;
    int acceleratedTimestep = 30; // days
    int accDate = getAccDate(state, numAccelaratedTimesteps, acceleratedTimestep);
    // Initialize with steady state before accelerated timestepping
    instance.ground->foundation.numericalScheme = Kiva::Foundation::NS_STEADY_STATE;
    setInitialBoundaryConditions(state, kivaWeather, accDate, 24, state.dataGlobal->TimeStepsInHour);
    instance.calculate();
    accDate += acceleratedTimestep;
    while (accDate > 365 + state.dataWeather->LeapYearAdd) {
        accDate = accDate - (365 + state.dataWeather->LeapYearAdd);
    }

    // Accelerated timestepping
    instance.ground->foundation.numericalScheme = Kiva::Foundation::NS_IMPLICIT;
    for (int i = 0; i < numAccelaratedTimesteps; ++i) {
        setInitialBoundaryConditions(state, kivaWeather, accDate, 24, state.dataGlobal->TimeStepsInHour);
        instance.calculate(acceleratedTimestep * 24 * 60 * 60);
        accDate += acceleratedTimestep;
        while (accDate > 365 + state.dataWeather->LeapYearAdd) {
            accDate = accDate - (365 + state.dataWeather->LeapYearAdd);
        }
    }

    instance.calculate_surface_averages();
    instance.foundation->numericalScheme = Kiva::Foundation::NS_ADI;
}

int KivaInstanceMap::getAccDate(EnergyPlusData &state, const int numAccelaratedTimesteps, const int acceleratedTimestep)
{
    // Determine accelerated intervals
    int accDate =
        state.dataEnvrn->DayOfYear - 1 - acceleratedTimestep * (numAccelaratedTimesteps + 1); // date time = last timestep from the day before
    while (accDate <= 0) {
        accDate = accDate + 365 + state.dataWeather->LeapYearAdd;
    }
    return accDate;
}

void KivaInstanceMap::setInitialBoundaryConditions(
    EnergyPlusData &state, const KivaWeatherData &kivaWeather, const int date, const int hour, const int timestep)
{

    unsigned index, indexPrev;
    unsigned dataSize = kivaWeather.windSpeed.size();
    Real64 weightNow;

    if (kivaWeather.intervalsPerHour == 1) {
        index = (date - 1) * 24 + (hour - 1);
        weightNow = min(1.0, (double(timestep) / double(state.dataGlobal->TimeStepsInHour)));
    } else {
        index = (date - 1) * 24 * state.dataGlobal->TimeStepsInHour + (hour - 1) * state.dataGlobal->TimeStepsInHour + (timestep - 1);
        weightNow = 1.0; // weather data interval must be the same as the timestep interval (i.e., no interpolation)
    }
    if (index == 0) {
        indexPrev = dataSize - 1;
    } else {
        indexPrev = index - 1;
    }

    instance.bcs = std::make_shared<Kiva::BoundaryConditions>();

    std::shared_ptr<Kiva::BoundaryConditions> bcs = instance.bcs;

    bcs->outdoorTemp = kivaWeather.dryBulb[index] * weightNow + kivaWeather.dryBulb[indexPrev] * (1.0 - weightNow) + Constant::Kelvin;

    bcs->localWindSpeed = (kivaWeather.windSpeed[index] * weightNow + kivaWeather.windSpeed[indexPrev] * (1.0 - weightNow)) *
                          state.dataEnvrn->WeatherFileWindModCoeff *
                          std::pow(instance.ground->foundation.grade.roughness / state.dataEnvrn->SiteWindBLHeight, state.dataEnvrn->SiteWindExp);
    bcs->skyEmissivity = kivaWeather.skyEmissivity[index] * weightNow + kivaWeather.skyEmissivity[indexPrev] * (1.0 - weightNow);
    bcs->solarAzimuth = 3.14;
    bcs->solarAltitude = 0.0;
    bcs->directNormalFlux = 0.0;
    bcs->diffuseHorizontalFlux = 0.0;
    bcs->slabAbsRadiation = 0.0;
    bcs->wallAbsRadiation = 0.0;
    bcs->deepGroundTemperature = kivaWeather.annualAverageDrybulbTemp + Constant::Kelvin;

    // Estimate indoor temperature
    constexpr Real64 defaultFlagTemp = -999;   // default sets this below -999 at -9999 so uses value if entered
    constexpr Real64 standardTemp = 22;        // degC
    Real64 assumedFloatingTemp = standardTemp; // degC (somewhat arbitrary assumption--not knowing anything else
                                               // about the building at this point)

    Real64 Tin;
    if (zoneAssumedTemperature > defaultFlagTemp) {
        Tin = zoneAssumedTemperature + Constant::Kelvin;
    } else {
        switch (zoneControlType) {
        case KIVAZONE_UNCONTROLLED: {
            Tin = assumedFloatingTemp + Constant::Kelvin;
            break;
        }
        case KIVAZONE_TEMPCONTROL: {

            auto const *ctrlTypeSched = state.dataZoneCtrls->TempControlledZone(zoneControlNum).setptTypeSched;
            HVAC::SetptType controlType = static_cast<HVAC::SetptType>(ctrlTypeSched->getHrTsVal(state, hour, timestep));

            switch (controlType) {
            case HVAC::SetptType::Uncontrolled:
                Tin = assumedFloatingTemp + Constant::Kelvin;
                break;
            case HVAC::SetptType::SingleHeat: {
                auto const *sched = state.dataZoneCtrls->TempControlledZone(zoneControlNum).setpts[(int)controlType].heatSetptSched;
                Real64 setpoint = sched->getHrTsVal(state, hour, timestep);
                Tin = setpoint + Constant::Kelvin;
            } break;

            case HVAC::SetptType::SingleCool: {
                auto const *sched = state.dataZoneCtrls->TempControlledZone(zoneControlNum).setpts[(int)controlType].coolSetptSched;
                Real64 setpoint = sched->getHrTsVal(state, hour, timestep);
                Tin = setpoint + Constant::Kelvin;
            } break;

            case HVAC::SetptType::SingleHeatCool: {
                // Heat and cool setpt scheds will be the same for this option
                auto const *sched = state.dataZoneCtrls->TempControlledZone(zoneControlNum).setpts[(int)controlType].heatSetptSched;
                Real64 setpoint = sched->getHrTsVal(state, hour, timestep);
                Tin = setpoint + Constant::Kelvin;
            } break;

            case HVAC::SetptType::DualHeatCool: {
                auto const *heatSched = state.dataZoneCtrls->TempControlledZone(zoneControlNum).setpts[(int)controlType].heatSetptSched;
                auto const *coolSched = state.dataZoneCtrls->TempControlledZone(zoneControlNum).setpts[(int)controlType].coolSetptSched;
                Real64 heatSetpoint = heatSched->getHrTsVal(state, hour, timestep);
                Real64 coolSetpoint = coolSched->getHrTsVal(state, hour, timestep);
                constexpr Real64 heatBalanceTemp = 10.0 + Constant::Kelvin; // (assumed)
                constexpr Real64 coolBalanceTemp = 15.0 + Constant::Kelvin; // (assumed)

                if (bcs->outdoorTemp < heatBalanceTemp) {
                    Tin = heatSetpoint + Constant::Kelvin;
                } else if (bcs->outdoorTemp > coolBalanceTemp) {
                    Tin = coolSetpoint + Constant::Kelvin;
                } else {
                    Real64 weight = (coolBalanceTemp - bcs->outdoorTemp) / (coolBalanceTemp - heatBalanceTemp);
                    Tin = heatSetpoint * weight + coolSetpoint * (1.0 - weight) + Constant::Kelvin;
                }
            } break;

            default: {
                Tin = 0.0;
                ShowSevereError(state,
                                format("Illegal control type for Zone={}, Found value={}, in Schedule={}",
                                       state.dataHeatBal->Zone(zoneNum).Name,
                                       controlType,
                                       state.dataZoneCtrls->TempControlledZone(zoneControlNum).setptTypeSched->Name));
            } break;

            } // switch (tstatType)
            break;
        }
        case KIVAZONE_COMFORTCONTROL: {

            Tin = standardTemp + Constant::Kelvin;
            break;
        }
        case KIVAZONE_STAGEDCONTROL: {

            auto const *heatSched = state.dataZoneCtrls->StageControlledZone(zoneControlNum).heatSetptBaseSched;
            auto const *coolSched = state.dataZoneCtrls->StageControlledZone(zoneControlNum).coolSetptBaseSched;
            Real64 heatSetpoint = heatSched->getHrTsVal(state, hour, timestep);
            Real64 coolSetpoint = coolSched->getHrTsVal(state, hour, timestep);
            constexpr Real64 heatBalanceTemp = 10.0 + Constant::Kelvin; // (assumed)
            constexpr Real64 coolBalanceTemp = 15.0 + Constant::Kelvin; // (assumed)
            if (bcs->outdoorTemp < heatBalanceTemp) {
                Tin = heatSetpoint + Constant::Kelvin;
            } else if (bcs->outdoorTemp > coolBalanceTemp) {
                Tin = coolSetpoint + Constant::Kelvin;
            } else {
                Real64 weight = (coolBalanceTemp - bcs->outdoorTemp) / (coolBalanceTemp - heatBalanceTemp);
                Tin = heatSetpoint * weight + coolSetpoint * (1.0 - weight) + Constant::Kelvin;
            }
            break;
        }
        default: {
            // error?
            Tin = assumedFloatingTemp + Constant::Kelvin;
            break;
        }
        }
    }
    bcs->slabConvectiveTemp = bcs->wallConvectiveTemp = bcs->slabRadiantTemp = bcs->wallRadiantTemp = Tin;

    bcs->gradeForcedTerm = kmPtr->surfaceConvMap[floorSurface].f;
    bcs->gradeConvectionAlgorithm = kmPtr->surfaceConvMap[floorSurface].out;
    bcs->slabConvectionAlgorithm = kmPtr->surfaceConvMap[floorSurface].in;

    if (!wallSurfaces.empty()) {
        bcs->extWallForcedTerm = kmPtr->surfaceConvMap[wallSurfaces[0]].f;
        bcs->extWallConvectionAlgorithm = kmPtr->surfaceConvMap[wallSurfaces[0]].out;
        bcs->intWallConvectionAlgorithm = kmPtr->surfaceConvMap[wallSurfaces[0]].in;
    } else {
        // If no wall surfaces, assume that any exposed foundation wall in Kiva uses
        // same algorithm as exterior grade
        bcs->extWallForcedTerm = kmPtr->surfaceConvMap[floorSurface].f;
        bcs->extWallConvectionAlgorithm = kmPtr->surfaceConvMap[floorSurface].out;
        // No interior walls
    }
}

void KivaInstanceMap::setBoundaryConditions(EnergyPlusData &state)
{
    std::shared_ptr<Kiva::BoundaryConditions> bcs = instance.bcs;

    bcs->outdoorTemp = state.dataEnvrn->OutDryBulbTemp + Constant::Kelvin;
    bcs->localWindSpeed = DataEnvironment::WindSpeedAt(state, instance.ground->foundation.grade.roughness);
    bcs->windDirection = state.dataEnvrn->WindDir * Constant::DegToRad;
    bcs->solarAzimuth = std::atan2(state.dataEnvrn->SOLCOS(1), state.dataEnvrn->SOLCOS(2));
    bcs->solarAltitude = Constant::PiOvr2 - std::acos(state.dataEnvrn->SOLCOS(3));
    bcs->directNormalFlux = state.dataEnvrn->BeamSolarRad;
    bcs->diffuseHorizontalFlux = state.dataEnvrn->DifSolarRad;
    bcs->skyEmissivity = pow4(state.dataEnvrn->SkyTempKelvin) / pow4(bcs->outdoorTemp);

    bcs->slabAbsRadiation = state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(floorSurface) +      // solar
                            state.dataHeatBal->SurfQdotRadIntGainsInPerArea(floorSurface) + // internal gains
                            state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(floorSurface);  // HVAC

    bcs->slabConvectiveTemp = state.dataHeatBal->SurfTempEffBulkAir(floorSurface) + Constant::Kelvin;
    bcs->slabRadiantTemp = ThermalComfort::CalcSurfaceWeightedMRT(state, floorSurface, false) + Constant::Kelvin;
    bcs->gradeForcedTerm = kmPtr->surfaceConvMap[floorSurface].f;
    bcs->gradeConvectionAlgorithm = kmPtr->surfaceConvMap[floorSurface].out;
    bcs->slabConvectionAlgorithm = kmPtr->surfaceConvMap[floorSurface].in;

    // Calculate area weighted average for walls
    Real64 QAtotal = 0.0;
    Real64 Atotal = 0.0;
    Real64 TARadTotal = 0.0;
    Real64 TAConvTotal = 0.0;
    for (int wl : wallSurfaces) {
        Real64 Q = state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(wl) +      // solar
                   state.dataHeatBal->SurfQdotRadIntGainsInPerArea(wl) + // internal gains
                   state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(wl);  // HVAC

        Real64 const &A = state.dataSurface->Surface(wl).Area;

        Real64 Trad = ThermalComfort::CalcSurfaceWeightedMRT(state, wl, false);
        Real64 Tconv = state.dataHeatBal->SurfTempEffBulkAir(wl);

        QAtotal += Q * A;
        TARadTotal += Trad * A;
        TAConvTotal += Tconv * A;
        Atotal += A;
    }

    if (Atotal > 0.0) {
        bcs->wallAbsRadiation = QAtotal / Atotal;
        bcs->wallRadiantTemp = TARadTotal / Atotal + Constant::Kelvin;
        bcs->wallConvectiveTemp = TAConvTotal / Atotal + Constant::Kelvin;
        bcs->extWallForcedTerm = kmPtr->surfaceConvMap[wallSurfaces[0]].f;
        bcs->extWallConvectionAlgorithm = kmPtr->surfaceConvMap[wallSurfaces[0]].out;
        bcs->intWallConvectionAlgorithm = kmPtr->surfaceConvMap[wallSurfaces[0]].in;
    } else { // No wall surfaces
        // If no wall surfaces, assume that any exposed foundation wall in Kiva uses
        // same algorithm as exterior grade
        bcs->extWallForcedTerm = kmPtr->surfaceConvMap[floorSurface].f;
        bcs->extWallConvectionAlgorithm = kmPtr->surfaceConvMap[floorSurface].out;
    }
}

KivaManager::Settings::Settings()
    : soilK(0.864), soilRho(1510), soilCp(1260), groundSolarAbs(0.9), groundThermalAbs(0.9), groundRoughness(0.9), farFieldWidth(40.0),
      deepGroundBoundary(AUTO), deepGroundDepth(40.0), autocalculateDeepGroundDepth(true), minCellDim(0.02), maxGrowthCoeff(1.5), timestepType(HOURLY)
{
}

KivaManager::WallGroup::WallGroup(Real64 exposedPerimeter, std::vector<int> wallIDs) : exposedPerimeter(exposedPerimeter), wallIDs(wallIDs)
{
}

KivaManager::WallGroup::WallGroup() : exposedPerimeter(0.0)
{
}

KivaManager::KivaManager() : timestep(3600), defaultAdded(false), defaultIndex(0)
{
}

KivaManager::~KivaManager()
{
}

void KivaManager::readWeatherData(EnergyPlusData &state)
{
    // Below from OpenEPlusWeatherFile
    auto kivaWeatherFile = state.files.inputWeatherFilePath.open(state, "KivaManager::readWeatherFile");

    // Read in Header Information
    static Array1D_string const Header(8,
                                       {"LOCATION",
                                        "DESIGN CONDITIONS",
                                        "TYPICAL/EXTREME PERIODS",
                                        "GROUND TEMPERATURES",
                                        "HOLIDAYS/DAYLIGHT SAVING",
                                        "COMMENTS 1",
                                        "COMMENTS 2",
                                        "DATA PERIODS"});

    int HdLine = 1; // Look for first Header
    bool StillLooking = true;
    while (StillLooking) {
        auto LineResult = kivaWeatherFile.readLine();
        if (LineResult.eof) {
            ShowFatalError(
                state,
                format("Kiva::ReadWeatherFile: Unexpected End-of-File on EPW Weather file, while reading header information, looking for header={}",
                       Header(HdLine)));
        }

        // Use headers to know how to read data to memory (e.g., number of periods, number of intervals)
        int endcol = LineResult.data.size();
        if (endcol > 0) {
            if (int(LineResult.data[endcol - 1]) == DataSystemVariables::iUnicode_end) {
                ShowSevereError(state, "OpenWeatherFile: EPW Weather File appears to be a Unicode or binary file.");
                ShowContinueError(state, "...This file cannot be read by this program. Please save as PC or Unix file and try again");
                ShowFatalError(state, "Program terminates due to previous condition.");
            }
        }
        std::string::size_type Pos = FindNonSpace(LineResult.data);
        std::string::size_type const HdPos = index(LineResult.data, Header(HdLine));
        if (Pos != HdPos) {
            continue;
        }
        Pos = index(LineResult.data, ',');

        // Below borrowed from ProcessEPWHeader

        if ((Pos == std::string::npos) && (!has_prefixi(Header(HdLine), "COMMENTS"))) {
            ShowSevereError(state, "Invalid Header line in in.epw -- no commas");
            ShowContinueError(state, format("Line={}", LineResult.data));
            ShowFatalError(state, "Previous conditions cause termination.");
        }
        if (Pos != std::string::npos) {
            LineResult.data.erase(0, Pos + 1);
        }

        if (Util::makeUPPER(Header(HdLine)) == "DATA PERIODS") {
            bool IOStatus;
            uppercase(LineResult.data);
            int NumHdArgs = 2;
            int Count = 1;
            while (Count <= NumHdArgs) {
                strip(LineResult.data);
                Pos = index(LineResult.data, ',');
                if (Pos == std::string::npos) {
                    if (len(LineResult.data) == 0) {
                        while (Pos == std::string::npos) {
                            LineResult.update(kivaWeatherFile.readLine());
                            strip(LineResult.data);
                            uppercase(LineResult.data);
                            Pos = index(LineResult.data, ',');
                        }
                    } else {
                        Pos = len(LineResult.data);
                    }
                }

                switch (Count) {
                case 1:
                    NumHdArgs += 4 * Util::ProcessNumber(LineResult.data.substr(0, Pos), IOStatus);
                    // TODO: Error if more than one period? Less than full year?
                    break;
                case 2:
                    kivaWeather.intervalsPerHour = Util::ProcessNumber(LineResult.data.substr(0, Pos), IOStatus);
                    break;
                default:
                    break;
                }
                LineResult.data.erase(0, Pos + 1);
                ++Count;
            }
        }

        ++HdLine;
        if (HdLine == 9) {
            StillLooking = false;
        }
    }

    bool ErrorFound = false;
    int WYear;
    int WMonth;
    int WDay;
    int WHour;
    int WMinute;
    Real64 DryBulb;
    Real64 DewPoint;
    Real64 RelHum;
    Real64 AtmPress;
    Real64 ETHoriz;
    Real64 ETDirect;
    Real64 IRHoriz;
    Real64 GLBHoriz;
    Real64 DirectRad;
    Real64 DiffuseRad;
    Real64 GLBHorizIllum;
    Real64 DirectNrmIllum;
    Real64 DiffuseHorizIllum;
    Real64 ZenLum;
    Real64 WindDir;
    Real64 WindSpeed;
    Real64 TotalSkyCover;
    Real64 OpaqueSkyCover;
    Real64 Visibility;
    Real64 CeilHeight;
    Real64 PrecipWater;
    Real64 AerosolOptDepth;
    Real64 SnowDepth;
    Real64 DaysSinceLastSnow;
    Real64 Albedo;
    Real64 LiquidPrecip;
    int PresWeathObs;
    Array1D_int PresWeathConds(9);

    Real64 totalDB = 0.0;
    int count = 0;

    while (true) {
        auto WeatherDataLine = kivaWeatherFile.readLine();
        if (WeatherDataLine.eof) {
            break;
        }
        Weather::InterpretWeatherDataLine(state,
                                          WeatherDataLine.data,
                                          ErrorFound,
                                          WYear,
                                          WMonth,
                                          WDay,
                                          WHour,
                                          WMinute,
                                          DryBulb,
                                          DewPoint,
                                          RelHum,
                                          AtmPress,
                                          ETHoriz,
                                          ETDirect,
                                          IRHoriz,
                                          GLBHoriz,
                                          DirectRad,
                                          DiffuseRad,
                                          GLBHorizIllum,
                                          DirectNrmIllum,
                                          DiffuseHorizIllum,
                                          ZenLum,
                                          WindDir,
                                          WindSpeed,
                                          TotalSkyCover,
                                          OpaqueSkyCover,
                                          Visibility,
                                          CeilHeight,
                                          PresWeathObs,
                                          PresWeathConds,
                                          PrecipWater,
                                          AerosolOptDepth,
                                          SnowDepth,
                                          DaysSinceLastSnow,
                                          Albedo,
                                          LiquidPrecip);

        // Checks for missing value
        if (DryBulb >= 99.9) {
            DryBulb = state.dataWeather->wvarsMissing.OutDryBulbTemp;
        }
        if (DewPoint >= 99.9) {
            DewPoint = state.dataWeather->wvarsMissing.OutDewPointTemp;
        }
        if (WindSpeed >= 999.0) {
            WindSpeed = state.dataWeather->wvarsMissing.WindSpeed;
        }
        if (OpaqueSkyCover >= 99.0) {
            OpaqueSkyCover = state.dataWeather->wvarsMissing.OpaqueSkyCover;
        }

        kivaWeather.dryBulb.push_back(DryBulb);
        kivaWeather.windSpeed.push_back(WindSpeed);

        Real64 OSky = OpaqueSkyCover;
        Real64 TDewK = min(DryBulb, DewPoint) + Constant::Kelvin;
        Real64 ESky = (0.787 + 0.764 * std::log(TDewK / Constant::Kelvin)) * (1.0 + 0.0224 * OSky - 0.0035 * pow_2(OSky) + 0.00028 * pow_3(OSky));

        kivaWeather.skyEmissivity.push_back(ESky);

        ++count;
        totalDB += DryBulb;
    }

    // Annual averages
    kivaWeather.annualAverageDrybulbTemp = totalDB / count;
}

bool KivaManager::setupKivaInstances(EnergyPlusData &state)
{
    std::pair<EnergyPlusData *, std::string> contextPair{&state, ""};
    Kiva::setMessageCallback(kivaErrorCallback, &contextPair);
    bool ErrorsFound = false;

    if (state.dataZoneCtrls->GetZoneAirStatsInputFlag) {
        ZoneTempPredictorCorrector::GetZoneAirSetPoints(state);
        state.dataZoneCtrls->GetZoneAirStatsInputFlag = false;
    }

    readWeatherData(state);

    auto &Surfaces = state.dataSurface->Surface;
    auto &Constructs = state.dataConstruction->Construct;
    auto &materials = state.dataMaterial->materials;

    int inst = 0;
    int surfNum = 1;

    for (auto &surface : Surfaces) {
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation && surface.Class == DataSurfaces::SurfaceClass::Floor) {

            // Find other surfaces associated with the same floor
            std::vector<int> wallSurfaces;

            for (auto &wl : foundationInputs[surface.OSCPtr].surfaces) {
                if (Surfaces(wl).Zone == surface.Zone && wl != surfNum) {
                    if (Surfaces(wl).Class != DataSurfaces::SurfaceClass::Wall) {
                        if (Surfaces(wl).Class == DataSurfaces::SurfaceClass::Floor) {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format("Foundation:Kiva=\"{}\", only one floor per Foundation:Kiva Object allowed.",
                                                   foundationInputs[surface.OSCPtr].name));
                        } else {
                            ErrorsFound = true;
                            ShowSevereError(state,
                                            format("Foundation:Kiva=\"{}\", only floor and wall surfaces are allowed to reference Foundation Outside "
                                                   "Boundary Conditions.",
                                                   foundationInputs[surface.OSCPtr].name));
                            ShowContinueError(state, format("Surface=\"{}\", is not a floor or wall.", Surfaces(wl).Name));
                        }
                    } else {
                        wallSurfaces.push_back(wl);
                    }
                }
            }

            // Calculate total exposed perimeter attributes
            std::vector<bool> isExposedPerimeter;

            bool userSetExposedPerimeter = false;
            bool useDetailedExposedPerimeter = false;
            Real64 exposedFraction = 0.0;

            auto &expPerimMap = state.dataSurfaceGeometry->exposedFoundationPerimeter.surfaceMap;
            if (expPerimMap.count(surfNum) == 1) {
                userSetExposedPerimeter = true;
                useDetailedExposedPerimeter = expPerimMap[surfNum].useDetailedExposedPerimeter;
                if (useDetailedExposedPerimeter) {
                    for (bool s : expPerimMap[surfNum].isExposedPerimeter) {
                        isExposedPerimeter.push_back(s);
                    }
                } else {
                    exposedFraction = expPerimMap[surfNum].exposedFraction;
                }
            } else {
                ErrorsFound = true;
                ShowSevereError(state,
                                format("Surface=\"{}\", references a Foundation Outside Boundary Condition but there is no corresponding "
                                       "SURFACEPROPERTY:EXPOSEDFOUNDATIONPERIMETER object defined.",
                                       Surfaces(surfNum).Name));
            }

            Kiva::Polygon floorPolygon;
            for (std::size_t i = 0; i < surface.Vertex.size(); ++i) {
                auto const &v = surface.Vertex[i];
                floorPolygon.outer().push_back(Kiva::Point(v.x, v.y));
                if (!userSetExposedPerimeter) {
                    isExposedPerimeter.push_back(true);
                }
            }

            Real64 totalPerimeter = 0.0;
            for (std::size_t i = 0; i < surface.Vertex.size(); ++i) {
                std::size_t iNext;
                if (i == surface.Vertex.size() - 1) {
                    iNext = 0;
                } else {
                    iNext = i + 1;
                }
                auto const &v = surface.Vertex[i];
                auto const &vNext = surface.Vertex[iNext];
                totalPerimeter += distance(v, vNext);
            }

            if (useDetailedExposedPerimeter) {
                Real64 total2DPerimeter = 0.0;
                Real64 exposed2DPerimeter = 0.0;
                for (std::size_t i = 0; i < floorPolygon.outer().size(); ++i) {
                    std::size_t iNext;
                    if (i == floorPolygon.outer().size() - 1) {
                        iNext = 0;
                    } else {
                        iNext = i + 1;
                    }
                    auto const &p = floorPolygon.outer()[i];
                    auto const &pNext = floorPolygon.outer()[iNext];
                    Real64 perim = Kiva::getDistance(p, pNext);
                    total2DPerimeter += perim;
                    if (isExposedPerimeter[i]) {
                        exposed2DPerimeter += perim;
                    } else {
                        exposed2DPerimeter += 0.0;
                    }
                }
                exposedFraction = std::min(exposed2DPerimeter / total2DPerimeter, 1.0);
            }

            Real64 totalExposedPerimeter = exposedFraction * totalPerimeter;

            // Remaining exposed perimeter will be alloted to each instance as appropriate
            Real64 remainingExposedPerimeter = totalExposedPerimeter;

            // Get combinations of wall constructions and wall heights -- each different
            // combination gets its own Kiva instance. Combination map points each set
            // of construction and wall height to the associated exposed perimeter and
            // list of wall surface numbers.
            std::map<std::pair<int, Real64>, WallGroup> combinationMap;

            if (!wallSurfaces.empty()) {
                for (int wl : wallSurfaces) {

                    auto const &v = Surfaces(wl).Vertex;
                    size_t numVs = v.size();
                    // Enforce quadrilateralism
                    if (numVs > 4) {
                        ShowWarningError(state,
                                         format("Foundation:Kiva=\"{}\", wall surfaces with more than four vertices referencing",
                                                foundationInputs[surface.OSCPtr].name));
                        ShowContinueError(
                            state, "...Foundation Outside Boundary Conditions may not be interpreted correctly in the 2D finite difference model.");
                        ShowContinueError(state, format("Surface=\"{}\", has {} vertices.", Surfaces(wl).Name, numVs));
                        ShowContinueError(state,
                                          "Consider separating the wall into separate surfaces, each spanning from the floor slab to the top of "
                                          "the foundation wall.");
                    }

                    // get coplanar points with floor to determine perimeter
                    std::vector<int> coplanarPoints = Vectors::PointsInPlane(
                        Surfaces(surfNum).Vertex, Surfaces(surfNum).Sides, Surfaces(wl).Vertex, Surfaces(wl).Sides, ErrorsFound);

                    Real64 perimeter = 0.0;

                    // if there are two consecutive coplanar points, add the distance
                    // between them to the overall perimeter for this wall
                    for (std::size_t i = 0; i < coplanarPoints.size(); ++i) {
                        int p(coplanarPoints[i]);
                        int pC = p == (int)v.size() ? 1 : p + 1;                                             // next consecutive point
                        int p2 = i == coplanarPoints.size() - 1 ? coplanarPoints[0] : coplanarPoints[i + 1]; // next coplanar point

                        if (p2 == pC) { // if next coplanar point is the next consecutive point
                            perimeter += distance(v(p), v(p2));
                        }
                    }

                    if (perimeter == 0.0) {
                        ShowWarningError(state, format("Foundation:Kiva=\"{}\".", foundationInputs[surface.OSCPtr].name));
                        ShowContinueError(state, format("   Wall Surface=\"{}\", does not have any vertices that are", Surfaces(wl).Name));
                        ShowContinueError(state, format("   coplanar with the corresponding Floor Surface=\"{}\".", Surfaces(surfNum).Name));
                        ShowContinueError(state,
                                          "   Simulation will continue using the distance between the two lowest points in the wall for the "
                                          "interface distance.");

                        // sort vertices by Z-value
                        std::vector<int> zs;
                        for (std::size_t i = 0; i < numVs; ++i) {
                            zs.push_back(i);
                        }
                        sort(zs.begin(), zs.end(), [v](int a, int b) { return v[a].z < v[b].z; });
                        perimeter = distance(v[zs[0]], v[zs[1]]);
                    }

                    Real64 surfHeight = Surfaces(wl).get_average_height(state);
                    // round to avoid numerical precision differences
                    surfHeight = std::round((surfHeight) * 1000.0) / 1000.0;

                    if (combinationMap.count({Surfaces(wl).Construction, surfHeight}) == 0) {
                        // create new combination
                        std::vector<int> walls = {wl};
                        combinationMap[{Surfaces(wl).Construction, surfHeight}] = WallGroup(perimeter, walls);
                    } else {
                        // add to existing combination
                        combinationMap[{Surfaces(wl).Construction, surfHeight}].exposedPerimeter += perimeter;
                        combinationMap[{Surfaces(wl).Construction, surfHeight}].wallIDs.push_back(wl);
                    }
                }
            }

            // setup map to point floor surface to all related kiva instances
            Kiva::Aggregator floorAggregator(Kiva::Surface::ST_SLAB_CORE);

            // Loop through combinations and assign instances until there is no remaining exposed pereimeter
            bool assignKivaInstances = true;
            auto comb = combinationMap.begin();
            while (assignKivaInstances) {
                int constructionNum;
                Real64 wallHeight;
                Real64 perimeter;
                std::vector<int> wallIDs;
                if (comb != combinationMap.end()) {
                    // Loop through wall combinations first
                    constructionNum = comb->first.first;
                    wallHeight = comb->first.second;
                    perimeter = comb->second.exposedPerimeter;
                    wallIDs = comb->second.wallIDs;
                } else {
                    // Assign the remaining exposed perimeter to a slab instance
                    constructionNum = foundationInputs[surface.OSCPtr].wallConstructionIndex;
                    wallHeight = 0.0;
                    perimeter = remainingExposedPerimeter;
                }

                Real64 floorWeight;

                if (totalExposedPerimeter > 0.001) {
                    floorWeight = perimeter / totalExposedPerimeter;
                } else {
                    floorWeight = 1.0;
                }

                // Copy foundation input for this instance
                Kiva::Foundation fnd = foundationInputs[surface.OSCPtr].foundation;

                // Exposed Perimeter
                fnd.useDetailedExposedPerimeter = useDetailedExposedPerimeter;
                fnd.isExposedPerimeter = isExposedPerimeter;
                fnd.exposedFraction = exposedFraction;

                if (constructionNum > 0) {
                    auto &c = Constructs(constructionNum);

                    // Clear layers
                    fnd.wall.layers.clear();

                    // Push back construction's layers
                    for (int layer = 1; layer <= c.TotLayers; layer++) {
                        auto const *mat = materials(c.LayerPoint(layer));
                        if (mat->ROnly) {
                            ErrorsFound = true;
                            ShowSevereError(state, format("Construction=\"{}\", constructions referenced by surfaces with a", c.Name));
                            ShowContinueError(state, "\"Foundation\" Outside Boundary Condition must use only regular material objects");
                            ShowContinueError(state, format("Material=\"{}\", is not a regular material object", mat->Name));
                            return ErrorsFound;
                        }

                        Kiva::Layer tempLayer;

                        tempLayer.material = Kiva::Material(mat->Conductivity, mat->Density, mat->SpecHeat);
                        tempLayer.thickness = mat->Thickness;

                        fnd.wall.layers.push_back(tempLayer);
                    }
                    fnd.wall.interior.emissivity = Constructs(constructionNum).InsideAbsorpThermal;
                    fnd.wall.interior.absorptivity = Constructs(constructionNum).InsideAbsorpSolar;
                    fnd.wall.exterior.emissivity = Constructs(constructionNum).OutsideAbsorpThermal;
                    fnd.wall.exterior.absorptivity = Constructs(constructionNum).OutsideAbsorpSolar;
                }

                // Set slab construction
                for (int i = 0; i < Constructs(surface.Construction).TotLayers; ++i) {
                    auto const *mat = materials(Constructs(surface.Construction).LayerPoint[i]);
                    if (mat->ROnly) {
                        ErrorsFound = true;
                        ShowSevereError(
                            state, format("Construction=\"{}\", constructions referenced by surfaces with a", Constructs(surface.Construction).Name));
                        ShowContinueError(state, "\"Foundation\" Outside Boundary Condition must use only regular material objects");
                        ShowContinueError(state, format("Material=\"{}\", is not a regular material object", mat->Name));
                        return ErrorsFound;
                    }

                    Kiva::Layer tempLayer;

                    tempLayer.material = Kiva::Material(mat->Conductivity, mat->Density, mat->SpecHeat);
                    tempLayer.thickness = mat->Thickness;

                    fnd.slab.layers.push_back(tempLayer);
                }

                fnd.slab.interior.emissivity = Constructs(surface.Construction).InsideAbsorpThermal;
                fnd.slab.interior.absorptivity = Constructs(surface.Construction).InsideAbsorpSolar;

                fnd.foundationDepth = wallHeight;

                fnd.hasPerimeterSurface = false;
                fnd.perimeterSurfaceWidth = 0.0;

                // Add blocks
                auto intHIns = foundationInputs[surface.OSCPtr].intHIns; // (AUTO_OK_OBJ) intend to make a copy?
                auto intVIns = foundationInputs[surface.OSCPtr].intVIns; // (AUTO_OK_OBJ) intend to make a copy?
                auto extHIns = foundationInputs[surface.OSCPtr].extHIns; // (AUTO_OK_OBJ) intend to make a copy?
                auto extVIns = foundationInputs[surface.OSCPtr].extVIns; // (AUTO_OK_OBJ) intend to make a copy?
                auto footing = foundationInputs[surface.OSCPtr].footing; // (AUTO_OK_OBJ) intend to make a copy?

                if (std::abs(intHIns.width) > 0.0) {
                    intHIns.z += fnd.foundationDepth + fnd.slab.totalWidth();
                    fnd.inputBlocks.push_back(intHIns);
                }
                if (std::abs(intVIns.width) > 0.0) {
                    fnd.inputBlocks.push_back(intVIns);
                }
                if (std::abs(extHIns.width) > 0.0) {
                    extHIns.z += fnd.wall.heightAboveGrade;
                    extHIns.x = fnd.wall.totalWidth();
                    fnd.inputBlocks.push_back(extHIns);
                }
                if (std::abs(extVIns.width) > 0.0) {
                    extVIns.x = fnd.wall.totalWidth();
                    fnd.inputBlocks.push_back(extVIns);
                }
                if (std::abs(footing.width) > 0.0) {
                    footing.z = fnd.foundationDepth + fnd.slab.totalWidth() + fnd.wall.depthBelowSlab;
                    footing.x = fnd.wall.totalWidth() / 2.0 - footing.width / 2.0;
                    fnd.inputBlocks.push_back(footing);
                }

                Real64 initDeepGroundDepth = fnd.deepGroundDepth;
                fnd.deepGroundDepth = getDeepGroundDepth(fnd);

                if (fnd.deepGroundDepth > initDeepGroundDepth) {
                    ShowWarningError(state,
                                     format("Foundation:Kiva=\"{}\", the autocalculated deep ground depth ({:.3T} m) is shallower than "
                                            "foundation construction elements ({:.3T} m)",
                                            foundationInputs[surface.OSCPtr].name,
                                            initDeepGroundDepth,
                                            fnd.deepGroundDepth - 1.0));
                    ShowContinueError(state,
                                      format("The deep ground depth will be set one meter below the lowest element ({:.3T} m)", fnd.deepGroundDepth));
                }

                // polygon

                fnd.polygon = floorPolygon;

                std::pair<EnergyPlusData *, std::string> contexPair2{&state, format("Foundation:Kiva=\"{}\"", foundationInputs[surface.OSCPtr].name)};
                Kiva::setMessageCallback(kivaErrorCallback, &contexPair2);

                // point surface to associated ground instance(s)
                kivaInstances.emplace_back(state,
                                           fnd,
                                           surfNum,
                                           wallIDs,
                                           surface.Zone,
                                           foundationInputs[surface.OSCPtr].assumedIndoorTemperature,
                                           floorWeight,
                                           constructionNum,
                                           this);

                // Floors can point to any number of foundation surfaces
                floorAggregator.add_instance(kivaInstances[inst].instance.ground.get(), floorWeight);

                // Walls can only have one associated ground instance
                for (int wl : wallIDs) {
                    surfaceMap[wl] = Kiva::Aggregator(Kiva::Surface::ST_WALL_INT);
                    surfaceMap[wl].add_instance(kivaInstances[inst].instance.ground.get(), 1.0);
                }

                // Increment instance counter
                inst++;

                // Increment wall combinations iterator
                if (comb != combinationMap.end()) {
                    comb++;
                }

                remainingExposedPerimeter -= perimeter;

                if (remainingExposedPerimeter < 0.001) {
                    assignKivaInstances = false;
                    if (remainingExposedPerimeter < -0.1) {
                        ErrorsFound = true;
                        ShowSevereError(state, format("For Floor Surface=\"{}\", the Wall surfaces referencing", Surfaces(surfNum).Name));
                        ShowContinueError(state, format("  the same Foundation:Kiva=\"{}\" have", foundationInputs[Surfaces(surfNum).OSCPtr].name));
                        ShowContinueError(state, "  a combined length greater than the exposed perimeter of the foundation.");
                        ShowContinueError(state, "  Ensure that each Wall surface shares at least one edge with the corresponding");
                        ShowContinueError(state, "  Floor surface.");
                    }
                }
            }

            surfaceMap[surfNum] = floorAggregator;
        }

        surfNum++;
    }

    // Loop through Foundation surfaces and make sure they are all assigned to an instance
    for (int surfNum2 : state.dataSurface->AllHTKivaSurfaceList) {
        if (surfaceMap[surfNum2].size() == 0) {
            ErrorsFound = true;
            ShowSevereError(state, format("Surface=\"{}\" has a 'Foundation' Outside Boundary Condition", Surfaces(surfNum).Name));
            ShowContinueError(state, format("  referencing Foundation:Kiva=\"{}\".", foundationInputs[Surfaces(surfNum).OSCPtr].name));
            if (Surfaces(surfNum2).Class == DataSurfaces::SurfaceClass::Wall) {
                ShowContinueError(state, format("  You must also reference Foundation:Kiva=\"{}\"", foundationInputs[Surfaces(surfNum).OSCPtr].name));
                ShowContinueError(state,
                                  format("  in a floor surface within the same Zone=\"{}\".", state.dataHeatBal->Zone(Surfaces(surfNum).Zone).Name));
            } else if (Surfaces(surfNum2).Class == DataSurfaces::SurfaceClass::Floor) {
                ShowContinueError(state, "  However, this floor was never assigned to a Kiva instance.");
                ShowContinueError(state, "  This should not occur for floor surfaces. Please report to EnergyPlus Development Team.");
            } else {
                ShowContinueError(state, "  Only floor and wall surfaces are allowed to reference 'Foundation' Outside Boundary Conditions.");
                ShowContinueError(state, format("  Surface=\"{}\", is not a floor or wall.", Surfaces(surfNum).Name));
            }
        }
    }

    print(state.files.eio,
          "{}",
          "! <Kiva Foundation Name>, Horizontal Cells, Vertical Cells, Total Cells, Total Exposed "
          "Perimeter, Perimeter Fraction, Wall Height, Wall Construction, Floor Surface, Wall "
          "Surface(s)\n");

    for (auto &kv : kivaInstances) {
        auto grnd = kv.instance.ground.get(); // (AUTO_OK_OBJ)

        std::string constructionName;
        if (kv.constructionNum <= 0) {
            constructionName = "<Default Footing Wall Construction>";
        } else {
            constructionName = state.dataConstruction->Construct(kv.constructionNum).Name;
        }

        std::string wallSurfaceString;
        for (int wl : kv.wallSurfaces) {
            wallSurfaceString += "," + state.dataSurface->Surface(wl).Name;
        }

        static constexpr std::string_view fmt = "{},{},{},{},{:.2R},{:.2R},{:.2R},{},{}{}\n";
        print(state.files.eio,
              fmt,
              foundationInputs[state.dataSurface->Surface(kv.floorSurface).OSCPtr].name,
              grnd->nX,
              grnd->nZ,
              grnd->nX * grnd->nZ,
              grnd->foundation.netPerimeter,
              kv.floorWeight,
              grnd->foundation.foundationDepth,
              constructionName,
              state.dataSurface->Surface(kv.floorSurface).Name,
              wallSurfaceString);
    }

    return ErrorsFound;
}

Real64 KivaManager::getDeepGroundDepth(Kiva::Foundation fnd)
{
    Real64 totalDepthOfWallBelowGrade = fnd.wall.depthBelowSlab + (fnd.foundationDepth - fnd.wall.heightAboveGrade) + fnd.slab.totalWidth();
    if (fnd.deepGroundDepth < totalDepthOfWallBelowGrade + 1.0) {
        fnd.deepGroundDepth = totalDepthOfWallBelowGrade + 1.0;
    }
    for (auto &block : fnd.inputBlocks) {
        // Change temporary zero depth indicators to default foundation depth
        if (block.depth == 0.0) {
            block.depth = fnd.foundationDepth;
        }
        if (settings.deepGroundBoundary == Settings::AUTO) {
            // Ensure automatically set deep ground depth is at least 1 meter below lowest block
            if (block.z + block.depth + 1.0 > fnd.deepGroundDepth) {
                fnd.deepGroundDepth = block.z + block.depth + 1.0;
            }
        }
    }
    return fnd.deepGroundDepth;
}

void KivaManager::initKivaInstances(EnergyPlusData &state)
{
    // initialize temperatures at the beginning of run environment
    for (auto &kv : kivaInstances) {
        // Start with steady-state solution
        kv.initGround(state, kivaWeather);
    }
    calcKivaSurfaceResults(state);
}

void KivaManager::calcKivaInstances(EnergyPlusData &state)
{
    // calculate heat transfer through ground
    for (auto &kv : kivaInstances) {
        kv.setBoundaryConditions(state);
        kv.instance.calculate(timestep);
        kv.instance.calculate_surface_averages();
#ifdef GROUND_PLOT
        if (state.dataEnvrn->Month == 1 && state.dataEnvrn->DayOfMonth == 1 && state.dataGlobal->HourOfDay == 1 && state.dataGlobal->TimeStep == 1) {
            kv.plotDomain(state);
        }
#endif
    }

    calcKivaSurfaceResults(state);
}

#ifdef GROUND_PLOT
void KivaInstanceMap::plotDomain(EnergyPlusData &state)
{
    gp.createFrame(*instance.ground, format("{}/{} {}:00", state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth, state.dataGlobal->HourOfDay));

    instance.ground->writeCSV(format("{}/{}.csv", debugDir, plotNum));

    plotNum++;
}
#endif

void KivaManager::calcKivaSurfaceResults(EnergyPlusData &state)
{
    for (int surfNum : state.dataSurface->AllHTKivaSurfaceList) {
        std::pair<EnergyPlusData *, std::string> contextPair{&state, format("Surface=\"{}\"", state.dataSurface->Surface(surfNum).Name)};
        Kiva::setMessageCallback(kivaErrorCallback, &contextPair);
        surfaceMap[surfNum].calc_weighted_results();
        state.dataHeatBalSurf->SurfHConvInt(surfNum) = state.dataSurfaceGeometry->kivaManager.surfaceMap[surfNum].results.hconv;
    }
    Kiva::setMessageCallback(kivaErrorCallback, nullptr);
}

void KivaManager::defineDefaultFoundation(EnergyPlusData &state)
{

    Kiva::Foundation defFnd;

    // From settings
    defFnd.soil = Kiva::Material(settings.soilK, settings.soilRho, settings.soilCp);
    defFnd.grade.absorptivity = settings.groundSolarAbs;
    defFnd.grade.emissivity = settings.groundThermalAbs;
    defFnd.grade.roughness = settings.groundRoughness;
    defFnd.farFieldWidth = settings.farFieldWidth;

    Real64 waterTableDepth = 0.1022 * state.dataEnvrn->Elevation;

    if (settings.deepGroundBoundary == Settings::AUTO) {
        if (waterTableDepth <= 40.) {
            defFnd.deepGroundDepth = waterTableDepth;
            defFnd.deepGroundBoundary = Kiva::Foundation::DGB_FIXED_TEMPERATURE;
        } else {
            defFnd.deepGroundDepth = 40.;
            defFnd.deepGroundBoundary = Kiva::Foundation::DGB_ZERO_FLUX;
        }
        if (!settings.autocalculateDeepGroundDepth) {
            if (defFnd.deepGroundDepth != settings.deepGroundDepth) {
                ShowWarningError(state, "Foundation:Kiva:Settings, when Deep-Ground Boundary Condition is Autoselect,");
                ShowContinueError(state, format("the user-specified Deep-Ground Depth ({:.1R} m)", settings.deepGroundDepth));
                ShowContinueError(state, format("will be overridden with the Autoselected depth ({:.1R} m)", defFnd.deepGroundDepth));
            }
        }
    } else if (settings.deepGroundBoundary == Settings::ZERO_FLUX) {
        defFnd.deepGroundDepth = settings.deepGroundDepth;
        defFnd.deepGroundBoundary = Kiva::Foundation::DGB_ZERO_FLUX;
    } else { // if (settings.deepGroundBoundary == Settings::GROUNDWATER)
        defFnd.deepGroundDepth = settings.deepGroundDepth;
        defFnd.deepGroundBoundary = Kiva::Foundation::DGB_FIXED_TEMPERATURE;
    }

    defFnd.wall.heightAboveGrade = 0.2; // m

    Kiva::Material concrete;
    concrete.conductivity = 1.95; // W/m-K
    concrete.density = 2400;      // kg/m3
    concrete.specificHeat = 900;  // J/kg-K

    Kiva::Layer defaultFoundationWall;
    defaultFoundationWall.thickness = 0.3; // m
    defaultFoundationWall.material = concrete;

    defFnd.wall.layers.push_back(defaultFoundationWall);

    defFnd.wall.interior.emissivity = 0.9;
    defFnd.wall.interior.absorptivity = 0.9;
    defFnd.wall.exterior.emissivity = 0.9;
    defFnd.wall.exterior.absorptivity = 0.9;

    defFnd.wall.depthBelowSlab = 0.0;

    defFnd.mesh.minCellDim = settings.minCellDim;
    defFnd.mesh.maxNearGrowthCoeff = settings.maxGrowthCoeff;
    defFnd.mesh.maxDepthGrowthCoeff = settings.maxGrowthCoeff;
    defFnd.mesh.maxInteriorGrowthCoeff = settings.maxGrowthCoeff;
    defFnd.mesh.maxExteriorGrowthCoeff = settings.maxGrowthCoeff;

    defaultFoundation.foundation = defFnd;
    defaultFoundation.name = "<Default Foundation>";
    defaultFoundation.assumedIndoorTemperature = -9999;
}

void KivaManager::addDefaultFoundation()
{
    foundationInputs.push_back(defaultFoundation);
    defaultIndex = static_cast<int>(foundationInputs.size() - 1u);
    defaultAdded = true;
}

int KivaManager::findFoundation(std::string const &name)
{
    int fndNum = 0;
    for (auto const &fnd : foundationInputs) {
        // Check if foundation exists
        if (fnd.name == name) {
            return fndNum;
        }
        fndNum++;
    }
    return (int)foundationInputs.size();
}

} // namespace EnergyPlus::HeatBalanceKivaManager
