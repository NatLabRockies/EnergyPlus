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

#ifndef DataSizing_hh_INCLUDED
#define DataSizing_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DataSizing {

    enum class OAFlowCalcMethod
    // parameters for outside air flow method
    {
        Invalid = -1,
        PerPerson,    // set the outdoor air flow rate based on number of people in the zone
        PerZone,      // sum the outdoor air flow rate per zone based on user input
        PerArea,      // sum the outdoor air flow rate based on zone area
        ACH,          // sum the outdoor air flow rate based on number of air changes for the zone
        Sum,          // sum the outdoor air flow rate of the people component and the space floor area component
        Max,          // use the maximum of the outdoor air flow rate of the people component and the space floor area component
        IAQProcedure, // Use ASHRAE Standard 62.1-2007 IAQP to calculate the zone level outdoor air flow rates
        PCOccSch,     // ProportionalControlBasedOnOccupancySchedule, Use ASHRAE Standard 62.1-2004 or Trane Engineer's newsletter (volume 34-5) to
                      // calculate the zone level outdoor air flow rates based on scheduled occupancy
        PCDesOcc,     // ProportionalControlBasedOnDesignOccupancy, Use ASHRAE Standard 62.1-2004 or Trane Engineer's newsletter (volume 34-5) to
                      // calculate the zone level outdoor air flow rates based on design occupancy
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(OAFlowCalcMethod::Num)> OAFlowCalcMethodNames{
        "Flow/Person",
        "Flow/Zone",
        "Flow/Area",
        "AirChanges/Hour",
        "Sum",
        "Maximum",
        "IndoorAirQualityProcedure",
        "ProportionalControlBasedOnOccupancySchedule",
        "ProportionalControlBasedOnDesignOccupancy"};

    // parameters for outside air
    enum class OAControl
    {
        Invalid = -1,
        AllOA,
        MinOA,
        Num
    };

    // parameters for loop fluid type
    enum class TypeOfPlantLoop
    {
        Invalid = -1,
        Heating,
        Cooling,
        Condenser,
        Steam,
        Num
    };

    // parameters for zone and system sizing concurrence method
    enum class SizingConcurrence
    {
        Invalid = -1,
        NonCoincident,
        Coincident,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(SizingConcurrence::Num)> SizingConcurrenceNamesUC{"NONCOINCIDENT", "COINCIDENT"};
    constexpr std::array<std::string_view, static_cast<int>(SizingConcurrence::Num)> SizingConcurrenceNames{"NonCoincident", "Coincident"};

    // parameters for coil sizing concurrence method
    enum class CoilSizingConcurrence
    {
        Invalid = -1,
        NonCoincident,
        Coincident,
        Combination,
        NA,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(CoilSizingConcurrence::Num)> CoilSizingConcurrenceNames{
        "Non-Coincident", "Coincident", "Combination", "N/A"};

    // parameters for Cooling Peak Load Type
    enum class PeakLoad
    {
        Invalid = -1,
        SensibleCooling,
        TotalCooling,
        Num
    };

    // parameters for Central Cooling Capacity Control Method
    enum class CapacityControl
    {
        Invalid = -1,
        VAV,
        Bypass,
        VT,
        OnOff,
        Num
    };

    // parameters for supply air flow rate method
    constexpr int SupplyAirTemperature(1);
    constexpr int TemperatureDifference(2);
    constexpr int SupplyAirHumidityRatio(3);
    constexpr int HumidityRatioDifference(4);

    // parameters for sizing
    enum class AirflowSizingMethod
    {
        Invalid = -1,
        FromDDCalc,
        InpDesAirFlow,
        DesAirFlowWithLim,
        Num
    };

    enum class DOASControl
    {
        Invalid = -1,
        NeutralSup,
        NeutralDehumSup,
        CoolSup,
        Num
    };

    // parameters for Type of Load to Size On
    enum class LoadSizing
    {
        Invalid = -1,
        Sensible,
        Latent,
        Total,
        Ventilation,
        Num
    };

    // parameter for autosize
    constexpr Real64 AutoSize(-99999.0);

    // parameter for (time-of-peak) sizing format
    static constexpr std::string_view PeakHrMinFmt("{:02}:{:02}:00");

    enum class SysOAMethod
    {
        Invalid = -1,
        ZoneSum, // Sum the outdoor air flow rates of all zones
        VRP,     // Use ASHRAE Standard 62.1-2007 to calculate the system level outdoor air flow rates
        IAQP,    // Use ASHRAE Standard 62.1-2007 IAQP to calculate the system level outdoor air flow rates based on the CO2 setpoint
        ProportionalControlSchOcc, // Use ASHRAE Standard 62.1-2004 or Trane Engineer's newsletter (volume 34-5) to calculate the system level outdoor
                                   // air flow rates based on scheduled occupancy
        IAQPGC,  // Use ASHRAE Standard 62.1-2004 IAQP to calculate the system level outdoor air flow rates based on the generic contaminant setpoint
        IAQPCOM, // Take the maximum outdoor air rate from both CO2 and generic contaminant controls based on the generic contaminant setpoint
        ProportionalControlDesOcc, // Use ASHRAE Standard 62.1-2004 or Trane Engineer's newsletter (volume 34-5) to calculate the system level outdoor
                                   // air flow rates based on design occupancy
        ProportionalControlDesOARate, // Calculate the system level outdoor air flow rates based on design OA rate
        SP, // Use the ASHRAE Standard 62.1 Simplified Procedure to calculate the system level outdoor air flow rates considering the zone air
            // distribution effectiveness and the system ventilation efficiency
        VRPL, // Use ASHRAE Standard 62.1-2007 to calculate the system level outdoor air flow rates
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(SysOAMethod::Num)> SysOAMethodNames{"Zone Sum",
                                                                                                "Ventilation Rate Procedure",
                                                                                                "IAQ Proc",
                                                                                                "Proportional - Sch Occupancy",
                                                                                                "IAQ Proc - Generic Contaminant",
                                                                                                "IAQ Proc - Max Gen Cont or CO2.",
                                                                                                "Proportional - Des Occupancy",
                                                                                                "Proportional - Des OA Rate",
                                                                                                "Simplified Procure",
                                                                                                "Ventilation Rate Procedure Level"};

    // Zone HVAC Equipment Sizing Option
    enum class DesignSizingType
    {
        Invalid = -1,
        Dummy1BasedOffset,
        None, // until all models use this enum this numeric must match with constexpr int None(1) below
        SupplyAirFlowRate,
        FlowPerFloorArea,
        FractionOfAutosizedCoolingAirflow,
        FractionOfAutosizedHeatingAirflow,
        FlowPerCoolingCapacity,
        FlowPerHeatingCapacity,
        CoolingDesignCapacity,
        HeatingDesignCapacity,
        CapacityPerFloorArea,
        FractionOfAutosizedCoolingCapacity,
        FractionOfAutosizedHeatingCapacity,
        Num

    };
    constexpr std::array<std::string_view, static_cast<int>(DesignSizingType::Num)> DesignSizingTypeNamesUC{"DUMMY1BASEDOFFSET",
                                                                                                            "NONE",
                                                                                                            "SUPPLYAIRFLOWRATE",
                                                                                                            "FLOWPERFLOORAREA",
                                                                                                            "FRACTIONOFAUTOSIZEDCOOLINGAIRFLOW",
                                                                                                            "FRACTIONOFAUTOSIZEDHEATINGAIRFLOW",
                                                                                                            "FLOWPERCOOLINGCAPACITY",
                                                                                                            "FLOWPERHEATINGCAPACITY",
                                                                                                            "COOLINGDESIGNCAPACITY",
                                                                                                            "HEATINGDESIGNCAPACITY",
                                                                                                            "CAPACITYPERFLOORAREA",
                                                                                                            "FRACTIONOFAUTOSIZEDCOOLINGCAPACITY",
                                                                                                            "FRACTIONOFAUTOSIZEDHEATINGCAPACITY"};

    constexpr int None(1);
    constexpr int SupplyAirFlowRate(2);
    constexpr int FlowPerFloorArea(3);
    constexpr int FractionOfAutosizedCoolingAirflow(4);
    constexpr int FractionOfAutosizedHeatingAirflow(5);
    constexpr int FlowPerCoolingCapacity(6);
    constexpr int FlowPerHeatingCapacity(7);

    constexpr int CoolingDesignCapacity(8);
    constexpr int HeatingDesignCapacity(9);
    constexpr int CapacityPerFloorArea(10);
    constexpr int FractionOfAutosizedCoolingCapacity(11);
    constexpr int FractionOfAutosizedHeatingCapacity(12);

    constexpr int NoSizingFactorMode(101);
    constexpr int GlobalHeatingSizingFactorMode(102);
    constexpr int GlobalCoolingSizingFactorMode(103);
    constexpr int LoopComponentSizingFactorMode(104);

    enum class ZoneSizing
    {
        Invalid = -1,
        Sensible,
        Latent,
        SensibleAndLatent,
        SensibleOnly,
        Num
    };
    constexpr std::array<std::string_view, static_cast<int>(ZoneSizing::Num)> ZoneSizingMethodNamesUC{
        "SENSIBLE LOAD", "LATENT LOAD", "SENSIBLE AND LATENT LOAD", "SENSIBLE LOAD ONLY NO LATENT LOAD"};

    enum class HeatCoilSizMethod
    {
        Invalid = -1,
        None,
        CoolingCapacity,
        HeatingCapacity,
        GreaterOfHeatingOrCooling,
        Num
    };
    constexpr std::array<std::string_view, static_cast<int>(HeatCoilSizMethod::Num)> HeatCoilSizMethodNamesUC{
        "NONE", "COOLINGCAPACITY", "HEATINGCAPACITY", "GREATEROFHEATINGORCOOLING"};

    // Types

    struct ZoneSizingInputData
    {
        // Members
        std::string ZoneName;      // name of a zone
        int ZoneNum = 0;           // index of the zone
        int ZnCoolDgnSAMethod = 0; // choice of how to get zone cooling design air temperature;
        //  1 = specify supply air temperature, 2 = calculate from the temperature difference
        int ZnHeatDgnSAMethod = 0; // choice of how to get zone heating design air temperature;
        //  1 = specify supply air temperature, 2 = calculate from the temperature difference
        Real64 CoolDesTemp = 0.0;        // zone design cooling supply air temperature [C]
        Real64 HeatDesTemp = 0.0;        // zone design heating supply air temperature [C]
        Real64 CoolDesTempDiff = 0.0;    // zone design cooling supply air temperature difference [deltaC]
        Real64 HeatDesTempDiff = 0.0;    // zone design heating supply air temperature difference [deltaC]
        Real64 CoolDesHumRat = 0.0;      // zone design cooling supply air humidity ratio [kgWater/kgDryAir]
        Real64 HeatDesHumRat = 0.0;      // zone design heating supply air humidity ratio [kgWater/kgDryAir]
        std::string DesignSpecOAObjName; // name of the DesignSpecification:OutdoorAir or DesignSpecification:OutdoorAir:SpaceList object
        AirflowSizingMethod CoolAirDesMethod = AirflowSizingMethod::Invalid; // choice of how to get zone cooling design air flow rates;
        //  0 = calc from des day simulation; 1 = m3/s per zone, user input' 2 = apply limits to air flow rate from DD calc
        Real64 DesCoolAirFlow = 0.0;           // design zone supply air flow rate [m3/s]
        Real64 DesCoolMinAirFlowPerArea = 0.0; // design cooling minimum air flow rate per zone area [m3/s / m2]
        Real64 DesCoolMinAirFlow = 0.0;        // design cooling minimum air flow rate [m3/s]
        Real64 DesCoolMinAirFlowFrac = 0.0;    // design cooling minimum air flow rate fraction
        //  (of the cooling design air flow rate)
        AirflowSizingMethod HeatAirDesMethod = AirflowSizingMethod::Invalid; // choice of how to get zone heating design air flow rates;
        //  0 = calc from des day simulation; 1 = m3/s per zone, user input; 2 = apply limits to air flow rate from DD calc
        Real64 DesHeatAirFlow = 0.0;           // design zone heating supply air flow rate [m3/s]
        Real64 DesHeatMaxAirFlowPerArea = 0.0; // design heating maximum air flow rate per zone area [m3/s / m2]
        Real64 DesHeatMaxAirFlow = 0.0;        // design heating maximum air flow rate [m3/s]
        Real64 DesHeatMaxAirFlowFrac = 0.0;    // design heating maximum air flow rate fraction (of the cooling design air flow rate)
        Real64 HeatSizingFactor = 0.0;         // the zone heating sizing ratio
        Real64 CoolSizingFactor = 0.0;         // the zone cooling sizing ratio
        Real64 ZoneADEffCooling = 0.0;
        Real64 ZoneADEffHeating = 0.0;
        std::string ZoneAirDistEffObjName;       // name of the zone air distribution effectiveness object name
        int ZoneAirDistributionIndex = 0;        // index to the zone air distribution object
        int ZoneDesignSpecOAIndex = 0;           // index to the zone design spec OA object
        Real64 ZoneSecondaryRecirculation = 0.0; // the zone secondary air recirculation fraction
        Real64 ZoneVentilationEff = 0.0;         // zone ventilation efficiency
        bool AccountForDOAS = false;             // False: do nothing; True: calculate the effect of a DOA system on the zone sizing arrays
        DOASControl DOASControlStrategy = DOASControl::Invalid; // 0=neutral ventilation air; 1=neutral dehumidified ventilation air, 2 = cooled air;
        Real64 DOASLowSetpoint = 0.0;                           // Dedicated Outside Air Low Setpoint for Design [C]
        Real64 DOASHighSetpoint = 0.0;                          // Dedicated Outside Air High Setpoint for Design [C]
        DataSizing::SizingConcurrence spaceConcurrence = DataSizing::SizingConcurrence::Coincident; // coincident or noncoincident space loads

        // zone latent sizing inputs
        bool zoneLatentSizing = false;
        Real64 zoneRHDehumidifySetPoint = 50.0;
        Real64 zoneRHHumidifySetPoint = 50.0;
        Real64 LatentCoolDesHumRat = 0.0;                  // zone design dehumidification supply air humidity ratio [kgw/kga]
        Real64 CoolDesHumRatDiff = 0.005;                  // zone design cooling supply air humidity ratio difference [deltakgw/kga]
        Real64 LatentHeatDesHumRat = 0.0;                  // zone design humidification supply air humidity ratio [kgw/kga]
        Real64 HeatDesHumRatDiff = 0.005;                  // zone design heating supply air humidity ratio temperature difference [deltakgw/kga]
        int ZnLatCoolDgnSAMethod = 0;                      // choice of how to get zone latent cooling design air humidity ratio;
        int ZnLatHeatDgnSAMethod = 0;                      // choice of how to get zone latent heating design air humidity ratio;
        Sched::Schedule *zoneRHDehumidifySched = nullptr;  // zone RH dehumidifying schedule used for zone sizing
        Sched::Schedule *zoneRHHumidifySched = nullptr;    // zone RH humidifying schedule used for zone sizing
        ZoneSizing zoneSizingMethod = ZoneSizing::Invalid; // load to sizing on: sensible, latent, sensibleandlatent, sensibleonlynolatent
        HeatCoilSizMethod heatCoilSizingMethod = HeatCoilSizMethod::Invalid; // Used for sizing heat pumps
        Real64 maxHeatCoilToCoolingLoadSizingRatio = 0.0; // Used for sizing heat pumps, max size of heating coil to cooling load sizing ratio
    };

    // based on ZoneSizingData but only member variables that are actually used by terminal unit sizing
    struct TermUnitZoneSizingCommonData
    {
        std::string ZoneName;           // name of a zone
        std::string ADUName;            // Terminal Unit Name (air distribution unit or direct air unit) - only assigned for TermUnitFinalZoneSizing
        Real64 CoolDesTemp = 0.0;       // zone design cooling supply air temperature [C]
        Real64 HeatDesTemp = 0.0;       // zone design heating supply air temperature [C]
        Real64 CoolDesHumRat = 0.0;     // zone design cooling supply air humidity ratio [kgWater/kgDryAir]
        Real64 HeatDesHumRat = 0.0;     // zone design heating supply air humidity ratio [kgWater/kgDryAir]
        Real64 DesOAFlowPPer = 0.0;     // design outside air flow per person in zone [m3/s] (average for zone across spaces)
        Real64 DesOAFlowPerArea = 0.0;  // design outside air flow per zone area [m3/s / m2] (average for zone across spaces)
        Real64 DesCoolMinAirFlow = 0.0; // design cooling minimum air flow rate [m3/s]
        Real64 DesCoolMinAirFlowFrac = 0.0; // design cooling minimum air flow rate fraction (of the cooling design air flow rate)
        Real64 DesHeatMaxAirFlow = 0.0;     // design heating maximum air flow rate [m3/s]
        Real64 DesHeatMaxAirFlowFrac = 0.0; // design heating maximum air flow rate fraction (of the cooling design air flow rate)
        int ZoneNum = 0;                    // index into the Zone data array (in DataHeatBalance)
        Real64 DesHeatMassFlow = 0.0;       // zone design heating air mass flow rate [kg/s]
        Real64 DesHeatMassFlowNoOA = 0.0;   // zone design heating air mass flow rate without applying MinOA as a limit [kg/s]
        Real64 DesHeatOAFlowFrac = 0.0;     // zone design heating OA air volume fraction [-]
        Real64 DesCoolMassFlow = 0.0;       // zone design cooling air mass flow rate [kg/s]
        Real64 DesCoolMassFlowNoOA = 0.0;   // zone design cooling air mass flow rate without applying MinOA as a limit [kg/s]
        Real64 DesCoolOAFlowFrac = 0.0;     // zone design cooling OA air volume fraction [-]
        Real64 DesHeatLoad = 0.0;           // zone design heating load including sizing factor and scaled to match airflow sizing [W]
        Real64 NonAirSysDesHeatLoad = 0.0;  // base zone design heating load including sizing factor [W]
        Real64 DesCoolLoad = 0.0;           // zone design cooling load including sizing factor and scaled to match airflow sizing [W]
        Real64 NonAirSysDesCoolLoad = 0.0;  // base zone design cooling load including sizing factor [W]
        Real64 DesHeatVolFlow = 0.0;     // zone design heating air volume flow rate including sizing factor and scaled to match airflow sizing [m3/s]
        Real64 DesHeatVolFlowNoOA = 0.0; // zone design heating air volume flow rate including sizing factor and scaled to match airflow sizing
                                         // without MinOA limit [m3/s]
        Real64 NonAirSysDesHeatVolFlow = 0.0;    // base zone design heating air volume flow rate including sizing factor [m3/s]
        Real64 DesCoolVolFlow = 0.0;             // zone design cooling air volume flow rate [m3/s]
        Real64 DesCoolVolFlowNoOA = 0.0;         // zone design cooling air volume flow rate without applying MinOA as a limit [m3/s]
        Real64 NonAirSysDesCoolVolFlow = 0.0;    // base zone design cooling air volume flow rate including sizing factor [m3/s]
        Real64 DesHeatVolFlowMax = 0.0;          // zone design heating maximum air volume flow rate [m3/s]
        Real64 DesCoolVolFlowMin = 0.0;          // zone design cooling minimum air volume flow rate [m3/s]
        Real64 DesHeatCoilInTempTU = 0.0;        // zone heating coil design air inlet temperature (supply air)([C]
        Real64 DesCoolCoilInTempTU = 0.0;        // zone cooling coil design air inlet temperature (supply air)[C]
        Real64 DesHeatCoilInHumRatTU = 0.0;      // zone heating coil design air inlet humidity ratio  [kg/kg]
        Real64 DesCoolCoilInHumRatTU = 0.0;      // zone cooling coil design air inlet humidity ratio  [kg/kg]
        Real64 ZoneTempAtHeatPeak = 0.0;         // zone temp at max heating [C]
        Real64 ZoneRetTempAtHeatPeak = 0.0;      // zone return temp at max heating [C]
        Real64 ZoneTempAtCoolPeak = 0.0;         // zone temp at max cooling [C]
        Real64 ZoneRetTempAtCoolPeak = 0.0;      // zone return temp at max cooling [C]
        Real64 ZoneHumRatAtHeatPeak = 0.0;       // zone humidity ratio at max heating [kg/kg]
        Real64 ZoneHumRatAtCoolPeak = 0.0;       // zone humidity ratio at max cooling [kg/kg]
        int TimeStepNumAtHeatMax = 0;            // time step number (in day) at Heating peak
        int TimeStepNumAtCoolMax = 0;            // time step number (in day) at cooling peak
        int HeatDDNum = 0;                       // design day index of design day causing heating peak
        int CoolDDNum = 0;                       // design day index of design day causing cooling peak
        Real64 MinOA = 0.0;                      // design minimum outside air in m3/s
        Real64 DesCoolMinAirFlow2 = 0.0;         // design cooling minimum air flow rate [m3/s] derived from DesCoolMinAirFlowPerArea
        Real64 DesHeatMaxAirFlow2 = 0.0;         // design heating maximum air flow rate [m3/s] derived from DesHeatMaxAirFlowPerArea
        Array1D<Real64> HeatFlowSeq;             // daily sequence of zone heating air mass flow rate (zone time step) [kg/s]
        Array1D<Real64> HeatFlowSeqNoOA;         // daily sequence of zone heating air mass flow rate (zone time step) without MinOA limit [kg/s]
        Array1D<Real64> CoolFlowSeq;             // daily sequence of zone cooling air mass flow rate (zone time step) [kg/s]
        Array1D<Real64> CoolFlowSeqNoOA;         // daily sequence of zone cooling air mass flow rate (zone time step) without MinOA limit [kg/s]
        EPVector<Real64> HeatZoneTempSeq;        // daily sequence of zone temperatures (heating, zone time step)
        Array1D<Real64> HeatZoneRetTempSeq;      // daily sequence of zone return temperatures (heating, zone time step)
        EPVector<Real64> CoolZoneTempSeq;        // daily sequence of zone temperatures (cooling, zone time step)
        Array1D<Real64> CoolZoneRetTempSeq;      // daily sequence of zone return temperatures (cooling, zone time step)
        Real64 ZoneADEffCooling = 1.0;           // the zone air distribution effectiveness in cooling mode
        Real64 ZoneADEffHeating = 1.0;           // the zone air distribution effectiveness in heating mode
        Real64 ZoneSecondaryRecirculation = 0.0; // the zone secondary air recirculation fraction
        Real64 ZoneVentilationEff = 0.0;         // zone ventilation efficiency
        Real64 ZonePrimaryAirFraction = 0.0;     // the zone primary air fraction for cooling based calculations
        Real64 ZonePrimaryAirFractionHtg = 0.0;  // the zone primary air fraction for heating based calculations
        Real64 ZoneOAFracCooling = 0.0;          // OA fraction in cooling mode
        Real64 ZoneOAFracHeating = 0.0;          // OA fraction in heating mode
        Real64 TotalOAFromPeople = 0.0;          // Zone OA required due to people
        Real64 TotalOAFromArea = 0.0;            // Zone OA required based on floor area
        Real64 TotPeopleInZone = 0.0;            // total number of people in the zone
        Real64 TotalZoneFloorArea = 0.0;         // total zone floor area
        Real64 SupplyAirAdjustFactor = 1.0;      // supply air adjustment factor for next time step if OA is capped
        Real64 ZpzClgByZone = 0.0;               // OA Std 62.1 required fraction in cooling mode ? should this be ZdzClgByZone
        Real64 ZpzHtgByZone = 0.0;               // OA Std 62.1 required fraction in heating mode ? should this be ZdzHtgByZone
        Real64 VozClgByZone = 0.0; // value of required cooling vent to zone, used in 62.1 tabular report, already includes people diversity term
        Real64 VozHtgByZone = 0.0; // value of required heating vent to zone, used in 62.1 tabular report, already includes people diversity term
        bool VpzMinByZoneSPSized = false;    // is Vpz_min sized using the 62.1 Standard Simplified Procedure
        Real64 ZoneSizThermSetPtHi = 0.0;    // highest zone thermostat setpoint during zone sizing calcs
        Real64 ZoneSizThermSetPtLo = 1000.0; // lowest zone thermostat setpoint during zone sizing calcs
    };

    struct ZoneSizingData : TermUnitZoneSizingCommonData
    {
        // Members
        std::string CoolDesDay;    // name of a cooling design day
        std::string HeatDesDay;    // name of a heating design day
        int ZnCoolDgnSAMethod = 0; // choice of how to get zone cooling design air temperature;
        //  1 = specify supply air temperature, 2 = calculate from the temperature difference
        int ZnHeatDgnSAMethod = 0; // choice of how to get zone heating design air temperature;
        //  1 = specify supply air temperature, 2 = calculate from the temperature difference
        Real64 CoolDesTempDiff = 0.0;                                        // zone design cooling supply air temperature difference [deltaC]
        Real64 HeatDesTempDiff = 0.0;                                        // zone design heating supply air temperature difference [deltaC]
        int ZoneAirDistributionIndex = 0;                                    // index to DesignSpecification:ZoneAirDistribution object
        int ZoneDesignSpecOAIndex = 0;                                       // index to DesignSpecification:OutdoorAir object
        AirflowSizingMethod CoolAirDesMethod = AirflowSizingMethod::Invalid; // choice of how to get zone cooling design air flow rates;
        //  0 = calc from des day simulation; 1 = m3/s per zone, user input; 2 = apply limits to air flow rate from DD calc
        Real64 InpDesCoolAirFlow = 0.0;                                      // design zone supply air flow rate [m3/s]
        Real64 DesCoolMinAirFlowPerArea = 0.0;                               // design cooling minimum air flow rate per zone area [m3/s / m2]
        AirflowSizingMethod HeatAirDesMethod = AirflowSizingMethod::Invalid; // choice of how to get zone heating design air flow rates;
        //  1 = calc from des day simulation; 2 = m3/s per zone, user input
        //  3 = apply limits to air flow rate from DD calc
        Real64 InpDesHeatAirFlow = 0.0;        // design zone heating supply air flow rate [m3/s]
        Real64 DesHeatMaxAirFlowPerArea = 0.0; // design heating maximum air flow rate per zone area [m3/s / m2]
        Real64 HeatSizingFactor = 0.0;         // the zone heating sizing ratio
        Real64 CoolSizingFactor = 0.0;         // the zone cooling sizing ratio
        bool AccountForDOAS = false;           // False: do nothing; True: calculate the effect of a DOA system on the zone sizing arrays
        DOASControl DOASControlStrategy = DOASControl::Invalid; // 0=neutral ventilation air; 1=neutral dehumidified ventilation air, 2 = cooled air;
        // 3=supply cold ventilation air
        Real64 DOASLowSetpoint = 0.0;  // Dedicated Outside Air Low Setpoint for Design [C]
        Real64 DOASHighSetpoint = 0.0; // Dedicated Outside Air High Setpoint for Design [C]
        DataSizing::SizingConcurrence spaceConcurrence = DataSizing::SizingConcurrence::Coincident; // coincident or noncoincident space loads
        bool EMSOverrideDesHeatMassOn = false;                                                      // true if EMS is acting on this structure
        Real64 EMSValueDesHeatMassFlow = 0.0;  // Value EMS directing to use for Design Heating air mass flow [kg/s]
        bool EMSOverrideDesCoolMassOn = false; // true if EMS is acting on this structure
        Real64 EMSValueDesCoolMassFlow = 0.0;  // Value EMS directing to use for Design Cooling air mass flow [kg/s]
        bool EMSOverrideDesHeatLoadOn = false; // true if EMS is acting on this structure
        Real64 EMSValueDesHeatLoad = 0.0;      // Value EMS directing to use for zone design heating load  [W]
        bool EMSOverrideDesCoolLoadOn = false; // true if EMS is acting on this structure
        Real64 EMSValueDesCoolLoad = 0.0;      // Value EMS directing to use for zone design cooling load  [W]
        Real64 DesHeatDens = 0.0;              // zone design heating air density [kg/m3]
        Real64 DesCoolDens = 0.0;              // zone design cooling air density [kg/m3]
        bool EMSOverrideDesHeatVolOn = false;  // true if EMS is acting on this structure
        Real64 EMSValueDesHeatVolFlow = 0.0;   // Value EMS directing to use for Design Heating air volume flow [m3/s]
        bool EMSOverrideDesCoolVolOn = false;  // true if EMS is acting on this structure
        Real64 EMSValueDesCoolVolFlow = 0.0;   // Value EMS directing to use for Design cooling air volume flow [m3/s]
        Real64 DesHeatCoilInTemp = 0.0;        // zone heating coil design air inlet temperature [C]
        Real64 DesCoolCoilInTemp = 0.0;        // zone cooling coil design air inlet temperature [C]
        Real64 DesHeatCoilInHumRat = 0.0;      // zone heating coil design air inlet humidity ratio [kg/kg]
        Real64 DesCoolCoilInHumRat = 0.0;      // zone cooling coil design air inlet humidity ratio [kg/kg]
        Real64 HeatMassFlow = 0.0;             // current zone heating air mass flow rate (HVAC time step)
        Real64 CoolMassFlow = 0.0;             // current zone cooling air mass flow rate (HVAC time step)
        Real64 HeatLoad = 0.0;                 // current zone heating load (HVAC time step)
        Real64 CoolLoad = 0.0;                 // current zone heating load (HVAC time step)
        Real64 HeatZoneTemp = 0.0;             // current zone temperature (heating, time step)
        Real64 HeatOutTemp = 0.0;              // current outdoor temperature (heating, time step)
        Real64 HeatZoneRetTemp = 0.0;          // current zone return temperature (heating, time step)
        Real64 HeatTstatTemp = 0.0;            // current zone thermostat temperature (heating, time step)
        Real64 CoolZoneTemp = 0.0;             // current zone temperature (cooling, time step)
        Real64 CoolOutTemp = 0.0;              // current Outdoor temperature (cooling, time step)
        Real64 CoolZoneRetTemp = 0.0;          // current zone return temperature (cooling, time step)
        Real64 CoolTstatTemp = 0.0;            // current zone thermostat temperature (cooling, time step)
        Real64 HeatZoneHumRat = 0.0;           // current zone humidity ratio (heating, time step)
        Real64 CoolZoneHumRat = 0.0;           // current zone humidity ratio (cooling, time step)
        Real64 HeatOutHumRat = 0.0;            // current outdoor humidity ratio (heating, time step)
        Real64 CoolOutHumRat = 0.0;            // current outdoor humidity ratio (cooling, time step)
        Real64 OutTempAtHeatPeak = 0.0;        // outdoor temperature at max heating [C]
        Real64 OutTempAtCoolPeak = 0.0;        // outdoor temperature at max cooling [C]
        Real64 OutHumRatAtHeatPeak = 0.0;      // outdoor humidity at max heating [kg/kg]
        Real64 OutHumRatAtCoolPeak = 0.0;      // outdoor humidity at max cooling [kg/kg]
        std::string cHeatDDDate;               // date of design day causing heating peak
        std::string cCoolDDDate;               // date of design day causing cooling peak
        Array1D<Real64> HeatLoadSeq;           // daily sequence of zone heating load (zone time step)
        Array1D<Real64> CoolLoadSeq;           // daily sequence of zone cooling load (zone time step)
        Array1D<Real64> HeatOutTempSeq;        // daily sequence of outdoor temperatures (heating, zone time step)
        Array1D<Real64> HeatTstatTempSeq;      // daily sequence of zone thermostat temperatures (heating, zone time step)
        Array1D<Real64> DesHeatSetPtSeq;       // daily sequence of indoor set point temperatures (zone time step)
        Array1D<Real64> CoolOutTempSeq;        // daily sequence of outdoor temperatures (cooling, zone time step)
        Array1D<Real64> CoolTstatTempSeq;      // daily sequence of zone thermostat temperatures (cooling, zone time step)
        Array1D<Real64> DesCoolSetPtSeq;       // daily sequence of indoor set point temperatures (zone time step)
        Array1D<Real64> HeatZoneHumRatSeq;     // daily sequence of zone humidity ratios (heating, zone time step)
        Array1D<Real64> CoolZoneHumRatSeq;     // daily sequence of zone humidity ratios (cooling, zone time step)
        Array1D<Real64> HeatOutHumRatSeq;      // daily sequence of outdoor humidity ratios (heating, zone time step)
        Array1D<Real64> CoolOutHumRatSeq;      // daily sequence of outdoor humidity ratios (cooling, zone time step)
        Real64 ZonePeakOccupancy = 0.0;        // zone peak occupancy based on max schedule value
        Real64 DOASHeatLoad = 0.0;             // current heating load from DOAS supply air [W]
        Real64 DOASCoolLoad = 0.0;             // current cooling load from DOAS supply air [W]
        Real64 DOASHeatAdd = 0.0;              // current heat addition rate from DOAS supply air [W]
        Real64 DOASLatAdd = 0.0;               // current latent heat addition rate from DOAS supply air [W]
        Real64 DOASSupMassFlow = 0.0;          // current mass flow rate of DOAS supply air [kg/s]
        Real64 DOASSupTemp = 0.0;              // current DOAS supply air temperature [C]
        Real64 DOASSupHumRat = 0.0;            // current DOAS supply air humidity ratio [kgWater/kgDryAir]
        Real64 DOASTotCoolLoad = 0.0;          // current total cooling load imposed by DOAS supply air [W]
        Array1D<Real64> DOASHeatLoadSeq;       // daily sequence of zone DOAS heating load (zone time step) [W]
        Array1D<Real64> DOASCoolLoadSeq;       // daily sequence of zone DOAS cooling load (zone time step) [W]
        Array1D<Real64> DOASHeatAddSeq;        // daily sequence of zone DOAS heat addition rate (zone time step) [W]
        Array1D<Real64> DOASLatAddSeq;         // daily sequence of zone DOAS latent heat addition rate (zone time step) [W]
        Array1D<Real64> DOASSupMassFlowSeq;    // daily sequence of zone DOAS supply mass flow rate (zone time step) [Kg/s]
        Array1D<Real64> DOASSupTempSeq;        // daily sequence of zone DOAS supply temperature (zone time step) [C]
        Array1D<Real64> DOASSupHumRatSeq;      // daily sequence of zone DOAS supply humidity ratio (zone time step) [kgWater/kgDryAir]
        Array1D<Real64> DOASTotCoolLoadSeq;    // daily sequence of zone DOAS total cooling load (zone time step) [W]

        // Latent heat variables
        Real64 HeatLoadNoDOAS = 0.0;                       // current zone heating load no DOAS (HVAC time step)
        Real64 CoolLoadNoDOAS = 0.0;                       // current zone heating load no DOAS (HVAC time step)
        Real64 DesHeatLoadNoDOAS = 0.0;                    // design zone heating load no DOAS (HVAC time step)
        Real64 DesCoolLoadNoDOAS = 0.0;                    // design zone heating load no DOAS (HVAC time step)
        Real64 HeatLatentLoad = 0.0;                       // current zone humidification load (HVAC time step)
        Real64 CoolLatentLoad = 0.0;                       // current zone dehumidification load (HVAC time step)
        Real64 HeatLatentLoadNoDOAS = 0.0;                 // current zone humidification load without DOAS (HVAC time step)
        Real64 CoolLatentLoadNoDOAS = 0.0;                 // current zone dehumidification load without DOAS (HVAC time step)
        Real64 ZoneHeatLatentMassFlow = 0.0;               // current mass flow rate required to meet humidification load [kg/s]
        Real64 ZoneCoolLatentMassFlow = 0.0;               // current mass flow rate required to meet dehumidification load [kg/s]
        Real64 ZoneHeatLatentVolFlow = 0.0;                // current volume flow rate required to meet humidification load [m3/s]
        Real64 ZoneCoolLatentVolFlow = 0.0;                // current volume flow rate required to meet dehumidification load [m3/s]
        Real64 DesLatentHeatLoad = 0.0;                    // design zone humidification load (HVAC time step)
        Real64 DesLatentCoolLoad = 0.0;                    // design zone dehumidification load (HVAC time step)
        Real64 DesLatentHeatLoadNoDOAS = 0.0;              // design zone humidification load no DOAS (HVAC time step)
        Real64 DesLatentCoolLoadNoDOAS = 0.0;              // design zone dehumidification load no DOAS (HVAC time step)
        Real64 DesLatentHeatMassFlow = 0.0;                // design mass flow rate required to meet humidification load [kg/s]
        Real64 DesLatentCoolMassFlow = 0.0;                // design mass flow rate required to meet dehumidification load [kg/s]
        Real64 DesLatentHeatVolFlow = 0.0;                 // design volume flow rate required to meet humidification load [kg/s]
        Real64 DesLatentCoolVolFlow = 0.0;                 // design volume flow rate required to meet dehumidification load [kg/s]
        Real64 ZoneTempAtLatentCoolPeak = 0.0;             // zone temp at max latent cooling [C]
        Real64 OutTempAtLatentCoolPeak = 0.0;              // outdoor temp at max latent cooling [C]
        Real64 ZoneHumRatAtLatentCoolPeak = 0.0;           // zone humrat at max latent cooling [kg/kg]
        Real64 OutHumRatAtLatentCoolPeak = 0.0;            // outdoor humrat at max latent cooling [kg/kg]
        Real64 ZoneTempAtLatentHeatPeak = 0.0;             // zone temp at max latent heating [C]
        Real64 OutTempAtLatentHeatPeak = 0.0;              // outdoor temp at max latent heating [C]
        Real64 ZoneHumRatAtLatentHeatPeak = 0.0;           // zone humrat at max latent heating [kg/kg]
        Real64 OutHumRatAtLatentHeatPeak = 0.0;            // outdoor humrat at max latent heating [kg/kg]
        Real64 DesLatentHeatCoilInTemp = 0.0;              // zone latent heating coil design air inlet temperature [C]
        Real64 DesLatentCoolCoilInTemp = 0.0;              // zone latent cooling coil design air inlet temperature [C]
        Real64 DesLatentHeatCoilInHumRat = 0.0;            // zone latent heating coil design air inlet humidity ratio [kg/kg]
        Real64 DesLatentCoolCoilInHumRat = 0.0;            // zone latent cooling coil design air inlet humidity ratio [kg/kg]
        int TimeStepNumAtLatentHeatMax = 0;                // time step number (in day) at heating peak
        int TimeStepNumAtLatentCoolMax = 0;                // time step number (in day) at cooling peak
        int TimeStepNumAtLatentHeatNoDOASMax = 0;          // time step number (in day) at latent heating peak without DOAS
        int TimeStepNumAtLatentCoolNoDOASMax = 0;          // time step number (in day) at Latent cooling peak without DOAS
        int LatentHeatDDNum = 0;                           // design day index of design day causing heating peak
        int LatentCoolDDNum = 0;                           // design day index of design day causing cooling peak
        int LatentHeatNoDOASDDNum = 0;                     // design day index of design day causing latent heating peak with no DOAS
        int LatentCoolNoDOASDDNum = 0;                     // design day index of design day causing latent cooling peak with no DOAS
        std::string cLatentHeatDDDate;                     // date of design day causing heating peak
        std::string cLatentCoolDDDate;                     // date of design day causing cooling peak
        int TimeStepNumAtHeatNoDOASMax = 0;                // time step number (in day) at Heating peak without DOAS
        int TimeStepNumAtCoolNoDOASMax = 0;                // time step number (in day) at cooling peak without DOAS
        int HeatNoDOASDDNum = 0;                           // design day index of design day causing heating peak without DOAS
        int CoolNoDOASDDNum = 0;                           // design day index of design day causing cooling peak without DOAS
        std::string cHeatNoDOASDDDate;                     // date of design day causing heating peak without DOAS
        std::string cCoolNoDOASDDDate;                     // date of design day causing cooling peak without DOAS
        Array1D<Real64> HeatLoadNoDOASSeq;                 // daily sequence of zone heating load No DOAS (zone time step)
        Array1D<Real64> CoolLoadNoDOASSeq;                 // daily sequence of zone cooling load No DOAS (zone time step)
        Array1D<Real64> LatentHeatLoadSeq;                 // daily sequence of zone latent heating load (zone time step) [W]
        Array1D<Real64> LatentCoolLoadSeq;                 // daily sequence of zone latent cooling load (zone time step) [W]
        Array1D<Real64> HeatLatentLoadNoDOASSeq;           // daily sequence of zone latent heating load No DOAS (zone time step) [W]
        Array1D<Real64> CoolLatentLoadNoDOASSeq;           // daily sequence of zone latent cooling load No DOAS (zone time step) [W]
        Array1D<Real64> LatentCoolFlowSeq;                 // daily sequence of zone latent cooling supply mass flow rate (zone time step) [Kg/s]
        Array1D<Real64> LatentHeatFlowSeq;                 // daily sequence of zone latent heating supply mass flow rate (zone time step) [Kg/s]
        bool zoneLatentSizing = false;                     // trigger to do RH control during zone sizing
        Real64 zoneRHDehumidifySetPoint = 50.0;            // RH dehumidifying set point used during sizing, default to 50%
        Sched::Schedule *zoneRHDehumidifySched = nullptr;  // zone RH dehumidifying schedule used for zone sizing
        Real64 zoneRHHumidifySetPoint = 50.0;              // RH humidifying set point used during sizing, default to 50%
        Sched::Schedule *zoneRHHumidifySched = nullptr;    // zone RH humidifying schedule used for zone sizing
        Real64 LatentCoolDesHumRat = 0.0;                  // zone design dehumidification supply air humidity ratio [kgw/kga]
        Real64 CoolDesHumRatDiff = 0.005;                  // zone design cooling supply air humidity ratio difference [deltakgw/kga]
        Real64 LatentHeatDesHumRat = 0.0;                  // zone design humidification supply air humidity ratio [kgw/kga]
        Real64 HeatDesHumRatDiff = 0.005;                  // zone design heating supply air humidity ratio temperature difference [deltakgw/kga]
        int ZnLatCoolDgnSAMethod = 0;                      // choice of how to get zone latent cooling design air humidity ratio;
        int ZnLatHeatDgnSAMethod = 0;                      // choice of how to get zone latent heating design air humidity ratio;
        Real64 ZoneRetTempAtLatentCoolPeak = 0.0;          // zone return temp at latent cooling peak time step
        Real64 ZoneRetTempAtLatentHeatPeak = 0.0;          // zone return temp at latent heating peak time step
        std::string CoolNoDOASDesDay;                      // name of a cooling design day without DOAS
        std::string HeatNoDOASDesDay;                      // name of a heating design day without DOAS
        std::string LatCoolDesDay;                         // name of a cooling design day
        std::string LatHeatDesDay;                         // name of a heating design day
        std::string LatCoolNoDOASDesDay;                   // name of a cooling design day without DOAS
        std::string LatHeatNoDOASDesDay;                   // name of a heating design day without DOAS
        ZoneSizing zoneSizingMethod = ZoneSizing::Invalid; // load to sizing on: sensible, latent, sensibleandlatent, sensibleonlynolatent
        std::string CoolSizingType;                        // string reported to eio, Cooling or Latent Cooling
        std::string HeatSizingType;                        // string reported to eio, Heating or Latent Heating
        std::string CoolPeakDateHrMin;                     // date:hr:min of cooling peak
        std::string HeatPeakDateHrMin;                     // date:hr:min of heating peak
        std::string LatCoolPeakDateHrMin;                  // date:hr:min of latent cooling peak
        std::string LatHeatPeakDateHrMin;                  // date:hr:min of latent heating peak
        HeatCoilSizMethod heatCoilSizingMethod = HeatCoilSizMethod::Invalid; // Used for sizing heat pumps
        Real64 maxHeatCoilToCoolingLoadSizingRatio = 0.0;                    // Used for sizing heat pumps

        void zeroMemberData();
        void allocateMemberArrays(int numOfTimeStepInDay);
    };

    struct TermUnitZoneSizingData : TermUnitZoneSizingCommonData
    {
        void scaleZoneCooling(Real64 ratio);
        void scaleZoneHeating(Real64 ratio);
        void copyFromZoneSizing(DataSizing::ZoneSizingData const &sourceData);
        void allocateMemberArrays(int numOfTimeStepInDay);
    };

    struct TermUnitSizingData
    {
        // Members
        int CtrlZoneNum;               // Controlled zone number (index to FinalZoneSizing, etc.)
        std::string ADUName;           // Terminal Unit Name (air distribution unit or direct air unit)
        Real64 AirVolFlow;             // design air vol flow rate for single duct terminal unit [m3/s]
        Real64 MaxHWVolFlow;           // design Hot Water vol flow for single duct terminal unit [m3/s]
        Real64 MaxSTVolFlow;           // design Steam vol flow rate for single duct terminal unit [m3/s]
        Real64 MaxCWVolFlow;           // design Cold Water vol flow for single duct terminal unit [m3/s]
        Real64 MinPriFlowFrac;         // design minimum primary flow fraction for a terminal unit
        Real64 InducRat;               // design induction ratio for a terminal unit
        bool InducesPlenumAir;         // True if secondary air comes from the plenum
        Real64 ReheatAirFlowMult;      // multiplier for air flow in reheat coil UA calculation
        Real64 ReheatLoadMult;         // multiplier for load in reheat coil UA calculation
        Real64 DesCoolingLoad;         // design cooling load used for zone equipment [W]
        Real64 DesHeatingLoad;         // design heating load used for zone equipment [W]
        Real64 SpecDesSensCoolingFrac; // Fraction of Design Sensible Cooling Load from DesignSpecification:AirTerminal:Sizing
        Real64 SpecDesCoolSATRatio;    // Cooling Design Supply Air Temperature Difference Ratio from DesignSpecification:AirTerminal:Sizing
        Real64 SpecDesSensHeatingFrac; // Fraction of Design Sensible Heating Load from DesignSpecification:AirTerminal:Sizing
        Real64 SpecDesHeatSATRatio;    // Heating Design Supply Air Temperature Difference Ratio from DesignSpecification:AirTerminal:Sizing
        Real64 SpecMinOAFrac;          // Fraction of Minimum Outdoor Air Flow from DesignSpecification:AirTerminal:Sizing
        int plenumIndex = 0;           // plenum index for PIU inlet conditions

        // Default Constructor
        TermUnitSizingData()
            : CtrlZoneNum(0), AirVolFlow(0.0), MaxHWVolFlow(0.0), MaxSTVolFlow(0.0), MaxCWVolFlow(0.0), MinPriFlowFrac(0.0), InducRat(0.0),
              InducesPlenumAir(false), ReheatAirFlowMult(1.0), ReheatLoadMult(1.0), DesCoolingLoad(0.0), DesHeatingLoad(0.0),
              SpecDesSensCoolingFrac(1.0), SpecDesCoolSATRatio(1.0), SpecDesSensHeatingFrac(1.0), SpecDesHeatSATRatio(1.0), SpecMinOAFrac(1.0)
        {
        }

        Real64 applyTermUnitSizingCoolFlow(Real64 coolFlowWithOA, // Cooling flow rate with MinOA limit applied
                                           Real64 coolFlowNoOA    // Cooling flow rate without MinOA limit applied
        );

        Real64 applyTermUnitSizingHeatFlow(Real64 heatFlowWithOA, // Heating flow rate with MinOA limit applied
                                           Real64 heatFlowNoOA    // Heating flow rate without MinOA limit applied
        );

        Real64 applyTermUnitSizingCoolLoad(Real64 coolLoad) const
        {
            return coolLoad * this->SpecDesSensCoolingFrac; // Apply DesignSpecification:AirTerminal:Sizing to cooling load
        }

        Real64 applyTermUnitSizingHeatLoad(Real64 heatLoad) const
        {
            return heatLoad * this->SpecDesSensHeatingFrac; // Apply DesignSpecification:AirTerminal:Sizing to heating load
        }
    };

    struct ZoneEqSizingData // data saved from zone eq component sizing and passed to subcomponents
    {
        // Members
        Real64 AirVolFlow;            // design air vol flow rate for zone equipment unit [m3/s]
        Real64 MaxHWVolFlow;          // design Hot Water vol flow for zone equipment unit [m3/s]
        Real64 MaxCWVolFlow;          // design Cold Water vol flow for zone equipment unit [m3/s]
        Real64 OAVolFlow;             // design outside air flow for zone equipment unit [m3/s]
        Real64 ATMixerVolFlow;        // design ventilation air flow rate from air terminal mixer (central DOAS) [m3/s]
        Real64 ATMixerCoolPriDryBulb; // design ventilation drybulb temperature from air terminal mixer during cooling (central DOAS) [C]
        Real64 ATMixerCoolPriHumRat;  // design ventilation humidity ratio from air terminal mixer during cooling (central DOAS) [kgWater/kgDryAir]
        Real64 ATMixerHeatPriDryBulb; // design ventilation drybulb temperature from air terminal mixer during heating (central DOAS) [C]
        Real64 ATMixerHeatPriHumRat;  // design ventilation humidity ratio from air terminal mixer during heating (central DOAS) [kgWater/kgDryAir]
        Real64 DesCoolingLoad;        // design cooling load used for zone equipment [W]
        Real64 DesHeatingLoad;        // design heating load used for zone equipment [W]
        Real64 CoolingAirVolFlow;     // design cooling air vol flow rate for equipment[m3/s]
        Real64 HeatingAirVolFlow;     // design heating air vol flow rate for equipment[m3/s]
        Real64 SystemAirVolFlow;      // design heating air vol flow rate for equipment[m3/s]
        bool AirFlow;                 // TRUE if AirloopHVAC system air flow rate is calculated
        bool CoolingAirFlow;          // TRUE if AirloopHVAC system cooling air flow rate is calculated
        bool HeatingAirFlow;          // TRUE if AirloopHVAC system heating air flow rate is calculated
        bool SystemAirFlow;           // TRUE if AirloopHVAC system heating air flow rate is calculated
        bool Capacity;                // TRUE if AirloopHVAC system capacity is calculated
        bool CoolingCapacity;         // TRUE if AirloopHVAC system cooling capacity is calculated
        bool HeatingCapacity;         // TRUE if AirloopHVAC system heating capacity is calculated
        bool SystemCapacity;          // TRUE if AirloopHVAC system heating capacity is calculated
        bool DesignSizeFromParent;    // TRUE if design size is set by parent object - normally false, set to true for special cases e.g. ERV
        int HVACSizingIndex;          // index to DesignSpecification:ZoneHVAC:Sizing
        Array1D_int SizingMethod;    // supply air flow rate sizing method (SupplyAirFlowRate, FlowPerFloorArea, FractionOfAutosizedCoolingAirflow and
                                     // FractionOfAutosizedHeatingAirflow)
        Array1D_int CapSizingMethod; // capacity sizing methods (HeatingDesignCapacity, CoolingDesignCapacity, CapacityPerFloorArea,
                                     // FractionOfAutosizedCoolingCapacity and FractionOfAutosizedHeatingCapacity )

        // Default Constructor
        ZoneEqSizingData()
            : AirVolFlow(0.0), MaxHWVolFlow(0.0), MaxCWVolFlow(0.0), OAVolFlow(0.0),
              ATMixerVolFlow(0.0),         // design ventilation air flow rate from air terminal mixer (central DOAS) [m3/s]
              ATMixerCoolPriDryBulb(0.0),  // design air terminal mixer cooling outlet temperature [C]
              ATMixerCoolPriHumRat(0.0),   // design air terminal mixer cooling outlet humidity ratio [kgWater/kgDryAir]
              ATMixerHeatPriDryBulb(0.0),  // design air terminal mixer heating outlet temperature [C]
              ATMixerHeatPriHumRat(0.0),   // design air terminal mixer heating outlet humidity ratio [kgWater/kgDryAir]
              DesCoolingLoad(0.0),         // design cooling load used for zone equipment [W]
              DesHeatingLoad(0.0),         // design heating load used for zone equipment [W]
              CoolingAirVolFlow(0.0),      // design cooling air vol flow rate for equipment[m3/s]
              HeatingAirVolFlow(0.0),      // design heating air vol flow rate for equipment[m3/s]
              SystemAirVolFlow(0.0),       // design heating air vol flow rate for equipment[m3/s]
              AirFlow(false),              // TRUE if AirloopHVAC system air flow rate is calculated
              CoolingAirFlow(false),       // TRUE if AirloopHVAC system cooling air flow rate is calculated
              HeatingAirFlow(false),       // TRUE if AirloopHVAC system heating air flow rate is calculated
              SystemAirFlow(false),        // TRUE if AirloopHVAC system heating air flow rate is calculated
              Capacity(false),             // TRUE if AirloopHVAC system capacity is calculated
              CoolingCapacity(false),      // TRUE if AirloopHVAC system cooling capacity is calculated
              HeatingCapacity(false),      // TRUE if AirloopHVAC system heating capacity is calculated
              SystemCapacity(false),       // TRUE if AirloopHVAC system heating capacity is calculated
              DesignSizeFromParent(false), // TRUE if design size is set by parent object - normally false, set to true for special cases e.g. ERV
              HVACSizingIndex(0)           // index to DesignSpecification:ZoneHVAC:Sizing
        {
        }
    };

    // Data Structure for Zone HVAC sizing, referenced by various ZoneHVAC Equipment
    struct ZoneHVACSizingData
    {
        // Members
        std::string Name;
        int CoolingSAFMethod;           // - Method for cooling supply air flow rate sizing calculation (SupplyAirFlowRate,FlowPerFloorArea,
                                        // FractionOfAutoSizedCoolingValue, FlowPerCoolingCapacity)
        int HeatingSAFMethod;           // - Method for heating supply air flow rate sizing calculation (SupplyAirFlowRate,FlowPerFloorArea,
                                        // FractionOfAutoSizedHeatingValue, FlowPerHeatingCapacity,
        int NoCoolHeatSAFMethod;        // - Method for supply air flow sizing during no cooling and heating calculation (SupplyAirFlowRate,
                                        // FractionOfAutoSizedCoolingValue, FractionOfAutoSizedHeatingValue)
        int CoolingCapMethod;           // - Method for cooling capacity scaledsizing calculation (CoolingDesignCapacity, CapacityPerFloorArea,
                                        // FractionOfAutosizedHeatingCapacity)
        int HeatingCapMethod;           // - Method for heatiing capacity scaledsizing calculation (HeatingDesignCapacity, CapacityPerFloorArea,
                                        // FracOfAutosizedHeatingCapacity)
        Real64 MaxCoolAirVolFlow;       // - maximum cooling supply air flow rate, m3/s
        Real64 MaxHeatAirVolFlow;       // - maximum heating supply air flow rate, m3/s
        Real64 MaxNoCoolHeatAirVolFlow; // - maximum supply air flow rate when no cooling or heating, m3/s
        Real64 ScaledCoolingCapacity;   // - scaled maximum cooling capacity of zone HVAC equipment, W
        Real64 ScaledHeatingCapacity;   // - scaled maximum heating capacity of zone HVAC equipment, W
        bool RequestAutoSize;           // - true if autosizing is requested
        HeatCoilSizMethod heatCoilSizingMethod = HeatCoilSizMethod::Invalid; // Used for sizing heat pumps
        Real64 maxHeatCoilToCoolingLoadSizingRatio = 0.0;                    // Used for sizing heat pumps

        // Default Constructor
        ZoneHVACSizingData()
            : CoolingSAFMethod(0), HeatingSAFMethod(0), NoCoolHeatSAFMethod(0), CoolingCapMethod(0), HeatingCapMethod(0), MaxCoolAirVolFlow(0.0),
              MaxHeatAirVolFlow(0.0), MaxNoCoolHeatAirVolFlow(0.0), ScaledCoolingCapacity(0.0), ScaledHeatingCapacity(0.0), RequestAutoSize(false)
        {
        }
    };

    // Data Structure for air terminal sizing, referenced by ZoneHVAC:AirDistributionUnit
    struct AirTerminalSizingSpecData
    {
        // Members
        std::string Name;
        Real64 DesSensCoolingFrac; // Fraction of Design Sensible Cooling Load
        Real64 DesCoolSATRatio;    // Cooling Design Supply Air Temperature Difference Ratio
        Real64 DesSensHeatingFrac; // Fraction of Design Sensible Heating Load
        Real64 DesHeatSATRatio;    // Heating Design Supply Air Temperature Difference Ratio
        Real64 MinOAFrac;          // Fraction of Minimum Outdoor Air Flow

        // Default Constructor
        AirTerminalSizingSpecData() : DesSensCoolingFrac(1.0), DesCoolSATRatio(1.0), DesSensHeatingFrac(1.0), DesHeatSATRatio(1.0), MinOAFrac(1.0)
        {
        }
    };

    struct SystemSizingInputData
    {
        // Members
        std::string AirPriLoopName;                      // name of an AirLoopHVAC object
        int AirLoopNum = 0;                              // index number of air loop
        LoadSizing loadSizingType = LoadSizing::Invalid; // type of load to size on sensible, latent, total, ventilation
        DataSizing::SizingConcurrence SizingOption = DataSizing::SizingConcurrence::NonCoincident; // noncoincident, coincident
        OAControl CoolOAOption = OAControl::Invalid; // 1 = use 100% outside air; 2 = use min OA; for cooling sizing
        OAControl HeatOAOption = OAControl::Invalid; // 1 = use 100% outside air; 2 = use min OA; for heating sizing
        Real64 DesOutAirVolFlow = 0.0;               // design (minimum) outside air flow rate [m3/s]
        Real64 SysAirMinFlowRat = 0.0;               // minimum system air flow ratio for heating, Central Heating Maximum System Air Flow Ratio
        bool SysAirMinFlowRatWasAutoSized = false;   // true if central heating maximum system air flow ratio was autosize on input
        Real64 PreheatTemp = 0.0;                    // preheat design set temperature [C]
        Real64 PrecoolTemp = 0.0;                    // precool design set temperature [C]
        Real64 PreheatHumRat = 0.0;                  // preheat design humidity ratio [kg water/kg dry air]
        Real64 PrecoolHumRat = 0.0;                  // precool design humidity ratio [kg water/kg dry air]
        Real64 CoolSupTemp = 0.0;                    // cooling design supply air temperature [C]
        Real64 HeatSupTemp = 0.0;                    // heating design supply air temperature [C]
        Real64 CoolSupHumRat = 0.0;                  // cooling design supply air humidity ratio [kg water/kg dry air]
        Real64 HeatSupHumRat = 0.0;                  // heating design supply air humidity ratio [kg water/kg dry air]
        AirflowSizingMethod CoolAirDesMethod = AirflowSizingMethod::Invalid; // choice of how to get system cooling design air flow rates;
        //  1 = calc from des day simulation; 2=m3/s per system, user input
        Real64 DesCoolAirFlow = 0.0;                                         // design system supply air flow rate for cooling[m3/s]
        AirflowSizingMethod HeatAirDesMethod = AirflowSizingMethod::Invalid; // choice of how to get system heating design air flow rates;
        //  1 = calc from des day simulation; 2=m3/s per zone, user input
        Real64 DesHeatAirFlow = 0.0; // design system heating supply air flow rate [m3/s]
        int ScaleCoolSAFMethod = 0;  // choice of how to get system cooling scalable air flow rates; // (FlowPerFloorArea,
                                     // FractionOfAutosizedCoolingAirflow, FlowPerCoolingCapacity)
        int ScaleHeatSAFMethod = 0;  // choice of how to get system heating scalable air flow rates; // (FlowPerFloorArea,
                                     // FractionOfAutosizedCoolingAirflow, FractionOfAutosizedHeatingAirflow, FlowPerHeatingCapacity)
        SysOAMethod SystemOAMethod = SysOAMethod::Invalid; // System Outdoor Air Method; 1 = SOAM_ZoneSum, 2 = SOAM_VRP, 9 = SOAM_SP
        Real64 MaxZoneOAFraction = 0.0;                    // maximum value of min OA for zones served by system
        bool OAAutoSized = false;                          // Set to true if design OA vol flow is set to 'autosize' in Sizing:System
        int CoolingCapMethod = 0;           // - Method for cooling capacity scaledsizing calculation (CoolingDesignCapacity, CapacityPerFloorArea,
                                            // FractionOfAutosizedCoolingCapacity)
        int HeatingCapMethod = 0;           // - Method for heatiing capacity scaledsizing calculation (HeatingDesignCapacity, CapacityPerFloorArea,
                                            // FracOfAutosizedHeatingCapacity)
        Real64 ScaledCoolingCapacity = 0.0; // - scaled maximum cooling capacity of cooling coil in an air loop
        Real64 ScaledHeatingCapacity = 0.0; // - scaled maximum heating capacity of cooling coil in an air loop
        Real64 FloorAreaOnAirLoopCooled = 0.0; // total floor of cooled zones served by an airloop
        Real64 FloorAreaOnAirLoopHeated = 0.0; // total floor of heated zones served by an airloop
        Real64 FlowPerFloorAreaCooled = 0.0;   // ratio of cooling supply air flow rate to total floor area of cooled zones served by an airloop
        Real64 FlowPerFloorAreaHeated = 0.0;   // ratio of cooling supply air flow rate to total floor area of cooled zones served by an airloop
        Real64 FractionOfAutosizedCoolingAirflow = 1.0;            // fraction of of cooling supply air flow rate an airloop
        Real64 FractionOfAutosizedHeatingAirflow = 1.0;            // fraction of of heating supply air flow rate an airloop
        Real64 FlowPerCoolingCapacity = 0.0;                       // ratio of cooling supply air flow rate to cooling capacity of an airloop
        Real64 FlowPerHeatingCapacity = 0.0;                       // ratio of heating supply air flow rate to heating capacity of an airloop
        PeakLoad coolingPeakLoad = PeakLoad::Invalid;              // Type of peak to size cooling coils on SensibleCooling or TotalCooling
        CapacityControl CoolCapControl = CapacityControl::Invalid; // type of control of cooling coil  VAV, Bypass, VT, OnOff
        Real64 OccupantDiversity = 0.0;                            // occupant diversity
        HeatCoilSizMethod heatCoilSizingMethod = HeatCoilSizMethod::Invalid; // Used for sizing heat pumps
        Real64 maxHeatCoilToCoolingLoadSizingRatio = 0.0;                    // Used for sizing heat pumps
    };

    struct SystemSizingData // Contains data for system sizing
    {
        // Members
        std::string AirPriLoopName;                      // name of an AirLoopHVAC object
        std::string CoolDesDay;                          // name of a cooling design day
        std::string HeatDesDay;                          // name of a heating design day
        LoadSizing loadSizingType = LoadSizing::Invalid; // type of load to size on Sensible, Latent, Total, Ventilation
        DataSizing::SizingConcurrence SizingOption = DataSizing::SizingConcurrence::NonCoincident; // noncoincident, coincident.
        OAControl CoolOAOption = OAControl::Invalid; // 1 = use 100% outside air; 2 = use min OA; for cooling sizing
        OAControl HeatOAOption = OAControl::Invalid; // 1 = use 100% outside air; 2 = use min OA; for heating sizing
        Real64 DesOutAirVolFlow = 0.0;               // design (minimum) outside air flow rate [m3/s]
        Real64 SysAirMinFlowRat = 0.0;               // minimum system air flow ratio for heating, Central Heating Maximum System Air Flow Ratio
        bool SysAirMinFlowRatWasAutoSized = false;   // true if central heating maximum system air flow ratio was autosize on input
        Real64 PreheatTemp = 0.0;                    // preheat design set temperature
        Real64 PrecoolTemp = 0.0;                    // precool design set temperature [C]
        Real64 PreheatHumRat = 0.0;                  // preheat design humidity ratio [kg water/kg dry air]
        Real64 PrecoolHumRat = 0.0;                  // precool design humidity ratio [kg water/kg dry air]
        Real64 CoolSupTemp = 0.0;                    // cooling design supply air temperature [C]
        Real64 HeatSupTemp = 0.0;                    // heating design supply air temperature[C]
        Real64 CoolSupHumRat = 0.0;                  // cooling design supply air humidity ratio [kg water/kg dry air]
        Real64 HeatSupHumRat = 0.0;                  // heating design supply air humidity ratio [kg water/kg dry air]
        AirflowSizingMethod CoolAirDesMethod = AirflowSizingMethod::Invalid; // choice of how to get system design cooling air flow rates;
        //  1 = calc from des day simulation; 2=m3/s per system, user input
        AirflowSizingMethod HeatAirDesMethod = AirflowSizingMethod::Invalid; // choice of how to get system design heating air flow rates;
        //  1 = calc from des day simulation; 2=m3/s per system, user input
        Real64 InpDesCoolAirFlow = 0.0;                // input design system supply air flow rate [m3/s]
        Real64 InpDesHeatAirFlow = 0.0;                // input design system heating supply air flow rate [m3/s]
        Real64 CoinCoolMassFlow = 0.0;                 // coincident peak cooling mass flow rate [kg/s]
        bool EMSOverrideCoinCoolMassFlowOn = false;    // If true, EMS to change coincident peak cooling mass flow rate
        Real64 EMSValueCoinCoolMassFlow = 0.0;         // Value EMS wants for coincident peak cooling mass flow rate [kg/s]
        Real64 CoinHeatMassFlow = 0.0;                 // coincident peak heating mass flow rate [kg/s]
        bool EMSOverrideCoinHeatMassFlowOn = false;    // If true, EMS to set coincident peak heating mass flow rate
        Real64 EMSValueCoinHeatMassFlow = 0.0;         // Value EMS wants for coincident peak heating mass flow rate [kg/s]
        Real64 NonCoinCoolMassFlow = 0.0;              // noncoincident peak cooling mass flow rate [kg/s]
        bool EMSOverrideNonCoinCoolMassFlowOn = false; // true, EMS to set noncoincident peak cooling mass flow rate
        Real64 EMSValueNonCoinCoolMassFlow = 0.0;      // Value EMS for noncoincident peak cooling mass flow rate [kg/s]
        Real64 NonCoinHeatMassFlow = 0.0;              // noncoincident peak heating mass flow rate [kg/s]
        bool EMSOverrideNonCoinHeatMassFlowOn = false; // true, EMS to set noncoincident peak heating mass flow rate
        Real64 EMSValueNonCoinHeatMassFlow = 0.0;      // Value EMS for noncoincident peak heating mass flow rate [kg/s]
        Real64 DesMainVolFlow = 0.0;                   // design main supply duct volume flow [m3/s]
        bool EMSOverrideDesMainVolFlowOn = false;      // If true, EMS is acting to change DesMainVolFlow
        Real64 EMSValueDesMainVolFlow = 0.0;           // Value EMS providing for design main supply duct volume flow [m3/s]
        Real64 DesHeatVolFlow = 0.0;                   // design heat supply duct volume flow [m3/s]
        bool EMSOverrideDesHeatVolFlowOn = false;      // If true, EMS is acting to change DesCoolVolFlow
        Real64 EMSValueDesHeatVolFlow = 0.0;           // Value EMS providing for design cool  supply duct volume flow [m3/s]
        Real64 DesCoolVolFlow = 0.0;                   // design cool  supply duct volume flow [m3/s]
        bool EMSOverrideDesCoolVolFlowOn = false;      // If true, EMS is acting to change DesCoolVolFlow
        Real64 EMSValueDesCoolVolFlow = 0.0;           // Value EMS providing for design cool  supply duct volume flow [m3/s]
        Real64 SensCoolCap = 0.0;                      // design sensible cooling capacity [W]
        Real64 TotCoolCap = 0.0;                       // design total cooling capacity [W]
        Real64 HeatCap = 0.0;                          // design heating capacity [W]
        Real64 PreheatCap = 0.0;                       // design preheat capacity [W]
        Real64 MixTempAtCoolPeak = 0.0;                // design mixed air temperature for cooling [C]
        Real64 MixHumRatAtCoolPeak = 0.0;              // design mixed air hum ratio for cooling [kg water/kg dry air]
        Real64 RetTempAtCoolPeak = 0.0;                // design return air temperature for cooling [C]
        Real64 RetHumRatAtCoolPeak = 0.0;              // design return air hum ratio for cooling [kg water/kg dry air]
        Real64 OutTempAtCoolPeak = 0.0;                // design outside air temperature for cooling [C]
        Real64 OutHumRatAtCoolPeak = 0.0;              // design outside air hum ratio for cooling [kg water/kg dry air]
        Real64 MassFlowAtCoolPeak = 0.0;               // air mass flow rate at the cooling peak [kg/s]
        Real64 HeatMixTemp = 0.0;                      // design mixed air temperature for heating [C]
        Real64 HeatMixHumRat = 0.0;                    // design mixed air hum ratio for heating [kg water/kg dry air]
        Real64 HeatRetTemp = 0.0;                      // design return air temperature for heating [C]
        Real64 HeatRetHumRat = 0.0;                    // design return air hum ratio for heating [kg water/kg dry air]
        Real64 HeatOutTemp = 0.0;                      // design outside air temperature for heating [C]
        Real64 HeatOutHumRat = 0.0;                    // design outside air hum ratio for Heating [kg water/kg dry air]
        Real64 DesCoolVolFlowMin = 0.0;                // design minimum system cooling flow rate [m3/s]
        Array1D<Real64> HeatFlowSeq;                   // daily sequence of system heating air mass flow rate
        //  (zone time step)
        Array1D<Real64> SumZoneHeatLoadSeq; // daily sequence of zones summed heating load [W]
        //  (zone time step)
        Array1D<Real64> CoolFlowSeq; // daily sequence of system cooling air mass flow rate
        //  (zone time step)
        Array1D<Real64> SumZoneCoolLoadSeq; // daily sequence of zones summed cooling load [W]
        //  (zone time step)
        Array1D<Real64> CoolZoneAvgTempSeq; // daily sequence of zones flow weighted average temperature [C]
        //  (zone time step)
        Array1D<Real64> HeatZoneAvgTempSeq; // daily sequence of zones flow weighted average temperature [C]
        //  (zone time step)
        Array1D<Real64> SensCoolCapSeq; // daily sequence of system sensible cooling capacity
        //  (zone time step)
        Array1D<Real64> TotCoolCapSeq; // daily sequence of system total cooling capacity
        //  (zone time step)
        Array1D<Real64> HeatCapSeq;        // daily sequence of system heating capacity [zone time step]
        Array1D<Real64> PreheatCapSeq;     // daily sequence of system preheat capacity [zone time step]
        Array1D<Real64> SysCoolRetTempSeq; // daily sequence of system cooling return temperatures [C]
        //  [zone time step]
        Array1D<Real64> SysCoolRetHumRatSeq; // daily sequence of system cooling return humidity ratios
        //  [kg water/kg dry air] [zone time step]
        Array1D<Real64> SysHeatRetTempSeq; // daily sequence of system heating return temperatures [C]
        //   [zone time step]
        Array1D<Real64> SysHeatRetHumRatSeq; // daily sequence of system heating return humidity ratios
        //  [kg water/kg dry air] [zone time step]
        Array1D<Real64> SysCoolOutTempSeq; // daily sequence of system cooling outside temperatures [C]
        //  [zone time step]
        Array1D<Real64> SysCoolOutHumRatSeq; // daily sequence of system cooling outside humidity ratios
        //  [kg water/kg dry air] [zone time step]
        Array1D<Real64> SysHeatOutTempSeq; // daily sequence of system heating outside temperatures [C]
        //  [zone time step]
        Array1D<Real64> SysHeatOutHumRatSeq; // daily sequence of system heating outside humidity ratios
        //   [kg water/kg dry air] [zone time step]
        Array1D<Real64> SysDOASHeatAddSeq;                 // daily sequence of heat addition rate from DOAS supply air [W]
        Array1D<Real64> SysDOASLatAddSeq;                  // daily sequence of latent heat addition rate from DOAS supply air [W]
        SysOAMethod SystemOAMethod = SysOAMethod::Invalid; // System Outdoor Air Method; 1 = SOAM_ZoneSum, 2 = SOAM_VRP, 9 = SOAM_SP
        Real64 MaxZoneOAFraction = 0.0;                    // maximum value of min OA for zones served by system
        Real64 SysUncOA = 0.0;                             // uncorrected system outdoor air flow based on zone people and zone area
        bool OAAutoSized = false;                          // Set to true if design OA vol flow is set to 'autosize'
        int ScaleCoolSAFMethod = 0;                        // choice of how to get system cooling scalable air flow rates; (FlowPerFloorArea,
                                                           // FractionOfAutosizedCoolingAirflow,
                                                           // FlowPerCoolingCapacity)
        int ScaleHeatSAFMethod = 0;                        // choice of how to get system heating scalable air flow rates; (FlowPerFloorArea,
                                                           // FractionOfAutosizedCoolingAirflow,
                                                           // FractionOfAutosizedHeatingAirflow, FlowPerHeatingCapacity)
        int CoolingCapMethod = 0;           // - Method for cooling capacity scaledsizing calculation (CoolingDesignCapacity, CapacityPerFloorArea,
                                            // FractionOfAutosizedCoolingCapacity)
        int HeatingCapMethod = 0;           // - Method for heatiing capacity scaledsizing calculation (HeatingDesignCapacity, CapacityPerFloorArea,
                                            // FracOfAutosizedHeatingCapacity)
        Real64 ScaledCoolingCapacity = 0.0; // - scaled maximum cooling capacity of cooling coil in an air loop
        Real64 ScaledHeatingCapacity = 0.0; // - scaled maximum heating capacity of cooling coil in an air loop
        Real64 FloorAreaOnAirLoopCooled = 0.0; // total floor of cooled zones served by an airloop
        Real64 FloorAreaOnAirLoopHeated = 0.0; // total floor of heated zones served by an airloop
        Real64 FlowPerFloorAreaCooled = 0.0;   // ratio of cooling supply air flow rate to total floor area of cooled zones served by an airloop
        Real64 FlowPerFloorAreaHeated = 0.0;   // ratio of cooling supply air flow rate to total floor area of cooled zones served by an airloop
        Real64 FractionOfAutosizedCoolingAirflow = 0.0;            // fraction of of cooling supply air flow rate an airloop
        Real64 FractionOfAutosizedHeatingAirflow = 0.0;            // fraction of of heating supply air flow rate an airloop
        Real64 FlowPerCoolingCapacity = 0.0;                       // ratio of cooling supply air flow rate to cooling capacity of an airloop
        Real64 FlowPerHeatingCapacity = 0.0;                       // ratio of heating supply air flow rate to heating capacity of an airloop
        Real64 FractionOfAutosizedCoolingCapacity = 0.0;           // fraction of of cooling total capacity
        Real64 FractionOfAutosizedHeatingCapacity = 0.0;           // fraction of of heating total capacity
        Real64 CoolingTotalCapacity = 0.0;                         // system total cooling capacity
        Real64 HeatingTotalCapacity = 0.0;                         // system total heating capacity
        PeakLoad coolingPeakLoad = PeakLoad::Invalid;              // Type of peak to size cooling coils on SensibleCooling or TotalCooling
        CapacityControl CoolCapControl = CapacityControl::Invalid; // type of control of cooling coil  VAV, Bypass, VT, OnOff
        bool sysSizeHeatingDominant = false;
        bool sysSizeCoolingDominant = false;

        Real64 CoinCoolCoilMassFlow = 0.0; // coincident volume flow at time of cooling coil sensible+latent peak [m3/s]
        Real64 CoinHeatCoilMassFlow = 0.0; // coincident volume flow at time of heating coil sensible peak [m3/s]
        Real64 DesCoolCoilVolFlow = 0.0;   // design cooling air volume flow rate at time of coil sens+latent peak [m3/s]
        Real64 DesHeatCoilVolFlow = 0.0;   // design heating air volume flow rate at time of coil sens peak [m3/s]
        Real64 DesMainCoilVolFlow = 0.0;   // design main supply duct volume flow at time of coil peak [m3/s]
        // These are for reporting purposes

        int SysHeatCoilTimeStepPk = 0; // timestep in day of heating coil peak
        int SysHeatAirTimeStepPk = 0;  // timestep in day of heating airflow peak
        int HeatDDNum = 0;             // index of design day for heating
        int CoolDDNum = 0;             // index of design day for cooling

        Real64 SysCoolCoinSpaceSens = 0.0;                                   // sum of zone space sensible cooling loads at coincident peak
        Real64 SysHeatCoinSpaceSens = 0.0;                                   //  sum of zone space sensible heating loads at coincident peak
        Real64 SysDesCoolLoad = 0.0;                                         // system peak load with coincident
        int SysCoolLoadTimeStepPk = 0;                                       // timestep in day of cooling load peak
        Real64 SysDesHeatLoad = 0.0;                                         // system peak load with coincident
        int SysHeatLoadTimeStepPk = 0;                                       // timestep in day of cooling load peak
        HeatCoilSizMethod heatCoilSizingMethod = HeatCoilSizMethod::Invalid; // Used for sizing heat pumps
        Real64 maxHeatCoilToCoolingLoadSizingRatio = 0.0;                    // Used for sizing heat pumps
    };

    struct SysSizPeakDDNumData
    {
        // Members
        int SensCoolPeakDD;                // design day containing the sensible cooling peak
        std::string cSensCoolPeakDDDate;   // date string of design day causing sensible cooling peak
        int TotCoolPeakDD;                 // design day containing total cooling peak
        std::string cTotCoolPeakDDDate;    // date string of design day causing total cooling peak
        int CoolFlowPeakDD;                // design day containing the cooling air flow peak
        std::string cCoolFlowPeakDDDate;   // date string of design day causing cooling air flow peak
        int HeatPeakDD;                    // design day containing the heating peak
        std::string cHeatPeakDDDate;       // date string of design day causing heating peak
        Array1D<int> TimeStepAtSensCoolPk; // time step of the sensible cooling peak
        Array1D<int> TimeStepAtTotCoolPk;  // time step of the total cooling peak
        Array1D<int> TimeStepAtCoolFlowPk; // time step of the cooling air flow peak
        Array1D<int> TimeStepAtHeatPk;     // time step of the heating peak

        // Default Constructor
        SysSizPeakDDNumData() : SensCoolPeakDD(0), TotCoolPeakDD(0), CoolFlowPeakDD(0), HeatPeakDD(0)
        {
        }
    };

    struct PlantSizingData
    {
        // Members
        std::string PlantLoopName;                           // name of PLANT LOOP or CONDENSER LOOP object
        TypeOfPlantLoop LoopType = TypeOfPlantLoop::Invalid; // type of loop: 1=heating, 2=cooling, 3=condenser
        Real64 ExitTemp = 0.0;                               // loop design exit (supply) temperature [C]
        Real64 DeltaT = 0.0;                                 // loop design temperature drop (or rise) [DelK]
        DataSizing::SizingConcurrence ConcurrenceOption =
            DataSizing::SizingConcurrence::NonCoincident; // sizing option for coincident or noncoincident (default)
        int NumTimeStepsInAvg = 1;                        // number of zone timesteps in the averaging window for coincident plant flow
        int SizingFactorOption = 0;                       // option for what sizing factor to apply
        // Calculated
        Real64 DesVolFlowRate = 0.0; // loop design flow rate in m3/s
        bool VolFlowSizingDone = 0;  // flag to indicate when this loop has finished sizing flow rate
        Real64 PlantSizFac = 0.0;    // hold the loop and pump sizing factor
        Real64 DesCapacity = 0.0;    // final capacity in W
    };

    // based on ZoneSizingData but only have member variables that are related to the CheckSum/
    struct FacilitySizingData
    {
        // Members
        int CoolDDNum;                    // design day index of design day causing heating peak
        int HeatDDNum;                    // design day index of design day causing heating peak
        int TimeStepNumAtCoolMax;         // time step number (in day) at cooling peak
        Array1D<Real64> DOASHeatAddSeq;   // daily sequence of zone DOAS heat addition rate (zone time step) [W]
        Array1D<Real64> DOASLatAddSeq;    // daily sequence of zone DOAS latent heat addition rate (zone time step) [W]
        Array1D<Real64> CoolOutHumRatSeq; // daily sequence of outdoor humidity ratios (cooling, zone time step)
        Array1D<Real64> CoolOutTempSeq;   // daily sequence of outdoor temperatures (cooling, zone time step)
        Array1D<Real64> CoolZoneTempSeq;  // daily sequence of zone temperatures (cooling, zone time step)
        Array1D<Real64> CoolLoadSeq;      // daily sequence of cooling load (cooling, zone time step)
        Real64 DesCoolLoad;               // zone design cooling load [W]
        int TimeStepNumAtHeatMax;         // time step number (in day) at Heating peak
        Array1D<Real64> HeatOutHumRatSeq; // daily sequence of outdoor humidity ratios (heating, zone time step)
        Array1D<Real64> HeatOutTempSeq;   // daily sequence of outdoor temperatures (heating, zone time step)
        Array1D<Real64> HeatZoneTempSeq;  // daily sequence of zone temperatures (heating, zone time step)
        Array1D<Real64> HeatLoadSeq;      // daily sequence of heating load (cooling, zone time step)
        Real64 DesHeatLoad;               // zone design heating load [W]

        // Default Constructor
        FacilitySizingData() : CoolDDNum(0), HeatDDNum(0), TimeStepNumAtCoolMax(0), DesCoolLoad(0.0), TimeStepNumAtHeatMax(0), DesHeatLoad(0.0)
        {
        }
    };

    struct DesDayWeathData
    {
        // Members
        std::string DateString; // date of design day weather values
        Array1D<Real64> Temp;   // design day temperatures at the major time step
        Array1D<Real64> HumRat; // design day humidity ratios at the major time step
        Array1D<Real64> Press;  // design day barometric pressure at the major time step
    };

    struct CompDesWaterFlowData // design water flow rate for components that use water as an
    {
        // Members
        //  energy source or sink
        int SupNode;           // water inlet node number (condenser side for water / water)
        Real64 DesVolFlowRate; // water design flow rate [m3/s]

        // Default Constructor
        CompDesWaterFlowData() : SupNode(0), DesVolFlowRate(0.0)
        {
        }

        // Member Constructor
        CompDesWaterFlowData(int SupNode,          // water inlet node number (condenser side for water / water)
                             Real64 DesVolFlowRate // water design flow rate [m3/s]
                             )
            : SupNode(SupNode), DesVolFlowRate(DesVolFlowRate)
        {
        }
    };

    struct OARequirementsData
    {
        // Holds complete data for a single DesignSpecification:OutdoorAir object or
        // a list of indexes from a DesignSpecification:OutdoorAir:SpaceList object
        std::string Name;                     // Name of DesignSpecification:OutdoorAir or DesignSpecification:OutdoorAir:SpaceList object
        int numDSOA = 0;                      // Number of DesignSpecification:OutdoorAir objects for this instance (zero if not a list)
        EPVector<int> dsoaIndexes;            // Indexes to DesignSpecification:OutdoorAir objects (if this is a DSOA:SpaceList object)
        EPVector<std::string> dsoaSpaceNames; // Names of spaces if this is a (if this is a DSOA:SpaceList object)
        EPVector<int> dsoaSpaceIndexes;       // Indexes to Spaces (if this is a DSOA:SpaceList object)
        OAFlowCalcMethod OAFlowMethod =
            OAFlowCalcMethod::PerPerson;            // - Method for OA flow calculation (Flow/Person, Flow/Zone, Flow/Area, FlowACH, Sum, Maximum)
        Real64 OAFlowPerPerson = 0.0;               // - OA requirement per person
        Real64 OAFlowPerArea = 0.0;                 // - OA requirement per zone area
        Real64 OAFlowPerZone = 0.0;                 // - OA requirement per zone
        Real64 OAFlowACH = 0.0;                     // - OA requirement per zone per hour
        Sched::Schedule *oaFlowFracSched = nullptr; // - Fraction schedule applied to total OA requirement
        Sched::Schedule *oaPropCtlMinRateSched = nullptr; // - Fraction schedule applied to Proportional Control Minimum Outdoor Air Flow Rate
        int CO2MaxMinLimitErrorCount = 0; // Counter when max CO2 concentration < min CO2 concentration for SOAM_ProportionalControlSchOcc
        int CO2MaxMinLimitErrorIndex = 0; // Index for max CO2 concentration < min CO2 concentration recurring error message for
                                          // SOAM_ProportionalControlSchOcc
        int CO2GainErrorCount = 0;        // Counter when CO2 generation from people is zero for SOAM_ProportionalControlSchOcc
        int CO2GainErrorIndex = 0; // Index for recurring error message when CO2 generation from people is zero for SOAM_ProportionalControlSchOcc
        bool myEnvrnFlag = true;

        Real64 oaFlowArea(EnergyPlusData &state,
                          int const zoneNum,
                          bool const useMinOASchFlag = true, // Use min OA schedule in DesignSpecification:OutdoorAir object
                          int const spaceNum = 0);           // Space index (if applicable)

        Real64 floorArea(EnergyPlusData &state, int const zoneNum, int const spaceNum = 0);

        Real64 desFlowPerZoneArea(EnergyPlusData &state, int const zoneNum, int const spaceNum = 0);

        Real64 oaFlowPeople(EnergyPlusData &state,
                            int const zoneNum,                 // Zone index
                            bool const useOccSchFlag = true,   // Use occupancy schedule
                            bool const useMinOASchFlag = true, // Use min OA schedule in DesignSpecification:OutdoorAir object
                            int const spaceNum = 0);           // Space index (if applicable)

        Real64 people(EnergyPlusData &state,
                      int const zoneNum,               // Zone index
                      bool const useOccSchFlag = true, // Use occupancy schedule
                      int const spaceNum = 0);         // Space index (if applicable)

        Real64 desFlowPerZonePerson(EnergyPlusData &state, int const zoneNum, int const spaceNum = 0);

        Real64 desFlowPerZone(EnergyPlusData &state, int const spaceNum = 0);

        Real64 desFlowPerACH(EnergyPlusData &state, int const spaceNum = 0);

        Real64 calcOAFlowRate(EnergyPlusData &state,
                              int ActualZoneNum,               // Zone index
                              bool UseOccSchFlag,              // Zone occupancy schedule will be used instead of using total zone occupancy
                              bool UseMinOASchFlag,            // Use min OA schedule in DesignSpecification:OutdoorAir object
                              bool const PerPersonNotSet,      // when calculation should not include occupants (e.g., dual duct)
                              bool const MaxOAVolFlowFlag,     // TRUE when calculation uses occupancy schedule  (e.g., dual duct)
                              int const spaceNum = 0,          // Space index (if applicable)
                              bool const calcIAQMethods = true // For IAQProcedure, PCOccSch, and PCDesOcc, calculate if true, return zero if false
        );

        Sched::Schedule *getZoneFlowFracSched(EnergyPlusData &state, bool notAllSame);

        Sched::Schedule *getZonePropCtlMinRateSched(EnergyPlusData &state, bool notAllSame);
    };

    struct ZoneAirDistributionData
    {
        // Members
        std::string Name;
        std::string ZoneADEffSchName;              // - Zone air distribution effectiveness schedule name
        Real64 ZoneADEffCooling;                   // - Zone air distribution effectiveness in cooling mode
        Real64 ZoneADEffHeating;                   // - Zone air distribution effectiveness in heating mode
        Real64 ZoneSecondaryRecirculation;         // - Zone air secondary recirculation ratio
        Sched::Schedule *zoneADEffSched = nullptr; // - Zone air distribution effectiveness schedule index
        Real64 ZoneVentilationEff;                 // Zone ventilation effectiveness

        // Default Constructor
        ZoneAirDistributionData() : ZoneADEffCooling(1.0), ZoneADEffHeating(1.0), ZoneSecondaryRecirculation(0.0), ZoneVentilationEff(0.0)
        {
        }

        Real64 calculateEz(EnergyPlusData &state, int ZoneNum); // Zone index
    };

    // Resets Data globals so that previously set variables are not used in other equipment models
    void resetHVACSizingGlobals(EnergyPlusData &state,
                                int curZoneEqNum,
                                int curSysNum,
                                bool &firstPassFlag // Can be set to false during the routine
    );

    void GetCoilDesFlowT(EnergyPlusData &state,
                         int SysNum,           // central air system index
                         Real64 CpAir,         // specific heat to be used in calculations [J/kgC]
                         Real64 &DesFlow,      // returned design mass flow [kg/s]
                         Real64 &DesExitTemp,  // returned design coil exit temperature [kg/s]
                         Real64 &DesExitHumRat // returned design coil exit humidity ratio [kg/kg]
    );

    Real64 calcDesignSpecificationOutdoorAir(
        EnergyPlusData &state,
        int const DSOAPtr,                   // Pointer to DesignSpecification:OutdoorAir object
        int const ActualZoneNum,             // Zone index
        bool const UseOccSchFlag,            // Zone occupancy schedule will be used instead of using total zone occupancy
        bool const UseMinOASchFlag,          // Use min OA schedule in DesignSpecification:OutdoorAir object
        bool const PerPersonNotSet = false,  // when calculation should not include occupants (e.g., dual duct)
        bool const MaxOAVolFlowFlag = false, // TRUE when calculation uses occupancy schedule  (e.g., dual duct)
        int const spaceNum = 0,              // Space index (if applicable)
        bool const calcIAQMethods = true);   // For IAQProcedure, PCOccSch, and PCDesOcc, calculate if true, return zero if false

    int getDefaultOAReq(EnergyPlusData &state); // get index to default OA requirements - used by Controller:MechanicalVentilation

    void setHeatPumpSize(EnergyPlusData &state, Real64 &coolingCap, Real64 &heatingCap, Real64 const sizingRatio);
} // namespace DataSizing

struct SizingData : BaseGlobalStruct
{
    int NumOARequirements = 0;                                  // Number of OA Requirements objects
    int OARequirements_Default = 0;                             // Index to default DesignSpecification:OutdoorAir
    int NumZoneAirDistribution = 0;                             // Number of zone air distribution objects
    int NumZoneSizingInput = 0;                                 // Number of Zone Sizing objects
    int NumSysSizInput = 0;                                     // Number of System Sizing objects
    int NumPltSizInput = 0;                                     // Number of Plant Sizing objects
    int CurSysNum = 0;                                          // Current Air System index (0 if not in air loop)
    int CurOASysNum = 0;                                        // Current outside air system index (0 if not in OA Sys)
    int CurZoneEqNum = 0;                                       // Current Zone Equipment index (0 if not simulating ZoneEq)
    int CurTermUnitSizingNum = 0;                               // Current terminal unit sizing index for TermUnitSizing and TermUnitFinalZoneSizing
    int CurBranchNum = 0;                                       // Index of branch being simulated (or 0 if not air loop)
    HVAC::AirDuctType CurDuctType = HVAC::AirDuctType::Invalid; // Duct type of current branch
    int CurLoopNum = 0;                                         // the current plant loop index
    int CurCondLoopNum = 0;                                     // the current condenser loop number
    int CurEnvirNumSimDay = 0;                                  // current environment number for day simulated
    int CurOverallSimDay = 0;                                   // current day of simulation
    int NumTimeStepsInAvg = 0;                                  // number of time steps in the averaging window for the design flow and load sequences
    int SaveNumPlantComps = 0;                                  // Number of components using water as an energy source or sink (e.g. water coils)
    int DataTotCapCurveIndex = 0;                               // index to total capacity as a function of temperature curve
    Real64 DataTotCapCurveValue = 0;                            // value of total capacity as a function of temperature curve for CoilVRF_FluidTCtrl_*
    int DataPltSizCoolNum = 0;                                  // index to cooling plant sizing data
    int DataPltSizHeatNum = 0;                                  // index to heating plant sizing data
    int DataWaterLoopNum = 0;                                   // index to plant water loop
    int DataCoilNum = 0;                                        // index to coil object
    HVAC::FanOp DataFanOp = HVAC::FanOp::Invalid;               // fan operating mode (FanOp::Continuous or FanOp::Cycling)
    bool DataCoilIsSuppHeater = false;                          // TRUE if heating coil used as supplemental heater
    bool DataIsDXCoil = false;                                  // TRUE if direct-expansion coil
    bool DataAutosizable = true;                                // TRUE if component is autosizable
    bool DataEMSOverrideON = false;                             // boolean determines if user relies on EMS to override autosizing
    bool DataScalableSizingON = false;                          // boolean determines scalable flow sizing is specified
    bool DataScalableCapSizingON = false;                       // boolean determines scalable capacity sizing is specified
    bool DataSysScalableFlowSizingON = false;                   // boolean determines scalable system flow sizing is specified
    bool DataSysScalableCapSizingON = false;                    // boolean determines scalable system capacity sizing is specified
    bool SysSizingRunDone = false;                              // True if a system sizing run is successfully completed.
    bool TermUnitSingDuct = false;                              // TRUE if a non-induction single duct terminal unit
    bool TermUnitPIU = false;                                   // TRUE if a powered induction terminal unit
    bool TermUnitIU = false;                                    // TRUE if an unpowered induction terminal unit
    bool ZoneEqFanCoil = false;                                 // TRUE if a 4 pipe fan coil unit is being simulated
    bool ZoneEqOutdoorAirUnit = false;                          // TRUE if an OutdoorAirUnit is being simulated
    bool ZoneEqUnitHeater = false;                              // TRUE if a unit heater is being simulated
    bool ZoneEqUnitVent = false;                                // TRUE if a unit ventilator unit is being simulated
    bool ZoneEqVentedSlab = false;                              // TRUE if a ventilated slab is being simulated
    bool ZoneEqDXCoil = false;                                  // TRUE if a ZoneHVAC DX coil is being simulated
    bool ZoneEqUnitarySys = false;                              // TRUE if a zone UnitarySystem is being simulated
    bool ZoneCoolingOnlyFan = false;                            // TRUE if a ZoneHVAC DX cooling coil is only coil in parent
    bool ZoneHeatingOnlyFan = false;                            // TRUE if zone unit only does heating and contains a fam (such as Unit Heater)
    bool ZoneSizingRunDone = false;                             // True if a zone sizing run has been successfully completed.
    bool DataErrorsFound = false;                               // used for simulation termination when errors are found
    bool DataDXCoolsLowSpeedsAutozize = false;                  // true allows reporting lower speed CoilCoolingCurveFits Autosize
    Real64 AutoVsHardSizingThreshold = 0.1;                     // criteria threshold used to determine if user hard size and autosize disagree 10%
    Real64 AutoVsHardSizingDeltaTempThreshold = 1.5;            // temperature criteria threshold for autosize versus hard size [C]
    Real64 DataCoilSizingAirInTemp = 0.0;                       // saves sizing data for use in coil object reporting
    Real64 DataCoilSizingAirInHumRat = 0.0;                     // saves sizing data for use in coil object reporting
    Real64 DataCoilSizingAirOutTemp = 0.0;                      // saves sizing data for use in coil object reporting
    Real64 DataCoilSizingAirOutHumRat = 0.0;                    // saves sizing data for use in coil object reporting
    Real64 DataCoilSizingFanCoolLoad = 0.0;                     // saves sizing data for use in coil object reporting
    Real64 DataCoilSizingCapFT = 1.0;                           // saves sizing data for use in coil object reporting
    bool DataDesAccountForFanHeat = true;                       // include fan heat when true
    Real64 DataDesInletWaterTemp = 0.0;                         // coil inlet water temperature used for warning messages
    Real64 DataDesInletAirHumRat = 0.0;                         // coil inlet air humidity ratio used for warning messages
    Real64 DataDesInletAirTemp = 0.0;                           // coil inlet air temperature used for warning messages
    Real64 DataDesOutletAirTemp = 0.0;                          // coil outlet air temperature used for sizing
    Real64 DataDesOutletAirHumRat = 0.0;                        // coil air outlet humidity ratio used in sizing calculations [kg water / kg dry air]
    Real64 DataCoolCoilCap = 0.0;                               // cooling coil capacity used for sizing with scalable inputs [W]
    Real64 DataFlowUsedForSizing = 0.0;                         // air flow rate used for sizing with scalable inputs [m3/s]
    Real64 DataAirFlowUsedForSizing = 0.0;                      // air flow rate used for sizing with scalable inputs [m3/s]
    Real64 DataWaterFlowUsedForSizing = 0.0;                    // water flow rate used for sizing with scalable inputs [m3/s]
    Real64 DataCapacityUsedForSizing = 0.0;                     // capacity used for sizing with scalable inputs [W]
    Real64 DataDesignCoilCapacity = 0.0;                        // calculated capacity of coil at end of UA calculation
    Real64 DataHeatSizeRatio = 1.0;                             // heating coil size as a ratio of cooling coil capacity
    Real64 DataEMSOverride = 0.0;                               // value of EMS variable used to override autosizing
    Real64 DataBypassFrac = 0.0;                                // value of bypass fraction for Coil:Cooling:DX:TwoStageWithHumidityControlMode coils
    Real64 DataFracOfAutosizedCoolingAirflow = 1.0;             // fraction of design cooling supply air flow rate
    Real64 DataFracOfAutosizedHeatingAirflow = 1.0;             // fraction of design heating supply air flow rate
    Real64 DataFlowPerCoolingCapacity = 0.0;                    // cooling supply air flow per unit cooling capacity
    Real64 DataFlowPerHeatingCapacity = 0.0;                    // heating supply air flow per unit heating capacity
    Real64 DataFracOfAutosizedCoolingCapacity = 1.0;            // fraction of autosized cooling capacity
    Real64 DataFracOfAutosizedHeatingCapacity = 1.0;            // fraction of autosized heating capacit
    Real64 DataAutosizedCoolingCapacity = 0.0;                  // Autosized cooling capacity used for multiplying flow per capacity to get flow rate
    Real64 DataAutosizedHeatingCapacity = 0.0;                  // Autosized heating capacit used for multiplying flow per capacity to get flow rate
    Real64 DataConstantUsedForSizing = 0.0;                     // base value used for sizing inputs that are ratios of other inputs
    Real64 DataFractionUsedForSizing = 0.0;               // fractional value of base value used for sizing inputs that are ratios of other inputs
    Real64 DataNonZoneNonAirloopValue = 0.0;              // used when equipment is not located in a zone or airloop
    Real64 DataSizingFraction = 1.0;                      // used when ratios of sizing is required
    int DataZoneUsedForSizing = 0;                        // pointer to control zone for air loop equipment
    int DataZoneNumber = 0;                               // a pointer to a served by zoneHVAC equipment
    int NumZoneHVACSizing = 0;                            // Number of design specification zone HVAC sizing objects
    int NumAirTerminalSizingSpec = 0;                     // Number of design specfication air terminal sizing objects
    int NumAirTerminalUnits = 0;                          // Number of air terminal units (same as total number of zone inlet nodes)
    Real64 DXCoolCap = 0.0;                               // The rated cooling capacity of a DX unit.
    Real64 GlobalHeatSizingFactor = 0.0;                  // the global heating sizing ratio
    Real64 GlobalCoolSizingFactor = 0.0;                  // the global cooling sizing ratio
    Real64 SuppHeatCap = 0.0;                             // the heating capacity of the supplemental heater in a unitary system
    Real64 UnitaryHeatCap = 0.0;                          // the heating capacity of a unitary system
    char SizingFileColSep = DataStringGlobals::CharComma; // Character to separate columns in sizing outputs
    int DataDesicDehumNum = 0;                            // index to desiccant dehumidifier
    bool DataDesicRegCoil = false;                        // TRUE if heating coil desiccant regeneration coil
    bool HRFlowSizingFlag = false;                        // True, if it is a heat recovery heat exchanger flow sizing
    Real64 DataWaterCoilSizCoolDeltaT = 0.0;              // used for sizing cooling coil water design flow rate
    Real64 DataWaterCoilSizHeatDeltaT = 0.0;              // used for sizing heating coil water design flow rate
    bool DataNomCapInpMeth = false;                       // True if heating coil is sized by CoilPerfInpMeth == NomCa
    HVAC::FanType DataFanType = HVAC::FanType::Invalid;   // Fan type used during sizing
    int DataFanIndex = -1;                                // Fan index used during sizing
    HVAC::FanPlace DataFanPlacement = HVAC::FanPlace::Invalid; // identifies location of fan wrt coil
    int DataDXSpeedNum = 0;
    int DataCoolCoilType = -1;
    int DataCoolCoilIndex = -1;
    EPVector<DataSizing::OARequirementsData> OARequirements;
    EPVector<DataSizing::ZoneAirDistributionData> ZoneAirDistribution;
    EPVector<DataSizing::ZoneSizingInputData> ZoneSizingInput;            // Input data for zone sizing
    Array2D<DataSizing::ZoneSizingData> ZoneSizing;                       // Data for zone sizing (all data, all design)
    EPVector<DataSizing::ZoneSizingData> FinalZoneSizing;                 // Final data for zone sizing including effects
    Array2D<DataSizing::ZoneSizingData> CalcZoneSizing;                   // Data for zone sizing (all data)
    EPVector<DataSizing::ZoneSizingData> CalcFinalZoneSizing;             // Final data for zone sizing (calculated only)
    Array2D<DataSizing::ZoneSizingData> SpaceSizing;                      // Data for space sizing (all data, all design)
    EPVector<DataSizing::ZoneSizingData> FinalSpaceSizing;                // Final data for space sizing including effects
    Array2D<DataSizing::ZoneSizingData> CalcSpaceSizing;                  // Data for space sizing (all data)
    EPVector<DataSizing::ZoneSizingData> CalcFinalSpaceSizing;            // Final data for space sizing (calculated only)
    EPVector<DataSizing::TermUnitZoneSizingData> TermUnitFinalZoneSizing; // Final data for sizing terminal units (indexed per terminal unit)
    EPVector<DataSizing::SystemSizingInputData> SysSizInput;              // Input data array for system sizing object
    Array2D<DataSizing::SystemSizingData> SysSizing;                      // Data array for system sizing (all data)
    EPVector<DataSizing::SystemSizingData> FinalSysSizing;                // Data array for system sizing (max heat/cool)
    EPVector<DataSizing::SystemSizingData> CalcSysSizing;                 // Data array for system sizing (max heat/cool)
    EPVector<DataSizing::SysSizPeakDDNumData> SysSizPeakDDNum;            // data array for peak des day indices
    EPVector<DataSizing::TermUnitSizingData> TermUnitSizing;              // Data added in sizing routines (indexed per terminal unit)
    EPVector<DataSizing::ZoneEqSizingData> ZoneEqSizing;                  // Data added in zone eq component sizing routines
    EPVector<DataSizing::ZoneEqSizingData> UnitarySysEqSizing;            // Data added in unitary system sizing routines
    EPVector<DataSizing::ZoneEqSizingData> OASysEqSizing;                 // Data added in unitary system sizing routines
    EPVector<DataSizing::PlantSizingData> PlantSizData;                   // Input data array for plant sizing
    EPVector<DataSizing::DesDayWeathData> DesDayWeath;                    // design day weather saved at major time step
    EPVector<DataSizing::CompDesWaterFlowData> CompDesWaterFlow;          // array to store components' design water flow
    EPVector<DataSizing::ZoneHVACSizingData> ZoneHVACSizing;              // Input data for zone HVAC sizing
    EPVector<DataSizing::AirTerminalSizingSpecData>
        AirTerminalSizingSpec;                                   // Input data for zone HVAC sizing used only for Facility Load Component Summary
    EPVector<DataSizing::FacilitySizingData> CalcFacilitySizing; // Data for zone sizing
    DataSizing::FacilitySizingData CalcFinalFacilitySizing;      // Final data for zone sizing
    Array1D<Real64> VbzByZone;                                   // saved value of ZoneOAUnc which is Vbz used in 62.1 tabular report
    Array1D<Real64> VdzClgByZone;    // saved value of cooling based ZoneSA which is Vdz used in 62.1 tabular report (also used for zone level Vps)
    Array1D<Real64> VdzMinClgByZone; // minimum discharge flow for cooling, Vdz includes secondary and primary flows for dual path
    Array1D<Real64> VdzHtgByZone;    // saved value of heating based ZoneSA which is Vdz used in 62.1 tabular report (also used for zone level Vps)
    Array1D<Real64> VdzMinHtgByZone; // minimum discharge flow for heating, Vdz includes secondary and primary flows for dual path
    Array1D<Real64> ZdzClgByZone;    // minimum discharge outdoor-air fraction for cooling
    Array1D<Real64> ZdzHtgByZone;    // minimum discharge outdoor-air fraction for heating
    Array1D<Real64> VpzClgByZone;    // saved value of cooling based ZonePA which is Vpz used in 62.1 tabular report
    Array1D<Real64> VpzMinClgByZone; // saved value of minimum cooling based ZonePA which is VpzClg-min used in 62.1 tabular report
    Array1D<Real64> VpzHtgByZone;    // saved value of heating based ZonePA which is Vpz used in 62.1 tabular report
    Array1D<Real64> VpzMinHtgByZone; // saved value of minimum heating based ZonePA which is VpzHtg-min used in 62.1 tabular report
    Array1D<Real64> VpzClgSumBySys;  // sum of saved value of cooling based ZonePA which is Vpz-sum used in 62.1 tabular report
    Array1D<Real64> VpzHtgSumBySys;  // sum of saved value of heating based ZonePA which is Vpz-sum used in 62.1 tabular report
    Array1D<Real64> PzSumBySys;      // sum of design people for system, Pz_sum
    Array1D<Real64> PsBySys;         // sum of peak concurrent people by system, Ps
    Array1D<Real64> DBySys;          // Population Diversity by system
    Array1D<Real64> SumRpxPzBySys;   // Sum of per person OA times number of people by system, No D yet
    Array1D<Real64> SumRaxAzBySys;   // sum of per area OA time zone area by system, does not get altered by D
    Array1D<std::string> PeakPsOccurrenceDateTimeStringBySys;    // string describing when Ps peak occurs
    Array1D<std::string> PeakPsOccurrenceEnvironmentStringBySys; // string describing Environment when Ps peak occurs
    Array1D<Real64> VouBySys;                                    // uncorrected system outdoor air requirement, for std 62.1 VRP
    Array1D<Real64> VpsClgBySys;                                 // System primary airflow Vps, for cooling for std 62.1 VRP
    Array1D<Real64> VpsHtgBySys;                                 // system primary airflow Vps, for heating for std 62.1 VRP
    Array1D<Real64> FaByZoneHeat;                                // saved value of Fa used in 62.1 tabular report
    Array1D<Real64> FbByZoneCool;                                // saved value of Fb used in 62.1 tabular report
    Array1D<Real64> FbByZoneHeat;                                // saved value of Fb used in 62.1 tabular report
    Array1D<Real64> FcByZoneCool;                                // saved value of Fc used in 62.1 tabular report
    Array1D<Real64> FcByZoneHeat;                                // saved value of Fc used in 62.1 tabular report
    Array1D<Real64> XsBySysCool;                                 // saved value of Xs used in 62.1 tabular report
    Array1D<Real64> XsBySysHeat;                                 // saved value of Xs used in 62.1 tabular report
    Array1D<Real64> EvzByZoneCool;                               // saved value of Evz (zone vent effy) used in 62.1 tabular report
    Array1D<Real64> EvzByZoneHeat;                               // saved value of Evz (zone vent effy) used in 62.1 tabular report
    Array1D<Real64> EvzByZoneCoolPrev;                           // saved value of Evz (zone vent effy) used in 62.1 tabular report
    Array1D<Real64> EvzByZoneHeatPrev;                           // saved value of Evz (zone vent effy) used in 62.1 tabular report
    Array1D<Real64> VotClgBySys;     // saved value of cooling ventilation required at primary AHU, used in 62.1 tabular report
    Array1D<Real64> VotHtgBySys;     // saved value of heating ventilation required at primary AHU, used in 62.1 tabular report
    Array1D<Real64> VozSumClgBySys;  // saved value of cooling ventilation required at clg zones
    Array1D<Real64> VozSumHtgBySys;  // saved value of cooling ventilation required at htg zones
    Array1D<Real64> TotCoolCapTemp;  // scratch variable used for calculating peak load [W]
    Array1D<Real64> EvzMinBySysHeat; // saved value of EvzMin used in 62.1 tabular report
    Array1D<Real64> EvzMinBySysCool; // saved value of EvzMin used in 62.1 tabular report
    Array1D<Real64> FaByZoneCool;    // triggers allocation in UpdateSysSizing
    Array1D<Real64> SensCoolCapTemp; // triggers allocation in UpdateSysSizing

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) SizingData();
    }
};

} // namespace EnergyPlus

#endif
