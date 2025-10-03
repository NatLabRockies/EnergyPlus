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

// EnergyPlus::StandardRatings unit tests

// Google test headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/ChillerElectricEIR.hh>
#include <EnergyPlus/ChillerReformulatedEIR.hh>
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/Coils/CoilCoolingDXCurveFitPerformance.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DXCoils.hh>
// #include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/StandardRatings.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>
#include <EnergyPlus/WaterToAirHeatPumpSimple.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::StandardRatings;
using namespace EnergyPlus::Curve;
using namespace EnergyPlus::DXCoils;
using namespace EnergyPlus::ChillerElectricEIR;
using namespace EnergyPlus::ChillerReformulatedEIR;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, SingleSpeedHeatingCoilCurveTest)
{
    // Test that the standard ratings calculation with negative curve value

    using Psychrometrics::PsyRhoAirFnPbTdbW;
    using StandardRatings::SingleSpeedDXHeatingCoilStandardRatings;

    // Set up heating coil and curves.
    int DXCoilNum;
    state->dataDXCoils->NumDXCoils = 1;
    DXCoilNum = 1;
    state->dataDXCoils->DXCoil.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilNumericFields.allocate(1);
    state->dataDXCoils->DXCoilOutletTemp.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilOutletHumRat.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilFanOp.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilPartLoadRatio.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilTotalHeating.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilHeatInletAirDBTemp.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilHeatInletAirWBTemp.allocate(state->dataDXCoils->NumDXCoils);
    DXCoilData &Coil = state->dataDXCoils->DXCoil(DXCoilNum);

    Coil.Name = "DX Single Speed Heating Coil";
    Coil.DXCoilType = "Coil:Heating:DX:SingleSpeed";
    Coil.DXCoilType_Num = HVAC::CoilDX_HeatingEmpirical;
    Coil.availSched = Sched::GetScheduleAlwaysOn(*state);
    Coil.RatedSHR(1) = 1.0;
    Coil.RatedTotCap(1) = 1600.0;
    Coil.RatedCOP(1) = 4.0;
    Coil.RatedEIR(1) = 1 / Coil.RatedCOP(1);
    Coil.RatedAirVolFlowRate(1) = 0.50;
    Coil.RatedAirMassFlowRate(1) =
        Coil.RatedAirVolFlowRate(1) * PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->StdBaroPress, 21.11, 0.00881, "InitDXCoil");
    Coil.FanPowerPerEvapAirFlowRate(1) = 773.3;
    Coil.FanPowerPerEvapAirFlowRate_2023(1) = 934.4;
    Coil.MinOATCompressor = -10.0;
    Coil.CrankcaseHeaterCapacity = 0.0;
    Coil.MaxOATDefrost = 0.0;
    Coil.DefrostStrategy = StandardRatings::DefrostStrat::Resistive;
    Coil.DefrostControl = StandardRatings::HPdefrostControl::Invalid; // timed defrost control type
    Coil.DefrostTime = 0.058333;
    Coil.DefrostCapacity = 1000;
    Coil.PLRImpact = false;
    Coil.FuelType = Constant::eFuel::Electricity;
    Coil.RegionNum = 4;
    Coil.OATempCompressorOn = -5.0;
    Coil.OATempCompressorOnOffBlank = "true";

    auto *curve1 = Curve::AddCurve(*state, "PTHPHeatingCAPFT");
    curve1->curveType = CurveType::Cubic;
    curve1->numDims = 1;
    curve1->Name = curve1->coeff[0] = 0.876825;
    curve1->coeff[1] = -0.002955;
    curve1->coeff[2] = 5.8e-005;
    curve1->coeff[3] = 0.025335;
    curve1->inputLimits[0].min = -5;
    curve1->inputLimits[0].max = 25;

    Coil.CCapFTemp(1) = curve1->Num;

    auto *curve2 = Curve::AddCurve(*state, "HPHeatCapfFF");
    curve2->curveType = CurveType::Quadratic;
    curve2->numDims = 1;
    curve2->Name = curve2->coeff[0] = 1;
    curve2->coeff[1] = 0;
    curve2->coeff[2] = 0;
    curve2->inputLimits[0].min = 0;
    curve2->inputLimits[0].max = 2;
    curve2->outputLimits.min = 0;
    curve2->outputLimits.max = 2;
    Coil.CCapFFlow(1) = curve2->Num;

    auto *curve3 = Curve::AddCurve(*state, "PTHPHeatingEIRFT");
    curve3->curveType = CurveType::Cubic;
    curve3->numDims = 1;
    curve3->coeff[0] = 0.704658;
    curve3->coeff[1] = 0.008767;
    curve3->coeff[2] = 0.000625;
    curve3->coeff[3] = -0.009037;
    curve3->inputLimits[0].min = -5;
    curve3->inputLimits[0].max = 25;
    Coil.EIRFTemp(1) = curve3->Num;

    auto *curve4 = Curve::AddCurve(*state, "HPHeatEIRfFF");
    curve4->curveType = CurveType::Quadratic;
    curve4->numDims = 1;
    curve4->coeff[0] = 1;
    curve4->coeff[1] = 0;
    curve4->coeff[2] = 0;
    curve4->inputLimits[0].min = 0;
    curve4->inputLimits[0].max = 2;
    curve4->outputLimits.min = 0;
    curve4->outputLimits.max = 2;
    Coil.EIRFFlow(1) = curve4->Num;

    int constexpr nPLFfPLR = 5;
    auto *curve5 = Curve::AddCurve(*state, "HPHeatPLFfPLR");
    curve5->curveType = CurveType::Quadratic;
    curve5->numDims = 1;
    curve5->coeff[0] = 1;
    curve5->coeff[1] = 0;
    curve5->coeff[2] = 0;
    curve5->inputLimits[0].min = 0;
    curve5->inputLimits[0].max = 1;
    curve5->outputLimits.min = 0.7;
    curve5->outputLimits.max = 1;
    Coil.PLFFPLR(1) = nPLFfPLR;

    Real64 NetHeatingCapRatedHighTemp;
    Real64 NetHeatingCapRatedLowTemp;
    Real64 HSPF;
    // for HSPF2 2023
    Real64 NetHeatingCapRatedHighTemp_2023;
    Real64 NetHeatingCapRatedLowTemp_2023;
    Real64 HSPF_2023;
    std::map<std::string, Real64> StandardRatingsResults;
    StandardRatingsResults = SingleSpeedDXHeatingCoilStandardRatings(*state,
                                                                     Coil.DXCoilType,
                                                                     Coil.RatedTotCap(1),
                                                                     Coil.RatedCOP(1),
                                                                     Coil.CCapFFlow(1),
                                                                     Coil.CCapFTemp(1),
                                                                     Coil.EIRFFlow(1),
                                                                     Coil.EIRFTemp(1),
                                                                     Coil.RatedAirVolFlowRate(1),
                                                                     Coil.FanPowerPerEvapAirFlowRate(1),
                                                                     Coil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                     Coil.RegionNum,
                                                                     Coil.MinOATCompressor,
                                                                     Coil.OATempCompressorOn,
                                                                     Coil.OATempCompressorOnOffBlank,
                                                                     Coil.DefrostControl);

    NetHeatingCapRatedHighTemp = StandardRatingsResults["NetHeatingCapRated"];
    NetHeatingCapRatedLowTemp = StandardRatingsResults["NetHeatingCapH3Test"];
    HSPF = StandardRatingsResults["HSPF"];
    NetHeatingCapRatedHighTemp_2023 = StandardRatingsResults["NetHeatingCapRated_2023"];
    NetHeatingCapRatedLowTemp_2023 = StandardRatingsResults["NetHeatingCapH3Test_2023"];
    HSPF_2023 = StandardRatingsResults["HSPF2_2023"];
    ASSERT_TRUE(HSPF == HSPF_2023); // 0.0 for a negative Curve

    // evaluate capacity curves
    Real64 TotCapTempModFacRated = CurveValue(*state, Coil.CCapFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempRated);
    Real64 TotCapFlowModFac = CurveValue(*state, Coil.CCapFFlow(1), 1.0);
    Real64 NetHeatingCapRated =
        Coil.RatedTotCap(1) * TotCapTempModFacRated * TotCapFlowModFac + Coil.RatedAirVolFlowRate(1) * Coil.FanPowerPerEvapAirFlowRate(1);
    // check curve values and heating capacity
    EXPECT_GT(TotCapTempModFacRated, 0.0);
    EXPECT_DOUBLE_EQ(TotCapFlowModFac, 1.0);
    EXPECT_DOUBLE_EQ(NetHeatingCapRatedHighTemp, NetHeatingCapRated);
    // evaluate capacity curves
    Real64 CapTempModFacH2Test = CurveValue(*state, Coil.CCapFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempH2Test);
    EXPECT_GT(CapTempModFacH2Test, 0.0);
    Real64 CapTempModFacH3Test = CurveValue(*state, Coil.CCapFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempH3Test);
    // if CapTempModFacH3Test curves value is less than zero, NetHeatingCapRatedLowTemp is set to zero
    EXPECT_LT(CapTempModFacH3Test, 0.0);

    // evaluate capacity curves | HSPF2
    Real64 NetHeatingCapRated2023 =
        Coil.RatedTotCap(1) * TotCapTempModFacRated * TotCapFlowModFac + Coil.RatedAirVolFlowRate(1) * Coil.FanPowerPerEvapAirFlowRate_2023(1);
    EXPECT_DOUBLE_EQ(NetHeatingCapRatedHighTemp_2023, NetHeatingCapRated2023);

    // check heating capacity at low temperature
    EXPECT_DOUBLE_EQ(NetHeatingCapRatedLowTemp, 0.0);
    // evaluate EIR curves
    Real64 EIRTempModFacRated = CurveValue(*state, Coil.EIRFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempRated);
    Real64 EIRTempModFacH2Test = CurveValue(*state, Coil.EIRFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempH2Test);
    Real64 EIRTempModFacH3Test = CurveValue(*state, Coil.EIRFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempH3Test);
    // check EIR curve value
    EXPECT_LT(EIRTempModFacRated, 0.0);
    EXPECT_GT(EIRTempModFacH2Test, 0.0);
    EXPECT_GT(EIRTempModFacH3Test, 0.0);
    // if one of the CAP or EIR curves value is less than zero, then HSPF is set to zero
    EXPECT_DOUBLE_EQ(HSPF, 0.0);

    // check heating capacity at low temperature | HSPF2
    EXPECT_DOUBLE_EQ(NetHeatingCapRatedLowTemp_2023, 0.0);
    // if one of the CAP or EIR curves value is less than zero, then HSPF2/HSPF_2023 is set to zero
    EXPECT_DOUBLE_EQ(HSPF_2023, 0.0);
}

TEST_F(EnergyPlusFixture, SingleSpeedHeatingCoilCurveTest_PositiveCurve)
{
    // Test that the standard ratings calculation with positive curve value

    using Psychrometrics::PsyRhoAirFnPbTdbW;
    using StandardRatings::SingleSpeedDXHeatingCoilStandardRatings;

    // Set up heating coil and curves.
    int DXCoilNum;
    state->dataDXCoils->NumDXCoils = 1;
    DXCoilNum = 1;
    state->dataDXCoils->DXCoil.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilNumericFields.allocate(1);
    state->dataDXCoils->DXCoilOutletTemp.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilOutletHumRat.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilFanOp.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilPartLoadRatio.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilTotalHeating.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilHeatInletAirDBTemp.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilHeatInletAirWBTemp.allocate(state->dataDXCoils->NumDXCoils);
    DXCoilData &Coil = state->dataDXCoils->DXCoil(DXCoilNum);

    Coil.Name = "DX Single Speed Heating Coil";
    Coil.DXCoilType = "Coil:Heating:DX:SingleSpeed";
    Coil.DXCoilType_Num = HVAC::CoilDX_HeatingEmpirical;
    Coil.availSched = Sched::GetScheduleAlwaysOn(*state);
    Coil.RatedSHR(1) = 1.0;
    Coil.RatedTotCap(1) = 1600.0;
    Coil.RatedCOP(1) = 4.0;
    Coil.RatedEIR(1) = 1 / Coil.RatedCOP(1);
    Coil.RatedAirVolFlowRate(1) = 0.50;
    Coil.RatedAirMassFlowRate(1) =
        Coil.RatedAirVolFlowRate(1) * PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->StdBaroPress, 21.11, 0.00881, "InitDXCoil");
    Coil.FanPowerPerEvapAirFlowRate(1) = 773.3;
    Coil.FanPowerPerEvapAirFlowRate_2023(1) = 934.4;
    Coil.MinOATCompressor = -10.0;
    Coil.CrankcaseHeaterCapacity = 0.0;
    Coil.MaxOATDefrost = 0.0;
    Coil.DefrostStrategy = StandardRatings::DefrostStrat::Resistive;
    Coil.DefrostControl = StandardRatings::HPdefrostControl::Invalid; // timed defrost control type
    Coil.DefrostTime = 0.058333;
    Coil.DefrostCapacity = 1000;
    Coil.PLRImpact = false;
    Coil.FuelType = Constant::eFuel::Electricity;
    Coil.RegionNum = 4;
    Coil.OATempCompressorOn = -5.0;
    Coil.OATempCompressorOnOffBlank = "true";

    auto *curve1 = Curve::AddCurve(*state, "PTHPHeatingCAPFT");
    curve1->curveType = CurveType::Cubic;
    curve1->numDims = 1;
    curve1->coeff[0] = 0.876825;
    curve1->coeff[1] = 0.002955; // previously -ve
    curve1->coeff[2] = 5.8e-005;
    curve1->coeff[3] = 0.025335;
    curve1->inputLimits[0].min = 5; // previously -ve
    curve1->inputLimits[0].max = 25;

    Coil.CCapFTemp(1) = curve1->Num;

    auto *curve2 = Curve::AddCurve(*state, "HPHeatCapfFF");
    curve2->curveType = CurveType::Quadratic;
    curve2->numDims = 1;
    curve2->coeff[0] = 1;
    curve2->coeff[1] = 0;
    curve2->coeff[2] = 0;
    curve2->inputLimits[0].min = 0;
    curve2->inputLimits[0].max = 2;
    curve2->outputLimits.min = 0;
    curve2->outputLimits.max = 2;
    Coil.CCapFFlow(1) = curve2->Num;

    auto *curve3 = Curve::AddCurve(*state, "PTHPHeatingEIRFT");
    curve3->curveType = CurveType::Cubic;
    curve3->numDims = 1;
    curve3->coeff[0] = 0.704658;
    curve3->coeff[1] = 0.008767;
    curve3->coeff[2] = 0.000625;
    curve3->coeff[3] = 0.009037;    // previously -ve
    curve3->inputLimits[0].min = 5; // previously -ve
    curve3->inputLimits[0].max = 25;
    Coil.EIRFTemp(1) = curve3->Num;

    auto *curve4 = Curve::AddCurve(*state, "HPHeatEIRfFF");
    curve4->curveType = CurveType::Quadratic;
    curve4->numDims = 1;
    curve4->coeff[0] = 1;
    curve4->coeff[1] = 0;
    curve4->coeff[2] = 0;
    curve4->inputLimits[0].min = 0;
    curve4->inputLimits[0].max = 2;
    curve4->outputLimits.min = 0;
    curve4->outputLimits.max = 2;
    Coil.EIRFFlow(1) = curve4->Num;

    auto *curve5 = Curve::AddCurve(*state, "HPHeatPLFfPLR");
    curve5->curveType = CurveType::Quadratic;
    curve5->numDims = 1;
    curve5->coeff[0] = 1;
    curve5->coeff[1] = 0;
    curve5->coeff[2] = 0;
    curve5->inputLimits[0].min = 0;
    curve5->inputLimits[0].max = 1;
    curve5->outputLimits.min = 0.7;
    curve5->outputLimits.max = 1;
    Coil.PLFFPLR(1) = curve5->Num;

    Real64 NetHeatingCapRatedHighTemp;
    Real64 NetHeatingCapRatedLowTemp;
    Real64 HSPF;
    // for HSPF2 2023
    Real64 NetHeatingCapRatedHighTemp_2023;
    Real64 NetHeatingCapRatedLowTemp_2023;
    Real64 HSPF_2023;
    std::map<std::string, Real64> StandardRatingsResults;
    StandardRatingsResults = SingleSpeedDXHeatingCoilStandardRatings(*state,
                                                                     Coil.DXCoilType,
                                                                     Coil.RatedTotCap(1),
                                                                     Coil.RatedCOP(1),
                                                                     Coil.CCapFFlow(1),
                                                                     Coil.CCapFTemp(1),
                                                                     Coil.EIRFFlow(1),
                                                                     Coil.EIRFTemp(1),
                                                                     Coil.RatedAirVolFlowRate(1),
                                                                     Coil.FanPowerPerEvapAirFlowRate(1),
                                                                     Coil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                     Coil.RegionNum,
                                                                     Coil.MinOATCompressor,
                                                                     Coil.OATempCompressorOn,
                                                                     Coil.OATempCompressorOnOffBlank,
                                                                     Coil.DefrostControl);

    NetHeatingCapRatedHighTemp = StandardRatingsResults["NetHeatingCapRated"];
    NetHeatingCapRatedLowTemp = StandardRatingsResults["NetHeatingCapH3Test"];
    HSPF = StandardRatingsResults["HSPF"];

    NetHeatingCapRatedHighTemp_2023 = StandardRatingsResults["NetHeatingCapRated_2023"];
    NetHeatingCapRatedLowTemp_2023 = StandardRatingsResults["NetHeatingCapH3Test_2023"];
    HSPF_2023 = StandardRatingsResults["HSPF_2023"];
    // ASSERT_TRUE(HSPF < HSPF_2023);
    //??TODO: further extension of this test
    // evaluate capacity curves
    // Real64 TotCapTempModFacRated = CurveValue(*state, Coil.CCapFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempRated);
    // Real64 TotCapFlowModFac = CurveValue(*state, Coil.CCapFFlow(1), 1.0);
    // Real64 NetHeatingCapRated =
    //    Coil.RatedTotCap(1) * TotCapTempModFacRated * TotCapFlowModFac + Coil.RatedAirVolFlowRate(1) * Coil.FanPowerPerEvapAirFlowRate(1);
    //// check curve values and heating capacity
    // EXPECT_GT(TotCapTempModFacRated, 0.0);
    // EXPECT_DOUBLE_EQ(TotCapFlowModFac, 1.0);
    // EXPECT_DOUBLE_EQ(NetHeatingCapRatedHighTemp, NetHeatingCapRated);
    //// evaluate capacity curves
    // Real64 CapTempModFacH2Test = CurveValue(*state, Coil.CCapFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempH2Test);
    // EXPECT_GT(CapTempModFacH2Test, 0.0);
    // Real64 CapTempModFacH3Test = CurveValue(*state, Coil.CCapFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempH3Test);
    //// if CapTempModFacH3Test curves value is less than zero, NetHeatingCapRatedLowTemp is set to zero
    // EXPECT_LT(CapTempModFacH3Test, 0.0);

    //// check heating capacity at low temperature
    // EXPECT_DOUBLE_EQ(NetHeatingCapRatedLowTemp, 0.0);
    //// evaluate EIR curves
    // Real64 EIRTempModFacRated = CurveValue(*state, Coil.EIRFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempRated);
    // Real64 EIRTempModFacH2Test = CurveValue(*state, Coil.EIRFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempH2Test);
    // Real64 EIRTempModFacH3Test = CurveValue(*state, Coil.EIRFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempH3Test);
    //// check EIR curve value
    // EXPECT_LT(EIRTempModFacRated, 0.0);
    // EXPECT_GT(EIRTempModFacH2Test, 0.0);
    // EXPECT_GT(EIRTempModFacH3Test, 0.0);
    //// if one of the CAP or EIR curves value is less than zero, then HSPF is set to zero
    // EXPECT_DOUBLE_EQ(HSPF, 0.0);
}

TEST_F(EnergyPlusFixture, SingleSpeedHeatingCoilCurveTest2023)
{
    // Test that the standard ratings calculation with negative curve value

    using Psychrometrics::PsyRhoAirFnPbTdbW;
    using StandardRatings::SingleSpeedDXHeatingCoilStandardRatings;

    // Set up heating coil and curves.
    int DXCoilNum;
    state->dataDXCoils->NumDXCoils = 1;
    DXCoilNum = 1;
    state->dataDXCoils->DXCoil.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilNumericFields.allocate(1);
    state->dataDXCoils->DXCoilOutletTemp.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilOutletHumRat.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilFanOp.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilPartLoadRatio.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilTotalHeating.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilHeatInletAirDBTemp.allocate(state->dataDXCoils->NumDXCoils);
    state->dataDXCoils->DXCoilHeatInletAirWBTemp.allocate(state->dataDXCoils->NumDXCoils);
    DXCoilData &Coil = state->dataDXCoils->DXCoil(DXCoilNum);

    Coil.Name = "HeatingCoilDXSingleSpeedAutosize";
    Coil.DXCoilType = "Coil:Heating:DX:SingleSpeed";
    Coil.DXCoilType_Num = HVAC::CoilDX_HeatingEmpirical;
    Coil.availSched = Sched::GetScheduleAlwaysOn(*state);
    Coil.RatedSHR(1) = 1.0;
    Coil.RatedTotCap(1) = 1600.0;
    Coil.RatedCOP(1) = 3.8; //
    Coil.RatedEIR(1) = 1 / Coil.RatedCOP(1);
    Coil.RatedAirVolFlowRate(1) = 0.50;
    Coil.RatedAirMassFlowRate(1) =
        Coil.RatedAirVolFlowRate(1) * PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->StdBaroPress, 21.11, 0.00881, "InitDXCoil");
    Coil.FanPowerPerEvapAirFlowRate(1) = 773.3;
    Coil.FanPowerPerEvapAirFlowRate_2023(1) = 934.4;                 //
    Coil.MinOATCompressor = -5;                                      //
    Coil.CrankcaseHeaterCapacity = 200;                              //
    Coil.MaxOATDefrost = 5;                                          //
    Coil.DefrostStrategy = StandardRatings::DefrostStrat::Resistive; //
    Coil.DefrostControl = StandardRatings::HPdefrostControl::Timed;  // timed defrost control type
    Coil.DefrostTime = 0.167;                                        //
    Coil.DefrostCapacity = 20000;                                    //
    Coil.PLRImpact = false;
    Coil.FuelType = Constant::eFuel::Electricity;
    Coil.RegionNum = 4; //
    Coil.OATempCompressorOn = -5.0;
    Coil.OATempCompressorOnOffBlank = "true";

    auto *curve1 = AddCurve(*state, "PTHPHeatingCAPFT"); // Simpl_HPACHeatCapFT_Cubic
    curve1->curveType = CurveType::Cubic;
    curve1->numDims = 1;
    curve1->coeff[0] = 0.759;
    curve1->coeff[1] = 0.028;
    curve1->coeff[2] = 0;
    curve1->coeff[3] = 0;
    curve1->inputLimits[0].min = -20;
    curve1->inputLimits[0].max = 20;

    Coil.CCapFTemp(1) = curve1->Num;

    auto *curve2 = AddCurve(*state, "HPHeatCapfFF"); // Simpl_HPACHeatCapFFF_Cubic
    curve2->curveType = CurveType::Cubic;
    curve2->numDims = 1;
    curve2->coeff[0] = 0.84;
    curve2->coeff[1] = 0.16;
    curve2->coeff[2] = 0;
    curve2->coeff[3] = 0;
    curve2->inputLimits[0].min = 0.5;
    curve2->inputLimits[0].max = 1.5;
    Coil.CCapFFlow(1) = curve2->Num;

    auto *curve3 = AddCurve(*state, "PTHPHeatingEIRFT"); // Simpl_HPACEIRFT_Biquadratic
    curve3->curveType = CurveType::BiQuadratic;
    curve3->numDims = 1;
    curve3->coeff[0] = 0.342;
    curve3->coeff[1] = 0.035;
    curve3->coeff[2] = -0.001;
    curve3->coeff[3] = 0.005;
    curve3->coeff[4] = 0;
    curve3->coeff[5] = -0.001;
    curve3->inputLimits[0].min = 12.778;
    curve3->inputLimits[0].max = 23.889;
    curve3->inputLimits[1].min = 18;
    curve3->inputLimits[1].max = 46.111;
    Coil.EIRFTemp(1) = curve3->Num;

    auto *curve4 = AddCurve(*state, "HPHeatEIRfFF"); // Simpl_HPACHeatEIRFT_Cubic
    curve4->curveType = CurveType::Cubic;
    curve4->numDims = 1;
    curve4->coeff[0] = 1.192;
    curve4->coeff[1] = -0.03;
    curve4->coeff[2] = 0.001;
    curve4->coeff[3] = 0;
    curve4->inputLimits[0].min = -20;
    curve4->inputLimits[0].max = 20;
    curve4->outputLimits.min = -20;
    curve4->outputLimits.max = 20;
    Coil.EIRFFlow(1) = curve4->Num;

    auto *curve5 = AddCurve(*state, "HPHeatPLFfPLR"); // Simpl_HPACCOOLPLFFPLR_Quadratic
    curve5->curveType = CurveType::Quadratic;
    curve5->numDims = 1;
    curve5->coeff[0] = 0.75;
    curve5->coeff[1] = 0.25;
    curve5->coeff[2] = 0;
    curve5->inputLimits[0].min = 0;
    curve5->inputLimits[0].max = 1;
    curve5->outputLimits.min = 0;
    curve5->outputLimits.max = 1;
    Coil.PLFFPLR(1) = curve5->Num;

    Real64 NetHeatingCapRatedHighTemp;
    Real64 NetHeatingCapRatedLowTemp;
    Real64 HSPF;
    // for HSPF2 2023
    Real64 NetHeatingCapRatedHighTemp_2023;
    Real64 NetHeatingCapRatedLowTemp_2023;
    Real64 HSPF_2023;
    std::map<std::string, Real64> StandardRatingsResults;
    StandardRatingsResults = SingleSpeedDXHeatingCoilStandardRatings(*state,
                                                                     Coil.DXCoilType,
                                                                     Coil.RatedTotCap(1),
                                                                     Coil.RatedCOP(1),
                                                                     Coil.CCapFFlow(1),
                                                                     Coil.CCapFTemp(1),
                                                                     Coil.EIRFFlow(1),
                                                                     Coil.EIRFTemp(1),
                                                                     Coil.RatedAirVolFlowRate(1),
                                                                     Coil.FanPowerPerEvapAirFlowRate(1),
                                                                     Coil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                     Coil.RegionNum,
                                                                     Coil.MinOATCompressor,
                                                                     Coil.OATempCompressorOn,
                                                                     Coil.OATempCompressorOnOffBlank,
                                                                     Coil.DefrostControl);

    NetHeatingCapRatedHighTemp = StandardRatingsResults["NetHeatingCapRated"];
    NetHeatingCapRatedLowTemp = StandardRatingsResults["NetHeatingCapH3Test"];
    HSPF = StandardRatingsResults["HSPF"];

    NetHeatingCapRatedHighTemp_2023 = StandardRatingsResults["NetHeatingCapRated_2023"];
    NetHeatingCapRatedLowTemp_2023 = StandardRatingsResults["NetHeatingCapH3Test_2023"];
    HSPF_2023 = StandardRatingsResults["HSPF2_2023"];
    // ASSERT_TRUE(HSPF < HSPF_2023);
    //??TODO: further extension of this test
    //// evaluate capacity curves
    // Real64 TotCapTempModFacRated = CurveValue(*state, Coil.CCapFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempRated);
    // Real64 TotCapFlowModFac = CurveValue(*state, Coil.CCapFFlow(1), 1.0);
    // Real64 NetHeatingCapRated =
    //    Coil.RatedTotCap(1) * TotCapTempModFacRated * TotCapFlowModFac + Coil.RatedAirVolFlowRate(1) * Coil.FanPowerPerEvapAirFlowRate(1);
    //// check curve values and heating capacity
    // EXPECT_GT(TotCapTempModFacRated, 0.0);
    // EXPECT_DOUBLE_EQ(TotCapFlowModFac, 1.0);
    // EXPECT_DOUBLE_EQ(NetHeatingCapRatedHighTemp, NetHeatingCapRated);
    //// evaluate capacity curves
    // Real64 CapTempModFacH2Test = CurveValue(*state, Coil.CCapFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempH2Test);
    // EXPECT_GT(CapTempModFacH2Test, 0.0);
    // Real64 CapTempModFacH3Test = CurveValue(*state, Coil.CCapFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempH3Test);
    //// if CapTempModFacH3Test curves value is less than zero, NetHeatingCapRatedLowTemp is set to zero
    // EXPECT_LT(CapTempModFacH3Test, 0.0);

    //// check heating capacity at low temperature
    // EXPECT_DOUBLE_EQ(NetHeatingCapRatedLowTemp, 0.0);
    //// evaluate EIR curves
    // Real64 EIRTempModFacRated = CurveValue(*state, Coil.EIRFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempRated);
    // Real64 EIRTempModFacH2Test = CurveValue(*state, Coil.EIRFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempH2Test);
    // Real64 EIRTempModFacH3Test = CurveValue(*state, Coil.EIRFTemp(1), StandardRatings::HeatingOutdoorCoilInletAirDBTempH3Test);
    //// check EIR curve value
    // EXPECT_LT(EIRTempModFacRated, 0.0);
    // EXPECT_GT(EIRTempModFacH2Test, 0.0);
    // EXPECT_GT(EIRTempModFacH3Test, 0.0);
    //// if one of the CAP or EIR curves value is less than zero, then HSPF is set to zero
    // EXPECT_DOUBLE_EQ(HSPF, 0.0);
}

TEST_F(EnergyPlusFixture, SingleSpeedHeatingCurveTest2023_II)
{
    std::string const idf_objects1 = delimited_string({
        "Coil:Heating:DX:SingleSpeed,",
        "  Heat Pump 1 HP Heating Coil,                             !- Name",
        "  ,                                                        !- Availability Schedule Name",
        "  9000.0,                                                  !- Rated Total Heating Capacity {W}",
        "  2.75,                                                    !- Rated COP",
        "  0.5,                                                     !- Rated Air Flow Rate {m3/s}",
        "  773.3,                                                   !- 2017 Rated Supply Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  934.4,                                                   !- 2023 Rated Supply Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  Heat Pump 1 Cooling Coil Outlet,                         !- Air Inlet Node Name",
        "  Heat Pump 1 Heating Coil Outlet,                         !- Air Outlet Node Name",
        "  Heat Pump 1 HP Heating Coil Cap-FT,                      !- Total Heating Capacity Function of Temperature Curve Name",
        "  Heat Pump 1 HP Heating Coil Cap-FF,                      !- Total Heating Capacity Function of Flow Fraction Curve Name",
        "  Heat Pump 1 HP Heating Coil EIR-FT,                      !- Energy Input Ratio Function of Temperature Curve Name",
        "  Heat Pump 1 HP Heating Coil EIR-FF,                      !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "  Heat Pump 1 HP Heating Coil PLF,                         !- Part Load Fraction Correlation Curve Name",
        "  Heat Pump 1 HP Heating Coil DefrEIR-FT,                  !- Defrost Energy Input Ratio Function of Temperature Curve Name",
        "  -8,                                                      !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "  ,                                                        !- Outdoor Dry-Bulb Temperature to Turn On Compressor",
        "  5,                                                       !- Maximum Outdoor Dry-Bulb Temperature for Defrost Operation {C}",
        "  0,                                                       !- Crankcase Heater Capacity {W}",
        "  ,                                                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "  0,                                                       !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "  ReverseCycle,                                            !- Defrost Strategy",
        "  Timed,                                                   !- Defrost Control",
        "  0.058333,                                                !- Defrost Time Period Fraction",
        "  10.0;                                                    !- Resistive Defrost Heater Capacity {W}",

        "Curve:Cubic,",
        "  Heat Pump 1 HP Heating Coil Cap-FT,                      !- Name",
        "  0.758746,                                                !- Coefficient1 Constant",
        "  0.027626,                                                !- Coefficient2 x",
        "  0.000148716,                                             !- Coefficient3 x**2",
        "  0.0000034992,                                            !- Coefficient4 x**3",
        "  -20.0,                                                   !- Minimum Value of x",
        "  20.0;                                                    !- Maximum Value of x",

        "Curve:Cubic,",
        "  Heat Pump 1 HP Heating Coil Cap-FF,                      !- Name",
        "  0.84,                                                    !- Coefficient1 Constant",
        "  0.16,                                                    !- Coefficient2 x",
        "  0.0,                                                     !- Coefficient3 x**2",
        "  0.0,                                                     !- Coefficient4 x**3",
        "  0.5,                                                     !- Minimum Value of x",
        "  1.5;                                                     !- Maximum Value of x",

        "Curve:Cubic,",
        "  Heat Pump 1 HP Heating Coil EIR-FT,                      !- Name",
        "  1.19248,                                                 !- Coefficient1 Constant",
        "  -0.0300438,                                              !- Coefficient2 x",
        "  0.00103745,                                              !- Coefficient3 x**2",
        "  -0.000023328,                                            !- Coefficient4 x**3",
        "  -20.0,                                                   !- Minimum Value of x",
        "  20.0;                                                    !- Maximum Value of x",

        "Curve:Quadratic,",
        "  Heat Pump 1 HP Heating Coil EIR-FF,                      !- Name",
        "  1.3824,                                                  !- Coefficient1 Constant",
        "  -0.4336,                                                 !- Coefficient2 x",
        "  0.0512,                                                  !- Coefficient3 x**2",
        "  0.0,                                                     !- Minimum Value of x",
        "  1.0;                                                     !- Maximum Value of x",

        "Curve:Quadratic,",
        "  Heat Pump 1 HP Heating Coil PLF,                         !- Name",
        "  0.75,                                                    !- Coefficient1 Constant",
        "  0.25,                                                    !- Coefficient2 x",
        "  0.0,                                                     !- Coefficient3 x**2",
        "  0.0,                                                     !- Minimum Value of x",
        "  1.0;                                                     !- Maximum Value of x",

        "Curve:Biquadratic,",
        "  Heat Pump 1 HP Heating Coil DefrEIR-FT,                  !- Name",
        "  1,                                                       !- Coefficient1 Constant",
        "  0,                                                       !- Coefficient2 x",
        "  0,                                                       !- Coefficient3 x**2",
        "  0,                                                       !- Coefficient4 y",
        "  0,                                                       !- Coefficient5 y**2",
        "  0,                                                       !- Coefficient6 x*y",
        "  0,                                                       !- Minimum Value of x",
        "  50,                                                      !- Maximum Value of x",
        "  0,                                                       !- Minimum Value of y",
        "  50;                                                      !- Maximum Value of y",
    });

    ASSERT_TRUE(process_idf(idf_objects1));

    state->init_state(*state);

    GetDXCoils(*state);

    auto &Coil(state->dataDXCoils->DXCoil(1));

    Real64 NetHeatingCapRatedHighTemp;
    Real64 NetHeatingCapRatedLowTemp;
    Real64 HSPF;
    // for HSPF2 2023
    Real64 NetHeatingCapRatedHighTemp_2023;
    Real64 NetHeatingCapRatedLowTemp_2023;
    Real64 HSPF_2023;
    std::map<std::string, Real64> StandardRatingsResults;
    StandardRatingsResults = SingleSpeedDXHeatingCoilStandardRatings(*state,
                                                                     Coil.DXCoilType,
                                                                     Coil.RatedTotCap(1),
                                                                     Coil.RatedCOP(1),
                                                                     Coil.CCapFFlow(1),
                                                                     Coil.CCapFTemp(1),
                                                                     Coil.EIRFFlow(1),
                                                                     Coil.EIRFTemp(1),
                                                                     Coil.RatedAirVolFlowRate(1),
                                                                     Coil.FanPowerPerEvapAirFlowRate(1),
                                                                     Coil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                     Coil.RegionNum,
                                                                     Coil.MinOATCompressor,
                                                                     Coil.OATempCompressorOn,
                                                                     Coil.OATempCompressorOnOffBlank,
                                                                     Coil.DefrostControl);

    NetHeatingCapRatedHighTemp = StandardRatingsResults["NetHeatingCapRated"];
    NetHeatingCapRatedLowTemp = StandardRatingsResults["NetHeatingCapH3Test"];
    HSPF = StandardRatingsResults["HSPF"];

    NetHeatingCapRatedHighTemp_2023 = StandardRatingsResults["NetHeatingCapRated_2023"];
    NetHeatingCapRatedLowTemp_2023 = StandardRatingsResults["NetHeatingCapH3Test_2023"];
    HSPF_2023 = StandardRatingsResults["HSPF2_2023"];
}

TEST_F(EnergyPlusFixture, MultiSpeedHeatingCoil_HSPFValueTest_2Speed)
{

    std::string const idf_objects1 = delimited_string({
        " Coil:Heating:DX:MultiSpeed,",
        "   ashp htg coil,                          !- Name",
        "   ,                                       !- Availability Schedule Name",
        "   ashp unitary system Cooling Coil - Heating Coil Node, !- Air Inlet Node Name",
        "   ashp unitary system Heating Coil - Supplemental Coil Node, !- Air Outlet Node Name",
        "   -17.7777777777778,                      !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "   ,                                       !- Outdoor Dry-Bulb Temperature to Turn On Compressor {C}",
        "   50,                                     !- Crankcase Heater Capacity {W}",
        "   ,                                       !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "   10,                                     !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "   DefrostEIR,                             !- Defrost Energy Input Ratio Function of Temperature Curve Name",
        "   4.44444444444444,                       !- Maximum Outdoor Dry-Bulb Temperature for Defrost Operation {C}",
        "   ReverseCycle,                           !- Defrost Strategy",
        "   OnDemand,                               !- Defrost Control",
        "   0.058333,                               !- Defrost Time Period Fraction",
        "   AutoSize,                               !- Resistive Defrost Heater Capacity {W}",
        "   No,                                     !- Apply Part Load Fraction to Speeds Greater than 1",
        "   Electricity,                            !- Fuel Type",
        "   4,                                      !- Region number for Calculating HSPF",
        "   2,                                      !- Number of Speeds",
        "   10128.5361851424,                       !- Speed Gross Rated Heating Capacity 1 {W}",
        "   4.4518131589158,                        !- Speed Gross Rated Heating COP 1 {W/W}",
        "   0.531903646383625,                      !- Speed Rated Air Flow Rate 1 {m3/s}",
        "   773.3,                                  !- 2017 Speed 1 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "   934.3,                                  !- 2023 Speed 1 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}", //??
        "   HP_Heat-Cap-fT1,                        !- Speed Heating Capacity Function of Temperature Curve Name 1",
        "   HP_Heat-CAP-fFF1,                       !- Speed Heating Capacity Function of Flow Fraction Curve Name 1",
        "   HP_Heat-EIR-fT1,                        !- Speed Energy Input Ratio Function of Temperature Curve Name 1",
        "   HP_Heat-EIR-fFF1,                       !- Speed Energy Input Ratio Function of Flow Fraction Curve Name 1",
        "   HP_Heat-PLF-fPLR1,                      !- Speed Part Load Fraction Correlation Curve Name 1",
        "   0.2,                                    !- Speed Rated Waste Heat Fraction of Power Input 1 {dimensionless}",
        "   ConstantBiquadratic,                    !- Speed Waste Heat Function of Temperature Curve Name 1",
        "   14067.4113682534,                       !- Speed Gross Rated Heating Capacity 2 {W}",
        "   3.9871749697327,                        !- Speed Gross Rated Heating COP 2 {W/W}",
        "   0.664879557979531,                      !- Speed Rated Air Flow Rate 2 {m3/s}",
        "   773.3,                                  !- 2017 Speed 2 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "   934.3,                                  !- 2023 Speed 2 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "   HP_Heat-Cap-fT2,                        !- Speed Heating Capacity Function of Temperature Curve Name 2",
        "   HP_Heat-CAP-fFF2,                       !- Speed Heating Capacity Function of Flow Fraction Curve Name 2",
        "   HP_Heat-EIR-fT2,                        !- Speed Energy Input Ratio Function of Temperature Curve Name 2",
        "   HP_Heat-EIR-fFF2,                       !- Speed Energy Input Ratio Function of Flow Fraction Curve Name 2",
        "   HP_Heat-PLF-fPLR2,                      !- Speed Part Load Fraction Correlation Curve Name 2",
        "   0.2,                                    !- Speed Rated Waste Heat Fraction of Power Input 2 {dimensionless}",
        "   ConstantBiquadratic;                    !- Speed Waste Heat Function of Temperature Curve Name 2",

        " Curve:Biquadratic,",
        "   DefrostEIR,                             !- Name",
        "   0.1528,                                 !- Coefficient1 Constant",
        "   0,                                      !- Coefficient2 x",
        "   0,                                      !- Coefficient3 x**2",
        "   0,                                      !- Coefficient4 y",
        "   0,                                      !- Coefficient5 y**2",
        "   0,                                      !- Coefficient6 x*y",
        "   -100,                                   !- Minimum Value of x {BasedOnField A2}",
        "   100,                                    !- Maximum Value of x {BasedOnField A2}",
        "   -100,                                   !- Minimum Value of y {BasedOnField A3}",
        "   100;                                    !- Maximum Value of y {BasedOnField A3}",

        " Curve:Biquadratic,",
        "   HP_Heat-Cap-fT1,                        !- Name",
        "   0.84077409,                             !- Coefficient1 Constant",
        "   -0.0014336586,                          !- Coefficient2 x",
        "   -0.000150336,                           !- Coefficient3 x**2",
        "   0.029628603,                            !- Coefficient4 y",
        "   0.000161676,                            !- Coefficient5 y**2",
        "   -2.349e-005,                            !- Coefficient6 x*y",
        "   -100,                                   !- Minimum Value of x {BasedOnField A2}",
        "   100,                                    !- Maximum Value of x {BasedOnField A2}",
        "   -100,                                   !- Minimum Value of y {BasedOnField A3}",
        "   100;                                    !- Maximum Value of y {BasedOnField A3}",

        " Curve:Quadratic,",
        "   HP_Heat-CAP-fFF1,                       !- Name",
        "   0.741466907,                            !- Coefficient1 Constant",
        "   0.378645444,                            !- Coefficient2 x",
        "   -0.119754733,                           !- Coefficient3 x**2",
        "   0,                                      !- Minimum Value of x {BasedOnField A2}",
        "   2,                                      !- Maximum Value of x {BasedOnField A2}",
        "   0,                                      !- Minimum Curve Output {BasedOnField A3}",
        "   2;                                      !- Maximum Curve Output {BasedOnField A3}",

        " Curve:Biquadratic,",
        "   HP_Heat-EIR-fT1,                        !- Name",
        "   0.539472334,                            !- Coefficient1 Constant",
        "   0.0165103146,                           !- Coefficient2 x",
        "   0.00083874528,                          !- Coefficient3 x**2",
        "   -0.00403234020000001,                   !- Coefficient4 y",
        "   0.00142404156,                          !- Coefficient5 y**2",
        "   -0.00211806252,                         !- Coefficient6 x*y",
        "   -100,                                   !- Minimum Value of x {BasedOnField A2}",
        "   100,                                    !- Maximum Value of x {BasedOnField A2}",
        "   -100,                                   !- Minimum Value of y {BasedOnField A3}",
        "   100;                                    !- Maximum Value of y {BasedOnField A3}",

        " Curve:Quadratic,",
        "   HP_Heat-EIR-fFF1,                       !- Name",
        "   2.153618211,                            !- Coefficient1 Constant",
        "   -1.737190609,                           !- Coefficient2 x",
        "   0.584269478,                            !- Coefficient3 x**2",
        "   0,                                      !- Minimum Value of x {BasedOnField A2}",
        "   2,                                      !- Maximum Value of x {BasedOnField A2}",
        "   0,                                      !- Minimum Curve Output {BasedOnField A3}",
        "   2;                                      !- Maximum Curve Output {BasedOnField A3}",

        " Curve:Quadratic,",
        "   HP_Heat-PLF-fPLR1,                      !- Name",
        "   0.89,                                   !- Coefficient1 Constant",
        "   0.11,                                   !- Coefficient2 x",
        "   0,                                      !- Coefficient3 x**2",
        "   0,                                      !- Minimum Value of x {BasedOnField A2}",
        "   1,                                      !- Maximum Value of x {BasedOnField A2}",
        "   0.7,                                    !- Minimum Curve Output {BasedOnField A3}",
        "   1;                                      !- Maximum Curve Output {BasedOnField A3}",

        " Curve:Biquadratic,",
        "   ConstantBiquadratic,                    !- Name",
        "   1,                                      !- Coefficient1 Constant",
        "   0,                                      !- Coefficient2 x",
        "   0,                                      !- Coefficient3 x**2",
        "   0,                                      !- Coefficient4 y",
        "   0,                                      !- Coefficient5 y**2",
        "   0,                                      !- Coefficient6 x*y",
        "   -100,                                   !- Minimum Value of x {BasedOnField A2}",
        "   100,                                    !- Maximum Value of x {BasedOnField A2}",
        "   -100,                                   !- Minimum Value of y {BasedOnField A3}",
        "   100;                                    !- Maximum Value of y {BasedOnField A3}",

        " Curve:Biquadratic,",
        "   HP_Heat-Cap-fT2,                        !- Name",
        "   0.831506971,                            !- Coefficient1 Constant",
        "   0.0018392166,                           !- Coefficient2 x",
        "   -0.000187596,                           !- Coefficient3 x**2",
        "   0.0266002056,                           !- Coefficient4 y",
        "   0.000191484,                            !- Coefficient5 y**2",
        "   -6.5772e-005,                           !- Coefficient6 x*y",
        "   -100,                                   !- Minimum Value of x {BasedOnField A2}",
        "   100,                                    !- Maximum Value of x {BasedOnField A2}",
        "   -100,                                   !- Minimum Value of y {BasedOnField A3}",
        "   100;                                    !- Maximum Value of y {BasedOnField A3}",

        " Curve:Quadratic,",
        "   HP_Heat-CAP-fFF2,                       !- Name",
        "   0.76634609,                             !- Coefficient1 Constant",
        "   0.32840943,                             !- Coefficient2 x",
        "   -0.094701495,                           !- Coefficient3 x**2",
        "   0,                                      !- Minimum Value of x {BasedOnField A2}",
        "   2,                                      !- Maximum Value of x {BasedOnField A2}",
        "   0,                                      !- Minimum Curve Output {BasedOnField A3}",
        "   2;                                      !- Maximum Curve Output {BasedOnField A3}",

        " Curve:Biquadratic,",
        "   HP_Heat-EIR-fT2,                        !- Name",
        "   0.787746797,                            !- Coefficient1 Constant",
        "   -0.000652314599999999,                  !- Coefficient2 x",
        "   0.00078866784,                          !- Coefficient3 x**2",
        "   -0.0023209056,                          !- Coefficient4 y",
        "   0.00074760408,                          !- Coefficient5 y**2",
        "   -0.00109173096,                         !- Coefficient6 x*y",
        "   -100,                                   !- Minimum Value of x {BasedOnField A2}",
        "   100,                                    !- Maximum Value of x {BasedOnField A2}",
        "   -100,                                   !- Minimum Value of y {BasedOnField A3}",
        "   100;                                    !- Maximum Value of y {BasedOnField A3}",

        " Curve:Quadratic,",
        "   HP_Heat-EIR-fFF2,                       !- Name",
        "   2.001041353,                            !- Coefficient1 Constant",
        "   -1.58869128,                            !- Coefficient2 x",
        "   0.587593517,                            !- Coefficient3 x**2",
        "   0,                                      !- Minimum Value of x {BasedOnField A2}",
        "   2,                                      !- Maximum Value of x {BasedOnField A2}",
        "   0,                                      !- Minimum Curve Output {BasedOnField A3}",
        "   2;                                      !- Maximum Curve Output {BasedOnField A3}",

        " Curve:Quadratic,",
        "   HP_Heat-PLF-fPLR2,                      !- Name",
        "   0.89,                                   !- Coefficient1 Constant",
        "   0.11,                                   !- Coefficient2 x",
        "   0,                                      !- Coefficient3 x**2",
        "   0,                                      !- Minimum Value of x {BasedOnField A2}",
        "   1,                                      !- Maximum Value of x {BasedOnField A2}",
        "   0.7,                                    !- Minimum Curve Output {BasedOnField A3}",
        "   1;                                      !- Maximum Curve Output {BasedOnField A3}",
    });

    ASSERT_TRUE(process_idf(idf_objects1));

    state->init_state(*state);

    GetDXCoils(*state);

    auto &Coil(state->dataDXCoils->DXCoil(1));

    std::map<std::string, Real64> StandardRatingsResult;

    Real64 NetHeatingCapRatedHighTemp(0.0);
    Real64 NetHeatingCapRatedLowTemp(0.0);
    Real64 HSPF(0.0);

    Real64 NetHeatingCapRatedHighTemp_2023(0.0);
    Real64 NetHeatingCapRatedLowTemp_2023(0.0);
    Real64 HSPF2_2023(0.0);

    StandardRatingsResult = MultiSpeedDXHeatingCoilStandardRatings(*state,
                                                                   Coil.Name,          // DXCoilName
                                                                   Coil.DXCoilType,    // DXCoilType,
                                                                   Coil.MSCCapFTemp,   // CapFTempCurveIndex,
                                                                   Coil.MSCCapFFlow,   // CapFFlowCurveIndex,
                                                                   Coil.MSEIRFTemp,    // EIRFTempCurveIndex,
                                                                   Coil.MSEIRFFlow,    // EIRFFlowCurveIndex,
                                                                   Coil.MSPLFFPLR,     // PLFFPLRCurveIndex,
                                                                   Coil.MSRatedTotCap, // RatedTotalCapacity,
                                                                   Coil.MSRatedCOP,
                                                                   Coil.MSRatedAirVolFlowRate,
                                                                   Coil.MSFanPowerPerEvapAirFlowRate,      // FanPowerPerEvapAirFlowRateFromInput,
                                                                   Coil.MSFanPowerPerEvapAirFlowRate_2023, // FanPowerPerEvapAirFlowRateFromInput,
                                                                   Coil.NumOfSpeeds,
                                                                   Coil.RegionNum,
                                                                   Coil.MinOATCompressor,
                                                                   Coil.OATempCompressorOn,
                                                                   Coil.OATempCompressorOnOffBlank,
                                                                   Coil.DefrostControl);

    NetHeatingCapRatedHighTemp = StandardRatingsResult["NetHeatingCapRatedHighTemp"];
    NetHeatingCapRatedLowTemp = StandardRatingsResult["NetHeatingCapRatedLowTemp"];
    HSPF = StandardRatingsResult["HSPF"];

    NetHeatingCapRatedHighTemp_2023 = StandardRatingsResult["NetHeatingCapRatedHighTemp_2023"];
    NetHeatingCapRatedLowTemp_2023 = StandardRatingsResult["NetHeatingCapRatedLowTemp_2023"];
    HSPF2_2023 = StandardRatingsResult["HSPF2_2023"];

    ASSERT_TRUE(HSPF != 0.0);
    ASSERT_TRUE(NetHeatingCapRatedHighTemp != 0.0);
    ASSERT_TRUE(NetHeatingCapRatedLowTemp != 0.0);
    EXPECT_NEAR(1.8604449198065671, HSPF, 0.01);                       // 1.8378611856963720 at 934.3 W/(m3/s)
    EXPECT_NEAR(14723.494682539813, NetHeatingCapRatedHighTemp, 0.01); // 14830.540291374517 at 934.3 W/(m3/s)
    EXPECT_NEAR(8814.4702147982516, NetHeatingCapRatedLowTemp, 0.01);  // 8921.5158236329553 at 934.3 W/(m3/s)
    EXPECT_NEAR(6.35, HSPF * StandardRatings::ConvFromSIToIP, 0.01);   // //6.3481015667753340 at 773.3 W/(m3/s)

    ASSERT_TRUE(HSPF2_2023 != 0.0);                      // 1.6064633535795425
    ASSERT_TRUE(NetHeatingCapRatedHighTemp_2023 != 0.0); // 14830.540291374517
    ASSERT_TRUE(NetHeatingCapRatedLowTemp_2023 != 0.0);  // 6974.9677431654618

    ASSERT_TRUE(HSPF != HSPF2_2023);
}

TEST_F(EnergyPlusFixture, ChillerIPLVTestAirCooled)
{

    using StandardRatings::CalcChillerIPLV;

    // Setup an air-cooled Chiller:Electric:EIR chiller
    state->dataChillerElectricEIR->ElectricEIRChiller.allocate(1);
    state->dataChillerElectricEIR->ElectricEIRChiller(1).Name = "Air Cooled Chiller";
    state->dataChillerElectricEIR->ElectricEIRChiller(1).RefCap = 216000;           // W
    state->dataChillerElectricEIR->ElectricEIRChiller(1).RefCOP = 2.81673861898309; // W/W
    state->dataChillerElectricEIR->ElectricEIRChiller(1).CondenserType = DataPlant::CondenserType::AirCooled;
    state->dataChillerElectricEIR->ElectricEIRChiller(1).MinUnloadRat = 0.15;

    // Cap=f(T)
    auto *curve1 = AddCurve(*state, "AirCooledChillerScrewCmpCapfT");
    curve1->curveType = CurveType::BiQuadratic;
    curve1->numDims = 2;
    curve1->coeff[0] = 0.98898813;
    curve1->coeff[1] = 0.036832851;
    curve1->coeff[2] = 0.000174006;
    curve1->coeff[3] = -0.000275634;
    curve1->coeff[4] = -0.000143667;
    curve1->coeff[5] = -0.000246286;
    curve1->inputLimits[0].min = 4.44;
    curve1->inputLimits[0].max = 10;
    curve1->inputLimits[1].min = 23.89;
    curve1->inputLimits[1].max = 46.11;
    state->dataChillerElectricEIR->ElectricEIRChiller(1).ChillerCapFTIndex = curve1->Num;

    // EIR=f(T)
    auto *curve2 = AddCurve(*state, "AirCooledChillerScrewCmpEIRfT");
    curve2->curveType = CurveType::BiQuadratic;
    curve2->numDims = 2;
    curve2->coeff[0] = 0.814058418;
    curve2->coeff[1] = 0.002335553;
    curve2->coeff[2] = 0.000817786;
    curve2->coeff[3] = -0.017129784;
    curve2->coeff[4] = 0.000773288;
    curve2->coeff[5] = -0.000922024;
    curve2->inputLimits[0].min = 4.44;
    curve2->inputLimits[0].max = 10;
    curve2->inputLimits[1].min = 10;
    curve2->inputLimits[1].max = 46.11;
    state->dataChillerElectricEIR->ElectricEIRChiller(1).ChillerEIRFTIndex = curve2->Num;

    // EIR=f(PLR)
    auto *curve3 = AddCurve(*state, "AirCooledChillerScrewCmpEIRfPLR");
    curve3->curveType = CurveType::Cubic;
    curve3->numDims = 1;
    curve3->coeff[0] = -0.08117804;
    curve3->coeff[1] = 1.433532026;
    curve3->coeff[2] = -0.762289434;
    curve3->coeff[3] = 0.412199944;
    curve3->inputLimits[0].min = 0;
    curve3->inputLimits[0].max = 1;
    state->dataChillerElectricEIR->ElectricEIRChiller(1).ChillerEIRFPLRIndex = curve3->Num;

    Real64 IPLVSI = 0.0;
    Real64 IPLVIP = 0.0;
    CalcChillerIPLV(*state,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).Name,
                    DataPlant::PlantEquipmentType::Chiller_ElectricEIR,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).RefCap,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).RefCOP,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).CondenserType,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).ChillerCapFTIndex,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).ChillerEIRFTIndex,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).ChillerEIRFPLRIndex,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).MinUnloadRat,
                    IPLVSI,
                    IPLVIP,
                    ObjexxFCL::Optional<const Real64>(),
                    ObjexxFCL::Optional_int_const(),
                    ObjexxFCL::Optional<const Real64>());

    EXPECT_DOUBLE_EQ(round(IPLVSI * 100) / 100, 3.88); // 3.88 IPLV.SI (AHRI Std 551/591 Test Condition)
    EXPECT_DOUBLE_EQ(round(IPLVIP * 100) / 100, 3.89); // 13.27 IPLV.IP (AHRI Std 550/590 Test Condition)
}

TEST_F(EnergyPlusFixture, ChillerIPLVTestWaterCooled)
{

    using StandardRatings::CalcChillerIPLV;

    // Setup a water-cooled Chiller:Electric:EIR chiller with reference conditions being at non-rated conditions
    state->dataChillerElectricEIR->ElectricEIRChiller.allocate(1);
    state->dataChillerElectricEIR->ElectricEIRChiller(1).Name = "ElectricEIRChiller McQuay WSC 471kW/5.89COP/Vanes";
    state->dataChillerElectricEIR->ElectricEIRChiller(1).RefCap = 471200; // W
    state->dataChillerElectricEIR->ElectricEIRChiller(1).RefCOP = 5.89;   // W/W
    state->dataChillerElectricEIR->ElectricEIRChiller(1).CondenserType = DataPlant::CondenserType::WaterCooled;
    state->dataChillerElectricEIR->ElectricEIRChiller(1).MinUnloadRat = 0.10;
    state->dataChillerElectricEIR->ElectricEIRChiller(1).MaxPartLoadRat = 1.15;

    // Cap=f(T)
    auto *curve1 = AddCurve(*state, "ElectricEIRChiller McQuay WSC 471kW/5.89COP/Vanes CAPFT");
    curve1->curveType = CurveType::BiQuadratic;
    curve1->numDims = 2;
    curve1->coeff[0] = 2.521130E-01;
    curve1->coeff[1] = 1.324053E-02;
    curve1->coeff[2] = -8.637329E-03;
    curve1->coeff[3] = 8.581056E-02;
    curve1->coeff[4] = -4.261176E-03;
    curve1->coeff[5] = 8.661899E-03;
    curve1->inputLimits[0].min = 7.22;
    curve1->inputLimits[0].max = 12.78;
    curve1->inputLimits[1].min = 12.78;
    curve1->inputLimits[1].max = 26.67;
    state->dataChillerElectricEIR->ElectricEIRChiller(1).ChillerCapFTIndex = curve1->Num;

    // EIR=f(T)
    auto *curve2 = AddCurve(*state, "ElectricEIRChiller McQuay WSC 471kW/5.89COP/Vanes EIRFT");
    curve2->curveType = CurveType::BiQuadratic;
    curve2->numDims = 2;
    curve2->coeff[0] = 4.475238E-01;
    curve2->coeff[1] = -2.588210E-02;
    curve2->coeff[2] = -1.459053E-03;
    curve2->coeff[3] = 4.342595E-02;
    curve2->coeff[4] = -1.000651E-03;
    curve2->coeff[5] = 1.920106E-03;
    curve2->inputLimits[0].min = 7.22;
    curve2->inputLimits[0].max = 12.78;
    curve2->inputLimits[1].min = 12.78;
    curve2->inputLimits[1].max = 26.67;
    state->dataChillerElectricEIR->ElectricEIRChiller(1).ChillerEIRFTIndex = curve2->Num;

    // EIR=f(PLR)
    auto *curve3 = AddCurve(*state, "ElectricEIRChiller McQuay WSC 471kW/5.89COP/Vanes EIRFPLR");
    curve3->curveType = CurveType::Cubic;
    curve3->numDims = 1;
    curve3->coeff[0] = 2.778889E-01;
    curve3->coeff[1] = 2.338363E-01;
    curve3->coeff[2] = 4.883748E-01;
    curve3->inputLimits[0].min = 0;
    curve3->inputLimits[0].max = 1.15;
    state->dataChillerElectricEIR->ElectricEIRChiller(1).ChillerEIRFPLRIndex = curve3->Num;

    Real64 IPLVSI = 0.0;
    Real64 IPLVIP = 0.0;
    CalcChillerIPLV(*state,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).Name,
                    DataPlant::PlantEquipmentType::Chiller_ElectricEIR,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).RefCap,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).RefCOP,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).CondenserType,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).ChillerCapFTIndex,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).ChillerEIRFTIndex,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).ChillerEIRFPLRIndex,
                    state->dataChillerElectricEIR->ElectricEIRChiller(1).MinUnloadRat,
                    IPLVSI,
                    IPLVIP,
                    ObjexxFCL::Optional<const Real64>(),
                    ObjexxFCL::Optional_int_const(),
                    ObjexxFCL::Optional<const Real64>());

    EXPECT_DOUBLE_EQ(round(IPLVSI * 100) / 100, 5.44); // 5.44 IPLV.SI (AHRI Std 551/591 Test Condition)
    EXPECT_DOUBLE_EQ(round(IPLVIP * 100) / 100, 5.47); // 18.66 IPLV.IP (AHRI Std 550/590 Test Condition)
}

TEST_F(EnergyPlusFixture, ChillerIPLVTestWaterCooledReform)
{

    using StandardRatings::CalcChillerIPLV;
    state->init_state(*state);
    // Setup a water-cooled Chiller:Electric:ReformulatedEIR chiller with reference conditions being at non-rated conditions
    state->dataChillerReformulatedEIR->ElecReformEIRChiller.allocate(1);
    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).Name = "ReformEIRChiller McQuay WSC 471kW/5.89COP/Vanes";
    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).RefCap = 471200; // W
    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).RefCOP = 5.89;   // W/W
    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).CondenserType = DataPlant::CondenserType::WaterCooled;
    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).MinUnloadRat = 0.10;
    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).MaxPartLoadRat = 1.08;
    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).CondVolFlowRate = 0.01924;

    // Cap=f(T)
    auto *curve1 = AddCurve(*state, "ReformEIRChiller McQuay WSC 471kW/5.89COP/Vanes CAPFT");
    curve1->curveType = CurveType::BiQuadratic;
    curve1->numDims = 2;
    curve1->coeff[0] = -4.862465E-01;
    curve1->coeff[1] = -7.293218E-02;
    curve1->coeff[2] = -8.514849E-03;
    curve1->coeff[3] = 1.463106E-01;
    curve1->coeff[4] = -4.474066E-03;
    curve1->coeff[5] = 9.813408E-03;
    curve1->inputLimits[0].min = 7.22;
    curve1->inputLimits[0].max = 12.78;
    curve1->inputLimits[1].min = 18.81;
    curve1->inputLimits[1].max = 35.09;
    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).ChillerCapFTIndex = curve1->Num;

    // EIR=f(T)
    auto *curve2 = AddCurve(*state, "ReformEIRChiller McQuay WSC 471kW/5.89COP/Vanes EIRFT");
    curve2->curveType = CurveType::BiQuadratic;
    curve2->numDims = 2;
    curve2->coeff[0] = 3.522647E-01;
    curve2->coeff[1] = -3.311790E-02;
    curve2->coeff[2] = -1.374491E-04;
    curve2->coeff[3] = 3.469525E-02;
    curve2->coeff[4] = -3.624458E-04;
    curve2->coeff[5] = 6.749423E-04;
    curve2->inputLimits[0].min = 7.22;
    curve2->inputLimits[0].max = 12.78;
    curve2->inputLimits[1].min = 18.81;
    curve2->inputLimits[1].max = 35.09;
    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).ChillerEIRFTIndex = curve2->Num;

    // EIR=f(PLR)
    auto *curve3 = AddCurve(*state, "ReformEIRChiller McQuay WSC 471kW/5.89COP/Vanes EIRFPLR");
    curve3->curveType = CurveType::BiCubic;
    curve3->numDims = 2;
    curve3->coeff[0] = 8.215998E-01;
    curve3->coeff[1] = -2.209969E-02;
    curve3->coeff[2] = -1.725652E-05;
    curve3->coeff[3] = -3.831448E-02;
    curve3->coeff[4] = 1.896948E-01;
    curve3->coeff[5] = 2.308518E-02;
    curve3->coeff[6] = 0;
    curve3->coeff[7] = 1.349969E-02;
    curve3->coeff[8] = 0;
    curve3->coeff[9] = 0;
    curve3->inputLimits[0].min = 17.52;
    curve3->inputLimits[0].max = 33.32;
    curve3->inputLimits[1].min = 0.10;
    curve3->inputLimits[1].max = 1.08;
    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).ChillerEIRFPLRIndex = curve3->Num;

    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    auto &loopside(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand));
    loopside.TotalBranches = 1;
    loopside.Branch.allocate(1);
    auto &loopsidebranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
    loopsidebranch.TotalComponents = 1;
    loopsidebranch.Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    Real64 IPLVSI = 0.0;
    Real64 IPLVIP = 0.0;
    CalcChillerIPLV(*state,
                    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).Name,
                    DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR,
                    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).RefCap,
                    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).RefCOP,
                    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).CondenserType,
                    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).ChillerCapFTIndex,
                    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).ChillerEIRFTIndex,
                    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).ChillerEIRFPLRIndex,
                    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).MinUnloadRat,
                    IPLVSI,
                    IPLVIP,
                    state->dataChillerReformulatedEIR->ElecReformEIRChiller(1).CondVolFlowRate,
                    1,
                    1.0);

    EXPECT_DOUBLE_EQ(round(IPLVSI * 100) / 100, 4.83); // 4.83 IPLV.SI (AHRI Std 551/591 Test Condition)
    EXPECT_DOUBLE_EQ(round(IPLVIP * 100) / 100, 4.95); // 16.89 IPLV.IP (AHRI Std 550/590 Test Condition)
}

TEST_F(EnergyPlusFixture, SingleSpeedCoolingCoil_15000W_SameFanPower_SEER2_2023_ValueTest)
{
    // Tests SEER Value calculation for single speed DX cooling coil
    // using user's PLF curve and AHRI default PLF curve

    std::string const idf_objects = delimited_string({
        "   Coil:Cooling:DX:SingleSpeed,",
        "     Heat Pump ACDXCoil,      !- Name",
        "     ,                        !- Availability Schedule Name",
        "     autosize,                !- Gross Rated Total Cooling Capacity {W}",
        "     autosize,                !- Gross Rated Sensible Heat Ratio",
        "     4.0,                     !- Gross Rated Cooling COP {W/W}",
        "     autosize,                !- Rated Air Flow Rate {m3/s}",
        "     495.0,                   !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "     495.0,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "     DX Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "     Heating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "     HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "     HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "     HPACCOOLEIRFT,           !- Energy Input Ratio Function of Temperature Curve Name",
        "     HPACCOOLEIRFFF,          !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "     HPACCOOLPLFFPLR;         !- Part Load Fraction Correlation Curve Name",

        "   Curve:Biquadratic,",
        "     HPACCoolCapFT,           !- Name",
        "     0.766956,                !- Coefficient1 Constant",
        "     0.0107756,               !- Coefficient2 x",
        "    -0.0000414703,            !- Coefficient3 x**2",
        "     0.00134961,              !- Coefficient4 y",
        "    -0.000261144,             !- Coefficient5 y**2",
        "     0.000457488,             !- Coefficient6 x*y",
        "     12.77778,                !- Minimum Value of x",
        "     23.88889,                !- Maximum Value of x",
        "     18.0,                    !- Minimum Value of y",
        "     46.11111,                !- Maximum Value of y",
        "     ,                        !- Minimum Curve Output",
        "     ,                        !- Maximum Curve Output",
        "     Temperature,             !- Input Unit Type for X",
        "     Temperature,             !- Input Unit Type for Y",
        "     Dimensionless;           !- Output Unit Type",

        "   Curve:Biquadratic,",
        "     HPACCOOLEIRFT,           !- Name",
        "     0.297145,                !- Coefficient1 Constant",
        "     0.0430933,               !- Coefficient2 x",
        "    -0.000748766,             !- Coefficient3 x**2",
        "     0.00597727,              !- Coefficient4 y",
        "     0.000482112,             !- Coefficient5 y**2",
        "    -0.000956448,             !- Coefficient6 x*y",
        "     12.77778,                !- Minimum Value of x",
        "     23.88889,                !- Maximum Value of x",
        "     18.0,                    !- Minimum Value of y",
        "     46.11111,                !- Maximum Value of y",
        "     ,                        !- Minimum Curve Output",
        "     ,                        !- Maximum Curve Output",
        "     Temperature,             !- Input Unit Type for X",
        "     Temperature,             !- Input Unit Type for Y",
        "     Dimensionless;           !- Output Unit Type",

        "   Curve:Cubic,",
        "     HPACHeatCapFT,           !- Name",
        "     0.758746,                !- Coefficient1 Constant",
        "     0.027626,                !- Coefficient2 x",
        "     0.000148716,             !- Coefficient3 x**2",
        "     0.0000034992,            !- Coefficient4 x**3",
        "     -20.0,                   !- Minimum Value of x",
        "     20.0,                    !- Maximum Value of x",
        "     ,                        !- Minimum Curve Output",
        "     ,                        !- Maximum Curve Output",
        "     Temperature,             !- Input Unit Type for X",
        "     Dimensionless;           !- Output Unit Type",

        "   Curve:Cubic,",
        "     HPACHeatCapFFF,          !- Name",
        "     0.84,                    !- Coefficient1 Constant",
        "     0.16,                    !- Coefficient2 x",
        "     0.0,                     !- Coefficient3 x**2",
        "     0.0,                     !- Coefficient4 x**3",
        "     0.5,                     !- Minimum Value of x",
        "     1.5;                     !- Maximum Value of x",

        "   Curve:Cubic,",
        "     HPACHeatEIRFT,           !- Name",
        "     1.19248,                 !- Coefficient1 Constant",
        "    -0.0300438,               !- Coefficient2 x",
        "     0.00103745,              !- Coefficient3 x**2",
        "    -0.000023328,             !- Coefficient4 x**3",
        "    -20.0,                    !- Minimum Value of x",
        "     20.0,                    !- Maximum Value of x",
        "     ,                        !- Minimum Curve Output",
        "     ,                        !- Maximum Curve Output",
        "     Temperature,             !- Input Unit Type for X",
        "     Dimensionless;           !- Output Unit Type",

        "   Curve:Quadratic,",
        "     HPACCoolCapFFF,          !- Name",
        "     0.8,                     !- Coefficient1 Constant",
        "     0.2,                     !- Coefficient2 x",
        "     0.0,                     !- Coefficient3 x**2",
        "     0.5,                     !- Minimum Value of x",
        "     1.5;                     !- Maximum Value of x",

        "   Curve:Quadratic,",
        "     HPACCOOLEIRFFF,          !- Name",
        "     1.156,                   !- Coefficient1 Constant",
        "    -0.1816,                  !- Coefficient2 x",
        "     0.0256,                  !- Coefficient3 x**2",
        "     0.5,                     !- Minimum Value of x",
        "     1.5;                     !- Maximum Value of x",

        "   Curve:Linear,",
        "     HPACCOOLPLFFPLR,         !- Name",
        "     0.90,                    !- Coefficient1 Constant",
        "     0.10,                    !- Coefficient2 x",
        "     0.0,                     !- Minimum Value of x",
        "     1.0;                     !- Maximum Value of x",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.PLFFPLR(1)));
    // ckeck user PLF curve coefficients
    //?? Default PLF cofficients source ?
    EXPECT_EQ(0.90, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.10, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.PLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Test 1: user PLF curve is different from the AHRI Std 210/240-2008 default PLF Curve
    Array1D<Real64> NetCoolingCapRated(1);
    Real64 SEER_User(0.0);
    Real64 SEER_Standard(0.0);
    Real64 SEER2_User(0.0);
    Real64 SEER2_Standard(0.0);
    Real64 EER(0.0);
    Real64 IEER(0.0);
    NetCoolingCapRated = 0.0;
    // set rated capacity and flow rate
    //?? Did these come from 2008 standard Test condition or
    thisCoil.RatedTotCap(1) = 15000.0;
    thisCoil.RatedAirVolFlowRate(1) = 0.8;

    EXPECT_EQ(495.0, thisCoil.FanPowerPerEvapAirFlowRate_2023(1));
    std::map<std::string, Real64> StandarRatingResults;
    // calculate standard ratings
    StandarRatingResults = SingleSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.Name,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.CCapFTemp(1),
                                                                   thisCoil.CCapFFlow(1),
                                                                   thisCoil.EIRFTemp(1),
                                                                   thisCoil.EIRFFlow(1),
                                                                   thisCoil.PLFFPLR(1),
                                                                   thisCoil.RatedTotCap(1),
                                                                   thisCoil.RatedCOP(1),
                                                                   thisCoil.RatedAirVolFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                   thisCoil.CondenserType(1)
                                                                   //,
                                                                   // NetCoolingCapRated(1),
                                                                   // SEER_User,     // 3.81
                                                                   // SEER_Standard, // 3.51
                                                                   // EER,
                                                                   // IEER,
                                                                   // SEER2_User,      // 3.81
                                                                   // SEER2_Standard // 3.61
    );

    SEER_User = StandarRatingResults["SEER_User"];
    SEER_Standard = StandarRatingResults["SEER_Standard"];
    SEER2_User = StandarRatingResults["SEER2_User"];
    SEER2_Standard = StandarRatingResults["SEER2_Standard"];
    // check SEER values calculated using user PLF and default PLF curve
    EXPECT_NEAR(12.96, SEER_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(11.94, SEER_Standard * StandardRatings::ConvFromSIToIP, 0.01);
    // auto a = SEER2_User * StandardRatings::ConvFromSIToIP;
    // auto b = SEER2_Standard * StandardRatings::ConvFromSIToIP;
    EXPECT_NEAR(12.96, SEER2_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(12.28, SEER2_Standard * StandardRatings::ConvFromSIToIP, 0.01);

    // Test 2: user PLF curve is the same as the AHRI Std 210/240-2008 default PLF Curve
    // reset the user PLF curve to the AHRI Std 210/240-2008 default PLF curve
    // AHRI Std 210/240-2008 default PLF curve is linear equation, PLF = a + b * PLR
    thisCoolPLFfPLR->coeff[0] = 0.75;          // = a 0.8
    thisCoolPLFfPLR->coeff[1] = 0.25;          // = b 0.2
    thisCoolPLFfPLR->inputLimits[0].min = 0.0; // PLR minimum value allowed by the PLF curve
    thisCoolPLFfPLR->inputLimits[0].max = 1.0; // PLR maximum value allowed by the PLF curve
    // reset output variables
    SEER_User = 0.0;
    SEER_Standard = 0.0;
    SEER2_User = 0.0;
    SEER2_Standard = 0.0;
    EER = 0.0;
    IEER = 0.0;
    NetCoolingCapRated = 0.0;
    StandarRatingResults.clear();
    // rerun the standard ratings calculation
    StandarRatingResults = SingleSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.Name,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.CCapFTemp(1),
                                                                   thisCoil.CCapFFlow(1),
                                                                   thisCoil.EIRFTemp(1),
                                                                   thisCoil.EIRFFlow(1),
                                                                   thisCoil.PLFFPLR(1),
                                                                   thisCoil.RatedTotCap(1),
                                                                   thisCoil.RatedCOP(1),
                                                                   thisCoil.RatedAirVolFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                   thisCoil.CondenserType(1) //,
                                                                                             // NetCoolingCapRated(1),
                                                                                             // SEER_User,     // 3.51
                                                                                             // SEER_Standard, // 3.51
                                                                                             // EER,
                                                                                             // IEER,
                                                                                             // SEER2_User,    // 3.51
                                                                                             // SEER2_Standard // 3.61
    );
    SEER_User = StandarRatingResults["SEER_User"];
    SEER_Standard = StandarRatingResults["SEER_Standard"];
    SEER2_User = StandarRatingResults["SEER2_User"];
    SEER2_Standard = StandarRatingResults["SEER2_Standard"];
    // SEER and SEER_Default must match for the same PLF curve
    EXPECT_DOUBLE_EQ(SEER_User, SEER_Standard);
    EXPECT_NEAR(11.94, SEER_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(11.94, SEER_Standard * StandardRatings::ConvFromSIToIP, 0.01);

    // Test 3: user PLF curve is the same as the AHRI Std 210/240-2023 default PLF Curve
    // reset the user PLF curve to the AHRI Std 210/240-2008 default PLF curve
    // AHRI Std 210/240-2023 default PLF curve is linear equation, PLF = a + b * PLR
    thisCoolPLFfPLR->coeff[0] = 0.80;          //
    thisCoolPLFfPLR->coeff[1] = 0.20;          //
    thisCoolPLFfPLR->inputLimits[0].min = 0.0; // PLR minimum value allowed by the PLF curve
    thisCoolPLFfPLR->inputLimits[0].max = 1.0; // PLR maximum value allowed by the PLF curve
    // reset output variables
    SEER_User = 0.0;
    SEER_Standard = 0.0;
    SEER2_User = 0.0;
    SEER2_Standard = 0.0;
    EER = 0.0;
    IEER = 0.0;
    NetCoolingCapRated = 0.0;
    StandarRatingResults.clear();
    // rerun the standard ratings calculation for PLF Curve (AHRI 2023)
    StandarRatingResults = SingleSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.Name,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.CCapFTemp(1),
                                                                   thisCoil.CCapFFlow(1),
                                                                   thisCoil.EIRFTemp(1),
                                                                   thisCoil.EIRFFlow(1),
                                                                   thisCoil.PLFFPLR(1),
                                                                   thisCoil.RatedTotCap(1),
                                                                   thisCoil.RatedCOP(1),
                                                                   thisCoil.RatedAirVolFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                   thisCoil.CondenserType(1) //,
                                                                                             // NetCoolingCapRated(1),
                                                                                             // SEER_User,
                                                                                             // SEER_Standard,
                                                                                             // EER,
                                                                                             // IEER,
                                                                                             // SEER2_User,    // 3.61
                                                                                             // SEER2_Standard // 3.61
    );
    SEER_User = StandarRatingResults["SEER_User"];
    SEER_Standard = StandarRatingResults["SEER_Standard"];
    SEER2_User = StandarRatingResults["SEER2_User"];
    SEER2_Standard = StandarRatingResults["SEER2_Standard"];

    // SEER2_USER and SEER2_Default/Standard must match for the same PLF curve
    // auto c = SEER2_User * StandardRatings::ConvFromSIToIP;
    // auto d = SEER2_Standard * StandardRatings::ConvFromSIToIP;
    EXPECT_DOUBLE_EQ(SEER2_User, SEER2_Standard);
    EXPECT_NEAR(12.28, SEER2_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(12.28, SEER2_Standard * StandardRatings::ConvFromSIToIP, 0.01);

    // Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)} for both 2017 & 2023
    // are same (495) so the NetCoolingCapacity, EER and IEER for both 2017 & 2023 standard
    // should match
    EXPECT_DOUBLE_EQ(StandarRatingResults["EER_2022"], StandarRatingResults["EER"]);
    EXPECT_NEAR(3.51, StandarRatingResults["EER"], 0.01);
    EXPECT_NEAR(3.51, StandarRatingResults["EER_2022"], 0.01);
    EXPECT_NEAR(3.6342576029986851, StandarRatingResults["IEER"], 0.01);
    EXPECT_NEAR(3.6342083696690421, StandarRatingResults["IEER_2022"], 0.01);
    EXPECT_NEAR(14594.568882508802, StandarRatingResults["NetCoolingCapRated"], 0.01);
    EXPECT_NEAR(14594.568882508802, StandarRatingResults["NetCoolingCapRated2023"], 0.01);
}

TEST_F(EnergyPlusFixture, SingleSpeedCoolingCoil_9000W_SEER2_2023_ValueTest)
{
    // !- ### Taken from AirflowNetwork_MultiAirLoops.idf ###
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:SingleSpeed,",
        "    Heat Pump 1 Cooling Coil,!- Name",
        "    ,                        !- Availability Schedule Name",
        "    9000.0,                  !- Gross Rated Total Cooling Capacity {W}",
        "    0.8,                     !- Gross Rated Sensible Heat Ratio",
        "    3,                       !- Gross Rated Cooling COP {W/W}",
        "    0.5,                     !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    934.4,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    Heat Pump 1 Supply Fan Outlet,  !- Air Inlet Node Name",
        "    Heat Pump 1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    Heat Pump 1 Cool Coil Cap-FT,  !- Total Cooling Capacity Function of Temperature Curve Name",
        "    Heat Pump 1 Cool Coil Cap-FF,  !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    Heat Pump 1 Cool Coil EIR-FT,  !- Energy Input Ratio Function of Temperature Curve Name",
        "    Heat Pump 1 Cool Coil EIR-FF,  !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    Heat Pump 1 Cool Coil PLF,  !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    0,                       !- Nominal Time for Condensate Removal to Begin {s}",
        "    0,                       !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    0,                       !- Maximum Cycling Rate {cycles/hr}",
        "    0,                       !- Latent Capacity Time Constant {s}",
        "    Heat Pump 1 Cooling Coil Condenser Inlet,  !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    0,                       !- Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                        !- Evaporative Condenser Air Flow Rate {m3/s}",
        "    0,                       !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    0,                       !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10;                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "",
        "  Curve:Biquadratic,",
        "    Heat Pump 1 Cool Coil Cap-FT,  !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.00068377,              !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.00000972,             !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111;                !- Maximum Value of y",
        "",
        "  Curve:Quadratic,",
        "    Heat Pump 1 Cool Coil Cap-FF,  !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    Heat Pump 1 Cool Coil EIR-FT,  !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.0006237,              !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111;                !- Maximum Value of y",
        "",
        "  Curve:Quadratic,",
        "    Heat Pump 1 Cool Coil EIR-FF,  !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Quadratic,",
        "    Heat Pump 1 Cool Coil PLF,  !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0,                       !- Minimum Value of x",
        "    1;                       !- Maximum Value of x",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.PLFFPLR(1)));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.PLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Test 1: user PLF curve is different from the AHRI Std 210/240-2008 default PLF Curve
    Array1D<Real64> NetCoolingCapRated(1);
    Real64 SEER_User(0.0);
    Real64 SEER_Standard(0.0);
    Real64 SEER2_User(0.0);
    Real64 SEER2_Standard(0.0);
    Real64 EER(0.0);
    Real64 IEER(0.0);
    NetCoolingCapRated = 0.0;
    // set rated capacity and flow rate
    // thisCoil.RatedTotCap(1) = 25000.0;
    // thisCoil.RatedAirVolFlowRate(1) = 1.300;

    EXPECT_EQ(934.4, thisCoil.FanPowerPerEvapAirFlowRate_2023(1));
    std::map<std::string, Real64> StandarRatingResults;
    // calculate standard ratings
    StandarRatingResults = SingleSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.Name,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.CCapFTemp(1),
                                                                   thisCoil.CCapFFlow(1),
                                                                   thisCoil.EIRFTemp(1),
                                                                   thisCoil.EIRFFlow(1),
                                                                   thisCoil.PLFFPLR(1),
                                                                   thisCoil.RatedTotCap(1),
                                                                   thisCoil.RatedCOP(1),
                                                                   thisCoil.RatedAirVolFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                   thisCoil.CondenserType(1)
                                                                   //,
                                                                   // NetCoolingCapRated(1),
                                                                   // SEER_User,     // 3.81
                                                                   // SEER_Standard, // 3.51
                                                                   // EER,
                                                                   // IEER,
                                                                   // SEER2_User,      // 3.81
                                                                   // SEER2_Standard // 3.61
    );

    SEER_User = StandarRatingResults["SEER_User"];
    SEER_Standard = StandarRatingResults["SEER_Standard"];
    SEER2_User = StandarRatingResults["SEER2_User"];
    SEER2_Standard = StandarRatingResults["SEER2_Standard"];
    // check SEER values calculated using user PLF and default PLF curve
    EXPECT_NEAR(9.20, SEER_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(8.70, SEER_Standard * StandardRatings::ConvFromSIToIP, 0.01);
    // auto a = SEER2_User * StandardRatings::ConvFromSIToIP;
    // auto b = SEER2_Standard * StandardRatings::ConvFromSIToIP;
    EXPECT_NEAR(8.90, SEER2_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(8.66, SEER2_Standard * StandardRatings::ConvFromSIToIP, 0.01);

    // Test 2: user PLF curve is the same as the AHRI Std 210/240-2008 default PLF Curve
    // reset the user PLF curve to the AHRI Std 210/240-2008 default PLF curve
    // AHRI Std 210/240-2008 default PLF curve is linear equation, PLF = a + b * PLR
    thisCoolPLFfPLR->coeff[0] = 0.75;          // = a 0.8
    thisCoolPLFfPLR->coeff[1] = 0.25;          // = b 0.2
    thisCoolPLFfPLR->inputLimits[0].min = 0.0; // PLR minimum value allowed by the PLF curve
    thisCoolPLFfPLR->inputLimits[0].max = 1.0; // PLR maximum value allowed by the PLF curve
    // reset output variables
    SEER_User = 0.0;
    SEER_Standard = 0.0;
    SEER2_User = 0.0;
    SEER2_Standard = 0.0;
    EER = 0.0;
    IEER = 0.0;
    NetCoolingCapRated = 0.0;
    StandarRatingResults.clear();
    // rerun the standard ratings calculation
    StandarRatingResults = SingleSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.Name,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.CCapFTemp(1),
                                                                   thisCoil.CCapFFlow(1),
                                                                   thisCoil.EIRFTemp(1),
                                                                   thisCoil.EIRFFlow(1),
                                                                   thisCoil.PLFFPLR(1),
                                                                   thisCoil.RatedTotCap(1),
                                                                   thisCoil.RatedCOP(1),
                                                                   thisCoil.RatedAirVolFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                   thisCoil.CondenserType(1) //,
                                                                                             // NetCoolingCapRated(1),
                                                                                             // SEER_User,     // 3.51
                                                                                             // SEER_Standard, // 3.51
                                                                                             // EER,
                                                                                             // IEER,
                                                                                             // SEER2_User,    // 3.51
                                                                                             // SEER2_Standard // 3.61
    );
    SEER_User = StandarRatingResults["SEER_User"];
    SEER_Standard = StandarRatingResults["SEER_Standard"];
    SEER2_User = StandarRatingResults["SEER2_User"];
    SEER2_Standard = StandarRatingResults["SEER2_Standard"];
    // SEER and SEER_Default must match for the same PLF curve
    EXPECT_DOUBLE_EQ(SEER_User, SEER_Standard);
    EXPECT_NEAR(8.70, SEER_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(8.70, SEER_Standard * StandardRatings::ConvFromSIToIP, 0.01);

    // Test 3: user PLF curve is the same as the AHRI Std 210/240-2023 default PLF Curve
    // reset the user PLF curve to the AHRI Std 210/240-2008 default PLF curve
    // AHRI Std 210/240-2023 default PLF curve is linear equation, PLF = a + b * PLR
    thisCoolPLFfPLR->coeff[0] = 0.80;          //
    thisCoolPLFfPLR->coeff[1] = 0.20;          //
    thisCoolPLFfPLR->inputLimits[0].min = 0.0; // PLR minimum value allowed by the PLF curve
    thisCoolPLFfPLR->inputLimits[0].max = 1.0; // PLR maximum value allowed by the PLF curve
    // reset output variables
    SEER_User = 0.0;
    SEER_Standard = 0.0;
    SEER2_User = 0.0;
    SEER2_Standard = 0.0;
    EER = 0.0;
    IEER = 0.0;
    NetCoolingCapRated = 0.0;
    StandarRatingResults.clear();
    // rerun the standard ratings calculation for PLF Curve (AHRI 2023)
    StandarRatingResults = SingleSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.Name,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.CCapFTemp(1),
                                                                   thisCoil.CCapFFlow(1),
                                                                   thisCoil.EIRFTemp(1),
                                                                   thisCoil.EIRFFlow(1),
                                                                   thisCoil.PLFFPLR(1),
                                                                   thisCoil.RatedTotCap(1),
                                                                   thisCoil.RatedCOP(1),
                                                                   thisCoil.RatedAirVolFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                   thisCoil.CondenserType(1) //,
                                                                                             // NetCoolingCapRated(1),
                                                                                             // SEER_User,
                                                                                             // SEER_Standard,
                                                                                             // EER,
                                                                                             // IEER,
                                                                                             // SEER2_User,    // 3.61
                                                                                             // SEER2_Standard // 3.61
    );
    SEER_User = StandarRatingResults["SEER_User"];
    SEER_Standard = StandarRatingResults["SEER_Standard"];
    SEER2_User = StandarRatingResults["SEER2_User"];
    SEER2_Standard = StandarRatingResults["SEER2_Standard"];

    // SEER2_USER and SEER2_Default/Standard must match for the same PLF curve
    // auto c = SEER2_User * StandardRatings::ConvFromSIToIP;
    // auto d = SEER2_Standard * StandardRatings::ConvFromSIToIP;
    EXPECT_DOUBLE_EQ(SEER2_User, SEER2_Standard);
    EXPECT_NEAR(8.66, SEER2_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(8.66, SEER2_Standard * StandardRatings::ConvFromSIToIP, 0.01);

    EXPECT_NEAR(2.54, StandarRatingResults["EER"], 0.01);
    EXPECT_NEAR(2.46, StandarRatingResults["EER_2022"], 0.01);
    EXPECT_NEAR(2.60, StandarRatingResults["IEER"], 0.01);
    EXPECT_NEAR(2.47, StandarRatingResults["IEER_2022"], 0.01);
    EXPECT_NEAR(8611.903861568002, StandarRatingResults["NetCoolingCapRated"], 0.01);
    EXPECT_NEAR(8531.3538615680009, StandarRatingResults["NetCoolingCapRated2023"], 0.01);
}

TEST_F(EnergyPlusFixture, SingleSpeedCoolingCoil_18000W_SEER2_2023_ValueTest)
{
    // !- ## Take from: AirflowNetwork_MultiZone_House_OvercoolDehumid.idf ##

    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:SingleSpeed,",
        "    ACDXCoil 1,              !- Name",
        "    HVACAvailSched,          !- Availability Schedule Name",
        "    18000.0,                 !- Gross Rated Total Cooling Capacity {W}",
        "    Autosize,                !- Gross Rated Sensible Heat Ratio",
        "    4.49,                    !- Gross Rated Cooling COP {W/W}",
        "    1.00,                    !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    934.4,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    Air Loop Inlet Node,     !- Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "    CentralACCoolCapFT,      !- Total Cooling Capacity Function of Temperature Curve Name",
        "    CentralACCoolCapFFF,     !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    CentralACEIRFT,          !- Energy Input Ratio Function of Temperature Curve Name",
        "    CentralACEIRFFF,         !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    CentralACPLFFPLR;        !- Part Load Fraction Correlation Curve Name",
        "",
        "  Schedule:Compact,",
        "    HVACAvailSched,          !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",
        "",
        "  Curve:Biquadratic,",
        "    CentralACCoolCapFT,      !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    CentralACCoolCapFFF,     !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    CentralACEIRFT,          !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    CentralACEIRFFF,         !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Quadratic,",
        "    CentralACPLFFPLR,        !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.PLFFPLR(1)));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.PLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Test 1: user PLF curve is different from the AHRI Std 210/240-2008 default PLF Curve
    Array1D<Real64> NetCoolingCapRated(1);
    Real64 SEER_User(0.0);
    Real64 SEER_Standard(0.0);
    Real64 SEER2_User(0.0);
    Real64 SEER2_Standard(0.0);
    Real64 EER(0.0);
    Real64 IEER(0.0);
    NetCoolingCapRated = 0.0;

    EXPECT_EQ(934.4, thisCoil.FanPowerPerEvapAirFlowRate_2023(1));
    std::map<std::string, Real64> StandarRatingResults;
    // calculate standard ratings
    StandarRatingResults = SingleSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.Name,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.CCapFTemp(1),
                                                                   thisCoil.CCapFFlow(1),
                                                                   thisCoil.EIRFTemp(1),
                                                                   thisCoil.EIRFFlow(1),
                                                                   thisCoil.PLFFPLR(1),
                                                                   thisCoil.RatedTotCap(1),
                                                                   thisCoil.RatedCOP(1),
                                                                   thisCoil.RatedAirVolFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                   thisCoil.CondenserType(1)
                                                                   //,
                                                                   // NetCoolingCapRated(1),
                                                                   // SEER_User,     // 3.81
                                                                   // SEER_Standard, // 3.51
                                                                   // EER,
                                                                   // IEER,
                                                                   // SEER2_User,      // 3.81
                                                                   // SEER2_Standard // 3.61
    );

    SEER_User = StandarRatingResults["SEER_User"];
    SEER_Standard = StandarRatingResults["SEER_Standard"];
    SEER2_User = StandarRatingResults["SEER2_User"];
    SEER2_Standard = StandarRatingResults["SEER2_Standard"];
    // check SEER values calculated using user PLF and default PLF curve
    EXPECT_NEAR(12.99, SEER_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(12.29, SEER_Standard * StandardRatings::ConvFromSIToIP, 0.01);
    // auto a = SEER2_User * StandardRatings::ConvFromSIToIP;
    // auto b = SEER2_Standard * StandardRatings::ConvFromSIToIP;
    EXPECT_NEAR(12.44, SEER2_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(12.10, SEER2_Standard * StandardRatings::ConvFromSIToIP, 0.01);

    // Test 2: user PLF curve is the same as the AHRI Std 210/240-2008 default PLF Curve
    // reset the user PLF curve to the AHRI Std 210/240-2008 default PLF curve
    // AHRI Std 210/240-2008 default PLF curve is linear equation, PLF = a + b * PLR
    thisCoolPLFfPLR->coeff[0] = 0.75;          // = a 0.8
    thisCoolPLFfPLR->coeff[1] = 0.25;          // = b 0.2
    thisCoolPLFfPLR->inputLimits[0].min = 0.0; // PLR minimum value allowed by the PLF curve
    thisCoolPLFfPLR->inputLimits[0].max = 1.0; // PLR maximum value allowed by the PLF curve
    // reset output variables
    SEER_User = 0.0;
    SEER_Standard = 0.0;
    SEER2_User = 0.0;
    SEER2_Standard = 0.0;
    EER = 0.0;
    IEER = 0.0;
    NetCoolingCapRated = 0.0;
    StandarRatingResults.clear();
    // rerun the standard ratings calculation
    StandarRatingResults = SingleSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.Name,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.CCapFTemp(1),
                                                                   thisCoil.CCapFFlow(1),
                                                                   thisCoil.EIRFTemp(1),
                                                                   thisCoil.EIRFFlow(1),
                                                                   thisCoil.PLFFPLR(1),
                                                                   thisCoil.RatedTotCap(1),
                                                                   thisCoil.RatedCOP(1),
                                                                   thisCoil.RatedAirVolFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                   thisCoil.CondenserType(1) //,
                                                                                             // NetCoolingCapRated(1),
                                                                                             // SEER_User,     // 3.51
                                                                                             // SEER_Standard, // 3.51
                                                                                             // EER,
                                                                                             // IEER,
                                                                                             // SEER2_User,    // 3.51
                                                                                             // SEER2_Standard // 3.61
    );
    SEER_User = StandarRatingResults["SEER_User"];
    SEER_Standard = StandarRatingResults["SEER_Standard"];
    SEER2_User = StandarRatingResults["SEER2_User"];
    SEER2_Standard = StandarRatingResults["SEER2_Standard"];
    // SEER and SEER_Default must match for the same PLF curve
    EXPECT_DOUBLE_EQ(SEER_User, SEER_Standard);
    EXPECT_NEAR(12.29, SEER_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(12.29, SEER_Standard * StandardRatings::ConvFromSIToIP, 0.01);

    // Test 3: user PLF curve is the same as the AHRI Std 210/240-2023 default PLF Curve
    // reset the user PLF curve to the AHRI Std 210/240-2008 default PLF curve
    // AHRI Std 210/240-2023 default PLF curve is linear equation, PLF = a + b * PLR
    thisCoolPLFfPLR->coeff[0] = 0.80;          //
    thisCoolPLFfPLR->coeff[1] = 0.20;          //
    thisCoolPLFfPLR->inputLimits[0].min = 0.0; // PLR minimum value allowed by the PLF curve
    thisCoolPLFfPLR->inputLimits[0].max = 1.0; // PLR maximum value allowed by the PLF curve
    // reset output variables
    SEER_User = 0.0;
    SEER_Standard = 0.0;
    SEER2_User = 0.0;
    SEER2_Standard = 0.0;
    EER = 0.0;
    IEER = 0.0;
    NetCoolingCapRated = 0.0;
    StandarRatingResults.clear();
    // rerun the standard ratings calculation for PLF Curve (AHRI 2023)
    StandarRatingResults = SingleSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.Name,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.CCapFTemp(1),
                                                                   thisCoil.CCapFFlow(1),
                                                                   thisCoil.EIRFTemp(1),
                                                                   thisCoil.EIRFFlow(1),
                                                                   thisCoil.PLFFPLR(1),
                                                                   thisCoil.RatedTotCap(1),
                                                                   thisCoil.RatedCOP(1),
                                                                   thisCoil.RatedAirVolFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                   thisCoil.CondenserType(1) //,
                                                                                             // NetCoolingCapRated(1),
                                                                                             // SEER_User,
                                                                                             // SEER_Standard,
                                                                                             // EER,
                                                                                             // IEER,
                                                                                             // SEER2_User,    // 3.61
                                                                                             // SEER2_Standard // 3.61
    );
    SEER_User = StandarRatingResults["SEER_User"];
    SEER_Standard = StandarRatingResults["SEER_Standard"];
    SEER2_User = StandarRatingResults["SEER2_User"];
    SEER2_Standard = StandarRatingResults["SEER2_Standard"];

    // SEER2_USER and SEER2_Default/Standard must match for the same PLF curve
    // auto c = SEER2_User * StandardRatings::ConvFromSIToIP;
    // auto d = SEER2_Standard * StandardRatings::ConvFromSIToIP;
    EXPECT_DOUBLE_EQ(SEER2_User, SEER2_Standard);
    EXPECT_NEAR(12.10, SEER2_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(12.10, SEER2_Standard * StandardRatings::ConvFromSIToIP, 0.01);

    EXPECT_NEAR(3.60, StandarRatingResults["EER"], 0.01);
    EXPECT_NEAR(3.45, StandarRatingResults["EER_2022"], 0.01);
    EXPECT_NEAR(3.54, StandarRatingResults["IEER"], 0.01);
    EXPECT_NEAR(3.33, StandarRatingResults["IEER_2022"], 0.01);
    EXPECT_NEAR(17223.807723136004, StandarRatingResults["NetCoolingCapRated"], 0.01);
    EXPECT_NEAR(17062.707723136002, StandarRatingResults["NetCoolingCapRated2023"], 0.01);
}

TEST_F(EnergyPlusFixture, SingleSpeedCoolingCoilAir_25000W_IEER_2022_ValueTest)
{
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:SingleSpeed,",
        "    Heat Pump ACDXCoil 1,    !- Name",
        "    ,                        !- Availability Schedule Name", // FanAndCoilAvailSched
        "    autosize,                   !- Gross Rated Total Cooling Capacity {W}",
        "    autosize,                !- Gross Rated Sensible Heat Ratio",
        "    3.67,                    !- Gross Rated Cooling COP {W/W}",
        "    autosize,                     !- Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    934.4,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    DX Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACCOOLEIRFT,           !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACCOOLEIRFFF,          !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR;         !- Part Load Fraction Correlation Curve Name",

        "Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.766956,                !- Coefficient1 Constant",
        "    0.0107756,               !- Coefficient2 x",
        "    -0.0000414703,           !- Coefficient3 x**2",
        "    0.00134961,              !- Coefficient4 y",
        "    -0.000261144,            !- Coefficient5 y**2",
        "    0.000457488,             !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "Curve:Biquadratic,",
        "    HPACCOOLEIRFT,           !- Name",
        "    0.297145,                !- Coefficient1 Constant",
        "    0.0430933,               !- Coefficient2 x",
        "    -0.000748766,            !- Coefficient3 x**2",
        "    0.00597727,              !- Coefficient4 y",
        "    0.000482112,             !- Coefficient5 y**2",
        "    -0.000956448,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Quadratic,",
        "    HPACCOOLEIRFFF,          !- Name",
        "    1.156,                   !- Coefficient1 Constant",
        "    -0.1816,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "Curve:Quadratic,",
        "    HPACCOOLPLFFPLR,         !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.PLFFPLR(1)));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.PLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    std::map<std::string, Real64> StandardRatingsResult;
    Real64 NetCoolingCapRated(0.0);
    Real64 IEER_2022(0.0);
    Real64 NetCoolingCapRated2022(0.0);
    Real64 EER_2022(0.0);

    Real64 constexpr AirMassFlowRatioRated(1.0); // AHRI test is at the design flow rate and hence AirMassFlowRatio is 1.0
    int CapFFlowCurveIndex = thisCoil.CCapFFlow(1);
    int EIRFFlowCurveIndex = thisCoil.EIRFFlow(1);

    thisCoil.RatedTotCap(1) = 25619.345986365865;
    thisCoil.RatedAirVolFlowRate(1) = 1.3711908092591101;
    Real64 TotCapFlowModFac = Curve::CurveValue(*state, CapFFlowCurveIndex, AirMassFlowRatioRated);
    Real64 EIRFlowModFac = Curve::CurveValue(*state, EIRFFlowCurveIndex, AirMassFlowRatioRated);
    std::tie(IEER_2022, NetCoolingCapRated2022, EER_2022) = IEERCalculationSingleSpeed(*state,
                                                                                       thisCoil.DXCoilType,
                                                                                       thisCoil.CCapFTemp(1),
                                                                                       thisCoil.RatedTotCap(1),
                                                                                       TotCapFlowModFac,
                                                                                       thisCoil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                                       thisCoil.RatedAirVolFlowRate(1),
                                                                                       thisCoil.EIRFTemp(1),
                                                                                       thisCoil.RatedCOP(1),
                                                                                       EIRFlowModFac,
                                                                                       thisCoil.CondenserType(1));
    NetCoolingCapRated = NetCoolingCapRated2022;
    EXPECT_TRUE(IEER_2022 > 0.0);
    EXPECT_TRUE(EER_2022 > 0.0);
    EXPECT_TRUE(NetCoolingCapRated2022 > 0.0);
    EXPECT_NEAR(2.94, EER_2022, 0.01);
    EXPECT_NEAR(2.89, IEER_2022, 0.01);
    EXPECT_NEAR(24321.99, NetCoolingCapRated, 0.01);
    EXPECT_NEAR(9.88, IEER_2022 * StandardRatings::ConvFromSIToIP, 0.01);
}

TEST_F(EnergyPlusFixture, SingleSpeedCoolingCoilEvap_32000W_IEER_2022_ValueTest)
{
    std::string const idf_objects = delimited_string({
        " Coil:Cooling:DX:SingleSpeed,",
        "    Heat Pump ACDXCoil 1,    !- Name",
        "    ,    !- Availability Schedule Name", // FanAndCoilAvailSched
        "    32000,                   !- Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Gross Rated Sensible Heat Ratio",
        "    3.8,                     !- Gross Rated Cooling COP {W/W}",
        "    1.7,                     !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    934.4,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    DX Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACCOOLEIRFT,           !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACCOOLEIRFFF,          !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR,         !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate {cycles/hr}",
        "    ,                        !- Latent Capacity Time Constant {s}",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    EvaporativelyCooled,     !- Condenser Type",
        "    ,                        !- Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                        !- Evaporative Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    ,                        !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    ,                        !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    200;                     !- Basin Heater Capacity {W/K}",

        "	Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.766956,                !- Coefficient1 Constant",
        "    0.0107756,               !- Coefficient2 x",
        "    -0.0000414703,           !- Coefficient3 x**2",
        "    0.00134961,              !- Coefficient4 y",
        "    -0.000261144,            !- Coefficient5 y**2",
        "    0.000457488,             !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "	",
        "	 Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "	  Curve:Biquadratic,",
        "    HPACCOOLEIRFT,           !- Name",
        "    0.297145,                !- Coefficient1 Constant",
        "    0.0430933,               !- Coefficient2 x",
        "    -0.000748766,            !- Coefficient3 x**2",
        "    0.00597727,              !- Coefficient4 y",
        "    0.000482112,             !- Coefficient5 y**2",
        "    -0.000956448,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Quadratic,",
        "    HPACCOOLEIRFFF,          !- Name",
        "    1.156,                   !- Coefficient1 Constant",
        "    -0.1816,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "	  Curve:Quadratic,",
        "    HPACCOOLPLFFPLR,         !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.PLFFPLR(1)));

    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.PLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    std::map<std::string, Real64> StandardRatingsResult;

    thisCoil.RatedTotCap(1) = 32000.00;
    thisCoil.RatedAirVolFlowRate(1) = 1.70;
    std::map<std::string, Real64> StandarRatingResults;

    StandarRatingResults = SingleSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.Name,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.CCapFTemp(1),
                                                                   thisCoil.CCapFFlow(1),
                                                                   thisCoil.EIRFTemp(1),
                                                                   thisCoil.EIRFFlow(1),
                                                                   thisCoil.PLFFPLR(1),
                                                                   thisCoil.RatedTotCap(1),
                                                                   thisCoil.RatedCOP(1),
                                                                   thisCoil.RatedAirVolFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                   thisCoil.CondenserType(1));

    EXPECT_TRUE(StandarRatingResults["SEER_User"] > 0.0);
    EXPECT_TRUE(StandarRatingResults["SEER_Standard"] > 0.0);
    EXPECT_TRUE(StandarRatingResults["SEER2_User"] == 0.0);
    EXPECT_TRUE(StandarRatingResults["SEER2_Standard"] == 0.0);
    EXPECT_NEAR(3.14, StandarRatingResults["EER"], 0.01);
    EXPECT_NEAR(3.03, StandarRatingResults["EER_2022"], 0.01);
    EXPECT_NEAR(3.14, StandarRatingResults["IEER"], 0.01);
    EXPECT_NEAR(2.98, StandarRatingResults["IEER_2022"], 0.01);
    EXPECT_NEAR(10.16, StandarRatingResults["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(30665.270282685444, StandarRatingResults["NetCoolingCapRated"], 0.01);
    EXPECT_NEAR(30391.40, StandarRatingResults["NetCoolingCapRated2023"], 0.01);
    EXPECT_NEAR(3.30, StandarRatingResults["SEER_User"], 0.01);
    EXPECT_NEAR(3.12, StandarRatingResults["SEER_Standard"], 0.01);
}

TEST_F(EnergyPlusFixture, SingleSpeedCoolingCoilAir_AHRIExample_IEER_2022_ValueTest)
{
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:SingleSpeed,",
        "    Example AHRI 340-360,    !- Name",
        "    ,                        !- Availability Schedule Name",             // FanAndCoilAvailSched
        "    26669.5,                 !- Gross Rated Total Cooling Capacity {W}", // 91,000 Btu/h (rounded to the nearest 1,000 per 6.1.2)
        "    0.75,                    !- Gross Rated Sensible Heat Ratio",
        "    3.66,                     !- Gross Rated Cooling COP {W/W}",
        "    1.227,                   !- Rated Air Flow Rate {m3/s}", // ->
        "    ,                        !- Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    677.2616137,             !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3 /s)} ",
        "    DX Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACCOOLEIRFT,           !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACCOOLEIRFFF,          !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR;         !- Part Load Fraction Correlation Curve Name",

        "Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.766956,                !- Coefficient1 Constant",
        "    0.0107756,               !- Coefficient2 x",
        "    -0.0000414703,           !- Coefficient3 x**2",
        "    0.00134961,              !- Coefficient4 y",
        "    -0.000261144,            !- Coefficient5 y**2",
        "    0.000457488,             !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "Curve:Biquadratic,",
        "    HPACCOOLEIRFT,           !- Name",
        "    0.297145,                !- Coefficient1 Constant",
        "    0.0430933,               !- Coefficient2 x",
        "    -0.000748766,            !- Coefficient3 x**2",
        "    0.00597727,              !- Coefficient4 y",
        "    0.000482112,             !- Coefficient5 y**2",
        "    -0.000956448,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Quadratic,",
        "    HPACCOOLEIRFFF,          !- Name",
        "    1.156,                   !- Coefficient1 Constant",
        "    -0.1816,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "Curve:Quadratic,",
        "    HPACCOOLPLFFPLR,         !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.PLFFPLR(1)));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.PLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    std::map<std::string, Real64> StandardRatingsResult;

    EXPECT_EQ(26669.5, thisCoil.RatedTotCap(1));
    EXPECT_NEAR(1.227, thisCoil.RatedAirVolFlowRate(1), 0.01);

    std::map<std::string, Real64> StandarRatingResults;

    StandarRatingResults = SingleSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.Name,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.CCapFTemp(1),
                                                                   thisCoil.CCapFFlow(1),
                                                                   thisCoil.EIRFTemp(1),
                                                                   thisCoil.EIRFFlow(1),
                                                                   thisCoil.PLFFPLR(1),
                                                                   thisCoil.RatedTotCap(1),
                                                                   thisCoil.RatedCOP(1),
                                                                   thisCoil.RatedAirVolFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                   thisCoil.CondenserType(1));

    EXPECT_TRUE(StandarRatingResults["SEER_User"] > 0.0);
    EXPECT_TRUE(StandarRatingResults["SEER_Standard"] > 0.0);
    EXPECT_TRUE(StandarRatingResults["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandarRatingResults["SEER2_Standard"] > 0.0);
    EXPECT_NEAR(3.27, StandarRatingResults["SEER_User"], 0.01);
    EXPECT_NEAR(3.09, StandarRatingResults["SEER_Standard"], 0.01);
    EXPECT_NEAR(3.34, StandarRatingResults["SEER2_User"], 0.01);
    EXPECT_NEAR(3.25, StandarRatingResults["SEER2_Standard"], 0.01);
    EXPECT_NEAR(3.12, StandarRatingResults["EER"], 0.01);
    EXPECT_NEAR(3.18, StandarRatingResults["EER_2022"], 0.01);
    EXPECT_NEAR(3.16, StandarRatingResults["IEER"], 0.01);
    EXPECT_NEAR(3.26, StandarRatingResults["IEER_2022"], 0.01);
    EXPECT_NEAR(11.14, StandarRatingResults["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(25703.89, StandarRatingResults["NetCoolingCapRated"], 0.01);
    EXPECT_NEAR(25821.73, StandarRatingResults["NetCoolingCapRated2023"], 0.01);

    thisCoil.FanPowerPerEvapAirFlowRate(1) = 495;
    thisCoil.FanPowerPerEvapAirFlowRate_2023(1) = 495;
    StandarRatingResults = SingleSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.Name,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.CCapFTemp(1),
                                                                   thisCoil.CCapFFlow(1),
                                                                   thisCoil.EIRFTemp(1),
                                                                   thisCoil.EIRFFlow(1),
                                                                   thisCoil.PLFFPLR(1),
                                                                   thisCoil.RatedTotCap(1),
                                                                   thisCoil.RatedCOP(1),
                                                                   thisCoil.RatedAirVolFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate(1),
                                                                   thisCoil.FanPowerPerEvapAirFlowRate_2023(1),
                                                                   thisCoil.CondenserType(1));
    // Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)} for both 2017 & 2023
    // are same (495) so the NetCoolingCapacity, EER and IEER for both 2017 & 2023 standard
    // should match
    EXPECT_DOUBLE_EQ(StandarRatingResults["EER_2022"], StandarRatingResults["EER"]);
    EXPECT_NEAR(3.29, StandarRatingResults["EER"], 0.01);
    EXPECT_NEAR(3.29, StandarRatingResults["EER_2022"], 0.01);
    EXPECT_NEAR(3.46, StandarRatingResults["IEER"], 0.01);
    EXPECT_NEAR(3.46, StandarRatingResults["IEER_2022"], 0.01);
    EXPECT_NEAR(11.81, StandarRatingResults["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(26045.36, StandarRatingResults["NetCoolingCapRated"], 0.01);
    EXPECT_NEAR(26045.36, StandarRatingResults["NetCoolingCapRated2023"], 0.01);
    EXPECT_TRUE(StandarRatingResults["SEER_User"] > 0.0);
    EXPECT_TRUE(StandarRatingResults["SEER_Standard"] > 0.0);
    EXPECT_TRUE(StandarRatingResults["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandarRatingResults["SEER2_Standard"] > 0.0);
    EXPECT_NEAR(3.47, StandarRatingResults["SEER_User"], 0.01);
    EXPECT_NEAR(3.28, StandarRatingResults["SEER_Standard"], 0.01);
    EXPECT_NEAR(3.47, StandarRatingResults["SEER2_User"], 0.01);
    EXPECT_NEAR(3.37, StandarRatingResults["SEER2_Standard"], 0.01);
}

TEST_F(EnergyPlusFixture, MultiSpeedCoolingCoil_02_Speed_4400W_SEER2_2023_ValueTest)
{
    std::string const idf_objects = delimited_string({

        "  Zone,",
        "    SPACE2-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    103.311355591;           !- Volume {m3}",

        "ZoneControl:Thermostat,",
        "  SPACE2-1 Thermostat,                                     !- Name",
        "  SPACE2-1,                                                !- Zone or ZoneList Name",
        "  HVACTemplate-Always 4,                                   !- Control Type Schedule Name",
        "  ThermostatSetpoint:DualSetpoint,                         !- Control Object Type",
        "  All Zones Dual SP Control;                               !- Control Name",

        "ZoneHVAC:EquipmentConnections,",
        "  SPACE2-1,                                                !- Zone Name",
        "  SPACE2-1 Equipment,                                      !- Zone Conditioning Equipment List Name",
        "  SPACE2-1 Zone Inlet Node,                                !- Zone Air Inlet Node or NodeList Name",
        "  SPACE2-1 Zone Exhaust Node,                              !- Zone Air Exhaust Node or NodeList Name",
        "  SPACE2-1 Zone Air Node,                                  !- Zone Air Node Name",
        "  SPACE2-1 Return Outlet;                                  !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  SPACE2-1 Equipment,                                      !- Name",
        "  SequentialLoad,                                          !- Load Distribution Scheme",
        "  AirLoopHVAC:UnitarySystem,                               !- Zone Equipment Object Type",
        "  Unitary System Model,                                    !- Zone Equipment Name",
        "  1,                                                       !- Zone Equipment Cooling Sequence",
        "  1,                                                       !- Zone Equipment 1 Heating or No - Load Sequence",
        "  ,                                                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "  ;                                                        !- Zone Equipment 1 Sequential Heating Fraction",

        "ThermostatSetpoint:DualSetpoint,",
        "  All Zones Dual SP Control,                               !- Name",
        "  Htg-SetP-Sch,                                            !- Heating Setpoint Temperature Schedule Name",
        "  Clg-SetP-Sch;                                            !- Cooling Setpoint Temperature Schedule Name",

        "ScheduleTypeLimits,",
        "  HVACTemplate Any Number;                                 !- Name",

        "Schedule:Compact,",
        "  HVACTemplate-Always 4,                                   !- Name",
        "  HVACTemplate Any Number,                                 !- Schedule Type Limits Name",
        "  Through: 12/31,                                          !- Field 1",
        "  For: AllDays,                                            !- Field 2",
        "  Until: 24:00,                                            !- Field 3",
        "  4;                                                       !- Field 4",

        "  Schedule:Compact,",
        "    Htg-SetP-Sch,            !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 6:00,13.0,        !- Field 3",
        "    Until: 7:00,18.0,        !- Field 5",
        "    Until: 21:00,23.0,       !- Field 7",
        "    Until: 24:00,13.0,       !- Field 9",
        "    For: WeekEnds Holiday,   !- Field 11",
        "    Until: 24:00,13.0,       !- Field 12",
        "    For: SummerDesignDay,    !- Field 14",
        "    Until: 24:00,13.0,       !- Field 15",
        "    For: WinterDesignDay,    !- Field 17",
        "    Until: 24:00,23.0;       !- Field 18",

        "! For cooling, recover 1 hr early",

        "  Schedule:Compact,",
        "    Clg-SetP-Sch,            !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,32.0,        !- Field 3",
        "    Until: 21:00,24.0,       !- Field 5",
        "    Until: 24:00,32.0,       !- Field 7",
        "    For: WeekEnds Holiday,   !- Field 9",
        "    Until: 24:00,32.0,       !- Field 10",
        "    For: SummerDesignDay,    !- Field 12",
        "    Until: 24:00,24.0,       !- Field 13",
        "    For: WinterDesignDay,    !- Field 15",
        "    Until: 24:00,32.0;       !- Field 16",

        "  Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,0.0,         !- Field 3",
        "    Until: 21:00,1.0,        !- Field 5",
        "    Until: 24:00,0.0,        !- Field 7",
        "    For: Weekends Holiday,   !- Field 9",
        "    Until: 24:00,0.0,        !- Field 10",
        "    For: SummerDesignDay,    !- Field 12",
        "    Until: 24:00,1.0,        !- Field 13",
        "    For: WinterDesignDay,    !- Field 15",
        "    Until: 24:00,1.0;        !- Field 16",

        "  Schedule:Compact,",
        "    Min OA Sched,            !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 8:00,0.0,         !- Field 3",
        "    Until: 21:00,1.0,        !- Field 5",
        "    Until: 24:00,0.0,        !- Field 7",
        "    For: Weekends Holiday,   !- Field 9",
        "    Until: 24:00,0.0,        !- Field 10",
        "    For: SummerDesignDay,    !- Field 12",
        "    Until: 24:00,1.0,        !- Field 13",
        "    For: WinterDesignDay,    !- Field 15",
        "    Until: 24:00,1.0;        !- Field 16",

        "  Sizing:Parameters,",
        "    1.2,                     !- Heating Sizing Factor",
        "    1.2;                     !- Cooling Sizing Factor",

        "AvailabilityManagerAssignmentList,",
        "  Sys 2 Furnace DX Cool MultiSpd Availability Managers,    !- Name",
        "  AvailabilityManager:Scheduled,                           !- Availability Manager Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Availability;             !- Availability Manager Name",

        "AvailabilityManager:Scheduled,",
        "  Sys 2 Furnace DX Cool MultiSpd Availability,             !- Name",
        "  HVACTemplate-Always 1;                                   !- Schedule Name",

        "Schedule:Compact,",
        "  HVACTemplate-Always 1,                                   !- Name",
        "  HVACTemplate Any Number,                                 !- Schedule Type Limits Name",
        "  Through: 12/31,                                          !- Field 1",
        "  For: AllDays,                                            !- Field 2",
        "  Until: 24:00,                                            !- Field 3",
        "  1;                                                       !- Field 4",

        "AirLoopHVAC:UnitarySystem,",
        "  Unitary System Model,                                    !- Name",
        "  Load,                                                    !- Control Type",
        "  SPACE2-1,                                                !- Controlling Zone or Thermostat Location",
        "  None,                                                    !- Dehumidification Control Type",
        "  ,                                                        !- Availability Schedule Name",
        "  SPACE2-1 Zone Exhaust Node,                              !- Air Inlet Node Name",
        "  SPACE2-1 Zone Inlet Node,                                !- Air Outlet Node Name",
        "  Fan:VariableVolume,                                      !- Supply Fan Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Supply Fan,               !- Supply Fan Name",
        "  DrawThrough,                                             !- Fan Placement",
        "  FanAvailSched,                                           !- Supply Air Fan Operating Mode Schedule Name",
        "  Coil:Heating:Electric,                                   !- Heating Coil Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Heating Coil,             !- Heating Coil Name",
        "  1.0,                                                     !- DX Heating Coil Sizing Ratio",
        "  Coil:Cooling:DX:MultiSpeed,                              !- Cooling Coil Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil,             !- Cooling Coil Name",
        "  ,                                                        !- Use DOAS DX Cooling Coil",
        "  ,                                                        !- DOAS DX Cooling Coil Leaving Minimum Air Temperature {C}",
        "  ,                                                        !- Latent Load Control",
        "  ,                                                        !- Supplemental Heating Coil Object Type",
        "  ,                                                        !- Supplemental Heating Coil Name",
        "  SupplyAirFlowRate,                                       !- Cooling Supply Air Flow Rate Method",
        "  0.23122,                                                 !- Cooling Supply Air Flow Rate {m3/s}",
        "  ,                                                        !- Cooling Supply Air Flow Rate Per Floor Area {m3/s-m2",
        "  ,                                                        !- Cooling Fraction of Autosized Cooling Supply Air Flow Rate",
        "  ,                                                        !- Cooling Supply Air Flow Rate Per Unit of Capacity {m3/s-W",
        "  SupplyAirFlowRate,                                       !- Heating Supply Air Flow Rate Method",
        "  0.23122,                                                 !- Heating Supply Air Flow Rate {m3/s}",
        "  ,                                                        !- Heating Supply Air Flow Rate Per Floor Area {m3/s-m2",
        "  ,                                                        !- Heating Fraction of Autosized Heating Supply Air Flow Rate",
        "  ,                                                        !- Heating Supply Air Flow Rate Per Unit of Capacity {m3/s-W",
        "  SupplyAirFlowRate,                                       !- No Load Supply Air Flow Rate Method",
        "  0.23122,                                                 !- No Load Supply Air Flow Rate {m3/s}",
        "  ,                                                        !- No Load Supply Air Flow Rate Per Floor Area {m3/s-m2",
        "  ,                                                        !- No Load Fraction of Autosized Cooling Supply Air Flow Rate",
        "  ,                                                        !- No Load Fraction of Autosized Heating Supply Air Flow Rate",
        "  ,                                                        !- No Load Supply Air Flow Rate Per Unit of Capacity During Cooling Operation "
        "{m3/s-W}",
        "  ,                                                        !- No Load Supply Air Flow Rate Per Unit of Capacity During Heating Operation "
        "{m3/s-W}",
        "  No,                                                      !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "  Autosize,                                                !- Maximum Supply Air Temperature {C}",
        "  21,                                                      !- Maximum Outdoor Dry-Bulb Temperature for Supplemental Heater Operation {C}",
        "  ,                                                        !- Outdoor Dry-Bulb Temperature Sensor Node Name",
        "  ,                                                        !- Ancilliary On-Cycle Electric Power",
        "  ,                                                        !- Ancilliary Off-Cycle Electric Power",
        "  ,                                                        !- Design Heat Recovery Water Flow Rate",
        "  ,                                                        !- Maximum Temperature for Heat Recovery",
        "  ,                                                        !- Heat Recovery Water Inlet Node Name",
        "  ,                                                        !- Heat Recovery Water Outlet Node Name",
        "  UnitarySystemPerformance:Multispeed,                     !- Design Specification Multispeed Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Unitary System MultiSpeed Performance;  !- Design Specification Multispeed Object Name",

        "UnitarySystemPerformance:Multispeed,",
        "  Sys 2 Furnace DX Cool MultiSpd Unitary System MultiSpeed Performance,  !- Name",
        "  1,                                                       !- Number of Speeds for Heating",
        "  2,                                                       !- Number of Speeds for Cooling",
        "  No,                                                      !- Single Mode Operation",
        "  ,                                                        !- No Load Supply Air Flow Rate Ratio",
        "  1.0,                                                     !- Heating Speed 1 Supply Air Flow Ratio",
        "  0.5,                                                     !- Cooling Speed 1 Supply Air Flow Ratio",
        "  1.0,                                                     !- Heating Speed 2 Supply Air Flow Ratio",
        "  1.0;                                                     !- Cooling Speed 2 Supply Air Flow Ratio",

        "Coil:Cooling:DX:MultiSpeed,",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil,             !- Name",
        "  ,                                                        !- Availability Schedule Name",
        "  SPACE2-1 Zone Exhaust Node,                              !- Air Inlet Node Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil Outlet,      !- Air Outlet Node Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil Condenser Inlet,  !- Condenser Air Inlet Node Name",
        "  AirCooled,                                               !- Condenser Type",
        "  ,                                                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "  ,                                                        !- Supply Water Storage Tank Name",
        "  ,                                                        !- Condensate Collection Water Storage Tank Name",
        "  No,                                                      !- Apply Part Load Fraction to Speeds Greater than 1",
        "  No,                                                      !- Apply Latent Degradation to Speeds Greater than 1",
        "  0.0,                                                     !- Crankcase Heater Capacity {W}",
        "  ,                                                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "  10.0,                                                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "  ,                                                        !- Basin Heater Capacity {W/K}",
        "  ,                                                        !- Basin Heater Setpoint Temperature {C}",
        "  ,                                                        !- Basin Heater Operating Schedule Name",
        "  Electricity,                                             !- Fuel Type",
        "  2,                                                       !- Number of Speeds",
        "  2200.0,                                                  !- Speed 1 Gross Rated Total Cooling Capacity {W}",
        "  0.75232,                                                 !- Speed 1 Gross Rated Sensible Heat Ratio",
        "  3,                                                       !- Speed 1 Gross Rated Cooling COP {W/W}",
        "  0.15,                                                    !- Speed 1 Rated Air Flow Rate {m3/s}",
        "  ,                                                        !- 2017 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  450.0,                                                   !- 2023 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FT,         !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FF,         !- Speed 1 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FT,         !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FF,         !- Speed 1 Energy Input Ratio Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil PLF,            !- Speed 1 Part Load Fraction Correlation Curve Name",
        "  0,                                                       !- Speed 1 Nominal Time for Condensate Removal to Begin",
        "  0,                                                       !- Speed 1 Ratio of Initial Moisture Evaporation Rate and Steady State Latent "
        "Capacity",
        "  0,                                                       !- Speed 1 Maximum Cycling Rate",
        "  0,                                                       !- Speed 1 Latent Capacity Time Constant",
        "  0.2,                                                     !- Speed 1 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil WH-FT,          !- Speed 1 Waste Heat Function of Temperature Curve Name",
        "  ,                                                        !- Speed 1 Evaporative Condenser Effectiveness {dimensionless}",
        "  ,                                                        !- Speed 1 Evaporative Condenser Air Flow Rate {m3/s}",
        "  ,                                                        !- Speed 1 Rated Evaporative Condenser Pump Power Consumption {W}",
        "  4400.0,                                                  !- Speed 2 Gross Rated Total Cooling Capacity {W}",
        "  0.75232,                                                 !- Speed 2 Gross Rated Sensible Heat Ratio",
        "  3,                                                       !- Speed 2 Gross Rated Cooling COP {W/W}",
        "  0.25,                                                    !- Speed 2 Rated Air Flow Rate {m3/s}",
        "  ,                                                        !- 2017 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  934.4,                                                   !- 2023 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FT,         !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FF,         !- Speed 2 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FT,         !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FF,         !- Speed 2 Energy Input Ratio Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil PLF,            !- Speed 2 Part Load Fraction Correlation Curve Name",
        "  0,                                                       !- Speed 2 Nominal Time for Condensate Removal to Begin",
        "  0,                                                       !- Speed 2 Ratio of Initial Moisture Evaporation Rate and Steady State Latent "
        "Capacity",
        "  0,                                                       !- Speed 2 Maximum Cycling Rate",
        "  0,                                                       !- Speed 2 Latent Capacity Time Constant",
        "  0.2,                                                     !- Speed 2 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil WH-FT,          !- Speed 2 Waste Heat Function of Temperature Curve Name",
        "  ,                                                        !- Speed 2 Evaporative Condenser Effectiveness {dimensionless}",
        "  ,                                                        !- Speed 2 Evaporative Condenser Air Flow Rate {m3/s}",
        "  ;                                                        !- Speed 2 Rated Evaporative Condenser Pump Power Consumption {W}",

        "Curve:Biquadratic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FT,         !- Name",
        "  0.476428E+00,                                            !- Coefficient1 Constant",
        "  0.401147E-01,                                            !- Coefficient2 x",
        "  0.226411E-03,                                            !- Coefficient3 x**2",
        "  -0.827136E-03,                                           !- Coefficient4 y",
        "  -0.732240E-05,                                           !- Coefficient5 y**2",
        "  -0.446278E-03,                                           !- Coefficient6 x*y",
        "  0.0,                                                     !- Minimum Value of x",
        "  50.0,                                                    !- Maximum Value of x",
        "  0.0,                                                     !- Minimum Value of y",
        "  50.0,                                                    !- Maximum Value of y",
        "  0.0,                                                     !- Minimum Curve Output",
        "  5.0,                                                     !- Maximum Curve Output",
        "  Temperature,                                             !- Input Unit Type for X",
        "  Temperature,                                             !- Input Unit Type for Y",
        "  Dimensionless;                                           !- Output Unit Type",

        "Curve:Cubic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FF,         !- Name",
        "  .47278589,                                               !- Coefficient1 Constant",
        "  1.2433415,                                               !- Coefficient2 x",
        "  -1.0387055,                                              !- Coefficient3 x**2",
        "  .32257813,                                               !- Coefficient4 x**3",
        "  0.5,                                                     !- Minimum Value of x",
        "  1.5;                                                     !- Maximum Value of x",

        "Curve:Biquadratic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FT,         !- Name",
        "  0.632475E+00,                                            !- Coefficient1 Constant",
        "  -0.121321E-01,                                           !- Coefficient2 x",
        "  0.507773E-03,                                            !- Coefficient3 x**2",
        "  0.155377E-01,                                            !- Coefficient4 y",
        "  0.272840E-03,                                            !- Coefficient5 y**2",
        "  -0.679201E-03,                                           !- Coefficient6 x*y",
        "  0.0,                                                     !- Minimum Value of x",
        "  50.0,                                                    !- Maximum Value of x",
        "  0.0,                                                     !- Minimum Value of y",
        "  50.0,                                                    !- Maximum Value of y",
        "  0.0,                                                     !- Minimum Curve Output",
        "  5.0,                                                     !- Maximum Curve Output",
        "  Temperature,                                             !- Input Unit Type for X",
        "  Temperature,                                             !- Input Unit Type for Y",
        "  Dimensionless;                                           !- Output Unit Type",

        "Curve:Cubic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FF,         !- Name",
        "  .47278589,                                               !- Coefficient1 Constant",
        "  1.2433415,                                               !- Coefficient2 x",
        "  -1.0387055,                                              !- Coefficient3 x**2",
        "  .32257813,                                               !- Coefficient4 x**3",
        "  0.5,                                                     !- Minimum Value of x",
        "  1.5;                                                     !- Maximum Value of x",

        "Curve:Quadratic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil PLF,            !- Name",
        "  0.90,                                                    !- Coefficient1 Constant",
        "  0.10,                                                    !- Coefficient2 x",
        "  0,                                                       !- Coefficient3 x**2",
        "  0,                                                       !- Minimum Value of x",
        "  1;                                                       !- Maximum Value of x",

        "Curve:Biquadratic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil WH-FT,          !- Name",
        "  1.0,                                                     !- Coefficient1 Constant",
        "  0.0,                                                     !- Coefficient2 x",
        "  0.0,                                                     !- Coefficient3 x**2",
        "  0.0,                                                     !- Coefficient4 y",
        "  0.0,                                                     !- Coefficient5 y**2",
        "  0.0,                                                     !- Coefficient6 x*y",
        "  0,                                                       !- Minimum Value of x",
        "  50,                                                      !- Maximum Value of x",
        "  0,                                                       !- Minimum Value of y",
        "  50,                                                      !- Maximum Value of y",
        "  ,                                                        !- Minimum Curve Output",
        "  ,                                                        !- Maximum Curve Output",
        "  Temperature,                                             !- Input Unit Type for X",
        "  Temperature,                                             !- Input Unit Type for Y",
        "  Dimensionless;                                           !- Output Unit Type",

        "OutdoorAir:Node,",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil Condenser Inlet,  !- Name",
        "  -1;                                                      !- Height Above Ground",

        "Coil:Heating:Electric,",
        "  Sys 2 Furnace DX Cool MultiSpd Heating Coil,             !- Name",
        "  ,                                                        !- Availability Schedule Name",
        "  1,                                                       !- Efficiency",
        "  0.23122,                                                 !- Nominal Capacity of the Coil {W}",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil Outlet,      !- Air Inlet Node Name",
        "  Sys 2 Furnace DX Cool MultiSpd Heating Coil Outlet,      !- Air Outlet Node Name",
        "  ;                                                        !- Coil Temp Setpoint Node",

        "Fan:VariableVolume,",
        "  Sys 2 Furnace DX Cool MultiSpd Supply Fan,               !- Name",
        "  HVACTemplate-Always 1,                                   !- Availability Schedule Name",
        "  0.7,                                                     !- Fan Efficiency",
        "  600,                                                     !- Pressure Rise {Pa}",
        "  0.23122,                                                 !- Maximum Flow Rate {m3/s}",
        "  Fraction,                                                !- Fan Power Minimum Flow Rate Input Method",
        "  0.0,                                                     !- Fan Power Minimum Flow Fraction",
        "  ,                                                        !- Fan Power Minimum Air Flow Rate {m3/s}",
        "  0.9,                                                     !- Motor Efficiency",
        "  1,                                                       !- Motor in Airstream Fraction",
        "  0.0015302446,                                            !- Fan Power Coefficient 1",
        "  0.0052080574,                                            !- Fan Power Coefficient 2",
        "  1.1086242,                                               !- Fan Power Coefficient 3",
        "  -0.11635563,                                             !- Fan Power Coefficient 4",
        "  0,                                                       !- Fan Power Coefficient 5",
        "  Sys 2 Furnace DX Cool MultiSpd Heating Coil Outlet,      !- Air Inlet Node Name",
        "  SPACE2-1 Zone Inlet Node;                                !- Air Outlet Node Name",

        "OutdoorAir:NodeList,",
        "  Sys 2 Furnace DX Cool MultiSpd Outdoor Air Inlet;        !- Node or NodeList Name 1",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.MSPLFFPLR(1)));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.90, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.10, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.MSPLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Test 1: user PLF curve is different from the AHRI Std 210/240-2008 default PLF Curve
    std::map<std::string, Real64> StandardRatingsResult;
    Array1D<Real64> NetCoolingCapRated(thisCoil.NumOfSpeeds);
    Real64 SEER_User(0.0);
    Real64 SEER_Standard(0.0);

    Array1D<Real64> NetCoolingCapRated_2023(thisCoil.NumOfSpeeds);
    Real64 SEER2_User(0.0);
    Real64 SEER2_Standard(0.0);
    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = MultiSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.Name,
                                                                   thisCoil.MSCCapFTemp,
                                                                   thisCoil.MSCCapFFlow,
                                                                   thisCoil.MSEIRFTemp,
                                                                   thisCoil.MSEIRFFlow,
                                                                   thisCoil.MSPLFFPLR,
                                                                   thisCoil.MSRatedTotCap,
                                                                   thisCoil.MSRatedCOP,
                                                                   thisCoil.MSRatedAirVolFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
                                                                   thisCoil.NumOfSpeeds,
                                                                   thisCoil.CondenserType);
    NetCoolingCapRated(thisCoil.NumOfSpeeds) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed"];
    SEER_User = StandardRatingsResult["SEER_User"];
    SEER_Standard = StandardRatingsResult["SEER_Standard"];

    NetCoolingCapRated_2023(thisCoil.NumOfSpeeds) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"];
    SEER2_User = StandardRatingsResult["SEER2_User"];
    SEER2_Standard = StandardRatingsResult["SEER2_Standard"];
    // check SEER values calculated using user PLF and default PLF curve
    EXPECT_NEAR(2.88, SEER_User, 0.01);
    EXPECT_NEAR(2.79, SEER_Standard, 0.01);
    EXPECT_NEAR(4207.79, NetCoolingCapRated(thisCoil.NumOfSpeeds), 0.01);
    EXPECT_NEAR(9.85, SEER_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(9.54, SEER_Standard * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(2.62, StandardRatingsResult["EER"], 0.01);
    // check SEER values calculated using user PLF and default PLF curve | AHRI Std. 2023
    EXPECT_NEAR(2.94, SEER2_User, 0.01);
    EXPECT_NEAR(2.92, SEER2_Standard, 0.01);
    EXPECT_NEAR(4167.51, NetCoolingCapRated_2023(thisCoil.NumOfSpeeds), 0.01);
    EXPECT_NEAR(10.06, SEER2_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(9.96, SEER2_Standard * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(2.55, StandardRatingsResult["EER2"], 0.01);

    // Test 2: user PLF curve is the same as the AHRI Std 210/240-2008 default PLF Curve
    // reset the user PLF curve to the AHRI Std 210/240-2008 default PLF curve
    // AHRI Std 210/240-2008 default PLF curve is linear equation, PLF = a + b * PLR
    thisCoolPLFfPLR->coeff[0] = 0.75;          // = a
    thisCoolPLFfPLR->coeff[1] = 0.25;          // = b
    thisCoolPLFfPLR->inputLimits[0].min = 0.0; // PLR minimum value allowed by the PLF curve
    thisCoolPLFfPLR->inputLimits[0].max = 1.0; // PLR maximum value allowed by the PLF curve
    // reset output variables
    SEER_User = 0.0;
    SEER_Standard = 0.0;
    NetCoolingCapRated(thisCoil.NumOfSpeeds) = 0.0;

    SEER2_User = 0.0;
    SEER2_Standard = 0.0;

    NetCoolingCapRated_2023(thisCoil.NumOfSpeeds) = 0.0;
    StandardRatingsResult.clear();
    // rerun the standard ratings calculation
    StandardRatingsResult = MultiSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.Name,
                                                                   thisCoil.MSCCapFTemp,
                                                                   thisCoil.MSCCapFFlow,
                                                                   thisCoil.MSEIRFTemp,
                                                                   thisCoil.MSEIRFFlow,
                                                                   thisCoil.MSPLFFPLR,
                                                                   thisCoil.MSRatedTotCap,
                                                                   thisCoil.MSRatedCOP,
                                                                   thisCoil.MSRatedAirVolFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
                                                                   thisCoil.NumOfSpeeds,
                                                                   thisCoil.CondenserType);
    NetCoolingCapRated(thisCoil.NumOfSpeeds) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed"];
    SEER_User = StandardRatingsResult["SEER_User"];
    SEER_Standard = StandardRatingsResult["SEER_Standard"];

    NetCoolingCapRated_2023(thisCoil.NumOfSpeeds) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"];
    SEER2_User = StandardRatingsResult["SEER2_User"];
    SEER2_Standard = StandardRatingsResult["SEER2_Standard"];
    // SEER and SEER_Default must match for the same PLF curve
    EXPECT_NEAR(2.79, SEER_User, 0.01);
    EXPECT_NEAR(2.79, SEER_Standard, 0.01);
    EXPECT_NEAR(4207.79, NetCoolingCapRated(thisCoil.NumOfSpeeds), 0.01);
    EXPECT_DOUBLE_EQ(SEER_User, SEER_Standard);
    EXPECT_NEAR(9.54, SEER_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(9.54, SEER_Standard * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(2.62, StandardRatingsResult["EER"], 0.01);
    // check SEER values calculated using user PLF and default PLF curve | AHRI Std. 2023
    EXPECT_NEAR(2.9282, SEER2_User, 0.01);
    EXPECT_NEAR(2.9210, SEER2_Standard, 0.01);
    EXPECT_NEAR(4167.51, NetCoolingCapRated_2023(thisCoil.NumOfSpeeds), 0.01);
    EXPECT_NEAR(9.9916, SEER2_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(9.9670, SEER2_Standard * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(2.55, StandardRatingsResult["EER2"], 0.01);

    // Test 3: user PLF curve is the same as the AHRI Std 210/240-2023 default PLF Curve
    // reset the user PLF curve to the AHRI Std 210/240-2023 default PLF curve
    // AHRI Std 210/240-2023 default PLF curve is linear equation, PLF = a + b * PLR
    thisCoolPLFfPLR->coeff[0] = 0.80;          // = a
    thisCoolPLFfPLR->coeff[1] = 0.20;          // = b
    thisCoolPLFfPLR->inputLimits[0].min = 0.0; // PLR minimum value allowed by the PLF curve
    thisCoolPLFfPLR->inputLimits[0].max = 1.0; // PLR maximum value allowed by the PLF curve
    // reset output variables
    SEER_User = 0.0;
    SEER_Standard = 0.0;
    NetCoolingCapRated(thisCoil.NumOfSpeeds) = 0.0;

    SEER2_User = 0.0;
    SEER2_Standard = 0.0;
    NetCoolingCapRated_2023(thisCoil.NumOfSpeeds) = 0.0;
    StandardRatingsResult.clear();
    // rerun the standard ratings calculation
    StandardRatingsResult = MultiSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.Name,
                                                                   thisCoil.MSCCapFTemp,
                                                                   thisCoil.MSCCapFFlow,
                                                                   thisCoil.MSEIRFTemp,
                                                                   thisCoil.MSEIRFFlow,
                                                                   thisCoil.MSPLFFPLR,
                                                                   thisCoil.MSRatedTotCap,
                                                                   thisCoil.MSRatedCOP,
                                                                   thisCoil.MSRatedAirVolFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
                                                                   thisCoil.NumOfSpeeds,
                                                                   thisCoil.CondenserType);
    NetCoolingCapRated(thisCoil.NumOfSpeeds) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed"];
    SEER_User = StandardRatingsResult["SEER_User"];
    SEER_Standard = StandardRatingsResult["SEER_Standard"];

    NetCoolingCapRated_2023(thisCoil.NumOfSpeeds) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"];
    SEER2_User = StandardRatingsResult["SEER2_User"];
    SEER2_Standard = StandardRatingsResult["SEER2_Standard"];

    //  check SEER values calculated using user PLF and default PLF curve
    EXPECT_NEAR(2.82, SEER_User, 0.01);
    EXPECT_NEAR(2.79, SEER_Standard, 0.01);
    EXPECT_NEAR(4207.79, NetCoolingCapRated(thisCoil.NumOfSpeeds), 0.01);
    EXPECT_NEAR(9.65, SEER_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(9.54, SEER_Standard * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(2.62, StandardRatingsResult["EER"], 0.01);
    // SEER2 and SEER2_Default must match for the same PLF curve | AHRI Std. 2023
    EXPECT_NEAR(2.9355082864555562, SEER2_User, 0.01);     // 3.995
    EXPECT_NEAR(2.9210481459390056, SEER2_Standard, 0.01); // 3.999
    EXPECT_NEAR(4167.51, NetCoolingCapRated_2023(thisCoil.NumOfSpeeds), 0.01);
    EXPECT_NEAR(2.55, StandardRatingsResult["EER2"], 0.01);
    // EXPECT_NEAR(2.93, 2.92, 0.01);
    // EXPECT_NEAR(SEER2_User, SEER2_Standard, 0.01); // more than 0.01 i.e, 0.014 ??
    // EXPECT_DOUBLE_EQ(SEER2_User, SEER2_Standard);
    EXPECT_NEAR(10.01, SEER2_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(9.96, SEER2_Standard * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_NEAR(2.83, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(2.44, StandardRatingsResult["EER_2022"], 0.01);
    // SEER and SEER_Default must match for the same PLF curve
}

TEST_F(EnergyPlusFixture, MultiSpeedCoolingCoil_03_Speed_12000W_SEER2_2023_ValueTest)
{

    std::string const idf_objects = delimited_string({

        "  Zone,",
        "    SPACE2-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    103.311355591;           !- Volume {m3}",

        "ZoneControl:Thermostat,",
        "  SPACE2-1 Thermostat,                                     !- Name",
        "  SPACE2-1,                                                !- Zone or ZoneList Name",
        "  HVACTemplate-Always 4,                                   !- Control Type Schedule Name",
        "  ThermostatSetpoint:DualSetpoint,                         !- Control Object Type",
        "  All Zones Dual SP Control;                               !- Control Name",

        "ZoneHVAC:EquipmentConnections,",
        "  SPACE2-1,                                                !- Zone Name",
        "  SPACE2-1 Equipment,                                      !- Zone Conditioning Equipment List Name",
        "  SPACE2-1 Zone Inlet Node,                                !- Zone Air Inlet Node or NodeList Name",
        "  SPACE2-1 Zone Exhaust Node,                              !- Zone Air Exhaust Node or NodeList Name",
        "  SPACE2-1 Zone Air Node,                                  !- Zone Air Node Name",
        "  SPACE2-1 Return Outlet;                                  !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  SPACE2-1 Equipment,                                      !- Name",
        "  SequentialLoad,                                          !- Load Distribution Scheme",
        "  AirLoopHVAC:UnitarySystem,                               !- Zone Equipment Object Type",
        "  Unitary System Model,                                    !- Zone Equipment Name",
        "  1,                                                       !- Zone Equipment Cooling Sequence",
        "  1,                                                       !- Zone Equipment 1 Heating or No - Load Sequence",
        "  ,                                                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "  ;                                                        !- Zone Equipment 1 Sequential Heating Fraction",

        "ThermostatSetpoint:DualSetpoint,",
        "  All Zones Dual SP Control,                               !- Name",
        "  Htg-SetP-Sch,                                            !- Heating Setpoint Temperature Schedule Name",
        "  Clg-SetP-Sch;                                            !- Cooling Setpoint Temperature Schedule Name",

        "ScheduleTypeLimits,",
        "  HVACTemplate Any Number;                                 !- Name",

        "Schedule:Compact,",
        "  HVACTemplate-Always 4,                                   !- Name",
        "  HVACTemplate Any Number,                                 !- Schedule Type Limits Name",
        "  Through: 12/31,                                          !- Field 1",
        "  For: AllDays,                                            !- Field 2",
        "  Until: 24:00,                                            !- Field 3",
        "  4;                                                       !- Field 4",

        "  Schedule:Compact,",
        "    Htg-SetP-Sch,            !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 6:00,13.0,        !- Field 3",
        "    Until: 7:00,18.0,        !- Field 5",
        "    Until: 21:00,23.0,       !- Field 7",
        "    Until: 24:00,13.0,       !- Field 9",
        "    For: WeekEnds Holiday,   !- Field 11",
        "    Until: 24:00,13.0,       !- Field 12",
        "    For: SummerDesignDay,    !- Field 14",
        "    Until: 24:00,13.0,       !- Field 15",
        "    For: WinterDesignDay,    !- Field 17",
        "    Until: 24:00,23.0;       !- Field 18",

        "! For cooling, recover 1 hr early",

        "  Schedule:Compact,",
        "    Clg-SetP-Sch,            !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,32.0,        !- Field 3",
        "    Until: 21:00,24.0,       !- Field 5",
        "    Until: 24:00,32.0,       !- Field 7",
        "    For: WeekEnds Holiday,   !- Field 9",
        "    Until: 24:00,32.0,       !- Field 10",
        "    For: SummerDesignDay,    !- Field 12",
        "    Until: 24:00,24.0,       !- Field 13",
        "    For: WinterDesignDay,    !- Field 15",
        "    Until: 24:00,32.0;       !- Field 16",

        "  Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,0.0,         !- Field 3",
        "    Until: 21:00,1.0,        !- Field 5",
        "    Until: 24:00,0.0,        !- Field 7",
        "    For: Weekends Holiday,   !- Field 9",
        "    Until: 24:00,0.0,        !- Field 10",
        "    For: SummerDesignDay,    !- Field 12",
        "    Until: 24:00,1.0,        !- Field 13",
        "    For: WinterDesignDay,    !- Field 15",
        "    Until: 24:00,1.0;        !- Field 16",

        "  Schedule:Compact,",
        "    Min OA Sched,            !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 8:00,0.0,         !- Field 3",
        "    Until: 21:00,1.0,        !- Field 5",
        "    Until: 24:00,0.0,        !- Field 7",
        "    For: Weekends Holiday,   !- Field 9",
        "    Until: 24:00,0.0,        !- Field 10",
        "    For: SummerDesignDay,    !- Field 12",
        "    Until: 24:00,1.0,        !- Field 13",
        "    For: WinterDesignDay,    !- Field 15",
        "    Until: 24:00,1.0;        !- Field 16",

        "  Sizing:Parameters,",
        "    1.2,                     !- Heating Sizing Factor",
        "    1.2;                     !- Cooling Sizing Factor",

        "AvailabilityManagerAssignmentList,",
        "  Sys 2 Furnace DX Cool MultiSpd Availability Managers,    !- Name",
        "  AvailabilityManager:Scheduled,                           !- Availability Manager Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Availability;             !- Availability Manager Name",

        "AvailabilityManager:Scheduled,",
        "  Sys 2 Furnace DX Cool MultiSpd Availability,             !- Name",
        "  HVACTemplate-Always 1;                                   !- Schedule Name",

        "Schedule:Compact,",
        "  HVACTemplate-Always 1,                                   !- Name",
        "  HVACTemplate Any Number,                                 !- Schedule Type Limits Name",
        "  Through: 12/31,                                          !- Field 1",
        "  For: AllDays,                                            !- Field 2",
        "  Until: 24:00,                                            !- Field 3",
        "  1;                                                       !- Field 4",

        "AirLoopHVAC:UnitarySystem,",
        "  Unitary System Model,                                    !- Name",
        "  Load,                                                    !- Control Type",
        "  SPACE2-1,                                                !- Controlling Zone or Thermostat Location",
        "  None,                                                    !- Dehumidification Control Type",
        "  ,                                                        !- Availability Schedule Name",
        "  SPACE2-1 Zone Exhaust Node,                              !- Air Inlet Node Name",
        "  SPACE2-1 Zone Inlet Node,                                !- Air Outlet Node Name",
        "  Fan:VariableVolume,                                      !- Supply Fan Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Supply Fan,               !- Supply Fan Name",
        "  DrawThrough,                                             !- Fan Placement",
        "  FanAvailSched,                                           !- Supply Air Fan Operating Mode Schedule Name",
        "  Coil:Heating:Electric,                                   !- Heating Coil Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Heating Coil,             !- Heating Coil Name",
        "  1.0,                                                     !- DX Heating Coil Sizing Ratio",
        "  Coil:Cooling:DX:MultiSpeed,                              !- Cooling Coil Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil,             !- Cooling Coil Name",
        "  ,                                                        !- Use DOAS DX Cooling Coil",
        "  ,                                                        !- DOAS DX Cooling Coil Leaving Minimum Air Temperature {C}",
        "  ,                                                        !- Latent Load Control",
        "  ,                                                        !- Supplemental Heating Coil Object Type",
        "  ,                                                        !- Supplemental Heating Coil Name",
        "  SupplyAirFlowRate,                                       !- Cooling Supply Air Flow Rate Method",
        "  0.23122,                                                 !- Cooling Supply Air Flow Rate {m3/s}",
        "  ,                                                        !- Cooling Supply Air Flow Rate Per Floor Area {m3/s-m2",
        "  ,                                                        !- Cooling Fraction of Autosized Cooling Supply Air Flow Rate",
        "  ,                                                        !- Cooling Supply Air Flow Rate Per Unit of Capacity {m3/s-W",
        "  SupplyAirFlowRate,                                       !- Heating Supply Air Flow Rate Method",
        "  0.23122,                                                 !- Heating Supply Air Flow Rate {m3/s}",
        "  ,                                                        !- Heating Supply Air Flow Rate Per Floor Area {m3/s-m2",
        "  ,                                                        !- Heating Fraction of Autosized Heating Supply Air Flow Rate",
        "  ,                                                        !- Heating Supply Air Flow Rate Per Unit of Capacity {m3/s-W",
        "  SupplyAirFlowRate,                                       !- No Load Supply Air Flow Rate Method",
        "  0.23122,                                                 !- No Load Supply Air Flow Rate {m3/s}",
        "  ,                                                        !- No Load Supply Air Flow Rate Per Floor Area {m3/s-m2",
        "  ,                                                        !- No Load Fraction of Autosized Cooling Supply Air Flow Rate",
        "  ,                                                        !- No Load Fraction of Autosized Heating Supply Air Flow Rate",
        "  ,                                                        !- No Load Supply Air Flow Rate Per Unit of Capacity During Cooling Operation "
        "{m3/s-W}",
        "  ,                                                        !- No Load Supply Air Flow Rate Per Unit of Capacity During Heating Operation "
        "{m3/s-W}",
        "  No,                                                      !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "  Autosize,                                                !- Maximum Supply Air Temperature {C}",
        "  21,                                                      !- Maximum Outdoor Dry-Bulb Temperature for Supplemental Heater Operation {C}",
        "  ,                                                        !- Outdoor Dry-Bulb Temperature Sensor Node Name",
        "  ,                                                        !- Ancilliary On-Cycle Electric Power",
        "  ,                                                        !- Ancilliary Off-Cycle Electric Power",
        "  ,                                                        !- Design Heat Recovery Water Flow Rate",
        "  ,                                                        !- Maximum Temperature for Heat Recovery",
        "  ,                                                        !- Heat Recovery Water Inlet Node Name",
        "  ,                                                        !- Heat Recovery Water Outlet Node Name",
        "  UnitarySystemPerformance:Multispeed,                     !- Design Specification Multispeed Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Unitary System MultiSpeed Performance;  !- Design Specification Multispeed Object Name",

        "UnitarySystemPerformance:Multispeed,",
        "  Sys 2 Furnace DX Cool MultiSpd Unitary System MultiSpeed Performance,  !- Name",
        "  1,                                                       !- Number of Speeds for Heating",
        "  3,                                                       !- Number of Speeds for Cooling",
        "  No,                                                      !- Single Mode Operation",
        "  ,                                                        !- No Load Supply Air Flow Rate Ratio",
        "  1.0,                                                     !- Heating Speed 1 Supply Air Flow Ratio",
        "  0.333,                                                   !- Cooling Speed 1 Supply Air Flow Ratio",
        "  1.0,                                                     !- Heating Speed 2 Supply Air Flow Ratio",
        "  0.666,                                                   !- Cooling Speed 2 Supply Air Flow Ratio",
        "  1.0,                                                     !- Heating Speed 3 Supply Air Flow Ratio",
        "  1.0;                                                     !- Cooling Speed 3 Supply Air Flow Ratio",

        "Coil:Cooling:DX:MultiSpeed,",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil,             !- Name",
        "  ,                                                        !- Availability Schedule Name",
        "  SPACE2-1 Zone Exhaust Node,                              !- Air Inlet Node Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil Outlet,      !- Air Outlet Node Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil Condenser Inlet,  !- Condenser Air Inlet Node Name",
        "  AirCooled,                                               !- Condenser Type",
        "  ,                                                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "  ,                                                        !- Supply Water Storage Tank Name",
        "  ,                                                        !- Condensate Collection Water Storage Tank Name",
        "  No,                                                      !- Apply Part Load Fraction to Speeds Greater than 1",
        "  No,                                                      !- Apply Latent Degradation to Speeds Greater than 1",
        "  0.0,                                                     !- Crankcase Heater Capacity {W}",
        "  ,                                                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "  10.0,                                                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "  ,                                                        !- Basin Heater Capacity {W/K}",
        "  ,                                                        !- Basin Heater Setpoint Temperature {C}",
        "  ,                                                        !- Basin Heater Operating Schedule Name",
        "  Electricity,                                             !- Fuel Type",
        "  3,                                                       !- Number of Speeds",
        "  4000.0,                                                  !- Speed 1 Gross Rated Total Cooling Capacity {W}",
        "  0.7297,                                                  !- Speed 1 Gross Rated Sensible Heat Ratio",
        "  3.8,                                                     !- Speed 1 Gross Rated Cooling COP {W/W}",
        "  0.3,                                                    !- Speed 1 Rated Air Flow Rate {m3/s}",
        "  ,                                                        !- 2017 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  333.4,                                                   !- 2023 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FT,         !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FF,         !- Speed 1 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FT,         !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FF,         !- Speed 1 Energy Input Ratio Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil PLF,            !- Speed 1 Part Load Fraction Correlation Curve Name",
        "  0,                                                       !- Speed 1 Nominal Time for Condensate Removal to Begin",
        "  0,                                                       !- Speed 1 Ratio of Initial Moisture Evaporation Rate and Steady State Latent "
        "Capacity",
        "  0,                                                       !- Speed 1 Maximum Cycling Rate",
        "  0,                                                       !- Speed 1 Latent Capacity Time Constant",
        "  0.2,                                                     !- Speed 1 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil WH-FT,          !- Speed 1 Waste Heat Function of Temperature Curve Name",
        "  ,                                                        !- Speed 1 Evaporative Condenser Effectiveness {dimensionless}",
        "  ,                                                        !- Speed 1 Evaporative Condenser Air Flow Rate {m3/s}",
        "  ,                                                        !- Speed 1 Rated Evaporative Condenser Pump Power Consumption {W}",
        "  8000.0,                                                 !- Speed 2 Gross Rated Total Cooling Capacity {W}",
        "  0.7297,                                                  !- Speed 2 Gross Rated Sensible Heat Ratio",
        "  3.8,                                                     !- Speed 2 Gross Rated Cooling COP {W/W}",
        "  0.5,                                                    !- Speed 2 Rated Air Flow Rate {m3/s}",
        "  ,                                                        !- 2017 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  663.4,                                                   !- 2023 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FT,         !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FF,         !- Speed 2 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FT,         !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FF,         !- Speed 2 Energy Input Ratio Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil PLF,            !- Speed 2 Part Load Fraction Correlation Curve Name",
        "  0,                                                       !- Speed 2 Nominal Time for Condensate Removal to Begin",
        "  0,                                                       !- Speed 2 Ratio of Initial Moisture Evaporation Rate and Steady State Latent "
        "Capacity",
        "  0,                                                       !- Speed 2 Maximum Cycling Rate",
        "  0,                                                       !- Speed 2 Latent Capacity Time Constant",
        "  0.2,                                                     !- Speed 2 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil WH-FT,          !- Speed 2 Waste Heat Function of Temperature Curve Name",
        "  ,                                                        !- Speed 2 Evaporative Condenser Effectiveness {dimensionless}",
        "  ,                                                        !- Speed 2 Evaporative Condenser Air Flow Rate {m3/s}",
        "  ,                                                        !- Speed 2 Rated Evaporative Condenser Pump Power Consumption {W}",
        "  12000.0,                                                 !- Speed 3 Gross Rated Total Cooling Capacity {W}",
        "  0.7297,                                                  !- Speed 3 Gross Rated Sensible Heat Ratio",
        "  3.8,                                                     !- Speed 3 Gross Rated Cooling COP {W/W}",
        "  0.75,                                                    !- Speed 3 Rated Air Flow Rate {m3/s}",
        "  ,                                                        !- 2017 Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  934.4,                                                   !- 2023 Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FT,         !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FF,         !- Speed 3 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FT,         !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FF,         !- Speed 3 Energy Input Ratio Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil PLF,            !- Speed 3 Part Load Fraction Correlation Curve Name",
        "  0,                                                       !- Speed 3 Nominal Time for Condensate Removal to Begin",
        "  0,                                                       !- Speed 3 Ratio of Initial Moisture Evaporation Rate and Steady State Latent "
        "Capacity",
        "  0,                                                       !- Speed 3 Maximum Cycling Rate",
        "  0,                                                       !- Speed 3 Latent Capacity Time Constant",
        "  0.2,                                                     !- Speed 3 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil WH-FT,          !- Speed 3 Waste Heat Function of Temperature Curve Name",
        "  ,                                                        !- Speed 3 Evaporative Condenser Effectiveness {dimensionless}",
        "  ,                                                        !- Speed 3 Evaporative Condenser Air Flow Rate {m3/s}",
        "  ;                                                        !- Speed 3 Rated Evaporative Condenser Pump Power Consumption {W}",

        "Curve:Biquadratic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FT,         !- Name",
        "  0.476428E+00,                                            !- Coefficient1 Constant",
        "  0.401147E-01,                                            !- Coefficient2 x",
        "  0.226411E-03,                                            !- Coefficient3 x**2",
        "  -0.827136E-03,                                           !- Coefficient4 y",
        "  -0.732240E-05,                                           !- Coefficient5 y**2",
        "  -0.446278E-03,                                           !- Coefficient6 x*y",
        "  0.0,                                                     !- Minimum Value of x",
        "  50.0,                                                    !- Maximum Value of x",
        "  0.0,                                                     !- Minimum Value of y",
        "  50.0,                                                    !- Maximum Value of y",
        "  0.0,                                                     !- Minimum Curve Output",
        "  5.0,                                                     !- Maximum Curve Output",
        "  Temperature,                                             !- Input Unit Type for X",
        "  Temperature,                                             !- Input Unit Type for Y",
        "  Dimensionless;                                           !- Output Unit Type",

        "Curve:Cubic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FF,         !- Name",
        "  .47278589,                                               !- Coefficient1 Constant",
        "  1.2433415,                                               !- Coefficient2 x",
        "  -1.0387055,                                              !- Coefficient3 x**2",
        "  .32257813,                                               !- Coefficient4 x**3",
        "  0.5,                                                     !- Minimum Value of x",
        "  1.5;                                                     !- Maximum Value of x",

        "Curve:Biquadratic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FT,         !- Name",
        "  0.632475E+00,                                            !- Coefficient1 Constant",
        "  -0.121321E-01,                                           !- Coefficient2 x",
        "  0.507773E-03,                                            !- Coefficient3 x**2",
        "  0.155377E-01,                                            !- Coefficient4 y",
        "  0.272840E-03,                                            !- Coefficient5 y**2",
        "  -0.679201E-03,                                           !- Coefficient6 x*y",
        "  0.0,                                                     !- Minimum Value of x",
        "  50.0,                                                    !- Maximum Value of x",
        "  0.0,                                                     !- Minimum Value of y",
        "  50.0,                                                    !- Maximum Value of y",
        "  0.0,                                                     !- Minimum Curve Output",
        "  5.0,                                                     !- Maximum Curve Output",
        "  Temperature,                                             !- Input Unit Type for X",
        "  Temperature,                                             !- Input Unit Type for Y",
        "  Dimensionless;                                           !- Output Unit Type",

        "Curve:Cubic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FF,         !- Name",
        "  .47278589,                                               !- Coefficient1 Constant",
        "  1.2433415,                                               !- Coefficient2 x",
        "  -1.0387055,                                              !- Coefficient3 x**2",
        "  .32257813,                                               !- Coefficient4 x**3",
        "  0.5,                                                     !- Minimum Value of x",
        "  1.5;                                                     !- Maximum Value of x",

        "Curve:Quadratic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil PLF,            !- Name",
        "  0.85,                                                    !- Coefficient1 Constant",
        "  0.15,                                                    !- Coefficient2 x",
        "  0,                                                       !- Coefficient3 x**2",
        "  0,                                                       !- Minimum Value of x",
        "  1;                                                       !- Maximum Value of x",

        "Curve:Biquadratic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil WH-FT,          !- Name",
        "  1.0,                                                     !- Coefficient1 Constant",
        "  0.0,                                                     !- Coefficient2 x",
        "  0.0,                                                     !- Coefficient3 x**2",
        "  0.0,                                                     !- Coefficient4 y",
        "  0.0,                                                     !- Coefficient5 y**2",
        "  0.0,                                                     !- Coefficient6 x*y",
        "  0,                                                       !- Minimum Value of x",
        "  50,                                                      !- Maximum Value of x",
        "  0,                                                       !- Minimum Value of y",
        "  50,                                                      !- Maximum Value of y",
        "  ,                                                        !- Minimum Curve Output",
        "  ,                                                        !- Maximum Curve Output",
        "  Temperature,                                             !- Input Unit Type for X",
        "  Temperature,                                             !- Input Unit Type for Y",
        "  Dimensionless;                                           !- Output Unit Type",

        "OutdoorAir:Node,",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil Condenser Inlet,  !- Name",
        "  -1;                                                      !- Height Above Ground",

        "Coil:Heating:Electric,",
        "  Sys 2 Furnace DX Cool MultiSpd Heating Coil,             !- Name",
        "  ,                                                        !- Availability Schedule Name",
        "  1,                                                       !- Efficiency",
        "  0.23122,                                                 !- Nominal Capacity of the Coil {W}",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil Outlet,      !- Air Inlet Node Name",
        "  Sys 2 Furnace DX Cool MultiSpd Heating Coil Outlet,      !- Air Outlet Node Name",
        "  ;                                                        !- Coil Temp Setpoint Node",

        "Fan:VariableVolume,",
        "  Sys 2 Furnace DX Cool MultiSpd Supply Fan,               !- Name",
        "  HVACTemplate-Always 1,                                   !- Availability Schedule Name",
        "  0.7,                                                     !- Fan Efficiency",
        "  600,                                                     !- Pressure Rise {Pa}",
        "  0.23122,                                                 !- Maximum Flow Rate {m3/s}",
        "  Fraction,                                                !- Fan Power Minimum Flow Rate Input Method",
        "  0.0,                                                     !- Fan Power Minimum Flow Fraction",
        "  ,                                                        !- Fan Power Minimum Air Flow Rate {m3/s}",
        "  0.9,                                                     !- Motor Efficiency",
        "  1,                                                       !- Motor in Airstream Fraction",
        "  0.0015302446,                                            !- Fan Power Coefficient 1",
        "  0.0052080574,                                            !- Fan Power Coefficient 2",
        "  1.1086242,                                               !- Fan Power Coefficient 3",
        "  -0.11635563,                                             !- Fan Power Coefficient 4",
        "  0,                                                       !- Fan Power Coefficient 5",
        "  Sys 2 Furnace DX Cool MultiSpd Heating Coil Outlet,      !- Air Inlet Node Name",
        "  SPACE2-1 Zone Inlet Node;                                !- Air Outlet Node Name",

        "OutdoorAir:NodeList,",
        "  Sys 2 Furnace DX Cool MultiSpd Outdoor Air Inlet;        !- Node or NodeList Name 1",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.MSPLFFPLR(1)));

    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.MSPLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Test 1: user PLF curve is different from the AHRI Std 210/240-2008 default PLF Curve
    std::map<std::string, Real64> StandardRatingsResult;
    Array1D<Real64> NetCoolingCapRated(thisCoil.NumOfSpeeds);
    Real64 SEER_User(0.0);
    Real64 SEER_Standard(0.0);

    Array1D<Real64> NetCoolingCapRated_2023(thisCoil.NumOfSpeeds);
    Real64 SEER2_User(0.0);
    Real64 SEER2_Standard(0.0);
    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = MultiSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.Name,
                                                                   thisCoil.MSCCapFTemp,
                                                                   thisCoil.MSCCapFFlow,
                                                                   thisCoil.MSEIRFTemp,
                                                                   thisCoil.MSEIRFFlow,
                                                                   thisCoil.MSPLFFPLR,
                                                                   thisCoil.MSRatedTotCap,
                                                                   thisCoil.MSRatedCOP,
                                                                   thisCoil.MSRatedAirVolFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
                                                                   thisCoil.NumOfSpeeds,
                                                                   thisCoil.CondenserType);
    NetCoolingCapRated(thisCoil.NumOfSpeeds) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed"];
    SEER_User = StandardRatingsResult["SEER_User"];
    SEER_Standard = StandardRatingsResult["SEER_Standard"];

    NetCoolingCapRated_2023(thisCoil.NumOfSpeeds) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"];
    SEER2_User = StandardRatingsResult["SEER2_User"];
    SEER2_Standard = StandardRatingsResult["SEER2_Standard"];
    // check SEER values calculated using user PLF and default PLF curve
    EXPECT_NEAR(3.51, SEER_User, 0.01);
    EXPECT_NEAR(3.47, SEER_Standard, 0.01);
    EXPECT_NEAR(11423.07, NetCoolingCapRated(thisCoil.NumOfSpeeds), 0.01);
    EXPECT_NEAR(11.9824, SEER_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(11.8596, SEER_Standard * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(3.17, StandardRatingsResult["EER"], 0.01);
    // check SEER values calculated using user PLF and default PLF curve | AHRI Std. 2023
    EXPECT_NEAR(3.56, SEER2_User, 0.01);
    EXPECT_NEAR(3.72, SEER2_Standard, 0.01);
    EXPECT_NEAR(11302.25, NetCoolingCapRated_2023(thisCoil.NumOfSpeeds), 0.01);
    EXPECT_NEAR(12.16, SEER2_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(12.69, SEER2_Standard * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(3.06, StandardRatingsResult["EER2"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_NEAR(3.73, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(2.91, StandardRatingsResult["EER_2022"], 0.01);
}

TEST_F(EnergyPlusFixture, MultiSpeedCoolingCoil_04_Speed_17750W_SEER2_2023_ValueTest)
{
    // !- ## Adapted from 35500w Coil in: MultiSpeedHeatPump.idf

    std::string const idf_objects = delimited_string({

        "Coil:Cooling:DX:MultiSpeed,",
        "    Heat Pump ACDXCoil 1,    !- Name",
        "    FanAndCoilAvailSched,    !- Availability Schedule Name",
        "    DX Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "    Outdoor Condenser Air Node,  !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    -8.0,                    !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    No,                      !- Apply Part Load Fraction to Speeds Greater than 1",
        "    No,                      !- Apply Latent Degradation to Speeds Greater than 1",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    4,                       !- Number of Speeds",
        "    3750,                    !- Speed 1 Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 1 Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Speed 1 Gross Rated Cooling COP {W/W}",
        "    0.25,                    !- Speed 1 Rated Air Flow Rate {m3/s}",
        "    453.3,                   !- Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    934.4,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    HPACCoolCapFT Speed 1,   !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFF Speed 1,   !- Speed 1 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACCOOLEIRFT Speed 1,   !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACCOOLEIRFF Speed 1,   !- Speed 1 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR Speed 1, !- Speed 1 Part Load Fraction Correlation Curve Name",
        "    1000.0,                  !- Speed 1 Nominal Time for Condensate Removal to Begin {s}",
        "    1.5,                     !- Speed 1 Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    3.0,                     !- Speed 1 Maximum Cycling Rate {cycles/hr}",
        "    45.0,                    !- Speed 1 Latent Capacity Time Constant {s}",
        "    0.2,                     !- Speed 1 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    HAPCCoolWHFT Speed 1,    !- Speed 1 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 1 Evaporative Condenser Effectiveness {dimensionless}",
        "    0.05,                    !- Speed 1 Evaporative Condenser Air Flow Rate {m3/s}",
        "    50,                      !- Speed 1 Rated Evaporative Condenser Pump Power Consumption {W}",
        "    8750,                    !- Speed 2 Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 2 Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Speed 2 Gross Rated Cooling COP {W/W}",
        "    0.425,                   !- Speed 2 Rated Air Flow Rate {m3/s}",
        "    523.3,                   !- Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    934.4,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    HPACCoolCapFT Speed 2,   !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFF Speed 2,   !- Speed 2 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACCOOLEIRFT Speed 2,   !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACCOOLEIRFF Speed 2,   !- Speed 2 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR Speed 2, !- Speed 2 Part Load Fraction Correlation Curve Name",
        "    1000.0,                  !- Speed 2 Nominal Time for Condensate Removal to Begin {s}",
        "    1.5,                     !- Speed 2 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "    3.0,                     !- Speed 2 Maximum Cycling Rate {cycles/hr}",
        "    45.0,                    !- Speed 2 Latent Capacity Time Constant {s}",
        "    0.2,                     !- Speed 2 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    HAPCCoolWHFT Speed 2,    !- Speed 2 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 2 Evaporative Condenser Effectiveness {dimensionless}",
        "    0.1,                     !- Speed 2 Evaporative Condenser Air Flow Rate {m3/s}",
        "    60,                      !- Speed 2 Rated Evaporative Condenser Pump Power Consumption {W}",
        "    12750,                   !- Speed 3 Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 3 Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Speed 3 Gross Rated Cooling COP {W/W}",
        "    0.625,                   !- Speed 3 Rated Air Flow Rate {m3/s}",
        "    573.3,                   !- Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    934.4,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    HPACCoolCapFT Speed 3,   !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFF Speed 3,   !- Speed 3 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACCOOLEIRFT Speed 3,   !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACCOOLEIRFF Speed 3,   !- Speed 3 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR Speed 3, !- Speed 3 Part Load Fraction Correlation Curve Name",
        "    1000.0,                  !- Speed 3 Nominal Time for Condensate Removal to Begin {s}",
        "    1.5,                     !- Speed 3 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "    3.0,                     !- Speed 3 Maximum Cycling Rate {cycles/hr}",
        "    45.0,                    !- Speed 3 Latent Capacity Time Constant {s}",
        "    0.2,                     !- Speed 3 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    HAPCCoolWHFT Speed 3,    !- Speed 3 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 3 Evaporative Condenser Effectiveness {dimensionless}",
        "    0.2,                     !- Speed 3 Evaporative Condenser Air Flow Rate {m3/s}",
        "    80,                      !- Speed 3 Rated Evaporative Condenser Pump Power Consumption {W}",
        "    17750,                   !- Speed 4 Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 4 Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Speed 4 Gross Rated Cooling COP {W/W}",
        "    0.9,                     !- Speed 4 Rated Air Flow Rate {m3/s}",
        "    673.3,                   !- Speed 4 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    934.4,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    HPACCoolCapFT Speed 4,   !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFF Speed 4,   !- Speed 4 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACCOOLEIRFT Speed 4,   !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACCOOLEIRFF Speed 4,   !- Speed 4 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR Speed 4, !- Speed 4 Part Load Fraction Correlation Curve Name",
        "    1000.0,                  !- Speed 4 Nominal Time for Condensate Removal to Begin {s}",
        "    1.5,                     !- Speed 4 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "    3.0,                     !- Speed 4 Maximum Cycling Rate {cycles/hr}",
        "    45.0,                    !- Speed 4 Latent Capacity Time Constant {s}",
        "    0.2,                     !- Speed 4 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    HAPCCoolWHFT Speed 4,    !- Speed 4 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 4 Evaporative Condenser Effectiveness {dimensionless}",
        "    0.3,                     !- Speed 4 Evaporative Condenser Air Flow Rate {m3/s}",
        "    100;                     !- Speed 4 Rated Evaporative Condenser Pump Power Consumption {W}",
        "",
        "  Curve:Cubic,",
        "    HPACHeatCapFT Speed 1,   !- Name",
        "    0.758746,                !- Coefficient1 Constant",
        "    0.027626,                !- Coefficient2 x",
        "    0.000148716,             !- Coefficient3 x**2",
        "    0.0000034992,            !- Coefficient4 x**3",
        "    -20.0,                   !- Minimum Value of x",
        "    20.0,                    !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Biquadratic,",
        "    HPACCoolCapFT Speed 1,   !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Biquadratic,",
        "    HPACCoolCapFT Speed 2,   !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Biquadratic,",
        "    HPACCoolCapFT Speed 3,   !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Biquadratic,",
        "    HPACCoolCapFT Speed 4,   !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  ! same as Doe-2 SDL-C78",
        "",
        "  Curve:Cubic,",
        "    HPACCoolCapFF Speed 1,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Cubic,",
        "    HPACCoolCapFF Speed 2,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Cubic,",
        "    HPACCoolCapFF Speed 3,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Cubic,",
        "    HPACCoolCapFF Speed 4,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  ! same as Doe-2 SDL-C82",
        "",
        "  Curve:Biquadratic,",
        "    HPACCOOLEIRFT Speed 1,   !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Biquadratic,",
        "    HPACCOOLEIRFT Speed 2,   !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Biquadratic,",
        "    HPACCOOLEIRFT Speed 3,   !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Biquadratic,",
        "    HPACCOOLEIRFT Speed 4,   !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  ! same as Doe-2 SDL-C78",
        "",
        "  Curve:Cubic,",
        "    HPACCOOLEIRFF Speed 1,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Cubic,",
        "    HPACCOOLEIRFF Speed 2,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Cubic,",
        "    HPACCOOLEIRFF Speed 3,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Cubic,",
        "    HPACCOOLEIRFF Speed 4,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Quadratic,",
        "    HPACCOOLPLFFPLR Speed 1, !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "",
        "  Curve:Quadratic,",
        "    HPACCOOLPLFFPLR Speed 2, !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "",
        "  Curve:Quadratic,",
        "    HPACCOOLPLFFPLR Speed 3, !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "",
        "  Curve:Quadratic,",
        "    HPACCOOLPLFFPLR Speed 4, !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "",
        "! operation condition varies",
        "",
        "  Curve:Biquadratic,",
        "    HAPCCoolWHFT Speed 1,    !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    50,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "! operation condition varies",
        "",
        "  Curve:Biquadratic,",
        "    HAPCCoolWHFT Speed 2,    !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    50,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "! operation condition varies",
        "",
        "  Curve:Biquadratic,",
        "    HAPCCoolWHFT Speed 3,    !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    50,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "! operation condition varies",
        "",
        "  Curve:Biquadratic,",
        "    HAPCCoolWHFT Speed 4,    !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    50,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Schedule:Compact,",
        "    FanAndCoilAvailSched,    !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 3/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0,        !- Field 3",
        "    Through: 9/30,           !- Field 5",
        "    For: WeekDays,           !- Field 6",
        "    Until: 7:00,0.0,         !- Field 7",
        "    Until: 17:00,1.0,        !- Field 9",
        "    Until: 24:00,0.0,        !- Field 11",
        "    For: WinterDesignDay,    !- Field 13",
        "    Until: 24:00,1.0,        !- Field 14",
        "    For: SummerDesignDay,    !- Field 16",
        "    Until: 7:00,0.0,         !- Field 17",
        "    Until: 17:00,1.0,        !- Field 19",
        "    Until: 24:00,0.0,        !- Field 21",
        "    For: AllOtherDays,       !- Field 23",
        "    Until: 24:00,0.0,        !- Field 24",
        "    Through: 12/31,          !- Field 26",
        "    For: AllDays,            !- Field 27",
        "    Until: 24:00,1.0;        !- Field 28",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.MSPLFFPLR(1)));

    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.MSPLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Test 1: user PLF curve is different from the AHRI Std 210/240-2008 default PLF Curve
    std::map<std::string, Real64> StandardRatingsResult;
    Array1D<Real64> NetCoolingCapRated(thisCoil.NumOfSpeeds);
    Real64 SEER_User(0.0);
    Real64 SEER_Standard(0.0);

    Array1D<Real64> NetCoolingCapRated_2023(thisCoil.NumOfSpeeds);
    Real64 SEER2_User(0.0);
    Real64 SEER2_Standard(0.0);
    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = MultiSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.Name,
                                                                   thisCoil.MSCCapFTemp,
                                                                   thisCoil.MSCCapFFlow,
                                                                   thisCoil.MSEIRFTemp,
                                                                   thisCoil.MSEIRFFlow,
                                                                   thisCoil.MSPLFFPLR,
                                                                   thisCoil.MSRatedTotCap,
                                                                   thisCoil.MSRatedCOP,
                                                                   thisCoil.MSRatedAirVolFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
                                                                   thisCoil.NumOfSpeeds,
                                                                   thisCoil.CondenserType);
    NetCoolingCapRated(thisCoil.NumOfSpeeds) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed"];
    SEER_User = StandardRatingsResult["SEER_User"];
    SEER_Standard = StandardRatingsResult["SEER_Standard"];

    NetCoolingCapRated_2023(thisCoil.NumOfSpeeds) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"];
    SEER2_User = StandardRatingsResult["SEER2_User"];
    SEER2_Standard = StandardRatingsResult["SEER2_Standard"];
    // check SEER values calculated using user PLF and default PLF curve
    EXPECT_NEAR(3.12, SEER_User, 0.01);
    EXPECT_NEAR(3.11, SEER_Standard, 0.01);
    EXPECT_NEAR(17148.5444, NetCoolingCapRated(thisCoil.NumOfSpeeds), 0.01);
    EXPECT_NEAR(10.66, SEER_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(10.61, SEER_Standard * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(2.70, StandardRatingsResult["EER"], 0.01);
    // check SEER values calculated using user PLF and default PLF curve | AHRI Std. 2023
    EXPECT_NEAR(2.79, SEER2_User, 0.01);
    EXPECT_NEAR(2.84, SEER2_Standard, 0.01);
    EXPECT_NEAR(16913.5544, NetCoolingCapRated_2023(thisCoil.NumOfSpeeds), 0.01);
    EXPECT_NEAR(9.54, SEER2_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(9.71, SEER2_Standard * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(2.60, StandardRatingsResult["EER2"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_NEAR(2.92, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(2.49, StandardRatingsResult["EER_2022"], 0.01);
}

TEST_F(EnergyPlusFixture, MultiSpeedCoolingCoil_02_Speeds_27717W_IEER_2022_ValueOrigTest)
{
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:MultiSpeed,",
        "    PSZ-AC_2:2_CoolC DXCoil, !- Name",
        "    ,               !- Availability Schedule Name", // ALWAYS_ON
        "    PSZ-AC_2:2_OA-PSZ-AC_2:2_CoolCNode,  !- Air Inlet Node Name",
        "    PSZ-AC_2:2_CoolC-PSZ-AC_2:2_HeatCNode,  !- Air Outlet Node Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    No,                      !- Apply Part Load Fraction to Speeds Greater than 1",
        "    No,                      !- Apply Latent Degradation to Speeds Greater than 1",
        "    ,                        !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10,                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    2,                       !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    Electricity,             !- Fuel Type",
        "    2,                       !- Number of Speeds",
        "    13858.59,                !- Speed 1 Gross Rated Total Cooling Capacity {W}",
        "    0.72970,                 !- Speed 1 Gross Rated Sensible Heat Ratio",
        "    3.7988306741603,         !- Speed 1 Gross Rated Cooling COP {W/W}",
        "    0.906886666666667,       !- Speed 1 Rated Air Flow Rate {m3/s}",
        "    0,                       !- Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    453.3,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_CapFT,  !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_CapFF,  !- Speed 1 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_EIRFT,  !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_EIRFFF,  !- Speed 1 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_PLR,  !- Speed 1 Part Load Fraction Correlation Curve Name",
        "    ,                        !- Speed 1 Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Speed 1 Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    ,                        !- Speed 1 Maximum Cycling Rate {cycles/hr}",
        "    ,                        !- Speed 1 Latent Capacity Time Constant {s}",
        "    0.2,                     !- Speed 1 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Speed 1 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 1 Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                        !- Speed 1 Evaporative Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Rated Evaporative Condenser Pump Power Consumption {W}",
        "    27717.18,                !- Speed 2 Gross Rated Total Cooling Capacity {W}",
        "    0.72970,                 !- Speed 2 Gross Rated Sensible Heat Ratio",
        "    3.7988306741603,         !- Speed 2 Gross Rated Cooling COP {W/W}",
        "    1.36033,                 !- Speed 2 Rated Air Flow Rate {m3/s}",
        "    0,                       !- Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    812.9,                   !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_CapFT,  !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_CapFF,  !- Speed 2 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_EIRFT,  !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_EIRFFF,  !- Speed 2 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_PLR,  !- Speed 2 Part Load Fraction Correlation Curve Name",
        "    ,                        !- Speed 2 Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Speed 2 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "    ,                        !- Speed 2 Maximum Cycling Rate {cycles/hr}",
        "    ,                        !- Speed 2 Latent Capacity Time Constant {s}",
        "    0.2,                     !- Speed 2 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Speed 2 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 2 Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                        !- Speed 2 Evaporative Condenser Air Flow Rate {m3/s}",
        "    ;                        !- Speed 2 Rated Evaporative Condenser Pump Power Consumption {W}",

        "	  Curve:Biquadratic,",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_CapFT,  !- Name",
        "    0.42415,                 !- Coefficient1 Constant",
        "    0.04426,                 !- Coefficient2 x",
        "    -0.00042,                !- Coefficient3 x**2",
        "    0.00333,                 !- Coefficient4 y",
        "    -0.00008,                !- Coefficient5 y**2",
        "    -0.00021,                !- Coefficient6 x*y",
        "    17.00000,                !- Minimum Value of x",
        "    22.00000,                !- Maximum Value of x",
        "    29.00000,                !- Minimum Value of y",
        "    46.00000;                !- Maximum Value of y",

        "	  Curve:Quadratic,",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_CapFF,  !- Name",
        "    0.77136,                 !- Coefficient1 Constant",
        "    0.34053,                 !- Coefficient2 x",
        "    -0.11088,                !- Coefficient3 x**2",
        "    0.75918,                 !- Minimum Value of x",
        "    1.13877;                 !- Maximum Value of x",

        "	  Curve:Biquadratic,",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_EIRFT,  !- Name",
        "    1.23649,                 !- Coefficient1 Constant",
        "    -0.02431,                !- Coefficient2 x",
        "    0.00057,                 !- Coefficient3 x**2",
        "    -0.01434,                !- Coefficient4 y",
        "    0.00063,                 !- Coefficient5 y**2",
        "    -0.00038,                !- Coefficient6 x*y",
        "    17.00000,                !- Minimum Value of x",
        "    22.00000,                !- Maximum Value of x",
        "    29.00000,                !- Minimum Value of y",
        "    46.00000;                !- Maximum Value of y",

        "	  Curve:Quadratic,",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_EIRFFF,  !- Name",
        "    1.20550,                 !- Coefficient1 Constant",
        "    -0.32953,                !- Coefficient2 x",
        "    0.12308,                 !- Coefficient3 x**2",
        "    0.75918,                 !- Minimum Value of x",
        "    1.13877;                 !- Maximum Value of x",

        "	  Curve:Quadratic,",
        "    PSZ-AC_CoolCLennoxStandard10Ton_TGA120S2B_PLR,  !- Name",
        "    0.77100,                 !- Coefficient1 Constant",
        "    0.22900,                 !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetDXCoils(*state);
    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.MSPLFFPLR(1)));
    // ckeck user PLF curve coefficients | HPACCOOLPLFFPLR Speed 1
    EXPECT_EQ(0.771, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.229, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.MSPLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // check Gross Rated Total cooling Capacity
    EXPECT_EQ(13858.59, thisCoil.MSRatedTotCap(1));
    EXPECT_EQ(27717.18, thisCoil.MSRatedTotCap(2));

    std::map<std::string, Real64> StandardRatingsResult;
    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = MultiSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.Name,
                                                                   thisCoil.MSCCapFTemp,
                                                                   thisCoil.MSCCapFFlow,
                                                                   thisCoil.MSEIRFTemp,
                                                                   thisCoil.MSEIRFFlow,
                                                                   thisCoil.MSPLFFPLR,
                                                                   thisCoil.MSRatedTotCap,
                                                                   thisCoil.MSRatedCOP,
                                                                   thisCoil.MSRatedAirVolFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
                                                                   thisCoil.NumOfSpeeds,
                                                                   thisCoil.CondenserType);

    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(26735.034484053773, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["SEER_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER"] > 0.0);
    EXPECT_NEAR(3.36, StandardRatingsResult["SEER_User"], 0.01);
    EXPECT_NEAR(3.34, StandardRatingsResult["SEER_Standard"], 0.01);
    EXPECT_NEAR(3.33, StandardRatingsResult["EER"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_NEAR(3.74, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.52, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(3.31, StandardRatingsResult["EER2"], 0.01);

    EXPECT_NEAR(3.19, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_NEAR(3.36, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(26681.165416053773, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);
    EXPECT_NEAR(11.49, StandardRatingsResult["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
}

TEST_F(EnergyPlusFixture, MultiSpeedCoolingCoil_03_Speeds_27717W_IEER_2022_ValueOrigTest)
{
    std::string const idf_objects = delimited_string({

        "  Zone,",
        "    SPACE2-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    103.311355591;           !- Volume {m3}",

        "ZoneControl:Thermostat,",
        "  SPACE2-1 Thermostat,                                     !- Name",
        "  SPACE2-1,                                                !- Zone or ZoneList Name",
        "  HVACTemplate-Always 4,                                   !- Control Type Schedule Name",
        "  ThermostatSetpoint:DualSetpoint,                         !- Control Object Type",
        "  All Zones Dual SP Control;                               !- Control Name",

        "ZoneHVAC:EquipmentConnections,",
        "  SPACE2-1,                                                !- Zone Name",
        "  SPACE2-1 Equipment,                                      !- Zone Conditioning Equipment List Name",
        "  SPACE2-1 Zone Inlet Node,                                !- Zone Air Inlet Node or NodeList Name",
        "  SPACE2-1 Zone Exhaust Node,                              !- Zone Air Exhaust Node or NodeList Name",
        "  SPACE2-1 Zone Air Node,                                  !- Zone Air Node Name",
        "  SPACE2-1 Return Outlet;                                  !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  SPACE2-1 Equipment,                                      !- Name",
        "  SequentialLoad,                                          !- Load Distribution Scheme",
        "  AirLoopHVAC:UnitarySystem,                               !- Zone Equipment Object Type",
        "  Unitary System Model,                                    !- Zone Equipment Name",
        "  1,                                                       !- Zone Equipment Cooling Sequence",
        "  1,                                                       !- Zone Equipment 1 Heating or No - Load Sequence",
        "  ,                                                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "  ;                                                        !- Zone Equipment 1 Sequential Heating Fraction",

        "ThermostatSetpoint:DualSetpoint,",
        "  All Zones Dual SP Control,                               !- Name",
        "  Htg-SetP-Sch,                                            !- Heating Setpoint Temperature Schedule Name",
        "  Clg-SetP-Sch;                                            !- Cooling Setpoint Temperature Schedule Name",

        "ScheduleTypeLimits,",
        "  HVACTemplate Any Number;                                 !- Name",

        "Schedule:Compact,",
        "  HVACTemplate-Always 4,                                   !- Name",
        "  HVACTemplate Any Number,                                 !- Schedule Type Limits Name",
        "  Through: 12/31,                                          !- Field 1",
        "  For: AllDays,                                            !- Field 2",
        "  Until: 24:00,                                            !- Field 3",
        "  4;                                                       !- Field 4",

        "  Schedule:Compact,",
        "    Htg-SetP-Sch,            !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 6:00,13.0,        !- Field 3",
        "    Until: 7:00,18.0,        !- Field 5",
        "    Until: 21:00,23.0,       !- Field 7",
        "    Until: 24:00,13.0,       !- Field 9",
        "    For: WeekEnds Holiday,   !- Field 11",
        "    Until: 24:00,13.0,       !- Field 12",
        "    For: SummerDesignDay,    !- Field 14",
        "    Until: 24:00,13.0,       !- Field 15",
        "    For: WinterDesignDay,    !- Field 17",
        "    Until: 24:00,23.0;       !- Field 18",

        "! For cooling, recover 1 hr early",

        "  Schedule:Compact,",
        "    Clg-SetP-Sch,            !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,32.0,        !- Field 3",
        "    Until: 21:00,24.0,       !- Field 5",
        "    Until: 24:00,32.0,       !- Field 7",
        "    For: WeekEnds Holiday,   !- Field 9",
        "    Until: 24:00,32.0,       !- Field 10",
        "    For: SummerDesignDay,    !- Field 12",
        "    Until: 24:00,24.0,       !- Field 13",
        "    For: WinterDesignDay,    !- Field 15",
        "    Until: 24:00,32.0;       !- Field 16",

        "  Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,0.0,         !- Field 3",
        "    Until: 21:00,1.0,        !- Field 5",
        "    Until: 24:00,0.0,        !- Field 7",
        "    For: Weekends Holiday,   !- Field 9",
        "    Until: 24:00,0.0,        !- Field 10",
        "    For: SummerDesignDay,    !- Field 12",
        "    Until: 24:00,1.0,        !- Field 13",
        "    For: WinterDesignDay,    !- Field 15",
        "    Until: 24:00,1.0;        !- Field 16",

        "  Schedule:Compact,",
        "    Min OA Sched,            !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 8:00,0.0,         !- Field 3",
        "    Until: 21:00,1.0,        !- Field 5",
        "    Until: 24:00,0.0,        !- Field 7",
        "    For: Weekends Holiday,   !- Field 9",
        "    Until: 24:00,0.0,        !- Field 10",
        "    For: SummerDesignDay,    !- Field 12",
        "    Until: 24:00,1.0,        !- Field 13",
        "    For: WinterDesignDay,    !- Field 15",
        "    Until: 24:00,1.0;        !- Field 16",

        "  Sizing:Parameters,",
        "    1.2,                     !- Heating Sizing Factor",
        "    1.2;                     !- Cooling Sizing Factor",

        "AvailabilityManagerAssignmentList,",
        "  Sys 2 Furnace DX Cool MultiSpd Availability Managers,    !- Name",
        "  AvailabilityManager:Scheduled,                           !- Availability Manager Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Availability;             !- Availability Manager Name",

        "AvailabilityManager:Scheduled,",
        "  Sys 2 Furnace DX Cool MultiSpd Availability,             !- Name",
        "  HVACTemplate-Always 1;                                   !- Schedule Name",

        "Schedule:Compact,",
        "  HVACTemplate-Always 1,                                   !- Name",
        "  HVACTemplate Any Number,                                 !- Schedule Type Limits Name",
        "  Through: 12/31,                                          !- Field 1",
        "  For: AllDays,                                            !- Field 2",
        "  Until: 24:00,                                            !- Field 3",
        "  1;                                                       !- Field 4",

        "AirLoopHVAC:UnitarySystem,",
        "  Unitary System Model,                                    !- Name",
        "  Load,                                                    !- Control Type",
        "  SPACE2-1,                                                !- Controlling Zone or Thermostat Location",
        "  None,                                                    !- Dehumidification Control Type",
        "  ,                                                        !- Availability Schedule Name",
        "  SPACE2-1 Zone Exhaust Node,                              !- Air Inlet Node Name",
        "  SPACE2-1 Zone Inlet Node,                                !- Air Outlet Node Name",
        "  Fan:VariableVolume,                                      !- Supply Fan Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Supply Fan,               !- Supply Fan Name",
        "  DrawThrough,                                             !- Fan Placement",
        "  FanAvailSched,                                           !- Supply Air Fan Operating Mode Schedule Name",
        "  Coil:Heating:Electric,                                   !- Heating Coil Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Heating Coil,             !- Heating Coil Name",
        "  1.0,                                                     !- DX Heating Coil Sizing Ratio",
        "  Coil:Cooling:DX:MultiSpeed,                              !- Cooling Coil Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil,             !- Cooling Coil Name",
        "  ,                                                        !- Use DOAS DX Cooling Coil",
        "  ,                                                        !- DOAS DX Cooling Coil Leaving Minimum Air Temperature {C}",
        "  ,                                                        !- Latent Load Control",
        "  ,                                                        !- Supplemental Heating Coil Object Type",
        "  ,                                                        !- Supplemental Heating Coil Name",
        "  SupplyAirFlowRate,                                       !- Cooling Supply Air Flow Rate Method",
        "  0.23122,                                                 !- Cooling Supply Air Flow Rate {m3/s}",
        "  ,                                                        !- Cooling Supply Air Flow Rate Per Floor Area {m3/s-m2",
        "  ,                                                        !- Cooling Fraction of Autosized Cooling Supply Air Flow Rate",
        "  ,                                                        !- Cooling Supply Air Flow Rate Per Unit of Capacity {m3/s-W",
        "  SupplyAirFlowRate,                                       !- Heating Supply Air Flow Rate Method",
        "  0.23122,                                                 !- Heating Supply Air Flow Rate {m3/s}",
        "  ,                                                        !- Heating Supply Air Flow Rate Per Floor Area {m3/s-m2",
        "  ,                                                        !- Heating Fraction of Autosized Heating Supply Air Flow Rate",
        "  ,                                                        !- Heating Supply Air Flow Rate Per Unit of Capacity {m3/s-W",
        "  SupplyAirFlowRate,                                       !- No Load Supply Air Flow Rate Method",
        "  0.23122,                                                 !- No Load Supply Air Flow Rate {m3/s}",
        "  ,                                                        !- No Load Supply Air Flow Rate Per Floor Area {m3/s-m2",
        "  ,                                                        !- No Load Fraction of Autosized Cooling Supply Air Flow Rate",
        "  ,                                                        !- No Load Fraction of Autosized Heating Supply Air Flow Rate",
        "  ,                                                        !- No Load Supply Air Flow Rate Per Unit of Capacity During Cooling Operation "
        "{m3/s-W}",
        "  ,                                                        !- No Load Supply Air Flow Rate Per Unit of Capacity During Heating Operation "
        "{m3/s-W}",
        "  No,                                                      !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "  Autosize,                                                !- Maximum Supply Air Temperature {C}",
        "  21,                                                      !- Maximum Outdoor Dry-Bulb Temperature for Supplemental Heater Operation {C}",
        "  ,                                                        !- Outdoor Dry-Bulb Temperature Sensor Node Name",
        "  ,                                                        !- Ancilliary On-Cycle Electric Power",
        "  ,                                                        !- Ancilliary Off-Cycle Electric Power",
        "  ,                                                        !- Design Heat Recovery Water Flow Rate",
        "  ,                                                        !- Maximum Temperature for Heat Recovery",
        "  ,                                                        !- Heat Recovery Water Inlet Node Name",
        "  ,                                                        !- Heat Recovery Water Outlet Node Name",
        "  UnitarySystemPerformance:Multispeed,                     !- Design Specification Multispeed Object Type",
        "  Sys 2 Furnace DX Cool MultiSpd Unitary System MultiSpeed Performance;  !- Design Specification Multispeed Object Name",

        "UnitarySystemPerformance:Multispeed,",
        "  Sys 2 Furnace DX Cool MultiSpd Unitary System MultiSpeed Performance,  !- Name",
        "  1,                                                       !- Number of Speeds for Heating",
        "  3,                                                       !- Number of Speeds for Cooling",
        "  No,                                                      !- Single Mode Operation",
        "  ,                                                        !- No Load Supply Air Flow Rate Ratio",
        "  1.0,                                                     !- Heating Speed 1 Supply Air Flow Ratio",
        "  0.333,                                                   !- Cooling Speed 1 Supply Air Flow Ratio",
        "  1.0,                                                     !- Heating Speed 2 Supply Air Flow Ratio",
        "  0.666,                                                   !- Cooling Speed 2 Supply Air Flow Ratio",
        "  1.0,                                                     !- Heating Speed 3 Supply Air Flow Ratio",
        "  1.0;                                                     !- Cooling Speed 3 Supply Air Flow Ratio",

        "Coil:Cooling:DX:MultiSpeed,",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil,             !- Name",
        "  ,                                                        !- Availability Schedule Name",
        "  SPACE2-1 Zone Exhaust Node,                              !- Air Inlet Node Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil Outlet,      !- Air Outlet Node Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil Condenser Inlet,  !- Condenser Air Inlet Node Name",
        "  AirCooled,                                               !- Condenser Type",
        "  ,                                                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "  ,                                                        !- Supply Water Storage Tank Name",
        "  ,                                                        !- Condensate Collection Water Storage Tank Name",
        "  No,                                                      !- Apply Part Load Fraction to Speeds Greater than 1",
        "  No,                                                      !- Apply Latent Degradation to Speeds Greater than 1",
        "  0.0,                                                     !- Crankcase Heater Capacity {W}",
        "  ,                                                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "  10.0,                                                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "  ,                                                        !- Basin Heater Capacity {W/K}",
        "  ,                                                        !- Basin Heater Setpoint Temperature {C}",
        "  ,                                                        !- Basin Heater Operating Schedule Name",
        "  Electricity,                                             !- Fuel Type",
        "  3,                                                       !- Number of Speeds",
        "  9229.76,                                                 !- Speed 1 Gross Rated Total Cooling Capacity {W}",
        "  0.7297,                                                  !- Speed 1 Gross Rated Sensible Heat Ratio",
        "  3.8,                                                     !- Speed 1 Gross Rated Cooling COP {W/W}",
        "  0.544,                                                   !- Speed 1 Rated Air Flow Rate {m3/s}",
        "  ,                                                        !- 2017 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  333.4,                                                   !- 2023 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FT,         !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FF,         !- Speed 1 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FT,         !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FF,         !- Speed 1 Energy Input Ratio Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil PLF,            !- Speed 1 Part Load Fraction Correlation Curve Name",
        "  0,                                                       !- Speed 1 Nominal Time for Condensate Removal to Begin",
        "  0,                                                       !- Speed 1 Ratio of Initial Moisture Evaporation Rate and Steady State Latent "
        "Capacity",
        "  0,                                                       !- Speed 1 Maximum Cycling Rate",
        "  0,                                                       !- Speed 1 Latent Capacity Time Constant",
        "  0.2,                                                     !- Speed 1 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil WH-FT,          !- Speed 1 Waste Heat Function of Temperature Curve Name",
        "  ,                                                        !- Speed 1 Evaporative Condenser Effectiveness {dimensionless}",
        "  ,                                                        !- Speed 1 Evaporative Condenser Air Flow Rate {m3/s}",
        "  ,                                                        !- Speed 1 Rated Evaporative Condenser Pump Power Consumption {W}",
        "  18459.522,                                               !- Speed 2 Gross Rated Total Cooling Capacity {W}",
        "  0.7297,                                                  !- Speed 2 Gross Rated Sensible Heat Ratio",
        "  3.8,                                                     !- Speed 2 Gross Rated Cooling COP {W/W}",
        "  0.95,                                                     !- Speed 2 Rated Air Flow Rate {m3/s}",
        "  ,                                                        !- 2017 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  663.4,                                                   !- 2023 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FT,         !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FF,         !- Speed 2 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FT,         !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FF,         !- Speed 2 Energy Input Ratio Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil PLF,            !- Speed 2 Part Load Fraction Correlation Curve Name",
        "  0,                                                       !- Speed 2 Nominal Time for Condensate Removal to Begin",
        "  0,                                                       !- Speed 2 Ratio of Initial Moisture Evaporation Rate and Steady State Latent "
        "Capacity",
        "  0,                                                       !- Speed 2 Maximum Cycling Rate",
        "  0,                                                       !- Speed 2 Latent Capacity Time Constant",
        "  0.2,                                                     !- Speed 2 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil WH-FT,          !- Speed 2 Waste Heat Function of Temperature Curve Name",
        "  ,                                                        !- Speed 2 Evaporative Condenser Effectiveness {dimensionless}",
        "  ,                                                        !- Speed 2 Evaporative Condenser Air Flow Rate {m3/s}",
        "  ,                                                        !- Speed 2 Rated Evaporative Condenser Pump Power Consumption {W}",
        "  27717.18,                                                !- Speed 3 Gross Rated Total Cooling Capacity {W}",
        "  0.7297,                                                  !- Speed 3 Gross Rated Sensible Heat Ratio",
        "  3.8,                                                     !- Speed 3 Gross Rated Cooling COP {W/W}",
        "  1.36,                                                    !- Speed 3 Rated Air Flow Rate {m3/s}",
        "  ,                                                        !- 2017 Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  934.4,                                                   !- 2023 Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FT,         !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FF,         !- Speed 3 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FT,         !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FF,         !- Speed 3 Energy Input Ratio Function of Flow Fraction Curve Name",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil PLF,            !- Speed 3 Part Load Fraction Correlation Curve Name",
        "  0,                                                       !- Speed 3 Nominal Time for Condensate Removal to Begin",
        "  0,                                                       !- Speed 3 Ratio of Initial Moisture Evaporation Rate and Steady State Latent "
        "Capacity",
        "  0,                                                       !- Speed 3 Maximum Cycling Rate",
        "  0,                                                       !- Speed 3 Latent Capacity Time Constant",
        "  0.2,                                                     !- Speed 3 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil WH-FT,          !- Speed 3 Waste Heat Function of Temperature Curve Name",
        "  ,                                                        !- Speed 3 Evaporative Condenser Effectiveness {dimensionless}",
        "  ,                                                        !- Speed 3 Evaporative Condenser Air Flow Rate {m3/s}",
        "  ;                                                        !- Speed 3 Rated Evaporative Condenser Pump Power Consumption {W}",

        "Curve:Biquadratic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FT,         !- Name",
        "  0.476428E+00,                                            !- Coefficient1 Constant",
        "  0.401147E-01,                                            !- Coefficient2 x",
        "  0.226411E-03,                                            !- Coefficient3 x**2",
        "  -0.827136E-03,                                           !- Coefficient4 y",
        "  -0.732240E-05,                                           !- Coefficient5 y**2",
        "  -0.446278E-03,                                           !- Coefficient6 x*y",
        "  0.0,                                                     !- Minimum Value of x",
        "  50.0,                                                    !- Maximum Value of x",
        "  0.0,                                                     !- Minimum Value of y",
        "  50.0,                                                    !- Maximum Value of y",
        "  0.0,                                                     !- Minimum Curve Output",
        "  5.0,                                                     !- Maximum Curve Output",
        "  Temperature,                                             !- Input Unit Type for X",
        "  Temperature,                                             !- Input Unit Type for Y",
        "  Dimensionless;                                           !- Output Unit Type",

        "Curve:Cubic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil Cap-FF,         !- Name",
        "  .47278589,                                               !- Coefficient1 Constant",
        "  1.2433415,                                               !- Coefficient2 x",
        "  -1.0387055,                                              !- Coefficient3 x**2",
        "  .32257813,                                               !- Coefficient4 x**3",
        "  0.5,                                                     !- Minimum Value of x",
        "  1.5;                                                     !- Maximum Value of x",

        "Curve:Biquadratic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FT,         !- Name",
        "  0.632475E+00,                                            !- Coefficient1 Constant",
        "  -0.121321E-01,                                           !- Coefficient2 x",
        "  0.507773E-03,                                            !- Coefficient3 x**2",
        "  0.155377E-01,                                            !- Coefficient4 y",
        "  0.272840E-03,                                            !- Coefficient5 y**2",
        "  -0.679201E-03,                                           !- Coefficient6 x*y",
        "  0.0,                                                     !- Minimum Value of x",
        "  50.0,                                                    !- Maximum Value of x",
        "  0.0,                                                     !- Minimum Value of y",
        "  50.0,                                                    !- Maximum Value of y",
        "  0.0,                                                     !- Minimum Curve Output",
        "  5.0,                                                     !- Maximum Curve Output",
        "  Temperature,                                             !- Input Unit Type for X",
        "  Temperature,                                             !- Input Unit Type for Y",
        "  Dimensionless;                                           !- Output Unit Type",

        "Curve:Cubic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil EIR-FF,         !- Name",
        "  .47278589,                                               !- Coefficient1 Constant",
        "  1.2433415,                                               !- Coefficient2 x",
        "  -1.0387055,                                              !- Coefficient3 x**2",
        "  .32257813,                                               !- Coefficient4 x**3",
        "  0.5,                                                     !- Minimum Value of x",
        "  1.5;                                                     !- Maximum Value of x",

        "Curve:Quadratic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil PLF,            !- Name",
        "  0.85,                                                    !- Coefficient1 Constant",
        "  0.15,                                                    !- Coefficient2 x",
        "  0,                                                       !- Coefficient3 x**2",
        "  0,                                                       !- Minimum Value of x",
        "  1;                                                       !- Maximum Value of x",

        "Curve:Biquadratic,",
        "  Sys 2 Furnace DX Cool MultiSpd Cool Coil WH-FT,          !- Name",
        "  1.0,                                                     !- Coefficient1 Constant",
        "  0.0,                                                     !- Coefficient2 x",
        "  0.0,                                                     !- Coefficient3 x**2",
        "  0.0,                                                     !- Coefficient4 y",
        "  0.0,                                                     !- Coefficient5 y**2",
        "  0.0,                                                     !- Coefficient6 x*y",
        "  0,                                                       !- Minimum Value of x",
        "  50,                                                      !- Maximum Value of x",
        "  0,                                                       !- Minimum Value of y",
        "  50,                                                      !- Maximum Value of y",
        "  ,                                                        !- Minimum Curve Output",
        "  ,                                                        !- Maximum Curve Output",
        "  Temperature,                                             !- Input Unit Type for X",
        "  Temperature,                                             !- Input Unit Type for Y",
        "  Dimensionless;                                           !- Output Unit Type",

        "OutdoorAir:Node,",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil Condenser Inlet,  !- Name",
        "  -1;                                                      !- Height Above Ground",

        "Coil:Heating:Electric,",
        "  Sys 2 Furnace DX Cool MultiSpd Heating Coil,             !- Name",
        "  ,                                                        !- Availability Schedule Name",
        "  1,                                                       !- Efficiency",
        "  0.23122,                                                 !- Nominal Capacity of the Coil {W}",
        "  Sys 2 Furnace DX Cool MultiSpd Cooling Coil Outlet,      !- Air Inlet Node Name",
        "  Sys 2 Furnace DX Cool MultiSpd Heating Coil Outlet,      !- Air Outlet Node Name",
        "  ;                                                        !- Coil Temp Setpoint Node",

        "Fan:VariableVolume,",
        "  Sys 2 Furnace DX Cool MultiSpd Supply Fan,               !- Name",
        "  HVACTemplate-Always 1,                                   !- Availability Schedule Name",
        "  0.7,                                                     !- Fan Efficiency",
        "  600,                                                     !- Pressure Rise {Pa}",
        "  0.23122,                                                 !- Maximum Flow Rate {m3/s}",
        "  Fraction,                                                !- Fan Power Minimum Flow Rate Input Method",
        "  0.0,                                                     !- Fan Power Minimum Flow Fraction",
        "  ,                                                        !- Fan Power Minimum Air Flow Rate {m3/s}",
        "  0.9,                                                     !- Motor Efficiency",
        "  1,                                                       !- Motor in Airstream Fraction",
        "  0.0015302446,                                            !- Fan Power Coefficient 1",
        "  0.0052080574,                                            !- Fan Power Coefficient 2",
        "  1.1086242,                                               !- Fan Power Coefficient 3",
        "  -0.11635563,                                             !- Fan Power Coefficient 4",
        "  0,                                                       !- Fan Power Coefficient 5",
        "  Sys 2 Furnace DX Cool MultiSpd Heating Coil Outlet,      !- Air Inlet Node Name",
        "  SPACE2-1 Zone Inlet Node;                                !- Air Outlet Node Name",

        "OutdoorAir:NodeList,",
        "  Sys 2 Furnace DX Cool MultiSpd Outdoor Air Inlet;        !- Node or NodeList Name 1",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetDXCoils(*state);
    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.MSPLFFPLR(1)));
    // ckeck user PLF curve coefficients | HPACCOOLPLFFPLR Speed 1
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.MSPLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // check Gross Rated Total cooling Capacity
    EXPECT_EQ(9229.76, thisCoil.MSRatedTotCap(1));
    EXPECT_EQ(18459.522, thisCoil.MSRatedTotCap(2));

    std::map<std::string, Real64> StandardRatingsResult;
    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = MultiSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.Name,
                                                                   thisCoil.MSCCapFTemp,
                                                                   thisCoil.MSCCapFFlow,
                                                                   thisCoil.MSEIRFTemp,
                                                                   thisCoil.MSEIRFFlow,
                                                                   thisCoil.MSPLFFPLR,
                                                                   thisCoil.MSRatedTotCap,
                                                                   thisCoil.MSRatedCOP,
                                                                   thisCoil.MSRatedAirVolFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
                                                                   thisCoil.NumOfSpeeds,
                                                                   thisCoil.CondenserType);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(26672.541390621947, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["SEER_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER"] > 0.0);
    EXPECT_NEAR(3.65, StandardRatingsResult["SEER_User"], 0.01);
    EXPECT_NEAR(3.61, StandardRatingsResult["SEER_Standard"], 0.01);
    EXPECT_NEAR(3.29, StandardRatingsResult["EER"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_NEAR(3.66, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.81, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(3.20, StandardRatingsResult["EER2"], 0.01);

    EXPECT_NEAR(3.07, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_NEAR(3.8899851971608825, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(26453.445390621946, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);
    EXPECT_NEAR(13.27318044298636, StandardRatingsResult["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
}

TEST_F(EnergyPlusFixture, MultiSpeedCoolingCoil_04_Speeds_35500W_COP3_IEER_2022_ValueTest)
{
    std::string const idf_objects = delimited_string({
        " Coil:Cooling:DX:MultiSpeed,",
        "    Heat Pump ACDXCoil 1,    !- Name",
        "    ,    !- Availability Schedule Name", // FanAndCoilAvailSched
        "    DX Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "    Outdoor Condenser Air Node,  !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    -8.0,                    !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    No,                      !- Apply Part Load Fraction to Speeds Greater than 1",
        "    No,                      !- Apply Latent Degradation to Speeds Greater than 1",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    4,                       !- Number of Speeds",
        "    7500,                    !- Speed 1 Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 1 Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Speed 1 Gross Rated Cooling COP {W/W}",
        "    0.40,                    !- Speed 1 Rated Air Flow Rate {m3/s}",
        "    453.3,                   !- Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    453.3,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    HPACCoolCapFT Speed 1,   !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFF Speed 1,   !- Speed 1 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACCOOLEIRFT Speed 1,   !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACCOOLEIRFF Speed 1,   !- Speed 1 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR Speed 1, !- Speed 1 Part Load Fraction Correlation Curve Name",
        "    1000.0,                  !- Speed 1 Nominal Time for Condensate Removal to Begin {s}",
        "    1.5,                     !- Speed 1 Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    3.0,                     !- Speed 1 Maximum Cycling Rate {cycles/hr}",
        "    45.0,                    !- Speed 1 Latent Capacity Time Constant {s}",
        "    0.2,                     !- Speed 1 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    HAPCCoolWHFT Speed 1,    !- Speed 1 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 1 Evaporative Condenser Effectiveness {dimensionless}",
        "    0.05,                    !- Speed 1 Evaporative Condenser Air Flow Rate {m3/s}",
        "    50,                      !- Speed 1 Rated Evaporative Condenser Pump Power Consumption {W}",
        "    17500,                   !- Speed 2 Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 2 Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Speed 2 Gross Rated Cooling COP {W/W}",
        "    0.85,                    !- Speed 2 Rated Air Flow Rate {m3/s}",
        "    523.3,                   !- Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    631.7,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    HPACCoolCapFT Speed 2,   !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFF Speed 2,   !- Speed 2 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACCOOLEIRFT Speed 2,   !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACCOOLEIRFF Speed 2,   !- Speed 2 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR Speed 1, !- Speed 2 Part Load Fraction Correlation Curve Name",
        "    1000.0,                  !- Speed 2 Nominal Time for Condensate Removal to Begin {s}",
        "    1.5,                     !- Speed 2 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "    3.0,                     !- Speed 2 Maximum Cycling Rate {cycles/hr}",
        "    45.0,                    !- Speed 2 Latent Capacity Time Constant {s}",
        "    0.2,                     !- Speed 2 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    HAPCCoolWHFT Speed 2,    !- Speed 2 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 2 Evaporative Condenser Effectiveness {dimensionless}",
        "    0.1,                     !- Speed 2 Evaporative Condenser Air Flow Rate {m3/s}",
        "    60,                      !- Speed 2 Rated Evaporative Condenser Pump Power Consumption {W}",
        "    25500,                   !- Speed 3 Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 3 Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Speed 3 Gross Rated Cooling COP {W/W}",
        "    1.25,                    !- Speed 3 Rated Air Flow Rate {m3/s}",
        "    573.3,                   !- Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    692.1,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    HPACCoolCapFT Speed 3,   !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFF Speed 3,   !- Speed 3 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACCOOLEIRFT Speed 3,   !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACCOOLEIRFF Speed 3,   !- Speed 3 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR Speed 1, !- Speed 3 Part Load Fraction Correlation Curve Name",
        "    1000.0,                  !- Speed 3 Nominal Time for Condensate Removal to Begin {s}",
        "    1.5,                     !- Speed 3 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "    3.0,                     !- Speed 3 Maximum Cycling Rate {cycles/hr}",
        "    45.0,                    !- Speed 3 Latent Capacity Time Constant {s}",
        "    0.2,                     !- Speed 3 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    HAPCCoolWHFT Speed 3,    !- Speed 3 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 3 Evaporative Condenser Effectiveness {dimensionless}",
        "    0.2,                     !- Speed 3 Evaporative Condenser Air Flow Rate {m3/s}",
        "    80,                      !- Speed 3 Rated Evaporative Condenser Pump Power Consumption {W}",
        "    35500,                   !- Speed 4 Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 4 Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Speed 4 Gross Rated Cooling COP {W/W}",
        "    1.75,                    !- Speed 4 Rated Air Flow Rate {m3/s}",
        "    673.3,                   !- Speed 4 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    812.9,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    HPACCoolCapFT Speed 4,   !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFF Speed 4,   !- Speed 4 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACCOOLEIRFT Speed 4,   !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACCOOLEIRFF Speed 4,   !- Speed 4 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR Speed 1, !- Speed 4 Part Load Fraction Correlation Curve Name",
        "    1000.0,                  !- Speed 4 Nominal Time for Condensate Removal to Begin {s}",
        "    1.5,                     !- Speed 4 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "    3.0,                     !- Speed 4 Maximum Cycling Rate {cycles/hr}",
        "    45.0,                    !- Speed 4 Latent Capacity Time Constant {s}",
        "    0.2,                     !- Speed 4 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    HAPCCoolWHFT Speed 4,    !- Speed 4 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 4 Evaporative Condenser Effectiveness {dimensionless}",
        "    0.3,                     !- Speed 4 Evaporative Condenser Air Flow Rate {m3/s}",
        "    100;                     !- Speed 4 Rated Evaporative Condenser Pump Power Consumption {W}",

        "  OutdoorAir:Node,",
        "    Outdoor Condenser Air Node,  !- Name",
        "    1.0;                     !- Height Above Ground {m}",

        //"  ! same as Doe-2 SDL-C4"

        "  Curve:Biquadratic,",
        "    HPACCoolCapFT Speed 1,   !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPACCoolCapFT Speed 2,   !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPACCoolCapFT Speed 3,   !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPACCoolCapFT Speed 4,   !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        //"  ! same as Doe-2 SDL-C78"

        "  Curve:Cubic,",
        "    HPACCoolCapFF Speed 1,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    HPACCoolCapFF Speed 2,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    HPACCoolCapFF Speed 3,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    HPACCoolCapFF Speed 4,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        //"  ! same as Doe-2 SDL-C82"

        "  Curve:Biquadratic,",
        "    HPACCOOLEIRFT Speed 1,   !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPACCOOLEIRFT Speed 2,   !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPACCOOLEIRFT Speed 3,   !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPACCOOLEIRFT Speed 4,   !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        //"  ! same as Doe-2 SDL-C78"

        "  Curve:Cubic,",
        "    HPACCOOLEIRFF Speed 1,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    HPACCOOLEIRFF Speed 2,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    HPACCOOLEIRFF Speed 3,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    HPACCOOLEIRFF Speed 4,   !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACCOOLPLFFPLR Speed 1, !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACCOOLPLFFPLR Speed 2, !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACCOOLPLFFPLR Speed 3, !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACCOOLPLFFPLR Speed 4, !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        //"! operation condition varies"

        "  Curve:Biquadratic,",
        "    HAPCCoolWHFT Speed 1,    !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    50,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        //"! operation condition varies"

        "  Curve:Biquadratic,",
        "    HAPCCoolWHFT Speed 2,    !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    50,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        //"! operation condition varies"

        "  Curve:Biquadratic,",
        "    HAPCCoolWHFT Speed 3,    !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    50,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        //"! operation condition varies"

        "  Curve:Biquadratic,",
        "    HAPCCoolWHFT Speed 4,    !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    50,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetDXCoils(*state);
    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.MSPLFFPLR(1)));
    // ckeck user PLF curve coefficients | HPACCOOLPLFFPLR Speed 1
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.MSPLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // check Gross Rated Total cooling Capacity
    EXPECT_EQ(7500, thisCoil.MSRatedTotCap(1));
    EXPECT_EQ(17500, thisCoil.MSRatedTotCap(2));
    EXPECT_EQ(25500, thisCoil.MSRatedTotCap(3));
    EXPECT_EQ(35500, thisCoil.MSRatedTotCap(4));

    std::map<std::string, Real64> StandardRatingsResult;
    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = MultiSpeedDXCoolingCoilStandardRatings(*state,
                                                                   thisCoil.DXCoilType,
                                                                   thisCoil.Name,
                                                                   thisCoil.MSCCapFTemp,
                                                                   thisCoil.MSCCapFFlow,
                                                                   thisCoil.MSEIRFTemp,
                                                                   thisCoil.MSEIRFFlow,
                                                                   thisCoil.MSPLFFPLR,
                                                                   thisCoil.MSRatedTotCap,
                                                                   thisCoil.MSRatedCOP,
                                                                   thisCoil.MSRatedAirVolFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate,
                                                                   thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
                                                                   thisCoil.NumOfSpeeds,
                                                                   thisCoil.CondenserType);

    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(34330.753817761368, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["SEER_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER"] > 0.0);
    EXPECT_NEAR(3.14, StandardRatingsResult["SEER_User"], 0.01);
    EXPECT_NEAR(3.12, StandardRatingsResult["SEER_Standard"], 0.01);
    EXPECT_NEAR(2.70, StandardRatingsResult["EER"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_NEAR(2.89, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.07, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(2.65, StandardRatingsResult["EER2"], 0.01);

    EXPECT_NEAR(2.56, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_NEAR(3.11, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(34086.45, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);
    EXPECT_NEAR(10.62, StandardRatingsResult["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
}

TEST_F(EnergyPlusFixture, MultiSpeedCoolingCoil_04_Speed_35500W_COP4_IEER_2022_ValueTest)
{
    // Tests SEER Value calculation for multi speed DX cooling coil
    // using user's PLF curve and AHRI default PLF curve

    std::string const idf_objects = delimited_string({
        "   Coil:Cooling:DX:MultiSpeed,",
        "     MS DX Cooling Coil,      !- Name",
        "     ,                        !- Availability Schedule Name",
        "     MSDXClgCoil AirInletNode,!- Air Inlet Node Name",
        "     MSDXHtgCoil AirInletNode,!- Air Outlet Node Name",
        "     Condenser Air Inlet Node,!- Condenser Air Inlet Node Name",
        "     AirCooled,               !- Condenser Type",
        "     -8.0,                    !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "     ,                        !- Supply Water Storage Tank Name",
        "     ,                        !- Condensate Collection Water Storage Tank Name",
        "     No,                      !- Apply Part Load Fraction to Speeds Greater than 1",
        "     No,                      !- Apply Latent Degradation to Speeds Greater than 1",
        "     200.0,                   !- Crankcase Heater Capacity {W}",
        "     ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "     10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "     ,                        !- Basin Heater Capacity {W/K}",
        "     ,                        !- Basin Heater Setpoint Temperature {C}",
        "     ,                        !- Basin Heater Operating Schedule Name",
        "     NaturalGas,              !- Fuel Type",
        "     4,                       !- Number of Speeds",
        "     7500,                    !- Speed 1 Gross Rated Total Cooling Capacity {W}",
        "     0.75,                    !- Speed 1 Gross Rated Sensible Heat Ratio",
        "     4.0,                     !- Speed 1 Gross Rated Cooling COP {W/W}",
        "     0.40,                    !- Speed 1 Rated Air Flow Rate {m3/s}",
        "     453.3,                   !- 2017 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "     453.3,                   !- 2023 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}", //??TBD:BPS
        "     HPACCoolCapFT Speed,     !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "     HPACCoolCapFF Speed,     !- Speed 1 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "     HPACCOOLEIRFT Speed,     !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "     HPACCOOLEIRFF Speed,     !- Speed 1 Energy Input Ratio Function of Flow Fraction Curve Name",
        "     HPACCOOLPLFFPLR Speed,   !- Speed 1 Part Load Fraction Correlation Curve Name",
        "     1000.0,                  !- Speed 1 Nominal Time for Condensate Removal to Begin {s}",
        "     1.5,                     !- Speed 1 Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "     3.0,                     !- Speed 1 Maximum Cycling Rate {cycles/hr}",
        "     45.0,                    !- Speed 1 Latent Capacity Time Constant {s}",
        "     0.2,                     !- Speed 1 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "     HAPCCoolWHFT Speed,      !- Speed 1 Waste Heat Function of Temperature Curve Name",
        "     0.9,                     !- Speed 1 Evaporative Condenser Effectiveness {dimensionless}",
        "     0.05,                    !- Speed 1 Evaporative Condenser Air Flow Rate {m3/s}",
        "     50,                      !- Speed 1 Rated Evaporative Condenser Pump Power Consumption {W}",
        "     17500,                   !- Speed 2 Gross Rated Total Cooling Capacity {W}",
        "     0.75,                    !- Speed 2 Gross Rated Sensible Heat Ratio",
        "     4.0,                     !- Speed 2 Gross Rated Cooling COP {W/W}",
        "     0.85,                    !- Speed 2 Rated Air Flow Rate {m3/s}",
        "     523.3,                   !- 2017 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "     523.3,                   !- 2023 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}", //??TBD:BPS
        "     HPACCoolCapFT Speed,     !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "     HPACCoolCapFF Speed,     !- Speed 2 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "     HPACCOOLEIRFT Speed,     !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "     HPACCOOLEIRFF Speed,     !- Speed 2 Energy Input Ratio Function of Flow Fraction Curve Name",
        "     HPACCOOLPLFFPLR Speed,   !- Speed 2 Part Load Fraction Correlation Curve Name",
        "     1000.0,                  !- Speed 2 Nominal Time for Condensate Removal to Begin {s}",
        "     1.5,                     !- Speed 2 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "     3.0,                     !- Speed 2 Maximum Cycling Rate {cycles/hr}",
        "     45.0,                    !- Speed 2 Latent Capacity Time Constant {s}",
        "     0.2,                     !- Speed 2 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "     HAPCCoolWHFT Speed,      !- Speed 2 Waste Heat Function of Temperature Curve Name",
        "     0.9,                     !- Speed 2 Evaporative Condenser Effectiveness {dimensionless}",
        "     0.1,                     !- Speed 2 Evaporative Condenser Air Flow Rate {m3/s}",
        "     60,                      !- Speed 2 Rated Evaporative Condenser Pump Power Consumption {W}",
        "     25500,                   !- Speed 3 Gross Rated Total Cooling Capacity {W}",
        "     0.75,                    !- Speed 3 Gross Rated Sensible Heat Ratio",
        "     4.0,                     !- Speed 3 Gross Rated Cooling COP {W/W}",
        "     1.25,                    !- Speed 3 Rated Air Flow Rate {m3/s}",
        "     573.3,                   !- 2017 Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "     573.3,                   !- 2023 Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}", //??TBD:BPS
        "     HPACCoolCapFT Speed,     !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "     HPACCoolCapFF Speed,     !- Speed 3 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "     HPACCOOLEIRFT Speed,     !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "     HPACCOOLEIRFF Speed,     !- Speed 3 Energy Input Ratio Function of Flow Fraction Curve Name",
        "     HPACCOOLPLFFPLR Speed,   !- Speed 3 Part Load Fraction Correlation Curve Name",
        "     1000.0,                  !- Speed 3 Nominal Time for Condensate Removal to Begin {s}",
        "     1.5,                     !- Speed 3 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "     3.0,                     !- Speed 3 Maximum Cycling Rate {cycles/hr}",
        "     45.0,                    !- Speed 3 Latent Capacity Time Constant {s}",
        "     0.2,                     !- Speed 3 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "     HAPCCoolWHFT Speed,      !- Speed 3 Waste Heat Function of Temperature Curve Name",
        "     0.9,                     !- Speed 3 Evaporative Condenser Effectiveness {dimensionless}",
        "     0.2,                     !- Speed 3 Evaporative Condenser Air Flow Rate {m3/s}",
        "     80,                      !- Speed 3 Rated Evaporative Condenser Pump Power Consumption {W}",
        "     35500,                   !- Speed 4 Gross Rated Total Cooling Capacity {W}",
        "     0.75,                    !- Speed 4 Gross Rated Sensible Heat Ratio",
        "     4.0,                     !- Speed 4 Gross Rated Cooling COP {W/W}",
        "     1.75,                    !- Speed 4 Rated Air Flow Rate {m3/s}",
        "     673.3,                   !- 2017 Speed 4 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "     673.3,                   !- 2023 Speed 4 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}", //??TBD:BPS
        "     HPACCoolCapFT Speed,     !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "     HPACCoolCapFF Speed,     !- Speed 4 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "     HPACCOOLEIRFT Speed,     !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "     HPACCOOLEIRFF Speed,     !- Speed 4 Energy Input Ratio Function of Flow Fraction Curve Name",
        "     HPACCOOLPLFFPLR Speed,   !- Speed 4 Part Load Fraction Correlation Curve Name",
        "     1000.0,                  !- Speed 4 Nominal Time for Condensate Removal to Begin {s}",
        "     1.5,                     !- Speed 4 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "     4.0,                     !- Speed 4 Maximum Cycling Rate {cycles/hr}",
        "     45.0,                    !- Speed 4 Latent Capacity Time Constant {s}",
        "     0.2,                     !- Speed 4 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "     HAPCCoolWHFT Speed,      !- Speed 4 Waste Heat Function of Temperature Curve Name",
        "     0.9,                     !- Speed 4 Evaporative Condenser Effectiveness {dimensionless}",
        "     0.3,                     !- Speed 4 Evaporative Condenser Air Flow Rate {m3/s}",
        "     100;                     !- Speed 4 Rated Evaporative Condenser Pump Power Consumption {W}",

        "   OutdoorAir:Node,",
        "     Outdoor Condenser Air Node,  !- Name",
        "     1.0;                     !- Height Above Ground {m}",

        "   Curve:Biquadratic,",
        "     HPACCoolCapFT Speed,     !- Name",
        "     0.476428E+00,            !- Coefficient1 Constant",
        "     0.401147E-01,            !- Coefficient2 x",
        "     0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,            !- Coefficient4 y",
        "    -0.732240E-05,            !- Coefficient5 y**2",
        "    -0.446278E-03,            !- Coefficient6 x*y",
        "     12.77778,                !- Minimum Value of x",
        "     23.88889,                !- Maximum Value of x",
        "     23.88889,                !- Minimum Value of y",
        "     46.11111,                !- Maximum Value of y",
        "     ,                        !- Minimum Curve Output",
        "     ,                        !- Maximum Curve Output",
        "     Temperature,             !- Input Unit Type for X",
        "     Temperature,             !- Input Unit Type for Y",
        "     Dimensionless;           !- Output Unit Type",

        "   Curve:Cubic,",
        "     HPACCoolCapFF Speed,     !- Name",
        "     0.47278589,              !- Coefficient1 Constant",
        "     1.2433415,               !- Coefficient2 x",
        "    -1.0387055,               !- Coefficient3 x**2",
        "     0.32257813,              !- Coefficient4 x**3",
        "     0.5,                     !- Minimum Value of x",
        "     1.5;                     !- Maximum Value of x",

        "   Curve:Biquadratic,",
        "     HPACCOOLEIRFT Speed,     !- Name",
        "     0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,            !- Coefficient2 x",
        "     0.507773E-03,            !- Coefficient3 x**2",
        "     0.155377E-01,            !- Coefficient4 y",
        "     0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,            !- Coefficient6 x*y",
        "     12.77778,                !- Minimum Value of x",
        "     23.88889,                !- Maximum Value of x",
        "     23.88889,                !- Minimum Value of y",
        "     46.11111,                !- Maximum Value of y",
        "     ,                        !- Minimum Curve Output",
        "     ,                        !- Maximum Curve Output",
        "     Temperature,             !- Input Unit Type for X",
        "     Temperature,             !- Input Unit Type for Y",
        "     Dimensionless;           !- Output Unit Type",

        "   Curve:Cubic,",
        "     HPACCOOLEIRFF Speed,     !- Name",
        "     0.47278589,              !- Coefficient1 Constant",
        "     1.2433415,               !- Coefficient2 x",
        "    -1.0387055,               !- Coefficient3 x**2",
        "     0.32257813,              !- Coefficient4 x**3",
        "     0.5,                     !- Minimum Value of x",
        "     1.5;                     !- Maximum Value of x",

        "   Curve:Linear,",
        "     HPACCOOLPLFFPLR Speed,   !- Name",
        "     0.90,                    !- Coefficient1 Constant",
        "     0.10,                    !- Coefficient2 x",
        "     0.0,                     !- Minimum Value of x",
        "     1.0;                     !- Maximum Value of x",

        "   Curve:Biquadratic,",
        "     HAPCCoolWHFT Speed,      !- Name",
        "     1.0,                     !- Coefficient1 Constant",
        "     0.0,                     !- Coefficient2 x",
        "     0.0,                     !- Coefficient3 x**2",
        "     0.0,                     !- Coefficient4 y",
        "     0.0,                     !- Coefficient5 y**2",
        "     0.0,                     !- Coefficient6 x*y",
        "     0,                       !- Minimum Value of x",
        "     50,                      !- Maximum Value of x",
        "     0,                       !- Minimum Value of y",
        "     50,                      !- Maximum Value of y",
        "     ,                        !- Minimum Curve Output",
        "     ,                        !- Maximum Curve Output",
        "     Temperature,             !- Input Unit Type for X",
        "     Temperature,             !- Input Unit Type for Y",
        "     Dimensionless;           !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.MSPLFFPLR(1)));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.90, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.10, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.MSPLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    std::map<std::string, Real64> StandardRatingsResult;
    Array1D<Real64> NetCoolingCapRated(thisCoil.NumOfSpeeds);
    Real64 IEER_2022(0.0);
    Real64 EER_2022(0.0);
    Real64 NetCoolingCapRated2022(0.0);
    std::tie(IEER_2022, NetCoolingCapRated2022, EER_2022) = IEERCalculationMultiSpeed(*state,
                                                                                      thisCoil.DXCoilType,
                                                                                      thisCoil.NumOfSpeeds,
                                                                                      thisCoil.MSCCapFTemp,
                                                                                      thisCoil.MSRatedTotCap,
                                                                                      thisCoil.MSCCapFFlow,
                                                                                      thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
                                                                                      thisCoil.MSRatedAirVolFlowRate,
                                                                                      thisCoil.MSEIRFTemp,
                                                                                      thisCoil.MSRatedCOP,
                                                                                      thisCoil.MSEIRFFlow,
                                                                                      thisCoil.CondenserType);
    NetCoolingCapRated(thisCoil.NumOfSpeeds) = NetCoolingCapRated2022;
    EXPECT_TRUE(IEER_2022 > 0.0);
    EXPECT_TRUE(EER_2022 > 0.0);
    EXPECT_TRUE(NetCoolingCapRated2022 > 0.0);
    EXPECT_NEAR(4.1206483269988903, IEER_2022, 0.01);
    EXPECT_NEAR(3.4007815026870829, EER_2022, 0.01);
    EXPECT_NEAR(34330.75, NetCoolingCapRated(thisCoil.NumOfSpeeds), 0.01);
    EXPECT_NEAR(14.060235711504712, IEER_2022 * StandardRatings::ConvFromSIToIP, 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_01_Speed_7200W_SEER2_2023_ValueTest)
{
    // 1-Speed_7200W (Based on Cooling Coil in PkgTermHPVSAS.idf)
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:VariableSpeed,",
        "    Zone1PTHPDXCoolCoil,     !- Name",
        "    Zone1PTHPFanOutletNode,  !- Indoor Air Inlet Node Name",
        "    Zone1PTHPDXCoolCoilOutletNode,  !- Indoor Air Outlet Node Name",
        "    1,                       !- Number of Speeds {dimensionless}",
        "    1,                       !- Nominal Speed Level {dimensionless}",
        "    7200.0,                  !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}",
        "    0.8,                     !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACPLFFPLR,             !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    7200.0,                  !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.8,                     !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.6,                     !- Speed 1 Reference Unit Condenser Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF;              !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "",
        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "ZONE1PTHPDXCOOLCOIL");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(7200, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(0.0, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(0.80, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.0, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(4, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(0, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(0.0, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(0.942587793, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.009543347, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(0.342414409, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.034885008, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(0.8, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.2, thisCapFFlowHs->coeff[1]);

    //// EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.1552, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(-0.1808, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(7200, thisCoil.RatedCapCoolTotal);
    EXPECT_EQ(0.8, thisCoil.RatedAirVolFlowRate);
    EXPECT_EQ(1, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult;
    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(2.73, StandardRatingsResult["EER2"], 0.01);
    EXPECT_NEAR(2.82, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(2.65, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(9.65, StandardRatingsResult["SEER2_User"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(9.06, StandardRatingsResult["SEER2_Standard"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(6451.3230892544016, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_NEAR(2.27, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(2.53, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(6580.2030892544008, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_02_Speed_7200W_SEER2_2023_ValueTest)
{
    //  2-Speed_7200W (Based on Cooling Coil in PkgTermHPVSAS.idf) ---
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:VariableSpeed,",
        "    Zone1PTHPDXCoolCoil,     !- Name",
        "    Zone1PTHPFanOutletNode,  !- Indoor Air Inlet Node Name",
        "    Zone1PTHPDXCoolCoilOutletNode,  !- Indoor Air Outlet Node Name",
        "    2,                       !- Number of Speeds {dimensionless}",
        "    2,                       !- Nominal Speed Level {dimensionless}",
        "    7200.0,                  !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}",
        "    0.8,                     !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACPLFFPLR,             !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    2027.4,                  !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.154782,                !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.30,                    !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    6758.0,                  !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.37752,                 !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.57,                    !- Speed 2 Reference Unit Condenser Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF;              !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "",
        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "ZONE1PTHPDXCOOLCOIL");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(2027.40, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(6758, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(0.15, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.37, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(4, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(4, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(0.942587793, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.009543347, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(0.342414409, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.034885008, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(0.8, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.2, thisCapFFlowHs->coeff[1]);

    //// EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.1552, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(-0.1808, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(7200, thisCoil.RatedCapCoolTotal);
    EXPECT_EQ(0.8, thisCoil.RatedAirVolFlowRate);
    EXPECT_EQ(2, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult;
    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(2.73, StandardRatingsResult["EER2"], 0.01);
    EXPECT_NEAR(2.65, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(2.97, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(9.04, StandardRatingsResult["SEER2_User"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(10.16, StandardRatingsResult["SEER2_Standard"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(6451.3230892544016, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_NEAR(2.66, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(2.53, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(6580.20, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_03_Speed_7200W_SEER2_2023_ValueTest)
{
    // Based on Cooling Coil in PkgTermHPVSAS.idf
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:VariableSpeed,",
        "    Zone1PTHPDXCoolCoil,     !- Name",
        "    Zone1PTHPFanOutletNode,  !- Indoor Air Inlet Node Name",
        "    Zone1PTHPDXCoolCoilOutletNode,  !- Indoor Air Outlet Node Name",
        "    3,                       !- Number of Speeds {dimensionless}",
        "    3,                       !- Nominal Speed Level {dimensionless}",
        "    7200.0,                  !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}",
        "    0.8,                     !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACPLFFPLR,             !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    1700.0,                  !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.154782,                !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.30,                    !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    3400.0,                  !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.230289,                !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.38,                    !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    6800.0,                  !- Speed 3 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 3 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 3 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.37752,                 !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 3 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.57,                    !- Speed 3 Reference Unit Condenser Flow Rate {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF;              !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "",
        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "ZONE1PTHPDXCOOLCOIL");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(1700, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(3400, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(0.15, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.23, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(4, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(4, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(0.942587793, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.009543347, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(0.342414409, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.034885008, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(0.8, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.2, thisCapFFlowHs->coeff[1]);

    //// EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.1552, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(-0.1808, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(7200, thisCoil.RatedCapCoolTotal);
    EXPECT_EQ(0.8, thisCoil.RatedAirVolFlowRate);
    EXPECT_EQ(3, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult;
    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(2.73, StandardRatingsResult["EER2"], 0.01);
    EXPECT_NEAR(2.42, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(2.84, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(8.26, StandardRatingsResult["SEER2_User"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(9.69, StandardRatingsResult["SEER2_Standard"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(6451.3230892544016, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_NEAR(2.71, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(2.53, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(6580.20, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_04_Speed_7200W_SEER2_2023_ValueTest)
{
    // 4-Speed_7200W Based on Cooling Coil in PkgTermHPVSAS.idf
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:VariableSpeed,",
        "    Zone1PTHPDXCoolCoil,     !- Name",
        "    Zone1PTHPFanOutletNode,  !- Indoor Air Inlet Node Name",
        "    Zone1PTHPDXCoolCoilOutletNode,  !- Indoor Air Outlet Node Name",
        "    4,                       !- Number of Speeds {dimensionless}",
        "    4,                       !- Nominal Speed Level {dimensionless}",
        "    7200.0,                  !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}",
        "    0.8,                     !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACPLFFPLR,             !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    1700.0,                  !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.128355,                !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.26,                    !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    3400.0,                  !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.1510,                  !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.38,                    !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    5100.0,                  !- Speed 3 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 3 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 3 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.30220,                 !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 3 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.44,                    !- Speed 3 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    6800.0,                  !- Speed 4 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 4 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 4 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.37752,                 !- Speed 4 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 4 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 4 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.57,                    !- Speed 4 Reference Unit Condenser Flow Rate {m3/s}",
        "    ,                        !- Speed 4 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 4 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF;              !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "",
        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);
    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "ZONE1PTHPDXCOOLCOIL");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(1700, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(3400, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(0.12, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.151, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(4, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(4, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(0.942587793, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.009543347, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(0.342414409, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.034885008, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(0.8, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.2, thisCapFFlowHs->coeff[1]);

    //// EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.1552, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(-0.1808, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(7200, thisCoil.RatedCapCoolTotal);
    EXPECT_EQ(0.8, thisCoil.RatedAirVolFlowRate);
    EXPECT_EQ(4, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult;
    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(2.73, StandardRatingsResult["EER2"], 0.01);
    EXPECT_NEAR(3.16, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.01, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(10.81, StandardRatingsResult["SEER2_User"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(10.27, StandardRatingsResult["SEER2_Standard"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(6451.3230892544016, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_NEAR(2.84, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(2.53, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(6580.20, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_07_Speed_7200W_SEER2_2023_ValueTest)
{
    // !--- 7-Speed_7200W(Based on Cooling Coil in PkgTermHPVSAS.idf) ---
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:VariableSpeed,",
        "    Zone1PTHPDXCoolCoil,     !- Name",
        "    Zone1PTHPFanOutletNode,  !- Indoor Air Inlet Node Name",
        "    Zone1PTHPDXCoolCoilOutletNode,  !- Indoor Air Outlet Node Name",
        "    7,                       !- Number of Speeds {dimensionless}",
        "    7,                       !- Nominal Speed Level {dimensionless}",
        "    7200.0,                  !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}",
        "    0.4,                     !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACPLFFPLR,             !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    1351.6,                  !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.128355,                !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.26,                    !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    2027.4,                  !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.154782,                !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.30,                    !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    2703.2,                  !- Speed 3 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 3 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 3 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.18121,                 !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 3 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.33,                    !- Speed 3 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    3716.9,                  !- Speed 4 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 4 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 4 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.230289,                !- Speed 4 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 4 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 4 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.38,                    !- Speed 4 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 4 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 4 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    4730.6,                  !- Speed 5 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 5 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 5 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.279365,                !- Speed 5 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 5 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 5 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.44,                    !- Speed 5 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 5 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 5 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 5 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 5 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 5 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    5744.3,                  !- Speed 6 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 6 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 6 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.328443,                !- Speed 6 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 6 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 6 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.50,                    !- Speed 6 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 6 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 6 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 6 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 6 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 6 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    6758.0,                  !- Speed 7 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 7 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 7 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.37752,                 !- Speed 7 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 7 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 7 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.57,                    !- Speed 7 Reference Unit Condenser Flow Rate {m3/s}",
        "    ,                        !- Speed 7 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 7 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 7 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 7 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF;              !- Speed 7 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "",
        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "ZONE1PTHPDXCOOLCOIL");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(1351.5999999999999, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(2027.4000000000001, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(0.128355, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.154782, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(4, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(4, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(0.942587793, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.009543347, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(0.342414409, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.034885008, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(0.8, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.2, thisCapFFlowHs->coeff[1]);

    //// EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.1552, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(-0.1808, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(7200, thisCoil.RatedCapCoolTotal);
    EXPECT_EQ(0.4, thisCoil.RatedAirVolFlowRate);
    EXPECT_EQ(7, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult;
    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(3.28, StandardRatingsResult["EER2"], 0.01);
    EXPECT_NEAR(3.46, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.34, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(11.81, StandardRatingsResult["SEER2_User"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(11.42, StandardRatingsResult["SEER2_Standard"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(6825.08, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_NEAR(3.53, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(3.13, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(6889.52, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_07_Speed_14400W_SEER2_2023_ValueTest)
{
    // !--- 7-Speed_14400W(Based on Cooling Coil in PkgTermHPVSAS.idf) ---

    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:VariableSpeed,",
        "    Zone1PTHPDXCoolCoil,     !- Name",
        "    Zone1PTHPFanOutletNode,  !- Indoor Air Inlet Node Name",
        "    Zone1PTHPDXCoolCoilOutletNode,  !- Indoor Air Outlet Node Name",
        "    7,                       !- Number of Speeds {dimensionless}",
        "    7,                       !- Nominal Speed Level {dimensionless}",
        "    14400.0,                 !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}",
        "    0.75,                    !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACPLFFPLR,             !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    1351.6,                  !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.128355,                !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.26,                    !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    2027.4,                  !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.154782,                !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.30,                    !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    2703.2,                  !- Speed 3 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 3 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 3 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.18121,                 !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 3 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.33,                    !- Speed 3 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    3716.9,                  !- Speed 4 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 4 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 4 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.230289,                !- Speed 4 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 4 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 4 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.38,                    !- Speed 4 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 4 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 4 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    4730.6,                  !- Speed 5 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 5 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 5 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.279365,                !- Speed 5 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 5 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 5 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.44,                    !- Speed 5 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 5 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 5 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 5 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 5 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 5 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    5744.3,                  !- Speed 6 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 6 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 6 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.328443,                !- Speed 6 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 6 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 6 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.50,                    !- Speed 6 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 6 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 6 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 6 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 6 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 6 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    6758.0,                  !- Speed 7 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 7 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 7 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.37752,                 !- Speed 7 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 7 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 7 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.57,                    !- Speed 7 Reference Unit Condenser Flow Rate {m3/s}",
        "    ,                        !- Speed 7 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 7 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 7 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 7 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF;              !- Speed 7 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "",
        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);
    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "ZONE1PTHPDXCOOLCOIL");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(1351.5999999999999, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(2027.4000000000001, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(0.128355, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.154782, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(4, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(4, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(0.942587793, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.009543347, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(0.342414409, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.034885008, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(0.8, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.2, thisCapFFlowHs->coeff[1]);

    //// EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.1552, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(-0.1808, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(14400, thisCoil.RatedCapCoolTotal);
    EXPECT_EQ(0.75, thisCoil.RatedAirVolFlowRate);
    EXPECT_EQ(7, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult;
    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(3.31, StandardRatingsResult["EER2"], 0.01);
    EXPECT_NEAR(3.50, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.40, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(11.97, StandardRatingsResult["SEER2_User"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(11.61, StandardRatingsResult["SEER2_Standard"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(13696.88, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_NEAR(3.59, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(3.18, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(13696.88, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_10_Speed_7200W_SEER2_2023_ValueTest)
{
    // !--- Based on Cooling Coil in PkgTermHPVSAS.idf
    std::string const idf_objects = delimited_string({
        "  Coil:Cooling:DX:VariableSpeed,",
        "    Zone1PTHPDXCoolCoil,     !- Name",
        "    Zone1PTHPFanOutletNode,  !- Indoor Air Inlet Node Name",
        "    Zone1PTHPDXCoolCoilOutletNode,  !- Indoor Air Outlet Node Name",
        "    10.0,                    !- Number of Speeds {dimensionless}",
        "    10.0,                    !- Nominal Speed Level {dimensionless}",
        "    7200.0,                  !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}",
        "    0.4,                     !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACPLFFPLR,             !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    1524.1,                  !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.1359072,               !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.26,                    !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    1877.9,                  !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.151008,                !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.30,                    !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    2226.6,                  !- Speed 3 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 3 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 3 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.1661088,               !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 3 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.33,                    !- Speed 3 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    2911.3,                  !- Speed 4 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 4 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 4 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.1963104,               !- Speed 4 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 4 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 4 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.38,                    !- Speed 4 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 4 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 4 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    3581.7,                  !- Speed 5 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 5 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 5 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.226512,                !- Speed 5 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 5 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 5 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.44,                    !- Speed 5 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 5 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 5 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 5 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 5 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 5 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    4239.5,                  !- Speed 6 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 6 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 6 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.2567136,               !- Speed 6 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 6 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 6 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.50,                    !- Speed 6 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 6 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 6 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 6 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 6 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 6 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    4885.7,                  !- Speed 7 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 7 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 7 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.2869152,               !- Speed 7 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 7 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 7 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.57,                    !- Speed 7 Reference Unit Condenser Flow Rate {m3/s}",
        "    ,                        !- Speed 7 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 7 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 7 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 7 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 7 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    5520.7,                  !- Speed 8 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 8 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 8 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.3171168,               !- Speed 8 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 8 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 8 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.63,                    !- Speed 8 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 8 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 8 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 8 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 8 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 8 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    6144.8,                  !- Speed 9 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 9 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 9 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.3473184,               !- Speed 9 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 9 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 9 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.69,                    !- Speed 9 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 9 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 9 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 9 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 9 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 9 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    6758.0,                  !- Speed 10 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 10 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 10 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.37752,                 !- Speed 10 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 10 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 10 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.74,                    !- Speed 10 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 10 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 10 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 10 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 10 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF;              !- Speed 10 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "",
        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "ZONE1PTHPDXCOOLCOIL");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(1524.09, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(1877.90, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(0.128355, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.154782, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(4, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(4, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(0.942587793, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.009543347, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(0.342414409, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.034885008, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(0.8, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.2, thisCapFFlowHs->coeff[1]);

    //// EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.1552, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(-0.1808, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(7200, thisCoil.RatedCapCoolTotal);
    EXPECT_EQ(0.4, thisCoil.RatedAirVolFlowRate);
    EXPECT_EQ(10, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult;
    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(3.28, StandardRatingsResult["EER2"], 0.01);
    EXPECT_NEAR(3.50, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.36, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(11.94, StandardRatingsResult["SEER2_User"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(11.46, StandardRatingsResult["SEER2_Standard"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(6825.0830892544009, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_NEAR(3.52, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(3.13, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(6889.52, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_10_Speed_14400W_SEER2_2023_ValueTest)
{
    // Cooling Cap & Air Flow Rate Scaled
    std::string const idf_objects = delimited_string({
        "  Coil:Cooling:DX:VariableSpeed,",
        "    Zone1PTHPDXCoolCoil,     !- Name",
        "    Zone1PTHPFanOutletNode,  !- Indoor Air Inlet Node Name",
        "    Zone1PTHPDXCoolCoilOutletNode,  !- Indoor Air Outlet Node Name",
        "    10.0,                    !- Number of Speeds {dimensionless}",
        "    10.0,                    !- Nominal Speed Level {dimensionless}",
        "    14400.0,                  !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}",
        "    0.75,                     !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACPLFFPLR,             !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    1524.1,                  !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.1359072,               !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.26,                    !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    1877.9,                  !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.151008,                !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.30,                    !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    2226.6,                  !- Speed 3 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 3 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 3 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.1661088,               !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 3 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.33,                    !- Speed 3 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    2911.3,                  !- Speed 4 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 4 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 4 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.1963104,               !- Speed 4 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 4 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 4 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.38,                    !- Speed 4 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 4 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 4 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    3581.7,                  !- Speed 5 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 5 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 5 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.226512,                !- Speed 5 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 5 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 5 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.44,                    !- Speed 5 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 5 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 5 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 5 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 5 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 5 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    4239.5,                  !- Speed 6 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 6 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 6 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.2567136,               !- Speed 6 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 6 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 6 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.50,                    !- Speed 6 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 6 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 6 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 6 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 6 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 6 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    4885.7,                  !- Speed 7 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 7 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 7 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.2869152,               !- Speed 7 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 7 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 7 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.57,                    !- Speed 7 Reference Unit Condenser Flow Rate {m3/s}",
        "    ,                        !- Speed 7 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 7 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 7 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 7 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 7 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    5520.7,                  !- Speed 8 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 8 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 8 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.3171168,               !- Speed 8 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 8 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 8 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.63,                    !- Speed 8 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 8 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 8 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 8 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 8 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 8 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    6144.8,                  !- Speed 9 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 9 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 9 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.3473184,               !- Speed 9 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 9 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 9 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.69,                    !- Speed 9 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 9 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 9 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 9 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 9 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 9 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    6758.0,                  !- Speed 10 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 10 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 10 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.37752,                 !- Speed 10 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- Speed 10 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 10 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.74,                    !- Speed 10 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 10 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 10 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 10 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 10 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF;              !- Speed 10 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "",
        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
        "",
        "",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "ZONE1PTHPDXCOOLCOIL");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)
    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(1524.09, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(1877.90, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(0.1359072, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.151008, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(4.0, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(4.0, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.4, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(934.4, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(0.942587793, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.009543347, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(0.342414409, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.034885008, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(0.8, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.2, thisCapFFlowHs->coeff[1]);

    //// EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.1552, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(-0.1808, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(14400, thisCoil.RatedCapCoolTotal);
    EXPECT_EQ(0.75, thisCoil.RatedAirVolFlowRate);
    EXPECT_EQ(10, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult;

    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_NEAR(3.3198431754619815, StandardRatingsResult["EER2"], 0.01);
    EXPECT_NEAR(3.5536583254817069, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.4236942820076912, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(12.12, StandardRatingsResult["SEER2_User"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(11.68, StandardRatingsResult["SEER2_Standard"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(13696.886178508803, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_NEAR(3.58, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(3.18, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(13817.71, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_01_Speed_22000W_IEER_2022_ValueTest)
{
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:VariableSpeed,",
        "    Desiccant DXSystem VS Cooling Coil,  !- Name",
        "    Desiccant DXSystem Mixed Air Node,  !- Indoor Air Inlet Node Name",
        "    HX Process Inlet Node,   !- Indoor Air Outlet Node Name",
        "    1.0,                     !- Number of Speeds {dimensionless}",
        "    1.0,                     !- Nominal Speed Level {dimensionless}",
        "    22000,                   !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}",
        "    1.05,                    !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    1000,                    !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.4,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACCOOLPLFFPLR,         !- Energy Part Load Fraction Curve Name",
        "    Condenser Inlet Node,    !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    0.0,                     !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    22000,                   !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}", // 36991.44197
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.5,                     !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.05,                    !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}", // 3.776
        "    773.3,                   !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    10.62,                   !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPCoolingCAPFTemp4,      !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPCoolingEIRFTemp4,      !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACFFF;                 !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        "	Curve:Quadratic,",
        "    HPACCOOLPLFFPLR,         !- Name",
        "    0.75,                    !- Coefficient1 Constant",
        "    0.25,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    HPACFFF,                 !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Biquadratic,",
        "    HPCoolingEIRFTemp4,      !- Name",
        "    0.0001514017,            !- Coefficient1 Constant",
        "    0.0655062896,            !- Coefficient2 x",
        "    -0.0020370821,           !- Coefficient3 x**2",
        "    0.0067823041,            !- Coefficient4 y",
        "    0.0004087196,            !- Coefficient5 y**2",
        "    -0.0003552302,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.5141,                  !- Minimum Curve Output",
        "    1.7044,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPCoolingCAPFTemp4,      !- Name",
        "    1.3544202152,            !- Coefficient1 Constant",
        "    -0.0493402773,           !- Coefficient2 x",
        "    0.0022649843,            !- Coefficient3 x**2",
        "    0.0008517727,            !- Coefficient4 y",
        "    -0.0000426316,           !- Coefficient5 y**2",
        "    -0.0003364517,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.7923,                  !- Minimum Curve Output",
        "    1.2736,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "DESICCANT DXSYSTEM VS COOLING COIL");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)
    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.75, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.25, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(22000, thisCoil.MSRatedTotCap(1), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(1.05, thisCoil.MSRatedAirVolFlowRate(1), 0.01);

    // Rated COP
    EXPECT_NEAR(3.5, thisCoil.MSRatedCOP(1), 0.01);

    // Rated Fan Power Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.4, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(1.3544202152, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(-0.0493402773, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(0.0001514017, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.0655062896, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(1.0, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.0, thisCapFFlowHs->coeff[1]);

    //// EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.0, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(0.0, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(22000, thisCoil.RatedCapCoolTotal);
    EXPECT_EQ(1.05, thisCoil.RatedAirVolFlowRate);
    EXPECT_EQ(1, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult;
    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_NEAR(3.11, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.11, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(3.00, StandardRatingsResult["EER2"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(21185.399247418562, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(2.89, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_NEAR(3.1242000205487308, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(21016.244247418563, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);
    EXPECT_NEAR(10.66021296, StandardRatingsResult["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_02_Speed_36000W_IEER_2022_ValueTest)
{
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:VariableSpeed,",
        "            Heat Pump ACDXCoil 1,    !- Name,",
        "            DX Cooling Coil Air Inlet Node,  !- Indoor Air Inlet Node Name,",
        "            Heating Coil Air Inlet Node,  !- Indoor Air Outlet Node Name,",
        "            2.0,                     !- Number of Speeds {dimensionless},",
        "            2.0,                     !- Nominal Speed Level {dimensionless},",
        "            36000.0,                 !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W},",
        "            2.0,                     !- Rated Air Flow Rate At Selected Nominal Speed Level {m3s},",
        "            0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s},",
        "            0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless},",
        "            ,                        !- Maximum Cycling Rate,",
        "            ,                        !- Latent Capacity Time Constant,",
        "            ,                        !- Fan Delay Time,",
        "            HPACCOOLPLFFPLR,         !- Energy Part Load Fraction Curve Name,",
        "            ,                        !- Condenser Air Inlet Node Name,",
        "            AirCooled,               !- Condenser Type,",
        "            ,                        !- Evaporative Condenser Pump Rated Power Consumption {W},",
        "            200.0,                   !- Crankcase Heater Capacity {W},",
        "            ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name,",
        "            10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C},",
        "            ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C},",
        "            ,                        !- Supply Water Storage Tank Name,",
        "            ,                        !- Condensate Collection Water Storage Tank Name,",
        "            ,                        !- Basin Heater Capacity {WK},",
        "            ,                        !- Basin Heater Setpoint Temperature {C},",
        "            ,                        !- Basin Heater Operating Schedule Name,",
        "            9000.0,                  !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W},",
        "            0.5,                     !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless},",
        "            3.980488789,             !- Speed 1 Reference Unit Gross Rated Cooling COP {WW},",
        "            1.0,                     !- Speed 1 Reference Unit Rated Air Flow Rate {m3s},",
        "            453.3,                   !- Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W(m3s)},",
        "            453.3,                   !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow {W(m3s)},",
        "            2.655,                   !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3s},",
        "            ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless},",
        "            HPCoolingCAPFTemp1,      !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name,",
        "            HPACFFF,                 !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name,",
        "            HPCoolingEIRFTemp1,      !- Speed 1 Energy Input Ratio Function of Temperature Curve Name,",
        "            HPACFFF,                 !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name,",
        "            36000.0,                 !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W},",
        "            0.75,                    !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless},",
        "            3.866381837,             !- Speed 2 Reference Unit Gross Rated Cooling COP {WW},",
        "            2.0,                     !- Speed 2 Reference Unit Rated Air Flow Rate {m3s},",
        "            673.3,                   !- Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W(m3s)},",
        "            812.9,                   !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow {W(m3s)},",
        "            10.62,                   !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3s},",
        "            ,                        !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless},",
        "            HPCoolingCAPFTemp4,      !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name,",
        "            HPACFFF,                 !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name,",
        "            HPCoolingEIRFTemp4,      !- Speed 2 Energy Input Ratio Function of Temperature Curve Name,",
        "            HPACFFF;                 !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name,",

        "         Curve:Quadratic,",
        "            HPACCOOLPLFFPLR,         !- Name,",
        "            0.85,                    !- Coefficient1 Constant,",
        "            0.15,                    !- Coefficient2 x,",
        "            0.0,                     !- Coefficient3 x2,",
        "            0.0,                     !- Minimum Value of x,",
        "            1.0;                     !- Maximum Value of x,",

        "        	  Curve:Biquadratic,",
        "            HPCoolingCAPFTemp1,      !- Name,",
        "            1.6253449506,            !- Coefficient1 Constant,",
        "            -0.0786550838,           !- Coefficient2 x,",
        "            0.0030679776,            !- Coefficient3 x2,",
        "            0.0008002088,            !- Coefficient4 y,",
        "            -0.0000354060,           !- Coefficient5 y2,",
        "            -0.0003534409,           !- Coefficient6 xy,",
        "            13.89,                   !- Minimum Value of x,",
        "            22.22,                   !- Maximum Value of x,",
        "            12.78,                   !- Minimum Value of y,",
        "            51.67,                   !- Maximum Value of y,",
        "            0.8026,                  !- Minimum Curve Output,",
        "            1.2772,                  !- Maximum Curve Output,",
        "            Temperature,             !- Input Unit Type for X,",
        "            Temperature,             !- Input Unit Type for Y,",
        "            Dimensionless;           !- Output Unit Type,",

        "        	  Curve:Biquadratic,",
        "            HPCoolingEIRFTemp1,      !- Name,",
        "            -0.2808139299,           !- Coefficient1 Constant,",
        "            0.0987778868,            !- Coefficient2 x,",
        "            -0.0030382726,           !- Coefficient3 x2,",
        "            0.0057742302,            !- Coefficient4 y,",
        "            0.0004171046,            !- Coefficient5 y2,",
        "            -0.0003000325,           !- Coefficient6 xy,",
        "            13.89,                   !- Minimum Value of x,",
        "            22.22,                   !- Maximum Value of x,",
        "            12.78,                   !- Minimum Value of y,",
        "            51.67,                   !- Maximum Value of y,",
        "            0.4975,                  !- Minimum Curve Output,",
        "            1.7115,                  !- Maximum Curve Output,",
        "            Temperature,             !- Input Unit Type for X,",
        "            Temperature,             !- Input Unit Type for Y,",
        "            Dimensionless;           !- Output Unit Type,",

        "        	  Curve:Cubic,",
        "            HPACFFF,                 !- Name,",
        "            1.0,                     !- Coefficient1 Constant,",
        "            0.0,                     !- Coefficient2 x,",
        "            0.0,                     !- Coefficient3 x2,",
        "            0.0,                     !- Coefficient4 x3,",
        "            0.5,                     !- Minimum Value of x,",
        "            1.5;                     !- Maximum Value of x,",

        "        	  Curve:Biquadratic,",
        "            HPCoolingCAPFTemp4,      !- Name,",
        "            1.3544202152,            !- Coefficient1 Constant,",
        "            -0.0493402773,           !- Coefficient2 x,",
        "            0.0022649843,            !- Coefficient3 x2,",
        "            0.0008517727,            !- Coefficient4 y,",
        "            -0.0000426316,           !- Coefficient5 y2,",
        "            -0.0003364517,           !- Coefficient6 xy,",
        "            13.89,                   !- Minimum Value of x,",
        "            22.22,                   !- Maximum Value of x,",
        "            12.78,                   !- Minimum Value of y,",
        "            51.67,                   !- Maximum Value of y,",
        "            0.7923,                  !- Minimum Curve Output,",
        "            1.2736,                  !- Maximum Curve Output,",
        "            Temperature,             !- Input Unit Type for X,",
        "            Temperature,             !- Input Unit Type for Y,",
        "            Dimensionless;           !- Output Unit Type,",

        "        	  Curve:Biquadratic,",
        "            HPCoolingEIRFTemp4,      !- Name,",
        "            0.0001514017,            !- Coefficient1 Constant,",
        "            0.0655062896,            !- Coefficient2 x,",
        "            -0.0020370821,           !- Coefficient3 x2,",
        "            0.0067823041,            !- Coefficient4 y,",
        "            0.0004087196,            !- Coefficient5 y2,",
        "            -0.0003552302,           !- Coefficient6 xy,",
        "            13.89,                   !- Minimum Value of x,",
        "            22.22,                   !- Maximum Value of x,",
        "            12.78,                   !- Minimum Value of y,",
        "            51.67,                   !- Maximum Value of y,",
        "            0.5141,                  !- Minimum Curve Output,",
        "            1.7044,                  !- Maximum Curve Output,",
        "            Temperature,             !- Input Unit Type for X,",
        "            Temperature,             !- Input Unit Type for Y,",
        "            Dimensionless;           !- Output Unit Type,",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "HEAT PUMP ACDXCOIL 1");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)
    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(9000, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(36000, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(1, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(2, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(3.980488789, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(3.866381837, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(453.3, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(812.9, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(1.6253449506, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(-0.0786550838, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(-0.2808139299, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.0987778868, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(1.0, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.0, thisCapFFlowHs->coeff[1]);

    // EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.0, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(0.0, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(2, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult;

    // calculate standard ratings for multispeed DX cooling coil
    StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    // NetCoolingCapRated(thisCoil.NumOfSpeeds) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed"];

    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_NEAR(3.67, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.77, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(3.26, StandardRatingsResult["EER2"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(34649.086950321282, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(3.14, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_NEAR(4.05, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(34369.886950321277, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);
    EXPECT_NEAR(13.83, StandardRatingsResult["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_03_Speed_36000W_IEER_2022_ValueTest)
{
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:VariableSpeed,",
        "    Heat Pump ACDXCoil 1,    !- Name",
        "    DX Cooling Coil Air Inlet Node,  !- Indoor Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- Indoor Air Outlet Node Name",
        "    3.0,                     !- Number of Speeds {dimensionless}",
        "    3.0,                     !- Nominal Speed Level {dimensionless}",
        "    36000.0,                 !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}",
        "    2.0,                     !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACCOOLPLFFPLR,         !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    9000.0,                  !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.79,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.980488789,             !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.0,                     !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    453.3,                   !- Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    453.3,                   !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    2.655,                   !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPCoolingCAPFTemp1,      !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPCoolingEIRFTemp1,      !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    18000.0,                 !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.73,                    !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.08,                    !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.5,                     !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    523.3,                   !- Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    631.7,                   !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    5.31,                    !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPCoolingCAPFTemp2,      !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPCoolingEIRFTemp2,      !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    36000.0,                 !- Speed 3 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 3 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.866381837,             !- Speed 3 Reference Unit Gross Rated Cooling COP {W/W}",
        "    2.0,                     !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
        "    673.3,                   !- Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    812.9,                   !- Speed 3 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    10.62,                   !- Speed 3 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPCoolingCAPFTemp3,      !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPCoolingEIRFTemp3,      !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACFFF;                 !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        " Curve:Quadratic,",
        "    HPACCOOLPLFFPLR,         !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "	  Curve:Biquadratic,",
        "    HPCoolingCAPFTemp1,      !- Name",
        "    1.6253449506,            !- Coefficient1 Constant",
        "    -0.0786550838,           !- Coefficient2 x",
        "    0.0030679776,            !- Coefficient3 x**2",
        "    0.0008002088,            !- Coefficient4 y",
        "    -0.0000354060,           !- Coefficient5 y**2",
        "    -0.0003534409,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.8026,                  !- Minimum Curve Output",
        "    1.2772,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Biquadratic,",
        "    HPCoolingEIRFTemp1,      !- Name",
        "    -0.2808139299,           !- Coefficient1 Constant",
        "    0.0987778868,            !- Coefficient2 x",
        "    -0.0030382726,           !- Coefficient3 x**2",
        "    0.0057742302,            !- Coefficient4 y",
        "    0.0004171046,            !- Coefficient5 y**2",
        "    -0.0003000325,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.4975,                  !- Minimum Curve Output",
        "    1.7115,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Cubic,",
        "    HPACFFF,                 !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "	  Curve:Biquadratic,",
        "    HPCoolingCAPFTemp2,      !- Name",
        "    1.4240389306,            !- Coefficient1 Constant",
        "    -0.0593310687,           !- Coefficient2 x",
        "    0.0026068070,            !- Coefficient3 x**2",
        "    0.0008867551,            !- Coefficient4 y",
        "    -0.0000369191,           !- Coefficient5 y**2",
        "    -0.0003552805,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.7891,                  !- Minimum Curve Output",
        "    1.2786,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Biquadratic,",
        "    HPCoolingEIRFTemp2,      !- Name",
        "    -0.1062902436,           !- Coefficient1 Constant",
        "    0.0758517935,            !- Coefficient2 x",
        "    -0.0023342253,           !- Coefficient3 x**2",
        "    0.0085794421,            !- Coefficient4 y",
        "    0.0003952199,            !- Coefficient5 y**2",
        "    -0.0003974572,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.5088,                  !- Minimum Curve Output",
        "    1.7121,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Biquadratic,",
        "    HPCoolingCAPFTemp3,      !- Name",
        "    1.3544202152,            !- Coefficient1 Constant",
        "    -0.0493402773,           !- Coefficient2 x",
        "    0.0022649843,            !- Coefficient3 x**2",
        "    0.0008517727,            !- Coefficient4 y",
        "    -0.0000426316,           !- Coefficient5 y**2",
        "    -0.0003364517,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.7923,                  !- Minimum Curve Output",
        "    1.2736,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Biquadratic,",
        "    HPCoolingEIRFTemp3,      !- Name",
        "    0.0001514017,            !- Coefficient1 Constant",
        "    0.0655062896,            !- Coefficient2 x",
        "    -0.0020370821,           !- Coefficient3 x**2",
        "    0.0067823041,            !- Coefficient4 y",
        "    0.0004087196,            !- Coefficient5 y**2",
        "    -0.0003552302,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.5141,                  !- Minimum Curve Output",
        "    1.7044,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "HEAT PUMP ACDXCOIL 1");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)
    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(9000, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(18000, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(1.0, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(1.5, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(3.980488789, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(4.08, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(453.3, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(631.7, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(1.6253449506, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(-0.0786550838, thisCCpaFTempHS->coeff[1]);

    // EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(-0.2808139299, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.0987778868, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(1.0, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.0, thisCapFFlowHs->coeff[1]);

    // EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.0, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(0.0, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(3, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_NEAR(3.67, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.77, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(3.26, StandardRatingsResult["EER2"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(34649.086950321282, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(3.14, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_NEAR(4.0565457462887604, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(34369.88, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);
    EXPECT_NEAR(13.841508627080936, StandardRatingsResult["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_04_Speed_36000W_IEER_2022_ValueTest)
{
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:VariableSpeed,",
        "    Sys 4 Heat Pump Air Source Cooling Coil,  !- Name",
        "    Sys 4 Heat Pump Air Source Supply Fan Outlet,  !- Indoor Air Inlet Node Name",
        "    Sys 4 Heat Pump Air Source Cooling Coil Outlet,  !- Indoor Air Outlet Node Name",
        "    4,                       !- Number of Speeds {dimensionless}",
        "    4,                       !- Nominal Speed Level {dimensionless}",
        "    36000.0,                 !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}",
        "    2.0,                     !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    ,                        !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    ,                        !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    Sys 4 Heat Pump Air Source Heat Coil PLF,  !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    ,                        !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10,                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    -25,                     !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    2,                       !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    1302.02679,              !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.70108,                 !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3,                       !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.0524326,               !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    Sys 4 Heat Pump Air Source Cool Coil Cap-FT,  !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    Sys 4 Heat Pump Air Source Cool Coil Cap-FF,  !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    Sys 4 Heat Pump Air Source Cool Coil EIR-FT,  !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    Sys 4 Heat Pump Air Source Cool Coil EIR-FF,  !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    2604.05358,              !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.70108,                 !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3,                       !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.10487,                 !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    Sys 4 Heat Pump Air Source Cool Coil Cap-FT,  !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    Sys 4 Heat Pump Air Source Cool Coil Cap-FF,  !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    Sys 4 Heat Pump Air Source Cool Coil EIR-FT,  !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    Sys 4 Heat Pump Air Source Cool Coil EIR-FF,  !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    3906.08036,              !- Speed 3 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.70108,                 !- Speed 3 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3,                       !- Speed 3 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.1573,                  !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 3 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    Sys 4 Heat Pump Air Source Cool Coil Cap-FT,  !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    Sys 4 Heat Pump Air Source Cool Coil Cap-FF,  !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    Sys 3 Heat Pump Air Source Cool Coil EIR-FT,  !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    Sys 4 Heat Pump Air Source Cool Coil EIR-FF,  !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    5208.10715,              !- Speed 4 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.70108,                 !- Speed 4 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3,                       !- Speed 4 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.20973,                 !- Speed 4 Reference Unit Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 4 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 4 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 4 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 4 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    Sys 4 Heat Pump Air Source Cool Coil Cap-FT,  !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    Sys 4 Heat Pump Air Source Cool Coil Cap-FF,  !- Speed 4 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    Sys 4 Heat Pump Air Source Cool Coil EIR-FT,  !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    Sys 4 Heat Pump Air Source Cool Coil EIR-FF;  !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        "	  Curve:Quadratic,",
        "    Sys 4 Heat Pump Air Source Heat Coil PLF,  !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0,                       !- Minimum Value of x",
        "    1;                       !- Maximum Value of x",

        "	 Curve:Biquadratic,",
        "    Sys 4 Heat Pump Air Source Cool Coil Cap-FT,  !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    50.0,                    !- Maximum Value of x",
        "    0.0,                     !- Minimum Value of y",
        "    50.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    5.0,                     !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	 Curve:Biquadratic,",
        "    Sys 4 Heat Pump Air Source Cool Coil EIR-FT,  !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    50.0,                    !- Maximum Value of x",
        "    0.0,                     !- Minimum Value of y",
        "    50.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    5.0,                     !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Cubic,",
        "    Sys 4 Heat Pump Air Source Cool Coil Cap-FF,  !- Name",
        "    0.47278589,              !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    0.32257813,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    Sys 4 Heat Pump Air Source Cool Coil EIR-FF,  !- Name",
        "    0.47278589,              !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    0.32257813,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "	  Curve:Biquadratic,",
        "    Sys 3 Heat Pump Air Source Cool Coil EIR-FT,  !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    50.0,                    !- Maximum Value of x",
        "    0.0,                     !- Minimum Value of y",
        "    50.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    5.0,                     !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "SYS 4 HEAT PUMP AIR SOURCE COOLING COIL");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)
    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(1302.02679, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(2604.0535, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(0.0524326, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.10487, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(3.0, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(3.0, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(934.39999999999998, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(0.476428, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.0401147, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(0.632475, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(-0.0121321, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_NEAR(0.472786, thisCapFFlowHs->coeff[0], 0.01);
    EXPECT_NEAR(1.24334, thisCapFFlowHs->coeff[1], 0.01);

    // EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_NEAR(0.472786, thisEIRFFlowHs->coeff[0], 0.01);
    EXPECT_NEAR(1.24334, thisEIRFFlowHs->coeff[1], 0.01);

    EXPECT_EQ(4, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_NEAR(2.80, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(2.89, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(2.56, StandardRatingsResult["EER2"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(34462.55, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(2.45, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_NEAR(2.9167672861409688, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(34140.35, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);
    EXPECT_NEAR(9.9524230908140243, StandardRatingsResult["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_07_Speed_25001W_IEER_2022_ValueTest)
{
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:VariableSpeed,",
        "    Zone1PTHPDXCoolCoil,     !- Name",
        "    Zone1PTHPFanOutletNode,  !- Indoor Air Inlet Node Name",
        "    Zone1PTHPDXCoolCoilOutletNode,  !- Indoor Air Outlet Node Name",
        "    7.0,                    !- Number of Speeds {dimensionless}",
        "    7.0,                    !- Nominal Speed Level {dimensionless}",
        "    25001.3561,             !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}", //  7200.0 | 118,000 Btu/h
        "    1.6,                     !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",            // 0.4 | 3,400 scfm
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACPLFFPLR,             !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    7799.203151,             !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.575999992,               !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    162.10,                  !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.26,                    !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",       // CapFT
        "    HPACCoolCapFFF,          !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name", // CapFlow
        "    HPACEIRFT,               !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",           // EIRFTemp
        "    HPACEIRFFF,              !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",     // EIRFFLow
        "    9609.686765,             !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.639999991,             !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,             !- Speed 2 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    199.72,                  !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.30,                    !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    11394.07239,             !- Speed 3 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 3 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 3 Reference Unit Gross Rated Cooling COP {W/W}",
        "   0.703999991,              !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 3 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    236.81,                  !- Speed 3 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.33,                    !- Speed 3 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    14897.85456,             !- Speed 4 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 4 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 4 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.831999989,             !- Speed 4 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 4 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    309.63,                  !- Speed 4 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.38,                    !- Speed 4 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 4 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 4 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    18328.46003,             !- Speed 5 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 5 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 5 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.959999987,             !- Speed 5 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 5 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    380.93,                  !- Speed 5 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.44,                    !- Speed 5 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 5 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 5 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 5 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 5 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 5 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    21694.58812,             !- Speed 6 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 6 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 6 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.087999986,             !- Speed 6 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 6 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    450.89,                  !- Speed 6 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.50,                    !- Speed 6 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 6 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 6 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 6 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 6 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 6 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    25001.3561,              !- Speed 7 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 7 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 7 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.215999984,             !- Speed 7 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 7 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    519.62,                  !- Speed 7 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.57,                    !- Speed 7 Reference Unit Condenser Flow Rate {m3/s}",
        "    ,                        !- Speed 7 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 7 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 7 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 7 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF;              !- Speed 7 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        "	 Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "	Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	 Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "	Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
    });

    // Speed   Percent Load
    // 1 31.1951205 D
    // 2 38.4366621
    // 3 45.5738175 C_Low
    // 4 59.5881859 C_High  // Interpolation Required
    // 5 73.3098635 B_Low
    // 6 86.7736455 B_High  // Interpolation Required
    // 7 100        A (Exact Match)

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "ZONE1PTHPDXCOOLCOIL");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)
    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(7799.203151, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(9609.686765, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(0.575999992, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.639999991, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(3.6, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(3.6, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(162.10, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(199.72, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(0.942587793, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.009543347, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(0.342414409, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.034885008, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(0.8, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.2, thisCapFFlowHs->coeff[1]);

    //// EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.1552, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(-0.1808, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(25001.3561, thisCoil.RatedCapCoolTotal);
    EXPECT_EQ(1.6, thisCoil.RatedAirVolFlowRate);
    EXPECT_EQ(7, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_NEAR(3.44, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.60, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(3.20, StandardRatingsResult["EER2"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(24165.94683089908, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(3.10, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_NEAR(3.7796334706210803, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(23760.05, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
    EXPECT_NEAR(12.896644722586471, StandardRatingsResult["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_10_Speed_34582W_IEER_2022_ValueTest)
{
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:VariableSpeed,",
        "    Zone1PTHPDXCoolCoil,     !- Name",
        "    Zone1PTHPFanOutletNode,  !- Indoor Air Inlet Node Name",
        "    Zone1PTHPDXCoolCoilOutletNode,  !- Indoor Air Outlet Node Name",
        "    10.0,                    !- Number of Speeds {dimensionless}",
        "    10.0,                    !- Nominal Speed Level {dimensionless}",
        "    34582.38626,             !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}", //  7200.0 | 118,000 Btu/h
        "    1.6,                     !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",             // 0.4 | 3,400 scfm
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACPLFFPLR,             !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    7799.203151,             !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.575999992,             !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    162.10,                  !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.26,                    !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",       // CapFT
        "    HPACCoolCapFFF,          !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name", // CapFlow
        "    HPACEIRFT,               !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",           // EIRFTemp
        "    HPACEIRFFF,              !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",     // EIRFFLow
        "    9609.686765,             !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.639999991,             !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 2 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    199.72,                  !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.30,                    !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    11394.07239,             !- Speed 3 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 3 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 3 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.703999991,             !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 3 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    236.81,                  !- Speed 3 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.33,                    !- Speed 3 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    14897.85456,             !- Speed 4 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 4 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 4 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.831999989,             !- Speed 4 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 4 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    309.63,                  !- Speed 4 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.38,                    !- Speed 4 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 4 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 4 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    18328.46003,             !- Speed 5 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 5 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 5 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.959999987,             !- Speed 5 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 5 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    380.93,                  !- Speed 5 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.44,                    !- Speed 5 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 5 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 5 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 5 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 5 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 5 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    21694.58812,             !- Speed 6 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 6 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 6 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.087999986,             !- Speed 6 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 6 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    450.89,                  !- Speed 6 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.50,                    !- Speed 6 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 6 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 6 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 6 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 6 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 6 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    25001.3561,              !- Speed 7 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 7 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 7 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.215999984,             !- Speed 7 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 7 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    519.62,                  !- Speed 7 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.57,                    !- Speed 7 Reference Unit Condenser Flow Rate {m3/s}",
        "    ,                        !- Speed 7 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 7 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 7 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 7 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 7 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    28250.81086,             !- Speed 8 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 8 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 8 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.343999982,             !- Speed 8 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 8 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    587.16,                  !- Speed 8 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.63,                    !- Speed 8 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 8 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 8 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 8 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 8 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 8 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    31444.48758,             !- Speed 9 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 9 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 9 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.47199998,              !- Speed 9 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 9 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    653.53,                  !- Speed 9 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.69,                    !- Speed 9 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 9 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 9 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 9 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 9 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 9 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    34582.38626,             !- Speed 10 Reference Unit Gross Rated Total Cooling Capacity {W}", // 118,000 Btu/h
        "    0.75,                    !- Speed 10 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.6,                     !- Speed 10 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.599999979,             !- Speed 10 Reference Unit Rated Air Flow Rate {m3/s}", // 3,400 scfm
        "    773.3,                   !- Speed 10 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    718.75,                  !- Speed 10 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.74,                    !- Speed 10 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 10 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 10 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 10 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 10 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF;              !- Speed 10 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        "	 Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "	Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	 Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "	Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
    });

    // Speed   Percent Load
    //    1	22.55253033	  D_Low
    //    2	27.78780704	  D_High  // Interpolation Required
    //    3	32.94761764
    //    4	43.07931341	  C_Low
    //    5	52.99940811	  C_High  // Interpolation Required
    //    6	62.73305712
    //    7	72.29505771	  B_Low
    //    8	81.6913288	  B_High  // Interpolation Required
    //    9	90.92630956
    //    10	100	          A (Exact Match)

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);
    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "ZONE1PTHPDXCOOLCOIL");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)
    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(7799.203151, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(9609.686765, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(0.575999992, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.639999991, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(3.6, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(3.6, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(162.10, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(199.72, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(0.942587793, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.009543347, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(0.342414409, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.034885008, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(0.8, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.2, thisCapFFlowHs->coeff[1]);

    //// EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.1552, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(-0.1808, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(34582.38626, thisCoil.RatedCapCoolTotal);
    EXPECT_EQ(1.6, thisCoil.RatedAirVolFlowRate);
    EXPECT_EQ(10, thisCoil.NumOfSpeeds);

    EXPECT_NEAR(28250.81086, thisCoil.MSRatedTotCap(8), 0.01);
    EXPECT_NEAR(1.343999982, thisCoil.MSRatedAirVolFlowRate(8), 0.01);

    std::map<std::string, Real64> StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_NEAR(3.58, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.66, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(3.20, StandardRatingsResult["EER2"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(33426.83, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(3.10, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_NEAR(3.82, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(33339.549491351027, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
    EXPECT_NEAR(13.03, StandardRatingsResult["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_10_Speed_32000W_IEER_2022_ValueTest)
{
    // Cooling Cap & Air Flow Rate Scaled
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:VariableSpeed,",
        "    Zone1PTHPDXCoolCoil,     !- Name",
        "    Zone1PTHPFanOutletNode,  !- Indoor Air Inlet Node Name",
        "    Zone1PTHPDXCoolCoilOutletNode,  !- Indoor Air Outlet Node Name",
        "    10.0,                    !- Number of Speeds {dimensionless}",
        "    10.0,                    !- Nominal Speed Level {dimensionless}",
        "    32000,                   !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}", //  32000.0 W | 109188.53226 Btu/h
        "    1.6,                     !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",             // 0.4 | 3,400 scfm
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACPLFFPLR,             !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    3200,                     !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.575999992,               !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.26,                    !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",       // CapFT
        "    HPACCoolCapFFF,          !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name", // CapFlow
        "    HPACEIRFT,               !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",           // EIRFTemp
        "    HPACEIRFFF,              !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",     // EIRFFLow
        "    6400,                     !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.639999991,                !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 2 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.30,                    !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    9600,                      !- Speed 3 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 3 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 3 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.703999991,               !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 3 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 3 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.33,                    !- Speed 3 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    12800,                    !- Speed 4 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 4 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 4 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.831999989,               !- Speed 4 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 4 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 4 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.38,                    !- Speed 4 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 4 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 4 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    16000,                    !- Speed 5 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 5 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 5 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.959999987,                !- Speed 5 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 5 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 5 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.44,                    !- Speed 5 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 5 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 5 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 5 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 5 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 5 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    19200,                    !- Speed 6 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 6 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 6 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.087999986,               !- Speed 6 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 6 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 6 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.50,                    !- Speed 6 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 6 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 6 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 6 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 6 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 6 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    22400,                    !- Speed 7 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 7 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 7 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.215999984,               !- Speed 7 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 7 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 72023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.57,                    !- Speed 7 Reference Unit Condenser Flow Rate {m3/s}",
        "    ,                        !- Speed 7 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 7 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 7 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 7 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 7 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    25600,                    !- Speed 8 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 8 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 8 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.343999982,               !- Speed 8 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 8 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 8 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.63,                    !- Speed 8 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 8 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 8 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 8 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 8 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 8 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    28800,                    !- Speed 9 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 9 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 9 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.47199998,               !- Speed 9 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 9 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 9 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.69,                    !- Speed 9 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 9 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 9 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 9 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 9 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Speed 9 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    32000,                    !- Speed 10 Reference Unit Gross Rated Total Cooling Capacity {W}", // 118,000 Btu/h
        "    0.75,                    !- Speed 10 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.0,                     !- Speed 10 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.599999979,                 !- Speed 10 Reference Unit Rated Air Flow Rate {m3/s}", // 3,400 scfm
        "    773.3,                   !- Speed 10 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 10 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    0.74,                    !- Speed 10 Reference Unit Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 10 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPACCoolCapFT,           !- Speed 10 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Speed 10 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Speed 10 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF;              !- Speed 10 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        "	 Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "	Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	 Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "	Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",
    });

    //    Speed   % Load
    //        1	10
    //        2	20	      D_Low
    //        3	30	      D_High // Interpolation Required
    //        4	40
    //        5	50	      C (Exact Match)
    //        6	60
    //        7	70	      B_Low
    //        8	80	      B_High // Interpolation Required
    //        9	90
    //        10	100	      A (Exact Match)

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);
    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "ZONE1PTHPDXCOOLCOIL");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType;
    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(3200, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(6400, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(0.575999992, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.639999991, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(4.0, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(4.0, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.4, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(934.4, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(0.942587793, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.009543347, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(0.342414409, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.034885008, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(0.8, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.2, thisCapFFlowHs->coeff[1]);

    //// EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.1552, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(-0.1808, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(32000, thisCoil.RatedCapCoolTotal);
    EXPECT_EQ(1.6, thisCoil.RatedAirVolFlowRate);
    EXPECT_EQ(10, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_NEAR(3.52, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.29, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(3.34, StandardRatingsResult["EER2"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(30499.818, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(3.21, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_NEAR(3.59, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(30757.57, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
    EXPECT_NEAR(12.24, StandardRatingsResult["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
}

TEST_F(EnergyPlusFixture, VariableSpeedCooling_04_Speed_AutoSize_IEER_2022_ValueTest)
{
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:VariableSpeed,",
        "    Heat Pump ACDXCoil 1,    !- Name",
        "    DX Cooling Coil Air Inlet Node,  !- Indoor Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- Indoor Air Outlet Node Name",
        "    4.0,                     !- Number of Speeds {dimensionless}",
        "    3.0,                     !- Nominal Speed Level {dimensionless}",
        "    AUTOSIZE,                !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}",
        "    AUTOSIZE,                !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACCOOLPLFFPLR,         !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    8914.185229,             !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.79,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.980488789,             !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.944,                   !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    453.3,                   !- Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    453.3,                   !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    2.655,                   !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPCoolingCAPFTemp1,      !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPCoolingEIRFTemp1,      !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    16456.94607,             !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.73,                    !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.08,                    !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    1.888,                   !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    523.3,                   !- Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    631.7,                   !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    5.31,                    !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPCoolingCAPFTemp2,      !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPCoolingEIRFTemp2,      !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    23463.3646,              !- Speed 3 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.78,                    !- Speed 3 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    4.19,                    !- Speed 3 Reference Unit Gross Rated Cooling COP {W/W}",
        "    2.832,                   !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
        "    573.3,                   !- Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    692.1,                   !- Speed 3 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    7.965,                   !- Speed 3 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPCoolingCAPFTemp3,      !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPCoolingEIRFTemp3,      !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    36991.44197,             !- Speed 4 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Speed 4 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.866381837,             !- Speed 4 Reference Unit Gross Rated Cooling COP {W/W}",
        "    3.776,                   !- Speed 4 Reference Unit Rated Air Flow Rate {m3/s}",
        "    673.3,                   !- Speed 4 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    812.9,                   !- Speed 4 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    10.62,                   !- Speed 4 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 4 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPCoolingCAPFTemp4,      !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 4 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPCoolingEIRFTemp4,      !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACFFF;                 !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        " Curve:Quadratic,",
        "    HPACCOOLPLFFPLR,         !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "	  Curve:Biquadratic,",
        "    HPCoolingCAPFTemp1,      !- Name",
        "    1.6253449506,            !- Coefficient1 Constant",
        "    -0.0786550838,           !- Coefficient2 x",
        "    0.0030679776,            !- Coefficient3 x**2",
        "    0.0008002088,            !- Coefficient4 y",
        "    -0.0000354060,           !- Coefficient5 y**2",
        "    -0.0003534409,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.8026,                  !- Minimum Curve Output",
        "    1.2772,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Biquadratic,",
        "    HPCoolingEIRFTemp1,      !- Name",
        "    -0.2808139299,           !- Coefficient1 Constant",
        "    0.0987778868,            !- Coefficient2 x",
        "    -0.0030382726,           !- Coefficient3 x**2",
        "    0.0057742302,            !- Coefficient4 y",
        "    0.0004171046,            !- Coefficient5 y**2",
        "    -0.0003000325,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.4975,                  !- Minimum Curve Output",
        "    1.7115,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Cubic,",
        "    HPACFFF,                 !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "	  Curve:Biquadratic,",
        "    HPCoolingCAPFTemp2,      !- Name",
        "    1.4240389306,            !- Coefficient1 Constant",
        "    -0.0593310687,           !- Coefficient2 x",
        "    0.0026068070,            !- Coefficient3 x**2",
        "    0.0008867551,            !- Coefficient4 y",
        "    -0.0000369191,           !- Coefficient5 y**2",
        "    -0.0003552805,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.7891,                  !- Minimum Curve Output",
        "    1.2786,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Biquadratic,",
        "    HPCoolingEIRFTemp2,      !- Name",
        "    -0.1062902436,           !- Coefficient1 Constant",
        "    0.0758517935,            !- Coefficient2 x",
        "    -0.0023342253,           !- Coefficient3 x**2",
        "    0.0085794421,            !- Coefficient4 y",
        "    0.0003952199,            !- Coefficient5 y**2",
        "    -0.0003974572,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.5088,                  !- Minimum Curve Output",
        "    1.7121,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Biquadratic,",
        "    HPCoolingCAPFTemp3,      !- Name",
        "    1.5159775390,            !- Coefficient1 Constant",
        "    -0.0668925955,           !- Coefficient2 x",
        "    0.0027457672,            !- Coefficient3 x**2",
        "    0.0009848890,            !- Coefficient4 y",
        "    -0.0000405294,           !- Coefficient5 y**2",
        "    -0.0003500788,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.8017,                  !- Minimum Curve Output",
        "    1.2811,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	 Curve:Biquadratic,",
        "    HPCoolingEIRFTemp3,      !- Name",
        "    -0.1799894531,           !- Coefficient1 Constant",
        "    0.0841627301,            !- Coefficient2 x",
        "    -0.0025709433,           !- Coefficient3 x**2",
        "    0.0065952253,            !- Coefficient4 y",
        "    0.0004162808,            !- Coefficient5 y**2",
        "    -0.000330974,            !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.4936,                  !- Minimum Curve Output",
        "    1.7105,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Biquadratic,",
        "    HPCoolingCAPFTemp4,      !- Name",
        "    1.3544202152,            !- Coefficient1 Constant",
        "    -0.0493402773,           !- Coefficient2 x",
        "    0.0022649843,            !- Coefficient3 x**2",
        "    0.0008517727,            !- Coefficient4 y",
        "    -0.0000426316,           !- Coefficient5 y**2",
        "    -0.0003364517,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.7923,                  !- Minimum Curve Output",
        "    1.2736,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Biquadratic,",
        "    HPCoolingEIRFTemp4,      !- Name",
        "    0.0001514017,            !- Coefficient1 Constant",
        "    0.0655062896,            !- Coefficient2 x",
        "    -0.0020370821,           !- Coefficient3 x**2",
        "    0.0067823041,            !- Coefficient4 y",
        "    0.0004087196,            !- Coefficient5 y**2",
        "    -0.0003552302,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.5141,                  !- Minimum Curve Output",
        "    1.7044,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);
    // get coil inputs
    EnergyPlus::VariableSpeedCoils::GetVarSpeedCoilInput(*state);
    EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "HEAT PUMP ACDXCOIL 1");
    auto &thisCoil(state->dataVariableSpeedCoils->VarSpeedCoil(1));
    auto condenserType = thisCoil.CondenserType; // Air(0)
    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == condenserType);
    auto varSpeedCoilType = thisCoil.VarSpeedCoilType;
    EXPECT_EQ(varSpeedCoilType, "Coil:Cooling:DX:VariableSpeed");
    auto vsCoilType = thisCoil.VSCoilType; // 30
    EXPECT_TRUE(30 == vsCoilType);
    auto pLFfPLR_Curve = thisCoil.PLFFPLR;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(8914.185229, thisCoil.MSRatedTotCap(1), 0.01);
    EXPECT_NEAR(16456.94607, thisCoil.MSRatedTotCap(2), 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(0.944, thisCoil.MSRatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(1.888, thisCoil.MSRatedAirVolFlowRate(2), 0.01);

    // Rated COP
    EXPECT_NEAR(3.980488789, thisCoil.MSRatedCOP(1), 0.01);
    EXPECT_NEAR(4.08, thisCoil.MSRatedCOP(2), 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(453.3, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(1), 0.01);
    EXPECT_NEAR(631.7, thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023(2), 0.01);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType);
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.MSCCapFTemp(1)));
    EXPECT_EQ(1.6253449506, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(-0.0786550838, thisCCpaFTempHS->coeff[1]);

    //// EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.MSEIRFTemp(1)));
    EXPECT_EQ(-0.2808139299, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(0.0987778868, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.MSCCapAirFFlow(1)));
    EXPECT_EQ(1.0, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0.0, thisCapFFlowHs->coeff[1]);

    // EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.MSEIRAirFFlow(1)));
    EXPECT_EQ(1.0, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(0.0, thisEIRFFlowHs->coeff[1]);

    EXPECT_EQ(4, thisCoil.NumOfSpeeds);

    std::map<std::string, Real64> StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(
        *state,
        thisCoil.VarSpeedCoilType, // thisCoil.DXCoilType,
        thisCoil.Name,
        thisCoil.MSCCapFTemp,
        thisCoil.MSCCapAirFFlow, // thisCoil.MSCCapFFlow,
        thisCoil.MSEIRFTemp,
        thisCoil.MSEIRAirFFlow, // thisCoil.MSEIRFFlow,
        thisCoil.PLFFPLR,       // thisCoil.MSPLFFPLR,
        thisCoil.MSRatedTotCap,
        thisCoil.MSRatedCOP,
        thisCoil.MSRatedAirVolFlowRate,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2017,
        thisCoil.MSRatedEvaporatorFanPowerPerVolumeFlowRate2023, // thisCoil.MSFanPowerPerEvapAirFlowRate_2023,
        thisCoil.NumOfSpeeds,
        thisCoil.CondenserType,
        thisCoil.RatedCapCoolTotal,
        thisCoil.RatedAirVolFlowRate);

    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_NEAR(2.86, StandardRatingsResult["EER2"], 0.01);
    EXPECT_NEAR(3.45, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.55, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_NEAR(33917.499738697108, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(2.68, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_NEAR(3.7865746033246772, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(34444.62, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
    EXPECT_NEAR(12.91836822, StandardRatingsResult["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
}

TEST_F(EnergyPlusFixture, TwoSpeedCoolingCoilAir_18000W_SEER2_2023_ValueTest)
{
    // Modified from Coil:Cooling:DX:TwoSpeed coil in "DesiccantDehumidifierWithAirToAirCoil.idf"
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:TwoSpeed,",
        "    Main Cooling Coil 1,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    18000,                   !- High Speed Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- High Speed Rated Sensible Heat Ratio",
        "    3.0,                     !- High Speed Gross Rated Cooling COP {W/W}",
        "    1.0,                     !- High Speed Rated Air Flow Rate {m3/s}",
        "    ,                        !- High Speed 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- High Speed 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Unit Internal Static Air Pressure {Pa}",
        "    Mixed Air Node 1,        !- Air Inlet Node Name",
        "    Main Cooling Coil 1 Outlet Node,  !- Air Outlet Node Name",
        "    VarSpeedCoolCapFT,       !- Total Cooling Capacity Function of Temperature Curve Name",
        "    PackagedRatedCoolCapFFlow,  !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    VarSpeedCoolEIRFT,       !- Energy Input Ratio Function of Temperature Curve Name",
        "    PackagedRatedCoolEIRFFlow,  !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    VarSpeedCyclingPLFFPLR,  !- Part Load Fraction Correlation Curve Name",
        "    6000,                    !- Low Speed Gross Rated Total Cooling Capacity {W}",
        "    0.69,                    !- Low Speed Gross Rated Sensible Heat Ratio",
        "    4.2,                     !- Low Speed Gross Rated Cooling COP {W/W}",
        "    0.4,                     !- Low Speed Rated Air Flow Rate {m3/s}",
        "    ,                        !- Low Speed 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Low Speed 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    VarSpeedCoolCapLSFT,     !- Low Speed Total Cooling Capacity Function of Temperature Curve Name",
        "    VarSpeedCoolEIRLSFT,     !- Low Speed Energy Input Ratio Function of Temperature Curve Name",
        "    Main Cooling Coil 1 Condenser Node;  !- Condenser Air Inlet Node Name",
        "",
        "  Curve:Biquadratic,",
        "    VarSpeedCoolCapFT,       !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Cubic,",
        "    PackagedRatedCoolCapFFlow,  !- Name",
        "    0.47278589,              !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    0.32257813,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Biquadratic,",
        "    VarSpeedCoolEIRFT,       !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Cubic,",
        "    PackagedRatedCoolEIRFFlow,  !- Name",
        "    1.0079484,               !- Coefficient1 Constant",
        "    0.34544129,              !- Coefficient2 x",
        "    -.6922891,               !- Coefficient3 x**2",
        "    0.33889943,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    VarSpeedCyclingPLFFPLR,  !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    VarSpeedCoolCapLSFT,     !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Biquadratic,",
        "    VarSpeedCoolEIRLSFT,     !- Name",
        "    0.774645E+00,            !- Coefficient1 Constant",
        "    -0.343731E-01,           !- Coefficient2 x",
        "    0.783173E-03,            !- Coefficient3 x**2",
        "    0.146596E-01,            !- Coefficient4 y",
        "    0.488851E-03,            !- Coefficient5 y**2",
        "    -0.752036E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 3/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0,        !- Field 3",
        "    Through: 9/30,           !- Field 5",
        "    For: WeekDays,           !- Field 6",
        "    Until: 7:00,0.0,         !- Field 7",
        "    Until: 17:00,1.0,        !- Field 9",
        "    Until: 24:00,0.0,        !- Field 11",
        "    For: SummerDesignDay WinterDesignDay, !- Field 13",
        "    Until: 24:00,1.0,        !- Field 14",
        "    For: AllOtherDays,       !- Field 16",
        "    Until: 24:00,0.0,        !- Field 17",
        "    Through: 12/31,          !- Field 19",
        "    For: AllDays,            !- Field 20",
        "    Until: 24:00,1.0;        !- Field 21",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);
    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.PLFFPLR(1)));

    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.PLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(18000, thisCoil.RatedTotCap(1), 0.01);
    EXPECT_NEAR(6000, thisCoil.RatedTotCap2, 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(1.0, thisCoil.RatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.40, thisCoil.RatedAirVolFlowRate2, 0.01);

    // Rated COP
    EXPECT_NEAR(3.0, thisCoil.RatedCOP(1), 0.01);
    EXPECT_NEAR(4.2, thisCoil.RatedCOP2, 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.39, thisCoil.FanPowerPerEvapAirFlowRate_2023(1), 0.01);
    EXPECT_NEAR(934.39, thisCoil.FanPowerPerEvapAirFlowRate_2023_LowSpeed(1), 0.01);

    EXPECT_EQ("Coil:Cooling:DX:TwoSpeed", thisCoil.DXCoilType);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType(1));
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType(1));

    // Ckeck user curve coefficients

    // CCapFTemp High Speed
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.CCapFTemp(1)));
    EXPECT_EQ(0.476428E+00, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.401147E-01, thisCCpaFTempHS->coeff[1]);
    // CCapFTemp Low Speed
    auto &thisCCpaFTempLS(state->dataCurveManager->curves(thisCoil.CCapFTemp2));
    EXPECT_EQ(0.476428E+00, thisCCpaFTempLS->coeff[0]);
    EXPECT_EQ(0.226411E-03, thisCCpaFTempLS->coeff[2]);

    // EIRFTemp High Speed Curve
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.EIRFTemp(1)));
    EXPECT_EQ(0.632475E+00, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(-0.121321E-01, thisEIRFTempHS->coeff[1]);
    // EIRFTemp Low Speed Curve
    auto &thisEIRFTempLS(state->dataCurveManager->curves(thisCoil.EIRFTemp2));
    EXPECT_EQ(0.774645E+00, thisEIRFTempLS->coeff[0]);
    EXPECT_EQ(-0.343731E-01, thisEIRFTempLS->coeff[1]);

    // CapFFlow High Speed
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.CCapFFlow(1)));
    EXPECT_EQ(0.47278589, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(1.2433415, thisCapFFlowHs->coeff[1]);
    // Note -- No CapFlow for Low Speed

    // EIRFFlow High Speed
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.EIRFFlow(1)));
    EXPECT_EQ(1.0079484, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(0.34544129, thisEIRFFlowHs->coeff[1]);
    // Note -- No EIRFlow for Low Speed

    std::map<std::string, Real64> StandardRatingsResult;
    Real64 NetCoolingCapRatedMaxSpeed(0.0);
    Real64 SEER2_User(0.0);
    Real64 SEER2_Standard(0.0);
    Real64 EER2(0.0); // Energy Efficiency Ratio using AHRI 210/240 - 2023
    Real64 NetCoolingCapRated2023(0.0);

    Array1D_int TSCCapFTemp;
    TSCCapFTemp.push_back(thisCoil.CCapFTemp(1)); // High Speed
    TSCCapFTemp.push_back(thisCoil.CCapFTemp2);   // Low Speed

    Array1D<Real64> TSFanPowerPerEvapAirFlowRate2023;
    TSFanPowerPerEvapAirFlowRate2023.push_back(thisCoil.FanPowerPerEvapAirFlowRate_2023(1));          // High Speed
    TSFanPowerPerEvapAirFlowRate2023.push_back(thisCoil.FanPowerPerEvapAirFlowRate_2023_LowSpeed(1)); // Low Speed

    Array1D<Real64> TSRatedTotCap;
    TSRatedTotCap.push_back(thisCoil.RatedTotCap(1)); // High Speed
    TSRatedTotCap.push_back(thisCoil.RatedTotCap2);   // Low Speed

    Array1D<Real64> TSRatedAirVolFlowRate;
    TSRatedAirVolFlowRate.push_back(thisCoil.RatedAirVolFlowRate(1)); // High Speed
    TSRatedAirVolFlowRate.push_back(thisCoil.RatedAirVolFlowRate2);   // Low Speed

    Array1D_int TSEIRFTemp;
    TSEIRFTemp.push_back(thisCoil.EIRFTemp(1)); // High Speed
    TSEIRFTemp.push_back(thisCoil.EIRFTemp2);   // Low Speed

    Array1D<Real64> TSRatedCOP;
    TSRatedCOP.push_back(thisCoil.RatedCOP(1)); // High Speed
    TSRatedCOP.push_back(thisCoil.RatedCOP2);   // Low Speed

    std::tie(NetCoolingCapRated2023, SEER2_User, SEER2_Standard, EER2) =
        TwoSpeedDXCoolingCoilSEER2(*state,
                                   // 2, // nsp will always be 2 in case of Two Speed Coil
                                   thisCoil.CCapFFlow, // CapFFlowCurveIndex, | Only High Speed
                                   TSRatedTotCap,
                                   TSCCapFTemp, // CapFTempCurveIndex,
                                   TSFanPowerPerEvapAirFlowRate2023,
                                   TSRatedAirVolFlowRate,
                                   thisCoil.EIRFFlow, // EIRFFlowCurveIndex, | Only High Speed
                                   TSRatedCOP,
                                   TSEIRFTemp,      // EIRFTempCurveIndex,
                                   thisCoil.PLFFPLR // PLFFPLRCurveIndex | Coil Level Decleration
        );
    NetCoolingCapRatedMaxSpeed = NetCoolingCapRated2023;
    EXPECT_TRUE(SEER2_User > 0.0);
    EXPECT_TRUE(SEER2_Standard > 0.0);
    EXPECT_TRUE(NetCoolingCapRatedMaxSpeed > 0.0);
    EXPECT_TRUE(EER2 > 0.0);
    EXPECT_NEAR(17070.177992104353, NetCoolingCapRatedMaxSpeed, 0.01);
    EXPECT_NEAR(3.39616, SEER2_User, 0.01);
    EXPECT_NEAR(3.38201, SEER2_Standard, 0.01);
    EXPECT_NEAR(11.5882, SEER2_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(11.5399, SEER2_Standard * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(2.56, EER2, 0.01);
}

TEST_F(EnergyPlusFixture, TwoSpeedCoolingCoilAir_12000W_SEER2_2023_ValueTest)
{
    // Modified from Coil:Cooling:DX:TwoSpeed coil in "DesiccantDehumidifierWithAirToAirCoil.idf"
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:TwoSpeed,",
        "    Main Cooling Coil 1,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    12000,                   !- High Speed Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- High Speed Rated Sensible Heat Ratio",
        "    3.0,                     !- High Speed Gross Rated Cooling COP {W/W}",
        "    0.8,                     !- High Speed Rated Air Flow Rate {m3/s}",
        "    ,                        !- High Speed 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- High Speed 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Unit Internal Static Air Pressure {Pa}",
        "    Mixed Air Node 1,        !- Air Inlet Node Name",
        "    Main Cooling Coil 1 Outlet Node,  !- Air Outlet Node Name",
        "    VarSpeedCoolCapFT,       !- Total Cooling Capacity Function of Temperature Curve Name",
        "    PackagedRatedCoolCapFFlow,  !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    VarSpeedCoolEIRFT,       !- Energy Input Ratio Function of Temperature Curve Name",
        "    PackagedRatedCoolEIRFFlow,  !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    VarSpeedCyclingPLFFPLR,  !- Part Load Fraction Correlation Curve Name",
        "    4000,                    !- Low Speed Gross Rated Total Cooling Capacity {W}",
        "    0.69,                    !- Low Speed Gross Rated Sensible Heat Ratio",
        "    4.2,                     !- Low Speed Gross Rated Cooling COP {W/W}",
        "    0.4,                     !- Low Speed Rated Air Flow Rate {m3/s}",
        "    ,                        !- Low Speed 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Low Speed 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    VarSpeedCoolCapLSFT,     !- Low Speed Total Cooling Capacity Function of Temperature Curve Name",
        "    VarSpeedCoolEIRLSFT,     !- Low Speed Energy Input Ratio Function of Temperature Curve Name",
        "    Main Cooling Coil 1 Condenser Node;  !- Condenser Air Inlet Node Name",
        "",
        "  Curve:Biquadratic,",
        "    VarSpeedCoolCapFT,       !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Cubic,",
        "    PackagedRatedCoolCapFFlow,  !- Name",
        "    0.47278589,              !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    0.32257813,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Biquadratic,",
        "    VarSpeedCoolEIRFT,       !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Cubic,",
        "    PackagedRatedCoolEIRFFlow,  !- Name",
        "    1.0079484,               !- Coefficient1 Constant",
        "    0.34544129,              !- Coefficient2 x",
        "    -.6922891,               !- Coefficient3 x**2",
        "    0.33889943,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Quadratic,",
        "    VarSpeedCyclingPLFFPLR,  !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "",
        "  Curve:Biquadratic,",
        "    VarSpeedCoolCapLSFT,     !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "  Curve:Biquadratic,",
        "    VarSpeedCoolEIRLSFT,     !- Name",
        "    0.774645E+00,            !- Coefficient1 Constant",
        "    -0.343731E-01,           !- Coefficient2 x",
        "    0.783173E-03,            !- Coefficient3 x**2",
        "    0.146596E-01,            !- Coefficient4 y",
        "    0.488851E-03,            !- Coefficient5 y**2",
        "    -0.752036E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "",
        "Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 3/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0,        !- Field 3",
        "    Through: 9/30,           !- Field 5",
        "    For: WeekDays,           !- Field 6",
        "    Until: 7:00,0.0,         !- Field 7",
        "    Until: 17:00,1.0,        !- Field 9",
        "    Until: 24:00,0.0,        !- Field 11",
        "    For: SummerDesignDay WinterDesignDay, !- Field 13",
        "    Until: 24:00,1.0,        !- Field 14",
        "    For: AllOtherDays,       !- Field 16",
        "    Until: 24:00,0.0,        !- Field 17",
        "    Through: 12/31,          !- Field 19",
        "    For: AllDays,            !- Field 20",
        "    Until: 24:00,1.0;        !- Field 21",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.PLFFPLR(1)));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.PLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(12000, thisCoil.RatedTotCap(1), 0.01);
    EXPECT_NEAR(4000, thisCoil.RatedTotCap2, 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(0.80000000000000004, thisCoil.RatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.40000000000000002, thisCoil.RatedAirVolFlowRate2, 0.01);

    // Rated COP
    EXPECT_NEAR(3.0, thisCoil.RatedCOP(1), 0.01);
    EXPECT_NEAR(4.20, thisCoil.RatedCOP2, 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.39, thisCoil.FanPowerPerEvapAirFlowRate_2023(1), 0.01);
    EXPECT_NEAR(934.39, thisCoil.FanPowerPerEvapAirFlowRate_2023_LowSpeed(1), 0.01);

    EXPECT_EQ("Coil:Cooling:DX:TwoSpeed", thisCoil.DXCoilType);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType(1));
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType(1));

    // Ckeck user curve coefficients

    // CCapFTemp High Speed
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.CCapFTemp(1)));
    EXPECT_EQ(0.476428E+00, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.401147E-01, thisCCpaFTempHS->coeff[1]);
    // CCapFTemp Low Speed
    auto &thisCCpaFTempLS(state->dataCurveManager->curves(thisCoil.CCapFTemp2));
    EXPECT_EQ(0.476428E+00, thisCCpaFTempLS->coeff[0]);
    EXPECT_EQ(0.226411E-03, thisCCpaFTempLS->coeff[2]);

    // EIRFTemp High Speed Curve
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.EIRFTemp(1)));
    EXPECT_EQ(0.632475E+00, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(-0.121321E-01, thisEIRFTempHS->coeff[1]);
    // EIRFTemp Low Speed Curve
    auto &thisEIRFTempLS(state->dataCurveManager->curves(thisCoil.EIRFTemp2));
    EXPECT_EQ(0.774645E+00, thisEIRFTempLS->coeff[0]);
    EXPECT_EQ(-0.343731E-01, thisEIRFTempLS->coeff[1]);

    // CapFFlow High Speed
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.CCapFFlow(1)));
    EXPECT_EQ(0.47278589, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(1.2433415, thisCapFFlowHs->coeff[1]);
    // Note -- No CapFlow for Low Speed

    // EIRFFlow High Speed
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.EIRFFlow(1)));
    EXPECT_EQ(1.0079484, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(0.34544129, thisEIRFFlowHs->coeff[1]);
    // Note -- No EIRFlow for Low Speed

    std::map<std::string, Real64> StandardRatingsResult = TwoSpeedDXCoilStandardRatings(*state,
                                                                                        thisCoil.Name,
                                                                                        thisCoil.DXCoilType,
                                                                                        thisCoil.DXCoilType_Num,
                                                                                        thisCoil.RatedTotCap,
                                                                                        thisCoil.RatedTotCap2,
                                                                                        thisCoil.RatedCOP,
                                                                                        thisCoil.RatedCOP2,
                                                                                        thisCoil.CCapFFlow, // High Speed
                                                                                        thisCoil.CCapFTemp,
                                                                                        thisCoil.CCapFTemp2,
                                                                                        thisCoil.EIRFFlow, // High Speed
                                                                                        thisCoil.EIRFTemp,
                                                                                        thisCoil.EIRFTemp2,
                                                                                        thisCoil.RatedAirVolFlowRate,
                                                                                        thisCoil.RatedAirVolFlowRate2,
                                                                                        thisCoil.FanPowerPerEvapAirFlowRate_2023,
                                                                                        thisCoil.FanPowerPerEvapAirFlowRate_2023_LowSpeed,
                                                                                        thisCoil.CondenserType,
                                                                                        thisCoil.PLFFPLR(1));
    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_NEAR(11255.531994736235, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);
    EXPECT_NEAR(3.1266048533133741, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(3.11318185145895, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(10.668418589930422, StandardRatingsResult["SEER2_User"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(10.622617406463105, StandardRatingsResult["SEER2_Standard"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(2.49, StandardRatingsResult["EER2"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(2.98, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(2.36, StandardRatingsResult["EER_2022"], 0.01);
    EXPECT_NEAR(11255.53, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
}

TEST_F(EnergyPlusFixture, TwoSpeedCoolingCoilAir_39000W_IEER_2022_ValueTest)
{
    // DX_TwoSpeed_IEER_39000W_ModFrom_AirflowNetwork_MultiZone_SmallOffice_VAV.idf
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:TwoSpeed,",
        "    ACDXCoil 1,              !- Name",
        "    CoolingCoilAvailSched,   !- Availability Schedule Name",
        "    39000,                   !- High Speed Gross Rated Total Cooling Capacity {W}",
        "    0.8,                     !- High Speed Rated Sensible Heat Ratio",
        "    3.4,                     !- High Speed Gross Rated Cooling COP {W/W}",
        "    1.0,                     !- High Speed Rated Air Flow Rate {m3/s}",
        "    ,                        !- High Speed 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- High Speed 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Unit Internal Static Air Pressure {Pa}",
        "    Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "    VarSpeedCoolCapFT,       !- Total Cooling Capacity Function of Temperature Curve Name",
        "    PackagedRatedCoolCapFFlow,  !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    VarSpeedCoolEIRFT,       !- Energy Input Ratio Function of Temperature Curve Name",
        "    PackagedRatedCoolEIRFFlow,  !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    VarSpeedCyclingPLFFPLR,  !- Part Load Fraction Correlation Curve Name",
        "    12000,                   !- Low Speed Gross Rated Total Cooling Capacity {W}",
        "    0.8,                     !- Low Speed Gross Rated Sensible Heat Ratio",
        "    4.2,                     !- Low Speed Gross Rated Cooling COP {W/W}",
        "    0.3,                     !- Low Speed Rated Air Flow Rate {m3/s}",
        "    ,                        !- Low Speed 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Low Speed 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    VarSpeedCoolCapLSFT,     !- Low Speed Total Cooling Capacity Function of Temperature Curve Name",
        "    VarSpeedCoolEIRLSFT,     !- Low Speed Energy Input Ratio Function of Temperature Curve Name",
        "    Outside Air Inlet Node;  !- Condenser Air Inlet Node Name",

        "! same as Doe-2 SDL-C4",

        "  Curve:Biquadratic,",
        "    VarSpeedCoolCapFT,       !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "! same as Doe-2 SDL-C82",

        "  Curve:Biquadratic,",
        "    VarSpeedCoolEIRFT,       !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "! same as Doe-2 SDL-C4",

        "  Curve:Biquadratic,",
        "    VarSpeedCoolCapLSFT,     !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "! same as Doe-2 SDL-C132",

        "  Curve:Biquadratic,",
        "    VarSpeedCoolEIRLSFT,     !- Name",
        "    0.774645E+00,            !- Coefficient1 Constant",
        "    -0.343731E-01,           !- Coefficient2 x",
        "    0.783173E-03,            !- Coefficient3 x**2",
        "    0.146596E-01,            !- Coefficient4 y",
        "    0.488851E-03,            !- Coefficient5 y**2",
        "    -0.752036E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "! same as Doe-2 SDL-C78",

        "  Curve:Cubic,",
        "    PackagedRatedCoolCapFFlow,  !- Name",
        "    0.47278589,              !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    0.32257813,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "! same as Doe-2 SDL-C93",

        "  Curve:Cubic,",
        "    PackagedRatedCoolEIRFFlow,  !- Name",
        "    1.0079484,               !- Coefficient1 Constant",
        "    0.34544129,              !- Coefficient2 x",
        "    -.6922891,               !- Coefficient3 x**2",
        "    0.33889943,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Quadratic,",
        "    VarSpeedCyclingPLFFPLR,  !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "Schedule:Compact,",
        "    CoolingCoilAvailSched,   !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 3/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,0.0,        !- Field 3",
        "    Through: 9/30,           !- Field 5",
        "    For: WeekDays SummerDesignDay, !- Field 6",
        "    Until: 7:00,0.0,         !- Field 7",
        "    Until: 17:00,1.0,        !- Field 9",
        "    Until: 24:00,0.0,        !- Field 11",
        "    For: WinterDesignDay,    !- Field 13",
        "    Until: 24:00,0.0,        !- Field 14",
        "    For: AllOtherDays,       !- Field 16",
        "    Until: 24:00,0.0,        !- Field 17",
        "    Through: 12/31,          !- Field 19",
        "    For: AllDays,            !- Field 20",
        "    Until: 24:00,0.0;        !- Field 21",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);
    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.PLFFPLR(1)));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.PLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(39000, thisCoil.RatedTotCap(1), 0.01);
    EXPECT_NEAR(12000, thisCoil.RatedTotCap2, 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(1.0, thisCoil.RatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.3, thisCoil.RatedAirVolFlowRate2, 0.01);

    // Rated COP
    EXPECT_NEAR(3.39, thisCoil.RatedCOP(1), 0.01);
    EXPECT_NEAR(4.2, thisCoil.RatedCOP2, 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.39, thisCoil.FanPowerPerEvapAirFlowRate_2023(1), 0.01);
    EXPECT_NEAR(934.39, thisCoil.FanPowerPerEvapAirFlowRate_2023_LowSpeed(1), 0.01);

    EXPECT_EQ("Coil:Cooling:DX:TwoSpeed", thisCoil.DXCoilType);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType(1));
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType(1));

    // Ckeck user curve coefficients

    // CCapFTemp High Speed
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.CCapFTemp(1)));
    EXPECT_EQ(0.476428E+00, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.401147E-01, thisCCpaFTempHS->coeff[1]);
    // CCapFTemp Low Speed
    auto &thisCCpaFTempLS(state->dataCurveManager->curves(thisCoil.CCapFTemp2));
    EXPECT_EQ(0.476428E+00, thisCCpaFTempLS->coeff[0]);
    EXPECT_EQ(0.226411E-03, thisCCpaFTempLS->coeff[2]);

    // EIRFTemp High Speed Curve
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.EIRFTemp(1)));
    EXPECT_EQ(0.632475E+00, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(-0.121321E-01, thisEIRFTempHS->coeff[1]);
    // EIRFTemp Low Speed Curve
    auto &thisEIRFTempLS(state->dataCurveManager->curves(thisCoil.EIRFTemp2));
    EXPECT_EQ(0.774645E+00, thisEIRFTempLS->coeff[0]);
    EXPECT_EQ(-0.343731E-01, thisEIRFTempLS->coeff[1]);

    // CapFFlow High Speed
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.CCapFFlow(1)));
    EXPECT_EQ(0.47278589, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(1.2433415, thisCapFFlowHs->coeff[1]);
    // Note -- No CapFlow for Low Speed

    // EIRFFlow High Speed
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.EIRFFlow(1)));
    EXPECT_EQ(1.0079484, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(0.34544129, thisEIRFFlowHs->coeff[1]);
    // Note -- No EIRFlow for Low Speed

    std::map<std::string, Real64> StandardRatingsResult;

    StandardRatingsResult = TwoSpeedDXCoilStandardRatings(*state,
                                                          thisCoil.Name,
                                                          thisCoil.DXCoilType,
                                                          thisCoil.DXCoilType_Num,
                                                          thisCoil.RatedTotCap,
                                                          thisCoil.RatedTotCap2,
                                                          thisCoil.RatedCOP,
                                                          thisCoil.RatedCOP2,
                                                          thisCoil.CCapFFlow, // High Speed
                                                          thisCoil.CCapFTemp,
                                                          thisCoil.CCapFTemp2,
                                                          thisCoil.EIRFFlow, // High Speed
                                                          thisCoil.EIRFTemp,
                                                          thisCoil.EIRFTemp2,
                                                          thisCoil.RatedAirVolFlowRate,
                                                          thisCoil.RatedAirVolFlowRate2,
                                                          thisCoil.FanPowerPerEvapAirFlowRate_2023,
                                                          thisCoil.FanPowerPerEvapAirFlowRate_2023_LowSpeed,
                                                          thisCoil.CondenserType,
                                                          thisCoil.PLFFPLR(1));
    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_NEAR(4.05, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(4.04, StandardRatingsResult["SEER2_Standard"], 0.01);
    EXPECT_NEAR(3.12, StandardRatingsResult["EER2"], 0.01);
    EXPECT_NEAR(38075.51, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(4.12, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(38075.51, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
    EXPECT_NEAR(14.07, StandardRatingsResult["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(3.05, StandardRatingsResult["EER_2022"], 0.01);
}
TEST_F(EnergyPlusFixture, TwoSpeedCoolingCoilAir_54000W_IEER_2022_ValueTest)
{
    // Single Speed Two Cycle
    std::string const idf_objects = delimited_string({
        "  Coil:Cooling:DX:TwoSpeed,",
        "    Main Cooling Coil 1,     !- Name",
        "    CoolingCoilAvailSched,   !- Availability Schedule Name",
        "    54883.780765317439,                !- High Speed Gross Rated Total Cooling Capacity {W}",
        "    0.8,                     !- High Speed Rated Sensible Heat Ratio",
        "    3.0,                     !- High Speed Gross Rated Cooling COP {W/W}",
        "    3.3155291960328266,                !- High Speed Rated Air Flow Rate {m3/s}",
        "    ,                        !- High Speed 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- High Speed 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Unit Internal Static Air Pressure {Pa}",
        "    Mixed Air Node 1,        !- Air Inlet Node Name",
        "    Main Cooling Coil 1 Outlet Node,  !- Air Outlet Node Name",
        "    VarSpeedCoolCapFT,       !- Total Cooling Capacity Function of Temperature Curve Name",
        "    PackagedRatedCoolCapFFlow,  !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    VarSpeedCoolEIRFT,       !- Energy Input Ratio Function of Temperature Curve Name",
        "    PackagedRatedCoolEIRFFlow,  !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    VarSpeedCyclingPLFFPLR,  !- Part Load Fraction Correlation Curve Name",
        "    18292.764129080300,                !- Low Speed Gross Rated Total Cooling Capacity {W}",
        "    0.8,                     !- Low Speed Gross Rated Sensible Heat Ratio",
        "    4.2,                     !- Low Speed Gross Rated Cooling COP {W/W}",
        "    1.1050658810377409,                !- Low Speed Rated Air Flow Rate {m3/s}",
        "    ,                        !- Low Speed 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Low Speed 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    VarSpeedCoolCapLSFT,     !- Low Speed Total Cooling Capacity Function of Temperature Curve Name",
        "    VarSpeedCoolEIRLSFT,     !- Low Speed Energy Input Ratio Function of Temperature Curve Name",
        "    Main Cooling Coil 1 Condenser Node;  !- Condenser Air Inlet Node Name",

        "  OutdoorAir:Node,",
        "    Main Cooling Coil 1 Condenser Node,  !- Name",
        "    -1.0;                    !- Height Above Ground {m}",

        "  Schedule:Compact,",
        "    CoolingCoilAvailSched,   !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 3/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,0.0,        !- Field 3",
        "    Through: 9/30,           !- Field 5",
        "    For: WeekDays,           !- Field 6",
        "    Until: 7:00,0.0,         !- Field 7",
        "    Until: 17:00,1.0,        !- Field 9",
        "    Until: 24:00,0.0,        !- Field 11",
        "    For: SummerDesignDay WinterDesignDay, !- Field 13",
        "    Until: 24:00,1.0,        !- Field 14",
        "    For: AllOtherDays,       !- Field 16",
        "    Until: 24:00,0.0,        !- Field 17",
        "    Through: 12/31,          !- Field 19",
        "    For: AllDays,            !- Field 20",
        "	Until: 24:00,0.0;        !- Field 21",

        "  Curve:Biquadratic,",
        "    VarSpeedCoolCapFT,       !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Cubic,",
        "    PackagedRatedCoolCapFFlow,  !- Name",
        "    0.47278589,              !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    0.32257813,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Biquadratic,",
        "    VarSpeedCoolEIRFT,       !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Cubic,",
        "    PackagedRatedCoolEIRFFlow,  !- Name",
        "    1.0079484,               !- Coefficient1 Constant",
        "    0.34544129,              !- Coefficient2 x",
        "    -.6922891,               !- Coefficient3 x**2",
        "    0.33889943,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Quadratic,",
        "    VarSpeedCyclingPLFFPLR,  !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "	  Curve:Biquadratic,",
        "    VarSpeedCoolCapLSFT,     !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	 Curve:Biquadratic,",
        "    VarSpeedCoolEIRLSFT,     !- Name",
        "    0.774645E+00,            !- Coefficient1 Constant",
        "    -0.343731E-01,           !- Coefficient2 x",
        "    0.783173E-03,            !- Coefficient3 x**2",
        "    0.146596E-01,            !- Coefficient4 y",
        "    0.488851E-03,            !- Coefficient5 y**2",
        "    -0.752036E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        " NodeList,",
        "  Supply Fan Upstream Nodes,  !- Name",
        "  Mixed Air Node 1,        !- Node 1 Name",
        "   Main Cooling Coil 1 Outlet Node,  !- Node 2 Name",
        "   Main Heating Coil 1 Outlet Node;  !- Node 3 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);
    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.PLFFPLR(1)));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.PLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(54883.780765317439, thisCoil.RatedTotCap(1), 0.01);
    EXPECT_NEAR(18292.764129080300, thisCoil.RatedTotCap2, 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(3.3155291960328266, thisCoil.RatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(1.1050658810377409, thisCoil.RatedAirVolFlowRate2, 0.01);

    // Rated COP
    EXPECT_NEAR(3.0, thisCoil.RatedCOP(1), 0.01);
    EXPECT_NEAR(4.2, thisCoil.RatedCOP2, 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(934.39, thisCoil.FanPowerPerEvapAirFlowRate_2023(1), 0.01);
    EXPECT_NEAR(934.39, thisCoil.FanPowerPerEvapAirFlowRate_2023_LowSpeed(1), 0.01);

    EXPECT_EQ("Coil:Cooling:DX:TwoSpeed", thisCoil.DXCoilType);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType(1));
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType(1));

    // Ckeck user curve coefficients

    // CCapFTemp High Speed
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.CCapFTemp(1)));
    EXPECT_EQ(0.476428E+00, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.401147E-01, thisCCpaFTempHS->coeff[1]);
    // CCapFTemp Low Speed
    auto &thisCCpaFTempLS(state->dataCurveManager->curves(thisCoil.CCapFTemp2));
    EXPECT_EQ(0.476428E+00, thisCCpaFTempLS->coeff[0]);
    EXPECT_EQ(0.226411E-03, thisCCpaFTempLS->coeff[2]);

    // EIRFTemp High Speed Curve
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.EIRFTemp(1)));
    EXPECT_EQ(0.632475E+00, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(-0.121321E-01, thisEIRFTempHS->coeff[1]);
    // EIRFTemp Low Speed Curve
    auto &thisEIRFTempLS(state->dataCurveManager->curves(thisCoil.EIRFTemp2));
    EXPECT_EQ(0.774645E+00, thisEIRFTempLS->coeff[0]);
    EXPECT_EQ(-0.343731E-01, thisEIRFTempLS->coeff[1]);

    // CapFFlow High Speed
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.CCapFFlow(1)));
    EXPECT_EQ(0.47278589, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(1.2433415, thisCapFFlowHs->coeff[1]);
    // Note -- No CapFlow for Low Speed

    // EIRFFlow High Speed
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.EIRFFlow(1)));
    EXPECT_EQ(1.0079484, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(0.34544129, thisEIRFFlowHs->coeff[1]);
    // Note -- No EIRFlow for Low Speed

    std::map<std::string, Real64> StandardRatingsResult;

    StandardRatingsResult = TwoSpeedDXCoilStandardRatings(*state,
                                                          thisCoil.Name,
                                                          thisCoil.DXCoilType,
                                                          thisCoil.DXCoilType_Num,
                                                          thisCoil.RatedTotCap,
                                                          thisCoil.RatedTotCap2,
                                                          thisCoil.RatedCOP,
                                                          thisCoil.RatedCOP2,
                                                          thisCoil.CCapFFlow, // High Speed
                                                          thisCoil.CCapFTemp,
                                                          thisCoil.CCapFTemp2,
                                                          thisCoil.EIRFFlow, // High Speed
                                                          thisCoil.EIRFTemp,
                                                          thisCoil.EIRFTemp2,
                                                          thisCoil.RatedAirVolFlowRate,
                                                          thisCoil.RatedAirVolFlowRate2,
                                                          thisCoil.FanPowerPerEvapAirFlowRate_2023,
                                                          thisCoil.FanPowerPerEvapAirFlowRate_2023_LowSpeed,
                                                          thisCoil.CondenserType,
                                                          thisCoil.PLFFPLR(1));
    EXPECT_TRUE(StandardRatingsResult["SEER2_User"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["SEER2_Standard"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER2"] > 0.0);
    EXPECT_NEAR(3.42, StandardRatingsResult["SEER2_User"], 0.01);
    EXPECT_NEAR(51799.70, StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"], 0.01);
    EXPECT_NEAR(11.62, StandardRatingsResult["SEER2_Standard"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(2.53, StandardRatingsResult["EER2"], 0.01);

    EXPECT_TRUE(StandardRatingsResult["IEER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["EER_2022"] > 0.0);
    EXPECT_TRUE(StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] > 0.0);
    EXPECT_NEAR(3.26, StandardRatingsResult["IEER_2022"], 0.01);
    EXPECT_NEAR(51799.70, StandardRatingsResult["NetCoolingCapRatedMaxSpeed"], 0.01);
    EXPECT_NEAR(11.15, StandardRatingsResult["IEER_2022"] * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(2.41, StandardRatingsResult["EER_2022"], 0.01);
}

TEST_F(EnergyPlusFixture, TwoSpeedCoolingCoilAir_33000W_IEER_2022_ValueTest)
{
    std::string const idf_objects = delimited_string({
        "Coil:Cooling:DX:TwoSpeed,",
        "    Main Cooling Coil 1,                   !- Name",
        "    ,                                      !- Availability Schedule Name",                        // CoolingCoilAvailSched
        "    33703.17305,                           !- High Speed Gross Rated Total Cooling Capacity {W}", // = 115,000 Btu/h (rounded to the
                                                                                                           // nearest 1,000 per 6.1.2)
        "    0.8,                                   !- High Speed Rated Sensible Heat Ratio",
        "    3.9,                                   !- High Speed Gross Rated Cooling COP {W/W}",
        "    1.557426585,                           !- High Speed Rated Air Flow Rate {m3/s}", // 3,300 scfm
        "	 561.8242352,                           !- High Speed 2017 Rated Evaporator Fan Power Per Volume FlowRate[W / (m3 / s)] ",
        "    674.1890822,                           !- High Speed 2023 Rated Evaporator Fan Power Per Volume Flow Rate[W / (m3 / s)] ",
        "    450,                                   !- Unit Internal Static Air Pressure {Pa}",
        "    Mixed Air Node 1,                      !- Air Inlet Node Name",
        "    Main Cooling Coil 1 Outlet Node,       !- Air Outlet Node Name",
        "    VarSpeedCoolCapFT,                     !- Total Cooling Capacity Function of Temperature Curve Name",   // High Speed CapFT
        "    PackagedRatedCoolCapFFlow,             !- Total Cooling Capacity Function of Flow Fraction Curve Name", // High Speed CapFlow
        "    VarSpeedCoolEIRFT,                     !- Energy Input Ratio Function of Temperature Curve Name",       // High Speed EIRFT
        "    PackagedRatedCoolEIRFFlow,             !- Energy Input Ratio Function of Flow Fraction Curve Name",     // High Speed EIRFlow
        "    VarSpeedCyclingPLFFPLR,                !- Part Load Fraction Correlation Curve Name",
        "    18463.47741,                           !- Low Speed Gross Rated Total Cooling Capacity {W}", // 63,000 Btu/h
        "    0.8,                                   !- Low Speed Gross Rated Sensible Heat Ratio",
        "    3.1,                                   !- Low Speed Gross Rated Cooling COP {W/W}",
        "    0.9344,                                !- Low Speed Rated Air Flow Rate {m3/s}",                                       // 1980 scfm
        "    233.644,                               !- Low Speed 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3 /s)}", // 280.3770469
        "    280.394,                               !-Low Speed 2023 Rated Evaporator Fan Power Per Volume Flow Rate{W / (m3 / s)} ",
        "    VarSpeedCoolCapLSFT,                   !- Low Speed Total Cooling Capacity Function of Temperature Curve Name", // Low Speed CapFTemp
        "    VarSpeedCoolEIRLSFT,                   !- Low Speed Energy Input Ratio Function of Temperature Curve Name",     // Low Speed  EIRFTemp
        "    Main Cooling Coil 1 Condenser Node,    !- Condenser Air Inlet Node Name",
        "    AirCooled,                             !- Condenser Type",
        "    ,                                      !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                                      !- High Speed Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                                      !- High Speed Evaporative Condenser Air Flow Rate {m3/s}",
        "    ,                                      !- High Speed Evaporative Condenser Pump Rated Power Consumption {W}",
        "    ,                                      !- Low Speed Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                                      !- Low Speed Evaporative Condenser Air Flow Rate {m3/s}",
        "    ,                                      !- Low Speed Evaporative Condenser Pump Rated Power Consumption {W}",
        "    ,                                      !- Supply Water Storage Tank Name",
        "    ,                                      !- Condensate Collection Water Storage Tank Name",
        "    200,                                   !- Basin Heater Capacity {W/K}",
        "    2,                                     !- Basin Heater Setpoint Temperature {C}",
        "    ;                                      !- Basin Heater Operating Schedule Name", // BasinHeaterSched

        "	Curve:Biquadratic,",
        "    VarSpeedCoolCapFT,       !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	 Curve:Cubic,",
        "    PackagedRatedCoolCapFFlow,  !- Name",
        "    0.47278589,              !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    0.32257813,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "	 Curve:Biquadratic,",
        "    VarSpeedCoolEIRFT,       !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    PackagedRatedCoolEIRFFlow,  !- Name",
        "    1.0079484,               !- Coefficient1 Constant",
        "    0.34544129,              !- Coefficient2 x",
        "    -.6922891,               !- Coefficient3 x**2",
        "    0.33889943,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "	  Curve:Quadratic,",
        "    VarSpeedCyclingPLFFPLR,  !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "	 Curve:Biquadratic,",
        "    VarSpeedCoolCapLSFT,     !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	Curve:Biquadratic,",
        "    VarSpeedCoolEIRLSFT,     !- Name",
        "    0.774645E+00,            !- Coefficient1 Constant",
        "    -0.343731E-01,           !- Coefficient2 x",
        "    0.783173E-03,            !- Coefficient3 x**2",
        "    0.146596E-01,            !- Coefficient4 y",
        "    0.488851E-03,            !- Coefficient5 y**2",
        "    -0.752036E-03,           !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	  Schedule:Compact,",
        "    CoolingCoilAvailSched,   !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 3/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,0.0,        !- Field 3",
        "    Through: 9/30,           !- Field 5",
        "    For: WeekDays,           !- Field 6",
        "    Until: 7:00,0.0,         !- Field 7",
        "    Until: 17:00,1.0,        !- Field 9",
        "    Until: 24:00,0.0,        !- Field 11",
        "    For: SummerDesignDay WinterDesignDay, !- Field 13",
        "    Until: 24:00,1.0,        !- Field 14",
        "    For: AllOtherDays,       !- Field 16",
        "    Until: 24:00,0.0,        !- Field 17",
        "    Through: 12/31,          !- Field 19",
        "    For: AllDays,            !- Field 20",
        "    Until: 24:00,0.0;        !- Field 21",

        " OutdoorAir:Node,",
        "    Main Cooling Coil 1 Condenser Node,  !- Name",
        "    -1.0;                    !- Height Above Ground {m}",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);
    GetDXCoils(*state);

    auto &thisCoil(state->dataDXCoils->DXCoil(1));
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(thisCoil.PLFFPLR(1)));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);
    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, thisCoil.PLFFPLR(1), minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(33703.17, thisCoil.RatedTotCap(1), 0.01);
    EXPECT_NEAR(18463.47741, thisCoil.RatedTotCap2, 0.01);

    // Reated Air Vol Flow Rate
    EXPECT_NEAR(1.557426585, thisCoil.RatedAirVolFlowRate(1), 0.01);
    EXPECT_NEAR(0.9344, thisCoil.RatedAirVolFlowRate2, 0.01);

    // Rated COP
    EXPECT_NEAR(3.9, thisCoil.RatedCOP(1), 0.01);
    EXPECT_NEAR(3.1, thisCoil.RatedCOP2, 0.01);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(674.1890822, thisCoil.FanPowerPerEvapAirFlowRate_2023(1), 0.01);
    EXPECT_NEAR(280.394, thisCoil.FanPowerPerEvapAirFlowRate_2023_LowSpeed(1), 0.01);

    EXPECT_EQ("Coil:Cooling:DX:TwoSpeed", thisCoil.DXCoilType);

    EXPECT_TRUE(DataHeatBalance::RefrigCondenserType::Air == thisCoil.CondenserType(1));
    EXPECT_FALSE(DataHeatBalance::RefrigCondenserType::Evap == thisCoil.CondenserType(1));

    // Ckeck user curve coefficients

    // CCapFTemp High Speed
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(thisCoil.CCapFTemp(1)));
    EXPECT_EQ(0.476428E+00, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.401147E-01, thisCCpaFTempHS->coeff[1]);
    // CCapFTemp Low Speed
    auto &thisCCpaFTempLS(state->dataCurveManager->curves(thisCoil.CCapFTemp2));
    EXPECT_EQ(0.476428E+00, thisCCpaFTempLS->coeff[0]);
    EXPECT_EQ(0.226411E-03, thisCCpaFTempLS->coeff[2]);

    // EIRFTemp High Speed Curve
    auto &thisEIRFTempHS(state->dataCurveManager->curves(thisCoil.EIRFTemp(1)));
    EXPECT_EQ(0.632475E+00, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(-0.121321E-01, thisEIRFTempHS->coeff[1]);
    // EIRFTemp Low Speed Curve
    auto &thisEIRFTempLS(state->dataCurveManager->curves(thisCoil.EIRFTemp2));
    EXPECT_EQ(0.774645E+00, thisEIRFTempLS->coeff[0]);
    EXPECT_EQ(-0.343731E-01, thisEIRFTempLS->coeff[1]);

    // CapFFlow High Speed
    auto &thisCapFFlowHs(state->dataCurveManager->curves(thisCoil.CCapFFlow(1)));
    EXPECT_EQ(0.47278589, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(1.2433415, thisCapFFlowHs->coeff[1]);
    // Note -- No CapFlow for Low Speed

    // EIRFFlow High Speed
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(thisCoil.EIRFFlow(1)));
    EXPECT_EQ(1.0079484, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(0.34544129, thisEIRFFlowHs->coeff[1]);
    // Note -- No EIRFlow for Low Speed

    std::map<std::string, Real64> StandardRatingsResult;
    Real64 NetCoolingCapRated(0.0);
    Real64 IEER_2022(0.0);
    Real64 EER_2022(0.0);
    Real64 NetCoolingCapRated2022(0.0);

    Array1D_int TSCCapFTemp;
    TSCCapFTemp.push_back(thisCoil.CCapFTemp(1));
    TSCCapFTemp.push_back(thisCoil.CCapFTemp2);

    Array1D<Real64> TSFanPowerPerEvapAirFlowRate2023;
    TSFanPowerPerEvapAirFlowRate2023.push_back(thisCoil.FanPowerPerEvapAirFlowRate_2023(1));
    TSFanPowerPerEvapAirFlowRate2023.push_back(thisCoil.FanPowerPerEvapAirFlowRate_2023_LowSpeed(1));

    Array1D<Real64> TSRatedTotCap;
    TSRatedTotCap.push_back(thisCoil.RatedTotCap(1));
    TSRatedTotCap.push_back(thisCoil.RatedTotCap2);

    Array1D<Real64> TSRatedAirVolFlowRate;
    TSRatedAirVolFlowRate.push_back(thisCoil.RatedAirVolFlowRate(1));
    TSRatedAirVolFlowRate.push_back(thisCoil.RatedAirVolFlowRate2);

    Array1D_int TSEIRFTemp;
    TSEIRFTemp.push_back(thisCoil.EIRFTemp(1));
    TSEIRFTemp.push_back(thisCoil.EIRFTemp2);

    Array1D<Real64> TSRatedCOP;
    TSRatedCOP.push_back(thisCoil.RatedCOP(1));
    TSRatedCOP.push_back(thisCoil.RatedCOP2);

    std::tie(IEER_2022, NetCoolingCapRated2022, EER_2022) = IEERCalculationTwoSpeed(*state,
                                                                                    thisCoil.DXCoilType,
                                                                                    thisCoil.CondenserType,
                                                                                    TSCCapFTemp,
                                                                                    TSRatedTotCap,
                                                                                    thisCoil.CCapFFlow,
                                                                                    TSFanPowerPerEvapAirFlowRate2023,
                                                                                    TSRatedAirVolFlowRate,
                                                                                    TSEIRFTemp,
                                                                                    TSRatedCOP,
                                                                                    thisCoil.EIRFFlow);

    NetCoolingCapRated = NetCoolingCapRated2022;
    EXPECT_TRUE(IEER_2022 > 0.0);
    EXPECT_TRUE(EER_2022 > 0.0);
    EXPECT_TRUE(NetCoolingCapRated2022 > 0.0);
    EXPECT_NEAR(3.7132772690788105, IEER_2022, 0.01);
    EXPECT_NEAR(3.3559704312515519, EER_2022, 0.01);
    EXPECT_NEAR(32661.744875626890, NetCoolingCapRated, 0.01);
    EXPECT_NEAR(12.670, IEER_2022 * StandardRatings::ConvFromSIToIP, 0.01);
}

TEST_F(EnergyPlusFixture, CurveFit_02_Speed_15000W_alternateMode_SEER2_2023_ValueTest)
{
    std::string const idf_objects = delimited_string({

        "  Schedule:Compact, AVAILSCHED, FRACTION, Through: 12/31, For: Alldays, Until: 24:00,1.00; ",

        "  CoilSystem:Cooling:DX,",
        "    DX Cooling Coil System,          !- Name",
        "    AvailSched,                      !- Availability Schedule Name",
        "    DX Cooling Coil Air Inlet Node,  !- DX Cooling Coil System Inlet Node Name",
        "    Heating Coil Air Inlet Node,     !- DX Cooling Coil System Outlet Node Name",
        "    Heating Coil Air Inlet Node,     !- DX Cooling Coil System Sensor Node Name",
        "    Coil:Cooling:DX,                 !- Cooling Coil Object Type",
        "    DX Cooling Coil,                 !- Cooling Coil Name",
        "    CoolReheat,                      !- Dehumidification Control Type",
        "    Yes,                             !- Run on Sensible Load",
        "    Yes,                             !- Run on Latent Load",
        "    No;                              !- Use Outdoor Air DX Cooling Coil",

        "  Coil:Cooling:DX,",
        "    DX Cooling Coil,                 !- Name",
        "    DX Cooling Coil Air Inlet Node,  !- Evaporator Inlet Node Name",
        "    Heating Coil Air Inlet Node,     !- Evaporator Outlet Node Name",
        "    ,                                !- Availability Schedule Name",
        "    ,                                !- Condenser Zone Name",
        "    DX Cool Cooling Coil Condenser Inlet,        !- Condenser Inlet Node Name",
        "    DX Cool Cooling Coil Condenser Outlet Node,  !- Condenser Outlet Node Name",
        "    DX Cool Cooling Coil Performance,            !- Performance Object Name",
        "    ,                                !- Condensate Collection Water Storage Tank Name",
        "    ;                                !- Evaporative Condenser Supply Water Storage Tank Name",

        "  Coil:Cooling:DX:CurveFit:Performance,",
        "    DX Cool Cooling Coil Performance,  !- Name",
        "    0,                       !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    10,                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Unit Internal Static Air Pressure {Pa}",
        "    ,                        !- Capacity Control Method",
        "    ,                        !- Evaporative Condenser Basin Heater Capacity {W/K}",
        "    ,                        !- Evaporative Condenser Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Evaporative Condenser Basin Heater Operating Schedule Name",
        "    Electricity,             !- Compressor Fuel Type",
        "    DX Cool Cooling Coil Operating Mode,  !- Base Operating Mode",
        "    DX Cool Cooling Coil Operating Mode2, !- Alternative Operating Mode 1",
        "    ;                        !- Alternative Operating Mode 1",

        "  Coil:Cooling:DX:CurveFit:OperatingMode,",
        "    DX Cool Cooling Coil Operating Mode,  !- Name",
        "    15000,                   !- Rated Gross Total Cooling Capacity {W}",
        "    0.8,                     !- Rated Evaporator Air Flow Rate {m3/s}",
        "    ,                        !- Rated Condenser Air Flow Rate {m3/s}",
        "    0,                       !- Maximum Cycling Rate {cycles/hr}",
        "    0,                       !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    0,                       !- Latent Capacity Time Constant {s}",
        "    0,                       !- Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Apply Latent Degradation to Speeds Greater than 1",
        "    AirCooled,               !- Condenser Type",
        "    0,                       !- Nominal Evaporative Condenser Pump Power {W}",
        "    2,                       !- Nominal Speed Number",
        "    DX Cool Cooling Coil Speed 1 Performance,  !- Speed 1 Name",
        "    DX Cool Cooling Coil Speed 2 Performance;  !- Speed 2 Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    DX Cool Cooling Coil Speed 1 Performance,  !- Name",
        "    0.5,                     !- Gross Total Cooling Capacity Fraction",
        "    0.5,                     !- Evaporator Air Flow Rate Fraction",
        "    0.5,                     !- Condenser Air Flow Rate Fraction",
        "    0.77,                    !- Gross Sensible Heat Ratio",
        "    4.17,                    !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    453.3,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.5,                     !- Evaporative Condenser Pump Power Fraction",
        "    0,                       !- Evaporative Condenser Effectiveness {dimensionless}",
        "    1Cap,  !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    CAPFF,  !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    1Pow,  !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    EIRFF,  !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    PLFCurve,  !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,  !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;  !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    DX Cool Cooling Coil Speed 2 Performance,  !- Name",
        "    1.0,                     !- Gross Total Cooling Capacity Fraction",
        "    1.0,                     !- Evaporator Air Flow Rate Fraction",
        "    1.0,                     !- Condenser Air Flow Rate Fraction",
        "    0.77,                    !- Gross Sensible Heat Ratio",
        "    4.17,                    !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    631.3,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    1.0,                     !- Evaporative Condenser Pump Power Fraction",
        "    0,                       !- Evaporative Condenser Effectiveness {dimensionless}",
        "    1Cap,  !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    CAPFF,  !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    1Pow,  !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    EIRFF,  !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    PLFCurve,  !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,  !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;  !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "  Coil:Cooling:DX:CurveFit:OperatingMode,",
        "    DX Cool Cooling Coil Operating Mode2,  !- Name",
        "    15000,                   !- Rated Gross Total Cooling Capacity {W}",
        "    0.7,                     !- Rated Evaporator Air Flow Rate {m3/s}",
        "    ,                        !- Rated Condenser Air Flow Rate {m3/s}",
        "    0,                       !- Maximum Cycling Rate {cycles/hr}",
        "    0,                       !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    0,                       !- Latent Capacity Time Constant {s}",
        "    0,                       !- Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Apply Latent Degradation to Speeds Greater than 1",
        "    AirCooled,               !- Condenser Type",
        "    0,                       !- Nominal Evaporative Condenser Pump Power {W}",
        "    2,                       !- Nominal Speed Number",
        "    DX Cool Cooling Coil Speed 1 Performance2,  !- Speed 1 Name",
        "    DX Cool Cooling Coil Speed 2 Performance2;  !- Speed 2 Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    DX Cool Cooling Coil Speed 1 Performance2,  !- Name",
        "    0.5,                     !- Gross Total Cooling Capacity Fraction",
        "    0.5,                     !- Evaporator Air Flow Rate Fraction",
        "    0.5,                     !- Condenser Air Flow Rate Fraction",
        "    0.67,                    !- Gross Sensible Heat Ratio",
        "    4.17,                    !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.5,                     !- Evaporative Condenser Pump Power Fraction",
        "    0,                       !- Evaporative Condenser Effectiveness {dimensionless}",
        "    1Cap,  !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    CAPFF,  !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    1Pow,  !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    EIRFF,  !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    PLFCurve,  !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,  !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;  !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    DX Cool Cooling Coil Speed 2 Performance2,  !- Name",
        "    1.0,                     !- Gross Total Cooling Capacity Fraction",
        "    1.0,                     !- Evaporator Air Flow Rate Fraction",
        "    1.0,                     !- Condenser Air Flow Rate Fraction",
        "    0.67,                    !- Gross Sensible Heat Ratio",
        "    4.17,                    !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    1.0,                     !- Evaporative Condenser Pump Power Fraction",
        "    0,                       !- Evaporative Condenser Effectiveness {dimensionless}",
        "    1Cap,  !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    CAPFF,  !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    1Pow,  !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    EIRFF,  !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    PLFCurve,  !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,  !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;  !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "Curve:Quadratic, PLFCurve, 0.85, 0.83, 0.0, 0.0, 0.3, 0.85, 1.0, Dimensionless, Dimensionless; ",
        "Curve:Cubic, CAPFF, 1, 0, 0, 0, 0, 1, , , Dimensionless, Dimensionless; ",
        "Curve:Cubic, EIRFF, 1, 0, 0, 0, 0, 1, , , Dimensionless, Dimensionless; ",
        "Curve:Biquadratic, 1Cap, 0.483, 0.0305, 0.0000458, 0.00511, -1.50E-04, -1.28E-04, 8.89, 21.67, 12.78, 51.67, , , "
        "Temperature,Temperature,Dimensionless;",
        "Curve:Biquadratic, 1Pow, 1.33, -0.034, 0.00094, -0.0086, 0.00077, -0.000972, 8.89, 21.7, 12.8, 51.7, , , Temperature, Temperature, "
        "Dimensionless;"

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    int coilIndex = CoilCoolingDX::factory(*state, "DX Cooling Coil");
    auto &thisCoil(state->dataCoilCoolingDX->coilCoolingDXs[coilIndex]);
    // size it
    thisCoil.size(*state);
    auto performance{dynamic_cast<CoilCoolingDXCurveFitPerformance *>(thisCoil.performance.get())};

    ASSERT_EQ("DX COOLING COIL", thisCoil.name);
    ASSERT_EQ("DX COOL COOLING COIL PERFORMANCE", performance->name);
    ASSERT_EQ("DX COOL COOLING COIL OPERATING MODE", performance->normalMode.name);
    ASSERT_EQ("DX COOL COOLING COIL OPERATING MODE2", performance->alternateMode.name);
    int nsp = performance->numSpeeds();
    ASSERT_EQ(2, nsp);
    auto speed1 = performance->normalMode.speeds[0];
    ASSERT_EQ("DX COOL COOLING COIL SPEED 1 PERFORMANCE", speed1.name);
    auto speed2 = performance->normalMode.speeds[1];
    ASSERT_EQ("DX COOL COOLING COIL SPEED 2 PERFORMANCE", speed2.name);

    auto coilMode = performance->maxAvailCoilMode;
    auto normalMode = performance->normalMode.speeds;
    auto alternateMode1 = performance->alternateMode;
    EXPECT_EQ((int)HVAC::CoilMode::Enhanced, (int)coilMode);
    EXPECT_TRUE(2 == normalMode.size());
    auto speedA1 = alternateMode1.speeds[0];
    ASSERT_EQ("DX COOL COOLING COIL SPEED 1 PERFORMANCE2", speedA1.name);
    auto speedA2 = alternateMode1.speeds[1];
    ASSERT_EQ("DX COOL COOLING COIL SPEED 2 PERFORMANCE2", speedA2.name);

    auto pLFfPLR_Curve = speed1.indexPLRFPLF;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.83, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(0.3, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(0.3, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(15000, performance->ratedGrossTotalCap(), 0.01);

    // Reated Air Vol Flow Rate | evap air flow rate and condenser air flow rate ??
    EXPECT_NEAR(0.80, performance->ratedEvapAirFlowRate(*state), 0.01);

    EXPECT_NEAR(7500, speed1.rated_total_capacity, 0.01);
    EXPECT_NEAR(15000, speed2.rated_total_capacity, 0.01);

    EXPECT_NEAR(0.40, speed1.evap_air_flow_rate, 0.01);
    EXPECT_NEAR(0.80, speed2.evap_air_flow_rate, 0.01);

    // Rated COP
    EXPECT_EQ(4.17, speed1.ratedCOP);
    EXPECT_EQ(4.17, speed2.ratedCOP);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(453.3, speed1.rated_evap_fan_power_per_volume_flow_rate_2023, 0.01);
    EXPECT_NEAR(631.3, speed2.rated_evap_fan_power_per_volume_flow_rate_2023, 0.01);

    // CondenserType is a different enum that is being used in case of CurveFit in comparison to other Cooling DX Coils
    EXPECT_TRUE(CoilCoolingDXCurveFitOperatingMode::CondenserType::AIRCOOLED == performance->normalMode.condenserType);
    EXPECT_FALSE(CoilCoolingDXCurveFitOperatingMode::CondenserType::EVAPCOOLED == performance->normalMode.condenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(speed1.indexCapFT));
    EXPECT_EQ(0.483, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.0305, thisCCpaFTempHS->coeff[1]);

    // EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(speed1.indexEIRFT));
    EXPECT_EQ(1.33, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(-0.034, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(speed1.indexCapFFF));
    EXPECT_EQ(1, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0, thisCapFFlowHs->coeff[1]);

    // EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(speed1.indexEIRFFF));
    EXPECT_EQ(1, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(0, thisEIRFFlowHs->coeff[1]);

    ASSERT_TRUE(performance->standardRatingEER == 0);
    ASSERT_TRUE(performance->standardRatingSEER == 0);
    ASSERT_TRUE(performance->standardRatingCoolingCapacity == 0);

    ASSERT_TRUE(performance->standardRatingSEER2_User == 0);
    ASSERT_TRUE(performance->standardRatingSEER2_Standard == 0);
    ASSERT_TRUE(performance->standardRatingCoolingCapacity2023 == 0);

    performance->calcStandardRatings210240(*state);
    // 2017 TBD
    ASSERT_TRUE(performance->standardRatingEER > 0);
    ASSERT_TRUE(performance->standardRatingSEER > 0);
    ASSERT_TRUE(performance->standardRatingCoolingCapacity > 0);
    EXPECT_NEAR(3.39, performance->standardRatingEER, 0.01);
    EXPECT_NEAR(3.93, performance->standardRatingSEER, 0.01);
    EXPECT_NEAR(14399.91, performance->standardRatingCoolingCapacity, 0.01);
    EXPECT_NEAR(13.42, performance->standardRatingSEER * StandardRatings::ConvFromSIToIP, 0.01);
    // 2023
    ASSERT_TRUE(performance->standardRatingEER2 > 0);
    ASSERT_TRUE(performance->standardRatingSEER2_User > 0.0);
    EXPECT_TRUE(performance->standardRatingSEER2_Standard > 0.0);
    EXPECT_TRUE(performance->standardRatingCoolingCapacity2023 > 0.0);
    EXPECT_NEAR(3.51, performance->standardRatingEER2, 0.01);
    EXPECT_NEAR(4.18, performance->standardRatingSEER2_User, 0.01);
    EXPECT_NEAR(4.12, performance->standardRatingSEER2_Standard, 0.01);
    EXPECT_NEAR(14513.51, performance->standardRatingCoolingCapacity2023, 0.01);
    EXPECT_NEAR(14.26, performance->standardRatingSEER2_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(14.07, performance->standardRatingSEER2_Standard * StandardRatings::ConvFromSIToIP, 0.01);

    ASSERT_TRUE(performance->standardRatingIEER > 0);
    EXPECT_NEAR(3.42, performance->standardRatingIEER, 0.01);
    ASSERT_TRUE(performance->standardRatingIEER2 > 0);
    EXPECT_NEAR(3.97, performance->standardRatingIEER2, 0.01);
}

TEST_F(EnergyPlusFixture, CurveFit_03_Speed_5000W_SEER2_2023_ValueTest)
{
    std::string idf_objects = delimited_string({
        "  Coil:Cooling:DX,",
        "    Sys 2 Furnace DX Cool Cooling Coil,  !- Name",
        "    Sys 2 Furnace DX Cool Mixed Air Outlet,  !- Evaporator Inlet Node Name",
        "    Sys 2 Furnace DX Cool Cooling Coil Outlet,  !- Evaporator Outlet Node Name",
        "    ,                        !- Availability Schedule Name",
        "    ,                        !- Condenser Zone Name",
        "    Sys 2 Furnace DX Cool Cooling Coil Condenser Inlet,  !- Condenser Inlet Node Name",
        "    Sys 2 Furnace DX Cool Cooling Coil Condenser Outlet Node,  !- Condenser Outlet Node Name",
        "    Sys 2 Furnace DX Cool Cooling Coil Performance,  !- Performance Object Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ;                        !- Evaporative Condenser Supply Water Storage Tank Name",

        "  Coil:Cooling:DX:CurveFit:Performance,",
        "    Sys 2 Furnace DX Cool Cooling Coil Performance,  !- Name",
        "    0.0,                     !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Unit Internal Static Air Pressure {Pa}",
        "    Discrete,                !- Capacity Control Method",
        "    ,                        !- Evaporative Condenser Basin Heater Capacity {W/K}",
        "    ,                        !- Evaporative Condenser Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Evaporative Condenser Basin Heater Operating Schedule Name",
        "    Electricity,             !- Compressor Fuel Type",
        "    Sys 2 Furnace DX Cool Cooling Coil Operating Mode;  !- Base Operating Mode",

        "	 Coil:Cooling:DX:CurveFit:OperatingMode,",
        "    Sys 2 Furnace DX Cool Cooling Coil Operating Mode,  !- Name",
        "    5000,                !- Rated Gross Total Cooling Capacity {W}",
        "    0.25,                !- Rated Evaporator Air Flow Rate {m3/s}",
        "    ,                        !- Rated Condenser Air Flow Rate {m3/s}",
        "    0,                       !- Maximum Cycling Rate {cycles/hr}",
        "    0,                       !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    0,                       !- Latent Capacity Time Constant {s}",
        "    0,                       !- Nominal Time for Condensate Removal to Begin {s}",
        "    No,                      !- Apply Latent Degradation to Speeds Greater than 1",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Nominal Evaporative Condenser Pump Power {W}",
        "    3,                       !- Nominal Speed Number",
        "    Sys 2 Furnace DX Cool Cooling Coil Speed 1 Performance,  !- Speed 1 Name",
        "    Sys 2 Furnace DX Cool Cooling Coil Speed 2 Performance,  !- Speed 2 Name",
        "    Sys 2 Furnace DX Cool Cooling Coil Speed 3 Performance;  !- Speed 3 Name",

        "	Coil:Cooling:DX:CurveFit:Speed,",
        "    Sys 2 Furnace DX Cool Cooling Coil Speed 1 Performance,  !- Name",
        "    0.3333,                  !- Gross Total Cooling Capacity Fraction",
        "    0.3333,                  !- Evaporator Air Flow Rate Fraction",
        "    ,                        !- Condenser Air Flow Rate Fraction",
        "    autosize,                !- Gross Sensible Heat Ratio",
        "    3,                       !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    377.8,                   !- Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    453.3,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    ,                        !- Evaporative Condenser Pump Power Fraction",
        "    ,                        !- Evaporative Condenser Effectiveness {dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FT,  !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FF,  !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FT,  !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FF,  !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil PLF,  !- Part Load Fraction Correlation Curve Name",
        "    0.2,                     !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil WH-FT,  !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,                        !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;                        !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    Sys 2 Furnace DX Cool Cooling Coil Speed 2 Performance,  !- Name",
        "    0.6667,                  !- Gross Total Cooling Capacity Fraction",
        "    0.6667,                  !- Evaporator Air Flow Rate Fraction",
        "    ,                        !- Condenser Air Flow Rate Fraction",
        "    autosize,                !- Gross Sensible Heat Ratio",
        "    3,                       !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    511.0,                        !- Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    631.3,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    ,                        !- Evaporative Condenser Pump Power Fraction",
        "    ,                        !- Evaporative Condenser Effectiveness {dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FT,  !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FF,  !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FT,  !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FF,  !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil PLF,  !- Part Load Fraction Correlation Curve Name",
        "    0.2,                     !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil WH-FT,  !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,                        !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;                        !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    Sys 2 Furnace DX Cool Cooling Coil Speed 3 Performance,  !- Name",
        "    1.0000,                  !- Gross Total Cooling Capacity Fraction",
        "    1.0000,                  !- Evaporator Air Flow Rate Fraction",
        "    ,                        !- Condenser Air Flow Rate Fraction",
        "    autosize,                !- Gross Sensible Heat Ratio",
        "    3,                       !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    677.4,                        !- Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    812.9,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    ,                        !- Evaporative Condenser Pump Power Fraction",
        "    ,                        !- Evaporative Condenser Effectiveness {dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FT,  !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FF,  !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FT,  !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FF,  !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil PLF,  !- Part Load Fraction Correlation Curve Name",
        "    0.2,                     !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil WH-FT,  !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,                        !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;                        !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        //   "	! Curves from example file MultiSpeedHeatPump.idf, Sep 2013, same curves for all speeds."

        "  Curve:Biquadratic,",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FT,  !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    50.0,                    !- Maximum Value of x",
        "    0.0,                     !- Minimum Value of y",
        "    50.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    5.0,                     !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	 Curve:Cubic,",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FF,  !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "	  Curve:Biquadratic,",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FT,  !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    50.0,                    !- Maximum Value of x",
        "    0.0,                     !- Minimum Value of y",
        "    50.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    5.0,                     !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	 Curve:Cubic,",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FF,  !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        //    "	! PLF = l.- Cd(1.-PLR) where Cd = 0.15"

        "  Curve:Quadratic,",
        "    Sys 2 Furnace DX Cool Cool Coil PLF,  !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0,                       !- Minimum Value of x",
        "    1;                       !- Maximum Value of x",

        "	  Curve:Biquadratic,",
        "    Sys 2 Furnace DX Cool Cool Coil WH-FT,  !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    50,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    int coilIndex = CoilCoolingDX::factory(*state, "Sys 2 Furnace DX Cool Cooling Coil");
    auto &thisCoil(state->dataCoilCoolingDX->coilCoolingDXs[coilIndex]);

    // size it
    thisCoil.size(*state);

    auto performance{dynamic_cast<CoilCoolingDXCurveFitPerformance *>(thisCoil.performance.get())};

    ASSERT_EQ("SYS 2 FURNACE DX COOL COOLING COIL", thisCoil.name);
    ASSERT_EQ("SYS 2 FURNACE DX COOL COOLING COIL PERFORMANCE", performance->name);
    ASSERT_EQ("SYS 2 FURNACE DX COOL COOLING COIL OPERATING MODE", performance->normalMode.name);
    int nsp = performance->numSpeeds();
    ASSERT_EQ(3, nsp);
    auto speed1 = performance->normalMode.speeds[0];
    ASSERT_EQ("SYS 2 FURNACE DX COOL COOLING COIL SPEED 1 PERFORMANCE", speed1.name);
    auto speed2 = performance->normalMode.speeds[1];
    ASSERT_EQ("SYS 2 FURNACE DX COOL COOLING COIL SPEED 2 PERFORMANCE", speed2.name);
    auto speed3 = performance->normalMode.speeds[2];
    ASSERT_EQ("SYS 2 FURNACE DX COOL COOLING COIL SPEED 3 PERFORMANCE", speed3.name);

    HVAC::CoilMode coilMode = performance->maxAvailCoilMode;
    auto alternateMode1 = performance->alternateMode.speeds;
    auto alternateMode2 = performance->alternateMode2.speeds;
    EXPECT_EQ((int)HVAC::CoilMode::Normal, (int)coilMode);
    EXPECT_TRUE(alternateMode1.empty());
    EXPECT_TRUE(alternateMode2.empty());
    auto pLFfPLR_Curve = speed1.indexPLRFPLF;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(5000, performance->ratedGrossTotalCap(), 0.01);

    // Reated Air Vol Flow Rate | evap air flow rate and condenser air flow rate ??
    EXPECT_NEAR(0.25, performance->ratedEvapAirFlowRate(*state), 0.01);

    EXPECT_NEAR(1666.5, speed1.rated_total_capacity, 0.01);
    EXPECT_NEAR(3333.5, speed2.rated_total_capacity, 0.01);
    EXPECT_NEAR(5000.0, speed3.rated_total_capacity, 0.01);

    EXPECT_NEAR(0.08, speed1.evap_air_flow_rate, 0.01);
    EXPECT_NEAR(0.16, speed2.evap_air_flow_rate, 0.01);
    EXPECT_NEAR(0.25, speed3.evap_air_flow_rate, 0.01);

    // Rated COP
    EXPECT_EQ(3, speed1.ratedCOP);
    EXPECT_EQ(3, speed2.ratedCOP);
    EXPECT_EQ(3, speed3.ratedCOP);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(453.3, speed1.rated_evap_fan_power_per_volume_flow_rate_2023, 0.01);
    EXPECT_NEAR(631.3, speed2.rated_evap_fan_power_per_volume_flow_rate_2023, 0.01);
    EXPECT_NEAR(812.9, speed3.rated_evap_fan_power_per_volume_flow_rate_2023, 0.01);

    // CondenserType is a different enum that is being used in case of CurveFit in comparison to other Cooling DX Coils
    EXPECT_TRUE(CoilCoolingDXCurveFitOperatingMode::CondenserType::AIRCOOLED == performance->normalMode.condenserType);
    EXPECT_FALSE(CoilCoolingDXCurveFitOperatingMode::CondenserType::EVAPCOOLED == performance->normalMode.condenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(speed1.indexCapFT));
    EXPECT_EQ(0.476428E+00, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.401147E-01, thisCCpaFTempHS->coeff[1]);

    // EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(speed1.indexEIRFT));
    EXPECT_EQ(0.632475E+00, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(-0.121321E-01, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(speed1.indexCapFFF));
    EXPECT_EQ(.47278589, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(1.2433415, thisCapFFlowHs->coeff[1]);

    // EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(speed1.indexEIRFFF));
    EXPECT_EQ(.47278589, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(1.2433415, thisEIRFFlowHs->coeff[1]);

    ASSERT_TRUE(performance->standardRatingEER == 0);
    ASSERT_TRUE(performance->standardRatingSEER == 0);
    ASSERT_TRUE(performance->standardRatingCoolingCapacity == 0);

    ASSERT_TRUE(performance->standardRatingSEER2_User == 0);
    ASSERT_TRUE(performance->standardRatingSEER2_Standard == 0);
    ASSERT_TRUE(performance->standardRatingCoolingCapacity2023 == 0);

    performance->calcStandardRatings210240(*state);

    ASSERT_TRUE(performance->standardRatingEER > 0);
    ASSERT_TRUE(performance->standardRatingSEER > 0);
    ASSERT_TRUE(performance->standardRatingCoolingCapacity > 0);
    EXPECT_NEAR(2.62, performance->standardRatingEER, 0.01);
    EXPECT_NEAR(2.80, performance->standardRatingSEER, 0.01);
    EXPECT_NEAR(4831.92, performance->standardRatingCoolingCapacity, 0.01);
    EXPECT_NEAR(9.56, performance->standardRatingSEER * StandardRatings::ConvFromSIToIP, 0.01);

    ASSERT_TRUE(performance->standardRatingEER2 > 0);
    ASSERT_TRUE(performance->standardRatingSEER2_User > 0.0);
    EXPECT_TRUE(performance->standardRatingSEER2_Standard > 0.0);
    EXPECT_TRUE(performance->standardRatingCoolingCapacity2023 > 0.0);
    EXPECT_NEAR(2.55, performance->standardRatingEER2, 0.01);
    EXPECT_NEAR(3.05, performance->standardRatingSEER2_User, 0.01);
    EXPECT_NEAR(3.07, performance->standardRatingSEER2_Standard, 0.01);
    EXPECT_NEAR(4798.04, performance->standardRatingCoolingCapacity2023, 0.01);
    EXPECT_NEAR(10.41, performance->standardRatingSEER2_User * StandardRatings::ConvFromSIToIP, 0.01);
    EXPECT_NEAR(10.47, performance->standardRatingSEER2_Standard * StandardRatings::ConvFromSIToIP, 0.01);

    ASSERT_TRUE(performance->standardRatingIEER > 0);
    EXPECT_NEAR(2.78, performance->standardRatingIEER, 0.01);
    ASSERT_TRUE(performance->standardRatingIEER2 > 0);
    EXPECT_NEAR(3.17, performance->standardRatingIEER2, 0.01);
}

TEST_F(EnergyPlusFixture, CurveFit_02_Speed_30000W_alternateMode_IEER_2022_ValueTest)
{
    std::string const idf_objects = delimited_string({

        "  Schedule:Compact, AVAILSCHED, FRACTION, Through: 12/31, For: Alldays, Until: 24:00,1.00; ",

        "  CoilSystem:Cooling:DX,",
        "    DX Cooling Coil System,          !- Name",
        "    AvailSched,                      !- Availability Schedule Name",
        "    DX Cooling Coil Air Inlet Node,  !- DX Cooling Coil System Inlet Node Name",
        "    Heating Coil Air Inlet Node,     !- DX Cooling Coil System Outlet Node Name",
        "    Heating Coil Air Inlet Node,     !- DX Cooling Coil System Sensor Node Name",
        "    Coil:Cooling:DX,                 !- Cooling Coil Object Type",
        "    DX Cooling Coil,                 !- Cooling Coil Name",
        "    CoolReheat,                      !- Dehumidification Control Type",
        "    Yes,                             !- Run on Sensible Load",
        "    Yes,                             !- Run on Latent Load",
        "    No;                              !- Use Outdoor Air DX Cooling Coil",

        "  Coil:Cooling:DX,",
        "    DX Cooling Coil,                 !- Name",
        "    DX Cooling Coil Air Inlet Node,  !- Evaporator Inlet Node Name",
        "    Heating Coil Air Inlet Node,     !- Evaporator Outlet Node Name",
        "    ,                                !- Availability Schedule Name",
        "    ,                                !- Condenser Zone Name",
        "    DX Cool Cooling Coil Condenser Inlet,        !- Condenser Inlet Node Name",
        "    DX Cool Cooling Coil Condenser Outlet Node,  !- Condenser Outlet Node Name",
        "    DX Cool Cooling Coil Performance,            !- Performance Object Name",
        "    ,                                !- Condensate Collection Water Storage Tank Name",
        "    ;                                !- Evaporative Condenser Supply Water Storage Tank Name",

        "  Coil:Cooling:DX:CurveFit:Performance,",
        "    DX Cool Cooling Coil Performance,  !- Name",
        "    0,                       !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    10,                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Unit Internal Static Air Pressure {Pa}",
        "    ,                        !- Capacity Control Method",
        "    ,                        !- Evaporative Condenser Basin Heater Capacity {W/K}",
        "    ,                        !- Evaporative Condenser Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Evaporative Condenser Basin Heater Operating Schedule Name",
        "    Electricity,             !- Compressor Fuel Type",
        "    DX Cool Cooling Coil Operating Mode,  !- Base Operating Mode",
        "    DX Cool Cooling Coil Operating Mode2, !- Alternative Operating Mode 1",
        "    ;                        !- Alternative Operating Mode 1",

        "  Coil:Cooling:DX:CurveFit:OperatingMode,",
        "    DX Cool Cooling Coil Operating Mode,  !- Name",
        "    30000,                   !- Rated Gross Total Cooling Capacity {W}", // 1500
        "    1.6,                     !- Rated Evaporator Air Flow Rate {m3/s}",  // 0.8
        "    ,                        !- Rated Condenser Air Flow Rate {m3/s}",
        "    0,                       !- Maximum Cycling Rate {cycles/hr}",
        "    0,                       !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    0,                       !- Latent Capacity Time Constant {s}",
        "    0,                       !- Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Apply Latent Degradation to Speeds Greater than 1",
        "    AirCooled,               !- Condenser Type",
        "    0,                       !- Nominal Evaporative Condenser Pump Power {W}",
        "    2,                       !- Nominal Speed Number",
        "    DX Cool Cooling Coil Speed 1 Performance,  !- Speed 1 Name",
        "    DX Cool Cooling Coil Speed 2 Performance;  !- Speed 2 Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    DX Cool Cooling Coil Speed 1 Performance,  !- Name",
        "    0.5,                     !- Gross Total Cooling Capacity Fraction",
        "    0.5,                     !- Evaporator Air Flow Rate Fraction",
        "    0.5,                     !- Condenser Air Flow Rate Fraction",
        "    0.77,                    !- Gross Sensible Heat Ratio",
        "    4.17,                    !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    453.3,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.5,                     !- Evaporative Condenser Pump Power Fraction",
        "    0,                       !- Evaporative Condenser Effectiveness {dimensionless}",
        "    1Cap,  !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    CAPFF,  !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    1Pow,  !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    EIRFF,  !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    PLFCurve,  !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,  !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;  !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    DX Cool Cooling Coil Speed 2 Performance,  !- Name",
        "    1.0,                     !- Gross Total Cooling Capacity Fraction",
        "    1.0,                     !- Evaporator Air Flow Rate Fraction",
        "    1.0,                     !- Condenser Air Flow Rate Fraction",
        "    0.77,                    !- Gross Sensible Heat Ratio",
        "    4.17,                    !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    631.3,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    1.0,                     !- Evaporative Condenser Pump Power Fraction",
        "    0,                       !- Evaporative Condenser Effectiveness {dimensionless}",
        "    1Cap,  !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    CAPFF,  !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    1Pow,  !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    EIRFF,  !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    PLFCurve,  !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,  !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;  !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "  Coil:Cooling:DX:CurveFit:OperatingMode,",
        "    DX Cool Cooling Coil Operating Mode2,  !- Name",
        "    15000,                   !- Rated Gross Total Cooling Capacity {W}",
        "    0.7,                     !- Rated Evaporator Air Flow Rate {m3/s}",
        "    ,                        !- Rated Condenser Air Flow Rate {m3/s}",
        "    0,                       !- Maximum Cycling Rate {cycles/hr}",
        "    0,                       !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    0,                       !- Latent Capacity Time Constant {s}",
        "    0,                       !- Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Apply Latent Degradation to Speeds Greater than 1",
        "    AirCooled,               !- Condenser Type",
        "    0,                       !- Nominal Evaporative Condenser Pump Power {W}",
        "    2,                       !- Nominal Speed Number",
        "    DX Cool Cooling Coil Speed 1 Performance2,  !- Speed 1 Name",
        "    DX Cool Cooling Coil Speed 2 Performance2;  !- Speed 2 Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    DX Cool Cooling Coil Speed 1 Performance2,  !- Name",
        "    0.5,                     !- Gross Total Cooling Capacity Fraction",
        "    0.5,                     !- Evaporator Air Flow Rate Fraction",
        "    0.5,                     !- Condenser Air Flow Rate Fraction",
        "    0.67,                    !- Gross Sensible Heat Ratio",
        "    4.17,                    !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.5,                     !- Evaporative Condenser Pump Power Fraction",
        "    0,                       !- Evaporative Condenser Effectiveness {dimensionless}",
        "    1Cap,  !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    CAPFF,  !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    1Pow,  !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    EIRFF,  !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    PLFCurve,  !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,  !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;  !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    DX Cool Cooling Coil Speed 2 Performance2,  !- Name",
        "    1.0,                     !- Gross Total Cooling Capacity Fraction",
        "    1.0,                     !- Evaporator Air Flow Rate Fraction",
        "    1.0,                     !- Condenser Air Flow Rate Fraction",
        "    0.67,                    !- Gross Sensible Heat Ratio",
        "    4.17,                    !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    1.0,                     !- Evaporative Condenser Pump Power Fraction",
        "    0,                       !- Evaporative Condenser Effectiveness {dimensionless}",
        "    1Cap,  !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    CAPFF,  !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    1Pow,  !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    EIRFF,  !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    PLFCurve,  !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,  !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;  !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "Curve:Quadratic, PLFCurve, 0.85, 0.83, 0.0, 0.0, 0.3, 0.85, 1.0, Dimensionless, Dimensionless; ",
        "Curve:Cubic, CAPFF, 1, 0, 0, 0, 0, 1, , , Dimensionless, Dimensionless; ",
        "Curve:Cubic, EIRFF, 1, 0, 0, 0, 0, 1, , , Dimensionless, Dimensionless; ",
        "Curve:Biquadratic, 1Cap, 0.483, 0.0305, 0.0000458, 0.00511, -1.50E-04, -1.28E-04, 8.89, 21.67, 12.78, 51.67, , , Temperature, "
        "Temperature,Dimensionless;",
        "Curve:Biquadratic, 1Pow, 1.33, -0.034, 0.00094, -0.0086, 0.00077, -0.000972, 8.89, 21.7, 12.8, 51.7, , , Temperature, Temperature, "
        "Dimensionless;",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    int coilIndex = CoilCoolingDX::factory(*state, "DX Cooling Coil");
    auto &thisCoil(state->dataCoilCoolingDX->coilCoolingDXs[coilIndex]);
    // size it
    thisCoil.size(*state);
    auto performance{dynamic_cast<CoilCoolingDXCurveFitPerformance *>(thisCoil.performance.get())};

    ASSERT_EQ("DX COOLING COIL", thisCoil.name);
    ASSERT_EQ("DX COOL COOLING COIL PERFORMANCE", performance->name);
    ASSERT_EQ("DX COOL COOLING COIL OPERATING MODE", performance->normalMode.name);
    ASSERT_EQ("DX COOL COOLING COIL OPERATING MODE2", performance->alternateMode.name);
    int nsp = (int)performance->normalMode.speeds.size();
    ASSERT_EQ(2, nsp);
    auto speed1 = performance->normalMode.speeds[0];
    ASSERT_EQ("DX COOL COOLING COIL SPEED 1 PERFORMANCE", speed1.name);
    auto speed2 = performance->normalMode.speeds[1];
    ASSERT_EQ("DX COOL COOLING COIL SPEED 2 PERFORMANCE", speed2.name);

    // auto hasAlternateMode = performance->hasAlternateMode;
    auto normalMode = performance->normalMode.speeds;
    auto alternateMode1 = performance->alternateMode;
    // EXPECT_EQ(1, hasAlternateMode);
    EXPECT_TRUE(2 == normalMode.size());
    auto speedA1 = alternateMode1.speeds[0];
    ASSERT_EQ("DX COOL COOLING COIL SPEED 1 PERFORMANCE2", speedA1.name);
    auto speedA2 = alternateMode1.speeds[1];
    ASSERT_EQ("DX COOL COOLING COIL SPEED 2 PERFORMANCE2", speedA2.name);

    auto pLFfPLR_Curve = speed1.indexPLRFPLF;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.83, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(0.3, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(0.3, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(30000, performance->normalMode.ratedGrossTotalCap, 0.01);

    // Reated Air Vol Flow Rate | evap air flow rate and condenser air flow rate ??
    EXPECT_NEAR(1.60, performance->normalMode.ratedEvapAirFlowRate, 0.01);

    EXPECT_NEAR(15000, speed1.rated_total_capacity, 0.01);
    EXPECT_NEAR(30000, speed2.rated_total_capacity, 0.01);

    EXPECT_NEAR(0.80, speed1.evap_air_flow_rate, 0.01);
    EXPECT_NEAR(1.60, speed2.evap_air_flow_rate, 0.01);

    // Rated COP
    EXPECT_EQ(4.17, speed1.ratedCOP);
    EXPECT_EQ(4.17, speed2.ratedCOP);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(453.3, speed1.rated_evap_fan_power_per_volume_flow_rate_2023, 0.01);
    EXPECT_NEAR(631.3, speed2.rated_evap_fan_power_per_volume_flow_rate_2023, 0.01);

    // CondenserType is a different enum that is being used in case of CurveFit in comparison to other Cooling DX Coils
    EXPECT_TRUE(CoilCoolingDXCurveFitOperatingMode::CondenserType::AIRCOOLED == performance->normalMode.condenserType);
    EXPECT_FALSE(CoilCoolingDXCurveFitOperatingMode::CondenserType::EVAPCOOLED == performance->normalMode.condenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(speed1.indexCapFT));
    EXPECT_EQ(0.483, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.0305, thisCCpaFTempHS->coeff[1]);

    // EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(speed1.indexEIRFT));
    EXPECT_EQ(1.33, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(-0.034, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(speed1.indexCapFFF));
    EXPECT_EQ(1, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(0, thisCapFFlowHs->coeff[1]);

    // EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(speed1.indexEIRFFF));
    EXPECT_EQ(1, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(0, thisEIRFFlowHs->coeff[1]);

    ASSERT_TRUE(performance->standardRatingEER == 0);
    ASSERT_TRUE(performance->standardRatingSEER == 0);
    ASSERT_TRUE(performance->standardRatingCoolingCapacity == 0);

    ASSERT_TRUE(performance->standardRatingSEER2_User == 0);
    ASSERT_TRUE(performance->standardRatingSEER2_Standard == 0);
    ASSERT_TRUE(performance->standardRatingCoolingCapacity2023 == 0);

    ASSERT_TRUE(performance->standardRatingIEER == 0);
    ASSERT_TRUE(performance->standardRatingIEER2 == 0);

    performance->calcStandardRatings210240(*state);

    ASSERT_TRUE(performance->standardRatingEER2 > 0);
    ASSERT_TRUE(performance->standardRatingIEER > 0);
    ASSERT_TRUE(performance->standardRatingIEER2 > 0);
    ASSERT_TRUE(performance->standardRatingCoolingCapacity2023 > 0);
    EXPECT_NEAR(3.51, performance->standardRatingEER2, 0.01);
    EXPECT_NEAR(3.97, performance->standardRatingIEER2, 0.01);
    EXPECT_NEAR(29027.03, performance->standardRatingCoolingCapacity2023, 0.01);
    EXPECT_NEAR(13.56, performance->standardRatingIEER2 * StandardRatings::ConvFromSIToIP, 0.01);

    ASSERT_TRUE(performance->standardRatingEER > 0);
    EXPECT_NEAR(3.39, performance->standardRatingEER, 0.01);
    ASSERT_TRUE(performance->standardRatingSEER > 0);
    EXPECT_NEAR(3.93, performance->standardRatingSEER, 0.01);

    ASSERT_TRUE(performance->standardRatingSEER2_User > 0);
    EXPECT_NEAR(4.18, performance->standardRatingSEER2_User, 0.01);
    ASSERT_TRUE(performance->standardRatingSEER2_Standard > 0);
    EXPECT_NEAR(4.12, performance->standardRatingSEER2_Standard, 0.01);
}

TEST_F(EnergyPlusFixture, CurveFit_03_Speed_20000W_IEER_2022_ValueTest)
{
    std::string idf_objects = delimited_string({
        "  Coil:Cooling:DX,",
        "    Sys 2 Furnace DX Cool Cooling Coil,  !- Name",
        "    Sys 2 Furnace DX Cool Mixed Air Outlet,  !- Evaporator Inlet Node Name",
        "    Sys 2 Furnace DX Cool Cooling Coil Outlet,  !- Evaporator Outlet Node Name",
        "    ,                        !- Availability Schedule Name",
        "    ,                        !- Condenser Zone Name",
        "    Sys 2 Furnace DX Cool Cooling Coil Condenser Inlet,  !- Condenser Inlet Node Name",
        "    Sys 2 Furnace DX Cool Cooling Coil Condenser Outlet Node,  !- Condenser Outlet Node Name",
        "    Sys 2 Furnace DX Cool Cooling Coil Performance,  !- Performance Object Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ;                        !- Evaporative Condenser Supply Water Storage Tank Name",

        "  Coil:Cooling:DX:CurveFit:Performance,",
        "    Sys 2 Furnace DX Cool Cooling Coil Performance,  !- Name",
        "    0.0,                     !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Unit Internal Static Air Pressure {Pa}",
        "    Discrete,                !- Capacity Control Method",
        "    ,                        !- Evaporative Condenser Basin Heater Capacity {W/K}",
        "    ,                        !- Evaporative Condenser Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Evaporative Condenser Basin Heater Operating Schedule Name",
        "    Electricity,             !- Compressor Fuel Type",
        "    Sys 2 Furnace DX Cool Cooling Coil Operating Mode;  !- Base Operating Mode",

        "	 Coil:Cooling:DX:CurveFit:OperatingMode,",
        "    Sys 2 Furnace DX Cool Cooling Coil Operating Mode,  !- Name",
        "    20000,                !- Rated Gross Total Cooling Capacity {W}", // 5000
        "    1.0,                !- Rated Evaporator Air Flow Rate {m3/s}",    // 0.25
        "    ,                        !- Rated Condenser Air Flow Rate {m3/s}",
        "    0,                       !- Maximum Cycling Rate {cycles/hr}",
        "    0,                       !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    0,                       !- Latent Capacity Time Constant {s}",
        "    0,                       !- Nominal Time for Condensate Removal to Begin {s}",
        "    No,                      !- Apply Latent Degradation to Speeds Greater than 1",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Nominal Evaporative Condenser Pump Power {W}",
        "    3,                       !- Nominal Speed Number",
        "    Sys 2 Furnace DX Cool Cooling Coil Speed 1 Performance,  !- Speed 1 Name",
        "    Sys 2 Furnace DX Cool Cooling Coil Speed 2 Performance,  !- Speed 2 Name",
        "    Sys 2 Furnace DX Cool Cooling Coil Speed 3 Performance;  !- Speed 3 Name",

        "	Coil:Cooling:DX:CurveFit:Speed,",
        "    Sys 2 Furnace DX Cool Cooling Coil Speed 1 Performance,  !- Name",
        "    0.3333,                  !- Gross Total Cooling Capacity Fraction",
        "    0.3333,                  !- Evaporator Air Flow Rate Fraction",
        "    ,                        !- Condenser Air Flow Rate Fraction",
        "    autosize,                !- Gross Sensible Heat Ratio",
        "    3,                       !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    377.8,                   !- Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    453.3,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    ,                        !- Evaporative Condenser Pump Power Fraction",
        "    ,                        !- Evaporative Condenser Effectiveness {dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FT,  !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FF,  !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FT,  !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FF,  !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil PLF,  !- Part Load Fraction Correlation Curve Name",
        "    0.2,                     !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil WH-FT,  !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,                        !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;                        !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    Sys 2 Furnace DX Cool Cooling Coil Speed 2 Performance,  !- Name",
        "    0.6667,                  !- Gross Total Cooling Capacity Fraction",
        "    0.6667,                  !- Evaporator Air Flow Rate Fraction",
        "    ,                        !- Condenser Air Flow Rate Fraction",
        "    autosize,                !- Gross Sensible Heat Ratio",
        "    3,                       !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    511.0,                        !- Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    631.3,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    ,                        !- Evaporative Condenser Pump Power Fraction",
        "    ,                        !- Evaporative Condenser Effectiveness {dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FT,  !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FF,  !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FT,  !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FF,  !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil PLF,  !- Part Load Fraction Correlation Curve Name",
        "    0.2,                     !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil WH-FT,  !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,                        !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;                        !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    Sys 2 Furnace DX Cool Cooling Coil Speed 3 Performance,  !- Name",
        "    1.0000,                  !- Gross Total Cooling Capacity Fraction",
        "    1.0000,                  !- Evaporator Air Flow Rate Fraction",
        "    ,                        !- Condenser Air Flow Rate Fraction",
        "    autosize,                !- Gross Sensible Heat Ratio",
        "    3,                       !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    677.4,                        !- Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    812.9,                   !- 2023 Rated Evaporator Fan Power Per Volume Flow {W/(m3/s)}",
        "    ,                        !- Evaporative Condenser Pump Power Fraction",
        "    ,                        !- Evaporative Condenser Effectiveness {dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FT,  !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FF,  !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FT,  !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FF,  !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil PLF,  !- Part Load Fraction Correlation Curve Name",
        "    0.2,                     !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil WH-FT,  !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,                        !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;                        !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        //   "	! Curves from example file MultiSpeedHeatPump.idf, Sep 2013, same curves for all speeds."

        "  Curve:Biquadratic,",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FT,  !- Name",
        "    0.476428E+00,            !- Coefficient1 Constant",
        "    0.401147E-01,            !- Coefficient2 x",
        "    0.226411E-03,            !- Coefficient3 x**2",
        "    -0.827136E-03,           !- Coefficient4 y",
        "    -0.732240E-05,           !- Coefficient5 y**2",
        "    -0.446278E-03,           !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    50.0,                    !- Maximum Value of x",
        "    0.0,                     !- Minimum Value of y",
        "    50.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    5.0,                     !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	 Curve:Cubic,",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FF,  !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "	  Curve:Biquadratic,",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FT,  !- Name",
        "    0.632475E+00,            !- Coefficient1 Constant",
        "    -0.121321E-01,           !- Coefficient2 x",
        "    0.507773E-03,            !- Coefficient3 x**2",
        "    0.155377E-01,            !- Coefficient4 y",
        "    0.272840E-03,            !- Coefficient5 y**2",
        "    -0.679201E-03,           !- Coefficient6 x*y",
        "    0.0,                     !- Minimum Value of x",
        "    50.0,                    !- Maximum Value of x",
        "    0.0,                     !- Minimum Value of y",
        "    50.0,                    !- Maximum Value of y",
        "    0.0,                     !- Minimum Curve Output",
        "    5.0,                     !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "	 Curve:Cubic,",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FF,  !- Name",
        "    .47278589,               !- Coefficient1 Constant",
        "    1.2433415,               !- Coefficient2 x",
        "    -1.0387055,              !- Coefficient3 x**2",
        "    .32257813,               !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        //    "	! PLF = l.- Cd(1.-PLR) where Cd = 0.15"

        "  Curve:Quadratic,",
        "    Sys 2 Furnace DX Cool Cool Coil PLF,  !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0,                       !- Minimum Value of x",
        "    1;                       !- Maximum Value of x",

        "	  Curve:Biquadratic,",
        "    Sys 2 Furnace DX Cool Cool Coil WH-FT,  !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 y",
        "    0.0,                     !- Coefficient5 y**2",
        "    0.0,                     !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    50,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    int coilIndex = CoilCoolingDX::factory(*state, "Sys 2 Furnace DX Cool Cooling Coil");
    auto &thisCoil(state->dataCoilCoolingDX->coilCoolingDXs[coilIndex]);
    auto performance{dynamic_cast<CoilCoolingDXCurveFitPerformance *>(thisCoil.performance.get())};

    // size it
    thisCoil.size(*state);

    ASSERT_EQ("SYS 2 FURNACE DX COOL COOLING COIL", thisCoil.name);
    ASSERT_EQ("SYS 2 FURNACE DX COOL COOLING COIL PERFORMANCE", performance->name);
    ASSERT_EQ("SYS 2 FURNACE DX COOL COOLING COIL OPERATING MODE", performance->normalMode.name);
    int nsp = (int)performance->normalMode.speeds.size();
    ASSERT_EQ(3, nsp);
    auto speed1 = performance->normalMode.speeds[0];
    ASSERT_EQ("SYS 2 FURNACE DX COOL COOLING COIL SPEED 1 PERFORMANCE", speed1.name);
    auto speed2 = performance->normalMode.speeds[1];
    ASSERT_EQ("SYS 2 FURNACE DX COOL COOLING COIL SPEED 2 PERFORMANCE", speed2.name);
    auto speed3 = performance->normalMode.speeds[2];
    ASSERT_EQ("SYS 2 FURNACE DX COOL COOLING COIL SPEED 3 PERFORMANCE", speed3.name);

    // auto hasAlternateMode = performance->hasAlternateMode;
    auto alternateMode1 = performance->alternateMode.speeds;
    auto alternateMode2 = performance->alternateMode2.speeds;
    // EXPECT_EQ(0, hasAlternateMode);
    EXPECT_TRUE(alternateMode1.empty());
    EXPECT_TRUE(alternateMode2.empty());
    auto pLFfPLR_Curve = speed1.indexPLRFPLF;
    auto &thisCoolPLFfPLR(state->dataCurveManager->curves(pLFfPLR_Curve));
    // ckeck user PLF curve coefficients
    EXPECT_EQ(0.85, thisCoolPLFfPLR->coeff[0]);
    EXPECT_EQ(0.15, thisCoolPLFfPLR->coeff[1]);
    EXPECT_EQ(0.0, thisCoolPLFfPLR->inputLimits[0].min);
    EXPECT_EQ(1.0, thisCoolPLFfPLR->inputLimits[0].max);

    Real64 minEIRfLowPLRXInput(0.0);
    Real64 maxEIRfLowPLRXInput(0.0);
    // check user PLF curve PLR limits
    Curve::GetCurveMinMaxValues(*state, pLFfPLR_Curve, minEIRfLowPLRXInput, maxEIRfLowPLRXInput);
    EXPECT_EQ(0.0, minEIRfLowPLRXInput);
    EXPECT_EQ(1.0, maxEIRfLowPLRXInput);

    // Rated Total Capacity
    EXPECT_NEAR(20000, performance->normalMode.ratedGrossTotalCap, 0.01);

    // Reated Air Vol Flow Rate | evap air flow rate and condenser air flow rate ??
    EXPECT_NEAR(1.0, performance->normalMode.ratedEvapAirFlowRate, 0.01);

    EXPECT_NEAR(6666, speed1.rated_total_capacity, 0.01);
    EXPECT_NEAR(13334, speed2.rated_total_capacity, 0.01);
    EXPECT_NEAR(20000.0, speed3.rated_total_capacity, 0.01);

    EXPECT_NEAR(0.33329999999999999, speed1.evap_air_flow_rate, 0.01);
    EXPECT_NEAR(0.66669999999999996, speed2.evap_air_flow_rate, 0.01);
    EXPECT_NEAR(1.0, speed3.evap_air_flow_rate, 0.01);

    // Rated COP
    EXPECT_EQ(3, speed1.ratedCOP);
    EXPECT_EQ(3, speed2.ratedCOP);
    EXPECT_EQ(3, speed3.ratedCOP);

    // Rated Fan POwer Per Evap Air Flow Rate 2023
    EXPECT_NEAR(453.3, speed1.rated_evap_fan_power_per_volume_flow_rate_2023, 0.01);
    EXPECT_NEAR(631.3, speed2.rated_evap_fan_power_per_volume_flow_rate_2023, 0.01);
    EXPECT_NEAR(812.9, speed3.rated_evap_fan_power_per_volume_flow_rate_2023, 0.01);

    // CondenserType is a different enum that is being used in case of CurveFit in comparison to other Cooling DX Coils
    EXPECT_TRUE(CoilCoolingDXCurveFitOperatingMode::CondenserType::AIRCOOLED == performance->normalMode.condenserType);
    EXPECT_FALSE(CoilCoolingDXCurveFitOperatingMode::CondenserType::EVAPCOOLED == performance->normalMode.condenserType);

    // Check user curve coefficients

    // CCapFTemp Speed 1
    auto &thisCCpaFTempHS(state->dataCurveManager->curves(speed1.indexCapFT));
    EXPECT_EQ(0.476428E+00, thisCCpaFTempHS->coeff[0]);
    EXPECT_EQ(0.401147E-01, thisCCpaFTempHS->coeff[1]);

    // EIRFTemp Speed 1
    auto &thisEIRFTempHS(state->dataCurveManager->curves(speed1.indexEIRFT));
    EXPECT_EQ(0.632475E+00, thisEIRFTempHS->coeff[0]);
    EXPECT_EQ(-0.121321E-01, thisEIRFTempHS->coeff[1]);

    // CapFFlow Speed 1
    auto &thisCapFFlowHs(state->dataCurveManager->curves(speed1.indexCapFFF));
    EXPECT_EQ(.47278589, thisCapFFlowHs->coeff[0]);
    EXPECT_EQ(1.2433415, thisCapFFlowHs->coeff[1]);

    // EIRFFlow Speed 1
    auto &thisEIRFFlowHs(state->dataCurveManager->curves(speed1.indexEIRFFF));
    EXPECT_EQ(.47278589, thisEIRFFlowHs->coeff[0]);
    EXPECT_EQ(1.2433415, thisEIRFFlowHs->coeff[1]);

    ASSERT_TRUE(performance->standardRatingEER == 0);
    ASSERT_TRUE(performance->standardRatingSEER == 0);
    ASSERT_TRUE(performance->standardRatingCoolingCapacity == 0);

    ASSERT_TRUE(performance->standardRatingSEER2_User == 0);
    ASSERT_TRUE(performance->standardRatingSEER2_Standard == 0);
    ASSERT_TRUE(performance->standardRatingCoolingCapacity2023 == 0);

    ASSERT_TRUE(performance->standardRatingIEER == 0);
    ASSERT_TRUE(performance->standardRatingIEER2 == 0);

    performance->calcStandardRatings210240(*state);

    ASSERT_TRUE(performance->standardRatingEER2 > 0);
    ASSERT_TRUE(performance->standardRatingIEER > 0);
    ASSERT_TRUE(performance->standardRatingIEER2 > 0);
    ASSERT_TRUE(performance->standardRatingCoolingCapacity2023 > 0);
    EXPECT_NEAR(2.55, performance->standardRatingEER2, 0.01);
    EXPECT_NEAR(3.18, performance->standardRatingIEER2, 0.01);
    EXPECT_NEAR(19192.186657893722, performance->standardRatingCoolingCapacity2023, 0.01);
    EXPECT_NEAR(10.85, performance->standardRatingIEER2 * StandardRatings::ConvFromSIToIP, 0.01);

    ASSERT_TRUE(performance->standardRatingEER > 0);
    EXPECT_NEAR(2.62, performance->standardRatingEER, 0.01);
    ASSERT_TRUE(performance->standardRatingSEER > 0);
    EXPECT_NEAR(2.80, performance->standardRatingSEER, 0.01);
    ASSERT_TRUE(performance->standardRatingSEER2_User > 0);
    EXPECT_NEAR(3.05, performance->standardRatingSEER2_User, 0.01);
    ASSERT_TRUE(performance->standardRatingSEER2_Standard > 0);
    EXPECT_NEAR(3.07, performance->standardRatingSEER2_Standard, 0.01);
}

TEST_F(EnergyPlusFixture, ChillerCondenserEnteringFluidTemp_AHRIIPTestConditions)
{
    // Tests chiller's condenser entering fluid temperature
    // at different AHRI Std 550/590 (IP) test conditions

    // set AHRI test load ratios
    Real64 constexpr LoadRatio100 = 1.0;
    Real64 constexpr LoadRatio75 = 0.75;
    Real64 constexpr LoadRatio50 = 0.50;
    Real64 constexpr LoadRatio25 = 0.25;

    // Test 1: WaterCooled Condenser Entering Water Temperatures
    DataPlant::CondenserType CondenserType = DataPlant::CondenserType::WaterCooled;
    StandardRatings::AhriChillerStd Ahri_Chiller_Std = StandardRatings::AhriChillerStd::AHRI550_590;
    Real64 result_EWT100LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio100);
    Real64 result_EWT75LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio75);
    Real64 result_EWT50LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio50);
    Real64 result_EWT25LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio25);
    // checking expected water temperatures
    EXPECT_NEAR(29.44, result_EWT100LoadRatio, 0.01);
    EXPECT_NEAR(23.89, result_EWT75LoadRatio, 0.01);
    EXPECT_NEAR(18.33, result_EWT50LoadRatio, 0.01);
    EXPECT_NEAR(18.33, result_EWT25LoadRatio, 0.01);

    // Test 2: AirCooled Condenser Entering Air Dry-Bulb Temperatures
    CondenserType = DataPlant::CondenserType::AirCooled;
    Real64 result_EDBT100LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio100);
    Real64 result_EDBT75LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio75);
    Real64 result_EDBT50LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio50);
    Real64 result_EDBT25LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio25);
    // checking expected dry-bulb temperatures
    EXPECT_NEAR(35.0, result_EDBT100LoadRatio, 0.01);
    EXPECT_NEAR(26.67, result_EDBT75LoadRatio, 0.01);
    EXPECT_NEAR(18.33, result_EDBT50LoadRatio, 0.01);
    EXPECT_NEAR(12.78, result_EDBT25LoadRatio, 0.01);

    // Test 3: EvapCooled Condenser Entering Air Wet-Bulb Temperatures
    CondenserType = DataPlant::CondenserType::EvapCooled;
    Real64 result_EWBT100LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio100);
    Real64 result_EWBT75LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio75);
    Real64 result_EWBT50LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio50);
    Real64 result_EWBT25LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio25);
    // checking expected wet-bulb temperatures
    EXPECT_NEAR(23.89, result_EWBT100LoadRatio, 0.01);
    EXPECT_NEAR(20.42, result_EWBT75LoadRatio, 0.01);
    EXPECT_NEAR(16.94, result_EWBT50LoadRatio, 0.01);
    EXPECT_NEAR(13.47, result_EWBT25LoadRatio, 0.01);
}

TEST_F(EnergyPlusFixture, ChillerCondenserEnteringFluidTemp_AHRISITestConditions)
{
    // Tests chiller's condenser entering fluid temperature
    // at different AHRI Std 551/591 (SI) test conditions

    // set AHRI test load ratios
    Real64 constexpr LoadRatio100 = 1.0;
    Real64 constexpr LoadRatio75 = 0.75;
    Real64 constexpr LoadRatio50 = 0.50;
    Real64 constexpr LoadRatio25 = 0.25;

    // Test 1: WaterCooled Condenser Entering Water Temperatures
    DataPlant::CondenserType CondenserType = DataPlant::CondenserType::WaterCooled;
    StandardRatings::AhriChillerStd Ahri_Chiller_Std = StandardRatings::AhriChillerStd::AHRI551_591;
    Real64 result_EWT100LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio100);
    Real64 result_EWT75LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio75);
    Real64 result_EWT50LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio50);
    Real64 result_EWT25LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio25);
    // checking expected water temperatures
    EXPECT_NEAR(30.0, result_EWT100LoadRatio, 0.01);
    EXPECT_NEAR(24.5, result_EWT75LoadRatio, 0.01);
    EXPECT_NEAR(19.0, result_EWT50LoadRatio, 0.01);
    EXPECT_NEAR(19.0, result_EWT25LoadRatio, 0.01);

    // Test 2: AirCooled Condenser Entering Air Dry-Bulb Temperatures
    CondenserType = DataPlant::CondenserType::AirCooled;
    Real64 result_EDBT100LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio100);
    Real64 result_EDBT75LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio75);
    Real64 result_EDBT50LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio50);
    Real64 result_EDBT25LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio25);
    // checking expected dry-bulb temperatures
    EXPECT_NEAR(35.0, result_EDBT100LoadRatio, 0.01);
    EXPECT_NEAR(27.0, result_EDBT75LoadRatio, 0.01);
    EXPECT_NEAR(19.0, result_EDBT50LoadRatio, 0.01);
    EXPECT_NEAR(13.0, result_EDBT25LoadRatio, 0.01);

    // Test 3: EvapCooled Condenser Entering Air Wet-Bulb Temperatures
    CondenserType = DataPlant::CondenserType::EvapCooled;
    Real64 result_EWBT100LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio100);
    Real64 result_EWBT75LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio75);
    Real64 result_EWBT50LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio50);
    Real64 result_EWBT25LoadRatio = CondenserEnteringFluidTemperature(CondenserType, Ahri_Chiller_Std, LoadRatio25);
    // checking expected wet-bulb temperatures
    EXPECT_NEAR(24.0, result_EWBT100LoadRatio, 0.01);
    EXPECT_NEAR(20.5, result_EWBT75LoadRatio, 0.01);
    EXPECT_NEAR(17.0, result_EWBT50LoadRatio, 0.01);
    EXPECT_NEAR(13.5, result_EWBT25LoadRatio, 0.01);
}

} // namespace EnergyPlus
