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

// EnergyPlus::HVACDXSystem and VariableSpeedCoils Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/HVACDXHeatPumpSystem.hh>
#include <EnergyPlus/OutputReportPredefined.hh>

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, ExerciseHVACDXHeatPumpSystem)
{
    // issue #6040
    std::string const idf_objects =
        delimited_string({"CoilSystem:Heating:DX,",
                          "    HeatPump DX Coil 1,      !- Name",
                          "    ,                        !- Availability Schedule Name",
                          "    Coil:Heating:DX:SingleSpeed,  !- Heating Coil Object Type",
                          "    Heat Pump DX Heating Coil 1;  !- Heating Coil Name",
                          "Coil:Heating:DX:SingleSpeed,",
                          "    Heat Pump DX Heating Coil 1,  !- Name",
                          "    FanAndCoilAvailSched,    !- Availability Schedule Name",
                          "    autosize,                !- Gross Rated Heating Capacity {W}",
                          "    2.75,                    !- Gross Rated Heating COP {W/W}",
                          "    autosize,                !- Rated Air Flow Rate {m3/s}",
                          "    ,                        !- 2017 Rated Supply Fan Power Per Volume Flow Rate {W/(m3/s)}",
                          "    ,                        !- 2023 Rated Supply Fan Power Per Volume Flow Rate {W/(m3/s)}",
                          "    Heating Coil Air Inlet Node,  !- Air Inlet Node Name",
                          "    SuppHeating Coil Air Inlet Node,  !- Air Outlet Node Name",
                          "    HPACHeatCapFT,           !- Heating Capacity Function of Temperature Curve Name",
                          "    HPACHeatCapFFF,          !- Heating Capacity Function of Flow Fraction Curve Name",
                          "    HPACHeatEIRFT,           !- Energy Input Ratio Function of Temperature Curve Name",
                          "    HPACHeatEIRFFF,          !- Energy Input Ratio Function of Flow Fraction Curve Name",
                          "    HPACCOOLPLFFPLR,         !- Part Load Fraction Correlation Curve Name",
                          "    ,                        !- Defrost Energy Input Ratio Function of Temperature Curve Name",
                          "    -8.0,                    !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
                          "    ,                        !- Outdoor Dry-Bulb Temperature to Turn On Compressor {C}",
                          "    5.0,                     !- Maximum Outdoor Dry-Bulb Temperature for Defrost Operation {C}",
                          "    200.0,                   !- Crankcase Heater Capacity {W}",
                          "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
                          "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
                          "    Resistive,               !- Defrost Strategy",
                          "    TIMED,                   !- Defrost Control",
                          "    0.166667,                !- Defrost Time Period Fraction",
                          "    autosize,                !- Resistive Defrost Heater Capacity {W}",
                          "    ,                        !- Region number for calculating HSPF",
                          "    Heat Pump 1 Evaporator Node;  !- Evaporator Air Inlet Node Name"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataLoopNodes->NodeID.allocate(2);
    state->dataLoopNodes->Node.allocate(2);

    // manually add a dx coil
    state->dataDXCoils->NumDXCoils = 1;
    state->dataDXCoils->GetCoilsInputFlag = false;
    state->dataDXCoils->DXCoil.allocate(1);
    state->dataDXCoils->DXCoil(1).Name = "HEAT PUMP DX HEATING COIL 1";
    state->dataDXCoils->DXCoil(1).availSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataDXCoils->DXCoil(1).AirInNode = 1;
    state->dataDXCoils->DXCoil(1).AirOutNode = 2;
    state->dataDXCoils->DXCoil(1).DXCoilType = "COIL:HEATING:DX:SINGLESPEED";
    state->dataDXCoils->DXCoil(1).RatedTotCap(1) = 1;
    state->dataDXCoils->DXCoil(1).RatedCOP(1) = 1;
    state->dataDXCoils->DXCoil(1).CCapFFlow(1) = 1;
    state->dataDXCoils->DXCoil(1).CCapFTemp(1) = 1;
    state->dataDXCoils->DXCoil(1).EIRFFlow(1) = 1;
    state->dataDXCoils->DXCoil(1).EIRFTemp(1) = 1;
    state->dataDXCoils->DXCoil(1).PLFFPLR(1) = 1;
    state->dataDXCoils->DXCoil(1).RatedAirVolFlowRate(1) = 1.0;
    state->dataDXCoils->DXCoil(1).FanPowerPerEvapAirFlowRate(1) = 0.0;
    state->dataDXCoils->DXCoil(1).FanPowerPerEvapAirFlowRate_2023(1) = 0.0;
    state->dataDXCoils->DXCoil(1).RegionNum = 1;
    state->dataDXCoils->DXCoilOutletTemp.allocate(1);
    state->dataDXCoils->DXCoilOutletHumRat.allocate(1);
    state->dataDXCoils->DXCoilFanOp.allocate(1);
    state->dataDXCoils->DXCoilPartLoadRatio.allocate(1);
    state->dataDXCoils->DXCoilTotalHeating.allocate(1);
    state->dataDXCoils->DXCoilHeatInletAirDBTemp.allocate(1);
    state->dataDXCoils->DXCoilHeatInletAirWBTemp.allocate(1);
    state->dataDXCoils->DXCoilNumericFields.allocate(1);
    state->dataDXCoils->DXCoilNumericFields(1).PerfMode.allocate(1);
    state->dataDXCoils->DXCoilNumericFields(1).PerfMode(1).FieldNames.allocate(4);
    state->dataDXCoils->DXCoil(1).DXCoilType_Num = HVAC::CoilDX_HeatingEmpirical;

    // manually add a curve
    [[maybe_unused]] auto *curve = Curve::AddCurve(*state, "Curve1");

    // setup some outputs
    OutputReportPredefined::SetPredefinedTables(*state);

    int compIndex = 0;
    HVACDXHeatPumpSystem::SimDXHeatPumpSystem(*state, "HEATPUMP DX COIL 1", true, -1, compIndex, -1, 0.0);
}

} // namespace EnergyPlus
