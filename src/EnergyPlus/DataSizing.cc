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

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SizingManager.hh>

namespace EnergyPlus::DataSizing {

// MODULE INFORMATION:
//       AUTHOR         Fred Buhl
//       DATE WRITTEN   December 2000

// PURPOSE OF THIS MODULE:
// This data-only module contains type definitions and variables
// associated with HVAC system design flow rates, temperatures and
// capacities. This data is available to the HVAC component modules
// for their self sizing calculations.

//  days; includes effects of user multiplier
//  and user set flows)
//  of user input multiplier and flows
//  all design days, calculated only)
//  using user input system flow rates.
//  before applying user input sys flow rates.

Real64 TermUnitSizingData::applyTermUnitSizingCoolFlow(Real64 const coolFlowWithOA, // Cooling flow rate with MinOA limit applied
                                                       Real64 const coolFlowNoOA    // Cooling flow rate without MinOA limit applied
)
{
    // Apply DesignSpecification:AirTerminal:Sizing to cooling flow (could be vol flow or mass flow)
    Real64 coolFlowRatio = 1.0;
    if (this->SpecDesCoolSATRatio > 0.0) {
        coolFlowRatio = this->SpecDesSensCoolingFrac / this->SpecDesCoolSATRatio;
    } else {
        coolFlowRatio = this->SpecDesSensCoolingFrac;
    }
    Real64 adjustedFlow = coolFlowNoOA * coolFlowRatio + (coolFlowWithOA - coolFlowNoOA) * this->SpecMinOAFrac;
    return adjustedFlow;
}

Real64 TermUnitSizingData::applyTermUnitSizingHeatFlow(Real64 const heatFlowWithOA, // Heating flow rate with MinOA limit applied
                                                       Real64 const heatFlowNoOA    // Heating flow rate without MinOA limit applied
)
{
    // Apply DesignSpecification:AirTerminal:Sizing to heating flow (could be vol flow or mass flow)
    Real64 heatFlowRatio = 1.0;
    if (this->SpecDesHeatSATRatio > 0.0) {
        heatFlowRatio = this->SpecDesSensHeatingFrac / this->SpecDesHeatSATRatio;
    } else {
        heatFlowRatio = this->SpecDesSensHeatingFrac;
    }
    Real64 adjustedFlow = heatFlowNoOA * heatFlowRatio + (heatFlowWithOA - heatFlowNoOA) * this->SpecMinOAFrac;
    return adjustedFlow;
}

void TermUnitZoneSizingData::scaleZoneCooling(Real64 const ratio)
{
    // Apply scaling ratio to TermUnitFinalZoneSizing cooling flow and load
    this->DesCoolVolFlow *= ratio;
    this->DesCoolMassFlow *= ratio;
    this->DesCoolLoad *= ratio;
    for (auto &cfs : this->CoolFlowSeq) {
        cfs *= ratio;
    }
}

void TermUnitZoneSizingData::scaleZoneHeating(Real64 const ratio)
{
    // Apply scaling ratio to TermUnitFinalZoneSizing heating flow and load
    this->DesHeatVolFlow *= ratio;
    this->DesHeatMassFlow *= ratio;
    this->DesHeatLoad *= ratio;
    for (auto &cfs : this->HeatFlowSeq) {
        cfs *= ratio;
    }
}

void ZoneSizingData::zeroMemberData()
{
    if (!allocated(this->DOASSupMassFlowSeq)) {
        return;
    }
    std::fill(this->DOASSupMassFlowSeq.begin(), this->DOASSupMassFlowSeq.end(), 0.0);
    std::fill(this->DOASHeatLoadSeq.begin(), this->DOASHeatLoadSeq.end(), 0.0);
    std::fill(this->DOASCoolLoadSeq.begin(), this->DOASCoolLoadSeq.end(), 0.0);
    std::fill(this->DOASHeatAddSeq.begin(), this->DOASHeatAddSeq.end(), 0.0);
    std::fill(this->DOASLatAddSeq.begin(), this->DOASLatAddSeq.end(), 0.0);
    std::fill(this->DOASSupTempSeq.begin(), this->DOASSupTempSeq.end(), 0.0);
    std::fill(this->DOASSupHumRatSeq.begin(), this->DOASSupHumRatSeq.end(), 0.0);
    std::fill(this->DOASTotCoolLoadSeq.begin(), this->DOASTotCoolLoadSeq.end(), 0.0);
    std::fill(this->HeatFlowSeq.begin(), this->HeatFlowSeq.end(), 0.0);
    std::fill(this->HeatFlowSeqNoOA.begin(), this->HeatFlowSeqNoOA.end(), 0.0);
    std::fill(this->HeatLoadSeq.begin(), this->HeatLoadSeq.end(), 0.0);
    std::fill(this->HeatZoneTempSeq.begin(), this->HeatZoneTempSeq.end(), 0.0);
    std::fill(this->DesHeatSetPtSeq.begin(), this->DesHeatSetPtSeq.end(), 0.0);
    std::fill(this->HeatOutTempSeq.begin(), this->HeatOutTempSeq.end(), 0.0);
    std::fill(this->HeatZoneRetTempSeq.begin(), this->HeatZoneRetTempSeq.end(), 0.0);
    std::fill(this->HeatTstatTempSeq.begin(), this->HeatTstatTempSeq.end(), 0.0);
    std::fill(this->HeatZoneHumRatSeq.begin(), this->HeatZoneHumRatSeq.end(), 0.0);
    std::fill(this->HeatOutHumRatSeq.begin(), this->HeatOutHumRatSeq.end(), 0.0);
    std::fill(this->CoolFlowSeq.begin(), this->CoolFlowSeq.end(), 0.0);
    std::fill(this->CoolFlowSeqNoOA.begin(), this->CoolFlowSeqNoOA.end(), 0.0);
    std::fill(this->CoolLoadSeq.begin(), this->CoolLoadSeq.end(), 0.0);
    std::fill(this->CoolZoneTempSeq.begin(), this->CoolZoneTempSeq.end(), 0.0);
    std::fill(this->DesCoolSetPtSeq.begin(), this->DesCoolSetPtSeq.end(), 0.0);
    std::fill(this->CoolOutTempSeq.begin(), this->CoolOutTempSeq.end(), 0.0);
    std::fill(this->CoolZoneRetTempSeq.begin(), this->CoolZoneRetTempSeq.end(), 0.0);
    std::fill(this->CoolTstatTempSeq.begin(), this->CoolTstatTempSeq.end(), 0.0);
    std::fill(this->CoolZoneHumRatSeq.begin(), this->CoolZoneHumRatSeq.end(), 0.0);
    std::fill(this->CoolOutHumRatSeq.begin(), this->CoolOutHumRatSeq.end(), 0.0);
    std::fill(this->HeatLoadNoDOASSeq.begin(), this->HeatLoadNoDOASSeq.end(), 0.0);
    std::fill(this->CoolLoadNoDOASSeq.begin(), this->CoolLoadNoDOASSeq.end(), 0.0);
    std::fill(this->LatentHeatLoadSeq.begin(), this->LatentHeatLoadSeq.end(), 0.0);
    std::fill(this->LatentCoolLoadSeq.begin(), this->LatentCoolLoadSeq.end(), 0.0);
    std::fill(this->HeatLatentLoadNoDOASSeq.begin(), this->HeatLatentLoadNoDOASSeq.end(), 0.0);
    std::fill(this->CoolLatentLoadNoDOASSeq.begin(), this->CoolLatentLoadNoDOASSeq.end(), 0.0);
    std::fill(this->LatentCoolFlowSeq.begin(), this->LatentCoolFlowSeq.end(), 0.0);
    std::fill(this->LatentHeatFlowSeq.begin(), this->LatentHeatFlowSeq.end(), 0.0);

    this->CoolDesDay = "";          // name of a sensible cooling design day
    this->HeatDesDay = "";          // name of a sensible heating design day
    this->CoolNoDOASDesDay = "";    // name of a sensible cooling design day without DOAS
    this->HeatNoDOASDesDay = "";    // name of a sensible heating design day without DOAS
    this->LatCoolDesDay = "";       // name of a latent cooling design day
    this->LatHeatDesDay = "";       // name of a latent heating design day
    this->LatCoolNoDOASDesDay = ""; // name of a latent cooling design day without DOAS
    this->LatHeatNoDOASDesDay = ""; // name of a latent heating design day without DOAS

    this->DesHeatMassFlow = 0.0;                // zone design heating air mass flow rate [kg/s]
    this->DesCoolMassFlow = 0.0;                // zone design cooling air mass flow rate [kg/s]
    this->DesHeatLoad = 0.0;                    // zone design heating load [W]
    this->DesCoolLoad = 0.0;                    // zone design cooling load [W]
    this->DesHeatDens = 0.0;                    // zone design heating air density [kg/m3]
    this->DesCoolDens = 0.0;                    // zone design cooling air density [kg/m3]
    this->DesHeatVolFlow = 0.0;                 // zone design heating air volume flow rate [m3/s]
    this->DesCoolVolFlow = 0.0;                 // zone design cooling air volume flow rate [m3/s]
    this->DesHeatVolFlowMax = 0.0;              // zone design heating maximum air volume flow rate [m3/s]
    this->DesCoolVolFlowMin = 0.0;              // zone design cooling minimum air volume flow rate [m3/s]
    this->DesHeatCoilInTemp = 0.0;              // zone heating coil design air inlet temperature [C]
    this->DesCoolCoilInTemp = 0.0;              // zone cooling coil design air inlet temperature [C]
    this->DesHeatCoilInHumRat = 0.0;            // zone heating coil design air inlet humidity ratio [kg/kg]
    this->DesCoolCoilInHumRat = 0.0;            // zone cooling coil design air inlet humidity ratio [kg/kg]
    this->DesHeatCoilInTempTU = 0.0;            // zone heating coil design air inlet temperature (supply air)([C]
    this->DesCoolCoilInTempTU = 0.0;            // zone cooling coil design air inlet temperature (supply air)[C]
    this->DesHeatCoilInHumRatTU = 0.0;          // zone heating coil design air inlet humidity ratio
    this->DesCoolCoilInHumRatTU = 0.0;          // zone cooling coil design air inlet humidity ratio
    this->HeatMassFlow = 0.0;                   // current zone heating air mass flow rate (HVAC time step)
    this->CoolMassFlow = 0.0;                   // current zone cooling air mass flow rate (HVAC time step)
    this->HeatLoad = 0.0;                       // current zone heating load (HVAC time step)
    this->CoolLoad = 0.0;                       // current zone heating load (HVAC time step)
    this->HeatZoneTemp = 0.0;                   // current zone temperature (heating, time step)
    this->HeatOutTemp = 0.0;                    // current outdoor temperature (heating, time step)
    this->HeatZoneRetTemp = 0.0;                // current zone return temperature (heating, time step)
    this->HeatTstatTemp = 0.0;                  // current zone thermostat temperature (heating, time step)
    this->CoolZoneTemp = 0.0;                   // current zone temperature (cooling, time step)
    this->CoolOutTemp = 0.0;                    // current Outdoor temperature (cooling, time step)
    this->CoolZoneRetTemp = 0.0;                // current zone return temperature (cooling, time step)
    this->CoolTstatTemp = 0.0;                  // current zone thermostat temperature (cooling, time step)
    this->HeatZoneHumRat = 0.0;                 // current zone humidity ratio (heating, time step)
    this->CoolZoneHumRat = 0.0;                 // current zone humidity ratio (cooling, time step)
    this->HeatOutHumRat = 0.0;                  // current outdoor humidity ratio (heating, time step)
    this->CoolOutHumRat = 0.0;                  // current outdoor humidity ratio (cooling, time step)
    this->ZoneTempAtHeatPeak = 0.0;             // zone temp at max heating [C]
    this->ZoneRetTempAtHeatPeak = 0.0;          // zone return temp at max heating [C]
    this->OutTempAtHeatPeak = 0.0;              // outdoor temperature at max heating [C]
    this->ZoneTempAtCoolPeak = 0.0;             // zone temp at max cooling [C]
    this->ZoneRetTempAtCoolPeak = 0.0;          // zone return temp at max cooling [C]
    this->OutTempAtCoolPeak = 0.0;              // outdoor temperature at max cooling [C]
    this->ZoneHumRatAtHeatPeak = 0.0;           // zone humidity ratio at max heating [kg/kg]
    this->ZoneHumRatAtCoolPeak = 0.0;           // zone humidity ratio at max cooling [kg/kg]
    this->OutHumRatAtHeatPeak = 0.0;            // outdoor humidity at max heating [kg/kg]
    this->OutHumRatAtCoolPeak = 0.0;            // outdoor humidity at max cooling [kg/kg]
    this->TimeStepNumAtHeatMax = 0;             // time step number (in day) at Heating peak
    this->TimeStepNumAtCoolMax = 0;             // time step number (in day) at cooling peak
    this->HeatDDNum = 0;                        // design day index of design day causing heating peak
    this->CoolDDNum = 0;                        // design day index of design day causing heating peak
    this->LatentHeatDDNum = 0;                  // design day index of design day causing heating peak
    this->LatentCoolDDNum = 0;                  // design day index of design day causing cooling peak
    this->LatentHeatNoDOASDDNum = 0;            // design day index of design day causing latent heating peak without DOAS
    this->LatentCoolNoDOASDDNum = 0;            // design day index of design day causing latent cooling peak without DOAS
    this->cHeatDDDate = "";                     // date of design day causing heating peak
    this->cCoolDDDate = "";                     // date of design day causing cooling peak
    this->cLatentHeatDDDate = "";               // date of design day causing heating peak
    this->cLatentCoolDDDate = "";               // date of design day causing cooling peak
    this->DOASHeatLoad = 0.0;                   // current heating load from DOAS supply air [W]
    this->DOASCoolLoad = 0.0;                   // current cooling load from DOAS supply air [W]
    this->DOASSupMassFlow = 0.0;                // current mass flow rate of DOAS supply air [kg/s]
    this->DOASSupTemp = 0.0;                    // current DOAS supply air temperature [C]
    this->DOASSupHumRat = 0.0;                  // current DOAS supply air humidity ratio [kgWater/kgDryAir]
    this->DOASTotCoolLoad = 0.0;                // current total cooling load imposed by DOAS supply air [W]
    this->HeatLoadNoDOAS = 0.0;                 // current zone heating load no DOAS (HVAC time step)
    this->CoolLoadNoDOAS = 0.0;                 // current zone heating load no DOAS (HVAC time step)
    this->HeatLatentLoad = 0.0;                 // current zone humidification load (HVAC time step)
    this->CoolLatentLoad = 0.0;                 // current zone dehumidification load (HVAC time step)
    this->HeatLatentLoadNoDOAS = 0.0;           // current zone humidification load without DOAS (HVAC time step)
    this->CoolLatentLoadNoDOAS = 0.0;           // current zone dehumidification load without DOAS (HVAC time step)
    this->ZoneHeatLatentMassFlow = 0.0;         // current mass flow rate required to meet humidification load [kg/s]
    this->ZoneCoolLatentMassFlow = 0.0;         // current mass flow rate required to meet dehumidification load [kg/s]
    this->ZoneHeatLatentVolFlow = 0.0;          // current volume flow rate required to meet humidification load [m3/s]
    this->ZoneCoolLatentVolFlow = 0.0;          // current volume flow rate required to meet dehumidification load [m3/s]
    this->DesHeatLoadNoDOAS = 0.0;              // zone design heating load without DOAS [W]
    this->DesCoolLoadNoDOAS = 0.0;              // zone design cooling load without DOAS [W]
    this->DesLatentHeatLoad = 0.0;              // current zone humidification load (HVAC time step)
    this->DesLatentCoolLoad = 0.0;              // current zone dehumidification load (HVAC time step)
    this->DesLatentHeatLoadNoDOAS = 0.0;        // current zone humidification load no DOAS (HVAC time step)
    this->DesLatentCoolLoadNoDOAS = 0.0;        // current zone dehumidification load no DOAS (HVAC time step)
    this->DesLatentHeatMassFlow = 0.0;          // current mass flow rate required to meet humidification load [kg/s]
    this->DesLatentCoolMassFlow = 0.0;          // current mass flow rate required to meet dehumidification load [kg/s]
    this->DesLatentHeatVolFlow = 0.0;           // current volume flow rate required to meet humidification load [m3/s]
    this->DesLatentCoolVolFlow = 0.0;           // current volume flow rate required to meet dehumidification load [m3/s]
    this->DesLatentHeatCoilInTemp = 0.0;        // zone heating coil design air inlet temperature [C]
    this->DesLatentCoolCoilInTemp = 0.0;        // zone cooling coil design air inlet temperature [C]
    this->DesLatentHeatCoilInHumRat = 0.0;      // zone heating coil design air inlet humidity ratio [kg/kg]
    this->DesLatentCoolCoilInHumRat = 0.0;      // zone cooling coil design air inlet humidity ratio [kg/kg]
    this->TimeStepNumAtLatentHeatMax = 0;       // time step number (in day) at latent heating peak
    this->TimeStepNumAtLatentCoolMax = 0;       // time step number (in day) at latent cooling peak
    this->TimeStepNumAtLatentHeatNoDOASMax = 0; // time step number (in day) at latent heating peak without DOAS
    this->TimeStepNumAtLatentCoolNoDOASMax = 0; // time step number (in day) at latent cooling peak without DOAS
    this->OutTempAtLatentCoolPeak = 0.0;        // outdoor temperature at max latent cooling [C]
    this->OutHumRatAtLatentCoolPeak = 0.0;      // outdoor humrat at max latent cooling [C]
    this->OutTempAtLatentHeatPeak = 0.0;        // outdoor temperature at max latent heating [C]
    this->OutHumRatAtLatentHeatPeak = 0.0;      // outdoor humrat at max latent heating [C]
    this->ZoneRetTempAtLatentCoolPeak = 0.0;    // zone return temp at max cooling [C]
    this->ZoneRetTempAtLatentHeatPeak = 0.0;    // zone return temp at max heating [C]
}

void ZoneSizingData::allocateMemberArrays(int const numOfTimeStepInDay)
{
    this->HeatFlowSeq.dimension(numOfTimeStepInDay, 0.0);
    this->CoolFlowSeq.dimension(numOfTimeStepInDay, 0.0);
    this->HeatFlowSeqNoOA.dimension(numOfTimeStepInDay, 0.0);
    this->CoolFlowSeqNoOA.dimension(numOfTimeStepInDay, 0.0);
    this->HeatLoadSeq.dimension(numOfTimeStepInDay, 0.0);
    this->CoolLoadSeq.dimension(numOfTimeStepInDay, 0.0);
    this->HeatZoneTempSeq.dimension(numOfTimeStepInDay, 0.0);
    this->DesHeatSetPtSeq.dimension(numOfTimeStepInDay, 0.0);
    this->CoolZoneTempSeq.dimension(numOfTimeStepInDay, 0.0);
    this->DesCoolSetPtSeq.dimension(numOfTimeStepInDay, 0.0);
    this->HeatOutTempSeq.dimension(numOfTimeStepInDay, 0.0);
    this->CoolOutTempSeq.dimension(numOfTimeStepInDay, 0.0);
    this->HeatZoneRetTempSeq.dimension(numOfTimeStepInDay, 0.0);
    this->HeatTstatTempSeq.dimension(numOfTimeStepInDay, 0.0);
    this->CoolZoneRetTempSeq.dimension(numOfTimeStepInDay, 0.0);
    this->CoolTstatTempSeq.dimension(numOfTimeStepInDay, 0.0);
    this->HeatZoneHumRatSeq.dimension(numOfTimeStepInDay, 0.0);
    this->CoolZoneHumRatSeq.dimension(numOfTimeStepInDay, 0.0);
    this->HeatOutHumRatSeq.dimension(numOfTimeStepInDay, 0.0);
    this->CoolOutHumRatSeq.dimension(numOfTimeStepInDay, 0.0);
    this->DOASHeatLoadSeq.dimension(numOfTimeStepInDay, 0.0);
    this->DOASCoolLoadSeq.dimension(numOfTimeStepInDay, 0.0);
    this->DOASHeatAddSeq.dimension(numOfTimeStepInDay, 0.0);
    this->DOASLatAddSeq.dimension(numOfTimeStepInDay, 0.0);
    this->DOASSupMassFlowSeq.dimension(numOfTimeStepInDay, 0.0);
    this->DOASSupTempSeq.dimension(numOfTimeStepInDay, 0.0);
    this->DOASSupHumRatSeq.dimension(numOfTimeStepInDay, 0.0);
    this->DOASTotCoolLoadSeq.dimension(numOfTimeStepInDay, 0.0);
    this->HeatLoadNoDOASSeq.dimension(numOfTimeStepInDay, 0.0);
    this->CoolLoadNoDOASSeq.dimension(numOfTimeStepInDay, 0.0);
    this->LatentHeatLoadSeq.dimension(numOfTimeStepInDay, 0.0);
    this->LatentCoolLoadSeq.dimension(numOfTimeStepInDay, 0.0);
    this->HeatLatentLoadNoDOASSeq.dimension(numOfTimeStepInDay, 0.0);
    this->CoolLatentLoadNoDOASSeq.dimension(numOfTimeStepInDay, 0.0);
    this->LatentCoolFlowSeq.dimension(numOfTimeStepInDay, 0.0);
    this->LatentHeatFlowSeq.dimension(numOfTimeStepInDay, 0.0);
}

void TermUnitZoneSizingData::allocateMemberArrays(int const numOfTimeStepInDay)
{
    this->HeatFlowSeq.dimension(numOfTimeStepInDay, 0.0);
    this->CoolFlowSeq.dimension(numOfTimeStepInDay, 0.0);
    this->HeatFlowSeqNoOA.dimension(numOfTimeStepInDay, 0.0);
    this->CoolFlowSeqNoOA.dimension(numOfTimeStepInDay, 0.0);
    this->HeatZoneTempSeq.dimension(numOfTimeStepInDay, 0.0);
    this->HeatZoneRetTempSeq.dimension(numOfTimeStepInDay, 0.0);
    this->CoolZoneTempSeq.dimension(numOfTimeStepInDay, 0.0);
    this->CoolZoneRetTempSeq.dimension(numOfTimeStepInDay, 0.0);
}

void TermUnitZoneSizingData::copyFromZoneSizing(ZoneSizingData const &sourceData)
{
    this->ZoneName = sourceData.ZoneName;
    this->ADUName = sourceData.ADUName;
    this->CoolDesTemp = sourceData.CoolDesTemp;
    this->HeatDesTemp = sourceData.HeatDesTemp;
    this->CoolDesHumRat = sourceData.CoolDesHumRat;
    this->HeatDesHumRat = sourceData.HeatDesHumRat;
    this->DesOAFlowPPer = sourceData.DesOAFlowPPer;
    this->DesOAFlowPerArea = sourceData.DesOAFlowPerArea;
    this->DesCoolMinAirFlow = sourceData.DesCoolMinAirFlow;
    this->DesCoolMinAirFlowFrac = sourceData.DesCoolMinAirFlowFrac;
    this->DesHeatMaxAirFlow = sourceData.DesHeatMaxAirFlow;
    this->DesHeatMaxAirFlowFrac = sourceData.DesHeatMaxAirFlowFrac;
    this->ZoneNum = sourceData.ZoneNum;
    this->DesHeatMassFlow = sourceData.DesHeatMassFlow;
    this->DesHeatMassFlowNoOA = sourceData.DesHeatMassFlowNoOA;
    this->DesHeatOAFlowFrac = sourceData.DesHeatOAFlowFrac;
    this->DesCoolMassFlow = sourceData.DesCoolMassFlow;
    this->DesCoolMassFlowNoOA = sourceData.DesCoolMassFlowNoOA;
    this->DesCoolOAFlowFrac = sourceData.DesCoolOAFlowFrac;
    this->DesHeatLoad = sourceData.DesHeatLoad;
    this->NonAirSysDesHeatLoad = sourceData.NonAirSysDesHeatLoad;
    this->DesCoolLoad = sourceData.DesCoolLoad;
    this->NonAirSysDesCoolLoad = sourceData.NonAirSysDesCoolLoad;
    this->DesHeatVolFlow = sourceData.DesHeatVolFlow;
    this->DesHeatVolFlowNoOA = sourceData.DesHeatVolFlowNoOA;
    this->NonAirSysDesHeatVolFlow = sourceData.NonAirSysDesHeatVolFlow;
    this->DesCoolVolFlow = sourceData.DesCoolVolFlow;
    this->DesCoolVolFlowNoOA = sourceData.DesCoolVolFlowNoOA;
    this->NonAirSysDesCoolVolFlow = sourceData.NonAirSysDesCoolVolFlow;
    this->DesHeatVolFlowMax = sourceData.DesHeatVolFlowMax;
    this->DesCoolVolFlowMin = sourceData.DesCoolVolFlowMin;
    this->DesHeatCoilInTempTU = sourceData.DesHeatCoilInTempTU;
    this->DesCoolCoilInTempTU = sourceData.DesCoolCoilInTempTU;
    this->DesHeatCoilInHumRatTU = sourceData.DesHeatCoilInHumRatTU;
    this->DesCoolCoilInHumRatTU = sourceData.DesCoolCoilInHumRatTU;
    this->ZoneTempAtHeatPeak = sourceData.ZoneTempAtHeatPeak;
    this->ZoneRetTempAtHeatPeak = sourceData.ZoneRetTempAtHeatPeak;
    this->ZoneTempAtCoolPeak = sourceData.ZoneTempAtCoolPeak;
    this->ZoneRetTempAtCoolPeak = sourceData.ZoneRetTempAtCoolPeak;
    this->ZoneHumRatAtHeatPeak = sourceData.ZoneHumRatAtHeatPeak;
    this->ZoneHumRatAtCoolPeak = sourceData.ZoneHumRatAtCoolPeak;
    this->TimeStepNumAtHeatMax = sourceData.TimeStepNumAtHeatMax;
    this->TimeStepNumAtCoolMax = sourceData.TimeStepNumAtCoolMax;
    this->HeatDDNum = sourceData.HeatDDNum;
    this->CoolDDNum = sourceData.CoolDDNum;
    this->MinOA = sourceData.MinOA;
    this->DesCoolMinAirFlow2 = sourceData.DesCoolMinAirFlow2;
    this->DesHeatMaxAirFlow2 = sourceData.DesHeatMaxAirFlow2;
    for (size_t t = 1; t <= this->HeatFlowSeq.size(); ++t) {
        this->HeatFlowSeq(t) = sourceData.HeatFlowSeq(t);
        this->HeatFlowSeqNoOA(t) = sourceData.HeatFlowSeqNoOA(t);
        this->CoolFlowSeq(t) = sourceData.CoolFlowSeq(t);
        this->CoolFlowSeqNoOA(t) = sourceData.CoolFlowSeqNoOA(t);
        this->HeatZoneTempSeq(t) = sourceData.HeatZoneTempSeq(t);
        this->HeatZoneRetTempSeq(t) = sourceData.HeatZoneRetTempSeq(t);
        this->CoolZoneTempSeq(t) = sourceData.CoolZoneTempSeq(t);
        this->CoolZoneRetTempSeq(t) = sourceData.CoolZoneRetTempSeq(t);
    }
    this->ZoneADEffCooling = sourceData.ZoneADEffCooling;
    this->ZoneADEffHeating = sourceData.ZoneADEffHeating;
    this->ZoneSecondaryRecirculation = sourceData.ZoneSecondaryRecirculation;
    this->ZoneVentilationEff = sourceData.ZoneVentilationEff;
    this->ZonePrimaryAirFraction = sourceData.ZonePrimaryAirFraction;
    this->ZonePrimaryAirFractionHtg = sourceData.ZonePrimaryAirFractionHtg;
    this->ZoneOAFracCooling = sourceData.ZoneOAFracCooling;
    this->ZoneOAFracHeating = sourceData.ZoneOAFracHeating;
    this->TotalOAFromPeople = sourceData.TotalOAFromPeople;
    this->TotalOAFromArea = sourceData.TotalOAFromArea;
    this->TotPeopleInZone = sourceData.TotPeopleInZone;
    this->TotalZoneFloorArea = sourceData.TotalZoneFloorArea;
    this->SupplyAirAdjustFactor = sourceData.SupplyAirAdjustFactor;
    this->ZpzClgByZone = sourceData.ZpzClgByZone;
    this->ZpzHtgByZone = sourceData.ZpzHtgByZone;
    this->VozClgByZone = sourceData.VozClgByZone;
    this->VozHtgByZone = sourceData.VozHtgByZone;
    this->VpzMinByZoneSPSized = sourceData.VpzMinByZoneSPSized;
    this->ZoneSizThermSetPtHi = sourceData.ZoneSizThermSetPtHi;
    this->ZoneSizThermSetPtLo = sourceData.ZoneSizThermSetPtLo;
}

void resetHVACSizingGlobals(EnergyPlusData &state,
                            int const curZoneEqNum,
                            int const curSysNum,
                            bool &firstPassFlag) // called in zone equipment Report function
{
    // reset Data globals so that previously set variables are not used in other equipment models
    state.dataSize->DataTotCapCurveIndex = 0;
    state.dataSize->DataPltSizCoolNum = 0;
    state.dataSize->DataPltSizHeatNum = 0;
    state.dataSize->DataWaterLoopNum = 0;
    state.dataSize->DataCoilNum = 0;
    state.dataSize->DataFanOp = HVAC::FanOp::Invalid;
    state.dataSize->DataCoilIsSuppHeater = false;
    state.dataSize->DataIsDXCoil = false;
    state.dataSize->DataAutosizable = true;
    state.dataSize->DataEMSOverrideON = false;
    state.dataSize->DataScalableSizingON = false;
    state.dataSize->DataScalableCapSizingON = false;
    state.dataSize->DataSysScalableFlowSizingON = false;
    state.dataSize->DataSysScalableCapSizingON = false;
    state.dataSize->DataDesAccountForFanHeat = true;
    state.dataSize->DataDXCoolsLowSpeedsAutozize = false;

    state.dataSize->DataDesInletWaterTemp = 0.0;
    state.dataSize->DataDesInletAirHumRat = 0.0;
    state.dataSize->DataDesInletAirTemp = 0.0;
    state.dataSize->DataDesOutletAirTemp = 0.0;
    state.dataSize->DataDesOutletAirHumRat = 0.0;
    state.dataSize->DataCoolCoilCap = 0.0;
    state.dataSize->DataFlowUsedForSizing = 0.0;
    state.dataSize->DataAirFlowUsedForSizing = 0.0;
    state.dataSize->DataWaterFlowUsedForSizing = 0.0;
    state.dataSize->DataCapacityUsedForSizing = 0.0;
    state.dataSize->DataDesignCoilCapacity = 0.0;
    state.dataSize->DataHeatSizeRatio = 1.0;
    state.dataSize->DataEMSOverride = 0.0;
    state.dataSize->DataBypassFrac = 0.0;
    state.dataSize->DataFracOfAutosizedCoolingAirflow = 1.0;
    state.dataSize->DataFracOfAutosizedHeatingAirflow = 1.0;
    state.dataSize->DataFlowPerCoolingCapacity = 0.0;
    state.dataSize->DataFlowPerHeatingCapacity = 0.0;
    state.dataSize->DataFracOfAutosizedCoolingCapacity = 1.0;
    state.dataSize->DataFracOfAutosizedHeatingCapacity = 1.0;
    state.dataSize->DataAutosizedCoolingCapacity = 0.0;
    state.dataSize->DataAutosizedHeatingCapacity = 0.0;
    state.dataSize->DataConstantUsedForSizing = 0.0;
    state.dataSize->DataFractionUsedForSizing = 0.0;
    state.dataSize->DataNonZoneNonAirloopValue = 0.0;
    state.dataSize->DataZoneNumber = 0;
    state.dataSize->DataFanType = HVAC::FanType::Invalid;
    state.dataSize->DataFanIndex = 0;
    state.dataSize->DataWaterCoilSizCoolDeltaT = 0.0;
    state.dataSize->DataWaterCoilSizHeatDeltaT = 0.0;
    state.dataSize->DataNomCapInpMeth = false;
    state.dataSize->DataFanPlacement = HVAC::FanPlace::Invalid;
    state.dataSize->DataDXSpeedNum = 0;
    state.dataSize->DataCoilSizingAirInTemp = 0.0;
    state.dataSize->DataCoilSizingAirInHumRat = 0.0;
    state.dataSize->DataCoilSizingAirOutTemp = 0.0;
    state.dataSize->DataCoilSizingAirOutHumRat = 0.0;
    state.dataSize->DataCoolCoilType = -1;
    state.dataSize->DataCoolCoilIndex = -1;

    // These zone specific sizing variables are set in zone equipment to use for sizing.
    // Reset to avoid chance that second zone equipment will size using these variables set by first zone equipment to be sized
    if (curZoneEqNum > 0) {

        if (state.dataSize->ZoneEqSizing.size() == 0) {
            firstPassFlag = false;
            return;
        }

        auto &ZoneEqSizing = state.dataSize->ZoneEqSizing(curZoneEqNum);
        ZoneEqSizing.AirFlow = false;
        ZoneEqSizing.CoolingAirFlow = false;
        ZoneEqSizing.HeatingAirFlow = false;
        ZoneEqSizing.SystemAirFlow = false;
        ZoneEqSizing.Capacity = false;
        ZoneEqSizing.CoolingCapacity = false;
        ZoneEqSizing.HeatingCapacity = false;
        ZoneEqSizing.AirVolFlow = 0.0;
        ZoneEqSizing.MaxHWVolFlow = 0.0;
        ZoneEqSizing.MaxCWVolFlow = 0.0;
        ZoneEqSizing.OAVolFlow = 0.0;
        ZoneEqSizing.DesCoolingLoad = 0.0;
        ZoneEqSizing.DesHeatingLoad = 0.0;
        ZoneEqSizing.CoolingAirVolFlow = 0.0;
        ZoneEqSizing.HeatingAirVolFlow = 0.0;
        ZoneEqSizing.SystemAirVolFlow = 0.0;
        ZoneEqSizing.DesignSizeFromParent = false;
    }

    if (curSysNum > 0) {

        if (state.dataSize->UnitarySysEqSizing.size() == 0) {
            firstPassFlag = false;
            return;
        }

        auto &UnitarySysEqSizing = state.dataSize->UnitarySysEqSizing(curSysNum);
        UnitarySysEqSizing.AirFlow = false;
        UnitarySysEqSizing.CoolingAirFlow = false;
        UnitarySysEqSizing.HeatingAirFlow = false;
        UnitarySysEqSizing.Capacity = false;
        UnitarySysEqSizing.CoolingCapacity = false;
        UnitarySysEqSizing.HeatingCapacity = false;
    }

    firstPassFlag = false;
}

void GetCoilDesFlowT(EnergyPlusData &state,
                     int SysNum,           // central air system index
                     Real64 CpAir,         // specific heat to be used in calculations [J/kgC]
                     Real64 &DesFlow,      // returned design mass flow [kg/s]
                     Real64 &DesExitTemp,  // returned design coil exit temperature [kg/s]
                     Real64 &DesExitHumRat // returned design coil exit humidity ratio [kg/kg]
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   September 2014

    // PURPOSE OF THIS FUNCTION:
    // This function calculates the coil design air flow rate and exit temperature depending on the
    // cooling capacity control method

    // METHODOLOGY EMPLOYED:
    // energy and mass flow balance

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    int TimeStepAtPeak = 0;
    Real64 ZoneCoolLoadSum = 0; // sum of zone cooling loads at the peak [W]
    Real64 AvgZoneTemp = 0;     // average zone temperature [C]

    auto &finalSysSizing = state.dataSize->FinalSysSizing(SysNum);
    auto &sysSizPeakDDNum = state.dataSize->SysSizPeakDDNum(SysNum);
    auto &calcSysSizing = state.dataSize->CalcSysSizing(SysNum);

    int sysSizIndex = Util::FindItemInList(finalSysSizing.AirPriLoopName, state.dataSize->SysSizInput, &SystemSizingInputData::AirPriLoopName);
    if (sysSizIndex == 0) {
        sysSizIndex = 1;
    }
    auto &sysSizInput = state.dataSize->SysSizInput(sysSizIndex);

    if (sysSizPeakDDNum.SensCoolPeakDD > 0) {
        if (sysSizInput.coolingPeakLoad == PeakLoad::TotalCooling) {
            TimeStepAtPeak = sysSizPeakDDNum.TimeStepAtTotCoolPk(sysSizPeakDDNum.TotCoolPeakDD);
        } else {
            TimeStepAtPeak = sysSizPeakDDNum.TimeStepAtSensCoolPk(sysSizPeakDDNum.SensCoolPeakDD);
        }
    } else {
        if ((sysSizInput.CoolCapControl == CapacityControl::VT) || (sysSizInput.CoolCapControl == CapacityControl::Bypass)) {
            ShowWarningError(state,
                             format("GetCoilDesFlow: AirLoopHVAC = {} has no time of peak cooling load for sizing.", sysSizInput.AirPriLoopName));
            ShowContinueError(state, "Using Central Cooling Capacity Control Method=VAV instead of Bypass or VT.");
            sysSizInput.CoolCapControl = CapacityControl::VAV;
        }
    }

    if (sysSizInput.CoolCapControl == CapacityControl::VAV) {
        DesExitTemp = finalSysSizing.CoolSupTemp;
        DesFlow = finalSysSizing.MassFlowAtCoolPeak / state.dataEnvrn->StdRhoAir;
        DesExitHumRat = finalSysSizing.CoolSupHumRat;
    } else if (sysSizInput.CoolCapControl == CapacityControl::OnOff) {
        DesExitTemp = finalSysSizing.CoolSupTemp;
        DesFlow = state.dataSize->DataAirFlowUsedForSizing;
        DesExitHumRat = finalSysSizing.CoolSupHumRat;
    } else if (sysSizInput.CoolCapControl == CapacityControl::VT) {
        ZoneCoolLoadSum = calcSysSizing.SumZoneCoolLoadSeq(TimeStepAtPeak);
        AvgZoneTemp = calcSysSizing.CoolZoneAvgTempSeq(TimeStepAtPeak);
        DesExitTemp =
            max(finalSysSizing.CoolSupTemp, AvgZoneTemp - ZoneCoolLoadSum / (state.dataEnvrn->StdRhoAir * CpAir * finalSysSizing.DesCoolVolFlow));
        DesFlow = finalSysSizing.DesCoolVolFlow;
        DesExitHumRat = Psychrometrics::PsyWFnTdbRhPb(state, DesExitTemp, 0.9, state.dataEnvrn->StdBaroPress, "GetCoilDesFlowT");
    } else if (sysSizInput.CoolCapControl == CapacityControl::Bypass) {
        ZoneCoolLoadSum = calcSysSizing.SumZoneCoolLoadSeq(TimeStepAtPeak);
        AvgZoneTemp = calcSysSizing.CoolZoneAvgTempSeq(TimeStepAtPeak);
        DesExitTemp = finalSysSizing.CoolSupTemp;
        if (calcSysSizing.MixTempAtCoolPeak > DesExitTemp) {
            Real64 AvgSupTemp = AvgZoneTemp - ZoneCoolLoadSum / (state.dataEnvrn->StdRhoAir * CpAir * finalSysSizing.DesCoolVolFlow);
            DesFlow = finalSysSizing.DesCoolVolFlow *
                      max(0.0, min(1.0, ((calcSysSizing.MixTempAtCoolPeak - AvgSupTemp) / (calcSysSizing.MixTempAtCoolPeak - DesExitTemp))));
        } else {
            DesFlow = finalSysSizing.DesCoolVolFlow;
        }
        DesExitHumRat = Psychrometrics::PsyWFnTdbRhPb(state, DesExitTemp, 0.9, state.dataEnvrn->StdBaroPress, "GetCoilDesFlowT");
    }
}

Real64 ZoneAirDistributionData::calculateEz(EnergyPlusData &state, int const ZoneNum) // Zone index
{
    Real64 zoneEz = 1.0;
    // Calc the zone supplied OA flow rate counting the zone air distribution effectiveness
    //  First check whether the zone air distribution effectiveness schedule exists, if yes uses it;
    //   otherwise uses the inputs of zone distribution effectiveness in cooling mode or heating mode
    if (this->zoneADEffSched != nullptr) {
        // Get schedule value for the zone air distribution effectiveness
        zoneEz = this->zoneADEffSched->getCurrentVal();
    } else {
        Real64 zoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).TotalOutputRequired;

        // Zone in cooling mode
        if (zoneLoad < 0.0) {
            zoneEz = this->ZoneADEffCooling;
        }

        // Zone in heating mode
        if (zoneLoad > 0.0) {
            zoneEz = this->ZoneADEffHeating;
        }
    }
    if (zoneEz <= 0.0) {
        // Enforce defaults
        zoneEz = 1.0;
    }
    return zoneEz;
}

Real64
calcDesignSpecificationOutdoorAir(EnergyPlusData &state,
                                  int const DSOAPtr,           // Pointer to DesignSpecification:OutdoorAir object
                                  int const ActualZoneNum,     // Zone index
                                  bool const UseOccSchFlag,    // Zone occupancy schedule will be used instead of using total zone occupancy
                                  bool const UseMinOASchFlag,  // Use min OA schedule in DesignSpecification:OutdoorAir object
                                  bool const PerPersonNotSet,  // when calculation should not include occupants (e.g., dual duct)
                                  bool const MaxOAVolFlowFlag, // TRUE when calculation uses occupancy schedule  (e.g., dual duct)
                                  int const spaceNum,          // Space index (if applicable)
                                  bool const calcIAQMethods)   // For IAQProcedure, PCOccSch, and PCDesOcc, calculate if true, return zero if false
{
    Real64 totOAFlowRate = 0.0;
    if (DSOAPtr == 0) {
        return totOAFlowRate;
    }

    auto &thisDSOA = state.dataSize->OARequirements(DSOAPtr);

    if (thisDSOA.numDSOA == 0) {
        // This is a simple DesignSpecification:OutdoorAir
        return thisDSOA.calcOAFlowRate(
            state, ActualZoneNum, UseOccSchFlag, UseMinOASchFlag, PerPersonNotSet, MaxOAVolFlowFlag, spaceNum, calcIAQMethods);
    } else {
        // This is a DesignSpecification:OutdoorAir:SpaceList
        for (int dsoaCount = 1; dsoaCount <= thisDSOA.numDSOA; ++dsoaCount) {
            if ((spaceNum == 0) || ((spaceNum > 0) && (spaceNum == thisDSOA.dsoaSpaceIndexes(dsoaCount)))) {
                totOAFlowRate += state.dataSize->OARequirements(thisDSOA.dsoaIndexes(dsoaCount))
                                     .calcOAFlowRate(state,
                                                     ActualZoneNum,
                                                     UseOccSchFlag,
                                                     UseMinOASchFlag,
                                                     PerPersonNotSet,
                                                     MaxOAVolFlowFlag,
                                                     thisDSOA.dsoaSpaceIndexes(dsoaCount),
                                                     calcIAQMethods);
            }
        }
        return totOAFlowRate;
    }
}

void setHeatPumpSize(EnergyPlusData &state, Real64 &coolingCap, Real64 &heatingCap, Real64 const sizingRatio)
{
    state.dataSize->DataHeatSizeRatio = sizingRatio;
    if (state.dataSize->CurSysNum > 0) {
        auto const &finalSysSizing = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum);
        if (finalSysSizing.heatCoilSizingMethod == DataSizing::HeatCoilSizMethod::HeatingCapacity) {
            if (heatingCap > coolingCap * finalSysSizing.maxHeatCoilToCoolingLoadSizingRatio) {
                heatingCap = (coolingCap * finalSysSizing.maxHeatCoilToCoolingLoadSizingRatio) / sizingRatio;
                coolingCap = heatingCap;
            } else {
                coolingCap = heatingCap; // sizing ratio gets applied when coil sizes
            }
        } else if (finalSysSizing.heatCoilSizingMethod == DataSizing::HeatCoilSizMethod::GreaterOfHeatingOrCooling) {
            if (heatingCap > coolingCap * finalSysSizing.maxHeatCoilToCoolingLoadSizingRatio) {
                coolingCap = heatingCap / finalSysSizing.maxHeatCoilToCoolingLoadSizingRatio;
            } else if (heatingCap > coolingCap * sizingRatio) {
                coolingCap = heatingCap; // sizing ratio gets applied when coil sizes
            } else {
                heatingCap = coolingCap; // sizing ratio gets applied when coil sizes
            }
        } else if (finalSysSizing.heatCoilSizingMethod == DataSizing::HeatCoilSizMethod::CoolingCapacity) {
            if (heatingCap > coolingCap * finalSysSizing.maxHeatCoilToCoolingLoadSizingRatio) {
                heatingCap = (coolingCap * finalSysSizing.maxHeatCoilToCoolingLoadSizingRatio) / sizingRatio;
            } else {
                heatingCap = coolingCap; // sizing ratio gets applied when coil sizes
            }
        }
    } else if (state.dataSize->CurZoneEqNum > 0) {
        auto const &finalZoneSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum);
        if (finalZoneSizing.heatCoilSizingMethod == DataSizing::HeatCoilSizMethod::HeatingCapacity) {
            if (heatingCap > coolingCap * finalZoneSizing.maxHeatCoilToCoolingLoadSizingRatio) {
                heatingCap = (coolingCap * finalZoneSizing.maxHeatCoilToCoolingLoadSizingRatio) / sizingRatio;
                coolingCap = heatingCap;
            } else {
                coolingCap = heatingCap; // sizing ratio gets applied when coil sizes
            }
        } else if (finalZoneSizing.heatCoilSizingMethod == DataSizing::HeatCoilSizMethod::GreaterOfHeatingOrCooling) {
            if (heatingCap > coolingCap * finalZoneSizing.maxHeatCoilToCoolingLoadSizingRatio) {
                coolingCap = heatingCap / finalZoneSizing.maxHeatCoilToCoolingLoadSizingRatio;
            } else if (heatingCap > coolingCap * sizingRatio) {
                coolingCap = heatingCap / sizingRatio;
            } else {
                heatingCap = coolingCap; // sizing ratio gets applied when coil sizes
            }
        } else if (finalZoneSizing.heatCoilSizingMethod == DataSizing::HeatCoilSizMethod::CoolingCapacity) {
            if (heatingCap > coolingCap * finalZoneSizing.maxHeatCoilToCoolingLoadSizingRatio) {
                heatingCap = (coolingCap * finalZoneSizing.maxHeatCoilToCoolingLoadSizingRatio) / sizingRatio;
            } else if (heatingCap < coolingCap) {
                heatingCap = coolingCap; // sizing ratio gets applied when coil sizes
            }
        }
    }
}

int getDefaultOAReq(EnergyPlusData &state)
{
    if (state.dataSize->OARequirements_Default == 0) {
        SizingManager::GetOARequirements(state); // get the OA requirements object and set up the default
    }
    return state.dataSize->OARequirements_Default;
}

Real64 OARequirementsData::oaFlowArea(EnergyPlusData &state,
                                      int const zoneNum,
                                      bool const useMinOASchFlag, // Use min OA schedule in DesignSpecification:OutdoorAir object
                                      int const spaceNum)         // Space index (if applicable)
{
    Real64 sumAreaOA = 0.0; // OA Flow Rate based on area [m3/s]
    if (this->numDSOA == 0) {
        // This is a simple DesignSpecification:OutdoorAir
        if (this->OAFlowMethod != OAFlowCalcMethod::PerPerson && this->OAFlowMethod != OAFlowCalcMethod::PerZone &&
            this->OAFlowMethod != OAFlowCalcMethod::ACH) {
            if (spaceNum == 0) {
                sumAreaOA = state.dataHeatBal->Zone(zoneNum).FloorArea * this->OAFlowPerArea;
            } else {
                sumAreaOA = state.dataHeatBal->space(spaceNum).FloorArea * this->OAFlowPerArea;
            }
            if (useMinOASchFlag && (this->oaFlowFracSched != nullptr)) {
                sumAreaOA *= this->oaFlowFracSched->getCurrentVal();
            }
        }
    } else {
        // This is a DesignSpecification:OutdoorAir:SpaceList
        for (int dsoaCount = 1; dsoaCount <= this->numDSOA; ++dsoaCount) {
            auto const &thisDSOA = state.dataSize->OARequirements(this->dsoaIndexes(dsoaCount));
            int const dsoaSpaceNum = this->dsoaSpaceIndexes(dsoaCount);
            if (thisDSOA.OAFlowMethod != OAFlowCalcMethod::PerPerson && thisDSOA.OAFlowMethod != OAFlowCalcMethod::PerZone &&
                thisDSOA.OAFlowMethod != OAFlowCalcMethod::ACH) {
                if ((spaceNum == 0) || (spaceNum == dsoaSpaceNum)) {
                    Real64 spaceArea = state.dataHeatBal->space(this->dsoaSpaceIndexes(dsoaCount)).FloorArea;
                    Real64 spaceOAArea = thisDSOA.OAFlowPerArea * spaceArea;
                    if (useMinOASchFlag && (thisDSOA.oaFlowFracSched != nullptr)) {
                        spaceOAArea *= thisDSOA.oaFlowFracSched->getCurrentVal();
                    }
                    sumAreaOA += spaceOAArea;
                }
            }
        }
    }
    auto &thisZone = state.dataHeatBal->Zone(zoneNum);
    sumAreaOA = sumAreaOA * thisZone.Multiplier * thisZone.ListMultiplier;
    return sumAreaOA;
}

Real64 OARequirementsData::floorArea(EnergyPlusData &state, int const zoneNum,
                                     int const spaceNum) // Space index (if applicable)
{
    Real64 sumArea = 0.0; // Zone or space floor area [m2]
    if (this->numDSOA == 0) {
        // This is a simple DesignSpecification:OutdoorAir
        if (spaceNum == 0) {
            sumArea = state.dataHeatBal->Zone(zoneNum).FloorArea;
        } else {
            sumArea = state.dataHeatBal->space(spaceNum).FloorArea;
        }
    } else {
        // This is a DesignSpecification:OutdoorAir:SpaceList
        for (int dsoaCount = 1; dsoaCount <= this->numDSOA; ++dsoaCount) {
            int const dsoaSpaceNum = this->dsoaSpaceIndexes(dsoaCount);
            if ((spaceNum == 0) || (spaceNum == dsoaSpaceNum)) {
                Real64 spaceArea = state.dataHeatBal->space(this->dsoaSpaceIndexes(dsoaCount)).FloorArea;
                sumArea += spaceArea;
            }
        }
    }
    auto &thisZone = state.dataHeatBal->Zone(zoneNum);
    sumArea = sumArea * thisZone.Multiplier * thisZone.ListMultiplier;
    return sumArea;
}

Real64 OARequirementsData::desFlowPerZoneArea(EnergyPlusData &state, int const zoneNum, int const spaceNum)
{
    Real64 desFlowPA = 0.0;
    if (this->numDSOA == 0) {
        // This is a simple DesignSpecification:OutdoorAir
        if (this->OAFlowMethod != OAFlowCalcMethod::PerPerson && this->OAFlowMethod != OAFlowCalcMethod::PerZone &&
            this->OAFlowMethod != OAFlowCalcMethod::ACH) {
            desFlowPA = this->OAFlowPerArea;
        }
    } else {
        // This is a DesignSpecification:OutdoorAir:SpaceList
        bool useMinOASch = false;
        Real64 sumAreaOA = this->oaFlowArea(state, zoneNum, useMinOASch, spaceNum);
        Real64 sumArea = this->floorArea(state, zoneNum, spaceNum);
        if (sumArea > 0.0) {
            desFlowPA = sumAreaOA / sumArea;
        }
    }
    return desFlowPA;
}

Real64 OARequirementsData::oaFlowPeople(EnergyPlusData &state,
                                        int const zoneNum,          // Zone index
                                        bool const useOccSchFlag,   // Use occupancy schedule
                                        bool const useMinOASchFlag, // Use min OA schedule in DesignSpecification:OutdoorAir object
                                        int const spaceNum)         // Space index (if applicable)
{
    Real64 sumPeopleOA = 0.0; // OA Flow Rate based on people [m3/s]
    if (this->numDSOA == 0) {
        // This is a simple DesignSpecification:OutdoorAir
        if (this->OAFlowMethod != OAFlowCalcMethod::PerArea && this->OAFlowMethod != OAFlowCalcMethod::PerZone &&
            this->OAFlowMethod != OAFlowCalcMethod::ACH) {
            Real64 sumPeople = 0.0; // Zone or space number of people
            if (spaceNum == 0) {
                if (useOccSchFlag) {
                    sumPeople = state.dataHeatBal->ZoneIntGain(zoneNum).NOFOCC;
                } else {
                    sumPeople = state.dataHeatBal->Zone(zoneNum).TotOccupants;
                }
            } else {
                if (useOccSchFlag) {
                    sumPeople = state.dataHeatBal->space(spaceNum).TotOccupants;
                } else {
                    sumPeople = state.dataHeatBal->spaceIntGain(spaceNum).NOFOCC;
                }
            }
            sumPeopleOA = sumPeople * this->OAFlowPerPerson;
            // Apply schedule as needed. Sizing does not use schedule.
            if (useMinOASchFlag && (this->oaFlowFracSched != nullptr)) {
                sumPeopleOA *= this->oaFlowFracSched->getCurrentVal();
            }
        }
    } else {
        // This is a DesignSpecification:OutdoorAir:SpaceList
        for (int dsoaCount = 1; dsoaCount <= this->numDSOA; ++dsoaCount) {
            auto const &thisDSOA = state.dataSize->OARequirements(this->dsoaIndexes(dsoaCount));
            int const dsoaSpaceNum = this->dsoaSpaceIndexes(dsoaCount);
            if (thisDSOA.OAFlowMethod != OAFlowCalcMethod::PerArea && thisDSOA.OAFlowMethod != OAFlowCalcMethod::PerZone &&
                thisDSOA.OAFlowMethod != OAFlowCalcMethod::ACH) {
                if ((spaceNum == 0) || (spaceNum == dsoaSpaceNum)) {
                    Real64 spacePeople = 0.0;
                    if (useOccSchFlag) {
                        spacePeople = state.dataHeatBal->spaceIntGain(spaceNum).NOFOCC;
                    } else {
                        spacePeople = state.dataHeatBal->space(dsoaSpaceNum).TotOccupants;
                    }
                    sumPeopleOA += thisDSOA.OAFlowPerPerson * spacePeople;
                    if (useMinOASchFlag && (thisDSOA.oaFlowFracSched != nullptr)) {
                        sumPeopleOA *= thisDSOA.oaFlowFracSched->getCurrentVal();
                    }
                }
            }
        }
    }
    auto &thisZone = state.dataHeatBal->Zone(zoneNum);
    sumPeopleOA = sumPeopleOA * thisZone.Multiplier * thisZone.ListMultiplier;
    return sumPeopleOA;
}

Real64 OARequirementsData::people(EnergyPlusData &state,
                                  int const zoneNum,        // Zone index
                                  bool const useOccSchFlag, // Use zone occupancy schedule
                                  int const spaceNum)       // Space index (if applicable)
{
    Real64 sumPeople = 0.0; // Zone or space number of people
    if (this->numDSOA == 0) {
        // This is a simple DesignSpecification:OutdoorAir
        if (spaceNum == 0) {
            if (useOccSchFlag) {
                sumPeople = state.dataHeatBal->ZoneIntGain(zoneNum).NOFOCC;
            } else {
                sumPeople = state.dataHeatBal->Zone(zoneNum).TotOccupants;
            }
        } else {
            if (useOccSchFlag) {
                sumPeople = state.dataHeatBal->space(spaceNum).TotOccupants;
            } else {
                sumPeople = state.dataHeatBal->spaceIntGain(spaceNum).NOFOCC;
            }
        }
    } else {
        // This is a DesignSpecification:OutdoorAir:SpaceList
        for (int dsoaCount = 1; dsoaCount <= this->numDSOA; ++dsoaCount) {
            int const dsoaSpaceNum = this->dsoaSpaceIndexes(dsoaCount);
            if ((spaceNum == 0) || (spaceNum == dsoaSpaceNum)) {
                Real64 spacePeople = 0.0;
                if (useOccSchFlag) {
                    spacePeople = state.dataHeatBal->spaceIntGain(spaceNum).NOFOCC;
                } else {
                    spacePeople = state.dataHeatBal->space(dsoaSpaceNum).TotOccupants;
                }
                sumPeople += spacePeople;
            }
        }
    }
    auto &thisZone = state.dataHeatBal->Zone(zoneNum);
    sumPeople = sumPeople * thisZone.Multiplier * thisZone.ListMultiplier;
    return sumPeople;
}

Real64 OARequirementsData::desFlowPerZonePerson(EnergyPlusData &state, int const zoneNum, int const spaceNum)
{
    Real64 desFlowPP = 0.0;
    if (this->numDSOA == 0) {
        // This is a simple DesignSpecification:OutdoorAir
        if (this->OAFlowMethod != OAFlowCalcMethod::PerArea && this->OAFlowMethod != OAFlowCalcMethod::PerZone &&
            this->OAFlowMethod != OAFlowCalcMethod::ACH) {
            desFlowPP = this->OAFlowPerPerson;
        }
    } else {
        // This is a DesignSpecification:OutdoorAir:SpaceList
        bool useOccSch = false; // Use the design people value
        bool useMinOASch = false;
        Real64 sumPeopleOA = this->oaFlowPeople(state, zoneNum, useOccSch, useMinOASch, spaceNum);
        Real64 sumPeople = this->people(state, zoneNum, useOccSch, spaceNum);
        if (sumPeople > 0.0) {
            desFlowPP = sumPeopleOA / sumPeople;
        }
    }
    return desFlowPP;
}

Real64 OARequirementsData::desFlowPerZone(EnergyPlusData &state, int const spaceNum)
{
    Real64 desFlowPZ = 0.0; // [m3/s]
    if (this->numDSOA == 0) {
        // This is a simple DesignSpecification:OutdoorAir
        if (this->OAFlowMethod != OAFlowCalcMethod::PerPerson && this->OAFlowMethod != OAFlowCalcMethod::PerArea &&
            this->OAFlowMethod != OAFlowCalcMethod::ACH) {
            desFlowPZ = this->OAFlowPerZone;
        }
    } else {
        // This is a DesignSpecification:OutdoorAir:SpaceList
        for (int dsoaCount = 1; dsoaCount <= this->numDSOA; ++dsoaCount) {
            auto const &thisDSOA = state.dataSize->OARequirements(this->dsoaIndexes(dsoaCount));
            int const dsoaSpaceNum = this->dsoaSpaceIndexes(dsoaCount);
            if (thisDSOA.OAFlowMethod != OAFlowCalcMethod::PerPerson && thisDSOA.OAFlowMethod != OAFlowCalcMethod::PerArea &&
                thisDSOA.OAFlowMethod != OAFlowCalcMethod::ACH) {
                if ((spaceNum == 0) || (spaceNum == dsoaSpaceNum)) {
                    desFlowPZ += thisDSOA.OAFlowPerZone;
                }
            }
        }
    }
    return desFlowPZ;
}

Real64 OARequirementsData::desFlowPerACH(EnergyPlusData &state, int const spaceNum)
{
    Real64 desFlowPACH = 0.0; // [1/hr]
    if (this->numDSOA == 0) {
        // This is a simple DesignSpecification:OutdoorAir
        if (this->OAFlowMethod != OAFlowCalcMethod::PerPerson && this->OAFlowMethod != OAFlowCalcMethod::PerArea &&
            this->OAFlowMethod != OAFlowCalcMethod::PerZone) {
            desFlowPACH = this->OAFlowACH;
        }
    } else {
        // This is a DesignSpecification:OutdoorAir:SpaceList
        Real64 sumVolume = 0.0;
        Real64 sumACHFlow = 0.0;
        for (int dsoaCount = 1; dsoaCount <= this->numDSOA; ++dsoaCount) {
            auto const &thisDSOA = state.dataSize->OARequirements(this->dsoaIndexes(dsoaCount));
            int const dsoaSpaceNum = this->dsoaSpaceIndexes(dsoaCount);
            if (thisDSOA.OAFlowMethod != OAFlowCalcMethod::PerPerson && thisDSOA.OAFlowMethod != OAFlowCalcMethod::PerArea &&
                thisDSOA.OAFlowMethod != OAFlowCalcMethod::PerZone) {
                if ((spaceNum == 0) || (spaceNum == dsoaSpaceNum)) {
                    Real64 spaceVol = state.dataHeatBal->space(this->dsoaSpaceIndexes(dsoaCount)).Volume;
                    sumVolume += spaceVol;
                    Real64 spaceOAACH = thisDSOA.OAFlowACH * spaceVol;
                    sumACHFlow += spaceOAACH;
                }
            }
        }
        if (sumVolume > 0.0) {
            desFlowPACH = Constant::rSecsInHour * sumACHFlow / sumVolume;
        }
    }
    return desFlowPACH;
}

Real64
OARequirementsData::calcOAFlowRate(EnergyPlusData &state,
                                   int const ActualZoneNum,     // Zone index
                                   bool const UseOccSchFlag,    // Zone occupancy schedule will be used instead of using total zone occupancy
                                   bool const UseMinOASchFlag,  // Use min OA schedule in DesignSpecification:OutdoorAir object
                                   bool const PerPersonNotSet,  // when calculation should not include occupants (e.g., dual duct)
                                   bool const MaxOAVolFlowFlag, // TRUE when calculation uses occupancy schedule  (e.g., dual duct)
                                   int const spaceNum,          // Space index (if applicable)
                                   bool const calcIAQMethods    // For IAQProcedure, PCOccSch, and PCDesOcc, calculate if true, return zero if false
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   October 2012

    // PURPOSE OF THIS FUNCTION:
    // This function returns the air volume flow rate based on DesignSpecification:OutdoorAir object.

    // METHODOLOGY EMPLOYED:
    // User inputs and zone index allows calculation of outdoor air quantity.
    // Sizing does not use occupancy or min OA schedule and will call with flags set to FALSE
    // Ventilation Rate Procedure uses occupancy schedule based on user input.

    // Return value
    Real64 OAVolumeFlowRate; // Return value for calculated outdoor air volume flow rate [m3/s]

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 DSOAFlowPeople;            // Outdoor air volume flow rate based on occupancy (m3/s)
    Real64 DSOAFlowPerZone;           // Outdoor air volume flow rate (m3/s)
    Real64 DSOAFlowPerArea;           // Outdoor air volume flow rate based on zone floor area (m3/s)
    Real64 DSOAFlowACH;               // Outdoor air volume flow rate based on air changes per hour (m3/s)
    Real64 ZoneOAPeople;              // Zone OA flow rate based on number of occupants [m3/s]
    Real64 ZoneOAArea;                // Zone OA flow rate based on space floor area [m3/s]
    Real64 ZoneOAMin;                 // Minimum Zone OA flow rate when the zone is unoccupied (i.e. ZoneOAPeople = 0)
                                      // used for "ProportionalControl" System outdoor air method
    Real64 ZoneOAMax;                 // Maximum Zone OA flow rate (ZoneOAPeople + ZoneOAArea)
                                      // used for "ProportionalControl" System outdoor air method
    Real64 ZoneMaxCO2;                // Breathing-zone CO2 concentration
    Real64 ZoneMinCO2;                // Minimum CO2 concentration in zone
    Real64 ZoneContamControllerSched; // Schedule value for ZoneControl:ContaminantController
    Real64 CO2PeopleGeneration;       // CO2 generation from people at design level

    OAVolumeFlowRate = 0.0;

    auto &thisZone = state.dataHeatBal->Zone(ActualZoneNum);
    Real64 floorArea = 0.0;
    Real64 volume = 0.0;
    Real64 nomTotOccupants = 0.0;
    Real64 curNumOccupants = 0.0;
    Real64 maxOccupants = 0.0;
    if (spaceNum > 0) {
        auto const &thisSpace = state.dataHeatBal->space(spaceNum);
        floorArea = thisSpace.FloorArea;
        volume = thisSpace.Volume;
        nomTotOccupants = thisSpace.TotOccupants;
        curNumOccupants = state.dataHeatBal->spaceIntGain(spaceNum).NOFOCC;
        maxOccupants = thisSpace.maxOccupants;
    } else {
        floorArea = thisZone.FloorArea;
        volume = thisZone.Volume;
        nomTotOccupants = thisZone.TotOccupants;
        curNumOccupants = state.dataHeatBal->ZoneIntGain(ActualZoneNum).NOFOCC;
        maxOccupants = thisZone.maxOccupants;
    }

    if (this->OAFlowMethod == DataSizing::OAFlowCalcMethod::IAQProcedure && this->myEnvrnFlag) {
        if (!state.dataContaminantBalance->Contaminant.CO2Simulation) {
            ShowSevereError(state,
                            format("DesignSpecification:OutdoorAir=\"{}{}",
                                   this->Name,
                                   R"(" valid Outdoor Air Method =" IndoorAirQualityProcedure" requires CO2 simulation.)"));
            ShowContinueError(state, "The choice must be Yes for the field Carbon Dioxide Concentration in ZoneAirContaminantBalance");
            ShowFatalError(state, "CalcDesignSpecificationOutdoorAir: Errors found in input. Preceding condition(s) cause termination.");
        }
        this->myEnvrnFlag = false;
    }
    if (this->OAFlowMethod == DataSizing::OAFlowCalcMethod::PCOccSch && this->myEnvrnFlag) {
        if (!state.dataContaminantBalance->Contaminant.CO2Simulation) {
            ShowSevereError(state,
                            format("DesignSpecification:OutdoorAir=\"{}{}",
                                   this->Name,
                                   R"(" valid Outdoor Air Method =" ProportionalControlBasedOnDesignOccupancy" requires CO2 simulation.)"));
            ShowContinueError(state, "The choice must be Yes for the field Carbon Dioxide Concentration in ZoneAirContaminantBalance");
            ShowFatalError(state, "CalcDesignSpecificationOutdoorAir: Errors found in input. Preceding condition(s) cause termination.");
        }
        this->myEnvrnFlag = false;
    }
    if (this->OAFlowMethod == DataSizing::OAFlowCalcMethod::PCDesOcc && this->myEnvrnFlag) {
        if (!state.dataContaminantBalance->Contaminant.CO2Simulation) {
            ShowSevereError(state,
                            format("DesignSpecification:OutdoorAir=\"{}{}",
                                   this->Name,
                                   R"(" valid Outdoor Air Method =" ProportionalControlBasedOnOccupancySchedule" requires CO2 simulation.)"));
            ShowContinueError(state, "The choice must be Yes for the field Carbon Dioxide Concentration in ZoneAirContaminantBalance");
            ShowFatalError(state, "CalcDesignSpecificationOutdoorAir: Errors found in input. Preceding condition(s) cause termination.");
        }
        this->myEnvrnFlag = false;
    }

    if (!calcIAQMethods &&
        (this->OAFlowMethod == DataSizing::OAFlowCalcMethod::IAQProcedure || this->OAFlowMethod == DataSizing::OAFlowCalcMethod::PCOccSch ||
         this->OAFlowMethod == DataSizing::OAFlowCalcMethod::PCDesOcc)) {
        return OAVolumeFlowRate;
    }

    // Calculate people outdoor air flow rate as needed
    switch (this->OAFlowMethod) {
    case OAFlowCalcMethod::PerPerson:
    case OAFlowCalcMethod::Sum:
    case OAFlowCalcMethod::Max: {
        if (UseOccSchFlag) {
            if (MaxOAVolFlowFlag) {
                // OAPerPersonMode == PerPersonDCVByCurrentLevel (UseOccSchFlag = TRUE)
                // for dual duct, get max people according to max schedule value when requesting MaxOAFlow
                DSOAFlowPeople = maxOccupants * this->OAFlowPerPerson;
            } else {
                DSOAFlowPeople = curNumOccupants * this->OAFlowPerPerson;
            }
        } else {
            if (MaxOAVolFlowFlag) {
                // OAPerPersonMode == PerPersonByDesignLevel (UseOccSchFlag = FALSE)
                // use total people when requesting MaxOAFlow
                DSOAFlowPeople = nomTotOccupants * this->OAFlowPerPerson;
            } else {
                DSOAFlowPeople = nomTotOccupants * this->OAFlowPerPerson;
            }
        }
        if (PerPersonNotSet) {
            DSOAFlowPeople = 0.0; // for Dual Duct if Per Person Ventilation Rate Mode is not entered
        }
    } break;
    default: {
        DSOAFlowPeople = 0.0;
    } break;
    }

    // Calculate minimum outdoor air flow rate
    switch (this->OAFlowMethod) {
    case OAFlowCalcMethod::PerPerson: {
        // Multiplied by occupancy
        OAVolumeFlowRate = DSOAFlowPeople;
    } break;
    case OAFlowCalcMethod::PerZone: {
        // User input
        OAVolumeFlowRate = this->OAFlowPerZone;
    } break;
    case OAFlowCalcMethod::PerArea: {
        // Multiplied by zone floor area
        OAVolumeFlowRate = this->OAFlowPerArea * floorArea;
    } break;
    case OAFlowCalcMethod::ACH: {
        // Multiplied by zone volume
        OAVolumeFlowRate = this->OAFlowACH * volume / 3600.0;
    } break;
    case OAFlowCalcMethod::Sum:
    case OAFlowCalcMethod::Max: {
        // Use sum or max of per person and the following
        DSOAFlowPerZone = this->OAFlowPerZone;
        DSOAFlowPerArea = this->OAFlowPerArea * floorArea;
        DSOAFlowACH = this->OAFlowACH * volume / 3600.0;
        if (this->OAFlowMethod == OAFlowCalcMethod::Max) {
            OAVolumeFlowRate = max(DSOAFlowPeople, DSOAFlowPerZone, DSOAFlowPerArea, DSOAFlowACH);
        } else {
            OAVolumeFlowRate = DSOAFlowPeople + DSOAFlowPerZone + DSOAFlowPerArea + DSOAFlowACH;
        }
    } break;
    case DataSizing::OAFlowCalcMethod::IAQProcedure: {
        if (state.dataGlobal->DoingSizing) {
            DSOAFlowPeople = nomTotOccupants * this->OAFlowPerPerson;
            DSOAFlowPerZone = this->OAFlowPerZone;
            DSOAFlowPerArea = this->OAFlowPerArea * floorArea;
            DSOAFlowACH = this->OAFlowACH * volume / 3600.0;
            OAVolumeFlowRate = DSOAFlowPeople + DSOAFlowPerZone + DSOAFlowPerArea + DSOAFlowACH;
        } else {
            OAVolumeFlowRate = state.dataContaminantBalance->ZoneSysContDemand(ActualZoneNum).OutputRequiredToCO2SP / state.dataEnvrn->StdRhoAir;
        }
    } break;
    case DataSizing::OAFlowCalcMethod::PCOccSch:
    case DataSizing::OAFlowCalcMethod::PCDesOcc: {
        ZoneOAPeople = 0.0;
        if (this->OAFlowMethod != DataSizing::OAFlowCalcMethod::PCDesOcc) {
            ZoneOAPeople = curNumOccupants * thisZone.Multiplier * thisZone.ListMultiplier * this->OAFlowPerPerson;
        } else {
            ZoneOAPeople = nomTotOccupants * thisZone.Multiplier * thisZone.ListMultiplier * this->OAFlowPerPerson;
            CO2PeopleGeneration = 0.0;
            if (this->OAFlowMethod == DataSizing::OAFlowCalcMethod::PCDesOcc) {
                // Accumulate CO2 generation from people at design occupancy and current activity level
                for (int PeopleNum = 1; PeopleNum <= state.dataHeatBal->TotPeople; ++PeopleNum) {
                    if (spaceNum > 0) {
                        if (state.dataHeatBal->People(PeopleNum).spaceIndex != spaceNum) {
                            continue;
                        }
                    } else {
                        if (state.dataHeatBal->People(PeopleNum).ZonePtr != ActualZoneNum) {
                            continue;
                        }
                    }
                    CO2PeopleGeneration += state.dataHeatBal->People(PeopleNum).NumberOfPeople * state.dataHeatBal->People(PeopleNum).CO2RateFactor *
                                           state.dataHeatBal->People(PeopleNum).activityLevelSched->getCurrentVal();
                }
            }
        }
        ZoneOAArea = floorArea * thisZone.Multiplier * thisZone.ListMultiplier * this->OAFlowPerArea;
        ZoneOAMin = ZoneOAArea;
        ZoneOAMax = (ZoneOAArea + ZoneOAPeople);
        if (thisZone.zoneContamControllerSched != nullptr) {
            // Check the availability schedule value for ZoneControl:ContaminantController
            ZoneContamControllerSched = thisZone.zoneContamControllerSched->getCurrentVal();
            if (ZoneContamControllerSched > 0.0) {
                if (ZoneOAPeople > 0.0) {
                    if (state.dataContaminantBalance->ZoneCO2GainFromPeople(ActualZoneNum) > 0.0) {
                        if (thisZone.zoneMinCO2Sched != nullptr) {
                            // Take the schedule value of "Minimum Carbon Dioxide Concentration Schedule Name"
                            // in the ZoneControl:ContaminantController
                            ZoneMinCO2 = thisZone.zoneMinCO2Sched->getCurrentVal();
                        } else {
                            ZoneMinCO2 = state.dataContaminantBalance->OutdoorCO2;
                        }

                        // Calculate zone maximum target CO2 concentration in PPM
                        if (this->OAFlowMethod == DataSizing::OAFlowCalcMethod::PCDesOcc) {
                            ZoneMaxCO2 = state.dataContaminantBalance->OutdoorCO2 +
                                         (CO2PeopleGeneration * thisZone.Multiplier * thisZone.ListMultiplier * 1.0e6) / ZoneOAMax;
                        } else {
                            ZoneMaxCO2 =
                                state.dataContaminantBalance->OutdoorCO2 + (state.dataContaminantBalance->ZoneCO2GainFromPeople(ActualZoneNum) *
                                                                            thisZone.Multiplier * thisZone.ListMultiplier * 1.0e6) /
                                                                               ZoneOAMax;
                        }

                        if (ZoneMaxCO2 <= ZoneMinCO2) {
                            ++this->CO2MaxMinLimitErrorCount;
                            if (this->OAFlowMethod == DataSizing::OAFlowCalcMethod::PCOccSch) {
                                if (this->CO2MaxMinLimitErrorCount < 2) {
                                    ShowSevereError(state,
                                                    format("CalcDesignSpecificationOutdoorAir DesignSpecification:OutdoorAir = \"{}\".", this->Name));
                                    ShowContinueError(
                                        state,
                                        format("For System Outdoor Air Method = ProportionalControlBasedOnOccupancySchedule, maximum target "
                                               "CO2 concentration ({:.2R}), is not greater than minimum target CO2 concentration ({:.2R}).",
                                               ZoneMaxCO2,
                                               ZoneMinCO2));
                                    ShowContinueError(state,
                                                      "\"ProportionalControlBasedOnOccupancySchedule\" will not be modeled. "
                                                      "Default \"Flow/Person+Flow/Area\" will be modeled. Simulation continues...");
                                    ShowContinueErrorTimeStamp(state, "");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(
                                        state,
                                        format("DesignSpecification:OutdoorAir = \"{}\", For System Outdoor Air Method = "
                                               "ProportionalControlBasedOnOccupancySchedule, maximum target CO2 concentration is not greater than "
                                               "minimum target CO2 concentration. Error continues...",
                                               this->Name),
                                        this->CO2MaxMinLimitErrorIndex);
                                }
                            }
                            if (this->OAFlowMethod == DataSizing::OAFlowCalcMethod::PCDesOcc) {
                                if (this->CO2MaxMinLimitErrorCount < 2) {
                                    ShowSevereError(state,
                                                    format("CalcDesignSpecificationOutdoorAir DesignSpecification:OutdoorAir = \"{}\".", this->Name));
                                    ShowContinueError(
                                        state,
                                        format("For System Outdoor Air Method = ProportionalControlBasedOnDesignOccupancy, maximum target "
                                               "CO2 concentration ({:.2R}), is not greater than minimum target CO2 concentration ({:.2R}).",
                                               ZoneMaxCO2,
                                               ZoneMinCO2));
                                    ShowContinueError(state,
                                                      "\"ProportionalControlBasedOnDesignOccupancy\" will not be modeled. "
                                                      "Default \"Flow/Person+Flow/Area\" will be modeled. Simulation continues...");
                                    ShowContinueErrorTimeStamp(state, "");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(
                                        state,
                                        format("DesignSpecification:OutdoorAir = \"{}\", For System Outdoor Air Method = "
                                               "ProportionalControlBasedOnDesignOccupancy, maximum target CO2 concentration is not greater than "
                                               "minimum target CO2 concentration. Error continues...",
                                               this->Name),
                                        this->CO2MaxMinLimitErrorIndex);
                                }
                            }

                            OAVolumeFlowRate = ZoneOAMax;
                        } else {

                            if (state.dataContaminantBalance->ZoneAirCO2(ActualZoneNum) <= ZoneMinCO2) {
                                // Zone air CO2 concentration is less than minimum zone CO2 concentration, set the Zone OA flow rate to
                                // minimum Zone OA flow rate when the zone is unoccupied
                                OAVolumeFlowRate = ZoneOAMin;
                            } else if (state.dataContaminantBalance->ZoneAirCO2(ActualZoneNum) >= ZoneMaxCO2) {
                                // Zone air CO2 concentration is greater than maximum zone CO2 concentration, set the Zone OA flow rate to
                                // maximum Zone OA flow rate (i.e. ZoneOAArea + ZoneOAPeople)
                                OAVolumeFlowRate = ZoneOAMax;
                            } else {
                                // Zone air CO2 concentration is between maximum and minimum limits of zone CO2 concentration,
                                // set Zone OA flow rate by proportionally adjusting between ZoneOAMin and ZoneOAMax
                                OAVolumeFlowRate =
                                    ZoneOAMin + (ZoneOAMax - ZoneOAMin) * ((state.dataContaminantBalance->ZoneAirCO2(ActualZoneNum) - ZoneMinCO2) /
                                                                           (ZoneMaxCO2 - ZoneMinCO2));
                            }
                        }
                    } else {
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            ++this->CO2GainErrorCount;
                            if (this->OAFlowMethod == DataSizing::OAFlowCalcMethod::PCOccSch) {
                                if (this->CO2GainErrorCount < 2) {
                                    ShowSevereError(state,
                                                    format("CalcDesignSpecificationOutdoorAir DesignSpecification:OutdoorAir = \"{}\".", this->Name));
                                    ShowContinueError(state,
                                                      format("For System Outdoor Air Method = ProportionalControlBasedOnOccupancySchedule, CO2 "
                                                             "generation from people is not greater than zero. Occurs in Zone =\"{}\". ",
                                                             thisZone.Name));
                                    ShowContinueError(state,
                                                      "\"ProportionalControlBasedOnOccupancySchedule\" will not be modeled. "
                                                      "Default \"Flow/Person+Flow/Area\" will be modeled. Simulation continues...");
                                    ShowContinueErrorTimeStamp(state, "");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(state,
                                                                   format("DesignSpecification:OutdoorAir = \"{}\", For System Outdoor Air Method = "
                                                                          "ProportionalControlBasedOnOccupancySchedule, CO2 generation from people "
                                                                          "is not greater than zero. Error continues...",
                                                                          this->Name),
                                                                   this->CO2GainErrorIndex);
                                }
                            }
                            if (this->OAFlowMethod == DataSizing::OAFlowCalcMethod::PCDesOcc) {
                                if (this->CO2GainErrorCount < 2) {
                                    ShowSevereError(state,
                                                    format("CalcDesignSpecificationOutdoorAir DesignSpecification:OutdoorAir = \"{}\".", this->Name));
                                    ShowContinueError(state,
                                                      format("For System Outdoor Air Method = ProportionalControlBasedOnDesignOccupancy, CO2 "
                                                             "generation from people is not greater than zero. Occurs in Zone =\"{}\". ",
                                                             thisZone.Name));
                                    ShowContinueError(state,
                                                      "\"ProportionalControlBasedOnDesignOccupancy\" will not be modeled. "
                                                      "Default \"Flow/Person+Flow/Area\" will be modeled. Simulation continues...");
                                    ShowContinueErrorTimeStamp(state, "");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(state,
                                                                   format("DesignSpecification:OutdoorAir = \"{}\", For System Outdoor Air Method = "
                                                                          "ProportionalControlBasedOnDesignOccupancy, CO2 generation from people is "
                                                                          "not greater than zero. Error continues...",
                                                                          this->Name),
                                                                   this->CO2GainErrorIndex);
                                }
                            }
                        }
                        OAVolumeFlowRate = ZoneOAMax;
                    }
                } else {
                    // ZoneOAPeople is less than or equal to zero
                    OAVolumeFlowRate = ZoneOAMax;
                }
            } else {
                // ZoneControl:ContaminantController is scheduled off (not available)
                OAVolumeFlowRate = ZoneOAMax;
            }
        } else {
            // "Carbon Dioxide Control Availability Schedule" for ZoneControl:ContaminantController not found
            OAVolumeFlowRate = ZoneOAMax;
        }
    } break;
    default: {
        // Will never get here
        OAVolumeFlowRate = 0.0;
    } break;
    }

    // Apply zone multipliers and zone list multipliers
    // TODO MJW: this looks like it's double-counting the multipliers - it *is* double counting for methods PCOccSch and PCDesOcc
    OAVolumeFlowRate *= thisZone.Multiplier * thisZone.ListMultiplier;

    // Apply schedule as needed. Sizing does not use schedule.
    if (this->oaFlowFracSched != nullptr && UseMinOASchFlag) {
        if (MaxOAVolFlowFlag) {
            OAVolumeFlowRate *= this->oaFlowFracSched->getMaxVal(state);
        } else {
            OAVolumeFlowRate *= this->oaFlowFracSched->getCurrentVal();
        }
    }

    return OAVolumeFlowRate;
}

Sched::Schedule *OARequirementsData::getZoneFlowFracSched(EnergyPlusData &state, bool notAllSame)
{
    notAllSame = false;
    Sched::Schedule *schedPtr = nullptr;
    if (this->numDSOA == 0) {
        // This is a simple DesignSpecification:OutdoorAir
        schedPtr = this->oaFlowFracSched;
    } else {
        // This is a DesignSpecification:OutdoorAir:SpaceList
        for (int dsoaCount = 1; dsoaCount <= this->numDSOA; ++dsoaCount) {
            auto const &thisDSOA = state.dataSize->OARequirements(this->dsoaIndexes(dsoaCount));
            if (dsoaCount == 1) {
                schedPtr = thisDSOA.oaFlowFracSched;
            } else {
                if (schedPtr != thisDSOA.oaFlowFracSched) {
                    notAllSame = true;
                    ShowWarningError(state,
                                     format("getZoneFlowFracSched: Outdoor Air Schedules are not the same for all spaces in "
                                            "DesignSpecification:OutdoorAir:SpaceList={}.",
                                            this->Name));
                    ShowContinueError(state, format("Using the first space schedule={}", schedPtr->Name));
                    break;
                }
            }
        }
    }
    return schedPtr;
}

Sched::Schedule *OARequirementsData::getZonePropCtlMinRateSched(EnergyPlusData &state, bool notAllSame)
{
    notAllSame = false;
    Sched::Schedule *schedPtr = nullptr;
    if (this->numDSOA == 0) {
        // This is a simple DesignSpecification:OutdoorAir
        schedPtr = this->oaPropCtlMinRateSched;
    } else {
        // This is a DesignSpecification:OutdoorAir:SpaceList
        for (int dsoaCount = 1; dsoaCount <= this->numDSOA; ++dsoaCount) {
            auto const &thisDSOA = state.dataSize->OARequirements(this->dsoaIndexes(dsoaCount));
            if (dsoaCount == 1) {
                schedPtr = thisDSOA.oaPropCtlMinRateSched;
            } else {
                if (schedPtr != thisDSOA.oaPropCtlMinRateSched) {
                    notAllSame = true;
                    ShowWarningError(
                        state,
                        format(
                            "getZoneFlowFracSched: Proportional Control Minimum Outdoor Air Flow Rate Schedules are not the same for all spaces in "
                            "DesignSpecification:OutdoorAir:SpaceList={}.",
                            this->Name));
                    ShowContinueError(state, format("Using the first space schedule={}", schedPtr->Name));
                    break;
                }
            }
        }
    }
    return schedPtr;
}

} // namespace EnergyPlus::DataSizing
