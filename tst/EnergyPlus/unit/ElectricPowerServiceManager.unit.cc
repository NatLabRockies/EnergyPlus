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

// EnergyPlus::ElectricPowerServiceManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// C++ Headers
#include <memory>

// EnergyPlus Headers
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/ScheduleManager.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, ManageElectricPowerTest_BatteryDischargeTest)
{
    std::string const idf_objects = delimited_string({
        "ElectricLoadCenter:Distribution,",
        "    PV Array Load Center,    !- Name",
        "    Generator List,          !- Generator List Name",
        "    TrackElectrical,         !- Generator Operation Scheme Type",
        "    0,                       !- Demand Limit Scheme Purchased Electric Demand Limit {W}",
        "    ,                        !- Track Schedule Name Scheme Schedule Name",
        "    ,                        !- Track Meter Scheme Meter Name",
        "    DirectCurrentWithInverterDCStorage,  !- Electrical Buss Type",
        "    PV Inverter,             !- Inverter Object Name",
        "    Kibam;                   !- Electrical Storage Object Name",

        "  Curve:DoubleExponentialDecay,",
        "    Doubleexponential,       !- Name",
        "    1380,                    !- Coefficient1 C1",
        "    6834,                    !- Coefficient2 C2",
        "    -8.75,                   !- Coefficient3 C3",
        "    6747,                    !- Coefficient4 C4",
        "    -6.22,                   !- Coefficient5 C5",
        "    0,                       !- Minimum Value of x",
        "    1,                       !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for x",
        "    Dimensionless;           !- Output Unit Type",

        "  ElectricLoadCenter:Storage:Battery,",
        "    Kibam,                   !- Name",
        "    CONSTANT-1.0,               !- Availability Schedule Name",
        "    ,                        !- Zone Name",
        "    0,                       !- Radiative Fraction",
        "    10,                      !- Number of Battery Modules in Parallel",
        "    10,                      !- Number of Battery Modules in Series",
        "    86.1,                    !- Maximum Module Capacity {Ah}",
        "    0.7,                     !- Initial Fractional State of Charge",
        "    0.37,                    !- Fraction of Available Charge Capacity",
        "    0.5874,                  !- Change Rate from Bound Charge to Available Charge {1/hr}",
        "    12.6,                    !- Fully Charged Module Open Circuit Voltage {V}",
        "    12.4,                    !- Fully Discharged Module Open Circuit Voltage {V}",
        "    charging,                !- Voltage Change Curve Name for Charging",
        "    discharging,             !- Voltage Change Curve Name for Discharging",
        "    0.054,                   !- Module Internal Electrical Resistance {ohms}",
        "    100,                     !- Maximum Module Discharging Current {A}",
        "    10,                      !- Module Cut-off Voltage {V}",
        "    1,                       !- Module Charge Rate Limit",
        "    Yes,                     !- Battery Life Calculation",
        "    5,                       !- Number of Cycle Bins",
        "    Doubleexponential;       !- Battery Life Curve Name",

        "  Curve:RectangularHyperbola2,",
        "    charging,                !- Name",
        "    -.2765,                  !- Coefficient1 C1",
        "    -93.27,                  !- Coefficient2 C2",
        "    0.0068,                  !- Coefficient3 C3",
        "    0,                       !- Minimum Value of x",
        "    1,                       !- Maximum Value of x",
        "    -100,                    !- Minimum Curve Output",
        "    100,                     !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for x",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:RectangularHyperbola2,",
        "    discharging,             !- Name",
        "    0.0899,                  !- Coefficient1 C1",
        "    -98.24,                  !- Coefficient2 C2",
        "    -.0082,                  !- Coefficient3 C3",
        "    0,                       !- Minimum Value of x",
        "    1,                       !- Maximum Value of x",
        "    -100,                    !- Minimum Curve Output",
        "    100,                     !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for x",
        "    Dimensionless;           !- Output Unit Type",

        "  ElectricLoadCenter:Inverter:LookUpTable,",
        "    PV Inverter,             !- Name",
        "    CONSTANT-1.0,               !- Availability Schedule Name",
        "    ,                        !- Zone Name",
        "    0.25,                    !- Radiative Fraction",
        "    14000,                   !- Rated Maximum Continuous Output Power {W}",
        "    200.0,                   !- Night Tare Loss Power {W}",
        "    368,                     !- Nominal Voltage Input {V}",
        "    0.839,                   !- Efficiency at 10% Power and Nominal Voltage",
        "    0.897,                   !- Efficiency at 20% Power and Nominal Voltage",
        "    0.916,                   !- Efficiency at 30% Power and Nominal Voltage",
        "    0.931,                   !- Efficiency at 50% Power and Nominal Voltage",
        "    0.934,                   !- Efficiency at 75% Power and Nominal Voltage",
        "    0.930;                   !- Efficiency at 100% Power and Nominal Voltage",

        "  ElectricLoadCenter:Generators,",
        "    Generator List,          !- Name",
        "    PV:ZN_1_FLR_1_SEC_1_Ceiling,  !- Generator 1 Name",
        "    Generator:Photovoltaic,  !- Generator 1 Object Type",
        "    9000.0,                  !- Generator 1 Rated Electric Power Output {W}",
        "    CONSTANT-1.0,               !- Generator 1 Availability Schedule Name",
        "    ;                        !- Generator 1 Rated Thermal to Electrical Power Ratio",

        "  Generator:Photovoltaic,",
        "    PV:ZN_1_FLR_1_SEC_1_Ceiling,  !- Name",
        "    ZN_1_FLR_1_SEC_1_Ceiling,!- Surface Name",
        "    PhotovoltaicPerformance:Simple,  !- Photovoltaic Performance Object Type",
        "    20percentEffPVhalfArea,  !- Module Performance Name",
        "    Decoupled,               !- Heat Transfer Integration Mode",
        "    1.0,                     !- Number of Series Strings in Parallel {dimensionless}",
        "    1.0;                     !- Number of Modules in Series {dimensionless}",

        "  PhotovoltaicPerformance:Simple,",
        "    20percentEffPVhalfArea,  !- Name",
        "    0.5,                     !- Fraction of Surface Area with Active Solar Cells {dimensionless}",
        "    Fixed,                   !- Conversion Efficiency Input Mode",
        "    0.20,                    !- Value for Cell Efficiency if Fixed",
        "    ;                        !- Efficiency Schedule Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    createFacilityElectricPowerServiceObject(*state);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs.emplace_back(new ElectPowerLoadCenter(*state, 1));

    Curve::Curve *curve1 = state->dataCurveManager->curves(1);
    Real64 k = 0.5874;
    Real64 c = 0.37;
    Real64 qmax = 86.1;
    Real64 E0c = 12.6;
    Real64 InternalR = 0.054;

    Real64 I0 = 0.159;
    Real64 T0 = 537.9;
    Real64 Volt = 12.59;
    Real64 Pw = 2.0;
    Real64 q0 = 60.2;

    EXPECT_TRUE(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->storageObj->determineCurrentForBatteryDischarge(
        *state, I0, T0, Volt, Pw, q0, curve1, k, c, qmax, E0c, InternalR));

    I0 = -222.7;
    T0 = -0.145;
    Volt = 24.54;
    Pw = 48000;
    q0 = 0;

    EXPECT_FALSE(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->storageObj->determineCurrentForBatteryDischarge(
        *state, I0, T0, Volt, Pw, q0, curve1, k, c, qmax, E0c, InternalR));
}

TEST_F(EnergyPlusFixture, ManageElectricPowerTest_UpdateLoadCenterRecords_Case1)
{
    std::string const idf_objects = delimited_string({

        "  ElectricLoadCenter:Distribution,",
        "    Test Load Center,    !- Name",
        "    Test Generator List,          !- Generator List Name",
        "    TrackElectrical,         !- Generator Operation Scheme Type",
        "    0,                       !- Demand Limit Scheme Purchased Electric Demand Limit {W}",
        "    ,                        !- Track Schedule Name Scheme Schedule Name",
        "    ,                        !- Track Meter Scheme Meter Name",
        "    AlternatingCurrent,                  !- Electrical Buss Type",
        "    ,                        !- Inverter Object Name",
        "    ;                        !- Electrical Storage Object Name",

        "  ElectricLoadCenter:Generators,",
        "    Test Generator List,          !- Name",
        "    Test Gen 1,  !- Generator 1 Name",
        "    Generator:InternalCombustionEngine,  !- Generator 1 Object Type",
        "    1000.0,                  !- Generator 1 Rated Electric Power Output {W}",
        "    CONSTANT-1.0,               !- Generator 1 Availability Schedule Name",
        "    ,                        !- Generator 1 Rated Thermal to Electrical Power Ratio",
        "    Test Gen 2,  !- Generator 2 Name",
        "    Generator:WindTurbine,  !- Generator 2 Object Type",
        "    2000.0,                  !- Generator 2 Rated Electric Power Output {W}",
        "    CONSTANT-1.0,               !- Generator 2 Availability Schedule Name",
        "    ;                        !- Generator 2 Rated Thermal to Electrical Power Ratio",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    createFacilityElectricPowerServiceObject(*state);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs.emplace_back(new ElectPowerLoadCenter(*state, 1));

    // Case 1 ACBuss - Generators 1000+2000=3000, thermal 500+750=1250
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->bussType = ElectPowerLoadCenter::ElectricBussType::ACBuss;

    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->electProdRate = 1000.0;
    //	ElecLoadCenter( LoadCenterNum ).ElecGen( 1 ).ElectProdRate = 1000.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->electProdRate = 2000.0;
    //	ElecLoadCenter( LoadCenterNum ).ElecGen( 2 ).ElectProdRate = 2000.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->electricityProd = 1000.0 * 3600.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->electricityProd = 2000.0 * 3600.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->thermProdRate = 500.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->thermProdRate = 750.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->thermalProd = 500.0 * 3600.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->thermalProd = 750.0 * 3600.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->updateLoadCenterGeneratorRecords(*state);

    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->genElectProdRate, 3000.0, 0.1);
    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->genElectricProd, 3000.0 * 3600.0, 0.1);
    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->thermalProdRate, 1250.0, 0.1);
    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->thermalProd, 1250.0 * 3600.0, 0.1);
}

TEST_F(EnergyPlusFixture, ManageElectricPowerTest_UpdateLoadCenterRecords_Case2)
{
    std::string const idf_objects = delimited_string({

        "  ElectricLoadCenter:Distribution,",
        "    Test Load Center,        !- Name",
        "    Test Generator List,     !- Generator List Name",
        "    TrackElectrical,         !- Generator Operation Scheme Type",
        "    0,                       !- Demand Limit Scheme Purchased Electric Demand Limit {W}",
        "    ,                        !- Track Schedule Name Scheme Schedule Name",
        "    ,                        !- Track Meter Scheme Meter Name",
        "    AlternatingCurrentWithStorage,                  !- Electrical Buss Type",
        "     ,                        !- Inverter Object Name",
        "    Test Storage Bank;       !- Electrical Storage Object Name",

        "  ElectricLoadCenter:Storage:Simple,",
        "    Test Storage Bank,",
        "    CONSTANT-1.0, !- availability schedule",
        "    , !- zone name",
        "    , !- radiative fraction",
        "    1.0 , !- Nominal Energetic Efficiency for Charging",
        "    1.0,  !- Nominal Discharging Energetic efficiency",
        "    1.0E9, !- Maximum storage capacity",
        "    5000.0, !- Maximum Power for Discharging",
        "    5000.0, !- Maximum Power for Charging",
        "    1.0E9; !- initial stat of charge",

        "  ElectricLoadCenter:Generators,",
        "    Test Generator List,     !- Name",
        "    Test Gen 1,              !- Generator 1 Name",
        "    Generator:InternalCombustionEngine,  !- Generator 1 Object Type",
        "    1000.0,                  !- Generator 1 Rated Electric Power Output {W}",
        "    CONSTANT-1.0,               !- Generator 1 Availability Schedule Name",
        "    ,                        !- Generator 1 Rated Thermal to Electrical Power Ratio",
        "    Test Gen 2,              !- Generator 2 Name",
        "    Generator:WindTurbine,   !- Generator 2 Object Type",
        "    2000.0,                  !- Generator 2 Rated Electric Power Output {W}",
        "    CONSTANT-1.0,               !- Generator 2 Availability Schedule Name",
        "    ;                        !- Generator 2 Rated Thermal to Electrical Power Ratio",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    createFacilityElectricPowerServiceObject(*state);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs.emplace_back(new ElectPowerLoadCenter(*state, 1));

    // Case 2 ACBussStorage - Generators 1000+2000=3000, Storage 200-150=50
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->bussType = ElectPowerLoadCenter::ElectricBussType::ACBussStorage;
    //	ElectricPowerService::facilityElectricServiceObj->elecLoadCenterObjs[ 0 ]->storagePresent
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->storageObj =
        std::make_unique<ElectricStorage>(*state, "TEST STORAGE BANK");

    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->electProdRate = 1000.0;

    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->electProdRate = 2000.0;

    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->electricityProd = 1000.0 * 3600.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->electricityProd = 2000.0 * 3600.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->thermProdRate = 500.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->thermProdRate = 750.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->thermalProd = 500.0 * 3600.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->thermalProd = 750.0 * 3600.0;

    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->storOpCVDischargeRate = 200.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->storOpCVChargeRate = 150.0;

    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->updateLoadCenterGeneratorRecords(*state);

    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->genElectProdRate, 3000.0, 0.1);
    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->genElectricProd, 3000.0 * 3600.0, 0.1);
    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->subpanelFeedInRate, 3050.0, 0.1);
    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->subpanelDrawRate, 0.0, 0.1);
}

TEST_F(EnergyPlusFixture, ManageElectricPowerTest_UpdateLoadCenterRecords_Case3)
{
    std::string const idf_objects = delimited_string({

        "  ElectricLoadCenter:Distribution,",
        "    Test Load Center,        !- Name",
        "    Test Generator List,     !- Generator List Name",
        "    TrackElectrical,         !- Generator Operation Scheme Type",
        "    0,                       !- Demand Limit Scheme Purchased Electric Demand Limit {W}",
        "    ,                        !- Track Schedule Name Scheme Schedule Name",
        "    ,                        !- Track Meter Scheme Meter Name",
        "    DirectCurrentWithInverter,    !- Electrical Buss Type",
        "    Test Inverter ,          !- Inverter Object Name",
        "    ,                        !- Electrical Storage Object Name",
        "    ,                        !- Transformer Object Name",
        "    ,                        !- Storage Operation Scheme",
        "    ,                        !- Storage Control Track Meter Name",
        "    ,                        !- Storage Converter Object Name",
        "    ,                        !- Maximum Storage State of Charge Fraction",
        "    ,                        !- Minimum Storage State of Charge Fraction",
        "    100000,                  !- Design Storage Control Charge Power",
        "    ,                        !- Storage Charge Power Fraction Schedule Name",
        "    100000,                  !- Design Storage Control Discharge Power",
        "    ,                        !- Storage Discharge Power Fraction Schedule Name",
        "    ,                        !- Storage Control Utility Demand Target",
        "    ;                        !- Storage Control Utility Demand Target Fraction Schedule Name  ",

        "  ElectricLoadCenter:Inverter:Simple,",
        "    Test Inverter,",
        "    CONSTANT-1.0, !- availability schedule",
        "    , !- zone name",
        "    , !- radiative fraction",
        "    1.0 ; !- Inverter efficiency",

        "  ElectricLoadCenter:Generators,",
        "    Test Generator List,     !- Name",
        "    Test Gen 1,              !- Generator 1 Name",
        "    Generator:InternalCombustionEngine,  !- Generator 1 Object Type",
        "    1000.0,                  !- Generator 1 Rated Electric Power Output {W}",
        "    CONSTANT-1.0,               !- Generator 1 Availability Schedule Name",
        "    ,                        !- Generator 1 Rated Thermal to Electrical Power Ratio",
        "    Test Gen 2,              !- Generator 2 Name",
        "    Generator:WindTurbine,   !- Generator 2 Object Type",
        "    2000.0,                  !- Generator 2 Rated Electric Power Output {W}",
        "    CONSTANT-1.0,               !- Generator 2 Availability Schedule Name",
        "    ;                        !- Generator 2 Rated Thermal to Electrical Power Ratio",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // get availability schedule to work
    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Sched::UpdateScheduleVals(*state);

    createFacilityElectricPowerServiceObject(*state);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs.emplace_back(new ElectPowerLoadCenter(*state, 1));

    // Case 3 DCBussInverter   Inverter = 3000,
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->bussType = ElectPowerLoadCenter::ElectricBussType::DCBussInverter;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->inverterObj =
        std::make_unique<DCtoACInverter>(*state, "TEST INVERTER");
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->inverterPresent = true;

    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->electProdRate = 1000.0;

    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->electProdRate = 2000.0;

    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->electricityProd = 1000.0 * 3600.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->electricityProd = 2000.0 * 3600.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->updateLoadCenterGeneratorRecords(*state);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->inverterObj->simulate(*state, 3000.0);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->updateLoadCenterGeneratorRecords(*state);
    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->genElectProdRate, 3000.0, 0.1);
    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->genElectricProd, 3000.0 * 3600.0, 0.1);
    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->subpanelFeedInRate, 3000.0, 0.1);
}

TEST_F(EnergyPlusFixture, ManageElectricPowerTest_UpdateLoadCenterRecords_Case4)
{
    std::string const idf_objects = delimited_string({

        "  ElectricLoadCenter:Distribution,",
        "    Test Load Center,        !- Name",
        "    Test Generator List,     !- Generator List Name",
        "    TrackElectrical,         !- Generator Operation Scheme Type",
        "    0,                       !- Demand Limit Scheme Purchased Electric Demand Limit {W}",
        "    ,                        !- Track Schedule Name Scheme Schedule Name",
        "    ,                        !- Track Meter Scheme Meter Name",
        "    DirectCurrentWithInverterDCStorage,                  !- Electrical Buss Type",
        "    Test Inverter ,                        !- Inverter Object Name",
        "    Test Storage Bank,       !- Electrical Storage Object Name",
        "    ,                        !- Transformer Object Name",
        "    TrackFacilityElectricDemandStoreExcessOnSite,  !- Storage Operation Scheme",
        "    ,                        !- Storage Control Track Meter Name",
        "    ,                        !- Storage Converter Object Name",
        "    ,                        !- Maximum Storage State of Charge Fraction",
        "    ,                        !- Minimum Storage State of Charge Fraction",
        "    100000,                  !- Design Storage Control Charge Power",
        "    ,                        !- Storage Charge Power Fraction Schedule Name",
        "    100000,                  !- Design Storage Control Discharge Power",
        "    ,                        !- Storage Discharge Power Fraction Schedule Name",
        "    ,                        !- Storage Control Utility Demand Target",
        "    ;                        !- Storage Control Utility Demand Target Fraction Schedule Name  ",

        "  ElectricLoadCenter:Inverter:Simple,",
        "    Test Inverter,",
        "    CONSTANT-1.0, !- availability schedule",
        "    , !- zone name",
        "    , !- radiative fraction",
        "    1.0 ; !- Inverter efficiency",

        "  ElectricLoadCenter:Storage:Simple,",
        "    Test Storage Bank,",
        "    CONSTANT-1.0, !- availability schedule",
        "    , !- zone name",
        "    , !- radiative fraction",
        "    1.0 , !- Nominal Energetic Efficiency for Charging",
        "    1.0,  !- Nominal Discharging Energetic efficiency",
        "    1.0E9, !- Maximum storage capacity",
        "    5000.0, !- Maximum Power for Discharging",
        "    5000.0, !- Maximum Power for Charging",
        "    1.0E9; !- initial stat of charge",

        "  ElectricLoadCenter:Generators,",
        "    Test Generator List,     !- Name",
        "    Test Gen 1,              !- Generator 1 Name",
        "    Generator:InternalCombustionEngine,  !- Generator 1 Object Type",
        "    1000.0,                  !- Generator 1 Rated Electric Power Output {W}",
        "    CONSTANT-1.0,               !- Generator 1 Availability Schedule Name",
        "    ,                        !- Generator 1 Rated Thermal to Electrical Power Ratio",
        "    Test Gen 2,              !- Generator 2 Name",
        "    Generator:WindTurbine,   !- Generator 2 Object Type",
        "    2000.0,                  !- Generator 2 Rated Electric Power Output {W}",
        "    CONSTANT-1.0,               !- Generator 2 Availability Schedule Name",
        "    ;                        !- Generator 2 Rated Thermal to Electrical Power Ratio",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);
    createFacilityElectricPowerServiceObject(*state);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs.emplace_back(new ElectPowerLoadCenter(*state, 1));

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Sched::UpdateScheduleVals(*state);

    // Case 4 DCBussInverterDCStorage    Inverter = 5000,
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->bussType =
        ElectPowerLoadCenter::ElectricBussType::DCBussInverterDCStorage;

    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->inverterObj =
        std::make_unique<DCtoACInverter>(*state, "TEST INVERTER");
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->inverterPresent = true;

    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->electProdRate = 2000.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->electProdRate = 3000.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->electricityProd = 2000.0 * 3600.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->electricityProd = 3000.0 * 3600.0;

    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->updateLoadCenterGeneratorRecords(*state);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->inverterObj->simulate(*state, 5000.0);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->updateLoadCenterGeneratorRecords(*state);

    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->genElectProdRate, 5000.0, 0.1);
    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->genElectricProd, 5000.0 * 3600.0, 0.1);
    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->subpanelFeedInRate, 5000.0, 0.1);
}

TEST_F(EnergyPlusFixture, ManageElectricPowerTest_UpdateLoadCenterRecords_Case5)
{
    std::string const idf_objects = delimited_string({

        "  ElectricLoadCenter:Distribution,",
        "    Test Load Center,        !- Name",
        "    Test Generator List,     !- Generator List Name",
        "    TrackElectrical,         !- Generator Operation Scheme Type",
        "    0,                       !- Demand Limit Scheme Purchased Electric Demand Limit {W}",
        "    ,                        !- Track Schedule Name Scheme Schedule Name",
        "    ,                        !- Track Meter Scheme Meter Name",
        "    DirectCurrentWithInverterACStorage,                  !- Electrical Buss Type",
        "    Test Inverter ,                        !- Inverter Object Name",
        "    Test Storage Bank;       !- Electrical Storage Object Name",

        "  ElectricLoadCenter:Inverter:Simple,",
        "    Test Inverter,",
        "    CONSTANT-1.0, !- availability schedule",
        "    , !- zone name",
        "    , !- radiative fraction",
        "    1.0 ; !- Inverter efficiency",

        "  ElectricLoadCenter:Storage:Simple,",
        "    Test Storage Bank,",
        "    CONSTANT-1.0, !- availability schedule",
        "    , !- zone name",
        "    , !- radiative fraction",
        "    1.0 , !- Nominal Energetic Efficiency for Charging",
        "    1.0,  !- Nominal Discharging Energetic efficiency",
        "    1.0E9, !- Maximum storage capacity",
        "    5000.0, !- Maximum Power for Discharging",
        "    5000.0, !- Maximum Power for Charging",
        "    1.0E9; !- initial stat of charge",

        "  ElectricLoadCenter:Generators,",
        "    Test Generator List,     !- Name",
        "    Test Gen 1,              !- Generator 1 Name",
        "    Generator:InternalCombustionEngine,  !- Generator 1 Object Type",
        "    1000.0,                  !- Generator 1 Rated Electric Power Output {W}",
        "    CONSTANT-1.0,               !- Generator 1 Availability Schedule Name",
        "    ,                        !- Generator 1 Rated Thermal to Electrical Power Ratio",
        "    Test Gen 2,              !- Generator 2 Name",
        "    Generator:WindTurbine,   !- Generator 2 Object Type",
        "    2000.0,                  !- Generator 2 Rated Electric Power Output {W}",
        "    CONSTANT-1.0,               !- Generator 2 Availability Schedule Name",
        "    ;                        !- Generator 2 Rated Thermal to Electrical Power Ratio",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // get availability schedule to work
    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Sched::UpdateScheduleVals(*state);

    createFacilityElectricPowerServiceObject(*state);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs.emplace_back(new ElectPowerLoadCenter(*state, 1));

    // Case 5 DCBussInverterACStorage     Inverter = 5000, , Storage 200-150=50, thermal should still be same as Case 1
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->bussType =
        (ElectPowerLoadCenter::ElectricBussType::DCBussInverterACStorage);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->inverterObj =
        std::make_unique<DCtoACInverter>(*state, "TEST INVERTER");
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->inverterPresent = true;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->storageObj =
        std::make_unique<ElectricStorage>(*state, "TEST STORAGE BANK");
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->storOpCVDischargeRate = 200.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->storOpCVChargeRate = 150.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->electProdRate = 2000.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->electProdRate = 3000.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->electricityProd = 2000.0 * 3600.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->electricityProd = 3000.0 * 3600.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->thermProdRate = 500.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->thermProdRate = 750.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[0]->thermalProd = 500.0 * 3600.0;
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->elecGenCntrlObj[1]->thermalProd = 750.0 * 3600.0;

    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->updateLoadCenterGeneratorRecords(*state);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->inverterObj->simulate(*state, 5000.0);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->updateLoadCenterGeneratorRecords(*state);

    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->subpanelFeedInRate, 5050.0, 0.1);
    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->subpanelDrawRate, 0.0, 0.1);
    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->thermalProdRate, 1250.0, 0.1);
    EXPECT_NEAR(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->thermalProd, 1250.0 * 3600.0, 0.1);
}
TEST_F(EnergyPlusFixture, ManageElectricPowerTest_CheckOutputReporting)
{
    std::string const idf_objects = delimited_string({

        "  LoadProfile:Plant,",
        "    Campus Load Profile, !- Name",
        "    Node 41, !- Inlet Node Name",
        "    Node 42, !- Outlet Node Name",
        "    Campus output Load, !- Load Schedule Name",
        "    0.320003570569675, !- Peak Flow Rate{ m3 / s }",
        "    Campus output Flow Frac;         !- Flow Rate Fraction Schedule Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataHVACGlobal->TimeStepSys = 1.0;
    state->dataHVACGlobal->TimeStepSysSec = 3600.0;
    state->dataGlobal->TimeStepZoneSec = 3600.0;
    state->init_state(*state);

    createFacilityElectricPowerServiceObject(*state);
    bool SimElecCircuitsFlag = false;
    // GetInput and other code will be executed and SimElectricCircuits will be true
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->manageElectricPowerService(*state, true, SimElecCircuitsFlag, false);
    EXPECT_TRUE(SimElecCircuitsFlag);
    EXPECT_EQ(state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->numGenerators,
              0); // dummy generator has been added and report variables are available
}
TEST_F(EnergyPlusFixture, ManageElectricPowerTest_TransformerLossTest)
{
    std::string const idf_objects = delimited_string({

        "  ElectricLoadCenter:Distribution,",
        "    Test Load Center,        !- Name",
        "    Generator List,          !- Generator List Name",
        "    TrackElectrical,         !- Generator Operation Scheme Type",
        "    10000.0,                 !- Demand Limit Scheme Purchased Electric Demand Limit {W}",
        "    ,                        !- Track Schedule Name Scheme Schedule Name",
        "    ,                        !- Track Meter Scheme Meter Name",
        "    AlternatingCurrent,      !- Electrical Buss Type",
        "    Test Inverter,           !- Inverter Object Name",
        "    Test Storage Bank,       !- Electrical Storage Object Name",
        "    Transformer;             !- Transformer Object Name",

        "  ElectricLoadCenter:Inverter:Simple,",
        "    Test Inverter,",
        "    CONSTANT-1.0,               !- availability schedule",
        "    ,                        !- zone name",
        "    ,                        !- radiative fraction",
        "    1.0;                     !- Inverter efficiency",

        "  ElectricLoadCenter:Storage:Simple,",
        "    Test Storage Bank,",
        "    CONSTANT-1.0,               !- availability schedule",
        "    ,                        !- zone name",
        "    ,                        !- radiative fraction",
        "    1.0,                     !- Nominal Energetic Efficiency for Charging",
        "    1.0,                     !- Nominal Discharging Energetic efficiency",
        "    1.0E9,                   !- Maximum storage capacity",
        "    5000.0,                  !- Maximum Power for Discharging",
        "    5000.0,                  !- Maximum Power for Charging",
        "    1.0E9;                   !- initial stat of charge",

        "  ElectricLoadCenter:Generators,",
        "    Generator List,          !- Name",
        "    Test Gen 1,              !- Generator 1 Name",
        "    Generator:InternalCombustionEngine,  !- Generator 1 Object Type",
        "    1000.0,                  !- Generator 1 Rated Electric Power Output {W}",
        "    CONSTANT-1.0,               !- Generator 1 Availability Schedule Name",
        "    ,                        !- Generator 1 Rated Thermal to Electrical Power Ratio",
        "    Test Gen 2,              !- Generator 2 Name",
        "    Generator:WindTurbine,   !- Generator 2 Object Type",
        "    2000.0,                  !- Generator 2 Rated Electric Power Output {W}",
        "    CONSTANT-1.0,               !- Generator 2 Availability Schedule Name",
        "    ;                        !- Generator 2 Rated Thermal to Electrical Power Ratio",

        "  ElectricLoadCenter:Transformer,",
        "    Transformer,             !- Name",
        "    CONSTANT-1.0,               !- Availability Schedule Name",
        "    PowerOutToGrid,          !- Transformer Usage",
        "    ,                        !- Zone Name",
        "    ,                        !- Radiative Fraction",
        "    ,                        !- Rated Capacity {VA}",
        "    3,                       !- Phase",
        "    Aluminum,                !- Conductor Material",
        "    150,                     !- Full Load Temperature Rise {C}",
        "    0.1,                     !- Fraction of Eddy Current Losses",
        "    RatedLosses,             !- Performance Input Method",
        "    300,                     !- Rated No Load Loss {W}",
        "    2000,                    !- Rated Load Loss {W}",
        "    ,                        !- Nameplate Efficiency",
        "    ,                        !- Per Unit Load for Nameplate Efficiency",
        "    ,                        !- Reference Temperature for Nameplate Efficiency {C}",
        "    ,                        !- Per Unit Load for Maximum Efficiency",
        "    ;                        !- Consider Transformer Loss for Utility Cost",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // get availability schedule to work
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataHVACGlobal->TimeStepSys = 1.0;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Sched::UpdateScheduleVals(*state);

    createFacilityElectricPowerServiceObject(*state);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs.emplace_back(new ElectPowerLoadCenter(*state, 1));
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->transformerObj =
        std::make_unique<ElectricTransformer>(*state, "TRANSFORMER");
    Real64 expectedtransformerObjLossRate =
        state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs[0]->transformerObj->getLossRateForOutputPower(*state, 2000.0);
    // check the transformer loss rate for load and no load condition
    EXPECT_EQ(expectedtransformerObjLossRate, 0.0);
}

// #7151: If an ElectricLoadCenter:Generators lists a Generator:Phototoltaic of type Simple PV with an availability schedule, warn that it will be
// unused. Any other performance type shouldn't warn
TEST_F(EnergyPlusFixture, ElectricLoadCenter_WarnAvailabilitySchedule_Photovoltaic_Simple)
{
    std::string const idf_objects = delimited_string({
        "ElectricLoadCenter:Distribution,",
        "  PV Electric Load Center, !- Name",
        "  PV Generator List,       !- Generator List Name",
        "  Baseload,                !- Generator Operation Scheme Type",
        "  0,                       !- Generator Demand Limit Scheme Purchased Electric Demand Limit {W}",
        "  ,                        !- Generator Track Schedule Name Scheme Schedule Name",
        "  ,                        !- Generator Track Meter Scheme Meter Name",
        "  DirectCurrentWithInverter,  !- Electrical Buss Type",
        "  Simple Ideal Inverter;   !- Inverter Name",

        "ElectricLoadCenter:Inverter:Simple,",
        "  Simple Ideal Inverter,   !- Name",
        "  PV_ON,                   !- Availability Schedule Name",
        "  ,                        !- Zone Name",
        "  0.0,                     !- Radiative Fraction",
        "  1.0;                     !- Inverter Efficiency",

        "ScheduleTypeLimits,",
        "  OnOff,                   !- Name",
        "  0,                       !- Lower Limit Value",
        "  1,                       !- Upper Limit Value",
        "  Discrete;                !- Numeric Type",

        "Schedule:Compact,",
        "  PV_ON,                   !- Name",
        "  OnOff,                   !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 11:00,            !- Field 3",
        "  0.0,                     !- Field 4",
        "  Until: 15:00,            !- Field 5",
        "  1.0,                     !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  0.0;                     !- Field 8",

        "ElectricLoadCenter:Generators,",
        "  PV Generator List,       !- Name",
        "  SimplePV,                !- Generator 1 Name",
        "  Generator:Photovoltaic,  !- Generator 1 Object Type",
        "  20000,                   !- Generator 1 Rated Electric Power Output {W}",
        "  PV_ON,                   !- Generator 1 Availability Schedule Name",
        "  ,                        !- Generator 1 Rated Thermal to Electrical Power Ratio",
        "  SimplePV2,               !- Generator 2 Name",
        "  Generator:Photovoltaic,  !- Generator 2 Object Type",
        "  20000,                   !- Generator 2 Rated Electric Power Output {W}",
        "  ,                        !- Generator 2 Availability Schedule Name",
        "  ,                        !- Generator 2 Rated Thermal to Electrical Power Ratio",
        "  TRNSYSPV INTEGRATED PV,  !- Generator 3 Name",
        "  Generator:Photovoltaic,  !- Generator 3 Object Type",
        "  20000,                   !- Generator 3 Rated Electric Power Output {W}",
        "  ,                        !- Generator 3 Availability Schedule Name",
        "  ;                        !- Generator 3 Rated Thermal to Electrical Power Ratio",

        "Shading:Site:Detailed,",
        "  FlatSurface,             !- Name",
        "  ,                        !- Transmittance Schedule Name",
        "  4,                       !- Number of Vertices",
        "  40.0,2.0,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "  40.0,0.00,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "  45.0,0.00,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "  45.0,2.0,0.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "PhotovoltaicPerformance:Simple,",
        "  12percentEffPVFullArea,  !- Name",
        "  1.0,                     !- Fraction of Surface Area with Active Solar Cells {dimensionless}",
        "  Fixed,                   !- Conversion Efficiency Input Mode",
        "  0.12;                    !- Value for Cell Efficiency if Fixed",

        "Generator:Photovoltaic,",
        "  SimplePV,                !- Name",
        "  FlatSurface,             !- Surface Name",
        "  PhotovoltaicPerformance:Simple,  !- Photovoltaic Performance Object Type",
        "  12percentEffPVFullArea,  !- Module Performance Name",
        "  Decoupled,               !- Heat Transfer Integration Mode",
        "  1.0,                     !- Number of Series Strings in Parallel {dimensionless}",
        "  1.0;                     !- Number of Modules in Series {dimensionless}",

        "Generator:Photovoltaic,",
        "  SimplePV2,               !- Name",
        "  FlatSurface,             !- Surface Name",
        "  PhotovoltaicPerformance:Simple,  !- Photovoltaic Performance Object Type",
        "  12percentEffPVFullArea,  !- Module Performance Name",
        "  Decoupled,               !- Heat Transfer Integration Mode",
        "  1.0,                     !- Number of Series Strings in Parallel {dimensionless}",
        "  1.0;                     !- Number of Modules in Series {dimensionless}",

        "Generator:Photovoltaic,",
        "  TRNSYSPV INTEGRATED PV,  !- Name",
        "  FlatSurface,          !- Surface Name",
        "  PhotovoltaicPerformance:EquivalentOne-Diode,  !- Photovoltaic Performance Object Type",
        "  Example PV Model Inputs, !- Module Performance Name",
        "  IntegratedSurfaceOutsideFace,  !- Heat Transfer Integration Mode",
        "  3.0,                     !- Number of Series Strings in Parallel {dimensionless}",
        "  6.0;                     !- Number of Modules in Series {dimensionless}",

        "PhotovoltaicPerformance:EquivalentOne-Diode,",
        "  Example PV Model Inputs, !- Name",
        "  CrystallineSilicon,      !- Cell type",
        "  36,                      !- Number of Cells in Series {dimensionless}",
        "  0.63,                    !- Active Area {m2}",
        "  0.9,                     !- Transmittance Absorptance Product {dimensionless}",
        "  1.12,                    !- Semiconductor Bandgap {eV}",
        "  1000000,                 !- Shunt Resistance {ohms}",
        "  4.75,                    !- Short Circuit Current {A}",
        "  21.4,                    !- Open Circuit Voltage {V}",
        "  25.0,                    !- Reference Temperature {C}",
        "  1000.0,                  !- Reference Insolation {W/m2}",
        "  4.45,                    !- Module Current at Maximum Power {A}",
        "  17,                      !- Module Voltage at Maximum Power {V}",
        "  0.00065,                 !- Temperature Coefficient of Short Circuit Current {A/K}",
        "  -0.08,                   !- Temperature Coefficient of Open Circuit Voltage {V/K}",
        "  20,                      !- Nominal Operating Cell Temperature Test Ambient Temperature {C}",
        "  47,                      !- Nominal Operating Cell Temperature Test Cell Temperature {C}",
        "  800.0,                   !- Nominal Operating Cell Temperature Test Insolation {W/m2}",
        "  30.0,                    !- Module Heat Loss Coefficient {W/m2-K}",
        "  50000;                   !- Total Heat Capacity {J/m2-K}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    createFacilityElectricPowerServiceObject(*state);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs.emplace_back(new ElectPowerLoadCenter(*state, 1));

    // Should warn only for SimplePV because SimplePV2 doesn't have a schedule, and the other one is a Perf One-Diode and not "Simple"
    std::string const error_string = delimited_string({
        "   ** Warning ** GeneratorController constructor : GENERATOR:PHOTOVOLTAIC = SIMPLEPV",
        "   **   ~~~   ** Availability Schedule will be ignored (runs all the time).",
        "   **   ~~~   ** To limit this Generator:Photovoltaic's output, please use the Inverter's availability schedule instead.",
    });
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

// #7151: If an ElectricLoadCenter:Generators lists a Generator:PVWatts with an availability schedule, warn that it will be unused
TEST_F(EnergyPlusFixture, ElectricLoadCenter_WarnAvailabilitySchedule_PVWatts)
{
    std::string const idf_objects = delimited_string({
        "ElectricLoadCenter:Distribution,",
        "  PVWatts Electric Load Center,  !- Name",
        "  PVWatts Generator List,  !- Generator List Name",
        "  Baseload,                !- Generator Operation Scheme Type",
        "  0,                       !- Generator Demand Limit Scheme Purchased Electric Demand Limit {W}",
        "  ,                        !- Generator Track Schedule Name Scheme Schedule Name",
        "  ,                        !- Generator Track Meter Scheme Meter Name",
        "  DirectCurrentWithInverter,  !- Electrical Buss Type",
        "  PVWatts Inverter;        !- Inverter Name",

        "ElectricLoadCenter:Inverter:PVWatts,",
        "  PVWatts Inverter,        !- Name",
        "  1.10,                    !- DC to AC Size Ratio",
        "  0.96;                    !- Inverter Efficiency",

        "ScheduleTypeLimits,",
        "  OnOff,                   !- Name",
        "  0,                       !- Lower Limit Value",
        "  1,                       !- Upper Limit Value",
        "  Discrete;                !- Numeric Type",

        "Schedule:Compact,",
        "  PV_ON,                   !- Name",
        "  OnOff,                   !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 11:00,            !- Field 3",
        "  0.0,                     !- Field 4",
        "  Until: 15:00,            !- Field 5",
        "  1.0,                     !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  0.0;                     !- Field 8",

        "ElectricLoadCenter:Generators,",
        "  PVWatts Generator List,  !- Name",
        "  PVWatts1,                !- Generator 1 Name",
        "  Generator:PVWatts,       !- Generator 1 Object Type",
        "  4000,                    !- Generator 1 Rated Electric Power Output {W}",
        "  PV_ON,                   !- Generator 1 Availability Schedule Name",
        "  ,                        !- Generator 1 Rated Thermal to Electrical Power Ratio",
        "  PVWatts2,                !- Generator 2 Name",
        "  Generator:PVWatts,       !- Generator 2 Object Type",
        "  3000,                    !- Generator 2 Rated Electric Power Output {W}",
        "  ,                        !- Generator 2 Availability Schedule Name",
        "  ;                        !- Generator 2 Rated Thermal to Electrical Power Ratio",

        "Generator:PVWatts,",
        "  PVWatts1,                !- Name",
        "  5,                       !- PVWatts Version",
        "  4000,                    !- DC System Capacity {W}",
        "  Standard,                !- Module Type",
        "  FixedOpenRack,           !- Array Type",
        "  0.14,                    !- System Losses",
        "  TiltAzimuth,             !- Array Geometry Type",
        "  20,                      !- Tilt Angle {deg}",
        "  180;                     !- Azimuth Angle {deg}",

        "Generator:PVWatts,",
        "  PVWatts2,                !- Name",
        "  5,                       !- PVWatts Version",
        "  4000,                    !- DC System Capacity {W}",
        "  Standard,                !- Module Type",
        "  FixedOpenRack,           !- Array Type",
        "  0.14,                    !- System Losses",
        "  TiltAzimuth,             !- Array Geometry Type",
        "  20,                      !- Tilt Angle {deg}",
        "  180;                     !- Azimuth Angle {deg}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    createFacilityElectricPowerServiceObject(*state);
    state->dataElectPwrSvcMgr->facilityElectricServiceObj->elecLoadCenterObjs.emplace_back(new ElectPowerLoadCenter(*state, 1));

    // Should warn only for PVWatts1 because PVWatts2 doesn't have a schedule
    std::string const error_string = delimited_string({"   ** Warning ** GeneratorController constructor : GENERATOR:PVWATTS = PVWATTS1",
                                                       "   **   ~~~   ** Availability Schedule will be ignored (runs all the time)."});
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, Battery_LiIonNmc_Constructor)
{
    std::string const idf_objects = delimited_string({
        "ElectricLoadCenter:Storage:LiIonNMCBattery,",
        "  Battery1,       !- Name",
        "  ,               !- Availability Schedule Name",
        "  ,               !- Zone Name",
        "  ,               !- Radiative Fraction",
        "  KandlerSmith,   !- Lifetime Model",
        "  139,            !- Number of Cells in Series",
        "  8,              !- Number of Strings in Parallel",
        "  0.95,           !- Initial Fractional State of Charge",
        "  ,               !- DC to DC Charging Efficiency",
        "  100,            !- Battery Mass",
        "  0.75,           !- Battery Surface Area",
        "  1500,           !- Battery Specific Heat Capacity",
        "  8.1;            !- Heat Transfer Coefficient Between Battery and Ambient",

        "ElectricLoadCenter:Storage:LiIonNMCBattery,",
        "  Battery2,       !- Name",
        "  ,               !- Availability Schedule Name",
        "  ,               !- Zone Name",
        "  ,               !- Radiative Fraction",
        "  None,           !- Lifetime Model",
        "  139,            !- Number of Cells in Series",
        "  10,             !- Number of Strings in Parallel",
        "  0.5,            !- Initial Fractional State of Charge",
        "  ,               !- DC to DC Charging Efficiency",
        "  100,            !- Battery Mass",
        "  0.75,           !- Battery Surface Area",
        "  ,               !- Battery Specific Heat Capacity",
        "  ,               !- Heat Transfer Coefficient Between Battery and Ambient",
        "  1,              !- Fully Charged Cell Voltage",
        "  2,              !- Cell Voltage at End of Exponential Zone",
        "  3,              !- Cell Voltage at End of Nominal Zone",
        "  ,               !- Default Nominal Cell Voltage",
        "  ,               !- Fully Charged Cell Capacity",
        "  0.9,            !- Fraction of Cell Capacity Removed at the End of Exponential Zone",
        "  0.8;            !- Fraction of Cell Capacity Removed at the End of Nominal Zone",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    ElectricStorage battery1{*state, "Battery1"};
    ASSERT_TRUE(Util::SameString(battery1.name(), "Battery1"));

    ASSERT_THROW(ElectricStorage battery2(*state, "Battery2"), EnergyPlus::FatalError);
    std::string const error_string = delimited_string(
        {"   ** Severe  ** ElectricStorage constructor ElectricLoadCenter:Storage:LiIonNMCBattery=\"BATTERY2\", invalid entry.",
         "   **   ~~~   ** Fully Charged Cell Voltage must be greater than Cell Voltage at End of Exponential Zone,",
         "   **   ~~~   ** which must be greater than Cell Voltage at End of Nominal Zone.",
         "   **   ~~~   ** Fully Charged Cell Voltage = 1.00000",
         "   **   ~~~   ** Cell Voltage at End of Exponential Zone = 2.00000",
         "   **   ~~~   ** Cell Voltage at End of Nominal Zone = 3.00000",
         "   ** Severe  ** ElectricStorage constructor ElectricLoadCenter:Storage:LiIonNMCBattery=\"BATTERY2\", invalid entry.",
         "   **   ~~~   ** Fraction of Cell Capacity Removed at the End of Nominal Zone must be greater than Fraction of Cell Capacity Removed at "
         "the End of Exponential Zone.",
         "   **   ~~~   ** Fraction of Cell Capacity Removed at the End of Exponential Zone = 0.90000",
         "   **   ~~~   ** Fraction of Cell Capacity Removed at the End of Nominal Zone = 0.80000",
         "   **  Fatal  ** ElectricStorage constructor Preceding errors terminate program.",
         "   ...Summary of Errors that led to program termination:",
         "   ..... Reference severe error count=2",
         "   ..... Last severe error=ElectricStorage constructor ElectricLoadCenter:Storage:LiIonNMCBattery=\"BATTERY2\", invalid entry."});
    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, Battery_LiIonNmc_Simulate)
{
    std::string const idf_objects = delimited_string({
        "ElectricLoadCenter:Storage:LiIonNMCBattery,",
        "  Battery1,       !- Name",
        "  ,               !- Availability Schedule Name",
        "  ,               !- Zone Name",
        "  ,               !- Radiative Fraction",
        "  KandlerSmith,   !- Lifetime Model",
        "  139,            !- Number of Cells in Series",
        "  8,              !- Number of Strings in Parallel",
        "  0.95,           !- Initial Fractional State of Charge",
        "  ,               !- DC to DC Charging Efficiency",
        "  100,            !- Battery Mass",
        "  0.75,           !- Battery Surface Area",
        "  ,               !- Battery Specific Heat Capacity",
        "  ;               !- Heat Transfer Coefficient Between Battery and Ambient",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    state->dataHVACGlobal->TimeStepSys = 0.25;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    state->init_state(*state);

    ElectricStorage battery{*state, "Battery1"};

    state->dataEnvrn->OutDryBulbTemp = 23.0;
    Real64 socMin = 0.1;
    Real64 socMax = 0.95;

    Real64 powerCharge = 0.0;
    Real64 powerDischarge = 3000.0;
    bool charging = false;
    bool discharging = true;

    battery.simulate(*state, powerCharge, powerDischarge, charging, discharging, socMax, socMin);

    ASSERT_NEAR(battery.storedPower(), 0.0, 0.1);
    ASSERT_NEAR(battery.storedEnergy(), 0.0, 0.1);
    ASSERT_NEAR(battery.drawnPower(), powerDischarge, 0.1);
    ASSERT_NEAR(battery.drawnEnergy(), powerDischarge * 15 * 60, 0.1);
    ASSERT_NEAR(battery.stateOfChargeFraction(), 0.90, 0.01);
    ASSERT_NEAR(battery.batteryTemperature(), 20.1, 0.1);

    state->dataHVACGlobal->SysTimeElapsed += state->dataHVACGlobal->TimeStepSys;
    powerDischarge = 0.0;
    powerCharge = 5000.0;
    charging = true;
    discharging = false;

    battery.timeCheckAndUpdate(*state);
    battery.simulate(*state, powerCharge, powerDischarge, charging, discharging, socMax, socMin);

    // Doesn't accept all the power, because the battery will be at capacity.
    ASSERT_NEAR(battery.storedPower(), 3148.85, 0.1);
    ASSERT_NEAR(battery.storedEnergy(), 2833963.14, 0.1);
    ASSERT_NEAR(battery.drawnPower(), 0.0, 0.1);
    ASSERT_NEAR(battery.drawnEnergy(), 0.0, 0.1);
    ASSERT_NEAR(battery.stateOfChargeFraction(), socMax, 0.01);

    // Discharge the battery some more (redo of last timestep)
    powerDischarge = 5000.0;
    powerCharge = 0.0;
    charging = false;
    discharging = true;
    battery.timeCheckAndUpdate(*state);
    battery.simulate(*state, powerCharge, powerDischarge, charging, discharging, socMax, socMin);
    ASSERT_NEAR(battery.stateOfChargeFraction(), 0.813, 0.01);

    // See that the battery state is reset at the beginning of a new environment (and also at the end of warmup)
    state->dataHVACGlobal->SysTimeElapsed = 0.0;
    battery.reinitAtBeginEnvironment();
    battery.timeCheckAndUpdate(*state);
    powerDischarge = 0.0;
    powerCharge = 0.0;
    charging = false;
    discharging = false;
    battery.simulate(*state, powerCharge, powerDischarge, charging, discharging, socMax, socMin);
    ASSERT_NEAR(battery.stateOfChargeFraction(), 0.95, 0.1);
}

TEST_F(EnergyPlusFixture, Battery_checkUserEfficiencyInputTest)
{
    Real64 userInputEfficiencyCharge;
    Real64 userInputEfficiencyDischarge;
    Real64 functionResult;
    Real64 expectedResult;
    bool errorsFound;

    // Fix for Defect #8867: EnergyPlus was allowing zero efficiency which led to a divide by zero in ElectricPowerServiceManager.cc.
    // Input is now tested to make sure that a zero value is not allowed.

    state->init_state(*state);
    // Test 1: charging, charging efficiency zero-->gets reset to minimum (0.001)
    userInputEfficiencyCharge = 0.0;
    expectedResult = 0.001;
    errorsFound = false;
    functionResult = checkUserEfficiencyInput(*state, userInputEfficiencyCharge, true, "Tatooine", errorsFound);
    EXPECT_NEAR(functionResult, expectedResult, 0.00001);
    std::string const error_string1 = delimited_string(
        {EnergyPlus::format("   ** Warning ** Version: missing in IDF, processing for EnergyPlus version=\"{}\"", DataStringGlobals::MatchVersion),
         "   ** Severe  ** ElectricStorage charge efficiency was too low.  This occurred for electric storage unit named Tatooine",
         "   **   ~~~   ** Please check your input value  for this electric storage unit and fix the charge efficiency."});
    EXPECT_TRUE(compare_err_stream(error_string1, true));
    EXPECT_TRUE(errorsFound);

    // Test 2: charging, value greater than minimum (0.001)-->just keep the user input value
    userInputEfficiencyCharge = 0.7;
    expectedResult = 0.7;
    errorsFound = false;
    functionResult = checkUserEfficiencyInput(*state, userInputEfficiencyCharge, true, "Tatooine", errorsFound);
    EXPECT_NEAR(functionResult, expectedResult, 0.00001);
    EXPECT_FALSE(errorsFound);

    // Test 3: discharging, discharging efficiency less than minimum allowed-->gets reset to minimum (0.001)
    userInputEfficiencyDischarge = -1.0;
    expectedResult = 0.001;
    errorsFound = false;
    functionResult = checkUserEfficiencyInput(*state, userInputEfficiencyDischarge, false, "Tatooine", errorsFound);
    EXPECT_NEAR(functionResult, expectedResult, 0.00001);
    std::string const error_string2 = delimited_string(
        {"   ** Severe  ** ElectricStorage discharge efficiency was too low.  This occurred for electric storage unit named Tatooine",
         "   **   ~~~   ** Please check your input value  for this electric storage unit and fix the discharge efficiency."});
    EXPECT_TRUE(compare_err_stream(error_string2, true));
    EXPECT_TRUE(errorsFound);

    // Test 4: discharging, value greater than minimum (0.001)-->just keep the user input value
    userInputEfficiencyDischarge = 0.9;
    expectedResult = 0.9;
    errorsFound = false;
    functionResult = checkUserEfficiencyInput(*state, userInputEfficiencyDischarge, false, "Tatooine", errorsFound);
    EXPECT_NEAR(functionResult, expectedResult, 0.00001);
    EXPECT_FALSE(errorsFound);
}

TEST_F(EnergyPlusFixture, Battery_checkChargeDischargeVoltageCurves)
{
    Real64 e0c;
    Real64 e0d;
    Curve::Curve *chaCurve;
    Curve::Curve *disCurve;

    std::string const idf_objects = delimited_string({
        "Curve:RectangularHyperbola2,",
        "  charging1,               !- Name",
        "  0.25,                    !- Coefficient1 C1",
        "  4.0,                     !- Coefficient2 C2",
        "  2.0,                     !- Coefficient3 C3",
        "  0,                       !- Minimum Value of x",
        "  1,                       !- Maximum Value of x",
        "  -100,                    !- Minimum Curve Output",
        "  100,                     !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for x",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:RectangularHyperbola2,",
        "  discharging1,            !- Name",
        "  0.25,                    !- Coefficient1 C1",
        "  -4.0,                    !- Coefficient2 C2",
        "  -2.0,                    !- Coefficient3 C3",
        "  0,                       !- Minimum Value of x",
        "  1,                       !- Maximum Value of x",
        "  -100,                    !- Minimum Curve Output",
        "  100,                     !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for x",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:RectangularHyperbola2,",
        "  charging2,               !- Name",
        "  -.2765,                  !- Coefficient1 C1",
        "  -93.27,                  !- Coefficient2 C2",
        "  0.0068,                  !- Coefficient3 C3",
        "  0,                       !- Minimum Value of x",
        "  1,                       !- Maximum Value of x",
        "  -100,                    !- Minimum Curve Output",
        "  100,                     !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for x",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:RectangularHyperbola2,",
        "  discharging2,            !- Name",
        "  0.0899,                  !- Coefficient1 C1",
        "  -98.24,                  !- Coefficient2 C2",
        "  -.0082,                  !- Coefficient3 C3",
        "  0,                       !- Minimum Value of x",
        "  1,                       !- Maximum Value of x",
        "  -100,                    !- Minimum Curve Output",
        "  100,                     !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for x",
        "  Dimensionless;           !- Output Unit Type",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // Test 1: Curves which do not produce errors
    e0c = 13.0;
    e0d = 11.0;
    chaCurve = Curve::GetCurve(*state, "CHARGING1");
    disCurve = Curve::GetCurve(*state, "DISCHARGING1");

    checkChargeDischargeVoltageCurves(*state, "Battery for Test 1", e0c, e0d, chaCurve, disCurve);

    EXPECT_TRUE(compare_err_stream(""));

    // Test 2: Curves from ShopwithPVBattery.idf as of 7/2023
    e0c = 12.6;
    e0d = 12.4;
    chaCurve = Curve::GetCurve(*state, "CHARGING2");
    disCurve = Curve::GetCurve(*state, "DISCHARGING2");

    checkChargeDischargeVoltageCurves(*state, "Battery for Test 2", e0c, e0d, chaCurve, disCurve);

    std::string const error_string2 = delimited_string({
        "   ** Warning ** Kinetic Battery Model: Battery for Test 2 has a charging/discharging voltage curve conflict.",
        "   **   ~~~   ** Discharging voltage is higher than charging voltage which may potentially lead to an imbalance in the stored energy.",
        "   **   ~~~   ** Check the charging and discharging curves to make sure that the charging voltage is greater than discharging.",
        "   **   ~~~   ** Also check the charging and discharging energy outputs to find any discrepancies.",
        "   **   ~~~   ** Charged fraction = 0.00, Charging voltage = 12.400 V, Discharging voltage = 12.591 V",
        "   **   ~~~   ** Charged fraction = 0.10, Charging voltage = 12.401 V, Discharging voltage = 12.592 V",
        "   **   ~~~   ** Charged fraction = 0.20, Charging voltage = 12.402 V, Discharging voltage = 12.593 V",
        "   **   ~~~   ** Charged fraction = 0.30, Charging voltage = 12.403 V, Discharging voltage = 12.594 V",
        "   **   ~~~   ** Charged fraction = 0.40, Charging voltage = 12.404 V, Discharging voltage = 12.595 V",
        "   **   ~~~   ** Charged fraction = 0.50, Charging voltage = 12.405 V, Discharging voltage = 12.595 V",
        "   **   ~~~   ** Charged fraction = 0.60, Charging voltage = 12.406 V, Discharging voltage = 12.596 V",
        "   **   ~~~   ** Charged fraction = 0.70, Charging voltage = 12.407 V, Discharging voltage = 12.597 V",
        "   **   ~~~   ** Charged fraction = 0.80, Charging voltage = 12.408 V, Discharging voltage = 12.598 V",
        "   **   ~~~   ** Charged fraction = 0.90, Charging voltage = 12.409 V, Discharging voltage = 12.599 V",
        "   **   ~~~   ** Charged fraction = 1.00, Charging voltage = 12.410 V, Discharging voltage = 12.600 V",
    });
    EXPECT_TRUE(compare_err_stream(error_string2, true));
}
