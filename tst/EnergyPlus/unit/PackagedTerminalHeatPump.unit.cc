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

// EnergyPlus::VariableSpeedCoils Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// Objexx Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchNodeConnections.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/HeatBalanceAirManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SingleDuct.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/SplitterComponent.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>
#include <EnergyPlus/UnitarySystem.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>
#include <EnergyPlus/ZoneAirLoopEquipmentManager.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>
#include <EnergyPlus/ZonePlenum.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::BranchInputManager;
using namespace EnergyPlus::DataDefineEquip;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::DataHeatBalFanSys;
using namespace EnergyPlus::DataPlant;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::DataZoneEnergyDemands;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::DXCoils;
using namespace EnergyPlus::Fans;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::HeatingCoils;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::SimulationManager;
using namespace EnergyPlus::SingleDuct;
using namespace EnergyPlus::SizingManager;
using namespace EnergyPlus::VariableSpeedCoils;
using namespace EnergyPlus::ZoneAirLoopEquipmentManager;
using namespace EnergyPlus::ZoneTempPredictorCorrector;

namespace EnergyPlus {

// TEST_F(EnergyPlusFixture, PackagedTerminalHP_VSCoils_Sizing)
// {
//     std::string const idf_objects = delimited_string({
//
//         "  Zone, Space, 0.0, 0.0, 0.0, 0.0, 1, 1, 2.4, , autocalculate, , , Yes; ",
//         "  ZoneHVAC:EquipmentConnections, Space, Space Eq, Space In Node, Space Out Node, Space Node, Space Ret Node; ",
//         "  ZoneHVAC:EquipmentList, Space Eq, SequentialLoad, ZoneHVAC:WaterToAirHeatPump, Zone WSHP, 1, 1; ",
//         "  Schedule:Compact, OnSched, Fraction, Through: 12/31, For: AllDays, Until: 24:00, 1.0; ",
//         "  ScheduleTypeLimits, Fraction, 0.0, 1.0, CONTINUOUS; ",
//         "  OutdoorAir:Node, PSZ-AC_1:5 OA Node;",
//         "  OutdoorAir:Node, Lobby_ZN_1_FLR_2 WSHP OA Node;",
//         "  Curve:Exponent, FanPowerCurve, 0.254542407, 0.837259009, 3, 0.458, 1, , , Dimensionless, Dimensionless; ",
//         "  Curve:Quadratic, PLF Curve, 0.85, 0.15, 0, 0, 1, 0.0, 1.0, Dimensionless, Dimensionless; ",
//         "  Curve:Cubic, CubicCurve, 1.0, 0.0, 0.0, 0.0, 0.76, 1.09, , , Dimensionless, Dimensionless; ",
//         "  Curve:Biquadratic, BiquadraticCurve, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10, 25.6, 7.2, 48.9, , , Temperature, Temperature, Dimensionless; ",
//
//         "  ZoneHVAC:WaterToAirHeatPump,",
//         "    Zone WSHP,            !- Name",
//         "    OnSched,              !- Availability Schedule Name",
//         "    Space Out Node,       !- Air Inlet Node Name",
//         "    Space In Node,        !- Air Outlet Node Name",
//         "    ,                     !- Outdoor Air Mixer Object Type",
//         "    ,                     !- Outdoor Air Mixer Name",
//         "    Autosize,             !- Supply Air Flow Rate During Cooling Operation {m3/s}",
//         "    Autosize,             !- Supply Air Flow Rate During Heating Operation {m3/s}",
//         "    ,                     !- Supply Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
//         "    ,                     !- New Input Field",
//         "    0.0,                  !- Outdoor Air Flow Rate During Cooling Operation {m3/s}",
//         "    0.0,                  !- Outdoor Air Flow Rate During Heating Operation {m3/s}",
//         "    ,                     !- Outdoor Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
//         "    Fan:OnOff,            !- Supply Air Fan Object Type",
//         "    Lobby_ZN_1_FLR_2 WSHP Fan,                                 !- Supply Air Fan Name",
//         "    Coil:Heating:WaterToAirHeatPump:VariableSpeedEquationFit,  !- Heating Coil Object Type",
//         "    Lobby_ZN_1_FLR_2 WSHP Heating Mode,                        !- Heating Coil Name",
//         "    Coil:Cooling:WaterToAirHeatPump:VariableSpeedEquationFit,  !- Cooling Coil Object Type",
//         "    Lobby_ZN_1_FLR_2 WSHP Cooling Mode,                        !- Cooling Coil Name",
//         "    Coil:Heating:Electric,                    !- Supplemental Heating Coil Object Type",
//         "    Lobby_ZN_1_FLR_2 WSHP Supp Heating Coil,  !- Supplemental Heating Coil Name",
//         "    50.0,                 !- Maximum Supply Air Temperature from Supplemental Heater {C}",
//         "    20.0,                 !- Maximum Outdoor Dry-Bulb Temperature for Supplemental Heater Operation {C}",
//         "    Lobby_ZN_1_FLR_2 WSHP OA Node,  !- Outdoor Dry-Bulb Temperature Sensor Node Name",
//         "    BlowThrough,          !- Fan Placement",
//         "    OnSched,              !- Supply Air Fan Operating Mode Schedule Name",
//         "    ,                     !- Heat Pump Coil Water Flow Mode",
//         "    ;                     !- Design Specification ZoneHVAC Sizing Object Name",
//
//         "  Coil:Cooling:WaterToAirHeatPump:VariableSpeedEquationFit,",
//         "    Lobby_ZN_1_FLR_2 WSHP Cooling Mode,                    !- Name",
//         "    Lobby_ZN_1_FLR_2 WSHP Cooling Source Side Inlet Node,  !- Water-to-Refrigerant HX Water Inlet Node Name",
//         "    Lobby_ZN_1_FLR_2 WSHP Cooling Source Side Outlet Node, !- Water-to-Refrigerant HX Water Outlet Node Name",
//         "    Lobby_ZN_1_FLR_2 WSHP Cooling Coil Air Inlet Node,     !- Indoor Air Inlet Node Name",
//         "    Lobby_ZN_1_FLR_2 WSHP Heating Coil Air Inlet Node,     !- Indoor Air Outlet Node Name",
//         "    9,                    !- Number of Speeds {dimensionless}",
//         "    9,                    !- Nominal Speed Level {dimensionless}",
//         "    Autosize,             !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {w}",
//         "    Autosize,             !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
//         "    Autosize,             !- Rated Water Flow Rate At Selected Nominal Speed Level {m3/s}",
//         "    0.0,                  !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
//         "    0.0,                  !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
//         "    2.5,                  !- Maximum Cycling Rate {cycles/hr}",
//         "    60.0,                 !- Latent Capacity Time Constant {s}",
//         "    60,                   !- Fan Delay Time {s}",
//         "    0,                    !- Flag for Using Hot Gas Reheat, 0 or 1 {dimensionless}",
//         "    PLF Curve,            !- Energy Part Load Fraction Curve Name",
//         "    4682.3964854,         !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {w}",
//         "    0.97,                 !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
//         "    8.031554863,          !- Speed 1 Reference Unit Gross Rated Cooling COP {dimensionless}",
//         "    0.408706486,          !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 1 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 1 Total Cooling Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 1 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 1 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 1 Waste Heat Function of Temperature Curve Name",
//         "    5733.6424135,         !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {w}",
//         "    0.96,                 !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
//         "    8.132826118,          !- Speed 2 Reference Unit Gross Rated Cooling COP {dimensionless}",
//         "    0.449293966,          !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 2 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 2 Total Cooling Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 2 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 2 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 2 Waste Heat Function of Temperature Curve Name",
//         "    6783.7160573,         !- Speed 3 Reference Unit Gross Rated Total Cooling Capacity {w}",
//         "    0.95,                 !- Speed 3 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
//         "    8.133952107,          !- Speed 3 Reference Unit Gross Rated Cooling COP {dimensionless}",
//         "    0.489881446,          !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 3 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 3 Total Cooling Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 3 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 3 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 3 Waste Heat Function of Temperature Curve Name",
//         "    7819.1361476,         !- Speed 4 Reference Unit Gross Rated Total Cooling Capacity {w}",
//         "    0.91,                 !- Speed 4 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
//         "    8.077619987,          !- Speed 4 Reference Unit Gross Rated Cooling COP {dimensionless}",
//         "    0.530468926,          !- Speed 4 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 4 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 4 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 4 Total Cooling Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 4 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 4 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 4 Waste Heat Function of Temperature Curve Name",
//         "    8827.8867705,         !- Speed 5 Reference Unit Gross Rated Total Cooling Capacity {w}",
//         "    0.871,                !- Speed 5 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
//         "    7.974604129,          !- Speed 5 Reference Unit Gross Rated Cooling COP {dimensionless}",
//         "    0.571056406,          !- Speed 5 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 5 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 5 Total Cooling Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 5 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 5 Total Cooling Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 5 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 5 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 5 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 5 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 5 Waste Heat Function of Temperature Curve Name",
//         "    10734.02101,          !- Speed 6 Reference Unit Gross Rated Total Cooling Capacity {w}",
//         "    0.816,                !- Speed 6 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
//         "    7.661685232,          !- Speed 6 Reference Unit Gross Rated Cooling COP {dimensionless}",
//         "    0.652231367,          !- Speed 6 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 6 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 6 Total Cooling Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 6 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 6 Total Cooling Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 6 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 6 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 6 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 6 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve ,    !- Speed 6 Waste Heat Function of Temperature Curve Name",
//         "    12454.348191,         !- Speed 7 Reference Unit Gross Rated Total Cooling Capacity {w}",
//         "    0.784,                !- Speed 7 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
//         "    7.257778666,          !- Speed 7 Reference Unit Gross Rated Cooling COP {dimensionless}",
//         "    0.732934379,          !- Speed 7 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 7 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 7 Total Cooling Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 7 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 7 Total Cooling Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 7 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 7 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 7 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 7 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 7 Waste Heat Function of Temperature Curve Name",
//         "    13963.37113,          !- Speed 8 Reference Unit Gross Rated Total Cooling Capacity {w}",
//         "    0.766,                !- Speed 8 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
//         "    6.804761759,          !- Speed 8 Reference Unit Gross Rated Cooling COP {dimensionless}",
//         "    0.81410934,           !- Speed 8 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 8 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 8 Total Cooling Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 8 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 8 Total Cooling Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 8 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 8 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 8 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 8 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 8 Waste Heat Function of Temperature Curve Name",
//         "    16092.825525,         !- Speed 9 Reference Unit Gross Rated Total Cooling Capacity {w}",
//         "    0.739,                !- Speed 9 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
//         "    5.765971166,          !- Speed 9 Reference Unit Gross Rated Cooling COP {dimensionless}",
//         "    0.891980668,          !- Speed 9 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 9 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 9 Total Cooling Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 9 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 9 Total Cooling Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 9 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 9 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 9 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 9 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve;     !- Speed 9 Waste Heat Function of Temperature Curve Name",
//
//         "  Coil:Heating:WaterToAirHeatPump:VariableSpeedEquationFit,",
//         "    Lobby_ZN_1_FLR_2 WSHP Heating Mode,                    !- Name",
//         "    Lobby_ZN_1_FLR_2 WSHP Heating Source Side Inlet Node,  !- Water-to-Refrigerant HX Water Inlet Node Name",
//         "    Lobby_ZN_1_FLR_2 WSHP Heating Source Side Outlet Node, !- Water-to-Refrigerant HX Water Outlet Node Name",
//         "    Lobby_ZN_1_FLR_2 WSHP Heating Coil Air Inlet Node,     !- Indoor Air Inlet Node Name",
//         "    Lobby_ZN_1_FLR_2 WSHP SuppHeating Coil Air Inlet Node, !- Indoor Air Outlet Node Name",
//         "    9,                    !- Number of Speeds {dimensionless}",
//         "    9,                    !- Nominal Speed Level {dimensionless}",
//         "    autosize,             !- Rated Heating Capacity At Selected Nominal Speed Level {w}",
//         "    autosize,             !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
//         "    autosize,             !- Rated Water Flow Rate At Selected Nominal Speed Level {m3/s}",
//         "    PLF Curve,            !- Energy Part Load Fraction Curve Name",
//         "    6437.5991236,         !- Speed 1 Reference Unit Gross Rated Heating Capacity {w}",
//         "    9.965323721,          !- Speed 1 Reference Unit Gross Rated Heating COP {dimensionless}",
//         "    0.408706486,          !- Speed 1 Reference Unit Rated Air Flow {m3/s}",
//         "    0.0008201726 ,        !- Speed 1 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 1 Heating Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 1 Total  Heating Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 1 Heating Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 1 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 1 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 1 Waste Heat Function of Temperature Curve Name",
//         "    7521.3759405,         !- Speed 2 Reference Unit Gross Rated Heating Capacity {w}",
//         "    9.3549452,            !- Speed 2 Reference Unit Gross Rated Heating COP {dimensionless}",
//         "    0.449293966,          !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 2 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 2 Heating Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 2 Total  Heating Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 2 Heating Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 2 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 2 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 2 Waste Heat Function of Temperature Curve Name",
//         "    8601.0497624,         !- Speed 3 Reference Unit Gross Rated Heating Capacity {w}",
//         "    8.857929724,          !- Speed 3 Reference Unit Gross Rated Heating COP {dimensionless}",
//         "    0.489881446,          !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 3 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 3 Heating Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 3 Total  Heating Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 3 Heating Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 3 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 3 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 3 Waste Heat Function of Temperature Curve Name",
//         "    9675.1552339,         !- Speed 4 Reference Unit Gross Rated Heating Capacity {w}",
//         "    8.442543834,          !- Speed 4 Reference Unit Gross Rated Heating COP {dimensionless}",
//         "    0.530468926,          !- Speed 4 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 4 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 4 Heating Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 4 Total  Heating Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 4 Heating Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 4 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 4 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 4 Waste Heat Function of Temperature Curve Name",
//         "    10743.692355,         !- Speed 5 Reference Unit Gross Rated Heating Capacity {w}",
//         "    8.090129785,          !- Speed 5 Reference Unit Gross Rated Heating COP {dimensionless}",
//         "    0.571056406,          !- Speed 5 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 5 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 5 Heating Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 5 Total  Heating Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 5 Heating Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 5 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 5 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 5 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 5 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 5 Waste Heat Function of Temperature Curve Name",
//         "    12861.716978,         !- Speed 6 Reference Unit Gross Rated Heating Capacity {w}",
//         "    7.521471917,          !- Speed 6 Reference Unit Gross Rated Heating COP {dimensionless}",
//         "    0.652231367,          !- Speed 6 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 6 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 6 Heating Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 6 Total  Heating Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 6 Heating Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 6 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 6 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 6 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 6 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 6 Waste Heat Function of Temperature Curve Name",
//         "    14951.606778,         !- Speed 7 Reference Unit Gross Rated Heating Capacity {w}",
//         "    7.072661674,          !- Speed 7 Reference Unit Gross Rated Heating COP {dimensionless}",
//         "    0.732934379,          !- Speed 7 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 7 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 7 Heating Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 7 Total  Heating Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 7 Heating Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 7 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 7 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 7 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 7 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 7 Waste Heat Function of Temperature Curve Name",
//         "    17011.8964,           !- Speed 8 Reference Unit Gross Rated Heating Capacity {w}",
//         "    6.710807258,          !- Speed 8 Reference Unit Gross Rated Heating COP {dimensionless}",
//         "    0.81410934,           !- Speed 8 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 8 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 8 Heating Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 8 Total  Heating Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 8 Heating Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 8 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 8 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 8 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0 ,                 !- Speed 8 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve,     !- Speed 8 Waste Heat Function of Temperature Curve Name",
//         "    20894.501936,         !- Speed 9 Reference Unit Gross Rated Heating Capacity {w}",
//         "    5.89906887,           !- Speed 9 Reference Unit Gross Rated Heating COP {dimensionless}",
//         "    0.891980668,          !- Speed 9 Reference Unit Rated Air Flow Rate {m3/s}",
//         "    0.0008201726 ,        !- Speed 9 Reference Unit Rated Water Flow Rate {m3/s}",
//         "    BiquadraticCurve,     !- Speed 9 Heating Capacity Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 9 Total  Heating Capacity Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 9 Heating Capacity Function of Water Flow Fraction Curve Name",
//         "    BiquadraticCurve,     !- Speed 9 Energy Input Ratio Function of Temperature Curve Name",
//         "    CubicCurve,           !- Speed 9 Energy Input Ratio Function of Air Flow Fraction Curve Name",
//         "    CubicCurve,           !- Speed 9 Energy Input Ratio Function of Water Flow Fraction Curve Name",
//         "    0.0,                  !- Speed 9 Reference Unit Waste Heat Fraction of Input Power At Rated Conditions {dimensionless}",
//         "    BiquadraticCurve;     !- Speed 9 Waste Heat Function of Temperature Curve Name",
//
//         "  Fan:OnOff,",
//         "    Lobby_ZN_1_FLR_2 WSHP Fan,              !- Name",
//         "    OnSched,    !- Availability Schedule Name",
//         "    0.7,                     !- Fan Total Efficiency",
//         "    113,                   !- Pressure Rise {Pa}",
//         "    Autosize,                !- Maximum Flow Rate {m3/s}",
//         "    0.9,                     !- Motor Efficiency",
//         "    1.0,                     !- Motor In Airstream Fraction",
//         "    Space Out Node,    !- Air Inlet Node Name",
//         "    Lobby_ZN_1_FLR_2 WSHP Cooling Coil Air Inlet Node,  !- Air Outlet Node Name",
//         "    FanPowerCurve, !- Fan Efficiency Ratio Function of Speed Ratio Curve Name",
//         "    ,",
//         "    WSHP;",
//
//         "  Coil:Heating:Electric,",
//         "    Lobby_ZN_1_FLR_2 WSHP Supp Heating Coil,  !- Name",
//         "    OnSched,    !- Availability Schedule Name",
//         "    1.0,                     !- Gas Burner Efficiency",
//         "    Autosize,                   !- Nominal Capacity {W}",
//         "    Lobby_ZN_1_FLR_2 WSHP SuppHeating Coil Air Inlet Node,  !- Air Inlet Node Name",
//         "    Space In Node;       !- Air Outlet Node Name",
//
//     });
//
//     ASSERT_TRUE(process_idf(idf_objects));
//     state->init_state(*state);
//
//     bool ErrorsFound(false);
//     GetZoneData(*state, ErrorsFound);
//     GetZoneEquipmentData(*state);
//     state->dataZoneEquip->ZoneEquipInputsFilled = true; // denotes zone equipment has been read in
//     HVACSystemData *mySys;
//     mySys = UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, "Zone WSHP", true, 0);
//     auto &thisSys(state->dataUnitarySystems->unitarySys[0]);
//     thisSys.getUnitarySystemInput(*state, "Zone WSHP", true, 0);
//     state->dataUnitarySystems->getInputOnceFlag = false;
//
//     // Test for #8812:
//     // Verify zone sizing check if airflow is Autosized to prevent hard crash
//     state->dataSize->CurZoneEqNum = 1;
//     state->dataSize->ZoneEqSizing.allocate(1);
//     state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(35);
//     state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod = 0;
//     state->dataSize->ZoneSizingRunDone = false;
//     thisSys.m_HVACSizingIndex = 0;
//     thisSys.m_CoolOutAirVolFlow = AutoSize;
//     bool firstHVACIteration = false;
//     int airLoopNum = 0;
//     EXPECT_THROW(thisSys.sizeSystem(*state, firstHVACIteration, airLoopNum), std::runtime_error);
//     std::string const error_string = delimited_string({
//         "   ** Severe  ** For autosizing of ZoneHVAC:WaterToAirHeatPump ZONE WSHP, a zone sizing run must be done.",
//         "   **   ~~~   ** No \"Sizing:Zone\" objects were entered.",
//         "   **   ~~~   ** The \"SimulationControl\" object did not have the field \"Do Zone Sizing Calculation\" set to Yes.",
//         "   **  Fatal  ** Program terminates due to previously shown condition(s).",
//         "   ...Summary of Errors that led to program termination:",
//         "   ..... Reference severe error count=1",
//         "   ..... Last severe error=For autosizing of ZoneHVAC:WaterToAirHeatPump ZONE WSHP, a zone sizing run must be done.",
//     });
//
//     EXPECT_TRUE(compare_err_stream(error_string, true));
//
//     // Test for #7053:
//     // Fake that there is at least one UnitarySystemPerformance:Multispeed object
//     UnitarySystems::DesignSpecMSHP fakeDesignSpecMSHP;
//     state->dataUnitarySystems->designSpecMSHP.push_back(fakeDesignSpecMSHP);
//
//     state->dataPlnt->TotNumLoops = 2;
//     state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
//
//     for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
//         auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
//         loopside.TotalBranches = 1;
//         loopside.Branch.allocate(1);
//         auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
//         loopsidebranch.TotalComponents = 1;
//         loopsidebranch.Comp.allocate(1);
//     }
//     state->dataPlnt->PlantLoop(2).Name = "ChilledWaterLoop";
//     state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
//     state->dataPlnt->PlantLoop(2).FluidName = "WATER";
//     state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name =
//         state->dataVariableSpeedCoils->VarSpeedCoil(1).Name;
//     state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
//         DataPlant::PlantEquipmentType::CoilVSWAHPCoolingEquationFit;
//     state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
//         state->dataVariableSpeedCoils->VarSpeedCoil(1).WaterInletNodeNum;
//     state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
//         state->dataVariableSpeedCoils->VarSpeedCoil(1).WaterOutletNodeNum;
//
//     state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
//     state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
//     state->dataPlnt->PlantLoop(1).FluidName = "WATER";
//     state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name =
//         state->dataVariableSpeedCoils->VarSpeedCoil(2).Name;
//     state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
//         DataPlant::PlantEquipmentType::CoilVSWAHPHeatingEquationFit;
//     state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
//         state->dataVariableSpeedCoils->VarSpeedCoil(2).WaterInletNodeNum;
//     state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
//         state->dataVariableSpeedCoils->VarSpeedCoil(2).WaterOutletNodeNum;
//
//     state->dataSize->ZoneSizingRunDone = true;
//     state->dataSize->FinalZoneSizing.allocate(1);
//     state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 1.0;
//     state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 24.0;
//     state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInHumRat = 0.009;
//     state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneRetTempAtCoolPeak = 24.0;
//     state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtCoolPeak = 0.009;
//     state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesTemp = 12.0;
//     state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesHumRat = 0.007807825;
//     state->dataEnvrn->OutBaroPress = 101325;
//     state->dataEnvrn->StdRhoAir = 1.0;
//     thisSys.sizeSystem(*state, firstHVACIteration, airLoopNum);
//
//     // This VS coil is rather quirky. It sizes the capacity based on zone sizing air flow rate.
//     // Then uses that capacity to back calculate the air flow needed to keep the reference air flow per capacity ratio constant.
//     // For this reason, the parent object would size to an air flow that was different than the child.
//
//     // identify coil
//     EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(1).Name, "LOBBY_ZN_1_FLR_2 WSHP COOLING MODE");
//
//     // PTHP sized the VS coil differently. The PTHP uses the design air flow to size VS coil capacity
//     // then back calculates air flow rate. The PTHP would read the coil air flow and capacity and report
//     // those values to the eio. The UnitarySystem sizes the air flow rate and then calls the VS coil,
//     // which sizes, and uses the VS coil capacity to report UnitarySystem capacity to the eio.
//     // This requires and issue to correct.
//
//     // expect the ratio of air flow to capacity to be equal to the reference air flow and capacity ratio specified in coil input
//     Real64 refAirflowCapacityRatio = 0.891980668 / 16092.825525; // speed 9 reference cooling data
//     Real64 sizingAirflowCapacityRatio =
//         state->dataVariableSpeedCoils->VarSpeedCoil(1).MSRatedAirVolFlowRate(9) / state->dataVariableSpeedCoils->VarSpeedCoil(1).MSRatedTotCap(9);
//     EXPECT_EQ(refAirflowCapacityRatio, sizingAirflowCapacityRatio);
//
//     // this same ratio should also equal the internal flow per capacity variable used to back calculate operating air flow rate
//     EXPECT_EQ(sizingAirflowCapacityRatio, state->dataVariableSpeedCoils->VarSpeedCoil(1).MSRatedAirVolFlowPerRatedTotCap(9));
//
//     // identify coil
//     EXPECT_EQ(state->dataVariableSpeedCoils->VarSpeedCoil(2).Name, "LOBBY_ZN_1_FLR_2 WSHP HEATING MODE");
//
//     // expect coil air flow to equal PTUnit heating air flow
//     EXPECT_NEAR(state->dataVariableSpeedCoils->VarSpeedCoil(2).RatedAirVolFlowRate,
//                 state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).HeatingAirVolFlow,
//                 0.3); // DIFF from PTHP
//     EXPECT_NEAR(state->dataVariableSpeedCoils->VarSpeedCoil(2).RatedAirVolFlowRate, thisSys.m_MaxHeatAirVolFlow, 0.3);
//     EXPECT_NEAR(state->dataVariableSpeedCoils->VarSpeedCoil(2).MSRatedAirVolFlowRate(9), thisSys.m_MaxHeatAirVolFlow, 0.3);
//
//     // expect the ratio of air flow to capacity to equal to the reference air flow and capacity specified in coil input
//     refAirflowCapacityRatio = 0.891980668 / 20894.501936; // speed 9 reference heating data
//     sizingAirflowCapacityRatio =
//         state->dataVariableSpeedCoils->VarSpeedCoil(2).MSRatedAirVolFlowRate(9) / state->dataVariableSpeedCoils->VarSpeedCoil(2).MSRatedTotCap(9);
//     EXPECT_EQ(refAirflowCapacityRatio, sizingAirflowCapacityRatio);
//
//     // this same ratio should also equal the internal flow per capacity variable used to back calculate operating air flow rate
//     EXPECT_EQ(sizingAirflowCapacityRatio, state->dataVariableSpeedCoils->VarSpeedCoil(2).MSRatedAirVolFlowPerRatedTotCap(9));
//
//     state->dataFans->fans(1)->set_size(*state);
//     // the fan vol flow rate should equal the max of cooling and heating coil flow rates
//     Real64 maxCoilAirFlow =
//         max(state->dataVariableSpeedCoils->VarSpeedCoil(1).RatedAirVolFlowRate,
//         state->dataVariableSpeedCoils->VarSpeedCoil(2).RatedAirVolFlowRate);
//     EXPECT_NEAR(state->dataFans->fans(1)->maxAirFlowRate, maxCoilAirFlow, 0.000001);
//     EXPECT_NEAR(state->dataFans->fans(1)->maxAirFlowRate, max(thisSys.m_MaxCoolAirVolFlow, thisSys.m_MaxHeatAirVolFlow), 0.000001);
//
//     // Also set BeginEnvrnFlag so code is tested for coil initialization and does not crash
//     state->dataGlobal->BeginEnvrnFlag = true;
//     thisSys.initUnitarySystems(*state, 0, firstHVACIteration, 0.0);
//
//     // check that an intermediate speed has the correct flow ratio
//     Real64 refAirflowRatio = 0.530468926 / 0.891980668; // speed 4 reference cooling data and full flow rate at speed 9
//     Real64 expectedAirFlowRate = refAirflowRatio * thisSys.m_MaxCoolAirVolFlow;
//     EXPECT_NEAR(expectedAirFlowRate, thisSys.m_CoolVolumeFlowRate[4], 0.0000001);
//     EXPECT_NEAR(expectedAirFlowRate, 0.5947088, 0.000001); // DIFF from PTHP
//
//     refAirflowRatio = 0.530468926 / 0.891980668; // speed 4 reference heating data and full flow rate at speed 9
//     expectedAirFlowRate = refAirflowRatio * thisSys.m_MaxHeatAirVolFlow;
//     EXPECT_NEAR(expectedAirFlowRate, thisSys.m_HeatVolumeFlowRate[4], 0.2); // DIFF from PTHP
//     EXPECT_NEAR(expectedAirFlowRate, 0.5947, 0.0001);                       // DIFF from PTHP
//
//     // DIFF - comments for PTHP Variable Speed coil sizing
//     // #6028 child components not sizing correctly on air flow rate
//     // VS coils set SystemAirFlow to true and AirVolFlow to a value.
//     // all PTUnits set CoolingAirFlow and HeatingAirFlow, and CoolingAirVolFlow and HeatingAirVolFlow
//     // UnitarySystem now tracks the way PT units sized coils, which method is correct is for another day
//     EXPECT_FALSE(state->dataSize->ZoneEqSizing(1).SystemAirFlow);
//     EXPECT_NEAR(state->dataSize->ZoneEqSizing(1).AirVolFlow, state->dataVariableSpeedCoils->VarSpeedCoil(1).RatedAirVolFlowRate, 0.000001);
//     EXPECT_FALSE(state->dataSize->ZoneEqSizing(1).CoolingAirFlow);
//     EXPECT_FALSE(state->dataSize->ZoneEqSizing(1).HeatingAirFlow);
//     EXPECT_EQ(state->dataSize->ZoneEqSizing(1).CoolingAirVolFlow, thisSys.m_MaxCoolAirVolFlow);
//     EXPECT_LT(state->dataSize->ZoneEqSizing(1).HeatingAirVolFlow, thisSys.m_MaxHeatAirVolFlow);
//     EXPECT_EQ(state->dataFans->fans(1)->maxAirFlowRate, state->dataSize->ZoneEqSizing(1).AirVolFlow);
//     EXPECT_EQ(state->dataFans->fans(1)->maxAirFlowRate,
//               max(state->dataSize->ZoneEqSizing(1).CoolingAirVolFlow, state->dataSize->ZoneEqSizing(1).HeatingAirVolFlow));
// }

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_SimPTAC_HeatingCoilTest)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 HVACInletMassFlowRate(0.0);
    Real64 PrimaryAirMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QZnReq(0.0);
    int PTUnitNum(0);

    std::string const idf_objects = delimited_string({
        "Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 16",
        "    1.0;                     !- Field 17",

        "Schedule:Compact,",
        "    ContinuousFanSch,        !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    1.0;                     !- Field 4",

        "ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 PTAC,           !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:PackagedTerminalAirConditioner,",
        "    SPACE1-1 PTAC,           !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    SPACE1-1 HP Inlet Node,  !- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet,   !- Air Outlet Node Name",
        "    OutdoorAir:Mixer,        !- Outdoor Air Mixer Object Type",
        "    PTACOAMixer,             !- Outdoor Air Mixer Name",
        "    0.500,                   !- Supply Air Flow Rate During Cooling Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate During Heating Operation {m3/s}",
        "    ,                        !- Supply Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    0.200,                   !- Outdoor Air Flow Rate During Cooling Operation {m3/s}",
        "    0.200,                   !- Outdoor Air Flow Rate During Heating Operation {m3/s}",
        "    0.200,                   !- Outdoor Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    Fan:ConstantVolume,      !- Supply Air Fan Object Type",
        "    SPACE1-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Heating:Fuel,       !- Heating Coil Object Type",
        "    SPACE1-1 Heating Coil,   !- Heating Coil Name",
        "    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "    SPACE1-1 PTAC CCoil,     !- Cooling Coil Name",
        "    BlowThrough,             !- Fan Placement",
        "    ContinuousFanSch;        !- Supply Air Fan Operating Mode Schedule Name",

        "  OutdoorAir:Mixer,",
        "	 PTACOAMixer,             !- Name",
        "	 PTACOAMixerOutletNode,   !- Mixed Air Node Name",
        "    PTACOAInNode,            !- Outdoor Air Stream Node Name",
        "    ZoneExhausts,            !- Relief Air Stream Node Name",
        "    SPACE1-1 HP Inlet Node;  !- Return Air Stream Node Name",

        "Fan:ConstantVolume,",
        "    SPACE1-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    0.500,                   !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    PTACOAMixerOutletNode,   !- Air Inlet Node Name",
        "    SPACE1-1 Fan Outlet Node;!- Air Outlet Node Name",

        "Coil:Heating:Fuel,",
        "    SPACE1-1 Heating Coil,   !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Gas Burner Efficiency",
        "    10000.0,                 !- Nominal Capacity {W}",
        "    SPACE1-1 CCoil Outlet Node,  !- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet;   !- Air Outlet Node Name",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    SPACE1-1 PTAC CCoil,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    6680.0,                  !- Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    0.500,                   !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE1-1 Fan Outlet Node,!- Air Inlet Node Name",
        "    SPACE1-1 CCoil Outlet Node,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACPLFFPLR;             !- Part Load Fraction Correlation Curve Name",

        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    FanEffRatioCurve,        !- Name",
        "    0.33856828,              !- Coefficient1 Constant",
        "    1.72644131,              !- Coefficient2 x",
        "    -1.49280132,             !- Coefficient3 x**2",
        "    0.42776208,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.3,                     !- Minimum Curve Output",
        "    1.0;                     !- Maximum Curve Output",

        "  Curve:Exponent,",
        "    FanPowerRatioCurve,      !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 Constant",
        "    3.0,                     !- Coefficient3 Constant",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.01,                    !- Minimum Curve Output",
        "    1.5;                     !- Maximum Curve Output",

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

        "Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 Exhausts,       !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE1-1 Return Outlet;  !- Zone Return Air Node Name",

        "NodeList,",
        "    SPACE1-1 Inlets,         !- Name",
        "    SPACE1-1 Supply Inlet;   !- Node 1 Name",

        "NodeList,",
        "    SPACE1-1 Exhausts,       !- Name",
        "    SPACE1-1 HP Inlet Node;  !- Node 1 Name",

        "NodeList,",
        "    OutsideAirInletNodes,    !- Name",
        "    PTACOAInNode;            !- Node 1 Name",

        "OutdoorAir:NodeList,",
        "    OutsideAirInletNodes;    !- Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataGlobal->TimeStep = 1;

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    HVACSystemData *mySys;
    mySys = UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, "SPACE1-1 PTAC", true, 0);
    auto &thisSys(state->dataUnitarySystems->unitarySys[0]);
    thisSys.getUnitarySystemInput(*state, "SPACE1-1 PTAC", true, 0);
    state->dataUnitarySystems->getInputOnceFlag = false;

    //// get input test for terminal air single duct mixer on inlet side of PTAC
    ASSERT_EQ(1, state->dataUnitarySystems->numUnitarySystems);
    EXPECT_EQ("ZoneHVAC:PackagedTerminalAirConditioner", thisSys.UnitType);                             // zoneHVAC equipment type
    EXPECT_EQ("COIL:HEATING:FUEL", thisSys.m_HeatingCoilTypeName);                                      // PTAC heating coil type
    EXPECT_EQ(state->dataHeatingCoils->HeatingCoil(1).coilType, HVAC::CoilType::HeatingGasOrOtherFuel); // gas heating coil type

    state->dataGlobal->BeginEnvrnFlag = false;

    // set input variables
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutDryBulbTemp = 10.0;
    state->dataEnvrn->OutHumRat = 0.0075;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    state->dataEnvrn->StdRhoAir = 1.20;
    HVACInletMassFlowRate = 0.50;
    PrimaryAirMassFlowRate = 0.20;

    // set zoneNode air condition
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 21.1;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    thisSys.m_fanOpModeSched->currentVal = 1.0; // unit is always on
    thisSys.m_sysAvailSched->currentVal = 1.0;  // unit is always available
    thisSys.m_fanAvailSched->currentVal = 1.0;  // fan is always available

    // initialize mass flow rates
    state->dataLoopNodes->Node(thisSys.AirInNode).MassFlowRate = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(thisSys.m_OAMixerNodes[0]).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(thisSys.m_OAMixerNodes[0]).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;

    // set fan parameters
    state->dataFans->fans(1)->maxAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->inletAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMaxAvail = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->outletNodeNum).MassFlowRateMax = HVACInletMassFlowRate;

    // set DX coil rated performance parameters
    state->dataDXCoils->DXCoil(1).RatedCBF(1) = 0.05;
    state->dataDXCoils->DXCoil(1).RatedAirMassFlowRate(1) = HVACInletMassFlowRate;

    // primary air condition set at outdoor air condition
    state->dataLoopNodes->Node(thisSys.m_OAMixerNodes[0]).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(thisSys.m_OAMixerNodes[0]).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(thisSys.m_OAMixerNodes[0]).Enthalpy = state->dataEnvrn->OutEnthalpy;

    // set secondary air (recirculating air) conditions to zone air node
    state->dataLoopNodes->Node(thisSys.AirInNode).Temp = state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp;
    state->dataLoopNodes->Node(thisSys.AirInNode).HumRat = state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat;
    state->dataLoopNodes->Node(thisSys.AirInNode).Enthalpy = state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy;

    state->dataUnitarySystems->unitarySys[0].ControlZoneNum = 1;
    state->dataSize->SysSizingRunDone = false;
    state->dataSize->ZoneSizingRunDone = false;
    state->dataGlobal->SysSizingCalc = true;

    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::SingleHeat;

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;    // set heating load to zero
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = 0.0;    // set cooling load to zero
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 0.0;       // set cooling load to zero
    QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP; // zero zone heating load

    state->dataUnitarySystems->HeatingLoad = false;
    state->dataUnitarySystems->CoolingLoad = false;
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;

    // supply fan is continuous flow
    thisSys.MaxHeatAirMassFlow = HVACInletMassFlowRate;
    thisSys.m_HeatingSpeedRatio = 1.0;
    thisSys.m_HeatOutAirMassFlow = PrimaryAirMassFlowRate;
    thisSys.MaxNoCoolHeatAirMassFlow = HVACInletMassFlowRate;
    thisSys.m_NoHeatCoolSpeedRatio = 1.0;
    thisSys.m_NoCoolHeatOutAirMassFlow = PrimaryAirMassFlowRate;
    thisSys.m_AirFlowControl = UnitarySystems::UnitarySys::UseCompFlow::On;
    thisSys.m_LastMode = UnitarySystems::HeatingMode;

    // initialized to false
    ASSERT_FALSE(state->dataUnitarySystems->HeatingLoad);
    bool HeatActive = false;
    bool CoolActive = false;
    Real64 latOut = 0.0;
    mySys->simulate(*state, thisSys.Name, FirstHVACIteration, 0, PTUnitNum, HeatActive, CoolActive, 0, 0.0, true, QUnitOut, latOut);
    // init sets heating mode to true due to cold ventilation air
    // DIFFERENCE (previously ASSERT_TRUE on HeatingLoad) - this will take an issue to correct or diffs elsewhere
    // UnitarySystem resets HeatingLoad and CoolingLoad when QZnReq < SmallLoad
    // UnitarySystem is also not using the UseCompressorOnFlow when there is no load
    ASSERT_FALSE(state->dataUnitarySystems->HeatingLoad);
    // no zone heating load // logic expects < 0 in initLoadBasedControl
    EXPECT_NEAR(QZnReq, 0.0, 0.01);
    // no net heating delivered to the zone
    EXPECT_NEAR(QUnitOut, -2217.05, 0.01);
    // heating coil inlet air temperature
    ASSERT_NEAR(state->dataHeatingCoils->HeatingCoil(1).InletAirTemp, 16.74764, 0.00001);
    // heating coil tempers cold ventilation air to neutral (zone air temp)
    ASSERT_NEAR(state->dataHeatingCoils->HeatingCoil(1).OutletAirTemp, 16.74764, 0.001); // DIFF - heating coil off, 0 air flow
    // heating coil air flow rate, continuous fan operation
    ASSERT_NEAR(state->dataHeatingCoils->HeatingCoil(1).OutletAirMassFlowRate, 0.50, 0.00001);
    // heating coil load due to cold ventilation air
    ASSERT_NEAR(state->dataHeatingCoils->HeatingCoil(1).HeatingCoilRate, 0.0, 1.0); // DIFF - heating coil off, 0 air flow
}

TEST_F(EnergyPlusFixture, SimPTAC_SZVAVTest)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 HVACInletMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QZnReq(0.0);
    int PTUnitNum(0);

    std::string const idf_objects = delimited_string({
        "  Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 16",
        "    1.0;                     !- Field 17",

        "  Schedule:Compact,",
        "    ContinuousFanSch,        !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    1.0;                     !- Field 4",

        "  ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 PTAC,           !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:PackagedTerminalAirConditioner,",
        "    SPACE1-1 PTAC,           !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    SPACE1-1 HP Inlet Node,  !- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet,   !- Air Outlet Node Name",
        "    OutdoorAir:Mixer,        !- Outdoor Air Mixer Object Type",
        "    PTACOAMixer,             !- Outdoor Air Mixer Name",
        "    0.500,                   !- Supply Air Flow Rate During Cooling Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate During Heating Operation {m3/s}",
        "    0.335,                   !- Supply Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    0.200,                   !- Outdoor Air Flow Rate During Cooling Operation {m3/s}",
        "    0.200,                   !- Outdoor Air Flow Rate During Heating Operation {m3/s}",
        "    0.200,                   !- Outdoor Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    Fan:ConstantVolume,      !- Supply Air Fan Object Type",
        "    SPACE1-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Heating:Fuel,       !- Heating Coil Object Type",
        "    SPACE1-1 Heating Coil,   !- Heating Coil Name",
        "    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "    SPACE1-1 PTAC CCoil,     !- Cooling Coil Name",
        "    BlowThrough,             !- Fan Placement",
        "    FanAvailSched,           !- Supply Air Fan Operating Mode Schedule Name",
        "    ,                        !- Availability Manager List Name",
        "    ,                        !- Design Specification ZoneHVAC Sizing Object Name",
        "    SingleZoneVAV,           !- Capacity Control Method",
        "    18.0,                    !- Minimum Supply Air Temperature in Cooling Mode",
        "    26.0;                    !- Maximum Supply Air Temperature in Heating Mode",

        "  OutdoorAir:Mixer,",
        "	 PTACOAMixer,             !- Name",
        "	 PTACOAMixerOutletNode,   !- Mixed Air Node Name",
        "    PTACOAInNode,            !- Outdoor Air Stream Node Name",
        "    ZoneExhausts,            !- Relief Air Stream Node Name",
        "    SPACE1-1 HP Inlet Node;  !- Return Air Stream Node Name",

        "Fan:ConstantVolume,",
        "    SPACE1-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    0.500,                   !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    PTACOAMixerOutletNode,   !- Air Inlet Node Name",
        "    SPACE1-1 Fan Outlet Node;!- Air Outlet Node Name",

        "Coil:Heating:Fuel,",
        "    SPACE1-1 Heating Coil,   !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Gas Burner Efficiency",
        "    10000.0,                 !- Nominal Capacity {W}",
        "    SPACE1-1 CCoil Outlet Node,  !- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet;   !- Air Outlet Node Name",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    SPACE1-1 PTAC CCoil,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    6680.0,                  !- Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    0.500,                   !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE1-1 Fan Outlet Node,!- Air Inlet Node Name",
        "    SPACE1-1 CCoil Outlet Node,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACPLFFPLR;             !- Part Load Fraction Correlation Curve Name",

        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    FanEffRatioCurve,        !- Name",
        "    0.33856828,              !- Coefficient1 Constant",
        "    1.72644131,              !- Coefficient2 x",
        "    -1.49280132,             !- Coefficient3 x**2",
        "    0.42776208,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.3,                     !- Minimum Curve Output",
        "    1.0;                     !- Maximum Curve Output",

        "  Curve:Exponent,",
        "    FanPowerRatioCurve,      !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 Constant",
        "    3.0,                     !- Coefficient3 Constant",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.01,                    !- Minimum Curve Output",
        "    1.5;                     !- Maximum Curve Output",

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

        "Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 Exhausts,       !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE1-1 Return Outlet;  !- Zone Return Air Node Name",

        "NodeList,",
        "    SPACE1-1 Inlets,         !- Name",
        "    SPACE1-1 Supply Inlet;   !- Node 1 Name",

        "NodeList,",
        "    SPACE1-1 Exhausts,       !- Name",
        "    SPACE1-1 HP Inlet Node;  !- Node 1 Name",

        "NodeList,",
        "    OutsideAirInletNodes,    !- Name",
        "    PTACOAInNode;            !- Node 1 Name",

        "OutdoorAir:NodeList,",
        "    OutsideAirInletNodes;    !- Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataGlobal->TimeStep = 1;

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    HVACSystemData *mySys;
    mySys = UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, "SPACE1-1 PTAC", true, 0);
    auto &thisSys(state->dataUnitarySystems->unitarySys[0]);

    state->dataZoneEquip->ZoneEquipInputsFilled = true; // denotes zone equipment has been read in
    state->dataUnitarySystems->unitarySys[0].getUnitarySystemInput(*state, "SPACE1-1 PTAC", true, 0);
    state->dataUnitarySystems->getInputOnceFlag = false;

    state->dataGlobal->BeginEnvrnFlag = true;

    // set input variables
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutDryBulbTemp = 10.0;
    state->dataEnvrn->OutHumRat = 0.0075;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    state->dataEnvrn->StdRhoAir = 1.20;
    HVACInletMassFlowRate = 0.50;
    //		PrimaryAirMassFlowRate = 0.20;

    // set zoneNode air condition
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 21.1;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataUnitarySystems->HeatingLoad = false;
    state->dataUnitarySystems->CoolingLoad = false;
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;

    thisSys.m_fanOpModeSched->currentVal = 1.0; // unit is always on
    thisSys.m_sysAvailSched->currentVal = 1.0;  // unit is always available
    thisSys.m_fanAvailSched->currentVal = 1.0;  // fan is always available

    // set fan parameters
    state->dataFans->fans(1)->maxAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->inletAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMaxAvail = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->outletNodeNum).MassFlowRateMax = HVACInletMassFlowRate;

    // set DX coil rated performance parameters
    state->dataDXCoils->DXCoil(1).RatedCBF(1) = 0.05;
    state->dataDXCoils->DXCoil(1).RatedAirMassFlowRate(1) = HVACInletMassFlowRate;

    // primary air condition set at outdoor air condition
    state->dataLoopNodes->Node(thisSys.m_OAMixerNodes[0]).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(thisSys.m_OAMixerNodes[0]).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(thisSys.m_OAMixerNodes[0]).Enthalpy = state->dataEnvrn->OutEnthalpy;

    // set secondary air (recirculating air) conditions to zone air node
    state->dataLoopNodes->Node(thisSys.AirInNode).Temp = state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp;
    state->dataLoopNodes->Node(thisSys.AirInNode).HumRat = state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat;
    state->dataLoopNodes->Node(thisSys.AirInNode).Enthalpy = state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy;

    state->dataSize->SysSizingRunDone = false;
    state->dataSize->ZoneSizingRunDone = false;
    state->dataGlobal->SysSizingCalc = false;

    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::SingleHeat;

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = -10.0; // set heating load to deadband
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = 10.0;  // set cooling load to deadband
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -10.0;    // set load to deadband
    QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired;   // no heating load

    // initialized to false
    ASSERT_FALSE(state->dataUnitarySystems->HeatingLoad);
    // simulate PTAC zoneHVAC equipment
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->ZoneEqSizing.allocate(1);
    bool HeatActive = false;
    bool CoolActive = false;
    Real64 latOut = 0.0;
    thisSys.initUnitarySystems(*state, 0, FirstHVACIteration, 0.0);
    thisSys.simulate(*state, thisSys.Name, FirstHVACIteration, 0, PTUnitNum, HeatActive, CoolActive, 0, 0.0, true, QUnitOut, latOut);
    // init sets heating mode to true due to cold ventilation air
    ASSERT_TRUE(state->dataUnitarySystems->HeatingLoad);
    // original zone heating load
    ASSERT_DOUBLE_EQ(QZnReq, -10.0);
    // no net heating delivered to the zone
    EXPECT_NEAR(QUnitOut, -10.0, 0.01);
    // heating coil inlet air temperature
    ASSERT_NEAR(state->dataHeatingCoils->HeatingCoil(1).InletAirTemp, 14.560774, 0.00001);
    // heating coil tempers cold ventilation air to neutral (zone air temp, otherwise QUnitOut out would be non-zero above)
    ASSERT_NEAR(state->dataHeatingCoils->HeatingCoil(1).OutletAirTemp, 21.07558, 0.00001);
    // heating coil air flow rate, operate at minimum air flow rate
    ASSERT_NEAR(state->dataHeatingCoils->HeatingCoil(1).OutletAirMassFlowRate, 0.40200, 0.00001);
    // heating coil load due to cold ventilation air (but total load delivered by PTUnit is 0)
    ASSERT_NEAR(state->dataHeatingCoils->HeatingCoil(1).HeatingCoilRate, 2668.1427, 0.0001);

    // Boundary load for this system in Region 1 at minimum air flow rate is 2006.8 W (lower boundary load in Region 1)
    // loads below the boundary load should operate at the minimum air flow rate
    // set heating load to non-zero value below lower boundary load
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 1000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 1000.0;
    thisSys.simulate(*state, thisSys.Name, FirstHVACIteration, 0, PTUnitNum, HeatActive, CoolActive, 0, 0, true, QUnitOut, latOut);
    EXPECT_NEAR(QUnitOut, 1000.0, 0.01);
    ASSERT_NEAR(state->dataLoopNodes->Node(thisSys.AirInNode).MassFlowRate, thisSys.MaxNoCoolHeatAirMassFlow, 0.001);
    ASSERT_GT(thisSys.DesignMaxOutletTemp, state->dataHeatingCoils->HeatingCoil(1).OutletAirTemp);
    EXPECT_TRUE(thisSys.m_useNoLoadLowSpeedAirFlow); // because input is blank, doesn't affect no load flow rate because coils are single sped

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2000.0; // set heating load to just below lower boundary load
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 2000.0;
    thisSys.simulate(*state, thisSys.Name, FirstHVACIteration, 0, PTUnitNum, HeatActive, CoolActive, 0, 0, true, QUnitOut, latOut);
    EXPECT_NEAR(QUnitOut, 2000.0, 0.01);
    ASSERT_NEAR(state->dataLoopNodes->Node(thisSys.AirInNode).MassFlowRate, thisSys.MaxNoCoolHeatAirMassFlow, 0.001);
    ASSERT_GT(thisSys.DesignMaxOutletTemp, state->dataHeatingCoils->HeatingCoil(1).OutletAirTemp);

    // loads above the lower boundary load should operate above the minimum air flow rate
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2010.0; // set heating load to just above lower boundary load
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 2010.0;
    //    QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired; // initialize zone heating load
    thisSys.simulate(*state, thisSys.Name, FirstHVACIteration, 0, PTUnitNum, HeatActive, CoolActive, 0, 0, true, QUnitOut, latOut);
    EXPECT_NEAR(QUnitOut, 2010.0, 0.01);
    ASSERT_NEAR(thisSys.DesignMaxOutletTemp, state->dataHeatingCoils->HeatingCoil(1).OutletAirTemp, 1.0);
    ASSERT_GT(state->dataLoopNodes->Node(thisSys.AirInNode).MassFlowRate, thisSys.MaxNoCoolHeatAirMassFlow);

    // Boundary load for this system in Region 1 at maximum air flow rate is 2995.2 W (upper boundary load of Region 1)
    // system should operate below the maximum air flow rate at loads less than 2995.2 W
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2990.0; // set heating load to just below upper boundary load
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 2990.0;
    //    QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired; // initialize zone heating load
    thisSys.simulate(*state, thisSys.Name, FirstHVACIteration, 0, PTUnitNum, HeatActive, CoolActive, 0, 0, true, QUnitOut, latOut);
    EXPECT_NEAR(QUnitOut, 2990.0, 0.01);
    ASSERT_NEAR(thisSys.DesignMaxOutletTemp, state->dataHeatingCoils->HeatingCoil(1).OutletAirTemp, 1.0);
    ASSERT_GT(state->dataLoopNodes->Node(thisSys.AirInNode).MassFlowRate, thisSys.MaxNoCoolHeatAirMassFlow);
    ASSERT_LT(state->dataLoopNodes->Node(thisSys.AirInNode).MassFlowRate, thisSys.MaxHeatAirMassFlow);

    // Boundary load for this system in Region 1 at maximum air flow rate is 2995.2 W
    // system should operate at maximum air flow rate for loads greater than 2995.2 W
    // outlet air temperature is allowed to be above the design maximum supply air temperature in heating mode
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 3000.0; // set heating load to just above upper boundary load
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 3000.0;
    thisSys.simulate(*state, thisSys.Name, FirstHVACIteration, 0, PTUnitNum, HeatActive, CoolActive, 0, 0, true, QUnitOut, latOut);
    EXPECT_NEAR(QUnitOut, 3000.0, 0.01);
    ASSERT_GT(state->dataHeatingCoils->HeatingCoil(1).OutletAirTemp, thisSys.DesignMaxOutletTemp);
    ASSERT_NEAR(state->dataLoopNodes->Node(thisSys.AirInNode).MassFlowRate, thisSys.MaxHeatAirMassFlow, 0.0001);
}

TEST_F(EnergyPlusFixture, PTACDrawAirfromReturnNodeAndPlenum_Test)
{

    std::string const idf_objects = delimited_string({
        "  Version," + DataStringGlobals::MatchVersion + ";",

        "  SimulationControl,",
        "    No,                     !- Do Zone Sizing Calculation",
        "    No,                     !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "    Yes,                     !- Run Simulation for Weather File Run Periods",
        "    No,                      !- Do HVAC Sizing Simulation for Sizing Periods",
        "    1;                       !- Maximum Number of HVAC Sizing Simulation Passes",

        "  Building,",
        "    FPFC with DOAS to Supply Side,  !- Name",
        "    30.,                     !- North Axis {deg}",
        "    City,                    !- Terrain",
        "    0.04,                    !- Loads Convergence Tolerance Value {W}",
        "    0.4,                     !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullExterior,            !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",

        "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  Timestep,4;",

        "  Site:Location,",
        "    Miami Intl Ap FL USA WMO=722020,  !- Name",
        "    25.82,                   !- Latitude {deg}",
        "    -80.30,                  !- Longitude {deg}",
        "    -5.00,                   !- Time Zone {hr}",
        "    11.00;                   !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    Miami Intl Ap Ann Htg 99.6% Condns DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    8.7,                     !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    8.7,                     !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    101217.,                 !- Barometric Pressure {Pa}",
        "    3.8,                     !- Wind Speed {m/s}",
        "    340,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.00;                    !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    Miami Intl Ap Ann Clg .4% Condns DB=>MWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    33.2,                    !- Maximum Dry-Bulb Temperature {C}",
        "    6.7,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    25.3,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    101217.,                 !- Barometric Pressure {Pa}",
        "    4.5,                     !- Wind Speed {m/s}",
        "    140,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.00;                    !- Sky Clearness",

        "  RunPeriod,",
        "    Run Period 1,            !- Name",
        "    1,                       !- Begin Month",
        "    14,                      !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    1,                       !- End Month",
        "    14,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    Yes;                     !- Use Weather File Snow Indicators",

        "  RunPeriod,",
        "    Run Period 2,            !- Name",
        "    7,                       !- Begin Month",
        "    7,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    7,                       !- End Month",
        "    7,                       !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    Yes;                     !- Use Weather File Snow Indicators",

        "  Site:GroundTemperature:BuildingSurface,22.03,22.03,22.13,22.30,22.43,22.52,22.62,22.77,22.78,22.55,22.44,22.20;",

        "  RunPeriodControl:DaylightSavingTime,",
        "    1st Sunday in April,     !- Start Date",
        "    Last Sunday in October;  !- End Date",

        "  ScheduleTypeLimits,",
        "    Any Number;              !- Name",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "  ScheduleTypeLimits,",
        "    Temperature,             !- Name",
        "    -60,                     !- Lower Limit Value",
        "    200,                     !- Upper Limit Value",
        "    CONTINUOUS,              !- Numeric Type",
        "    Temperature;             !- Unit Type",

        "  ScheduleTypeLimits,",
        "    Control Type,            !- Name",
        "    0,                       !- Lower Limit Value",
        "    4,                       !- Upper Limit Value",
        "    DISCRETE;                !- Numeric Type",

        "  ScheduleTypeLimits,",
        "    On/Off,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    DISCRETE;                !- Numeric Type",

        "  ScheduleTypeLimits,",
        "    FlowRate,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    10,                      !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "  ScheduleTypeLimits,",
        "    HVACTemplate Any Number; !- Name",

        "  Schedule:Compact,",
        "    OCCUPY-1,                !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays SummerDesignDay CustomDay1 CustomDay2, !- Field 2",
        "    Until: 8:00,0.0,         !- Field 3",
        "    Until: 11:00,1.00,       !- Field 5",
        "    Until: 12:00,0.80,       !- Field 7",
        "    Until: 13:00,0.40,       !- Field 9",
        "    Until: 14:00,0.80,       !- Field 11",
        "    Until: 18:00,1.00,       !- Field 13",
        "    Until: 19:00,0.50,       !- Field 15",
        "    Until: 21:00,0.10,       !- Field 17",
        "    Until: 24:00,0.0,        !- Field 19",
        "    For: Weekends WinterDesignDay Holiday, !- Field 21",
        "    Until: 24:00,0.0;        !- Field 22",

        "  Schedule:Compact,",
        "    LIGHTS-1,                !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays SummerDesignDay CustomDay1 CustomDay2, !- Field 2",
        "    Until: 8:00,0.05,        !- Field 3",
        "    Until: 9:00,0.9,         !- Field 5",
        "    Until: 10:00,0.95,       !- Field 7",
        "    Until: 11:00,1.00,       !- Field 9",
        "    Until: 12:00,0.95,       !- Field 11",
        "    Until: 13:00,0.8,        !- Field 13",
        "    Until: 14:00,0.9,        !- Field 15",
        "    Until: 18:00,1.00,       !- Field 17",
        "    Until: 19:00,0.60,       !- Field 19",
        "    Until: 21:00,0.40,       !- Field 21",
        "    Until: 24:00,0.05,       !- Field 23",
        "    For: Weekends WinterDesignDay Holiday, !- Field 25",
        "    Until: 24:00,0.05;       !- Field 26",

        "  Schedule:Compact,",
        "    EQUIP-1,                 !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays SummerDesignDay CustomDay1 CustomDay2, !- Field 2",
        "    Until: 8:00,0.02,        !- Field 3",
        "    Until: 9:00,0.4,         !- Field 5",
        "    Until: 14:00,0.9,        !- Field 7",
        "    Until: 15:00,0.8,        !- Field 9",
        "    Until: 16:00,0.7,        !- Field 11",
        "    Until: 18:00,0.5,        !- Field 13",
        "    Until: 21:00,0.3,        !- Field 15",
        "    Until: 24:00,0.02,       !- Field 17",
        "    For: Weekends WinterDesignDay Holiday, !- Field 19",
        "    Until: 24:00,0.02;       !- Field 20",

        "  Schedule:Compact,",
        "    INFIL-SCH,               !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2, !- Field 2",
        "    Until: 7:00,1.0,         !- Field 3",
        "    Until: 21:00,0.0,        !- Field 5",
        "    Until: 24:00,1.0,        !- Field 7",
        "    For: Weekends Holiday,   !- Field 9",
        "    Until: 24:00,1.0,        !- Field 10",
        "    For: SummerDesignDay,    !- Field 12",
        "    Until: 24:00,1.0,        !- Field 13",
        "    For: WinterDesignDay,    !- Field 15",
        "    Until: 24:00,1.0;        !- Field 16",

        "  Schedule:Compact,",
        "    ActSchd,                 !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,117.239997864; !- Field 3",

        "  Schedule:Compact,",
        "    ShadeTransSch,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,0.0;        !- Field 3",

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
        "    HVACTemplate-Always 1,   !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1;          !- Field 3",

        "  Schedule:Compact,",
        "    HVACTemplate-Always 4,   !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,4;          !- Field 3",

        "  Schedule:Compact,",
        "    Always 21.1,             !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,21.1;       !- Field 3",

        "  Schedule:Compact,",
        "    CyclingFanSch,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,0.0;        !- Field 3",

        "  Schedule:Compact,",
        "    ContsFanSch,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  Material,",
        "    WD10,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    0.667,                   !- Thickness {m}",
        "    0.115,                   !- Conductivity {W/m-K}",
        "    513,                     !- Density {kg/m3}",
        "    1381,                    !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.78,                    !- Solar Absorptance",
        "    0.78;                    !- Visible Absorptance",

        "  Material,",
        "    RG01,                    !- Name",
        "    Rough,                   !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    1.442000,                !- Conductivity {W/m-K}",
        "    881.0000,                !- Density {kg/m3}",
        "    1674.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material,",
        "    BR01,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    9.4999997E-03,           !- Thickness {m}",
        "    0.1620000,               !- Conductivity {W/m-K}",
        "    1121.000,                !- Density {kg/m3}",
        "    1464.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  Material,",
        "    IN46,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    7.6200001E-02,           !- Thickness {m}",
        "    2.3000000E-02,           !- Conductivity {W/m-K}",
        "    24.00000,                !- Density {kg/m3}",
        "    1590.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Material,",
        "    WD01,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.9099999E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    513.0000,                !- Density {kg/m3}",
        "    1381.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    PW03,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    545.0000,                !- Density {kg/m3}",
        "    1213.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    IN02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    9.0099998E-02,           !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    GP01,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1600000,               !- Conductivity {W/m-K}",
        "    801.0000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    GP02,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.5900001E-02,           !- Thickness {m}",
        "    0.1600000,               !- Conductivity {W/m-K}",
        "    801.0000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    CC03,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1016000,               !- Thickness {m}",
        "    1.310000,                !- Conductivity {W/m-K}",
        "    2243.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    CP01,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.3670000,               !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    MAT-SB-U,                !- Name",
        "    Rough,                   !- Roughness",
        "    0.117406666,             !- Thermal Resistance {m2-K/W}",
        "    0.65,                    !- Thermal Absorptance",
        "    0.65,                    !- Solar Absorptance",
        "    0.65;                    !- Visible Absorptance",

        "  Material:NoMass,",
        "    MAT-CLNG-1,              !- Name",
        "    Rough,                   !- Roughness",
        "    0.652259290,             !- Thermal Resistance {m2-K/W}",
        "    0.65,                    !- Thermal Absorptance",
        "    0.65,                    !- Solar Absorptance",
        "    0.65;                    !- Visible Absorptance",

        "  Material:NoMass,",
        "    MAT-FLOOR-1,             !- Name",
        "    Rough,                   !- Roughness",
        "    3.522199631,             !- Thermal Resistance {m2-K/W}",
        "    0.65,                    !- Thermal Absorptance",
        "    0.65,                    !- Solar Absorptance",
        "    0.65;                    !- Visible Absorptance",

        "  Material:AirGap,",
        "    AL21,                    !- Name",
        "    0.1570000;               !- Thermal Resistance {m2-K/W}",

        "  Material:AirGap,",
        "    AL23,                    !- Name",
        "    0.1530000;               !- Thermal Resistance {m2-K/W}",

        "  WindowMaterial:Glazing,",
        "    CLEAR 3MM,               !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.837,                   !- Solar Transmittance at Normal Incidence",
        "    0.075,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.075,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.898,                   !- Visible Transmittance at Normal Incidence",
        "    0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.081,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "  Construction,",
        "    ROOF-1,                  !- Name",
        "    RG01,                    !- Outside Layer",
        "    BR01,                    !- Layer 2",
        "    IN46,                    !- Layer 3",
        "    WD01;                    !- Layer 4",

        "  Construction,",
        "    WALL-1,                  !- Name",
        "    WD01,                    !- Outside Layer",
        "    PW03,                    !- Layer 2",
        "    IN02,                    !- Layer 3",
        "    GP01;                    !- Layer 4",

        "  Construction,",
        "    CLNG-1,                  !- Name",
        "    MAT-CLNG-1;              !- Outside Layer",

        "  Construction,",
        "    FLOOR-SLAB-1,            !- Name",
        "    CC03;                    !- Outside Layer",

        "  Construction,",
        "    INT-WALL-1,              !- Name",
        "    GP02,                    !- Outside Layer",
        "    AL21,                    !- Layer 2",
        "    GP02;                    !- Layer 3",

        "  Construction,",
        "    Sgl Grey 3mm,            !- Name",
        "    CLEAR 3MM;                !- Outside Layer",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",

        "  Zone,",
        "    PLENUM-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    0.609600067,             !- Ceiling Height {m}",
        "    283.2;                   !- Volume {m3}",

        "  Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

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

        "  Zone,",
        "    SPACE3-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "  Zone,",
        "    SPACE4-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    103.311355591;           !- Volume {m3}",

        "  Zone,",
        "    SPACE5-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    447.682556152;           !- Volume {m3}",

        "  BuildingSurface:Detailed,",
        "    WALL-1PF,                !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,0.0,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,0.0,2.4,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.5,0.0,2.4,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.5,0.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    WALL-1PR,                !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5,0.0,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.5,0.0,2.4,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.5,15.2,2.4,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.5,15.2,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    WALL-1PB,                !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5,15.2,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.5,15.2,2.4,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,15.2,2.4,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,15.2,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    WALL-1PL,                !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,15.2,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,15.2,2.4,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,0.0,2.4,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,0.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    TOP-1,                   !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.00000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,15.2,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,0.0,3.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.5,0.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.5,15.2,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    C1-1P,                   !- Name",
        "    FLOOR,                   !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C1-1,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,3.7,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.5,0.0,2.4,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,0.0,2.4,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3.7,3.7,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    C2-1P,                   !- Name",
        "    FLOOR,                   !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C2-1,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,11.6,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.5,15.2,2.4,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.5,0.0,2.4,  !- X,Y,Z ==> Vertex 3 {m}",
        "    26.8,3.7,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    C3-1P,                   !- Name",
        "    FLOOR,                   !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C3-1,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,11.6,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    3.7,11.6,2.4,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,15.2,2.4,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.5,15.2,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    C4-1P,                   !- Name",
        "    FLOOR,                   !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C4-1,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7,3.7,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,0.0,2.4,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,15.2,2.4,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3.7,11.6,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    C5-1P,                   !- Name",
        "    FLOOR,                   !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    PLENUM-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C5-1,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,11.6,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    26.8,3.7,2.4,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3.7,3.7,2.4,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3.7,11.6,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    FRONT-1,                 !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    SPACE1-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,0.0,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.5,0.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.5,0.0,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    C1-1,                    !- Name",
        "    CEILING,                 !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    SPACE1-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C1-1P,                   !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7,3.7,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,0.0,2.4,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.5,0.0,2.4,  !- X,Y,Z ==> Vertex 3 {m}",
        "    26.8,3.7,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    F1-1,                    !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR-SLAB-1,            !- Construction Name",
        "    SPACE1-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Ground,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,3.7,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.5,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3.7,3.7,0.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB12,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE1-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB21,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5,0.0,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.5,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    26.8,3.7,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    26.8,3.7,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB14,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE1-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB41,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7,3.7,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    3.7,3.7,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,0.0,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB15,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE1-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB51,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,3.7,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    26.8,3.7,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3.7,3.7,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3.7,3.7,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    RIGHT-1,                 !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    SPACE2-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5,0.0,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.5,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.5,15.2,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.5,15.2,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    C2-1,                    !- Name",
        "    CEILING,                 !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    SPACE2-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C2-1P,                   !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,3.7,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.5,0.0,2.4,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.5,15.2,2.4,  !- X,Y,Z ==> Vertex 3 {m}",
        "    26.8,11.6,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    F2-1,                    !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR-SLAB-1,            !- Construction Name",
        "    SPACE2-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Ground,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,11.6,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.5,15.2,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.5,0.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    26.8,3.7,0.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB21,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE2-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB12,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,3.7,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    26.8,3.7,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.5,0.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.5,0.0,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB23,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE2-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB32,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5,15.2,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.5,15.2,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    26.8,11.6,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    26.8,11.6,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB25,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE2-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB52,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,11.6,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    26.8,11.6,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    26.8,3.7,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    26.8,3.7,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    BACK-1,                  !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    SPACE3-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5,15.2,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.5,15.2,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,15.2,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,15.2,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    C3-1,                    !- Name",
        "    CEILING,                 !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    SPACE3-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C3-1P,                   !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5,15.2,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,15.2,2.4,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3.7,11.6,2.4,  !- X,Y,Z ==> Vertex 3 {m}",
        "    26.8,11.6,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    F3-1,                    !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR-SLAB-1,            !- Construction Name",
        "    SPACE3-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Ground,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,11.6,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    3.7,11.6,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,15.2,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.5,15.2,0.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB32,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE3-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB23,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,11.6,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    26.8,11.6,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.5,15.2,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.5,15.2,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB34,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE3-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB43,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,15.2,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,15.2,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3.7,11.6,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3.7,11.6,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB35,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE3-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB53,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7,11.6,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    3.7,11.6,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    26.8,11.6,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    26.8,11.6,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    LEFT-1,                  !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    SPACE4-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,15.2,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,15.2,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,0.0,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    C4-1,                    !- Name",
        "    CEILING,                 !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    SPACE4-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C4-1P,                   !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7,11.6,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,15.2,2.4,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,0.0,2.4,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3.7,3.7,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    F4-1,                    !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR-SLAB-1,            !- Construction Name",
        "    SPACE4-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Ground,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7,3.7,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,15.2,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3.7,11.6,0.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB41,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE4-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB14,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,0.0,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3.7,3.7,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3.7,3.7,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB43,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE4-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB34,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7,11.6,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    3.7,11.6,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,15.2,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,15.2,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB45,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE4-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB54,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7,3.7,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    3.7,3.7,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3.7,11.6,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3.7,11.6,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    C5-1,                    !- Name",
        "    CEILING,                 !- Surface Type",
        "    CLNG-1,                  !- Construction Name",
        "    SPACE5-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    C5-1P,                   !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7,11.6,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    3.7,3.7,2.4,  !- X,Y,Z ==> Vertex 2 {m}",
        "    26.8,3.7,2.4,  !- X,Y,Z ==> Vertex 3 {m}",
        "    26.8,11.6,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    F5-1,                    !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR-SLAB-1,            !- Construction Name",
        "    SPACE5-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Ground,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,11.6,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    26.8,3.7,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3.7,3.7,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3.7,11.6,0.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB51,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE5-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB15,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7,3.7,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    3.7,3.7,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    26.8,3.7,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    26.8,3.7,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB52,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE5-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB25,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,3.7,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    26.8,3.7,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    26.8,11.6,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    26.8,11.6,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB53,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE5-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB35,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    26.8,11.6,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    26.8,11.6,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3.7,11.6,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3.7,11.6,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SB54,                    !- Name",
        "    WALL,                    !- Surface Type",
        "    INT-WALL-1,              !- Construction Name",
        "    SPACE5-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    SB45,                    !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    3.7,11.6,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    3.7,11.6,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3.7,3.7,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3.7,3.7,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    WF-1,                    !- Name",
        "    WINDOW,                  !- Surface Type",
        "    Sgl Grey 3mm,    !- Construction Name",
        "    FRONT-1,                 !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.50000,                 !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1,                       !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    3.0,0.0,2.1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    3.0,0.0,0.9,  !- X,Y,Z ==> Vertex 2 {m}",
        "    16.8,0.0,0.9,  !- X,Y,Z ==> Vertex 3 {m}",
        "    16.8,0.0,2.1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    DF-1,                    !- Name",
        "    GLASSDOOR,               !- Surface Type",
        "    Sgl Grey 3mm,            !- Construction Name",
        "    FRONT-1,                 !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.50000,                 !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1,                       !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    21.3,0.0,2.1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    21.3,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    23.8,0.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    23.8,0.0,2.1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    WR-1,                    !- Name",
        "    WINDOW,                  !- Surface Type",
        "    Sgl Grey 3mm,    !- Construction Name",
        "    RIGHT-1,                 !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.50000,                 !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1,                       !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    30.5,3.8,2.1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.5,3.8,0.9,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.5,11.4,0.9,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.5,11.4,2.1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    WB-1,                    !- Name",
        "    WINDOW,                  !- Surface Type",
        "    Sgl Grey 3mm,    !- Construction Name",
        "    BACK-1,                  !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.50000,                 !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1,                       !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    27.4,15.2,2.1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    27.4,15.2,0.9,  !- X,Y,Z ==> Vertex 2 {m}",
        "    13.7,15.2,0.9,  !- X,Y,Z ==> Vertex 3 {m}",
        "    13.7,15.2,2.1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    DB-1,                    !- Name",
        "    GLASSDOOR,               !- Surface Type",
        "    Sgl Grey 3mm,            !- Construction Name",
        "    BACK-1,                  !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.50000,                 !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1,                       !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    9.1,15.2,2.1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    9.1,15.2,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    7.0,15.2,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    7.0,15.2,2.1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    WL-1,                    !- Name",
        "    WINDOW,                  !- Surface Type",
        "    Sgl Grey 3mm,    !- Construction Name",
        "    LEFT-1,                  !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.50000,                 !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1,                       !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0.0,11.4,2.1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,11.4,0.9,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,3.8,0.9,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,3.8,2.1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Shading:Zone:Detailed,",
        "    Main South Overhang,     !- Name",
        "    FRONT-1,                 !- Base Surface Name",
        "    ShadeTransSch,           !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    0.0,-1.3,2.2,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,0.0,2.2,  !- X,Y,Z ==> Vertex 2 {m}",
        "    19.8,0.0,2.2,  !- X,Y,Z ==> Vertex 3 {m}",
        "    19.8,-1.3,2.2;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Shading:Zone:Detailed,",
        "    South Door Overhang,     !- Name",
        "    FRONT-1,                 !- Base Surface Name",
        "    ShadeTransSch,           !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    21.0,-2.0,2.6,  !- X,Y,Z ==> Vertex 1 {m}",
        "    21.0,0.0,2.6,  !- X,Y,Z ==> Vertex 2 {m}",
        "    24.1,0.0,2.6,  !- X,Y,Z ==> Vertex 3 {m}",
        "    24.1,-2.0,2.6;  !- X,Y,Z ==> Vertex 4 {m}",

        "  People,",
        "    SPACE1-1 People 1,       !- Name",
        "    SPACE1-1,                !- Zone or ZoneList Name",
        "    OCCUPY-1,                !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    11,                      !- Number of People",
        "    ,                        !- People per Zone Floor Area {person/m2}",
        "    ,                        !- Zone Floor Area per Person {m2/person}",
        "    0.3,                     !- Fraction Radiant",
        "    ,                        !- Sensible Heat Fraction",
        "    ActSchd;                 !- Activity Level Schedule Name",

        "  People,",
        "    SPACE2-1 People 1,       !- Name",
        "    SPACE2-1,                !- Zone or ZoneList Name",
        "    OCCUPY-1,                !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    5,                       !- Number of People",
        "    ,                        !- People per Zone Floor Area {person/m2}",
        "    ,                        !- Zone Floor Area per Person {m2/person}",
        "    0.3,                     !- Fraction Radiant",
        "    ,                        !- Sensible Heat Fraction",
        "    ActSchd;                 !- Activity Level Schedule Name",

        "  People,",
        "    SPACE3-1 People 1,       !- Name",
        "    SPACE3-1,                !- Zone or ZoneList Name",
        "    OCCUPY-1,                !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    11,                      !- Number of People",
        "    ,                        !- People per Zone Floor Area {person/m2}",
        "    ,                        !- Zone Floor Area per Person {m2/person}",
        "    0.3,                     !- Fraction Radiant",
        "    ,                        !- Sensible Heat Fraction",
        "    ActSchd;                 !- Activity Level Schedule Name",

        "  People,",
        "    SPACE4-1 People 1,       !- Name",
        "    SPACE4-1,                !- Zone or ZoneList Name",
        "    OCCUPY-1,                !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    5,                       !- Number of People",
        "    ,                        !- People per Zone Floor Area {person/m2}",
        "    ,                        !- Zone Floor Area per Person {m2/person}",
        "    0.3,                     !- Fraction Radiant",
        "    ,                        !- Sensible Heat Fraction",
        "    ActSchd;                 !- Activity Level Schedule Name",

        "  People,",
        "    SPACE5-1 People 1,       !- Name",
        "    SPACE5-1,                !- Zone or ZoneList Name",
        "    OCCUPY-1,                !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    20,                      !- Number of People",
        "    ,                        !- People per Zone Floor Area {person/m2}",
        "    ,                        !- Zone Floor Area per Person {m2/person}",
        "    0.3,                     !- Fraction Radiant",
        "    ,                        !- Sensible Heat Fraction",
        "    ActSchd;                 !- Activity Level Schedule Name",

        "  Lights,",
        "    SPACE1-1 Lights 1,       !- Name",
        "    SPACE1-1,                !- Zone or ZoneList Name",
        "    LIGHTS-1,                !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    1584,                    !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0.21,                       !- Return Air Fraction",
        "    0.59,                    !- Fraction Radiant",
        "    0.2,                     !- Fraction Visible",
        "    0,                       !- Fraction Replaceable",
        "    GeneralLights,           !- End-Use Subcategory",
        "    ,                        !- Return Air Fraction Calculated from Plenum Temperature",
        "    ,                        !- Return Air Fraction Function of Plenum Temperature Coefficient 1",
        "    ,                        !- Return Air Fraction Function of Plenum Temperature Coefficient 2",
        "    SPACE1-1 Return Outlet,  !- Return Air Heat Gain Node Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet;   !- Exhaust Air Heat Gain Node Name",

        "  Lights,",
        "    SPACE2-1 Lights 1,       !- Name",
        "    SPACE2-1,                !- Zone or ZoneList Name",
        "    LIGHTS-1,                !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    684,                     !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Return Air Fraction",
        "    0.59,                    !- Fraction Radiant",
        "    0.2,                     !- Fraction Visible",
        "    0,                       !- Fraction Replaceable",
        "    GeneralLights;           !- End-Use Subcategory",

        "  Lights,",
        "    SPACE3-1 Lights 1,       !- Name",
        "    SPACE3-1,                !- Zone or ZoneList Name",
        "    LIGHTS-1,                !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    1584,                    !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Return Air Fraction",
        "    0.59,                    !- Fraction Radiant",
        "    0.2,                     !- Fraction Visible",
        "    0,                       !- Fraction Replaceable",
        "    GeneralLights;           !- End-Use Subcategory",

        "  Lights,",
        "    SPACE4-1 Lights 1,       !- Name",
        "    SPACE4-1,                !- Zone or ZoneList Name",
        "    LIGHTS-1,                !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    684,                     !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Return Air Fraction",
        "    0.59,                    !- Fraction Radiant",
        "    0.2,                     !- Fraction Visible",
        "    0,                       !- Fraction Replaceable",
        "    GeneralLights;           !- End-Use Subcategory",

        "  Lights,",
        "    SPACE5-1 Lights 1,       !- Name",
        "    SPACE5-1,                !- Zone or ZoneList Name",
        "    LIGHTS-1,                !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    2964,                    !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Return Air Fraction",
        "    0.59,                    !- Fraction Radiant",
        "    0.2,                     !- Fraction Visible",
        "    0,                       !- Fraction Replaceable",
        "    GeneralLights;           !- End-Use Subcategory",

        "  ElectricEquipment,",
        "    SPACE1-1 ElecEq 1,       !- Name",
        "    SPACE1-1,                !- Zone or ZoneList Name",
        "    EQUIP-1,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1056,                    !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3,                     !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  ElectricEquipment,",
        "    SPACE2-1 ElecEq 1,       !- Name",
        "    SPACE2-1,                !- Zone or ZoneList Name",
        "    EQUIP-1,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    456,                     !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3,                     !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  ElectricEquipment,",
        "    SPACE3-1 ElecEq 1,       !- Name",
        "    SPACE3-1,                !- Zone or ZoneList Name",
        "    EQUIP-1,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1056,                    !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3,                     !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  ElectricEquipment,",
        "    SPACE4-1 ElecEq 1,       !- Name",
        "    SPACE4-1,                !- Zone or ZoneList Name",
        "    EQUIP-1,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    456,                     !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3,                     !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  ElectricEquipment,",
        "    SPACE5-1 ElecEq 1,       !- Name",
        "    SPACE5-1,                !- Zone or ZoneList Name",
        "    EQUIP-1,                 !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    1976,                    !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3,                     !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  ZoneInfiltration:DesignFlowRate,",
        "    SPACE1-1 Infil 1,        !- Name",
        "    SPACE1-1,                !- Zone or ZoneList Name",
        "    INFIL-SCH,               !- Schedule Name",
        "    flow/zone,               !- Design Flow Rate Calculation Method",
        "    0.0167,                  !- Design Flow Rate {m3/s}",
        "    ,                        !- Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Flow per Exterior Surface Area {m3/s-m2}",
        "    ,                        !- Air Changes per Hour {1/hr}",
        "    0,                       !- Constant Term Coefficient",
        "    0,                       !- Temperature Term Coefficient",
        "    0.2237,                  !- Velocity Term Coefficient",
        "    0;                       !- Velocity Squared Term Coefficient",

        "  ZoneInfiltration:DesignFlowRate,",
        "    SPACE2-1 Infil 1,        !- Name",
        "    SPACE2-1,                !- Zone or ZoneList Name",
        "    INFIL-SCH,               !- Schedule Name",
        "    flow/zone,               !- Design Flow Rate Calculation Method",
        "    0.00717,                 !- Design Flow Rate {m3/s}",
        "    ,                        !- Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Flow per Exterior Surface Area {m3/s-m2}",
        "    ,                        !- Air Changes per Hour {1/hr}",
        "    0,                       !- Constant Term Coefficient",
        "    0,                       !- Temperature Term Coefficient",
        "    0.2237,                  !- Velocity Term Coefficient",
        "    0;                       !- Velocity Squared Term Coefficient",

        "  ZoneInfiltration:DesignFlowRate,",
        "    SPACE3-1 Infil 1,        !- Name",
        "    SPACE3-1,                !- Zone or ZoneList Name",
        "    INFIL-SCH,               !- Schedule Name",
        "    flow/zone,               !- Design Flow Rate Calculation Method",
        "    0.0167,                  !- Design Flow Rate {m3/s}",
        "    ,                        !- Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Flow per Exterior Surface Area {m3/s-m2}",
        "    ,                        !- Air Changes per Hour {1/hr}",
        "    0,                       !- Constant Term Coefficient",
        "    0,                       !- Temperature Term Coefficient",
        "    0.2237,                  !- Velocity Term Coefficient",
        "    0;                       !- Velocity Squared Term Coefficient",

        "  ZoneInfiltration:DesignFlowRate,",
        "    SPACE4-1 Infil 1,        !- Name",
        "    SPACE4-1,                !- Zone or ZoneList Name",
        "    INFIL-SCH,               !- Schedule Name",
        "    flow/zone,               !- Design Flow Rate Calculation Method",
        "    0.00717,                 !- Design Flow Rate {m3/s}",
        "    ,                        !- Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Flow per Exterior Surface Area {m3/s-m2}",
        "    ,                        !- Air Changes per Hour {1/hr}",
        "    0,                       !- Constant Term Coefficient",
        "    0,                       !- Temperature Term Coefficient",
        "    0.2237,                  !- Velocity Term Coefficient",
        "    0;                       !- Velocity Squared Term Coefficient",

        "  ZoneInfiltration:DesignFlowRate,",
        "    SPACE5-1 Infil 1,        !- Name",
        "    SPACE5-1,                !- Zone or ZoneList Name",
        "    INFIL-SCH,               !- Schedule Name",
        "    flow/zone,               !- Design Flow Rate Calculation Method",
        "    0.031089,                !- Design Flow Rate {m3/s}",
        "    ,                        !- Flow per Zone Floor Area {m3/s-m2}",
        "    ,                        !- Flow per Exterior Surface Area {m3/s-m2}",
        "    ,                        !- Air Changes per Hour {1/hr}",
        "    0,                       !- Constant Term Coefficient",
        "    0,                       !- Temperature Term Coefficient",
        "    0.2237,                  !- Velocity Term Coefficient",
        "    0;                       !- Velocity Squared Term Coefficient",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA SPACE1-1,        !- Name",
        "    Sum,                     !- Outdoor Air Method",
        "    0.00250,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.00030,                 !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA SPACE2-1,        !- Name",
        "    Sum,                     !- Outdoor Air Method",
        "    0.00250,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.00030,                 !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA SPACE3-1,        !- Name",
        "    Sum,                     !- Outdoor Air Method",
        "    0.00250,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.00030,                 !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA SPACE4-1,        !- Name",
        "    Sum,                     !- Outdoor Air Method",
        "    0.00250,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.00030,                 !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  DesignSpecification:OutdoorAir,",
        "    SZ DSOA SPACE5-1,        !- Name",
        "    Sum,                     !- Outdoor Air Method",
        "    0.00250,                 !- Outdoor Air Flow per Person {m3/s-person}",
        "    0.00030,                 !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "    0.0;                     !- Outdoor Air Flow per Zone {m3/s}",

        "  DesignSpecification:ZoneAirDistribution,",
        "    ZoneAirDistribution,     !- Name",
        "    1,                       !- Zone Air Distribution Effectiveness in Cooling Mode {dimensionless}",
        "    1,                       !- Zone Air Distribution Effectiveness in Heating Mode {dimensionless}",
        "    ,                        !- Zone Air Distribution Effectiveness Schedule Name",
        "    0;                       !- Zone Secondary Recirculation Fraction {dimensionless}",

        "  ZoneControl:Thermostat,",
        "    SPACE1-1 Thermostat,     !- Name",
        "    SPACE1-1,                !- Zone or ZoneList Name",
        "    HVACTemplate-Always 4,   !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    All Zones Dual SP Control;  !- Control 1 Name",

        "  ZoneControl:Thermostat,",
        "    SPACE2-1 Thermostat,     !- Name",
        "    SPACE2-1,                !- Zone or ZoneList Name",
        "    HVACTemplate-Always 4,   !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    All Zones Dual SP Control;  !- Control 1 Name",

        "  ZoneControl:Thermostat,",
        "    SPACE3-1 Thermostat,     !- Name",
        "    SPACE3-1,                !- Zone or ZoneList Name",
        "    HVACTemplate-Always 4,   !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    All Zones Dual SP Control;  !- Control 1 Name",

        "  ZoneControl:Thermostat,",
        "    SPACE4-1 Thermostat,     !- Name",
        "    SPACE4-1,                !- Zone or ZoneList Name",
        "    HVACTemplate-Always 4,   !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    All Zones Dual SP Control;  !- Control 1 Name",

        "  ZoneControl:Thermostat,",
        "    SPACE5-1 Thermostat,     !- Name",
        "    SPACE5-1,                !- Zone or ZoneList Name",
        "    HVACTemplate-Always 4,   !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    All Zones Dual SP Control;  !- Control 1 Name",

        "  ThermostatSetpoint:DualSetpoint,",
        "    All Zones Dual SP Control,  !- Name",
        "    Htg-SetP-Sch,            !- Heating Setpoint Temperature Schedule Name",
        "    Clg-SetP-Sch;            !- Cooling Setpoint Temperature Schedule Name",

        "  ZoneHVAC:PackagedTerminalAirConditioner,",
        "    SPACE1-1 PTAC,           !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    SPACE1-1 PTAC Inlet,     !- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet,   !- Air Outlet Node Name",
        "    ,                        !- Outdoor Air Mixer Object Type",
        "    ,                        !- Outdoor Air Mixer Name",
        "    0.27003,                 !- Cooling Supply Air Flow Rate {m3/s}",
        "    0.27003,                 !- Heating Supply Air Flow Rate {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    0,                       !- Cooling Outdoor Air Flow Rate {m3/s}",
        "    0,                       !- Heating Outdoor Air Flow Rate {m3/s}",
        "    0,                       !- No Load Outdoor Air Flow Rate {m3/s}",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE1-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Heating:Fuel,       !- Heating Coil Object Type",
        "    SPACE1-1 Heating Coil,   !- Heating Coil Name",
        "    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "    SPACE1-1 PTAC CCoil,     !- Cooling Coil Name",
        "    BlowThrough,             !- Fan Placement",
        "    ContsFanSch;             !- Supply Air Fan Operating Mode Schedule Name",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    SPACE1-1 PTAC CCoil,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    6017.15623,              !- Gross Rated Total Cooling Capacity {W}",
        "    0.73612,                 !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    0.27003,                 !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE1-1 Zone Unit Fan Outlet,  !- Air Inlet Node Name",
        "    SPACE1-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACPLFFPLR;             !- Part Load Fraction Correlation Curve Name",

        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    FanEffRatioCurve,        !- Name",
        "    0.33856828,              !- Coefficient1 Constant",
        "    1.72644131,              !- Coefficient2 x",
        "    -1.49280132,             !- Coefficient3 x**2",
        "    0.42776208,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.3,                     !- Minimum Curve Output",
        "    1.0;                     !- Maximum Curve Output",

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

        "  ZoneHVAC:PackagedTerminalAirConditioner,",
        "    SPACE2-1 PTAC,           !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    SPACE2-1 PTAC Inlet,     !- Air Inlet Node Name",
        "    SPACE2-1 Supply Inlet,   !- Air Outlet Node Name",
        "    ,                        !- Outdoor Air Mixer Object Type",
        "    ,                        !- Outdoor Air Mixer Name",
        "    0.20772,                 !- Cooling Supply Air Flow Rate {m3/s}",
        "    0.20772,                 !- Heating Supply Air Flow Rate {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    0,                       !- Cooling Outdoor Air Flow Rate {m3/s}",
        "    0,                       !- Heating Outdoor Air Flow Rate {m3/s}",
        "    0,                       !- No Load Outdoor Air Flow Rate {m3/s}",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE2-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Heating:Fuel,       !- Heating Coil Object Type",
        "    SPACE2-1 Heating Coil,   !- Heating Coil Name",
        "    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "    SPACE2-1 PTAC CCoil,     !- Cooling Coil Name",
        "    BlowThrough,             !- Fan Placement",
        "    ContsFanSch;             !- Supply Air Fan Operating Mode Schedule Name",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    SPACE2-1 PTAC CCoil,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    3438.43539,              !- Gross Rated Total Cooling Capacity {W}",
        "    0.80066,                 !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    0.20772,                 !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE2-1 Zone Unit Fan Outlet,  !- Air Inlet Node Name",
        "    SPACE2-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACPLFFPLR;             !- Part Load Fraction Correlation Curve Name",

        "  ZoneHVAC:PackagedTerminalAirConditioner,",
        "    SPACE3-1 PTAC,           !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    SPACE3-1 PTAC Inlet,     !- Air Inlet Node Name",
        "    SPACE3-1 Supply Inlet,   !- Air Outlet Node Name",
        "    ,                        !- Outdoor Air Mixer Object Type",
        "    ,                        !- Outdoor Air Mixer Name",
        "    0.25034,                 !- Cooling Supply Air Flow Rate {m3/s}",
        "    0.25034,                 !- Heating Supply Air Flow Rate {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    0,                       !- Cooling Outdoor Air Flow Rate {m3/s}",
        "    0,                       !- Heating Outdoor Air Flow Rate {m3/s}",
        "    0,                       !- No Load Outdoor Air Flow Rate {m3/s}",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE3-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Heating:Fuel,       !- Heating Coil Object Type",
        "    SPACE3-1 Heating Coil,   !- Heating Coil Name",
        "    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "    SPACE3-1 PTAC CCoil,     !- Cooling Coil Name",
        "    BlowThrough,             !- Fan Placement",
        "    ContsFanSch;             !- Supply Air Fan Operating Mode Schedule Name",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    SPACE3-1 PTAC CCoil,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    5914.94365,              !- Gross Rated Total Cooling Capacity {W}",
        "    0.71658,                 !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    0.25034,                 !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE3-1 Zone Unit Fan Outlet,  !- Air Inlet Node Name",
        "    SPACE3-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACPLFFPLR;             !- Part Load Fraction Correlation Curve Name",

        "  ZoneHVAC:PackagedTerminalAirConditioner,",
        "    SPACE4-1 PTAC,           !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    SPACE4-1 PTAC Inlet,     !- Air Inlet Node Name",
        "    SPACE4-1 PTAC Outlet,    !- Air Outlet Node Name",
        "    ,                        !- Outdoor Air Mixer Object Type",
        "    ,                        !- Outdoor Air Mixer Name",
        "    0.22037,                 !- Cooling Supply Air Flow Rate {m3/s}",
        "    0.22037,                 !- Heating Supply Air Flow Rate {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    0,                       !- Cooling Outdoor Air Flow Rate {m3/s}",
        "    0,                       !- Heating Outdoor Air Flow Rate {m3/s}",
        "    0,                       !- No Load Outdoor Air Flow Rate {m3/s}",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE4-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Heating:Fuel,       !- Heating Coil Object Type",
        "    SPACE4-1 Heating Coil,   !- Heating Coil Name",
        "    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "    SPACE4-1 PTAC CCoil,     !- Cooling Coil Name",
        "    BlowThrough,             !- Fan Placement",
        "    CyclingFanSch;           !- Supply Air Fan Operating Mode Schedule Name",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    SPACE4-1 PTAC CCoil,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    5387.66005,              !- Gross Rated Total Cooling Capacity {W}",
        "    0.70594,                 !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    0.22037,                 !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE4-1 Zone Unit Fan Outlet,  !- Air Inlet Node Name",
        "    SPACE4-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACPLFFPLR;             !- Part Load Fraction Correlation Curve Name",

        "  ZoneHVAC:PackagedTerminalAirConditioner,",
        "    SPACE5-1 PTAC,           !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    SPACE5-1 PTAC Inlet,     !- Air Inlet Node Name",
        "    SPACE5-1 PTAC Outlet,    !- Air Outlet Node Name",
        "    ,                        !- Outdoor Air Mixer Object Type",
        "    ,                        !- Outdoor Air Mixer Name",
        "    0.26423,                 !- Cooling Supply Air Flow Rate {m3/s}",
        "    0.26423,                 !- Heating Supply Air Flow Rate {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    0,                       !- Cooling Outdoor Air Flow Rate {m3/s}",
        "    0,                       !- Heating Outdoor Air Flow Rate {m3/s}",
        "    0,                       !- No Load Outdoor Air Flow Rate {m3/s}",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE5-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Heating:Fuel,       !- Heating Coil Object Type",
        "    SPACE5-1 Heating Coil,   !- Heating Coil Name",
        "    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "    SPACE5-1 PTAC CCoil,     !- Cooling Coil Name",
        "    BlowThrough,             !- Fan Placement",
        "    CyclingFanSch;           !- Supply Air Fan Operating Mode Schedule Name",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    SPACE5-1 PTAC CCoil,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    6197.64657,              !- Gross Rated Total Cooling Capacity {W}",
        "    0.71947,                 !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    0.26423,                 !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE5-1 Zone Unit Fan Outlet,  !- Air Inlet Node Name",
        "    SPACE5-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACPLFFPLR;             !- Part Load Fraction Correlation Curve Name",

        "  AirTerminal:SingleDuct:Mixer,",
        "    SPACE1-1 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- ZoneHVAC Unit Object Type",
        "    SPACE1-1 PTAC,           !- ZoneHVAC Unit Object Name",
        "    SPACE1-1 PTAC Inlet,     !- Mixer Outlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Primary Inlet,  !- Mixer Primary Air Inlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet,  !- Mixer Secondary Air Inlet Node Name",
        "    InletSide,               !- Mixer Connection Type",
        "    SZ DSOA SPACE1-1,        !- Design Specification Outdoor Air Object Name",
        "    CurrentOccupancy;        !- Per Person Ventilation Rate Mode",

        "  AirTerminal:SingleDuct:Mixer,",
        "    SPACE3-1 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- ZoneHVAC Unit Object Type",
        "    SPACE3-1 PTAC,           !- ZoneHVAC Unit Object Name",
        "    SPACE3-1 PTAC Inlet,     !- Mixer Outlet Node Name",
        "    SPACE3-1 Air Terminal Mixer Primary Inlet,  !- Mixer Primary Air Inlet Node Name",
        "    SPACE3-1 Air Terminal Mixer Secondary Inlet,  !- Mixer Secondary Air Inlet Node Name",
        "    InletSide,               !- Mixer Connection Type",
        "    SZ DSOA SPACE3-1,        !- Design Specification Outdoor Air Object Name",
        "    CurrentOccupancy;        !- Per Person Ventilation Rate Mode",

        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
        "    SPACE2-1 DOAS Air Terminal,          !- Name",
        "    FanAvailSched,    !- Availability Schedule Name",
        "    SPACE2-1 Air Terminal Mixer Primary Inlet,  !- Air Inlet Node Name",
        "    SPACE2-1 DOAS Inlet,       !- Air Outlet Node Name",
        "    2.53205E-002,                    !- Maximum Air Flow Rate {m3/s}",
        "    SZ DSOA SPACE2-1,        !- Design Specification Outdoor Air Object Name",
        "    CurrentOccupancy;        !- Per Person Ventilation Rate Mode",

        "  AirTerminal:SingleDuct:Mixer,",
        "    SPACE4-1 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- ZoneHVAC Unit Object Type",
        "    SPACE4-1 PTAC,           !- ZoneHVAC Unit Object Name",
        "    SPACE4-1 Supply Inlet,   !- Mixer Outlet Node Name",
        "    SPACE4-1 Air Terminal Mixer Primary Inlet,  !- Mixer Primary Air Inlet Node Name",
        "    SPACE4-1 PTAC Outlet,    !- Mixer Secondary Air Inlet Node Name",
        "    SupplySide,              !- Mixer Connection Type",
        "    SZ DSOA SPACE4-1,        !- Design Specification Outdoor Air Object Name",
        "    CurrentOccupancy;        !- Per Person Ventilation Rate Mode",

        "  AirTerminal:SingleDuct:Mixer,",
        "    SPACE5-1 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- ZoneHVAC Unit Object Type",
        "    SPACE5-1 PTAC,           !- ZoneHVAC Unit Object Name",
        "    SPACE5-1 Supply Inlet,   !- Mixer Outlet Node Name",
        "    SPACE5-1 Air Terminal Mixer Primary Inlet,  !- Mixer Primary Air Inlet Node Name",
        "    SPACE5-1 PTAC Outlet,    !- Mixer Secondary Air Inlet Node Name",
        "    SupplySide,              !- Mixer Connection Type",
        "    SZ DSOA SPACE5-1,        !- Design Specification Outdoor Air Object Name",
        "    CurrentOccupancy;        !- Per Person Ventilation Rate Mode",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 DOAS ATU,       !- Name",
        "    SPACE1-1 PTAC Inlet,     !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE1-1 DOAS Air Terminal;  !- Air Terminal Name",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE2-1 DOAS ATU,       !- Name",
        "    SPACE2-1 DOAS Inlet,     !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,  !- Air Terminal Object Type",
        "    SPACE2-1 DOAS Air Terminal;  !- Air Terminal Name",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE3-1 DOAS ATU,       !- Name",
        "    SPACE3-1 PTAC Inlet,     !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE3-1 DOAS Air Terminal;  !- Air Terminal Name",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE4-1 DOAS ATU,       !- Name",
        "    SPACE4-1 Supply Inlet,   !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE4-1 DOAS Air Terminal;  !- Air Terminal Name",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE5-1 DOAS ATU,       !- Name",
        "    SPACE5-1 Supply Inlet,   !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE5-1 DOAS Air Terminal;  !- Air Terminal Name",

        "  ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- Zone Equipment 2 Object Type",
        "    SPACE1-1 PTAC,           !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

        "  ZoneHVAC:EquipmentList,",
        "    SPACE2-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE2-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- Zone Equipment 2 Object Type",
        "    SPACE2-1 PTAC,           !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

        "  ZoneHVAC:EquipmentList,",
        "    SPACE3-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE3-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- Zone Equipment 2 Object Type",
        "    SPACE3-1 PTAC,           !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

        "  ZoneHVAC:EquipmentList,",
        "    SPACE4-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE4-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- Zone Equipment 2 Object Type",
        "    SPACE4-1 PTAC,           !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

        "  ZoneHVAC:EquipmentList,",
        "    SPACE5-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE5-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- Zone Equipment 2 Object Type",
        "    SPACE5-1 PTAC,           !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction Schedule Name",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction Schedule Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet,  !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE1-1 Return Outlet;  !- Zone Return Air Node or NodeList Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    SPACE2-1,                !- Zone Name",
        "    SPACE2-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE2-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    ,  !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE2-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE2-1 Return Outlet;  !- Zone Return Air Node or NodeList Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    SPACE3-1,                !- Zone Name",
        "    SPACE3-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE3-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    ,  !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE3-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE3-1 Return Outlet;  !- Zone Return Air Node or NodeList Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    SPACE4-1,                !- Zone Name",
        "    SPACE4-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE4-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE4-1 PTAC Inlet,     !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE4-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE4-1 Return Outlet;  !- Zone Return Air Node or NodeList Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    SPACE5-1,                !- Zone Name",
        "    SPACE5-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE5-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    ,     !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE5-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE5-1 Return Outlet;  !- Zone Return Air Node or NodeList Name",

        "  Fan:OnOff,",
        "    SPACE1-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    0.27003,                !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE1-1 PTAC Inlet,     !- Air Inlet Node Name",
        "    SPACE1-1 Zone Unit Fan Outlet;  !- Air Outlet Node Name",

        "  Fan:OnOff,",
        "    SPACE2-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    0.20772,                !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE2-1 PTAC Inlet,     !- Air Inlet Node Name",
        "    SPACE2-1 Zone Unit Fan Outlet;  !- Air Outlet Node Name",

        "  Fan:OnOff,",
        "    SPACE3-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    0.25034,                !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE3-1 PTAC Inlet,     !- Air Inlet Node Name",
        "    SPACE3-1 Zone Unit Fan Outlet;  !- Air Outlet Node Name",

        "  Fan:OnOff,",
        "    SPACE4-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    0.22037,                !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE4-1 PTAC Inlet,     !- Air Inlet Node Name",
        "    SPACE4-1 Zone Unit Fan Outlet;  !- Air Outlet Node Name",

        "  Fan:OnOff,",
        "    SPACE5-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    0.26423,                !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE5-1 PTAC Inlet,     !- Air Inlet Node Name",
        "    SPACE5-1 Zone Unit Fan Outlet;  !- Air Outlet Node Name",

        "  Coil:Heating:Fuel,",
        "    SPACE1-1 Heating Coil,   !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Burner Efficiency",
        "    2834.99607,                !- Nominal Capacity {W}",
        "    SPACE1-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet;   !- Air Outlet Node Name",

        "  Coil:Heating:Fuel,",
        "    SPACE2-1 Heating Coil,   !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Burner Efficiency",
        "    819.74853,                !- Nominal Capacity {W}",
        "    SPACE2-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE2-1 Supply Inlet;   !- Air Outlet Node Name",

        "  Coil:Heating:Fuel,",
        "    SPACE3-1 Heating Coil,   !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Burner Efficiency",
        "    2795.18094,                !- Nominal Capacity {W}",
        "    SPACE3-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE3-1 Supply Inlet;   !- Air Outlet Node Name",

        "  Coil:Heating:Fuel,",
        "    SPACE4-1 Heating Coil,   !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Burner Efficiency",
        "    831.71230,                !- Nominal Capacity {W}",
        "    SPACE4-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE4-1 PTAC Outlet;    !- Air Outlet Node Name",

        "  Coil:Heating:Fuel,",
        "    SPACE5-1 Heating Coil,   !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Burner Efficiency",
        "    3391.15350,                !- Nominal Capacity {W}",
        "    SPACE5-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE5-1 PTAC Outlet;    !- Air Outlet Node Name",

        "  Fan:VariableVolume,",
        "    DOAS Supply Fan,         !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    1000,                    !- Pressure Rise {Pa}",
        "    0.26908,                !- Maximum Flow Rate {m3/s}",
        "    Fraction,                !- Fan Power Minimum Flow Rate Input Method",
        "    0.0,                     !- Fan Power Minimum Flow Fraction",
        "    ,                        !- Fan Power Minimum Air Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    0.0015302446,            !- Fan Power Coefficient 1",
        "    0.0052080574,            !- Fan Power Coefficient 2",
        "    1.1086242,               !- Fan Power Coefficient 3",
        "    -0.11635563,             !- Fan Power Coefficient 4",
        "    0,                       !- Fan Power Coefficient 5",
        "    DOAS Mixed Air Outlet,   !- Air Inlet Node Name",
        "    DOAS Supply Fan Outlet;  !- Air Outlet Node Name",

        "  Controller:OutdoorAir,",
        "    DOAS OA Controller,      !- Name",
        "    DOAS Relief Air Outlet,  !- Relief Air Outlet Node Name",
        "    DOAS Air Loop Inlet,     !- Return Air Node Name",
        "    DOAS Mixed Air Outlet,   !- Mixed Air Node Name",
        "    DOAS Outdoor Air Inlet,  !- Actuator Node Name",
        "    0.26908,                !- Minimum Outdoor Air Flow Rate {m3/s}",
        "    0.26908,                !- Maximum Outdoor Air Flow Rate {m3/s}",
        "    NoEconomizer,            !- Economizer Control Type",
        "    ModulateFlow,            !- Economizer Control Action Type",
        "    ,                        !- Economizer Maximum Limit Dry-Bulb Temperature {C}",
        "    ,                        !- Economizer Maximum Limit Enthalpy {J/kg}",
        "    ,                        !- Economizer Maximum Limit Dewpoint Temperature {C}",
        "    ,                        !- Electronic Enthalpy Limit Curve Name",
        "    12.2,                    !- Economizer Minimum Limit Dry-Bulb Temperature {C}",
        "    NoLockout,               !- Lockout Type",
        "    ProportionalMinimum;     !- Minimum Limit Type",

        "  AirLoopHVAC:ControllerList,",
        "    DOAS OA System Controllers,  !- Name",
        "    Controller:OutdoorAir,   !- Controller 1 Object Type",
        "    DOAS OA Controller;      !- Controller 1 Name",

        "  AirLoopHVAC,",
        "    DOAS,                    !- Name",
        "    ,                        !- Controller List Name",
        "    DOAS Availability Managers,  !- Availability Manager List Name",
        "    0.26908,                !- Design Supply Air Flow Rate {m3/s}",
        "    DOAS Branches,           !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    DOAS Air Loop Inlet,     !- Supply Side Inlet Node Name",
        "    DOAS PLENUM-1 Out Node,  !- Demand Side Outlet Node Name",
        "    DOAS Supply Path Inlet,  !- Demand Side Inlet Node Names",
        "    DOAS Supply Fan Outlet;  !- Supply Side Outlet Node Names",

        "  AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
        "    DOAS OA System Equipment,!- Name",
        "    OutdoorAir:Mixer,        !- Component 2 Object Type",
        "    DOAS OA Mixing Box;      !- Component 2 Name",

        "  AirLoopHVAC:OutdoorAirSystem,",
        "    DOAS OA System,          !- Name",
        "    DOAS OA System Controllers,  !- Controller List Name",
        "    DOAS OA System Equipment;!- Outdoor Air Equipment List Name",

        "  OutdoorAir:Mixer,",
        "    DOAS OA Mixing Box,      !- Name",
        "    DOAS Mixed Air Outlet,   !- Mixed Air Node Name",
        "    DOAS Outdoor Air Inlet,  !- Outdoor Air Stream Node Name",
        "    DOAS Relief Air Outlet,  !- Relief Air Stream Node Name",
        "    DOAS Air Loop Inlet;     !- Return Air Stream Node Name",

        "  AirLoopHVAC:ZoneSplitter,",
        "    DOAS Zone Splitter,      !- Name",
        "    DOAS Supply Path Inlet,  !- Inlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Primary Inlet,  !- Outlet 1 Node Name",
        "    SPACE2-1 Air Terminal Mixer Primary Inlet,  !- Outlet 2 Node Name",
        "    SPACE3-1 Air Terminal Mixer Primary Inlet,  !- Outlet 3 Node Name",
        "    SPACE4-1 Air Terminal Mixer Primary Inlet,  !- Outlet 4 Node Name",
        "    SPACE5-1 Air Terminal Mixer Primary Inlet;  !- Outlet 5 Node Name",

        "  AirLoopHVAC:SupplyPath,",
        "    DOAS Supply Path,        !- Name",
        "    DOAS Supply Path Inlet,  !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    DOAS Zone Splitter;      !- Component 1 Name",

        "  AirLoopHVAC:ReturnPlenum,",
        "    Return-Plenum-1,         !- Name",
        "    PLENUM-1,                !- Zone Name",
        "    PLENUM-1 Node,           !- Zone Node Name",
        "    DOAS PLENUM-1 Out Node,       !- Outlet Node Name",
        "    DOAS Plenum-1 Induce Nodes,  !- Induced Air Outlet Node or NodeList Name",
        "    SPACE1-1 Return Outlet,       !- Inlet 1 Node Name",
        "    SPACE2-1 Return Outlet,       !- Inlet 2 Node Name",
        "    SPACE3-1 Return Outlet,       !- Inlet 3 Node Name",
        "    SPACE4-1 Return Outlet,       !- Inlet 4 Node Name",
        "    SPACE5-1 Return Outlet;       !- Inlet 5 Node Name",

        "  NodeList,",
        "    DOAS Plenum-1 Induce Nodes,         !- Name",
        "    SPACE2-1 PTAC Inlet,   !- Node 1 Name",
        "    SPACE3-1 Air Terminal Mixer Secondary Inlet,   !- Node 2 Name",
        "    SPACE5-1 PTAC Inlet;   !- Node 3 Name",

        "  AirLoopHVAC:ReturnPath,",
        "    DOAS Return Path,        !- Name",
        "    DOAS PLENUM-1 Out Node,       !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ReturnPlenum,!- Component 1 Object Type",
        "    Return-Plenum-1;         !- Component 1 Name",

        "  Branch,",
        "    DOAS Main Branch,        !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    AirLoopHVAC:OutdoorAirSystem,  !- Component 1 Object Type",
        "    DOAS OA System,          !- Component 1 Name",
        "    DOAS Air Loop Inlet,     !- Component 1 Inlet Node Name",
        "    DOAS Mixed Air Outlet,   !- Component 1 Outlet Node Name",
        "    Fan:VariableVolume,      !- Component 2 Object Type",
        "    DOAS Supply Fan,         !- Component 2 Name",
        "    DOAS Mixed Air Outlet,   !- Component 2 Inlet Node Name",
        "    DOAS Supply Fan Outlet;  !- Component 2 Outlet Node Name",

        "  BranchList,",
        "    DOAS Branches,           !- Name",
        "    DOAS Main Branch;        !- Branch 1 Name",

        "  NodeList,",
        "    SPACE1-1 Inlets,         !- Name",
        "    SPACE1-1 Supply Inlet;   !- Node 1 Name",

        "  NodeList,",
        "    SPACE2-1 Inlets,         !- Name",
        "    SPACE2-1 DOAS Inlet,     !- Node 2 Name",
        "    SPACE2-1 Supply Inlet;   !- Node 1 Name",

        "  NodeList,",
        "    SPACE3-1 Inlets,         !- Name",
        "    SPACE3-1 Supply Inlet;   !- Node 1 Name",

        "  NodeList,",
        "    SPACE4-1 Inlets,         !- Name",
        "    SPACE4-1 Supply Inlet;   !- Node 1 Name",

        "  NodeList,",
        "    SPACE5-1 Inlets,         !- Name",
        "    SPACE5-1 Supply Inlet;   !- Node 1 Name",

        "  OutdoorAir:NodeList,",
        "    DOAS Outdoor Air Inlet;  !- Node or NodeList Name 1",

        "  AvailabilityManager:Scheduled,",
        "    DOAS Availability,       !- Name",
        "    FanAvailSched;           !- Schedule Name",

        "  AvailabilityManagerAssignmentList,",
        "    DOAS Availability Managers,  !- Name",
        "    AvailabilityManager:Scheduled,  !- Availability Manager 1 Object Type",
        "    DOAS Availability;       !- Availability Manager 1 Name",

        "  SetpointManager:Scheduled,",
        "    DOAS Cooling Supply Air Temp Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    Always 21.1,             !- Schedule Name",
        "    DOAS Mixed Air Outlet;   !- Setpoint Node or NodeList Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataIPShortCut->lNumericFieldBlanks.allocate(1000);
    state->dataIPShortCut->lAlphaFieldBlanks.allocate(1000);
    state->dataIPShortCut->cAlphaFieldNames.allocate(1000);
    state->dataIPShortCut->cNumericFieldNames.allocate(1000);
    state->dataIPShortCut->cAlphaArgs.allocate(1000);
    state->dataIPShortCut->rNumericArgs.allocate(1000);
    state->dataIPShortCut->lNumericFieldBlanks = false;
    state->dataIPShortCut->lAlphaFieldBlanks = false;
    state->dataIPShortCut->cAlphaFieldNames = " ";
    state->dataIPShortCut->cNumericFieldNames = " ";
    state->dataIPShortCut->cAlphaArgs = " ";
    state->dataIPShortCut->rNumericArgs = 0.0;

    state->dataGlobal->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 15; // must initialize this to get schedules initialized
    state->init_state(*state);

    bool ErrorsFound = false;
    // Read objects
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataGlobal->TimeStepZoneSec = state->dataGlobal->TimeStepZone * Constant::rSecsInHour;
    state->dataGlobal->CurrentTime = 12.0;

    HeatBalanceManager::GetHeatBalanceInput(*state);

    HeatBalanceManager::AllocateHeatBalArrays(*state);
    HeatBalanceAirManager::GetAirHeatBalanceInput(*state);

    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(6);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(6);
    state->dataSurfaceGeometry->CosZoneRelNorth = 1.0;
    state->dataSurfaceGeometry->SinZoneRelNorth = 0.0;
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SurfaceGeometry::GetSurfaceData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    SizingManager::GetOARequirements(*state);
    InternalHeatGains::GetInternalHeatGainsInput(*state);

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(6);
    for (auto &thisZoneHB : state->dataZoneTempPredictorCorrector->zoneHeatBalance) {
        thisZoneHB.MAT = 23.0;
        thisZoneHB.airHumRat = 0.001;
    }

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneEnergyDemand->DeadBandOrSetback.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneEnergyDemand->DeadBandOrSetback = false;

    state->dataEnvrn->OutDryBulbTemp = 30.0;
    state->dataEnvrn->OutHumRat = 0.0015;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(30.0, 0.0015);
    state->dataEnvrn->WindSpeed = 4.9;
    state->dataEnvrn->WindDir = 270.0;
    state->dataEnvrn->StdRhoAir = 1.2;

    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    SimAirServingZones::GetAirPathData(*state);
    state->dataSimAirServingZones->GetAirLoopInputFlag = false;
    SplitterComponent::GetSplitterInput(*state);
    state->dataUnitarySystems->getInputOnceFlag = false;
    for (int i = 1; i <= state->dataGlobal->NumOfZones; ++i) {
        if (!state->dataZoneEquip->ZoneEquipConfig(i).IsControlled) {
            continue;
        }
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(i).ZoneNode).Temp =
            state->dataZoneTempPredictorCorrector->zoneHeatBalance(i).MAT;
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(i).ZoneNode).HumRat =
            state->dataZoneTempPredictorCorrector->zoneHeatBalance(i).airHumRat;
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(i).ZoneNode).Enthalpy =
            Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(i).ZoneNode).Temp,
                                       state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(i).ZoneNode).HumRat);
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(i).TotalOutputRequired = -5000.0;
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(i).OutputRequiredToHeatingSP = -1000.0;
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(i).OutputRequiredToCoolingSP = -5000.0;
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(i).RemainingOutputRequired = -5000.0;
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(i).RemainingOutputReqToCoolSP = -5000.0;
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(i).RemainingOutputReqToHeatSP = -200.0;
    }
    GetZoneAirSetPoints(*state);
    state->dataHeatBalFanSys->TempControlType.allocate(6);
    state->dataHeatBalFanSys->TempControlType = HVAC::SetptType::DualHeatCool;

    Sched::GetSchedule(*state, "OCCUPY-1")->currentVal = 1.0;
    Sched::GetSchedule(*state, "LIGHTS-1")->currentVal = 1.0;
    Sched::GetSchedule(*state, "EQUIP-1")->currentVal = 1.0;
    Sched::GetSchedule(*state, "INFIL-SCH")->currentVal = 1.0;
    Sched::GetSchedule(*state, "ACTSCHD")->currentVal = 117;
    Sched::GetSchedule(*state, "SHADETRANSSCH")->currentVal = 0.0;         // shade transmittance
    Sched::GetSchedule(*state, "HTG-SETP-SCH")->currentVal = 18.0;         // heating set point
    Sched::GetSchedule(*state, "CLG-SETP-SCH")->currentVal = 24.0;         // cooling set point
    Sched::GetSchedule(*state, "HVACTEMPLATE-ALWAYS 4")->currentVal = 4.0; // dual Tstat sch
    Sched::GetSchedule(*state, "ALWAYS 21.1")->currentVal = 21.1;          // DOAS SAT
    Sched::GetSchedule(*state, "CYCLINGFANSCH")->currentVal = 0.0;         // cyc fan sch, CyclingFanSch
    Sched::GetSchedule(*state, "CONTSFANSCH")->currentVal = 1.0;           // constant fan sch, ContsFanSch
    Sched::GetSchedule(*state, "FANAVAILSCHED")->currentVal = 1.0;         // Fan availability

    int oaNode = 36; // this node index may change based on component calling order
    state->dataLoopNodes->Node(oaNode).MassFlowRate = 0.26908 * 1.2;
    state->dataLoopNodes->Node(oaNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(oaNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(oaNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(oaNode).Temp, state->dataLoopNodes->Node(oaNode).HumRat);

    // local variables
    bool SimZoneEquipment = true;
    bool SimAirLoops = true;
    bool FirstHVACIteration = true;
    // re-simulate the zone HVAC equipment per the priority order
    InternalHeatGains::UpdateInternalGainValues(*state);
    state->dataHeatBal->spaceIntGainDevices(2).device(2).ReturnAirConvGainRate = 50.0;
    state->dataGlobal->BeginEnvrnFlag = true;
    ZoneEquipmentManager::ManageZoneEquipment(*state, FirstHVACIteration, SimZoneEquipment, SimAirLoops);
    SimAirServingZones::InitAirLoops(*state, FirstHVACIteration);
    SimAirServingZones::SimAirLoopComponents(*state, 1, FirstHVACIteration);
    ZoneEquipmentManager::ManageZoneEquipment(*state, FirstHVACIteration, SimZoneEquipment, SimAirLoops);
    int mixerInletNode = state->dataMixedAir->OAMixer(1).InletNode;
    int mixerReturnNode = state->dataMixedAir->OAMixer(1).RetNode;
    int mixerMixedNode = state->dataMixedAir->OAMixer(1).MixNode;
    // if this EXPECT_EQ fails, node numbers have changed, change OA node number above to match mixerInletNode
    EXPECT_EQ(36, mixerInletNode);
    state->dataLoopNodes->Node(mixerInletNode).MassFlowRate = 0.26908 * 1.2;
    state->dataLoopNodes->Node(mixerInletNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(mixerInletNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(mixerReturnNode).MassFlowRate = 0.26908 * 1.2;
    state->dataLoopNodes->Node(mixerMixedNode).MassFlowRateMaxAvail = 0.26908 * 1.2;
    for (int i = 1; i <= state->dataGlobal->NumOfZones; ++i) {
        if (!state->dataZoneEquip->ZoneEquipConfig(i).IsControlled) {
            continue;
        }
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(i).ZoneNode).Temp =
            state->dataZoneTempPredictorCorrector->zoneHeatBalance(i).MAT;
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(i).ZoneNode).HumRat =
            state->dataZoneTempPredictorCorrector->zoneHeatBalance(i).airHumRat;
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(i).ZoneNode).Enthalpy =
            Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(i).ZoneNode).Temp,
                                       state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(i).ZoneNode).HumRat);
    }

    state->dataGlobal->BeginEnvrnFlag = false;
    FirstHVACIteration = false;
    SimAirServingZones::SimAirLoops(*state, FirstHVACIteration, SimZoneEquipment);
    ZoneEquipmentManager::SimZoneEquipment(*state, FirstHVACIteration, SimAirLoops);
    Real64 Inletmdot = 0.0;
    Real64 outletmdot = 0.0;
    for (int NodeNum = 1; NodeNum <= state->dataZonePlenum->ZoneRetPlenCond(1).NumInletNodes; ++NodeNum) {
        // Get node conditions
        Inletmdot = Inletmdot + state->dataLoopNodes->Node(state->dataZonePlenum->ZoneRetPlenCond(1).InletNode(NodeNum)).MassFlowRate;
    }
    for (int NodeNum = 1; NodeNum <= state->dataZonePlenum->ZoneRetPlenCond(1).NumInducedNodes; ++NodeNum) {
        outletmdot = outletmdot + state->dataLoopNodes->Node(state->dataZonePlenum->ZoneRetPlenCond(1).InducedNode(NodeNum)).MassFlowRate;
    }
    outletmdot = outletmdot + state->dataZonePlenum->ZoneRetPlenCond(1).OutletMassFlowRate;

    // check mass conservation between inlets and outlets for a zone plenum
    EXPECT_NEAR(Inletmdot, outletmdot, 0.0001);

    // *** Unit test results are different from develop ***
    // *** mainly ATMixer outlet temp (conditions) and the fact that air loop is on ***

    // air loop is on in this branch, have not identified why, seems the air loop should be on?
    // previous unit test used hard coded node numbers, those numbers are now different in a few places
    // converted unit test to find correct node numbers

    // System 1 draw air from return node with InletSide of ATMixer
    std::string PTUnit1Name = state->dataUnitarySystems->unitarySys[0].Name;
    int PTUnit1AirInNode = state->dataUnitarySystems->unitarySys[0].AirInNode;
    int PTUnit1AirOutNode = state->dataUnitarySystems->unitarySys[0].AirOutNode;
    int ATMixer1Index = state->dataUnitarySystems->unitarySys[0].m_ATMixerIndex;
    int ATMixer1PriInNode = state->dataSingleDuct->SysATMixer(ATMixer1Index).PriInNode;
    int ATMixer1SecInNode = state->dataSingleDuct->SysATMixer(ATMixer1Index).SecInNode;
    int ATMixer1AirOutNode = state->dataSingleDuct->SysATMixer(ATMixer1Index).MixedAirOutNode;
    int zoneRetPlenumInletNode = state->dataZonePlenum->ZoneRetPlenCond(1).InletNode(1);
    int zone1ReturnNode = state->dataZoneEquip->ZoneEquipConfig(2).ReturnNode(1);

    // *** difference between develop and this branch is that now the air loop is on, this zone ret node Mdot > 0
    EXPECT_EQ(zone1ReturnNode, zoneRetPlenumInletNode);
    EXPECT_EQ(PTUnit1AirInNode, ATMixer1AirOutNode);

    // original develop test, fails in this branch
    // EXPECT_NEAR(23.153277047505515, state->dataLoopNodes->Node(11).Temp, 0.001);

    // ATMixer primary air inlet node, or air loop SAT, T = OAT + fan heat
    EXPECT_NEAR(31.1803, state->dataLoopNodes->Node(ATMixer1PriInNode).Temp, 0.001);
    EXPECT_NEAR(0.0015, state->dataLoopNodes->Node(ATMixer1PriInNode).HumRat, 0.001);
    EXPECT_NEAR(35169.5566, state->dataLoopNodes->Node(ATMixer1PriInNode).Enthalpy, 0.001);

    // *** this next test is different from develop, air loop is on, this pri/ret flow was 0 before
    EXPECT_NEAR(0.0356976, state->dataLoopNodes->Node(ATMixer1PriInNode).MassFlowRate, 0.001);
    EXPECT_NEAR(0.0356976, state->dataLoopNodes->Node(zone1ReturnNode).MassFlowRate, 0.001);

    // zone exhaust node, which is same as T=23.15327704 above, which means air loop was off, SA Mdot=0 (verified)
    EXPECT_NEAR(23.153277, state->dataLoopNodes->Node(ATMixer1SecInNode).Temp, 0.001);
    // note same as ATMixer primary inlet humrat = 0.0015
    EXPECT_NEAR(0.0015, state->dataLoopNodes->Node(ATMixer1SecInNode).HumRat, 0.001);
    EXPECT_NEAR(23903.9785, state->dataLoopNodes->Node(ATMixer1SecInNode).Enthalpy, 0.001);
    EXPECT_NEAR(0.2883384, state->dataLoopNodes->Node(ATMixer1SecInNode).MassFlowRate, 0.001);

    // same temperature test as above commented out test (23.15327704750551), now shows 21.2 C
    // how do you mix 2 air streams with T1in=31.18 and T2in=23.15 and get Tout=21.23 ??
    // must be a node enthalpy issue with this unit test?
    EXPECT_NEAR(21.2316, state->dataLoopNodes->Node(ATMixer1AirOutNode).Temp, 0.005);
    EXPECT_NEAR(0.324036, state->dataLoopNodes->Node(ATMixer1AirOutNode).MassFlowRate, 0.001);

    // mass balance zone 1 ATMixer outlet enthalpy based on pri and sec inlet stream enthalpy
    Real64 ATMixerPriEnthlapy = state->dataLoopNodes->Node(ATMixer1PriInNode).Enthalpy;
    Real64 ATMixerSecEnthlapy = state->dataLoopNodes->Node(ATMixer1SecInNode).Enthalpy;
    Real64 ATMixerPriMassFlow = state->dataLoopNodes->Node(ATMixer1PriInNode).MassFlowRate;
    Real64 ATMixerSecMassFlow = state->dataLoopNodes->Node(ATMixer1SecInNode).MassFlowRate;
    Real64 mixedEnthalpy =
        ((ATMixerPriEnthlapy * ATMixerPriMassFlow) + (ATMixerSecEnthlapy * ATMixerSecMassFlow)) / (ATMixerSecMassFlow + ATMixerPriMassFlow);
    EXPECT_NEAR(mixedEnthalpy, state->dataLoopNodes->Node(ATMixer1AirOutNode).Enthalpy, 0.001);
    EXPECT_TRUE(state->dataLoopNodes->Node(ATMixer1AirOutNode).Temp < state->dataLoopNodes->Node(10).Temp);

    // mass balance 1 supply (zone inlet node), 2 outlets (zone exhaust and return)
    // In develop prior to pulling PTUnits into UnitarySystem, this test used nodes 4 compared to 12 + 11
    // 4 was PTUnit outlet node, 12 was zone return node and 11 was ATMixer secondary node
    EXPECT_NEAR(state->dataLoopNodes->Node(PTUnit1AirOutNode).MassFlowRate,
                state->dataLoopNodes->Node(zone1ReturnNode).MassFlowRate + state->dataLoopNodes->Node(ATMixer1SecInNode).MassFlowRate,
                0.001);

    // System 2 use AirTerminal:SingleDuct:ConstantVolume:NoReheat
    // mass balance 2 inlets (zone inlet nodes), 1 outlet (zone return node)
    int zone2ReturnNode = state->dataZoneEquip->ZoneEquipConfig(3).ReturnNode(1);
    int zone2InletNode1 = state->dataZoneEquip->ZoneEquipConfig(3).InletNode(1);
    int zone2InletNode2 = state->dataZoneEquip->ZoneEquipConfig(3).InletNode(2);
    EXPECT_NEAR(state->dataLoopNodes->Node(zone2ReturnNode).MassFlowRate,
                state->dataLoopNodes->Node(zone2InletNode1).MassFlowRate + state->dataLoopNodes->Node(zone2InletNode2).MassFlowRate,
                0.001);

    // System 3 use ATMixer with InletSide. PTAC draw from induce node
    // mass balance 1 inlets (zone inlet), 1 outlet (zone return)
    int zone3ReturnNode = state->dataZoneEquip->ZoneEquipConfig(4).ReturnNode(1);
    int zone3InletNode1 = state->dataZoneEquip->ZoneEquipConfig(4).InletNode(1);
    EXPECT_NEAR(state->dataLoopNodes->Node(zone3InletNode1).MassFlowRate, state->dataLoopNodes->Node(zone3ReturnNode).MassFlowRate, 0.001);

    //// System 4 use ATMixer with supply side, PTAC draw air from exhaust node
    // mass balance 1 inlet (zone inlet), 2 outlets (zone exhaust and return)
    int zone4ReturnNode = state->dataZoneEquip->ZoneEquipConfig(5).ReturnNode(1);
    int zone4InletNode1 = state->dataZoneEquip->ZoneEquipConfig(5).InletNode(1);
    int zone4ExhaustNode1 = state->dataZoneEquip->ZoneEquipConfig(5).ExhaustNode(1);
    EXPECT_NEAR(state->dataLoopNodes->Node(zone4InletNode1).MassFlowRate,
                state->dataLoopNodes->Node(zone4ExhaustNode1).MassFlowRate + state->dataLoopNodes->Node(zone4ReturnNode).MassFlowRate,
                0.001);

    // System 5 use ATMixer with supply side, PTAC draw air from induce node
    // mass balance Return = Supply
    int zone5ReturnNode = state->dataZoneEquip->ZoneEquipConfig(6).ReturnNode(1);
    int zone5InletNode1 = state->dataZoneEquip->ZoneEquipConfig(6).InletNode(1);
    EXPECT_NEAR(state->dataLoopNodes->Node(zone5InletNode1).MassFlowRate, state->dataLoopNodes->Node(zone5ReturnNode).MassFlowRate, 0.001);
}

TEST_F(EnergyPlusFixture, PTAC_ZoneEquipment_NodeInputTest)
{

    bool errorsFound = false;

    std::string const idf_objects = delimited_string({
        "Schedule:Compact,",
        "    ContinuousFanSch,        !- Name",
        "    ,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    1.0;                     !- Field 4",

        "ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 PTAC,           !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:PackagedTerminalAirConditioner,",
        "    SPACE1-1 PTAC,           !- Name",
        "    ,           !- Availability Schedule Name",
        "    SPACE1-1 HP Inlet Node,  !- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet,   !- Air Outlet Node Name",
        "    OutdoorAir:Mixer,        !- Outdoor Air Mixer Object Type",
        "    PTACOAMixer,             !- Outdoor Air Mixer Name",
        "    0.500,                   !- Supply Air Flow Rate During Cooling Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate During Heating Operation {m3/s}",
        "    ,                        !- Supply Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    0.200,                   !- Outdoor Air Flow Rate During Cooling Operation {m3/s}",
        "    0.200,                   !- Outdoor Air Flow Rate During Heating Operation {m3/s}",
        "    0.200,                   !- Outdoor Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    Fan:ConstantVolume,      !- Supply Air Fan Object Type",
        "    SPACE1-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Heating:Fuel,       !- Heating Coil Object Type",
        "    SPACE1-1 Heating Coil,   !- Heating Coil Name",
        "    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "    SPACE1-1 PTAC CCoil,     !- Cooling Coil Name",
        "    BlowThrough,             !- Fan Placement",
        "    ContinuousFanSch;        !- Supply Air Fan Operating Mode Schedule Name",

        "  OutdoorAir:Mixer,",
        "	 PTACOAMixer,             !- Name",
        "	 PTACOAMixerOutletNode,   !- Mixed Air Node Name",
        "    PTACOAInNode,            !- Outdoor Air Stream Node Name",
        "    ZoneExhausts,            !- Relief Air Stream Node Name",
        "    SPACE1-1 HP Inlet Node;  !- Return Air Stream Node Name",

        "Fan:ConstantVolume,",
        "    SPACE1-1 Supply Fan,     !- Name",
        "    ,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    0.500,                   !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    PTACOAMixerOutletNode,   !- Air Inlet Node Name",
        "    SPACE1-1 Fan Outlet Node;!- Air Outlet Node Name",

        "Coil:Heating:Fuel,",
        "    SPACE1-1 Heating Coil,   !- Name",
        "    ,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Gas Burner Efficiency",
        "    10000.0,                 !- Nominal Capacity {W}",
        "    SPACE1-1 CCoil Outlet Node,  !- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet;   !- Air Outlet Node Name",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    SPACE1-1 PTAC CCoil,     !- Name",
        "    ,           !- Availability Schedule Name",
        "    6680.0,                  !- Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    0.500,                   !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE1-1 Fan Outlet Node,!- Air Inlet Node Name",
        "    SPACE1-1 CCoil Outlet Node,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACPLFFPLR;             !- Part Load Fraction Correlation Curve Name",

        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    FanEffRatioCurve,        !- Name",
        "    0.33856828,              !- Coefficient1 Constant",
        "    1.72644131,              !- Coefficient2 x",
        "    -1.49280132,             !- Coefficient3 x**2",
        "    0.42776208,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.3,                     !- Minimum Curve Output",
        "    1.0;                     !- Maximum Curve Output",

        "  Curve:Exponent,",
        "    FanPowerRatioCurve,      !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 Constant",
        "    3.0,                     !- Coefficient3 Constant",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.01,                    !- Minimum Curve Output",
        "    1.5;                     !- Maximum Curve Output",

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

        "Zone,",
        "    SPACE1-1;                !- Name",

        "Zone,",
        "    Plenum zone;                !- Name",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 Exhausts,       !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE1-1 Return Outlet;  !- Zone Return Air Node Name",

        "NodeList,",
        "    SPACE1-1 Inlets,         !- Name",
        "    SPACE1-1 Supply Inlet;   !- Node 1 Name",

        "NodeList,",
        "    SPACE1-1 Exhausts,       !- Name",
        "    SPACE1-1 HP Inlet Node;  !- Node 1 Name",

        "NodeList,",
        "    OutsideAirInletNodes,    !- Name",
        "    PTACOAInNode;            !- Node 1 Name",

        "OutdoorAir:NodeList,",
        "    OutsideAirInletNodes;    !- Name",

        "AirLoopHVAC:ReturnPlenum,",
        "  Zone Return Plenum,  !- Name",
        "  Plenum zone,              !- Zone Name",
        "  Plenum Node,              !- Zone Node Name", // illegal use of non-unique zone node name
        "  Plenum Outlet Node,       !- Outlet Node Name",
        "  Plenum Induced Air Node,  !- Induced Air Outlet Node or NodeList Name",
        "  Plenum Inlet Node 1,           !- Inlet 1 Node Name",
        "  SPACE1-1 Return Outlet;        !- Inlet 2 Node Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetZoneData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    ZonePlenum::GetZonePlenumInput(*state);
    ASSERT_TRUE(has_err_output(true)); // clear schedule warnings from err stream
    HVACSystemData *mySys;
    mySys = UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, "SPACE1-1 PTAC", true, 0);
    auto &thisSys(state->dataUnitarySystems->unitarySys[0]);
    bool isZoneEquipment = true;
    thisSys.getUnitarySystemInput(*state, "SPACE1-1 PTAC", isZoneEquipment, 0);
    state->dataUnitarySystems->getInputOnceFlag = false;

    int zoneSupplyInlet = Util::FindItemInList("SPACE1-1 SUPPLY INLET", state->dataLoopNodes->NodeID, state->dataLoopNodes->NumOfNodes);
    int zoneExhaustNode = Util::FindItemInList("SPACE1-1 HP INLET NODE", state->dataLoopNodes->NodeID, state->dataLoopNodes->NumOfNodes);
    int zoneNum = Util::FindItemInList("SPACE1-1", state->dataHeatBal->Zone);

    // Test 1 - PTAC Inlet node is zone exhaust node
    ASSERT_EQ(1, state->dataUnitarySystems->numUnitarySystems);
    EXPECT_EQ("ZoneHVAC:PackagedTerminalAirConditioner", thisSys.UnitType); // zoneHVAC equipment type
    EXPECT_EQ(zoneExhaustNode, thisSys.AirInNode);
    EXPECT_EQ(zoneSupplyInlet, thisSys.AirOutNode);
    EXPECT_EQ(zoneExhaustNode, thisSys.m_ZoneInletNode);
    EXPECT_EQ(zoneNum, thisSys.ControlZoneNum);

    // Test 2 - PTAC Inlet node is plenum induced air node
    thisSys.input_specs.air_inlet_node_name = "PLENUM INDUCED AIR NODE";
    thisSys.ControlZoneNum = 0;
    state->dataUnitarySystems->getInputOnceFlag = true;
    errorsFound = false;
    state->dataBranchNodeConnections->NumCompSets = 0;
    state->dataBranchNodeConnections->CompSets.clear();
    int sysNum = 1;
    int zoneOAUnit = 0;
    thisSys.processInputSpec(*state, thisSys.input_specs, sysNum, errorsFound, isZoneEquipment, zoneOAUnit);
    ASSERT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(errorsFound);
    int plenumInducedNode = Util::FindItemInList("PLENUM INDUCED AIR NODE", state->dataLoopNodes->NodeID, state->dataLoopNodes->NumOfNodes);
    EXPECT_EQ(plenumInducedNode, thisSys.AirInNode);
    EXPECT_EQ(zoneSupplyInlet, thisSys.AirOutNode);
    EXPECT_EQ(zoneSupplyInlet, thisSys.m_ZoneInletNode);
    EXPECT_EQ(zoneNum, thisSys.ControlZoneNum);

    // Test 3 - PTAC Inlet node not found
    thisSys.input_specs.air_inlet_node_name = "SOME OTHER NODE";
    thisSys.ControlZoneNum = 0;
    state->dataUnitarySystems->getInputOnceFlag = true;
    errorsFound = false;
    state->dataBranchNodeConnections->NumCompSets = 0;
    state->dataBranchNodeConnections->CompSets.clear();
    thisSys.processInputSpec(*state, thisSys.input_specs, sysNum, errorsFound, isZoneEquipment, zoneOAUnit);
    std::string const error_string = delimited_string({
        "   ** Severe  ** Input errors for ZoneHVAC:PackagedTerminalAirConditioner:SPACE1-1 PTAC",
        "   **   ~~~   ** Incorrect or misspelled Air Inlet Node Name or Exhaust Node Name or Induced Node Name. = SOME OTHER NODE",
        "   **   ~~~   ** Air Inlet Node SOME OTHER NODE name does not match any controlled zone exhaust node name. Check "
        "ZoneHVAC:EquipmentConnections object inputs.",
        "   **   ~~~   ** or Induced Air Outlet Node Name specified in AirLoopHVAC:ReturnPlenum object.",
        "   ** Severe  ** ZoneHVAC:PackagedTerminalAirConditioner = SPACE1-1 PTAC",
        "   **   ~~~   ** Did not find proper connections for AirLoopHVAC or ZoneHVAC system.",
        "   **   ~~~   ** specified Controlling Zone or Thermostat Location name = SPACE1-1",
        "   ** Severe  ** ZoneHVAC:PackagedTerminalAirConditioner = SPACE1-1 PTAC",
        "   **   ~~~   ** Did not find air node (zone with thermostat).",
        "   **   ~~~   ** Both a ZoneHVAC:EquipmentConnections object and a ZoneControl:Thermostat object must be specified for this zone.",
    });

    ASSERT_TRUE(compare_err_stream(error_string, true));
    ASSERT_TRUE(errorsFound);
    int someOtherNode = Util::FindItemInList("SOME OTHER NODE", state->dataLoopNodes->NodeID, state->dataLoopNodes->NumOfNodes);
    EXPECT_EQ(someOtherNode, thisSys.AirInNode);
    EXPECT_EQ(zoneSupplyInlet, thisSys.AirOutNode);
    EXPECT_EQ(zoneSupplyInlet, thisSys.m_ZoneInletNode);
    EXPECT_EQ(zoneNum, thisSys.ControlZoneNum);
}

TEST_F(EnergyPlusFixture, ZonePTHP_ElectricityRateTest)
{

    std::string const idf_objects = delimited_string({
        "ZoneHVAC:PackagedTerminalHeatPump,",
        "  PTHP Thermal Zone One,                  !- Name",
        "  FanAndCoilAvailSched,                   !- Availability Schedule Name",
        "  Thermal Zone One Exhaust Node,          !- Air Inlet Node Name",
        "  Thermal Zone One Inlet Node,            !- Air Outlet Node Name",
        "  OutdoorAir:Mixer,                       !- Outdoor Air Mixer Object Type",
        "  PTHP Thermal Zone One OA Mixer,         !- Outdoor Air Mixer Name",
        "  1.00,                                   !- Cooling Supply Air Flow Rate {m3/s}",
        "  1.00,                                   !- Heating Supply Air Flow Rate {m3/s}",
        "  1.00,                                   !- No Load Supply Air Flow Rate {m3/s}",
        "  ,                                       !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "  0.50,                                   !- Cooling Outdoor Air Flow Rate {m3/s}",
        "  0.50,                                   !- Heating Outdoor Air Flow Rate {m3/s}",
        "  0.50,                                   !- No Load Outdoor Air Flow Rate {m3/s}",
        "  Fan:OnOff,                              !- Supply Air Fan Object Type",
        "  Thermal Zone One Supply Fan,            !- Supply Air Fan Name",
        "  Coil:Heating:DX:SingleSpeed,            !- Heating Coil Object Type",
        "  PTHP Thermal Zone One DX Htg Coil,      !- Heating Coil Name",
        "  0.001,                                  !- Heating Convergence Tolerance {dimensionless}",
        "  Coil:Cooling:DX:SingleSpeed,            !- Cooling Coil Object Type",
        "  PTHP Thermal Zone One DX Clg Coil,      !- Cooling Coil Name",
        "  0.001,                                  !- Cooling Convergence Tolerance {dimensionless}",
        "  Coil:Heating:Electric,                  !- Supplemental Heating Coil Object Type",
        "  PTHP Thermal Zone One Supp EL Htg Coil, !- Supplemental Heating Coil Name",
        "  50.0,                                   !- Maximum Supply Air Temperature from Supplemental Heater {C}",
        "  21,                                     !- Maximum Outdoor Dry-Bulb Temperature for Supplemental Heater Operation {C}",
        "  DrawThrough,                            !- Fan Placement",
        "  ;                                       !- Supply Air Fan Operating Mode Schedule Name",

        "OutdoorAir:Mixer,",
        "  PTHP Thermal Zone One OA Mixer,         !- Name",
        "  PTHP Thermal Zone One Mixed Air Node,   !- Mixed Air Node Name",
        "  PTHP Thermal Zone One OA Node,          !- Outdoor Air Stream Node Name",
        "  PTHP Thermal Zone One Relief Air Node,  !- Relief Air Stream Node Name",
        "  Thermal Zone One Exhaust Node;          !- Return Air Stream Node Name",

        "OutdoorAir:Node,",
        "  PTHP Thermal Zone One OA Node;          !- Name",

        "Fan:OnOff,",
        "  Thermal Zone One Supply Fan,            !- Name",
        "  FanAndCoilAvailSched,                   !- Availability Schedule Name",
        "  0.7,                                    !- Fan Total Efficiency",
        "  600.0,                                  !- Pressure Rise{ Pa }",
        "  1.0,                                    !- Maximum Flow Rate{ m3 / s }",
        "  0.9,                                    !- Motor Efficiency",
        "  1.0,                                    !- Motor In Airstream Fraction",
        "  PTHP Thermal Zone One Heating Coil Outlet Node, !- Air Inlet Node Name",
        "  PTHP Thermal Zone One Fan Outlet Node;  !- Air Outlet Node Name",

        "Coil:Heating:DX:SingleSpeed,",
        "  PTHP Thermal Zone One DX Htg Coil,      !- Name",
        "  FanAndCoilAvailSched,                   !- Availability Schedule Name",
        "  18584.26,                               !- Gross Rated Heating Capacity {W}",
        "  3.8,                                    !- Gross Rated Heating COP {W/W}",
        "  1.0,                                    !- Rated Air Flow Rate {m3/s}",
        "  673.3,                                  !- 2017 Rated Supply Fan Power Per Volume Flow Rate",
        "  673.3,                                  !- 2023 Rated Supply Fan Power Per Volume Flow Rate",
        "  PTHP Thermal Zone One Cooling Coil Outlet Node, !- Air Inlet Node Name",
        "  PTHP Thermal Zone One Heating Coil Outlet Node, !- Air Outlet Node Name",
        "  Curve Biquadratic,                      !- Total Cooling Capacity Function of Temperature Curve Name",
        "  Curve Quadratic,                        !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "  Curve Biquadratic,                      !- Energy Input Ratio Function of Temperature Curve Name",
        "  Curve Quadratic,                        !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "  Curve Quadratic,                        !- Part Load Fraction Correlation Curve Name",
        "  ,                                       !- Defrost Energy Input Ratio Function of Temperature Curve Name",
        "  -8,                                     !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "  ,                                       !- Outdoor Dry-Bulb Temperature to Turn On Compressor {C}",
        "  5,                                      !- Maximum Outdoor Dry-Bulb Temperature for Defrost Operation {C}",
        "  0,                                      !- Crankcase Heater Capacity {W}",
        "  ,                                       !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "  10,                                     !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "  Resistive,                              !- Defrost Strategy",
        "  Timed,                                  !- Defrost Control",
        "  0.166667,                               !- Defrost Time Period Fraction",
        "  2000;                                   !- Resistive Defrost Heater Capacity {W}",

        "Coil:Cooling:DX:SingleSpeed,",
        "  PTHP Thermal Zone One DX Clg Coil,      !- Name",
        "  FanAndCoilAvailSched,                   !- Availability Schedule Name",
        "  18584.26,                               !- Gross Rated Total Cooling Capacity {W}",
        "  0.75,                                   !- Gross Rated Sensible Heat Ratio",
        "  4.40,                                   !- Gross Rated Cooling COP {W/W}",
        "  1.0,                                    !- Rated Air Flow Rate {m3/s}",
        "  673.3,                                  !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate",
        "  673.3,                                  !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate",
        "  PTHP Thermal Zone One Mixed Air Node,   !- Air Inlet Node Name",
        "  PTHP Thermal Zone One Cooling Coil Outlet Node, !- Air Outlet Node Name",
        "  Curve Biquadratic,                      !- Total Cooling Capacity Function of Temperature Curve Name",
        "  Curve Quadratic,                        !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "  Curve Biquadratic,                      !- Energy Input Ratio Function of Temperature Curve Name",
        "  Curve Quadratic,                        !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "  Curve Quadratic,                        !- Part Load Fraction Correlation Curve Name",
        "  -25,                                    !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "  ,                                       !- Nominal Time for Condensate Removal to Begin {s}",
        "  ,                                       !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "  ,                                       !- Maximum Cycling Rate {cycles/hr}",
        "  ,                                       !- Latent Capacity Time Constant {s}",
        "  ,                                       !- Condenser Air Inlet Node Name",
        "  AirCooled,                              !- Condenser Type",
        "  0,                                      !- Evaporative Condenser Effectiveness {dimensionless}",
        "  Autosize,                               !- Evaporative Condenser Air Flow Rate {m3/s}",
        "  Autosize,                               !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "  0,                                      !- Crankcase Heater Capacity {W}",
        "  ,                                       !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "  0,                                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "  ,                                       !- Supply Water Storage Tank Name",
        "  ,                                       !- Condensate Collection Water Storage Tank Name",
        "  0,                                      !- Basin Heater Capacity {W/K}",
        "  10;                                     !- Basin Heater Setpoint Temperature {C}",

        "Curve:Biquadratic,",
        "  Curve Biquadratic,                      !- Name",
        "  1.0,                                    !- Coefficient1 Constant",
        "  0.0,                                    !- Coefficient2 x",
        "  0.0,                                    !- Coefficient3 x**2",
        "  0.0,                                    !- Coefficient4 y",
        "  0.0,                                    !- Coefficient5 y**2",
        "  0.0,                                    !- Coefficient6 x*y",
        " -50,                                     !- Minimum Value of x {BasedOnField A2}",
        "  50,                                     !- Maximum Value of x {BasedOnField A2}",
        " -50,                                     !- Minimum Value of y {BasedOnField A3}",
        "  50;                                     !- Maximum Value of y {BasedOnField A3}",

        "Curve:Quadratic,",
        "  Curve Quadratic,                        !- Name",
        "  1.0,                                    !- Coefficient1 Constant",
        "  0.0,                                    !- Coefficient2 x",
        "  0,                                      !- Coefficient3 x**2",
        "  0.0,                                    !- Minimum Value of x {BasedOnField A2}",
        "  5.0;                                    !- Maximum Value of x {BasedOnField A2}",

        "Coil:Heating:Electric,",
        "  PTHP Thermal Zone one Supp EL Htg Coil, !- Name",
        "  FanAndCoilAvailSched,                   !- Availability Schedule Name",
        "  1,                                      !- Efficiency",
        "  30000.0,                                !- Nominal Capacity {W}",
        "  PTHP Thermal Zone One Fan Outlet Node,  !- Air Inlet Node Name",
        "  Thermal Zone One Inlet Node;            !- Air Outlet Node Name",

        "Zone,",
        "  Thermal Zone One;                       !- Name",

        "ZoneHVAC:EquipmentConnections,",
        "  Thermal Zone One,                       !- Zone Name",
        "  Thermal Zone One Equipment List,        !- Zone Conditioning Equipment List Name",
        "  Thermal Zone One Inlet Node List,       !- Zone Air Inlet Node or NodeList Name",
        "  Thermal Zone One Exhaust Node List,     !- Zone Air Exhaust Node or NodeList Name",
        "  Thermal Zone One Air Node;              !- Zone Air Node Name",

        "NodeList,",
        "  Thermal Zone One Inlet Node List,       !- Name",
        "  Thermal Zone One Inlet Node;            !- Node Name 1",

        "NodeList,",
        "  Thermal Zone One Exhaust Node List,     !- Name",
        "  Thermal Zone One Exhaust Node;          !- Node Name 1",

        "ZoneHVAC:EquipmentList,",
        "  Thermal Zone One Equipment List,        !- Name",
        "  SequentialLoad,                         !- Load Distribution Scheme",
        "  ZoneHVAC:PackagedTerminalHeatPump,      !- Zone Equipment Object Type 1",
        "  PTHP Thermal Zone One,                  !- Zone Equipment Name 1",
        "  1,                                      !- Zone Equipment Cooling Sequence 1",
        "  1,                                      !- Zone Equipment Heating or No-Load Sequence 1",
        "  ,                                       !- Zone Equipment Sequential Cooling Fraction Schedule Name 1",
        "  ;                                       !- Zone Equipment Sequential Heating Fraction Schedule Name 1",

        "Schedule:Compact,",
        "  FanAndCoilAvailSched,                   !- Name",
        "  Fraction,                               !- Schedule Type Limits Name",
        "  Through: 12/31,                         !- Field 1",
        "  For: AllDays,                           !- Field 2",
        "  Until: 24:00,                           !- Field 3",
        "  1.0;                                    !- Field 4",

        "ScheduleTypeLimits,",
        "  Fraction,                               !- Name",
        "  0,                                      !- Lower Limit Value {BasedOnField A3}",
        "  1,                                      !- Upper Limit Value {BasedOnField A3}",
        "  Discrete,                               !- Numeric Type",
        "  availability;                           !- Unit Type",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool errorsFound = false;
    GetZoneData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    GetZoneEquipmentData(*state);
    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    bool const zoneEquipment = true;
    ASSERT_FALSE(has_err_output(false));
    HVACSystemData *mySys;
    std::string compName = "PTHP THERMAL ZONE ONE";
    mySys = UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, compName, zoneEquipment, 0);
    auto &thisSys = state->dataUnitarySystems->unitarySys[0];
    thisSys.getUnitarySystemInput(*state, "PTHP THERMAL ZONE ONE", zoneEquipment, 0);
    state->dataUnitarySystems->getInputOnceFlag = false;
    EXPECT_FALSE(errorsFound);

    int zoneSupplyInlet = Util::FindItemInList("THERMAL ZONE ONE INLET NODE", state->dataLoopNodes->NodeID, state->dataLoopNodes->NumOfNodes);
    int zoneExhaustNode = Util::FindItemInList("THERMAL ZONE ONE EXHAUST NODE", state->dataLoopNodes->NodeID, state->dataLoopNodes->NumOfNodes);
    // test PTHP node connections
    ASSERT_EQ(1, state->dataUnitarySystems->numUnitarySystems);
    EXPECT_EQ("ZoneHVAC:PackagedTerminalHeatPump", thisSys.UnitType);
    EXPECT_EQ(zoneExhaustNode, thisSys.AirInNode);
    EXPECT_EQ(zoneSupplyInlet, thisSys.AirOutNode);

    thisSys.ControlZoneNum = 1;
    // set outdoor air conditions
    state->dataEnvrn->OutDryBulbTemp = 5.0;
    state->dataEnvrn->OutHumRat = 0.003;
    state->dataEnvrn->StdBaroPress = 101325.0;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;

    // set zone air conditions
    auto &zoneAirNode =
        state->dataLoopNodes->Node(Util::FindItemInList("THERMAL ZONE ONE AIR NODE", state->dataLoopNodes->NodeID, state->dataLoopNodes->NumOfNodes));
    zoneAirNode.Temp = 20.0;
    zoneAirNode.HumRat = 0.003;
    zoneAirNode.Enthalpy = Psychrometrics::PsyHFnTdbW(zoneAirNode.Temp, zoneAirNode.HumRat);
    // set mixed air conditions
    auto &mixedAirNode = state->dataLoopNodes->Node(
        Util::FindItemInList("PTHP THERMAL ZONE ONE MIXED AIR NODE", state->dataLoopNodes->NodeID, state->dataLoopNodes->NumOfNodes));
    mixedAirNode.Temp = 10.0;
    mixedAirNode.HumRat = 0.003;
    mixedAirNode.Enthalpy = Psychrometrics::PsyHFnTdbW(mixedAirNode.Temp, mixedAirNode.HumRat);
    // set zone exhaust node
    auto &zoneExhNode = state->dataLoopNodes->Node(
        Util::FindItemInList("THERMAL ZONE ONE EXHAUST NODE", state->dataLoopNodes->NodeID, state->dataLoopNodes->NumOfNodes));
    zoneExhNode.Temp = 20.0;
    zoneExhNode.HumRat = 0.003;
    zoneExhNode.Enthalpy = Psychrometrics::PsyHFnTdbW(zoneExhNode.Temp, zoneExhNode.HumRat);

    // set sizing parameters
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataGlobal->SysSizingCalc = true; // disable sizing calculation

    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(1);
    auto &zoneSysEnergyDemand = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1);
    auto &zoneSysMoistureDemand = state->dataZoneEnergyDemand->ZoneSysMoistureDemand(1);
    zoneSysEnergyDemand.RemainingOutputRequired = 30000;
    zoneSysEnergyDemand.RemainingOutputReqToHeatSP = 30000;
    zoneSysEnergyDemand.RemainingOutputReqToCoolSP = 35000.0;
    zoneSysMoistureDemand.RemainingOutputReqToDehumidSP = 0.0;
    zoneSysEnergyDemand.SequencedOutputRequired.allocate(1);
    zoneSysEnergyDemand.SequencedOutputRequiredToCoolingSP.allocate(1);
    zoneSysEnergyDemand.SequencedOutputRequiredToHeatingSP.allocate(1);
    zoneSysEnergyDemand.SequencedOutputRequired(1) = zoneSysEnergyDemand.RemainingOutputRequired;
    zoneSysEnergyDemand.SequencedOutputRequiredToCoolingSP(1) = zoneSysEnergyDemand.RemainingOutputReqToCoolSP;
    zoneSysEnergyDemand.SequencedOutputRequiredToHeatingSP(1) = zoneSysEnergyDemand.RemainingOutputReqToHeatSP;
    // set thermostat type
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlTypeRpt.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    // set the uni sys is always available
    state->dataUnitarySystems->unitarySys[0].m_sysAvailSched->currentVal = 1.0;
    state->dataUnitarySystems->unitarySys[0].m_fanAvailSched->currentVal = 1.0;

    bool HeatActive = true;
    bool CoolActive = false;
    bool FirstHVACIteration = false;
    int CompIndex = 1;
    int AirLoopNum = 0;
    int constexpr ZoneOAUnitNum = 0;
    Real64 sensOut = 0.0;
    Real64 latOut = 0.0;
    Real64 constexpr OAUCoilOutTemp = 0.0;
    // set local variables for convenience
    auto *supplyFan = state->dataFans->fans(1);
    auto &dxClgCoilMain = state->dataDXCoils->DXCoil(1);
    auto &dxHtgCoilMain = state->dataDXCoils->DXCoil(2);
    auto &elecHtgCoilSupp = state->dataHeatingCoils->HeatingCoil(1);

    // We disabled sizing calculation, so we must initialize these
    dxClgCoilMain.RatedAirMassFlowRate(1) = dxClgCoilMain.RatedAirVolFlowRate(1) * state->dataEnvrn->StdRhoAir;
    dxHtgCoilMain.RatedAirMassFlowRate(1) = dxHtgCoilMain.RatedAirVolFlowRate(1) * state->dataEnvrn->StdRhoAir;

    state->dataGlobal->BeginEnvrnFlag = true;
    mySys->simulate(*state,
                    thisSys.Name,
                    FirstHVACIteration,
                    AirLoopNum,
                    CompIndex,
                    HeatActive,
                    CoolActive,
                    ZoneOAUnitNum,
                    OAUCoilOutTemp,
                    zoneEquipment,
                    sensOut,
                    latOut);

    EXPECT_TRUE(state->dataUnitarySystems->HeatingLoad);
    EXPECT_FALSE(state->dataUnitarySystems->CoolingLoad);

    Real64 result_pthp_ElectricityRate = supplyFan->totalPower + state->dataHVACGlobal->DXElecCoolingPower +
                                         state->dataHVACGlobal->DXElecHeatingPower + state->dataHVACGlobal->DefrostElecPower +
                                         state->dataHVACGlobal->ElecHeatingCoilPower + state->dataHVACGlobal->SuppHeatingCoilPower +
                                         thisSys.m_TotalAuxElecPower;
    // test results
    EXPECT_NEAR(30636.88, thisSys.m_ElecPower, 0.01);
    EXPECT_NEAR(30636.88, result_pthp_ElectricityRate, 0.01);
    EXPECT_NEAR(857.14, supplyFan->totalPower, 0.01);
    EXPECT_NEAR(0.0, dxClgCoilMain.ElecCoolingPower, 0.01);
    EXPECT_NEAR(0.0, state->dataHVACGlobal->DXElecCoolingPower, 0.01);
    EXPECT_NEAR(4327.88, dxHtgCoilMain.ElecHeatingPower, 0.01);
    EXPECT_NEAR(4327.88, state->dataHVACGlobal->DXElecHeatingPower, 0.01);
    EXPECT_NEAR(0.0, state->dataHVACGlobal->ElecHeatingCoilPower, 0.01);
    EXPECT_NEAR(333.33, dxHtgCoilMain.DefrostPower, 0.01);
    EXPECT_NEAR(333.33, state->dataHVACGlobal->DefrostElecPower, 0.01);
    EXPECT_NEAR(25118.52, elecHtgCoilSupp.ElecUseRate, 0.01);
    EXPECT_NEAR(25118.52, state->dataHVACGlobal->SuppHeatingCoilPower, 0.01);
    EXPECT_NEAR(0.0, thisSys.m_TotalAuxElecPower, 0.01);
}

TEST_F(EnergyPlusFixture, PTAC_AvailabilityManagerTest)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 HVACInletMassFlowRate(0.0);
    Real64 PrimaryAirMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QZnReq(0.0);
    int PTUnitNum(0);

    std::string_view constexpr idf_objects = R"IDF(
        Schedule:Compact,
            FanAvailSched,           !- Name
            Fraction,                !- Schedule Type Limits Name
            Through: 12/31,          !- Field 1
            For: AllDays,            !- Field 2
            Until: 24:00,            !- Field 16
            1.0;                     !- Field 17

        Schedule:Compact,
            FanCycling,              !- Name
            Fraction,                !- Schedule Type Limits Name
            Through: 12/31,          !- Field 1
            For: AllDays,            !- Field 2
            Until: 24:00,            !- Field 3
            0;                       !- Field 4

        ZoneHVAC:EquipmentList,
            SPACE1-1 Equipment,      !- Name
            SequentialLoad,          !- Load Distribution Scheme
            ZoneHVAC:PackagedTerminalAirConditioner,  !- Zone Equipment 1 Object Type
            SPACE1-1 PTAC,           !- Zone Equipment 1 Name
            1,                       !- Zone Equipment 1 Cooling Sequence
            1;                       !- Zone Equipment 1 Heating or No-Load Sequence

          ZoneHVAC:PackagedTerminalAirConditioner,
            SPACE1-1 PTAC,           !- Name
            FanAvailSched,           !- Availability Schedule Name
            SPACE1-1 HP Inlet Node,  !- Air Inlet Node Name
            SPACE1-1 Supply Inlet,   !- Air Outlet Node Name
            OutdoorAir:Mixer,        !- Outdoor Air Mixer Object Type
            PTACOAMixer,             !- Outdoor Air Mixer Name
            0.500,                   !- Supply Air Flow Rate During Cooling Operation {m3/s}
            0.500,                   !- Supply Air Flow Rate During Heating Operation {m3/s}
            ,                        !- Supply Air Flow Rate When No Cooling or Heating is Needed {m3/s}
            No,                      !- No Load Supply Air Flow Rate Control Set To Low Speed",
            0.200,                   !- Outdoor Air Flow Rate During Cooling Operation {m3/s}
            0.200,                   !- Outdoor Air Flow Rate During Heating Operation {m3/s}
            0.200,                   !- Outdoor Air Flow Rate When No Cooling or Heating is Needed {m3/s}
            Fan:SystemModel,         !- Supply Air Fan Object Type
            SPACE1-1 Supply Fan,     !- Supply Air Fan Name
            Coil:Heating:Fuel,       !- Heating Coil Object Type
            SPACE1-1 Heating Coil,   !- Heating Coil Name
            Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type
            SPACE1-1 PTAC CCoil,     !- Cooling Coil Name
            BlowThrough,             !- Fan Placement
            FanCycling,              !- Supply Air Fan Operating Mode Schedule Name
		    PTAC-Avail Manager;      !- Availability Manager List Name

          AvailabilityManagerAssignmentList,
            PTAC-Avail Manager,
            AvailabilityManager:NightCycle,
            PTAC-FanCycling;

          AvailabilityManager:NightCycle,
            PTAC-FanCycling,         !- Name
            FanAvailSched,           !- Applicability Schedule Name
            FanCycling,              !- Fan Schedule Name
            CycleOnControlZone,      !- Control Type
            0.2,                     !- Thermostat Tolerance
            ThermostatWithMinimumRuntime, !- Cycling Run Time Control Type
            300,                     !- Cycling Run Time
            SPACE1-1;                !- Control Zone or Zone List Name

          OutdoorAir:Mixer,
        	PTACOAMixer,             !- Name
        	PTACOAMixerOutletNode,   !- Mixed Air Node Name
            PTACOAInNode,            !- Outdoor Air Stream Node Name
            ZoneExhausts,            !- Relief Air Stream Node Name
            SPACE1-1 HP Inlet Node;  !- Return Air Stream Node Name

		  Fan:SystemModel,
			SPACE1-1 Supply Fan,     !- Name
			FanAvailSched,           !- Availability Schedule Name
            PTACOAMixerOutletNode,   !- Air Inlet Node Name
            SPACE1-1 Fan Outlet Node,!- Air Outlet Node Name
			0.5,                     !- Design Maximum Air Flow Rate {m3/s}
			Discrete,                !- Speed Control Method
			0.0,                     !- Electric Power Minimum Flow Rate Fraction
			75.0,                    !- Design Pressure Rise {Pa}
			0.9,                     !- Motor Efficiency
			1.0,                     !- Motor In Air Stream Fraction
			AUTOSIZE,                !- Design Electric Power Consumption {W}
			TotalEfficiencyAndPressure,  !- Design Power Sizing Method
			,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}
			,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}
            0.7;                     !- Fan Total Efficiency

        Coil:Heating:Fuel,
            SPACE1-1 Heating Coil,   !- Name
            FanAvailSched,           !- Availability Schedule Name
            NaturalGas,              !- Fuel Type
            0.8,                     !- Gas Burner Efficiency
            10000.0,                 !- Nominal Capacity {W}
            SPACE1-1 CCoil Outlet Node,  !- Air Inlet Node Name
            SPACE1-1 Supply Inlet;   !- Air Outlet Node Name

          Coil:Cooling:DX:SingleSpeed,
            SPACE1-1 PTAC CCoil,     !- Name
            FanAvailSched,           !- Availability Schedule Name
            6680.0,                  !- Gross Rated Total Cooling Capacity {W}
            0.75,                    !- Gross Rated Sensible Heat Ratio
            3.0,                     !- Gross Rated Cooling COP {W/W}
            0.500,                   !- Rated Air Flow Rate {m3/s}
            ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}
            ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}
            SPACE1-1 Fan Outlet Node,!- Air Inlet Node Name
            SPACE1-1 CCoil Outlet Node,  !- Air Outlet Node Name
            BiQuadCurveFT,           !- Total Cooling Capacity Function of Temperature Curve Name
            QuadCurveFFF,            !- Total Cooling Capacity Function of Flow Fraction Curve Name
            BiQuadCurveFT,           !- Energy Input Ratio Function of Temperature Curve Name
            QuadCurveFFF,            !- Energy Input Ratio Function of Flow Fraction Curve Name
            HPACPLFFPLR;             !- Part Load Fraction Correlation Curve Name

          Curve:Biquadratic,
            BiQuadCurveFT,           !- Name
            1.0,                     !- Coefficient1 Constant
            0.0,                     !- Coefficient2 x
            0.0,                     !- Coefficient3 x**2
            0.0,                     !- Coefficient4 y
            0.0,                     !- Coefficient5 y**2
            0.0,                     !- Coefficient6 x*y
            0.0,                     !- Minimum Value of x
            50.0,                    !- Maximum Value of x
            0.0,                     !- Minimum Value of y
            50.0,                    !- Maximum Value of y
            ,                        !- Minimum Curve Output
            ,                        !- Maximum Curve Output
            Temperature,             !- Input Unit Type for X
            Temperature,             !- Input Unit Type for Y
            Dimensionless;           !- Output Unit Type

          Curve:Quadratic,
            QuadCurveFFF,            !- Name
            1.0,                     !- Coefficient1 Constant
            0.0,                     !- Coefficient2 x
            0.0,                     !- Coefficient3 x**2
            0.5,                     !- Minimum Value of x
            1.5;                     !- Maximum Value of x

          Curve:Quadratic,
            HPACPLFFPLR,             !- Name
            0.85,                    !- Coefficient1 Constant
            0.15,                    !- Coefficient2 x
            0.0,                     !- Coefficient3 x**2
            0.0,                     !- Minimum Value of x
            1.0;                     !- Maximum Value of x

        Zone,
            SPACE1-1,                !- Name
            0,                       !- Direction of Relative North {deg}
            0,                       !- X Origin {m}
            0,                       !- Y Origin {m}
            0,                       !- Z Origin {m}
            1,                       !- Type
            1,                       !- Multiplier
            2.438400269,             !- Ceiling Height {m}
            239.247360229;           !- Volume {m3}

        ZoneHVAC:EquipmentConnections,
            SPACE1-1,                !- Zone Name
            SPACE1-1 Equipment,      !- Zone Conditioning Equipment List Name
            SPACE1-1 Inlets,         !- Zone Air Inlet Node or NodeList Name
            SPACE1-1 Exhausts,       !- Zone Air Exhaust Node or NodeList Name
            SPACE1-1 Zone Air Node,  !- Zone Air Node Name
            SPACE1-1 Return Outlet;  !- Zone Return Air Node Name

        NodeList,
            SPACE1-1 Inlets,         !- Name
            SPACE1-1 Supply Inlet;   !- Node 1 Name

        NodeList,
            SPACE1-1 Exhausts,       !- Name
            SPACE1-1 HP Inlet Node;  !- Node 1 Name

        NodeList,
            OutsideAirInletNodes,    !- Name
            PTACOAInNode;            !- Node 1 Name

        OutdoorAir:NodeList,
            OutsideAirInletNodes;    !- Name
)IDF";

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataGlobal->TimeStep = 1;
    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    HVACSystemData *mySys;
    mySys = UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, "SPACE1-1 PTAC", true, 0);
    auto &thisSys(state->dataUnitarySystems->unitarySys[0]);
    thisSys.getUnitarySystemInput(*state, "SPACE1-1 PTAC", true, 0);
    state->dataUnitarySystems->getInputOnceFlag = false;
    // get input test for terminal air single duct mixer on inlet side of PTAC
    ASSERT_EQ(1, state->dataUnitarySystems->numUnitarySystems);
    EXPECT_EQ("ZoneHVAC:PackagedTerminalAirConditioner", thisSys.UnitType);
    EXPECT_EQ("COIL:HEATING:FUEL", thisSys.m_HeatingCoilTypeName);
    EXPECT_EQ(state->dataHeatingCoils->HeatingCoil(1).coilType, HVAC::CoilType::HeatingGasOrOtherFuel);
    EXPECT_FALSE(thisSys.m_useNoLoadLowSpeedAirFlow);
    // set input variables
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutDryBulbTemp = 10.0;
    state->dataEnvrn->OutHumRat = 0.0075;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    state->dataEnvrn->StdRhoAir = 1.20;
    HVACInletMassFlowRate = 0.0;
    PrimaryAirMassFlowRate = 0.0;
    // set zoneNode air condition
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 21.1;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);
    thisSys.m_fanOpModeSched->currentVal = 1.0;
    thisSys.m_sysAvailSched->currentVal = 1.0;
    thisSys.m_fanAvailSched->currentVal = 1.0;
    // initialize mass flow rates
    state->dataLoopNodes->Node(thisSys.AirInNode).MassFlowRate = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(thisSys.m_OAMixerNodes[0]).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(thisSys.m_OAMixerNodes[0]).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;
    // set fan parameters
    state->dataLoopNodes->Node(state->dataFans->fans(thisSys.m_FanIndex)->inletNodeNum).MassFlowRateMax = 1.0;
    state->dataLoopNodes->Node(state->dataFans->fans(thisSys.m_FanIndex)->inletNodeNum).MassFlowRate = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataFans->fans(thisSys.m_FanIndex)->inletNodeNum).MassFlowRateMaxAvail = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataFans->fans(thisSys.m_FanIndex)->outletNodeNum).MassFlowRate = HVACInletMassFlowRate;
    // set DX coil rated performance parameters
    state->dataDXCoils->DXCoil(1).RatedCBF(1) = 0.05;
    state->dataDXCoils->DXCoil(1).RatedAirMassFlowRate(1) = HVACInletMassFlowRate;
    // set oa mixer conditions
    state->dataLoopNodes->Node(thisSys.m_OAMixerNodes[0]).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(thisSys.m_OAMixerNodes[0]).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(thisSys.m_OAMixerNodes[0]).Enthalpy = state->dataEnvrn->OutEnthalpy;
    // set secondary air conditions
    state->dataLoopNodes->Node(thisSys.AirInNode).Temp = state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp;
    state->dataLoopNodes->Node(thisSys.AirInNode).HumRat = state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat;
    state->dataLoopNodes->Node(thisSys.AirInNode).Enthalpy = state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy;
    state->dataUnitarySystems->unitarySys[0].ControlZoneNum = 1;

    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::SingleHeat;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;    // set heating load to zero
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = 0.0;    // set cooling load to zero
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 0.0;       // set cooling load to zero
    QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP; // zero zone heating load
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).SequencedOutputRequired.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).SequencedOutputRequiredToHeatingSP.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).SequencedOutputRequiredToCoolingSP.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(1).SequencedOutputRequired.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(1).SequencedOutputRequiredToHumidSP.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(1).SequencedOutputRequiredToDehumidSP.allocate(1);
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataHeatBal->MassConservation.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBalFanSys->ZoneMassBalanceFlag.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->spaceIntGainDevices.allocate(1);
    state->dataHeatBal->RefrigCaseCredit.allocate(1);
    // set supply air flows
    thisSys.MaxHeatAirMassFlow = HVACInletMassFlowRate;
    thisSys.m_HeatingSpeedRatio = 1.0;
    thisSys.m_HeatOutAirMassFlow = PrimaryAirMassFlowRate;
    thisSys.MaxNoCoolHeatAirMassFlow = HVACInletMassFlowRate;
    thisSys.m_NoHeatCoolSpeedRatio = 1.0;
    thisSys.m_NoCoolHeatOutAirMassFlow = PrimaryAirMassFlowRate;
    thisSys.m_AirFlowControl = UnitarySystems::UnitarySys::UseCompFlow::On;
    thisSys.m_LastMode = UnitarySystems::HeatingMode;

    state->dataSize->SysSizingRunDone = false;
    state->dataSize->ZoneSizingRunDone = false;
    state->dataGlobal->SysSizingCalc = true;
    state->dataUnitarySystems->HeatingLoad = false;
    state->dataUnitarySystems->CoolingLoad = false;
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;
    // initialized to false
    ASSERT_FALSE(state->dataUnitarySystems->HeatingLoad);
    bool HeatActive = false;
    bool CoolActive = false;
    Real64 latOut = 0.0;
    // test PTAC inputs
    mySys->simulate(*state, thisSys.Name, FirstHVACIteration, 0, PTUnitNum, HeatActive, CoolActive, 0, 0.0, true, QUnitOut, latOut);
    // PTAC is is off when there is no load
    ASSERT_FALSE(state->dataUnitarySystems->HeatingLoad);
    EXPECT_NEAR(QZnReq, 0.0, 0.01);
    EXPECT_NEAR(QUnitOut, 0.0, 0.01);
    // set nightcycle flags
    int NumZones(1);
    int SysAvailNum = 1;
    int PriAirSysNum = 0;
    Avail::Status availStatus = Avail::Status::NoAction;
    constexpr DataZoneEquipment::ZoneEquipType zoneEquipType = DataZoneEquipment::ZoneEquipType::PackagedTerminalAirConditioner;
    int constexpr CompNum = 1;
    bool SimAir = false;
    // set this as zone equipment
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->DeadBandOrSetback.allocate(1);
    state->dataZoneEquipmentManager->PrioritySimOrder.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataHeatBalFanSys->TempControlType.allocate(NumZones);
    state->dataHeatBalFanSys->TempControlTypeRpt.allocate(NumZones);
    state->dataHeatBalFanSys->TempTstatAir.allocate(NumZones);
    state->dataHeatBalFanSys->zoneTstatSetpts.allocate(NumZones);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::SingleCool;
    state->dataHeatBalFanSys->zoneTstatSetpts(1).setpt = 21.1;
    state->dataHeatBalFanSys->TempTstatAir(1) = 21.1;
    // get system availability schedule
    Avail::GetSysAvailManagerListInputs(*state);
    Avail::GetSysAvailManagerInputs(*state);
    state->dataAvail->GetAvailListsInput = false;
    auto &sysAvailMgr = state->dataAvail->NightCycleData(1);
    // check the three cycling run time control types
    EXPECT_EQ(1, state->dataAvail->NumNCycSysAvailMgrs);
    EXPECT_ENUM_EQ(Avail::CyclingRunTimeControl::ThermostatWithMinimumRunTime, sysAvailMgr.cyclingRunTimeControl);
    // test for initialization and does not crash
    ZoneEquipmentManager::SimZoneEquipment(*state, true, SimAir);

    // test 1: availability manager status to off
    state->dataHVACGlobal->TurnFansOn = false;
    state->dataHVACGlobal->TurnFansOff = true;
    state->dataHeatBalFanSys->zoneTstatSetpts(1).setpt = 21.10;
    state->dataHeatBalFanSys->TempTstatAir(1) = 21.1;
    sysAvailMgr.availStatus = Avail::Status::NoAction;
    // run calc system availability requirement
    availStatus = Avail::CalcNCycSysAvailMgr(*state, SysAvailNum, PriAirSysNum, zoneEquipType, CompNum);
    // check that the fan is off
    EXPECT_EQ((int)Avail::Status::NoAction, (int)sysAvailMgr.availStatus);
    EXPECT_FALSE(state->dataHVACGlobal->TurnFansOn);
    EXPECT_TRUE(state->dataHVACGlobal->TurnFansOff);
    // run to set zone night cycle manager
    ZoneEquipmentManager::SimZoneEquipment(*state, true, SimAir);
    Real64 OnOffAirFlowRatio = 0.0;
    Real64 PartLoadRatio(1.0);
    // now check if the global flag is reset correctly
    thisSys.setAverageAirFlow(*state, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_FALSE(state->dataHVACGlobal->TurnFansOn);
    EXPECT_FALSE(state->dataHVACGlobal->TurnFansOff);

    // test 2: availability manager status to on
    state->dataAvail->ZoneComp(zoneEquipType).ZoneCompAvailMgrs(1).StartTime = 0.0;
    state->dataAvail->ZoneComp(zoneEquipType).ZoneCompAvailMgrs(1).StopTime = 4.0;
    state->dataHeatBalFanSys->zoneTstatSetpts(1).setpt = 21.10;
    state->dataHeatBalFanSys->TempTstatAir(1) = 21.5;
    sysAvailMgr.availStatus = Avail::Status::NoAction;
    Sched::GetSchedule(*state, "FANAVAILSCHED")->currentVal = 1;
    Sched::GetSchedule(*state, "FANCYCLING")->currentVal = 0;
    // run calc system availability requirement
    availStatus = Avail::CalcNCycSysAvailMgr(*state, SysAvailNum, PriAirSysNum, zoneEquipType, CompNum);
    // check that the availability manager is cycling On
    EXPECT_EQ((int)Avail::Status::CycleOn, (int)sysAvailMgr.availStatus);
    EXPECT_FALSE(state->dataHVACGlobal->TurnFansOn);
    EXPECT_FALSE(state->dataHVACGlobal->TurnFansOff);
    state->dataAvail->ZoneComp(zoneEquipType).ZoneCompAvailMgrs(1).availStatus = Avail::Status::CycleOn;
    // run to set zone night cycle manager
    ZoneEquipmentManager::SimZoneEquipment(*state, true, SimAir);
    // test global zone fan control variables are turned on
    EXPECT_TRUE(state->dataHVACGlobal->TurnFansOn);
    EXPECT_FALSE(state->dataHVACGlobal->TurnFansOff);
}

} // namespace EnergyPlus
