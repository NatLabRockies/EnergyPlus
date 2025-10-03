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

#include <exception>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DisplacementVentMgr.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/ExteriorEnergyUse.hh>
#include <EnergyPlus/HVACManager.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, InternalHeatGains_OtherEquipment_CheckFuelType)
{

    std::string const idf_objects = delimited_string({
        "Zone,Zone1;",

        "ScheduleTypeLimits,SchType1,0.0,1.0,Continuous,Dimensionless;",

        "Schedule:Constant,Schedule1,,1.0;",

        "OtherEquipment,",
        "  OtherEq1,",
        "  ,",
        "  Zone1,",
        "  Schedule1,",
        "  EquipmentLevel,",
        "  100.0,,,",
        "  0.1,",
        "  0.2,",
        "  0.05;",

        "OtherEquipment,",
        "  OtherEq2,",
        "  Propane,",
        "  Zone1,",
        "  Schedule1,",
        "  EquipmentLevel,",
        "  100.0,,,",
        "  0.1,",
        "  0.2,",
        "  0.05;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_FALSE(has_err_output());

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    bool ErrorsFound(false);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    InternalHeatGains::GetInternalHeatGainsInput(*state);

    ASSERT_EQ(state->dataHeatBal->ZoneOtherEq.size(), 2u);

    for (unsigned long i = 1; i <= state->dataHeatBal->ZoneOtherEq.size(); ++i) {
        const DataHeatBalance::ZoneEquipData &equip = state->dataHeatBal->ZoneOtherEq(i);
        if (equip.Name == "OTHEREQ1") {
            ASSERT_ENUM_EQ(equip.OtherEquipFuelType, Constant::eFuel::None);
        } else if (equip.Name == "OTHEREQ2") {
            ASSERT_ENUM_EQ(equip.OtherEquipFuelType, Constant::eFuel::Propane);
        }
    }
}

TEST_F(EnergyPlusFixture, InternalHeatGains_OtherEquipment_NegativeDesignLevel)
{

    std::string const idf_objects = delimited_string({
        "Zone,Zone1;",

        "ScheduleTypeLimits,SchType1,0.0,1.0,Continuous,Dimensionless;",

        "Schedule:Constant,Schedule1,,1.0;",

        "OtherEquipment,",
        "  OtherEq1,",
        "  FuelOilNo1,",
        "  Zone1,",
        "  Schedule1,",
        "  EquipmentLevel,",
        "  -100.0,,,",
        "  0.1,",
        "  0.2,",
        "  0.05;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_FALSE(has_err_output());

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    bool ErrorsFound(false);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    ASSERT_THROW(InternalHeatGains::GetInternalHeatGainsInput(*state), std::runtime_error);

    std::string const error_string = delimited_string(
        {"   ** Warning ** ProcessScheduleInput: Schedule:Constant = SCHEDULE1",
         "   **   ~~~   ** Schedule Type Limits Name is empty.",
         "   **   ~~~   ** Schedule will not be validated.",
         "   ** Severe  ** GetInternalHeatGains: OtherEquipment=\"OTHEREQ1\", Design Level is not allowed to be negative",
         "   **   ~~~   ** ... when a fuel type of FuelOilNo1 is specified.",
         "   **  Fatal  ** GetInternalHeatGains: Errors found in Getting Internal Gains Input, Program Stopped",
         "   ...Summary of Errors that led to program termination:",
         "   ..... Reference severe error count=1",
         "   ..... Last severe error=GetInternalHeatGains: OtherEquipment=\"OTHEREQ1\", Design Level is not allowed to be negative"});

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, InternalHeatGains_OtherEquipment_BadFuelType)
{

    std::string const idf_objects = delimited_string({
        "Zone,Zone1;",

        "ScheduleTypeLimits,SchType1,0.0,1.0,Continuous,Dimensionless;",

        "Schedule:Constant,Schedule1,,1.0;",

        "OtherEquipment,",
        "  OtherEq1,",
        "  Water,",
        "  Zone1,",
        "  Schedule1,",
        "  EquipmentLevel,",
        "  100.0,,,",
        "  0.1,",
        "  0.2,",
        "  0.05;",

    });

    ASSERT_FALSE(process_idf(idf_objects, false)); // add false to supress error assertions
    EXPECT_TRUE(has_err_output(false));

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    std::string error_string =
        delimited_string({"   ** Severe  ** <root>[OtherEquipment][OtherEq1][fuel_type] - \"Water\" - Failed to match against any enum values.",
                          "   ** Warning ** ProcessScheduleInput: Schedule:Constant = SCHEDULE1",
                          "   **   ~~~   ** Schedule Type Limits Name is empty.",
                          "   **   ~~~   ** Schedule will not be validated."});
    EXPECT_TRUE(compare_err_stream(error_string, true));

    bool ErrorsFound(false);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    ASSERT_THROW(InternalHeatGains::GetInternalHeatGainsInput(*state), std::runtime_error);

    error_string =
        delimited_string({"   ** Severe  ** GetInternalHeatGains: OtherEquipment: invalid Fuel Type entered=WATER for Name=OTHEREQ1",
                          "   **  Fatal  ** GetInternalHeatGains: Errors found in Getting Internal Gains Input, Program Stopped",
                          "   ...Summary of Errors that led to program termination:",
                          "   ..... Reference severe error count=2",
                          "   ..... Last severe error=GetInternalHeatGains: OtherEquipment: invalid Fuel Type entered=WATER for Name=OTHEREQ1"});

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, InternalHeatGains_AllowBlankFieldsForAdaptiveComfortModel)
{
    // Adaptive comfort model fatal for irrelevant blank fields  #5948

    std::string const idf_objects = delimited_string({
        "ScheduleTypeLimits,SchType1,0.0,1.0,Continuous,Dimensionless;",

        "  Schedule:Compact,",
        "    HOUSE OCCUPANCY,    !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  Schedule:Compact,",
        "    Activity Sch,    !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "Zone,LIVING ZONE;",

        "People,",
        "LIVING ZONE People, !- Name",
        "LIVING ZONE, !- Zone or ZoneList Name",
        "HOUSE OCCUPANCY, !- Number of People Schedule Name",
        "people, !- Number of People Calculation Method",
        "3.000000, !- Number of People",
        ", !- People per Zone Floor Area{ person / m2 }",
        ", !- Zone Floor Area per Person{ m2 / person }",
        "0.3000000, !- Fraction Radiant",
        ", !- Sensible Heat Fraction",
        "Activity Sch, !- Activity Level Schedule Name",
        "3.82E-8, !- Carbon Dioxide Generation Rate{ m3 / s - W }",
        ", !- Enable ASHRAE 55 Comfort Warnings",
        "EnclosureAveraged, !- Mean Radiant Temperature Calculation Type",
        ", !- Surface Name / Angle Factor List Name",
        ", !- Work Efficiency Schedule Name",
        ", !- Clothing Insulation Calculation Method",
        ", !- Clothing Insulation Calculation Method Schedule Name",
        ", !- Clothing Insulation Schedule Name",
        ", !- Air Velocity Schedule Name",
        "AdaptiveASH55;                  !- Thermal Comfort Model 1 Type",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    bool ErrorsFound1(false);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound1);
    ASSERT_FALSE(ErrorsFound1);

    auto *occSched = Sched::GetSchedule(*state, "HOUSE OCCUPANCY");
    occSched->isUsed = true;
    occSched->currentVal = 1.0;
    occSched->minVal = 1.0;
    occSched->maxVal = 1.0;
    occSched->isMinMaxSet = true;

    auto *actSched = Sched::GetSchedule(*state, "ACTIVITY SCH");
    actSched->isUsed = true;
    actSched->currentVal = 131.8;
    actSched->minVal = 131.8;
    actSched->maxVal = 131.8;
    actSched->isMinMaxSet = true;

    InternalHeatGains::GetInternalHeatGainsInput(*state);

    EXPECT_FALSE(state->dataInternalHeatGains->ErrorsFound);
}

TEST_F(EnergyPlusFixture, InternalHeatGains_ElectricEquipITE_BeginEnvironmentReset)
{
    using namespace DataHeatBalance;

    std::string const idf_objects = delimited_string({
        "Zone,Main Zone;",

        "ZoneHVAC:EquipmentConnections,",
        "  Main Zone,                   !- Zone Name",
        "  Main Zone Equipment,         !- Zone Conditioning Equipment List Name",
        "  Main Zone Inlet Node,        !- Zone Air Inlet Node or NodeList Name",
        "  ,                            !- Zone Air Exhaust Node or NodeList Name",
        "  Main Zone Node,              !- Zone Air Node Name",
        "  Main Zone Outlet Node;       !- Zone Return Air Node or NodeList Name",

        "ZoneHVAC:EquipmentList,",
        "  Main Zone Equipment,     !- Name",
        "  SequentialLoad,          !- Load Distribution Scheme",
        "  ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "  Main Zone ATU,           !- Zone Equipment 1 Name",
        "  1,                       !- Zone Equipment 1 Cooling Sequence",
        "  2,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "  ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "  ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "  ZoneHVAC:Baseboard:Convective:Electric,  !- Zone Equipment 2 Object Type",
        "  Main Zone Baseboard,     !- Zone Equipment 2 Name",
        "  2,                       !- Zone Equipment 2 Cooling Sequence",
        "  1,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "  ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
        "  ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

        "ZoneHVAC:AirDistributionUnit,",
        "  Main Zone ATU,               !- Name",
        "  Main Zone Inlet Node,        !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "  Main Zone VAV Air;           !- Air Terminal Name",

        "AirTerminal:SingleDuct:VAV:NoReheat,",
        "  Main Zone VAV Air,           !- Name",
        "  System Availability Schedule,  !- Availability Schedule Name",
        "  Main Zone Inlet Node,    !- Air Outlet Node Name",
        "  Main Zone ATU In Node,   !- Air Inlet Node Name",
        "  8.5,                     !- Maximum Air Flow Rate {m3/s}",
        "  Constant,                !- Zone Minimum Air Flow Input Method",
        "  0.05;                    !- Constant Minimum Air Flow Fraction",

        "ZoneHVAC:Baseboard:Convective:Electric,",
        "  Main Zone Baseboard,     !- Name",
        "  System Availability Schedule,  !- Availability Schedule Name",
        "  HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "  8000,                    !- Heating Design Capacity {W}",
        "  ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "  ,                        !- Fraction of Autosized Heating Design Capacity",
        "  0.97;                    !- Efficiency",

        "ElectricEquipment:ITE:AirCooled,",
        "  Data Center Servers,     !- Name",
        "  Main Zone,               !- Zone Name",
        "  ,",
        "  Watts/Unit,              !- Design Power Input Calculation Method",
        "  500,                     !- Watts per Unit {W}",
        "  100,                     !- Number of Units",
        "  ,                        !- Watts per Zone Floor Area {W/m2}",
        "  ,  !- Design Power Input Schedule Name",
        "  ,  !- CPU Loading  Schedule Name",
        "  Data Center Servers Power fLoadTemp,  !- CPU Power Input Function of Loading and Air Temperature Curve Name",
        "  0.4,                     !- Design Fan Power Input Fraction",
        "  0.0001,                  !- Design Fan Air Flow Rate per Power Input {m3/s-W}",
        "  Data Center Servers Airflow fLoadTemp,  !- Air Flow Function of Loading and Air Temperature Curve Name",
        "  ECM FanPower fFlow,      !- Fan Power Input Function of Flow Curve Name",
        "  15,                      !- Design Entering Air Temperature {C}",
        "  A3,                      !- Environmental Class",
        "  AdjustedSupply,          !- Air Inlet Connection Type",
        "  ,                        !- Air Inlet Room Air Model Node Name",
        "  ,                        !- Air Outlet Room Air Model Node Name",
        "  Main Zone Inlet Node,    !- Supply Air Node Name",
        "  0.1,                     !- Design Recirculation Fraction",
        "  Data Center Recirculation fLoadTemp,  !- Recirculation Function of Loading and Supply Temperature Curve Name",
        "  0.9,                     !- Design Electric Power Supply Efficiency",
        "  UPS Efficiency fPLR,     !- Electric Power Supply Efficiency Function of Part Load Ratio Curve Name",
        "  1,                       !- Fraction of Electric Power Supply Losses to Zone",
        "  ITE-CPU,                 !- CPU End-Use Subcategory",
        "  ITE-Fans,                !- Fan End-Use Subcategory",
        "  ITE-UPS;                 !- Electric Power Supply End-Use Subcategory",

        "Curve:Quadratic,",
        "  ECM FanPower fFlow,      !- Name",
        "  0.0,                     !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Minimum Value of x",
        "  99.0;                    !- Maximum Value of x",

        "Curve:Quadratic,",
        "  UPS Efficiency fPLR,     !- Name",
        "  1.0,                     !- Coefficient1 Constant",
        "  0.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Minimum Value of x",
        "  99.0;                    !- Maximum Value of x",

        "Curve:Biquadratic,",
        "  Data Center Servers Power fLoadTemp,  !- Name",
        "  -1.0,                    !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.06667,                 !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "  Data Center Servers Airflow fLoadTemp,  !- Name",
        "  -1.4,                    !- Coefficient1 Constant",
        "  0.9,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.1,                     !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "  Data Center Recirculation fLoadTemp,  !- Name",
        "  1.0,                     !- Coefficient1 Constant",
        "  0.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_FALSE(has_err_output());

    bool ErrorsFound(false);
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.008;

    InternalHeatGains::GetInternalHeatGainsInput(*state);
    InternalHeatGains::CalcZoneITEq(*state);
    auto &thisZoneITEq = state->dataHeatBal->ZoneITEq(1);
    Real64 InitialPower =
        thisZoneITEq.PowerRpt[(int)PERptVars::CPU] + thisZoneITEq.PowerRpt[(int)PERptVars::Fan] + thisZoneITEq.PowerRpt[(int)PERptVars::UPS];

    state->dataLoopNodes->Node(1).Temp = 45.0;
    InternalHeatGains::CalcZoneITEq(*state);
    Real64 NewPower =
        thisZoneITEq.PowerRpt[(int)PERptVars::CPU] + thisZoneITEq.PowerRpt[(int)PERptVars::Fan] + thisZoneITEq.PowerRpt[(int)PERptVars::UPS];
    ASSERT_NE(InitialPower, NewPower);
    HVACManager::ResetNodeData(*state);

    InternalHeatGains::CalcZoneITEq(*state);
    NewPower = thisZoneITEq.PowerRpt[(int)PERptVars::CPU] + thisZoneITEq.PowerRpt[(int)PERptVars::Fan] + thisZoneITEq.PowerRpt[(int)PERptVars::UPS];
    ASSERT_EQ(InitialPower, NewPower);
}

TEST_F(EnergyPlusFixture, InternalHeatGains_CheckZoneComponentLoadSubtotals)
{

    std::string const idf_objects = delimited_string({
        "Zone,Main Zone;",

        "ZoneHVAC:EquipmentConnections,",
        "  Main Zone,                   !- Zone Name",
        "  Main Zone Equipment,         !- Zone Conditioning Equipment List Name",
        "  Main Zone Inlet Node,        !- Zone Air Inlet Node or NodeList Name",
        "  ,                            !- Zone Air Exhaust Node or NodeList Name",
        "  Main Zone Node,              !- Zone Air Node Name",
        "  Main Zone Outlet Node;       !- Zone Return Air Node or NodeList Name",

        "ZoneHVAC:EquipmentList,",
        "  Main Zone Equipment,     !- Name",
        "  SequentialLoad,          !- Load Distribution Scheme",
        "  ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "  Main Zone ATU,           !- Zone Equipment 1 Name",
        "  1,                       !- Zone Equipment 1 Cooling Sequence",
        "  2,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "  ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "  ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "  ZoneHVAC:Baseboard:Convective:Electric,  !- Zone Equipment 2 Object Type",
        "  Main Zone Baseboard,     !- Zone Equipment 2 Name",
        "  2,                       !- Zone Equipment 2 Cooling Sequence",
        "  1,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "  ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
        "  ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

        "ZoneHVAC:AirDistributionUnit,",
        "  Main Zone ATU,               !- Name",
        "  Main Zone Inlet Node,        !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "  Main Zone VAV Air;           !- Air Terminal Name",

        "AirTerminal:SingleDuct:VAV:NoReheat,",
        "  Main Zone VAV Air,           !- Name",
        "  System Availability Schedule,  !- Availability Schedule Name",
        "  Main Zone Inlet Node,    !- Air Outlet Node Name",
        "  Main Zone ATU In Node,   !- Air Inlet Node Name",
        "  8.5,                     !- Maximum Air Flow Rate {m3/s}",
        "  Constant,                !- Zone Minimum Air Flow Input Method",
        "  0.05;                    !- Constant Minimum Air Flow Fraction",

        "ZoneHVAC:Baseboard:Convective:Electric,",
        "  Main Zone Baseboard,     !- Name",
        "  System Availability Schedule,  !- Availability Schedule Name",
        "  HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "  8000,                    !- Heating Design Capacity {W}",
        "  ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "  ,                        !- Fraction of Autosized Heating Design Capacity",
        "  0.97;                    !- Efficiency",

        "ElectricEquipment:ITE:AirCooled,",
        "  Data Center Servers,     !- Name",
        "  Main Zone,               !- Zone Name",
        "  ,",
        "  Watts/Unit,              !- Design Power Input Calculation Method",
        "  500,                     !- Watts per Unit {W}",
        "  100,                     !- Number of Units",
        "  ,                        !- Watts per Zone Floor Area {W/m2}",
        "  ,  !- Design Power Input Schedule Name",
        "  ,  !- CPU Loading  Schedule Name",
        "  Data Center Servers Power fLoadTemp,  !- CPU Power Input Function of Loading and Air Temperature Curve Name",
        "  0.4,                     !- Design Fan Power Input Fraction",
        "  0.0001,                  !- Design Fan Air Flow Rate per Power Input {m3/s-W}",
        "  Data Center Servers Airflow fLoadTemp,  !- Air Flow Function of Loading and Air Temperature Curve Name",
        "  ECM FanPower fFlow,      !- Fan Power Input Function of Flow Curve Name",
        "  15,                      !- Design Entering Air Temperature {C}",
        "  A3,                      !- Environmental Class",
        "  AdjustedSupply,          !- Air Inlet Connection Type",
        "  ,                        !- Air Inlet Room Air Model Node Name",
        "  ,                        !- Air Outlet Room Air Model Node Name",
        "  Main Zone Inlet Node,    !- Supply Air Node Name",
        "  0.1,                     !- Design Recirculation Fraction",
        "  Data Center Recirculation fLoadTemp,  !- Recirculation Function of Loading and Supply Temperature Curve Name",
        "  0.9,                     !- Design Electric Power Supply Efficiency",
        "  UPS Efficiency fPLR,     !- Electric Power Supply Efficiency Function of Part Load Ratio Curve Name",
        "  1,                       !- Fraction of Electric Power Supply Losses to Zone",
        "  ITE-CPU,                 !- CPU End-Use Subcategory",
        "  ITE-Fans,                !- Fan End-Use Subcategory",
        "  ITE-UPS;                 !- Electric Power Supply End-Use Subcategory",

        "Curve:Quadratic,",
        "  ECM FanPower fFlow,      !- Name",
        "  0.0,                     !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Minimum Value of x",
        "  99.0;                    !- Maximum Value of x",

        "Curve:Quadratic,",
        "  UPS Efficiency fPLR,     !- Name",
        "  1.0,                     !- Coefficient1 Constant",
        "  0.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Minimum Value of x",
        "  99.0;                    !- Maximum Value of x",

        "Curve:Biquadratic,",
        "  Data Center Servers Power fLoadTemp,  !- Name",
        "  -1.0,                    !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.06667,                 !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "  Data Center Servers Airflow fLoadTemp,  !- Name",
        "  -1.4,                    !- Coefficient1 Constant",
        "  0.9,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.1,                     !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "  Data Center Recirculation fLoadTemp,  !- Name",
        "  1.0,                     !- Coefficient1 Constant",
        "  0.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_FALSE(has_err_output());
    state->init_state(*state);

    bool ErrorsFound(false);
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    InternalHeatGains::GetInternalHeatGainsInput(*state);

    // Set up a simple convective gain for each gain type
    int zoneNum = 1;
    int numGainTypes = static_cast<int>(DataHeatBalance::IntGainType::Num);
    Array1D<Real64> convGains({0, numGainTypes - 1});
    convGains = 0.0;
    Real64 totConvGains = 0.0;
    Real64 expectedTotConvGains = 0.0;

    for (int gainType = 0; gainType < numGainTypes; ++gainType) {
        convGains(gainType) = 100 * gainType;
        expectedTotConvGains += convGains(gainType);
        SetupZoneInternalGain(*state, zoneNum, "Gain", static_cast<DataHeatBalance::IntGainType>(gainType), &convGains(gainType));
    }

    InternalHeatGains::UpdateInternalGainValues(*state);

    // Check total of all convective gains
    totConvGains = InternalHeatGains::zoneSumAllInternalConvectionGains(*state, zoneNum);
    EXPECT_EQ(totConvGains, expectedTotConvGains);

    // Check subtotals used in zone component loads
    state->dataEnvrn->TotDesDays = 1;
    state->dataEnvrn->TotRunDesPersDays = 0;
    state->dataSize->CurOverallSimDay = 1;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStepsInHour = 10;
    state->dataGlobal->TimeStep = 1;
    OutputReportTabular::AllocateLoadComponentArrays(*state);
    int timeStepInDay = (state->dataGlobal->HourOfDay - 1) * state->dataGlobal->TimeStepsInHour + state->dataGlobal->TimeStep;

    state->dataGlobal->CompLoadReportIsReq = true;
    state->dataGlobal->isPulseZoneSizing = false;
    InternalHeatGains::GatherComponentLoadsIntGain(*state);
    auto &znCompLoadDayTS = state->dataOutRptTab->znCompLoads[state->dataSize->CurOverallSimDay - 1].ts[timeStepInDay - 1].spacezone[zoneNum - 1];
    totConvGains = znCompLoadDayTS.peopleInstantSeq + znCompLoadDayTS.lightInstantSeq + znCompLoadDayTS.equipInstantSeq +
                   znCompLoadDayTS.refrigInstantSeq + znCompLoadDayTS.waterUseInstantSeq + znCompLoadDayTS.hvacLossInstantSeq +
                   znCompLoadDayTS.powerGenInstantSeq;

    // Legitimate gain types excluded from this total
    expectedTotConvGains -=
        convGains(static_cast<int>(DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkCarbonDioxide)); // this is only used for CO2
    expectedTotConvGains -= convGains(
        static_cast<int>(DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkGenericContam)); // this is only used for generic contaminants
    expectedTotConvGains -= convGains(
        static_cast<int>(DataHeatBalance::IntGainType::DaylightingDeviceTubular)); // this is included in Fenestration Conduction - Sensible Instant

    // ** NOTE: If this unit test fails, the likely cause is that a new internal gain type was added, but it was not added to one of the subtotal
    // types in InternalHeatGains::GatherComponentLoadsIntGain() this also means that the new type may be missing from other places that collect
    // internal gains by subgroups, such as the room air models and output reporting for zone-level gains search for
    // "DataHeatBalance::IntGainType::Lights" for places where these types of subtotals occur and add the new type as appropriate
    EXPECT_EQ(totConvGains, expectedTotConvGains);

    // cleanup
    convGains.deallocate();
}

TEST_F(EnergyPlusFixture, InternalHeatGains_ElectricEquipITE_ApproachTemperatures)
{

    std::string const idf_objects = delimited_string({
        "Zone,Main Zone;",

        "ZoneHVAC:EquipmentConnections,",
        "  Main Zone,                   !- Zone Name",
        "  Main Zone Equipment,         !- Zone Conditioning Equipment List Name",
        "  Main Zone Inlet Node,        !- Zone Air Inlet Node or NodeList Name",
        "  ,                            !- Zone Air Exhaust Node or NodeList Name",
        "  Main Zone Node,              !- Zone Air Node Name",
        "  Main Zone Outlet Node;       !- Zone Return Air Node or NodeList Name",

        "ZoneHVAC:EquipmentList,",
        "  Main Zone Equipment,     !- Name",
        "  SequentialLoad,          !- Load Distribution Scheme",
        "  ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "  Main Zone ATU,           !- Zone Equipment 1 Name",
        "  1,                       !- Zone Equipment 1 Cooling Sequence",
        "  2,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "  ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "  ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "  ZoneHVAC:Baseboard:Convective:Electric,  !- Zone Equipment 2 Object Type",
        "  Main Zone Baseboard,     !- Zone Equipment 2 Name",
        "  2,                       !- Zone Equipment 2 Cooling Sequence",
        "  1,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "  ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
        "  ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

        "ZoneHVAC:AirDistributionUnit,",
        "  Main Zone ATU,               !- Name",
        "  Main Zone Inlet Node,        !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "  Main Zone VAV Air;           !- Air Terminal Name",

        "AirTerminal:SingleDuct:VAV:NoReheat,",
        "  Main Zone VAV Air,           !- Name",
        "  System Availability Schedule,  !- Availability Schedule Name",
        "  Main Zone Inlet Node,    !- Air Outlet Node Name",
        "  Main Zone ATU In Node,   !- Air Inlet Node Name",
        "  8.5,                     !- Maximum Air Flow Rate {m3/s}",
        "  Constant,                !- Zone Minimum Air Flow Input Method",
        "  0.05;                    !- Constant Minimum Air Flow Fraction",

        "ZoneHVAC:Baseboard:Convective:Electric,",
        "  Main Zone Baseboard,     !- Name",
        "  System Availability Schedule,  !- Availability Schedule Name",
        "  HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "  8000,                    !- Heating Design Capacity {W}",
        "  ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "  ,                        !- Fraction of Autosized Heating Design Capacity",
        "  0.97;                    !- Efficiency",

        "ElectricEquipment:ITE:AirCooled,",
        "  Data Center Servers,     !- Name",
        "  Main Zone,               !- Zone Name",
        "  FlowControlWithApproachTemperatures,    !- Calculation Method",
        "  Watts/Unit,              !- Design Power Input Calculation Method",
        "  500,                     !- Watts per Unit {W}",
        "  100,                     !- Number of Units",
        "  ,                        !- Watts per Zone Floor Area {W/m2}",
        "  ,  !- Design Power Input Schedule Name",
        "  ,  !- CPU Loading  Schedule Name",
        "  Data Center Servers Power fLoadTemp,  !- CPU Power Input Function of Loading and Air Temperature Curve Name",
        "  0.4,                     !- Design Fan Power Input Fraction",
        "  0.0001,                  !- Design Fan Air Flow Rate per Power Input {m3/s-W}",
        "  Data Center Servers Airflow fLoadTemp,  !- Air Flow Function of Loading and Air Temperature Curve Name",
        "  ECM FanPower fFlow,      !- Fan Power Input Function of Flow Curve Name",
        "  15,                      !- Design Entering Air Temperature {C}",
        "  A3,                      !- Environmental Class",
        "  AdjustedSupply,          !- Air Inlet Connection Type",
        "  ,                        !- Air Inlet Room Air Model Node Name",
        "  ,                        !- Air Outlet Room Air Model Node Name",
        "  Main Zone Inlet Node,    !- Supply Air Node Name",
        "  0.1,                     !- Design Recirculation Fraction",
        "  Data Center Recirculation fLoadTemp,  !- Recirculation Function of Loading and Supply Temperature Curve Name",
        "  0.9,                     !- Design Electric Power Supply Efficiency",
        "  UPS Efficiency fPLR,     !- Electric Power Supply Efficiency Function of Part Load Ratio Curve Name",
        "  1,                       !- Fraction of Electric Power Supply Losses to Zone",
        "  ITE-CPU,                 !- CPU End-Use Subcategory",
        "  ITE-Fans,                !- Fan End-Use Subcategory",
        "  ITE-UPS,                 !- Electric Power Supply End-Use Subcategory",
        "  2,                       !- Supply Approach Temperature",
        "  ,                        !- Supply Approach Temperature Schedule",
        "  -2,                      !- Return Approach Temperature",
        "  ;                        !- Return Approach Temperature Schedule",

        "Curve:Quadratic,",
        "  ECM FanPower fFlow,      !- Name",
        "  0.0,                     !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Minimum Value of x",
        "  99.0;                    !- Maximum Value of x",

        "Curve:Quadratic,",
        "  UPS Efficiency fPLR,     !- Name",
        "  1.0,                     !- Coefficient1 Constant",
        "  0.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Minimum Value of x",
        "  99.0;                    !- Maximum Value of x",

        "Curve:Biquadratic,",
        "  Data Center Servers Power fLoadTemp,  !- Name",
        "  -1.0,                    !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.06667,                 !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "  Data Center Servers Airflow fLoadTemp,  !- Name",
        "  -1.4,                    !- Coefficient1 Constant",
        "  0.9,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.1,                     !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "  Data Center Recirculation fLoadTemp,  !- Name",
        "  1.0,                     !- Coefficient1 Constant",
        "  0.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_FALSE(has_err_output());

    bool ErrorsFound(false);
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataHeatBal->ZoneRpt.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig.allocate(1);

    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.008;

    InternalHeatGains::GetInternalHeatGainsInput(*state);

    state->dataLoopNodes->Node(1).Temp = 45.0;
    InternalHeatGains::CalcZoneITEq(*state);
    auto &thisZoneITEq = state->dataHeatBal->ZoneITEq(1);
    ASSERT_DOUBLE_EQ(thisZoneITEq.AirOutletDryBulbT + thisZoneITEq.ReturnApproachTemp, state->dataHeatBal->Zone(1).AdjustedReturnTempByITE);
    ASSERT_DOUBLE_EQ(state->dataLoopNodes->Node(1).Temp + thisZoneITEq.SupplyApproachTemp, thisZoneITEq.AirInletDryBulbT);
}

TEST_F(EnergyPlusFixture, InternalHeatGains_ElectricEquipITE_DefaultCurves)
{
    using namespace DataHeatBalance;

    std::string const idf_objects =
        delimited_string({"Zone,Main Zone;",

                          "ZoneHVAC:EquipmentConnections,",
                          "  Main Zone,                   !- Zone Name",
                          "  Main Zone Equipment,         !- Zone Conditioning Equipment List Name",
                          "  Main Zone Inlet Node,        !- Zone Air Inlet Node or NodeList Name",
                          "  ,                            !- Zone Air Exhaust Node or NodeList Name",
                          "  Main Zone Node,              !- Zone Air Node Name",
                          "  Main Zone Outlet Node;       !- Zone Return Air Node or NodeList Name",

                          "ZoneHVAC:EquipmentList,",
                          "  Main Zone Equipment,     !- Name",
                          "  SequentialLoad,          !- Load Distribution Scheme",
                          "  ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
                          "  Main Zone ATU,           !- Zone Equipment 1 Name",
                          "  1,                       !- Zone Equipment 1 Cooling Sequence",
                          "  2,                       !- Zone Equipment 1 Heating or No-Load Sequence",
                          "  ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
                          "  ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
                          "  ZoneHVAC:Baseboard:Convective:Electric,  !- Zone Equipment 2 Object Type",
                          "  Main Zone Baseboard,     !- Zone Equipment 2 Name",
                          "  2,                       !- Zone Equipment 2 Cooling Sequence",
                          "  1,                       !- Zone Equipment 2 Heating or No-Load Sequence",
                          "  ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
                          "  ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

                          "ZoneHVAC:AirDistributionUnit,",
                          "  Main Zone ATU,               !- Name",
                          "  Main Zone Inlet Node,        !- Air Distribution Unit Outlet Node Name",
                          "  AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
                          "  Main Zone VAV Air;           !- Air Terminal Name",

                          "AirTerminal:SingleDuct:VAV:NoReheat,",
                          "  Main Zone VAV Air,           !- Name",
                          "  System Availability Schedule,  !- Availability Schedule Name",
                          "  Main Zone Inlet Node,    !- Air Outlet Node Name",
                          "  Main Zone ATU In Node,   !- Air Inlet Node Name",
                          "  8.5,                     !- Maximum Air Flow Rate {m3/s}",
                          "  Constant,                !- Zone Minimum Air Flow Input Method",
                          "  0.05;                    !- Constant Minimum Air Flow Fraction",

                          "ZoneHVAC:Baseboard:Convective:Electric,",
                          "  Main Zone Baseboard,     !- Name",
                          "  System Availability Schedule,  !- Availability Schedule Name",
                          "  HeatingDesignCapacity,   !- Heating Design Capacity Method",
                          "  8000,                    !- Heating Design Capacity {W}",
                          "  ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
                          "  ,                        !- Fraction of Autosized Heating Design Capacity",
                          "  0.97;                    !- Efficiency",

                          "ElectricEquipment:ITE:AirCooled,",
                          "  Data Center Servers,     !- Name",
                          "  Main Zone,               !- Zone Name",
                          "  ,                        !- Air Flow Calculation Method",
                          "  Watts/Unit,              !- Design Power Input Calculation Method",
                          "  500,                     !- Watts per Unit {W}",
                          "  100,                     !- Number of Units",
                          "  ,                        !- Watts per Zone Floor Area {W/m2}",
                          "  ,                        !- Design Power Input Schedule Name",
                          "  ,                        !- CPU Loading  Schedule Name",
                          "  Data Center Servers Power fLoadTemp,        !- CPU Power Input Function of Loading and Air Temperature Curve Name",
                          "  0.4,                     !- Design Fan Power Input Fraction",
                          "  0.0001,                  !- Design Fan Air Flow Rate per Power Input {m3/s-W}",
                          "  Data Center Servers Airflow fLoadTemp,      !- Air Flow Function of Loading and Air Temperature Curve Name",
                          "  ECM FanPower fFlow,      !- Fan Power Input Function of Flow Curve Name",
                          "  15,                      !- Design Entering Air Temperature {C}",
                          "  A3,                      !- Environmental Class",
                          "  AdjustedSupply,          !- Air Inlet Connection Type",
                          "  ,                        !- Air Inlet Room Air Model Node Name",
                          "  ,                        !- Air Outlet Room Air Model Node Name",
                          "  Main Zone Inlet Node,    !- Supply Air Node Name",
                          "  0.1,                     !- Design Recirculation Fraction",
                          // This one should be assumed to always 1
                          "  ,                        !- Recirculation Function of Loading and Supply Temperature Curve Name",
                          "  0.9,                     !- Design Electric Power Supply Efficiency",
                          // This one should be assumed to always 1
                          "  ,                        !- Electric Power Supply Efficiency Function of Part Load Ratio Curve Name",
                          "  1,                       !- Fraction of Electric Power Supply Losses to Zone",
                          "  ITE-CPU,                 !- CPU End-Use Subcategory",
                          "  ITE-Fans,                !- Fan End-Use Subcategory",
                          "  ITE-UPS;                 !- Electric Power Supply End-Use Subcategory",

                          "Curve:Quadratic,",
                          "  ECM FanPower fFlow,      !- Name",
                          "  0.0,                     !- Coefficient1 Constant",
                          "  1.0,                     !- Coefficient2 x",
                          "  0.0,                     !- Coefficient3 x**2",
                          "  0.0,                     !- Minimum Value of x",
                          "  99.0;                    !- Maximum Value of x",

                          "Curve:Biquadratic,",
                          "  Data Center Servers Power fLoadTemp,  !- Name",
                          "  -1.0,                    !- Coefficient1 Constant",
                          "  1.0,                     !- Coefficient2 x",
                          "  0.0,                     !- Coefficient3 x**2",
                          "  0.06667,                 !- Coefficient4 y",
                          "  0.0,                     !- Coefficient5 y**2",
                          "  0.0,                     !- Coefficient6 x*y",
                          "  0.0,                     !- Minimum Value of x",
                          "  1.5,                     !- Maximum Value of x",
                          "  -10,                     !- Minimum Value of y",
                          "  99.0,                    !- Maximum Value of y",
                          "  0.0,                     !- Minimum Curve Output",
                          "  99.0,                    !- Maximum Curve Output",
                          "  Dimensionless,           !- Input Unit Type for X",
                          "  Temperature,             !- Input Unit Type for Y",
                          "  Dimensionless;           !- Output Unit Type",

                          "Curve:Biquadratic,",
                          "  Data Center Servers Airflow fLoadTemp,  !- Name",
                          "  -1.4,                    !- Coefficient1 Constant",
                          "  0.9,                     !- Coefficient2 x",
                          "  0.0,                     !- Coefficient3 x**2",
                          "  0.1,                     !- Coefficient4 y",
                          "  0.0,                     !- Coefficient5 y**2",
                          "  0.0,                     !- Coefficient6 x*y",
                          "  0.0,                     !- Minimum Value of x",
                          "  1.5,                     !- Maximum Value of x",
                          "  -10,                     !- Minimum Value of y",
                          "  99.0,                    !- Maximum Value of y",
                          "  0.0,                     !- Minimum Curve Output",
                          "  99.0,                    !- Maximum Curve Output",
                          "  Dimensionless,           !- Input Unit Type for X",
                          "  Temperature,             !- Input Unit Type for Y",
                          "  Dimensionless;           !- Output Unit Type"});

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_FALSE(has_err_output());

    state->init_state(*state);
    bool ErrorsFound(false);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.008;

    InternalHeatGains::GetInternalHeatGainsInput(*state);
    InternalHeatGains::CalcZoneITEq(*state);

    auto &thisZoneITEq = state->dataHeatBal->ZoneITEq(1);
    // If Electric Power Supply Efficiency Function of Part Load Ratio Curve Name is blank => always 1, so UPSPower is calculated as such
    Real64 DefaultUPSPower = (thisZoneITEq.PowerRpt[(int)PERptVars::CPU] + thisZoneITEq.PowerRpt[(int)PERptVars::Fan]) *
                             max((1.0 - thisZoneITEq.DesignUPSEfficiency), 0.0);

    ASSERT_EQ(DefaultUPSPower, thisZoneITEq.PowerRpt[(int)PERptVars::UPS]);
}

TEST_F(EnergyPlusFixture, InternalHeatGains_CheckThermalComfortSchedules)
{

    bool WorkEffSchPresent; // true equals blank, false equals not blank
    bool CloInsSchPresent;  // true equals blank, false equals not blank
    bool AirVelSchPresent;  // true equals blank, false equals not blank
    bool FunctionCallResult;
    bool ExpectedResult;

    // Test 1: everything blank--should result in false result
    WorkEffSchPresent = true;
    CloInsSchPresent = true;
    AirVelSchPresent = true;
    ExpectedResult = false;
    FunctionCallResult = EnergyPlus::InternalHeatGains::CheckThermalComfortSchedules(WorkEffSchPresent, CloInsSchPresent, AirVelSchPresent);
    EXPECT_EQ(ExpectedResult, FunctionCallResult);

    // Additional Tests: test various combinations where at least one flag is not blank (false)--should result in a true result
    WorkEffSchPresent = false;
    CloInsSchPresent = true;
    AirVelSchPresent = true;
    ExpectedResult = true;
    FunctionCallResult = EnergyPlus::InternalHeatGains::CheckThermalComfortSchedules(WorkEffSchPresent, CloInsSchPresent, AirVelSchPresent);
    EXPECT_EQ(ExpectedResult, FunctionCallResult);

    WorkEffSchPresent = true;
    CloInsSchPresent = false;
    AirVelSchPresent = true;
    ExpectedResult = true;
    FunctionCallResult = EnergyPlus::InternalHeatGains::CheckThermalComfortSchedules(WorkEffSchPresent, CloInsSchPresent, AirVelSchPresent);
    EXPECT_EQ(ExpectedResult, FunctionCallResult);

    WorkEffSchPresent = true;
    CloInsSchPresent = true;
    AirVelSchPresent = false;
    ExpectedResult = true;
    FunctionCallResult = EnergyPlus::InternalHeatGains::CheckThermalComfortSchedules(WorkEffSchPresent, CloInsSchPresent, AirVelSchPresent);
    EXPECT_EQ(ExpectedResult, FunctionCallResult);

    WorkEffSchPresent = false;
    CloInsSchPresent = false;
    AirVelSchPresent = true;
    ExpectedResult = true;
    FunctionCallResult = EnergyPlus::InternalHeatGains::CheckThermalComfortSchedules(WorkEffSchPresent, CloInsSchPresent, AirVelSchPresent);
    EXPECT_EQ(ExpectedResult, FunctionCallResult);

    WorkEffSchPresent = false;
    CloInsSchPresent = true;
    AirVelSchPresent = false;
    ExpectedResult = true;
    FunctionCallResult = EnergyPlus::InternalHeatGains::CheckThermalComfortSchedules(WorkEffSchPresent, CloInsSchPresent, AirVelSchPresent);
    EXPECT_EQ(ExpectedResult, FunctionCallResult);

    WorkEffSchPresent = true;
    CloInsSchPresent = false;
    AirVelSchPresent = false;
    ExpectedResult = true;
    FunctionCallResult = EnergyPlus::InternalHeatGains::CheckThermalComfortSchedules(WorkEffSchPresent, CloInsSchPresent, AirVelSchPresent);
    EXPECT_EQ(ExpectedResult, FunctionCallResult);

    WorkEffSchPresent = false;
    CloInsSchPresent = false;
    AirVelSchPresent = false;
    ExpectedResult = true;
    FunctionCallResult = EnergyPlus::InternalHeatGains::CheckThermalComfortSchedules(WorkEffSchPresent, CloInsSchPresent, AirVelSchPresent);
    EXPECT_EQ(ExpectedResult, FunctionCallResult);
}
TEST_F(EnergyPlusFixture, InternalHeatGains_ZnRpt_Outputs)
{

    std::string const idf_objects = delimited_string({
        "Zone,Main Zone;",

        "Space,",
        "Space 1,            !- Name",
        "Main Zone,             !- Zone Name",
        ",                   !- Ceiling Height {m}",
        ",                   !- Volume {m3}",
        "5.0;                !- Floor Area {m2}",

        "Space,",
        "Space 2,            !- Name",
        "Main Zone,             !- Zone Name",
        ",                   !- Ceiling Height {m}",
        ",                   !- Volume {m3}",
        "15.0;                !- Floor Area {m2}",
        "SpaceList, All Spaces, Space 1, Space 2;"

        "ZoneHVAC:EquipmentConnections,",
        "  Main Zone,                   !- Zone Name",
        "  Main Zone Equipment,         !- Zone Conditioning Equipment List Name",
        "  Main Zone Inlet Node,        !- Zone Air Inlet Node or NodeList Name",
        "  ,                            !- Zone Air Exhaust Node or NodeList Name",
        "  Main Zone Node,              !- Zone Air Node Name",
        "  Main Zone Outlet Node;       !- Zone Return Air Node or NodeList Name",

        "ZoneHVAC:EquipmentList,",
        "  Main Zone Equipment,     !- Name",
        "  SequentialLoad,          !- Load Distribution Scheme",
        "  ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "  Main Zone ATU,           !- Zone Equipment 1 Name",
        "  1,                       !- Zone Equipment 1 Cooling Sequence",
        "  2,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "  ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "  ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "  ZoneHVAC:Baseboard:Convective:Electric,  !- Zone Equipment 2 Object Type",
        "  Main Zone Baseboard,     !- Zone Equipment 2 Name",
        "  2,                       !- Zone Equipment 2 Cooling Sequence",
        "  1,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "  ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
        "  ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

        "ZoneHVAC:AirDistributionUnit,",
        "  Main Zone ATU,               !- Name",
        "  Main Zone Inlet Node,        !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "  Main Zone VAV Air;           !- Air Terminal Name",

        "AirTerminal:SingleDuct:VAV:NoReheat,",
        "  Main Zone VAV Air,           !- Name",
        "  System Availability Schedule,  !- Availability Schedule Name",
        "  Main Zone Inlet Node,    !- Air Outlet Node Name",
        "  Main Zone ATU In Node,   !- Air Inlet Node Name",
        "  8.5,                     !- Maximum Air Flow Rate {m3/s}",
        "  Constant,                !- Zone Minimum Air Flow Input Method",
        "  0.05;                    !- Constant Minimum Air Flow Fraction",

        "ZoneHVAC:Baseboard:Convective:Electric,",
        "  Main Zone Baseboard,     !- Name",
        "  System Availability Schedule,  !- Availability Schedule Name",
        "  HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "  8000,                    !- Heating Design Capacity {W}",
        "  ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "  ,                        !- Fraction of Autosized Heating Design Capacity",
        "  0.97;                    !- Efficiency",

        "ScheduleTypeLimits,SchType1,0.0,1.0,Continuous,Dimensionless;",

        "Schedule:Constant,Schedule1,,1.0;",

        "  People,",
        "    Main Zone People,        !- Name",
        "    Main Zone,               !- Zone or ZoneList Name",
        "    Schedule1,               !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    3.000000,                !- Number of People",
        "    ,                        !- People per Zone Floor Area{ person / m2 }",
        "    ,                        !- Zone Floor Area per Person{ m2 / person }",
        "    0.3000000,               !- Fraction Radiant",
        "    0.5,                     !- Sensible Heat Fraction",
        "    Schedule1,               !- Activity Level Schedule Name",
        "    3.82E-8;                 !- Carbon Dioxide Generation Rate{ m3 / s - W }",

        "  Lights,",
        "    Main Zone Lights,        !- Name",
        "    Main Zone,               !- Zone or ZoneList Name",
        "    Schedule1,               !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    100.0,                   !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0.0000,                  !- Return Air Fraction",
        "    0.7000,                  !- Fraction Radiant",
        "    0.2000,                  !- Fraction Visible",
        "    1.0000,                  !- Fraction Replaceable",
        "    General,                 !- End-Use Subcategory",
        "    No;                      !- Return Air Fraction Calculated from Plenum Temperature",

        "  ElectricEquipment,",
        "    Main Zone Electric Equipment,  !- Name",
        "    Main Zone,               !- Zone or ZoneList Name",
        "    Schedule1,               !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    150.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0.0000,                  !- Fraction Latent",
        "    0.5000,                  !- Fraction Radiant",
        "    0.0000;                  !- Fraction Lost",

        "  GasEquipment,",
        "    Main Zone Gas Equipment,  !- Name",
        "    Main Zone,               !- Zone or ZoneList Name",
        "    Schedule1,               !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    200.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0.0000,                  !- Fraction Latent",
        "    0.5000,                  !- Fraction Radiant",
        "    0.0000,                  !- Fraction Lost",
        "    1.0E-7;                  !- Carbon Dioxide Generation Rate {m3/s-W}",

        "  HotWaterEquipment,",
        "    Main Zone Hot Water Equipment,  !- Name",
        "    Main Zone,               !- Zone or ZoneList Name",
        "    Schedule1,               !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    250.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0.0000,                  !- Fraction Latent",
        "    0.5000,                  !- Fraction Radiant",
        "    0.0000;                  !- Fraction Lost",

        "  SteamEquipment,",
        "    Main Zone Steam Equipment,  !- Name",
        "    Main Zone,               !- Zone or ZoneList Name",
        "    Schedule1,               !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    300.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0.0000,                  !- Fraction Latent",
        "    0.5000,                  !- Fraction Radiant",
        "    0.0000;                  !- Fraction Lost",

        "  OtherEquipment,",
        "    Main Zone Other Equipment,  !- Name",
        "    OtherFuel1,              !- Fuel Type",
        "    Main Zone,               !- Zone or ZoneList Name",
        "    Schedule1,               !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    350.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0.0000,                  !- Fraction Latent",
        "    0.5000,                  !- Fraction Radiant",
        "    0.0000,                  !- Fraction Lost",
        "    2.0E-7;                  !- Carbon Dioxide Generation Rate {m3/s-W}",

        "  OtherEquipment,",
        "    Main Zone Other Equipment2,  !- Name",
        "    FuelOilNo2,              !- Fuel Type",
        "    Main Zone,               !- Zone or ZoneList Name",
        "    Schedule1,               !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    375.0,                   !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0.0000,                  !- Fraction Latent",
        "    0.5000,                  !- Fraction Radiant",
        "    0.0000,                  !- Fraction Lost",
        "    2.0E-7;                  !- Carbon Dioxide Generation Rate {m3/s-W}",

        "  ZoneBaseboard:OutdoorTemperatureControlled,",
        "    Main Zone BBHeat,           !- Name",
        "    Main Zone,                  !- Zone Name",
        "    Schedule1,               !- Schedule Name",
        "    1500,                    !- Capacity at Low Temperature {W}",
        "    0,                       !- Low Temperature {C}",
        "    500,                     !- Capacity at High Temperature {W}",
        "    10,                      !- High Temperature {C}",
        "    0.5,                     !- Fraction Radiant",
        "    Baseboard Heat;          !- End - Use Subcategory",

        "  ZoneContaminantSourceAndSink:CarbonDioxide,",
        "    CO2people,               !- Name",
        "    Main Zone,               !- Zone Name",
        "    0.0001125,               !- Design Generation Rate{ m3 / s }",
        "    Schedule1;               !- Schedule Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_FALSE(has_err_output());

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    bool ErrorsFound(false);

    state->dataEnvrn->DayOfYear_Schedule = 1;
    state->dataEnvrn->DayOfMonth = 1;
    state->dataEnvrn->DayOfWeek = 1;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;
    Sched::UpdateScheduleVals(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    ZoneEquipmentManager::GetZoneEquipment(*state);
    HeatBalanceManager::AllocateHeatBalArrays(*state);
    state->dataHeatBal->Zone(1).FloorArea = 20.0;
    state->dataHeatBal->space(1).FloorArea = 5.0;
    state->dataHeatBal->space(2).FloorArea = 15.0;

    InternalHeatGains::GetInternalHeatGainsInput(*state);

    EXPECT_EQ(state->dataHeatBal->TotPeople, 2);
    EXPECT_EQ(state->dataHeatBal->TotLights, 2);
    EXPECT_EQ(state->dataHeatBal->TotElecEquip, 2);
    EXPECT_EQ(state->dataHeatBal->TotGasEquip, 2);
    EXPECT_EQ(state->dataHeatBal->TotHWEquip, 2);
    EXPECT_EQ(state->dataHeatBal->TotStmEquip, 2);
    EXPECT_EQ(state->dataHeatBal->TotOthEquip, 4);
    EXPECT_EQ(state->dataHeatBal->TotBBHeat, 2);

    EnergyPlus::createFacilityElectricPowerServiceObject(*state); // Needs to happen before InitInternalHeatGains

    // First time should be all good, because ZoneRpt/spaceRpt values initialize to zero
    InternalHeatGains::InitInternalHeatGains(*state);
    InternalHeatGains::ReportInternalHeatGains(*state);

    auto &zoneRpt1 = state->dataHeatBal->ZoneRpt(1);
    EXPECT_NEAR(zoneRpt1.LtsPower, 100.0, 0.01);
    EXPECT_NEAR(zoneRpt1.ElecPower, 150.0, 0.01);
    EXPECT_NEAR(zoneRpt1.OtherPower[(int)Constant::eFuel::OtherFuel1], 350.0, 0.01);
    EXPECT_NEAR(zoneRpt1.OtherPower[(int)Constant::eFuel::FuelOilNo2], 375.0, 0.01);
    EXPECT_NEAR(zoneRpt1.GasPower, 200.0, 0.01);
    EXPECT_NEAR(zoneRpt1.HWPower, 250.0, 0.01);
    EXPECT_NEAR(zoneRpt1.SteamPower, 300.0, 0.01);
    EXPECT_NEAR(zoneRpt1.BaseHeatPower, 1500.0, 0.01);
    EXPECT_NEAR(zoneRpt1.CO2Rate, 0.0001125, 0.01);
    EXPECT_NEAR(zoneRpt1.ITEqSHI, 0, 0.01);

    auto &spaceRpt1 = state->dataHeatBal->spaceRpt(1);
    EXPECT_NEAR(spaceRpt1.LtsPower, 100.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.ElecPower, 150.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.OtherPower[(int)Constant::eFuel::OtherFuel1], 350.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.OtherPower[(int)Constant::eFuel::FuelOilNo2], 375.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.GasPower, 200.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.HWPower, 250.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.SteamPower, 300.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.BaseHeatPower, 1500.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.CO2Rate, 0.0001125 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.ITEqSHI, 0, 0.01);

    auto &spaceRpt2 = state->dataHeatBal->spaceRpt(2);
    EXPECT_NEAR(spaceRpt2.LtsPower, 100.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.ElecPower, 150.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.OtherPower[(int)Constant::eFuel::OtherFuel1], 350.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.OtherPower[(int)Constant::eFuel::FuelOilNo2], 375.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.GasPower, 200.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.HWPower, 250.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.SteamPower, 300.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.BaseHeatPower, 1500.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.CO2Rate, 0.0001125 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.ITEqSHI, 0, 0.01);

    // Not implemented yet EXPECT_EQ(spaceRpt1.CO2Rate, 0.0001125);
    EXPECT_EQ(spaceRpt1.ITEqSHI, 0);

    // Second time should should give the same answers, because everything should reset before accumulating
    InternalHeatGains::InitInternalHeatGains(*state);
    InternalHeatGains::ReportInternalHeatGains(*state);

    EXPECT_EQ(zoneRpt1.LtsPower, 100.0);
    EXPECT_EQ(zoneRpt1.ElecPower, 150.0);
    EXPECT_EQ(zoneRpt1.OtherPower[(int)Constant::eFuel::OtherFuel1], 350.0);
    EXPECT_EQ(zoneRpt1.OtherPower[(int)Constant::eFuel::FuelOilNo2], 375.0);
    EXPECT_EQ(zoneRpt1.GasPower, 200.0);
    EXPECT_EQ(zoneRpt1.HWPower, 250.0);
    EXPECT_EQ(zoneRpt1.SteamPower, 300.0);
    EXPECT_EQ(zoneRpt1.BaseHeatPower, 1500.0);
    EXPECT_EQ(zoneRpt1.CO2Rate, 0.0001125);
    EXPECT_EQ(zoneRpt1.ITEqSHI, 0);

    EXPECT_NEAR(spaceRpt1.LtsPower, 100.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.ElecPower, 150.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.OtherPower[(int)Constant::eFuel::OtherFuel1], 350.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.OtherPower[(int)Constant::eFuel::FuelOilNo2], 375.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.GasPower, 200.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.HWPower, 250.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.SteamPower, 300.0 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.BaseHeatPower, 1500.0 * 0.25, 0.01);
    // space CO2 not implemented yet - EXPECT_NEAR(spaceRpt1.CO2Rate, 0.0001125 * 0.25, 0.01);
    EXPECT_NEAR(spaceRpt1.ITEqSHI, 0, 0.01);

    EXPECT_NEAR(spaceRpt2.LtsPower, 100.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.ElecPower, 150.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.OtherPower[(int)Constant::eFuel::OtherFuel1], 350.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.OtherPower[(int)Constant::eFuel::FuelOilNo2], 375.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.GasPower, 200.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.HWPower, 250.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.SteamPower, 300.0 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.BaseHeatPower, 1500.0 * 0.75, 0.01);
    // space CO2 not implemented yet - EXPECT_NEAR(spaceRpt2.CO2Rate, 0.0001125 * 0.75, 0.01);
    EXPECT_NEAR(spaceRpt2.ITEqSHI, 0, 0.01);
}

TEST_F(EnergyPlusFixture, InternalHeatGains_AdjustedSupplyGoodInletNode)
{
    std::string const idf_objects = delimited_string({
        "  Zone,",
        "    Main Zone,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    Main Zone,               !- Zone Name",
        "    Main Zone Equipment,     !- Zone Conditioning Equipment List Name",
        "    Main Zone Inlet Node,    !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Main Zone Node,          !- Zone Air Node Name",
        "    Main Zone Outlet Node;   !- Zone Return Air Node or NodeList Name",

        "  ZoneHVAC:EquipmentList,",
        "    Main Zone Equipment,     !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    Main Zone ATU,           !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    2,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "    ZoneHVAC:Baseboard:Convective:Electric,  !- Zone Equipment 2 Object Type",
        "    Main Zone Baseboard,     !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    1,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

        "  ZoneHVAC:AirDistributionUnit,",
        "    Main Zone ATU,           !- Name",
        "    Main Zone Inlet Node,    !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    Main Zone VAV Air;       !- Air Terminal Name",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    Main Zone VAV Air,       !- Name",
        "    System Availability Schedule,  !- Availability Schedule Name",
        "    Main Zone Inlet Node,    !- Air Outlet Node Name",
        "    Main Zone ATU In Node,   !- Air Inlet Node Name",
        "    8.5,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.05;                    !- Constant Minimum Air Flow Fraction",

        "  ZoneHVAC:Baseboard:Convective:Electric,",
        "    Main Zone Baseboard,     !- Name",
        "    System Availability Schedule,  !- Availability Schedule Name",
        "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "    8000,                    !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    0.97;                    !- Efficiency",

        "  ElectricEquipment:ITE:AirCooled,",
        "    Data Center Servers,     !- Name",
        "    Main Zone,               !- Zone Name",
        "    FlowFromSystem,          !- Air Flow Calculation Method",
        "    Watts/Unit,              !- Design Power Input Calculation Method",
        "    500,                     !- Watts per Unit {W}",
        "    100,                     !- Number of Units",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    Data Center Operation Schedule,  !- Design Power Input Schedule Name",
        "    Data Center CPU Loading Schedule,  !- CPU Loading  Schedule Name",
        "    Data Center Servers Power fLoadTemp,  !- CPU Power Input Function of Loading and Air Temperature Curve Name",
        "    0.4,                     !- Design Fan Power Input Fraction",
        "    0.0001,                  !- Design Fan Air Flow Rate per Power Input {m3/s-W}",
        "    Data Center Servers Airflow fLoadTemp,  !- Air Flow Function of Loading and Air Temperature Curve Name",
        "    ECM FanPower fFlow,      !- Fan Power Input Function of Flow Curve Name",
        "    15,                      !- Design Entering Air Temperature {C}",
        "    A3,                      !- Environmental Class",
        "    AdjustedSupply,          !- Air Inlet Connection Type",
        "    ,                        !- Air Inlet Room Air Model Node Name",
        "    ,                        !- Air Outlet Room Air Model Node Name",
        "    Main Zone Inlet Node,    !- Supply Air Node Name",
        "    0.1,                     !- Design Recirculation Fraction",
        "    Data Center Recirculation fLoadTemp,  !- Recirculation Function of Loading and Supply Temperature Curve Name",
        "    0.9,                     !- Design Electric Power Supply Efficiency",
        "    UPS Efficiency fPLR,     !- Electric Power Supply Efficiency Function of Part Load Ratio Curve Name",
        "    1,                       !- Fraction of Electric Power Supply Losses to Zone",
        "    ITE-CPU,                 !- CPU End-Use Subcategory",
        "    ITE-Fans,                !- Fan End-Use Subcategory",
        "    ITE-UPS;                 !- Electric Power Supply End-Use Subcategory",

        "  Curve:Quadratic,",
        "    ECM FanPower fFlow,      !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    99.0;                    !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    UPS Efficiency fPLR,     !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    99.0;                    !- Maximum Value of x",

        "  Curve:Biquadratic,",
        "    Data Center Servers Power fLoadTemp,  !- Name",
        "    -1.0,                    !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.06667,                 !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Data Center Servers Airflow fLoadTemp,  !- Name",
        "    -1.4,                    !- Coefficient1 Constant",
        "    0.9,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.1,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Data Center Recirculation fLoadTemp,  !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Schedule:Constant,Data Center Operation Schedule,Any Number,1.0;",

        "  Schedule:Compact,",
        "    Data Center CPU Loading Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 1/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0,        !- Field 3",
        "    Through: 2/29,           !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    Until: 24:00,0.50,       !- Field 7",
        "    Through: 3/31,           !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    Until: 24:00,0.75,       !- Field 11",
        "    Through: 4/30,           !- Field 13",
        "    For: AllDays,            !- Field 14",
        "    Until: 24:00,1.0,        !- Field 15",
        "    Through: 5/31,           !- Field 17",
        "    For: AllDays,            !- Field 18",
        "    Until: 24:00,0.25,       !- Field 19",
        "    Through: 6/30,           !- Field 21",
        "    For: AllDays,            !- Field 22",
        "    Until: 24:00,0.50,       !- Field 23",
        "    Through: 7/31,           !- Field 25",
        "    For: AllDays,            !- Field 26",
        "    Until: 24:00,0.1,        !- Field 27",
        "    Through: 8/31,           !- Field 29",
        "    For: AllDays,            !- Field 30",
        "    Until: 24:00,1.0,        !- Field 31",
        "    Through: 9/30,           !- Field 33",
        "    For: AllDays,            !- Field 34",
        "    Until: 24:00,0.25,       !- Field 35",
        "    Through: 10/31,          !- Field 37",
        "    For: AllDays,            !- Field 38",
        "    Until: 24:00,0.50,       !- Field 39",
        "    Through: 11/30,          !- Field 41",
        "    For: AllDays,            !- Field 42",
        "    Until: 24:00,0.75,       !- Field 43",
        "    Through: 12/31,          !- Field 45",
        "    For: AllDays,            !- Field 46",
        "    Until: 24:00,1.00;       !- Field 47",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false);
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.008;

    InternalHeatGains::GetInternalHeatGainsInput(*state);
    ASSERT_FALSE(ErrorsFound);
}

TEST_F(EnergyPlusFixture, InternalHeatGains_AdjustedSupplyBadInletNode)
{
    std::string const idf_objects = delimited_string({
        "  Zone,",
        "    Main Zone,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    Main Zone,               !- Zone Name",
        "    Main Zone Equipment,     !- Zone Conditioning Equipment List Name",
        "    Main Zone Inlet Node,    !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Main Zone Node,          !- Zone Air Node Name",
        "    Main Zone Outlet Node;   !- Zone Return Air Node or NodeList Name",

        "  ZoneHVAC:EquipmentList,",
        "    Main Zone Equipment,     !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    Main Zone ATU,           !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    2,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "    ZoneHVAC:Baseboard:Convective:Electric,  !- Zone Equipment 2 Object Type",
        "    Main Zone Baseboard,     !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    1,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

        "  ZoneHVAC:AirDistributionUnit,",
        "    Main Zone ATU,           !- Name",
        "    Main Zone Inlet Node,    !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    Main Zone VAV Air;       !- Air Terminal Name",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    Main Zone VAV Air,       !- Name",
        "    System Availability Schedule,  !- Availability Schedule Name",
        "    Main Zone Inlet Node,    !- Air Outlet Node Name",
        "    Main Zone ATU In Node,   !- Air Inlet Node Name",
        "    8.5,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.05;                    !- Constant Minimum Air Flow Fraction",

        "  ZoneHVAC:Baseboard:Convective:Electric,",
        "    Main Zone Baseboard,     !- Name",
        "    System Availability Schedule,  !- Availability Schedule Name",
        "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "    8000,                    !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    0.97;                    !- Efficiency",

        "  ElectricEquipment:ITE:AirCooled,",
        "    Data Center Servers,     !- Name",
        "    Main Zone,               !- Zone Name",
        "    FlowFromSystem,          !- Air Flow Calculation Method",
        "    Watts/Unit,              !- Design Power Input Calculation Method",
        "    500,                     !- Watts per Unit {W}",
        "    100,                     !- Number of Units",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    Data Center Operation Schedule,  !- Design Power Input Schedule Name",
        "    Data Center CPU Loading Schedule,  !- CPU Loading  Schedule Name",
        "    Data Center Servers Power fLoadTemp,  !- CPU Power Input Function of Loading and Air Temperature Curve Name",
        "    0.4,                     !- Design Fan Power Input Fraction",
        "    0.0001,                  !- Design Fan Air Flow Rate per Power Input {m3/s-W}",
        "    Data Center Servers Airflow fLoadTemp,  !- Air Flow Function of Loading and Air Temperature Curve Name",
        "    ECM FanPower fFlow,      !- Fan Power Input Function of Flow Curve Name",
        "    15,                      !- Design Entering Air Temperature {C}",
        "    A3,                      !- Environmental Class",
        "    AdjustedSupply,          !- Air Inlet Connection Type",
        "    ,                        !- Air Inlet Room Air Model Node Name",
        "    ,                        !- Air Outlet Room Air Model Node Name",
        "    Inlet Node Not Found,    !- Supply Air Node Name",
        "    0.1,                     !- Design Recirculation Fraction",
        "    Data Center Recirculation fLoadTemp,  !- Recirculation Function of Loading and Supply Temperature Curve Name",
        "    0.9,                     !- Design Electric Power Supply Efficiency",
        "    UPS Efficiency fPLR,     !- Electric Power Supply Efficiency Function of Part Load Ratio Curve Name",
        "    1,                       !- Fraction of Electric Power Supply Losses to Zone",
        "    ITE-CPU,                 !- CPU End-Use Subcategory",
        "    ITE-Fans,                !- Fan End-Use Subcategory",
        "    ITE-UPS;                 !- Electric Power Supply End-Use Subcategory",

        "  Curve:Quadratic,",
        "    ECM FanPower fFlow,      !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    99.0;                    !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    UPS Efficiency fPLR,     !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    99.0;                    !- Maximum Value of x",

        "  Curve:Biquadratic,",
        "    Data Center Servers Power fLoadTemp,  !- Name",
        "    -1.0,                    !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.06667,                 !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Data Center Servers Airflow fLoadTemp,  !- Name",
        "    -1.4,                    !- Coefficient1 Constant",
        "    0.9,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.1,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Data Center Recirculation fLoadTemp,  !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Schedule:Constant,Data Center Operation Schedule,Any Number,1.0;",

        "  Schedule:Compact,",
        "    Data Center CPU Loading Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 1/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0,        !- Field 3",
        "    Through: 2/29,           !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    Until: 24:00,0.50,       !- Field 7",
        "    Through: 3/31,           !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    Until: 24:00,0.75,       !- Field 11",
        "    Through: 4/30,           !- Field 13",
        "    For: AllDays,            !- Field 14",
        "    Until: 24:00,1.0,        !- Field 15",
        "    Through: 5/31,           !- Field 17",
        "    For: AllDays,            !- Field 18",
        "    Until: 24:00,0.25,       !- Field 19",
        "    Through: 6/30,           !- Field 21",
        "    For: AllDays,            !- Field 22",
        "    Until: 24:00,0.50,       !- Field 23",
        "    Through: 7/31,           !- Field 25",
        "    For: AllDays,            !- Field 26",
        "    Until: 24:00,0.1,        !- Field 27",
        "    Through: 8/31,           !- Field 29",
        "    For: AllDays,            !- Field 30",
        "    Until: 24:00,1.0,        !- Field 31",
        "    Through: 9/30,           !- Field 33",
        "    For: AllDays,            !- Field 34",
        "    Until: 24:00,0.25,       !- Field 35",
        "    Through: 10/31,          !- Field 37",
        "    For: AllDays,            !- Field 38",
        "    Until: 24:00,0.50,       !- Field 39",
        "    Through: 11/30,          !- Field 41",
        "    For: AllDays,            !- Field 42",
        "    Until: 24:00,0.75,       !- Field 43",
        "    Through: 12/31,          !- Field 45",
        "    For: AllDays,            !- Field 46",
        "    Until: 24:00,1.00;       !- Field 47",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false);
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    ZoneEquipmentManager::GetZoneEquipment(*state);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.008;

    EXPECT_ANY_THROW(InternalHeatGains::GetInternalHeatGainsInput(*state));
}

TEST_F(EnergyPlusFixture, InternalHeatGains_FlowControlWithApproachTemperaturesGoodInletNode)
{
    std::string const idf_objects = delimited_string({
        "  Zone,",
        "    Main Zone,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    Main Zone,               !- Zone Name",
        "    Main Zone Equipment,     !- Zone Conditioning Equipment List Name",
        "    Main Zone Inlet Node,    !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Main Zone Node,          !- Zone Air Node Name",
        "    Main Zone Outlet Node;   !- Zone Return Air Node or NodeList Name",

        "  ZoneHVAC:EquipmentList,",
        "    Main Zone Equipment,     !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    Main Zone ATU,           !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    2,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "    ZoneHVAC:Baseboard:Convective:Electric,  !- Zone Equipment 2 Object Type",
        "    Main Zone Baseboard,     !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    1,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

        "  ZoneHVAC:AirDistributionUnit,",
        "    Main Zone ATU,           !- Name",
        "    Main Zone Inlet Node,    !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    Main Zone VAV Air;       !- Air Terminal Name",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    Main Zone VAV Air,       !- Name",
        "    System Availability Schedule,  !- Availability Schedule Name",
        "    Main Zone Inlet Node,    !- Air Outlet Node Name",
        "    Main Zone ATU In Node,   !- Air Inlet Node Name",
        "    8.5,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.05;                    !- Constant Minimum Air Flow Fraction",

        "  ZoneHVAC:Baseboard:Convective:Electric,",
        "    Main Zone Baseboard,     !- Name",
        "    System Availability Schedule,  !- Availability Schedule Name",
        "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "    8000,                    !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    0.97;                    !- Efficiency",

        "  ElectricEquipment:ITE:AirCooled,",
        "    Data Center Servers,     !- Name",
        "    Main Zone,               !- Zone Name",
        "    FlowControlWithApproachTemperatures,  !- Air Flow Calculation Method",
        "    Watts/Unit,              !- Design Power Input Calculation Method",
        "    500,                     !- Watts per Unit {W}",
        "    100,                     !- Number of Units",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    Data Center Operation Schedule,  !- Design Power Input Schedule Name",
        "    Data Center CPU Loading Schedule,  !- CPU Loading  Schedule Name",
        "    Data Center Servers Power fLoadTemp,  !- CPU Power Input Function of Loading and Air Temperature Curve Name",
        "    0.4,                     !- Design Fan Power Input Fraction",
        "    0.0001,                  !- Design Fan Air Flow Rate per Power Input {m3/s-W}",
        "    Data Center Servers Airflow fLoadTemp,  !- Air Flow Function of Loading and Air Temperature Curve Name",
        "    ECM FanPower fFlow,      !- Fan Power Input Function of Flow Curve Name",
        "    15,                      !- Design Entering Air Temperature {C}",
        "    A3,                      !- Environmental Class",
        "    ,                        !- Air Inlet Connection Type",
        "    ,                        !- Air Inlet Room Air Model Node Name",
        "    ,                        !- Air Outlet Room Air Model Node Name",
        "    Main Zone Inlet Node,    !- Supply Air Node Name",
        "    0.1,                     !- Design Recirculation Fraction",
        "    Data Center Recirculation fLoadTemp,  !- Recirculation Function of Loading and Supply Temperature Curve Name",
        "    0.9,                     !- Design Electric Power Supply Efficiency",
        "    UPS Efficiency fPLR,     !- Electric Power Supply Efficiency Function of Part Load Ratio Curve Name",
        "    1,                       !- Fraction of Electric Power Supply Losses to Zone",
        "    ITE-CPU,                 !- CPU End-Use Subcategory",
        "    ITE-Fans,                !- Fan End-Use Subcategory",
        "    ITE-UPS,                 !- Electric Power Supply End-Use Subcategory",
        "    2,                       !- Supply Temperature Difference {deltaC}",
        "    ,                        !- Supply Temperature Difference Schedule",
        "    -1,                      !- Return Temperature Difference {deltaC}",
        "    ;                        !- Return Temperature Difference Schedule",

        "  Curve:Quadratic,",
        "    ECM FanPower fFlow,      !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    99.0;                    !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    UPS Efficiency fPLR,     !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    99.0;                    !- Maximum Value of x",

        "  Curve:Biquadratic,",
        "    Data Center Servers Power fLoadTemp,  !- Name",
        "    -1.0,                    !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.06667,                 !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Data Center Servers Airflow fLoadTemp,  !- Name",
        "    -1.4,                    !- Coefficient1 Constant",
        "    0.9,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.1,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Data Center Recirculation fLoadTemp,  !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Schedule:Constant,Data Center Operation Schedule,Any Number,1.0;",

        "  Schedule:Compact,",
        "    Data Center CPU Loading Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 1/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0,        !- Field 3",
        "    Through: 2/29,           !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    Until: 24:00,0.50,       !- Field 7",
        "    Through: 3/31,           !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    Until: 24:00,0.75,       !- Field 11",
        "    Through: 4/30,           !- Field 13",
        "    For: AllDays,            !- Field 14",
        "    Until: 24:00,1.0,        !- Field 15",
        "    Through: 5/31,           !- Field 17",
        "    For: AllDays,            !- Field 18",
        "    Until: 24:00,0.25,       !- Field 19",
        "    Through: 6/30,           !- Field 21",
        "    For: AllDays,            !- Field 22",
        "    Until: 24:00,0.50,       !- Field 23",
        "    Through: 7/31,           !- Field 25",
        "    For: AllDays,            !- Field 26",
        "    Until: 24:00,0.1,        !- Field 27",
        "    Through: 8/31,           !- Field 29",
        "    For: AllDays,            !- Field 30",
        "    Until: 24:00,1.0,        !- Field 31",
        "    Through: 9/30,           !- Field 33",
        "    For: AllDays,            !- Field 34",
        "    Until: 24:00,0.25,       !- Field 35",
        "    Through: 10/31,          !- Field 37",
        "    For: AllDays,            !- Field 38",
        "    Until: 24:00,0.50,       !- Field 39",
        "    Through: 11/30,          !- Field 41",
        "    For: AllDays,            !- Field 42",
        "    Until: 24:00,0.75,       !- Field 43",
        "    Through: 12/31,          !- Field 45",
        "    For: AllDays,            !- Field 46",
        "    Until: 24:00,1.00;       !- Field 47",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false);
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.008;

    InternalHeatGains::GetInternalHeatGainsInput(*state);
    ASSERT_FALSE(ErrorsFound);
}

TEST_F(EnergyPlusFixture, InternalHeatGains_FlowControlWithApproachTemperaturesBadInletNode)
{
    std::string const idf_objects = delimited_string({
        "  Zone,",
        "    Main Zone,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    Main Zone,               !- Zone Name",
        "    Main Zone Equipment,     !- Zone Conditioning Equipment List Name",
        "    Main Zone Inlet Node,    !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Main Zone Node,          !- Zone Air Node Name",
        "    Main Zone Outlet Node;   !- Zone Return Air Node or NodeList Name",

        "  ZoneHVAC:EquipmentList,",
        "    Main Zone Equipment,     !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    Main Zone ATU,           !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    2,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "    ZoneHVAC:Baseboard:Convective:Electric,  !- Zone Equipment 2 Object Type",
        "    Main Zone Baseboard,     !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    1,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

        "  ZoneHVAC:AirDistributionUnit,",
        "    Main Zone ATU,           !- Name",
        "    Main Zone Inlet Node,    !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    Main Zone VAV Air;       !- Air Terminal Name",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    Main Zone VAV Air,       !- Name",
        "    System Availability Schedule,  !- Availability Schedule Name",
        "    Main Zone Inlet Node,    !- Air Outlet Node Name",
        "    Main Zone ATU In Node,   !- Air Inlet Node Name",
        "    8.5,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.05;                    !- Constant Minimum Air Flow Fraction",

        "  ZoneHVAC:Baseboard:Convective:Electric,",
        "    Main Zone Baseboard,     !- Name",
        "    System Availability Schedule,  !- Availability Schedule Name",
        "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "    8000,                    !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    0.97;                    !- Efficiency",

        "  ElectricEquipment:ITE:AirCooled,",
        "    Data Center Servers,     !- Name",
        "    Main Zone,               !- Zone Name",
        "    FlowControlWithApproachTemperatures,  !- Air Flow Calculation Method",
        "    Watts/Unit,              !- Design Power Input Calculation Method",
        "    500,                     !- Watts per Unit {W}",
        "    100,                     !- Number of Units",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    Data Center Operation Schedule,  !- Design Power Input Schedule Name",
        "    Data Center CPU Loading Schedule,  !- CPU Loading  Schedule Name",
        "    Data Center Servers Power fLoadTemp,  !- CPU Power Input Function of Loading and Air Temperature Curve Name",
        "    0.4,                     !- Design Fan Power Input Fraction",
        "    0.0001,                  !- Design Fan Air Flow Rate per Power Input {m3/s-W}",
        "    Data Center Servers Airflow fLoadTemp,  !- Air Flow Function of Loading and Air Temperature Curve Name",
        "    ECM FanPower fFlow,      !- Fan Power Input Function of Flow Curve Name",
        "    15,                      !- Design Entering Air Temperature {C}",
        "    A3,                      !- Environmental Class",
        "    RoomAirModel,            !- Air Inlet Connection Type",
        "    ,                        !- Air Inlet Room Air Model Node Name",
        "    ,                        !- Air Outlet Room Air Model Node Name",
        "    Inlet Node Not Found,    !- Supply Air Node Name",
        "    0.1,                     !- Design Recirculation Fraction",
        "    Data Center Recirculation fLoadTemp,  !- Recirculation Function of Loading and Supply Temperature Curve Name",
        "    0.9,                     !- Design Electric Power Supply Efficiency",
        "    UPS Efficiency fPLR,     !- Electric Power Supply Efficiency Function of Part Load Ratio Curve Name",
        "    1,                       !- Fraction of Electric Power Supply Losses to Zone",
        "    ITE-CPU,                 !- CPU End-Use Subcategory",
        "    ITE-Fans,                !- Fan End-Use Subcategory",
        "    ITE-UPS,                 !- Electric Power Supply End-Use Subcategory",
        "    2,                       !- Supply Temperature Difference {deltaC}",
        "    ,                        !- Supply Temperature Difference Schedule",
        "    -1,                      !- Return Temperature Difference {deltaC}",
        "    ;                        !- Return Temperature Difference Schedule",

        "  Curve:Quadratic,",
        "    ECM FanPower fFlow,      !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    99.0;                    !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    UPS Efficiency fPLR,     !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    99.0;                    !- Maximum Value of x",

        "  Curve:Biquadratic,",
        "    Data Center Servers Power fLoadTemp,  !- Name",
        "    -1.0,                    !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.06667,                 !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Data Center Servers Airflow fLoadTemp,  !- Name",
        "    -1.4,                    !- Coefficient1 Constant",
        "    0.9,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.1,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Data Center Recirculation fLoadTemp,  !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Schedule:Constant,Data Center Operation Schedule,Any Number,1.0;",

        "  Schedule:Compact,",
        "    Data Center CPU Loading Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 1/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0,        !- Field 3",
        "    Through: 2/29,           !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    Until: 24:00,0.50,       !- Field 7",
        "    Through: 3/31,           !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    Until: 24:00,0.75,       !- Field 11",
        "    Through: 4/30,           !- Field 13",
        "    For: AllDays,            !- Field 14",
        "    Until: 24:00,1.0,        !- Field 15",
        "    Through: 5/31,           !- Field 17",
        "    For: AllDays,            !- Field 18",
        "    Until: 24:00,0.25,       !- Field 19",
        "    Through: 6/30,           !- Field 21",
        "    For: AllDays,            !- Field 22",
        "    Until: 24:00,0.50,       !- Field 23",
        "    Through: 7/31,           !- Field 25",
        "    For: AllDays,            !- Field 26",
        "    Until: 24:00,0.1,        !- Field 27",
        "    Through: 8/31,           !- Field 29",
        "    For: AllDays,            !- Field 30",
        "    Until: 24:00,1.0,        !- Field 31",
        "    Through: 9/30,           !- Field 33",
        "    For: AllDays,            !- Field 34",
        "    Until: 24:00,0.25,       !- Field 35",
        "    Through: 10/31,          !- Field 37",
        "    For: AllDays,            !- Field 38",
        "    Until: 24:00,0.50,       !- Field 39",
        "    Through: 11/30,          !- Field 41",
        "    For: AllDays,            !- Field 42",
        "    Until: 24:00,0.75,       !- Field 43",
        "    Through: 12/31,          !- Field 45",
        "    For: AllDays,            !- Field 46",
        "    Until: 24:00,1.00;       !- Field 47",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false);
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    ZoneEquipmentManager::GetZoneEquipment(*state);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.008;

    ASSERT_ANY_THROW(InternalHeatGains::GetInternalHeatGainsInput(*state));
}

TEST_F(EnergyPlusFixture, InternalHeatGains_WarnMissingInletNode)
{
    std::string const idf_objects = delimited_string({
        "  Zone,",
        "    Main Zone,               !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    Main Zone,               !- Zone Name",
        "    Main Zone Equipment,     !- Zone Conditioning Equipment List Name",
        "    Main Zone Inlet Node,    !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Main Zone Node,          !- Zone Air Node Name",
        "    Main Zone Outlet Node;   !- Zone Return Air Node or NodeList Name",

        "  ZoneHVAC:EquipmentList,",
        "    Main Zone Equipment,     !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    Main Zone ATU,           !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    2,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "    ZoneHVAC:Baseboard:Convective:Electric,  !- Zone Equipment 2 Object Type",
        "    Main Zone Baseboard,     !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    1,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

        "  ZoneHVAC:AirDistributionUnit,",
        "    Main Zone ATU,           !- Name",
        "    Main Zone Inlet Node,    !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    Main Zone VAV Air;       !- Air Terminal Name",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    Main Zone VAV Air,       !- Name",
        "    System Availability Schedule,  !- Availability Schedule Name",
        "    Main Zone Inlet Node,    !- Air Outlet Node Name",
        "    Main Zone ATU In Node,   !- Air Inlet Node Name",
        "    8.5,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.05;                    !- Constant Minimum Air Flow Fraction",

        "  ZoneHVAC:Baseboard:Convective:Electric,",
        "    Main Zone Baseboard,     !- Name",
        "    System Availability Schedule,  !- Availability Schedule Name",
        "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "    8000,                    !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    0.97;                    !- Efficiency",

        "  ElectricEquipment:ITE:AirCooled,",
        "    Data Center Servers,     !- Name",
        "    Main Zone,               !- Zone Name",
        "    FlowFromSystem,          !- Air Flow Calculation Method",
        "    Watts/Unit,              !- Design Power Input Calculation Method",
        "    500,                     !- Watts per Unit {W}",
        "    100,                     !- Number of Units",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    Data Center Operation Schedule,  !- Design Power Input Schedule Name",
        "    Data Center CPU Loading Schedule,  !- CPU Loading  Schedule Name",
        "    Data Center Servers Power fLoadTemp,  !- CPU Power Input Function of Loading and Air Temperature Curve Name",
        "    0.4,                     !- Design Fan Power Input Fraction",
        "    0.0001,                  !- Design Fan Air Flow Rate per Power Input {m3/s-W}",
        "    Data Center Servers Airflow fLoadTemp,  !- Air Flow Function of Loading and Air Temperature Curve Name",
        "    ECM FanPower fFlow,      !- Fan Power Input Function of Flow Curve Name",
        "    15,                      !- Design Entering Air Temperature {C}",
        "    A3,                      !- Environmental Class",
        "    RoomAirModel,            !- Air Inlet Connection Type",
        "    ,                        !- Air Inlet Room Air Model Node Name",
        "    ,                        !- Air Outlet Room Air Model Node Name",
        "    Inlet Node Not Found,    !- Supply Air Node Name",
        "    0.1,                     !- Design Recirculation Fraction",
        "    Data Center Recirculation fLoadTemp,  !- Recirculation Function of Loading and Supply Temperature Curve Name",
        "    0.9,                     !- Design Electric Power Supply Efficiency",
        "    UPS Efficiency fPLR,     !- Electric Power Supply Efficiency Function of Part Load Ratio Curve Name",
        "    1,                       !- Fraction of Electric Power Supply Losses to Zone",
        "    ITE-CPU,                 !- CPU End-Use Subcategory",
        "    ITE-Fans,                !- Fan End-Use Subcategory",
        "    ITE-UPS,                 !- Electric Power Supply End-Use Subcategory",
        "    2,                       !- Supply Temperature Difference {deltaC}",
        "    ,                        !- Supply Temperature Difference Schedule",
        "    -1,                      !- Return Temperature Difference {deltaC}",
        "    ;                        !- Return Temperature Difference Schedule",

        "  Curve:Quadratic,",
        "    ECM FanPower fFlow,      !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    99.0;                    !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    UPS Efficiency fPLR,     !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    99.0;                    !- Maximum Value of x",

        "  Curve:Biquadratic,",
        "    Data Center Servers Power fLoadTemp,  !- Name",
        "    -1.0,                    !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.06667,                 !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Data Center Servers Airflow fLoadTemp,  !- Name",
        "    -1.4,                    !- Coefficient1 Constant",
        "    0.9,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.1,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Data Center Recirculation fLoadTemp,  !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    99.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    99.0,                    !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Schedule:Constant,Data Center Operation Schedule,Any Number,1.0;",

        "  Schedule:Compact,",
        "    Data Center CPU Loading Schedule,  !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 1/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0,        !- Field 3",
        "    Through: 2/29,           !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    Until: 24:00,0.50,       !- Field 7",
        "    Through: 3/31,           !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    Until: 24:00,0.75,       !- Field 11",
        "    Through: 4/30,           !- Field 13",
        "    For: AllDays,            !- Field 14",
        "    Until: 24:00,1.0,        !- Field 15",
        "    Through: 5/31,           !- Field 17",
        "    For: AllDays,            !- Field 18",
        "    Until: 24:00,0.25,       !- Field 19",
        "    Through: 6/30,           !- Field 21",
        "    For: AllDays,            !- Field 22",
        "    Until: 24:00,0.50,       !- Field 23",
        "    Through: 7/31,           !- Field 25",
        "    For: AllDays,            !- Field 26",
        "    Until: 24:00,0.1,        !- Field 27",
        "    Through: 8/31,           !- Field 29",
        "    For: AllDays,            !- Field 30",
        "    Until: 24:00,1.0,        !- Field 31",
        "    Through: 9/30,           !- Field 33",
        "    For: AllDays,            !- Field 34",
        "    Until: 24:00,0.25,       !- Field 35",
        "    Through: 10/31,          !- Field 37",
        "    For: AllDays,            !- Field 38",
        "    Until: 24:00,0.50,       !- Field 39",
        "    Through: 11/30,          !- Field 41",
        "    For: AllDays,            !- Field 42",
        "    Until: 24:00,0.75,       !- Field 43",
        "    Through: 12/31,          !- Field 45",
        "    For: AllDays,            !- Field 46",
        "    Until: 24:00,1.00;       !- Field 47",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false);
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.008;

    InternalHeatGains::GetInternalHeatGainsInput(*state);
    ASSERT_FALSE(ErrorsFound);
}

TEST_F(EnergyPlusFixture, InternalHeatGains_GetHeatColdStressTemp)
{
    std::string const idf_objects = delimited_string({

        "ScheduleTypeLimits,SchType1,0.0,1.0,Continuous,Dimensionless;",

        "Schedule:Constant,Schedule1,,1.0;",

        "People,",
        "  Main Zone People,        !- Name",
        "  Main Zone,               !- Zone or ZoneList Name",
        "  Schedule1,               !- Number of People Schedule Name",
        "  people,                  !- Number of People Calculation Method",
        "  3.000000,                !- Number of People",
        "  ,                        !- People per Zone Floor Area{ person / m2 }",
        "  ,                        !- Zone Floor Area per Person{ m2 / person }",
        "  0.3000000,               !- Fraction Radiant",
        "  0.5,                     !- Sensible Heat Fraction",
        "  Schedule1,               !- Activity Level Schedule Name",
        "  3.82E-8,                 !- Carbon Dioxide Generation Rate{ m3 / s - W }",
        "  No,                      !- Enable ASHRAE 55 Comfort Warnings",
        "  EnclosureAveraged,            !- Mean Radiant Temperature Calculation Type",
        "  ,                        !- Surface Name/Angle Factor List Name",
        "  ,                        !- Work Efficiency Schedule Name",
        "  ,                        !- Clothing Insulation Calculation Method",
        "  ,                        !- Clothing Insulation Calculation Method Schedule Name",
        "  ,                        !- Clothing Insulation Schedule Name",
        "  ,                        !- Air Velocity Schedule Name",
        "  ,                        !- Thermal Comfort Model 1 Type",
        "  ,                        !- Thermal Comfort Model 2 Type",
        "  ,                        !- Thermal Comfort Model 3 Type",
        "  ,                        !- Thermal Comfort Model 4 Type",
        "  ,                        !- Thermal Comfort Model 5 Type",
        "  ,                        !- Thermal Comfort Model 6 Type",
        "  ,                        !- Thermal Comfort Model 7 Type",
        "  ,                        !- Ankle Level Air Velocity Schedule Name",
        "  11.5,                    !- Cold Stress Temperature Threshold [C]",
        "  30.5;                    !- Heat Stress Temperature Threshold [C]",

        "Zone,",
        "  Main Zone,               !- Name",
        "  0,                       !- Direction of Relative North {deg}",
        "  0,                       !- X Origin {m}",
        "  0,                       !- Y Origin {m}",
        "  0,                       !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  autocalculate,           !- Ceiling Height {m}",
        "  autocalculate;           !- Volume {m3}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    bool ErrorsFound(false);
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    // cold and heat threshold is properly read
    InternalHeatGains::GetInternalHeatGainsInput(*state);
    EXPECT_EQ(state->dataHeatBal->People(1).ColdStressTempThresh, 11.5);
    EXPECT_EQ(state->dataHeatBal->People(1).HeatStressTempThresh, 30.5);
}

TEST_F(EnergyPlusFixture, ITEwithUncontrolledZoneTest)
{
    using namespace DataHeatBalance;

    std::string const idf_objects = delimited_string({
        " Zone,",
        "  ZONE ONE,                !- Name",
        "  0,                       !- Direction of Relative North {deg}",
        "  0,                       !- X Origin {m}",
        "  0,                       !- Y Origin {m}",
        "  0,                       !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  autocalculate,           !- Ceiling Height {m}",
        "  autocalculate;           !- Volume {m3}",

        " ElectricEquipment:ITE:AirCooled,",
        "  Data Center Servers,     !- Name",
        "  ZONE ONE,                !- Zone Name",
        "  FlowFromSystem,          !- Air Flow Calculation Method",
        "  Watts/Unit,              !- Design Power Input Calculation Method",
        "  50,                      !- Watts per Unit {W}",
        "  10,                      !- Number of Units",
        "  ,                        !- Watts per Zone Floor Area {W/m2}",
        "  ,                        !- Design Power Input Schedule Name",
        "  ,                        !- CPU Loading  Schedule Name",
        "  Data Center Servers Power fLoadTemp,  !- CPU Power Input Function of Loading and Air Temperature Curve Name",
        "  0.4,                     !- Design Fan Power Input Fraction",
        "  0.0001,                  !- Design Fan Air Flow Rate per Power Input {m3/s-W}",
        "  Data Center Servers Airflow fLoadTemp,  !- Air Flow Function of Loading and Air Temperature Curve Name",
        "  ECM FanPower fFlow,      !- Fan Power Input Function of Flow Curve Name",
        "  15,                      !- Design Entering Air Temperature {C}",
        "  A3,                      !- Environmental Class",
        "  ZoneAirNode,             !- Air Inlet Connection Type",
        "  ,                        !- Air Inlet Room Air Model Node Name",
        "  ,                        !- Air Outlet Room Air Model Node Name",
        "  ,                        !- Supply Air Node Name",
        "  0.1,                     !- Design Recirculation Fraction",
        "  ,                        !- Recirculation Function of Loading and Supply Temperature Curve Name",
        "  0.9,                     !- Design Electric Power Supply Efficiency",
        "  ,                        !- Electric Power Supply Efficiency Function of Part Load Ratio Curve Name",
        "  1,                       !- Fraction of Electric Power Supply Losses to Zone",
        "  ITE-CPU,                 !- CPU End-Use Subcategory",
        "  ITE-Fans,                !- Fan End-Use Subcategory",
        "  ITE-UPS,                 !- Electric Power Supply End-Use Subcategory",
        "  2,                       !- Supply Temperature Difference {deltaC}",
        "  ,                        !- Supply Temperature Difference Schedule",
        "  -1,                      !- Return Temperature Difference {deltaC}",
        "  ;                        !- Return Temperature Difference Schedule",

        " Curve:Biquadratic,",
        "  Data Center Servers Power fLoadTemp,  !- Name",
        "  -1.0,                    !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.06667,                 !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "  Data Center Servers Airflow fLoadTemp,  !- Name",
        "  -1.4,                    !- Coefficient1 Constant",
        "  0.9,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.1,                     !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        " Curve:Quadratic,",
        "  ECM FanPower fFlow,      !- Name",
        "  0.0,                     !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Minimum Value of x",
        "  99.0;                    !- Maximum Value of x",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false);
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.008;

    InternalHeatGains::GetInternalHeatGainsInput(*state);
    ASSERT_FALSE(ErrorsFound);

    state->dataEnvrn->StdBaroPress = 101400.0;

    auto &thisZoneITEq = state->dataHeatBal->ZoneITEq(1);

    InternalHeatGains::CalcZoneITEq(*state);
    Real64 calculatedResult1 = thisZoneITEq.PowerRpt[(int)PERptVars::CPU];
    Real64 calculatedResult2 = thisZoneITEq.PowerRpt[(int)PERptVars::Fan];
    Real64 calculatedResult3 = thisZoneITEq.PowerRpt[(int)PERptVars::UPS];
    Real64 calculatedResult4 = thisZoneITEq.PowerRpt[(int)PERptVars::UPSGainToZone];

    Real64 expectedResult1 = 480.024;
    Real64 expectedResult2 = 380.0;
    Real64 expectedResult3 = 86.0024;
    Real64 expectedResult4 = 86.0024;
    Real64 tol = 0.001;
    EXPECT_NEAR(calculatedResult1, expectedResult1, tol);
    EXPECT_NEAR(calculatedResult2, expectedResult2, tol);
    EXPECT_NEAR(calculatedResult3, expectedResult3, tol);
    EXPECT_NEAR(calculatedResult4, expectedResult4, tol);
}

TEST_F(EnergyPlusFixture, ITE_Env_Class_Fix_41C)
{
    // Test PR 9541 for Issue 9538
    std::string const idf_objects = delimited_string({
        " Zone,",
        "  ZONE ONE,                !- Name",
        "  0,                       !- Direction of Relative North {deg}",
        "  0,                       !- X Origin {m}",
        "  0,                       !- Y Origin {m}",
        "  0,                       !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  autocalculate,           !- Ceiling Height {m}",
        "  autocalculate;           !- Volume {m3}",

        " ElectricEquipment:ITE:AirCooled,",
        "  Data Center Servers,     !- Name",
        "  ZONE ONE,                !- Zone Name",
        "  FlowFromSystem,          !- Air Flow Calculation Method",
        "  Watts/Unit,              !- Design Power Input Calculation Method",
        "  50,                      !- Watts per Unit {W}",
        "  10,                      !- Number of Units",
        "  ,                        !- Watts per Zone Floor Area {W/m2}",
        "  ,                        !- Design Power Input Schedule Name",
        "  ,                        !- CPU Loading  Schedule Name",
        "  Data Center Servers Power fLoadTemp,  !- CPU Power Input Function of Loading and Air Temperature Curve Name",
        "  0.4,                     !- Design Fan Power Input Fraction",
        "  0.0001,                  !- Design Fan Air Flow Rate per Power Input {m3/s-W}",
        "  Data Center Servers Airflow fLoadTemp,  !- Air Flow Function of Loading and Air Temperature Curve Name",
        "  ECM FanPower fFlow,      !- Fan Power Input Function of Flow Curve Name",
        "  15,                      !- Design Entering Air Temperature {C}",
        "  A3,                      !- Environmental Class",
        "  ZoneAirNode,             !- Air Inlet Connection Type",
        "  ,                        !- Air Inlet Room Air Model Node Name",
        "  ,                        !- Air Outlet Room Air Model Node Name",
        "  ,                        !- Supply Air Node Name",
        "  0.1,                     !- Design Recirculation Fraction",
        "  ,                        !- Recirculation Function of Loading and Supply Temperature Curve Name",
        "  0.9,                     !- Design Electric Power Supply Efficiency",
        "  ,                        !- Electric Power Supply Efficiency Function of Part Load Ratio Curve Name",
        "  1,                       !- Fraction of Electric Power Supply Losses to Zone",
        "  ITE-CPU,                 !- CPU End-Use Subcategory",
        "  ITE-Fans,                !- Fan End-Use Subcategory",
        "  ITE-UPS,                 !- Electric Power Supply End-Use Subcategory",
        "  2,                       !- Supply Temperature Difference {deltaC}",
        "  ,                        !- Supply Temperature Difference Schedule",
        "  -1,                      !- Return Temperature Difference {deltaC}",
        "  ;                        !- Return Temperature Difference Schedule",

        " Curve:Biquadratic,",
        "  Data Center Servers Power fLoadTemp,  !- Name",
        "  -1.0,                    !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.06667,                 !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "  Data Center Servers Airflow fLoadTemp,  !- Name",
        "  -1.4,                    !- Coefficient1 Constant",
        "  0.9,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.1,                     !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        " Curve:Quadratic,",
        "  ECM FanPower fFlow,      !- Name",
        "  0.0,                     !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Minimum Value of x",
        "  99.0;                    !- Maximum Value of x",
    });

    static constexpr std::array<Real64, 7> DBMin = {-99.0, 15.0, 10.0, 5.0, 5.0, 5.0, 5.0};           // Minimum dry-bulb temperature [C]
    static constexpr std::array<Real64, 7> DBMax = {99.0, 32.0, 35.0, 40.0, 45.0, 35.0, 40.0};        // Maximum dry-bulb temperature [C]
    static constexpr std::array<Real64, 7> DPMin = {-99.0, -99.0, -99.0, -12.0, -12.0, -99.0, -99.0}; // Minimum dewpoint temperature [C]
    static constexpr std::array<Real64, 7> DPMax = {99.0, 17.0, 21.0, 24.0, 24.0, 28.0, 28.0};        // Maximum dewpoint temperature [C]
    static constexpr std::array<Real64, 7> RHMin = {0.0, 20.0, 20.0, 8.0, 8.0, 8.0, 8.0};             // Minimum relative humidity [%]
    static constexpr std::array<Real64, 7> RHMax = {99.0, 80.0, 80.0, 85.0, 90.0, 80.0, 80.0};        // Maximum relative humidity [%]

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false);
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataGlobal->TimeStepZone = 1.0;

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    // Test 1: 41C;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 41.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.015;

    InternalHeatGains::GetInternalHeatGainsInput(*state);
    ASSERT_FALSE(ErrorsFound);

    state->dataEnvrn->StdBaroPress = 101400.0;

    InternalHeatGains::CalcZoneITEq(*state);

    int Loop = 1;
    int NZ = 1;
    int spaceNum = 1;
    int EnvClass = 3;
    Real64 TAirIn = state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT;
    Real64 TDPAirIn = 20;
    Real64 RHAirIn = 40;

    TDPAirIn = 20.335339775634917;
    RHAirIn = 30.667066060140435;

    EXPECT_NEAR(state->dataGlobal->TimeStepZone, 1.0, 1e-6);

    // Test 1: 41C
    // if (TAirIn > DBMax[EnvClass]): A3 upper bounds hit
    EXPECT_EQ(state->dataHeatBal->ZoneITEq(Loop).TimeAboveDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NEAR(state->dataHeatBal->ZoneITEq(Loop).DryBulbTAboveDeltaT, TAirIn - DBMax[EnvClass], 1e-6);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeAboveDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    EXPECT_EQ(state->dataHeatBal->ZoneITEq(Loop).TimeAboveDryBulbT, 1.0);
    EXPECT_EQ(state->dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange, 1.0);
    EXPECT_NEAR(state->dataHeatBal->ZoneITEq(Loop).DryBulbTAboveDeltaT, TAirIn - DBMax[EnvClass], 1e-6);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeAboveDryBulbT, 1.0);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange, 1.0);
    EXPECT_EQ(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveDryBulbT, 1.0);
    EXPECT_EQ(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange, 1.0);

    // if (TAirIn < DBMin[EnvClass])
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeBelowDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).DryBulbTBelowDeltaT, TAirIn - DBMin[EnvClass]);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeBelowDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // if (TDPAirIn > DPMax[EnvClass])
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeAboveDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).DewpointTAboveDeltaT, TDPAirIn - DPMax[EnvClass]);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeAboveDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // if (TDPAirIn < DPMin[EnvClass])
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeBelowDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).DewpointTBelowDeltaT, TDPAirIn - DPMin[EnvClass]);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeBelowDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // if (RHAirIn > RHMax[EnvClass])
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeAboveRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).RHAboveDeltaRH, RHAirIn - RHMax[EnvClass]);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeAboveRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // if (RHAirIn < RHMin[EnvClass])
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeBelowRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).RHBelowDeltaRH, RHAirIn - RHMin[EnvClass]);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeBelowRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
}

TEST_F(EnergyPlusFixture, ITE_Env_Class_Fix_39C)
{
    // Test PR 9541 for Issue 9538
    std::string const idf_objects = delimited_string({
        " Zone,",
        "  ZONE ONE,                !- Name",
        "  0,                       !- Direction of Relative North {deg}",
        "  0,                       !- X Origin {m}",
        "  0,                       !- Y Origin {m}",
        "  0,                       !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  autocalculate,           !- Ceiling Height {m}",
        "  autocalculate;           !- Volume {m3}",

        " ElectricEquipment:ITE:AirCooled,",
        "  Data Center Servers,     !- Name",
        "  ZONE ONE,                !- Zone Name",
        "  FlowFromSystem,          !- Air Flow Calculation Method",
        "  Watts/Unit,              !- Design Power Input Calculation Method",
        "  50,                      !- Watts per Unit {W}",
        "  10,                      !- Number of Units",
        "  ,                        !- Watts per Zone Floor Area {W/m2}",
        "  ,                        !- Design Power Input Schedule Name",
        "  ,                        !- CPU Loading  Schedule Name",
        "  Data Center Servers Power fLoadTemp,  !- CPU Power Input Function of Loading and Air Temperature Curve Name",
        "  0.4,                     !- Design Fan Power Input Fraction",
        "  0.0001,                  !- Design Fan Air Flow Rate per Power Input {m3/s-W}",
        "  Data Center Servers Airflow fLoadTemp,  !- Air Flow Function of Loading and Air Temperature Curve Name",
        "  ECM FanPower fFlow,      !- Fan Power Input Function of Flow Curve Name",
        "  15,                      !- Design Entering Air Temperature {C}",
        "  A3,                      !- Environmental Class",
        "  ZoneAirNode,             !- Air Inlet Connection Type",
        "  ,                        !- Air Inlet Room Air Model Node Name",
        "  ,                        !- Air Outlet Room Air Model Node Name",
        "  ,                        !- Supply Air Node Name",
        "  0.1,                     !- Design Recirculation Fraction",
        "  ,                        !- Recirculation Function of Loading and Supply Temperature Curve Name",
        "  0.9,                     !- Design Electric Power Supply Efficiency",
        "  ,                        !- Electric Power Supply Efficiency Function of Part Load Ratio Curve Name",
        "  1,                       !- Fraction of Electric Power Supply Losses to Zone",
        "  ITE-CPU,                 !- CPU End-Use Subcategory",
        "  ITE-Fans,                !- Fan End-Use Subcategory",
        "  ITE-UPS,                 !- Electric Power Supply End-Use Subcategory",
        "  2,                       !- Supply Temperature Difference {deltaC}",
        "  ,                        !- Supply Temperature Difference Schedule",
        "  -1,                      !- Return Temperature Difference {deltaC}",
        "  ;                        !- Return Temperature Difference Schedule",

        " Curve:Biquadratic,",
        "  Data Center Servers Power fLoadTemp,  !- Name",
        "  -1.0,                    !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.06667,                 !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "  Data Center Servers Airflow fLoadTemp,  !- Name",
        "  -1.4,                    !- Coefficient1 Constant",
        "  0.9,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.1,                     !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        " Curve:Quadratic,",
        "  ECM FanPower fFlow,      !- Name",
        "  0.0,                     !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Minimum Value of x",
        "  99.0;                    !- Maximum Value of x",
    });

    static constexpr std::array<Real64, 7> DBMin = {-99.0, 15.0, 10.0, 5.0, 5.0, 5.0, 5.0};           // Minimum dry-bulb temperature [C]
    static constexpr std::array<Real64, 7> DBMax = {99.0, 32.0, 35.0, 40.0, 45.0, 35.0, 40.0};        // Maximum dry-bulb temperature [C]
    static constexpr std::array<Real64, 7> DPMin = {-99.0, -99.0, -99.0, -12.0, -12.0, -99.0, -99.0}; // Minimum dewpoint temperature [C]
    static constexpr std::array<Real64, 7> DPMax = {99.0, 17.0, 21.0, 24.0, 24.0, 28.0, 28.0};        // Maximum dewpoint temperature [C]
    static constexpr std::array<Real64, 7> RHMin = {0.0, 20.0, 20.0, 8.0, 8.0, 8.0, 8.0};             // Minimum relative humidity [%]
    static constexpr std::array<Real64, 7> RHMax = {99.0, 80.0, 80.0, 85.0, 90.0, 80.0, 80.0};        // Maximum relative humidity [%]

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false);
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataGlobal->TimeStepZone = 1.0;

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    // Test 2: 39C;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 39.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.015;

    InternalHeatGains::GetInternalHeatGainsInput(*state);
    ASSERT_FALSE(ErrorsFound);

    state->dataEnvrn->StdBaroPress = 101400.0;

    InternalHeatGains::CalcZoneITEq(*state);

    int Loop = 1;
    int NZ = 1;
    int spaceNum = 1;
    int EnvClass = 3;
    Real64 TAirIn = state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT;
    Real64 TDPAirIn = 20;
    Real64 RHAirIn = 40;

    TDPAirIn = 20.335339775634917;
    RHAirIn = 34.117980814511832;

    EXPECT_NEAR(state->dataGlobal->TimeStepZone, 1.0, 1e-6);

    // Test 2: The following test should pass in with the fix (PR9541);
    // Witout the fix, some of the following items would fail if tested in the original develop branch
    // if (TAirIn > DBMax[EnvClass])
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeAboveDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).DryBulbTAboveDeltaT, TAirIn - DBMax[EnvClass]);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeAboveDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    EXPECT_EQ(state->dataHeatBal->ZoneITEq(Loop).TimeAboveDryBulbT, 0.0);
    EXPECT_EQ(state->dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange, 0.0);
    EXPECT_NEAR(state->dataHeatBal->ZoneITEq(Loop).DryBulbTAboveDeltaT, 0.0, 1e-6);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeAboveDryBulbT, 0.0);
    EXPECT_EQ(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange, 0.0);
    EXPECT_EQ(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveDryBulbT, 0.0);
    EXPECT_EQ(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange, 0.0);

    // if (TAirIn < DBMin[EnvClass])
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeBelowDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).DryBulbTBelowDeltaT, TAirIn - DBMin[EnvClass]);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeBelowDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // if (TDPAirIn > DPMax[EnvClass])
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeAboveDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).DewpointTAboveDeltaT, TDPAirIn - DPMax[EnvClass]);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeAboveDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // if (TDPAirIn < DPMin[EnvClass])
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeBelowDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).DewpointTBelowDeltaT, TDPAirIn - DPMin[EnvClass]);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeBelowDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // if (RHAirIn > RHMax[EnvClass])
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeAboveRH, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).RHAboveDeltaRH, RHAirIn - RHMax[EnvClass]);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeAboveRH, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveRH, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // if (RHAirIn < RHMin[EnvClass])
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeBelowRH, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneITEq(Loop).RHBelowDeltaRH, RHAirIn - RHMin[EnvClass]);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeBelowRH, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowRH, state->dataGlobal->TimeStepZone);
    EXPECT_NE(state->dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
}

TEST_F(EnergyPlusFixture, ITE_Env_Class_Update_Class_H1)
{
    // Test PR 9537 for Issue 9418
    std::string const idf_objects = delimited_string({
        " Zone,",
        "  ZONE ONE,                !- Name",
        "  0,                       !- Direction of Relative North {deg}",
        "  0,                       !- X Origin {m}",
        "  0,                       !- Y Origin {m}",
        "  0,                       !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  autocalculate,           !- Ceiling Height {m}",
        "  autocalculate;           !- Volume {m3}",

        " ElectricEquipment:ITE:AirCooled,",
        "  Data Center Servers,     !- Name",
        "  ZONE ONE,                !- Zone Name",
        "  FlowFromSystem,          !- Air Flow Calculation Method",
        "  Watts/Unit,              !- Design Power Input Calculation Method",
        "  50,                      !- Watts per Unit {W}",
        "  10,                      !- Number of Units",
        "  ,                        !- Watts per Zone Floor Area {W/m2}",
        "  ,                        !- Design Power Input Schedule Name",
        "  ,                        !- CPU Loading  Schedule Name",
        "  Data Center Servers Power fLoadTemp,  !- CPU Power Input Function of Loading and Air Temperature Curve Name",
        "  0.4,                     !- Design Fan Power Input Fraction",
        "  0.0001,                  !- Design Fan Air Flow Rate per Power Input {m3/s-W}",
        "  Data Center Servers Airflow fLoadTemp,  !- Air Flow Function of Loading and Air Temperature Curve Name",
        "  ECM FanPower fFlow,      !- Fan Power Input Function of Flow Curve Name",
        "  15,                      !- Design Entering Air Temperature {C}",
        "  H1,                      !- Environmental Class",
        "  ZoneAirNode,             !- Air Inlet Connection Type",
        "  ,                        !- Air Inlet Room Air Model Node Name",
        "  ,                        !- Air Outlet Room Air Model Node Name",
        "  ,                        !- Supply Air Node Name",
        "  0.1,                     !- Design Recirculation Fraction",
        "  ,                        !- Recirculation Function of Loading and Supply Temperature Curve Name",
        "  0.9,                     !- Design Electric Power Supply Efficiency",
        "  ,                        !- Electric Power Supply Efficiency Function of Part Load Ratio Curve Name",
        "  1,                       !- Fraction of Electric Power Supply Losses to Zone",
        "  ITE-CPU,                 !- CPU End-Use Subcategory",
        "  ITE-Fans,                !- Fan End-Use Subcategory",
        "  ITE-UPS,                 !- Electric Power Supply End-Use Subcategory",
        "  2,                       !- Supply Temperature Difference {deltaC}",
        "  ,                        !- Supply Temperature Difference Schedule",
        "  -1,                      !- Return Temperature Difference {deltaC}",
        "  ;                        !- Return Temperature Difference Schedule",

        " Curve:Biquadratic,",
        "  Data Center Servers Power fLoadTemp,  !- Name",
        "  -1.0,                    !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.06667,                 !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "  Data Center Servers Airflow fLoadTemp,  !- Name",
        "  -1.4,                    !- Coefficient1 Constant",
        "  0.9,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.1,                     !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  -10,                     !- Minimum Value of y",
        "  99.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  99.0,                    !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        " Curve:Quadratic,",
        "  ECM FanPower fFlow,      !- Name",
        "  0.0,                     !- Coefficient1 Constant",
        "  1.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Minimum Value of x",
        "  99.0;                    !- Maximum Value of x",
    });

    static constexpr std::array<Real64, static_cast<int>(DataHeatBalance::ITEClass::Num)> DBMin = {
        -99.0, 15.0, 10.0, 5.0, 5.0, 5.0, 5.0, 5.0}; // Minimum dry-bulb temperature [C]
    static constexpr std::array<Real64, static_cast<int>(DataHeatBalance::ITEClass::Num)> DBMax = {
        99.0, 32.0, 35.0, 40.0, 45.0, 35.0, 40.0, 25.0}; // Maximum dry-bulb temperature [C]
    static constexpr std::array<Real64, static_cast<int>(DataHeatBalance::ITEClass::Num)> DPMin = {
        -99.0, -12.0, -12.0, -12.0, -12.0, -99.0, -99.0, -12.0}; // Minimum dewpoint temperature [C]
    static constexpr std::array<Real64, static_cast<int>(DataHeatBalance::ITEClass::Num)> DPMax = {
        99.0, 17.0, 21.0, 24.0, 24.0, 28.0, 28.0, 17.0}; // Maximum dewpoint temperature [C]
    static constexpr std::array<Real64, static_cast<int>(DataHeatBalance::ITEClass::Num)> RHMin = {
        0.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0}; // Minimum relative humidity [%]
    static constexpr std::array<Real64, static_cast<int>(DataHeatBalance::ITEClass::Num)> RHMax = {
        99.0, 80.0, 80.0, 85.0, 90.0, 80.0, 80.0, 80.0}; // Maximum relative humidity [%]

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false);
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataGlobal->TimeStepZone = 1.0;

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    state->dataEnvrn->StdBaroPress = 101325.0;

    // Test: 41C
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 41.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.015;

    InternalHeatGains::GetInternalHeatGainsInput(*state);
    ASSERT_FALSE(ErrorsFound);

    int Loop = 1;
    auto &thisZoneITEq = state->dataHeatBal->ZoneITEq(Loop);
    // Test the processing results of the Environmental Class H1 input
    EXPECT_TRUE(thisZoneITEq.Class == DataHeatBalance::ITEClass::H1);

    state->dataEnvrn->StdRhoAir = 0.8;
    InternalHeatGains::CalcZoneITEq(*state);
    // add a test to verify the standard density air volume flow rate calculation
    EXPECT_NEAR(state->dataHeatBal->ZoneITEq(1).AirVolFlowStdDensity, 0.2469055, 1e-6);

    int NZ = 1;
    int spaceNum = 1;
    int EnvClass = static_cast<int>(thisZoneITEq.Class); // DataHeatBalance::ITEClass::H1, or 7
    Real64 TAirIn = state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT;
    Real64 TDPAirIn = 20;
    Real64 RHAirIn = 40;

    TDPAirIn = 20.323364421767739;
    RHAirIn = 30.644383318971691;

    EXPECT_NEAR(state->dataGlobal->TimeStepZone, 1.0, 1e-6);

    auto &thisZnRpt = state->dataHeatBal->ZoneRpt(NZ);
    auto &thisspaceRpt = state->dataHeatBal->spaceRpt(spaceNum);
    // Test: The following test should pass
    // if (TAirIn > DBMax[EnvClass])
    EXPECT_EQ(thisZoneITEq.TimeAboveDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_GT(thisZoneITEq.DryBulbTAboveDeltaT, 0.0);
    EXPECT_EQ(thisZnRpt.ITEqTimeAboveDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeAboveDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    EXPECT_EQ(thisZoneITEq.TimeAboveDryBulbT, 1.0);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, 1.0);
    EXPECT_EQ(thisZoneITEq.DryBulbTAboveDeltaT, TAirIn - DBMax[EnvClass]);
    EXPECT_EQ(thisZnRpt.ITEqTimeAboveDryBulbT, 1.0);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, 1.0);
    EXPECT_EQ(thisspaceRpt.ITEqTimeAboveDryBulbT, 1.0);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, 1.0);

    // if (TAirIn < DBMin[EnvClass])
    EXPECT_NE(thisZoneITEq.TimeBelowDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisZoneITEq.DryBulbTBelowDeltaT, TAirIn - DBMin[EnvClass]);
    EXPECT_NE(thisZnRpt.ITEqTimeBelowDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisspaceRpt.ITEqTimeBelowDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // This block should be activated and set to correct tests when PR9541 is merged
    // if (TDPAirIn > DPMax[EnvClass])
    EXPECT_EQ(thisZoneITEq.TimeAboveDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NEAR(thisZoneITEq.DewpointTAboveDeltaT, TDPAirIn - DPMax[EnvClass], 0.000000000001);
    EXPECT_EQ(thisZnRpt.ITEqTimeAboveDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeAboveDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // if (TDPAirIn < DPMin[EnvClass])
    EXPECT_NE(thisZoneITEq.TimeBelowDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisZoneITEq.DewpointTBelowDeltaT, TDPAirIn - DPMin[EnvClass]);
    EXPECT_NE(thisZnRpt.ITEqTimeBelowDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisspaceRpt.ITEqTimeBelowDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // if (RHAirIn > RHMax[EnvClass])
    EXPECT_NE(thisZoneITEq.TimeAboveRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisZoneITEq.RHAboveDeltaRH, RHAirIn - RHMax[EnvClass]);
    EXPECT_NE(thisZnRpt.ITEqTimeAboveRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisspaceRpt.ITEqTimeAboveRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // if (RHAirIn < RHMin[EnvClass])
    EXPECT_NE(thisZoneITEq.TimeBelowRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisZoneITEq.RHBelowDeltaRH, RHAirIn - RHMin[EnvClass]);
    EXPECT_NE(thisZnRpt.ITEqTimeBelowRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisspaceRpt.ITEqTimeBelowRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // Now Test 33C (after PR9541/Issue9538 is merged/fixed)
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 33.0;
    // state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZoneAirHumRat = 0.015;

    InternalHeatGains::GetInternalHeatGainsInput(*state);
    ASSERT_FALSE(ErrorsFound);

    EXPECT_TRUE(thisZoneITEq.Class == DataHeatBalance::ITEClass::H1);

    InternalHeatGains::CalcZoneITEq(*state);

    TAirIn = state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT;
    TDPAirIn = 20.323364421767739;
    RHAirIn = 47.395745113895885;

    EXPECT_NEAR(state->dataGlobal->TimeStepZone, 1.0, 1e-6);

    // Test: The following test should pass
    // if (TAirIn > DBMax[EnvClass])
    EXPECT_EQ(thisZoneITEq.TimeAboveDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_GT(thisZoneITEq.DryBulbTAboveDeltaT, 0.0);
    EXPECT_EQ(thisZnRpt.ITEqTimeAboveDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeAboveDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    EXPECT_EQ(thisZoneITEq.TimeAboveDryBulbT, 1.0);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, 1.0);
    EXPECT_EQ(thisZoneITEq.DryBulbTAboveDeltaT, TAirIn - DBMax[EnvClass]);
    EXPECT_EQ(thisZnRpt.ITEqTimeAboveDryBulbT, 1.0);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, 1.0);
    EXPECT_EQ(thisspaceRpt.ITEqTimeAboveDryBulbT, 1.0);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, 1.0);

    // if (TAirIn < DBMin[EnvClass])
    EXPECT_NE(thisZoneITEq.TimeBelowDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisZoneITEq.DryBulbTBelowDeltaT, TAirIn - DBMin[EnvClass]);
    EXPECT_NE(thisZnRpt.ITEqTimeBelowDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisspaceRpt.ITEqTimeBelowDryBulbT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // This block should be activated and set to correct tests when PR9541 is merged
    // if (TDPAirIn > DPMax[EnvClass])
    EXPECT_EQ(thisZoneITEq.TimeAboveDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NEAR(thisZoneITEq.DewpointTAboveDeltaT, TDPAirIn - DPMax[EnvClass], 0.000000000001);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeAboveDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    EXPECT_EQ(thisZoneITEq.TimeAboveDewpointT, 1.0);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, 1.0);
    EXPECT_NEAR(thisZoneITEq.DewpointTAboveDeltaT, TDPAirIn - DPMax[EnvClass], 0.000000000001);
    EXPECT_EQ(thisZnRpt.ITEqTimeAboveDewpointT, 1.0);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, 1.0);
    EXPECT_EQ(thisspaceRpt.ITEqTimeAboveDewpointT, 1.0);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, 1.0);

    // if (TDPAirIn < DPMin[EnvClass])
    EXPECT_NE(thisZoneITEq.TimeBelowDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisZoneITEq.DewpointTBelowDeltaT, TDPAirIn - DPMin[EnvClass]);
    EXPECT_NE(thisZnRpt.ITEqTimeBelowDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisspaceRpt.ITEqTimeBelowDewpointT, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // if (RHAirIn > RHMax[EnvClass])
    EXPECT_NE(thisZoneITEq.TimeAboveRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisZoneITEq.RHAboveDeltaRH, RHAirIn - RHMax[EnvClass]);
    EXPECT_NE(thisZnRpt.ITEqTimeAboveRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisspaceRpt.ITEqTimeAboveRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);

    // if (RHAirIn < RHMin[EnvClass])
    EXPECT_NE(thisZoneITEq.TimeBelowRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZoneITEq.TimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisZoneITEq.RHBelowDeltaRH, RHAirIn - RHMin[EnvClass]);
    EXPECT_NE(thisZnRpt.ITEqTimeBelowRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisZnRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
    EXPECT_NE(thisspaceRpt.ITEqTimeBelowRH, state->dataGlobal->TimeStepZone);
    EXPECT_EQ(thisspaceRpt.ITEqTimeOutOfOperRange, state->dataGlobal->TimeStepZone);
}
TEST_F(EnergyPlusFixture, InternalHeatGains_SpaceAllocation)
{

    std::string const idf_objects = delimited_string({
        " Zone,",
        "  Zone 1,                  !- Name",
        "  0,                       !- Direction of Relative North {deg}",
        "  0,                       !- X Origin {m}",
        "  0,                       !- Y Origin {m}",
        "  0,                       !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  autocalculate,           !- Ceiling Height {m}",
        "  autocalculate,           !- Volume {m3}",
        "  20.0;           !- Area {m2}",

        "Space,",
        "Space 1A,            !- Name",
        "Zone 1,             !- Zone Name",
        ",                   !- Ceiling Height {m}",
        ",                   !- Volume {m3}",
        "5.0;                !- Floor Area {m2}",

        "Space,",
        "Space 1B,            !- Name",
        "Zone 1,             !- Zone Name",
        ",                   !- Ceiling Height {m}",
        ",                   !- Volume {m3}",
        "15.0;                !- Floor Area {m2}",

        " Zone,",
        "  Zone 2,                  !- Name",
        "  0,                       !- Direction of Relative North {deg}",
        "  0,                       !- X Origin {m}",
        "  0,                       !- Y Origin {m}",
        "  0,                       !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  autocalculate,           !- Ceiling Height {m}",
        "  autocalculate,           !- Volume {m3}",
        "  0.0;           !- Area {m2}",

        "Space,",
        "Space 2A,            !- Name",
        "Zone 2,             !- Zone Name",
        ",                   !- Ceiling Height {m}",
        ",                   !- Volume {m3}",
        "0.0;                !- Floor Area {m2}",

        "SpaceList, All Spaces, Space 1A, Space 1B, Space 2A;",

        "ZoneList, All Zones, Zone 1, Zone 2;",

        "ScheduleTypeLimits,SchType1,0.0,1.0,Continuous,Dimensionless;",

        "Schedule:Constant,Schedule1,,1.0;",

        "  Lights,",
        "    Zone 1 Lights,        !- Name",
        "    Zone 1,               !- Zone or ZoneList Name",
        "    Schedule1,               !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    100.0,                   !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0.0000,                  !- Return Air Fraction",
        "    0.7000,                  !- Fraction Radiant",
        "    0.2000,                  !- Fraction Visible",
        "    1.0000,                  !- Fraction Replaceable",
        "    General,                 !- End-Use Subcategory",
        "    No;                      !- Return Air Fraction Calculated from Plenum Temperature",

        "  Lights,",
        "    Zone 2 Lights,        !- Name",
        "    Zone 2,               !- Zone or ZoneList Name",
        "    Schedule1,               !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    100.0,                   !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0.0000,                  !- Return Air Fraction",
        "    0.7000,                  !- Fraction Radiant",
        "    0.2000,                  !- Fraction Visible",
        "    1.0000,                  !- Fraction Replaceable",
        "    General,                 !- End-Use Subcategory",
        "    No;                      !- Return Air Fraction Calculated from Plenum Temperature",

        "  Lights,",
        "    Space 1A Lights,        !- Name",
        "    Space 1A,               !- Zone or ZoneList Name",
        "    Schedule1,               !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    100.0,                   !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0.0000,                  !- Return Air Fraction",
        "    0.7000,                  !- Fraction Radiant",
        "    0.2000,                  !- Fraction Visible",
        "    1.0000,                  !- Fraction Replaceable",
        "    General,                 !- End-Use Subcategory",
        "    No;                      !- Return Air Fraction Calculated from Plenum Temperature",

        "  Lights,",
        "    All Space Lights,        !- Name",
        "    All Spaces,               !- Zone or ZoneList Name",
        "    Schedule1,               !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    100.0,                   !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0.0000,                  !- Return Air Fraction",
        "    0.7000,                  !- Fraction Radiant",
        "    0.2000,                  !- Fraction Visible",
        "    1.0000,                  !- Fraction Replaceable",
        "    General,                 !- End-Use Subcategory",
        "    No;                      !- Return Air Fraction Calculated from Plenum Temperature",

        "  Lights,",
        "    All Zone Lights,        !- Name",
        "    All Zones,               !- Zone or ZoneList Name",
        "    Schedule1,               !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    100.0,                   !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0.0000,                  !- Return Air Fraction",
        "    0.7000,                  !- Fraction Radiant",
        "    0.2000,                  !- Fraction Visible",
        "    1.0000,                  !- Fraction Replaceable",
        "    General,                 !- End-Use Subcategory",
        "    No;                      !- Return Air Fraction Calculated from Plenum Temperature",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_FALSE(has_err_output());

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    bool ErrorsFound(false);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    InternalHeatGains::GetInternalHeatGainsInput(*state);

    // Zone 1 Lights: 2 spaces
    // Zone 2 Lights: 1 space
    // Space 1A lights: 1 space
    // All Space Lights: 3 spaces
    // All Zone Lights: 3 spaces
    // 2+1+1+3+3 = 10
    EXPECT_EQ(state->dataHeatBal->TotLights, 10);
}
