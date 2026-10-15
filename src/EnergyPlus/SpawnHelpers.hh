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

#ifndef ENERGYPLUS_SPAWNHELPERS_HH
#define ENERGYPLUS_SPAWNHELPERS_HH

// C++ Headers
#include <string>
#include <string_view>
#include <vector>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/api/TypeDefs.h>

namespace EnergyPlus::Spawn {

class ZoneSums
{
public:
    ZoneSums(EnergyPlus::EnergyPlusData &state, int zoneNum);

    [[nodiscard]] Real64 TempDepCoef() const;
    [[nodiscard]] Real64 TempIndCoef() const;
    [[nodiscard]] Real64 QConSenFlow() const;

private:
    Real64 m_tempDepCoef;
    Real64 m_tempIndCoef;
    Real64 m_qConSenFlow;
};

[[nodiscard]] int ZoneNum(EnergyPlus::EnergyPlusData &state, std::string_view zoneName);

[[nodiscard]] std::vector<int> ZoneNums(EnergyPlus::EnergyPlusData &state, const std::vector<std::string> &zoneNames);

[[nodiscard]] Real64 ZoneVolume(EnergyPlus::EnergyPlusData &state, int zoneNum);

[[nodiscard]] Real64 ZoneFloorArea(EnergyPlus::EnergyPlusData &state, int zoneNum);

[[nodiscard]] Real64 ZoneVolCapMultpSens(EnergyPlus::EnergyPlusData &state, int zoneNum);

[[nodiscard]] Real64 ZoneLatentGain(EnergyPlus::EnergyPlusData &state, int zoneNum);

[[nodiscard]] Real64 ZoneMeanRadiantTemp(EnergyPlus::EnergyPlusData &state, int zoneNum);

[[nodiscard]] bool HaveSizingInfo(EnergyPlus::EnergyPlusData &state);

namespace zone_sizing {

    [[nodiscard]] Real64 SensibleCoolingLoad(EnergyPlus::EnergyPlusData &state, int zoneNum);

    [[nodiscard]] Real64 LatentCoolingLoad(EnergyPlus::EnergyPlusData &state, int zoneNum);

    [[nodiscard]] Real64 OutdoorTempAtPeakCool(EnergyPlus::EnergyPlusData &state, int zoneNum);

    [[nodiscard]] Real64 OutdoorHumidityRatioAtPeakCool(EnergyPlus::EnergyPlusData &state, int zoneNum);

    [[nodiscard]] Real64 TimeAtPeakCool(EnergyPlus::EnergyPlusData &state, int zoneNum);

    [[nodiscard]] Real64 HeatingLoad(EnergyPlus::EnergyPlusData &state, int zoneNum);

    [[nodiscard]] Real64 OutdoorTempAtPeakHeat(EnergyPlus::EnergyPlusData &state, int zoneNum);

    [[nodiscard]] Real64 OutdoorHumidityRatioAtPeakHeat(EnergyPlus::EnergyPlusData &state, int zoneNum);

    [[nodiscard]] Real64 TimeAtPeakHeat(EnergyPlus::EnergyPlusData &state, int zoneNum);

    [[nodiscard]] Real64 MinCoolOA(EnergyPlus::EnergyPlusData &state, int zoneNum);

    [[nodiscard]] Real64 MinHeatOA(EnergyPlus::EnergyPlusData &state, int zoneNum);

} // namespace zone_sizing

namespace zone_group_sizing {

    [[nodiscard]] Real64 SensibleCoolingLoad(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers);

    [[nodiscard]] Real64 LatentCoolingLoad(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers);

    [[nodiscard]] Real64 OutdoorTempAtPeakCool(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers);

    [[nodiscard]] Real64 OutdoorHumidityRatioAtPeakCool(EnergyPlus::EnergyPlusData &state,
                                                        const std::vector<int> &zoneNumbers);

    [[nodiscard]] Real64 TimeAtPeakCool(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers);

    [[nodiscard]] Real64 HeatingLoad(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers);

    [[nodiscard]] Real64 OutdoorTempAtPeakHeat(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers);

    [[nodiscard]] Real64 OutdoorHumidityRatioAtPeakHeat(EnergyPlus::EnergyPlusData &state,
                                                        const std::vector<int> &zoneNumbers);

    [[nodiscard]] Real64 TimeAtPeakHeat(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers);

    [[nodiscard]] Real64 MinCoolOA(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers);

    [[nodiscard]] Real64 MinHeatOA(EnergyPlus::EnergyPlusData &state, const std::vector<int> &zoneNumbers);

} // namespace zone_group_sizing

void SetZoneTemperature(EnergyPlus::EnergyPlusData &state, int zoneNum, Real64 temperature);

Real64 ZoneTemperature(EnergyPlus::EnergyPlusData &state, int zoneNum);

void SetZoneHumidityRatio(EnergyPlus::EnergyPlusData &state, int zoneNum, Real64 humidityRatio);

Real64 ZoneHumidityRatio(EnergyPlus::EnergyPlusData &state, int zoneNum);

[[nodiscard]] int VariableHandle(EnergyPlus::EnergyPlusData &state, std::string_view name, std::string_view key);

[[nodiscard]] Real64 VariableValue(EnergyPlus::EnergyPlusData &state, int handle);

[[nodiscard]] int ActuatorHandle(EnergyPlus::EnergyPlusData &state,
                                 const std::string &componentType,
                                 const std::string &controlType,
                                 const std::string &componentName);

void SetActuatorValue(EnergyPlus::EnergyPlusData &state, int handle, Real64 value);

void ResetActuator(EnergyPlus::EnergyPlusData &state, int handle);

[[nodiscard]] int SurfaceNum(EnergyPlus::EnergyPlusData &state, std::string_view surfaceName);

[[nodiscard]] Real64 SurfaceArea(EnergyPlus::EnergyPlusData &state, int surfaceNum);

[[nodiscard]] Real64 SurfaceInsideHeatFlow(EnergyPlus::EnergyPlusData &state, int surfaceNum);

[[nodiscard]] Real64 SurfaceOutsideHeatFlow(EnergyPlus::EnergyPlusData &state, int surfaceNum);

std::vector<int> InsideSurfaceTemperatureActuatorHandles(EnergyPlus::EnergyPlusData &state, std::string_view surfaceName);

std::vector<int> OutsideSurfaceTemperatureActuatorHandles(EnergyPlus::EnergyPlusData &state, std::string_view surfaceName);

void UpdateZoneTemperature(EnergyPlus::EnergyPlusData &state, int zoneNum, Real64 timeStepSeconds);

void UpdateZoneHumidityRatio(EnergyPlus::EnergyPlusData &state, int zoneNum, Real64 timeStepSeconds);

void UpdateLatentGains(EnergyPlus::EnergyPlusData &state);

} // namespace EnergyPlus::Spawn

#endif // ENERGYPLUS_SPAWNHELPERS_HH
