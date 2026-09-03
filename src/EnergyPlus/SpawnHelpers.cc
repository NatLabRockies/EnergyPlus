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

#include <EnergyPlus/SpawnHelpers.hh>

// C++ Headers
#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>
#include <EnergyPlus/api/datatransfer.h>

#include <fmt/format.h>

namespace EnergyPlus::Spawn {

namespace {

std::string upperCase(std::string_view value)
{
    std::string uppercaseValue(value);
    std::transform(uppercaseValue.begin(), uppercaseValue.end(), uppercaseValue.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return uppercaseValue;
}

bool validZoneIndex(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    return zoneNum > 0 && zoneNum <= state.dataGlobal->NumOfZones;
}

bool validSurfaceIndex(EnergyPlus::EnergyPlusData &state, int surfaceNum)
{
    return surfaceNum > 0 && surfaceNum <= state.dataSurface->TotSurfaces;
}

} // namespace

ZoneSums::ZoneSums(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!validZoneIndex(state, zoneNum)) {
        m_tempDepCoef = 0.0;
        m_tempIndCoef = 0.0;
        m_qConSenFlow = 0.0;
        return;
    }

    auto &zoneHeatBalance = state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum);
    zoneHeatBalance.calcZoneOrSpaceSums(state, true, zoneNum);

    m_tempDepCoef = zoneHeatBalance.SumHA + zoneHeatBalance.SumMCp;
    m_tempIndCoef = zoneHeatBalance.SumIntGain + zoneHeatBalance.SumHATsurf + zoneHeatBalance.SumMCpT;
    m_qConSenFlow = m_tempIndCoef - (m_tempDepCoef * zoneHeatBalance.MAT);
}

Real64 ZoneSums::TempDepCoef() const
{
    return m_tempDepCoef;
}

Real64 ZoneSums::TempIndCoef() const
{
    return m_tempIndCoef;
}

Real64 ZoneSums::QConSenFlow() const
{
    return m_qConSenFlow;
}

int ZoneNum(EnergyPlus::EnergyPlusData &state, std::string_view zoneName)
{
    std::string const upperZoneName = upperCase(zoneName);

    for (int zoneIndex = 1; zoneIndex <= state.dataGlobal->NumOfZones; ++zoneIndex) {
        if (state.dataHeatBal->Zone(zoneIndex).Name == upperZoneName) {
            return zoneIndex;
        }
    }

    return 0;
}

std::vector<int> ZoneNums(EnergyPlus::EnergyPlusData &state, const std::vector<std::string> &zoneNames)
{
    std::vector<int> result;
    result.reserve(zoneNames.size());

    for (auto const &name : zoneNames) {
        result.push_back(ZoneNum(state, name));
    }

    return result;
}

Real64 ZoneVolume(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataHeatBal->Zone(zoneNum).Volume;
}

Real64 ZoneFloorArea(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataHeatBal->Zone(zoneNum).FloorArea;
}

Real64 ZoneVolCapMultpSens(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataHeatBal->Zone(zoneNum).ZoneVolCapMultpSens;
}

Real64 ZoneLatentGain(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum).latentGain;
}

Real64 ZoneMeanRadiantTemp(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum).MRT;
}

bool HaveSizingInfo(EnergyPlus::EnergyPlusData &state)
{
    return !state.dataSize->FinalZoneSizing.empty();
}

namespace zone_sizing {

Real64 SensibleCoolingLoad(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!HaveSizingInfo(state) || !validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataSize->FinalZoneSizing(zoneNum).DesCoolLoad;
}

Real64 LatentCoolingLoad(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!HaveSizingInfo(state) || !validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataSize->FinalZoneSizing(zoneNum).DesLatentCoolLoad;
}

Real64 OutdoorTempAtPeakCool(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!HaveSizingInfo(state) || !validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataSize->FinalZoneSizing(zoneNum).OutTempAtCoolPeak;
}

Real64 OutdoorHumidityRatioAtPeakCool(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!HaveSizingInfo(state) || !validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataSize->FinalZoneSizing(zoneNum).OutHumRatAtCoolPeak;
}

Real64 TimeAtPeakCool(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!HaveSizingInfo(state) || !validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataSize->FinalZoneSizing(zoneNum).TimeStepNumAtCoolMax * state.dataGlobal->TimeStepZoneSec;
}

Real64 HeatingLoad(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!HaveSizingInfo(state) || !validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataSize->FinalZoneSizing(zoneNum).DesHeatLoad;
}

Real64 OutdoorTempAtPeakHeat(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!HaveSizingInfo(state) || !validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataSize->FinalZoneSizing(zoneNum).OutTempAtHeatPeak;
}

Real64 OutdoorHumidityRatioAtPeakHeat(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!HaveSizingInfo(state) || !validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataSize->FinalZoneSizing(zoneNum).OutHumRatAtHeatPeak;
}

Real64 TimeAtPeakHeat(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!HaveSizingInfo(state) || !validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataSize->FinalZoneSizing(zoneNum).TimeStepNumAtHeatMax * state.dataGlobal->TimeStepZoneSec;
}

Real64 MinCoolOA(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!HaveSizingInfo(state) || !validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataSize->FinalZoneSizing(zoneNum).MinOA * state.dataSize->FinalZoneSizing(zoneNum).DesCoolDens;
}

Real64 MinHeatOA(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!HaveSizingInfo(state) || !validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataSize->FinalZoneSizing(zoneNum).MinOA * state.dataSize->FinalZoneSizing(zoneNum).DesHeatDens;
}

} // namespace zone_sizing

namespace zone_group_sizing {

struct PeakLoad
{
    Real64 value{0.0};
    int dayTimeStep{0};
    int designDayIndex{0};
    int referenceZone{0};
};

using GetLoadSeqFunc = std::function<ObjexxFCL::Array1D<Real64>(const EnergyPlus::DataSizing::ZoneSizingData &)>;

PeakLoad getPeakLoad(EnergyPlus::EnergyPlusData &state,
                     const std::vector<int> &zoneNumbers,
                     const GetLoadSeqFunc &getLoadSeq)
{
    PeakLoad result{};

    if (!HaveSizingInfo(state)) {
        return result;
    }

    std::vector<int> validZones;
    validZones.reserve(zoneNumbers.size());
    for (int zoneNumber : zoneNumbers) {
        if (validZoneIndex(state, zoneNumber)) {
            validZones.push_back(zoneNumber);
        }
    }

    if (validZones.empty()) {
        return result;
    }

    auto &zoneSizing = state.dataSize->ZoneSizing;
    int const firstZone = validZones.front();
    result.referenceZone = firstZone;

    int const numDesignDays = static_cast<int>(zoneSizing.isize1());
    bool peakFound = false;

    for (int designDay = 1; designDay <= numDesignDays; ++designDay) {
        auto const loadSeqForFirstZone = getLoadSeq(zoneSizing(designDay, firstZone));
        int const numTimeSteps = static_cast<int>(loadSeqForFirstZone.size());
        std::vector<Real64> combinedLoads(numTimeSteps, 0.0);

        for (int zoneNumber : validZones) {
            auto const loadSeq = getLoadSeq(zoneSizing(designDay, zoneNumber));
            for (int timeStep = 1; timeStep <= numTimeSteps; ++timeStep) {
                combinedLoads[timeStep - 1] += loadSeq(timeStep);
            }
        }

        auto const peakIter = std::max_element(combinedLoads.begin(), combinedLoads.end());
        if (peakIter == combinedLoads.end()) {
            continue;
        }

        Real64 const peakValue = *peakIter;
        int const peakTimeStep = static_cast<int>(std::distance(combinedLoads.begin(), peakIter) + 1);

        if (!peakFound || peakValue > result.value) {
            result.value = peakValue;
            result.dayTimeStep = peakTimeStep;
            result.designDayIndex = designDay;
            peakFound = true;
        }
    }

    if (!peakFound) {
        result.referenceZone = 0;
    }

    return result;
}

Real64 SensibleCoolingLoad(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers)
{
    auto const getCoolingLoadSeq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizingData) {
        return sizingData.CoolLoadSeq;
    };

    return getPeakLoad(state, zoneNumbers, getCoolingLoadSeq).value;
}

Real64 LatentCoolingLoad(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers)
{
    auto const getLatentCoolingLoadSeq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizingData) {
        return sizingData.LatentCoolLoadSeq;
    };

    return getPeakLoad(state, zoneNumbers, getLatentCoolingLoadSeq).value;
}

Real64 OutdoorTempAtPeakCool(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers)
{
    auto const getCoolingLoadSeq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizingData) {
        return sizingData.CoolLoadSeq;
    };

    auto const peakLoad = getPeakLoad(state, zoneNumbers, getCoolingLoadSeq);
    if (peakLoad.designDayIndex == 0 || peakLoad.referenceZone == 0) {
        return 0.0;
    }

    auto const &zoneSizing = state.dataSize->ZoneSizing(peakLoad.designDayIndex, peakLoad.referenceZone);
    return zoneSizing.CoolOutTempSeq(peakLoad.dayTimeStep);
}

Real64 OutdoorHumidityRatioAtPeakCool(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers)
{
    auto const getCoolingLoadSeq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizingData) {
        return sizingData.CoolLoadSeq;
    };

    auto const peakLoad = getPeakLoad(state, zoneNumbers, getCoolingLoadSeq);
    if (peakLoad.designDayIndex == 0 || peakLoad.referenceZone == 0) {
        return 0.0;
    }

    auto const &zoneSizing = state.dataSize->ZoneSizing(peakLoad.designDayIndex, peakLoad.referenceZone);
    return zoneSizing.CoolOutHumRatSeq(peakLoad.dayTimeStep);
}

Real64 TimeAtPeakCool(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers)
{
    auto const getCoolingLoadSeq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizingData) {
        return sizingData.CoolLoadSeq;
    };

    auto const peakLoad = getPeakLoad(state, zoneNumbers, getCoolingLoadSeq);
    return peakLoad.dayTimeStep * state.dataGlobal->TimeStepZoneSec;
}

Real64 HeatingLoad(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers)
{
    auto const getHeatingLoadSeq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizingData) {
        return sizingData.HeatLoadSeq;
    };

    return getPeakLoad(state, zoneNumbers, getHeatingLoadSeq).value;
}

Real64 OutdoorTempAtPeakHeat(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers)
{
    auto const getHeatingLoadSeq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizingData) {
        return sizingData.HeatLoadSeq;
    };

    auto const peakLoad = getPeakLoad(state, zoneNumbers, getHeatingLoadSeq);
    if (peakLoad.designDayIndex == 0 || peakLoad.referenceZone == 0) {
        return 0.0;
    }

    auto const &zoneSizing = state.dataSize->ZoneSizing(peakLoad.designDayIndex, peakLoad.referenceZone);
    return zoneSizing.HeatOutTempSeq(peakLoad.dayTimeStep);
}

Real64 OutdoorHumidityRatioAtPeakHeat(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers)
{
    auto const getHeatingLoadSeq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizingData) {
        return sizingData.HeatLoadSeq;
    };

    auto const peakLoad = getPeakLoad(state, zoneNumbers, getHeatingLoadSeq);
    if (peakLoad.designDayIndex == 0 || peakLoad.referenceZone == 0) {
        return 0.0;
    }

    auto const &zoneSizing = state.dataSize->ZoneSizing(peakLoad.designDayIndex, peakLoad.referenceZone);
    return zoneSizing.HeatOutHumRatSeq(peakLoad.dayTimeStep);
}

Real64 TimeAtPeakHeat(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers)
{
    auto const getHeatingLoadSeq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizingData) {
        return sizingData.HeatLoadSeq;
    };

    auto const peakLoad = getPeakLoad(state, zoneNumbers, getHeatingLoadSeq);
    return peakLoad.dayTimeStep * state.dataGlobal->TimeStepZoneSec;
}

Real64 MinCoolOA(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers)
{
    if (!HaveSizingInfo(state)) {
        return 0.0;
    }

    Real64 sumOutdoorAir = 0.0;
    for (int zoneNumber : zoneNumbers) {
        if (validZoneIndex(state, zoneNumber)) {
            sumOutdoorAir += state.dataSize->FinalZoneSizing(zoneNumber).MinOA *
                             state.dataSize->FinalZoneSizing(zoneNumber).DesCoolDens;
        }
    }

    return sumOutdoorAir;
}

Real64 MinHeatOA(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers)
{
    if (!HaveSizingInfo(state)) {
        return 0.0;
    }

    Real64 sumOutdoorAir = 0.0;
    for (int zoneNumber : zoneNumbers) {
        if (validZoneIndex(state, zoneNumber)) {
            sumOutdoorAir += state.dataSize->FinalZoneSizing(zoneNumber).MinOA *
                             state.dataSize->FinalZoneSizing(zoneNumber).DesHeatDens;
        }
    }

    return sumOutdoorAir;
}

} // namespace zone_group_sizing

void SetZoneTemperature(EnergyPlus::EnergyPlusData &state, int zoneNum, Real64 temperature)
{
    if (!validZoneIndex(state, zoneNum)) {
        return;
    }

    auto &zoneHeatBalance = state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum);
    zoneHeatBalance.ZT = temperature;
    zoneHeatBalance.ZTAV = temperature;
    zoneHeatBalance.MAT = temperature;

    auto &zone = state.dataHeatBal->Zone(zoneNum);
    for (int spaceNum : zone.spaceIndexes) {
        auto &spaceHeatBalance = state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum);
        spaceHeatBalance.ZT = temperature;
        spaceHeatBalance.ZTAV = temperature;
        spaceHeatBalance.MAT = temperature;
    }
}

Real64 ZoneTemperature(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum).ZT;
}

void SetZoneHumidityRatio(EnergyPlus::EnergyPlusData &state, int zoneNum, Real64 humidityRatio)
{
    if (!validZoneIndex(state, zoneNum)) {
        return;
    }

    auto &zoneHeatBalance = state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum);

    zoneHeatBalance.airHumRatAvg = humidityRatio;
    zoneHeatBalance.airHumRat = humidityRatio;
    zoneHeatBalance.airHumRatTemp = humidityRatio;

    auto &zone = state.dataHeatBal->Zone(zoneNum);
    for (int spaceNum : zone.spaceIndexes) {
        auto &spaceHeatBalance = state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum);
        spaceHeatBalance.airHumRatAvg = humidityRatio;
        spaceHeatBalance.airHumRat = humidityRatio;
        spaceHeatBalance.airHumRatTemp = humidityRatio;
    }
}

Real64 ZoneHumidityRatio(EnergyPlus::EnergyPlusData &state, int zoneNum)
{
    if (!validZoneIndex(state, zoneNum)) {
        return 0.0;
    }

    return state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum).airHumRatAvg;
}

int VariableHandle(EnergyPlus::EnergyPlusData &state, std::string_view name, std::string_view key)
{
    auto const handle = ::getVariableHandle(static_cast<EnergyPlusState>(&state), std::string(name).c_str(), std::string(key).c_str());
    if (handle == -1) {
        throw std::runtime_error(fmt::format("Attempt to get invalid variable using name '{}' and key '{}'", name, key));
    }

    return handle;
}

Real64 VariableValue(EnergyPlus::EnergyPlusData &state, int handle)
{
    return ::getVariableValue(static_cast<EnergyPlusState>(&state), handle);
}

int ActuatorHandle(EnergyPlus::EnergyPlusData &state,
                   const std::string &componentType,
                   const std::string &controlType,
                   const std::string &componentName)
{
    auto const handle = ::getActuatorHandle(static_cast<EnergyPlusState>(&state),
                                            componentType.c_str(),
                                            controlType.c_str(),
                                            componentName.c_str());
    if (handle == -1) {
        throw std::runtime_error(fmt::format("Attempt to get invalid actuator using component type '{}', component name '{}', and control type '{}'",
                                             componentType,
                                             componentName,
                                             controlType));
    }

    return handle;
}

void SetActuatorValue(EnergyPlus::EnergyPlusData &state, int handle, Real64 value)
{
    ::setActuatorValue(static_cast<EnergyPlusState>(&state), handle, value);
}

void ResetActuator(EnergyPlus::EnergyPlusData &state, int handle)
{
    ::resetActuator(static_cast<EnergyPlusState>(&state), handle);
}

int SurfaceNum(EnergyPlus::EnergyPlusData &state, std::string_view surfaceName)
{
    std::string const upperName = upperCase(surfaceName);

    for (int surfaceNumber : state.dataSurface->AllHTNonWindowSurfaceList) {
        if (state.dataSurface->Surface(surfaceNumber).Name == upperName) {
            return surfaceNumber;
        }
    }

    return 0;
}

Real64 SurfaceArea(EnergyPlus::EnergyPlusData &state, int surfaceNum)
{
    if (!validSurfaceIndex(state, surfaceNum)) {
        return 0.0;
    }

    return state.dataSurface->Surface(surfaceNum).GrossArea;
}

Real64 SurfaceInsideHeatFlow(EnergyPlus::EnergyPlusData &state, int surfaceNum)
{
    if (!validSurfaceIndex(state, surfaceNum)) {
        return 0.0;
    }

    return state.dataHeatBalSurf->SurfQdotConvInRep(surfaceNum) +
           state.dataHeatBalSurf->SurfQdotRadNetSurfInRep(surfaceNum);
}

Real64 SurfaceOutsideHeatFlow(EnergyPlus::EnergyPlusData &state, int surfaceNum)
{
    if (!validSurfaceIndex(state, surfaceNum)) {
        return 0.0;
    }

    auto const &surface = state.dataSurface->Surface(surfaceNum);
    int const extBoundCond = surface.ExtBoundCond;

    if (validSurfaceIndex(state, extBoundCond)) {
        return state.dataHeatBalSurf->SurfOpaqInsFaceCondFlux(surfaceNum) +
               state.dataHeatBalSurf->SurfOpaqInsFaceCondFlux(extBoundCond) +
               state.dataHeatBalSurf->SurfOpaqOutFaceCondFlux(surfaceNum) +
               state.dataHeatBalSurf->SurfOpaqOutFaceCondFlux(extBoundCond);
    }

    return state.dataHeatBalSurf->SurfOpaqOutFaceCondFlux(surfaceNum);
}

std::vector<int> InsideSurfaceTemperatureActuatorHandles(EnergyPlus::EnergyPlusData &state, std::string_view surfaceName)
{
    int const surfaceNumber = SurfaceNum(state, surfaceName);
    if (!validSurfaceIndex(state, surfaceNumber)) {
        return {};
    }

    auto const &surface = state.dataSurface->Surface(surfaceNumber);
    std::vector<int> handles;
    handles.push_back(ActuatorHandle(state, "Surface", "Surface Inside Temperature", surface.Name));

    int const extBoundCond = surface.ExtBoundCond;
    if (extBoundCond == surfaceNumber) {
        throw std::runtime_error(
            fmt::format("Attempt to control surface named {} that has a self referencing exterior boundary condition. This is not supported by Spawn",
                        surface.Name));
    }

    if (validSurfaceIndex(state, extBoundCond)) {
        auto &otherSurface = state.dataSurface->Surface(extBoundCond);
        handles.push_back(ActuatorHandle(state, "Surface", "Surface Outside Temperature", otherSurface.Name));
    }

    return handles;
}

std::vector<int> OutsideSurfaceTemperatureActuatorHandles(EnergyPlus::EnergyPlusData &state, std::string_view surfaceName)
{
    int const surfaceNumber = SurfaceNum(state, surfaceName);
    if (!validSurfaceIndex(state, surfaceNumber)) {
        return {};
    }

    auto const &surface = state.dataSurface->Surface(surfaceNumber);
    std::vector<int> handles;
    handles.push_back(ActuatorHandle(state, "Surface", "Surface Outside Temperature", surface.Name));

    int const extBoundCond = surface.ExtBoundCond;
    if (extBoundCond == surfaceNumber) {
        throw std::runtime_error(
            fmt::format("Attempt to control surface named {} that has a self referencing exterior boundary condition. This is not supported by Spawn",
                        surface.Name));
    }

    if (validSurfaceIndex(state, extBoundCond)) {
        auto &otherSurface = state.dataSurface->Surface(extBoundCond);
        handles.push_back(ActuatorHandle(state, "Surface", "Surface Inside Temperature", otherSurface.Name));
    }

    return handles;
}

void UpdateZoneTemperature(EnergyPlus::EnergyPlusData &state, int zoneNum, Real64 timeStepSeconds)
{
    if (!validZoneIndex(state, zoneNum)) {
        return;
    }

    auto &zoneHeatBalance = state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum);

    Real64 const currentZoneTemperature = ZoneTemperature(state, zoneNum);
    Real64 const airCapacity = state.dataHeatBal->Zone(zoneNum).Volume *
                               state.dataHeatBal->Zone(zoneNum).ZoneVolCapMultpSens *
                               EnergyPlus::Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                              state.dataEnvrn->OutBaroPress,
                                                                              currentZoneTemperature,
                                                                              zoneHeatBalance.airHumRat,
                                                                              "") *
                               EnergyPlus::Psychrometrics::PsyCpAirFnW(zoneHeatBalance.airHumRat);

    ZoneSums const sums(state, zoneNum);
    Real64 newZoneTemperature = currentZoneTemperature;

    if (sums.TempDepCoef() == 0.0) {
        newZoneTemperature += (sums.TempIndCoef() / airCapacity) * timeStepSeconds;
    } else {
        Real64 const exponent = std::min(700.0, -sums.TempDepCoef() / airCapacity * timeStepSeconds);
        newZoneTemperature =
            (currentZoneTemperature - (sums.TempIndCoef() / sums.TempDepCoef())) * std::exp(exponent) +
            (sums.TempIndCoef() / sums.TempDepCoef());
    }

    SetZoneTemperature(state, zoneNum, newZoneTemperature);
}

void UpdateZoneHumidityRatio(EnergyPlus::EnergyPlusData &state, int zoneNum, Real64 timeStepSeconds)
{
    if (!validZoneIndex(state, zoneNum)) {
        return;
    }

    auto &zoneHeatBalance = state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum);

    static constexpr std::string_view routineName("UpdateZoneHumidityRatio");
    Real64 const humidityRatio = ZoneHumidityRatio(state, zoneNum);

    Real64 const latentGain = zoneHeatBalance.latentGain + state.dataHeatBalFanSys->SumLatentHTRadSys(zoneNum) +
                              state.dataHeatBalFanSys->SumLatentPool(zoneNum);

    Real64 const airDensity = EnergyPlus::Psychrometrics::PsyRhoAirFnPbTdbW(
        state, state.dataEnvrn->OutBaroPress, zoneHeatBalance.ZT, zoneHeatBalance.airHumRat, routineName);
    Real64 const latentHeatOfVaporization = EnergyPlus::Psychrometrics::PsyHgAirFnWTdb(zoneHeatBalance.airHumRat, zoneHeatBalance.ZT);

    Real64 const B = (latentGain / latentHeatOfVaporization) +
                     ((zoneHeatBalance.OAMFL + zoneHeatBalance.VAMFL + zoneHeatBalance.CTMFL) * state.dataEnvrn->OutHumRat) +
                     zoneHeatBalance.EAMFLxHumRat + zoneHeatBalance.SumHmARaW +
                     zoneHeatBalance.MixingMassFlowXHumRat +
                     (zoneHeatBalance.MDotOA * state.dataEnvrn->OutHumRat);

    Real64 const C = airDensity * state.dataHeatBal->Zone(zoneNum).Volume *
                     state.dataHeatBal->Zone(zoneNum).ZoneVolCapMultpMoist / timeStepSeconds;

    Real64 newHumidityRatio = humidityRatio + (B / C);
    newHumidityRatio = std::max(newHumidityRatio, 0.0);

    Real64 const saturationHumidityRatio = EnergyPlus::Psychrometrics::PsyWFnTdbRhPb(
        state, zoneHeatBalance.ZT, 1.0, state.dataEnvrn->OutBaroPress, routineName);

    newHumidityRatio = std::min(newHumidityRatio, saturationHumidityRatio);

    SetZoneHumidityRatio(state, zoneNum, newHumidityRatio);
}

void UpdateLatentGains(EnergyPlus::EnergyPlusData &state)
{
    for (int zoneIndex = 1; zoneIndex <= state.dataGlobal->NumOfZones; ++zoneIndex) {
        EnergyPlus::InternalHeatGains::SumAllInternalLatentGains(state, zoneIndex);
    }
}

} // namespace EnergyPlus::Spawn
