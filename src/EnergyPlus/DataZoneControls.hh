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

#ifndef DataZoneControls_hh_INCLUDED
#define DataZoneControls_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace DataZoneControls {

    // Average method parameter with multiple people objects in a zone
    enum class AverageMethod
    {
        Invalid = -1,
        NO,  // No multiple people objects
        SPE, // Specific people object
        OBJ, // People object average
        PEO, // People number average
        Num
    };

    constexpr std::array<std::string_view, (int)AverageMethod::Num> averageMethodNames = {
        "NoMultiplePeopleObjects", "SpecificObject", "ObjectAverage", "PeopleAverage"};
    constexpr std::array<std::string_view, (int)AverageMethod::Num> averageMethodNamesUC = {
        "NOMULTIPLEPEOPLEOBJECTS", "SPECIFICOBJECT", "OBJECTAVERAGE", "PEOPLEAVERAGE"};

    enum class TempCtrl
    {
        Invalid = -1,
        None,
        Constant,
        Scheduled,
        Num
    };

    constexpr std::array<std::string_view, (int)TempCtrl::Num> tempCtrlNames = {"None", "Constant", "Scheduled"};
    constexpr std::array<std::string_view, (int)TempCtrl::Num> tempCtrlNamesUC = {"NONE", "CONSTANT", "SCHEDULED"};

    struct TempSetptType
    {
        std::string Name;
        bool isUsed = false;
        Sched::Schedule *heatSetptSched = nullptr;
        Sched::Schedule *coolSetptSched = nullptr;
    };

    struct ZoneTempControls
    {
        // Members
        std::string Name;     // Name of the thermostat
        std::string ZoneName; // Name of the zone
        int ActualZoneNum;

        Sched::Schedule *setptTypeSched = nullptr; // Index for this schedule

        std::array<TempSetptType, (int)HVAC::SetptType::Num> setpts;

        bool ManageDemand;                       // Flag to indicate whether to use demand limiting
        Real64 HeatingResetLimit;                // Lowest heating setpoint that can be set by demand manager [C]
        Real64 CoolingResetLimit;                // Highest cooling setpoint that can be set by demand manager [C]
        bool EMSOverrideHeatingSetPointOn;       // EMS is calling to override heating setpoint
        Real64 EMSOverrideHeatingSetPointValue;  // value EMS is directing to use for heating setpoint [C]
        bool EMSOverrideCoolingSetPointOn;       // EMS is calling to override cooling setpoint
        Real64 EMSOverrideCoolingSetPointValue;  // value EMS is directing to use for cooling setpoint [C]
        TempCtrl OpTempCtrl = TempCtrl::Invalid; // flag to indicate if radiative fraction is scheduled,
        // else constant
        Real64 FixedRadiativeFraction;                           // weighting factor for mean radiant temp for Operative temperature
        Sched::Schedule *opTempRadiativeFractionSched = nullptr; // schedule for when fraction is scheduled

        bool AdaptiveComfortTempControl;   // flag to indicate whether control based on Operative Temp
        int AdaptiveComfortModelTypeIndex; // index to adaptive comfort model type

        Real64 ZoneOvercoolRange;                  // Zone overcool temperature range (max), deg C
        TempCtrl OvercoolCtrl = TempCtrl::Invalid; // Flag to indicate if zone overcool range is scheduled
        //   or constant
        Real64 ZoneOvercoolConstRange;                     // Overcool Range for Zone Air Setpoint Temperature [deltaC]
        Sched::Schedule *zoneOvercoolRangeSched = nullptr; // Overcool Range Schedule
        Real64 ZoneOvercoolControlRatio;                   // Zone relative humidity shift per dry-bulb temperature overcooling
        //      below the original cooling setpoint, %RH/deltaC
        Sched::Schedule *dehumidifyingSched = nullptr; // dehumidifying schedule
        Real64 DeltaTCutSet;                           // Temperature difference between cutout and setpoint
        Real64 ZoneThermostatSetPointHi;               // Cooling setpoint
        Real64 ZoneThermostatSetPointLo;               // Heating setpoint
        bool CoolModeLast;
        bool HeatModeLast;
        bool CoolModeLastSave;
        bool HeatModeLastSave;
        bool CoolOffFlag; // Cooling off flag
        bool HeatOffFlag; // Heating off flag

        // Default Constructor
        ZoneTempControls()
            : ActualZoneNum(0), ManageDemand(false), HeatingResetLimit(0.0), CoolingResetLimit(0.0), EMSOverrideHeatingSetPointOn(false),
              EMSOverrideHeatingSetPointValue(0.0), EMSOverrideCoolingSetPointOn(false), EMSOverrideCoolingSetPointValue(0.0),
              FixedRadiativeFraction(0.0), AdaptiveComfortTempControl(false), AdaptiveComfortModelTypeIndex(0), ZoneOvercoolRange(0.0),
              ZoneOvercoolConstRange(0.0), ZoneOvercoolControlRatio(0.0), DeltaTCutSet(0), ZoneThermostatSetPointHi(0.0),
              ZoneThermostatSetPointLo(0.0), CoolModeLast(false), HeatModeLast(false), CoolModeLastSave(false), HeatModeLastSave(false),
              CoolOffFlag(false), HeatOffFlag(false)

        {
        }
    };

    struct ZoneHumidityControls
    {
        // Members
        std::string ControlName; // Name of this humidity controller
        std::string ZoneName;    // Name of the zone
        int ActualZoneNum;
        Sched::Schedule *humidifyingSched = nullptr;   // humidifying schedule
        Sched::Schedule *dehumidifyingSched = nullptr; // dehumidifying schedule
        int ErrorIndex;                                // Error index when LowRH setpoint > HighRH setpoint
        bool EMSOverrideHumidifySetPointOn;            // EMS is calling to override humidifying setpoint
        Real64 EMSOverrideHumidifySetPointValue;       // value EMS is directing to use for humidifying setpoint
        bool EMSOverrideDehumidifySetPointOn;          // EMS is calling to override dehumidifying setpoint
        Real64 EMSOverrideDehumidifySetPointValue;     // value EMS is directing to use for dehumidifying setpoint

        // Default Constructor
        ZoneHumidityControls()
            : ActualZoneNum(0), ErrorIndex(0), EMSOverrideHumidifySetPointOn(false), EMSOverrideHumidifySetPointValue(0.0),
              EMSOverrideDehumidifySetPointOn(false), EMSOverrideDehumidifySetPointValue(0.0)
        {
        }
    };

    struct ComfortSetptType
    {
        std::string Name;
        bool isUsed = false;
        Sched::Schedule *heatSetptSched = nullptr;
        Sched::Schedule *coolSetptSched = nullptr;
    };

    struct ZoneComfortControls
    {
        // Members
        std::string Name;                          // Name of the thermostat
        std::string ZoneName;                      // Name of the zone
        int ActualZoneNum;                         // Index number of zone
        Sched::Schedule *setptTypeSched = nullptr; // Schedule determines which thermostat type is active

        std::array<ComfortSetptType, (int)HVAC::SetptType::Num> setpts; // Type of control

        bool ManageDemand;                                    // Flag to indicate whether to use demand limiting
        Real64 HeatingResetLimit;                             // Lowest heating setpoint that can be set by demand manager [C]
        Real64 CoolingResetLimit;                             // Highest cooling setpoint that can be set by demand manager [C]
        bool EMSOverrideHeatingSetPointOn;                    // EMS is calling to override heating setpoint
        Real64 EMSOverrideHeatingSetPointValue;               // value EMS is directing to use for heating setpoint
        bool EMSOverrideCoolingSetPointOn;                    // EMS is calling to override cooling setpoint
        Real64 EMSOverrideCoolingSetPointValue;               // value EMS is directing to use for cooling setpoint
        Real64 TdbMaxSetPoint;                                // Maximum dry-bulb temperature setpoint [C]
        Real64 TdbMinSetPoint;                                // Minimum dry-bulb temperature setpoint [C]
        std::string AverageObjectName;                        // Object Name for Specific Object Average
        AverageMethod averageMethod = AverageMethod::Invalid; // Averaging method
        int SpecificObjectNum;                                // People Object number used for Specific people object choice
        int PeopleAverageErrIndex;                            // People average error index
        int TdbMaxErrIndex;                                   // Single cooling setpoint error index
        int TdbMinErrIndex;                                   // Single heating setpoint error index
        int TdbHCErrIndex;                                    // Single heating cooling setpoint error index
        int TdbDualMaxErrIndex;                               // Dual cooling setpoint error index
        int TdbDualMinErrIndex;                               // Dual heating setpoint error index

        // Default Constructor
        ZoneComfortControls()
            : ActualZoneNum(0), ManageDemand(false), HeatingResetLimit(0.0), CoolingResetLimit(0.0), EMSOverrideHeatingSetPointOn(false),
              EMSOverrideHeatingSetPointValue(0.0), EMSOverrideCoolingSetPointOn(false), EMSOverrideCoolingSetPointValue(0.0), TdbMaxSetPoint(50.0),
              TdbMinSetPoint(0.0), SpecificObjectNum(0), PeopleAverageErrIndex(0), TdbMaxErrIndex(0), TdbMinErrIndex(0), TdbHCErrIndex(0),
              TdbDualMaxErrIndex(0), TdbDualMinErrIndex(0)
        {
        }
    };

    struct ZoneStagedControls
    {
        // Members
        std::string Name;                              // Name of the thermostat
        std::string ZoneName;                          // Name of the zone
        int ActualZoneNum;                             // Index number of zone
        Sched::Schedule *heatSetptBaseSched = nullptr; // schedule which provides zone heating setpoint base
        Sched::Schedule *coolSetptBaseSched = nullptr; // schedule which provides zone cooling setpoint base
        int NumOfHeatStages;                           // Number of heating stages
        int NumOfCoolStages;                           // Number of cooling stages
        Real64 HeatThroRange;                          // Heating throttling tempeature range
        Real64 CoolThroRange;                          // Cooling throttling tempeature range
        Array1D<Real64> HeatTOffset;                   // Heating temperature offset
        Array1D<Real64> CoolTOffset;                   // Cooling temperature offset
        Real64 HeatSetPoint;                           // Heating throttling tempeature range
        Real64 CoolSetPoint;                           // Cooling throttling tempeature range
        int StageErrCount;                             // Staged setpoint error count
        int StageErrIndex;                             // Staged setpoint error index

        // Default Constructor
        ZoneStagedControls()
            : ActualZoneNum(0), NumOfHeatStages(0), NumOfCoolStages(0), HeatThroRange(0.0), CoolThroRange(0.0), HeatSetPoint(0.0), CoolSetPoint(0.0),
              StageErrCount(0), StageErrIndex(0)
        {
        }
    };

    struct TStatObject
    {
        // Members
        std::string Name;
        int ZoneOrZoneListPtr;
        int NumOfZones;
        int TempControlledZoneStartPtr;
        int ComfortControlledZoneStartPtr;
        int StageControlledZoneStartPtr;
        bool ZoneListActive;

        // Default Constructor
        TStatObject()
            : ZoneOrZoneListPtr(0), NumOfZones(0), TempControlledZoneStartPtr(0), ComfortControlledZoneStartPtr(0), StageControlledZoneStartPtr(0),
              ZoneListActive(false)
        {
        }
    };

} // namespace DataZoneControls

struct DataZoneControlsData : BaseGlobalStruct
{
    int NumTempControlledZones = 0;
    int NumHumidityControlZones = 0;
    int NumComfortControlledZones = 0;
    int NumTStatStatements = 0;
    int NumComfortTStatStatements = 0;
    int NumOpTempControlledZones = 0;           // number of zones with operative temp control
    int NumTempAndHumidityControlledZones = 0;  // number of zones with over cool control
    bool AnyOpTempControl = false;              // flag set true if any zones have op temp control
    bool AnyZoneTempAndHumidityControl = false; // flag set true if any zones have over cool control
    Array1D_bool StageZoneLogic;                // Logical array, A zone with staged thermostat = .TRUE.
    Array1D<Real64> OccRoomTSetPointHeat;       // occupied heating set point for optimum start period
    Array1D<Real64> OccRoomTSetPointCool;       // occupied cooling set point for optimum start period
    bool GetZoneAirStatsInputFlag = true;       // True when need to get input
    Array1D<DataZoneControls::ZoneHumidityControls> HumidityControlZone;
    Array1D<DataZoneControls::ZoneTempControls> TempControlledZone;
    Array1D<DataZoneControls::ZoneComfortControls> ComfortControlledZone;
    Array1D<DataZoneControls::TStatObject> TStatObjects;
    Array1D<DataZoneControls::TStatObject> ComfortTStatObjects;
    Array1D<DataZoneControls::TStatObject> StagedTStatObjects;
    Array1D<DataZoneControls::ZoneStagedControls> StageControlledZone;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumTempControlledZones = 0;
        this->NumHumidityControlZones = 0;
        this->NumComfortControlledZones = 0;
        this->NumTStatStatements = 0;
        this->NumComfortTStatStatements = 0;
        this->NumOpTempControlledZones = 0;
        this->NumTempAndHumidityControlledZones = 0;
        this->AnyOpTempControl = false;
        this->AnyZoneTempAndHumidityControl = false;
        this->StageZoneLogic.deallocate();
        this->OccRoomTSetPointHeat.deallocate();
        this->OccRoomTSetPointCool.deallocate();
        this->GetZoneAirStatsInputFlag = true;
        this->HumidityControlZone.deallocate();
        this->TempControlledZone.deallocate();
        this->ComfortControlledZone.deallocate();
        this->TStatObjects.deallocate();
        this->ComfortTStatObjects.deallocate();
        this->StagedTStatObjects.deallocate();
        this->StageControlledZone.deallocate();
    }
};

} // namespace EnergyPlus

#endif
