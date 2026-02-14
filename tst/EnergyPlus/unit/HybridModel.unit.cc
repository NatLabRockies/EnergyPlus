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

// EnergyPlus::ZoneTempPredictorCorrector Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

#include "Fixtures/EnergyPlusFixture.hh"

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HybridModel.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/ZoneContaminantPredictorCorrector.hh>
#include <EnergyPlus/ZonePlenum.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataContaminantBalance;
using namespace EnergyPlus::DataHeatBalFanSys;
using namespace EnergyPlus::DataZoneControls;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::DataZoneEnergyDemands;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::ZonePlenum;
using namespace EnergyPlus::ZoneTempPredictorCorrector;
using namespace EnergyPlus::ZoneContaminantPredictorCorrector;
using namespace EnergyPlus::DataLoopNode;
using namespace EnergyPlus::DataSurfaces;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::RoomAir;
using namespace EnergyPlus::HybridModel;
using namespace EnergyPlus::DataPrecisionGlobals;

TEST_F(EnergyPlusFixture, HybridModel_correctZoneAirTempsTest)
{
    state->init_state(*state);
    // ZoneTempPredictorCorrector variable initialization
    state->dataHeatBal->Zone.allocate(1);
    state->dataHybridModel->hybridModelZones.allocate(1);
    state->dataHybridModel->FlagHybridModel = true;
    state->dataRoomAir->AirModel.allocate(1);
    state->dataRoomAir->ZTOC.allocate(1);
    state->dataRoomAir->ZTMX.allocate(1);
    state->dataRoomAir->ZTMMX.allocate(1);
    state->afn->exchangeData.allocate(1);
    state->dataLoopNodes->Node.allocate(1);
    state->dataHeatBalFanSys->TempTstatAir.allocate(1);
    state->dataHeatBalFanSys->LoadCorrectionFactor.allocate(1);
    state->dataHeatBalFanSys->PreviousMeasuredZT1.allocate(1);
    state->dataHeatBalFanSys->PreviousMeasuredZT2.allocate(1);
    state->dataHeatBalFanSys->PreviousMeasuredZT3.allocate(1);
    state->dataHeatBalFanSys->PreviousMeasuredHumRat1.allocate(1);
    state->dataHeatBalFanSys->PreviousMeasuredHumRat2.allocate(1);
    state->dataHeatBalFanSys->PreviousMeasuredHumRat3.allocate(1);

    // CalcZoneComponentLoadSums variable initialization
    state->dataSurface->SurfaceWindow.allocate(1);
    state->dataSurface->Surface.allocate(2);
    state->dataHeatBalSurf->SurfHConvInt.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataRoomAir->IsZoneDispVent3Node.dimension(1, false);
    state->dataRoomAir->IsZoneCrossVent.dimension(1, false);
    state->dataRoomAir->IsZoneUFAD.dimension(1, false);
    state->dataRoomAir->ZoneDispVent3NodeMixedFlag.allocate(1);
    state->dataHeatBal->ZnAirRpt.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataHeatBal->ZoneIntGain.allocate(1);
    state->dataSize->ZoneEqSizing.allocate(1);

    // CorrectZoneHumRat variable initialization
    state->dataHeatBalFanSys->SumLatentHTRadSys.allocate(1);
    state->dataHeatBalFanSys->SumLatentHTRadSys(1) = 0.0;
    state->dataHeatBalFanSys->SumConvHTRadSys.allocate(1);
    state->dataHeatBalFanSys->SumConvHTRadSys(1) = 0.0;
    state->dataHeatBalFanSys->SumConvPool.allocate(1);
    state->dataHeatBalFanSys->SumConvPool(1) = 0.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->spaceHeatBalance.allocate(1);
    auto &thisZoneHB = state->dataZoneTempPredictorCorrector->zoneHeatBalance(1);
    thisZoneHB.MixingMassFlowXHumRat = 0.0;
    thisZoneHB.MixingMassFlowZone = 0.0;
    thisZoneHB.ZT = 0.0;
    state->dataHeatBalFanSys->SumLatentPool.allocate(1);
    state->dataHeatBalFanSys->SumLatentPool(1) = 0.0;

    // CorrectZoneContaminants variable initialization
    state->dataContaminantBalance->AZ.allocate(1);
    state->dataContaminantBalance->BZ.allocate(1);
    state->dataContaminantBalance->CZ.allocate(1);
    state->dataContaminantBalance->AZGC.allocate(1);
    state->dataContaminantBalance->BZGC.allocate(1);
    state->dataContaminantBalance->CZGC.allocate(1);
    state->dataContaminantBalance->AZ(1) = 0.0;
    state->dataContaminantBalance->BZ(1) = 0.0;
    state->dataContaminantBalance->CZ(1) = 0.0;
    state->dataContaminantBalance->AZGC(1) = 0.0;
    state->dataContaminantBalance->BZGC(1) = 0.0;
    state->dataContaminantBalance->CZGC(1) = 0.0;
    state->dataContaminantBalance->ZoneAirDensityCO.allocate(1);
    state->dataContaminantBalance->ZoneAirDensityCO(1) = 0.0;
    state->dataContaminantBalance->ZoneGCGain.allocate(1);
    state->dataContaminantBalance->ZoneGCGain(1) = 0.0;

    // Parameter setup
    state->dataGlobal->NumOfZones = 1;
    state->dataSize->CurZoneEqNum = 1;
    state->dataZonePlenum->NumZoneReturnPlenums = 0;
    state->dataZonePlenum->NumZoneSupplyPlenums = 0;
    state->dataHeatBal->Zone(1).IsControlled = true;
    state->dataHeatBal->Zone(1).Multiplier = 1;
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 1;
    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->spaceIntGainDevices.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes.emplace_back(1);
    state->dataHeatBal->space(1).HTSurfaceFirst = 0;
    state->dataHeatBal->space(1).HTSurfaceLast = -1;
    state->dataHeatBal->Zone(1).Volume = 1061.88;
    state->dataGlobal->TimeStepZone = 10.0 / 60.0; // Zone timestep in hours
    state->dataHVACGlobal->TimeStepSys = 10.0 / 60.0;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    Real64 ZoneTempChange;

    // Hybrid modeling trigger
    state->dataHybridModel->FlagHybridModel_TM = true;
    state->dataGlobal->WarmupFlag = false;
    state->dataGlobal->DoingSizing = false;
    state->dataEnvrn->DayOfYear = 1;

    // Case 1: Hybrid model internal thermal mass (free-floating)
    auto &hmZone = state->dataHybridModel->hybridModelZones(1);
    hmZone.InternalThermalMassCalc_T = true;
    hmZone.InfiltrationCalc_T = false;
    hmZone.InfiltrationCalc_H = false;
    hmZone.InfiltrationCalc_C = false;
    hmZone.PeopleCountCalc_T = false;
    hmZone.PeopleCountCalc_H = false;
    hmZone.PeopleCountCalc_C = false;
    hmZone.HybridStartDayOfYear = 1;
    hmZone.HybridEndDayOfYear = 2;
    thisZoneHB.MAT = 0.0;
    state->dataHeatBalFanSys->PreviousMeasuredZT1(1) = 0.1;
    state->dataHeatBalFanSys->PreviousMeasuredZT2(1) = 0.2;
    state->dataHeatBalFanSys->PreviousMeasuredZT3(1) = 0.3;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = -5.21;
    thisZoneHB.airHumRat = 0.002083;
    thisZoneHB.MCPV = 1414.60;   // Assign TempDepCoef
    thisZoneHB.MCPTV = -3335.10; // Assign TempIndCoef
    state->dataEnvrn->OutBaroPress = 99166.67;

    ZoneTempChange = correctZoneAirTemps(*state, true);
    EXPECT_NEAR(15.13, state->dataHeatBal->Zone(1).ZoneVolCapMultpSensHM, 0.01);

    // Case 2: Hybrid model infiltration with measured temperature (free-floating)

    hmZone.InternalThermalMassCalc_T = false;
    hmZone.InfiltrationCalc_T = true;
    hmZone.InfiltrationCalc_H = false;
    hmZone.InfiltrationCalc_C = false;
    hmZone.PeopleCountCalc_T = false;
    hmZone.PeopleCountCalc_H = false;
    hmZone.PeopleCountCalc_C = false;
    hmZone.HybridStartDayOfYear = 1;
    hmZone.HybridEndDayOfYear = 2;
    thisZoneHB.MAT = 0.0;
    state->dataHeatBalFanSys->PreviousMeasuredZT1(1) = 0.02;
    state->dataHeatBalFanSys->PreviousMeasuredZT2(1) = 0.04;
    state->dataHeatBalFanSys->PreviousMeasuredZT3(1) = 0.06;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpSens = 8.0;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = -6.71;
    thisZoneHB.airHumRat = 0.002083;
    thisZoneHB.MCPV = 539.49;  // Assign TempDepCoef
    thisZoneHB.MCPTV = 270.10; // Assign TempIndCoef
    state->dataEnvrn->OutBaroPress = 99250;

    ZoneTempChange = correctZoneAirTemps(*state, true);
    EXPECT_NEAR(0.2444, state->dataHeatBal->Zone(1).InfilOAAirChangeRateHM, 0.01);

    // Case 3: Hybrid model infiltration with measured humidity ratio (free-floating)

    hmZone.InternalThermalMassCalc_T = false;
    hmZone.InfiltrationCalc_T = false;
    hmZone.InfiltrationCalc_H = true;
    hmZone.InfiltrationCalc_C = false;
    hmZone.PeopleCountCalc_T = false;
    hmZone.PeopleCountCalc_H = false;
    hmZone.PeopleCountCalc_C = false;
    hmZone.HybridStartDayOfYear = 1;
    hmZone.HybridEndDayOfYear = 2;
    state->dataHeatBal->Zone(1).Volume = 4000;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = -10.62;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpMoist = 1.0;
    thisZoneHB.airHumRat = 0.001120003;
    thisZoneHB.ZT = -6.08;
    state->dataEnvrn->OutHumRat = 0.0011366887816818931;
    state->dataHeatBalFanSys->PreviousMeasuredHumRat1(1) = 0.0011186324286;
    state->dataHeatBalFanSys->PreviousMeasuredHumRat2(1) = 0.0011172070768;
    state->dataHeatBalFanSys->PreviousMeasuredHumRat3(1) = 0.0011155109625;
    hmZone.measuredHumRatSched = Sched::AddScheduleConstant(*state, "Measured HumRat 1");
    hmZone.measuredHumRatSched->currentVal = 0.001120003;
    thisZoneHB.MCPV = 539.49;
    thisZoneHB.MCPTV = 270.10;
    state->dataEnvrn->OutBaroPress = 99500;

    thisZoneHB.correctHumRat(*state, 1);
    EXPECT_NEAR(0.5, state->dataHeatBal->Zone(1).InfilOAAirChangeRateHM, 0.01);

    // Case 4: Hybrid model people count with measured temperature (free-floating)

    hmZone.InternalThermalMassCalc_T = false;
    hmZone.InfiltrationCalc_T = false;
    hmZone.InfiltrationCalc_H = false;
    hmZone.InfiltrationCalc_C = false;
    hmZone.PeopleCountCalc_T = true;
    hmZone.PeopleCountCalc_H = false;
    hmZone.PeopleCountCalc_C = false;
    hmZone.HybridStartDayOfYear = 1;
    hmZone.HybridEndDayOfYear = 2;

    thisZoneHB.MAT = -2.89;
    state->dataHeatBalFanSys->PreviousMeasuredZT1(1) = -2.887415174;
    state->dataHeatBalFanSys->PreviousMeasuredZT2(1) = -2.897557416;
    state->dataHeatBalFanSys->PreviousMeasuredZT3(1) = -2.909294101;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpSens = 1.0;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = -6.71;
    thisZoneHB.airHumRat = 0.0024964;
    state->dataEnvrn->OutBaroPress = 98916.7;
    thisZoneHB.MCPV = 5163.5;    // Assign TempDepCoef
    thisZoneHB.MCPTV = -15956.8; // Assign TempIndCoef
    hmZone.measuredTempSched = Sched::AddScheduleConstant(*state, "Measured Temp 1");
    hmZone.measuredTempSched->currentVal = -2.923892218;

    ZoneTempChange = correctZoneAirTemps(*state, true);
    EXPECT_NEAR(0, state->dataHeatBal->Zone(1).NumOccHM, 0.1); // Need to initialize SumIntGain

    // Case 5: Hybrid model people count with measured humidity ratio (free-floating)

    hmZone.InternalThermalMassCalc_T = false;
    hmZone.InfiltrationCalc_T = false;
    hmZone.InfiltrationCalc_H = false;
    hmZone.InfiltrationCalc_C = false;
    hmZone.PeopleCountCalc_T = false;
    hmZone.PeopleCountCalc_H = true;
    hmZone.PeopleCountCalc_C = false;
    hmZone.HybridStartDayOfYear = 1;
    hmZone.HybridEndDayOfYear = 2;
    state->dataHeatBal->Zone(1).Volume = 4000;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = -10.62;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpMoist = 1.0;
    thisZoneHB.airHumRat = 0.0024964;
    thisZoneHB.ZT = -2.92;
    state->dataEnvrn->OutHumRat = 0.0025365002784602363;
    state->dataEnvrn->OutBaroPress = 98916.7;
    thisZoneHB.OAMFL = 0.700812;
    thisZoneHB.latentGain = 211.2;
    thisZoneHB.latentGainExceptPeople = 0.0;
    state->dataHeatBalFanSys->PreviousMeasuredHumRat1(1) = 0.002496356;
    state->dataHeatBalFanSys->PreviousMeasuredHumRat2(1) = 0.002489048;
    state->dataHeatBalFanSys->PreviousMeasuredHumRat3(1) = 0.002480404;
    hmZone.measuredHumRatSched = Sched::GetSchedule(*state, "MEASURED HUMRAT 1");
    hmZone.measuredHumRatSched->currentVal = 0.002506251487737;

    thisZoneHB.correctHumRat(*state, 1);
    EXPECT_NEAR(4, state->dataHeatBal->Zone(1).NumOccHM, 0.1);

    // Case 6: Hybrid model infiltration with measured temperature (with HVAC)

    hmZone.InternalThermalMassCalc_T = false;
    hmZone.InfiltrationCalc_T = true;
    hmZone.InfiltrationCalc_H = false;
    hmZone.InfiltrationCalc_C = false;
    hmZone.PeopleCountCalc_T = false;
    hmZone.PeopleCountCalc_H = false;
    hmZone.PeopleCountCalc_C = false;
    hmZone.IncludeSystemSupplyParameters = true;
    hmZone.HybridStartDayOfYear = 1;
    hmZone.HybridEndDayOfYear = 2;
    thisZoneHB.MAT = 15.56;
    state->dataHeatBalFanSys->PreviousMeasuredZT1(1) = 15.56;
    state->dataHeatBalFanSys->PreviousMeasuredZT2(1) = 15.56;
    state->dataHeatBalFanSys->PreviousMeasuredZT3(1) = 15.56;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpSens = 1.0;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = -10.62;
    thisZoneHB.airHumRat = 0.0077647;
    thisZoneHB.MCPV = 4456;   // Assign TempDepCoef
    thisZoneHB.MCPTV = 60650; // Assign TempIndCoef
    state->dataEnvrn->OutBaroPress = 99500;
    state->dataEnvrn->OutHumRat = 0.00113669;
    hmZone.measuredTempSched = Sched::GetSchedule(*state, "MEASURED TEMP 1");
    hmZone.supplyAirTempSched = Sched::AddScheduleConstant(*state, "Supply Temp 1");
    hmZone.supplyAirMassFlowRateSched = Sched::AddScheduleConstant(*state, "Mass Flow Rate 1");
    hmZone.measuredTempSched->currentVal = 15.56;
    hmZone.supplyAirTempSched->currentVal = 50;
    hmZone.supplyAirMassFlowRateSched->currentVal = 0.7974274;

    ZoneTempChange = correctZoneAirTemps(*state, true);
    EXPECT_NEAR(0.49, state->dataHeatBal->Zone(1).InfilOAAirChangeRateHM, 0.01);

    // Case 7: Hybrid model infiltration with measured humidity ratio (with HVAC)

    hmZone.InternalThermalMassCalc_T = false;
    hmZone.InfiltrationCalc_T = false;
    hmZone.InfiltrationCalc_H = true;
    hmZone.InfiltrationCalc_C = false;
    hmZone.PeopleCountCalc_T = false;
    hmZone.PeopleCountCalc_H = false;
    hmZone.PeopleCountCalc_C = false;
    hmZone.IncludeSystemSupplyParameters = true;
    hmZone.HybridStartDayOfYear = 1;
    hmZone.HybridEndDayOfYear = 2;
    state->dataHeatBal->Zone(1).Volume = 4000;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = -10.62;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpMoist = 1.0;
    thisZoneHB.airHumRat = 0.001120003;
    thisZoneHB.ZT = -6.08;
    state->dataEnvrn->OutHumRat = 0.0011366887816818931;
    state->dataHeatBalFanSys->PreviousMeasuredHumRat1(1) = 0.007855718;
    state->dataHeatBalFanSys->PreviousMeasuredHumRat2(1) = 0.007852847;
    state->dataHeatBalFanSys->PreviousMeasuredHumRat3(1) = 0.007850236;
    hmZone.measuredHumRatSched = Sched::GetSchedule(*state, "MEASURED HUMRAT 1");
    hmZone.supplyAirHumRatSched = Sched::AddScheduleConstant(*state, "Supply HumRat 1");
    hmZone.supplyAirMassFlowRateSched = Sched::AddScheduleConstant(*state, "Supply Mass Flow Rate 1");
    hmZone.measuredHumRatSched->currentVal = 0.00792;
    hmZone.supplyAirHumRatSched->currentVal = 0.015;
    hmZone.supplyAirMassFlowRateSched->currentVal = 0.8345;
    state->dataEnvrn->OutBaroPress = 99500;

    thisZoneHB.correctHumRat(*state, 1);
    EXPECT_NEAR(0.5, state->dataHeatBal->Zone(1).InfilOAAirChangeRateHM, 0.01);

    // Case 8: Hybrid model people count with measured temperature (with HVAC)

    hmZone.InternalThermalMassCalc_T = false;
    hmZone.InfiltrationCalc_T = false;
    hmZone.InfiltrationCalc_H = false;
    hmZone.InfiltrationCalc_C = false;
    hmZone.PeopleCountCalc_T = true;
    hmZone.PeopleCountCalc_H = false;
    hmZone.PeopleCountCalc_C = false;
    hmZone.IncludeSystemSupplyParameters = true;
    hmZone.HybridStartDayOfYear = 1;
    hmZone.HybridEndDayOfYear = 2;
    thisZoneHB.MAT = -2.89;
    state->dataHeatBalFanSys->PreviousMeasuredZT1(1) = 21.11;
    state->dataHeatBalFanSys->PreviousMeasuredZT2(1) = 21.11;
    state->dataHeatBalFanSys->PreviousMeasuredZT3(1) = 21.11;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpSens = 1.0;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = -6.71;
    thisZoneHB.airHumRat = 0.0024964;
    state->dataEnvrn->OutBaroPress = 98916.7;
    thisZoneHB.MCPV = 6616;      // Assign TempDepCoef
    thisZoneHB.MCPTV = 138483.2; // Assign TempIndCoef
    hmZone.measuredTempSched = Sched::GetSchedule(*state, "MEASURED TEMP 1");
    hmZone.supplyAirTempSched = Sched::GetSchedule(*state, "SUPPLY TEMP 1");
    hmZone.supplyAirMassFlowRateSched = Sched::GetSchedule(*state, "SUPPLY MASS FLOW RATE 1");
    hmZone.peopleActivityLevelSched = Sched::AddScheduleConstant(*state, "People Activity Level 1");
    hmZone.peopleSensibleFracSched = Sched::AddScheduleConstant(*state, "People Sensible Fraction 1");
    hmZone.peopleRadiantFracSched = Sched::AddScheduleConstant(*state, "People Radiation Fraction 1");
    hmZone.measuredTempSched->currentVal = 21.11;
    hmZone.supplyAirTempSched->currentVal = 50;
    hmZone.supplyAirMassFlowRateSched->currentVal = 1.446145794;
    hmZone.peopleActivityLevelSched->currentVal = 120;
    hmZone.peopleSensibleFracSched->currentVal = 0.6;
    hmZone.peopleRadiantFracSched->currentVal = 0.3;

    ZoneTempChange = correctZoneAirTemps(*state, true);
    EXPECT_NEAR(0, state->dataHeatBal->Zone(1).NumOccHM, 0.1); // Need to initialize SumIntGain

    // Case 9: Hybrid model people count with measured humidity ratio (with HVAC)
    hmZone.InternalThermalMassCalc_T = false;
    hmZone.InfiltrationCalc_T = false;
    hmZone.InfiltrationCalc_H = false;
    hmZone.InfiltrationCalc_C = false;
    hmZone.PeopleCountCalc_T = false;
    hmZone.PeopleCountCalc_H = true;
    hmZone.PeopleCountCalc_C = false;
    hmZone.IncludeSystemSupplyParameters = true;
    hmZone.HybridStartDayOfYear = 1;
    hmZone.HybridEndDayOfYear = 2;
    state->dataHeatBal->Zone(1).Volume = 4000;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = -10.62;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpMoist = 1.0;
    thisZoneHB.airHumRat = 0.001120003;
    thisZoneHB.ZT = -6.08;
    state->dataEnvrn->OutHumRat = 0.0011366887816818931;
    state->dataHeatBalFanSys->PreviousMeasuredHumRat1(1) = 0.011085257;
    state->dataHeatBalFanSys->PreviousMeasuredHumRat2(1) = 0.011084959;
    state->dataHeatBalFanSys->PreviousMeasuredHumRat3(1) = 0.011072322;
    hmZone.measuredHumRatSched = Sched::GetSchedule(*state, "MEASURED HUMRAT 1");
    hmZone.supplyAirHumRatSched = Sched::GetSchedule(*state, "SUPPLY HUMRAT 1");
    hmZone.supplyAirMassFlowRateSched = Sched::GetSchedule(*state, "SUPPLY MASS FLOW RATE 1");
    hmZone.peopleActivityLevelSched = Sched::GetSchedule(*state, "PEOPLE ACTIVITY LEVEL 1");
    hmZone.peopleSensibleFracSched = Sched::GetSchedule(*state, "PEOPLE SENSIBLE FRACTION 1");
    hmZone.peopleRadiantFracSched = Sched::GetSchedule(*state, "PEOPLE RADIATION FRACTION 1");
    hmZone.measuredHumRatSched->currentVal = 0.01107774;
    hmZone.supplyAirHumRatSched->currentVal = 0.015;
    hmZone.supplyAirMassFlowRateSched->currentVal = 1.485334886;
    hmZone.peopleActivityLevelSched->currentVal = 120;
    hmZone.peopleSensibleFracSched->currentVal = 0.6;
    hmZone.peopleRadiantFracSched->currentVal = 0.3;
    state->dataEnvrn->OutBaroPress = 99500;

    thisZoneHB.correctHumRat(*state, 1);
    EXPECT_NEAR(4, state->dataHeatBal->Zone(1).NumOccHM, 0.1);
}

TEST_F(EnergyPlusFixture, HybridModel_CorrectZoneContaminantsTest)
{
    state->init_state(*state);
    // ZoneContaminantPredictorCorrector variable initialization
    state->dataHeatBal->Zone.allocate(1);
    state->dataHybridModel->hybridModelZones.allocate(1);

    state->dataHybridModel->FlagHybridModel = true;
    state->dataRoomAir->AirModel.allocate(1);
    state->dataRoomAir->ZTOC.allocate(1);
    state->afn->exchangeData.allocate(1);
    state->dataLoopNodes->Node.allocate(1);
    state->dataHeatBalFanSys->TempTstatAir.allocate(1);
    state->dataHeatBalFanSys->LoadCorrectionFactor.allocate(1);
    state->dataHeatBalFanSys->PreviousMeasuredZT1.allocate(1);
    state->dataHeatBalFanSys->PreviousMeasuredZT2.allocate(1);
    state->dataHeatBalFanSys->PreviousMeasuredZT3.allocate(1);
    state->dataContaminantBalance->CO2ZoneTimeMinus1Temp.allocate(1);
    state->dataContaminantBalance->CO2ZoneTimeMinus2Temp.allocate(1);
    state->dataContaminantBalance->CO2ZoneTimeMinus3Temp.allocate(1);
    state->dataContaminantBalance->CO2ZoneTimeMinus1.allocate(1);
    state->dataContaminantBalance->CO2ZoneTimeMinus2.allocate(1);
    state->dataContaminantBalance->CO2ZoneTimeMinus3.allocate(1);

    // CalcZoneComponentLoadSums variable initialization
    state->dataSurface->SurfaceWindow.allocate(1);
    state->dataSurface->Surface.allocate(2);
    state->dataHeatBalSurf->SurfHConvInt.allocate(1);
    state->dataRoomAir->IsZoneDispVent3Node.dimension(1, false);
    state->dataRoomAir->IsZoneCrossVent.dimension(1, false);
    state->dataRoomAir->IsZoneUFAD.dimension(1, false);
    state->dataRoomAir->ZoneDispVent3NodeMixedFlag.allocate(1);
    state->dataHeatBal->ZnAirRpt.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataSize->ZoneEqSizing.allocate(1);

    // CorrectZoneContaminants variable initialization
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->spaceHeatBalance.allocate(1);
    auto &thisZoneHB = state->dataZoneTempPredictorCorrector->zoneHeatBalance(1);
    auto &hmZone = state->dataHybridModel->hybridModelZones(1);

    thisZoneHB.MixingMassFlowZone = 0.0;
    thisZoneHB.ZT = 0.0;
    state->dataContaminantBalance->AZ.allocate(1);
    state->dataContaminantBalance->BZ.allocate(1);
    state->dataContaminantBalance->CZ.allocate(1);
    state->dataContaminantBalance->AZGC.allocate(1);
    state->dataContaminantBalance->BZGC.allocate(1);
    state->dataContaminantBalance->CZGC.allocate(1);
    state->dataContaminantBalance->AZ(1) = 0.0;
    state->dataContaminantBalance->BZ(1) = 0.0;
    state->dataContaminantBalance->CZ(1) = 0.0;
    state->dataContaminantBalance->AZGC(1) = 0.0;
    state->dataContaminantBalance->BZGC(1) = 0.0;
    state->dataContaminantBalance->CZGC(1) = 0.0;
    state->dataContaminantBalance->ZoneAirCO2.allocate(1);
    state->dataContaminantBalance->ZoneAirCO2(1) = 0.0;
    state->dataContaminantBalance->ZoneAirCO2Temp.allocate(1);
    state->dataContaminantBalance->ZoneAirCO2Temp(1) = 0.0;
    state->dataContaminantBalance->ZoneAirDensityCO.allocate(1);
    state->dataContaminantBalance->ZoneAirDensityCO(1) = 0.0;
    state->dataContaminantBalance->ZoneCO2Gain.allocate(1);
    state->dataContaminantBalance->ZoneCO2Gain(1) = 0.0;
    state->dataContaminantBalance->ZoneCO2GainExceptPeople.allocate(1);
    state->dataContaminantBalance->ZoneCO2GainExceptPeople(1) = 0.0;
    state->dataContaminantBalance->ZoneGCGain.allocate(1);
    state->dataContaminantBalance->ZoneGCGain(1) = 0.0;
    state->dataContaminantBalance->MixingMassFlowCO2.allocate(1);
    state->dataContaminantBalance->MixingMassFlowCO2(1) = 0.0;

    // Parameter setup
    state->dataGlobal->NumOfZones = 1;
    state->dataSize->CurZoneEqNum = 1;
    state->dataZonePlenum->NumZoneReturnPlenums = 0;
    state->dataZonePlenum->NumZoneSupplyPlenums = 0;
    state->dataHeatBal->Zone(1).IsControlled = true;
    state->dataHeatBal->Zone(1).Multiplier = 1;
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 1;
    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->spaceIntGainDevices.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes.emplace_back(1);
    state->dataHeatBal->space(1).HTSurfaceFirst = 0;
    state->dataHeatBal->space(1).HTSurfaceLast = -1;
    state->dataHeatBal->Zone(1).Volume = 4000;
    state->dataGlobal->TimeStepZone = 10.0 / 60.0; // Zone timestep in hours
    state->dataHVACGlobal->TimeStepSys = 10.0 / 60.0;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    // Hybrid modeling trigger
    state->dataHybridModel->FlagHybridModel_TM = false;
    state->dataGlobal->WarmupFlag = false;
    state->dataGlobal->DoingSizing = false;
    state->dataEnvrn->DayOfYear = 1;

    // Case 1: Hybrid model infiltration with measured CO2 concentration (free-floating)

    state->dataContaminantBalance->Contaminant.CO2Simulation = true;
    hmZone.InternalThermalMassCalc_T = false;
    hmZone.InfiltrationCalc_T = false;
    hmZone.InfiltrationCalc_H = false;
    hmZone.InfiltrationCalc_C = true;
    hmZone.PeopleCountCalc_T = false;
    hmZone.PeopleCountCalc_H = false;
    hmZone.PeopleCountCalc_C = false;
    hmZone.HybridStartDayOfYear = 1;
    hmZone.HybridEndDayOfYear = 2;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpCO2 = 1.0;
    thisZoneHB.airHumRat = 0.001120003;
    state->dataContaminantBalance->OutdoorCO2 = 387.6064554;
    state->dataEnvrn->OutHumRat = 0.001147;
    state->dataEnvrn->OutBaroPress = 99500;
    state->dataContaminantBalance->CO2ZoneTimeMinus1(1) = 388.595225;
    state->dataContaminantBalance->CO2ZoneTimeMinus2(1) = 389.084601;
    state->dataContaminantBalance->CO2ZoneTimeMinus3(1) = 388.997009;
    hmZone.measuredCO2ConcSched = Sched::AddScheduleConstant(*state, "Measured CO2");
    hmZone.measuredCO2ConcSched->currentVal = 388.238646;

    CorrectZoneContaminants(*state, true);
    EXPECT_NEAR(0.5, state->dataHeatBal->Zone(1).InfilOAAirChangeRateHM, 0.01);

    // Case 2: Hybrid model people count with measured CO2 concentration (free-floating)

    state->dataContaminantBalance->Contaminant.CO2Simulation = true;
    hmZone.InternalThermalMassCalc_T = false;
    hmZone.InfiltrationCalc_T = false;
    hmZone.InfiltrationCalc_H = false;
    hmZone.InfiltrationCalc_C = false;
    hmZone.PeopleCountCalc_T = false;
    hmZone.PeopleCountCalc_H = false;
    hmZone.PeopleCountCalc_C = true;
    hmZone.HybridStartDayOfYear = 1;
    hmZone.HybridEndDayOfYear = 2;
    state->dataHeatBal->Zone(1).Volume = 4000;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpCO2 = 1.0;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = -1.0394166434012677;
    thisZoneHB.ZT = -2.92;
    thisZoneHB.airHumRat = 0.00112;
    state->dataContaminantBalance->OutdoorCO2 = 387.6064554;
    state->dataEnvrn->OutBaroPress = 98916.7;
    thisZoneHB.OAMFL = 0.700812;
    state->dataContaminantBalance->ZoneCO2Gain(1) = 0.00001989;
    state->dataContaminantBalance->CO2ZoneTimeMinus1(1) = 387.9962885;
    state->dataContaminantBalance->CO2ZoneTimeMinus2(1) = 387.676037;
    state->dataContaminantBalance->CO2ZoneTimeMinus3(1) = 387.2385685;
    hmZone.measuredCO2ConcSched = Sched::AddScheduleConstant(*state, "Measured CO2");
    hmZone.measuredCO2ConcSched->currentVal = 389.8511796;
    CorrectZoneContaminants(*state, true);
    EXPECT_NEAR(4, state->dataHeatBal->Zone(1).NumOccHM, 0.1);

    // Case 3: Hybrid model infiltration with measured CO2 concentration (with HVAC)
    state->dataContaminantBalance->Contaminant.CO2Simulation = true;
    hmZone.InternalThermalMassCalc_T = false;
    hmZone.InfiltrationCalc_T = false;
    hmZone.InfiltrationCalc_H = false;
    hmZone.InfiltrationCalc_C = true;
    hmZone.PeopleCountCalc_T = false;
    hmZone.PeopleCountCalc_H = false;
    hmZone.PeopleCountCalc_C = false;
    hmZone.IncludeSystemSupplyParameters = true;
    hmZone.HybridStartDayOfYear = 1;
    hmZone.HybridEndDayOfYear = 2;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpCO2 = 1.0;
    thisZoneHB.ZT = 15.56;
    thisZoneHB.airHumRat = 0.00809;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = -10.7;
    state->dataEnvrn->OutBaroPress = 99500;
    state->dataContaminantBalance->ZoneCO2Gain(1) = 0.0;
    state->dataContaminantBalance->CO2ZoneTimeMinus1(1) = 388.54049;
    state->dataContaminantBalance->CO2ZoneTimeMinus2(1) = 389.0198771;
    state->dataContaminantBalance->CO2ZoneTimeMinus3(1) = 388.9201464;
    hmZone.measuredCO2ConcSched = Sched::GetSchedule(*state, "MEASURED CO2");
    hmZone.supplyAirCO2ConcSched = Sched::AddScheduleConstant(*state, "Supply CO2");
    hmZone.supplyAirMassFlowRateSched = Sched::AddScheduleConstant(*state, "Supply Mass Flow Rate");
    hmZone.measuredCO2ConcSched->currentVal = 388.2075472;
    hmZone.supplyAirCO2ConcSched->currentVal = 388.54049;
    hmZone.supplyAirMassFlowRateSched->currentVal = 0.898375186;

    CorrectZoneContaminants(*state, true);
    EXPECT_NEAR(0.5, state->dataHeatBal->Zone(1).InfilOAAirChangeRateHM, 0.01);

    // Case 4: Hybrid model people count with measured CO2 concentration (with HVAC)

    state->dataContaminantBalance->Contaminant.CO2Simulation = true;
    hmZone.InternalThermalMassCalc_T = false;
    hmZone.InfiltrationCalc_T = false;
    hmZone.InfiltrationCalc_H = false;
    hmZone.InfiltrationCalc_C = false;
    hmZone.PeopleCountCalc_T = false;
    hmZone.PeopleCountCalc_H = false;
    hmZone.PeopleCountCalc_C = true;
    hmZone.IncludeSystemSupplyParameters = true;
    hmZone.HybridStartDayOfYear = 1;
    hmZone.HybridEndDayOfYear = 2;
    state->dataHeatBal->Zone(1).ZoneVolCapMultpCO2 = 1.0;
    thisZoneHB.ZT = 21.1;
    thisZoneHB.airHumRat = 0.01102;
    state->dataEnvrn->OutBaroPress = 98933.3;
    state->dataContaminantBalance->ZoneCO2Gain(1) = 0.00003333814;
    state->dataContaminantBalance->ZoneCO2GainExceptPeople(1) = 0.0;
    state->dataContaminantBalance->CO2ZoneTimeMinus1(1) = 387.2253194;
    state->dataContaminantBalance->CO2ZoneTimeMinus2(1) = 387.1898423;
    state->dataContaminantBalance->CO2ZoneTimeMinus3(1) = 387.4064128;
    hmZone.measuredCO2ConcSched = Sched::GetSchedule(*state, "MEASURED CO2");
    hmZone.supplyAirCO2ConcSched = Sched::GetSchedule(*state, "SUPPLY CO2");
    hmZone.supplyAirMassFlowRateSched = Sched::GetSchedule(*state, "SUPPLY MASS FLOW RATE");
    hmZone.peopleActivityLevelSched = Sched::AddScheduleConstant(*state, "People Activity Level");
    hmZone.peopleSensibleFracSched = Sched::AddScheduleConstant(*state, "People Sensible Fraction");
    hmZone.peopleRadiantFracSched = Sched::AddScheduleConstant(*state, "People Radiation Fraction");
    hmZone.peopleCO2GenRateSched = Sched::AddScheduleConstant(*state, "People CO2 Gen Rate");
    hmZone.measuredCO2ConcSched->currentVal = 389.795807;
    hmZone.supplyAirCO2ConcSched->currentVal = 387.2253194;
    hmZone.supplyAirMassFlowRateSched->currentVal = 1.427583795;
    hmZone.peopleActivityLevelSched->currentVal = 120;
    hmZone.peopleSensibleFracSched->currentVal = 0.6;
    hmZone.peopleRadiantFracSched->currentVal = 0.3;
    hmZone.peopleCO2GenRateSched->currentVal = 0.0000000382;

    CorrectZoneContaminants(*state, true);
    EXPECT_NEAR(7.27, state->dataHeatBal->Zone(1).NumOccHM, 0.1);
}
